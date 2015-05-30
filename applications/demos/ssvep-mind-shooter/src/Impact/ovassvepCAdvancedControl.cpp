
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCAdvancedControl.h"
#include "ovassvepCImpactApplication.h"

using namespace OpenViBESSVEP;

CAdvancedControl::CAdvancedControl(CImpactApplication* poApplication) :
	m_poApplication( poApplication )
{
	m_vCommandStates[0] = 0;
	m_vCommandStates[1] = 0;
	m_vCommandStates[2] = 0;
}

const std::string stateToString(CImpactShip::TargetState ts)
{
	if (ts == CImpactShip::TS_NORMAL)
	{
		return "N";
	}
	if (ts == CImpactShip::TS_INHIBITED)
	{
		return "I";
	}
	if (ts == CImpactShip::TS_OFF)
	{
		return "X";
	}

	return "-";
}

void CAdvancedControl::processFrame(double fShoot, double fLeft, double fRight)
{
	ShipCommand l_eCommand = NOTHING;
	int l_iCommandsActivated = 0;

	m_vCommandStates[0] = 0;
	m_vCommandStates[1] = 0;
	m_vCommandStates[2] = 0;


	if (fShoot <= 0.0 && m_poApplication->getShip()->getTargetState(0) != CImpactShip::TS_OFF)
	{
		m_vCommandStates[0] = 1;
		l_iCommandsActivated++;
		l_eCommand = SHOOT;
	}

	if (fLeft <= 0.0 && m_poApplication->getShip()->getTargetState(1) != CImpactShip::TS_OFF)
	{
		m_vCommandStates[1] = 1;
		l_iCommandsActivated++;
		l_eCommand = MOVE_LEFT;
	}

	if (fRight <= 0.0 && m_poApplication->getShip()->getTargetState(2) != CImpactShip::TS_OFF)
	{
		m_vCommandStates[2] = 1;
		l_iCommandsActivated++;
		l_eCommand = MOVE_RIGHT;
	}

	if (l_iCommandsActivated > 1)
	{
		l_eCommand = NOTHING;
	}

	/*
	std::cout << "commands " << m_vCommandStates[0] << " " << m_vCommandStates[1] << " " << m_vCommandStates[2] << "\n";
	std::cout << "states " << stateToString(m_poApplication->getShip()->getTargetState(0)) << " "
			  << stateToString(m_poApplication->getShip()->getTargetState(1))
			  << " " << stateToString(m_poApplication->getShip()->getTargetState(2)) << "\n";
			  */

	commandeerShip(l_eCommand);
}

void CAdvancedControl::commandeerShip(CAdvancedControl::ShipCommand sCommand)
{
	CImpactShip* l_poShip = m_poApplication->getShip();

	switch (sCommand)
	{
	case SHOOT:
		l_poShip->shoot();
		break;
	case MOVE_LEFT:
		l_poShip->move( -6 );
		break;
	case MOVE_RIGHT:
		l_poShip->move( 6 );
		break;
	case NOTHING:
		break;
	}
}


#endif