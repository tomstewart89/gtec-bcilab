
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCCommandImpactTargetControl.h"
#include "ovassvepCImpactApplication.h"

using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;

CCommandImpactTargetControl::CCommandImpactTargetControl(CImpactApplication* poApplication)
	: ICommandVRPNButton(poApplication, "SSVEP_VRPN_TargetControl")
{
	m_poVRPNServer = CVRPNServer::getInstance( poApplication );
	m_poVRPNServer->addButton("SSVEP_VRPN_TargetRequest", 1);
}

void CCommandImpactTargetControl::processFrame()
{
	CImpactApplication* l_poImpactApplication = dynamic_cast<CImpactApplication*>(m_poApplication);

	ICommandVRPNButton::processFrame();

	if (l_poImpactApplication->m_bTargetRequest)
	{
		m_poApplication->getLogManager() << LogLevel_Info << "Requesting target\n";
		m_poVRPNServer->changeButtonState("SSVEP_VRPN_TargetRequest", 0, 1);
		l_poImpactApplication->m_bTargetRequest = false;
	}
	else
	{
		m_poVRPNServer->changeButtonState("SSVEP_VRPN_TargetRequest", 0, 0);
	}

	m_poVRPNServer->processFrame();
}

void CCommandImpactTargetControl::execute(int iButton, int iState)
{
//	CImpactApplication* l_poImpactApplication = dynamic_cast<CImpactApplication*>(m_poApplication);
	m_poApplication->getLogManager() << LogLevel_Info << "Adding target, position : " << iButton << "\n";
	dynamic_cast<CImpactApplication*>(m_poApplication)->addTarget(iButton);
}

#endif