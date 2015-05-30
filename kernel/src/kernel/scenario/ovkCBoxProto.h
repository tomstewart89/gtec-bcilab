#ifndef __OpenViBEKernel_Kernel_Scenario_CBoxProto_H__
#define __OpenViBEKernel_Kernel_Scenario_CBoxProto_H__

#include "../ovkTKernelObject.h"

namespace OpenViBE
{
	namespace Kernel
	{
		class CBoxProto : public OpenViBE::Kernel::TKernelObject<OpenViBE::Kernel::IBoxProto>
		{
		public:

			CBoxProto(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IBox& rBox);

			virtual OpenViBE::boolean addInput(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier);

			virtual OpenViBE::boolean addMessageInput(
				const OpenViBE::CString& sName);
			virtual OpenViBE::boolean addMessageOutput(
				const OpenViBE::CString& sName);

			virtual OpenViBE::boolean addOutput(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier);

			/*
			virtual OpenViBE::boolean addSetting(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier,
				const OpenViBE::CString& sDefaultValue);
				//*/

			virtual OpenViBE::boolean addSetting(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier,
				const OpenViBE::CString& sDefaultValue,
				const OpenViBE::boolean bModifiable = false);

			virtual OpenViBE::boolean addFlag(
				const OpenViBE::Kernel::EBoxFlag eBoxFlag);
			virtual OpenViBE::boolean addInputSupport(
				const OpenViBE::CIdentifier &rTypeIdentifier);
			virtual OpenViBE::boolean addOutputSupport(
				const OpenViBE::CIdentifier &rTypeIdentifier);

			_IsDerivedFromClass_Final_(OpenViBE::Kernel::IBoxProto, OVK_ClassId_Kernel_Scenario_BoxProto)

		protected:

			OpenViBE::Kernel::IBox& m_rBox;

		private:

			CBoxProto(void);
		};
	};
};

#endif // __OpenViBEKernel_Kernel_Scenario_CBoxProto_H__
