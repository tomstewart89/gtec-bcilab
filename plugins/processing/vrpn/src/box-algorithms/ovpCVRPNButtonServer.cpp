#if defined TARGET_HAS_ThirdPartyVRPN

#include "ovpCVRPNButtonServer.h"
#include "../ovpIVRPNServerManager.h"

using namespace OpenViBE;
using namespace OpenViBE::Plugins;
using namespace OpenViBE::Kernel;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::VRPN;
using namespace OpenViBEToolkit;
using namespace std;

CVRPNButtonServer::CVRPNButtonServer()
	:m_ui32CurrentInput(0)
{
}

boolean CVRPNButtonServer::initialize()
{
	const IBox * l_pBox=getBoxAlgorithmContext()->getStaticBoxContext();


	//get server name, and creates a button server for this server
	CString l_oServerName;
	l_pBox->getSettingValue(0, l_oServerName);

	IVRPNServerManager::getInstance().initialize();
	IVRPNServerManager::getInstance().addServer(l_oServerName, m_oServerIdentifier);
	IVRPNServerManager::getInstance().setButtonCount(m_oServerIdentifier, l_pBox->getInputCount());

	m_vStimulationDecoders.resize(l_pBox->getInputCount());

	//get stim id
	for(uint32 i=0; i<l_pBox->getInputCount(); i++)
	{
		CString l_sOnStimulationIdentifier;
		CString l_sOffStimulationIdentifier;
		l_pBox->getSettingValue(1+i*2, l_sOnStimulationIdentifier);
		l_pBox->getSettingValue(2+i*2, l_sOffStimulationIdentifier);
		m_vStimulationPair[i]=
			pair<uint64, uint64>(
				getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_Stimulation, l_sOnStimulationIdentifier),
				getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_Stimulation, l_sOffStimulationIdentifier));

		m_vStimulationDecoders[i] = new OpenViBEToolkit::TStimulationDecoder < CVRPNButtonServer >;
		m_vStimulationDecoders[i]->initialize(*this,i);
	}

	return true;
}

boolean CVRPNButtonServer::uninitialize()
{
	for(size_t i=0;i<m_vStimulationDecoders.size();i++) {
		delete m_vStimulationDecoders[i];
	}
	m_vStimulationDecoders.clear();

	IVRPNServerManager::getInstance().uninitialize();

	return true;
}

boolean CVRPNButtonServer::processClock(IMessageClock& rMessageClock)
{
	IVRPNServerManager::getInstance().process();
	return true;
}

boolean CVRPNButtonServer::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CVRPNButtonServer::process()
{
	IBoxIO * l_pBoxIO=getBoxAlgorithmContext()->getDynamicBoxContext();

	for(uint32 input=0 ; input<getBoxAlgorithmContext()->getStaticBoxContext()->getInputCount() ; input++)
	{
		m_ui32CurrentInput = input;

		for(uint32 chunk=0; chunk<l_pBoxIO->getInputChunkCount(input); chunk++)
		{
			m_vStimulationDecoders[input]->decode(chunk);

			if(m_vStimulationDecoders[input]->isBufferReceived())
			{
				const IStimulationSet* l_oStimSet = m_vStimulationDecoders[input]->getOutputStimulationSet();
				for(uint32 s=0;s<l_oStimSet->getStimulationCount();s++)
				{
					setStimulation(s,l_oStimSet->getStimulationIdentifier(s), l_oStimSet->getStimulationDate(s));
				}
			}
		}
	}

	return true;
}

void CVRPNButtonServer::setStimulation(const uint32 ui32StimulationIndex, const uint64 ui64StimulationIdentifier, const uint64 ui64StimulationDate)
{
	pair < uint64, uint64 > l_oStimulationPair=m_vStimulationPair[m_ui32CurrentInput];

	if(l_oStimulationPair.first==l_oStimulationPair.second)
	{
		if(l_oStimulationPair.first==ui64StimulationIdentifier)
		{
			getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Received TOGGLE stimulation for button " << m_ui32CurrentInput << " (" << ui64StimulationIdentifier << ")\n";
			IVRPNServerManager::getInstance().setButtonState(m_oServerIdentifier, m_ui32CurrentInput, !IVRPNServerManager::getInstance().getButtonState(m_oServerIdentifier, m_ui32CurrentInput));
			IVRPNServerManager::getInstance().reportButton(m_oServerIdentifier);
		}
	}
	else
	{
		if(l_oStimulationPair.first==ui64StimulationIdentifier)
		{
			getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Received ON stimulation for button " << m_ui32CurrentInput << " (" << ui64StimulationIdentifier << ")\n";
			IVRPNServerManager::getInstance().setButtonState(m_oServerIdentifier, m_ui32CurrentInput, true);
			IVRPNServerManager::getInstance().reportButton(m_oServerIdentifier);
		}
		if(l_oStimulationPair.second==ui64StimulationIdentifier)
		{
			getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Received OFF stimulation for button " << m_ui32CurrentInput << " (" << ui64StimulationIdentifier << ")\n";
			IVRPNServerManager::getInstance().setButtonState(m_oServerIdentifier, m_ui32CurrentInput, false);
			IVRPNServerManager::getInstance().reportButton(m_oServerIdentifier);
		}
	}
}

#endif // OVP_HAS_Vrpn
