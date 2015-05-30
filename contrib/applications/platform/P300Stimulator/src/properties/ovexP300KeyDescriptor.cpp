#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300KeyDescriptor.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

NoSuchEventException noSuchEventException;

void P300KeyDescriptor::addScaleSize(const VisualState event, const OpenViBE::float32& value) 
{ 
	m_vEventMapScaleSize->insert(std::pair<VisualState, OpenViBE::float32>(event, value));
}
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
void P300KeyDescriptor::addForegroundColor(const VisualState event, const GColor& value) 
{ 
	m_vEventMapForegroundColor->insert(std::pair<VisualState, GColor>(event, value));
}
void P300KeyDescriptor::addBackgroundColor(const VisualState event, const GColor& value) 
{ 
	m_vEventMapBackgroundColor->insert(std::pair<VisualState, GColor>(event, value));
}
#endif
void P300KeyDescriptor::addSource(const VisualState event, const OpenViBE::CString& value) 
{ 
	m_vEventMapSource->insert(std::pair<VisualState, OpenViBE::CString>(event, value));
}
void P300KeyDescriptor::addLabel(const VisualState event, const std::string& value) 
{ 
	//overwrites the value for a key that is already there
	if (m_vEventMapLabel->find(event)!=m_vEventMapLabel->end())
		m_vEventMapLabel->erase(event);
	m_vEventMapLabel->insert(std::pair<VisualState, std::string>(event, value));
}
void P300KeyDescriptor::setIfTextSymbol(const VisualState event, const OpenViBE::boolean value) 
{ 
	m_vEventMapIsTextSymbol->insert(std::pair<VisualState, OpenViBE::boolean>(event, value));
}

const boolean P300KeyDescriptor::isActionEnabled(CString action)
{
	boolean l_bInList = false;
	for (uint32 i=0; i<m_vActions->size(); i++)
	{
		if (m_vActions->at(i)==action)
		{
			l_bInList = true; 
			break;
		}
	}
	return l_bInList;
}

const float32 P300KeyDescriptor::getScaleSize(const VisualState event) const 
{ 
	std::map<VisualState, OpenViBE::float32>::iterator iterator = m_vEventMapScaleSize->find(event); 
	if (iterator!=m_vEventMapScaleSize->end())
		return iterator->second; 
	else
		throw noSuchEventException;
}
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
const GColor& P300KeyDescriptor::getForegroundColor(const VisualState event) const 
{ 
	std::map<VisualState, GColor>::iterator iterator = m_vEventMapForegroundColor->find(event);
	if (iterator!=m_vEventMapForegroundColor->end())
		return iterator->second; 
	else
		throw noSuchEventException;
}

const GColor& P300KeyDescriptor::getBackgroundColor(const VisualState event) const 
{ 
	std::map<VisualState, GColor>::iterator iterator = m_vEventMapBackgroundColor->find(event);
	if (iterator!=m_vEventMapBackgroundColor->end())
		return iterator->second; 
	else
		throw noSuchEventException;
}
#endif
const CString& P300KeyDescriptor::getSource(const VisualState event) const 
{ 
	std::map<VisualState, OpenViBE::CString>::iterator iterator = m_vEventMapSource->find(event);
	if (iterator!=m_vEventMapSource->end())
		return iterator->second; 
	else
		throw noSuchEventException;
}

const std::string& P300KeyDescriptor::getLabel(const VisualState event) const 
{ 
	std::map<VisualState, std::string>::iterator iterator = m_vEventMapLabel->find(event);
	if (iterator!=m_vEventMapLabel->end())
		return iterator->second; 
	else
		throw noSuchEventException;
}

const boolean P300KeyDescriptor::isTextSymbol(const VisualState event) const 
{ 
	std::map<VisualState, OpenViBE::boolean>::iterator iterator = m_vEventMapIsTextSymbol->find(event);
	if (iterator!=m_vEventMapIsTextSymbol->end())
		return iterator->second; 
	else
		throw noSuchEventException;
}

#endif
