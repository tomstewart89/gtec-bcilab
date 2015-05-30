#include "ovasCConfigurationGenericRawReader.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;

CConfigurationGenericRawReader::CConfigurationGenericRawReader(const char* sGtkBuilderFileName,
	boolean& rLimitSpeed,
	uint32& rSampleFormat,
	uint32& rSampleEndian,
	uint32& rStartSkip,
	uint32& rHeaderSkip,
	uint32& rFooterSkip,
	CString& rFilename)
	:CConfigurationNetworkBuilder(sGtkBuilderFileName)
	,m_rLimitSpeed(rLimitSpeed)
	,m_rSampleFormat(rSampleFormat)
	,m_rSampleEndian(rSampleEndian)
	,m_rStartSkip(rStartSkip)
	,m_rHeaderSkip(rHeaderSkip)
	,m_rFooterSkip(rFooterSkip)
	,m_rFilename(rFilename)
{
}

boolean CConfigurationGenericRawReader::preConfigure(void)
{
	if(!CConfigurationNetworkBuilder::preConfigure())
	{
		return false;
	}

	::GtkToggleButton* l_pToggleButtonSpeedLimit=GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_limit_speed"));
	::GtkEntry* l_pEntryFilename=GTK_ENTRY(gtk_builder_get_object(m_pBuilderConfigureInterface, "entry_filename"));
	::GtkComboBox* l_pComboBoxEndianness=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_endianness"));
	::GtkComboBox* l_pComboBoxSampleType=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_sample_type"));
	::GtkSpinButton* l_pSpinButtonStartSize=GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_start_size"));
	::GtkSpinButton* l_pSpinButtonHeaderSize=GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_header_size"));
	::GtkSpinButton* l_pSpinButtonFooterSize=GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_footer_size"));

	::gtk_toggle_button_set_active(l_pToggleButtonSpeedLimit, m_rLimitSpeed?TRUE:FALSE);
	::gtk_entry_set_text(l_pEntryFilename, m_rFilename.toASCIIString());
	::gtk_combo_box_set_active(l_pComboBoxEndianness, m_rSampleEndian);
	::gtk_combo_box_set_active(l_pComboBoxSampleType, m_rSampleFormat);
	::gtk_spin_button_set_value(l_pSpinButtonStartSize, m_rStartSkip);
	::gtk_spin_button_set_value(l_pSpinButtonHeaderSize, m_rHeaderSkip);
	::gtk_spin_button_set_value(l_pSpinButtonFooterSize, m_rFooterSkip);

	return true;
}

boolean CConfigurationGenericRawReader::postConfigure(void)
{
	if(m_bApplyConfiguration)
	{
		::GtkToggleButton* l_pToggleButtonSpeedLimit=GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_limit_speed"));
		::GtkEntry* l_pEntryFilename=GTK_ENTRY(gtk_builder_get_object(m_pBuilderConfigureInterface, "entry_filename"));
		::GtkComboBox* l_pComboBoxEndianness=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_endianness"));
		::GtkComboBox* l_pComboBoxSampleType=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_sample_type"));
		::GtkSpinButton* l_pSpinButtonStartSize=GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_start_size"));
		::GtkSpinButton* l_pSpinButtonHeaderSize=GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_header_size"));
		::GtkSpinButton* l_pSpinButtonFooterSize=GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_footer_size"));

		gtk_spin_button_update(l_pSpinButtonStartSize);
		gtk_spin_button_update(l_pSpinButtonHeaderSize);
		gtk_spin_button_update(l_pSpinButtonFooterSize);

		m_rLimitSpeed=::gtk_toggle_button_get_active(l_pToggleButtonSpeedLimit)?true:false;
		m_rFilename=::gtk_entry_get_text(l_pEntryFilename);
		m_rSampleEndian=(uint32)::gtk_combo_box_get_active(l_pComboBoxEndianness);
		m_rSampleFormat=(uint32)::gtk_combo_box_get_active(l_pComboBoxSampleType);
		m_rStartSkip=(uint32)::gtk_spin_button_get_value(l_pSpinButtonStartSize);
		m_rHeaderSkip=(uint32)::gtk_spin_button_get_value(l_pSpinButtonHeaderSize);
		m_rFooterSkip=(uint32)::gtk_spin_button_get_value(l_pSpinButtonFooterSize);
	}

	if(!CConfigurationNetworkBuilder::postConfigure())
	{
		return false;
	}
	return true;
}
