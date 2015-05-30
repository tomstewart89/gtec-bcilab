#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __GP300MainContainer_OV_H__
#define __GP300MainContainer_OV_H__

#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include <list>
#include <vector>
#include <queue>

#include "../properties/ovexP300InterfacePropertyReader.h"
#include "../properties/ovexP300ScreenLayoutReader.h"
#include "../tagging/ovexITagger.h"
#include "../handlers/ovexP300KeyboardHandler.h"
#include "../handlers/ovexP300ResultAreaHandler.h"
#include "../handlers/ovexUndoHandler.h"
#include "../handlers/ovexBackspaceHandler.h"
#include "../handlers/ovexP300PredictionboardHandler.h"
#include "../handlers/ovexP300TargetAreaHandler.h"

#include "glGTable.h"

#include <GLFW/glfw3.h>

#define WORDPREDICTION

namespace OpenViBEApplications
{
	
	/**
	 * This class initializes the OpenGL context, creates the different GContainer or GTable for the keyboard,
	 * the results and the predicted words. It also creates the handlers that will observe those areas
	 * in case something changes. For example the keyboard handler P300KeyboardHandler will observer the word prediction area 
	 * and the keyboard area. It will also populate it with the necessary symbols.
	 * This class also has the drawAndSync function so that everything is drawn to screen and the corresponding events
	 * written to the ITagger
	 */
	class P300MainContainer : public GContainer
	{

		
	public:
		/**
		 * @param propertyObject this is the property reader that reads from interface-properties.xml
		 * @param layoutPropObject the reader that contains all information/properties of all keys as specified in for example
		 * the 5by10grid-abc-gray.xml file
		 (added the two following param because it originally took them from the file but in fullscreen mode they are automatically computed by glfw)
		 * @param width
		 * @param height
		 */
		P300MainContainer(P300InterfacePropertyReader* propertyObject, P300ScreenLayoutReader* layoutPropObject, int width, int height);
		P300MainContainer(const P300MainContainer& gcontainer);
		virtual ~P300MainContainer();		
		
		/*P300MainContainer& operator= (P300MainContainer const& mainContainer)
		{
			if(this!=&mainContainer)
			{
				GTable* l_gGridArea = mainContainer.m_gGridArea->clone();
				GTable* l_gResultArea = mainContainer.m_gResultArea->clone();
				GTable* l_gPredictionArea = mainContainer.m_gPredictionArea->clone();

				delete m_gGridArea;
				delete m_gResultArea;
				delete m_gPredictionArea;
				
				this->m_gResultArea = l_gResultArea; this->getChild(0) = this->m_gResultArea;
				this->m_gGridArea = l_gGridArea; this->getChild(1) = this->m_gGridArea;
				this->m_gPredictionArea = l_gPredictionArea; this->getChild(2) = this->m_gPredictionArea;
				//this->m_bSymbolPositionsChanged = mainContainer.m_bSymbolPositionsChanged;
				//this->m_pPropertyObject = mainContainer.m_pPropertyObject;
				//this->m_oSDLSurface = mainContainer.m_oSDLSurface;
			}
			return *this;
		}*/
		
		P300MainContainer* clone() const
		{
			return new P300MainContainer(*this);
		}		
		
		/**
		 * There is an area dedicated for use with a photo diode (TODO: specify its location in configuration file instead of hardcoded)
		 * With this function you can change its background color
		 */
		void changeBackgroundColorDiodeArea(GColor bColor);
		void DiodeAreaFlash(OpenViBE::boolean bFlash);
		
		/**
		 * @return the keyboard handler, this object gives you the ability to make changes to the screen (symbols flashed, symbols not flashed...)
		 * this object is used in the ExternalP300Visualiser for making updates
		 */
		P300KeyboardHandler* getKeyboardHandler();	
		
		//inherited functions		
		std::string toString() const;
		
		//encapsulation for draw functions
		/**
		 * @param tagger either a software or hardware tagger which will send the stimuli in l_qEventQueue to either
		 * the acquisition server (software) or parallel port (hardware)
		 * @param l_qEventQueue a queue of stimuli that need to be send for synchronisation to the tagger
		 */ 
		void drawAndSync(ITagger * tagger, std::queue<OpenViBE::uint32>& l_qEventQueue);
		
		/**
		 * Before calling this class constructor you need to call this function to initialize the OpenGL context
		 * otherwise the constructor will already create graphical objects while the OpenGL context does not exist yet
		 * @param fullScreen enable full screen or not
		 * @param width width of window (also needs to be specified for full screen, TODO in full screen mode the width and 
		 * height should be detected automatically)
		 * @param height height of the window
		 * @param monitorIndex index of the monitor the app appears on (only relevant in fullscreen mode, count start at 0)
		 */
		static void initializeGL(OpenViBE::boolean fullScreen, OpenViBE::float32 width, OpenViBE::float32 height, OpenViBE::uint32 monitorIndex);

		//getter to the window (should it be made const?)
		static GLFWwindow*  getWindow(void) {return m_oGLFWWindow;};
	private:
		/**
		 * This function is used by drawAndSync
		 */
		void draw();
		
		/**
		 * Helper function to initialize all the areas and the handlers
		 */
		void initialize(OpenViBE::uint32 nGridCells);
		/**
		 * Helper function to initialize the main keyboard area
		 */		
		void initializeGridArea(OpenViBE::uint32 nCells);
		/**
		 * Helper function to initialize the result area
		 */		
		void initializeResultArea();
		/**
		 * Helper function to initialize the word prediction area
		 */		
		void initializePredictionArea();
		/**
		 * Helper function to initialize the target area (for the blue letters in copy mode)
		 */		
		void initializeTargetArea();
		/**
		 * Helper function to initialize the diode area
		 */		
		void initializeDiodeArea();
	
		static void do_ortho(OpenViBE::float32 width, OpenViBE::float32 height);
		
	private:
		//static SDL_Surface* m_oSDLSurface;
		static GLFWwindow* m_oGLFWWindow;
		
		#ifdef WORDPREDICTION
		P300PredictionboardHandler* m_pP300PredictionHandler;
		#endif
		P300UndoHandler* m_pP300UndoHandler;
		P300ResultAreaHandler* m_pP300ResultAreaHandler;
		P300TargetAreaHandler* m_pP300TargetAreaHandler;
		P300KeyboardHandler* m_pP300KeyboardHandler;
		P300BackspaceHandler* m_pP300BackspaceHandler;
		
		//WordPredictionInterface* m_pWordPredictionEngine;
		
		GContainer* m_gGridArea; //cleaned up by GContainer where they are added as childs
		GTable* m_gTargetArea;
		GTable* m_gResultArea;
		GTable* m_gPredictionArea;
		GContainer* m_gDiodeArea;//diode area when it is dark
		GContainer* m_gLitDiodeArea;//diode area when it is light
		
		P300InterfacePropertyReader* m_pInterfacePropertyObject; //responsibility of the caller to clean this up
		P300ScreenLayoutReader* m_pScreenLayoutObject;

		#ifdef OUTPUT_TIMING
            FILE * timingFile;
		#endif
	};
	
};
#endif//TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#endif


#endif
