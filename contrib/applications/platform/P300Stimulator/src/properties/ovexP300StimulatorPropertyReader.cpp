#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300StimulatorPropertyReader.h"
//#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

void P300StimulatorPropertyReader::openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount)
{
	writeAttribute(sName, sAttributeName, sAttributeValue, ui64AttributeCount);
	
	m_vNode.push(sName);
}

void P300StimulatorPropertyReader::processChildData(const char* sData)
{
	writeElement(m_vNode.top().toASCIIString(), sData);
	CString l_sExpandedValue = m_pKernelContext->getConfigurationManager().expand(CString(sData));

	if(m_vNode.top()==CString("BaseStimulationGroup1"))
	{
		m_ui64StimulationBase = strtoul(sData,NULL,0);
	}
	if(m_vNode.top()==CString("InitialNumberOfGroups"))
	{
		m_ui32InitialNumberOfGroups = static_cast<uint32>(m_pKernelContext->getConfigurationManager().expandAsUInteger(l_sExpandedValue));
	}
	if(m_vNode.top()==CString("MinRepetitions"))
	{
		m_ui32MinRepetitions = static_cast<uint32>(m_pKernelContext->getConfigurationManager().expandAsUInteger(l_sExpandedValue));
	}
	if(m_vNode.top()==CString("MaxRepetitions"))
	{
		m_ui32RepetitionCountInTrial = static_cast<uint32>(m_pKernelContext->getConfigurationManager().expandAsUInteger(l_sExpandedValue));
		if(m_ui32RepetitionCountInTrial<=m_ui32MinRepetitions)
		{
			m_pKernelContext->getLogManager() << LogLevel_Info << "Tried to set MaxRepetitions to " << m_ui32RepetitionCountInTrial << " when min repetitions is"
											  << m_ui32MinRepetitions << " MaxRepetitions will be " << m_ui32MinRepetitions+1 << "\n";
			m_ui32RepetitionCountInTrial=m_ui32MinRepetitions+1;
		}
	}
	if(m_vNode.top()==CString("NumberOfTrials"))
	{
		m_ui32TrialCount = static_cast<uint32>(m_pKernelContext->getConfigurationManager().expandAsUInteger(l_sExpandedValue));
	}
	if(m_vNode.top()==CString("FlashDuration"))
	{
		m_ui64FlashDuration = _AutoCast_(l_sExpandedValue.toASCIIString());
	}
	if(m_vNode.top()==CString("InterStimulusOnset"))
	{
		m_ui64InterStimulusOnset = _AutoCast_(l_sExpandedValue.toASCIIString());
	}

	if(m_vNode.top()==CString("InterRepetitionDelay"))
	{
		m_ui64InterRepetitionDuration = _AutoCast_(l_sExpandedValue.toASCIIString());
	}
	if(m_vNode.top()==CString("InterTrialDelay"))
	{
		uint64 l_ui64InterTrialDelay = _AutoCast_(sData);
		m_pKernelContext->getLogManager() << LogLevel_Warning << "Inter trial delay is " << l_ui64InterTrialDelay << "\n";
		if (l_ui64InterTrialDelay>= ITimeArithmetics::sampleCountToTime(1, 4LL))//(4LL<<32))
			m_ui64InterTrialDuration = l_ui64InterTrialDelay;
		else
		{
			m_pKernelContext->getLogManager() << LogLevel_Warning << "Inter trial delay should be bigger than 4 seconds; setting it to 4 seconds\n";
			m_ui64InterTrialDuration = ITimeArithmetics::sampleCountToTime(1, 4LL);
		}
	}
	if(m_vNode.top()==CString("TargetWord"))
	{
		m_sTargetWord = l_sExpandedValue.toASCIIString();
	}
	if(m_vNode.top()==CString("SharedMemoryName"))
	{
		m_sSharedMemoryName = CString(sData);
	}
	if(m_vNode.top()==CString("EarlyStopping"))
	{
		m_bEarlyStopping = false;
		if(l_sExpandedValue==CString("True"))
				m_bEarlyStopping=true;
	}
	if(m_vNode.top()==CString("StopCondition"))
	{
		m_f64StopCondition = ::atof(l_sExpandedValue.toASCIIString());
	}
}

void P300StimulatorPropertyReader::closeChild(void)
{
	if(m_vNode.top()==CString("ExternelP300Stimulator"))
	{
		for (uint32 i=0; i<m_sTargetWord.size(); i++)
			m_oTargetStimuli->push(findSymbolIndex(m_sTargetWord.substr(i,1)));
	}
		
	m_vNode.pop();
}

uint64 P300StimulatorPropertyReader::findSymbolIndex(std::string symbol)
{
	list<string>::iterator l_Iterator = m_lSymbolList->begin();
	uint64 l_ui64Counter = 0;
	for(;l_Iterator!=m_lSymbolList->end(); l_Iterator++, l_ui64Counter++)
		if(*l_Iterator==symbol)
			return l_ui64Counter+1;

	return 0;
}
//#endif

#endif
