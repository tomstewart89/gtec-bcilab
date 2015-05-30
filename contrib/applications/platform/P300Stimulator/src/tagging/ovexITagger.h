#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ITagger_H__
#define __ITagger_H__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>


namespace OpenViBEApplications
{
	class ITagger
	{
	public :
		ITagger(unsigned short portNumber, OpenViBE::uint32 sampleFrequency);
		ITagger(){};
		virtual ~ITagger(){};

		virtual int open()=0;
		virtual int write(OpenViBE::uint32 value)=0;
	protected:
		virtual int close()=0;
	};
}
#endif

#endif
