#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __WordPredictionInterface_H__
#define __WordPredictionInterface_H__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <iostream>
#include <vector>
#include <string>

#include "../ova_defines.h"

namespace OpenViBEApplications
{	
	/**
	 * Interface for get the most probable words given a prefix
	 */
	class WordPredictionInterface
	{	
	public:
		virtual ~WordPredictionInterface(){};
		/**
		 * @param prefix the letters or words previously spelled on which we need to base our predictions for the next word or the word to complete
		 * @param nWords number of words we want to get back in the result vector
		 * @return vector of strings with the nWords most probable words given the prefix
		 */
		virtual std::vector<std::string>* getMostProbableWords(const std::string&  prefix, OpenViBE::uint32 nWords)=0;
	};
};
#endif

#endif
