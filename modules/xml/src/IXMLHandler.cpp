#include "IXMLHandler.h"

#include <expat.h>
#include <cstring>
#include <stack>
#include <fstream>
#include <iostream>

namespace XML
{
	class IXMLHandlerImpl: public IXMLHandler
	{
	public:
		virtual void release(void);
		IXMLHandlerImpl();

		//Parsing
		virtual XML::IXMLNode* parseFile(const char* sPath);
		virtual XML::IXMLNode* parseString(const char* sString, const uint32& uiSize);

		//XML extraction
		virtual XML::boolean writeXMLInFile(const IXMLNode &rNode, const char* sPath) const;

		//Internal function for parsing
		virtual void openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, uint64 ui64AttributeCount);
		virtual void processChildData(const char* sData);
		virtual void closeChild(void);

	protected:
		virtual ~IXMLHandlerImpl();

	private:
		::XML_Parser m_pXMLParser;
		std::stack<XML::IXMLNode *> m_oNodeStack;
		XML::IXMLNode* m_pRootNode;
	};

	//Callback for expat
	static void XMLCALL expat_xml_start(void* pData, const char* pElement, const char** ppAttribute);
	static void XMLCALL expat_xml_end(void* pData, const char* pElement);
	static void XMLCALL expat_xml_data(void* pData, const char* pDataValue, int iDataLength);
}

using namespace std;
using namespace XML;


IXMLHandlerImpl::~IXMLHandlerImpl()
{
	XML_ParserFree(m_pXMLParser);
	while(!m_oNodeStack.empty())
	{
		IXMLNode * l_pNode = m_oNodeStack.top();
		l_pNode->release();
		m_oNodeStack.pop();
	}
}

void IXMLHandlerImpl::release(void)
{
	delete this;
}


IXMLHandlerImpl::IXMLHandlerImpl():
	m_pXMLParser(NULL),
	m_pRootNode(NULL)
{
	m_pXMLParser=XML_ParserCreate(NULL);
	XML_SetElementHandler(m_pXMLParser, expat_xml_start, expat_xml_end);
	XML_SetCharacterDataHandler(m_pXMLParser, expat_xml_data);
	XML_SetUserData(m_pXMLParser, this);
}

IXMLNode *IXMLHandlerImpl::parseFile(const char *sPath)
{
	ifstream l_oFile(sPath, ios::binary);
	if(l_oFile.is_open())
	{
		char* l_sBuffer;
		size_t l_iFileLen;

		//Compute size
		l_oFile.seekg(0, ios::end);
		l_iFileLen=(size_t)l_oFile.tellg();
		l_oFile.seekg(0, ios::beg);

		//Read the file
		l_sBuffer = new char[l_iFileLen];
		l_oFile.read(l_sBuffer, l_iFileLen);
		l_oFile.close();

		//Start the parsing with the other function
		IXMLNode *l_pRes = parseString(l_sBuffer, l_iFileLen);

		delete[] l_sBuffer;
		return l_pRes;
	}
	std::cout << "Error : unable to open the file" << sPath << "." << std::endl;
	return NULL;
}

IXMLNode *IXMLHandlerImpl::parseString(const char *sString, const uint32& uiSize)
{
	m_pRootNode = NULL;
	XML_Status l_eStatus=XML_Parse(m_pXMLParser, sString, uiSize, false);

	//We delete what is still in the stack
	while(!m_oNodeStack.empty())
	{
		std::cout << "Warning : the file has been parsed but some tags are not closed. The file is probably not well-formed." << std::endl;
		IXMLNode * l_pNode = m_oNodeStack.top();
		l_pNode->release();
		m_oNodeStack.pop();
	}
	if(l_eStatus!=XML_STATUS_OK) {
		XML_Error l_oErrorCode = XML_GetErrorCode(m_pXMLParser);
		// Although printf() is not too elegant, this component has no context to use e.g. LogManager -> printf() is better than a silent fail.
		std::cout << "processData(): expat error " << l_oErrorCode << " on the line " << XML_GetCurrentLineNumber(m_pXMLParser) << " of the input .xml\n";
		return NULL;
	}
	return m_pRootNode;
}

boolean IXMLHandlerImpl::writeXMLInFile(const IXMLNode &rNode, const char *sPath) const
{
	std::ofstream l_oFile(sPath, ios::binary);
	if(l_oFile.is_open())
	{
		char* l_sXML = rNode.getXML();
		l_oFile.write(l_sXML, ::strlen(l_sXML));
		l_oFile.close();
		free(l_sXML);
		return true;
	}
	std::cout << "Error : unable to open the file " << sPath << "." << std::endl;
	return false;
}

void IXMLHandlerImpl::openChild(const char *sName, const char **sAttributeName, const char **sAttributeValue, uint64 ui64AttributeCount)
{
	IXMLNode * l_pNode = createNode(sName);
	for(uint32 i =0 ; i < ui64AttributeCount ; ++i)
	{
		l_pNode->addAttribute(sAttributeName[i], sAttributeValue[i]);
	}
	m_oNodeStack.push(l_pNode);
}

void IXMLHandlerImpl::processChildData(const char *sData)
{
	IXMLNode * l_pNode = m_oNodeStack.top();
	l_pNode->setPCData(sData);
}

void IXMLHandlerImpl::closeChild()
{
	IXMLNode * l_pNode = m_oNodeStack.top();
	m_oNodeStack.pop();
	//If the stack is empty this means that l_pNode is the root
	if(m_oNodeStack.empty())
	{
		m_pRootNode = l_pNode;
	}
	else{//If node, that means that the l_pNode if
		IXMLNode *l_pParentNode = m_oNodeStack.top();
		l_pParentNode->addChild(l_pNode);
	}

}



static void XMLCALL XML::expat_xml_start(void* pData, const char* pElement, const char** ppAttribute)
{
	uint64 l_ui64AttributeCount=0;
	while(ppAttribute[l_ui64AttributeCount++]);
	l_ui64AttributeCount>>=1;

	// $$$ TODO take 64bits size into consideration
	const char** l_pAttributeName=new const char*[static_cast<size_t>(l_ui64AttributeCount)];
	const char** l_pAttributeValue=new const char*[static_cast<size_t>(l_ui64AttributeCount)];

	for(uint64 i=0; i<l_ui64AttributeCount; i++)
	{
		l_pAttributeName[i]=ppAttribute[(i<<1)];
		l_pAttributeValue[i]=ppAttribute[(i<<1)+1];
	}

	static_cast<IXMLHandlerImpl*>(pData)->openChild(pElement, l_pAttributeName, l_pAttributeValue, l_ui64AttributeCount);

	delete [] l_pAttributeName;
	delete [] l_pAttributeValue;
}

static void XMLCALL XML::expat_xml_end(void* pData, const char* pElement)
{
	static_cast<IXMLHandlerImpl*>(pData)->closeChild();
}

static void XMLCALL XML::expat_xml_data(void* pData, const char* pDataValue, int iDataLength)
{
	string sData(pDataValue, iDataLength);
	static_cast<IXMLHandlerImpl*>(pData)->processChildData(sData.c_str());
}

OV_API IXMLHandler* XML::createXMLHandler(void)
{
	return new IXMLHandlerImpl();
}

