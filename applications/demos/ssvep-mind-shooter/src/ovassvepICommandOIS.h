#ifndef __OpenViBEApplication_ICommandOIS_H__
#define __OpenViBEApplication_ICommandOIS_H__

#include <vector>

#include <OIS.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include "ovassvepICommand.h"

namespace OpenViBESSVEP
{

	class ICommandOIS : public ICommand, OIS::KeyListener, OIS::MouseListener
	{
		private:
			static std::vector<ICommandOIS*> m_oInstances;
			static int m_iInstanceCount;

		protected:
			static OIS::InputManager* m_poInputManager;
			static OIS::Keyboard* m_poKeyboard;
			static OIS::Mouse* m_poMouse;

		public:
			ICommandOIS(CApplication* poApplication);
			virtual ~ICommandOIS();

			virtual void processFrame();

		protected:
			virtual void receiveKeyPressedEvent( const OIS::KeyCode oKey ) = 0;
			virtual void receiveKeyReleasedEvent( const OIS::KeyCode oKey ) = 0;
			virtual void receiveMouseEvent( const OIS::MouseEvent &oEvent ) {}

		private:
			bool keyPressed( const OIS::KeyEvent &oEvent );
			bool keyReleased( const OIS::KeyEvent &oEvent );
			bool mouseMoved( const OIS::MouseEvent &oEvent );
			bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) { return true; }
			bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) { return true; }
	};

}

#endif // __OpenViBEApplication_ICommandOIS_H__
