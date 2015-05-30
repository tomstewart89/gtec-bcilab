#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "ebml/IReader.h"
#include "ebml/IReaderHelper.h"
#include "ebml/CReader.h"
#include "ebml/CReaderHelper.h"

using namespace std;

std::ostream* g_pOutput = &std::cout;

class CReaderCallBack : public EBML::IReaderCallBack
{
public:
	CReaderCallBack(void)
		:m_iDepth(0)
	{
	}

	virtual ~CReaderCallBack(void)
	{
	}

	virtual bool isMasterChild(const EBML::CIdentifier& rIdentifier)
	{
		if(rIdentifier==EBML_Identifier_Header) return true;
		if(rIdentifier==EBML::CIdentifier(0xffff)) return true;

		return false;
	}

	virtual void openChild(const EBML::CIdentifier& rIdentifier)
	{
		m_oCurrentIdentifier=rIdentifier;

		for(int i=0; i<m_iDepth; i++) *g_pOutput << "   ";
		*g_pOutput << "Opening child node [0x" << setw(16) << setfill('0') << hex << m_oCurrentIdentifier << dec << "]\n";
		m_iDepth++;
	}

	virtual void processChildData(const void* pBuffer, const EBML::uint64 ui64BufferSize)
	{
		for(int i=0; i<m_iDepth; i++) *g_pOutput << "   ";
		if(m_oCurrentIdentifier==EBML_Identifier_DocType)
			*g_pOutput << "Got doc type : [" << m_oReaderHelper.getASCIIStringFromChildData(pBuffer, ui64BufferSize) << "]\n";
		else if(m_oCurrentIdentifier==EBML_Identifier_EBMLVersion)
			*g_pOutput << "Got EBML version : [0x" << setw(16) << setfill('0') << hex << m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize) << dec << "]\n";
		else if(m_oCurrentIdentifier==EBML_Identifier_EBMLIdLength)
			*g_pOutput << "Got EBML ID length : [0x" << setw(16) << setfill('0') << hex << m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize) << dec << "]\n";
		else if(m_oCurrentIdentifier==EBML_Identifier_DocTypeVersion)
			*g_pOutput << "Got doc type version : [0x" << setw(16) << setfill('0') << hex << m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize) << dec << "]\n";
		else if(m_oCurrentIdentifier==EBML_Identifier_DocTypeReadVersion)
			*g_pOutput <<"Got doc type read version : [0x" << setw(16) << setfill('0') << hex << m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize) << dec << "]\n";
		else if(m_oCurrentIdentifier==EBML::CIdentifier(0x1234))
			*g_pOutput <<"Got uinteger : [0x" << setw(16) << setfill('0') << hex << m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize) << dec << "]\n";
		else if(m_oCurrentIdentifier==EBML::CIdentifier(0xffffffffffffffffLL))
			*g_pOutput <<"Got uinteger : [0x" << setw(16) << setfill('0') << hex << m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize) << dec << "]\n";
		else if(m_oCurrentIdentifier==EBML::CIdentifier(0x4321))
			*g_pOutput <<"Got float64 : [" << m_oReaderHelper.getFloatFromChildData(pBuffer, ui64BufferSize) << "]\n";
		else if(m_oCurrentIdentifier==EBML::CIdentifier(0x8765))
			*g_pOutput <<"Got float32 : [" << m_oReaderHelper.getFloatFromChildData(pBuffer, ui64BufferSize) << "]\n";
		else
			*g_pOutput << "Got " << ui64BufferSize << " data bytes, node id not known\n";
	}

	virtual void closeChild(void)
	{
		m_iDepth--;
		for(int i=0; i<m_iDepth; i++) *g_pOutput << "   ";
		*g_pOutput << "Node closed\n";
	}

	int m_iDepth;
	EBML::CReaderHelper m_oReaderHelper;
	EBML::CIdentifier m_oCurrentIdentifier;
};

int main(int argc, char** argv)
{
	srand(static_cast<unsigned int>(time(NULL)));

	if(argc<2)
	{
		*g_pOutput << "syntax : " << argv[0] << " filein.ebml [output]\n";
		return -1;
	}
	if(argc==3)
	{
		g_pOutput = new std::ofstream(argv[2], ios::trunc);
		if(!g_pOutput || !g_pOutput->good() || g_pOutput->bad())
		{
			cout << "Unable to open " << argv[2] << " for writing\n";
			return 1;
		}
	}

	for(unsigned long n=17; n>=1; n--)
	//unsigned long n=(rand()&0xf) + 1;
	{
		CReaderCallBack cb;
		EBML::CReader l_oReader(cb);

		*g_pOutput << "testing with n=" << n << endl;;
		FILE* f=fopen(argv[1], "rb");
		if(!f) {
			cout << "Unable to open " << argv[1] << " for reading\n";
			return 2;
		}
		unsigned char* c=new unsigned char[n];
		int i=0;
		while(!feof(f))
		{
			i=fread(c, 1, n*sizeof(unsigned char), f);
			// *g_pOutput << " --> reader has read " << dec << i << " bytes\n";
			l_oReader.processData(c, i);
		}
		delete [] c;
		fclose(f);
	}

	if(argc==3) {
		delete g_pOutput;
	}

	return 0;
}
