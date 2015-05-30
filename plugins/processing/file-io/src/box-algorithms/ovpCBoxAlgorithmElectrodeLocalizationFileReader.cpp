#include "ovpCBoxAlgorithmElectrodeLocalizationFileReader.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::FileIO;

uint64 CBoxAlgorithmElectrodeLocalisationFileReader::getClockFrequency(void)
{
	return ((uint64)1LL)<<32;
}

boolean CBoxAlgorithmElectrodeLocalisationFileReader::initialize(void)
{
	m_bHeaderSent = false;
	m_bBufferSent = false;

	// Creates algorithms
	m_pOVMatrixFileReader = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_OVMatrixFileReader));
	m_pChannelLocalisationStreamEncoder = new OpenViBEToolkit::TChannelLocalisationEncoder<CBoxAlgorithmElectrodeLocalisationFileReader>;
	m_pOVMatrixFileReader->initialize();
	m_pChannelLocalisationStreamEncoder->initialize(*this,0);

	//*
	// OVMatrix file reader parameters
	TParameterHandler < CString* > ip_sFilename(m_pOVMatrixFileReader->getInputParameter(OVP_Algorithm_OVMatrixFileReader_InputParameterId_Filename));
	TParameterHandler < IMatrix* > op_pMatrix(m_pOVMatrixFileReader->getOutputParameter(OVP_Algorithm_OVMatrixFileReader_OutputParameterId_Matrix));
/*
	// Channel localisation parameters
	TParameterHandler < boolean > ip_bDynamic(m_pChannelLocalisationStreamEncoder->getInputParameter(OVP_GD_Algorithm_ChannelLocalisationStreamEncoder_InputParameterId_Dynamic));
	TParameterHandler < IMatrix* > ip_pMatrix(m_pChannelLocalisationStreamEncoder->getInputParameter(OVP_GD_Algorithm_ChannelLocalisationStreamEncoder_InputParameterId_Matrix));

	// Configure parameters

	ip_bDynamic = false;
	ip_pMatrix.setReferenceTarget(op_pMatrix);
//*/

	m_pChannelLocalisationStreamEncoder->getInputDynamic() = false;



	// Configures settings according to box
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	l_rStaticBoxContext.getSettingValue(0, m_sFilename);
	*ip_sFilename = m_sFilename;

	return true;
}

boolean CBoxAlgorithmElectrodeLocalisationFileReader::uninitialize(void)
{
	//m_pOVMatrixFileReader->process(OVP_Algorithm_OVMatrixFileReader_InputTriggerId_Close);


	m_pOVMatrixFileReader->uninitialize();
	getAlgorithmManager().releaseAlgorithm(*m_pOVMatrixFileReader);

	if(m_pChannelLocalisationStreamEncoder)
	{
		m_pChannelLocalisationStreamEncoder->uninitialize();
		delete m_pChannelLocalisationStreamEncoder;
	}

	return true;
}

boolean CBoxAlgorithmElectrodeLocalisationFileReader::processClock(OpenViBE::CMessageClock& rMessageClock)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CBoxAlgorithmElectrodeLocalisationFileReader::process(void)
{
	if(m_bHeaderSent == true && m_bBufferSent == true)
	{
		return true;
	}

	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	// Channel localisation stream encoder parameters
	TParameterHandler < IMatrix* > op_pMatrix(m_pOVMatrixFileReader->getOutputParameter(OVP_Algorithm_OVMatrixFileReader_OutputParameterId_Matrix));

	m_pOVMatrixFileReader->process(/*OVP_Algorithm_OVMatrixFileReader_InputTriggerId_Next*/);

	//ensure matrix is 2 dimensional and that dimension sizes are correct
	if(op_pMatrix->getDimensionCount() != 2 ||
		op_pMatrix->getDimensionSize(1) != 3)
	{
		getLogManager() << LogLevel_Warning << "Wrong format for electrode localisation matrix loaded from file " << m_sFilename << " !\n";
		return false;
	}

	if(m_bHeaderSent == false)
	{
		// Connects parameters to memory buffer
		//op_pChannelLocalisationMemoryBuffer = l_rDynamicBoxContext.getOutputChunk(0);

		//open file and load matrix dimensions
		m_pOVMatrixFileReader->process(OVP_Algorithm_BrainampFileReader_InputTriggerId_Open);

		// Produces header
		IMatrix* l_pInputMatrix = m_pChannelLocalisationStreamEncoder->getInputMatrix();
		OpenViBEToolkit::Tools::Matrix::copy(*l_pInputMatrix, *op_pMatrix);

		m_pChannelLocalisationStreamEncoder->encodeHeader();

		// Sends header
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, 0, 0);

		m_bHeaderSent=true;
	}

	if(m_bBufferSent == false /*&&
		m_pOVMatrixFileReader->isOutputTriggerActive(OVP_Algorithm_OVMatrixFileReader_OutputTriggerId_DataProduced)*/)
	{
		// Connects parameters to memory buffer
		IMatrix* l_pInputMatrix = m_pChannelLocalisationStreamEncoder->getInputMatrix();
		OpenViBEToolkit::Tools::Matrix::copy(*l_pInputMatrix, *op_pMatrix);

		// Produces buffer
		m_pChannelLocalisationStreamEncoder->encodeBuffer();

		// Sends buffer
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, 0/*op_ui64CurrentStartTime*/, 0/*op_ui64CurrentEndTime*/);

		m_bBufferSent = true;
	}

	return true;
}
