#if defined(TARGET_HAS_ThirdPartyEIGEN)

#include "ovpCBoxAlgorithmARCoefficients.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace std;

boolean CBoxAlgorithmARCoefficients::initialize(void)
{
	// Signal stream decoder
	m_oAlgo0_SignalDecoder.initialize(*this,0);

	m_pARBurgMethodAlgorithm=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_ARBurgMethod));
	m_pARBurgMethodAlgorithm->initialize();
		
	ip_pARBurgMethodAlgorithm_Matrix.initialize(m_pARBurgMethodAlgorithm->getInputParameter(OVP_Algorithm_ARBurgMethod_InputParameterId_Matrix));	
	op_pARBurgMethodAlgorithm_Matrix.initialize(m_pARBurgMethodAlgorithm->getOutputParameter(OVP_Algorithm_ARBurgMethod_OutputParameterId_Matrix));

	ip_ui64ARBurgMethodAlgorithm_Order.initialize(m_pARBurgMethodAlgorithm->getInputParameter(OVP_Algorithm_ARBurgMethod_InputParameterId_UInteger));
	ip_ui64ARBurgMethodAlgorithm_Order = (uint64)FSettingValueAutoCast(*this->getBoxAlgorithmContext(),0);

	// Feature vector stream encoder
	m_oAlgo1_Encoder.initialize(*this,0);

	// The AR Burg's Method algorithm will take the matrix coming from the signal decoder:
	ip_pARBurgMethodAlgorithm_Matrix.setReferenceTarget(m_oAlgo0_SignalDecoder.getOutputMatrix());

	// The feature vector encoder will take the matrix from the AR Burg's Method algorithm:
	m_oAlgo1_Encoder.getInputMatrix().setReferenceTarget(op_pARBurgMethodAlgorithm_Matrix);

	
	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmARCoefficients::uninitialize(void)
{
	m_oAlgo0_SignalDecoder.uninitialize();

	
	ip_pARBurgMethodAlgorithm_Matrix.uninitialize();
	ip_ui64ARBurgMethodAlgorithm_Order.uninitialize();
	op_pARBurgMethodAlgorithm_Matrix.uninitialize();

	m_pARBurgMethodAlgorithm->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pARBurgMethodAlgorithm);

	m_oAlgo1_Encoder.uninitialize();

	return true;
}

/*******************************************************************************/


boolean CBoxAlgorithmARCoefficients::processInput(uint32 ui32InputIndex)
{

	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	
	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmARCoefficients::process(void)
{
	
	// the static box context describes the box inputs, outputs, settings structures
	// TODO:is necessary next line ?
	//IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	
	// we decode the input signal chunks
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0);i++)
	{
		m_oAlgo0_SignalDecoder.decode(i);

		if(m_oAlgo0_SignalDecoder.isHeaderReceived())
		{
			
			// Header received
			m_pARBurgMethodAlgorithm->process(OVP_Algorithm_ARBurgMethod_InputTriggerId_Initialize);
			
			// Make sure the algo initialization was successful			
			if(!m_pARBurgMethodAlgorithm->process(OVP_Algorithm_ARBurgMethod_InputTriggerId_Initialize))
			{
				this->getLogManager() << LogLevel_Error << "Initialization was unsuccessful\n";
				return false;
			}

			// Pass the header to the next boxes, by encoding a header on the output 0:	
			m_oAlgo1_Encoder.encodeHeader();

			// send the output chunk containing the header. The dates are the same as the input chunk:
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}


		if(m_oAlgo0_SignalDecoder.isBufferReceived())
		{
			// we process the signal matrix with our algorithm
			m_pARBurgMethodAlgorithm->process(OVP_Algorithm_ARBurgMethod_InputTriggerId_Process);
 
			// If the process is done successfully, we can encode the buffer
			if(m_pARBurgMethodAlgorithm->isOutputTriggerActive(OVP_Algorithm_ARBurgMethod_OutputTriggerId_ProcessDone))
			{
				// Encode the output buffer :        		    	
				m_oAlgo1_Encoder.encodeBuffer();

				// and send it to the next boxes :
				l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i),
																l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			}
			
		}
		if(m_oAlgo0_SignalDecoder.isEndReceived())
		{
			// End of stream received. This happens only once when pressing "stop". Just pass it to the next boxes so they receive the message :
			m_oAlgo1_Encoder.encodeEnd();
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));	

		}
	}
	
	return true;
}

#endif // #if defined(TARGET_HAS_ThirdPartyEIGEN)
