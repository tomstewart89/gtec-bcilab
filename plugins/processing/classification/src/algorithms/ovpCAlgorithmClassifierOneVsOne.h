#ifndef __OpenViBEPlugins_Algorithm_OneVsOne_H__
#define __OpenViBEPlugins_Algorithm_OneVsOne_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <xml/IXMLNode.h>

#include <vector>

#define OVP_ClassId_Algorithm_ClassifierOneVsOne								OpenViBE::CIdentifier(0x638C2F90, 0xEAE10226)
#define OVP_ClassId_Algorithm_ClassifierOneVsOneDesc							OpenViBE::CIdentifier(0xE78E7CDB, 0x369AA9EF)

#define OVP_Algorithm_OneVsOneStrategy_InputParameterId_DecisionType			OpenViBE::CIdentifier(0x0C347BBA, 0x180577F9)

namespace OpenViBEPlugins
{
	namespace Classification
	{
		extern OV_API void registerAvailableDecisionEnumeration(const OpenViBE::CIdentifier& rAlgorithmIdentifier, OpenViBE::CIdentifier pDecision);
		extern OV_API OpenViBE::CIdentifier getAvailableDecisionEnumeration(const OpenViBE::CIdentifier& rAlgorithmIdentifier);


		typedef struct{
			OpenViBE::float64 m_f64FirstClass;
			OpenViBE::float64 m_f64SecondClass;
			OpenViBE::Kernel::IAlgorithmProxy* m_pSubClassifierProxy;
		} SSubClassifierDescriptor;


		class CAlgorithmClassifierOneVsOne : public OpenViBEToolkit::CAlgorithmPairingStrategy
		{
		public:

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean train(const OpenViBEToolkit::IFeatureVectorSet& rFeatureVectorSet);
			virtual OpenViBE::boolean classify(const OpenViBEToolkit::IFeatureVector& rFeatureVector
											   , OpenViBE::float64& rf64Class
											   , OpenViBEToolkit::IVector& rDistanceValue
											   , OpenViBEToolkit::IVector& rProbabilityValue);
			virtual OpenViBE::boolean designArchitecture(const OpenViBE::CIdentifier& rId, OpenViBE::uint32 rClassAmount);

			virtual XML::IXMLNode* saveConfiguration(void);
			virtual OpenViBE::boolean loadConfiguration(XML::IXMLNode *pConfigurationNode);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::CAlgorithmPairingStrategy, OVP_ClassId_Algorithm_ClassifierOneVsOne)

		private:
			std::vector<SSubClassifierDescriptor> m_oSubClassifierDescriptorList;
			XML::IXMLNode *m_pConfigurationNode;
			fClassifierComparison m_fAlgorithmComparison;

			OpenViBE::Kernel::IAlgorithmProxy* m_pDecisionStrategyAlgorithm;
			OpenViBE::CIdentifier m_oPairwiseDecisionIdentifier;

			XML::IXMLNode* getClassifierConfiguration(SSubClassifierDescriptor &rDescriptor);
			XML::IXMLNode* getPairwiseDecisionConfiguration(void);
			OpenViBE::uint32 getClassAmount(void) const;

			OpenViBE::boolean loadSubClassifierConfiguration(XML::IXMLNode *pSubClassifiersNode);
			void generateConfigurationNode(void);

			SSubClassifierDescriptor& getSubClassifierDescriptor(const OpenViBE::uint32 f64FirstClass, const OpenViBE::uint32 f64SecondClass);
			OpenViBE::boolean setSubClassifierIdentifier(const OpenViBE::CIdentifier &rId);
		};

		class CAlgorithmClassifierOneVsOneDesc : public OpenViBEToolkit::CAlgorithmPairingStrategyDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("OneVsOne pairing classifier"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Guillaume Serriere"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/Loria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Algorithm_ClassifierOneVsOne; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Classification::CAlgorithmClassifierOneVsOne; }

			virtual OpenViBE::boolean getAlgorithmPrototype(
					OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
			{
				CAlgorithmPairingStrategyDesc::getAlgorithmPrototype(rAlgorithmPrototype);
				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_OneVsOneStrategy_InputParameterId_DecisionType,"Pairwise Decision Strategy",
													  OpenViBE::Kernel::ParameterType_Enumeration, OVP_TypeId_ClassificationPairwiseStrategy);
				return true;
			}

			_IsDerivedFromClass_Final_(CAlgorithmPairingStrategyDesc, OVP_ClassId_Algorithm_ClassifierOneVsOneDesc)
		};
	}
}

#endif // __OpenViBEPlugins_Algorithm_OneVsAll_H__
