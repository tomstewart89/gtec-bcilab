#include "ovpCInputChannel.h"

#include <iostream>
#include <system/ovCMemory.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

namespace
{
	class _AutoCast_
	{
	public:
		_AutoCast_(IBox& rBox, IConfigurationManager& rConfigurationManager, const uint32 ui32Index) : m_rConfigurationManager(rConfigurationManager) { rBox.getSettingValue(ui32Index, m_sSettingValue); }
		operator uint64 (void) { return m_rConfigurationManager.expandAsUInteger(m_sSettingValue); }
		operator int64 (void) { return m_rConfigurationManager.expandAsInteger(m_sSettingValue); }
		operator float64 (void) { return m_rConfigurationManager.expandAsFloat(m_sSettingValue); }
		operator boolean (void) { return m_rConfigurationManager.expandAsBoolean(m_sSettingValue); }
		operator const CString (void) { return m_sSettingValue; }
	protected:
		IConfigurationManager& m_rConfigurationManager;
		CString m_sSettingValue;
	};
};

CInputChannel::CInputChannel(const OpenViBE::uint16 ui16InputIndex /*= 0*/)
	: m_ui32SignalChannel(ui16InputIndex*NB_CHANNELS + SIGNAL_CHANNEL)
	, m_ui32StimulationChannel(ui16InputIndex*NB_CHANNELS + STIMULATION_CHANNEL)

{
}

CInputChannel::~CInputChannel()
{ 
}

boolean CInputChannel::initialize(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>* pTBoxAlgorithm)
{
	m_bIsWorking                    = false;

	m_ui64StartTimestamp            = 0;
	m_ui64EndTimestamp              = 0;
	
	m_oIStimulationSet              = 0;
	m_pTBoxAlgorithm                = pTBoxAlgorithm;

	m_pStreamDecoderSignal          = new OpenViBEToolkit::TSignalDecoder <OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm> >();
	m_pStreamDecoderSignal->initialize(*m_pTBoxAlgorithm,0);

	m_pStreamDecoderStimulation     = new OpenViBEToolkit::TStimulationDecoder <OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm> >();
	m_pStreamDecoderStimulation->initialize(*m_pTBoxAlgorithm,1);

	return true;
}

boolean CInputChannel::uninitialize()
{
	m_pStreamDecoderStimulation->uninitialize();
	delete m_pStreamDecoderStimulation;
	
	m_pStreamDecoderSignal->uninitialize();
	delete m_pStreamDecoderSignal;

	return true;
}

boolean CInputChannel::waitForSignalHeader()
{
	IBoxIO& l_rDynamicBoxContext=m_pTBoxAlgorithm->getDynamicBoxContext();

	if(l_rDynamicBoxContext.getInputChunkCount(m_ui32SignalChannel))
	{
		m_pStreamDecoderSignal->decode(0);

		if(m_pStreamDecoderSignal->isHeaderReceived())
		{
			m_bIsWorking          = true;

			m_ui64StartTimestamp  = l_rDynamicBoxContext.getInputChunkStartTime(m_ui32SignalChannel, 0);
			m_ui64EndTimestamp    = l_rDynamicBoxContext.getInputChunkEndTime(m_ui32SignalChannel, 0);

			l_rDynamicBoxContext.markInputAsDeprecated(m_ui32SignalChannel, 0);

			return true;
		}
	}

	return false;
}

OpenViBE::uint32 CInputChannel::getNbOfStimulationBuffers()
{
	IBoxIO& l_rDynamicBoxContext  = m_pTBoxAlgorithm->getDynamicBoxContext();

	return l_rDynamicBoxContext.getInputChunkCount(m_ui32StimulationChannel);
}

OpenViBE::uint32 CInputChannel::getNbOfSignalBuffers()
{
	IBoxIO& l_rDynamicBoxContext  = m_pTBoxAlgorithm->getDynamicBoxContext();

	return l_rDynamicBoxContext.getInputChunkCount(m_ui32SignalChannel);
}

OpenViBE::IStimulationSet* CInputChannel::getStimulation(OpenViBE::uint64& startTimestamp, OpenViBE::uint64& endTimestamp, const OpenViBE::uint32 stimulationIndex)
{
	IBoxIO& l_rDynamicBoxContext  = m_pTBoxAlgorithm->getDynamicBoxContext();

	m_pStreamDecoderStimulation->decode(stimulationIndex);
	m_oIStimulationSet            = m_pStreamDecoderStimulation->getOutputStimulationSet();

	startTimestamp                = l_rDynamicBoxContext.getInputChunkStartTime(m_ui32StimulationChannel, stimulationIndex);
	endTimestamp                  = l_rDynamicBoxContext.getInputChunkEndTime(m_ui32StimulationChannel, stimulationIndex);

	l_rDynamicBoxContext.markInputAsDeprecated(m_ui32StimulationChannel, stimulationIndex);

	return m_oIStimulationSet;
}

OpenViBE::IStimulationSet* CInputChannel::discardStimulation(const OpenViBE::uint32 stimulationIndex)
{
	IBoxIO& l_rDynamicBoxContext  = m_pTBoxAlgorithm->getDynamicBoxContext();

	m_pStreamDecoderStimulation->decode(stimulationIndex);
	m_oIStimulationSet            = m_pStreamDecoderStimulation->getOutputStimulationSet();

	l_rDynamicBoxContext.markInputAsDeprecated(m_ui32StimulationChannel, stimulationIndex);
	
	return m_oIStimulationSet;
}


OpenViBE::float64* CInputChannel::getSignal(OpenViBE::uint64& startTimestamp, OpenViBE::uint64& endTimestamp, const OpenViBE::uint32 signalIndex)
{
	IBoxIO& l_rDynamicBoxContext  = m_pTBoxAlgorithm->getDynamicBoxContext();
	m_pStreamDecoderSignal->decode(signalIndex);
	if(!m_pStreamDecoderSignal->isBufferReceived())
		return 0;

	startTimestamp                = l_rDynamicBoxContext.getInputChunkStartTime(m_ui32SignalChannel, signalIndex);
	endTimestamp                  = l_rDynamicBoxContext.getInputChunkEndTime(m_ui32SignalChannel, signalIndex);

	l_rDynamicBoxContext.markInputAsDeprecated(m_ui32SignalChannel, signalIndex);

	return m_pStreamDecoderSignal->getOutputMatrix()->getBuffer();
}


OpenViBE::float64* CInputChannel::discardSignal(const OpenViBE::uint32 signalIndex)
{
	IBoxIO& l_rDynamicBoxContext  = m_pTBoxAlgorithm->getDynamicBoxContext();
	m_pStreamDecoderSignal->decode(signalIndex);
	if(!m_pStreamDecoderSignal->isBufferReceived())
		return 0;

	l_rDynamicBoxContext.markInputAsDeprecated(m_ui32SignalChannel, signalIndex);

	return m_pStreamDecoderSignal->getOutputMatrix()->getBuffer();
}

#if 0
void CInputChannel::copyData(const OpenViBE::boolean copyFirstBlock, OpenViBE::uint64 matrixIndex)
{
	OpenViBE::CMatrix*&    l_pMatrixBuffer = m_oMatrixBuffer[matrixIndex & 1];

	OpenViBE::float64*     l_pSrcData = m_pStreamDecoderSignal->getOutputMatrix()->getBuffer() + (copyFirstBlock ? 0 : m_ui64FirstBlock);
	OpenViBE::float64*     l_pDstData = l_pMatrixBuffer->getBuffer()  + (copyFirstBlock ? m_ui64SecondBlock : 0);
	OpenViBE::uint64       l_ui64Size = (copyFirstBlock ? m_ui64FirstBlock : m_ui64SecondBlock)*sizeof(OpenViBE::float64);

	for(OpenViBE::uint64 i=0; i < m_ui64NbChannels; i++, l_pSrcData+=m_ui64NbSamples, l_pDstData+=m_ui64NbSamples)
	{
		System::Memory::copy(l_pDstData, l_pSrcData, size_t (l_ui64Size));
	}
}
#endif
