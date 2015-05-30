#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __GObservableAAAA_OV_H__
#define __GObservableAAAA_OV_H__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <iostream>
#include <vector>

#include "glGObserver.h"

#include "../ova_defines.h"

namespace OpenViBEApplications
{
	/**
	 * This class implements some kind of MVC (Model-View-Controller) pattern. Each class implementing GObservable can register observers that are notified when an event occurs.
	 */
	class GObservable
	{
	public:
		/**
		 * Simple constructor that creates a vector of to hold the observers
		 */
		GObservable();

		/**
		 * The destructor is not responsible for the clean up of the GObserver objects as they are created elsewhere, only the vector that holds the GObserver objects is cleaned up. So all the GObserver objects that are registered should be cleaned up afterwards by the user.
		 */
		virtual ~GObservable();
		
		/**
		 * Copy constructor calls the vector copy constructor
		 * @param gobservable GObservable object that is copied
		 */
		GObservable(const GObservable& gobservable);		
		
		/**
		 * Assignment operator clears the vector holding the GObserver objects and fills them with the GObserver objects in the gobservable vector
		 * @param gobservable observable object that is assigned to this
		 */		
		GObservable& operator= (GObservable const& gobservable);
		
		/**
		 * Registers an GObserver to the GObservable
		 * @param observer GObserver object that is stored in the vector and is notified when an event occurs
		 */
		virtual void addObserver(GObserver * observer);
		
		/**
		 * Notifies all the registered GObserver objects when an event occurs
		 * @param pUserData data/message/event that is transmitted to the GObserver objects
		 */		
		virtual void notifyObservers(const void * pUserData);
		
	private:
		std::vector< GObserver* > * m_vObservers;
	};
};

#endif

#endif
