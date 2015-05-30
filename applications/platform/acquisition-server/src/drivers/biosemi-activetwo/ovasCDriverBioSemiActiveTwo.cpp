/*
 * ovasCDriverBioSemiActiveTwo.cpp
 *
 * Copyright (c) 2012, Mensia Technologies SA. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */

#ifdef TARGET_HAS_ThirdPartyBioSemiAPI

#include "ovasCDriverBioSemiActiveTwo.h"
#include "ovasCConfigurationBioSemiActiveTwo.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#else
// linux, mac
#include <unistd.h>
#endif	// WIN32

#include <toolkit/ovtk_all.h>
#include <system/ovCTime.h>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
#define boolean OpenViBE::boolean

//___________________________________________________________________//
//                                                                   //

CDriverBioSemiActiveTwo::CDriverBioSemiActiveTwo(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_oSettings("AcquisitionServer_Driver_BioSemiActiveTwo" ,m_rDriverContext.getConfigurationManager())
	,m_bUseEXChannel(false)
	,m_pInformationWindow(NULL)
	,m_ui32LastCmsBackInRange(0)
	,m_ui32LastBatteryLow(0)
	,m_bCMSBackInRange(true)
	,m_bBatteryBackOk(true)
	,m_bAcquisitionStopped(false)
{
	m_oHeader.setSamplingFrequency(2048);

	// The amplifier can send up to 256+8 channels
	// as a request from BioSemi, we will make available the maximum channel count
	// User is able to select from 1 to MAX channels. If no data is present on the 
	// corresponding channels, zeros will be sent.
	// The number of channels present in the data flow will still be displayed in 
	// the driver configuration window. Previously selected value will be saved
	// with other settings.
	m_oHeader.setChannelCount(BIOSEMI_ACTIVETWO_MAXCHANNELCOUNT + BIOSEMI_ACTIVETWO_EXCHANNELCOUNT);
	m_bUseEXChannel = true;
	
	m_vTriggers.resize(16,false);

	m_ui32LastWarningTime = 0;
	m_ui32StartTime = 0;
	m_bCMCurrentlyInRange = true;
	m_bBatteryCurrentlyOk = true;

	m_oSettings.add("Header", &m_oHeader);
	m_oSettings.add("UseEXChannel", &m_bUseEXChannel);
	m_oSettings.load();
	m_oBridge.setUseEXChannels(m_bUseEXChannel);
}

void CDriverBioSemiActiveTwo::release(void)
{
	delete this;
}

const char* CDriverBioSemiActiveTwo::getName(void)
{
	return "BioSemi Active Two (MkI and MkII)";
}


//___________________________________________________________________//
//                                                                   //

gint information_window_callback(void* pInformationWindow)
{
	SInformationWindow* l_pInformationWindow = reinterpret_cast<SInformationWindow*>(pInformationWindow);
	boost::mutex::scoped_lock l_lock(reinterpret_cast<SInformationWindow*>(pInformationWindow)->m_oMutex);
	// If nothing changed, directly return from the callback
	if(!l_pInformationWindow->m_bIsChanged)
	{
		return TRUE;
	}
	if(l_pInformationWindow->m_bIsAcquisitionEnded)
	{
		// If the acquisition is ended, delete the window
		gtk_widget_destroy(GTK_WIDGET(gtk_builder_get_object(l_pInformationWindow->m_pBuilderConfigureInterface, "device-information")));
		g_object_unref(l_pInformationWindow->m_pBuilderConfigureInterface);
		// The loop should now be stopped
		return FALSE;
	}
	if(l_pInformationWindow->m_bIsBatteryLow)
	{
		gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(l_pInformationWindow->m_pBuilderConfigureInterface, "label-battery-level")), "- <i>Device battery is low !</i> -");
		gtk_image_set_from_stock(GTK_IMAGE(gtk_builder_get_object(l_pInformationWindow->m_pBuilderConfigureInterface, "image-battery-level")), GTK_STOCK_NO, GTK_ICON_SIZE_BUTTON);
	}
	else
	{
		gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(l_pInformationWindow->m_pBuilderConfigureInterface, "label-battery-level")), "- <i> Device battery is ok </i> -");
		gtk_image_set_from_stock(GTK_IMAGE(gtk_builder_get_object(l_pInformationWindow->m_pBuilderConfigureInterface, "image-battery-level")), GTK_STOCK_YES, GTK_ICON_SIZE_BUTTON);
	}

	if(l_pInformationWindow->m_bIsCMSInRange)
	{
		gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(l_pInformationWindow->m_pBuilderConfigureInterface, "label-CMS-status")), "- <i>CMS/DRL is in range </i> -");
		gtk_image_set_from_stock(GTK_IMAGE(gtk_builder_get_object(l_pInformationWindow->m_pBuilderConfigureInterface, "image-CMS-status")), GTK_STOCK_YES, GTK_ICON_SIZE_BUTTON);
	}
	else
	{
		gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(l_pInformationWindow->m_pBuilderConfigureInterface, "label-CMS-status")), "- <i>CMS/DRL is not in range </i> -");
		gtk_image_set_from_stock(GTK_IMAGE(gtk_builder_get_object(l_pInformationWindow->m_pBuilderConfigureInterface, "image-CMS-status")), GTK_STOCK_NO, GTK_ICON_SIZE_BUTTON);
	}
	gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(l_pInformationWindow->m_pBuilderConfigureInterface, "label-error-message")), (l_pInformationWindow->m_sErrorMessage).c_str());

	l_pInformationWindow->m_bIsChanged = false;
	return TRUE;
}

boolean CDriverBioSemiActiveTwo::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	m_bAcquisitionStopped = false;

	if(m_rDriverContext.isConnected()) { return false; }
	
	m_pCallback=&rCallback;

	if(!m_oBridge.open())
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "Could not open the device.\n";
		return false;
	}

	if(!m_oBridge.start())
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "Could not start the device.\n";
		return false;
	}

	// wait to be sure we get the first packet from which we deduce the actual channel count and other initial configuration.
	System::Time::sleep(500);

	int32 l_i32ByteRead = m_oBridge.read();
	if(l_i32ByteRead < 0)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "An error occured while reading first data packet from device !\n";
		m_oBridge.close();
		return false;
	}
	
	if(!m_oBridge.discard()) // we discard the samples.
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "An error occured while dropping unused samples at initialization time.\n";
		m_oBridge.close();
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Trace << "Bridge initialized with: [SF:"<<m_oBridge.getSamplingFrequency()
		<< "] [CH:"<<m_oBridge.getChannelCount()
		<< "] [MKII:"<<m_oBridge.isDeviceMarkII()
		<< "] [CMInRange:"<<m_oBridge.isCMSInRange()
		<< "] [LowBat:"<<m_oBridge.isBatteryLow()<<"]\n";

	if(m_oBridge.isBatteryLow())
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "Device battery is low !\n";
	}

	// the sample buffer is resized to get samples from ALL channels even if the user selected
	// less channels. We will adjust the content when calling setSamples(...)
	// in case the user required more channels than the number available in the stream, we will
	// add 0-padding.
	uint32 l_ui32ChannelCountInStream = m_oBridge.getChannelCount();
	uint32 l_ui32ChannelCountRequested = m_oHeader.getChannelCount();
	m_vSample.clear();
	m_vSample.resize(l_ui32ChannelCountRequested, 0);
	if(l_ui32ChannelCountRequested > l_ui32ChannelCountInStream)
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "The required channel count cannot be reached in current device configuration (data stream contains "<<m_oBridge.getChannelCount()<<" channels). Please check the device speed mode and setup capabilities. Channels with no data will be filled with zeros.\n";
	}
	
	m_ui64SampleCount = 0;
	m_rDriverContext.getLogManager() << LogLevel_Trace << "Driver initialized...\n";

	//Check speed mode: speed modes 1, 2 and 3 should not be used for acquisition
	if(m_oBridge.getSpeedMode() < 4)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Speed modes 1 to 3 are designed to realize daisy chained montages, they should not be used for acquisition.";
		return false;
	}

	// Rename EX channels after settings were saved 
	if(m_oBridge.isUseEXChannels())
	{
		int j=1;
		for (uint32 i = m_oHeader.getChannelCount() - BIOSEMI_ACTIVETWO_EXCHANNELCOUNT; i < m_oHeader.getChannelCount(); i++)
		{
			char l_sEXName[5];
			sprintf(l_sEXName, "EX %i", j) ;
			m_oHeader.setChannelName(i, l_sEXName);
			m_oHeader.setChannelUnits(i, OVTK_UNIT_Volts, OVTK_FACTOR_Micro);
			m_rDriverContext.getLogManager() << LogLevel_Trace << "Channel name: " << m_oHeader.getChannelName(i) << "\n";
			j++;
		}
	}

	// Initialize information window
	gdk_threads_enter();
	m_pInformationWindow = new SInformationWindow();
	m_pInformationWindow->m_pBuilderConfigureInterface=gtk_builder_new();
	if(!gtk_builder_add_from_file(m_pInformationWindow->m_pBuilderConfigureInterface, OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-BioSemi-ActiveTwo.ui", NULL))
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "File not found: "<<  OpenViBE::Directories::getDataDir() << "/applications/acquisition-server/interface-BioSemi-ActiveTwo.ui\n";
	}
	gtk_label_set_markup(GTK_LABEL(gtk_builder_get_object(m_pInformationWindow->m_pBuilderConfigureInterface, "label-device-type")), (m_oBridge.isDeviceMarkII() ? "- <i>ActiveTwo Mark II</i> -" : "- <i>ActiveTwo Mark I</i> -"));
	m_pInformationWindow->m_bIsCMSInRange = m_oBridge.isCMSInRange();
	m_pInformationWindow->m_bIsBatteryLow = m_oBridge.isBatteryLow();
	m_pInformationWindow->m_bIsChanged = true;
	gtk_widget_show_all(GTK_WIDGET(gtk_builder_get_object(m_pInformationWindow->m_pBuilderConfigureInterface, "device-information")));

	//Launch idle loop: update the information window in a separate glib thread
	g_idle_add(information_window_callback, m_pInformationWindow);
	gdk_threads_leave();
	return true;
}

boolean CDriverBioSemiActiveTwo::start(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }
	
	m_ui32StartTime = System::Time::getTime();
	m_ui32LastWarningTime = 0;

	m_rDriverContext.getLogManager() << LogLevel_Trace << "Acquisition started...\n";
	return true;
}

boolean CDriverBioSemiActiveTwo::loop(void)
{
	if(m_bAcquisitionStopped || !m_rDriverContext.isConnected())
	{
		return true; 
	}

	if(m_rDriverContext.isStarted())
	{
		uint32 l_ui32ChannelCountInStream = m_oBridge.getChannelCount();
		uint32 l_ui32ChannelCountRequested = m_oHeader.getChannelCount();
		
		//uint32 l_ui32Max = (l_ui32ChannelCountRequested > l_ui32ChannelCountInStream) ? l_ui32ChannelCountRequested : l_ui32ChannelCountInStream;
		uint32 l_ui32Max = l_ui32ChannelCountRequested;
		int32 l_i32ByteRead = m_oBridge.read();
		if(l_i32ByteRead > 0)
		{
			for(uint32 i = 0; i<m_oBridge.getAvailableSampleCount(); i++)
			{
				// we consume one sample per channel, values are given in uV
				if(!m_oBridge.consumeOneSamplePerChannel(&m_vSample[0], l_ui32Max))
				{
					boost::mutex::scoped_lock l_lock(m_pInformationWindow->m_oMutex);
					m_rDriverContext.getLogManager() << LogLevel_Error << "Something bad happened while consuming samples from device.\n";
					m_pInformationWindow->m_bIsChanged = true;
					m_pInformationWindow->m_sErrorMessage = "<span color=\"darkred\">Something bad happened while consuming samples from device.</span>\n";
					if(m_oBridge.getLastError() == Mensia::BioSemiError_SyncLost)
					{
						m_pInformationWindow->m_sErrorMessage += "\t <span color=\"darkred\"> Synchronization lost during acquisition. Fiber optic may be loose or damaged.</span>";
						m_rDriverContext.getLogManager() << "\t >Synchronization lost during acquisition. Fiber optic may be loose or damaged.\n";
					}
					if(m_oBridge.getLastError() == Mensia::BioSemiError_BufferOverflow)
					{	
						m_pInformationWindow->m_sErrorMessage += "\t <span color=\"darkred\"> Buffer overflow. Please check that you have enough CPU and memory available to run the acquisition server at full speed before retrying.</span>";
						m_rDriverContext.getLogManager() << "\t > Buffer overflow. Please check that you have enough CPU and memory available to run the acquisition server at full speed before retrying.\n";
					}
					m_bAcquisitionStopped = true;
					return true;
				}
				
				// this call uses the header's channel count, so it will naturally take the first samples (not necessarily all channels).
				m_pCallback->setSamples(&m_vSample[0],1);

				// triggers:
				// we simply send OVTK_StimulationId_Label_X where X is the trigger index between 1 and 16
				// We don't handle rising and falling edges.
				for(uint32 i=0; i<m_vTriggers.size(); i++)
				{
					if(m_oBridge.getTrigger(i) != m_vTriggers[i])
					{
						m_vTriggers[i] = m_oBridge.getTrigger(i);
						uint64 l_ui64Date = (1LL<<32) / m_oBridge.getSamplingFrequency(); // date is relative to the buffer start. I only have one sample in the buffer so it's fairly simple
						m_oStimulationSet.appendStimulation(OVTK_StimulationId_Label(i+1), l_ui64Date, 0);
						m_rDriverContext.getLogManager() << LogLevel_Trace << "Trigger "<<i+1<<"/16 has switched to " << m_vTriggers[i]<<"\n";
					}
				}

				// "CMS/DRL in range" warning, once every 2 seconds max
				/*
				From: Coen (BioSemi):

				The current flow via the DRL is constantly monitored by the safety circuitry inside the AD-box. The current flow is limited to 50 uA (IEC 601 safety limit).
				If the current runs into the limit, the CMS/DRL circuit cannot keep the Common Mode value within its normal working range, and the blue LED turns off.
		
				The safety circuitry reacts on this error situation by shutting the power supply to ALL active electrodes off. Consequently, no meaningful signals can be measured so long as the blue LED is off. 
				The circuit operation described above implies that any electrode can be the cause of a CM out of range error. 
				Examples of errors are broken wires, bad connector contacts (pollution of connector with gel), defect IC inside the electrode, 
				bare electrode wire contacting the subject (damaged cable isolation) etc.. For example, if one of the active electrode wires is broken,
				the electrode input circuit is not anymore biased correctly, and the input resistance may fall below its specified value of 10^12 Ohm.
				The resultant extra input current is detected by the CMS/DRL circuit, and the blue LED goes off. 
				Save operation of the system is ensured because the power supply to the active electrodes is only restored if ALL electrodes connected to the subject work correctly.
				In other words, both cap en EX electrodes can in principle cause CM out of range errors. 
				*/

				boolean l_bWarningDisplayed = false;
				
				if (!m_oBridge.isCMSInRange())
				{
					// we print a warning message once every 2secs maximum
					if(System::Time::getTime() > m_ui32LastWarningTime + 2000)
					{
						l_bWarningDisplayed = true;
						m_rDriverContext.getLogManager() << LogLevel_Warning << "("<<((System::Time::getTime() - m_ui32StartTime)/1000)<<"') CMS/DRL is not in range. For safety purpose, any active electrode connected has been shut down and signals should not be used.\n";
						m_rDriverContext.getLogManager() << "\t  The corresponding sample range has been tagged with OVTK_StimulationId_SegmentStart and OVTK_StimulationId_SegmentStop.\n";
						m_rDriverContext.getLogManager() << "\t  Possible causes include broken wires, damaged cable isolation, bad connector contacts, defect IC inside the electrode.\n";
						{
							boost::mutex::scoped_lock l_lock(m_pInformationWindow->m_oMutex);
							m_pInformationWindow->m_bIsCMSInRange = false;
							m_pInformationWindow->m_bIsChanged = true;
						}
					}

					// the possibly incorrect sample range is tagged using OVTK_StimulationId_SegmentStart and OVTK_StimulationId_SegmentStop
					// CM is now not in range
					if(m_bCMCurrentlyInRange)
					{					
						uint64 l_ui64Date = (1LL<<32) / m_oBridge.getSamplingFrequency(); 
						m_oStimulationSet.appendStimulation(OVTK_StimulationId_SegmentStart, l_ui64Date, 0);
					}
					m_bCMCurrentlyInRange = false;
					m_bCMSBackInRange = false;
				}
				else
				{
					// CMS/DRL is now back in range
					if(!m_bCMCurrentlyInRange)
					{
						m_ui32LastCmsBackInRange = System::Time::getTime();
						m_rDriverContext.getLogManager() << LogLevel_Trace << "isCMSInRange \n";
						uint64 l_ui64Date = (1LL<<32) / m_oBridge.getSamplingFrequency(); 
						m_oStimulationSet.appendStimulation(OVTK_StimulationId_SegmentStop, l_ui64Date, 0);
						m_bCMCurrentlyInRange = true;
					}
					// CMS is considered "back in range" if it stayed in range for more than 500ms 
					// -> avoids changing the gtk markup too often (which would not be readable for the user nor efficient)
					else if(!m_bCMSBackInRange && (System::Time::getTime() > m_ui32LastCmsBackInRange + 100))
					{
						m_bCMSBackInRange = true;
						m_rDriverContext.getLogManager() << LogLevel_Info << "Back in range \n";
						{
							boost::mutex::scoped_lock l_lock(m_pInformationWindow->m_oMutex);
							m_pInformationWindow->m_bIsCMSInRange = true;
							m_pInformationWindow->m_bIsChanged = true;
						}
					}
				}

				if(m_oBridge.isBatteryLow())
				{
					// we print a warning message once every 2secs maximum
					if(System::Time::getTime() > m_ui32LastWarningTime + 2000)
					{
						l_bWarningDisplayed = true;
						m_rDriverContext.getLogManager() << LogLevel_Warning << "("<<((System::Time::getTime() - m_ui32StartTime)/1000)<<"') Device battery is low !\n";
						{
							boost::mutex::scoped_lock l_lock(m_pInformationWindow->m_oMutex);
							m_pInformationWindow->m_bIsBatteryLow = true;
							m_pInformationWindow->m_bIsChanged = true;
						}
					}
					m_bBatteryCurrentlyOk = false;
					m_bBatteryBackOk = false;
				}
				else
				{
					if(!m_bBatteryCurrentlyOk)
					{
						m_ui32LastBatteryLow = System::Time::getTime();
						m_rDriverContext.getLogManager() << LogLevel_Warning << "("<<((System::Time::getTime() - m_ui32StartTime)/1000)<<"') Device battery seems ok.\n";
						m_bBatteryCurrentlyOk = true;
					}
					// CMS is considered "back in range" if it stayed in range for more than 500ms 
					// -> avoids changing the gtk markup too often (which would not be readable for the user nor efficient)
					else if(!m_bBatteryBackOk && (System::Time::getTime() > m_ui32LastBatteryLow + 100))
					{
						m_bBatteryBackOk = true;
						m_rDriverContext.getLogManager() << LogLevel_Info << "Back in range \n";
						{
							boost::mutex::scoped_lock l_lock(m_pInformationWindow->m_oMutex);
							m_pInformationWindow->m_bIsBatteryLow = false;
							m_pInformationWindow->m_bIsChanged = true;
						}
					}
				}

				if(l_bWarningDisplayed)
				{
					m_ui32LastWarningTime = System::Time::getTime();
				}

				m_pCallback->setStimulationSet(m_oStimulationSet);
				m_oStimulationSet.clear();
				
			}
		}
		if(l_i32ByteRead < 0)
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "An error occured while reading data from device !\n";
			m_bAcquisitionStopped = true;
			{
				boost::mutex::scoped_lock l_lock(m_pInformationWindow->m_oMutex);
				m_pInformationWindow->m_sErrorMessage = "<span color=\"darkred\">An error occured while reading data from device !</span>";
				m_pInformationWindow->m_bIsChanged = true;
			}
			return true;
		}

		m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());
	}
	else
	{
		// acquisition is not started, but device is.
		int32 l_i32ByteRead = m_oBridge.read();
		while(l_i32ByteRead > 0)
		{
			if(!m_oBridge.discard()) // we discard the samples.
			{
				m_rDriverContext.getLogManager() << LogLevel_Error << "An error occured while dropping samples.\n";
				m_bAcquisitionStopped = true;
				{
					boost::mutex::scoped_lock l_lock(m_pInformationWindow->m_oMutex);
					m_pInformationWindow->m_sErrorMessage = "<span color=\"darkred\">An error occured while dropping samples.</span>";
					m_pInformationWindow->m_bIsChanged = true;
				}
				return true;
			}
			l_i32ByteRead = m_oBridge.read();
		}
		if(l_i32ByteRead < 0)
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "An error occured while reading data from device (drop)!\n";
			m_bAcquisitionStopped = true;
			{
				boost::mutex::scoped_lock l_lock(m_pInformationWindow->m_oMutex);
				m_pInformationWindow->m_sErrorMessage = "<span color=\"darkred\">An error occured while reading data from device (drop)!</span>";
				m_pInformationWindow->m_bIsChanged = true;
			}
			return true;
		}
	}

	return true;
}

boolean CDriverBioSemiActiveTwo::stop(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }

	m_rDriverContext.getLogManager() << LogLevel_Trace << "Acquisition stopped...\n";
	
	return true;
}

boolean CDriverBioSemiActiveTwo::uninitialize(void)
{
	{
		boost::mutex::scoped_lock l_lock(m_pInformationWindow->m_oMutex);
		m_pInformationWindow->m_bIsChanged = true;
		m_pInformationWindow->m_bIsAcquisitionEnded = true;
	}
	// Rename EX channels as "" so that the names are not saved as settings
	if(m_oBridge.isUseEXChannels())
	{
		for (uint32 i = m_oHeader.getChannelCount() - m_oBridge.getEXChannelCount(); i < m_oHeader.getChannelCount(); i++)
		{
			m_oHeader.setChannelName(i, "");
			m_oHeader.setChannelUnits(i, OVTK_UNIT_Volts, OVTK_FACTOR_Micro);
		}
	}

	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }
	if(!m_oBridge.close())
	{
		m_rDriverContext.getLogManager() << LogLevel_Warning << "Could not close the device.\n";
		return false;
	}
	m_rDriverContext.getLogManager() << LogLevel_Error << "Driver uninitialized...\n";
	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverBioSemiActiveTwo::isConfigurable(void)
{
	return true;
}

boolean CDriverBioSemiActiveTwo::configure(void)
{
	CConfigurationBioSemiActiveTwo m_oConfiguration(OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-BioSemi-ActiveTwo.ui", m_bUseEXChannel);

	if(m_oConfiguration.configure(m_oHeader))
	{
		m_oBridge.setUseEXChannels(m_bUseEXChannel);
		if(m_oBridge.isUseEXChannels())
		{
			m_oHeader.setChannelCount(m_oHeader.getChannelCount() + BIOSEMI_ACTIVETWO_EXCHANNELCOUNT);
		}

		m_oSettings.save();

		return true;
	}

	return false;
}
/*
uint32 CDriverBioSemiActiveTwo::getChannelCount()
{
	if(m_bUseEXChannel)
	{
		return m_oHeader.getChannelCount() + m_oBridge.getEXChannelCount(); 
	}
	else
	{
		return m_oHeader.getChannelCount();
	}
}
*/
#endif
