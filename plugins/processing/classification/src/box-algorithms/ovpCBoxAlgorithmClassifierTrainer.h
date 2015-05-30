#ifndef __OpenViBEPlugins_BoxAlgorithm_ClassifierTrainer_H__
#define __OpenViBEPlugins_BoxAlgorithm_ClassifierTrainer_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "ovpCBoxAlgorithmCommonClassifierListener.inl"

#include <map>
#include <vector>
#include <iostream>


#define OVP_ClassId_BoxAlgorithm_ClassifierTrainer       OpenViBE::CIdentifier(0xF3DAE8A8, 0x3B444154)
#define OVP_ClassId_BoxAlgorithm_ClassifierTrainerDesc   OpenViBE::CIdentifier(0xFE277C91, 0x1593B824)

#define OVP_BoxAlgorithm_ClassifierTrainer_CommonSettingsCount 5

namespace{
const char* const c_sTrainTriggerSettingName = "Train trigger";
const char* const c_sFilenameSettingName = "Filename to save configuration to";
const char* const c_sMulticlassStrategySettingName = "Multiclass strategy to apply";
const char* const c_sAlgorithmSettingName = "Algorithm to use";
const char* const c_sKFoldSettingName = "Number of partitions for k-fold cross-validation test";
}


namespace OpenViBEPlugins
{
	namespace Classification
	{
		class CBoxAlgorithmClassifierTrainer : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_ClassifierTrainer)

		protected:

			virtual OpenViBE::boolean train(const size_t uiStartIndex, const size_t uiStopIndex);
			virtual OpenViBE::float64 getAccuracy(const size_t uiStartIndex, const size_t uiStopIndex);

		private:
			OpenViBE::boolean saveConfiguration(void);

		protected:

			std::map < OpenViBE::uint32, OpenViBE::uint32 > m_vFeatureCount;
			std::vector < OpenViBE::uint32 > m_vFeatureVectorIndex;
			std::map < OpenViBE::uint32, OpenViBE::Kernel::IAlgorithmProxy*> m_vFeatureVectorsDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pStimulationsDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pClassifier;
			OpenViBE::uint64 m_ui64TrainStimulation;
			OpenViBE::uint64 m_ui64PartitionCount;

			OpenViBE::Kernel::IAlgorithmProxy* m_pStimulationsEncoder;
			std::map < OpenViBE::CString, OpenViBE::CString> *m_pParameter;

			typedef struct
			{
				OpenViBE::CMatrix* m_pFeatureVectorMatrix;
				OpenViBE::uint64 m_ui64StartTime;
				OpenViBE::uint64 m_ui64EndTime;
				OpenViBE::uint32 m_ui32InputIndex;
			} SFeatureVector;

			std::vector < CBoxAlgorithmClassifierTrainer::SFeatureVector > m_vFeatureVector;
		};

		class CBoxAlgorithmClassifierTrainerDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Classifier trainer"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Yann Renard, Guillaume Serriere"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Generic classifier trainer, relying on several box algorithms"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Performs classifier training with cross-validation -based error estimation"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Classification"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("2.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-apply"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_ClassifierTrainer; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Classification::CBoxAlgorithmClassifierTrainer; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput  ("Stimulations",                         OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput  ("Features for class 1",                 OV_TypeId_FeatureVector);
				rBoxAlgorithmPrototype.addInput  ("Features for class 2",                 OV_TypeId_FeatureVector);

				rBoxAlgorithmPrototype.addOutput ("Train-completed Flag",                 OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addSetting(c_sTrainTriggerSettingName,         OV_TypeId_Stimulation,               "OVTK_StimulationId_Train");
				rBoxAlgorithmPrototype.addSetting(c_sFilenameSettingName,             OV_TypeId_Filename,                  "${Path_UserData}/my-classifier.xml");

				rBoxAlgorithmPrototype.addSetting(c_sMulticlassStrategySettingName,   OVTK_TypeId_ClassificationStrategy,  "Native");
				//Pairing startegy argument
				//Class label

				rBoxAlgorithmPrototype.addSetting(c_sAlgorithmSettingName,            OVTK_TypeId_ClassificationAlgorithm, "Linear Discrimimant Analysis (LDA)");
				//Argument of algorithm

				rBoxAlgorithmPrototype.addSetting(c_sKFoldSettingName,                OV_TypeId_Integer,                   "10");




				rBoxAlgorithmPrototype.addFlag   (OpenViBE::Kernel::BoxFlag_CanAddInput);
				return true;
			}

			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const { return new CBoxAlgorithmCommonClassifierListener(OVP_BoxAlgorithm_ClassifierTrainer_CommonSettingsCount); }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) { delete pBoxListener; }

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_ClassifierTrainerDesc)
		};
	}
}

#endif // __OpenViBEPlugins_BoxAlgorithm_ClassifierTrainer_H__
