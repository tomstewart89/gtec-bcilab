#include "ovasCDriverCognionics.h"
#include "ovasCConfigurationCognionics.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(WIN32)

#include <windows.h>


#include <toolkit/ovtk_all.h>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

//Cognionics functions

int init_SPP(int port);
void getData();
double get_IMP_CH(int CH);
void close_SPP();
unsigned long grab_SPP(int bytes, unsigned char* buf);
void write_SPP(int bytes, unsigned char* buf);

//Cognionics defs
#define GAIN 3.0
#define VREF 2.5
#define ISTIM 0.000000024
#define ADC_TO_VOLTS 2.0*(VREF/(4294967296.0*GAIN))
#define TO_Z 1.4/(ISTIM*2.0)

//Cognionics variables
int EEG_GRAB;
int CHS;
int SAMPLE_RATE;
float32* dataBufferPtr;
int prev_trigger;
OpenViBE::CStimulationSet cogStimulationSet;

//Cognionics Buffers
int* prev_4_samples; //buffer for previous 4 samples in each channel for computing MA and IMP



//___________________________________________________________________//
//                                                                   //

CDriverCognionics::CDriverCognionics(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_oSettings("AcquisitionServer_Driver_Cognionics", m_rDriverContext.getConfigurationManager())
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
	,m_ui32ComPort(1)
{
	m_oHeader.setSamplingFrequency(300);
	m_oHeader.setChannelCount(64);

	m_oSettings.add("Header", &m_oHeader);
	m_oSettings.add("ComPort", &m_ui32ComPort);
	m_oSettings.load();
}

CDriverCognionics::~CDriverCognionics(void)
{
}

const char* CDriverCognionics::getName(void)
{
	return "Cognionics";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverCognionics::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) return false;
	if(!m_oHeader.isChannelCountSet()||!m_oHeader.isSamplingFrequencySet()) return false;

	// Builds up a buffer to store
	// acquired samples. This buffer
	// will be sent to the acquisition
	// server later...
	m_pSample=new float32[m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock];

	//initalize Cognionics varaibles, pointers and buffers
	EEG_GRAB = ui32SampleCountPerSentBlock;
	SAMPLE_RATE = m_oHeader.getSamplingFrequency();
	CHS = m_oHeader.getChannelCount();
	dataBufferPtr = &m_pSample[0];
	prev_4_samples = new int[EEG_GRAB*CHS];

	if(!m_pSample)
	{
		delete [] m_pSample;
		delete [] prev_4_samples;
		m_pSample=NULL;
		return false;
	}

	// ...
	// initialize hardware and get
	// available header information
	// from it
	// Using for example the connection ID provided by the configuration (m_ui32ConnectionID)
	// ...

	//init serial port
	m_rDriverContext.getLogManager() << LogLevel_Info << "Attempting to Connect to Device at COM Port: " << m_ui32ComPort <<"\n";

	int ser_success = init_SPP(m_ui32ComPort);
	if(ser_success == -1)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Unable to Open Port, Please Check Device and Settings\n";
		return false;
	}
	m_rDriverContext.getLogManager() << LogLevel_Info << "Successfully Opened Port at COM: " << m_ui32ComPort <<"\n";
	m_rDriverContext.getLogManager() << LogLevel_Info << "Number Channels (EEG + Packet Counter + Trigger): " << m_oHeader.getChannelCount() <<"\n";
	m_rDriverContext.getLogManager() << LogLevel_Info << "Sampling Rate: " << m_oHeader.getSamplingFrequency() <<"\n";

	for(uint32 c=0;c<m_oHeader.getChannelCount();c++)
	{
		m_oHeader.setChannelUnits(c, OVTK_UNIT_Volts, OVTK_FACTOR_Base);
	}

	//set impedance check mode ON
	getData();
	unsigned char imp_on = 0x11;
	write_SPP(1, &imp_on);

	// Saves parameters
	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
	return true;
}

boolean CDriverCognionics::start(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;

	// ...
	// request hardware to start
	// sending data
	// ...

	//set impedance check mode off
	getData();
	unsigned char imp_off = 0x12;
	write_SPP(1, &imp_off);

	return true;
}

boolean CDriverCognionics::loop(void)
{
	if(!m_rDriverContext.isConnected()) return false;

	if(m_rDriverContext.isStarted()){

		// ...
		// receive samples from hardware
		// put them the correct way in the sample array
		// whether the buffer is full, send it to the acquisition server
		//...

		//get the number of samples specified
		getData();

		//OpenVibe call back for new samples
		m_pCallback->setSamples(m_pSample);

		// When your sample buffer is fully loaded, 
		// it is advised to ask the acquisition server 
		// to correct any drift in the acquisition automatically.
		m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());

		// ...
		// receive events from hardware
		// and put them the correct way in a CStimulationSet object
		//...
		m_pCallback->setStimulationSet(cogStimulationSet);
		cogStimulationSet.clear();
	}
	else
	{
		//get a fresh batch of samples to keep connection alive
		getData();
		if(m_rDriverContext.isImpedanceCheckRequested())
		{
			//update impedance values
			int c;
			for(c=0; c<CHS; c++)
			{
				m_rDriverContext.updateImpedance(c, get_IMP_CH(c));
			}
		}
	}

	return true;
}

boolean CDriverCognionics::stop(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(!m_rDriverContext.isStarted()) return false;

	// ...
	// request the hardware to stop
	// sending data
	// ...

	//set impedance check mode on
	getData();
	unsigned char imp_off = 0x11;
	write_SPP(1, &imp_off);

	return true;
}

boolean CDriverCognionics::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;

	// ...
	// uninitialize hardware here
	// ...
	close_SPP();

	//free buffers
	delete [] m_pSample;
	delete [] prev_4_samples;
	m_pSample=NULL;
	m_pCallback=NULL;

	return true;
}

//___________________________________________________________________//
//                                                                   //
boolean CDriverCognionics::isConfigurable(void)
{
	return true; // change to false if your device is not configurable
}

boolean CDriverCognionics::configure(void)
{

	// Change this line if you need to specify some references to your driver attribute that need configuration, e.g. the connection ID.
	CConfigurationCognionics m_oConfiguration(m_rDriverContext, 
		OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-Cognionics.ui", m_ui32ComPort); // the specific header is passed into the specific configuration

	if(!m_oConfiguration.configure(m_oHeader))
	{
		return false;
	}

	m_oSettings.save();

	return true;
}

void getData(){

	unsigned char packet_start=0;
	int msb, lsb2, lsb1;
	unsigned char temp;

	double eeg_sample;

	for(int c=0; c<EEG_GRAB; c++){
		packet_start = 0;

		//wait for packet start
		while(packet_start!=0xFF){
			grab_SPP(1, &packet_start);
		}

		//get packet counter
		grab_SPP(1, &packet_start);

		//grab CHS-2 since the last two channels are for sync and packet counter
		for(int j=0; j<(CHS-2); j++){

			grab_SPP(1, &temp);
			msb = temp;
			grab_SPP(1, &temp);
			lsb2 = temp;
			grab_SPP(1, &temp);
			lsb1 = temp;

			//reassemble 24-bit 2's compltement promoted to 32  bit int
			msb =  (msb<<24)|(lsb2<<17)|(lsb1<<10);

			//shift previous 4 sample buffer
			*(prev_4_samples + j*4 + 3) = *(prev_4_samples + j*4 + 2);
			*(prev_4_samples + j*4 + 2) = *(prev_4_samples + j*4 + 1);
			*(prev_4_samples + j*4 + 1) = *(prev_4_samples + j*4 + 0);
			*(prev_4_samples + j*4 + 0) = msb;

			eeg_sample = ADC_TO_VOLTS*(double) msb;

			//4-point MA for devices without way to disable impedance
			/*
			eeg_sample = ADC_TO_VOLTS * ((double) *(prev_4_samples + j*4 + 3) +
			*(prev_4_samples + j*4 + 2) +
			*(prev_4_samples + j*4 + 1) +
			*(prev_4_samples + j*4 + 0) )/4.0;
			*/

			*(dataBufferPtr+j*EEG_GRAB+c) = (float32) eeg_sample;
		}


		//save packet counter;
		*(dataBufferPtr+(CHS-1)*EEG_GRAB+c) = (float32) packet_start;

		//dummy data
		grab_SPP(1, &temp);
		grab_SPP(1, &temp);

		//grab trigger
		grab_SPP(1, &temp);
		lsb2 = temp;
		grab_SPP(1, &temp);
		lsb1 = temp;

		//assemble trigger code
		int new_trigger = (lsb2<<8) | lsb1;
		//downshift serial triggers if necessary
		if(new_trigger > 255)
			new_trigger = new_trigger>>8;

		//store in EEG channel
		*(dataBufferPtr+(CHS-2)*EEG_GRAB+c) = (float32) new_trigger;

		//detect new stimulation
		if(new_trigger != prev_trigger)
		{
			//time offset from start of chunk (c - current sample in the chunk)
			uint64 l_ui64Date = c * (1LL<<32) / SAMPLE_RATE;
			cogStimulationSet.appendStimulation(OVTK_StimulationId_Label(new_trigger&0x000000ff), l_ui64Date, 0);
		}

		prev_trigger = new_trigger;
	}
}

double get_IMP_CH(int ch) {

	double d1 = (double) *(prev_4_samples + ch*4 + 2) -  (double) *(prev_4_samples + ch*4 + 0);
	double d2 = (double) *(prev_4_samples + ch*4 + 3) -  (double) *(prev_4_samples + ch*4 + 1);
	d1 = d1*ADC_TO_VOLTS;
	d2 = d2*ADC_TO_VOLTS;

	if(d1<0)
		d1 = -d1;
	if(d2<0)
		d2 = -d2;

	if(d2>d1)
		d1 = d2;

	double impedance = d1*TO_Z;

	return impedance;

}

//serial port handling

HANDLE hSerial;
COMMTIMEOUTS timeouts={0};

int init_SPP(int port){

	char com[100];

	sprintf(com, "\\\\.\\COM%d", port);

	hSerial = CreateFile(com,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if(hSerial==INVALID_HANDLE_VALUE){
		return -1;
	}

	//setup serial port parameters
	DCB dcbSerialParams = {0};

	if (!GetCommState(hSerial, &dcbSerialParams)) {
		//error getting state
	}

	dcbSerialParams.BaudRate=1500000;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=ONESTOPBIT;
	dcbSerialParams.Parity=NOPARITY;

	if(!SetCommState(hSerial, &dcbSerialParams)){
		//error setting serial port state
	}

	//setup serial port timeout
	COMMTIMEOUTS timeouts={0};
	timeouts.ReadIntervalTimeout=500;
	timeouts.ReadTotalTimeoutConstant=50;
	timeouts.ReadTotalTimeoutMultiplier=10;

	timeouts.WriteTotalTimeoutConstant=50;
	timeouts.WriteTotalTimeoutMultiplier=10;
	if(!SetCommTimeouts(hSerial, &timeouts)){ 
		//error occureed. Inform user
	}
	return 0;
}

void close_SPP(){
	CloseHandle(hSerial);
}

unsigned long grab_SPP(int bytes, unsigned char* buf){
	DWORD dwBytesRead = 0;
	ReadFile(hSerial, buf, bytes, &dwBytesRead, NULL);

	int a = dwBytesRead;
	return a;
}

void write_SPP(int bytes, unsigned char* buf){
	DWORD dwBytesWritten = 0;	
	WriteFile(hSerial, buf, bytes, &dwBytesWritten, NULL);
}

#endif //WIN32
