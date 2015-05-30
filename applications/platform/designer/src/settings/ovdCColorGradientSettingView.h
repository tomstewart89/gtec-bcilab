#ifndef __OpenViBE_Designer_Setting_CColorGradientSettingView_H__
#define __OpenViBE_Designer_Setting_CColorGradientSettingView_H__

#include "../ovd_base.h"
#include "ovdCAbstractSettingView.h"

#include <string>
#include <map>
#include <vector>

namespace OpenViBEDesigner
{
	namespace Setting
	{
		typedef struct
		{
			OpenViBE::float64 fPercent;
			::GdkColor oColor;
			::GtkColorButton* pColorButton;
			::GtkSpinButton* pSpinButton;
		} SColorGradientDataNode;


		class CColorGradientSettingView : public CAbstractSettingView
		{
		public:
			CColorGradientSettingView(OpenViBE::Kernel::IBox& rBox,
									  OpenViBE::uint32 ui32Index,
									  OpenViBE::CString &rBuilderName,
									  const OpenViBE::Kernel::IKernelContext& rKernelContext);

			virtual void getValue(OpenViBE::CString &rValue) const;
			virtual void setValue(const OpenViBE::CString &rValue);

			void configurePressed();

			void initializeGradient();
			void refreshColorGradient();
			void addColor();
			void removeColor();

			void spinChange(::GtkSpinButton *pButton);
			void colorChange(::GtkColorButton *pButton);

			void onChange();


		private:
			::GtkEntry* m_pEntry;
			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			OpenViBE::CString m_sBuilderName;

			::GtkWidget* pDialog;
			::GtkWidget* pContainer;
			::GtkWidget* pDrawingArea;
			std::vector < SColorGradientDataNode > vColorGradient;
			std::map < ::GtkColorButton*, OpenViBE::uint32 > vColorButtonMap;
			std::map < ::GtkSpinButton*, OpenViBE::uint32 > vSpinButtonMap;

			OpenViBE::boolean m_bOnValueSetting;
		};
	}

}

#endif // __OpenViBE_Designer_Setting_CColorGradientSettingView_H__
