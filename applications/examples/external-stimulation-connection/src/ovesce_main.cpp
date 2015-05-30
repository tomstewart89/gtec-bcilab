#include "openvibeStimulationConnection.h"


// for the purpose of this example we are using the Beep stimulation
#define OVTK_StimulationId_Beep 0x00008202

#include <iostream>

#if defined(TARGET_OS_Linux)
#include <unistd.h>
#else
#include <windows.h>
#endif


/** @file This program is a very simple example of the usage of the Software
 * Stimulation capabilities of OpenViBE Acquisition server.
 *
 * This program sends a Beep stimulation to the acquisition server, using the default
 * message queue name. 
 */

int main()
{
	std::cout << "Creating a new OpenvibeStimulationConnection object" << std::endl;
	OpenViBE::StimulationConnection* osc = new OpenViBE::StimulationConnection();

	while(1)
	{
		std::cout << "Sending a Beep stimulation" << std::endl;
		osc->sendStimulation(OVTK_StimulationId_Beep);
#if defined(TARGET_OS_Linux)
		sleep(1);
#else
		Sleep(1000);
#endif
	}

	delete osc;

	return 0;
}
