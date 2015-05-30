
#ifdef TARGET_HAS_ThirdPartyLSL

/*
 * Notes: This code should be kept compatible with changes to LSL Input Driver and Output Plugin in OpenViBE Acquisition Server.
 *
 */
#include "ovpCBoxAlgorithmLSLExport.h"

#include <ctime>
#include <iostream>
#include <string>

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::NetworkIO;

boolean CBoxAlgorithmLSLExport::initialize(void)
{
	m_pSignalOutlet = NULL;
	m_pStimulusOutlet = NULL;
	m_pSingleSampleBuffer = NULL;

	m_oSignalDecoder.initialize(*this, 0);
	m_oStimulationDecoder.initialize(*this, 1);

	m_sSignalStreamName = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_sMarkerStreamName = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

	// These are supposed to be unique, so we don't have them in the box config
	m_sSignalStreamID = CIdentifier::random().toString();
	m_sMarkerStreamID = CIdentifier::random().toString();

	while(m_sMarkerStreamID == m_sSignalStreamID) // very unlikely
	{
		m_sMarkerStreamID = CIdentifier::random().toString();
	}

	this->getLogManager() << LogLevel_Trace << "Will create streams [" << m_sSignalStreamName << ", id " << m_sSignalStreamID << "] and ["
		<< m_sMarkerStreamName << ", id " << m_sMarkerStreamID << "]\n";

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmLSLExport::uninitialize(void)
{
	m_oSignalDecoder.uninitialize();
	m_oStimulationDecoder.uninitialize();

	if(m_pSignalOutlet) 
	{
		delete m_pSignalOutlet;
		m_pSignalOutlet = NULL;
	}
	if(m_pStimulusOutlet) 
	{
		delete m_pStimulusOutlet;
		m_pStimulusOutlet = NULL;
	}

	if(m_pSingleSampleBuffer)
	{
		delete m_pSingleSampleBuffer;
		m_pSingleSampleBuffer = NULL;
	}

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmLSLExport::processInput(uint32 ui32InputIndex)
{
	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmLSLExport::process(void)
{
	// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	// Process signals
	for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(0); j++)
	{
		m_oSignalDecoder.decode(j);
		if(m_oSignalDecoder.isHeaderReceived() && !m_pSignalOutlet)
		{
			const uint32 l_ui32ChannelCount = m_oSignalDecoder.getOutputMatrix()->getDimensionSize(0);
			const uint32 l_ui32SamplesPerBlock = m_oSignalDecoder.getOutputMatrix()->getDimensionSize(1);
			const uint32 l_ui32Frequency = static_cast<uint32>(m_oSignalDecoder.getOutputSamplingRate());

			m_pSingleSampleBuffer = new float[l_ui32ChannelCount];

			// Open a signal stream 
			lsl::stream_info l_oSignalInfo(m_sSignalStreamName.toASCIIString(),"signal",l_ui32ChannelCount,l_ui32Frequency,lsl::cf_float32,m_sSignalStreamID.toASCIIString());

			lsl::xml_element l_oChannels = l_oSignalInfo.desc().append_child("channels");
			m_oSignalDecoder.getOutputMatrix()->getDimensionLabel(0,1);

			for (uint32 i=0;i<l_ui32ChannelCount;i++)
			{
				const char *l_sChannelName = m_oSignalDecoder.getOutputMatrix()->getDimensionLabel(0,i);

				l_oChannels.append_child("channel")
					.append_child_value("label",l_sChannelName)
					.append_child_value("unit","unknown")
					.append_child_value("type","signal");
			}

#ifdef DEBUG
			lsl::xml_element l_oDebug = l_oSignalInfo.desc().child("channels");
			if(l_oDebug.child("channel").child_value("label")) {
				std::cout << "channel label " << l_oDebug.child("channel").child_value("label") << "\n";
			}
#endif

			// make a new outlet
			try {
				m_pSignalOutlet = new lsl::stream_outlet(l_oSignalInfo, l_ui32SamplesPerBlock);
			} 
			catch(...)
			{
				this->getLogManager() << "Unable to create signal outlet\n";
				return false;
			}
		}
		if(m_oSignalDecoder.isBufferReceived()) 
		{
			if(m_pSignalOutlet->have_consumers())
			{
				const IMatrix* l_pMatrix = m_oSignalDecoder.getOutputMatrix();
				const uint32 l_ui32ChannelCount = l_pMatrix->getDimensionSize(0);
				const uint32 l_ui32SamplesPerBlock = l_pMatrix->getDimensionSize(1);
				const float64* l_pInputBuffer = l_pMatrix->getBuffer();

				const uint64 l_ui64StartTime = l_rDynamicBoxContext.getInputChunkStartTime(0, j);
				const uint64 l_ui64EndTime = l_rDynamicBoxContext.getInputChunkEndTime(0, j);

				// note: the step computed below should be exactly the same as could be obtained from the sampling rate
				const float64 l_f64Start = ITimeArithmetics::timeToSeconds(l_ui64StartTime);
				const float64 l_f64Step = ITimeArithmetics::timeToSeconds(l_ui64EndTime-l_ui64StartTime)/static_cast<float64>(l_ui32SamplesPerBlock);

				for(uint32 s=0;s<l_ui32SamplesPerBlock;s++)
				{
					for(uint32 c=0;c<l_ui32ChannelCount;c++) {
						m_pSingleSampleBuffer[c] = static_cast<float32>(l_pInputBuffer[c*l_ui32SamplesPerBlock+s]);
					}
					m_pSignalOutlet->push_sample(m_pSingleSampleBuffer, l_f64Start + s*l_f64Step);
				}

				// m_rKernelContext.getLogManager() << LogLevel_Info << "Pushed first signal at " << l_f64Start << "\n"; 
				// m_rKernelContext.getLogManager() << LogLevel_Info << "Step is " << l_f64Step << "\n";
			}

		}
		if(m_oSignalDecoder.isEndReceived())
		{
			if(m_pSignalOutlet)
			{
				delete m_pSignalOutlet;
				m_pSignalOutlet = NULL;
			}
		}
	}
	
	// Process stimuli -> LSL markers. 
	// Note that stimuli with identifiers not fitting to int32 will be mangled by a static cast.
	for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(1); j++)
	{
		m_oStimulationDecoder.decode(j);
		if(m_oStimulationDecoder.isHeaderReceived() && !m_pStimulusOutlet)
		{
			// Open a stimulus stream
			lsl::stream_info l_oStimulusInfo(m_sMarkerStreamName.toASCIIString(),"Markers",1,lsl::IRREGULAR_RATE,lsl::cf_int32,m_sMarkerStreamID.toASCIIString());

			l_oStimulusInfo.desc().append_child("channels")
							.append_child("channel")
							.append_child_value("label","Stimulations")
							.append_child_value("type","marker");

			try {
				m_pStimulusOutlet = new lsl::stream_outlet(l_oStimulusInfo);
			}
			catch(...)
			{
				this->getLogManager() << "Unable to create marker outlet\n";
				return false;
			}

		}
		if(m_oStimulationDecoder.isBufferReceived())
		{
			// Output stimuli
			if(m_pStimulusOutlet->have_consumers())
			{
				const IStimulationSet* l_pStimulationSet = m_oStimulationDecoder.getOutputStimulationSet();

				for(uint32 s=0;s<l_pStimulationSet->getStimulationCount();s++)
				{	
					const int32 l_i32StimulationCode = static_cast<int32>(l_pStimulationSet->getStimulationIdentifier(s));
					const float64 l_f64StimulationDate = ITimeArithmetics::timeToSeconds(l_pStimulationSet->getStimulationDate(s));

					// m_rKernelContext.getLogManager() << LogLevel_Info << "Push stim " << l_i32StimulationCode << " at " << l_f64StimulationDate << "s\n";
					m_pStimulusOutlet->push_sample(&l_i32StimulationCode,l_f64StimulationDate);
				}
			}
		}
		if(m_oStimulationDecoder.isEndReceived())
		{
			if(m_pStimulusOutlet)
			{
				delete m_pStimulusOutlet;
				m_pStimulusOutlet = NULL;
			}
		}
	}
	return true;
}

#endif 
