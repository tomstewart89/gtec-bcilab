#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "glGButton.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

GButton::GButton() : GObject(true, 1) 
{
    m_pLabel =  NULL;
    m_iButtonState = GButton_Inactive;
	m_iButtonAction = GButton_None;
	
	/*If you want to use VBO buffers uncomment this and call GObject(false,2). 
	 Two buffers are need one for the position of the vertices and one for the color.*/
	//generateVBOBuffers(); 
	
	/*If you want to use display lists uncomment this and call GObject(true,1). 
	 Only one display list is needed*/	
	generateGLDisplayLists();
}	

GButton::GButton(const GButton& gbutton) : GObject(gbutton)
{
    this->m_pLabel = gbutton.m_pLabel->clone();
	assignHelper(gbutton);
}

GButton::~GButton()
{
      delete m_pLabel;
}

GButton& GButton::operator= (GButton const& gbutton)
{
	if(this!=&gbutton)
	{
		this->GObject::operator=(gbutton);
		delete this->m_pLabel;
		this->m_pLabel = gbutton.m_pLabel->clone(); // by cloning isChanged of the label should be set to true (in copy constructor of GObject)
		assignHelper(gbutton);
	}
	return *this;
}

void GButton::setDimParameters(BoxDimensions dim)
{
	GObject::setDimParameters(dim);
	
	//If you want to use VBO buffers uncomment this
    //generateVBOBuffers();
	
	//If you want to use display lists uncomment this
	generateGLDisplayLists();

	dim.depth += 0.01f;
	m_pLabel->setDimParameters(dim);
}

void GButton::draw()
{
	if (isChanged())
	{
		/*glEnable(GL_SCISSOR_TEST);
		glScissor(static_cast<GLint>(getX()), static_cast<GLint>(getY()), static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);*/
		
		glColor3f(getBackgroundColor().red,getBackgroundColor().green,getBackgroundColor().blue);
		glCallList(getGLResourceID(0));
		
		m_pLabel->draw();
		//this->setChanged(false);
	}
	
	//This code is for VBO buffers (TODO should be placed inside if-statement to check for isChanged())
	/*glBindBuffer(GL_ARRAY_BUFFER, getGLResourceID(0));
	glVertexPointer(3,GL_FLOAT,0,0); 
	glBindBuffer(GL_ARRAY_BUFFER, getGLResourceID(1));
	glColorPointer(3, GL_FLOAT, 0, 0);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glDrawArrays(GL_LINE_LOOP, 0, GButton::VertexCount);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER,0);*/
}

void GButton::setLabel(GLabel* glabel)
{
	if (glabel!=m_pLabel)
	{
		if (m_pLabel!=NULL)
			delete m_pLabel;
		m_pLabel = glabel;
		BoxDimensions l_Dimensions;
		l_Dimensions.x = this->getX(); l_Dimensions.y = this->getY();
		l_Dimensions.width = this->getWidth(); l_Dimensions.height = this->getHeight();
		l_Dimensions.depth = this->getDepth()+0.01f;
		m_pLabel->setDimParameters(l_Dimensions);
	}
}

void GButton::setState(ButtonState state)
{
	m_iButtonState = state;
}

void GButton::setAction(OpenViBE::CString action)
{
	if (action==CString("wordprediction"))
		m_iButtonAction = GButton_WordPrediction;
	else if (action==CString("write"))
		m_iButtonAction = GButton_Write;
	else if (action==CString("undo"))
		m_iButtonAction = GButton_Undo;
	else if (action==CString("redo"))
		m_iButtonAction = GButton_Redo;
	else
		m_iButtonAction = GButton_None;	
}

void GButton::setChanged(OpenViBE::boolean l_bChanged)
{
	GObject::setChanged(l_bChanged);
	m_pLabel->setChanged(l_bChanged);
}

void GButton::generateVBOBuffers()
{
	/*GLfloat m_fBuffer[GButton::VBOSize];	
	fillVertexBuffer(m_fBuffer, 0.5);
	glBindBuffer(GL_ARRAY_BUFFER, getGLResourceID(0));
	glBufferData(GL_ARRAY_BUFFER, GButton::VBOSize*sizeof(GLfloat), m_fBuffer, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	GColor l_oFrameColor;
	l_oFrameColor.red = 0.3; l_oFrameColor.green = 0.3; l_oFrameColor.blue = 0.3;
	fillColorBuffer(m_fBuffer, l_oFrameColor);
	glBindBuffer(GL_ARRAY_BUFFER, getGLResourceID(1));
	glBufferData(GL_ARRAY_BUFFER, GButton::VBOSize*sizeof(GLfloat), m_fBuffer, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);*/	
}

void GButton::generateGLDisplayLists()
{
      glNewList(getGLResourceID(0),GL_COMPILE); 
		glLoadIdentity();
		//glColor3f(getBackgroundColor().red,getBackgroundColor().green,getBackgroundColor().blue); 
		glBegin(GL_QUADS);
			glVertex3f(getX(), getY(), getDepth());
			glVertex3f(getX()+getWidth(), getY(), getDepth());
			glVertex3f(getX()+getWidth(), getY()+getHeight(), getDepth());
			glVertex3f(getX(), getY()+getHeight(), getDepth());
		glEnd();
		GColor l_oFrameColor;
		l_oFrameColor.red = 0.3f; l_oFrameColor.green = 0.3f; l_oFrameColor.blue = 0.3f;
		glLoadIdentity();
		glColor3f(l_oFrameColor.red,l_oFrameColor.green,l_oFrameColor.blue); 
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(2);
		glBegin(GL_QUADS);
			glVertex3f(getX(), getY(), getDepth()+0.5f);
			glVertex3f(getX()+getWidth(), getY(), getDepth()+0.5f);
			glVertex3f(getX()+getWidth(), getY()+getHeight(), getDepth()+0.5f);
			glVertex3f(getX(), getY()+getHeight(), getDepth()+0.5f);
		glEnd();		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEndList();
}

void GButton::fillVertexBuffer(GLfloat* buffer, float32 depth)
{
	buffer[0] = getX(); 			buffer[6] = getX()+getWidth();
	buffer[1] = getY(); 			buffer[7] = getY()+getHeight();
	buffer[2] = getDepth()+depth; 	buffer[8] = getDepth()+depth;
	buffer[3] = getX()+getWidth(); 	buffer[9] = getX();
	buffer[4] = getY(); 			buffer[10] = getY()+getHeight();
	buffer[5] = getDepth()+depth; 	buffer[11] = getDepth()+depth;	
}

void GButton::fillColorBuffer(GLfloat* buffer, const GColor& l_color)
{
	buffer[0] = l_color.red; 			buffer[6] = l_color.red;
	buffer[1] = l_color.green; 			buffer[7] = l_color.green;
	buffer[2] = l_color.blue; 			buffer[8] = l_color.blue;
	buffer[3] = l_color.red; 			buffer[9] = l_color.red;
	buffer[4] = l_color.green; 			buffer[10] = l_color.green;
	buffer[5] = l_color.blue; 			buffer[11] = l_color.blue;		
}

void GButton::assignHelper(const GButton& gbutton)
{
	this->m_iButtonState = gbutton.m_iButtonState;
	this->m_iButtonAction = gbutton.m_iButtonAction;
}

std::string GButton::toString() const
{
	std::string text("button with ");
	text += std::string(m_pLabel->toString());
	return text;	
}
#endif

#endif
