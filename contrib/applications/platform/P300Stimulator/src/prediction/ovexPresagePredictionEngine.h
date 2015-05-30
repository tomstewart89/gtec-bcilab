#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifdef TARGET_HAS_ThirdPartyPresage

#ifndef __PresagePredictionEngine_H__
#define __PresagePredictionEngine_H__

#include "ovexWordPredictionInterface.h"
#include "presage.h"

namespace OpenViBEApplications
{	
	#ifdef TARGET_OS_Windows
	/**
	 * On windows we have to use the C interface of the Presage library
	 */
	static const char * get_past_stream(void* arg)
	{
		return (char*)arg;
	}
	static const char * get_future_stream(void* arg)
	{
		return (char*)arg;
	}
	#endif

	#ifdef TARGET_OS_Linux
	/**
	 * Implentation of the PresageCallback
	 */
	class MyPresageCallback : public PresageCallback
	{
	public:
		MyPresageCallback(const std::string& _past_context) : past_context(_past_context) { }

		std::string get_past_stream() const { return past_context; }
		std::string get_future_stream() const { return empty; }

	private:
		const std::string& past_context;
		const std::string empty;

	};	
	#endif
	
	/**
	 * Specific implementation of the WordPredictionInterface using the Presage library for word prediction/completion
	 */
	class PresagePredictionEngine : public WordPredictionInterface
	{	
	public:
		
		/**
		 * Constructor
		 * @param nGramDatabaseName the full path name that points to the n-gram database for Presage
		 */
		PresagePredictionEngine(OpenViBE::CString nGramDatabaseName);
		
		~PresagePredictionEngine();
		
		/**
		 * @param prefix the letters or words previously spelled on which we need to base our predictions for the next word or the word to complete
		 * @param nWords number of words we want to get back in the result vector
		 * @return vector of strings with the nWords most probable words given the prefix
		 */		
		virtual std::vector<std::string>* getMostProbableWords(const std::string& prefix, OpenViBE::uint32 nWords);
		
	protected:
		std::vector<std::string>* m_lPredictedWords;
		
		#ifdef TARGET_OS_Linux
		/**
		 * Contains the previously spelled letters or words on which we base our predictions
		 */
		std::string m_sSpellerContext;
		
		MyPresageCallback* m_pPresageCallback;
		Presage* m_pPresageEngine;	
		#endif
		#ifdef TARGET_OS_Windows
		presage_t m_pPresageEngine;
		
		/**
		 * Contains the previously spelled letters or words on which we base our predictions
		 */		
		char * m_sSpellerContext;
		#endif
	};
};
#endif
#endif
#endif
