#include "ovd_base.h"

#include <system/ovCTime.h>

#include <stack>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <cstring>
#include <cstdlib>
#include <algorithm>

#include <gdk/gdkkeysyms.h>



// round is defined in <cmath> on c++11
inline int ov_round(double dbl)
{ return dbl >= 0.0 ? (int)(dbl + 0.5) : ((dbl - (double)(int)dbl) <= -0.5 ? (int)dbl : (int)(dbl - 0.5));
}

#include <openvibe/ovITimeArithmetics.h>

#if defined TARGET_OS_Linux
 #define _strcmpi strcasecmp
#endif




#define OVD_GUI_File          OpenViBE::Directories::getDataDir() + "/applications/designer/interface.ui"
#define OVD_GUI_Settings_File OpenViBE::Directories::getDataDir() + "/applications/designer/interface-settings.ui"

#include "ovdCDesignerVisualisation.h"
#include "ovdCPlayerVisualisation.h"
#include "ovdCInterfacedObject.h"
#include "ovdCInterfacedScenario.h"
#include "ovdCApplication.h"
#include "ovdCLogListenerDesigner.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEDesigner;
using namespace std;

namespace
{
	::guint __g_idle_add__(::GSourceFunc fpCallback, ::gpointer pUserData, ::gint iPriority=G_PRIORITY_DEFAULT_IDLE)
	{
		::GSource* l_pSource=g_idle_source_new();
		g_source_set_priority(l_pSource, G_PRIORITY_LOW);
		g_source_set_callback(l_pSource, fpCallback, pUserData, NULL);
		return g_source_attach(l_pSource, NULL);
	}

	::guint __g_timeout_add__(::guint uiInterval, ::GSourceFunc fpCallback, ::gpointer pUserData, ::gint iPriority=G_PRIORITY_DEFAULT)
	{
		::GSource* l_pSource=g_timeout_source_new(uiInterval);
		g_source_set_priority(l_pSource, G_PRIORITY_LOW);
		g_source_set_callback(l_pSource, fpCallback, pUserData, NULL);
		return g_source_attach(l_pSource, NULL);
	}

	void drag_data_get_cb(::GtkWidget* pWidget, ::GdkDragContext* pDragContex, ::GtkSelectionData* pSelectionData, guint uiInfo, guint uiT, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->dragDataGetCB(pWidget, pDragContex, pSelectionData, uiInfo, uiT);
	}
	void menu_undo_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->undoCB();
	}
	void menu_redo_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->redoCB();
	}
	void menu_focus_search_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		CApplication* l_pApplication = static_cast<CApplication*>(pUserData);
		//if we want the log area GtkEntry to be able to grab the focus, this one must not grab it
		if(!(l_pApplication->isLogAreaClicked()))
		{
			gtk_widget_grab_focus(GTK_WIDGET(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-box_algorithm_searchbox")));
		}
		else
		{
			gtk_widget_grab_focus(GTK_WIDGET(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "searchEntry")));
		}
	}
	void menu_copy_selection_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->copySelectionCB();
	}
	void menu_cut_selection_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->cutSelectionCB();
	}
	void menu_paste_selection_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->pasteSelectionCB();
	}
	void menu_delete_selection_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->deleteSelectionCB();
	}
	void menu_preferences_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->preferencesCB();
	}

	/*
	void menu_test_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->testCB();
	}
	*/
	void menu_new_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->newScenarioCB();
	}
	void menu_open_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->openScenarioCB();
	}
	void menu_save_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->saveScenarioCB();
	}
	void menu_save_scenario_as_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->saveScenarioAsCB();
	}
	void menu_close_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->closeScenarioCB(static_cast<CApplication*>(pUserData)->getCurrentInterfacedScenario());
	}
	void menu_quit_application_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		if(static_cast<CApplication*>(pUserData)->quitApplicationCB())
		{
			gtk_main_quit();
		}
	}

	void menu_about_scenario_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->aboutScenarioCB(static_cast<CApplication*>(pUserData)->getCurrentInterfacedScenario());
	}
	void menu_about_openvibe_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->aboutOpenViBECB();
	}
#if defined(TARGET_OS_Windows)
	void menu_about_link_clicked_cb(::GtkAboutDialog* pAboutDialog, const gchar *linkPtr, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->aboutLinkClickedCB(linkPtr);
	}
#endif
	void menu_browse_documentation_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->browseDocumentationCB();
	}
	void button_new_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->newScenarioCB();
	}
	void button_open_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->openScenarioCB();
	}
	void button_save_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->saveScenarioCB();
	}
	void button_save_scenario_as_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->saveScenarioAsCB();
	}
	void button_close_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->closeScenarioCB(static_cast<CApplication*>(pUserData)->getCurrentInterfacedScenario());
	}

	void delete_designer_visualisation_cb(gpointer user_data)
	{
		static_cast<CApplication*>(user_data)->deleteDesignerVisualisationCB();
	}
	void button_toggle_window_manager_cb(::GtkToggleToolButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->toggleDesignerVisualisationCB();
	}

	void button_comment_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->addCommentCB(static_cast<CApplication*>(pUserData)->getCurrentInterfacedScenario());
	}
	void button_about_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->aboutScenarioCB(static_cast<CApplication*>(pUserData)->getCurrentInterfacedScenario());
	}

	void stop_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->stopScenarioCB();
	}
	void play_pause_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		if(std::string(gtk_tool_button_get_stock_id(GTK_TOOL_BUTTON(pButton)))==GTK_STOCK_MEDIA_PLAY)
		{
			static_cast<CApplication*>(pUserData)->playScenarioCB();
		}
		else
		{
			static_cast<CApplication*>(pUserData)->pauseScenarioCB();
		}
	}
	void next_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->nextScenarioCB();
	}
	void forward_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->forwardScenarioCB();
	}

	void zoom_in_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->zoomInCB();
	}

	void zoom_out_scenario_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->zoomOutCB();
	}


	void spinner_zoom_changed_cb(::GtkSpinButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->spinnerZoomChangedCB((uint32)gtk_spin_button_get_value(pButton));
	}

static	void window_menu_check_item_toggled_cb(GtkCheckMenuItem* pCheckMenuItem, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->windowItemToggledCB(pCheckMenuItem);
	}

	gboolean button_quit_application_cb(::GtkWidget* pWidget, ::GdkEvent* pEvent, gpointer pUserData)
	{
		if(static_cast<CApplication*>(pUserData)->quitApplicationCB())
		{
			gtk_main_quit();
			return FALSE;
		}
		return TRUE;
	}

	void log_level_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->logLevelCB();
	}

	void cpu_usage_cb(::GtkToggleButton* pButton, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->CPUUsageCB();
	}

	gboolean change_current_scenario_cb(::GtkNotebook* pNotebook, ::GtkNotebookPage* pNotebookPage, guint uiPageNumber, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->changeCurrentScenario(static_cast<int32>(uiPageNumber));
		return TRUE;
	}

	gboolean reorder_scenario_cb(::GtkNotebook* pNotebook, ::GtkNotebookPage* pNotebookPage, guint uiPageNumber, gpointer pUserData)
	{
		static_cast<CApplication*>(pUserData)->reorderCurrentScenario(static_cast<int32>(uiPageNumber));
		return TRUE;
	}

	void box_algorithm_title_button_expand_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_tree_view_expand_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-box_algorithm_tree")));
		gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-resource_notebook")), 0);
	}
	void box_algorithm_title_button_collapse_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_tree_view_collapse_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-box_algorithm_tree")));
		gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-resource_notebook")), 0);
	}

	void algorithm_title_button_expand_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_tree_view_expand_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-algorithm_tree")));
		gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-resource_notebook")), 1);
	}
	void algorithm_title_button_collapse_cb(::GtkButton* pButton, gpointer pUserData)
	{
		gtk_tree_view_collapse_all(GTK_TREE_VIEW(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-algorithm_tree")));
		gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-resource_notebook")), 1);
	}

	void clear_messages_cb(::GtkButton* pButton, gpointer pUserData)
	{
		static_cast<CLogListenerDesigner*>(pUserData)->clearMessages();
	}

	void search_messages_cb(::GtkButton* pButton, gpointer pUserData)
	{
		CApplication* l_pApplication=static_cast<CApplication*>(pUserData);
		CString l_sSearchTerm(static_cast<const char*>(l_pApplication->m_sLogSearchTerm));
		l_pApplication->m_pLogListenerDesigner->searchMessages(l_sSearchTerm);
	}

	void refresh_search_log_entry(::GtkEntry* pTextfield, CApplication* pApplication)
	{
		pApplication->m_sLogSearchTerm = gtk_entry_get_text(pTextfield);
		//immediately redo the search to refresh log field
		pApplication->m_pLogListenerDesigner->searchMessages(pApplication->m_sLogSearchTerm);
	}

	string strtoupper(string str)
	{
		int leng=str.length();
		for(int i=0; i<leng; i++)
			if (97<=str[i]&&str[i]<=122)//a-z
				str[i]-=32;
		return str;
	}

	static gboolean box_algorithm_search_func(GtkTreeModel *model, GtkTreeIter *iter, gpointer pUserData)
	{
		CApplication* l_pApplication=static_cast<CApplication*>(pUserData);
		/* Visible if row is non-empty and first column is "HI" */


		gboolean l_bVisible = false;
		gboolean l_bShowUnstable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(static_cast<CApplication*>(pUserData)->m_pBuilderInterface, "openvibe-show_unstable")));

		gchar* l_sHaystackName;
		gchar* l_sHaystackDescription;
		gboolean l_bHaystackUnstable;

		gtk_tree_model_get(model, iter, 0, &l_sHaystackName, 1, &l_sHaystackDescription, 6, &l_bHaystackUnstable, -1);

		// consider only leaf nodes which match the search term
		if (l_sHaystackName!=NULL && l_sHaystackDescription!=NULL)
		{
			if ((l_bShowUnstable || !l_bHaystackUnstable) && (string::npos != strtoupper(l_sHaystackName).find(strtoupper(l_pApplication->m_sSearchTerm)) || string::npos != strtoupper(l_sHaystackDescription).find(strtoupper(l_pApplication->m_sSearchTerm)) || gtk_tree_model_iter_has_child(model, iter)))
			{
				//std::cout << "value : " << l_pApplication->m_sSearchTerm << "\n";
				l_bVisible = true;
			}

			g_free(l_sHaystackName);
			g_free(l_sHaystackDescription);
		}
		else
		{
			l_bVisible = true;
		}

		return l_bVisible;
	}

	static gboolean box_algorithm_prune_empty_folders(GtkTreeModel *model, GtkTreeIter *iter, gpointer pUserData)
	{
		gboolean l_bIsPlugin;
		gtk_tree_model_get(model, iter, 5, &l_bIsPlugin, -1);

		if (gtk_tree_model_iter_has_child(model, iter) || l_bIsPlugin)
		{
			return true;
		}

		return false;
	}

	static gboolean	do_refilter( CApplication *pApplication )
	{
		/*
		if (0 == strcmp(pApplication->m_sSearchTerm, ""))
		{
			// reattach the old model
			gtk_tree_view_set_model(pApplication->m_pBoxAlgorithmTreeView, GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModel));
		}
		else
		*/
		{
			pApplication->m_pBoxAlgorithmTreeModelFilter = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModel), NULL);
			pApplication->m_pBoxAlgorithmTreeModelFilter2 = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter), NULL);
			pApplication->m_pBoxAlgorithmTreeModelFilter3 = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter2), NULL);
			pApplication->m_pBoxAlgorithmTreeModelFilter4 = gtk_tree_model_filter_new(GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter3), NULL);
			// detach the normal model from the treeview
			gtk_tree_view_set_model(pApplication->m_pBoxAlgorithmTreeView, NULL);

			// clear the model

			// add a filtering function to the model
			gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter), box_algorithm_search_func, pApplication, NULL );
			gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter2), box_algorithm_prune_empty_folders, pApplication, NULL );
			gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter3), box_algorithm_prune_empty_folders, pApplication, NULL );
			gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(pApplication->m_pBoxAlgorithmTreeModelFilter4), box_algorithm_prune_empty_folders, pApplication, NULL );

			// attach the model to the treeview
			gtk_tree_view_set_model(pApplication->m_pBoxAlgorithmTreeView, GTK_TREE_MODEL(pApplication->m_pBoxAlgorithmTreeModelFilter4));

			if (0 == strcmp(pApplication->m_sSearchTerm, ""))
			{
				gtk_tree_view_collapse_all(pApplication->m_pBoxAlgorithmTreeView);
			}
			else
			{
				gtk_tree_view_expand_all(pApplication->m_pBoxAlgorithmTreeView);
			}
		}

		pApplication->m_giFilterTimeout = 0;

		return false;
	}

	static void	queue_refilter( CApplication* pApplication )
	{
		if( pApplication->m_giFilterTimeout )
			g_source_remove( pApplication->m_giFilterTimeout );

		pApplication->m_giFilterTimeout = g_timeout_add( 300, (GSourceFunc)do_refilter, pApplication );
	}

	void refresh_search_cb(::GtkEntry* pTextfield, CApplication* pApplication)
	{
		pApplication->m_sSearchTerm = gtk_entry_get_text(pTextfield);

		queue_refilter(pApplication);
	}

	void refresh_search_no_data_cb(::GtkToggleButton* pToggleButton, CApplication* pApplication)
	{
		pApplication->m_sSearchTerm = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-box_algorithm_searchbox")));

		queue_refilter(pApplication);
	}


	static gboolean searchbox_focus_in_cb(::GtkWidget* pWidget, ::GdkEvent* pEvent, CApplication* pApplication)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-menu_edit")), false);

		return false;
	}

	static gboolean searchbox_focus_out_cb(::GtkWidget* pWidget, ::GdkEvent* pEvent, CApplication* pApplication)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(pApplication->m_pBuilderInterface, "openvibe-menu_edit")), true);

		return false;
	}

	gboolean idle_application_loop(gpointer pUserData)
	{
		CApplication* l_pApplication=static_cast<CApplication*>(pUserData);
		CInterfacedScenario* l_pCurrentInterfacedScenario=l_pApplication->getCurrentInterfacedScenario();
		if(l_pCurrentInterfacedScenario)
		{

			if(l_pApplication->getPlayer() && l_pCurrentInterfacedScenario->m_ePlayerStatus != l_pApplication->getPlayer()->getStatus())
			{
				switch(l_pApplication->getPlayer()->getStatus())
				{
					case PlayerStatus_Stop:
						switch(l_pCurrentInterfacedScenario->m_ePlayerStatus)
						{
						case PlayerStatus_Play:
							l_pApplication->m_eReplayMode = CApplication::EReplayMode_Play;
							break;
						case PlayerStatus_Forward:
							l_pApplication->m_eReplayMode = CApplication::EReplayMode_Forward;
							break;
						// case PlayerStatus_Stop:
						// case PlayerStatus_Uninitialized:
						// case PlayerStatus_Pause:
						// case PlayerStatus_Step:
						default:
							// don't care
							l_pApplication->m_rKernelContext.getLogManager() << LogLevel_Trace << "Ran into unhandled status " << l_pCurrentInterfacedScenario->m_ePlayerStatus << "\n";
							break;
						}
						
						gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_stop")), "clicked");
						break;
					case PlayerStatus_Pause:   while(l_pCurrentInterfacedScenario->m_ePlayerStatus != PlayerStatus_Pause) gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_play_pause")), "clicked"); break;
					case PlayerStatus_Play:    while(l_pCurrentInterfacedScenario->m_ePlayerStatus != PlayerStatus_Play)  gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_play_pause")), "clicked"); break;
					case PlayerStatus_Forward: gtk_signal_emit_by_name(GTK_OBJECT(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-button_forward")), "clicked"); break;
					default: std::cout << "unhandled :(\n"; break;
				}
			}
			else
			{
				float64 l_f64Time=(l_pCurrentInterfacedScenario->m_pPlayer 
					? ITimeArithmetics::timeToSeconds(l_pCurrentInterfacedScenario->m_pPlayer->getCurrentSimulatedTime()) : 0);
				if(l_pApplication->m_f64LastTimeRefresh!=l_f64Time)
				{
					l_pApplication->m_f64LastTimeRefresh=l_f64Time;
					uint32 l_ui32Milli  = ((uint32)(l_f64Time*1000)%1000);
					uint32 l_ui32Seconds=  ((uint32)l_f64Time)%60;
					uint32 l_ui32Minutes= (((uint32)l_f64Time)/60)%60;
					uint32 l_ui32Hours  =((((uint32)l_f64Time)/60)/60);

					float64 l_f64CPUUsage=(l_pCurrentInterfacedScenario->m_pPlayer?l_pCurrentInterfacedScenario->m_pPlayer->getCPUUsage(OV_UndefinedIdentifier):0);

					char l_sTime[1024];
					if(l_ui32Hours)				sprintf(l_sTime, "Time : %02dh %02dm %02ds %03dms", l_ui32Hours, l_ui32Minutes, l_ui32Seconds, l_ui32Milli);
					else if(l_ui32Minutes)		sprintf(l_sTime, "Time : %02dm %02ds %03dms", l_ui32Minutes, l_ui32Seconds, l_ui32Milli);
					else if(l_ui32Seconds)		sprintf(l_sTime, "Time : %02ds %03dms", l_ui32Seconds, l_ui32Milli);
					else						sprintf(l_sTime, "Time : %03dms", l_ui32Milli);

					gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-label_current_time")), l_sTime);

					char l_sCPU[1024];
					sprintf(l_sCPU, "%3.01f%%", l_f64CPUUsage);

					gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-progressbar_cpu_usage")), l_f64CPUUsage*.01);
					gtk_progress_bar_set_text(GTK_PROGRESS_BAR(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-progressbar_cpu_usage")), l_sCPU);
					if(l_pCurrentInterfacedScenario->m_pPlayer&&l_pCurrentInterfacedScenario->m_bDebugCPUUsage)
					{
						// redraws scenario
						l_pCurrentInterfacedScenario->forceRedraw();
					}
				}
			}
		}
		else
		{
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-label_current_time")), "Time : 000ms");
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-progressbar_cpu_usage")), "");
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(gtk_builder_get_object(l_pApplication->m_pBuilderInterface, "openvibe-progressbar_cpu_usage")), 0);
		}

		if(!l_pApplication->hasRunningScenario())
		{
			System::Time::sleep(50);
		}

		return TRUE;
	}

	gboolean idle_scenario_loop(gpointer pUserData)
	{
		CInterfacedScenario* l_pInterfacedScenario=static_cast<CInterfacedScenario*>(pUserData);
		uint64 l_ui64CurrentTime=System::Time::zgetTime();
		l_pInterfacedScenario->m_pPlayer->loop(l_ui64CurrentTime-l_pInterfacedScenario->m_ui64LastLoopTime);
		l_pInterfacedScenario->m_ui64LastLoopTime=l_ui64CurrentTime;
		return TRUE;
	}

	gboolean timeout_application_loop(gpointer pUserData)
	{
		CApplication* l_pApplication=static_cast<CApplication*>(pUserData);
		if(!l_pApplication->hasRunningScenario() && l_pApplication->m_eCommandLineFlags&CommandLineFlag_NoGui)
		{
			l_pApplication->quitApplicationCB();
			gtk_main_quit();
			return FALSE;
		}
		return TRUE;
	}

	void click_callback(::GtkWidget* pWidget, GdkEventButton *pEvent, gpointer pData)
	{
		//log text view grab the focus so isLogAreaClicked() return true and CTRL+F will focus on the log searchEntry
		gtk_widget_grab_focus(pWidget);

		CApplication* l_pApplication=static_cast<CApplication*>(pData);

		//if left click
		if (pEvent->button == 1)
		{
			GtkTextView* l_pTextView = GTK_TEXT_VIEW(pWidget);
			GtkTextWindowType l_oWindowType = gtk_text_view_get_window_type(l_pTextView, pEvent->window);
			gint l_iBufferX, l_iBufferY;
			//convert event coord (mouse position) in buffer coord (character in buffer)
			gtk_text_view_window_to_buffer_coords(l_pTextView, l_oWindowType, ov_round(pEvent->x), ov_round(pEvent->y), &l_iBufferX, &l_iBufferY);
			//get the text iter corresponding to that position
			GtkTextIter l_oIter;
			gtk_text_view_get_iter_at_location(l_pTextView, &l_oIter, l_iBufferX, l_iBufferY);

			//if this position is not tagged, exit
			GtkTextTag* l_pTag = const_cast<GtkTextTag*>(l_pApplication->m_pCIdentifierTag);
			if(!gtk_text_iter_has_tag(&l_oIter, l_pTag))
			{
				return;
			}
			else //if the position is tagged, we are on a CIdentifier
			{
				GtkTextIter l_oStart = l_oIter;
				GtkTextIter l_oEnd = l_oIter;

				while(gtk_text_iter_has_tag(&l_oEnd, l_pTag))
				{
					gtk_text_iter_forward_char(&l_oEnd);
				}
				while(gtk_text_iter_has_tag(&l_oStart, l_pTag))
				{
					gtk_text_iter_backward_char(&l_oStart);
				}
				//we went one char to far for start
				gtk_text_iter_forward_char(&l_oStart);
				//this contains the CIdentifier
				gchar * l_sLink=gtk_text_iter_get_text(&l_oStart, &l_oEnd);
				//cout << "cid is |" << link << "|" << endl;
				CIdentifier l_oId;
				l_oId.fromString(CString(l_sLink));
				l_pApplication->getCurrentInterfacedScenario()->centerOnBox(l_oId);
			}

		}


	}
}

static gboolean key_press_event_cb(::GtkWidget* pWidget, ::GdkEventKey* pEvent, gpointer pUserData)
{
	static_cast<CApplication*>(pUserData)->keyPressEventCB(pWidget, pEvent);
	return false;
}

static ::GtkTargetEntry g_vTargetEntry[]= {
	{ (gchar*)"STRING", 0, 0 },
	{ (gchar*)"text/plain", 0, 0 } };

CApplication::CApplication(const IKernelContext& rKernelContext)
	:m_rKernelContext(rKernelContext)
	,m_pPluginManager(NULL)
	,m_pScenarioManager(NULL)
	,m_pVisualisationManager(NULL)
	,m_pClipboardScenario(NULL)
	,m_eCommandLineFlags(CommandLineFlag_None)
	,m_pBuilderInterface(NULL)
	,m_pMainWindow(NULL)
	,m_pScenarioNotebook(NULL)
	,m_pResourceNotebook(NULL)
	,m_pBoxAlgorithmTreeModel(NULL)
	,m_pBoxAlgorithmTreeView(NULL)
	,m_pAlgorithmTreeModel(NULL)
	,m_pAlgorithmTreeView(NULL)
	,m_giFilterTimeout(0)
	,m_f64LastTimeRefresh(0.0)
	,m_bIsQuitting(false)
	,m_i32CurrentScenarioPage(-1)
	,m_pInitAlert(NULL)
	,m_eReplayMode(EReplayMode_None)
{
	m_pPluginManager=&m_rKernelContext.getPluginManager();
	m_pScenarioManager=&m_rKernelContext.getScenarioManager();
	m_pVisualisationManager=&m_rKernelContext.getVisualisationManager();
	m_pLogListenerDesigner = NULL;
	m_pTextView = NULL;
}

CApplication::~CApplication(void) 
{
	if(m_pBuilderInterface)
	{
		// @FIXME this likely still does not deallocate the actual widgets allocated by add_from_file
		g_object_unref(G_OBJECT(m_pBuilderInterface));
		m_pBuilderInterface = NULL;
	}
}

void CApplication::initialize(ECommandLineFlag eCommandLineFlags)
{
	m_eCommandLineFlags=eCommandLineFlags;
	m_sSearchTerm = "";
	m_sLogSearchTerm = "";
	m_pCIdentifierTag = NULL;

	// m_vCheckItems.clear();

	// Prepares scenario clipboard
	CIdentifier l_oClipboardScenarioIdentifier;
	if(m_pScenarioManager->createScenario(l_oClipboardScenarioIdentifier))
	{
		m_pClipboardScenario=&m_pScenarioManager->getScenario(l_oClipboardScenarioIdentifier);
	}

	m_pBuilderInterface=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "openvibe", NULL);
	gtk_builder_add_from_file(m_pBuilderInterface, OVD_GUI_File, NULL);
	gtk_builder_connect_signals(m_pBuilderInterface, NULL);

	m_pMainWindow=GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe"));
	g_signal_connect(G_OBJECT(m_pMainWindow), "key-press-event", G_CALLBACK(key_press_event_cb), this);
	m_pZoomSpinner = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-zoom_spinner"));

	gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_window")), false);

	// Catch delete events when close button is clicked
	g_signal_connect(m_pMainWindow, "delete_event", G_CALLBACK(button_quit_application_cb), this);

	// Connects menu actions
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_undo")),        "activate", G_CALLBACK(menu_undo_cb),               this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_redo")),        "activate", G_CALLBACK(menu_redo_cb),               this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_focus_search")),"activate", G_CALLBACK(menu_focus_search_cb),     this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_copy")),        "activate", G_CALLBACK(menu_copy_selection_cb),     this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_cut")),         "activate", G_CALLBACK(menu_cut_selection_cb),      this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_paste")),       "activate", G_CALLBACK(menu_paste_selection_cb),    this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_delete")),      "activate", G_CALLBACK(menu_delete_selection_cb),   this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_preferences")), "activate", G_CALLBACK(menu_preferences_cb),        this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_new")),         "activate", G_CALLBACK(menu_new_scenario_cb),       this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_open")),        "activate", G_CALLBACK(menu_open_scenario_cb),      this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_save")),        "activate", G_CALLBACK(menu_save_scenario_cb),      this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_save_as")),     "activate", G_CALLBACK(menu_save_scenario_as_cb),   this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_close")),       "activate", G_CALLBACK(menu_close_scenario_cb),     this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_quit")),        "activate", G_CALLBACK(menu_quit_application_cb),   this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_about")),          "activate", G_CALLBACK(menu_about_openvibe_cb),  this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_scenario_about")), "activate", G_CALLBACK(menu_about_scenario_cb),  this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_documentation")),  "activate", G_CALLBACK(menu_browse_documentation_cb),   this);

	// g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_test")),        "activate", G_CALLBACK(menu_test_cb),               this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_new")),       "clicked",  G_CALLBACK(button_new_scenario_cb),     this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_open")),      "clicked",  G_CALLBACK(button_open_scenario_cb),    this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_save")),      "clicked",  G_CALLBACK(button_save_scenario_cb),    this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_save_as")),   "clicked",  G_CALLBACK(button_save_scenario_as_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_close")),     "clicked",  G_CALLBACK(button_close_scenario_cb),   this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_log_level")),     "clicked",  G_CALLBACK(log_level_cb),                    this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), "toggled",  G_CALLBACK(button_toggle_window_manager_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_comment")),       "clicked", G_CALLBACK(button_comment_cb),        this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_aboutscenario")), "clicked", G_CALLBACK(button_about_scenario_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),       "clicked",  G_CALLBACK(stop_scenario_cb),          this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), "clicked",  G_CALLBACK(play_pause_scenario_cb),    this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),       "clicked",  G_CALLBACK(next_scenario_cb),          this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),    "clicked",  G_CALLBACK(forward_scenario_cb),       this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_zoomin")),    "clicked",  G_CALLBACK(zoom_in_scenario_cb),       this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_zoomout")),    "clicked",  G_CALLBACK(zoom_out_scenario_cb),       this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-zoom_spinner")),    "value-changed",  G_CALLBACK(spinner_zoom_changed_cb),       this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_title_button_expand")),   "clicked", G_CALLBACK(box_algorithm_title_button_expand_cb),   this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_title_button_collapse")), "clicked", G_CALLBACK(box_algorithm_title_button_collapse_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_title_button_expand")),   "clicked", G_CALLBACK(algorithm_title_button_expand_cb),   this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_title_button_collapse")), "clicked", G_CALLBACK(algorithm_title_button_collapse_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "changed", G_CALLBACK(refresh_search_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "focus-in-event", G_CALLBACK(searchbox_focus_in_cb), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_searchbox")), "focus-out-event", G_CALLBACK(searchbox_focus_out_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-show_unstable")), "toggled", G_CALLBACK(refresh_search_no_data_cb), this);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "searchEntry")),		"changed", G_CALLBACK(refresh_search_log_entry), this);

#if defined(TARGET_OS_Windows)
#if GTK_CHECK_VERSION(2,24,0)
	// expect it to work */
#else
	gtk_about_dialog_set_url_hook ((GtkAboutDialogActivateLinkFunc)menu_about_link_clicked_cb, this, NULL);
#endif
#endif

	__g_idle_add__(idle_application_loop, this);
	__g_timeout_add__(1000, timeout_application_loop, this);

	// Prepares main notebooks
	m_pScenarioNotebook=GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderInterface, "openvibe-scenario_notebook"));
	g_signal_connect(G_OBJECT(m_pScenarioNotebook), "switch-page", G_CALLBACK(change_current_scenario_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioNotebook), "page-reordered", G_CALLBACK(reorder_scenario_cb), this);
	m_pResourceNotebook=GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderInterface, "openvibe-resource_notebook"));

	m_pInitAlert = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_init_alert"));
	gtk_toggle_button_set_active(m_pInitAlert, (OpenViBE::boolean)(m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_PopUpOnInitError}")));

	// Creates an empty scnenario
	gtk_notebook_remove_page(m_pScenarioNotebook, 0);
	//newScenarioCB();
	{
		// Prepares box algorithm view
		m_pBoxAlgorithmTreeView=GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_tree"));
		::GtkTreeViewColumn* l_pTreeViewColumnName=gtk_tree_view_column_new();
		::GtkTreeViewColumn* l_pTreeViewColumnDesc=gtk_tree_view_column_new();
		::GtkCellRenderer* l_pCellRendererIcon=gtk_cell_renderer_pixbuf_new();
		::GtkCellRenderer* l_pCellRendererName=gtk_cell_renderer_text_new();
		::GtkCellRenderer* l_pCellRendererDesc=gtk_cell_renderer_text_new();
		gtk_tree_view_column_set_title(l_pTreeViewColumnName, "Name");
		gtk_tree_view_column_set_title(l_pTreeViewColumnDesc, "Description");
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererIcon, FALSE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererName, TRUE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnDesc, l_pCellRendererDesc, TRUE);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererIcon, "stock-id", Resource_StringStockIcon, NULL);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererName, "text", Resource_StringName, "foreground", Resource_StringColor, NULL);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnDesc, l_pCellRendererDesc, "text", Resource_StringShortDescription, "foreground", Resource_StringColor, NULL);
		gtk_tree_view_column_set_sizing(l_pTreeViewColumnName, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_sizing(l_pTreeViewColumnDesc, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_expand(l_pTreeViewColumnName, FALSE);
		gtk_tree_view_column_set_expand(l_pTreeViewColumnDesc, FALSE);
		gtk_tree_view_column_set_resizable(l_pTreeViewColumnName, TRUE);
		gtk_tree_view_column_set_resizable(l_pTreeViewColumnDesc, TRUE);
		gtk_tree_view_column_set_min_width(l_pTreeViewColumnName, 64);
		gtk_tree_view_column_set_min_width(l_pTreeViewColumnDesc, 64);
		gtk_tree_view_column_set_fixed_width(l_pTreeViewColumnName, 256);
		gtk_tree_view_column_set_fixed_width(l_pTreeViewColumnDesc, 512);
		gtk_tree_view_append_column(m_pBoxAlgorithmTreeView, l_pTreeViewColumnName);
		gtk_tree_view_append_column(m_pBoxAlgorithmTreeView, l_pTreeViewColumnDesc);

		// g_signal_connect(G_OBJECT(m_pBoxAlgorithmTreeView), "querry_tooltip", G_CALLBACK(resource_query_tooltip_cb), this);
		//
		// Prepares box algorithm model
		m_pBoxAlgorithmTreeModel=gtk_tree_store_new(7, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);

		// Tree Storage for the searches
		gtk_tree_view_set_model(m_pBoxAlgorithmTreeView, GTK_TREE_MODEL(m_pBoxAlgorithmTreeModel) );
	}

	{
		// Prepares algorithm view
		m_pAlgorithmTreeView=GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-algorithm_tree"));
		::GtkTreeViewColumn* l_pTreeViewColumnName=gtk_tree_view_column_new();
		::GtkTreeViewColumn* l_pTreeViewColumnDesc=gtk_tree_view_column_new();
		::GtkCellRenderer* l_pCellRendererIcon=gtk_cell_renderer_pixbuf_new();
		::GtkCellRenderer* l_pCellRendererName=gtk_cell_renderer_text_new();
		::GtkCellRenderer* l_pCellRendererDesc=gtk_cell_renderer_text_new();
		gtk_tree_view_column_set_title(l_pTreeViewColumnName, "Name");
		gtk_tree_view_column_set_title(l_pTreeViewColumnDesc, "Description");
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererIcon, FALSE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnName, l_pCellRendererName, TRUE);
		gtk_tree_view_column_pack_start(l_pTreeViewColumnDesc, l_pCellRendererDesc, TRUE);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererIcon, "stock-id", Resource_StringStockIcon, NULL);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnName, l_pCellRendererName, "text", Resource_StringName, "foreground", Resource_StringColor, NULL);
		gtk_tree_view_column_set_attributes(l_pTreeViewColumnDesc, l_pCellRendererDesc, "text", Resource_StringShortDescription, "foreground", Resource_StringColor, NULL);

		gtk_tree_view_column_set_sizing(l_pTreeViewColumnName, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_sizing(l_pTreeViewColumnDesc, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_expand(l_pTreeViewColumnName, FALSE);
		gtk_tree_view_column_set_expand(l_pTreeViewColumnDesc, FALSE);
		gtk_tree_view_column_set_resizable(l_pTreeViewColumnName, TRUE);
		gtk_tree_view_column_set_resizable(l_pTreeViewColumnDesc, TRUE);
		gtk_tree_view_column_set_min_width(l_pTreeViewColumnName, 64);
		gtk_tree_view_column_set_min_width(l_pTreeViewColumnDesc, 64);
		gtk_tree_view_column_set_fixed_width(l_pTreeViewColumnName, 256);
		gtk_tree_view_column_set_fixed_width(l_pTreeViewColumnDesc, 512);
		gtk_tree_view_append_column(m_pAlgorithmTreeView, l_pTreeViewColumnName);
		gtk_tree_view_append_column(m_pAlgorithmTreeView, l_pTreeViewColumnDesc);
		// g_signal_connect(G_OBJECT(m_pAlgorithmTreeView), "querry_tooltip", G_CALLBACK(resource_query_tooltip_cb), this);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-show_unstable")), m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowUnstable}"));

		// Prepares algorithm model
		m_pAlgorithmTreeModel=gtk_tree_store_new(7, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);
		gtk_tree_view_set_model(m_pAlgorithmTreeView, GTK_TREE_MODEL(m_pAlgorithmTreeModel));


	}

	GtkHPaned *l_pPaned = GTK_HPANED(gtk_builder_get_object(m_pBuilderInterface, "openvibe-horizontal_container"));
	const int64 l_i64Position = m_rKernelContext.getConfigurationManager().expandAsInteger("${Designer_HorizontalContainerPosition}", -1);
	if(l_i64Position != -1)
	{
		gtk_paned_set_position(GTK_PANED(l_pPaned), static_cast<gint>(l_i64Position));
	}


	// Prepares drag & drop for box creation
	gtk_drag_source_set(GTK_WIDGET(m_pBoxAlgorithmTreeView), GDK_BUTTON1_MASK, g_vTargetEntry, sizeof(g_vTargetEntry)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);
	g_signal_connect(
		G_OBJECT(m_pBoxAlgorithmTreeView),
		"drag_data_get",
		G_CALLBACK(drag_data_get_cb),
		this);

	// Shows main window
	gtk_builder_connect_signals(m_pBuilderInterface, NULL);
	if(m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_FullscreenEditor}"))
	{
		gtk_window_maximize(GTK_WINDOW(m_pMainWindow));
	}
	else 
	{
		const gint l_iWidth = static_cast<gint>(m_rKernelContext.getConfigurationManager().expandAsInteger("${Designer_WindowWidth}", -1));
		const gint l_iHeight = static_cast<gint>(m_rKernelContext.getConfigurationManager().expandAsInteger("${Designer_WindowHeight}", -1));
		if (l_iWidth != -1 && l_iHeight != -1)
		{
			gtk_window_resize(GTK_WINDOW(m_pMainWindow), l_iWidth, l_iHeight);
		}
	}

	if(!m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowAlgorithms}"))
	{
		gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderInterface, "openvibe-resource_notebook")), 1);
	}

	// gtk_window_set_icon_name(GTK_WINDOW(m_pMainWindow), "ov-logo");
	// gtk_window_set_icon_from_file(GTK_WINDOW(m_pMainWindow), "../share/applications/designer/ov-logo.png", NULL);

	if(!(m_eCommandLineFlags&CommandLineFlag_NoManageSession))
	{
		CIdentifier l_oTokenIdentifier;
		char l_sVarName[1024];
		unsigned i=0;
		do
		{
			::sprintf(l_sVarName, "Designer_LastScenarioFilename_%03i", ++i);
			if((l_oTokenIdentifier=m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier(l_sVarName))!=OV_UndefinedIdentifier)
			{
				CString l_sFilename;
				l_sFilename=m_rKernelContext.getConfigurationManager().getConfigurationTokenValue(l_oTokenIdentifier);
				l_sFilename=m_rKernelContext.getConfigurationManager().expand(l_sFilename);
				m_rKernelContext.getLogManager() << LogLevel_Info << "Restoring scenario [" << l_sFilename << "]\n";
				if(!this->openScenario(l_sFilename.toASCIIString()))
				{
					m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Failed to restore scenario [" << l_sFilename << "]\n";
				}
			}
		}
		while(l_oTokenIdentifier!=OV_UndefinedIdentifier);
	}
	refresh_search_no_data_cb(NULL, this);

	if(!(m_eCommandLineFlags&CommandLineFlag_NoGui))
	{
		m_pLogListenerDesigner = new CLogListenerDesigner(m_rKernelContext, m_pBuilderInterface);
		m_rKernelContext.getLogManager().addListener(m_pLogListenerDesigner);

		logLevelRestore(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_debug"), LogLevel_Debug, "${Designer_DebugCanal}");
		logLevelRestore(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_bench"), LogLevel_Benchmark, "${Designer_BenchCanal}");
		logLevelRestore(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_trace"), LogLevel_Trace, "${Designer_TraceCanal}");
		logLevelRestore(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_info"), LogLevel_Info, "${Designer_InfoCanal}");
		logLevelRestore(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_warning"), LogLevel_Warning, "${Designer_WarningCanal}");
		logLevelRestore(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_impwarning"), LogLevel_ImportantWarning, "${Designer_ImportantWarningCanal}");
		logLevelRestore(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_error"), LogLevel_Error, "${Designer_ErrorCanal}");
		logLevelRestore(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_fatal"), LogLevel_Fatal, "${Designer_FatalCanal}");


		CIdentifier l_oTokenIdentifier;
		l_oTokenIdentifier = m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier("Designer_LogExpanderStatus");
		if(l_oTokenIdentifier != OV_UndefinedIdentifier)
		{
			CString l_sExpanderStatus;
			l_sExpanderStatus = m_rKernelContext.getConfigurationManager().getConfigurationTokenValue(l_oTokenIdentifier);
			gtk_expander_set_expanded(GTK_EXPANDER(gtk_builder_get_object(m_pBuilderInterface, "openvibe-expander_messages")),
									  m_rKernelContext.getConfigurationManager().expandAsBoolean(l_sExpanderStatus));
		}


		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_clear")),       "clicked",  G_CALLBACK(clear_messages_cb), m_pLogListenerDesigner);

		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_search")),       "clicked",  G_CALLBACK(search_messages_cb), this);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "searchEntry")),		"activate", G_CALLBACK(search_messages_cb), this);

		m_pTextView = GTK_TEXT_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-textview_messages"));
		GtkTextBuffer* l_pBuffer =  gtk_text_view_get_buffer( m_pTextView );
		GtkTextTagTable* l_pTagtable =  gtk_text_buffer_get_tag_table(l_pBuffer);
		//GtkTextTag* url
		m_pCIdentifierTag = gtk_text_tag_table_lookup(l_pTagtable, "link");
		if(m_pCIdentifierTag!=NULL)
		{
			//g_object_set_data (url, "tag", url);
			//g_object_set_data (url, "application", this);
			//m_pCIdentifierTag =url;
			g_signal_connect(G_OBJECT(m_pTextView), "button_press_event", G_CALLBACK(click_callback), this);
		}

		int64 l_i64LastScenarioPage = m_rKernelContext.getConfigurationManager().expandAsInteger("${Designer_CurrentScenarioPage}", -1);
		if(l_i64LastScenarioPage>=0 && l_i64LastScenarioPage<static_cast<int64>(m_vInterfacedScenario.size()))		
		{
			gtk_notebook_set_current_page(m_pScenarioNotebook, static_cast<gint>(l_i64LastScenarioPage));
		}
		gtk_widget_show(m_pMainWindow);
	}
}

boolean CApplication::openScenario(const char* sFileName)
{
	CIdentifier l_oScenarioIdentifier;
	if(m_pScenarioManager->createScenario(l_oScenarioIdentifier))
	{
		IScenario& l_rScenario=m_pScenarioManager->getScenario(l_oScenarioIdentifier);

		CMemoryBuffer l_oMemoryBuffer;
		boolean l_bSuccess=false;

		if(::strcmp(sFileName, "-")==0)
		{
			m_rKernelContext.getLogManager() << LogLevel_Info << "Reading from standard input...\n";
			unsigned int l_uiSize=0;
			FILE* l_pFile=stdin;
			while(1)
			{
				unsigned char c=::fgetc(l_pFile);
				if(::feof(l_pFile)) break;
				l_uiSize++;
				l_oMemoryBuffer.setSize(l_uiSize, false);
				l_oMemoryBuffer[l_uiSize-1]=c;
			}
			m_rKernelContext.getLogManager() << LogLevel_Debug << "Finished reading standard input...\n" << (const char*)&l_oMemoryBuffer[0] << "\n";
		}
		else
		{
			FILE* l_pFile=fopen(sFileName, "rb");
			if(l_pFile)
			{
				::fseek(l_pFile, 0, SEEK_END);
				l_oMemoryBuffer.setSize(::ftell(l_pFile), true);
				::fseek(l_pFile, 0, SEEK_SET);
				if(::fread(reinterpret_cast<char*>(l_oMemoryBuffer.getDirectPointer()), (size_t)l_oMemoryBuffer.getSize(), 1, l_pFile)!=1)
				{
					m_rKernelContext.getLogManager() << LogLevel_Error << "Problem reading '" << sFileName << "'\n";
					::fclose(l_pFile);
					m_pScenarioManager->releaseScenario(l_oScenarioIdentifier);
					return false;
				}
				::fclose(l_pFile);
			} 
			else 
			{
				m_rKernelContext.getLogManager() << LogLevel_Error << "Unable to open '" << sFileName << "' for reading\n";
				m_pScenarioManager->releaseScenario(l_oScenarioIdentifier);
				return false;
			}
		}

		if(l_oMemoryBuffer.getSize())
		{
			CIdentifier l_oImporterIdentifier=m_rKernelContext.getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_XMLScenarioImporter);
			if(l_oImporterIdentifier!=OV_UndefinedIdentifier)
			{
				IAlgorithmProxy* l_pImporter=&m_rKernelContext.getAlgorithmManager().getAlgorithm(l_oImporterIdentifier);
				if(l_pImporter)
				{
					m_rKernelContext.getLogManager() << LogLevel_Info << "Importing scenario...\n";

					l_bSuccess = l_pImporter->initialize();

					TParameterHandler < const IMemoryBuffer* > ip_pMemoryBuffer(l_pImporter->getInputParameter(OVTK_Algorithm_ScenarioImporter_InputParameterId_MemoryBuffer));
					TParameterHandler < IScenario* > op_pScenario(l_pImporter->getOutputParameter(OVTK_Algorithm_ScenarioImporter_OutputParameterId_Scenario));

					ip_pMemoryBuffer=&l_oMemoryBuffer;
					op_pScenario=&l_rScenario;

					l_bSuccess &= l_pImporter->process();
					l_bSuccess &= l_pImporter->uninitialize();

					m_rKernelContext.getAlgorithmManager().releaseAlgorithm(*l_pImporter);

				}
			}
		}

		if(l_bSuccess)
		{
			CIdentifier l_oVisualisationWidgetIdentifier;
			CIdentifier l_oBoxIdentifier;

			//ensure visualisation widgets contained in the scenario (if any) appear in the window manager
			//even when the <VisualisationTree> section of a scenario file is missing, erroneous or deprecated
			IVisualisationTree& l_rVisualisationTree = m_rKernelContext.getVisualisationManager().getVisualisationTree(l_rScenario.getVisualisationTreeIdentifier());

			//no visualisation widget was added to visualisation tree : ensure there aren't any in scenario
			while((l_oBoxIdentifier=l_rScenario.getNextBoxIdentifier(l_oBoxIdentifier)) != OV_UndefinedIdentifier)
			{
				if(l_rVisualisationTree.getVisualisationWidgetFromBoxIdentifier(l_oBoxIdentifier)==NULL)
				{
					const IBox* l_pBox = l_rScenario.getBoxDetails(l_oBoxIdentifier);
					CIdentifier l_oAlgorithmIdentifier = l_pBox->getAlgorithmClassIdentifier();
					const IPluginObjectDesc* l_pPOD = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_oAlgorithmIdentifier);
					if((l_pPOD != NULL && l_pPOD->hasFunctionality(PluginFunctionality_Visualization))||(l_pBox->hasModifiableSettings()))
					{
						//a visualisation widget was found in scenario : manually add it to visualisation tree
						l_rVisualisationTree.addVisualisationWidget(
								l_oVisualisationWidgetIdentifier,
								l_pBox->getName(),
								EVisualisationWidget_VisualisationBox,
								OV_UndefinedIdentifier,
								0,
								l_pBox->getIdentifier(),
								0,
								OV_UndefinedIdentifier);
					}
				}
			}

			// Closes first unnamed scenario
			if(m_vInterfacedScenario.size()==1)
			{
				if(m_vInterfacedScenario[0]->m_bHasBeenModified==false && !m_vInterfacedScenario[0]->m_bHasFileName)
				{
					CIdentifier l_oScenarioIdentifierTmp=m_vInterfacedScenario[0]->m_oScenarioIdentifier;
					delete m_vInterfacedScenario[0];
					m_pScenarioManager->releaseScenario(l_oScenarioIdentifierTmp);
					m_vInterfacedScenario.clear();
				}
			}

			// Creates interfaced scenario
			CInterfacedScenario* l_pInterfacedScenario=new CInterfacedScenario(m_rKernelContext, *this, l_rScenario, l_oScenarioIdentifier, *m_pScenarioNotebook, OVD_GUI_File, OVD_GUI_Settings_File);
			if(l_pInterfacedScenario->m_pDesignerVisualisation != NULL)
			{
				l_pInterfacedScenario->m_pDesignerVisualisation->setDeleteEventCB(&::delete_designer_visualisation_cb, this);
				l_pInterfacedScenario->m_pDesignerVisualisation->load();
			}
			l_pInterfacedScenario->snapshotCB();
			l_pInterfacedScenario->m_sFileName=sFileName;
			l_pInterfacedScenario->m_bHasFileName=true;
			l_pInterfacedScenario->m_bHasBeenModified=false;
			l_pInterfacedScenario->updateScenarioLabel();
			m_vInterfacedScenario.push_back(l_pInterfacedScenario);
			gtk_notebook_set_current_page(m_pScenarioNotebook, gtk_notebook_get_n_pages(m_pScenarioNotebook)-1);
			//this->changeCurrentScenario(gtk_notebook_get_n_pages(m_pScenarioNotebook)-1);

			updateWorkingDirectoryToken(l_oScenarioIdentifier);

			return true;
		}
		else
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "Importing scenario from [" << sFileName << "] failed...\n";

			m_pScenarioManager->releaseScenario(l_oScenarioIdentifier);

			std::stringstream l_oStringStream;
			l_oStringStream << "The requested file: " << sFileName << "\n";
			l_oStringStream << "may either not be an OpenViBE scenario file, \n";
			l_oStringStream << "it may be corrupted or not compatible with \n";
			l_oStringStream << "the selected scenario importer...";

			if(!this->isNoGuiActive())
			{
				::GtkWidget* l_pErrorDialog=gtk_message_dialog_new(
						NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_OK,
						"Scenario importation process failed !");
				gtk_message_dialog_format_secondary_text(
						GTK_MESSAGE_DIALOG(l_pErrorDialog), "%s", l_oStringStream.str().c_str());
				gtk_dialog_run(GTK_DIALOG(l_pErrorDialog));
				gtk_widget_destroy(l_pErrorDialog);
			}
			else
			{
				m_rKernelContext.getLogManager() << LogLevel_Error << l_oStringStream.str().c_str() << "\n";
				releasePlayer();
				return false;
			}
		}
	}
	return false;
}

CString CApplication::getWorkingDirectory(void)
{
	CString l_sWorkingDirectory=m_rKernelContext.getConfigurationManager().expand("${Designer_DefaultWorkingDirectory}");

	CInterfacedScenario* l_pCurrentScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentScenario)
	{
		if(l_pCurrentScenario->m_bHasFileName)
		{
			std::string l_sCurrentDirectory=std::string(g_path_get_dirname(l_pCurrentScenario->m_sFileName.c_str()));
#if defined TARGET_OS_Windows
			std::replace(l_sCurrentDirectory.begin(), l_sCurrentDirectory.end(), '\\', '/');
#endif
			l_sWorkingDirectory=l_sCurrentDirectory.c_str();
		}
	}

	if(!g_path_is_absolute(l_sWorkingDirectory.toASCIIString()))
	{
		std::string l_sCurrentDirectory=g_get_current_dir();
#if defined TARGET_OS_Windows
		std::replace(l_sCurrentDirectory.begin(), l_sCurrentDirectory.end(), '\\', '/');
#endif
		l_sWorkingDirectory=l_sCurrentDirectory.c_str()+CString("/")+l_sWorkingDirectory;
	}

	return l_sWorkingDirectory;
}

// Change the working directory token to the current scenario location.
void CApplication::updateWorkingDirectoryToken(const OpenViBE::CIdentifier &oScenarioIdentifier) {
	// Store unique token for the working directory of the scenario. Note that OpenViBE will change the token value by itself.
	const OpenViBE::CString l_sWorkingDir = getWorkingDirectory();

	// This is the 'new' token
	const OpenViBE::CString l_sPathTokenWithID = "Player_ScenarioDirectory" + oScenarioIdentifier.toString();
	m_rKernelContext.getConfigurationManager().addOrReplaceConfigurationToken(l_sPathTokenWithID, l_sWorkingDir);
	const OpenViBE::CString l_sPathToken = "Player_ScenarioDirectory";
	m_rKernelContext.getConfigurationManager().addOrReplaceConfigurationToken(l_sPathToken, l_sWorkingDir);

	// We also need to save the token with the deprecated name as some scenarios might rely on it.
	// This token we do not need to save with an ID as the new token subsumes. 
	const CString l_sDeprecatedPathToken("__volatile_ScenarioDir");
	m_rKernelContext.getConfigurationManager().addOrReplaceConfigurationToken(l_sDeprecatedPathToken, l_sWorkingDir);

	m_rKernelContext.getLogManager() << LogLevel_Trace << "Scenario ( " << oScenarioIdentifier.toString() << " ) working directory changed to "  << l_sWorkingDir << "\n";
}

void CApplication::removeScenarioDirectoryToken(const CIdentifier &oScenarioIdentifier)
{
	const OpenViBE::CString l_sGlobalToken = "Player_ScenarioDirectory" + oScenarioIdentifier.toString();
	CIdentifier l_oToken = m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier(l_sGlobalToken);
	if(l_oToken != OV_UndefinedIdentifier)
	{
		m_rKernelContext.getConfigurationManager().releaseConfigurationToken(l_oToken);
	}

	// No need to handle the old __volatile_ScenarioDir with the id postfix as that token is no longer created
}

void CApplication::resetVolatileScenarioDirectoryToken()
{
	const OpenViBE::CString l_sGlobalToken = "Player_ScenarioDirectory";
	CIdentifier l_oToken = m_rKernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier(l_sGlobalToken);
	if(l_oToken != OV_UndefinedIdentifier)
	{
		m_rKernelContext.getConfigurationManager().releaseConfigurationToken(l_oToken);
	}

	// No need to handle the old __volatile_ScenarioDir with the id postfix as that token is no longer created
}

boolean CApplication::hasRunningScenario(void)
{
	vector<CInterfacedScenario*>::const_iterator itInterfacedScenario;
	for(itInterfacedScenario=m_vInterfacedScenario.begin(); itInterfacedScenario!=m_vInterfacedScenario.end(); itInterfacedScenario++)
	{
		if((*itInterfacedScenario)->m_pPlayer)
		{
			return true;
		}
	}
	return false;
}

boolean CApplication::hasUnsavedScenario(void)
{
	vector<CInterfacedScenario*>::const_iterator itInterfacedScenario;
	for(itInterfacedScenario=m_vInterfacedScenario.begin(); itInterfacedScenario!=m_vInterfacedScenario.end(); itInterfacedScenario++)
	{
		if((*itInterfacedScenario)->m_bHasBeenModified)
		{
			return true;
		}
	}
	return false;
}

CInterfacedScenario* CApplication::getCurrentInterfacedScenario(void)
{
	if(m_i32CurrentScenarioPage<static_cast<int32>(m_vInterfacedScenario.size()) && m_i32CurrentScenarioPage >= 0)
	{
		return m_vInterfacedScenario[m_i32CurrentScenarioPage];
	}
	return NULL;
}

void CApplication::dragDataGetCB(::GtkWidget* pWidget, ::GdkDragContext* pDragContex, ::GtkSelectionData* pSelectionData, guint uiInfo, guint uiT)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "dragDataGetCB\n";

	::GtkTreeView* l_pTreeView=GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderInterface, "openvibe-box_algorithm_tree"));
	::GtkTreeSelection* l_pTreeSelection=gtk_tree_view_get_selection(l_pTreeView);
	::GtkTreeModel* l_pTreeModel=NULL;
	::GtkTreeIter l_oTreeIter;
	if(gtk_tree_selection_get_selected(l_pTreeSelection, &l_pTreeModel, &l_oTreeIter))
	{
		const char* l_sBoxAlgorithmIdentifier=NULL;
		gtk_tree_model_get(
				l_pTreeModel, &l_oTreeIter,
				Resource_StringIdentifier, &l_sBoxAlgorithmIdentifier,
				-1);
		if(l_sBoxAlgorithmIdentifier)
		{
			gtk_selection_data_set(
					pSelectionData,
					GDK_SELECTION_TYPE_STRING,
					8,
					(const guchar*)l_sBoxAlgorithmIdentifier,
					strlen(l_sBoxAlgorithmIdentifier)+1);
		}
	}
}

void CApplication::undoCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "undoCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->undoCB();
	}
}

void CApplication::redoCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "redoCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->redoCB();
	}
}

void CApplication::copySelectionCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "copySelectionCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->copySelection();
	}
}

void CApplication::cutSelectionCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "cutSelectionCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->cutSelection();
	}
}

void CApplication::pasteSelectionCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "pasteSelectionCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->pasteSelection();
	}
}

void CApplication::deleteSelectionCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "deleteSelectionCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=this->getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->deleteSelection();
	}
}

void CApplication::preferencesCB(void)
{
	enum
	{
		Resource_TokenName,
		Resource_TokenValue,
		Resource_TokenExpand,
	};

	m_rKernelContext.getLogManager() << LogLevel_Trace << "preferencesCB\n";
	::GtkBuilder* l_pBuilderInterface=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "configuration_manager", NULL);
	gtk_builder_add_from_file(l_pBuilderInterface, OVD_GUI_File, NULL);
	gtk_builder_connect_signals(l_pBuilderInterface, NULL);

	::GtkWidget* l_pConfigurationManager=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "configuration_manager"));
	::GtkTreeView* l_pConfigurationManagerTreeView=GTK_TREE_VIEW(gtk_builder_get_object(l_pBuilderInterface, "configuration_manager-treeview"));

	// Prepares tree view
	::GtkTreeViewColumn* l_pTreeViewColumnTokenName=gtk_tree_view_column_new();
	::GtkTreeViewColumn* l_pTreeViewColumnTokenValue=gtk_tree_view_column_new();
	::GtkTreeViewColumn* l_pTreeViewColumnTokenExpand=gtk_tree_view_column_new();
	::GtkCellRenderer* l_pCellRendererTokenName=gtk_cell_renderer_text_new();
	::GtkCellRenderer* l_pCellRendererTokenValue=gtk_cell_renderer_text_new();
	::GtkCellRenderer* l_pCellRendererTokenExpand=gtk_cell_renderer_text_new();
	gtk_tree_view_column_set_title(l_pTreeViewColumnTokenName, "Token name");
	gtk_tree_view_column_set_title(l_pTreeViewColumnTokenValue, "Token value");
	gtk_tree_view_column_set_title(l_pTreeViewColumnTokenExpand, "Expanded token value");
	gtk_tree_view_column_pack_start(l_pTreeViewColumnTokenName, l_pCellRendererTokenName, TRUE);
	gtk_tree_view_column_pack_start(l_pTreeViewColumnTokenValue, l_pCellRendererTokenValue, TRUE);
	gtk_tree_view_column_pack_start(l_pTreeViewColumnTokenExpand, l_pCellRendererTokenExpand, TRUE);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnTokenName, l_pCellRendererTokenName, "text", Resource_TokenName, NULL);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnTokenValue, l_pCellRendererTokenValue, "text", Resource_TokenValue, NULL);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnTokenExpand, l_pCellRendererTokenExpand, "text", Resource_TokenExpand, NULL);
	gtk_tree_view_column_set_sort_column_id(l_pTreeViewColumnTokenName, Resource_TokenName);
	gtk_tree_view_column_set_sort_column_id(l_pTreeViewColumnTokenValue, Resource_TokenValue);
	gtk_tree_view_column_set_sort_column_id(l_pTreeViewColumnTokenExpand, Resource_TokenExpand);
	gtk_tree_view_column_set_sizing(l_pTreeViewColumnTokenName, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sizing(l_pTreeViewColumnTokenValue, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sizing(l_pTreeViewColumnTokenExpand, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(l_pTreeViewColumnTokenName, TRUE);
	gtk_tree_view_column_set_expand(l_pTreeViewColumnTokenValue, TRUE);
	gtk_tree_view_column_set_expand(l_pTreeViewColumnTokenExpand, TRUE);
	gtk_tree_view_column_set_resizable(l_pTreeViewColumnTokenName, TRUE);
	gtk_tree_view_column_set_resizable(l_pTreeViewColumnTokenValue, TRUE);
	gtk_tree_view_column_set_resizable(l_pTreeViewColumnTokenExpand, TRUE);
	gtk_tree_view_column_set_min_width(l_pTreeViewColumnTokenName, 256);
	gtk_tree_view_column_set_min_width(l_pTreeViewColumnTokenValue, 256);
	gtk_tree_view_column_set_min_width(l_pTreeViewColumnTokenExpand, 256);
	gtk_tree_view_append_column(l_pConfigurationManagerTreeView, l_pTreeViewColumnTokenName);
	gtk_tree_view_append_column(l_pConfigurationManagerTreeView, l_pTreeViewColumnTokenValue);
	gtk_tree_view_append_column(l_pConfigurationManagerTreeView, l_pTreeViewColumnTokenExpand);
	gtk_tree_view_column_set_sort_indicator(l_pTreeViewColumnTokenName, TRUE);

	// Prepares tree model
	CIdentifier l_oTokenIdentifier;
	::GtkTreeStore* l_pConfigurationManagerTreeModel=gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	while((l_oTokenIdentifier=m_rKernelContext.getConfigurationManager().getNextConfigurationTokenIdentifier(l_oTokenIdentifier))!=OV_UndefinedIdentifier)
	{
		::GtkTreeIter l_oGtkIterChild;
		CString l_sTokenName=m_rKernelContext.getConfigurationManager().getConfigurationTokenName(l_oTokenIdentifier);
		CString l_sTokenValue=m_rKernelContext.getConfigurationManager().getConfigurationTokenValue(l_oTokenIdentifier);
		CString l_sTokenExpand=m_rKernelContext.getConfigurationManager().expand(l_sTokenValue);
		gtk_tree_store_append(
				l_pConfigurationManagerTreeModel,
				&l_oGtkIterChild,
				NULL);
		gtk_tree_store_set(
				l_pConfigurationManagerTreeModel,
				&l_oGtkIterChild,
				Resource_TokenName, l_sTokenName.toASCIIString(),
				Resource_TokenValue, l_sTokenValue.toASCIIString(),
				Resource_TokenExpand, l_sTokenExpand.toASCIIString(),
				-1);
	}
	gtk_tree_view_set_model(l_pConfigurationManagerTreeView, GTK_TREE_MODEL(l_pConfigurationManagerTreeModel));
	g_signal_emit_by_name(l_pTreeViewColumnTokenName, "clicked");

	gtk_dialog_run(GTK_DIALOG(l_pConfigurationManager));
	gtk_widget_destroy(l_pConfigurationManager);

	g_object_unref(l_pConfigurationManagerTreeModel);
	g_object_unref(l_pBuilderInterface);
}

void CApplication::testCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "testCB\n";
}

void CApplication::newScenarioCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "newScenarioCB\n";

	CIdentifier l_oScenarioIdentifier;
	if(m_pScenarioManager->createScenario(l_oScenarioIdentifier))
	{
		IScenario& l_rScenario=m_pScenarioManager->getScenario(l_oScenarioIdentifier);
		CInterfacedScenario* l_pInterfacedScenario=new CInterfacedScenario(m_rKernelContext, *this, l_rScenario, l_oScenarioIdentifier, *m_pScenarioNotebook, OVD_GUI_File, OVD_GUI_Settings_File);
		if(l_pInterfacedScenario->m_pDesignerVisualisation != NULL)
		{
			l_pInterfacedScenario->m_pDesignerVisualisation->setDeleteEventCB(&::delete_designer_visualisation_cb, this);
			l_pInterfacedScenario->m_pDesignerVisualisation->newVisualisationWindow("Default window");
		}
		l_pInterfacedScenario->updateScenarioLabel();
		m_vInterfacedScenario.push_back(l_pInterfacedScenario);
		gtk_notebook_set_current_page(m_pScenarioNotebook, gtk_notebook_get_n_pages(m_pScenarioNotebook)-1);
		//this->changeCurrentScenario(gtk_notebook_get_n_pages(m_pScenarioNotebook)-1);
	}
}

void CApplication::openScenarioCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "openScenarioCB\n";

	::GtkFileFilter* l_pFileFilterXML=gtk_file_filter_new();
	::GtkFileFilter* l_pFileFilterAll=gtk_file_filter_new();
	gtk_file_filter_set_name(l_pFileFilterXML, "OpenViBE XML scenario");
	gtk_file_filter_add_pattern(l_pFileFilterXML, "*.xml");
	gtk_file_filter_set_name(l_pFileFilterAll, "All files");
	gtk_file_filter_add_pattern(l_pFileFilterAll, "*");

	::GtkWidget* l_pWidgetDialogOpen=gtk_file_chooser_dialog_new(
			"Select scenario to open...",
			NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterXML);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterAll);
	gtk_file_chooser_set_current_folder(
			GTK_FILE_CHOOSER(l_pWidgetDialogOpen),
			this->getWorkingDirectory().toASCIIString());

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(l_pWidgetDialogOpen),true);

	if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen))==GTK_RESPONSE_ACCEPT)
	{
		//char* l_sFileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		GSList * l_pFile, *l_pList;
		l_pFile = l_pList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		while(l_pFile)
		{
			char* l_sFileName = (char*)l_pFile->data;
			char* l_pBackslash = NULL;
			while((l_pBackslash = ::strchr(l_sFileName, '\\'))!=NULL)
			{
				*l_pBackslash = '/';
			}
			this->openScenario(l_sFileName);
			g_free(l_pFile->data);
			l_pFile=l_pFile->next;
		}
		g_slist_free(l_pList);
	}
	gtk_widget_destroy(l_pWidgetDialogOpen);
}

boolean CApplication::saveScenarioCB(CInterfacedScenario* pScenario)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "saveScenarioCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=pScenario?pScenario:getCurrentInterfacedScenario();
	if(!l_pCurrentInterfacedScenario)
	{
		return false;
	}
	if(!l_pCurrentInterfacedScenario->m_bHasFileName)
	{
		return saveScenarioAsCB(pScenario);
	}
	else
	{
		boolean l_bSuccess=false;
		CMemoryBuffer l_oMemoryBuffer;
		CIdentifier l_oClassIdentifier(l_pCurrentInterfacedScenario->m_oExporterIdentifier);

		CIdentifier l_oExporterIdentifier=m_rKernelContext.getAlgorithmManager().createAlgorithm(l_oClassIdentifier!=OV_UndefinedIdentifier?l_oClassIdentifier:OVP_GD_ClassId_Algorithm_XMLScenarioExporter);
		if(l_oExporterIdentifier!=OV_UndefinedIdentifier)
		{
			IAlgorithmProxy* l_pExporter=&m_rKernelContext.getAlgorithmManager().getAlgorithm(l_oExporterIdentifier);
			if(l_pExporter)
			{
				l_bSuccess = true;

				m_rKernelContext.getLogManager() << LogLevel_Info << "Exporting scenario...\n";

				l_bSuccess &= l_pExporter->initialize();

				TParameterHandler < const IScenario* > ip_pScenario(l_pExporter->getInputParameter(OVTK_Algorithm_ScenarioExporter_InputParameterId_Scenario));
				TParameterHandler < IMemoryBuffer* > op_pMemoryBuffer(l_pExporter->getOutputParameter(OVTK_Algorithm_ScenarioExporter_OutputParameterId_MemoryBuffer));

				ip_pScenario=&l_pCurrentInterfacedScenario->m_rScenario;
				op_pMemoryBuffer=&l_oMemoryBuffer;

				l_bSuccess &= l_pExporter->process();
				l_bSuccess &= l_pExporter->uninitialize();
				m_rKernelContext.getAlgorithmManager().releaseAlgorithm(*l_pExporter);

				l_bSuccess &= (l_oMemoryBuffer.getSize()!=0);

				if(l_bSuccess)
				{
					// Only write if successful so far
					std::ofstream l_oFile(l_pCurrentInterfacedScenario->m_sFileName.c_str(), ios::binary);
					if(l_oFile.good())
					{
						l_oFile.write(reinterpret_cast<const char*>(l_oMemoryBuffer.getDirectPointer()), l_oMemoryBuffer.getSize());
						l_oFile.close();
					} 
					else
					{
						m_rKernelContext.getLogManager() << LogLevel_Error << "Unable to write to file [" << CString(l_pCurrentInterfacedScenario->m_sFileName.c_str()) << "]. Check permissions?\n";
						l_bSuccess = false;
					}
				}

				if(l_bSuccess)
				{
					l_pCurrentInterfacedScenario->m_bHasBeenModified=false;
					l_pCurrentInterfacedScenario->updateScenarioLabel();
				}
			}
		}

		if(!l_bSuccess)
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "Exporting scenario failed...\n";
		}

		return l_bSuccess;
	}
}

OpenViBE::boolean CApplication::saveScenarioAsCB(CInterfacedScenario* pScenario)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "saveScenarioAsCB\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=pScenario?pScenario:getCurrentInterfacedScenario();
	if(!l_pCurrentInterfacedScenario)
	{
		return false;
	}

	boolean l_bResult = false;

	::GtkFileFilter* l_pFileFilterXML=gtk_file_filter_new();
	// ::GtkFileFilter* l_pFileFilterSVG=gtk_file_filter_new();
	::GtkFileFilter* l_pFileFilterAll=gtk_file_filter_new();
	gtk_file_filter_set_name(l_pFileFilterXML, "OpenViBE XML scenario");
	gtk_file_filter_add_pattern(l_pFileFilterXML, "*.xml");
	// gtk_file_filter_set_name(l_pFileFilterSVG, "SVG image");
	// gtk_file_filter_add_pattern(l_pFileFilterSVG, "*.svg");
	gtk_file_filter_set_name(l_pFileFilterAll, "All files");
	gtk_file_filter_add_pattern(l_pFileFilterAll, "*");

	::GtkWidget* l_pWidgetDialogSaveAs=gtk_file_chooser_dialog_new(
			"Select scenario to save...",
			NULL,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
			NULL);

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs), l_pFileFilterXML);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs), l_pFileFilterAll);
	// gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs), true);
	if(l_pCurrentInterfacedScenario->m_bHasFileName)
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs), l_pCurrentInterfacedScenario->m_sFileName.c_str());
	}
	else
	{
		gtk_file_chooser_set_current_folder(
				GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs),
				this->getWorkingDirectory().toASCIIString());
	}
	if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogSaveAs))==GTK_RESPONSE_ACCEPT)
	{
		//ensure file extension is added after filename
		char* l_sTempFileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs));
		char* l_pBackslash = NULL;
		while((l_pBackslash = ::strchr(l_sTempFileName, '\\'))!=NULL)
		{
			*l_pBackslash = '/';
		}

		char l_sFileName[1024];
		::sprintf(l_sFileName, "%s", l_sTempFileName);
		g_free(l_sTempFileName);

		::GtkFileFilter* l_pFileFilter=gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(l_pWidgetDialogSaveAs));
		if(l_pFileFilter == l_pFileFilterXML)
		{
			if(::strlen(l_sFileName) > 4 && ::_strcmpi(l_sFileName+strlen(l_sFileName)-4, ".xml")!=0)
			{
				::strcat(l_sFileName, ".xml");
			}
		}

		const std::string l_sOldFileName = l_pCurrentInterfacedScenario->m_sFileName;
		const boolean l_bOldHasFileName = l_pCurrentInterfacedScenario->m_bHasFileName;

		l_pCurrentInterfacedScenario->m_sFileName=l_sFileName;
		l_pCurrentInterfacedScenario->m_bHasFileName=true;

		l_pCurrentInterfacedScenario->m_oExporterIdentifier=OVP_GD_ClassId_Algorithm_XMLScenarioExporter;

		if(!saveScenarioCB(l_pCurrentInterfacedScenario))
		{
			// restore
			l_pCurrentInterfacedScenario->m_sFileName = l_sOldFileName;
			l_pCurrentInterfacedScenario->m_bHasFileName = l_bOldHasFileName;
		} 
		else
		{
			l_pCurrentInterfacedScenario->m_bHasBeenModified=false;
			l_pCurrentInterfacedScenario->updateScenarioLabel();

			updateWorkingDirectoryToken(l_pCurrentInterfacedScenario->m_oScenarioIdentifier);

			l_bResult = true;
		}

	}
	gtk_widget_destroy(l_pWidgetDialogSaveAs);

	return l_bResult;
}

void CApplication::closeScenarioCB(CInterfacedScenario* pInterfacedScenario)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "closeScenarioCB\n";

	if(!pInterfacedScenario)
	{
		return;
	}
	if(pInterfacedScenario->isLocked())
	{
		::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
		gtk_builder_connect_signals(l_pBuilder, NULL);

		::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_running_scenario"));
		gtk_builder_connect_signals(l_pBuilder, NULL);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
		gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);
		return;
	}
	if(pInterfacedScenario->m_bHasBeenModified)
	{
		gint l_iResponseId;

		::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
		gtk_builder_connect_signals(l_pBuilder, NULL);

		::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_unsaved_scenario"));
		gtk_builder_connect_signals(l_pBuilder, NULL);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
		l_iResponseId=gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);

		switch(l_iResponseId)
		{
			case GTK_RESPONSE_OK:
				this->saveScenarioCB(pInterfacedScenario);
				if(pInterfacedScenario->m_bHasBeenModified)
				{
					return;
				}
				break;
			case GTK_RESPONSE_DELETE_EVENT:
			case GTK_RESPONSE_CANCEL:
				return;
			default:
				break;
		}
	}

	vector<CInterfacedScenario*>::iterator i=m_vInterfacedScenario.begin();
	while(i!=m_vInterfacedScenario.end() && *i!=pInterfacedScenario) i++;
	if(i!=m_vInterfacedScenario.end())
	{
		// We need to erase the scenario from the list first, because deleting the scenario will launch a "switch-page" 
		// callback accessing this array with the identifier of the deleted scenario (if its not the last one) -> boom.
		m_vInterfacedScenario.erase(i);
		CIdentifier l_oScenarioIdentifier=pInterfacedScenario->m_oScenarioIdentifier;
		delete pInterfacedScenario;
		m_pScenarioManager->releaseScenario(l_oScenarioIdentifier);
		this->removeScenarioDirectoryToken(l_oScenarioIdentifier);
		//when closing last open scenario, no "switch-page" event is triggered so we manually handle this case
		if(m_vInterfacedScenario.empty() == true)
		{
			//This is the last, we need to reset the volatile scenario dir
			resetVolatileScenarioDirectoryToken();
			changeCurrentScenario(-1);
		}
	}
}

void CApplication::deleteDesignerVisualisationCB()
{
	//untoggle window manager button when its associated dialog is closed
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), FALSE);

	CInterfacedScenario* l_pCurrentInterfacedScenario = getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->snapshotCB();
	}
}

void CApplication::toggleDesignerVisualisationCB()
{
	CInterfacedScenario* l_pCurrentInterfacedScenario = getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario != NULL && l_pCurrentInterfacedScenario->isLocked() == false)
	{
		uint32 l_ui32Index=(uint32)gtk_notebook_get_current_page(m_pScenarioNotebook);
		if(l_ui32Index<m_vInterfacedScenario.size())
		{
			m_vInterfacedScenario[l_ui32Index]->toggleDesignerVisualisation();
		}
	}
}

void CApplication::aboutOpenViBECB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::aboutOpenViBECB\n";
	::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
	gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
	gtk_builder_connect_signals(l_pBuilder, NULL);

	::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "about"));
	gtk_builder_connect_signals(l_pBuilder, NULL);
	gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
	gtk_dialog_run(GTK_DIALOG(l_pDialog));
	gtk_widget_destroy(l_pDialog);
	g_object_unref(l_pBuilder);
}

void CApplication::aboutScenarioCB(CInterfacedScenario* pScenario)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::aboutScenarioCB\n";
	if(pScenario && !pScenario->isLocked())
	{
		pScenario->contextMenuScenarioAboutCB();
	}
}

void CApplication::aboutLinkClickedCB(const gchar *url)
{
	if(!url) 
	{
		return;
	}
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::aboutLinkClickedCB\n";
	CString l_sCommand = m_rKernelContext.getConfigurationManager().expand("${Designer_WebBrowserCommand} " + OpenViBE::CString(url));
	int l_iResult = system(l_sCommand.toASCIIString());
	if(l_iResult<0)
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Could not launch command " << l_sCommand << "\n";
	}
}

//Increase the zoom of the current scenario
void CApplication::zoomInCB(void)
{
	gtk_spin_button_set_value(m_pZoomSpinner, ov_round(getCurrentInterfacedScenario()->getScale()*100.0) + 5);
}
	
//Decrease the zoom of the current scenario
void CApplication::zoomOutCB(void)
{
	gtk_spin_button_set_value(m_pZoomSpinner, ov_round(getCurrentInterfacedScenario()->getScale()*100.0) - 5);
}

void CApplication::spinnerZoomChangedCB(uint32 scalePercentage)
{
	if(getCurrentInterfacedScenario() != NULL)
	{
		getCurrentInterfacedScenario()->setScale((static_cast<float64>(scalePercentage))/100.0);
	}
}

void CApplication::windowItemToggledCB(::GtkCheckMenuItem* pCheckMenuItem)
{
	uint32 l_ui32Index = 0;
	// Look for item corresponding index
	for(unsigned int i=0; i<this->getCurrentInterfacedScenario()->m_vCheckItems.size(); i++)
	{
		if (this->getCurrentInterfacedScenario()->m_vCheckItems[i]==GTK_WIDGET(pCheckMenuItem))
		{
			l_ui32Index = i;
		}
	}

	if (gtk_check_menu_item_get_active(pCheckMenuItem))
	{
		this->getCurrentInterfacedScenario()->onItemToggledOn(l_ui32Index);
	}
	else
	{
		this->getCurrentInterfacedScenario()->onItemToggledOff(l_ui32Index);
	}
}

void CApplication::toggleOnWindowItem(uint32 ui32Index, int32 i32PageIndex)
{

       //block callback to prevent from showing windows twice
        g_signal_handlers_block_by_func(G_OBJECT(m_vInterfacedScenario[i32PageIndex]->m_vCheckItems[ui32Index]), reinterpret_cast<gpointer>(G_CALLBACK(window_menu_check_item_toggled_cb)), this);

        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(m_vInterfacedScenario[i32PageIndex]->m_vCheckItems[ui32Index]),true);

       //unblock
        g_signal_handlers_unblock_by_func(G_OBJECT(m_vInterfacedScenario[i32PageIndex]->m_vCheckItems[ui32Index]), reinterpret_cast<gpointer>(G_CALLBACK(window_menu_check_item_toggled_cb)), this);

}

void CApplication::toggleOffWindowItem(uint32 ui32Index)
{
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(this->getCurrentInterfacedScenario()->m_vCheckItems[ui32Index]),false);
}

void CApplication::browseDocumentationCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::browseDocumentationCB\n";
	const CString l_sCommand = m_rKernelContext.getConfigurationManager().expand("${Designer_WebBrowserCommand} \"${Designer_WebBrowserOpenViBEHomepage}/documentation-index\"");
	const int l_iResult = system(l_sCommand.toASCIIString());

	if(l_iResult<0)
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Could not launch command " << l_sCommand << "\n";
	}
}

void CApplication::addCommentCB(
		CInterfacedScenario* pScenario)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "CApplication::addCommentCB\n";
	if(pScenario && !pScenario->isLocked())
	{
		pScenario->addCommentCB();
	}
}

IPlayer* CApplication::getPlayer(void)
{
	CInterfacedScenario* l_pCurrentInterfacedScenario=getCurrentInterfacedScenario();
	return (l_pCurrentInterfacedScenario?l_pCurrentInterfacedScenario->m_pPlayer:NULL);
}

OpenViBE::boolean CApplication::createPlayer(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "createPlayer\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario && !l_pCurrentInterfacedScenario->m_pPlayer)
	{
		// create a snapshot so settings override does not modify the scenario !
		l_pCurrentInterfacedScenario->snapshotCB(false);

		// generate player windows
		l_pCurrentInterfacedScenario->createPlayerVisualisation();

		m_rKernelContext.getPlayerManager().createPlayer(l_pCurrentInterfacedScenario->m_oPlayerIdentifier);
		CIdentifier l_oScenarioIdentifier=l_pCurrentInterfacedScenario->m_oScenarioIdentifier;
		CIdentifier l_oPlayerIdentifier=l_pCurrentInterfacedScenario->m_oPlayerIdentifier;
		l_pCurrentInterfacedScenario->m_pPlayer=&m_rKernelContext.getPlayerManager().getPlayer(l_oPlayerIdentifier);
		l_pCurrentInterfacedScenario->m_pPlayer->setScenario(l_oScenarioIdentifier);

		EPlayerReturnCode l_eCode = l_pCurrentInterfacedScenario->m_pPlayer->initialize();
		if(l_eCode == PlayerReturnCode_Failed)
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "Failed to initialize player\n";
			l_pCurrentInterfacedScenario->m_oPlayerIdentifier = OV_UndefinedIdentifier;
			l_pCurrentInterfacedScenario->m_pPlayer=NULL;
			m_rKernelContext.getPlayerManager().releasePlayer(l_oPlayerIdentifier);
			return false;
		}
		else if(l_eCode == PlayerReturnCode_BoxInitializationFailed){
			gint res = 1;
			if(gtk_toggle_button_get_active(m_pInitAlert) )
			{
				if(!this->isNoGuiActive())
				{
					::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
					gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
					gtk_builder_connect_signals(l_pBuilder, NULL);
					::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_init_error_popup"));

					res = gtk_dialog_run(GTK_DIALOG(l_pDialog));

					gtk_widget_destroy(l_pDialog);
					g_object_unref(l_pBuilder);
				}
				else{
					m_rKernelContext.getLogManager() << LogLevel_Error << "Initilization of scenario didn't work properly."
														" Aborting the execution. (To prevent this, deactivate the warning on initialization option)\n" ;
					res=0;//No matter what happen, if the user ask for a warning on initilization we consider that we don't run the scenario
				}
			}
			if(res == 0)
			{
				releasePlayer();
				return false;
			}
		}
		l_pCurrentInterfacedScenario->m_ui64LastLoopTime=System::Time::zgetTime();

		//set up idle function
		__g_idle_add__(idle_scenario_loop, l_pCurrentInterfacedScenario);

		// redraws scenario
		l_pCurrentInterfacedScenario->forceRedraw();
	}
	return true;
}

boolean CApplication::isPlayerExisting(void)
{
	CInterfacedScenario* l_pCurrentInterfacedScenario=getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario && !l_pCurrentInterfacedScenario->m_pPlayer){
		return false;
	}
	return true;
}

void CApplication::releasePlayer(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "releasePlayer\n";

	CInterfacedScenario* l_pCurrentInterfacedScenario=getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario && l_pCurrentInterfacedScenario->m_pPlayer)
	{
		// removes idle function
		g_idle_remove_by_data(l_pCurrentInterfacedScenario);

		l_pCurrentInterfacedScenario->m_pPlayer->uninitialize();

		//must delete the CBoxCOnfiguration dialog of the mod UI boxes here before the undoCB
		l_pCurrentInterfacedScenario->deleteModifiableSettingsBoxes();

		m_rKernelContext.getPlayerManager().releasePlayer(l_pCurrentInterfacedScenario->m_oPlayerIdentifier);

		l_pCurrentInterfacedScenario->m_oPlayerIdentifier=OV_UndefinedIdentifier;
		l_pCurrentInterfacedScenario->m_pPlayer=NULL;

		// restore the snapshot so settings override does not modify the scenario !

		// A. commenting this line allow modified (by UI) settings to be saved (however, the scenario is not marked as changed)
		//should already be commented in wip-all-designer branch
		// B. commenting this line make centerOnBox still valid after stop
		//l_pCurrentInterfacedScenario->undoCB(false);

		// destroy player windows
		l_pCurrentInterfacedScenario->releasePlayerVisualisation();

		// destroy window menu
		destroyWindowMenu();

		// redraws scenario
		l_pCurrentInterfacedScenario->forceRedraw();
	}
}

void CApplication::destroyWindowMenu(void)
{
	for(unsigned int i=0; i<this->getCurrentInterfacedScenario()->m_vCheckItems.size(); i++)
	{
		gtk_widget_destroy(this->getCurrentInterfacedScenario()->m_vCheckItems[i]);
	}
}

void CApplication::stopScenarioCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "stopScenarioCB\n";

	if(this->getCurrentInterfacedScenario()->m_ePlayerStatus == PlayerStatus_Play || this->getCurrentInterfacedScenario()->m_ePlayerStatus == PlayerStatus_Pause || this->getCurrentInterfacedScenario()->m_ePlayerStatus == PlayerStatus_Forward)
	{
		this->getPlayer()->stop();
		this->getCurrentInterfacedScenario()->m_ePlayerStatus=this->getPlayer()->getStatus();
		this->getCurrentInterfacedScenario()->updateScenarioLabel();
		this->releasePlayer();

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),          false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),    true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),          true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),       true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_comment")),       true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_aboutscenario")), true);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

		if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-toggle_button_replay"))))
		{
			switch(m_eReplayMode)
			{
			case EReplayMode_Play: playScenarioCB(); break;
			case EReplayMode_Forward: forwardScenarioCB(); break;
			case EReplayMode_None:
				// nop
				break;
			default:
				m_rKernelContext.getLogManager() << LogLevel_Error << "Unsupported replaymode " << m_eReplayMode << "\n";
				break;
			}
		}

		m_eReplayMode = EReplayMode_None;
		
		if(this->hasRunningScenario() == false) // if stopping last running scenario, hide window menu
		{
		    gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_window")), false);
		}

	}
}

void CApplication::pauseScenarioCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "pauseScenarioCB\n";

	this->createPlayer();
	this->getPlayer()->pause();
	this->getCurrentInterfacedScenario()->m_ePlayerStatus=this->getPlayer()->getStatus();
	this->getCurrentInterfacedScenario()->updateScenarioLabel();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),    true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),       true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_comment")),       false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_aboutscenario")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);
}

void CApplication::nextScenarioCB(void)
{
	boolean l_bIsAlreadyStarted = false;
	m_rKernelContext.getLogManager() << LogLevel_Trace << "nextScenarioCB\n";

	if(!this->isPlayerExisting())
	{
		if(!this->createPlayer())
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "CreatePlayer failed\n";
			return;
		}
	}
	else
	{
		l_bIsAlreadyStarted = true;
	}
	this->getPlayer()->step();
	this->getCurrentInterfacedScenario()->m_ePlayerStatus=this->getPlayer()->getStatus();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),    true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),       true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_comment")),       false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_aboutscenario")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

	gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_window")), true);
	if(!l_bIsAlreadyStarted)
	{
		//Add top level window item in menu_window
		std::vector < ::GtkWindow* > l_vTopLevelWindows = this->getCurrentInterfacedScenario()->m_pPlayerVisualisation->getTopLevelWindows();
		this->getCurrentInterfacedScenario()->m_vCheckItems.resize(l_vTopLevelWindows.size());
		for(unsigned int i=0; i<l_vTopLevelWindows.size(); i++)
		{
			const gchar* l_cTitle = gtk_window_get_title(l_vTopLevelWindows[i]);
			this->getCurrentInterfacedScenario()->m_vCheckItems[i] = gtk_check_menu_item_new_with_label (l_cTitle);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(this->getCurrentInterfacedScenario()->m_vCheckItems[i]), true);
			gtk_menu_append(GTK_MENU(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_show_content")),this->getCurrentInterfacedScenario()->m_vCheckItems[i]);
			gtk_widget_show(this->getCurrentInterfacedScenario()->m_vCheckItems[i]);

			g_signal_connect(G_OBJECT(this->getCurrentInterfacedScenario()->m_vCheckItems[i]), "toggled", G_CALLBACK(window_menu_check_item_toggled_cb), this);

		}
	}
}

boolean CApplication::playScenarioCB(void)
{
	boolean l_bIsAlreadyStarted = false;
	m_rKernelContext.getLogManager() << LogLevel_Trace << "playScenarioCB\n";

	if(!this->isPlayerExisting())
	{
		if(!this->createPlayer())
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "CreatePlayer failed\n";
			return false;
		}
	}
	else
	{
		l_bIsAlreadyStarted = true;
	}
	this->getPlayer()->play();
	this->getCurrentInterfacedScenario()->m_ePlayerStatus=this->getPlayer()->getStatus();
	this->getCurrentInterfacedScenario()->updateScenarioLabel();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),    true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),       true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_comment")),       false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_aboutscenario")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PAUSE);

	gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_window")), true);

	if(!l_bIsAlreadyStarted)
	{
		//Add top level window item in menu_window
		std::vector < ::GtkWindow* > l_vTopLevelWindows = this->getCurrentInterfacedScenario()->m_pPlayerVisualisation->getTopLevelWindows();
		this->getCurrentInterfacedScenario()->m_vCheckItems.resize(l_vTopLevelWindows.size());
		for(unsigned int i=0; i<l_vTopLevelWindows.size(); i++)
		{
			const gchar* l_cTitle = gtk_window_get_title(l_vTopLevelWindows[i]);
			this->getCurrentInterfacedScenario()->m_vCheckItems[i] = gtk_check_menu_item_new_with_label (l_cTitle);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(this->getCurrentInterfacedScenario()->m_vCheckItems[i]), true);
			gtk_menu_append(GTK_MENU(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_show_content")),this->getCurrentInterfacedScenario()->m_vCheckItems[i]);
			gtk_widget_show(this->getCurrentInterfacedScenario()->m_vCheckItems[i]);

			g_signal_connect(G_OBJECT(this->getCurrentInterfacedScenario()->m_vCheckItems[i]), "toggled", G_CALLBACK(window_menu_check_item_toggled_cb), this);

		}
	}
	return true;
}

boolean CApplication::forwardScenarioCB(void)
{
	boolean l_bIsAlreadyStarted = false;
	m_rKernelContext.getLogManager() << LogLevel_Trace << "forwardScenarioCB\n";

	if(!this->isPlayerExisting())
	{
		if(!this->createPlayer())
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "CreatePlayer failed\n";
			return false;
		}
	}
	else
	{
		l_bIsAlreadyStarted = true;
	}

	this->getPlayer()->forward();
	this->getCurrentInterfacedScenario()->m_ePlayerStatus=this->getPlayer()->getStatus();
	this->getCurrentInterfacedScenario()->updateScenarioLabel();

	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")),    true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),          true);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),       false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_comment")),       false);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_aboutscenario")), false);
	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

	gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_window")), true);
	if(!l_bIsAlreadyStarted)
	{
		//Add top level window item in menu_window
		std::vector < ::GtkWindow* > l_vTopLevelWindows = this->getCurrentInterfacedScenario()->m_pPlayerVisualisation->getTopLevelWindows();
		this->getCurrentInterfacedScenario()->m_vCheckItems.resize(l_vTopLevelWindows.size());
		for(unsigned int i=0; i<l_vTopLevelWindows.size(); i++)
		{
			const gchar* l_cTitle = gtk_window_get_title(l_vTopLevelWindows[i]);
			this->getCurrentInterfacedScenario()->m_vCheckItems[i] = gtk_check_menu_item_new_with_label (l_cTitle);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(this->getCurrentInterfacedScenario()->m_vCheckItems[i]), true);
			gtk_menu_append(GTK_MENU(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_show_content")),this->getCurrentInterfacedScenario()->m_vCheckItems[i]);
			gtk_widget_show(this->getCurrentInterfacedScenario()->m_vCheckItems[i]);

			g_signal_connect(G_OBJECT(this->getCurrentInterfacedScenario()->m_vCheckItems[i]), "toggled", G_CALLBACK(window_menu_check_item_toggled_cb), this);

		}
	}
	return true;
}

void CApplication::keyPressEventCB(::GtkWidget* pWidget, ::GdkEventKey* pEvent)
{
	//The shortcuts respect the order in the toolbar

	// F7 :play/pause
	if(pEvent->keyval==GDK_F7)
	{
		if(this->getCurrentInterfacedScenario()->m_ePlayerStatus == PlayerStatus_Play)
		{
			this->pauseScenarioCB();
		}
		else
		{
			this->playScenarioCB();
		}
	}
	// F6 : step
	if(pEvent->keyval==GDK_F6)
	{
		this->nextScenarioCB();
	}
	// F8 :fastforward
	if(pEvent->keyval==GDK_F8)
	{
		this->forwardScenarioCB();
	}
	// F5 : stop
	if(pEvent->keyval==GDK_F5)
	{
		this->stopScenarioCB();
	}
}

boolean CApplication::quitApplicationCB(void)
{
	std::vector < CInterfacedScenario* >::iterator it;

	CIdentifier l_oIdentifier;
	m_rKernelContext.getLogManager() << LogLevel_Trace << "quitApplicationCB\n";

	// can't quit while scenarios are running
	if(this->hasRunningScenario() == true)
	{
		::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
		gtk_builder_connect_signals(l_pBuilder, NULL);

		::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_running_scenario_global"));
		gtk_builder_connect_signals(l_pBuilder, NULL);
		// gtk_dialog_set_response_sensitive(GTK_DIALOG(l_pDialog), GTK_RESPONSE_CLOSE, true);
		gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);

		// prevent Gtk from handling delete_event and killing app
		return false;
	}

	// can't quit while scenarios are unsaved
	if(this->hasUnsavedScenario() == true)
	{
		gint l_iResponseId;

		::GtkBuilder* l_pBuilder=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "about", NULL);
		gtk_builder_add_from_file(l_pBuilder, OVD_GUI_File, NULL);
		gtk_builder_connect_signals(l_pBuilder, NULL);

		::GtkWidget* l_pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilder, "dialog_unsaved_scenario_global"));
		gtk_builder_connect_signals(l_pBuilder, NULL);
		l_iResponseId=gtk_dialog_run(GTK_DIALOG(l_pDialog));
		gtk_widget_destroy(l_pDialog);
		g_object_unref(l_pBuilder);

		switch(l_iResponseId)
		{
			case GTK_RESPONSE_OK:
				for(std::vector < CInterfacedScenario* >::iterator i=m_vInterfacedScenario.begin(); i!=m_vInterfacedScenario.end(); i++)
				{
					this->saveScenarioCB(*i);
				}
				if(this->hasUnsavedScenario())
				{
					// prevent Gtk from handling delete_event and killing app
					return false;
				}
				break;
			case GTK_RESPONSE_DELETE_EVENT:
			case GTK_RESPONSE_CANCEL:
				// prevent Gtk from handling delete_event and killing app
				return false;
			default:
				break;
		}
	}

	// Switch to quitting mode
	m_bIsQuitting=true;

	// Saves opened scenarios
	if(!(m_eCommandLineFlags&CommandLineFlag_NoManageSession))
	{
		OpenViBE::CString l_sAppConfigFile = m_rKernelContext.getConfigurationManager().expand("${CustomConfigurationApplication}");

		FILE* l_pFile=::fopen(l_sAppConfigFile.toASCIIString(), "wt");
		if(l_pFile)
		{
			unsigned int i=1;
			::fprintf(l_pFile, "# This file is generated\n");
			::fprintf(l_pFile, "# Do not modify\n");
			::fprintf(l_pFile, "\n");
			::fprintf(l_pFile, "Designer_ShowUnstable = %s\n", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-show_unstable")))?"True":"False");
			::fprintf(l_pFile, "# Last files opened in the designer\n");
			for(it=m_vInterfacedScenario.begin(); it!=m_vInterfacedScenario.end(); it++)
			{
				if((*it)->m_sFileName != "")
				{
					::fprintf(l_pFile, "Designer_LastScenarioFilename_%03i = %s\n", i, (*it)->m_sFileName.c_str());
					i++;
				}
			}
			::fprintf(l_pFile, "Designer_CurrentScenarioPage = %d\n", m_i32CurrentScenarioPage);

			::fprintf(l_pFile, "Designer_DebugCanal = %d\n",getLogState("openvibe-messages_tb_debug"));
			::fprintf(l_pFile, "Designer_BenchCanal = %d\n",getLogState("openvibe-messages_tb_bench"));
			::fprintf(l_pFile, "Designer_TraceCanal = %d\n",getLogState("openvibe-messages_tb_trace"));
			::fprintf(l_pFile, "Designer_InfoCanal = %d\n",getLogState("openvibe-messages_tb_info"));
			::fprintf(l_pFile, "Designer_WarningCanal = %d\n",getLogState("openvibe-messages_tb_warning"));
			::fprintf(l_pFile, "Designer_ImportantWarningCanal = %d\n",getLogState("openvibe-messages_tb_impwarning"));
			::fprintf(l_pFile, "Designer_ErrorCanal = %d\n",getLogState("openvibe-messages_tb_error"));
			::fprintf(l_pFile, "Designer_FatalCanal = %d\n",getLogState("openvibe-messages_tb_fatal"));

			::fprintf(l_pFile, "Designer_LogExpanderStatus = %s\n",
					  gtk_expander_get_expanded(GTK_EXPANDER(gtk_builder_get_object(m_pBuilderInterface, "openvibe-expander_messages")))?"True":"False");

			::fprintf(l_pFile, "Designer_HorizontalContainerPosition = %d\n",gtk_paned_get_position(GTK_PANED(gtk_builder_get_object(m_pBuilderInterface, "openvibe-horizontal_container"))));
			gint l_iWidth=0, l_iHeight = 0;
			gtk_window_get_size (GTK_WINDOW(this->m_pMainWindow) ,&l_iWidth, &l_iHeight);
			::fprintf(l_pFile, "Designer_WindowWidth = %d\n",l_iWidth);
			::fprintf(l_pFile, "Designer_WindowHeight = %d\n",l_iHeight);
			::fclose(l_pFile);
		}
		else 
		{
			m_rKernelContext.getLogManager() << LogLevel_Error << "Error writing to '" << l_sAppConfigFile << "'\n";
		}
	}

	// Clears all existing interfaced scenarios
	for(it=m_vInterfacedScenario.begin(); it!=m_vInterfacedScenario.end(); it++)
	{
		delete *it;
	}

	// Clears all existing scenarios
	vector < CIdentifier > l_vScenarioIdentifiers;
	while((l_oIdentifier=m_rKernelContext.getScenarioManager().getNextScenarioIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
	{
		l_vScenarioIdentifiers.push_back(l_oIdentifier);
	}
	for(vector < CIdentifier > ::iterator i=l_vScenarioIdentifiers.begin(); i!=l_vScenarioIdentifiers.end(); i++)
	{
		m_rKernelContext.getScenarioManager().releaseScenario(*i);
	}

	// release the log manager and free the memory
	if(m_pLogListenerDesigner) 
	{
		m_rKernelContext.getLogManager().removeListener( m_pLogListenerDesigner );
		delete m_pLogListenerDesigner;
		m_pLogListenerDesigner = NULL;
	}

	// OK to kill app
	return true;
}

uint32 CApplication::getLogState(const char* sButtonName)
{
	if(!gtk_widget_get_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, sButtonName))))
	{
		return Log_NotAvailable;
	}
	else if(!gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, sButtonName))))
	{
		return Log_AvailableNotActivate;
	}
	return Log_AvailableActivate;
}

void CApplication::logLevelCB(void)
{
	// Loads log level dialog
	::GtkBuilder* l_pBuilderInterface=gtk_builder_new(); // glade_xml_new(OVD_GUI_File, "loglevel", NULL);
	gtk_builder_add_from_file(l_pBuilderInterface, OVD_GUI_File, NULL);
	gtk_builder_connect_signals(l_pBuilderInterface, NULL);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_fatal")),             m_rKernelContext.getLogManager().isActive(LogLevel_Fatal));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_error")),             m_rKernelContext.getLogManager().isActive(LogLevel_Error));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_important_warning")), m_rKernelContext.getLogManager().isActive(LogLevel_ImportantWarning));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_warning")),           m_rKernelContext.getLogManager().isActive(LogLevel_Warning));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_info")),              m_rKernelContext.getLogManager().isActive(LogLevel_Info));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_trace")),             m_rKernelContext.getLogManager().isActive(LogLevel_Trace));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_benchmark")),         m_rKernelContext.getLogManager().isActive(LogLevel_Benchmark));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_debug")),             m_rKernelContext.getLogManager().isActive(LogLevel_Debug));

	::GtkDialog* l_pLogLevelDialog=GTK_DIALOG(gtk_builder_get_object(l_pBuilderInterface, "loglevel"));
	gint l_iResult=gtk_dialog_run(l_pLogLevelDialog);
	if(l_iResult==GTK_RESPONSE_APPLY)
	{
		m_rKernelContext.getLogManager().activate(LogLevel_Fatal,            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_fatal")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Error,            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_error")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_ImportantWarning, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_important_warning")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Warning,          gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_warning")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Info,             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_info")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Trace,            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_trace")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Benchmark,        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_benchmark")))?true:false);
		m_rKernelContext.getLogManager().activate(LogLevel_Debug,            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "loglevel-checkbutton_loglevel_debug")))?true:false);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_debug")), m_rKernelContext.getLogManager().isActive(LogLevel_Debug));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_bench")), m_rKernelContext.getLogManager().isActive(LogLevel_Benchmark));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_trace")), m_rKernelContext.getLogManager().isActive(LogLevel_Trace));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_info")), m_rKernelContext.getLogManager().isActive(LogLevel_Info));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_warning")), m_rKernelContext.getLogManager().isActive(LogLevel_Warning));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_impwarning")), m_rKernelContext.getLogManager().isActive(LogLevel_ImportantWarning));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_error")), m_rKernelContext.getLogManager().isActive(LogLevel_Error));
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-messages_tb_fatal")), m_rKernelContext.getLogManager().isActive(LogLevel_Fatal));
	}

	gtk_widget_destroy(GTK_WIDGET(l_pLogLevelDialog));
	g_object_unref(l_pBuilderInterface);
}

void CApplication::CPUUsageCB(void)
{
	CInterfacedScenario* l_pCurrentInterfacedScenario=getCurrentInterfacedScenario();
	if(l_pCurrentInterfacedScenario)
	{
		l_pCurrentInterfacedScenario->m_bDebugCPUUsage=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")))?true:false);
		l_pCurrentInterfacedScenario->forceRedraw();
	}
}

void CApplication::changeCurrentScenario(int32 i32PageIndex)
{
	if(m_bIsQuitting) return;

	//hide window manager of previously active scenario, if any
	int i = gtk_notebook_get_current_page(m_pScenarioNotebook);
	if(i >= 0 && i < (int)m_vInterfacedScenario.size())
	{
		m_vInterfacedScenario[i]->hideCurrentVisualisation();
	}

	//closing last open scenario
	if(i32PageIndex == -1)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),       false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),       false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),    false);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), false);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), false);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		//toggle off window manager button
		GtkWidget* l_pWindowManagerButton=GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(l_pWindowManagerButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(l_pWindowManagerButton), false);
		g_signal_connect(l_pWindowManagerButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);
	
		m_i32CurrentScenarioPage = -1;
		this->spinnerZoomChangedCB(100);
	}
	//switching to an existing scenario
	else if(i32PageIndex<(int32)m_vInterfacedScenario.size())
	{
		CInterfacedScenario* l_pCurrentInterfacedScenario=m_vInterfacedScenario[i32PageIndex];
		EPlayerStatus l_ePlayerStatus=(l_pCurrentInterfacedScenario->m_pPlayer?l_pCurrentInterfacedScenario->m_pPlayer->getStatus():PlayerStatus_Stop);

		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),       l_ePlayerStatus!=PlayerStatus_Stop);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),       true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),    l_ePlayerStatus!=PlayerStatus_Forward);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), (l_ePlayerStatus==PlayerStatus_Stop || l_ePlayerStatus==PlayerStatus_Pause || l_ePlayerStatus==PlayerStatus_Uninitialized) ? GTK_STOCK_MEDIA_PLAY : GTK_STOCK_MEDIA_PAUSE);

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), l_ePlayerStatus==PlayerStatus_Stop);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_comment")),       l_ePlayerStatus==PlayerStatus_Stop);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_aboutscenario")), l_ePlayerStatus==PlayerStatus_Stop);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), l_pCurrentInterfacedScenario->m_bDebugCPUUsage);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		// gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_save")), l_pCurrentInterfacedScenario->m_bHasFileName && l_pCurrentInterfacedScenario->m_bHasBeenModified);
		// gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-menu_save")),   l_pCurrentInterfacedScenario->m_bHasFileName && l_pCurrentInterfacedScenario->m_bHasBeenModified);

		//don't show window manager if in offline mode and it is toggled off
		if(l_ePlayerStatus==PlayerStatus_Stop && m_vInterfacedScenario[i32PageIndex]->isDesignerVisualisationToggled() == false)
		{
			m_vInterfacedScenario[i32PageIndex]->hideCurrentVisualisation();
		}
		else
		{
			m_vInterfacedScenario[i32PageIndex]->showCurrentVisualisation();
			m_vInterfacedScenario[i32PageIndex]->showWindowMenu();
		}

		//update window manager button state
		GtkWidget* l_pWindowManagerButton=GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(l_pWindowManagerButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(l_pWindowManagerButton), m_vInterfacedScenario[i32PageIndex]->isDesignerVisualisationToggled() ? true : false);
		g_signal_connect(l_pWindowManagerButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);
		
		m_i32CurrentScenarioPage = i32PageIndex;
		gtk_spin_button_set_value(m_pZoomSpinner, ov_round(m_vInterfacedScenario[m_i32CurrentScenarioPage]->getScale()*100.0));
		updateWorkingDirectoryToken(m_vInterfacedScenario[m_i32CurrentScenarioPage]->m_oScenarioIdentifier);
	}
	//first scenario is created (or a scenario is opened and replaces first unnamed unmodified scenario)
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_stop")),       false);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_next")),       true);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_forward")),    true);
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_play_pause")), GTK_STOCK_MEDIA_PLAY);

		g_signal_handlers_disconnect_by_func(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), G_CALLBACK2(cpu_usage_cb), this);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager")), true);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), false);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pBuilderInterface, "openvibe-togglebutton_cpu_usage")), "toggled", G_CALLBACK(cpu_usage_cb), this);

		//toggle off window manager button
		GtkWidget* l_pWindowManagerButton=GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "openvibe-button_windowmanager"));
		g_signal_handlers_disconnect_by_func(l_pWindowManagerButton, G_CALLBACK2(button_toggle_window_manager_cb), this);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(l_pWindowManagerButton), false);
		g_signal_connect(l_pWindowManagerButton, "toggled", G_CALLBACK(button_toggle_window_manager_cb), this);
		
		m_i32CurrentScenarioPage = 0;
		gtk_spin_button_set_value(m_pZoomSpinner, 100);
	}
}

void CApplication::reorderCurrentScenario(OpenViBE::uint32 i32NewPageIndex)
{
	CInterfacedScenario* temp = m_vInterfacedScenario[m_i32CurrentScenarioPage];
	m_vInterfacedScenario.erase(m_vInterfacedScenario.begin() + m_i32CurrentScenarioPage);
	m_vInterfacedScenario.insert(m_vInterfacedScenario.begin() + i32NewPageIndex, temp);
	this->changeCurrentScenario(i32NewPageIndex);
}

void CApplication::logLevelRestore(GObject* ToolButton, OpenViBE::Kernel::ELogLevel level, const char* configName)
{
	uint64 l_ui64Active;
	l_ui64Active = m_rKernelContext.getConfigurationManager().expandAsUInteger(configName, m_rKernelContext.getLogManager().isActive(level)?Log_AvailableActivate:Log_NotAvailable);
	//At the beginning all buttons are sensitive and not active
	switch(l_ui64Active)
	{
	case Log_NotAvailable:
		gtk_widget_set_sensitive(GTK_WIDGET(ToolButton), false);
		m_rKernelContext.getLogManager().activate(level, false);
		break;

	case Log_AvailableActivate:
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(ToolButton), true);
		m_rKernelContext.getLogManager().activate(level, true);
		break;

	case Log_AvailableNotActivate:
		m_rKernelContext.getLogManager().activate(level, true);
		break;

	default:
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Unknown log state " << l_ui64Active << "\n";
		m_rKernelContext.getLogManager().activate(level, false);
		gtk_widget_set_sensitive(GTK_WIDGET(ToolButton), false);
		break;
	}
}

boolean CApplication::isLogAreaClicked()
{
	if(m_pTextView!=NULL)
	{
		return gtk_widget_is_focus(GTK_WIDGET(m_pTextView))!=FALSE;
	}
	else
		return false;
}

boolean CApplication::isNoGuiActive()
{
	return ( (m_eCommandLineFlags & CommandLineFlag_NoGui) ? true : false);
}
