
// @copyright notice: Possibly due to dependencies, this box used to be GPL before upgrade to AGPL3

#ifndef __OpenViBEPlugins_SignalProcessing_BoxAlgorithms_Filter_CModTemporalFilterBoxAlgorithm_H__
#define __OpenViBEPlugins_SignalProcessing_BoxAlgorithms_Filter_CModTemporalFilterBoxAlgorithm_H__

#include "../ovp_defines.h"

#include <toolkit/ovtk_all.h>

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CModTemporalFilterBoxAlgorithm : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_Box_ModTemporalFilterBoxAlgorithm)

		protected:

			//update the settings value from the UI
			//return true if any setting has changed
			bool updateSettings();

			//compute the filter coeff
			//return false if failed
			bool compute();

			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pStreamEncoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pComputeModTemporalFilterCoefficients;
			OpenViBE::Kernel::IAlgorithmProxy* m_pApplyModTemporalFilter;

			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* > ip_pMemoryBufferToDecode;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pEncodedMemoryBuffer;
			OpenViBE::uint64 m_ui64LastEndTime;

			//setting last value to avoid recompute if they haven't changed
			OpenViBE::CString m_sFilterMethod;
			OpenViBE::CString m_sFilterType;
			OpenViBE::CString m_sFilterOrder;
			OpenViBE::CString m_sLowBand;
			OpenViBE::CString m_sHighBand;
			OpenViBE::CString m_sPassBandRiple;
		};

		class CModTemporalFilterBoxAlgorithmDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Modifiable Temporal filter"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Guillaume Gibert / lmahe"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INSERM/U821 INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Applies temporal filtering on time signal, modifiable parameters"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("The user can choose among a variety of filter types to process the signal and change the settings online"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Filtering"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString(""); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Box_ModTemporalFilterBoxAlgorithm; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CModTemporalFilterBoxAlgorithm(); }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addInput  ("Input signal",            OV_TypeId_Signal);
				rPrototype.addOutput ("Filtered signal",         OV_TypeId_Signal);
				rPrototype.addSetting("Filter method",           OVP_TypeId_FilterMethod, OVP_TypeId_FilterMethod_Butterworth.toString(), true);
				rPrototype.addSetting("Filter type",             OVP_TypeId_FilterType,   OVP_TypeId_FilterType_BandPass.toString(), true);
				rPrototype.addSetting("Filter order",            OV_TypeId_Integer,       "4", true);
				rPrototype.addSetting("Low cut frequency (Hz)",  OV_TypeId_Float,         "29", true);
				rPrototype.addSetting("High cut frequency (Hz)", OV_TypeId_Float,         "40", true);
				rPrototype.addSetting("Pass band ripple (dB)",   OV_TypeId_Float,         "0.5", true);

				rPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_Box_ModTemporalFilterBoxAlgorithmDesc)
		};
	};
};

#endif // __OpenViBEPlugins_SignalProcessing_BoxAlgorithms_Filter_CModTemporalFilterBoxAlgorithm_H__
