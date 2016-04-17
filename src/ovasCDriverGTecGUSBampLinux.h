#ifndef __OpenViBE_AcquisitionServer_CDriverGTecGUSBampLinux_H__
#define __OpenViBE_AcquisitionServer_CDriverGTecGUSBampLinux_H__

#if defined TARGET_HAS_ThirdPartyGUSBampCAPI_Linux

#include "ovasIDriver.h"
#include "../ovasCHeader.h"
#include <openvibe/ov_all.h>

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

#include <gAPI.h>
#include "Queue.h"

namespace OpenViBEAcquisitionServer
{
    void OnDataReady(void *param);

	/**
	 * \class CDriverGTecGUSBampLinux
	 * \author Tom Stewart (University of Tsukuba)
	 * \date Mon Feb  9 18:59:22 2015
	 * \brief The CDriverGTecGUSBampLinux allows the acquisition server to acquire data from a g.tec g.USBamp from Linux.
	 *
	 * \sa CConfigurationGTecGUSBampLinux
	 */
	class CDriverGTecGUSBampLinux : public OpenViBEAcquisitionServer::IDriver
	{
        static const int ReceiveBufferSize = 8192;
	public:
        friend void OnDataReady(void *param);

		CDriverGTecGUSBampLinux(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverGTecGUSBampLinux(void);
		virtual const char* getName(void);

		virtual OpenViBE::boolean initialize(const OpenViBE::uint32 ui32SampleCountPerSentBlock, OpenViBEAcquisitionServer::IDriverCallback& rCallback);
		virtual OpenViBE::boolean uninitialize(void);

		virtual OpenViBE::boolean start(void);
		virtual OpenViBE::boolean stop(void);
		virtual OpenViBE::boolean loop(void);

		virtual OpenViBE::boolean isConfigurable(void);
		virtual OpenViBE::boolean configure(void);
		virtual const OpenViBEAcquisitionServer::IHeader* getHeader(void) { return &m_oHeader; }
		
		virtual OpenViBE::boolean isFlagSet(const OpenViBEAcquisitionServer::EDriverFlag eFlag) const
		{
			return eFlag==DriverFlag_IsUnstable;
		}

	protected:
		
		SettingsHelper m_oSettings;
		
		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;

		// Replace this generic Header with any specific header you might have written
		OpenViBEAcquisitionServer::CHeader m_oHeader;

        OpenViBE::uint32 m_ui32SampleCountPerSentBlock;

        OpenViBE::float32 *m_pSampleSend, *m_pSampleReceive, *m_pSampleBuffer;
		Queue<OpenViBE::float32> m_oSampleQueue;
	private:

		/*
		 * Insert here all specific attributes, such as USB port number or device ID.
		 */
        std::string m_oDeviceName;
        gt_usbamp_config m_oConfig;
        gt_usbamp_analog_out_config m_oAnalogOutConfig;

        // Keeps track of where we are with filling up the buffer
        OpenViBE::uint32 m_ui32CurrentSample, m_ui32CurrentChannel;

	};
};

#endif // TARGET_HAS_ThirdPartyGUSBampCAPI_Linux

#endif // __OpenViBE_AcquisitionServer_CDriverGTecGUSBampLinux_H__
