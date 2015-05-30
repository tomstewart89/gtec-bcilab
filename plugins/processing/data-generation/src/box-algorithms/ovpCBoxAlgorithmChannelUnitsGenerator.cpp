/*
 * Generates a channel units stream with user-specified unit and factor
 */ 
#include "ovpCBoxAlgorithmChannelUnitsGenerator.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEPlugins::DataGeneration;

boolean CChannelUnitsGenerator::initialize(void)
{
	m_bHeaderSent = false;

	m_ui64ChannelCount=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0); 
	m_ui64Unit=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_ui64Factor=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);

	m_oEncoder.initialize(*this,0);

	return true;
}

boolean CChannelUnitsGenerator::uninitialize(void)
{
	m_oEncoder.uninitialize();

	return true;
}

void CChannelUnitsGenerator::release(void)
{
	delete this;
}

boolean CChannelUnitsGenerator::processClock(OpenViBE::Kernel::IMessageClock& /* rMessageClock */)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CChannelUnitsGenerator::process(void)
{
	IBoxIO* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();

	if(!m_bHeaderSent)
	{	
		IMatrix* l_pUnits = m_oEncoder.getInputMatrix();
		l_pUnits->setDimensionCount(2);
		l_pUnits->setDimensionSize(0,static_cast<uint32>(m_ui64ChannelCount));
		l_pUnits->setDimensionSize(1,2);
		l_pUnits->setDimensionLabel(1,0, "Unit");
		l_pUnits->setDimensionLabel(1,1, "Factor");

		for(uint32 i=0;i<static_cast<uint32>(m_ui64ChannelCount);i++) 
		{
			l_pUnits->getBuffer()[i*2+0] = static_cast<float64>(m_ui64Unit);
			l_pUnits->getBuffer()[i*2+1] = static_cast<float64>(m_ui64Factor);
			
			char buffer[512];
			sprintf(buffer, "Channel %d", i+1);
			l_pUnits->setDimensionLabel(0,i, buffer);
		}

		m_oEncoder.encodeHeader();
		l_pDynamicBoxContext->markOutputAsReadyToSend(0, 0, 0);
		m_oEncoder.encodeBuffer();

		l_pDynamicBoxContext->markOutputAsReadyToSend(0, 0, 0);
		
		m_bHeaderSent = true;
	}

	return true;
}

