#ifndef __OpenViBE_Designer_Setting_CFilenameSettingView_H__
#define __OpenViBE_Designer_Setting_CFilenameSettingView_H__

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CFilenameSettingView : public CAbstractSettingView
		{
		public:
			CFilenameSettingView(OpenViBE::Kernel::IBox& rBox,
								 OpenViBE::uint32 ui32Index,
								 OpenViBE::CString &rBuilderName,
								 const OpenViBE::Kernel::IKernelContext& rKernelContext);

			virtual void getValue(OpenViBE::CString &rValue) const;
			virtual void setValue(const OpenViBE::CString &rValue);

			void browse();
			void onChange();

		private:
			::GtkEntry* m_pEntry;

			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			OpenViBE::boolean m_bOnValueSetting;
		};
	}

}


#endif // __OpenViBE_Designer_Setting_CFilenameSettingView_H__
