#ifndef __OpenViBEApplication_ICommandVRPNAnalog_H__
#define __OpenViBEApplication_ICommandVRPNAnalog_H__

#include "ovassvepICommand.h"

class vrpn_Analog_Remote;

namespace OpenViBESSVEP
{
	class ICommandVRPNAnalog : public ICommand
	{
		public:
			ICommandVRPNAnalog(CApplication* poApplication, OpenViBE::CString sName);
			
			virtual ~ICommandVRPNAnalog();

			virtual void processFrame();
			virtual void execute(int iChannelCount, double* channel ) = 0;

		protected:
			vrpn_Analog_Remote* m_poVRPNAnalog;


	};


}


#endif // __OpenViBEApplication_ICommandVRPNAnalog_H__
