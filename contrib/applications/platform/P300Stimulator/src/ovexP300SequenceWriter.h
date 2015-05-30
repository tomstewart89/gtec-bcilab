#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovCoAdaptP300SequenceWriter__
#define __ovCoAdaptP300SequenceWriter__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <iostream>
//#include <list>
#include <vector>

namespace OpenViBEApplications
{			
	class P300SequenceWriter
	{
	public:
		P300SequenceWriter(OpenViBE::CString fileName) 
		: m_sFileName(fileName)
		{
		}
		
		virtual ~P300SequenceWriter()
		{
		}
		
		virtual void writeSequence(OpenViBE::uint64 trialIndex, std::vector< std::vector<OpenViBE::uint32>* >* l_lSequence) = 0;

		virtual OpenViBE::CString getFilename(){return m_sFileName;}
		
	protected:
		OpenViBE::CString m_sFileName;
		
	};
};
#endif

#endif
