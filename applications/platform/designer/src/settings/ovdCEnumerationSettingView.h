#ifndef __OpenViBE_Designer_Setting_CEnumerationSettingView_H__
#define __OpenViBE_Designer_Setting_CEnumerationSettingView_H__

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

#include <map>

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CEnumerationSettingView : public CAbstractSettingView
		{
		public:
			CEnumerationSettingView(OpenViBE::Kernel::IBox& rBox,
								OpenViBE::uint32 ui32Index,
								OpenViBE::CString &rBuilderName,
								const OpenViBE::Kernel::IKernelContext& rKernelContext,
									const OpenViBE::CIdentifier &rTypeIdentifier);

			virtual void getValue(OpenViBE::CString &rValue) const;
			virtual void setValue(const OpenViBE::CString &rValue);

			void onChange();


		private:
			::GtkComboBox* m_pComboBox;
			OpenViBE::CIdentifier m_oTypeIdentifier;
			OpenViBE::boolean p;

			std::map < OpenViBE::CString, OpenViBE::uint64 > m_mEntriesIndex;

			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			OpenViBE::boolean m_bOnValueSetting;
		};
	}

}

#endif // __OpenViBE_Designer_Setting_CEnumerationSettingView_H__
