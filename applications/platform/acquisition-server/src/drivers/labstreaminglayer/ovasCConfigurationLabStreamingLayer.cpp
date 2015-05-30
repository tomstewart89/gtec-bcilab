
#if defined(TARGET_HAS_ThirdPartyLSL)

#include "ovasCConfigurationLabStreamingLayer.h"

#include <lsl_cpp.h>
#include "ovasIHeader.h"

#include <iostream>
#include <sstream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace std;


CConfigurationLabStreamingLayer::CConfigurationLabStreamingLayer(IDriverContext& rDriverContext, const char* sGtkBuilderFileName, 
	IHeader& rHeader,
	CString& rSignalStream,
	CString& rSignalStreamID,
	CString& rMarkerStream,
	CString& rMarkerStreamID
	)
	:CConfigurationBuilder(sGtkBuilderFileName)
	,m_rDriverContext(rDriverContext) 
	,m_rHeader(rHeader)
	,m_rSignalStream(rSignalStream)
	,m_rSignalStreamID(rSignalStreamID)
	,m_rMarkerStream(rMarkerStream)
	,m_rMarkerStreamID(rMarkerStreamID)
{
}

boolean CConfigurationLabStreamingLayer::preConfigure(void)
{
	if(! CConfigurationBuilder::preConfigure())
	{
		return false;
	}

	::GtkComboBox* l_pComboBox=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_signal_stream"));
	if(!l_pComboBox)
	{
		return false;
	}

	::GtkComboBox* l_pMarkerComboBox=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_marker_stream"));
	if(!l_pMarkerComboBox)
	{
		return false;
	}

	m_vSignalIndex.clear();
	m_vMarkerIndex.clear();

	// Allow operation without a marker stream
	::gtk_combo_box_append_text(l_pMarkerComboBox, "None");
	::gtk_combo_box_set_active(l_pMarkerComboBox, 0);
	m_vMarkerIndex.push_back(-1);

	m_vStreams = lsl::resolve_streams(1.0);

	// See if any of the streams can be interpreted as signal or marker
	uint32 l_ui32nStreams = 0;
	uint32 l_ui32nMarkerStreams = 0;
	boolean l_bExactSignalMatch = false; 	// If we can match both name and ID, stop at that. Otherwise we auto-accept the 'name only' match.
	boolean l_bExactMarkerMatch = false;

	m_rDriverContext.getLogManager() << LogLevel_Trace << "Discovered " << static_cast<uint64>(m_vStreams.size()) << " streams in total\n";

	for(uint32 i=0; i<m_vStreams.size(); i++)
	{
		if(m_vStreams[i].channel_format() == lsl::cf_float32)
		{	
			std::stringstream ss; ss << m_vStreams[i].name().c_str() << " / " << m_vStreams[i].source_id().c_str() ;

			m_rDriverContext.getLogManager() << LogLevel_Trace << i << ". Discovered signal stream " << m_vStreams[i].name().c_str() << ", id " 
				<< ss.str().c_str() << "\n";

			::gtk_combo_box_append_text(l_pComboBox, ss.str().c_str());
			m_vSignalIndex.push_back(i);
			if((m_rSignalStream==CString(m_vStreams[i].name().c_str()) && !l_bExactSignalMatch)
				|| !l_ui32nStreams)
			{
				if(m_rSignalStreamID==CString(m_vStreams[i].source_id().c_str()))
				{
					l_bExactSignalMatch = true;
				}
				::gtk_combo_box_set_active(l_pComboBox,l_ui32nStreams);
			}		
			l_ui32nStreams++;
		}
		else if(m_vStreams[i].channel_format() == lsl::cf_int32)
		{
			std::stringstream ss; ss << m_vStreams[i].name().c_str() << " / " << m_vStreams[i].source_id().c_str() ;
			
			m_rDriverContext.getLogManager() << LogLevel_Trace << i << ". Discovered marker stream " << m_vStreams[i].name().c_str() << ", id " 
				<< ss.str().c_str() << "\n";

			::gtk_combo_box_append_text(l_pMarkerComboBox, ss.str().c_str());
			m_vMarkerIndex.push_back(i);
			if((m_rMarkerStream==CString(m_vStreams[i].name().c_str()) && !l_bExactMarkerMatch) 
				|| !l_ui32nMarkerStreams)
			{
				if(m_rMarkerStreamID==CString(m_vStreams[i].source_id().c_str()))
				{
					// If we can match both name and ID, stop at that. Otherwise we accept the 'name only' match.
					l_bExactMarkerMatch = true;
				}
				::gtk_combo_box_set_active(l_pMarkerComboBox,l_ui32nMarkerStreams+1);
			}
			l_ui32nMarkerStreams++;
		} 
		else 
		{
			// Only float32 and int32 are currently supported for signals and markers respectively
			m_rDriverContext.getLogManager() << LogLevel_Trace << i << ". Discovered stream with channel format " << m_vStreams[i].channel_format() << " of stream [" << m_vStreams[i].name().c_str() << "] which is not supported, skipped.\n";

			continue;
		}
	}

	return true;
}

boolean CConfigurationLabStreamingLayer::postConfigure(void)
{
	if(m_bApplyConfiguration)
	{
		// Retrieve signal stream info
		::GtkComboBox* l_pComboBox=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_signal_stream"));
		if(!l_pComboBox)
		{	
			m_bApplyConfiguration = false;
			CConfigurationBuilder::postConfigure(); // close window etc
			return false;
		}

		if(m_vSignalIndex.size()==0) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "LSL: Cannot proceed without a signal stream.\n";
			m_bApplyConfiguration = false;
			CConfigurationBuilder::postConfigure(); // close window etc
			return false;
		}

		const int32 l_i32SignalIndex = m_vSignalIndex[gtk_combo_box_get_active(l_pComboBox)];

		m_rSignalStream = m_vStreams[l_i32SignalIndex].name().c_str();
		m_rSignalStreamID = m_vStreams[l_i32SignalIndex].source_id().c_str();

		::GtkComboBox* l_pMarkerComboBox=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_marker_stream"));
		if(!l_pMarkerComboBox)
		{
			m_bApplyConfiguration = false;
			CConfigurationBuilder::postConfigure(); // close window etc
			return false;
		}
		
		// Retrieve marker stream info
		const int32 l_i32MarkerIndex = m_vMarkerIndex[gtk_combo_box_get_active(l_pMarkerComboBox)];
		if(l_i32MarkerIndex>=0) 
		{
			m_rMarkerStream = m_vStreams[l_i32MarkerIndex].name().c_str();
			m_rMarkerStreamID = m_vStreams[l_i32MarkerIndex].source_id().c_str();
		} 
		else
		{
			m_rMarkerStream = "None";
			m_rMarkerStreamID = "";
		}

		m_rDriverContext.getLogManager() << LogLevel_Trace << "Binding to [" << m_rSignalStream << ", id " << m_rSignalStreamID << "] and ["
			<< m_rMarkerStream << ", id " << m_rMarkerStreamID << "]\n";

	}

	if(! CConfigurationBuilder::postConfigure()) // normal header is filled (Subject ID, Age, Gender, channels, sampling frequency), ressources are realesed
	{
		return false;
	}

	return true;
}

#endif
