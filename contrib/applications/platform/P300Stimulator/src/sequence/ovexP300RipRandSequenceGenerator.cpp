#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300RipRandSequenceGenerator.h"

#include <system/ovCTime.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

vector< vector<uint32>* >* P300RipRandSequenceGenerator::generateSequence()
{
	P300SequenceGenerator::generateSequence();
	
	if(m_ui64TrialIndex==0)
		generateRipRandBaseMatrix();
	
	//uint32 l_ui32NumberOfRipRepetitions = m_ui32MaxNumberOfRepetitions/m_ui32Passes; //should always be integer by construction
	srand(static_cast<uint32>(System::Time::zgetTime()));
		
		
	//permute RipRand groups
	uint32 ri=0;
	//boolean l_bBreak = false;
	while(ri<m_ui32MaxNumberOfRepetitions)
	{	
		//construct permutation matrix
		vector<uint32> l_vSymbolIndices;
		vector<uint32> l_vPermutation;
		for (uint32 i=0; i<m_ui32NumberOfSymbols; i++)
			l_vSymbolIndices.push_back(i);
		for (uint32 i=0; i<m_ui32NumberOfSymbols-1; i++)
		{
			uint32 l_ui32Index = (rand() % (m_ui32NumberOfSymbols-i));
			l_vPermutation.push_back(l_vSymbolIndices[l_ui32Index]);
			l_vSymbolIndices.erase(l_vSymbolIndices.begin()+l_ui32Index);
		}
		l_vPermutation.push_back(l_vSymbolIndices[0]);
		
		//for (uint32 gi=0; gi<m_vRipRandBaseMatrix.size(); gi++)
		//{
		for (uint32 pi=0; pi<m_ui32Passes; pi++)
		{
			for (uint32 gi=0; gi<m_ui32NumberOfGroups; gi++)
			{
				m_lSequence->push_back(new vector<uint32>());
				for(uint32 eli=0; eli<m_vRipRandBaseMatrix[pi*m_ui32NumberOfGroups+gi].size(); eli++)
				{
					m_lSequence->back()->push_back(m_vRipRandBaseMatrix[pi*m_ui32NumberOfGroups+gi][ l_vPermutation[eli] ]);
					//std::cout << m_vRipRandBaseMatrix[pi*m_ui32NumberOfGroups+gi][ l_vPermutation[eli] ] << " ";
				}
				//std::cout << "\n";
			}
			ri++;
			if (ri>=m_ui32MaxNumberOfRepetitions && ri>=m_ui32Passes)
			{
				//std::cout << "Reached " << ri << " repetitions, size of structure " << m_lSequence->size() << ", number of groups " << m_ui32NumberOfGroups << ", number of passes " << m_ui32Passes << "\n"; 
				//l_bBreak = true;
				pi=m_ui32Passes;
			}
		}
	}
	
	if(m_wSequenceWriter!=NULL)	
		m_wSequenceWriter->writeSequence(m_ui64TrialIndex,m_lSequence);	
	
	return m_lSequence;
}


void P300RipRandSequenceGenerator::generateRipRandBaseMatrix()
{	
	uint32 l_ui32GroupSize = static_cast<OpenViBE::uint32>(ceil((float32)m_ui32NumberOfSymbols/m_ui32NumberOfGroups));
	
	m_vRipRandBaseMatrix.reserve(m_ui32Passes*m_ui32NumberOfGroups);

	vector< vector<uint32> > l_vTmpSequence(m_ui32NumberOfGroups);
	for (uint32 pi=0; pi<m_ui32Passes; pi++)
	{
		
		for(uint32 gi=0; gi<m_ui32NumberOfGroups; gi++)
		{
			l_vTmpSequence[gi] = vector<uint32>(l_ui32GroupSize*m_ui32NumberOfGroups,0);
			for(uint32 eli=gi*l_ui32GroupSize; eli<(gi+1)*l_ui32GroupSize; eli++)
				l_vTmpSequence[gi].at(eli) = 1;
		}
		uint32 l_ui32NumberOfCopies = static_cast<OpenViBE::uint32>(ceil((float32)m_ui32NumberOfSymbols/(float32)(l_ui32GroupSize*m_ui32NumberOfGroups)));
		for(uint32 gi=0; gi<m_ui32NumberOfGroups; gi++)
		{
			uint32 l_ui32Size = l_vTmpSequence[gi].size();
			for(uint32 ci=1; ci<=l_ui32NumberOfCopies-1; ci++)
				l_vTmpSequence[gi].insert(l_vTmpSequence[gi].end(), 
								  l_vTmpSequence[gi].begin(), l_vTmpSequence[gi].begin()+l_ui32Size);
			l_vTmpSequence[gi].resize(m_ui32NumberOfSymbols);		
		}	
		
		m_vRipRandBaseMatrix.insert(m_vRipRandBaseMatrix.begin()+pi*m_ui32NumberOfGroups, l_vTmpSequence.begin(), l_vTmpSequence.end());
		l_ui32GroupSize = static_cast<OpenViBE::uint32>(ceil((float32)l_ui32GroupSize/m_ui32NumberOfGroups));		
	}
	
	/*std::cout << "RipRAND matrix \n";
	P300RipRandSequenceGenerator::writeSequenceToConsole(m_vRipRandBaseMatrix);*/
}

void  P300RipRandSequenceGenerator::writeSequenceToConsole(vector< vector<uint32> >& v)
{
	for (uint32 i=0; i<v.size(); i++)
	{
		for(uint32 j=0; j<v[i].size(); j++)
			std::cout << " " << v[i][j];
		std::cout << "\n";
	}
}

#endif
