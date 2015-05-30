#ifndef __System_ovCChrono_H__
#define __System_ovCChrono_H__

#include "defines.h"

namespace System
{
	class OV_API CChrono
	{
	public:

		CChrono(void);
		virtual ~CChrono(void);

		virtual System::boolean reset(System::uint32 ui32StepCount);

		virtual System::boolean stepIn(void);
		virtual System::boolean stepOut(void);

		virtual System::uint64 getTotalStepInDuration(void) const;
		virtual System::uint64 getTotalStepOutDuration(void) const;
		virtual System::uint64 getAverageStepInDuration(void) const;
		virtual System::uint64 getAverageStepOutDuration(void) const;
		virtual System::float64 getStepInPercentage(void) const;
		virtual System::float64 getStepOutPercentage(void) const;

		virtual System::boolean hasNewEstimation(void);

	private:

		System::uint64* m_pStepInTime;
		System::uint64* m_pStepOutTime;
		System::uint32 m_ui32StepCount;
		System::uint32 m_ui32StepIndex;
		System::boolean m_bIsInStep;
		System::boolean m_bHasNewEstimation;

		System::uint64 m_ui64TotalStepInTime;
		System::uint64 m_ui64TotalStepOutTime;
	};
};

#endif // __System_ovCChrono_H__
