
#if defined(TARGET_HAS_ThirdPartyFFTW3) // required by wavelet2s

#ifndef __OpenViBEPlugins_BoxAlgorithm_DiscreteWaveletTransform_H__
#define __OpenViBEPlugins_BoxAlgorithm_DiscreteWaveletTransform_H__

//You may have to change this path to match your folder organisation
#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>
#include <string>
#include <sstream>

// The unique identifiers for the box and its descriptor.
// Identifier are randomly chosen by the skeleton-generator.
#define OVP_ClassId_BoxAlgorithm_DiscreteWaveletTransform OpenViBE::CIdentifier(0x824194C5, 0x46D7FDE9)
#define OVP_ClassId_BoxAlgorithm_DiscreteWaveletTransformDesc OpenViBE::CIdentifier(0x6744711B, 0xF21B59EC)

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		/**
		 * \class CBoxAlgorithmDiscreteWaveletTransform
		 * \author Joao-Pedro Berti-Ligabo / Inria
		 * \date Wed Jul 16 15:05:16 2014
		 * \brief The class CBoxAlgorithmDiscreteWaveletTransform describes the box DiscreteWaveletTransform.
		 *
		 */
		class CBoxAlgorithmDiscreteWaveletTransform : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
				

			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);

			
			virtual OpenViBE::boolean process(void);
			

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_DiscreteWaveletTransform);

		protected:
			// Codec algorithms specified in the skeleton-generator:
			// Signal stream decoder
			OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmDiscreteWaveletTransform > m_oAlgo0_SignalDecoder;

			OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmDiscreteWaveletTransform > m_oAlgoInfo_SignalEncoder;
			std::vector< OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmDiscreteWaveletTransform >* > m_vAlgoX_SignalEncoder;

			OpenViBE::CString m_sWaveletType;
			OpenViBE::CString m_sDecompositionLevel;

			OpenViBE::uint32 m_ui32Infolength;
			std::vector< std::vector<double> > m_sig;

		};



		// The box listener can be used to call specific callbacks whenever the box structure changes : input added, name changed, etc.
		// Please uncomment below the callbacks you want to use.
		class CBoxAlgorithmDiscreteWaveletTransformListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			virtual OpenViBE::boolean onSettingValueChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{

				if (ui32Index==0)
				{
					return true;
				}

				if (ui32Index==1)
				{
					OpenViBE::uint32 l_ui32OutputsCount = rBox.getOutputCount();

					OpenViBE::CIdentifier l_oLevelsIdentifier;
					OpenViBE::CString l_sNumberDecompositionLevels;

					rBox.getSettingValue(1,l_sNumberDecompositionLevels);

					OpenViBE::uint32 l_ui32NbDecompositionLevels;

					l_ui32NbDecompositionLevels = atoi(l_sNumberDecompositionLevels);

					if (l_ui32OutputsCount!=l_ui32NbDecompositionLevels+2)
					{
						for (OpenViBE::uint32 i=0; i<l_ui32OutputsCount; i++)
						{
							rBox.removeOutput(l_ui32OutputsCount-i-1);
						}

						rBox.addOutput("Info",OV_TypeId_Signal);
						rBox.addOutput("A",OV_TypeId_Signal);
						std::string l_sLevel;
						std::string l_sLevelName;
						for (OpenViBE::uint32 i=l_ui32NbDecompositionLevels;i>0;i--)
						{
							std::ostringstream l_ostringstreamConvert;
							l_ostringstreamConvert << i;
							l_sLevel = l_ostringstreamConvert.str();
							l_sLevelName = "D";
							l_sLevelName=l_sLevelName + l_sLevel;
							rBox.addOutput(l_sLevelName.c_str(),OV_TypeId_Signal);

						}
					}
				}

				return true;
			};

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};


		/**
		 * \class CBoxAlgorithmDiscreteWaveletTransformDesc
		 * \author Joao-Pedro Berti-Ligabo / Inria
		 * \date Wed Jul 16 15:05:16 2014
		 * \brief Descriptor of the box DiscreteWaveletTransform.
		 *
		 */
		class CBoxAlgorithmDiscreteWaveletTransformDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Discrete Wavelet Transform"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Joao-Pedro Berti-Ligabo"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Calculate DiscreteWaveletTransform"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Calculate DiscreteWaveletTransform using different types of wavelets"); }
            virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Wavelets"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gnome-fs-regular.png"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_DiscreteWaveletTransform; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CBoxAlgorithmDiscreteWaveletTransform; }
			

			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmDiscreteWaveletTransformListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("Signal",OV_TypeId_Signal);

                rBoxAlgorithmPrototype.addOutput("Info",OV_TypeId_Signal);
                rBoxAlgorithmPrototype.addOutput("A",OV_TypeId_Signal);
                rBoxAlgorithmPrototype.addOutput("D2",OV_TypeId_Signal);
                rBoxAlgorithmPrototype.addOutput("D1",OV_TypeId_Signal);

                rBoxAlgorithmPrototype.addSetting("Wavelet type",OVP_TypeId_WaveletType,"");
                rBoxAlgorithmPrototype.addSetting("Wavelet decomposition levels",OVP_TypeId_WaveletLevel,"");
				
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_DiscreteWaveletTransformDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_DiscreteWaveletTransform_H__

#endif