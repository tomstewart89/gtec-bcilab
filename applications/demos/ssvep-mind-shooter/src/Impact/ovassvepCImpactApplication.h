#ifndef __OpenViBEApplication_CImpactApplication_H__
#define __OpenViBEApplication_CImpactApplication_H__

#include <iostream>

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "dotscene/DotSceneLoader.h"

#include "../ovassvepCApplication.h"
#include "ovassvepCImpactEnemyShip.h"
#include "ovassvepCImpactShip.h"


namespace OpenViBESSVEP
{
	class CAdvancedControl;

	class CImpactApplication : public CApplication
	{
		public:
			enum GameState { BOOTING, IDLE_TRAINING, TRAINING, IDLE_STARTED, STARTED };

			struct stimulator_state
			{
				float ship_position;
				double feedback_level[3];
				int ship_direction;
				bool shooting;
			};

			struct game_state
			{
				int movements_right;
				int movements_left;
				int shoots_fired;
			};

			CImpactApplication(OpenViBE::CString sScenarioDir, OpenViBE::CString sApplicationSubtype);
			~CImpactApplication();
	
			bool setup(OpenViBE::Kernel::IKernelContext* poKernelContext);

			CImpactShip* getShip()
			{
				return m_poShip;
			}

			bool isActive() { return m_bActive; }
			
			void startExperiment();
			void stopExperiment();
			void startFlickering();
			void stopFlickering();

			void debugAction1();
			void debugAction2();
			void debugAction3();
			void debugAction4();

			int getNextTargetType();
			void addTarget(OpenViBE::uint32 ui32TargetPosition);

			void insertEnemy( int ui32TargetPosition );
			CImpactEnemyShip* m_poCurrentEnemy;
			CImpactEnemyShip* getCurrentEnemy();
			void calculateFeedback(int iChannelCount, double* channel);

			bool m_bTargetRequest;
			Ogre::Real m_rNextOrigin;
			OpenViBE::CString getSubtype() { return m_sApplicationSubtype; }
			GameState getState() { return m_eGameState; }
			DotSceneLoader* getSceneLoader() { return m_poSceneLoader; }

		private:
			void setupScene();

			DotSceneLoader* m_poSceneLoader;
			static bool enemyDestroyed(CImpactEnemyShip*);

			OpenViBE::CString m_sApplicationSubtype;
			GameState m_eGameState;

			std::queue<int> m_oEnemyOrder;
			OpenViBE::float64 m_pMaxFeedbackLevel[3];

			bool m_bActive;

			time_t m_ttStartTime;

			CAdvancedControl* m_poAdvancedControl;
			
			void processFrame(OpenViBE::uint32 ui32CurrentFrame);

			CEGUI::Window* m_poStatusWindow;
			CEGUI::Window* m_poInstructionWindow;
			OpenViBE::uint32 m_ui32DialogHideDelay;

			CImpactShip* m_poShip;
			std::vector<CImpactEnemyShip*> m_oTargets;
			int m_iScore;
	};
}

#endif // __OpenViBEApplication_CImpactApplication_H__
