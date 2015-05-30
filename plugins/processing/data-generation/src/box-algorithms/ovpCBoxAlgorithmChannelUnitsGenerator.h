#ifndef __OpenViBEPlugins_BoxAlgorithm_CChannelUnitsGenerator_H__
#define __OpenViBEPlugins_BoxAlgorithm_CChannelUnitsGenerator_H__

#include "../ovp_defines.h"
#include <toolkit/ovtk_all.h>
#include <cstdio>

#define OVP_ClassId_ChannelUnitsGenerator             OpenViBE::CIdentifier(0x42B09186, 0x582C8422)
#define OVP_ClassId_ChannelUnitsGeneratorDesc         OpenViBE::CIdentifier(0x4901A752, 0xD8578577)

namespace OpenViBEPlugins
{
	namespace DataGeneration
	{
		class CChannelUnitsGenerator : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			virtual void release(void);
			virtual OpenViBE::uint64 getClockFrequency(void) { return 1LL<<32; }
			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processClock(OpenViBE::Kernel::IMessageClock& /* rMessageClock */);
			virtual OpenViBE::boolean process(void);

			OpenViBE::boolean m_bHeaderSent;
			OpenViBE::uint64 m_ui64ChannelCount;
			OpenViBE::uint64 m_ui64Unit;
			OpenViBE::uint64 m_ui64Factor;

			OpenViBEToolkit::TChannelUnitsEncoder<CChannelUnitsGenerator> m_oEncoder;

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_ChannelUnitsGenerator)
		};

		class CChannelUnitsGeneratorListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};

		class CChannelUnitsGeneratorDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Channel units generator"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Jussi T. Lindgren"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Generates channel units"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("This box can generate a channel unit stream if specific measurement units are needed. The box is mainly provided for completeness."); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Data generation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-execute"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_ChannelUnitsGenerator; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::DataGeneration::CChannelUnitsGenerator(); }
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CChannelUnitsGeneratorListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addSetting("Number of channels",  OV_TypeId_Integer,         "4");
				rPrototype.addSetting("Unit",                OV_TypeId_MeasurementUnit, "V");
				rPrototype.addSetting("Factor",              OV_TypeId_Factor,          "1e-06");

				rPrototype.addOutput("Channel units",        OV_TypeId_ChannelUnits);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_ChannelUnitsGeneratorDesc)
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_CChannelUnitsGenerator_H__