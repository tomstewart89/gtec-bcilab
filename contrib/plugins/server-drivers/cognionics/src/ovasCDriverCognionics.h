#ifndef __OpenViBE_AcquisitionServer_CDriverCognionics_H__
#define __OpenViBE_AcquisitionServer_CDriverCognionics_H__

#include "ovasIDriver.h"
#include "../ovasCHeader.h"
#include <openvibe/ov_all.h>

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CDriverCognionics
	 * \author Mike Chi (Cognionics, Inc.)
	 * \copyright AGPL3
	 * \date Thu Apr 18 21:19:49 2013
	 
	 * \brief The CDriverCognionics allows the acquisition server to acquire data from a Cognionics device.
	 *
	 * TODO: details
	 *
	 * \sa CConfigurationCognionics
	 */
	class CDriverCognionics : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverCognionics(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverCognionics(void);
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

		// Replace this generic Header with any specific header you might have written
		OpenViBEAcquisitionServer::CHeader m_oHeader;

		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::float32* m_pSample;
	
	private:

		/*
		 * Insert here all specific attributes, such as USB port number or device ID.
		 * Example :
		 */
		OpenViBE::uint32 m_ui32ComPort;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverCognionics_H__
