#ifndef __OpenViBE_AcquisitionServer_CDriverLabStreamingLayer_H__
#define __OpenViBE_AcquisitionServer_CDriverLabStreamingLayer_H__

#if defined(TARGET_HAS_ThirdPartyLSL)

#include "ovasIDriver.h"
#include "../ovasCHeader.h"
#include <openvibe/ov_all.h>

#include <lsl_cpp.h>

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CDriverLabStreamingLayer
	 * \author Jussi T. Lindgren / Inria
	 * \date Wed Oct 15 09:41:18 2014
	 * \brief The CDriverLabStreamingLayer allows the acquisition server to acquire data from a LabStreamingLayer (LSL) device.
	 *
	 * \sa CConfigurationLabStreamingLayer
	 */
	class CDriverLabStreamingLayer : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverLabStreamingLayer(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverLabStreamingLayer(void);
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

		OpenViBEAcquisitionServer::CHeader m_oHeader;

	private:

		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::float32* m_pSample;
		OpenViBE::float32* m_pBuffer;

		OpenViBE::uint64 m_ui64StartTime;
		OpenViBE::uint64 m_ui64SampleCount;

		lsl::stream_info m_oSignalStream;
		lsl::stream_inlet* m_pSignalInlet;

		lsl::stream_info m_oMarkerStream;
		lsl::stream_inlet* m_pMarkerInlet;

		OpenViBE::CString m_sSignalStream;
		OpenViBE::CString m_sSignalStreamID;
		OpenViBE::CString m_sMarkerStream;
		OpenViBE::CString m_sMarkerStreamID;

	};
};

#endif

#endif // __OpenViBE_AcquisitionServer_CDriverLabStreamingLayer_H__
