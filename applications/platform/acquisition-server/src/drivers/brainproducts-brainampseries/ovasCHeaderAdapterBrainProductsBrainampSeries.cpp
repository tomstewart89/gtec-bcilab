/*
 * Brain Products Brainamp Series driver for OpenViBE
 * Copyright (C) 2010 INRIA - Author : Yann Renard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "ovasCHeaderAdapterBrainProductsBrainampSeries.h"

#if defined TARGET_OS_Windows

using namespace OpenViBE;
using namespace OpenViBEAcquisitionServer;

CHeaderAdapterBrainProductsBrainampSeries::CHeaderAdapterBrainProductsBrainampSeries(
	IHeader& rAdaptedHeader,
	EParameter* peChannelSelected)

	:CHeaderAdapter(rAdaptedHeader)
	,m_peChannelSelected(peChannelSelected)
{
}

boolean CHeaderAdapterBrainProductsBrainampSeries::setChannelCount(const uint32 ui32ChannelCount)
{
	return false;
}

boolean CHeaderAdapterBrainProductsBrainampSeries::setChannelName(const uint32 ui32ChannelIndex, const char* sChannelName)
{
	return false;
}

uint32 CHeaderAdapterBrainProductsBrainampSeries::getChannelCount(void) const
{
	uint32 i, j=0;
	for(i=0; i<m_rAdaptedHeader.getChannelCount(); i++)
	{
		if(m_peChannelSelected[i]==Channel_Selected)
		{
			j++;
		}
	}
	return j;
}

const char* CHeaderAdapterBrainProductsBrainampSeries::getChannelName(const uint32 ui32ChannelIndex) const
{
	uint32 i, j=0;
	for(i=0; i<m_rAdaptedHeader.getChannelCount(); i++)
	{
		if(m_peChannelSelected[i]==Channel_Selected)
		{
			if(j==ui32ChannelIndex)
			{
				return m_rAdaptedHeader.getChannelName(i);
			}
			j++;
		}
	}
	return "";
}

#endif // TARGET_OS_Windows
