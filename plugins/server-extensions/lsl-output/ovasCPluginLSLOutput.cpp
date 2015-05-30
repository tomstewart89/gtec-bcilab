
#if defined TARGET_HAS_ThirdPartyLSL

/*
 * Notes: This code should be kept compatible with changes to LSL Input Driver in OpenViBE Acquisition Server, 
 * and LSL Export box in Designer.
 *
 */

#include "ovasCPluginLSLOutput.h"

#include <vector>
#include <iostream>

#include <stdio.h>  // sprintf on Linux

#include <openvibe/ovITimeArithmetics.h>

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

// #define boolean OpenViBE::boolean

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace OpenViBEAcquisitionServerPlugins;
using namespace std;

CPluginLSLOutput::CPluginLSLOutput(const IKernelContext& rKernelContext) :
	IAcquisitionServerPlugin(rKernelContext, CString("AcquisitionServer_Plugin_LabStreamingLayerOutput")),
	m_bIsLSLOutputEnabled(false),
	m_sSignalStreamName("openvibeSignal"),
	m_sMarkerStreamName("openvibeMarkers"),
	m_oSignalOutlet(NULL),
	m_oStimulusOutlet(NULL),
	m_ui32SampleCountPerSentBlock(0)
{
	m_rKernelContext.getLogManager() << LogLevel_Info << "Loading plugin: LSL Output\n";

	m_oSettingsHelper.add("LSL_EnableLSLOutput",  &m_bIsLSLOutputEnabled);
	m_oSettingsHelper.add("LSL_SignalStreamName", &m_sSignalStreamName);
	m_oSettingsHelper.add("LSL_MarkerStreamName", &m_sMarkerStreamName);
	m_oSettingsHelper.load();

	// These are not saved or loaded from .conf as they are supposed to be unique
	m_sSignalStreamID = CIdentifier::random().toString();
	m_sMarkerStreamID = CIdentifier::random().toString();

	while(m_sMarkerStreamID == m_sSignalStreamID) // very unlikely
	{
		m_sMarkerStreamID = CIdentifier::random().toString();
	}

}

CPluginLSLOutput::~CPluginLSLOutput()
{
	if(m_oSignalOutlet) 
	{
		delete m_oSignalOutlet;
		m_oSignalOutlet = NULL;
	}
	if(m_oStimulusOutlet) 
	{
		delete m_oStimulusOutlet;
		m_oStimulusOutlet = NULL;
	}
}

// Hooks


void CPluginLSLOutput::startHook(const std::vector<OpenViBE::CString>& vSelectedChannelNames, OpenViBE::uint32 ui32SamplingFrequency, OpenViBE::uint32 ui32ChannelCount, OpenViBE::uint32 ui32SampleCountPerSentBlock)
{

	m_ui32SampleCountPerSentBlock = ui32SampleCountPerSentBlock;

	if (m_bIsLSLOutputEnabled)
	{
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Will create streams [" << m_sSignalStreamName << ", id " << m_sSignalStreamID << "] and ["
			<< m_sMarkerStreamName << ", id " << m_sMarkerStreamID << "]\n";

		// Open a signal stream 
		lsl::stream_info l_oSignalInfo(m_sSignalStreamName.toASCIIString(),"signal",ui32ChannelCount,ui32SamplingFrequency,lsl::cf_float32,m_sSignalStreamID.toASCIIString());

		lsl::xml_element l_oChannels = l_oSignalInfo.desc().append_child("channels");

		for (uint32 i=0;i<ui32ChannelCount;i++)
		{
			l_oChannels.append_child("channel")
				.append_child_value("label",vSelectedChannelNames[i])
				.append_child_value("unit","unknown")
				.append_child_value("type","signal");
		}

		// make a new outlet
		m_oSignalOutlet = new lsl::stream_outlet(l_oSignalInfo, m_ui32SampleCountPerSentBlock);

		// Open a stimulus stream
		lsl::stream_info l_oStimulusInfo(m_sMarkerStreamName.toASCIIString(),"Markers",1,lsl::IRREGULAR_RATE,lsl::cf_int32,m_sMarkerStreamID.toASCIIString());

		l_oStimulusInfo.desc().append_child("channels")
						.append_child("channel")
						.append_child_value("label","Stimulations")
						.append_child_value("type","marker");

		m_oStimulusOutlet = new lsl::stream_outlet(l_oStimulusInfo);

		m_rKernelContext.getLogManager() << LogLevel_Info << "LSL Output activated...\n";
	}

	// m_rKernelContext.getLogManager() << LogLevel_Trace << "Step from sampling rate is " << 1.0 / static_cast<float64>(ui32SamplingFrequency) << "\n";

}

void CPluginLSLOutput::loopHook(std::vector < std::vector < OpenViBE::float32 > >& vPendingBuffer, CStimulationSet &stimulationSet, uint64 start, uint64 end)
{
	if (m_bIsLSLOutputEnabled)
	{
		// Output signal
		if(m_oSignalOutlet->have_consumers())
		{
			// note: the step computed below should be exactly the same as could be obtained from the sampling rate
			const float64 l_f64Start = ITimeArithmetics::timeToSeconds(start);
			const float64 l_f64Step = ITimeArithmetics::timeToSeconds(end-start)/static_cast<float64>(m_ui32SampleCountPerSentBlock);

			for(uint32 i=0;i<m_ui32SampleCountPerSentBlock;i++)
			{
				m_oSignalOutlet->push_sample(vPendingBuffer[i], l_f64Start + i*l_f64Step);
			}

			// m_rKernelContext.getLogManager() << LogLevel_Info << "Pushed first signal at " << l_f64Start << "\n"; 
			// m_rKernelContext.getLogManager() << LogLevel_Info << "Step is " << l_f64Step << "\n";
		}

		// Output stimuli
		if(m_oStimulusOutlet->have_consumers())
		{
			for(uint32 i=0;i<stimulationSet.getStimulationCount();i++)
			{	
				if(stimulationSet.getStimulationDate(i) >= start &&
				  stimulationSet.getStimulationDate(i) < end)
				{
					const int32 l_i32StimulationCode = static_cast<int32>(stimulationSet.getStimulationIdentifier(i));
					const float64 l_f64StimulationDate = ITimeArithmetics::timeToSeconds(stimulationSet.getStimulationDate(i));

					// m_rKernelContext.getLogManager() << LogLevel_Info << "Push stim " << l_i32StimulationCode << " at " << l_f64StimulationDate << "s\n";
					m_oStimulusOutlet->push_sample(&l_i32StimulationCode,l_f64StimulationDate);
				}
			}
		}
	}

}

void CPluginLSLOutput::stopHook()
{
	if (m_bIsLSLOutputEnabled)
	{
		if(m_oSignalOutlet)
		{
			delete m_oSignalOutlet;
			m_oSignalOutlet = NULL;
		}
		if(m_oStimulusOutlet)
		{
			delete m_oStimulusOutlet;
			m_oStimulusOutlet = NULL;
		}
	}
}


#endif