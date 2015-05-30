#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "glGObject.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

GObject::GObject(boolean usingDisplayLists, uint32 n) : m_bUsingDisplayLists(usingDisplayLists)
{
	m_oDimensions.x = 0.0;
	m_oDimensions.y = 0.0;
	m_oDimensions.width = 0.0;
	m_oDimensions.height = 0.0;
	m_oDimensions.depth = 0.0;
	
	m_cBackgroundColor.red = 0.0;
	m_cBackgroundColor.green = 0.0;
	m_cBackgroundColor.blue = 0.0;

	m_bChanged = true;
	
	if (usingDisplayLists)
		m_pGLResourceManager = new OpenGLDListManager(n);
	else
		m_pGLResourceManager = new OpenGLVBOManager(n);
}	

GObject::GObject(boolean b, uint32 n, float32 x, float32 y, float32 w, float32 h): m_bUsingDisplayLists(b)
{
	m_oDimensions.x = x;
	m_oDimensions.y = y;
	m_oDimensions.width = w;
	m_oDimensions.height = h;
	m_oDimensions.depth = 0.0;
	
	m_cBackgroundColor.red = 0.0;
	m_cBackgroundColor.green = 0.0;
	m_cBackgroundColor.blue = 0.0;
	
	m_bChanged = true;
	
	if (m_bUsingDisplayLists)
		m_pGLResourceManager = new OpenGLDListManager(n);
	else
		m_pGLResourceManager = new OpenGLVBOManager(n);	
}

GObject::GObject(const GObject& gobject) : GObservable(gobject)
{
	assignHelper(gobject);
}		

GObject::~GObject() 
{
	delete m_pGLResourceManager;
	m_pGLResourceManager = NULL;
}

GObject& GObject::operator= (GObject const& gobject)
{
	if(this!=&gobject)
	{
		this->GObservable::operator=(gobject);
		delete m_pGLResourceManager;
		assignHelper(gobject);
	}
	return *this;
}	

void GObject::setDimParameters(BoxDimensions dim)
{
	if (m_oDimensions!=dim)
	{
		//the code needs to be rewritten to graphical memory with the new dimensions so we need to recreate the OpenGL resources
		deleteAndCreateGLResources();	
		
		m_oDimensions.x = dim.x;
		m_oDimensions.y = dim.y;
		m_oDimensions.width = dim.width;
		m_oDimensions.height = dim.height;
		m_oDimensions.depth = dim.depth;
		this->setChanged(true);
	}
}

void GObject::setBackgroundColor(GColor color) 
{ 
	if (m_cBackgroundColor.red!=color.red || m_cBackgroundColor.green!=color.green || m_cBackgroundColor.blue!=color.blue)
	{
		m_cBackgroundColor=color; 
		this->setChanged(true);
	}
}

GLuint GObject::getGLResourceID(OpenViBE::uint32 gl_index) const
{
	return m_pGLResourceManager->getResource(gl_index);
}

void GObject::deleteAndCreateGLResources()
{
	uint32 l_ui32NumberOfResources = m_pGLResourceManager->getNumberOfResources();
	delete m_pGLResourceManager;
	if (m_bUsingDisplayLists)
		m_pGLResourceManager = new OpenGLDListManager(l_ui32NumberOfResources);
	else
		m_pGLResourceManager = new OpenGLVBOManager(l_ui32NumberOfResources);		
}

void GObject::assignHelper(GObject const& gobject)
{
	m_oDimensions.x = gobject.getX();
	m_oDimensions.y = gobject.getY();
	m_oDimensions.width = gobject.getWidth();
	m_oDimensions.height = gobject.getHeight();
	m_oDimensions.depth = gobject.getDepth();
	this->setBackgroundColor(gobject.getBackgroundColor());
	this->setChanged(true);
	this->m_bUsingDisplayLists = gobject.m_bUsingDisplayLists;
	m_pGLResourceManager = gobject.m_pGLResourceManager->clone();
}
#endif

#endif
