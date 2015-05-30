#include "ovp_defines.h"

#include "box-algorithms/ovpCBoxAlgorithmNoiseGenerator.h"
#include "box-algorithms/ovpCSinusSignalGenerator.h"
#include "box-algorithms/ovpCTimeSignalGenerator.h"
#include "box-algorithms/ovpCBoxAlgorithmChannelUnitsGenerator.h"

OVP_Declare_Begin();

	OVP_Declare_New(OpenViBEPlugins::DataGeneration::CNoiseGeneratorDesc);
	OVP_Declare_New(OpenViBEPlugins::DataGeneration::CSinusSignalGeneratorDesc);
	OVP_Declare_New(OpenViBEPlugins::DataGeneration::CTimeSignalGeneratorDesc);
	OVP_Declare_New(OpenViBEPlugins::DataGeneration::CChannelUnitsGeneratorDesc);



OVP_Declare_End();
