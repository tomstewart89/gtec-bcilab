#ifndef __System_ovCTime_H__
#define __System_ovCTime_H__

#include "defines.h"

namespace System
{
	class OV_API Time
	{
	public:

		static System::boolean sleep(const System::uint32 ui32MilliSeconds);
		static System::boolean zsleep(const System::uint64 ui64Seconds); // fixed point 32:32 seconds
		static System::uint32 getTime(void);  // milliseconds
		static System::uint64 zgetTime(void); // fixed point 32:32 seconds

		// Please see openvibe/ovITimeArithmetics.h for conversion routines to/from OpenViBE fixed point time

	private:

		Time(void);
	};
};

#endif // __System_ovCTime_H__
