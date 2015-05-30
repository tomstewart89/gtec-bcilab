#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

// author: Dieter Devlaminck
// affiliation: INRIA 
// date: 26/01/2013
#if defined TARGET_OS_Linux || (defined TARGET_OS_Windows && defined TARGET_HAS_ThirdPartyInpout)


#ifndef __ParallelPort_H__
#define __ParallelPort_H__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "ovexITagger.h"

#if defined TARGET_OS_Linux
	#include <fcntl.h>
	#include <sys/perm.h>
	#include <sys/io.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/ioctl.h>
	#include <linux/parport.h>
	#include <linux/ppdev.h>
	#include <unistd.h>
	#define PARALLELBASE 0x378
#elif defined TARGET_OS_Windows
#endif



namespace OpenViBEApplications
{
        class ParallelPort : public ITagger
	{
		public :
			ParallelPort(unsigned short portNumber, OpenViBE::uint32 sampleFrequency);
			virtual ~ParallelPort();
			virtual int open();
			virtual int write(OpenViBE::uint32 value);
		protected:
			virtual int close();	
		protected :
			int m_iParallelPortHandle;
			unsigned short m_usPortNumber;
			OpenViBE::boolean m_bOpen;
			OpenViBE::uint32 m_ui32SampleFrequency;
	};
};

#endif
#endif //__ParallelPort_H__

#endif
