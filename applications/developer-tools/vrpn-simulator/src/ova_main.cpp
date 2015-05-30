#if defined TARGET_HAS_ThirdPartyVRPN

#include <gtk/gtk.h>

#include <gdk/gdk.h>

#include <openvibe/ov_directories.h>
#include <openvibe/ovCString.h>

#include <vrpn_Button.h>
#include <vrpn_Analog.h>
#include <vrpn_Connection.h>

#include <iostream>

#define _vrpn_peripheral_name_ "openvibe-vrpn@localhost"

// #define _DEBUG

::vrpn_Connection* g_pConnection=NULL;
::vrpn_Button_Server* g_pButtonServer=NULL;
::vrpn_Analog_Server* g_pAnalogServer=NULL;
long g_iAnalogCount=0;
long g_iButtonCount=0;

typedef union
{
	gpointer pUserData;
	int iData;
} TUserData;

void fScrollCB(::GtkRange* pRange, gpointer pUserData)
{
	TUserData  l_oUserData;
	l_oUserData.pUserData=pUserData;
	gdouble l_oNewValue = gtk_range_get_value(pRange);
	g_pAnalogServer->channels()[l_oUserData.iData]=l_oNewValue;

#if defined _DEBUG
	std::cout << "Channel " << (int)(pUserData) << " value changed to " << l_oNewValue << "\n";
#endif
}

void fSwitchCB(::GtkToggleButton* pTogglebutton, gpointer pUserData)
{
	TUserData  l_oUserData;
	l_oUserData.pUserData=pUserData;
	gboolean l_bToggleValue = gtk_toggle_button_get_active(pTogglebutton);
	g_pButtonServer->set_button(l_oUserData.iData, l_bToggleValue);

#if defined _DEBUG
	std::cout << "Channel " << (int)(l_oUserData.iData) << " toggled to " << l_bToggleValue << "\n";
#endif
}

void fConnectCB(::GtkWidget* pWidget, gpointer data)
{
	if(GTK_IS_RANGE(pWidget))
	{
		g_signal_connect(G_OBJECT(pWidget), "value-changed", G_CALLBACK(fScrollCB), (gpointer) g_iAnalogCount);
		g_iAnalogCount++;
	}

	if(GTK_IS_TOGGLE_BUTTON(pWidget))
	{
		g_signal_connect(G_OBJECT(pWidget), "toggled", G_CALLBACK(fSwitchCB), (void*)g_iButtonCount);
		g_iButtonCount++;
	}
}

gboolean fIdleApplicationLoop(gpointer pUserData)
{
	g_pButtonServer->mainloop();
	g_pAnalogServer->report_changes();
	g_pAnalogServer->mainloop();
	g_pConnection->mainloop();
	return TRUE;
}

#endif // TARGET_HAS_ThirdPartyVRPN

int main(int argc, char ** argv)
{
#if defined TARGET_HAS_ThirdPartyVRPN
	const int l_nChannels = 8;

	gtk_init(&argc, &argv);
	// g_pConnection=new ::vrpn_Connection;
	g_pConnection=vrpn_create_server_connection();
	g_pButtonServer=new ::vrpn_Button_Server(_vrpn_peripheral_name_, g_pConnection, l_nChannels);
	g_pAnalogServer=new ::vrpn_Analog_Server(_vrpn_peripheral_name_, g_pConnection, l_nChannels);

	::GtkBuilder* l_pInterface=gtk_builder_new(); // glade_xml_new(OpenViBE::Directories::getDataDir() + "/applications/vrpn-simulator/interface.ui", "window", NULL);
	const OpenViBE::CString l_sFilename = OpenViBE::Directories::getDataDir() + "/applications/vrpn-simulator/interface.ui";
	if(!gtk_builder_add_from_file(l_pInterface, l_sFilename, NULL)) {
		std::cout << "Problem loading [" << l_sFilename << "]\n";
		return -1;
	}

	::GtkWidget* l_pMainWindow=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "window"));
	::GtkWidget* l_pHBoxButton=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "hbox_button"));
	::GtkWidget* l_pHBoxAnalog=GTK_WIDGET(gtk_builder_get_object(l_pInterface, "hbox_analog"));

	g_signal_connect(G_OBJECT(l_pMainWindow), "destroy", gtk_main_quit, NULL);
	gtk_container_foreach(GTK_CONTAINER(l_pHBoxButton), fConnectCB, NULL);
	gtk_container_foreach(GTK_CONTAINER(l_pHBoxAnalog), fConnectCB, NULL);
	gtk_builder_connect_signals(l_pInterface, NULL);

	std::cout << "VRPN Stimulator\n";
	std::cout << "Got " << g_iAnalogCount << " analogs...\n";
	std::cout << "Got " << g_iButtonCount << " buttons...\n";
	std::cout << "Using " << l_nChannels << " VRPN channels...\n";
	std::cout << "Signals will be sent to peripheral [" << _vrpn_peripheral_name_ << "]\n";

	g_idle_add(fIdleApplicationLoop, NULL);

	gtk_widget_show(l_pMainWindow);
	gtk_main();

	delete g_pAnalogServer;
	delete g_pButtonServer;
	delete g_pConnection;

#endif // TARGET_HAS_ThirdPartyVRPN
	return 0;
}

