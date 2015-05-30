#include "ovpCBoxAlgorithmOSCController.h"

// #include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::NetworkIO;

OpenViBE::boolean CBoxAlgorithmOSCController::initialize(void)
{
	const CString l_sServerAddress = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	const uint64 l_ui64ServerPort = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_sOSCAddress = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);

	const char *l_sTemp = m_sOSCAddress.toASCIIString();
	if(!l_sTemp || !l_sTemp[0] || l_sTemp[0]!='/')
	{
		this->getLogManager() << LogLevel_Error << "OSC Address must start with a '/'\n";
		return false;
	}

	// the static box context describes the box inputs, outputs, settings structures
	IBox& l_rStaticBoxContext = this->getStaticBoxContext();
	
	// Connect the socket 
	std::string l_sAddress = std::string(l_sServerAddress.toASCIIString());
	m_oUdpSocket.connectTo(l_sAddress, static_cast<uint32>(l_ui64ServerPort));

	if (!m_oUdpSocket.isOk()) {
		this->getLogManager() << LogLevel_Error << "Error connecting to socket\n";
		return false;
	}

	// Get appropriate decoder
	CIdentifier l_oStreamType;
	l_rStaticBoxContext.getInputType(0, l_oStreamType);

	m_pStreamDecoder = NULL;
	if(this->getTypeManager().isDerivedFromStream(l_oStreamType,OV_TypeId_StreamedMatrix))
	{
		m_pStreamDecoder = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamDecoder));
	}
	else if(l_oStreamType==OV_TypeId_Stimulations) 
	{
		m_pStreamDecoder = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	} 
	else
	{
		this->getLogManager() << LogLevel_Error << "Unsupported type\n";
		return false;
	}
	m_pStreamDecoder->initialize();
	
	return true;
}

OpenViBE::boolean CBoxAlgorithmOSCController::uninitialize(void)
{
	if(m_oUdpSocket.isOk())
	{
		m_oUdpSocket.close();
	}

	if(m_pStreamDecoder)
	{
		this->getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoder);
		m_pStreamDecoder = NULL;
	}

	return true;
}

OpenViBE::boolean CBoxAlgorithmOSCController::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

OpenViBE::boolean CBoxAlgorithmOSCController::process(void)
{
	// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
	IBoxIO& l_rDynamicBoxContext = this->getDynamicBoxContext();
	IBox& l_rStaticBoxContext=getStaticBoxContext();

	CIdentifier l_oStreamType;
	l_rStaticBoxContext.getInputType(0, l_oStreamType);

	oscpkt::PacketWriter pw;
	oscpkt::Message msg;
	bool l_bHaveData = false;

	for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(0); j++)
	{
		if(this->getTypeManager().isDerivedFromStream(l_oStreamType,OV_TypeId_StreamedMatrix))
		{
			TParameterHandler < const IMemoryBuffer* > ip_pMemoryBuffer(m_pStreamDecoder->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_InputParameterId_MemoryBufferToDecode));
			TParameterHandler < const IMatrix* > op_pMatrix(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputParameterId_Matrix));

			ip_pMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, j);
			m_pStreamDecoder->process();

			if (m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedBuffer))
			{
				// Check that the dimensions are acceptable
				const IMatrix* l_pMatrix = op_pMatrix;
				if(l_pMatrix->getDimensionCount() < 1 ||  l_pMatrix->getDimensionCount() > 2)
				{
					this->getLogManager() << LogLevel_Error << "Only matrixes of 1 or 2 dimensions are supported\n";
					return false;
				}
				if(l_pMatrix->getDimensionCount() == 2 && l_pMatrix->getDimensionSize(0) != 1)
				{
					this->getLogManager() << LogLevel_Error << "The matrix should have only 1 channel. Use e.g. Channel Selector to prune\n";
					return false;
				}

				if (!l_bHaveData) {
					l_bHaveData = true;
					pw.startBundle();
				}

				for(uint32 k=0;k<l_pMatrix->getBufferElementCount();k++)
				{
					const float32 l_f32InputVal = static_cast<float32>(l_pMatrix->getBuffer()[k]);
					pw.addMessage(msg.init(m_sOSCAddress.toASCIIString()).pushFloat(l_f32InputVal));

					// std::cout << "Add float " << l_f32InputVal << "\n";
				}
			}
		}
		else if(l_oStreamType==OV_TypeId_Stimulations)
		{
			TParameterHandler < const IMemoryBuffer* > ip_pMemoryBuffer(m_pStreamDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
			TParameterHandler < const IStimulationSet* > op_pStimulationSet(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

			ip_pMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, j);
			m_pStreamDecoder->process();

			if (m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
			{
				if (!l_bHaveData) {
					l_bHaveData = true;
					pw.startBundle();
				}

				for(uint64 k=0; k<op_pStimulationSet->getStimulationCount(); k++)
				{
					const uint32 l_ui32Stimulus = static_cast<uint32>(op_pStimulationSet->getStimulationIdentifier(k));

					pw.addMessage(msg.init(m_sOSCAddress.toASCIIString()).pushInt32(l_ui32Stimulus));

					// std::cout << "Add stimulus " << l_ui32Stimulus << "\n";
				}
			}
		}
		else
		{
			this->getLogManager() << LogLevel_Error << "Unknown stream type\n";
			return false;
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, j);
	}

	if (l_bHaveData) {
		pw.endBundle();
		if(!m_oUdpSocket.sendPacket(pw.packetData(), pw.packetSize()))
		{
			this->getLogManager() << LogLevel_Warning << "Error sending out UDP packet\n";
		}
	}
	return true;
}
