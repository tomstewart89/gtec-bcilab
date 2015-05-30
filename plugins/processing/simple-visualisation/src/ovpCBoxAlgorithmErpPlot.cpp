#include "ovpCBoxAlgorithmErpPlot.h"
#include <boost/lexical_cast.hpp>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEToolkit;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;


namespace
{
	class _AutoCast_
	{
	public:
		_AutoCast_(IBox& rBox, IConfigurationManager& rConfigurationManager, const uint32 ui32Index) : m_rConfigurationManager(rConfigurationManager) { rBox.getSettingValue(ui32Index, m_sSettingValue); }
		operator ::GdkColor (void)
		{
			::GdkColor l_oColor;
			int r=0, g=0, b=0;
			sscanf(m_sSettingValue.toASCIIString(), "%i,%i,%i", &r, &g, &b);
			l_oColor.pixel=0;
			l_oColor.red=(r*65535)/100;
			l_oColor.green=(g*65535)/100;
			l_oColor.blue=(b*65535)/100;
			// std::cout << r << " " << g << " " << b << "\n";
			return l_oColor;
		}
	protected:
		IConfigurationManager& m_rConfigurationManager;
		CString m_sSettingValue;
	};
}

static void event_handler(GtkWidget *widget, gint width, gint height, gpointer data)
{

		std::list<Graph*>* l_GraphList = reinterpret_cast<std::list<Graph*>*>(data);
		std::list<Graph*>::iterator l_Iterator;
		for ( l_Iterator=l_GraphList->begin(); l_Iterator!=l_GraphList->end(); l_Iterator++)
		{
			(*l_Iterator)->resizeAxis(width, height, l_GraphList->size());
			(*l_Iterator)->draw(widget);
		}
}

static void on_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
		//std::cout << "on_configure_event"<<event->width<<" "<< event->height<<"on widget "<<widget->allocation.width<<" "<<widget->allocation.height << "\n";
		gtk_widget_queue_draw_area(widget,0, 0,event->width, event->height );
		event_handler(widget, event->width, event->height, data);
}

/*
static gboolean on_resize_event(GtkWidget *widget,  GdkRectangle * event, gpointer data)
{
		//std::cout << "on_resize_event" << "\n";
	event_handler(widget, event->width, event->height, data);
	return TRUE;
}
//*/

static gboolean on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	//std::cout << "on_expose_event" <<event->area.width<<" "<< event->area.height<<"on widget "<<widget->allocation.width<<" "<<widget->allocation.height<< "\n";
	//gtk_widget_queue_draw_area(widget,0, 0,event->area.width, event->area.height );
	event_handler(widget, event->area.width, event->area.height, data);

	return FALSE;
}

static void rendertext(cairo_t *cr, const char* text, double x, double y) {

		PangoLayout *layout;							// layout for a paragraph of text
		PangoFontDescription *desc;						// this structure stores a description of the style of font you'd most like
		cairo_identity_matrix(cr);
		cairo_translate(cr, x, y);						// set the origin of cairo instance 'cr' to (10,20) (i.e. this is where
																				// drawing will start from).
		layout = pango_cairo_create_layout(cr);					// init pango layout ready for use
		pango_layout_set_text(layout, text, -1);			// sets the text to be associated with the layout (final arg is length, -1
																				// to calculate automatically when passing a nul-terminated string)
		desc = pango_font_description_from_string("Sans Bold 10");		// specify the font that would be ideal for your particular use
		pango_layout_set_font_description(layout, desc);			// assign the previous font description to the layout
		pango_font_description_free(desc);					// free the description

		pango_cairo_update_layout(cr, layout);					// if the target surface or transformation properties of the cairo instance
																				// have changed, update the pango layout to reflect this
		pango_cairo_show_layout(cr, layout);					// draw the pango layout onto the cairo surface // mandatory

		g_object_unref(layout);							// free the layout
}

void Graph::resizeAxis(gint width, gint height, int nrOfGraphs)
{
	uint32 l_ui32NrOfRows = (uint32)ceil(sqrt((double)nrOfGraphs));
	uint32 l_ui32NrOfColumns = l_ui32NrOfRows;
	if ( (uint32)nrOfGraphs<=(l_ui32NrOfRows-1)*l_ui32NrOfRows)
		l_ui32NrOfRows--;
	this->m_dGraphWidth = ((double)(width))/(double)l_ui32NrOfColumns;
	this->m_dGraphHeight = ((double)(height))/(double)l_ui32NrOfRows;
	this->m_dGraphOriginX = this->m_dGraphWidth*((double)this->colIndex);
	this->m_dGraphOriginY = this->m_dGraphHeight*((double)this->rowIndex);

		//std::cout << "resizeAxis: origin x: " << m_dGraphOriginX << ", origin y: " << m_dGraphOriginY << ", width: " << m_dGraphWidth << ", height: " << m_dGraphHeight << "\n";
}

void Graph::draw(GtkWidget* widget)//cairo_t * cairoContext)
{
	cairo_t * l_pCairoContext;
	l_pCairoContext = gdk_cairo_create(widget->window);

	cairo_set_line_width (l_pCairoContext, 1);
	cairo_translate(l_pCairoContext, m_dGraphOriginX+20, m_dGraphOriginY+20);
	cairo_scale(l_pCairoContext, m_dGraphWidth-40, m_dGraphHeight-40);

	cairo_save(l_pCairoContext);
	drawAxis(l_pCairoContext);
	cairo_restore(l_pCairoContext);

	cairo_save(l_pCairoContext);
	drawLegend(l_pCairoContext);
	cairo_restore(l_pCairoContext);

	cairo_save(l_pCairoContext);
	drawVar(l_pCairoContext);
	cairo_restore(l_pCairoContext);

	cairo_save(l_pCairoContext);
	drawCurves(l_pCairoContext);
	cairo_restore(l_pCairoContext);

	cairo_save(l_pCairoContext);
	drawAxisLabels(l_pCairoContext);
	cairo_restore(l_pCairoContext);

	cairo_destroy(l_pCairoContext);

}

void Graph::drawAxis(cairo_t * cairoContext)
{
	//make background white by drawing white rectangle
	cairo_set_source_rgb(cairoContext, 1.0, 1.0, 1.0);	
		cairo_rectangle(cairoContext, 0, 0, 1, 1);
	cairo_fill(cairoContext);
	
	double ux=1, uy=1;
	cairo_device_to_user_distance (cairoContext, &ux, &uy);
	if (ux < uy)
		ux = uy;
	cairo_set_line_width (cairoContext, ux);

	cairo_set_source_rgb(cairoContext, 0, 0, 0);

	//cairo_save(cairoContext);
	//draw x-axis
	double Xo = 0;
	double Yo = 0.5;
	double Xe = 1.0;
	double Ye = 0.5;
	drawLine(cairoContext, &Xo, &Yo, &Xe, &Ye);

	double dXo = 0;
	double dYo = 0;
	double dXe = 0;
	double dYe = 1.0;
	drawLine(cairoContext, &dXo, &dYo, &dXe, &dYe);
}

void Graph::drawLine(cairo_t *cairoContext, double* Xo, double* Yo, double* Xe, double* Ye)
{
	cairo_save(cairoContext);

	snapCoords(cairoContext, Xo, Yo);
	snapCoords(cairoContext, Xe, Ye);

	cairo_identity_matrix(cairoContext);
	cairo_set_line_width (cairoContext, 1.0);
	cairo_move_to(cairoContext, *Xo, *Yo);
	cairo_line_to(cairoContext, *Xe, *Ye);
	cairo_stroke(cairoContext);
	cairo_restore(cairoContext);

}

void Graph::snapCoords(cairo_t * cairoContext, double* x, double* y )
{
	cairo_user_to_device(cairoContext, x, y);
	*x = ceil(*x) + 0.5;
	*y = ceil(*y) + 0.5;
}

void Graph::drawAxisLabels(cairo_t * cairoContext)
{
	cairo_set_source_rgb(cairoContext, 0, 0, 0);

	float X = 0;
	float Y = 0;

	while (Y <= 1)
	{
		cairo_move_to(cairoContext, 0, Y);

		double cx, cy;
		cairo_get_current_point(cairoContext, &cx, &cy);
		//std::cout<<"current point "<<cx<<" "<<cy<<"\n";
		cairo_user_to_device(cairoContext, &cx, &cy);
		//std::cout<<"device current point "<<cx<<" "<<cy<<"\n";

		float64 YtoPrint = adjustValueToScale( Y );

		std::string Ystr = boost::lexical_cast<std::string>( YtoPrint );
		Ystr.resize(4);//2 significant numbers after the comma
		const char* Ychar;
		Ychar =  Ystr.c_str();


		cairo_save(cairoContext);
		rendertext(cairoContext, Ychar, cx, cy);
		cairo_restore(cairoContext);

		Y += (float)0.1;
	}

	uint64 XBegin = this->StartTime;
	uint64 XEnd = this->EndTime;


	while (X <= 1)
	{
		cairo_move_to(cairoContext, X, 1);

		double cx, cy;
		cairo_get_current_point(cairoContext, &cx, &cy);
		//std::cout<<"current point "<<cx<<" "<<cy<<"\n";
		cairo_user_to_device(cairoContext, &cx, &cy);


		//X value to print range from XBegin to XEnd
		uint64 XtoPrint = (XEnd - XBegin)*(uint64)X + XBegin ;
		float64 l_f64Time = (XtoPrint>>22)/1024.0;
		std::string Xstr = boost::lexical_cast<std::string>( l_f64Time );

		//to always have 2 significant numbers after the comma
		int SignificantNumbers = 4;
		if (l_f64Time !=0)
			SignificantNumbers = (int)log10(l_f64Time)+4;
		Xstr.resize(SignificantNumbers);


		const char* Xchar;
		Xchar =  Xstr.c_str();

		cairo_save(cairoContext);
		rendertext(cairoContext, Xchar, cx, cy);
		cairo_restore(cairoContext);

		X += (float)0.2;
	}

}

void Graph::drawCurves(cairo_t * cairoContext)
{
	double ux=1, uy=1;
	cairo_device_to_user_distance (cairoContext, &ux, &uy);
	if (ux < uy)
		ux = uy;
	cairo_set_line_width (cairoContext, ux);

	for(uint32 gi=0; gi<m_lCurves.size(); gi++)
	{
		cairo_set_source_rgb(cairoContext, (double)m_cLineColor[gi].red/65535.0, 
				(double)m_cLineColor[gi].green/65535.0, 
				(double)m_cLineColor[gi].blue/65535.0);
		float64* l_pIterator = this->begin(gi);

		float64 l_f64Y = *l_pIterator;
		//center
		l_f64Y = adjustValueToScale( *(l_pIterator) );
		float64 l_f64X = 0.0;
		cairo_move_to(cairoContext, l_f64X, l_f64Y);

		for(int si = 1; si<curveSize; si++)
		{
			l_f64Y = adjustValueToScale( *(l_pIterator+si) );
			l_f64X = ((double)si)/((double)curveSize);
			cairo_line_to(cairoContext, l_f64X, l_f64Y);
		}

		cairo_save(cairoContext);
		cairo_identity_matrix(cairoContext);
		cairo_set_line_width (cairoContext, 1.0);
		cairo_stroke(cairoContext);
		cairo_restore(cairoContext);
	}
}

void Graph::drawVar(cairo_t *cairoContext)
{
	double ux=1, uy=1;
	cairo_device_to_user_distance (cairoContext, &ux, &uy);
	if (ux < uy)
		ux = uy;
	cairo_set_line_width (cairoContext, ux);

	for(uint32 gi=0; gi<m_lCurves.size(); gi++)
	{
		cairo_set_source_rgba(cairoContext, (double)m_cLineColor[gi].red/65535.0,
						(double)m_cLineColor[gi].green/65535.0,
						(double)m_cLineColor[gi].blue/65535.0,
							 0.5);
		float64* l_pIterator = this->begin(gi);
		float64* l_pVarianceIterator = m_pStandardDeviation[gi];

		float64 l_f64Y = *l_pIterator;
		float64 Var = *l_pVarianceIterator;
		l_f64Y = adjustValueToScale( *(l_pIterator)-Var );
		float64 l_f64X = 0.0;
		cairo_move_to(cairoContext, l_f64X, l_f64Y);

		for(int si = 1; si<curveSize; si++)
		{
			Var = (*(l_pVarianceIterator+si));
			l_f64Y = adjustValueToScale( *(l_pIterator+si)-Var );
			l_f64X = ((double)si)/((double)curveSize);
			cairo_line_to(cairoContext, l_f64X, l_f64Y);

		}

		cairo_line_to(cairoContext, l_f64X, l_f64Y+2*Var);

		for(int si = curveSize-1; si>0; si--)
		{
			Var = (*(l_pVarianceIterator+si));
			l_f64Y = adjustValueToScale( *(l_pIterator+si)+Var );
			l_f64X = ((double)si)/((double)curveSize);
			cairo_line_to(cairoContext, l_f64X, l_f64Y);

		}
		cairo_fill(cairoContext);
	}

}

void Graph::drawLegend(cairo_t * cairoContext)
{
	double ux=1, uy=1;
	cairo_device_to_user_distance (cairoContext, &ux, &uy);
	cairo_select_font_face(cairoContext, "Purisa",
		  CAIRO_FONT_SLANT_NORMAL,
		  CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size(cairoContext, 0.015);

	for(uint32 gi=0; gi<m_lCurves.size(); gi++)
	{
		cairo_set_source_rgb(cairoContext, (double)m_cLineColor[gi].red/65535.0,
						(double)m_cLineColor[gi].green/65535.0,
						(double)m_cLineColor[gi].blue/65535.0);

		cairo_move_to(cairoContext, 0, 15*uy*gi);

		double cx, cy;
		cairo_get_current_point(cairoContext, &cx, &cy);
		//std::cout<<"current point "<<cx<<" "<<cy<<"\n";
		cairo_user_to_device(cairoContext, &cx, &cy);
		//std::cout<<m_sLineText[gi]<<"\n";
		//cairo_show_text(cairoContext, m_sLineText[gi].toASCIIString());
		cairo_save(cairoContext);
		rendertext(cairoContext, m_sLineText[gi].toASCIIString(), cx, cy);
		cairo_restore(cairoContext);

	}
}

float64 Graph::adjustValueToScale(OpenViBE::float64 value)
{

	//std::cout<<m_pStandardDeviation[m_pArgMinimum.first][m_pArgMinimum.second] <<" "<<m_pStandardDeviation[m_pArgMaximum.first][m_pArgMaximum.second]<<"\n";
	float64 GraphMin = m_pMinimum - m_pStandardDeviation[m_pArgMinimum.first][m_pArgMinimum.second];
	float64 GraphMax = m_pMaximum + m_pStandardDeviation[m_pArgMaximum.first][m_pArgMaximum.second];

	GraphMin = GraphMin-0.10f*std::fabs(GraphMin);
	GraphMax = GraphMax+0.10f*std::fabs(GraphMax);
	return (GraphMin - value)/(GraphMax-GraphMin)+ 1.0f;
}

void Graph::updateCurves(OpenViBE::float64* curve, unsigned int curveIndex)
{
	delete [] m_lCurves[curveIndex];
	m_lCurves[curveIndex] = curve;

	m_pMaximum = -FLT_MAX;
	m_pMinimum = FLT_MAX;
	for (uint32 j=0; j<m_lCurves.size(); j++)
	{
		for(int i = 0; i<curveSize; i++)
		{
			//m_pMaximum = m_lCurves[j][i]>m_pMaximum ? m_lCurves[j][i]:m_pMaximum;
			if (m_lCurves[j][i]>m_pMaximum)
			{
				m_pMaximum = m_lCurves[j][i];
				m_pArgMaximum.first = j;
				m_pArgMaximum.second = i;
			}
			//m_pMinimum = m_lCurves[j][i]<m_pMinimum ? m_lCurves[j][i]:m_pMinimum;
			if (m_lCurves[j][i]<m_pMinimum)
			{
				m_pMinimum = m_lCurves[j][i];
				m_pArgMinimum.first = j;
				m_pArgMinimum.second = i;
			}
		}
		//m_pMaximum = m_pMaximum+0.01f*std::fabs(m_pMaximum);
		//m_pMinimum = m_pMinimum-0.01f*std::fabs(m_pMinimum);
	}
}

void Graph::setVariance(OpenViBE::float64* Variance, unsigned int curveIndex)
{
	delete [] m_pStandardDeviation[curveIndex];
	m_pStandardDeviation[curveIndex] = Variance;
}

float64* Graph::begin(int curveIndex) 
{
	return m_lCurves[curveIndex];
}




boolean CBoxAlgorithmErpPlot::initialize(void)
{	
	// If you need to retrieve setting values, use the FSettingValueAutoCast function.
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	m_sFigureFileName = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_sTriggerToSave = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

	m_lGraphList = new std::list<Graph*>;

	//should be a Graph per channel/electrode not per input (should be done when first header is received)
	for (uint32 i=1; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		if ((i%2)==1)
		{
			uint32 l_ui32Class = i/2 +1;
			m_oLegendColors.push_back(_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), 2*l_ui32Class));
			m_oLegend.push_back(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1+l_ui32Class*2));
			m_vDecoders.push_back( new TStreamedMatrixDecoder<CBoxAlgorithmErpPlot>(*this,i) );
		}
		else
		{
			m_vVarianceDecoders.push_back( new TStreamedMatrixDecoder<CBoxAlgorithmErpPlot>(*this,i) );
		}
	}

	m_oStimulationDecoder = new TStimulationDecoder<CBoxAlgorithmErpPlot>(*this,0);

	//*
	//initialize graphic component
	::GtkBuilder* m_pMainWidgetInterface=gtk_builder_new(); // glade_xml_new(m_sInterfaceFilename.toASCIIString(), "p300-speller-toolbar", NULL);
	GError* error = NULL;
	this->getLogManager() << LogLevel_Trace << "Path to erp.ui " << OpenViBE::Directories::getDataDir() + CString("/plugins/simple-visualisation/erp-plot.ui\n");
	gtk_builder_add_from_file(m_pMainWidgetInterface, OpenViBE::Directories::getDataDir() + "/plugins/simple-visualisation/erp-plot.ui", &error);

	m_pDrawWindow=GTK_WIDGET(gtk_builder_get_object(m_pMainWidgetInterface, "plot-window"));

	this->getBoxAlgorithmContext()->getVisualisationContext()->setWidget(m_pDrawWindow);

	g_signal_connect(m_pDrawWindow, "expose-event", G_CALLBACK (on_expose_event), m_lGraphList);
	g_signal_connect(m_pDrawWindow, "configure-event", G_CALLBACK (on_configure_event), m_lGraphList);


	gtk_widget_show_all(m_pDrawWindow);

	//*/
	m_bFirstHeaderReceived = false;



	return true;
}

boolean CBoxAlgorithmErpPlot::uninitialize(void)
{
	for (uint32 inputi=0; inputi<m_vDecoders.size(); inputi++)
	{
		m_vDecoders[inputi]->uninitialize();
		m_vVarianceDecoders[inputi]->uninitialize();
	}

	m_oStimulationDecoder->uninitialize();

	if (m_pDrawWindow)
	{
		gtk_widget_destroy(m_pDrawWindow);
		m_pDrawWindow = NULL;
	}

	while(!m_lGraphList->empty()) delete m_lGraphList->front(), m_lGraphList->pop_front();

	return true;
}

boolean CBoxAlgorithmErpPlot::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

//saving the graph in png images
boolean CBoxAlgorithmErpPlot::save(void)
{
	//main context
	cairo_t * cairoContext;
	cairoContext = gdk_cairo_create(m_pDrawWindow->window);

	//the main surface
	cairo_surface_t * surface = cairo_get_target(cairoContext);

	//building filename
	OpenViBE::CString CSFilename = m_sFigureFileName;
	std::string extension = ".png";


	std::list<Graph*>::iterator l_Iterator;
	for ( l_Iterator=m_lGraphList->begin(); l_Iterator!=m_lGraphList->end(); l_Iterator++)
	{
		//cutting this graph
		cairo_surface_t * subsurface = cairo_surface_create_for_rectangle  (surface, (*l_Iterator)->m_dGraphOriginX, (*l_Iterator)->m_dGraphOriginY, (*l_Iterator)->m_dGraphWidth, (*l_Iterator)->m_dGraphHeight);

		std::stringstream filename;
		//creating filename
		filename << m_sFigureFileName << ((*l_Iterator)->rowIndex) <<"_"<< ((*l_Iterator)->colIndex) << extension;
		std::string Sfilename;
		filename >> Sfilename;
		this->getLogManager() << LogLevel_Info << "Saving [" << Sfilename.c_str() <<"] \n";
		cairo_surface_write_to_png ( subsurface, Sfilename.c_str() );//m_sFigureFileName.toASCIIString() );
	}

	return true;

}

boolean CBoxAlgorithmErpPlot::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	//listen for stimulation input
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		m_oStimulationDecoder->decode(i);
		if(m_oStimulationDecoder->isBufferReceived())
		{
			IStimulationSet * l_sStimSet = m_oStimulationDecoder->getOutputStimulationSet();
			for (uint32 j=0; j<l_sStimSet->getStimulationCount(); j++)
			{
				if (l_sStimSet->getStimulationIdentifier(j)==m_sTriggerToSave)
				{
					this->getLogManager() << LogLevel_Trace << "Saving\n";
					save();
				}
			}
		}
	}


	for (uint32 inputi=1; inputi<l_rStaticBoxContext.getInputCount(); inputi++)
	{
		for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(inputi); i++)
		{
			if( (inputi)%2 ==1 )
			{
				m_vDecoders[inputi/2]->decode(i);

				if(m_vDecoders[inputi/2]->isHeaderReceived() && !m_bFirstHeaderReceived)
				{
					uint32 l_ui32NrOfElectrodes = m_vDecoders[inputi/2]->getOutputMatrix()->getDimensionSize(0);
					uint32 l_ui32NrOfColumns = (uint32)ceil(sqrt((double)l_ui32NrOfElectrodes));

					//create list of graph subplots
					for (uint32 dimi=0; dimi<l_ui32NrOfElectrodes; dimi++)
					{
						Graph* l_pGraph = (Graph*) new Graph(m_oLegendColors, m_oLegend, (uint32)floor((float)dimi/l_ui32NrOfColumns),
												(uint32)(dimi%l_ui32NrOfColumns), m_vDecoders[inputi/2]->getOutputMatrix()->getDimensionSize(1));
						m_lGraphList->push_back(l_pGraph);
					}

					//draw the empty graphs
					std::list<Graph*>::iterator l_Iterator;
					for ( l_Iterator=m_lGraphList->begin(); l_Iterator!=m_lGraphList->end(); l_Iterator++)
					{

						(*l_Iterator)->StartTime = 0;
						(*l_Iterator)->EndTime = 1;
						//(*l_Iterator)->m_pStandardDeviation = NULL;

						cairo_t * l_pCairoContext;
						l_pCairoContext = gdk_cairo_create(m_pDrawWindow->window);
						(*l_Iterator)->resizeAxis(400, 400, m_lGraphList->size());//default init size
						(*l_Iterator)->drawAxis(l_pCairoContext);
						cairo_destroy(l_pCairoContext);

						cairo_t * l_pCairoContext2;
						l_pCairoContext2 = gdk_cairo_create(m_pDrawWindow->window);
						(*l_Iterator)->drawAxisLabels(l_pCairoContext2);
						cairo_destroy(l_pCairoContext2);

					}
					m_bFirstHeaderReceived = true;


				}
				if(m_vDecoders[inputi/2]->isBufferReceived())
				{
					uint64 ChunkStartTime = l_rDynamicBoxContext.getInputChunkStartTime (inputi, i);
					uint64 ChunkEndTime = l_rDynamicBoxContext.getInputChunkEndTime (inputi, i);

					//redraw all
					//gtk_widget_queue_draw(m_pDrawWindow);

					IMatrix* l_pMatrix = m_vDecoders[inputi/2]->getOutputMatrix();
					uint32 l_ui32NrOfElectrodes = l_pMatrix->getDimensionSize(0);
					uint32 l_ui32NrOfSamples = l_pMatrix->getDimensionSize(1);
					std::list<Graph*>::iterator l_iGraphIterator = m_lGraphList->begin();
					for (uint32 dimi=0; dimi<l_ui32NrOfElectrodes; dimi++, l_iGraphIterator++)
					{
						float64* l_pDestinationMatrix = new float64[l_ui32NrOfSamples];
						System::Memory::copy(l_pDestinationMatrix, l_pMatrix->getBuffer()+dimi*l_ui32NrOfSamples, l_ui32NrOfSamples*sizeof(float64));
						(*l_iGraphIterator)->updateCurves(l_pDestinationMatrix, inputi/2);
						//std::cout << "update curve " << inputi/2 << " beginning value " << l_pDestinationMatrix[0] << ", second value " << l_pDestinationMatrix[42] << "\n";

						(*l_iGraphIterator)->StartTime = ChunkStartTime;
						(*l_iGraphIterator)->EndTime = ChunkEndTime;
						//(*l_iGraphIterator)->draw(m_pDrawWindow);
					}
					l_rDynamicBoxContext.markInputAsDeprecated(inputi,i);

				}
				//if(m_vDecoders[inputi/2]->isEndReceived())
				//{
				//}

			}
			else
			{
				//std::cout<<" variance input"<<(inputi/2-1)<<"\n";
				m_vVarianceDecoders[inputi/2-1]->decode(i);


				if(m_vVarianceDecoders[inputi/2-1]->isBufferReceived())
				{
					IMatrix* l_pMatrix = m_vVarianceDecoders[inputi/2-1]->getOutputMatrix();

					uint32 l_ui32NrOfSamples = l_pMatrix->getDimensionSize(1);
					uint32 l_ui32NrOfElectrodes = l_pMatrix->getDimensionSize(0);

					std::list<Graph*>::iterator l_iGraphIterator = m_lGraphList->begin();
					for (uint32 dimi=0; dimi<l_ui32NrOfElectrodes; dimi++, l_iGraphIterator++)
					{
						float64* l_pDestinationMatrix = new float64[l_ui32NrOfSamples];
						System::Memory::copy(l_pDestinationMatrix, l_pMatrix->getBuffer()+dimi*l_ui32NrOfSamples, l_ui32NrOfSamples*sizeof(float64));
						(*l_iGraphIterator)->setVariance(l_pDestinationMatrix, inputi/2-1);

					}

					l_rDynamicBoxContext.markInputAsDeprecated(inputi,i);

				}
			}
		}
	}


//*
	//redraw all
	gtk_widget_queue_draw(m_pDrawWindow);
	std::list<Graph*>::iterator l_Iterator;
	for ( l_Iterator=m_lGraphList->begin(); l_Iterator!=m_lGraphList->end(); l_Iterator++)
	{
		(*l_Iterator)->draw(m_pDrawWindow);
	}
		//*/

	return true;
}
