
// @copyright notice: Possibly due to dependencies, this box used to be GPL before upgrade to AGPL3

#if defined TARGET_HAS_ThirdPartyITPP

#ifndef __OpenViBEPlugins_SignalProcessing_CSpectralAnalysis_H__
#define __OpenViBEPlugins_SignalProcessing_CSpectralAnalysis_H__

#include "../ovp_defines.h"

#include <toolkit/ovtk_all.h>

#include <vector>
#include <map>
#include <string>

#ifndef  CString2Boolean
	#define CString2Boolean(string) (strcmp(string,"true"))?0:1
#endif

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		/**
		* The Spectral Anlaysis plugin's main class.
		*/
		class CSpectralAnalysis : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CSpectralAnalysis(void);

			virtual void release(void);
			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_SpectralAnalysis)


		public:

			//start time and end time of the last arrived chunk
			OpenViBE::uint64 m_ui64LastChunkStartTime;
			OpenViBE::uint64 m_ui64LastChunkEndTime;

			//codecs
			OpenViBEToolkit::TSignalDecoder< CSpectralAnalysis > m_oSignalDecoder;
			OpenViBEToolkit::TSpectrumEncoder< CSpectralAnalysis > m_vSpectrumEncoder[4];

			///number of channels
			OpenViBE::uint32 m_ui32ChannelCount;
			OpenViBE::uint32 m_ui32SamplingRate;
			OpenViBE::uint32 m_ui32FrequencyBandCount;
			OpenViBE::uint32 m_ui32SampleCount;

			OpenViBE::uint32 m_ui32HalfFFTSize;			// m_ui32SampleCount / 2 + 1;

			OpenViBE::boolean m_bAmplitudeSpectrum;
			OpenViBE::boolean m_bPhaseSpectrum;
			OpenViBE::boolean m_bRealPartSpectrum;
			OpenViBE::boolean m_bImagPartSpectrum;
		};

		class CSpectralAnalysisDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Spectral analysis (FFT)"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Guillaume Gibert"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INSERM"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Compute spectral analysis using Fast Fourier Transform"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Spectral analysis"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_SpectralAnalysis; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CSpectralAnalysis(); }

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addInput("Input signal", OV_TypeId_Signal);

				rPrototype.addOutput("Amplitude", OV_TypeId_Spectrum);
				rPrototype.addOutput("Phase", OV_TypeId_Spectrum);
				rPrototype.addOutput("Real Part", OV_TypeId_Spectrum);
				rPrototype.addOutput("Imag Part", OV_TypeId_Spectrum);

				rPrototype.addSetting("Spectral components", OVP_TypeId_SpectralComponent, "Amplitude");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_SpectralAnalysisDesc)
		};
	}
}

#endif // __SamplePlugin_CSpectralAnalysis_H__

#endif // TARGET_HAS_ThirdPartyITPP

