#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __GContainer_OV_H__
#define __GContainer_OV_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "glGObject.h"

#include <vector>

namespace OpenViBEApplications
{
	/**
	 * This class will represent an area in the P300 speller that can be filled with graphical objects such as
	 * GButton, GSymbol... When draw function is called it will call the draw function on itself and all its children
	 */
	class GContainer : public GObject
	{
		
	static const OpenViBE::uint32 VertexCount = 4;	
	static const OpenViBE::uint32 VBOSize = 3*VertexCount; 
	
	public:
		GContainer();
		/*GContainer(OpenViBE::float32 w, OpenViBE::float32 h) : GObject(w,h) 
		{
			m_lChildren = new std::vector<GObject*>();
		}	*/		
		GContainer(OpenViBE::float32 x, OpenViBE::float32 y, OpenViBE::float32 w, OpenViBE::float32 h);
		GContainer(const GContainer& gcontainer);
		virtual ~GContainer();
		virtual GContainer& operator= (GContainer const& mainContainer);	
		
		//inherited functions	
		virtual GContainer* clone() const
		{
			return new GContainer(*this);
		}
		/**
		 * When draw function is called it will call the draw function on itself and all its children
		 */
		virtual void draw();
		virtual std::string toString() const;

		//inherited setters
		virtual void setDimParameters(BoxDimensions dim);	
		virtual void setBackgroundColor(GColor color) { GObject::setBackgroundColor(color); }	
		
		//inherited getters
		virtual OpenViBE::float32 getX() const { return GObject::getX(); }
		virtual OpenViBE::float32 getY() const { return GObject::getY(); }
		virtual OpenViBE::float32 getWidth() const { return GObject::getWidth(); }
		virtual OpenViBE::float32 getHeight() const { return GObject::getHeight(); }
		virtual OpenViBE::float32 getDepth() const { return GObject::getDepth(); }
		virtual GColor getBackgroundColor() const { return GObject::getBackgroundColor(); }	
		
		//additional functions
		/**
		 * The position of the graphical object child is computed based on the parameters you provide here
		 * (those are relative to the container, left lower corner represented by 0,0 and upper right
		 * corner represented by 1,1)
		 * @param child the graphical object you want to add to the container
		 * @param offsetX number between zero and one indicating where it should be placed within the container
		 * 0.5 would mean will be positioned halfway (horizontally) in the container
		 * @param offsetY number between zero and one indicating where it should be placed within the container
		 * @param width number between zero and one indicating the width of the element relative to the container
		 * 1.0 would mean the child takes up the entire width of the container
		 * @param height number between zero and one, same interpretation as the height
		 * @param depth number between -1.0 and 1.0, -1.0 meaning towards the back, 1.0 towards the front
		 */
		virtual void addChild(GObject* child, OpenViBE::float32 offsetX, OpenViBE::float32 offsetY, 
					OpenViBE::float32 width, OpenViBE::float32 height, OpenViBE::float32 depth);		
		virtual void removeChild(OpenViBE::uint32 childIndex);
		virtual void removeAllChildren();
		
		//additional getters
		/**
		 * gets the graphical object of the container at index childIndex
		 * @return a reference to a pointer so you can actually replace the graphical object with another one by letting
		 * it point to that new object
		 */
		virtual GObject*& getChild(OpenViBE::uint32 childIndex) const { return m_lChildren->at(childIndex); }
		virtual OpenViBE::uint32 getNumberOfChildren() const { return m_lChildren->size(); }
		
		/*virtual void setChild(GObject * gobject, OpenViBE::uint32 childIndex)
		{
			
		}*/
	protected:
		virtual void generateVBOBuffers();
		virtual void generateGLDisplayLists();
        virtual void fillVertexBuffer(GLfloat* buffer, OpenViBE::float32 depth);
		virtual void fillColorBuffer(GLfloat* buffer, const GColor& l_color);
		
	protected:	
		std::vector<GObject*> * m_lChildren;
		//OpenViBE::uint32 m_ui32NumberOfChildren;
		
	};
	
};

#endif
#endif

#endif
