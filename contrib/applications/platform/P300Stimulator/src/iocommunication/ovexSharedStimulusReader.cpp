#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexSharedStimulusReader.h"

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

boolean SharedStimulusReader::open()
{
	ISharedMemoryReader::open();
	
	if (!m_bFailedToFind)
	{
		m_vStimulusVector  = m_oSharedMemory.find< MyVectorStimulation >(m_sSharedVariableName.toASCIIString()).first;
		if(!m_vStimulusVector)
			m_bFailedToFind = true;
	}
	
	return !m_bFailedToFind;
}

boolean SharedStimulusReader::open(CString sharedMemoryName, CString sharedVariableName)
{
	ISharedMemoryReader::open(sharedMemoryName, sharedVariableName);
	return this->open();
}

//unsafe private method should be enclosed by mutex lock
IStimulationSet* SharedStimulusReader::_front()
{
	CStimulationSet* l_pStimulusSet;
	if (!m_bFailedToFind && m_vStimulusVector->size()>0)
	{
		uint64 l_sStimulusId = m_vStimulusVector->front();

		l_pStimulusSet = new CStimulationSet();
		l_pStimulusSet->appendStimulation(l_sStimulusId, 0, 0);
	}
	else
		l_pStimulusSet = NULL;
	
	return l_pStimulusSet;
}

IStimulationSet* SharedStimulusReader::front()
{
	IStimulationSet* l_pStimulusSet;
	
	scoped_lock<named_mutex> lock(*m_pMutex, try_to_lock);
	if (lock)
		l_pStimulusSet = this->_front();
	else
		l_pStimulusSet = NULL;
	
	return l_pStimulusSet;
}

IStimulationSet* SharedStimulusReader::pop_front()
{
	IStimulationSet* l_pStimulusSet;	
	scoped_lock<named_mutex> lock(*m_pMutex, try_to_lock);
	//MAKE SURE TO USE THE MEMBER MUTEX AGAIN
	//CString l_sMutexName = m_sSharedMemoryName+CString("_Mutex");
	//named_mutex l_mutex(open_or_create, l_sMutexName.toASCIIString());
	//scoped_lock<named_mutex> lock(l_mutex, try_to_lock);
	if (lock)
	{	
		l_pStimulusSet = this->_front();
		if (!m_bFailedToFind && m_vStimulusVector->size()>0)
			m_vStimulusVector->erase(m_vStimulusVector->begin());
	}
	else
		l_pStimulusSet = NULL;
	
	return l_pStimulusSet;
}

void SharedStimulusReader::clear()
{
	m_vStimulusVector->clear();
}

void SharedStimulusReader::close()
{
	this->clear();		
	m_oSharedMemory.destroy<MyVectorStimulation>(m_sSharedVariableName.toASCIIString());	
	//ISharedMemoryReader::close();
}

#endif
