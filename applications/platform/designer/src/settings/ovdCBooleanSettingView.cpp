#include "ovdCBooleanSettingView.h"
#include "../ovd_base.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace OpenViBEDesigner::Setting;

static void on_checkbutton_setting_boolean_pressed(::GtkToggleButton* pButton, gpointer pUserData)
{
	static_cast<CBooleanSettingView *>(pUserData)->toggleButtonClick();
}

CBooleanSettingView::CBooleanSettingView(OpenViBE::Kernel::IBox &rBox, OpenViBE::uint32 ui32Index, CString &rBuilderName):
	CAbstractSettingView(rBox, ui32Index, rBuilderName, "settings_collection-hbox_setting_boolean"), m_bOnValueSetting(false)
{
	::GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	std::vector< ::GtkWidget* > l_vWidget;
	extractWidget(l_pSettingWidget, l_vWidget);
	m_pToggle = GTK_TOGGLE_BUTTON(l_vWidget[1]);
	m_pEntry = GTK_ENTRY(l_vWidget[0]);
	gtk_widget_set_sensitive(GTK_WIDGET(m_pEntry), false);

	g_signal_connect(G_OBJECT(m_pToggle), "toggled", G_CALLBACK(on_checkbutton_setting_boolean_pressed), this);

	initializeValue();
}


void CBooleanSettingView::getValue(OpenViBE::CString &rValue) const
{
	rValue = CString(gtk_entry_get_text(m_pEntry));
}


void CBooleanSettingView::setValue(const OpenViBE::CString &rValue)
{
	m_bOnValueSetting = true;
	if(rValue==CString("true"))
	{
		gtk_toggle_button_set_active(m_pToggle, true);
	}
	else if(rValue==CString("false"))
	{
		gtk_toggle_button_set_active(m_pToggle, false);
	}
	else
	{
		gtk_toggle_button_set_inconsistent(m_pToggle, true);
	}

	gtk_entry_set_text(m_pEntry, rValue);
	m_bOnValueSetting =false;
}


void CBooleanSettingView::toggleButtonClick()
{
	if(!m_bOnValueSetting)
	{
		if(::gtk_toggle_button_get_active(m_pToggle))
		{
			getBox().setSettingValue(getSettingIndex(), "true");
		}
		else
		{
			getBox().setSettingValue(getSettingIndex(), "false");
		}
	}

}
