#ifndef __OpenViBEApplication_CImpactShip_H__
#define __OpenViBEApplication_CImpactShip_H__

#include <vector>

#include <Ogre.h>

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

namespace OpenViBESSVEP
{
	class CImpactApplication;
	class CImpactEnemyShip;
	class CSSVEPFlickeringObject;

	class CImpactShip
	{
		public:
			enum TargetState { TS_NORMAL, TS_INHIBITED, TS_OFF };

//SP			CImpactShip( CImpactApplication* poApplication, Ogre::SceneNode* poParentNode, std::vector<std::pair<OpenViBE::uint32, OpenViBE::uint32> >* pFrequencies);
			CImpactShip( CImpactApplication* poApplication, Ogre::SceneNode* poParentNode, std::vector<OpenViBE::uint64>* pFrequencies);
			void processFrame( OpenViBE::uint32 ui32CurrentFrame );

			void moveToPosition() { m_iPrepareAnimationFrame = 70; }
			void move( int iDisplacementVector );
			void shoot();
			bool evaluateHit( CImpactEnemyShip* poEnemy );
			void setAllTargetsVisibility( bool bVisible );
			Ogre::Vector2 getPosition();
			Ogre::Vector2 getNormalizedPosition();
			void setTargetLocked( bool bTargetLocked ) { m_bTargetLocked = bTargetLocked; }

			void setFeedbackLevels(int iF1, int iF2, int iF3);
			void activatePilotAssist( bool bPilotAssist );
			void activateCannonInhibitor( bool bCannonInhibitor );
			void activateTargetLockdown( bool bTargetLockdown );
			void activateFeedback( bool bFeedback );
			void activateFocusPoint( bool bFocusPoint );
			void returnToMiddleAndWaitForFoe( int ui32TargetPosition, Ogre::Real rPosition = 0.0 );

			TargetState getTargetState(int iTarget)
			{
				return m_vTargetStates[iTarget];
			}

		private:

			struct blaster_projectile {
				Ogre::ParticleSystem* pParticleSystem;
				Ogre::SceneNode* pNode;
				Ogre::SceneNode* pOutlineNode;
				int iTTL;
				Ogre::Real rRelX, rRelY;
			};

			static bool projectileExhausted(struct blaster_projectile*);

			CImpactApplication* m_poApplication;

			Ogre::SceneNode* m_poShipNode;
			Ogre::SceneNode* m_poTargetingNode;
			Ogre::SceneNode* m_poProjectilesNode;

			CSSVEPFlickeringObject* m_poShipCannon;
			CSSVEPFlickeringObject* m_poShipLeftWing;
			CSSVEPFlickeringObject* m_poShipRightWing;

			TargetState m_vTargetStates[3];

			int m_iPrepareAnimationFrame;
			bool m_bTargetLocked;
			bool m_bReturning;
			std::deque<OpenViBE::uint32> m_vNextEnemyPosition;
			bool m_bPilotAssistActive;
			bool m_bTargetLockdownActive;
			bool m_bFeedbackActive;
			bool m_bFocusPointActive;
			bool m_bCannonInhibited;

			Ogre::Real m_rAngularSpeed;
			Ogre::Radian m_rCurrentAngle;
			int m_iCurrentRotationCount;
			int m_iCurrentDisplacementCount;

			int m_iShootingCooldown;
			std::vector<struct blaster_projectile*> m_vProjectiles;
			int m_iShotCount;
			Ogre::Real m_rOrigin;
			Ogre::Real m_rOldPosition;



	};
}

#endif // __OpenViBEApplication_CImpactShip_H__
