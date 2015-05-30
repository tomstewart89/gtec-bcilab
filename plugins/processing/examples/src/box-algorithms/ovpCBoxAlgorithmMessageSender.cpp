#include "ovpCBoxAlgorithmMessageSender.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Examples;

boolean CBoxAlgorithmMessageSender::initialize(void)
{
	//get box clock frequency as defined by the user
	m_ui64BoxFrequency = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_bAppendTestMatrix = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	uint32 l_ui32NumberOfSettings = l_rStaticBoxContext.getSettingCount();
	//get the other settings, they will fill the message (except the setting 0 and 1 which are hardcoded)
	// only Integer, Float and String settings are allowed (no CMatrix setting available)
	for (uint32 i=2; i<l_ui32NumberOfSettings; i++)
	{
		CString l_sSettingName;
		CIdentifier l_oSettingId;
		l_rStaticBoxContext.getSettingName(i, l_sSettingName);
		l_rStaticBoxContext.getSettingType(i, l_oSettingId);

		boolean l_bIsInteger = (l_oSettingId==OV_TypeId_Integer);
		boolean l_bIsFloat = (l_oSettingId==OV_TypeId_Float);
		boolean l_bIsString = (l_oSettingId==OV_TypeId_String);

		if (l_bIsInteger)
		{
			uint64 l_ui64Value = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
			m_oIntegers[l_sSettingName] = l_ui64Value;
		}
		else if (l_bIsFloat)
		{
			float64 l_f64Value = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
			m_oFloats[l_sSettingName] = l_f64Value;
		}
		else if (l_bIsString)
		{
			CString l_sValue = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
			m_oStrings[l_sSettingName] = l_sValue;
		}
	}

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmMessageSender::uninitialize(void)
{

	return true;
}
/*******************************************************************************/


boolean CBoxAlgorithmMessageSender::processClock(IMessageClock& rMessageClock)
{
	// some pre-processing code if needed...

	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
/*******************************************************************************/


uint64 CBoxAlgorithmMessageSender::getClockFrequency(void)
{
	// Note that the time is coded on a 64 bits unsigned integer, fixed decimal point (32:32)
	return m_ui64BoxFrequency<<32; // the box clock frequency
}
/*******************************************************************************/


boolean CBoxAlgorithmMessageSender::process(void)
{
	// the static box context describes the box inputs, outputs, settings structures
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	//create the message
	IMessageWithData& msg = this->getPlayerContext().createMessage();

	//set optional parent class params; setTime() may not be meaningful in general use as the message should be processed immediately during sendMessage().
	msg.setIdentifier(CIdentifier(0x01234567,0x89abcdef));
	msg.setTime(getPlayerContext().getCurrentTime());

	//put the integers in the message
	std::map<CString, uint64>::const_iterator l_oIntegerIterator;
	for (l_oIntegerIterator=m_oIntegers.begin(); l_oIntegerIterator!=m_oIntegers.end(); l_oIntegerIterator++)
	{
		msg.setValueUint64(l_oIntegerIterator->first, l_oIntegerIterator->second);
	}

	//put the floats
	std::map<CString, float64>::const_iterator l_oFloatIterator;
	for (l_oFloatIterator=m_oFloats.begin(); l_oFloatIterator!=m_oFloats.end(); l_oFloatIterator++)
	{
		msg.setValueFloat64(l_oFloatIterator->first, l_oFloatIterator->second);
	}

	//put the strings
	std::map<CString, CString>::const_iterator l_oStringIterator;
	for (l_oStringIterator=m_oStrings.begin(); l_oStringIterator!=m_oStrings.end(); l_oStringIterator++)
	{
		msg.setValueCString(l_oStringIterator->first, l_oStringIterator->second);
	}

	//adding a matrix to the message to test if it is received correctly
	if(m_bAppendTestMatrix) 
	{
		CMatrix l_oMatrix;
		l_oMatrix.setDimensionCount(3);
		l_oMatrix.setDimensionSize(0,2);
		l_oMatrix.setDimensionSize(1,3);
		l_oMatrix.setDimensionSize(2,2);

		float64* l_f64Buffer = l_oMatrix.getBuffer();

		for (uint64 i=0; i<l_oMatrix.getBufferElementCount(); i++)
		{
			l_f64Buffer[i] = (float64)i;
		}
		msg.setValueIMatrix( CString("Matrix"), l_oMatrix);
	}

	//send the message to all available outputs
	for (uint32 output = 0; output<l_rStaticBoxContext.getMessageOutputCount(); output++)
	{
		getLogManager() << OpenViBE::Kernel::LogLevel_Trace << "sending message on output " << output << "\n";
		this->getPlayerContext().sendMessage(msg, output);
	}

	return true;
}
