#if defined(TARGET_HAS_ThirdPartyEnobioAPI)

#include "ovasCDriverEnobio3G.h"
#include "ovasCConfigurationEnobio3G.h"
#include <string.h>
#include <toolkit/ovtk_all.h>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

//___________________________________________________________________//
//                                                                   //

CDriverEnobio3G::CDriverEnobio3G(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_oSettings("AcquisitionServer_Driver_Enobio3G", m_rDriverContext.getConfigurationManager())
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
{
	m_oHeader.setSamplingFrequency(_ENOBIO_SAMPLE_RATE_);
	m_oHeader.setChannelCount(32);
	
	// The following class allows saving and loading driver settings from the acquisition server .conf file
	m_oSettings.add("Header", &m_oHeader);
	// To save your custom driver settings, register each variable to the SettingsHelper
	//m_oSettings.add("SettingName", &variable);
	m_oSettings.load();	

	// register consumers for enobio data and enobio status
	// we register for data and status
	m_enobioDevice.registerConsumer(Enobio3G::ENOBIO_DATA, *this);
	//	m_enobioDevice.registerConsumer(Enobio3G::STATUS, this);
	// DONT Get the data from the accelerometer HACK: should ask through config dialog
	m_enobioDevice.activateAccelerometer(false);
	// set sampling rate of enobio device
	m_ui32SampleRate = _ENOBIO_SAMPLE_RATE_;
	// allocate space for m_macAddres
	m_macAddress = new unsigned char[6];
}

CDriverEnobio3G::~CDriverEnobio3G(void)
{
	// Note: Device itself is closed in uninitialize()

	if(m_pSample!=NULL)
	{
		delete m_pSample;
	}
	delete m_macAddress;
}

const char* CDriverEnobio3G::getName(void)
{
	return "Enobio3G";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverEnobio3G::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) return false;

	// open the BT connection to the device
	if(m_enobioDevice.openDevice(m_macAddress))
	{
		m_ui32nChannels = m_enobioDevice.numOfChannels();
	}
	else
	{
		return false;
	}

	m_oHeader.setChannelCount(m_ui32nChannels);
	if(!m_oHeader.isChannelCountSet()||!m_oHeader.isSamplingFrequencySet()) return false;
	
	for(uint32 c=0;c<m_ui32nChannels;c++)
	{
		m_oHeader.setChannelUnits(c, OVTK_UNIT_Volts, OVTK_FACTOR_Micro);
	}

	// Builds up a buffer to store
	// acquired samples. This buffer
	// will be sent to the acquisition
	// server later...
	// number of cycling buffers we will use
	m_ui32nBuffers = 32;
	m_rDriverContext.getLogManager() << LogLevel_Debug << "Need " 
		<< m_ui32nBuffers << " buffers of " 
		<< m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock << " size for " 
		<< m_ui32nChannels << " channels\n";
	m_pSample = new float32*[m_ui32nBuffers];
	for(unsigned int i=0;i<m_ui32nBuffers;++i)
	{
		// each buffer will be of length samplecountpersentblock, defined by the configuration interface
		m_pSample[i]=new float32[m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock];
	}
	m_bNewData = false;

	if(!m_pSample)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Memory allocation error\n";
		delete [] m_pSample;
		m_pSample=NULL;
		return false;
	}
	m_ui32currentBuffer = 0;
	m_ui32lastBufferFilled = 0;
	m_ui32bufHead = 0;

	// ...
	// initialize hardware and get
	// available header information
	// from it
	// Using for example the connection ID provided by the configuration (m_ui32ConnectionID)
	// ...

	// Saves parameters
	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;

	return true;

}

boolean CDriverEnobio3G::start(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;

	// ...
	// request hardware to start
	// sending data
	// ...
	// tell Enobio device to start streaming EEG data
	m_enobioDevice.startStreaming();

	return true;
}

/**
	Running loop used by OV engine to query for new data from device
*/
boolean CDriverEnobio3G::loop(void) {
	if(!m_rDriverContext.isConnected()) return false;
	if(!m_rDriverContext.isStarted()) return true;

	// query new data flag state
	OpenViBE::boolean l_bNewData;
	{
		boost::mutex::scoped_lock lock(m_oMutex);
		l_bNewData = m_bNewData;
	}



	// if new data flag is raised it means there's a buffer with new data ready to be submitted
	if(l_bNewData)
	{
		// submit new data on the buffer pointed by the lastbufferfilled variable
		m_pCallback->setSamples(m_pSample[m_ui32lastBufferFilled]);
		// lower new data flag
		{
			boost::mutex::scoped_lock lock(m_oMutex);
			m_bNewData = false;
		}
		// When your sample buffer is fully loaded, 
		// it is advised to ask the acquisition server 
		// to correct any drift in the acquisition automatically.
		m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());
	}


	// ...
	// receive events from hardware
	// and put them the correct way in a CStimulationSet object
	//...
	//	m_pCallback->setStimulationSet(l_oStimulationSet);


	return true;
}

boolean CDriverEnobio3G::stop(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(!m_rDriverContext.isStarted()) return false;

	// ...
	// request the hardware to stop
	// sending data
	// Tell Enobio device to stop streaming EEG. 
	m_enobioDevice.stopStreaming();

	return true;
}

boolean CDriverEnobio3G::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;

	// ...
	// uninitialize hardware here
	// ...

	if(m_pSample)
	{
		for(unsigned int i=0;i<m_ui32nBuffers;++i)
		{
			delete m_pSample[i];
		}
		delete [] m_pSample;
		m_pSample=NULL;
	}

	m_pCallback=NULL;
	// close BT connection with the Enobio device
	m_enobioDevice.closeDevice();

	return true;
}

//___________________________________________________________________//
//                                                                   //
boolean CDriverEnobio3G::isConfigurable(void)
{
	return true; // change to false if your device is not configurable
}

boolean CDriverEnobio3G::configure(void)
{
	// Change this line if you need to specify some references to your driver attribute that need configuration, e.g. the connection ID.
  	CConfigurationEnobio3G m_oConfiguration(m_rDriverContext, OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-Enobio3G.ui");

	if(!m_oConfiguration.configure(m_oHeader))
	{
		return false;
	}
	m_oSettings.save();
	unsigned char *l_macAddress = m_oConfiguration.getMacAddress();
	for(int i=0;i<6;++i)
	{
		m_macAddress[i] = (unsigned char)*(l_macAddress+i);
	}

	return true;
}
/**
	Callback function that will be called by the Enobio API for each sample received from the device. 
*/
void CDriverEnobio3G::receiveData(const PData &data){
  ChannelData *_receivedData = (ChannelData*)data.getData();
  // We'll need to iterate through channels instead of memcpy because we need 
  // to cast from int to float. will also convert to microvolts

	int *samples;
	samples = _receivedData->data();
	OpenViBE::float64 l_sample;
	for(unsigned int i=0;i<m_ui32nChannels;i++)
	{
		l_sample = samples[i]/1000.0;
		m_pSample[m_ui32currentBuffer][i*m_ui32SampleCountPerSentBlock+m_ui32bufHead] = static_cast<float32>(l_sample);
	}

	// mutex for writing header and new data flag
	{
		boost::mutex::scoped_lock lock(m_oMutex);

		m_ui32bufHead++;
		// if we already filled the current buffer we need to raise the new data flag
		// and cycle to the next buffer
		if(m_ui32bufHead>=m_ui32SampleCountPerSentBlock)
		{
			// update the pointer to the last buffer filled
			m_ui32lastBufferFilled = m_ui32currentBuffer;
			// reset the buffer writing head to the beginning. 
			m_ui32bufHead=0;
			// we update pointer to the current buffer
			m_ui32currentBuffer++;
			// if we are at the end of the buffers set, cycle to the first one
			if(m_ui32currentBuffer>=m_ui32nBuffers)
			{
				m_ui32currentBuffer=0;
			}
			// raise the flag to mark existence of new data to be submittted
			m_bNewData = true;
		}
	}

}

/**
	Callback from EnobioAPI to receive status data. 
	CURRENTLY NOT USED
*/
void CDriverEnobio3G::newStatusFromDevice(const PData &data){
}




#endif
