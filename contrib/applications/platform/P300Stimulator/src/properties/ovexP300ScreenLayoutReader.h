#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovCoAdaptP300LetterGroupReader__
#define __ovCoAdaptP300LetterGroupReader__


#include "ovexP300PropertyReader.h"
#include "ovexP300KeyDescriptor.h"

#include <exception>
#include <list>
#include <vector>
#include <cstring>
#include <map>

namespace OpenViBEApplications
{
	
	/**
	* Class that reads the xml file specifying the keyboard layout, files such as 5by10grid-abc-gray.xml
	*/	
	class P300ScreenLayoutReader : public CoAdaptP300PropertyReader
	{
		
	public:
		
		P300ScreenLayoutReader(OpenViBE::Kernel::IKernelContext* kernelContext);
		
		~P300ScreenLayoutReader();
		
		virtual void readPropertiesFromFile(OpenViBE::CString propertyFile);
		
		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		/**
		 * Dimensions of the area where the predictions are shown
		 * @return the box dimensions are numbers between 0 and 1. (0,0) is the left lower corner of the window
		 * (1,1) the right upper corner
		 */
		BoxDimensions getResultAreaDimensions() { return m_dResultAreaDimensions; }
		/**
		 * dimensions for area where the predicted words will be shown
		 */
		BoxDimensions getPredictionAreaDimensions() { return m_dPredictionAreaDimensions; }
		BoxDimensions getP300KeyboardDimensions() { return m_dKeyboardDimensions; }
		BoxDimensions getTargetAreaDimensions() { return m_dTargetAreaDimensions; }
		#endif
		/**
		 * @return vector of P300KeyDescriptor objects that describe the properties of each key in the keyboard for each state
		 */
		std::vector<P300KeyDescriptor*>* getP300KeyboardLayout() { return m_lKeyList; } 
		std::list<std::string> * getSymbolList() { return m_lSymbolList; }
		/**
		 * number of keys that are not predicted words. The labels of this type of keys can't change during the experiment
		 */
		OpenViBE::uint32 getNumberOfStandardKeys() { return m_ui32NumberOfStandardKeys; }
		/**
		 * number of keys will contain the predicted words. The labels of this type of keys will change after each trial
		 */		
		OpenViBE::uint32 getNumberOfPredictiveKeys() { return m_ui32PredictionAreaRows*m_ui32PredictionAreaColumns; }
		/**
		 * total number of keys
		 */
		OpenViBE::uint32 getNumberOfKeys() { return m_ui32NumberOfStandardKeys+getNumberOfPredictiveKeys(); }
		OpenViBE::uint32 getPredictionAreaRows() { return m_ui32PredictionAreaRows;}
		OpenViBE::uint32 getPredictionAreaColumns() { return m_ui32PredictionAreaColumns;}
		/**
		 * Checks whether dimensions parameters are given per key or not, if not a simple GTable will be created
		 */
		OpenViBE::boolean isKeyboardTable() { return m_bKeyboardIsGrid;}
		
		/**
		 *  some default properties that can be overridden per key
		 */
		const OpenViBE::float32 getDefaultScaleSize(const VisualState event) const { return m_mDefaultEventMapScaleSize->find(event)->second; }

		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		/**
		 *  some default properties that can be overridden per key
		 */
		const GColor& getDefaultForegroundColor(const VisualState event) const { return m_mDefaultEventMapForegroundColor->find(event)->second; }
		/**
		 *  some default properties that can be overridden per key
		 */
		const GColor& getDefaultBackgroundColor(const VisualState event) const { return m_mDefaultEventMapBackgroundColor->find(event)->second; }
		//const OpenViBE::CString& getDefaultSource(const VisualState event) const;// { return eventMapSource->find(event)->second; }
		//const std::string& getDefaultLabel(const VisualState event) const;// { return eventMapLabel->find(event)->second; }
		//const OpenViBE::boolean isDefaultTextSymbol(const VisualState event) const;// { return eventMapIsTextSymbol->find(event)->second; }
		#endif

	protected:
		void openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount); // XML IReaderCallback
		void processChildData(const char* sData); // XML IReaderCallback
		void closeChild(void); // XML ReaderCallback
		
	private:
		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		void parseDimensions(BoxDimensions& dimensions, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount);
		#endif
		void parseKeyLabels(const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount);

	protected:
		OpenViBE::uint32 m_ui32NumberOfStandardKeys;
		//OpenViBE::uint32 m_ui32NumberOfPredictiveKeys;
		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		BoxDimensions m_dKeyboardDimensions;
		BoxDimensions m_dPredictionAreaDimensions;
		BoxDimensions m_dResultAreaDimensions;
		BoxDimensions m_dTargetAreaDimensions;
		#endif
		/**
		 * vector of P300KeyDescriptor objects that describe the properties of each key in the keyboard for each state
		 */
		std::vector<P300KeyDescriptor*> * m_lKeyList;
		std::list<std::string> * m_lSymbolList;
		
		/**
		 * This default map variable maps each state, that a key can be in, into a property, here scale size of an image or font
		 * these default values are then used to fill in the properties that have not been specified
		 */
		std::map<OpenViBE::uint32, OpenViBE::float32>* m_mDefaultEventMapScaleSize;

		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		/**
		 * This default map variable maps each state, that a key can be in, into a property, here the foreground color
		 * these default values are then used to fill in the properties that have not been specified
		 */	
		std::map<OpenViBE::uint32, GColor>* m_mDefaultEventMapForegroundColor;
		/**
		 * This default map variable maps each state, that a key can be in, into a property, here the background color
		 * these default values are then used to fill in the properties that have not been specified
		 */			
		std::map<OpenViBE::uint32, GColor>* m_mDefaultEventMapBackgroundColor;
		#endif

		/**
		 * This default map variable maps each state, that a key can be in, into a property, here the source of the key, either a font file or a png file
		 * these default values are then used to fill in the properties that have not been specified
		 */		
		std::map<OpenViBE::uint32, OpenViBE::CString>* m_mDefaultEventMapSource;
		/**
		 * This default map variable maps each state, that a key can be in, into a property, here the default label of a key in case it is a text label
		 * these default values are then used to fill in the properties that have not been specified
		 */		
		std::map<OpenViBE::uint32, std::string>* m_mDefaultEventMapLabel;	
		/**
		 * This default map variable maps each state, that a key can be in, into a property, here if it concerns a text symbol or a picture
		 * these default values are then used to fill in the properties that have not been specified
		 */		
		std::map<OpenViBE::uint32, OpenViBE::boolean>* m_mDefaultIsTextSymbol;
		
		OpenViBE::boolean m_bKeyboardIsGrid;
		OpenViBE::uint32 m_ui32PredictionAreaRows;
		OpenViBE::uint32 m_ui32PredictionAreaColumns;
		
	private:
		P300KeyDescriptor* m_pKey;
		OpenViBE::boolean m_bDefaultKeyProperties;
		OpenViBE::boolean m_bEventElement;
		OpenViBE::boolean m_bScaleSize, m_bForegroundColor, m_bBackgroundColor, m_bLabel, m_bSource;
		VisualState m_iState;
		
		static OpenViBE::CString KeyEventStrings[5];
		/**
		 * will map the string representation of the key states to an enum value
		 */
		std::map<OpenViBE::CString,VisualState> EventStringMap;				
	};
};

#endif

#endif
