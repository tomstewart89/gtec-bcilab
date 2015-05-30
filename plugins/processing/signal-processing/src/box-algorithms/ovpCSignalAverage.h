#ifndef __OpenViBEPlugins_SignalProcessing_CSignalAverage_H__
#define __OpenViBEPlugins_SignalProcessing_CSignalAverage_H__

#include "../ovp_defines.h"

#include <toolkit/ovtk_all.h>

#include <string>
#include <vector>

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		/**
		*/
		class CSignalAverage : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{

		public:

			CSignalAverage(void);

			virtual void release(void);

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);

			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_SignalAverage)

		public:

			virtual void computeAverage(void);

		public:

			// Needed to read the input and write the output
			OpenViBEToolkit::TSignalDecoder<CSignalAverage> m_oSignalDecoder;		
			OpenViBEToolkit::TSignalEncoder<CSignalAverage> m_oSignalEncoder;

		};

		/**
		* Description of the channel selection plugin
		*/
		class CSignalAverageDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Signal average"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Bruno Renier"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Computes the average of each input buffer."); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Averaging"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.5"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_SignalAverage; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CSignalAverage(); }

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addInput("Input signal", OV_TypeId_Signal);
				rPrototype.addOutput("Filtered signal", OV_TypeId_Signal);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_SignalAverageDesc)
		};
	}
}

#endif // __OpenViBEPlugins_SignalProcessing_CSignalAverage_H__

