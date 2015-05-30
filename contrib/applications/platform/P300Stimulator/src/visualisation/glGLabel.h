#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __GLabel_H__
#define __GLabel_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <cstring>

#include "glGObject.h"

namespace OpenViBEApplications
{
	/**
	 * Base class of GPictureSymbol and GSymbol that handles the common properties such as
	 * foreground color, filename of the source file (font file for GSymbol, png file for GPictureSymbol)
	 * label size and label position
	 * TODO maybe some of these constructors should be protected as we should always construct
	 * an object of the derived classes
	 */
	class GLabel : public GObject
	{
	public:
		GLabel(); //can serve as a dummy label
		/**
		 * By calling the super class constructor GObject it will ask for two OpenGL resources, one for itself
		 * and one for the derived class as we SHOULD never construct a GLabel directly
		 * It generates the code for its own display list
		 * @param sourceFile font file for GSymbol, png file for GPictureSymbol
		 * @param scaleSize a number between 0 and 1 to define its size (1 almost filling up the entire label box)
		 */
		GLabel(OpenViBE::CString sourceFile, OpenViBE::float32 scaleSize);//should be made protected
		GLabel(const GLabel& glabel);
		virtual ~GLabel() {}		
		
		virtual GLabel& operator= (GLabel const& glabel);
		
		virtual GLabel* clone() const
		{
			return new GLabel(*this);
		}			
		
		/**
		 * This just draws a rectangle in the background color 
		 * (in case the object is changed by calling for example the assign operator somewhere, otherwise it is not drawn)
		 */
		virtual void draw();
		virtual std::string toString() const;
		
		//inherited setters
		/**
		 * This will call setDimParameters from GObject which will recreate the OpenGL resources
		 * so that we can generate new code in graphical memory with the new dimensions
		 */
		virtual void setDimParameters(BoxDimensions dim);
		
		//additional setters and getters
		virtual void setForegroundColor(GColor color) { m_cForegroundColor = color; }
		/**
		 * 
		 */
		virtual void setLabelScaleSize(OpenViBE::float32 labelScaleSize);
		//virtual void setSourceFile(OpenViBE::CString sourceFile) { m_sSourceFile = sourceFile; }
		
		virtual GColor getForegroundColor() const { return m_cForegroundColor; }
		virtual OpenViBE::float32 getLabelScaleSize() const { return m_f32LabelScaleSize; }
		virtual OpenViBE::float32 getMaxLabelSize() const { return m_f32MaxLabelSize; }
		virtual OpenViBE::CString getSourceFile() const { return m_sSourceFile; }
		
	protected:		
		virtual void computeLabelPosition();	
		virtual void computeMaximumLabelSize();
		/**
		 * This just constructs a rectangle in the background color
		 */
		virtual void generateGLDisplayLists();
		
	private:
		void assignHelper(const GLabel& glabel);
		
	protected:
		GColor m_cForegroundColor;
		OpenViBE::float32 m_f32LabelScaleSize;
		OpenViBE::float32 m_f32MaxLabelSize;		
		std::pair<OpenViBE::float32, OpenViBE::float32> m_pLabelPosition;
		OpenViBE::CString m_sSourceFile;
	};
};

#endif
#endif

#endif
