#include "ovpCFeatureVectorEncoder.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::StreamCodecs;


boolean CFeatureVectorEncoder::processHeader(void)
{
	if(ip_pMatrix->getDimensionCount() != 1)
	{
		this->getLogManager() << LogLevel_Warning << "Trying to encode a feature vector with more than 1 dimension (" << ip_pMatrix->getDimensionCount() << "). This will violate the Feature Vector stream specification.\n";
	}

	CStreamedMatrixEncoder::processHeader();

	return true;
}
