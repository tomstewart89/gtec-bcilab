
#if defined(TARGET_HAS_ThirdPartyFFTW3) // fftw3 required by wavelet2s

#include "ovpCBoxAlgorithmDiscreteWaveletTransform.h"

#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>

#include "../../../contrib/packages/wavelet2d/wavelet2s.h"


using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

boolean CBoxAlgorithmDiscreteWaveletTransform::initialize(void)
{

	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	// Signal stream decoder
	m_oAlgo0_SignalDecoder.initialize(*this,0);

	//Signal stream encoder
	m_oAlgoInfo_SignalEncoder.initialize(*this,0);

	m_sWaveletType=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_sDecompositionLevel=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

	for (uint32 o = 0; o < l_rStaticBoxContext.getOutputCount()-1; o++)
	{
		m_vAlgoX_SignalEncoder.push_back(new OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmDiscreteWaveletTransform >(*this, o+1));
	}

	m_ui32Infolength = 0;

	return true;
}


boolean CBoxAlgorithmDiscreteWaveletTransform::uninitialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	m_oAlgo0_SignalDecoder.uninitialize();
	m_oAlgoInfo_SignalEncoder.uninitialize();

	for (uint32 o = 0; o < l_rStaticBoxContext.getOutputCount()-1; o++)
	{
		m_vAlgoX_SignalEncoder[o]->uninitialize();
		delete m_vAlgoX_SignalEncoder[o];
	}
	m_vAlgoX_SignalEncoder.clear();
	
	return true;
}


boolean CBoxAlgorithmDiscreteWaveletTransform::processInput(uint32 ui32InputIndex)
{
	// some pre-processing code if needed...

	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}


boolean CBoxAlgorithmDiscreteWaveletTransform::process(void)
{
	
	// the static box context describes the box inputs, outputs, settings structures
	// IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	const int J = std::atoi(m_sDecompositionLevel);
	const std::string nm (m_sWaveletType.toASCIIString());

	for(uint32 ii=0; ii<l_rDynamicBoxContext.getInputChunkCount(0); ii++)
	{
		//Decode input signal
		m_oAlgo0_SignalDecoder.decode(ii);

		// Construct header when we receive one
		if(m_oAlgo0_SignalDecoder.isHeaderReceived())
		{
			uint32 l_ui32NbChannels0 = m_oAlgo0_SignalDecoder.getOutputMatrix()->getDimensionSize(0);
			uint32 l_ui32NbSamples0 = m_oAlgo0_SignalDecoder.getOutputMatrix()->getDimensionSize(1);

			if (l_ui32NbSamples0<=std::pow(2.0,J+1))
			{
				this->getLogManager() << LogLevel_Error << "Number of samples [" << l_ui32NbSamples0 << "] is smaller or equal than 2^{J+1} == [" 
					<< std::pow(2.0,J+1) << "]\n";
				this->getLogManager() << LogLevel_Error << "Verify quantity of samples and number of decomposition levels" << "\n";
				this->getLogManager() << LogLevel_Error << "You can introduce a Time based epoching to have more samples per chunk or reduce the decomposition levels" << "\n";
				return false;
			}

			//sig will be resized to the number of channels and the total number of samples (Channels x Samples)
			m_sig.resize(l_ui32NbChannels0);
			for(uint32 i = 0; i < l_ui32NbChannels0; i++)
			{
				m_sig[i].resize(l_ui32NbSamples0);
			}

			//Do one dummy transform to get the m_flag and m_length filled. Since all channels & blocks have the same chunk size in OV, once is enough.
			std::vector<double> l_flag;          //flag is an auxiliar vector (see wavelet2d library)
			std::vector<int> l_length;           //length contains the length of each decomposition level. last entry is the length of the original signal.
			std::vector<double> l_dwt_output;    //dwt_output is the vector containing the decomposition levels

			dwt(m_sig[0],J,nm,l_dwt_output,l_flag,l_length);

			// Set info stream dimension
			m_ui32Infolength = (l_length.size()+l_flag.size()+2);
			m_oAlgoInfo_SignalEncoder.getInputMatrix()->setDimensionCount(2);
			m_oAlgoInfo_SignalEncoder.getInputMatrix()->setDimensionSize(0,l_ui32NbChannels0);
			m_oAlgoInfo_SignalEncoder.getInputMatrix()->setDimensionSize(1,m_ui32Infolength);

			// Set decomposition stream dimensions
			for (uint32 o = 0; o < m_vAlgoX_SignalEncoder.size(); o++)
			{
				m_vAlgoX_SignalEncoder[o]->getInputMatrix()->setDimensionCount(2);
				m_vAlgoX_SignalEncoder[o]->getInputMatrix()->setDimensionSize(0,l_ui32NbChannels0);
				m_vAlgoX_SignalEncoder[o]->getInputMatrix()->setDimensionSize(1,l_length[o]);

			}

			// Set decomposition stream channel names
			for (uint32 d_i=0; d_i<l_ui32NbChannels0; d_i++)
			{
				for (uint32 o = 0; o < m_vAlgoX_SignalEncoder.size(); o++)
				{
					m_vAlgoX_SignalEncoder[o]->getInputMatrix()->setDimensionLabel(0,d_i,m_oAlgo0_SignalDecoder.getOutputMatrix()->getDimensionLabel(0,d_i));
				}
			}


			// Info stream header
			m_oAlgoInfo_SignalEncoder.getInputSamplingRate().setReferenceTarget(m_oAlgo0_SignalDecoder.getOutputSamplingRate());
			m_oAlgoInfo_SignalEncoder.encodeHeader();
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));
		
			// Decomposition stream headers
			for (uint32 o = 0; o < m_vAlgoX_SignalEncoder.size(); o++)
			{
				const float64 l_f64SamplingRate = static_cast<float64>(m_oAlgo0_SignalDecoder.getOutputSamplingRate()) / std::pow(2.0,static_cast<int32>(o));
				m_vAlgoX_SignalEncoder[o]->getInputSamplingRate() = static_cast<uint64>(std::floor(l_f64SamplingRate));

				m_vAlgoX_SignalEncoder[o]->encodeHeader();
				l_rDynamicBoxContext.markOutputAsReadyToSend(o+1, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));
			}
		}

		if(m_oAlgo0_SignalDecoder.isBufferReceived())
		{
			const IMatrix* l_pMatrix_0 = m_oAlgo0_SignalDecoder.getOutputMatrix();
			const float64* l_pBuffer0 = l_pMatrix_0->getBuffer();

			const uint32 l_ui32NbChannels0 = l_pMatrix_0->getDimensionSize(0);
			const uint32 l_ui32NbSamples0 = l_pMatrix_0->getDimensionSize(1);

			//sig will store the samples of the different channels
			for(uint32 i=0; i<l_ui32NbChannels0; i++)    //Number of EEG channels
			{
				for(uint32 j=0; j<l_ui32NbSamples0; j++)    //Number of Samples per Chunk
				{
					m_sig[i][j]=(l_pBuffer0[j+i*l_ui32NbSamples0]);
				}
			}

			// Due to how wavelet2s works, we'll have to have the output variables empty before each call.
			std::vector< std::vector<double> > l_flag;
			std::vector< std::vector<int> > l_length;
			std::vector< std::vector<double> > l_dwt_output;
			l_flag.resize(l_ui32NbChannels0);
			l_length.resize(l_ui32NbChannels0);
			l_dwt_output.resize(l_ui32NbChannels0);

			//Calculation of wavelets coefficients for each channel.
			for(uint32 i=0; i<l_ui32NbChannels0; i++)
			{
				dwt(m_sig[i],J,nm,l_dwt_output[i],l_flag[i],l_length[i]);
			}

			//Transmission of some information (flag and legth) to the inverse dwt box
			//@fixme since the data dimensions do not change runtime, it should be sufficient to send this only once
			for(uint32 i=0; i<l_ui32NbChannels0; i++)
			{
				uint32 f=0;
				m_oAlgoInfo_SignalEncoder.getInputMatrix()->getBuffer()[f+i*m_ui32Infolength]=l_length[i].size();
				for (uint32 l=0;l<l_length[i].size();l++)
				{
					m_oAlgoInfo_SignalEncoder.getInputMatrix()->getBuffer()[l+1+i*m_ui32Infolength]=l_length[i][l];
					f=l;
				}
				m_oAlgoInfo_SignalEncoder.getInputMatrix()->getBuffer()[f+2+i*m_ui32Infolength]=l_flag[i].size();
				for (uint32 l=0;l<l_flag[i].size();l++)
				{
					m_oAlgoInfo_SignalEncoder.getInputMatrix()->getBuffer()[f+3+l+i*m_ui32Infolength]=l_flag[i][l];
				}
			}

			//Decode the dwt coefficients of each decomposition level to separate channels
			for(uint32 i=0; i<l_ui32NbChannels0; i++)
			{
				for (uint32 o = 0, l_ui32Vector_Position = 0; o < m_vAlgoX_SignalEncoder.size() ; o++)
				{  
					IMatrix *l_pOutMatrix = m_vAlgoX_SignalEncoder[o]->getInputMatrix();
					float64* l_pOutBuffer = l_pOutMatrix->getBuffer();

					// loop levels
					for (uint32 l=0; l<(uint32)l_length[i][o]; l++)
					{
						l_pOutBuffer[l+i*l_length[i][o]] = l_dwt_output[i][l+l_ui32Vector_Position];
					}

					l_ui32Vector_Position=l_ui32Vector_Position+l_length[i][o];
				}
			}
			
			m_oAlgoInfo_SignalEncoder.encodeBuffer();
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));

			for (uint32 o = 0; o < m_vAlgoX_SignalEncoder.size(); o++)
			{
				m_vAlgoX_SignalEncoder[o]->encodeBuffer();
				l_rDynamicBoxContext.markOutputAsReadyToSend(o+1, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));
			}
		}

		if(m_oAlgo0_SignalDecoder.isEndReceived())
		{
			m_oAlgoInfo_SignalEncoder.encodeEnd();
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));

			for (uint32 o = 0; o < m_vAlgoX_SignalEncoder.size(); o++)
			{
				m_vAlgoX_SignalEncoder[o]->encodeEnd();
				l_rDynamicBoxContext.markOutputAsReadyToSend(o+1, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));
			}
		}
	}

	return true;
}



#endif