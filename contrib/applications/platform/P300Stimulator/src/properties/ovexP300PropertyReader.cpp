#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300PropertyReader.h"
//#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <string>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;

void CoAdaptP300PropertyReader::readPropertiesFromFile(CString propertyFile)
{
	CMemoryBuffer l_PropertiesBuffer;
	ifstream l_oPropertiesStream(propertyFile.toASCIIString(), ios::binary);
	m_pKernelContext->getLogManager() << LogLevel_Info << "Reading configuration parameters from file " << propertyFile << "\n";

	if(l_oPropertiesStream.is_open())
	{
		int l_iFileLen;
		l_oPropertiesStream.seekg(0, ios::end);
		l_iFileLen=(int)l_oPropertiesStream.tellg();
		l_oPropertiesStream.seekg(0, ios::beg);

		l_PropertiesBuffer.setSize(l_iFileLen, true);//set size and discard true
		l_oPropertiesStream.read((char*)l_PropertiesBuffer.getDirectPointer(), l_iFileLen);
		l_oPropertiesStream.close();

		XML::IReader* l_pReader=XML::createReader(*this);
		l_pReader->processData(l_PropertiesBuffer.getDirectPointer(), l_PropertiesBuffer.getSize());
		l_pReader->release();
		l_pReader=NULL;
	}			
	else
	{
		m_pKernelContext->getLogManager() << LogLevel_Error << "Could not open configuration file\n";
	}
}

void CoAdaptP300PropertyReader::writeAttribute(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount)
{
	m_pKernelContext->getLogManager() << LogLevel_Trace << "Reading property " << sName;

	for (uint32 it=0; it<ui64AttributeCount; it++)
	{
		CString l_sExpandedString = m_pKernelContext->getConfigurationManager().expand(CString(*(sAttributeValue+it)));
		m_pKernelContext->getLogManager() << " with attribute " << *(sAttributeName+it) << "=" << l_sExpandedString << ",";	
	}
	m_pKernelContext->getLogManager() << "\n";
}

void CoAdaptP300PropertyReader::writeElement(const char* sName, const char* sData)
{
	std::string s(sData);
	s.erase(s.find_last_not_of(" \n\r\t")+1);
	if(s.length()!=0)
	{
		CString l_sExpandedString = m_pKernelContext->getConfigurationManager().expand(CString(sData));
		m_pKernelContext->getLogManager() << LogLevel_Trace << "	--> value: " << l_sExpandedString << "\n";
	}
	else
		m_pKernelContext->getLogManager() << LogLevel_Trace << "End property " << sName << "\n";
}
//#endif

#endif
