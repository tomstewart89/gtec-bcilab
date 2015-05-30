#ifndef __OpenViBE_AcquisitionServer_CAcquisitionServer_H__
#define __OpenViBE_AcquisitionServer_CAcquisitionServer_H__

#include "ovas_base.h"
#include "ovasIDriver.h"
#include "ovasIHeader.h"

#include <socket/IConnectionServer.h>

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/version.hpp>

#include <string>
#include <vector>
#include <list>
#include <deque>



namespace OpenViBEAcquisitionServer
{
	class CConnectionServerHandlerThread;
	class CConnectionClientHandlerThread;
	class IAcquisitionServerPlugin;

	typedef struct
	{
		OpenViBE::uint64 m_ui64ConnectionTime;
		OpenViBE::uint64 m_ui64StimulationTimeOffset;
		OpenViBE::uint64 m_ui64SignalSampleCountToSkip;
		CConnectionClientHandlerThread* m_pConnectionClientHandlerThread;
		boost::thread* m_pConnectionClientHandlerBoostThread;
		bool m_bChannelUnitsSent; 
		bool m_bChannelLocalisationSent;
	} SConnectionInfo;

	typedef enum
	{
		DriftCorrectionPolicy_DriverChoice=0,
		DriftCorrectionPolicy_Forced,
		DriftCorrectionPolicy_Disabled,
	} EDriftCorrectionPolicy;

	typedef enum
	{
		NaNReplacementPolicy_LastCorrectValue=0,
		NaNReplacementPolicy_Zero,
		NaNReplacementPolicy_Disabled,
	} ENaNReplacementPolicy;

	class CDriverContext;
	class CAcquisitionServer : public OpenViBEAcquisitionServer::IDriverCallback
	{
	public:

		CAcquisitionServer(const OpenViBE::Kernel::IKernelContext& rKernelContext);
		virtual ~CAcquisitionServer(void);

		virtual OpenViBEAcquisitionServer::IDriverContext& getDriverContext();

		OpenViBE::uint32 getClientCount(void);
		OpenViBE::float64 getDrift(void);
		OpenViBE::float64 getImpedance(const OpenViBE::uint32 ui32ChannelIndex);

		OpenViBE::boolean loop(void);

		OpenViBE::boolean connect(OpenViBEAcquisitionServer::IDriver& rDriver, OpenViBEAcquisitionServer::IHeader& rHeaderCopy, OpenViBE::uint32 ui32SamplingCountPerSentBlock, OpenViBE::uint32 ui32ConnectionPort);
		OpenViBE::boolean start(void);
		OpenViBE::boolean stop(void);
		OpenViBE::boolean disconnect(void);

		// Driver samples information callback
		virtual void setSamples(const OpenViBE::float32* pSample);
		virtual void setSamples(const OpenViBE::float32* pSample, const OpenViBE::uint32 ui32SampleCount);
		virtual void setStimulationSet(const OpenViBE::IStimulationSet& rStimulationSet);

		// Driver context callback
		virtual OpenViBE::boolean isConnected(void) const { return m_bInitialized; }
		virtual OpenViBE::boolean isStarted(void) const { return m_bStarted; }
		virtual OpenViBE::int64 getDriftSampleCount(void) const { return m_eDriftCorrectionPolicy==DriftCorrectionPolicy_Disabled?0:m_i64DriftSampleCount; }
		virtual OpenViBE::int64 getDriftToleranceSampleCount(void) const { return m_i64DriftToleranceSampleCount; }
		virtual OpenViBE::boolean correctDriftSampleCount(OpenViBE::int64 i64SampleCount);
		virtual OpenViBE::int64 getSuggestedDriftCorrectionSampleCount(void) const;
		virtual OpenViBE::boolean setInnerLatencySampleCount(OpenViBE::int64 i64SampleCount);
		virtual OpenViBE::int64 getInnerLatencySampleCount(void) const;
		virtual OpenViBE::boolean updateImpedance(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::float64 f64Impedance);

		// General parameters configurable from the GUI
		OpenViBEAcquisitionServer::ENaNReplacementPolicy getNaNReplacementPolicy(void);
		OpenViBE::CString getNaNReplacementPolicyStr(void);
		OpenViBEAcquisitionServer::EDriftCorrectionPolicy getDriftCorrectionPolicy(void);
		OpenViBE::CString getDriftCorrectionPolicyStr(void);
		OpenViBE::uint64 getDriftToleranceDuration(void);
		OpenViBE::uint64 getJitterEstimationCountForDrift(void);
		OpenViBE::uint64 getOversamplingFactor(void);
		OpenViBE::boolean setNaNReplacementPolicy(OpenViBEAcquisitionServer::ENaNReplacementPolicy eNaNReplacementPolicy);
		OpenViBE::boolean setDriftCorrectionPolicy(OpenViBEAcquisitionServer::EDriftCorrectionPolicy eDriftCorrectionPolicy);
		OpenViBE::boolean isImpedanceCheckRequested(void);
		OpenViBE::boolean isChannelSelectionRequested(void);
		OpenViBE::boolean setDriftToleranceDuration(OpenViBE::uint64 ui64DriftToleranceDuration);
		OpenViBE::boolean setJitterEstimationCountForDrift(OpenViBE::uint64 ui64JitterEstimationCountForDrift);
		OpenViBE::boolean setOversamplingFactor(OpenViBE::uint64 ui64OversamplingFactor);
		OpenViBE::boolean setImpedanceCheckRequest(OpenViBE::boolean bActive);
		OpenViBE::boolean setChannelSelectionRequest(OpenViBE::boolean bActive);

		std::vector<IAcquisitionServerPlugin*> getPlugins() { return m_vPlugins; }

		//
		virtual OpenViBE::boolean acceptNewConnection(Socket::IConnection* pConnection);

	public:

		boost::mutex m_oExecutionMutex;
		boost::mutex m_oProtectionMutex;

		boost::mutex m_oPendingConnectionExectutionMutex;
		boost::mutex m_oPendingConnectionProtectionMutex;

		boost::thread* m_pConnectionServerHandlerBoostThread;

	public:

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBEAcquisitionServer::CDriverContext* m_pDriverContext;
		OpenViBEAcquisitionServer::IDriver* m_pDriver;
		const OpenViBEAcquisitionServer::IHeader* m_pHeaderCopy;

		OpenViBE::Kernel::IAlgorithmProxy* m_pStreamEncoder;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_ui64SubjectIdentifier;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_ui64SubjectAge;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_ui64SubjectGender;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > ip_pSignalMatrix;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > ip_pChannelUnits;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_ui64SignalSamplingRate;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* > ip_pStimulationSet;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_ui64BufferDuration;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pEncodedMemoryBuffer;

		OpenViBE::Kernel::TParameterHandler < OpenViBE::boolean > ip_bEncodeChannelLocalisationData;
		OpenViBE::Kernel::TParameterHandler < OpenViBE::boolean > ip_bEncodeChannelUnitData;

		std::list < std::pair < Socket::IConnection*, SConnectionInfo > > m_vConnection;
		std::list < std::pair < Socket::IConnection*, SConnectionInfo > > m_vPendingConnection;
		std::vector < std::vector < OpenViBE::float32 > > m_vPendingBuffer;
		std::vector < OpenViBE::float32 > m_vSwapBuffer;
		std::vector < OpenViBE::float32 > m_vOverSamplingSwapBuffer;
		std::vector < OpenViBE::float64 > m_vImpedance;
		std::vector < OpenViBE::uint32 >  m_vSelectedChannels;
		std::vector < OpenViBE::CString >  m_vSelectedChannelNames;
		Socket::IConnectionServer* m_pConnectionServer;

		OpenViBEAcquisitionServer::EDriftCorrectionPolicy m_eDriftCorrectionPolicy;

		OpenViBEAcquisitionServer::ENaNReplacementPolicy m_eNaNReplacementPolicy;
		OpenViBE::boolean m_bReplacementInProgress;

		OpenViBE::boolean m_bInitialized;
		OpenViBE::boolean m_bStarted;
		OpenViBE::boolean m_bIsImpedanceCheckRequested;
		OpenViBE::boolean m_bIsChannelSelectionRequested;
		OpenViBE::boolean m_bGotData;
		OpenViBE::boolean m_bDriftCorrectionCalled;
		OpenViBE::uint64 m_ui64OverSamplingFactor;
		OpenViBE::uint32 m_ui32ChannelCount;
		OpenViBE::uint32 m_ui32SamplingFrequency;
		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::uint64 m_ui64SampleCount;
		OpenViBE::uint64 m_ui64LastSampleCount;
		OpenViBE::uint64 m_ui64StartTime;
		OpenViBE::uint64 m_ui64LastDeliveryTime;

		std::list < OpenViBE::int64 > m_vJitterSampleCount;
		OpenViBE::uint64 m_ui64DriftToleranceDuration;
		OpenViBE::float64 m_f64DriftSampleCount;
		OpenViBE::int64 m_i64DriftSampleCount;
		OpenViBE::int64 m_i64DriftToleranceSampleCount;
		OpenViBE::int64 m_i64DriftCorrectionSampleCountAdded;
		OpenViBE::int64 m_i64DriftCorrectionSampleCountRemoved;
		OpenViBE::int64 m_i64InnerLatencySampleCount;

		OpenViBE::uint64 m_ui64JitterEstimationCountForDrift;
		OpenViBE::uint64 m_ui64DriverTimeoutDuration;
		OpenViBE::uint64 m_ui64StartedDriverSleepDuration;
		OpenViBE::uint64 m_ui64StoppedDriverSleepDuration;

		OpenViBE::uint8* m_pSampleBuffer;
		OpenViBE::CStimulationSet m_oPendingStimulationSet;

		std::vector<IAcquisitionServerPlugin*> m_vPlugins;
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcquisitionServer_H__
