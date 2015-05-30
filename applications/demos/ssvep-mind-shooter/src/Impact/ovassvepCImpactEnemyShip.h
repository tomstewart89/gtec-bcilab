#ifndef __OpenViBEApplication_CImpactEnemyShip_H__
#define __OpenViBEApplication_CImpactEnemyShip_H__

#include <Ogre.h>
#include <iostream>

#include "../ovassvepCBasicPainter.h"

#define SSVEP_SHOOTER_TARGET_SIZE 0.15f

namespace OpenViBESSVEP
{
	class CImpactApplication;

	class CImpactEnemyShip
	{
	public:
		static CImpactEnemyShip* createTarget( Ogre::Real rPosition );
		static void initialize( CImpactApplication* poApplication );
		~CImpactEnemyShip();

		void processFrame();
		void processHit(Ogre::Vector2 oPoint);

		Ogre::Vector2 getEnemyPosition();
		bool isDestroyed() { return m_bShipDestroyed; }
		bool isTouchable() {

			return m_ui32DestructionStatus < 10 && !m_bEnemyLeaving;
		}
		int getPointValue() { return m_iPointValue; }
		Ogre::Real getWidth() { return m_rShipWidth; }
		int m_iLeaveCountdown;
		bool m_bEnemyLeaving;

	private:

		struct explosion_animation {
			Ogre::ParticleSystem* pParticleSystem;
			Ogre::SceneNode* pNode;
			int iTTL;
		};

		static bool explostionExhausted(struct explosion_animation*);
		static void destroyAllAttachedMovableObjects( Ogre::SceneNode* pSceneNode );

		static CImpactApplication* m_poApplication;
		static Ogre::SceneNode* m_poParentNode;
		static CBasicPainter* m_poPainter;

		Ogre::SceneNode* m_poEnemyNode;
		Ogre::SceneNode* m_poHitBox;
		Ogre::SceneNode* m_vFragments[4];
		Ogre::Real m_rIncomingStatus;
		Ogre::Real m_rShipWidth;
		Ogre::uint32 m_ui32DestructionStatus;
		bool m_bShipDestroyed;
		int m_iPointValue;

		CImpactEnemyShip( Ogre::Real rPosition );

		static int m_iCurrentExplosionCount;
		std::vector<struct explosion_animation*> m_vExplosions;

	};
}


#endif // __OpenViBEApplication_CImpactEnemyShip_H__
