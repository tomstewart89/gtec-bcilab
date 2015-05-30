#ifndef __OpenViBEApplication_CBasicPainter_H__
#define __OpenViBEApplication_CBasicPainter_H__

/// this class is a wrapper around some ogre methods to display basic shapes

#include <Ogre.h>

#define SSVEP_DEFAULT_COLOUR Ogre::ColourValue(1.0f, 1.0f, 1.0f)

namespace OpenViBESSVEP
{
	class CApplication;

	class CBasicPainter
	{
		public:
			CBasicPainter( CApplication* poApplication );
			~CBasicPainter() {}

			Ogre::SceneManager* getSceneManager()
			{
				return m_poSceneManager;
			}

			Ogre::ManualObject* paintRectangle( Ogre::Rectangle oRectangle, Ogre::ColourValue oColour = SSVEP_DEFAULT_COLOUR, int iPlane = 1 );
			Ogre::ManualObject* paintTexturedRectangle( Ogre::Rectangle oRectangle, Ogre::String sSurface, int iPlane = 1 );
			Ogre::ManualObject* paintTriangle( Ogre::Vector2 oP1, Ogre::Vector2 oP2, Ogre::Vector2 oP3, Ogre::ColourValue oColour = SSVEP_DEFAULT_COLOUR, int iPlane = 1 );
			Ogre::ManualObject* paintTriangle( Ogre::Vector3 oP1, Ogre::Vector3 oP2, Ogre::Vector3 oP3, Ogre::ColourValue oColour = SSVEP_DEFAULT_COLOUR, int iPlane = 1 );
			Ogre::ManualObject* paintTexturedTriangle( Ogre::Vector2 oP1, Ogre::Vector2 oP2, Ogre::Vector2 oP3, Ogre::String sSurface, int iPlane = 1 );
			Ogre::ManualObject* paintCircle( Ogre::Real rX, Ogre::Real rY, Ogre::Real rR, Ogre::ColourValue oColour = SSVEP_DEFAULT_COLOUR, bool bFilled = true, int iPlane = 1);

			Ogre::ManualObject* beginPainingPolygon( bool bFilled = true, Ogre::String sMaterial = "BasicSurface/Diffuse" );
			void addPointToPolygon( Ogre::ManualObject* pPolygon, Ogre::Real rX, Ogre::Real rY, Ogre::ColourValue oColour = SSVEP_DEFAULT_COLOUR );
			void finishPaintingPolygon( Ogre::ManualObject* pPolygon, int iPlane = 1 );
			
			void paintText( 
					const std::string& sID,
					const std::string& sText,
					Ogre::Real rX, Ogre::Real rY,
					Ogre::Real rWidth, Ogre::Real rHeight,
					const Ogre::ColourValue& oColour );

		protected:
			CApplication* m_poApplication;
			Ogre::OverlayManager* m_poOverlayManager;
			Ogre::OverlayContainer* m_poOverlayContainer;
			Ogre::SceneManager* m_poSceneManager;
			Ogre::AxisAlignedBox m_oAABInf;


	};

}


#endif // __OpenViBEApplication_CPainter_H__
