#include "ovdCEnumerationSettingView.h"
#include "../ovd_base.h"

#include <algorithm> // std::sort
#include <iostream>
#include <map>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace OpenViBEDesigner::Setting;

static void on_change(::GtkEntry *entry, gpointer pUserData)
{
	static_cast<CEnumerationSettingView *>(pUserData)->onChange();
}

CEnumerationSettingView::CEnumerationSettingView(OpenViBE::Kernel::IBox &rBox, OpenViBE::uint32 ui32Index,
												 CString &rBuilderName, const Kernel::IKernelContext &rKernelContext,
												 const OpenViBE::CIdentifier &rTypeIdentifier):
	CAbstractSettingView(rBox, ui32Index, rBuilderName, "settings_collection-comboboxentry_setting_enumeration"),
	m_oTypeIdentifier(rTypeIdentifier),
	m_rKernelContext(rKernelContext),
	m_bOnValueSetting(false)
{
	p=false;
	::GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	m_pComboBox = GTK_COMBO_BOX(l_pSettingWidget);

	std::vector<std::string> l_vEntries;

	for(uint64 i=0; i<m_rKernelContext.getTypeManager().getEnumerationEntryCount(m_oTypeIdentifier); i++)
	{
		CString l_sEntryName;
		uint64 l_ui64EntryValue;
		if(m_rKernelContext.getTypeManager().getEnumerationEntry(m_oTypeIdentifier, i, l_sEntryName, l_ui64EntryValue))
		{
			l_vEntries.push_back(l_sEntryName.toASCIIString());
		}
	}

	std::sort(l_vEntries.begin(), l_vEntries.end());

	::GtkTreeIter l_oListIter;
	::GtkListStore* l_pList=GTK_LIST_STORE(gtk_combo_box_get_model(m_pComboBox));
	gtk_combo_box_set_wrap_width(m_pComboBox, 0);
	gtk_list_store_clear(l_pList);

	for(size_t i=0;i<l_vEntries.size();i++)
	{
		gtk_list_store_append(l_pList, &l_oListIter);
		gtk_list_store_set(l_pList, &l_oListIter, 0, l_vEntries[i].c_str(), -1);

		m_mEntriesIndex[CString(l_vEntries[i].c_str())] = static_cast<uint64>(i);
	}

	initializeValue();

	g_signal_connect(G_OBJECT(m_pComboBox), "changed", G_CALLBACK(on_change), this);
}


void CEnumerationSettingView::getValue(OpenViBE::CString &rValue) const
{
	rValue = CString(gtk_combo_box_get_active_text(m_pComboBox));
}


void CEnumerationSettingView::setValue(const OpenViBE::CString &rValue)
{
	m_bOnValueSetting = true;
	gtk_combo_box_set_active(m_pComboBox, (gint)m_mEntriesIndex[rValue]);
	m_bOnValueSetting =false;
}

void CEnumerationSettingView::onChange()
{
	if(!m_bOnValueSetting)
	{
		gchar* l_sValue = gtk_combo_box_get_active_text(m_pComboBox);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
		g_free(l_sValue);
	}
}

