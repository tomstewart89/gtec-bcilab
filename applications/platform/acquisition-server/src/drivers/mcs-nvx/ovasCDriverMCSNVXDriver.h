
#if defined(TARGET_HAS_ThirdPartyMCS)

#ifndef __OpenViBE_AcquisitionServer_CDriverMKSNVXDriver_H__
#define __OpenViBE_AcquisitionServer_CDriverMKSNVXDriver_H__

#include <vector>
#include "ovasIDriver.h"
#include "../../ovasCHeader.h"
#include <openvibe/ov_all.h>

#include "../../ovasCSettingsHelper.h"
#include "../../ovasCSettingsHelperOperators.h"
#include "NVX.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CDriverMKSNVXDriver
	 * \author mkochetkov (MKS)
	 * \date Tue Jan 21 23:21:03 2014
	 * \brief The CDriverMKSNVXDriver allows the acquisition server to acquire data from a MKSNVXDriver device.
	 *
	 * TODO: details
	 *
	 * \sa CConfigurationMKSNVXDriver
	 */
	const size_t maxNumberOfChannels = 36;
	class CDriverMKSNVXDriver : public OpenViBEAcquisitionServer::IDriver
	{
		OpenViBE::uint8 deviceBuffer_[1024*maxNumberOfChannels];
		int nvxDeviceId_;
		t_NVXDataSettins nvxDataSettings_;
		int nvxDataModel_;
		OpenViBE::uint32 dataMode_; // normal, test, impedance
		t_NVXProperty nvxProperty_;
		unsigned int samplesCounter_; // previous t_NVXDataModelXX.Counter
		OpenViBE::uint32 triggerStates_;
		bool showAuxChannels_;
		t_NVXInformation nvxInfo_;
	public:

		CDriverMKSNVXDriver(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		size_t getDeviceBufferSamplesCapacity() const {
			return sizeof(deviceBuffer_) / sizeof(t_NVXDataModel36) * maxNumberOfChannels; // I really do not know if it is accurate for all modes.
		}
		virtual ~CDriverMKSNVXDriver(void);
		virtual const char* getName(void);

		virtual OpenViBE::boolean initialize(
			const OpenViBE::uint32 ui32SampleCountPerSentBlock,
			OpenViBEAcquisitionServer::IDriverCallback& rCallback);
		virtual OpenViBE::boolean uninitialize(void);

		virtual OpenViBE::boolean start(void);
		virtual OpenViBE::boolean stop(void);
		virtual OpenViBE::boolean loop(void);

		virtual OpenViBE::boolean isConfigurable(void);
		virtual OpenViBE::boolean configure(void);
		virtual const OpenViBEAcquisitionServer::IHeader* getHeader(void) { return &m_oHeader; }
		
		virtual OpenViBE::boolean isFlagSet(
			const OpenViBEAcquisitionServer::EDriverFlag eFlag) const
		{
			return eFlag==DriverFlag_IsUnstable;
		}

	protected:
		
		SettingsHelper m_oSettings;
		
		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;

		// Replace this generic Header with any specific header you might have written
		OpenViBEAcquisitionServer::CHeader m_oHeader;

		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		std::vector<OpenViBE::float32> sampleData_;
	private:

		/*
		 * Insert here all specific attributes, such as USB port number or device ID.
		 * Example :
		 */
		// OpenViBE::uint32 m_ui32ConnectionID;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverMKSNVXDriver_H__

#endif