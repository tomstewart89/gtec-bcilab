#include "ovpCSignalDisplayView.h"

#include <iostream>

#include <sstream>

#include <algorithm>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

using namespace OpenViBEToolkit;

using namespace std;

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		void scrollModeButtonCallback(::GtkWidget *widget, gpointer data);
		void unitsButtonCallback(::GtkWidget *widget, gpointer data);
		void scalingModeButtonCallback(::GtkWidget *widget, gpointer data);
		void toggleLeftRulerButtonCallback(::GtkWidget *widget, gpointer data);
		void toggleBottomRulerButtonCallback(::GtkWidget *widget, gpointer data);
		void customVerticalScaleChangedCallback(::GtkSpinButton* pSpinButton, gpointer data);
		void customVerticalOffsetChangedCallback(::GtkSpinButton* pSpinButton, gpointer data);
		gboolean spinButtonValueChangedCallback(::GtkSpinButton *widget,  gpointer data); // time scale
		void channelSelectButtonCallback(::GtkButton *button, gpointer data);
		void channelSelectDialogApplyButtonCallback(::GtkButton *button, gpointer data);
		void stimulationColorsButtonCallback(::GtkButton *button, gpointer data);
		gint closeStimulationColorsWindow(GtkWidget *widget, GdkEvent  *event, gpointer data);
		void informationButtonCallback(::GtkButton *button, gpointer data);
		void multiViewButtonCallback(::GtkButton *button, gpointer data);
		void multiViewDialogApplyButtonCallback(::GtkButton *button, gpointer data);

		const char* CSignalDisplayView::m_vScalingModes[] = { "Per channel", "Global", "None" };

		CSignalDisplayView::CSignalDisplayView(CBufferDatabase& oBufferDatabase, 
			CIdentifier oDisplayMode, 
			CIdentifier oScalingMode, 
			float64 f64VerticalScale, 
			float64 f64VerticalOffset,
			float64 f64TimeScale,
			boolean bHorizontalRuler,
			boolean bVerticalRuler,
			boolean bMultiview
			)
			:
			m_pBuilderInterface(NULL)
			,m_pSignalDisplayTable(NULL)
			,m_bShowLeftRulers(bVerticalRuler)
			,m_bShowBottomRuler(bHorizontalRuler)
			,m_ui64LeftmostDisplayedTime(0)
			,m_f64LargestDisplayedValueRange(0)
			,m_f64ValueRangeMargin(0)
            ,m_f64MarginFactor(0.4f) //add 40% space above and below extremums
			,m_bVerticalScaleRefresh(true)
			,m_bVerticalScaleForceUpdate(false)
			,m_f64CustomVerticalScaleValue(f64VerticalScale)
			,m_f64CustomVerticalOffset(f64VerticalOffset)
			,m_pBufferDatabase(&oBufferDatabase)
			,m_bMultiViewEnabled(bMultiview)
			,m_pBottomBox(NULL)
			,m_pBottomRuler(NULL)
			,m_oScalingMode(oScalingMode)
			,m_bStimulationColorsShown(false)
		{

			m_bVerticalScaleForceUpdate=true;
			m_ui32SelectedChannelCount = 0;

			m_vSelectedChannels.clear();
			m_vChannelUnits.clear();
			m_vErrorState.clear();

			construct(oBufferDatabase,f64TimeScale,oDisplayMode);
		}

		void CSignalDisplayView::construct(CBufferDatabase& oBufferDatabase, float64 f64TimeScale, CIdentifier oDisplayMode)
		{
			//load the gtk builder interface
			m_pBuilderInterface=::gtk_builder_new(); // glade_xml_new(OpenViBE::Directories::getDataDir() + "/plugins/simple-visualisation/openvibe-simple-visualisation-SignalDisplay.ui", NULL, NULL);
			gtk_builder_add_from_file(m_pBuilderInterface, OpenViBE::Directories::getDataDir() + "/plugins/simple-visualisation/openvibe-simple-visualisation-SignalDisplay.ui", NULL);

			if(!m_pBuilderInterface)
			{
				g_warning("Couldn't load the interface!");
				return;
			}

			::gtk_builder_connect_signals(m_pBuilderInterface, NULL);

			//initialize display mode
			m_pBufferDatabase->setDisplayMode(oDisplayMode);
			::gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayScrollModeButton")),
				oDisplayMode == OVP_TypeId_SignalDisplayMode_Scroll);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayScrollModeButton")),  true);

			//connect display mode callbacks
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayScrollModeButton")),  "toggled", G_CALLBACK(scrollModeButtonCallback),        this);
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayToggleUnitsButton")), "toggled", G_CALLBACK(unitsButtonCallback),             this);

			//creates the cursors
			m_pCursor[0] = gdk_cursor_new(GDK_LEFT_PTR);
			m_pCursor[1] = gdk_cursor_new(GDK_SIZING);

			//button callbacks
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayChannelSelectButton")),     "clicked", G_CALLBACK(channelSelectButtonCallback),     this);
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayStimulationColorsButton")), "clicked", G_CALLBACK(stimulationColorsButtonCallback), this);
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMultiViewButton")),         "clicked", G_CALLBACK(multiViewButtonCallback),         this);
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayInformationButton")),       "clicked", G_CALLBACK(informationButtonCallback),       this);

			//initialize vertical scale
			// ::gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayVerticalScaleToggleButton")), m_bAutoVerticalScale);
			::gtk_spin_button_set_value(GTK_SPIN_BUTTON(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayCustomVerticalScaleSpinButton")), m_f64CustomVerticalScaleValue);
			// ::gtk_spin_button_set_increments(GTK_SPIN_BUTTON(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayCustomVerticalScaleSpinButton")),0.001,1.0);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayCustomVerticalScaleSpinButton")),  m_oScalingMode == OVP_TypeId_SignalDisplayScaling_None);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayDC")), m_oScalingMode == OVP_TypeId_SignalDisplayScaling_None);
			::gtk_spin_button_set_value(GTK_SPIN_BUTTON(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayDC")), m_f64CustomVerticalOffset);
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayDC")), "value-changed", G_CALLBACK(customVerticalOffsetChangedCallback), this);

	//		::gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayToggleUnitsButton")), false);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayToggleUnitsButton")), false);

			//connect vertical scale callbacks
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayCustomVerticalScaleSpinButton")), "value-changed", G_CALLBACK(customVerticalScaleChangedCallback), this);

			//time scale
			//----------
			::GtkSpinButton* l_pSpinButton = GTK_SPIN_BUTTON(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayTimeScale"));
			::gtk_spin_button_set_value(l_pSpinButton, f64TimeScale);
			g_signal_connect(G_OBJECT(l_pSpinButton), "value-changed", G_CALLBACK(spinButtonValueChangedCallback),  this);
			//notify database of current time scale
			m_pBufferDatabase->adjustNumberOfDisplayedBuffers(::gtk_spin_button_get_value(l_pSpinButton));

			//channel select dialog's signals
			//-------------------------------
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayChannelSelectApplyButton")), "clicked", G_CALLBACK(channelSelectDialogApplyButtonCallback), this);

			//connect the cancel button to the dialog's hide command
			g_signal_connect_swapped(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayChannelSelectCancelButton")),
					"clicked",
					G_CALLBACK(::gtk_widget_hide),
					G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayChannelSelectDialog")));

			//hides the dialog if the user tries to close it
			g_signal_connect (G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayChannelSelectDialog")),
					"delete_event",
					G_CALLBACK(::gtk_widget_hide), NULL);

			//stimulation colors dialog's signals
			//-----------------------------------
			//connect the close button to the dialog's hide command
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayStimulationColorsCloseButton")),
					"clicked",
					G_CALLBACK(stimulationColorsButtonCallback),
					this);

			//hides the dialog if the user tries to close it
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayStimulationColorsDialog")),
					"delete_event",
					 G_CALLBACK(closeStimulationColorsWindow),
					 this);

			//multiview signals
			//-----------------
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMultiViewApplyButton")), "clicked", G_CALLBACK(multiViewDialogApplyButtonCallback), this);

			//connect the cancel button to the dialog's hide command
			g_signal_connect_swapped(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMultiViewCancelButton")),
				"clicked",
				G_CALLBACK(::gtk_widget_hide),
				G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMultiViewDialog")));

			//hides the dialog if the user tries to close it
			g_signal_connect (G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMultiViewDialog")),
				"delete_event",
				G_CALLBACK(::gtk_widget_hide), NULL);

			//bottom box
			//----------
			m_pBottomBox = GTK_BOX(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayBottomBox"));

			// ::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayBestFitButton")), false);
			// ::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayGlobalBestFitButton")), false);

			::GtkComboBox* l_pComboBox=GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderInterface, "ScalingMode"));

			for(uint32 i=0;i<sizeof(m_vScalingModes)/sizeof(const char *);i++)
			{
				::gtk_combo_box_append_text(l_pComboBox, m_vScalingModes[i]);
			}

			g_signal_connect(G_OBJECT(l_pComboBox), "changed", G_CALLBACK(scalingModeButtonCallback), this);
			::gtk_combo_box_set_active(l_pComboBox, (gint)m_oScalingMode.toUInteger());

			::gtk_widget_set_sensitive(GTK_WIDGET(l_pComboBox), true);

		//	GtkWidget* l_pMainWindow = GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMainWindow"));
		//	::gtk_window_set_default_size(GTK_WINDOW(l_pMainWindow), 640, 200);
		}

		CSignalDisplayView::~CSignalDisplayView()
		{
			// @fixme who destroys this beast? It seems to be accessed by visualisationtree later?!  pointer ownership unclear.
			::gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "Toolbar")));

			::gtk_widget_destroy(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayInformationDialog")));
			::gtk_widget_destroy(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayChannelSelectDialog")));
			::gtk_widget_destroy(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMultiViewDialog")));
			::gtk_widget_destroy(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayStimulationColorsDialog")));

			//destroy the window and its children
			::gtk_widget_destroy(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMainWindow")));

			//destroy the rest
			for(int i=0 ; i<2 ; i++)
			{
				gdk_cursor_unref(m_pCursor[i]);
			}

			//unref the xml file as it's not needed anymore
			g_object_unref(G_OBJECT(m_pBuilderInterface));
			m_pBuilderInterface=NULL;

			std::vector < CSignalChannelDisplay* >::iterator it;
			for(it=m_oChannelDisplay.begin(); it!=m_oChannelDisplay.end(); it++) {
                delete (*it);
			}

			delete m_pBottomRuler;
			m_pBottomRuler = NULL;
		}

		void CSignalDisplayView::getWidgets(::GtkWidget*& pWidget, ::GtkWidget*& pToolbarWidget)
		{
			pWidget=GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayScrolledWindow"));
			pToolbarWidget=GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "Toolbar"));
		}

		void CSignalDisplayView::changeMultiView()
		{

            CSignalChannelDisplay* l_pMultiViewDisplay = m_oChannelDisplay[m_oChannelDisplay.size()-1];

			//check if there are channels to display in multiview
			m_bMultiViewEnabled=false;
            boolean l_bNoneSelected=false;
			for(uint32 i=0; i<m_oChannelLabel.size(); i++)
			{
				//Check if None is selected
                if(i == m_oChannelLabel.size()-1)
				{
                    l_bNoneSelected = m_vMultiViewSelectedChannels[i];
				}

                if(!l_bNoneSelected)
				{
                    //Enable Multiview only if None item isn't selected and at list one channel is selected
					m_bMultiViewEnabled|=m_vMultiViewSelectedChannels[i];
				}
                else
                {
                    //Disable Multiview if None is selected
                    m_bMultiViewEnabled = false;
                }
			}

			//if there are no channels to display in the multiview (None selected only)
			if(!m_bMultiViewEnabled)
			{
				//hides the multiview display (last one in the list)
				l_pMultiViewDisplay->resetChannelList();
				toggleChannelMultiView(false);
			}
			//there are channels to display in the multiview
			else
			{
				if(!GTK_WIDGET_VISIBLE(GTK_WIDGET(m_pSignalDisplayTable)))
				{
					//if there were no selected channels before, but now there are, show the table again
					::gtk_widget_show(GTK_WIDGET(m_pSignalDisplayTable));
				}

				if(isChannelDisplayVisible(m_oChannelDisplay.size()-1) == false)
				{
					toggleChannelMultiView(true);
				}

				//updates channels to display list
				l_pMultiViewDisplay->resetChannelList();

                for(size_t i=0 ; i<m_vMultiViewSelectedChannels.size() ; i++)
				{
					if(m_vMultiViewSelectedChannels[i])
					{
                        l_pMultiViewDisplay->addChannelList(i);
					}
				}

				l_pMultiViewDisplay->updateLimits();

                if(m_bShowLeftRulers == true)
                {
                    ::gtk_widget_show(GTK_WIDGET(m_oLeftRulers[m_oChannelDisplay.size()-1]));
                }

                l_pMultiViewDisplay->m_bMultiView = true;

				m_bVerticalScaleForceUpdate = true; // need to pass the scale params to multiview, use this to make them refresh...
				m_bVerticalScaleRefresh = true;


				//request a redraw
				/*
				if(l_pChannelDisplay->getSignalDisplayWidget()->window) 
				{
					gdk_window_invalidate_rect(l_pChannelDisplay->getSignalDisplayWidget()->window, NULL, false);
				}
				*/

			}


		}

		void CSignalDisplayView::init()
		{
			//retrieve channel count
			const OpenViBE::uint32 l_ui32ChannelCount = (uint32)m_pBufferDatabase->getChannelCount();
            const OpenViBE::uint32 l_ui32TableSize = 2;

			//allocate channel labels and channel displays arrays accordingly
            m_oChannelDisplay.resize(l_ui32TableSize);
			m_oChannelLabel.resize(l_ui32ChannelCount+1);
			m_vChannelName.resize(l_ui32ChannelCount+1);

			GtkWidget* l_pScrolledWindow = GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayScrolledWindow"));
			::gtk_widget_set_size_request(l_pScrolledWindow, 400, 200);

			//retrieve and allocate main table accordingly
			m_pSignalDisplayTable = GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMainTable"));
			//rows : for each channel, [0] channel data, [1] horizontal separator
			//columns : [0] label, [1] vertical separator, [2] left ruler, [3] signal display
			::gtk_table_resize(GTK_TABLE(m_pSignalDisplayTable), l_ui32ChannelCount+1, 4);

			const int32 l_i32LeftRulerWidthRequest = 50;
			const int32 l_i32ChannelDisplayWidthRequest = 20;
			const int32 l_i32BottomRulerWidthRequest = 0;

			const int32 l_i32LeftRulerHeightRequest = 20;
			const int32 l_i32ChannelDisplayHeightRequest = 20;
			const int32 l_i32BottomRulerHeightRequest = 20;

			m_ui32SelectedChannelCount = l_ui32ChannelCount;	// All channels selected by default

			updateDisplayTableSize();

			//add a vertical separator
			m_pSeparator = ::gtk_vseparator_new();
			::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable), m_pSeparator,
				1, 2, //second column
				0, l_ui32ChannelCount+1, //run over the whole table height
				GTK_SHRINK, static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL), 0, 0);
			::gtk_widget_show(m_pSeparator);

			//create a size group for channel labels and the empty bottom left widget
			//(useful to position the bottom ruler correctly)
			//::GtkSizeGroup* l_pSizeGroup = ::gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

			//channels selection widget
			::GtkWidget * l_pChannelSelectList = GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayChannelSelectList"));

			//multiple channels selection widget
			::GtkWidget * l_pMultiViewSelectList = GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMultiViewSelectList"));

			//vector of channel names
			vector<string>& l_oChannelName = m_pBufferDatabase->m_pDimensionLabels[0];

			stringstream l_oLabelString;

			::GtkListStore* l_pChannelListStore=::gtk_list_store_new(1, G_TYPE_STRING);
			::GtkTreeIter l_oChannelIter;

            ::GtkListStore* l_pMultiViewChannelListStore=::gtk_list_store_new(1, G_TYPE_STRING);
            ::GtkTreeIter l_oMultiViewChannelIter;

			//create channel widgets and add them to display table
			for(uint32 i=0 ; i<l_ui32ChannelCount ; i++)
			{
				//add channel label
				//-----------------
				// Convention: Channels are numbered as 1,2,... when shown to user
				if(l_oChannelName[i] == "")
				{
					//if no name has been set, use channel index
					l_oLabelString << "Channel " << (i+1);
				}
				else //prepend name with channel index
				{
                    l_oLabelString << (i+1) << " : " << l_oChannelName[i];
				}

                // In either mode (eeg or non-eeg) create and attach label widget for each channel
				::GtkWidget* l_pLabel = ::gtk_label_new(l_oLabelString.str().c_str());
				m_vChannelName[i] = CString(l_oLabelString.str().c_str());
				m_oChannelLabel[i] = l_pLabel;
				::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),l_pLabel,
					0, 1, //first column
					i, i+1,
                    GTK_FILL, static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL),
					0, 0);
				::gtk_widget_show(l_pLabel);

				// Using the labels in a size group causes it to freeze after changing the labels in a callback. Disabled for now.
		//		::gtk_size_group_add_widget(l_pSizeGroup, l_pLabel);
				if(m_vChannelUnits.size()<=i)
				{
					m_vChannelUnits[i]=std::pair<CString, CString>(CString("Unknown"), CString("Unspecified"));
				}

				//create channel display widget
				//-----------------------------

				//add checkbox in channel select window
				//-------------------------------------
				::gtk_list_store_append(l_pChannelListStore, &l_oChannelIter);
				::gtk_list_store_set(l_pChannelListStore, &l_oChannelIter, 0, l_oChannelName[i].c_str(), -1);

                ::gtk_list_store_append(l_pMultiViewChannelListStore, &l_oMultiViewChannelIter);
                ::gtk_list_store_set(l_pMultiViewChannelListStore, &l_oMultiViewChannelIter, 0, l_oChannelName[i].c_str(), -1);

				l_oLabelString.str("");

				//a channel is selected by default
				m_vSelectedChannels[i]=true;
				if(m_bMultiViewEnabled) 
				{
					m_vMultiViewSelectedChannels[i]=true;
				}
				else
				{
					m_vMultiViewSelectedChannels[i]=false;
				}

				//clear label
				l_oLabelString.str("");
			}

			 // create one display for all channels

            //create and attach display widget
            CSignalChannelDisplay* l_pChannelDisplay = new CSignalChannelDisplay(
                        this,
                        l_i32ChannelDisplayWidthRequest, l_i32ChannelDisplayHeightRequest,
                        l_i32LeftRulerWidthRequest, l_i32LeftRulerHeightRequest);
            m_oChannelDisplay[0] = l_pChannelDisplay;
            for(uint32 i=0 ; i<l_ui32ChannelCount ; i++)
            {
                l_pChannelDisplay->addChannel(i);

                // Still attach left rulers
                ::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),
                    l_pChannelDisplay->getRulerWidget(i),
                    2, 3, //third column
                    i, i+1,
                    GTK_FILL, static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL),	0, 0);
                ::gtk_widget_show(l_pChannelDisplay->getRulerWidget(i));
            }
			l_pChannelDisplay->updateLimits();

            // attach display
            ::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),
                        l_pChannelDisplay->getSignalDisplayWidget(),
                        3, 4, //fourth column
                        0, l_ui32ChannelCount,// run over the whole table (last row for multiview)
                        static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL), 
						static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL),
                        0, 0);
            ::gtk_widget_show(m_oChannelDisplay[0]->getSignalDisplayWidget());

			::gtk_tree_selection_set_mode(::gtk_tree_view_get_selection(GTK_TREE_VIEW(l_pChannelSelectList)), GTK_SELECTION_MULTIPLE);
			::gtk_tree_view_append_column(GTK_TREE_VIEW(l_pChannelSelectList), ::gtk_tree_view_column_new_with_attributes("Channel", ::gtk_cell_renderer_text_new(), "text", 0, NULL));
			::gtk_tree_view_set_model(GTK_TREE_VIEW(l_pChannelSelectList), GTK_TREE_MODEL(l_pChannelListStore));

            ::gtk_list_store_append(l_pMultiViewChannelListStore, &l_oMultiViewChannelIter);
            ::gtk_list_store_set(l_pMultiViewChannelListStore, &l_oMultiViewChannelIter, 0, "None", -1);

			::gtk_tree_selection_set_mode(::gtk_tree_view_get_selection(GTK_TREE_VIEW(l_pMultiViewSelectList)), GTK_SELECTION_MULTIPLE);
			::gtk_tree_view_append_column(GTK_TREE_VIEW(l_pMultiViewSelectList), ::gtk_tree_view_column_new_with_attributes("Channel", ::gtk_cell_renderer_text_new(), "text", 0, NULL));
            ::gtk_tree_view_set_model(GTK_TREE_VIEW(l_pMultiViewSelectList), GTK_TREE_MODEL(l_pMultiViewChannelListStore));

			//multiview channel
			//-----------------
			//create and attach label
			::GtkWidget * l_pLabel =  ::gtk_label_new("Multi-View");
			m_oChannelLabel[l_ui32ChannelCount] = l_pLabel;
			::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),l_pLabel,
				0, 1,
				l_ui32ChannelCount, (l_ui32ChannelCount)+1,
				GTK_FILL, GTK_SHRINK,
				0, 0);
			//create and attach display widget
			CSignalChannelDisplay* l_pMultiViewDisplay = new CSignalChannelDisplay(
				this,
				l_i32ChannelDisplayWidthRequest, l_i32ChannelDisplayHeightRequest,
				l_i32LeftRulerWidthRequest, l_i32LeftRulerHeightRequest);
            m_oChannelDisplay[l_ui32TableSize-1] = l_pMultiViewDisplay;
            l_pMultiViewDisplay->addChannel(0);
			::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),
                l_pMultiViewDisplay->getRulerWidget(0),
				2, 3, //third column
				(l_ui32ChannelCount), (l_ui32ChannelCount)+1,
				GTK_FILL, GTK_FILL,
				0, 0);
			::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),
				l_pMultiViewDisplay->getSignalDisplayWidget(),
				3, 4, //fourth column
				(l_ui32ChannelCount), (l_ui32ChannelCount)+1,
				static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL), static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL),
				0, 0);
			//create bottom ruler
			//-------------------
			m_pBottomRuler = new CBottomTimeRuler(*m_pBufferDatabase, l_i32BottomRulerWidthRequest, l_i32BottomRulerHeightRequest);
			//::gtk_size_group_add_widget(l_pSizeGroup, GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayEmptyLabel1")));
			::gtk_box_pack_start(m_pBottomBox, m_pBottomRuler->getWidget(), false, false, 0);
			// tell ruler has to resize when channel displays are resized
			if(m_oChannelDisplay.size() != 0)
			{
				m_pBottomRuler->linkWidthToWidget(m_oChannelDisplay[0]->getSignalDisplayWidget());
			}
			::gtk_widget_show_all(m_pBottomRuler->getWidget());

			//allocate memory to store sample points
			//--------------------------------------
			//reserve the maximum space needed for computing the points to display
			//(when cropping the lines, there can be up to two times the number of original points)
			m_pPoints.reserve((size_t)(m_pBufferDatabase->m_pDimensionSizes[1]*m_pBufferDatabase->m_ui64NumberOfBufferToDisplay * 2));
			//resize the vector of raw points
			m_pRawPoints.resize((size_t)(m_pBufferDatabase->m_pDimensionSizes[1]*m_pBufferDatabase->m_ui64NumberOfBufferToDisplay));

            for(uint32 j = 0; j<m_oChannelDisplay.size();j++)
            {
                for(uint32 i = 0; i<m_oChannelDisplay[j]->m_oLeftRuler.size();i++)
                {
                    ::GtkWidget* l_pLeftRuler = m_oChannelDisplay[j]->getRulerWidget(i);
                    m_oLeftRulers.push_back(l_pLeftRuler);
                }
            }

			toggleLeftRulers(m_bShowLeftRulers);
			::gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayToggleLeftRulerButton")), m_bShowLeftRulers);
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayToggleLeftRulerButton")),     "toggled",       G_CALLBACK(toggleLeftRulerButtonCallback),   this);

			toggleBottomRuler(m_bShowBottomRuler);
			::gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayToggleBottomRulerButton")), m_bShowBottomRuler);
			g_signal_connect(G_OBJECT(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayToggleBottomRulerButton")),   "toggled",       G_CALLBACK(toggleBottomRulerButtonCallback), this);

			if(m_bMultiViewEnabled) 
			{
				changeMultiView();
			}

			activateToolbarButtons(true);

		}

		void CSignalDisplayView::redraw()
		{
			//nothing to redraw if the table isn't visible or no data was received
			if(m_pSignalDisplayTable == NULL || !GTK_WIDGET_VISIBLE(m_pSignalDisplayTable) || m_pBufferDatabase->isFirstBufferReceived() == false)
			{
				return;
			}

			if(m_bVerticalScaleRefresh || m_bVerticalScaleForceUpdate)
			{
				const float64 l_f64MarginMultiplier = 0.2;

				// @note the reason the applying of scale parameters is here and not inside SignalChannelDisplay is that in
				// some situations we wish to estimate and set params across two SignalChannelDisplay objects: 
				// the main view and multiview.
				if(m_oScalingMode == OVP_TypeId_SignalDisplayScaling_Global) 
				{
					// Auto global
				
					// Find the global min and max
					vector<float64> l_vValueMin;
					vector<float64> l_vValueMax;
					m_oChannelDisplay[0]->getDisplayedValueRange(l_vValueMin,l_vValueMax);
			
					float64 l_f64MinValue = *(std::min_element(l_vValueMin.begin(), l_vValueMin.end()));
					float64 l_f64MaxValue = *(std::max_element(l_vValueMax.begin(), l_vValueMax.end()));

					if(m_bMultiViewEnabled)
					{
						vector<float64> l_vMultiValueMin;
						vector<float64> l_vMultiValueMax;
						m_oChannelDisplay[1]->getDisplayedValueRange(l_vMultiValueMin,l_vMultiValueMax);

						l_f64MinValue = std::min(l_f64MinValue, *(std::min_element(l_vMultiValueMin.begin(), l_vMultiValueMin.end())));
						l_f64MaxValue = std::max(l_f64MaxValue, *(std::max_element(l_vMultiValueMax.begin(), l_vMultiValueMax.end())));
					}

					// @todo some robust & fast estimate of a high quantile instead of max/min...
					const float64 l_f64Margin = l_f64MarginMultiplier * (l_f64MaxValue - l_f64MinValue);

					const float64 l_f64MinimumTopMargin    = m_oChannelDisplay[0]->m_vMinimumTopMargin[0];
					const float64 l_f64MaximumBottomMargin = m_oChannelDisplay[0]->m_vMaximumBottomMargin[0];

					if( m_bVerticalScaleForceUpdate || 
						l_f64MinValue < l_f64MaximumBottomMargin-l_f64Margin || l_f64MaxValue > l_f64MinimumTopMargin + l_f64Margin ||
						l_f64MinValue > l_f64MaximumBottomMargin+l_f64Margin || l_f64MaxValue < l_f64MinimumTopMargin - l_f64Margin)
					{
						m_oChannelDisplay[0]->setGlobalScaleParameters(l_f64MinValue, l_f64MaxValue, l_f64Margin); // normal chns
						m_oChannelDisplay[1]->setGlobalScaleParameters(l_f64MinValue, l_f64MaxValue, l_f64Margin); // multiview
					}
				} 
				else if(m_oScalingMode == OVP_TypeId_SignalDisplayScaling_None) 
				{
					// Manual global, only updated when triggered as necessary
					if(m_bVerticalScaleForceUpdate)
					{
						const float64 l_f64Min = m_f64CustomVerticalOffset - m_f64CustomVerticalScaleValue/2;
						const float64 l_f64Max = m_f64CustomVerticalOffset + m_f64CustomVerticalScaleValue/2;
						const float64 l_f64Margin = l_f64MarginMultiplier * (l_f64Max - l_f64Min);

						m_oChannelDisplay[0]->setGlobalScaleParameters(l_f64Min, l_f64Max, l_f64Margin); // normal chns
						m_oChannelDisplay[1]->setGlobalScaleParameters(l_f64Min, l_f64Max, l_f64Margin); // multiview
					}
				}
				else if(m_oScalingMode == OVP_TypeId_SignalDisplayScaling_PerChannel) 
				{
					// Auto local
					vector<float64> l_vValueMin;
					vector<float64> l_vValueMax;
					m_oChannelDisplay[0]->getDisplayedValueRange(l_vValueMin,l_vValueMax);

					bool updated = false;
					for(uint32 i=0;i<l_vValueMin.size();i++) 
					{
						const float64 l_f64Margin = l_f64MarginMultiplier * (l_vValueMax[i] - l_vValueMin[i]);
						const float64 l_f64MinimumTopMargin    = m_oChannelDisplay[0]->m_vMinimumTopMargin[i];
						const float64 l_f64MaximumBottomMargin = m_oChannelDisplay[0]->m_vMaximumBottomMargin[i];

						if(m_bVerticalScaleForceUpdate ||
						   l_vValueMin[i] < l_f64MaximumBottomMargin-l_f64Margin || l_vValueMax[i] > l_f64MinimumTopMargin+l_f64Margin || 
						   l_vValueMin[i] > l_f64MaximumBottomMargin+l_f64Margin || l_vValueMax[i] < l_f64MinimumTopMargin-l_f64Margin)
						{
#ifdef DEBUG
							std::cout << "Channel " << i+1 << " params updated: " 
								<< l_vValueMin[i] << " not in [" << l_f64MaximumBottomMargin-f64Margin << "," << l_f64MaximumBottomMargin+f64Margin << "], or "
								<< l_vValueMax[i] << " not in [" << l_f64MinimumTopMargin-f64Margin << "," << l_f64MinimumTopMargin+f64Margin << "], "
								<< " margin was " << l_f64Margin << "\n";
#endif
							m_oChannelDisplay[0]->setLocalScaleParameters(i, l_vValueMin[i], l_vValueMax[i], l_f64Margin);
							updated = true;
						}
						else
						{
#ifdef DEBUG
							std::cout << "No need to update channel " << i+1 << ", "
								<< l_vValueMin[i] << " in [" << l_f64MaximumBottomMargin-f64Margin << "," << l_f64MaximumBottomMargin+f64Margin << "], or "
								<< l_vValueMax[i] << " in [" << l_f64MinimumTopMargin-f64Margin << "," << l_f64MinimumTopMargin+f64Margin << "], "
								<< " margin was " << l_f64Margin << "\n";
#endif
						}
					}
					if(updated)
					{
						m_oChannelDisplay[0]->updateDisplayParameters();
					}

					// For multiview, we take the maxes of the involved signals
					if(m_bMultiViewEnabled)
					{
						m_oChannelDisplay[1]->getDisplayedValueRange(l_vValueMin,l_vValueMax);

						const float64 l_f64MinValue = *(std::min_element(l_vValueMin.begin(), l_vValueMin.end()));
						const float64 l_f64MaxValue = *(std::max_element(l_vValueMax.begin(), l_vValueMax.end()));

						// @todo some robust & fast estimate of a high quantile instead of max/min...
						const float64 l_f64Margin = l_f64MarginMultiplier * (l_f64MaxValue - l_f64MinValue);
						const float64 l_f64MinimumTopMargin    = m_oChannelDisplay[1]->m_vMinimumTopMargin[0];
						const float64 l_f64MaximumBottomMargin = m_oChannelDisplay[1]->m_vMaximumBottomMargin[0];

						if( m_bVerticalScaleForceUpdate
							|| l_f64MaxValue > l_f64MinimumTopMargin    + l_f64Margin 
							|| l_f64MaxValue < l_f64MinimumTopMargin    - l_f64Margin 
							|| l_f64MinValue > l_f64MaximumBottomMargin + l_f64Margin 
							|| l_f64MinValue < l_f64MaximumBottomMargin - l_f64Margin)
						{
							m_oChannelDisplay[1]->setGlobalScaleParameters(l_f64MinValue, l_f64MaxValue, l_f64Margin); // multiview
							m_oChannelDisplay[1]->updateDisplayParameters();
						}
					}
				}
				else
				{
					std::stringstream ss; 

					ss << "Error: unknown scaling mode " << m_oScalingMode.toString() << ". Did you update the box?\n";

					m_vErrorState.push_back(CString(ss.str().c_str()));

					return;
				}
				m_bVerticalScaleRefresh = false;
				m_bVerticalScaleForceUpdate = false;
			}

			// todo don't reset every frame

			/*


			std::cout << "Range is " << l_f64LargestDisplayedValueRange << " at " << l_ui32MaxIdxI << "," << l_ui32MaxIdxJ 
				<< " with lim [" << m_f64LargestDisplayedValueRange - m_f64ValueRangeMargin << "," 
								 << m_f64LargestDisplayedValueRange + m_f64ValueRangeMargin << ","
				<< " largest " << l_f64LargestDisplayedValue
				<< " vs " << m_f64LargestDisplayedValue 
				<< " smallest " << l_f64SmallestDisplayedValue << " vs " << m_f64SmallestDisplayedValue
				<< "\n";
			*/

			//if in scan mode, check whether time scale needs to be updated
			if(m_pBufferDatabase->getDisplayMode() == OVP_TypeId_SignalDisplayMode_Scan && m_ui64LeftmostDisplayedTime < m_pBufferDatabase->m_oStartTime[0])
			{
				//printf("Time basis needs to be updated\n");
				if(m_pBufferDatabase->m_oSampleBuffers.size() < m_pBufferDatabase->m_ui64NumberOfBufferToDisplay)
				{
					m_ui64LeftmostDisplayedTime = m_pBufferDatabase->m_oStartTime[0];
				}
				else //catch up with current time interval
				{
					if(m_pBufferDatabase->m_ui64TotalStep == 0)
					{
						// Error
						//
						// @note This can happen at least during changing of time scale, however on the next attempt it seems
						// to be already fixed in the bufferdatabase and things seem to work, so don't bother returning error.
						// @fixme should get proper understanding of this part to properly handle it, i.e. should we 
						// really raise an error state in some situations or not.
//						m_vErrorState.push_back(CString("Error: Buffer database m_ui64TotalStep is 0\n"));

					}
					else
					{
						m_ui64LeftmostDisplayedTime += m_pBufferDatabase->m_ui64TotalStep;

						uint64 l_ui64UpperLimit = 0;
						if(m_pBufferDatabase->m_ui64BufferStep <= m_pBufferDatabase->m_oStartTime[0]) // This bubblegum-patch test is here for uint, should be an assert
						{
							l_ui64UpperLimit = m_pBufferDatabase->m_oStartTime[0] - m_pBufferDatabase->m_ui64BufferStep;
						}
						else
						{		
							m_vErrorState.push_back(CString("Error: Buffer step is larger than the start time\n"));
						}

						//while there is time to catch up
						while(m_ui64LeftmostDisplayedTime < l_ui64UpperLimit)
						{
							m_ui64LeftmostDisplayedTime += m_pBufferDatabase->m_ui64TotalStep;
						}

						//round leftmost displayed time to start of closest data buffer
						for(uint32 i=0; i<m_pBufferDatabase->m_oStartTime.size(); i++)
						{
							if(m_pBufferDatabase->m_oEndTime[i] > m_ui64LeftmostDisplayedTime)
							{
								m_ui64LeftmostDisplayedTime = m_pBufferDatabase->m_oStartTime[i];
							}
						}

						//if drawing is not up to date, force a full redraw
						if(m_oChannelDisplay[0]->m_ui64LatestDisplayedTime != m_ui64LeftmostDisplayedTime)
						{
							for(size_t i=0; i<m_oChannelDisplay.size(); i++)
							{
#ifdef DEBUG
								std::cout << "Requesting full redraw for " << i << ", case D\n";
#endif
								m_oChannelDisplay[i]->redrawAllAtNextRefresh();
							}
						}
					}
				}
			}

			//redraw channels
			for(size_t i=0 ; i<m_oChannelDisplay.size(); i++)
			{
				if(GTK_WIDGET_VISIBLE(m_oChannelDisplay[i]->getSignalDisplayWidget()))
				{
					/*
					//if in scroll mode, or if time basis changed, redraw all
					if(m_pBufferDatabase->getDisplayMode() == OVP_TypeId_SignalDisplayMode_Scroll || l_pChannelDisplay->mustRedrawAll() == true)
					{
						printf("full redraw\n");*/
						GdkRectangle l_oUpdateRect;
						m_oChannelDisplay[i]->getUpdateRectangle(l_oUpdateRect);
						if(m_oChannelDisplay[i]->getSignalDisplayWidget()->window) 
						{
#ifdef DEBUG
							std::cout 
								<< "Invalidate rect B " << l_oUpdateRect.x << "+" << l_oUpdateRect.width 
								<< "," << l_oUpdateRect.y << "+" << l_oUpdateRect.height << "\n";
#endif
							gdk_window_invalidate_rect(m_oChannelDisplay[i]->getSignalDisplayWidget()->window, &l_oUpdateRect, false);
						}
					/*}
					else
					{
						GdkRectangle l_oUpdateRect;
						m_oChannelDisplay[i]->getUpdateRectangle(l_oUpdateRect);
						//printf("partial redraw : x=%d, w=%d\n", l_oUpdateRect.x, l_oUpdateRect.width);
						gdk_window_clear_area_e(m_oChannelDisplay[i]->getSignalDisplayWidget()->window, l_oUpdateRect.x, l_oUpdateRect.y, l_oUpdateRect.width, l_oUpdateRect.height);
					}*/
				}
			}

			//redraw ruler
			m_pBottomRuler->setLeftmostDisplayedTime(m_ui64LeftmostDisplayedTime);
			if(GTK_WIDGET(m_pBottomRuler->getWidget())->window) gdk_window_invalidate_rect(GTK_WIDGET(m_pBottomRuler->getWidget())->window, NULL, true);
		}

		void CSignalDisplayView::toggleLeftRulers(boolean bActive)
		{
            m_bShowLeftRulers = bActive;

			for(uint32 j = 0 ; j<m_oChannelDisplay[0]->m_oLeftRuler.size();j++)
			{
                if(bActive && isChannelDisplayVisible(0) && m_vSelectedChannels[j])
				{
                    ::gtk_widget_show(m_oChannelDisplay[0]->getRulerWidget(j));
				}
				else
				{
                    ::gtk_widget_hide(m_oChannelDisplay[0]->getRulerWidget(j));
                }
			}
			
			// Multiview
			if(m_bMultiViewEnabled)
			{
				if(bActive)
				{
					::gtk_widget_show(m_oChannelDisplay[1]->getRulerWidget(0));
				}
				else
				{
	                ::gtk_widget_hide(m_oChannelDisplay[1]->getRulerWidget(0));
				}
			}
        }

		void CSignalDisplayView::toggleBottomRuler(boolean bActive)
		{
			m_bShowBottomRuler = bActive;

			if(bActive)
			{
				::gtk_widget_show_all(GTK_WIDGET(m_pBottomBox));
			}
			else
			{
				::gtk_widget_hide_all(GTK_WIDGET(m_pBottomBox));
			}
		}

		void CSignalDisplayView::toggleChannel(uint32 ui32ChannelIndex, boolean bActive)
		{
			CSignalChannelDisplay* l_pChannelDisplay = getChannelDisplay(ui32ChannelIndex);

			if(bActive)
			{
				::gtk_widget_show(m_oChannelLabel[ui32ChannelIndex]);
				if(m_bShowLeftRulers == true)
				{
					::gtk_widget_show(l_pChannelDisplay->getRulerWidget(l_pChannelDisplay->m_oChannelList.size()-1));
				}
				::gtk_widget_show(l_pChannelDisplay->getSignalDisplayWidget());
				::gtk_widget_show(m_vSeparator[ui32ChannelIndex]);
			}
			else
			{
				::gtk_widget_hide(m_oChannelLabel[ui32ChannelIndex]);
				::gtk_widget_hide(l_pChannelDisplay->getRulerWidget(l_pChannelDisplay->m_oChannelList.size()-1));
				::gtk_widget_hide(l_pChannelDisplay->getSignalDisplayWidget());
				::gtk_widget_hide(m_vSeparator[ui32ChannelIndex]);
			}
		}

		// If we swap multiview on/off, it seems we need to do another size request to 
		// get the labels and signals properly aligned. The problem appears if there are many channels and this is not done.
		void CSignalDisplayView::updateDisplayTableSize(void)
		{
				const int32 l_i32LeftRulerWidthRequest = 50;
				const int32 l_i32ChannelDisplayWidthRequest = 20;
	
				const int32 l_i32ChannelDisplayHeightRequest = 20;
			
				//sets a minimum size for the table (needed to scroll)
				::gtk_widget_set_size_request(
					m_pSignalDisplayTable,
					l_i32LeftRulerWidthRequest + l_i32ChannelDisplayWidthRequest,
					(m_ui32SelectedChannelCount + (m_bMultiViewEnabled ? 1 : 0))*l_i32ChannelDisplayHeightRequest);

		}

		void CSignalDisplayView::toggleChannelMultiView(boolean bActive)
		{
			updateDisplayTableSize();

			CSignalChannelDisplay* l_pChannelDisplay = getChannelDisplay(m_oChannelDisplay.size()-1);
			if(bActive)
			{

				::gtk_widget_show(m_oChannelLabel[m_oChannelLabel.size()-1]);
				if(m_bShowLeftRulers == true)
				{
                    ::gtk_widget_show(l_pChannelDisplay->getRulerWidget(0));
				}
				::gtk_widget_show(l_pChannelDisplay->getSignalDisplayWidget());
			}
			else
			{
				::gtk_widget_hide(m_oChannelLabel[m_oChannelLabel.size()-1]);
                ::gtk_widget_hide(l_pChannelDisplay->getRulerWidget(0));
				::gtk_widget_hide(l_pChannelDisplay->getSignalDisplayWidget());

			}
		}

		// This removes all the per-channel rulers and widgets. It adds ref count to the removed
		// widgets so we can later add them back.
		void CSignalDisplayView::removeOldWidgets(void)
		{
			// Remove labels and rulers
			for(uint32 i=0;i<m_vSelectedChannels.size();i++)
			{
				// Only remove those which we know are displayed
				if(m_vSelectedChannels[i])
				{
					g_object_ref(m_oChannelLabel[i]);
					g_object_ref(m_oChannelDisplay[0]->getRulerWidget(i));
					gtk_container_remove(GTK_CONTAINER(m_pSignalDisplayTable), m_oChannelLabel[i]);
					gtk_container_remove(GTK_CONTAINER(m_pSignalDisplayTable), m_oChannelDisplay[0]->getRulerWidget(i));
				}
			}

			// Remove the separator
			g_object_ref(m_pSeparator);
			gtk_container_remove(GTK_CONTAINER(m_pSignalDisplayTable), m_pSeparator);

			// Remove the drawing area
			g_object_ref(m_oChannelDisplay[0]->getSignalDisplayWidget());
			gtk_container_remove(GTK_CONTAINER(m_pSignalDisplayTable), m_oChannelDisplay[0]->getSignalDisplayWidget());


		}

		// When channels are added or removed, this function removes and recreates the table holding the
		// rulers. The reason to do this is that the size of the drawing canvas is dependent on the size
		// of the table, and we want to use the window space to draw the selected signals, likely much
		// smaller than the size of canvas for all the channes.
		// @note refcounts of the added widgets are decreased. Its expected removeOldWidgets() has been called before.
		// @fixme this code could really use some refactoring, for example
		// make a struct to hold label and ruler and keep them in a vector. Also, similar attach code is
		// already in init(). Turn to functions.
		void CSignalDisplayView::recreateWidgets(uint32 ui32ChannelCount)
		{
			// Resize the table to fit only the selected amount of channels (+multiview)
			::gtk_table_resize(GTK_TABLE(m_pSignalDisplayTable), ui32ChannelCount+1, 4);

			// Add selected channel widgets back
			for(uint32 i=0,cnt=0;i<m_vSelectedChannels.size();i++)
			{
				if(m_vSelectedChannels[i])
				{
					::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),
	                    m_oChannelDisplay[0]->getRulerWidget(i),
						2, 3, //third column
						cnt, cnt+1,
						GTK_FILL, static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL),	0, 0);
					::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),m_oChannelLabel[i],
						0, 1, //first column
						cnt, cnt+1,
						GTK_FILL, static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL),
						0, 0);
					cnt++;
					g_object_unref(m_oChannelLabel[i]);
					g_object_unref(m_oChannelDisplay[0]->getRulerWidget(i));
				}
			}

			// Add separator back
			::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable), m_pSeparator,
				1, 2, //second column
				0, ui32ChannelCount+1, //run over the whole table height
				GTK_SHRINK, static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL), 0, 0);
			g_object_unref(m_pSeparator);

			// Add drawing canvas back
            ::gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),
                        m_oChannelDisplay[0]->getSignalDisplayWidget(),
                        3, 4, //fourth column
                        0, ui32ChannelCount,// run over the whole table (last row for multiview)
                        static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL), static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL),
                        0, 0);
			g_object_unref(m_oChannelDisplay[0]->getSignalDisplayWidget());

			updateDisplayTableSize();
		}


		void CSignalDisplayView::updateMainTableStatus()
		{
			// Do we have multiview channels selected?
			boolean l_bMultiView=false;
			for(uint32 i=0; i<m_vMultiViewSelectedChannels.size(); i++)
			{
				l_bMultiView|=m_vMultiViewSelectedChannels[i];
			}

			// See if any normal channels have been selected
			boolean l_bChannels=false;
			for(uint32 i=0; i<m_vSelectedChannels.size(); i++)
			{
				l_bChannels|=m_vSelectedChannels[i];
			}

			//if nothing has been selected, hide & bail out
			if(!l_bChannels && !l_bMultiView)
			{
				//hide the whole table
				::gtk_widget_hide(GTK_WIDGET(m_pSignalDisplayTable));
				return;
			}

			// If a multiview channel has been selected, we link the bottom ruler to the multiview 
			if(!GTK_WIDGET_VISIBLE(GTK_WIDGET(m_pSignalDisplayTable)))
			{
				//if there were no selected channels before, but now there are, show the table again
				::gtk_widget_show(GTK_WIDGET(m_pSignalDisplayTable));
			}

			if(!l_bMultiView)
			{
				m_pBottomRuler->linkWidthToWidget(m_oChannelDisplay[0]->getSignalDisplayWidget());
			}
			else
			{
				m_pBottomRuler->linkWidthToWidget(m_oChannelDisplay[1]->getSignalDisplayWidget());
			}
		}

		void CSignalDisplayView::activateToolbarButtons(boolean bActive)
		{
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayScrollModeButton")), bActive);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayToggleLeftRulerButton")), bActive);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayToggleBottomRulerButton")), bActive);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayToggleUnitsButton")), bActive);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayChannelSelectButton")), bActive);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayStimulationColorsButton")), bActive);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayMultiViewButton")), bActive);
			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayInformationButton")), bActive);
		}

		boolean CSignalDisplayView::onUnitsToggledCB(boolean active)
		{
			// dont update for multiview
			for(size_t i=0 ; i<m_oChannelLabel.size()-1; i++)
			{
				if(active)
				{
					std::stringstream label(""); 
					label << m_vChannelName[i].toASCIIString(); 
					label << "\n(" << m_vChannelUnits[i].first.toASCIIString();
					label << ", " << m_vChannelUnits[i].second.toASCIIString() << ")"; 

					gtk_label_set_text(GTK_LABEL(m_oChannelLabel[i]), label.str().c_str());
				}
				else
				{					
					std::stringstream label(""); label << m_vChannelName[i]; 
					gtk_label_set_text(GTK_LABEL(m_oChannelLabel[i]), label.str().c_str());
				}
			}

			return true;
		}

		boolean CSignalDisplayView::onDisplayModeToggledCB(CIdentifier oDisplayMode)
		{
			m_pBufferDatabase->setDisplayMode(oDisplayMode);

			//force full redraw of all channels when display mode changes
			for(size_t i=0 ; i<m_oChannelDisplay.size(); i++)
			{
#ifdef DEBUG
				std::cout << "Requesting full redraw for " << i << ", case E\n";
#endif
				m_oChannelDisplay[i]->redrawAllAtNextRefresh();
			}

			//redraw channels
			redraw();

			return true;
		}

		boolean CSignalDisplayView::onVerticalScaleModeToggledCB(::GtkToggleButton* pToggleButton)
		{
			m_bVerticalScaleForceUpdate = true;
			m_bVerticalScaleRefresh = true;

//			::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayCustomVerticalScaleSpinButton")), !m_bAutoVerticalScale);
//			::gtk_spin_button_set_value(GTK_SPIN_BUTTON(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayCustomVerticalScaleSpinButton")), m_f64LargestDisplayedValueRange);

			return true;
		}

		boolean CSignalDisplayView::onCustomVerticalScaleChangedCB(::GtkSpinButton *pSpinButton)
		{
			m_bVerticalScaleForceUpdate = true;
			m_bVerticalScaleRefresh = true;
			m_f64CustomVerticalScaleValue = ::gtk_spin_button_get_value(pSpinButton);
			return true;
		}

		boolean CSignalDisplayView::onCustomVerticalOffsetChangedCB(::GtkSpinButton *pSpinButton)
		{
			m_bVerticalScaleForceUpdate = true;
			m_bVerticalScaleRefresh = true;
			m_f64CustomVerticalOffset = ::gtk_spin_button_get_value(pSpinButton);
			return true;
		}



		CSignalChannelDisplay* CSignalDisplayView::getChannelDisplay(uint32 ui32ChannelIndex)
		{
			if(ui32ChannelIndex < m_oChannelDisplay.size())
			{
				return m_oChannelDisplay[ui32ChannelIndex];
			}
			else
			{
				return NULL;
			}
		}

		boolean CSignalDisplayView::isChannelDisplayVisible(uint32 ui32ChannelIndex)
		{
			return GTK_WIDGET_VISIBLE(getChannelDisplay(ui32ChannelIndex)->getSignalDisplayWidget());
		}

		void CSignalDisplayView::onStimulationReceivedCB(uint64 ui64StimulationCode, const CString& rStimulationName)
		{
			if(m_mStimulations.find(ui64StimulationCode) == m_mStimulations.end())
			{
				//only the lower 32 bits of the stimulation code are currently used to compute the color
				uint32 ui32Code = (uint32)ui64StimulationCode;
				GdkColor l_oColor;
				l_oColor.red = 0;
				l_oColor.green = 0;
				l_oColor.blue = 0;

				//go through the lower 32 bits to compute RGB components. Bit positions are
				//inverted so that close code values result in very different colors.
				for(uint32 i=0; i<11; i++)
				{
					l_oColor.red   |= ( (ui32Code>>(3*i))   & 0x1 ) << (10-i);
					l_oColor.green |= ( (ui32Code>>(3*i+1)) & 0x1 ) << (10-i);
					if(i<10) //only 10 bits for blue component
					{
						l_oColor.blue  |= ( (ui32Code>>(3*i+2)) & 0x1 ) << (9-i);
					}
				}

				//convert to 16 bits per channel
				l_oColor.red   = (l_oColor.red * 65535)   / 0x7FF; //red coded on 11 bits
				l_oColor.green = (l_oColor.green * 65535) / 0x7FF; //green coded on 11 bits
				l_oColor.blue  = (l_oColor.blue * 65535)  / 0x3FF; //blue coded on 10 bits

				//store stimulation color in map
				m_mStimulations[ui64StimulationCode].first = rStimulationName;
				m_mStimulations[ui64StimulationCode].second = l_oColor;

				//update stimulations dialog
				updateStimulationColorsDialog(rStimulationName, l_oColor);
			}

			// @note We should not redraw after the stimuli, as the stim timestamp may be in the future compared
			// to the signal database. If that is the case, we get an expensive redraw from the code. 
			// The redraw will be carried out in the normal course of events when plotting the signal.

		}

		boolean CSignalDisplayView::setChannelUnits(const std::vector< std::pair<OpenViBE::CString, OpenViBE::CString> >& oChannelUnits)
		{
			for(uint32 i=0;i<oChannelUnits.size();i++)
			{
				m_vChannelUnits[i] = oChannelUnits[i];
			}

			return true;
		}

		void CSignalDisplayView::getStimulationColor(uint64 ui64StimulationCode, GdkColor& rColor)
		{
			if(m_mStimulations.find(ui64StimulationCode) != m_mStimulations.end())
			{
				rColor = m_mStimulations[ui64StimulationCode].second;
			}
		}

		void CSignalDisplayView::getMultiViewColor(uint32 ui32ChannelIndex, ::GdkColor& rColor)
		{
			if(m_mSignals.find(ui32ChannelIndex) != m_mSignals.end())
			{
				rColor = m_mSignals[ui32ChannelIndex].second;
			}
			else
			{
				uint32 ui32Code = ui32ChannelIndex;
				rColor.red = 0;
				rColor.green = 0;
				rColor.blue = 0;

				//go through the lower 32 bits to compute RGB components. Bit positions are
				//inverted so that close code values result in very different colors.
				for(uint32 i=0; i<11; i++)
				{
					rColor.red   |= ( (ui32Code>>(3*i))   & 0x1 ) << (10-i);
					rColor.green |= ( (ui32Code>>(3*i+1)) & 0x1 ) << (10-i);
					if(i<10) //only 10 bits for blue component
					{
						rColor.blue  |= ( (ui32Code>>(3*i+2)) & 0x1 ) << (9-i);
					}
				}

				//convert to 16 bits per channel
				rColor.red   = (rColor.red * 65535)   / 0x7FF; //red coded on 11 bits
				rColor.green = (rColor.green * 65535) / 0x7FF; //green coded on 11 bits
				rColor.blue  = (rColor.blue * 65535)  / 0x3FF; //blue coded on 10 bits

				//store signal color in map
				m_mSignals[ui32ChannelIndex].first = "";
				m_mSignals[ui32ChannelIndex].second = rColor;
			}
		}

		void CSignalDisplayView::updateStimulationColorsDialog(const CString& rStimulationLabel, const GdkColor& rStimulationColor)
		{
			//retrieve table
			::GtkTable* l_pStimulationColorsTable = GTK_TABLE(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayStimulationColorsTable"));

			//resize table and store new colors
			::gtk_table_resize(l_pStimulationColorsTable, l_pStimulationColorsTable->nrows + 1, 2);

			//set a minimum size request (needed to scroll)
			int32 l_i32LabelWidthRequest = -1;
			int32 l_i32ColorWidthRequest = 50;
			int32 l_i32RowHeightRequest = 20;

			::gtk_widget_set_size_request(
				GTK_WIDGET(l_pStimulationColorsTable),
				-1,
				(l_pStimulationColorsTable->nrows + 1) * l_i32RowHeightRequest);

			::GtkLabel* l_pStimLabel = GTK_LABEL(::gtk_label_new("Stimulations"));
			::gtk_widget_set_size_request(GTK_WIDGET(l_pStimLabel), -1, 20);
			::gtk_table_attach(l_pStimulationColorsTable, GTK_WIDGET(l_pStimLabel), 0, 1, 0, 1,
				static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL), GTK_FILL,	0, 0);

			::GtkLabel* l_pColorLabel = GTK_LABEL(::gtk_label_new("Colors"));
			::gtk_widget_set_size_request(GTK_WIDGET(l_pColorLabel), -1, 20);
			::gtk_table_attach(l_pStimulationColorsTable, GTK_WIDGET(l_pColorLabel), 1, 2, 0, 1,
				static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL), GTK_FILL,	0, 0);

			::GtkLabel* l_pLabel = GTK_LABEL(::gtk_label_new(rStimulationLabel.toASCIIString()));
			::gtk_widget_set_size_request(GTK_WIDGET(l_pLabel), l_i32LabelWidthRequest, l_i32RowHeightRequest);
			::gtk_table_attach(l_pStimulationColorsTable, GTK_WIDGET(l_pLabel),
				0, 1, //first column
				l_pStimulationColorsTable->nrows-1, l_pStimulationColorsTable->nrows-1+1, //last row
				static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL), static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL),	0, 0);
#if 1
			GdkPixbuf* l_pPixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, l_i32ColorWidthRequest, l_i32RowHeightRequest);
			//fill with RGBA value
			const guint32 l_ui32Color = (((guint32)(rStimulationColor.red * 255 / 65535)) << 24) +
				(((guint32)(rStimulationColor.green * 255 / 65535)) << 16) +
				(((guint32)(rStimulationColor.blue * 255 / 65535)) << 8);
			gdk_pixbuf_fill(l_pPixbuf, l_ui32Color);
			::GtkWidget* l_pImage = ::gtk_image_new_from_pixbuf(l_pPixbuf);
			::gtk_table_attach(l_pStimulationColorsTable, GTK_WIDGET(l_pImage),
				1, 2, //2nd column
				l_pStimulationColorsTable->nrows-1, l_pStimulationColorsTable->nrows-1+1, //last row
				static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL), static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL),	0, 0);
#else
			::GtkColorButton* l_pButton = GTK_COLOR_BUTTON(::gtk_color_button_new_with_color(&rStimulationColor));
			::gtk_widget_set_size_request(GTK_WIDGET(l_pButton), l_i32ColorWidthRequest, l_i32RowHeightRequest);
			//g_signal_connect(G_OBJECT(l_pButton), "clicked", G_CALLBACK(dummyButtonCallback), NULL);
			::gtk_table_attach(l_pStimulationColorsTable, GTK_WIDGET(l_pButton),
				1, 2, //2nd column
				l_pStimulationColorsTable->nrows-1, l_pStimulationColorsTable->nrows-1+1, //last row
				static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL), static_cast < ::GtkAttachOptions >(GTK_EXPAND | GTK_FILL),	0, 0);
#endif
			::GtkWidget* l_pStimulationColorsDialog = GTK_WIDGET(::gtk_builder_get_object(m_pBuilderInterface, "SignalDisplayStimulationColorsDialog"));
			if(m_bStimulationColorsShown)
			{
				// Forces a redraw of it all
				::gtk_widget_show_all(l_pStimulationColorsDialog);
				gtk_widget_queue_draw(l_pStimulationColorsDialog);
			}
		}

		void CSignalDisplayView::refreshScale(void) 
		{
			m_bVerticalScaleRefresh = true;
			// But do not force an update, its just a recommendation to check...
		}

		//
		//CALLBACKS
		//

		void scrollModeButtonCallback(::GtkWidget *widget, gpointer data)
		{
			reinterpret_cast < CSignalDisplayView* >(data)->onDisplayModeToggledCB(
				::gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget)) != 0 ?
					OVP_TypeId_SignalDisplayMode_Scroll : OVP_TypeId_SignalDisplayMode_Scan);
		}

		void unitsButtonCallback(::GtkWidget *widget, gpointer data)
		{
			reinterpret_cast < CSignalDisplayView* >(data)->onUnitsToggledCB(
				::gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget)) != 0 ?
					 true : false);
		}

		void scalingModeButtonCallback(::GtkWidget *widget, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);

			const int32 l_i32Selection = (gint)::gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
			
			if(l_pView->m_oScalingMode != l_i32Selection)
			{
				l_pView->m_oScalingMode = l_i32Selection;
				l_pView->m_bVerticalScaleForceUpdate = true;
				l_pView->m_bVerticalScaleRefresh = true;

				l_pView->redraw(); // immediate redraw

				bool l_pcontrolsActive = (l_pView->m_oScalingMode == OVP_TypeId_SignalDisplayScaling_None);

				::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayCustomVerticalScaleSpinButton")),  l_pcontrolsActive);
				::gtk_widget_set_sensitive(GTK_WIDGET(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayDC")), l_pcontrolsActive);
			}

		}

		void toggleLeftRulerButtonCallback(::GtkWidget *widget, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);
			l_pView->toggleLeftRulers(::gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget))?true:false);
		}

		void toggleBottomRulerButtonCallback(::GtkWidget *widget, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);
			l_pView->toggleBottomRuler(::gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget))?true:false);
		}

		void customVerticalScaleChangedCallback(::GtkSpinButton* pSpinButton, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);
			l_pView->onCustomVerticalScaleChangedCB(pSpinButton);
		}

		void customVerticalOffsetChangedCallback(::GtkSpinButton *pSpinButton, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);
			l_pView->onCustomVerticalOffsetChangedCB(pSpinButton);
		}

		gboolean spinButtonValueChangedCallback(::GtkSpinButton *widget,  gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);

			const float64 l_f64NewValue = ::gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));

			//Compute and save the new number of buffers to display
			boolean l_bNumberOfDisplayedBufferChanged = l_pView->m_pBufferDatabase->adjustNumberOfDisplayedBuffers(l_f64NewValue);

			if(l_bNumberOfDisplayedBufferChanged)
			{
				//reserve the maximum space needed for computing the points to display
				//(when cropping the lines, there can be up to two times the number of original points)
				l_pView->m_pPoints.reserve((size_t)(l_pView->m_pBufferDatabase->m_pDimensionSizes[1] * l_pView->m_pBufferDatabase->m_ui64NumberOfBufferToDisplay * 2));

				//resize the vector of raw points (before cropping)
				l_pView->m_pRawPoints.resize((size_t)(l_pView->m_pBufferDatabase->m_pDimensionSizes[1] * l_pView->m_pBufferDatabase->m_ui64NumberOfBufferToDisplay));

				//force full redraw of all channels when time scale changes
				for(size_t i=0 ; i<l_pView->m_oChannelDisplay.size(); i++)
				{
					l_pView->getChannelDisplay(i)->updateScale();
				}

				//redraw channels

				l_pView->m_bVerticalScaleForceUpdate = true;
				l_pView->m_bVerticalScaleRefresh = true;

				l_pView->redraw();
			}

			return FALSE;
		}


		//called when the channel select button is pressed (opens the channel selection dialog)
		void channelSelectButtonCallback(::GtkButton *button, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);

			::GtkWidget * l_pChannelSelectDialog = GTK_WIDGET(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayChannelSelectDialog"));
			::GtkTreeView* l_pChannelSelectTreeView = GTK_TREE_VIEW(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayChannelSelectList"));
			::GtkTreeSelection* l_pChannelSelectTreeSelection = ::gtk_tree_view_get_selection(l_pChannelSelectTreeView);
			::GtkTreeModel* l_pChannelSelectTreeModel = ::gtk_tree_view_get_model(l_pChannelSelectTreeView);
			::GtkTreeIter l_oIter;

			if(::gtk_tree_model_get_iter_first(l_pChannelSelectTreeModel, &l_oIter))
			{
				for(uint32 i=0;i<l_pView->m_vSelectedChannels.size();i++)
				{
					if(l_pView->m_vSelectedChannels[i])
					{
						::gtk_tree_selection_select_iter(l_pChannelSelectTreeSelection, &l_oIter);
					}
					else
					{
						::gtk_tree_selection_unselect_iter(l_pChannelSelectTreeSelection, &l_oIter);
					}
					if(!::gtk_tree_model_iter_next(l_pChannelSelectTreeModel, &l_oIter)) 
					{
						break;
					}
				}
			}

			//finally, show the dialog
			::gtk_widget_show_all(l_pChannelSelectDialog);
		}

		//Called when the user presses the apply button of the channel selection dialog
		void channelSelectDialogApplyButtonCallback(::GtkButton *button, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);

			::GtkTreeView* l_pChannelSelectTreeView = GTK_TREE_VIEW(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayChannelSelectList"));
			::GtkTreeSelection* l_pChannelSelectTreeSelection = ::gtk_tree_view_get_selection(l_pChannelSelectTreeView);
			::GtkTreeModel* l_pChannelSelectTreeModel = ::gtk_tree_view_get_model(l_pChannelSelectTreeView);
			::GtkTreeIter l_oIter;
			uint32 l_ui32SelectedCount = 0;

			l_pView->m_oChannelDisplay[0]->resetChannelList();

			// We first remove the widgets while we still know from m_vSelectedChannels which are displayed
			l_pView->removeOldWidgets();

			if(::gtk_tree_model_get_iter_first(l_pChannelSelectTreeModel, &l_oIter))
			{
				for(uint32 i=0;i<l_pView->m_vSelectedChannels.size();i++)
				{
					l_pView->m_vSelectedChannels[i]=(::gtk_tree_selection_iter_is_selected(l_pChannelSelectTreeSelection, &l_oIter)?true:false);

                    if(gtk_tree_selection_iter_is_selected(l_pChannelSelectTreeSelection, &l_oIter)?true:false)
                    {
                        l_pView->m_oChannelDisplay[0]->addChannelList(i);
                        gtk_widget_show(l_pView->m_oChannelLabel[i]);
                        if(l_pView->m_bShowLeftRulers == true)
                        {
                            gtk_widget_show(l_pView->m_oLeftRulers[i]);
                        }
						l_ui32SelectedCount++;
					}
					else
					{
						gtk_widget_hide(l_pView->m_oChannelLabel[i]);
						gtk_widget_hide(l_pView->m_oLeftRulers[i]);

					}

					if(!::gtk_tree_model_iter_next(l_pChannelSelectTreeModel, &l_oIter))
					{
						break;
					}
				}
			}

			l_pView->m_ui32SelectedChannelCount = l_ui32SelectedCount;

			// Add the widgets back with the new list of channels
			l_pView->recreateWidgets(l_ui32SelectedCount);

			l_pView->updateMainTableStatus();

			l_pView->m_bVerticalScaleForceUpdate = true;
			l_pView->m_bVerticalScaleRefresh = true;

            //redraw channels
            // l_pView->redraw();

			//hides the channel selection dialog
			::gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayChannelSelectDialog")));
		}

		gint closeStimulationColorsWindow(GtkWidget *widget, GdkEvent  *event, gpointer data)
		{
			stimulationColorsButtonCallback(NULL, data);

			return TRUE;
		}

		void stimulationColorsButtonCallback(::GtkButton *button, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);
			::GtkWidget* l_pStimulationColorsDialog = GTK_WIDGET(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayStimulationColorsDialog"));

			if(l_pView->m_bStimulationColorsShown)
			{
				::gtk_widget_hide(l_pStimulationColorsDialog);
				l_pView->m_bStimulationColorsShown = false;
			}
			else
			{
				::gtk_widget_show_all(l_pStimulationColorsDialog);
				l_pView->m_bStimulationColorsShown = true;
			}
		}

		//Called when the user presses the Information button (opens the information dialog)
		void informationButtonCallback(::GtkButton *button, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);

			//gets the different values from the database and updates the corresponding label's text field
			stringstream l_oValueString;
			l_oValueString<<l_pView->m_pBufferDatabase->m_pDimensionSizes[0];
			::gtk_label_set_text(GTK_LABEL(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayNumberOfChannels")),
				 l_oValueString.str().c_str() );

			l_oValueString.str("");
			l_oValueString<<l_pView->m_pBufferDatabase->m_ui32SamplingFrequency;
			::gtk_label_set_text(GTK_LABEL(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplaySamplingFrequency")),
				l_oValueString.str().c_str() );

			l_oValueString.str("");
			l_oValueString<<l_pView->m_pBufferDatabase->m_pDimensionSizes[1];
			::gtk_label_set_text(GTK_LABEL(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplaySamplesPerBuffer")),
				l_oValueString.str().c_str() );

			l_oValueString.str("");
			l_oValueString<<l_pView->m_pBufferDatabase->m_f64MinimumValue;
			::gtk_label_set_text(GTK_LABEL(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayMinimumValue")),
				l_oValueString.str().c_str() );

			l_oValueString.str("");
			l_oValueString<<l_pView->m_pBufferDatabase->m_f64MaximumValue;
			::gtk_label_set_text(GTK_LABEL(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayMaximumValue")),
				   l_oValueString.str().c_str() );

			::GtkWidget * l_pInformationDialog = GTK_WIDGET(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayInformationDialog"));

			//connect the close button to the dialog's hide command
			g_signal_connect_swapped(G_OBJECT(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayInformationClose")),
					"clicked",
					G_CALLBACK(::gtk_widget_hide),
					G_OBJECT(l_pInformationDialog));

			g_signal_connect(G_OBJECT(l_pInformationDialog),
				"delete_event",
				G_CALLBACK(::gtk_widget_hide),
				NULL);

			//finally, show the information dialog
			::gtk_widget_show_all(l_pInformationDialog);
		}

		//called when the channel select button is pressed (opens the channel selection dialog)
		void multiViewButtonCallback(::GtkButton *button, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);

			::GtkWidget * l_pMultiViewDialog = GTK_WIDGET(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayMultiViewDialog"));
			::GtkTreeView* l_pMultiViewTreeView = GTK_TREE_VIEW(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayMultiViewSelectList"));
			::GtkTreeSelection* l_pMultiViewTreeSelection = ::gtk_tree_view_get_selection(l_pMultiViewTreeView);
			::GtkTreeModel* l_pMultiViewTreeModel = ::gtk_tree_view_get_model(l_pMultiViewTreeView);
			::GtkTreeIter l_oIter;

			if(::gtk_tree_model_get_iter_first(l_pMultiViewTreeModel, &l_oIter))
			{
				for(uint32 i=0;l_pView->m_vMultiViewSelectedChannels.size();i++)
				{
					if(l_pView->m_vMultiViewSelectedChannels[i])
					{
						::gtk_tree_selection_select_iter(l_pMultiViewTreeSelection, &l_oIter);
					}
					else
					{
						::gtk_tree_selection_unselect_iter(l_pMultiViewTreeSelection, &l_oIter);
					}
					if(!::gtk_tree_model_iter_next(l_pMultiViewTreeModel, &l_oIter))
					{
						break;
					}
				}
			}

			//finally, show the information dialog
			::gtk_widget_show_all(l_pMultiViewDialog);
		}

		//Called when the user presses the apply button of the channel selection dialog
		void multiViewDialogApplyButtonCallback(::GtkButton *button, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast < CSignalDisplayView* >(data);

			::GtkTreeView* l_pMultiViewTreeView = GTK_TREE_VIEW(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayMultiViewSelectList"));
			::GtkTreeSelection* l_pMultiViewTreeSelection = ::gtk_tree_view_get_selection(l_pMultiViewTreeView);
			::GtkTreeModel* l_pMultiViewTreeModel = ::gtk_tree_view_get_model(l_pMultiViewTreeView);
			::GtkTreeIter l_oIter;

			if(::gtk_tree_model_get_iter_first(l_pMultiViewTreeModel, &l_oIter))
			{
				for(uint32 i=0;i<l_pView->m_vMultiViewSelectedChannels.size();i++)
				{
					l_pView->m_vMultiViewSelectedChannels[i]=(::gtk_tree_selection_iter_is_selected(l_pMultiViewTreeSelection, &l_oIter)?true:false);
					
					if(!::gtk_tree_model_iter_next(l_pMultiViewTreeModel, &l_oIter))
					{
						break;
					}
				}
			}

			l_pView->changeMultiView();
			l_pView->updateMainTableStatus();

			l_pView->redraw(); // immediate redraw

			//hides the channel selection dialog
			::gtk_widget_hide(GTK_WIDGET(::gtk_builder_get_object(l_pView->m_pBuilderInterface, "SignalDisplayMultiViewDialog")));
		}
	}
}
