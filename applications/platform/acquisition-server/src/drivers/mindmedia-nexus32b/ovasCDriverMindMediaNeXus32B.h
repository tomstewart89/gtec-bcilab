#ifndef __OpenViBE_AcquisitionServer_CDriverMindMediaNeXus32B_H__
#define __OpenViBE_AcquisitionServer_CDriverMindMediaNeXus32B_H__

#if defined(TARGET_HAS_ThirdPartyNeXus)

#include "ovasIDriver.h"
#include "../ovasCHeader.h"

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

#if defined TARGET_OS_Windows

namespace OpenViBEAcquisitionServer
{
	class CDriverMindMediaNeXus32B : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverMindMediaNeXus32B(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
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

		virtual void processData(OpenViBE::uint32 ui32SampleCount, OpenViBE::uint32 ui32Channel, OpenViBE::float32* pSample);

	protected:
		
		SettingsHelper m_oSettings;

		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;
		OpenViBEAcquisitionServer::CHeader m_oHeader;

		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::float32* m_pSample;
		OpenViBE::uint32 m_ui32SampleIndex;

	};
};

#endif // defined TARGET_OS_Windows
#endif

#endif // __OpenViBE_AcquisitionServer_CDriverMindMediaNeXus32B_H__
