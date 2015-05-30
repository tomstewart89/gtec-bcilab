#ifndef __OpenViBE_AcquisitionServer_CConfigurationMKSNVXDriver_H__
#define __OpenViBE_AcquisitionServer_CConfigurationMKSNVXDriver_H__

#include "../../ovasCConfigurationBuilder.h"
#include "ovasIDriver.h"

#include <gtk/gtk.h>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CConfigurationMKSNVXDriver
	 * \author mkochetkov (MKS)
	 * \date Tue Jan 21 23:21:03 2014
	 * \brief The CConfigurationMKSNVXDriver handles the configuration dialog specific to the MKSNVXDriver device.
	 *
	 * TODO: details
	 *
	 * \sa CDriverMKSNVXDriver
	 */
	class CConfigurationMKSNVXDriver : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
		OpenViBE::uint32& dataMode_;
		bool& showAuxChannels_;
	protected:
		OpenViBEAcquisitionServer::IDriverContext& m_rDriverContext;
	public:
		// you may have to add to your constructor some reference parameters
		// for example, a connection ID:
		//CConfigurationMKSNVXDriver(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName, OpenViBE::uint32& rConnectionId);
		CConfigurationMKSNVXDriver(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName, OpenViBE::uint32& dataMode, bool& auxChannels);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);
	};
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationMKSNVXDriver_H__
