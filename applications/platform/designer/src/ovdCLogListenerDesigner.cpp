#include "ovdCLogListenerDesigner.h"

#include <iostream>
#include <sstream>

#include <openvibe/ovITimeArithmetics.h>

#define OVD_GUI_File OpenViBE::Directories::getDataDir() + "/applications/designer/interface.ui"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEDesigner;
using namespace std;

namespace OpenViBEDesigner {
	static const char* g_sWatercourse = "c_watercourse";
	static const char* g_sBluechill   = "c_blueChill";
};

namespace
{
	void close_messages_alert_window_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_widget_hide(GTK_WIDGET(pUserData));
	}

	void focus_message_window_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CLogListenerDesigner*>(pUserData)->focusMessageWindow();
	}

	void expander_expand_cb(::GtkExpander* pExpander, GParamSpec *param_spec, gpointer pUserData)
	{
		if(gtk_expander_get_expanded(pExpander))
		{
			static_cast<CLogListenerDesigner*>(pUserData)->scrollToBottom();
		}
	}

}

CLogListenerDesigner::CLogListenerDesigner(const IKernelContext& rKernelContext, ::GtkBuilder* pBuilderInterface)
	:m_pBuilderInterface( pBuilderInterface )
	,m_pAlertWindow( NULL)
	,m_bIgnoreMessages( false )
	,m_ui32CountMessages( 0 )
	,m_ui32CountWarnings( 0 )
	,m_ui32CountErrors( 0 )
	,m_pCurrentLog( NULL )
{
	m_pTextView = GTK_TEXT_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-textview_messages"));
	m_pAlertWindow = GTK_WINDOW(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_popup"));

	m_pToggleButtonPopup = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_alert_on_error"));

	m_pLabelCountMessages = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_message_label"));
	m_pLabelCountWarnings = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_warning_label"));
	m_pLabelCountErrors = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_error_label"));
	m_pLabelDialogCountWarnings = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_popup-warning_count"));
	m_pLabelDialogCountErrors = GTK_LABEL(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_popup-error_count"));

	m_pImageWarnings = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_warning_image"));
	m_pImageErrors = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_count_error_image"));

	m_vToggleLogButtons[LogLevel_Debug]            = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_debug"));
	m_vToggleLogButtons[LogLevel_Benchmark]        = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_bench"));
	m_vToggleLogButtons[LogLevel_Trace]            = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_trace"));
	m_vToggleLogButtons[LogLevel_Info]             = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_info"));
	m_vToggleLogButtons[LogLevel_Warning         ] = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_warning"));
	m_vToggleLogButtons[LogLevel_ImportantWarning] = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_impwarning"));
	m_vToggleLogButtons[LogLevel_Error]            = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_error"));
	m_vToggleLogButtons[LogLevel_Fatal]            = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_fatal"));

	m_pExpander = GTK_EXPANDER(gtk_builder_get_object(m_pBuilderInterface, "openvibe-expander_messages"));
	g_signal_connect(G_OBJECT(m_pExpander), "notify::expanded", G_CALLBACK(::expander_expand_cb), this);

	// set the popup-on-error checkbox according to the configuration token
	gtk_toggle_button_set_active(m_pToggleButtonPopup, (OpenViBE::boolean)(rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_PopUpOnError}")));

	g_signal_connect(G_OBJECT(m_pAlertWindow), "delete_event", G_CALLBACK(::gtk_widget_hide), NULL);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_popup-button_view")), "clicked", G_CALLBACK(::focus_message_window_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "dialog_error_popup-button_ok")), "clicked", G_CALLBACK(::close_messages_alert_window_cb), m_pAlertWindow);

	m_pBuffer = gtk_text_view_get_buffer( m_pTextView );
	gtk_text_buffer_set_text(m_pBuffer, "", -1);
	m_sSearchTerm = CString("");

	gtk_text_buffer_create_tag(m_pBuffer, "f_mono", "family", "monospace", NULL);
	gtk_text_buffer_create_tag(m_pBuffer, "w_bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(m_pBuffer, "c_blue", "foreground", "#0000FF", NULL); // debug
	gtk_text_buffer_create_tag(m_pBuffer, "c_magenta", "foreground", "#FF00FF", NULL); // benchmark
	gtk_text_buffer_create_tag(m_pBuffer, "c_darkOrange", "foreground", "#FF9000", NULL); // important warning
	gtk_text_buffer_create_tag(m_pBuffer, "c_red", "foreground", "#FF0000", NULL); // error, fatal
	gtk_text_buffer_create_tag(m_pBuffer, g_sWatercourse, "foreground", "#008238", NULL); // trace
	gtk_text_buffer_create_tag(m_pBuffer, "c_aqua", "foreground", "#00FFFF", NULL); // number
	gtk_text_buffer_create_tag(m_pBuffer, "c_darkViolet", "foreground", "#6900D7", NULL); // warning
	gtk_text_buffer_create_tag(m_pBuffer, g_sBluechill, "foreground", "#3d889b", NULL); // information
	gtk_text_buffer_create_tag(m_pBuffer, "link", "underline", PANGO_UNDERLINE_SINGLE, NULL); // link for CIdentifier

	m_bConsoleLogWithHexa = rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ConsoleLogWithHexa}",false);
	m_bConsoleLogTimeInSecond = rKernelContext.getConfigurationManager().expandAsBoolean("${Kernel_ConsoleLogTimeInSecond}",false);
	m_ui32ConsoleLogTimePrecision = (uint32) rKernelContext.getConfigurationManager().expandAsUInteger("${Designer_ConsoleLogTimePrecision}",3);
}

CLogListenerDesigner::~CLogListenerDesigner(void) {
	clearMessages();
}

boolean CLogListenerDesigner::isActive(ELogLevel eLogLevel)
{
	map<ELogLevel, boolean>::iterator itLogLevel=m_vActiveLevel.find(eLogLevel);
	if(itLogLevel==m_vActiveLevel.end())
	{
		return true;
	}
	return itLogLevel->second;
}

boolean CLogListenerDesigner::activate(ELogLevel eLogLevel, boolean bActive)
{
	m_vActiveLevel[eLogLevel]=bActive;
	return true;
}

boolean CLogListenerDesigner::activate(ELogLevel eStartLogLevel, ELogLevel eEndLogLevel, boolean bActive)
{
	for(int i=eStartLogLevel; i<=eEndLogLevel; i++)
	{
		m_vActiveLevel[ELogLevel(i)]=bActive;
	}
	return true;
}

boolean CLogListenerDesigner::activate(boolean bActive)
{
	return activate(LogLevel_First, LogLevel_Last, bActive);
}

void CLogListenerDesigner::log(const time64 time64Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	if(m_bConsoleLogTimeInSecond)
	{
		float64 l_f64Time=ITimeArithmetics::timeToSeconds(time64Value.m_ui64TimeValue);
		std::stringstream ss;
		ss.precision(m_ui32ConsoleLogTimePrecision);
		ss.setf(std::ios::fixed,std::ios::floatfield);
		ss << l_f64Time;
		ss << " sec";
		if(m_bConsoleLogWithHexa)
		{
			ss << " (0x" << hex << time64Value.m_ui64TimeValue << ")";
		}

		l_sText << ss.str().c_str();
	}
	else
	{
		l_sText << dec << time64Value.m_ui64TimeValue;
		if(m_bConsoleLogWithHexa)
		{
			l_sText << " (0x" << hex << time64Value.m_ui64TimeValue << ")";
		}
	}
	
	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());
}

void CLogListenerDesigner::log(const uint64 ui64Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui64Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << ui64Value << ")";
	}

	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());
}

void CLogListenerDesigner::log(const uint32 ui32Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui32Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << ui32Value << ")";
	}
	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());

}

void CLogListenerDesigner::log(const uint16 ui16Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui16Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << ui16Value << ")";
	}
	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());

}

void CLogListenerDesigner::log(const uint8 ui8Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << ui8Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << ui8Value << ")";
	}
	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());
}

void CLogListenerDesigner::log(const int64 i64Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i64Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << i64Value << ")";
	}
	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());

}

void CLogListenerDesigner::log(const int32 i32Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i32Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << i32Value << ")";
	}
	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());
}

void CLogListenerDesigner::log(const int16 i16Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i16Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << i16Value << ")";
	}
	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());
}

void CLogListenerDesigner::log(const int8 i8Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << dec << i8Value;
	if(m_bConsoleLogWithHexa)
	{
		l_sText << " (0x" << hex << i8Value << ")";
	}
	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());
}

void CLogListenerDesigner::log(const float32 f32Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << f32Value;

	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());
}

void CLogListenerDesigner::log(const float64 f64Value)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << f64Value;

	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());
}

void CLogListenerDesigner::log(const boolean bValue)
{
	if(m_bIgnoreMessages) return;

	stringstream l_sText;
	l_sText << (bValue ? "true" : "false");

	appendToCurrentLog(g_sWatercourse, l_sText.str().c_str());
}

void CLogListenerDesigner::log(const CIdentifier& rValue)
{
	if(m_bIgnoreMessages) return;

	appendToCurrentLog(g_sBluechill, rValue.toString(), true);
}

void CLogListenerDesigner::log(const CString& rValue)
{
	if(m_bIgnoreMessages) return;

	appendToCurrentLog(g_sBluechill,  rValue);
}

void CLogListenerDesigner::log(const char* pValue)
{
	if(m_bIgnoreMessages) return;

	appendToCurrentLog(NULL, pValue);
}

void CLogListenerDesigner::log(const ELogLevel eLogLevel)
{
	m_bIgnoreMessages = !gtk_toggle_tool_button_get_active(m_vToggleLogButtons[eLogLevel]);	
	if(m_bIgnoreMessages) 
	{
		return;
	}

	m_ui32CountMessages++;

	//new log, will be deleted when m_vStoredLog is cleared
	m_pCurrentLog = new CLogObject(m_pBuffer);//m_pNonFilteredBuffer);

	//copy this newly added content in the current log
	GtkTextIter l_oEndLogIter;
	gtk_text_buffer_get_end_iter(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter);

	switch(eLogLevel)
	{
		case LogLevel_Debug:
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "[ ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "DEBUG", -1, "w_bold", "f_mono", "c_blue", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, " ] ", -1, "w_bold", "f_mono", NULL);
			break;
		case LogLevel_Benchmark:
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "[ ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "BENCH", -1, "w_bold", "f_mono", "c_magenta", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, " ] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Trace:
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "[ ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "TRACE", -1, "w_bold", "f_mono", g_sWatercourse, NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, " ] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Info:
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "[  ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "INF", -1, "w_bold", "f_mono", g_sBluechill, NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "  ] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Warning:
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "[", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "WARNING", -1, "w_bold", "f_mono", "c_darkViolet", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_ImportantWarning:
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "[", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "WARNING", -1, "w_bold", "f_mono", "c_darkOrange", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Error:
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "[ ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "ERROR", -1, "w_bold", "f_mono", "c_red", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, " ] ", -1, "w_bold", "f_mono", NULL);
			break;

		case LogLevel_Fatal:
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "[ ", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "FATAL", -1, "w_bold", "f_mono", "c_red", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, " ] ", -1, "w_bold", "f_mono", NULL);
			break;

		default:
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "[", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "UNKNOWN", -1, "w_bold", "f_mono", NULL);
			gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, "] ", -1, "w_bold", "f_mono", NULL);
			break;
	}

	m_vStoredLog.push_back(m_pCurrentLog);

	//see if the log passes the filter
	boolean l_bPassFilter = m_pCurrentLog->Filter(m_sSearchTerm);
	//if it does mark this position and insert the log in the text buffer displayed
	GtkTextIter l_oEndDisplayedTextIter;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oEndDisplayedTextIter);
	gtk_text_buffer_create_mark(m_pBuffer, "current_log", &l_oEndDisplayedTextIter, true );//creating a mark will erase the previous one with the same name so no worry here
	if(l_bPassFilter)
	{
		displayLog(m_pCurrentLog);
	}

	if(gtk_toggle_button_get_active(m_pToggleButtonPopup) && (eLogLevel == LogLevel_Warning || eLogLevel == LogLevel_ImportantWarning || eLogLevel == LogLevel_Error || eLogLevel == LogLevel_Fatal))
	{
		if(!gtk_widget_get_visible(GTK_WIDGET(m_pAlertWindow)))
		{
			gtk_window_set_position(GTK_WINDOW(m_pAlertWindow), GTK_WIN_POS_CENTER);
			gtk_window_present(GTK_WINDOW(m_pAlertWindow));
			gtk_window_set_keep_above(GTK_WINDOW(m_pAlertWindow), true);
		}
	}

	this->updateMessageCounts();

	if(gtk_expander_get_expanded(m_pExpander))
	{
		scrollToBottom();
	}
}

void CLogListenerDesigner::log(const ELogColor eLogColor)
{
	// Manual color change not supported

	if(m_bIgnoreMessages) return;

	(void)eLogColor; // suppress possible unused warning
}

void CLogListenerDesigner::updateMessageCounts()
{
	stringstream l_sCountMessages;
	l_sCountMessages << "<b>" << m_ui32CountMessages << "</b> Message";

	if(m_ui32CountMessages > 1)
	{
		l_sCountMessages << "s";
	}

	gtk_label_set_markup(m_pLabelCountMessages, l_sCountMessages.str().c_str());

	if(m_ui32CountWarnings > 0)
	{
		stringstream l_sCountWarnings;
		l_sCountWarnings << "<b>" << m_ui32CountWarnings << "</b> Warning";

		if(m_ui32CountWarnings > 1)
		{
			l_sCountWarnings << "s";
		}

		gtk_label_set_markup(m_pLabelCountWarnings, l_sCountWarnings.str().c_str());
		gtk_label_set_markup(m_pLabelDialogCountWarnings, l_sCountWarnings.str().c_str());
		gtk_widget_set_visible(GTK_WIDGET(m_pLabelCountWarnings), true);
		gtk_widget_set_visible(GTK_WIDGET(m_pImageWarnings), true);
	}

	if(m_ui32CountErrors > 0)
	{
		stringstream l_sCountErrors;
		l_sCountErrors << "<b>" << m_ui32CountErrors << "</b> Error";

		if(m_ui32CountErrors > 1)
		{
			l_sCountErrors << "s";
		}

		gtk_label_set_markup(m_pLabelCountErrors, l_sCountErrors.str().c_str());
		gtk_label_set_markup(m_pLabelDialogCountErrors, l_sCountErrors.str().c_str());

		gtk_widget_set_visible(GTK_WIDGET(m_pLabelCountErrors), true);
		gtk_widget_set_visible(GTK_WIDGET(m_pImageErrors), true);
	}
}

void CLogListenerDesigner::appendToCurrentLog(const char *textColor, const char *logMessage, bool bIsLink /* = false */) 
{
	if(!m_pCurrentLog) {
		std::cout << "Ouch, current log had been deleted before creating new, this shouldn't happen...\n";
		return;
	}

	GtkTextIter l_oEndLogIter;
	gtk_text_buffer_get_end_iter(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter);

	if(bIsLink)
	{
		gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, logMessage, -1, "f_mono", textColor, "link", NULL);
	} 
	else
	{
		gtk_text_buffer_insert_with_tags_by_name(m_pCurrentLog->getTextBuffer(), &l_oEndLogIter, logMessage, -1, "f_mono", textColor, NULL);
	}

	if(m_pCurrentLog->Filter(m_sSearchTerm))
	{
		displayLog(m_pCurrentLog);
	}
}

void CLogListenerDesigner::clearMessages()
{
	m_ui32CountMessages = 0;
	m_ui32CountWarnings = 0;
	m_ui32CountErrors = 0;

	gtk_label_set_markup(m_pLabelCountMessages, "<b>0</b> Message");
	gtk_label_set_markup(m_pLabelCountWarnings, "<b>0</b> Warning");
	gtk_label_set_markup(m_pLabelCountErrors, "<b>0</b> Error");
	gtk_label_set_markup(m_pLabelDialogCountWarnings, "<b>0</b> Warning");
	gtk_label_set_markup(m_pLabelDialogCountErrors, "<b>0</b> Error");

	gtk_widget_set_visible(m_pImageWarnings, false);
	gtk_widget_set_visible(GTK_WIDGET(m_pLabelCountWarnings), false);
	gtk_widget_set_visible(m_pImageErrors, false);
	gtk_widget_set_visible(GTK_WIDGET(m_pLabelCountErrors), false);

	gtk_text_buffer_set_text(m_pBuffer, "", -1);

	for(size_t i=0;i<m_vStoredLog.size();i++) {
		delete m_vStoredLog[i];
	}
	m_vStoredLog.clear();

	m_pCurrentLog = NULL;
}

void CLogListenerDesigner::displayLog(CLogObject *oLog)
{
	GtkTextMark* l_oMark = gtk_text_buffer_get_mark(m_pBuffer, "current_log");
	GtkTextIter l_oIter, l_oEndter, l_oLogBegin, l_oLogEnd;
	gtk_text_buffer_get_iter_at_mark(m_pBuffer, &l_oIter, l_oMark);
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oEndter);

	//delete what after the mark
	gtk_text_buffer_delete(m_pBuffer, &l_oIter, &l_oEndter);
	//get iter
	gtk_text_buffer_get_iter_at_mark(m_pBuffer, &l_oIter, l_oMark);
	//rewrite the log
	gtk_text_buffer_get_start_iter(oLog->getTextBuffer(), &l_oLogBegin);
	gtk_text_buffer_get_end_iter(oLog->getTextBuffer(), &l_oLogEnd);
	gtk_text_buffer_insert_range(m_pBuffer, &l_oIter, &l_oLogBegin, &l_oLogEnd );
}

void CLogListenerDesigner::appendLog(CLogObject *oLog)
{
	GtkTextIter l_oEndter, l_oLogBegin, l_oLogEnd;
	gtk_text_buffer_get_end_iter(m_pBuffer, &l_oEndter);
	//get log buffer bounds
	gtk_text_buffer_get_start_iter(oLog->getTextBuffer(), &l_oLogBegin);
	gtk_text_buffer_get_end_iter(oLog->getTextBuffer(), &l_oLogEnd);
	//copy at the end of the displayed buffer
	gtk_text_buffer_insert_range(m_pBuffer, &l_oEndter, &l_oLogBegin, &l_oLogEnd );
}

void CLogListenerDesigner::searchMessages(CString l_sSearchTerm)
{
	//clear displayed buffer
	gtk_text_buffer_set_text(m_pBuffer, "", -1);
	m_sSearchTerm = l_sSearchTerm;
	for(uint32 log=0; log<m_vStoredLog.size(); log++)
	{
		if(m_vStoredLog[log]->Filter(l_sSearchTerm))
		{
			//display the log
			appendLog(m_vStoredLog[log]);
		}
	}
}

void CLogListenerDesigner::focusMessageWindow()
{
	// cout << "focus in message window " << endl;
	gtk_widget_hide(GTK_WIDGET(m_pAlertWindow));
	gtk_expander_set_expanded(GTK_EXPANDER(gtk_builder_get_object(m_pBuilderInterface, "openvibe-expander_messages")), true);
}

void CLogListenerDesigner::scrollToBottom(void){
	::GtkTextMark l_oMark;
	l_oMark = *(gtk_text_buffer_get_mark (gtk_text_view_get_buffer( m_pTextView ), "insert"));
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (m_pTextView), &l_oMark, 0.0, FALSE, 0.0, 0.0);
}
