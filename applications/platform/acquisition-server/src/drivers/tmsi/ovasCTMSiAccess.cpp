/*
 * ovasCTMSiAccess.cpp
 *
 * Copyright (c) 2014, Mensia Technologies SA. All rights reserved.
 * -- Rights transferred to Inria, contract signed 21.11.2014
 *
 */

#if defined TARGET_HAS_ThirdPartyTMSi

#include "ovasIDriver.h"
#include "../ovasCHeader.h"
#include "ovasCTMSiAccess.h"

#include <iostream>
#include <string>
#include <gtk/gtk.h>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;

static const OpenViBE::CString s_sTMSiDLL = "TMSiSDK.dll";
static const unsigned long s_ulLengthOfBufferInSeconds = 10;
static const int s_iCalibrationVoltage = IC_VOLT_050;
static const int s_iImpedanceLimit = IC_OHM_005;

HINSTANCE m_oLibTMSi; // Library Handle

template<typename T>
void CTMSiAccess::loadDLLfunct(T* functionPointer, const char* functionName)
{
	*functionPointer = (T)::GetProcAddress(m_oLibTMSi, functionName);
	if (!*functionPointer)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "(TMSi) " << "Load method " << functionName << "\n";
		m_bValid=false;
	}
}

CTMSiAccess::CTMSiAccess(IDriverContext& rDriverContext)
    : m_rDriverContext(rDriverContext),
      m_bInitialized(false),
      m_bOpened(false),
      m_bHasChannelStructure(false),
      m_bStarted(false)
{
	// Create a map of available protocols, each protocol has an Enum value coming from TMSi and an index (used for the dropbown box)
	m_mConnectionProtocols["USB"] = std::make_pair(TMSiConnectionUSB, 0);
	m_mConnectionProtocols["WiFi"] = std::make_pair(TMSiConnectionWifi, 1);
	m_mConnectionProtocols["Network"] = std::make_pair(TMSiConnectionNetwork, 2);
	m_mConnectionProtocols["Bluetooth"] = std::make_pair(TMSiConnectionBluetooth, 3);

	m_hLibraryHandle = NULL;
	m_pSampleBuffer = NULL;
	m_pSignalFormat = NULL;

	// Load the Mensia Acquisition Library
	OpenViBE::CString l_sPath = m_rDriverContext.getConfigurationManager().expand("${Path_Bin}") + "/" + s_sTMSiDLL;
	m_oLibTMSi = ::LoadLibrary(l_sPath.toASCIIString());

	if (m_oLibTMSi == NULL)
	{
		const DWORD l_dwWindowsError = GetLastError();
		m_rDriverContext.getLogManager() << LogLevel_Error << "(TMSi) " << "Can not load library, windows error = " << static_cast<uint64>(l_dwWindowsError) << "\n";
	}

	m_bValid = true;

	loadDLLfunct<POPEN>(&m_fpOpen, "Open");
	loadDLLfunct<PCLOSE>(&m_fpClose, "Close");
	loadDLLfunct<PSTART>(&m_fpStart, "Start");
	loadDLLfunct<PSTOP>(&m_fpStop, "Stop");
	loadDLLfunct<PSETSIGNALBUFFER>(&m_fpSetSignalBuffer, "SetSignalBuffer");
	loadDLLfunct<PGETBUFFERINFO>(&m_fpGetBufferInfo, "GetBufferInfo");
	loadDLLfunct<PGETSAMPLES>(&m_fpGetSamples, "GetSamples");
	loadDLLfunct<PGETSIGNALFORMAT>(&m_fpGetSignalFormat, "GetSignalFormat");
	loadDLLfunct<PFREE>(&m_fpFree, "Free");
	loadDLLfunct<PLIBRARYINIT>(&m_fpLibraryInit, "LibraryInit");
	loadDLLfunct<PLIBRARYEXIT>(&m_fpLibraryExit, "LibraryExit");
	loadDLLfunct<PGETFRONTENDINFO>(&m_fpGetFrontEndInfo, "GetFrontEndInfo");

	loadDLLfunct<PSETRTCTIME>(&m_fpSetRtcTime, "SetRtcTime");
	loadDLLfunct<PGETRTCTIME>(&m_fpGetRtcTime, "GetRtcTime");
	loadDLLfunct<PGETERRORCODE>(&m_fpGetErrorCode, "GetErrorCode");
	loadDLLfunct<PGETERRORCODEMESSAGE>(&m_fpGetErrorCodeMessage, "GetErrorCodeMessage");

	loadDLLfunct<PGETDEVICELIST>(&m_fpGetDeviceList, "GetDeviceList");
	loadDLLfunct<PFREEDEVICELIST>(&m_fpFreeDeviceList, "FreeDeviceList");

	loadDLLfunct<PSETREFCALCULATION>(&m_fpSetRefCalculation, "SetRefCalculation");
	loadDLLfunct<PGETCONNECTIONPROPERTIES>(&m_fpGetConnectionProperties, "GetConnectionProperties");
	loadDLLfunct<PSETMEASURINGMODE>(&m_fpSetMeasuringMode, "SetMeasuringMode");
	loadDLLfunct<PGETEXTFRONTENDINFO>(&m_fpGetExtFrontEndInfo, "GetExtFrontEndInfo");

	/*
	// NeXus10MkII functionality
	loadDLLfunct<PGETRANDOMKEY>(&m_fpGetRandomKey, "GetRandomKey");
	loadDLLfunct<PUNLOCKFRONTEND>(&m_fpUnlockFrontEnd, "UnlockFrontEnd");
	loadDLLfunct<PGETOEMSIZE>(&m_fpGetOEMSize, "GetOEMSize");
	loadDLLfunct<PSETOEMDATA>(&m_fpSetOEMData, "SetOEMData");
	loadDLLfunct<PGETOEMDATA>(&m_fpGetOEMData, "GetOEMData");
	loadDLLfunct<POPENFIRSTDEVICE>(&m_oFopenFirstDevice, "OpenFirstDevice");
	loadDLLfunct<PSETSTORAGEMODE>(&m_fpSetStorageMode, "SetStorageMode");
	loadDLLfunct<PGETFLASHSTATUS>(&m_fpGetFlashStatus, "GetFlashStatus");
	loadDLLfunct<PSTARTFLASHDATA>(&m_fpStartFlashData, "StartFlashData");
	loadDLLfunct<PGETFLASHSAMPLES>(&m_fpGetFlashSamples, "GetFlashSamples");
	loadDLLfunct<PSTOPFLASHDATA>(&m_fpStopFlashData, "StopFlashData");
	loadDLLfunct<PFLASHERASEMEMORY>(&m_fpFlashEraseMemory, "FlashEraseMemory");
	loadDLLfunct<PSETFLASHDATA>(&m_fpSetFlashData, "SetFlashData");
*/
}

CTMSiAccess::~CTMSiAccess()
{
	// Close the intefrace if it is still open, should prevent device locking in some cases
	if (m_bOpened)
	{
		closeFrontEnd();
	}

	if (m_hLibraryHandle != NULL)
	{
		m_fpLibraryExit(m_hLibraryHandle);
	}

	FreeLibrary(m_oLibTMSi);
}

boolean CTMSiAccess::initializeTMSiLibrary(const char* sConnectionProtocol)
{
	if (!m_bValid)
	{
		return false;
	}

	// TODO_JL block this function if the driver is in use

	m_rDriverContext.getLogManager() << LogLevel_Info << "(TMSi) " << "Initializing TMSi library on [" << sConnectionProtocol << "] protocol " << "\n";
	_TMSiConnectionEnum l_eConnectionProtocol = TMSiConnectionUndefined;

	l_eConnectionProtocol = m_mConnectionProtocols[sConnectionProtocol].first;

	int l_iLibraryError;

	if (m_hLibraryHandle != NULL)
	{
		// if the library is already in use then exit it first
		m_fpLibraryExit(m_hLibraryHandle);
		m_hLibraryHandle = NULL;
	}

	m_hLibraryHandle = m_fpLibraryInit(l_eConnectionProtocol, &l_iLibraryError);

	if (l_iLibraryError != 0)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "(TMSi) " << "Can not initialize TMSi library, errorcode = " << l_iLibraryError << "\n";
		m_hLibraryHandle = NULL;
		return false;
	}

	if (m_hLibraryHandle == INVALID_HANDLE_VALUE)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "(TMSi) " << "Can not initialize TMSi library, INVALID_ERROR_HANDLE" << "\n";
		m_hLibraryHandle = NULL;
		return false;
	}

	int l_iNumberOfDevices;
	// Once the library is initialized, get the list of devices on the frontend
	char** l_pDeviceList = m_fpGetDeviceList(m_hLibraryHandle, &l_iNumberOfDevices);
	m_vDeviceList.clear();

	if (l_iNumberOfDevices == 0)
	{
		l_iLibraryError = m_fpGetErrorCode(m_hLibraryHandle);
		if (l_iLibraryError == 0)
		{
			m_rDriverContext.getLogManager() << LogLevel_Info << "(TMSi) " << "No TMSi devices connected" << "\n";
		}
		else
		{
			m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Could not list TMSi devices, errorcode = " << l_iLibraryError << " message : \"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iLibraryError) << "\"" << "\n";
		}

		return false;
	}
	else
	{
		m_rDriverContext.getLogManager() << LogLevel_Info << "(TMSi) " << "Found [" << l_iNumberOfDevices << "] TMSi devices" << "\n";

		for (int l_iDeviceIndex = 0; l_iDeviceIndex < l_iNumberOfDevices; l_iDeviceIndex++)
		{
			m_vDeviceList.push_back(CString(l_pDeviceList[l_iDeviceIndex]));
		}
	}

	// the device list allocated in the library has to be freed
	m_fpFreeDeviceList(m_hLibraryHandle, l_iNumberOfDevices, l_pDeviceList);

	m_bInitialized = true;
	return true;
}

boolean CTMSiAccess::openFrontEnd(const char* sDeviceIdentifier)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Opening FrontEnd [" << sDeviceIdentifier << "]" << "\n";
	if (m_hLibraryHandle == NULL)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "TMSi library not accessible" << "\n";
		return false;
	}

	if(m_fpOpen(m_hLibraryHandle, sDeviceIdentifier))
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Frontend opened\n";
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Frontend NOT available, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";

		// If we receive "Access Refused (10061)" error we show a dialog prompting the user to restart his device
		GtkWidget* l_pGtkMessageDialog = NULL;
		if (l_iErrorCode == 10061)
		{
			l_pGtkMessageDialog = gtk_message_dialog_new_with_markup(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "The device <b>%s</b> is present but refuses access. It might be connected to another instance of OpenViBE. Restarting the device might help", sDeviceIdentifier);
		}
		else
		{
			l_pGtkMessageDialog = gtk_message_dialog_new_with_markup(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "The device <b>%s</b> is present but can not be opened. Restarting the device might help. Error reported by driver : %s", sDeviceIdentifier, m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode));
		}

		gtk_dialog_run(GTK_DIALOG(l_pGtkMessageDialog));
		gtk_widget_destroy(l_pGtkMessageDialog);

		// if we fail to open the frontend we close the library immediately
		//m_fpLibraryExit(m_hLibraryHandle);
		//m_hLibraryHandle = NULL;
		return false;
	}

	m_bOpened = true;
	return true;
}

boolean CTMSiAccess::closeFrontEnd()
{
	if (m_hLibraryHandle == NULL)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "TMSi library not accessible" << "\n";
		return false;
	}

	if (!m_bOpened)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Can not close unopened FrontEnd\n";
		return false;
	}

	if(m_fpClose(m_hLibraryHandle))
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Frontend closed\n";
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Can not close frontend, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		return false;
	}

	m_bOpened = false;
	m_bHasChannelStructure = false;
	m_bHasBufferSet = false;
	return true;
}

std::vector<unsigned long> CTMSiAccess::discoverDeviceSamplingFrequencies()
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " <<  "Checking for sampling frequencies" << "\n";

	std::vector<unsigned long> l_vSamplingFrequencies;

	m_ulMaxBufferSize = MAX_BUFFER_SIZE;
	l_vSamplingFrequencies.push_back(128 * 1000);
	l_vSamplingFrequencies.push_back(256 * 1000);
	l_vSamplingFrequencies.push_back(512 * 1000);
	l_vSamplingFrequencies.push_back(1024 * 1000);
	l_vSamplingFrequencies.push_back(2048 * 1000);

	if (!m_bOpened)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " <<  "No FrontEnd opened" << "\n";
		l_vSamplingFrequencies.clear();
		return l_vSamplingFrequencies;
	}

	for (size_t l_uiSamplingFrequencyIndex = 0; l_uiSamplingFrequencyIndex < l_vSamplingFrequencies.size(); l_uiSamplingFrequencyIndex++)
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " <<  "Trying frequency [" << static_cast<uint64>(l_vSamplingFrequencies[l_uiSamplingFrequencyIndex]) << "]" << "\n";

		unsigned long l_uiSamplingFrequency = l_vSamplingFrequencies[l_uiSamplingFrequencyIndex];

		// Max bufer size will be set to the right value in this step
		if (m_fpSetSignalBuffer(m_hLibraryHandle, &l_uiSamplingFrequency, &m_ulMaxBufferSize))
		{
			l_vSamplingFrequencies[l_uiSamplingFrequencyIndex] = l_uiSamplingFrequency;
			m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Sample rate: " << static_cast<uint64>(l_uiSamplingFrequency) << ", Buffer Size: " << static_cast<uint64>(m_ulMaxBufferSize) << "\n";
		}
		else
		{
			int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
			m_rDriverContext.getLogManager() << LogLevel_Error << "(TMSi) " << "Failed to set sampling frequency, errorcode = " << l_iErrorCode << ", message = \"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
			l_vSamplingFrequencies.clear();
			return l_vSamplingFrequencies;
		}
	}
	m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " <<  "Sampling frequencies OK" << "\n";
	return l_vSamplingFrequencies;
}

boolean CTMSiAccess::runDiagnostics()
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " <<  "Running diagnostics" << "\n";

	if (!m_bOpened)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " <<  "No FrontEnd opened" << "\n";
		return false;
	}

	FRONTENDINFO l_sFrontEndInfo;

	if(m_fpGetFrontEndInfo(m_hLibraryHandle, &l_sFrontEndInfo))
	{
		char l_sLogString[1024];
		sprintf(l_sLogString, "Frontend has Serial %lu", l_sFrontEndInfo.Serial );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "Frontend has HwVersion 0x%x", l_sFrontEndInfo.HwVersion );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "Frontend has SwVersion 0x%x", l_sFrontEndInfo.SwVersion );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "Frontend has BaseSf %d", l_sFrontEndInfo.BaseSf );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "Frontend has maxRS232 %d", l_sFrontEndInfo.maxRS232 );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "Frontend has %d channels", l_sFrontEndInfo.NrOfChannels );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";


	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "FrontendInfo NOT available, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		// return false;
	}

	TMSiBatReportType l_sTMSiBatReport;
	TMSiStorageReportType l_sTMSiStorageReport;
	TMSiDeviceReportType l_sTMSiDeviceReport;
	TMSiExtFrontendInfoType l_sTMSiExtFrontEndInfo ;

	if(m_fpGetExtFrontEndInfo(m_hLibraryHandle, &l_sTMSiExtFrontEndInfo, &l_sTMSiBatReport, &l_sTMSiStorageReport, &l_sTMSiDeviceReport))
	{
		char l_sLogString[1024];
		sprintf(l_sLogString, "CurrentSamplerate %d Hz", l_sTMSiExtFrontEndInfo.CurrentSamplerate );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "CurrentBlockType %d", l_sTMSiExtFrontEndInfo.CurrentBlockType );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "CurrentInterface %d", l_sTMSiExtFrontEndInfo.CurrentInterface );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "MemoryStatus.TotalSize %d MByte",  l_sTMSiStorageReport.TotalSize );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "MemoryStatus.UsedSpace %d MByte",  l_sTMSiStorageReport.UsedSpace );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "BatteryStatus.AccumCurrent %d mA", l_sTMSiBatReport.AccumCurrent );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "BatteryStatus.Current %d mA", l_sTMSiBatReport.Current );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "BatteryStatus.Temp %d C",  l_sTMSiBatReport.Temp );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "BatteryStatus.Voltage %d mV",  l_sTMSiBatReport.Voltage );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "TMSiDeviceReport.AdapterSerial %d",  l_sTMSiDeviceReport.AdapterSN );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "TMSiDeviceReport.AdapterCycles %d",  l_sTMSiDeviceReport.AdapterCycles );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "TMSiDeviceReport.AdapterStatus %d",  l_sTMSiDeviceReport.AdapterStatus );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "TMSiDeviceReport.MobitaCycles %d",  l_sTMSiDeviceReport.MobitaCycles);
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

		sprintf(l_sLogString, "TMSiDeviceReport.MobitaStatus %d",  l_sTMSiDeviceReport.MobitaStatus);
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";

	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		// Most of the devices do not have the extended frontend info so this message will popup quite often
		m_rDriverContext.getLogManager() << LogLevel_Debug << "(TMSi) " << "Extended FrontendInfo NOT available, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		// return false;
	}

	// only fails if the device is not opened, as not all devices support the getFrontEndInfo and getExtFrontEndInfo functions
	return true;
	m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " <<  "Diagnostics OK" << "\n";
}

boolean CTMSiAccess::getImpedanceTestingCapability(bool* pHasImpedanceTestingAbility)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " <<  "Getting impedance capability" << "\n";

	if (!m_bOpened)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " <<  "No FrontEnd opened" << "\n";
		return false;
	}

	FRONTENDINFO l_sFrontEndInfo;

	if(m_fpGetFrontEndInfo(m_hLibraryHandle, &l_sFrontEndInfo))
	{
		// get the first three numbers from the serial number
		// TMSi devices that can measure impedance have their serial numbers starting by a number between 107 and 128
		char l_sSerialNumber[1024];
		sprintf(l_sSerialNumber, "%lu", l_sFrontEndInfo.Serial );

		char l_sDeviceIdentifier[4];
		strncpy(l_sDeviceIdentifier, l_sSerialNumber, 3);
		l_sDeviceIdentifier[3] = 0;

		int l_iDeviceIdentifier = atoi(l_sDeviceIdentifier);

		*pHasImpedanceTestingAbility = (l_iDeviceIdentifier >= 107 && l_iDeviceIdentifier <= 128);
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "FrontendInfo NOT available, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		// return false;
	}
	return true;
}

boolean CTMSiAccess::calculateSignalFormat(const char *sDeviceIdentifier)
{
	if (!m_bOpened)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " <<  "No FrontEnd opened" << "\n";
		return false;
	}

	m_bHasChannelStructure = false;
	m_ulActualChannelCount = 0;
	m_ulMaxEEGChannelCount = 0;

	char l_sDeviceIdentifierString[1024];
	strcpy(l_sDeviceIdentifierString, sDeviceIdentifier);
	m_pSignalFormat = m_fpGetSignalFormat(m_hLibraryHandle, l_sDeviceIdentifierString);

	if(m_pSignalFormat == NULL)
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Error getting channel format, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		return false;
	}

	// Count the EEG channels
	m_ulMaxEEGChannelCount = 0;

	m_ulActualChannelCount = m_pSignalFormat->Elements;
	// go through the channels and find the number of EEG channels and additional channels
	// NOTE that this ONLY work if all EEG channels are listed first
	for( size_t l_uiChannelIndex = 0 ; l_uiChannelIndex < static_cast<int>(m_pSignalFormat->Elements); l_uiChannelIndex++ )
	{
		if (m_pSignalFormat[l_uiChannelIndex].Type == CHANNELTYPE_EXG)
		{
			m_ulMaxEEGChannelCount++;
		}
		else if (m_pSignalFormat[l_uiChannelIndex].Type == CHANNELTYPE_BIP)
		{
			m_ulMaxEEGChannelCount++;
		}
		else if (m_pSignalFormat[l_uiChannelIndex].Type == CHANNELTYPE_AUX)
		{
			m_ulMaxEEGChannelCount++;
		}
	}

	m_bHasChannelStructure = true;

	// the pointer is initialized and held by the library so we can store it
	return true;
}

boolean CTMSiAccess::printSignalFormat()
{
	if(m_pSignalFormat != NULL)
	{
		for( int l_iChannelIndex = 0 ; l_iChannelIndex < static_cast<int>(m_pSignalFormat->Elements); l_iChannelIndex++ )
		{
			char l_sChannelInfoString[1024];
			CString l_sChannelNameString = getChannelName(l_iChannelIndex);

			sprintf( l_sChannelInfoString, "%3d: %s Format %d Type %d Bytes %d Subtype %d UnitId %d UnitExponent %d",
				l_iChannelIndex,
			         l_sChannelNameString.toASCIIString(),
			         m_pSignalFormat[l_iChannelIndex].Format,
			         m_pSignalFormat[l_iChannelIndex].Type,
			         m_pSignalFormat[l_iChannelIndex].Bytes,
			         m_pSignalFormat[l_iChannelIndex].SubType,
			         m_pSignalFormat[l_iChannelIndex].UnitId,
			         m_pSignalFormat[l_iChannelIndex].UnitExponent );

			m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sChannelInfoString << "\n";
		}
	}
	else
	{
		return false;
	}

	return true;
}

CString CTMSiAccess::getChannelName(size_t stChannelIndex)
{
	if (m_pSignalFormat == NULL || stChannelIndex >= m_pSignalFormat->Elements)
	{
		return "";
	}

	char l_sChannelNameString[SIGNAL_NAME + 1];
	for( int l_iChannelNameIndex = 0 ; l_iChannelNameIndex < SIGNAL_NAME; l_iChannelNameIndex++ )
	{
		l_sChannelNameString[l_iChannelNameIndex] = static_cast<char>(m_pSignalFormat[stChannelIndex].Name[l_iChannelNameIndex]);
	}
	l_sChannelNameString[SIGNAL_NAME] = '\0';

	return CString(l_sChannelNameString);
}

CString CTMSiAccess::getChannelType(size_t uiChannelIndex)
{
	if (m_pSignalFormat == NULL || uiChannelIndex >= m_pSignalFormat->Elements)
	{
		return "";
	}

	switch (m_pSignalFormat[uiChannelIndex].Type)
	{
		case CHANNELTYPE_UNKNOWN:
			return CString("Unknown");
		case CHANNELTYPE_EXG:
			return CString("EXG");
		case CHANNELTYPE_BIP:
			return CString("BIP");
		case CHANNELTYPE_AUX:
			return CString("AUX");
		case CHANNELTYPE_DIG:
			return CString("DIG");
		case CHANNELTYPE_TIME:
			return CString("TIME");
		case CHANNELTYPE_LEAK:
			return CString("LEAK");
		case CHANNELTYPE_PRESSURE:
			return CString("PRESSURE");
		case CHANNELTYPE_ENVELOPE:
			return CString("ENVELOPE");
		case CHANNELTYPE_MARKER:
			return CString("MARKER");
		case CHANNELTYPE_SAW:
			return CString("RAMP");
		case CHANNELTYPE_SAO2:
			return CString("SAO2");
		default:
			return CString("Unknown");
	}

	return CString("Unknown");
}

void CTMSiAccess::freeSignalFormat()
{
	if (m_pSignalFormat != NULL)
	{
		m_fpFree(m_pSignalFormat);
		m_pSignalFormat = NULL;
	}
}

boolean CTMSiAccess::setCommonModeRejection(boolean bIsCommonModeRejectionEnabled)
{
	int l_iCARStatus = bIsCommonModeRejectionEnabled ? 1 : 0;

	if (m_fpSetRefCalculation(m_hLibraryHandle, l_iCARStatus))
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Common mode rejection set to " << l_iCARStatus << "\n";
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Can not set common mode rejection, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		return false;
	}

	return true;
}

boolean CTMSiAccess::setActiveChannels(CHeader* pHeader, CString sAdditionalChannels)
{
	uint32 l_ui32ActiveChannels = pHeader->getChannelCount();
	boolean l_bHasRenamedAChannel = false;

	m_vIsChannelActivated.clear();
	m_vIsChannelActivated.resize(m_ulActualChannelCount, false);

	size_t l_uiActiveAdditionalChannelCount = 0;

	for (size_t l_uiChannelIndex = m_ulMaxEEGChannelCount; l_uiChannelIndex < m_ulActualChannelCount; l_uiChannelIndex++)
	{
		if (std::string(sAdditionalChannels.toASCIIString()).find(std::string(";") + std::string(getChannelName(l_uiChannelIndex).toASCIIString()) + std::string(";")) != std::string::npos)
		{
			m_vIsChannelActivated[l_uiChannelIndex] = true;
			l_uiActiveAdditionalChannelCount++;
		}
		else
		{
			m_vIsChannelActivated[l_uiChannelIndex] = false;
		}
	}

	size_t l_uiLastActiveEEGChannelIndex = l_ui32ActiveChannels - l_uiActiveAdditionalChannelCount;

	for (size_t l_uiChannelIndex = 0; l_uiChannelIndex < l_uiLastActiveEEGChannelIndex; l_uiChannelIndex++)
	{
		m_vIsChannelActivated[l_uiChannelIndex] = true;
		if (strcmp(pHeader->getChannelName(l_uiChannelIndex), "") != 0)
		{
			l_bHasRenamedAChannel = true;
		}
	}
	for (size_t l_uiChannelIndex = l_uiLastActiveEEGChannelIndex; l_uiChannelIndex < m_ulMaxEEGChannelCount; l_uiChannelIndex++)
	{
		m_vIsChannelActivated[l_uiChannelIndex] = false;
	}

	m_ui32ActiveChannelcount = l_ui32ActiveChannels;

	// set names of EEG channels if none were renamed
	if (!l_bHasRenamedAChannel)
	{
		for (size_t l_uiChannelIndex = 0; l_uiChannelIndex < l_uiLastActiveEEGChannelIndex; l_uiChannelIndex++)
		{
			pHeader->setChannelName(l_uiChannelIndex, getChannelName(l_uiChannelIndex));
		}
	}

	// set names of additional channels
	size_t l_uiCurrentChannel = l_uiLastActiveEEGChannelIndex;
	for (size_t l_uiChannelIndex = m_ulMaxEEGChannelCount; l_uiChannelIndex < m_ulActualChannelCount; l_uiChannelIndex++)
	{
		if (m_vIsChannelActivated[l_uiChannelIndex])
		{
			pHeader->setChannelName(l_uiCurrentChannel, getChannelName(l_uiChannelIndex));
			l_uiCurrentChannel++;
		}
	}

	return true;
}

boolean CTMSiAccess::setSignalBuffer(unsigned long ulSamplingFrequency, unsigned long ulBufferSizeInSamples)
{
	if (!m_bOpened)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " <<  "No FrontEnd opened" << "\n";
		return false;
	}

	if (!m_bHasChannelStructure)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " <<  "The SignalFormat structure was not initialized" << "\n";
		return false;
	}

	// if the buffer size in samples is too big, change it automatically to the MAX for the current device
	if (ulBufferSizeInSamples > m_ulMaxBufferSize)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "The desired buffer size is too large, setting to maximum for the device instead" << "\n";
		ulBufferSizeInSamples = m_ulMaxBufferSize;
	}

	ULONG l_ulSamplingFrequency = ulSamplingFrequency;
	ULONG l_ulBufferSizeInSamples = ulBufferSizeInSamples;

	if (m_fpSetSignalBuffer(m_hLibraryHandle, &l_ulSamplingFrequency, &l_ulBufferSizeInSamples))
	{
		if (l_ulSamplingFrequency == ulSamplingFrequency && l_ulBufferSizeInSamples == ulBufferSizeInSamples)
		{
			m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "SignalBuffer set to " << static_cast<uint64>(l_ulSamplingFrequency) << " " << static_cast<uint64>(l_ulBufferSizeInSamples) << "\n";
		}
		else
		{
			m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Frontend does not support this Sampling Rate/Buffer Size combination" << "\n";
			return false;
		}
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Can not setSignalBuffer, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		return false;
	}

	if (m_pSampleBuffer != NULL)
	{
		delete[] m_pSampleBuffer;
		m_pSampleBuffer = NULL;
	}


	m_ulSignalBufferSizeInBytes = l_ulBufferSizeInSamples * m_ulActualChannelCount * sizeof(m_pSampleBuffer[0]);
	m_pSampleBuffer = new unsigned long[m_ulSignalBufferSizeInBytes];
	m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Allocating sample buffer with " << static_cast<uint32>(m_ulSignalBufferSizeInBytes) << "\n";

	m_bHasBufferSet = true;

	return true;
}

boolean CTMSiAccess::setSignalMeasuringModeToCalibration()
{
	return setSignalMeasuringMode(MEASURE_MODE_CALIBRATION_EX);
}

boolean CTMSiAccess::setSignalMeasuringModeToImpedanceCheck(int iImpedanceLimit)
{
	return setSignalMeasuringMode(MEASURE_MODE_IMPEDANCE_EX, iImpedanceLimit);
}

boolean CTMSiAccess::setSignalMeasuringModeToNormal()
{
	return setSignalMeasuringMode(MEASURE_MODE_NORMAL);
}

boolean CTMSiAccess::startAcquisition()
{
	if (m_bStarted)
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Acquisition has already been started" << "\n";
		return true;
	}

	if (m_hLibraryHandle == NULL)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "TMSi library not accessible" << "\n";
		return false;
	}

	if (!m_bHasBufferSet)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Can not start acquisition without setting buffer first" << "\n";
		return false;
	}

	if(m_fpStart(m_hLibraryHandle))
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Starting Acquisition" << "\n";
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Can not start acqusition, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		return false;
	}

	m_ui32LastSampleIndexInBuffer=0;
	m_bStarted = true;

	return true;
}

boolean CTMSiAccess::stopAcquisition()
{
	if (m_hLibraryHandle == NULL)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "TMSi library not accessible" << "\n";
		return false;
	}

	if (!m_bStarted)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Acquisition has not been started yet" << "\n";
		return true;
	}

	if(m_fpStop(m_hLibraryHandle))
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Stopping Acquisition" << "\n";
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Can not stop acqusition, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		return false;
	}

	getConnectionProperties();

	m_ui32LastSampleIndexInBuffer=0;

	m_bStarted = false;

	return true;
}

int32 CTMSiAccess::getSamples(float32* pSamples, IDriverCallback* pDriverCallback, uint64 ui64SampleCountPerSentBlock)
{
	// since this function is called all the time, we do not do safety checks
	long l_lNumberOfBytesReceived = m_fpGetSamples(m_hLibraryHandle, m_pSampleBuffer, m_ulSignalBufferSizeInBytes);

	if (l_lNumberOfBytesReceived > 0)
	{
		uint32 l_ui32SamplesReceived = l_lNumberOfBytesReceived / m_ulActualChannelCount / sizeof(m_pSampleBuffer[0]);

		for (uint32 l_ui32SampleIndex = 0; l_ui32SampleIndex < l_ui32SamplesReceived; l_ui32SampleIndex++)
		{
			uint32 l_ui32VirtualChannel = 0;
			for (uint32 l_ui32ChannelIndex = 0; l_ui32ChannelIndex < m_ulActualChannelCount; l_ui32ChannelIndex++)
			{
				if (m_vIsChannelActivated[l_ui32ChannelIndex])
				{
					// pointer to the current sample inside the signal sample buffer
					float32* l_f32SampleValueInFloat = &pSamples[l_ui32VirtualChannel*ui64SampleCountPerSentBlock + m_ui32LastSampleIndexInBuffer];
					//						std::cout << l_ui32VirtualChannel*m_ui32ActiveChannelcount + m_ui32SampleIndex << std::endl;

					PSIGNAL_FORMAT l_pCurrentSampleSignalFormat = &m_pSignalFormat[l_ui32ChannelIndex];

					// Calculate the floating value from the received integer value
					// For overflow of a analog channel, set the value to zero
					if( m_pSampleBuffer[l_ui32ChannelIndex + l_ui32SampleIndex*m_ulActualChannelCount] == OVERFLOW_32BITS &&
					        (l_pCurrentSampleSignalFormat->Type == CHANNELTYPE_EXG ||
					         l_pCurrentSampleSignalFormat->Type == CHANNELTYPE_BIP ||
					         l_pCurrentSampleSignalFormat->Type == CHANNELTYPE_AUX ))
					{
						*l_f32SampleValueInFloat = 0 ; // Set it to a value you find a good sign of a overflow
					}
					else
					{
						switch( l_pCurrentSampleSignalFormat->Format )
						{
							case SF_UNSIGNED  : // unsigned integer
								*l_f32SampleValueInFloat = m_pSampleBuffer[l_ui32ChannelIndex + l_ui32SampleIndex*m_ulActualChannelCount] *  l_pCurrentSampleSignalFormat->UnitGain + l_pCurrentSampleSignalFormat->UnitOffSet ;
								break ;
							case SF_INTEGER: // signed integer
								*l_f32SampleValueInFloat = ((int)(m_pSampleBuffer[l_ui32ChannelIndex + l_ui32SampleIndex*m_ulActualChannelCount])) *  l_pCurrentSampleSignalFormat->UnitGain + l_pCurrentSampleSignalFormat->UnitOffSet ;
								break ;
							default :
								*l_f32SampleValueInFloat = 0 ; // For unknown types, set the value to zero
								break ;
						}
					}

					l_ui32VirtualChannel++;
				}
			}
			m_ui32LastSampleIndexInBuffer++;

			if (m_ui32LastSampleIndexInBuffer == ui64SampleCountPerSentBlock)
			{
				pDriverCallback->setSamples(pSamples);
				m_ui32LastSampleIndexInBuffer = 0;
				m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());
			}
		}
	}
	else if (l_lNumberOfBytesReceived == 0)
	{
		ULONG l_ulOverflow, l_ulPercentFull;
		int l_iStatus = m_fpGetBufferInfo( m_hLibraryHandle, &l_ulOverflow, &l_ulPercentFull );

		if( l_iStatus != 0 && l_ulOverflow > 0 && l_ulPercentFull > 0 )
		{
			char l_sLogString[1024];
			sprintf(l_sLogString, "Overflow %d PercentFull %d", l_ulOverflow, l_ulPercentFull );
			m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";
		}
	}
	else
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Can not read data, errorcode = " << l_lNumberOfBytesReceived << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_lNumberOfBytesReceived) << "\"" << "\n";

		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		if (l_iErrorCode != 0)
		{
			m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Additional error, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		}
		return -1;
	}

	return l_lNumberOfBytesReceived;
}

boolean CTMSiAccess::getImpedanceValues(std::vector<float64>* pvImpedanceValues)
{
	pvImpedanceValues->resize(m_ulActualChannelCount);

	// since this function is called all the time, we do not do safety checks
	long l_lNumberOfBytesReceived = m_fpGetSamples(m_hLibraryHandle, m_pSampleBuffer, m_ulSignalBufferSizeInBytes);

	if (l_lNumberOfBytesReceived > 0)
	{
		uint32 l_ui32SamplesReceived = l_lNumberOfBytesReceived / m_ulActualChannelCount / sizeof(m_pSampleBuffer[0]);

		// We only look at the first sample in the whole buffer, no need to look at all values
		for (uint32 l_ui32SampleIndex = 0; l_ui32SampleIndex < 1; l_ui32SampleIndex++)
		{
			uint32 l_ui32VirtualChannel = 0;
			for (uint32 l_ui32ChannelIndex = 0; l_ui32ChannelIndex < m_ulActualChannelCount; l_ui32ChannelIndex++)
			{
				if (m_vIsChannelActivated[l_ui32ChannelIndex])
				{
					// Impedance values are returned in MOhm it seems
					(*pvImpedanceValues)[l_ui32VirtualChannel] = m_pSampleBuffer[l_ui32ChannelIndex + l_ui32SampleIndex*m_ulActualChannelCount] * 1000.0;
					l_ui32VirtualChannel++;
				}
			}
		}
	}
	else if (l_lNumberOfBytesReceived == 0)
	{

	}
	else
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Can not read impedance data, errorcode = " << l_lNumberOfBytesReceived << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_lNumberOfBytesReceived) << "\"" << "\n";

		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		if (l_iErrorCode != 0)
		{
			m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Additional error, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		}
		return false;
	}

	return true;
}

boolean CTMSiAccess::getConnectionProperties()
{
	unsigned int l_iSignalStrength;
	unsigned int l_iNrOfCRCErrors;
	unsigned int l_iNrOfSampleBlocks;

	if( m_fpGetConnectionProperties( m_hLibraryHandle, &l_iSignalStrength, &l_iNrOfCRCErrors, &l_iNrOfSampleBlocks ) )
	{
		char l_sLogString[1024];
		sprintf(l_sLogString, "fpGetConnectionProperties SignalStrength %d NrOfCRCErrors %d NrOfSampleBlocks %d", l_iSignalStrength, l_iNrOfCRCErrors, l_iNrOfSampleBlocks);
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sLogString << "\n";
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Error getting channel format, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		return false;
	}

	return true;
}

// PRIVATE METHODS

boolean CTMSiAccess::setSignalMeasuringMode(ULONG ulMeasuringMode, int iValue)
{
	if (!m_bOpened)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " <<  "No FrontEnd opened" << "\n";
		return false;
	}

	if (!m_bStarted)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " <<  "This method must be called after the frontend is started" << "\n";
		return false;
	}

	int l_iModeValue = 0;
	CString l_sModeName("Normal");

	if (ulMeasuringMode == MEASURE_MODE_CALIBRATION_EX)
	{
		l_iModeValue = s_iCalibrationVoltage;
		l_sModeName = "Calibration";
	}
	else if (ulMeasuringMode == MEASURE_MODE_IMPEDANCE_EX)
	{
		l_iModeValue = iValue;
		l_sModeName = "Impedance";
	}

	if (m_fpSetMeasuringMode(m_hLibraryHandle, ulMeasuringMode, l_iModeValue))
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Device set to " << l_sModeName << " mode [" << l_iModeValue << "]" << "\n";
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Failed setting the device to " << l_sModeName << " mode, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		return false;
	}

	return true;
}

#endif
