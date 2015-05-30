#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "glGPictureSymbol.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;	

GPictureSymbol::GPictureSymbol(OpenViBE::CString sourcePath, OpenViBE::float32 scaleSize) : GLabel(sourcePath, scaleSize) 
{	
	m_pTextureManager = new OpenGLTextureManager(sourcePath);
	//if the source file was not known yet, we have to call the opengl initialisation code for creating the texture
	if (!m_pTextureManager->sourceExists())
		initializeOpenGLTexture(); // TODO maybe this should be handled by the OpenGLTextureManager
	else
		m_bImageLoaded = true;
		
	generateGLDisplayLists();
}

GPictureSymbol::GPictureSymbol(const GPictureSymbol& gsymbol) : GLabel(gsymbol)
{
	assignHelper(gsymbol);
}

GPictureSymbol::~GPictureSymbol()
{
	delete m_pTextureManager;
}

/*GPictureSymbol& GPictureSymbol::operator= (GPictureSymbol const& gsymbol)
{
	if(this!=&gsymbol)
	{
		std::cout << "operator= " << gsymbol.toString() << "\n";
		this->GLabel::operator=(gsymbol);
		delete m_pTextureManager;
		assignHelper(gsymbol);
	}
	return *this;
}*/

// TODO maybe this should be moved to the OpenGLTextureManager
void GPictureSymbol::initializeOpenGLTexture()
{
	//SDL_Surface* l_pSurface = IMG_Load(m_sSourceFile);
	GdkPixbuf* l_pPixBuff = gdk_pixbuf_new_from_file(m_sSourceFile, NULL);
	m_bImageLoaded = true;
	if (!l_pPixBuff)
	{
		std::cout << "GPictureSymbol: could not load picture " << m_sSourceFile.toASCIIString() << "\n";
		m_bImageLoaded = false;
		//SDL_FreeSurface(l_pSurface);
		g_object_unref(l_pPixBuff);
	}
	else
	{
		//std::cout << "image mode " << gdk_pixbuf_get_bits_per_sample(l_pPixBuff) << " " << (int)l_pSurface->format->BytesPerPixel << std::endl << " has alpha " << gdk_pixbuf_get_has_alpha(l_pPixBuff) << std::endl;
		if (gdk_pixbuf_get_bits_per_sample(l_pPixBuff)/2==3)//(l_pSurface->format->BytesPerPixel == 3)
			m_iMode = GL_RGB;
		else if (gdk_pixbuf_get_bits_per_sample(l_pPixBuff)/2==4)//(l_pSurface->format->BytesPerPixel == 4)
			m_iMode = GL_RGBA;
		else 
		{
			m_bImageLoaded = false;
			//SDL_FreeSurface(l_pSurface);
			g_object_unref(l_pPixBuff);
			std::cout << "Something went wrong when trying to determine BMP mode\n";
		}
		//create one texture name
		if(m_bImageLoaded)
		{
			//SDL_SetAlpha(l_pSurface, SDL_SRCALPHA | SDL_RLEACCEL, 50);

			//tell opengl to use the generated texture name
			glBindTexture(GL_TEXTURE_2D, m_pTextureManager->getResource(0));

			//this reads from the sdl surface and puts it into an opengl texture
			glTexImage2D(GL_TEXTURE_2D, 0, m_iMode, gdk_pixbuf_get_width(l_pPixBuff), gdk_pixbuf_get_height(l_pPixBuff), 0, m_iMode, GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(l_pPixBuff));
			//glTexImage2D(GL_TEXTURE_2D, 0, m_iMode, l_pSurface->w, l_pSurface->h, 0, m_iMode, GL_UNSIGNED_BYTE, l_pSurface->pixels);

			//these affect how this texture is drawn later on...
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			
			glBindTexture(GL_TEXTURE_2D, 0);
			
			g_object_unref(l_pPixBuff);//SDL_FreeSurface(l_pSurface);
		}
	}
}

void GPictureSymbol::draw()
{
	if (isChanged())
	{
		GLabel::draw();
		if (m_bImageLoaded)
		{
			glColor3d(m_cForegroundColor.red,m_cForegroundColor.green,m_cForegroundColor.blue);
			glCallList(getGLResourceID(1));
		}
	}
}

void GPictureSymbol::setDimParameters(BoxDimensions dim)
{	
	GLabel::setDimParameters(dim);
	//every time we set new dimensions to the object we should recreate the display list code in graphical memory
	//by calling the setDimParameters of GLabel we recreate the OpenGL resources
	generateGLDisplayLists();
}

/*void GPictureSymbol::setSourceFile(OpenViBE::CString sourceFile)
{
	if (sourceFile!=m_sSourceFile)
	{
		m_sSourceFile = sourceFile;
		initializeOpenGLTexture();
	}
}*/

void GPictureSymbol::generateGLDisplayLists()
{
	//setLabelDimParameters();//this should not happen for each call to draw because it will regenerate the vbo or display buffers
	computeLabelPosition();
	float32 l_f32PictureWidth = m_f32MaxLabelSize*m_f32LabelScaleSize;
	float32 l_f32PictureHeight = l_f32PictureWidth;
	
	GLabel::generateGLDisplayLists();
	
	glNewList(getGLResourceID(1),GL_COMPILE); 
		glBindTexture(GL_TEXTURE_2D, m_pTextureManager->getResource(0));
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);//enable transparancy
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
		
		glLoadIdentity();
		glPushMatrix();
		glBegin(GL_QUADS);
			glTexCoord2i(0, 0);glVertex3f(m_pLabelPosition.first, m_pLabelPosition.second+l_f32PictureHeight, getDepth()+0.01f);
			glTexCoord2i(1, 0);glVertex3f(m_pLabelPosition.first+l_f32PictureWidth, m_pLabelPosition.second+l_f32PictureHeight, getDepth()+0.01f);
			glTexCoord2i(1, 1);glVertex3f(m_pLabelPosition.first+l_f32PictureWidth, m_pLabelPosition.second, getDepth()+0.01f);
			glTexCoord2i(0, 1);glVertex3f(m_pLabelPosition.first, m_pLabelPosition.second, getDepth()+0.01f);		
		glEnd();
		glPopMatrix();
		
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	glEndList();
}

void GPictureSymbol::computeLabelPosition()
{
	float32 l_fBoundingWidth = m_f32MaxLabelSize*m_f32LabelScaleSize;
	float32 l_fxOffset = (getWidth()-l_fBoundingWidth)/2.0f;	
	float32 l_fBoundingHeight = l_fBoundingWidth;
	float32 l_fyOffset = (getHeight()-l_fBoundingHeight)/2.0f;		
	m_pLabelPosition.first = getX()+l_fxOffset;
	m_pLabelPosition.second = getY()+l_fyOffset;
}

void GPictureSymbol::assignHelper(GPictureSymbol const& gpicturesymbol)
{
	this->m_bImageLoaded = gpicturesymbol.m_bImageLoaded;
	this->m_iMode = gpicturesymbol.m_iMode;
	this->m_pTextureManager = gpicturesymbol.m_pTextureManager->clone();
}

std::string GPictureSymbol::toString() const
{
	std::string text;
	text = GLabel::toString() + std::string("picture ");
	return text;
}
#endif

#endif
