#ifndef __OpenViBEPlugins_Algorithm_DecisionStrategy_Voting_H__
#define __OpenViBEPlugins_Algorithm_DecisionStrategy_Voting_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "ovpCAlgorithmPairwiseDecision.h"

#define OVP_ClassId_Algorithm_PairwiseDecision_Voting												OpenViBE::CIdentifier(0xA111B830, 0x4679BAFD)
#define OVP_ClassId_Algorithm_PairwiseDecision_VotingDesc											OpenViBE::CIdentifier(0xAC5A39E8, 0x3A57822A)

namespace OpenViBEPlugins
{
	namespace Classification
	{
		/**
		 * @brief The CAlgorithmPairwiseDecisionVoting class
		 * This strategy relies on a basic voting system. If class A beats class B, class A win 1 point and B 0 point. At the end, the vector of
		 * probability is composed by the normalized score of each class.
		 *
		 * Probability required.
		 */
		class CAlgorithmPairwiseDecisionVoting : virtual public OpenViBEPlugins::Classification::CAlgorithmPairwiseDecision
		{

		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean parametrize(void);

			virtual OpenViBE::boolean compute(OpenViBE::IMatrix* pSubClassifierMatrix, OpenViBE::IMatrix* pProbabiltyVector);
			virtual XML::IXMLNode* saveConfiguration(void);
			virtual OpenViBE::boolean loadConfiguration(XML::IXMLNode& rNode);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVP_ClassId_Algorithm_PairwiseDecision_Voting)

		private:
			fClassifierComparison m_fAlgorithmComparison;
		};

		class CAlgorithmPairwiseDecisionVotingDesc : virtual public OpenViBEPlugins::Classification::CAlgorithmPairwiseDecisionDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Pairwise decision strategy based on Voting"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Serrière Guillaume"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("."); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Algorithm_PairwiseDecision_Voting; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Classification::CAlgorithmPairwiseDecisionVoting; }

			virtual OpenViBE::boolean getAlgorithmPrototype(
				OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
			{
				CAlgorithmPairwiseDecisionDesc::getAlgorithmPrototype(rAlgorithmPrototype);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_Algorithm_PairwiseDecision_VotingDesc)
		};
	}
}

#endif //__OpenViBEPlugins_Algorithm_DecisionStrategy_Voting_H__
