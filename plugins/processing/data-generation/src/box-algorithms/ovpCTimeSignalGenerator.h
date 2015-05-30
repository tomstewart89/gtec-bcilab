#ifndef __SamplePlugin_CTimeSignalGenerator_H__
#define __SamplePlugin_CTimeSignalGenerator_H__

#include "../ovp_defines.h"

#include <toolkit/ovtk_all.h>

namespace OpenViBEPlugins
{
	namespace DataGeneration
	{
		class CTimeSignalGenerator : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CTimeSignalGenerator(void);

			virtual void release(void);

			virtual OpenViBE::uint64 getClockFrequency(void);

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processClock(OpenViBE::Kernel::IMessageClock& rMessageClock);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_TimeSignalGenerator)

		protected:

			OpenViBEToolkit::TSignalEncoder < CTimeSignalGenerator > m_oSignalEncoder;

			OpenViBE::boolean m_bHeaderSent;
			OpenViBE::uint32 m_ui32SamplingFrequency;
			OpenViBE::uint32 m_ui32GeneratedEpochSampleCount;
			OpenViBE::uint32 m_ui32SentSampleCount;
		};

		class CTimeSignalGeneratorDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Time signal"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Yann Renard"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Simple time signal generator (for use with DSP)"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Data generation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_TimeSignalGenerator; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::DataGeneration::CTimeSignalGenerator(); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-execute"); }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addOutput("Generated signal",              OV_TypeId_Signal);

				rPrototype.addSetting("Sampling frequency",           OV_TypeId_Integer, "512");
				rPrototype.addSetting("Generated epoch sample count", OV_TypeId_Integer, "32");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_TimeSignalGeneratorDesc)
		};
	};
};

#endif // __SamplePlugin_CTimeSignalGenerator_H__
