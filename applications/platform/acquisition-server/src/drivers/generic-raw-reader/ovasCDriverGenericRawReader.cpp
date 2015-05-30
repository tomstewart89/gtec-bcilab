/*
 * The raw reader expects the data to be formatted as follows
 *
 * [START][BLOCK0][BLOCK1][BLOCK2]...
 *  skip   parse   parse   parse  ...
 *
 * where each block is          [===========BLOCKX=================]
 *                  is read as  [===========dataFrameSize==========]
 *                  breaks to   [header====][sample====][footer====]
 *                  equals      [headerSize][sampleSize][footerSize]
 *                  means          skip        keep         skip
 *
 * For correct parsing, user must provide the exact sizes of the skipped parts "start", "header" and "footer" in bytes. 
 *
 */

#include "ovasCDriverGenericRawReader.h"
#include "../ovasCConfigurationBuilder.h"

#include <toolkit/ovtk_all.h>
#include <openvibe/ovITimeArithmetics.h>

#include <system/ovCMemory.h>
#include <system/ovCTime.h>

#include <cmath>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;

// #define OPENVIBE_DEBUG_RAW_READER

namespace
{
	template <class T>
	float32 decode_little_endian(const uint8* pBuffer)
	{
		T t;
		System::Memory::littleEndianToHost(pBuffer, &t);
		return float32(t);
	}

	template <class T>
	float32 decode_big_endian(const uint8* pBuffer)
	{
		T t;
		System::Memory::bigEndianToHost(pBuffer, &t);
		return float32(t);
	}
};

//___________________________________________________________________//
//                                                                   //

CDriverGenericRawReader::CDriverGenericRawReader(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,m_ui32SampleFormat(Format_SignedInteger32)
	,m_ui32SampleEndian(Endian_Little)
	,m_ui32StartSkip(0)
	,m_ui32HeaderSkip(0)
	,m_ui32FooterSkip(20)
	,m_bLimitSpeed(false)
	,m_pDataFrame(NULL)
	,m_pSample(NULL)
{
	m_oHeader.setSamplingFrequency(512);
	m_oHeader.setChannelCount(16);
}

void CDriverGenericRawReader::release(void)
{
	delete this;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverGenericRawReader::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) { return false; }

	switch(m_ui32SampleFormat)
	{
		case Format_UnsignedInteger8:
		case Format_SignedInteger8:
			m_ui32SampleSize=1;
			break;
		case Format_UnsignedInteger16:
		case Format_SignedInteger16:
			m_ui32SampleSize=2;
			break;
		case Format_UnsignedInteger32:
		case Format_SignedInteger32:
		case Format_Float32:
			m_ui32SampleSize=4;
			break;
		case Format_Float64:
			m_ui32SampleSize=8;
			break;
		default:
			m_rDriverContext.getLogManager() << LogLevel_Error << "Unsupported data format " << m_ui32SampleFormat << "\n";
			return false;
	}

	m_ui32DataFrameSize=m_ui32SampleSize*m_oHeader.getChannelCount();
	m_ui32DataFrameSize+=m_ui32HeaderSkip;
	m_ui32DataFrameSize+=m_ui32FooterSkip;

	m_pSample=new float32[m_oHeader.getChannelCount()];
	m_pDataFrame=new uint8[m_ui32DataFrameSize];
	if(!m_pSample || !m_pDataFrame)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Could not allocate memory !\n";
		return false;
	}

	// open() should skip m_ui32StartSkip worth of bytes already
	if(!this->open())
	{
		return false;
	}

	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;

	return true;
}

boolean CDriverGenericRawReader::start(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }
	m_ui64TotalSampleCount=0;
	m_ui64StartTime=System::Time::zgetTime();
	return true;
}

boolean CDriverGenericRawReader::loop(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	// if(!m_rDriverContext.isStarted()) { return true; }

	uint64 l_ui64SampleTime = ITimeArithmetics::sampleCountToTime(m_oHeader.getSamplingFrequency(), m_ui64TotalSampleCount);

	if(m_bLimitSpeed && (l_ui64SampleTime > System::Time::zgetTime() - m_ui64StartTime))
	{
		return true;
	}

#ifdef OPENVIBE_DEBUG_RAW_READER
	m_rDriverContext.getLogManager() << LogLevel_Info << "Decoded : ";
#endif

	for(uint32 j=0; j<m_ui32SampleCountPerSentBlock; j++)
	{
		if(!this->read())
		{
			return false;
		}

		for(uint32 i=0; i<m_oHeader.getChannelCount(); i++)
		{
			uint8* l_pDataFrame=m_pDataFrame+m_ui32HeaderSkip+i*m_ui32SampleSize;
			switch(m_ui32SampleEndian)
			{
				case Endian_Little:
					switch(m_ui32SampleFormat)
					{
						case Format_UnsignedInteger8:  m_pSample[i]=*l_pDataFrame; break;
						case Format_UnsignedInteger16: m_pSample[i]=decode_little_endian<uint16>(l_pDataFrame); break;
						case Format_UnsignedInteger32: m_pSample[i]=decode_little_endian<uint32>(l_pDataFrame); break;
						case Format_SignedInteger8:    m_pSample[i]=*l_pDataFrame; break;
						case Format_SignedInteger16:   m_pSample[i]=decode_little_endian<int16>(l_pDataFrame); break;
						case Format_SignedInteger32:   m_pSample[i]=decode_little_endian<int32>(l_pDataFrame); break;
						case Format_Float32:           m_pSample[i]=decode_little_endian<float32>(l_pDataFrame); break;
						case Format_Float64:           m_pSample[i]=decode_little_endian<float64>(l_pDataFrame); break;
						default: break;
					}
					break;

				case Endian_Big:
					switch(m_ui32SampleFormat)
					{
						case Format_UnsignedInteger8:  m_pSample[i]=*l_pDataFrame; break;
						case Format_UnsignedInteger16: m_pSample[i]=decode_big_endian<uint16>(l_pDataFrame); break;
						case Format_UnsignedInteger32: m_pSample[i]=decode_big_endian<uint32>(l_pDataFrame); break;
						case Format_SignedInteger8:    m_pSample[i]=*l_pDataFrame; break;
						case Format_SignedInteger16:   m_pSample[i]=decode_big_endian<int16>(l_pDataFrame); break;
						case Format_SignedInteger32:   m_pSample[i]=decode_big_endian<int32>(l_pDataFrame); break;
						case Format_Float32:           m_pSample[i]=decode_big_endian<float32>(l_pDataFrame); break;
						case Format_Float64:           m_pSample[i]=decode_big_endian<float64>(l_pDataFrame); break;
						default: break;
					}
					break;
				default:
					break;
			}
#ifdef OPENVIBE_DEBUG_RAW_READER
			m_rDriverContext.getLogManager() << m_pSample[i] << " ";
#endif
		}

		if(m_rDriverContext.isStarted())
		{
			m_pCallback->setSamples(m_pSample, 1);
		}
	}
#ifdef OPENVIBE_DEBUG_RAW_READER
	m_rDriverContext.getLogManager() << "\n";
#endif

	if(m_rDriverContext.isStarted())
	{
		m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());
	}

	m_ui64TotalSampleCount+=m_ui32SampleCountPerSentBlock;

	return true;
}

boolean CDriverGenericRawReader::stop(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }

	return true;
}

boolean CDriverGenericRawReader::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

	if(!this->close())
	{
		return false;
	}

	delete [] m_pSample;
	delete [] m_pDataFrame;
	m_pSample=NULL;
	m_pDataFrame=NULL;
	m_pCallback=NULL;

	return true;
}
