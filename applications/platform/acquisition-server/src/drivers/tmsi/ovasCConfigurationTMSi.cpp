/*
 * ovasCConfigurationTMSi.cpp
 *
 * Copyright (c) 2014, Mensia Technologies SA. All rights reserved.
 * -- Rights transferred to Inria, contract signed 21.11.2014
 *
 */

#include "ovasCConfigurationTMSi.h"
#include "ovasCDriverTMSi.h"
#include "ovasCTMSiAccess.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;

#if defined TARGET_HAS_ThirdPartyTMSi

namespace
{

	void combobox_communication_protocol_changed_cb(GtkComboBox* pComboBox, CConfigurationTMSi* pConfiguration)
	{
		pConfiguration->showWaitWindow();
		pConfiguration->m_pDriver->m_pTMSiAccess->initializeTMSiLibrary(gtk_combo_box_get_active_text(pComboBox));
		pConfiguration->hideWaitWindow();

		pConfiguration->fillDeviceCombobox();
	}

	void combobox_device_changed_cb(GtkComboBox* pComboBox, CConfigurationTMSi* pConfiguration)
	{
		CTMSiAccess* l_pTMSiAccess = pConfiguration->m_pDriver->m_pTMSiAccess;

		CString l_sCurrentDevice = CString(gtk_combo_box_get_active_text(pComboBox));

		// If the currently chosen device has no name it means the combo box is empty
		if (l_sCurrentDevice == CString(""))
		{
			pConfiguration->clearAdditionalChannelsTable();

			// clear the list of sampling frequencies
			gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(pConfiguration->m_pComboBoxSamplingFrequency)));
			return;
		}

		pConfiguration->showWaitWindow();
		l_pTMSiAccess->openFrontEnd(l_sCurrentDevice);

		bool l_bDeviceHasImpedanceTestingAbility = false;
		if (!l_pTMSiAccess->getImpedanceTestingCapability(&l_bDeviceHasImpedanceTestingAbility))
		{
			l_pTMSiAccess->closeFrontEnd();
			pConfiguration->hideWaitWindow();
			return;
		}


		if (!l_pTMSiAccess->calculateSignalFormat(l_sCurrentDevice))
		{
			l_pTMSiAccess->closeFrontEnd();
			pConfiguration->hideWaitWindow();
			return;
		}

		if (!pConfiguration->fillSamplingFrequencyCombobox())
		{
			l_pTMSiAccess->freeSignalFormat();
			l_pTMSiAccess->closeFrontEnd();
			pConfiguration->hideWaitWindow();
			return;
		}

		pConfiguration->fillAdditionalChannelsTable();

		l_pTMSiAccess->freeSignalFormat();
		l_pTMSiAccess->closeFrontEnd();
		pConfiguration->hideWaitWindow();

		// activate/deactivate the impedance checking dropbown box
		gtk_widget_set_sensitive(GTK_WIDGET(pConfiguration->m_pComboBoxImpedanceLimit), l_bDeviceHasImpedanceTestingAbility);

		// modify the EEG channel count spinbox to reflect the detected maximum number of channels
		GtkAdjustment* l_pEEGChannelCountAdjustment = gtk_spin_button_get_adjustment(pConfiguration->m_pSpinButtonChannelCount);
		gtk_adjustment_set_upper(l_pEEGChannelCountAdjustment, l_pTMSiAccess->getMaximumEEGChannelCount());
		gtk_adjustment_set_value(l_pEEGChannelCountAdjustment, l_pTMSiAccess->getMaximumEEGChannelCount());

	}

	void channel_count_changed_cb(GtkSpinButton* pSpinButton, CConfigurationTMSi* pConfiguration)
	{
		gtk_spin_button_update(pSpinButton);
	}

	void remove_widgets(GtkWidget *widget, gpointer data)
	{
		gtk_widget_destroy(widget);
	}
}

/* Fills the combobox containing list of devices with TMSi devices connected via the
 * currently selected protocol
 */
void CConfigurationTMSi::fillDeviceCombobox()
{
	int l_iCurrentDeviceIndex = -1;
	GtkListStore* l_pDeviceListStore = GTK_LIST_STORE(gtk_combo_box_get_model(m_pComboBoxDeviceIdentifier));
	gtk_list_store_clear(l_pDeviceListStore);

	for (size_t l_uiDeviceIndex = 0; l_uiDeviceIndex < m_pDriver->m_pTMSiAccess->getDeviceList().size(); l_uiDeviceIndex++)
	{
		gtk_combo_box_append_text(m_pComboBoxDeviceIdentifier, m_pDriver->m_pTMSiAccess->getDeviceList()[l_uiDeviceIndex].toASCIIString());
		if (m_pDriver->m_sDeviceIdentifier == m_pDriver->m_pTMSiAccess->getDeviceList()[l_uiDeviceIndex])
		{
			l_iCurrentDeviceIndex = l_uiDeviceIndex;
		}
	}

	// set the active combobox field to the currently set device (if it is present)
	if (l_iCurrentDeviceIndex != -1)
	{
		gtk_combo_box_set_active(m_pComboBoxDeviceIdentifier, l_iCurrentDeviceIndex);
	}

}

// fills the sampling freuqency combobox, as a side-effect, will also correctly set the maximum number of EEG channels inside the TMSiAccess object
OpenViBE::boolean CConfigurationTMSi::fillSamplingFrequencyCombobox()
{
	CTMSiAccess* l_pTMSiAccess = m_pDriver->m_pTMSiAccess;

	// clear the list of sampling frequencies
	GtkListStore* l_pFrequencyListStore = GTK_LIST_STORE(gtk_combo_box_get_model(m_pComboBoxSamplingFrequency));
	gtk_list_store_clear(l_pFrequencyListStore);

	showWaitWindow();
	std::vector<unsigned long> l_vDeviceSamplingFrequencies = l_pTMSiAccess->discoverDeviceSamplingFrequencies();
	// Emtpy the list of acquisition frequencies and fill it with the frequencies supported by the device
	hideWaitWindow();

	if (l_vDeviceSamplingFrequencies.empty())
	{
		return false;
	}

	int l_iCurrentSamplingFrequencyIndex = -1;
	for (size_t l_uiSamplingFrequencyIndex = 0; l_uiSamplingFrequencyIndex < l_vDeviceSamplingFrequencies.size(); l_uiSamplingFrequencyIndex++)
	{
		std::stringstream l_ssFrequencyString;

		// TMSi uses sampling frequencies in mHz but we want to display them in Hz
		l_ssFrequencyString << l_vDeviceSamplingFrequencies[l_uiSamplingFrequencyIndex] / 1000;
		gtk_combo_box_append_text(m_pComboBoxSamplingFrequency, l_ssFrequencyString.str().c_str());

		if (m_pDriver->m_oHeader.getSamplingFrequency() == l_vDeviceSamplingFrequencies[l_uiSamplingFrequencyIndex] / 1000)
		{
			l_iCurrentSamplingFrequencyIndex = l_uiSamplingFrequencyIndex;
		}
	}

	// set the active combobox field to the currently set sampling frequency (if it is present)
	if (l_iCurrentSamplingFrequencyIndex != -1)
	{
		gtk_combo_box_set_active(m_pComboBoxSamplingFrequency, l_iCurrentSamplingFrequencyIndex);
	}
	else
	{
		// set the sampling frequency to the biggest one
		gtk_combo_box_set_active(m_pComboBoxSamplingFrequency, l_vDeviceSamplingFrequencies.size() - 1);
	}

	return true;
}

// clears the table with additional channels
void CConfigurationTMSi::clearAdditionalChannelsTable()
{
	gtk_container_foreach(GTK_CONTAINER(m_pTableAdditionalChannels), remove_widgets, m_pTableAdditionalChannels);
	gtk_table_resize(m_pTableAdditionalChannels, 1, 2);
	m_vAdditionalChannelCheckButtons.erase(m_vAdditionalChannelCheckButtons.begin(), m_vAdditionalChannelCheckButtons.end());
	m_vAdditionalChannelNames.erase(m_vAdditionalChannelNames.begin(), m_vAdditionalChannelNames.end());
	gtk_widget_hide(GTK_WIDGET(m_pTableAdditionalChannels));
	gtk_widget_hide(GTK_WIDGET(m_pLabelAdditionalChannels));
}

// fills the table with additional channels
void CConfigurationTMSi::fillAdditionalChannelsTable()
{
	CTMSiAccess* l_pTMSiAccess = m_pDriver->m_pTMSiAccess;

	uint32 l_ui32AdditionalChannelsCount = l_pTMSiAccess->getActualChannelCount() - l_pTMSiAccess->getMaximumEEGChannelCount();
	uint32 l_ui32FirstAdditionalChannelIndex = l_pTMSiAccess->getMaximumEEGChannelCount();

	clearAdditionalChannelsTable();
	gtk_table_resize(m_pTableAdditionalChannels, l_ui32AdditionalChannelsCount, 2);

	for (size_t l_uiChannelIndex = 0; l_uiChannelIndex < l_ui32AdditionalChannelsCount; l_uiChannelIndex++)
	{
		size_t l_uiActualIndex = l_uiChannelIndex + l_ui32FirstAdditionalChannelIndex;

		CString l_pChannelLabel = "Channel <b>" + l_pTMSiAccess->getChannelName(l_uiActualIndex) + "</b> of type " + l_pTMSiAccess->getChannelType(l_uiActualIndex).toASCIIString();

		GtkWidget* l_pLabelChannelName = gtk_label_new(l_pChannelLabel.toASCIIString());
		gtk_label_set_use_markup(GTK_LABEL(l_pLabelChannelName), TRUE);

		GtkWidget* l_pCheckButtonChannelActive = gtk_check_button_new();
		gtk_table_set_row_spacings(m_pTableAdditionalChannels, 5);
		m_vAdditionalChannelCheckButtons.push_back(GTK_CHECK_BUTTON(l_pCheckButtonChannelActive));
		m_vAdditionalChannelNames.push_back(l_pTMSiAccess->getChannelName(l_uiActualIndex));

		// if the channel was previously selected, check it in the list
		// the list of selected channels is in form ;CH1;CH2;CH3, thus we seek for substring of type ;CHX;
		if (std::string(m_pDriver->m_sActiveAdditionalChannels).find(std::string(";") + std::string(l_pTMSiAccess->getChannelName(l_uiActualIndex).toASCIIString()) + std::string(";")) != std::string::npos)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pCheckButtonChannelActive), TRUE);
		}
		else
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pCheckButtonChannelActive), FALSE);
		}

		gtk_table_attach_defaults(m_pTableAdditionalChannels, l_pLabelChannelName, 0, 1, l_uiChannelIndex, l_uiChannelIndex+1);
		gtk_table_attach_defaults(m_pTableAdditionalChannels, l_pCheckButtonChannelActive, 1, 2, l_uiChannelIndex, l_uiChannelIndex+1);
	}

	gtk_widget_show(GTK_WIDGET(m_pLabelAdditionalChannels));
	gtk_widget_show_all(GTK_WIDGET(m_pTableAdditionalChannels));
}


CString CConfigurationTMSi::getActiveAdditionalChannels()
{
	CString l_sAdditionalChannelsString = ";";

	CTMSiAccess* l_pTMSiAccess = m_pDriver->m_pTMSiAccess;

	uint32 l_ui32AdditionalChannelsCount = l_pTMSiAccess->getActualChannelCount() - l_pTMSiAccess->getMaximumEEGChannelCount();

	for (size_t l_uiChannelIndex = 0; l_uiChannelIndex < l_ui32AdditionalChannelsCount; l_uiChannelIndex++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_vAdditionalChannelCheckButtons[l_uiChannelIndex])))
		{
			l_sAdditionalChannelsString = l_sAdditionalChannelsString + m_vAdditionalChannelNames[l_uiChannelIndex] + ";";
		}
	}

	return l_sAdditionalChannelsString;
}

CConfigurationTMSi::CConfigurationTMSi(const char* sGtkBuilderFileName, CDriverTMSi* pDriver)
	:CConfigurationBuilder(sGtkBuilderFileName),
	  m_pDriver(pDriver)
{
	m_pWaitWindow = NULL;
}

CConfigurationTMSi::~CConfigurationTMSi()
{
	// Hide the wait window when the object dies
	hideWaitWindow();
}

boolean CConfigurationTMSi::preConfigure(void)
{
	if(!CConfigurationBuilder::preConfigure())
	{
		return false;
	}

	m_pSpinButtonChannelCount = GTK_SPIN_BUTTON(::gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_number_of_channels"));
	m_pComboBoxSamplingFrequency = GTK_COMBO_BOX(::gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_sampling_frequency"));
	m_pComboBoxConnectionProtocol = GTK_COMBO_BOX(::gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_tmsi_connection_protocol"));
	m_pComboBoxDeviceIdentifier = GTK_COMBO_BOX(::gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_tmsi_device"));
	m_pComboBoxImpedanceLimit = GTK_COMBO_BOX(::gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_tmsi_impedance_limit"));
//	m_pToggleButtonCommonAverageReference = GTK_TOGGLE_BUTTON(::gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_tmsi_average_reference"));
	m_pLabelAdditionalChannels = GTK_LABEL(::gtk_builder_get_object(m_pBuilderConfigureInterface, "label_tmsi_additional_channels"));
	m_pTableAdditionalChannels = GTK_TABLE(::gtk_builder_get_object(m_pBuilderConfigureInterface, "table_tmsi_additional_channels"));

	g_signal_connect(G_OBJECT(m_pComboBoxConnectionProtocol), "changed", G_CALLBACK(combobox_communication_protocol_changed_cb), this);
	g_signal_connect(G_OBJECT(m_pComboBoxDeviceIdentifier), "changed", G_CALLBACK(combobox_device_changed_cb), this);
	g_signal_connect(G_OBJECT(m_pSpinButtonChannelCount), "changed", G_CALLBACK(channel_count_changed_cb), this);

	// The order of the drivers corresponds to indexes in the ConnectionProtocols map inside the driver
	gtk_combo_box_append_text(m_pComboBoxConnectionProtocol, "USB");
	gtk_combo_box_append_text(m_pComboBoxConnectionProtocol, "WiFi");
	gtk_combo_box_append_text(m_pComboBoxConnectionProtocol, "Network");
	gtk_combo_box_append_text(m_pComboBoxConnectionProtocol, "Bluetooth");

	// Get the index of the currently set connection protocol from the map
	gtk_combo_box_set_active(m_pComboBoxConnectionProtocol, m_pDriver->m_pTMSiAccess->getConnectionProtocols()[m_pDriver->m_sConnectionProtocol].second);

	gtk_combo_box_set_active(m_pComboBoxImpedanceLimit, static_cast<gint>(m_pDriver->m_ui64ImpedanceLimit));

//	gtk_toggle_button_set_active(m_pToggleButtonCommonAverageReference, m_pDriver->m_bCommonAverageReference);
	gtk_spin_button_set_value(m_pSpinButtonChannelCount, static_cast<double>(m_pDriver->m_ui64ActiveEEGChannels));

	return true;
}

boolean CConfigurationTMSi::postConfigure(void)
{
	if (m_bApplyConfiguration)
	{
		// set the channel count to the number of EEG channels + number of active additional channels
		CTMSiAccess* l_pTMSiAccess = m_pDriver->m_pTMSiAccess;

		m_pDriver->m_sConnectionProtocol = gtk_combo_box_get_active_text(m_pComboBoxConnectionProtocol);
		m_pDriver->m_sDeviceIdentifier = gtk_combo_box_get_active_text(m_pComboBoxDeviceIdentifier);

		// If the device was set to an actual existing device
		if (m_pDriver->m_sDeviceIdentifier != CString(""))
		{
//			m_pDriver->m_bCommonAverageReference = gtk_toggle_button_get_active(m_pToggleButtonCommonAverageReference) != 0;
			m_pDriver->m_sActiveAdditionalChannels = getActiveAdditionalChannels();
			m_pDriver->m_ui64ActiveEEGChannels = gtk_spin_button_get_value_as_int(m_pSpinButtonChannelCount);
			m_pDriver->m_ui64ImpedanceLimit = gtk_combo_box_get_active(m_pComboBoxImpedanceLimit);
		}
		// If the device is unset then set all driver settings to neutral values
		else
		{
//			m_pDriver->m_bCommonAverageReference = 0;
			m_pDriver->m_sActiveAdditionalChannels = CString(";");
			m_pDriver->m_ui64ActiveEEGChannels = 0;
			m_pDriver->m_ui64ImpedanceLimit = 1;
		}

		// Increase the Channel Count spin button to the value of all active channels (EEG+additional)
		uint32 l_ui32ActiveChannels = gtk_spin_button_get_value_as_int(m_pSpinButtonChannelCount);

		for(size_t l_uiAdditionalChannelIndex = 0; l_uiAdditionalChannelIndex < m_vAdditionalChannelCheckButtons.size(); l_uiAdditionalChannelIndex++)
		{
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_vAdditionalChannelCheckButtons[l_uiAdditionalChannelIndex])))
			{
				l_ui32ActiveChannels++;
			}
		}

		GtkAdjustment* l_pChannelCountAdjustment = gtk_spin_button_get_adjustment(m_pSpinButtonChannelCount);
		gtk_adjustment_set_upper(l_pChannelCountAdjustment, l_pTMSiAccess->getActualChannelCount());
		gtk_adjustment_set_value(l_pChannelCountAdjustment, l_ui32ActiveChannels);

	}
	if(!CConfigurationBuilder::postConfigure())
	{
		return false;
	}

	return true;
}

void CConfigurationTMSi::showWaitWindow()
{
	if (m_pWaitWindow != NULL)
	{
		return;
	}

	m_pWaitWindow = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_position(GTK_WINDOW(m_pWaitWindow), GTK_WIN_POS_CENTER);
	gtk_window_set_decorated(GTK_WINDOW(m_pWaitWindow), FALSE);
	gtk_window_set_modal(GTK_WINDOW(m_pWaitWindow), TRUE);
	GdkColor l_oBackgroundColor;
	l_oBackgroundColor.blue = 0;
	l_oBackgroundColor.green = 0xffff;
	l_oBackgroundColor.red = 0xffff;
	gtk_widget_modify_bg(m_pWaitWindow, GTK_STATE_NORMAL, &l_oBackgroundColor);
	GtkWidget* l_pVBox = gtk_vbox_new(FALSE, 0);
	GtkWidget* l_pLabel = gtk_label_new("  COMMUNICATING WITH DEVICE...  ");
	gtk_box_pack_end(GTK_BOX(l_pVBox), l_pLabel, TRUE, TRUE, 10);

	gtk_container_add(GTK_CONTAINER(m_pWaitWindow), l_pVBox);


	gtk_widget_show_all(m_pWaitWindow);
	while (gtk_events_pending ())
	{
		gtk_main_iteration ();
	}

}

void CConfigurationTMSi::hideWaitWindow()
{
	if (m_pWaitWindow != NULL)
	{
		gtk_widget_destroy(GTK_WIDGET(m_pWaitWindow));
		m_pWaitWindow = NULL;
	}
}


#endif // TARGET_OS_Windows
