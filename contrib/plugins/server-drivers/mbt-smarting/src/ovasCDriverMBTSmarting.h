#ifndef __OpenViBE_AcquisitionServer_CDriverMBTSmarting_H__
#define __OpenViBE_AcquisitionServer_CDriverMBTSmarting_H__

#include "ovasIDriver.h"
#include "../ovasCHeader.h"
#include <openvibe/ov_all.h>

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

#include <boost/shared_ptr.hpp>
#include "ovasCSmartingAmp.h"

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CDriverMBTSmarting
	 * \author mBrainTrain dev team (mBrainTrain)
	 * \date Tue Oct 14 16:09:43 2014
	 * \brief The CDriverMBTSmarting allows the acquisition server to acquire data from a MBTSmarting device.
	 *
	 * TODO: details
	 *
	 * \sa CConfigurationMBTSmarting
	 */
	class CDriverMBTSmarting : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverMBTSmarting(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverMBTSmarting(void);
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
	#ifdef TARGET_OS_Windows
		return false;
	#elif defined TARGET_OS_Linux
		return true;
	#endif
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
		OpenViBE::uint32 m_ui32ConnectionID;
		boost::shared_ptr< SmartingAmp > m_pSmartingAmp;
		std::vector< unsigned char > m_byteArray;
		int sample_number;
		int latency;
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverMBTSmarting_H__