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
		 * The stimulator class is the main loop of the program. This NULL stimulator does nothing exept transmit stimulations from
		 * the openvibe data (via shared memory) to the visualizer. It is used when replaying data
		 */
		class CoAdaptP300CNULLStimulator : public CoAdaptP300IStimulator
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
				CoAdaptP300CNULLStimulator(P300StimulatorPropertyReader* propertyObject, P300SequenceGenerator* l_pSequenceGenerator);
				
				~CoAdaptP300CNULLStimulator();

				/**
				 * At the beginning of the the next trial, generate the whole sequence of letters that have to be flashed in the trial
				 */ 
				virtual void generateNewSequence() {m_pSequenceGenerator->generateSequence(); }
				
				/**
				 * @return return vector of zeros and ones defining which letters will be flashed next
				 for the NULL stimulator (REPLAY MODE) this information comes from the csv file associated to the data we are replaying
				 */
				virtual std::vector<OpenViBE::uint32>* getNextFlashGroup()
				{					return m_pSequenceGenerator->getNextFlashGroup();}
				
				/**
				 * @return The shared memory reader that is created during construction of the stimulator
				 */
				//virtual CoAdaptP300SharedMemoryReader* getSharedMemoryReader() { return &m_oSharedMemoryReader; }
				
			protected:
				//OpenViBE::CString m_sSharedMemoryName;
				//xternalP300SharedMemoryReader m_oSharedMemoryReader;
				P300SequenceGenerator* m_pSequenceGenerator;
				P300StimulatorPropertyReader* m_pPropertyObject;

			private:
				#ifdef OUTPUT_TIMING
                FILE* timingFile;
				#endif
				
				OpenViBE::uint64 m_ui64StimulatedCycleTime;
				OpenViBE::uint64 m_ui64RealCycleTime;
				OpenViBE::uint32 m_ui32TrialCount;
				OpenViBE::uint32 m_ui32TrialIndex;

		};

};
#endif

#endif
