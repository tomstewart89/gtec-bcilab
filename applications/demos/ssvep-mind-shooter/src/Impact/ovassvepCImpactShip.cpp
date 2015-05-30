
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCImpactShip.h"

#include "ovassvepCImpactEnemyShip.h"
#include "ovassvepCImpactApplication.h"
#include "../ovassvepCSSVEPFlickeringObject.h"

#define SSVEP_SHIP_HULL_COLOUR Ogre::ColourValue(0.0f, 0.5f, 0.5f)
#define SIGN(x) ( (x) / abs(x) )
#define FSIGN(x) ( (x) / fabs(x) )

#define SCREEN_RATIO ( 16.0f / 9.0f )
#define IMPACTSHIP_SIZE 0.2f
#define IMPACTSHIP_SPEEDFACTOR 0.7f

using namespace OpenViBESSVEP;
using namespace Ogre;

//SP CImpactShip::CImpactShip(CImpactApplication* poApplication, Ogre::SceneNode* poParentNode, std::vector<std::pair<OpenViBE::uint32, OpenViBE::uint32> >* pFrequencies) :
CImpactShip::CImpactShip(CImpactApplication* poApplication, Ogre::SceneNode* poParentNode, std::vector<OpenViBE::uint64>* pFrequencies) :
	m_poApplication(poApplication),
	m_iPrepareAnimationFrame(0),
	m_bTargetLocked(true),
	m_bReturning(false),
	m_rCurrentAngle(0),
	m_iCurrentRotationCount(0),
	m_iCurrentDisplacementCount(0),
	m_iShootingCooldown(0),
	m_iShotCount(0),
	m_rOrigin(0.0),
	m_rOldPosition(0)
{
	std::cout << "Loading Ship\n";
	m_poApplication->getSceneLoader()->parseDotScene( "v5.scene", "SSVEPImpact", m_poApplication->getSceneManager());

	m_vTargetStates[0] = TS_NORMAL;
	m_vTargetStates[1] = TS_NORMAL;
	m_vTargetStates[2] = TS_NORMAL;

	SceneManager* l_poSceneManager = m_poApplication->getSceneManager();
	m_poProjectilesNode = poParentNode->createChildSceneNode();

	m_poShipNode = l_poSceneManager->getSceneNode("battleship");
	m_poShipNode->scale(0.6f, 0.6f, 0.6f);
	m_poShipNode->yaw(Radian(-Math::PI / 2.0f));
	m_poShipNode->translate(-25.0f, 0.0f, 0.0f, Node::TS_LOCAL);

	if(!pFrequencies || pFrequencies->size()<4) {
		std::cout << "Error: Expected the frequencies vector to contain indexes 0...3 but its size is " << (pFrequencies ? pFrequencies->size() : -1) << ". Crash is likely imminent.\n";
	}

	l_poSceneManager->getSceneNode("v5_ecran_avant2")->setVisible(false);
//SP	m_poShipCannon = new CSSVEPFlickeringObject( l_poSceneManager->getSceneNode("shipCannon"), (*pFrequencies)[1].first, (*pFrequencies)[1].second);
	m_poShipCannon = new CSSVEPFlickeringObject( l_poSceneManager->getSceneNode("shipCannon"), (*pFrequencies)[1]);

	l_poSceneManager->getSceneNode("v5_aile_gauche2")->setVisible(false);
//SP	m_poShipLeftWing = new CSSVEPFlickeringObject( l_poSceneManager->getSceneNode("shipLeftWing"), (*pFrequencies)[2].first, (*pFrequencies)[2].second);
	m_poShipLeftWing = new CSSVEPFlickeringObject( l_poSceneManager->getSceneNode("shipLeftWing"), (*pFrequencies)[2]);

	l_poSceneManager->getSceneNode("v5_aile_droite2")->setVisible(false);
//SP	m_poShipRightWing = new CSSVEPFlickeringObject( l_poSceneManager->getSceneNode("shipRightWing"), (*pFrequencies)[3].first, (*pFrequencies)[3].second);
	m_poShipRightWing = new CSSVEPFlickeringObject( l_poSceneManager->getSceneNode("shipRightWing"), (*pFrequencies)[3]);


	l_poSceneManager->getSceneNode("feedback")->setVisible(true);

	m_poTargetingNode = l_poSceneManager->getSceneNode("v5_viseur");
	/*
	m_poTargetingNode->attachObject(l_poPainter->paintTriangle(
									Vector2(+0.2, +0.2),
									Vector2(-0.2, +0.2),
									Vector2( 0.0, -0.2)
									));
									*/

}

void CImpactShip::activatePilotAssist( bool bPilotAssist )
{
	m_bPilotAssistActive = bPilotAssist;
	m_poApplication->logPrefix() << "Pilot Assist : " << (bPilotAssist ? "ON" : "OFF") << "\n";
}

void CImpactShip::activateCannonInhibitor(bool bCannonInhibitor)
{

	m_bCannonInhibited = bCannonInhibitor;

	m_poApplication->logPrefix() << "Cannon Inhibitor : " << (bCannonInhibitor ? "ON" : "OFF") << "\n";
}

void CImpactShip::activateTargetLockdown(bool bTargetLockdown)
{
	m_bTargetLockdownActive = bTargetLockdown;
	m_poApplication->logPrefix() << "Target Lockdown : " << (bTargetLockdown ? "ON" : "OFF") << "\n";
}

void CImpactShip::activateFeedback( bool bFeedback )
{
	m_bFeedbackActive = bFeedback;
	m_poApplication->getSceneManager()->getSceneNode("feedback")->setVisible(bFeedback);
	m_poApplication->logPrefix() << "Feedback : " << (bFeedback ? "ON" : "OFF") << "\n";
}

void CImpactShip::activateFocusPoint( bool bFocusPoint )
{
	m_bFocusPointActive = bFocusPoint;
	m_poApplication->getSceneManager()->getSceneNode("feedback")->setVisible(!m_bFocusPointActive && m_bFeedbackActive);

	if (bFocusPoint)
	{
		m_poApplication->getSceneManager()->getSceneNode("v5_a1a")->setVisible(false);
		m_poApplication->getSceneManager()->getSceneNode("v5_a2a")->setVisible(true);
		m_poApplication->getSceneManager()->getSceneNode("v5_a3a")->setVisible(false);
		m_poApplication->getSceneManager()->getSceneNode("v5_g1a")->setVisible(true);
		m_poApplication->getSceneManager()->getSceneNode("v5_g2a")->setVisible(false);
		m_poApplication->getSceneManager()->getSceneNode("v5_g3a")->setVisible(false);
		m_poApplication->getSceneManager()->getSceneNode("v5_d1a")->setVisible(true);
		m_poApplication->getSceneManager()->getSceneNode("v5_d2a")->setVisible(false);
		m_poApplication->getSceneManager()->getSceneNode("v5_d3a")->setVisible(false);
	}
	
	m_poApplication->logPrefix() << "FocusPoint : " << (bFocusPoint ? "ON" : "OFF") << "\n";
}


bool CImpactShip::projectileExhausted(struct blaster_projectile* bp)
{
	return bp->iTTL <= 0;
}

void CImpactShip::setAllTargetsVisibility(bool bVisible)
{
	m_poShipCannon->setVisible(bVisible);
	m_poShipLeftWing->setVisible(bVisible);
	m_poShipRightWing->setVisible(bVisible);
}

void CImpactShip::processFrame( OpenViBE::uint32 ui32CurrentFrame )
{
	if (m_iShootingCooldown > 0)
	{
		m_iShootingCooldown--;
	}
	if (m_poApplication->isActive())
	{

		// deactivate the wing targets when the ship is on the border
		// std::cout << this->getNormalizedPosition() << "\n";

		if (getNormalizedPosition().x >= -0.75 || !m_bTargetLockdownActive)
		{
			m_poShipLeftWing->processFrame();
			m_vTargetStates[1] = TS_NORMAL;
		}
		else
		{
			m_poShipLeftWing->setVisible(false);
			m_vTargetStates[1] = TS_OFF;
		}


		if (getNormalizedPosition().x <= 0.75 || !m_bTargetLockdownActive)
		{
			m_poShipRightWing->processFrame();
			m_vTargetStates[2] = TS_NORMAL;
		}
		else
		{
			m_poShipRightWing->setVisible(false);
			m_vTargetStates[2] = TS_OFF;
		}

		// deactivate the cannon when there is no target locked
		if (m_bTargetLocked || (!m_bTargetLockdownActive))
		{
			m_poShipCannon->processFrame();
			m_vTargetStates[0] = (m_bCannonInhibited ? TS_INHIBITED : TS_NORMAL);
		}
		else
		{
			m_poShipCannon->setVisible(false);
			m_vTargetStates[0] = TS_OFF;
		}
	}

	m_poTargetingNode->pitch(Degree(2), Node::TS_PARENT);

	if (m_bTargetLocked)
	{
		m_poTargetingNode->setVisible(true);
	}
	else
	{
		m_poTargetingNode->setVisible(false);
	}

	/*
	if (m_iPrepareAnimationFrame > 0)
	{
		m_poShipNode->translate(0.0f, -0.01f, 0.0f);
		m_iPrepareAnimationFrame--;
	}
	*/
	if (m_bReturning)
	{
		if ( getNormalizedPosition().x < m_rOrigin - 5e-2 || getNormalizedPosition().x > m_rOrigin + 5e-2 )
		{
			m_iCurrentDisplacementCount = (getNormalizedPosition().x < m_rOrigin) ? 1 : -1;
		}
		else
		{
			m_poApplication->logPrefix() << "Ship Returned to Initial Position\n";
			m_bReturning = false;
			m_poApplication->startFlickering();
			while (m_vNextEnemyPosition.size() > 0)
			{
				uint32 l_ui32NextEnemyPosition = m_vNextEnemyPosition.front();
				m_vNextEnemyPosition.pop_front();
				m_poApplication->insertEnemy(l_ui32NextEnemyPosition);
			}
		}
	}

	if (m_iCurrentDisplacementCount != 0)
	{
		// m_poApplication->logPrefix() << "Displacement Vector : value = " << m_iCurrentDisplacementCount << "\n";

		if (m_bReturning || !m_bTargetLocked || !m_bPilotAssistActive)
		{
			m_poApplication->logPrefix() << "Ship Moving : direction " << (m_iCurrentDisplacementCount > 0 ? "left" : "right") << ", full speed\n";
			m_poShipNode->translate( 0.5f * -SIGN(m_iCurrentDisplacementCount) * IMPACTSHIP_SPEEDFACTOR, 0.0f, 0.0f );
		}
		else
		{
			Ogre::Real l_rSlowdown = 0.0;

			if (m_poApplication->getCurrentEnemy() != NULL && m_poApplication->getCurrentEnemy()->getPointValue() > 0)
			{
				Ogre::Real l_rEnemyPosition = m_poApplication->getCurrentEnemy()->getEnemyPosition().x;
				Ogre::Real l_rShipPosition = getNormalizedPosition().x;


				if (fabs( l_rEnemyPosition - l_rShipPosition) < m_poApplication->getCurrentEnemy()->getWidth() / 2)
				{
					Ogre::Real l_rDistance = fabs(l_rShipPosition - l_rEnemyPosition) / (m_poApplication->getCurrentEnemy()->getWidth() / 2);
					//std::cout << "Distance : " << l_rDistance << "\n";

					if (SIGN(m_iCurrentDisplacementCount) == FSIGN(l_rShipPosition - l_rEnemyPosition))
					{
						l_rSlowdown = 1.0f - l_rDistance * l_rDistance * l_rDistance;
					}

					if (SIGN(m_iCurrentDisplacementCount) > 0)
					{
						m_vTargetStates[2] = TS_INHIBITED;
					}
					else
					{
						m_vTargetStates[1] = TS_INHIBITED;
					}
				}
			}
			m_poApplication->logPrefix() << "Ship Moving : direction " << (m_iCurrentDisplacementCount > 0 ? "left" : "right") << ", slowdown = " << l_rSlowdown << "\n";
			m_poShipNode->translate( (0.5f - l_rSlowdown * 0.3f ) * -SIGN(m_iCurrentDisplacementCount) * IMPACTSHIP_SPEEDFACTOR, 0.0f, 0.0f );
		}

		//		m_poTargetingNode->translate( 0.007f * SIGN(m_iCurrentDisplacementCount), 0.0f, 0.0f );
		m_iCurrentDisplacementCount -= SIGN(m_iCurrentDisplacementCount);
	}


	for (std::vector<struct blaster_projectile*>::iterator it = m_vProjectiles.begin(); it != m_vProjectiles.end(); ++it)
	{
		(*it)->iTTL--;

		(*it)->pNode->translate(0.0, -2.0, 0.0, Node::TS_LOCAL);
		(*it)->rRelY -= 1.0f / 65.0f;


		if (projectileExhausted(*it))
		{
			m_poApplication->getSceneManager()->destroyParticleSystem( (*it)->pParticleSystem );
			(*it)->pNode->getCreator()->destroySceneNode( (*it)->pNode );
			delete *it;
		}
	}
	m_vProjectiles.erase(std::remove_if(m_vProjectiles.begin(), m_vProjectiles.end(), projectileExhausted), m_vProjectiles.end());


	if (m_rOldPosition != m_poShipNode->getPosition().x)
	{
		m_rOldPosition = m_poShipNode->getPosition().x;
		m_poApplication->logPrefix() << "New Ship Position : " << m_rOldPosition << " / Normalized X : " << getNormalizedPosition().x << "\n";
	}
}

void CImpactShip::move( int iDisplacementVector )
{
	if (iDisplacementVector < 0 && getNormalizedPosition().x <= -0.75)
	{
		return;
	}
	if (iDisplacementVector > 0 && getNormalizedPosition().x >= 0.75)
	{
		return;
	}

	//m_poApplication->logPrefix() << "Move : diff vector = " << iDisplacementVector << ", current vector = " << m_iCurrentDisplacementCount;

	if (abs(m_iCurrentDisplacementCount) < 6)
	{
		m_iCurrentDisplacementCount += iDisplacementVector;
	}

	//m_poApplication->getLogManager() << ", resulting vector = " << m_iCurrentDisplacementCount << "\n";
}

void CImpactShip::shoot()
{

	m_poApplication->logPrefix() << "Shoot : ";

    if (m_bTargetLockdownActive && !m_bTargetLocked)
	{
		m_poApplication->logPrefix() << "CANCELED : pilot assist blocked the shot\n";
		return;
	}


	if (m_iShootingCooldown > 0)
	{
		m_poApplication->logPrefix() << "CANCELED : shooting cooldown blocked the shot\n";
		return;
	}

	m_iShootingCooldown = (m_vTargetStates[0] == TS_INHIBITED) ? 24 : 12;

	struct blaster_projectile* l_poProjectile = new struct blaster_projectile;
	char l_sProjectileName[255];
	sprintf(l_sProjectileName, "Projectile_%d", ++m_iShotCount);


	l_poProjectile->pParticleSystem = m_poApplication->getSceneManager()->createParticleSystem(l_sProjectileName, "Particle/PurpleFountain");


	l_poProjectile->iTTL = 70;
	l_poProjectile->pNode = m_poProjectilesNode->createChildSceneNode();
	l_poProjectile->pNode->pitch(Radian(-Math::PI / 2.0f));
	l_poProjectile->pNode->translate(this->getPosition().x, 20.0, 0.0, Node::TS_LOCAL);

	l_poProjectile->pNode->attachObject(l_poProjectile->pParticleSystem);

	l_poProjectile->rRelX = -m_poShipNode->getPosition().x / 70.0f;
	l_poProjectile->rRelY = 0.6f;


	m_poApplication->getLogManager() << "OK : creating particle system\n";
	m_vProjectiles.push_back(l_poProjectile);
}

void CImpactShip::setFeedbackLevels(int iF1, int iF2, int iF3)
{
	if (m_bFocusPointActive)
	{
		return;
	}
	
	SceneManager* l_poSceneManager = m_poApplication->getSceneManager();

	if (m_vTargetStates[1] == TS_OFF)
	{
		iF2 = 0;
	}

	if (m_vTargetStates[2] == TS_OFF)
	{
		iF3 = 0;
	}

	l_poSceneManager->getSceneNode("v5_a1b")->setVisible(false);
	l_poSceneManager->getSceneNode("v5_a2b")->setVisible(false);
	l_poSceneManager->getSceneNode("v5_a3b")->setVisible(false);
	l_poSceneManager->getSceneNode("v5_g1b")->setVisible(false);
	l_poSceneManager->getSceneNode("v5_g2b")->setVisible(false);
	l_poSceneManager->getSceneNode("v5_g3b")->setVisible(false);
	l_poSceneManager->getSceneNode("v5_d1b")->setVisible(false);
	l_poSceneManager->getSceneNode("v5_d2b")->setVisible(false);
	l_poSceneManager->getSceneNode("v5_d3b")->setVisible(false);

	if (m_bFeedbackActive)
	{
		int l_iNbActives = 0;

		if (iF1 > 0) { l_poSceneManager->getSceneNode("v5_a1b")->setVisible(true); }
		if (iF1 > 1) { l_poSceneManager->getSceneNode("v5_a2b")->setVisible(true); }
		if (iF1 > 2) { l_poSceneManager->getSceneNode("v5_a3b")->setVisible(true); l_iNbActives++; }
		if (iF2 > 0) { l_poSceneManager->getSceneNode("v5_g1b")->setVisible(true); }
		if (iF2 > 1) { l_poSceneManager->getSceneNode("v5_g2b")->setVisible(true); }
		if (iF2 > 2) { l_poSceneManager->getSceneNode("v5_g3b")->setVisible(true); l_iNbActives++; }
		if (iF3 > 0) { l_poSceneManager->getSceneNode("v5_d1b")->setVisible(true); }
		if (iF3 > 1) { l_poSceneManager->getSceneNode("v5_d2b")->setVisible(true); }
		if (iF3 > 2) { l_poSceneManager->getSceneNode("v5_d3b")->setVisible(true); l_iNbActives++; }

		if (l_iNbActives > 1)
		{
			l_poSceneManager->getSceneNode("v5_a3b")->setVisible(false);
			l_poSceneManager->getSceneNode("v5_g3b")->setVisible(false);
			l_poSceneManager->getSceneNode("v5_d3b")->setVisible(false);
		}

		if (m_bTargetLockdownActive && !m_bTargetLocked)
		{
			l_poSceneManager->getSceneNode("v5_a1b")->setVisible(false);
			l_poSceneManager->getSceneNode("v5_a2b")->setVisible(false);
			l_poSceneManager->getSceneNode("v5_a3b")->setVisible(false);
		}
	}
}

Ogre::Vector2 CImpactShip::getPosition()
{
	return Ogre::Vector2(m_poShipNode->getPosition().x, m_poShipNode->getPosition().y);
}

Ogre::Vector2 CImpactShip::getNormalizedPosition()
{
	return Ogre::Vector2(-m_poShipNode->getPosition().x / 70.0f, m_poShipNode->getPosition().y);
}


void CImpactShip::returnToMiddleAndWaitForFoe(int ui32TargetPosition, Real rPosition)
{
	m_poApplication->logPrefix() << "Ship Returning to Middle Position\n";
	m_vNextEnemyPosition.push_back(ui32TargetPosition);
	m_poApplication->stopFlickering();
	m_rOrigin = rPosition;
	m_bReturning = true;
}

bool CImpactShip::evaluateHit(CImpactEnemyShip* poEnemy)
{
	Ogre::Vector2 l_oPoint = poEnemy->getEnemyPosition();
//	std::cout << "enemy:" << poEnemy->getEnemyPosition() << "\n";
	for (std::vector<struct blaster_projectile*>::iterator it = m_vProjectiles.begin(); it != m_vProjectiles.end(); ++it)
	{
//		std::cout << "proj: " << (*it)->rRelX << " " << (*it)->rRelY << "\n";

		if (Ogre::Math::pointInTri2D(
				Ogre::Vector2((*it)->rRelX, (*it)->rRelY),
				Ogre::Vector2(l_oPoint.x + poEnemy->getWidth() / 2.0f, l_oPoint.y + poEnemy->getWidth() / 2.0f),
				Ogre::Vector2(l_oPoint.x - poEnemy->getWidth() / 2.0f, l_oPoint.y + poEnemy->getWidth() / 2.0f),
				Ogre::Vector2(l_oPoint.x , l_oPoint.y - 0.1f)
				))
		{
			m_poApplication->logPrefix() << "Enemy Hit!\n";
			(*it)->iTTL = 1;
			poEnemy->processHit(Ogre::Vector2((*it)->rRelX, (*it)->rRelY));
			return true;
		}
	}

	return false;
}

#endif