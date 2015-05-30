#if defined(TARGET_HAS_ThirdPartyEIGEN)

#ifndef __OpenViBEPlugins_Algorithm_HilbertTransform_H__
#define __OpenViBEPlugins_Algorithm_HilbertTransform_H__

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>
#include <complex>

// This class could be in its own file
class HilbertTransform {

public:
	
	OpenViBE::boolean transform(const Eigen::VectorXcd& vecXcdInput, Eigen::VectorXcd& vecXcdOutput);

private:
	Eigen::VectorXcd m_vecXcdSignalFourier;    // Fourier Transform of the input signal
	Eigen::VectorXcd m_vecXcdHilbert;          // Vector h used to apply Hilbert transform

	Eigen::FFT< double, Eigen::internal::kissfft_impl<double > > m_oFFT; // Instance of the fft transform

};

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CAlgorithmHilbertTransform : public OpenViBEToolkit::TAlgorithm <OpenViBE::Plugins::IAlgorithm>
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TAlgorithm<OpenViBE::Plugins::IAlgorithm>, OVP_ClassId_Algorithm_HilbertTransform)


		protected:

			OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> ip_pMatrix; //input matrix
			OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> op_pHilbertMatrix; //output matrix 1 : Hilbert transform of the signal
			OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> op_pEnvelopeMatrix; //output matrix 2 : Envelope of the signal
			OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> op_pPhaseMatrix; //output matrix 3 : Phase of the signal


		private:

			HilbertTransform m_oHilbert; // Instance of the Hilbert transform doing the actual computation

		};

		class CAlgorithmHilbertTransformDesc : public OpenViBE::Plugins::IAlgorithmDesc
		{
		public:
			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Hilbert Transform"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Alison Cellard"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Computes the Hilbert transform of a signal"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Give the analytic signal ua(t) = u(t) + iH(u(t)) of the input signal u(t) using Hilbert transform"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Basic"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.2"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-execute"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Algorithm_HilbertTransform; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CAlgorithmHilbertTransform; }

			virtual OpenViBE::boolean getAlgorithmPrototype(
					OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
			{
				rAlgorithmPrototype.addInputParameter (OVP_Algorithm_HilbertTransform_InputParameterId_Matrix,     	    "Matrix", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmPrototype.addOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_HilbertMatrix,  "Hilbert Matrix", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmPrototype.addOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_EnvelopeMatrix, "Envelope Matrix", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmPrototype.addOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_PhaseMatrix,    "Phase Matrix", OpenViBE::Kernel::ParameterType_Matrix);

				rAlgorithmPrototype.addInputTrigger   (OVP_Algorithm_HilbertTransform_InputTriggerId_Initialize,   "Initialize");
				rAlgorithmPrototype.addInputTrigger   (OVP_Algorithm_HilbertTransform_InputTriggerId_Process,      "Process");
				rAlgorithmPrototype.addOutputTrigger  (OVP_Algorithm_HilbertTransform_OutputTriggerId_ProcessDone, "Process done");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_Algorithm_HilbertTransformDesc);
		};
	};  // namespace SignalProcessing
}; // namespace OpenViBEPlugins

#endif //__OpenViBEPlugins_Algorithm_HilbertTransform_H__
#endif //TARGET_HAS_ThirdPartyEIGEN
