
#if defined(TARGET_HAS_ThirdPartyEIGEN)

#ifndef __OpenViBEPlugins_BoxAlgorithm_EOG_Denoising_Calibration_H__
#define __OpenViBEPlugins_BoxAlgorithm_EOG_Denoising_Calibration_H__

//You may have to change this path to match your folder organisation
#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <iostream>
#include <fstream>

// The unique identifiers for the box and its descriptor.
// Identifier are randomly chosen by the skeleton-generator.
#define OVP_ClassId_BoxAlgorithm_EOG_Denoising_Calibration OpenViBE::CIdentifier(0xE8DFE002, 0x70389932)
#define OVP_ClassId_BoxAlgorithm_EOG_Denoising_CalibrationDesc OpenViBE::CIdentifier(0xF4D74831, 0x88B80DCF)

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		/**
		 * \class CBoxAlgorithmEOG_Denoising_Calibration
		 * \author Joao-Pedro Berti-Ligabo / Inria
		 * \date Fri May 23 15:30:58 2014
		 * \brief The class CBoxAlgorithmEOG_Denoising_Calibration describes the box EOG_Denoising_Calibration.
		 *
		 */
		class CBoxAlgorithmEOG_Denoising_Calibration : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
				
			//Here is the different process callbacks possible
			// - On clock ticks :
            virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock& rMessageClock);
			// - On new input received (the most common behaviour for signal processing) :
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			// - On message received :
            //virtual OpenViBE::boolean processMessage(const OpenViBE::Kernel::IMessageWithData& msg, OpenViBE::uint32 inputIndex);
			
			// If you want to use processClock, you must provide the clock frequency.
            virtual OpenViBE::uint64 getClockFrequency(void);
			
			virtual OpenViBE::boolean process(void);

            //virtual OpenViBE::boolean openfile();
			
			// As we do with any class in openvibe, we use the macro below 
			// to associate this box to an unique identifier. 
			// The inheritance information is also made available, 
			// as we provide the superclass OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_EOG_Denoising_Calibration);

		protected:
			// Codec algorithms specified in the skeleton-generator:
			// Signal stream decoder
			OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmEOG_Denoising_Calibration > m_oAlgo0_SignalDecoder;
			// Signal stream decoder
			OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmEOG_Denoising_Calibration > m_oAlgo1_SignalDecoder;

       //     OpenViBE::Kernel::IAlgorithmProxy* m_pMatrixRegressionAlgorithmCalibration;
       //     OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > ip_pMatrixRegressionAlgorithmCalibration_Matrix0;
       //     OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > ip_pMatrixRegressionAlgorithmCalibration_Matrix1;
            OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmEOG_Denoising_Calibration > m_oAlgo2_StimulationDecoder;

            OpenViBEToolkit::TStimulationEncoder < CBoxAlgorithmEOG_Denoising_Calibration > m_oStimulationEncoder;

            OpenViBE::CString m_sRegressionDenoisingCalibrationFilename;

            OpenViBE::uint32 m_ui32ChunksVerify;
            OpenViBE::uint32 m_ui32ChunksCount;
            OpenViBE::boolean m_bEndProcess;
            std::fstream m_oEEGFile;
            std::fstream m_oEOGFile;
            std::ofstream m_oMatrixFile;

            OpenViBE::float64 m_f64StartTime;
            OpenViBE::float64 m_f64EndTime;

            OpenViBE::uint32 m_ui32StartTimeChunks;
            OpenViBE::uint32 m_ui32EndTimeChunks;

            OpenViBE::uint64 m_ui64TrainDate;
            OpenViBE::uint64 m_ui64TrainChunkStartTime;
            OpenViBE::uint64 m_ui64TrainChunkEndTime;

            OpenViBE::float64 m_f64Time;

            OpenViBE::uint32 m_ui32NbChannels0;
            OpenViBE::uint32 m_ui32NbChannels1;

            OpenViBE::uint32 m_ui32NbSamples0;
            OpenViBE::uint32 m_ui32NbSamples1;

            OpenViBE::uint64 m_ui64StimulationIdentifier;

			OpenViBE::CString m_sEEGTempFilename;
			OpenViBE::CString m_sEOGTempFilename;

		};

		/**
		 * \class CBoxAlgorithmEOG_Denoising_CalibrationDesc
		 * \author Joao-Pedro Berti-Ligabo / Inria
		 * \date Fri May 23 15:30:58 2014
		 * \brief Descriptor of the box EOG_Denoising_Calibration.
		 *
		 */
		class CBoxAlgorithmEOG_Denoising_CalibrationDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("EOG Denoising Calibration"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Joao-Pedro Berti-Ligabo"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
            virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Algorithm implementation as suggested in Schlogl's article of 2007."); }
            virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Press 'a' to set start point and 'u' to set end point, you can connect the Keyboard Stimulator for that"); }
            virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Denoising"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gnome-fs-regular.png"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_EOG_Denoising_Calibration; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CBoxAlgorithmEOG_Denoising_Calibration; }
			
			/*
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmEOG_Denoising_CalibrationListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }
			*/
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{

				rBoxAlgorithmPrototype.addInput("EEG",OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addInput("EOG",OV_TypeId_Signal);
                rBoxAlgorithmPrototype.addInput  ("Stimulations", OV_TypeId_Stimulations);

                rBoxAlgorithmPrototype.addSetting("Filename b Matrix", OV_TypeId_Filename, "b-Matrix-EEG.cfg");

//                rBoxAlgorithmPrototype.addSetting("Start time (s)", OV_TypeId_Float, "3");
//                rBoxAlgorithmPrototype.addSetting("End time (s)", OV_TypeId_Float, "40");

                rBoxAlgorithmPrototype.addSetting("End trigger", OV_TypeId_Stimulation, "OVTK_GDF_End_Of_Session");

                rBoxAlgorithmPrototype.addOutput ("Train-completed Flag",OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_EOG_Denoising_CalibrationDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_EOG_Denoising_Calibration_H__

#endif