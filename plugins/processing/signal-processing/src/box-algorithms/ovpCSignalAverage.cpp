#include "ovpCSignalAverage.h"

#include <cmath>
#include <iostream>
#include <sstream>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace OpenViBEToolkit;
using namespace std;

void CSignalAverage::computeAverage(void)
{
	const float64* l_pInput = m_oSignalDecoder.getOutputMatrix()->getBuffer();
	float64* l_pOutput = m_oSignalEncoder.getInputMatrix()->getBuffer();

	const uint32 l_ui32ChannelCount = m_oSignalDecoder.getOutputMatrix()->getDimensionSize(0);
	const uint32 l_ui32SampleCount = m_oSignalDecoder.getOutputMatrix()->getDimensionSize(1);

	//for each channel
	for(uint32 c=0 ; c<l_ui32ChannelCount ; c++)
	{
		float64 l_f64SamplesSum = 0;

		//sum its samples
		for(uint32 i=0 ; i<l_ui32SampleCount ; i++)
		{
			l_f64SamplesSum += l_pInput[(c*l_ui32SampleCount)+i];
		}

		//computes and stores the average for a channel
		l_pOutput[c] = l_f64SamplesSum / l_ui32SampleCount;
	}
}


CSignalAverage::CSignalAverage(void)
{
}

void CSignalAverage::release(void)
{
}

boolean CSignalAverage::initialize()
{	 
	m_oSignalDecoder.initialize(*this,0);
	m_oSignalEncoder.initialize(*this,0);

	return true;
}

boolean CSignalAverage::uninitialize()
{
	m_oSignalEncoder.uninitialize();
	m_oSignalDecoder.uninitialize();

	return true;
}

boolean CSignalAverage::processInput( uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CSignalAverage::process()
{
	IDynamicBoxContext* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();

	// Process input data
	for(uint32 i=0; i<l_pDynamicBoxContext->getInputChunkCount(0); i++)
	{
		m_oSignalDecoder.decode(i);

		if(m_oSignalDecoder.isHeaderReceived())
		{
			// Construct the properties of the output stream
			const IMatrix* l_pInputMatrix = m_oSignalDecoder.getOutputMatrix();
			IMatrix* l_pOutputMatrix = m_oSignalEncoder.getInputMatrix();

			// Sampling rate will be decimated in the output
			const uint64 l_ui64InputSamplingRate =  m_oSignalDecoder.getOutputSamplingRate();
			const uint32 l_ui32InputSampleCount =  l_pInputMatrix->getDimensionSize(1);
			const uint64 l_ui64NewSamplingRate = static_cast<uint64>(ceil((float64)l_ui64InputSamplingRate/(float64)l_ui32InputSampleCount));

			m_oSignalEncoder.getInputSamplingRate() = l_ui64NewSamplingRate;

			// We keep the number of channels, but the output chunk size will be 1
			l_pOutputMatrix->setDimensionCount(2);
			l_pOutputMatrix->setDimensionSize(0,l_pInputMatrix->getDimensionSize(0));
			l_pOutputMatrix->setDimensionSize(1,1);

			for(uint32 j=0;j<l_pOutputMatrix->getDimensionSize(0);j++)
			{
				l_pOutputMatrix->setDimensionLabel(0, j, l_pInputMatrix->getDimensionLabel(0, j));
			}

			m_oSignalEncoder.encodeHeader();

			getBoxAlgorithmContext()->getDynamicBoxContext()->markOutputAsReadyToSend(0, 0, 0);
		}

		if(m_oSignalDecoder.isBufferReceived())
		{
			const uint64 l_ui64LastChunkStartTime = l_pDynamicBoxContext->getInputChunkStartTime(0,i);
			const uint64 l_ui64LastChunkEndTime = l_pDynamicBoxContext->getInputChunkEndTime(0,i);

			computeAverage();

			m_oSignalEncoder.encodeBuffer();

			getBoxAlgorithmContext()->getDynamicBoxContext()->markOutputAsReadyToSend(0, l_ui64LastChunkStartTime, l_ui64LastChunkEndTime);
		}

		if(m_oSignalDecoder.isEndReceived()) 
		{
			// NOP
		}
	}

	return true;
}
