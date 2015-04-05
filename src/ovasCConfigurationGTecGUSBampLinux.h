#ifndef __OpenViBE_AcquisitionServer_CConfigurationGTecGUSBampLinux_H__
#define __OpenViBE_AcquisitionServer_CConfigurationGTecGUSBampLinux_H__

#if defined TARGET_HAS_ThirdPartyGUSBampCAPI_Linux

#include "../ovasCConfigurationBuilder.h"
#include "ovasIDriver.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gAPI.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CConfigurationGTecGUSBampLinux
	 * \author Tom Stewart (Tsukuba University)
	 * \date Mon Feb  9 18:59:22 2015
	 * \brief The CConfigurationGTecGUSBampLinux handles the configuration dialog specific to the g.tec g.USBamp for Linux device.
	 *
	 * TODO: details
	 *
	 * \sa CDriverGTecGUSBampLinux
	 */
	class CConfigurationGTecGUSBampLinux : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:
        // Thresholds for reporting on measured impedance, these are the same as the ones that the simulink driver uses
        static const int LowImpedance = 5, ModerateImpedance = 7, HighImpedance = 100;
        enum ChannelTreeViewColumn { ChannelColumn = 0, BipolarColumn, NotchColumn, NotchIdColumn, BandpassColumn, BandpassIdColumn };
        // you may have to add to your constructor some reference parameters
		// for example, a connection ID:
        CConfigurationGTecGUSBampLinux(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName, std::string *pDeviceName, gt_usbamp_config *pConfig);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

        void OnButtonApplyConfigPressed(ChannelTreeViewColumn type);
        void OnButtonCheckImpedanceClicked(void);
        void OnComboboxSamplingFrequencyChanged(void);
        void OnCheckButtonDIOToggled(void);

    protected:
        OpenViBEAcquisitionServer::IDriverContext& m_rDriverContext;

	private:
		/*
		 * Insert here all specific attributes, such as a connection ID.
		 * use references to directly modify the corresponding attribute of the driver
		 * Example:
         */
        std::string *m_pDeviceName;
        gt_usbamp_config *m_pConfig;
	};
};

#endif // TARGET_HAS_ThirdPartyGUSBampCAPI_Linux

#endif // __OpenViBE_AcquisitionServer_CConfigurationGTecGUSBampLinux_H__
