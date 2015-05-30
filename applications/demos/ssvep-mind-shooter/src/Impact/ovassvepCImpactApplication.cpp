
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCImpactApplication.h"

#include "openvibe/ov_all.h"

#include "../ovassvepCCommandStartStop.h"
#include "../ovassvepCCommandStimulatorControl.h"
#include "../ovassvepCCommandCamera.h"
#include "ovassvepCCommandImpactTargetControl.h"
#include "ovassvepCCommandImpactCustomTargetControl.h"
#include "ovassvepCCommandImpactShipControl.h"
#include "ovassvepCCommandImpactShipControlOIS.h"
#include "ovassvepCCommandFeedbackHandler.h"

#include "../log/ovkCLogListenerFileBuffered.h"

#include "ovassvepCAdvancedControl.h"

#include <ctime>

using namespace Ogre;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBESSVEP;

#define DIALOG_CYCLES_TO_REMOVE 180
//#define NO_KEYBOARD
//#define NO_VRPN

CImpactApplication::CImpactApplication(CString sScenarioDir, CString sApplicationSubtype)
	: CApplication(sScenarioDir),
	  m_poCurrentEnemy( NULL ),
	  m_bTargetRequest( false ),
	  m_rNextOrigin(0.0),
	  m_poSceneLoader( NULL ),
	  m_sApplicationSubtype( sApplicationSubtype ),
	  m_eGameState( BOOTING ),
	  m_bActive(false),
	  m_poAdvancedControl( NULL ),
	  m_poStatusWindow( NULL ),
	  m_poInstructionWindow( NULL ),
	  m_ui32DialogHideDelay(0),
	  m_poShip( NULL ),
	  m_iScore(0)
{
}

CImpactApplication::~CImpactApplication()
{
	(*m_poLogManager) << LogLevel_Debug << "- m_poShip\n";
	delete m_poShip;
}

bool CImpactApplication::setup(OpenViBE::Kernel::IKernelContext* poKernelContext)
{
	if(!CApplication::setup(poKernelContext)) {
		return false;
	}

	IConfigurationManager* l_poConfigurationManager = &(m_poKernelContext->getConfigurationManager());
	l_poConfigurationManager->addConfigurationFromFile(m_sScenarioDir + "/appconf/impact-configuration.conf");

	poKernelContext->getLogManager().activate(LogLevel_First, LogLevel_Last, true);

	ILogListener* l_poLogListenerFileBuffered = new CLogListenerFileBuffered(*poKernelContext, "ssvep-stimulator",
															 l_poConfigurationManager->expand("${SSVEP_UserDataFolder}/log-[$core{date}-$core{time}].log"));
	poKernelContext->getLogManager().addListener(l_poLogListenerFileBuffered);


	m_poSceneLoader = new DotSceneLoader();

	std::stringstream l_sEnemyOrder;
	l_sEnemyOrder << l_poConfigurationManager->expand("${SSVEP_EnemyOrder}");
	std::cout << "enemy order " << l_poConfigurationManager->expand("${SSVEP_EnemyOrder}");

	while (l_sEnemyOrder.peek() != EOF)
	{
		int l_iNextType;
		l_sEnemyOrder >> l_iNextType;
		m_oEnemyOrder.push(l_iNextType);
	}

	(*m_poLogManager) << LogLevel_Debug << "Adding Impact game resources\n";


	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(CString(l_poConfigurationManager->expand("${Path_Data}/applications/${SSVEP_MindShooterFolderName}/resources/impact")).toASCIIString(), "FileSystem", "SSVEPImpact", true);
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(CString(l_poConfigurationManager->expand("${Path_Data}/applications/${SSVEP_MindShooterFolderName}/resources/impact/battleship")).toASCIIString(), "FileSystem", "SSVEPImpact");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(CString(l_poConfigurationManager->expand("${Path_Data}/applications/${SSVEP_MindShooterFolderName}/resources/impact/battleship-destroyed")).toASCIIString(), "FileSystem", "SSVEPImpact");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(CString(l_poConfigurationManager->expand("${Path_Data}/applications/${SSVEP_MindShooterFolderName}/resources/impact/enemy1")).toASCIIString(), "FileSystem", "SSVEPImpact");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(CString(l_poConfigurationManager->expand("${Path_Data}/applications/${SSVEP_MindShooterFolderName}/resources/impact/enemy2")).toASCIIString(), "FileSystem", "SSVEPImpact");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(CString(l_poConfigurationManager->expand("${Path_Data}/applications/${SSVEP_MindShooterFolderName}/resources/impact/enemy3")).toASCIIString(), "FileSystem", "SSVEPImpact");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(CString(l_poConfigurationManager->expand("${Path_Data}/applications/${SSVEP_MindShooterFolderName}/resources/impact/ships")).toASCIIString(), "FileSystem", "SSVEPImpact");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(CString(l_poConfigurationManager->expand("${Path_Data}/applications/${SSVEP_MindShooterFolderName}/resources/impact/starsky")).toASCIIString(), "FileSystem", "SSVEPImpact");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("SSVEPImpact");

	setupScene();

	m_pMaxFeedbackLevel[0] = 0.5;
	m_pMaxFeedbackLevel[1] = 0.5;
	m_pMaxFeedbackLevel[2] = 0.5;

	// Create the StarShip object
	(*m_poLogManager) << LogLevel_Debug << "+ m_poShip = new CStarShip(...)\n";
	m_poShip = new CImpactShip( this, m_poSceneNode, &m_oFrequencies );
	m_poShip->activatePilotAssist(l_poConfigurationManager->expandAsBoolean("${SSVEP_PilotAssist}"));
	m_poShip->activateTargetLockdown(l_poConfigurationManager->expandAsBoolean("${SSVEP_TargetLockdown}")  && (m_sApplicationSubtype == CString("shooter")));
    m_poShip->setFeedbackLevels(0,0,0);
	m_poShip->activateFeedback(l_poConfigurationManager->expandAsBoolean("${SSVEP_Feedback}") && (m_sApplicationSubtype == CString("shooter")));
	m_poShip->activateFocusPoint(l_poConfigurationManager->expandAsBoolean("${SSVEP_FocusPoint}"));

	// Initialize the Target class

	CImpactEnemyShip::initialize( this );

	// create CEGUI windows
	m_poInstructionWindow = static_cast<CEGUI::Window*>(m_poGUIWindowManager->createWindow("TaharezLook/StaticText", "InstructionsWindow" ));
	m_poInstructionWindow->setFont("BlueHighway-impact");
	m_poStatusWindow = static_cast<CEGUI::Window*>(m_poGUIWindowManager->createWindow("TaharezLook/StaticText", "StatusWindow" ));
	m_poStatusWindow->setFont("BlueHighway-impact");
	m_poStatusWindow->hide();

	m_poSheet->addChildWindow(m_poStatusWindow);
	m_poSheet->addChildWindow(m_poInstructionWindow);

	// Create commands



#ifndef NO_KEYBOARD
	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CCommandCamera(...)\n";
	this->addCommand(new CCommandCamera( this ));

	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CCommandShipControlOIS(...))\n";
	this->addCommand(new CCommandImpactShipControlOIS( this ));
#endif

#ifndef NO_VRPN

	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CCommandStartStop(...)\n";
	this->addCommand(new CCommandStartStop( this ));

	(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CCommandStimulatorControl(...))\n";
	this->addCommand(new CCommandStimulatorControl( this ));

	if (m_sApplicationSubtype == CString("trainer"))
	{
		(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CCommandTargetControl(...)\n";
		this->addCommand(new CCommandImpactTargetControl( this ));
	}
	else
	{
		if (l_poConfigurationManager->expandAsBoolean("${SSVEP_OneByOne}"))
		{
			(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CCommandTargetControl(...)\n";
			this->addCommand(new CCommandImpactTargetControl( this ));
		}
		else
		{
			(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CCommandImpactCustomTargetControl(...)\n";
			this->addCommand(new CCommandImpactCustomTargetControl( this ));
		}

		(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CCommandFeedbackHandler(...)\n";
		this->addCommand(new CCommandFeedbackHandler( this ));

		if (l_poConfigurationManager->expandAsBoolean("${SSVEP_AdvancedControl}"))
		{
			m_poAdvancedControl = new CAdvancedControl( this );
		}
		else
		{
			(*m_poLogManager) << LogLevel_Debug << "+ addCommand(new CCommandImpactShipControl(...))\n";
			this->addCommand(new CCommandImpactShipControl( this ));
		}
	}
#endif

	if (m_sApplicationSubtype == CString("shooter"))
	{
		m_eGameState = IDLE_STARTED;
		m_poShip->moveToPosition();

		// draw the initial text
		m_poInstructionWindow->show();
		m_poInstructionWindow->setSize( CEGUI::UVector2( CEGUI::UDim( 0.50f, 0 ), CEGUI::UDim( 0.10f, 0 ) ) );
		m_poInstructionWindow->setPosition( CEGUI::UVector2( CEGUI::UDim( 0.25f, 0 ), CEGUI::UDim( 0.45f, 0 ) ) );
		// m_poInstructionWindow->setText("Appuyez sur la touche ESPACE pour commencer le jeu ...");
		m_poInstructionWindow->setText("Press SPACE to start the game ...");
	}
	else
	{
		m_eGameState = IDLE_TRAINING;

		// draw the initial text
		m_poInstructionWindow->show();
		m_poInstructionWindow->setSize( CEGUI::UVector2( CEGUI::UDim( 0.50f, 0 ), CEGUI::UDim( 0.15f, 0 ) ) );
		m_poInstructionWindow->setPosition( CEGUI::UVector2( CEGUI::UDim( 0.25f, 0 ), CEGUI::UDim( 0.425f, 0 ) ) );
		// m_poInstructionWindow->setText("Le systeme a besoin de faire quelques reglages.\nVeuillez vous concentrer sur les cibles indiques par les instructions.\n\nAppuyez sur la touche ESPACE pour commencer l'entrainement ...");
		m_poInstructionWindow->setText("The system needs to calibrate.\nPlease concentrate on the targets as instructed.\n\nPlease press SPACE to start the process ...");
	}
	return true;
}

void CImpactApplication::setupScene()
{

	m_poSceneManager->setAmbientLight(ColourValue(1.0f, 1.0f, 1.0f));
	m_poCamera->setNearClipDistance(5);

	m_poCamera->setPosition(Ogre::Vector3(0,100,0));
	m_poCamera->lookAt(0, 0, 1);
	m_poCamera->move(Ogre::Vector3(0,0,0));

	Ogre::Light* l_pPointLight;

	l_pPointLight = m_poSceneManager->createLight();
	l_pPointLight->setType(Ogre::Light::LT_POINT);
	l_pPointLight->setPosition(Ogre::Vector3(0, 50, 0));

	l_pPointLight->setDiffuseColour(1.0f, 1.0f, 0.8f);
	l_pPointLight->setSpecularColour(1.0f, 1.0f, 1.0f);

	l_pPointLight = m_poSceneManager->createLight();
	l_pPointLight->setType(Ogre::Light::LT_POINT);
	l_pPointLight->setPosition(Ogre::Vector3(10, -10, 0));

	l_pPointLight->setDiffuseColour(0.5f, 0.0f, 0.0f);

	m_poSceneLoader->parseDotScene("v6.scene", "SSVEPImpact", m_poSceneManager);
	m_poSceneManager->getSceneNode("v6_sky")->showBoundingBox(true);
	m_poSceneManager->getSceneNode("v6_sky")->scale(200.0, 300.0, 300.0);
	m_poSceneManager->getSceneNode("v6_sky")->translate(0.0, 100.0, 0.0);
}

bool CImpactApplication::enemyDestroyed(CImpactEnemyShip* es)
{
	return es->isDestroyed();
}

void CImpactApplication::calculateFeedback(int iChannelCount, double *pChannel)
{
	static int l_iSkipControl = 0;
	static int l_vPreviousLevels[3] = {0, 0, 0};

	if (l_iSkipControl < 2)
	{
		l_iSkipControl++;
		return;
	}
	else
	{
		l_iSkipControl = 0;
	}


	if (m_poAdvancedControl != NULL)
	{
		m_poAdvancedControl->processFrame(pChannel[0], pChannel[1], pChannel[2]);
	}

	int l_vLevel[3];

	// @FIXME bad, modifying non-ref input parameter
	if(iChannelCount!=3) {
		getLogManager() << LogLevel_Warning << "Changing iChannelCount from " << iChannelCount << " to 3\n";
	}
	iChannelCount = 3;

	//std::cout << "feedback: ";
	for (int i = 0; i < iChannelCount; i++)
	{
		if (pChannel[i] > m_pMaxFeedbackLevel[i])
		{
			m_pMaxFeedbackLevel[i] += 0.1;
		}

		float64 l_f64CLevel = pChannel[i] / m_pMaxFeedbackLevel[i];

		//std::cout << cLevel << ", ";
		l_vLevel[i] = 0;

		if (l_f64CLevel < 0.7)
		{
			l_vLevel[i]++;
		}
		if (l_f64CLevel < 0.3)
		{
			l_vLevel[i]++;
		}
		if (l_f64CLevel <= 0.0)
		{
			l_vLevel[i]++;
		}
	}
	//std::cout << "\n";


	if (l_vLevel[0] != l_vPreviousLevels[0] || l_vLevel[1] != l_vPreviousLevels[1] || l_vLevel[2] != l_vPreviousLevels[2])
	{
		logPrefix() << "Feedback levels : " << l_vLevel[0] << " " << l_vLevel[1] << " " << l_vLevel[2] << "\n";

		l_vPreviousLevels[0] = l_vLevel[0];
		l_vPreviousLevels[1] = l_vLevel[1];
		l_vPreviousLevels[2] = l_vLevel[2];

		m_poShip->setFeedbackLevels(l_vLevel[0], l_vLevel[1], l_vLevel[2]);
	}

}

void CImpactApplication::processFrame(OpenViBE::uint32 ui32CurrentFrame)
{

	CApplication::processFrame(ui32CurrentFrame);
	//std::cout << m_poWindow->getAverageFPS() << "\n";

	m_poShip->processFrame( ui32CurrentFrame );
	m_poShip->setTargetLocked(false);
	m_poSceneManager->getSceneNode("v6_sky")->pitch(Radian(0.0001f));

	if (m_eGameState == STARTED)
	{

		OpenViBE::uint32 l_ui32EnemyCount = 0;
		bool l_bTargetWasDestroyed = false;

		int l_iCurrentEnemyPointValue = 0;

		if (m_poCurrentEnemy != NULL)
		{
			l_iCurrentEnemyPointValue = m_poCurrentEnemy->getPointValue();
		}

		m_poCurrentEnemy = NULL;
		for (std::vector<CImpactEnemyShip*>::iterator it = m_oTargets.begin(); it != m_oTargets.end(); ++it)
		{
			(*it)->processFrame();

			if ((*it)->isTouchable())
			{
				m_poShip->evaluateHit(*it);

				Ogre::Vector2 l_vEnemyPosition = (*it)->getEnemyPosition();
				if (l_vEnemyPosition.x > m_poShip->getNormalizedPosition().x - (*it)->getWidth() / 2.0
						&& l_vEnemyPosition.x < m_poShip->getNormalizedPosition().x + (*it)->getWidth() / 2.0)
				{
					m_poCurrentEnemy = (*it);
					m_poShip->setTargetLocked(true);

					if (l_iCurrentEnemyPointValue != m_poCurrentEnemy->getPointValue())
					{
						m_poShip->activateCannonInhibitor(m_poCurrentEnemy->getPointValue() < 0);

						logPrefix() << "Targeting enemy : point value = " << m_poCurrentEnemy->getPointValue() << "\n";
					}
				}

				if ((*it)->getPointValue() > 0)
				{
					l_ui32EnemyCount++;
				}
			}


			if ((*it)->isDestroyed())
			{
				m_iScore += (*it)->getPointValue();
				delete *it;

				// update the score table
				char l_sScore[255];
				sprintf(l_sScore, "Score: %d", m_iScore);
				m_poStatusWindow->setText(l_sScore);

				l_bTargetWasDestroyed = true;
			}
		}

		if (l_iCurrentEnemyPointValue != 0 && m_poCurrentEnemy == NULL)
		{
			logPrefix() << "Focus on enemy lost\n";
		}

		if (l_bTargetWasDestroyed)
		{
			OpenViBE::uint32 l_ui32TargetCount = m_oTargets.size();

			m_oTargets.erase(std::remove_if(m_oTargets.begin(), m_oTargets.end(), enemyDestroyed), m_oTargets.end());

			if (m_oTargets.size() == 0 && m_oTargets.size() < l_ui32TargetCount)
			{
				m_bTargetRequest = true;
			}

			// if there are only friendly targets remaining make them leave
			if (l_ui32TargetCount > 0 && l_ui32EnemyCount == 0)
			{
				for (std::vector<CImpactEnemyShip*>::iterator it = m_oTargets.begin(); it != m_oTargets.end(); ++it)
				{
					if (!(*it)->m_bEnemyLeaving)
					{
						(*it)->m_iLeaveCountdown = 0;
					}
				}
			}
		}
	}

	if (m_ui32DialogHideDelay > 0)
	{
		if (--m_ui32DialogHideDelay == 0)
		{
			m_poInstructionWindow->hide();
		}
	}
}

CImpactEnemyShip* CImpactApplication::getCurrentEnemy()
{
	return m_poCurrentEnemy;
}

int CImpactApplication::getNextTargetType()
{
	int l_iNTT;

	if (m_oEnemyOrder.size() > 0)
	{
		l_iNTT = m_oEnemyOrder.front();
		m_oEnemyOrder.pop();

		return l_iNTT;
	}

	return 0;
}

void CImpactApplication::addTarget(OpenViBE::uint32 ui32TargetPosition)
{
	float l_fTextSize = 0.25f;

	logPrefix() << "Add Target : position = " << ui32TargetPosition << "\n";

	getLogManager() << LogLevel_Info << "Adding target on position " << ui32TargetPosition << "\n";

	if (m_eGameState == TRAINING)
	{
		switch (ui32TargetPosition)
		{
		case 0:
			l_fTextSize = 0.35f;
			// m_poInstructionWindow->setText("Regardez au milieu (MIDDLE) du vaisseau!");
			m_poInstructionWindow->setText("Focus on the MIDDLE of the ship");
			break;

		case 1:
			l_fTextSize = 0.35f;
			// m_poInstructionWindow->setText("Regardez le CANON (CANNON) du vaisseau pour tirer!");
			m_poInstructionWindow->setText("Focus on the CANNON to fire!");
			break;

		case 2:
			l_fTextSize = 0.50f;
			// m_poInstructionWindow->setText("Regardez l'aile GAUCHE (LEFT WING) du vaisseau pour de`placer le vaisseau vers la gauche");
			m_poInstructionWindow->setText("Focus on the LEFT WING to turn the ship to the left");
			break;

		case 3:
			l_fTextSize = 0.50f;
			// m_poInstructionWindow->setText("Regardez l'aile DROITE (RIGHT WING) du vaisseau pour de`placer le vaisseau vers la droite");
			m_poInstructionWindow->setText("Focus on the RIGHT WING to turn the ship to the right");
			break;
		}

		m_poInstructionWindow->setSize( CEGUI::UVector2( CEGUI::UDim( l_fTextSize, 0 ), CEGUI::UDim( 0.10f, 0 ) ) );
		m_poInstructionWindow->setPosition( CEGUI::UVector2( CEGUI::UDim( (1.0f - l_fTextSize) / 2.0f, 0 ), CEGUI::UDim( 0.45f, 0 ) ) );
		m_poInstructionWindow->show();
		m_ui32DialogHideDelay = DIALOG_CYCLES_TO_REMOVE;

	}
	else if (m_eGameState == STARTED)
	{

//		std::cout << (Ogre::Real( ui32TargetPosition ) - 3.5) / 7.0 * 100.0 << "\n";
		if (m_oEnemyOrder.size() > 0)
		{
			m_poShip->returnToMiddleAndWaitForFoe(ui32TargetPosition, m_rNextOrigin);
		}
		else
		{
			getLogManager() << LogLevel_Error << "No more enemy types\n";
		}
	}
}

void CImpactApplication::insertEnemy( int ui32TargetPosition )
{
	logPrefix() << "Creating Enemy : position = " << ui32TargetPosition << "\n";
	m_oTargets.push_back( CImpactEnemyShip::createTarget( (Ogre::Real( ui32TargetPosition ) - 3.5f) / 7.0f * 100.0f ));
}

void CImpactApplication::startFlickering()
{
	m_poInstructionWindow->hide();
	OpenViBE::uint32 l_ui32CurrentTime = (OpenViBE::uint32)(time(NULL) - m_ttStartTime);
	(*m_poLogManager) << LogLevel_Info << l_ui32CurrentTime << "    > Starting Visual Stimulation\n";
	m_bActive = true;
}

void CImpactApplication::stopFlickering()
{
	OpenViBE::uint32 l_ui32CurrentTime = (OpenViBE::uint32)(time(NULL) - m_ttStartTime);
	(*m_poLogManager) << LogLevel_Info << l_ui32CurrentTime << "    > Stopping Visual Stimulation\n";
	m_bActive = false;
	m_poShip->setAllTargetsVisibility(true);
}

void CImpactApplication::startExperiment()
{
	CApplication::startExperiment();

	if (m_eGameState == IDLE_TRAINING)
	{
		m_ttStartTime = time(NULL);
		m_poInstructionWindow->hide();
		m_ui32DialogHideDelay = 0;
		m_eGameState = TRAINING;

		m_poShip->activatePilotAssist(false);

		logPrefix() << "Start Experiment : IDLE_TRAINING -> TRAINING\n";

	}
	else if (m_eGameState == IDLE_STARTED)
	{

		m_bTargetRequest = true;
		m_bActive = true;
		m_poInstructionWindow->hide();
		m_ui32DialogHideDelay = 0;
		m_poStatusWindow->setPosition( CEGUI::UVector2( CEGUI::UDim( 0.0f, 0 ), CEGUI::UDim( 0.0f, 0 ) ) );
		m_poStatusWindow->setSize( CEGUI::UVector2( CEGUI::UDim( 0.15f, 0 ), CEGUI::UDim( 0.05f, 0 ) ) );
		m_poStatusWindow->setText("Score: 0");
		m_poStatusWindow->show();
		m_eGameState = STARTED;

		logPrefix() << "Start Experiment : IDLE_STARTED -> STARTED\n";

	}

}

void CImpactApplication::stopExperiment()
{
	if (m_eGameState == TRAINING)
	{
		if (m_sApplicationSubtype == CString("trainer"))
		{
			logPrefix() << "Stop Experiment : exiting\n";
			this->exit();
		}
		else
		{
			OpenViBE::float32 l_fTextSize = 0.35f;
			// m_poInstructionWindow->setText("L'application va prendre quelque temps pour le calcul, veuillez patienter.\nLe jeu va commencer automatiquement une fois l'application est prete.");
			m_poInstructionWindow->setText("The application needs some time to compute, please be patient.\nThe game starts automatically when the application is ready.");
			m_poInstructionWindow->setSize( CEGUI::UVector2( CEGUI::UDim( l_fTextSize, 0 ), CEGUI::UDim( 0.10f, 0 ) ) );
			m_poInstructionWindow->setPosition( CEGUI::UVector2( CEGUI::UDim( (1.0f - l_fTextSize) / 2.0f, 0 ), CEGUI::UDim( 0.45f, 0 ) ) );
			m_poInstructionWindow->show();

			logPrefix() << "Stop Experiment : Training Finished\n";
		}
	}

	if (m_eGameState == STARTED)
	{
		OpenViBE::float32 l_fTextSize = 0.45f;
		char l_sText[1024];
		// sprintf(l_sText, "Bravo!\nVous avez obtenu %d points!\nAppuyez sur ECHAP pour fermer le programme", m_iScore);
		sprintf(l_sText, "Bravo!\nYou have obtained %d points!\nPress ESC to close the program", m_iScore);
		m_poInstructionWindow->setText(l_sText);
		m_poInstructionWindow->setSize( CEGUI::UVector2( CEGUI::UDim( l_fTextSize, 0 ), CEGUI::UDim( 0.10f, 0 ) ) );
		m_poInstructionWindow->setPosition( CEGUI::UVector2( CEGUI::UDim( (1.0f - l_fTextSize) / 2.0f, 0 ), CEGUI::UDim( 0.45f, 0 ) ) );
		m_poInstructionWindow->show();
		m_bActive = false;
		m_poShip->setAllTargetsVisibility(true);

		logPrefix() << "Stop Experiment : Game Ended\n";
	}
}

void CImpactApplication::debugAction1()
{
	(*m_poLogManager) << LogLevel_Debug << "Debug Action 1 triggered\n";
	this->startExperiment();
	m_poShip->activatePilotAssist(true);
}

void CImpactApplication::debugAction2()
{
	(*m_poLogManager) << LogLevel_Info << "Debug Action 2 triggered\n";

 //if (m_bTargetRequest && m_oEnemyOrder.size() > 2)
	{
		this->addTarget(1);
		this->addTarget(6);
		this->addTarget(4);
		m_bTargetRequest = false;
	}
}

void CImpactApplication::debugAction3()
{
	(*m_poLogManager) << LogLevel_Info << "Debug Action 3 triggered\n";

	m_bTargetRequest = true;
}

void CImpactApplication::debugAction4()
{
	(*m_poLogManager) << LogLevel_Info << "Debug Action 4 triggered\n";

	(*m_poLogManager) << LogLevel_Info << "You (X: " << m_poShip->getPosition().x << ", Y: " << m_poShip->getPosition().y << ")\n";

	if (m_oTargets.size() > 0)
	{
		Ogre::Vector2 l_oEnemyPosition = m_oTargets[0]->getEnemyPosition();
		(*m_poLogManager) << LogLevel_Info << "Him (X: " << l_oEnemyPosition.x << ", Y: " << l_oEnemyPosition.y << ")\n";
	}

}

#endif