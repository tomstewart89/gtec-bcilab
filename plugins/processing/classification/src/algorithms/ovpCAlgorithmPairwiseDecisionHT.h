#ifndef __OpenViBEPlugins_Algorithm_PairwiseDecision_HT_H__
#define __OpenViBEPlugins_Algorithm_PairwiseDecision_HT_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "ovpCAlgorithmPairwiseDecision.h"

#define OVP_ClassId_Algorithm_PairwiseDecision_HT												OpenViBE::CIdentifier(0xD24F7F19, 0xA744FAD2)
#define OVP_ClassId_Algorithm_PairwiseDecision_HTDesc											OpenViBE::CIdentifier(0xE837F5C0, 0xF65C1341)

namespace OpenViBEPlugins
{
	namespace Classification
	{	/**
		 * @brief The CAlgorithmPairwiseDecisionHT class is a decision strategy for the One Vs One pairwise decision that implement the
		 * method describe in the article Hastie, Trevor; Tibshirani, Robert. Classification by pairwise coupling. The Annals of Statistics 26 (1998), no. 2, 451--471
		 *
		 * Probability required
		 */
		class CAlgorithmPairwiseDecisionHT : virtual public OpenViBEPlugins::Classification::CAlgorithmPairwiseDecision
		{

		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean parametrize(void);

			virtual OpenViBE::boolean compute(OpenViBE::IMatrix* pSubClassifierMatrix, OpenViBE::IMatrix* pProbabiltyVector);
			virtual XML::IXMLNode* saveConfiguration(void);
			virtual OpenViBE::boolean loadConfiguration(XML::IXMLNode& rNode);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVP_ClassId_Algorithm_PairwiseDecision_HT)
		};

		class CAlgorithmPairwiseDecisionHTDesc : virtual public OpenViBEPlugins::Classification::CAlgorithmPairwiseDecisionDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Pairwise decision strategy based on HT"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Serrière Guillaume"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("."); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("This method is based on the method describe in the article "
																									"Hastie, Trevor; Tibshirani, Robert. Classification by pairwise coupling."
																									"The Annals of Statistics 26 (1998), no. 2, 451--471"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Algorithm_PairwiseDecision_HT; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Classification::CAlgorithmPairwiseDecisionHT; }

			virtual OpenViBE::boolean getAlgorithmPrototype(
				OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
			{
				CAlgorithmPairwiseDecisionDesc::getAlgorithmPrototype(rAlgorithmPrototype);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_Algorithm_PairwiseDecision_HTDesc)
		};
	}
}

#endif //__OpenViBEPlugins_Algorithm_PairwiseDecision_HT_H__
