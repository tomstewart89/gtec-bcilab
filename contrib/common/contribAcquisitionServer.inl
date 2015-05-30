/*
#include "openeeg-modulareeg/src/ovasCDriverOpenEEGModularEEG.h"
#include "field-trip-protocol/src/ovasCDriverFieldtrip.h"
#include "brainproducts-brainvisionrecorder/src/ovasCDriverBrainProductsBrainVisionRecorder.h"
*/

#include "ovasCPluginExternalStimulations.h"

#include "ovasCDriverBrainmasterDiscovery.h"
#include "ovasCDriverBrainProductsBrainVisionRecorder.h"
#include "ovasCDriverCognionics.h"
#include "ovasCDriverCtfVsmMeg.h"
#include "ovasCDriverGTecGUSBamp.h"
#include "ovasCDriverGTecGUSBampLegacy.h"
#include "ovasCDriverGTecGUSBampLinux.h"
#include "ovasCDriverGTecGMobiLabPlus.h"
#include "ovasCDriverFieldtrip.h"
#include "ovasCDriverMBTSmarting.h"
#include "ovasCDriverMitsarEEG202A.h"
#include "ovasCDriverOpenALAudioCapture.h"
#include "ovasCDriverOpenEEGModularEEG.h"
#include "ovasCDriverOpenBCI.h"

namespace OpenViBEContributions {




	void initiateContributions(OpenViBEAcquisitionServer::CAcquisitionServerGUI* pGUI, OpenViBEAcquisitionServer::CAcquisitionServer* pAcquisitionServer, const OpenViBE::Kernel::IKernelContext& rKernelContext, std::vector<OpenViBEAcquisitionServer::IDriver*>* vDriver)
	{
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverBrainProductsBrainVisionRecorder(pAcquisitionServer->getDriverContext()));
#if defined WIN32
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverCognionics(pAcquisitionServer->getDriverContext()));
#endif
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverCtfVsmMeg(pAcquisitionServer->getDriverContext()));
#if defined TARGET_HAS_ThirdPartyGUSBampCAPI
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverGTecGUSBamp(pAcquisitionServer->getDriverContext()));
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverGTecGUSBampLegacy(pAcquisitionServer->getDriverContext()));
#endif
#if defined TARGET_HAS_ThirdPartyGUSBampCAPI_Linux
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverGTecGUSBampLinux(pAcquisitionServer->getDriverContext()));
#endif
#if defined TARGET_HAS_ThirdPartyGMobiLabPlusAPI
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverGTecGMobiLabPlus(pAcquisitionServer->getDriverContext()));
#endif
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverFieldtrip(pAcquisitionServer->getDriverContext()));

#if defined TARGET_HAS_ThirdPartyBrainmasterCodeMakerAPI
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverBrainmasterDiscovery(pAcquisitionServer->getDriverContext()));
#endif
 
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverMBTSmarting(pAcquisitionServer->getDriverContext()));
		
#if defined(TARGET_HAS_ThirdPartyMitsar)
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverMitsarEEG202A(pAcquisitionServer->getDriverContext()));
#endif

#if defined TARGET_HAS_ThirdPartyOpenAL
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverOpenALAudioCapture(pAcquisitionServer->getDriverContext()));
#endif

		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverOpenEEGModularEEG(pAcquisitionServer->getDriverContext()));
		
		vDriver->push_back(new OpenViBEAcquisitionServer::CDriverOpenBCI(pAcquisitionServer->getDriverContext()));

		pGUI->registerPlugin(new OpenViBEAcquisitionServer::OpenViBEAcquisitionServerPlugins::CPluginExternalStimulations(rKernelContext));
	}

}
