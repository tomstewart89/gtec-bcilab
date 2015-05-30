#include "ovpCBoxAlgorithmCSVFileReader.h"
#include <iostream>
#include <sstream>
#include <map>
#include <cmath>  // std::ceil() on Linux

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::FileIO;

namespace
{
	std::vector < std::string > split(const std::string& sString, const std::string c)
	{
		std::vector < std::string > l_vResult;
		std::string::size_type i=0;
		std::string::size_type j=0;
		while((j=sString.find(c,i))!=std::string::npos)
		{

			l_vResult.push_back(std::string(sString,i,j-i));
			i=j+c.size();
		}
		//the last element without the \n character
		l_vResult.push_back(std::string(sString,i,sString.size()-1-i));

		return l_vResult;
	}

	void clearMatrix(std::vector<std::vector<std::string> >& vMatrix)
	{
		for(uint32 i=0;i<vMatrix.size();i++)
		{
			vMatrix[i].clear();
		}
		vMatrix.clear();
	}
};

CBoxAlgorithmCSVFileReader::CBoxAlgorithmCSVFileReader(void)
	: m_pFile(NULL),
	m_ui64SamplingRate(0),
	m_fpRealProcess(NULL),
	m_bHeaderSent(false)
{
}

uint64 CBoxAlgorithmCSVFileReader::getClockFrequency(void)
{
	return 128LL<<32; // the box clock frequency
}

boolean CBoxAlgorithmCSVFileReader::initialize(void)
{
	m_ui64SamplingRate = 0;
	m_pAlgorithmEncoder = NULL;

	this->getStaticBoxContext().getOutputType(0,m_oTypeIdentifier);

	m_sFilename= FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	CString l_sSeparator=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_sSeparator=l_sSeparator.toASCIIString();
	m_bDoNotUseFileTime = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
	m_ui32SamplesPerBuffer=1;
	if(m_oTypeIdentifier == OV_TypeId_ChannelLocalisation)
	{
		CString l_sParam;
		this->getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(3,l_sParam);
		m_ui32ChannelNumberPerBuffer=static_cast<uint32>(atoi((const char*)l_sParam));

	}
	else if(m_oTypeIdentifier != OV_TypeId_Stimulations
			&& m_oTypeIdentifier != OV_TypeId_Spectrum)
	{
		CString l_sParam;
		this->getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(3,l_sParam);
		m_ui32SamplesPerBuffer=static_cast<uint32>(atoi((const char*)l_sParam));
	}


	m_bUseCompression=false;
	getLogManager() << LogLevel_Trace << "use the file time: "<<(!m_bDoNotUseFileTime)<<"\n";
	m_f64NextTime=0.;

	m_ui64ChunkStartTime=0;
	m_ui64ChunkEndTime=0;

	return true;
}

boolean CBoxAlgorithmCSVFileReader::uninitialize(void)
{
	if(m_pFile)
	{
		::fclose(m_pFile);
		m_pFile=NULL;
	}
	if(m_pAlgorithmEncoder) 
	{
		m_pAlgorithmEncoder->uninitialize();
		delete m_pAlgorithmEncoder;
		m_pAlgorithmEncoder = NULL;
	}
	return true;
}

boolean CBoxAlgorithmCSVFileReader::initializeFile()
{
	//open file
	m_pFile=::fopen(m_sFilename.toASCIIString(), "r"); // we don't open as binary as that gives us \r\n on Windows as line-endings and leaves a dangling char after split. CSV files should be text.
	if(!m_pFile)
	{
		this->getLogManager() << LogLevel_Error << "Could not open file [" << m_sFilename << "] for reading\n";
		return false;
	}

	//read the header
	char l_pLine[m_ui32bufferLen];
	char *l_pResult=::fgets(l_pLine,m_ui32bufferLen,m_pFile);
	if(NULL==l_pResult)
	{
		// bad : FIXME handle reading error here
		fclose(m_pFile);
		m_pFile = NULL;
		this->getLogManager() << LogLevel_Error << "fgets() error\n";
		return false;
	}
	m_vHeaderFile=split(std::string(l_pLine),m_sSeparator);
	m_ui32NbColumn=m_vHeaderFile.size();

	if(m_oTypeIdentifier == OV_TypeId_ChannelLocalisation)
	{
		m_pAlgorithmEncoder= new OpenViBEToolkit::TChannelLocalisationEncoder < CBoxAlgorithmCSVFileReader >(*this,0);
		//number of column without the column contains the dynamic parameter
		//m_ui32NbColumn-=1;
		m_fpRealProcess=&CBoxAlgorithmCSVFileReader::process_channelLocalisation;
	}
	else if(m_oTypeIdentifier == OV_TypeId_FeatureVector)
	{
		m_pAlgorithmEncoder=new OpenViBEToolkit::TFeatureVectorEncoder < CBoxAlgorithmCSVFileReader >(*this,0);
		m_fpRealProcess=&CBoxAlgorithmCSVFileReader::process_featureVector;
		m_ui32SamplesPerBuffer = 1;
	}
	else if(m_oTypeIdentifier == OV_TypeId_Spectrum)
	{
		m_pAlgorithmEncoder=new OpenViBEToolkit::TSpectrumEncoder < CBoxAlgorithmCSVFileReader >(*this,0);
		m_fpRealProcess=&CBoxAlgorithmCSVFileReader::process_spectrum;

		//number of column without columns contains min max frequency bands parameters
		m_ui32NbColumn-=2;
	}
	else if(m_oTypeIdentifier == OV_TypeId_Signal)
	{
		m_pAlgorithmEncoder=new OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmCSVFileReader >(*this,0);
		m_fpRealProcess=&CBoxAlgorithmCSVFileReader::process_signal;

		//find the sampling rate
		l_pResult=::fgets(l_pLine,m_ui32bufferLen,m_pFile);
		if(NULL==l_pResult)
		{
			fclose(m_pFile);
			m_pFile=NULL;
			this->getLogManager() << LogLevel_Error << "fgets() error\n";
			return false;
		}
		std::vector<std::string> l_vParsed = split(std::string(l_pLine),m_sSeparator);
		if((m_ui32NbColumn-1)>=l_vParsed.size()) {
			fclose(m_pFile);
			m_pFile=NULL;
			this->getLogManager() << LogLevel_Error << "Line didn't have enough components. Remember it needs to have +1 for freq. \n";
			return false;
		}
		const float64 l_f64SamplingRate = static_cast<float64>(atof(l_vParsed[m_ui32NbColumn-1].c_str()));
		if(std::ceil(l_f64SamplingRate) != l_f64SamplingRate)
		{
			this->getLogManager() << LogLevel_Error << "Extracted sampling rate (column "
				<< m_ui32NbColumn << ") appears fractional (" << l_f64SamplingRate << " hz), but fractional sampling rates are not supported. "
				<< "As this is likely a parsing error, please modify the file to correspond to OV conventions (see CSV Reader documentation).\n";
			return false;
		}

		m_ui64SamplingRate=static_cast<uint64>(l_f64SamplingRate);

		if(m_ui64SamplingRate == 0)
		{
			this->getLogManager() << LogLevel_Error << "Parsed the sampling rate as 0, this is not acceptable for a signal stream.\n";
			return false;
		}

		// Skip the header
		::rewind(m_pFile);
		l_pResult=::fgets(l_pLine,m_ui32bufferLen,m_pFile);
		if(NULL==l_pResult)
		{
			fclose(m_pFile);
			m_pFile=NULL;
			this->getLogManager() << LogLevel_Error << "fgets() error\n";
			return false;
		}

		//number of column without the column contains the sampling rate parameters
		m_ui32NbColumn-=1;
	}
	else if(m_oTypeIdentifier == OV_TypeId_StreamedMatrix)
	{
		m_pAlgorithmEncoder=new OpenViBEToolkit::TStreamedMatrixEncoder < CBoxAlgorithmCSVFileReader >(*this,0);
		m_fpRealProcess=&CBoxAlgorithmCSVFileReader::process_streamedMatrix;
	}
	else if(m_oTypeIdentifier == OV_TypeId_Stimulations)
	{
		m_pAlgorithmEncoder=new OpenViBEToolkit::TStimulationEncoder < CBoxAlgorithmCSVFileReader >(*this,0);
		m_fpRealProcess=&CBoxAlgorithmCSVFileReader::process_stimulation;
	}
	else
	{
		this->getLogManager() << LogLevel_Error << "Invalid input type identifier " << this->getTypeManager().getTypeName(m_oTypeIdentifier) << "\n";
		return false;
	}
	getLogManager()<< LogLevel_Trace <<"number of column without parameters: "<<m_ui32NbColumn<<"\n";

	return true;
}

boolean CBoxAlgorithmCSVFileReader::processClock(IMessageClock& rMessageClock)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CBoxAlgorithmCSVFileReader::process(void)
{
	if(m_pFile == NULL)
	{
		if(!initializeFile())
		{
			return false;
		}
	}
	//line buffer
	char l_pLine[m_ui32bufferLen];
	const float64 l_f64currentTime=ITimeArithmetics::timeToSeconds(getPlayerContext().getCurrentTime());
	//IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	//if no line was read, read the first data line.
	if(m_vLastLineSplit.size() == 0)
	{
		//next line
		uint32 l_ui32NbSamples=0;
		while(!feof(m_pFile) && l_ui32NbSamples<m_ui32SamplesPerBuffer && fgets(l_pLine, m_ui32bufferLen, m_pFile) != NULL )
		{
			m_vLastLineSplit=split(std::string(l_pLine),m_sSeparator);

			l_ui32NbSamples++;

			if(m_oTypeIdentifier != OV_TypeId_Stimulations
					&& m_oTypeIdentifier != OV_TypeId_Spectrum
					&& m_oTypeIdentifier != OV_TypeId_ChannelLocalisation)
			{
				m_vDataMatrix.push_back(m_vLastLineSplit);
			}
		}
		if( (m_oTypeIdentifier == OV_TypeId_StreamedMatrix || m_oTypeIdentifier == OV_TypeId_Signal)
			&& feof(m_pFile) && l_ui32NbSamples<m_ui32SamplesPerBuffer)
		{
			// Last chunk will be partial, zero the whole output matrix... 
			IMatrix* ip_pMatrix=((OpenViBEToolkit::TStreamedMatrixEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMatrix();
			OpenViBEToolkit::Tools::Matrix::clearContent(*ip_pMatrix);
		}
	}

	bool l_bSomethingToSend= (m_vLastLineSplit.size()!=0)
		&& atof(m_vLastLineSplit[0].c_str())<l_f64currentTime;
	l_bSomethingToSend |= (m_oTypeIdentifier == OV_TypeId_Stimulations); // we always send a stim chunk, even if empty

	if(m_oTypeIdentifier == OV_TypeId_Stimulations
			|| m_oTypeIdentifier == OV_TypeId_ChannelLocalisation
			|| m_oTypeIdentifier == OV_TypeId_Spectrum)
	{
		while(m_vLastLineSplit.size()!=0 && atof(m_vLastLineSplit[0].c_str())<l_f64currentTime)
		{
			m_vDataMatrix.push_back(m_vLastLineSplit);

			l_bSomethingToSend=true;

			if(!feof(m_pFile) && fgets(l_pLine, m_ui32bufferLen, m_pFile) != NULL)
			{
				m_vLastLineSplit=split(std::string(l_pLine),m_sSeparator);

			}
			else
			{
				m_vLastLineSplit.clear();
			}
		}
	}

	//convert data to the good output type

	if(l_bSomethingToSend)
	{
		// Encode the data
		if(!(this->*m_fpRealProcess)())
		{
			return false;
		}

		//for the stimulation, the line contents in m_vLastLineSplit isn't processed.
		if(m_oTypeIdentifier != OV_TypeId_Stimulations
				&& m_oTypeIdentifier != OV_TypeId_Spectrum
				&& m_oTypeIdentifier != OV_TypeId_ChannelLocalisation)
		{
			m_vLastLineSplit.clear();
		}

		//clear the Data Matrix.
		clearMatrix(m_vDataMatrix);
	}
	return true;

}

boolean CBoxAlgorithmCSVFileReader::process_streamedMatrix(void)
{
	IMatrix* ip_pMatrix=((OpenViBEToolkit::TStreamedMatrixEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMatrix();

	//Header
	if(!m_bHeaderSent)
	{
		IMatrix* ip_pMatrix=((OpenViBEToolkit::TStreamedMatrixEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMatrix();
		ip_pMatrix->setDimensionCount(2);
		ip_pMatrix->setDimensionSize(0,m_ui32NbColumn-1);
		ip_pMatrix->setDimensionSize(1,m_ui32SamplesPerBuffer);

		for(uint32 i=1;i<m_ui32NbColumn;i++)
		{
			ip_pMatrix->setDimensionLabel(0,i-1,m_vHeaderFile[i].c_str());
		}
		m_pAlgorithmEncoder->encodeHeader();
		m_bHeaderSent = true;

		this->getDynamicBoxContext().markOutputAsReadyToSend(0,0,0);
	}

	if(!convertVectorDataToMatrix(ip_pMatrix))
	{
		return false;
	}

	m_pAlgorithmEncoder->encodeBuffer();

	if(m_bDoNotUseFileTime)
	{
		m_ui64ChunkStartTime=m_ui64ChunkEndTime;
		m_ui64ChunkEndTime=this->getPlayerContext().getCurrentTime();
	}
	else
	{
		m_ui64ChunkStartTime=ITimeArithmetics::secondsToTime(atof(m_vDataMatrix[0][0].c_str()));
		m_ui64ChunkEndTime=ITimeArithmetics::secondsToTime(atof(m_vDataMatrix.back()[0].c_str()));
	}

	this->getDynamicBoxContext().markOutputAsReadyToSend(0,m_ui64ChunkStartTime,m_ui64ChunkEndTime);

	return true;
}

OpenViBE::boolean CBoxAlgorithmCSVFileReader::process_stimulation(void)
{
	//Header
	if(!m_bHeaderSent)
	{
		m_pAlgorithmEncoder->encodeHeader();
		m_bHeaderSent = true;

		this->getDynamicBoxContext().markOutputAsReadyToSend(0,0,0);
	}

	IStimulationSet* ip_pStimulationSet=((OpenViBEToolkit::TStimulationEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputStimulationSet();
	ip_pStimulationSet->clear();

	for(uint32 i=0;i<m_vDataMatrix.size();i++)
	{
		if(m_vDataMatrix[i].size()!=3)
		{
			this->getLogManager() << LogLevel_Warning << "Unexpected data row length, skipping\n";
			continue;
		}
		//stimulation date
		const uint64 l_ui64StimulationDate = ITimeArithmetics::secondsToTime(atof(m_vDataMatrix[i][0].c_str()));

		//stimulation indices
		const uint64 l_ui64Stimulation=(uint64)atof(m_vDataMatrix[i][1].c_str());

		//stimulation duration
		const uint64 l_ui64StimulationDuration = ITimeArithmetics::secondsToTime(atof(m_vDataMatrix[i][2].c_str()));

		getLogManager()<<LogLevel_Trace<<"  Stimulation "<<l_ui64Stimulation<<" start at time: "<<l_ui64StimulationDate<<"("<<ITimeArithmetics::timeToSeconds(l_ui64StimulationDate)<<" s)"<<" end during:"<<l_ui64StimulationDuration<<"("<<ITimeArithmetics::timeToSeconds(l_ui64StimulationDuration)<<" s)\n";

		ip_pStimulationSet->appendStimulation(l_ui64Stimulation,l_ui64StimulationDate,l_ui64StimulationDuration);

	}

	m_pAlgorithmEncoder->encodeBuffer();

	// Never use file time
	m_ui64ChunkStartTime = m_ui64ChunkEndTime;
	m_ui64ChunkEndTime = this->getPlayerContext().getCurrentTime();

	this->getDynamicBoxContext().markOutputAsReadyToSend(0,m_ui64ChunkStartTime, m_ui64ChunkEndTime);

	return true;
}

OpenViBE::boolean CBoxAlgorithmCSVFileReader::process_signal(void)
{
	IMatrix* ip_pMatrix=((OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMatrix();

	//Header
	if(!m_bHeaderSent)
	{
		// This is the first chunk, find out the start time from the file
		// (to keep time chunks continuous, start time is previous end time, hence set end time)
		if(!m_bDoNotUseFileTime)
		{
			m_ui64ChunkEndTime =  ITimeArithmetics::secondsToTime(atof(m_vDataMatrix[0][0].c_str()));
		}

		IMatrix* ip_pMatrix=((OpenViBEToolkit::TStreamedMatrixEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMatrix();
		ip_pMatrix->setDimensionCount(2);
		ip_pMatrix->setDimensionSize(0,m_ui32NbColumn-1);
		ip_pMatrix->setDimensionSize(1,m_ui32SamplesPerBuffer);

		for(uint32 i=1;i<m_ui32NbColumn;i++)
		{
			ip_pMatrix->setDimensionLabel(0,i-1,m_vHeaderFile[i].c_str());
		}

		this->getLogManager() << LogLevel_Trace << "Sampling rate is " << m_ui64SamplingRate << "hz.\n";
		((OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputSamplingRate()=m_ui64SamplingRate;

		m_pAlgorithmEncoder->encodeHeader();
		m_bHeaderSent = true;

		this->getDynamicBoxContext().markOutputAsReadyToSend(0,0,0);
	}

	if(!convertVectorDataToMatrix(ip_pMatrix))
	{
		return false;
	}

	// this->getLogManager() << LogLevel_Info << "Cols from header " << m_ui32NbColumn << "\n";
	// this->getLogManager() << LogLevel_Info << "InMatrix " << (m_vDataMatrix.size() > 0 ? m_vDataMatrix[0].size() : 0) << " outMatrix " << ip_pMatrix->getDimensionSize(0) << "\n";

	m_pAlgorithmEncoder->encodeBuffer();

	if(m_bDoNotUseFileTime)
	{
		// We use time dictated by the sampling rate
		m_ui64ChunkStartTime = m_ui64ChunkEndTime; // previous time end is current time start
		m_ui64ChunkEndTime = m_ui64ChunkStartTime + ITimeArithmetics::sampleCountToTime(m_ui64SamplingRate, m_ui32SamplesPerBuffer);
	}
	else
	{
		// We use time suggested by the last sample of the chunk
		m_ui64ChunkStartTime = ITimeArithmetics::secondsToTime(atof(m_vDataMatrix[0][0].c_str()));
		m_ui64ChunkEndTime = ITimeArithmetics::secondsToTime(atof(m_vDataMatrix.back()[0].c_str()));
	}

	this->getDynamicBoxContext().markOutputAsReadyToSend(0,m_ui64ChunkStartTime,m_ui64ChunkEndTime);

	return true;
}
OpenViBE::boolean CBoxAlgorithmCSVFileReader::process_channelLocalisation(void)
{
	IMatrix* ip_pMatrix = ((OpenViBEToolkit::TChannelLocalisationEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMatrix();

	if(!m_bHeaderSent)
	{
		IMatrix* ip_pMatrix=((OpenViBEToolkit::TStreamedMatrixEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMatrix();
		ip_pMatrix->setDimensionCount(2);
		ip_pMatrix->setDimensionSize(0,m_ui32NbColumn-1);
		ip_pMatrix->setDimensionSize(1,m_ui32SamplesPerBuffer);

		for(uint32 i=1;i<m_ui32NbColumn;i++)
		{
			ip_pMatrix->setDimensionLabel(0,i-1,m_vHeaderFile[i].c_str());
		}

		((OpenViBEToolkit::TChannelLocalisationEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputDynamic()=false;//atoi(m_vDataMatrix[0][m_ui32NbColumn].c_str());

		m_pAlgorithmEncoder->encodeHeader();

		this->getDynamicBoxContext().markOutputAsReadyToSend(0,0,0);

		m_bHeaderSent = true;
	}

	std::vector<std::vector<std::string> > l_vChannelBloc;
	for(uint32 i=0;i<m_vDataMatrix.size();i++)
	{
		l_vChannelBloc.push_back(m_vDataMatrix[i]);
	}

	//clear matrix
	clearMatrix(m_vDataMatrix);

	for(uint64 i=0;i<l_vChannelBloc.size();i++)
	{
		m_vDataMatrix.push_back(l_vChannelBloc[(unsigned int)i]);

		//send the current bloc if the next data hasn't the same date
		if(i>=l_vChannelBloc.size()-1 || l_vChannelBloc[(unsigned int)(i+1)][0]!=m_vDataMatrix[0][0])
		{

			if(!convertVectorDataToMatrix(ip_pMatrix))
			{
				return false;
			}

			m_pAlgorithmEncoder->encodeBuffer();
			const uint64 l_ui64Date=ITimeArithmetics::secondsToTime(atof(m_vDataMatrix[0][0].c_str()));
			this->getDynamicBoxContext().markOutputAsReadyToSend(0,l_ui64Date,l_ui64Date);

			//clear matrix
			clearMatrix(m_vDataMatrix);
		}

	}

	//clear matrix
	clearMatrix(l_vChannelBloc);

	return true;
}

OpenViBE::boolean CBoxAlgorithmCSVFileReader::process_featureVector(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	IMatrix* l_pMatrix = ((OpenViBEToolkit::TFeatureVectorEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMatrix();

	//Header
	if(!m_bHeaderSent)
	{
		// in this case we need to transpose it
		IMatrix* ip_pMatrix=((OpenViBEToolkit::TStreamedMatrixEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMatrix();

		ip_pMatrix->setDimensionCount(1);
		ip_pMatrix->setDimensionSize(0,m_ui32NbColumn-1);

		for(uint32 i=1;i<m_ui32NbColumn;i++)
		{
			ip_pMatrix->setDimensionLabel(0,i-1,m_vHeaderFile[i].c_str());
		}

		m_pAlgorithmEncoder->encodeHeader();

		this->getDynamicBoxContext().markOutputAsReadyToSend(0,0,0);

		m_bHeaderSent = true;
	}

	// Each vector has to be sent separately
	for(uint32 i=0;i<m_vDataMatrix.size();i++)
	{
		if(m_vDataMatrix[i].size()!=m_ui32NbColumn) {
			this->getLogManager() << LogLevel_Warning << "Unexpected number of elements"
				<< "(got " << static_cast<uint64>(m_vDataMatrix[i].size()) << ", expected " << m_ui32NbColumn << "), skipping line\n";
			continue;
		}
		for(uint32 j=0;j<m_ui32NbColumn-1;j++)
		{

			l_pMatrix->getBuffer()[j]=atof(m_vDataMatrix[i][j+1].c_str());
		}

		m_pAlgorithmEncoder->encodeBuffer();

		const uint64 l_ui64StartTime = ITimeArithmetics::secondsToTime(atof(m_vDataMatrix[i][0].c_str()));
		l_rDynamicBoxContext.markOutputAsReadyToSend(0,l_ui64StartTime,l_ui64StartTime);
	}
		
	clearMatrix(m_vDataMatrix);

	return true;
}
OpenViBE::boolean CBoxAlgorithmCSVFileReader::process_spectrum(void)
{
	IMatrix* ip_pMatrix = ((OpenViBEToolkit::TSpectrumEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMatrix();
	IMatrix* ip_pMinMaxFrequencyBands = ((OpenViBEToolkit::TSpectrumEncoder < CBoxAlgorithmCSVFileReader >*)m_pAlgorithmEncoder)->getInputMinMaxFrequencyBands();

	//Header
	if(!m_bHeaderSent)
	{
		ip_pMatrix->setDimensionCount(2);
		ip_pMatrix->setDimensionSize(0,m_ui32NbColumn-1);
		ip_pMatrix->setDimensionSize(1,m_vDataMatrix.size());

		for(uint32 i=1;i<m_ui32NbColumn;i++)
		{
			ip_pMatrix->setDimensionLabel(0,i-1,m_vHeaderFile[i].c_str());
		}
		ip_pMinMaxFrequencyBands->setDimensionCount(3);
		ip_pMinMaxFrequencyBands->setDimensionSize(0,m_ui32NbColumn-1);
		ip_pMinMaxFrequencyBands->setDimensionSize(1,m_vDataMatrix.size());
		ip_pMinMaxFrequencyBands->setDimensionSize(2,2);
		for(uint32 j=0;j<m_ui32NbColumn-1;j++)
		{
			for(uint32 i=0;i<m_vDataMatrix.size();i++)
			{
				ip_pMinMaxFrequencyBands->getBuffer()[j*m_vDataMatrix.size()*2+i*2]=atof(m_vDataMatrix[i][m_ui32NbColumn].c_str());
				ip_pMinMaxFrequencyBands->getBuffer()[j*m_vDataMatrix.size()*2+i*2+1]=atof(m_vDataMatrix[i][m_ui32NbColumn+1].c_str());
				std::stringstream l_sLabel;
				l_sLabel<<(16*i)<<"-"<<(16*(i+1));
				ip_pMinMaxFrequencyBands->setDimensionLabel(1,i,l_sLabel.str().c_str());
			}
		}
		m_bHeaderSent = true;
		m_pAlgorithmEncoder->encodeHeader();

		this->getDynamicBoxContext().markOutputAsReadyToSend(0,0,0);
	}

	std::vector<std::vector<std::string> > l_vSpectrumBloc;
	for(uint32 i=0;i<m_vDataMatrix.size();i++)
	{
		l_vSpectrumBloc.push_back(m_vDataMatrix[i]);
	}

	//clear matrix
	clearMatrix(m_vDataMatrix);

	for(uint64 i=0;i<l_vSpectrumBloc.size();i++)
	{
		m_vDataMatrix.push_back(l_vSpectrumBloc[(unsigned int)i]);
		//send the current bloc if the next data hasn't the same date
		if(i>=l_vSpectrumBloc.size()-1 || l_vSpectrumBloc[(unsigned int)(i+1)][0]!=m_vDataMatrix[0][0])
		{
			if(!convertVectorDataToMatrix(ip_pMatrix))
			{
				return false;
			}

			m_pAlgorithmEncoder->encodeBuffer();
			const uint64 l_ui64Date = ITimeArithmetics::secondsToTime(atof(m_vDataMatrix[0][0].c_str()));
			this->getDynamicBoxContext().markOutputAsReadyToSend(0,l_ui64Date-1,l_ui64Date);

			//clear matrix
			clearMatrix(m_vDataMatrix);
		}

	}

	//clear matrix
	clearMatrix(l_vSpectrumBloc);
	return true;
}

bool CBoxAlgorithmCSVFileReader::convertVectorDataToMatrix(IMatrix* matrix)
{
	// note: Chunk size shouldn't change after encoding header, do not mess with it here, even if the input has different size

	// We accept partial data, but not buffer overruns ...
	if(matrix->getDimensionSize(1) < m_vDataMatrix.size()
		|| matrix->getDimensionSize(0) < m_ui32NbColumn-1 ) {
		this->getLogManager() << LogLevel_Error 
			<< "Matrix size incompatibility, data suggests " 
			<< m_ui32NbColumn-1 << "x" << static_cast<uint64>(m_vDataMatrix.size()) << ", expected at most " 
			<< matrix->getDimensionSize(0) << "x" << matrix->getDimensionSize(0) 
			<< "\n";
		return false;
	}
	
	std::stringstream  l_sMatrix;
	for(uint32 i=0;i<m_vDataMatrix.size();i++)
	{
		l_sMatrix<<"at time ("<<m_vDataMatrix[i][0].c_str()<<"):";
		for(uint32 j=0;j<m_ui32NbColumn-1;j++)
		{
			matrix->getBuffer()[j*matrix->getDimensionSize(1)+i]=atof(m_vDataMatrix[i][j+1].c_str());
			l_sMatrix<<matrix->getBuffer()[j*matrix->getDimensionSize(1)+i]<<";";
		}
		l_sMatrix<<"\n";
	}
	getLogManager()<<LogLevel_Debug<<"Matrix:\n"<<l_sMatrix.str().c_str();
	getLogManager()<<LogLevel_Debug<<"Matrix:\n"<<l_sMatrix.str().c_str();

	return true;
}
