
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCCommandReceiveTarget.h"
#include "ovassvepCApplication.h"

using namespace OpenViBESSVEP;

CCommandReceiveTarget::CCommandReceiveTarget(CApplication* poApplication)
	: ICommandVRPNButton(poApplication, "SSVEP_VRPN_TargetControl")
{
}

void CCommandReceiveTarget::execute(int iButton, int iState)
{
	dynamic_cast<CApplication*>(m_poApplication)->setTarget(iButton);
}

#endif