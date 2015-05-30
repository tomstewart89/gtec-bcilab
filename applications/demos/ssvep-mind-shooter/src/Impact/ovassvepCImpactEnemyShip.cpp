
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCImpactEnemyShip.h"
#include "ovassvepCImpactApplication.h"

#define IMPACTSHIP_SIZE 0.2f
#define SCREEN_RATIO ( 16.0f / 9.0f )

using namespace OpenViBESSVEP;
using namespace Ogre;

CImpactApplication* CImpactEnemyShip::m_poApplication = NULL;
SceneNode* CImpactEnemyShip::m_poParentNode = NULL;
CBasicPainter* CImpactEnemyShip::m_poPainter = NULL;
int CImpactEnemyShip::m_iCurrentExplosionCount = 0;

void CImpactEnemyShip::initialize( CImpactApplication* poApplication )
{
	m_poApplication = poApplication;
	m_poPainter = m_poApplication->getPainter();
	m_poParentNode = m_poApplication->getSceneNode();
}

CImpactEnemyShip* CImpactEnemyShip::createTarget( Ogre::Real rPosition )
{
	if ( m_poApplication == NULL )
	{
		std::cerr << "The CImpactEnemyShip was not initialized" << std::endl;
		return NULL;
	}

	return new CImpactEnemyShip( rPosition );
}

CImpactEnemyShip::CImpactEnemyShip( Ogre::Real rPosition )
	: m_rShipWidth(0.1f),
	  m_ui32DestructionStatus(0),
	  m_bShipDestroyed(false),
	  m_iPointValue(100)
{

	m_bEnemyLeaving = false;
	int l_iType = m_poApplication->getNextTargetType();
	m_poApplication->logPrefix() << "Creating enemy " << l_iType << "\n";

	if (l_iType > 0 && l_iType <= 6)
	{

		switch (l_iType)
		{
		case 1:
			m_poApplication->getSceneLoader()->parseDotScene("v2.scene", "SSVEPImpact", m_poApplication->getSceneManager());

			m_poEnemyNode = m_poApplication->getSceneManager()->getSceneNode("v2_vaisseau");
			m_poEnemyNode->scale(1.0f, 1.0f, 1.0f);
			m_rShipWidth = 0.5f;
			m_iLeaveCountdown = (int)(2.0f * 3600);
			m_iPointValue = 100;
			break;
		case 2:
			m_poApplication->getSceneLoader()->parseDotScene("v1.scene", "SSVEPImpact", m_poApplication->getSceneManager());

			m_poEnemyNode = m_poApplication->getSceneManager()->getSceneNode("v1_vaisseau");
			m_poEnemyNode->scale(1.9f, 1.9f, 1.9f);
			m_rShipWidth = 0.45f;
			m_iLeaveCountdown = (int)(1.5f * 3600);
			m_iPointValue = 200;
			break;
		case 3:
			m_poApplication->getSceneLoader()->parseDotScene("v3.scene", "SSVEPImpact", m_poApplication->getSceneManager());

			m_poEnemyNode = m_poApplication->getSceneManager()->getSceneNode("v3_vaisseau");
			m_poEnemyNode->scale(2.2f, 2.0f, 2.0f);
			m_rShipWidth = 0.4f;
			m_iLeaveCountdown = (int)(1.0f * 3600);
			m_iPointValue = 400;
			break;
		case 4:
			m_poApplication->getSceneLoader()->parseDotScene("ship-cheap.scene", "SSVEPImpact", m_poApplication->getSceneManager());
			m_poEnemyNode = m_poApplication->getSceneManager()->getSceneNode("ship-cheap_node");
			m_iPointValue = 500;
		break;
		case 5:
			m_poApplication->getSceneLoader()->parseDotScene("ship-costly.scene", "SSVEPImpact", m_poApplication->getSceneManager());
			m_poEnemyNode = m_poApplication->getSceneManager()->getSceneNode("ship-costly_node");
			m_iPointValue = 1500;
			break;
		case 6:
			m_poApplication->getSceneLoader()->parseDotScene("ship-friend.scene", "SSVEPImpact", m_poApplication->getSceneManager());
			m_poEnemyNode = m_poApplication->getSceneManager()->getSceneNode("ship-friend_node");
			m_iPointValue = -500;
			break;
		default:
			break;
		}

		if (l_iType >= 4 && l_iType <= 6)
		{
			m_poEnemyNode->scale(1.2f, 1.2f, 1.2f);
			m_rShipWidth = 0.3f;
			m_iLeaveCountdown = (int)(1.0f * 3600);
		}


		m_poEnemyNode->yaw(Radian(Math::PI));

		m_poHitBox = m_poApplication->getSceneNode()->createChildSceneNode();


		/*
		m_poHitBox->attachObject(m_poPainter->paintTriangle(
									 Vector2(+ m_rShipWidth / 2, +0.9),
									 Vector2(- m_rShipWidth / 2, +0.9),
									 Vector2( 0.0, +0.5)
									 ));
									 */


		m_poHitBox->translate(rPosition / 70.0f, 1.2f, 0.0f, Node::TS_LOCAL);
		m_poEnemyNode->translate( rPosition, 0.0f, -60.0f, Node::TS_LOCAL);
		m_rIncomingStatus = 30.0f;
	}
	else
	{
		m_poApplication->getLogManager() << OpenViBE::Kernel::LogLevel_Fatal << "Unknown enemy type " << l_iType << " ! The application will now crash!\n";
	}


}

void CImpactEnemyShip::destroyAllAttachedMovableObjects( Ogre::SceneNode* pSceneNode )
{
	if ( !pSceneNode )
		return;

	// Destroy all the attached objects
	Ogre::SceneNode::ObjectIterator itObject = pSceneNode->getAttachedObjectIterator();

	while ( itObject.hasMoreElements() )
	{
		Ogre::MovableObject* pObject = static_cast<Ogre::MovableObject*>(itObject.getNext());
		pSceneNode->getCreator()->destroyMovableObject( pObject );
	}

	Ogre::SceneNode::ChildNodeIterator itChild = pSceneNode->getChildIterator();

	while ( itChild.hasMoreElements() )
	{
		Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
		destroyAllAttachedMovableObjects( pChildNode );
	}
}

CImpactEnemyShip::~CImpactEnemyShip()
{
	m_poApplication->logPrefix() << "Enemy Destroyed\n";

	destroyAllAttachedMovableObjects(m_poEnemyNode);
	m_poEnemyNode->removeAndDestroyAllChildren();
	m_poEnemyNode->getCreator()->destroySceneNode( m_poEnemyNode );

	destroyAllAttachedMovableObjects(m_poHitBox);
	m_poHitBox->removeAndDestroyAllChildren();
	m_poHitBox->getCreator()->destroySceneNode( m_poHitBox );
}

Ogre::Vector2 CImpactEnemyShip::getEnemyPosition()
{
	return Ogre::Vector2(m_poHitBox->getPosition().x, m_poHitBox->getPosition().y);
}

bool CImpactEnemyShip::explostionExhausted(struct explosion_animation* ea)
{
	return ea->iTTL < -25;
}

void CImpactEnemyShip::processFrame()
{


	// oscillation animation
	static int l_iSinOscillator = 90;

	if (m_iLeaveCountdown > 0 && !m_bEnemyLeaving)
	{
		m_iLeaveCountdown--;
	}
	if (m_iLeaveCountdown == 0 && !m_bEnemyLeaving)
	{
		m_poApplication-> logPrefix() << "Enemy Leaving\n";

		m_bEnemyLeaving = true;
		m_iPointValue = 0;
	}

	if (!m_bEnemyLeaving)
	{
		l_iSinOscillator++;
		l_iSinOscillator = l_iSinOscillator % 360;
		//	m_poEnemyNode->roll(Radian(Math::Sin(Degree(sin_oscillator)) / 80));
		m_poEnemyNode->translate(Math::Sin(Degree((Ogre::Real)l_iSinOscillator)) / 20.0f * 0, Math::Sin(Degree((Ogre::Real)(l_iSinOscillator*2 + 90))) / 20, 0.0f,  Node::TS_LOCAL);
		m_poHitBox->translate(Math::Sin(Degree((Ogre::Real)l_iSinOscillator)) / 20.0f / 50.0f * 0, 0.0f, 0.0f,  Node::TS_LOCAL);


		// incoming enemy animation
		if (m_rIncomingStatus > 0)
		{
			m_poHitBox->translate(0.0f, -0.04f, 0.0f, Node::TS_LOCAL);
			m_poEnemyNode->translate(0.0f, 0.0f, 1.0f, Node::TS_LOCAL);
			m_rIncomingStatus -= 1.0f;
		}

		// exploding ship animation
		if (m_ui32DestructionStatus > 3)
		{
			m_ui32DestructionStatus++;

			m_poEnemyNode->translate(0.1f, 2, 1, Node::TS_PARENT);
			m_poEnemyNode->rotate(Vector3(0.3f, 0.25f, 1.0f), Radian(3.14f/20.0f), Node::TS_LOCAL);
			m_poEnemyNode->scale(0.95f, 0.95f, 0.95f);
		}
	}
	else
	{
		m_poEnemyNode->translate(0.0f, -1.0f, m_iLeaveCountdown * 0.5f, Node::TS_LOCAL);
		m_iLeaveCountdown++;

		if (m_poEnemyNode->getPosition().z < -100)
		{
			m_bShipDestroyed = true;
			m_poApplication->logPrefix() << "Enemy Left : marked as destroyed\n";
		}
	}

	if (m_ui32DestructionStatus > 50)
	{
		m_bShipDestroyed = true;
		if (this->getPointValue() > 0)
		{
			m_poApplication->logPrefix() << "Enemy Shot Down : marked as destroyed\n";
		}
		else
		{
			m_poApplication->logPrefix() << "Friendly Target Shot Down : marked as destroyed\n";
		}
	}



	// handle the explosions
	for (std::vector<struct explosion_animation*>::iterator it = m_vExplosions.begin(); it != m_vExplosions.end(); ++it)
	{
		(*it)->iTTL--;

		if ((*it)->iTTL < 4 && (*it)->iTTL >= 0)
		{
			Ogre::ParticleEmitter* l_poEmitter = (*it)->pParticleSystem->getEmitter(0);
			l_poEmitter->setEmissionRate(l_poEmitter->getEmissionRate() - 50);
		}

		if (explostionExhausted(*it))
		{
			m_poPainter->getSceneManager()->destroyParticleSystem( (*it)->pParticleSystem );
			(*it)->pNode->getCreator()->destroySceneNode( (*it)->pNode );
			delete *it;
		}

	}

	m_vExplosions.erase(std::remove_if(m_vExplosions.begin(), m_vExplosions.end(), explostionExhausted), m_vExplosions.end());
}

void CImpactEnemyShip::processHit(Ogre::Vector2 oPoint)
{
	// set status of the ship as exploding
	if (m_ui32DestructionStatus <= 3)
	{
		m_ui32DestructionStatus++;
		m_poApplication->logPrefix() << "Enemy Lost a Hitpoint : current hits = " << m_ui32DestructionStatus << "\n";
	}

	// create the explosion upon hit
	struct explosion_animation* l_poExplosion = new struct explosion_animation;
	char l_sProjectileName[255];
	sprintf(l_sProjectileName, "Explosion_%d", ++m_iCurrentExplosionCount);

	l_poExplosion->pParticleSystem = m_poPainter->getSceneManager()->createParticleSystem(l_sProjectileName, "Particle/Explosion");


	l_poExplosion->iTTL = 25;
	l_poExplosion->pNode = m_poParentNode->createChildSceneNode();

	l_poExplosion->pNode->attachObject(l_poExplosion->pParticleSystem);
	l_poExplosion->pNode->translate(oPoint.x * -65.0f, 0.0f, 30.0f, Ogre::Node::TS_WORLD);

	m_vExplosions.push_back(l_poExplosion);
}

#endif