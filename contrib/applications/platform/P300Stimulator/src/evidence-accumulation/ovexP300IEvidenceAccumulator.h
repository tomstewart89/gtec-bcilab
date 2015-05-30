#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __IEvidenceAccumulator_H__
#define __IEvidenceAccumulator_H__

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

#include "../ovexP300SharedMemoryReader.h"
#include "../properties/ovexP300StimulatorPropertyReader.h"
#include "../sequence/ovexP300SequenceGenerator.h"


namespace OpenViBEApplications
{		

		/**
		  * The EvidenceAccumulator class takes evidence from OpenVibe (via shared memory) and determines which symbol is selected.
		 *	Each time data is received from the openvibe scenario, the probabilities are updated
		 *
		 */
		class CoAdaptP300IEvidenceAccumulator
		{
			public:
				
				/**
				 * @param propertyObject the object containing the properties for the EvidenceAccumulator such as flash duration, interflash duration, intertrial...
				 * @param l_pSequenceGenerator the sequence generator that defines which letters are flashed at one single point in time (does that for the whole trial)
				 */
				CoAdaptP300IEvidenceAccumulator(P300StimulatorPropertyReader* propertyObject, P300SequenceGenerator* l_pSequenceGenerator):m_opropertyObject(propertyObject),m_pSequenceGenerator(l_pSequenceGenerator),m_bIsReadyToPredict(false)
				{
					m_pAccumulatedEvidence = new OpenViBE::CMatrix();
					m_pAccumulatedEvidence->setDimensionCount(1);
					m_pAccumulatedEvidence->setDimensionSize(0, l_pSequenceGenerator->getNumberOfSymbols());

					m_pCurrentEvidence = new OpenViBE::CMatrix();
					m_pCurrentEvidence->setDimensionCount(1);
					m_pCurrentEvidence->setDimensionSize(0, l_pSequenceGenerator->getNumberOfSymbols());

					//this should fill the matrix with 0
					OpenViBEToolkit::Tools::MatrixManipulation::clearContent(*m_pAccumulatedEvidence);
					OpenViBEToolkit::Tools::MatrixManipulation::clearContent(*m_pCurrentEvidence);

					m_bStopCondition = propertyObject->getStopCondition();
					earlyStoppingEnabled = propertyObject->getEarlyStopping();
					maxRepetition = propertyObject->getNumberOfRepetitions();
					m_oSharedMemoryReader = new CoAdaptP300SharedMemoryReader();
					OpenViBE::CString l_sSharedMemoryName = propertyObject->getSharedMemoryName();
					m_oSharedMemoryReader->openSharedMemory(l_sSharedMemoryName);
					m_ui64Prediction=0;

				}

				virtual ~CoAdaptP300IEvidenceAccumulator()
				{
					m_opropertyObject=NULL;
					m_pSequenceGenerator=NULL;
					m_oSharedMemoryReader->closeSharedMemory();
				}

				
				/**
				 * At the beginning of the the next trial, generate the whole sequence of letters that have to be flashed in the trial
				 */
				virtual void generateNewSequence() { m_pSequenceGenerator->generateSequence(); }

				/**
				* @return the index of the predicted symbol, starts at 1. A prediction equal to 0 is ignored (not enough evidence for a prediction)
				*/
				virtual OpenViBE::uint64 getPrediction()=0;


				//reset all accumulated evidence, used when a new trial start
				//implement another in the derived class if you have additionnal evidence data holder
				virtual void flushEvidence()
				{
					std::cout <<"flush\n";
					m_ui64Prediction=0;
					m_ui32CurrentFlashIndex=0;
					m_bIsReadyToPredict=false;
					//clear buffer
					OpenViBEToolkit::Tools::MatrixManipulation::clearContent(*m_pAccumulatedEvidence);

				}

				//function called at each loop of the stimulator
				virtual OpenViBE::boolean update()
				{
					//get the evidence from shared memory
					OpenViBE::IMatrix* currentEvidence = m_oSharedMemoryReader->readNextSymbolProbabilities();
					
					if(currentEvidence!=NULL)
					{
						//the matrix should only contain a single value, the proba of the group containing a p300
						OpenViBE::float64* proba = currentEvidence->getBuffer();
						//match to the current flash group
						std::cout << "Evidence Acc update with proba " << proba[0] << std::endl;
						OpenViBE::IMatrix* evidenceToAdd = matchProbaLetters(proba[0]);
						accumulate(evidenceToAdd);
						if(earlyStoppingEnabled)//get the token wich says if early stopping is enabled or not
						{
							m_bIsReadyToPredict = stopEarly();
							std::cout << "Early stop enabled and early stop condition is ";
							if(!m_bIsReadyToPredict)
								std::cout << " not ";
							std::cout << "met" <<  std::endl;
						}
						//there is NumberOfGroup flashes by repetition so if we have more than NumberOfGroup*maxRepetition flashes, we force a predicition
						if(m_ui32CurrentFlashIndex>=maxRepetition*m_pSequenceGenerator->getNumberOfGroups())
						{
							std::cout << "Waited more than " << maxRepetition*m_pSequenceGenerator->getNumberOfGroups() << " (" <<  m_ui32CurrentFlashIndex << ")" << std::endl;
							m_bIsReadyToPredict=true;
						}
						std::cout << "Evidence Accumulator clear " << std::endl;
						m_oSharedMemoryReader->clearSymbolProbabilities();//clear to avoid reading it again
						return true;//we updated
					}
					return false;//nothing in input to update with
				}
				
				/**
				 * @return return vector of zeros and ones defining which letters will be flashed next
				 */
				virtual std::vector<OpenViBE::uint32>* getNextFlashGroup() { return m_pSequenceGenerator->getFlashGroupAt(m_ui32CurrentFlashIndex); }
				
				/**
				 * @return The shared memory reader that is created during construction of the EvidenceAccumulator
				 */
				virtual CoAdaptP300SharedMemoryReader* getSharedMemoryReader() { return m_oSharedMemoryReader; }

		protected:

				virtual void accumulate(OpenViBE::IMatrix* mEvidenceToAdd)=0;

				//find maximum
				void findMaximum(OpenViBE::float64* vector, OpenViBE::uint32* l_ui32MaximumIndex, OpenViBE::float32* l_f32Maximum)
				{
					*l_f32Maximum = std::numeric_limits<int>::min();
					*l_ui32MaximumIndex = 0;
					for (unsigned int j=0; j<m_pAccumulatedEvidence->getBufferElementCount(); j++)
						if (*(vector+j)>(*l_f32Maximum))
						{
							*l_f32Maximum = *(vector+j);
							*l_ui32MaximumIndex = j;
						}
					//if an index of 0 is a mistake, we must start to count at 1
					(*l_ui32MaximumIndex)++;

					//return  l_ui32MaximumIndex;
				}

				//update the proba of the letters based on the current flash
				OpenViBE::IMatrix* matchProbaLetters(OpenViBE::float64 proba)
				{
					//get letter in the current flash
					std::vector<OpenViBE::uint32>* currentFlashGroup = this->getNextFlashGroup();
					m_ui32CurrentFlashIndex++;
					std::cout << " got group  " << m_ui32CurrentFlashIndex << std::endl;
					for(OpenViBE::uint32 i=0; i<currentFlashGroup->size(); i++)
					{
						std::cout << currentFlashGroup->at(i) << ", ";
					}
					std::cout << std::endl;

					OpenViBEToolkit::Tools::MatrixManipulation::clearContent(*m_pCurrentEvidence);
					OpenViBE::float64* buffer = m_pCurrentEvidence->getBuffer();

					//check
					if(!(m_pCurrentEvidence->getBufferElementCount()==currentFlashGroup->size()))
						return NULL;

					for(unsigned int i=0; i<currentFlashGroup->size(); i++)
					{
						(*(buffer+i))=proba*currentFlashGroup->at(i);
					}

					return m_pCurrentEvidence;
				}

				//check against early stopping criteria, implementation details left to derived class
				//return true if we can stop, false if we have to wait for more evidence
				virtual OpenViBE::boolean stopEarly()=0;


				CoAdaptP300SharedMemoryReader* m_oSharedMemoryReader;
				OpenViBE::IMatrix* m_pAccumulatedEvidence;
				OpenViBE::IMatrix* m_pCurrentEvidence;
				OpenViBE::uint32 m_ui64Prediction;
				OpenViBE::uint32 m_ui32CurrentFlashIndex;
				P300StimulatorPropertyReader* m_opropertyObject;
				P300SequenceGenerator* m_pSequenceGenerator;
				bool m_bIsReadyToPredict;
				bool earlyStoppingEnabled;
				OpenViBE::uint32 maxRepetition;//number of repetition before we force the accumulator to make a prediction
				OpenViBE::float64 m_bStopCondition;
				
		};

};
#endif

#endif
