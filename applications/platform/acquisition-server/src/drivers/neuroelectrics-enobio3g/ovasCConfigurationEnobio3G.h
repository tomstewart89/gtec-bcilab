#ifndef __OpenViBE_AcquisitionServer_CConfigurationEnobio3G_H__
#define __OpenViBE_AcquisitionServer_CConfigurationEnobio3G_H__

#include "../ovasCConfigurationBuilder.h"
#include "ovasIDriver.h"

#include <gtk/gtk.h>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CConfigurationEnobio3G
	 * \author Anton Albajes-Eizagirre (NeuroElectrics anton.albajes-eizagirre@neuroelectrics.com)
	 * \date Tue Apr 15 09:25:20 2014
	 * \brief The CConfigurationEnobio3G handles the configuration dialog specific to the Enobio3G device.
	 *
	 * TODO: details
	 *
	 * \sa CDriverEnobio3G
	 */
	class CConfigurationEnobio3G : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:

		// you may have to add to your constructor some reference parameters
		// for example, a connection ID:
		//CConfigurationEnobio3G(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName, OpenViBE::uint32& rConnectionId);
		CConfigurationEnobio3G(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);
		unsigned char * getMacAddress();

	protected:

		OpenViBEAcquisitionServer::IDriverContext& m_rDriverContext;

	private:

		/*
		 * Insert here all specific attributes, such as a connection ID.
		 * use references to directly modify the corresponding attribute of the driver
		 * Example:
		 */
		// OpenViBE::uint32& m_ui32ConnectionID;
		unsigned char m_macAddress[6]; // mac address of the device
	};
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationEnobio3G_H__
