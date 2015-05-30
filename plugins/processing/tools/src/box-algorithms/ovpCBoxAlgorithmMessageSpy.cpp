#include "ovpCBoxAlgorithmMessageSpy.h"
#include <sstream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Tools;

boolean CBoxAlgorithmMessageSpy::initialize(void)
{
	uint64  l_ui64LogLevel=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_eLogLevel=static_cast<ELogLevel>(l_ui64LogLevel);

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmMessageSpy::uninitialize(void)
{
	return true;
}
/*******************************************************************************/


boolean CBoxAlgorithmMessageSpy::processClock(IMessageClock& rMessageClock)
{
	// some pre-processing code if needed...

	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
/*******************************************************************************/


uint64 CBoxAlgorithmMessageSpy::getClockFrequency(void)
{
	// Note that the time is coded on a 64 bits unsigned integer, fixed decimal point (32:32)
	return 1LL<<32; // the box clock frequency
}
/*******************************************************************************/

OpenViBE::boolean CBoxAlgorithmMessageSpy::processMessage(const IMessageWithData& msg, uint32 inputIndex)
{
	bool l_bReceivedSomething = false;
	const CString* l_sKey;

	this->getLogManager() << m_eLogLevel << "Spying message input socket " << inputIndex << "\n";
	this->getLogManager() << m_eLogLevel << "Message time stamp is "<< msg.getTime() << " and id " << msg.getIdentifier() << "\n";
	
	CString l_sMessageContent;

	// for each type, get the first key and then loop over all the available keys 

	l_sKey = msg.getFirstCStringToken();
	if(l_sKey)
	{
		l_bReceivedSomething = true;

		while(l_sKey!=NULL)
		{
			const CString* l_sValue;
			
			bool success = msg.getValueCString(*l_sKey, &l_sValue);
			l_sMessageContent = l_sMessageContent + CString("[String] ") + (*l_sKey) + CString(" = ") + (success ? *l_sValue : "NOTFOUND") + CString("\n");
			l_sKey = msg.getNextCStringToken(*l_sKey);
		}
	}

	l_sKey = msg.getFirstUInt64Token();
	if(l_sKey)
	{
		l_bReceivedSomething = true;

		while(l_sKey!=NULL)
		{
			uint64 l_ui64Value = 0;
			
			bool success = msg.getValueUint64(*l_sKey, l_ui64Value);
			//easier way to convert uint -> char* -> CString ?
			std::stringstream l_oStream;
			l_oStream << l_ui64Value;
			l_sMessageContent = l_sMessageContent+CString("[Uint64] ")+ (*l_sKey) + CString(" = ") + 
				(success ? CString(l_oStream.str().c_str()) : "NOTFOUND") + CString("\n");
			l_sKey = msg.getNextUInt64Token(*l_sKey);
		}
	}

	l_sKey = msg.getFirstFloat64Token();
	if(l_sKey)
	{
		l_bReceivedSomething = true;

		while(l_sKey!=NULL)
		{
			float64 l_f64Value = 0;
			
			bool success = msg.getValueFloat64(*l_sKey, l_f64Value);
			//easier way to convert Float -> char* -> CString ?
			std::stringstream l_oStream;
			l_oStream << l_f64Value;
			l_sMessageContent = l_sMessageContent + CString("[Float64] ") + (*l_sKey) + CString(" = ") + 
				(success ? CString(l_oStream.str().c_str()) : "NOTFOUND") + CString("\n");
			l_sKey = msg.getNextFloat64Token(*l_sKey);
		}
	}


	l_sKey = msg.getFirstIMatrixToken();
	if(l_sKey)
	{
		l_bReceivedSomething = true;

		while(l_sKey!=NULL)
		{
			const IMatrix* l_oMatrix;
			bool success = msg.getValueIMatrix(*l_sKey, &l_oMatrix);
			//easier way to convert Float -> char* -> CString ?
			std::stringstream l_oStream;

			if(success) 
			{
				l_oStream << "Dims [";
				for (uint32 i=0; i<l_oMatrix->getDimensionCount();i++) {
					l_oStream << " " << l_oMatrix->getDimensionSize(i);
				}
				l_oStream << " ] Data [";

				const float64* l_f64Buffer = l_oMatrix->getBuffer();
				const uint32 l_ui32BufferSize = l_oMatrix->getBufferElementCount();
				for (uint32 i=0; i<l_ui32BufferSize; i++)
				{
					l_oStream << " " << l_f64Buffer[i];
				}
				l_oStream << " ]";
			} 
			else
			{
				l_oStream << "NOTFOUND";
			}

			l_sMessageContent = l_sMessageContent+CString("[Matrix] ")+(*l_sKey)+CString(" = ")+CString(l_oStream.str().c_str())+CString("\n");
			l_sKey = msg.getNextIMatrixToken(*l_sKey);
		}
	}

	if(l_bReceivedSomething) 
	{
		getLogManager() << m_eLogLevel << "Extracted contents are\n" << l_sMessageContent;
	}
	else
	{
		getLogManager() << m_eLogLevel << "The message appeared to be empty\n";
	}

	return true;
}




/*******************************************************************************/

boolean CBoxAlgorithmMessageSpy::process(void)
{
	// The work of this box is done inside processMessage()

	return true;
}
