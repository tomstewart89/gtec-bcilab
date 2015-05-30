/*
 * \author Christoph Veigl, Yann Renard
 *
 * \copyright AGPL3
 *
 */
#include "ovasCConfigurationOpenEEGModularEEG.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;

CConfigurationOpenEEGModularEEG::CConfigurationOpenEEGModularEEG(const char* sGtkBuilderFileName, OpenViBE::uint32& rUSBIndex)
	:CConfigurationBuilder(sGtkBuilderFileName)
	,m_rUSBIndex(rUSBIndex)
{
	m_pListStore=gtk_list_store_new(1, G_TYPE_STRING);
}

CConfigurationOpenEEGModularEEG::~CConfigurationOpenEEGModularEEG(void)
{
	g_object_unref(m_pListStore);
}

boolean CConfigurationOpenEEGModularEEG::preConfigure(void)
{
	if(!CConfigurationBuilder::preConfigure())
	{
		return false;
	}

	::GtkComboBox* l_pComboBox=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_device"));

	g_object_unref(m_pListStore);
	m_pListStore=gtk_list_store_new(1, G_TYPE_STRING);

	gtk_combo_box_set_model(l_pComboBox, GTK_TREE_MODEL(m_pListStore));

	char l_sBuffer[1024];
	boolean l_bSelected=false;

	for(uint32 i=1; i<17; i++)
	{
#if defined TARGET_OS_Windows
		::sprintf(l_sBuffer, "\\\\.\\COM%i", i);
#elif defined TARGET_OS_Linux
		if(i<10)
		{
			::sprintf(l_sBuffer, i<10?"/dev/ttyS%d":"/dev/ttyUSB%d", i);
		}
		else
		{
			::sprintf(l_sBuffer, "/dev/ttyUSB%d", i-10);
		}
#else
		::sprintf(l_sBuffer, "");
#endif
		::gtk_combo_box_append_text(l_pComboBox, l_sBuffer);
		if(m_rUSBIndex==i)
		{
			::gtk_combo_box_set_active(l_pComboBox, i-1);
			l_bSelected=true;
		}
	}

	if(!l_bSelected)
	{
		::gtk_combo_box_set_active(l_pComboBox, 0);
	}

	return true;
}

boolean CConfigurationOpenEEGModularEEG::postConfigure(void)
{
	::GtkComboBox* l_pComboBox=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_device"));

	if(m_bApplyConfiguration)
	{
		int l_iUSBIndex=gtk_combo_box_get_active(l_pComboBox);
		if(l_iUSBIndex>=0)
		{
			m_rUSBIndex=(uint32)l_iUSBIndex+1;
		}
	}

	if(!CConfigurationBuilder::postConfigure())
	{
		return false;
	}
	return true;
}
