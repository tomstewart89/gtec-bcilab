#ifndef __OpenViBEPlugins_Algorithm_ClassifierLDA_H__
#define __OpenViBEPlugins_Algorithm_ClassifierLDA_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#if defined TARGET_HAS_ThirdPartyEIGEN

#include <xml/IXMLNode.h>

#include <stack>

#include <Eigen/Dense>

#define OVP_ClassId_Algorithm_ClassifierLDA                                        OpenViBE::CIdentifier(0x2BA17A3C, 0x1BD46D84)
#define OVP_ClassId_Algorithm_ClassifierLDA_DecisionAvailable                      OpenViBE::CIdentifier(0x79146976, 0xD7F01A25)
#define OVP_ClassId_Algorithm_ClassifierLDADesc                                    OpenViBE::CIdentifier(0x78FE2929, 0x644945B4)

#define OVP_Algorithm_ClassifierLDA_InputParameterId_UseShrinkage                  OpenViBE::CIdentifier(0x01357534, 0x028312A0)
#define OVP_Algorithm_ClassifierLDA_InputParameterId_Shrinkage                     OpenViBE::CIdentifier(0x01357534, 0x028312A1)
#define OVP_Algorithm_ClassifierLDA_InputParameterId_DiagonalCov                   OpenViBE::CIdentifier(0x067E45C5, 0x15285CC7)

namespace OpenViBEPlugins
{
	namespace Classification
	{
		OpenViBE::int32 getLDABestClassification(OpenViBE::IMatrix& rFirstClassificationValue, OpenViBE::IMatrix& rSecondClassificationValue);

		class CAlgorithmClassifierLDA : public OpenViBEToolkit::CAlgorithmClassifier
		{
		typedef Eigen::Matrix< double , Eigen::Dynamic , Eigen::Dynamic, Eigen::RowMajor > MatrixXdRowMajor;

		public:

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean train(const OpenViBEToolkit::IFeatureVectorSet& rFeatureVectorSet);
			virtual OpenViBE::boolean classify(const OpenViBEToolkit::IFeatureVector& rFeatureVector
											   , OpenViBE::float64& rf64Class
											   , OpenViBEToolkit::IVector& rDistanceValue
											   , OpenViBEToolkit::IVector& rProbabilityValue);

			virtual XML::IXMLNode* saveConfiguration(void);
			virtual OpenViBE::boolean loadConfiguration(XML::IXMLNode *pConfigurationNode);

			_IsDerivedFromClass_Final_(CAlgorithmClassifier, OVP_ClassId_Algorithm_ClassifierLDA);

		protected:
			// Debug method. Prints the matrix to the logManager. May be disabled in implementation.
			void dumpMatrix(OpenViBE::Kernel::ILogManager& pMgr, const MatrixXdRowMajor& mat, const OpenViBE::CString& desc);

			OpenViBE::float64 m_f64Class1;
			OpenViBE::float64 m_f64Class2;

			Eigen::MatrixXd m_oCoefficients;
			Eigen::MatrixXd m_oWeights;
			OpenViBE::float64 m_f64BiasDistance;
			OpenViBE::float64 m_f64w0;

			OpenViBE::uint32 m_ui32NumCols;

			XML::IXMLNode *m_pConfigurationNode;

			OpenViBE::Kernel::IAlgorithmProxy* m_pCovarianceAlgorithm;

		private:
			void loadClassesFromNode(XML::IXMLNode *pNode);
			void loadCoefficientsFromNode(XML::IXMLNode *pNode);

			void generateConfigurationNode(void);
		};

		class CAlgorithmClassifierLDADesc : public OpenViBEToolkit::CAlgorithmClassifierDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("LDA Classifier"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Jussi T. Lindgren / Guillaume Serrière"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria / Loria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Estimates LDA using regularized or classic covariances"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Algorithm_ClassifierLDA; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Classification::CAlgorithmClassifierLDA; }

			virtual OpenViBE::boolean getAlgorithmPrototype(
				OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
			{
				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_ClassifierLDA_InputParameterId_UseShrinkage, "Use shrinkage", OpenViBE::Kernel::ParameterType_Boolean);
				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_ClassifierLDA_InputParameterId_DiagonalCov,"Shrinkage: Force diagonal cov (DDA)",OpenViBE::Kernel::ParameterType_Boolean);
				rAlgorithmPrototype.addInputParameter(OVP_Algorithm_ClassifierLDA_InputParameterId_Shrinkage, "Shrinkage coefficient (-1 == auto)",OpenViBE::Kernel::ParameterType_Float);


				CAlgorithmClassifierDesc::getAlgorithmPrototype(rAlgorithmPrototype);
				return true;
			}

			_IsDerivedFromClass_Final_(CAlgorithmClassifierDesc, OVP_ClassId_Algorithm_ClassifierLDADesc);
		};
	};
};

#endif // __OpenViBEPlugins_Algorithm_ClassifierLDA_H__

#endif // TARGET_HAS_ThirdPartyEIGEN

