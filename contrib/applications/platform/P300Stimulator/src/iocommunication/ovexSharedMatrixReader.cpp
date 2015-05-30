#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexSharedMatrixReader.h"

#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

boolean SharedMatrixReader::open()
{
	ISharedMemoryReader::open();
	
	if (!m_bFailedToFind)
	{
		m_vMatrixVector  = m_oSharedMemory.find< MyVectorStreamedMatrix >(m_sSharedVariableName.toASCIIString()).first;
		if (!m_vMatrixVector)
			m_bFailedToFind = true;
	}	
	
	return !m_bFailedToFind;
}

boolean SharedMatrixReader::open(CString sharedMemoryName, CString sharedVariableName)
{
	ISharedMemoryReader::open(sharedMemoryName, sharedVariableName);
	return this->open();
}

//unsafe private method should be enclosed by mutex lock
IMatrix* SharedMatrixReader::_front()
{
	CMatrix* l_pMatrix;
	
	if (!m_bFailedToFind && m_vMatrixVector->size()>0)
	{
		offset_ptr<SMatrix> l_sMatrix = m_vMatrixVector->front();
		l_pMatrix = new CMatrix();
		l_pMatrix->setDimensionCount(2);
		l_pMatrix->setDimensionSize(0,l_sMatrix->columnDimension);
		l_pMatrix->setDimensionSize(1,l_sMatrix->rowDimension);
		for (uint32 di=0; di<l_pMatrix->getBufferElementCount(); di++)
			*(l_pMatrix->getBuffer()+di) = *(l_sMatrix->data+di);	
	}
	else
		l_pMatrix = NULL;
	
	return l_pMatrix;
}

//calller should clean up returned IMatrix
IMatrix* SharedMatrixReader::front()
{
	IMatrix* l_pMatrix;	
	
	scoped_lock<named_mutex> lock(*m_pMutex, try_to_lock);
	if (lock)
		l_pMatrix = this->_front();
	else
		l_pMatrix = NULL;
	
	return l_pMatrix;
}

IMatrix* SharedMatrixReader::pop_front()
{
	IMatrix* l_pMatrix;	
	
	scoped_lock<named_mutex> lock(*m_pMutex, try_to_lock);
	if (lock)
	{	
		l_pMatrix = this->_front();
		if (!m_bFailedToFind && m_vMatrixVector->size()>0)
			m_vMatrixVector->erase(m_vMatrixVector->begin());
	}
	else
		l_pMatrix = NULL;
	
	return l_pMatrix;
}

void SharedMatrixReader::clear()
{
	for (uint32 it=0; it<m_vMatrixVector->size(); it++)
	{
		m_oSharedMemory.deallocate(m_vMatrixVector->at(it)->data.get());
		m_oSharedMemory.deallocate(m_vMatrixVector->at(it).get());
	}
	m_vMatrixVector->clear();
}

void SharedMatrixReader::close()
{
	this->clear();	
	m_oSharedMemory.destroy<MyVectorStreamedMatrix>(m_sSharedVariableName.toASCIIString());	
	//ISharedMemoryReader::close();
}

#endif
