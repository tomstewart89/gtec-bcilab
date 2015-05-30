#ifndef __OpenViBEApplication_CCommandCamera_H__
#define __OpenViBEApplication_CCommandCamera_H__

#include <map>

#include "ovassvepICommandOIS.h"

namespace OpenViBESSVEP
{
	class CVRPNServer;

	class CCommandCamera : public ICommandOIS
	{
		public:
			CCommandCamera(CApplication* poApplication);
			~CCommandCamera();

			void processFrame();

			void receiveKeyPressedEvent( const OIS::KeyCode oKey );
			void receiveKeyReleasedEvent( const OIS::KeyCode oKey );
			void receiveMouseEvent( const OIS::MouseEvent &oEvent );

		private:
			void updateCamera();
			std::map< OIS::KeyCode, bool > m_vKeyPressed;

			bool m_bCameraMode;



	};
}


#endif // __OpenViBEApplication_CCommandCamera_H__
