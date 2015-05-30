#include "ovdCInterfacedScenario.h"
#include "ovdCApplication.h"
#include "ovdCBoxProxy.h"
#include "ovdCCommentProxy.h"
#include "ovdCLinkProxy.h"
#include "ovdCConnectorEditor.h"
#include "ovdCInterfacedObject.h"
#include "ovdTAttributeHandler.h"
#include "ovdCDesignerVisualisation.h"
#include "ovdCPlayerVisualisation.h"
#include "ovdCRenameDialog.h"
#include "ovdCAboutPluginDialog.h"
#include "ovdCAboutScenarioDialog.h"
#include "ovdCSettingEditorDialog.h"
#include "ovdCCommentEditorDialog.h"

#include <iostream>

#include <gdk/gdkkeysyms.h>

#include <cstdlib>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEDesigner;
using namespace std;

namespace{
	const int32 gridSize = 16;
}


// round is defined in <cmath> on c++11
inline int ov_round(double dbl)
{ return dbl >= 0.0 ? (int)(dbl + 0.5) : ((dbl - (double)(int)dbl) <= -0.5 ? (int)dbl : (int)(dbl - 0.5));
}

// Decodes a number in %XX to one letter
char decodeNumber(const char* sString) 
{
	const char l_char0 = sString[0];
	const char l_char1 = sString[1];
	char l_cOut;

	l_cOut  = (l_char0 >= 'A' ? ( (l_char0 & 0xDF) - 'A' ) + 10 : (l_char0 - '0') ) * 16;
	l_cOut += (l_char1 >= 'A' ? ( (l_char1 & 0xDF) - 'A' ) + 10 : (l_char1 - '0') );

	return l_cOut;
}

// Decodes url from all its %XX strings
std::string decodeUrl(const std::string& sUrl)
{
	std::string l_sOut = sUrl;

	for(uint32 i=0;i<l_sOut.length();i++)
	{
		if(l_sOut[i]=='%' && i<l_sOut.length()-2)
		{
			l_sOut[i] = decodeNumber(&l_sOut[i+1]);
			l_sOut.erase(i+1, 2);
		}
	}

	return l_sOut;
}



extern map<uint32, ::GdkColor> g_vColors;

static ::GtkTargetEntry g_vTargetEntry[]= {
	{ (gchar*)"STRING", 0, 0 },
	{ (gchar*)"text/plain", 0, 0 } };

static ::GdkColor colorFromIdentifier(const CIdentifier& rIdentifier)
{
	::GdkColor l_oGdkColor;
	unsigned int l_ui32Value1=0;
	unsigned int l_ui32Value2=0;
	uint64 l_ui64Result=0;

	sscanf(rIdentifier.toString(), "(0x%08X, 0x%08X)", &l_ui32Value1, &l_ui32Value2);
	l_ui64Result+=l_ui32Value1;
	l_ui64Result<<=32;
	l_ui64Result+=l_ui32Value2;

	l_oGdkColor.pixel=(guint16)0;
	//TODO: check if the or is relevant or just introduce a possibility of bug
	l_oGdkColor.red  =(guint16)(((l_ui64Result    )&0xffff)|0x8000);
	l_oGdkColor.green=(guint16)(((l_ui64Result>>16)&0xffff)|0x8000);
	l_oGdkColor.blue =(guint16)(((l_ui64Result>>32)&0xffff)|0x8000);

	return l_oGdkColor;
}

static std::string getBoxAlgorithmURL(const std::string& sInput, const boolean bRemoveSlash=false)
{
	std::string l_sInput(sInput);
	std::string l_sOutput;
	bool l_bLastWasSeparator=true;

	for(std::string::size_type i=0; i<l_sInput.length(); i++)
	{
		if((l_sInput[i]>='a' && l_sInput[i]<='z') || (l_sInput[i]>='A' && l_sInput[i]<='Z') || (l_sInput[i]>='0' && l_sInput[i]<='9') || (!bRemoveSlash && l_sInput[i]=='/'))
		{
			if(l_sInput[i]=='/')
			{
				l_sOutput+="_";
			}
			else
			{
				if(l_bLastWasSeparator)
				{
					if('a' <= l_sInput[i] && l_sInput[i] <= 'z')
					{
						l_sOutput+=l_sInput[i]+'A'-'a';
					}
					else
					{
						l_sOutput+=l_sInput[i];
					}
				}
				else
				{
					l_sOutput+=l_sInput[i];
				}
			}
			l_bLastWasSeparator=false;
		}
		else
		{
/*
			if(!l_bLastWasSeparator)
			{
				l_sOutput+="_";
			}
*/
			l_bLastWasSeparator=true;
		}
	}
	return l_sOutput;
}

static gboolean scenario_scrolledwindow_scroll_event_cb(::GtkWidget* pWidget, ::GdkEventScroll* pEvent)
{
	guint l_uiState = pEvent->state & gtk_accelerator_get_default_mod_mask ();

	/* Shift+Wheel scrolls the in the perpendicular direction */
	if (l_uiState & GDK_SHIFT_MASK) {
		if (pEvent->direction == GDK_SCROLL_UP)
			pEvent->direction = GDK_SCROLL_LEFT;
		else if (pEvent->direction == GDK_SCROLL_LEFT)
			pEvent->direction = GDK_SCROLL_UP;
		else if (pEvent->direction == GDK_SCROLL_DOWN)
			pEvent->direction = GDK_SCROLL_RIGHT;
		else if (pEvent->direction == GDK_SCROLL_RIGHT)
			pEvent->direction = GDK_SCROLL_DOWN;

		pEvent->state &= ~GDK_SHIFT_MASK;
		l_uiState &= ~GDK_SHIFT_MASK;
	}

	return FALSE;
}

static void scenario_drawing_area_expose_cb(::GtkWidget* pWidget, ::GdkEventExpose* pEvent, gpointer pUserData)
{
	static_cast<CInterfacedScenario*>(pUserData)->scenarioDrawingAreaExposeCB(pEvent);
}
static void scenario_drawing_area_drag_data_received_cb(::GtkWidget* pWidget, ::GdkDragContext* pDragContext, gint iX, gint iY, ::GtkSelectionData* pSelectionData, guint uiInfo, guint uiT, gpointer pUserData)
{
	static_cast<CInterfacedScenario*>(pUserData)->scenarioDrawingAreaDragDataReceivedCB(pDragContext, iX, iY, pSelectionData, uiInfo, uiT);
}
static gboolean scenario_drawing_area_motion_notify_cb(::GtkWidget* pWidget, ::GdkEventMotion* pEvent, gpointer pUserData)
{
	static_cast<CInterfacedScenario*>(pUserData)->scenarioDrawingAreaMotionNotifyCB(pWidget, pEvent);
	return FALSE;
}
static void scenario_drawing_area_button_pressed_cb(::GtkWidget* pWidget, ::GdkEventButton* pEvent, gpointer pUserData)
{
	static_cast<CInterfacedScenario*>(pUserData)->scenarioDrawingAreaButtonPressedCB(pWidget, pEvent);
}
static void scenario_drawing_area_button_released_cb(::GtkWidget* pWidget, ::GdkEventButton* pEvent, gpointer pUserData)
{
	static_cast<CInterfacedScenario*>(pUserData)->scenarioDrawingAreaButtonReleasedCB(pWidget, pEvent);
}
static gboolean scenario_drawing_area_key_press_event_cb(::GtkWidget* pWidget, ::GdkEventKey* pEvent, gpointer pUserData)
{
	static_cast<CInterfacedScenario*>(pUserData)->scenarioDrawingAreaKeyPressEventCB(pWidget, pEvent);
	return true;
}
static void scenario_drawing_area_key_release_event_cb(::GtkWidget* pWidget, ::GdkEventKey* pEvent, gpointer pUserData)
{
	static_cast<CInterfacedScenario*>(pUserData)->scenarioDrawingAreaKeyReleaseEventCB(pWidget, pEvent);
}
static void scenario_drawing_area_configure_event_cb(::GtkWidget *pWidget, ::GdkRectangle *pRectangle, gpointer pUserData)
{
	static_cast<CInterfacedScenario*>(pUserData)->scenarioDrawingAreaConfigureEventCB(pWidget, pRectangle);
}
static void scenario_drawing_area_leave_notify_event_cb(::GtkWidget* pWidget, ::GdkEventKey* pEvent, gpointer pUserData)
{
	static_cast<CInterfacedScenario*>(pUserData)->scenarioDrawingAreaLeaveNotifyCB(pWidget, pEvent);
}


static void context_menu_cb(::GtkMenuItem* pMenuItem, gpointer pUserData)
{
	CInterfacedScenario::BoxContextMenuCB* l_pContextMenuCB=static_cast < CInterfacedScenario::BoxContextMenuCB* >(pUserData);
	switch(l_pContextMenuCB->ui32Command)
	{
		case ContextMenu_SelectionCopy:    l_pContextMenuCB->pInterfacedScenario->copySelection(); break;
		case ContextMenu_SelectionCut:     l_pContextMenuCB->pInterfacedScenario->cutSelection(); break;
		case ContextMenu_SelectionPaste:   l_pContextMenuCB->pInterfacedScenario->pasteSelection(); break;
		case ContextMenu_SelectionDelete:  l_pContextMenuCB->pInterfacedScenario->deleteSelection(); break;
		case ContextMenu_SelectionMute:    l_pContextMenuCB->pInterfacedScenario->muteSelection(); break;

		case ContextMenu_BoxRename:        l_pContextMenuCB->pInterfacedScenario->contextMenuBoxRenameCB(*l_pContextMenuCB->pBox); break;
		//case ContextMenu_BoxRename:        l_pContextMenuCB->pInterfacedScenario->contextMenuBoxRenameAllCB(); break;
		case ContextMenu_BoxDelete:        l_pContextMenuCB->pInterfacedScenario->contextMenuBoxDeleteCB(*l_pContextMenuCB->pBox); break;
		case ContextMenu_BoxAddInput:      l_pContextMenuCB->pInterfacedScenario->contextMenuBoxAddInputCB(*l_pContextMenuCB->pBox); break;
		case ContextMenu_BoxEditInput:     l_pContextMenuCB->pInterfacedScenario->contextMenuBoxEditInputCB(*l_pContextMenuCB->pBox, l_pContextMenuCB->ui32Index); break;
		case ContextMenu_BoxRemoveInput:   l_pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveInputCB(*l_pContextMenuCB->pBox, l_pContextMenuCB->ui32Index); break;
		case ContextMenu_BoxAddOutput:     l_pContextMenuCB->pInterfacedScenario->contextMenuBoxAddOutputCB(*l_pContextMenuCB->pBox); break;
		case ContextMenu_BoxEditOutput:    l_pContextMenuCB->pInterfacedScenario->contextMenuBoxEditOutputCB(*l_pContextMenuCB->pBox, l_pContextMenuCB->ui32Index); break;
		case ContextMenu_BoxRemoveOutput:  l_pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveOutputCB(*l_pContextMenuCB->pBox, l_pContextMenuCB->ui32Index); break;
		case ContextMenu_BoxAddSetting:    l_pContextMenuCB->pInterfacedScenario->contextMenuBoxAddSettingCB(*l_pContextMenuCB->pBox); break;
		case ContextMenu_BoxEditSetting:   l_pContextMenuCB->pInterfacedScenario->contextMenuBoxEditSettingCB(*l_pContextMenuCB->pBox, l_pContextMenuCB->ui32Index); break;
		case ContextMenu_BoxRemoveSetting: l_pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveSettingCB(*l_pContextMenuCB->pBox, l_pContextMenuCB->ui32Index); break;
		case ContextMenu_BoxConfigure:     l_pContextMenuCB->pInterfacedScenario->contextMenuBoxConfigureCB(*l_pContextMenuCB->pBox); break;
		case ContextMenu_BoxAbout:         l_pContextMenuCB->pInterfacedScenario->contextMenuBoxAboutCB(*l_pContextMenuCB->pBox); break;
		case ContextMenu_BoxDocumentation: l_pContextMenuCB->pInterfacedScenario->contextMenuBoxDocumentationCB(*l_pContextMenuCB->pBox); break;
		case ContextMenu_BoxMute:          l_pContextMenuCB->pInterfacedScenario->contextMenuBoxMuteCB(*l_pContextMenuCB->pBox); break;

		case ContextMenu_ScenarioAbout:    l_pContextMenuCB->pInterfacedScenario->contextMenuScenarioAboutCB(); break;

		case ContextMenu_BoxAddMessageInput:       l_pContextMenuCB->pInterfacedScenario->contextMenuBoxAddMessageInputCB(*l_pContextMenuCB->pBox); break;
		case ContextMenu_BoxRemoveMessageInput:    l_pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveMessageInputCB(*l_pContextMenuCB->pBox, l_pContextMenuCB->ui32Index); break;
		case ContextMenu_BoxAddMessageOutput:      l_pContextMenuCB->pInterfacedScenario->contextMenuBoxAddMessageOutputCB(*l_pContextMenuCB->pBox); break;
		case ContextMenu_BoxRemoveMessageOutput:   l_pContextMenuCB->pInterfacedScenario->contextMenuBoxRemoveMessageOutputCB(*l_pContextMenuCB->pBox, l_pContextMenuCB->ui32Index); break;

		case ContextMenu_BoxEditMessageInput:      l_pContextMenuCB->pInterfacedScenario->contextMenuBoxEditMessageInputCB(*l_pContextMenuCB->pBox, l_pContextMenuCB->ui32Index); break;
		case ContextMenu_BoxEditMessageOutput:     l_pContextMenuCB->pInterfacedScenario->contextMenuBoxEditMessageOutputCB(*l_pContextMenuCB->pBox,  l_pContextMenuCB->ui32Index); break;


	}
	// Redraw in any case, as some of the actual callbacks can forget to redraw. As this callback is only called after the user has accessed
	// the right-click menu, so its not a large overhead to do it in general. @TODO might remove the individual redraws.
	l_pContextMenuCB->pInterfacedScenario->forceRedraw();
}

static void gdk_draw_rounded_rectangle(::GdkDrawable* pDrawable, ::GdkGC* pDrawGC, ::gboolean bFill, gint x, gint y, gint width, gint height, gint radius=8)
{
	if(bFill)
	{
#if defined TARGET_OS_Linux
		gdk_draw_rectangle(
			pDrawable,
			pDrawGC,
			TRUE,
			x+radius, y, width-2*radius, height);
		gdk_draw_rectangle(
			pDrawable,
			pDrawGC,
			TRUE,
			x, y+radius, width, height-2*radius);
#elif defined TARGET_OS_Windows
		gdk_draw_rectangle(
			pDrawable,
			pDrawGC,
			TRUE,
			x+radius, y, width-2*radius+1, height+1);
		gdk_draw_rectangle(
			pDrawable,
			pDrawGC,
			TRUE,
			x, y+radius, width+1, height-2*radius+1);
#else
	#pragma error("you should give a version of this function for your OS")
#endif
	}
	else
	{
		gdk_draw_line(
			pDrawable,
			pDrawGC,
			x+radius, y, x+width-radius, y);
		gdk_draw_line(
			pDrawable,
			pDrawGC,
			x+radius, y+height, x+width-radius, y+height);
		gdk_draw_line(
			pDrawable,
			pDrawGC,
			x, y+radius, x, y+height-radius);
		gdk_draw_line(
			pDrawable,
			pDrawGC,
			x+width, y+radius, x+width, y+height-radius);
	}
#if defined TARGET_OS_Linux
	gdk_draw_arc(
		pDrawable,
		pDrawGC,
		bFill,
		x+width-radius*2, y, radius*2, radius*2, 0*64, 90*64);
	gdk_draw_arc(
		pDrawable,
		pDrawGC,
		bFill,
		x, y, radius*2, radius*2, 90*64, 90*64);
	gdk_draw_arc(
		pDrawable,
		pDrawGC,
		bFill,
		x, y+height-radius*2, radius*2, radius*2, 180*64, 90*64);
	gdk_draw_arc(
		pDrawable,
		pDrawGC,
		bFill,
		x+width-radius*2, y+height-radius*2, radius*2, radius*2, 270*64, 90*64);
#elif defined TARGET_OS_Windows
	gdk_draw_arc(
		pDrawable,
		pDrawGC,
		bFill,
		x+width-radius*2, y, radius*2+(bFill?2:1), radius*2+(bFill?2:1), 0*64, 90*64);
	gdk_draw_arc(
		pDrawable,
		pDrawGC,
		bFill,
		x, y, radius*2+(bFill?2:1), radius*2+(bFill?2:1), 90*64, 90*64);
	gdk_draw_arc(
		pDrawable,
		pDrawGC,
		bFill,
		x, y+height-radius*2, radius*2+(bFill?2:1), radius*2+(bFill?2:1), 180*64, 90*64);
	gdk_draw_arc(
		pDrawable,
		pDrawGC,
		bFill,
		x+width-radius*2, y+height-radius*2, radius*2+(bFill?2:1), radius*2+(bFill?2:1), 270*64, 90*64);
#else
	#pragma error("you should give a version of this function for your OS")
#endif
}


void scenario_title_button_close_cb(::GtkButton* pButton, gpointer pUserData)
{
	static_cast<CInterfacedScenario*>(pUserData)->m_rApplication.closeScenarioCB(static_cast<CInterfacedScenario*>(pUserData));
}

CInterfacedScenario::CInterfacedScenario(const IKernelContext& rKernelContext, CApplication& rApplication, IScenario& rScenario, CIdentifier& rScenarioIdentifier, ::GtkNotebook& rNotebook, const char* sGUIFilename, const char* sGUISettingsFilename)
	:m_ePlayerStatus(PlayerStatus_Uninitialized)
	,m_oScenarioIdentifier(rScenarioIdentifier)
	,m_rApplication(rApplication)
	,m_rKernelContext(rKernelContext)
	,m_rScenario(rScenario)
	,m_pPlayer(NULL)
	,m_rNotebook(rNotebook)
	,m_pVisualisationTree(NULL)
	,m_bDesignerVisualisationToggled(false)
	,m_pDesignerVisualisation(NULL)
	,m_pPlayerVisualisation(NULL)
	,m_pBuilderDummyScenarioNotebookTitle(NULL)
	,m_pBuilderDummyScenarioNotebookClient(NULL)
	,m_pBuilderTooltip(NULL)
	,m_pNotebookPageTitle(NULL)
	,m_pNotebookPageContent(NULL)
	,m_pScenarioViewport(NULL)
	,m_pScenarioDrawingArea(NULL)
	,m_pBufferedDrawingArea(NULL)
	,m_pStencilBuffer(NULL)
	,m_pTooltip(NULL)
	,m_bScenarioModified(true)
	,m_bHasFileName(false)
	,m_bHasBeenModified(false)
	,m_bButtonPressed(false)
	,m_bShiftPressed(false)
	,m_bControlPressed(false)
	,m_bAltPressed(false)
	,m_bAPressed(false)
	,m_bWPressed(false)
	,m_bDebugCPUUsage(false)
	,m_sGUIFilename(sGUIFilename)
	,m_sGUISettingsFilename(sGUISettingsFilename)
	,m_i32ViewOffsetX(0)
	,m_i32ViewOffsetY(0)
	,m_ui32CurrentMode(Mode_None)
	,m_oStateStack(rKernelContext, rApplication, rScenario)
{
	//empty Window-menu
	m_vCheckItems.clear();

	m_pBuilderDummyScenarioNotebookTitle=gtk_builder_new(); // glade_xml_new(m_sGUIFilename.c_str(), "openvibe_scenario_notebook_title", NULL);
	gtk_builder_add_from_file(m_pBuilderDummyScenarioNotebookTitle, m_sGUIFilename.c_str(), NULL);
	gtk_builder_connect_signals(m_pBuilderDummyScenarioNotebookTitle, NULL);

	m_pBuilderDummyScenarioNotebookClient=gtk_builder_new(); // glade_xml_new(m_sGUIFilename.c_str(), "openvibe_scenario_notebook_scrolledwindow", NULL);
	gtk_builder_add_from_file(m_pBuilderDummyScenarioNotebookClient, m_sGUIFilename.c_str(), NULL);
	gtk_builder_connect_signals(m_pBuilderDummyScenarioNotebookClient, NULL);

	m_pBuilderTooltip=gtk_builder_new(); // glade_xml_new(m_sGUIFilename.c_str(), "tooltip", NULL);
	gtk_builder_add_from_file(m_pBuilderTooltip, m_sGUIFilename.c_str(), NULL);
	gtk_builder_connect_signals(m_pBuilderTooltip, NULL);

	m_pNotebookPageTitle=GTK_WIDGET(gtk_builder_get_object(m_pBuilderDummyScenarioNotebookTitle, "openvibe_scenario_notebook_title"));
	m_pNotebookPageContent=GTK_WIDGET(gtk_builder_get_object(m_pBuilderDummyScenarioNotebookClient, "openvibe_scenario_notebook_scrolledwindow"));

	gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderDummyScenarioNotebookTitle, "openvibe-scenario_notebook")), 0);
	gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilderDummyScenarioNotebookClient, "openvibe-scenario_notebook")), 0);
	gtk_notebook_append_page(&m_rNotebook, m_pNotebookPageContent, m_pNotebookPageTitle);
	gtk_notebook_set_tab_reorderable(&m_rNotebook, m_pNotebookPageContent, true);

	GtkWidget* l_pCloseWidget=GTK_WIDGET(gtk_builder_get_object(m_pBuilderDummyScenarioNotebookTitle, "openvibe-scenario_button_close"));
	g_signal_connect(G_OBJECT(l_pCloseWidget), "clicked", G_CALLBACK(scenario_title_button_close_cb), this);

	m_pScenarioDrawingArea=GTK_DRAWING_AREA(gtk_builder_get_object(m_pBuilderDummyScenarioNotebookClient, "openvibe-scenario_drawing_area"));
	m_pScenarioViewport=GTK_VIEWPORT(gtk_builder_get_object(m_pBuilderDummyScenarioNotebookClient, "openvibe-scenario_viewport"));
	//get scrolled window for arrow keys scrolling
	m_pScrolledWindow = GTK_SCROLLED_WINDOW(gtk_builder_get_object(m_pBuilderDummyScenarioNotebookClient, "openvibe_scenario_notebook_scrolledwindow"));

	gtk_drag_dest_set(GTK_WIDGET(m_pScenarioDrawingArea), GTK_DEST_DEFAULT_ALL, g_vTargetEntry, sizeof(g_vTargetEntry)/sizeof(::GtkTargetEntry), GDK_ACTION_COPY);
	//gtk_drag_dest_add_text_targets(GTK_WIDGET(m_pScenarioDrawingArea));
	gtk_drag_dest_add_uri_targets(GTK_WIDGET(m_pScenarioDrawingArea));//for drag&drop scenario file, on windows we get file uri so we need to enable this

	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "expose_event", G_CALLBACK(scenario_drawing_area_expose_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "drag_data_received", G_CALLBACK(scenario_drawing_area_drag_data_received_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "motion_notify_event", G_CALLBACK(scenario_drawing_area_motion_notify_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "button_press_event", G_CALLBACK(scenario_drawing_area_button_pressed_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "button_release_event", G_CALLBACK(scenario_drawing_area_button_released_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "key-press-event", G_CALLBACK(scenario_drawing_area_key_press_event_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "key-release-event", G_CALLBACK(scenario_drawing_area_key_release_event_cb), this);
	g_signal_connect(G_OBJECT(m_pScenarioDrawingArea), "leave-notify-event", G_CALLBACK(scenario_drawing_area_leave_notify_event_cb), this);
	g_signal_connect(G_OBJECT(m_pNotebookPageContent), "scroll-event", G_CALLBACK(scenario_scrolledwindow_scroll_event_cb), this);
	g_signal_connect(G_OBJECT(m_pScrolledWindow), "size-allocate", G_CALLBACK(scenario_drawing_area_configure_event_cb), this);

	//retrieve visualisation tree
	m_oVisualisationTreeIdentifier = m_rScenario.getVisualisationTreeIdentifier();
	m_pVisualisationTree = &m_rKernelContext.getVisualisationManager().getVisualisationTree(m_oVisualisationTreeIdentifier);

	//By default there is no zoom
	m_f64CurrentScale = 1;

	//We store the normal size of the font, to handle scaling.
	PangoContext* l_pPongoContext = gtk_widget_get_pango_context(GTK_WIDGET(m_pScenarioDrawingArea));
	PangoFontDescription *l_pPangoFontDescription = pango_context_get_font_description(l_pPongoContext);
	m_ui32NormalFontSize = pango_font_description_get_size(l_pPangoFontDescription);

	//create window manager
	m_pDesignerVisualisation = new CDesignerVisualisation(m_rKernelContext, *m_pVisualisationTree, *this);
	m_pDesignerVisualisation->init(string(sGUIFilename));
	this->snapshotCB();
	m_bHasBeenModified=false;
	this->updateScenarioLabel();

	m_pTooltip=GTK_WIDGET(gtk_builder_get_object(m_pBuilderTooltip, "tooltip"));
	gtk_widget_set_name(m_pTooltip, "gtk-tooltips");
}

CInterfacedScenario::~CInterfacedScenario(void)
{
	//delete window manager
	if(m_pDesignerVisualisation)
	{
		delete m_pDesignerVisualisation;
	}

	if(m_pStencilBuffer)
	{
		g_object_unref(m_pStencilBuffer);
	}

	g_object_unref(m_pBuilderDummyScenarioNotebookTitle);
	g_object_unref(m_pBuilderDummyScenarioNotebookClient);
	g_object_unref(m_pBuilderTooltip);

	gtk_notebook_remove_page(
		&m_rNotebook,
		gtk_notebook_page_num(&m_rNotebook, m_pNotebookPageContent));
}

boolean CInterfacedScenario::isLocked(void) const
{
	return m_pPlayer!=NULL?true:false;
}

void CInterfacedScenario::forceRedraw(void)
{
	m_bScenarioModified = true;
	this->redraw();
}

void CInterfacedScenario::redraw(void)
{
	if(GDK_IS_WINDOW(GTK_WIDGET(m_pScenarioDrawingArea)->window))
	{
		gdk_window_invalidate_rect(
			GTK_WIDGET(m_pScenarioDrawingArea)->window,
			NULL,
			true);
	}
}

void CInterfacedScenario::updateScenarioLabel(void)
{
	::GtkLabel* l_pTitleLabel=GTK_LABEL(gtk_builder_get_object(m_pBuilderDummyScenarioNotebookTitle, "openvibe-scenario_label"));
	string l_sLabel;
	string l_sTempFileName=m_sFileName;
	string::size_type l_iBackSlashIndex;
	while((l_iBackSlashIndex=l_sTempFileName.find('\\'))!=string::npos)
	{
		l_sTempFileName[l_iBackSlashIndex]='/';
	}
	l_sLabel+=m_bHasBeenModified?"*":"";
	if(this->m_ePlayerStatus == PlayerStatus_Play)
	{
		l_sLabel+=" &#9656; ";
	}
	else if(this->m_ePlayerStatus == PlayerStatus_Pause)
	{
		l_sLabel+="<b> &#8403; &#8403;</b> ";
	}
	else if(this->m_ePlayerStatus == PlayerStatus_Forward)
	{
		l_sLabel+=" &#9656;&#9656; ";
	}
	else
		l_sLabel += " ";
	l_sLabel+=m_bHasFileName?l_sTempFileName.substr(l_sTempFileName.rfind('/')+1):"(untitled)";
	l_sLabel+=" ";
	l_sLabel+=m_bHasBeenModified?"*":"";
	gtk_label_set_markup(l_pTitleLabel, l_sLabel.c_str());
}

#define updateStencilIndex(id,stencilgc) { id++; ::GdkColor sc={0, (guint16)((id&0xff0000)>>8), (guint16)(id&0xff00), (guint16)((id&0xff)<<8) }; gdk_gc_set_rgb_fg_color(stencilgc, &sc); }


void CInterfacedScenario::redraw(IBox& rBox)
{
	::GdkGC* l_pStencilGC=gdk_gc_new(GDK_DRAWABLE(m_pStencilBuffer));
	::GdkGC* l_pDrawGC=gdk_gc_new(GTK_WIDGET(m_pScenarioDrawingArea)->window);

	vector<pair<int32, int32> > l_vInputPosition;
	vector<pair<int32, int32> > l_vOutputPosition;

	const int xMargin=ov_round(5.0*m_f64CurrentScale);
	const int yMargin=ov_round(5.0*m_f64CurrentScale);
	const int iCircleSize=ov_round(11.0*m_f64CurrentScale);
	const int iCircleSpace=ov_round(4.0*m_f64CurrentScale);

	CBoxProxy l_oBoxProxy(m_rKernelContext, rBox);
	const int xSize=(int)l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) + xMargin*2;
	const int ySize=(int)l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) + yMargin*2;
	const int xStart=ov_round(l_oBoxProxy.getXCenter()*m_f64CurrentScale+m_i32ViewOffsetX-(xSize>>1));
	const int yStart=ov_round(l_oBoxProxy.getYCenter()*m_f64CurrentScale+m_i32ViewOffsetY-(ySize>>1));

	updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
	gdk_draw_rounded_rectangle(
		GDK_DRAWABLE(m_pStencilBuffer),
		l_pStencilGC,
		TRUE,
		xStart, yStart, xSize, ySize, ov_round(8.0*m_f64CurrentScale));
	m_vInterfacedObject[m_ui32InterfacedObjectId]=CInterfacedObject(rBox.getIdentifier());

	const boolean l_bCanCreate =l_oBoxProxy.isBoxAlgorithmPluginPresent();
	const boolean l_bUpToDate  =l_bCanCreate ?  l_oBoxProxy.isUpToDate() : true;
	const boolean l_bDeprecated=l_bCanCreate && l_oBoxProxy.isDeprecated();
	const boolean l_bUnstable  =l_bCanCreate && l_oBoxProxy.isUnstable();
	const boolean l_bMuted     =l_oBoxProxy.getMute();

	if(!this->isLocked() || !m_bDebugCPUUsage)
	{
		if(m_vCurrentObject[rBox.getIdentifier()])
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundSelected]);
		}
		else if(!l_bCanCreate)
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundMissing]);
		}
		else if(l_bDeprecated)
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundDeprecated]);
		}
		else if(!l_bUpToDate)
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundNeedsUpdate]);
		}
		else if(l_bMuted)
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundMuted]);
		}
		else if(l_bUnstable)
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackgroundUnstable]);
		}
		else
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxBackground]);
		}
	}
	else
	{
		CIdentifier l_oComputationTime;
		l_oComputationTime.fromString(rBox.getAttributeValue(OV_AttributeId_Box_ComputationTimeLastSecond));

		// FIXME use timearithmetics.h?
		const uint64 l_ui64ComputationTime=(l_oComputationTime==OV_UndefinedIdentifier?0:l_oComputationTime.toUInteger());
		const uint64 l_ui64ComputationTimeReference=(1LL<<32)/(m_ui32BoxCount==0?1:m_ui32BoxCount);

		::GdkColor l_oColor;
		if(l_ui64ComputationTime<l_ui64ComputationTimeReference)
		{
			l_oColor.pixel=0;
			l_oColor.red  =(guint16)((l_ui64ComputationTime<<16)/l_ui64ComputationTimeReference);
			l_oColor.green=32768;
			l_oColor.blue =0;
		}
		else
		{
			if(l_ui64ComputationTime<l_ui64ComputationTimeReference*4)
			{
				l_oColor.pixel=0;
				l_oColor.red  =65535;
				l_oColor.green=(guint16)(32768-((l_ui64ComputationTime<<15)/(l_ui64ComputationTimeReference*4)));
				l_oColor.blue =0;
			}
			else
			{
				l_oColor.pixel=0;
				l_oColor.red  =65535;
				l_oColor.green=0;
				l_oColor.blue =0;
			}
		}
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oColor);
	}
	gdk_draw_rounded_rectangle(
		GDK_DRAWABLE(m_pBufferedDrawingArea),
		l_pDrawGC,
		TRUE,
		xStart, yStart, xSize, ySize, ov_round(8.0*m_f64CurrentScale));
	gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[m_vCurrentObject[rBox.getIdentifier()]?Color_BoxBorderSelected:Color_BoxBorder]);
	gdk_draw_rounded_rectangle(
		GDK_DRAWABLE(m_pBufferedDrawingArea),
		l_pDrawGC,
		FALSE,
		xStart, yStart, xSize, ySize, ov_round(8.0*m_f64CurrentScale));

	TAttributeHandler l_oAttributeHandler(rBox);

	if(!l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_XSize))
		l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Box_XSize, xSize);
	else
		l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Box_XSize, xSize);

	if(!l_oAttributeHandler.hasAttribute(OV_AttributeId_Box_YSize))
		l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Box_YSize, ySize);
	else
		l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Box_YSize, ySize);

	const int l_iInputOffset=xSize/2-rBox.getInputCount()*(iCircleSpace+iCircleSize)/2+iCircleSize/4;
	for(uint32 i=0; i<rBox.getInputCount(); i++)
	{
		CIdentifier l_oInputIdentifier;
		rBox.getInputType(i, l_oInputIdentifier);
		::GdkColor l_oInputColor=colorFromIdentifier(l_oInputIdentifier);

		::GdkPoint l_vPoint[4];
		l_vPoint[0].x=iCircleSize>>1;
		l_vPoint[0].y=iCircleSize;
		l_vPoint[1].x=0;
		l_vPoint[1].y=0;
		l_vPoint[2].x=iCircleSize-1;
		l_vPoint[2].y=0;
		for(int j=0; j<3; j++)
		{
			l_vPoint[j].x+=xStart+i*(iCircleSpace+iCircleSize)+l_iInputOffset;
			l_vPoint[j].y+=yStart-(iCircleSize>>1);
		}

		updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
		gdk_draw_polygon(
			GDK_DRAWABLE(m_pStencilBuffer),
			l_pStencilGC,
			TRUE,
			l_vPoint,
			3);
		m_vInterfacedObject[m_ui32InterfacedObjectId]=CInterfacedObject(rBox.getIdentifier(), Connector_Input, i);

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oInputColor);
		gdk_draw_polygon(
			GDK_DRAWABLE(m_pBufferedDrawingArea),
			l_pDrawGC,
			TRUE,
			l_vPoint,
			3);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxInputBorder]);
		gdk_draw_polygon(
			GDK_DRAWABLE(m_pBufferedDrawingArea),
			l_pDrawGC,
			FALSE,
			l_vPoint,
			3);

		const int32 x=xStart+i*(iCircleSpace+iCircleSize)+(iCircleSize>>1)-m_i32ViewOffsetX+l_iInputOffset;
		const int32 y=yStart-(iCircleSize>>1)-m_i32ViewOffsetY;
		CIdentifier l_oLinkIdentifier=m_rScenario.getNextLinkIdentifierToBoxInput(OV_UndefinedIdentifier, rBox.getIdentifier(), i);
		while(l_oLinkIdentifier!=OV_UndefinedIdentifier)
		{
			ILink* l_pLink=m_rScenario.getLinkDetails(l_oLinkIdentifier);
			if(l_pLink)
			{
				TAttributeHandler l_oAttributeHandler(*l_pLink);

				if(!l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_XTargetPosition))
					l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_XTargetPosition, x);
				else
					l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XTargetPosition, x);

				if(!l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_YTargetPosition))
					l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_YTargetPosition, y);
				else
					l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YTargetPosition, y);
			}
			l_oLinkIdentifier=m_rScenario.getNextLinkIdentifierToBoxInput(l_oLinkIdentifier, rBox.getIdentifier(), i);
		}
	}
	//m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "message input "<< " / " << rBox.getMessageInputCount() <<"\n";
	//draw messages input
	const int l_iMessageInputOffset=ySize/2-rBox.getMessageInputCount()*(iCircleSpace+iCircleSize)/2+iCircleSize;
	//int minput = 3;
	//int l_iMessageInputOffset=ySize/2-minput*(iCircleSpace+iCircleSize)/2+iCircleSize;
		//for(i=0; i<minput; i++)
	for(uint32 i=0; i<rBox.getMessageInputCount(); i++)
	{
		::GdkColor l_oMessageInputColor;
		l_oMessageInputColor.pixel=(guint16)0;
		l_oMessageInputColor.red  =(guint16)117;
		l_oMessageInputColor.green=(guint16)117;
		l_oMessageInputColor.blue =(guint16)117;

		::GdkPoint l_vPoint[4];
		l_vPoint[0].x=iCircleSize;
		l_vPoint[0].y=-iCircleSize>>1;
		l_vPoint[1].x=0;
		l_vPoint[1].y=-(iCircleSize-1);
		l_vPoint[2].x=0;
		l_vPoint[2].y=0;
		for(int j=0; j<3; j++)
		{
			l_vPoint[j].x+=xStart-(iCircleSize>>1);
			l_vPoint[j].y+=yStart+i*(iCircleSpace+iCircleSize)+l_iMessageInputOffset;;
		}

		updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
		gdk_draw_polygon(
			GDK_DRAWABLE(m_pStencilBuffer),
			l_pStencilGC,
			TRUE,
			l_vPoint,
			3);
		m_vInterfacedObject[m_ui32InterfacedObjectId]=CInterfacedObject(rBox.getIdentifier(), Connector_MessageInput, i);

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oMessageInputColor);
		gdk_draw_polygon(
			GDK_DRAWABLE(m_pBufferedDrawingArea),
			l_pDrawGC,
			TRUE,
			l_vPoint,
			3);


		//int32 x=xStart+i*(iCircleSpace+iCircleSize)+(iCircleSize>>1)-m_i32ViewOffsetX+l_iMessageInputOffset;
		//int32 y=yStart-(iCircleSize>>1)-m_i32ViewOffsetY;
		const int32 x=xStart+(iCircleSize>>1)-m_i32ViewOffsetX - iCircleSize;
		const int32 y=yStart+i*(iCircleSpace+iCircleSize)-m_i32ViewOffsetY+l_iMessageInputOffset - iCircleSize/2;
		CIdentifier l_oLinkIdentifier=m_rScenario.getNextMessageLinkIdentifierToBoxInput(OV_UndefinedIdentifier, rBox.getIdentifier(), i);
		while(l_oLinkIdentifier!=OV_UndefinedIdentifier)
		{
			ILink* l_pLink=m_rScenario.getMessageLinkDetails(l_oLinkIdentifier);
			if(l_pLink)
			{
				TAttributeHandler l_oAttributeHandler(*l_pLink);

				if(!l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_XTargetPosition))
					l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_XTargetPosition, x);
				else
					l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XTargetPosition, x);

				if(!l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_YTargetPosition))
					l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_YTargetPosition, y);
				else
					l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YTargetPosition, y);
			}
			l_oLinkIdentifier=m_rScenario.getNextMessageLinkIdentifierToBoxInput(l_oLinkIdentifier, rBox.getIdentifier(), i);
		}

	}
	const int l_iMessageOutputOffset=ySize/2-rBox.getMessageOutputCount()*(iCircleSpace+iCircleSize)/2+iCircleSize;
	for(uint32 i=0; i<rBox.getMessageOutputCount(); i++)
	{
		::GdkColor l_oMessageOutputColor;
		l_oMessageOutputColor.pixel=(guint16)0;
		l_oMessageOutputColor.red  =(guint16)117;
		l_oMessageOutputColor.green=(guint16)117;
		l_oMessageOutputColor.blue =(guint16)117;

		::GdkPoint l_vPoint[4];
		l_vPoint[0].x=iCircleSize;
		l_vPoint[0].y=-iCircleSize>>1;
		l_vPoint[1].x=0;
		l_vPoint[1].y=-(iCircleSize-1);
		l_vPoint[2].x=0;
		l_vPoint[2].y=0;
		for(int j=0; j<3; j++)
		{
			l_vPoint[j].x+=xStart-(iCircleSize>>1)+xSize;
			l_vPoint[j].y+=yStart+i*(iCircleSpace+iCircleSize)+l_iMessageOutputOffset;;
		}

		updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
		gdk_draw_polygon(
			GDK_DRAWABLE(m_pStencilBuffer),
			l_pStencilGC,
			TRUE,
			l_vPoint,
			3);
		m_vInterfacedObject[m_ui32InterfacedObjectId]=CInterfacedObject(rBox.getIdentifier(), Connector_MessageOutput, i);

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oMessageOutputColor);
		gdk_draw_polygon(
			GDK_DRAWABLE(m_pBufferedDrawingArea),
			l_pDrawGC,
			TRUE,
			l_vPoint,
			3);

		//int32 x=xStart+i*(iCircleSpace+iCircleSize)+(iCircleSize>>1)-m_i32ViewOffsetX+l_iMessageOutputOffset;
		//int32 y=yStart+ySize+(iCircleSize>>1)+1-m_i32ViewOffsetY;
		const int32 x=xStart+(iCircleSize>>1)-m_i32ViewOffsetX + xSize;
		//int32 y=yStart+i*(iCircleSpace+iCircleSize)+ySize-m_i32ViewOffsetY+l_iMessageOutputOffset-iCircleSize;
		const int32 y=yStart+i*(iCircleSpace+iCircleSize)-m_i32ViewOffsetY+l_iMessageOutputOffset - iCircleSize/2;
		CIdentifier l_oLinkIdentifier=m_rScenario.getNextMessageLinkIdentifierFromBoxOutput(OV_UndefinedIdentifier, rBox.getIdentifier(), i);
		while(l_oLinkIdentifier!=OV_UndefinedIdentifier)
		{
			ILink* l_pLink=m_rScenario.getMessageLinkDetails(l_oLinkIdentifier);
			if(l_pLink)
			{
				TAttributeHandler l_oAttributeHandler(*l_pLink);

				if(!l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_XSourcePosition))
					l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_XSourcePosition, x);
				else
					l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XSourcePosition, x);

				if(!l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_YSourcePosition))
					l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_YSourcePosition, y);
				else
					l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YSourcePosition, y);
			}
			l_oLinkIdentifier=m_rScenario.getNextMessageLinkIdentifierFromBoxOutput(l_oLinkIdentifier, rBox.getIdentifier(), i);
		}

	}
	const int l_iOutputOffset=xSize/2-rBox.getOutputCount()*(iCircleSpace+iCircleSize)/2+iCircleSize/4;
	for(uint32 i=0; i<rBox.getOutputCount(); i++)
	{
		CIdentifier l_oOutputIdentifier;
		rBox.getOutputType(i, l_oOutputIdentifier);
		::GdkColor l_oOutputColor=colorFromIdentifier(l_oOutputIdentifier);

		::GdkPoint l_vPoint[4];
		l_vPoint[0].x=iCircleSize>>1;
		l_vPoint[0].y=iCircleSize;
		l_vPoint[1].x=0;
		l_vPoint[1].y=0;
		l_vPoint[2].x=iCircleSize-1;
		l_vPoint[2].y=0;
		for(int j=0; j<3; j++)
		{
			l_vPoint[j].x+=xStart+i*(iCircleSpace+iCircleSize)+l_iOutputOffset;
			l_vPoint[j].y+=yStart-(iCircleSize>>1)+ySize;
		}

		updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
		gdk_draw_polygon(
			GDK_DRAWABLE(m_pStencilBuffer),
			l_pStencilGC,
			TRUE,
			l_vPoint,
			3);
		m_vInterfacedObject[m_ui32InterfacedObjectId]=CInterfacedObject(rBox.getIdentifier(), Connector_Output, i);

		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oOutputColor);
		gdk_draw_polygon(
			GDK_DRAWABLE(m_pBufferedDrawingArea),
			l_pDrawGC,
			TRUE,
			l_vPoint,
			3);
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_BoxOutputBorder]);
		gdk_draw_polygon(
			GDK_DRAWABLE(m_pBufferedDrawingArea),
			l_pDrawGC,
			FALSE,
			l_vPoint,
			3);

		const int32 x=xStart+i*(iCircleSpace+iCircleSize)+(iCircleSize>>1)-m_i32ViewOffsetX+l_iOutputOffset;
		const int32 y=yStart+ySize+(iCircleSize>>1)+1-m_i32ViewOffsetY;
		CIdentifier l_oLinkIdentifier=m_rScenario.getNextLinkIdentifierFromBoxOutput(OV_UndefinedIdentifier, rBox.getIdentifier(), i);
		while(l_oLinkIdentifier!=OV_UndefinedIdentifier)
		{
			ILink* l_pLink=m_rScenario.getLinkDetails(l_oLinkIdentifier);
			if(l_pLink)
			{
				TAttributeHandler l_oAttributeHandler(*l_pLink);

				if(!l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_XSourcePosition))
					l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_XSourcePosition, x);
				else
					l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_XSourcePosition, x);

				if(!l_oAttributeHandler.hasAttribute(OV_AttributeId_Link_YSourcePosition))
					l_oAttributeHandler.addAttribute<int>(OV_AttributeId_Link_YSourcePosition, y);
				else
					l_oAttributeHandler.setAttributeValue<int>(OV_AttributeId_Link_YSourcePosition, y);
			}
			l_oLinkIdentifier=m_rScenario.getNextLinkIdentifierFromBoxOutput(l_oLinkIdentifier, rBox.getIdentifier(), i);
		}
	}

/*
	::GdkPixbuf* l_pPixbuf=gtk_widget_render_icon(l_pWidget, GTK_STOCK_EXECUTE, GTK_ICON_SIZE_SMALL_TOOLBAR, "openvibe");
	if(l_pPixbuf)
	{
		gdk_draw_pixbuf(l_pWidget->window, l_pDrawGC, l_pPixbuf, 0, 0, 10, 10, 64, 64, GDK_RGB_DITHER_NONE, 0, 0);
		g_object_unref(l_pPixbuf);
	}
*/

	::PangoContext* l_pPangoContext=NULL;
	::PangoLayout* l_pPangoLayout=NULL;
	l_pPangoContext=gtk_widget_get_pango_context(GTK_WIDGET(m_pScenarioDrawingArea));
	l_pPangoLayout=pango_layout_new(l_pPangoContext);
	pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
	pango_layout_set_markup(l_pPangoLayout, l_oBoxProxy.getLabel(), -1);
	gdk_draw_layout(
		GDK_DRAWABLE(m_pBufferedDrawingArea),
		GTK_WIDGET(m_pScenarioDrawingArea)->style->text_gc[GTK_WIDGET_STATE(GTK_WIDGET(m_pScenarioDrawingArea))],
		xStart+xMargin, yStart+yMargin, l_pPangoLayout);
	g_object_unref(l_pPangoLayout);

	g_object_unref(l_pDrawGC);
	g_object_unref(l_pStencilGC);

/*
	CLinkPositionSetterEnum l_oLinkPositionSetterInput(Connector_Input, l_vInputPosition);
	CLinkPositionSetterEnum l_oLinkPositionSetterOutput(Connector_Output, l_vOutputPosition);
	rScenario.enumerateLinksToBox(l_oLinkPositionSetterInput, rBox.getIdentifier());
	rScenario.enumerateLinksFromBox(l_oLinkPositionSetterOutput, rBox.getIdentifier());
*/
}

void CInterfacedScenario::redraw(IComment& rComment)
{
	::GdkGC* l_pStencilGC=gdk_gc_new(GDK_DRAWABLE(m_pStencilBuffer));
	::GdkGC* l_pDrawGC=gdk_gc_new(GTK_WIDGET(m_pScenarioDrawingArea)->window);

	// uint32 i;
	const int xMargin=ov_round(16.0*m_f64CurrentScale);
	const int yMargin=ov_round(16.0*m_f64CurrentScale);

	CCommentProxy l_oCommentProxy(m_rKernelContext, rComment);
	const int xSize=(int)l_oCommentProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea))+xMargin*2;
	const int ySize=(int)l_oCommentProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea))+yMargin*2;
	const int xStart=ov_round(l_oCommentProxy.getXCenter()*m_f64CurrentScale+m_i32ViewOffsetX-(xSize>>1));
	const int yStart=ov_round(l_oCommentProxy.getYCenter()*m_f64CurrentScale+m_i32ViewOffsetY-(ySize>>1));

	updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
	gdk_draw_rounded_rectangle(
		GDK_DRAWABLE(m_pStencilBuffer),
		l_pStencilGC,
		TRUE,
		xStart, yStart, xSize, ySize, ov_round(16.0*m_f64CurrentScale));
	m_vInterfacedObject[m_ui32InterfacedObjectId]=CInterfacedObject(rComment.getIdentifier());

	gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[m_vCurrentObject[rComment.getIdentifier()]?Color_CommentBackgroundSelected:Color_CommentBackground]);
	gdk_draw_rounded_rectangle(
		GDK_DRAWABLE(m_pBufferedDrawingArea),
		l_pDrawGC,
		TRUE,
		xStart, yStart, xSize, ySize, ov_round(16.0*m_f64CurrentScale));
	gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[m_vCurrentObject[rComment.getIdentifier()]?Color_CommentBorderSelected:Color_CommentBorder]);
	gdk_draw_rounded_rectangle(
		GDK_DRAWABLE(m_pBufferedDrawingArea),
		l_pDrawGC,
		FALSE,
		xStart, yStart, xSize, ySize, ov_round(16.0*m_f64CurrentScale));

	::PangoContext* l_pPangoContext=NULL;
	::PangoLayout* l_pPangoLayout=NULL;
	l_pPangoContext=gtk_widget_get_pango_context(GTK_WIDGET(m_pScenarioDrawingArea));
	l_pPangoLayout=pango_layout_new(l_pPangoContext);
	pango_layout_set_alignment(l_pPangoLayout, PANGO_ALIGN_CENTER);
	if(pango_parse_markup(rComment.getText().toASCIIString(), -1, 0, NULL, NULL, NULL, NULL))
	{
		pango_layout_set_markup(l_pPangoLayout, rComment.getText().toASCIIString(), -1);
	}
	else
	{
		pango_layout_set_text(l_pPangoLayout, rComment.getText().toASCIIString(), -1);
	}
	gdk_draw_layout(
		GDK_DRAWABLE(m_pBufferedDrawingArea),
		GTK_WIDGET(m_pScenarioDrawingArea)->style->text_gc[GTK_WIDGET_STATE(GTK_WIDGET(m_pScenarioDrawingArea))],
		xStart+xMargin, yStart+yMargin, l_pPangoLayout);
	g_object_unref(l_pPangoLayout);

	g_object_unref(l_pDrawGC);
	g_object_unref(l_pStencilGC);
}



void CInterfacedScenario::redraw(ILink& rLink)
{
	::GdkGC* l_pStencilGC=gdk_gc_new(GDK_DRAWABLE(m_pStencilBuffer));
	::GdkGC* l_pDrawGC=gdk_gc_new(GTK_WIDGET(m_pScenarioDrawingArea)->window);

	CLinkProxy l_oLinkProxy(rLink);

	CIdentifier l_oSourceOutputTypeIdentifier;
	CIdentifier l_oTargetInputTypeIdentifier;

	m_rScenario.getBoxDetails(rLink.getSourceBoxIdentifier())->getOutputType(rLink.getSourceBoxOutputIndex(), l_oSourceOutputTypeIdentifier);
	m_rScenario.getBoxDetails(rLink.getTargetBoxIdentifier())->getInputType(rLink.getTargetBoxInputIndex(), l_oTargetInputTypeIdentifier);

	if(m_vCurrentObject[rLink.getIdentifier()])
	{
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkSelected]);
	}
	else if(l_oTargetInputTypeIdentifier==l_oSourceOutputTypeIdentifier)
	{
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_Link]);
	}
	else
	{
		if(m_rKernelContext.getTypeManager().isDerivedFromStream(l_oSourceOutputTypeIdentifier, l_oTargetInputTypeIdentifier))
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkDownCast]);
		}
		else if(m_rKernelContext.getTypeManager().isDerivedFromStream(l_oTargetInputTypeIdentifier, l_oSourceOutputTypeIdentifier))
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkUpCast]);
		}
		else
		{
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_LinkInvalid]);
		}
	}

	updateStencilIndex(m_ui32InterfacedObjectId, l_pStencilGC);
	gdk_draw_line(
		GDK_DRAWABLE(m_pStencilBuffer),
		l_pStencilGC,
		ov_round(l_oLinkProxy.getXSource()+m_i32ViewOffsetX), ov_round(l_oLinkProxy.getYSource()+m_i32ViewOffsetY),
		ov_round(l_oLinkProxy.getXTarget()+m_i32ViewOffsetX), ov_round(l_oLinkProxy.getYTarget()+m_i32ViewOffsetY));
	gdk_draw_line(
		GDK_DRAWABLE(m_pBufferedDrawingArea),
		l_pDrawGC,
		ov_round(l_oLinkProxy.getXSource()+m_i32ViewOffsetX), ov_round(l_oLinkProxy.getYSource()+m_i32ViewOffsetY),
		ov_round(l_oLinkProxy.getXTarget()+m_i32ViewOffsetX), ov_round(l_oLinkProxy.getYTarget()+m_i32ViewOffsetY));
	m_vInterfacedObject[m_ui32InterfacedObjectId]=CInterfacedObject(rLink.getIdentifier(), Connector_Link, 0);

	g_object_unref(l_pDrawGC);
	g_object_unref(l_pStencilGC);
}

#undef updateStencilIndex

uint32 CInterfacedScenario::pickInterfacedObject(int x, int y)
{
	if(!GDK_DRAWABLE(m_pStencilBuffer))
	{
		// m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "No stencil buffer defined - couldn't pick object... this should never happen !\n";
		return 0xffffffff;
	}

	int l_iMaxX;
	int l_iMaxY;
	uint32 l_ui32InterfacedObjectId=0xffffffff;
	gdk_drawable_get_size(GDK_DRAWABLE(m_pStencilBuffer), &l_iMaxX, &l_iMaxY);

	//to congigure : conf manager token?
	int xSizeOfSelection = 50;
	int ySizeOfSelection = 50;

	//we want a zone around the click
	//gdk_pixbuf_get_from_drawable(NULL, GDK_DRAWABLE(m_pStencilBuffer), NULL, x, y, 0, 0, xSizeOfSelection, ySizeOfSelection);
	//return a pixbuf origin (x,y) of the required size
	//	(x,y) - - -|
	//	  |	       |
	//    |		   |
	//    | _ _ _ _|
	//we want a zone centered around the click
	//thus origin of the zone is x-xSizeOfSelection/2, y-ySizeOfSelection/2
	x-=xSizeOfSelection/2;
	y-=ySizeOfSelection/2;

	//we have to consider limit case near the y=0 and x=0 borders
	if(x<0)
	{
		xSizeOfSelection+=x; // we decrease xSizeOfSelection (x is negative)
		x=0;//origin of the zone is x=0

	}
	if(y<0)
	{
		ySizeOfSelection+=y; // we decrease ySizeOfSelection (y is negative)
		y=0;//origin of the zone is y=0
	}


	if(x>=0 && y>=0 && x<l_iMaxX && y<l_iMaxY)
	{
		//to avoid going out of pixbuff bounds
		//we want
		xSizeOfSelection = (xSizeOfSelection+x>l_iMaxX)?l_iMaxX-x:xSizeOfSelection;
		ySizeOfSelection = (ySizeOfSelection+y>l_iMaxY)?l_iMaxY-y:ySizeOfSelection;

		m_rKernelContext.getLogManager() << LogLevel_Debug << x << " " << y
										 << " for " << xSizeOfSelection << " by " << ySizeOfSelection <<  "\n";



		//::GdkPixbuf* l_pPixbuf=gdk_pixbuf_get_from_drawable(NULL, GDK_DRAWABLE(m_pStencilBuffer), NULL, x, y, 0, 0, 1, 1);
		::GdkPixbuf* l_pPixbuf=gdk_pixbuf_get_from_drawable(NULL, GDK_DRAWABLE(m_pStencilBuffer), NULL, x, y, 0, 0, xSizeOfSelection, ySizeOfSelection);
		if(!l_pPixbuf)
		{
			m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Could not get pixbuf from stencil buffer - couldn't pick object... this should never happen !\n";
			return 0xffffffff;
		}

		guchar* l_pPixels=gdk_pixbuf_get_pixels(l_pPixbuf);
		if(!l_pPixels)
		{
			m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Could not get pixels from pixbuf - couldn't pick object... this should never happen !\n";
			return 0xffffffff;
		}


		//here comes the tricky part
		//we have to check xSizeOfSelection*ySizeOfSelection pixels. If we start from 0 a go incrementaly, we will begin to check the upper left pixel
		//in case there are several objects in the pixbuf, we want the one closer to the center (the click) so we have to go through the pixels in a spiral fashion
		//see http://stackoverflow.com/questions/398299/looping-in-a-spiral slightly modified here
		//first we made the change of variable x=x+X/2 and y=y+Y/2 because our table start from (0.0) and we changed some variables name

		//get some infos from the pixbuf
		int rowstride = gdk_pixbuf_get_rowstride (l_pPixbuf);
		int n_channels = gdk_pixbuf_get_n_channels (l_pPixbuf);
		l_ui32InterfacedObjectId=0;

		int px,py;//coordinates of the pixels we are currently looking at in the pixbuf
		px=xSizeOfSelection/2;
		py=ySizeOfSelection/2;
		int dx,dy;
		dx=0;
		dy=-1;
		int t = std::max(xSizeOfSelection,ySizeOfSelection);
		unsigned int maxI = t*t;
		uint32 l_ui32Counter=0;

		while((l_ui32InterfacedObjectId==0)&&(l_ui32Counter<maxI))
		{
			if((0<=px)&&(px<xSizeOfSelection)&&(0<=py)&&(py<ySizeOfSelection))
			{
				//see gdk pixbuf strucutre documentation for clarification as to why we go through the pixbuf that way
				guchar* p = l_pPixels + px*n_channels + py*rowstride;
				l_ui32InterfacedObjectId+=(p[0]<<16);
				l_ui32InterfacedObjectId+=(p[1]<<8);
				l_ui32InterfacedObjectId+=(p[2]);
			}
			if((px-xSizeOfSelection/2==py-ySizeOfSelection/2)||((px<xSizeOfSelection/2)&&(px-xSizeOfSelection/2==-py+ySizeOfSelection/2))||((px>xSizeOfSelection/2)&&(px-xSizeOfSelection/2==1-py+ySizeOfSelection/2)))
			{
				t = dx;
				dx = -dy;
				dy=t;
			}
			px+=dx;
			py+=dy;
			l_ui32Counter++;
		}

		m_rKernelContext.getLogManager() << LogLevel_Debug << "Found " << l_ui32InterfacedObjectId << " on pixel  " << l_ui32Counter << "/"  << xSizeOfSelection*ySizeOfSelection << " \n";


		g_object_unref(l_pPixbuf);
	}
	return l_ui32InterfacedObjectId;
}

//Return true if new object have been add to m_vCurrentObject
boolean CInterfacedScenario::pickInterfacedObject(int x, int y, int iSizeX, int iSizeY)
{
	if(!GDK_DRAWABLE(m_pStencilBuffer))
	{
		// m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "No stencil buffer defined - couldn't pick object... this should never happen !\n";
		return false;
	}

	int i,j;
	int l_iMaxX;
	int l_iMaxY;
	bool l_bRes = false;
	gdk_drawable_get_size(GDK_DRAWABLE(m_pStencilBuffer), &l_iMaxX, &l_iMaxY);

	int iStartX=x;
	int iStartY=y;
	int iEndX=x+iSizeX;
	int iEndY=y+iSizeY;

	// crops according to drawing area boundings
	if(iStartX<0) iStartX=0;
	if(iStartY<0) iStartY=0;
	if(iEndX<0) iEndX=0;
	if(iEndY<0) iEndY=0;
	if(iStartX>=l_iMaxX-1) iStartX=l_iMaxX-1;
	if(iStartY>=l_iMaxY-1) iStartY=l_iMaxY-1;
	if(iEndX>=l_iMaxX-1) iEndX=l_iMaxX-1;
	if(iEndY>=l_iMaxY-1) iEndY=l_iMaxY-1;

	// recompute new size
	iSizeX=iEndX-iStartX+1;
	iSizeY=iEndY-iStartY+1;

	::GdkPixbuf* l_pPixbuf=gdk_pixbuf_get_from_drawable(NULL, GDK_DRAWABLE(m_pStencilBuffer), NULL, iStartX, iStartY, 0, 0, iSizeX, iSizeY);
	if(!l_pPixbuf)
	{
		m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Could not get pixbuf from stencil buffer - couldn't pick object... this should never happen !\n";
		return false;
	}

	guchar* l_pPixels=gdk_pixbuf_get_pixels(l_pPixbuf);
	if(!l_pPixels)
	{
		m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Could not get pixels from pixbuf - couldn't pick object... this should never happen !\n";
		return false;
	}

	const int l_iRowBytesCount=gdk_pixbuf_get_rowstride(l_pPixbuf);
	const int l_iChannelCount=gdk_pixbuf_get_n_channels(l_pPixbuf);
	for(j=0; j<iSizeY; j++)
	{
		for(i=0; i<iSizeX; i++)
		{
			uint32 l_ui32InterfacedObjectId=0;
			l_ui32InterfacedObjectId+=(l_pPixels[j*l_iRowBytesCount+i*l_iChannelCount+0]<<16);
			l_ui32InterfacedObjectId+=(l_pPixels[j*l_iRowBytesCount+i*l_iChannelCount+1]<<8);
			l_ui32InterfacedObjectId+=(l_pPixels[j*l_iRowBytesCount+i*l_iChannelCount+2]);
			if(m_vInterfacedObject[l_ui32InterfacedObjectId].m_oIdentifier!=OV_UndefinedIdentifier)
			{
				if(!m_vCurrentObject[m_vInterfacedObject[l_ui32InterfacedObjectId].m_oIdentifier])
				{
					m_vCurrentObject[m_vInterfacedObject[l_ui32InterfacedObjectId].m_oIdentifier]=true;
					l_bRes=true;
				}
			}
		}
	}

	g_object_unref(l_pPixbuf);
	return l_bRes;
}

#define OV_ClassId_Selected OpenViBE::CIdentifier(0xC67A01DC, 0x28CE06C1)

void CInterfacedScenario::undoCB(boolean bManageModifiedStatusFlag)
{
	if(m_oStateStack.undo())
	{
		CIdentifier l_oIdentifier;
		m_vCurrentObject.clear();
		while((l_oIdentifier=m_rScenario.getNextBoxIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
			if(m_rScenario.getBoxDetails(l_oIdentifier)->hasAttribute(OV_ClassId_Selected))
				m_vCurrentObject[l_oIdentifier]=true;
		while((l_oIdentifier=m_rScenario.getNextLinkIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
			if(m_rScenario.getLinkDetails(l_oIdentifier)->hasAttribute(OV_ClassId_Selected))
				m_vCurrentObject[l_oIdentifier]=true;

		m_pDesignerVisualisation->load();
		if(bManageModifiedStatusFlag)
		{
			m_bHasBeenModified=true;
			this->updateScenarioLabel();
		}
		m_bScenarioModified = true;
		this->redraw();
	}
	else
	{
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Can not undo\n";
	}
}

void CInterfacedScenario::setScale(OpenViBE::float64 rScale)
{
	m_f64CurrentScale = rScale;
	if(m_f64CurrentScale < 0.1)
		m_f64CurrentScale=0.1;

	PangoContext* l_pPongoContext = gtk_widget_get_pango_context(GTK_WIDGET(m_pScenarioDrawingArea));
	PangoFontDescription *l_pPangoFontDescription = pango_context_get_font_description(l_pPongoContext);
	pango_font_description_set_size(l_pPangoFontDescription, (gint)(m_ui32NormalFontSize*m_f64CurrentScale));

	m_bScenarioModified = true;
	this->redraw();
}

float64 CInterfacedScenario::getScale()
{
	return m_f64CurrentScale;
}

void CInterfacedScenario::redoCB(boolean bManageModifiedStatusFlag)
{
	if(m_oStateStack.redo())
	{
		CIdentifier l_oIdentifier;
		m_vCurrentObject.clear();
		while((l_oIdentifier=m_rScenario.getNextBoxIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
			if(m_rScenario.getBoxDetails(l_oIdentifier)->hasAttribute(OV_ClassId_Selected))
				m_vCurrentObject[l_oIdentifier]=true;
		while((l_oIdentifier=m_rScenario.getNextLinkIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
			if(m_rScenario.getLinkDetails(l_oIdentifier)->hasAttribute(OV_ClassId_Selected))
				m_vCurrentObject[l_oIdentifier]=true;

		m_pDesignerVisualisation->load();
		if(bManageModifiedStatusFlag)
		{
			m_bHasBeenModified=true;
			this->updateScenarioLabel();
		}
		m_bScenarioModified = true;
		this->redraw();
	}
	else
	{
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Can not redo\n";
	}
}

void CInterfacedScenario::snapshotCB(boolean bManageModifiedStatusFlag)
{
	CIdentifier l_oIdentifier;
	while((l_oIdentifier=m_rScenario.getNextBoxIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
		if(m_vCurrentObject[l_oIdentifier])
			m_rScenario.getBoxDetails(l_oIdentifier)->addAttribute(OV_ClassId_Selected, "");
		else
			m_rScenario.getBoxDetails(l_oIdentifier)->removeAttribute(OV_ClassId_Selected);
	while((l_oIdentifier=m_rScenario.getNextLinkIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
		if(m_vCurrentObject[l_oIdentifier])
			m_rScenario.getLinkDetails(l_oIdentifier)->addAttribute(OV_ClassId_Selected, "");
		else
			m_rScenario.getLinkDetails(l_oIdentifier)->removeAttribute(OV_ClassId_Selected);

	while((l_oIdentifier=m_rScenario.getNextMessageLinkIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
		if(m_vCurrentObject[l_oIdentifier])
			m_rScenario.getMessageLinkDetails(l_oIdentifier)->addAttribute(OV_ClassId_Selected, "");
		else
			m_rScenario.getMessageLinkDetails(l_oIdentifier)->removeAttribute(OV_ClassId_Selected);

	if(bManageModifiedStatusFlag)
	{
		m_bHasBeenModified=true;
	}
	this->updateScenarioLabel();
	m_oStateStack.snapshot();
}

void CInterfacedScenario::addCommentCB(int x, int y)
{
	CIdentifier l_oIdentifier;
	m_rScenario.addComment(l_oIdentifier, OV_UndefinedIdentifier);
	if(x==-1 || y==-1)
	{
		::GtkWidget* l_pScrolledWindow=gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(m_pScenarioDrawingArea)));
		::GtkAdjustment* l_pHAdjustment=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(l_pScrolledWindow));
		::GtkAdjustment* l_pVAdjustment=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(l_pScrolledWindow));

#if defined TARGET_OS_Linux && !defined TARGET_OS_MacOS
		x=(int)(gtk_adjustment_get_value(l_pHAdjustment)+gtk_adjustment_get_page_size(l_pHAdjustment)/2);
		y=(int)(gtk_adjustment_get_value(l_pVAdjustment)+gtk_adjustment_get_page_size(l_pVAdjustment)/2);
#elif defined TARGET_OS_Windows
		gint wx,wy;
		::gdk_window_get_size(gtk_widget_get_parent(GTK_WIDGET(m_pScenarioDrawingArea))->window, &wx, &wy);
		x=(int)(gtk_adjustment_get_value(l_pHAdjustment)+wx/2);
		y=(int)(gtk_adjustment_get_value(l_pVAdjustment)+wy/2);
#else
		x=(int)(gtk_adjustment_get_value(l_pHAdjustment)+32);
		y=(int)(gtk_adjustment_get_value(l_pVAdjustment)+32);
#endif
	}

	CCommentProxy l_oCommentProxy(m_rKernelContext, m_rScenario, l_oIdentifier);
	l_oCommentProxy.setCenter(x-m_i32ViewOffsetX, y-m_i32ViewOffsetY);
	this->alignCommentOnGrid(l_oCommentProxy);

	// Applies modifications before snapshot
	l_oCommentProxy.apply();

	CCommentEditorDialog l_oCommentEditorDialog(m_rKernelContext, *m_rScenario.getCommentDetails(l_oIdentifier), m_sGUIFilename.c_str());
	if(!l_oCommentEditorDialog.run())
	{
		m_rScenario.removeComment(l_oIdentifier);
	}
	else
	{
		m_vCurrentObject.clear();
		m_vCurrentObject[l_oIdentifier]=true;

		this->snapshotCB();
	}

	m_bScenarioModified = true;
	this->redraw();
}

// @Important No log should be displayed in this method because it could lead to a crash of gtk during the expand of the log section
//not true anymore on fedora 17 TODO check all platform
void CInterfacedScenario::scenarioDrawingAreaExposeCB(::GdkEventExpose* pEvent)
{
	// @fixme uncomment this log will create a crash of gtk (not on fedora 17)
	//m_rKernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaExposeCB\n";
	gint x = -1;
	gint y = -1;
	::GdkGC* l_pDrawGC= gdk_gc_new(GTK_WIDGET(m_pScenarioDrawingArea)->window);
	//The scenario needs to be redraw entirely so let's go
	if(m_bScenarioModified)
	{
		gint l_iViewportX=-1;
		gint l_iViewportY=-1;

		gint l_iMinX= 0x7fff;
		gint l_iMaxX=-0x7fff;
		gint l_iMinY= 0x7fff;
		gint l_iMaxY=-0x7fff;

		const gint l_iMarginX=ov_round(32.0 * m_f64CurrentScale);
		const gint l_iMarginY=ov_round(32.0 * m_f64CurrentScale);

		CIdentifier l_oBoxIdentifier;
		while((l_oBoxIdentifier=m_rScenario.getNextBoxIdentifier(l_oBoxIdentifier))!=OV_UndefinedIdentifier)
		{
			CBoxProxy l_oBoxProxy(m_rKernelContext, *m_rScenario.getBoxDetails(l_oBoxIdentifier));
			gint l_w = l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea))/2;
			gint l_h = l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea))/2;
			if(l_iMinX>l_oBoxProxy.getXCenter()-l_w) l_iMinX=ov_round(l_oBoxProxy.getXCenter()-l_w);
			if(l_iMaxX<l_oBoxProxy.getXCenter()+l_w) l_iMaxX=ov_round(l_oBoxProxy.getXCenter()+l_w);
			if(l_iMinY>l_oBoxProxy.getYCenter()-l_h) l_iMinY=ov_round(l_oBoxProxy.getYCenter()-l_h);
			if(l_iMaxY<l_oBoxProxy.getYCenter()+l_h) l_iMaxY=ov_round(l_oBoxProxy.getYCenter()+l_h);
		}

		CIdentifier l_oCommentIdentifier;
		while((l_oCommentIdentifier=m_rScenario.getNextCommentIdentifier(l_oCommentIdentifier))!=OV_UndefinedIdentifier)
		{
			CCommentProxy l_oCommentProxy(m_rKernelContext, *m_rScenario.getCommentDetails(l_oCommentIdentifier));
			gint l_w = l_oCommentProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea))/2;
			gint l_h = l_oCommentProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea))/2;
			if(l_iMinX>l_oCommentProxy.getXCenter()-l_w) l_iMinX=ov_round(l_oCommentProxy.getXCenter()-l_w);
			if(l_iMaxX<l_oCommentProxy.getXCenter()+l_w) l_iMaxX=ov_round(l_oCommentProxy.getXCenter()+l_w);
			if(l_iMinY>l_oCommentProxy.getYCenter()-l_h) l_iMinY=ov_round(l_oCommentProxy.getYCenter()-l_h);
			if(l_iMaxY<l_oCommentProxy.getYCenter()-l_h) l_iMaxY=ov_round(l_oCommentProxy.getYCenter()+l_h);
		}

		//Now we apply scale on drawing area dimension
		l_iMaxX = ov_round(l_iMaxX *m_f64CurrentScale);
		l_iMaxY = ov_round(l_iMaxY *m_f64CurrentScale);
		l_iMinX = ov_round(l_iMinX *m_f64CurrentScale);
		l_iMinY = ov_round(l_iMinY *m_f64CurrentScale);
		gint l_iNewScenarioSizeX=l_iMaxX-l_iMinX;
		gint l_iNewScenarioSizeY=l_iMaxY-l_iMinY;
		gint l_iOldScenarioSizeX=-1;
		gint l_iOldScenarioSizeY=-1;

		gdk_window_get_size(GTK_WIDGET(m_pScenarioViewport)->window, &l_iViewportX, &l_iViewportY);
		gtk_widget_get_size_request(GTK_WIDGET(m_pScenarioDrawingArea), &l_iOldScenarioSizeX, &l_iOldScenarioSizeY);

		if(l_iNewScenarioSizeX>=0 && l_iNewScenarioSizeY>=0)
		{
			x = max(l_iNewScenarioSizeX+2*l_iMarginX, l_iViewportX);
			y = max(l_iNewScenarioSizeY+2*l_iMarginY, l_iViewportY);
			if(l_iOldScenarioSizeX!=x || l_iOldScenarioSizeY!=y)
			{
				//the scenario change size
				m_iCurrentWidth = x;
				m_iCurrentHeight = y;
				gtk_widget_set_size_request(GTK_WIDGET(m_pScenarioDrawingArea), x, y);
			}

			//if a box enlarge the scenario, we have to recalculate the ViewOffset
			if(l_iMaxX+m_i32ViewOffsetX>-l_iMarginX+max(l_iViewportX, l_iNewScenarioSizeX+2*l_iMarginX)) { m_i32ViewOffsetX=-l_iMaxX-l_iMarginX+max(l_iViewportX, l_iNewScenarioSizeX+2*l_iMarginX); }
			if(l_iMinX+m_i32ViewOffsetX< l_iMarginX)                                                     { m_i32ViewOffsetX=-l_iMinX+l_iMarginX; }
			if(l_iMaxY+m_i32ViewOffsetY>-l_iMarginY+max(l_iViewportY, l_iNewScenarioSizeY+2*l_iMarginY)) { m_i32ViewOffsetY=-l_iMaxY-l_iMarginY+max(l_iViewportY, l_iNewScenarioSizeY+2*l_iMarginY); }
			if(l_iMinY+m_i32ViewOffsetY< l_iMarginY)                                                     { m_i32ViewOffsetY=-l_iMinY+l_iMarginY; }
		}
		else
		{
			//If scenario has no size, let's say we gonna print nothing
			gtk_widget_set_size_request(GTK_WIDGET(m_pScenarioDrawingArea), l_iViewportX, l_iViewportY);
			if(m_pBufferedDrawingArea) g_object_unref(m_pBufferedDrawingArea);
			m_pBufferedDrawingArea = gdk_pixmap_new(GTK_WIDGET(m_pScenarioDrawingArea)->window, l_iViewportX, l_iViewportY, -1);

			::GdkColor l_oColor;
			l_oColor.pixel=0;
			l_oColor.red  =0xf000;
			l_oColor.green=0xf000;
			l_oColor.blue =0xf000;
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oColor);
			gdk_draw_rectangle(
				GDK_DRAWABLE(m_pBufferedDrawingArea),
				l_pDrawGC,
				TRUE,
				0, 0, l_iViewportX, l_iViewportY);
			gdk_draw_drawable(GTK_WIDGET(m_pScenarioDrawingArea)->window, l_pDrawGC, m_pBufferedDrawingArea, 0, 0, 0, 0, -1, -1);
			return;
		}

		if(m_pStencilBuffer) g_object_unref(m_pStencilBuffer);
		m_pStencilBuffer=gdk_pixmap_new(GTK_WIDGET(m_pScenarioDrawingArea)->window, x, y, -1);
		if(m_pBufferedDrawingArea) g_object_unref(m_pBufferedDrawingArea);
		m_pBufferedDrawingArea = gdk_pixmap_new(GTK_WIDGET(m_pScenarioDrawingArea)->window, x, y, -1);

		::GdkGC* l_pStencilGC=gdk_gc_new(m_pStencilBuffer);
		::GdkColor l_oColor={0, 0, 0, 0};
		gdk_gc_set_rgb_fg_color(l_pStencilGC, &l_oColor);
		gdk_draw_rectangle(
			GDK_DRAWABLE(m_pStencilBuffer),
			l_pStencilGC,
			TRUE,
			0, 0, x, y);
		g_object_unref(l_pStencilGC);

		if(this->isLocked())
		{
			l_oColor.pixel=0;
			l_oColor.red  =0xf700;
			l_oColor.green=0xf000;
			l_oColor.blue =0xf000;
		}
		else
		{
			l_oColor.pixel=0;
			l_oColor.red  =0xf000;
			l_oColor.green=0xf000;
			l_oColor.blue =0xf000;
		}
		gdk_gc_set_rgb_fg_color(l_pDrawGC, &l_oColor);
		gdk_draw_rectangle(
			GDK_DRAWABLE(m_pBufferedDrawingArea),
			l_pDrawGC,
			TRUE,
			0, 0, x, y);

		m_ui32InterfacedObjectId=0;
		m_vInterfacedObject.clear();

		uint32 l_ui32CommentCount=0;
		while((l_oCommentIdentifier=m_rScenario.getNextCommentIdentifier(l_oCommentIdentifier))!=OV_UndefinedIdentifier)
		{
			redraw(*m_rScenario.getCommentDetails(l_oCommentIdentifier));
			l_ui32CommentCount++;
		}
		m_ui32CommentCount=l_ui32CommentCount;

		uint32 l_ui32BoxCount=0;
		while((l_oBoxIdentifier=m_rScenario.getNextBoxIdentifier(l_oBoxIdentifier))!=OV_UndefinedIdentifier)
		{
			redraw(*m_rScenario.getBoxDetails(l_oBoxIdentifier));
			l_ui32BoxCount++;
		}
		m_ui32BoxCount=l_ui32BoxCount;

		uint32 l_ui32LinkCount=0;
		CIdentifier l_oLinkIdentifier;
		while((l_oLinkIdentifier=m_rScenario.getNextLinkIdentifier(l_oLinkIdentifier))!=OV_UndefinedIdentifier)
		{
			redraw(*m_rScenario.getLinkDetails(l_oLinkIdentifier));
			l_ui32LinkCount++;
		}
		m_ui32LinkCount=l_ui32LinkCount;

		uint32 l_ui32MessageLinkCount=0;
		CIdentifier l_oMessageLinkIdentifier;
		while((l_oMessageLinkIdentifier=m_rScenario.getNextMessageLinkIdentifier(l_oMessageLinkIdentifier))!=OV_UndefinedIdentifier)
		{
			redraw(*m_rScenario.getMessageLinkDetails(l_oMessageLinkIdentifier));
			l_ui32MessageLinkCount++;
		}
		m_bScenarioModified = false;
	}
	if (m_pBufferedDrawingArea != NULL){
		gdk_draw_drawable(GTK_WIDGET(m_pScenarioDrawingArea)->window, l_pDrawGC, m_pBufferedDrawingArea, 0, 0, 0, 0, -1, -1);

		if(m_ui32CurrentMode==Mode_Selection || m_ui32CurrentMode==Mode_SelectionAdd)
		{
			const int l_iStartX=(int)min(m_f64PressMouseX, m_f64CurrentMouseX);
			const int l_iStartY=(int)min(m_f64PressMouseY, m_f64CurrentMouseY);
			const int l_iSizeX=(int)max(m_f64PressMouseX-m_f64CurrentMouseX, m_f64CurrentMouseX-m_f64PressMouseX);
			const int l_iSizeY=(int)max(m_f64PressMouseY-m_f64CurrentMouseY, m_f64CurrentMouseY-m_f64PressMouseY);

			::GtkWidget* l_pWidget=GTK_WIDGET(m_pScenarioDrawingArea);
			::GdkGC* l_pDrawGC=gdk_gc_new(l_pWidget->window);
			gdk_gc_set_function(l_pDrawGC, GDK_OR);
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_SelectionArea]);
			gdk_draw_rectangle(
				l_pWidget->window,
				l_pDrawGC,
				TRUE,
				l_iStartX, l_iStartY, l_iSizeX, l_iSizeY);
			gdk_gc_set_function(l_pDrawGC, GDK_COPY);
			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_SelectionAreaBorder]);
			gdk_draw_rectangle(
				l_pWidget->window,
				l_pDrawGC,
				FALSE,
				l_iStartX, l_iStartY, l_iSizeX, l_iSizeY);
			g_object_unref(l_pDrawGC);
		}

		if(m_ui32CurrentMode==Mode_Connect)
		{
			::GtkWidget* l_pWidget=GTK_WIDGET(m_pScenarioDrawingArea);
			::GdkGC* l_pDrawGC=gdk_gc_new(l_pWidget->window);

			gdk_gc_set_rgb_fg_color(l_pDrawGC, &g_vColors[Color_Link]);
			gdk_draw_line(
				l_pWidget->window,
				l_pDrawGC,
				(int)m_f64PressMouseX, (int)m_f64PressMouseY,
				(int)m_f64CurrentMouseX, (int)m_f64CurrentMouseY);
			g_object_unref(l_pDrawGC);
		}
	}
}
void CInterfacedScenario::scenarioDrawingAreaDragDataReceivedCB(::GdkDragContext* pDragContext, gint iX, gint iY, ::GtkSelectionData* pSelectionData, guint uiInfo, guint uiT)
{
	//check if I need to free later
	const char* l_sSelectionText = (const char*)gtk_selection_data_get_text(pSelectionData);
	gchar** l_sSelectionUri = gtk_selection_data_get_uris(pSelectionData);

	m_rKernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaDragDataReceivedCB [";
	if(l_sSelectionText!=NULL)
		m_rKernelContext.getLogManager() << LogLevel_Debug << l_sSelectionText;
	else if (*l_sSelectionUri!=NULL)
		m_rKernelContext.getLogManager() << LogLevel_Debug << *l_sSelectionUri;
	m_rKernelContext.getLogManager() << LogLevel_Debug << "]\n";

	if(this->isLocked()) return;

	CIdentifier l_oBoxIdentifier;
	CIdentifier l_oBoxAlgorithmClassIdentifier = OV_UndefinedIdentifier;
	if(l_sSelectionText!=NULL)
		l_oBoxAlgorithmClassIdentifier.fromString(l_sSelectionText);

	if(l_oBoxAlgorithmClassIdentifier!=OV_UndefinedIdentifier)
	{
		m_rScenario.addBox(l_oBoxIdentifier, l_oBoxAlgorithmClassIdentifier, OV_UndefinedIdentifier);

		IBox* l_pBox = m_rScenario.getBoxDetails(l_oBoxIdentifier);
		CIdentifier l_oId = l_pBox->getAlgorithmClassIdentifier();
		const IPluginObjectDesc* l_pPOD = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_oId);

		m_vCurrentObject.clear();
		m_vCurrentObject[l_oBoxIdentifier]=true;

		// If a visualisation box was dropped, add it in window manager
		if((l_pPOD && l_pPOD->hasFunctionality(Kernel::PluginFunctionality_Visualization))||(l_pBox->hasModifiableSettings()))
		{
			// Let window manager know about new box
			m_pDesignerVisualisation->onVisualisationBoxAdded(l_pBox);

		}

		CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, l_oBoxIdentifier);
		l_oBoxProxy.setCenter((iX-m_i32ViewOffsetX) / m_f64CurrentScale, (iY-m_i32ViewOffsetY) / m_f64CurrentScale);
		alignBoxOnGrid(l_oBoxProxy);

		// Applies modifications before snapshot
		l_oBoxProxy.apply();
		m_bScenarioModified = true;

		this->snapshotCB();
		//We need to grab the focus to enable shortcuts like F1
		gtk_widget_grab_focus(GTK_WIDGET(m_pScenarioDrawingArea));
	}
	else
	{
		m_rKernelContext.getLogManager() << LogLevel_Debug << "not a box, might be a scenario\n";
		std::string l_sFilename;
		if(l_sSelectionText!=NULL)
		{
			l_sFilename = l_sSelectionText;
		}
		else if (*l_sSelectionUri!=NULL)
		{
			l_sFilename = *l_sSelectionUri;
		}
		m_rKernelContext.getLogManager() << LogLevel_Trace <<  "Drag and drop gave URL [" << l_sFilename.c_str() << "]\n";
		l_sFilename.erase(l_sFilename.begin(), l_sFilename.begin()+7);//erase the file:// at the begining
#if defined TARGET_OS_Windows
		l_sFilename.erase(l_sFilename.begin(), l_sFilename.begin()+1);//we have an additionnal / to erase before the uri starts
#endif
		// Get rid of the %20 sequences etc
		l_sFilename = ::decodeUrl(l_sFilename);
		// Remove possible linefeed in the end
		while(l_sFilename.length()>0 && (l_sFilename[l_sFilename.length()-1] == '\n' || l_sFilename[l_sFilename.length()-1] == '\r'))
		{
			l_sFilename = l_sFilename.substr(0,l_sFilename.length()-1);
		}
		// l_sFilename.erase(l_sFilename.begin()+l_sFilename.find(".xml")+4, l_sFilename.end());//erase \n at the end

		m_rKernelContext.getLogManager() << LogLevel_Trace <<  "Parsed out [" << l_sFilename.c_str() << "]\n";
		m_rApplication.openScenario(l_sFilename.c_str());
	}

	m_f64CurrentMouseX=iX;
	m_f64CurrentMouseY=iY;
}



void CInterfacedScenario::scenarioDrawingAreaMotionNotifyCB(::GtkWidget* pWidget, ::GdkEventMotion* pEvent)
{
	//m_rKernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaMotionNotifyCB\n";

	if(this->isLocked()) return;

	const uint32 l_ui32InterfacedObjectId=pickInterfacedObject((int)pEvent->x, (int)pEvent->y);
	CInterfacedObject& l_rObject=m_vInterfacedObject[l_ui32InterfacedObjectId];
	if(l_rObject.m_oIdentifier!=OV_UndefinedIdentifier
	&& l_rObject.m_ui32ConnectorType!=Connector_Link
	&& l_rObject.m_ui32ConnectorType!=Connector_None)
	{
		IBox* l_pBoxDetails=m_rScenario.getBoxDetails(l_rObject.m_oIdentifier);
		if(l_pBoxDetails)
		{
			CString l_sName;
			CString l_sType;
			if(l_rObject.m_ui32ConnectorType==Connector_Input)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getInputName(l_rObject.m_ui32ConnectorIndex, l_sName);
				l_pBoxDetails->getInputType(l_rObject.m_ui32ConnectorIndex, l_oType);
				l_sType=m_rKernelContext.getTypeManager().getTypeName(l_oType);
			}
			if(l_rObject.m_ui32ConnectorType==Connector_Output)
			{
				CIdentifier l_oType;
				l_pBoxDetails->getOutputName(l_rObject.m_ui32ConnectorIndex, l_sName);
				l_pBoxDetails->getOutputType(l_rObject.m_ui32ConnectorIndex, l_oType);
				l_sType=m_rKernelContext.getTypeManager().getTypeName(l_oType);
			}
			if(l_rObject.m_ui32ConnectorType==Connector_MessageInput)
			{
				CIdentifier l_oType = OV_TypeId_Message;
				l_pBoxDetails->getMessageInputName(l_rObject.m_ui32ConnectorIndex, l_sName);
				l_sType=m_rKernelContext.getTypeManager().getTypeName(l_oType);
			}
			if(l_rObject.m_ui32ConnectorType==Connector_MessageOutput)
			{
				CIdentifier l_oType = OV_TypeId_Message;
				l_pBoxDetails->getMessageOutputName(l_rObject.m_ui32ConnectorIndex, l_sName);
				l_sType=m_rKernelContext.getTypeManager().getTypeName(l_oType);
			}
			l_sType=CString("[")+l_sType+CString("]");
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(m_pBuilderTooltip, "tooltip-label_name_content")), l_sName);
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(m_pBuilderTooltip, "tooltip-label_type_content")), l_sType);
			gtk_window_move(GTK_WINDOW(m_pTooltip), (gint)pEvent->x_root, (gint)pEvent->y_root+40);
			gtk_widget_show(m_pTooltip);
		}
	}
	else
	{
		gtk_widget_hide(m_pTooltip);
	}

	if(m_ui32CurrentMode!=Mode_None)
	{
		if(m_ui32CurrentMode==Mode_MoveScenario)
		{
			m_i32ViewOffsetX+=(int32)(pEvent->x-m_f64CurrentMouseX);
			m_i32ViewOffsetY+=(int32)(pEvent->y-m_f64CurrentMouseY);
			m_bScenarioModified = true;
		}
		if(m_ui32CurrentMode==Mode_MoveSelection)
		{
			if(m_bControlPressed)
			{
				m_vCurrentObject[m_oCurrentObject.m_oIdentifier]=true;
			}
			else
			{
				if(!m_vCurrentObject[m_oCurrentObject.m_oIdentifier])
				{
					m_vCurrentObject.clear();
					m_vCurrentObject[m_oCurrentObject.m_oIdentifier]=true;
				}
			}
			map<CIdentifier, boolean>::const_iterator i;
			for(i=m_vCurrentObject.begin(); i!=m_vCurrentObject.end(); i++)
			{
				if(i->second && m_rScenario.isBox(i->first))
				{
					CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, i->first);
					l_oBoxProxy.setCenter(
						l_oBoxProxy.getXCenter()+ (pEvent->x-m_f64CurrentMouseX) / m_f64CurrentScale,
						l_oBoxProxy.getYCenter()+ (pEvent->y-m_f64CurrentMouseY) / m_f64CurrentScale);
				}
				if(i->second && m_rScenario.isComment(i->first))
				{
					CCommentProxy l_oCommentProxy(m_rKernelContext, m_rScenario, i->first);
					l_oCommentProxy.setCenter(
						l_oCommentProxy.getXCenter()+ (pEvent->x-m_f64CurrentMouseX) / m_f64CurrentScale,
						l_oCommentProxy.getYCenter()+ (pEvent->y-m_f64CurrentMouseY) / m_f64CurrentScale);
				}
			}
			m_bScenarioModified = true;
		}
		this->redraw();
	}
	m_f64CurrentMouseX=pEvent->x;
	m_f64CurrentMouseY=pEvent->y;
}
void CInterfacedScenario::scenarioDrawingAreaButtonPressedCB(::GtkWidget* pWidget, ::GdkEventButton* pEvent)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaButtonPressedCB\n";
	gtk_widget_grab_focus(pWidget);

	if(this->isLocked()) return;

	::GtkWidget* l_pTooltip=GTK_WIDGET(gtk_builder_get_object(m_pBuilderTooltip, "tooltip"));
	gtk_widget_hide(l_pTooltip);


	m_bButtonPressed|=((pEvent->type==GDK_BUTTON_PRESS)&&(pEvent->button==1));
	m_f64PressMouseX=pEvent->x;
	m_f64PressMouseY=pEvent->y;

	m_f64CurrentMouseX=pEvent->x;
	m_f64CurrentMouseY=pEvent->y;

	uint32 l_ui32InterfacedObjectId=pickInterfacedObject((int)m_f64PressMouseX, (int)m_f64PressMouseY);
	m_oCurrentObject=m_vInterfacedObject[l_ui32InterfacedObjectId];

	switch(pEvent->button)
	{
		case 1:
			m_bScenarioModified = true;
			switch(pEvent->type)
			{
				case GDK_BUTTON_PRESS:
					if(m_oCurrentObject.m_oIdentifier==OV_UndefinedIdentifier)
					{
						if(m_bShiftPressed)
						{
							m_ui32CurrentMode=Mode_MoveScenario;
						}
						else
						{
							if(m_bControlPressed)
							{
								m_ui32CurrentMode=Mode_SelectionAdd;
							}
							else
							{
								m_ui32CurrentMode=Mode_Selection;
							}
							m_bScenarioModified = false;
						}
						//In this case we don't have to modify something in the scenario
					}
					else
					{
						if(m_oCurrentObject.m_ui32ConnectorType==Connector_Input || m_oCurrentObject.m_ui32ConnectorType==Connector_Output || m_oCurrentObject.m_ui32ConnectorType==Connector_MessageOutput || m_oCurrentObject.m_ui32ConnectorType==Connector_MessageInput)
						{
							m_ui32CurrentMode=Mode_Connect;
						}
						else
						{
							if(m_bControlPressed)
							{
								m_vCurrentObject[m_oCurrentObject.m_oIdentifier]=!m_vCurrentObject[m_oCurrentObject.m_oIdentifier];
							}
							else
							{
								if(!m_vCurrentObject[m_oCurrentObject.m_oIdentifier])
								{
									m_vCurrentObject.clear();
									m_vCurrentObject[m_oCurrentObject.m_oIdentifier]=true;
								}
							}
							m_ui32CurrentMode=Mode_MoveSelection;
						}
					}

					break;
				case GDK_2BUTTON_PRESS:
					if(m_oCurrentObject.m_oIdentifier!=OV_UndefinedIdentifier)
					{

						m_ui32CurrentMode=Mode_EditSettings;
						m_bShiftPressed=false;
						m_bControlPressed=false;
						m_bAltPressed=false;
						m_bAPressed=false;
						m_bWPressed=false;

						if(m_oCurrentObject.m_ui32ConnectorType==Connector_Input || m_oCurrentObject.m_ui32ConnectorType==Connector_Output)
						{
							IBox* l_pBox=m_rScenario.getBoxDetails(m_oCurrentObject.m_oIdentifier);
							if(l_pBox)
							{
								if((m_oCurrentObject.m_ui32ConnectorType==Connector_Input  && l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput))
								|| (m_oCurrentObject.m_ui32ConnectorType==Connector_Output && l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput)))
								{
									CConnectorEditor l_oConnectorEditor(m_rKernelContext, *l_pBox, m_oCurrentObject.m_ui32ConnectorType, m_oCurrentObject.m_ui32ConnectorIndex, m_oCurrentObject.m_ui32ConnectorType==Connector_Input?"Edit Input":"Edit Output", m_sGUIFilename.c_str());
									if(l_oConnectorEditor.run())
									{
										this->snapshotCB();
										m_bScenarioModified = true;
									}
								}
							}
						}//double clicking on a message in/output
						else if(m_oCurrentObject.m_ui32ConnectorType==Connector_MessageInput || m_oCurrentObject.m_ui32ConnectorType==Connector_MessageOutput)
						{
							IBox* l_pBox=m_rScenario.getBoxDetails(m_oCurrentObject.m_oIdentifier);
							if(l_pBox)
							{
								if((m_oCurrentObject.m_ui32ConnectorType==Connector_MessageInput  && l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyMessageInput))
								|| (m_oCurrentObject.m_ui32ConnectorType==Connector_MessageOutput && l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyMessageOutput)))
								{
									CString l_sMessageSocketName;
									if (m_oCurrentObject.m_ui32ConnectorType==Connector_MessageInput)
									{
										l_pBox->getMessageInputName(m_oCurrentObject.m_ui32ConnectorIndex, l_sMessageSocketName);
									}
									if (m_oCurrentObject.m_ui32ConnectorType==Connector_MessageOutput)
									{
										l_pBox->getMessageOutputName(m_oCurrentObject.m_ui32ConnectorIndex, l_sMessageSocketName);
									}

									//m_rKernelContext.getLogManager() << LogLevel_Fatal << "box connector " << m_oCurrentObject.m_ui32ConnectorIndex <<  " message socket name " << l_sMessageSocketName <<"\n";

									CRenameDialog l_oRename(m_rKernelContext, l_sMessageSocketName , l_sMessageSocketName, m_sGUIFilename.c_str());
									if(l_oRename.run())
									{
										if (m_oCurrentObject.m_ui32ConnectorType==Connector_MessageInput)
												l_pBox->setMessageInputName(m_oCurrentObject.m_ui32ConnectorIndex, l_oRename.getResult());
										if (m_oCurrentObject.m_ui32ConnectorType==Connector_MessageOutput)
											l_pBox->setMessageOutputName(m_oCurrentObject.m_ui32ConnectorIndex, l_oRename.getResult());

										this->snapshotCB();
									}
								}
							}
						}
						else
						{
							if(m_rScenario.isBox(m_oCurrentObject.m_oIdentifier))
							{
								IBox* l_pBox=m_rScenario.getBoxDetails(m_oCurrentObject.m_oIdentifier);
								if(l_pBox)
								{
									CBoxConfigurationDialog l_oBoxConfigurationDialog(m_rKernelContext, *l_pBox, m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str(), false);
									if(l_oBoxConfigurationDialog.run(false))//modUI
									{
										m_bScenarioModified = true;
										this->snapshotCB();
									}
								}
							}
							else if(m_rScenario.isComment(m_oCurrentObject.m_oIdentifier))
							{
								IComment* l_pComment=m_rScenario.getCommentDetails(m_oCurrentObject.m_oIdentifier);
								if(l_pComment)
								{
									CCommentEditorDialog l_oCommentEditorDialog(m_rKernelContext, *l_pComment, m_sGUIFilename.c_str());
									if(l_oCommentEditorDialog.run())
									{
										this->snapshotCB();
										m_bScenarioModified = true;
									}
								}
							}

						}
					}
					else
					{
						m_bScenarioModified = false;
					}
					break;
				default:
					m_bScenarioModified = false;
					break;
			}
			break;

		case 2:
			break;

		case 3:
			m_bScenarioModified = true;
			switch(pEvent->type)
			{
				case GDK_BUTTON_PRESS:
					{
						::GtkMenu* l_pMenu=GTK_MENU(gtk_menu_new());
						m_vBoxContextMenuCB.clear();

						#define gtk_menu_add_new_image_menu_item(menu, menuitem, icon, label) \
							::GtkImageMenuItem* menuitem=NULL; \
							{ \
								menuitem=GTK_IMAGE_MENU_ITEM(gtk_image_menu_item_new_with_label(label)); \
								gtk_image_menu_item_set_image(menuitem, gtk_image_new_from_stock(icon, GTK_ICON_SIZE_MENU)); \
								gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(menuitem)); \
							}
						#define gtk_menu_add_new_image_menu_item_with_cb(menu, menuitem, icon, label, cb, cb_box, cb_command, cb_index) \
							gtk_menu_add_new_image_menu_item(menu, menuitem, icon, label); \
							{ \
								CInterfacedScenario::BoxContextMenuCB l_oBoxContextMenuCB; \
								l_oBoxContextMenuCB.ui32Command=cb_command; \
								l_oBoxContextMenuCB.ui32Index=cb_index; \
								l_oBoxContextMenuCB.pBox=cb_box; \
								l_oBoxContextMenuCB.pInterfacedScenario=this; \
								uint32 l_ui32MapIndex=m_vBoxContextMenuCB.size(); \
								m_vBoxContextMenuCB[l_ui32MapIndex]=l_oBoxContextMenuCB; \
								g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(cb), &m_vBoxContextMenuCB[l_ui32MapIndex]); \
							}
						#define gtk_menu_add_new_image_menu_item_with_cb_condition(condition, menu, menuitem, icon, label, cb, cb_box, cb_command, cb_index) \
							if(condition) \
							{ \
								gtk_menu_add_new_image_menu_item_with_cb(menu, menuitem, icon, label, cb, cb_box, cb_command, cb_index); \
							}
						#define gtk_menu_add_separator_menu_item(menu) \
							{ \
								::GtkSeparatorMenuItem* menuitem=GTK_SEPARATOR_MENU_ITEM(gtk_separator_menu_item_new()); \
								gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(menuitem)); \
							}
						#define gtk_menu_add_separator_menu_item_with_condition(condition, menu) \
							if(condition) \
							{ \
								::GtkSeparatorMenuItem* menuitem=GTK_SEPARATOR_MENU_ITEM(gtk_separator_menu_item_new()); \
								gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(menuitem)); \
							}

						// -------------- SELECTION -----------

						if(this->hasSelection()) { gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemSelectionCut, GTK_STOCK_CUT, "cut...", context_menu_cb, NULL, ContextMenu_SelectionCut, -1); }
						if(this->hasSelection()) { gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemSelectionCopy, GTK_STOCK_COPY, "copy...", context_menu_cb, NULL, ContextMenu_SelectionCopy, -1); }
						if(m_rApplication.m_pClipboardScenario->getNextBoxIdentifier(OV_UndefinedIdentifier)!=OV_UndefinedIdentifier) { gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemSelectionPaste, GTK_STOCK_PASTE, "paste...", context_menu_cb, NULL, ContextMenu_SelectionPaste, -1); }
						if(this->hasSelection()) { gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemSelectionCut, GTK_STOCK_DELETE, "delete...", context_menu_cb, NULL, ContextMenu_SelectionDelete, -1); }
						if(this->hasSelection()) { gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemSelectionCut, GTK_STOCK_CUT, "mute toggle...", context_menu_cb, NULL, ContextMenu_SelectionMute, -1); }

						if(m_oCurrentObject.m_oIdentifier!=OV_UndefinedIdentifier && m_rScenario.isBox(m_oCurrentObject.m_oIdentifier))
						{
							IBox* l_pBox=m_rScenario.getBoxDetails(m_oCurrentObject.m_oIdentifier);
							if(l_pBox)
							{
								uint32 i;
								char l_sCompleteName[1024];

								if (!this->hasSelection())
								{
									gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemRename, GTK_STOCK_EDIT, "mute toggle...", context_menu_cb, l_pBox, ContextMenu_BoxMute, -1);
								}

								gtk_menu_add_separator_menu_item_with_condition(m_vBoxContextMenuCB.size()!=0, l_pMenu);

								// -------------- INPUTS --------------

								const boolean l_bFlagCanAddInput=l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddInput);
								const boolean l_bFlagCanModifyInput=l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyInput);
								if(l_bFlagCanAddInput || l_bFlagCanModifyInput)
								{
									uint32 l_ui32FixedInputCount=0;
									::sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialInputCount).toASCIIString(), "%d", &l_ui32FixedInputCount);
									::GtkMenu* l_pMenuInput=GTK_MENU(gtk_menu_new());
									gtk_menu_add_new_image_menu_item(l_pMenu, l_pMenuItemInput, GTK_STOCK_PROPERTIES, "modify inputs");
									for(i=0; i<l_pBox->getInputCount(); i++)
									{
										CString l_sName;
										CIdentifier l_oType;
										l_pBox->getInputName(i, l_sName);
										l_pBox->getInputType(i, l_oType);
										sprintf(l_sCompleteName, "%i : %s", (int)i+1, l_sName.toASCIIString());
										gtk_menu_add_new_image_menu_item(l_pMenuInput, l_pMenuInputMenuItem, GTK_STOCK_PROPERTIES, l_sCompleteName);

										if(l_bFlagCanModifyInput || l_ui32FixedInputCount <= i)
										{
											::GtkMenu* l_pMenuInputMenuAction=GTK_MENU(gtk_menu_new());
											gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanModifyInput, l_pMenuInputMenuAction, l_pMenuInputInputMenuItemConfigure, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox, ContextMenu_BoxEditInput, i);
											gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanAddInput && l_ui32FixedInputCount <= i, l_pMenuInputMenuAction, l_pMenuInputInputMenuItemRemove, GTK_STOCK_REMOVE, "delete...", context_menu_cb, l_pBox, ContextMenu_BoxRemoveInput, i);
											gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuInputMenuItem), GTK_WIDGET(l_pMenuInputMenuAction));
										}
										else
										{
											gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuInputMenuItem), false);
										}
									}
									gtk_menu_add_separator_menu_item(l_pMenuInput);
									gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanAddInput, l_pMenuInput, l_pMenuInputMenuItemAdd, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddInput, -1);
									gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemInput), GTK_WIDGET(l_pMenuInput));
								}

								// -------------- MESSAGE INPUTS -----------------
								const boolean l_bFlagCanAddMessageInput=l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddMessageInput);
								const boolean l_bFlagCanModifyMessageInput=l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyMessageInput);
								//m_rKernelContext.getLogManager() << LogLevel_Fatal << "box "<< l_pBox->getName() << " can add message input " << l_bFlagCanAddMessageInput    <<"\n";
								//m_rKernelContext.getLogManager() << LogLevel_Fatal << "box "<< l_pBox->getName() << " can Modify message input " << l_bFlagCanModifyMessageInput    <<"\n";

								if(l_bFlagCanAddMessageInput || l_bFlagCanModifyMessageInput)
								{
									uint32 l_ui32FixedMessageInputCount=0;
									::sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialMessageInputCount).toASCIIString(), "%d", &l_ui32FixedMessageInputCount);
									::GtkMenu* l_pMenuInput=GTK_MENU(gtk_menu_new());
									gtk_menu_add_new_image_menu_item(l_pMenu, l_pMenuItemInput, GTK_STOCK_PROPERTIES, "modify Message inputs");
									for(i=0; i<l_pBox->getMessageInputCount(); i++)
									{
										CString l_sName;
										//CIdentifier l_oType;
										l_pBox->getMessageInputName(i, l_sName);
										//l_pBox->getMessageInputType(i, l_oType);
										sprintf(l_sCompleteName, "%i : %s", (int)i+1, l_sName.toASCIIString());
										gtk_menu_add_new_image_menu_item(l_pMenuInput, l_pMenuInputMenuItem, GTK_STOCK_PROPERTIES, l_sCompleteName);

										if(l_bFlagCanModifyMessageInput || l_ui32FixedMessageInputCount <= i)
										{
											::GtkMenu* l_pMenuInputMenuAction=GTK_MENU(gtk_menu_new());
											gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanModifyMessageInput, l_pMenuInputMenuAction, l_pMenuInputInputMenuItemConfigure, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox, ContextMenu_BoxEditMessageInput, i);
											gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanAddMessageInput && l_ui32FixedMessageInputCount <= i, l_pMenuInputMenuAction, l_pMenuInputInputMenuItemRemove, GTK_STOCK_REMOVE, "delete...", context_menu_cb, l_pBox, ContextMenu_BoxRemoveMessageInput, i);
											gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuInputMenuItem), GTK_WIDGET(l_pMenuInputMenuAction));
										}
										else
										{
											gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuInputMenuItem), false);
										}
									}
									gtk_menu_add_separator_menu_item(l_pMenuInput);
									gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanAddMessageInput, l_pMenuInput, l_pMenuInputMenuItemAdd, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddMessageInput, -1);
									gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemInput), GTK_WIDGET(l_pMenuInput));
								}

								// -------------- OUTPUTS --------------

								const boolean l_bFlagCanAddOutput=l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddOutput);
								const boolean l_bFlagCanModifyOutput=l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput);
								if(l_bFlagCanAddOutput || l_bFlagCanModifyOutput)
								{
									uint32 l_ui32FixedOutputCount=0;
									::sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialOutputCount).toASCIIString(), "%d", &l_ui32FixedOutputCount);
									gtk_menu_add_new_image_menu_item(l_pMenu, l_pMenuItemOutput, GTK_STOCK_PROPERTIES, "modify outputs");
									::GtkMenu* l_pMenuOutput=GTK_MENU(gtk_menu_new());
									for(i=0; i<l_pBox->getOutputCount(); i++)
									{
										CString l_sName;
										CIdentifier l_oType;
										l_pBox->getOutputName(i, l_sName);
										l_pBox->getOutputType(i, l_oType);
										sprintf(l_sCompleteName, "%i : %s", (int)i+1, l_sName.toASCIIString());
										gtk_menu_add_new_image_menu_item(l_pMenuOutput, l_pMenuOutputMenuItem, GTK_STOCK_PROPERTIES, l_sCompleteName);

										if(l_bFlagCanModifyOutput || l_ui32FixedOutputCount <= i)
										{
											::GtkMenu* l_pMenuOutputMenuAction=GTK_MENU(gtk_menu_new());
											gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanModifyOutput, l_pMenuOutputMenuAction, l_pMenuOutputInputMenuItemConfigure, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox, ContextMenu_BoxEditOutput, i);
											gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanAddOutput && l_ui32FixedOutputCount <= i, l_pMenuOutputMenuAction, l_pMenuOutputInputMenuItemRemove, GTK_STOCK_REMOVE, "delete...", context_menu_cb, l_pBox, ContextMenu_BoxRemoveOutput, i);
											gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuOutputMenuItem), GTK_WIDGET(l_pMenuOutputMenuAction));
										}
										else
										{
											gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuOutputMenuItem), false);
										}
									}
									gtk_menu_add_separator_menu_item(l_pMenuOutput);
									gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanAddOutput, l_pMenuOutput, l_pMenuOutputMenuItemAdd, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddOutput, -1);
									gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemOutput), GTK_WIDGET(l_pMenuOutput));
								}

								// -------------- MESSAGE OUTPUTS -----------------
								const boolean l_bFlagCanAddMessageOutput=l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddMessageOutput);
								const boolean l_bFlagCanModifyMessageOutput=l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifyMessageOutput);
								//m_rKernelContext.getLogManager() << LogLevel_Fatal << "box "<< l_pBox->getName() << " can add message Output " << l_bFlagCanAddMessageOutput    <<"\n";
								//m_rKernelContext.getLogManager() << LogLevel_Fatal << "box "<< l_pBox->getName() << " can Modify message Output " << l_bFlagCanModifyMessageOutput    <<"\n";

								if(l_bFlagCanAddMessageOutput || l_bFlagCanModifyMessageOutput)
								{
									uint32 l_ui32FixedMessageOutputCount=0;
									::sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialMessageOutputCount).toASCIIString(), "%d", &l_ui32FixedMessageOutputCount);
									::GtkMenu* l_pMenuOutput=GTK_MENU(gtk_menu_new());
									gtk_menu_add_new_image_menu_item(l_pMenu, l_pMenuItemOutput, GTK_STOCK_PROPERTIES, "modify Message Outputs");
									for(i=0; i<l_pBox->getMessageOutputCount(); i++)
									{
										CString l_sName;

										l_pBox->getMessageOutputName(i, l_sName);

										sprintf(l_sCompleteName, "%i : %s", (int)i+1, l_sName.toASCIIString());
										gtk_menu_add_new_image_menu_item(l_pMenuOutput, l_pMenuOutputMenuItem, GTK_STOCK_PROPERTIES, l_sCompleteName);

										if(l_bFlagCanModifyMessageOutput || l_ui32FixedMessageOutputCount <= i)
										{
											::GtkMenu* l_pMenuOutputMenuAction=GTK_MENU(gtk_menu_new());
											gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanModifyMessageOutput, l_pMenuOutputMenuAction, l_pMenuInputInputMenuItemConfigure, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox, ContextMenu_BoxEditMessageOutput, i);
											gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanAddMessageOutput && l_ui32FixedMessageOutputCount <= i, l_pMenuOutputMenuAction, l_pMenuInputOutputMenuItemRemove, GTK_STOCK_REMOVE, "delete...", context_menu_cb, l_pBox, ContextMenu_BoxRemoveMessageOutput, i);
											gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuOutputMenuItem), GTK_WIDGET(l_pMenuOutputMenuAction));
										}
										else
										{
											gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuOutputMenuItem), false);
										}
									}
									gtk_menu_add_separator_menu_item(l_pMenuOutput);
									gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanAddMessageOutput, l_pMenuOutput, l_pMenuOutputMenuItemAdd, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddMessageOutput, -1);
									gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemOutput), GTK_WIDGET(l_pMenuOutput));
								}

								// -------------- SETTINGS --------------

								const boolean l_bFlagCanAddSetting=l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanAddSetting);
								const boolean l_bFlagCanModifySetting=l_pBox->hasAttribute(OV_AttributeId_Box_FlagCanModifySetting);
								if(l_bFlagCanAddSetting || l_bFlagCanModifySetting)
								{
									uint32 l_ui32FixedSettingCount=0;
									::sscanf(l_pBox->getAttributeValue(OV_AttributeId_Box_InitialSettingCount).toASCIIString(), "%d", &l_ui32FixedSettingCount);
									gtk_menu_add_new_image_menu_item(l_pMenu, l_pMenuItemSetting, GTK_STOCK_PROPERTIES, "modify settings");
									::GtkMenu* l_pMenuSetting=GTK_MENU(gtk_menu_new());
									for(i=0; i<l_pBox->getSettingCount(); i++)
									{
										CString l_sName;
										CIdentifier l_oType;
										l_pBox->getSettingName(i, l_sName);
										l_pBox->getSettingType(i, l_oType);
										sprintf(l_sCompleteName, "%i : %s", (int)i+1, l_sName.toASCIIString());
										gtk_menu_add_new_image_menu_item(l_pMenuSetting, l_pMenuSettingMenuItem, GTK_STOCK_PROPERTIES, l_sCompleteName);

										if(l_bFlagCanModifySetting || l_ui32FixedSettingCount <= i)
										{
											::GtkMenu* l_pMenuSettingMenuAction=GTK_MENU(gtk_menu_new());
											gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanModifySetting, l_pMenuSettingMenuAction, l_pMenuSettingInputMenuItemConfigure, GTK_STOCK_EDIT, "configure...", context_menu_cb, l_pBox, ContextMenu_BoxEditSetting, i);
											gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanAddSetting && l_ui32FixedSettingCount <= i, l_pMenuSettingMenuAction, l_pMenuSettingInputMenuItemRemove, GTK_STOCK_REMOVE, "delete...", context_menu_cb, l_pBox, ContextMenu_BoxRemoveSetting, i);
											gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuSettingMenuItem), GTK_WIDGET(l_pMenuSettingMenuAction));
										}
										else
										{
											gtk_widget_set_sensitive(GTK_WIDGET(l_pMenuSettingMenuItem), false);
										}
									}
									gtk_menu_add_separator_menu_item(l_pMenuSetting);
									gtk_menu_add_new_image_menu_item_with_cb_condition(l_bFlagCanAddSetting, l_pMenuSetting, l_pMenuSettingMenuItemAdd, GTK_STOCK_ADD, "new...", context_menu_cb, l_pBox, ContextMenu_BoxAddSetting, -1);
									gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemSetting), GTK_WIDGET(l_pMenuSetting));
								}

								// -------------- PROCESSING UNIT --------------
#if 0
								gtk_menu_add_new_image_menu_item(l_pMenu, l_pMenuItemProcessUnit, GTK_STOCK_EXECUTE, "process unit", NULL);
								::GtkMenu* l_pMenuProcessingUnit=GTK_MENU(gtk_menu_new());
								gtk_menu_add_new_image_menu_item(l_pMenuProcessingUnit, l_pMenuProcessingUnitDefault, GTK_STOCK_HOME, "default", NULL);
								gtk_menu_add_separator_menu_item(l_pMenuProcessingUnit);
								gtk_menu_add_new_image_menu_item(l_pMenuProcessingUnit, l_pMenuProcessingUnitAdd, GTK_STOCK_ADD, "new...", NULL);
								gtk_menu_item_set_submenu(GTK_MENU_ITEM(l_pMenuItemProcessUnit), GTK_WIDGET(l_pMenuProcessingUnit));
#endif
								// -------------- ABOUT / RENAME --------------

								gtk_menu_add_separator_menu_item_with_condition(m_vBoxContextMenuCB.size()!=0, l_pMenu);
								if(l_pBox->getSettingCount()!=0)
								{
									gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemEdit, GTK_STOCK_EDIT, "configure box...", context_menu_cb, l_pBox, ContextMenu_BoxConfigure, -1);
								}
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemRename, GTK_STOCK_EDIT, "rename box...", context_menu_cb, l_pBox, ContextMenu_BoxRename, -1);
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemDelete, GTK_STOCK_CUT, "delete box...", context_menu_cb, l_pBox, ContextMenu_BoxDelete, -1);
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemAbout, GTK_STOCK_ABOUT, "about box...", context_menu_cb, l_pBox, ContextMenu_BoxAbout, -1);
								gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemDocumentation, GTK_STOCK_ABOUT, "box documentation...", context_menu_cb, l_pBox, ContextMenu_BoxDocumentation, -1);
							}
						}

						gtk_menu_add_new_image_menu_item_with_cb(l_pMenu, l_pMenuItemScenarioAbout, GTK_STOCK_ABOUT, "about scenario...", context_menu_cb, NULL, ContextMenu_ScenarioAbout, -1);

						// -------------- RUN --------------

						#undef gtk_menu_add_separator_menu_item
						#undef gtk_menu_add_new_image_menu_item_with_cb
						#undef gtk_menu_add_new_image_menu_item

						gtk_widget_show_all(GTK_WIDGET(l_pMenu));
						gtk_menu_popup(l_pMenu, NULL, NULL, NULL, NULL, 3, pEvent->time);
						if(m_vBoxContextMenuCB.size()==0)
						{
							gtk_menu_popdown(l_pMenu);
						}
					}
					break;
				default:
					break;
			}
			break;

		default:
			break;
	}
	this->redraw();
}
void CInterfacedScenario::scenarioDrawingAreaButtonReleasedCB(::GtkWidget* pWidget, ::GdkEventButton* pEvent)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "scenarioDrawingAreaButtonReleasedCB\n";
	if(this->isLocked()) return;

	if(pEvent->button == 3)
	{
		return;
	}

	m_bButtonPressed&=!((pEvent->type==GDK_BUTTON_RELEASE)&&(pEvent->button==1));
	m_f64ReleaseMouseX=pEvent->x;
	m_f64ReleaseMouseY=pEvent->y;

	if(m_ui32CurrentMode!=Mode_None)
	{

		const int l_iStartX=(int)min(m_f64PressMouseX, m_f64CurrentMouseX);
		const int l_iStartY=(int)min(m_f64PressMouseY, m_f64CurrentMouseY);
		const int l_iSizeX=(int)max(m_f64PressMouseX-m_f64CurrentMouseX, m_f64CurrentMouseX-m_f64PressMouseX);
		const int l_iSizeY=(int)max(m_f64PressMouseY-m_f64CurrentMouseY, m_f64CurrentMouseY-m_f64PressMouseY);

		if(m_ui32CurrentMode==Mode_Selection || m_ui32CurrentMode==Mode_SelectionAdd)
		{
			bool l_bEmpty = true;
			map < CIdentifier, boolean >::const_iterator it;
			if(m_ui32CurrentMode==Mode_Selection)
			{
				for(it=m_vCurrentObject.begin(); it!=m_vCurrentObject.end(); it++)
				{
					if(it->second)
					{
						l_bEmpty = false;
					}
				}
				if(!l_bEmpty)
				{
					m_vCurrentObject.clear();
					m_bScenarioModified = true;
				}
			}
			if(pickInterfacedObject(l_iStartX, l_iStartY, l_iSizeX, l_iSizeY))
			{
				m_bScenarioModified = true;
			}
		}
		if(m_ui32CurrentMode==Mode_Connect)
		{
			boolean l_bIsActuallyConnecting=false;
			boolean l_bConnectionIsMessage=false;
			const uint32 l_ui32InterfacedObjectId=pickInterfacedObject((int)m_f64ReleaseMouseX, (int)m_f64ReleaseMouseY);
			CInterfacedObject l_oCurrentObject=m_vInterfacedObject[l_ui32InterfacedObjectId];
			CInterfacedObject l_oSourceObject;
			CInterfacedObject l_oTargetObject;
			if(l_oCurrentObject.m_ui32ConnectorType==Connector_Output && m_oCurrentObject.m_ui32ConnectorType==Connector_Input)
			{
				l_oSourceObject=l_oCurrentObject;
				l_oTargetObject=m_oCurrentObject;
				l_bIsActuallyConnecting=true;
				m_bScenarioModified = true;
			}
			if(l_oCurrentObject.m_ui32ConnectorType==Connector_Input && m_oCurrentObject.m_ui32ConnectorType==Connector_Output)
			{
				l_oSourceObject=m_oCurrentObject;
				l_oTargetObject=l_oCurrentObject;
				l_bIsActuallyConnecting=true;
				m_bScenarioModified = true;
			}
			//
			if(l_oCurrentObject.m_ui32ConnectorType==Connector_MessageOutput && m_oCurrentObject.m_ui32ConnectorType==Connector_MessageInput)
			{
				l_oSourceObject=l_oCurrentObject;
				l_oTargetObject=m_oCurrentObject;
				l_bIsActuallyConnecting=true;
				l_bConnectionIsMessage = true;
				m_bScenarioModified = true;
			}
			if(l_oCurrentObject.m_ui32ConnectorType==Connector_MessageInput && m_oCurrentObject.m_ui32ConnectorType==Connector_MessageOutput)
			{
				l_oSourceObject=m_oCurrentObject;
				l_oTargetObject=l_oCurrentObject;
				l_bIsActuallyConnecting=true;
				l_bConnectionIsMessage = true;
				m_bScenarioModified = true;
			}
			//
			if(l_bIsActuallyConnecting)
			{
				CIdentifier l_oSourceTypeIdentifier;
				CIdentifier l_oTargetTypeIdentifier;
				const IBox* l_pSourceBox=m_rScenario.getBoxDetails(l_oSourceObject.m_oIdentifier);
				const IBox* l_pTargetBox=m_rScenario.getBoxDetails(l_oTargetObject.m_oIdentifier);
				if(l_pSourceBox && l_pTargetBox)
				{
					l_pSourceBox->getOutputType(l_oSourceObject.m_ui32ConnectorIndex, l_oSourceTypeIdentifier);
					l_pTargetBox->getInputType(l_oTargetObject.m_ui32ConnectorIndex, l_oTargetTypeIdentifier);
					if((m_rKernelContext.getTypeManager().isDerivedFromStream(l_oSourceTypeIdentifier, l_oTargetTypeIdentifier)
							|| m_rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_AllowUpCastConnection}", false))&&(!l_bConnectionIsMessage))
					{
						CIdentifier l_oLinkIdentifier;
						m_rScenario.connect(
							l_oLinkIdentifier,
							l_oSourceObject.m_oIdentifier,
							l_oSourceObject.m_ui32ConnectorIndex,
							l_oTargetObject.m_oIdentifier,
							l_oTargetObject.m_ui32ConnectorIndex,
							OV_UndefinedIdentifier);
						m_bScenarioModified = true;
						this->snapshotCB();
					}

					//or if it is a message
					else if (l_bConnectionIsMessage)
					{
						m_rKernelContext.getLogManager() << LogLevel_Debug << "connect message\n";
						CIdentifier l_oLinkIdentifier;
						m_rScenario.connectMessage(
							l_oLinkIdentifier,
							l_oSourceObject.m_oIdentifier,
							l_oSourceObject.m_ui32ConnectorIndex,
							l_oTargetObject.m_oIdentifier,
							l_oTargetObject.m_ui32ConnectorIndex,
							OV_UndefinedIdentifier);
						m_bScenarioModified = true;
						this->snapshotCB();
					}
					//
					else
					{
						m_rKernelContext.getLogManager() << LogLevel_Warning << "Invalid connection\n";
					}
				}
			}
		}
		if(m_ui32CurrentMode==Mode_MoveSelection)
		{
			m_bScenarioModified = true;
			//If we move the mouse since the initial pressed, we need to move the selection
			if(l_iSizeX!=0 && l_iSizeY!=0)
			{
				map<CIdentifier, boolean>::const_iterator i;
				for(i=m_vCurrentObject.begin(); i!=m_vCurrentObject.end(); i++)
				{
					if(i->second && m_rScenario.isBox(i->first))
					{
						CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, i->first);
						this->alignBoxOnGrid(l_oBoxProxy);
					}
					if(i->second && m_rScenario.isComment(i->first))
					{
						CCommentProxy l_oCommentProxy(m_rKernelContext, m_rScenario, i->first);
						this->alignCommentOnGrid(l_oCommentProxy);
					}
				}
				this->snapshotCB();
			}
		}

		this->redraw();
	}

	m_ui32CurrentMode=Mode_None;
}

int32 alignCoordinate(float64 f64Coordinate){
	int32 i32Coordinate = static_cast<int32>(f64Coordinate);
	if(i32Coordinate >= 0)
	{
		i32Coordinate = i32Coordinate + gridSize/2;
		i32Coordinate = i32Coordinate - i32Coordinate % gridSize;
	}
	else
	{
		i32Coordinate = i32Coordinate - gridSize/2;
		i32Coordinate = i32Coordinate - i32Coordinate % gridSize;
	}
	return i32Coordinate;
}

void CInterfacedScenario::alignBoxOnGrid(CBoxProxy& rProxy)
{
	rProxy.setCenter(alignCoordinate(rProxy.getXCenter()), alignCoordinate(rProxy.getYCenter()));
}

void CInterfacedScenario::alignCommentOnGrid(CCommentProxy& rProxy)
{
	rProxy.setCenter(alignCoordinate(rProxy.getXCenter()), alignCoordinate(rProxy.getYCenter()));
}

void CInterfacedScenario::scenarioDrawingAreaKeyPressEventCB(::GtkWidget* pWidget, ::GdkEventKey* pEvent)
{
	m_bShiftPressed  |=(pEvent->keyval==GDK_Shift_L   || pEvent->keyval==GDK_Shift_R);
	m_bControlPressed|=(pEvent->keyval==GDK_Control_L || pEvent->keyval==GDK_Control_R);
	m_bAltPressed    |=(pEvent->keyval==GDK_Alt_L     || pEvent->keyval==GDK_Alt_R);
	m_bAPressed      |=(pEvent->keyval==GDK_a         || pEvent->keyval==GDK_A);
	m_bWPressed      |=(pEvent->keyval==GDK_w         || pEvent->keyval==GDK_W);

	// m_rKernelContext.getLogManager() << LogLevel_Info << "Key pressed " << (uint32) pEvent->keyval << "\n";
/*
	if((pEvent->keyval==GDK_Z || pEvent->keyval==GDK_z) && m_bControlPressed)
	{
		this->undoCB();
	}

	if((pEvent->keyval==GDK_Y || pEvent->keyval==GDK_y) && m_bControlPressed)
	{
		this->redoCB();
	}
*/
	// CTRL+A = select all
	if(m_bAPressed && m_bControlPressed && !m_bShiftPressed && ! m_bAltPressed)
	{
		map < CIdentifier, boolean >::iterator it;
		for(it=m_vCurrentObject.begin(); it!=m_vCurrentObject.end(); it++)
		{
			it->second = true;
		}
		m_bScenarioModified = true;
		this->redraw();
	}

	//CTRL+W : close current scenario
	if(m_bWPressed && m_bControlPressed && !m_bShiftPressed && ! m_bAltPressed)
	{
		m_rApplication.closeScenarioCB(this);
		return;
	}

	if((pEvent->keyval==GDK_C || pEvent->keyval==GDK_c) && m_ui32CurrentMode==Mode_None)
	{
		gint iX=0;
		gint iY=0;
		::gdk_window_get_pointer(GTK_WIDGET(m_pScenarioDrawingArea)->window, &iX, &iY, NULL);

		this->addCommentCB(iX, iY);
	}

	if(pEvent->keyval==GDK_F12 && m_bShiftPressed)
	{
		CIdentifier l_oIdentifier;
		while((l_oIdentifier=m_rScenario.getNextBoxIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
		{
			IBox* l_pBox=m_rScenario.getBoxDetails(l_oIdentifier);
			CIdentifier l_oAlgorithmIdentifier=l_pBox->getAlgorithmClassIdentifier();
			CIdentifier l_oHashValue=m_rKernelContext.getPluginManager().getPluginObjectHashValue(l_oAlgorithmIdentifier);
			if(l_pBox->hasAttribute(OV_AttributeId_Box_InitialPrototypeHashValue))
			{
				l_pBox->setAttributeValue(OV_AttributeId_Box_InitialPrototypeHashValue, l_oHashValue.toString());
			}
			else
			{
				l_pBox->addAttribute(OV_AttributeId_Box_InitialPrototypeHashValue, l_oHashValue.toString());
			}
		}
		m_bScenarioModified = true;
		this->redraw();
		this->snapshotCB();
	}

	// F1 : browse documentation
	if(pEvent->keyval==GDK_F1)
	{
		map < CIdentifier, boolean > l_vSelectedBoxAlgorithm;
		map < CIdentifier, boolean >::const_iterator it;
		for(it=m_vCurrentObject.begin(); it!=m_vCurrentObject.end(); it++)
		{
			if(it->second)
			{
				if(m_rScenario.isBox(it->first))
				{
					l_vSelectedBoxAlgorithm[m_rScenario.getBoxDetails(it->first)->getAlgorithmClassIdentifier()]=true;
				}
			}
		}

		if(l_vSelectedBoxAlgorithm.size())
		{
			for(it=l_vSelectedBoxAlgorithm.begin(); it!=l_vSelectedBoxAlgorithm.end(); it++)
			{
				browseBoxDocumentation(it->first);
			}
		}
		else
		{
			CString l_sFullURL=m_rScenario.getAttributeValue(OV_AttributeId_Scenario_DocumentationPage);
			if(l_sFullURL!=CString(""))
			{
				browseURL(l_sFullURL);
			}
			else
			{
				m_rKernelContext.getLogManager() << LogLevel_Info << "The scenario does not define a documentation page.\n";
			}
		}
	}

	// F2 : rename all selected box(es)
	if(pEvent->keyval==GDK_F2)
	{
		contextMenuBoxRenameAllCB();
	}

	//Designer trim : arrow keys scrolling
	if((pEvent->keyval==GDK_Left)||(pEvent->keyval==GDK_Right))
	{
		//get the parameters of the current adjustement
		GtkAdjustment* l_pOldAdjustement = gtk_scrolled_window_get_hadjustment(m_pScrolledWindow);//gtk_viewport_get_vadjustment(m_pScenarioViewport);
		gdouble upper;
		gdouble lower;
		gdouble step;
		gdouble page;
		gdouble pagesize;
		gdouble value;
		g_object_get(l_pOldAdjustement, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value", &value, NULL);
//		//get the size of the current viewport
//		gint l_iViewportX = -1;
//		gint l_iViewportY = -1;
//		gdk_window_get_size(GTK_WIDGET(m_pScenarioViewport)->window, &l_iViewportX, &l_iViewportY);

//		//the upper bound of the adjustement is too big, we must substract the current size of the viewport
//		upper-=l_iViewportX;

		//crete a new adjustement with the correct value since we can not change the upper bound of the old adjustement
		GtkAdjustment* l_pAdjustement = (GtkAdjustment*)gtk_adjustment_new(value, lower, upper, step, page, pagesize);
		//shift + right or left keys gives the equivalent of page up page down
		if (m_bShiftPressed&&(pEvent->keyval==GDK_Left))
		{
			gtk_adjustment_set_value(l_pAdjustement, gtk_adjustment_get_value(l_pAdjustement)-page);
			gtk_scrolled_window_set_hadjustment(m_pScrolledWindow, l_pAdjustement);
		}
		else if (m_bShiftPressed&&(pEvent->keyval==GDK_Right))
		{
			gtk_adjustment_set_value(l_pAdjustement, gtk_adjustment_get_value(l_pAdjustement)+page);
			gtk_scrolled_window_set_hadjustment(m_pScrolledWindow, l_pAdjustement);
		}
		else if(pEvent->keyval==GDK_Left)
		{
			gtk_adjustment_set_value(l_pAdjustement, gtk_adjustment_get_value(l_pAdjustement)-step);
			gtk_scrolled_window_set_hadjustment(m_pScrolledWindow, l_pAdjustement);
		}
		else if(pEvent->keyval==GDK_Right)
		{
			gtk_adjustment_set_value(l_pAdjustement, gtk_adjustment_get_value(l_pAdjustement)+step);
			gtk_scrolled_window_set_hadjustment(m_pScrolledWindow, l_pAdjustement);
		}

	}

	if((pEvent->keyval==GDK_Up)||(pEvent->keyval==GDK_Down)||(pEvent->keyval==GDK_Page_Up)||(pEvent->keyval==GDK_Page_Down))
	{
		//get the parameters of the current adjustement
		GtkAdjustment* l_pOldAdjustement = gtk_scrolled_window_get_vadjustment(m_pScrolledWindow);//gtk_viewport_get_vadjustment(m_pScenarioViewport);
		gdouble upper;
		gdouble lower;
		gdouble step;
		gdouble page;
		gdouble pagesize;
		gdouble value;
		g_object_get(l_pOldAdjustement, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value", &value, NULL);
		//get the size of the current viewport
//		gint l_iViewportX = -1;
//		gint l_iViewportY = -1;
//		gdk_window_get_size(GTK_WIDGET(m_pScenarioViewport)->window, &l_iViewportX, &l_iViewportY);
//		//the upper bound of the adjustement is too big, we must substract the current size of the viewport
//		upper-=l_iViewportY;

		//crete a new adjustement with the correct value since we can not change the upper bound of the old adjustement
		GtkAdjustment* l_pAdjustement = (GtkAdjustment*)gtk_adjustment_new(value, lower, upper, step, page, pagesize);
		if(pEvent->keyval==GDK_Up)
		{
			gtk_adjustment_set_value(l_pAdjustement, gtk_adjustment_get_value(l_pAdjustement)-step);
			gtk_scrolled_window_set_vadjustment(m_pScrolledWindow, l_pAdjustement);
		}
		else if(pEvent->keyval==GDK_Down)
		{
			gtk_adjustment_set_value(l_pAdjustement, gtk_adjustment_get_value(l_pAdjustement)+step);
			gtk_scrolled_window_set_vadjustment(m_pScrolledWindow, l_pAdjustement);
		}
		else if(pEvent->keyval==GDK_Page_Up)
		{
			gtk_adjustment_set_value(l_pAdjustement, gtk_adjustment_get_value(l_pAdjustement)-page);
			gtk_scrolled_window_set_vadjustment(m_pScrolledWindow, l_pAdjustement);
		}
		else if(pEvent->keyval==GDK_Page_Down)
		{
			gtk_adjustment_set_value(l_pAdjustement, gtk_adjustment_get_value(l_pAdjustement)+page);
			gtk_scrolled_window_set_vadjustment(m_pScrolledWindow, l_pAdjustement);
		}
	}


	m_rKernelContext.getLogManager() << LogLevel_Debug
		<< "scenarioDrawingAreaKeyPressEventCB ("
		<< (m_bShiftPressed?"true":"false") << "|"
		<< (m_bControlPressed?"true":"false") << "|"
		<< (m_bAltPressed?"true":"false") << "|"
		<< (m_bAPressed?"true":"false") << "|"
		<< (m_bWPressed?"true":"false") << "|"
		<< ")\n";

	if(this->isLocked()) return;

	if(pEvent->keyval==GDK_Delete || pEvent->keyval==GDK_KP_Delete)
	{
		this->deleteSelection();
		m_bScenarioModified = true;
		this->redraw();
	}
}
void CInterfacedScenario::scenarioDrawingAreaKeyReleaseEventCB(::GtkWidget* pWidget, ::GdkEventKey* pEvent)
{
	m_bShiftPressed  &=!(pEvent->keyval==GDK_Shift_L   || pEvent->keyval==GDK_Shift_R);
	m_bControlPressed&=!(pEvent->keyval==GDK_Control_L || pEvent->keyval==GDK_Control_R);
	m_bAltPressed    &=!(pEvent->keyval==GDK_Alt_L     || pEvent->keyval==GDK_Alt_R);
	m_bAPressed      &=!(pEvent->keyval==GDK_A         || pEvent->keyval==GDK_a);
	m_bWPressed      &=!(pEvent->keyval==GDK_W         || pEvent->keyval==GDK_w);

	m_rKernelContext.getLogManager() << LogLevel_Debug
		<< "scenarioDrawingAreaKeyReleaseEventCB ("
		<< (m_bShiftPressed?"true":"false") << "|"
		<< (m_bControlPressed?"true":"false") << "|"
		<< (m_bAltPressed?"true":"false") << "|"
		<< (m_bAPressed?"true":"false") << "|"
		<< (m_bWPressed?"true":"false") << "|"
		<< ")\n";

	if(this->isLocked()) return;

	// ...
}

void CInterfacedScenario::scenarioDrawingAreaLeaveNotifyCB(::GtkWidget* pWidget, ::GdkEventKey* pEvent)
{
	gtk_widget_hide(m_pTooltip);
}

void CInterfacedScenario::scenarioDrawingAreaConfigureEventCB(GtkWidget *pWidget, GdkRectangle *pEvent)
{
	if(pEvent->width != m_iCurrentWidth || pEvent->height != m_iCurrentHeight){
		this->forceRedraw();
	}
}

void CInterfacedScenario::copySelection(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "copySelection\n";

	// Prepares copy
	map < CIdentifier, CIdentifier > l_vIdMapping;
	map < CIdentifier, boolean >::const_iterator it;
	m_rApplication.m_pClipboardScenario->clear();

	// Copies boxes to clipboard
	for(it=m_vCurrentObject.begin(); it!=m_vCurrentObject.end(); it++)
	{
		if(it->second)
		{
			if(m_rScenario.isBox(it->first))
			{
				CIdentifier l_oNewIdentifier;
				const IBox* l_pBox=m_rScenario.getBoxDetails(it->first);
				m_rApplication.m_pClipboardScenario->addBox(
					l_oNewIdentifier,
					*l_pBox,
					it->first);
				l_vIdMapping[it->first]=l_oNewIdentifier;
			}
		}
	}

	// Copies comments to clipboard
	for(it=m_vCurrentObject.begin(); it!=m_vCurrentObject.end(); it++)
	{
		if(it->second)
		{
			if(m_rScenario.isComment(it->first))
			{
				CIdentifier l_oNewIdentifier;
				const IComment* l_pComment=m_rScenario.getCommentDetails(it->first);
				m_rApplication.m_pClipboardScenario->addComment(
					l_oNewIdentifier,
					*l_pComment,
					it->first);
				l_vIdMapping[it->first]=l_oNewIdentifier;
			}
		}
	}

	// Copies links to clipboard
	for(it=m_vCurrentObject.begin(); it!=m_vCurrentObject.end(); it++)
	{
		if(it->second)
		{
			if(m_rScenario.isLink(it->first))
			{
				CIdentifier l_oNewIdentifier;
				const ILink* l_pLink=m_rScenario.getLinkDetails(it->first);
				m_rApplication.m_pClipboardScenario->connect(
					l_oNewIdentifier,
					l_vIdMapping[l_pLink->getSourceBoxIdentifier()],
					l_pLink->getSourceBoxOutputIndex(),
					l_vIdMapping[l_pLink->getTargetBoxIdentifier()],
					l_pLink->getTargetBoxInputIndex(),
					l_pLink->getIdentifier());
			}
		}
	}

	// Copies message links to clipboard
	for(it=m_vCurrentObject.begin(); it!=m_vCurrentObject.end(); it++)
	{
		if(it->second)
		{
			if(m_rScenario.isMessageLink(it->first))
			{
				CIdentifier l_oNewIdentifier;
				const ILink* l_pLink=m_rScenario.getMessageLinkDetails(it->first);
				m_rApplication.m_pClipboardScenario->connectMessage(
					l_oNewIdentifier,
					l_vIdMapping[l_pLink->getSourceBoxIdentifier()],
					l_pLink->getSourceBoxOutputIndex(),
					l_vIdMapping[l_pLink->getTargetBoxIdentifier()],
					l_pLink->getTargetBoxInputIndex(),
					l_pLink->getIdentifier());
			}
		}
	}
}
void CInterfacedScenario::cutSelection(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "cutSelection\n";

	this->copySelection();
	this->deleteSelection();
}
void CInterfacedScenario::pasteSelection(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "pasteSelection\n";

	// Prepares paste
	CIdentifier l_oIdentifier;
	map < CIdentifier, CIdentifier > l_vIdMapping;
	map < CIdentifier, CIdentifier >::const_iterator it;
	// TODO: this code below CenterX and CenterY seems to be useless
	//float64 l_f64CenterX=0;
	//float64 l_f64CenterY=0;
	int32 l_iBoxCount=0;

	// Pastes boxes from clipboard
	while((l_oIdentifier=m_rApplication.m_pClipboardScenario->getNextBoxIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewIdentifier;
		IBox* l_pBox=m_rApplication.m_pClipboardScenario->getBoxDetails(l_oIdentifier);
		m_rScenario.addBox(
			l_oNewIdentifier,
			*l_pBox,
			l_oIdentifier);
		l_vIdMapping[l_oIdentifier]=l_oNewIdentifier;

		// Updates visualisation manager
		CIdentifier l_oBoxAlgorithmIdentifier=l_pBox->getAlgorithmClassIdentifier();
		const IPluginObjectDesc* l_pPOD = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_oBoxAlgorithmIdentifier);

		// If a visualisation box was dropped, add it in window manager
		if((l_pPOD && l_pPOD->hasFunctionality(Kernel::PluginFunctionality_Visualization))||(l_pBox->hasModifiableSettings()))
		{
			// Let window manager know about new box
			m_pDesignerVisualisation->onVisualisationBoxAdded(m_rScenario.getBoxDetails(l_oNewIdentifier));
		}

		// TODO: computes average box location seem to be useless
		// Computes average box location
		//CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, l_oNewIdentifier);
		//l_f64CenterX+=l_oBoxProxy.getXCenter();
		//l_f64CenterY+=l_oBoxProxy.getYCenter();
		l_iBoxCount++;
	}

	// Pastes comments from clipboard
	while((l_oIdentifier=m_rApplication.m_pClipboardScenario->getNextCommentIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewIdentifier;
		IComment* l_pComment=m_rApplication.m_pClipboardScenario->getCommentDetails(l_oIdentifier);
		m_rScenario.addComment(
			l_oNewIdentifier,
			*l_pComment,
			l_oIdentifier);
		l_vIdMapping[l_oIdentifier]=l_oNewIdentifier;
	}

	// Pastes links from clipboard
	while((l_oIdentifier=m_rApplication.m_pClipboardScenario->getNextLinkIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewIdentifier;
		ILink* l_pLink=m_rApplication.m_pClipboardScenario->getLinkDetails(l_oIdentifier);
		m_rScenario.connect(
			l_oNewIdentifier,
			l_vIdMapping[l_pLink->getSourceBoxIdentifier()],
			l_pLink->getSourceBoxOutputIndex(),
			l_vIdMapping[l_pLink->getTargetBoxIdentifier()],
			l_pLink->getTargetBoxInputIndex(),
			l_pLink->getIdentifier());
	}

	// Pastes message links from clipboard
	while((l_oIdentifier=m_rApplication.m_pClipboardScenario->getNextMessageLinkIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
	{
		CIdentifier l_oNewIdentifier;
		ILink* l_pLink=m_rApplication.m_pClipboardScenario->getMessageLinkDetails(l_oIdentifier);
		m_rScenario.connectMessage(
			l_oNewIdentifier,
			l_vIdMapping[l_pLink->getSourceBoxIdentifier()],
			l_pLink->getSourceBoxOutputIndex(),
			l_vIdMapping[l_pLink->getTargetBoxIdentifier()],
			l_pLink->getTargetBoxInputIndex(),
			l_pLink->getIdentifier());
	}

	// Makes pasted stuff the default selection
	// Moves boxes under cursor
	// Moves comments under cursor
	if(m_rApplication.m_pClipboardScenario->getNextBoxIdentifier(OV_UndefinedIdentifier)!=OV_UndefinedIdentifier || m_rApplication.m_pClipboardScenario->getNextCommentIdentifier(OV_UndefinedIdentifier)!=OV_UndefinedIdentifier)
	{
		m_vCurrentObject.clear();
		for(it=l_vIdMapping.begin(); it!=l_vIdMapping.end(); it++)
		{
			m_vCurrentObject[it->second]=true;

			if(m_rScenario.isBox(it->second))
			{
				// Moves boxes under cursor
				CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, it->second);
				l_oBoxProxy.setCenter(l_oBoxProxy.getXCenter(), l_oBoxProxy.getYCenter()-32);
				this->alignBoxOnGrid(l_oBoxProxy);
			}

			if(m_rScenario.isComment(it->second))
			{
				// Moves commentes under cursor
				CCommentProxy l_oCommentProxy(m_rKernelContext, m_rScenario, it->second);
				l_oCommentProxy.setCenter(l_oCommentProxy.getXCenter(), l_oCommentProxy.getYCenter()-32);
				this->alignCommentOnGrid(l_oCommentProxy);
			}
		}
	}
	m_bScenarioModified = true;
	this->redraw();
	this->snapshotCB();
}
void CInterfacedScenario::deleteSelection(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "deleteSelection\n";

	map<CIdentifier, boolean>::const_iterator i;
	for(i=m_vCurrentObject.begin(); i!=m_vCurrentObject.end(); i++)
	{
		if(i->second)
		{
			if(m_rScenario.isBox(i->first))
			{
				// removes visualisation box from window manager
				m_pDesignerVisualisation->onVisualisationBoxRemoved(i->first);

				// removes box from scenario
				m_rScenario.removeBox(i->first);
			}
			if(m_rScenario.isComment(i->first))
			{
				// removes comment from scenario
				m_rScenario.removeComment(i->first);
			}
			if(m_rScenario.isLink(i->first))
			{
				// removes link from scenario
				m_rScenario.disconnect(i->first);
			}
			if(m_rScenario.isMessageLink(i->first))
			{
				// removes message link from scenario
				m_rScenario.disconnectMessage(i->first);
			}
		}
	}
	m_vCurrentObject.clear();
	m_bScenarioModified = true;
	this->redraw();
	this->snapshotCB();
}

void CInterfacedScenario::muteSelection(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "muteSelection\n";

	map<CIdentifier, boolean>::const_iterator i;
	for(i=m_vCurrentObject.begin(); i!=m_vCurrentObject.end(); i++)
	{
		if(i->second)
		{
			if(m_rScenario.isBox(i->first))
			{
				IBox* l_pBox=m_rScenario.getBoxDetails(i->first);
				this->contextMenuBoxMuteCB(*l_pBox);
			}
		}
	}
	m_vCurrentObject.clear();
	m_bScenarioModified = true;
	this->redraw();
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxRenameCB(IBox& rBox)
{
	const IPluginObjectDesc* l_pPluginObjectDescriptor=m_rKernelContext.getPluginManager().getPluginObjectDescCreating(rBox.getAlgorithmClassIdentifier());
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRenameCB\n";
	CRenameDialog l_oRename(m_rKernelContext, rBox.getName(), l_pPluginObjectDescriptor?l_pPluginObjectDescriptor->getName():rBox.getName(), m_sGUIFilename.c_str());
	if(l_oRename.run())
	{
		rBox.setName(l_oRename.getResult());

		//check whether it is a visualisation box
		CIdentifier l_oId = rBox.getAlgorithmClassIdentifier();
		const IPluginObjectDesc* l_pPOD = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_oId);

		//if a visualisation box was renamed, tell window manager about it
		if((l_pPOD && l_pPOD->hasFunctionality(Kernel::PluginFunctionality_Visualization))||(rBox.hasModifiableSettings()))
		{
			m_pDesignerVisualisation->onVisualisationBoxRenamed(rBox.getIdentifier());
		}
		this->snapshotCB();
	}
}

void CInterfacedScenario::contextMenuBoxRenameAllCB()
{
	//we find all selected boxes
	map < CIdentifier, CIdentifier > l_vSelectedBox; // map(object,class)
	map < CIdentifier, boolean >::const_iterator it_current;
	for(it_current=m_vCurrentObject.begin(); it_current!=m_vCurrentObject.end(); it_current++)
	{
		if(it_current->second)
		{
			if(m_rScenario.isBox(it_current->first))
			{
				l_vSelectedBox[it_current->first]=m_rScenario.getBoxDetails(it_current->first)->getAlgorithmClassIdentifier();
			}
		}
	}

	map < CIdentifier, CIdentifier >::const_iterator it;
	if(l_vSelectedBox.size())
	{
		boolean l_bDialogOk = true;
		boolean l_bFirstBox = true;
		CString l_sNewName = "";
		for(it=l_vSelectedBox.begin(); it!=l_vSelectedBox.end() && l_bDialogOk; it++)
		{
			if(it->second!=OV_UndefinedIdentifier)
			{
				if(m_rKernelContext.getPluginManager().canCreatePluginObject(it->second))
				{
					IBox* l_pBox=m_rScenario.getBoxDetails(it->first);
					if(l_bFirstBox)
					{
						l_bFirstBox = false;
						const IPluginObjectDesc* l_pPluginObjectDescriptor=m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_pBox->getAlgorithmClassIdentifier());
						CRenameDialog l_oRename(m_rKernelContext, l_pBox->getName(), l_pPluginObjectDescriptor?l_pPluginObjectDescriptor->getName():l_pBox->getName(), m_sGUIFilename.c_str());
						if(l_oRename.run())
						{
							l_sNewName = l_oRename.getResult();
						}
						else
						{
							// no rename at all.
							l_bDialogOk = false;
						}
					}
					if(l_bDialogOk)
					{
						l_pBox->setName(l_sNewName);

						//check whether it is a visualisation box
						CIdentifier l_oId = l_pBox->getAlgorithmClassIdentifier();
						const IPluginObjectDesc* l_pPOD = m_rKernelContext.getPluginManager().getPluginObjectDescCreating(l_oId);

						//if a visualisation box was renamed, tell window manager about it
						if((l_pPOD && l_pPOD->hasFunctionality(Kernel::PluginFunctionality_Visualization))||(l_pBox->hasModifiableSettings()))
						{
							m_pDesignerVisualisation->onVisualisationBoxRenamed(l_pBox->getIdentifier());
						}
					}
				}
			}
		}
		if(l_bDialogOk)
		{
			this->snapshotCB();
		}
	}
}

void CInterfacedScenario::contextMenuBoxDeleteCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxDeleteCB\n";
	m_pDesignerVisualisation->onVisualisationBoxRemoved(rBox.getIdentifier());
	m_rScenario.removeBox(rBox.getIdentifier());
	this->snapshotCB();
	m_bScenarioModified = true;
	this->redraw();
}
void CInterfacedScenario::contextMenuBoxAddInputCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddInputCB\n";
	rBox.addInput("New input", OV_TypeId_Signal);
	if(rBox.hasAttribute(OV_AttributeId_Box_FlagCanModifyInput))
	{
		CConnectorEditor l_oConnectorEditor(m_rKernelContext, rBox, Connector_Input, rBox.getInputCount()-1, "Add Input", m_sGUIFilename.c_str());
		if(l_oConnectorEditor.run())
		{
			this->snapshotCB();
		}
		else
		{
			rBox.removeInput(rBox.getInputCount()-1);
		}
	}
	else
	{
		this->snapshotCB();
	}
}

void CInterfacedScenario::contextMenuBoxAddMessageInputCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddMessageInputCB\n";
	CString l_sMessageInputName("A message input");

	CRenameDialog l_oRename(m_rKernelContext, l_sMessageInputName , l_sMessageInputName, m_sGUIFilename.c_str());
	if(l_oRename.run())
	{
		rBox.addMessageInput(l_oRename.getResult());
		this->snapshotCB();
	}

}

void CInterfacedScenario::contextMenuBoxRemoveMessageInputCB(IBox& rBox, uint32 ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveInputCB\n";
	rBox.removeMessageInput(ui32Index);
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxAddMessageOutputCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddMessageOutputCB\n";

	CString l_sMessageInputName("A message output");

	CRenameDialog l_oRename(m_rKernelContext, l_sMessageInputName , l_sMessageInputName, m_sGUIFilename.c_str());
	if(l_oRename.run())
	{
		rBox.addMessageOutput(l_oRename.getResult());
	}


}

void CInterfacedScenario::contextMenuBoxRemoveMessageOutputCB(IBox& rBox, uint32 ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveOutputCB\n";
	rBox.removeMessageOutput(ui32Index);
	this->snapshotCB();
}
void CInterfacedScenario::contextMenuBoxEditMessageInputCB(IBox& rBox, uint32 ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditMessageInputCB\n";
	CString l_sMessageInputName;
	rBox.getMessageInputName(ui32Index, l_sMessageInputName);

	CRenameDialog l_oRename(m_rKernelContext, l_sMessageInputName , l_sMessageInputName, m_sGUIFilename.c_str());
	if(l_oRename.run())
	{
		rBox.setMessageInputName(ui32Index, l_oRename.getResult());
		this->snapshotCB();
	}
}
void CInterfacedScenario::contextMenuBoxEditMessageOutputCB(IBox& rBox, uint32 ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditMessageOutputCB\n";
	CString l_sMessageOutputName;
	rBox.getMessageOutputName(ui32Index, l_sMessageOutputName);

	CRenameDialog l_oRename(m_rKernelContext, l_sMessageOutputName , l_sMessageOutputName, m_sGUIFilename.c_str());
	if(l_oRename.run())
	{
		rBox.setMessageOutputName(ui32Index, l_oRename.getResult());
		this->snapshotCB();
	}
}


void CInterfacedScenario::contextMenuBoxEditInputCB(IBox& rBox, uint32 ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditInputCB\n";

	CConnectorEditor l_oConnectorEditor(m_rKernelContext, rBox, Connector_Input, ui32Index, "Edit Input", m_sGUIFilename.c_str());
	if(l_oConnectorEditor.run())
	{
		this->snapshotCB();
	}
}
void CInterfacedScenario::contextMenuBoxRemoveInputCB(IBox& rBox, uint32 ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveInputCB\n";
	rBox.removeInput(ui32Index);
	this->snapshotCB();
}
void CInterfacedScenario::contextMenuBoxAddOutputCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddOutputCB\n";
	rBox.addOutput("New output", OV_TypeId_Signal);
	if(rBox.hasAttribute(OV_AttributeId_Box_FlagCanModifyOutput))
	{
		CConnectorEditor l_oConnectorEditor(m_rKernelContext, rBox, Connector_Output, rBox.getOutputCount()-1, "Add Output", m_sGUIFilename.c_str());
		if(l_oConnectorEditor.run())
		{
			this->snapshotCB();
		}
		else
		{
			rBox.removeOutput(rBox.getOutputCount()-1);
		}
	}
	else
	{
		this->snapshotCB();
	}
}
void CInterfacedScenario::contextMenuBoxEditOutputCB(IBox& rBox, uint32 ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditOutputCB\n";

	CConnectorEditor l_oConnectorEditor(m_rKernelContext, rBox, Connector_Output, ui32Index, "Edit Output", m_sGUIFilename.c_str());
	if(l_oConnectorEditor.run())
	{
		this->snapshotCB();
	}
}
void CInterfacedScenario::contextMenuBoxRemoveOutputCB(IBox& rBox, uint32 ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveOutputCB\n";
	rBox.removeOutput(ui32Index);
	this->snapshotCB();
}
void CInterfacedScenario::contextMenuBoxAddSettingCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAddSettingCB\n";
	rBox.addSetting("New setting", OV_UndefinedIdentifier, "");
	if(rBox.hasAttribute(OV_AttributeId_Box_FlagCanModifySetting))
	{
		CSettingEditorDialog l_oSettingEditorDialog(m_rKernelContext, rBox, rBox.getSettingCount()-1, "Add Setting", m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str());
		if(l_oSettingEditorDialog.run())
		{
			this->snapshotCB();
		}
		else
		{
			rBox.removeSetting(rBox.getSettingCount()-1);
		}
	}
	else
	{
		this->snapshotCB();
	}
}
void CInterfacedScenario::contextMenuBoxEditSettingCB(IBox& rBox, uint32 ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxEditSettingCB\n";
	CSettingEditorDialog l_oSettingEditorDialog(m_rKernelContext, rBox, ui32Index, "Edit Setting", m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str());
	if(l_oSettingEditorDialog.run())
	{
		this->snapshotCB();
	}
}
void CInterfacedScenario::contextMenuBoxRemoveSettingCB(IBox& rBox, uint32 ui32Index)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxRemoveSettingCB\n";
	rBox.removeSetting(ui32Index);
	this->snapshotCB();
}
void CInterfacedScenario::contextMenuBoxConfigureCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxConfigureCB\n";
	CBoxConfigurationDialog l_oBoxConfigurationDialog(m_rKernelContext, rBox, m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str());
	l_oBoxConfigurationDialog.run(false);//modUI
	this->snapshotCB();
}
void CInterfacedScenario::contextMenuBoxAboutCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxAboutCB\n";
	CAboutPluginDialog l_oAboutPluginDialog(m_rKernelContext, rBox.getAlgorithmClassIdentifier(), m_sGUIFilename.c_str());
	l_oAboutPluginDialog.run();
}

bool CInterfacedScenario::browseURL(CString sURL)  {
	const CString l_sWebBrowser=m_rKernelContext.getConfigurationManager().expand("${Designer_WebBrowserCommand}");

	m_rKernelContext.getLogManager() << LogLevel_Trace << "Requesting web browser on URL " << sURL << "\n";
#if TARGET_OS_Linux
	const CString l_sCommand = l_sWebBrowser+CString(" ")+sURL+CString(" &");
#else
	const CString l_sCommand = l_sWebBrowser+CString(" ")+sURL;
#endif
	m_rKernelContext.getLogManager() << LogLevel_Trace << "Launching '" << l_sCommand << "'\n";
	const int l_iResult=::system(l_sCommand.toASCIIString());
	if(l_iResult<0)
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Could not launch command " << l_sWebBrowser+CString(" ")+sURL << "\n";
		return false;
	}
	return true;
}

bool CInterfacedScenario::browseBoxDocumentation(CIdentifier oBoxId)
{
	if(oBoxId!=OV_UndefinedIdentifier && m_rKernelContext.getPluginManager().canCreatePluginObject(oBoxId))
	{
		const CString l_sURLBase=m_rKernelContext.getConfigurationManager().expand("${Designer_WebBrowserHelpURLBase}");

		const IPluginObjectDesc* l_pPluginObjectDesc=m_rKernelContext.getPluginManager().getPluginObjectDescCreating(oBoxId);
		const CString l_sHTMLName=CString("Doc_BoxAlgorithm_")+CString(getBoxAlgorithmURL(l_pPluginObjectDesc->getName().toASCIIString()).c_str())+CString(".html");
		const CString l_sFullURL=l_sURLBase+CString("/")+l_sHTMLName;

		return browseURL(l_sFullURL);
	} else {
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Box with id " << oBoxId << " can not create a pluging object\n";
		return false;
	}
	return true;
}

void CInterfacedScenario::contextMenuBoxDocumentationCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuBoxDocumentationCB\n";
	CIdentifier l_oBoxId = rBox.getAlgorithmClassIdentifier();

	browseBoxDocumentation(l_oBoxId);
}

void CInterfacedScenario::contextMenuScenarioAboutCB(void)
{
	m_rKernelContext.getLogManager() << LogLevel_Debug << "contextMenuScenarioAboutCB\n";
	CAboutScenarioDialog l_oAboutScenarioDialog(m_rKernelContext, m_rScenario, m_sGUIFilename.c_str());
	l_oAboutScenarioDialog.run();
	this->snapshotCB();
}

void CInterfacedScenario::contextMenuBoxMuteCB(IBox& rBox)
{
	m_rKernelContext.getLogManager() << LogLevel_Trace << "contextMenuBoxMuteCB\n";

	//two constructors for the BoxProxy
	//if I use the first one, I can not use setMute and have to use l_oAttributeHandler.setAttributeValue<bool>

	CBoxProxy l_oBoxProxy(m_rKernelContext, rBox);
	TAttributeHandler l_oAttributeHandler(rBox);
	//CBoxProxy l_oBoxProxy(m_rKernelContext, m_rScenario, rBox.getIdentifier() );

	//first if the attribute does not already exist, set it to false
	if(!rBox.hasAttribute(OV_AttributeId_Box_Muted))
	{
		rBox.addAttribute(OV_AttributeId_Box_Muted, "false");
	}

	const boolean l_bIsmuted = l_oBoxProxy.getMute();
	const boolean l_bNewValue = !l_bIsmuted;

	m_rKernelContext.getLogManager() << LogLevel_Debug << "was " <<  l_bIsmuted <<"\n";
	//l_oBoxProxy.setMute( l_bNewValue );
	l_oAttributeHandler.setAttributeValue<bool>(OV_AttributeId_Box_Muted, l_bNewValue);
	m_bScenarioModified = true;
	this->redraw();
	this->snapshotCB();
}

void CInterfacedScenario::toggleDesignerVisualisation()
{
	m_bDesignerVisualisationToggled = !m_bDesignerVisualisationToggled;

	if(m_bDesignerVisualisationToggled)
	{
		m_pDesignerVisualisation->show();
	}
	else
	{
		m_pDesignerVisualisation->hide();
	}
}

boolean CInterfacedScenario::isDesignerVisualisationToggled()
{
	return m_bDesignerVisualisationToggled;
}

void CInterfacedScenario::showCurrentVisualisation()
{
	if(isLocked())
	{
		if(m_pPlayerVisualisation != NULL)
		{
			for(unsigned int i=0; i<this->m_vCheckItems.size(); i++)
			{
				if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(this->m_vCheckItems[i])))
				{
					m_pPlayerVisualisation->showSelectedWindow(i);
				}
			}
		}
	}
	else
	{
		if(m_pDesignerVisualisation != NULL)
		{
			m_pDesignerVisualisation->show();
		}
	}
}

void CInterfacedScenario::hideCurrentVisualisation()
{
	if(isLocked())
	{
		if(m_pPlayerVisualisation != NULL)
		{
			m_pPlayerVisualisation->hideTopLevelWindows();
		}
	}
	else
	{
		if(m_pDesignerVisualisation != NULL)
		{
			m_pDesignerVisualisation->hide();
		}
	}

}

void CInterfacedScenario::createPlayerVisualisation()
{
	//hide window manager
	m_pDesignerVisualisation->hide();

	if(m_pPlayerVisualisation == NULL)
	{
		m_pPlayerVisualisation = new CPlayerVisualisation(m_rKernelContext, *m_pVisualisationTree, m_rApplication, *this);

		//we go here when we press start
		//we have to set the modUI here
		//first, find the concerned boxes
		CIdentifier l_oId = m_rScenario.getNextBoxIdentifier(OV_UndefinedIdentifier);
		while (l_oId!=OV_UndefinedIdentifier)
		{
			IBox* l_oBox = m_rScenario.getBoxDetails (l_oId);
			if(l_oBox->hasModifiableSettings())//if the box has modUI
			{
				//create a BoxConfigurationDialog in mode true
				CBoxConfigurationDialog* l_oBoxConfigurationDialog = new CBoxConfigurationDialog(m_rKernelContext,*l_oBox,m_sGUIFilename.c_str(), m_sGUISettingsFilename.c_str(), true);
				//store it
				m_vBoxConfigurationDialog.push_back(l_oBoxConfigurationDialog);
			}
			l_oId = m_rScenario.getNextBoxIdentifier(l_oId);
		}
	}

	//initialize and show windows
	m_pPlayerVisualisation->init();
}

void CInterfacedScenario::releasePlayerVisualisation(void)
{
	if(m_pPlayerVisualisation != NULL)
	{
		delete m_pPlayerVisualisation;
		m_pPlayerVisualisation = NULL;
	}

	//reload designer visualisation
	m_pDesignerVisualisation->load();

	//show it if it was toggled on
	if(m_bDesignerVisualisationToggled == true)
	{
		m_pDesignerVisualisation->show();
	}
}

//Called when a visualisation window is closed in Player Visualisation
void CInterfacedScenario::onWindowClosed(const char* title)
{
	//Get list of displayed windows
	std::vector < ::GtkWindow* > l_vWindows = m_pPlayerVisualisation->getTopLevelWindows();
	uint32 l_ui32Index = 0;
	//Look for index of closed window
	for(unsigned int i = 0; i<l_vWindows.size();i++)
	{
		const char* l_cWindowName = (char*) gtk_window_get_title(l_vWindows[i]);
		// if title are the same
		if(strcmp (l_cWindowName, title) == 0)
		{
			l_ui32Index = i;
		}
	}
	// toggle corresponding check menu item in application
	m_rApplication.toggleOffWindowItem(l_ui32Index);
}

//Called when menu (Windows->Show) item  is toggled
void CInterfacedScenario::onItemToggledOn(uint32 ui32Index)
{
	//Get list of displayed windows
	std::vector < ::GtkWindow* > l_vWindows = m_pPlayerVisualisation->getTopLevelWindows();
	m_pPlayerVisualisation->showSelectedWindow(ui32Index);
	//gtk_widget_show(GTK_WIDGET(l_vWindows[ui32Index]));
}
void CInterfacedScenario::onItemToggledOff(uint32 ui32Index)
{
	//Get list of displayed windows
	std::vector < ::GtkWindow* > l_vWindows = m_pPlayerVisualisation->getTopLevelWindows();
	m_pPlayerVisualisation->hideSelectedWindow(ui32Index);
}

//Hide window menu when switching scenario
void CInterfacedScenario::hideWindowMenu(void)
{
	for (unsigned int i = 0; i<m_vCheckItems.size();i++)
	{
		gtk_widget_hide(m_vCheckItems[i]);
	}

}
//Show window menu when switching back to running scenario
void CInterfacedScenario::showWindowMenu(void)
{
	for (unsigned int i = 0; i<m_vCheckItems.size();i++)
	{
		gtk_widget_show(m_vCheckItems[i]);
	}
}

boolean CInterfacedScenario::hasSelection(void)
{
	map<CIdentifier, boolean>::const_iterator i;
	for(i=m_vCurrentObject.begin(); i!=m_vCurrentObject.end(); i++)
	{
		if(i->second)
		{
			return true;
		}
	}
	return false;
}


boolean CInterfacedScenario::deleteModifiableSettingsBoxes(void)
{
	for (uint32 i=0; i<m_vBoxConfigurationDialog.size(); i++)
	{
		m_vBoxConfigurationDialog[i]->revertChange();
		delete m_vBoxConfigurationDialog[i];
	}
	m_vBoxConfigurationDialog.clear();

	return true;

}

//give the PlayerVisualisation the matching between the GtkWidget created by the CBoxConfigurationDialog and the Box CIdentifier
boolean CInterfacedScenario::setModifiableSettingsWidgets(void)
{
	for (uint32 i=0; i<m_vBoxConfigurationDialog.size(); i++)
	{
		m_pPlayerVisualisation->setWidget(m_vBoxConfigurationDialog[i]->getBoxID(),  m_vBoxConfigurationDialog[i]->getWidget());
	}

	return true;
}

boolean CInterfacedScenario::centerOnBox(CIdentifier rIdentifier)
{
	//m_rKernelContext.getLogManager() << LogLevel_Fatal << "CInterfacedScenario::centerOnBox" << "\n";
	boolean ret_val = false;
	if(m_rScenario.isBox(rIdentifier))
	{
		//m_rKernelContext.getLogManager() << LogLevel_Fatal << "CInterfacedScenario::centerOnBox is box" << "\n";
		IBox* rBox = m_rScenario.getBoxDetails(rIdentifier);

		//clear previous selection
		m_vCurrentObject.clear();
		//to select the box
		m_vCurrentObject[rIdentifier] = true;
		m_bScenarioModified=true;
		this->redraw();

		CBoxProxy l_oBoxProxy(m_rKernelContext, *rBox);
		const float64 xMargin = 5.0 * m_f64CurrentScale;
		const float64 yMargin = 5.0 * m_f64CurrentScale;
		int xSize=ov_round(l_oBoxProxy.getWidth(GTK_WIDGET(m_pScenarioDrawingArea)) + xMargin * 2.0 );
		int ySize=ov_round(l_oBoxProxy.getHeight(GTK_WIDGET(m_pScenarioDrawingArea)) + yMargin * 2.0);
		const float64 l_f64XCenter = l_oBoxProxy.getXCenter() * m_f64CurrentScale;
		const float64 l_f64YCenter = l_oBoxProxy.getYCenter() * m_f64CurrentScale;
		int x;
		int y;

		//get the parameters of the current adjustement
		GtkAdjustment* l_pOldHAdjustement = gtk_scrolled_window_get_hadjustment(m_pScrolledWindow);//gtk_viewport_get_vadjustment(m_pScenarioViewport);
		GtkAdjustment* l_pOldVAdjustement = gtk_scrolled_window_get_vadjustment(m_pScrolledWindow);
		gdouble upper;
		gdouble lower;
		gdouble step;
		gdouble page;
		gdouble pagesize;
		gdouble value;
		g_object_get(l_pOldHAdjustement, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value", &value, NULL);
		//get the size of the current viewport
//		gint l_iViewportX = -1;
//		gint l_iViewportY = -1;
//		gdk_window_get_size(GTK_WIDGET(m_pScenarioViewport)->window, &l_iViewportX, &l_iViewportY);

//		//the upper bound of the adjustement is too big, we must substract the current size of the viewport
//		upper-=l_iViewportX;

		//crete a new adjustement with the correct value since we can not change the upper bound of the old adjustement
		GtkAdjustment* l_pAdjustement = (GtkAdjustment*)gtk_adjustment_new(value, lower, upper, step, page, pagesize);
		if(l_f64XCenter + m_i32ViewOffsetX < m_iCurrentWidth/2)
		{
			x = ov_round(l_f64XCenter - 2*xSize) + m_i32ViewOffsetX;
		}
		else{
			x = ov_round(l_f64XCenter + 2*xSize) + m_i32ViewOffsetX;
			x = ov_round( (x/(float64)m_iCurrentWidth) * upper - pagesize);
		}


		gtk_adjustment_set_value(l_pAdjustement, x);
		gtk_scrolled_window_set_hadjustment(m_pScrolledWindow, l_pAdjustement);

		g_object_get(l_pOldVAdjustement, "upper", &upper, "lower", &lower, "step-increment", &step, "page-increment", &page, "page-size", &pagesize, "value", &value, NULL);
		//upper-=l_iViewportY;
		l_pAdjustement = (GtkAdjustment*)gtk_adjustment_new(value, lower, upper, step, page, pagesize);
		if(l_f64YCenter + m_i32ViewOffsetY < m_iCurrentHeight/2)
		{
			y = ov_round(l_f64YCenter - 2*ySize) + m_i32ViewOffsetY;
		}
		else{
			y = ov_round(l_f64YCenter + 2*ySize) + m_i32ViewOffsetY;
			//Here the formula is different because we need to align the other side of the scroll bar
			y = ov_round((y / (float64)m_iCurrentHeight) * upper - pagesize);
		}
		gtk_adjustment_set_value(l_pAdjustement, y);
		gtk_scrolled_window_set_vadjustment(m_pScrolledWindow, l_pAdjustement);
		ret_val = true;
	}
	return ret_val;
}
