#include "ovdCColorSettingView.h"
#include "../ovd_base.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace OpenViBEDesigner::Setting;

// round is defined in <cmath> on c++11
inline int ov_round(double dbl)
{ return dbl >= 0.0 ? (int)(dbl + 0.5) : ((dbl - (double)(int)dbl) <= -0.5 ? (int)dbl : (int)(dbl - 0.5));
}

static void on_button_setting_color_choose_pressed(::GtkColorButton* pButton, gpointer pUserData)
{
	static_cast< CColorSettingView *>(pUserData)->selectColor();
}

static void on_change(::GtkEntry *entry, gpointer pUserData)
{
	static_cast<CColorSettingView *>(pUserData)->onChange();
}


CColorSettingView::CColorSettingView(OpenViBE::Kernel::IBox &rBox, OpenViBE::uint32 ui32Index, CString &rBuilderName, const Kernel::IKernelContext &rKernelContext):
	CAbstractSettingView(rBox, ui32Index, rBuilderName, "settings_collection-hbox_setting_color"), m_rKernelContext(rKernelContext), m_bOnValueSetting(false)
{
	::GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	std::vector< ::GtkWidget* > l_vWidget;
	extractWidget(l_pSettingWidget, l_vWidget);
	m_pEntry = GTK_ENTRY(l_vWidget[0]);
	m_pButton = GTK_COLOR_BUTTON(l_vWidget[1]);

	g_signal_connect(G_OBJECT(m_pEntry), "changed", G_CALLBACK(on_change), this);
	g_signal_connect(G_OBJECT(m_pButton), "color-set", G_CALLBACK(on_button_setting_color_choose_pressed), this);

	initializeValue();
}


void CColorSettingView::getValue(OpenViBE::CString &rValue) const
{
	rValue = CString(gtk_entry_get_text(m_pEntry));
}


void CColorSettingView::setValue(const OpenViBE::CString &rValue)
{
	m_bOnValueSetting = true;
	int r=0, g=0, b=0;
	::sscanf(m_rKernelContext.getConfigurationManager().expand(rValue).toASCIIString(), "%i,%i,%i", &r, &g, &b);

	::GdkColor l_oColor;
	l_oColor.red  =(r*65535)/100;
	l_oColor.green=(g*65535)/100;
	l_oColor.blue =(b*65535)/100;
	gtk_color_button_set_color(m_pButton, &l_oColor);

	gtk_entry_set_text(m_pEntry, rValue);
	m_bOnValueSetting =false;

}

void CColorSettingView::selectColor()
{
	::GdkColor l_oColor;
	gtk_color_button_get_color(m_pButton, &l_oColor);

	char l_sBuffer[1024];
	sprintf(l_sBuffer, "%i,%i,%i", ov_round((l_oColor.red*100)/65535.), ov_round((l_oColor.green*100)/65535.), ov_round((l_oColor.blue*100)/65535.));

	getBox().setSettingValue(getSettingIndex(), l_sBuffer);
}

void CColorSettingView::onChange()
{
	if(!m_bOnValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_pEntry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}

