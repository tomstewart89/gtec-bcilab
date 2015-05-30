#include "ovpCChannelUnitsDecoder.h"

#include <system/ovCMemory.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::StreamCodecs;

// ________________________________________________________________________________________________________________
//

boolean CChannelUnitsDecoder::initialize(void)
{
	CStreamedMatrixDecoder::initialize();

	op_bDynamic.initialize(getOutputParameter(OVP_Algorithm_ChannelUnitsStreamDecoder_OutputParameterId_Dynamic));

	return true;
}

boolean CChannelUnitsDecoder::uninitialize(void)
{
	op_bDynamic.uninitialize();

	CStreamedMatrixDecoder::uninitialize();

	return true;
}

// ________________________________________________________________________________________________________________
//

EBML::boolean CChannelUnitsDecoder::isMasterChild(const EBML::CIdentifier& rIdentifier)
{
	     if(rIdentifier==OVTK_NodeId_Header_ChannelUnits)         { return true; }
	else if(rIdentifier==OVTK_NodeId_Header_ChannelUnits_Dynamic) { return false; }
	return CStreamedMatrixDecoder::isMasterChild(rIdentifier);
}

void CChannelUnitsDecoder::openChild(const EBML::CIdentifier& rIdentifier)
{
	m_vNodes.push(rIdentifier);

	EBML::CIdentifier& l_rTop=m_vNodes.top();

	if((l_rTop==OVTK_NodeId_Header_ChannelUnits)
	 ||(l_rTop==OVTK_NodeId_Header_ChannelUnits_Dynamic)
	 )
	{
	}
	else
	{
		CStreamedMatrixDecoder::openChild(rIdentifier);
	}
}

void CChannelUnitsDecoder::processChildData(const void* pBuffer, const EBML::uint64 ui64BufferSize)
{
	EBML::CIdentifier& l_rTop=m_vNodes.top();

	if((l_rTop==OVTK_NodeId_Header_ChannelUnits)
	 ||(l_rTop==OVTK_NodeId_Header_ChannelUnits_Dynamic)
	 )
	{
		if(l_rTop==OVTK_NodeId_Header_ChannelUnits_Dynamic)
		{
			op_bDynamic=(m_pEBMLReaderHelper->getUIntegerFromChildData(pBuffer, ui64BufferSize)?true:false);
		}

		//if(l_rTop==OVTK_NodeId_Header_ChannelUnits_MeasurementUnit_Unit)    op_pMeasurementUnits->getBuffer()[m_ui32UnitIndex*2  ]=m_pEBMLReaderHelper->getFloatFromChildData(pBuffer, ui64BufferSize);
		//if(l_rTop==OVTK_NodeId_Header_ChannelUnits_MeasurementUnit_Factor)  op_pMeasurementUnits->getBuffer()[m_ui32UnitIndex*2+1]=m_pEBMLReaderHelper->getFloatFromChildData(pBuffer, ui64BufferSize);

	}
	else
	{
		CStreamedMatrixDecoder::processChildData(pBuffer, ui64BufferSize);
	}
}

void CChannelUnitsDecoder::closeChild(void)
{
	EBML::CIdentifier& l_rTop=m_vNodes.top();

	if((l_rTop==OVTK_NodeId_Header_ChannelUnits)
	 ||(l_rTop==OVTK_NodeId_Header_ChannelUnits_Dynamic)
	 )
	{
		//if(l_rTop==OVTK_NodeId_Header_ChannelUnits_MeasurementUnit)
		//{
		//	m_ui32UnitIndex++;
		//}
	}
	else
	{
		CStreamedMatrixDecoder::closeChild();
	}

	m_vNodes.pop();
}
