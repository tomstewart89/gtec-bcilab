#include "ovdCBoxProxy.h"
#include "ovdTAttributeHandler.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEDesigner;
using namespace std;

CBoxProxy::CBoxProxy(const IKernelContext& rKernelContext, const IBox& rBox)
	:m_rKernelContext(rKernelContext)
	,m_pConstBox(&rBox)
	,m_pBox(NULL)
	,m_bApplied(false)
	,m_iWidth(-1)
	,m_iHeight(-1)
	,m_f64XCenter(0)
	,m_f64YCenter(0)
{
	if(m_pConstBox)
	{
		TAttributeHandler l_oAttributeHandler(*m_pConstBox);
		m_f64XCenter=l_oAttributeHandler.getAttributeValue<OpenViBE::float64>(OV_AttributeId_Box_XCenterPosition);
		m_f64YCenter=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Box_YCenterPosition);
	}
	m_bShowOriginalNameWhenModified=m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowOriginalBoxName}", true);
}

CBoxProxy::CBoxProxy(const IKernelContext& rKernelContext, IScenario& rScenario, const CIdentifier& rBoxIdentifier)
	:m_rKernelContext(rKernelContext)
	,m_pConstBox(rScenario.getBoxDetails(rBoxIdentifier))
	,m_pBox(rScenario.getBoxDetails(rBoxIdentifier))
	,m_bApplied(false)
	,m_f64XCenter(0)
	,m_f64YCenter(0)
{
	if(m_pConstBox)
	{
		TAttributeHandler l_oAttributeHandler(*m_pConstBox);
		m_f64XCenter=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Box_XCenterPosition);
		m_f64YCenter=l_oAttributeHandler.getAttributeValue<double>(OV_AttributeId_Box_YCenterPosition);
	}
	m_bShowOriginalNameWhenModified=m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowOriginalBoxName}", true);
}

CBoxProxy::~CBoxProxy(void)
{
	if(!m_bApplied)
	{
		this->apply();
	}
}

CBoxProxy::operator IBox* (void)
{
	return m_pBox;
}

CBoxProxy::operator const IBox* (void)
{
	return m_pConstBox;
}

int32 CBoxProxy::getWidth(::GtkWidget* pWidget) const
{
	if(m_iWidth==-1)
	    updateSize(pWidget, getLabel(), &m_iWidth, &m_iHeight);
	return m_iWidth;
}

int32 CBoxProxy::getHeight(::GtkWidget* pWidget) const
{
	if(m_iHeight==-1)
	    updateSize(pWidget, getLabel(), &m_iWidth, &m_iHeight);
	return m_iHeight;
}

float64 CBoxProxy::getXCenter(void) const
{
	return m_f64XCenter;
}

float64 CBoxProxy::getYCenter(void) const
{
	return m_f64YCenter;
}

void CBoxProxy::setCenter(float64 f64XCenter, float64 f64YCenter)
{
	m_f64XCenter=f64XCenter;
	m_f64YCenter=f64YCenter;
	m_bApplied=false;
}

void CBoxProxy::apply(void)
{
	if(m_pBox)
	{
		TAttributeHandler l_oAttributeHandler(*m_pBox);

		if(l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_XCenterPosition))
			l_oAttributeHandler.setAttributeValue<double>(OV_AttributeId_Box_XCenterPosition, m_f64XCenter);
		else
			l_oAttributeHandler.addAttribute<double>(OV_AttributeId_Box_XCenterPosition, m_f64XCenter);

		if(l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_YCenterPosition))
			l_oAttributeHandler.setAttributeValue<double>(OV_AttributeId_Box_YCenterPosition, m_f64YCenter);
		else
			l_oAttributeHandler.addAttribute<double>(OV_AttributeId_Box_YCenterPosition, m_f64YCenter);

		m_bApplied=true;
	}
}

const char* CBoxProxy::getLabel(void) const
{
   if (m_sLabel.size()==0)
   {
	    const boolean l_bBoxCanChangeInput  (m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput)  ||m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanAddInput));
	    const boolean l_bBoxCanChangeOutput (m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput) ||m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanAddOutput));
	    const boolean l_bBoxCanChangeSetting(m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanModifySetting)||m_pConstBox->hasAttribute(OV_AttributeId_Box_FlagCanAddSetting));
	    const boolean l_bBoxIsUpToDate      (this->isBoxAlgorithmPluginPresent()  ? this->isUpToDate() : true);
	    const boolean l_bBoxIsDeprecated    (this->isBoxAlgorithmPluginPresent() && this->isDeprecated());
	    const boolean l_bBoxIsUnstable      (this->isBoxAlgorithmPluginPresent() && this->isUnstable());
	    const boolean l_bIsMuted            (this->getMute());
	    const IPluginObjectDesc* l_pDesc=m_rKernelContext.getPluginManager().getPluginObjectDescCreating(m_pConstBox->getAlgorithmClassIdentifier());

		//not const need to change the & to &amp; (see below)
		string l_sBoxName(m_pConstBox->getName());
	    const string l_sBoxIden(m_pConstBox->getIdentifier().toString());

	    const string l_sRed("#602020");
	    const string l_sGreen("#206020");
	    const string l_sBlue("#202060");
	    const string l_sGrey("#404040");

		//replace & by &amp; so the markup will not mess up the display
		size_t l_oIterator = l_sBoxName.find('&');
		while(l_oIterator!=std::string::npos)
		{
			l_sBoxName.insert(l_oIterator+1, "amp;");
			l_oIterator = l_sBoxName.find('&', l_oIterator+1);
		}

		m_sLabel=l_sBoxName;

	    if(m_pConstBox->getSettingCount()!=0)
	    {
		    m_sLabel="<span weight=\"bold\">"+m_sLabel+"</span>";
	    }

	    if(m_bShowOriginalNameWhenModified)
	    {
		    const string l_sBoxOriginalName(l_pDesc?string(l_pDesc->getName()):l_sBoxName);
		    if(l_sBoxOriginalName!=l_sBoxName)
		    {
			    m_sLabel="<small><i><span foreground=\""+l_sGrey+"\">"+l_sBoxOriginalName+"</span></i></small>\n"+m_sLabel;
		    }
	    }

	    if(l_bBoxCanChangeInput || l_bBoxCanChangeOutput || l_bBoxCanChangeSetting)
	    {
		    m_sLabel+="\n";
		    m_sLabel+="<span size=\"smaller\">";
		    m_sLabel+="<span foreground=\""+(l_bBoxCanChangeInput?l_sGreen:l_sRed)+"\">In</span>";
		    m_sLabel+="|";
		    m_sLabel+="<span foreground=\""+(l_bBoxCanChangeOutput?l_sGreen:l_sRed)+"\">Out</span>";
		    m_sLabel+="|";
		    m_sLabel+="<span foreground=\""+(l_bBoxCanChangeSetting?l_sGreen:l_sRed)+"\">Set</span>";
		    m_sLabel+="</span>";
	    }

	    if(l_bBoxIsDeprecated || l_bBoxIsUnstable || !l_bBoxIsUpToDate || l_bIsMuted)
	    {
		    m_sLabel+="\n";
		    m_sLabel+="<span size=\"smaller\" foreground=\""+l_sBlue+"\">";
		    if(l_bBoxIsDeprecated) m_sLabel+=" <span style=\"italic\">deprecated</span>";
		    if(l_bBoxIsUnstable)   m_sLabel+=" <span style=\"italic\">unstable</span>";
		    if(!l_bBoxIsUpToDate)  m_sLabel+=" <span style=\"italic\">update</span>";
		    if(l_bIsMuted)         m_sLabel+=" <span style=\"italic\">muted</span>";

		    m_sLabel+=" </span>";
	    }
	}

	return m_sLabel.c_str();
}

boolean CBoxProxy::isBoxAlgorithmPluginPresent(void) const
{
	return m_rKernelContext.getPluginManager().canCreatePluginObject(m_pConstBox->getAlgorithmClassIdentifier());
}

boolean CBoxProxy::isUpToDate(void) const
{
	CIdentifier l_oBoxHashCode1;
	CIdentifier l_oBoxHashCode2;
	l_oBoxHashCode1=m_rKernelContext.getPluginManager().getPluginObjectHashValue(m_pConstBox->getAlgorithmClassIdentifier());
	l_oBoxHashCode2.fromString(m_pConstBox->getAttributeValue(OV_AttributeId_Box_InitialPrototypeHashValue));
	return l_oBoxHashCode1==OV_UndefinedIdentifier || (l_oBoxHashCode1!=OV_UndefinedIdentifier && l_oBoxHashCode1==l_oBoxHashCode2);
}

boolean CBoxProxy::isDeprecated(void) const
{
	return m_rKernelContext.getPluginManager().isPluginObjectFlaggedAsDeprecated(m_pConstBox->getAlgorithmClassIdentifier());
}

boolean CBoxProxy::isUnstable(void) const
{
	return m_rKernelContext.getPluginManager().isPluginObjectFlaggedAsUnstable  (m_pConstBox->getAlgorithmClassIdentifier());
}

void CBoxProxy::updateSize(::GtkWidget* pWidget, const char* sText, int* pXSize, int* pYSize) const
{
	::PangoContext* l_pPangoContext=NULL;
	::PangoLayout* l_pPangoLayout=NULL;
	::PangoRectangle l_oPangoRectangle;
	l_pPangoContext=gtk_widget_get_pango_context(pWidget);
	l_pPangoLayout=pango_layout_new(l_pPangoContext);
	pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
	pango_layout_set_markup(l_pPangoLayout, sText, -1);
	pango_layout_get_pixel_extents(l_pPangoLayout, NULL, &l_oPangoRectangle);
	*pXSize=l_oPangoRectangle.width;
	*pYSize=l_oPangoRectangle.height;
	g_object_unref(l_pPangoLayout);
}

boolean CBoxProxy::getMute() const
{
	if(m_pConstBox && m_pConstBox->hasAttribute(OV_AttributeId_Box_Muted))
	{
		TAttributeHandler l_oAttributeHandler(*m_pConstBox);
		if(l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Muted))
		{
			return l_oAttributeHandler.getAttributeValue<bool>(OV_AttributeId_Box_Muted);
		}
		return false;//box not muted by default
	}
	return false; //box not muted by default
}

void CBoxProxy::setMute(boolean bIsMute)
{
	//depending on the constructor, we may have m_pConstBox but not m_pBox
	//could use a const_cast though
	if(m_pBox)
	{
		TAttributeHandler l_oAttributeHandler(*m_pBox);
		if(l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_Muted))
		{
			l_oAttributeHandler.setAttributeValue<bool>(OV_AttributeId_Box_Muted, (bool)bIsMute);
		}
	}
}
