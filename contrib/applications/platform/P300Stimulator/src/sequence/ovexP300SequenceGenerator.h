#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovCoAdaptP300SequenceGenerator__
#define __ovCoAdaptP300SequenceGenerator__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <iostream>
#include <cmath>
#include <vector>
//#include <list>
#include <cstdlib>

#include "../ovexP300SequenceWriter.h"

namespace OpenViBEApplications
{			
	class P300SequenceGenerator
	{
	public:
		P300SequenceGenerator(OpenViBE::uint32 numberOfSymbols, OpenViBE::uint32 numberOfGroups, OpenViBE::uint32 nrOfRepetitions) 
		: m_ui32NumberOfSymbols(numberOfSymbols), m_ui32NumberOfGroups(numberOfGroups), m_ui32MaxNumberOfRepetitions(nrOfRepetitions)
		{
			//std::cout << "Sequence generator, number of repetitions " << m_ui32MaxNumberOfRepetitions << "\n";
			m_lSequence = new std::vector< std::vector<OpenViBE::uint32>* >();
			m_wSequenceWriter = NULL;
			m_ui64TrialIndex = -1;
		}	
		
		virtual ~P300SequenceGenerator()
		{
			std::vector< std::vector<OpenViBE::uint32>* >::iterator it = m_lSequence->begin();
			for (; it!=m_lSequence->end(); it++)
				delete *it;
			delete m_lSequence;
		}
		
		virtual std::vector< std::vector<OpenViBE::uint32>* >* generateSequence()
		{
			m_ui64TrialIndex++;
			m_ui32FlashCount = 0;
			
			std::vector< std::vector<OpenViBE::uint32>* >::iterator it = m_lSequence->begin();
			for (; it!=m_lSequence->end(); it++)
				delete *it;
			delete m_lSequence;	
			
			m_lSequence = new std::vector< std::vector<OpenViBE::uint32>* >();
			
			return m_lSequence;		
		}
		void setSequenceWriter(P300SequenceWriter* l_wSequenceWriter)
		{
			m_wSequenceWriter = l_wSequenceWriter;
		}
		
		virtual void changeNumberOfSymbols(OpenViBE::uint32 numberOfSymbols) = 0;
		virtual void changeNumberOfGroups(OpenViBE::uint32 numberOfGroups) = 0;
		
		OpenViBE::uint32 getNumberOfRepetitions() { return m_ui32MaxNumberOfRepetitions; }
		OpenViBE::uint32 getNumberOfSymbols() { return m_ui32NumberOfSymbols; }
		OpenViBE::uint32 getNumberOfGroups() { return m_ui32NumberOfGroups; }
		std::vector<OpenViBE::uint32>* getNextFlashGroup()
		{
			return m_lSequence->at(m_ui32FlashCount++);
		}


		//for evidence accumulator
		std::vector<OpenViBE::uint32>* getFlashGroupAt(OpenViBE::uint32 index)
		{
			return m_lSequence->at(index);
		}
		
		
	protected:
		OpenViBE::float64 round(OpenViBE::float64 x)
		{
			return floor(x+0.5);
		}

		OpenViBE::float64 log2(OpenViBE::float64 x)
		{
			return log(x)/log(2.0);
		}
	
		
	protected:
		OpenViBE::uint32 m_ui32NumberOfSymbols;
		OpenViBE::uint32 m_ui32NumberOfGroups;
		OpenViBE::uint32 m_ui32MaxNumberOfRepetitions;
		//OpenViBE::uint32 m_ui32RequestedNumberOfRepetitions;
		OpenViBE::uint32 m_ui32FlashCount;
		std::vector< std::vector<OpenViBE::uint32>* >* m_lSequence;
		P300SequenceWriter* m_wSequenceWriter;
		OpenViBE::uint64 m_ui64TrialIndex;
	};
};

#endif

#endif
