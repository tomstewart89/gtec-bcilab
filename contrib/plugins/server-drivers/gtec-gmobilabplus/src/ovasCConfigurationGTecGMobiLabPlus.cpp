/**
 * The gMobilab driver was contributed
 * by Lucie Daubigney from Supelec Metz
 */

#include "ovasCConfigurationGTecGMobiLabPlus.h"

#if defined TARGET_HAS_ThirdPartyGMobiLabPlusAPI

#define boolean OpenViBE::boolean

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

CConfigurationGTecGMobiLabPlus::CConfigurationGTecGMobiLabPlus(const char* sGtkBuilderFileName, std::string& rPortName, boolean& rTestMode)
	:CConfigurationBuilder(sGtkBuilderFileName)
	,m_rPortName(rPortName)
	,m_rTestMode(rTestMode)
{
}

boolean CConfigurationGTecGMobiLabPlus::preConfigure(void)
{
	if(!CConfigurationBuilder::preConfigure())
	{
		return false;
	}

	::GtkEntry* l_pEntryPort=GTK_ENTRY(gtk_builder_get_object(m_pBuilderConfigureInterface, "entry_port"));
	::gtk_entry_set_text(l_pEntryPort, m_rPortName.c_str());

	::GtkToggleButton* l_pToggleTestMode = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_testmode"));
	gtk_toggle_button_set_active(l_pToggleTestMode, m_rTestMode);

	return true;
}


boolean CConfigurationGTecGMobiLabPlus::postConfigure(void)
{
	if(m_bApplyConfiguration)
	{
		::GtkEntry* l_pEntryPort=GTK_ENTRY(gtk_builder_get_object(m_pBuilderConfigureInterface, "entry_port"));
		m_rPortName = ::gtk_entry_get_text(l_pEntryPort);

		::GtkToggleButton* l_pToggleTestMode = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_testmode"));
		m_rTestMode = (::gtk_toggle_button_get_active(l_pToggleTestMode)>0);
	}

	if(!CConfigurationBuilder::postConfigure())
	{
		return false;
	}

	return true;
}

#endif // TARGET_HAS_ThirdPartyGMobiLabPlusAPI
