#ifndef __OpenViBE_AcquisitionServer_CConfigurationCognionics_H__
#define __OpenViBE_AcquisitionServer_CConfigurationCognionics_H__

#include "../ovasCConfigurationBuilder.h"
#include "ovasIDriver.h"

#include <gtk/gtk.h>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CConfigurationCognionics
	 * \author Mike Chi (Cognionics, Inc.)
	 * \copyright AGPL3
	 * \date Thu Apr 18 21:19:49 2013
	 * \brief The CConfigurationCognionics handles the configuration dialog specific to the Cognionics device.
	 *
	 * TODO: details
	 *
	 * \sa CDriverCognionics
	 */
	class CConfigurationCognionics : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:

		// you may have to add to your constructor some reference parameters
		// for example, a connection ID:
		//CConfigurationCognionics(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName, OpenViBE::uint32& rConnectionId);
		CConfigurationCognionics(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName, OpenViBE::uint32& rCOMPORT);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

		OpenViBE::uint32& m_rCOMPORT;


	protected:

		OpenViBEAcquisitionServer::IDriverContext& m_rDriverContext;
		

	private:

		/*
		 * Insert here all specific attributes, such as a connection ID.
		 * use references to directly modify the corresponding attribute of the driver
		 * Example:
		 */
		
	};
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationCognionics_H__
