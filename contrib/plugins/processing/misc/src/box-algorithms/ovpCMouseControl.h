#ifndef __OpenViBEPlugins_Tools_CMouseControl_H__
#define __OpenViBEPlugins_Tools_CMouseControl_H__

#include "../ovp_defines.h"

#include <toolkit/ovtk_all.h>

#include <vector>
#include <string>
#include <map>

#if defined TARGET_OS_Linux
	#include <X11/X.h>
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
#endif

namespace OpenViBEPlugins
{
	namespace Tools
	{
		class CMouseControl : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CMouseControl(void);

			virtual void release(void);
			virtual OpenViBE::boolean initialize();
			virtual OpenViBE::boolean uninitialize();
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process();

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_MouseControl)

		protected:

			//codec
			OpenViBEToolkit::TStreamedMatrixDecoder < CMouseControl >* m_pStreamedMatrixDecoder;

#if defined TARGET_OS_Linux
			::Display* m_pMainDisplay;
			::Window m_oRootWindow;
#endif
		};

		class CMouseControlDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Mouse Control"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Guillaume Gibert"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INSERM"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Mouse Control for Feedback"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Experimental box to move the mouse in x direction with respect to the input value. Only implemented on Linux."); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Tools"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }
			virtual void release(void)                                   { }
			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_MouseControl; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Tools::CMouseControl(); }

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addInput("Amplitude", OV_TypeId_StreamedMatrix);
				rPrototype.addFlag (OpenViBE::Kernel::BoxFlag_IsUnstable);

				rPrototype.addInputSupport(OV_TypeId_StreamedMatrix);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_MouseControlDesc)
		};
	};
};

#endif
