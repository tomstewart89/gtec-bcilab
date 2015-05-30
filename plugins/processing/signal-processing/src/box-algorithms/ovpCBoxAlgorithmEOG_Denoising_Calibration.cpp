
#if defined(TARGET_HAS_ThirdPartyEIGEN)

#include "ovpCBoxAlgorithmEOG_Denoising_Calibration.h"

#include <Eigen/Dense>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

boolean CBoxAlgorithmEOG_Denoising_Calibration::initialize(void)
{
    // Signal stream decoder
    m_oAlgo0_SignalDecoder.initialize(*this,0);
    // Signal stream decoder
    m_oAlgo1_SignalDecoder.initialize(*this,1);

    m_oAlgo2_StimulationDecoder.initialize(*this,2);

    m_oStimulationEncoder.initialize(*this,0);

    m_sRegressionDenoisingCalibrationFilename=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
    m_f64StartTime=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
    m_f64EndTime=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
    m_ui64StimulationIdentifier=FSettingValueAutoCast(*this->getBoxAlgorithmContext(),3);

    m_ui32ChunksCount = 0;
    m_ui32ChunksVerify = -1;
    m_bEndProcess = false;
    m_f64Time=0;
    m_ui32StartTimeChunks=0;
    m_ui32EndTimeChunks=0;

	// Random id for tmp token, clash possible if multiple boxes run in parallel (but unlikely)
	const CString l_sRandomToken = CIdentifier::random().toString();
	m_sEEGTempFilename = this->getConfigurationManager().expand("${Path_Tmp}/denoising_") + l_sRandomToken + "_EEG_tmp.dat";
	m_sEOGTempFilename = this->getConfigurationManager().expand("${Path_Tmp}/denoising_") + l_sRandomToken + "_EOG_tmp.dat";

    m_oEEGFile.open (m_sEEGTempFilename, std::ios::out | std::ios::in |std::ios::trunc );
	if(m_oEEGFile.fail())
	{
		this->getLogManager() << LogLevel_Error << "Opening [" << m_sEEGTempFilename << "] for r/w failed\n";
		return false;
	}

    m_oEOGFile.open (m_sEOGTempFilename, std::ios::out | std::ios::in |std::ios::trunc );
	if(m_oEOGFile.fail())
	{
		this->getLogManager() << LogLevel_Error << "Opening [" << m_sEOGTempFilename << "] for r/w failed\n";
		return false;
	}

    m_oMatrixFile.open(m_sRegressionDenoisingCalibrationFilename.toASCIIString(),std::ios::out | std::ios::trunc);
	if(m_oMatrixFile.fail())
	{
		this->getLogManager() << LogLevel_Error << "Opening [" << m_sRegressionDenoisingCalibrationFilename << "] for writing failed\n";
		return false;
	}

    return true;
}


boolean CBoxAlgorithmEOG_Denoising_Calibration::uninitialize(void)
{
    m_oAlgo0_SignalDecoder.uninitialize();
    m_oAlgo1_SignalDecoder.uninitialize();
    m_oAlgo2_StimulationDecoder.uninitialize();
    m_oStimulationEncoder.uninitialize();

	// Clean up temporary files
	if(m_oEEGFile.is_open())
	{
		m_oEEGFile.close();
	}
	if(m_sEEGTempFilename!=CString("")) {
		std::remove(m_sEEGTempFilename);
	}

	if(m_oEOGFile.is_open())
	{
		m_oEOGFile.close();
	}
	if(m_sEOGTempFilename!=CString("")) {
		std::remove(m_sEOGTempFilename);
	}

    if(m_oMatrixFile.is_open())
	{
		m_oMatrixFile.close();
	}

    return true;
}



boolean CBoxAlgorithmEOG_Denoising_Calibration::processClock(IMessageClock& rMessageClock)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	if (m_ui32ChunksCount!=m_ui32ChunksVerify && m_bEndProcess == false)
	{
		m_ui32ChunksVerify = m_ui32ChunksCount;
		if (m_f64Time==m_f64StartTime)
		{
			m_ui32StartTimeChunks=m_ui32ChunksCount;
		}

		if (m_f64Time==m_f64EndTime)
		{
			m_ui32EndTimeChunks=m_ui32ChunksCount;
		}

	}
	else if (m_ui32ChunksCount==m_ui32ChunksVerify && m_bEndProcess == false)
	{
		if ( (m_f64StartTime>=m_f64EndTime) || (m_f64EndTime>=m_f64Time) )
		{
			this->getLogManager() << LogLevel_Warning << "Verify time interval of sampling" << "\n";
			this->getLogManager() << LogLevel_Warning << "Total time of your sample: " << m_f64Time << "\n";
			this->getLogManager() << LogLevel_Warning << "b Matrix was NOT successfully calculated" << "\n";

			m_oStimulationEncoder.getInputStimulationSet()->clear();
			m_oStimulationEncoder.getInputStimulationSet()->appendStimulation(OVTK_StimulationId_TrainCompleted, 0, 0);
			m_oStimulationEncoder.encodeBuffer();

			l_rDynamicBoxContext.markOutputAsReadyToSend(0,l_rDynamicBoxContext.getInputChunkStartTime(0, 0),l_rDynamicBoxContext.getInputChunkEndTime(0, 0));

			//this->getLogManager() << LogLevel_Warning << "You can stop this scenario " <<"\n";
			m_ui32ChunksVerify = -1;
			m_bEndProcess = true;
		}
		else
		{
			this->getLogManager() << LogLevel_Info << "End of data gathering...calculating b matrix" << "\n";
			
			m_oEEGFile.close();
			m_oEOGFile.close();

			m_oEEGFile.open (m_sEEGTempFilename, std::ios::in | std::ios::app);
			if(m_oEEGFile.fail())
			{
				this->getLogManager() << LogLevel_Error << "Opening [" << m_sEEGTempFilename << "] for reading failed\n";
				return false;
			}

			m_oEOGFile.open (m_sEOGTempFilename, std::ios::in | std::ios::app);
			if(m_oEOGFile.fail())
			{
				this->getLogManager() << LogLevel_Error << "Opening [" << m_sEOGTempFilename << "] for reading failed\n";
				return false;
			}

			//Process to extract the Matrix B
			float64 l_f64Aux;

			Eigen::MatrixXd l_oData0((int)m_ui32NbChannels0,(int)m_ui32NbSamples0*(int)m_ui32ChunksCount);   //EEG data
			Eigen::MatrixXd l_oData1((int)m_ui32NbChannels1,(int)m_ui32NbSamples1*(int)m_ui32ChunksCount);   //EOG data

			for (uint32 k=0; k<m_ui32ChunksCount;k++)
			{
				for(uint32 i=0; i<m_ui32NbChannels0; i++)    //Number of channels
				{
					for(uint32 j=0; j<m_ui32NbSamples0; j++)    //Number of Samples per Chunk
					{
						m_oEEGFile >> l_f64Aux;
						l_oData0(i,j+k*(int)m_ui32NbSamples0)=l_f64Aux;
					}
				}
			}

			for (uint32 k=0; k<m_ui32ChunksCount;k++)
			{
				for(uint32 i=0; i<m_ui32NbChannels1; i++)    //Number of channels
				{
					for(uint32 j=0; j<m_ui32NbSamples1; j++)    //Number of Samples per Chunk
					{
						m_oEOGFile >> l_f64Aux;
						l_oData1(i,j+k*(int)m_ui32NbSamples1)=l_f64Aux;
					}
				}
			}

			// We will eliminate the firsts and lasts chunks of each channel

			Eigen::MatrixXd l_oData0n((int)m_ui32NbChannels0,(int)m_ui32NbSamples0*((int)m_ui32ChunksCount));   //EEG data
			Eigen::MatrixXd l_oData1n((int)m_ui32NbChannels1,(int)m_ui32NbSamples1*((int)m_ui32ChunksCount));   //EOG data

			uint32 l_ui32ValidChunks = m_ui32EndTimeChunks - m_ui32StartTimeChunks;

			uint32 l_ui32iblockeeg=0;
			uint32 l_ui32jblockeeg=m_ui32StartTimeChunks*(int)m_ui32NbSamples0-1;
			uint32 l_ui32pblockeeg=(int)m_ui32NbChannels0;
			uint32 l_ui32qblockeeg=(int)m_ui32NbSamples0*(l_ui32ValidChunks);
			uint32 l_ui32iblockeog=0;
			uint32 l_ui32jblockeog=m_ui32StartTimeChunks*(int)m_ui32NbSamples1-1;
			uint32 l_ui32pblockeog=(int)m_ui32NbChannels1;
			uint32 l_ui32qblockeog=(int)m_ui32NbSamples1*(l_ui32ValidChunks);


			l_oData0n = l_oData0.block(l_ui32iblockeeg,l_ui32jblockeeg,l_ui32pblockeeg,l_ui32qblockeeg);
			l_oData1n = l_oData1.block(l_ui32iblockeog,l_ui32jblockeog,l_ui32pblockeog,l_ui32qblockeog);


			float64 l_f64nbval = 0;
			float64 l_f64min = 1e-6;
			float64 l_f64max = 1e6;

			Eigen::VectorXd mean_row_eeg(m_ui32NbChannels0);
			Eigen::VectorXd mean_row_eog(m_ui32NbChannels1);

			mean_row_eeg.setZero(m_ui32NbChannels0,1);
			mean_row_eog.setZero(m_ui32NbChannels1,1);


			for(uint32 i=0; i<m_ui32NbChannels0; i++)    //Number of channels
			{
				l_f64nbval = 0;
				for(uint32 j=0; j<m_ui32NbSamples0*l_ui32ValidChunks; j++)    //Number of Samples per Chunk
				{
					if ((l_oData0n(i,j)>-l_f64max && l_oData0n(i,j)<-l_f64min) || (l_oData0n(i,j)>l_f64min && l_oData0n(i,j)<l_f64max))

					{
						//Valid Interval
						mean_row_eeg(i) = mean_row_eeg(i) + l_oData0n(i,j);
						l_f64nbval=l_f64nbval+1;

					}
				}
				if (l_f64nbval!=0)
				{
					mean_row_eeg(i)=mean_row_eeg(i)/l_f64nbval;
				}
				else
				{
					mean_row_eeg(i)=0;
				}
			}



			for(uint32 i=0; i<m_ui32NbChannels1; i++)    //Number of channels
			{
				l_f64nbval = 0;
				for(uint32 j=0; j<m_ui32NbSamples1*l_ui32ValidChunks; j++)    //Number of Samples per Chunk
				{
					if ((l_oData1n(i,j)>-l_f64max && l_oData1n(i,j)<-l_f64min) || (l_oData1n(i,j)>l_f64min && l_oData1n(i,j)<l_f64max))
					{
						//Valid Interval
						mean_row_eog(i) = mean_row_eog(i) + l_oData1n(i,j);
						l_f64nbval=l_f64nbval+1;
					}

				}
				if (l_f64nbval!=0)
				{
					mean_row_eog(i)=mean_row_eog(i)/l_f64nbval;
				}
				else
				{
					mean_row_eog(i)=0;
				}

			}


			// The values which are not valid (very large or very small) will be set to the mean value
			// So these values will not influence to the covariance calcul because the covariance is centered (value - mean)


			for(uint32 i=0; i<m_ui32NbChannels0; i++)    //Number of channels
			{
				for(uint32 j=0; j<m_ui32NbSamples0*l_ui32ValidChunks; j++)   //Number of total samples
				{
					if ((l_oData0n(i,j)>-l_f64max && l_oData0n(i,j)<-l_f64min) || (l_oData0n(i,j)>l_f64min && l_oData0n(i,j)<l_f64max))
					{
						//Valid Interval
						l_oData0n(i,j)=l_oData0n(i,j)-mean_row_eeg(i);
					}
					else
					{
						//Invalid
						l_oData0n(i,j)=0;
					}
				}
			}



			for (uint32 i=0; i<m_ui32NbChannels1; i++)    //Number of channels
			{
				for (uint32 j=0; j<m_ui32NbSamples1*l_ui32ValidChunks; j++)    //Number of total samples
				{
					if ((l_oData1n(i,j)>-l_f64max && l_oData1n(i,j)<-l_f64min) || (l_oData1n(i,j)>l_f64min && l_oData1n(i,j)<l_f64max))
					{
						//Valid Interval
						l_oData1n(i,j)=l_oData1n(i,j)-mean_row_eog(i);
					}
					else
					{
						//Invalid
						l_oData1n(i,j)=0;
					}

				}

			}


			//Now we need to calculate the matrix b (which tells us the correct weights to be stored in b matrix)


			Eigen::MatrixXd l_oNoiseCoeff((int)m_ui32NbChannels0,(int)m_ui32NbChannels1);   //Noise Coefficients Matrix (Dim: Channels EEG x Channels EOG)

			Eigen::MatrixXd l_oCovEog((int)m_ui32NbChannels1,(int)m_ui32NbChannels1);

			Eigen::MatrixXd l_oCovEogInv((int)m_ui32NbChannels1,(int)m_ui32NbChannels1);

			Eigen::MatrixXd l_oCovEEGandEOG((int)m_ui32NbChannels0,(int)m_ui32NbChannels1);

			l_oCovEog = (l_oData1n * l_oData1n.transpose());

			l_oCovEogInv = l_oCovEog.inverse();

			l_oCovEEGandEOG = l_oData0n*(l_oData1n.transpose());

			l_oNoiseCoeff =  l_oCovEEGandEOG * l_oCovEogInv;


			// Save Matrix b to the file specified in the parameters


			m_oMatrixFile << m_ui32NbChannels0 << " " << m_ui32NbChannels1 << " " << m_ui32NbSamples0 <<"\n";

			for(uint32 i=0; i<m_ui32NbChannels0; i++)    //Number of channels EEG
			{
				for(uint32 j=0; j<m_ui32NbChannels1; j++)    //Number of channels EOG
				{
					m_oMatrixFile << l_oNoiseCoeff(i,j) <<"\n";
				}
			}


			m_oEEGFile.close();
			m_oEOGFile.close();
			m_oMatrixFile.close();

			m_ui32ChunksVerify = -1;
			m_bEndProcess = true;


			this->getLogManager() << LogLevel_Info << "b Matrix was successfully calculated" << "\n";
			this->getLogManager() << LogLevel_Info << "Wrote the matrix to [" << m_sRegressionDenoisingCalibrationFilename << "]\n";

			//this->getLogManager() << LogLevel_Warning << "You can stop this scenario " <<"\n";

			m_oStimulationEncoder.getInputStimulationSet()->clear();
			m_oStimulationEncoder.getInputStimulationSet()->appendStimulation(OVTK_StimulationId_TrainCompleted, 0, 0);
			m_oStimulationEncoder.encodeBuffer();

			l_rDynamicBoxContext.markOutputAsReadyToSend(0,l_rDynamicBoxContext.getInputChunkStartTime(0, 0),l_rDynamicBoxContext.getInputChunkEndTime(0, 0));

		}
	}


	m_f64Time++;
	return true;
}



uint64 CBoxAlgorithmEOG_Denoising_Calibration::getClockFrequency(void)
{
    // Note that the time is coded on a 64 bits unsigned integer, fixed decimal point (32:32)

    return 1LL<<32; // the box clock frequency
}



boolean CBoxAlgorithmEOG_Denoising_Calibration::processInput(uint32 ui32InputIndex)
{
    // some pre-processing code if needed...

    // ready to process !
    getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

    return true;
}





boolean CBoxAlgorithmEOG_Denoising_Calibration::process(void)
{

    // the static box context describes the box inputs, outputs, settings structures
    //IBox& l_rStaticBoxContext=this->getStaticBoxContext();
    // the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
    IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();


    if (m_bEndProcess)
    {
		// We have done our stuff and have sent out a stimuli that we're done. However, if we're called again, we just do nothing,
		// but do not return false (==error) as this state is normal after training.
        return true;
    }


    // Signal EEG
    for(uint32 ii=0; ii<l_rDynamicBoxContext.getInputChunkCount(0); ii++)
    {

        m_oAlgo0_SignalDecoder.decode(ii);

        m_ui32NbChannels0 = m_oAlgo0_SignalDecoder.getOutputMatrix()->getDimensionSize(0);
        m_ui32NbSamples0 = m_oAlgo0_SignalDecoder.getOutputMatrix()->getDimensionSize(1);

        //this->getLogManager() << LogLevel_Warning << "samples0" << m_ui32NbSamples0 << "\n";


        IMatrix* l_pMatrix_0 = m_oAlgo0_SignalDecoder.getOutputMatrix();


        float64* l_pBuffer0 = l_pMatrix_0->getBuffer();


        for (uint32 jj=0; jj<l_pMatrix_0->getBufferElementCount(); jj++)
        {
            m_oEEGFile << l_pBuffer0[jj] <<"\n";
        }

    }
    //Signal EOG
    for(uint32 ii=0; ii<l_rDynamicBoxContext.getInputChunkCount(1); ii++)
    {

        m_oAlgo1_SignalDecoder.decode(ii);

        m_ui32NbChannels1 = m_oAlgo1_SignalDecoder.getOutputMatrix()->getDimensionSize(0);
        m_ui32NbSamples1 = m_oAlgo1_SignalDecoder.getOutputMatrix()->getDimensionSize(1);

        IMatrix* l_pMatrix_1 = m_oAlgo1_SignalDecoder.getOutputMatrix();

        float64* l_pBuffer1 = l_pMatrix_1->getBuffer();

        for(uint32 jj=0; jj<l_pMatrix_1->getBufferElementCount(); jj++)
        {
            m_oEOGFile << l_pBuffer1[jj] <<"\n";
        }

        m_ui32ChunksCount++;

    }



    for (uint32 chunk = 0; chunk<l_rDynamicBoxContext.getInputChunkCount(2); chunk++)
    {
        m_oAlgo2_StimulationDecoder.decode(chunk);
        for(uint32 j=0; j<m_oAlgo2_StimulationDecoder.getOutputStimulationSet()->getStimulationCount(); j++)
        {
            if (m_oAlgo2_StimulationDecoder.getOutputStimulationSet()->getStimulationIdentifier(j) == 33025)
            {
                m_f64StartTime=m_f64Time;
                this->getLogManager() << LogLevel_Info << "Start time: " << m_f64StartTime <<"\n";
            }

            if (m_oAlgo2_StimulationDecoder.getOutputStimulationSet()->getStimulationIdentifier(j) == 33031)
            {
                m_f64EndTime=m_f64Time;
                this->getLogManager() << LogLevel_Info << "End time: " << m_f64EndTime <<"\n";

            }

            // m_ui64TrainDate = m_oAlgo2_StimulationDecoder.getOutputStimulationSet()->getStimulationDate(m_oAlgo2_StimulationDecoder.getOutputStimulationSet()->getStimulationCount());
            m_ui64TrainDate = m_oAlgo2_StimulationDecoder.getOutputStimulationSet()->getStimulationDate(j);


//            if(m_oAlgo2_StimulationDecoder.isHeaderReceived()) // ->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
//            {
//                m_oStimulationEncoder1.encodeHeader(0);
//                l_rDynamicBoxContext.markOutputAsReadyToSend(0,l_rDynamicBoxContext.getInputChunkStartTime(0, chunk),l_rDynamicBoxContext.getInputChunkEndTime(0, chunk));
//            }


//            //        m_ui64TrainChunkStartTime = l_rDynamicBoxContext.getInputChunkStartTime(0, chunk);
//            //        m_ui64TrainChunkEndTime = l_rDynamicBoxContext.getInputChunkEndTime(0, chunk);



//            if (m_oAlgo2_StimulationDecoder.isBufferReceived())
//            {
//                //m_oStimulationEncoder1.getInputStimulationSet()->appendStimulation(OVTK_StimulationId_TrainCompleted, 0, 0);
//                m_oStimulationEncoder1.encodeBuffer(0);
//                //l_rDynamicBoxContext.markOutputAsReadyToSend(0,m_ui64TrainChunkStartTime,m_ui64TrainChunkEndTime);
//                l_rDynamicBoxContext.markOutputAsReadyToSend(0,l_rDynamicBoxContext.getInputChunkStartTime(0, chunk),l_rDynamicBoxContext.getInputChunkEndTime(0, chunk));
//            }


//            if (m_oAlgo2_StimulationDecoder.isEndReceived())
//            {
//                m_oStimulationEncoder1.encodeEnd(0);
//                l_rDynamicBoxContext.markOutputAsReadyToSend(0,l_rDynamicBoxContext.getInputChunkStartTime(0, chunk),l_rDynamicBoxContext.getInputChunkEndTime(0, chunk));
//            }


        }
    }




    return true;
}

#endif