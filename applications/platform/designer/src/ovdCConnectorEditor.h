#ifndef __OpenViBEDesigner_CConnectorEditor_H__
#define __OpenViBEDesigner_CConnectorEditor_H__

#include "ovd_base.h"

#include <string>

namespace OpenViBEDesigner
{
	class CConnectorEditor
	{
	public:

		CConnectorEditor(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32ConnectorType, OpenViBE::uint32 ui32ConnectorIndex, const char* sTitle, const char* sGUIFilename);
		virtual ~CConnectorEditor(void);
		virtual OpenViBE::boolean run(void);

	protected:

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IBox& m_rBox;
		OpenViBE::uint32 m_ui32ConnectorType;
		OpenViBE::uint32 m_ui32ConnectorIndex;
		const std::string m_sGUIFilename;
		const std::string m_sTitle;

	private:

		typedef OpenViBE::uint32 (OpenViBE::Kernel::IBox::*t_getConnectorCount)(void) const;
		typedef OpenViBE::boolean (OpenViBE::Kernel::IBox::*t_getConnectorType)(
			const OpenViBE::uint32 ui32InputIndex,
			OpenViBE::CIdentifier& rTypeIdentifier) const;
		typedef OpenViBE::boolean (OpenViBE::Kernel::IBox::*t_getConnectorName)(
			const OpenViBE::uint32 ui32InputIndex,
			OpenViBE::CString& rName) const;
		typedef OpenViBE::boolean (OpenViBE::Kernel::IBox::*t_setConnectorType)(
			const OpenViBE::uint32 ui32InputIndex,
			const OpenViBE::CIdentifier& rTypeIdentifier);
		typedef OpenViBE::boolean (OpenViBE::Kernel::IBox::*t_setConnectorName)(
			const OpenViBE::uint32 ui32InputIndex,
			const OpenViBE::CString& rName);
		typedef OpenViBE::boolean (OpenViBE::Kernel::IBox::*t_isTypeSupported)(
			const OpenViBE::CIdentifier& rTypeIdentifier) const;
	};
};

#endif // __OpenViBEDesigner_CConnectorEditor_H__
