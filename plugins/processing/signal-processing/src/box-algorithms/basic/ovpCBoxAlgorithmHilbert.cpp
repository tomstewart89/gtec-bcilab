#include "ovpCBoxAlgorithmHilbert.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessingBasic;

boolean CBoxAlgorithmHilbert::initialize(void)
{
	// Signal stream decoder
	m_oAlgo0_SignalDecoder.initialize(*this,0);
	// Signal stream encoder
	m_oAlgo1_SignalEncoder.initialize(*this,0);
	m_oAlgo2_SignalEncoder.initialize(*this,1);
	m_oAlgo3_SignalEncoder.initialize(*this,2);
	
	m_pHilbertAlgo = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_HilbertTransform));
	m_pHilbertAlgo->initialize();

	ip_pSignal_Matrix.initialize(m_pHilbertAlgo->getInputParameter(OVP_Algorithm_HilbertTransform_InputParameterId_Matrix));
	op_pHilbert_Matrix.initialize(m_pHilbertAlgo->getOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_HilbertMatrix));
	op_pEnvelope_Matrix.initialize(m_pHilbertAlgo->getOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_EnvelopeMatrix));
	op_pPhase_Matrix.initialize(m_pHilbertAlgo->getOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_PhaseMatrix));

	ip_pSignal_Matrix.setReferenceTarget(m_oAlgo0_SignalDecoder.getOutputMatrix());

	m_oAlgo1_SignalEncoder.getInputSamplingRate().setReferenceTarget(m_oAlgo0_SignalDecoder.getOutputSamplingRate());
	m_oAlgo2_SignalEncoder.getInputSamplingRate().setReferenceTarget(m_oAlgo0_SignalDecoder.getOutputSamplingRate());
	m_oAlgo3_SignalEncoder.getInputSamplingRate().setReferenceTarget(m_oAlgo0_SignalDecoder.getOutputSamplingRate());

	m_oAlgo1_SignalEncoder.getInputMatrix().setReferenceTarget(op_pHilbert_Matrix);
	m_oAlgo2_SignalEncoder.getInputMatrix().setReferenceTarget(op_pEnvelope_Matrix);
	m_oAlgo3_SignalEncoder.getInputMatrix().setReferenceTarget(op_pPhase_Matrix);

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmHilbert::uninitialize(void)
{
	m_oAlgo0_SignalDecoder.uninitialize();
	m_oAlgo1_SignalEncoder.uninitialize();
	m_oAlgo2_SignalEncoder.uninitialize();
	m_oAlgo3_SignalEncoder.uninitialize();

	ip_pSignal_Matrix.uninitialize();
	op_pHilbert_Matrix.uninitialize();
	op_pEnvelope_Matrix.uninitialize();
	op_pPhase_Matrix.uninitialize();

	m_pHilbertAlgo->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pHilbertAlgo);

	return true;
}
/*******************************************************************************/


boolean CBoxAlgorithmHilbert::processInput(uint32 ui32InputIndex)
{
	// some pre-processing code if needed...

	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmHilbert::process(void)
{
	
	// the static box context describes the box inputs, outputs, settings structures
	//IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	//iterate over all chunk on input 0
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		// decode the chunk i on input 0
		m_oAlgo0_SignalDecoder.decode(i);
		// the decoder may have decoded 3 different parts : the header, a buffer or the end of stream.
		if(m_oAlgo0_SignalDecoder.isHeaderReceived())
		{
			// Header received
			m_pHilbertAlgo->process(OVP_Algorithm_HilbertTransform_InputTriggerId_Initialize);
			
			// Pass the header to the next boxes, by encoding a header on the output 0:
			m_oAlgo1_SignalEncoder.encodeHeader();
			m_oAlgo2_SignalEncoder.encodeHeader();
			m_oAlgo3_SignalEncoder.encodeHeader();

			// send the output chunk containing the header. The dates are the same as the input chunk:
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			l_rDynamicBoxContext.markOutputAsReadyToSend(2, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}
		if(m_oAlgo0_SignalDecoder.isBufferReceived())
		{

			m_pHilbertAlgo->process(OVP_Algorithm_HilbertTransform_InputTriggerId_Process);

			// Encode the output buffer :
			m_oAlgo1_SignalEncoder.encodeBuffer();
			m_oAlgo2_SignalEncoder.encodeBuffer();
			m_oAlgo3_SignalEncoder.encodeBuffer();

			// and send it to the next boxes :
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			l_rDynamicBoxContext.markOutputAsReadyToSend(2, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}
		if(m_oAlgo0_SignalDecoder.isEndReceived())
		{
			// End of stream received. This happens only once when pressing "stop". Just pass it to the next boxes so they receive the message :
			m_oAlgo1_SignalEncoder.encodeEnd();
			m_oAlgo2_SignalEncoder.encodeEnd();
			m_oAlgo3_SignalEncoder.encodeEnd();

			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			l_rDynamicBoxContext.markOutputAsReadyToSend(2, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}

		// The current input chunk has been processed, and automaticcaly discarded.
		// you don't need to call "l_rDynamicBoxContext.markInputAsDeprecated(0, i);"
	}


	return true;
}
