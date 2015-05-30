#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

#define OVP_Classification_BoxTrainerXMLVersion								3

//___________________________________________________________________//
//                                                                   //
// Plugin Object Descriptor Class Identifiers                        //
//___________________________________________________________________//
//                                                                   //

#define OVP_Algorithm_ClassifierPLDA_InputParameterId_Shrinkage		OpenViBE::CIdentifier(0x7FEFDCA9, 0x816ED903) 
#define OVP_Algorithm_ClassifierPLDA_InputParameterId_Lambda		OpenViBE::CIdentifier(0xEBAEB213, 0xDD4735A0)
#define OVP_Algorithm_ClassifierPLDA_InputParameterId_BufferSize	OpenViBE::CIdentifier(0xB083614E, 0x26C6B4BD)
#define OVP_TypeId_ShrinkageType					OpenViBE::CIdentifier(0x344A52F5, 0x489DB439)
enum { FULL=0, DIAG=1, SHRINK_TO_DIAG=2, SHRINK_TO_UNITY=3 };
//___________________________________________________________________//
//                                                                   //
// Plugin Object Class Identifiers                                   //
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


#define OVP_TypeId_ClassificationPairwiseStrategy							OpenViBE::CIdentifier(0x0DD51C74, 0x3C4E74C9)


extern const char* const c_sXmlVersionAttributeName;
extern const char* const c_sIdentifierAttributeName;

extern const char* const c_sStrategyNodeName;
extern const char* const c_sAlgorithmNodeName;
extern const char* const c_sStimulationsNodeName;
extern const char* const c_sRejectedClassNodeName;
extern const char* const c_sClassStimulationNodeName;

extern const char* const c_sClassificationBoxRoot;
extern const char* const c_sClassifierRoot;

extern const char* const c_sPairwiseStrategyEnumerationName;

bool ov_float_equal(double f64First, double f64Second);

#endif // __OpenViBEPlugins_Defines_H__
