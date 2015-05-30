#if defined(TARGET_HAS_ThirdPartyEnobioAPI)

#include "ovasCConfigurationEnobio3G.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>

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
	CConfigurationEnobio3G* l_pConfig=static_cast<CConfigurationEnobio3G*>(pUserData);
	l_pConfig->buttonCalibratePressedCB();
}

//Callback actually called:
void CConfigurationGTecGUSBamp::buttonCalibratePressedCB(void)
{
	// Connect to the hardware, ask for calibration, verify the return code, etc.
}
_________________________________________________*/

// If you added more reference attribute, initialize them here
CConfigurationEnobio3G::CConfigurationEnobio3G(IDriverContext& rDriverContext, const char* sGtkBuilderFileName)
	:CConfigurationBuilder(sGtkBuilderFileName)
	,m_rDriverContext(rDriverContext)
{
	for(int i=0;i<6;i++)
	{
		m_macAddress[i]=0x00;
	}
}

boolean CConfigurationEnobio3G::preConfigure(void)
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

	return true;
}

// function to parse from string to HEX, decimal, etc...
template <class T>
bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}

boolean CConfigurationEnobio3G::postConfigure(void)
{
	if(m_bApplyConfiguration)
	{
		// If the user pressed the "apply" button, you need to save the changes made in the configuration.
		// For example, you can save the connection ID of the selected device:
		// m_ui32ConnectionID = <value-from-gtk-widget>
	}

	::GtkEntry* l_pMacField = GTK_ENTRY(gtk_builder_get_object(m_pBuilderConfigureInterface,"entry_address"));

	std::string l_sMac((char*)gtk_entry_get_text(l_pMacField));

	if(! CConfigurationBuilder::postConfigure()) // normal header is filled (Subject ID, Age, Gender, channels, sampling frequency), ressources are realesed
	{
		return false;
	}

	// from the string typped in the address text field, parse
	// the 6 hex values for the mac address
	// first, tokenize the string using ':' as delimeter
	std::vector<std::string> v;
	std::istringstream buf(l_sMac);
	for(std::string token; getline(buf, token, ':'); )
	{
		v.push_back(token);
	}

	// each token from the string, parse it as HEX values
	int a;
	from_string<int >(a,v[5],std::hex);
	m_macAddress[0] = (unsigned char)a;
	from_string<int >(a,v[4],std::hex);
	m_macAddress[1] = (unsigned char)a;
	from_string<int >(a,v[3],std::hex);
	m_macAddress[2] = (unsigned char)a;
	from_string<int >(a,v[2],std::hex);
	m_macAddress[3] = (unsigned char)a;
	from_string<int >(a,v[1],std::hex);
	m_macAddress[4] = (unsigned char)a;
	from_string<int >(a,v[0],std::hex);
	m_macAddress[5] = (unsigned char)a;



	return true;
}

unsigned char* CConfigurationEnobio3G::getMacAddress(){

  return m_macAddress;
}

#endif