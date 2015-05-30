#ifndef __OpenViBE_AcquisitionServer_CConfigurationTMSIRefa32B_H__
#define __OpenViBE_AcquisitionServer_CConfigurationTMSIRefa32B_H__

#include "../ovasCConfigurationBuilder.h"
#include <gtk/gtk.h>
#include <string.h>
#include <iostream>

namespace OpenViBEAcquisitionServer
{

	class CConfigurationTMSIRefa32B : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:
		CConfigurationTMSIRefa32B(const char* sGtkBuilderFileName);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);
		OpenViBE::boolean setDeviceList(const std::vector <std::string> deviceList,std::string *deviceMaster,std::vector<std::string> *deviceSlaves);
		virtual void buttonRemoveSlaveDevice(void);
		virtual void buttonAddSlaveDevice(void);

	protected:
		std::string *m_sDeviceMaster;
		std::vector <std::string> m_vDeviceList;
		std::vector <std::string> *m_vDeviceSlaves;
		std::vector <std::string> m_vDeviceSlavesTemp;
	};

	// Translates a vector of strings to a stream for storing configuration. Used strings cannot contain ';'.
	inline std::ostream& operator<< (std::ostream& out, const std::vector<std::string>& var)
	{
		std::vector<std::string>::const_iterator it = var.begin();
		for(;it!=var.end();++it) {
			out << (*it);
			out << ";";
		}

		return out;
	}

	inline std::istream& operator>> (std::istream& in, std::vector<std::string>& var)
	{
		var.clear();
		std::string token;

		while( std::getline(in, token, ';'))
		{
			var.push_back(token);
		}

		return in;
	}
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationTMSIRefa32B_H__
