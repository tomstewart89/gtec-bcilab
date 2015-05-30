#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300RowColumnSequenceGenerator.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

vector< vector<uint32>* >* P300RowColumnSequenceGenerator::generateSequence()
{
	P300SequenceGenerator::generateSequence();
	
	if(m_ui64TrialIndex==0)
		P300RipRandSequenceGenerator::generateRipRandBaseMatrix();
	//std::cout << "Rip rand size " << m_vRipRandBaseMatrix.size() << "," << m_vRipRandBaseMatrix[0].size() << "\n";
	//uint32 l_ui32NumberOfRipRepetitions = m_ui32MaxNumberOfRepetitions/m_ui32Passes; //should always be integer by construction
	
	//permute RipRand groups
	uint32 ri=0;
	//boolean l_bBreak = false;
	while(ri<m_ui32MaxNumberOfRepetitions/2)
	{	
		//construct permutation matrix
		vector<uint32> l_vSymbolIndices;
		vector<uint32> l_vPermutation;
		for (uint32 i=0; i<2*m_ui32NumberOfGroups; i++)
			l_vSymbolIndices.push_back(i);
		//std::cout << "Row column permutation\n";
		for (uint32 i=0; i<m_ui32NumberOfGroups-1; i++)
		{
			uint32 l_ui32IndexRow = (rand() % (m_ui32NumberOfGroups-i));
			l_vPermutation.push_back(l_vSymbolIndices[l_ui32IndexRow]);
			uint32 l_ui32IndexColumn = (rand() % (m_ui32NumberOfGroups-i))+m_ui32NumberOfGroups-i;
			l_vPermutation.push_back(l_vSymbolIndices[l_ui32IndexColumn]);
			//std::cout << l_vSymbolIndices[l_ui32IndexRow] << ", " << l_vSymbolIndices[l_ui32IndexColumn] << ", ";
			l_vSymbolIndices.erase(l_vSymbolIndices.begin()+l_ui32IndexRow);
			l_vSymbolIndices.erase(l_vSymbolIndices.begin()+l_ui32IndexColumn-1);
		}
		l_vPermutation.push_back(l_vSymbolIndices[0]);
		l_vPermutation.push_back(l_vSymbolIndices[1]);
		//std::cout << l_vSymbolIndices[0] << ", " << l_vSymbolIndices[1] << "\n";
		
		for (uint32 gi=0; gi<2*m_ui32NumberOfGroups; gi++)
		{
			m_lSequence->push_back(new vector<uint32>());
			for(uint32 eli=0; eli<m_vRipRandBaseMatrix[gi].size(); eli++)
			{
				//std::cout << m_vRipRandBaseMatrix[ l_vPermutation[gi] ][eli] << " ";
				m_lSequence->back()->push_back(m_vRipRandBaseMatrix[ l_vPermutation[gi] ][eli]);
			}
			//std::cout << "\n";
		}
		ri++;
	}
	
	if(m_wSequenceWriter!=NULL)	
		m_wSequenceWriter->writeSequence(m_ui64TrialIndex,m_lSequence);	
	
	return m_lSequence;
}
#endif
