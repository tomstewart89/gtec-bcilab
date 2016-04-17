#if defined TARGET_HAS_ThirdPartyGUSBampCAPI_Linux

#include "ovasCDriverGTecGUSBampLinux.h"
#include "ovasCConfigurationGTecGUSBampLinux.h"

#include <toolkit/ovtk_all.h>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

//___________________________________________________________________//
//                                                                   //

CDriverGTecGUSBampLinux::CDriverGTecGUSBampLinux(IDriverContext& rDriverContext) :
    IDriver(rDriverContext),
    m_oSettings("AcquisitionServer_Driver_GTecGUSBampLinux", m_rDriverContext.getConfigurationManager()),
    m_pCallback(NULL),
    m_ui32SampleCountPerSentBlock(0),
    m_pSampleSend(NULL),
    m_pSampleReceive(NULL),
    m_pSampleBuffer(NULL),
    m_ui32CurrentSample(0),
    m_ui32CurrentChannel(0)
{
    m_oHeader.setSamplingFrequency(512);
    m_oHeader.setChannelCount(16);

    m_oConfig.ao_config = &m_oAnalogOutConfig;

    // Configure some defaults so the settings are reasonable as soon as the driver loads and the user can tweak them from there

    // Configure the analog waveform to be created by the internal signal generator
    m_oAnalogOutConfig.shape = GT_ANALOGOUT_SINE;
    m_oAnalogOutConfig.frequency = 1;
    m_oAnalogOutConfig.amplitude = 0;
    m_oAnalogOutConfig.offset = 0;

    // Set the sampling rate
    m_oConfig.sample_rate = 512;

    // This pretty much has to be GT_NOS_AUTOSET, don't know why, so says the documentation
    m_oConfig.number_of_scans = GT_NOS_AUTOSET;
    // Disable the trigger line, digital io scan, slave mode and the shortcut
    m_oConfig.enable_trigger_line = m_oConfig.scan_dio = m_oConfig.slave_mode = m_oConfig.enable_sc = GT_FALSE;

    // Set the mode to just take readings
    m_oConfig.mode = GT_MODE_NORMAL;

    // Number of channels to read from
    m_oConfig.num_analog_in = m_oHeader.getChannelCount();

    // Set all the blocks A-D to use the common ground and reference voltages
    for (unsigned int i = 0; i < GT_USBAMP_NUM_GROUND; i++)
    {
        m_oConfig.common_ground[i] = GT_TRUE;
        m_oConfig.common_reference[i] = GT_TRUE;
    }

    // Configure each input
    for (unsigned char i = 0; i < m_oConfig.num_analog_in; i++)
    {
        // Should be from 1 - 16, specifies which channel to observe as input i
        m_oConfig.analog_in_channel[i] = i + 1;
        // Don't use any of the filters on channel i
        m_oConfig.bandpass[i] = GT_FILTER_NONE;
        // Don't use any of the notch filters on channel i
        m_oConfig.notch[i] =  GT_FILTER_NONE;
        // Don't use any of the other channels for bi-polar derivation
        m_oConfig.bipolar[i] = GT_BIPOLAR_DERIVATION_NONE;
    }

    // Now look for any connected devices. If any exist we'll set the name to the first one found
    char** l_pDeviceList = 0;
    size_t l_ui32ListSize = 0;

    // Refresh and get the list of currently connnected devices
    GT_UpdateDevices();
    l_ui32ListSize = GT_GetDeviceListSize();
    l_pDeviceList = GT_GetDeviceList();

    // If any devices were found at all, set the combo box to the first one listed
    if(l_ui32ListSize)
    {
        m_oDeviceName = l_pDeviceList[0];
    }

    GT_FreeDeviceList(l_pDeviceList,l_ui32ListSize);

    // Now retrieve all those configs from the settings file if they are there to be found (don't need to worry about sample rate or channel number though since they're already in the header)
    m_oSettings.add("DeviceName", (string*)&m_oDeviceName);
    m_oSettings.add("Mode", (int*)&m_oConfig.mode);
    m_oSettings.add("EnableTrigger", (bool*)&m_oConfig.enable_trigger_line);
    m_oSettings.add("ScanDIO", (bool*)&m_oConfig.scan_dio);
    m_oSettings.add("SlaveMode", (bool*)&m_oConfig.slave_mode);
    m_oSettings.add("EnableShortcut", (bool*)&m_oConfig.enable_sc);
    m_oSettings.add("AnalogOutShape", (int*)&m_oAnalogOutConfig.shape);
    m_oSettings.add("AnalogOutFrequency", (int*)&m_oAnalogOutConfig.frequency);
    m_oSettings.add("AnalogOutAmplitude", (int*)&m_oAnalogOutConfig.amplitude);
    m_oSettings.add("AnalogOutOffset", (int*)&m_oAnalogOutConfig.offset);

    // Set all the blocks A-D to use the common ground and reference voltages
    for (unsigned int i = 0; i < GT_USBAMP_NUM_GROUND; i++)
    {
        stringstream l_oGndConfigName, l_oRefConfigName;
        l_oGndConfigName << "CommonGround" << i;
        l_oRefConfigName << "CommonReference" << i;
        m_oSettings.add(l_oGndConfigName.str().c_str(), (bool*)&m_oConfig.common_ground[i]);
        m_oSettings.add(l_oRefConfigName.str().c_str(), (bool*)&m_oConfig.common_reference[i]);
    }

    // Configure each input
    for (unsigned int i = 0; i < m_oConfig.num_analog_in; i++)
    {
        stringstream l_oBandpassConfigName, l_oNotchConfigName, l_oBipolarConfigName;
        l_oBandpassConfigName << "Bandpass" << i;
        l_oNotchConfigName << "Notch" << i;
        l_oBipolarConfigName << "Bipolar" << i;
        m_oSettings.add(l_oBandpassConfigName.str().c_str(), (int*)&m_oConfig.bandpass[i]);
        m_oSettings.add(l_oNotchConfigName.str().c_str(), (int*)&m_oConfig.notch[i]);
        m_oSettings.add(l_oBipolarConfigName.str().c_str(), (int*)&m_oConfig.bipolar[i]);
    }

    m_oSettings.load();   
}

CDriverGTecGUSBampLinux::~CDriverGTecGUSBampLinux(void)
{
}

const char* CDriverGTecGUSBampLinux::getName(void)
{
    return "g.tec g.USBamp for Linux";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverGTecGUSBampLinux::initialize(const uint32 ui32SampleCountPerSentBlock, IDriverCallback& rCallback)
{
    if(m_rDriverContext.isConnected()) return false;
    if(!m_oHeader.isChannelCountSet()||!m_oHeader.isSamplingFrequencySet()) return false;

    // If the scan digital inputs flag is set, the API will return one extra channel outside of the analog data requested, so we need to match that on the header
    if(m_oConfig.scan_dio == GT_TRUE)
    {
        m_oHeader.setChannelCount(m_oHeader.getChannelCount() + 1);
    }

    // Allocate buffers for...

    // Sending to OpenViBE
    m_pSampleSend = new float32[m_oHeader.getChannelCount() * ui32SampleCountPerSentBlock];
    // Receiving from the hardware,
    m_pSampleReceive = new float32[m_oHeader.getChannelCount() * ui32SampleCountPerSentBlock];
    // Storing the data so we pass it between the two threads - we're using the recommended buffer size put out by gtec, which is enormous
    m_pSampleBuffer = new float32[GT_USBAMP_RECOMMENDED_BUFFER_SIZE / sizeof(float)];

    // Set up the queue to help pass the data out of the hardware thread
    m_oSampleQueue.SetBuffer(m_pSampleBuffer,m_oHeader.getChannelCount() * m_oHeader.getSamplingFrequency() / 8);

    // If any of that allocation fails then give up. Not sure what setting it all to NULL is for, but we'll go with it.
    if(!m_pSampleSend || !m_pSampleReceive || !m_pSampleBuffer)
    {
        delete[] m_pSampleSend;
        delete[] m_pSampleReceive;
        delete[] m_pSampleBuffer;
        m_pSampleSend = m_pSampleReceive = m_pSampleBuffer = NULL;

        return false;
    }

    // Apparently this causes the API to print debug info to the console, I'm yet to see any though
    GT_ShowDebugInformation(GT_TRUE);

    // Try to open the device with the configured name, let the user know how it goes
    if(!GT_OpenDevice(m_oDeviceName.c_str()))
    {
        m_rDriverContext.getLogManager() << LogLevel_Error << "Could not open device: " << (OpenViBE::CString)m_oDeviceName.c_str() << "\n";
        return false;
    }

    if(!GT_SetConfiguration(m_oDeviceName.c_str(), &m_oConfig))
    {
        m_rDriverContext.getLogManager() << LogLevel_Error << "Could not apply configuration to device: " << (OpenViBE::CString)m_oDeviceName.c_str() << "\n";
        return false;
    }

    GT_SetDataReadyCallBack(m_oDeviceName.c_str(), &OnDataReady, (void*)(this));

    // Saves parameters
    m_pCallback = &rCallback;
    m_ui32SampleCountPerSentBlock = ui32SampleCountPerSentBlock;

    return true;
}

boolean CDriverGTecGUSBampLinux::start(void)
{
    if(!m_rDriverContext.isConnected()) return false;
    if(m_rDriverContext.isStarted()) return false;

    // ...
    // request hardware to start
    // sending data
    // ...

    // Need to reset these in case the device is stopped mid-sample and then started again
    m_ui32CurrentChannel = m_ui32CurrentSample = 0;

    GT_StartAcquisition(m_oDeviceName.c_str());

    m_rDriverContext.getLogManager() << LogLevel_Info << "Acquisition Started\n";

    return true;
}

// So when the gtec buffer grows larger than a send buffer, copy it all to a send buffer sized array, then copy it into the actual send buffer one by one.
boolean CDriverGTecGUSBampLinux::loop(void)
{
    if(!m_rDriverContext.isConnected()) return false;
    if(!m_rDriverContext.isStarted()) return true;

    OpenViBE::CStimulationSet l_oStimulationSet;

    // while there's new data available on the queue
    while(m_oSampleQueue.Avail())
    {
        // take it off and put it in the appropriate element in the outgoing buffer
        m_oSampleQueue.Get(m_pSampleSend + m_ui32CurrentChannel * m_ui32SampleCountPerSentBlock + m_ui32CurrentSample,1);

        // Increment the current channel
        m_ui32CurrentChannel++;

        // If the current channel reaches the channel count then move to the next sample
        if(m_ui32CurrentChannel == m_oHeader.getChannelCount())
        {
            m_ui32CurrentChannel = 0;
            m_ui32CurrentSample++;
        }

        // If the sample count reaches the number per sent block, then send it and start again
        if(m_ui32CurrentSample == m_ui32SampleCountPerSentBlock)
        {
            m_pCallback->setSamples(m_pSampleSend); // it looks as if this copies the buffer, so we're free modify it as soon as it executes

            // When your sample buffer is fully loaded,
            // it is advised to ask the acquisition server
            // to correct any drift in the acquisition automatically.
            m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());

            // ...
            // receive events from hardware
            // and put them the correct way in a CStimulationSet object
            //...
            m_pCallback->setStimulationSet(l_oStimulationSet);

            m_ui32CurrentSample = 0;
        }
    }

    return true;
}

boolean CDriverGTecGUSBampLinux::stop(void)
{
    if(!m_rDriverContext.isConnected()) return false;
    if(!m_rDriverContext.isStarted()) return false;

    // ...
    // request the hardware to stop
    // sending data
    // ...
    GT_StopAcquisition(m_oDeviceName.c_str());
    m_rDriverContext.getLogManager() << LogLevel_Info << "Acquisition Stopped";

    return true;
}

boolean CDriverGTecGUSBampLinux::uninitialize(void)
{
    if(!m_rDriverContext.isConnected()) return false;
    if(m_rDriverContext.isStarted()) return false;

    GT_CloseDevice(m_oDeviceName.c_str());

    m_rDriverContext.getLogManager() << LogLevel_Info << "Closed Device: " << (OpenViBE::CString)m_oDeviceName.c_str() << "\n";

    // ...
    // uninitialize hardware here
    // ...
    m_oSampleQueue.SetBuffer(NULL, 0);

    delete[] m_pSampleSend;
    delete[] m_pSampleBuffer;
    delete[] m_pSampleReceive;

    m_pSampleSend = m_pSampleReceive = m_pSampleBuffer = NULL;

    m_pCallback = NULL;

    return true;
}

//___________________________________________________________________//
//                                                                   //
boolean CDriverGTecGUSBampLinux::isConfigurable(void)
{
    return true; // change to false if your device is not configurable
}

boolean CDriverGTecGUSBampLinux::configure(void)
{
    // Change this line if you need to specify some references to your driver attribute that need configuration, e.g. the connection ID.
    CConfigurationGTecGUSBampLinux m_oConfiguration(m_rDriverContext, OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-GTecGUSBampLinux.ui", &m_oDeviceName, &m_oConfig);

    if(!m_oConfiguration.configure(m_oHeader))
    {
        return false;
    }

    m_oSettings.save();

    return true;
}

/*void OpenViBEAcquisitionServer::OnDataReady(void *param)
{
    // Like the 'this' pointer, but for a friend function
    CDriverGTecGUSBampLinux *that = (CDriverGTecGUSBampLinux*)param;

    // This is pretty tricky to know in advance, the API decides how many values to spit out depnding on a few factors it seems.
    // We'll allocate a reasonble buffer and call GT_GetData as many times as is necessary
    while(size_t l_ui32SamplesToRead = GT_GetSamplesAvailable(that->m_oDeviceName.c_str()))
    {
        // If there are more samples than will fit in the buffer, just get as many as possible and we can get the rest next iteration
        if(l_ui32SamplesToRead > CDriverGTecGUSBampLinux::ReceiveBufferSize * sizeof(OpenViBE::float32))
            l_ui32SamplesToRead = CDriverGTecGUSBampLinux::ReceiveBufferSize * sizeof(OpenViBE::float32);

        // Get the data -- TODO: rewrite this algorithm such that we can copy directly from GT_GetData into the buffer read in the loop() function - is this a bug?? Maybe, but probably not since the calibration mode was always perfect
        GT_GetData(that->m_oDeviceName.c_str(), reinterpret_cast<unsigned char*>(that->m_pSampleReceive), l_ui32SamplesToRead);

        // Put it on the sample queue
        that->m_oSampleQueue.Put(that->m_pSampleReceive, l_ui32SamplesToRead / sizeof(OpenViBE::float32));
    }
}*/

void OpenViBEAcquisitionServer::OnDataReady(void *param)
{
    // Like the 'this' pointer, but for a friend function
    CDriverGTecGUSBampLinux *that = (CDriverGTecGUSBampLinux*)param;

    // This is pretty tricky to know in advance, the API decides how many values to spit out depnding on a few factors it seems.
    // We'll allocate a reasonble buffer and call GT_GetData as many times as is necessary
    while(size_t l_ui32SamplesToRead = GT_GetSamplesAvailable(that->m_oDeviceName.c_str()))
    {
        // If there are more samples than will fit in the buffer, just get as many as possible and we can get the rest next iteration
        if(l_ui32SamplesToRead > that->m_oSampleQueue.FreeContiguous() * sizeof(OpenViBE::float32))
            l_ui32SamplesToRead = that->m_oSampleQueue.FreeContiguous() * sizeof(OpenViBE::float32);

		// Get the data and put it directly onto the queue
        GT_GetData(that->m_oDeviceName.c_str(), reinterpret_cast<unsigned char*>(that->m_oSampleQueue.NextFreeAddress()), l_ui32SamplesToRead);

		// Pad the queue so it recognises how much data was just added to it
        that->m_oSampleQueue.Pad(l_ui32SamplesToRead / sizeof(OpenViBE::float32));
    }
}

#endif // TARGET_HAS_ThirdPartyGUSBampCAPI_Linux
