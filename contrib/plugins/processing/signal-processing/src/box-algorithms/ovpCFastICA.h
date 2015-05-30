
// @copyright notice: Possibly due to dependencies, this box used to be GPL before upgrade to AGPL3

#ifndef __OpenViBEPlugins_SignalProcessing_CFastICA_H__
#define __OpenViBEPlugins_SignalProcessing_CFastICA_H__

#if defined TARGET_HAS_ThirdPartyITPP

#include "../ovp_defines.h"

#include <toolkit/ovtk_all.h>

#include <vector>
#include <map>
#include <string>

// TODO create a member function to get rid of this
#ifndef  CString2Boolean
	#define CString2Boolean(string) (strcmp(string,"true"))?0:1
#endif

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		/**
		* The FastICA plugin's main class.
		*/
		class CFastICA : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CFastICA(void);

			virtual void release(void);

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);

			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_FastICA)

		public:

			virtual void computeICA(void);

		public:

			OpenViBEToolkit::TSignalDecoder<CFastICA> m_oDecoder;
			OpenViBEToolkit::TSignalEncoder<CFastICA> m_oEncoder;

		};

		class CFastICADesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Independent component analysis (FastICA)"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Guillaume Gibert"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INSERM"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Computes fast independent component analysis"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Independent component analysis"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_FastICA; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CFastICA(); }

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addInput ("Input signal",  OV_TypeId_Signal);
				rPrototype.addOutput("Output signal", OV_TypeId_Signal);
				rPrototype.addFlag  (OpenViBE::Kernel::BoxFlag_IsUnstable);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_FastICADesc)
		};
	}
}
#endif // TARGET_HAS_ThirdPartyITPP

#endif // __SamplePlugin_CFastICA_H__
