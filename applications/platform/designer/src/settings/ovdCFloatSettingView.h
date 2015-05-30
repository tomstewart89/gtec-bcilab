#ifndef __OpenViBE_Designer_Setting_CFloatSettingView_H__
#define __OpenViBE_Designer_Setting_CFloatSettingView_H__

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CFloatSettingView : public CAbstractSettingView
		{
		public:
			CFloatSettingView(OpenViBE::Kernel::IBox& rBox,
								OpenViBE::uint32 ui32Index,
								OpenViBE::CString &rBuilderName,
								const OpenViBE::Kernel::IKernelContext& rKernelContext);

			virtual void getValue(OpenViBE::CString &rValue) const;
			virtual void setValue(const OpenViBE::CString &rValue);

			void adjustValue(OpenViBE::float64 amount);
			void onChange();




		private:
			::GtkEntry* m_pEntry;

			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			OpenViBE::boolean m_bOnValueSetting;
		};
	}

}

#endif // __OpenViBE_Designer_Setting_CFloatSettingView_H__
