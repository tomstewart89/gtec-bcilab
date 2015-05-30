#ifndef __OpenViBEPlugins_SignalProcessing_Algorithms_Basic_CMatrixVariance_H__
#define __OpenViBEPlugins_SignalProcessing_Algorithms_Basic_CMatrixVariance_H__

#if defined(TARGET_HAS_ThirdPartyITPP)

#include "../../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <itpp/base/vec.h>
#include <deque>

using namespace itpp;

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CMatrixVariance : public OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVP_ClassId_Algorithm_MatrixVariance);

		protected:

			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_ui64AveragingMethod;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_ui64MatrixCount;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::float64 > ip_f64SignificanceLevel;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > ip_pMatrix;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pAveragedMatrix;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pMatrixVariance;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pConfidenceBound;

			std::deque < Vec<OpenViBE::float64>* > m_vHistory;

			Vec<OpenViBE::float64> m_vMean;
			Vec<OpenViBE::float64> m_vM;	
			Vec<OpenViBE::float64> m_f64Variance;
			OpenViBE::uint32 m_ui32InputCounter;
		};

		class CMatrixVarianceDesc : public OpenViBE::Plugins::IAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Matrix variance"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Dieter Devlaminck"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Basic"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Algorithm_MatrixVariance; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CMatrixVariance(); }

			virtual OpenViBE::boolean getAlgorithmPrototype(
				OpenViBE::Kernel::IAlgorithmProto& rAlgorithmProto) const
			{
				rAlgorithmProto.addInputParameter (OVP_Algorithm_MatrixVariance_InputParameterId_Matrix,                    "Matrix",              OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmProto.addInputParameter (OVP_Algorithm_MatrixVariance_InputParameterId_MatrixCount,               "Matrix count",        OpenViBE::Kernel::ParameterType_UInteger);
				rAlgorithmProto.addInputParameter (OVP_Algorithm_MatrixVariance_InputParameterId_SignificanceLevel,               "Significance Level",        OpenViBE::Kernel::ParameterType_UInteger);
				rAlgorithmProto.addInputParameter (OVP_Algorithm_MatrixVariance_InputParameterId_AveragingMethod,           "Averaging Method",    OpenViBE::Kernel::ParameterType_UInteger);

				rAlgorithmProto.addOutputParameter(OVP_Algorithm_MatrixVariance_OutputParameterId_AveragedMatrix,           "Averaged matrix",     OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmProto.addOutputParameter(OVP_Algorithm_MatrixVariance_OutputParameterId_Variance,           "Matrix variance",     OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmProto.addOutputParameter(OVP_Algorithm_MatrixVariance_OutputParameterId_ConfidenceBound,           "Confidence bound",     OpenViBE::Kernel::ParameterType_Matrix);

				rAlgorithmProto.addInputTrigger   (OVP_Algorithm_MatrixVariance_InputTriggerId_Reset,                       "Reset");
				rAlgorithmProto.addInputTrigger   (OVP_Algorithm_MatrixVariance_InputTriggerId_FeedMatrix,                  "Feed matrix");
				rAlgorithmProto.addInputTrigger   (OVP_Algorithm_MatrixVariance_InputTriggerId_ForceAverage,                "Force average");

				rAlgorithmProto.addOutputTrigger  (OVP_Algorithm_MatrixVariance_OutputTriggerId_AveragePerformed,           "Average performed");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_Algorithm_MatrixVarianceDesc);
		};
	};
};

#endif

#endif // __OpenViBEPlugins_SignalProcessing_Algorithms_Basic_CMatrixVariance_H__
