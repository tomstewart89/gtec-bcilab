#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __GObserverAAAAB_OV_H__
#define __GObserverAAAAB_OV_H__

#include "../ova_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "glGObservable.h"

namespace OpenViBEApplications
{
	class GObservable;
	
	/**
	 * This interface is part of some kind of MVC (Model-View-Controller) pattern. Each class implementing this interface can be registered to an GObservable object so that it is notified in case of an event.
	 */
	class GObserver
	{
	public:
		/**
		 * Each class inheriting from this interface should implement this method. It defines what the GObserver will do in case it is notified by the GObservable
		 * @param observable the GObservable object that is observed
		 * @param pUserData the event/message/data that is send by the GObservable object to the GObserver
		 */
		virtual void update(GObservable* observable, const void * pUserData) = 0;
	};
};

#endif

#endif
