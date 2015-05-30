
#include "ovasCSettingsHelper.h"


// Save all registered variables to the configuration manager
void OpenViBEAcquisitionServer::SettingsHelper::save(void) 
{
	std::map<OpenViBE::CString, Property*>::const_iterator it = m_vProperties.begin();

	for(;it!=m_vProperties.end();++it) {
		std::stringstream ss;
			
		ss << *(it->second);

		// m_rContext.getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Token " << it->first << " wrote [" << ss.str().c_str() << "]\n";
		const OpenViBE::CString l_sTokenName = m_sPrefix + OpenViBE::CString("_") + it->first;
		OpenViBE::CIdentifier l_oTokenId = m_rConfigurationManager.lookUpConfigurationTokenIdentifier(l_sTokenName);
		if(l_oTokenId == OV_UndefinedIdentifier) 
		{
			m_rConfigurationManager.createConfigurationToken(m_sPrefix + OpenViBE::CString("_") + it->first, OpenViBE::CString(ss.str().c_str()));
		} 
		else 
		{
			// replacing token value
			m_rConfigurationManager.setConfigurationTokenValue(l_oTokenId, OpenViBE::CString(ss.str().c_str()));
		}
	}
		
}

// Load all registered variables from the configuration manager
void OpenViBEAcquisitionServer::SettingsHelper::load(void) 
{
	std::map<OpenViBE::CString, Property*>::const_iterator it = m_vProperties.begin();
	for(;it!=m_vProperties.end();++it) {
		const OpenViBE::CString l_sTokenName = m_sPrefix + OpenViBE::CString("_") + it->first;
		if(m_rConfigurationManager.lookUpConfigurationTokenIdentifier(l_sTokenName) != OV_UndefinedIdentifier) {
				
			const OpenViBE::CString value = m_rConfigurationManager.expand(OpenViBE::CString("${") + l_sTokenName + OpenViBE::CString("}"));

			// m_rContext.getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Token " << it->first << " found, mgr setting = [" << value << "]\n";

			// Note that we have to accept empty strings as the user may have intended to keep the token empty. So we do not check here.
			std::stringstream ss;
			ss << value.toASCIIString();
			ss >> *(it->second);

			// m_rContext.getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Token " << it->first << " inserted as [" << *(it->second) << "]\n";
		} else {
			// token not found
		}
	}
}


