#if defined TARGET_HAS_ThirdPartyITPP

//#define __OpenViBEPlugins_BoxAlgorithm_IFFTbox_CPP__
// to get ifft:
#include <itpp/itsignal.h>
#include "ovpCBoxAlgorithmIFFTbox.h"


using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessingBasic;


boolean CBoxAlgorithmIFFTbox::initialize(void)
{
	// Spectrum stream real part decoder
	m_oAlgo0_SpectrumDecoder[0].initialize(*this,0);
	// Spectrum stream imaginary part decoder
	m_oAlgo0_SpectrumDecoder[1].initialize(*this,1);
	// Signal stream encoder
	m_oAlgo1_SignalEncoder.initialize(*this,0);
	
	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmIFFTbox::uninitialize(void)
{

	m_oAlgo0_SpectrumDecoder[0].uninitialize();
	m_oAlgo0_SpectrumDecoder[1].uninitialize();
	m_oAlgo1_SignalEncoder.uninitialize();
	
	return true;
}

boolean CBoxAlgorithmIFFTbox::processInput(uint32 ui32InputIndex)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IDynamicBoxContext& l_rDynamicBoxContext=this->getDynamicBoxContext();

	if(l_rDynamicBoxContext.getInputChunkCount(0) == 0)
	{
		return true;
	}
	uint64 l_ui64StartTime=l_rDynamicBoxContext.getInputChunkStartTime(0, 0);
	uint64 l_ui64EndTime=l_rDynamicBoxContext.getInputChunkEndTime(0, 0);
	for(uint32 i=1; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		if(l_rDynamicBoxContext.getInputChunkCount(i)==0)
		{
			return true;
		}

		boolean l_bValidDates=true;
		if(l_ui64StartTime!=l_rDynamicBoxContext.getInputChunkStartTime(i, 0)) { l_bValidDates=false; }
		if(l_ui64EndTime!=l_rDynamicBoxContext.getInputChunkEndTime(i, 0)) { l_bValidDates=false; }
		if(!l_bValidDates)
		{
			this->getLogManager() << LogLevel_Warning << "Chunk dates mismatch, check stream structure or parameters\n";
			return l_bValidDates;
		}
	}

	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmIFFTbox::process(void)
{
	
	// the static box context describes the box inputs, outputs, settings structures
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	
	uint32 l_ui32HeaderCount=0;
	uint32 l_ui32BufferCount=0;
	uint32 l_ui32EndCount=0;

	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		m_oAlgo0_SpectrumDecoder[i].decode(0);
		if(m_oAlgo0_SpectrumDecoder[i].isHeaderReceived())
		{
			//detect if header of other input is already received 
			if(0==l_ui32HeaderCount)
			{
				// Header received. This happens only once when pressing "play". For example with a StreamedMatrix input, you now know the dimension count, sizes, and labels of the matrix
				// ... maybe do some process ...
				m_channelsNumber=m_oAlgo0_SpectrumDecoder[i].getOutputMatrix()->getDimensionSize(0);
				m_ui32SampleCount = m_oAlgo0_SpectrumDecoder[i].getOutputMatrix()->getDimensionSize(1);
				if(m_channelsNumber == 0 || m_ui32SampleCount == 0)
				{
					this->getLogManager() << LogLevel_Error << "Both dims of the input matrix must have positive size\n";					
					return false;
				}
				m_ui32SampleCount = (m_ui32SampleCount-1)*2;
				if(m_ui32SampleCount == 0) 
				{
					m_ui32SampleCount = 1;
				}
			}
			else
			{
				if(!OpenViBEToolkit::Tools::Matrix::isDescriptionSimilar(*m_oAlgo0_SpectrumDecoder[0].getOutputMatrix(), *m_oAlgo0_SpectrumDecoder[i].getOutputMatrix(), false))
				{
					// problem!
					this->getLogManager() << LogLevel_Error << "The matrix components of the two streams have different properties, check stream structures or parameters\n";
					return false;
				}
				if(!OpenViBEToolkit::Tools::Matrix::isDescriptionSimilar(*m_oAlgo0_SpectrumDecoder[0].getOutputMinMaxFrequencyBands(), *m_oAlgo0_SpectrumDecoder[i].getOutputMinMaxFrequencyBands(), false))
				{
					// problem!
					this->getLogManager() << LogLevel_Error << "The band descriptors of the two streams have different properties, check stream structures or parameters\n";
					return false;
				}
				if(m_oAlgo0_SpectrumDecoder[0].getOutputMatrix()->getDimensionSize(1) != m_oAlgo0_SpectrumDecoder[i].getOutputMinMaxFrequencyBands()->getDimensionSize(1))
				{
					this->getLogManager() << LogLevel_Error << "Frequency band count " << m_oAlgo0_SpectrumDecoder[0].getOutputMatrix()->getDimensionSize(1) 
						<< " does not match the corresponding matrix chunk size " << m_oAlgo0_SpectrumDecoder[i].getOutputMinMaxFrequencyBands()->getDimensionSize(0) 
						<< ", check stream structures or parameters\n";
					return false;
				}
			}

			l_ui32HeaderCount++;
		}
		if(m_oAlgo0_SpectrumDecoder[i].isBufferReceived()) l_ui32BufferCount++;
		if(m_oAlgo0_SpectrumDecoder[i].isEndReceived()) l_ui32EndCount++;
	}
	
	if((l_ui32HeaderCount && l_ui32HeaderCount!=l_rStaticBoxContext.getInputCount())
	|| (l_ui32BufferCount && l_ui32BufferCount!=l_rStaticBoxContext.getInputCount())
	|| (l_ui32EndCount && l_ui32EndCount!=l_rStaticBoxContext.getInputCount()))
	{
		this->getLogManager() << LogLevel_Warning << "Stream structure mismatch\n";
		return false;
	}
	
	if(l_ui32HeaderCount)
	{
		m_frequencyBuffer.resize(m_channelsNumber);
		m_signalBuffer.resize(m_channelsNumber);
		for(uint32 channel=0; channel< m_channelsNumber; channel++)
		{
			m_signalBuffer[channel].set_size(m_ui32SampleCount);
			m_frequencyBuffer[channel].set_size(m_ui32SampleCount);
		}
		m_oAlgo1_SignalEncoder.getInputSamplingRate()=m_ui32SampleCount;
		m_oAlgo1_SignalEncoder.getInputMatrix()->setDimensionCount(2);
		m_oAlgo1_SignalEncoder.getInputMatrix()->setDimensionSize(0,m_channelsNumber);
		m_oAlgo1_SignalEncoder.getInputMatrix()->setDimensionSize(1,m_ui32SampleCount);
		for(uint32 channel=0; channel< m_channelsNumber; channel++)
		{
			m_oAlgo1_SignalEncoder.getInputMatrix()->setDimensionLabel(0, channel, 
													m_oAlgo0_SpectrumDecoder[0].getOutputMatrix()->getDimensionLabel(0, channel));
		}

		// Pass the header to the next boxes, by encoding a header on the output 0:
		m_oAlgo1_SignalEncoder.encodeHeader();
		// send the output chunk containing the header. The dates are the same as the input chunk:
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, 0), l_rDynamicBoxContext.getInputChunkEndTime(0, 0));
																					
	}
	if(l_ui32BufferCount)
	{
			const float64* bufferInput0= m_oAlgo0_SpectrumDecoder[0].getOutputMatrix()->getBuffer();
			const float64* bufferInput1= m_oAlgo0_SpectrumDecoder[1].getOutputMatrix()->getBuffer();

			const uint32 l_ui32HalfSize = m_oAlgo0_SpectrumDecoder[0].getOutputMatrix()->getDimensionSize(1);
			for(uint32 channel=0; channel< m_channelsNumber; channel++)
			{
				m_frequencyBuffer[channel][0].real(bufferInput0[channel*l_ui32HalfSize+0]);
				m_frequencyBuffer[channel][0].imag(bufferInput1[channel*l_ui32HalfSize+0]);

				for(uint32 j=1; j< l_ui32HalfSize; j++)
				{
					m_frequencyBuffer[channel][j].real(bufferInput0[channel*l_ui32HalfSize+j]);
					m_frequencyBuffer[channel][j].imag(bufferInput1[channel*l_ui32HalfSize+j]);

					m_frequencyBuffer[channel][m_ui32SampleCount-j].real(bufferInput0[channel*l_ui32HalfSize+j]);
					m_frequencyBuffer[channel][m_ui32SampleCount-j].imag(bufferInput1[channel*l_ui32HalfSize+j]);
				}

				m_signalBuffer[channel]= itpp::ifft_real(m_frequencyBuffer[channel]);

				// Test block
				// std::cout << "Iy: " << m_frequencyBuffer[channel].size() << ", y=" << m_frequencyBuffer[channel] << "\n";
				// std::cout << "Ix: " << m_signalBuffer[channel].size() << ", x'=" << m_signalBuffer[channel] << "\n";

				float64* bufferOutput=m_oAlgo1_SignalEncoder.getInputMatrix()->getBuffer();
				for(uint32 j=0; j< m_ui32SampleCount; j++)
				{
					bufferOutput[channel*m_ui32SampleCount+j]= m_signalBuffer[channel][j];
				}
			}
			// Encode the output buffer :
			m_oAlgo1_SignalEncoder.encodeBuffer();
			// and send it to the next boxes :
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, 0), 	l_rDynamicBoxContext.getInputChunkEndTime(0, 0));
		
	}																										
	if(l_ui32EndCount)
	{
				// End of stream received. This happens only once when pressing "stop". Just pass it to the next boxes so they receive the message :
			m_oAlgo1_SignalEncoder.encodeEnd();
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, 0), l_rDynamicBoxContext.getInputChunkEndTime(0, 0));
	}																								

	return true;
}

#endif //TARGET_HAS_ThirdPartyITPP
