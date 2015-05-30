#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300CNULLStimulator.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "../ova_defines.h"
#include <system/ovCTime.h>

#include <openvibe/ovITimeArithmetics.h>//if debug?

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

CoAdaptP300CNULLStimulator::CoAdaptP300CNULLStimulator(P300StimulatorPropertyReader* propertyObject, P300SequenceGenerator* l_pSequenceGenerator)
				: CoAdaptP300IStimulator(), m_pPropertyObject(propertyObject)
{
	m_pSequenceGenerator = l_pSequenceGenerator;
	
	#ifdef OUTPUT_TIMING
	timingFile = fopen(OpenViBE::Directories::getUserDataDir() + "/xP300-stimulator_round_timing.txt","w");
	#endif
	
	m_ui64RealCycleTime = 0;
	m_ui64StimulatedCycleTime = 0;
	m_ui32TrialCount=0;
	m_ui32TrialIndex=0;

}

CoAdaptP300CNULLStimulator::~CoAdaptP300CNULLStimulator()
{
	#ifdef OUTPUT_TIMING
	fclose(timingFile);
	#endif
}

void CoAdaptP300CNULLStimulator::run()
{

	if (m_ui32TrialCount==0)
		m_ui32TrialCount = UINT_MAX-1;

	uint32 l_ui32StimulatorFrequency = 250; //TODO should be a configurable parameter
	uint64 l_ui64TimeStep = static_cast<uint64>(ITimeArithmetics::sampleCountToTime(l_ui32StimulatorFrequency, 1LL));
	uint64 l_ui64CurrentTime = 0;
	


	while (m_ui32TrialIndex<=m_ui32TrialCount)
	{
		uint64 l_ui64TimeBefore = System::Time::zgetTime();
		#ifdef OUTPUT_TIMING
			fprintf(timingFile, "%f \n",float64(ITimeArithmetics::timeToSeconds(System::Time::zgetTime())));
		#endif
		
		//very often one cycle of this loop does not take much time, so we just put the program to sleep until the next time step
		if (m_ui64RealCycleTime<m_ui64StimulatedCycleTime)
			System::Time::sleep(static_cast<uint32>(std::ceil(1000.0*ITimeArithmetics::timeToSeconds(m_ui64StimulatedCycleTime-m_ui64RealCycleTime+l_ui64TimeStep))));

		IStimulationSet* l_pStimSet = m_oEvidenceAcc->getSharedMemoryReader()->readStimulation();

		if(l_pStimSet!=NULL)//&&(l_ui64Prediction!=OVA_StimulationId_Flash))
		{
			for(unsigned int i=0; i<l_pStimSet->getStimulationCount(); i++)
			{
				//int j = l_pStimSet->getStimulationCount()-1 - i;
				//std::cout << i << "/" << l_pStimSet->getStimulationCount() << std::endl;
				int l_ui64Prediction = l_pStimSet->getStimulationIdentifier(i);

				//the first recorded file index letters from 0
				//but since the visualizer now consider 0 to be an error
				//if(l_ui64Prediction==0)
					///l_ui64Prediction=1;
				//std::cout << "\n		Stimulator callback on visu for  " << int(l_ui64Prediction) <<  std::endl;
				m_oFuncVisualiserCallback(l_ui64Prediction);
			}
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
		std::cout << "Stimulator waiting " << std::endl;
		m_oFuncVisualiserWaitCallback(1);
		while (!checkForQuitEvent())
		{
			m_oFuncVisualiserWaitCallback(1);
		}
	}
	m_oFuncVisualiserCallback(OVA_StimulationId_ExperimentStop);
}
#endif

#endif
