#ifndef __OpenViBEPlugins_BoxAlgorithm_ClassifierProcessor_H__
#define __OpenViBEPlugins_BoxAlgorithm_ClassifierProcessor_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <map>

#define OVP_ClassId_BoxAlgorithm_ClassifierProcessor       OpenViBE::CIdentifier(0x5FE23D17, 0x95B0452C)
#define OVP_ClassId_BoxAlgorithm_ClassifierProcessorDesc   OpenViBE::CIdentifier(0x29B66B00, 0xB4683D49)

namespace OpenViBEPlugins
{
	namespace Classification
	{
		class CBoxAlgorithmClassifierProcessor : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_ClassifierProcessor)

		private:

			OpenViBE::Kernel::IAlgorithmProxy* m_pFeaturesDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pLabelsEncoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pClassificationStateEncoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pProbabilityValues;
			OpenViBE::Kernel::IAlgorithmProxy* m_pClassifier;

			std::map < OpenViBE::float64, OpenViBE::uint64 > m_vStimulation;
			OpenViBE::boolean m_bOutputHeaderSent;
		};

		class CBoxAlgorithmClassifierProcessorDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Classifier processor"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Yann Renard, Guillaume Serriere"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Generic classification, relying on several box algorithms"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Classifies incoming feature vectors using a previously learned classifier."); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Classification"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("2.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-apply"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_ClassifierProcessor; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Classification::CBoxAlgorithmClassifierProcessor; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput  ("Features",                            OV_TypeId_FeatureVector);
				rBoxAlgorithmPrototype.addOutput ("Labels",                              OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addOutput ("Hyperplane distance",                 OV_TypeId_StreamedMatrix);
				rBoxAlgorithmPrototype.addOutput ("Probability values",                  OV_TypeId_StreamedMatrix);

				//We load everything in the save filed
				rBoxAlgorithmPrototype.addSetting("Filename to load configuration from", OV_TypeId_Filename,    "");
				return true;
			}

			// virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const { return new CBoxAlgorithmCommonClassifierListener(5); }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) { delete pBoxListener; }

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_ClassifierProcessorDesc)
		};
	}
}

#endif // __OpenViBEPlugins_BoxAlgorithm_ClassifierProcessor_H__
