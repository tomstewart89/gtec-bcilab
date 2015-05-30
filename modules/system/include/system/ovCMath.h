#ifndef __System_ovCMath_H__
#define __System_ovCMath_H__

#include "defines.h"

namespace System
{
	class OV_API Math
	{
	public:

		static System::boolean initializeRandomMachine(const System::uint64 ui64RandomSeed);

		static System::uint8 randomUInteger8(void);
		static System::uint16 randomUInteger16(void);
		static System::uint32 randomUInteger32(void);
		static System::uint64 randomUInteger64(void);

		// returns a value in [0,ui32upperLimit( -- i.e. ui32upperLimit not included in range
		static System::uint32 randomUInteger32WithCeiling(uint32 ui32upperLimit);

		static System::int8 randomSInterger8(void);
		static System::int16 randomSInterger16(void);
		static System::int32 randomSInterger32(void);
		static System::int64 randomSInterger64(void);

		static System::float32 randomFloat32(void);
		static System::float32 randomFloat32BetweenZeroAndOne(void);
		static System::float64 randomFloat64(void);

	private:

		Math(void);
	};
};

#endif // __System_ovCMath_H__
