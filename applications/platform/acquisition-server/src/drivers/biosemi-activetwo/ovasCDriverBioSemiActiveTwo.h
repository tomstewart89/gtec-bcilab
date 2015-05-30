/*
 * ovasCDriverBioSemiActiveTwo.h
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

#ifndef __OpenViBE_AcquisitionServer_CDriverBioSemiActiveTwo_H__
#define __OpenViBE_AcquisitionServer_CDriverBioSemiActiveTwo_H__

#include "ovasIDriver.h"
#include "../ovasCHeader.h"

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

#include "mCBridgeBioSemiActiveTwo.h"

#include "gtk/gtk.h"
#include "boost/thread.hpp"
#include <vector>

namespace OpenViBEAcquisitionServer
{
	/**
	* SInformationWindow: encapsulate the GtkBuilder used to build the window
	* and the information that we want to display in it.
	* The use of a structure allows to use a GLib idle loop.
	*/
	typedef struct SInformationWindow
	{
		SInformationWindow():
			m_bIsCMSInRange(false)
			,m_bIsBatteryLow(false)
			,m_sErrorMessage("")
			,m_bIsAcquisitionEnded(false)
			,m_bIsChanged(false)
		{};
		// Builder of the window
		GtkBuilder* m_pBuilderConfigureInterface;
		// When set to true, the idle loop is stopped and the window is destroyed
		bool m_bIsAcquisitionEnded;
		// Set to true when at least one of the acquisition window member changed
		bool m_bIsChanged;
		bool m_bIsCMSInRange;
		bool m_bIsBatteryLow;
		std::string m_sErrorMessage;
	public:
		// The window is changed in a idle loop in function of m_bIsCMSInRange, 
		// m_bIsBatteryLow and m_sErrorMessage that change in the driver loop
		// A mutex is necessary to secure the access to the data
		boost::mutex m_oMutex;
	} SInformationWindow;

	class CDriverBioSemiActiveTwo : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverBioSemiActiveTwo(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual void release(void);
		virtual const char* getName(void);

		virtual OpenViBE::boolean initialize(
			const OpenViBE::uint32 ui32SampleCountPerSentBlock,
			OpenViBEAcquisitionServer::IDriverCallback& rCallback);
		virtual OpenViBE::boolean uninitialize(void);

		virtual OpenViBE::boolean start(void);
		virtual OpenViBE::boolean stop(void);
		virtual OpenViBE::boolean loop(void);

		virtual OpenViBE::boolean isConfigurable(void);
		virtual OpenViBE::boolean configure(void);
		virtual const OpenViBEAcquisitionServer::IHeader* getHeader(void) { return &m_oHeader; }
		//OpenViBE::uint32 getChannelCount();

	protected:
		SettingsHelper m_oSettings;

		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;
		OpenViBEAcquisitionServer::CHeader m_oHeader;

		std::vector < unsigned int > m_vImpedance;
		std::vector < OpenViBE::float32 > m_vSample;
	
		OpenViBE::CStimulationSet m_oStimulationSet;
	
		Mensia::CBridgeBioSemiActiveTwo m_oBridge;
		
		std::vector<OpenViBE::boolean> m_vTriggers;
		OpenViBE::uint64 m_ui64SampleCount;

		OpenViBE::boolean m_bCMCurrentlyInRange;
		OpenViBE::boolean m_bBatteryCurrentlyOk;
		OpenViBE::uint32 m_ui32LastWarningTime;
		// Used to determine for how long CMS was in range
		OpenViBE::uint32 m_ui32LastCmsBackInRange;
		OpenViBE::uint32 m_ui32LastBatteryLow;
		// True if CMS is in range for more than 100ms
		OpenViBE::boolean m_bCMSBackInRange;
		OpenViBE::boolean m_bBatteryBackOk;
		OpenViBE::uint32 m_ui32StartTime;
		OpenViBE::boolean m_bUseEXChannel;
		// Set to true 
		OpenViBE::boolean m_bAcquisitionStopped;

		SInformationWindow* m_pInformationWindow;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverBioSemiActiveTwo_H__
#endif // TARGET_HAS_ThirdPartyBioSemiAPI
