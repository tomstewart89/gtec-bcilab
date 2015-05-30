#ifndef __OpenViBEDesigner_CBoxConfigurationDialog_H__
#define __OpenViBEDesigner_CBoxConfigurationDialog_H__

#include "ovd_base.h"

#include <string>
#include <vector>
#include <map>
#include "settings/ovdCAbstractSettingView.h"
#include "settings/ovdCSettingViewFactory.h"

namespace OpenViBEDesigner
{
	class CBoxConfigurationDialog : public OpenViBE::IObserver
	{
	public:

		CBoxConfigurationDialog(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IBox& rBox, const char* sGUIFilename, const char* sGUISettingsFilename, bool bMode=false);
		virtual ~CBoxConfigurationDialog(void);
		virtual OpenViBE::boolean run(bool bMode);
		virtual ::GtkWidget* getWidget();
		virtual const OpenViBE::CIdentifier getBoxID() const;

		virtual void update(OpenViBE::CObservable &o, void* data);

		void updateSize();
		void saveConfiguration();
		void loadConfiguration();
		void onOverrideBrowse(void);

		//This function is introduce to revert the change made during the execution of a scenario (modifiable settings)
		void revertChange(void);

	protected:
		void generateSettingsTable(void);
		OpenViBE::boolean addSettingsToView(OpenViBE::uint32 ui32SettingIndex, OpenViBE::uint32 ui32TableIndex);

		void settingChange(OpenViBE::uint32 ui32SettingIndex);
		void addSetting(OpenViBE::uint32 ui32SettingIndex);

		void clearSettingWrappersVector(void);
		void removeSetting(OpenViBE::uint32 ui32SettingIndex, OpenViBE::boolean bShift = true);
		OpenViBE::int32 getTableIndex(OpenViBE::uint32 ui32SettingIndex);
		OpenViBE::uint32 getTableSize(void);

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IBox& m_rBox;
		OpenViBE::CString m_sGUIFilename;
		OpenViBE::CString m_sGUISettingsFilename;
		//
		::GtkWidget* m_pWidget;//widget with the dialog for configuration (used whole for box config when no scenario is running)
		::GtkWidget* m_pWidgetToReturn; //child of m_oWidget, if we are running a scenario, this is the widget we need, the rest can be discarded

		::GtkEntry* m_pOverrideEntry;

		::GtkTable *m_pSettingsTable;
		::GtkViewport *m_pViewPort;
		::GtkScrolledWindow * m_pScrolledWindow;

		bool m_bIsScenarioRunning; // true if the scenario is running, false otherwise

		Setting::CSettingViewFactory m_oSettingFactory;

		std::vector<Setting::CAbstractSettingView* > m_vSettingViewVector;

		::GtkCheckButton* m_pFileOverrideCheck;
	};
}

#endif // __OpenViBEDesigner_CBoxConfigurationDialog_H__
