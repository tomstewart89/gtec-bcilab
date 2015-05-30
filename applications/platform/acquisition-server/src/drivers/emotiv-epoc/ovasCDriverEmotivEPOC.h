#ifndef __OpenViBE_AcquisitionServer_CDriverEmotivEPOC_H__
#define __OpenViBE_AcquisitionServer_CDriverEmotivEPOC_H__

#if defined TARGET_HAS_ThirdPartyEmotivAPI

#include "ovasIDriver.h"
#include "../ovasCHeader.h"

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

#include <openvibe/ov_all.h>

#include "EmoStateDLL.h"
#include "edk.h"
#include "edkErrorCode.h"

#include <vector>

#include <windows.h>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CDriverEmotivEPOC
	 * \author Laurent Bonnet (INRIA)
	 * \date 21 july 2010
	 * \erief The CDriverEmotivEPOC allows the acquisition server to acquire data from a Emotiv EPOC amplifier, Research Edition or above.
	 *
	 */
	class CDriverEmotivEPOC : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverEmotivEPOC(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverEmotivEPOC(void);
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
		OpenViBE::uint32 m_ui32TotalSampleCount;
		OpenViBE::float32* m_pSample;
		//OpenViBE::float64* m_pBuffer;

	private:

		OpenViBE::uint32 m_ui32EDK_LastErrorCode;

		EmoEngineEventHandle m_tEEEventHandle;
		OpenViBE::uint32 m_ui32UserID;
		OpenViBE::boolean m_bReadyToCollect;

		DataHandle m_tDataHandle;
		OpenViBE::boolean m_bFirstStart;

		OpenViBE::boolean m_bUseGyroscope;
		OpenViBE::CString m_sPathToEmotivSDK;
		OpenViBE::CString m_sCommandForPathModification;
		virtual OpenViBE::boolean buildPath(void);

	};
};

#endif // TARGET_HAS_ThirdPartyEmotivAPI

#endif // __OpenViBE_AcquisitionServer_CDriverEmotivEPOC_H__
