#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __P300ResultAreaHandler_H__
#define __P300ResultAreaHandler_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <cstring>

#include "../visualisation/glGTable.h"
#include "../visualisation/glGLabel.h"
#include "../properties/ovexP300ScreenLayoutReader.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <vector>

namespace OpenViBEApplications
{	
	/**
	 * This class will handle all changes that need to be made to the result bar/region, this means
	 * adding letters, words, undoing actions, removing letters, redoing actions
	 */
	class P300ResultAreaHandler : public GObserver, public GObservable
	{	

	public:
		
		/**
		 * constructor
		 */
		P300ResultAreaHandler(GTable* container, P300ScreenLayoutReader* propertyObject);
		
		virtual ~P300ResultAreaHandler();
		
		/**inherited from GObserver
		 * @param observable can be one of four objects, i.e. P300UndoHandler, P300BackspaceHandler, P300TargetAreaHandler and GButton.
		 * If it is a GButton then it is a notification to add a symbol or a predicted word
		 * If it is a P300UndoHandler then we have to undo the last letters added or undo a backspace. It's also used for redoing actions.
		 * If it is a P300BackspaceHandler we have to erase the last letters
		 * If it is a P300TargetAreaHandler then it is to notify the result bar to move its symbols one position to the left during copy spelling 
		 */
		virtual void update(GObservable* observable, const void * pUserData);
		
		/**
		 * @return the letters in the result region
		 */
		std::string& getResultBuffer() { return m_sSpelledLetters; }
		
	private:
		/**
		 * @param nshift number of positions to shift the labels in the result bar to the left
		 */
		void moveSymbolsLeft(OpenViBE::uint32 nshift);
		
		/**
		 * when a word is predicted you have to remove the letters that already have been spelled
		 * @param textLabel the predicted word from which we remove the letters that already have been spelled so that
		 * we only add the letters to complete the word     
		 */
		void trimPrefix(std::string& textLabel);
		
		/**
		 * Updates the string buffer in the background based on the new labels that were added to the result bar
		 * also updates the counter
		 */
		void updateResultBuffer();
		
		/**
		 * @return 
		 */
		std::string eraseLastCharacter();
		
	protected:
		
		/**
		 * The GTable that will contain copies of the labels that have been spelled
		 */
		GTable* m_pSymbolContainer;
		
	private:
		OpenViBE::uint32 m_ui32State;
		P300ScreenLayoutReader* m_pScreenLayoutObject;
		std::string m_sSpelledLetters;
		OpenViBE::uint32 m_ui32ResultCounter;
		#ifdef OUTPUT_TIMING
		FILE * timingFile;
		FILE* stackFile;
		#endif
		GLabel* m_oLastAddedLabel;
		OpenViBE::float64 m_f32LastFontSize;

		//keep states of buffer in memory for undo/redo
		std::vector< std::string > m_vStates;
	};
};
#endif
#endif

#endif
