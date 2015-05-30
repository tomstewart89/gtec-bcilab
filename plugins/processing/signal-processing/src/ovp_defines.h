#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
//                                                                   //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_Identity                                                           OpenViBE::CIdentifier(0x5DFFE431, 0x35215C50)
#define OVP_ClassId_IdentityDesc                                                       OpenViBE::CIdentifier(0x54743810, 0x6A1A88CC)

#define OVP_ClassId_TimeBasedEpoching                                                  OpenViBE::CIdentifier(0x00777FA0, 0x5DC3F560)
#define OVP_ClassId_TimeBasedEpochingDesc                                              OpenViBE::CIdentifier(0x00ABDABE, 0x41381683)

#define OVP_ClassId_Algorithm_ApplySpatialFilter                                       OpenViBE::CIdentifier(0xC5DC28FF, 0x2394AFBC)
#define OVP_ClassId_Algorithm_ApplySpatialFilterDesc                                   OpenViBE::CIdentifier(0x0601FABE, 0x85692BFD)
#define OVP_Algorithm_ApplySpatialFilter_InputParameterId_SignalMatrix                 OpenViBE::CIdentifier(0x6FF50741, 0xB9757B1F)
#define OVP_Algorithm_ApplySpatialFilter_InputParameterId_FilterCoefficientsMatrix     OpenViBE::CIdentifier(0x43DC06E5, 0x48E413BA)
#define OVP_Algorithm_ApplySpatialFilter_OutputParameterId_FilteredSignalMatrix        OpenViBE::CIdentifier(0x695B69B2, 0x02DBE696)
#define OVP_Algorithm_ApplySpatialFilter_InputTriggerId_Initialize                     OpenViBE::CIdentifier(0x06D0E2D6, 0xE1E9C082)
#define OVP_Algorithm_ApplySpatialFilter_InputTriggerId_ApplyFilter                    OpenViBE::CIdentifier(0x316BBE5D, 0xBEE0F747)

#define OVP_TypeId_EpochAverageMethod                                                  OpenViBE::CIdentifier(0x6530BDB1, 0xD057BBFE)
#define OVP_TypeId_EpochAverageMethod_MovingAverage                                    OpenViBE::CIdentifier(0x426377E7, 0xCF8E32CD)
#define OVP_TypeId_EpochAverageMethod_MovingAverageImmediate                           OpenViBE::CIdentifier(0x1F830F88, 0xAA01A592)
#define OVP_TypeId_EpochAverageMethod_BlockAverage                                     OpenViBE::CIdentifier(0x2E4ACA75, 0x7E02B507)
#define OVP_TypeId_EpochAverageMethod_CumulativeAverage                                OpenViBE::CIdentifier(0xC18311B7, 0x001C1953)

#define OVP_ClassId_BoxAlgorithm_EpochAverage                                          OpenViBE::CIdentifier(0x21283D9F, 0xE76FF640)
#define OVP_ClassId_BoxAlgorithm_EpochAverageDesc                                      OpenViBE::CIdentifier(0x95F5F43E, 0xBE629D82)
#define OVP_ClassId_Algorithm_MatrixAverage                                            OpenViBE::CIdentifier(0x5E5A6C1C, 0x6F6BEB03)
#define OVP_ClassId_Algorithm_MatrixAverageDesc                                        OpenViBE::CIdentifier(0x1992881F, 0xC938C0F2)
#define OVP_Algorithm_MatrixAverage_InputParameterId_Matrix                            OpenViBE::CIdentifier(0x913E9C3B, 0x8A62F5E3)
#define OVP_Algorithm_MatrixAverage_InputParameterId_MatrixCount                       OpenViBE::CIdentifier(0x08563191, 0xE78BB265)
#define OVP_Algorithm_MatrixAverage_InputParameterId_AveragingMethod                   OpenViBE::CIdentifier(0xE63CD759, 0xB6ECF6B7)
#define OVP_Algorithm_MatrixAverage_OutputParameterId_AveragedMatrix                   OpenViBE::CIdentifier(0x03CE5AE5, 0xBD9031E0)
#define OVP_Algorithm_MatrixAverage_InputTriggerId_Reset                               OpenViBE::CIdentifier(0x670EC053, 0xADFE3F5C)
#define OVP_Algorithm_MatrixAverage_InputTriggerId_FeedMatrix                          OpenViBE::CIdentifier(0x50B6EE87, 0xDC42E660)
#define OVP_Algorithm_MatrixAverage_InputTriggerId_ForceAverage                        OpenViBE::CIdentifier(0xBF597839, 0xCD6039F0)
#define OVP_Algorithm_MatrixAverage_OutputTriggerId_AveragePerformed                   OpenViBE::CIdentifier(0x2BFF029B, 0xD932A613)

#define OVP_ClassId_BoxAlgorithm_StimulationBasedEpoching                              OpenViBE::CIdentifier(0x426163D1, 0x324237B0)
#define OVP_ClassId_BoxAlgorithm_StimulationBasedEpochingDesc                          OpenViBE::CIdentifier(0x4F60616D, 0x468E0A8C)
#define OVP_ClassId_Algorithm_StimulationBasedEpoching                                 OpenViBE::CIdentifier(0x421E3F46, 0x12003E6C)
#define OVP_ClassId_Algorithm_StimulationBasedEpochingDesc                             OpenViBE::CIdentifier(0x2EAF37FC, 0x63195EB8)
#define OVP_Algorithm_StimulationBasedEpoching_InputParameterId_InputSignal            OpenViBE::CIdentifier(0x0ED5C92B, 0xE16BEF25)
#define OVP_Algorithm_StimulationBasedEpoching_InputParameterId_OffsetSampleCount      OpenViBE::CIdentifier(0x7646CE65, 0xE128FC4E)
#define OVP_Algorithm_StimulationBasedEpoching_OutputParameterId_OutputSignal          OpenViBE::CIdentifier(0x00D331A2, 0xC13DF043)
#define OVP_Algorithm_StimulationBasedEpoching_InputTriggerId_Reset                    OpenViBE::CIdentifier(0x6BA44128, 0x418CF901)
#define OVP_Algorithm_StimulationBasedEpoching_InputTriggerId_PerformEpoching          OpenViBE::CIdentifier(0xD05579B5, 0x2649A4B2)
#define OVP_Algorithm_StimulationBasedEpoching_OutputTriggerId_EpochingDone            OpenViBE::CIdentifier(0x755BC3FE, 0x24F7B50F)
#define OVP_Algorithm_StimulationBasedEpoching_InputParameterId_EndTimeChunkToProcess  OpenViBE::CIdentifier(0x8B552604, 0x10CD1F94)

#define OVP_TypeId_CropMethod                                                          OpenViBE::CIdentifier(0xD0643F9E, 0x8E35FE0A)
#define OVP_TypeId_CropMethod_Min                                                      OpenViBE::CIdentifier(0x0CCC9DE4, 0x93F495D2)
#define OVP_TypeId_CropMethod_Max                                                      OpenViBE::CIdentifier(0x2FFEB29C, 0xD8F21FB4)
#define OVP_TypeId_CropMethod_MinMax                                                   OpenViBE::CIdentifier(0x3CEA8129, 0xA772413A)
#define OVP_ClassId_BoxAlgorithm_Crop                                                  OpenViBE::CIdentifier(0x7F1A3002, 0x358117BA)
#define OVP_ClassId_BoxAlgorithm_CropDesc                                              OpenViBE::CIdentifier(0x64D619D7, 0x26CC42C9)

#define OVP_ClassId_BoxAlgorithm_MatrixTranspose                                       OpenViBE::CIdentifier(0x5E0F04B5, 0x5B5005CF)
#define OVP_ClassId_BoxAlgorithm_MatrixTransposeDesc                                   OpenViBE::CIdentifier(0x119249F7, 0x556C7E0D)

#define OVP_TypeId_SelectionMethod                         OpenViBE::CIdentifier(0x3BCF9E67, 0x0C23994D)
#define OVP_TypeId_SelectionMethod_Select                  OpenViBE::CIdentifier(0x1970FAAF, 0x4FD1CC4D)
#define OVP_TypeId_SelectionMethod_Reject                  OpenViBE::CIdentifier(0x4C05408D, 0x6EAC6F85)

#define OVP_TypeId_MatchMethod                             OpenViBE::CIdentifier(0x666F25E9, 0x3E5738D6)
#define OVP_TypeId_MatchMethod_Name                        OpenViBE::CIdentifier(0x58032A35, 0x4393A5D2)
#define OVP_TypeId_MatchMethod_Index                       OpenViBE::CIdentifier(0x0E0BF9E8, 0x3D612618)
#define OVP_TypeId_MatchMethod_Smart                       OpenViBE::CIdentifier(0x2D87EF07, 0xA2828AC0)

#define OVP_ClassId_BoxAlgorithm_DifferentialIntegral         OpenViBE::CIdentifier(0xCE490CBF, 0xDF7BA2E2)
#define OVP_ClassId_BoxAlgorithm_DifferentialIntegralDesc     OpenViBE::CIdentifier(0xCE490CBF, 0xDF7BA2E2)
#define OVP_TypeId_DifferentialIntegralOperation              OpenViBE::CIdentifier(0x6E6AD85D, 0x14FD203A)
#define OVP_TypeId_DifferentialIntegralOperation_Differential OpenViBE::CIdentifier(0x8EEF4E01, 0x1F9823C2)
#define OVP_TypeId_DifferentialIntegralOperation_Integral     OpenViBE::CIdentifier(0x44269C45, 0x77988564)

#define OVP_ClassId_Algorithm_ARBurgMethod                        			OpenViBE::CIdentifier(0x3EC6A165, 0x2823A034)
#define OVP_ClassId_Algorithm_ARBurgMethodDesc                    			OpenViBE::CIdentifier(0xD7234DFF, 0x55447A14)
#define OVP_Algorithm_ARBurgMethod_InputParameterId_Matrix       			OpenViBE::CIdentifier(0x36A69669, 0x3651271D)
#define OVP_Algorithm_ARBurgMethod_OutputParameterId_Matrix      			OpenViBE::CIdentifier(0x55EF8C81, 0x178A51B2)
#define OVP_Algorithm_ARBurgMethod_InputParameterId_UInteger     			OpenViBE::CIdentifier(0x33139BC1, 0x03D30D3B)
#define OVP_Algorithm_ARBurgMethod_InputTriggerId_Initialize 	 			OpenViBE::CIdentifier(0xC27B06C6, 0xB8EB5F8D)
#define OVP_Algorithm_ARBurgMethod_InputTriggerId_Process		     		OpenViBE::CIdentifier(0xBEEBBE84, 0x4F14F8F8)
#define OVP_Algorithm_ARBurgMethod_OutputTriggerId_ProcessDone           	OpenViBE::CIdentifier(0xA5AAD435, 0x9EC3DB80) 

#define OVP_ClassId_BoxAlgorithm_ConnectivityMeasure 					OpenViBE::CIdentifier(0x8E3A1AEF, 0x7CACD368)
#define OVP_ClassId_BoxAlgorithm_ConnectivityMeasureDesc 				OpenViBE::CIdentifier(0xA20B0A40, 0x1A92D645)
#define OVP_TypeId_Algorithm_SingleTrialPhaseLockingValue				OpenViBE::CIdentifier(0x740E1284, 0x24107052)
#define OVP_TypeId_Algorithm_SingleTrialPhaseLockingValueDesc           OpenViBE::CIdentifier(0x71CA3FD4, 0x0ED22534)
#define OVP_TypeId_Algorithm_MagnitudeSquaredCoherence                  OpenViBE::CIdentifier(0x5BAB50C3, 0x3A0E7D20)
#define OVP_TypeId_Algorithm_MagnitudeSquaredCoherenceDesc              OpenViBE::CIdentifier(0x24322906, 0xDE1D4AB3)
#define OVP_Algorithm_MagnitudeSquaredCoherence_InputParameterId_SegLength    OpenViBE::CIdentifier(0xA4826743, 0x0FA27C06)
#define OVP_Algorithm_MagnitudeSquaredCoherence_InputParameterId_Overlap      OpenViBE::CIdentifier(0x527F8AEC, 0xA25F2EAB)
#define OVP_Algorithm_MagnitudeSquaredCoherence_InputParameterId_Window       OpenViBE::CIdentifier(0x0EA349EE, 0xB9DC95D0)
#define OVP_Algorithm_MagnitudeSquaredCoherence_InputParameterId_Nfft         OpenViBE::CIdentifier(0x7726C677, 0xE266C5A2)
#define OVP_Algorithm_MagnitudeSquaredCoherence_OutputParameterId_OutputMatrixSpectrum	  OpenViBE::CIdentifier(0x331326BA, 0xA94CFC8A)
#define OVP_Algorithm_MagnitudeSquaredCoherence_OutputParameterId_FreqVector              OpenViBE::CIdentifier(0xD9FAA21C, 0x67D7C451)

#define OVP_ClassId_Algorithm_HilbertTransform								OpenViBE::CIdentifier(0x344B79DE, 0x89EAAABB)
#define OVP_ClassId_Algorithm_HilbertTransformDesc							OpenViBE::CIdentifier(0x8CAB236A, 0xA789800D)
#define OVP_Algorithm_HilbertTransform_InputParameterId_Matrix				OpenViBE::CIdentifier(0xC117CE9A, 0x3FFCB156)
#define OVP_Algorithm_HilbertTransform_OutputParameterId_HilbertMatrix		OpenViBE::CIdentifier(0xDAE13CB8, 0xEFF82E69)
#define OVP_Algorithm_HilbertTransform_OutputParameterId_EnvelopeMatrix		OpenViBE::CIdentifier(0x9D0A023A, 0x7690C48E)
#define OVP_Algorithm_HilbertTransform_OutputParameterId_PhaseMatrix		OpenViBE::CIdentifier(0x495B55E2, 0x8CAAC08E)
#define OVP_Algorithm_HilbertTransform_InputTriggerId_Initialize			OpenViBE::CIdentifier(0xE4B3CB4A, 0xF0121A20)
#define OVP_Algorithm_HilbertTransform_InputTriggerId_Process				OpenViBE::CIdentifier(0xC3DC087D, 0x4AAFC1F0)
#define OVP_Algorithm_HilbertTransform_OutputTriggerId_ProcessDone			OpenViBE::CIdentifier(0xB0B2A2DD, 0x73529B46)

#define OVP_ClassId_BoxAlgorithm_Hilbert				OpenViBE::CIdentifier(0x7878A47F, 0x9A8FE349)
#define OVP_ClassId_BoxAlgorithm_HilbertDesc			OpenViBE::CIdentifier(0x2DB54E2F, 0x435675EF)

#define OVP_ClassId_WindowFunctions					OpenViBE::CIdentifier(0x45FD3D73, 0x6C197A9C)

#define OVP_TypeId_WindowType						OpenViBE::CIdentifier(0x332BBB80, 0xC212810A)
#define OVP_TypeId_WindowType_Bartlett				OpenViBE::CIdentifier(0x4D7037DB, 0x3D2FD99A)
#define OVP_TypeId_WindowType_Hamming				OpenViBE::CIdentifier(0x4DDFE902, 0xA5318E97)
#define OVP_TypeId_WindowType_Hann					OpenViBE::CIdentifier(0xD784EB3B, 0x7B4A2658)
#define OVP_TypeId_WindowType_Parzen				OpenViBE::CIdentifier(0xDC2C83B7, 0xA78F8B31)
#define OVP_TypeId_WindowType_Welch					OpenViBE::CIdentifier(0x872D9FA8, 0x663C10A0)


//___________________________________________________________________//
//                                                                   //
// Plugin Object Descriptor Class Identifiers                        //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_ReferenceChannelDesc                                               OpenViBE::CIdentifier(0x1873B151, 0x969DD4E4)
#define OVP_ClassId_ChannelSelectorDesc                                                OpenViBE::CIdentifier(0x34893489, 0x44934897)
#define OVP_ClassId_SimpleDSPDesc                                                      OpenViBE::CIdentifier(0x00C44BFE, 0x76C9269E)
#define OVP_ClassId_SignalAverageDesc                                                  OpenViBE::CIdentifier(0x007CDCE9, 0x16034F77)
#define OVP_ClassId_SignalConcatenationDesc                                            OpenViBE::CIdentifier(0x3921BACD, 0x1E9546FE)
#define OVP_ClassId_BoxAlgorithm_QuadraticFormDesc                                     OpenViBE::CIdentifier(0x31C11856, 0x3E4F7B67)
//___________________________________________________________________//
//                                                                   //
// Plugin Object Class Identifiers                                   //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_ReferenceChannel                                                   OpenViBE::CIdentifier(0xEFA8E95B, 0x4F22551B)
#define OVP_ClassId_ChannelSelector                                                    OpenViBE::CIdentifier(0x39484563, 0x46386889)
#define OVP_ClassId_SimpleDSP                                                          OpenViBE::CIdentifier(0x00E26FA1, 0x1DBAB1B2)
#define OVP_ClassId_SignalAverage                                                      OpenViBE::CIdentifier(0x00642C4D, 0x5DF7E50A)
#define OVP_ClassId_SignalConcatenation                                                OpenViBE::CIdentifier(0x6568D29B, 0x0D753CCA)
#define OVP_ClassId_BoxAlgorithm_QuadraticForm                                         OpenViBE::CIdentifier(0x54E73B81, 0x1AD356C6)

/*/Dieter p300 stuff
#define OVP_ClassId_BoxAlgorithm_TwoSampleTTestDesc		OpenViBE::CIdentifier(0x020300BF, 0xB4BAC0A1)
#define OVP_ClassId_BoxAlgorithm_LikelinessDistributorDesc      OpenViBE::CIdentifier(0xE5103C63, 0x08D825E0)

#define OVP_ClassId_BoxAlgorithm_TwoSampleTTest			OpenViBE::CIdentifier(0x16DFF805, 0x0BC431BF)
#define OVP_ClassId_BoxAlgorithm_LikelinessDistributor		OpenViBE::CIdentifier(0x781F51CA, 0xE6E3B0B8)



#define OVP_ClassId_BoxAlgorithm_XDAWNSpatialFilterTrainerCoAdapt     OpenViBE::CIdentifier(0xEE31A115, 0x00B25FE1)
#define OVP_ClassId_BoxAlgorithm_XDAWNSpatialFilterTrainerCoAdaptDesc OpenViBE::CIdentifier(0xB59256B0, 0xC3389515)
#define OVP_ClassId_BoxAlgorithm_SpatialFilterWithUpdate     OpenViBE::CIdentifier(0x8E7CD4B3, 0xDAAD41C1)
#define OVP_ClassId_BoxAlgorithm_SpatialFilterWithUpdateDesc OpenViBE::CIdentifier(0xE2E6A1D6, 0xCBCEEE3A)

#define OVP_ClassId_ConditionalIdentity OpenViBE::CIdentifier(0xE4CE5D77, 0xBCC64C3F)
#define OVP_ClassId_ConditionalIdentityDesc OpenViBE::CIdentifier(0xDDE86CE9, 0xE75B0A29)

#define OVP_ClassId_BoxAlgorithm_MultipleSpatialFilters	 	OpenViBE::CIdentifier(0x04C1466C, 0xFC89B739)
#define OVP_ClassId_BoxAlgorithm_MultipleSpatialFiltersDesc	OpenViBE::CIdentifier(0xB1A96C51, 0xD75136A6)
//*///End of Dieter's stuff

#define OVP_ClassId_BoxAlgorithm_EpochVariance                  OpenViBE::CIdentifier(0x335384EA, 0x88C917D9)
#define OVP_ClassId_BoxAlgorithm_EpochVarianceDesc              OpenViBE::CIdentifier(0xA15EAEC5, 0xAB0CE73D)
#define OVP_ClassId_Algorithm_MatrixVarianceDesc 		OpenViBE::CIdentifier(0xE405260B, 0x59EEFAE4)
#define OVP_ClassId_Algorithm_MatrixVariance 			OpenViBE::CIdentifier(0x7FEFDCA9, 0x816ED903)

#define OVP_Algorithm_MatrixVariance_InputParameterId_Matrix			OpenViBE::CIdentifier(0x781F51CA, 0xE6E3B0B8)
#define OVP_Algorithm_MatrixVariance_InputParameterId_MatrixCount		OpenViBE::CIdentifier(0xE5103C63, 0x08D825E0)
#define OVP_Algorithm_MatrixVariance_InputParameterId_AveragingMethod		OpenViBE::CIdentifier(0x043A1BC4, 0x925D3CD6)
#define OVP_Algorithm_MatrixVariance_InputParameterId_SignificanceLevel		OpenViBE::CIdentifier(0x1E1065B2, 0x2CA32013)
#define OVP_Algorithm_MatrixVariance_OutputParameterId_AveragedMatrix		OpenViBE::CIdentifier(0x5CF66A73, 0xF5BBF0BF)
#define OVP_Algorithm_MatrixVariance_OutputParameterId_Variance			OpenViBE::CIdentifier(0x1BD67420, 0x587600E6)
#define OVP_Algorithm_MatrixVariance_OutputParameterId_ConfidenceBound		OpenViBE::CIdentifier(0x1E1065B2, 0x2CA32013)
#define OVP_Algorithm_MatrixVariance_InputTriggerId_Reset			OpenViBE::CIdentifier(0xD5C5EF91, 0xE1B1C4F4)
#define OVP_Algorithm_MatrixVariance_InputTriggerId_FeedMatrix			OpenViBE::CIdentifier(0xEBAEB213, 0xDD4735A0)
#define OVP_Algorithm_MatrixVariance_InputTriggerId_ForceAverage		OpenViBE::CIdentifier(0x344A52F5, 0x489DB439)
#define OVP_Algorithm_MatrixVariance_OutputTriggerId_AveragePerformed		OpenViBE::CIdentifier(0x2F9ECA0B, 0x8D3CA7BD)


// @BEGIN JP

/*
#define OVP_ClassId_Algorithm_RegressionAnalysis                                       OpenViBE::CIdentifier(0xFBFC00F8, 0x2D962BC0)
#define OVP_ClassId_Algorithm_RegressionAnalysisDesc                                   OpenViBE::CIdentifier(0x46C3A63E, 0x849E9890)

#define OVP_Algorithm_RegressionAnalysis_InputParameterId_Matrix0                      OpenViBE::CIdentifier(0x97DD0514, 0x5DD4E799)
#define OVP_Algorithm_RegressionAnalysis_InputParameterId_Matrix1                      OpenViBE::CIdentifier(0xBA8FFD91, 0x99B00152)
#define OVP_Algorithm_RegressionAnalysis_OutputParameterId_Matrix                      OpenViBE::CIdentifier(0x14EE5B8F, 0xE9A7C9DD)
#define OVP_Algorithm_RegressionAnalysis_InputTriggerId_Initialize                     OpenViBE::CIdentifier(0x9D9615C1, 0xDDD91FAF)
#define OVP_Algorithm_RegressionAnalysis_InputTriggerId_Process                        OpenViBE::CIdentifier(0xEB8827CE, 0xD5FFE7E6)
#define OVP_Algorithm_RegressionAnalysis_OutputTriggerId_ProcessDone                   OpenViBE::CIdentifier(0x260CA56C, 0x66B98B64)
#define OVP_Algorithm_RegressionAnalysis_InputParameterFilename                        OpenViBE::CIdentifier(0xA6CE7CD8, 0xB2788B53)
*/


#define OVP_ClassId_BoxAlgorithm_EOG_Denoising_Calibration                             OpenViBE::CIdentifier(0xE8DFE002, 0x70389932)
#define OVP_ClassId_BoxAlgorithm_EOG_Denoising_CalibrationDesc                         OpenViBE::CIdentifier(0xF4D74831, 0x88B80DCF)

/*
#define OVP_ClassId_Algorithm_RegressionAnalysisCalibration                            OpenViBE::CIdentifier(0x2AB3E6DC, 0x1DC3E29D)
#define OVP_ClassId_Algorithm_RegressionAnalysisCalibrationDesc                        OpenViBE::CIdentifier(0x1F1C316C, 0xAF7E5BDC)

#define OVP_Algorithm_RegressionAnalysisCalibration_InputParameterId_Matrix0           OpenViBE::CIdentifier(0x6D68A402, 0x27B4F10E)
#define OVP_Algorithm_RegressionAnalysisCalibration_InputParameterId_Matrix1           OpenViBE::CIdentifier(0xF0B8ABE9, 0x3D2DE4DF)
#define OVP_Algorithm_RegressionAnalysisCalibration_InputTriggerId_Initialize          OpenViBE::CIdentifier(0xECE6CADD, 0x128912F7)
#define OVP_Algorithm_RegressionAnalysisCalibration_InputTriggerId_Process             OpenViBE::CIdentifier(0xB7E93840, 0x13A8CCE1)
#define OVP_Algorithm_RegressionAnalysisCalibration_OutputTriggerId_ProcessDone        OpenViBE::CIdentifier(0x3CC023B2, 0xFEC907D2)
*/





/*
#define OVP_ClassId_Algorithm_MatrixMaximum                     OpenViBE::CIdentifier(0x3B7723EC, 0x16C30A39)
#define OVP_ClassId_Algorithm_MatrixMaximumDesc                 OpenViBE::CIdentifier(0x11BE2168, 0x5B444BBB)
#define OVP_Algorithm_MatrixMaximum_InputParameterId_Matrix     OpenViBE::CIdentifier(0x56254223, 0x42180588)
#define OVP_Algorithm_MatrixMaximum_OutputParameterId_Matrix    OpenViBE::CIdentifier(0x025A4450, 0x6DFD17DB)
#define OVP_Algorithm_MatrixMaximum_InputTriggerId_Initialize   OpenViBE::CIdentifier(0x41803B07, 0x667A69BC)
#define OVP_Algorithm_MatrixMaximum_InputTriggerId_Process      OpenViBE::CIdentifier(0x641A59C0, 0x12FB7F74)
#define OVP_Algorithm_MatrixMaximum_OutputTriggerId_ProcessDone OpenViBE::CIdentifier(0x37802521, 0x785D51FD)
*/

#define OVP_ClassId_BoxAlgorithm_Inverse_DWT                        OpenViBE::CIdentifier(0x5B5B8468, 0x212CF963)
#define OVP_ClassId_BoxAlgorithm_Inverse_DWTDesc                    OpenViBE::CIdentifier(0x01B9BC9A, 0x34766AE9)

#define OVP_ClassId_BoxAlgorithm_DiscreteWaveletTransform           OpenViBE::CIdentifier(0x824194C5, 0x46D7FDE9)
#define OVP_ClassId_BoxAlgorithm_DiscreteWaveletTransformDesc       OpenViBE::CIdentifier(0x6744711B, 0xF21B59EC)
#define OVP_TypeId_WaveletType                                      OpenViBE::CIdentifier(0x393EAC3E, 0x793C0F1D)
#define OVP_TypeId_WaveletType_Haar                                 OpenViBE::CIdentifier(0xECEBCD83, 0x55F416B2)
#define OVP_TypeId_WaveletType_db1                                  OpenViBE::CIdentifier(0x2B47E7A4, 0x684A7356)
#define OVP_TypeId_WaveletType_db2                                  OpenViBE::CIdentifier(0x876B44BD, 0xC0F101AC)
#define OVP_TypeId_WaveletType_db3                                  OpenViBE::CIdentifier(0x1660F693, 0x7D3A84EE)
#define OVP_TypeId_WaveletType_db4                                  OpenViBE::CIdentifier(0x8C7586D0, 0x03D2EAD3)
#define OVP_TypeId_WaveletType_db5                                  OpenViBE::CIdentifier(0xD7C497DB, 0x86C9C4B0)
#define OVP_TypeId_WaveletType_db6                                  OpenViBE::CIdentifier(0x655EDCBD, 0xDB629E9C)
#define OVP_TypeId_WaveletType_db7                                  OpenViBE::CIdentifier(0x88FB549E, 0xADBAF914)
#define OVP_TypeId_WaveletType_db8                                  OpenViBE::CIdentifier(0x768486EF, 0xDC41A1CB)
#define OVP_TypeId_WaveletType_db9                                  OpenViBE::CIdentifier(0x6E93448B, 0x1522F5FE)
#define OVP_TypeId_WaveletType_db10                                 OpenViBE::CIdentifier(0x8948D613, 0xF7AA9FA8)
#define OVP_TypeId_WaveletType_db11                                 OpenViBE::CIdentifier(0xCCA674E4, 0x2496591B)
#define OVP_TypeId_WaveletType_db12                                 OpenViBE::CIdentifier(0xAAB48F2E, 0x732CAE6C)
#define OVP_TypeId_WaveletType_db13                                 OpenViBE::CIdentifier(0xF532049B, 0x6FE27D70)
#define OVP_TypeId_WaveletType_db14                                 OpenViBE::CIdentifier(0x6D62C0F3, 0xF4BA32C9)
#define OVP_TypeId_WaveletType_db15                                 OpenViBE::CIdentifier(0xF40DB692, 0xCD6E80A4)
#define OVP_TypeId_WaveletType_bior11                               OpenViBE::CIdentifier(0xF89E2927, 0x120E240C)
#define OVP_TypeId_WaveletType_bior13                               OpenViBE::CIdentifier(0x01DC3924, 0x919CE5D5)
#define OVP_TypeId_WaveletType_bior15                               OpenViBE::CIdentifier(0x318E5D28, 0xD189E902)
#define OVP_TypeId_WaveletType_bior22                               OpenViBE::CIdentifier(0xB1B67B9B, 0x212F5C6A)
#define OVP_TypeId_WaveletType_bior24                               OpenViBE::CIdentifier(0x0AC9945B, 0x0AD7FFFC)
#define OVP_TypeId_WaveletType_bior26                               OpenViBE::CIdentifier(0x98F77AB9, 0x7D6C0659)
#define OVP_TypeId_WaveletType_bior28                               OpenViBE::CIdentifier(0x3BAD7227, 0x39222FBA)
#define OVP_TypeId_WaveletType_bior31                               OpenViBE::CIdentifier(0x28B90690, 0xB05F2157)
#define OVP_TypeId_WaveletType_bior33                               OpenViBE::CIdentifier(0x2FB7C26D, 0x45633194)
#define OVP_TypeId_WaveletType_bior35                               OpenViBE::CIdentifier(0xFB91D6FF, 0x17692D1F)
#define OVP_TypeId_WaveletType_bior37                               OpenViBE::CIdentifier(0x3427E8F2, 0x1622E5DD)
#define OVP_TypeId_WaveletType_bior39                               OpenViBE::CIdentifier(0x648D3751, 0x42486F57)
#define OVP_TypeId_WaveletType_bior44                               OpenViBE::CIdentifier(0x649EAB0A, 0xCAEF44DB)
#define OVP_TypeId_WaveletType_bior55                               OpenViBE::CIdentifier(0x3F04FF6B, 0x87201FE0)
#define OVP_TypeId_WaveletType_bior68                               OpenViBE::CIdentifier(0xEFB334E8, 0x6F4AD8AC)
#define OVP_TypeId_WaveletType_coif1                                OpenViBE::CIdentifier(0x435DEE7C, 0x1985545B)
#define OVP_TypeId_WaveletType_coif2                                OpenViBE::CIdentifier(0xD80C8600, 0x28A4D449)
#define OVP_TypeId_WaveletType_coif3                                OpenViBE::CIdentifier(0x2CADE482, 0xF1FDB654)
#define OVP_TypeId_WaveletType_coif4                                OpenViBE::CIdentifier(0x98D74D5A, 0x1CBD4241)
#define OVP_TypeId_WaveletType_coif5                                OpenViBE::CIdentifier(0x8E04A881, 0xCA14F2A3)
#define OVP_TypeId_WaveletType_sym1                                 OpenViBE::CIdentifier(0x538B9504, 0x377E928F)
#define OVP_TypeId_WaveletType_sym2                                 OpenViBE::CIdentifier(0x946FBE9E, 0xB26F8423)
#define OVP_TypeId_WaveletType_sym3                                 OpenViBE::CIdentifier(0xF38621BA, 0x5CCF36E4)
#define OVP_TypeId_WaveletType_sym4                                 OpenViBE::CIdentifier(0x10367654, 0x8B3FE842)
#define OVP_TypeId_WaveletType_sym5                                 OpenViBE::CIdentifier(0xFC55B3E3, 0xBC8C2902)
#define OVP_TypeId_WaveletType_sym6                                 OpenViBE::CIdentifier(0x9865AE89, 0xDF57313C)
#define OVP_TypeId_WaveletType_sym7                                 OpenViBE::CIdentifier(0xFBE4FC14, 0x737D89E8)
#define OVP_TypeId_WaveletType_sym8                                 OpenViBE::CIdentifier(0x2CEC3288, 0x6DFA03C4)
#define OVP_TypeId_WaveletType_sym9                                 OpenViBE::CIdentifier(0x90B6508F, 0xEA8D26EF)
#define OVP_TypeId_WaveletType_sym10                                OpenViBE::CIdentifier(0x40FA4136, 0x9928BED0)

#define OVP_TypeId_WaveletLevel                                     OpenViBE::CIdentifier(0xF80A2144, 0x6E692C51)
#define OVP_TypeId_WaveletLevel_1                                   OpenViBE::CIdentifier(0x250F2D72, 0x2D46B76E)
#define OVP_TypeId_WaveletLevel_2                                   OpenViBE::CIdentifier(0x070FCB6A, 0x8ECBA520)
#define OVP_TypeId_WaveletLevel_3                                   OpenViBE::CIdentifier(0xDAADEACA, 0x5D55FF60)
#define OVP_TypeId_WaveletLevel_4                                   OpenViBE::CIdentifier(0x3447C01C, 0x0B0467FD)
#define OVP_TypeId_WaveletLevel_5                                   OpenViBE::CIdentifier(0x36278DFC, 0xDDBC678C)



// @END JP




//___________________________________________________________________//
//                                                                   //
// Global defines                                                   //
//___________________________________________________________________//
//                                                                   //

#ifdef TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines
 #include "ovp_global_defines.h"
#endif // TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines


#define OVP_Value_CoupledStringSeparator                                '-'
//#define OVP_Value_AllSelection											'*'

#endif // __OpenViBEPlugins_Defines_H__
