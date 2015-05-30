/*
 * ovasCConfigurationTMSi.h
 *
 * Copyright (c) 2014, Mensia Technologies SA. All rights reserved.
 * -- Rights transferred to Inria, contract signed 21.11.2014
 *
 */

#ifndef __OpenViBE_AcquisitionServer_CConfigurationTMSi_H__
#define __OpenViBE_AcquisitionServer_CConfigurationTMSi_H__

#include "../ovasCConfigurationBuilder.h"
#include <gtk/gtk.h>
#include <string>
#include <iostream>

namespace OpenViBEAcquisitionServer
{
	class CDriverTMSi;

	class CConfigurationTMSi : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:
		CConfigurationTMSi(const char* sGtkBuilderFileName, CDriverTMSi* pDriver);
		virtual ~CConfigurationTMSi();

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

		void fillDeviceCombobox();
		OpenViBE::boolean fillSamplingFrequencyCombobox();
		void fillAdditionalChannelsTable();
		void clearAdditionalChannelsTable();
		OpenViBE::CString getActiveAdditionalChannels();

		void showWaitWindow();
		void hideWaitWindow();

	public:
		CDriverTMSi* m_pDriver;

		GtkSpinButton* m_pSpinButtonChannelCount;
		GtkComboBox* m_pComboBoxConnectionProtocol;
		GtkComboBox* m_pComboBoxDeviceIdentifier;
		GtkComboBox* m_pComboBoxSamplingFrequency;
		GtkComboBox* m_pComboBoxImpedanceLimit;
//		GtkToggleButton* m_pToggleButtonCommonAverageReference;
		GtkLabel* m_pLabelAdditionalChannels;
		GtkTable* m_pTableAdditionalChannels;
		std::vector<GtkCheckButton*> m_vAdditionalChannelCheckButtons;
	private:
		std::vector<OpenViBE::CString> m_vAdditionalChannelNames;


		GtkWidget* m_pWaitWindow;
	};

}

#endif // __OpenViBE_AcquisitionServer_CConfigurationTMSi_H__
