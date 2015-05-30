/*
 * OpenBCI driver for OpenViBE
 *
 * \author Jeremy Frey
 *
 * \note Based on OpenEEG code; inherits its AGPL3 license conditions
 *
 */

#include "ovasCDriverOpenBCI.h"
#include "ovasCConfigurationOpenBCI.h"

#include <toolkit/ovtk_all.h>

#include <system/ovCTime.h>
#include <system/ovCMemory.h>
#include <cmath>
#include <iostream>

#include <time.h>
#include <cstring>
#include <sstream>

#if defined TARGET_OS_Windows
 #include <windows.h>
 #include <winbase.h>
 #include <cstdio>
 #include <cstdlib>
 #include <commctrl.h>
 #include <winsock2.h> // htons and co.
 //#define TERM_SPEED 57600
 #define TERM_SPEED CBR_115200 // OpenBCI is a bit faster than others
#elif defined TARGET_OS_Linux
 #include <cstdio>
 #include <unistd.h>
 #include <fcntl.h>
 #include <termios.h>
 #include <sys/select.h>
 #include <netinet/in.h> // htons and co.
 #include <unistd.h>
 #define TERM_SPEED B115200
#else
#endif

#define boolean OpenViBE::boolean
using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;

//___________________________________________________________________//
// Heavily inspired by OpenEEG code. Will override channel count and sampling late upon "daisy" selection. If daisy module is attached, will concatenate EEG values and average accelerometer values every two samples.
//                                                                   //

CDriverOpenBCI::CDriverOpenBCI(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_oSettings("AcquisitionServer_Driver_OpenBCI", m_rDriverContext.getConfigurationManager())
	,m_pCallback(NULL)
	,m_ui32ChannelCount(19)
	,m_ui32DeviceIdentifier(uint32(-1))
	,m_pSample(NULL)
{
	m_sComInit="";
	m_ui32ComDelay=100;

	m_oSettings.add("Header", &m_oHeader);
	m_oSettings.add("DeviceIdentifier", &m_ui32DeviceIdentifier);
	m_oSettings.add("ComInit", &m_sComInit);
	m_oSettings.add("ComDelay", &m_ui32ComDelay);
	m_oSettings.add("DaisyModule", &m_bDaisyModule);
	
	m_oSettings.load();
	
	// default parameter loaded, update channel count and frequency
	updateDaisy(true);
}

void CDriverOpenBCI::release(void)
{
	delete this;
}

const char* CDriverOpenBCI::getName(void)
{
	return "OpenBCI";
}

//___________________________________________________________________//
//                                                                   //

void  CDriverOpenBCI::updateDaisy(bool bBeQuiet) {
	// change channel and sampling rate according to daisy module
	if (m_bDaisyModule) {
		m_oHeader.setSamplingFrequency(DefaultSamplingRate/2);
		m_oHeader.setChannelCount(2*EEGNbValuesPerSample+AccNbValuesPerSample);
		if(!bBeQuiet) m_rDriverContext.getLogManager() << LogLevel_Info << "Daisy module attached, " << m_oHeader.getChannelCount() << " channels -- " << (int)(2*EEGNbValuesPerSample) << " EEG and " << (int)AccNbValuesPerSample << " accelerometer -- at " << m_oHeader.getSamplingFrequency() << "Hz." << "\n";
	}
	else {
	  	m_oHeader.setSamplingFrequency(DefaultSamplingRate);
		m_oHeader.setChannelCount(EEGNbValuesPerSample+AccNbValuesPerSample);
		if(!bBeQuiet) m_rDriverContext.getLogManager() << LogLevel_Info << "NO daisy module attached, " << m_oHeader.getChannelCount() << " channels -- " << (int)EEGNbValuesPerSample << " EEG and " << (int)AccNbValuesPerSample << " accelerometer -- at " << m_oHeader.getSamplingFrequency() << "Hz." << "\n";
	}
	
	int32 l_ui32nChannels = m_oHeader.getChannelCount();
	// microvolt for EEG channels
	for(int32 c=0;c<l_ui32nChannels-AccNbValuesPerSample;c++)
	{
		m_oHeader.setChannelUnits(c, OVTK_UNIT_Volts, OVTK_FACTOR_Micro);
	}
	// undefined for accelerometer/extra channels
	for(int32 c=l_ui32nChannels-AccNbValuesPerSample;c<l_ui32nChannels;c++)
	{
		m_oHeader.setChannelUnits(c, OVTK_UNIT_Unspecified, OVTK_FACTOR_Base);
	}
}

boolean CDriverOpenBCI::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) { return false; }

	// change channel and sampling rate according to daisy module
	updateDaisy(false);
	
	// init state
	m_ui16Readstate=0;
	m_ui16ExtractPosition=0;
	m_i16SampleNumber  = -1;
	m_ui16ExtractPosition = 0;
	m_bSeenPacketFooter = true; // let's say we will start with header

	if(!this->initTTY(&m_i32FileDescriptor, m_ui32DeviceIdentifier!=uint32(-1)?m_ui32DeviceIdentifier:1))
	{
		return false;
	}
	
	// check board status and print response
	if(!this->initBoard(m_i32FileDescriptor))
	{
		return false;
	}
	
	m_pSample=new float32[m_oHeader.getChannelCount()];
	if(!m_pSample)
	{
		delete [] m_pSample;
		m_pSample=NULL;
		return false;
	}
	// prepare buffer for samples
	m_vSampleEEGBuffer.resize(EEGNbValuesPerSample);
	m_vSampleEEGBufferDaisy.resize(EEGNbValuesPerSample);
	m_vSampleAccBuffer.resize(AccNbValuesPerSample);
	m_vSampleAccBufferDaisy.resize(AccNbValuesPerSample);
	
	// init buffer for 1 EEG value and 1 accel value
	m_vEEGValueBuffer.resize(EEGValueBufferSize);
	m_vAccValueBuffer.resize(AccValueBufferSize);
	
	m_pCallback=&rCallback;
	m_ui32ChannelCount=m_oHeader.getChannelCount();
	m_i16LastPacketNumber=-1;

	m_rDriverContext.getLogManager() << LogLevel_Debug << CString(this->getName()) << " driver initialized.\n";
	
	// init scale factor
	ScaleFacuVoltsPerCount = (float32) (ADS1299_VREF/(pow(2.,23)-1)/ADS1299_GAIN*1000000.);
	
	return true;
}

boolean CDriverOpenBCI::start(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }
	m_rDriverContext.getLogManager() << LogLevel_Debug << CString(this->getName()) << " driver started.\n";
	return true;
}

boolean CDriverOpenBCI::loop(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }

	if(this->readPacketFromTTY(m_i32FileDescriptor)<0)
	{
		m_rDriverContext.getLogManager() << LogLevel_ImportantWarning << "Could not receive data from " << m_sTTYName << "\n";
		return false;
	}

	if(m_vChannelBuffer.size()!=0)
	{
		if(m_rDriverContext.isStarted())
		{
			for(uint32 i=0; i<m_vChannelBuffer.size(); i++)
			{
				for(uint32 j=0; j<m_ui32ChannelCount; j++)
				{
					m_pSample[j]= m_vChannelBuffer[i][j];
				}
				m_pCallback->setSamples(m_pSample, 1);
			}
			m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());
		}
		m_vChannelBuffer.clear();
	}

	return true;
}

boolean CDriverOpenBCI::stop(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }
	m_rDriverContext.getLogManager() << LogLevel_Debug << CString(this->getName()) << " driver stopped.\n";
	return true;
}

boolean CDriverOpenBCI::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

	this->closeTTY(m_i32FileDescriptor);

	m_rDriverContext.getLogManager() << LogLevel_Debug << CString(this->getName()) << " driver closed.\n";

	delete [] m_pSample;
	m_pSample=NULL;
	m_pCallback=NULL;

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverOpenBCI::isConfigurable(void)
{
	return true;
}

boolean CDriverOpenBCI::configure(void)
{
	CConfigurationOpenBCI m_oConfiguration(
		OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-OpenBCI.ui", 
		m_ui32DeviceIdentifier);

	m_oConfiguration.setComInit(m_sComInit);
	m_oConfiguration.setComDelay(m_ui32ComDelay);
	m_oConfiguration.setDaisyModule(m_bDaisyModule);
	
	if(!m_oConfiguration.configure(m_oHeader)) {
		return false;
	}

	m_sComInit=m_oConfiguration.getComInit();
	m_ui32ComDelay=m_oConfiguration.getComDelay();
	m_bDaisyModule=m_oConfiguration.getDaisyModule();
	m_oSettings.save();

	updateDaisy(false);
	
	return true;
}
  
// Convert EEG value format from int24 MSB (network order) to int32 host
// TODO: check on big endian architecture
int32 CDriverOpenBCI::interpret24bitAsInt32(std::vector < uint8 > byteBuffer) {
	// create a big endian so that we could adapt to host architecture later on
	int32 newInt = (byteBuffer[2] << 24) | (byteBuffer[1] << 16) | byteBuffer[0] << 8;
	// depending on most significant byte, set positive or negative value
	if ((newInt & 0x00008000) > 0) {
		newInt |= 0x000000FF;
	} else {
		newInt &= 0xFFFFFF00;
	}
	// convert back from big endian (network order) to host
	return htonl(newInt);
}

// Convert EEG value format from int16 MSB (network order) to int32 host
int32 CDriverOpenBCI::interpret16bitAsInt32(std::vector < uint8 > byteBuffer) {
	// create a big endian so that we could adapt to host architecture later on
	int32 newInt = (byteBuffer[1] << 24) | byteBuffer[0] << 16;
	// depending on most significant byte, set positive or negative value
	if ((newInt & 0x00800000) > 0) {
		newInt |= 0x0000FFFF;
	} else {
		newInt &= 0xFFFF0000;
	}
	// convert back from big endian (network order) to host
	return htonl(newInt);
}

//___________________________________________________________________//
//                                                                   //
// return sample number once one is received (between 0 and 255, -1 if none)
// NB: will wait to get footer and then header in a row, may miss a packet but will prevent a bad sync with stream (thx BrainBay for the tip!)
OpenViBE::int16 CDriverOpenBCI::parseByte(uint8 ui8Actbyte)
{
	// finished to read sample or not
	bool l_bSampleStatus = false;
	
	switch(m_ui16Readstate)
	{
		case 0:
			// if first byte is not the one expected, won't go further
			if(ui8Actbyte==SAMPLE_STOP_BYTE) {
				m_bSeenPacketFooter = true;
			}
			else {
				if(ui8Actbyte==SAMPLE_START_BYTE && m_bSeenPacketFooter)
				{
					m_ui16Readstate++;
				}
				m_bSeenPacketFooter = false;
			}
			// reset sample info
			m_i16SampleNumber  = -1;
			m_ui16ExtractPosition = 0;
			m_ui8SampleBufferPosition = 0;
			break;
		// Byte 2: Sample Number
		case 1:
			m_i16SampleNumber = ui8Actbyte;
			m_ui16Readstate++;
			break;
		// reading EEG data 
		/* 
		Note: values are 24-bit signed, MSB first

		* Bytes 3-5: Data value for EEG channel 1
		* Bytes 6-8: Data value for EEG channel 2
		* Bytes 9-11: Data value for EEG channel 3
		* Bytes 12-14: Data value for EEG channel 4
		* Bytes 15-17: Data value for EEG channel 5
		* Bytes 18-20: Data value for EEG channel 6
		* Bytes 21-23: Data value for EEG channel 6
		* Bytes 24-26: Data value for EEG channel 8
		*/
		case 2:
			if (m_ui16ExtractPosition < EEGNbValuesPerSample) {	
				// fill EEG buffer
				if (m_ui8SampleBufferPosition < EEGValueBufferSize) {
					m_vEEGValueBuffer[m_ui8SampleBufferPosition] = ui8Actbyte;	
					m_ui8SampleBufferPosition++;
				}
				// we got EEG value
				if (m_ui8SampleBufferPosition == EEGValueBufferSize)
				{
					// fill EEG channel buffer, converting at the same time from 24 to 32 bits + scaling
					// TODO: scale depends on gain
					m_vSampleEEGBuffer[m_ui16ExtractPosition] = (float) interpret24bitAsInt32(m_vEEGValueBuffer)*ScaleFacuVoltsPerCount;
					// reset for next value
					m_ui8SampleBufferPosition = 0;
					m_ui16ExtractPosition++;
				
				}
			}
			// finished with EEG
			if (m_ui16ExtractPosition == EEGNbValuesPerSample) {
				// next step: accelerometer
				m_ui16Readstate++;
				// re-use the same variable to know position inside accelerometer block (I know, I'm bad!).
				m_ui16ExtractPosition=0;
			}
			break;
		// reading accelerometer data
		/*
		 Note: values are 16-bit signed, MSB first
		 
		* Bytes 27-28: Data value for accelerometer channel X
		* Bytes 29-30: Data value for accelerometer channel Y
		* Bytes 31-32: Data value for accelerometer channel Z
		*/
		case 3:
			if (m_ui16ExtractPosition < AccNbValuesPerSample) {	
				// fill Acc buffer
				if (m_ui8SampleBufferPosition < AccValueBufferSize) {
					m_vAccValueBuffer[m_ui8SampleBufferPosition] = ui8Actbyte;
					m_ui8SampleBufferPosition++;
				}
				// we got Acc value
				if (m_ui8SampleBufferPosition == AccValueBufferSize) {
					// fill Acc channel buffer, converting at the same time from 16 to 32 bits
					m_vSampleAccBuffer[m_ui16ExtractPosition] = (float) interpret16bitAsInt32(m_vAccValueBuffer);
					// reset for next value
					m_ui8SampleBufferPosition = 0;
					m_ui16ExtractPosition++;
				 }
			}
			// finished with acc
			if (m_ui16ExtractPosition == AccNbValuesPerSample) {
				// next step: footer
				m_ui16Readstate++;
			}
			break;
		// footer: Byte 33: 0xC0
		case 4:
			// expected footer: perfect, returns sample number
			if(ui8Actbyte==SAMPLE_STOP_BYTE)
			{
				// we shall pass
				l_bSampleStatus = true;
				// we're ockay for next time
				m_bSeenPacketFooter = true;
			}
			// if last byte is not the one expected, discard whole sample
			else
			{
			}
			// whatever happened, it'll be the end of this journey
			m_ui16Readstate=0;
			break;
		// uh-oh, should not be there
		default:
			if(ui8Actbyte==SAMPLE_STOP_BYTE)
			{
				// we're ockay for next time
				m_bSeenPacketFooter = true;
			}
			m_ui16Readstate = 0;
			break;
	}
	// if it's a GO, returns sample number, may trigger channel push
	if (l_bSampleStatus) {
		return m_i16SampleNumber;
	}
	// by default we're not ready
	return -1;
}

boolean CDriverOpenBCI::initTTY(::FD_TYPE* pFileDescriptor, uint32 ui32TTYNumber)
{
	char l_sTTYName[1024];

#if defined TARGET_OS_Windows

	::sprintf(l_sTTYName, "\\\\.\\COM%d", ui32TTYNumber);
	DCB dcb = {0};
	*pFileDescriptor=::CreateFile(
		(LPCSTR)l_sTTYName,
		GENERIC_READ|GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if(*pFileDescriptor == INVALID_HANDLE_VALUE)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Could not open port [" << CString(l_sTTYName) << "]\n";
		return false;
	}

	if(!::GetCommState(*pFileDescriptor, &dcb))
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Could not get comm state on port [" << CString(l_sTTYName) << "]\n";
		return false;
	}

	// update DCB rate, byte size, parity, and stop bits size
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate  = TERM_SPEED;
	dcb.ByteSize  = 8;
	dcb.Parity    = NOPARITY;
	dcb.StopBits  = ONESTOPBIT;
	dcb.EvtChar   = '\0';

	// update flow control settings
	dcb.fDtrControl       = DTR_CONTROL_ENABLE;
	dcb.fRtsControl       = RTS_CONTROL_ENABLE;
	dcb.fOutxCtsFlow      = FALSE;
	dcb.fOutxDsrFlow      = FALSE;
	dcb.fDsrSensitivity   = FALSE;;
	dcb.fOutX             = FALSE;
	dcb.fInX              = FALSE;
	dcb.fTXContinueOnXoff = FALSE;
	dcb.XonChar           = 0;
	dcb.XoffChar          = 0;
	dcb.XonLim            = 0;
	dcb.XoffLim           = 0;
	dcb.fParity           = FALSE;

	::SetCommState(*pFileDescriptor, &dcb);
	::SetupComm(*pFileDescriptor, 64/*1024*/, 64/*1024*/);
	::EscapeCommFunction(*pFileDescriptor, SETDTR);
	::SetCommMask (*pFileDescriptor, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_RING);

#elif defined TARGET_OS_Linux

	struct termios l_oTerminalAttributes;

	// open ttyS<i> for i < 10, else open ttyUSB<i-10>
	if(ui32TTYNumber<10)
	{
		::sprintf(l_sTTYName, "/dev/ttyS%d", ui32TTYNumber);
	}
	else
	{
		::sprintf(l_sTTYName, "/dev/ttyUSB%d", ui32TTYNumber-10);
	}

	if((*pFileDescriptor=::open(l_sTTYName, O_RDWR))==-1)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Could not open port [" << CString(l_sTTYName) << "]\n";
		return false;
	}

	if(::tcgetattr(*pFileDescriptor, &l_oTerminalAttributes)!=0)
	{
		::close(*pFileDescriptor);
		*pFileDescriptor=-1;
		m_rDriverContext.getLogManager() << LogLevel_Error << "terminal: tcgetattr() failed - did you use the right port [" << CString(l_sTTYName) << "] ?\n";
		return false;
	}

	/* l_oTerminalAttributes.c_cflag = TERM_SPEED | CS8 | CRTSCTS | CLOCAL | CREAD; */
	l_oTerminalAttributes.c_cflag = TERM_SPEED | CS8 | CLOCAL | CREAD;
	l_oTerminalAttributes.c_iflag = 0;
	l_oTerminalAttributes.c_oflag = OPOST | ONLCR;
	l_oTerminalAttributes.c_lflag = 0;
	if(::tcsetattr(*pFileDescriptor, TCSAFLUSH, &l_oTerminalAttributes)!=0)
	{
		::close(*pFileDescriptor);
		*pFileDescriptor=-1;
		m_rDriverContext.getLogManager() << LogLevel_Error << "terminal: tcsetattr() failed - did you use the right port [" << CString(l_sTTYName) << "] ?\n";
		return false;
	}

#else

	return false;

#endif

	m_sTTYName = l_sTTYName;

	return true;
}

// if waitForResponse, will print output after the sequence of character is sent
// sleepBetween: time to sleep between each character written (in ms)
boolean CDriverOpenBCI::boardWriteAndPrint(::FD_TYPE i32FileDescriptor, const char * cmd, bool waitForResponse, uint32 sleepBetween) {
	// no command: don't go further
	if (strlen(cmd) == 0) {
		return true;
	}
	uint32 cmdSize = strlen(cmd);
	uint8  l_ui8ReadBuffer[1]; // TODO: better size for buffer
	uint32 l_ui32BytesProcessed=0;
	// buffer for serial reading
	std::ostringstream serial_read_buff;
	
	// wait a little before we send value
	System::Time::sleep(SLEEP_BEFORE_WRITE);
	
#if defined TARGET_OS_Windows

	uint32 l_ui32ReadLength=0;
	uint32 l_ui32ReadOk=0;
	uint32 l_ui32WriteOk=0;
	struct _COMSTAT l_oStatus;
	::DWORD l_dwState;
	
	// write
	unsigned int spot = 0;
	bool returnWrite = false;
	do {
		// conversion from char a bit messy...
		m_rDriverContext.getLogManager() << LogLevel_Info << "Write: " << std::string(1, cmd[spot]).c_str() << "\n";
		returnWrite = ::WriteFile(i32FileDescriptor, (LPCVOID) &cmd[spot], 1, (LPDWORD)&l_ui32WriteOk, NULL) != 0;
		spot+=l_ui32WriteOk;
		if (!returnWrite) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "Error: " << (int32)::GetLastError() << "\n";
		}
		if (sleepBetween > 0) {
			// give some time to the board to register
			m_rDriverContext.getLogManager() << LogLevel_Info << "Sleep for: " << sleepBetween << "ms" << "\n";
			System::Time::sleep(sleepBetween);
		}
	} while (spot < cmdSize && returnWrite); //traling 
	// ended before end, problem
	if (spot != cmdSize) {;
		m_rDriverContext.getLogManager() << LogLevel_Error << "Error: write stopped too early." << "\n";
		return false;
	}
	// read
	if (waitForResponse) {
		m_rDriverContext.getLogManager() << LogLevel_Info << "---- Response ----" << "\n";
		if(::ClearCommError(i32FileDescriptor, &l_dwState, &l_oStatus))
		{
			l_ui32ReadLength=l_oStatus.cbInQue;
		}

		for(l_ui32BytesProcessed=0; l_ui32BytesProcessed<l_ui32ReadLength; l_ui32BytesProcessed++)
		{
			::ReadFile(i32FileDescriptor, l_ui8ReadBuffer, 1, (LPDWORD)&l_ui32ReadOk, 0);
			if(l_ui32ReadOk==1)
			{
				serial_read_buff << l_ui8ReadBuffer[0];
			}
		}
		// serial_read_buff stream to std::string and then to const to please log manager
		m_rDriverContext.getLogManager() << LogLevel_Info  << serial_read_buff.str().c_str() << "\n";
		m_rDriverContext.getLogManager() << LogLevel_Info << "---- End response ----" << "\n";
	}
	
#elif defined TARGET_OS_Linux
	fd_set  l_inputFileDescriptorSet;
	struct timeval l_timeout;
	size_t l_ui32ReadLength=0;
	bool finished=false;

	l_timeout.tv_sec=0;
	l_timeout.tv_usec=0;
	
 	FD_ZERO(&l_inputFileDescriptorSet);
	FD_SET(i32FileDescriptor, &l_inputFileDescriptorSet);
	
	// write
	int n_written = 0;
	unsigned int spot = 0;
	do {
		// conversion from char a bit messy...
		m_rDriverContext.getLogManager() << LogLevel_Info << "Write: " << std::string(1, cmd[spot]).c_str() << "\n";
		n_written = write(i32FileDescriptor, &cmd[spot], 1 );
		if (sleepBetween > 0) {
			// give some time to the board to register
			m_rDriverContext.getLogManager() << LogLevel_Info << "Sleep for: " << sleepBetween << "ms" << "\n";
			System::Time::sleep(sleepBetween);
		} else {
			m_rDriverContext.getLogManager() << LogLevel_Info << "\n";
		}
		spot += n_written;
	} while (spot < cmdSize && n_written > 0); //traling 
	// ended before end, problem
	if (spot != cmdSize) {;
		m_rDriverContext.getLogManager() << LogLevel_Error << "Error: write stopped too early." << "\n";
		return false;
	}
	
	if (waitForResponse) {
		m_rDriverContext.getLogManager() << LogLevel_Info << "---- Response ----" << "\n";
		do
		{
			switch(::select(i32FileDescriptor+1, &l_inputFileDescriptorSet, NULL, NULL, &l_timeout))
			{
				case -1: // error or timeout
				case  0:
					finished=true;
					break;

				default:
					if(FD_ISSET(i32FileDescriptor, &l_inputFileDescriptorSet))
					{
						l_ui32ReadLength=::read(i32FileDescriptor, l_ui8ReadBuffer, sizeof(l_ui8ReadBuffer));		  
						if((l_ui32ReadLength) > 0)
						{					   
							for(l_ui32BytesProcessed=0; l_ui32BytesProcessed<l_ui32ReadLength; l_ui32BytesProcessed++)
							{
								serial_read_buff << l_ui8ReadBuffer[l_ui32BytesProcessed];
							}
						}
					}
					else
					{
						finished=true;
					}
					break;
			}
		}
		while(!finished);
		// serial_read_buff stream to std::string and then to const to please log manager
	        m_rDriverContext.getLogManager() << LogLevel_Info  << serial_read_buff.str().c_str() << "\n";
		m_rDriverContext.getLogManager() << LogLevel_Info << "---- End response ----" << "\n";
	}
#else
#endif
	// TODO: detect errors
	return true;
}

boolean CDriverOpenBCI::initBoard(::FD_TYPE i32FileDescriptor)
{
	// stop/reset/default board
	m_rDriverContext.getLogManager() << LogLevel_Info << "Stop board streaming..." << "\n";
	boardWriteAndPrint(i32FileDescriptor, "s", false, 1000);
	
	// reset 32-bit board (no effect with 8bit board)
	m_rDriverContext.getLogManager() << LogLevel_Info << "Soft reset of the board..." << "\n";
	boardWriteAndPrint(i32FileDescriptor, "v", false, 1000);
	
	// TODO: flush remaining data?
	
	// send correct config for daisy module
	if (m_bDaisyModule) {
		m_rDriverContext.getLogManager() << LogLevel_Info << "Tell the board to enable daisy module..." << "\n";
	      boardWriteAndPrint(i32FileDescriptor, "C", true, 1000);
	}
	else {
		m_rDriverContext.getLogManager() << LogLevel_Info << "Tell the board to disable daisy module..." << "\n";
		boardWriteAndPrint(i32FileDescriptor, "c", true, 1000);
	}
	
	// reset 32-bit board (no effect with 8bit board)
	m_rDriverContext.getLogManager() << LogLevel_Info << "Channels settings back to default..." << "\n";
	boardWriteAndPrint(i32FileDescriptor, "d", true, 1000);
	
	m_rDriverContext.getLogManager() << LogLevel_Info << "ComInit: [" << m_sComInit << "]" << "\n";
	if (strlen(m_sComInit) > 0) {
		boardWriteAndPrint(i32FileDescriptor, m_sComInit, true, m_ui32ComDelay);
	}
	
	
	// reset 32-bit board (no effect with 8bit board)
	m_rDriverContext.getLogManager() << LogLevel_Info << "Print register settings..." << "\n";
	boardWriteAndPrint(i32FileDescriptor, "?", true, 2500);
	
	// start stream
	m_rDriverContext.getLogManager() << LogLevel_Info << "Starting stream..." << "\n";
	boardWriteAndPrint(i32FileDescriptor, "b", false, 1000);
	
	// should start streaming!
	m_ui32tick = System::Time::getTime();
	return true;
}

// a sub-list of command compared to init; the timeout is threatening us
// FIXME: we will force the delay of users' commands here because of the 5s timeout + too many user's command will timeout...
void CDriverOpenBCI::fastReco(::FD_TYPE i32FileDescriptor)
{
	// concatenate all commands to speedup further reco
	std::ostringstream write_buff;
	// send correct config for daisy module
	if (m_bDaisyModule) {
	      write_buff << "C";
	}
	else {
	      write_buff << "c";
	}
	
	// reset 32-bit board (no effect with 8bit board)
	write_buff << "d";
	
	// command from user
	if (strlen(m_sComInit) > 0) {
		write_buff <<  m_sComInit;
	}
	
	// start stream
	write_buff << "b";

	// send all
	boardWriteAndPrint(i32FileDescriptor, write_buff.str().c_str(), false, 100);
	
	// should start streaming!
	m_ui32tick = System::Time::getTime();
}

void CDriverOpenBCI::closeTTY(::FD_TYPE i32FileDescriptor)
{
#if defined TARGET_OS_Windows
	::CloseHandle(i32FileDescriptor);
#elif defined TARGET_OS_Linux
	::close(i32FileDescriptor);
#else
#endif
}

// update internal state (lastPacket, number of packet processed, etc.).
// returns true if a new sample is created
bool CDriverOpenBCI::handleCurrentSample(int32 packetNumber) {
	bool l_bSampleOK = false; // true if a sample is added to m_vChannelBuffer
	// if == -1, current sample is incomplete or corrupted
	if (packetNumber >= 0) {
		// check packet drop
		if ((m_i16LastPacketNumber + 1) % 256 !=  packetNumber) {
			m_rDriverContext.getLogManager() << LogLevel_Warning << "Last packet drop! Last: " << (int) m_i16LastPacketNumber << ", current packet number: " << packetNumber << "\n";
		}
		
		// no daisy module: push directly values
		if (!m_bDaisyModule) {
			// concatenate EEG and Acc
			std::vector < float32 > l_vSampleBuffer;
			l_vSampleBuffer.reserve(m_vSampleEEGBuffer.size() + m_vSampleAccBuffer.size());
			l_vSampleBuffer.insert(l_vSampleBuffer.end(), m_vSampleEEGBuffer.begin(), m_vSampleEEGBuffer.end());
			l_vSampleBuffer.insert(l_vSampleBuffer.end(), m_vSampleAccBuffer.begin(), m_vSampleAccBuffer.end());
			// copy them to current chunk
			m_vChannelBuffer.push_back(l_vSampleBuffer);
			l_bSampleOK = true;
		}
		// even: daisy, odd: first 8 channels
		else {
			// on odd packet, got complete sample
			if (packetNumber % 2) {
				// won't concatenate if there was packet drop
				if ((m_i16LastPacketNumber + 1) % 256 ==  packetNumber) {
					// Average Acc values between daisy and current sample, as on the board EEG were averaged
					std::vector < float32 > l_AccAvgBuffer;
					l_AccAvgBuffer.resize(m_vSampleAccBuffer.size());
					for (size_t i = 0; i < l_AccAvgBuffer.size(); i++) {
						l_AccAvgBuffer[i] = (m_vSampleAccBuffer[i] + m_vSampleAccBufferDaisy[i]) / 2;
					}
					// Concatenate EEG values and averaged Acc
					std::vector < float32 > l_vSampleBuffer;
					l_vSampleBuffer.reserve(2*m_vSampleEEGBuffer.size() + m_vSampleAccBuffer.size());
					l_vSampleBuffer.insert(l_vSampleBuffer.end(), m_vSampleEEGBuffer.begin(), m_vSampleEEGBuffer.end());
					l_vSampleBuffer.insert(l_vSampleBuffer.end(), m_vSampleEEGBufferDaisy.begin(), m_vSampleEEGBufferDaisy.end());
					l_vSampleBuffer.insert(l_vSampleBuffer.end(), l_AccAvgBuffer.begin(), l_AccAvgBuffer.end());
					// at last, add to chunk
					m_vChannelBuffer.push_back(l_vSampleBuffer);
					l_bSampleOK = true;

				}
			}
			// an even packet: it's Daisy, store values for later
			else {
				// swap may modify origin, but it's faster
				m_vSampleEEGBufferDaisy.swap(m_vSampleEEGBuffer);
				m_vSampleAccBufferDaisy.swap(m_vSampleAccBuffer);
			}
		}
		
		m_i16LastPacketNumber = packetNumber;
	}
	
	// something to read: won't have to poll before "long"
	if (l_bSampleOK) {
		m_ui32tick = System::Time::getTime();
	}
	  
	return l_bSampleOK;
}

// fear the code duplication!
int32 CDriverOpenBCI::readPacketFromTTY(::FD_TYPE i32FileDescriptor)
{
	m_rDriverContext.getLogManager() << LogLevel_Debug << "Enters readPacketFromTTY\n";

	// try to awake the board if there's something wrong
	if (System::Time::getTime() - m_ui32tick > PollingDelay) {
		m_rDriverContext.getLogManager() << LogLevel_ImportantWarning << "No response for " << (uint32)PollingDelay << "ms, emergency reset.\n";
		fastReco(i32FileDescriptor);
	}
				
	uint8  l_ui8ReadBuffer[1]; // TODO: better size for buffer
	uint32 l_ui32BytesProcessed=0;
	int32  l_i32PacketsProcessed=0;

#if defined TARGET_OS_Windows

	uint32 l_ui32ReadLength=0;
	uint32 l_ui32ReadOk=0;
	struct _COMSTAT l_oStatus;
	::DWORD l_dwState;

	if(::ClearCommError(i32FileDescriptor, &l_dwState, &l_oStatus))
	{
		l_ui32ReadLength=l_oStatus.cbInQue;
	}

	for(l_ui32BytesProcessed=0; l_ui32BytesProcessed<l_ui32ReadLength; l_ui32BytesProcessed++)
	{
		::ReadFile(i32FileDescriptor, l_ui8ReadBuffer, 1, (LPDWORD)&l_ui32ReadOk, 0);
		if(l_ui32ReadOk==1)
		{
			// will have effect only if complete sample/packet
			if (handleCurrentSample(this->parseByte(l_ui8ReadBuffer[0]))) {
				l_i32PacketsProcessed++;
			}
		}
	}

#elif defined TARGET_OS_Linux

	fd_set  l_inputFileDescriptorSet;
	struct timeval l_timeout;
	size_t l_ui32ReadLength=0;
	bool finished=false;

	l_timeout.tv_sec=0;
	l_timeout.tv_usec=0;

	do
	{
		FD_ZERO(&l_inputFileDescriptorSet);
		FD_SET(i32FileDescriptor, &l_inputFileDescriptorSet);
		
		switch(::select(i32FileDescriptor+1, &l_inputFileDescriptorSet, NULL, NULL, &l_timeout))
		{
			case -1: // error or timeout
			case  0:
				finished=true;
				break;

			default:
				if(FD_ISSET(i32FileDescriptor, &l_inputFileDescriptorSet))
				{
					l_ui32ReadLength=::read(i32FileDescriptor, l_ui8ReadBuffer, sizeof(l_ui8ReadBuffer));		  
					if((l_ui32ReadLength) > 0)
					{					   
						for(l_ui32BytesProcessed=0; l_ui32BytesProcessed<l_ui32ReadLength; l_ui32BytesProcessed++)
						{
							// will have effect only if complete sample/packet
							if (handleCurrentSample(this->parseByte(l_ui8ReadBuffer[l_ui32BytesProcessed]))) {
								l_i32PacketsProcessed++;
							}
						}
					}
				}
				else
				{
					finished=true;
				}
				break;
		}
	}
	while(!finished);
#endif

	m_rDriverContext.getLogManager() << LogLevel_Debug << "Leaves readPacketFromTTY\n";
	return l_i32PacketsProcessed;
 }


