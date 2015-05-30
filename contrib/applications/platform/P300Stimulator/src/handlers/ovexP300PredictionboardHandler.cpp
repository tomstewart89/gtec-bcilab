#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include <vector>

#include "ovexP300PredictionboardHandler.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "ovexUndoHandler.h"
#include "ovexP300ResultAreaHandler.h"
#include "ovexP300KeyboardHandler.h"
#include "../visualisation/glGButton.h"
#include "../visualisation/glGSymbol.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

P300PredictionboardHandler::P300PredictionboardHandler(GContainer* container, std::string& resultBuffer, OpenViBE::CString nGramDatabaseName) 
: m_pSymbolContainer(container), m_sSpelledLetters(resultBuffer)
{
	#if defined TARGET_HAS_ThirdPartyPresage
		m_pWordPredictionEngine = new PresagePredictionEngine(nGramDatabaseName);
		std::cout << "Creating presage prediction engine\n";
	#else
		m_pWordPredictionEngine = new WordPredictionEngine();
	#endif
	GTable* l_pTable = dynamic_cast<GTable*>(m_pSymbolContainer);
	if (l_pTable!=NULL)
		m_ui32NPredictions = l_pTable->getRowDimension()*l_pTable->getColumnDimension();
	else
		m_ui32NPredictions = 10;
}

P300PredictionboardHandler::~P300PredictionboardHandler()
{
	delete m_pWordPredictionEngine;
}
		

void P300PredictionboardHandler::update(GObservable* observable, const void * pUserData) 
{	
	std::vector<std::string>* l_lWords = m_pWordPredictionEngine->getMostProbableWords(m_sSpelledLetters, m_ui32NPredictions);
	this->notifyObservers(l_lWords);	
}
#endif

#endif
