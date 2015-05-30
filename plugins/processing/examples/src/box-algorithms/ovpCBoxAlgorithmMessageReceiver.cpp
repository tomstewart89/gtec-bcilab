#include "ovpCBoxAlgorithmMessageReceiver.h"
#include <sstream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Examples;

boolean CBoxAlgorithmMessageReceiver::initialize(void)
{
	m_bMatrixReceived = false;

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmMessageReceiver::uninitialize(void)
{

	return true;
}
/*******************************************************************************/


boolean CBoxAlgorithmMessageReceiver::processClock(IMessageClock& rMessageClock)
{
	// some pre-processing code if needed...

	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
/*******************************************************************************/


uint64 CBoxAlgorithmMessageReceiver::getClockFrequency(void)
{
	// Note that the time is coded on a 64 bits unsigned integer, fixed decimal point (32:32)
	return 1LL<<32; // the box clock frequency
}
/*******************************************************************************/


OpenViBE::boolean CBoxAlgorithmMessageReceiver::processMessage(const IMessageWithData& msg, uint32 inputIndex)
{

	/*
	//test that message sending is impossible from processMessage()
	IMessageWithData& MMM = this->getPlayerContext().createMessage();
	this->getPlayerContext().sendMessage(MMM, 0);
	*/

	getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Reading message input " << inputIndex << "\n";

	bool l_bSuccess;
	
	uint64 l_ui64IntMessage = 0;
	l_bSuccess = msg.getValueUint64("Meaning of life", l_ui64IntMessage);
	if(l_bSuccess) 
	{
		getLogManager() << OpenViBE::Kernel::LogLevel_Info << "The meaning of life is " << l_ui64IntMessage << "\n";
	}
	else
	{
		getLogManager() << OpenViBE::Kernel::LogLevel_Warning << "Key \"Meaning of life\" not found under type Uint64\n";
		getLogManager() << OpenViBE::Kernel::LogLevel_Warning << "The message did not contain the meaning of life.\n";
	}

	float64 l_f64FloatMessage = 0;
	l_bSuccess = msg.getValueFloat64("Pi", l_f64FloatMessage);
	if(l_bSuccess) 
	{
		getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Pi is " << l_f64FloatMessage << "\n";
	}
	else 
	{
		getLogManager() << OpenViBE::Kernel::LogLevel_Warning << "Key \"Pi\" not found under type Float64\n";	
	}

	// Note that the string pointer is no longer valid after processMessage()
	const CString* l_pStringMessage = NULL;
	l_bSuccess =  msg.getValueCString("Quote", &l_pStringMessage);
	if(l_bSuccess) 
	{
		getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Quote is \"" << *l_pStringMessage << "\"\n";
	} 
	else
	{
		getLogManager() << OpenViBE::Kernel::LogLevel_Warning << "Key \"Quote\" not found under type CString\n";	
	}

	// Note that the matrix pointer is no longer valid after processMessage()
	const IMatrix* l_pMatrixMessage = NULL;
	l_bSuccess = msg.getValueIMatrix("Matrix", &l_pMatrixMessage);
	if(l_bSuccess) {
		getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Matrix received, " << l_pMatrixMessage->getBufferElementCount() << " elements\n";

		const float64* l_f64Buffer = l_pMatrixMessage->getBuffer();
		std::stringstream l_sstream;
		for (uint64 i=0; i<l_pMatrixMessage->getBufferElementCount(); i++)
		{
			l_sstream << l_f64Buffer[i] << " ";
		}
		getLogManager() << OpenViBE::Kernel::LogLevel_Info << l_sstream.str().c_str() << "\n";

		// When we want to use the matrix contents for something outside the processMessage() function, we must make your own copy.

		getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Storing the matrix\n";

		l_bSuccess = OpenViBEToolkit::Tools::Matrix::copy(m_oMatrix, *l_pMatrixMessage);

		getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Matrix copy, success: " << l_bSuccess << "\n";

		if(l_bSuccess) 
		{
			m_bMatrixReceived = true;
		}
	} 
	else 
	{
		getLogManager() << OpenViBE::Kernel::LogLevel_Warning << "Key \"Matrix\" not found under type IMatrix.\n";
	}

	return true;
}




boolean CBoxAlgorithmMessageReceiver::process(void)
{
	
	if (m_bMatrixReceived)
	{
		// Print out the matrix we stored before

		const float64* l_f64Buffer = m_oMatrix.getBuffer();
		std::stringstream l_sstream;
		for (uint64 i=0; i<m_oMatrix.getBufferElementCount(); i++)
		{
			l_sstream << l_f64Buffer[i] << " ";
		}
		getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Stored matrix: " << l_sstream.str().c_str() << "\n";

	}
	return true;
}
