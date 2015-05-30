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

#include "ovexP300IEvidenceAccumulator.h"

#include "../ovexP300SharedMemoryReader.h"
#include "../properties/ovexP300StimulatorPropertyReader.h"
#include "../sequence/ovexP300SequenceGenerator.h"

namespace OpenViBEApplications
{		


		
		/**
		 * The EvidenceAccumulator class is the main loop of the program. Based on stimulated time steps it will go from one state to another
		 * (e.g. if the duration of flash is over then it will go to the noflash state). Each time it changes to another state it notifies the 
		 * main ExternalP300Visualiser class by means of a stimulation id as defined in ova_defines.h
		 */
		class ExternalP300EvidenceAccumulator : public CoAdaptP300IEvidenceAccumulator
		{
			public:

				/**
				 * The main loop of the program
				 */
				virtual void run();
				
				/**
				 * Constructor that will create an ExternalP300SharedMemoryReader object for reading the predictions and probabilities of the letters
				 * as computed by the openvibe-designer TODO the EvidenceAccumulator should not handle reading from shared memory,
				 * a separate thread should do that and then notify the the EvidenceAccumulator of that event
				 * @param propertyObject the object containing the properties for the EvidenceAccumulator such as flash duration, interflash duration, intertrial...
				 * @param l_pSequenceGenerator the sequence generator that defines which letters are flashed at one single point in time (does that for the whole trial)
				 */
				ExternalP300EvidenceAccumulator(P300StimulatorPropertyReader* propertyObject, P300SequenceGenerator* l_pSequenceGenerator);
				
				~ExternalP300EvidenceAccumulator();
				
				/**
				 * At the beginning of the the next trial, generate the whole sequence of letters that have to be flashed in the trial
				 */ 
				virtual void generateNewSequence() { m_pSequenceGenerator->generateSequence(); }
				
				/**
				 * @return return vector of zeros and ones defining which letters will be flashed next
				 */
				virtual std::vector<OpenViBE::uint32>* getNextFlashGroup() { return m_pSequenceGenerator->getNextFlashGroup(); }
				
				/**
				 * @return The shared memory reader that is created during construction of the EvidenceAccumulator
				 */
				virtual CoAdaptP300SharedMemoryReader* getSharedMemoryReader() { return &m_oSharedMemoryReader; }
				
			private:


			protected:

				OpenViBE::CString m_sSharedMemoryName;
				CoAdaptP300SharedMemoryReader m_oSharedMemoryReader;
				P300SequenceGenerator* m_pSequenceGenerator;
				P300StimulatorPropertyReader* m_pPropertyObject;

			private:
				OpenViBE::IMatrix* m_pAccumulatedEvidence;
				OpenViBE::IMatrix* m_pNormalizedAccumulatedEvidence;
				OpenViBE::CIdentifier m_oEvidenceAlgorithm;
		};

};

#endif
