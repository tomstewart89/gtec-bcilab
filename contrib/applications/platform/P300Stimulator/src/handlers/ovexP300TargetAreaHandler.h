#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __P300TargetAreaHandler_H__
#define __P300TargetAreaHandler_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <cstring>

#include "../visualisation/glGTable.h"
#include "../properties/ovexP300ScreenLayoutReader.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>

namespace OpenViBEApplications
{	
	//template<class TContainer>
	class P300TargetAreaHandler : public GObserver, public GObservable
	{	

	public:
		
		P300TargetAreaHandler(GTable* container, P300ScreenLayoutReader* propertyObject);
		
		virtual ~P300TargetAreaHandler()
		{
		}
		
		//inherited from GObserver
		virtual void update(GObservable* observable, const void * pUserData);
		
	private:
		void moveSymbolsLeft(OpenViBE::uint32 nshift);
		
	protected:
		GTable* m_pSymbolContainer;
		
	private:
		OpenViBE::uint32 m_ui32State;
		P300ScreenLayoutReader* m_pScreenLayoutObject;
	};
};
#endif
#endif

#endif
