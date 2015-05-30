#include "ovtkCAlgorithmClassifier.h"
#include "ovtkCFeatureVector.hpp"
#include "ovtkCFeatureVectorSet.hpp"
#include "ovtkCVector.hpp"

#include <xml/IXMLHandler.h>
#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEToolkit;

boolean CAlgorithmClassifier::initialize()
{
	m_pAlgorithmProxy = NULL;
	m_pExtraParameter = NULL;
	return true;
}

boolean CAlgorithmClassifier::uninitialize()
{
	if(m_pAlgorithmProxy != NULL)
	{
		m_pAlgorithmProxy->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pAlgorithmProxy);
		m_pAlgorithmProxy = NULL;
	}
	return true;
}

boolean CAlgorithmClassifier::process(void)
{
	TParameterHandler < IMatrix* > ip_pFeatureVector(this->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_FeatureVector));
	TParameterHandler < XML::IXMLNode* > ip_pConfiguration(this->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_Configuration));
	TParameterHandler < float64 > op_pClass(this->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Class));
	TParameterHandler < IMatrix* > op_pClassificationValues(this->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_ClassificationValues));
	TParameterHandler < IMatrix* > op_pProbabilityValues(this->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_ProbabilityValues));

	TParameterHandler < IMatrix* > ip_pFeatureVectorSet(this->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_FeatureVectorSet));
	TParameterHandler < XML::IXMLNode* > op_pConfiguration(this->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Configuration));

	if(this->isInputTriggerActive(OVTK_Algorithm_Classifier_InputTriggerId_Train))
	{
		IMatrix* l_pFeatureVectorSet=ip_pFeatureVectorSet;
		if(!l_pFeatureVectorSet)
		{
			this->getLogManager() << LogLevel_ImportantWarning << "Feature vector set matrix is NULL\n";
			this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Failed, true);
		}
		else
		{
			CFeatureVectorSet l_oFeatureVectorSetAdapter(*l_pFeatureVectorSet);
			if(this->train(l_oFeatureVectorSetAdapter))
			{
				this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Success, true);
			}
			else
			{
				this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Failed, true);
				return false;
			}
		}
	}

	if(this->isInputTriggerActive(OVTK_Algorithm_Classifier_InputTriggerId_Classify))
	{
		IMatrix* l_pFeatureVector=ip_pFeatureVector;
		float64 l_f64Class=0;
		IMatrix* l_pClassificationValues=op_pClassificationValues;
		IMatrix* l_pProbabilityValues=op_pProbabilityValues;

		if(!l_pFeatureVector || !l_pClassificationValues)
		{
			this->getLogManager() << LogLevel_ImportantWarning << "Either feature vector matrix is NULL or classification values matrix is NULL\n";
			this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Failed, true);
		}
		else
		{
			CFeatureVector l_oFeatureVectorAdapter(*l_pFeatureVector);
			CVector l_oClassificationValuesAdapter(*l_pClassificationValues);
			CVector l_oProbabilityValuesAdapter(*l_pProbabilityValues);

			if(this->classify(l_oFeatureVectorAdapter, l_f64Class, l_oClassificationValuesAdapter, l_oProbabilityValuesAdapter))
			{
				op_pClass=l_f64Class;

				/* --------------------------------------------------------------------------
				this->getLogManager() << LogLevel_Trace << "Classified feature vector [ ";
				for(i=0; i<l_oFeatureVectorAdapter.getSize(); i++) this->getLogManager() << l_oFeatureVectorAdapter[i] << " ";
				this->getLogManager() << "] with class " << l_f64Class << " and status [ ";
				for(i=0; i<l_oClassificationValuesAdapter.getSize(); i++) this->getLogManager() << l_oClassificationValuesAdapter[i] << " ";
				this->getLogManager() << "]\n";
				-------------------------------------------------------------------------- */

				this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Success, true);
			}
			else
			{
				this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Failed, true);
				return false;
			}
		}
	}

	if(this->isInputTriggerActive(OVTK_Algorithm_Classifier_InputTriggerId_SaveConfiguration))
	{
		XML::IXMLNode *l_pNode = this->saveConfiguration();
		op_pConfiguration = l_pNode;
		if(l_pNode)
		{
			this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Success, true);
		}
		else
		{
			this->getLogManager() << LogLevel_Error << "Unable to save configuration\n";
			this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Failed, true);
			return false;
		}
	}

	if(this->isInputTriggerActive(OVTK_Algorithm_Classifier_InputTriggerId_LoadConfiguration))
	{
		XML::IXMLNode *l_pNode = ip_pConfiguration;
		if(!l_pNode)
		{
			this->getLogManager() << LogLevel_ImportantWarning << "Configuration XML node is NULL\n";
			this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Failed, true);
			return false;
		}
		else
		{

			if(this->loadConfiguration(l_pNode))
			{
				this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Success, true);
			}
			else
			{
				this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Failed, true);
				return false;
			}
		}
	}

	return true;
}

boolean CAlgorithmClassifier::initializeExtraParameterMechanism()
{
	TParameterHandler < std::map<CString, CString>* > ip_pExtraParameter(this->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_ExtraParameter));
	m_pExtraParameter = (std::map<CString, CString>*) ip_pExtraParameter;

	m_pAlgorithmProxy = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(this->getClassIdentifier()));
	m_pAlgorithmProxy->initialize();
	return true;
}

boolean CAlgorithmClassifier::uninitializeExtraParameterMechanism()
{
	m_pAlgorithmProxy->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pAlgorithmProxy);

	m_pAlgorithmProxy = NULL;
	m_pExtraParameter = NULL;
	return true;
}

CString& CAlgorithmClassifier::getParameterValue(const CIdentifier &rParameterIdentifier)
{
	CString l_pParameterName = m_pAlgorithmProxy->getInputParameterName(rParameterIdentifier);
	return (*static_cast<std::map<CString, CString>* >(m_pExtraParameter))[l_pParameterName];
}

int64 CAlgorithmClassifier::getInt64Parameter(const CIdentifier &rParameterIdentifier)
{
	TParameterHandler < int64 > ip_i64Temp(getInputParameter(rParameterIdentifier));
	ip_i64Temp = this->getAlgorithmContext().getConfigurationManager().expandAsInteger(getParameterValue(rParameterIdentifier));
	return (int64)ip_i64Temp;
}

float64 CAlgorithmClassifier::getFloat64Parameter(const CIdentifier &rParameterIdentifier)
{
	TParameterHandler < float64 > ip_f64Temp(getInputParameter(rParameterIdentifier));
	ip_f64Temp = this->getAlgorithmContext().getConfigurationManager().expandAsFloat(getParameterValue(rParameterIdentifier));
	return (float64)ip_f64Temp;
}

boolean CAlgorithmClassifier::getBooleanParameter(const CIdentifier &rParameterIdentifier)
{
	TParameterHandler < boolean > ip_bTemp(getInputParameter(rParameterIdentifier));
	ip_bTemp = this->getAlgorithmContext().getConfigurationManager().expandAsBoolean(getParameterValue(rParameterIdentifier));
	return (boolean)ip_bTemp;
}

CString *CAlgorithmClassifier::getCStringParameter(const CIdentifier &rParameterIdentifier)
{
	TParameterHandler < CString* > ip_pTemp(getInputParameter(rParameterIdentifier));
	ip_pTemp = &getParameterValue(rParameterIdentifier);
	return (CString*)ip_pTemp;
}

int64 CAlgorithmClassifier::getEnumerationParameter(const CIdentifier &rParameterIdentifier, const CIdentifier &rEnumerationIdentifier)
{
	TParameterHandler < int64 > ip_i64Temp(getInputParameter(rParameterIdentifier));
	ip_i64Temp = this->getTypeManager().getEnumerationEntryValueFromName(rEnumerationIdentifier, getParameterValue(rParameterIdentifier));
	return (int64) ip_i64Temp;
}
