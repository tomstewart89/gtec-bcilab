
#if defined(TARGET_HAS_ThirdPartyOgre3D)

#include "ovavrdCOgreVRApplication.h"

#include <system/ovCTime.h>
#include <fs/Files.h>

#include <iostream>

#include <vrpn_Tracker.h>
#include <vrpn_Button.h>
#include <vrpn_Analog.h>

#include <openvibe/ov_directories.h>

#if defined sleep
#undef sleep
#endif

using namespace OpenViBEVRDemos;
using namespace Ogre;

static const float g_fRotationSpeedMouse = 0.5f;
static const float g_fTranslationSpeed = 0.2f;

COgreVRApplication::COgreVRApplication(const char *sResourceRoot) :
	m_sResourcePath(sResourceRoot)
{
	m_dClock = 0;
	m_bContinue=true;
	m_rGUIRenderer = NULL;
	m_bCameraMode = false;
	m_poVrpnPeripheral = NULL;
	m_poCamera = NULL;
	m_poInputManager = NULL;
	m_poSceneManager = NULL;
	m_poMouse = NULL;
	m_poKeyboard = NULL;
}

COgreVRApplication::~COgreVRApplication()
{
	if(m_poSceneManager)
		m_poSceneManager->clearScene(); // does not destroy cameras

	if(m_poVrpnPeripheral)
		delete m_poVrpnPeripheral;

	if(m_poCamera)
		delete m_poCamera;

	if(m_poWindow)
		delete m_poWindow;

	if( m_poInputManager ) {
        if( m_poMouse ) {
            m_poInputManager->destroyInputObject( m_poMouse );
            m_poMouse = NULL;
        }

        if( m_poKeyboard ) {
            m_poInputManager->destroyInputObject( m_poKeyboard );
            m_poKeyboard = NULL;
        }

        m_poInputManager->destroyInputSystem( m_poInputManager );
	}

	if(m_rGUIRenderer)
	{
		CEGUI::OgreRenderer::destroySystem();
	}

}

void COgreVRApplication::go(void)
{
	if (!this->setup())
	{
		std::cerr<<"[FAILED] Setup failed, end of program."<< std::endl;
		return;
	}

	this->initialise();

	std::cout<<std::endl<< "START RENDERING..."<<std::endl;

    m_poRoot->startRendering();
}

bool COgreVRApplication::setup()
{
	// Plugin config path setup
	Ogre::String pluginsPath;
#if defined TARGET_OS_Windows
 #if defined TARGET_BUILDTYPE_Debug
	pluginsPath = std::string(getenv("OGRE_HOME"))+std::string("/bin/debug/plugins_d.cfg");
 #else
	pluginsPath = std::string(getenv("OGRE_HOME"))+std::string("/bin/release/plugins.cfg");
 #endif
#elif defined TARGET_OS_Linux
	pluginsPath = std::string(OpenViBE::Directories::getDataDir()) + std::string("/openvibe-ogre-plugins.cfg");
#else
	#error "failing text"
#endif

	// Create LogManager to stop Ogre flooding the console and creating random files
	OpenViBE::CString l_sOgreLog = OpenViBE::Directories::getLogDir() + "/openvibe-vr-demo-ogre.log";
	std::cout << "+ Ogre log will be in " << l_sOgreLog << "\n";
	Ogre::LogManager* l_poLogManagerSingleton = new Ogre::LogManager();
	FS::Files::createParentPath(l_sOgreLog);
	l_poLogManagerSingleton->createLog(l_sOgreLog.toASCIIString(), true, false, false );

	// Root creation
	OpenViBE::CString l_sOgreCfg = OpenViBE::Directories::getUserDataDir() + "/openvibe-vr-demo-ogre.cfg";
	std::cout << "+ Ogre cfg will be in " << l_sOgreCfg << "\n";
	m_poRoot = new Ogre::Root(pluginsPath, l_sOgreCfg.toASCIIString(), l_sOgreLog.toASCIIString() );
	// Resource handling
	this->setupResources();
	//Configuration from file or dialog window if needed
	if (!this->configure())
	{
		std::cerr<<"[FAILED] The configuration process ended unexpectedly."<< std::endl;
		return false;
	}

	// load ressources
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	// Scene graph and rendering initialisation
	// m_poSceneManager = m_poRoot->createSceneManager("TerrainSceneManager", "DefaultSceneManager");
	m_poSceneManager = m_poRoot->createSceneManager(Ogre::ST_GENERIC);

	//Camera
	m_poCamera = m_poSceneManager->createCamera("DefaultCamera");
	m_poCameraNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode();
	m_poCameraNode->attachObject(m_poCamera);
	//m_poCameraNodeYawAndPos = m_poSceneManager->getRootSceneNode()->createChildSceneNode();
	//m_poCameraNodeYawAndPos->setPosition(Ogre::Vector3::ZERO);
	//m_poCameraNodeYawAndPos->setOrientation(Ogre::Quaternion::IDENTITY);

	//m_poCameraNodePitch = m_poCameraNodeYawAndPos->createChildSceneNode();
	//m_poCameraNodePitch->setPosition(Ogre::Vector3::ZERO);
	//m_poCameraNodePitch->setOrientation(Ogre::Quaternion::IDENTITY);
	//
	//m_poCameraNodePitch->attachObject(m_poCamera);

	m_poCamera->setNearClipDistance(0.05f);
	m_poCamera->setFarClipDistance(300.0f);
	m_poCamera->setRenderingDistance(0.01f);

	// Create one viewport, entire window
    Ogre::Viewport* l_poViewPort = m_poWindow->addViewport(m_poCamera);
    l_poViewPort->setBackgroundColour(Ogre::ColourValue(0,0,0));
    // Alter the camera aspect ratio to match the viewport
    m_poCamera->setAspectRatio(Ogre::Real(l_poViewPort->getActualWidth()) / Ogre::Real(l_poViewPort->getActualHeight()));

	// Set default mipmap level (NB some APIs ignore this)
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

	//Listening the frame rendering
	m_poRoot->addFrameListener(this);

	//Listening the window events
	WindowEventUtilities::addWindowEventListener(m_poWindow,this);

	//OIS
	this->initOIS();

	//CEGUI
	this->initCEGUI(OpenViBE::Directories::getLogDir() + "/openvibe-vr-demo-cegui.log");

	//VRPN
	m_poVrpnPeripheral = new CAbstractVrpnPeripheral("openvibe-vrpn@localhost");
	m_poVrpnPeripheral->init();

	return true;
}

void COgreVRApplication::setupResources()
{
	 // Load resource paths from config file
    Ogre::ConfigFile l_oConfigFile;
    l_oConfigFile.load(m_sResourcePath + "/resources.cfg");
    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator l_oSectionIterator = l_oConfigFile.getSectionIterator();

    Ogre::String l_sSecName, l_sTypeName, l_sArchName;
    while (l_oSectionIterator.hasMoreElements())
    {
        l_sSecName = l_oSectionIterator.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = l_oSectionIterator.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            l_sTypeName = i->first;
            l_sArchName = i->second;

            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(m_sResourcePath + "/" + l_sArchName, l_sTypeName, l_sSecName);
        }
    }
}

bool COgreVRApplication::configure()
{
	if(! m_poRoot->restoreConfig())
	{
		if( ! m_poRoot->showConfigDialog() )
		{
			std::cerr<<"[FAILED] No configuration created from the dialog window."<< std::endl;
			return false;
		}
	}

	m_poWindow = m_poRoot->initialise(true,"VR Application - powered by OpenViBE");

	return true;
}

bool COgreVRApplication::initOIS()
{
	OIS::ParamList l_oParamList;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	// Retrieve the rendering window
	RenderWindow* window = Ogre::Root::getSingleton().getAutoCreatedWindow();
	window->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	l_oParamList.insert(make_pair(std::string("WINDOW"), windowHndStr.str()));
	l_oParamList.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
    l_oParamList.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));

	// Create the input manager
	m_poInputManager = OIS::InputManager::createInputSystem( l_oParamList );

	//Create all devices
	m_poKeyboard = static_cast<OIS::Keyboard*>(m_poInputManager->createInputObject( OIS::OISKeyboard, true ));
	m_poMouse = static_cast<OIS::Mouse*>(m_poInputManager->createInputObject( OIS::OISMouse, true ));

	m_poKeyboard->setEventCallback(this);
	m_poMouse->setEventCallback(this);

	std::cout<<"OIS initialised"<<std::endl;

	return true;
}

bool COgreVRApplication::initCEGUI(const char *logFilename)
{
	// Instantiate logger before bootstrapping the system, this way we will be able to get the log redirected
	if (!CEGUI::Logger::getSingletonPtr()) 
	{
		new CEGUI::DefaultLogger();		// Singleton, instantiate only, no delete
	}
	std::cout << "+ CEGUI log will be in " << logFilename << "\n";
	FS::Files::createParentPath(logFilename);
	try {
		CEGUI::Logger::getSingleton().setLogFilename(logFilename, false);
	} catch (char *error) {
		std::cout << "  CEGUI::getSingleton() exception: " << error << "\n";
	} catch (const char *error) {
		std::cout << "  CEGUI::getSingleTon() exception: " << error << "\n";
	}

	m_rGUIRenderer = &(CEGUI::OgreRenderer::bootstrapSystem(*m_poWindow));

	CEGUI::SchemeManager::getSingleton().create((CEGUI::utf8*)"TaharezLook-ov.scheme");

	m_poGUIWindowManager = CEGUI::WindowManager::getSingletonPtr();
	m_poSheet = m_poGUIWindowManager->createWindow("DefaultGUISheet", "Sheet");

	CEGUI::System::getSingleton().setGUISheet(m_poSheet);

	return true;
}


//--------------------------------------------------------------

bool COgreVRApplication::keyPressed(const OIS::KeyEvent& evt)
{
	if(evt.key == OIS::KC_ESCAPE)
	{
		std::cout<<"[ESC] pressed, user termination."<<std::endl;
		m_bContinue = false;
	}
	if(evt.key == OIS::KC_RCONTROL)
	{
		std::cout<<"Camera mode ON"<<std::endl;
		m_bCameraMode = true;
	}

	
	if(evt.key == OIS::KC_UP)
	{
		m_mKeysPressed[OIS::KC_UP] = true;
	}
	if(evt.key == OIS::KC_RIGHT)
	{
		m_mKeysPressed[OIS::KC_RIGHT] = true;
	}
	if(evt.key == OIS::KC_LEFT)
	{
		m_mKeysPressed[OIS::KC_LEFT] = true;
	}
	if(evt.key == OIS::KC_DOWN)
	{
		m_mKeysPressed[OIS::KC_DOWN] = true;
	}
	
	return true;
}


bool COgreVRApplication::keyReleased(const OIS::KeyEvent& evt)
{
	if(evt.key == OIS::KC_RCONTROL)
	{
		std::cout<<"Camera mode OFF"<<std::endl;
		m_bCameraMode = false;
	}

	if(evt.key == OIS::KC_UP)
	{
		m_mKeysPressed[OIS::KC_UP] = false;
	}
	if(evt.key == OIS::KC_RIGHT)
	{
		m_mKeysPressed[OIS::KC_RIGHT] = false;
	}
	if(evt.key == OIS::KC_LEFT)
	{
		m_mKeysPressed[OIS::KC_LEFT] = false;
	}
	if(evt.key == OIS::KC_DOWN)
	{
		m_mKeysPressed[OIS::KC_DOWN] = false;
	}

	
	return true;
}

bool COgreVRApplication::mouseMoved(const OIS::MouseEvent &arg)
{
	if(m_bCameraMode)
	{
		m_poCamera->yaw(Ogre::Degree(-arg.state.X.rel * g_fRotationSpeedMouse));
		m_poCamera->pitch(Ogre::Degree(-arg.state.Y.rel * g_fRotationSpeedMouse));
		
	}

	return true;
}

void COgreVRApplication::updateCamera()
{
	Vector3 l_vTranslation(0,0,0);
	if(m_mKeysPressed[OIS::KC_UP])
	{
		l_vTranslation.z -= g_fTranslationSpeed;
	}
	if(m_mKeysPressed[OIS::KC_RIGHT])
	{
		l_vTranslation.x += g_fTranslationSpeed;
	}
	if(m_mKeysPressed[OIS::KC_LEFT])
	{
		l_vTranslation.x -= g_fTranslationSpeed;
	}
	if(m_mKeysPressed[OIS::KC_DOWN])
	{
		l_vTranslation.z += g_fTranslationSpeed;
	}
	Vector3 l_vTranslateVectorFinal = m_poCamera->getDerivedOrientation() * l_vTranslation;
	m_poCameraNode->translate(l_vTranslateVectorFinal, Ogre::Node::TS_WORLD);
}

bool COgreVRApplication::frameStarted(const FrameEvent& evt)
{
	m_dClock += evt.timeSinceLastFrame;
	if(m_dClock >= 1/MAX_FREQUENCY)
	{
		m_poKeyboard->capture();
		m_poMouse->capture();

		m_poVrpnPeripheral->loop();
		//the button states are added in the peripheric, but they have to be popped.
		//the basic class does not pop the states.

		if(m_bCameraMode) this->updateCamera();

		m_bContinue = this->process(m_dClock);
		m_dClock -= 1/MAX_FREQUENCY;
	}
	else
	{
		System::Time::sleep(1);
	}

	return m_bContinue;
}

void COgreVRApplication::windowResized(RenderWindow* rw)
{
	CEGUI::System::getSingleton().notifyDisplaySizeChanged(CEGUI::Size((float)rw->getWidth(), (float)rw->getHeight()));
}

#endif