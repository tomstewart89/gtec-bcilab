
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCBasicPainter.h"
#include "ovassvepCApplication.h"


using namespace Ogre;
using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;

	CBasicPainter::CBasicPainter( CApplication* poApplication ) :
		m_poApplication( poApplication ),
		m_poSceneManager( poApplication->getSceneManager() )
{
	m_oAABInf.setInfinite();

	m_poOverlayManager = Ogre::OverlayManager::getSingletonPtr();

	(m_poApplication->getLogManager()) << LogLevel_Debug << "  + Creating OverlayManager\n";
	Overlay* l_poOverlay = m_poOverlayManager->create("TextOverlay");

	
	m_poOverlayContainer = static_cast<Ogre::OverlayContainer*>(m_poOverlayManager->createOverlayElement("Panel", "TextContainer"));
	m_poOverlayContainer->setDimensions(1, 1);
	m_poOverlayContainer->setPosition(0, 0);

	l_poOverlay->add2D( m_poOverlayContainer );
	l_poOverlay->show();
	
}



ManualObject* CBasicPainter::paintRectangle( Ogre::Rectangle oRectangle, Ogre::ColourValue oColour, int iPlane )
{
	ManualObject *l_poObject;

	l_poObject = m_poSceneManager->createManualObject();
	l_poObject->begin("BasicSurface/Diffuse", RenderOperation::OT_TRIANGLE_FAN);
	l_poObject->setUseIdentityProjection(true);
	l_poObject->setUseIdentityView(true);

	l_poObject->position(oRectangle.right, oRectangle.top, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(0);

	l_poObject->position(oRectangle.left, oRectangle.top, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(1);

  	l_poObject->position(oRectangle.left, oRectangle.bottom, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(2);

	l_poObject->position(oRectangle.right, oRectangle.bottom, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(3);

	l_poObject->index(0);

	l_poObject->end();

	l_poObject->setBoundingBox( m_oAABInf );
	l_poObject->setRenderQueueGroup( RENDER_QUEUE_OVERLAY - iPlane );

	l_poObject->setVisible(true);

	return l_poObject;
}

ManualObject* CBasicPainter::paintTexturedRectangle( Ogre::Rectangle oRectangle, Ogre::String sSurface, int iPlane)
{
	ManualObject *l_poObject;

	l_poObject = m_poSceneManager->createManualObject();
	l_poObject->begin(sSurface, RenderOperation::OT_TRIANGLE_FAN);
	l_poObject->setUseIdentityProjection(true);
	l_poObject->setUseIdentityView(true);

	l_poObject->position(oRectangle.right, oRectangle.top, 0.0);
	l_poObject->textureCoord(0, 0);
	l_poObject->index(0);

	l_poObject->position(oRectangle.left, oRectangle.top, 0.0);
	l_poObject->textureCoord(1, 0);
	l_poObject->index(1);

	l_poObject->position(oRectangle.left, oRectangle.bottom, 0.0);
	l_poObject->textureCoord(1, 1);
	l_poObject->index(2);

	l_poObject->position(oRectangle.right, oRectangle.bottom, 0.0);
	l_poObject->textureCoord(0, 1);
	l_poObject->index(3);

	l_poObject->index(0);

	l_poObject->end();

	l_poObject->setBoundingBox( m_oAABInf );
	l_poObject->setRenderQueueGroup( RENDER_QUEUE_OVERLAY - iPlane );

	l_poObject->setVisible(true);

	return l_poObject;
}


ManualObject* CBasicPainter::paintTriangle( Ogre::Vector2 oP1, Ogre::Vector2 oP2, Ogre::Vector2 oP3, Ogre::ColourValue oColour, int iPlane )
{
	ManualObject *l_poObject;

	l_poObject = m_poSceneManager->createManualObject();
	l_poObject->begin("BasicSurface/Diffuse", RenderOperation::OT_TRIANGLE_FAN);
	l_poObject->setUseIdentityProjection(true);
	l_poObject->setUseIdentityView(true);

	l_poObject->position(oP1.x, oP1.y, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(0);

	l_poObject->position(oP2.x, oP2.y, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(1);

	l_poObject->position(oP3.x, oP3.y, 0.0);
	l_poObject->colour(oColour);
	l_poObject->index(2);

	l_poObject->index(0);

	l_poObject->end();

	l_poObject->setBoundingBox( m_oAABInf );
	l_poObject->setRenderQueueGroup( RENDER_QUEUE_OVERLAY - iPlane );

	l_poObject->setVisible(true);

	return l_poObject;

}

ManualObject* CBasicPainter::paintTriangle( Ogre::Vector3 oP1, Ogre::Vector3 oP2, Ogre::Vector3 oP3, Ogre::ColourValue oColour, int iPlane )
{
	ManualObject *l_poObject;

	l_poObject = m_poSceneManager->createManualObject();
	l_poObject->begin("BasicSurface/Diffuse", RenderOperation::OT_TRIANGLE_FAN);
	l_poObject->setUseIdentityProjection(true);
	l_poObject->setUseIdentityView(true);

	l_poObject->position(oP1.x, oP1.y, oP1.z);
	l_poObject->colour(oColour);
	l_poObject->index(0);

	l_poObject->position(oP2.x, oP2.y, oP2.z);
	l_poObject->colour(oColour);
	l_poObject->index(1);

	l_poObject->position(oP3.x, oP3.y, oP3.z);
	l_poObject->colour(oColour);
	l_poObject->index(2);

	l_poObject->index(0);

	l_poObject->end();

	l_poObject->setBoundingBox( m_oAABInf );
	l_poObject->setRenderQueueGroup( RENDER_QUEUE_OVERLAY - iPlane );

	l_poObject->setVisible(true);

	return l_poObject;

}
Ogre::ManualObject* CBasicPainter::paintCircle( Ogre::Real rX, Ogre::Real rY, Ogre::Real rR, Ogre::ColourValue oColour, bool bFilled, int iPlane )
{
	ManualObject *l_poObject;

	l_poObject = m_poSceneManager->createManualObject();

	if (bFilled)
	{
		l_poObject->begin("BasicSurface/Diffuse", RenderOperation::OT_TRIANGLE_FAN);
	}
	else
	{
		l_poObject->begin("BasicSurface/Diffuse", RenderOperation::OT_LINE_STRIP);
	}

	l_poObject->setUseIdentityProjection(true);
	l_poObject->setUseIdentityView(true);

    float const fAccuracy = 16;
	unsigned uiIndex = 0;

	for (float theta = 0; theta <= 2 * Math::PI; theta += Math::PI / fAccuracy)
	{
		l_poObject->position(rX + rR * cos(theta), rY + rR * sin(theta), 0);
		l_poObject->colour(oColour);
		l_poObject->index(uiIndex++);
	}

	l_poObject->index(0);

	l_poObject->end();

	l_poObject->setBoundingBox( m_oAABInf );
	l_poObject->setRenderQueueGroup( RENDER_QUEUE_OVERLAY - iPlane );

	l_poObject->setVisible( true );

	return l_poObject;
}

ManualObject* CBasicPainter::beginPainingPolygon(bool bFilled, Ogre::String sMaterial )
{
	ManualObject *l_poObject;
	l_poObject = m_poSceneManager->createManualObject();
	l_poObject->begin(sMaterial, RenderOperation::OT_TRIANGLE_FAN);
	l_poObject->setUseIdentityProjection(true);
	l_poObject->setUseIdentityView(true);

	return l_poObject;

}

void CBasicPainter::addPointToPolygon(Ogre::ManualObject *pPolygon, Ogre::Real rX, Ogre::Real rY, Ogre::ColourValue oColour)
{
	pPolygon->position(rX, rY, 0.0);
	pPolygon->colour(oColour);
}

void CBasicPainter::finishPaintingPolygon(Ogre::ManualObject *pPolygon, int iPlane)
{
	pPolygon->end();
	pPolygon->setBoundingBox( m_oAABInf );
	pPolygon->setRenderQueueGroup( RENDER_QUEUE_OVERLAY - iPlane );

	pPolygon->setVisible( true );
}


void CBasicPainter::paintText(
		const std::string& sID,
		const std::string& sText,
		Ogre::Real rX, Ogre::Real rY,
		Ogre::Real rWidth, Ogre::Real rHeight,
		const Ogre::ColourValue& oColour )
{
	Ogre::OverlayElement* l_pText = m_poOverlayManager->createOverlayElement("TextArea", sID);

	l_pText->setDimensions(rWidth, rHeight);
	l_pText->setMetricsMode(Ogre::GMM_PIXELS);
	l_pText->setPosition(rX, rY);


	l_pText->setParameter("font_name", "DejaVu");

	l_pText->setColour(oColour);

	l_pText->setCaption(sText);

	m_poOverlayContainer->addChild(l_pText);

}




#endif