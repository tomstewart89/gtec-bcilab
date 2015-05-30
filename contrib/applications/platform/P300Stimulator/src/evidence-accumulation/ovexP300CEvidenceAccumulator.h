#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __EvidenceCAccumulator_H__
#define __EvidenceCAccumulator_H__

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
		 * The EvidenceAccumulator class takes evidence from OpenVibe (via shared memory) and determines which symbol is selected
		 */
		class CoAdaptP300CEvidenceAccumulator : public CoAdaptP300IEvidenceAccumulator
		{
			public:
				
				CoAdaptP300CEvidenceAccumulator(P300StimulatorPropertyReader* propertyObject, P300SequenceGenerator* l_pSequenceGenerator):CoAdaptP300IEvidenceAccumulator(propertyObject, l_pSequenceGenerator)
				{
					m_pNormalizedAccumulatedEvidence = new OpenViBE::CMatrix();
					OpenViBEToolkit::Tools::Matrix::copy(*m_pNormalizedAccumulatedEvidence, *m_pAccumulatedEvidence);
					m_ui32CurrentFlashIndex=0;
				}
				
				virtual ~CoAdaptP300CEvidenceAccumulator(){}
				
				virtual OpenViBE::uint64 getPrediction()
				{
					if(m_bIsReadyToPredict)
					{
						OpenViBE::float32 max;
						findMaximum(m_pNormalizedAccumulatedEvidence->getBuffer(), &m_ui64Prediction, &max);
						m_bIsReadyToPredict=false;
					}
					//if we are not ready, the prediction will be 0 and ignored
					return m_ui64Prediction;
				}

				virtual OpenViBE::boolean stopEarly()
				{
					OpenViBE::uint32 l_ui32Argmax;
					OpenViBE::float32 l_f32Max;
					unsigned int j=0;
					OpenViBE::float64* l_pBuffer = m_pNormalizedAccumulatedEvidence->getBuffer();
					findMaximum(l_pBuffer, &l_ui32Argmax, &l_f32Max);


					bool l_bEarlyStoppingConditionMet = true;
					while((l_bEarlyStoppingConditionMet)&&(j<m_pNormalizedAccumulatedEvidence->getBufferElementCount()))
					{
						if((l_pBuffer[j]>l_f32Max-m_bStopCondition)&&(j!=l_ui32Argmax-1))
							l_bEarlyStoppingConditionMet=false;
						j++;
					}

					if(l_bEarlyStoppingConditionMet)
					{
						m_ui64Prediction = l_ui32Argmax;
					}
					return l_bEarlyStoppingConditionMet;
				}

				virtual void flushEvidence()
				{
					CoAdaptP300IEvidenceAccumulator::flushEvidence();
					//clear buffer
					OpenViBEToolkit::Tools::MatrixManipulation::clearContent(*m_pNormalizedAccumulatedEvidence);
				}



				
			private:



				void accumulate(OpenViBE::IMatrix* mEvidenceToAdd)
				{
					accumulateBayesian(mEvidenceToAdd);
				}

				void accumulateCounter(OpenViBE::IMatrix* mEvidenceToAdd)
				{	
						//getLogManager() << LogLevel_Info << "Evidence algorithm 'Counter' update\n";
						OpenViBE::float64 l_f64Sum = 0;
						for (OpenViBE::uint32 j=0; j<mEvidenceToAdd->getBufferElementCount(); j++)
						{
							*(m_pAccumulatedEvidence->getBuffer()+j) += *(mEvidenceToAdd->getBuffer()+j);
							*(m_pNormalizedAccumulatedEvidence->getBuffer()+j) = *(m_pAccumulatedEvidence->getBuffer()+j);
							l_f64Sum += *(m_pAccumulatedEvidence->getBuffer()+j);
						}
						for (OpenViBE::uint32 j=0; j<mEvidenceToAdd->getBufferElementCount(); j++)
						{
							//std::cout << " " << *(m_pNormalizedAccumulatedEvidence->getBuffer()+j);
							*(m_pNormalizedAccumulatedEvidence->getBuffer()+j) /= l_f64Sum;
						}
				}

				void accumulateBayesian(OpenViBE::IMatrix* l_pInputMatrix)
				{
					OpenViBE::float64 l_f64RealPredictionValue = 0.0;
					OpenViBE::uint32 l_ui32NumberOfNonZero = 0;;
					for (OpenViBE::uint32 j=0; j<l_pInputMatrix->getBufferElementCount(); j++)
					{
						if(*(l_pInputMatrix->getBuffer()+j)!=0)
						{
							l_f64RealPredictionValue = *(l_pInputMatrix->getBuffer()+j);
							l_ui32NumberOfNonZero++;
						}
					}
					OpenViBE::uint32 l_ui32NumberOfZero = l_pInputMatrix->getBufferElementCount()-l_ui32NumberOfNonZero;

					for (OpenViBE::uint32 j=0; j<l_pInputMatrix->getBufferElementCount(); j++)
					{
						OpenViBE::float64 l_f64ProbabilityEstimate = 0.0;
						OpenViBE::float64 l_f64InputValue = *(l_pInputMatrix->getBuffer()+j);
						double m_bScaleFactor=0.5;
						if (false)//(m_oInputType==OVP_InputType_EvidenceAccumulationDistance)
						{
							l_f64ProbabilityEstimate = std::exp(m_bScaleFactor*l_f64InputValue) /(1+std::exp(m_bScaleFactor*l_f64RealPredictionValue));
						}
						else if (true)//(m_oInputType==OVP_InputType_EvidenceAccumulationProbability)
						{
							l_f64ProbabilityEstimate = l_f64InputValue==0?1.0-l_f64InputValue:l_f64InputValue;
						}
						if (l_f64InputValue!=0)
							*(m_pAccumulatedEvidence->getBuffer()+j) += std::log(l_f64ProbabilityEstimate/(OpenViBE::float64)l_ui32NumberOfNonZero);
						else
							*(m_pAccumulatedEvidence->getBuffer()+j) += std::log(l_f64ProbabilityEstimate/(OpenViBE::float64)l_ui32NumberOfZero);
					}

					OpenViBE::float32 l_f32Maximum;
					OpenViBE::uint32 m_ui32MaximumIndex;
					findMaximum(m_pAccumulatedEvidence->getBuffer(), &m_ui32MaximumIndex, &l_f32Maximum);

					OpenViBE::float64 l_f64Sum = 0.0;
					for (OpenViBE::uint32 j=0; j<l_pInputMatrix->getBufferElementCount(); j++)
					{
						*(m_pAccumulatedEvidence->getBuffer()+j) -= l_f32Maximum;
						l_f64Sum += std::exp(*(m_pAccumulatedEvidence->getBuffer()+j));
					}

					for (OpenViBE::uint32 j=0; j<l_pInputMatrix->getBufferElementCount(); j++)
					{
						*(m_pNormalizedAccumulatedEvidence->getBuffer()+j) = std::exp(*(m_pAccumulatedEvidence->getBuffer()+j))/l_f64Sum;
					}
				}

			private:

				OpenViBE::IMatrix* m_pNormalizedAccumulatedEvidence;
				OpenViBE::CIdentifier m_oEvidenceAlgorithm;

		};

};

#endif

#endif
