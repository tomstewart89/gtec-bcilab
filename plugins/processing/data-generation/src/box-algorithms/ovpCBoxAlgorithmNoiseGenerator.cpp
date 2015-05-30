#include "ovpCBoxAlgorithmNoiseGenerator.h"

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <sstream>

#include <openvibe/ovITimeArithmetics.h>
#include <system/ovCMath.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::DataGeneration;
using namespace OpenViBEToolkit;
using namespace std;

CNoiseGenerator::CNoiseGenerator(void)
	:
	m_bHeaderSent(false)
	,m_ui64ChannelCount(0)
	,m_ui64SamplingFrequency(0)
	,m_ui64GeneratedEpochSampleCount(0)
	,m_ui64SentSampleCount(0)
{
}

void CNoiseGenerator::release(void)
{
	delete this;
}

boolean CNoiseGenerator::initialize(void)
{
	m_oSignalEncoder.initialize(*this,0);

	m_ui64ChannelCount=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0); 
	m_ui64SamplingFrequency=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_ui64GeneratedEpochSampleCount=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
	m_bHeaderSent=false;

	if(m_ui64ChannelCount == 0)
	{
		this->getLogManager() << LogLevel_Error << "Channel count is 0. At least 1 channel is required. Check box settings.\n";
		return false;
	}

	if(m_ui64SamplingFrequency == 0)
	{
		this->getLogManager() << LogLevel_Error << "Sampling rate of 0 is not supported. Check box settings.\n";
		return false;
	}

	if(m_ui64GeneratedEpochSampleCount == 0)
	{
		this->getLogManager() << LogLevel_Error << "Epoch sample count is 0. An epoch must have at least 1 sample. Check box settings.\n";
		return false;
	}

	// Set parameters of the encoder

	m_oSignalEncoder.getInputSamplingRate() = m_ui64SamplingFrequency;

	IMatrix* l_pSampleMatrix = m_oSignalEncoder.getInputMatrix();

	l_pSampleMatrix->setDimensionCount(2);
	l_pSampleMatrix->setDimensionSize(0,static_cast<uint32>(m_ui64ChannelCount));
	l_pSampleMatrix->setDimensionSize(1,static_cast<uint32>(m_ui64GeneratedEpochSampleCount));
	for(uint32 i=0;i<static_cast<uint32>(m_ui64ChannelCount);i++)
	{
		// Convention: channel shown to users go as 1,2,...
		std::stringstream ss; ss << "Noise " << (i+1); 
		l_pSampleMatrix->setDimensionLabel(0, i, ss.str().c_str());
	}

	return true;
}

boolean CNoiseGenerator::uninitialize(void)
{
	m_oSignalEncoder.uninitialize();

	return true;
}

boolean CNoiseGenerator::processClock(CMessageClock& rMessageClock)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CNoiseGenerator::process(void)
{
	IBoxIO* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();

	// Send header?
	if(!m_bHeaderSent)
	{
		m_oSignalEncoder.encodeHeader();

		l_pDynamicBoxContext->markOutputAsReadyToSend(0, 0, 0);

		m_bHeaderSent=true;
	}

	// Send buffer
	float64* l_pSampleBuffer = m_oSignalEncoder.getInputMatrix()->getBuffer();
	const uint64 l_ui64SamplesAtBeginning=m_ui64SentSampleCount;
	for(uint32 i=0; i<(uint32)m_ui64ChannelCount; i++)
	{
		for(uint32 j=0; j<(uint32)m_ui64GeneratedEpochSampleCount; j++)
		{
			l_pSampleBuffer[i*m_ui64GeneratedEpochSampleCount+j]=static_cast<float64>(System::Math::randomFloat32BetweenZeroAndOne());
		}
	}

	m_ui64SentSampleCount = l_ui64SamplesAtBeginning + m_ui64GeneratedEpochSampleCount;

	const uint64 l_ui64StartTime = ITimeArithmetics::sampleCountToTime(m_ui64SamplingFrequency, l_ui64SamplesAtBeginning);
	const uint64 l_ui64EndTime = ITimeArithmetics::sampleCountToTime(m_ui64SamplingFrequency, m_ui64SentSampleCount);
		
	m_oSignalEncoder.encodeBuffer();

	l_pDynamicBoxContext->markOutputAsReadyToSend(0, l_ui64StartTime, l_ui64EndTime);


	return true;
}

OpenViBE::uint64 CNoiseGenerator::getClockFrequency(void) 
{
	// Intentional parameter swap to get the frequency
	return ITimeArithmetics::sampleCountToTime(m_ui64GeneratedEpochSampleCount, m_ui64SamplingFrequency);
}
