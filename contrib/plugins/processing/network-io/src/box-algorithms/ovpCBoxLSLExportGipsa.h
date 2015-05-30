#ifndef __OpenViBEPlugins_BoxAlgorithm_LSLExportGipsa_H__
#define __OpenViBEPlugins_BoxAlgorithm_LSLExportGipsa_H__

#if defined TARGET_HAS_ThirdPartyLSL

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <vector>

#include "../ovpCInputChannel.h"
#include <lsl_cpp.h>

#define OVP_ClassId_BoxAlgorithm_CBoxAlgorithmLSLExportGipsa 	 	OpenViBE::CIdentifier(0x591D2E94, 0x221C23AD)
#define OVP_ClassId_BoxAlgorithm_CBoxAlgorithmLSLExportGipsaDesc 	OpenViBE::CIdentifier(0x22AF11F5, 0x58F2787D)

namespace OpenViBEPlugins
{
	namespace NetworkIO
	{
		class CBoxAlgorithmLSLExportGipsa : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			CBoxAlgorithmLSLExportGipsa();

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_CBoxAlgorithmLSLExportGipsa);

		protected:

			OpenViBE::int64 m_i64DecimationFactor;
			OpenViBE::uint64 m_ui64OutputSamplingFrequency;
			OpenViBE::CString m_sStreamName;
			OpenViBE::CString m_sStreamType;

			OpenViBEPlugins::SignalProcessing::CInputChannel     m_oCInputChannel1;

			lsl::stream_outlet* m_outlet;
			std::vector< std::pair<OpenViBE::float32,OpenViBE::uint64> > m_stims;//identifier,time
		};

		class CBoxAlgorithmLSLExportGipsaDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("LSL Export (Gipsa)"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Anton Andreev"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Gipsa-lab"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Streams signal outside OpenVibe using Lab Streaming Layer library"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("More on how to read the signal in your application: https://code.google.com/p/labstreaminglayer/"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Acquisition and network IO"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-connect"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_CBoxAlgorithmLSLExportGipsa; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::NetworkIO::CBoxAlgorithmLSLExportGipsa; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput  ("Input signal",  OV_TypeId_Signal); 
				rBoxAlgorithmPrototype.addInput  ("Input stimulations",  OV_TypeId_Stimulations); 
				rBoxAlgorithmPrototype.addSetting("Stream name", OV_TypeId_String, "OpenViBE Stream");
				rBoxAlgorithmPrototype.addSetting("Stream type", OV_TypeId_String, "EEG");
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_CBoxAlgorithmLSLExportGipsaDesc);
		};
	};
};

#endif

#endif // __OpenViBEPlugins_BoxAlgorithm_LSLExportGipsa_H__
