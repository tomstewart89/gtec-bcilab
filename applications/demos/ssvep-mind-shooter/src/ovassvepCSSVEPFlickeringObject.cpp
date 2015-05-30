
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCSSVEPFlickeringObject.h"
#include <iostream>
#include <cmath>

using namespace OpenViBESSVEP;

// SPCSSVEPFlickeringObject::CSSVEPFlickeringObject(Ogre::SceneNode* poObjectNode, OpenViBE::uint32 ui32LitFrames, OpenViBE::uint32 ui32DarkFrames) :
CSSVEPFlickeringObject::CSSVEPFlickeringObject(Ogre::SceneNode* poObjectNode, OpenViBE::uint64 ui64StimulationPattern):
	m_poObjectNode( poObjectNode ),
	m_ui32CurrentFrame( 0 ),
//SP	m_ui32LitFrames( ui32LitFrames ),
//SP	m_ui32DarkFrames( ui32DarkFrames ),
	m_ui64StimulationPattern( ui64StimulationPattern ),
	m_bVisible( true )
{
	static int l_iNextid = 0;

	m_iId = ++l_iNextid;
}

void CSSVEPFlickeringObject::setVisible( bool bVisibility )
{
	if ( (!m_bVisible && bVisibility) || (m_bVisible && !bVisibility) )
	{
		m_poObjectNode->flipVisibility();
	}
	m_bVisible = bVisibility;
}

void CSSVEPFlickeringObject::processFrame()
{

//SP	if (m_ui32CurrentFrame < m_ui32LitFrames)
	if (((m_ui64StimulationPattern >> m_ui32CurrentFrame) % 2) == 1)
	{
		this->setVisible( true );
	}
	else
	{
		this->setVisible( false );
	}

	m_ui32CurrentFrame++;

//SP	if (m_ui32CurrentFrame == m_ui32LitFrames + m_ui32DarkFrames)
	if (m_ui64StimulationPattern >> m_ui32CurrentFrame == 1)
	{
		m_ui32CurrentFrame = 0;
	}


}

#endif