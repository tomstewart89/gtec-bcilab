#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "glGSymbol.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

#define REFFONTSIZE 10

GSymbol::GSymbol(const char * symbol, OpenViBE::CString fontPath, OpenViBE::float32 fontScaleSize) : GLabel(fontPath, fontScaleSize) 
{		
	m_sTextLabel = std::string(symbol);	

	cairo_t *layout_context = create_layout_context();
	m_olayout = pango_cairo_create_layout(layout_context);
	cairo_destroy (layout_context);
	/* Load the font */
	PangoFontDescription *desc;

	m_sFontFile = std::string(fontPath.toASCIIString());

#if defined TARGET_OS_Windows
	m_sFontFile = std::string("Sans 100");//can't just use a ttf file on windows, need to register the font. That'll do for now
#endif

	desc = pango_font_description_from_string (m_sFontFile.c_str());
	pango_font_description_set_size(desc, gint(m_f32LabelScaleSize*m_f32MaxLabelSize*PANGO_SCALE));
	pango_layout_set_font_description (m_olayout, desc);
	pango_font_description_free (desc);
	generateGLDisplayLists();
}

GSymbol::GSymbol(const GSymbol& gsymbol) : GLabel(gsymbol)
{
	assignHelper(gsymbol);
}

GSymbol::~GSymbol()
{
	//m_ftglFont.reset();
	//g_object_unref(m_olayout);
}

/*GSymbol& GSymbol::operator= (GSymbol const& gsymbol)
{
	//std::cout << this->m_sTextLabel << " will be replaced by " << gsymbol.m_sTextLabel << "\n";
	if(this!=&gsymbol)
	{
		//std::cout << "operator= " << gsymbol.toString() << "\n";
		this->GLabel::operator=(gsymbol);
		assignHelper(gsymbol);
	}
	return *this;
}*/

void GSymbol::draw()
{
	if (isChanged())
	{
		GLabel::draw();
		//rendering font with given foreground color, if this would be in the display list we should recreate
		//the display list each time we change color as well, this way we don't have to do that
		glColor3f(m_cForegroundColor.red,m_cForegroundColor.green,m_cForegroundColor.blue);
		glCallList(getGLResourceID(1));	
	}
}

void GSymbol::setDimParameters(BoxDimensions dim)
{
	/*
	* This will call the setDimParameters from the super class GLabel which will recompute the maximum label size 
	* (by calling the computeMaximumLabelSize of this class GSymbol) and will call the generateGLDisplayLists
	* of this class GSymbol (which also calls the generateGLDisplayLists of GLabel). This is because these functions
	* are all virtual and first the derived class function is called before the base class function. So we don't have
	* to call generateGLDisplayLists and computeMaximumLabelSize here again it is implicitely done
	*/	
	GLabel::setDimParameters(dim);
}

/*void GSymbol::setLabelScaleSize(float32 fontScaleSize)
{
	m_f32LabelScaleSize = fontScaleSize;
	m_ftglFont->FaceSize(m_f32LabelScaleSize*m_f32MaxLabelSize);
	computeLabelPosition();
}	*/

/*void GSymbol::setSourceFile(OpenViBE::CString sourceFile)
{
	if (sourceFile!=m_sSourceFile)
	{
		m_sSourceFile = sourceFile;
		m_ftglFont.reset();
		m_ftglFont = createFont(sourceFile.toASCIIString());
		computeMaximumLabelSize();
		setFontSize();
		generateGLDisplayLists();		
	}
}*/

/*void GSymbol::setLabelScaleSize(OpenViBE::float32 labelScaleSize) 
{
	GLabel::setLabelScaleSize(labelScaleSize);
	deleteAndCreateGLResources();
	generateGLDisplayLists(); //should be in GLabel then we probably don't need this specialisation
}*/

void GSymbol::setTextLabel(const char * symbol)
{
	m_sTextLabel = std::string(symbol); 
	computeMaximumLabelSize();
    //we have to recreate the OpenGL resources of course and rewrite the code that should be saved in graphical memory
	deleteAndCreateGLResources();
	generateGLDisplayLists();
}

cairo_t* GSymbol::create_layout_context()
{
	cairo_surface_t *temp_surface;
	cairo_t *context;

	temp_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 0, 0);
	context = cairo_create (temp_surface);
	cairo_surface_destroy (temp_surface);

	return context;
}

void GSymbol::get_text_size (PangoLayout *layout, int *width, int *height, PangoRectangle* ink, PangoRectangle* logical)
{
	if (layout!=NULL)
	{
		//PangoRectangle ink, logical;
		pango_layout_get_size(layout, width, height);
		if(ink!=NULL)
		{
			pango_layout_get_extents(layout, ink, NULL);
			*width = (int)ink->width;
			*height = (int)ink->height;
		}
		/* Divide by pango scale to get dimensions in pixels. */
		*width /= PANGO_SCALE;
		*height /= PANGO_SCALE;
	}
}

void GSymbol::render_text(const char *text, int *text_width, int *text_height, unsigned int *texture_id)
{
		//cairo_t *layout_context;
		cairo_t *render_context;
		//cairo_surface_t *temp_surface;
		cairo_surface_t *surface;
		unsigned char* surface_data = NULL;

		pango_layout_set_text(m_olayout, text, -1);

		surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, (*text_width), (*text_height));
		render_context = cairo_create(surface);

		//surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, *text_width, *text_height);
		//render_context = cairo_create(surface);

		/* Render */
		//cairo_set_operator(render_context, CAIRO_OPERATOR_OVER);
		//cairo_set_source_rgba (render_context, 0.5, 1, 0.5, 1);
		//*
		//GColor back = this->getBackgroundColor();
		GColor fore = this->getForegroundColor();
		cairo_set_source_rgba (render_context, fore.red, fore.green, fore.blue, 0);
		//cairo_rectangle(render_context, 0, 0, *text_width, *text_height);
		//cairo_fill(render_context);
		//*/
		cairo_set_operator(render_context, CAIRO_OPERATOR_OVER);
		cairo_paint_with_alpha(render_context, 0);
		//*
		cairo_save(render_context);
		{

			cairo_set_source_rgba (render_context, fore.red, fore.green, fore.blue, 1);
			PangoLayout *layout;							// layout for a paragraph of text
			PangoFontDescription *desc;						// this structure stores a description of the style of font you'd most like
			cairo_identity_matrix(render_context);
			//cairo_translate(render_context, -5, -50);						// set the origin of cairo instance 'cr' to (10,20) (i.e. this is where
																					// drawing will start from).
			layout = pango_cairo_create_layout(render_context);					// init pango layout ready for use
			pango_layout_set_text(layout, text, -1);			// sets the text to be associated with the layout (final arg is length, -1
																					// to calculate automatically when passing a nul-terminated string)
			desc = pango_font_description_from_string(pango_font_description_to_string(pango_layout_get_font_description(m_olayout)));//("Sans Bold 100");		// specify the font that would be ideal for your particular use
			gint size =  gint( (m_f32LabelScaleSize*m_f32MaxLabelSize*PANGO_SCALE)/1.387535079);
			pango_font_description_set_size(desc,size);//*10000);
			pango_layout_set_font_description(layout, desc);			// assign the previous font description to the layout
			pango_font_description_free(desc);					// free the description

			pango_cairo_update_layout(render_context, layout);					// if the target surface or transformation properties of the cairo instance
																					// have changed, update the pango layout to reflect this
			pango_cairo_show_layout(render_context, layout);					// draw the pango layout onto the cairo surface // mandatory

			g_object_unref(layout);
		}
		cairo_restore(render_context);
		//*/
		//pango_cairo_update_layout(render_context, m_olayout);
		//pango_cairo_show_layout(render_context, m_olayout);
		surface_data = cairo_image_surface_get_data(surface);

		*texture_id = create_texture((*text_width), (*text_height), surface_data);

		/* Render a texture in immediate mode. */
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
		//glClear (GL_COLOR_BUFFER_BIT);
		glPushMatrix ();

		glEnable(GL_BLEND);//enable transparancy
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindTexture(GL_TEXTURE_2D, *texture_id);
		

		glBegin(GL_QUADS);
			glTexCoord2i(0, 0);glVertex3f(m_pLabelPosition.first, m_pLabelPosition.second+(*text_height), getDepth()+0.01f);
			glTexCoord2i(1, 0);glVertex3f(m_pLabelPosition.first+(*text_width), m_pLabelPosition.second+(*text_height), getDepth()+0.01f);
			glTexCoord2i(1, 1);glVertex3f(m_pLabelPosition.first+(*text_width), m_pLabelPosition.second, getDepth()+0.01f);
			glTexCoord2i(0, 1);glVertex3f(m_pLabelPosition.first, m_pLabelPosition.second, getDepth()+0.01f);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);//bind back, to default texture, otherwise only the current letter is drawn

		glPopMatrix ();
		/* Clean up */
		//*
		cairo_destroy(render_context);
		//*/

}

unsigned int GSymbol::create_texture ( int width, int height, unsigned char *pixels)
{
	unsigned int texture_id;

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	//*
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	//
	#if defined TARGET_OS_Windows
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
	#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
	#endif
	return texture_id;
}

void GSymbol::computeMaximumLabelSize()
{
	//using a reference string and reference font size because even with monospace 
	//fonts the bounding box width of each character is still different
	std::string l_sRefString = "";
	for (uint32 i=0; i<m_sTextLabel.length(); i++)
		l_sRefString += "X";
	
	PangoFontDescription *desc;
	desc = pango_font_description_from_string(m_sFontFile.c_str());
	pango_font_description_set_size(desc, REFFONTSIZE*PANGO_SCALE);
	pango_layout_set_font_description(m_olayout, desc);			// assign the previous font description to the layout
	//pango_font_description_free(desc);

	pango_layout_set_text (m_olayout, l_sRefString.c_str(), -1);
	 int text_width;
	 int text_height;
	 PangoRectangle ink, logical;
	get_text_size (m_olayout, &text_width, &text_height, &ink, &logical);
	//float32 ratio = 1.387535079*getWidth()/((float32)text_width);
	float32 ratio = getWidth()/((float32)text_width);
	//my best explanation is taht 1.387535079 is the pango/ftgl ratio since their respective point for font size do not match
	m_f32MaxLabelSize = 0.9f*REFFONTSIZE*ratio;

	pango_font_description_set_size(desc, (gint)m_f32MaxLabelSize*PANGO_SCALE);
	pango_layout_set_font_description(m_olayout, desc);
	get_text_size (m_olayout, &text_width, &text_height, &ink, &logical);
	float32 heightratio = (float32)1.387535079*getHeight()/(float32)(text_height);
	if(heightratio<1)
	{
		m_f32MaxLabelSize = 0.9f*m_f32MaxLabelSize*heightratio;
	}
}

void GSymbol::computeLabelPosition()
{
	int text_width;
	int text_height;
	PangoRectangle ink, logical;


	

	pango_layout_set_text(m_olayout, m_sTextLabel.c_str(), -1);
	pango_layout_get_size(m_olayout, NULL, NULL);
	get_text_size(m_olayout, &text_width, &text_height, &ink, &logical);//get the layout?

	float32	l_fxOffset = (getWidth()-(text_width))/2.0f - ink.x/PANGO_SCALE;//l_oBoundingBox.Lower().Xf();
	float32 l_fyOffset = (getHeight()-(text_height))/2.0f - (ink.y)/PANGO_SCALE;//l_oBoundingBox.Lower().Yf();

	m_pLabelPosition.first = getX()+l_fxOffset;
	m_pLabelPosition.second = getY()+l_fyOffset;
	//std::cout << "GSymbol::computeLabelPosition() for label " << m_sTextLabel.c_str() << " got position " << m_pLabelPosition.first << " and " << m_pLabelPosition.second  <<std::endl;
}

/*void GSymbol::setFontSize()
{
	m_ftglFont->FaceSize(static_cast<uint32>(m_f32LabelScaleSize*m_f32MaxLabelSize));
	computeLabelPosition();
}*/

void GSymbol::generateGLDisplayLists()
{
	unsigned int texture_id;
	GLabel::generateGLDisplayLists();
	gint size =  gint( (m_f32LabelScaleSize*m_f32MaxLabelSize*PANGO_SCALE)/1.387535079);
	PangoFontDescription *desc;
	desc = pango_font_description_from_string(m_sFontFile.c_str());//("Sans Bold 100");		// specify the font that would be ideal for your particular use
	pango_font_description_set_size(desc, size);
	pango_layout_set_font_description(m_olayout, desc);			// assign the previous font description to the layout
	pango_font_description_free(desc);
	computeLabelPosition();

	int offx = (int)(m_pLabelPosition.first - getX());
	int offy = (int)(m_pLabelPosition.second-getY());
	int text_width = (int)(getWidth()-2*offx);
	int text_height = (int)(getHeight()-2*offy);

	glNewList(getGLResourceID(1),GL_COMPILE); 
		glLoadIdentity();
		render_text(m_sTextLabel.c_str(), &text_width, &text_height, &texture_id);
	glEndList();
}

void GSymbol::assignHelper(GSymbol const& gsymbol)
{
	this->m_sTextLabel = std::string(gsymbol.m_sTextLabel);
	this->m_olayout = gsymbol.m_olayout;
}

std::string GSymbol::toString() const
{
	std::string text;
	text = GLabel::toString(); 
	text += m_sTextLabel; 
	text += std::string(" ");
	return text;
}
#endif

#endif
