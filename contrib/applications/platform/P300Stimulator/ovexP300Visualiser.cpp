#include "ovexP300Visualiser.h"

#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <system/ovCTime.h>
#include <GLFW/glfw3.h>

#include <openvibe/ovITimeArithmetics.h>//if debug?

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

CoAdaptP300Visualiser * g_CoAdaptVisualiser;

CoAdaptP300Visualiser::CoAdaptP300Visualiser()
{
	m_bChanged = false;
	m_bInRest = true;
	m_bInTarget = false;
	m_bInFeedback = false;

	m_ui32FeedbackCueCounter = 0;
	m_ui32FeedbackResultCounter = 0;
	m_ui32PreviousFeedbackResultCounter = 0;

	m_bReplayMode=false;
	m_vCurrentFlashGroup = new std::vector<OpenViBE::uint32>();

	//initializes openvibe kernel
	this->initializeOpenViBEKernel();

	#ifdef OUTPUT_TIMING
	const CString l_sPathUserData = m_pKernelContext->getConfigurationManager().expand(CString("${Path_UserData}"));
	timingFile = fopen(l_sPathUserData + "/xP300-symbol_update_timing.txt","w");
	fprintf(timingFile, "%s \n","beforeUpdate;afterUpdate;");
	timingFile3 = fopen(l_sPathUserData + "/xP300-generate_sequence_timing.txt","w");
	#endif
	

	
	//this will read all configuration files, interface-properties.xml, stimulator-properties.xml and the keyboard layout in share/openvibe/applications/CoAdaptP300Stimulator/
	this->m_pInterfacePropReader = new P300InterfacePropertyReader(this->m_pKernelContext);

	CString l_sPathRoot = m_pKernelContext->getConfigurationManager().expand(CString("${Path_Root}"));

	this->m_pInterfacePropReader->readPropertiesFromFile(l_sPathRoot + "/share/openvibe/applications/CoAdaptP300Stimulator/interface-properties.xml");
	this->m_pScreenLayoutReader = new P300ScreenLayoutReader(this->m_pKernelContext);
	this->m_pScreenLayoutReader->readPropertiesFromFile(this->m_pInterfacePropReader->getScreenDefinitionFile());
	this->m_pStimulatorPropReader = new P300StimulatorPropertyReader(this->m_pKernelContext, this->m_pScreenLayoutReader->getSymbolList());
	this->m_pStimulatorPropReader->readPropertiesFromFile(this->m_pInterfacePropReader->getStimulatorConfigFile());	
	


	//In case hardware of software tagging is enable a different object for tagging is constructed
	if (!this->m_pInterfacePropReader->getHardwareTagging())
	{
		try
		{
			this->m_pTagger = new CSoftTagger();
		}
		catch (exception& e)
		{
			this->m_pKernelContext->getLogManager() << LogLevel_Info << "Opening software tagger failed: " << e.what() << "\n";
		}
		if (this->m_pTagger->open())
			this->m_pKernelContext->getLogManager() << LogLevel_Info << "Opened software tagger\n";
	}
#if defined TARGET_OS_Linux || (defined TARGET_OS_Windows && defined TARGET_HAS_ThirdPartyInpout)
	else if (this->m_pInterfacePropReader->getHardwareTagging())
	{
		this->m_pTagger = new ParallelPort(this->m_pInterfacePropReader->getParallelPortNumber(),
									this->m_pInterfacePropReader->getSampleFrequency());
		if (this->m_pTagger->open())
			this->m_pKernelContext->getLogManager() << LogLevel_Info << "Opened parallel port\n";
	}
#endif
	
	//this is a file writer that will write the generated flash groups to a file when they are generator by the sequence generator
	m_pSequenceWriter = new P300SequenceFileWriter(this->m_pInterfacePropReader->getFlashGroupDefinitionFile()); 
	
	//this will create a sequence generator that is responsible for defining which symbols are in each flash. This is either row-column or RIPRAND
	//P300SequenceGenerator* m_pSequenceGenerator;
	if (this->m_pInterfacePropReader->getFlashMode()==CString("rowcol"))
		m_pSequenceGenerator = new P300RowColumnSequenceGenerator(
			this->m_pScreenLayoutReader->getNumberOfKeys(), 
			this->m_pStimulatorPropReader->getNumberOfGroups(), 
			this->m_pStimulatorPropReader->getNumberOfRepetitions());		
	else if(this->m_pInterfacePropReader->getFlashMode()==CString("file"))
		m_pSequenceGenerator = new ovexP300CSVReader(
					this->m_pScreenLayoutReader->getNumberOfKeys(),
					this->m_pStimulatorPropReader->getNumberOfGroups(),
					this->m_pStimulatorPropReader->getNumberOfRepetitions());
	else
		m_pSequenceGenerator = new P300RipRandSequenceGenerator(
			this->m_pScreenLayoutReader->getNumberOfKeys(), 
			this->m_pStimulatorPropReader->getNumberOfGroups(), 
			this->m_pStimulatorPropReader->getNumberOfRepetitions());

	//register the file write with the sequence generator
	m_pSequenceGenerator->setSequenceWriter(m_pSequenceWriter);
	
	//create the stimulator object and register the callback function that is implemented above.
	//if we are in replay, create a NULLStimulator
	if(m_pInterfacePropReader->getStimulatorMode()==CString("Replay"))
	{
		this->m_pKernelContext->getLogManager() << LogLevel_Info << " REPLAY MODE " << m_pInterfacePropReader->getStimulatorMode().toASCIIString() <<"\n";
		this->m_oStimulator = new CoAdaptP300CNULLStimulator(this->m_pStimulatorPropReader, m_pSequenceGenerator);
		this->m_pSequenceGenerator->generateSequence();//to test csv reading
		m_bReplayMode=true;
	}
	else
	{
		this->m_pKernelContext->getLogManager() << LogLevel_Info << " ONLINE MODE " << m_pInterfacePropReader->getStimulatorMode().toASCIIString() << "\n";
		this->m_oStimulator = new CoAdaptP300CStimulator(this->m_pStimulatorPropReader, m_pSequenceGenerator);
		m_bReplayMode=false;
	}

	if(m_pInterfacePropReader->getSpellingMode()!=CALIBRATION_MODE)
	{
		this->m_oEvidenceAccumulator = new CoAdaptP300CEvidenceAccumulator(m_pStimulatorPropReader,m_pSequenceGenerator);

	}
	else
	{
		this->m_oEvidenceAccumulator = NULL;
	}

	this->m_oStimulator->setEvidenceAccumulator(m_oEvidenceAccumulator);

	this->m_oStimulator->setCallBack(CoAdaptP300Visualiser::processCallback);	
	this->m_oStimulator->setWaitCallBack(CoAdaptP300Visualiser::processWaitCallback);
	this->m_oStimulator->setQuitEventCheck(CoAdaptP300Visualiser::areWeQuitting);


	//initialize the OpenGL context and the main container that is needed to draw everything on the screen by calling the drawAndSync function
	P300MainContainer::initializeGL(this->m_pInterfacePropReader->getFullScreen(),
						this->m_pInterfacePropReader->getWidth(),
						this->m_pInterfacePropReader->getHeight(),
						this->m_pInterfacePropReader->getMonitorIndex());
	//if fullscreen, size has been computed by initializeGL, otherwise, it returns the value defined in the file
	int width, height;
	GLFWwindow* window = P300MainContainer::getWindow();
	if(window!=NULL)
	{
		std::cout << "glfw window ok\n";
		glfwGetWindowSize(window, &width, &height);
	}
	else
	{
		//should not go there, if window is NULL something else will fail anyway
		width = this->m_pInterfacePropReader->getWidth();
		height = this->m_pInterfacePropReader->getHeight();
	}
	this->m_pMainContainer = new P300MainContainer(this->m_pInterfacePropReader, this->m_pScreenLayoutReader, width, height);
				
	//draw everything on the screen			
	this->m_pMainContainer->drawAndSync(this->m_pTagger,this->m_qEventQueue);		
}

CoAdaptP300Visualiser::~CoAdaptP300Visualiser()
{
	delete m_oStimulator;
	delete m_pTagger;
	delete m_pMainContainer; //should be delete before m_pScreenLayoutReader is deleted (needed to iterate over buttons) (why was it commented?)
	delete m_pInterfacePropReader; 
	delete m_pScreenLayoutReader; 
	delete m_pStimulatorPropReader;

	#ifdef OUTPUT_TIMING
	fclose(timingFile);
	fclose(timingFile3);
	#endif
	
	delete m_pSequenceWriter;
	delete m_pSequenceGenerator;
	if(m_oEvidenceAccumulator!=NULL)
		delete m_oEvidenceAccumulator;
}

void CoAdaptP300Visualiser::initializeOpenViBEKernel()
{
	CKernelLoader l_oKernelLoader;

	cout<<"[  INF  ] Created kernel loader, trying to load kernel module"<<"\n";
	CString m_sError;
	#if defined TARGET_OS_Windows
	if(!l_oKernelLoader.load(OpenViBE::Directories::getLibDir() + "/openvibe-kernel.dll", &m_sError))
	#elif defined TARGET_OS_Linux
	if(!l_oKernelLoader.load(OpenViBE::Directories::getLibDir() + "/libopenvibe-kernel.so", &m_sError))
	#endif
	{
			cout<<"[ FAILED ] Error loading kernel ("<<m_sError<<")"<<"\n";
	}
	else
	{
		cout<<"[  INF  ] Kernel module loaded, trying to get kernel descriptor"<<"\n";
		IKernelDesc* l_pKernelDesc=NULL;
		m_pKernelContext=NULL;
		l_oKernelLoader.initialize();
		l_oKernelLoader.getKernelDesc(l_pKernelDesc);
		if(!l_pKernelDesc)
		{
			cout<<"[ FAILED ] No kernel descriptor"<<"\n";
		}
		else
		{
			cout<<"[  INF  ] Got kernel descriptor, trying to create kernel"<<"\n";
			m_pKernelContext=l_pKernelDesc->createKernel("CoAdaptP300Stimulator", OpenViBE::Directories::getDataDir() + "/kernel/openvibe.conf");
			if(!m_pKernelContext)
			{
				cout<<"[ FAILED ] No kernel created by kernel descriptor"<<"\n";
			}
			else
			{
				OpenViBEToolkit::initialize(*m_pKernelContext);
			}
		}
	}
}

void CoAdaptP300Visualiser::processCallback(uint32 eventID)
{
	g_CoAdaptVisualiser->process(eventID);
}

void CoAdaptP300Visualiser::processWaitCallback(uint32 eventID)
{
	//std::cout << "CoAdaptP300Visualiser::processWaitCallback" << std::endl;
	if(eventID==0)
	{
		//process events and return immediately (do not wait)
		glfwPollEvents();
	}
	else
	{
		//wait for events
		glfwWaitEvents();
	}
}

boolean CoAdaptP300Visualiser::areWeQuitting(void)
{
	GLFWwindow* window = g_CoAdaptVisualiser->getMainContainer()->getWindow();
	return ( glfwWindowShouldClose(window)!=0 );
}

void CoAdaptP300Visualiser::process(uint32 eventID)
{
	m_bChanged = false;
	//std::vector<uint32>* l_lSymbolChangeList;
	uint32* l_lSymbolChangeList;					
	IMatrix* l_pLetterProbabilities;
	
	#ifdef OUTPUT_TIMING
	float64 l_f64TimeBefore;
	float64 l_f64TimeAfter;
	#endif
	
    switch(eventID)
	{
		case 0:
			m_pKernelContext->getLogManager() << LogLevel_Warning << "Something abnormal happened, probably during prediction nothing was received\n";
			break;
		case OVA_StimulationId_ExperimentStart:
			m_pMainContainer->getKeyboardHandler()->resetChildStates();
			//m_pMainContainer->setChanged(true);
			m_bChanged = true;
			m_qEventQueue.push(eventID);		
			break;
		case OVA_StimulationId_TrialStart:
			m_pTagger->write(eventID);
			break;
		case OVA_StimulationId_SegmentStart:
			break;
		case OVA_StimulationId_VisualStimulationStop:			
			m_pMainContainer->getKeyboardHandler()->resetChildStates();
			//m_pMainContainer->setChanged(true);
			m_bChanged = true;
			m_qEventQueue.push(eventID);
			if (m_pInterfacePropReader->isPhotoDiodeEnabled())//add that because segment end on flash
			{
				m_pMainContainer->DiodeAreaFlash(false);
			}
			break;
		case OVA_StimulationId_RestStop:
			#ifdef OUTPUT_TIMING
			l_f64TimeBefore = float64((System::Time::zgetTime()>>22)/1024.0);
			fprintf(timingFile3, "%f \n",l_f64TimeBefore);
			#endif
			
			m_oStimulator->generateNewSequence();
			
			#ifdef OUTPUT_TIMING
			l_f64TimeAfter = float64((System::Time::zgetTime()>>22)/1024.0);
			fprintf(timingFile3, "%f \n",l_f64TimeAfter);
			#endif
			
			m_bInRest = false;
			m_bInTarget = false;
			break;
		case OVA_StimulationId_ExperimentStop:
			m_pTagger->write(eventID);
			//exit when we receive experiment stop (useful when we want replay to stop automatically)
			glfwSetWindowShouldClose(this->getMainContainer()->getWindow(), GL_TRUE);
			break;
		case OVA_StimulationId_TrialStop:	
			m_pTagger->write(eventID);
			break;
		case OVA_StimulationId_SegmentStop:
			break;
		case OVA_StimulationId_RestStart:
			m_pTagger->write(eventID);
			m_bInRest = true;
			break;
		case OVA_StimulationId_TargetCue:
			m_bInFeedback = false;
			m_bInTarget = true;
			m_pTagger->write(eventID);
			break;
		case OVA_StimulationId_FeedbackCue:
			m_ui32FeedbackCueCounter++;
			m_bInFeedback = true;
			m_pTagger->write(eventID);
			break;
		case OVA_StimulationId_LetterColorFeedback:
			if (!m_bInRest)
			{
				l_pLetterProbabilities = m_oStimulator->getSharedMemoryReader()->readNextSymbolProbabilities();
				m_oStimulator->getSharedMemoryReader()->clearSymbolProbabilities();
				if (l_pLetterProbabilities!=NULL)
				{
					m_pMainContainer->getKeyboardHandler()->updateChildProbabilities(l_pLetterProbabilities->getBuffer());
					delete l_pLetterProbabilities;
				}
			}
			break;
		case OVA_StimulationId_FlashStop:
			//*TODO: should use the screen layout property reader to find out the background color
			if (m_pInterfacePropReader->isPhotoDiodeEnabled())
			{
				m_pMainContainer->DiodeAreaFlash(false);
			}
			//*/
		
			//reset the child states, but only the ones that have been flashed for the longest (in case of overlapping stimuli), 
			//the others should stay in their current state
			m_pMainContainer->getKeyboardHandler()->resetMostActiveChildStates();
			m_bChanged = true;
			m_qEventQueue.push(eventID);			
			break;
		case OVA_StimulationId_Flash:
			if (m_pInterfacePropReader->isPhotoDiodeEnabled())
			{
				m_pMainContainer->DiodeAreaFlash(true);	
			}
			
			m_qEventQueue.push(eventID);
			
			//get the next flash group which is a vector, the size of the number of symbols on the keyboard, with one or zero to indicate
			//whether it is flashed or not
			//m_pKernelContext->getLogManager() << LogLevel_Info << "Flash " << eventID << " getting group \n";
			*m_vCurrentFlashGroup = *(m_oStimulator->getNextFlashGroup());
			l_lSymbolChangeList = m_vCurrentFlashGroup->data();
			changeStates(l_lSymbolChangeList,FLASH);
			m_pMainContainer->getKeyboardHandler()->updateChildStates(l_lSymbolChangeList);	
					
			m_bChanged = true;
			
			//flagging sequence stimuli as either target or non target
			if (m_bChanged && m_pInterfacePropReader->getSpellingMode()!=FREE_MODE)
			{
				if(l_lSymbolChangeList[m_bTargetId]==1)	
					m_qEventQueue.push(OVA_StimulationId_Target);
				else
					m_qEventQueue.push(OVA_StimulationId_NonTarget);
			}		
			break;
		default:
			eventID--;
			//the first recorded files index letter from 0
			if(m_pInterfacePropReader->getStimulatorMode()==CString("Replay"))
			{
				eventID++;
			}
			//m_pKernelContext->getLogManager() << LogLevel_Info << "Default (label) received, eventID " << eventID << "\n";
			m_qEventQueue.push(eventID+1);
			l_lSymbolChangeList = new uint32[m_pScreenLayoutReader->getNumberOfKeys()]();
			//we are in the period where feedback is presented
			if (m_bInFeedback)
			{
				m_ui32FeedbackResultCounter++;
				//std::cout << "FeedbackResultCounter " << m_ui32FeedbackResultCounter << "\n";
				m_pKernelContext->getLogManager() << LogLevel_Info << "Feedback received, eventID " << eventID << "\n";
				//FREE MODE
				if(m_pInterfacePropReader->getSpellingMode()==FREE_MODE)
				{
					l_lSymbolChangeList[eventID] = 1;
					
					//feedback is presented in the centre of the screen
					if(m_pInterfacePropReader->getCentralFeedbackFreeMode())
						changeStates(l_lSymbolChangeList,CENTRAL_FEEDBACK_CORRECT,NOFLASH);	
					//letter in the grid is highlighted as feedback
					else
						changeStates(l_lSymbolChangeList,NONCENTRAL_FEEDBACK_CORRECT,NOFLASH);
					
					m_bChanged = true;
				}
				//COPY MODE: here we have to compare with the target letter to either show the prediction in red or green
				else if(m_pInterfacePropReader->getSpellingMode()==COPY_MODE)
				{	
					//feedback is presented in the centre of the screen
					if(m_pInterfacePropReader->getCentralFeedbackCopyMode())	
					{
						l_lSymbolChangeList[eventID] = 1;
																				
						if(eventID==m_bTargetId)
							changeStates(l_lSymbolChangeList,CENTRAL_FEEDBACK_CORRECT,NOFLASH);							
						else
							changeStates(l_lSymbolChangeList,CENTRAL_FEEDBACK_WRONG,NOFLASH);							
					}
					//letter in the grid is highlighted as feedback
					else
					{
						l_lSymbolChangeList[m_bTargetId] = 1;
													
						if(eventID==m_bTargetId)
							changeStates(l_lSymbolChangeList,NONCENTRAL_FEEDBACK_CORRECT,NOFLASH);	
						else
						{
							changeStates(l_lSymbolChangeList,NONCENTRAL_FEEDBACK_WRONG,NOFLASH);
							l_lSymbolChangeList[eventID]=NONCENTRAL_FEEDBACK_WRONG_SELECTED;	
						}		
					}
					m_bChanged = true;
				}
				m_bInFeedback = false;
			}
			//we are in the period where the target is displayed
			else if(m_bInTarget && m_pInterfacePropReader->getSpellingMode()!=FREE_MODE) 
			{
				m_pKernelContext->getLogManager() << LogLevel_Info << "Target displayed, eventID " << eventID << "\n";
				l_lSymbolChangeList[eventID] = 1;	
				changeStates(l_lSymbolChangeList,TARGET,NOFLASH);
				
				m_bTargetId = eventID;
				m_bChanged = true;
				m_bInTarget = false;
			}
			
			//this tells the keyboard handler the state of the symbols (i.e. flashed, not flashed, feedback...)
			m_pMainContainer->getKeyboardHandler()->updateChildStates(l_lSymbolChangeList);
			delete l_lSymbolChangeList;
			
			break;
	}
	
	if (m_bChanged)
	{
		#ifdef OUTPUT_TIMING
            l_f64TimeBefore = float64((System::Time::zgetTime()>>22)/1024.0);
			//fprintf(timingFile, "%f \n",l_f64TimeBefore);
			fprintf(timingFile, "%f;",l_f64TimeBefore);
		#endif

		//this will notify all listeners/observers of the keyboard buttons that they should be updated	
		m_pMainContainer->getKeyboardHandler()->updateChildProperties();

		#ifdef OUTPUT_TIMING
            l_f64TimeAfter = float64((System::Time::zgetTime()>>22)/1024.0);
			fprintf(timingFile, "%f;\n",l_f64TimeAfter);
		#endif
		
		//send a stimulus to openvibe in order to train the XDAWN and the classifier 
		//(this is used in the subject-independent classifier scenario to initialize the subject-specific classifier)
		if (m_ui32FeedbackResultCounter!= 0 && 
			m_ui32PreviousFeedbackResultCounter!=m_ui32FeedbackResultCounter && 
			m_ui32FeedbackResultCounter%10 == 0) //TODO: should  be a configurable parameter
		{
			m_ui32PreviousFeedbackResultCounter = m_ui32FeedbackResultCounter;
			m_qEventQueue.push(OVA_StimulationId_UpdateModel);
			m_pKernelContext->getLogManager() << LogLevel_Info << "Sending stimulus to OpenViBE to update model\n";
		}
		//draw all the changes on screen and tag the event
        m_pMainContainer->drawAndSync(m_pTagger,m_qEventQueue);	
	}
}

void CoAdaptP300Visualiser::start()
{
	g_CoAdaptVisualiser->m_oStimulator->run();
}

void CoAdaptP300Visualiser::changeStates(uint32* states, VisualState ifState, VisualState elseState)
{
	for(uint32 it=0; it<m_pScreenLayoutReader->getNumberOfKeys() ; it++)
	{
		if (states[it]==1)
			states[it] = ifState;
		else
			states[it] = elseState;
	}	
}

static void error_callback(int error, const char* description)
{
	std::cout << "GLFW " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::cout << "key pressed " << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		std::cout << "ESCAPE key pressed " << std::endl;
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		std::cout << "START key pressed " << std::endl;
	}
}
#endif

/**
MAIN: press 's' to start the stimulator
*/
int main (int argc, char *argv[])
{
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
	glfwSetErrorCallback(error_callback);

	//create the main visualiser object
	g_CoAdaptVisualiser = new CoAdaptP300Visualiser();

	GLFWwindow* window = g_CoAdaptVisualiser->getMainContainer()->getWindow();
	glfwSetKeyCallback(window, key_callback);

	//listen for an key event. If 's' is pressed the stimulator is started.
	if(!g_CoAdaptVisualiser->getReplayMode())
	{
		std::cout << "waitingfor s key" << std::endl;
		boolean l_bEventReceived = false;
		while (!l_bEventReceived && glfwGetKey(window, GLFW_KEY_S)!=GLFW_PRESS)//SDL_WaitEvent(&event))
		{
			glfwWaitEvents();
			if(glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS)
			{
				l_bEventReceived = true;
				g_CoAdaptVisualiser->start();
			}

			System::Time::sleep(10);
		}
	}
	else
	{
		//in replay mode we do not wait for the user to press 's
		g_CoAdaptVisualiser->start();
	}
	//clean up
	delete g_CoAdaptVisualiser;

	glfwTerminate();
#else
	std::cout << "You do not have the required libraries for the CoAdapt stimulator " << std::endl;
#endif
	return 0;
}
