#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include <vector>

#include "ovexBackspaceHandler.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "ovexP300ResultAreaHandler.h"
#include "ovexP300KeyboardHandler.h"
#include "../visualisation/glGButton.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

//class P300KeyboardHandler;
//class P300ResultAreaHandler;

void P300BackspaceHandler::update(GObservable* observable, const void * pUserData)
{
	P300ResultAreaHandler* l_pResultHandler = dynamic_cast<P300ResultAreaHandler*>(observable);
	GButton* l_pButtonHandler = dynamic_cast<GButton*>(observable);
	//std::vector<GLabel*>* l_sNewLabelVector = static_cast< std::vector<GLabel*>* >(pUserData);
	//const char * l_sNewString = static_cast< const char* >(pUserData);
	if (l_pResultHandler!=NULL)
	{
		m_sSpeltLetters = l_pResultHandler->getResultBuffer();
		std::cout <<"backspace handler, spelt letters are " << m_sSpeltLetters.c_str() <<"\n";
	}

	//if Backspace button is clicked
	if ((l_pButtonHandler!=NULL)&&(m_sSpeltLetters.compare("")!=0))
	{
		std::cout << "backspace handler clicked\n";
		const GButton* l_pButton = static_cast<const GButton*>(pUserData);
		if (l_pButton->getState()==GButton_Clicked)
		{
			std::cout << "backspace handler notifying observers\n";
			this->notifyObservers("<");
		}
	}
}
#endif

#endif
