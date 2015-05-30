#define VOTING_DEBUG 0
#include "ovpCAlgorithmPairwiseDecisionVoting.h"

#include <iostream>


#include <xml/IXMLNode.h>
#include <xml/IXMLHandler.h>

namespace{
	const char* const c_sTypeNodeName = "PairwiseDecision_Voting";
}

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Classification;

using namespace OpenViBEToolkit;

boolean CAlgorithmPairwiseDecisionVoting::initialize()
{
	return true;
}

boolean CAlgorithmPairwiseDecisionVoting::uninitialize()
{
	return true;
}

boolean CAlgorithmPairwiseDecisionVoting::parametrize()
{
	TParameterHandler < CIdentifier *> ip_pAlgorithmIdentifier(this->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_AlgorithmIdentifier));
	CIdentifier l_oId = *ip_pAlgorithmIdentifier;
	m_fAlgorithmComparison = getClassificationComparisonFunction(l_oId);

	return true;
}



boolean CAlgorithmPairwiseDecisionVoting::compute(OpenViBE::IMatrix* pSubClassifierMatrix, OpenViBE::IMatrix* pProbabiltyVector)
{
	OpenViBE::uint32 l_ui32AmountClass = pSubClassifierMatrix->getDimensionSize(0);

#if VOTING_DEBUG
	std::cout << pSubClassifierMatrix->getDimensionSize(0) << std::endl;

	for(OpenViBE::uint32 i = 0 ; i< l_ui32AmountClass ; ++i){

		for(OpenViBE::uint32 j = 0 ; j<l_ui32AmountClass ; ++j){
			std::cout << pSubClassifierMatrix->getBuffer()[i*l_ui32AmountClass + j] << " ";
		}
		std::cout << std::endl;
	}
#endif

	float64* l_pMatrixBuffer = pSubClassifierMatrix->getBuffer();
	float64* l_pProbVector = new float64[l_ui32AmountClass];
	uint32 l_pProbVectorSum = 0;
	CMatrix l_oM1, l_oM2;//Temporary matrix used to compare with m_fAlgorithmComparison
	l_oM1.setDimensionCount(1);
	l_oM1.setDimensionSize(0,1);

	l_oM2.setDimensionCount(1);
	l_oM2.setDimensionSize(0,1);


	for(OpenViBE::uint32 l_ui32FirstClass = 0 ; l_ui32FirstClass < l_ui32AmountClass ; ++l_ui32FirstClass)
	{
		uint32 l_pTempSum = 0;
		for(OpenViBE::uint32 l_ui32SecondClass = 0 ; l_ui32SecondClass<l_ui32AmountClass ; ++l_ui32SecondClass)
		{
			if(l_ui32SecondClass != l_ui32FirstClass)
			{
				l_oM1.getBuffer()[0]= l_pMatrixBuffer[l_ui32AmountClass*l_ui32FirstClass + l_ui32SecondClass];
				l_oM2.getBuffer()[0]= l_pMatrixBuffer[l_ui32AmountClass*l_ui32SecondClass + l_ui32FirstClass];
				if((*m_fAlgorithmComparison)(l_oM1, l_oM2 ) < 0)
				{
					++l_pTempSum;
				}
			}
		}
		l_pProbVector[l_ui32FirstClass] = l_pTempSum;
		l_pProbVectorSum += l_pTempSum;
	}

	for(OpenViBE::uint32 i = 0; i<l_ui32AmountClass ; ++i)
	{
		l_pProbVector[i] /= l_pProbVectorSum;
	}

#if VOTING_DEBUG
	for(OpenViBE::uint32 i = 0; i<l_ui32AmountClass ; ++i)
	{
		std::cout << l_pProbVector[i] << " ";
	}
	std::cout << std::endl;
#endif

	pProbabiltyVector->setDimensionCount(1);
	pProbabiltyVector->setDimensionSize(0,l_ui32AmountClass);

	for(OpenViBE::uint32 i = 0 ; i<l_ui32AmountClass ; ++i)
	{
		pProbabiltyVector->getBuffer()[i] = l_pProbVector[i];
	}

	delete[] l_pProbVector;
	return true;
}

XML::IXMLNode* CAlgorithmPairwiseDecisionVoting::saveConfiguration()
{
	XML::IXMLNode* l_pRootNode = XML::createNode(c_sTypeNodeName);
	return l_pRootNode;
}

boolean CAlgorithmPairwiseDecisionVoting::loadConfiguration(XML::IXMLNode& rNode)
{
	return true;
}
