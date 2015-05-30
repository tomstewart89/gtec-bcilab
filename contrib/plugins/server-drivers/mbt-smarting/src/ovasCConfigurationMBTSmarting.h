#ifndef __OpenViBE_AcquisitionServer_CConfigurationMBTSmarting_H__
#define __OpenViBE_AcquisitionServer_CConfigurationMBTSmarting_H__

#include "../ovasCConfigurationBuilder.h"
#include "ovasIDriver.h"

#include <gtk/gtk.h>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CConfigurationMBTSmarting
	 * \author mBrainTrain dev team (mBrainTrain)
	 * \date Tue Oct 14 16:09:43 2014
	 * \brief The CConfigurationMBTSmarting handles the configuration dialog specific to the MBTSmarting device.
	 *
	 * TODO: details
	 *
	 * \sa CDriverMBTSmarting
	 */
	class CConfigurationMBTSmarting : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:

		// you may have to add to your constructor some reference parameters
		// for example, a connection ID:
		CConfigurationMBTSmarting(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName, OpenViBE::uint32& rConnectionId);
		//CConfigurationMBTSmarting(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

		virtual ~CConfigurationMBTSmarting(void);

	protected:

		OpenViBEAcquisitionServer::IDriverContext& m_rDriverContext;

	private:

		/*
		 * Insert here all specific attributes, such as a connection ID.
		 * use references to directly modify the corresponding attribute of the driver
		 * Example:
		 */
		OpenViBE::uint32& m_ui32ConnectionID;
		//::GtkListStore* m_pListStore;
	};
};


#endif // __OpenViBE_AcquisitionServer_CConfigurationMBTSmarting_H__
