#if defined TARGET_HAS_ThirdPartyGUSBampCAPI_Linux

#include "ovasCConfigurationGTecGUSBampLinux.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace std;

/*_________________________________________________

Callbacks to specific widgets
_________________________________________________*/

void button_apply_bipolar_pressed_cb(::GtkButton* pButton, void* pUserData)
{
    CConfigurationGTecGUSBampLinux* l_pConfig=static_cast<CConfigurationGTecGUSBampLinux*>(pUserData);
    l_pConfig->OnButtonApplyConfigPressed(CConfigurationGTecGUSBampLinux::BipolarColumn);
}

void button_apply_bandpass_pressed_cb(::GtkButton* pButton, void* pUserData)
{
    CConfigurationGTecGUSBampLinux* l_pConfig=static_cast<CConfigurationGTecGUSBampLinux*>(pUserData);
    l_pConfig->OnButtonApplyConfigPressed(CConfigurationGTecGUSBampLinux::BandpassColumn);
}

void button_apply_notch_pressed_cb(::GtkButton* pButton, void* pUserData)
{
    CConfigurationGTecGUSBampLinux* l_pConfig=static_cast<CConfigurationGTecGUSBampLinux*>(pUserData);
    l_pConfig->OnButtonApplyConfigPressed(CConfigurationGTecGUSBampLinux::NotchColumn);
}

void button_check_impedance_pressed_cb(::GtkButton* pButton, void* pUserData)
{
    gtk_button_set_label(pButton,"Checking...");
}

void button_check_impedance_clicked_cb(::GtkButton* pButton, void* pUserData)
{
    CConfigurationGTecGUSBampLinux* l_pConfig=static_cast<CConfigurationGTecGUSBampLinux*>(pUserData);
    l_pConfig->OnButtonCheckImpedanceClicked();
}

void combobox_sampling_frequency_changed_cb(::GtkComboBox* pCombobox, void* pUserData)
{
    CConfigurationGTecGUSBampLinux* l_pConfig=static_cast<CConfigurationGTecGUSBampLinux*>(pUserData);
    l_pConfig->OnComboboxSamplingFrequencyChanged();
}

void entry_impedance_activate_cb(::GtkEntry* pEntry, void* pUserData)
{
    // Reset the background colour and stick a question mark on the box so the impedance check function will check this channel
    GdkColor color;
    color.red = color.green = color.blue = 0xFFFF;
    gtk_entry_set_text(pEntry, "?");
    gtk_widget_modify_base(GTK_WIDGET(pEntry), GTK_STATE_NORMAL, &color);
}

/*_________________________________________________*/

// If you added more reference attribute, initialize them here
CConfigurationGTecGUSBampLinux::CConfigurationGTecGUSBampLinux(IDriverContext& rDriverContext, const char* sGtkBuilderFileName, std::string *pDeviceName, gt_usbamp_config *pConfig) :
    CConfigurationBuilder(sGtkBuilderFileName),
    m_rDriverContext(rDriverContext),
    m_pDeviceName(pDeviceName),
    m_pConfig(pConfig)
{
}

boolean CConfigurationGTecGUSBampLinux::preConfigure(void)
{
    char** l_pDeviceList = 0;
    size_t l_ui32ListSize = 0;

    if(!CConfigurationBuilder::preConfigure())
        return false;

    // Refresh and get the list of currently connnected devices
    GT_UpdateDevices();
    l_ui32ListSize = GT_GetDeviceListSize();
    l_pDeviceList = GT_GetDeviceList();

    GtkComboBox* l_pComboBox = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface,"combobox_device"));

    for(unsigned int i = 0; i < l_ui32ListSize; i++)
    {
        gtk_combo_box_append_text(l_pComboBox, l_pDeviceList[i]);
    }

    GT_FreeDeviceList(l_pDeviceList,l_ui32ListSize);

    // If any devices were found at all, set the combo box to the first one listed
    if(l_ui32ListSize)
    {
        gtk_combo_box_set_active(l_pComboBox, 0);
    }

    // This'll update the filter combo boxes
    OnComboboxSamplingFrequencyChanged();

    // Connect all the callbacks
    g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "button_apply_bipolar"), "pressed", G_CALLBACK(button_apply_bipolar_pressed_cb), this);
    g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "button_apply_bandpass"), "pressed", G_CALLBACK(button_apply_bandpass_pressed_cb), this);
    g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "button_apply_notch"), "pressed", G_CALLBACK(button_apply_notch_pressed_cb), this);
    g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "button_check_impedance"), "pressed", G_CALLBACK(button_check_impedance_pressed_cb), this);
    g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "button_check_impedance"), "clicked", G_CALLBACK(button_check_impedance_clicked_cb), this);
    g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_sampling_frequency"), "changed", G_CALLBACK(combobox_sampling_frequency_changed_cb), this);

    // Connect up all the activate methods for the text entries so the user can select them and read the associated channel impedance
    for(int i = 0; i < (GT_USBAMP_NUM_ANALOG_IN + GT_USBAMP_NUM_REFERENCE); i++)
    {
        std::stringstream l_oNameStrm;
        // Compile the string corresponding to the name of the widget displaying the impedance information
        l_oNameStrm << "entry_impedance" << i + 1;
        g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface, l_oNameStrm.str().c_str()), "focus-in-event", G_CALLBACK(entry_impedance_activate_cb), this);
    }

    // Couldn't work out how to do this in the designer, set the treeview box to be able to select multiple items at once
    GtkTreeView* l_pTreeView = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderConfigureInterface,"treeview_channel_config"));
    GtkTreeSelection *l_pSelection = gtk_tree_view_get_selection(l_pTreeView);
    gtk_tree_selection_set_mode(l_pSelection, GTK_SELECTION_MULTIPLE);

    // Now apply all the configs recovered from the settings helper to the GUI - don't have to worry about the sampling rate and number of channels though since they're already taken care of

    // Fill out the analog output configs
    // Shape
    GtkComboBox* l_pComboBoxShape = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_analog_out_shape"));
    gtk_combo_box_set_active(l_pComboBoxShape,m_pConfig->ao_config->shape);
    // Amplitude
    GtkSpinButton* l_pSpinButtonAmplitude = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface,"spinbutton_analog_out_amplitude"));
    gtk_spin_button_set_value(l_pSpinButtonAmplitude, m_pConfig->ao_config->amplitude);
    // Offset
    GtkSpinButton* l_pSpinButtonOffset = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface,"spinbutton_analog_out_offset"));
    gtk_spin_button_set_value(l_pSpinButtonOffset, m_pConfig->ao_config->offset);
    // Frequency
    GtkSpinButton* l_pSpinButtonFrequency = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface,"spinbutton_analog_out_frequency"));
    gtk_spin_button_set_value(l_pSpinButtonFrequency, m_pConfig->ao_config->frequency);

    // Fill out the options
    // Slave
    GtkToggleButton* l_pCheckButtonSlave = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_slave"));
    gtk_toggle_button_set_active(l_pCheckButtonSlave, m_pConfig->slave_mode);
    // Shortcut
    GtkToggleButton* l_pCheckButtonShortcut = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_shortcut"));
    gtk_toggle_button_set_active(l_pCheckButtonShortcut, m_pConfig->enable_sc);
    // Shortcut
    GtkToggleButton* l_pCheckButtonDIO = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_dio"));
    gtk_toggle_button_set_active(l_pCheckButtonDIO, m_pConfig->scan_dio);
    // Trigger
    GtkToggleButton* l_pCheckButtonTrigger = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_trigger"));
    gtk_toggle_button_set_active(l_pCheckButtonTrigger, m_pConfig->enable_trigger_line);
    // Mode
    GtkComboBox* l_pComboBoxMode = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_mode"));
    gtk_combo_box_set_active(l_pComboBoxMode,m_pConfig->mode);

    // Set all the blocks A-D to use the common ground and reference voltages
    for (unsigned int i = 0; i < GT_USBAMP_NUM_GROUND; i++)
    {
        std::stringstream l_oGndNameStrm, l_oRefNameStrm;
        l_oGndNameStrm << "checkbutton_block_gnd" << i + 1;
        l_oRefNameStrm << "checkbutton_block_ref" << i + 1;

        GtkToggleButton* l_pCheckButtonGnd = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, l_oGndNameStrm.str().c_str()));
        GtkToggleButton* l_pCheckButtonRef = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, l_oRefNameStrm.str().c_str()));

        gtk_toggle_button_set_active(l_pCheckButtonGnd, m_pConfig->common_ground[i]);
        gtk_toggle_button_set_active(l_pCheckButtonRef, m_pConfig->common_reference[i]);
    }

    /*
    // Config each channel with the info
    GtkTreeView* l_pTreeView = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderConfigureInterface,"treeview_channel_config"));
    GtkTreeModel* l_pTreeModel = gtk_tree_view_get_model(l_pTreeView);
    gint l_ui32Value;

    GtkTreeIter l_oIter;
    int i = 0;
    for(gboolean l_bEnd = gtk_tree_model_get_iter_first(l_pTreeModel, &l_oIter); l_bEnd; l_bEnd = gtk_tree_model_iter_next(l_pTreeModel, &l_oIter), i++)
    {
        gtk_tree_model_get(l_pTreeModel, &l_oIter, BipolarColumn, &l_ui32Value,-1);
        m_pConfig->bipolar[i] = (l_ui32Value == 0? GT_BIPOLAR_DERIVATION_NONE : l_ui32Value);

        gtk_tree_model_get(l_pTreeModel, &l_oIter, NotchIdColumn, &l_ui32Value, -1);
        m_pConfig->notch[i] = l_ui32Value;

        gtk_tree_model_get(l_pTreeModel, &l_oIter, BandpassIdColumn, &l_ui32Value, -1);
        m_pConfig->bandpass[i] = l_ui32Value;
    }
    */

    return true;
}

void CConfigurationGTecGUSBampLinux::OnButtonApplyConfigPressed(ChannelTreeViewColumn type)
{
    // Get the tree view widget
    GtkTreeView* l_pTreeView = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderConfigureInterface,"treeview_channel_config"));

    // Now get it's model, both as a list store so we can set the entries and it's "base class" tree model so we can iterate through it
    GtkTreeModel* l_pTreeModel = gtk_tree_view_get_model(l_pTreeView);
    GtkListStore* l_pListStore = GTK_LIST_STORE(gtk_builder_get_object(m_pBuilderConfigureInterface,"model_channel_config"));

    // Also get the subset of paths that are selected, if any
    GtkTreeSelection *l_pSelection = gtk_tree_view_get_selection(l_pTreeView);

    // Iterate through them and set the fields
    GtkTreeIter l_oIter;
    for(gboolean end = gtk_tree_model_get_iter_first(l_pTreeModel, &l_oIter); end; end = gtk_tree_model_iter_next(l_pTreeModel, &l_oIter))
    {
        // If the given row is selected
        if(gtk_tree_selection_iter_is_selected(l_pSelection, &l_oIter))
        {
            // Fill in the bipolar field with the spin button contents if the bipolar apply button was pressed
            if(type == BipolarColumn)
            {
                GtkSpinButton* l_pSpinButton = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface,"spinbutton_bipolar"));
                gtk_list_store_set(l_pListStore, &l_oIter, BipolarColumn,  (gint)gtk_spin_button_get_value(l_pSpinButton) ,-1);
            }

            // Fill in the filter field with the combobox contents and fill in a hidden field id with another hidden field in the combobox model that stores the filter's id
            gint l_ui32FilterId;
            GtkTreeIter l_oSelectedComboIter;

            if(type == NotchColumn)
            {
                GtkComboBox* l_pComboBox = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_notch"));

                // Get the iterator for the active row in the combo box's model
                gtk_combo_box_get_active_iter(l_pComboBox, &l_oSelectedComboIter);
                // Get the value of the id column of that row
                gtk_tree_model_get(gtk_combo_box_get_model(l_pComboBox), &l_oSelectedComboIter, 1, &l_ui32FilterId, -1);
                // Put the combo box text in the tree view and the id value in the hidden column of the tree view's model
                gtk_list_store_set(l_pListStore, &l_oIter, NotchColumn,  gtk_combo_box_get_active_text(l_pComboBox), NotchIdColumn, l_ui32FilterId, -1);
            }

            if(type == BandpassColumn)
            {
                GtkComboBox* l_pComboBox = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_bandpass"));

                // Get the iterator for the active row in the combo box's model
                gtk_combo_box_get_active_iter(l_pComboBox, &l_oSelectedComboIter);
                // Get the value of the id column of that row
                gtk_tree_model_get(gtk_combo_box_get_model(l_pComboBox), &l_oSelectedComboIter, 1, &l_ui32FilterId, -1);
                // Put the combo box text in the tree view and the id value in the hidden column of the tree view's model
                gtk_list_store_set(l_pListStore, &l_oIter, BandpassColumn,  gtk_combo_box_get_active_text(l_pComboBox), BandpassIdColumn, l_ui32FilterId,-1);
            }
        }
    }
}

// We'll make this just check the impedance of the selcected box.
void CConfigurationGTecGUSBampLinux::OnButtonCheckImpedanceClicked()
{
    int l_i32Impedance;
    GdkColor l_oColor;

    // Get the name of the selected device
    GtkComboBox* l_pComboBoxDevice = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface,"combobox_device"));
    char* l_pDeviceName = gtk_combo_box_get_active_text(l_pComboBoxDevice);

    // This takes a while so we'll keep the user informed via the console - might have to put this in a thread at some point though
    m_rDriverContext.getLogManager() << LogLevel_Info << "Opening device...\n";

    // Try opening the device
    if(GT_OpenDevice(l_pDeviceName))
    {
        // If that worked then for each channel
        for(unsigned int i = 0; i < (GT_USBAMP_NUM_ANALOG_IN + GT_USBAMP_NUM_REFERENCE); i++)
        {
            std::stringstream l_oNameStrm;
            std::stringstream l_oImpedanceStrm;

            // Reset the color so we don't get one impedance's color bleeding into the next
            l_oColor.red = l_oColor.green = l_oColor.blue = l_oColor.pixel = 0;

            // Compile the string corresponding to the name of the widget displaying the impedance information
            l_oNameStrm << "entry_impedance" << i + 1;

            // Get the relevant text entry
            GtkEntry* l_pText = GTK_ENTRY(gtk_builder_get_object(m_pBuilderConfigureInterface,l_oNameStrm.str().c_str()));

            if(strcmp(gtk_entry_get_text(l_pText),"?") == 0)
            {
                m_rDriverContext.getLogManager() << LogLevel_Info << "Reading from channel " << i + 1 << "...\n";

                // Try to get the impedance for each channel (channels 17 - 20 correspond to references A through D)
                if(GT_GetImpedance(l_pDeviceName, i+1, &l_i32Impedance))
                {
                    // Convert the impedance into kohms
                    l_i32Impedance /= 1000;

                    // impedance is good
                    if(l_i32Impedance < LowImpedance)
                    {
                        l_oColor.green = 0xFFFF;
                    }
                    // impedance is moderate
                    else if(l_i32Impedance < ModerateImpedance)
                    {
                        l_oColor.red = l_oColor.green = 0xFFFF;
                    }
                    // impedance is high
                    else if(l_i32Impedance < HighImpedance)
                    {
                        l_oColor.red = 0xFFFF;
                    }
                    // impedance is so high that the channel is probably not connected
                    else
                    {
                        l_oColor.blue = 0xFFFF;
                    }

                    // If the impedance is larger than 100kohm just write NC
                    if(l_i32Impedance < HighImpedance)
                    {
                        l_oImpedanceStrm << l_i32Impedance;
                    }
                    else
                    {
                        l_oImpedanceStrm << "NC";
                    }

                    // Set the text
                    gtk_entry_set_text(l_pText, l_oImpedanceStrm.str().c_str());
                }
                else
                {
                    // If impedance reading fails, produce an error message and set the relevant cell to black
                    m_rDriverContext.getLogManager() << LogLevel_Error << "Failed to read from channel " << i + 1 << "...\n";
                    l_oColor.red = l_oColor.green = l_oColor.blue = 1;
                    gtk_entry_set_text(l_pText, "");
                }

                // Then get it as just a widget so we can set the background colour
                GtkWidget* l_pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, l_oNameStrm.str().c_str()));
                gtk_widget_modify_base(l_pWidget, GTK_STATE_NORMAL, &l_oColor);
            }
        }

        m_rDriverContext.getLogManager() << LogLevel_Info << "Closing device...\n";
        GT_CloseDevice(l_pDeviceName);
    }
    else
    {
        m_rDriverContext.getLogManager() << LogLevel_Error << "Could not open device\n";
    }

    // Set the text back so the user knows the test is over
    GtkButton* l_pButtonCheckImpedance = GTK_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface,"button_check_impedance"));
    gtk_button_set_label(l_pButtonCheckImpedance,"Check Impedance");
}

// Open the device, get all the possible filters given the sampling frequency and we also blank out all the preset filters since they might not be valid or the same for the new frequency
void CConfigurationGTecGUSBampLinux::OnComboboxSamplingFrequencyChanged()
{    
    GtkListStore* l_pListStoreChannelConfig = GTK_LIST_STORE(gtk_builder_get_object(m_pBuilderConfigureInterface,"model_channel_config"));
    GtkTreeView* l_pTreeView = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderConfigureInterface,"treeview_channel_config"));
    GtkTreeModel* l_pTreeModel = gtk_tree_view_get_model(l_pTreeView);
    GtkTreeIter l_oIter;

    // If the sampling frequency changes the filters may no longer be valid, so clear them all and have the user start again
    for(gboolean end = gtk_tree_model_get_iter_first(l_pTreeModel, &l_oIter); end; end = gtk_tree_model_iter_next(l_pTreeModel,&l_oIter))
    {
        gtk_list_store_set(l_pListStoreChannelConfig, &l_oIter, NotchColumn, "none",-1);
        gtk_list_store_set(l_pListStoreChannelConfig, &l_oIter, NotchIdColumn,  GT_FILTER_NONE,-1);

        gtk_list_store_set(l_pListStoreChannelConfig, &l_oIter, BandpassColumn,  "none",-1);
        gtk_list_store_set(l_pListStoreChannelConfig, &l_oIter, BandpassIdColumn,  GT_FILTER_NONE,-1);
    }

    // Get pointers to the comboboxes
    GtkComboBox* l_pComboBoxDevice = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_device"));
    GtkComboBox* l_pComboBoxSamplingFrequency = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_sampling_frequency"));

    // Get the device name and the sample rate from the comboboxes
    char* l_pDeviceName = gtk_combo_box_get_active_text(l_pComboBoxDevice);
    gt_size l_ui32SampleRate = (gt_size)strtol(gtk_combo_box_get_active_text(l_pComboBoxSamplingFrequency),NULL,10);

    // This takes a while so we'll keep the user informed via the console - might have to put this in a thread at some point though
    m_rDriverContext.getLogManager() << LogLevel_Info << "Opening device...\n";

    // Try opening the device
    if(GT_OpenDevice(l_pDeviceName))
    {
        GtkTreeIter l_oIterBandpass, l_oIterNotch;

        GtkListStore* l_pListStoreBandpass = GTK_LIST_STORE(gtk_builder_get_object(m_pBuilderConfigureInterface,"model_bandpass"));
        GtkListStore* l_pListStoreNotch = GTK_LIST_STORE(gtk_builder_get_object(m_pBuilderConfigureInterface,"model_notch"));

        // Clear all the entries from the filter comboboxes
        gtk_list_store_clear(l_pListStoreBandpass);
        gtk_list_store_clear(l_pListStoreNotch);

        // Add the none and autoset configurations back in
        // none
        gtk_list_store_append(l_pListStoreBandpass, &l_oIterBandpass);
        gtk_list_store_set(l_pListStoreBandpass, &l_oIterBandpass, 0, "none", 1, GT_FILTER_NONE, -1);
        gtk_list_store_append(l_pListStoreNotch, &l_oIterNotch);
        gtk_list_store_set(l_pListStoreNotch, &l_oIterNotch, 0, "none", 1, GT_FILTER_NONE, -1);

        // autoset
        gtk_list_store_append(l_pListStoreBandpass, &l_oIterBandpass);
        gtk_list_store_set(l_pListStoreBandpass, &l_oIterBandpass, 0, "autoset", 1, GT_FILTER_AUTOSET, -1);
        gtk_list_store_append(l_pListStoreNotch, &l_oIterNotch);
        gtk_list_store_set(l_pListStoreNotch, &l_oIterNotch, 0, "autoset", 1, GT_FILTER_AUTOSET, -1);

        // Set the combo boxes to show none, it's a bit tidier that way
        gtk_combo_box_set_active(GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_bandpass")),0);
        gtk_combo_box_set_active(GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_notch")),0);

        // Get the sizes of the lists
        gt_size l_ui32BandpassListSize = GT_GetBandpassFilterListSize(l_pDeviceName, l_ui32SampleRate);
        gt_size l_ui32NotchListSize = GT_GetNotchFilterListSize(l_pDeviceName, l_ui32SampleRate);

        // Allocate them
        gt_filter_specification* l_pBandpassList = new gt_filter_specification[l_ui32BandpassListSize];
        gt_filter_specification* l_pNotchList = new gt_filter_specification[l_ui32NotchListSize];

        // Get the lists themselves - note the last parameter to these two should be specified in bytes
        GT_GetBandpassFilterList(l_pDeviceName, l_ui32SampleRate, l_pBandpassList, l_ui32BandpassListSize * sizeof(gt_filter_specification));
        GT_GetNotchFilterList(l_pDeviceName, l_ui32SampleRate, l_pNotchList, l_ui32NotchListSize * sizeof(gt_filter_specification));

        // Repopulate the comboboxes - for each returned filter make a description and put it in the combobox
        // Bandpass
        for(unsigned int i = 0; i < l_ui32BandpassListSize; i++)
        {
            std::stringstream l_oFilterDescStrm;
            l_oFilterDescStrm << "HP: " << l_pBandpassList[i].f_lower << " / LP: " << l_pBandpassList[i].f_upper; // HP = High Pass, LP = Low Pass
            gtk_list_store_append(l_pListStoreBandpass, &l_oIterBandpass);
            gtk_list_store_set(l_pListStoreBandpass, &l_oIterBandpass, 0, l_oFilterDescStrm.str().c_str(), 1, l_pBandpassList[i].id, -1);
        }
        // Notch
        for(unsigned int i = 0; i < l_ui32NotchListSize; i++)
        {
            std::stringstream l_oFilterDescStrm;
            l_oFilterDescStrm << "HS: " << l_pNotchList[i].f_lower << " / LS: " << l_pNotchList[i].f_upper; // HS = High Stop, LS = Low Stop
            gtk_list_store_append(l_pListStoreNotch, &l_oIterNotch);
            gtk_list_store_set(l_pListStoreNotch, &l_oIterNotch, 0, l_oFilterDescStrm.str().c_str(), 1, l_pNotchList[i].id, -1);
        }

        GT_CloseDevice(l_pDeviceName);
    }
    else
    {
        m_rDriverContext.getLogManager() << LogLevel_Error << "Could not open device\n";
    }
}

boolean CConfigurationGTecGUSBampLinux::postConfigure(void)
{
    if(m_bApplyConfiguration)
    {
        // Fill in all the parts of the config that differ from the default configuration we've set out in the CGTecGUSBampLinux constructor
        GtkComboBox* l_pComboBoxDevice = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_device"));

        // If there's any active text in the device combo box then set it to the
        if(char* l_pDeviceName = gtk_combo_box_get_active_text(l_pComboBoxDevice))
        {
            *m_pDeviceName = l_pDeviceName;
        }

        // Get the sample rate and the number of channels
        GtkComboBox* l_pComboBoxSamplingFrequency = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_sampling_frequency"));
        m_pConfig->sample_rate = (gt_size)strtol(gtk_combo_box_get_active_text(l_pComboBoxSamplingFrequency),NULL,10);

        GtkSpinButton *l_pChannelSpinButton = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_number_of_channels"));
        m_pConfig->num_analog_in = gtk_spin_button_get_value(l_pChannelSpinButton);

        // Fill out the analog output configs
        // Shape
        GtkComboBox* l_pComboBoxShape = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_analog_out_shape"));
        m_pConfig->ao_config->shape = (usbamp_analog_out_shape)(gtk_combo_box_get_active(l_pComboBoxShape));
        // Amplitude
        GtkSpinButton* l_pSpinButtonAmplitude = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface,"spinbutton_analog_out_amplitude"));
        m_pConfig->ao_config->amplitude = gtk_spin_button_get_value(l_pSpinButtonAmplitude);
        // Offset
        GtkSpinButton* l_pSpinButtonOffset = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface,"spinbutton_analog_out_offset"));
        m_pConfig->ao_config->offset = gtk_spin_button_get_value(l_pSpinButtonOffset);
        // Frequency
        GtkSpinButton* l_pSpinButtonFrequency = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface,"spinbutton_analog_out_frequency"));
        m_pConfig->ao_config->frequency = gtk_spin_button_get_value(l_pSpinButtonFrequency);

        // Fill out the options
        // Slave
        GtkToggleButton* l_pCheckButtonSlave = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_slave"));
        m_pConfig->slave_mode = gtk_toggle_button_get_active(l_pCheckButtonSlave);
        // Shortcut
        GtkToggleButton* l_pCheckButtonShortcut = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_shortcut"));
        m_pConfig->enable_sc = gtk_toggle_button_get_active(l_pCheckButtonShortcut);
        // Shortcut
        GtkToggleButton* l_pCheckButtonDIO = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_dio"));
        m_pConfig->scan_dio = gtk_toggle_button_get_active(l_pCheckButtonDIO);
        // Trigger
        GtkToggleButton* l_pCheckButtonTrigger = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_trigger"));
        m_pConfig->enable_trigger_line = gtk_toggle_button_get_active(l_pCheckButtonTrigger);
        // Mode
        GtkComboBox* l_pComboBoxMode = GTK_COMBO_BOX(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_mode"));
        m_pConfig->mode = (usbamp_device_mode)(gtk_combo_box_get_active(l_pComboBoxMode));

        // Set all the blocks A-D to use the common ground and reference voltages
        for (unsigned int i = 0; i < GT_USBAMP_NUM_GROUND; i++)
        {
            std::stringstream l_oGndNameStrm, l_oRefNameStrm;
            l_oGndNameStrm << "checkbutton_block_gnd" << i + 1;
            l_oRefNameStrm << "checkbutton_block_ref" << i + 1;

            GtkToggleButton* l_pCheckButtonGnd = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, l_oGndNameStrm.str().c_str()));
            GtkToggleButton* l_pCheckButtonRef = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, l_oRefNameStrm.str().c_str()));

            m_pConfig->common_ground[i] = gtk_toggle_button_get_active(l_pCheckButtonGnd);
            m_pConfig->common_reference[i] = gtk_toggle_button_get_active(l_pCheckButtonRef);
        }

        // Config each channel with the info
        GtkTreeView* l_pTreeView = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilderConfigureInterface,"treeview_channel_config"));
        GtkTreeModel* l_pTreeModel = gtk_tree_view_get_model(l_pTreeView);
        gint l_ui32Value;

        GtkTreeIter l_oIter;
        int i = 0;
        for(gboolean l_bEnd = gtk_tree_model_get_iter_first(l_pTreeModel, &l_oIter); l_bEnd; l_bEnd = gtk_tree_model_iter_next(l_pTreeModel, &l_oIter), i++)
        {
            gtk_tree_model_get(l_pTreeModel, &l_oIter, BipolarColumn, &l_ui32Value,-1);
            m_pConfig->bipolar[i] = (l_ui32Value == 0? GT_BIPOLAR_DERIVATION_NONE : l_ui32Value);

            gtk_tree_model_get(l_pTreeModel, &l_oIter, NotchIdColumn, &l_ui32Value, -1);
            m_pConfig->notch[i] = l_ui32Value;

            gtk_tree_model_get(l_pTreeModel, &l_oIter, BandpassIdColumn, &l_ui32Value, -1);
            m_pConfig->bandpass[i] = l_ui32Value;
        }
    }

    if(!CConfigurationBuilder::postConfigure()) // normal header is filled (Subject ID, Age, Gender, channels, sampling frequency), ressources are realesed
        return false;

    return true;
}

#endif // TARGET_HAS_ThirdPartyGUSBampCAPI_Linux
