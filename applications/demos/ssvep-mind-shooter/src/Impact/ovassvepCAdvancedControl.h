/// Controls the ship movement by parsing the feedback levels

#ifndef __OpenViBEApplication_CAdvancedControl_H__
#define __OpenViBEApplication_CAdvancedControl_H__

#include <vector>

namespace OpenViBESSVEP
{
	class CImpactApplication;

	class CAdvancedControl
	{
	public:
		enum ShipCommand { NOTHING, SHOOT, MOVE_LEFT, MOVE_RIGHT };

		CAdvancedControl(CImpactApplication* poApplication);
		~CAdvancedControl() {}

		void processFrame(double fShoot, double fLeft, double fRight);

	private:
		CImpactApplication* m_poApplication;
		int m_vCommandStates[3];

		void commandeerShip(ShipCommand sCommand);



	};
}


#endif // __OpenViBEApplication_CCommandImpactShipAdvancedControl_H__
