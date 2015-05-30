#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "glGContainer.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

GContainer::GContainer() : GObject(true,1) 
{
	m_lChildren = new std::vector<GObject*>();
	//generateVBOBuffers();
	generateGLDisplayLists();
}

GContainer::GContainer(OpenViBE::float32 x, OpenViBE::float32 y, OpenViBE::float32 w, OpenViBE::float32 h) : GObject(true,1,x,y,w,h) 
{
	m_lChildren = new std::vector<GObject*>();
	//generateVBOBuffers();
	generateGLDisplayLists();
}

GContainer::GContainer(const GContainer& gcontainer) : GObject(gcontainer)
{
	m_lChildren = new std::vector<GObject*>();
	for (OpenViBE::uint32 i=0; i<gcontainer.getNumberOfChildren(); i++)
	{
		GObject* child = gcontainer.getChild(i)->clone();
		m_lChildren->push_back(child);
	}
	generateGLDisplayLists();//lm I added that
}

GContainer::~GContainer()
{
	for (std::vector<GObject*>::iterator it=m_lChildren->begin();it!=m_lChildren->end(); it++)
		delete *it;
	delete m_lChildren;
}

GContainer& GContainer::operator= (GContainer const& mainContainer)
{
	if(this!=&mainContainer)
	{
		this->GObject::operator=(mainContainer);
		std::vector<GObject*> * l_lChildren = new std::vector<GObject*>();
		for (OpenViBE::uint32 i=0; i<mainContainer.getNumberOfChildren(); i++)
		{
			GObject* child = mainContainer.getChild(i)->clone();
			l_lChildren->push_back(child);
		}				
		
		for (std::vector<GObject*>::iterator it=m_lChildren->begin();it!=m_lChildren->end(); it++)
			delete *it;
		delete m_lChildren;
		
		this->m_lChildren = l_lChildren;
	}
	return *this;
}			

void GContainer::draw()
{
	if (isChanged())
	{
		//code for drawing only certain parts of the screen
		/*glEnable(GL_SCISSOR_TEST);
		glScissor(static_cast<GLint>(getX()), static_cast<GLint>(getY()), static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
		glDisable(GL_SCISSOR_TEST);*/
		glCallList(getGLResourceID(0));
	}
	
	//This is code for the VBO buffers
	/*glBindBuffer(GL_ARRAY_BUFFER, getGLResourceID(0));
	glVertexPointer(3,GL_FLOAT,0,0); 
	glBindBuffer(GL_ARRAY_BUFFER, getGLResourceID(1));
	glColorPointer(3, GL_FLOAT, 0, 0);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glDrawArrays(GL_QUADS, 0, GContainer::VertexCount); //should move this to triangles instead
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER,0);*/	
	
	std::vector<GObject*>::iterator it = m_lChildren->begin();
	for(; it!=m_lChildren->end(); it++)
	{
			//draw everything, if the object, on which draw is called, has not changed it should not execute its own draw function
			//if (isChanged())
			//	(*it)->setChanged(true);
			(*it)->draw();
	}
}

void GContainer::addChild(GObject* child, float32 offsetX, float32 offsetY, float32 width, float32 height, float32 depth)
{
	m_lChildren->push_back(child);
	BoxDimensions l_Dimensions;
	l_Dimensions.x = getX()+offsetX*getWidth();
	l_Dimensions.y = getY()+offsetY*getHeight();
	l_Dimensions.width = width*getWidth();
	l_Dimensions.height = height*getHeight();
	l_Dimensions.depth = depth;
	child->setDimParameters(l_Dimensions);
}

void GContainer::removeChild(OpenViBE::uint32 childIndex)
{
	if (childIndex<m_lChildren->size())
	{
		std::vector<GObject*>::iterator l_Iterator = m_lChildren->begin()+childIndex;
		delete *l_Iterator;
		m_lChildren->erase(l_Iterator);
	}
}
void GContainer::removeAllChildren()
{
	for (OpenViBE::uint32 i=0; i<m_lChildren->size(); i++)
		delete m_lChildren->at(i);
	m_lChildren->clear();
}

void GContainer::setDimParameters(BoxDimensions dim) 
{
	GObject::setDimParameters(dim);
	//generateVBOBuffers();
	generateGLDisplayLists();
}

void GContainer::generateVBOBuffers()
{
	/*GLfloat m_fBuffer[GContainer::VBOSize];	
	fillVertexBuffer(m_fBuffer, 0.0);
	//glGenBuffers(1,&VBOHandles[VBO_VERTEX]);
	glBindBuffer(GL_ARRAY_BUFFER, getGLResourceID(0));
	glBufferData(GL_ARRAY_BUFFER, GContainer::VBOSize*sizeof(GLfloat), m_fBuffer, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	fillColorBuffer(m_fBuffer, getBackgroundColor());
	//glGenBuffers(1,&VBOHandles[VBO_COLOR]);
	glBindBuffer(GL_ARRAY_BUFFER, getGLResourceID(1));
	glBufferData(GL_ARRAY_BUFFER, GContainer::VBOSize*sizeof(GLfloat), m_fBuffer, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);*/
}

void GContainer::generateGLDisplayLists()
{
      glNewList(getGLResourceID(0),GL_COMPILE); 
		glLoadIdentity();
		glColor3f(getBackgroundColor().red,getBackgroundColor().green,getBackgroundColor().blue); 
		//glColor3f(getBackgroundColor().red,getBackgroundColor().green,getBackgroundColor().blue); 
		glBegin(GL_QUADS);
			glVertex3f(getX(), getY(), getDepth());
			glVertex3f(getX()+getWidth(), getY(), getDepth());
			glVertex3f(getX()+getWidth(), getY()+getHeight(), getDepth());
			glVertex3f(getX(), getY()+getHeight(), getDepth());
		glEnd();
      glEndList();
}

void GContainer::fillVertexBuffer(GLfloat* buffer, float32 depth)
{
	buffer[0] = getX(); 			buffer[6] = getX()+getWidth();
	buffer[1] = getY(); 			buffer[7] = getY()+getHeight();
	buffer[2] = getDepth()+depth; 	buffer[8] = getDepth()+depth;
	buffer[3] = getX()+getWidth(); 	buffer[9] = getX();
	buffer[4] = getY(); 			buffer[10] = getY()+getHeight();
	buffer[5] = getDepth()+depth; 	buffer[11] = getDepth()+depth;	
}

void GContainer::fillColorBuffer(GLfloat* buffer, const GColor& l_color)
{
	buffer[0] = l_color.red; 			buffer[6] = l_color.red;
	buffer[1] = l_color.green; 			buffer[7] = l_color.green;
	buffer[2] = l_color.blue; 			buffer[8] = l_color.blue;
	buffer[3] = l_color.red; 			buffer[9] = l_color.red;
	buffer[4] = l_color.green; 			buffer[10] = l_color.green;
	buffer[5] = l_color.blue; 			buffer[11] = l_color.blue;		
}

std::string GContainer::toString() const
{
	return std::string("GContainer");
}
#endif

#endif
