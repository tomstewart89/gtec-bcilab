#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#ifndef __IStimulator_H__
#define __IStimulator_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <map>
#include <queue>
#include <vector>

#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "../ovexP300SharedMemoryReader.h"
#include "../properties/ovexP300StimulatorPropertyReader.h"
#include "../sequence/ovexP300SequenceGenerator.h"

#include "../evidence-accumulation/ovexP300IEvidenceAccumulator.h"

namespace OpenViBEApplications
{		
		typedef void (*Callback2Visualiser)(OpenViBE::uint32);	
		typedef OpenViBE::boolean (*getFromVisualiser)(void);

		/**
		 * States of the stimulator (depends on the time within a trial)
		 */
		enum
		{
			State_None,
			State_Flash,
			State_NoFlash,
			State_RepetitionRest,
			State_TrialRest,
			State_Target,
			State_Feedback,
			State_InterFlash,
		};		
		
		/**
		 * @author : Dieter Devlaminck / Loic Mahe
		 * The stimulator class is the main loop of the program. Based on stimulated time steps it will go from one state to another
		 * (e.g. if the duration of flash is over then it will go to the noflash state). Each time it changes to another state it notifies the 
		 * main CoAdaptP300Visualiser class by means of a stimulation id as defined in ova_defines.h
		 */
		class CoAdaptP300IStimulator
		{
			public:

				/**
				 * The main loop of the program
				 */
				virtual void run()=0;
				
				/**
				 * Constructor that will create an CoAdaptP300SharedMemoryReader object for reading the predictions and probabilities of the letters
				 * as computed by the openvibe-designer TODO the stimulator should not handle reading from shared memory, 
				 * a separate thread should do that and then notify the the stimulator of that event
				 * @param propertyObject the object containing the properties for the stimulator such as flash duration, interflash duration, intertrial...
				 * @param l_pSequenceGenerator the sequence generator that defines which letters are flashed at one single point in time (does that for the whole trial)
				 */
				//CoAdaptP300IStimulator(P300StimulatorPropertyReader* propertyObject, P300SequenceGenerator* l_pSequenceGenerator)=0;
				
				CoAdaptP300IStimulator(){};

				virtual ~CoAdaptP300IStimulator(){};

				/**
				 * The callback that the stimulator will call to notify the CoAdaptP300Visualiser that the state has changed and the display should be updated
				 */
				virtual void setCallBack( Callback2Visualiser callback) {m_oFuncVisualiserCallback = callback;}


				virtual void setWaitCallBack( Callback2Visualiser callback) {m_oFuncVisualiserWaitCallback = callback;}
				virtual void setQuitEventCheck( getFromVisualiser callback) {m_oQuitEvent = callback;}

				virtual void setEvidenceAccumulator(CoAdaptP300IEvidenceAccumulator* evAcc){m_oEvidenceAcc = evAcc;}
				
				/**
				 * At the beginning of the the next trial, generate the whole sequence of letters that have to be flashed in the trial
				 */ 
				virtual void generateNewSequence()=0;
				
				/**
				 * @return return vector of zeros and ones defining which letters will be flashed next
				 */
				virtual std::vector<OpenViBE::uint32>* getNextFlashGroup()=0;
				
				/**
				 * @return The shared memory reader that is created during construction of the stimulator
				 */
				virtual CoAdaptP300SharedMemoryReader* getSharedMemoryReader() { return &m_oSharedMemoryReader; }

		protected:
			/**
			 * If you stop early then it will adjust the variables such as m_ui64TrialStartTime so that the next
			 * trial can begin
			 */
			//virtual void adjustForNextTrial(OpenViBE::uint64 currentTime);

			/**
			 * Checks if the escape button is pressed
			 */
			virtual OpenViBE::boolean checkForQuitEvent()
			{
				OpenViBE::boolean l_bQuitEventReceived = false;
				l_bQuitEventReceived = m_oQuitEvent();
				return l_bQuitEventReceived;
			}

			getFromVisualiser m_oQuitEvent;
			Callback2Visualiser m_oFuncVisualiserCallback;
			Callback2Visualiser m_oFuncVisualiserWaitCallback;
			CoAdaptP300SharedMemoryReader m_oSharedMemoryReader;

			CoAdaptP300IEvidenceAccumulator* m_oEvidenceAcc;
				
		};

};
#endif
#endif

#endif
