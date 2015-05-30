#ifndef OVCOBSERVABLE_H
#define OVCOBSERVABLE_H

#include "ovIObserver.h"
#include "ov_base.h"

namespace OpenViBE
{
	class IObserver;

	/**
	* \class CObservable
	* \author Guillaume Serrière (Inria/Loria)
	* \date 2014-11-7
	* \brief OpenViBE Observable class
	*
	* This class furnished all mecanisms to handle the Observable part of the Observer/Observable
	* pattern implementation of OpenViBE.
	*/
	class OV_API CObservable
	{
	public:
		CObservable(void);
		virtual ~CObservable(void);

		/**
		 * \brief Add the observer give in parameter in the list of observers of the object.
		 * \param o [in] : the observer to add
		 * \note A same observer can be add multiple time.
		 */
		virtual void addObserver(IObserver *o);

		/**
		 * \brief Remove the observer give in parameter from the list of observers of the object. Only the first
		 * occurence of the observer will be remove from the list.
		 * \param o [in] : the observer to remove
		 */
		virtual void deleteObserver(IObserver *o);

	protected:
		/**
		 * \brief This function is used to indicate when the notification need to reach the observers. If this function
		 * is not called before notifyObserver, the notification will be ignore.
		 *
		 * \note The "changed" state will be reverted after each call to clearChanged and notifyObservers.
		 */
		virtual void setChanged();

		/**
		 * \brief Revert the effect of setChanged.
		 */
		virtual void clearChanged();

		/**
		 * \brief Indicate if some change have been made.
		 * \return \e true if setChanged have been called this the last clearChanged/notifyObservers call.
		 * \return \e false in other case.
		 */
		virtual OpenViBE::boolean hasChanged();

		/**
		 * \brief Notify all registered observers.
		 * \param data [in] : a pointer to data that will be send to observers.
		 */
		virtual void notifyObservers(void* data = NULL);

	private:
		struct ObserverList;
		ObserverList* m_pObserverList;
		OpenViBE::boolean m_bHasChanged;
	};
}


#endif // OVCOBSERVABLE_H
