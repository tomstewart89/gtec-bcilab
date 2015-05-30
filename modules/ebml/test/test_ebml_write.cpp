#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "ebml/defines.h"
#include "ebml/IWriter.h"
#include "ebml/IWriterHelper.h"
#include "ebml/CWriterHelper.h"

#define OVP_NodeId_OpenViBEStream_Header              EBML::CIdentifier(0xF59505AB, 0x3684C8D8)
#define OVP_NodeId_OpenViBEStream_Header_Compression  EBML::CIdentifier(0x40358769, 0x166380D1)
#define OVP_NodeId_OpenViBEStream_Header_StreamType  EBML::CIdentifier(0x732EC1D1, 0xFE904087)
#define OVP_NodeId_OpenViBEStream_Buffer              EBML::CIdentifier(0x2E60AD18, 0x87A29BDF)
#define OVP_NodeId_OpenViBEStream_Buffer_StreamIndex EBML::CIdentifier(0x30A56D8A, 0xB9C12238)
#define OVP_NodeId_OpenViBEStream_Buffer_StartTime    EBML::CIdentifier(0x093E6A0A, 0xC5A9467B)
#define OVP_NodeId_OpenViBEStream_Buffer_EndTime      EBML::CIdentifier(0x8B5CCCD9, 0xC5024F29)
#define OVP_NodeId_OpenViBEStream_Buffer_Content      EBML::CIdentifier(0x8D4B0BE8, 0x7051265C)

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

class CWriterCallBack : public EBML::IWriterCallBack
{
public:
	CWriterCallBack(char* filename)
	{
		f=fopen(filename, "wb");
	}

	virtual ~CWriterCallBack(void)
	{
		if(f) fclose(f);
	}

	virtual void write(const void* pBuffer, const EBML::uint64 ui64BufferSize)
	{
		if(f) fwrite(pBuffer, static_cast<size_t>(ui64BufferSize), 1, f);
	}

	FILE* f;
};

int main(int argc, char** argv)
{
	if(argc<2)
	{
		printf("syntax : %s fileout.ebml\n", argv[0]);
		return -1;
	}

	CWriterCallBack cb(argv[1]);
	EBML::IWriter* l_pWriter=EBML::createWriter(cb);
	EBML::CWriterHelper l_oWriterHelper;

	l_oWriterHelper.connect(l_pWriter);

	l_oWriterHelper.openChild(EBML_Identifier_Header);

		l_oWriterHelper.openChild(EBML_Identifier_DocType);
		 l_oWriterHelper.setASCIIStringAsChildData("matroska");
		l_oWriterHelper.closeChild();

		l_oWriterHelper.openChild(EBML_Identifier_DocTypeVersion);
		 l_oWriterHelper.setUIntegerAsChildData(1);
		l_oWriterHelper.closeChild();

		l_oWriterHelper.openChild(EBML_Identifier_DocTypeReadVersion);
		 l_oWriterHelper.setSIntegerAsChildData(655356);
		l_oWriterHelper.closeChild();

	l_oWriterHelper.closeChild();

	l_oWriterHelper.openChild(0x1234);
	 l_oWriterHelper.setUIntegerAsChildData(0);
	l_oWriterHelper.closeChild();

	l_oWriterHelper.openChild(0xffffffffffffffffLL);
	 l_oWriterHelper.setUIntegerAsChildData(0xff000000ff000000LL);
	l_oWriterHelper.closeChild();

	l_oWriterHelper.openChild(0x4321);
	 l_oWriterHelper.setFloat64AsChildData(M_PI);
	l_oWriterHelper.closeChild();

	l_oWriterHelper.openChild(0x8765);
	 l_oWriterHelper.setFloat32AsChildData(static_cast<EBML::float32>(M_PI));
	l_oWriterHelper.closeChild();

	l_pWriter->release();

	return 0;
}
