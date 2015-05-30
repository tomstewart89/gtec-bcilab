#include "ovp_defines.h"

#include "box-algorithms/ovpCBoxAlgorithmEBMLStreamSpy.h"
#include "box-algorithms/ovpCBoxAlgorithmStimulationListener.h"

// #include "box-algorithms/ovpCBoxAlgorithmLatencyEvaluation.h"
#include "box-algorithms/ovpCBoxAlgorithmMatrixValidityChecker.h"

#include "box-algorithms/ovpCBoxAlgorithmMessageSpy.h"
#include "box-algorithms/ovpCBoxAlgorithmMouseTracking.h"

OVP_Declare_Begin();

	OVP_Declare_New(OpenViBEPlugins::Tools::CBoxAlgorithmStimulationListenerDesc);
	OVP_Declare_New(OpenViBEPlugins::Tools::CBoxAlgorithmEBMLStreamSpyDesc);
// 	OVP_Declare_New(OpenViBEPlugins::Tools::CBoxAlgorithmLatencyEvaluationDesc);
	OVP_Declare_New(OpenViBEPlugins::Tools::CBoxAlgorithmMatrixValidityCheckerDesc);
    OVP_Declare_New(OpenViBEPlugins::Tools::CBoxAlgorithmMessageSpyDesc);
#if defined(TARGET_HAS_ThirdPartyGTK)
    OVP_Declare_New(OpenViBEPlugins::Tools::CBoxAlgorithmMouseTrackingDesc);
#endif
OVP_Declare_End();
