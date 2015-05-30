#ifndef __OpenViBE_AcquisitionServer_CDriverGTecGUSBampLegacy_H__
#define __OpenViBE_AcquisitionServer_CDriverGTecGUSBampLegacy_H__

#if defined TARGET_HAS_ThirdPartyGUSBampCAPI

#include "ovasIDriver.h"
#include "../ovasCHeader.h"

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

#include <gtk/gtk.h>
#include <vector>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CDriverGTecGUSBampLegacy
	 * \author Unknown
	 * \date unknown
	 * \brief GTEC driver 
	 *
	 */
	class CDriverGTecGUSBampLegacy : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverGTecGUSBampLegacy(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual void release(void);
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

	protected:

		SettingsHelper m_oSettings;

		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;
		OpenViBEAcquisitionServer::CHeader m_oHeader;

		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::uint32 m_ui32DeviceIndex;
		OpenViBE::uint32 m_ui32ActualDeviceIndex;
		OpenViBE::uint32 m_ui32BufferSize;
		OpenViBE::uint8* m_pBuffer;
		OpenViBE::float32* m_pSampleTranspose;
		OpenViBE::float32* m_pSample;
		void* m_pDevice;
		void* m_pEvent;
		void* m_pOverlapped;

		OpenViBE::uint32 m_ui32ActualImpedanceIndex;

		OpenViBE::uint8 m_ui8CommonGndAndRefBitmap;

		OpenViBE::int32 m_i32NotchFilterIndex;
		OpenViBE::int32 m_i32BandPassFilterIndex;

		OpenViBE::boolean m_bTriggerInputEnabled;
		OpenViBE::uint32 m_ui32LastStimulation;

		// EVENT CHANNEL : contribution Anton Andreev (Gipsa-lab) - 0.14.0
		typedef enum
		{
			STIMULATION_0	= 0,
			STIMULATION_64	= 64,
			STIMULATION_128	= 128,
			STIMULATION_192	= 192
		} gtec_triggers_t;

		OpenViBE::uint32 m_ui32TotalHardwareStimulations; //since start button clicked
		OpenViBE::uint32 m_ui32TotalDriverChunksLost; //since start button clicked
		OpenViBE::uint32 m_ui32AcquiredChannelCount; //number of channels 1..16 specified bu user

	};
};

#endif // TARGET_HAS_ThirdPartyGUSBampCAPI

#endif // __OpenViBE_AcquisitionServer_CDriverGTecGUSBampLegacy_H__
