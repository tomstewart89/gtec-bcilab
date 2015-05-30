
// @copyright notice: Possibly due to dependencies, this box used to be GPL before upgrade to AGPL3

#ifndef __OpenViBEPlugins_SignalProcessing_CWindowingFunctions_H__
#define __OpenViBEPlugins_SignalProcessing_CWindowingFunctions_H__

#if defined TARGET_HAS_ThirdPartyITPP

#include "../ovp_defines.h"

#include <toolkit/ovtk_all.h>

#include <vector>
#include <map>
#include <string>

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		namespace WindowingFunctions
		{
			// Used to store information about the signal stream
			class CSignalDescription
			{
				public:

					CSignalDescription(void)
						:m_ui32StreamVersion(1)
						,m_ui32SamplingRate(0)
						,m_ui32ChannelCount(0)
						,m_ui32SampleCount(0)
						,m_ui32CurrentChannel(0)
						,m_bReadyToSend(false)
					{
					}

				public:

					OpenViBE::uint32 m_ui32StreamVersion;
					OpenViBE::uint32 m_ui32SamplingRate;
					OpenViBE::uint32 m_ui32ChannelCount;
					OpenViBE::uint32 m_ui32SampleCount;
					std::vector<std::string> m_pChannelName;
					OpenViBE::uint32 m_ui32CurrentChannel;

					OpenViBE::boolean m_bReadyToSend;
			};
		}

		/**
		* The Window Anlaysis plugin's main class.
		*/
		class CWindowingFunctions : virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CWindowingFunctions(void);

			virtual void release(void);

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);

			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_WindowingFunctions)

		public:

			void setSampleBuffer(const OpenViBE::float64* pBuffer);

		public:

			//start time and end time of the last arrived chunk
			OpenViBE::uint64 m_ui64LastChunkStartTime;
			OpenViBE::uint64 m_ui64LastChunkEndTime;
			OpenViBE::uint64 m_ui64SamplesPerBuffer;
			OpenViBE::uint64 m_ui64ChannelCount;

			// Needed to write on the plugin output
			OpenViBEToolkit::TSignalDecoder < CWindowingFunctions >* m_pSignalDecoder;
			OpenViBEToolkit::TSignalEncoder < CWindowingFunctions >* m_pSignalEncoder;

			//! Structure containing information about the signal stream
			WindowingFunctions::CSignalDescription * m_pSignalDescription;

			//! Size of the matrix buffer (output signal)
			OpenViBE::uint64 m_ui64MatrixBufferSize;
			//! Output signal's matrix buffer
			OpenViBE::float64* m_pMatrixBuffer;

			OpenViBE::uint64 m_ui64WindowMethod;
		};

		class CWindowingFunctionsDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Windowing functions"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Guillaume Gibert"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INSERM"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Apply a window to the signal buffer"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Windowing"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_WindowingFunctions; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CWindowingFunctions(); }

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addInput("Input signal", OV_TypeId_Signal);

				rPrototype.addOutput("Output signal", OV_TypeId_Signal);

				rPrototype.addSetting("Window method", OVP_TypeId_WindowMethod, "Hamming");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_WindowingFunctionsDesc)
		};
	}
}
#endif // TARGET_HAS_ThirdPartyITPP

#endif // __SamplePlugin_CWindowingFunctions_H__
