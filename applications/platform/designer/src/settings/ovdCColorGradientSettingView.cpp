
#include "ovdCColorGradientSettingView.h"
#include "../ovd_base.h"

#include <iostream>
#include <cstring>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace OpenViBEDesigner::Setting;

// round is defined in <cmath> on c++11
inline int ov_round(double dbl)
{ return dbl >= 0.0 ? (int)(dbl + 0.5) : ((dbl - (double)(int)dbl) <= -0.5 ? (int)dbl : (int)(dbl - 0.5));
}

static void on_color_gradient_color_button_pressed(::GtkColorButton* pButton, gpointer pUserData)
{
	static_cast< CColorGradientSettingView *>(pUserData)->colorChange(pButton);
}

static void on_button_setting_color_gradient_configure_pressed(::GtkButton* pButton, gpointer pUserData)
{
	static_cast< CColorGradientSettingView *>(pUserData)->configurePressed();
}

static void on_refresh_color_gradient(::GtkWidget* pWidget, ::GdkEventExpose* pEvent, gpointer pUserData)
{
	static_cast< CColorGradientSettingView *>(pUserData)->refreshColorGradient();
}

static void on_gtk_widget_destroy_cb(::GtkWidget* pWidget, gpointer pUserData)
{
	gtk_widget_destroy(pWidget);
}

static void on_initialize_color_gradient(::GtkWidget* pWidget, gpointer pUserData)
{
	static_cast< CColorGradientSettingView *>(pUserData)->initializeGradient();
}

static void on_button_color_gradient_add_pressed(::GtkButton* pButton, gpointer pUserData)
{
	static_cast< CColorGradientSettingView *>(pUserData)->addColor();
}

static void on_button_color_gradient_remove_pressed(::GtkButton* pButton, gpointer pUserData)
{
	static_cast< CColorGradientSettingView *>(pUserData)->removeColor();
}

static void on_color_gradient_spin_button_value_changed(::GtkSpinButton* pButton, gpointer pUserData)
{
	static_cast< CColorGradientSettingView *>(pUserData)->spinChange(pButton);
}

static void on_change(::GtkEntry *entry, gpointer pUserData)
{
	static_cast<CColorGradientSettingView *>(pUserData)->onChange();
}



CColorGradientSettingView::CColorGradientSettingView(OpenViBE::Kernel::IBox &rBox, OpenViBE::uint32 ui32Index, CString &rBuilderName, const Kernel::IKernelContext &rKernelContext):
	CAbstractSettingView(rBox, ui32Index, rBuilderName, "settings_collection-hbox_setting_color_gradient"), m_rKernelContext(rKernelContext), m_sBuilderName(rBuilderName), m_bOnValueSetting(false)
{
	::GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	std::vector< ::GtkWidget* > l_vWidget;
	extractWidget(l_pSettingWidget, l_vWidget);
	m_pEntry = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(m_pEntry), "changed", G_CALLBACK(on_change), this);
	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_color_gradient_configure_pressed), this);

	initializeValue();
}


void CColorGradientSettingView::getValue(OpenViBE::CString &rValue) const
{
	rValue = CString(gtk_entry_get_text(m_pEntry));
}


void CColorGradientSettingView::setValue(const OpenViBE::CString &rValue)
{
	m_bOnValueSetting = true;
	gtk_entry_set_text(m_pEntry, rValue);
	m_bOnValueSetting =false;
}

void CColorGradientSettingView::configurePressed()
{
	::GtkBuilder* l_pBuilderInterface=gtk_builder_new(); // glade_xml_new(l_oUserData.sGUIFilename.c_str(), "setting_editor-color_gradient-dialog", NULL);
	gtk_builder_add_from_file(l_pBuilderInterface, m_sBuilderName.toASCIIString(), NULL);
	gtk_builder_connect_signals(l_pBuilderInterface, NULL);

	pDialog=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-dialog"));

	CString l_sInitialGradient=m_rKernelContext.getConfigurationManager().expand(gtk_entry_get_text(m_pEntry));
	CMatrix l_oInitialGradient;

	OpenViBEToolkit::Tools::ColorGradient::parse(l_oInitialGradient, l_sInitialGradient);

	vColorGradient.resize(l_oInitialGradient.getDimensionSize(1) > 2 ? l_oInitialGradient.getDimensionSize(1) : 2);
	for(uint32 i=0; i<l_oInitialGradient.getDimensionSize(1); i++)
	{
		vColorGradient[i].fPercent    =l_oInitialGradient[i*4];
		vColorGradient[i].oColor.red  =(guint)(l_oInitialGradient[i*4+1]*.01*65535.);
		vColorGradient[i].oColor.green=(guint)(l_oInitialGradient[i*4+2]*.01*65535.);
		vColorGradient[i].oColor.blue =(guint)(l_oInitialGradient[i*4+3]*.01*65535.);
	}

	pContainer=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-vbox"));
	pDrawingArea=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-drawingarea"));

	g_signal_connect(G_OBJECT(pDialog), "show", G_CALLBACK(on_initialize_color_gradient), this);
	g_signal_connect(G_OBJECT(pDrawingArea), "expose_event", G_CALLBACK(on_refresh_color_gradient), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-add_button")), "pressed", G_CALLBACK(on_button_color_gradient_add_pressed), this);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-remove_button")), "pressed", G_CALLBACK(on_button_color_gradient_remove_pressed), this);

	if(gtk_dialog_run(GTK_DIALOG(pDialog))==GTK_RESPONSE_APPLY)
	{
		CString l_sFinalGradient;
		CMatrix l_oFinalGradient;
		l_oFinalGradient.setDimensionCount(2);
		l_oFinalGradient.setDimensionSize(0, 4);
		l_oFinalGradient.setDimensionSize(1, vColorGradient.size());
		for(uint32 i=0; i<vColorGradient.size(); i++)
		{
			l_oFinalGradient[i*4]   = vColorGradient[i].fPercent;
			l_oFinalGradient[i*4+1] = ov_round(vColorGradient[i].oColor.red   * 100. / 65535.);
			l_oFinalGradient[i*4+2] = ov_round(vColorGradient[i].oColor.green * 100. / 65535.);
			l_oFinalGradient[i*4+3] = ov_round(vColorGradient[i].oColor.blue  * 100. / 65535.);
		}
		OpenViBEToolkit::Tools::ColorGradient::format(l_sFinalGradient, l_oFinalGradient);
		if(!m_bOnValueSetting)
		{
			getBox().setSettingValue(getSettingIndex(), l_sFinalGradient.toASCIIString());
		}
		//gtk_entry_set_text(m_pEntry, l_sFinalGradient.toASCIIString());
	}

	gtk_widget_destroy(pDialog);
	g_object_unref(l_pBuilderInterface);
}


void CColorGradientSettingView::initializeGradient()
{
	gtk_widget_hide(pContainer);

	gtk_container_foreach(GTK_CONTAINER(pContainer), on_gtk_widget_destroy_cb, NULL);

	std::vector < SColorGradientDataNode >::iterator it;

	uint32 i=0;
	uint32 count=vColorGradient.size();
	vColorButtonMap.clear();
	vSpinButtonMap.clear();
	for(it=vColorGradient.begin(); it!=vColorGradient.end(); it++, i++)
	{
		::GtkBuilder* l_pBuilderInterface=gtk_builder_new(); // glade_xml_new(l_pUserData->sGUIFilename.c_str(), "setting_editor-color_gradient-hbox", NULL);
		gtk_builder_add_from_file(l_pBuilderInterface, m_sBuilderName.toASCIIString(), NULL);
		gtk_builder_connect_signals(l_pBuilderInterface, NULL);

		::GtkWidget* l_pWidget=GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-hbox"));

		it->pColorButton=GTK_COLOR_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-colorbutton"));
		it->pSpinButton=GTK_SPIN_BUTTON(gtk_builder_get_object(l_pBuilderInterface, "setting_editor-color_gradient-spinbutton"));

		gtk_color_button_set_color(it->pColorButton, &it->oColor);
		gtk_spin_button_set_value(it->pSpinButton, it->fPercent);

		g_signal_connect(G_OBJECT(it->pColorButton), "color-set", G_CALLBACK(on_color_gradient_color_button_pressed), this);
		g_signal_connect(G_OBJECT(it->pSpinButton), "value-changed", G_CALLBACK(on_color_gradient_spin_button_value_changed), this);

		g_object_ref(l_pWidget);
		gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(l_pWidget)), l_pWidget);
		gtk_container_add(GTK_CONTAINER(pContainer), l_pWidget);
		g_object_unref(l_pWidget);

		g_object_unref(l_pBuilderInterface);

		vColorButtonMap[it->pColorButton]=i;
		vSpinButtonMap[it->pSpinButton]=i;
	}

	gtk_spin_button_set_value(vColorGradient[0].pSpinButton, 0);
	gtk_spin_button_set_value(vColorGradient[count-1].pSpinButton, 100);

	gtk_widget_show(pContainer);
}

void CColorGradientSettingView::refreshColorGradient()
{
	uint32 i;
	uint32 ui32Steps=100;
	gint sizex=0;
	gint sizey=0;
	gdk_drawable_get_size(pDrawingArea->window, &sizex, &sizey);

	CMatrix l_oGradientMatrix;
	l_oGradientMatrix.setDimensionCount(2);
	l_oGradientMatrix.setDimensionSize(0, 4);
	l_oGradientMatrix.setDimensionSize(1, vColorGradient.size());
	for(i=0; i<vColorGradient.size(); i++)
	{
		l_oGradientMatrix[i*4  ]=vColorGradient[i].fPercent;
		l_oGradientMatrix[i*4+1]=vColorGradient[i].oColor.red  *100./65535.;
		l_oGradientMatrix[i*4+2]=vColorGradient[i].oColor.green*100./65535.;
		l_oGradientMatrix[i*4+3]=vColorGradient[i].oColor.blue *100./65535.;
	}

	CMatrix l_oInterpolatedMatrix;
	OpenViBEToolkit::Tools::ColorGradient::interpolate(l_oInterpolatedMatrix, l_oGradientMatrix, ui32Steps);

	::GdkGC* l_pGC=gdk_gc_new(pDrawingArea->window);
	::GdkColor l_oColor;

	for(i=0; i<ui32Steps; i++)
	{
		l_oColor.red  =(guint)(l_oInterpolatedMatrix[i*4+1]*65535*.01);
		l_oColor.green=(guint)(l_oInterpolatedMatrix[i*4+2]*65535*.01);
		l_oColor.blue =(guint)(l_oInterpolatedMatrix[i*4+3]*65535*.01);
		gdk_gc_set_rgb_fg_color(l_pGC, &l_oColor);
		gdk_draw_rectangle(
			pDrawingArea->window,
			l_pGC,
			TRUE,
			(sizex*i)/ui32Steps,
			0,
			(sizex*(i+1))/ui32Steps,
			sizey);
	}
	g_object_unref(l_pGC);
}

void CColorGradientSettingView::addColor()
{
	vColorGradient.resize(vColorGradient.size()+1);
	vColorGradient[vColorGradient.size()-1].fPercent=100;
	initializeGradient();
	refreshColorGradient();
}

void CColorGradientSettingView::removeColor()
{
	if(vColorGradient.size() > 2)
	{
		vColorGradient.resize(vColorGradient.size()-1);
		vColorGradient[vColorGradient.size()-1].fPercent=100;
		initializeGradient();
		refreshColorGradient();
	}
}

void CColorGradientSettingView::spinChange(::GtkSpinButton* pButton)
{
	gtk_spin_button_update(pButton);

	uint32 i=vSpinButtonMap[pButton];
	::GtkSpinButton* l_pPrevSpinButton=(i>                                   0?vColorGradient[i-1].pSpinButton:NULL);
	::GtkSpinButton* l_pNextSpinButton=(i<vColorGradient.size()-1?vColorGradient[i+1].pSpinButton:NULL);
	if(!l_pPrevSpinButton)
	{
		gtk_spin_button_set_value(pButton, 0);
	}
	if(!l_pNextSpinButton)
	{
		gtk_spin_button_set_value(pButton, 100);
	}
	if(l_pPrevSpinButton && gtk_spin_button_get_value(pButton) < gtk_spin_button_get_value(l_pPrevSpinButton))
	{
		gtk_spin_button_set_value(pButton, gtk_spin_button_get_value(l_pPrevSpinButton));
	}
	if(l_pNextSpinButton && gtk_spin_button_get_value(pButton) > gtk_spin_button_get_value(l_pNextSpinButton))
	{
		gtk_spin_button_set_value(pButton, gtk_spin_button_get_value(l_pNextSpinButton));
	}

	vColorGradient[i].fPercent=gtk_spin_button_get_value(pButton);

	refreshColorGradient();
}

void CColorGradientSettingView::colorChange(GtkColorButton *pButton)
{
	::GdkColor l_oColor;
	gtk_color_button_get_color(pButton, &l_oColor);

	vColorGradient[vColorButtonMap[pButton]].oColor=l_oColor;

	refreshColorGradient();
}

void CColorGradientSettingView::onChange()
{
	if(!m_bOnValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_pEntry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}

}



