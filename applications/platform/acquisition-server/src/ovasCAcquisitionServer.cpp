#include "ovasCAcquisitionServer.h"
#include "ovasIAcquisitionServerPlugin.h"

#include <toolkit/ovtk_all.h>
#include <openvibe/ovITimeArithmetics.h>

#include <ovp_global_defines.h>

#include <system/ovCMemory.h>
#include <system/ovCTime.h>

#include <fstream>
#include <sstream>

#include <string>
#include <algorithm>
#include <functional>
#include <cctype>
#include <cstring>

#include <cassert>

#include <iostream>



#define boolean OpenViBE::boolean

namespace
{
	// because std::tolower has multiple signatures,
	// it can not be easily used in std::transform
	// this workaround is taken from http://www.gcek.net/ref/books/sw/cpp/ticppv2/
	template <class charT>
	charT to_lower(charT c)
	{
		return std::tolower(c);
	}
};

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace std;

namespace OpenViBEAcquisitionServer
{
	class CDriverContext : public OpenViBEAcquisitionServer::IDriverContext
	{
	public:

		CDriverContext(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBEAcquisitionServer::CAcquisitionServer& rAcquisitionServer)
			:m_rKernelContext(rKernelContext)
			,m_rAcquisitionServer(rAcquisitionServer)
		{
		}

		virtual ILogManager& getLogManager(void) const
		{
			return m_rKernelContext.getLogManager();
		}

		virtual IConfigurationManager& getConfigurationManager(void) const
		{
			return m_rKernelContext.getConfigurationManager();
		}

		virtual boolean isConnected(void) const
		{
			return m_rAcquisitionServer.isConnected();
		}

		virtual boolean isStarted(void) const
		{
			return m_rAcquisitionServer.isStarted();
		}

		virtual boolean isImpedanceCheckRequested(void) const
		{
			return m_rAcquisitionServer.isImpedanceCheckRequested();
		}

		virtual boolean isChannelSelectionRequested(void) const
		{
			return m_rAcquisitionServer.isChannelSelectionRequested();
		}

		virtual int64 getDriftSampleCount(void) const
		{
			return m_rAcquisitionServer.getDriftSampleCount();
		}

		virtual boolean correctDriftSampleCount(int64 i64SampleCount)
		{
			return m_rAcquisitionServer.correctDriftSampleCount(i64SampleCount);
		}

		virtual int64 getDriftToleranceSampleCount(void) const
		{
			return m_rAcquisitionServer.getDriftToleranceSampleCount();
		}

		virtual int64 getSuggestedDriftCorrectionSampleCount(void) const
		{
			return m_rAcquisitionServer.getSuggestedDriftCorrectionSampleCount();
		}

		virtual boolean setInnerLatencySampleCount(int64 i64SampleCount)
		{
			return m_rAcquisitionServer.setInnerLatencySampleCount(i64SampleCount);
		}

		virtual int64 getInnerLatencySampleCount(void) const
		{
			return m_rAcquisitionServer.getInnerLatencySampleCount();
		}

		virtual boolean updateImpedance(const uint32 ui32ChannelIndex, const float64 f64Impedance)
		{
			return m_rAcquisitionServer.updateImpedance(ui32ChannelIndex, f64Impedance);
		}

	protected:

		const IKernelContext& m_rKernelContext;
		CAcquisitionServer& m_rAcquisitionServer;
	};

	class CConnectionServerHandlerThread
	{
	public:
		CConnectionServerHandlerThread(CAcquisitionServer& rAcquisitionServer, Socket::IConnectionServer& rConnectionServer)
			:m_rAcquisitionServer(rAcquisitionServer)
			,m_rConnectionServer(rConnectionServer)
		{
		}

		void operator()(void)
		{
			Socket::IConnection* l_pConnection=NULL;
			do
			{
				l_pConnection=m_rConnectionServer.accept();
				m_rAcquisitionServer.acceptNewConnection(l_pConnection);
			}
			while(l_pConnection && m_rAcquisitionServer.isConnected());
		}

		CAcquisitionServer& m_rAcquisitionServer;
		Socket::IConnectionServer& m_rConnectionServer;
	};

	class CConnectionClientHandlerThread
	{
	public:
		CConnectionClientHandlerThread(CAcquisitionServer& rAcquisitionServer, Socket::IConnection& rConnection)
			:m_rAcquisitionServer(rAcquisitionServer)
			,m_rConnection(rConnection)
		{
		}

		void operator()(void)
		{
			do
			{
				CMemoryBuffer* l_pMemoryBuffer=NULL;

				{
					boost::mutex::scoped_lock l_oProtectionLock(m_oPendingBufferProtectionMutex);
					boost::mutex::scoped_lock l_oExecutionLock(m_oPendingBufferExectutionMutex);
					l_oProtectionLock.unlock();
					if(m_vPendingBuffer.size())
					{
						l_pMemoryBuffer=m_vPendingBuffer.front();
						m_vPendingBuffer.pop_front();
					}
				}

				if(l_pMemoryBuffer)
				{
					uint64 l_ui64MemoryBufferSize=l_pMemoryBuffer->getSize();
					m_rConnection.sendBufferBlocking(&l_ui64MemoryBufferSize, sizeof(l_ui64MemoryBufferSize));
					m_rConnection.sendBufferBlocking(l_pMemoryBuffer->getDirectPointer(), (uint32)l_pMemoryBuffer->getSize());
					delete l_pMemoryBuffer;
				}
				else
				{
					System::Time::sleep((uint32)m_rAcquisitionServer.m_ui64StartedDriverSleepDuration);
				}
			}
			while(m_rConnection.isConnected());

			while(m_vPendingBuffer.size())
			{
				delete m_vPendingBuffer.front();
				m_vPendingBuffer.pop_front();
			}
		}

		void scheduleBuffer(const IMemoryBuffer& rMemoryBuffer)
		{
			CMemoryBuffer* l_pMemoryBuffer=new CMemoryBuffer(rMemoryBuffer);
			boost::mutex::scoped_lock l_oProtectionLock(m_oPendingBufferProtectionMutex);
			boost::mutex::scoped_lock l_oExecutionLock(m_oPendingBufferExectutionMutex);
			l_oProtectionLock.unlock();
			m_vPendingBuffer.push_back(l_pMemoryBuffer);
		}

		CAcquisitionServer& m_rAcquisitionServer;
		Socket::IConnection& m_rConnection;

		std::deque < CMemoryBuffer* > m_vPendingBuffer;
		boost::mutex m_oPendingBufferExectutionMutex;
		boost::mutex m_oPendingBufferProtectionMutex;
	};

	static void start_connection_client_handler_thread(CConnectionClientHandlerThread* pThread)
	{
		(*pThread)();
	}
}

//___________________________________________________________________//
//                                                                   //

CAcquisitionServer::CAcquisitionServer(const IKernelContext& rKernelContext)
	:m_pConnectionServerHandlerBoostThread(NULL)
	,m_rKernelContext(rKernelContext)
	,m_pDriverContext(NULL)
	,m_pDriver(NULL)
	,m_pStreamEncoder(NULL)
	,m_pConnectionServer(NULL)
	,m_eDriftCorrectionPolicy(DriftCorrectionPolicy_DriverChoice)
	,m_eNaNReplacementPolicy(NaNReplacementPolicy_LastCorrectValue)
	,m_bReplacementInProgress(false)
	,m_bInitialized(false)
	,m_bStarted(false)
{
	m_pDriverContext=new CDriverContext(rKernelContext, *this);

	m_pStreamEncoder=&m_rKernelContext.getAlgorithmManager().getAlgorithm(m_rKernelContext.getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_MasterAcquisitionStreamEncoder));
	// m_pStreamEncoder=&m_rKernelContext.getAlgorithmManager().getAlgorithm(m_rKernelContext.getAlgorithmManager().createAlgorithm(OVP_ClassId_GD_Algorithm_MasterAcquisitionStreamEncoderCSV));
	m_pStreamEncoder->initialize();

	ip_ui64SubjectIdentifier.initialize(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SubjectIdentifier));
	ip_ui64SubjectAge.initialize(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SubjectAge));
	ip_ui64SubjectGender.initialize(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SubjectGender));
	ip_pSignalMatrix.initialize(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SignalMatrix));
	ip_ui64SignalSamplingRate.initialize(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SignalSamplingRate));
	ip_pStimulationSet.initialize(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_StimulationSet));
	ip_ui64BufferDuration.initialize(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_BufferDuration));
	ip_pChannelUnits.initialize(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_ChannelUnits));
	op_pEncodedMemoryBuffer.initialize(m_pStreamEncoder->getOutputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	ip_bEncodeChannelLocalisationData.initialize(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_EncodeChannelLocalisationData));
	ip_bEncodeChannelUnitData.initialize(m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_EncodeChannelUnitData));

	CString l_sNaNReplacementPolicy=m_rKernelContext.getConfigurationManager().expand("${AcquisitionServer_NaNReplacementPolicy}");
	if(l_sNaNReplacementPolicy==CString("Disabled"))
	{
		this->setNaNReplacementPolicy(NaNReplacementPolicy_Disabled);
	}
	else if(l_sNaNReplacementPolicy==CString("Zero"))
	{
		this->setNaNReplacementPolicy(NaNReplacementPolicy_Zero);
	}
	else
	{
		this->setNaNReplacementPolicy(NaNReplacementPolicy_LastCorrectValue);
	}


	CString l_sDriftCorrectionPolicy=m_rKernelContext.getConfigurationManager().expand("${AcquisitionServer_DriftCorrectionPolicy}");
	if(l_sDriftCorrectionPolicy==CString("Forced"))
	{
		this->setDriftCorrectionPolicy(DriftCorrectionPolicy_Forced);
	}
	else if(l_sDriftCorrectionPolicy==CString("Disabled"))
	{
		this->setDriftCorrectionPolicy(DriftCorrectionPolicy_Disabled);
	}
	else
	{
		this->setDriftCorrectionPolicy(DriftCorrectionPolicy_DriverChoice);
	}

	this->setDriftToleranceDuration(m_rKernelContext.getConfigurationManager().expandAsUInteger("${AcquisitionServer_DriftToleranceDuration}", 5));
	this->setJitterEstimationCountForDrift(m_rKernelContext.getConfigurationManager().expandAsUInteger("${AcquisitionServer_JitterEstimationCountForDrift}", 128));
	this->setOversamplingFactor(m_rKernelContext.getConfigurationManager().expandAsUInteger("${AcquisitionServer_OverSamplingFactor}", 1));
	this->setImpedanceCheckRequest(m_rKernelContext.getConfigurationManager().expandAsBoolean("${AcquisitionServer_CheckImpedance}", false));
	this->setChannelSelectionRequest(m_rKernelContext.getConfigurationManager().expandAsBoolean("${AcquisitionServer_ChannelSelection}", false));

	m_ui64StartedDriverSleepDuration=m_rKernelContext.getConfigurationManager().expandAsUInteger("${AcquisitionServer_StartedDriverSleepDuration}", 2);
	m_ui64StoppedDriverSleepDuration=m_rKernelContext.getConfigurationManager().expandAsUInteger("${AcquisitionServer_StoppedDriverSleepDuration}", 100);
	m_ui64DriverTimeoutDuration=m_rKernelContext.getConfigurationManager().expandAsUInteger("${AcquisitionServer_DriverTimeoutDuration}", 5000);




	m_i64DriftSampleCount=0;

	for(std::vector<IAcquisitionServerPlugin*>::iterator itp = m_vPlugins.begin(); itp != m_vPlugins.end(); ++itp)
	{
		(*itp)->createHook();
	}
}

CAcquisitionServer::~CAcquisitionServer(void)
{
	if(m_bStarted)
	{
		m_pDriver->stop();
		// m_pDriverContext->onStop(*m_pDriver->getHeader());
		m_bStarted=false;
	}

	if(m_bInitialized)
	{
		m_pDriver->uninitialize();
		// m_pDriverContext->onUninitialize(*m_pDriver->getHeader());
		m_bInitialized=false;
	}

	if(m_pConnectionServer)
	{
		m_pConnectionServer->release();
		m_pConnectionServer=NULL;
	}

	/* -- should already be empty after call to stop -- */
	/*
    list < pair < Socket::IConnection*, SConnectionInfo > >::iterator itConnection=m_vConnection.begin();
    while(itConnection!=m_vConnection.end())
    {
	itConnection->first->release();
	itConnection=m_vConnection.erase(itConnection);
    }
*/

	ip_ui64SubjectIdentifier.uninitialize();
	ip_ui64SubjectAge.uninitialize();
	ip_ui64SubjectGender.uninitialize();
	ip_pSignalMatrix.uninitialize();
	ip_ui64SignalSamplingRate.uninitialize();
	ip_pStimulationSet.uninitialize();
	ip_ui64BufferDuration.uninitialize();
	op_pEncodedMemoryBuffer.uninitialize();
	ip_pChannelUnits.uninitialize();

	ip_bEncodeChannelLocalisationData.uninitialize();
	ip_bEncodeChannelUnitData.uninitialize();

	m_pStreamEncoder->uninitialize();
	m_rKernelContext.getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoder);
	m_pStreamEncoder=NULL;

	delete m_pDriverContext;
	m_pDriverContext=NULL;
}

IDriverContext& CAcquisitionServer::getDriverContext(void)
{
	return *m_pDriverContext;
}

uint32 CAcquisitionServer::getClientCount(void)
{
	return uint32(m_vConnection.size());
}

float64 CAcquisitionServer::getDrift(void)
{
	return m_f64DriftSampleCount*1000./m_ui32SamplingFrequency;
}

float64 CAcquisitionServer::getImpedance(const uint32 ui32ChannelIndex)
{
	if(ui32ChannelIndex < m_vImpedance.size())
	{
		return m_vImpedance[ui32ChannelIndex];
	}
	return OVAS_Impedance_Unknown;
}

//___________________________________________________________________//
//                                                                   //

boolean CAcquisitionServer::loop(void)
{
	// m_rKernelContext.getLogManager() << LogLevel_Debug << "idleCB\n";

	list < pair < Socket::IConnection*, SConnectionInfo > >::iterator itConnection;

	// Searches for new connection(s)
	if(m_pConnectionServer)
	{
		boost::mutex::scoped_lock m_oProtectionLock(m_oPendingConnectionProtectionMutex);
		boost::mutex::scoped_lock m_oExecutionLock(m_oPendingConnectionExectutionMutex);
		m_oProtectionLock.unlock();

		for(itConnection=m_vPendingConnection.begin(); itConnection!=m_vPendingConnection.end(); itConnection++)
		{
			m_rKernelContext.getLogManager() << LogLevel_Info << "Received new connection...\n";

			Socket::IConnection* l_pConnection=itConnection->first;
			if(this->isStarted())
			{
				// When a new connection is found and the
				// acq server is started, send the header

				// Computes inner data to skip
				int64 l_i64SignedTheoreticalSampleCountToSkip=0;
				if(m_bDriftCorrectionCalled)
				{
					l_i64SignedTheoreticalSampleCountToSkip=((int64(itConnection->second.m_ui64ConnectionTime-m_ui64StartTime)*m_ui32SamplingFrequency)>>32)-m_ui64SampleCount+m_vPendingBuffer.size();
				}
				else
				{
					l_i64SignedTheoreticalSampleCountToSkip=((int64(itConnection->second.m_ui64ConnectionTime-m_ui64LastDeliveryTime)*m_ui32SamplingFrequency)>>32)+m_vPendingBuffer.size();
				}

				uint64 l_ui64TheoreticalSampleCountToSkip=(l_i64SignedTheoreticalSampleCountToSkip<0?0:uint64(l_i64SignedTheoreticalSampleCountToSkip));

				m_rKernelContext.getLogManager() << LogLevel_Trace << "Sample count offset at connection : " << l_ui64TheoreticalSampleCountToSkip << "\n";

				SConnectionInfo l_oInfo;
				l_oInfo.m_ui64ConnectionTime=itConnection->second.m_ui64ConnectionTime;
				l_oInfo.m_ui64StimulationTimeOffset=ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, l_ui64TheoreticalSampleCountToSkip+m_ui64SampleCount-m_vPendingBuffer.size());
				l_oInfo.m_ui64SignalSampleCountToSkip=l_ui64TheoreticalSampleCountToSkip;
				l_oInfo.m_pConnectionClientHandlerThread=new CConnectionClientHandlerThread(*this, *l_pConnection);
				l_oInfo.m_pConnectionClientHandlerBoostThread=new boost::thread(boost::bind(&start_connection_client_handler_thread, l_oInfo.m_pConnectionClientHandlerThread));
				l_oInfo.m_bChannelLocalisationSent = false;
				l_oInfo.m_bChannelUnitsSent = false;
				//applyPriority(l_oInfo.m_pConnectionClientHandlerBoostThread,15);

				m_vConnection.push_back(pair < Socket::IConnection*, SConnectionInfo >(l_pConnection, l_oInfo));

#if DEBUG_STREAM
				m_rKernelContext.getLogManager() << LogLevel_Debug << "Creating header\n";
#endif

				op_pEncodedMemoryBuffer->setSize(0, true);
				m_pStreamEncoder->process(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputTriggerId_EncodeHeader);

#if 0
				uint64 l_ui64MemoryBufferSize=op_pEncodedMemoryBuffer->getSize();
				l_pConnection->sendBufferBlocking(&l_ui64MemoryBufferSize, sizeof(l_ui64MemoryBufferSize));
				l_pConnection->sendBufferBlocking(op_pEncodedMemoryBuffer->getDirectPointer(), (uint32)op_pEncodedMemoryBuffer->getSize());
#else
				l_oInfo.m_pConnectionClientHandlerThread->scheduleBuffer(*op_pEncodedMemoryBuffer);
#endif
			}
			else
			{
				// When a new connection is found and the
				// acq server is _not_ started, drop the
				// connection

				m_rKernelContext.getLogManager() << LogLevel_Warning << "Dropping connection - acquisition is not started\n";
				l_pConnection->release();
			}
		}
		m_vPendingConnection.clear();
	}

	// Cleans disconnected client(s)
	for(itConnection=m_vConnection.begin(); itConnection!=m_vConnection.end(); )
	{
		Socket::IConnection* l_pConnection=itConnection->first;
		if(!l_pConnection->isConnected())
		{
			l_pConnection->release();
			if(itConnection->second.m_pConnectionClientHandlerBoostThread)
			{
				itConnection->second.m_pConnectionClientHandlerBoostThread->join();
				delete itConnection->second.m_pConnectionClientHandlerBoostThread;
				delete itConnection->second.m_pConnectionClientHandlerThread;
			}
			itConnection=m_vConnection.erase(itConnection);
			m_rKernelContext.getLogManager() << LogLevel_Info << "Closed connection...\n";
		}
		else
		{
			itConnection++;
		}
	}

	// Handles driver's main loop
	if(m_pDriver)
	{
		boolean l_bResult;
		boolean l_bTimeout;
		if(this->isStarted())
		{
			l_bResult=true;
			l_bTimeout=false;
			m_bGotData=false;
			uint32 l_ui32StartTime=System::Time::getTime();
			while(l_bResult && !m_bGotData && !l_bTimeout)
			{
				// Calls driver's loop
				l_bResult=m_pDriver->loop();
				if(!m_bGotData)
				{
					System::Time::sleep((uint32)m_ui64StartedDriverSleepDuration);
					l_bTimeout=(System::Time::getTime()>l_ui32StartTime+m_ui64DriverTimeoutDuration);
				}
			}
			if(l_bTimeout)
			{
				m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "After " << m_ui64DriverTimeoutDuration << " milliseconds, did not receive anything from the driver - Timed out\n";
				return false;
			}
			if(m_bGotData && m_eDriftCorrectionPolicy == DriftCorrectionPolicy_Forced)
			{
				this->correctDriftSampleCount(this->getSuggestedDriftCorrectionSampleCount());
			}
		}
		else
		{
			// Calls driver's loop
			l_bResult=m_pDriver->loop();
			System::Time::sleep((uint32)m_ui64StoppedDriverSleepDuration);
		}

		// Calls driver's loop
		if(!l_bResult)
		{
			m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Something bad happened in the loop callback, stopping the acquisition\n";
			return false;
		}
	}

	// Eventually builds up buffer and
	// sends data to connected client(s)
	while(m_vPendingBuffer.size() >= m_ui32SampleCountPerSentBlock*2)
	{
		const int64 p = m_ui64SampleCount-m_vPendingBuffer.size();
		if (p < 0)
			m_rKernelContext.getLogManager() << LogLevel_Error << "Signed number used for bit operations:" << p << " (case A)\n";

		const uint64 l_ui64BufferStartSamples = m_ui64SampleCount-m_vPendingBuffer.size();
		const uint64 l_ui64BufferEndSamples   = m_ui64SampleCount-m_vPendingBuffer.size()+m_ui32SampleCountPerSentBlock;
		const uint64 l_ui64BufferStartTime    = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, l_ui64BufferStartSamples);
		const uint64 l_ui64BufferEndTime      = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, l_ui64BufferEndSamples);

		// Pass the stimuli and buffer to all plugins; note that they may modify them
		for(std::vector<IAcquisitionServerPlugin*>::iterator itp = m_vPlugins.begin(); itp != m_vPlugins.end(); ++itp)
		{
			(*itp)->loopHook(m_vPendingBuffer, m_oPendingStimulationSet, l_ui64BufferStartTime, l_ui64BufferEndTime);
		}

		// Handle connections
		for(itConnection=m_vConnection.begin(); itConnection!=m_vConnection.end(); itConnection++)
		{
			// Socket::IConnection* l_pConnection=itConnection->first;
			SConnectionInfo& l_rInfo=itConnection->second;

			if(l_rInfo.m_ui64SignalSampleCountToSkip<m_ui32SampleCountPerSentBlock)
			{
#if DEBUG_STREAM
				m_rKernelContext.getLogManager() << LogLevel_Debug << "Creating buffer for connection " << uint64(l_pConnection) << "\n";
#endif

				// Signal buffer
				for(uint32 j=0; j<m_ui32ChannelCount; j++)
				{
					for(uint32 i=0; i<m_ui32SampleCountPerSentBlock; i++)
					{
						ip_pSignalMatrix->getBuffer()[j*m_ui32SampleCountPerSentBlock+i]=m_vPendingBuffer[(int)(i+l_rInfo.m_ui64SignalSampleCountToSkip)][j];
					}
				}

				//				int l = l_oStimulationSet.getStimulationCount();

				const int64 p = l_ui64BufferStartSamples+l_rInfo.m_ui64SignalSampleCountToSkip;
				if (p < 0)
					m_rKernelContext.getLogManager() << LogLevel_Error << "Signed number used for bit operations:" << p << " (case B)\n";

				const uint64 l_ui64ConnBlockStartTime = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, l_ui64BufferStartSamples + l_rInfo.m_ui64SignalSampleCountToSkip);
				const uint64 l_ui64ConnBlockEndTime   = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, l_ui64BufferEndSamples   + l_rInfo.m_ui64SignalSampleCountToSkip);

				//m_rKernelContext.getLogManager() << LogLevel_Info << "start: " << time64(start) << "end: " << time64(end) << "\n";

				// Stimulation buffer
				CStimulationSet l_oStimulationSet;

				// Take the stimuli range valid for the connection
				OpenViBEToolkit::Tools::StimulationSet::appendRange(
							l_oStimulationSet,
							m_oPendingStimulationSet,
							l_ui64ConnBlockStartTime,l_ui64ConnBlockEndTime);

				OpenViBEToolkit::Tools::StimulationSet::copy(*ip_pStimulationSet, l_oStimulationSet, -int64(l_rInfo.m_ui64StimulationTimeOffset));

				// Send a chunk of channel units? Note that we'll always send the units header.
				if(!l_rInfo.m_bChannelUnitsSent)
				{
					// If default values in channel units, don't bother sending unit data chunk
					ip_bEncodeChannelUnitData = m_pHeaderCopy->isChannelUnitSet();
					// std::cout << "Set " <<  ip_pChannelUnits->getBufferElementCount()  << "\n";
				}

				op_pEncodedMemoryBuffer->setSize(0, true);
				m_pStreamEncoder->process(OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputTriggerId_EncodeBuffer);

				if(!l_rInfo.m_bChannelUnitsSent)
				{
					// Do not send again
					l_rInfo.m_bChannelUnitsSent = true;
					ip_bEncodeChannelUnitData = false;
				}

#if 0
				uint64 l_ui64MemoryBufferSize=op_pEncodedMemoryBuffer->getSize();
				l_pConnection->sendBufferBlocking(&l_ui64MemoryBufferSize, sizeof(l_ui64MemoryBufferSize));
				l_pConnection->sendBufferBlocking(op_pEncodedMemoryBuffer->getDirectPointer(), (uint32)op_pEncodedMemoryBuffer->getSize());
#else
				l_rInfo.m_pConnectionClientHandlerThread->scheduleBuffer(*op_pEncodedMemoryBuffer);
#endif
			}
			else
			{
				l_rInfo.m_ui64SignalSampleCountToSkip-=m_ui32SampleCountPerSentBlock;
			}
		}

		// Clears pending stimulations
		OpenViBEToolkit::Tools::StimulationSet::removeRange(
					m_oPendingStimulationSet,
					l_ui64BufferStartTime,
					l_ui64BufferEndTime
					);

		// Clears pending signal
		m_vPendingBuffer.erase(m_vPendingBuffer.begin(), m_vPendingBuffer.begin()+m_ui32SampleCountPerSentBlock);
	}

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CAcquisitionServer::connect(IDriver& rDriver, IHeader& rHeaderCopy, uint32 ui32SamplingCountPerSentBlock, uint32 ui32ConnectionPort)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "connect\n";

	m_i64InnerLatencySampleCount=0;
	m_pDriver=&rDriver;
	m_pHeaderCopy=&rHeaderCopy;
	m_ui32SampleCountPerSentBlock=ui32SamplingCountPerSentBlock;

	m_rKernelContext.getLogManager() << LogLevel_Info << "Connecting to device [" << CString(m_pDriver->getName()) << "]...\n";

	// Initializes driver
	if(!m_pDriver->initialize(m_ui32SampleCountPerSentBlock, *this))
	{
		m_rKernelContext.getLogManager() << LogLevel_Error << "Connection failed...\n";
		m_ui64StartTime=System::Time::zgetTime();
		return false;
	}

	// m_pDriverContext->onInitialize(*m_pDriver->getHeader());

	m_rKernelContext.getLogManager() << LogLevel_Info << "Connection succeeded !\n";

	const IHeader& l_rHeader=*rDriver.getHeader();
	IHeader::copy(rHeaderCopy, l_rHeader);

	m_ui32ChannelCount=rHeaderCopy.getChannelCount();
	m_ui32SamplingFrequency=(uint32)(rHeaderCopy.getSamplingFrequency()*m_ui64OverSamplingFactor);

	m_vSelectedChannels.clear();
	if (m_bIsChannelSelectionRequested)
	{
		for (OpenViBE::uint32 i=0,l=0;i<m_ui32ChannelCount;++i) {
			const std::string name = rHeaderCopy.getChannelName(i);
			if (name!="") {
				m_vSelectedChannels.push_back(i);
				rHeaderCopy.setChannelName(l,name.c_str());

				uint32 l_ui32Unit = 0, l_ui32Factor = 0;
				if(rHeaderCopy.isChannelUnitSet())
				{
					rHeaderCopy.getChannelUnits(i, l_ui32Unit, l_ui32Factor); // no need to check, will be defaults on failure
					rHeaderCopy.setChannelUnits(l, l_ui32Unit, l_ui32Factor);
				}
				l++;
			}
		}
		rHeaderCopy.setChannelCount(m_vSelectedChannels.size());
		m_ui32ChannelCount = rHeaderCopy.getChannelCount();

	} else {

		for (OpenViBE::uint32 i=0;i<m_ui32ChannelCount;++i)
			m_vSelectedChannels.push_back(i);
	}
	
	// These are passed to plugins
	m_vSelectedChannelNames.clear();
	for(uint32 i=0;i<rHeaderCopy.getChannelCount();i++) 
	{
		m_vSelectedChannelNames.push_back(CString(rHeaderCopy.getChannelName(i)));
	}

	if(m_ui32ChannelCount==0)
	{
		std::stringstream ss;
		ss << "Driver claimed to have " << uint32(0) << " channel";
		if (isChannelSelectionRequested())
			ss << "(check whether the property `Select only named channel' is set).\n";
		m_rKernelContext.getLogManager() << LogLevel_Error << ss.str().c_str();
		return false;
	}

	if(m_ui32SamplingFrequency==0)
	{
		m_rKernelContext.getLogManager() << LogLevel_Error << "Driver claimed to have a sample frequency of " << uint32(0) << "\n";
		return false;
	}

	m_vImpedance.resize(m_ui32ChannelCount, OVAS_Impedance_NotAvailable);
	m_vSwapBuffer.resize(m_ui32ChannelCount);

	m_pConnectionServer=Socket::createConnectionServer();
	if(m_pConnectionServer->listen(ui32ConnectionPort))
	{
		m_bInitialized=true;

		m_i64DriftSampleCount=0;
		m_i64DriftToleranceSampleCount=(m_ui64DriftToleranceDuration * m_ui32SamplingFrequency) / 1000;
		m_i64DriftCorrectionSampleCountAdded=0;
		m_i64DriftCorrectionSampleCountRemoved=0;

		m_rKernelContext.getLogManager() << LogLevel_Trace << "Drift correction is set to ";
		switch(m_eDriftCorrectionPolicy)
		{
			default:
			case DriftCorrectionPolicy_DriverChoice: m_rKernelContext.getLogManager() << CString("DriverChoice") << "\n"; break;
			case DriftCorrectionPolicy_Forced:       m_rKernelContext.getLogManager() << CString("Forced") << "\n"; break;
			case DriftCorrectionPolicy_Disabled:     m_rKernelContext.getLogManager() << CString("Disabled") << "\n"; break;
		};

		m_rKernelContext.getLogManager() << LogLevel_Trace << "NaN value correction is set to ";
		switch(m_eNaNReplacementPolicy)
		{
			default:
			case NaNReplacementPolicy_LastCorrectValue: m_rKernelContext.getLogManager() << CString("LastCorrectValue") << "\n"; break;
			case NaNReplacementPolicy_Zero:             m_rKernelContext.getLogManager() << CString("Zero") << "\n"; break;
			case NaNReplacementPolicy_Disabled:         m_rKernelContext.getLogManager() << CString("Disabled") << "\n"; break;
		};

		m_rKernelContext.getLogManager() << LogLevel_Trace << "Oversampling factor set to " << m_ui64OverSamplingFactor << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Sampling frequency set to " << m_ui32SamplingFrequency << "Hz\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Driver monitoring drift tolerance set to " << m_ui64DriftToleranceDuration << " milliseconds - eq " << m_i64DriftToleranceSampleCount << " samples\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Driver monitoring drift estimation on " << m_ui64JitterEstimationCountForDrift << " jitter measures\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Started driver sleeping duration is " << m_ui64StartedDriverSleepDuration << " milliseconds\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Stopped driver sleeping duration is " << m_ui64StoppedDriverSleepDuration << " milliseconds\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Driver timeout duration set to " << m_ui64DriverTimeoutDuration << " milliseconds\n";

		ip_ui64BufferDuration=ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, m_ui32SampleCountPerSentBlock);

		ip_ui64SubjectIdentifier=rHeaderCopy.getExperimentIdentifier();
		ip_ui64SubjectAge=rHeaderCopy.getSubjectAge();
		ip_ui64SubjectGender=rHeaderCopy.getSubjectGender();

		ip_ui64SignalSamplingRate=m_ui32SamplingFrequency;
		ip_pSignalMatrix->setDimensionCount(2);
		ip_pSignalMatrix->setDimensionSize(0, m_ui32ChannelCount);
		ip_pSignalMatrix->setDimensionSize(1, m_ui32SampleCountPerSentBlock);

		m_rKernelContext.getLogManager() << LogLevel_Trace << "Sampling rate     : " << m_ui32SamplingFrequency << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Samples per block : " << m_ui32SampleCountPerSentBlock << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Channel count     : " << m_ui32ChannelCount << "\n";

		for(uint32 i=0; i<m_ui32ChannelCount; i++)
		{
			const std::string l_sChannelName=rHeaderCopy.getChannelName(i);
			if(l_sChannelName!="")
			{
				ip_pSignalMatrix->setDimensionLabel(0, i, l_sChannelName.c_str());
			}
			else
			{
				std::stringstream ss;
				ss << "Channel " << i+1;
				ip_pSignalMatrix->setDimensionLabel(0, i, ss.str().c_str());
			}
			m_rKernelContext.getLogManager() << LogLevel_Trace << "Channel name      : " << CString(ip_pSignalMatrix->getDimensionLabel(0, i)) << "\n";
		}

		// Construct channel units stream header & matrix
		// Note: Channel units, although part of IHeader, will be sent as a matrix chunk during loop() once - if at all - to each client
		if(m_pHeaderCopy->isChannelUnitSet())
		{
			ip_pChannelUnits->setDimensionCount(2);
			ip_pChannelUnits->setDimensionSize(0, m_ui32ChannelCount);
			ip_pChannelUnits->setDimensionSize(1,2);                    // Units, Factor
			for(uint32 c=0;c<m_ui32ChannelCount;c++)
			{
				ip_pChannelUnits->setDimensionLabel(0, c, m_pHeaderCopy->getChannelName(c));
				if(m_pHeaderCopy->isChannelUnitSet())
				{
					uint32 l_ui32Unit = 0, l_ui32Factor = 0;
				
					m_pHeaderCopy->getChannelUnits(c, l_ui32Unit, l_ui32Factor); // no need to check, will be defaults on failure

					ip_pChannelUnits->getBuffer()[c*2+0] = static_cast<float64>(l_ui32Unit);
					ip_pChannelUnits->getBuffer()[c*2+1] = static_cast<float64>(l_ui32Factor);

					m_rKernelContext.getLogManager() << LogLevel_Trace << "Channel Unit      : " << l_ui32Unit << ", factor=" << OVTK_DECODE_FACTOR(l_ui32Factor) << "\n";
				}
			}
			ip_pChannelUnits->setDimensionLabel(1, 0, "Unit");
			ip_pChannelUnits->setDimensionLabel(1, 1, "Factor");
		}
		else
		{
			// Driver did not set units. Convention: send empty header matrix in this case.
			ip_pChannelUnits->setDimensionCount(2);
			ip_pChannelUnits->setDimensionSize(0,0);
			ip_pChannelUnits->setDimensionSize(0,0);
			m_rKernelContext.getLogManager() << LogLevel_Trace << "Driver did not set units, sending empty channel units matrix\n";
		}

		// @TODO Channel localisation
		{
			ip_bEncodeChannelLocalisationData = false; // never at the moment

		}

		// @TODO Gain is ignored
	}
	else
	{
		m_rKernelContext.getLogManager() << LogLevel_Error << "Could not listen on TCP port (firewall problem ?)\n";
		return false;
	}

	m_pConnectionServerHandlerBoostThread=new boost::thread(CConnectionServerHandlerThread(*this, *m_pConnectionServer));
	//applyPriority(m_pConnectionServerHandlerBoostThread,15);

	return true;
}

boolean CAcquisitionServer::start(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "buttonStartPressedCB\n";

	m_rKernelContext.getLogManager() << LogLevel_Info << "Starting the acquisition...\n";

	// Starts driver
	if(!m_pDriver->start())
	{
		m_ui64StartTime=System::Time::zgetTime();
		m_rKernelContext.getLogManager() << LogLevel_Error << "Starting failed !\n";
		return false;
	}
	// m_pDriverContext->onStart(*m_pDriver->getHeader());


	m_rKernelContext.getLogManager() << LogLevel_Info << "Now acquiring...\n";

	m_vPendingBuffer.clear();
	m_oPendingStimulationSet.clear();
	m_vJitterSampleCount.clear();

	m_ui64SampleCount=0;
	m_ui64LastSampleCount=0;
	m_i64DriftSampleCount=0;
	m_i64DriftCorrectionSampleCountAdded=0;
	m_i64DriftCorrectionSampleCountRemoved=0;
	m_bDriftCorrectionCalled=false;
	m_ui64StartTime=System::Time::zgetTime();
	m_ui64LastDeliveryTime=m_ui64StartTime;

	for(std::vector<IAcquisitionServerPlugin*>::iterator itp = m_vPlugins.begin(); itp != m_vPlugins.end(); ++itp)
	{
		(*itp)->startHook(m_vSelectedChannelNames, m_ui32SamplingFrequency, m_ui32ChannelCount, m_ui32SampleCountPerSentBlock);
	}

	m_bStarted=true;
	return true;
}

boolean CAcquisitionServer::stop(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "buttonStopPressedCB\n";

	m_rKernelContext.getLogManager() << LogLevel_Info << "Stopping the acquisition.\n";

	int64 l_i64DriftSampleCount=m_i64DriftSampleCount-(m_i64DriftCorrectionSampleCountAdded-m_i64DriftCorrectionSampleCountRemoved);
	uint64 l_ui64TheoreticalSampleCount=m_ui64SampleCount-m_i64DriftSampleCount;
	uint64 l_ui64ReceivedSampleCount=m_ui64SampleCount-(m_i64DriftCorrectionSampleCountAdded-m_i64DriftCorrectionSampleCountRemoved);
	float64 l_f64DriftRatio=(l_ui64ReceivedSampleCount?((l_i64DriftSampleCount*10000)/int64(l_ui64ReceivedSampleCount))/100.:0);
	float64 l_f64AddedRatio=(l_ui64ReceivedSampleCount?((m_i64DriftCorrectionSampleCountAdded*10000)/int64(l_ui64ReceivedSampleCount))/100.:0);
	float64 l_f64RemovedRatio=(l_ui64ReceivedSampleCount?((m_i64DriftCorrectionSampleCountRemoved*10000)/int64(l_ui64ReceivedSampleCount))/100.:0);
	
	if(-m_i64DriftToleranceSampleCount * 5 <= m_i64DriftSampleCount && m_i64DriftSampleCount <= m_i64DriftToleranceSampleCount * 5 && l_f64DriftRatio <= 5)
	{
		m_rKernelContext.getLogManager() << LogLevel_Trace << "For information, after " << (((System::Time::zgetTime()-m_ui64StartTime) * 1000) >> 32) * .001f << " seconds we got the following statistics :\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "  Received : " << l_ui64ReceivedSampleCount << " samples\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "  Should have received : " << l_ui64TheoreticalSampleCount << " samples\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "  Drift was : " << l_i64DriftSampleCount << " samples (" << l_f64DriftRatio << "%)\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "  Added : " << m_i64DriftCorrectionSampleCountAdded << " samples (" << l_f64AddedRatio << "%)\n";
		m_rKernelContext.getLogManager() << LogLevel_Trace << "  Removed : " << m_i64DriftCorrectionSampleCountRemoved << " samples (" << l_f64RemovedRatio << "%)\n";
	}
	else
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "After " << (((System::Time::zgetTime()-m_ui64StartTime) * 1000) >> 32) * .001f << " seconds, theoretical samples per second does not match real samples per second\n";
		m_rKernelContext.getLogManager() << LogLevel_Warning << "  Received : " << l_ui64ReceivedSampleCount << " samples\n";
		m_rKernelContext.getLogManager() << LogLevel_Warning << "  Should have received : " << l_ui64TheoreticalSampleCount << " samples\n";
		m_rKernelContext.getLogManager() << LogLevel_Warning << "  Drift was : " << l_i64DriftSampleCount << " samples (" << l_f64DriftRatio << "%)\n";
		if(m_i64DriftCorrectionSampleCountAdded==0 && m_i64DriftCorrectionSampleCountRemoved==0)
		{
			m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "  The driver did not try to correct this difference. This may be a feature of the driver.\n";
		}
		else
		{
			m_rKernelContext.getLogManager() << LogLevel_Warning << "  Added : " << m_i64DriftCorrectionSampleCountAdded << " samples (" << l_f64AddedRatio << "%)\n";
			m_rKernelContext.getLogManager() << LogLevel_Warning << "  Removed : " << m_i64DriftCorrectionSampleCountRemoved << " samples (" << l_f64RemovedRatio << "%)\n";
			m_rKernelContext.getLogManager() << LogLevel_Warning << "  The driver obviously tried to correct this difference\n";
		}
		// m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "  Please submit a bug report (including the acquisition server log file or at least this complete message) for the driver you are using\n";
	}

	// Stops driver
	m_pDriver->stop();
	// m_pDriverContext->onStop(*m_pDriver->getHeader());


	list < pair < Socket::IConnection*, SConnectionInfo > >::iterator itConnection=m_vConnection.begin();
	while(itConnection!=m_vConnection.end())
	{
		itConnection->first->close();
		if(itConnection->second.m_pConnectionClientHandlerBoostThread)
		{
			itConnection->second.m_pConnectionClientHandlerBoostThread->join();
			delete itConnection->second.m_pConnectionClientHandlerBoostThread;
			delete itConnection->second.m_pConnectionClientHandlerThread;
		}
		itConnection->first->release();

		itConnection=m_vConnection.erase(itConnection);
	}

	for(std::vector<IAcquisitionServerPlugin*>::iterator itp = m_vPlugins.begin(); itp != m_vPlugins.end(); ++itp)
	{
		(*itp)->stopHook();
	}


	m_bStarted=false;


	return true;
}

boolean CAcquisitionServer::disconnect(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Info << "Disconnecting.\n";

	if(m_bInitialized)
	{
		m_pDriver->uninitialize();
		// m_pDriverContext->onUninitialize(*m_pDriver->getHeader());
	}

	m_vImpedance.clear();

	if(m_pConnectionServer)
	{
		m_pConnectionServer->close();
		m_pConnectionServer->release();
		m_pConnectionServer=NULL;
	}

	m_bInitialized=false;

	// Thread joining must be done after
	// switching m_bInitialized to false
	if(m_pConnectionServerHandlerBoostThread)
	{
		m_pConnectionServerHandlerBoostThread->join();
		delete m_pConnectionServerHandlerBoostThread;
		m_pConnectionServerHandlerBoostThread=NULL;
	}

	return true;
}

//___________________________________________________________________//
//                                                                   //

void CAcquisitionServer::setSamples(const float32* pSample)
{
	if (pSample==NULL) m_rKernelContext.getLogManager() << LogLevel_Warning << "Null data detected\n";
	this->setSamples(pSample, m_ui32SampleCountPerSentBlock);
}

void CAcquisitionServer::setSamples(const float32* pSample, const uint32 ui32SampleCount)
{
	if(m_bStarted)
	{
		for(uint32 i=0; i<ui32SampleCount; i++)
		{
			if(!m_bReplacementInProgress)
			{
				// otherwise NaN are propagating
				m_vOverSamplingSwapBuffer=m_vSwapBuffer;
			}
			for(uint32 k=0; k<m_ui64OverSamplingFactor; k++)
			{
				const float32 alpha=float32(k+1)/m_ui64OverSamplingFactor;

				bool l_bHadNaN = false;

				for(uint32 j=0; j<m_ui32ChannelCount; j++)
				{
					const uint32 l_ui32Channel = m_vSelectedChannels[j];

#ifdef TARGET_OS_Windows
					if(_isnan(pSample[l_ui32Channel*ui32SampleCount+i]) || !_finite(pSample[l_ui32Channel*ui32SampleCount+i])) // NaN or infinite values
#else
					if(std::isnan(pSample[l_ui32Channel*ui32SampleCount+i]) || !finite(pSample[l_ui32Channel*ui32SampleCount+i])) // NaN or infinite values
#endif
					{
						l_bHadNaN = true;

						switch(m_eNaNReplacementPolicy)
						{
							case NaNReplacementPolicy_Disabled:
								m_vSwapBuffer[j] = std::numeric_limits<float>::quiet_NaN();
								break;
							case NaNReplacementPolicy_Zero:
								m_vSwapBuffer[j] = 0;
								break;
							case NaNReplacementPolicy_LastCorrectValue:
								// we simply don't update the value
								break;
							default:
								break;
						}
					}
					else
					{
						m_vSwapBuffer[j]=alpha*pSample[l_ui32Channel*ui32SampleCount+i]+(1-alpha)*m_vOverSamplingSwapBuffer[j];
					}
				}
		
				const uint64 l_ui64CurrentSampleIndex = m_ui64SampleCount + i*m_ui64OverSamplingFactor + k;		// j is not included here as all channels have the equal sample time

				if(l_bHadNaN)
				{
					// When a NaN is encountered at time t1 on any channel, OVTK_GDF_Incorrect stimulus is sent. When a first good sample is encountered 
					// after the last bad sample t2, OVTK_GDF_Correct stimulus is sent, i.e. specifying a range of bad data : [t1,t2]. The stimuli are global 
					// and not specific to channels.

					if(!m_bReplacementInProgress)
					{
						const uint64 l_ui64IncorrectBlockStarts = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, l_ui64CurrentSampleIndex);

						m_oPendingStimulationSet.appendStimulation(OVTK_GDF_Incorrect, l_ui64IncorrectBlockStarts, 0);
						m_bReplacementInProgress = true;
					}
				} 
				else 
				{
					if(m_bReplacementInProgress)
					{
						// @note -1 is used here because the incorrect-correct range is inclusive, [a,b]. So when sample is good at b+1, we set the end point at b.
						const uint64 l_ui64IncorrectBlockStops = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, l_ui64CurrentSampleIndex - 1);

						m_oPendingStimulationSet.appendStimulation(OVTK_GDF_Correct, l_ui64IncorrectBlockStops, 0);
						m_bReplacementInProgress = false;
					}
				}

				m_vPendingBuffer.push_back(m_vSwapBuffer);
			}
		}
		m_ui64LastSampleCount=m_ui64SampleCount;
		m_ui64SampleCount+=ui32SampleCount*m_ui64OverSamplingFactor;

		{
			const uint64 l_ui64ElapsedTime = System::Time::zgetTime()-m_ui64StartTime;
			const uint64 l_ui64TheoricalSampleCount= (m_ui32SamplingFrequency * l_ui64ElapsedTime) >> 32;
			const int64 l_i64JitterSampleCount=int64(m_ui64SampleCount-l_ui64TheoricalSampleCount)+m_i64InnerLatencySampleCount;
			
			m_vJitterSampleCount.push_back(l_i64JitterSampleCount);
			if(m_vJitterSampleCount.size() > m_ui64JitterEstimationCountForDrift)
			{
				m_vJitterSampleCount.pop_front();
			}
			m_i64DriftSampleCount=0;
			for(list<int64>::iterator j=m_vJitterSampleCount.begin(); j!=m_vJitterSampleCount.end(); j++)
			{
				m_i64DriftSampleCount+=*j;
			}
			m_f64DriftSampleCount=m_i64DriftSampleCount/float64(m_vJitterSampleCount.size());
			m_i64DriftSampleCount/=int64(m_vJitterSampleCount.size());

			m_rKernelContext.getLogManager() << LogLevel_Debug << "Acquisition monitoring [drift:" << m_i64DriftSampleCount << "][jitter:" << l_i64JitterSampleCount << "] samples.\n";
		}

		m_ui64LastDeliveryTime=System::Time::zgetTime();
		m_bGotData=true;
	}
	else
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "The acquisition is not started\n";
	}
}

void CAcquisitionServer::setStimulationSet(const IStimulationSet& rStimulationSet)
{
	if(m_bStarted)
	{
		uint64 l_ui64StimulationTime = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, m_ui64LastSampleCount);
		OpenViBEToolkit::Tools::StimulationSet::append(m_oPendingStimulationSet, rStimulationSet, l_ui64StimulationTime);
	}
	else
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "The acquisition is not started\n";
	}
}

boolean CAcquisitionServer::correctDriftSampleCount(int64 i64SampleCount)
{
	if(!m_bStarted)
	{
		return false;
	}

	m_bDriftCorrectionCalled=true;

	if(i64SampleCount == 0)
	{
		return true;
	}
	else
	{
		if(m_eDriftCorrectionPolicy == DriftCorrectionPolicy_Disabled)
		{
			return false;
		}

		char l_sTime[1024];
		uint64 l_ui64Time=System::Time::zgetTime()-m_ui64StartTime;
		float64 l_f64Time=ITimeArithmetics::timeToSeconds(l_ui64Time);
		::sprintf(l_sTime, "%.03lf", l_f64Time);
		m_rKernelContext.getLogManager() << LogLevel_Trace << "At time " << CString(l_sTime) << " : Correcting drift by " << i64SampleCount << " samples\n";
		if(i64SampleCount > 0)
		{
			for(int64 i=0; i<i64SampleCount; i++)
			{
				m_vPendingBuffer.push_back(m_vSwapBuffer);
			}

			uint64 l_ui64TimeOfIncorrect     = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, m_ui64SampleCount-1);
			uint64 l_ui64DurationOfIncorrect = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, i64SampleCount);
			uint64 l_ui64TimeOfCorrect       = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, m_ui64SampleCount-1+i64SampleCount);
			m_oPendingStimulationSet.appendStimulation(OVTK_GDF_Incorrect, l_ui64TimeOfIncorrect, l_ui64DurationOfIncorrect);
			m_oPendingStimulationSet.appendStimulation(OVTK_GDF_Correct,   l_ui64TimeOfCorrect, 0);

			m_f64DriftSampleCount+=i64SampleCount;
			m_i64DriftSampleCount+=i64SampleCount;
			m_ui64LastSampleCount=m_ui64SampleCount;
			m_ui64SampleCount+=i64SampleCount;
			m_i64DriftCorrectionSampleCountAdded+=i64SampleCount;
		}
		else if(i64SampleCount < 0)
		{
			uint64 l_ui64SamplesToRemove=uint64(-i64SampleCount);
			if(l_ui64SamplesToRemove>m_vPendingBuffer.size())
			{
				l_ui64SamplesToRemove=m_vPendingBuffer.size();
			}

			m_vPendingBuffer.erase(m_vPendingBuffer.begin()+m_vPendingBuffer.size()-(int)l_ui64SamplesToRemove, m_vPendingBuffer.begin()+m_vPendingBuffer.size());

#if 0
			uint64 l_ui64Start = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, m_ui64SampleCount-l_ui64SamplesToRemove);
			uint64 l_ui64End   = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, m_ui64SampleCount);
			OpenViBEToolkit::Tools::StimulationSet::removeRange(m_oPendingStimulationSet, l_ui64Start, l_ui64End);
#else
			uint64 l_ui64LastSampleDate = ITimeArithmetics::sampleCountToTime(m_ui32SamplingFrequency, m_ui64SampleCount-l_ui64SamplesToRemove);
			for(uint32 i=0; i<m_oPendingStimulationSet.getStimulationCount(); i++)
			{
				if(m_oPendingStimulationSet.getStimulationDate(i) > l_ui64LastSampleDate)
				{
					m_oPendingStimulationSet.setStimulationDate(i, l_ui64LastSampleDate);
				}
			}
#endif

			m_f64DriftSampleCount-=l_ui64SamplesToRemove;
			m_i64DriftSampleCount-=l_ui64SamplesToRemove;
			m_ui64LastSampleCount=m_ui64SampleCount;
			m_ui64SampleCount-=l_ui64SamplesToRemove;
			m_i64DriftCorrectionSampleCountRemoved+=l_ui64SamplesToRemove;
		}

		for(list<int64>::iterator j=m_vJitterSampleCount.begin(); j!=m_vJitterSampleCount.end(); j++)
		{
			(*j)+=i64SampleCount;
		}
	}

	return true;
}

int64 CAcquisitionServer::getSuggestedDriftCorrectionSampleCount(void) const
{
	if(m_eDriftCorrectionPolicy == DriftCorrectionPolicy_Disabled)
	{
		return 0;
	}

	if(this->getDriftSampleCount() >= this->getDriftToleranceSampleCount())
	{
		return -this->getDriftSampleCount();
	}

	if(this->getDriftSampleCount() <= -this->getDriftToleranceSampleCount())
	{
		return -this->getDriftSampleCount();
	}

	return 0;
}

boolean CAcquisitionServer::setInnerLatencySampleCount(OpenViBE::int64 i64SampleCount)
{
	m_i64InnerLatencySampleCount=i64SampleCount;
	return true;
}

int64 CAcquisitionServer::getInnerLatencySampleCount(void) const
{
	return m_i64InnerLatencySampleCount;
}

boolean CAcquisitionServer::updateImpedance(const uint32 ui32ChannelIndex, const float64 f64Impedance)
{
	for (OpenViBE::uint32 i=0;i<m_vSelectedChannels.size();++i)
		if (ui32ChannelIndex==m_vSelectedChannels[i]) {
			m_vImpedance[i] = f64Impedance;
			return true;
		}
	return false;
}

// ____________________________________________________________________________
//

ENaNReplacementPolicy CAcquisitionServer::getNaNReplacementPolicy(void)
{
	return m_eNaNReplacementPolicy;
}


EDriftCorrectionPolicy CAcquisitionServer::getDriftCorrectionPolicy(void)
{
	return m_eDriftCorrectionPolicy;
}

CString CAcquisitionServer::getNaNReplacementPolicyStr(void)
{
	switch (m_eNaNReplacementPolicy)
	{
		case NaNReplacementPolicy_Disabled:
			return CString("Disabled");
		case NaNReplacementPolicy_LastCorrectValue:
			return CString("LastCorrectValue");
		case NaNReplacementPolicy_Zero:
			return CString("Zero");
		default :
			return CString("N/A");
	}
}


CString CAcquisitionServer::getDriftCorrectionPolicyStr(void)
{
	switch (m_eDriftCorrectionPolicy)
	{
		case DriftCorrectionPolicy_Disabled:
			return CString("Disabled");
		case DriftCorrectionPolicy_DriverChoice:
			return CString("DriverChoice");
		case DriftCorrectionPolicy_Forced:
			return CString("Forced");
		default :
			return CString("N/A");
	}
}

uint64 CAcquisitionServer::getDriftToleranceDuration(void)
{
	return m_ui64DriftToleranceDuration;
}

uint64 CAcquisitionServer::getJitterEstimationCountForDrift(void)
{
	return m_ui64JitterEstimationCountForDrift;
}

uint64 CAcquisitionServer::getOversamplingFactor(void)
{
	return m_ui64OverSamplingFactor;
}

boolean CAcquisitionServer::setNaNReplacementPolicy(ENaNReplacementPolicy eNaNReplacementPolicy)
{
	m_eNaNReplacementPolicy=eNaNReplacementPolicy;
	return true;
}

boolean CAcquisitionServer::setDriftCorrectionPolicy(EDriftCorrectionPolicy eDriftCorrectionPolicy)
{
	m_eDriftCorrectionPolicy=eDriftCorrectionPolicy;
	return true;
}

boolean CAcquisitionServer::isImpedanceCheckRequested(void)
{
	return m_bIsImpedanceCheckRequested;
}

boolean CAcquisitionServer::isChannelSelectionRequested(void)
{
	return m_bIsChannelSelectionRequested;
}

boolean CAcquisitionServer::setDriftToleranceDuration(uint64 ui64DriftToleranceDuration)
{
	m_ui64DriftToleranceDuration=ui64DriftToleranceDuration;
	return true;
}

boolean CAcquisitionServer::setJitterEstimationCountForDrift(uint64 ui64JitterEstimationCountForDrift)
{
	m_ui64JitterEstimationCountForDrift=ui64JitterEstimationCountForDrift;
	return true;
}

boolean CAcquisitionServer::setOversamplingFactor(uint64 ui64OversamplingFactor)
{
	m_ui64OverSamplingFactor=ui64OversamplingFactor;
	if(m_ui64OverSamplingFactor<1) m_ui64OverSamplingFactor=1;
	if(m_ui64OverSamplingFactor>16) m_ui64OverSamplingFactor=16;
	return true;
}

boolean CAcquisitionServer::setImpedanceCheckRequest(boolean bActive)
{
	m_bIsImpedanceCheckRequested=bActive;
	return true;
}

boolean CAcquisitionServer::setChannelSelectionRequest(const boolean bActive)
{
	m_bIsChannelSelectionRequested=bActive;
	return true;
}

// ____________________________________________________________________________
//

boolean CAcquisitionServer::acceptNewConnection(Socket::IConnection* pConnection)
{
	if(!pConnection)
	{
		return false;
	}

	uint64 l_ui64Time=System::Time::zgetTime();

	boost::mutex::scoped_lock m_oProtectionLock(m_oPendingConnectionProtectionMutex);
	boost::mutex::scoped_lock m_oExecutionLock(m_oPendingConnectionExectutionMutex);
	m_oProtectionLock.unlock();

	SConnectionInfo l_oInfo;
	l_oInfo.m_ui64ConnectionTime=l_ui64Time;
	l_oInfo.m_ui64StimulationTimeOffset=0; // not used
	l_oInfo.m_ui64SignalSampleCountToSkip=0; // not used
	l_oInfo.m_pConnectionClientHandlerThread=NULL; // not used
	l_oInfo.m_pConnectionClientHandlerBoostThread=NULL; // not used
	l_oInfo.m_bChannelLocalisationSent = false;
	l_oInfo.m_bChannelUnitsSent = false;

	m_vPendingConnection.push_back(pair < Socket::IConnection*, SConnectionInfo > (pConnection, l_oInfo));

	for(std::vector<IAcquisitionServerPlugin*>::iterator itp = m_vPlugins.begin(); itp != m_vPlugins.end(); ++itp)
	{
		(*itp)->acceptNewConnectionHook();
	}

	return true;
}


