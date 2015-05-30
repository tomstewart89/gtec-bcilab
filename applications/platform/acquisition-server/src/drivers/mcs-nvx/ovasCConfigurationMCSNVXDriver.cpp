
#if defined(TARGET_HAS_ThirdPartyMCS)

#include "ovasCConfigurationMCSNVXDriver.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace std;

/*_________________________________________________

Insert callback to specific widget here
Example with a button that launch a calibration of the device:

//Callback connected to a dedicated gtk button:
static void button_calibrate_pressed_cb(::GtkButton* pButton, void* pUserData)
{
	CConfigurationMKSNVXDriver* l_pConfig=static_cast<CConfigurationMKSNVXDriver*>(pUserData);
	l_pConfig->buttonCalibratePressedCB();
}

//Callback actually called:
void CConfigurationGTecGUSBamp::buttonCalibratePressedCB(void)
{
	// Connect to the hardware, ask for calibration, verify the return code, etc.
}
_________________________________________________*/

// If you added more reference attribute, initialize them here
CConfigurationMKSNVXDriver::CConfigurationMKSNVXDriver(IDriverContext& rDriverContext, const char* sGtkBuilderFileName, OpenViBE::uint32& dataMode, bool& auxChannels)
	:CConfigurationBuilder(sGtkBuilderFileName)
	,m_rDriverContext(rDriverContext)
	, dataMode_(dataMode)
	, showAuxChannels_(auxChannels)
{
}

boolean CConfigurationMKSNVXDriver::preConfigure(void)
{
	if(! CConfigurationBuilder::preConfigure())
	{
		return false;
	}

	// Connect here all callbacks
	// Example:
	// g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "button_calibrate"), "pressed", G_CALLBACK(button_calibrate_pressed_cb), this);

	// Insert here the pre-configure code.
	// For example, you may want to check if a device is currently connected
	// and if more than one are connected. Then you can list in a dedicated combo-box 
	// the device currently connected so the user can choose which one he wants to acquire from.
	::GtkComboBox* l_pComboDataMode = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_mode"));
	::gtk_combo_box_set_active(l_pComboDataMode, dataMode_);
	::GtkToggleButton* l_pToggleShowAuxChannels = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_show_aux_channels"));
	::gtk_toggle_button_set_active(l_pToggleShowAuxChannels, showAuxChannels_);
	return true;
}

boolean CConfigurationMKSNVXDriver::postConfigure(void)
{
	if(m_bApplyConfiguration)
	{
		// If the user pressed the "apply" button, you need to save the changes made in the configuration.
		// For example, you can save the connection ID of the selected device:
		// m_ui32ConnectionID = <value-from-gtk-widget>
		::GtkComboBox* l_pComboDataMode = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_mode"));
		dataMode_ = (uint32)::gtk_combo_box_get_active(l_pComboDataMode);
		::GtkToggleButton* l_pToggleShowAuxChannels = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_show_aux_channels"));
		showAuxChannels_ = (::gtk_toggle_button_get_active(l_pToggleShowAuxChannels) > 0);
	}

	if(! CConfigurationBuilder::postConfigure()) // normal header is filled (Subject ID, Age, Gender, channels, sampling frequency), ressources are realesed
	{
		return false;
	}

	return true;
}


#endif