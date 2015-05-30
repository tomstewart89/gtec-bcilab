#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __WordPredictionEngine_H__
#define __WordPredictionEngine_H__

#include "ovexWordPredictionInterface.h"

namespace OpenViBEApplications
{	
	/**
	 * A dummy class for word prediction/completion
	 */
	class WordPredictionEngine : public WordPredictionInterface
	{	
	public:
		WordPredictionEngine()
		{
			m_lPredictedWords = new std::vector<std::string>();
		}
		
		~WordPredictionEngine()
		{
			delete m_lPredictedWords;
		}
		
		virtual std::vector<std::string>* getMostProbableWords(const std::string& prefix, OpenViBE::uint32 nWords);
		
	protected:
		std::vector<std::string>* m_lPredictedWords;
	};
};
#endif

#endif
