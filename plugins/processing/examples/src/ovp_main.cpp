#include "ovp_defines.h"

#include "box-algorithms/ovpCHelloWorld.h"
#include "box-algorithms/ovpCHelloWorldWithInput.h"
#include "box-algorithms/ovpCBoxAlgorithmModifiableSettings.h"
#include "box-algorithms/ovpCBoxAlgorithmMessageSender.h"
#include "box-algorithms/ovpCBoxAlgorithmMessageReceiver.h"

#include "box-algorithms/ovpCLog.h"
#include "box-algorithms/ovpCBoxAlgorithmNothing.h"
#include "box-algorithms/ovpCBoxAlgorithmClock.h"

OVP_Declare_Begin();

	OVP_Declare_New(OpenViBEPlugins::Examples::CHelloWorldDesc);
	OVP_Declare_New(OpenViBEPlugins::Examples::CHelloWorldWithInputDesc);
	OVP_Declare_New(OpenViBEPlugins::Examples::CBoxAlgorithmModifiableSettingsDesc);
	OVP_Declare_New(OpenViBEPlugins::Examples::CBoxAlgorithmMessageSenderDesc);
    OVP_Declare_New(OpenViBEPlugins::Examples::CBoxAlgorithmMessageReceiverDesc);

	OVP_Declare_New(OpenViBEPlugins::Examples::CLogDesc);
	OVP_Declare_New(OpenViBEPlugins::Examples::CBoxAlgorithmNothingDesc);
	OVP_Declare_New(OpenViBEPlugins::Examples::CBoxAlgorithmClockDesc);

OVP_Declare_End();
