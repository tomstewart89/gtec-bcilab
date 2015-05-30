#include "ovkCMessageWithData.h"

using namespace std;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;

CMessageWithData::CMessageWithData(const OpenViBE::Kernel::IKernelContext& rKernelContext)
	: OpenViBE::Kernel::TMessage<OpenViBE::Kernel::TKernelObject<OpenViBE::Kernel::IMessageWithData> >(rKernelContext)
{

}

CMessageWithData::~CMessageWithData()
{
	m_oUint64s.clear();
	m_oStrings.clear();
	m_oFloat64s.clear();

	for(std::map<CString, IMatrix* >::iterator it = m_oMatrices.begin();
		it!=m_oMatrices.end(); it++) 
	{
		delete (*it).second;
	}
	m_oMatrices.clear();
}

//---------------------------------------------------------------------------------------------------//
//------------------------------------------------- GETTERS -----------------------------------------//
//---------------------------------------------------------------------------------------------------//
//
// Note that any returned pointers are invalid after processMessage() scope has passed.
//

bool CMessageWithData::getValueUint64(const OpenViBE::CString &key, OpenViBE::uint64& rValueOut) const
{
	std::map<CString, uint64>::const_iterator l_oIterator = m_oUint64s.find(key);
	if (l_oIterator!=m_oUint64s.end())
	{
		rValueOut = l_oIterator->second;
		return true;
	}
	else
	{
		return false;
	}
}

bool CMessageWithData::getValueFloat64(const CString &key, OpenViBE::float64& rValueOut) const
{
	std::map<CString, float64>::const_iterator l_oIterator = m_oFloat64s.find(key);
	if (l_oIterator!=m_oFloat64s.end())
	{
		rValueOut = l_oIterator->second;
		return true;
	}
	else
	{
		return false;
	}
}

bool CMessageWithData::getValueCString(const CString &key, const OpenViBE::CString** pValueOut) const
{
	if(!pValueOut) 
	{
		this->getLogManager() << LogLevel_Error << "getValueCString() got passed in a NULL pointer\n";
		return false;
	}

	std::map<CString, CString>::const_iterator l_oIterator = m_oStrings.find(key);
	if (l_oIterator!=m_oStrings.end())
	{
		*pValueOut = &(l_oIterator->second);
		return true;
	}
	else
	{
		*pValueOut = NULL;
		return false;
	}
}

bool CMessageWithData::getValueIMatrix(const CString &key, const OpenViBE::IMatrix** pValueOut) const
{
	if(!pValueOut) 
	{
		this->getLogManager() << LogLevel_Error << "getValueIMatrix() got passed in a NULL pointer\n";
		return false;
	}

	std::map<CString, IMatrix*>::const_iterator l_oIterator = m_oMatrices.find(key);
	if (l_oIterator!=m_oMatrices.end())
	{
		*pValueOut = (l_oIterator->second);
		return true;
	}
	else
	{
		*pValueOut = NULL;
		return false;
	}
}

//---------------------------------------------------------------------------------------------------//
//------------------------------------------------- SETTERS -----------------------------------------//
//---------------------------------------------------------------------------------------------------//
bool CMessageWithData::setValueUint64(const CString &key, uint64 valueIn)
{
	m_oUint64s[key] = valueIn;

	return true;
}

bool CMessageWithData::setValueFloat64(const CString &key, float64 valueIn){
	m_oFloat64s[key] = valueIn;

	return true;
}

bool CMessageWithData::setValueCString(const CString &key, const CString &valueIn){
	//copy the ref inside
	m_oStrings[key] = valueIn;

	return true;
}

bool CMessageWithData::setValueIMatrix(const CString &key, const IMatrix &valueIn){
	
	CMatrix *l_pMatrix = new CMatrix();
	// we copy 'manually' since we do not have access to the toolkit functions
	const uint32 l_ui32DimensionCount = valueIn.getDimensionCount();
	l_pMatrix->setDimensionCount(l_ui32DimensionCount);
	for (uint32 i=0; i<l_ui32DimensionCount; i++)
	{
		const uint32 l_ui32DimensionSize = valueIn.getDimensionSize(i);
		l_pMatrix->setDimensionSize(i,l_ui32DimensionSize);
		for (uint32 j=0; j<l_ui32DimensionSize; j++)
		{
			const char* l_cLabel = valueIn.getDimensionLabel(i,j);
			l_pMatrix->setDimensionLabel(i,j,l_cLabel);
		}
	}

	float64* l_pBuffer = l_pMatrix->getBuffer();
	for (uint32 i=0;i<valueIn.getBufferElementCount();i++)
	{
		l_pBuffer[i] = valueIn.getBuffer()[i];
	}

	m_oMatrices[key] = l_pMatrix;

	return true;
}

//---------------------------------------------------------------------------------------------------//
//----------------------------------------------KEY GETTERS -----------------------------------------//
//---------------------------------------------------------------------------------------------------//
const OpenViBE::CString* CMessageWithData::getFirstCStringToken() const
{
	const CString* l_sToken = NULL;
	if (!m_oStrings.empty())
	{
		l_sToken = &m_oStrings.begin()->first;
	}
	return l_sToken;
}

const OpenViBE::CString* CMessageWithData::getFirstUInt64Token() const
{
	const CString* l_sToken = NULL;
	if (!m_oUint64s.empty())
	{
		l_sToken = &m_oUint64s.begin()->first;
	}
	return l_sToken;
}

const OpenViBE::CString* CMessageWithData::getFirstFloat64Token() const
{
	const CString* l_sToken = NULL;
	if (!m_oFloat64s.empty())
	{
		l_sToken = &m_oFloat64s.begin()->first;
	}
	return l_sToken;
}

const OpenViBE::CString* CMessageWithData::getFirstIMatrixToken() const
{
	const CString* l_sToken = NULL;
	if (!m_oMatrices.empty())
	{
		l_sToken = &m_oMatrices.begin()->first;
	}
	return l_sToken;
}

const OpenViBE::CString* CMessageWithData::getNextCStringToken(const OpenViBE::CString &previousToken) const
{
	const CString* l_sKey = NULL;
	std::map<CString, CString >::const_iterator it = m_oStrings.find(previousToken);
	it++;
	if (it!=m_oStrings.end())
	{
		l_sKey = &it->first;
	}
	return l_sKey;
}

const OpenViBE::CString* CMessageWithData::getNextUInt64Token(const OpenViBE::CString &previousToken) const
{
	const CString* l_sKey = NULL;
	std::map<CString, uint64>::const_iterator it = m_oUint64s.find(previousToken);
	it++;
	if (it!=m_oUint64s.end())
	{
		l_sKey = &it->first;
	}
	return l_sKey;
}

const OpenViBE::CString* CMessageWithData::getNextFloat64Token(const OpenViBE::CString &previousToken) const
{
	const CString* l_sKey = NULL;
	std::map<CString, float64>::const_iterator it = m_oFloat64s.find(previousToken);
	it++;
	if (it!=m_oFloat64s.end())
	{
		l_sKey = &it->first;
	}
	return l_sKey;
}

const OpenViBE::CString* CMessageWithData::getNextIMatrixToken(const OpenViBE::CString &previousToken) const
{
	const CString* l_sKey = NULL;
	std::map<CString, IMatrix* >::const_iterator it = m_oMatrices.find(previousToken);
	it++;
	if (it!=m_oMatrices.end())
	{
		l_sKey = &it->first;
	}
	return l_sKey;
}
