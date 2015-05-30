
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCCommandImpactCustomTargetControl.h"
#include "ovassvepCImpactApplication.h"

using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;

CCommandImpactCustomTargetControl::CCommandImpactCustomTargetControl(CImpactApplication* poApplication)
	: ICommandVRPNButton(poApplication, "SSVEP_VRPN_TargetControl")
{
	m_poVRPNServer = CVRPNServer::getInstance( poApplication );
	m_poVRPNServer->addButton("SSVEP_VRPN_TargetRequest", 1);
}

void CCommandImpactCustomTargetControl::processFrame()
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

/// Add enemies on specified positions depending on the vrpn button triggered
void CCommandImpactCustomTargetControl::execute(int iButton, int iState)
{
//	CImpactApplication* l_poImpactApplication = dynamic_cast<CImpactApplication*>(m_poApplication);
	m_poApplication->getLogManager() << LogLevel_Info << "Adding target, position : " << iButton << "\n";

	CImpactApplication* l_poImpactApplication = dynamic_cast<CImpactApplication*>(m_poApplication);

	l_poImpactApplication->m_rNextOrigin =  0.0;
	if (iButton == 0)
	{
		l_poImpactApplication->m_rNextOrigin = -0.21f;
		l_poImpactApplication->addTarget(1);
		l_poImpactApplication->addTarget(6);
		l_poImpactApplication->addTarget(4);
	}
	else if (iButton == 1)
	{
		l_poImpactApplication->m_rNextOrigin = 0.21f;
		l_poImpactApplication->addTarget(6);
		l_poImpactApplication->addTarget(1);
		l_poImpactApplication->addTarget(3);
	}
}

#endif