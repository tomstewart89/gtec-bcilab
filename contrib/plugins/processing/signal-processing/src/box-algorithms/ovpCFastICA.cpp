#if defined TARGET_HAS_ThirdPartyITPP

#include "ovpCFastICA.h"

#include <iostream>
#include <sstream>

#include <itpp/itstat.h>
#include <itpp/itsignal.h>

using namespace itpp;

using namespace OpenViBE;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace OpenViBEToolkit;
using namespace std;

void CFastICA::computeICA(void)
{
	const uint32 l_ui32ChannelCount = m_oDecoder.getOutputMatrix()->getDimensionSize(0);
	const uint32 l_ui32SampleCount = m_oDecoder.getOutputMatrix()->getDimensionSize(1);
	const float64 *l_pInputBuffer = m_oDecoder.getOutputMatrix()->getBuffer();
	float64 *l_pOutputBuffer = m_oEncoder.getInputMatrix()->getBuffer();

	mat sources(l_ui32ChannelCount, l_ui32SampleCount);
	mat ICs(l_ui32ChannelCount, l_ui32SampleCount);

	for (uint32 i=0;  i < l_ui32ChannelCount; i++)
	{
		for(uint32 j=0 ; j<l_ui32SampleCount ; j++)
		{
			sources((int)i, (int)j) =  (double)l_pInputBuffer[i*l_ui32SampleCount+j];
		}
	}

	Fast_ICA fastica(sources);
	fastica.set_nrof_independent_components(sources.rows());
	fastica.set_non_linearity(FICA_NONLIN_TANH);
	fastica.set_approach(FICA_APPROACH_DEFL);
	fastica.separate();
	ICs = fastica.get_independent_components();

	for (uint32 i=0;  i < l_ui32ChannelCount; i++)
	{
		for(uint32 j=0 ; j < l_ui32SampleCount ; j++)
		{
			l_pOutputBuffer[i*l_ui32SampleCount+j] = ICs((int)i,(int)j);
		}
	}
}

CFastICA::CFastICA(void)
{
}

void CFastICA::release(void)
{
	delete this;
}

boolean CFastICA::initialize()
{
	m_oDecoder.initialize(*this, 0);
	m_oEncoder.initialize(*this, 0);

	return true;
}

boolean CFastICA::uninitialize()
{
	m_oEncoder.uninitialize();
	m_oDecoder.uninitialize();

	return true;
}

boolean CFastICA::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CFastICA::process()
{
	IDynamicBoxContext* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();

	// Process input data
	for(uint32 i=0; i<l_pDynamicBoxContext->getInputChunkCount(0); i++)
	{
		m_oDecoder.decode(i);

		if(m_oDecoder.isHeaderReceived()) 
		{
			// Set the output (encoder) matrix prorperties from the input (decoder)
			IMatrix* l_pEncoderMatrix = m_oEncoder.getInputMatrix();

			OpenViBEToolkit::Tools::Matrix::copyDescription(*l_pEncoderMatrix, *m_oDecoder.getOutputMatrix());
			m_oEncoder.getInputSamplingRate() = m_oDecoder.getOutputSamplingRate();

			for(uint32 i=0 ; i<l_pEncoderMatrix->getDimensionSize(0) ; i++)
			{
				char l_sBuffer[64];
				sprintf(l_sBuffer, "IC %d", i+1);
				l_pEncoderMatrix->setDimensionLabel(0,i, l_sBuffer);
			}

			m_oEncoder.encodeHeader();

			getBoxAlgorithmContext()->getDynamicBoxContext()->markOutputAsReadyToSend(0, 0, 0);
		}

		if(m_oDecoder.isBufferReceived()) 
		{
			const uint64 l_ui64LastChunkStartTime = l_pDynamicBoxContext->getInputChunkStartTime(0,i);
			const uint64 l_ui64LastChunkEndTime = l_pDynamicBoxContext->getInputChunkEndTime(0,i);

			computeICA();

			m_oEncoder.encodeBuffer();

			getBoxAlgorithmContext()->getDynamicBoxContext()->markOutputAsReadyToSend(0, l_ui64LastChunkStartTime, l_ui64LastChunkEndTime);
		}

		if(m_oDecoder.isEndReceived()) 
		{
			// NOP
		}
	}

	return true;
}

#endif // TARGET_HAS_ThirdPartyITPP
