#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovCoAdaptP300RipRandSequenceGenerator__
#define __ovCoAdaptP300RipRandSequenceGenerator__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "ovexP300SequenceGenerator.h"

namespace OpenViBEApplications
{			
	class P300RipRandSequenceGenerator : public P300SequenceGenerator
	{
	public:
		P300RipRandSequenceGenerator(OpenViBE::uint32 numberOfSymbols, OpenViBE::uint32 numberOfGroups, OpenViBE::uint32 nrOfRepetitions) 
		: P300SequenceGenerator(numberOfSymbols,numberOfGroups,nrOfRepetitions)
		{ 
			updateParameters();		
		}
		
		virtual ~P300RipRandSequenceGenerator()
		{
			
		}
		
		virtual void changeNumberOfSymbols(OpenViBE::uint32 numberOfSymbols)
		{
			m_ui32NumberOfSymbols = numberOfSymbols;
			updateParameters();
		}
		virtual void changeNumberOfGroups(OpenViBE::uint32 numberOfGroups) 
		{
			m_ui32NumberOfGroups = numberOfGroups;
			updateParameters();
		}	
		
		virtual std::vector< std::vector<OpenViBE::uint32>* >* generateSequence();

	protected:
		void generateRipRandBaseMatrix();
		
		void updateParameters()
		{
			//std::cout << "RipRand update parameters\n";
			m_ui32Passes = static_cast<OpenViBE::uint32>(ceil(log2(m_ui32NumberOfSymbols)/log2(m_ui32NumberOfGroups)));
			m_ui32MaxNumberOfRepetitions = std::max(m_ui32MaxNumberOfRepetitions,m_ui32Passes);
			//m_ui32MaxNumberOfRepetitions = m_ui32Passes*ceil((OpenViBE::float32)m_ui32RequestedNumberOfRepetitions/m_ui32Passes);
		}		
	private:
		static void writeSequenceToConsole(std::vector< std::vector<OpenViBE::uint32> >& v);
		
	protected:
		std::vector< std::vector<OpenViBE::uint32> > m_vRipRandBaseMatrix;
		OpenViBE::uint32 m_ui32Passes;
		
	};
};
#endif

#endif
