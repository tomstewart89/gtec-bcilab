#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __GPictureSymbol_H__
#define __GPictureSymbol_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <string>
#include <boost/shared_ptr.hpp>

#include "glGLabel.h"
#include "OpenGLManagers/OpenGLTextureManager.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

namespace OpenViBEApplications
{
	/**
	 * A label class that does not have letters as content such as GSymbol, but pictures coming from png files
	 */
	class GPictureSymbol : public GLabel
	{
	public:	
		
		/**
		 * has to create a texture manager to make sure all buttons that use the same picture use the same resource 
		 * in graphical memory so that no multiple textures of the same pic are created\n
		 * On top of that it also uses display lists managed by OpenGLDListManager, both are created by the super class
		 * GLabel. One display list is filled in by GLabel, the other one by this class
		 * @param sourcePath the file name of the png file that will be used to make the texture
		 * @param scaleSize a number between 0 and 1 that defines the size of the texture in the button box
		 */			
		GPictureSymbol(OpenViBE::CString sourcePath, OpenViBE::float32 scaleSize);	
		GPictureSymbol(const GPictureSymbol& gsymbol);
		
		virtual ~GPictureSymbol();
		
		//virtual GPictureSymbol& operator= (GPictureSymbol const& gsymbol);
		
		//inherited functions
		virtual GPictureSymbol* clone() const { return new GPictureSymbol(*this);}			
		
		virtual void draw();
		virtual std::string toString() const;
		
		//inherited setters
		//virtual void setSourceFile(OpenViBE::CString sourceFile);
		virtual void setDimParameters(BoxDimensions dim);
		
	protected:
		/**
		 * Computes where the label should be positioned within the button/label box (saved in m_pLabelPosition variable of super class GLabel)
		 */
		virtual void computeLabelPosition();
		/**
		 * This will generate the display list code that will also bind the texture
		 */
		virtual void generateGLDisplayLists();
		
	private:
		/**
		 * this initializes the opengl texture
		 * TODO maybe this should be moved to the OpenGLTextureManager
		 */
		void initializeOpenGLTexture();
		void assignHelper(GPictureSymbol const& gpicturesymbol);
		
	protected:
		OpenViBE::boolean m_bImageLoaded;
		int m_iMode;	
		OpenGLTextureManager* m_pTextureManager;
	};
};

#endif
#endif

#endif
