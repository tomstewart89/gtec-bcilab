#ifndef __OpenViBE_AcquisitionServer_CConfigurationLabStreamingLayer_H__
#define __OpenViBE_AcquisitionServer_CConfigurationLabStreamingLayer_H__

#if defined(TARGET_HAS_ThirdPartyLSL)

#include "../ovasCConfigurationBuilder.h"
#include "ovasIDriver.h"
#include "ovasIHeader.h"

#include <gtk/gtk.h>
#include <lsl_cpp.h>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CConfigurationLabStreamingLayer
	 * \author Jussi T. Lindgren / Inria
	 * \date Wed Oct 15 09:41:18 2014
	 * \brief The CConfigurationLabStreamingLayer handles the configuration dialog specific to the LabStreamingLayer (LSL) device.
	 *
	 * \sa CDriverLabStreamingLayer
	 */
	class CConfigurationLabStreamingLayer : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:

		CConfigurationLabStreamingLayer(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName, 
			OpenViBEAcquisitionServer::IHeader& rHeader,
			OpenViBE::CString& rSignalStream,
			OpenViBE::CString& rSignalStreamID,
			OpenViBE::CString& rMarkerStream,
			OpenViBE::CString& rMarkerStreamID);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

	protected:

		OpenViBEAcquisitionServer::IDriverContext& m_rDriverContext;

	private:

		OpenViBEAcquisitionServer::IHeader& m_rHeader;

		OpenViBE::CString& m_rSignalStream;
		OpenViBE::CString& m_rSignalStreamID;
		OpenViBE::CString& m_rMarkerStream;
		OpenViBE::CString& m_rMarkerStreamID;

		std::vector<lsl::stream_info> m_vStreams;
		std::vector<OpenViBE::int32> m_vSignalIndex;
		std::vector<OpenViBE::int32> m_vMarkerIndex;
	};
};

#endif

#endif // __OpenViBE_AcquisitionServer_CConfigurationLabStreamingLayer_H__
