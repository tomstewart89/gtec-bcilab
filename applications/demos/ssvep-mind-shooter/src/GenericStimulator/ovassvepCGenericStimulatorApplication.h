#ifndef __OpenViBEApplication_CGenericStimulatorApplication_H__
#define __OpenViBEApplication_CGenericStimulatorApplication_H__

#include <iostream>
#include <CEGUI.h>

#include "../ovassvepCApplication.h"
#include "ovassvepCGenericStimulatorFlickeringObject.h"

namespace OpenViBESSVEP
{

	class CGenericStimulatorApplication : public CApplication
	{
		public:
			CGenericStimulatorApplication(OpenViBE::CString sScenarioDir);
			~CGenericStimulatorApplication() {}

			bool setup(OpenViBE::Kernel::IKernelContext* poKernelContext);
			void setTarget(OpenViBE::int32 i32Target);

			void startExperiment();
			void startFlickering();
			void stopFlickering();


		private:
			bool m_bActive;
			void processFrame(OpenViBE::uint32 ui32CurrentFrame);
			void addObject(CGenericStimulatorFlickeringObject *target);

			std::vector<CGenericStimulatorFlickeringObject*> m_oObjects;

			time_t m_ttStartTime;

			CEGUI::Window* m_poInstructionsReady;


	};
}


#endif // __OpenViBEApplication_CGenericStimulatorApplication_H__
