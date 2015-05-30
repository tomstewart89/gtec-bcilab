#ifndef __OpenViBEToolkit_Algorithm_Pairing_Strategy_H__
#define __OpenViBEToolkit_Algorithm_Pairing_Strategy_H__

#include "../ovtkTAlgorithm.h"
#include "../../ovtkIVector.h"
#include "../../ovtkIFeatureVector.h"
#include "../../ovtkIFeatureVectorSet.h"
#include "ovtkCAlgorithmClassifier.h"

#include <vector>


#define OVTK_ClassId_Algorithm_PairingStrategy                                      OpenViBE::CIdentifier(0xFD3CB444, 0x58F00765)
#define OVTK_ClassId_Algorithm_PairingStrategyDesc                                  OpenViBE::CIdentifier(0x4341B8D6, 0xC65B7BBB)

#define OVTK_Algorithm_PairingStrategy_InputParameterId_SubClassifierAlgorithm      OpenViBE::CIdentifier(0xD9E60DF9, 0x20EC8FC9)
#define OVTK_Algorithm_PairingStrategy_InputParameterId_ClassAmount                 OpenViBE::CIdentifier(0x2452FF63, 0x028133C8)

#define OVTK_Algorithm_PairingStrategy_InputTriggerId_DesignArchitecture            OpenViBE::CIdentifier(0x784A9CDF, 0xA41C27F8)


typedef OpenViBE::int32 (*fClassifierComparison)(OpenViBE::IMatrix&, OpenViBE::IMatrix&);

namespace OpenViBEToolkit
{
	extern OV_API void registerClassificationComparisionFunction(const OpenViBE::CIdentifier& rClassIdentifier, fClassifierComparison pComparision);
	extern OV_API fClassifierComparison getClassificationComparisonFunction(const OpenViBE::CIdentifier& rClassIdentifier);


	class OV_API CAlgorithmPairingStrategy : public CAlgorithmClassifier
	{
	public:

		virtual OpenViBE::boolean process(void);
		virtual void release(void) { delete this; }

		virtual OpenViBE::boolean designArchitecture(const OpenViBE::CIdentifier& rId, OpenViBE::uint32 rClassAmount) = 0;

		virtual OpenViBE::boolean train(const OpenViBEToolkit::IFeatureVectorSet& rFeatureVectorSet)=0;
		virtual OpenViBE::boolean classify(const OpenViBEToolkit::IFeatureVector& rFeatureVector
										   , OpenViBE::float64& rf64Class
										   , OpenViBEToolkit::IVector& rDistanceValue
										   , OpenViBEToolkit::IVector& rProbabilityValue)=0;

		virtual XML::IXMLNode* saveConfiguration(void)=0;
		virtual OpenViBE::boolean loadConfiguration(XML::IXMLNode* pConfiguratioNode)=0;
		_IsDerivedFromClass_(CAlgorithmClassifier, OVTK_ClassId_Algorithm_PairingStrategy)


	protected:
		//  std::vector <OpenViBE::float64> m_fClasses;
		//The vector will be use when the user will be able to specify class label
		OpenViBE::CIdentifier m_oSubClassifierAlgorithmIdentifier;
	};

	class OV_API CAlgorithmPairingStrategyDesc: public OpenViBEToolkit::CAlgorithmClassifierDesc
	{
	public:

		virtual OpenViBE::boolean getAlgorithmPrototype(
				OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
		{
			CAlgorithmClassifierDesc::getAlgorithmPrototype(rAlgorithmPrototype);

			rAlgorithmPrototype.addInputParameter (OVTK_Algorithm_PairingStrategy_InputParameterId_SubClassifierAlgorithm,        "Algorithm Identifier",        OpenViBE::Kernel::ParameterType_Identifier);
			rAlgorithmPrototype.addInputParameter (OVTK_Algorithm_PairingStrategy_InputParameterId_ClassAmount,                   "Number of classes",           OpenViBE::Kernel::ParameterType_UInteger);

			rAlgorithmPrototype.addInputTrigger   (OVTK_Algorithm_PairingStrategy_InputTriggerId_DesignArchitecture,              "Design Architecture");
			return true;
		}

		_IsDerivedFromClass_(OpenViBEToolkit::CAlgorithmClassifierDesc, OVTK_ClassId_Algorithm_PairingStrategyDesc)
	};
}

#endif // OVTKCALGORITHMPAIRINGSTRATEGY_H
