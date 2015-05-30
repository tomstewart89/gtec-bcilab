#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __P300PredictionboardHandler_H__
#define __P300PredictionboardHandler_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "../visualisation/glGContainer.h"
#include "../prediction/ovexWordPredictionEngine.h"
#include "../prediction/ovexPresagePredictionEngine.h"

namespace OpenViBEApplications
{	
	class P300PredictionboardHandler : public GObserver, public GObservable
	{	

	public:
		
		
		/**
		 * @param container the container displaying the predicted words
		 * @param resultBuffer a reference to the string buffer that holds the results as is displayed by the P300ResultAreaHandler 
		 * (so when the P300ResultAreaHandler changes its buffer due to other events such as undo this is automatically reflected in this buffer)
		 * @param nGramDatabaseName the path name that points to the n-gram database
		 */
		P300PredictionboardHandler(GContainer* container, std::string& resultBuffer, OpenViBE::CString nGramDatabaseName);
		
		~P300PredictionboardHandler();
		
		/**
		 * inherited from GObserver. Given that m_pSymbolContainer is a reference to the buffer in the P300ResultAreaHandler
		 * every notification can be handled in the same way independent of which class the observable is from
		 * @param observable could be the result handler, undo handler, TODO backspace handler, redo handler
		 */
		virtual void update(GObservable* observable, const  void * pUserData);
		
	protected:
		/**
		 * The container containing and displaying all the words suggested by the predictive engine
		 */
		GContainer* m_pSymbolContainer;
		
		/**
		 * a reference to the P300ResultAreaHandler string result buffer
		 */
		std::string& m_sSpelledLetters;
		
		/**
		 * The engine for predicting words
		 */
		WordPredictionInterface* m_pWordPredictionEngine;
		
	private:
		OpenViBE::uint32 m_ui32NPredictions;
	};
};
#endif
#endif

#endif
