#if defined TARGET_HAS_ThirdPartyITPP

#include "ovpCBoxAlgorithmCSPSpatialFilterTrainer.h"

#include <system/ovCMemory.h>

#include <complex>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>
#include <math.h>
#include <iostream>

#include <itpp/base/algebra/eigen.h>
#include <itpp/base/algebra/inv.h>
#include <itpp/base/converters.h>
#include <itpp/stat/misc_stat.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

using namespace itpp;

// Taken from http://techlogbook.wordpress.com/2009/08/12/adding-generalized-eigenvalue-functions-to-it
//            http://techlogbook.wordpress.com/2009/08/12/calling-lapack-functions-from-c-codes
//            http://sourceforge.net/projects/itpp/forums/forum/115656/topic/3363490?message=7557038
//
// http://icl.cs.utk.edu/projectsfiles/f2j/javadoc/org/netlib/lapack/DSYGV.html
// http://www.lassp.cornell.edu/sethna/GeneDynamics/NetworkCodeDocumentation/lapack_8h.html#a17

namespace
{
	extern "C"
	{
		// This symbol comes from LAPACK
/*
		void zggev_(char *jobvl, char *jobvr, int *n, std::complex<double> *a,
			int *lda, std::complex<double> *b, int *ldb, std::complex<double> *alpha,
			std::complex<double> *beta, std::complex<double> *vl,
			int *ldvl, std::complex<double> *vr, int *ldvr,
			std::complex<double> *work, int *lwork, double *rwork, int *info);
*/
		int dsygv_(int* itype, char* jobz, char* uplo, int* n, double* a,
			int* lda, double* b, int* ldb, double* w, double* work, int* lwork, int* info);
	}
};

namespace itppextcsp
{
	itpp::mat convert(const IMatrix& rMatrix)
	{
		itpp::mat l_oResult(
			rMatrix.getDimensionSize(1),
			rMatrix.getDimensionSize(0));
		System::Memory::copy(l_oResult._data(), rMatrix.getBuffer(), rMatrix.getBufferElementCount()*sizeof(float64));
		return l_oResult.transpose();
	}

	itpp::mat cov(const itpp::mat inputMat)
	{
		itpp::mat l_oResult(inputMat.rows(),inputMat.rows());
		itpp::mat centeredMat(inputMat.rows(),inputMat.rows());
		centeredMat = itpp::repmat(itpp::sum(inputMat,2), 1, inputMat.cols(), false);
		centeredMat = centeredMat / ((double)inputMat.cols());
		centeredMat = inputMat-centeredMat;
		l_oResult = centeredMat*centeredMat.transpose();
		l_oResult = l_oResult / ((double)(inputMat.cols()-1));
		l_oResult = l_oResult / itpp::trace(l_oResult);
		return l_oResult;
	}
}

boolean CBoxAlgorithmCSPSpatialFilterTrainer::initialize(void)
{
	m_pStimulationDecoder=new OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmCSPSpatialFilterTrainer >();
	m_pStimulationDecoder->initialize(*this,0);

	m_pSignalDecoderCondition1=new OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmCSPSpatialFilterTrainer >();
	m_pSignalDecoderCondition1->initialize(*this,1);

	m_pSignalDecoderCondition2=new OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmCSPSpatialFilterTrainer >();
	m_pSignalDecoderCondition2->initialize(*this,2);

	m_oStimulationEncoder.initialize(*this,0);

	m_ui64StimulationIdentifier=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_sSpatialFilterConfigurationFilename=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_ui64FilterDimension=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);

	return true;
}

boolean CBoxAlgorithmCSPSpatialFilterTrainer::uninitialize(void)
{
	m_pSignalDecoderCondition1->uninitialize();
	delete m_pSignalDecoderCondition1;
	m_pSignalDecoderCondition2->uninitialize();
	delete m_pSignalDecoderCondition2;
	m_pStimulationDecoder->uninitialize();
	delete m_pStimulationDecoder;

	m_oStimulationEncoder.uninitialize();


	return true;
}

boolean CBoxAlgorithmCSPSpatialFilterTrainer::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CBoxAlgorithmCSPSpatialFilterTrainer::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	boolean l_bShouldTrain=false;
	uint64 l_ui64TrainDate=0, l_ui64TrainChunkStartTime=0, l_ui64TrainChunkEndTime=0;

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		m_pStimulationDecoder->decode(i);
		if(m_pStimulationDecoder->isHeaderReceived())
		{
			m_oStimulationEncoder.encodeHeader();
			l_rDynamicBoxContext.markOutputAsReadyToSend(0,l_rDynamicBoxContext.getInputChunkStartTime(0, i),l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}
		if(m_pStimulationDecoder->isBufferReceived())
		{
			TParameterHandler < IStimulationSet* > op_pStimulationSet(m_pStimulationDecoder->getOutputStimulationSet());
			for(uint32 j=0; j<op_pStimulationSet->getStimulationCount(); j++)
			{
				l_bShouldTrain |= (op_pStimulationSet->getStimulationIdentifier(j)==m_ui64StimulationIdentifier);
			}
			if(l_bShouldTrain)
			{
				l_ui64TrainDate = op_pStimulationSet->getStimulationDate(op_pStimulationSet->getStimulationCount()-1);
				l_ui64TrainChunkStartTime = l_rDynamicBoxContext.getInputChunkStartTime(0, i);
				l_ui64TrainChunkEndTime = l_rDynamicBoxContext.getInputChunkEndTime(0, i);
			}
		}
		if(m_pStimulationDecoder->isEndReceived())
		{
			m_oStimulationEncoder.encodeEnd();
		}
		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}

	if(l_bShouldTrain)
	{
		this->getLogManager() << LogLevel_Info << "Received train stimulation - be patient\n";

		this->getLogManager() << LogLevel_Trace << "Estimating cov for condition 1...\n";

		itpp::mat l_oCovarianceMatrixCondition1;
		int l_iNumberOfCondition1Trials = 0;
		int l_iCondition1ChunkSize = 0;
		int l_iCondition2ChunkSize = 0;
		for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(1); i++)
		{
			m_pSignalDecoderCondition1->decode(i);
			if(m_pSignalDecoderCondition1->isHeaderReceived())
			{
				TParameterHandler<IMatrix*> ip_pMatrix(m_pSignalDecoderCondition1->getOutputMatrix());
				l_oCovarianceMatrixCondition1.set_size(ip_pMatrix->getDimensionSize(0),ip_pMatrix->getDimensionSize(0));
				l_oCovarianceMatrixCondition1.zeros();

				l_iCondition1ChunkSize = ip_pMatrix->getDimensionSize(1);
				this->getLogManager() << LogLevel_Debug << "Cov matrix size for condition 1 is [" 
					<< ip_pMatrix->getDimensionSize(0) << "x" << ip_pMatrix->getDimensionSize(0) 
					<< "], chunk size is " << l_iCondition1ChunkSize << " samples\n";
			}
			if(m_pSignalDecoderCondition1->isBufferReceived())
			{
				TParameterHandler<IMatrix*> ip_pMatrix(m_pSignalDecoderCondition1->getOutputMatrix());
				itpp::mat l_oMatrix=itppextcsp::convert(*ip_pMatrix);
				l_oCovarianceMatrixCondition1 += itppextcsp::cov(l_oMatrix);
				l_iNumberOfCondition1Trials++;
			}
			if(m_pSignalDecoderCondition1->isEndReceived())
			{
			}
			l_rDynamicBoxContext.markInputAsDeprecated(1, i);
		}
		l_oCovarianceMatrixCondition1 = l_oCovarianceMatrixCondition1/ ((double)l_iNumberOfCondition1Trials);
		this->getLogManager() << LogLevel_Trace << "Number of chunks for condition 1: " << l_iNumberOfCondition1Trials << "\n";

		this->getLogManager() << LogLevel_Trace << "Estimating cov for condition 2...\n";

		itpp::mat l_oCovarianceMatrixCondition2;
		int l_iNumberOfCondition2Trials = 0;
		for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(2); i++)
		{
			m_pSignalDecoderCondition2->decode(i);
			if(m_pSignalDecoderCondition2->isHeaderReceived())
			{
				TParameterHandler<IMatrix*> ip_pMatrix(m_pSignalDecoderCondition2->getOutputMatrix());
				l_oCovarianceMatrixCondition2.set_size(ip_pMatrix->getDimensionSize(0),ip_pMatrix->getDimensionSize(0));
				l_oCovarianceMatrixCondition2.zeros();

				l_iCondition2ChunkSize = ip_pMatrix->getDimensionSize(1);
				this->getLogManager() << LogLevel_Debug << "Cov matrix size for condition 2 is [" 
					<< ip_pMatrix->getDimensionSize(0) << "x" << ip_pMatrix->getDimensionSize(0) 
					<< "], chunk size is " << l_iCondition2ChunkSize << " samples\n";
			}
			if(m_pSignalDecoderCondition2->isBufferReceived())
			{
				TParameterHandler<IMatrix*> ip_pMatrix(m_pSignalDecoderCondition2->getOutputMatrix());
				itpp::mat l_oMatrix=itppextcsp::convert(*ip_pMatrix);
				l_oCovarianceMatrixCondition2 += itppextcsp::cov(l_oMatrix);
				l_iNumberOfCondition2Trials++;
			}
			if(m_pSignalDecoderCondition2->isEndReceived())
			{
			}
			l_rDynamicBoxContext.markInputAsDeprecated(2, i);
		}
		l_oCovarianceMatrixCondition2 = l_oCovarianceMatrixCondition2/ ((double)l_iNumberOfCondition2Trials);

		if(l_oCovarianceMatrixCondition1.cols() != l_oCovarianceMatrixCondition2.cols())
		{
			this->getLogManager() << LogLevel_Error << "The two inputs do not seem to have the same number of channels, "
				<< l_oCovarianceMatrixCondition1.cols() << " vs " << l_oCovarianceMatrixCondition2.cols() << "\n";
			return false;
		}

		this->getLogManager() << LogLevel_Info << "Data covariance dims are [" << l_oCovarianceMatrixCondition1.rows() << "x"
			<< l_oCovarianceMatrixCondition1.cols() 
			<< "]. Number of samples per condition : \n";
		this->getLogManager() << LogLevel_Info << "  cond1 = " 
			<< l_iNumberOfCondition1Trials << " chunks, sized " << l_iCondition1ChunkSize << " -> " << l_iNumberOfCondition1Trials*l_iCondition1ChunkSize << " samples\n";
		this->getLogManager() << LogLevel_Info << "  cond2 = " 
			<< l_iNumberOfCondition2Trials << " chunks, sized " << l_iCondition2ChunkSize << " -> " << l_iNumberOfCondition2Trials*l_iCondition2ChunkSize << " samples\n";

		if(l_iNumberOfCondition1Trials==0 || l_iNumberOfCondition2Trials==0)
		{
			this->getLogManager() << LogLevel_Error << "No signal received... Can't continue\n";
			return true;
		}

		this->getLogManager() << LogLevel_Trace << "Computing eigen vector decomposition...\n";

		itpp::cmat l_oEigenVector;
		itpp::cvec l_oEigenValue;
		uint32 l_ui32ChannelCount= l_oCovarianceMatrixCondition1.rows();
		
		if(itpp::eig(itpp::inv(l_oCovarianceMatrixCondition2)*l_oCovarianceMatrixCondition1, l_oEigenValue, l_oEigenVector))
		{
			std::map < double, itpp::vec > l_vEigenVector;
			for(uint32 i=0; i<l_ui32ChannelCount; i++)
			{
				itpp::cvec l_oVector=l_oEigenVector.get_col(i);
				l_vEigenVector[itpp::real(l_oEigenValue)[i]]=itpp::real(l_oVector);
			}

			FILE* l_pFile=::fopen(m_sSpatialFilterConfigurationFilename.toASCIIString(), "wb");
			if(!l_pFile)
			{
				this->getLogManager() << LogLevel_Error << "The file [" << m_sSpatialFilterConfigurationFilename << "] could not be opened for writing...\n";
				return false;
			}


			std::map < double, itpp::vec >::const_iterator it_forward;
			::fprintf(l_pFile, "<OpenViBE-SettingsOverride>\n");
			::fprintf(l_pFile, "\t<SettingValue>");
			this->getLogManager() << LogLevel_Debug << "lowest eigenvalues: " << "\n";
			uint32 l_u32Steps;
			for(it_forward=l_vEigenVector.begin(), l_u32Steps = 0; 
				it_forward!=l_vEigenVector.end() && l_u32Steps < ::ceil(m_ui64FilterDimension/2.0);
				it_forward++, l_u32Steps++)
			{
				this->getLogManager() << LogLevel_Debug << it_forward->first << ", ";
				for(uint32 j=0; j<l_ui32ChannelCount; j++)
				{
					::fprintf(l_pFile, "%e ", it_forward->second[j]);
				}
			}
			this->getLogManager() << LogLevel_Debug << "\n";

			std::map < double, itpp::vec >::const_reverse_iterator it_backward;
			this->getLogManager() << LogLevel_Debug << "highest eigenvalues: " << "\n";
			for(it_backward=l_vEigenVector.rbegin(), l_u32Steps = 0; 
				it_backward!=l_vEigenVector.rend() && l_u32Steps < ::floor(m_ui64FilterDimension/2.0); 
				it_backward++, l_u32Steps++)
			{
				this->getLogManager() << LogLevel_Debug << it_backward->first << ", ";
				for(uint32 j=0; j<l_ui32ChannelCount; j++)
				{
					::fprintf(l_pFile, "%e ", it_backward->second[j]);
				}
			}
			this->getLogManager() << LogLevel_Debug << "\n";
			::fprintf(l_pFile, "</SettingValue>\n");
			::fprintf(l_pFile, "\t<SettingValue>%d</SettingValue>\n", uint32(m_ui64FilterDimension));
			::fprintf(l_pFile, "\t<SettingValue>%d</SettingValue>\n", l_ui32ChannelCount);
			::fprintf(l_pFile, "</OpenViBE-SettingsOverride>\n");
			::fclose(l_pFile);

		}
		else
		{
			this->getLogManager() << LogLevel_ImportantWarning << "Eigen vector decomposition failed...\n";
			return true;
		}

		this->getLogManager() << LogLevel_Info << "CSP Spatial filter trained successfully.\n";

		m_oStimulationEncoder.getInputStimulationSet()->clear();
		m_oStimulationEncoder.getInputStimulationSet()->appendStimulation(OVTK_StimulationId_TrainCompleted, l_ui64TrainDate, 0);
		m_oStimulationEncoder.encodeBuffer();

		l_rDynamicBoxContext.markOutputAsReadyToSend(0,l_ui64TrainChunkStartTime,l_ui64TrainChunkEndTime);
	}

	return true;
}

#endif // TARGET_HAS_ThirdPartyITPP
