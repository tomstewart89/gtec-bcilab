#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "../../src/ovexP300SharedMemoryReader.h"
#include <iostream>
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <system/ovCTime.h>

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

int main(int argc, char *argv[])
{
	CoAdaptP300SharedMemoryReader* m_oSharedMemoryReader = new CoAdaptP300SharedMemoryReader();
	m_oSharedMemoryReader->openSharedMemory("SharedMemory_P300Stimulator");//TODO retrieve from configuration

	boolean l_bQuit=false;

	uint64 l_ui64TimeDataReceived = System::Time::zgetTime();

	while(!l_bQuit)
	{
		IMatrix* l_pMatrix = m_oSharedMemoryReader->readNextSymbolProbabilities();
		if(l_pMatrix!=NULL)
		{
			l_ui64TimeDataReceived = System::Time::zgetTime();
			uint32 l_ui32DimensionCount = l_pMatrix->getDimensionCount();
			for(uint32 i=0; i<l_ui32DimensionCount; i++)
			{
				for(uint32 j=0; j<l_pMatrix->getDimensionSize(i); j++)
				{
					cout << "dimension " << i << " / " << l_ui32DimensionCount << " size " << j << "\n";
				}
			}

			float64* l_pBuffer = l_pMatrix->getBuffer();
			for(uint32 c=0; c<l_pMatrix->getBufferElementCount(); c++)
			{
				cout << l_pBuffer[c] << "	";
			}
			cout << "\n";

			m_oSharedMemoryReader->clearSymbolProbabilities();
			cout << "cleared\n";
		}

		uint64 l_ui64Time = System::Time::zgetTime();
		if(ITimeArithmetics::timeToSeconds(l_ui64Time-l_ui64TimeDataReceived)>15)
		{
			cout << "Timeout " << ITimeArithmetics::timeToSeconds(l_ui64Time-l_ui64TimeDataReceived) << "\n";
			l_bQuit=true;
		}


	}
	
	

}

#else
#include <iostream>
int main(int argc, char *argv[])
{
	std::cout << "The compiler did not have the required libraries for the CoAdapt stimulator " << std::endl;
	return 1;
}

#endif
