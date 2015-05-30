#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovCoAdaptP300SequenceFileWriter__
#define __ovCoAdaptP300SequenceFileWriter__

#include <sstream>
#include <fstream>

#include "ovexP300SequenceWriter.h"

namespace OpenViBEApplications
{			
	class P300SequenceFileWriter : public P300SequenceWriter
	{
	public:
		P300SequenceFileWriter(OpenViBE::CString fileName) 
		: P300SequenceWriter(fileName)
		{		
		}
		
		virtual void writeSequence(OpenViBE::uint64 trialIndex, std::vector< std::vector<OpenViBE::uint32>* >* l_lSequence);
	
	protected:
		
	};
};

#endif

#endif
