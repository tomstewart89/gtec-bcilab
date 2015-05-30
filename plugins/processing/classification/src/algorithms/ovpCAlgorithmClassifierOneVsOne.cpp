
#include "ovpCAlgorithmClassifierOneVsOne.h"
#include "ovpCAlgorithmPairwiseDecision.h"

#include <map>
#include <cmath>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <utility>
#include <iostream>
#include <system/ovCMemory.h>

namespace{
	const char* const c_sTypeNodeName = "OneVsOne";
	const char* const c_sSubClassifierIdentifierNodeName = "SubClassifierIdentifier";
	const char* const c_sPairwiseDecisionName = "PairwiseDecision";
	const char* const c_sAlgorithmIdAttribute = "algorithm-id";
	const char* const c_sSubClassifierCountNodeName = "SubClassifierCount";
	const char* const c_sSubClassifiersNodeName = "SubClassifiers";
	const char* const c_sSubClassifierNodeName = "SubClassifier";

	const char* const c_sFirstClassAtrributeName = "first-class";
	const char* const c_sSecondClassAttributeName = "second-class";

	//This map is used to record the decision strategies available for each algorithm
	std::map<OpenViBE::uint64, OpenViBE::CIdentifier> g_oDecisionMap;
}

extern const char* const c_sClassifierRoot;

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Classification;

using namespace OpenViBEToolkit;


void OpenViBEPlugins::Classification::registerAvailableDecisionEnumeration(const CIdentifier& rAlgorithmIdentifier, CIdentifier pDecision)
{
	g_oDecisionMap[rAlgorithmIdentifier.toUInteger()] = pDecision;
}

CIdentifier OpenViBEPlugins::Classification::getAvailableDecisionEnumeration(const OpenViBE::CIdentifier& rAlgorithmIdentifier)
{
	if(g_oDecisionMap.count(rAlgorithmIdentifier.toUInteger()) == 0)
		return OV_UndefinedIdentifier;
	else
		return g_oDecisionMap[rAlgorithmIdentifier.toUInteger()];
}

boolean CAlgorithmClassifierOneVsOne::initialize()
{
	TParameterHandler < XML::IXMLNode* > op_pConfiguration(this->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Configuration));
	op_pConfiguration=NULL;

	TParameterHandler < uint64 > ip_pPairwise(this->getInputParameter(OVP_Algorithm_OneVsOneStrategy_InputParameterId_DecisionType));
	ip_pPairwise = OV_UndefinedIdentifier.toUInteger();

	m_pDecisionStrategyAlgorithm = NULL;
	m_oPairwiseDecisionIdentifier = OV_UndefinedIdentifier;

	return CAlgorithmPairingStrategy::initialize();
}

boolean CAlgorithmClassifierOneVsOne::uninitialize(void)
{
	if(m_pDecisionStrategyAlgorithm != NULL)
	{
		m_pDecisionStrategyAlgorithm->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pDecisionStrategyAlgorithm);
		m_pDecisionStrategyAlgorithm = NULL;
	}
	while(!m_oSubClassifierDescriptorList.empty())
	{
		IAlgorithmProxy* l_pSubClassifier = m_oSubClassifierDescriptorList.back().m_pSubClassifierProxy;
		l_pSubClassifier->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*l_pSubClassifier);
		this->m_oSubClassifierDescriptorList.pop_back();
	}
	return CAlgorithmPairingStrategy::uninitialize();
}



boolean CAlgorithmClassifierOneVsOne::train(const IFeatureVectorSet& rFeatureVectorSet)
{

	const uint32 l_ui32AmountClass = getClassAmount();
	//Create the decision strategy
	this->initializeExtraParameterMechanism();

	m_oPairwiseDecisionIdentifier=this->getEnumerationParameter(OVP_Algorithm_OneVsOneStrategy_InputParameterId_DecisionType, OVP_TypeId_ClassificationPairwiseStrategy);

	if(m_oPairwiseDecisionIdentifier==OV_UndefinedIdentifier) {
		this->getLogManager() << LogLevel_Error << "Tried to get algorithm id for pairwise decision strategy " << OVP_TypeId_ClassificationPairwiseStrategy << " but failed\n";
		return false;
	}
	
	if(m_pDecisionStrategyAlgorithm != NULL){
		m_pDecisionStrategyAlgorithm->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pDecisionStrategyAlgorithm);
		m_pDecisionStrategyAlgorithm = NULL;
	}
	m_pDecisionStrategyAlgorithm = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(m_oPairwiseDecisionIdentifier));
	m_pDecisionStrategyAlgorithm->initialize();

	TParameterHandler < CIdentifier *> ip_pClassificationAlgorithm(m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_AlgorithmIdentifier));
	ip_pClassificationAlgorithm = &m_oSubClassifierAlgorithmIdentifier;
	m_pDecisionStrategyAlgorithm->process(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Parametrize);

	this->uninitializeExtraParameterMechanism();


	//Calculate the amount of sample for each class
	std::map < float64, size_t > l_vClassLabels;
	for(uint32 i=0; i<rFeatureVectorSet.getFeatureVectorCount(); i++)
	{
		if(!l_vClassLabels.count(rFeatureVectorSet[i].getLabel()))
		{
			l_vClassLabels[rFeatureVectorSet[i].getLabel()] = 0;
		}
		l_vClassLabels[rFeatureVectorSet[i].getLabel()]++;
	}

	if(l_vClassLabels.size() != l_ui32AmountClass)
	{
		this->getLogManager() << LogLevel_Error << "There are samples for " << (uint32)l_vClassLabels.size() << " classes but expected samples for " << l_ui32AmountClass << " classes.\n";
		return false;
	}

	//Now we create the corresponding repartition set
	TParameterHandler<IMatrix*> ip_pRepartitionSet = m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_SetRepartition);
	ip_pRepartitionSet->setDimensionCount(1);
	ip_pRepartitionSet->setDimensionSize(0, l_ui32AmountClass);

	const size_t l_iFeatureVectorSize=rFeatureVectorSet[0].getSize();
	//Now let's train each classifier
	for(size_t l_iFirstClass=1 ; l_iFirstClass <= l_ui32AmountClass; ++l_iFirstClass)
	{
		ip_pRepartitionSet->getBuffer()[l_iFirstClass-1] = l_vClassLabels[static_cast<float64>(l_iFirstClass)];

		for(size_t l_iSecondClass = l_iFirstClass+1 ; l_iSecondClass <= l_ui32AmountClass ; ++l_iSecondClass)
		{
			size_t l_iFeatureCount = l_vClassLabels[(float64)l_iFirstClass] + l_vClassLabels[static_cast<float64>(l_iSecondClass)];

			IAlgorithmProxy* l_pSubClassifier = getSubClassifierDescriptor(l_iFirstClass, l_iSecondClass).m_pSubClassifierProxy;

			TParameterHandler < IMatrix* > ip_pFeatureVectorSet(l_pSubClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_FeatureVectorSet));
			ip_pFeatureVectorSet->setDimensionCount(2);
			ip_pFeatureVectorSet->setDimensionSize(0, l_iFeatureCount);
			ip_pFeatureVectorSet->setDimensionSize(1, l_iFeatureVectorSize+1);

			float64* l_pFeatureVectorSetBuffer=ip_pFeatureVectorSet->getBuffer();
			for(size_t j=0; j<rFeatureVectorSet.getFeatureVectorCount(); j++)
			{
				const float64 l_f64TempClass = rFeatureVectorSet[j].getLabel();
				if(l_f64TempClass == static_cast<float64>(l_iFirstClass) || l_f64TempClass == static_cast<float64>(l_iSecondClass))
				{
					System::Memory::copy(
								l_pFeatureVectorSetBuffer,
								rFeatureVectorSet[j].getBuffer(),
								l_iFeatureVectorSize*sizeof(float64));

					if(static_cast<size_t>(l_f64TempClass) == l_iFirstClass )
					{
						l_pFeatureVectorSetBuffer[l_iFeatureVectorSize]=1;
					}
					else
					{
						l_pFeatureVectorSetBuffer[l_iFeatureVectorSize]=2;
					}
					l_pFeatureVectorSetBuffer+=(l_iFeatureVectorSize+1);
				}
			}
			l_pSubClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_Train);
		}
	}
	return true;
}

boolean CAlgorithmClassifierOneVsOne::classify(const IFeatureVector& rFeatureVector, float64& rf64Class, IVector& rClassificationValues, IVector& rProbabilityValue)
{
	if(!m_pDecisionStrategyAlgorithm) {
		this->getLogManager() << LogLevel_Error << "Called without decision strategy algorithm\n";
		return false;
	}

//	std::cout << "Starting Classification" << std::endl;
	const OpenViBE::uint32 l_ui32AmountClass = this->getClassAmount();
	const uint32 l_ui32FeatureVectorSize=rFeatureVector.getSize();

	TParameterHandler<IMatrix*> ip_pProbabilityMatrix = m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_InputParameter_ProbabilityMatrix);
	IMatrix * l_pProbabilityMatrix = (IMatrix*)ip_pProbabilityMatrix;

	l_pProbabilityMatrix->setDimensionCount(2);
	l_pProbabilityMatrix->setDimensionSize(0, l_ui32AmountClass);
	l_pProbabilityMatrix->setDimensionSize(1, l_ui32AmountClass);

	for(OpenViBE::uint32 i =0; i < l_pProbabilityMatrix->getBufferElementCount() ; ++i)
	{
		l_pProbabilityMatrix->getBuffer()[i] = 0.0;
	}

	//Let's generate the matrix of confidence score
	for(OpenViBE::uint32 i =0; i< l_ui32AmountClass ; ++i)
	{
		for(OpenViBE::uint32 j = i+1 ; j< l_ui32AmountClass ; ++j)
		{
			IAlgorithmProxy * l_pTempProxy = this->getSubClassifierDescriptor(i+1, j+1).m_pSubClassifierProxy;
			TParameterHandler < IMatrix* > ip_pFeatureVector(l_pTempProxy->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_FeatureVector));
			TParameterHandler < IMatrix* > op_pClassificationValues(l_pTempProxy->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_ProbabilityValues));
			ip_pFeatureVector->setDimensionCount(1);
			ip_pFeatureVector->setDimensionSize(0, l_ui32FeatureVectorSize);

			float64* l_pFeatureVectorBuffer=ip_pFeatureVector->getBuffer();
			System::Memory::copy(
						l_pFeatureVectorBuffer,
						rFeatureVector.getBuffer(),
						l_ui32FeatureVectorSize*sizeof(float64));
			l_pTempProxy->process(OVTK_Algorithm_Classifier_InputTriggerId_Classify);

			//We have only probability here
			float64 l_f64Prob = op_pClassificationValues->getBuffer()[0];
			l_pProbabilityMatrix->getBuffer()[i*l_ui32AmountClass + j] = l_f64Prob;
			l_pProbabilityMatrix->getBuffer()[j*l_ui32AmountClass + i] = 1-l_f64Prob;
		}
	}

//	for(size_t i =0 ; i < l_ui32AmountClass ; ++i )
//	{
//		for(size_t j = 0; j < l_ui32AmountClass ; ++j)
//		{
//			std::cout << l_pProbabilityMatrix->getBuffer()[i*l_ui32AmountClass + j] << " " ;
//		}
//		std::cout << std::endl;
//	}
//	std::cout << std::endl;

	//Then ask to the startegy to make the decision
	m_pDecisionStrategyAlgorithm->process(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Compute);

	TParameterHandler<IMatrix*> op_pProbabilityVector = m_pDecisionStrategyAlgorithm->getOutputParameter(OVP_Algorithm_Classifier_OutputParameter_ProbabilityVector);
	float64 l_f64MaxProb = -1;
	int32 l_i32IndexSelectedClass = -1;

	//We just have to take the most relevant now.
	for(uint32 i = 0 ; i < l_ui32AmountClass ; ++i)
	{
		const float64 l_f64TempProb = op_pProbabilityVector->getBuffer()[i];
		if(l_f64TempProb > l_f64MaxProb)
		{
			l_i32IndexSelectedClass = i+1;
			l_f64MaxProb = l_f64TempProb;
		}
	}

	rf64Class = static_cast<float64>(l_i32IndexSelectedClass);
	rClassificationValues.setSize(0);
	rProbabilityValue.setSize(1);
	rProbabilityValue[0]=l_f64MaxProb;
	return true;
}

boolean CAlgorithmClassifierOneVsOne::designArchitecture(const OpenViBE::CIdentifier& rId, OpenViBE::uint32 rClassAmount)
{
	if(!setSubClassifierIdentifier(rId))
	{
		return false;
	}

	//Now let's instantiate all the sub_classifier
	for(size_t l_iFirstClass=1 ; l_iFirstClass <= rClassAmount; ++l_iFirstClass)
	{
		for(size_t l_iSecondClass = l_iFirstClass+1 ; l_iSecondClass <= rClassAmount ; ++l_iSecondClass)
		{
			IAlgorithmProxy* l_pSubClassifier = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(this->m_oSubClassifierAlgorithmIdentifier));
			l_pSubClassifier->initialize();

			//Set a references to the extra parameters input of the pairing strategy
			TParameterHandler< std::map<CString, CString>* > ip_pExtraParameters(l_pSubClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_ExtraParameter));
			ip_pExtraParameters.setReferenceTarget(this->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_ExtraParameter));

			SSubClassifierDescriptor l_pTempDesc = {static_cast<float64>(l_iFirstClass), static_cast<float64>(l_iSecondClass), l_pSubClassifier};
			this->m_oSubClassifierDescriptorList.push_back(l_pTempDesc);
		}
	}
	return true;
}

XML::IXMLNode* CAlgorithmClassifierOneVsOne::getClassifierConfiguration(SSubClassifierDescriptor &rDescriptor)
{
	XML::IXMLNode * l_pRes = XML::createNode(c_sSubClassifierNodeName);

	std::stringstream l_sFirstClass, l_sSecondClass;
	l_sFirstClass << rDescriptor.m_f64FirstClass;
	l_sSecondClass << rDescriptor.m_f64SecondClass;
	l_pRes->addAttribute(c_sFirstClassAtrributeName, l_sFirstClass.str().c_str());
	l_pRes->addAttribute(c_sSecondClassAttributeName, l_sSecondClass.str().c_str());

	TParameterHandler < XML::IXMLNode* > op_pConfiguration(rDescriptor.m_pSubClassifierProxy->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Configuration));
	rDescriptor.m_pSubClassifierProxy->process(OVTK_Algorithm_Classifier_InputTriggerId_SaveConfiguration);
	l_pRes->addChild((XML::IXMLNode*)op_pConfiguration);

	return l_pRes;
}

XML::IXMLNode* CAlgorithmClassifierOneVsOne::getPairwiseDecisionConfiguration()
{
	if(!m_pDecisionStrategyAlgorithm) {
		return NULL;
	}

	XML::IXMLNode *l_pTempNode = XML::createNode(c_sPairwiseDecisionName);

	TParameterHandler < XML::IXMLNode* > op_pConfiguration(m_pDecisionStrategyAlgorithm->getOutputParameter(OVP_Algorithm_Classifier_Pairwise_OutputParameterId_Configuration));
	m_pDecisionStrategyAlgorithm->process(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_SaveConfiguration);
	l_pTempNode->addChild((XML::IXMLNode*)op_pConfiguration);

	l_pTempNode->addAttribute(c_sAlgorithmIdAttribute, m_oPairwiseDecisionIdentifier.toString());

	return l_pTempNode;
}


void CAlgorithmClassifierOneVsOne::generateConfigurationNode(void)
{
	std::stringstream l_sAmountClasses;
	l_sAmountClasses << getClassAmount();
	m_pConfigurationNode = XML::createNode(c_sClassifierRoot);

	XML::IXMLNode *l_pOneVsOneNode = XML::createNode(c_sTypeNodeName);

	XML::IXMLNode *l_pTempNode = XML::createNode(c_sSubClassifierIdentifierNodeName);
	l_pTempNode->addAttribute(c_sAlgorithmIdAttribute,this->m_oSubClassifierAlgorithmIdentifier.toString());
	l_pTempNode->setPCData(this->getTypeManager().getEnumerationEntryNameFromValue(OVTK_TypeId_ClassificationAlgorithm, m_oSubClassifierAlgorithmIdentifier.toUInteger()).toASCIIString());
	l_pOneVsOneNode->addChild(l_pTempNode);

	l_pTempNode = XML::createNode(c_sSubClassifierCountNodeName);
	l_pTempNode->setPCData(l_sAmountClasses.str().c_str());
	l_pOneVsOneNode->addChild(l_pTempNode);

	l_pOneVsOneNode->addChild(this->getPairwiseDecisionConfiguration());

	XML::IXMLNode *l_pSubClassifersNode = XML::createNode(c_sSubClassifiersNodeName);
	for(size_t i = 0; i<m_oSubClassifierDescriptorList.size(); ++i)
	{
		l_pSubClassifersNode->addChild(getClassifierConfiguration(m_oSubClassifierDescriptorList[i]));
	}
	l_pOneVsOneNode->addChild(l_pSubClassifersNode);

	m_pConfigurationNode->addChild(l_pOneVsOneNode);
}

XML::IXMLNode* CAlgorithmClassifierOneVsOne::saveConfiguration(void)
{
	generateConfigurationNode();
	return m_pConfigurationNode;
}

boolean CAlgorithmClassifierOneVsOne::loadConfiguration(XML::IXMLNode *pConfigurationNode)
{
	XML::IXMLNode *l_pOneVsOneNode = pConfigurationNode->getChild(0);

	XML::IXMLNode *l_pTempNode = l_pOneVsOneNode->getChildByName(c_sSubClassifierIdentifierNodeName);

	CIdentifier l_pAlgorithmIdentifier;
	l_pAlgorithmIdentifier.fromString(l_pTempNode->getAttribute(c_sAlgorithmIdAttribute));

	if(m_oSubClassifierAlgorithmIdentifier != l_pAlgorithmIdentifier)
	{
		while(!m_oSubClassifierDescriptorList.empty())
		{
			IAlgorithmProxy* l_pSubClassifier = m_oSubClassifierDescriptorList.back().m_pSubClassifierProxy;
			l_pSubClassifier->uninitialize();
			this->getAlgorithmManager().releaseAlgorithm(*l_pSubClassifier);
			this->m_oSubClassifierDescriptorList.pop_back();
		}
		if(!this->setSubClassifierIdentifier(l_pAlgorithmIdentifier)){
			//if the sub classifier doesn't have comparison function it is an error
			return false;
		}
	}

	l_pTempNode = l_pOneVsOneNode->getChildByName(c_sPairwiseDecisionName);
	CIdentifier l_pPairwiseIdentifier;
	l_pPairwiseIdentifier.fromString(l_pTempNode->getAttribute(c_sAlgorithmIdAttribute));
	if(l_pPairwiseIdentifier != m_oPairwiseDecisionIdentifier)
	{
		if(m_pDecisionStrategyAlgorithm != NULL){
			m_pDecisionStrategyAlgorithm->uninitialize();
			this->getAlgorithmManager().releaseAlgorithm(*m_pDecisionStrategyAlgorithm);
			m_pDecisionStrategyAlgorithm = NULL;
		}
		m_oPairwiseDecisionIdentifier = l_pPairwiseIdentifier;
		m_pDecisionStrategyAlgorithm = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(m_oPairwiseDecisionIdentifier));
		m_pDecisionStrategyAlgorithm->initialize();
	}
	TParameterHandler < XML::IXMLNode* > ip_pConfiguration(m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_Configuration));
	ip_pConfiguration = l_pTempNode->getChild(0);

	TParameterHandler < CIdentifier *> ip_pClassificationAlgorithm(m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_AlgorithmIdentifier));
	ip_pClassificationAlgorithm = &l_pAlgorithmIdentifier;

	m_pDecisionStrategyAlgorithm->process(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_LoadConfiguration);
	m_pDecisionStrategyAlgorithm->process(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Parametrize);

	l_pTempNode = l_pOneVsOneNode->getChildByName(c_sSubClassifierCountNodeName);
	std::stringstream l_sCountData(l_pTempNode->getPCData());
	uint64 l_iAmountClass;
	l_sCountData >> l_iAmountClass;

	while(l_iAmountClass != getClassAmount())
	{
		if(l_iAmountClass < getClassAmount())
		{
			IAlgorithmProxy* l_pSubClassifier = m_oSubClassifierDescriptorList.back().m_pSubClassifierProxy;
			l_pSubClassifier->uninitialize();
			this->getAlgorithmManager().releaseAlgorithm(*l_pSubClassifier);
			this->m_oSubClassifierDescriptorList.pop_back();
		}
		else
		{
			IAlgorithmProxy* l_pSubClassifier = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(this->m_oSubClassifierAlgorithmIdentifier));
			l_pSubClassifier->initialize();

			//Set a references to the extra parameters input of the pairing strategy
			TParameterHandler< std::map<CString, CString>* > ip_pExtraParameters(l_pSubClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_ExtraParameter));
			ip_pExtraParameters.setReferenceTarget(this->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_ExtraParameter));

			SSubClassifierDescriptor l_pTempDesc = {1, 2, l_pSubClassifier};
			this->m_oSubClassifierDescriptorList.push_back(l_pTempDesc);
		}
	}

	return loadSubClassifierConfiguration(l_pOneVsOneNode->getChildByName(c_sSubClassifiersNodeName));
}

boolean CAlgorithmClassifierOneVsOne::loadSubClassifierConfiguration(XML::IXMLNode *pSubClassifiersNode)
{
	for(size_t i = 0; i < pSubClassifiersNode->getChildCount() ; ++i)
	{
		XML::IXMLNode *l_pSubClassifierNode = pSubClassifiersNode->getChild(i);
		IAlgorithmProxy* l_pSubClassifier = m_oSubClassifierDescriptorList[i].m_pSubClassifierProxy;
		TParameterHandler < XML::IXMLNode* > ip_pConfiguration(l_pSubClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_Configuration));
		ip_pConfiguration = l_pSubClassifierNode->getChild(0);
		if(!l_pSubClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_LoadConfiguration))
		{
			this->getLogManager() << LogLevel_Error << "Unable to load the configuration for the sub-classifier " << static_cast<uint64>(i+1) << "\n";
			return false;
		}

		//Now we have to restore classes name
		std::stringstream l_sFirstClass(l_pSubClassifierNode->getAttribute(c_sFirstClassAtrributeName));
		l_sFirstClass >> m_oSubClassifierDescriptorList[i].m_f64FirstClass;
		std::stringstream l_sSecondClass(l_pSubClassifierNode->getAttribute(c_sSecondClassAttributeName));
		l_sSecondClass >> m_oSubClassifierDescriptorList[i].m_f64SecondClass;
	}
	return true;
}

uint32 CAlgorithmClassifierOneVsOne::getClassAmount(void) const
{
	//We use a formula because the list as the reponsability ot the count of subClassifier and by extention of amount of classes
	//The amount of element in the list is: sum [n from 1 to Amount_class] (Amount_class - n)
	//The following computation is the resolution of the equation if amount_class is the unknown value
	uint32 l_ui32DeltaCarre = 1+8*m_oSubClassifierDescriptorList.size();
	return static_cast<uint32>((1+::sqrt(static_cast<double>(l_ui32DeltaCarre)))/2);
}

//The function take int because we don't take the "label" of the class but only the numero of declaration
SSubClassifierDescriptor &CAlgorithmClassifierOneVsOne::getSubClassifierDescriptor(const uint32 ui32FirstClass, const uint32 ui32SecondClass)
{
	uint32 ui32Max, ui32Min;
	if(ui32FirstClass > ui32SecondClass)
	{
		ui32Max = ui32FirstClass;
		ui32Min = ui32SecondClass;
	}
	else
	{
		ui32Max = ui32SecondClass;
		ui32Min = ui32FirstClass;
	}
	// The set of subclassifiers can be see as a triangular matrix like this
	// - (1,2) (1,3) .... (1,n)
	// -   -   (2,3) .... (2,n)
	//...
	// -   -     -        (n-1,n)
	//
	// with n the amount of classes.
	// But in memory the matrix look like this
	// (1,2) | (1,3] | ... | (1,n) | (2,3) | .... | (2,n) | ... | (n-1,n)
	//
	// So the index is composed of two parts, the first is the offset generated by the first class (the lower) and the second is the amount of
	// subclassifiers between the beginning of the line and the subclassifier we want.
	//
	// The ith line of the matrix has [n-1-(n-i-1)] elements, so for the jth line we have
	// [n*(jth-1) - sum x from 1, to (jth-1) x ] subclassifiers before.
	//
	// Then the amount of subclassifiers since the beginning of the line is give by [second_class - first_class - 1]
	const uint32 index = getClassAmount()*(ui32Min-1) - ((ui32Min -1)*ui32Min)/2 + ui32Max -ui32Min -1;
	return this->m_oSubClassifierDescriptorList[index];
}

boolean CAlgorithmClassifierOneVsOne::setSubClassifierIdentifier(const OpenViBE::CIdentifier &rId)
{
	m_oSubClassifierAlgorithmIdentifier = rId;
	m_fAlgorithmComparison = getClassificationComparisonFunction(rId);

	if(m_fAlgorithmComparison == NULL)
	{
		this->getLogManager() << LogLevel_Error << "Cannot find the comparison function for the sub classifier\n";
		return false;
	}
	return true;
}

