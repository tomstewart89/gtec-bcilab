#include "ovpCSinusSignalGenerator.h"

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::DataGeneration;
using namespace OpenViBEToolkit;
using namespace std;

CSinusSignalGenerator::CSinusSignalGenerator(void)
	:m_bHeaderSent(false)
	,m_ui32ChannelCount(0)
	,m_ui32SamplingFrequency(0)
	,m_ui32GeneratedEpochSampleCount(0)
	,m_ui32SentSampleCount(0)
{
}

void CSinusSignalGenerator::release(void)
{
	delete this;
}

boolean CSinusSignalGenerator::initialize(void)
{
	m_oSignalEncoder.initialize(*this,0);

	// Parses box settings to try connecting to server
	CString l_sChannelCount;
	CString l_sSamplingFrequency;
	CString l_sGeneratedEpochSampleCount;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_sChannelCount);
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(1, l_sSamplingFrequency);
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(2, l_sGeneratedEpochSampleCount);
	m_ui32ChannelCount=atoi(l_sChannelCount);
	m_ui32SamplingFrequency=atoi(l_sSamplingFrequency);
	m_ui32GeneratedEpochSampleCount=atoi(l_sGeneratedEpochSampleCount);
	m_bHeaderSent=false;

	if(m_ui32ChannelCount == 0)
	{
		this->getLogManager() << LogLevel_Error << "Channel count is 0. At least 1 channel required. Check box settings.\n";
		return false;
	}

	if(m_ui32SamplingFrequency == 0)
	{
		this->getLogManager() << LogLevel_Error << "Sampling rate of 0 is not supported. Check box settings.\n";
		return false;
	}

	if(m_ui32GeneratedEpochSampleCount == 0)
	{
		this->getLogManager() << LogLevel_Error << "Epoch sample count is 0. An epoch must have at least 1 sample. Check box settings.\n";
		return false;
	}

	return true;
}

boolean CSinusSignalGenerator::uninitialize(void)
{	
	m_oSignalEncoder.uninitialize();

	return true;
}

boolean CSinusSignalGenerator::processClock(CMessageClock& rMessageClock)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CSinusSignalGenerator::process(void)
{
	IBoxIO* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();

	if(!m_bHeaderSent)
	{
		m_oSignalEncoder.getInputSamplingRate() = m_ui32SamplingFrequency;

		IMatrix* l_pMatrix = m_oSignalEncoder.getInputMatrix();

		l_pMatrix->setDimensionCount(2);
		l_pMatrix->setDimensionSize(0,m_ui32ChannelCount);
		l_pMatrix->setDimensionSize(1,m_ui32GeneratedEpochSampleCount);

		for(uint32 i=0; i<m_ui32ChannelCount; i++)
		{
			// Convention: channel shown as users go as 1,2,...
			char l_sChannelName[1024];
			sprintf(l_sChannelName, "sinusOsc %i", (int)(i+1));
			l_pMatrix->setDimensionLabel(0, i, l_sChannelName);
		}

		m_oSignalEncoder.encodeHeader();

		m_bHeaderSent=true;

		uint64 l_ui64Time=ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, m_ui32SentSampleCount);
		l_pDynamicBoxContext->markOutputAsReadyToSend(0, l_ui64Time, l_ui64Time);
	}
	else
	{
		float64* l_pSampleBuffer = m_oSignalEncoder.getInputMatrix()->getBuffer();

		uint32 l_ui32SentSampleCount=m_ui32SentSampleCount;
		for(uint32 i=0; i<m_ui32ChannelCount; i++)
		{
			for(uint32 j=0; j<m_ui32GeneratedEpochSampleCount; j++)
			{
				l_pSampleBuffer[i*m_ui32GeneratedEpochSampleCount+j]=
					sin(((j+m_ui32SentSampleCount)*(i+1)*12.3)/m_ui32SamplingFrequency)+
					sin(((j+m_ui32SentSampleCount)*(i+1)* 4.5)/m_ui32SamplingFrequency)+
					sin(((j+m_ui32SentSampleCount)*(i+1)*67.8)/m_ui32SamplingFrequency);
			}
		}

		m_oSignalEncoder.encodeBuffer();

		m_ui32SentSampleCount+=m_ui32GeneratedEpochSampleCount;

		uint64 l_ui64StartTime = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, l_ui32SentSampleCount);
		uint64 l_ui64EndTime = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, m_ui32SentSampleCount);

		l_pDynamicBoxContext->markOutputAsReadyToSend(0, l_ui64StartTime, l_ui64EndTime);
	}

	return true;
}

OpenViBE::uint64 CSinusSignalGenerator::getClockFrequency(void) 
{
	// Intentional parameter swap to get the frequency
	return ITimeArithmetics::sampleCountToTime(m_ui32GeneratedEpochSampleCount, m_ui32SamplingFrequency);
}
