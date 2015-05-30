#include "ovpCAlgorithmClassifierSVM.h"

#include <map>
#include <sstream>
#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cmath>

#include <xml/IXMLHandler.h>

namespace{
	const char* const c_sTypeNodeName = "SVM";
	const char* const c_sParamNodeName = "Param";
	const char* const c_sSvmTypeNodeName = "svm_type";
	const char* const c_sKernelTypeNodeName = "kernel_type";
	const char* const c_sDegreeNodeName = "degree";
	const char* const c_sGammaNodeName = "gamma";
	const char* const c_sCoef0NodeName = "coef0";
	const char* const c_sModelNodeName = "Model";
	const char* const c_sNrClassNodeName = "nr_class";
	const char* const c_sTotalSvNodeName = "total_sv";
	const char* const c_sRhoNodeName = "rho";
	const char* const c_sLabelNodeName = "label";
	const char* const c_sProbANodeName = "probA";
	const char* const c_sProbBNodeName = "probB";
	const char* const c_sNrSvNodeName = "nr_sv";
	const char* const c_sSvsNodeName = "SVs";
	const char* const c_sSVNodeName = "SV";
	const char* const c_sCoefNodeName = "coef";
	const char* const c_sValueNodeName = "value";
}

extern const char* const c_sClassifierRoot;

OpenViBE::int32 OpenViBEPlugins::Classification::getSVMBestClassification(OpenViBE::IMatrix& rFirstClassificationValue, OpenViBE::IMatrix& rSecondClassificationValue)
{
	if(ov_float_equal(rFirstClassificationValue[0], ::fabs(rSecondClassificationValue[0])))
		return 0;
	else if(::fabs(rFirstClassificationValue[0]) > ::fabs(rSecondClassificationValue[0]))
		return -1;
	else
		return 1;
}

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Classification;

using namespace OpenViBEToolkit;

CAlgorithmClassifierSVM::CAlgorithmClassifierSVM(void)
	:m_pModel(NULL), m_bModelWasTrained(false)
{
}

boolean CAlgorithmClassifierSVM::initialize(void)
{
	TParameterHandler < int64 > ip_i64SVMType(this->getInputParameter(OVP_Algorithm_ClassifierSVM_InputParameterId_SVMType));
	TParameterHandler < int64 > ip_i64SVMKernelType(this->getInputParameter(OVP_Algorithm_ClassifierSVM_InputParameterId_SVMKernelType));
	TParameterHandler < int64 > ip_i64Degree(this->getInputParameter(OVP_Algorithm_ClassifierSVM_InputParameterId_SVMDegree));
	TParameterHandler < float64 > ip_f64Gamma(this->getInputParameter(OVP_Algorithm_ClassifierSVM_InputParameterId_SVMGamma));
	TParameterHandler < float64 > ip_f64Coef0(this->getInputParameter(OVP_Algorithm_ClassifierSVM_InputParameterId_SVMCoef0));
	TParameterHandler < float64 > ip_f64Cost(this->getInputParameter(OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMCost));
	TParameterHandler < float64 > ip_f64Nu(this->getInputParameter(OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMNu));
	TParameterHandler < float64 > ip_f64Epsilon(this->getInputParameter(OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMEpsilon));
	TParameterHandler < float64 > ip_f64CacheSize(this->getInputParameter(OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMCacheSize));
	TParameterHandler < float64 > ip_f64EpsilonTolerance(this->getInputParameter(OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMTolerance));
	TParameterHandler < boolean > ip_bShrinking(this->getInputParameter(OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMShrinking));
	//TParameterHandler < boolean > ip_bProbabilityEstimate(this->getInputParameter(OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMProbabilityEstimate));
	TParameterHandler < CString* > ip_sWeight(this->getInputParameter(OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMweight));
	TParameterHandler < CString* > ip_sWeightLabel(this->getInputParameter(OVP_Algorithm_ClassifierSVM_InputParameterId_SVMWeightLabel));

	ip_i64SVMType=C_SVC;
	ip_i64SVMKernelType=LINEAR;
	ip_i64Degree=3;
	ip_f64Gamma=0;
	ip_f64Coef0=0;
	ip_f64Cost=1;
	ip_f64Nu=0.5;
	ip_f64Epsilon=0.1;
	ip_f64CacheSize=100;
	ip_f64EpsilonTolerance=0.001;
	ip_bShrinking=true;
	//ip_bProbabilityEstimate=true;
	*ip_sWeight="";
	*ip_sWeightLabel="";

	TParameterHandler < XML::IXMLNode* > op_pConfiguration(this->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Configuration));
	op_pConfiguration=NULL;
	m_oProb.y=NULL;
	m_oProb.x=NULL;

	m_oParam.weight = NULL;
	m_oParam.weight_label = NULL;

	m_pModel = NULL;
	m_bModelWasTrained = false;

	return CAlgorithmClassifier::initialize();
}

boolean CAlgorithmClassifierSVM::uninitialize(void)
{
	if(m_oProb.x != NULL && m_oProb.y != NULL)
	{
		for(int i=0;i<m_oProb.l;i++)
		{
			delete[] m_oProb.x[i];
		}
		delete[] m_oProb.y;
		delete[] m_oProb.x;
		m_oProb.y=NULL;
		m_oProb.x=NULL;
	}

	if(m_oParam.weight) 
	{
		delete[] m_oParam.weight;
		m_oParam.weight = NULL;
	}

	if(m_oParam.weight_label) 
	{
		delete[] m_oParam.weight_label;
		m_oParam.weight_label = NULL;
	}

	deleteModel(m_pModel, !m_bModelWasTrained);
	m_pModel = NULL;
	m_bModelWasTrained = false;

	return CAlgorithmClassifier::uninitialize();
}

void CAlgorithmClassifierSVM::deleteModel(svm_model *pModel, bool bFreeSupportVectors)
{
	if(pModel) 
	{
		delete[] pModel->rho;
		delete[] pModel->probA;
		delete[] pModel->probB;
		delete[] pModel->label;
		delete[] pModel->nSV;

		for(int i=0;i<pModel->nr_class-1;i++)
		{
			delete[] pModel->sv_coef[i];
		}
		delete[] pModel->sv_coef;
		
		// We need the following depending on how the model was allocated. If we got it from svm_train,
		// the support vectors are pointers to the problem structure which is freed elsewhere. 
		// If we loaded the model from disk, we allocated the vectors separately.
		if(bFreeSupportVectors)
		{
			for(int i=0;i<pModel->l;i++)
			{
				delete[] pModel->SV[i];
			}
		}
		delete[] pModel->SV;

		delete pModel;
		pModel = NULL;
	}
}

void CAlgorithmClassifierSVM::setParameter(void)
{
	this->initializeExtraParameterMechanism();

	m_oParam.svm_type           = (int)this->getEnumerationParameter(  OVP_Algorithm_ClassifierSVM_InputParameterId_SVMType,       OVP_TypeId_SVMType);
	m_oParam.kernel_type        = (int)this->getEnumerationParameter(  OVP_Algorithm_ClassifierSVM_InputParameterId_SVMKernelType, OVP_TypeId_SVMKernelType);
	m_oParam.degree             = (int)this->getInt64Parameter(        OVP_Algorithm_ClassifierSVM_InputParameterId_SVMDegree);
	m_oParam.gamma              = this->getFloat64Parameter(           OVP_Algorithm_ClassifierSVM_InputParameterId_SVMGamma);
	m_oParam.coef0              = this->getFloat64Parameter(           OVP_Algorithm_ClassifierSVM_InputParameterId_SVMCoef0);
	m_oParam.C                  = this->getFloat64Parameter(           OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMCost);
	m_oParam.nu                 = this->getFloat64Parameter(           OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMNu);
	m_oParam.p                  = this->getFloat64Parameter(           OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMEpsilon);
	m_oParam.cache_size         = this->getFloat64Parameter(           OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMCacheSize);
	m_oParam.eps                = this->getFloat64Parameter(           OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMTolerance);
	m_oParam.shrinking          = this->getBooleanParameter(           OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMShrinking);
//	m_oParam.probability        = this->getBooleanParameter(           OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMShrinking);
	m_oParam.probability        = true;
	CString l_sParamWeight      = *this->getCStringParameter(          OVP_ALgorithm_ClassifierSVM_InputParameterId_SVMweight);
	CString l_sParamWeightLabel = *this->getCStringParameter(          OVP_Algorithm_ClassifierSVM_InputParameterId_SVMWeightLabel);

	this->uninitializeExtraParameterMechanism();

	std::vector<float64> l_vWeight;
	std::stringstream l_oStreamString((const char*)l_sParamWeight);
	float64 l_f64CurrentValue;
	while(l_oStreamString>>l_f64CurrentValue)
	{
		l_vWeight.push_back(l_f64CurrentValue);
	}
	m_oParam.nr_weight = l_vWeight.size();
	double * l_pWeight=new double[l_vWeight.size()];
	for(uint32 i=0;i<l_vWeight.size();i++)
	{
		l_pWeight[i]=l_vWeight[i];
	}
	m_oParam.weight = l_pWeight;//NULL;

	std::vector<int64> l_vWeightLabel;
	std::stringstream l_oStreamStringLabel((const char*)l_sParamWeightLabel);
	int64 l_i64CurrentValue;
	while(l_oStreamStringLabel>>l_i64CurrentValue)
	{
		l_vWeightLabel.push_back(l_i64CurrentValue);
	}

	//the number of weight label need to be equal to the number of weight
	while(l_vWeightLabel.size()<l_vWeight.size())
	{
		l_vWeightLabel.push_back(l_vWeightLabel.size()+1);
	}

	int * l_pWeightLabel=new int[l_vWeight.size()];
	for(uint32 i=0;i<l_vWeight.size();i++)
	{
		l_pWeightLabel[i]=(int)l_vWeightLabel[i];
	}
	m_oParam.weight_label = l_pWeightLabel;//NULL;
}

boolean CAlgorithmClassifierSVM::train(const IFeatureVectorSet& rFeatureVectorSet)
{

	if(m_oProb.x != NULL && m_oProb.y != NULL)
	{
		for(int i=0;i<m_oProb.l;i++)
		{
			delete[] m_oProb.x[i];
		}
		delete[] m_oProb.y;
		delete[] m_oProb.x;
		m_oProb.y=NULL;
		m_oProb.x=NULL;
	}
	// default Param values
	//std::cout<<"param config"<<std::endl;
	this->setParameter();
	this->getLogManager() << LogLevel_Trace << paramToString(&m_oParam);

	//configure m_oProb
	//std::cout<<"prob config"<<std::endl;
	m_oProb.l=rFeatureVectorSet.getFeatureVectorCount();
	m_ui32NumberOfFeatures=rFeatureVectorSet[0].getSize();

	m_oProb.y = new double[m_oProb.l];
	m_oProb.x = new svm_node*[m_oProb.l];

	//std::cout<< "number vector:"<<l_oProb.l<<" size of vector:"<<m_ui32NumberOfFeatures<<std::endl;

	for(int i=0;i<m_oProb.l;i++)
	{
		m_oProb.x[i] = new svm_node[m_ui32NumberOfFeatures+1];
		m_oProb.y[i] = rFeatureVectorSet[i].getLabel();
		for(uint32 j=0;j<m_ui32NumberOfFeatures;j++)
		{
			m_oProb.x[i][j].index=j;
			m_oProb.x[i][j].value=rFeatureVectorSet[i].getBuffer()[j];
		}
		m_oProb.x[i][m_ui32NumberOfFeatures].index=-1;
	}
	if(m_oParam.gamma == 0 && m_ui32NumberOfFeatures > 0)
	{
		m_oParam.gamma = 1.0/m_ui32NumberOfFeatures;
	}

	if(m_oParam.kernel_type == PRECOMPUTED)
	{
		for(int i=0;i<m_oProb.l;i++)
		{
			if(m_oProb.x[i][0].index!=0)
			{
				this->getLogManager() << LogLevel_Error << "Wrong input format: first column must be 0:sample_serial_number\n";
				return false;
			}
			if(m_oProb.x[i][0].value <= 0 || m_oProb.x[i][0].value > m_ui32NumberOfFeatures)
			{
				this->getLogManager() << LogLevel_Error << "Wrong input format: sample_serial_number out of range\n";
				return false;
			}
		}
	}

	this->getLogManager() << LogLevel_Trace << problemToString(&m_oProb);

	//make a model
	//std::cout<<"svm_train"<<std::endl;
	if(m_pModel)
	{
		//std::cout<<"delete model"<<std::endl;
		deleteModel(m_pModel, !m_bModelWasTrained);
		m_pModel=NULL;
		m_bModelWasTrained = false;
	}
	m_pModel=svm_train(&m_oProb,&m_oParam);

	if(m_pModel == NULL)
	{
		this->getLogManager() << LogLevel_Error << "the training with SVM had failed\n";
		return false;
	}

	m_bModelWasTrained = true;

	//std::cout<<"log model"<<std::endl;
	this->getLogManager() << LogLevel_Trace << modelToString();

	return true;
}

boolean CAlgorithmClassifierSVM::classify(const IFeatureVector& rFeatureVector, float64& rf64Class, IVector& rClassificationValues, IVector& rProbabilityValue)
{
	//std::cout<<"classify"<<std::endl;
	if(m_pModel==NULL)
	{
		this->getLogManager() << LogLevel_Error << "classify impossible with a model equal NULL\n";
		return false;
	}
	if(m_pModel->nr_class==0||m_pModel->rho==NULL)
	{
		this->getLogManager() << LogLevel_Error << "the model wasn't load correctly\n";
		return false;
	}
	//std::cout<<"create l_pX"<<std::endl;
	svm_node* l_pX=new svm_node[rFeatureVector.getSize()+1];
	//std::cout<<"rFeatureVector.getSize():"<<rFeatureVector.getSize()<<"m_ui32NumberOfFeatures"<<m_ui32NumberOfFeatures<<std::endl;
	for(unsigned int i=0;i<rFeatureVector.getSize();i++)
	{
		l_pX[i].index=i;
		l_pX[i].value=rFeatureVector.getBuffer()[i];
		//std::cout<< l_pX[i].index << ";"<<l_pX[i].value<<" ";
	}
	l_pX[rFeatureVector.getSize()].index=-1;

	//std::cout<<"create l_pProbEstimates"<<std::endl;
	double *l_pProbEstimates=NULL;
	l_pProbEstimates = new double[m_pModel->nr_class];
	for(int i=0;i<m_pModel->nr_class;i++)
	{
		l_pProbEstimates[i]=0;
	}

	rf64Class=svm_predict_probability(m_pModel,l_pX,l_pProbEstimates);
	//std::cout<<rf64Class<<std::endl;
	//std::cout<<"probability"<<std::endl;

	//If we are not in these mode, label is NULL and there is no probability
	if(m_pModel->param.svm_type == C_SVC || m_pModel->param.svm_type == NU_SVC)
	{
		this->getLogManager() << LogLevel_Trace <<"Label predict: "<<rf64Class<<"\n";

		for(int i=0;i<m_pModel->nr_class;i++)
		{
			this->getLogManager() << LogLevel_Trace << "index:"<<i<<" label:"<< m_pModel->label[i]<<" probability:"<<l_pProbEstimates[i]<<"\n";
			if( m_pModel->label[i] == 1 )
			{
				rProbabilityValue.setSize(1);
				rProbabilityValue[0]=l_pProbEstimates[i];

			}
		}
	}
	else
		rProbabilityValue.setSize(0);

	//The hyperplane distance is disabled for SVM
	rClassificationValues.setSize(0);

	//std::cout<<";"<<rf64Class<<";"<<rClassificationValues[0] <<";"<<l_pProbEstimates[0]<<";"<<l_pProbEstimates[1]<<std::endl;
	//std::cout<<"Label predict "<<rf64Class<< " proba:"<<rClassificationValues[0]<<std::endl;
	//std::cout<<"end classify"<<std::endl;
	delete[] l_pX;
	delete[] l_pProbEstimates;

	return true;
}

void CAlgorithmClassifierSVM::generateConfigurationNode(void)
{
	//xml file
	//std::cout<<"model save"<<std::endl;

	std::vector <OpenViBE::CString> l_vSVCoef;
	std::vector <OpenViBE::CString> l_vSVValue;

	//std::cout<<"model save: rho"<<std::endl;
	std::stringstream l_sRho;
	l_sRho << std::scientific << m_pModel->rho[0];

	for(int i=1;i<m_pModel->nr_class*(m_pModel->nr_class-1)/2;i++)
	{
		l_sRho << " " << m_pModel->rho[i];
	}

	//std::cout<<"model save: sv_coef and SV"<<std::endl;
	for(int i=0;i<m_pModel->l;i++)
	{
		std::stringstream l_sSVCoef;
		std::stringstream l_sSVValue;

		l_sSVCoef << m_pModel->sv_coef[0][i];
		for(int j=1;j<m_pModel->nr_class-1;j++)
		{
			l_sSVCoef << " " << m_pModel->sv_coef[j][i];
		}

		const svm_node *p = m_pModel->SV[i];

		if(m_pModel->param.kernel_type == PRECOMPUTED)
		{
			l_sSVValue << "0:" << (double)(p->value);
		}
		else
		{
			if(p->index != -1)
			{
				l_sSVValue << p->index << ":" <<p->value;
				p++;
			}
			while(p->index != -1)
			{
				l_sSVValue << " " << p->index << ":" << p->value;
				p++;
			}
		}
		l_vSVCoef.push_back(CString(l_sSVCoef.str().c_str()));
		l_vSVValue.push_back(CString(l_sSVValue.str().c_str()));
	}
	//std::cout<<"xml save"<<std::endl;
	m_pConfigurationNode = XML::createNode(c_sClassifierRoot);

	XML::IXMLNode *l_pSVMNode = XML::createNode(c_sTypeNodeName);

	//Param node
	XML::IXMLNode *l_pParamNode = XML::createNode(c_sParamNodeName);
	XML::IXMLNode *l_pTempNode = XML::createNode(c_sSvmTypeNodeName);
	l_pTempNode->setPCData(get_svm_type(m_pModel->param.svm_type));
	l_pParamNode->addChild(l_pTempNode);

	l_pTempNode = XML::createNode(c_sKernelTypeNodeName);
	l_pTempNode->setPCData(get_kernel_type(m_pModel->param.kernel_type));
	l_pParamNode->addChild(l_pTempNode);

	if(m_pModel->param.kernel_type == POLY)
	{
		std::stringstream l_sParamDegree;
		l_sParamDegree << m_pModel->param.degree;

		l_pTempNode = XML::createNode(c_sDegreeNodeName);
		l_pTempNode->setPCData(l_sParamDegree.str().c_str());
		l_pParamNode->addChild(l_pTempNode);
	}
	if(m_pModel->param.kernel_type == POLY || m_pModel->param.kernel_type == RBF || m_pModel->param.kernel_type == SIGMOID)
	{
		std::stringstream l_sParamGamma;
		l_sParamGamma << m_pModel->param.gamma;

		l_pTempNode = XML::createNode(c_sGammaNodeName);
		l_pTempNode->setPCData(l_sParamGamma.str().c_str());
		l_pParamNode->addChild(l_pTempNode);
	}
	if(m_pModel->param.kernel_type == POLY || m_pModel->param.kernel_type == SIGMOID)
	{
		std::stringstream l_sParamCoef0;
		l_sParamCoef0 << m_pModel->param.coef0;

		l_pTempNode = XML::createNode(c_sCoef0NodeName);
		l_pTempNode->setPCData(l_sParamCoef0.str().c_str());
		l_pParamNode->addChild(l_pTempNode);
	}
	l_pSVMNode->addChild(l_pParamNode);
	//End param node

	//Model Node
	XML::IXMLNode * l_pModelNode = XML::createNode(c_sModelNodeName);
	{
		l_pTempNode = XML::createNode(c_sNrClassNodeName);
		std::stringstream l_sModelNrClass;
		l_sModelNrClass << m_pModel->nr_class;
		l_pTempNode->setPCData(l_sModelNrClass.str().c_str());
		l_pModelNode->addChild(l_pTempNode);

		l_pTempNode = XML::createNode(c_sTotalSvNodeName);
		std::stringstream l_sModelTotalSV;
		l_sModelTotalSV << m_pModel->l;
		l_pTempNode->setPCData(l_sModelTotalSV.str().c_str());
		l_pModelNode->addChild(l_pTempNode);

		l_pTempNode = XML::createNode(c_sRhoNodeName);
		l_pTempNode->setPCData(l_sRho.str().c_str());
		l_pModelNode->addChild(l_pTempNode);

		if(m_pModel->label)
		{
			std::stringstream l_sLabel;
			l_sLabel << m_pModel->label[0];
			for(int i=1;i<m_pModel->nr_class;i++)
			{
				l_sLabel << " " << m_pModel->label[i];
			}

			l_pTempNode = XML::createNode(c_sLabelNodeName);
			l_pTempNode->setPCData(l_sLabel.str().c_str());
			l_pModelNode->addChild(l_pTempNode);
		}
		if(m_pModel->probA)
		{
			std::stringstream l_sProbA;
			l_sProbA << std::scientific << m_pModel->probA[0];
			for(int i=1;i<m_pModel->nr_class*(m_pModel->nr_class-1)/2;i++)
			{
				l_sProbA << " " << m_pModel->probA[i];
			}

			l_pTempNode = XML::createNode(c_sProbANodeName);
			l_pTempNode->setPCData(l_sProbA.str().c_str());
			l_pModelNode->addChild(l_pTempNode);
		}
		if(m_pModel->probB)
		{
			std::stringstream l_sProbB;
			l_sProbB << std::scientific << m_pModel->probB[0];
			for(int i=1;i<m_pModel->nr_class*(m_pModel->nr_class-1)/2;i++)
			{
				 l_sProbB << " " << m_pModel->probB[i];
			}

			l_pTempNode = XML::createNode(c_sProbBNodeName);
			l_pTempNode->setPCData(l_sProbB.str().c_str());
			l_pModelNode->addChild(l_pTempNode);
		}
		if(m_pModel->nSV)
		{
			std::stringstream l_sNrSV;
			l_sNrSV << m_pModel->nSV[0];
			for(int i=1;i<m_pModel->nr_class;i++)
			{
				l_sNrSV << " " << m_pModel->nSV[i];
			}

			l_pTempNode = XML::createNode(c_sNrSvNodeName);
			l_pTempNode->setPCData(l_sNrSV.str().c_str());
			l_pModelNode->addChild(l_pTempNode);
		}

		XML::IXMLNode * l_pSVsNode = XML::createNode(c_sSvsNodeName);
		{
			for(int i=0;i<m_pModel->l;i++)
			{
				XML::IXMLNode * l_pSVNode = XML::createNode(c_sSVNodeName);
				{
					l_pTempNode = XML::createNode(c_sCoefNodeName);
					l_pTempNode->setPCData(l_vSVCoef[i]);
					l_pSVNode->addChild(l_pTempNode);

					l_pTempNode = XML::createNode(c_sValueNodeName);
					l_pTempNode->setPCData(l_vSVValue[i]);
					l_pSVNode->addChild(l_pTempNode);
				}
				l_pSVsNode->addChild(l_pSVNode);
			}
		}
		l_pModelNode->addChild(l_pSVsNode);
	}
	l_pSVMNode->addChild(l_pModelNode);
	m_pConfigurationNode->addChild(l_pSVMNode);
}

XML::IXMLNode* CAlgorithmClassifierSVM::saveConfiguration(void)
{
	generateConfigurationNode();
	return m_pConfigurationNode;
}

boolean CAlgorithmClassifierSVM::loadConfiguration(XML::IXMLNode *pConfigurationNode)
{
	if(m_pModel)
	{
		//std::cout<<"delete m_pModel load config"<<std::endl;
		deleteModel(m_pModel, !m_bModelWasTrained);
		m_pModel=NULL;
		m_bModelWasTrained = false;
	}
	//std::cout<<"load config"<<std::endl;
	m_pModel=new svm_model();
	m_pModel->rho = NULL;
	m_pModel->probA = NULL;
	m_pModel->probB = NULL;
	m_pModel->label = NULL;
	m_pModel->nSV = NULL;
	m_i32IndexSV=-1;

	loadParamNodeConfiguration(pConfigurationNode->getChild(0)->getChildByName(c_sParamNodeName));
	loadModelNodeConfiguration(pConfigurationNode->getChild(0)->getChildByName(c_sModelNodeName));

	this->getLogManager() << LogLevel_Trace << modelToString();
	return true;
}

void CAlgorithmClassifierSVM::loadParamNodeConfiguration(XML::IXMLNode *pParamNode)
{
	//svm_type
	XML::IXMLNode* l_pTempNode = pParamNode->getChildByName(c_sSvmTypeNodeName);
	for(int i =0; get_svm_type(i);i++)
	{
		if ( strcmp(get_svm_type(i),l_pTempNode->getPCData())==0)
		{
			m_pModel->param.svm_type=i;
		}
	}
	if(get_svm_type(m_pModel->param.svm_type) == NULL)
	{
		this->getLogManager() << LogLevel_Error << "load configuration error: bad value for the parameter svm_type\n";
	}

	//kernel_type
	l_pTempNode = pParamNode->getChildByName(c_sKernelTypeNodeName);
	for(int i =0; get_kernel_type(i);i++)
	{
		if ( strcmp(get_kernel_type(i), l_pTempNode->getPCData())==0)
		{
			m_pModel->param.kernel_type=i;
		}
	}
	if(get_kernel_type(m_pModel->param.kernel_type) == NULL)
	{
		this->getLogManager() << LogLevel_Error << "load configuration error: bad value for the parameter kernel_type\n";
	}

	//Following parameters aren't required

	//degree
	l_pTempNode = pParamNode->getChildByName(c_sDegreeNodeName);
	if(l_pTempNode != NULL)
	{
		std::stringstream l_sData(l_pTempNode->getPCData());
		l_sData >> m_pModel->param.degree;
	}

	//gamma
	l_pTempNode = pParamNode->getChildByName(c_sDegreeNodeName);
	if(l_pTempNode != NULL)
	{
		std::stringstream l_sData(l_pTempNode->getPCData());
		l_sData >> m_pModel->param.gamma;
	}

	//coef0
	l_pTempNode = pParamNode->getChildByName(c_sCoef0NodeName);
	if(l_pTempNode != NULL)
	{
		std::stringstream l_sData(l_pTempNode->getPCData());
		l_sData >> m_pModel->param.coef0;
	}
}

void CAlgorithmClassifierSVM::loadModelNodeConfiguration(XML::IXMLNode *pModelNode)
{

	//nr_class
	XML::IXMLNode* l_pTempNode = pModelNode->getChildByName(c_sNrClassNodeName);
	std::stringstream l_sModelNrClass(l_pTempNode->getPCData());
	l_sModelNrClass >> m_pModel->nr_class;
	//total_sv
	l_pTempNode = pModelNode->getChildByName(c_sTotalSvNodeName);
	std::stringstream l_sModelTotalSv(l_pTempNode->getPCData());
	l_sModelTotalSv >> m_pModel->l;
	//rho
	l_pTempNode = pModelNode->getChildByName(c_sRhoNodeName);
	std::stringstream l_sModelRho(l_pTempNode->getPCData());
	m_pModel->rho = new double[m_pModel->nr_class*(m_pModel->nr_class-1)/2];
	for(int i=0;i<m_pModel->nr_class*(m_pModel->nr_class-1)/2;i++)
	{
		l_sModelRho >> m_pModel->rho[i];
	}

	//label
	l_pTempNode = pModelNode->getChildByName(c_sLabelNodeName);
	if(l_pTempNode != NULL)
	{
		std::stringstream l_sData(l_pTempNode->getPCData());
		m_pModel->label = new int[m_pModel->nr_class];
		for(int i=0;i<m_pModel->nr_class;i++)
		{
			l_sData >> m_pModel->label[i];
		}
	}
	//probA
	l_pTempNode = pModelNode->getChildByName(c_sProbANodeName);
	if(l_pTempNode != NULL)
	{
		std::stringstream l_sData(l_pTempNode->getPCData());
		m_pModel->probA = new double[m_pModel->nr_class*(m_pModel->nr_class-1)/2];
		for(int i=0;i<m_pModel->nr_class*(m_pModel->nr_class-1)/2;i++)
		{
			l_sData >> m_pModel->probA[i];
		}
	}
	//probB
	l_pTempNode = pModelNode->getChildByName(c_sProbBNodeName);
	if(l_pTempNode != NULL)
	{
		std::stringstream l_sData(l_pTempNode->getPCData());
		m_pModel->probB = new double[m_pModel->nr_class*(m_pModel->nr_class-1)/2];
		for(int i=0;i<m_pModel->nr_class*(m_pModel->nr_class-1)/2;i++)
		{
			l_sData >> m_pModel->probB[i];
		}
	}
	//nr_sv
	l_pTempNode = pModelNode->getChildByName(c_sNrSvNodeName);
	if(l_pTempNode != NULL)
	{
		std::stringstream l_sData(l_pTempNode->getPCData());
		m_pModel->nSV = new int[m_pModel->nr_class];
		for(int i=0;i<m_pModel->nr_class;i++)
		{
			l_sData >> m_pModel->nSV[i];
		}
	}

	loadModelSVsNodeConfiguration(pModelNode->getChildByName(c_sSvsNodeName));
}

void CAlgorithmClassifierSVM::loadModelSVsNodeConfiguration(XML::IXMLNode *pSVsNodeParam)
{
	//Reserve all memory space required
	m_pModel->sv_coef = new double*[m_pModel->nr_class-1];
	for(int i=0;i<m_pModel->nr_class-1;i++)
	{
		m_pModel->sv_coef[i]=new double[m_pModel->l];
	}
	m_pModel->SV = new svm_node*[m_pModel->l];

	//Now fill SV
	for(uint32 i = 0; i < pSVsNodeParam->getChildCount(); ++i)
	{
		XML::IXMLNode *l_pTempNode = pSVsNodeParam->getChild(i);
		std::stringstream l_sCoefData(l_pTempNode->getChildByName(c_sCoefNodeName)->getPCData());
		for(int j=0;j<m_pModel->nr_class-1;j++)
		{
			l_sCoefData >> m_pModel->sv_coef[j][i];
		}

		std::stringstream l_sValueData(l_pTempNode->getChildByName(c_sValueNodeName)->getPCData());
		std::vector <int> l_vSVMIndex;
		std::vector <double> l_vSVMValue;
		char l_cSeparateChar;
		while(!l_sValueData.eof())
		{
			int l_iIndex;
			double l_dValue;
			l_sValueData >> l_iIndex;
			l_sValueData >> l_cSeparateChar;
			l_sValueData >> l_dValue;
			l_vSVMIndex.push_back(l_iIndex);
			l_vSVMValue.push_back(l_dValue);
		}

		m_ui32NumberOfFeatures=l_vSVMIndex.size();
		m_pModel->SV[i] = new svm_node[l_vSVMIndex.size()+1];
		for(size_t j=0;j<l_vSVMIndex.size();j++)
		{
			m_pModel->SV[i][j].index=l_vSVMIndex[j];
			m_pModel->SV[i][j].value=l_vSVMValue[j];
		}
		m_pModel->SV[i][l_vSVMIndex.size()].index=-1;
	}
}

CString CAlgorithmClassifierSVM::paramToString(svm_parameter *pParam)
{
	std::stringstream l_sParam;
	if(pParam == NULL)
	{
		l_sParam << "Param: NULL\n";
		return CString(l_sParam.str().c_str());
	}
	l_sParam << "Param:\n";
	l_sParam << "\tsvm_type: "<<get_svm_type(pParam->svm_type)<<"\n";
	l_sParam << "\tkernel_type: "<<get_kernel_type(pParam->kernel_type)<<"\n";
	l_sParam << "\tdegree: "<<pParam->degree<<"\n";
	l_sParam << "\tgamma: "<<pParam->gamma<<"\n";
	l_sParam << "\tcoef0: "<<pParam->coef0<<"\n";
	l_sParam << "\tnu: "<<pParam->nu << "\n";
	l_sParam << "\tcache_size: "<<pParam->cache_size << "\n";
	l_sParam << "\tC: "<<pParam->C << "\n";
	l_sParam << "\teps: "<<pParam->eps << "\n";
	l_sParam << "\tp: "<<pParam->p << "\n";
	l_sParam << "\tshrinking: "<<pParam->shrinking << "\n";
	l_sParam << "\tprobability: "<<pParam->probability << "\n";
	l_sParam << "\tnr weight: "<<pParam->nr_weight << "\n";
	std::stringstream l_sWeightLabel;
	for(int i=0;i<pParam->nr_weight;i++)
	{
		l_sWeightLabel<<pParam->weight_label[i]<<";";
	}
	l_sParam << "\tweight label: "<<l_sWeightLabel.str().c_str()<< "\n";
	std::stringstream l_sWeight;
	for(int i=0;i<pParam->nr_weight;i++)
	{
		l_sWeight<<pParam->weight[i]<<";";
	}
	l_sParam << "\tweight: "<<l_sWeight.str().c_str()<< "\n";
	return CString(l_sParam.str().c_str());
}


CString CAlgorithmClassifierSVM::modelToString()
{
	std::stringstream l_sModel;
	if(m_pModel==NULL)
	{
		l_sModel << "Model: NULL\n";
		return CString(l_sModel.str().c_str());
	}
	l_sModel << paramToString(&m_pModel->param);
	l_sModel << "Model:" << "\n";
	l_sModel << "\tnr_class: "<< m_pModel->nr_class <<"\n";
	l_sModel << "\ttotal_sv: "<<m_pModel->l<<"\n";
	l_sModel << "\trho: ";
	if(m_pModel->rho)
	{
		l_sModel << m_pModel->rho[0];
		for(int i=1;i<m_pModel->nr_class*(m_pModel->nr_class-1)/2;i++)
		{
			l_sModel << " "<<m_pModel->rho[i];
		}
	}
	l_sModel << "\n";
	l_sModel << "\tlabel: ";
	if(m_pModel->label)
	{
		l_sModel << m_pModel->label[0];
		for(int i=1;i<m_pModel->nr_class;i++)
		{
			l_sModel << " "<<m_pModel->label[i];
		}
	}
	l_sModel << "\n";
	l_sModel << "\tprobA: ";
	if(m_pModel->probA)
	{
		l_sModel << m_pModel->probA[0];
		for(int i=1;i<m_pModel->nr_class*(m_pModel->nr_class-1)/2;i++)
		{
			l_sModel << " "<<m_pModel->probA[i];
		}
	}
	l_sModel << "\n";
	l_sModel << "\tprobB: ";
	if(m_pModel->probB)
	{
		l_sModel << m_pModel->probB[0];
		for(int i=1;i<m_pModel->nr_class*(m_pModel->nr_class-1)/2;i++)
		{
			l_sModel << " "<<m_pModel->probB[i];
		}
	}
	l_sModel << "\n";
	l_sModel << "\tnr_sv: ";
	if(m_pModel->nSV)
	{
		l_sModel << m_pModel->nSV[0];
		for(int i=1;i<m_pModel->nr_class;i++)
		{
			l_sModel << " "<<m_pModel->nSV[i];
		}
	}
	l_sModel << "\n";

	return CString(l_sModel.str().c_str());
}
CString CAlgorithmClassifierSVM::problemToString(svm_problem *pProb)
{
	std::stringstream l_sProb;
	if(pProb == NULL)
	{
		l_sProb << "Problem: NULL\n";
		return CString(l_sProb.str().c_str());
	}
	l_sProb << "Problem";
	l_sProb << "\ttotal sv: "<<pProb->l<<"\n";
	l_sProb << "\tnb features: " << m_ui32NumberOfFeatures << "\n";

	return CString(l_sProb.str().c_str());
}
