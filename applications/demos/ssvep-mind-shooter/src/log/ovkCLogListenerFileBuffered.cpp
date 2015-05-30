
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovkCLogListenerFileBuffered.h"

#include <cstdio>
#include <sstream>
#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;


CLogListenerFileBuffered::CLogListenerFileBuffered(const IKernelContext& rKernelContext, const CString& sApplicationName, const CString& sLogFilename)
	: m_sApplicationName(sApplicationName)
	,m_sLogFilename(sLogFilename)
{
	m_f = fopen(m_sLogFilename.toASCIIString(), "wt");
	if (m_f == NULL)
	{
		std::cout << "[  ERROR  ] This log listener can not be initialized, it will do nothing!" << std::endl;
	}
}

CLogListenerFileBuffered::~CLogListenerFileBuffered()
{
	if (m_f != NULL)
		fclose(m_f);
}

boolean CLogListenerFileBuffered::isActive(ELogLevel eLogLevel)
{
	map<ELogLevel, boolean>::iterator itLogLevel=m_vActiveLevel.find(eLogLevel);
	if(itLogLevel==m_vActiveLevel.end())
	{
		return true;
	}
	return itLogLevel->second;
}

boolean CLogListenerFileBuffered::activate(ELogLevel eLogLevel, boolean bActive)
{
	m_vActiveLevel[eLogLevel]=bActive;
	return true;
}

boolean CLogListenerFileBuffered::activate(ELogLevel eStartLogLevel, ELogLevel eEndLogLevel, boolean bActive)
{
	for(int i=eStartLogLevel; i<=eEndLogLevel; i++)
	{
		m_vActiveLevel[ELogLevel(i)]=bActive;
	}
	return true;
}

boolean CLogListenerFileBuffered::activate(boolean bActive)
{
	return activate(LogLevel_First, LogLevel_Last, bActive);
}

void CLogListenerFileBuffered::log(const time64 time64Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%llu", time64Value.m_ui64TimeValue);
}

void CLogListenerFileBuffered::log(const uint64 ui64Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%llu", ui64Value);
}

void CLogListenerFileBuffered::log(const uint32 ui32Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%u", ui32Value);
}

void CLogListenerFileBuffered::log(const uint16 ui16Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%u", ui16Value);
}

void CLogListenerFileBuffered::log(const uint8 ui8Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%u", ui8Value);
}

void CLogListenerFileBuffered::log(const int64 i64Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%lli", i64Value);
}

void CLogListenerFileBuffered::log(const int32 i32Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%i", i32Value);
}

void CLogListenerFileBuffered::log(const int16 i16Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%i", i16Value);
}

void CLogListenerFileBuffered::log(const int8 i8Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%i", i8Value);
}

void CLogListenerFileBuffered::log(const float32 f32Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%f", f32Value);
}

void CLogListenerFileBuffered::log(const float64 f64Value)
{
	if (m_f != NULL)
		fprintf(m_f, "%lf", f64Value);
}

void CLogListenerFileBuffered::log(const boolean bValue)
{
	if (m_f != NULL)
		fprintf(m_f, "%s", (bValue?"true":"false"));
}

void CLogListenerFileBuffered::log(const CIdentifier& rValue)
{
	if (m_f != NULL) 
	{
		CString l_sValue=rValue.toString();
		fprintf(m_f, "%s", (const char*)l_sValue);
	}
}

void CLogListenerFileBuffered::log(const CString& rValue)
{
	if (m_f != NULL)
		fprintf(m_f, "%s", (const char*)rValue);
}

void CLogListenerFileBuffered::log(const char* pValue)
{
	if (m_f != NULL)
		fprintf(m_f, "%s", pValue);
}

void CLogListenerFileBuffered::log(const ELogLevel eLogLevel)
{
	if (m_f != NULL)
	{
		switch(eLogLevel)
		{
		case LogLevel_Debug:
			fprintf(m_f, "[ DEBUG ] ");
			break;

		case LogLevel_Benchmark:
			fprintf(m_f, "[ BENCH ] ");
			break;

		case LogLevel_Trace:
			fprintf(m_f, "[ TRACE ] ");
			break;

		case LogLevel_Info:
			fprintf(m_f, "[  INF  ] ");
			break;

		case LogLevel_Warning:
			fprintf(m_f, "[WARNING] ");
			break;

		case LogLevel_ImportantWarning:
			fprintf(m_f, "[WARNING] ");
			break;

		case LogLevel_Error:
			fprintf(m_f, "[ ERROR ] ");
			break;

		case LogLevel_Fatal:
			fprintf(m_f, "[ FATAL ] ");
			break;

		default:
			fprintf(m_f, "[UNKNOWN] ");
			break;
		}
	}
}

void CLogListenerFileBuffered::log(const ELogColor eLogColor)
{
}

#endif