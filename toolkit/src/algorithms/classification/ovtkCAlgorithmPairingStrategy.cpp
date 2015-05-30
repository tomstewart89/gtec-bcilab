#include "ovtkCAlgorithmClassifier.h"
#include "ovtkCAlgorithmPairingStrategy.h"

#include <map>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEToolkit;

static std::map<uint64, fClassifierComparison> mComparisionFunctionMap;

void OpenViBEToolkit::registerClassificationComparisionFunction(const OpenViBE::CIdentifier& rClassIdentifier, fClassifierComparison pComparision)
{
	mComparisionFunctionMap[rClassIdentifier.toUInteger()] = pComparision;
}

fClassifierComparison OpenViBEToolkit::getClassificationComparisonFunction(const OpenViBE::CIdentifier& rClassIdentifier)
{
	if(mComparisionFunctionMap.count(rClassIdentifier.toUInteger()) == 0)
		return NULL;
	else
		return mComparisionFunctionMap[rClassIdentifier.toUInteger()];
}

boolean CAlgorithmPairingStrategy::process(void)
{
	if(this->isInputTriggerActive(OVTK_Algorithm_PairingStrategy_InputTriggerId_DesignArchitecture))
	{
		TParameterHandler < CIdentifier* > ip_pClassifierIdentifier(this->getInputParameter(OVTK_Algorithm_PairingStrategy_InputParameterId_SubClassifierAlgorithm));
		TParameterHandler < uint64 > ip_pClassAmount(this->getInputParameter(OVTK_Algorithm_PairingStrategy_InputParameterId_ClassAmount));

		const uint64 l_ui64ClassCount = (uint64) ip_pClassAmount;
		const CIdentifier l_oClassifierIdentifier = *((CIdentifier*)ip_pClassifierIdentifier);
		if(this->designArchitecture(l_oClassifierIdentifier, static_cast<uint32>(l_ui64ClassCount)))
		{
			this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Success, true);
		}
		else
		{
			this->activateOutputTrigger(OVTK_Algorithm_Classifier_OutputTriggerId_Failed, true);
			return false;
		}
	}
	else
	{
		return CAlgorithmClassifier::process();
	}
	return true;
}
