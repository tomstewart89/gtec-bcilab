
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCCommandCamera.h"
#include "ovassvepCApplication.h"
#include "ovassvepCVRPNServer.h"

using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;

CCommandCamera::CCommandCamera(CApplication* poApplication)
	: ICommandOIS(poApplication)
{
}

CCommandCamera::~CCommandCamera()
{
}

void CCommandCamera::updateCamera()
{
	Ogre::Vector3 l_vTranslation(0,0,0);
	Ogre::Real l_fTranslationSpeed = 10.0;

	if(m_vKeyPressed[OIS::KC_W])
	{
		l_vTranslation.z -= l_fTranslationSpeed;
	}
	if(m_vKeyPressed[OIS::KC_D])
	{
		l_vTranslation.x += l_fTranslationSpeed;
	}
	if(m_vKeyPressed[OIS::KC_A])
	{
		l_vTranslation.x -= l_fTranslationSpeed;
	}
	if(m_vKeyPressed[OIS::KC_S])
	{
		l_vTranslation.z += l_fTranslationSpeed;
	}



	Ogre::Vector3 l_vTranslateVectorFinal = m_poApplication->getCamera()->getDerivedOrientation() * l_vTranslation;
	m_poApplication->getCameraNode()->translate(l_vTranslateVectorFinal, Ogre::Node::TS_WORLD);
}

void CCommandCamera::processFrame()
{
	ICommandOIS::processFrame();

	this->updateCamera();

	if(m_vKeyPressed[OIS::KC_NUMPAD1])
	{
		m_poApplication->debugAction1();
		m_vKeyPressed[OIS::KC_NUMPAD1] = false;
	}
	if(m_vKeyPressed[OIS::KC_NUMPAD2])
	{
		m_poApplication->debugAction2();
		m_vKeyPressed[OIS::KC_NUMPAD2] = false;
	}
	if(m_vKeyPressed[OIS::KC_NUMPAD3])
	{
		m_poApplication->debugAction3();
		m_vKeyPressed[OIS::KC_NUMPAD3] = false;
	}
	if(m_vKeyPressed[OIS::KC_NUMPAD4])
	{
		m_poApplication->debugAction4();
		m_vKeyPressed[OIS::KC_NUMPAD4] = false;
	}
}

void CCommandCamera::receiveKeyPressedEvent( const OIS::KeyCode oKey )
{
	if (oKey == OIS::KC_LCONTROL)
	{
		m_bCameraMode = true;
	}

	if (oKey == OIS::KC_W)
	{
		m_vKeyPressed[OIS::KC_W] = true;
	}
	if (oKey == OIS::KC_A)
	{
		m_vKeyPressed[OIS::KC_A] = true;
	}
	if (oKey == OIS::KC_S)
	{
		m_vKeyPressed[OIS::KC_S] = true;
	}
	if (oKey == OIS::KC_D)
	{
		m_vKeyPressed[OIS::KC_D] = true;
	}
	if (oKey == OIS::KC_NUMPAD1)
	{
		m_vKeyPressed[OIS::KC_NUMPAD1] = true;
	}
	if (oKey == OIS::KC_NUMPAD2)
	{
		m_vKeyPressed[OIS::KC_NUMPAD2] = true;
	}	
	if (oKey == OIS::KC_NUMPAD3)
	{
		m_vKeyPressed[OIS::KC_NUMPAD3] = true;
	}
	if (oKey == OIS::KC_NUMPAD4)
	{
		m_vKeyPressed[OIS::KC_NUMPAD4] = true;
	}
	if (oKey == OIS::KC_ESCAPE)
	{
		m_poApplication->exit();
	}
}

void CCommandCamera::receiveKeyReleasedEvent( const OIS::KeyCode oKey )
{
	if (oKey == OIS::KC_LCONTROL)
	{
		m_bCameraMode = false;
	}

	if (oKey == OIS::KC_W)
	{
		m_vKeyPressed[OIS::KC_W] = false;
	}
	if (oKey == OIS::KC_A)
	{
		m_vKeyPressed[OIS::KC_A] = false;
	}
	if (oKey == OIS::KC_S)
	{
		m_vKeyPressed[OIS::KC_S] = false;
	}
	if (oKey == OIS::KC_D)
	{
		m_vKeyPressed[OIS::KC_D] = false;
	}
	if (oKey == OIS::KC_NUMPAD1)
	{
		m_vKeyPressed[OIS::KC_NUMPAD1] = false;
	}
	if (oKey == OIS::KC_NUMPAD2)
	{
		m_vKeyPressed[OIS::KC_NUMPAD2] = false;
	}
	if (oKey == OIS::KC_NUMPAD3)
	{
		m_vKeyPressed[OIS::KC_NUMPAD3] = false;
	}
	if (oKey == OIS::KC_NUMPAD4)
	{
		m_vKeyPressed[OIS::KC_NUMPAD4] = false;
	}
}

void CCommandCamera::receiveMouseEvent(const OIS::MouseEvent &arg)
{
	Ogre::Real l_fRotationSpeedMouse = 1.0f;

	std::cout << m_poApplication->getCamera()->getOrientation() << "\n";
	if(m_bCameraMode)
	{
		m_poApplication->getCamera()->yaw(Ogre::Degree(-arg.state.X.rel * l_fRotationSpeedMouse));
		m_poApplication->getCamera()->pitch(Ogre::Degree(-arg.state.Y.rel * l_fRotationSpeedMouse));

	}

}

#endif