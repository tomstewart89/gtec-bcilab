#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "glGObservable.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

GObservable::GObservable()
{
	m_vObservers = new std::vector< GObserver * >();
}

GObservable::GObservable(const GObservable& gobservable)
{
	this->m_vObservers = new std::vector< GObserver* >(*gobservable.m_vObservers);
}

GObservable::~GObservable()
{
	delete m_vObservers; // it is not the responsibality of this class to clean up the observer objects as they are created elsewhere
}

GObservable& GObservable::operator= (GObservable const& gobservable)
{
	if(this!=&gobservable)
	{
		this->m_vObservers->clear();
		for (OpenViBE::uint32 i=0; i<gobservable.m_vObservers->size(); i++)
			this->m_vObservers->push_back(gobservable.m_vObservers->at(i)); 
	}
	return *this;
}

void GObservable::addObserver(GObserver* observer)
{
	boolean l_bDuplicate = false;
	for (uint32 i=0; i<m_vObservers->size(); i++)
		if (m_vObservers->at(i)==observer)
			l_bDuplicate = true;
	if (!l_bDuplicate)	
		m_vObservers->push_back(observer);
	else
		std::cout << "Duplicate observer won't be added\n";
}

void GObservable::notifyObservers(const void * pUserData) 
{
	for (OpenViBE::uint32 i=0; i<m_vObservers->size(); i++)
		m_vObservers->at(i)->update(this, pUserData);
}
#endif
