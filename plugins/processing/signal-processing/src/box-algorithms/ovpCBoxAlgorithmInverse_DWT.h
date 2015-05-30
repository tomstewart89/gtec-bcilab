
#if defined(TARGET_HAS_ThirdPartyFFTW3) // required by wavelet2s

#ifndef __OpenViBEPlugins_BoxAlgorithm_Inverse_DWT_H__
#define __OpenViBEPlugins_BoxAlgorithm_Inverse_DWT_H__

//You may have to change this path to match your folder organisation
#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <string>
#include <sstream>

// The unique identifiers for the box and its descriptor.
// Identifier are randomly chosen by the skeleton-generator.
#define OVP_ClassId_BoxAlgorithm_Inverse_DWT OpenViBE::CIdentifier(0x5B5B8468, 0x212CF963)
#define OVP_ClassId_BoxAlgorithm_Inverse_DWTDesc OpenViBE::CIdentifier(0x01B9BC9A, 0x34766AE9)

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		/**
		 * \class CBoxAlgorithmInverse_DWT
		 * \author Joao-Pedro Berti-Ligabo / Inria
		 * \date Thu Jul 24 10:57:05 2014
		 * \brief The class CBoxAlgorithmInverse_DWT describes the box Inverse DWT.
		 *
		 */
		class CBoxAlgorithmInverse_DWT : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			CBoxAlgorithmInverse_DWT(void) : m_oAlgoX_SignalDecoder(NULL) { };

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
				

			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);

			
			virtual OpenViBE::boolean process(void);
			

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_Inverse_DWT);

		protected:
			// Codec algorithms specified in the skeleton-generator:
			// Signal stream encoder
			OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmInverse_DWT > m_oAlgo0_SignalEncoder;
            OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmInverse_DWT > m_AlgoInfo_SignalDecoder;
            OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmInverse_DWT >  *m_oAlgoX_SignalDecoder;

            OpenViBE::CString m_sWaveletType;
            OpenViBE::CString m_sDecompositionLevel;

		};


		

		class CBoxAlgorithmInverse_DWTListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
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
                OpenViBE::uint32 l_InputsCount = rBox.getInputCount();


                OpenViBE::CIdentifier l_LevelsIdentifier;
                OpenViBE::CString l_NumberDecompositionLevels;
                rBox.getSettingValue(1,l_NumberDecompositionLevels);

                OpenViBE::uint32 l_uintNbDecompositionLevels;

                l_uintNbDecompositionLevels = atoi(l_NumberDecompositionLevels);

                if (l_InputsCount!=l_uintNbDecompositionLevels+2)
                {
                    for (OpenViBE::uint32 i=0; i<l_InputsCount; i++)
                    {
                        rBox.removeInput(l_InputsCount-i-1);
                    }

                    rBox.addInput("Info",OV_TypeId_Signal);
                    rBox.addInput("A",OV_TypeId_Signal);
                    std::string l_stringLevel;
                    std::string l_stringLevelName;
                    for (OpenViBE::uint32 i=l_uintNbDecompositionLevels;i>0;i--)
                    {
                        std::ostringstream l_ostringstreamConvert;
                        l_ostringstreamConvert << i;
                        l_stringLevel = l_ostringstreamConvert.str();
                        l_stringLevelName = "D";
                        l_stringLevelName=l_stringLevelName + l_stringLevel;
                        rBox.addInput(l_stringLevelName.c_str(),OV_TypeId_Signal);
                    }
                }
                }

                return true;
            };

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};
		

		/**
		 * \class CBoxAlgorithmInverse_DWTDesc
		 * \author Joao-Pedro Berti-Ligabo / Inria
		 * \date Thu Jul 24 10:57:05 2014
		 * \brief Descriptor of the box Inverse DWT.
		 *
		 */
		class CBoxAlgorithmInverse_DWTDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Inverse DWT"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Joao-Pedro Berti-Ligabo"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Calculate Inverse DiscreteWaveletTransform"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Calculate Inverse DiscreteWaveletTransform using different types of wavelets"); }
            virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Wavelets"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gnome-fs-regular.png"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_Inverse_DWT; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CBoxAlgorithmInverse_DWT; }
			
			
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmInverse_DWTListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }
			
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
                rBoxAlgorithmPrototype.addInput("Info",OV_TypeId_Signal);
                rBoxAlgorithmPrototype.addInput("A",OV_TypeId_Signal);
                rBoxAlgorithmPrototype.addInput("D2",OV_TypeId_Signal);
                rBoxAlgorithmPrototype.addInput("D1",OV_TypeId_Signal);
				
                rBoxAlgorithmPrototype.addOutput("Signal",OV_TypeId_Signal);

                rBoxAlgorithmPrototype.addSetting("Wavelet type",OVP_TypeId_WaveletType,"");
                rBoxAlgorithmPrototype.addSetting("Wavelet decomposition levels",OVP_TypeId_WaveletLevel,"");
				
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_Inverse_DWTDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_Inverse_DWT_H__

#endif