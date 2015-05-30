#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <map>
#include <queue>
#include <vector>

#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "ovexP300IStimulator.h"

#include "../ovexP300SharedMemoryReader.h"
#include "../properties/ovexP300StimulatorPropertyReader.h"
#include "../sequence/ovexP300SequenceGenerator.h"

namespace OpenViBEApplications
{		


		
		/**
		 * @author : Dieter Devlaminck / Loic Mahe
		 * The stimulator class is the main loop of the program. Based on stimulated time steps it will go from one state to another
		 * (e.g. if the duration of flash is over then it will go to the noflash state). Each time it changes to another state it notifies the 
		 * main CoAdaptP300Visualiser class by means of a stimulation id as defined in ova_defines.h
		 */
		class CoAdaptP300CStimulator : public CoAdaptP300IStimulator
		{
			public:

				/**
				 * The main loop of the program
				 */
				virtual void run();
				
				/**
				 * Constructor that will create an CoAdaptP300SharedMemoryReader object for reading the predictions and probabilities of the letters
				 * as computed by the openvibe-designer TODO the stimulator should not handle reading from shared memory, 
				 * a separate thread should do that and then notify the the stimulator of that event
				 * @param propertyObject the object containing the properties for the stimulator such as flash duration, interflash duration, intertrial...
				 * @param l_pSequenceGenerator the sequence generator that defines which letters are flashed at one single point in time (does that for the whole trial)
				 */
				CoAdaptP300CStimulator(P300StimulatorPropertyReader* propertyObject, P300SequenceGenerator* l_pSequenceGenerator);
				
				~CoAdaptP300CStimulator();
				
				/**
				 * At the beginning of the the next trial, generate the whole sequence of letters that have to be flashed in the trial
				 */ 
				virtual void generateNewSequence() { m_pSequenceGenerator->generateSequence(); }
				
				/**
				 * @return return vector of zeros and ones defining which letters will be flashed next
				 */
				virtual std::vector<OpenViBE::uint32>* getNextFlashGroup() { return m_pSequenceGenerator->getNextFlashGroup(); }
				
				/**
				 * @return The shared memory reader that is created during construction of the stimulator
				 */
				virtual CoAdaptP300SharedMemoryReader* getSharedMemoryReader() { return &m_oSharedMemoryReader; }
				
			private:
				/**
				 * If you stop early then it will adjust the variables such as m_ui64TrialStartTime so that the next
				 * trial can begin
				 */
				virtual void adjustForNextTrial(OpenViBE::uint64 currentTime);

			protected:
				OpenViBE::uint64 m_ui64StartStimulation;
				OpenViBE::uint32 m_ui32RepetitionCountInTrial;
				OpenViBE::uint32 m_ui32MinRepetitions;
				OpenViBE::uint32 m_ui32TrialCount;
				OpenViBE::uint64 m_ui64FlashDuration;
				OpenViBE::uint64 m_ui64InterRepetitionDuration;
				OpenViBE::uint64 m_ui64InterTrialDuration;
				OpenViBE::uint64 m_ui64InterStimulusOnset;
				OpenViBE::uint64 m_ui64TimeToNextFlash;
				OpenViBE::uint64 m_ui64TimeToNextFlashStop;
				OpenViBE::CString m_sTargetWord;

				OpenViBE::CString m_sSharedMemoryName;
				CoAdaptP300SharedMemoryReader m_oSharedMemoryReader;
				P300SequenceGenerator* m_pSequenceGenerator;
				P300StimulatorPropertyReader* m_pPropertyObject;

			private:

				OpenViBE::uint64 m_ui64LastTime;
				OpenViBE::boolean m_bStopReceived;

				OpenViBE::uint32 m_ui32LastState;
				OpenViBE::uint64 m_ui64TrialStartTime;
				OpenViBE::uint64 m_ui64NextTargetStartTime;
				OpenViBE::uint64 m_ui64NextTargetEndTime;
				OpenViBE::uint64 m_ui64NextFeedbackStartTime;
				OpenViBE::uint64 m_ui64NextFeedbackEndTime;

				OpenViBE::uint32 m_ui32FlashCountInRepetition;
				OpenViBE::uint64 m_ui64RepetitionDuration;
				OpenViBE::uint64 m_ui64TrialDuration;
				OpenViBE::uint32 m_ui32TrialIndex;



				std::map < OpenViBE::uint64, OpenViBE::uint64 > m_vRow;
				std::map < OpenViBE::uint64, OpenViBE::uint64 > m_vColumn;

				OpenViBE::uint64 m_ui64Prediction;
				std::queue< OpenViBE::uint64 >* m_oTargetStimuli;
				
				//SDL_Event m_eKeyEvent;

				#ifdef OUTPUT_TIMING
                FILE* timingFile;
				#endif
				
				OpenViBE::uint64 m_ui64StimulatedCycleTime;
				OpenViBE::uint64 m_ui64RealCycleTime;
		};

};
#endif//TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#endif
