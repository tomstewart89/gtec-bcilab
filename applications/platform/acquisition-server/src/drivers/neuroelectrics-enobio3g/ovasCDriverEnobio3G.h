#ifndef __OpenViBE_AcquisitionServer_CDriverEnobio3G_H__
#define __OpenViBE_AcquisitionServer_CDriverEnobio3G_H__

#if defined(TARGET_HAS_ThirdPartyEnobioAPI)

#include "ovasIDriver.h"
#include "../ovasCHeader.h"
#include <openvibe/ov_all.h>

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

// Including Enobio headers gave 4275 on 11.07.2014 w/ VS2010
#pragma warning(disable:4275)

#include "enobio3g.h"
#include "StatusData.h"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

#ifndef _ENOBIO_SAMPLE_RATE_
 #ifdef FREQ_SAMP
  #define _ENOBIO_SAMPLE_RATE_ FREQSAMP
 #else
  #define _ENOBIO_SAMPLE_RATE_ 500
 #endif
#endif

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CDriverEnobio3G
	 * \author Anton Albajes-Eizagirre (NeuroElectrics) anton.albajes-eizagirre@neuroelectrics.com
	 * \date Tue Apr 15 09:25:20 2014
	 * \brief The CDriverEnobio3G allows the acquisition server to acquire data from a Enobio3G device.
	 *
	 * TODO: details
	 *
	 * \sa CConfigurationEnobio3G
	 */
  class CDriverEnobio3G : public OpenViBEAcquisitionServer::IDriver, public IDataConsumer
	{
	public:

		CDriverEnobio3G(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverEnobio3G(void);
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

		// enobio registered consumers callbacks
		void receiveData(const PData &data);
		void newStatusFromDevice(const PData &data);

	protected:
		
		SettingsHelper m_oSettings;
		
		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;

		// Replace this generic Header with any specific header you might have written
		OpenViBEAcquisitionServer::CHeader m_oHeader;

		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		// sample buffers. We will have a set of buffers that will be cycled. 
		OpenViBE::float32** m_pSample;
	
	private:

		/*
		 * Insert here all specific attributes, such as USB port number or device ID.
		 * Example :
		 */
		// OpenViBE::uint32 m_ui32ConnectionID;
		OpenViBE::uint32 m_ui32nChannels; // Number of channels on the device reported by the device
		unsigned char *m_macAddress; // mac address of the device
		Enobio3G m_enobioDevice; // Enobio device class instantiation
		OpenViBE::uint32 m_ui32SampleRate; // sampling rate of the device
		OpenViBE::uint32 m_ui32bufHead; // writing header for the current buffer
		OpenViBE::uint32 m_ui32nBuffers; // number of buffers
		OpenViBE::uint32 m_ui32currentBuffer; // current buffer in use
		OpenViBE::uint32 m_ui32lastBufferFilled; // last buffer filled with data ready to be submitted
		OpenViBE::boolean m_bNewData; // if there is a new buffer with data ready to be submitted
		
		boost::mutex m_oMutex;
	};
};

#endif

#endif // __OpenViBE_AcquisitionServer_CDriverEnobio3G_H__

