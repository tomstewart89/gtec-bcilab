
#if defined(TARGET_HAS_ThirdPartyOgre3D)

#include "ovassvepCApplication.h"
#include <cmath>

#include "fs/Files.h"

using namespace OpenViBE;
using namespace OpenViBESSVEP;
using namespace OpenViBE::Kernel;

#define MIN(a,b) ( a < b ? a : b )

	CApplication::CApplication()
: m_bContinueRendering( true ),
	m_ui32CurrentFrame( 0 ),
	m_roGUIRenderer( NULL )
{
}

CApplication::~CApplication()
{

	if (m_poPainter != NULL)
	{
		(*m_poLogManager) << LogLevel_Debug << "- m_poPainter\n";
		delete m_poPainter;
		m_poPainter = NULL;
	}

	for (std::vector<ICommand*>::iterator it = m_oCommands.begin();
			it != m_oCommands.end(); ++it) 
	{
		(*m_poLogManager) << LogLevel_Debug << "- ICommand\n";
		if (*it != NULL)
		{
			delete *it;
			*it = NULL;
		}
	}

	
	(*m_poLogManager) << LogLevel_Debug << "- m_poRoot\n";
	if (m_poRoot != NULL)
	{
		delete m_poRoot;
		m_poRoot = NULL;
	}
	
}

bool CApplication::setup(OpenViBE::Kernel::IKernelContext* poKernelContext)
{
	m_poKernelContext = poKernelContext;
	m_poLogManager = &(m_poKernelContext->getLogManager());

	IConfigurationManager* l_poConfigurationManager = &(m_poKernelContext->getConfigurationManager());

	(*m_poLogManager) << LogLevel_Debug << "  * CApplication::setup()\n";

	// Plugin config path setup
	OpenViBE::CString l_sPluginsPath = l_poConfigurationManager->expand("${Kernel_3DVisualisationOgrePlugins}");

	// Create LogManager to stop Ogre flooding the console and creating random files
	
	
	(*m_poLogManager) << LogLevel_Debug << "+ Creating Ogre logmanager\n";
	Ogre::LogManager* l_poLogManager = new Ogre::LogManager();
	(*m_poLogManager) << LogLevel_Info << "Ogre log to console : " << l_poConfigurationManager->expandAsBoolean("${SSVEP_Ogre_LogToConsole}", false) << "\n";
	CString l_sOgreLog = l_poConfigurationManager->expand("${Path_Log}") + "/openvibe-ssvep-demo-ogre.log";
	(*m_poLogManager) << LogLevel_Info << "Ogre log file : " << l_sOgreLog << "\n";
	FS::Files::createParentPath(l_sOgreLog);
	l_poLogManager->createLog(l_sOgreLog.toASCIIString(), true, l_poConfigurationManager->expandAsBoolean("${SSVEP_Ogre_LogToConsole}", false), false );
	
	// Root creation
	CString l_sOgreCfg = l_poConfigurationManager->expand("${Path_UserData}") + "/openvibe-ssvep-demo-ogre.cfg";
	(*m_poLogManager) << LogLevel_Debug << "+ m_poRoot = new Ogre::Root(...)\n";
	(*m_poLogManager) << LogLevel_Info << "Ogre cfg file : " << l_sOgreCfg << "\n";
	m_poRoot = new Ogre::Root(l_sPluginsPath.toASCIIString(), l_sOgreCfg.toASCIIString(), l_sOgreLog.toASCIIString());

	// Resource handling
	this->setupResources();

	// Configuration from file or dialog window if needed
	if (!this->configure())
	{
		(*m_poLogManager) << LogLevel_Fatal << "The configuration process ended unexpectedly.\n";
		return false;
	}

	// m_poWindow = m_poRoot->initialise(true);


	Ogre::NameValuePairList l_oOptionList;
	m_poRoot->initialise(false);

	l_oOptionList["vsync"] = "1";

	m_poWindow = m_poRoot->createRenderWindow("SSVEP Stimulator", 640, 480, l_poConfigurationManager->expandAsBoolean("${SSVEP_Ogre_FullScreen}", false), &l_oOptionList);
	m_ui32WindowWidth = m_poWindow->getWidth();
	m_ui32WindowHeight = m_poWindow->getHeight();

	m_poSceneManager = m_poRoot->createSceneManager(Ogre::ST_GENERIC);
	m_poCamera = m_poSceneManager->createCamera("SSVEPApplicationCamera");

	Ogre::SceneManager* l_poFillSceneManager = m_poRoot->createSceneManager(Ogre::ST_GENERIC);
	Ogre::Camera* l_poFillCamera = l_poFillSceneManager->createCamera("SSVEPFillCamera");
	m_poWindow->addViewport(l_poFillCamera, 0);

	m_poViewport = m_poWindow->addViewport(m_poCamera, 1);
	this->resizeViewport();
//	m_poViewport->setBackgroundColour(Ogre::ColourValue(0.0, 0.5, 0.5));

	m_poCamera->setAspectRatio(Ogre::Real(m_poViewport->getActualWidth()) / Ogre::Real(m_poViewport->getActualHeight()));

	m_poSceneNode = m_poSceneManager->getRootSceneNode()->createChildSceneNode("SSVEPApplicationNode");

	// initialize the painter object
	(*m_poLogManager) << LogLevel_Debug << "+ m_poPainter = new CBasicPainter(...)\n";
	m_poPainter = new CBasicPainter( this );

	(*m_poLogManager) << LogLevel_Debug << "  * initializing CEGUI\n";
	this->initCEGUI(l_poConfigurationManager->expand("${Path_Log}") + "/openvibe-ssvep-demo-cegui.log");
	(*m_poLogManager) << LogLevel_Debug << "  * CEGUI initialized\n";

	// create the vector of stimulation frequencies
	
	m_f64ScreenRefreshRate = (OpenViBE::uint32)(l_poConfigurationManager->expandAsUInteger("${SSVEP_ScreenRefreshRate}"));

	(*m_poLogManager) << LogLevel_Info << "Specified screen refresh rate :" << m_f64ScreenRefreshRate << "Hz\n";

	OpenViBE::uint32 i = 1;

	char l_sFrequencyString[32] = "1";

	CIdentifier l_oFrequencyId = l_poConfigurationManager->createConfigurationToken("SSVEP_FrequencyId", CString(l_sFrequencyString));

	m_oFrequencies.push_back(std::pair<OpenViBE::uint32, OpenViBE::uint32>(30, 30));

	while (l_poConfigurationManager->lookUpConfigurationTokenIdentifier(l_poConfigurationManager->expand("SSVEP_Frequency_${SSVEP_FrequencyId}")) != OV_UndefinedIdentifier)
	{
		std::pair<OpenViBE::uint32, OpenViBE::uint32> l_oFrequency;

		OpenViBE::float64 l_f64CurrentFrequency;
		OpenViBE::float64 l_f64ApproximatedFrameCount;

		l_f64CurrentFrequency = (OpenViBE::float64)(l_poConfigurationManager->expandAsFloat("${SSVEP_Frequency_${SSVEP_FrequencyId}}"));

		l_f64ApproximatedFrameCount= m_f64ScreenRefreshRate / l_f64CurrentFrequency; 

		if (fabs(l_f64ApproximatedFrameCount - floor(l_f64ApproximatedFrameCount + 0.5)) < 0.003)
		{

			l_oFrequency.first = int(floor(l_f64ApproximatedFrameCount + 0.5)) / 2 + int(floor(l_f64ApproximatedFrameCount + 0.5)) % 2;
			l_oFrequency.second = int(floor(l_f64ApproximatedFrameCount + 0.5)) / 2;

			(*m_poLogManager) << LogLevel_Info << "Frequency number " << i << ": " << l_f64CurrentFrequency << "Hz / " << floor(l_f64ApproximatedFrameCount + 0.5) << " ( " << l_oFrequency.first << ", " << l_oFrequency.second<< ") frames @ " << m_f64ScreenRefreshRate << "fps\n";

			m_oFrequencies.push_back(l_oFrequency);	
		}
		else
		{
			(*m_poLogManager) << LogLevel_Error << "The selected frequency (" << l_f64CurrentFrequency << "Hz) is not supported by your screen.\n";
		}

		l_poConfigurationManager->releaseConfigurationToken(l_oFrequencyId);

		sprintf(l_sFrequencyString, "%d", ++i);

		l_oFrequencyId = l_poConfigurationManager->createConfigurationToken("SSVEP_FrequencyId", CString(l_sFrequencyString));
	}


	return true;
}

bool CApplication::configure()
{
	if(! m_poRoot->restoreConfig())
	{
		if( ! m_poRoot->showConfigDialog() )
		{
			(*m_poLogManager) << LogLevel_Error << "No configuration created from the dialog window.\n";
			return false;
		}
	}

	// Set hard-coded parameters, VSync in particular
	m_poRoot->getRenderSystem()->setConfigOption("VSync", "True");
//	m_poRoot->getRenderSystem()->setConfigOption("Full Screen", "Yes");
	m_poRoot->getRenderSystem()->setConfigOption("Video Mode","640 x 480 @ 16-bit colour");


	return true;
}

void CApplication::initCEGUI(const char *logFilename)
{
	// Instantiate logger before bootstrapping the system, this way we will be able to get the log redirected
	if (!CEGUI::Logger::getSingletonPtr()) 
	{
		new CEGUI::DefaultLogger();		// singleton; instantiate only, no delete
	}
	(*m_poLogManager) << LogLevel_Info << "+ CEGUI log will be in '" << logFilename << "'\n";
	FS::Files::createParentPath(logFilename);
	CEGUI::Logger::getSingleton().setLogFilename(logFilename, false);

	(*m_poLogManager) << LogLevel_Debug << "+ Creating CEGUI Ogre bootstrap\n";
	m_roGUIRenderer = &(CEGUI::OgreRenderer::bootstrapSystem(*m_poWindow));
	(*m_poLogManager) << LogLevel_Debug << "+ Creating CEGUI Scheme Manager\n";
	CEGUI::SchemeManager::getSingleton().create((CEGUI::utf8*)"TaharezLook-ov.scheme");

	(*m_poLogManager) << LogLevel_Debug << "+ Creating CEGUI WindowManager\n";
	m_poGUIWindowManager = CEGUI::WindowManager::getSingletonPtr();
	m_poSheet = m_poGUIWindowManager->createWindow("DefaultWindow", "Sheet");

	(*m_poLogManager) << LogLevel_Debug << "+ Setting CEGUI StyleSheet\n";
	CEGUI::System::getSingleton().setGUISheet(m_poSheet);
}

void CApplication::resizeViewport()
{
	(*m_poLogManager) << LogLevel_Trace << "Creating a new viewport\n";

	Ogre::uint32 l_ui32ViewportSize = MIN(m_ui32WindowWidth, m_ui32WindowHeight);
	(*m_poLogManager) << LogLevel_Info << "New viewport size : " << l_ui32ViewportSize << "\n";

	m_poViewport->setDimensions(
				Ogre::Real(m_ui32WindowWidth - l_ui32ViewportSize) / Ogre::Real(m_ui32WindowWidth) / 2,
				Ogre::Real(m_ui32WindowHeight - l_ui32ViewportSize) / Ogre::Real(m_ui32WindowHeight) / 2,
				Ogre::Real(l_ui32ViewportSize) / Ogre::Real(m_ui32WindowWidth),
				Ogre::Real(l_ui32ViewportSize) / Ogre::Real(m_ui32WindowHeight)
				);
}

void CApplication::processFrame(OpenViBE::uint32 ui32CurrentFrame)
{
	if (m_ui32WindowWidth != m_poWindow->getWidth() || m_ui32WindowHeight != m_poWindow->getHeight())
	{
		m_ui32WindowWidth = m_poWindow->getWidth();
		m_ui32WindowHeight = m_poWindow->getHeight();
		this->resizeViewport();
	}

}


void CApplication::setupResources()
{
	Ogre::ResourceGroupManager::getSingleton().createResourceGroup("SSVEP");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation((OpenViBE::Directories::getDataDir() + "/applications/ssvep-demo/resources").toASCIIString(), "FileSystem", "SSVEP");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation((OpenViBE::Directories::getDataDir() + "/applications/ssvep-demo/resources/trainer").toASCIIString(), "FileSystem", "SSVEPTrainer");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation((OpenViBE::Directories::getDataDir() + "/applications/ssvep-demo/resources/gui").toASCIIString(), "FileSystem", "CEGUI");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("SSVEP");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("SSVEPTrainer");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("CEGUI");
}

bool CApplication::frameRenderingQueued(const Ogre::FrameEvent &evt)
{
	return (m_bContinueRendering && !m_poWindow->isClosed());
}

bool CApplication::frameStarted(const Ogre::FrameEvent &evt)
{
	++m_ui32CurrentFrame;
	m_ui32CurrentFrame = m_ui32CurrentFrame % int(m_f64ScreenRefreshRate);

	this->processFrame(m_ui32CurrentFrame);

	for (OpenViBE::uint32 i = 0; i < m_oCommands.size(); i++)
	{
		m_oCommands[i]->processFrame();
	}

	return true;
}

void CApplication::go()
{
	(*m_poLogManager) << LogLevel_Debug << "Associating application as Ogre frame listener\n";

	m_poRoot->addFrameListener(this);

	(*m_poLogManager) << LogLevel_Debug << "Entering Ogre rendering loop\n";
	m_poRoot->startRendering();
	(*m_poLogManager) << LogLevel_Debug << "Ogre rendering loop finished ... exiting\n";
}

void CApplication::addCommand(ICommand* pCommand)
{
	m_oCommands.push_back(pCommand);
}

void CApplication::startExperiment()
{
	(*m_poLogManager) << LogLevel_Info << "[!] Experiment starting\n";
}

void CApplication::stopExperiment()
{
	(*m_poLogManager) << LogLevel_Info << "[!] Experiment halting\n";
	this->exit();
}


#endif