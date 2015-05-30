#ifndef __OpenViBEDesigner_CLogListenerDesigner_H__
#define __OpenViBEDesigner_CLogListenerDesigner_H__

#include "ovd_base.h"

#include <vector>
#include <map>

#define OVK_ClassId_Designer_LogListener OpenViBE::CIdentifier(0x0FE155FA, 0x313C17A7)

namespace OpenViBEDesigner
{
	class CApplication;

	class CLogListenerDesigner : public OpenViBE::Kernel::ILogListener
	{
		public:

		//this object store a single log beginning with a Log_Level and ending at the next one
			class CLogObject
			{
			public:
				CLogObject(GtkTextBuffer* pBuffer)
				{
					m_pBuffer = gtk_text_buffer_new(gtk_text_buffer_get_tag_table(pBuffer));
					m_bPassedFilter = false;//by default the log does not pass the filter;
				}

				::GtkTextBuffer* getTextBuffer()
				{
					return m_pBuffer;
				}


				/*
				bool copyFromBuffer(GtkTextIter* range_begin, GtkTextIter* range_end)
				{
					GtkTextIter l_oEndLogIter;
					gtk_text_buffer_get_end_iter(m_pBuffer, &l_oEndLogIter);
					gtk_text_buffer_insert_range(m_pBuffer, &l_oEndLogIter, range_begin, range_end);
					return true;
				}//*/

				//determine if the log contains the sSearchTerm and tag the part with the sSerachTerm in gray
				const OpenViBE::boolean Filter(OpenViBE::CString sSearchTerm)
				{
					m_bPassedFilter = false;
					GtkTextIter start_find, end_find;
					gtk_text_buffer_get_start_iter(m_pBuffer, &start_find);
					gtk_text_buffer_get_end_iter(m_pBuffer, &end_find);

					//tag for highlighting the search term
					GtkTextTag* tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(m_pBuffer), "gray_bg");
					if(tag==NULL)
					{
						gtk_text_buffer_create_tag(m_pBuffer, "gray_bg", "background", "gray", NULL);
					}

					//remove previous tagging
					gtk_text_buffer_remove_tag_by_name(m_pBuffer, "gray_bg", &start_find, &end_find);

					//no term means no research so no filter we let all pass
					if(sSearchTerm==OpenViBE::CString(""))
					{
						m_bPassedFilter = true;
						return m_bPassedFilter;
					}


					GtkTextIter start_match, end_match;
					const gchar *text = sSearchTerm.toASCIIString();
					while ( gtk_text_iter_forward_search(&start_find, text, GTK_TEXT_SEARCH_TEXT_ONLY, &start_match, &end_match, NULL) )
					{
						gtk_text_buffer_apply_tag_by_name(m_pBuffer, "gray_bg", &start_match, &end_match);
						//offset to end_match
						int offset = gtk_text_iter_get_offset(&end_match);
						//begin next search at end match
						gtk_text_buffer_get_iter_at_offset(m_pBuffer, &start_find, offset);
						m_bPassedFilter = true;
					}
					return m_bPassedFilter;
				}

				public:
					::GtkTextBuffer* m_pBuffer;
					OpenViBE::boolean m_bPassedFilter;
					::GtkTreeIter m_oTreeIter;
					OpenViBE::boolean m_bIsPrinted;


			};

			CLogListenerDesigner(const OpenViBE::Kernel::IKernelContext& rKernelContext, ::GtkBuilder* pBuilderInterface);
			~CLogListenerDesigner(void);

			virtual OpenViBE::boolean isActive(OpenViBE::Kernel::ELogLevel eLogLevel);
			virtual OpenViBE::boolean activate(OpenViBE::Kernel::ELogLevel eLogLevel, OpenViBE::boolean bActive);
			virtual OpenViBE::boolean activate(OpenViBE::Kernel::ELogLevel eStartLogLevel, OpenViBE::Kernel::ELogLevel eEndLogLevel, OpenViBE::boolean bActive);
			virtual OpenViBE::boolean activate(OpenViBE::boolean bActive);

			virtual void log(const OpenViBE::time64 time64Value);

			virtual void log(const OpenViBE::uint64 ui64Value);
			virtual void log(const OpenViBE::uint32 ui32Value);
			virtual void log(const OpenViBE::uint16 ui16Value);
			virtual void log(const OpenViBE::uint8 ui8Value);

			virtual void log(const OpenViBE::int64 i64Value);
			virtual void log(const OpenViBE::int32 i32Value);
			virtual void log(const OpenViBE::int16 i16Value);
			virtual void log(const OpenViBE::int8 i8Value);

			virtual void log(const OpenViBE::float64 f64Value);
			virtual void log(const OpenViBE::float32 f32Value);

			virtual void log(const OpenViBE::boolean bValue);

			virtual void log(const OpenViBE::CIdentifier& rValue);
			virtual void log(const OpenViBE::CString& rValue);
			virtual void log(const char* pValue);

			virtual void log(const OpenViBE::Kernel::ELogLevel eLogLevel);
			virtual void log(const OpenViBE::Kernel::ELogColor eLogColor);

			void clearMessages();
			void searchMessages(OpenViBE::CString l_sSearchTerm);
			void focusMessageWindow();

			void displayLog(CLogObject* oLog);
			void appendLog(CLogObject* oLog);

			void scrollToBottom(void);

			_IsDerivedFromClass_Final_(OpenViBE::Kernel::ILogListener, OV_UndefinedIdentifier);

		protected:

			std::map<OpenViBE::Kernel::ELogLevel, OpenViBE::boolean> m_vActiveLevel;

			//logs
			std::vector<CLogObject*> m_vStoredLog;

		private:

			::GtkBuilder* m_pBuilderInterface;
			::GtkBuilder* m_pAlertBuilder;
			::GtkTextView* m_pTextView;
			//text buffer that is actually displayed
			::GtkTextBuffer* m_pBuffer;


			::GtkToggleButton* m_pToggleButtonPopup;

			::GtkToggleToolButton* m_vToggleLogButtons[OpenViBE::Kernel::LogLevel_Last];	// Array of togglebuttons, one per log level

			::GtkLabel* m_pLabelCountMessages;
			::GtkLabel* m_pLabelCountWarnings;
			::GtkLabel* m_pLabelCountErrors;
			::GtkLabel* m_pLabelDialogCountWarnings;
			::GtkLabel* m_pLabelDialogCountErrors;

			::GtkWidget* m_pImageWarnings;
			::GtkWidget* m_pImageErrors;

			::GtkWindow* m_pAlertWindow;

			::GtkExpander* m_pExpander;

			OpenViBE::boolean m_bIgnoreMessages;	// skip messages of the current log level?

			OpenViBE::uint32 m_ui32CountMessages;
			OpenViBE::uint32 m_ui32CountWarnings;
			OpenViBE::uint32 m_ui32CountErrors;

			void updateMessageCounts();
			void appendToCurrentLog(const char *textColor, const char *logMessage, bool bIsLink = false);

			OpenViBE::boolean m_bConsoleLogWithHexa;
			OpenViBE::boolean m_bConsoleLogTimeInSecond;
			OpenViBE::uint32 m_ui32ConsoleLogTimePrecision;

			CLogObject* m_pCurrentLog;
			OpenViBE::CString m_sSearchTerm;

	};
};

#endif // __OpenViBEDesigner_CLogListenerDesigner_H__
