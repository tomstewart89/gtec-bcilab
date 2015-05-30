#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300TargetAreaHandler.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "ovexUndoHandler.h"
#include "../visualisation/glGButton.h"
#include "../visualisation/glGSymbol.h"
#include "../visualisation/glGPictureSymbol.h"

#include <vector>
#include <stdexcept>

#include <system/ovCTime.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

P300TargetAreaHandler::P300TargetAreaHandler(GTable* container, P300ScreenLayoutReader* propertyObject) 
: m_pSymbolContainer(container), m_pScreenLayoutObject(propertyObject)
{
}

void P300TargetAreaHandler::update(GObservable* observable, const void * pUserData)
{
	GButton* l_pObservedButton = dynamic_cast<GButton*>(observable);
	//if the notification comes from a key of the keyboard
	if (l_pObservedButton!=NULL)
	{
		const GButton* l_pNewButton = static_cast<const GButton*>(pUserData);
		m_ui32State = l_pNewButton->getState();
		if (m_ui32State==GButton_Focus)
		{
			GLabel* l_pLabel = l_pNewButton->getLabel()->clone();

			boolean l_bMoveToLeft = false;
			if (m_pSymbolContainer->getNumberOfChildren()+1>m_pSymbolContainer->getColumnDimension())
			{
				moveSymbolsLeft(1); //if necessary
				l_bMoveToLeft = true;
			}
			
			m_pSymbolContainer->addChild(l_pLabel,l_pLabel->getDepth());
			//notify the result area that the target area moved left (to sync both areas)
			if (l_bMoveToLeft)
				this->notifyObservers(NULL);
		}
	}
}

void P300TargetAreaHandler::moveSymbolsLeft(OpenViBE::uint32 nshift)
{
	nshift = m_pSymbolContainer->getNumberOfChildren()+nshift-m_pSymbolContainer->getColumnDimension();
	for(int i=m_pSymbolContainer->getNumberOfChildren()-1-nshift;i>=0;i--)
	{
		//std::cout << "move left, number of children " << m_pSymbolContainer->getNumberOfChildren() << ", moving symbol " << i<< "\n";
		m_pSymbolContainer->getChild(0, i+nshift)->setDimParameters(m_pSymbolContainer->getChild(0, i)->getDimParameters());	
	}
	for(uint32 i=0;i<nshift;i++)
		m_pSymbolContainer->removeChild(0);	
}
#endif

#endif
