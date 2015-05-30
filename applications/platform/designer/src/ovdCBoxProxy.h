#ifndef __OpenViBEDesigner_CBoxProxy_H__
#define __OpenViBEDesigner_CBoxProxy_H__

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CBoxProxy
	{
	public:

		CBoxProxy(
			const OpenViBE::Kernel::IKernelContext& rKernelContext,
			const OpenViBE::Kernel::IBox& rBox);
		CBoxProxy(
			const OpenViBE::Kernel::IKernelContext& rKernelContext,
			OpenViBE::Kernel::IScenario& rScenario,
			const OpenViBE::CIdentifier& rBoxIdentifier);
		virtual ~CBoxProxy(void);

		operator OpenViBE::Kernel::IBox* (void);
		operator const OpenViBE::Kernel::IBox* (void);

		OpenViBE::int32 getWidth(
			::GtkWidget* pWidget) const;
		OpenViBE::int32 getHeight(
			::GtkWidget* pWidget) const;

		OpenViBE::float64 getXCenter(void) const;
		OpenViBE::float64 getYCenter(void) const;

		void setCenter(
			OpenViBE::float64 f64XCenter,
			OpenViBE::float64 f64YCenter);

		void apply(void);

		virtual const char* getLabel(void) const;

		OpenViBE::boolean isBoxAlgorithmPluginPresent(void) const;
		OpenViBE::boolean isUpToDate(void) const;
		OpenViBE::boolean isDeprecated(void) const;
		OpenViBE::boolean isUnstable(void) const;

		OpenViBE::boolean getMute() const;
		void setMute(OpenViBE::boolean bIsMute);

	protected:

		virtual void updateSize(
			::GtkWidget* pWidget,
			const char* sText,
			int* pXSize,
			int* pYSize) const;

	protected:

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		const OpenViBE::Kernel::IBox* m_pConstBox;
		OpenViBE::Kernel::IBox* m_pBox;
		OpenViBE::boolean m_bApplied;
		OpenViBE::boolean m_bShowOriginalNameWhenModified;
		mutable int m_iWidth;
		mutable int m_iHeight;
		OpenViBE::float64 m_f64XCenter;
		OpenViBE::float64 m_f64YCenter;
		mutable std::string m_sLabel;
	};
};

#endif // __OpenViBEDesigner_CBoxProxy_H__
