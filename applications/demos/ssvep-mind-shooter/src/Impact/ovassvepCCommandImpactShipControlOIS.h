/// Control ship movements with the keyboard

#ifndef __OpenViBEApplication_CCommandImpactShipControlOIS_H__
#define __OpenViBEApplication_CCommandImpactShipControlOIS_H__

#include <map>

#include "../ovassvepICommandOIS.h"

namespace OpenViBESSVEP
{
	class CImpactApplication;

	class CCommandImpactShipControlOIS : public ICommandOIS
	{
	public:
		CCommandImpactShipControlOIS(CImpactApplication* poApplication);
		~CCommandImpactShipControlOIS() {}

		void processFrame();

		void receiveKeyPressedEvent( const OIS::KeyCode oKey );
		void receiveKeyReleasedEvent( const OIS::KeyCode oKey );
	private:
		std::map< OIS::KeyCode, bool > m_vKeyPressed;




	};
}


#endif // __OpenViBEApplication_CCommandImpactShipControlOIS_H__
