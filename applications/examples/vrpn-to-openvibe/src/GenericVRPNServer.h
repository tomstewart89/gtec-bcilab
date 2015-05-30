/**
 * \file GenericVRPNServer.h
 * \author Jozef Legény
 *
 * Copyright : Inria (2012)
 * License : LGPLv2 -> AGPL3
 */

#ifndef __GenericVRPNServer__
#define __GenericVRPNServer__

#include <map>
#include <vector>
#include <string>

class vrpn_Connection;
class vrpn_Button_Server;
class vrpn_Analog_Server;

/**
 * \class GenericVRPNServer
 * \brief A class providing a very simple generic VRPN server capable of creating Analog and Button controls.
 */
class GenericVRPNServer
{

	public:

		struct ButtonServer {
			vrpn_Button_Server* server;
			int buttonCount;
			std::vector<int> cache;
		};

		struct AnalogServer {
			vrpn_Analog_Server* server;
			int channelCount;
		};


		/// Public singleton factory
		static GenericVRPNServer* getInstance(int port);

		static void deleteInstance(void);

		/// Public destructor
		~GenericVRPNServer();

		/// The loop() method has to be called periodically in order for vrpn to work
		void loop();

		/** Creates a new button object within the VRPN server
		 * \param name name of the vrpn peripheral
		 * \param buttonCount number of virtual buttons in the peripeheral
		 */
		void addButton(std::string name, int buttonCount);

		/** Change the button state of a button inside a created VRPN peripheral
		 * \param name name of the vrpn peripheral containing the button
		 * \param index index of the button (beginning by 0)
		 * \param state new state of the button 0 = off, 1 = on
		 */
		void changeButtonState(std::string name, int index, int state);


		/** Get the state of a button
		 * \param name name of the vrpn peripheral containing the button
		 * \param index index of the button (beginning by 0)
		 * \return the state of the button
		 */
		int getButtonState(std::string name, int index);

		/** Creates a new analog object within the VRPN server
		 * \param name name of the vrpn peripheral
		 * \param channelCount number of channels in the peripeheral
		 */
		void addAnalog(std::string name, int channelCount);

		/** Change the state of channels of an analog VRPN peripheral
		 * \param name name of the vrpn peripheral containing the analog control
		 * \param ellipsis list of the values (double)
		 */
		void changeAnalogState(std::string name, ...);


		/** Gets a pointer to the channel array
		 * \param name name of the vrpn peripheral containing the analog control
		 * \return pointer to the array containing the channels
		 */
		double* getAnalogChannels(std::string name);

		/** Marks the selected analog server channels as modified so the values are sent in the next loop
		 * \param name name of the vrpn peripheral containing the analog control
		 */ 
		void reportAnalogChanges(std::string name);

	private:
		static GenericVRPNServer* serverInstance;

		GenericVRPNServer(int port);

		vrpn_Connection* _connection;
		std::map<std::string, ButtonServer> _buttonServer;
		std::map<std::string, AnalogServer> _analogServer;

};

#endif // __GenericVRPNServer__
