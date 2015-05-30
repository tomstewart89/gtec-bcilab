#include "ovpCTimeBasedEpoching.h"

#include <system/ovCMemory.h>

#include <iostream>

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace OpenViBEToolkit;
using namespace std;

//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------

// This class wraps one output stream encoder and keeps track of the state using the epoch params specific of that output.
class CTimeBasedEpoching::COutputHandler
{
public:

	COutputHandler(CTimeBasedEpoching& rParent, uint32 ui32OutputIndex);
	virtual ~COutputHandler(void);

public:

	virtual void reset(const uint64 ui64DeltaTime);
	virtual bool process();

private:

	COutputHandler(void);

public:

	float64 m_f64EpochDuration;
	float64 m_f64EpochInterval;

protected:

	uint32 m_ui32OutputIndex;
	uint32 m_ui32EpochIndex;

	uint32 m_ui32ChannelCount;
	uint64 m_ui64SamplingRate;
	uint32 m_ui32SampleIndex;
	uint32 m_ui32SampleCountPerEpoch;
	uint32 m_ui32SampleCountBetweenEpoch;

	uint64 m_ui64DeltaTime;

	bool m_bHeaderSent;

	CTimeBasedEpoching& m_rParent;

	OpenViBEToolkit::TSignalEncoder<CTimeBasedEpoching> m_oSignalEncoder;

};

CTimeBasedEpoching::COutputHandler::COutputHandler(CTimeBasedEpoching& rParent, uint32 ui32OutputIndex)
	:m_ui32OutputIndex(ui32OutputIndex)
	,m_ui32EpochIndex(0)
	,m_ui32ChannelCount(0)
	,m_ui64SamplingRate(0)
	,m_ui32SampleIndex(0)
	,m_ui32SampleCountPerEpoch(0)
	,m_ui32SampleCountBetweenEpoch(0)
	,m_ui64DeltaTime(0)
	,m_bHeaderSent(false)
	,m_rParent(rParent)

{
	m_oSignalEncoder.initialize(rParent, ui32OutputIndex);
}

CTimeBasedEpoching::COutputHandler::~COutputHandler(void)
{
	m_oSignalEncoder.uninitialize();
}

void CTimeBasedEpoching::COutputHandler::reset(const uint64 ui64DeltaTime)
{
	m_ui64DeltaTime=ui64DeltaTime;
	m_ui32SampleIndex=0;
	m_ui32EpochIndex=0;
}

bool CTimeBasedEpoching::COutputHandler::process()
{
	if(!m_bHeaderSent)
	{
		OpenViBEToolkit::Tools::Matrix::copyDescription(*m_oSignalEncoder.getInputMatrix(), *m_rParent.m_oSignalDecoder.getOutputMatrix());

		m_ui64SamplingRate = m_rParent.m_oSignalDecoder.getOutputSamplingRate();
		m_ui32ChannelCount = m_rParent.m_oSignalDecoder.getOutputMatrix()->getDimensionSize(0);
		m_ui32SampleCountPerEpoch = ((uint32)(m_f64EpochDuration*m_ui64SamplingRate));
		m_ui32SampleCountBetweenEpoch = ((uint32)(m_f64EpochInterval*m_ui64SamplingRate));

		if(m_ui64SamplingRate == 0)
		{
			m_rParent.getLogManager() << LogLevel_Error << "Input sampling rate is 0, not supported\n";
			return false;
		}

		if(m_ui32SampleCountPerEpoch == 0)
		{
			m_rParent.getLogManager() << LogLevel_Error << "Computed sample count per epoch is 0\n";
			return false;
		}

		m_oSignalEncoder.getInputSamplingRate() = m_ui64SamplingRate;

		m_oSignalEncoder.getInputMatrix()->setDimensionSize(1, m_ui32SampleCountPerEpoch);	// Set new epoch size
		m_oSignalEncoder.encodeHeader();

		m_rParent.getDynamicBoxContext().markOutputAsReadyToSend(m_ui32OutputIndex, 0, 0);

		m_bHeaderSent = true;
	}

	const uint32 l_ui32InputSampleCount = m_rParent.m_oSignalDecoder.getOutputMatrix()->getDimensionSize(1);

	const float64* l_pInputBuffer = m_rParent.m_oSignalDecoder.getOutputMatrix()->getBuffer();
	float64* l_pSampleBuffer = m_oSignalEncoder.getInputMatrix()->getBuffer();

	// Iterates on bytes to process
	uint32 l_ui32SamplesProcessed=0;
	while(l_ui32SamplesProcessed!=l_ui32InputSampleCount)
	{
		if(m_ui32SampleIndex<m_ui32SampleCountPerEpoch) // Some samples should be filled
		{
			// Copies samples to buffer
			const uint32 l_ui32SamplesToFill=min(m_ui32SampleCountPerEpoch-m_ui32SampleIndex, l_ui32InputSampleCount-l_ui32SamplesProcessed);
			for(uint32 i=0; i<m_ui32ChannelCount; i++)
			{
				System::Memory::copy(
					l_pSampleBuffer+i*m_ui32SampleCountPerEpoch+m_ui32SampleIndex,
					l_pInputBuffer+i*l_ui32InputSampleCount+l_ui32SamplesProcessed,
					l_ui32SamplesToFill*sizeof(float64));
			}
			m_ui32SampleIndex+=l_ui32SamplesToFill;
			l_ui32SamplesProcessed+=l_ui32SamplesToFill;

			if(m_ui32SampleIndex==m_ui32SampleCountPerEpoch) // An epoch has been totally filled !
			{
				// Calculates start and end time
				const uint64 l_ui64StartTime=m_ui64DeltaTime+ITimeArithmetics::sampleCountToTime(m_ui64SamplingRate, (uint64)m_ui32EpochIndex*(uint64)m_ui32SampleCountBetweenEpoch);
				const uint64 l_ui64EndTime=  m_ui64DeltaTime+ITimeArithmetics::sampleCountToTime(m_ui64SamplingRate, (uint64)m_ui32EpochIndex*(uint64)m_ui32SampleCountBetweenEpoch+m_ui32SampleCountPerEpoch);
				m_ui32EpochIndex++;

				// m_rParent.getLogManager() << LogLevel_Info << "Out: " << l_ui64StartTime << " to " << l_ui64EndTime << "\n";

				// Writes epoch
				m_oSignalEncoder.encodeBuffer();

				m_rParent.getDynamicBoxContext().markOutputAsReadyToSend(m_ui32OutputIndex, l_ui64StartTime, l_ui64EndTime); // $$$$$$$$$$$
				m_rParent.getLogManager() << LogLevel_Debug << "New epoch written on output " << m_ui32OutputIndex << "(" << l_ui64StartTime << ":" << l_ui64EndTime << ")\n";

				if(m_ui32SampleCountBetweenEpoch<m_ui32SampleCountPerEpoch)
				{
					// Shifts samples for next epoch when overlap
					const uint32 l_ui32SamplesToSave=m_ui32SampleCountPerEpoch-m_ui32SampleCountBetweenEpoch;
					for(uint32 i=0; i<m_ui32ChannelCount; i++)
					{
						System::Memory::move(
							l_pSampleBuffer+i*m_ui32SampleCountPerEpoch,
							l_pSampleBuffer+i*m_ui32SampleCountPerEpoch+m_ui32SampleCountPerEpoch-l_ui32SamplesToSave,
							l_ui32SamplesToSave*sizeof(float64));
					}

					// The counter can be reseted
					m_ui32SampleIndex=l_ui32SamplesToSave;
				}
			}
		}
		else
		{
			// The next few samples are useless
			const uint32 l_ui32SamplesToSkip=min(m_ui32SampleCountBetweenEpoch-m_ui32SampleIndex, l_ui32InputSampleCount-l_ui32SamplesProcessed);
			m_ui32SampleIndex+=l_ui32SamplesToSkip;
			l_ui32SamplesProcessed+=l_ui32SamplesToSkip;

			if(m_ui32SampleIndex==m_ui32SampleCountBetweenEpoch)
			{
				// The counter can be reseted
				m_ui32SampleIndex=0;
			}
		}
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------


CTimeBasedEpoching::CTimeBasedEpoching(void)
	:m_ui32InputSampleCountPerBuffer(0)
{
}

void CTimeBasedEpoching::release(void)
{
	delete this;
}

boolean CTimeBasedEpoching::initialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	m_oSignalDecoder.initialize(*this,0);

	for(uint32 i=0; i<getBoxAlgorithmContext()->getStaticBoxContext()->getOutputCount(); i++)
	{
		CTimeBasedEpoching::COutputHandler* l_pOutputHandler=new CTimeBasedEpoching::COutputHandler(*this, i);
		CString l_sEpochDuration;
		CString l_sEpochInterval;

		float64 l_f64EpochDuration=0;
		float64 l_f64EpochInterval=0;

		l_rStaticBoxContext.getSettingValue(i*2+0, l_sEpochDuration);
		l_rStaticBoxContext.getSettingValue(i*2+1, l_sEpochInterval);

		sscanf(l_sEpochDuration, "%lf", &l_f64EpochDuration);
		sscanf(l_sEpochInterval, "%lf", &l_f64EpochInterval);

		if(l_f64EpochDuration>0 && l_f64EpochInterval>0)
		{
			l_pOutputHandler->m_f64EpochDuration=l_f64EpochDuration;
			l_pOutputHandler->m_f64EpochInterval=l_f64EpochInterval;
		}
		else
		{
			l_pOutputHandler->m_f64EpochDuration=1.0;
			l_pOutputHandler->m_f64EpochInterval=0.5;

			this->getLogManager() << LogLevel_Error << "Epocher settings for output " << i << " are invalid (duration:" << l_f64EpochDuration << "|" << "interval:" << l_f64EpochInterval << ")\n";
			return false;
		}
		m_vOutputHandler.push_back(l_pOutputHandler);
	}

	m_ui64LastStartTime=0;
	m_ui64LastEndTime=0;


	return true;
}

boolean CTimeBasedEpoching::uninitialize(void)
{
	m_oSignalDecoder.uninitialize();

	vector<CTimeBasedEpoching::COutputHandler*>::iterator itOutputHandler;
	for(itOutputHandler=m_vOutputHandler.begin(); itOutputHandler!=m_vOutputHandler.end(); itOutputHandler++)
	{
		delete *itOutputHandler;
	}
	m_vOutputHandler.clear();

	return true;
}

boolean CTimeBasedEpoching::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CTimeBasedEpoching::process(void)
{
	IDynamicBoxContext& l_rDynamicBoxContext=this->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		m_oSignalDecoder.decode(i);

		if(m_oSignalDecoder.isHeaderReceived()) 
		{
			// NOP
		}

		if(m_oSignalDecoder.isBufferReceived()) 
		{
			const uint64 l_ui64ChunkStartTime=l_rDynamicBoxContext.getInputChunkStartTime(0, i);
			const uint64 l_ui64ChunkEndTime=l_rDynamicBoxContext.getInputChunkEndTime(0, i);

			if(m_ui64LastEndTime!=l_ui64ChunkStartTime)
			{
				this->getLogManager() << LogLevel_Debug << "Consecutive chunk start/end time differ (" << m_ui64LastEndTime << ":" << l_ui64ChunkStartTime << "), the epocher will restart\n";
				vector<CTimeBasedEpoching::COutputHandler*>::iterator itOutputHandler;
				for(itOutputHandler=m_vOutputHandler.begin(); itOutputHandler!=m_vOutputHandler.end(); itOutputHandler++)
				{
					(*itOutputHandler)->reset(l_ui64ChunkStartTime);
				}
			}
			else
			{
				this->getLogManager() << LogLevel_Debug << "Consecutive chunk start/end time match (" << m_ui64LastEndTime << ":" << l_ui64ChunkStartTime << ")\n";
			}

			// Add the chunk appropriately to each output stream
			vector<CTimeBasedEpoching::COutputHandler*>::iterator itOutputHandler;
			for(itOutputHandler=m_vOutputHandler.begin(); itOutputHandler!=m_vOutputHandler.end(); itOutputHandler++)
			{
				if(!((*itOutputHandler)->process())) {
					return false;
				}
			}						

			m_ui64LastStartTime=l_ui64ChunkStartTime;
			m_ui64LastEndTime=l_ui64ChunkEndTime;
		}
	}
	return true;
}

//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------

