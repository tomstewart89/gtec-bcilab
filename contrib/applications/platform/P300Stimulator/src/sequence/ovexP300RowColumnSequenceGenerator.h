#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovCoAdaptP300RowColumnSequenceGenerator__
#define __ovCoAdaptP300RowColumnSequenceGenerator__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "ovexP300RipRandSequenceGenerator.h"

namespace OpenViBEApplications
{			
	class P300RowColumnSequenceGenerator : public P300RipRandSequenceGenerator
	{
	public:
		P300RowColumnSequenceGenerator(OpenViBE::uint32 numberOfSymbols, OpenViBE::uint32 numberOfGroups, OpenViBE::uint32 nrOfRepetitions) 
		: P300RipRandSequenceGenerator(numberOfSymbols,numberOfGroups,nrOfRepetitions)
		{ 	
			if (nrOfRepetitions%2!=0)
			{
				std::cout << "Warning: the number of repetitions should be a multiple of two for row-column, adding one repetition\n";
				nrOfRepetitions++;
			}
			m_ui32MaxNumberOfRepetitions = nrOfRepetitions;	
			updateParameters();
		}
		
		/*virtual ~P300RowColumnSequenceGenerator()
		{
		}*/
		
		virtual void changeNumberOfSymbols(OpenViBE::uint32 numberOfSymbols)
		{
			P300RipRandSequenceGenerator::changeNumberOfSymbols(numberOfSymbols);
		}
		virtual void changeNumberOfGroups(OpenViBE::uint32 numberOfGroups) 
		{
			P300RipRandSequenceGenerator::changeNumberOfGroups(numberOfGroups); 
		}	
		
		virtual std::vector< std::vector<OpenViBE::uint32>* >* generateSequence();

	/*protected:
		void generateRipRandBaseMatrix();*/
		
	protected:
		//static void writeSequenceToConsole(std::vector< std::vector<OpenViBE::uint32> >& v);
		void updateParameters()
		{
			//std::cout << "Row-column update parameters \n";
			P300RipRandSequenceGenerator::updateParameters();
			if(m_ui32NumberOfGroups*m_ui32NumberOfGroups != m_ui32NumberOfSymbols)
			{
				std::cout << "WARNING: the number of symbols should fill a square grid, this number will not result in row-column; or...\n";			
				std::cout << "WARNING: the number of groups should equal the square root of the number of symbols, this number will not result in row-column\n";	
			}
		}
		
	/*protected:
		std::vector< std::vector<OpenViBE::uint32> > m_vRipRandBaseMatrix;*/
		
	/*private:
		OpenViBE::uint32 m_ui32Passes;*/
	};
};
#endif

#endif
