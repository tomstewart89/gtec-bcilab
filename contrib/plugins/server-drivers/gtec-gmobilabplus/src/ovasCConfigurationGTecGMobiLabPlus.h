/**
 * The gMobilab driver was contributed
 * by Lucie Daubigney from Supelec Metz
 */

#ifndef __OpenViBE_AcquisitionServer_ovasCConfigurationGTecGMobiLabPlus_H__
#define __OpenViBE_AcquisitionServer_ovasCConfigurationGTecGMobiLabPlus_H__

#include "../ovasCConfigurationBuilder.h"

#if defined TARGET_HAS_ThirdPartyGMobiLabPlusAPI

#ifndef GMOBILABPLUS_API
#ifdef TARGET_OS_Windows
#include <Windows.h>
#endif
#include <gMOBIlabplus.h>
#endif

#include <gtk/gtk.h>

namespace OpenViBEAcquisitionServer
{
	class CConfigurationGTecGMobiLabPlus : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:

		CConfigurationGTecGMobiLabPlus(const char* sGtkBuilderFileName, std::string& rPortName, OpenViBE::boolean& rTestMode);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

	private:

		std::string& m_rPortName;
		OpenViBE::boolean& m_rTestMode;

	};
};

#endif // TARGET_HAS_ThirdPartyGMobiLabPlusAPI

#endif
