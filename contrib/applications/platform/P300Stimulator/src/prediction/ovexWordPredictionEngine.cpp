#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexWordPredictionEngine.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

std::vector<std::string>* WordPredictionEngine::getMostProbableWords(const std::string&  prefix, uint32 nWords)
{
	//basic
	m_lPredictedWords->clear();
	size_t l_iSpaceIndex = prefix.rfind(" ");
	std::string l_sSuffix;
	if (l_iSpaceIndex!=std::string::npos)
		l_sSuffix = prefix.substr(l_iSpaceIndex);
	else
		l_sSuffix = prefix;
	for (uint32 i=0; i<nWords; i++)
	{
		m_lPredictedWords->push_back(l_sSuffix+std::string(" "));
	}
	
	return m_lPredictedWords;
}
#endif
