#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovCoAdaptP300InterfacePropertyReader__
#define __ovCoAdaptP300InterfacePropertyReader__


#include "ovexP300PropertyReader.h"

namespace OpenViBEApplications
{
	
	/**
	* Class that reads the xml file specifying the general interface properties in the file interface-properties.xml
	*/		
	class P300InterfacePropertyReader : public CoAdaptP300PropertyReader
	{	
		
	public:	
		P300InterfacePropertyReader(OpenViBE::Kernel::IKernelContext* kernelContext);

		/**
		 * the file that specifies the keyboard layout
		 */
		OpenViBE::CString getScreenDefinitionFile() { return m_sSymbolDefinitionFile; }	
		
		/**
		 * name of the file where the stimulator will save which symbols are flashed each time
		 */
		OpenViBE::CString getFlashGroupDefinitionFile() { return m_sFlashGroupDefinitionFile; }
		OpenViBE::CString getStimulatorConfigFile() { return m_sStimulatorConfigFile; }
		OpenViBE::float32 getWidth() { return m_f32WindowWidth; }
		OpenViBE::float32 getHeight() { return m_f32WindowHeight; }	
		OpenViBE::boolean getFullScreen() { return m_bFullScreen; }
		OpenViBE::uint32 getParallelPortNumber() { return m_ui32ParallelPortNumber; }
		OpenViBE::uint32 getSampleFrequency() { return m_ui32SampleFrequency; }
		/**
		 * @return true if feedback is presented in the center of the screen, false if the symbol in the keyboard itself is highlighted
		 */
		OpenViBE::boolean getCentralFeedbackFreeMode() { return m_bCentralFeedbackFreeMode; }
		OpenViBE::boolean getCentralFeedbackCopyMode() { return m_bCentralFeedbackCopyMode; }
		SpellingMode getSpellingMode() { return m_eSpellingMode; }
		OpenViBE::CString getStimulatorMode() {return m_sStimulatorMode; }
		OpenViBE::boolean getHardwareTagging() { return m_bHardwareTagging; }
		/**
		 * TODO for now the code enabling the use of the photo diode is deprecated
		 */
		OpenViBE::boolean isPhotoDiodeEnabled() { return m_bEnablePhotoDiode; }
		OpenViBE::CString getFlashMode() { return m_sFlashMode; }
		/**
		 * number of most probable symbols that should be highlighted during flashing
		 */
		OpenViBE::uint32 getMaxFeedbackSymbols() { return m_ui32MaxFeedbackSymbols; }

		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		/**
		 * start color of the most probable symbols when the minimum probability threshold is reached as specified by getFeedbackStartValue
		 */
		GColor getFeedbackStartColor() { return m_oFeedbackStartColor;}
		/**
		 * end color of the most probable symbols, for the symbol reaching probability one
		 */		
		GColor getFeedbackEndColor() { return m_oFeedbackEndColor;}
		#endif
		/**
		 * number of steps for the gradient to go from start color to end color
		 */
		OpenViBE::uint32 getColorFeedbackSteps() { return m_ui32ColorFeedbackSteps;}
		/**
		 * The probability from which we want to start giving the most probable symbols a color during flashing
		 */
		OpenViBE::float32 getFeedbackStartValue() { return m_f32FeedbackStartValue;}
		/**
		 * whether continuous feedback during flashing is enabled (during flashing the colors are changed for the most probable symbols)\n
		 * The color gives the user an indication of how confident the system is in the symbols 
		 * TODO the code is deprecated and should be redone
		 */
		OpenViBE::boolean isContinuousFeedbackEnabled() { return m_bContinuousFeedback;}
		/**
		 * The name of the database used by Presage for word prediction
		 */
		OpenViBE::CString getNGramDatabaseName() { return m_sNGramDatabaseName; }

		OpenViBE::uint32 getMonitorIndex() { return m_ui32MonitorIndex;}

	protected:
		void openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount); // XML IReaderCallback
		void processChildData(const char* sData); // XML IReaderCallback
		void closeChild(void); // XML ReaderCallback

	protected:
		SpellingMode m_eSpellingMode;
		OpenViBE::CString m_sStimulatorConfigFile;
		OpenViBE::CString m_sSymbolDefinitionFile;
		OpenViBE::CString m_sAdditionalConfigurationFile;
		OpenViBE::CString m_sFlashGroupDefinitionFile;
		OpenViBE::boolean m_bFullScreen;
		OpenViBE::boolean m_bCentralFeedbackFreeMode;
		OpenViBE::boolean m_bCentralFeedbackCopyMode;
		OpenViBE::uint32 m_ui32ParallelPortNumber;
		OpenViBE::uint32 m_ui32SampleFrequency;
		OpenViBE::float32 m_f32WindowWidth;
		OpenViBE::float32 m_f32WindowHeight;
		OpenViBE::CString m_sFlashMode;
		OpenViBE::CString m_sNGramDatabaseName;
		
		OpenViBE::boolean m_bHardwareTagging;
		OpenViBE::CString m_sFeedbackPresentationMode;
		OpenViBE::boolean m_bEnablePhotoDiode;
		
		OpenViBE::uint32 m_ui32MaxFeedbackSymbols;
		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		GColor m_oFeedbackStartColor;
		GColor m_oFeedbackEndColor;
		#endif
		OpenViBE::uint32 m_ui32ColorFeedbackSteps;
		OpenViBE::float32 m_f32FeedbackStartValue;
		OpenViBE::boolean m_bContinuousFeedback;

		OpenViBE::CString m_sStimulatorMode;
		OpenViBE::uint32 m_ui32MonitorIndex;
		
	};
};
#endif

#endif
