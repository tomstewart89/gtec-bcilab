
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCGenericStimulatorFlickeringObject.h"
#include "ovassvepCGenericStimulatorApplication.h"

#include <cmath>

using namespace Ogre;
using namespace OpenViBESSVEP;

SceneNode* CGenericStimulatorFlickeringObject::m_poParentNode = NULL;
CBasicPainter* CGenericStimulatorFlickeringObject::m_poPainter = NULL;
ColourValue CGenericStimulatorFlickeringObject::m_oLightColour = ColourValue(1.0f, 1.0f, 1.0f);
ColourValue CGenericStimulatorFlickeringObject::m_oDarkColour = ColourValue(0.0f, 0.0f, 0.0f);
OpenViBE::float32 CGenericStimulatorFlickeringObject::m_f32TargetWidth = 0.2f;
OpenViBE::float32 CGenericStimulatorFlickeringObject::m_f32TargetHeight = 0.2f;
CGenericStimulatorApplication* CGenericStimulatorFlickeringObject::m_poApplication = NULL;

void CGenericStimulatorFlickeringObject::initialize( CGenericStimulatorApplication* poApplication )
{
	m_poApplication = poApplication;
	OpenViBE::Kernel::IConfigurationManager* l_poConfigurationManager = poApplication->getConfigurationManager();

	m_poPainter = poApplication->getPainter();
	m_poParentNode = poApplication->getSceneNode();

	m_f32TargetWidth = (OpenViBE::float32)(l_poConfigurationManager->expandAsFloat("${SSVEP_TargetWidth}"));
	m_f32TargetHeight = (OpenViBE::float32)(l_poConfigurationManager->expandAsFloat("${SSVEP_TargetHeight}"));

	m_oLightColour = ColourValue(
			(float)(l_poConfigurationManager->expandAsFloat("${SSVEP_TargetLightColourRed}")),
			(float)(l_poConfigurationManager->expandAsFloat("${SSVEP_TargetLightColourGreen}")),
			(float)(l_poConfigurationManager->expandAsFloat("${SSVEP_TargetLightColourBlue}")));

	m_oDarkColour = ColourValue(
			(float)(l_poConfigurationManager->expandAsFloat("${SSVEP_TargetDarkColourRed}")),
			(float)(l_poConfigurationManager->expandAsFloat("${SSVEP_TargetDarkColourGreen}")),
			(float)(l_poConfigurationManager->expandAsFloat("${SSVEP_TargetDarkColourBlue}")));

	m_poApplication->getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Target Size : ("
									 << m_f32TargetWidth << ", " << m_f32TargetHeight << ")\n";
	/*
	m_poApplication->getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Target Default Colour : Light = "
									 << Ogre::String(m_oLightColour) << ", Dark = " << Ogre::String(m_oDarkColour) << "\n";
									 */


}

CGenericStimulatorFlickeringObject* CGenericStimulatorFlickeringObject::createGenericFlickeringObject( OpenViBE::uint32 l_i32TargetId )
{
	OpenViBE::Kernel::IConfigurationManager* l_poConfigurationManager = m_poApplication->getConfigurationManager();

	if (m_poPainter != NULL)
	{
		ColourValue l_oCurrentTargetColour = ( l_i32TargetId == 0) ? m_oDarkColour : m_oLightColour;

		char l_sTargetIdString[255]; // this is an overkill
		sprintf(l_sTargetIdString, "%d", l_i32TargetId);

		OpenViBE::CIdentifier l_oTargetId = l_poConfigurationManager->createConfigurationToken("SSVEPTarget_Id", OpenViBE::CString(l_sTargetIdString));

		OpenViBE::float32 l_f32TargetX = (OpenViBE::float32)(l_poConfigurationManager->expandAsFloat("${SSVEP_Target_X_${SSVEPTarget_Id}}"));
		OpenViBE::float32 l_f32TargetY = (OpenViBE::float32)(l_poConfigurationManager->expandAsFloat("${SSVEP_Target_Y_${SSVEPTarget_Id}}"));
		OpenViBE::CString l_sMaterial = (OpenViBE::CString)(l_poConfigurationManager->expand("${SSVEP_Target_Material_${SSVEPTarget_Id}}"));
//SP		OpenViBE::uint32 l_ui32FramesL = (*(m_poApplication->getFrequencies()))[ l_i32TargetId ].first;
//SP		OpenViBE::uint32 l_ui32FramesD = (*(m_poApplication->getFrequencies()))[ l_i32TargetId ].second;
		OpenViBE::uint32 l_ui32StimulationPattern = (OpenViBE::uint32) (*(m_poApplication->getFrequencies()))[ l_i32TargetId ];

//SP		m_poApplication->getLogManager() << OpenViBE::Kernel::LogLevel_Info << "New trainer object : id=" << l_i32TargetId << " litFrames=" << l_ui32FramesL << " darkFrames=" << l_ui32FramesD << "\n";
		/*
		m_poApplication->getLogManager()
				<< OpenViBE::Kernel::LogLevel_Info
				<< "New trainer object : id=" << l_i32TargetId
				<< " stimulationPattern=" << l_ui32StimulationPattern
				<< ", length=" << int(ceil(log2(l_ui32StimulationPattern)) - 1)
				<< ", frequency@60=" << 60 / (ceil(log2(l_ui32StimulationPattern)) - 1) << "\n";
*/

		m_poApplication->getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Target (" << l_i32TargetId << ") : Position = (" << l_f32TargetX << ", " << l_f32TargetY << "), Texture = " << l_sMaterial << "\n";

		l_poConfigurationManager->releaseConfigurationToken(l_oTargetId);

//SP		return new CTrainerFlickeringObject( l_f32TargetX, l_f32TargetY, l_oCurrentTargetColour, l_ui32FramesL, l_ui32FramesD );
		return new CGenericStimulatorFlickeringObject( l_f32TargetX, l_f32TargetY, l_oCurrentTargetColour, l_sMaterial, l_ui32StimulationPattern );
	}
	else
	{
		m_poApplication->getLogManager() << OpenViBE::Kernel::LogLevel_Fatal << "TrainerTarget object was not properly initialized\n";
		return NULL;
	}
}

//SP CTrainerFlickeringObject::CTrainerFlickeringObject( OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::uint8 ui8LitFrames, OpenViBE::uint8 ui8DarkFrames ) :
//SP	CSSVEPFlickeringObject( NULL, ui8LitFrames, ui8DarkFrames )
CGenericStimulatorFlickeringObject::CGenericStimulatorFlickeringObject( OpenViBE::float32 f32PosX, OpenViBE::float32 f32PosY, Ogre::ColourValue oColour, OpenViBE::CString sMaterial, OpenViBE::uint32 ui32StimulationPattern ) :
	CSSVEPFlickeringObject( NULL, ui32StimulationPattern )
{
	Ogre::SceneNode* l_poPointerNode;

	Ogre::MovableObject* l_poLitObject;
	Ogre::MovableObject* l_poDarkObject;

	m_poElementNode = m_poParentNode->createChildSceneNode();
	m_poObjectNode = m_poElementNode->createChildSceneNode();
	l_poPointerNode = m_poElementNode->createChildSceneNode();

	Ogre::Rectangle l_oRectangle = { f32PosX - m_f32TargetWidth / 2, f32PosY + m_f32TargetHeight / 2, f32PosX + m_f32TargetWidth / 2, f32PosY - m_f32TargetHeight / 2};

	if (strcmp(sMaterial.toASCIIString(), "default") == 0)
	{
		l_poLitObject = m_poPainter->paintRectangle( l_oRectangle, oColour );
		l_poDarkObject = m_poPainter->paintRectangle( l_oRectangle, m_oDarkColour );
	}
	else
	{
		l_poLitObject = m_poPainter->paintTexturedRectangle(l_oRectangle, std::string("GenericSurface/").append(sMaterial));
		l_poDarkObject = m_poPainter->paintTexturedRectangle(l_oRectangle, std::string("GenericSurface/").append(sMaterial).append("-dark"));
	}

	m_poObjectNode->attachObject( l_poLitObject );
	l_poLitObject->setVisible( true );

	m_poObjectNode->attachObject( l_poDarkObject );
	l_poDarkObject->setVisible( false );

	m_poPointer = m_poPainter->paintTriangle( 
				Ogre::Vector2( f32PosX - 0.05f, f32PosY + m_f32TargetHeight ),
				Ogre::Vector2( f32PosX, f32PosY + m_f32TargetHeight - 0.05f ),
				Ogre::Vector2( f32PosX + 0.05f, f32PosY + m_f32TargetHeight ),
				ColourValue(1, 1, 0));

	l_poPointerNode->attachObject( m_poPointer );
	m_poPointer->setVisible( false );


}

void CGenericStimulatorFlickeringObject::setTarget( bool bIsTarget )
{
	m_poPointer->setVisible( bIsTarget );
}



#endif