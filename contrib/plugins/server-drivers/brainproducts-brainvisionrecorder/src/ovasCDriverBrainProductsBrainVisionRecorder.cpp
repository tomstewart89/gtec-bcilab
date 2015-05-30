#include "ovasCDriverBrainProductsBrainVisionRecorder.h"
#include "../ovasCConfigurationNetworkBuilder.h"

#include <system/ovCTime.h>

#include <cmath>

#include <iostream>

#include <cstdlib>
#include <cstring>

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

//___________________________________________________________________//
//                                                                   //

CDriverBrainProductsBrainVisionRecorder::CDriverBrainProductsBrainVisionRecorder(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_oSettings("AcquisitionServer_Driver_BrainVisionRecorder", m_rDriverContext.getConfigurationManager())
	,m_pCallback(NULL)
	,m_pConnectionClient(NULL)
	,m_sServerHostName("localhost")
	,m_ui32ServerHostPort(51244)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
{
	m_pStructRDA_MessageHeader = NULL;
	m_ui64HeaderSize = 0;
	m_ui64SampleSize = 0;
	m_bMarkerWarningGiven = false;

	m_oSettings.add("Header", &m_oHeader);
	m_oSettings.add("ServerHostName", &m_sServerHostName);
	m_oSettings.add("ServerHostPort", &m_ui32ServerHostPort);
	m_oSettings.load();
}

CDriverBrainProductsBrainVisionRecorder::~CDriverBrainProductsBrainVisionRecorder(void)
{
}

const char* CDriverBrainProductsBrainVisionRecorder::getName(void)
{
	return "Brain Products amplifiers (through BrainVision Recorder)";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverBrainProductsBrainVisionRecorder::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) { return false; }

	// Initialize GUID value
	DEFINE_GUID(GUID_RDAHeader,
		1129858446, 51606, 19590, uint8(175), uint8(74), uint8(152), uint8(187), uint8(246), uint8(201), uint8(20), uint8(80)
	);

	// Builds up client connection
	m_pConnectionClient=Socket::createConnectionClient();

	// Tries to connect to server
	m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort);

	// Checks if connection is correctly established
	if(!m_pConnectionClient->isConnected())
	{
		// In case it is not, try to reconnect
		m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort);
	}

	if(!m_pConnectionClient->isConnected())
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Connection problem! Tried 2 times without success! :(\n";
		m_rDriverContext.getLogManager() << LogLevel_Error << "Verify port number and/or Hostname...\n";
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Client connected\n";

	// Initialize vars for reception
	RDA_MessageHeader l_structRDA_MessageHeader;
	m_pcharStructRDA_MessageHeader = (char*)&l_structRDA_MessageHeader;

	uint32 l_ui32Received = 0;
	uint32 l_ui32ReqLength = 0;
	uint32 l_ui32Result = 0;
	uint32 l_ui32Datasize = sizeof(l_structRDA_MessageHeader);

	// Receive Header
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);

		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}

	// Check for correct header GUID.
	if (!COMPARE_GUID(l_structRDA_MessageHeader.guid, GUID_RDAHeader))
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "GUID received is not correct!\n";
		return false;
	}

	// Check for correct header nType
	if (l_structRDA_MessageHeader.nType !=1)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "First Message received is not an header!\n";
		m_rDriverContext.getLogManager() << LogLevel_Error << "Try to reconnect....\n";
		return false;
	}

	// If the header size is larger than the allocated memory, reallocate
	if(l_structRDA_MessageHeader.nSize > m_ui64HeaderSize)
	{
		if(m_pStructRDA_MessageHeader) 
		{
			free(m_pStructRDA_MessageHeader);
		}
		m_pStructRDA_MessageHeader = (RDA_MessageHeader*)malloc(l_structRDA_MessageHeader.nSize);
		if(!m_pStructRDA_MessageHeader)
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "Couldn't allocate " << l_structRDA_MessageHeader.nSize << " bytes of memory\n";
			return false;
		}
		m_ui64HeaderSize = l_structRDA_MessageHeader.nSize;
	}

	// Retrieve rest of data
	memcpy(*(&m_pStructRDA_MessageHeader), &l_structRDA_MessageHeader, sizeof(l_structRDA_MessageHeader));
	m_pcharStructRDA_MessageHeader = (char*)(*(&m_pStructRDA_MessageHeader)) + sizeof(l_structRDA_MessageHeader);
	l_ui32Received=0;
	l_ui32ReqLength = 0;
	l_ui32Result = 0;
	l_ui32Datasize = l_structRDA_MessageHeader.nSize - sizeof(l_structRDA_MessageHeader);
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);

		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}

	m_pStructRDA_MessageStart = NULL;
	m_pStructRDA_MessageStart = (RDA_MessageStart*)m_pStructRDA_MessageHeader;

	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Header received\n";

	// Save Header info into m_oHeader
	//m_oHeader.setExperimentIdentifier();
	//m_oHeader.setExperimentDate();

	//m_oHeader.setSubjectId();
	//m_oHeader.setSubjectName();
	//m_oHeader.setSubjectAge(m_structHeader.subjectAge);
	//m_oHeader.setSubjectGender();

	//m_oHeader.setLaboratoryId();
	//m_oHeader.setLaboratoryName();

	//m_oHeader.setTechnicianId();
	//m_oHeader.setTechnicianName();

	m_oHeader.setChannelCount((uint32)m_pStructRDA_MessageStart->nChannels);

	char* l_pszChannelNames = (char*)m_pStructRDA_MessageStart->dResolutions + (m_pStructRDA_MessageStart->nChannels * sizeof(m_pStructRDA_MessageStart->dResolutions[0]));
	for(uint32 i=0; i<m_oHeader.getChannelCount(); i++)
	{
		m_oHeader.setChannelName(i, l_pszChannelNames);
		m_oHeader.setChannelGain(i, (float32)((m_pStructRDA_MessageStart->dResolutions[i]))); // @note passing this outside makes little sense as this driver already appears to apply the gain before giving the data out
		m_oHeader.setChannelUnits(i, OVTK_UNIT_Volts, OVTK_FACTOR_Micro);
		l_pszChannelNames += strlen(l_pszChannelNames) + 1;
	}

	m_oHeader.setSamplingFrequency((uint32)(1000000/m_pStructRDA_MessageStart->dSamplingInterval)); //dSamplingInterval in microseconds

	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;

	m_pCallback=&rCallback;

	m_ui32IndexIn = 0;
	m_ui32IndexOut = 0;
	m_ui32BuffDataIndex = 0;

	m_ui32MarkerCount =0;

	m_bMarkerWarningGiven = false;

	return true;
}

boolean CDriverBrainProductsBrainVisionRecorder::start(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }
	return true;
}

boolean CDriverBrainProductsBrainVisionRecorder::loop(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return true; }

	DEFINE_GUID(GUID_RDAHeader,
		1129858446, 51606, 19590, uint8(175), uint8(74), uint8(152), uint8(187), uint8(246), uint8(201), uint8(20), uint8(80)
	);

	// Initialize var to receive buffer of data
	RDA_MessageHeader l_structRDA_MessageHeader;
	m_pcharStructRDA_MessageHeader = (char*)&l_structRDA_MessageHeader;
	uint32 l_ui32Received = 0;
	uint32 l_ui32ReqLength = 0;
	uint32 l_ui32Result = 0;
	uint32 l_ui32Datasize = sizeof(l_structRDA_MessageHeader);

	// Receive Header
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);
		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}

	// Check for correct header nType
	if (l_structRDA_MessageHeader.nType == 1)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Message received is a header!\n";
		return false;
	}
	if (l_structRDA_MessageHeader.nType == 3)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Message received is a STOP!\n";
		return false;
	}
	if (l_structRDA_MessageHeader.nType !=4)
	{
		return true;
	}

	// Check for correct header GUID.
	if (!COMPARE_GUID(l_structRDA_MessageHeader.guid, GUID_RDAHeader))
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "GUID received is not correct!\n";
		return false;
	}

	// If the header size is larger than the allocated memory, reallocate
	if(l_structRDA_MessageHeader.nSize > m_ui64HeaderSize)
	{
		if(m_pStructRDA_MessageHeader) 
		{
			free(m_pStructRDA_MessageHeader);
		}
		m_pStructRDA_MessageHeader = (RDA_MessageHeader*)malloc(l_structRDA_MessageHeader.nSize);
		if(!m_pStructRDA_MessageHeader)
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "Couldn't allocate " << l_structRDA_MessageHeader.nSize << " bytes of memory\n";
			return false;
		}
		m_ui64HeaderSize = l_structRDA_MessageHeader.nSize;
	}

	// Retrieve rest of block.
	memcpy(*(&m_pStructRDA_MessageHeader), &l_structRDA_MessageHeader, sizeof(l_structRDA_MessageHeader));
	m_pcharStructRDA_MessageHeader = (char*)(*(&m_pStructRDA_MessageHeader)) + sizeof(l_structRDA_MessageHeader);
	l_ui32Received=0;
	l_ui32ReqLength = 0;
	l_ui32Result = 0;
	l_ui32Datasize = l_structRDA_MessageHeader.nSize - sizeof(l_structRDA_MessageHeader);
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);

		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}
	m_ui32BuffDataIndex++;

	// Put the data into MessageData32 structure
	m_pStructRDA_MessageData32 = NULL;
	m_pStructRDA_MessageData32 = (RDA_MessageData32*)m_pStructRDA_MessageHeader;

	//////////////////////
	//Markers -> stimulations
	if (m_pStructRDA_MessageData32->nMarkers > 0)
	{
// 		if (m_pStructRDA_MessageData32->nMarkers == 0)
// 		{
// 			return true;
// 		}

		RDA_Marker* l_pStructRDA_Marker = (RDA_Marker*)((char*)m_pStructRDA_MessageData32->fData + m_pStructRDA_MessageData32->nPoints * m_oHeader.getChannelCount() * sizeof(m_pStructRDA_MessageData32->fData[0]));

		uint32 l_ui32NumberOfMarkers = 0; 

		std::vector<OpenViBE::uint32> l_vStimulationIdentifier;
		std::vector<OpenViBE::uint64> l_vStimulationDate;
		std::vector<OpenViBE::uint64> l_vStimulationSample;

		for (uint32 i = 0; i < m_pStructRDA_MessageData32->nMarkers; i++)
		{
			char* pszType = l_pStructRDA_Marker->sTypeDesc;
			char* pszDescription = pszType + strlen(pszType) + 1;
			 
			// OpenViBE only supports numeric markers. We assume the description is either a number or R#/S#, where # is a number. Skip the character.
			if(pszDescription[0]=='S' || pszDescription[0]=='R') 
			{
				pszDescription++;
			}

			// Test that the rest looks like an integer
			bool l_bLooksLikeInteger = true;
			char *l_pPtr = pszDescription;
			while(*l_pPtr) 
			{
				if(!isdigit(*l_pPtr))
				{
					l_bLooksLikeInteger = false;
					break;
				}
				l_pPtr++;
			}

			if(l_bLooksLikeInteger)
			{
				const uint32 l_ui32StimulationIdentifier = atoi(pszDescription);

				l_vStimulationIdentifier.push_back(l_ui32StimulationIdentifier);
				l_vStimulationDate.push_back(ITimeArithmetics::sampleCountToTime(m_oHeader.getSamplingFrequency(), l_pStructRDA_Marker->nPosition ));
				l_vStimulationSample.push_back(l_pStructRDA_Marker->nPosition);
				l_ui32NumberOfMarkers++;
			} 
			else 
			{
				if(!m_bMarkerWarningGiven) 
				{
					m_rDriverContext.getLogManager() << LogLevel_Warning << "Unable to parse numeric stimulation from marker '" << pszType << "' : '" << (pszType + strlen(pszType) + 1) << "'.\n";
					m_rDriverContext.getLogManager() << LogLevel_Warning << "OpenViBE supports only numeric stimulations. This and subsequent non-numeric markers will be dropped without further warnings.\n";
					m_bMarkerWarningGiven = true;
				}
			}

			// To find the next marker, we don't use/trust the l_pStructRDA_Marker->nSize field but consider the next marker to start after 
			// the present description ends. This seems to address issues with situations where multiple markers are present. Other implementations
			// also extract the markers in a similar streaming fashion rather than relying on the nSize.
			const uint32 l_ui32DescriptionLength = strlen(pszDescription);

			l_pStructRDA_Marker = (RDA_Marker*)(pszDescription + l_ui32DescriptionLength + 1);
		}

		m_ui32MarkerCount += m_pStructRDA_MessageData32->nMarkers;

		CStimulationSet l_oStimulationSet;
		l_oStimulationSet.setStimulationCount(l_ui32NumberOfMarkers);
		for (uint32 i = 0; i < l_ui32NumberOfMarkers; i++)
		{
			l_oStimulationSet.setStimulationIdentifier(i, OVTK_StimulationId_Label(l_vStimulationIdentifier[i]));
			l_oStimulationSet.setStimulationDate(i, l_vStimulationDate[i]);
			l_oStimulationSet.setStimulationDuration(i, 0);
		}

		m_pCallback->setStimulationSet(l_oStimulationSet);
	}

	// If the sample buffer is too small, grow
	if(m_oHeader.getChannelCount()*(uint32)m_pStructRDA_MessageData32->nPoints > m_ui64SampleSize)
	{
		if(m_pSample)
		{
			delete[] m_pSample;
		}
		m_pSample=new float32[m_oHeader.getChannelCount()*(uint32)m_pStructRDA_MessageData32->nPoints];
		m_ui64SampleSize = m_oHeader.getChannelCount()*(uint32)m_pStructRDA_MessageData32->nPoints;
	}

	for (uint32 i=0; i < m_oHeader.getChannelCount(); i++)
	{
		for (uint32 j=0; j < (uint32)m_pStructRDA_MessageData32->nPoints; j++)
		{
			m_pSample[j + (i*(uint32)m_pStructRDA_MessageData32->nPoints)] = (float32)m_pStructRDA_MessageData32->fData[(m_oHeader.getChannelCount()*j) + i]*m_oHeader.getChannelGain(i);
		}
	}

	// send data

	m_pCallback->setSamples(m_pSample,(uint32)m_pStructRDA_MessageData32->nPoints);
	m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());

	return true;

}

boolean CDriverBrainProductsBrainVisionRecorder::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Connection stopped\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }
	return true;
}

boolean CDriverBrainProductsBrainVisionRecorder::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

	if(m_pStructRDA_MessageHeader) 
	{
		free(m_pStructRDA_MessageHeader);
		m_pStructRDA_MessageHeader = NULL;
		m_ui64HeaderSize = 0;
	}

	m_pcharStructRDA_MessageHeader=NULL;
	m_pStructRDA_MessageStart=NULL;
	m_pStructRDA_MessageStop=NULL;
	m_pStructRDA_MessageData32=NULL;

	if(m_pSample) 
	{
		delete [] m_pSample;
		m_pSample=NULL;
		m_ui64SampleSize = 0;
	}
	m_pCallback=NULL;

	// Cleans up client connection
	m_pConnectionClient->close();
	m_pConnectionClient->release();
	m_pConnectionClient=NULL;
	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Client disconnected\n";

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverBrainProductsBrainVisionRecorder::isConfigurable(void)
{
	return true;
}

boolean CDriverBrainProductsBrainVisionRecorder::configure(void)
{
	CConfigurationNetworkBuilder l_oConfiguration(OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-BrainProducts-BrainVisionRecorder.ui");

	l_oConfiguration.setHostName(m_sServerHostName);
	l_oConfiguration.setHostPort(m_ui32ServerHostPort);

	if(l_oConfiguration.configure(m_oHeader))
	{
		m_sServerHostName=l_oConfiguration.getHostName();
		m_ui32ServerHostPort=l_oConfiguration.getHostPort();

		m_oSettings.save();

		return true;
	}

	return false;
}
