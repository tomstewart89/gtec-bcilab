
#if defined(TARGET_HAS_ThirdPartyFFTW3) // required by wavelet2s

#include "ovpCBoxAlgorithmInverse_DWT.h"

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

boolean CBoxAlgorithmInverse_DWT::initialize(void)
{

	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	m_AlgoInfo_SignalDecoder.initialize(*this,0);

	m_sWaveletType=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_sDecompositionLevel=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

	m_oAlgoX_SignalDecoder = new OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmInverse_DWT > [l_rStaticBoxContext.getInputCount()];

	for (uint32 o = 0; o < l_rStaticBoxContext.getInputCount()-1; o++)
	{
		m_oAlgoX_SignalDecoder[o].initialize(*this,o+1);
	}

	m_oAlgo0_SignalEncoder.initialize(*this,0);
	return true;
}

boolean CBoxAlgorithmInverse_DWT::uninitialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	m_AlgoInfo_SignalDecoder.uninitialize();



	for (uint32 o = 0; o < l_rStaticBoxContext.getInputCount()-1; o++)
	{
		m_oAlgoX_SignalDecoder[o].uninitialize();
	}

	if(m_oAlgoX_SignalDecoder)
	{
		delete[] m_oAlgoX_SignalDecoder;
		m_oAlgoX_SignalDecoder = NULL;
	}

	m_oAlgo0_SignalEncoder.uninitialize();

	return true;
}



boolean CBoxAlgorithmInverse_DWT::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

boolean CBoxAlgorithmInverse_DWT::process(void)
{

	// the static box context describes the box inputs, outputs, settings structures
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();


//    uint32 J = std::atoi(m_sDecompositionLevel);
	std::string nm (m_sWaveletType.toASCIIString());
	std::vector<std::vector<double> > dwtop;
	std::vector<std::vector<double> > idwt_output;
	std::vector<std::vector<double> > flag;
	std::vector<std::vector<int> > length;

	uint32 l_flagreceveid=0;


	std::vector<uint32> l_vNbChannels0 (l_rStaticBoxContext.getInputCount());
	std::vector<uint32> l_vNbSamples0 (l_rStaticBoxContext.getInputCount());

//Check if all inputs have some information to decode
	if (l_rDynamicBoxContext.getInputChunkCount(l_rStaticBoxContext.getInputCount()-1)==1)
	{
		//Decode the first input (Informations)
		for(uint32 ii=0; ii<l_rDynamicBoxContext.getInputChunkCount(0); ii++)
		{

			m_AlgoInfo_SignalDecoder.decode(ii);

			l_vNbChannels0[0] = m_AlgoInfo_SignalDecoder.getOutputMatrix()->getDimensionSize(0);
			l_vNbSamples0[0] = m_AlgoInfo_SignalDecoder.getOutputMatrix()->getDimensionSize(1);


			IMatrix* l_pMatrix_0 = m_AlgoInfo_SignalDecoder.getOutputMatrix();
			float64* l_pBuffer0 = l_pMatrix_0->getBuffer();

			//this->getLogManager() << LogLevel_Warning << "buffer 0 " << (uint32)l_pBuffer0[0] << "\n";

			length.resize(l_vNbChannels0[0]);
			flag.resize(l_vNbChannels0[0]);

			for(uint32 i=0; i<l_vNbChannels0[0]; i++)
			{
				uint32 f=0;

				for (uint32 l=0;l<(uint32)l_pBuffer0[0];l++)
				{
					length[i].push_back((uint32)l_pBuffer0[l+1]);
					f=l;
				}

				for (uint32 l=0;l<(uint32)l_pBuffer0[f+2];l++)
				{
					flag[i].push_back((uint32)l_pBuffer0[f+3+l]);
				}

			}
			l_flagreceveid=1;
		}

//If Informations is decoded
		if (l_flagreceveid==1)
		{
			//Decode each decomposition level
			for (uint32 o = 0; o < l_rStaticBoxContext.getInputCount()-1; o++)
			{
				//Decode data of channels
				for(uint32 ii=0; ii<l_rDynamicBoxContext.getInputChunkCount(o+1); ii++)
				{
					//   this->getLogManager() << LogLevel_Warning << "getinputchunkcount "<< o <<": "<< l_rDynamicBoxContext.getInputChunkCount(o+1)  << "\n";


					m_oAlgoX_SignalDecoder[o].decode(ii);


					l_vNbChannels0[o+1] = m_oAlgoX_SignalDecoder[o].getOutputMatrix()->getDimensionSize(0);
					// this->getLogManager() << LogLevel_Warning << "channels: "  << l_vNbChannels0[o+1] << "\n";

					l_vNbSamples0[o+1] = m_oAlgoX_SignalDecoder[o].getOutputMatrix()->getDimensionSize(1);

					IMatrix* l_pMatrix = m_oAlgoX_SignalDecoder[o].getOutputMatrix();
					float64* l_pBuffer = l_pMatrix->getBuffer();

					//dwtop is the dwt coefficients
					dwtop.resize(l_vNbChannels0[0]);

					//Store input data (dwt coefficients) in just one vector (dwtop)
					for(uint32 i=0; i<l_vNbChannels0[0]; i++)
					{
						for (uint32 jj=0; jj<l_vNbSamples0[o+1]; jj++)
						{
							dwtop[i].push_back(l_pBuffer[jj+i*l_vNbSamples0[o+1]]);
						}

					}

					//Check if received informations about dwt box are coherent with inverse dwt box settings
					if ((uint32)(length[0].size())>0 && o==l_rStaticBoxContext.getInputCount()-2)
					{
						//Check if quantity of samples received are the same
						if ((uint32)(length[0].at(l_rStaticBoxContext.getInputCount()-1))==(uint32)dwtop[0].size())
						{

							//Resize idwt vector
							idwt_output.resize(l_vNbChannels0[0]);

							//Calculate idwt for each channel
							for(uint32 i=0; i<l_vNbChannels0[0]; i++)
							{
								idwt(dwtop[i],flag[i],nm,idwt_output[i],length[i]);
							}


							m_oAlgo0_SignalEncoder.getInputSamplingRate() = 2*m_oAlgoX_SignalDecoder[o].getOutputSamplingRate();

							m_oAlgo0_SignalEncoder.getInputMatrix()->setDimensionCount(2);

							m_oAlgo0_SignalEncoder.getInputMatrix()->setDimensionSize(0,l_vNbChannels0[0]);
							m_oAlgo0_SignalEncoder.getInputMatrix()->setDimensionSize(1,length[0].at(l_rStaticBoxContext.getInputCount()-1));



							for (uint32 d_i=0; d_i<l_vNbChannels0[0]; d_i++)
							{
								m_oAlgo0_SignalEncoder.getInputMatrix()->setDimensionLabel(0,d_i,m_oAlgoX_SignalDecoder[o].getOutputMatrix()->getDimensionLabel(0,d_i));
							}



							//Encode resultant signal to output
							for(uint32 i=0; i<l_vNbChannels0[0]; i++)
							{
								for (uint32 k=0;k<(uint32)idwt_output[i].size();k++)
								{
									m_oAlgo0_SignalEncoder.getInputMatrix()->getBuffer()[k+i*(uint32)idwt_output[i].size()]= idwt_output[i][k];
								}
							}


							//                         if(m_oAlgoX_SignalDecoder[o].isHeaderReceived())
							//                         {
							m_oAlgo0_SignalEncoder.encodeHeader();
							//                             // send the output chunk containing the header. The dates are the same as the input chunk:
							l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));
							//                         }
							//                         if(m_oAlgoX_SignalDecoder[o].isBufferReceived())
							//                         {
							//                           this->getLogManager() << LogLevel_Warning << "Dentro buffer received "   << "\n";
							m_oAlgo0_SignalEncoder.encodeBuffer();
							// and send it to the next boxes :
							l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));

							//                         }
							//                         if(m_oAlgoX_SignalDecoder[o].isEndReceived())
							//                         {
							//                            this->getLogManager() << LogLevel_Warning << "Dentro end received "   << "\n";
							// End of stream received. This happens only once when pressing "stop". Just pass it to the next boxes so they receive the message :
							m_oAlgo0_SignalEncoder.encodeEnd();
							l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));
							//                         }
						}
					}
				}
			}
		}
	}


	return true;
}

#endif