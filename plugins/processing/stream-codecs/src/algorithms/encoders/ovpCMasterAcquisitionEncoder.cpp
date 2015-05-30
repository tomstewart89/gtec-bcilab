#include "ovpCMasterAcquisitionEncoder.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::StreamCodecs;

#include <iostream>

boolean CMasterAcquisitionEncoder::initialize(void)
{	
	// Manages sub-algorithms

	m_pAcquisitionStreamEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_AcquisitionStreamEncoder));
	m_pExperimentInformationStreamEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_ExperimentInformationStreamEncoder));
	m_pSignalStreamEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_SignalStreamEncoder));
	m_pStimulationStreamEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_StimulationStreamEncoder));
	m_pChannelLocalisationStreamEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_ChannelLocalisationStreamEncoder));
	m_pChannelUnitsStreamEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_ChannelUnitsStreamEncoder));
	
	m_pAcquisitionStreamEncoder->initialize();
	m_pExperimentInformationStreamEncoder->initialize();
	m_pSignalStreamEncoder->initialize();
	m_pStimulationStreamEncoder->initialize();
	m_pChannelLocalisationStreamEncoder->initialize();
	m_pChannelUnitsStreamEncoder->initialize();

	// Declares parameter handlers for this algorithm

	TParameterHandler < uint64 > ip_ui64SubjectIdentifier(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SubjectIdentifier));
	TParameterHandler < uint64 > ip_ui64SubjectAge(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SubjectAge));
	TParameterHandler < uint64 > ip_ui64SubjectGender(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SubjectGender));
	TParameterHandler < IMatrix* > ip_pMatrix(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SignalMatrix));
	TParameterHandler < uint64 > ip_ui64SamplingRate(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SignalSamplingRate));
	TParameterHandler < IStimulationSet* > ip_pStimulationSet(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_StimulationSet));
	TParameterHandler < uint64 > ip_ui64BufferDuration(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_BufferDuration));
	TParameterHandler < IMemoryBuffer* > op_pEncodedMemoryBuffer(this->getOutputParameter(OVP_Algorithm_EBMLStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
	TParameterHandler < IMatrix* > ip_pChannelLocalisationMaster(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_ChannelLocalisation));
	TParameterHandler < IMatrix* > ip_pChannelUnitsMaster(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_ChannelUnits));

	// Declares parameter handlers for sub-algorithm acquisition

	TParameterHandler < IMemoryBuffer* > ip_pAcquisitionChannelUnitsMemoryBuffer(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_ChannelUnitsStream));
	TParameterHandler < IMemoryBuffer* > ip_pAcquisitionChannelLocalisationMemoryBuffer(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_ChannelLocalisationStream));
	TParameterHandler < IMemoryBuffer* > ip_pAcquisitionExperimentInformationMemoryBuffer(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_ExperimentInformationStream));
	TParameterHandler < IMemoryBuffer* > ip_pAcquisitionSignalMemoryBuffer(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_SignalStream));
	TParameterHandler < IMemoryBuffer* > ip_pAcquisitionStimulationMemoryBuffer(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_StimulationStream));
	TParameterHandler < uint64 > ip_ui64AcquisitionBufferDuration(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_BufferDuration));
	TParameterHandler < IMemoryBuffer* > op_pAcquisitionMemoryBuffer(m_pAcquisitionStreamEncoder->getOutputParameter(OVP_Algorithm_EBMLStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	// Declares parameter handlers for sub-algorithm experiment information

	// TParameterHandler < uint64 > ip_ui64ExperimentInformationExperimentIdentifier(m_pExperimentInformationStreamEncoder->getInputParameter(OVP_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_ExperimentIdentifier));
	// TParameterHandler < CString* > ip_pExperimentInformationExperimentDate(m_pExperimentInformationStreamEncoder->getInputParameter(OVP_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_ExperimentDate));
	TParameterHandler < uint64 > ip_ui64ExperimentInformationSubjectIdentifier(m_pExperimentInformationStreamEncoder->getInputParameter(OVP_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_SubjectIdentifier));
	// TParameterHandler < CString* > ip_pExperimentInformationSubjectName(m_pExperimentInformationStreamEncoder->getInputParameter(OVP_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_SubjectName));
	TParameterHandler < uint64 > ip_ui64ExperimentInformationSubjectAge(m_pExperimentInformationStreamEncoder->getInputParameter(OVP_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_SubjectAge));
	TParameterHandler < uint64 > ip_ui64ExperimentInformationSubjectGender(m_pExperimentInformationStreamEncoder->getInputParameter(OVP_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_SubjectGender));
	// TParameterHandler < uint64 > ip_ui64ExperimentInformationLaboratoryIdentifier(m_pExperimentInformationStreamEncoder->getInputParameter(OVP_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_LaboratoryIdentifier));
	// TParameterHandler < CString* > ip_pExperimentInformationLaboratoryName(m_pExperimentInformationStreamEncoder->getInputParameter(OVP_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_LaboratoryName));
	// TParameterHandler < uint64 > ip_ui64ExperimentInformationTechnicianIdentifier(m_pExperimentInformationStreamEncoder->getInputParameter(OVP_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_TechnicianIdentifier));
	// TParameterHandler < CString* > ip_pExperimentInformationTehnicianName(m_pExperimentInformationStreamEncoder->getInputParameter(OVP_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_TechnicianName));
	TParameterHandler < IMemoryBuffer* > op_pExperimentInformationMemoryBuffer(m_pExperimentInformationStreamEncoder->getOutputParameter(OVP_Algorithm_EBMLStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	// Declares parameter handlers for sub-algorithm signal

	TParameterHandler < IMatrix* > ip_pSignalMatrix(m_pSignalStreamEncoder->getInputParameter(OVP_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix));
	TParameterHandler < uint64 > ip_ui64SignalSamplingRate(m_pSignalStreamEncoder->getInputParameter(OVP_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate));
	TParameterHandler < IMemoryBuffer* > op_pSignalMemoryBuffer(m_pSignalStreamEncoder->getOutputParameter(OVP_Algorithm_EBMLStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	// Declares parameter handlers for sub-algorithm stimulation

	TParameterHandler < IStimulationSet* > ip_pStimulationStimulationSet(m_pStimulationStreamEncoder->getInputParameter(OVP_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet));
	TParameterHandler < IMemoryBuffer* > op_pStimulationMemoryBuffer(m_pStimulationStreamEncoder->getOutputParameter(OVP_Algorithm_EBMLStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	TParameterHandler < IMatrix* > ip_pChannelLocalisation(m_pChannelLocalisationStreamEncoder->getInputParameter(OVP_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix));
	TParameterHandler < IMemoryBuffer* > op_pChannelLocalisationMemoryBuffer(m_pChannelLocalisationStreamEncoder->getOutputParameter(OVP_Algorithm_EBMLStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	TParameterHandler < IMatrix* > ip_pUnits(m_pChannelUnitsStreamEncoder->getInputParameter(OVP_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix));
	TParameterHandler < IMemoryBuffer* > op_pChannelUnitsMemoryBuffer(m_pChannelUnitsStreamEncoder->getOutputParameter(OVP_Algorithm_EBMLStreamEncoder_OutputParameterId_EncodedMemoryBuffer));



	// Manage parameter connection / referencing | this algorithm to sub algorithm

	ip_ui64ExperimentInformationSubjectIdentifier.setReferenceTarget(ip_ui64SubjectIdentifier);
	ip_ui64ExperimentInformationSubjectAge.setReferenceTarget(ip_ui64SubjectAge);
	ip_ui64ExperimentInformationSubjectGender.setReferenceTarget(ip_ui64SubjectGender);
	ip_pSignalMatrix.setReferenceTarget(ip_pMatrix);
	ip_ui64SignalSamplingRate.setReferenceTarget(ip_ui64SamplingRate);
	ip_pStimulationStimulationSet.setReferenceTarget(ip_pStimulationSet);
	ip_ui64AcquisitionBufferDuration.setReferenceTarget(ip_ui64BufferDuration);
	op_pEncodedMemoryBuffer.setReferenceTarget(op_pAcquisitionMemoryBuffer);
	ip_pChannelLocalisationMaster.setReferenceTarget(ip_pChannelLocalisation);
	ip_pChannelUnitsMaster.setReferenceTarget(ip_pUnits);

	// Manage parameter connection / referencing | sub-algorithm to sub algorithm

	ip_pAcquisitionExperimentInformationMemoryBuffer.setReferenceTarget(op_pExperimentInformationMemoryBuffer);
	ip_pAcquisitionSignalMemoryBuffer.setReferenceTarget(op_pSignalMemoryBuffer);
	ip_pAcquisitionStimulationMemoryBuffer.setReferenceTarget(op_pStimulationMemoryBuffer);
	ip_pAcquisitionChannelLocalisationMemoryBuffer.setReferenceTarget(op_pChannelLocalisationMemoryBuffer);
	ip_pAcquisitionChannelUnitsMemoryBuffer.setReferenceTarget(op_pChannelUnitsMemoryBuffer);

	return true;
}

boolean CMasterAcquisitionEncoder::uninitialize(void)
{
	m_pChannelUnitsStreamEncoder->uninitialize();
	m_pChannelLocalisationStreamEncoder->uninitialize();
	m_pStimulationStreamEncoder->uninitialize();
	m_pSignalStreamEncoder->uninitialize();
	m_pExperimentInformationStreamEncoder->uninitialize();
	m_pAcquisitionStreamEncoder->uninitialize();

	this->getAlgorithmManager().releaseAlgorithm(*m_pChannelUnitsStreamEncoder);
	this->getAlgorithmManager().releaseAlgorithm(*m_pChannelLocalisationStreamEncoder);
	this->getAlgorithmManager().releaseAlgorithm(*m_pStimulationStreamEncoder);
	this->getAlgorithmManager().releaseAlgorithm(*m_pSignalStreamEncoder);
	this->getAlgorithmManager().releaseAlgorithm(*m_pExperimentInformationStreamEncoder);
	this->getAlgorithmManager().releaseAlgorithm(*m_pAcquisitionStreamEncoder);

	return true;
}

boolean CMasterAcquisitionEncoder::process(void)
{
	TParameterHandler < IMemoryBuffer* > ip_pAcquisitionChannelUnitsMemoryBuffer(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_ChannelUnitsStream));
	TParameterHandler < IMemoryBuffer* > ip_pAcquisitionChannelLocalisationMemoryBuffer(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_ChannelLocalisationStream));
	TParameterHandler < IMemoryBuffer* > ip_pAcquisitionExperimentInformationMemoryBuffer(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_ExperimentInformationStream));
	TParameterHandler < IMemoryBuffer* > ip_pAcquisitionSignalMemoryBuffer(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_SignalStream));
	TParameterHandler < IMemoryBuffer* > ip_pAcquisitionStimulationMemoryBuffer(m_pAcquisitionStreamEncoder->getInputParameter(OVP_Algorithm_AcquisitionStreamEncoder_InputParameterId_StimulationStream));
	// TParameterHandler < IMemoryBuffer* > op_pAcquisitionMemoryBuffer(m_pAcquisitionStreamEncoder->getOutputParameter(OVP_Algorithm_EBMLStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	if(this->isInputTriggerActive(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeHeader))
	{
		ip_pAcquisitionChannelUnitsMemoryBuffer->setSize(0, true);
		ip_pAcquisitionChannelLocalisationMemoryBuffer->setSize(0, true);
		ip_pAcquisitionExperimentInformationMemoryBuffer->setSize(0, true);
		ip_pAcquisitionSignalMemoryBuffer->setSize(0, true);
		ip_pAcquisitionStimulationMemoryBuffer->setSize(0, true);
		// op_pAcquisitionMemoryBuffer->setSize(0, true);

		m_pChannelLocalisationStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeHeader);
		m_pChannelUnitsStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeHeader);
		m_pStimulationStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeHeader);
		m_pSignalStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeHeader);
		m_pExperimentInformationStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeHeader);
		m_pAcquisitionStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeHeader);
	}
	if(this->isInputTriggerActive(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeBuffer))
	{
   		ip_pAcquisitionChannelUnitsMemoryBuffer->setSize(0, true);
		ip_pAcquisitionChannelLocalisationMemoryBuffer->setSize(0, true);
		ip_pAcquisitionExperimentInformationMemoryBuffer->setSize(0, true);
		ip_pAcquisitionSignalMemoryBuffer->setSize(0, true);
		ip_pAcquisitionStimulationMemoryBuffer->setSize(0, true);
		// op_pAcquisitionMemoryBuffer->setSize(0, true);

		// For these streams, we only send the buffer if there is something to send
		const TParameterHandler < boolean > ip_bEncodeUnitData(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_EncodeChannelUnitData));
		if(ip_bEncodeUnitData) {
			// this->getLogManager() << LogLevel_Info << "Encoding units " << ip_pUnits->getBufferElementCount() << "\n";
			m_pChannelUnitsStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeBuffer);
		}

		const TParameterHandler < boolean > ip_bEncodeChannelLocalisationData(this->getInputParameter(OVP_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_EncodeChannelLocalisationData));
		if(ip_bEncodeChannelLocalisationData) {
			m_pChannelLocalisationStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeBuffer);
		}

		m_pStimulationStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeBuffer);
		m_pSignalStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeBuffer);
		m_pExperimentInformationStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeBuffer);
		m_pAcquisitionStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeBuffer);
	}
	if(this->isInputTriggerActive(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeEnd))
	{
		ip_pAcquisitionChannelUnitsMemoryBuffer->setSize(0, true);
		ip_pAcquisitionChannelLocalisationMemoryBuffer->setSize(0, true);
		ip_pAcquisitionExperimentInformationMemoryBuffer->setSize(0, true);
		ip_pAcquisitionSignalMemoryBuffer->setSize(0, true);
		ip_pAcquisitionStimulationMemoryBuffer->setSize(0, true);
		// op_pAcquisitionMemoryBuffer->setSize(0, true);

		m_pChannelUnitsStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeEnd);
		m_pChannelLocalisationStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeEnd);
		m_pStimulationStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeEnd);
		m_pSignalStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeEnd);
		m_pExperimentInformationStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeEnd);
		m_pAcquisitionStreamEncoder->process(OVP_Algorithm_EBMLStreamEncoder_InputTriggerId_EncodeEnd);
	}

	return true;
}
