#if defined TARGET_HAS_ThirdPartyITPP

#include "ovpCSpectralAnalysis.h"

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
using namespace OpenViBE::Kernel;

CSpectralAnalysis::CSpectralAnalysis(void)
	:m_ui64LastChunkStartTime(0),
	m_ui64LastChunkEndTime(0),
	m_ui32ChannelCount(0),
	m_ui32SampleCount(0),
	m_ui32HalfFFTSize(1)

{
}

void CSpectralAnalysis::release(void)
{
	delete this;
}

boolean CSpectralAnalysis::initialize()
{
	//reads the plugin settings
	CString l_sSpectralComponents;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_sSpectralComponents);
	uint64 l_ui64SpectralComponents=this->getTypeManager().getBitMaskEntryCompositionValueFromName(OVP_TypeId_SpectralComponent, l_sSpectralComponents);

	m_bAmplitudeSpectrum = ((l_ui64SpectralComponents & OVP_TypeId_SpectralComponent_Amplitude.toUInteger())>0);
	m_bPhaseSpectrum     = ((l_ui64SpectralComponents & OVP_TypeId_SpectralComponent_Phase.toUInteger())>0);
	m_bRealPartSpectrum  = ((l_ui64SpectralComponents & OVP_TypeId_SpectralComponent_RealPart.toUInteger())>0);
	m_bImagPartSpectrum  = ((l_ui64SpectralComponents & OVP_TypeId_SpectralComponent_ImaginaryPart.toUInteger())>0);

	m_oSignalDecoder.initialize(*this,0);
	for(uint32 i=0; i<4; i++)
	{
		m_vSpectrumEncoder[i].initialize(*this,i);
	}

	return true;
}

boolean CSpectralAnalysis::uninitialize()
{
	for(uint32 i=0; i<4; i++)
	{
		m_vSpectrumEncoder[i].uninitialize();
	}
	m_oSignalDecoder.uninitialize();

	return true;
}

boolean CSpectralAnalysis::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CSpectralAnalysis::process()
{
	IBoxIO * l_pDynamicContext = getBoxAlgorithmContext()->getDynamicBoxContext();
	uint32 l_ui32InputChunkCount = l_pDynamicContext->getInputChunkCount(0);
	float64 l_float64BandStart, l_float64BandStop;
	char l_sFrequencyBandName [1024];

	for(uint32 chunkIdx = 0; chunkIdx < l_ui32InputChunkCount; chunkIdx++)
    {
		m_ui64LastChunkStartTime=l_pDynamicContext->getInputChunkStartTime(0, chunkIdx);
		m_ui64LastChunkEndTime=l_pDynamicContext->getInputChunkEndTime(0, chunkIdx);

		m_oSignalDecoder.decode(chunkIdx);

		if(m_oSignalDecoder.isHeaderReceived())//dealing with the signal header
		{
			//get signal info
			m_ui32SampleCount = m_oSignalDecoder.getOutputMatrix()->getDimensionSize(1);
			m_ui32ChannelCount = m_oSignalDecoder.getOutputMatrix()->getDimensionSize(0);
			m_ui32SamplingRate = (uint32)m_oSignalDecoder.getOutputSamplingRate();

			if(m_ui32SampleCount == 0) {
				this->getLogManager() << LogLevel_Error << "Chunk size appears to be 0, not supported.\n";
				return false;
			}
			if(m_ui32ChannelCount == 0) {
				this->getLogManager() << LogLevel_Error << "Channel count appears to be 0, not supported.\n";
				return false;
			}
			if(m_ui32SamplingRate == 0) {
				this->getLogManager() << LogLevel_Error << "Sampling rate appears to be 0, not supported.\n";
				return false;
			}

			//we need two matrices for the spectrum encoders, the Frequency bands and the one inherited form streamed matrix (see doc for details)
			CMatrix* l_pFrequencyBands = new CMatrix();
			CMatrix* l_pStreamedMatrix = new CMatrix();
			l_pFrequencyBands->setDimensionCount(2);

			// For real signals, if N is sample count, bins [0,N/2] (inclusive) contain non-redundant information, i.e. N/2+1 entries.
			m_ui32HalfFFTSize = m_ui32SampleCount / 2 + 1;
			m_ui32FrequencyBandCount = m_ui32HalfFFTSize;

			OpenViBEToolkit::Tools::MatrixManipulation::copyDescription(*l_pStreamedMatrix, *m_oSignalDecoder.getOutputMatrix());
			l_pStreamedMatrix->setDimensionSize(1,m_ui32FrequencyBandCount);
			l_pFrequencyBands->setDimensionSize(0,2);
			l_pFrequencyBands->setDimensionSize(1,m_ui32FrequencyBandCount);
			float64* l_pBuffer = l_pFrequencyBands->getBuffer();

			// @fixme would be more proper to use 'bins', one bin with a hz tag per array entry
			for (uint32 j=0; j < m_ui32FrequencyBandCount; j++)
			{
				l_float64BandStart = static_cast<float64>(j*(m_ui32SamplingRate/(float64)m_ui32SampleCount));
				l_float64BandStop = static_cast<float64>((j+1)*(m_ui32SamplingRate/(float64)m_ui32SampleCount));
				if (l_float64BandStop <l_float64BandStart )
				{
					l_float64BandStop = l_float64BandStart;
				}

				*(l_pBuffer+2*j) = l_float64BandStart;
				*(l_pBuffer+2*j+1) = l_float64BandStop;


				sprintf(l_sFrequencyBandName, "%lg-%lg", l_float64BandStart, l_float64BandStop);
				l_pStreamedMatrix->setDimensionLabel(1,j,l_sFrequencyBandName);//set the names of the frequency bands
			}

			for(uint32 i=0;i<4;i++)
			{
				//copy the information for each encoder
				OpenViBEToolkit::Tools::MatrixManipulation::copy(*m_vSpectrumEncoder[i].getInputMinMaxFrequencyBands(),*l_pFrequencyBands);
				OpenViBEToolkit::Tools::MatrixManipulation::copy(*m_vSpectrumEncoder[i].getInputMatrix(),*l_pStreamedMatrix);
			}

			if (m_bAmplitudeSpectrum)
			{
				m_vSpectrumEncoder[0].encodeHeader();
				l_pDynamicContext->markOutputAsReadyToSend(0,m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			}
			if (m_bPhaseSpectrum)
			{
				m_vSpectrumEncoder[1].encodeHeader();
				l_pDynamicContext->markOutputAsReadyToSend(1,m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			}
			if (m_bRealPartSpectrum)
			{
				m_vSpectrumEncoder[2].encodeHeader();
				l_pDynamicContext->markOutputAsReadyToSend(2,m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			}
			if (m_bImagPartSpectrum)
			{
				m_vSpectrumEncoder[3].encodeHeader();
				l_pDynamicContext->markOutputAsReadyToSend(3,m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			}

			delete l_pFrequencyBands;
			delete l_pStreamedMatrix;
		}
		if(m_oSignalDecoder.isBufferReceived())
		{
			//get input buffer
			const float64* l_pBuffer = m_oSignalDecoder.getOutputMatrix()->getBuffer();
			//do the processing
			vec x(m_ui32SampleCount);
			cvec y(m_ui32SampleCount);
			cvec z(m_ui32ChannelCount*m_ui32HalfFFTSize);

			for (uint64 i=0;  i < m_ui32ChannelCount; i++)
			{
				for(uint64 j=0 ; j<m_ui32SampleCount ; j++)
				{
					x[(int)j] =  (double)*(l_pBuffer+i*m_ui32SampleCount+j);
				}

				y = fft_real(x);

				//test block
				// vec h = ifft_real(y);
				// std::cout << "Fx: " << x.size() << ", x=" << x << "\n";
				// std::cout << "FF: " << y.size() << ", y=" << y << "\n";
				// std::cout << "Fr: " << h.size() << ", x'=" << h << "\n";

				for(uint64 k=0 ; k<m_ui32HalfFFTSize ; k++)
				{
					z[(int)(k+i*m_ui32HalfFFTSize)] = y[(int)k];
				}
			}

			if (m_bAmplitudeSpectrum)
			{
				IMatrix* l_pMatrix = m_vSpectrumEncoder[0].getInputMatrix();
				float64* l_pBuffer = l_pMatrix->getBuffer();
				for (uint64 i=0;  i < m_ui32ChannelCount*m_ui32HalfFFTSize; i++)
				{
					*(l_pBuffer+i) = sqrt(real(z[(int)i])*real(z[(int)i])+ imag(z[(int)i])*imag(z[(int)i]));
				}
				m_vSpectrumEncoder[0].encodeBuffer();
				l_pDynamicContext->markOutputAsReadyToSend(0,m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			}
			if (m_bPhaseSpectrum)
			{
				IMatrix* l_pMatrix = m_vSpectrumEncoder[1].getInputMatrix();
				float64* l_pBuffer = l_pMatrix->getBuffer();
				for (uint64 i=0;  i < m_ui32ChannelCount*m_ui32HalfFFTSize; i++)
				{
					*(l_pBuffer+i) =  imag(z[(int)i])/real(z[(int)i]);
				}
				m_vSpectrumEncoder[1].encodeBuffer();
				l_pDynamicContext->markOutputAsReadyToSend(1,m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			}
			if (m_bRealPartSpectrum)
			{
				IMatrix* l_pMatrix = m_vSpectrumEncoder[2].getInputMatrix();
				float64* l_pBuffer = l_pMatrix->getBuffer();
				for (uint64 i=0;  i < m_ui32ChannelCount*m_ui32HalfFFTSize; i++)
				{
					*(l_pBuffer+i) = real(z[(int)i]);
				}
				m_vSpectrumEncoder[2].encodeBuffer();
				l_pDynamicContext->markOutputAsReadyToSend(2,m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			}
			if (m_bImagPartSpectrum)
			{
				IMatrix* l_pMatrix = m_vSpectrumEncoder[3].getInputMatrix();
				float64* l_pBuffer = l_pMatrix->getBuffer();
				for (uint64 i=0;  i < m_ui32ChannelCount*m_ui32HalfFFTSize; i++)
				{
					*(l_pBuffer+i) = imag(z[(int)i]);
				}
				m_vSpectrumEncoder[3].encodeBuffer();
				l_pDynamicContext->markOutputAsReadyToSend(3,m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			}
		}
		l_pDynamicContext->markInputAsDeprecated(0,chunkIdx);
	}
	return true;
}

#endif
