#if defined TARGET_HAS_ThirdPartyITPP

#include "ovpCWindowingFunctions.h"

#include <iostream>
#include <sstream>

#include <itpp/itcomm.h>

using namespace itpp;

using namespace OpenViBE;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace OpenViBEPlugins::SignalProcessing::WindowingFunctions;
using namespace OpenViBEToolkit;
using namespace std;


void CWindowingFunctions::setSampleBuffer(const float64* pBuffer)
{
	vec l_vecWindow((int)m_ui64SamplesPerBuffer);

	if (m_ui64WindowMethod==OVP_TypeId_WindowMethod_Hamming)
	{
		l_vecWindow = hamming((int)m_ui64SamplesPerBuffer);
	}
	else if (m_ui64WindowMethod==OVP_TypeId_WindowMethod_Hanning)
	{
		l_vecWindow = hanning((int)m_ui64SamplesPerBuffer);
	}
	else if (m_ui64WindowMethod==OVP_TypeId_WindowMethod_Hann)
	{
		l_vecWindow = hann((int)m_ui64SamplesPerBuffer);
	}
	else if (m_ui64WindowMethod==OVP_TypeId_WindowMethod_Blackman)
	{
		l_vecWindow = blackman((int)m_ui64SamplesPerBuffer);
	}
	else if (m_ui64WindowMethod==OVP_TypeId_WindowMethod_Triangular)
	{
		l_vecWindow = triang((int)m_ui64SamplesPerBuffer);
	}
	else if (m_ui64WindowMethod==OVP_TypeId_WindowMethod_SquareRoot)
	{
		l_vecWindow = sqrt_win((int)m_ui64SamplesPerBuffer);
	}
	else
	{
		l_vecWindow = ones((int)m_ui64SamplesPerBuffer);
	}

	for (uint64 i=0;  i < m_ui64ChannelCount; i++)
	{
		for(uint64 j=0 ; j<m_ui64SamplesPerBuffer ; j++)
		{
			m_pMatrixBuffer[i*m_ui64SamplesPerBuffer+j] =  (double)pBuffer[i*m_ui64SamplesPerBuffer+j]*l_vecWindow((int)j);
		}
	}
}

CWindowingFunctions::CWindowingFunctions(void):
	m_ui64LastChunkStartTime(0),
	m_ui64LastChunkEndTime(0),
	m_ui64MatrixBufferSize(0),
	m_pMatrixBuffer(NULL)
{
}

void CWindowingFunctions::release(void)
{
	delete this;
}

boolean CWindowingFunctions::initialize()
{
	//reads the plugin settings
	CString l_sWindowMethod;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_sWindowMethod);
	m_ui64WindowMethod=this->getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_WindowMethod, l_sWindowMethod);

	m_pSignalDecoder = new OpenViBEToolkit::TSignalDecoder < CWindowingFunctions >(*this,0);
	m_pSignalEncoder = new OpenViBEToolkit::TSignalEncoder < CWindowingFunctions >(*this,0);

	return true;
}

boolean CWindowingFunctions::uninitialize()
{
	m_pSignalDecoder->uninitialize();
	delete m_pSignalDecoder;
	m_pSignalEncoder->uninitialize();
	delete m_pSignalEncoder;

	return true;
}

boolean CWindowingFunctions::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CWindowingFunctions::process()
{
	IDynamicBoxContext* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();

	//reset the OutPut chunk
	//l_pDynamicBoxContext->setOutputChunkSize(0, 0);




	// Process input data
	for(uint32 i=0; i<l_pDynamicBoxContext->getInputChunkCount(0); i++)
	{
		uint64 l_ui64ChunkSize;
		const uint8* l_pBuffer;
		l_pDynamicBoxContext->getInputChunk(0, i, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime, l_ui64ChunkSize, l_pBuffer);


		m_pSignalDecoder->decode(i);
		if(m_pSignalDecoder->isHeaderReceived())
		{
			IMatrix* l_pInputMatrix = m_pSignalDecoder->getOutputMatrix();
			IMatrix* l_pOutputMatrix = m_pSignalEncoder->getInputMatrix();

			OpenViBEToolkit::Tools::MatrixManipulation::copy(*l_pOutputMatrix, *l_pInputMatrix);

			m_pMatrixBuffer = l_pOutputMatrix->getBuffer();
			m_ui64SamplesPerBuffer = l_pOutputMatrix->getDimensionSize(1);
			m_ui64ChannelCount = l_pOutputMatrix->getDimensionSize(0);

			const uint64 l_ui64SamplingRate = m_pSignalDecoder->getOutputSamplingRate();
			m_pSignalEncoder->getInputSamplingRate() = l_ui64SamplingRate;

			m_pSignalEncoder->encodeHeader();
			l_pDynamicBoxContext->markOutputAsReadyToSend(i, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
		}
		if(m_pSignalDecoder->isBufferReceived())
		{
			IMatrix* l_pInputMatrix = m_pSignalDecoder->getOutputMatrix();
			m_pMatrixBuffer = m_pSignalEncoder->getInputMatrix()->getBuffer();
			setSampleBuffer(l_pInputMatrix->getBuffer());

			m_pSignalEncoder->encodeBuffer();
			l_pDynamicBoxContext->markOutputAsReadyToSend(i, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
		}
		if(m_pSignalDecoder->isEndReceived())
		{
			m_pSignalEncoder->encodeEnd();
			l_pDynamicBoxContext->markOutputAsReadyToSend(i, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
		}






		l_pDynamicBoxContext->markInputAsDeprecated(0, i);
		//m_pReader->processData(l_pBuffer, l_ui64ChunkSize);
	}

	return true;
}

#endif // TARGET_HAS_ThirdPartyITPP
