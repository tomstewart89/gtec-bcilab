#include "ovkCMessageLink.h"
#include "ovkCScenario.h"

#include "../ovkCObjectVisitorContext.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;

//___________________________________________________________________//
//                                                                   //

CMessageLink::CMessageLink(const IKernelContext& rKernelContext, CScenario& rOwnerScenario)
	:TAttributable < TKernelObject < ILink > >(rKernelContext)
	,m_rOwnerScenario(rOwnerScenario)
	,m_oIdentifier(OV_UndefinedIdentifier)
	,m_oSourceBoxIdentifier(OV_UndefinedIdentifier)
	,m_oTargetBoxIdentifier(OV_UndefinedIdentifier)
	,m_ui32SourceOutputIndex(0)
	,m_ui32TargetInputIndex(0)
{
}

//___________________________________________________________________//
//                                                                   //

boolean CMessageLink::setIdentifier(
	const CIdentifier& rIdentifier)
{
	m_oIdentifier=rIdentifier;
	return true;
}

CIdentifier CMessageLink::getIdentifier(void) const
{
	return m_oIdentifier;
}

//___________________________________________________________________//
//                                                                   //

boolean CMessageLink::setSource(
	const CIdentifier& rBoxIdentifier,
	const uint32 ui32BoxOutputIndex)
{
	m_oSourceBoxIdentifier=rBoxIdentifier;
	m_ui32SourceOutputIndex=ui32BoxOutputIndex;
	return true;
}

boolean CMessageLink::setTarget(
	const CIdentifier& rBoxIdentifier,
	const uint32 ui32BoxInputIndex)
{
	m_oTargetBoxIdentifier=rBoxIdentifier;
	m_ui32TargetInputIndex=ui32BoxInputIndex;
	return true;
}

boolean CMessageLink::getSource(
	CIdentifier& rBoxIdentifier,
	uint32& ui32BoxOutputIndex) const
{
	rBoxIdentifier=m_oSourceBoxIdentifier;
	ui32BoxOutputIndex=m_ui32SourceOutputIndex;
	return true;
}

CIdentifier CMessageLink::getSourceBoxIdentifier(void) const
{
	return m_oSourceBoxIdentifier;
}

uint32 CMessageLink::getSourceBoxOutputIndex(void) const
{
	return m_ui32SourceOutputIndex;
}

boolean CMessageLink::getTarget(
	CIdentifier& rTargetBoxIdentifier,
	uint32& ui32BoxInputIndex) const
{
	rTargetBoxIdentifier=m_oTargetBoxIdentifier;
	ui32BoxInputIndex=m_ui32TargetInputIndex;
	return true;
}

CIdentifier CMessageLink::getTargetBoxIdentifier(void) const
{
	return m_oTargetBoxIdentifier;
}

uint32 CMessageLink::getTargetBoxInputIndex(void) const
{
	return m_ui32TargetInputIndex;
}

//___________________________________________________________________//
//                                                                   //

boolean CMessageLink::acceptVisitor(
	IObjectVisitor& rObjectVisitor)
{
	CObjectVisitorContext l_oObjectVisitorContext(getKernelContext());
	return rObjectVisitor.processBegin(l_oObjectVisitorContext, *this) && rObjectVisitor.processEnd(l_oObjectVisitorContext, *this);
}
