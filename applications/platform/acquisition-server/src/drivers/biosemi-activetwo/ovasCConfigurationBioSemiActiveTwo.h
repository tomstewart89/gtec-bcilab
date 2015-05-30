/*
 * ovasCConfigurationBioSemiActiveTwo.h
 *
 * Copyright (c) 2012, Mensia Technologies SA. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */

#ifdef TARGET_HAS_ThirdPartyBioSemiAPI

#ifndef __OpenViBE_AcquisitionServer_CConfigurationBioSemiActiveTwo_H__
#define __OpenViBE_AcquisitionServer_CConfigurationBioSemiActiveTwo_H__

#include "../ovasCConfigurationBuilder.h"

#include <gtk/gtk.h>

#include <vector>

namespace OpenViBEAcquisitionServer
{
	class CConfigurationBioSemiActiveTwo : public OpenViBEAcquisitionServer::CConfigurationBuilder
	{
	public:

		CConfigurationBioSemiActiveTwo(const char* sGtkBuilderFileName, OpenViBE::boolean& rUseEXChannels);

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);

	protected:
		OpenViBE::boolean& m_rUseEXChannels;
	};
};


#endif // __OpenViBE_AcquisitionServer_CConfigurationBioSemiActiveTwo_H__
#endif // TARGET_HAS_ThirdPartyBioSemiAPI
