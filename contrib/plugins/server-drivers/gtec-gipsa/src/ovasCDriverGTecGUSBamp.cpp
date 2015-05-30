#if defined TARGET_HAS_ThirdPartyGUSBampCAPI

/*
 *
 * Notes: According to gtec C API V3.12.00, the output from this driver
 * should be microvolts, except when giving out a calibration signal.
 *
 * The auto calibration should be run before actual acquisition to ensure 
 * that all the channels are scaled appropriately.
 *
 */

#include "ovasCDriverGTecGUSBamp.h"
#include "ovasCConfigurationGTecGUSBamp.h"

#include <toolkit/ovtk_all.h>

#include <system/ovCTime.h>
#include <cmath>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <limits>

#include <gUSBamp.h>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

#if defined(TARGET_OS_Windows)
#pragma warning(disable: 4800) // disable "forcing value to bool 'true' or 'false' (performance warning)" nag coming from BOOL->bool cast on e.g. VS2010
#endif

const DWORD CDriverGTecGUSBamp::bufferSizeBytes = HEADER_SIZE + nPoints * sizeof(float);

/*
	This driver always reads 17 channels: 16 + 1
	16 are EEG channels
	1 is the last channel that provides triggers from the parallel port of the GTEC
	Although 17 channels are read only "m_ui32AcquiredChannelCount" + 1 (if m_pTriggerInputEnabled==true) are displayed.
	If m_ui32AcquiredChannelCount=6 and m_pTriggerInputEnabled=true then the output in OpenVibe is 7 channels. If m_pTriggerInputEnabled=false then 6.
	"m_ui32AcquiredChannelCount" is a user modifiable variable with default value 16
*/

CDriverGTecGUSBamp::CDriverGTecGUSBamp(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_oSettings("AcquisitionServer_Driver_GTecGUSBamp", m_rDriverContext.getConfigurationManager())
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,m_ui32DeviceIndex(uint32(-1))
	,m_pSample(NULL)
	,m_ui8CommonGndAndRefBitmap(0)
	,m_i32NotchFilterIndex(-1)
	,m_i32BandPassFilterIndex(-1)
	,m_bTriggerInputEnabled(false)
	,m_ui32AcquiredChannelCount(GTEC_NUM_CHANNELS)
	,m_flagIsFirstLoop(true)
	,m_masterSerial("")
	,m_bBipolarEnabled(false)
	,m_bReconfigurationRequired(false)
	,m_bCalibrationSignalEnabled(false)
{
	m_oHeader.setSamplingFrequency(512);
	m_oHeader.setChannelCount(GTEC_NUM_CHANNELS);	

	m_oSettings.add("Header", &m_oHeader);
	m_oSettings.add("DeviceIndex", &m_ui32DeviceIndex);
	m_oSettings.add("CommonGndAndRefBitmap", &m_ui8CommonGndAndRefBitmap);
	m_oSettings.add("NotchFilterIndex", &m_i32NotchFilterIndex);
	m_oSettings.add("BandPassFilterIndex", &m_i32BandPassFilterIndex);
	m_oSettings.add("TriggerInputEnabled", &m_bTriggerInputEnabled);
	m_oSettings.add("DeviceSerials", &m_vDevicesSerials);
	m_oSettings.add("MasterSerial", &m_masterSerial);
	m_oSettings.add("Bipolar", &m_bBipolarEnabled);
	m_oSettings.add("CalibrationSignal", &m_bCalibrationSignalEnabled);
	m_oSettings.load();

	m_ui32AcquiredChannelCount = m_oHeader.getChannelCount();	

}

void CDriverGTecGUSBamp::release(void)
{
	delete this;
}

const char* CDriverGTecGUSBamp::getName(void)
{
	return "g.Tec gUSBamp Gipsa-lab";
}

//___________________________________________________________________//
//                                                                   //

OpenViBE::boolean CDriverGTecGUSBamp::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) return false;

	if(!m_oHeader.isChannelCountSet()
	 ||!m_oHeader.isSamplingFrequencySet())
	{
		return false;
	}                         
	
	m_callSequenceHandles.clear();
	numDevices = 0;

	detectDevices();

	if (numDevices==0) return false; 
	
	//assign automatically the last device as master if no master has been selected from "Device properties" before that
	if (numDevices>1 && m_masterSerial=="")
	{
		char serial[16];
		::GT_GetSerial(m_callSequenceHandles[numDevices-1], serial, 16);
		m_masterSerial = serial;
	}

	m_rDriverContext.getLogManager() << LogLevel_Info << "Number of devices: " << numDevices << "\n";

	//m_bTriggerInputEnabled = true; //for testing purposes

	//add the trigger channel
	if (m_bTriggerInputEnabled)
	{
		m_oHeader.setChannelCount((m_ui32AcquiredChannelCount+1)*numDevices);
	}
	else
	{
		m_oHeader.setChannelCount(m_ui32AcquiredChannelCount*numDevices);
	}

	m_ui32ActualImpedanceIndex=0;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
	m_pCallback=&rCallback;

	//create the temporary data buffers (the device will write data into those)
	validPoints = NUMBER_OF_SCANS * (GTEC_NUM_CHANNELS + 1) * numDevices;
	m_buffers = new BYTE**[numDevices];
	m_overlapped = new OVERLAPPED*[numDevices];
	m_pSample = new float[nPoints * numDevices]; //needed later when data is being acquired
	m_bufferReceivedData = new float[validPoints];

	m_RingBuffer.Initialize(BUFFER_SIZE_SECONDS * m_oHeader.getSamplingFrequency() * (GTEC_NUM_CHANNELS + 1) * numDevices );

	for (uint32 i=0;i<numDevices;i++)
	{
		if (m_bTriggerInputEnabled) 
		{
			int channelIndex = i*(m_ui32AcquiredChannelCount+1)+m_ui32AcquiredChannelCount;
			string str = "CH_Event";
			if (numDevices>1) //It will set event channels CH_Event0,1,2 and if 1 device then it will be just "CH_Event"
			{
				string number = static_cast<ostringstream*>( &(ostringstream() << i) )->str();
				str = str.append(number);
			}
			m_oHeader.setChannelName(channelIndex,str.c_str());
			m_rDriverContext.getLogManager() << LogLevel_Trace << "Channel name: " << m_oHeader.getChannelName(channelIndex) << "\n";
		}

		//Configure each device
		ConfigureDevice(i);
	}
	
	//Set Master and slaves
	if (numDevices>1)
	{
	    m_rDriverContext.getLogManager() << LogLevel_Warning << "Please configure the sync cabel according to the above master/slave configuration or set your master from \"Device properties\"\n";
	    setMasterDevice(m_masterSerial);
    }
	else if (numDevices == 1) //a single device must be Master
	{
		if(::GT_SetSlave(m_callSequenceHandles[0], false)) 
		{
			char serial[16];
		    ::GT_GetSerial(m_callSequenceHandles[0], serial, 16);
		    m_masterSerial = serial;
			m_rDriverContext.getLogManager() << LogLevel_Info << "Configured as MASTER device: " << m_masterSerial.c_str() << " \n";
			m_mastersCnt++;
		}
		else m_rDriverContext.getLogManager() << LogLevel_Error << "Unexpected error while calling GT_SetSlave\n";
	}
	
	// Set channel units
	for(uint32 c=0;c<m_oHeader.getChannelCount(); c++) 
	{
		if(!m_bCalibrationSignalEnabled)
		{
			m_oHeader.setChannelUnits(c, OVTK_UNIT_Volts, OVTK_FACTOR_Micro);
		} 
		else
		{
			// For calibration, the outputs are before-scaling raw values from the unit
			// following formula 'normal output = (raw-offset)*factor'. In normal use, 
			// offset and factor are obtained and set by the calibration procedure.
			m_oHeader.setChannelUnits(c, OVTK_UNIT_Unspecified, OVTK_FACTOR_Base);
		}
	}

	return true;
}

void CDriverGTecGUSBamp::detectDevices()
{
	m_callSequenceHandles.clear();
	m_vDevicesSerials.clear();

	int i=0;
	char serial[16];

	while(i < 11)
	{
		::HANDLE l_pHandle=::GT_OpenDevice(i);
		if(l_pHandle)
		{
			m_callSequenceHandles.push_back(l_pHandle);

			::GT_GetSerial(l_pHandle, serial, 16);
			m_rDriverContext.getLogManager() << LogLevel_Info << "Detected device with serial: " << serial << "\n";
			
			m_vDevicesSerials.push_back(serial);
		}
		i++;
	}

	numDevices = m_callSequenceHandles.size();
}

OpenViBE::boolean CDriverGTecGUSBamp::ConfigureDevice(OpenViBE::uint32 deviceNumber)
{
	HANDLE o_pDevice = m_callSequenceHandles[deviceNumber];

	char current_serial[16];
	::GT_GetSerial(o_pDevice, current_serial, 16);
	::UCHAR l_oChannel[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

    // The amplifier is divided in 4 blocks, A to D
    // each one has its own Ref/gnd connections,
    // user can specify whether or not to connect the block to the common ground and reference of the amplifier.
    ::GND l_oGround;
    l_oGround.GND1=(m_ui8CommonGndAndRefBitmap&1);
    l_oGround.GND2=(m_ui8CommonGndAndRefBitmap&(1<<1));
    l_oGround.GND3=(m_ui8CommonGndAndRefBitmap&(1<<2));
    l_oGround.GND4=(m_ui8CommonGndAndRefBitmap&(1<<3));

    ::REF l_oReference;
    l_oReference.ref1=(m_ui8CommonGndAndRefBitmap&(1<<4));
    l_oReference.ref2=(m_ui8CommonGndAndRefBitmap&(1<<5));
    l_oReference.ref3=(m_ui8CommonGndAndRefBitmap&(1<<6));
    l_oReference.ref4=(m_ui8CommonGndAndRefBitmap&(1<<7));

	uint32 l_ui32Mode = M_NORMAL;
	if(m_bCalibrationSignalEnabled) 
	{
		l_ui32Mode = M_CALIBRATE;
	}

    if(!::GT_SetMode(o_pDevice, l_ui32Mode)) m_rDriverContext.getLogManager() << LogLevel_Error << "Unexpected error while calling GT_SetMode for mode " << l_ui32Mode << "\n";
       
    if(!::GT_SetBufferSize(o_pDevice, NUMBER_OF_SCANS)) m_rDriverContext.getLogManager() << LogLevel_Error << "Unexpected error while calling GT_SetBufferSize\n";
       
    if(!::GT_SetChannels(o_pDevice, l_oChannel, sizeof(l_oChannel)/sizeof(::UCHAR))) m_rDriverContext.getLogManager() << LogLevel_Error << "Unexpected error while calling GT_SetChannels\n";
       
    if(!::GT_EnableTriggerLine(o_pDevice, TRUE)) m_rDriverContext.getLogManager() << LogLevel_Error << "Unexpected error while calling GT_EnableTriggerLine - the extra input trigger channel is disabled\n";
    // GT_EnableSC

	// GT_SetBipolar
	CHANNEL _bipolarSettings;	
	if (this->m_bBipolarEnabled)
	{
		//the following configurations produces 8 bipolar: 1,3,5,7,9,11,13,15 and 8 unipolar 2,4,6,8,10,12,14,16 
		
		_bipolarSettings.Channel1 = 2;
		_bipolarSettings.Channel2 = 0;
		_bipolarSettings.Channel3 = 4;
		_bipolarSettings.Channel4 = 0;
		_bipolarSettings.Channel5 = 6;
		_bipolarSettings.Channel6 = 0;
		_bipolarSettings.Channel7 = 8;
		_bipolarSettings.Channel8 = 0;
		_bipolarSettings.Channel9 = 10;
		_bipolarSettings.Channel10 = 0;
		_bipolarSettings.Channel11 = 12;
		_bipolarSettings.Channel12 = 0;
		_bipolarSettings.Channel13 = 14;
		_bipolarSettings.Channel14 = 0;
		_bipolarSettings.Channel15 = 16;
		_bipolarSettings.Channel16 = 0;

	}
	else
	{
		_bipolarSettings.Channel1 = 0;
		_bipolarSettings.Channel2 = 0;
		_bipolarSettings.Channel3 = 0;
		_bipolarSettings.Channel4 = 0;
		_bipolarSettings.Channel5 = 0;
		_bipolarSettings.Channel6 = 0;
		_bipolarSettings.Channel7 = 0;
		_bipolarSettings.Channel8 = 0;
		_bipolarSettings.Channel9 = 0;
		_bipolarSettings.Channel10 = 0;
		_bipolarSettings.Channel11 = 0;
		_bipolarSettings.Channel12 = 0;
		_bipolarSettings.Channel13 = 0;
		_bipolarSettings.Channel14 = 0;
		_bipolarSettings.Channel15 = 0;
		_bipolarSettings.Channel16 = 0;
	}
	if (!GT_SetBipolar(o_pDevice, _bipolarSettings))
		m_rDriverContext.getLogManager() << LogLevel_Error << "Error on GT_SetBipolar: Couldn't set unipolar derivation for device " << current_serial;
	else if (this->m_bBipolarEnabled) m_rDriverContext.getLogManager() << LogLevel_Info << "Bipolar configuration is active.\n";


	ConfigFiltering(o_pDevice);

    if(!::GT_SetSampleRate(o_pDevice, m_oHeader.getSamplingFrequency())) m_rDriverContext.getLogManager() << LogLevel_Error << "Unexpected error while calling GT_SetSampleRate\n";

	if(l_ui32Mode = M_NORMAL)
	{
		if(!::GT_SetReference(o_pDevice, l_oReference)) m_rDriverContext.getLogManager() << LogLevel_Error << "Unexpected error while calling GT_SetReference\n";
		if(!::GT_SetGround(o_pDevice, l_oGround)) m_rDriverContext.getLogManager() << LogLevel_Error << "Unexpected error while calling GT_SetGround\n";
	}
	

	return true;
}

OpenViBE::boolean CDriverGTecGUSBamp::start(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;

	if(m_bReconfigurationRequired) 
	{	
		// Impedance checking or some other GT_ call has changed the device configuration, so we need to reconf
		for(uint32 i=0;i<numDevices;i++)
		{
			ConfigureDevice(i);
		}
		m_bReconfigurationRequired = false;
	}

	//new set process priority
	HANDLE hProcess = GetCurrentProcess();
    SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);
	//end set process priority

	m_ui32TotalHardwareStimulations = 0;
	m_ui32TotalDriverChunksLost = 0;
	m_ui32TotalRingBufferOverruns = 0;
	m_ui32TotalDataUnavailable = 0;

	for (uint32 i=0;i<numDevices;i++)
	{
		HANDLE o_pDevice = m_callSequenceHandles[i];
		::GT_Start(o_pDevice);
	}
	
	m_bIsThreadRunning = true;
	m_flagIsFirstLoop = true;
	m_bufferOverrun = false;
	m_ui32CurrentQueueIndex = 0;

	m_ThreadPtr.reset(new boost::thread( boost::bind(&CDriverGTecGUSBamp::acquire , this )));
	//applyPriority(m_ThreadPtr.get(),15);

	return true;
}

//This method is called by the AS and it supplies the acquired data to the AS
OpenViBE::boolean CDriverGTecGUSBamp::loop(void)
{
	CStimulationSet   l_oStimulationSet;

	if(m_rDriverContext.isStarted())
	{
		bool l_bDataAvailable=false;

		{
			boost::mutex::scoped_lock lock(m_io_mutex);
			
			while(m_RingBuffer.GetSize()<validPoints)
			{
				m_itemAvailable.wait(lock);
			}

			try
			{
				if (m_bufferOverrun)
				{
					m_RingBuffer.Reset();
					m_bufferOverrun = false;
					m_ui32TotalRingBufferOverruns++;
					return true;
				}

				m_RingBuffer.Read(m_bufferReceivedData, validPoints);
			}
			catch(std::exception e)
			{
				m_rDriverContext.getLogManager() << LogLevel_Error << "Error reading GTEC ring buffer! Error is:" << e.what() << "\n";
			}
			
			m_itemAvailable.notify_one();
		}		
		
		//Data is aligned as follows: element at position destBuffer[scanIndex * (numberOfChannelsPerDevice * numDevices) + channelIndex] is 
		//sample of channel channelIndex (zero-based) of the scan with zero-based scanIndex.
		//channelIndex ranges from 0..numDevices*numChannelsPerDevices where numDevices equals the number of recorded devices 
		//and numChannelsPerDevice the number of channels from each of those devices.
		     
		uint32 o_limit=(GTEC_NUM_CHANNELS + 1)*numDevices;

		for(uint32 i=0; i<o_limit; i++)
		{
			for(uint32 j=0; j<NUMBER_OF_SCANS; j++)
			{
				m_pSample[i*NUMBER_OF_SCANS+j]=m_bufferReceivedData[j*o_limit+i];
			}
		}

		if (m_bTriggerInputEnabled)
		{
			//here convert parallel port values to stimulations if you need to
		}

		m_pCallback->setSamples(m_pSample, NUMBER_OF_SCANS);	
        m_pCallback->setStimulationSet(l_oStimulationSet);
		m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());
	}
    else
	{
		if(m_rDriverContext.isImpedanceCheckRequested())
		{
			HANDLE o_pDevice = m_callSequenceHandles[0];//works only for one device

			double l_dImpedance=DBL_MAX;
			if(!::GT_GetImpedance(o_pDevice, m_ui32ActualImpedanceIndex+1, &l_dImpedance)) 
			{
				m_rDriverContext.getLogManager() << LogLevel_Error << "Impedance check failed for channel " << m_ui32ActualImpedanceIndex+1 << ". The amp may need a reset.\n";
			}
			else 
			{
				m_rDriverContext.getLogManager() << LogLevel_Trace << "Channel " << m_ui32ActualImpedanceIndex+1 << " - " << CString(m_oHeader.getChannelName(m_ui32ActualImpedanceIndex)) << " : " << l_dImpedance << "\n";
			}
			
			if(l_dImpedance<0) 
			{
				l_dImpedance*=-1;
			}

			m_rDriverContext.updateImpedance(m_ui32ActualImpedanceIndex, l_dImpedance);

			m_ui32ActualImpedanceIndex++;
			m_ui32ActualImpedanceIndex%=m_oHeader.getChannelCount();

			m_rDriverContext.updateImpedance(m_ui32ActualImpedanceIndex, -1);

			m_bReconfigurationRequired = true;
		}
		else
		{
			System::Time::sleep(20);
		}
	}

	return true;
}

//This function is as close to the original as possible: DoAcquisition in gUSBampSyncDemo.cpp
OpenViBE::boolean CDriverGTecGUSBamp::acquire(void)
{		
	DWORD numBytesReceived = 0;

	//we can not make these checks even if they look good
	//if(!m_rDriverContext.isConnected()) return false;
	//if(!m_rDriverContext.isStarted()) return false;

	if (m_flagIsFirstLoop) //First time do some memory initialization, etc
	{
		//for each device create a number of QUEUE_SIZE data buffers
		for (uint32 deviceIndex=0; deviceIndex<numDevices; deviceIndex++)
		{
			m_buffers[deviceIndex] = new BYTE*[QUEUE_SIZE];
			m_overlapped[deviceIndex] = new OVERLAPPED[QUEUE_SIZE];

			//for each data buffer allocate a number of bufferSizeBytes bytes
			for (int queueIndex=0; queueIndex<QUEUE_SIZE; queueIndex++)
			{
				m_buffers[deviceIndex][queueIndex] = new BYTE[bufferSizeBytes];
				memset(&(m_overlapped[deviceIndex][queueIndex]), 0, sizeof(OVERLAPPED));

				//create a windows event handle that will be signalled when new data from the device has been received for each data buffer
				m_overlapped[deviceIndex][queueIndex].hEvent = CreateEvent(NULL, false, false, NULL);

				if (!m_overlapped[deviceIndex][queueIndex].hEvent) 
				{
					m_rDriverContext.getLogManager() << LogLevel_Error << "Could not create handle!\n";
					return false;
				}
			}
		}

		for (uint32 deviceIndex=0; deviceIndex<numDevices; deviceIndex++)
		{
			//devices are started in "Start" method, so this part is skipped from the original g.tec code
			HANDLE hDevice = m_callSequenceHandles[deviceIndex];
			
			//queue-up the first batch of transfer requests
			for (int queueIndex=0; queueIndex<QUEUE_SIZE; queueIndex++)
			{
				if (!GT_GetData(hDevice, m_buffers[deviceIndex][queueIndex], bufferSizeBytes, &m_overlapped[deviceIndex][queueIndex]))
				{
					m_rDriverContext.getLogManager() << LogLevel_Error << "Error on GT_GetData in the initialization phase of the loop.\n";
					return false;
				}
			}
		}

		m_flagIsFirstLoop=false;

		m_ui32CurrentQueueIndex = 0;
	}

	//__try
	{
		while(m_bIsThreadRunning==true)
		{
			try
			{
				OpenViBE::boolean m_flagChunkLostDetected=false;
				OpenViBE::boolean m_flagChunkTimeOutDetected=false;

				//acquire data from the amplifier(s)
				for (uint32 deviceIndex = 0; deviceIndex < numDevices; deviceIndex++)
				{
					HANDLE hDevice = m_callSequenceHandles[deviceIndex];

					//wait for notification from the system telling that new data is available
					if (WaitForSingleObject(m_overlapped[deviceIndex][m_ui32CurrentQueueIndex].hEvent, 1000) == WAIT_TIMEOUT)
					{
						//cout << "Error on data transfer: timeout occurred." << "\n";
						m_flagChunkTimeOutDetected = true;
					}

					//get number of received bytes...
					GetOverlappedResult(hDevice, &m_overlapped[deviceIndex][m_ui32CurrentQueueIndex], &numBytesReceived, false);

					//...and check if we lost something (number of received bytes must be equal to the previously allocated buffer size)
					if (numBytesReceived != bufferSizeBytes)
					{
						m_ui32TotalDriverChunksLost++;
						m_flagChunkLostDetected = true;
					}
				}

				//this line is commented on purpose
				//if (m_flagChunkTimeOutDetected==false && m_flagChunkLostDetected==false)

				//store to ring buffer
				{
					//to store the received data into the application data buffer at once, lock it
					{
						boost::mutex::scoped_lock lock(m_io_mutex);
						try
						{
							//if we are going to overrun on writing the received data into the buffer, set the appropriate flag; the reading thread will handle the overrun
							m_bufferOverrun = (m_RingBuffer.GetFreeSize() < static_cast<int>(nPoints * numDevices));

							//store received data from each device in the correct order (that is scan-wise, where one scan includes all channels of all devices) ignoring the header
							for (uint32 scanIndex = 0; scanIndex < NUMBER_OF_SCANS; scanIndex++)
								for (uint32 deviceIndex = 0; deviceIndex < numDevices; deviceIndex++)
								{
									m_RingBuffer.Write((float*) (m_buffers[deviceIndex][m_ui32CurrentQueueIndex] + scanIndex * (GTEC_NUM_CHANNELS + 1) * sizeof(float) + HEADER_SIZE), (GTEC_NUM_CHANNELS + 1));
								}
						}
						catch(std::exception e)
						{
							//buffer should be unclocked automatically once the scope is left
							m_rDriverContext.getLogManager() << LogLevel_Error << "Error writing to GTEC ring buffer! Error is: " << e.what() <<"\n";
						}
						//buffer should be unclocked automatically once the scope is left

						m_itemAvailable.notify_one();
					}
				}

				//add new GetData call to the queue replacing the currently processed one
				//this gives us time to process data while we wait for a new data chunk from the amplifier
				for (uint32 deviceIndex = 0; deviceIndex < numDevices; deviceIndex++)
				{
					HANDLE hDevice = m_callSequenceHandles[deviceIndex];
					if (!GT_GetData(hDevice, m_buffers[deviceIndex][m_ui32CurrentQueueIndex], bufferSizeBytes, &m_overlapped[deviceIndex][m_ui32CurrentQueueIndex]))
					{
						m_rDriverContext.getLogManager() << LogLevel_Error << "Error on GT_GetData in standard loop processing.\n";
						
						return false;
					}
				}
				//increment circular queueIndex to process the next queue at the next loop repetition (on overrun start at index 0 again)
				m_ui32CurrentQueueIndex = (m_ui32CurrentQueueIndex + 1) % QUEUE_SIZE;
			}
			catch(std::exception e)
			{
				m_rDriverContext.getLogManager() << LogLevel_Error << "General error in the thread function acquiring data from GTEC! Acquisition interrupted. Error is: " << e.what() << "\n";
				m_bIsThreadRunning = false;
				return false;
			}
		}
	}
	//__finally
	
	//This code stops the amplifiers in the same thread:
	{
		m_rDriverContext.getLogManager() << LogLevel_Info << "Stopping devices and cleaning up..." << "\n";
	
		char serial[16];

		if (numDevices > 1)
		{
			::GT_GetSerial(m_callSequenceHandles[numDevices-1], serial, 16);
			if (string(serial) != m_masterSerial)
			{
				m_rDriverContext.getLogManager() << LogLevel_Error << "Master device is not the last one! serial=" << serial << " master=" << m_masterSerial.c_str() << " .\n";
			}
		}

		//clean up allocated resources for each device
		for (uint32 i=0; i<numDevices; i++)
		{
			HANDLE hDevice = m_callSequenceHandles[i];

			//clean up allocated resources for each queue per device
			for (int j=0; j<QUEUE_SIZE; j++)
			{
				WaitForSingleObject(m_overlapped[i][m_ui32CurrentQueueIndex].hEvent, 1000);
				CloseHandle(m_overlapped[i][m_ui32CurrentQueueIndex].hEvent);

				//increment queue index
				m_ui32CurrentQueueIndex = (m_ui32CurrentQueueIndex + 1) % QUEUE_SIZE;
			}

			//stop device
			if(!::GT_Stop(hDevice)) 
			{
				::GT_GetSerial(m_callSequenceHandles[numDevices-1], serial, 16);
				m_rDriverContext.getLogManager() << LogLevel_Error << "Stopping device failed! Serial = " << serial << "\n";
			}

			//reset data transfer
			if(!::GT_ResetTransfer(hDevice)) 
			{
				::GT_GetSerial(m_callSequenceHandles[numDevices-1], serial, 16);
				m_rDriverContext.getLogManager() << LogLevel_Error << "Reset data transfer failed! Serial = " << serial << "\n";
			}

			// Sometimes when the amplifier is jammed, freeing the buffer causes heap corruption if its done before stop and reset. So we free the buffers here.
			for (int j=0; j<QUEUE_SIZE; j++) {
				delete [] m_buffers[i][j];
			}

			delete [] m_overlapped[i];
			delete [] m_buffers[i];
		}

		m_flagIsFirstLoop = true;
		m_bIsThreadRunning = false;
	}
	return true;
}

OpenViBE::boolean CDriverGTecGUSBamp::stop(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(!m_rDriverContext.isStarted()) return false;

	//reset the main process (data processing thread) to normal priority
	HANDLE hProcess = GetCurrentProcess();
	SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);

	//stop thread
	m_bIsThreadRunning = false;
	m_ThreadPtr->join(); //wait until the thread has stopped data acquisition

	m_rDriverContext.getLogManager() << LogLevel_Debug   << "Total number of hardware stimulations acquired: " << m_ui32TotalHardwareStimulations << "\n";
	m_rDriverContext.getLogManager() << LogLevel_Debug << "Total chunks lost: " << m_ui32TotalDriverChunksLost << "\n";
	m_rDriverContext.getLogManager() << LogLevel_Debug << "Total internal ring buffer overruns: " << m_ui32TotalRingBufferOverruns << "\n";
	m_rDriverContext.getLogManager() << LogLevel_Debug << "Total times GTEC ring data buffer was empty: " << 	m_ui32TotalDataUnavailable << "\n";

	return true;
}

OpenViBE::boolean CDriverGTecGUSBamp::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;
	
	int i=0;
	while (!m_callSequenceHandles.empty())
	{
		//closes each opened device and removes it from the call sequence
		if(!::GT_CloseDevice(&m_callSequenceHandles.front()))
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "Unable to close device!\n";
		}
		m_callSequenceHandles.pop_front();
		i++;
	}
	m_callSequenceHandles.clear();

	m_rDriverContext.getLogManager() << LogLevel_Info << "Total devices closed: " << i << "\n";

	//clear memory
	delete[] m_bufferReceivedData;
	delete[] m_overlapped;

	if (m_pSample!=NULL)
	{
		delete [] m_pSample;
		m_pSample=NULL;
	}
	m_pCallback=NULL;

	delete[] m_buffers;
	m_buffers = NULL;
	
	return true;
}

OpenViBE::boolean CDriverGTecGUSBamp::setMasterDevice(string targetMasterSerial)
{   
	int targetDeviceIndex = -1;//points to the one that needs to become master
	for (uint32 i=0;i<m_vDevicesSerials.size();i++)
	{
		if (m_vDevicesSerials[i] == targetMasterSerial)
		{
			targetDeviceIndex = i;
			break;
		}
	}
	
	if (numDevices>1 && targetDeviceIndex<static_cast<int>(numDevices) && targetDeviceIndex>=0)
	{
		uint32 lastIndex = numDevices-1;
		//swap the handlers and serials, set the desired one as last and master

		//start swap - put selected device as last
		HANDLE o_tempDevice = m_callSequenceHandles[lastIndex];
		string o_tempSerial = m_vDevicesSerials[lastIndex];

		m_callSequenceHandles[lastIndex] = m_callSequenceHandles[targetDeviceIndex];
		m_vDevicesSerials[lastIndex] = m_vDevicesSerials[targetDeviceIndex];

		m_callSequenceHandles[targetDeviceIndex] = o_tempDevice;
		m_vDevicesSerials[targetDeviceIndex] = o_tempSerial;
		//end swap

		//set slaves and new master
		m_slavesCnt = 0;
		m_mastersCnt = 0;
		for (uint32 i=0;i<numDevices;i++)
		{
			OpenViBE::boolean isSlave = (i != (numDevices - 1));
		    if (numDevices==1) {isSlave = false;}

			char serial[16];
			::GT_GetSerial(m_callSequenceHandles[i], serial, 16);

			if (isSlave)
			{
				m_rDriverContext.getLogManager() << LogLevel_Warning << "Configured as slave device: " << serial << " \n";
				m_slavesCnt++;
			}
			else 
			{
				m_rDriverContext.getLogManager() << LogLevel_Warning << "Configured as MASTER device: " << serial << " \n";
				m_mastersCnt++;
				m_masterSerial = serial;
			}
			if(!::GT_SetSlave(m_callSequenceHandles[i], isSlave)) m_rDriverContext.getLogManager() << LogLevel_Error << "Unexpected error while calling GT_SetSlave\n";
		}

		verifySyncMode();

	}
	else 
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Bad number of devices detected for this operation or invalid master selection!" << "\n";
		return false;
	}

	return true;
}

//Checks if devices are configured correctly when acquiring data from multiple devices
OpenViBE::boolean CDriverGTecGUSBamp::verifySyncMode()
{
	//g.tec check list
	if (numDevices>1 || m_callSequenceHandles.size()>1)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "More than 1 device detected, performing some basic sync checks.\n";
		
		if (numDevices != m_callSequenceHandles.size() && numDevices != m_vDevicesSerials.size())
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "Problem with number of amplifiers!" << numDevices << " " << m_callSequenceHandles.size() << "\n";
			return false;
		}

		//Test that only one device is master
		if ((m_mastersCnt + m_slavesCnt != numDevices) || (m_mastersCnt!=1))
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "Problem with number of slaves/masters compared to total number of devices!\n";
			return false;
		}

		//Test that the master device is the last in the sequence
		char serial[16];
		::GT_GetSerial(m_callSequenceHandles[numDevices-1], serial, 16);

		if (string(serial) != m_masterSerial && string(serial) == m_vDevicesSerials[numDevices-1])
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "Master device is not the last one! serial=" << serial << " master=" << m_masterSerial.c_str() << " .\n";
			return false;
		}
	}

	return true;
}

//___________________________________________________________________//
//                                                                   //
OpenViBE::boolean CDriverGTecGUSBamp::isConfigurable(void)
{
	return true;
}

OpenViBE::boolean CDriverGTecGUSBamp::configure(void)
{
	//CConfigurationGTecGUSBamp m_oConfiguration("../share/openvibe-applications/acquisition-server/interface-GTec-GUSBamp.ui", m_ui32DeviceIndex, m_ui8CommonGndAndRefBitmap, m_i32NotchFilterIndex,m_i32BandPassFilterIndex,m_bTriggerInputEnabled);

	detectDevices();
	
	string targetMasterSerial = (numDevices>1) ? m_masterSerial : "";
	CConfigurationGTecGUSBamp m_oConfiguration(OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-GTec-GUSBamp.ui", 
		m_ui32DeviceIndex, 
		m_ui8CommonGndAndRefBitmap, 
		m_i32NotchFilterIndex, 
		m_i32BandPassFilterIndex,
		m_bTriggerInputEnabled,
		m_vDevicesSerials,
		targetMasterSerial,
		m_bBipolarEnabled,
		m_bCalibrationSignalEnabled);

	//reduce from number of channels for all devices to the number of channels for one device
	m_oHeader.setChannelCount(m_ui32AcquiredChannelCount);
	
	if(!m_oConfiguration.configure(m_oHeader))
	{
		return false;
	}

	//get new value from header
	this->m_ui32AcquiredChannelCount = m_oHeader.getChannelCount();

	m_oSettings.save();

	//start reconfigure based on the new input:

	if (targetMasterSerial!="")
	    setMasterDevice(targetMasterSerial);

	//expand the value from one device to all devices
	if (m_bTriggerInputEnabled)
	{
		m_oHeader.setChannelCount((m_ui32AcquiredChannelCount+1)*numDevices);
	}
	else
	{
		m_oHeader.setChannelCount(m_ui32AcquiredChannelCount*numDevices);
	}
    //end reconfigure based on the new input

	return true;
}

/*
 This method should not be necessarily used.
 Use it if you have problems with frequencies above 10 000.
 "real time" is 15.
*/
void CDriverGTecGUSBamp::applyPriority(boost::thread* thread, int priority)
{
    if (!thread)
        return;

    BOOL res;
    HANDLE th = thread->native_handle();

    switch (priority)
    {
		case THREAD_PRIORITY_TIME_CRITICAL               : res = SetThreadPriority(th, THREAD_PRIORITY_TIME_CRITICAL);   break;
		case THREAD_PRIORITY_HIGHEST                   : res = SetThreadPriority(th, THREAD_PRIORITY_HIGHEST);                 break;
		case THREAD_PRIORITY_ABOVE_NORMAL   : res = SetThreadPriority(th, THREAD_PRIORITY_ABOVE_NORMAL);    break;
		case THREAD_PRIORITY_NORMAL                 : res = SetThreadPriority(th, THREAD_PRIORITY_NORMAL);                  break;
		case THREAD_PRIORITY_BELOW_NORMAL   : res = SetThreadPriority(th, THREAD_PRIORITY_BELOW_NORMAL);    break;
		case THREAD_PRIORITY_LOWEST                   : res = SetThreadPriority(th, THREAD_PRIORITY_LOWEST);                  break;
    }
}


void CDriverGTecGUSBamp::ConfigFiltering(HANDLE o_pDevice)
{
	OpenViBE::boolean status;
	int32 nrOfFilters;
	const float32 mySamplingRate = static_cast<float32>(m_oHeader.getSamplingFrequency());

	// note: the only reason to ask the filter specs here seems to be to be able to print some details to the LogManager().

	//Set BandPass
	
	// get the number of available filters
	status = GT_GetNumberOfFilter(&nrOfFilters);
	if (status==false) 
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Could not get the number of dsp filters! Filtering is disabled.\n";	
		return;
	}

	// create array of FILT structures to store the filter settings
	FILT *filters = new FILT[nrOfFilters];

	// fill array with filter settings
	status = GT_GetFilterSpec(filters);
	if (status==false) 
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Could not get the list of dsp filters! Filtering is disabled.\n";
		delete[] filters;
		return;
	}
	
	for(int i=1; i<=GTEC_NUM_CHANNELS; i++)  //channels must be [1..16]
	{
		status = GT_SetBandPass(o_pDevice, i, m_i32BandPassFilterIndex);
		if (status==false) 
		{ 
			char serial[20];
		    ::GT_GetSerial(o_pDevice, serial, 20);
			m_rDriverContext.getLogManager() << LogLevel_Error << "Could not set band pass filter on channel " << i << " on device " << serial << "\n";
			delete[] filters;
			return;
		}
	}
	
	if (m_i32BandPassFilterIndex ==-1) m_rDriverContext.getLogManager() << LogLevel_Info << "No BandPass filter applied.\n";
	else m_rDriverContext.getLogManager() << LogLevel_Info << "Bandpass filter applied: between " << filters[m_i32BandPassFilterIndex].fu << " and " << filters[m_i32BandPassFilterIndex].fo << ", order = " << filters[m_i32BandPassFilterIndex].order << ", type = " << ((filters[m_i32BandPassFilterIndex].type == 1) ? "butterworth" : "chebyshev") << ", frequency = " << mySamplingRate << "\n";
	
	delete[] filters;

	//Set Notch

	// get the number of available filters
	status = GT_GetNumberOfNotch(&nrOfFilters);
	if (status==false) 
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Could not get the number of notch filters! Filtering is disabled.\n";
		return;
	}

	// create array of FILT structures to store the filter settings
	filters = new FILT[nrOfFilters];

	// fill array with filter settings
	status = GT_GetNotchSpec(filters);
	if (status==false) 
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Could not get the list of notch filters! Filtering is disabled.\n";
		delete[] filters;
		return;
	}
	
	for(int i=1; i<=GTEC_NUM_CHANNELS; i++)  //channels must be [1..16]
	{
		status = GT_SetNotch(o_pDevice, i, m_i32NotchFilterIndex);
		if (status==0) 
		{ 
			char serial[20];
		    ::GT_GetSerial(o_pDevice, serial, 20);
			m_rDriverContext.getLogManager() << LogLevel_Error << "Could not set notch filter on channel " << i << " on device " << serial << "\n";
			delete[] filters;
			return;
		}
	}
	
	if (m_i32NotchFilterIndex ==-1) m_rDriverContext.getLogManager() << LogLevel_Info << "No Notch filter applied.\n";
	else m_rDriverContext.getLogManager() << LogLevel_Info << "Notch filter applied: " << (filters[m_i32NotchFilterIndex].fo +  filters[m_i32NotchFilterIndex].fu) /2 << " Hz.\n";	

	delete[] filters;
}

namespace OpenViBEAcquisitionServer {

inline std::ostream& operator<< (std::ostream& out, const vector<string>& var)
{
	for(size_t i=0;i<var.size();i++) {
		out << var[i] << " ";
	}

	return out;
}

inline std::istream& operator>> (std::istream& in, vector<string>& var)
{
	var.clear();
	string tmp;
	while( in >> tmp ) {
		var.push_back(tmp);
	}

	return in;
}

}

#endif // TARGET_HAS_ThirdPartyGUSBampCAPI
