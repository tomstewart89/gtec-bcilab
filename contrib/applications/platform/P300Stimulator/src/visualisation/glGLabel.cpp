#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "glGLabel.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

GLabel::GLabel() : GObject(true,1), m_f32LabelScaleSize(0.0), m_sSourceFile("")  {}

GLabel::GLabel(OpenViBE::CString sourceFile, OpenViBE::float32 scaleSize) : GObject(true,2) 
{
	m_cForegroundColor.red = 1.0;
	m_cForegroundColor.green = 1.0;
	m_cForegroundColor.blue = 1.0;
	
	m_sSourceFile = sourceFile;
	m_f32LabelScaleSize = scaleSize;
	m_f32MaxLabelSize = 10.0;	
	
	generateGLDisplayLists();
}

GLabel::GLabel(const GLabel& glabel) : GObject(glabel)
{
	assignHelper(glabel);
}

GLabel& GLabel::operator= (GLabel const& glabel)
{
	if(this!=&glabel)
	{
		this->GObject::operator=(glabel); //this will set isChanged to true
		assignHelper(glabel);
	}
	return *this;
}

void GLabel::draw()
{
	//rendering background rectangle
	if (isChanged())
	{
		//the below code was used to only redraw certain parts of the screen and not the entire screen 
		//(in conjunction with code in other files such as GContainer). The speedup didn't seem significant
		//and the code gave some weird issues that were sometimes hard to solve so I dropped it.
		/*glEnable(GL_SCISSOR_TEST);
		glScissor(static_cast<GLint>(getX())+2, static_cast<GLint>(getY())+2, static_cast<GLsizei>(getWidth())-4, static_cast<GLsizei>(getHeight())-4);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);*/
		
		glColor3f(getBackgroundColor().red,getBackgroundColor().green,getBackgroundColor().blue); 
		glCallList(getGLResourceID(0));
		
		//this->setChanged(false);
	}
}

void GLabel::setDimParameters(BoxDimensions dim)
{
	GObject::setDimParameters(dim);
	computeMaximumLabelSize();
	this->generateGLDisplayLists();
}

void GLabel::setLabelScaleSize(float32 labelScaleSize) 
{ 
	m_f32LabelScaleSize = labelScaleSize; 
	deleteAndCreateGLResources();
	generateGLDisplayLists();
}

void GLabel::computeLabelPosition()
{
	m_pLabelPosition.first = this->getX();
	m_pLabelPosition.second = this->getY();
}

void GLabel::computeMaximumLabelSize()
{
	m_f32MaxLabelSize = float32(0.9*std::min(getHeight(),getWidth()));
}

void GLabel::generateGLDisplayLists()
{
	//just a rectangle
      glNewList(getGLResourceID(0),GL_COMPILE); 
		glLoadIdentity();
		glBegin(GL_QUADS);
			glVertex3f(getX(), getY(), getDepth());
			glVertex3f(getX()+getWidth(), getY(), getDepth());
			glVertex3f(getX()+getWidth(), getY()+getHeight(), getDepth());
			glVertex3f(getX(), getY()+getHeight(), getDepth());
		glEnd();
      glEndList();
}

void GLabel::assignHelper(const GLabel& glabel)
{
	this->m_cForegroundColor = glabel.m_cForegroundColor;
	this->m_sSourceFile = glabel.m_sSourceFile;
	this->m_pLabelPosition = glabel.m_pLabelPosition;
	this->m_f32LabelScaleSize = glabel.m_f32LabelScaleSize;
	this->m_f32MaxLabelSize = glabel.m_f32MaxLabelSize;
}

std::string GLabel::toString() const
{
	return std::string("label ");
}
#endif

#endif
