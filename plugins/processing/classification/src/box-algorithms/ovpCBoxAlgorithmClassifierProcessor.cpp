#include "ovpCBoxAlgorithmClassifierProcessor.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <xml/IXMLHandler.h>
#include <xml/IXMLNode.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Classification;
using namespace std;

boolean CBoxAlgorithmClassifierProcessor::initialize(void)
{
	m_pFeaturesDecoder = NULL;
	m_pLabelsEncoder = NULL;
	m_pClassificationStateEncoder = NULL;
	m_pProbabilityValues=NULL;
	m_pClassifier = NULL;

	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	//First of all, let's get the XML file for configuration
	CString l_sConfigurationFilename;
	l_rStaticBoxContext.getSettingValue(0, l_sConfigurationFilename);

	if(l_sConfigurationFilename == CString("")) 
	{
		this->getLogManager() << LogLevel_Error << "You need to specify a classifier .xml for the box (use Classifier Trainer to create one)\n";
		return false;
	}

	XML::IXMLHandler *l_pHandler = XML::createXMLHandler();
	XML::IXMLNode *l_pRootNode = l_pHandler->parseFile(l_sConfigurationFilename.toASCIIString());

	if(!l_pRootNode) 
	{
		this->getLogManager() << LogLevel_Error << "Unable to get root node from [" << l_sConfigurationFilename << "]\n";
		return false;
	}

	//Now check the version, and let's display a message if the version is not good
	string l_sVersion;
	if(l_pRootNode->hasAttribute(c_sXmlVersionAttributeName))
	{
		l_sVersion = l_pRootNode->getAttribute(c_sXmlVersionAttributeName);
		std::stringstream l_sData(l_sVersion);
		uint32 l_ui32Version;
		l_sData >> l_ui32Version;
		if(l_ui32Version != OVP_Classification_BoxTrainerXMLVersion)
		{
			this->getLogManager() << LogLevel_Warning << "The configuration file doesn't have the same version numero than the box. Trouble may appear in loading process.\n";
		}
	}
	else
	{
		this->getLogManager() << LogLevel_Warning << "The configuration file has no version information. Trouble may appear in loading process.\n";
	}

	CIdentifier l_oAlgorithmClassIdentifier = OV_UndefinedIdentifier;

	XML::IXMLNode * l_pTempNode = l_pRootNode->getChildByName(c_sStrategyNodeName);
	if(l_pTempNode) {
		l_oAlgorithmClassIdentifier.fromString(l_pTempNode->getAttribute(c_sIdentifierAttributeName));
	} else {
		this->getLogManager() << LogLevel_Warning << "The configuration file had no node " << c_sStrategyNodeName << ". Trouble may appear later.\n";
	}

	//If the Identifier is undefined, that means we need to load a native algorithm
	if(l_oAlgorithmClassIdentifier == OV_UndefinedIdentifier){
		this->getLogManager() << LogLevel_Trace << "Using Native algorithm\n";
		l_pTempNode = l_pRootNode->getChildByName(c_sAlgorithmNodeName);
		if(l_pTempNode)
		{
			l_oAlgorithmClassIdentifier.fromString(l_pTempNode->getAttribute(c_sIdentifierAttributeName));
		}
		else
		{
			this->getLogManager() << LogLevel_Warning << "The configuration file had no node " << c_sAlgorithmNodeName << ". Trouble may appear later.\n";
		}

		//If the algorithm is still unknown, that means that we face an error
		if(l_oAlgorithmClassIdentifier==OV_UndefinedIdentifier)
		{
			this->getLogManager() << LogLevel_Error << "Couldn't restore a classifier from the file [" << l_sConfigurationFilename << "].\n";
			return false;
		}
	}

	//Now loading all stimulations output
	XML::IXMLNode *l_pStimulationsNode = l_pRootNode->getChildByName(c_sStimulationsNodeName);
	if(l_pStimulationsNode)
	{
		//Now load every stimulation and store them in the map with the right class id
		for(uint32 i=0; i < l_pStimulationsNode->getChildCount(); i++)
		{
			l_pTempNode = l_pStimulationsNode->getChild(i);
			if(!l_pTempNode)
			{
				this->getLogManager() << LogLevel_Error << "Expected child node " << i << " for node " << c_sStimulationsNodeName << ". Output labels not known. Aborting.\n";
				return false;
			}
			CString l_sStimulationName(l_pTempNode->getPCData());

			OpenViBE::float64 l_f64ClassId;
			std::stringstream l_sIdentifierData(l_pTempNode->getAttribute(c_sIdentifierAttributeName));
			l_sIdentifierData >> l_f64ClassId ;
			m_vStimulation[l_f64ClassId]=this->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_sStimulationName);
		}
	}
	else
	{
		this->getLogManager() << LogLevel_Warning << "The configuration file had no node " << c_sStimulationsNodeName << ". Trouble may appear later.\n";
	}

	m_pFeaturesDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_FeatureVectorStreamDecoder));
	m_pLabelsEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder));
	m_pClassificationStateEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder));
	m_pClassifier=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(l_oAlgorithmClassIdentifier));
	m_pProbabilityValues=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder));

	m_pFeaturesDecoder->initialize();
	m_pLabelsEncoder->initialize();
	m_pClassificationStateEncoder->initialize();
	m_pProbabilityValues->initialize();
	m_pClassifier->initialize();

	m_pClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_FeatureVector)->setReferenceTarget(m_pFeaturesDecoder->getOutputParameter(OVP_GD_Algorithm_FeatureVectorStreamDecoder_OutputParameterId_Matrix));
	m_pClassificationStateEncoder->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix)->setReferenceTarget(m_pClassifier->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_ClassificationValues));
	m_pProbabilityValues->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix)->setReferenceTarget(m_pClassifier->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_ProbabilityValues));

	TParameterHandler < XML::IXMLNode* > ip_pClassificationConfiguration(m_pClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_Configuration));
	ip_pClassificationConfiguration = l_pRootNode->getChildByName(c_sClassifierRoot);
	if(!m_pClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_LoadConfiguration)){
		this->getLogManager() << LogLevel_Error << "Subclassifier failed to load config\n";
		return false;
	}

	l_pRootNode->release();
	l_pHandler->release();

	m_bOutputHeaderSent=false;
	return true;
}

boolean CBoxAlgorithmClassifierProcessor::uninitialize(void)
{
	if(m_pClassifier)
	{
		m_pClassifier->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pClassifier);
		m_pClassifier = NULL;
	}

	if(m_pClassificationStateEncoder)
	{
		m_pClassificationStateEncoder->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pClassificationStateEncoder);
		m_pClassificationStateEncoder = NULL;
	}

	if(m_pProbabilityValues)
	{
		m_pProbabilityValues->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pProbabilityValues);
		m_pProbabilityValues = NULL;
	}

	if(m_pLabelsEncoder)
	{
		m_pLabelsEncoder->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pLabelsEncoder);
		m_pLabelsEncoder = NULL;
	}

	if(m_pFeaturesDecoder)
	{
		m_pFeaturesDecoder->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pFeaturesDecoder);
		m_pFeaturesDecoder = NULL;
	}

	return true;
}

boolean CBoxAlgorithmClassifierProcessor::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

boolean CBoxAlgorithmClassifierProcessor::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		uint64 l_ui64StartTime=l_rDynamicBoxContext.getInputChunkStartTime(0, i);
		uint64 l_ui64EndTime=l_rDynamicBoxContext.getInputChunkEndTime(0, i);

		TParameterHandler < const IMemoryBuffer* > ip_pFeatureVectorMemoryBuffer(m_pFeaturesDecoder->getInputParameter(OVP_GD_Algorithm_FeatureVectorStreamDecoder_InputParameterId_MemoryBufferToDecode));
		TParameterHandler < IMemoryBuffer* > op_pLabelsMemoryBuffer(m_pLabelsEncoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
		TParameterHandler < IMemoryBuffer* > op_pClassificationStateMemoryBuffer(m_pClassificationStateEncoder->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
		TParameterHandler < IMemoryBuffer* > op_pProbabilityValues(m_pProbabilityValues->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

		TParameterHandler < IStimulationSet* > ip_pLabelsStimulationSet(m_pLabelsEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet));
		TParameterHandler < float64 > op_f64ClassificationStateClass(m_pClassifier->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Class));

		ip_pFeatureVectorMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, i);
		op_pLabelsMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(0);
		op_pClassificationStateMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(1);
		op_pProbabilityValues = l_rDynamicBoxContext.getOutputChunk(2);

		m_pFeaturesDecoder->process();
		if(m_pFeaturesDecoder->isOutputTriggerActive(OVP_GD_Algorithm_FeatureVectorStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			m_bOutputHeaderSent=false;
		}
		if(m_pFeaturesDecoder->isOutputTriggerActive(OVP_GD_Algorithm_FeatureVectorStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			if(m_pClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_Classify))
			{
				if (m_pClassifier->isOutputTriggerActive(OVTK_Algorithm_Classifier_OutputTriggerId_Success))
				{
					//this->getLogManager() << LogLevel_Warning << "---Classification successful---\n";
					if(!m_bOutputHeaderSent)
					{
						m_pLabelsEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
						m_pClassificationStateEncoder->process(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader);
						m_pProbabilityValues->process(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader);
						l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_ui64StartTime, l_ui64StartTime);
						l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_ui64StartTime, l_ui64StartTime);
						l_rDynamicBoxContext.markOutputAsReadyToSend(2, l_ui64StartTime, l_ui64StartTime);
						m_bOutputHeaderSent=true;
					}

					ip_pLabelsStimulationSet->setStimulationCount(1);
					ip_pLabelsStimulationSet->setStimulationIdentifier(0, m_vStimulation[op_f64ClassificationStateClass]);
					ip_pLabelsStimulationSet->setStimulationDate(0, l_ui64EndTime);
					ip_pLabelsStimulationSet->setStimulationDuration(0, 0);

					m_pLabelsEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
					m_pClassificationStateEncoder->process(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer);
					m_pProbabilityValues->process(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer);
					l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_ui64StartTime, l_ui64EndTime);
					l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_ui64StartTime, l_ui64EndTime);
					l_rDynamicBoxContext.markOutputAsReadyToSend(2, l_ui64StartTime, l_ui64EndTime);
				}
				//else
				//	this->getLogManager() << LogLevel_Warning << "---Classification failed---\n";
			}
		}
		if(m_pFeaturesDecoder->isOutputTriggerActive(OVP_GD_Algorithm_FeatureVectorStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
			m_pLabelsEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeEnd);
			m_pClassificationStateEncoder->process(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd);
			m_pProbabilityValues->process(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd);
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_ui64StartTime, l_ui64EndTime);
			l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_ui64StartTime, l_ui64EndTime);
			l_rDynamicBoxContext.markOutputAsReadyToSend(2, l_ui64StartTime, l_ui64EndTime);
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}

	return true;
}
