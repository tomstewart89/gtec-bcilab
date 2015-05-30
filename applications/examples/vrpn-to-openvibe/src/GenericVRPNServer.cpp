#include "GenericVRPNServer.h"

#include <vrpn_Connection.h>
#include <vrpn_Button.h>
#include <vrpn_Analog.h>

#include <cstdarg>

GenericVRPNServer* GenericVRPNServer::serverInstance = NULL;

GenericVRPNServer::GenericVRPNServer(int port)
{
	_connection = vrpn_create_server_connection(port);
}

GenericVRPNServer::~GenericVRPNServer()
{
	deleteInstance();
}

GenericVRPNServer* GenericVRPNServer::getInstance(int port)
{
	if (serverInstance == NULL)
	{
		serverInstance = new GenericVRPNServer(port);
	}

	return serverInstance;
}

void GenericVRPNServer::deleteInstance(void)
{
	for (std::map<std::string, ButtonServer>::iterator it = serverInstance->_buttonServer.begin(); it != serverInstance->_buttonServer.end(); ++it)
	{
		delete it->second.server;
	}
	serverInstance->_buttonServer.clear();

	for (std::map<std::string, AnalogServer>::iterator it = serverInstance->_analogServer.begin(); it != serverInstance->_analogServer.end(); ++it)
	{
		delete it->second.server;
	}
	serverInstance->_analogServer.clear();

	delete serverInstance;
	serverInstance = NULL;
}

void GenericVRPNServer::loop()
{
	
	for (std::map<std::string, ButtonServer>::iterator it = _buttonServer.begin(); it != _buttonServer.end(); ++it)
	{
		it->second.server->mainloop();
	}
	

	for (std::map<std::string, AnalogServer>::iterator it = _analogServer.begin(); it != _analogServer.end(); ++it)
	{
		it->second.server->mainloop();
	}

	_connection->mainloop();
}


void GenericVRPNServer::addButton(std::string name, int buttonCount)
{
	ButtonServer serverObject;

	serverObject.server = new vrpn_Button_Server(name.c_str(), _connection, buttonCount);
	serverObject.buttonCount = buttonCount;

	_buttonServer.insert(std::pair<std::string, ButtonServer>(name, serverObject));
	_buttonServer[name].cache.clear();
	_buttonServer[name].cache.resize(buttonCount);
}

void GenericVRPNServer::changeButtonState(std::string name, int index, int state)
{
	_buttonServer[name].server->set_button(index, state);
	_buttonServer[name].cache[index] = state;
}

int GenericVRPNServer::getButtonState(std::string name, int index)
{
	return _buttonServer[name].cache[index];
}

void GenericVRPNServer::addAnalog(std::string name, int channelCount)
{
	AnalogServer serverObject;

	serverObject.server = new vrpn_Analog_Server(name.c_str(), _connection, channelCount);
	serverObject.channelCount = channelCount;

	_analogServer.insert(std::pair<std::string, AnalogServer>(name, serverObject));
}

void GenericVRPNServer::changeAnalogState(std::string name, ...)
{
	double* channels = _analogServer[name].server->channels();

	va_list list;

	va_start(list, name);

	for (int i = 0; i < _analogServer[name].channelCount; i++)
	{
		channels[i] = va_arg(list, double);
	}

	va_end(list);

	_analogServer[name].server->report();
}

double* GenericVRPNServer::getAnalogChannels(std::string name)
{
	return _analogServer[name].server->channels();
}

void GenericVRPNServer::reportAnalogChanges(std::string name)
{
	_analogServer[name].server->report();
}
