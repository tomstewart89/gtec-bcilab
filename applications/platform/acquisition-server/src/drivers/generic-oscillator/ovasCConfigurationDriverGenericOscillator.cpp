#include "ovasCConfigurationDriverGenericOscillator.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace std;

CConfigurationDriverGenericOscillator::CConfigurationDriverGenericOscillator(IDriverContext& rDriverContext, const char* sGtkBuilderFileName, boolean& rSendPeriodicStimulations)
	:CConfigurationBuilder(sGtkBuilderFileName)
	 ,m_rDriverContext(rDriverContext)
	 ,m_rSendPeriodicStimulations(rSendPeriodicStimulations)
{
}

boolean CConfigurationDriverGenericOscillator::preConfigure(void)
{
	if (!CConfigurationBuilder::preConfigure())
	{
		return false;
	}

	::GtkToggleButton* l_pToggleSendPeriodicStimulations = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_send_periodic_stimulations"));

	gtk_toggle_button_set_active(l_pToggleSendPeriodicStimulations, m_rSendPeriodicStimulations);

	return true;
}

boolean CConfigurationDriverGenericOscillator::postConfigure(void)
{
	if (m_bApplyConfiguration)
	{
		::GtkToggleButton* l_pToggleSendPeriodicStimulations = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_send_periodic_stimulations"));

		m_rSendPeriodicStimulations = (::gtk_toggle_button_get_active(l_pToggleSendPeriodicStimulations)>0);
	}

	if (!CConfigurationBuilder::postConfigure())
	{
		return false;
	}

	return true;
}

