
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCCommandImpactShipControlOIS.h"
#include "ovassvepCImpactApplication.h"

using namespace OpenViBESSVEP;

CCommandImpactShipControlOIS::CCommandImpactShipControlOIS(CImpactApplication* poApplication)
	: ICommandOIS(poApplication)
{
}

void CCommandImpactShipControlOIS::processFrame()
{
	ICommandOIS::processFrame();

	CImpactApplication* l_poShooterApplication = dynamic_cast<CImpactApplication*>(m_poApplication);

	if (m_vKeyPressed[OIS::KC_UP] || m_vKeyPressed[OIS::KC_DOWN])
	{
		l_poShooterApplication->getShip()->shoot();
	}

	if (m_vKeyPressed[OIS::KC_LEFT])
	{
		l_poShooterApplication->getShip()->move( -6 );
	}

	if (m_vKeyPressed[OIS::KC_RIGHT])
	{
		l_poShooterApplication->getShip()->move( 6 );
	}
}

void CCommandImpactShipControlOIS::receiveKeyPressedEvent( const OIS::KeyCode oKey )
{
	if (oKey == OIS::KC_UP)
	{
		m_vKeyPressed[OIS::KC_UP] = true;
	}

	if (oKey == OIS::KC_DOWN)
	{
		m_vKeyPressed[OIS::KC_DOWN] = true;
	}

	if (oKey == OIS::KC_LEFT)
	{
		m_vKeyPressed[OIS::KC_LEFT] = true;
	}

	if (oKey == OIS::KC_RIGHT)
	{
		m_vKeyPressed[OIS::KC_RIGHT] = true;
	}
}

void CCommandImpactShipControlOIS::receiveKeyReleasedEvent( const OIS::KeyCode oKey )
{
	if (oKey == OIS::KC_UP)
	{
		m_vKeyPressed[OIS::KC_UP] = false;
	}

	if (oKey == OIS::KC_DOWN)
	{
		m_vKeyPressed[OIS::KC_DOWN] = false;
	}

	if (oKey == OIS::KC_LEFT)
	{
		m_vKeyPressed[OIS::KC_LEFT] = false;
	}

	if (oKey == OIS::KC_RIGHT)
	{
		m_vKeyPressed[OIS::KC_RIGHT] = false;
	}}

#endif