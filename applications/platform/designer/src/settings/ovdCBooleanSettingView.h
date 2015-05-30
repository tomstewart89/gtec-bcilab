#ifndef __OpenViBE_Designer_Setting_CBooleanSettingView_H__
#define __OpenViBE_Designer_Setting_CBooleanSettingView_H__

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CBooleanSettingView : public CAbstractSettingView
		{
		public:
			CBooleanSettingView(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index, OpenViBE::CString &rBuilderName);

			virtual void getValue(OpenViBE::CString &rValue) const;
			virtual void setValue(const OpenViBE::CString &rValue);

			void toggleButtonClick();


		private:
			::GtkToggleButton* m_pToggle;
			::GtkEntry* m_pEntry;
			OpenViBE::boolean m_bOnValueSetting;
		};
	}

}


#endif // __OpenViBE_Designer_Setting_CBooleanSettingView_H__
