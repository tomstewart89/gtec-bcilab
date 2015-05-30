#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __GSymbol_H__
#define __GSymbol_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <cstring>
#include <boost/shared_ptr.hpp>

#include "glGLabel.h"

#include <gdk/gdk.h>//pango stuff
#include <cairo.h>

namespace OpenViBEApplications
{
	/**
	 *  A label class that has letters as content instead of pictures such as GPictureSymbol
	 */
	class GSymbol : public GLabel
	{
	public:
		
		/**
		 * TODO: certain copies of objects should use the same font source to save memory and time for deleting and creation of
		 * font object. The management of this is done in P300KeyboardHandler::constructGLabelFromDescriptor, but it does not belong
		 * there, a manager such as the OpenGL managers should be made for fonts as well
		 * in P300KeyboardHandler::constructGLabelFromDescriptor we check whether a certain key already exists that uses
		 * the same font resource, in which case no new font object is created
		 * @param symbol the string (word or character) that should be displayed
		 * @param font a shared pointer to a font object of the FTGL library
		 * @param fontScaleSize a number between 0 and 1 to define its size (1 almost filling up the entire label box)
		 */
		//GSymbol(const char * symbol, boost::shared_ptr<FTFont> font, OpenViBE::float32 fontScaleSize);
		/**
		 * @param symbol the string (word or character) that should be displayed
		 * @param fontPath the path to the ttf file which defines the font and from which a FTFont object will be created
		 * @param fontScaleSize a number between 0 and 1 to define its size (1 almost filling up the entire label box)
		 */		
		GSymbol(const char * symbol, OpenViBE::CString fontPath, OpenViBE::float32 fontScaleSize);	
		GSymbol(const GSymbol& gsymbol);
		virtual ~GSymbol();
		
		//virtual GSymbol& operator= (GSymbol const& gsymbol);	
		
		//inherited functions
		virtual GSymbol* clone() const
		{
			return new GSymbol(*this);
		}			
		
		virtual void draw();
		virtual std::string toString() const;
		
		//inherited setters
		/**
		 * This will call the setDimParameters from the super class GLabel which will recompute the maximum label size 
		 * (by calling the computeMaximumLabelSize of this class GSymbol) and will call the generateGLDisplayLists
		 * of this class GSymbol (which also calls the generateGLDisplayLists of GLabel). This is because these functions
		 * are all virtual and first the derived class function is called before the base class function
		 */
		virtual void setDimParameters(BoxDimensions dim);
		//virtual void setSourceFile(OpenViBE::CString sourceFile);
		//virtual void setLabelScaleSize(OpenViBE::float32 labelScaleSize);
		
		//additional setters
		virtual void setTextLabel(const char * symbol);
		
		//additional getters
		virtual std::string getTextLabel() const { return m_sTextLabel; }	
		
	protected:
		//inherited functions
		/**
		 * Computes where the label should be positioned within the button/label box (saved in m_pLabelPosition variable of super class GLabel)
		 * some specific properties of the font need to be used to compute this
		 */
		virtual void computeLabelPosition();
		/**
		 * Overwrites the default maximum size as computed by the super class GLabel. This is because we need some specific
		 * properties from the bounding box of the symbol in a certain font to compute the point size of that font.
		 */
		virtual void computeMaximumLabelSize();
		virtual void generateGLDisplayLists();

		cairo_t* create_layout_context();
		void get_text_size (PangoLayout *layout, int *width, int *height, PangoRectangle* ink, PangoRectangle* logical);
		void render_text(const char *text, int *text_width, int *text_height, unsigned int *texture_id);

		unsigned int create_texture ( int width,int height,unsigned char *pixels);
		
	private:
		void assignHelper(GSymbol const& gsymbol);
		
		//void setFontSize();
		
	protected:
		std::string m_sTextLabel;
		std::string m_sFontFile;
		PangoLayout *m_olayout;
	};
};

#endif
#endif

#endif
