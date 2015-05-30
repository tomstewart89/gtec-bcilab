#ifndef __OpenViBEPlugins_BoxAlgorithm_OSCController_H__
#define __OpenViBEPlugins_BoxAlgorithm_OSCController_H__

#include "../../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "oscpkt.h"
#include "oscpkt_udp.h"

#define OVP_ClassId_BoxAlgorithm_OSCController OpenViBE::CIdentifier(0xC66F2F0C, 0x3BA5B424)
#define OVP_ClassId_BoxAlgorithm_OSCControllerDesc OpenViBE::CIdentifier(0xF7A35BD7, 0x6331C7D9)

namespace OpenViBEPlugins
{
	namespace NetworkIO
	{
		/**
		 * \class CBoxAlgorithmOSCController
		 * \author Ozan Caglayan (Galatasaray University)
		 * \date Thu May  8 20:57:24 2014
		 * \brief The class CBoxAlgorithmOSCController describes the box OSC Controller.
		 *
		 */
		class CBoxAlgorithmOSCController : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
				
			//Here is the different process callbacks possible
			// - On new input received (the most common behaviour for signal processing) :
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			
			virtual OpenViBE::boolean process(void);
			
			// As we do with any class in openvibe, we use the macro below 
			// to associate this box to an unique identifier. 
			// The inheritance information is also made available, 
			// as we provide the superclass OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_OSCController);

		private:

			// Decodes the stream
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamDecoder;

			// UDP Socket (oscpkt_udp.h)
			oscpkt::UdpSocket m_oUdpSocket;
			
			// OSC Address to some device
			OpenViBE::CString m_sOSCAddress;
		};

		/**
		 * \class CBoxAlgorithmOSCControllerDesc
		 * \author Ozan Caglayan (Galatasaray University)
		 * \date Thu May  8 20:57:24 2014
		 * \brief Descriptor of the box OSC Controller.
		 *
		 */
		class CBoxAlgorithmOSCControllerDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("OSC Controller"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Ozan Caglayan"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Galatasaray University"); }
			// + Stimulation support & some code refactoring in v1.1 by Jussi T. Lindgren / Inria
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Sends OSC messages to an OSC controller"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("This box allows OpenViBE to send OSC (Open Sound Control) messages to an OSC server. See http://www.opensoundcontrol.org to learn about the OSC protocol and its use cases."); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Acquisition and network IO"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.1"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-network"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_OSCController; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::NetworkIO::CBoxAlgorithmOSCController; }
			
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{

				rBoxAlgorithmPrototype.addInput("Input",OV_TypeId_Signal);

				rBoxAlgorithmPrototype.addInputSupport(OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addInputSupport(OV_TypeId_StreamedMatrix);
				rBoxAlgorithmPrototype.addInputSupport(OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyInput);

				rBoxAlgorithmPrototype.addSetting("OSC Server IP",OV_TypeId_String,"127.0.0.1");
				rBoxAlgorithmPrototype.addSetting("OSC Server Port",OV_TypeId_Integer,"9001");
				rBoxAlgorithmPrototype.addSetting("OSC Address",OV_TypeId_String,"/a/b/c");

				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_OSCControllerDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_OSCController_H__
