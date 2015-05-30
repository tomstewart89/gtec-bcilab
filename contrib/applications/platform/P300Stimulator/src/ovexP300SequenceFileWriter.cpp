#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300SequenceFileWriter.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

void P300SequenceFileWriter::writeSequence(uint64 trialIndex, vector< vector<uint32>* >* l_lSequence)
{
	//std::stringstream l_ssIndexFileName(std::stringstream::in | std::stringstream::out);
	//l_ssIndexFileName << m_sFileName << "_" << trialIndex << ".txt";
	std::ofstream l_ofFile(m_sFileName.toASCIIString(),ios_base::app);
	//l_ofFile.seekp(0,ios_base::end);
	//std::cout << "Adding stuff on position " << l_ofFile.tellp() << "\n";
	
	vector< vector<uint32>* >::iterator l_RepetitionIterator;
	vector<uint32>::iterator l_GroupIterator;
	for (l_RepetitionIterator = l_lSequence->begin(); l_RepetitionIterator!=l_lSequence->end(); l_RepetitionIterator++)
	{
		std::stringstream l_ssGroup(std::stringstream::out);
		for (l_GroupIterator = (*l_RepetitionIterator)->begin(); l_GroupIterator!=(*l_RepetitionIterator)->end(); l_GroupIterator++)
			l_ssGroup << *l_GroupIterator << ",";
		std::streamoff l_lEndOfFile = l_ssGroup.tellp();
		l_ssGroup.seekp(l_lEndOfFile-1);
		l_ssGroup.write("\n",1);
		//std::cout << "SequenceWriter: " << l_ssGroup.str();
		l_ofFile.seekp(l_ofFile.tellp());
 		l_ofFile.write(l_ssGroup.str().c_str(), l_ssGroup.tellp());
	}
	l_ofFile.write("\n", 1);
	
	l_ofFile.close();
}
#endif
