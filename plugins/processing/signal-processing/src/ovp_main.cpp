
#include "algorithms/basic/ovpCMatrixAverage.h"
#include "algorithms/epoching/ovpCAlgorithmStimulationBasedEpoching.h"
//#include "algorithms/filters/ovpCApplySpatialFilter.h"

#include "box-algorithms/basic/ovpCIdentity.h"

#include "box-algorithms/basic/ovpCBoxAlgorithmChannelRename.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmChannelSelector.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmEpochAverage.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmCrop.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmMatrixTranspose.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmSignalDecimation.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmReferenceChannel.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmDifferentialIntegral.h"
#include "box-algorithms/epoching/ovpCBoxAlgorithmStimulationBasedEpoching.h"
//#include "box-algorithms/filters/ovpCSpatialFilterBoxAlgorithm.h"
#include "box-algorithms/filters/ovpCBoxAlgorithmCommonAverageReference.h"
#include "box-algorithms/filters/ovpCBoxAlgorithmSpatialFilter.h"

#include "box-algorithms/spectral-analysis/ovpCBoxAlgorithmFrequencyBandSelector.h"
#include "box-algorithms/spectral-analysis/ovpCBoxAlgorithmSpectrumAverage.h"

#include "box-algorithms/connectivity/ovpCBoxAlgorithmConnectivityMeasure.h"
#include "algorithms/connectivity/ovpCAlgorithmSingleTrialPhaseLockingValue.h"
#include "algorithms/connectivity/ovpCAlgorithmMagnitudeSquaredCoherence.h"

#include "box-algorithms/basic/ovpCBoxAlgorithmHilbert.h"
#include "algorithms/basic/ovpCHilbertTransform.h"
#include "algorithms/basic/ovpCWindowFunctions.h"

#include "box-algorithms/ovpCTimeBasedEpoching.h"
#include "box-algorithms/ovpCSimpleDSP.h"
#include "box-algorithms/ovpCSignalAverage.h"
#include "box-algorithms/ovpCBoxAlgorithmQuadraticForm.h"

#include "box-algorithms/filter/ovpCBoxAlgorithmXDAWNSpatialFilterTrainer.h"

#include "box-algorithms/basic/ovpCBoxAlgorithmIFFTbox.h"

#include "algorithms/basic/ovpCAlgorithmARBurgMethod.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmARCoefficients.h"


/*/Dieter boxes for p300
#include "box-algorithms/ovpCBoxAlgorithmTwoSampleTTest.h"
#include "box-algorithms/ovpCBoxAlgorithmLikelinessDistributor.h"

#include "box-algorithms/ovpCBoxAlgorithmXDAWNSpatialFilterTrainer.h"
#include "box-algorithms/ovpCBoxAlgorithmSpatialFilter.h"
#include "box-algorithms/ovpCBoxAlgorithmMultipleSpatialFilters.h"
#include "box-algorithms/ovpCBoxAlgorithmConditionalIdentity.h"

//*/
#include "algorithms/basic/ovpCMatrixVariance.h"
#include "box-algorithms/basic/ovpCBoxAlgorithmEpochVariance.h"

// @BEGIN JP
#include "box-algorithms/ovpCBoxAlgorithmEOG_Denoising.h"
#include "box-algorithms/ovpCBoxAlgorithmEOG_Denoising_Calibration.h"
#include "box-algorithms/ovpCBoxAlgorithmDiscreteWaveletTransform.h"
#include "box-algorithms/ovpCBoxAlgorithmInverse_DWT.h"
// @END JP


OVP_Declare_Begin()

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_EpochAverageMethod, "Epoch Average method");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverageMethod, "Moving epoch average", OVP_TypeId_EpochAverageMethod_MovingAverage.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverageMethod, "Moving epoch average (Immediate)", OVP_TypeId_EpochAverageMethod_MovingAverageImmediate.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverageMethod, "Epoch block average", OVP_TypeId_EpochAverageMethod_BlockAverage.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverageMethod, "Cumulative average", OVP_TypeId_EpochAverageMethod_CumulativeAverage.toUInteger());

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_CropMethod, "Crop method");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_CropMethod, "Min",     OVP_TypeId_CropMethod_Min.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_CropMethod, "Max",     OVP_TypeId_CropMethod_Max.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_CropMethod, "Min/Max", OVP_TypeId_CropMethod_MinMax.toUInteger());


	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_SelectionMethod, "Selection method");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SelectionMethod, "Select", OVP_TypeId_SelectionMethod_Select.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SelectionMethod, "Reject", OVP_TypeId_SelectionMethod_Reject.toUInteger());

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_MatchMethod, "Match method");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_MatchMethod, "Name",  OVP_TypeId_MatchMethod_Name.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_MatchMethod, "Index", OVP_TypeId_MatchMethod_Index.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_MatchMethod, "Smart", OVP_TypeId_MatchMethod_Smart.toUInteger());

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_DifferentialIntegralOperation, "Differential/Integral select");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_DifferentialIntegralOperation, "Differential", OVP_TypeId_DifferentialIntegralOperation_Differential.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_DifferentialIntegralOperation, "Integral",     OVP_TypeId_DifferentialIntegralOperation_Integral.toUInteger());

#if defined(TARGET_HAS_ThirdPartyEIGEN)
	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_ClassId_ConnectivityAlgorithm, "Connectivity measure method");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry (OVP_ClassId_ConnectivityAlgorithm, "Single-Trial Phase Locking Value", OVP_TypeId_Algorithm_SingleTrialPhaseLockingValue.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry (OVP_ClassId_ConnectivityAlgorithm, "Magnitude Squared Coherence", OVP_TypeId_Algorithm_MagnitudeSquaredCoherence.toUInteger());
#endif

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_WindowType, "Window method");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry (OVP_TypeId_WindowType, "Bartlett (triangular)", OVP_TypeId_WindowType_Bartlett.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry (OVP_TypeId_WindowType, "Hamming", OVP_TypeId_WindowType_Hamming.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry (OVP_TypeId_WindowType, "Hann", OVP_TypeId_WindowType_Hann.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry (OVP_TypeId_WindowType, "Parzen", OVP_TypeId_WindowType_Parzen.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry (OVP_TypeId_WindowType, "Welch", OVP_TypeId_WindowType_Welch.toUInteger());

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CIdentityDesc);

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CTimeBasedEpochingDesc);

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CMatrixAverageDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CAlgorithmStimulationBasedEpochingDesc)
//	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CApplySpatialFilterDesc)

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmChannelRenameDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmChannelSelectorDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmReferenceChannelDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmDifferentialIntegralDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CEpochAverageDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmCropDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmMatrixTransposeDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmSignalDecimationDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmStimulationBasedEpochingDesc)
//	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CSpatialFilterBoxAlgorithmDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmCommonAverageReferenceDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmSpatialFilterDesc)

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CSimpleDSPDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CSignalAverageDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmQuadraticFormDesc)

	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmFrequencyBandSelectorDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmSpectrumAverageDesc)

#if defined TARGET_HAS_ThirdPartyITPP
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmXDAWNSpatialFilterTrainerDesc);

	OVP_Declare_New(OpenViBEPlugins::SignalProcessingBasic::CBoxAlgorithmIFFTboxDesc);

#endif // TARGET_HAS_ThirdPartyITPP


#ifdef __OpenViBEPlugins_BoxAlgorithm_ARCoefficients_H__
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmARCoefficientsDesc);
#endif
#ifdef __OpenViBEPlugins_Algorithm_ARBurgMethod_H__
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CAlgorithmARBurgMethodDesc);
#endif
	
#ifdef __OpenViBEPlugins_BoxAlgorithm_ConnectivityMeasure_H__
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmConnectivityMeasureDesc)
#endif
#ifdef __OpenViBEPlugins_Algorithm_HilbertTransform_H__
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CAlgorithmHilbertTransformDesc)
#endif
#ifdef __OpenViBEPlugins_Algorithm_SingleTrialPhaseLockingValue_H__
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CAlgorithmSingleTrialPhaseLockingValueDesc)
#endif

#if defined TARGET_HAS_ThirdPartyEIGEN
	OVP_Declare_New(OpenViBEPlugins::SignalProcessingBasic::CBoxAlgorithmHilbertDesc)
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CAlgorithmMagnitudeSquaredCoherenceDesc)
#endif

	/*/Dieter stuff for p300


	OVP_Declare_New(OpenViBEPlugins::SignalProcessingStatistics::CBoxAlgorithmTwoSampleTTestDesc);
	OVP_Declare_New(OpenViBEPlugins::SignalProcessingStatistics::CBoxAlgorithmLikelinessDistributorDesc);
	
	
	OVP_Declare_New(OpenViBEPlugins::SignalProcessingCoAdapt::CBoxAlgorithmXDAWNSpatialFilterTrainerDesc);
	OVP_Declare_New(OpenViBEPlugins::SignalProcessingCoAdapt::CBoxAlgorithmSpatialFilterDesc);
	OVP_Declare_New(OpenViBEPlugins::SignalProcessingCoAdapt::CConditionalIdentityDesc);
	OVP_Declare_New(OpenViBEPlugins::SignalProcessingCoAdapt::CBoxAlgorithmMultipleSpatialFiltersDesc);
	//*/
	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_EpochAverageMethod, "Epoch Average method");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverageMethod, "Moving epoch average", OVP_TypeId_EpochAverageMethod_MovingAverage.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverageMethod, "Moving epoch average (Immediate)", OVP_TypeId_EpochAverageMethod_MovingAverageImmediate.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverageMethod, "Epoch block average", OVP_TypeId_EpochAverageMethod_BlockAverage.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_EpochAverageMethod, "Cumulative average", OVP_TypeId_EpochAverageMethod_CumulativeAverage.toUInteger());
	
#if defined(TARGET_HAS_ThirdPartyITPP)	
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CMatrixVarianceDesc);
#endif
	OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CEpochVarianceDesc);

    // @BEGIN JP
#if defined(__OpenViBEPlugins_BoxAlgorithm_EOG_Denoising_Calibration_H__)
    OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmEOG_DenoisingDesc);

    OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmEOG_Denoising_CalibrationDesc);
#endif

#if defined(__OpenViBEPlugins_BoxAlgorithm_DiscreteWaveletTransform_H__)
    OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmDiscreteWaveletTransformDesc);

    OVP_Declare_New(OpenViBEPlugins::SignalProcessing::CBoxAlgorithmInverse_DWTDesc);

    rPluginModuleContext.getTypeManager().registerEnumerationType(OVP_TypeId_WaveletType, "Wavelet type");
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "haar" , OVP_TypeId_WaveletType_Haar.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db1" , OVP_TypeId_WaveletType_db1.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db2" , OVP_TypeId_WaveletType_db2.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db3" , OVP_TypeId_WaveletType_db3.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db4" , OVP_TypeId_WaveletType_db4.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db5" , OVP_TypeId_WaveletType_db5.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db6" , OVP_TypeId_WaveletType_db6.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db7" , OVP_TypeId_WaveletType_db7.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db8" , OVP_TypeId_WaveletType_db8.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db9" , OVP_TypeId_WaveletType_db9.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db10" , OVP_TypeId_WaveletType_db10.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db11" , OVP_TypeId_WaveletType_db11.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db12" , OVP_TypeId_WaveletType_db12.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db13" , OVP_TypeId_WaveletType_db13.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db14" , OVP_TypeId_WaveletType_db14.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "db15" , OVP_TypeId_WaveletType_db15.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior1.1" , OVP_TypeId_WaveletType_bior11.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior1.3" , OVP_TypeId_WaveletType_bior13.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior1.5" , OVP_TypeId_WaveletType_bior15.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior2.2" , OVP_TypeId_WaveletType_bior22.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior2.4" , OVP_TypeId_WaveletType_bior24.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior2.6" , OVP_TypeId_WaveletType_bior26.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior2.8" , OVP_TypeId_WaveletType_bior28.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior3.1" , OVP_TypeId_WaveletType_bior31.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior3.3" , OVP_TypeId_WaveletType_bior33.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior3.5" , OVP_TypeId_WaveletType_bior35.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior3.7" , OVP_TypeId_WaveletType_bior37.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior3.9" , OVP_TypeId_WaveletType_bior39.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior4.4" , OVP_TypeId_WaveletType_bior44.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior5.5" , OVP_TypeId_WaveletType_bior55.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "bior6.8" , OVP_TypeId_WaveletType_bior68.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "coif1" , OVP_TypeId_WaveletType_coif1.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "coif2" , OVP_TypeId_WaveletType_coif2.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "coif3" , OVP_TypeId_WaveletType_coif3.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "coif4" , OVP_TypeId_WaveletType_coif4.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "coif5" , OVP_TypeId_WaveletType_coif5.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "sym1" , OVP_TypeId_WaveletType_sym1.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "sym2" , OVP_TypeId_WaveletType_sym2.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "sym3" , OVP_TypeId_WaveletType_sym3.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "sym4" , OVP_TypeId_WaveletType_sym4.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "sym5" , OVP_TypeId_WaveletType_sym5.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "sym6" , OVP_TypeId_WaveletType_sym6.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "sym7" , OVP_TypeId_WaveletType_sym7.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "sym8" , OVP_TypeId_WaveletType_sym8.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "sym9" , OVP_TypeId_WaveletType_sym9.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletType, "sym10" , OVP_TypeId_WaveletType_sym10.toUInteger());

    rPluginModuleContext.getTypeManager().registerEnumerationType(OVP_TypeId_WaveletLevel, "Wavelet decomposition levels");
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletLevel, "1" , OVP_TypeId_WaveletLevel_1.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletLevel, "2" , OVP_TypeId_WaveletLevel_2.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletLevel, "3" , OVP_TypeId_WaveletLevel_3.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletLevel, "4" , OVP_TypeId_WaveletLevel_4.toUInteger());
    rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_WaveletLevel, "5" , OVP_TypeId_WaveletLevel_5.toUInteger());
#endif
	// @END JP

OVP_Declare_End()
