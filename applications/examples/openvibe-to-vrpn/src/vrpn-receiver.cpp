/*
 * Receives data from OpenViBE's VRPN boxes
 *
 * See here: http://openvibe.inria.fr/vrpn-tutorial-sending-data-from-openvibe-to-an-external-application/
 *
 */

#include <iostream>
 
#include <vrpn_Button.h>
#include <vrpn_Analog.h>
 
void VRPN_CALLBACK vrpn_button_callback(void* user_data, vrpn_BUTTONCB button)
{
    std::cout << "Button ID : " << button.button << " / Button State : " << button.state << std::endl;
 
    if (button.button == 1)
    {
		std::cout << "Quit requested by button press" << std::endl;
        *(bool*)user_data = false;
    }
}
 
void VRPN_CALLBACK vrpn_analog_callback(void* user_data, vrpn_ANALOGCB analog)
{
    for (int i = 0; i < analog.num_channel; i++)
    {
        std::cout << "Analog Channel : " << i << " / Analog Value : " << analog.channel[i] << std::endl;
    }
}
 
int main(int argc, char** argv)
{
    /* flag used to stop the program execution */
    bool running = true;
 
    /* VRPN Button object */
    vrpn_Button_Remote* VRPNButton;
 
    /* Binding of the VRPN Button to a callback */
    VRPNButton = new vrpn_Button_Remote( "openvibe_vrpn_button@localhost" );
    VRPNButton->register_change_handler( &running, vrpn_button_callback );
 
    /* VRPN Analog object */
    vrpn_Analog_Remote* VRPNAnalog;
 
    /* Binding of the VRPN Analog to a callback */
    VRPNAnalog = new vrpn_Analog_Remote( "openvibe_vrpn_analog@localhost" );
    VRPNAnalog->register_change_handler( NULL, vrpn_analog_callback );
 
    /* The main loop of the program, each VRPN object must be called in order to process data */
    while (running)
    {
        VRPNButton->mainloop();
    }
 
	VRPNAnalog->unregister_change_handler(NULL, vrpn_analog_callback );
	VRPNButton->unregister_change_handler(&running, vrpn_button_callback );

	delete VRPNButton;
	delete VRPNAnalog;

    return 0;
}
