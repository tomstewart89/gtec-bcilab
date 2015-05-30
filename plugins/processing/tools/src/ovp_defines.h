#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
//                                                                   //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_BoxAlgorithm_LatencyEvaluation          OpenViBE::CIdentifier(0x0AD11EC1, 0x7EF3690B)
#define OVP_ClassId_BoxAlgorithm_LatencyEvaluationDesc      OpenViBE::CIdentifier(0x5DB56A54, 0x5380262B)

#define OVP_ClassId_BoxAlgorithm_MouseTracking OpenViBE::CIdentifier(0x1E386EE5, 0x203E13C6)
#define OVP_ClassId_BoxAlgorithm_MouseTrackingDesc OpenViBE::CIdentifier(0x7A31C11B, 0xF522262E)

//___________________________________________________________________//
//                                                                   //
// Global defines                                                   //
//___________________________________________________________________//
//                                                                   //

#ifdef TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines
 #include "ovp_global_defines.h"
#endif // TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines

//___________________________________________________________________//


#endif // __OpenViBEPlugins_Defines_H__
