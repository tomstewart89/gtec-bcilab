#include "ovpCFeatureVectorDecoder.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::StreamCodecs;


void CFeatureVectorDecoder::openChild(const EBML::CIdentifier& rIdentifier)
{
	m_oTop = rIdentifier;

	CStreamedMatrixDecoder::openChild(rIdentifier);
}

void CFeatureVectorDecoder::processChildData(const void* pBuffer, const EBML::uint64 ui64BufferSize)
{
	// Check for conforming dimension count, then pass to matrix decoder
	if(m_oTop==OVTK_NodeId_Header_StreamedMatrix_DimensionCount)  
	{ 
		const uint32 l_ui32DimensionCount = (uint32)m_pEBMLReaderHelper->getUIntegerFromChildData(pBuffer, ui64BufferSize);
		if(l_ui32DimensionCount!=1)
		{
			this->getLogManager() << LogLevel_Warning << "Trying to decode a feature vector with more than 1 dimension (" << l_ui32DimensionCount << "). The stream does not seem to conform to specification.\n";
		}
	}

	CStreamedMatrixDecoder::processChildData(pBuffer, ui64BufferSize);
}