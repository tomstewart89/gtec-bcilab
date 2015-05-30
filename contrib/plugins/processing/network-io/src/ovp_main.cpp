
#include <vector>
#include <openvibe/ov_all.h>
#include "ovp_defines.h"

#include "box-algorithms/osc-controller/ovpCBoxAlgorithmOSCController.h"

// @BEGIN gipsa
	
#include "box-algorithms/ovpCBoxLSLExportGipsa.h"

// @END gipsa


OVP_Declare_Begin();

	OVP_Declare_New(OpenViBEPlugins::NetworkIO::CBoxAlgorithmOSCControllerDesc);

// @BEGIN gipsa
#if defined TARGET_HAS_ThirdPartyLSL
	OVP_Declare_New(OpenViBEPlugins::NetworkIO::CBoxAlgorithmLSLExportGipsaDesc);

#endif // TARGET_HAS_ThirdPartyLSL

// @END gipsa
		 
OVP_Declare_End();
