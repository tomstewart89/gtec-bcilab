/* Project: Gipsa-lab plugins for OpenVibe
 * AUTHORS AND CONTRIBUTORS: Andreev A., Barachant A., Congedo M., Ionescu,Gelu,

 * This file is part of "Gipsa-lab plugins for OpenVibe".
 * You can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with Brain Invaders. If not, see http://www.gnu.org/licenses/.*/
 
#include "ovpCBoxAlgorithmBrainampFileWriter.h"

#include <string>
#include <iostream>
#include <sstream>

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::FileIO;

CBoxAlgorithmBrainampFileWriter::CBoxAlgorithmBrainampFileWriter(void)
	:
	m_pStreamDecoder(NULL)
	,m_pMatrix(NULL)
	,m_pStimulationDecoderTrigger(NULL)
	,m_uint32StimulationCounter(2) //because the first is outputed manually
{
}

boolean CBoxAlgorithmBrainampFileWriter::initialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	//init input signal 1
	m_pStreamDecoder=new OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmBrainampFileWriter >(*this,0);

	//init input stimulation 1 
	m_pStimulationDecoderTrigger=new OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmBrainampFileWriter >(*this,1);

	//Get parameters:
	CString l_sFilename=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);

	//Get binary format
	CIdentifier l_oBinaryFormatIdentifier;
	CString l_sBinaryFormatIdentifier;
	l_rStaticBoxContext.getSettingValue(1, l_sBinaryFormatIdentifier);
	l_oBinaryFormatIdentifier=this->getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_BinaryFormat, l_sBinaryFormatIdentifier);

	if (l_oBinaryFormatIdentifier == OVP_TypeId_BinaryFormat_int16) m_ui32BinaryFormat = BinaryFormat_Integer16;
	else if (l_oBinaryFormatIdentifier == OVP_TypeId_BinaryFormat_uint16) m_ui32BinaryFormat = BinaryFormat_UnsignedInteger16;
	else if (l_oBinaryFormatIdentifier == OVP_TypeId_BinaryFormat_float32) m_ui32BinaryFormat = BinaryFormat_Float32;
	else {this->getLogManager() << LogLevel_Error << "Unknown binary format: " << l_oBinaryFormatIdentifier << "\n"; return false;}
	
	//Perform checks:
	if (std::string(l_sFilename) == "")
	{
		this->getLogManager() << LogLevel_Error << "Header file path is empty!\n";
		return false;
	}

	if (std::string(l_sFilename).substr(l_sFilename.length()-5,5) != std::string(".vhdr"))
	{
		this->getLogManager() << LogLevel_Warning << "The supplied output file does not end with .vhdr\n";
	}

	m_sHeaderFilename = l_sFilename;

	m_oHeaderFile.open(m_sHeaderFilename.c_str());
	if(!m_oHeaderFile.good())
	{
		this->getLogManager() << LogLevel_Error << "Could not open header file [" << m_sHeaderFilename.c_str() << "]\n";
		return false;
	}
	
	//this->getLogManager() << LogLevel_ImportantWarning << m_sHeaderFilename.c_str() << "\n";

	m_sDataFilename =  m_sHeaderFilename.substr(0,m_sHeaderFilename.length()-5) + std::string(".eeg");

	m_oDataFile.open(m_sDataFilename.c_str(), std::ios::binary);
	if(!m_oDataFile.good())
	{
		this->getLogManager() << LogLevel_Error << "Could not open data file [" << m_sDataFilename.c_str() << "]\n";
		return false;
	}

	m_sMarkerFilename =  m_sHeaderFilename.substr(0,m_sHeaderFilename.length()-5) + std::string(".vmrk");

	m_oMarkerFile.open(m_sMarkerFilename.c_str());
	if(!m_oMarkerFile.good())
	{
		this->getLogManager() << LogLevel_Error << "Could not open marker file [" << m_sMarkerFilename.c_str() << "]\n";
		return false;
	}

	return true;
}

boolean CBoxAlgorithmBrainampFileWriter::uninitialize(void)
{

	if(m_pStreamDecoder)
	{
		m_pStreamDecoder->uninitialize();
		delete m_pStreamDecoder;
	}

	// uninit input stimulation
	if(m_pStimulationDecoderTrigger)
	{
		m_pStimulationDecoderTrigger->uninitialize();
		delete m_pStimulationDecoderTrigger;
	}

	//close files
	m_oHeaderFile.flush();
	m_oHeaderFile.close();

	m_oDataFile.flush();
	m_oDataFile.close();

	m_oMarkerFile.flush();
	m_oMarkerFile.close();

	return true;
}

boolean CBoxAlgorithmBrainampFileWriter::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CBoxAlgorithmBrainampFileWriter::process(void)
{
	// IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	//1. Process signal
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++) //first input channel data
	{
		// uint64 l_ui64StartTime=l_rDynamicBoxContext.getInputChunkStartTime(0, i);
		// uint64 l_ui64EndTime=l_rDynamicBoxContext.getInputChunkEndTime(0, i);
		

		m_pStreamDecoder->decode(i);

		//HEADER
		if(m_pStreamDecoder->isHeaderReceived())
		{
		  
		  m_pMatrix=m_pStreamDecoder->getOutputMatrix();
		  m_ui64SamplingFrequency = m_pStreamDecoder->getOutputSamplingRate();

			writeHeaderFile();
		}

		//BUFFER
		if(m_pStreamDecoder->isBufferReceived())
		{
			// OpenViBE::uint32 l_uint32ChannelCount = m_pMatrix->getDimensionSize(0);
	
			switch (m_ui32BinaryFormat)
			{
				case BinaryFormat_Integer16: 
						saveBuffer(OpenViBE::int16(0));
						break;

				case BinaryFormat_UnsignedInteger16: 
						saveBuffer(OpenViBE::uint16(0));
						break;

				case BinaryFormat_Float32: 
						saveBuffer(OpenViBE::float32(0));
						break;
			}
		}

		//END
		if(m_pStreamDecoder->isEndReceived())
		{

		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}

	//2. Process stimulations - input channel 1
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(1); i++)
	{
		// uint64 l_ui64ChunkStartTime =l_rDynamicBoxContext.getInputChunkStartTime(0, i);

		
		m_pStimulationDecoderTrigger->decode(i);
		
		//header
		if(m_pStimulationDecoderTrigger->isHeaderReceived())
		{
			boost::posix_time::ptime l_dtNow = boost::posix_time::second_clock::local_time();
		    std::string l_ftFormated(FormatTime(l_dtNow));

			m_oMarkerFile 
				<< "Brain Vision Data Exchange Marker File, Version 1.0" <<std::endl
				<< std::endl
				<< "[Common Infos]" <<std::endl
				<< "Codepage=ANSI" <<std::endl
				<< "DataFile=" << getShortName(m_sDataFilename).c_str() << std::endl 
				<< std::endl
				<< "[Marker Infos]" <<std::endl
				<< "; Each entry: Mk<Marker number>=<Type>,<Description>,<Position in data points>," <<std::endl
				<< "; <Size in data points>, <Channel number (0 = marker is related to all channels)>" <<std::endl
				<< "; Fields are delimited by commas, some fields might be omitted (empty)." <<std::endl
				<< "; Commas in type or description text are coded as \"\\1\"." <<std::endl
				<< "Mk1=New Segment,,1,1,0," << l_ftFormated.c_str() << "000000" <<std::endl;
			    
		}
    
		//buffer 
		if(m_pStimulationDecoderTrigger->isBufferReceived())
		{
		  IStimulationSet* op_pStimulationSetTrigger = m_pStimulationDecoderTrigger->getOutputStimulationSet();
		  
			  // Loop on stimulations
			  for(uint32 j=0; j<op_pStimulationSetTrigger->getStimulationCount(); j++)
			  {
				uint64 code = op_pStimulationSetTrigger->getStimulationIdentifier(j);

				uint64 position = ITimeArithmetics::timeToSampleCount(m_ui64SamplingFrequency, op_pStimulationSetTrigger->getStimulationDate(j)) + 1;

				m_oMarkerFile << "Mk" << m_uint32StimulationCounter << "=Stimulus," << "S" << std::right << std::setw(3) << code << "," << position << ",1,0" << std::endl;
			
				m_uint32StimulationCounter++;
        
			  }
		}

		l_rDynamicBoxContext.markInputAsDeprecated(1, i);
	}

	return true;
}

OpenViBE::boolean CBoxAlgorithmBrainampFileWriter::writeHeaderFile()
{
	// IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	// IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	OpenViBE::uint32 l_uint32ChannelCount = m_pMatrix->getDimensionSize(0);
	// OpenViBE::uint32 l_uint32SamplesPerChunk = m_pMatrix->getDimensionSize(1);

	OpenViBE::float64 l_f64Sampling_Interval = 1000000.0 / static_cast<float64>(m_ui64SamplingFrequency);

	CString format("UNKNOWN");
	switch (m_ui32BinaryFormat)
	{
		case BinaryFormat_Integer16: 
				format = "INT_16";
				break;

		case BinaryFormat_UnsignedInteger16: 
				format = "UINT_16";
				break;

		case BinaryFormat_Float32: 
				format = "IEEE_FLOAT_32"; //TODO: should be "IEEE_FLOAT_32", but OpenVibe Brainamp reader uses "FLOAT_32" for due to bug
				//this->getLogManager() << LogLevel_ImportantWarning<< "The BinaryFormat will be saved as 'FLOAT_32' instead of 'IEEE_FLOAT_32'. This can be a problem for other programs that read Brainamp format.\n";
				break;
		default:
				// Should not happen
				break;
	}

	m_oHeaderFile << "Brain Vision Data Exchange Header File Version 1.0" << std::endl
		<< "; Data created by the Vision Recorder" << std::endl
		<< std::endl
		<< "[Common Infos]" << std::endl
		<< "Codepage=ANSI" << std::endl
		<< "DataFile=" << getShortName(m_sDataFilename).c_str() << std::endl 
		<< "MarkerFile=" << getShortName(m_sMarkerFilename).c_str() << std::endl
		<< "DataFormat=BINARY" << std::endl
		<< "; Data orientation: MULTIPLEXED=ch1,pt1, ch2,pt1 ..." << std::endl //sample 1, sample 2 ...
		<< "DataOrientation=MULTIPLEXED" << std::endl
		<< "NumberOfChannels=" << l_uint32ChannelCount << std::endl 
		<< "; Sampling interval in microseconds" << std::endl
		<< "SamplingInterval=" << std::fixed << std::setprecision(5) << l_f64Sampling_Interval << std::endl
		<< std::endl
		<< "[Binary Infos]" << std::endl
		<< "BinaryFormat=" << format.toASCIIString() << std::endl
	    << std::endl; 

	m_oHeaderFile << "[Channel Infos]" << std::endl
        << "; Each entry: Ch<Channel number>=<Name>,<Reference channel name>," << std::endl
        << "; <Resolution in \"Unit\">,<Unit>, Future extensions.." << std::endl
        << "; Fields are delimited by commas, some fields might be omitted (empty)." << std::endl
        << "; Commas in channel names are coded as \"\\1\"." << std::endl;

		for(uint32 i=0; i< l_uint32ChannelCount; i++)
		{
			m_oHeaderFile << "Ch" << (i+1) << "=" << m_pMatrix->getDimensionLabel(0,i) << ",,1," << std::endl; //resolution = 1 
		}

		 m_oHeaderFile << std::endl;

	m_oHeaderFile 
		<< "[Comment]" << std::endl
		<< std::endl
		<< "A m p l i f i e r  S e t u p" << std::endl
		<< "============================" << std::endl
		<< "Number of channels: " << l_uint32ChannelCount << std::endl 
		<< "Sampling Rate [Hz]: " << (uint64)m_ui64SamplingFrequency << std::endl 
		<< "Interval [µS]: " << std::fixed << std::setprecision(5) << l_f64Sampling_Interval << std::endl
		<< std::endl;
	
		return true;
}

std::string CBoxAlgorithmBrainampFileWriter::getShortName(std::string fullpath)
{
	uint32 pos = fullpath.find_last_of("\\");
	if (pos == std::string::npos) pos = fullpath.find_last_of("/");

	if (pos != std::string::npos)
	{
		return fullpath.substr(pos+1, fullpath.size() - pos);
	}
	else
	{
		return fullpath;
	}
}

std::string CBoxAlgorithmBrainampFileWriter::FormatTime(boost::posix_time::ptime now)
{
  using namespace boost::posix_time;

  static std::locale loc(std::wcout.getloc(),
                         new wtime_facet(L"%Y%m%d%H%M%S"));

  std::basic_stringstream<wchar_t> wss;
  wss.imbue(loc);
  wss << now;
  std::wstring wstr = wss.str();
  std::string str(wstr.begin(),  wstr.end() );

  return str;
}


