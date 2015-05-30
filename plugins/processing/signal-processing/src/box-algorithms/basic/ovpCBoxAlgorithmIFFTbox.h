#ifndef __OpenViBEPlugins_BoxAlgorithm_IFFTbox_H__
#define __OpenViBEPlugins_BoxAlgorithm_IFFTbox_H__

#if defined TARGET_HAS_ThirdPartyITPP

//You may have to change this path to match your folder organisation
#include "../../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

// The unique identifiers for the box and its descriptor.
// Identifier are randomly chosen by the skeleton-generator.
#define OVP_ClassId_BoxAlgorithm_IFFTbox OpenViBE::CIdentifier(0xD533E997, 0x4AFD2423)
#define OVP_ClassId_BoxAlgorithm_IFFTboxDesc OpenViBE::CIdentifier(0xD533E997, 0x4AFD2423)
#include <complex>

#include <itpp/itbase.h>

namespace OpenViBEPlugins
{
	namespace SignalProcessingBasic
	{
		/**
		 * \class CBoxAlgorithmIFFTbox
		 * \author Guillermo Andrade B. (INRIA)
		 * \date Fri Jan 20 15:35:05 2012
		 * \brief The class CBoxAlgorithmIFFTbox describes the box IFFT box.
		 *
		 */
		class CBoxAlgorithmIFFTbox : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
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
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_IFFTbox);

		protected:
			// Codec algorithms specified in the skeleton-generator:
			// Spectrum real and imaginary parts stream decoders
			OpenViBEToolkit::TSpectrumDecoder < CBoxAlgorithmIFFTbox > m_oAlgo0_SpectrumDecoder[2];
			// Signal stream encoder
			OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmIFFTbox > m_oAlgo1_SignalEncoder;
		private:
		std::vector<itpp::Vec<std::complex<double> > > m_frequencyBuffer;
		std::vector<itpp::Vec<double> > m_signalBuffer;
		OpenViBE::uint32 m_bufferStartTime;
		OpenViBE::uint32 m_bufferEndTime;
		OpenViBE::uint32 m_ui32SampleCount;
		OpenViBE::uint32 m_channelsNumber;
		};

		/**
		 * \class CBoxAlgorithmIFFTboxDesc
		 * \author Guillermo Andrade B. (INRIA)
		 * \date Fri Jan 20 15:35:05 2012
		 * \brief Descriptor of the box IFFT box.
		 *
		 */
		class CBoxAlgorithmIFFTboxDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("IFFT"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Guillermo Andrade B."); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Compute Inverse Fast Fourier Transformation"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Compute Inverse Fast Fourier Transformation (depends on ITPP/fftw)"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Spectral analysis"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.2"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-execute"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_IFFTbox; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessingBasic::CBoxAlgorithmIFFTbox; }
			
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("real part",OV_TypeId_Spectrum);
				rBoxAlgorithmPrototype.addInput("imaginary part",OV_TypeId_Spectrum);
				
				rBoxAlgorithmPrototype.addOutput("Signal output",OV_TypeId_Signal);
				
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_IFFTboxDesc);
		};
	};
};
#endif //TARGET_HAS_ThirdPartyITPP

#endif // __OpenViBEPlugins_BoxAlgorithm_IFFTbox_H__
