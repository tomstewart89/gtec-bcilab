#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
// Tutorials                                                         //
//___________________________________________________________________//
//                                                                   //


#define OVP_ClassId_BoxAlgorithm_Clock                      OpenViBE::CIdentifier(0x14CB4CFC, 0x6D064CB3)
#define OVP_ClassId_BoxAlgorithm_ClockDesc                  OpenViBE::CIdentifier(0x754C233D, 0x37DF04A3)

#define OVP_ClassId_Log                                     OpenViBE::CIdentifier(0x00BE3E25, 0x274F2075)
#define OVP_ClassId_LogDesc                                 OpenViBE::CIdentifier(0x00780136, 0x57633D46)

//___________________________________________________________________//
//                                                                   //
// Plugin Object Descriptor Class Identifiers                        //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_HelloWorldDesc							OpenViBE::CIdentifier(0x62B739D3, 0x040A3F74)
#define OVP_ClassId_HelloWorldWithInputDesc                 OpenViBE::CIdentifier(0x6E067319, 0x775675F0)

//___________________________________________________________________//
//                                                                   //
// Plugin Object Class Identifiers                                   //
//___________________________________________________________________//
//                                                   //

#define OVP_ClassId_HelloWorld                              OpenViBE::CIdentifier(0x189445E8, 0x57C55950)
#define OVP_ClassId_HelloWorldWithInput                     OpenViBE::CIdentifier(0x657D5545, 0x1CE92FB8)

//___________________________________________________________________//
//                                                                   //
// Unstables                                                         //
//___________________________________________________________________//
//                                                                   //

//___________________________________________________________________//
//                                                                   //
// Global defines                                                   //
//___________________________________________________________________//
//                                                                   //

#ifdef TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines
 #include "ovp_global_defines.h"
#endif // TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines


#endif // __OpenViBEPlugins_Defines_H__
