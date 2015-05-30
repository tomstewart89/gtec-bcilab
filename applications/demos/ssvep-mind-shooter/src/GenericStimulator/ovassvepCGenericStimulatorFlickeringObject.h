#ifndef __OpenViBEApplication_CGenericStimulatorFlickeringObject_H__
#define __OpenViBEApplication_CGenericStimulatorFlickeringObject_H__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <Ogre.h>

#include "../ovassvepCSSVEPFlickeringObject.h"
#include "../ovassvepCBasicPainter.h"

/**
 */
namespace OpenViBESSVEP
{
	class CGenericStimulatorApplication;

	class CGenericStimulatorFlickeringObject : public CSSVEPFlickeringObject
	{
		public:
			static CGenericStimulatorFlickeringObject* createGenericFlickeringObject( OpenViBE::uint32 );
			static void initialize( CGenericStimulatorApplication* poApplication );

			void connectToNode( Ogre::SceneNode* poSceneNode );
			void setTarget( OpenViBE::boolean bIsTarget );

			OpenViBE::boolean isTarget()
			{
				return m_poPointer->getVisible();
			}

		private:
			static CGenericStimulatorApplication* m_poApplication;
			static Ogre::SceneNode* m_poParentNode;
			static CBasicPainter* m_poPainter;
			static OpenViBE::float32 m_f32TargetWidth;
			static OpenViBE::float32 m_f32TargetHeight;
			static Ogre::ColourValue m_oLightColour;
			static Ogre::ColourValue m_oDarkColour;

			CGenericStimulatorFlickeringObject(OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::CString sMaterial, OpenViBE::uint32 ui32StimulationPattern);


			Ogre::SceneNode* m_poElementNode;
			Ogre::MovableObject* m_poPointer;
	};
}


#endif // __OpenViBEApplication_CGenericStimulatorFlickeringObject_H__
