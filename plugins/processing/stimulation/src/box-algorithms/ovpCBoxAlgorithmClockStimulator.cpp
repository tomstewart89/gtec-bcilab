#include "ovpCBoxAlgorithmClockStimulator.h"
#include <cstdlib>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Stimulation;

uint64 CBoxAlgorithmClockStimulator::getClockFrequency(void)
{
	return (1LL<<32)*32;
}

boolean CBoxAlgorithmClockStimulator::initialize(void)
{
	CString l_sSettingValue;

	float64 l_f64InterstimulationInterval=0;
	getStaticBoxContext().getSettingValue(0, l_sSettingValue);
	l_f64InterstimulationInterval=::atof(l_sSettingValue.toASCIIString());
	
	const float64 l_f64MinInterstimulationInterval= 0.0001;
	if ( l_f64InterstimulationInterval < l_f64MinInterstimulationInterval )
	{
		getLogManager() << OpenViBE::Kernel::LogLevel_Error << ": " 
				<< "Stimulation Interval " << l_f64InterstimulationInterval 
				<<" is too small, them caped to "  <<  l_f64MinInterstimulationInterval <<"\n";
		l_f64InterstimulationInterval=l_f64MinInterstimulationInterval;
	}
	
	m_ui64InterstimulationInterval=(uint64)(l_f64InterstimulationInterval*(1LL<<32));

	getStaticBoxContext().getSettingValue(1, l_sSettingValue);
	m_ui64StimulationId=getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation, l_sSettingValue);

	m_ui64LastStimulationDate=0;
	m_ui64LastEndTime=0;

	m_oStimulationEncoder.initialize(*this, 0);

	return true;
}

boolean CBoxAlgorithmClockStimulator::uninitialize(void)
{
	m_oStimulationEncoder.uninitialize();

	return true;
}

boolean CBoxAlgorithmClockStimulator::processClock(IMessageClock& rMessageClock)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

boolean CBoxAlgorithmClockStimulator::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	uint64 l_ui64CurrentTime=getPlayerContext().getCurrentTime();

	CStimulationSet l_oStimulationSet;
	l_oStimulationSet.setStimulationCount(0);

	while(m_ui64LastStimulationDate+m_ui64InterstimulationInterval<l_ui64CurrentTime)
	{
		m_ui64LastStimulationDate+=m_ui64InterstimulationInterval;
		l_oStimulationSet.appendStimulation(m_ui64StimulationId, m_ui64LastStimulationDate, 0);
	}

	if(l_ui64CurrentTime==0)
	{
		m_oStimulationEncoder.encodeHeader();
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64LastEndTime, m_ui64LastEndTime);
	}

	m_oStimulationEncoder.getInputStimulationSet() = &l_oStimulationSet;

	m_oStimulationEncoder.encodeBuffer();
	l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64LastEndTime, l_ui64CurrentTime);

	m_ui64LastEndTime=l_ui64CurrentTime;

	return true;
}
