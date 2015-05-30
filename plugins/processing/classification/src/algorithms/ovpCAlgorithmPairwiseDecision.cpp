#include "ovpCAlgorithmPairwiseDecision.h"

#include <iostream>
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Classification;

using namespace OpenViBEToolkit;

boolean CAlgorithmPairwiseDecision::process()
{
	if(this->isInputTriggerActive(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Compute))
	{
		TParameterHandler<IMatrix *> ip_pProbabilityMatrix = this->getInputParameter(OVP_Algorithm_Classifier_InputParameter_ProbabilityMatrix);
		TParameterHandler<IMatrix*> op_pProbabilityVector = this->getOutputParameter(OVP_Algorithm_Classifier_OutputParameter_ProbabilityVector);
		return this->compute((IMatrix*)ip_pProbabilityMatrix, (IMatrix*)op_pProbabilityVector);
	}
	else if(this->isInputTriggerActive(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_SaveConfiguration))
	{
		TParameterHandler < XML::IXMLNode* > op_pConfiguration(this->getOutputParameter(OVP_Algorithm_Classifier_Pairwise_OutputParameterId_Configuration));
		XML::IXMLNode* l_pTempNode = this->saveConfiguration();
		if(l_pTempNode != NULL)
		{
			op_pConfiguration = l_pTempNode;
			return true;
		}
		return false;
	}
	else if(this->isInputTriggerActive(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_LoadConfiguration))
	{
		TParameterHandler < XML::IXMLNode* > op_pConfiguration(this->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_Configuration));
		XML::IXMLNode* l_pTempNode = (XML::IXMLNode*)op_pConfiguration;
		return this->loadConfiguration(*l_pTempNode);
	}
	else if(this->isInputTriggerActive(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Parametrize))
	{
		this->parametrize();
	}
	return true;
}
