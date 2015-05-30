
#if defined(TARGET_HAS_ThirdPartyEIGEN)

#include "ovpCBoxAlgorithmEOG_Denoising.h"

#include <string>


using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

boolean CBoxAlgorithmEOG_Denoising::initialize(void)
{
    // Signal stream decoder
    m_oAlgo0_SignalDecoder.initialize(*this,0);
    m_oAlgo1_SignalDecoder.initialize(*this,1);

    m_oAlgo2_SignalEncoder.getInputSamplingRate().setReferenceTarget(m_oAlgo0_SignalDecoder.getOutputSamplingRate());

    m_sFilename=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);

    m_fBMatrixFile.open(m_sFilename.toASCIIString(),std::ios::in);
	if(m_fBMatrixFile.fail())
	{
		this->getLogManager() << LogLevel_Error << "Failed to open [" << m_sFilename << "] for reading\n";
		return false;
	}

    m_fBMatrixFile >> m_ui32NbChannels0;
    m_fBMatrixFile >> m_ui32NbChannels1;
    m_fBMatrixFile >> m_ui32NbSamples0;

	if(m_fBMatrixFile.fail())
	{
		this->getLogManager() << LogLevel_Error << "Not able to successfully read dims from [" << m_sFilename << "]\n";
		m_fBMatrixFile.close();
		return false;
	}


    m_ui32NbSamples1 = m_ui32NbSamples0;

    l_oNoiseCoeff.resize(m_ui32NbChannels0,m_ui32NbChannels1);   //Noise Coefficients Matrix (Dim: Channels EEG x Channels EOG)
    l_oNoiseCoeff.setZero(m_ui32NbChannels0,m_ui32NbChannels1);

    for(uint32 i=0; i<m_ui32NbChannels0; i++)    //Number of channels
    {
        for(uint32 j=0; j<m_ui32NbChannels1; j++)    //Number of Samples per Chunk
        {
            m_fBMatrixFile >> l_oNoiseCoeff(i,j);
        }
    }

	if(m_fBMatrixFile.fail())
	{
		this->getLogManager() << LogLevel_Error << "Not able to successfully read coefficients from [" << m_sFilename << "]\n";
		m_fBMatrixFile.close();
		return false;
	}

    m_fBMatrixFile.close();

    // Signal stream encoder
    m_oAlgo2_SignalEncoder.initialize(*this,0);

    return true;
}


boolean CBoxAlgorithmEOG_Denoising::uninitialize(void)
{
    m_oAlgo0_SignalDecoder.uninitialize();
    m_oAlgo1_SignalDecoder.uninitialize();
    m_oAlgo2_SignalEncoder.uninitialize();


    return true;
}



boolean CBoxAlgorithmEOG_Denoising::processInput(uint32 ui32InputIndex)
{
    // some pre-processing code if needed...

    // ready to process !
    getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

    return true;
}


boolean CBoxAlgorithmEOG_Denoising::process(void)
{

    // the static box context describes the box inputs, outputs, settings structures
    //IBox& l_rStaticBoxContext=this->getStaticBoxContext();
    // the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
    IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();


    Eigen::MatrixXd l_oData0(m_ui32NbChannels0,m_ui32NbSamples0);   //EEG data
    Eigen::MatrixXd l_oData1(m_ui32NbChannels1,m_ui32NbSamples1);   //EOG data
    Eigen::MatrixXd l_oEEGc(m_ui32NbChannels0,m_ui32NbSamples0);   //Corrected Matrix


    if (l_rDynamicBoxContext.getInputChunkCount(0)!=0)
    {
        for(uint32 ii=0; ii<l_rDynamicBoxContext.getInputChunkCount(1); ii++)       //Don't know why getinputchunkcount(1)
        {
            // Signal EEG

            // decode the chunk ii on input 0
            m_oAlgo0_SignalDecoder.decode(ii);

            IMatrix* l_pMatrix_0 = m_oAlgo0_SignalDecoder.getOutputMatrix();
            float64* l_pBuffer0 = l_pMatrix_0->getBuffer();

            for(uint32 i=0; i<m_ui32NbChannels0; i++)    //Number of channels
            {
                for(uint32 j=0; j<m_ui32NbSamples0; j++)    //Number of Samples per Chunk
                {
                    l_oData0(i,j) = l_pBuffer0[j+i*m_ui32NbSamples0];
                }
            }

            //Signal EOG
            m_oAlgo1_SignalDecoder.decode(ii);

            IMatrix* l_pMatrix_1 = m_oAlgo1_SignalDecoder.getOutputMatrix();
            float64* l_pBuffer1 = l_pMatrix_1->getBuffer();

            for(uint32 i=0; i<m_ui32NbChannels1; i++)    //Number of channels
            {
                for(uint32 j=0; j<m_ui32NbSamples1; j++)    //Number of Samples per Chunk
                {
                    l_oData1(i,j) = l_pBuffer1[j+i*m_ui32NbSamples1];
                }
            }


            //Set the output (corrected EEG) to the same structure as the EEG input
            m_oAlgo2_SignalEncoder.getInputMatrix()->setDimensionCount(2);
            m_oAlgo2_SignalEncoder.getInputMatrix()->setDimensionSize(0,m_ui32NbChannels0);
            m_oAlgo2_SignalEncoder.getInputMatrix()->setDimensionSize(1,m_ui32NbSamples0);


            for (uint32 d_i=0; d_i<m_ui32NbChannels0; d_i++)
            {
                m_oAlgo2_SignalEncoder.getInputMatrix()->setDimensionLabel(0,d_i,m_oAlgo0_SignalDecoder.getOutputMatrix()->getDimensionLabel(0,d_i));
            }


            //Remove the noise
            l_oEEGc = l_oData0 - (l_oNoiseCoeff*l_oData1);


            for(uint32 i=0; i<m_ui32NbChannels0; i++)    //Number of EEG channels
            {
                for(uint32 j=0; j<m_ui32NbSamples0; j++)    //Number of Samples per Chunk
                {
                    m_oAlgo2_SignalEncoder.getInputMatrix()->getBuffer()[j+i*m_ui32NbSamples0] = l_oEEGc(i,j);
                }
            }


            m_oAlgo2_SignalEncoder.getInputSamplingRate().setReferenceTarget(m_oAlgo1_SignalDecoder.getOutputSamplingRate());


            if(m_oAlgo1_SignalDecoder.isHeaderReceived())
            {

                m_oAlgo2_SignalEncoder.encodeHeader();
                // send the output chunk containing the header. The dates are the same as the input chunk:
                l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));
            }

            if(m_oAlgo1_SignalDecoder.isBufferReceived())
            {
                // Encode the output buffer :
                m_oAlgo2_SignalEncoder.encodeBuffer();
                // and send it to the next boxes :
                l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));
            }
            if(m_oAlgo1_SignalDecoder.isEndReceived())
            {
                // End of stream received. This happens only once when pressing "stop". Just pass it to the next boxes so they receive the message :
                m_oAlgo2_SignalEncoder.encodeEnd();
                l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, ii), l_rDynamicBoxContext.getInputChunkEndTime(0, ii));
            }


        }
    }
    return true;
}

#endif