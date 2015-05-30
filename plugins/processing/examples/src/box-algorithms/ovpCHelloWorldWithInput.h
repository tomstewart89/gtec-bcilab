#ifndef __SamplePlugin_CHelloWorldWithInput_H__
#define __SamplePlugin_CHelloWorldWithInput_H__

#include "../ovp_defines.h"
#include <toolkit/ovtk_all.h>
#include <cstdio>

namespace OpenViBEPlugins
{
	namespace Examples
	{
		class CHelloWorldWithInput : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			virtual void release(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_HelloWorldWithInput)
		};

		class CHelloWorldWithInputListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			// The purposes of the following functions is to make the output correspond to the input

			virtual OpenViBE::boolean onInputNameChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				OpenViBE::CString l_sInputName;
				rBox.getInputName(ui32Index, l_sInputName);
				rBox.setOutputName(ui32Index, OpenViBE::CString("Copy of '") + l_sInputName + OpenViBE::CString("'")); 
				return true;
			}

			virtual OpenViBE::boolean onInputAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				// Duplicate input as new output
				rBox.addOutput("Temporary name", OV_TypeId_EBMLStream);
				return true;
			}

			virtual OpenViBE::boolean onInputRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				rBox.removeOutput(ui32Index);
				return true;
			}

			virtual OpenViBE::boolean onInputTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				// Keep input and output types identical
				OpenViBE::CIdentifier l_oTypeIdentifier;
				rBox.getInputType(ui32Index, l_oTypeIdentifier);
				rBox.setOutputType(ui32Index, l_oTypeIdentifier);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};

		class CHelloWorldWithInputDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("HelloWorldWithInput"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Duplicates input to output and prints a message to the log for each input block"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Examples/Basic"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-copy"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_HelloWorldWithInput; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Examples::CHelloWorldWithInput(); }
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CHelloWorldWithInputListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addSetting("Message",			OV_TypeId_String, "Hello!");		// setting 0

				rPrototype.addInput ("Input 0",  OV_TypeId_Signal);
				rPrototype.addOutput("Copy of 'Input 0'", OV_TypeId_Signal);
				rPrototype.addFlag  (OpenViBE::Kernel::BoxFlag_CanAddInput);
				rPrototype.addFlag  (OpenViBE::Kernel::BoxFlag_CanModifyInput);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_HelloWorldWithInputDesc)
		};
	};
};

#endif // __SamplePlugin_CHelloWorldWithInput_H__
