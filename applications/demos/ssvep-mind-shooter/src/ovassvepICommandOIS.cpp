
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepICommandOIS.h"
#include "ovassvepCApplication.h"

#define SSVEP_NO_MOUSE_CAPTURE

using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;
using namespace OIS;

InputManager* ICommandOIS::m_poInputManager = NULL;
Keyboard* ICommandOIS::m_poKeyboard = NULL;
Mouse* ICommandOIS::m_poMouse = NULL;

int ICommandOIS::m_iInstanceCount = 0;
std::vector<ICommandOIS*> ICommandOIS::m_oInstances;


ICommandOIS::ICommandOIS(CApplication* poApplication)
	: ICommand(poApplication)
{
	if (m_poKeyboard == NULL)
	{
		ParamList l_oParamList;
		std::ostringstream l_oWindowHandleString;
		size_t l_stWindowHandle;


		m_poApplication->getWindow()->getCustomAttribute("WINDOW", &l_stWindowHandle);

		l_oWindowHandleString << l_stWindowHandle;

		l_oParamList.insert(std::make_pair(std::string("WINDOW"), l_oWindowHandleString.str()));
		l_oParamList.insert(std::make_pair(std::string("x11_keyboard_grab"), "false"));

		m_poInputManager = OIS::InputManager::createInputSystem(l_oParamList);

		m_poKeyboard = static_cast<OIS::Keyboard*>(m_poInputManager->createInputObject(OIS::OISKeyboard, true));
		m_poKeyboard->setEventCallback( this );

#ifndef SSVEP_NO_MOUSE_CAPTURE
		m_poMouse = static_cast<OIS::Mouse*>(m_poInputManager->createInputObject(OIS::OISMouse, true));
		m_poMouse->setEventCallback( this );
#endif

	}

	m_iInstanceCount++;
	m_oInstances.push_back( this );
}

ICommandOIS::~ICommandOIS()
{
	--m_iInstanceCount;

	if (m_iInstanceCount == 0)
	{
		if (m_poInputManager != NULL)
		{
			if (m_poKeyboard != NULL)
			{
				m_poApplication->getLogManager() << LogLevel_Debug << "- destroy m_poKeyboard\n";
				m_poInputManager->destroyInputObject( m_poKeyboard );
				m_poKeyboard = NULL;
#ifndef SSVEP_NO_MOUSE_CAPTURE
				m_poApplication->getLogManager() << LogLevel_Debug << "- destroy m_poMouse\n";
				m_poInputManager->destroyInputObject( m_poMouse );
				m_poMouse = NULL;
#endif
			}

			m_poInputManager->destroyInputSystem( m_poInputManager );
		}


	}
}

void ICommandOIS::processFrame()
{
	m_poKeyboard->capture();
#ifndef SSVEP_NO_MOUSE_CAPTURE
	m_poMouse->capture();
#endif
}


bool ICommandOIS::keyPressed( const OIS::KeyEvent &oEvent )
{ 
	for (OpenViBE::uint32 i = 0; i < m_oInstances.size(); i++)
	{
		m_oInstances[i]->receiveKeyPressedEvent( oEvent.key );
	}

	return true;
}

bool ICommandOIS::keyReleased( const OIS::KeyEvent &oEvent )
{
	for (OpenViBE::uint32 i = 0; i < m_oInstances.size(); i++)
	{
		m_oInstances[i]->receiveKeyReleasedEvent( oEvent.key );
	}

	return true;
}		

bool ICommandOIS::mouseMoved( const OIS::MouseEvent &oEvent )
{
#ifndef SSVEP_NO_MOUSE_CAPTURE
	for (OpenViBE::uint32 i = 0; i < m_oInstances.size(); i++)
	{
		m_oInstances[i]->receiveMouseEvent( oEvent );
	}
#endif

	return true;
}

#endif