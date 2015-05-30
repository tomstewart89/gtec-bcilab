#include "ovpCConnectivityAlgorithm.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEToolkit;
using namespace OpenViBEPlugins;

boolean CConnectivityAlgorithm::initialize(void)
{
	ip_pSignal1.initialize(this->getInputParameter(OVP_Algorithm_Connectivity_InputParameterId_InputMatrix1));
	ip_pSignal2.initialize(this->getInputParameter(OVP_Algorithm_Connectivity_InputParameterId_InputMatrix2));
	ip_ui64SamplingRate1.initialize(this->getInputParameter(OVP_Algorithm_Connectivity_InputParameterId_ui64SamplingRate1));
	ip_ui64SamplingRate2.initialize(this->getInputParameter(OVP_Algorithm_Connectivity_InputParameterId_ui64SamplingRate2));
	ip_pChannelPairs.initialize(this->getInputParameter(OVP_Algorithm_Connectivity_InputParameterId_LookupMatrix));
	op_pMatrix.initialize(this->getInputParameter(OVP_Algorithm_Connectivity_OutputParameterId_OutputMatrix));

	return true;
}


boolean CConnectivityAlgorithm::uninitialize(void)
{
	ip_pSignal1.uninitialize();
	ip_pSignal2.uninitialize();
	ip_ui64SamplingRate1.uninitialize();
	ip_ui64SamplingRate2.uninitialize();
	ip_pChannelPairs.uninitialize();
	op_pMatrix.uninitialize();

	return true;
}

boolean CConnectivityAlgorithm::process(void)
{

	if(this->isInputTriggerActive(OVP_Algorithm_Connectivity_InputTriggerId_Initialize))
	{
		IMatrix* l_pChannelPairs = ip_pChannelPairs;
		if(!l_pChannelPairs)
		{
			this->getLogManager() << LogLevel_ImportantWarning << "Channel lookup matrix is NULL\n Channel pairs selection failed\n";
			return false;
		}
		return true;
	}

	return true;
}
