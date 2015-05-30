
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300ScreenLayoutReader.h"
#include <stdexcept>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

CString P300ScreenLayoutReader::KeyEventStrings[5] = {"flash","noflash","wrong_feedback", "correct_feedback","target"};

P300ScreenLayoutReader::P300ScreenLayoutReader(OpenViBE::Kernel::IKernelContext* kernelContext) : CoAdaptP300PropertyReader(kernelContext) 
{
	m_lKeyList = new std::vector<P300KeyDescriptor*>();
	m_lSymbolList = new std::list<std::string>();
	m_mDefaultEventMapScaleSize = new std::map<OpenViBE::uint32, OpenViBE::float32>();
	#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
	m_mDefaultEventMapForegroundColor = new std::map<OpenViBE::uint32, GColor>();
	m_mDefaultEventMapBackgroundColor = new std::map<OpenViBE::uint32, GColor>();
	#endif
	m_mDefaultEventMapSource = new std::map<OpenViBE::uint32, OpenViBE::CString>();
	m_mDefaultEventMapLabel = new std::map<OpenViBE::uint32, std::string>();
	m_mDefaultIsTextSymbol = new std::map<OpenViBE::uint32, OpenViBE::boolean>(); 
	
	m_bDefaultKeyProperties = false;
	m_bEventElement = false;
	m_bScaleSize = false;
	m_bForegroundColor = false;
	m_bBackgroundColor = false;
	m_bLabel = false;
	m_bSource = false;
	m_bKeyboardIsGrid = false;
	
	EventStringMap[ KeyEventStrings[0] ] = FLASH;
	EventStringMap[ KeyEventStrings[1] ] = NOFLASH;
	EventStringMap[ KeyEventStrings[2] ] = CENTRAL_FEEDBACK_WRONG;
	EventStringMap[ KeyEventStrings[3] ] = CENTRAL_FEEDBACK_CORRECT;
	EventStringMap[ KeyEventStrings[4] ] = TARGET;
}

P300ScreenLayoutReader::~P300ScreenLayoutReader()
{
	std::vector<P300KeyDescriptor*>::iterator l_ListIterator = m_lKeyList->begin();
	for (; l_ListIterator!=m_lKeyList->end(); l_ListIterator++)
		delete *l_ListIterator;
	delete m_lKeyList;
	delete m_lSymbolList;
	delete m_mDefaultEventMapScaleSize;
	#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
	delete m_mDefaultEventMapForegroundColor;
	delete m_mDefaultEventMapBackgroundColor;
	#endif
	delete m_mDefaultEventMapSource;
	delete m_mDefaultEventMapLabel;
	delete m_mDefaultIsTextSymbol;
}

void P300ScreenLayoutReader::openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount)
{
	writeAttribute(sName, sAttributeName, sAttributeValue, ui64AttributeCount);
	
	m_vNode.push(sName);
	
	if (CString(sName)==CString("DefaultKeyProperties"))
		m_bDefaultKeyProperties = true;
	#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
	if (CString(sName)==CString("TargetArea"))
		parseDimensions(m_dTargetAreaDimensions, sAttributeName, sAttributeValue,ui64AttributeCount);	
	if (CString(sName)==CString("ResultArea"))
		parseDimensions(m_dResultAreaDimensions, sAttributeName, sAttributeValue,ui64AttributeCount);
	#endif
	if (CString(sName)==CString("PredictionArea"))
	{
		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		parseDimensions(m_dPredictionAreaDimensions, sAttributeName, sAttributeValue,ui64AttributeCount);	
		#endif
		for (uint32 it=0; it<ui64AttributeCount; it++)
		{
			uint32 l_ui32Value = static_cast<uint32>(m_pKernelContext->getConfigurationManager().expandAsUInteger(CString(*(sAttributeValue+it))));
			if (CString(*(sAttributeName+it))==CString("ncolumns"))
				m_ui32PredictionAreaColumns = l_ui32Value;		
			else if (CString(*(sAttributeName+it))==CString("nrows"))
				m_ui32PredictionAreaRows = l_ui32Value;
		}		
	}
	if (CString(sName)==CString("P300Keyboard"))
	{
		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		parseDimensions(m_dKeyboardDimensions, sAttributeName, sAttributeValue,ui64AttributeCount);
		#endif
		for (uint32 it=0; it<ui64AttributeCount; it++)
		{
			uint32 l_ui32Value = static_cast<uint32>(m_pKernelContext->getConfigurationManager().expandAsUInteger(CString(*(sAttributeValue+it))));
			if (CString(*(sAttributeName+it))==CString("numberOfStandardKeys"))
				m_ui32NumberOfStandardKeys = l_ui32Value;	
		}
	}
	
	if (CString(sName)==CString("Event"))
	{
		m_bEventElement = true;
		for (uint32 it=0; it<ui64AttributeCount; it++)
		{
			if (CString(*(sAttributeName+it))==CString("id"))
			{
				CString l_sAttributeValue = CString(*(sAttributeValue+it));
				if (EventStringMap.find(l_sAttributeValue)!=EventStringMap.end())
					m_iState = EventStringMap[l_sAttributeValue];
				else
					m_pKernelContext->getLogManager() << LogLevel_Warning << "Event id " << *(l_sAttributeValue+it) << " unknown\n";
			}
		}		
	}
	
	if (CString(sName)==CString("Key"))	
	{
		P300KeyDescriptor* l_pKey = new P300KeyDescriptor();
		m_pKey = l_pKey;
		m_lKeyList->push_back(l_pKey);
		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		BoxDimensions dim; dim.x = 0; dim.y = 0; dim.height = 0; dim.width=0; dim.depth=0;
		parseDimensions(dim, sAttributeName, sAttributeValue, ui64AttributeCount);
		if (dim.x==0 && dim.y==0 && dim.height==0 && dim.width==0)
			m_bKeyboardIsGrid = true;
		m_pKey->setDimensions(dim);
		#endif
	}	
	
	if (m_bDefaultKeyProperties && m_vNode.top()==CString("TextSymbol"))
	{
		m_mDefaultIsTextSymbol->insert(std::pair<VisualState,boolean>(m_iState,true));
	}
	if (m_bDefaultKeyProperties && m_vNode.top()==CString("PictureSymbol"))
	{
		m_mDefaultIsTextSymbol->insert(std::pair<VisualState,boolean>(m_iState,false));
	}		
	if (!m_bDefaultKeyProperties && m_vNode.top()==CString("TextSymbol"))
	{
		m_pKey->setIfTextSymbol(m_iState,true);
	}
	if (!m_bDefaultKeyProperties && m_vNode.top()==CString("PictureSymbol"))
	{
		m_pKey->setIfTextSymbol(m_iState, false);
	}		
}

void P300ScreenLayoutReader::processChildData(const char* sData)
{
	writeElement(m_vNode.top().toASCIIString(), sData);
	
	if (m_vNode.top()==CString("Action"))	
	{
		CString l_sAction = CString(sData);
		m_pKey->addAction(l_sAction);
	}
	if (m_bDefaultKeyProperties && m_bEventElement)
	{
		if (m_vNode.top()==CString("ScaleSize"))
		{	
			float32 l_f32AttributeValue = static_cast<float32>(m_pKernelContext->getConfigurationManager().expandAsFloat(CString(sData)));
			m_mDefaultEventMapScaleSize->insert(std::pair<VisualState,float32>(m_iState,l_f32AttributeValue));
		}
		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		if (m_vNode.top()==CString("ForegroundColor"))
			m_mDefaultEventMapForegroundColor->insert(std::pair<VisualState,GColor>(m_iState,_AutoCast_(sData)));
		if (m_vNode.top()==CString("BackgroundColor"))
			m_mDefaultEventMapBackgroundColor->insert(std::pair<VisualState,GColor>(m_iState,_AutoCast_(sData)));
		#endif
		if (m_vNode.top()==CString("Source"))
		{	
			CString l_sSource = OpenViBE::Directories::getDistRootDir() +"/share/"+CString(sData);
			m_mDefaultEventMapSource->insert(std::pair<VisualState,CString>(m_iState, l_sSource));
		}
		if (m_vNode.top()==CString("Label"))
		{	
			m_mDefaultEventMapLabel->insert(std::pair<VisualState,std::string>(m_iState, std::string(sData)));
		}		
	}
	else if (!m_bDefaultKeyProperties && m_bEventElement)
	{
		if (m_vNode.top()==CString("ScaleSize"))
		{	
			m_bScaleSize = true;
			float32 l_f32AttributeValue = static_cast<float32>(m_pKernelContext->getConfigurationManager().expandAsFloat(CString(sData)));
			m_pKey->addScaleSize(m_iState,l_f32AttributeValue);
		}
		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		if (m_vNode.top()==CString("ForegroundColor"))
		{
			m_bForegroundColor = true;
			m_pKey->addForegroundColor(m_iState,_AutoCast_(sData));	
		}
		if (m_vNode.top()==CString("BackgroundColor"))
		{
			m_bBackgroundColor = true;
			m_pKey->addBackgroundColor(m_iState,_AutoCast_(sData));
		}
		#endif
		if (m_vNode.top()==CString("Source"))
		{	
			m_bSource = true;
			CString l_sSource = OpenViBE::Directories::getDistRootDir() + "/share/"+CString(sData);
			m_pKey->addSource(m_iState,l_sSource);
		}
		if (m_vNode.top()==CString("Label"))
		{	
			m_bLabel = true;
			m_pKey->addLabel(m_iState,std::string(sData));
		}		
	}
}

void P300ScreenLayoutReader::closeChild(void)
{
	if (m_vNode.top()==CString("DefaultKeyProperties"))
		m_bDefaultKeyProperties = false;
	if (m_vNode.top()==CString("Event"))
		m_bEventElement = false;
	//fill in all the key properties for all the events with the default ones in case they are not specified
	if (m_vNode.top()==CString("Key"))
	{
		for (uint32 i=0; i<5; i++)
		{
			VisualState l_State = EventStringMap[ KeyEventStrings[i] ];
			try { m_pKey->getScaleSize(l_State); }
			catch (exception&)
			{
				m_pKey->addScaleSize(l_State,m_mDefaultEventMapScaleSize->at(l_State));
			}
			#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
			try { m_pKey->getForegroundColor(l_State); }
			catch (exception&)
			{
				m_pKey->addForegroundColor(l_State,m_mDefaultEventMapForegroundColor->at(l_State));
			}
			try { m_pKey->getBackgroundColor(l_State); }
			catch (exception&)
			{
				m_pKey->addBackgroundColor(l_State,m_mDefaultEventMapBackgroundColor->at(l_State));
			}
			#endif
			try { m_pKey->getSource(l_State); }
			catch (exception&)
			{
				m_pKey->addSource(l_State,m_mDefaultEventMapSource->at(l_State));
			}	
			try { m_pKey->getLabel(l_State); }
			catch (exception&)
			{
				if (m_mDefaultEventMapLabel->find(l_State)!=m_mDefaultEventMapLabel->end())
				{
					m_pKey->addLabel(l_State,m_mDefaultEventMapLabel->at(l_State));
				}
			}
			try { m_pKey->isTextSymbol(l_State); }
			catch (exception&)
			{
				if (m_mDefaultIsTextSymbol->find(l_State)!=m_mDefaultIsTextSymbol->end())
				{
					m_pKey->setIfTextSymbol(l_State,m_mDefaultIsTextSymbol->at(l_State));
				}
			}			
		}	
	}
	m_vNode.pop();
}

void P300ScreenLayoutReader::parseDimensions(BoxDimensions& dim, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount)
{
	for (uint32 it=0; it<ui64AttributeCount; it++)
	{
		float32 l_f32AttributeValue = static_cast<float32>(m_pKernelContext->getConfigurationManager().expandAsFloat(CString(*(sAttributeValue+it))));
		if (CString(*(sAttributeName+it))==CString("width"))
			dim.width = (l_f32AttributeValue);
		else if(CString(*(sAttributeName+it))==CString("height"))
			dim.height = (l_f32AttributeValue);
		else if(CString(*(sAttributeName+it))==CString("x"))
			dim.x = (l_f32AttributeValue);
		else if(CString(*(sAttributeName+it))==CString("y"))	
			dim.y = (l_f32AttributeValue);
	}		
}

void P300ScreenLayoutReader::readPropertiesFromFile(CString propertyFile)
{
	CoAdaptP300PropertyReader::readPropertiesFromFile(propertyFile);
	for (uint32 i=0; i<m_lKeyList->size(); i++)
	{
		try
		{
			m_lSymbolList->push_back(m_lKeyList->at(i)->getLabel(NOFLASH));
		}
		catch (exception& )
		{
			m_lSymbolList->push_back(std::string("DUMMY"));
		}
	}
}


#endif
