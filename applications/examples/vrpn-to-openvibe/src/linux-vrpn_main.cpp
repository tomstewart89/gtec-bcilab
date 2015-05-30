
#if !defined(WIN32)

#include "GenericVRPNServer.h"
#include <iostream>
#include <cmath>
#include <unistd.h>

#define DEFAULT_PORT 50555

int main(int argc, char** argv)
{
	GenericVRPNServer* vrpnServer = GenericVRPNServer::getInstance(DEFAULT_PORT);

	vrpnServer->addButton("button_test", 1);
	vrpnServer->addAnalog("analog_test", 2);

	double time = 0;
	double period = 0;

	while (true)
	{
		if (period >= 2 * M_PI)
		{
			vrpnServer->changeButtonState("button_test", 0, 1 - vrpnServer->getButtonState("button_test", 0));
			period = 0;
		}

		vrpnServer->changeAnalogState("analog_test", sin(time), cos(time));

		time = time + 0.01;
		period = period + 0.01;
		
		vrpnServer->loop();

		// sleep for 10 miliseconds (on Unix)
		usleep(10000);
	}

	GenericVRPNServer::deleteInstance();
	vrpnServer = NULL;

	return 0;
}

#endif // !WIN32
