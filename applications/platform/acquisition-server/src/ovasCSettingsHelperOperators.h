#ifndef __OpenViBE_AcquisitionServer_CSettingsHelperOperators_H__
#define __OpenViBE_AcquisitionServer_CSettingsHelperOperators_H__

#include <ovas_base.h>
#include "ovasIHeader.h"

#include <sstream>
#include <map>
#include <iostream>

/*
 * \brief Operators used to convert between typical variables (as used in properties & settings) and streams
 *
 * 
 */

namespace OpenViBEAcquisitionServer
{

	inline std::ostream& operator<< (std::ostream& out, const OpenViBE::CString& var)
	{
		out << std::string(var.toASCIIString());

		return out;
	}

	inline std::istream& operator>> (std::istream& in, OpenViBE::CString& var)
	{
		std::string tmp;

		std::getline(in, tmp);

		var.set(tmp.c_str());

		// std::cout << "Parsed [" << var.toASCIIString() << "]\n";
		return in;
	}

	// Writes fields of IHeader to a stream
	inline std::ostream& operator<< (std::ostream& out, const OpenViBEAcquisitionServer::IHeader& var)
	{
		if(var.isExperimentIdentifierSet())
		{
			out << "ExperimentID " << var.getExperimentIdentifier() << " ";
		}

		if(var.isSubjectAgeSet()) 
		{
			out << "SubjectAge " << var.getSubjectAge() << " ";
		}

		if(var.isSubjectGenderSet())
		{
			out << "SubjectGender " << var.getSubjectGender() << " ";
		}

		if(var.isSamplingFrequencySet()) 
		{
			out << "SamplingFrequency " << var.getSamplingFrequency() << " ";
		}

		if(var.isChannelCountSet()) 
		{
			out << "Channels " << var.getChannelCount() << " ";
		}

		if(var.isChannelCountSet() && var.isChannelNameSet()) 
		{
			out << "Names ";
			for(size_t i=0;i<var.getChannelCount();i++) 
			{
				out << var.getChannelName(i) << ";";
			}
			out << " ";
		}

		if(var.isChannelCountSet() && var.isChannelGainSet()) 
		{
			out << "Gains ";
			for(size_t i=0;i<var.getChannelCount();i++) 
			{
				out << var.getChannelGain(i) << " ";
			}
		} 

		return out;
	}

	// Reads fields of IHeader from a stream
	inline std::istream& operator>> (std::istream& in, OpenViBEAcquisitionServer::IHeader& var)
	{
		std::string token;

		while(std::getline(in, token, ' ')) {
			if(token=="") 
			{
				continue;
			}
	
			// std::cout << "Got token [" << token << "]\n";

			if(token=="ExperimentID")
			{
				// std::cout << "Parsing experiment id\n";
				OpenViBE::uint32 l_ui32tmp;
				in >> l_ui32tmp; var.setExperimentIdentifier(l_ui32tmp);
			} 
			else if(token=="SubjectAge")
			{
				// std::cout << "Parsing age\n";
				OpenViBE::uint32 l_ui32tmp;
				in >> l_ui32tmp; var.setSubjectAge(l_ui32tmp);
			} 
			else if(token=="SubjectGender")
			{
				// std::cout << "Parsing gender\n";
				OpenViBE::uint32 l_ui32tmp;
				in >> l_ui32tmp; var.setSubjectGender(l_ui32tmp);
			} 
			else if(token=="SamplingFrequency") 
			{
				// std::cout << "Parsing freq\n";

				OpenViBE::uint32 l_ui32tmp;
				in >> l_ui32tmp; var.setSamplingFrequency(l_ui32tmp);

				// std::cout << "  Got " << l_ui32SamplingFrequency << "\n";
			} 
			else if(token=="Channels") 
			{
				// std::cout << "Parsing chn count\n";
				OpenViBE::uint32 l_ui32tmp;
				in >> l_ui32tmp; var.setChannelCount(l_ui32tmp);

				// std::cout << "  Got " << nChannels << "\n";
			} 
			else if(token == "Names" && var.isChannelCountSet())
			{
				// std::cout << "Parsing names\n";

				std::string l_sTmp;
				for(size_t i=0;i<var.getChannelCount();i++) 
				{
					std::getline(in, l_sTmp, ';');
					// std::cout << "  Parsed " << value << "\n";
					var.setChannelName(i, l_sTmp.c_str());
				}
			} 
			else if(token == "Gains" && var.isChannelCountSet())
			{

				// std::cout << "Parsing gains\n";
				OpenViBE::float32 l_f32tmp;
				for(size_t i=0;i<var.getChannelCount();i++) 
				{
					in >> l_f32tmp; var.setChannelGain(i, l_f32tmp);
					// std::cout << "  Parsed " << tmpGain << "\n";
				}
			}
			else
			{
				// std::cout << "Unexpected token [" << token << "]\n";
			}
		}

		return in;
	}

	inline std::ostream& operator<< (std::ostream& out, const std::map<OpenViBE::uint32, OpenViBE::uint32>& var)
	{
		std::map<OpenViBE::uint32, OpenViBE::uint32>::const_iterator it = var.begin();
		for(;it!=var.end();++it) {
			out << it->first;
			out << " ";
			out << it->second;
			out << " ";
		}

		return out;
	}

	inline std::istream& operator>> (std::istream& in, std::map<OpenViBE::uint32, OpenViBE::uint32>& var)
	{
		var.clear();
		OpenViBE::uint32 key;
		OpenViBE::uint32 value;
		while( in >> key ) {
			in >> value;
			var[key] = value; 
		}

		return in;
	}

};


#endif

