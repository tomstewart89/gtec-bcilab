#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __P300KeyboardHandler_H__
#define __P300KeyboardHandler_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <map>
#include <cstring>
#include <list>

#include "../visualisation/glGTable.h"
#include "../visualisation/glGButton.h"
#include "../visualisation/glGLabel.h"

#include "../properties/ovexP300InterfacePropertyReader.h"
#include "../properties/ovexP300ScreenLayoutReader.h"

#include <boost/shared_ptr.hpp>

namespace OpenViBEApplications
{	
	/**
	 * The P300KeyboardHandler listens to its own keys, e.g. when a key is in a flashed state the keyboard handler is notified of this event 
	 * so that it can change the key with its flashed version. It is also observed by other handlers such as P300ResultAreaHandler in case a symbol 
	 * is selected for example.
	 * It is the CoAdaptP300Visualiser that will communicate with this class in order to update the states of the keys depending on
	 * the stimuli the CoAdaptP300Visualiser gets from the CoAdaptP300Stimulator.
	 * In the background it will create vectors of GButtons for all keys specified in the keyboard layout (letters, undo, backspace, word predictions...)
	 * and make tweaked copies for all the possible states the button can be in. This takes up quite a bit of main memory, but that way
	 * the OpenGL code embedded in the GObjects can persist in graphical memory (improving performance) without having to be recreated/recompiled every time
	 * we draw something on the screen
	 */
	class P300KeyboardHandler : public GObserver, public GObservable
	{	

	public:

		/**
		 * Constructor
		 * @param container this will be the GContainer that contains all the regular static keys 
		 * (i.e. not the word prediction keys that change from trial to trial)
		 * @param container2 this is the GContainer that will contain the word predictions keys
		 * @param propertyObject an object containing the properties read from the interface-properties.xml file
		 * @param layoutPropObject an object containing the descriptions of the key properties as specified in the keyboard layout files such as 5by10-abc-gray.xml
		 */
		P300KeyboardHandler(GContainer* container, GContainer* container2, P300InterfacePropertyReader* propertyObject, P300ScreenLayoutReader* layoutPropObject);
		
		virtual ~P300KeyboardHandler();
		
		/**
		 * inherited from GObserver. If the GObservable is a GButton then it will copy the new button in pUserData over the old one, if
		 * the GObservable is a P300PredictionboardHandler then it will update the words in the word-prediction-containter with the new most
		 * probable words.
		 * @param observable will either be a GButton or the P300PredictionboardHandler
		 * @param pUserData in case observable is a GButton then pUserData will also be a GButton containing the new button that needs to be displyed.
		 * in case observable is the P300PredictionboardHandler, then pUserData will be a vector of strings containing the new predicted words.
		 */
		virtual void update(GObservable* observable, const void * pUserData);
		
		/**
		 * inherited from GObservable. Will add the observer to all the keys, all the background copies for the different states
		 * and the word prediction keys.
		 */
		virtual void addObserver(GObserver * observer);
		
		/**
		 * Special version of addObserver, where an observer will be added only to the keys that have a certain action specified 
		 * (such as undo for example)
		 * @param action the specific action for which you want to add an observer
		 * @param observer the observer you want to add to the keys with the specified action
		 */		
		virtual void addActionObserver(OpenViBE::CString action, GObserver * observer);
		
		/**
		 * This function will notify all the observers of all the keys that have changed state (improving performance)
		 */
		virtual void updateChildProperties();
		
		/**
		 * This will update the state of all keys to a state as specified in the array states (values defined in VisualState)
		 * in case the specified state is different from NONE. 
		 * For the keys that already were in the FLASH state and now are specified as state NONE, we increase instead a counter and leave
		 * the state in FLASH. This enables overlapping stimuli (see resetMostActiveChildStates)
		 * @param states an array of states that specifies the state for each key (values are defined in the enum VisualState)
		 */
		virtual void updateChildStates(OpenViBE::uint32* states);
		
		/**
		 * reset the states of all the keys to NOFLASH
		 */
		virtual void resetChildStates();
		
		/**
		 * Updates the probabilities of all the symbols to enable intermediate color feedback from flash to flash
		 * @param symbolProbabilities the intermediate probabilities of each of the symols
		 */ 
		virtual void updateChildProbabilities(OpenViBE::float64* symbolProbabilities); //to manage 'continuous' color feedback
		
		/**
		 * This will reset the state of the keys that have been in the FLASH state the longest, see also updateChildStates. 
		 */
		virtual void resetMostActiveChildStates(); //to enable overlapping stimuli
		
	private:
		GColor getAssignedProbabilityColor(OpenViBE::uint32 symbolIndex);
		
		/**
		 * Creates a map that associates each probability - ranging from sigLevel to 1.0 in nSteps number of steps - with a color
		 * starting from startColor and ending with endColor
		 */
		void initializeColorMap(GColor startColor, GColor endColor, OpenViBE::uint32 nSteps, OpenViBE::float32 sigLevel);
		
		void initializeKeyboard();
		
		/**
		 * Some helper function that modifies the gbutton with the properties as specified in the
		 * key descriptor for the key at index 'keyIndex' in state 'keyState'
		 * @param gbutton button for which the properties will be changed 
		 */
		void setGButtonFromSLabel(GButton* gbutton, OpenViBE::uint32 keyIndex, VisualState keyState);
		
		/**
		 * Some helper function that creates a GLabel for a GButton based on information in
		 * key descriptor for the key at index 'keyIndex' in state 'state'
		 * @return the newly created GLabel based on information of the key descriptor for the key at index 'keyIndex' in state 'state'
		 */		
		GLabel* constructGLabelFromDescriptor(OpenViBE::uint32 keyIndex, VisualState state);
		
	protected:
		
		/**
		 * Graphical container with all static regular keys
		 */
		GContainer* m_pSymbolContainer;
		
		/**
		 * Graphical container with the keys containing the predicted words
		 */
		GContainer* m_pPredictionContainer;
		
		/**
		 * an object containing the properties read from the interface-properties.xml file
		 */
		P300InterfacePropertyReader* m_pPropertyObject;
		
		/**
		 * an object containing the descriptions of the key properties as specified in the keyboard layout files such as 5by10-abc-gray.xml
		 */		
		P300ScreenLayoutReader* m_pLayoutObject;
		
	private:
		/**
		 * the map that associates probabilities to colors
		 */
		std::map< OpenViBE::float64, GColor > m_mColorMap;
		
		/**
		 * probabilities for each symbol/word/key in the layout that have been accumulated so far
		 */
		std::vector<OpenViBE::float64> m_vSymbolProbabilties;
		
		/**
		 * A pre fixed number of symbols that have a probability that is higher than the specified significance level
		 */
		std::vector<OpenViBE::boolean> m_vProbableSymbols;
		
		/**
		 * The number of times a group of symbols have been in FLASH state
		 */
		std::vector<OpenViBE::uint32> m_vActiveCycles;
		
		/**
		 * current states of all symbols
		 */
		std::vector<OpenViBE::uint32> m_vSymbolStates;	
		std::vector<OpenViBE::uint32> m_vPreviousSymbolStates;	
		
		/**
		 * this font id is constructed so that we can construct a new font when we encounter a new source for example and are able
		 * to reuse the created font in case the source, size and key is the same (not sure if the keyIndex has to be part of this, unless
		 * you want to use different fonts for different symbols)
		 */
		struct FontID
		{
				OpenViBE::CString source;
				OpenViBE::float32 size;
				OpenViBE::uint32 keyIndex;
				bool operator<(const FontID& rhs) const
				{
					if (keyIndex < rhs.keyIndex)
							return true;
					else if (keyIndex == rhs.keyIndex)
					{
							if (size < rhs.size)
								return true;
							else if (size == rhs.size)
								return keyIndex < rhs.keyIndex;
					}
					return false;
				}
		};

		//std::vector<GButtonDescriptor> m_vButtonDescriptors;
		/**
		 * a vector to a vector of buttons, each vector of buttons representing a certain state
		 */
		std::vector< std::vector<GButton*>* >* m_vButtons;
		
		OpenViBE::uint32	m_ui32MaximumSymbolActivity;
		
		std::map<VisualState,VisualState> VisualStateMap;
		
		/**
		 * see FontID
		 */
		//std::map<FontID,boost::shared_ptr<FTFont> > m_mFontSourceMap;
	};
};
#endif
#endif

#endif
