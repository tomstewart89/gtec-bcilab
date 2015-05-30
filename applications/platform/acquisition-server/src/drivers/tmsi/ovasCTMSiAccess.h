/*
 * ovasCTMSiAccess.h
 *
 * Copyright (c) 2014, Mensia Technologies SA. All rights reserved.
 * -- Rights transferred to Inria, contract signed 21.11.2014
 *
 */

#ifndef __OpenViBE_AcquisitionServer_CTMSiAccess_H__
#define __OpenViBE_AcquisitionServer_CTMSiAccess_H__

#if defined TARGET_OS_Windows

#include "ovas_base.h"
#include "Windows.h"
//#include "TMSiSDK.h"

#include <map>
#include <vector>

// TMSi Declared Defines, Enums and Types

// Measurement modes:
#define MEASURE_MODE_NORMAL			((ULONG)0x0)
#define MEASURE_MODE_IMPEDANCE		((ULONG)0x1)
#define MEASURE_MODE_CALIBRATION	((ULONG)0x2)

#define MEASURE_MODE_IMPEDANCE_EX	((ULONG)0x3)
#define MEASURE_MODE_CALIBRATION_EX	((ULONG)0x4)

// for MEASURE_MODE_IMPEDANCE:
#define IC_OHM_002	0 /*!< 2K Impedance limit */
#define IC_OHM_005	1 /*!<  5K Impedance limit */
#define IC_OHM_010	2 /*!<  10K Impedance limit */
#define IC_OHM_020	3 /*!<  20K Impedance limit */
#define IC_OHM_050	4 /*!<  50K Impedance limit */
#define IC_OHM_100	5 /*!<  100K Impedance limit */
#define IC_OHM_200	6 /*!<  200K Impedance limit */

// for MEASURE_MODE_CALIBRATION:
#define IC_VOLT_050 0	/*!< 50 uV t-t Calibration voltage */
#define IC_VOLT_100 1	/*!< 100 uV t-t Calibration voltage */
#define IC_VOLT_200 2	/*!< 200 uV t-t Calibration voltage */
#define IC_VOLT_500 3	/*!< 500 uV t-t Calibration voltage */

 // for Signat Format
#define SF_UNSIGNED 0x0   // Unsigned integer
#define SF_INTEGER  0x1	  // signed integer

// integer overflow value for analog channels
#define OVERFLOW_32BITS ((long) 0x80000000)

// Get Signal info
#define SIGNAL_NAME 40

typedef struct _SIGNAL_FORMAT
{
	ULONG Size;		 // Size of this structure
	ULONG Elements;	 // Number of elements in list

	ULONG Type;		 // One of the signal types above
	ULONG SubType;	 // One of the signal sub-types above
	ULONG Format;    // Float / Integer / Asci / Ect..
	ULONG Bytes;	 // Number of bytes per sample including subsignals

	FLOAT UnitGain;
	FLOAT UnitOffSet;
	ULONG UnitId;
	LONG UnitExponent;

	WCHAR Name[SIGNAL_NAME];

	ULONG Port;
	WCHAR PortName[SIGNAL_NAME];
	ULONG SerialNumber;

}SIGNAL_FORMAT, *PSIGNAL_FORMAT;

// This structure contains information about the possible configuration of the frontend
typedef struct _FRONTENDINFO
{	unsigned short NrOfChannels;	/*!<  Current number of channels used */
	unsigned short SampleRateSetting;	/*!<  Current sample rate setting (a.k.a. base sample rate divider ) */
	unsigned short Mode;		/*!<  operating mode */
	unsigned short maxRS232;
	unsigned long Serial;    	/*!<  Serial number */
	unsigned short NrExg;       /*!<  Number of Exg channels in this device */
	unsigned short NrAux;		/*!<  Number of Aux channels in this device */
	unsigned short HwVersion;	/*!<  Version number for the hardware */
	unsigned short SwVersion;	/*!<  Version number of the embedded software */
	unsigned short RecBufSize;	/*!<  Used for debugging only */
	unsigned short SendBufSize;	/*!<  Used for debugging only */
	unsigned short NrOfSwChannels;   /*!<  Max. number of channels supported by this device */
	unsigned short BaseSf;		/*!<  Max. sample frequency */
	unsigned short Power;
	unsigned short Check;
}FRONTENDINFO,*PFRONTENDINFO;

// Enum defined based on the communication methods from TMSiExtFrontendInfoType
typedef enum _TMSiConnectionEnum {
	TMSiConnectionUndefined = 0,	/*!< Undefined connection, indicates programming error */
	TMSiConnectionFiber,			/*!< Obsolete, do not use */
	TMSiConnectionBluetooth,		/*!< Bluetooth connection with Microsoft driver */
	TMSiConnectionUSB,			/*!< USB 2.0 connection direct */
	TMSiConnectionWifi,			/*!< Network connection, Ip-adress and port needed, wireless */
	TMSiConnectionNetwork		/*!< Network connection, Ip-adress and port needed, wired */
} TMSiConnectionType;


// Mobita specific: This structure contains information about the current battery state
typedef struct TMSiBatReport {
	short Temp;					/*!<  Battery temperatur in degree Celsius (�C) */
	short Voltage; 				/*!<  Battery Voltage in milliVolt  (mV) */
	short Current;				/*!<  Battery Current in milliAmpere (mA) */
	short AccumCurrent; 		/*!<  Battery Accumulated Current in milliAmpere (mA) */
	short AvailableCapacityInPercent; /*!<  Available battery Capacity In Percent, range 0-100 */
	unsigned short  DoNotUse1;	/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse2;	/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse3;	/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse4;	/*!<  Do not use, reserved for future use */
} TMSiBatReportType;

// Mobita specific: This structure contains information about the current state of the internal storage
typedef struct TMSiStorageReport
{
	unsigned int	StructSize;		/*!<  Size of struct in words */
	unsigned int 	TotalSize; 		/*!<  Total size of the internal storage in MByte (=1024x1024 bytes) */
	unsigned int 	UsedSpace;		/*!<  Used space on the internal storage in MByte (=1024x1024 bytes)*/
	unsigned int	SDCardCID[4];	/*!<  The CID register of the current SD-Card. */
	unsigned short	DoNotUse1;		/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse2;		/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse3;		/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse4;		/*!<  Do not use, reserved for future use */
} TMSiStorageReportType;

// Mobita specific: This structure contains information about the current and past use of the Mobita
typedef struct TMSiDeviceReport
{
	unsigned int	AdapterSN;		/*!<  Serial number of the current connected Adapter */
	unsigned int	AdapterStatus;	/*!<  0=Unknown; 1=Ok;2=MemError */
	unsigned int	AdapterCycles;	/*!<  Number of connections made by the Adapter. */
	unsigned int	MobitaSN;		/*!<  Serial number of the Mobita */
	unsigned int	MobitaStatus;	/*!<  Statis of the Mobita : 0=Unknown; 1=Ok;2=MemError;3=BatError; */
	unsigned int	MobitaCycles;	/*!<  Number of adapter connections made by the Mobita */
	unsigned short	DoNotUse1;		/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse2;		/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse3;		/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse4;		/*!<  Do not use, reserved for future use */
} TMSiDeviceReportType;

// Mobita specific: This structure contains information about the current sampling configuration
typedef struct TMSiExtFrontendInfo
{
	unsigned short	CurrentSamplerate;	/*!<  in Hz */
	unsigned short	CurrentInterface;   /*!<  0 = Unknown; 1 = Fiber;  2 = Bluetooth; 3 = USB; 4 = WiFi; 5 = Network*/
	unsigned short	CurrentBlockType; 	/*!<  The blocktype used to send sample data for the selected CurrentFs and selected CurrentInterface */
	unsigned short	DoNotUse1;			/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse2;			/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse3;			/*!<  Do not use, reserved for future use */
	unsigned short	DoNotUse4;			/*!<  Do not use, reserved for future use */
} TMSiExtFrontendInfoType;
//----------- TYPE ---------------------

#define CHANNELTYPE_UNKNOWN 0
#define CHANNELTYPE_EXG 1
#define CHANNELTYPE_BIP 2
#define CHANNELTYPE_AUX 3
#define CHANNELTYPE_DIG 4
#define CHANNELTYPE_TIME 5
#define CHANNELTYPE_LEAK 6
#define CHANNELTYPE_PRESSURE 7
#define CHANNELTYPE_ENVELOPE 8
#define CHANNELTYPE_MARKER 9
#define CHANNELTYPE_SAW 10
#define CHANNELTYPE_SAO2 11

#define MAX_BUFFER_SIZE 0xFFFFFFFF

typedef BOOLEAN			( __stdcall * POPEN	)			(void *Handle, const char *DeviceLocator );
typedef BOOLEAN			( __stdcall * PCLOSE ) 			(HANDLE hHandle);
typedef BOOLEAN			( __stdcall * PSTART)			(IN HANDLE Handle);
typedef BOOLEAN			( __stdcall * PSTOP)  			(IN HANDLE Handle);
typedef BOOLEAN			( __stdcall * PSETSIGNALBUFFER)	(IN HANDLE Handle,IN OUT PULONG SampleRate,IN OUT PULONG BufferSize);
typedef BOOLEAN			( __stdcall * PGETBUFFERINFO)	(IN HANDLE Handle,OUT PULONG Overflow,OUT PULONG PercentFull);
typedef LONG			( __stdcall * PGETSAMPLES)		(IN HANDLE Handle,OUT PULONG SampleBuffer,IN ULONG Size);
typedef PSIGNAL_FORMAT	( __stdcall * PGETSIGNALFORMAT)     (IN HANDLE Handle, IN OUT char* FrontEndName);
typedef BOOLEAN			( __stdcall * PFREE)			(IN VOID *Memory);
typedef HANDLE			( __stdcall * PLIBRARYINIT)		(IN TMSiConnectionType GivenConnectionType, IN OUT int *ErrorCode );
typedef int				( __stdcall * PLIBRARYEXIT)		(IN HANDLE Handle);
typedef BOOLEAN			( __stdcall * PGETFRONTENDINFO)	(IN HANDLE Handle, IN OUT FRONTENDINFO *FrontEndInfo );
typedef BOOLEAN			( __stdcall * PSETRTCTIME)		(IN HANDLE Handle,IN SYSTEMTIME *InTime );
typedef BOOLEAN			( __stdcall * PGETRTCTIME)		(IN HANDLE Handle,IN SYSTEMTIME *InTime );
typedef int				( __stdcall * PGETERRORCODE)	( IN HANDLE Handle );
typedef const char*		( __stdcall * PGETERRORCODEMESSAGE)( IN HANDLE Handle, IN int ErrorCode );
typedef char**			( __stdcall * PGETDEVICELIST)		( IN HANDLE Handle, IN OUT int *NrOfFrontEnds);
typedef void			( __stdcall * PFREEDEVICELIST)	( HANDLE Handle, int NrOfFrontEnds, char** DeviceList );
typedef BOOLEAN			( __stdcall * PSETREFCALCULATION)(IN HANDLE Handle, int OnOrOff );
typedef BOOLEAN			( __stdcall * PSETMEASURINGMODE)(IN HANDLE Handle,IN ULONG Mode, IN int Value );
typedef BOOLEAN			( __stdcall * PGETCONNECTIONPROPERTIES)( IN HANDLE Handle, IN OUT unsigned int *SignalStrength,
									   IN OUT unsigned int *NrOfCRCErrors, IN OUT unsigned int *NrOfSampleBlocks );
typedef BOOLEAN			( __stdcall * PGETEXTFRONTENDINFO)( IN HANDLE Handle, IN OUT TMSiExtFrontendInfoType *ExtFrontEndInfo,
									TMSiBatReportType *BatteryReport,
									TMSiStorageReportType *StorageReport,
									TMSiDeviceReportType *DeviceReport );


// END TMSi

namespace OpenViBEAcquisitionServer
{
	class IDriverContext;
	class IDriverCallback;
	class CHeader;

	class CTMSiAccess
	{

	public:
		CTMSiAccess(IDriverContext& rDriverContext);
		~CTMSiAccess();

		std::map<OpenViBE::CString, std::pair<_TMSiConnectionEnum, int> > getConnectionProtocols() { return m_mConnectionProtocols; }

		// Initialize the TMSi library with the currently chosen protocol
		OpenViBE::boolean initializeTMSiLibrary(const char* sConnectionProtocol);

		// Open a frontend (identified by a string) on the currently set protocol
		OpenViBE::boolean openFrontEnd(const char* sDeviceIdentifier);

		// Close the currently opened frontend
		OpenViBE::boolean closeFrontEnd();

		// Returns a vector of available sampling frequencies of the device, in case of error returns an empty vector
		std::vector<unsigned long> discoverDeviceSamplingFrequencies();

		// Run diagnostics on the device, ask for FrontEndInfo and ExtFrontEndInfo
		OpenViBE::boolean runDiagnostics();

		OpenViBE::boolean getImpedanceTestingCapability(bool* pHasImpedanceTestingAbility);

		// Initializes the SignalFormat structure inside the driver
		// Calculates number of EEG and additional Channels
		OpenViBE::boolean calculateSignalFormat(const char* sDeviceIdentifier);

		// Print the signal format into Trace Log
		OpenViBE::boolean printSignalFormat();

		// Return the number of EEG channels on the device (must call calculateSignalFormat first)
		OpenViBE::uint32 getMaximumEEGChannelCount() { return m_ulMaxEEGChannelCount; }

		// Return the number of all channels on the device (must call calculateSignalFormat first)
		OpenViBE::uint32 getActualChannelCount() { return m_ulActualChannelCount; }

		// Return the name of the channel at desired index
		OpenViBE::CString getChannelName(size_t uiChannelIndex);

		// Return the type of the channel at desired index
		OpenViBE::CString getChannelType(size_t uiChannelIndex);

		// Frees the SignalFormat structure in this object and in the library
		void freeSignalFormat();

		// Returns the list of devices found on the current protocol
		std::vector<OpenViBE::CString> getDeviceList() { return m_vDeviceList; }

		// ACQUISITION SETTINGS

		// Enable or disable the common average reference calculation
		OpenViBE::boolean setCommonModeRejection(OpenViBE::boolean bIsCommonModeRejectionEnabled);

		OpenViBE::boolean setActiveChannels(CHeader* pHeader, OpenViBE::CString sAdditionalChannels);

		// sets the signal buffer to values known to be functional, returns false if they are inconsistent
		OpenViBE::boolean setSignalBuffer(unsigned long ulSamplingFrequency, unsigned long ulBufferSizeInSamples);

		OpenViBE::boolean setSignalMeasuringModeToNormal();
		OpenViBE::boolean setSignalMeasuringModeToImpedanceCheck(int iImpedanceLimit);
		OpenViBE::boolean setSignalMeasuringModeToCalibration();

		// ACQUISITION HANDLING

		OpenViBE::boolean startAcquisition();
		OpenViBE::int32 getSamples(OpenViBE::float32 *pSamples, IDriverCallback* pDriverCallback, OpenViBE::uint64 ui64SampleCountPerSentBlock);
		OpenViBE::boolean getImpedanceValues(std::vector<OpenViBE::float64>* pvImpedanceValues);
		OpenViBE::boolean stopAcquisition();


		// Gets connection properties, such as signal strength or errors since last call and prints them to Trace Log
		OpenViBE::boolean getConnectionProperties();

	private: // private variables

		// status holders
		OpenViBE::boolean m_bInitialized;
		OpenViBE::boolean m_bOpened;
		OpenViBE::boolean m_bHasChannelStructure;
		OpenViBE::boolean m_bHasBufferSet;
		OpenViBE::boolean m_bStarted;

		// informations about the last scanned protocol
		std::vector<OpenViBE::CString> m_vDeviceList;

		// informations about the last scanned device
		PSIGNAL_FORMAT m_pSignalFormat;
		unsigned long m_ulMaxBufferSize;

		unsigned long m_ulMaxEEGChannelCount;
		unsigned long m_ulActualChannelCount;
		std::vector<OpenViBE::boolean> m_vIsChannelActivated;
		OpenViBE::uint32 m_ui32ActiveChannelcount;

		unsigned long m_ulSignalBufferSizeInBytes;

		// buffer for stored signal
		unsigned long* m_pSampleBuffer;
		// index of the current (last) sample in the buffer
		OpenViBE::uint32 m_ui32LastSampleIndexInBuffer;

		// device connection protocols
		std::map<OpenViBE::CString, std::pair<_TMSiConnectionEnum, int> > m_mConnectionProtocols;

		IDriverContext& m_rDriverContext;
		OpenViBE::boolean m_bValid;

	private: // private methods

		OpenViBE::boolean setSignalMeasuringMode(ULONG ulMeasuringMode, int iValue = 0);

	private: // DLL library handling members

		// The HANDLE type returned by the library is basically a void* (avoids including windows headers to the .h)
		HANDLE m_hLibraryHandle;

		template<typename T>
		void loadDLLfunct(T* functionPointer, const char* functionName);

		// TMSi Library functions
		POPEN m_fpOpen;
		PCLOSE m_fpClose;
		PSTART m_fpStart;
		PSTOP m_fpStop;
		PSETSIGNALBUFFER m_fpSetSignalBuffer;
		PGETBUFFERINFO m_fpGetBufferInfo;
		PGETSAMPLES m_fpGetSamples;
		PGETSIGNALFORMAT m_fpGetSignalFormat;
		PFREE m_fpFree;
		PLIBRARYINIT m_fpLibraryInit;
		PLIBRARYEXIT m_fpLibraryExit;
		PGETFRONTENDINFO m_fpGetFrontEndInfo;
		PSETRTCTIME m_fpSetRtcTime;
		PGETRTCTIME m_fpGetRtcTime;

		PGETERRORCODE m_fpGetErrorCode;
		PGETERRORCODEMESSAGE m_fpGetErrorCodeMessage;

		PGETDEVICELIST m_fpGetDeviceList;
		PFREEDEVICELIST m_fpFreeDeviceList;

		PGETCONNECTIONPROPERTIES m_fpGetConnectionProperties;
		PSETREFCALCULATION m_fpSetRefCalculation;
		PSETMEASURINGMODE m_fpSetMeasuringMode;
		PGETEXTFRONTENDINFO m_fpGetExtFrontEndInfo;

		/*
		// NeXus10MkII functionality
		PGETRANDOMKEY m_fpGetRandomKey;
		PUNLOCKFRONTEND m_fpUnlockFrontEnd;
		PGETOEMSIZE m_fpGetOEMSize;
		PSETOEMDATA m_fpSetOEMData;
		PGETOEMDATA m_fpGetOEMData;
		POPENFIRSTDEVICE m_oFopenFirstDevice;
		PSETSTORAGEMODE m_fpSetStorageMode;
		PGETDIGSENSORID m_fpGetDigSensorId;
		PGETDIGSENSORCONFIG m_fpGetDigSensorConfig;
		PGETDIGSENSORDATA m_fpGetDigSensorData;
		PGETFLASHSTATUS m_fpGetFlashStatus;
		PSTARTFLASHDATA m_fpStartFlashData;
		PGETFLASHSAMPLES m_fpGetFlashSamples;
		PSTOPFLASHDATA m_fpStopFlashData;
		PFLASHERASEMEMORY m_fpFlashEraseMemory;
		PSETFLASHDATA m_fpSetFlashData;
		*/
	};
}

#endif

#endif // __OpenViBE_AcquisitionServer_CTMSiAccess_H__
