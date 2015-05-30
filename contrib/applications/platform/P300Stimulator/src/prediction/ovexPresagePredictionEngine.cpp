#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifdef TARGET_HAS_ThirdPartyPresage

#include "ovexPresagePredictionEngine.h"
#include <boost/algorithm/string.hpp>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

		
PresagePredictionEngine::PresagePredictionEngine(CString nGramDatabaseName) 
{
	m_lPredictedWords = new std::vector<std::string>();
	#ifdef TARGET_OS_Linux
	m_sSpellerContext = std::string("");
	m_pPresageCallback = new MyPresageCallback(m_sSpellerContext);
	CString l_sPath(OpenViBE::Directories::getDistRootDir() + "/share/openvibe/applications/CoAdaptP300Stimulator/presage.xml");
	m_pPresageEngine = new Presage(m_pPresageCallback,l_sPath.toASCIIString());
	m_pPresageEngine->config("Presage.Predictors.DefaultSmoothedNgramPredictor.DBFILENAME",nGramDatabaseName.toASCIIString());
	#endif
	#ifdef TARGET_OS_Windows
	m_sSpellerContext = new char[1000];
	const char* FUTURE = "";
	if (PRESAGE_OK == 
		presage_new_with_config (get_past_stream, (void *) PresagePredictionEngine::m_sSpellerContext, get_future_stream, (void *) FUTURE, 
		(OpenViBE::Directories::getDistRootDir() + "/share/openvibe/applications/CoAdaptP300Stimulator/presage.xml").toASCIIString(), &m_pPresageEngine))
	{
		presage_config_set(m_pPresageEngine, "Presage.Predictors.DefaultSmoothedNgramPredictor.DBFILENAME", nGramDatabaseName.toASCIIString());	
	}
	else
		std::cout << "Failed to initialize presage\n";
	#endif
}

PresagePredictionEngine::~PresagePredictionEngine()
{
	delete m_lPredictedWords;
	#ifdef TARGET_OS_Linux
	delete m_pPresageCallback;
	delete m_pPresageEngine;
	#endif
	#ifdef TARGET_OS_Windows
	presage_free(m_pPresageEngine);
	delete[] m_sSpellerContext;
	#endif
}

std::vector<std::string>* PresagePredictionEngine::getMostProbableWords(const std::string&  prefix, uint32 nWords)
{	
	#ifdef TARGET_OS_Windows
	std::strcpy(m_sSpellerContext, prefix.c_str());
	char**         prediction;
	size_t         i;

	if (PRESAGE_OK == presage_predict (m_pPresageEngine, &prediction))
	{
		m_lPredictedWords->clear();
		for (i = 0; prediction[i] != 0; i++)
		{
			m_lPredictedWords->push_back(std::string(prediction[i]));
			//printf ("prediction[%d]: %s\n", i, prediction[i]);
		}
		presage_free_string_array (prediction);
	}	
	#endif
	#ifdef TARGET_OS_Linux
	m_sSpellerContext = prefix;
	*m_lPredictedWords = m_pPresageEngine->predict();
	#endif
	
	for (uint32 i=0; i<m_lPredictedWords->size(); i++)
	{
		boost::to_upper(m_lPredictedWords->at(i));
		m_lPredictedWords->at(i) += std::string(" ");
	}

	uint32 l_ui32Size = m_lPredictedWords->size();
	if (l_ui32Size<nWords)
	{
		for(uint32 i=0; i<nWords-l_ui32Size; i++)
			m_lPredictedWords->push_back(std::string(""));
	}
	else if (l_ui32Size>nWords)
	{
		for(uint32 i=l_ui32Size; i>nWords; i--)
			m_lPredictedWords->pop_back();
	}

	return m_lPredictedWords;
}

#endif

#endif
