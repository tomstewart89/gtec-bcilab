#ifndef __OpenViBEPlugins_Algorithm_PairwiseDecision_H__
#define __OpenViBEPlugins_Algorithm_PairwiseDecision_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <xml/IXMLNode.h>

#define OVP_ClassId_Algorithm_PairwiseDecision												OpenViBE::CIdentifier(0x26EF6DDA, 0xF137053C)
#define OVP_ClassId_Algorithm_PairwiseDecisionDesc											OpenViBE::CIdentifier(0x191EB02A, 0x6866214A)

#define OVP_Algorithm_Classifier_InputParameter_ProbabilityMatrix							OpenViBE::CIdentifier(0xF48D35AD, 0xB8EFF834)
#define OVP_Algorithm_Classifier_Pairwise_InputParameterId_Configuration					OpenViBE::CIdentifier(0x10EBAC09, 0x80926A63)
#define OVP_Algorithm_Classifier_Pairwise_InputParameterId_SetRepartition					OpenViBE::CIdentifier(0xBE71BE18, 0x82A0E018)
#define OVP_Algorithm_Classifier_Pairwise_InputParameterId_AlgorithmIdentifier				OpenViBE::CIdentifier(0xBE71BE18, 0x82A0E017)


#define OVP_Algorithm_Classifier_OutputParameter_ProbabilityVector							OpenViBE::CIdentifier(0x883599FE, 0x2FDB32FF)
#define OVP_Algorithm_Classifier_Pairwise_OutputParameterId_Configuration					OpenViBE::CIdentifier(0x69F05A61, 0x25C94515)

#define OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Train								OpenViBE::CIdentifier(0x32219D21, 0xD3BE6105)
#define OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Parametrize						OpenViBE::CIdentifier(0x32219D21, 0xD3BE6106)
#define OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Compute							OpenViBE::CIdentifier(0x3637344B, 0x05D03D7E)
#define OVP_Algorithm_Classifier_Pairwise_InputTriggerId_SaveConfiguration					OpenViBE::CIdentifier(0xF19574AD, 0x024045A7)
#define OVP_Algorithm_Classifier_Pairwise_InputTriggerId_LoadConfiguration					OpenViBE::CIdentifier(0x97AF6C6C, 0x670A12E6)

namespace OpenViBEPlugins
{
	namespace Classification
	{
		/**
		 * @brief The CAlgorithmPairwiseDecision class
		 * This is the default class for every decision usable with the One Vs One pairwise strategy.
		 */
		class CAlgorithmPairwiseDecision : virtual public OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >
		{

		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void)=0;
			virtual OpenViBE::boolean uninitialize(void)=0;

			virtual OpenViBE::boolean parametrize(void)=0;

			virtual OpenViBE::boolean compute(OpenViBE::IMatrix* pSubClassifierMatrix, OpenViBE::IMatrix* pProbabiltyVector) =0;
			virtual XML::IXMLNode* saveConfiguration(void) = 0;
			virtual OpenViBE::boolean loadConfiguration(XML::IXMLNode& rNode) = 0;

			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVP_ClassId_Algorithm_PairwiseDecision)
		};

		class CAlgorithmPairwiseDecisionDesc : virtual public OpenViBE::Plugins::IAlgorithmDesc
		{
		public:
			virtual OpenViBE::boolean getAlgorithmPrototype(
				OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
			{
				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_Classifier_InputParameter_ProbabilityMatrix, "Probability Matrix", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_Configuration, "Configuration node", OpenViBE::Kernel::ParameterType_Pointer);
				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_SetRepartition, "Set repartition", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_AlgorithmIdentifier, "Classification Algorithm", OpenViBE::Kernel::ParameterType_Identifier);

				rAlgorithmPrototype.addOutputParameter(OVP_Algorithm_Classifier_OutputParameter_ProbabilityVector, "Probability Vector", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmPrototype.addOutputParameter(OVP_Algorithm_Classifier_Pairwise_OutputParameterId_Configuration, "Configuration node", OpenViBE::Kernel::ParameterType_Pointer);


				rAlgorithmPrototype.addInputTrigger(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Compute, "Compute");
				rAlgorithmPrototype.addInputTrigger(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Parametrize, "Parametrize");
				rAlgorithmPrototype.addInputTrigger(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_SaveConfiguration, "Save configuration");
				rAlgorithmPrototype.addInputTrigger(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_LoadConfiguration, "Load configuration");
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_Algorithm_PairwiseDecisionDesc)
		};
	}
}



#endif // __OpenViBEPlugins_Algorithm_PairwiseStrategy_PKPD_H__
