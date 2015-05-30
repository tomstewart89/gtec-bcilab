#ifndef __OpenViBEPlugins_BoxAlgorithm_LSLExport_H__
#define __OpenViBEPlugins_BoxAlgorithm_LSLExport_H__

#ifdef TARGET_HAS_ThirdPartyLSL

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <lsl_cpp.h>


#include <ctime>
#include <iostream>
#include <string>

// The unique identifiers for the box and its descriptor.
#define OVP_ClassId_BoxAlgorithm_LSLExport     OpenViBE::CIdentifier(0x6F3467FF, 0x52794DA6)
#define OVP_ClassId_BoxAlgorithm_LSLExportDesc OpenViBE::CIdentifier(0x40C03C3F, 0x034A19C2)

namespace OpenViBEPlugins
{
	namespace NetworkIO
	{
		/**
		 * \class CBoxAlgorithmLSLExport
		 * \author Jussi T. Lindgren (Inria)
		 * \date Fri Jan 30 09:55:22 2015
		 * \brief The class CBoxAlgorithmLSLExport describes the box LSL Export.
		 *
		 */
		class CBoxAlgorithmLSLExport : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
				
			//Here is the different process callbacks possible
			// - On clock ticks :
			//virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock& rMessageClock);
			// - On new input received (the most common behaviour for signal processing) :
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			
			// If you want to use processClock, you must provide the clock frequency.
			//virtual OpenViBE::uint64 getClockFrequency(void);
			
			virtual OpenViBE::boolean process(void);

			// As we do with any class in openvibe, we use the macro below 
			// to associate this box to an unique identifier. 
			// The inheritance information is also made available, 
			// as we provide the superclass OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_LSLExport);

		protected:

			// Decoders
			OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmLSLExport > m_oStimulationDecoder;
			OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmLSLExport >      m_oSignalDecoder;

			lsl::stream_outlet* m_pSignalOutlet;
			lsl::stream_outlet* m_pStimulusOutlet;

			OpenViBE::float32* m_pSingleSampleBuffer;

			OpenViBE::CString m_sSignalStreamName;
			OpenViBE::CString m_sSignalStreamID;
			OpenViBE::CString m_sMarkerStreamName;
			OpenViBE::CString m_sMarkerStreamID;
		};

		class CBoxAlgorithmLSLExportListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
			public:

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);

		};

		/**
		 * \class CBoxAlgorithmLSLExportDesc
		 * \author Jussi T. Lindgren (Inria)
		 * \date Fri Jan 30 09:55:22 2015
		 * \brief Descriptor of the box LSL Export.
		 *
		 */
		class CBoxAlgorithmLSLExportDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("LSL Export"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Jussi T. Lindgren"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Send input stream out via LabStreamingLayer (LSL)"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("\n"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Acquisition and network IO"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-connect"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_LSLExport; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::NetworkIO::CBoxAlgorithmLSLExport; }
			
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmLSLExportListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("Input signal",       OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addInput("Input stimulations", OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addSetting("Signal stream",     OV_TypeId_String, "openvibeSignal");
				rBoxAlgorithmPrototype.addSetting("Marker stream",     OV_TypeId_String, "openvibeMarkers");

				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_LSLExportDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_LSLExport_H__

#endif // TARGET_HAS_Boost
