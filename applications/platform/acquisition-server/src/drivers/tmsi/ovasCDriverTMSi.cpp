/*
 * ovasCDriverTMSi.cpp
 *
 * Copyright (c) 2014, Mensia Technologies SA. All rights reserved.
 * -- Rights transferred to Inria, contract signed 21.11.2014
 *
 */

#include "ovasCDriverTMSi.h"
#include "ovasCConfigurationTMSi.h"
#include "ovasCTMSiAccess.h"


#if defined TARGET_HAS_ThirdPartyTMSi

#include <toolkit/ovtk_all.h>
#include <openvibe/ovITimeArithmetics.h>

#include <iostream>
#include <cmath>
#include <cstring>
#include <system/ovCTime.h>
using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

// Public driver functions

CDriverTMSi::CDriverTMSi(IDriverContext& rDriverContext)
:IDriver(rDriverContext)
 ,m_oSettings("AcquisitionServer_Driver_TMSi", m_rDriverContext.getConfigurationManager())
 ,m_pCallback(NULL)
 ,m_ui32SampleCountPerSentBlock(0)
 ,m_pTMSiAccess(NULL)
 {
	// default parameters, will be overriden by saved settings
	m_oHeader.setSamplingFrequency(512);
	m_oHeader.setChannelCount(32);
	m_sConnectionProtocol = "USB";
	m_sDeviceIdentifier = "";
	m_bCommonAverageReference = true;
	m_ui64ActiveEEGChannels = 0;
	m_sActiveAdditionalChannels = ";";
	m_ui64ImpedanceLimit = 1;

	m_oSettings.add("Header", &m_oHeader);
	m_oSettings.add("ConnectionProtocol", &m_sConnectionProtocol);
	m_oSettings.add("DeviceIdentifier", &m_sDeviceIdentifier);
//	m_oSettings.add("CommonAverageReference", &m_bCommonAverageReference);
	m_oSettings.add("ActiveEEGChannels", &m_ui64ActiveEEGChannels);
	m_oSettings.add("ActiveAdditionalChannels", &m_sActiveAdditionalChannels);
	m_oSettings.add("ImpedanceLimit", &m_ui64ImpedanceLimit);
	m_oSettings.load();

	m_bValid = true;

	if (!m_bValid)
	{
		return;
	}

	m_pTMSiAccess = new CTMSiAccess(m_rDriverContext);

}

CDriverTMSi::~CDriverTMSi(void)
{
	this->uninitialize();
}

void CDriverTMSi::release(void)
{
	delete this;
}

const char* CDriverTMSi::getName(void)
{
	return "TMSi amplifiers";
}

boolean CDriverTMSi::initialize(
		const uint32 ui32SampleCountPerSentBlock,
		IDriverCallback& rCallback)
{
	if(!m_bValid) { return false; }
	if(m_rDriverContext.isConnected()) { return false; }

	// Show a window prompting the user to wait, this window is automatically closed when the Configuration object is destroyed
	CConfigurationTMSi l_oConfiguration(OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-TMSi.ui", this);
	l_oConfiguration.showWaitWindow();


	if (m_sDeviceIdentifier == CString(""))
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "No TMSi device selected" << "\n";
		return false;
	}
	if (m_oHeader.getSamplingFrequency() == 0)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "No Sampling Frequency selected" << "\n";
	}

	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;

	m_pTMSiAccess->initializeTMSiLibrary(m_sConnectionProtocol.toASCIIString());
	m_pTMSiAccess->openFrontEnd(m_sDeviceIdentifier.toASCIIString());


	// by default CAR (called Common Mode Rejection in TMSi documents) is enabled, one can disable it by token
	m_bCommonAverageReference = !(m_rDriverContext.getConfigurationManager().expandAsBoolean("${AcquisitionServer_TMSI_DisableCommonModeRejection}", false));

	if (!m_pTMSiAccess->setCommonModeRejection(m_bCommonAverageReference))
	{
		m_pTMSiAccess->closeFrontEnd();
		return false;
	}

	if (!m_pTMSiAccess->runDiagnostics())
	{
		m_pTMSiAccess->closeFrontEnd();
		return false;
	}
/*
	// Get Time
	SYSTEMTIME l_tTime;
	if(m_fpGetRtcTime( m_hLibraryHandle, &l_tTime ))
	{
		char l_sTimeString[255];
		sprintf(l_sTimeString, "Current Device Time Weekday %d on %d-%d-%d Time %d:%d:%d", l_tTime.wDayOfWeek, l_tTime.wDay, l_tTime.wMonth, l_tTime.wYear, l_tTime.wHour, l_tTime.wMinute, l_tTime.wSecond );
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << l_sTimeString << "\n";
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Error getting time from device, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		return false;
	}

	// Set Time
	GetLocalTime(&l_tTime);
	if(m_fpSetRtcTime(m_hLibraryHandle, &l_tTime ))
	{
		m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Time set" << "\n";
	}
	else
	{
		int l_iErrorCode = m_fpGetErrorCode(m_hLibraryHandle);
		m_rDriverContext.getLogManager() << LogLevel_Warning << "(TMSi) " << "Error setting time on device, errorcode = " << l_iErrorCode << ", message=\"" << m_fpGetErrorCodeMessage(m_hLibraryHandle, l_iErrorCode) << "\"" << "\n";
		return false;
	}
*/

	// Get Signal Format
	if(m_pTMSiAccess->calculateSignalFormat(m_sDeviceIdentifier))
	{
		m_pTMSiAccess->printSignalFormat();
	}
	else
	{
		m_pTMSiAccess->closeFrontEnd();
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << "Number of Channels on [" << m_sDeviceIdentifier << "]" << " = " << static_cast<uint64>(m_pTMSiAccess->getActualChannelCount()) << "\n";

	if (!m_pTMSiAccess->setSignalBuffer(m_oHeader.getSamplingFrequency() * 1000, m_oHeader.getSamplingFrequency() * 10))
	{
		m_pTMSiAccess->closeFrontEnd();
		return false;
	}

	m_pTMSiAccess->setActiveChannels(&m_oHeader, m_sActiveAdditionalChannels);

	m_pSample=new float32[m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock];
	if(!m_pSample)
	{
		delete [] m_pSample;
		m_pSample=NULL;
		return false;
	}

	if (!m_pTMSiAccess->getConnectionProperties())
	{
		m_pTMSiAccess->closeFrontEnd();
		return false;
	}

	if (m_rDriverContext.isImpedanceCheckRequested())
	{
		// Acquisition on the device must be started in order to set the acquiring mode to impedance
		if (!m_pTMSiAccess->startAcquisition())
		{
			return false;
		}
		if (!m_pTMSiAccess->setSignalMeasuringModeToImpedanceCheck(static_cast<int>(m_ui64ImpedanceLimit)))
		{
			m_pTMSiAccess->stopAcquisition();
			m_bIgnoreImpedanceCheck = true;
		}
		else
		{
			m_vImpedance.resize(m_oHeader.getChannelCount());
			m_bIgnoreImpedanceCheck = false;
		}
	}

	// @todo modify the API to provide this info (depends on the device)
	for(uint32 i=0;i<m_oHeader.getChannelCount();i++)
	{
		m_oHeader.setChannelUnits(i, OVTK_UNIT_Unspecified, OVTK_FACTOR_Base);
	}

	return true;
}

boolean CDriverTMSi::start(void)
{
	if(!m_bValid) { return false; }
	if(!m_rDriverContext.isConnected()){ return false;}
	if(m_rDriverContext.isStarted()){ return false;}

	if (m_rDriverContext.isImpedanceCheckRequested())
	{
		m_pTMSiAccess->setSignalMeasuringModeToNormal();
		m_pTMSiAccess->stopAcquisition();
	}
	// sometimes the driver might refuse to start, some sampling frequencies can be set but not used for example
	if (!m_pTMSiAccess->startAcquisition())
	{
		return false;
	}

	// TODO_JL Do impedance check when applicable

	m_ui32TotalSampleReceived=0;
	return true;
}

boolean CDriverTMSi::loop(void)
{
	if(!m_bValid) { return false; }
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted())
	{
		int32 l_i32BytesReceived =  m_pTMSiAccess->getSamples(m_pSample, m_pCallback, m_ui32SampleCountPerSentBlock);

		if (l_i32BytesReceived >= 0)
		{
			m_ui32TotalSampleReceived += l_i32BytesReceived;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (m_rDriverContext.isImpedanceCheckRequested() && !m_bIgnoreImpedanceCheck)
		{
			if (!m_pTMSiAccess->getImpedanceValues(&m_vImpedance))
			{
				return false;
			}

			for (size_t l_uiChannelIndex = 0; l_uiChannelIndex < m_oHeader.getChannelCount(); l_uiChannelIndex++)
			{
				m_rDriverContext.updateImpedance(l_uiChannelIndex, m_vImpedance[l_uiChannelIndex]);
			}
		}
	}

	return true;
}

boolean CDriverTMSi::stop(void)
{
	if(!m_rDriverContext.isConnected()){ return false;}
	if(!m_rDriverContext.isStarted()){ return false;}
	m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << ">Stop TMSI\n";

	m_pTMSiAccess->stopAcquisition();

	m_ui32TotalSampleReceived=0;
	return true;
}

boolean CDriverTMSi::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()){ return false;}
	if(m_rDriverContext.isStarted()){ return false;}
	m_rDriverContext.getLogManager() << LogLevel_Trace << "(TMSi) " << ">Uninit TMSI\n";

	m_pTMSiAccess->stopAcquisition();

	m_pTMSiAccess->freeSignalFormat();

	if (!m_pTMSiAccess->closeFrontEnd())
	{
		return false;
	}


	return true;
}

boolean CDriverTMSi::configure(void)
{
	CConfigurationTMSi l_oConfiguration(OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-TMSi.ui", this);

	bool result=l_oConfiguration.configure(m_oHeader);
	if(!result)
	{
		return false;
	}

	m_oSettings.save();

	return true;
}

boolean CDriverTMSi::isConfigurable(void)
{
	return true;
}

#endif // TARGET_OS_Windows
