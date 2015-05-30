#ifndef __OpenViBE_AcquisitionServer_CConfigurationDriverGenericOscillator_H__
#define __OpenViBE_AcquisitionServer_CConfigurationDriverGenericOscillator_H__

#include "../ovasCConfigurationBuilder.h"
#include "ovasIDriver.h"

#include <gtk/gtk.h>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CConfigurationDriverGenericOscillator
	 * \author Jozef Legeny (Inria)
	 * \date 28 jan 2013
	 * \brief The CConfigurationDriverGenericOscillator handles the configuration dialog specific to the Generic Oscillator driver
	 *
	 * \sa CDriverGenericOscillator
	 */

	class CConfigurationDriverGenericOscillator : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
		public:
			CConfigurationDriverGenericOscillator(OpenViBEAcquisitionServer::IDriverContext& rDriverContext, const char* sGtkBuilderFileName, OpenViBE::boolean& rSendPeriodicStimulations);

			virtual OpenViBE::boolean preConfigure(void);
			virtual OpenViBE::boolean postConfigure(void);

		protected:

		OpenViBEAcquisitionServer::IDriverContext& m_rDriverContext;

		OpenViBE::boolean& m_rSendPeriodicStimulations;

	};
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationDriverGenericOscillator_H__

