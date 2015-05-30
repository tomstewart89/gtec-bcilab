#include "ovdCAbstractSettingView.h"

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace OpenViBEDesigner::Setting;


static void collect_widget_cb(::GtkWidget* pWidget, gpointer pUserData)
{
	static_cast< std::vector< ::GtkWidget* > *>(pUserData)->push_back(pWidget);
}

CAbstractSettingView::~CAbstractSettingView()
{
	if(GTK_IS_WIDGET(m_pNameWidget)){
		gtk_widget_destroy(m_pNameWidget);
	}
	if(GTK_IS_WIDGET(m_pEntryNameWidget)){
		gtk_widget_destroy(m_pEntryNameWidget);
	}
	if(G_IS_OBJECT(m_pBuilder))
	{
		g_object_unref(m_pBuilder);
	}
}

CAbstractSettingView::CAbstractSettingView(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index,
										   const char* sBuilderName, const char* sWidgetName):
	m_rBox(rBox),
	m_ui32Index(ui32Index),
	m_sSettingWidgetName(""),
	m_pNameWidget(NULL),
	m_pEntryNameWidget(NULL),
	m_bOnValueSetting(false)
{
	if(sBuilderName != NULL)
	{
		m_pBuilder = gtk_builder_new();
		gtk_builder_add_from_file(m_pBuilder, sBuilderName, NULL);
		gtk_builder_connect_signals(m_pBuilder, NULL);

		if(sWidgetName != NULL)
		{
			m_sSettingWidgetName = sWidgetName;
			generateNameWidget();
			m_pEnrtyFieldWidget = generateEntryWidget();
		}
	}
}



OpenViBE::Kernel::IBox& CAbstractSettingView::getBox(void)
{
	return m_rBox;
}

OpenViBE::uint32 CAbstractSettingView::getSettingIndex(void)
{
	return m_ui32Index;
}


void CAbstractSettingView::setNameWidget(::GtkWidget* pWidget)
{
	if(m_pNameWidget){
		gtk_widget_destroy(m_pNameWidget);
	}
	m_pNameWidget = pWidget;
}

::GtkWidget* CAbstractSettingView::getNameWidget(void)
{
	return m_pNameWidget;
}

void CAbstractSettingView::setEntryWidget(::GtkWidget* pWidget)
{
	if(m_pEntryNameWidget){
		gtk_widget_destroy(m_pEntryNameWidget);
	}
	m_pEntryNameWidget = pWidget;
}

void CAbstractSettingView::generateNameWidget(void)
{
	::GtkWidget* l_pSettingName=GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "settings_collection-label_setting_name"));
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingName)), l_pSettingName);
	setNameWidget(l_pSettingName);

	CString l_sSettingName;
	getBox().getSettingName(m_ui32Index, l_sSettingName);
	gtk_label_set_text(GTK_LABEL(l_pSettingName), l_sSettingName);
}

GtkWidget *CAbstractSettingView::generateEntryWidget(void)
{
	::GtkTable* m_pTable = GTK_TABLE(gtk_table_new(1, 3, false));

	::GtkWidget* l_pSettingWidget=GTK_WIDGET(gtk_builder_get_object(m_pBuilder, m_sSettingWidgetName.toASCIIString()));
	::GtkWidget* l_pSettingRevert=GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "settings_collection-button_setting_revert"));
	::GtkWidget* l_pSettingDefault=GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "settings_collection-button_setting_default"));

	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingWidget)), l_pSettingWidget);
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingRevert)), l_pSettingRevert);
	gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pSettingDefault)), l_pSettingDefault);

	gtk_table_attach(m_pTable, l_pSettingWidget,  0, 1, 0, 1, ::GtkAttachOptions(GTK_FILL|GTK_EXPAND), ::GtkAttachOptions(GTK_FILL|GTK_EXPAND), 0, 0);
	//gtk_table_attach(m_pTable, l_pSettingDefault, 1, 2, 0, 1, ::GtkAttachOptions(GTK_SHRINK),          ::GtkAttachOptions(GTK_SHRINK),          0, 0);
	//gtk_table_attach(m_pTable, l_pSettingRevert,  2, 3, 0, 1, ::GtkAttachOptions(GTK_SHRINK),          ::GtkAttachOptions(GTK_SHRINK),          0, 0);

	setEntryWidget(GTK_WIDGET(m_pTable));
	gtk_widget_set_visible(getEntryWidget(), true);
	//If we don't increase the ref counter it will cause trouble when we gonna move it later
	g_object_ref(G_OBJECT(m_pTable));
	return l_pSettingWidget;
}

void CAbstractSettingView::initializeValue()
{
	CString l_sSettingValue;
	getBox().getSettingValue(m_ui32Index, l_sSettingValue);
	setValue(l_sSettingValue);
}

void CAbstractSettingView::extractWidget(GtkWidget *pWidget, std::vector< ::GtkWidget* > &rVector)
{
	gtk_container_foreach(GTK_CONTAINER(pWidget), collect_widget_cb, &rVector);
}

::GtkWidget *CAbstractSettingView::getEntryFieldWidget()
{
	return m_pEnrtyFieldWidget;
}

::GtkWidget* CAbstractSettingView::getEntryWidget(void)
{
	return m_pEntryNameWidget;
}


void CAbstractSettingView::setSettingIndex(uint32 m_ui32NewIndex)
{
	m_ui32Index = m_ui32NewIndex;
}
