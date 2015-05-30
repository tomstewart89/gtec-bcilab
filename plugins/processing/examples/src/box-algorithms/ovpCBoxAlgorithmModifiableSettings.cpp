#include "ovpCBoxAlgorithmModifiableSettings.h"
#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Examples;

boolean CBoxAlgorithmModifiableSettings::initialize(void)
{
	m_ui64LastTime = 0;
	
	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmModifiableSettings::uninitialize(void)
{
	return true;
}
/*******************************************************************************/


uint64 CBoxAlgorithmModifiableSettings::getClockFrequency(void)
{
	// Once every 5 second
	return 	(uint64)( double( (uint64)1<<32 ) * 5);
}

boolean CBoxAlgorithmModifiableSettings::processClock(OpenViBE::Kernel::IMessageClock& /* rMessageClock */)
{
	uint64 l_ui64Time = (uint64)ITimeArithmetics::timeToSeconds(this->getPlayerContext().getCurrentTime());

	if(l_ui64Time-m_ui64LastTime>=5)
	{
		updateSettings();
		//print settings values
		for(uint32 i=0; i<m_vSettingsValue.size(); i++)
		{
			this->getLogManager() << LogLevel_Info << "Setting " << i << " value is " << m_vSettingsValue[i] << "\n";
		}
		this->getLogManager() << LogLevel_Info << "\n";

		m_ui64LastTime = l_ui64Time;
	}
	return true;
}

/*******************************************************************************/

boolean CBoxAlgorithmModifiableSettings::updateSettings()
{
	m_vSettingsValue.clear();
	IBox& l_rStaticBoxContext = this->getStaticBoxContext();
	for(uint32 i=0; i<l_rStaticBoxContext.getSettingCount(); i++)
	{
		CString l_sSettingValue;
		getStaticBoxContext().getSettingValue(i, l_sSettingValue);
		m_vSettingsValue.push_back(l_sSettingValue);
	}
	return true;
}


boolean CBoxAlgorithmModifiableSettings::process(void)
{


	return true;
}
