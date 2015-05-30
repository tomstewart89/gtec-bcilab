#ifndef __OpenViBEPlugins_Streaming_InputChannel_H__
#define __OpenViBEPlugins_Streaming_InputChannel_H__

// @author Gipsa-lab

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

/**
	Use this class to receive send and stimulations channels
*/

namespace OpenViBEPlugins
{

	namespace SignalProcessing
	{
		class CInputChannel
		{
		private:
			typedef enum
			{	SIGNAL_CHANNEL,
				STIMULATION_CHANNEL,
				NB_CHANNELS,
			} channel_t;
						
		public:

			CInputChannel(const OpenViBE::uint16 ui16InputIndex = 0);
			~CInputChannel();
			OpenViBE::boolean initialize(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>* pTBoxAlgorithm);
			OpenViBE::boolean uninitialize();

			OpenViBE::boolean			isLastChannel(const OpenViBE::uint32 ui32InputIndex) { return ui32InputIndex == m_ui32StimulationChannel; }
			OpenViBE::boolean			isWorking() { return m_bIsWorking; }
			OpenViBE::boolean           waitForSignalHeader();
			OpenViBE::uint32            getNbOfStimulationBuffers();
			OpenViBE::uint32            getNbOfSignalBuffers();
			OpenViBE::IStimulationSet*  getStimulation(OpenViBE::uint64& startTimestamp, OpenViBE::uint64& endTimestamp, const OpenViBE::uint32 stimulationIndex);
			OpenViBE::IStimulationSet*  discardStimulation(const OpenViBE::uint32 stimulationIndex);
			OpenViBE::float64*          getSignal(OpenViBE::uint64& startTimestamp, OpenViBE::uint64& endTimestamp, const OpenViBE::uint32 signalIndex);
			OpenViBE::float64*          discardSignal(const OpenViBE::uint32 signalIndex);
			OpenViBE::uint64            getSamplingRate() const {return m_pStreamDecoderSignal->getOutputSamplingRate();}
			OpenViBE::uint64            getNbOfChannels() const {return m_pStreamDecoderSignal->getOutputMatrix()->getDimensionSize(0);}
			OpenViBE::uint64            getNbOfSamples() const {return m_pStreamDecoderSignal->getOutputMatrix()->getDimensionSize(1);}
			OpenViBE::uint64            getStartTimestamp() const {return m_ui64StartTimestamp;}
			OpenViBE::uint64            getEndTimestamp() const {return m_ui64EndTimestamp;}
			const char*                 getChannelName(OpenViBE::uint32 index) { return m_pStreamDecoderSignal->getOutputMatrix()->getDimensionLabel(0,index); }
			const OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* >& getOpMatrix() const {return m_pStreamDecoderSignal->getOutputMatrix();}
		
		protected:
			OpenViBE::uint32														m_ui32SignalChannel;
			OpenViBE::uint32														m_ui32StimulationChannel;
			OpenViBE::boolean                                                       m_bIsWorking;

			OpenViBE::uint64                                                        m_ui64StartTimestamp;
			OpenViBE::uint64                                                        m_ui64EndTimestamp;
		
			OpenViBE::IStimulationSet*                                              m_oIStimulationSet;

			// parent memory
			OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm>*     m_pTBoxAlgorithm;
			
			// signal section
			//OpenViBE::Kernel::IAlgorithmProxy*                                      m_pStreamDecoderSignal;
			OpenViBEToolkit::TSignalDecoder < OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm> >*	m_pStreamDecoderSignal;

			// stimulation section
			OpenViBEToolkit::TStimulationDecoder < OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm> >*		m_pStreamDecoderStimulation;

		};
	};
};

#endif // __OpenViBEPlugins_InputChannel_H__
