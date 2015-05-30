#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
// Plugin Object Descriptor Class Identifiers                        //
//___________________________________________________________________//
//                                                                   //




#define OVP_ClassId_SinusSignalGeneratorDesc                OpenViBE::CIdentifier(0x2633AFA2, 0x6974E32F)
#define OVP_ClassId_TimeSignalGeneratorDesc                 OpenViBE::CIdentifier(0x57AD8655, 0x1966B4DC)
#define OVP_ClassId_NoiseGeneratorDesc			            OpenViBE::CIdentifier(0x7237458A, 0x1F312C4A)
#define OVP_ClassId_NoiseGenerator			                OpenViBE::CIdentifier(0x0E3929F1, 0x15AF76B9)

//___________________________________________________________________//
//                                                                   //
// Plugin Object Class Identifiers                                   //
//___________________________________________________________________//
//                                                                   //


#define OVP_ClassId_SinusSignalGenerator                    OpenViBE::CIdentifier(0x7E33BDB8, 0x68194A4A)
#define OVP_ClassId_TimeSignalGenerator                     OpenViBE::CIdentifier(0x28A5E7FF, 0x530095DE)


//___________________________________________________________________//
//                                                                   //
// Global defines                                                   //
//___________________________________________________________________//
//                                                                   //

#ifdef TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines
 #include "ovp_global_defines.h"
#endif // TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines



#endif // __OpenViBEPlugins_Defines_H__
