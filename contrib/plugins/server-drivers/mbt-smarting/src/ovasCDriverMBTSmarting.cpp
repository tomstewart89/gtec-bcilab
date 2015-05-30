#include "ovasCDriverMBTSmarting.h"
#include "ovasCConfigurationMBTSmarting.h"

#include <toolkit/ovtk_all.h>

#include <string>
#include <sstream>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

//___________________________________________________________________//
//                                                                   //

CDriverMBTSmarting::CDriverMBTSmarting(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_oSettings("AcquisitionServer_Driver_MBTSmarting", m_rDriverContext.getConfigurationManager())
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
{
	m_oHeader.setSamplingFrequency(500);
	m_oHeader.setChannelCount(27);
	
	// The following class allows saving and loading driver settings from the acquisition server .conf file
	m_oSettings.add("Header", &m_oHeader);
	// To save your custom driver settings, register each variable to the SettingsHelper
	//m_oSettings.add("SettingName", &variable);
	m_oSettings.load();	
}

CDriverMBTSmarting::~CDriverMBTSmarting(void)
{
}

const char* CDriverMBTSmarting::getName(void)
{
	return "mBrainTrain Smarting";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverMBTSmarting::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) return false;
	if(!m_oHeader.isChannelCountSet()||!m_oHeader.isSamplingFrequencySet()) return false;
	
	// Builds up a buffer to store
	// acquired samples. This buffer
	// will be sent to the acquisition
	// server later...
	m_pSample=new float32[m_oHeader.getChannelCount()];
	if(!m_pSample)
	{
		delete [] m_pSample;
		m_pSample=NULL;
		return false;
	}
	
	// ...
	// initialize hardware and get
	// available header information
	// from it
	// Using for example the connection ID provided by the configuration (m_ui32ConnectionID)
	// ...
	
	m_pSmartingAmp.reset(new SmartingAmp);
	
	stringstream port_ss;
	#ifdef TARGET_OS_Windows
		port_ss << "COM" << m_ui32ConnectionID;
	#elif defined TARGET_OS_Linux
		port_ss << "/dev/rfcomm" << m_ui32ConnectionID;
	#endif

	m_rDriverContext.getLogManager() << LogLevel_Info << "Attempting to Connect to Device at : " << port_ss.str().c_str() <<"\n";

	string port(port_ss.str().c_str());
	bool connected = m_pSmartingAmp->connect(port);
	if(connected)
	{
		// set sampling frequency
		switch(m_oHeader.getSamplingFrequency())
		{
		case 250:
			m_pSmartingAmp->send_command(FREQUENCY_250);
			m_rDriverContext.getLogManager() << LogLevel_Info << "Setting the sampling frequency at " << 250 <<"\n";
			break;
		case 500:
			m_rDriverContext.getLogManager() << LogLevel_Info << "Setting the sampling frequency at " << 500 <<"\n";
			m_pSmartingAmp->send_command(FREQUENCY_500);
			break;
		}

		// Declare channel units
		for(uint32 c=0;c<24;c++) 
		{
			m_oHeader.setChannelUnits(c, OVTK_UNIT_Volts, OVTK_FACTOR_Micro);         // signal channels
		}
		m_oHeader.setChannelUnits(24, OVTK_UNIT_Degree_Per_Second, OVTK_FACTOR_Base); // gyroscope outputs
		m_oHeader.setChannelUnits(25, OVTK_UNIT_Degree_Per_Second, OVTK_FACTOR_Base);
		m_oHeader.setChannelUnits(26, OVTK_UNIT_Degree_Per_Second, OVTK_FACTOR_Base);

		// Saves parameters
		m_pCallback=&rCallback;
		m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;

		return true;
	}

	return false;
}

boolean CDriverMBTSmarting::start(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;

	// ...
	// request hardware to start
	// sending data
	// ...

	m_pSmartingAmp->send_command(ON);
	m_byteArray.clear();
	
	sample_number = 1;
	latency = 1;

	return true;
}

boolean CDriverMBTSmarting::loop(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(!m_rDriverContext.isStarted()) return true;

	OpenViBE::CStimulationSet l_oStimulationSet;

	// ...
	// receive samples from hardware
	// put them the correct way in the sample array
	// whether the buffer is full, send it to the acquisition server
	//...

	#ifdef TARGET_OS_Windows
		HANDLE current_thread =  GetCurrentThread();
		SetThreadPriority(current_thread, THREAD_PRIORITY_HIGHEST);
	#endif

	unsigned char* receiveBuffer = new unsigned char[MAX_PACKAGE_SIZE];

	int readed = m_pSmartingAmp->read(receiveBuffer, MAX_PACKAGE_SIZE);

	for (int i = 0; i < readed; i++)
	{
		if (m_byteArray.size() > 0)
		{
			m_byteArray.push_back(receiveBuffer[i]);
			if(m_byteArray.size() == 83)
			{
				if(m_byteArray[82] == '<')
				{
					if (sample_number % 5000 == 0)
					{
						sample_number = 1;
						
						if (m_rDriverContext.getDriftSampleCount() < 2)
						{
							m_pSample = m_pSmartingAmp->convert_data(m_byteArray);
							m_pCallback->setSamples(m_pSample, 1);
							m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());
						}
					}
					else
					{
						sample_number++;

						m_pSample = m_pSmartingAmp->convert_data(m_byteArray);
						m_pCallback->setSamples(m_pSample, 1);
						m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());
					
					}
	
				}
				
				m_byteArray.clear();
				
			}
		}

		if(m_byteArray.size() == 0 && receiveBuffer[i] == '>')
		{
			m_byteArray.push_back(receiveBuffer[i]);
		}
	}

	if (latency == 300)
	{
		latency = 0;
		m_rDriverContext.setInnerLatencySampleCount(-m_rDriverContext.getDriftSampleCount());
	}
	else
	{
		if (latency != 0)
		{
			latency++;
		}
	}

	// ...
	// receive events from hardware
	// and put them the correct way in a CStimulationSet object
	//...
	m_pCallback->setStimulationSet(l_oStimulationSet);

	return true;
}

boolean CDriverMBTSmarting::stop(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(!m_rDriverContext.isStarted()) return false;

	// ...
	// request the hardware to stop
	// sending data
	// ...
	m_rDriverContext.setInnerLatencySampleCount(0);
	m_pSmartingAmp->off();

	return true;
}

boolean CDriverMBTSmarting::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;

	// ...
	// uninitialize hardware here
	// ...

	m_pSmartingAmp->disconnect();

	delete [] m_pSample;
	m_pSample=NULL;
	m_pCallback=NULL;

	return true;
}

//___________________________________________________________________//
//                                                                   //
boolean CDriverMBTSmarting::isConfigurable(void)
{
	return true; // change to false if your device is not configurable
}

boolean CDriverMBTSmarting::configure(void)
{
	// Change this line if you need to specify some references to your driver attribute that need configuration, e.g. the connection ID.
	CConfigurationMBTSmarting m_oConfiguration(m_rDriverContext, OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-MBTSmarting.ui", m_ui32ConnectionID);
	
	if(!m_oConfiguration.configure(m_oHeader))
	{
		return false;
	}
	m_oSettings.save();
	
	return true;
}
