
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepICommandVRPNAnalog.h"
#include "ovassvepCApplication.h"

#include <vrpn_Connection.h>
#include <vrpn_Analog.h>

using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;


namespace
{
	void VRPN_CALLBACK ssvep_vrpn_callback_analog(void *command, vrpn_ANALOGCB analog)
	{
		((OpenViBESSVEP::ICommandVRPNAnalog*)command)->execute(analog.num_channel, analog.channel);
	}
}


ICommandVRPNAnalog::ICommandVRPNAnalog(CApplication* poApplication, OpenViBE::CString sName)
	: ICommand( poApplication )
{
	char l_sAnalogName[1024];

	OpenViBE::Kernel::IConfigurationManager* l_poConfigurationManager = poApplication->getConfigurationManager();

	sprintf(l_sAnalogName, "%s@%s", sName.toASCIIString(), (l_poConfigurationManager->expand("${SSVEP_VRPNHost}")).toASCIIString());

	m_poApplication->getLogManager() << LogLevel_Debug << "+ m_poVRPNAnalog = new vrpn_Analog_Remote(" << OpenViBE::CString(l_sAnalogName) << ")\n";

	
	m_poVRPNAnalog = new vrpn_Analog_Remote( l_sAnalogName );
	m_poVRPNAnalog->register_change_handler( (void*)this, ssvep_vrpn_callback_analog);
}

ICommandVRPNAnalog::~ICommandVRPNAnalog()
{
	m_poApplication->getLogManager() << LogLevel_Debug << "- delete m_poVRPNAnalog\n";
	delete m_poVRPNAnalog;
}

void ICommandVRPNAnalog::processFrame()
{
	m_poVRPNAnalog->mainloop();
}

#endif