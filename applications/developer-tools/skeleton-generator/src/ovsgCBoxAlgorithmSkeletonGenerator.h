#ifndef __OpenViBESkeletonGenerator_CBoxAlgorithmSkeletonGenerator_H__
#define __OpenViBESkeletonGenerator_CBoxAlgorithmSkeletonGenerator_H__

#include "ovsgCSkeletonGenerator.h"
#include <map>
#include <vector>

namespace OpenViBESkeletonGenerator
{
	class CBoxAlgorithmSkeletonGenerator : public CSkeletonGenerator
	{
		public:

			CBoxAlgorithmSkeletonGenerator(OpenViBE::Kernel::IKernelContext & rKernelContext,::GtkBuilder * pBuilderInterface);
			virtual ~CBoxAlgorithmSkeletonGenerator(void);

			OpenViBE::boolean initialize(void);
			OpenViBE::boolean save(OpenViBE::CString sFileName);
			OpenViBE::boolean load(OpenViBE::CString sFileName);
			void getCurrentParameters(void);
		
			// Box Description
			OpenViBE::CString              m_sName;
			OpenViBE::CString              m_sVersion;
			OpenViBE::CString              m_sClassName;
			OpenViBE::CString              m_sCategory;
			OpenViBE::CString              m_sShortDescription;
			OpenViBE::CString              m_sDetailedDescription;
			OpenViBE::int32                m_i32GtkStockItemIndex;
			OpenViBE::CString              m_sGtkStockItemName;

			struct IOSStruct{
				OpenViBE::CString _name;
				OpenViBE::CString _type;
				OpenViBE::CString _typeId;
				OpenViBE::CString _defaultValue;
			};

			// Inputs
			OpenViBE::boolean              m_bCanModifyInputs;
			OpenViBE::boolean              m_bCanAddInputs;
			std::vector<IOSStruct>         m_vInputs;
			// Outputs
			OpenViBE::boolean              m_bCanModifyOutputs;
			OpenViBE::boolean              m_bCanAddOutputs;
			std::vector<IOSStruct>         m_vOutputs;
			// Settings
			OpenViBE::boolean              m_bCanModifySettings;
			OpenViBE::boolean              m_bCanAddSettings;
			std::vector<IOSStruct>         m_vSettings;
			// Message Inputs
			OpenViBE::boolean              m_bCanModifyMessageInputs;
			OpenViBE::boolean              m_bCanAddMessageInputs;
			std::vector<IOSStruct>         m_vMessageInputs;
			// Message Outputs
			OpenViBE::boolean              m_bCanModifyMessageOutputs;
			OpenViBE::boolean              m_bCanAddMessageOutputs;
			std::vector<IOSStruct>         m_vMessageOutputs;

			//Algorithms
			std::vector<OpenViBE::CString> m_vAlgorithms; // the algorithm selected by user
			// Can be made non-const after '= false' produces working code
			static const OpenViBE::boolean              m_bUseCodecToolkit = true; // use or not the codec toolkit for encoder and decoder algorithms
			std::map <OpenViBE::CString, OpenViBE::CString> m_mAlgorithmHeaderDeclaration; //the map between algorithm and corresponding header declaration (all variables algo/input/output).
			std::map <OpenViBE::CString, OpenViBE::CString> m_mAlgorithmInitialisation;//the map between algorithm and corresponding initialisation
			std::map <OpenViBE::CString, OpenViBE::CString> m_mAlgorithmInitialisation_ReferenceTargets;//the map between algorithm and corresponding initialisation of ref targets
			std::map <OpenViBE::CString, OpenViBE::CString> m_mAlgorithmUninitialisation;//the map between algorithm and corresponding uninitialisation
			
			// Box Listener
			OpenViBE::boolean              m_bUseBoxListener;
			// input
			OpenViBE::boolean              m_bBoxListenerOnInputAdded;
			OpenViBE::boolean              m_bBoxListenerOnInputRemoved;
			OpenViBE::boolean              m_bBoxListenerOnInputTypeChanged;
			OpenViBE::boolean              m_bBoxListenerOnInputNameChanged;
			OpenViBE::boolean              m_bBoxListenerOnInputConnected;
			OpenViBE::boolean              m_bBoxListenerOnInputDisconnected;
			// output
			OpenViBE::boolean              m_bBoxListenerOnOutputAdded;
			OpenViBE::boolean              m_bBoxListenerOnOutputRemoved;
			OpenViBE::boolean              m_bBoxListenerOnOutputTypeChanged;
			OpenViBE::boolean              m_bBoxListenerOnOutputNameChanged;
			OpenViBE::boolean              m_bBoxListenerOnOutputConnected;
			OpenViBE::boolean              m_bBoxListenerOnOutputDisconnected;
			// setting
			OpenViBE::boolean              m_bBoxListenerOnSettingAdded;
			OpenViBE::boolean              m_bBoxListenerOnSettingRemoved;
			OpenViBE::boolean              m_bBoxListenerOnSettingTypeChanged;
			OpenViBE::boolean              m_bBoxListenerOnSettingNameChanged;
			OpenViBE::boolean              m_bBoxListenerOnSettingDefaultValueChanged;
			OpenViBE::boolean              m_bBoxListenerOnSettingValueChanged;
			
			OpenViBE::boolean              m_bProcessInput;
			OpenViBE::boolean              m_bProcessClock;
			OpenViBE::uint32               m_ui32ClockFrequency;
			OpenViBE::boolean              m_bProcessMessage;

			void buttonCheckCB(void);
			void buttonOkCB(void);
			void forceRecheckCB(void);
			void toggleListenerCheckbuttonsStateCB(OpenViBE::boolean bNewState);
			void toggleClockFrequencyStateCB(OpenViBE::boolean bNewState);
			void buttonTooltipCB(::GtkButton* pButton);
			void buttonExitCB(void);

			void buttonAddInputCB(void);
			void buttonRemoveInputCB(void);
			void buttonAddOutputCB(void);
			void buttonRemoveOutputCB(void);
			void buttonAddSettingCB(void);
			void buttonRemoveSettingCB(void);
			void buttonAddAlgorithmCB(void);
			void buttonRemoveAlgorithmCB(void);
			void algorithmSelectedCB(OpenViBE::int32 i32IndexSelected);

			void buttonAddMessageInputCB(void);
			void buttonRemoveMessageInputCB(void);
			void buttonAddMessageOutputCB(void);
			void buttonRemoveMessageOutputCB(void);

		private:

			std::map < ::GtkButton*, OpenViBE::CString > m_vTooltips;

			OpenViBE::CString getRandomIdentifierString(void);

			std::vector<OpenViBE::CString> m_vParameterType_EnumTypeCorrespondance;

			// Sanity checks that a string is not empty or consist of spaces
			OpenViBE::boolean isStringValid(const char *string);
	};

	class CDummyAlgoProto : public OpenViBE::Kernel::IAlgorithmProto
	{
	public:
		std::map<OpenViBE::CString, OpenViBE::Kernel::EParameterType> m_vInputs;
		std::map<OpenViBE::CString, OpenViBE::Kernel::EParameterType> m_vOutputs;
		std::vector<OpenViBE::CString> m_vInputTriggers;
		std::vector<OpenViBE::CString> m_vOutputTriggers;
	public:
		OpenViBE::boolean addInputParameter(
			const OpenViBE::CIdentifier& rInputParameterIdentifier,
			const OpenViBE::CString& sInputName,
			const OpenViBE::Kernel::EParameterType eParameterType,
			const OpenViBE::CIdentifier& rSubTypeIdentifier=OV_UndefinedIdentifier);
			
		OpenViBE::boolean addOutputParameter(
			const OpenViBE::CIdentifier& rOutputParameterIdentifier,
			const OpenViBE::CString& sOutputName,
			const OpenViBE::Kernel::EParameterType eParameterType,
			const OpenViBE::CIdentifier& rSubTypeIdentifier=OV_UndefinedIdentifier);
			
		OpenViBE::boolean addInputTrigger(
			const OpenViBE::CIdentifier& rInputTriggerIdentifier,
			const OpenViBE::CString& rInputTriggerName);
			
		OpenViBE::boolean addOutputTrigger(
			const OpenViBE::CIdentifier& rOutputTriggerIdentifier,
			const OpenViBE::CString& rOutputTriggerName);

		OpenViBE::CIdentifier getClassIdentifier(void) const {return OV_UndefinedIdentifier;}
	};
}

#endif //__OpenViBESkeletonGenerator_CBoxAlgorithmSkeletonGenerator_H__
