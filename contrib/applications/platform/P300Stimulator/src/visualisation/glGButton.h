#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __GButton_H__
#define __GButton_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <cstring>

#include "glGLabel.h"

namespace OpenViBEApplications
{
	enum ButtonState
	{
		GButton_Inactive = 1,
		GButton_Clicked = 2,
		GButton_Active = 8,
		GButton_Focus = 16,
		GButton_WrongClick = 32,
	};	
	
	enum ButtonAction
	{
		GButton_None = 1,
		GButton_Write = 2,
		GButton_WordPrediction = 8,
		GButton_Undo = 16,
		GButton_Redo = 32,
	};	
	
	/**
	 * This class will represent the keys of the P300 keyboard which can be selected for feedback or for focus 
	 * (target letter in copy spelling task). It can also have an action
	 * associated with them in case they are selected, e.g. write, wordprediction, undo (specified by a number; enum ButtonAction ).\n
	 * The GButton should always have a GLabel set. The GLabel can either be a GPictureSymbol or a regular text GSymbol.
	 */
	class GButton : public GObject
	{
		
	static const OpenViBE::uint32 VertexCount = 4;	
	static const OpenViBE::uint32 VBOSize = 3*VertexCount; 	
		
	public:
		/**
		 * GButton constructor will call GObject(true, 1) which means it will use one OpenGL display list. Also generates 
		 * the display list. GButton_Inactive
		 */
		GButton();
		
		/**
		 * GButton copy constructor
		 */		
		GButton(const GButton& gbutton);
		
		/**
		 * GButton destructor will invoke the GLabel destructor. The OpenGL resources will be released by subsequent call 
		 * (in reverse order) to the GObject destructor
		 */		
		virtual ~GButton();
		
		/**
		 * The assignment operator calls the GObject assignment operator first and marks the object as changed so that it is
		 * drawn the next time. Through this call the OpenGL resources are also being taken care of.
		 * @param gbutton the GButton to be assigned to this
		 */		
		GButton& operator= (GButton const& gbutton);		

		/**
		 * Clone function used for making a copy of a GButton (invokes the copy constructor)
		 */
		virtual GButton* clone() const { return new GButton(*this); }		
		
		/**
		 * This method should contain the OpenGL code to draw on the back buffer.
		 * It will first check if the GButton has changed and if it has to be redrawn. If it has to be redrawn it will
		 * only clear the space on the screen it occupies and redraw that by setting the background color and calling
		 * the display list (or vbo buffer if it is used). After that it will call the draw function of GLabel and set
		 * the status of changed to false.
		 */		
		virtual void draw();
		
		/**
		 * String representation of GButton. Also calls the toString method of the GLabel.
		 * @return the string representation of GButton
		 */		
		virtual std::string toString() const;
		
		/**
		 * Sets the dimension parameters of the GButton. Therefore it first call the corresponding method of GObject which
		 * recreates the OpenGL resources (as the dimensions of GButton have been changed). Then it regenerates the contents of the
		 * display list or the vbo buffers. Finally it will increase the depth dimension with 0.01 for the GLabel and set
		 * the label's dimensions.
		 */
		virtual void setDimParameters(BoxDimensions dim);
		
		/**
		 * Sets a new GLabel. If it's different from the previous one, the old GLabel will be deleted. The new one will point to 
		 * the parameter glabel and the correct (according to GButton) dimensions will be set (glabel.depth = gbutton.depth + 0.01)
		 * @param glabel the GLabel pointer that will be set as the new GLabel
		 */
		virtual void setLabel(GLabel* glabel);
		
		/**
		 * @return GButton's label
		 */		
		virtual GLabel* getLabel() const { return m_pLabel; }
		
		/**
		 * Sets the button's state, e.g. GButton_Clicked when it is selected for feedback
		 * The handlers, such as P300KeyboardHandler, P300ResultAreaHandler, etc. need to know the state of the button.
		 * @param state GButton_Inactive, GButton_Active, GButton_Clicked, GButton_Focus or GButton_WrongClick
		 */		
		virtual void setState(ButtonState state);
		
		/**
		 * @return GButton's state, GButton_Inactive, GButton_Active, GButton_Clicked, GButton_Focus or GButton_WrongClick
		 */		
		virtual ButtonState getState() const { return m_iButtonState; }
		
		/**
		 * Sets the action associated with GButton when it is clicked (in state GButton_Clicked)
		 * @param action 'write' or 'wordprediction'
		 */		
		virtual void setAction(OpenViBE::CString action);
		
		/**
		 * Returns GButton's action when clicked
		 * @return GButton_None, GButton_Write, GButton_WordPrediction or GButton_Undo.
		 */		
		virtual ButtonAction getAction() const { return m_iButtonAction; }	
		
		/**
		 * Sets the status of the GButton to changed or not changed, and that of the GLabel as well.
		 */		
		virtual void setChanged(OpenViBE::boolean l_bChanged); 

      private:
            void generateVBOBuffers();
		void generateGLDisplayLists();
            void fillVertexBuffer(GLfloat* buffer, OpenViBE::float32 depth);
		void fillColorBuffer(GLfloat* buffer, const GColor& l_color);
		void assignHelper(const GButton& gbutton);

	protected:
		GLabel* m_pLabel;
		ButtonState m_iButtonState;
		ButtonAction m_iButtonAction;
	};
};

#endif
#endif

#endif
