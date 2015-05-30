
#if defined(TARGET_HAS_ThirdPartyOgre3DTerrain)

#include "ovassvepCCommandFeedbackHandler.h"
#include "ovassvepCImpactApplication.h"

using namespace OpenViBESSVEP;

CCommandFeedbackHandler::CCommandFeedbackHandler(CImpactApplication* poApplication)
	: ICommandVRPNAnalog(poApplication, "SSVEP_VRPN_Feedback")
{
}

void CCommandFeedbackHandler::execute(int iChannelCount, double* pChannel)
{
	CImpactApplication* l_poImpactApplication = dynamic_cast<CImpactApplication*>(m_poApplication);

	l_poImpactApplication->calculateFeedback(iChannelCount, pChannel);
}

#endif