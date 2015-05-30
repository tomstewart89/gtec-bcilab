#if defined(TARGET_HAS_ThirdPartyEIGEN)

#ifndef __OpenViBEPlugins_Algorithm_MagnitudeSquaredCoherence_H__
#define __OpenViBEPlugins_Algorithm_MagnitudeSquaredCoherence_H__

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include "ovpCConnectivityAlgorithm.h"
#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>
#include "../basic/ovpCWindowFunctions.h"



//#define OVP_TypeId_Algorithm_MagnitudeSquaredCoherence      			OpenViBE::CIdentifier(0x5BAB50C3, 0x3A0E7D20)
//#define OVP_TypeId_Algorithm_MagnitudeSquaredCoherenceDesc				OpenViBE::CIdentifier(0x24322906, 0xDE1D4AB3)




namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CAlgorithmMagnitudeSquaredCoherence : public OpenViBEPlugins::CConnectivityAlgorithm
		{
		public:

			virtual void release(void) { delete this; }

			OpenViBE::boolean computePeriodogram(const Eigen::VectorXd& vecXcdInput, Eigen::MatrixXcd& matXcdPeriodograms,const Eigen::VectorXd& vecXdWindow, const OpenViBE::uint32& ui32NSegments, const OpenViBE::uint32& ui32LSegments, const OpenViBE::uint32& ui32NOverlap);
			OpenViBE::boolean powerSpectralDensity(const Eigen::VectorXd& vecXdInput, Eigen::VectorXd& vecXdOutput, const Eigen::VectorXd& vecXdWindow, const OpenViBE::uint32& ui32NSegments, const OpenViBE::uint32& ui32LSegments, const OpenViBE::uint32& ui32NOverlap);
			OpenViBE::boolean crossSpectralDensity(const Eigen::VectorXd& vecXdInput1, const Eigen::VectorXd& vecXdInput2, Eigen::VectorXcd& vecXcdOutput, const Eigen::VectorXd& vecXdWindow, const OpenViBE::uint32& ui32NSegments, const OpenViBE::uint32& ui32LSegments, const OpenViBE::uint32& ui32NOverlap);

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean process(void);


			_IsDerivedFromClass_Final_(OpenViBEPlugins::CConnectivityAlgorithm, OVP_TypeId_Algorithm_MagnitudeSquaredCoherence);

		protected:


			OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> ip_pSignal1;
			OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> ip_pSignal2;

			OpenViBE::Kernel::TParameterHandler <OpenViBE::uint64> ip_ui64SamplingRate;

			OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> ip_pChannelPairs;
			OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> op_pMatrixMean;
			OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> op_pMatrixSpectrum;

			OpenViBE::Kernel::TParameterHandler <OpenViBE::uint64> ip_ui64SegmentLength;
			OpenViBE::Kernel::TParameterHandler <OpenViBE::uint64> ip_ui64Overlap;
			OpenViBE::Kernel::TParameterHandler <OpenViBE::uint64> ip_ui64WindowType;

			OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> op_pFrequencyBandVector;

		private:
			// why members?
			Eigen::VectorXd m_vecXdPowerSpectrum1;
			Eigen::VectorXd m_vecXdPowerSpectrum2;
			Eigen::VectorXcd m_vecXcdCrossSpectrum;

			Eigen::VectorXd m_vecXdWindow;  // Window weight vector
			OpenViBE::float64 m_f64U;       // Window's normalization constant

			WindowFunctions m_oWindow;

			Eigen::FFT< double, Eigen::internal::kissfft_impl<double > > m_oFFT; // Instance of the fft transform


		};

		class CAlgorithmMagnitudeSquaredCoherenceDesc : public OpenViBEPlugins::CConnectivityAlgorithmDesc
		{
		public:
			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Magnitude Squared Coherence"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Alison Cellard"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Compute Coherence"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Computes the Magnitude Squared Coherence algorithm between two signals"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Connectivity"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-execute"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_TypeId_Algorithm_MagnitudeSquaredCoherence; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CAlgorithmMagnitudeSquaredCoherence; }

			virtual OpenViBE::boolean getAlgorithmPrototype(
					OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
			{

				CConnectivityAlgorithmDesc::getAlgorithmPrototype(rAlgorithmPrototype);

				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_MagnitudeSquaredCoherence_InputParameterId_Window, "Window method", OpenViBE::Kernel::ParameterType_Enumeration, OVP_TypeId_WindowType);
				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_MagnitudeSquaredCoherence_InputParameterId_SegLength, "Length of segments", OpenViBE::Kernel::ParameterType_Integer, 32);
				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_MagnitudeSquaredCoherence_InputParameterId_Overlap, "Overlap (percentage)", OpenViBE::Kernel::ParameterType_Integer);

				rAlgorithmPrototype.addOutputParameter(OVP_Algorithm_Connectivity_OutputParameterId_OutputMatrix,    "Mean coherence signal", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmPrototype.addOutputParameter(OVP_Algorithm_MagnitudeSquaredCoherence_OutputParameterId_OutputMatrixSpectrum, "Coherence spectrum", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmPrototype.addOutputParameter(OVP_Algorithm_MagnitudeSquaredCoherence_OutputParameterId_FreqVector, "Frequency vector", OpenViBE::Kernel::ParameterType_Matrix);

				rAlgorithmPrototype.addInputTrigger   (OVP_Algorithm_Connectivity_InputTriggerId_Initialize,   "Initialize");
				rAlgorithmPrototype.addInputTrigger   (OVP_Algorithm_Connectivity_InputTriggerId_Process,      "Process");
				rAlgorithmPrototype.addOutputTrigger  (OVP_Algorithm_Connectivity_OutputTriggerId_ProcessDone, "Process done");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEPlugins::CConnectivityAlgorithmDesc, OVP_TypeId_Algorithm_MagnitudeSquaredCoherenceDesc);
		};
	};  // namespace SignalProcessing
}; // namespace OpenViBEPlugins

#endif //__OpenViBEPlugins_Algorithm_MagnitudeSquaredCoherence_H__
#endif //TARGET_HAS_ThirdPartyEIGEN
