#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __GObject_OV_H__
#define __GObject_OV_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "glGObservable.h"
#include "OpenGLManagers/OpenGLVBOManager.h"
#include "OpenGLManagers/OpenGLDListManager.h"

#include <GLFW/glfw3.h>

namespace OpenViBEApplications
{
	/**
	 * Struct representing a color in RGB space
	 */
	typedef struct 
	{
		GLfloat red; /**< A value between 0.0 and 1.0 representing the red color component*/
		GLfloat green; /**< A value between 0.0 and 1.0 representing the green color component*/
		GLfloat blue; /**< A value between 0.0 and 1.0 representing the blue color component*/
	} GColor;

	/**
	 * Struct that represents the lower left position of a graphical object, but also the width, height and depth.\n
	 * P300MainContainer redefines the OpenGL space so that 1 unit is 1 pixel and x=0,y=0 is the lower left corner of the screen.
	 */
	struct _BoxDimensions
	{
		OpenViBE::float32 width; /**< width of graphical object in OpenGL space */
		OpenViBE::float32 height; /**< height of graphical object in OpenGL space */
		OpenViBE::float32 x; /**< lower left x coordinate of graphical object in OpenGL space */
		OpenViBE::float32 y; /**< lower left y coordinate of graphical object in OpenGL space */
		OpenViBE::float32 depth; /**< depth somewhere between -1.0 and 1.0, 1.0 being the closest to the observer*/
		
		/**
		 * an assignment operator to easily initialize all the members with a fixed value, 0.0 for example
		 */
		_BoxDimensions& operator=(const OpenViBE::float32& a) 
		{ 
			this->x=a; this->y=a; this->width=a; this->depth=a; this->height=a;
			return *this; 
		}
		
		/**
		 * simple comparison operator to see if all members equal one fixed value, 0.0 for example
		 */		
		bool operator==(const OpenViBE::float32& b) 
		{
			if (this->width==b && this->height==b && this->x==b && this->y==b && this->depth==b)
				return true;
			else
				return false;
		}
		
		/**
		 * simple comparison operator to see if two BoxDimensions objects have different values or not
		 */			
		bool operator!=(const _BoxDimensions& b) 
		{
			if (this->width!=b.width || this->height!=b.height || this->x!=b.x || this->y!=b.y || this->depth!=b.depth)
				return true;
			else
				return false;
		}		
			
	};
	typedef _BoxDimensions BoxDimensions;
	
	/**
	 * This is the base class for all graphical objects which holds information about the dimensions and the background color.\n 
	 * As it implements the GObservable class, all graphical objects can be observed. This will be used in P300KeyboardHandler to notify all interested observers that a symbol has been flashed or selected.\n
	 * GObjects cannot be instantiated directly. The constructor can only be called by derived graphical objects.\n
	 * Each graphical object knows how to draw itself. To this end the draw() function should be implemented.\n
	 * Each graphical object can choose to either use OpenGL Display Lists or OpenGL Vertex Buffer Objects. This display lists or vbo's hold the OpenGL code and the data that is necessary for drawing the object\n
	 * Note: OpenGL Vertex Buffer Objects can not yet be used for drawing GSymbol objects as this requires a library that uses display lists for rendering fonts, the use of Display Lists is however deprecated. For now although VBO functionality is there, all graphical objects will use Display Lists for rendering.\n
	 * Each graphical object also has a resource manager OpenGLResourceManager. This object keeps track of the display list or vbo identifiers.\n
	 */
	class GObject : public GObservable
	{
		
	protected:
		/**
		 * GObject constructor 
		 * @param usingDisplayLists indicates whether to use OpenGL Display Lists or OpenGL Vertex Buffer Objects
		 * @param n indicates the number of display lists or vbo's that need to be created. A resource manager OpenGLResourceManager object is created that keeps track of n number of display lists or vbo's.
		 */
		GObject(OpenViBE::boolean usingDisplayLists, OpenViBE::uint32 n);	
		
		/**
		 * GObject constructor 
		 * @param b indicates whether to use OpenGL Display Lists or OpenGL Vertex Buffer Objects
		 * @param n indicates the number of display lists or vbo's need to be created
		 * @param x lower left x position coordinate
		 * @param y lower left y position coordinate
		 * @param w width
		 * @param h height
		 */	
		GObject(OpenViBE::boolean b, OpenViBE::uint32 n, OpenViBE::float32 x, OpenViBE::float32 y, OpenViBE::float32 w, OpenViBE::float32 h);
		
		/**
		 * Copy constructor, an internal variable is set to indicate that the state of the object has changed so that the next call to draw() actually redraws the object.
		 * @param gobject GObject to be copied
		 */
		GObject(const GObject& gobject);
		
		/**
		 * Assignment operator, an internal variable is set to indicate that the state of the object has changed so that the next call to draw() actually redraws the object.
		 * @param gobject the object that is assigned to this. The own OpenGL resource manager OpenGLResourceManager is deleted and the one of gobject is cloned so that the same OpenGL video resources are reused (more efficient).
		 */		
		GObject& operator= (GObject const& gobject);		
		
	public:
		/**
		 * String representation of GObject, e.g. for a button with a text label it should output that it is a button with the text of the label
		 * @return the string representation of GObject
		 */
		virtual std::string toString() const = 0;			
		
		/**
		 * Destructor deletes the OpenGL resource manager OpenGLResourceManager and decreases the use count of the display list or vbo resources
		 */
		virtual ~GObject();
		
		/**
		 * Virtual clone function used for making a copy of a GObject (invokes the copy constructor)
		 */
		virtual GObject* clone() const = 0;		
		
		/**
		 * This method should contain the OpenGL code to draw on the back buffer and it has to be implemented by every class that inherits from GObject. 
		 * In P300MainContainer the buffers will be swapped and the glFinish() function will be called in order to actually show the changes on the screen and sync the event with the EEG data.
		 */
		virtual void draw() = 0;
		
		/**
		 * @return the lower left x coordinate (in OpenGL units, P300MainContainer redefines the space so that 1 unit is 1 pixel)
		 */
		virtual OpenViBE::float32 getX() const { return m_oDimensions.x; }
		
		/**
		 * Getter
		 * @return the lower left y coordinate (in OpenGL units, P300MainContainer redefines the space so that 1 unit is 1 pixel)
		 */		
		virtual OpenViBE::float32 getY() const { return m_oDimensions.y; }
		
		/**
		 * @return the width (in OpenGL units, P300MainContainer redefines the space so that 1 unit is 1 pixel)
		 */		
		virtual OpenViBE::float32 getWidth() const { return m_oDimensions.width; }
		
		/**
		 * @return the height (in OpenGL units, P300MainContainer redefines the space so that 1 unit is 1 pixel)
		 */		
		virtual OpenViBE::float32 getHeight() const { return m_oDimensions.height; }
		
		/**
		 * @return the depth (between -1.0 and 1.0, 1.0 being closest)
		 */		
		virtual OpenViBE::float32 getDepth() const { return m_oDimensions.depth; }
		
		/**
		 * @return background color 
		 * @see GColor
		 */		
		virtual GColor getBackgroundColor() const { return m_cBackgroundColor; }
		
		/**
		 * @return the dimensions of the object 
		 * @see _BoxDimensions
		 */		
		virtual BoxDimensions getDimParameters() const { return m_oDimensions; }
		
		/**
		 * Checks whether something has been changed in the object that requires the object to be redrawn in the next iteration
		 * @return boolean to indicate that the object properties have changed
		 */		
		virtual OpenViBE::boolean isChanged() const { return m_bChanged; }
		
		/**
		 * Sets the x, y, height, width and depth parameters of the object. It also sets the internal state of the object to changed.
		 * It will also call deleteAndCreateGLResources because the code needs to be rewritten to graphical memory with the new dimensions
		 * and thus new resources need to be created and the old ones deleted
		 * @param dim the dimensions of the object (in OpenGL units, P300MainContainer redefines the space so that 1 unit is 1 pixel).
		 */
		virtual void setDimParameters(BoxDimensions dim);
		
		/**
		 * Sets the background color. It also sets the internal state of the object to changed.
		 * @param color the background color
		 */		
		virtual void setBackgroundColor(GColor color);
	
		/**
		 * When constructing a GObject, during assignment or when changing the object's dimensions, the state should be set to changed.
		 * @param l_bChanged indicates whether some properties have been changed or not.
		 */	
		virtual void setChanged(OpenViBE::boolean l_bChanged) { m_bChanged=l_bChanged; } 

      protected:
		/**
		 * @param gl_index the index of the OpenGL resource identifier you want. If GObject is created with n number of display lists or vbo's then you can access the first by gl_index=0, the second by gl_index=1 etc.
		 * @return the resource id that is associated with the preconstructed OpenGL display list or vbo in video memory.
		 */
            virtual GLuint getGLResourceID(OpenViBE::uint32 gl_index) const;
		
		/**
		 * This deletes the resource manager OpenGLResourceManager and decreases the use count of the resources and then requests OpenGL for new resources. This is necessary for example when the dimension parameters of GObject change and the display list or vbo has to be recreated/recomputed. So we have to delete the old resource, ask for a new resource identifier and create new OpenGL code associated with the new resource identifier. The latter operation is done in the specific derived class, not in GObject.
		 */
            virtual void deleteAndCreateGLResources();
	
	private:	
		void assignHelper(GObject const& gobject);
	
	private:
		GColor m_cBackgroundColor;
		BoxDimensions m_oDimensions;
            
		OpenViBE::boolean m_bUsingDisplayLists;
		OpenGLResourceManager* m_pGLResourceManager;
		OpenViBE::boolean m_bChanged;
	};
};

#endif//TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#endif

#endif
