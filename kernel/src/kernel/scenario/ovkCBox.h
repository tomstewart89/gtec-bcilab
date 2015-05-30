#ifndef __OpenViBEKernel_Kernel_Scenario_CBox_H__
#define __OpenViBEKernel_Kernel_Scenario_CBox_H__

#include "../ovkTKernelObject.h"

#include "ovkTAttributable.h"

#include <vector>
#include <string>
#include <iostream>

namespace OpenViBE
{
	namespace Kernel
	{
		class CScenario;

		class CBox : public OpenViBE::Kernel::TAttributable < OpenViBE::Kernel::TKernelObject < OpenViBE::Kernel::IBox > >
		{
		public:

			CBox(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::CScenario& rOwnerScenario);
			virtual ~CBox(void);

			virtual OpenViBE::CIdentifier getIdentifier(void) const;
			virtual OpenViBE::CString getName(void) const;
			virtual OpenViBE::CIdentifier getAlgorithmClassIdentifier(void) const;
			virtual OpenViBE::CIdentifier getProcessingUnitIdentifier(void) const;
			virtual OpenViBE::boolean setIdentifier(
				const OpenViBE::CIdentifier& rIdentifier);
			virtual OpenViBE::boolean setName(
				const OpenViBE::CString& sName);
			virtual OpenViBE::boolean setAlgorithmClassIdentifier(
				const OpenViBE::CIdentifier& rAlgorithmClassIdentifier);
			virtual OpenViBE::boolean setProcessingUnitIdentifier(
				const OpenViBE::CIdentifier& rProcessingUnitIdentifier);

			virtual OpenViBE::boolean initializeFromAlgorithmClassIdentifier(
				const OpenViBE::CIdentifier& rAlgorithmClassIdentifier);
			virtual OpenViBE::boolean initializeFromExistingBox(
				const OpenViBE::Kernel::IBox& rExistingBox);

			virtual OpenViBE::boolean addInput(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier);
			virtual OpenViBE::boolean removeInput(
				const OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::uint32 getInputCount(void) const;
			virtual OpenViBE::boolean getInputType(
				const OpenViBE::uint32 ui32InputIndex,
				OpenViBE::CIdentifier& rTypeIdentifier) const;
			virtual OpenViBE::boolean getInputName(
				const OpenViBE::uint32 ui32InputIndex,
				OpenViBE::CString& rName) const;
			virtual OpenViBE::boolean setInputType(
				const OpenViBE::uint32 ui32InputIndex,
				const OpenViBE::CIdentifier& rTypeIdentifier);
			virtual OpenViBE::boolean setInputName(
				const OpenViBE::uint32 ui32InputIndex,
				const OpenViBE::CString& rName);


			//Connector type
			virtual OpenViBE::boolean addInputSupport(
					const OpenViBE::CIdentifier& rTypeIdentifier);
			virtual OpenViBE::boolean hasInputSupport(
					const OpenViBE::CIdentifier& rTypeIdentifier) const;
			virtual OpenViBE::boolean addOutputSupport(
					const OpenViBE::CIdentifier& rTypeIdentifier);
			virtual OpenViBE::boolean hasOutputSupport(
					const OpenViBE::CIdentifier& rTypeIdentifier) const;
			virtual OpenViBE::boolean setSupportTypeFromAlgorithmIdentifier(
					const OpenViBE::CIdentifier& rTypeIdentifier);


			//messages input
			virtual OpenViBE::boolean addMessageInput(
				const OpenViBE::CString& sName);
			virtual OpenViBE::boolean removeMessageInput(
				const OpenViBE::uint32 ui32InputIndex);


			virtual OpenViBE::uint32 getMessageInputCount(void) const;
			virtual OpenViBE::boolean getMessageInputName(
				const OpenViBE::uint32 ui32InputIndex,
				OpenViBE::CString& rName) const;
			virtual OpenViBE::boolean setMessageInputName(
				const OpenViBE::uint32 ui32InputIndex,
				const OpenViBE::CString& rName);

			//message output
			virtual OpenViBE::boolean addMessageOutput(
				const OpenViBE::CString& sName);
			virtual OpenViBE::boolean removeMessageOutput(
				const OpenViBE::uint32 ui32InputIndex);


			virtual OpenViBE::uint32 getMessageOutputCount(void) const;
			virtual OpenViBE::boolean getMessageOutputName(
				const OpenViBE::uint32 ui32InputIndex,
				OpenViBE::CString& rName) const;
			virtual OpenViBE::boolean setMessageOutputName(
				const OpenViBE::uint32 ui32InputIndex,
				const OpenViBE::CString& rName);

			//

			virtual OpenViBE::boolean addOutput(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier);
			virtual OpenViBE::boolean removeOutput(
				const OpenViBE::uint32 ui32OutputIndex);
			virtual OpenViBE::uint32 getOutputCount(void) const;
			virtual OpenViBE::boolean getOutputType(
				const OpenViBE::uint32 ui32OutputIndex,
				OpenViBE::CIdentifier& rTypeIdentifier) const;
			virtual OpenViBE::boolean getOutputName(
				const OpenViBE::uint32 ui32OutputIndex,
				OpenViBE::CString& rName) const;
			virtual OpenViBE::boolean setOutputType(
				const OpenViBE::uint32 ui32OutputIndex,
				const OpenViBE::CIdentifier& rTypeIdentifier);
			virtual OpenViBE::boolean setOutputName(
				const OpenViBE::uint32 ui32OutputIndex,
				const OpenViBE::CString& rName);

			virtual OpenViBE::boolean addSetting(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier,
				const OpenViBE::CString& sDefaultValue,
				const OpenViBE::int32 i32Index=-1,
				const OpenViBE::boolean bModifiability = false);
			virtual OpenViBE::boolean removeSetting(
				const OpenViBE::uint32 ui32Index);
			virtual OpenViBE::uint32 getSettingCount(void) const;
			virtual OpenViBE::boolean getSettingType(
				const OpenViBE::uint32 ui32SettingIndex,
				OpenViBE::CIdentifier& rTypeIdentifier) const;
			virtual OpenViBE::boolean getSettingName(
				const OpenViBE::uint32 ui32SettingIndex,
				OpenViBE::CString& rName) const;
			virtual OpenViBE::boolean getSettingDefaultValue(
				const OpenViBE::uint32 ui32SettingIndex,
				OpenViBE::CString& rDefaultValue) const;
			virtual OpenViBE::boolean getSettingValue(
				const OpenViBE::uint32 ui32SettingIndex,
				OpenViBE::CString& rValue) const;
			virtual OpenViBE::boolean setSettingName(
				const OpenViBE::uint32 ui32SettingIndex,
				const OpenViBE::CString& rName);
			virtual OpenViBE::boolean setSettingType(
				const OpenViBE::uint32 ui32SettingIndex,
				const OpenViBE::CIdentifier& rTypeIdentifier);
			virtual OpenViBE::boolean setSettingDefaultValue(
				const OpenViBE::uint32 ui32SettingIndex,
				const OpenViBE::CString& rDefaultValue);
			virtual OpenViBE::boolean setSettingValue(
				const OpenViBE::uint32 ui32SettingIndex,
				const OpenViBE::CString& rValue);
			virtual void notifySettingChange(BoxEventMessageType eType,
					OpenViBE::int32 i32FirstIndex = -1,
					OpenViBE::int32 i32SecondIndex = -1);

			//*
			virtual OpenViBE::boolean getSettingMod(
				const OpenViBE::uint32 ui32SettingIndex,
				OpenViBE::boolean& rValue) const;
			virtual OpenViBE::boolean setSettingMod(
				const OpenViBE::uint32 ui32SettingIndex,
				const OpenViBE::boolean rValue);
			virtual OpenViBE::boolean hasModifiableSettings(void)const;

			virtual OpenViBE::uint32* getModifiableSettings(OpenViBE::uint32& rCount)const;
			//*/

			virtual OpenViBE::boolean acceptVisitor(
				OpenViBE::IObjectVisitor& rObjectVisitor);

			virtual void storeState(void);

			virtual void restoreState(void);

			_IsDerivedFromClass_Final_(OpenViBE::Kernel::TAttributable < OpenViBE::Kernel::TKernelObject < OpenViBE::Kernel::IBox > >, OVK_ClassId_Kernel_Scenario_Box)

		protected:

			virtual void clear(void);

			virtual void enableNotification(void);
			virtual void disableNotification(void);
			virtual void notify(
				const OpenViBE::Kernel::EBoxModification eBoxModificationType,
				const OpenViBE::uint32 ui32Index);
			virtual void notify(
				const OpenViBE::Kernel::EBoxModification eBoxModificationType)
			{
				this->notify(eBoxModificationType, 0xffffffff);
			}

		protected:

			class CInput
			{
			public:
				CInput(void) { }
				CInput(const CInput& i)
					:m_sName(i.m_sName)
					,m_oTypeIdentifier(i.m_oTypeIdentifier) { }
				OpenViBE::CString m_sName;
				OpenViBE::CIdentifier m_oTypeIdentifier;
			};

			class COutput
			{
			public:
				COutput(void) { }
				COutput(const COutput& o)
					:m_sName(o.m_sName)
					,m_oTypeIdentifier(o.m_oTypeIdentifier) { }
				OpenViBE::CString m_sName;
				OpenViBE::CIdentifier m_oTypeIdentifier;
			};

			class CSetting
			{
			public:
				CSetting(void) { }
				CSetting(const CSetting& s)
					:m_sName(s.m_sName)
					,m_oTypeIdentifier(s.m_oTypeIdentifier)
					,m_sDefaultValue(s.m_sDefaultValue)
					,m_sValue(s.m_sValue)
					,m_bMod(s.m_bMod) { }
				OpenViBE::CString m_sName;
				OpenViBE::CIdentifier m_oTypeIdentifier;
				OpenViBE::CString m_sDefaultValue;
				OpenViBE::CString m_sValue;
				OpenViBE::boolean m_bMod;
			};

			class CMessageInput
			{
			public:
				CMessageInput(void) { }
				CMessageInput(const CMessageInput& mi)
					:m_sName(mi.m_sName) { }
				OpenViBE::CString m_sName;

			};

			class CMessageOutput
			{
			public:
				CMessageOutput(void) { }
				CMessageOutput(const CMessageOutput& mi)
					:m_sName(mi.m_sName) { }
				OpenViBE::CString m_sName;

			};

		protected:

			OpenViBE::Kernel::CScenario& m_rOwnerScenario;
			const OpenViBE::Plugins::IBoxAlgorithmDesc* m_pBoxAlgorithmDescriptor;
			OpenViBE::Plugins::IBoxListener* m_pBoxListener;
			OpenViBE::boolean m_bIsNotifyingDescriptor;
			OpenViBE::boolean m_bIsNotificationActive;
			OpenViBE::boolean m_bIsObserverNotificationActive;

			OpenViBE::CIdentifier m_oIdentifier;
			OpenViBE::CIdentifier m_oAlgorithmClassIdentifier;
			OpenViBE::CIdentifier m_oProcessingUnitIdentifier;
			OpenViBE::CString m_sName;

			std::vector<CInput> m_vInput;
			std::vector<COutput> m_vOutput;
			std::vector<CSetting> m_vSetting;
			//to avoid having to recheck every setting every time
			//careful to update at each setting modification
			std::vector<OpenViBE::uint32> m_vModifiableSettingIndexes;

			std::vector<CIdentifier> m_vSupportInputType;
			std::vector<CIdentifier> m_vSupportOutputType;

			//only the name of the in/output are stored for message socket
			std::vector<CMessageInput> m_vMessageInput;
			std::vector<CMessageOutput> m_vMessageOutput;

			OpenViBE::Kernel::CBox *m_pSavedState;
		};
	};
};

#endif // __OpenViBEKernel_Kernel_Scenario_CBox_H__
