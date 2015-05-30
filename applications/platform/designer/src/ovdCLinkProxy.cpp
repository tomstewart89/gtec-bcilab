#include "ovdCLinkProxy.h"
#include "ovdTAttributeHandler.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEDesigner;

CLinkProxy::CLinkProxy(const ILink& rLink)
	:m_pConstLink(&rLink)
	,m_pLink(NULL)
	,m_f64XSource(0)
	,m_f64YSource(0)
	,m_f64XTarget(0)
	,m_f64YTarget(0)
{
	if(m_pConstLink)
	{
		TAttributeHandler l_oAttributeHandler(*m_pConstLink);
		m_f64XSource=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Link_XSourcePosition);
		m_f64YSource=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Link_YSourcePosition);
		m_f64XTarget=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Link_XTargetPosition);
		m_f64YTarget=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Link_YTargetPosition);
	}
}

CLinkProxy::CLinkProxy(IScenario& rScenario, const CIdentifier& rLinkIdentifier)
	:m_pConstLink(rScenario.getLinkDetails(rLinkIdentifier))
	,m_pLink(rScenario.getLinkDetails(rLinkIdentifier))
	,m_f64XSource(0)
	,m_f64YSource(0)
	,m_f64XTarget(0)
	,m_f64YTarget(0)
{
	if(m_pConstLink)
	{
		TAttributeHandler l_oAttributeHandler(*m_pConstLink);
		m_f64XSource=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Link_XSourcePosition);
		m_f64YSource=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Link_YSourcePosition);
		m_f64XTarget=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Link_XTargetPosition);
		m_f64YTarget=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Link_YTargetPosition);
	}
}

CLinkProxy::~CLinkProxy(void)
{
	if(m_pLink)
	{
		TAttributeHandler l_oAttributeHandler(*m_pLink);

		if(l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_XSourcePosition))
			l_oAttributeHandler.setAttributeValue<double>(OV_AttributeId_Link_XSourcePosition, m_f64XSource);
		else
			l_oAttributeHandler.addAttribute<double>(OV_AttributeId_Link_XSourcePosition, m_f64XSource);

		if(l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_YSourcePosition))
			l_oAttributeHandler.setAttributeValue<double>(OV_AttributeId_Link_YSourcePosition, m_f64YSource);
		else
			l_oAttributeHandler.addAttribute<double>(OV_AttributeId_Link_YSourcePosition, m_f64YSource);

		if(l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_XTargetPosition))
			l_oAttributeHandler.setAttributeValue<double>(OV_AttributeId_Link_XTargetPosition, m_f64XTarget);
		else
			l_oAttributeHandler.addAttribute<double>(OV_AttributeId_Link_XTargetPosition, m_f64XTarget);

		if(l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_YTargetPosition))
			l_oAttributeHandler.setAttributeValue<double>(OV_AttributeId_Link_YTargetPosition, m_f64YTarget);
		else
			l_oAttributeHandler.addAttribute<double>(OV_AttributeId_Link_YTargetPosition, m_f64YTarget);
	}
}

CLinkProxy::operator ILink* (void)
{
	return m_pLink;
}

CLinkProxy::operator const ILink* (void)
{
	return m_pConstLink;
}

float64 CLinkProxy::getXSource(void)
{
	return m_f64XSource;
}

float64 CLinkProxy::getYSource(void)
{
	return m_f64YSource;
}

float64 CLinkProxy::getXTarget(void)
{
	return m_f64XTarget;
}

float64 CLinkProxy::getYTarget(void)
{
	return m_f64YTarget;
}

void CLinkProxy::setSource(float64 f64XSource, float64 f64YSource)
{
	m_f64XSource=f64XSource;
	m_f64YSource=f64YSource;
}

void CLinkProxy::setTarget(float64 f64XTarget, float64 f64YTarget)
{
	m_f64XTarget=f64XTarget;
	m_f64YTarget=f64YTarget;
}
