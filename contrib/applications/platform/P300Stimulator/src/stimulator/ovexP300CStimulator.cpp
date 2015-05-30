#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300CStimulator.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "../ova_defines.h"
#include <system/ovCTime.h>

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

//struct timeval currentLTime;

//#define time2ms(x,y) ((x) * 1000 + y/1000.0) + 0.5

CoAdaptP300CStimulator::CoAdaptP300CStimulator(P300StimulatorPropertyReader* propertyObject, P300SequenceGenerator* l_pSequenceGenerator)
				: m_pPropertyObject(propertyObject)
{
	m_pSequenceGenerator = l_pSequenceGenerator;
	
	m_ui32RepetitionCountInTrial = m_pSequenceGenerator->getNumberOfRepetitions();
	m_ui32MinRepetitions = propertyObject->getMinNumberOfRepetitions();
	m_ui32TrialCount = propertyObject->getNumberOfTrials();
	m_ui64FlashDuration = propertyObject->getFlashDuration();
	m_ui64InterStimulusOnset = propertyObject->getInterStimulusOnset();
	m_ui64InterRepetitionDuration = propertyObject->getInterRepetitionDelay();
	m_ui64InterTrialDuration = propertyObject->getInterTrialDuration();
	m_oTargetStimuli = propertyObject->getTargetStimuli();
	m_sSharedMemoryName = propertyObject->getSharedMemoryName();
	
	m_ui64LastTime=0;
	m_bStopReceived = false;

	m_ui32LastState=State_None;
	adjustForNextTrial(0);

	m_ui32FlashCountInRepetition=m_pSequenceGenerator->getNumberOfGroups();
	m_ui64RepetitionDuration = (m_ui32FlashCountInRepetition-1)*m_ui64InterStimulusOnset+m_ui64FlashDuration;
	m_ui64TrialDuration=m_ui32RepetitionCountInTrial*(m_ui64RepetitionDuration+m_ui64InterRepetitionDuration);
	m_ui32TrialIndex=0;					

	m_ui64Prediction = 0;				
	
	#ifdef OUTPUT_TIMING
	timingFile = fopen(OpenViBE::Directories::getUserDataDir() + "/xP300-stimulator_round_timing.txt","w");
	#endif
	
	m_ui64RealCycleTime = 0;
	m_ui64StimulatedCycleTime = 0;
}

CoAdaptP300CStimulator::~CoAdaptP300CStimulator()
{
	#ifdef OUTPUT_TIMING
	fclose(timingFile);
	#endif
}

void CoAdaptP300CStimulator::adjustForNextTrial(uint64 currentTime)
{
	m_ui64TrialStartTime=currentTime+m_ui64InterTrialDuration;
	uint64 l_ui64InterTrialPartition = m_ui64InterTrialDuration >> 3;
	m_ui64NextFeedbackStartTime = m_ui64TrialStartTime-m_ui64InterTrialDuration+(l_ui64InterTrialPartition<<1);
	m_ui64NextFeedbackEndTime = m_ui64NextFeedbackStartTime+(l_ui64InterTrialPartition<<1);
	m_ui64NextTargetStartTime = m_ui64NextFeedbackEndTime+l_ui64InterTrialPartition;
	m_ui64NextTargetEndTime = m_ui64NextTargetStartTime+(l_ui64InterTrialPartition<<1);
	
	m_ui64TimeToNextFlash = 0;
	m_ui64TimeToNextFlashStop = m_ui64TimeToNextFlash + m_ui64FlashDuration;

	m_ui32TrialIndex++;	
}

void CoAdaptP300CStimulator::run()
{
	uint32 l_ui32State=State_NoFlash;

	if (m_ui32TrialCount==0)
		m_ui32TrialCount = UINT_MAX-1;

	uint32 l_ui32StimulatorFrequency = 250; //TODO should be a configurable parameter
	uint64 l_ui64TimeStep = static_cast<uint64>(ITimeArithmetics::sampleCountToTime(l_ui32StimulatorFrequency, 1LL));
	uint64 l_ui64CurrentTime = 0;

	//uint64 l_ui64MyInterFlash = 0;
	//uint64 l_ui64MyLastFlash = 0;
	uint64 l_ui64LastCurrentTime = 0;
	
	while (m_ui32TrialIndex<=m_ui32TrialCount)
	{
		uint64 l_ui64TimeBefore = System::Time::zgetTime();
		#ifdef OUTPUT_TIMING
			fprintf(timingFile, "%f \n",float64(ITimeArithmetics::timeToSeconds(System::Time::zgetTime())));
		#endif
		
		//m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Info << "real " << time64(m_ui64RealCycleTime) << " simulated " << time64(l_ui64CurrentTime) << "\n";
		//very often one cycle of this loop does not take much time, so we just put the program to sleep until the next time step
		//*
		if (m_ui64RealCycleTime<l_ui64CurrentTime)
		{
			uint32 l_ui32WaitFor = static_cast<uint32>(std::ceil(1000.0*ITimeArithmetics::timeToSeconds(l_ui64CurrentTime-m_ui64RealCycleTime+l_ui64TimeStep)));
			System::Time::sleep(l_ui32WaitFor);
		}
		else if(m_ui64RealCycleTime-l_ui64CurrentTime>l_ui64TimeStep)//if the simulated time is behind real time, we make up for lost time by skipping the necessary amount of cycles
		{
			uint64 l_ui64NumberOfCycles = ceil((double)(m_ui64RealCycleTime-l_ui64CurrentTime)/l_ui64TimeStep);
			l_ui64CurrentTime+=l_ui64NumberOfCycles*l_ui64TimeStep;

		}
		//*/
		uint64 l_ui64Prediction = 0;

		if(m_oEvidenceAcc!=NULL)
		{
			m_oEvidenceAcc->update();//update with data from the scenario classifier
			m_oEvidenceAcc->getPrediction();
			l_ui64Prediction = m_oEvidenceAcc->getPrediction();
		}
			
		if (l_ui64Prediction!=0 && l_ui32State!=State_TrialRest && l_ui32State!=State_Feedback && l_ui32State!=State_Target ) //induce early stopping
		{
			this->adjustForNextTrial(l_ui64CurrentTime);
			m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Info << "Prediction received that is different from zero, induce early stopping...\n";
			m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Info << "Prediction received " << l_ui64Prediction << "\n";
		}

		//rest, target or feedback
		if(l_ui64CurrentTime<m_ui64TrialStartTime)
		{
			l_ui32State=State_TrialRest;
			if ((l_ui64CurrentTime>=m_ui64NextFeedbackStartTime) && (l_ui64CurrentTime<=m_ui64NextFeedbackEndTime))
			{
				l_ui32State=State_Feedback;
				if (m_ui32LastState==State_Feedback && l_ui64Prediction!=0)
				{
					m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Info << "Forced prediction from OpenViBE " << l_ui64Prediction << "\n";// << (uint32)currentLTime.tv_sec << "," << (uint32)currentLTime.tv_usec << "\n";
					m_ui64Prediction=l_ui64Prediction;
					l_ui32State=State_TrialRest;
					if(m_oEvidenceAcc!=NULL)
					{
						m_oEvidenceAcc->flushEvidence();//flush
					}
				}
			}
			//showing targets on screen
			if (l_ui64CurrentTime>=m_ui64NextTargetStartTime && l_ui64CurrentTime<=m_ui64NextTargetEndTime)
				l_ui32State=State_Target;
		}
		else if (m_ui32TrialIndex!=m_ui32TrialCount)
		{
			uint64 l_ui64CurrentTimeInTrial     =l_ui64CurrentTime-m_ui64TrialStartTime;
			uint64 l_ui64CurrentTimeInRepetition=l_ui64CurrentTimeInTrial%(m_ui64RepetitionDuration+m_ui64InterRepetitionDuration);

			if(l_ui64CurrentTimeInTrial >= m_ui64TrialDuration)
			{
				if(m_ui32TrialCount==0 || m_ui32TrialIndex<m_ui32TrialCount)
				{
					this->adjustForNextTrial(l_ui64CurrentTime);
					l_ui32State=State_TrialRest;
				}
				else
					m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Warning << "This should not occur\n";
			}
			else
			{
				if(l_ui64CurrentTimeInRepetition >= m_ui64RepetitionDuration)
				{
					l_ui32State=State_RepetitionRest;
					m_ui64TimeToNextFlash = 0;
					m_ui64TimeToNextFlashStop = m_ui64TimeToNextFlash + m_ui64FlashDuration;
					//m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Warning << "Rep rest\n";
				}
				else
				{
					if (l_ui64CurrentTimeInRepetition<m_ui64TimeToNextFlash)
						l_ui32State = State_InterFlash;
					if (l_ui64CurrentTimeInRepetition>=m_ui64TimeToNextFlash && m_ui32LastState!=State_Flash && 
					    (m_ui64TimeToNextFlash<=m_ui64RepetitionDuration-m_ui64FlashDuration))
					{
						l_ui32State = State_Flash;
						m_ui64TimeToNextFlash += m_ui64InterStimulusOnset;
						// l_ui64MyLastFlash = System::Time::zgetTime();
						m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Debug << "Flash at " << time64(l_ui64CurrentTime) << " diff is " << time64(l_ui64CurrentTime-l_ui64LastCurrentTime) << "\n";// next flash at " << time64(m_ui64TimeToNextFlash) << "\n";
						// l_ui64MyInterFlash = l_ui64MyLastFlash;
						l_ui64LastCurrentTime = l_ui64CurrentTime;
					}
					if (l_ui64CurrentTimeInRepetition>=m_ui64TimeToNextFlashStop && m_ui32LastState!=State_NoFlash)
					{
						l_ui32State = State_NoFlash;
						m_ui64TimeToNextFlashStop += m_ui64InterStimulusOnset;
						m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Debug << "Flash stop at " << l_ui64CurrentTimeInRepetition << " next flash stop at " << m_ui64TimeToNextFlashStop << "\n";
					}					
				}
			}
		}
		else
		{
			m_ui32TrialIndex++; //to exit the main while loop
			l_ui32State=State_None;
		}

		if(l_ui32State!=m_ui32LastState)
		{
			//why reset? unnecessary
			//m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Info << "reset times\n";
			//m_ui64RealCycleTime = 0;
			//m_ui64StimulatedCycleTime = 0;
			m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Debug << "Switch states from " << m_ui32LastState << " to " << l_ui32State << "\n";

			switch(m_ui32LastState)
			{
				case State_Flash:
					break;

				case State_NoFlash:
					break;

				case State_RepetitionRest:
					/*if(l_ui32State!=State_TrialRest && l_ui32State!=State_None)
						m_funcVisualiserCallback(OVA_StimulationId_SegmentStart);*/
					break;

				case State_TrialRest:
					if (l_ui32State!=State_Target && l_ui32State!=State_Feedback)
					{
						m_oFuncVisualiserCallback(OVA_StimulationId_RestStop);
						if (l_ui32State!=State_None)
						{
							m_oFuncVisualiserCallback(OVA_StimulationId_TrialStart);
							//m_funcVisualiserCallback(OVA_StimulationId_SegmentStart);
						}
					}

					break;

				case State_Target:
					m_ui64Prediction = 0;
					break;

				case State_Feedback:
					break;

				case State_None:
					m_oFuncVisualiserCallback(OVA_StimulationId_ExperimentStart);
					break;

				default:
					break;
			}
			uint32 targetStimulus = 0;
			switch(l_ui32State)
			{
				case State_Flash:
					m_oFuncVisualiserCallback(OVA_StimulationId_Flash);
					break;

				case State_NoFlash:
					m_oFuncVisualiserCallback(OVA_StimulationId_FlashStop);
					break;

				case State_RepetitionRest:
					m_oFuncVisualiserCallback(OVA_StimulationId_VisualStimulationStop);
					break;

				case State_TrialRest:
					if (m_ui32LastState==State_Target || m_ui32LastState==State_Feedback)
					{
						m_oFuncVisualiserCallback(OVA_StimulationId_VisualStimulationStop);
					}
					if(m_ui32LastState!=State_None && m_ui32LastState!=State_Target && m_ui32LastState!=State_Feedback)
					{
						m_oFuncVisualiserCallback(OVA_StimulationId_TrialStop);
						m_oFuncVisualiserCallback(OVA_StimulationId_RestStart);
					}
					break;

				case State_Target:
					if (m_oTargetStimuli->size()!=0)
					{
						targetStimulus = static_cast<uint32>(m_oTargetStimuli->front()); //TODO: we should type all stimuli consistently as either uint64 or uint32
						m_oTargetStimuli->pop();
						m_oFuncVisualiserCallback(OVA_StimulationId_TargetCue);
						m_oFuncVisualiserCallback(targetStimulus);
					}
					break;

				case State_Feedback:
					if(m_ui64Prediction==0)
					{		
						m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Info << "Stimulator forces feedback in OpenViBE at time\n ";// << (uint32)currentLTime.tv_sec << "," << (uint32)currentLTime.tv_usec << "\n";
						m_oFuncVisualiserCallback(OVA_StimulationId_FeedbackCue);
					}
					else
					{		
						m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Info << "Received feedback: \n";// << m_ui64Prediction << " at time " << (uint32)currentLTime.tv_sec << "," << (uint32)currentLTime.tv_usec <<  "\n";
						m_oFuncVisualiserCallback(static_cast<uint32>(m_ui64Prediction));//TODO: we should type all stimuli consistently as either uint64 or uint32
						m_ui64Prediction=0;
					}
					break;

				case State_None:
					if(m_ui32LastState!=State_RepetitionRest && m_ui32LastState!=State_TrialRest)
						m_pPropertyObject->getKernelContext()->getLogManager() << LogLevel_Warning << "We should not reach this state\n";
					m_oFuncVisualiserCallback(OVA_StimulationId_TrialStop);
					break;

				default:
					break;
			}

			m_ui32LastState=l_ui32State;
		}
		
		l_ui64CurrentTime += l_ui64TimeStep;
		m_ui64StimulatedCycleTime += l_ui64TimeStep;

		m_oFuncVisualiserWaitCallback(0);
		if(checkForQuitEvent())
			m_ui32TrialIndex = UINT_MAX;
		
		uint64 l_ui64TimeDifference = System::Time::zgetTime()-l_ui64TimeBefore;
		m_ui64RealCycleTime += l_ui64TimeDifference;


		#ifdef OUTPUT_TIMING
		fprintf(timingFile, "%f \n",float64(ITimeArithmetics::timeToSeconds(System::Time::zgetTime())));
		#endif
	}
	
	//in case it is not stopped in the middle of the stimulation process we want to wait on an event before quitting the application
	if (m_ui32TrialIndex != UINT_MAX)
	{
		m_oFuncVisualiserWaitCallback(1);
		while (!checkForQuitEvent())
		{
			m_oFuncVisualiserWaitCallback(1);
			System::Time::sleep(50);
		}
	}
	m_oFuncVisualiserCallback(OVA_StimulationId_ExperimentStop);
}
#endif

#endif
