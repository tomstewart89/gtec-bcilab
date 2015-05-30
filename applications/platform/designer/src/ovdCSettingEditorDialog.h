#ifndef __OpenViBEDesigner_CSettingEditorDialog_H__
#define __OpenViBEDesigner_CSettingEditorDialog_H__

#include "settings/ovdCSettingViewFactory.h"
#include "settings/ovdCAbstractSettingView.h"

#include "ovd_base.h"

#include <string>
#include <map>

namespace OpenViBEDesigner
{
	class CSettingEditorDialog : public OpenViBE::IObserver
	{
	public:

		CSettingEditorDialog(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32SettingIndex, const char* sTitle, const char* sGUIFilename, const char* sGUISettingsFilename);
		virtual ~CSettingEditorDialog(void);
		virtual OpenViBE::boolean run(void);

		virtual void typeChangedCB(void);
		virtual void update(OpenViBE::CObservable &o, void* data);


	protected:

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IBox& m_rBox;
		Setting::CSettingViewFactory m_oSettingFactory;
		OpenViBE::uint32 m_ui32SettingIndex;
		OpenViBE::CString m_sGUIFilename;
		OpenViBE::CString m_sGUISettingsFilename;
		std::string m_sTitle;
		::GtkWidget* m_pTable;
		::GtkWidget* m_pType;
		::GtkWidget* m_pDefaultValue;
		std::map<std::string, OpenViBE::CIdentifier> m_vSettingTypes;
		Setting::CAbstractSettingView * m_pSettingView;

	};
};

#endif // __OpenViBEDesigner_CSettingEditorDialog_H__
