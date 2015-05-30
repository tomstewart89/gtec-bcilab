#ifndef __OpenViBEPlugins_FileIO_CGDFFileReader_H__
#define __OpenViBEPlugins_FileIO_CGDFFileReader_H__

#include "../ovp_defines.h"
#include "../ovp_gdf_helpers.h"

#include <openvibe/ov_all.h>

#include <toolkit/ovtk_all.h>

#include <system/ovCMemory.h>

#include <fstream>
#include <string>
#include <vector>

#include <iostream>
using namespace std;

#define GDFReader_ExperimentInfoOutput  0
#define GDFReader_SignalOutput          1
#define GDFReader_StimulationOutput     2

namespace OpenViBEPlugins
{
	namespace FileIO
	{
		/**
		* The GDF reader plugin main class
		*
		*/
		class CGDFFileReader : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:
			//Helper structures
			class CExperimentInfoHeader
			{
				public:
					OpenViBE::uint64 m_ui64ExperimentId;
					std::string m_sExperimentDate;

					OpenViBE::uint64 m_ui64SubjectId;
					std::string m_sSubjectName;
					OpenViBE::uint64 m_ui64SubjectAge;
					OpenViBE::uint64 m_ui64SubjectSex;

					OpenViBE::uint64 m_ui64LaboratoryId;
					std::string m_sLaboratoryName;
					OpenViBE::uint64 m_ui64TechnicianId;
					std::string m_sTechnicianName;

					bool m_bReadyToSend;
			};

			// Used to store information about the signal stream
			class CSignalDescription
			{
				public:
					CSignalDescription() : m_ui32StreamVersion(1), m_ui32SamplingRate(0), m_ui32ChannelCount (0), m_ui32SampleCount(0), m_ui32CurrentChannel(0), m_bReadyToSend(false)
					{
					}

				public:
					OpenViBE::uint32 m_ui32StreamVersion;
					OpenViBE::uint32 m_ui32SamplingRate;
					OpenViBE::uint32 m_ui32ChannelCount;
					OpenViBE::uint32 m_ui32SampleCount;
					std::vector<std::string> m_pChannelName;
					OpenViBE::uint32 m_ui32CurrentChannel;

					bool m_bReadyToSend;
			};

		public:

			CGDFFileReader(void);

			virtual void release(void);

			virtual OpenViBE::boolean initialize(void);

			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock& rMessageClock);

			virtual OpenViBE::boolean process(void);

			virtual OpenViBE::uint64 getClockFrequency(){ return m_ui64ClockFrequency; }

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_GDFFileReader)

		public:
			OpenViBE::boolean readFileHeader();
		public:

			OpenViBE::boolean m_bErrorOccurred;	//true if an error has occurred while reading the GDF file

			//The GDF filename and handle
			OpenViBE::CString m_sFileName;
			std::ifstream m_oFile;
			OpenViBE::uint64 m_ui64FileSize;
			OpenViBE::uint64 m_ui64Header3Length;

			OpenViBE::float32 m_f32FileVersion;

			OpenViBEToolkit::TSignalEncoder<CGDFFileReader>* m_pSignalEncoder;
			OpenViBEToolkit::TExperimentInformationEncoder<CGDFFileReader>* m_pExperimentInformationEncoder;
			OpenViBEToolkit::TStimulationEncoder<CGDFFileReader>* m_pStimulationEncoder;


			//Stream information
			OpenViBE::uint64 m_ui32SamplesPerBuffer;	//user defined

			//input
			OpenViBE::uint64 m_ui64NumberOfDataRecords;
			OpenViBE::float64 m_f64DurationOfDataRecord;
			OpenViBE::uint16 m_ui16NumberOfChannels;

			OpenViBE::uint32 m_ui32NumberOfSamplesPerRecord; // We only handle the files where it is the same for all the channels

			//info about channel's data type in data record
			OpenViBE::uint32 * m_pChannelType;
			OpenViBE::uint16 * m_pChannelDataSize;
			OpenViBE::float64 * m_pChannelScale;
			OpenViBE::float64 * m_pChannelTranslate;

			//Size of a data record
			OpenViBE::uint64 m_ui64DataRecordSize;

			//The current data record's data
			OpenViBE::uint8 * m_pDataRecordBuffer;

			//pointers to each channel's information in the current data record
			OpenViBE::uint8 ** m_pChannelDataInDataRecord;

			//Output Stream matrix
			OpenViBE::float64 * m_pMatrixBuffer;
			OpenViBE::uint64 m_ui64MatrixBufferSize;
			OpenViBE::boolean m_bMatricesSent;

			//Total number of samples sent up to now (used to compute start/end time)
			OpenViBE::uint32 m_ui32SentSampleCount;

			//indexes of current data record, channel, and sample
			OpenViBE::uint64 m_ui64CurrentDataRecord;
			OpenViBE::uint32 m_ui32CurrentSampleInDataRecord;

			//Events variables
			OpenViBE::uint8 m_ui8EventTableMode;		//mode of the event table
			OpenViBE::uint32 m_ui32NumberOfEvents;		//number of events in the event table
			OpenViBE::uint32 * m_pEventsPositionBuffer;	//pointer on the array of event's positions
			OpenViBE::uint16 * m_pEventsTypeBuffer;		//pointer on the array of event's types

			std::vector<GDF::CGDFEvent> m_oEvents;		//current stimulation block

			OpenViBE::uint32 m_ui32CurrentEvent;		//current event in event table
			OpenViBE::boolean m_bEventsSent;		//true if all the events have been sent

			OpenViBE::uint64 m_ui64StimulationPerBuffer;	//user defined

			//helper structures
			CExperimentInfoHeader * m_pExperimentInfoHeader;
			OpenViBE::boolean m_bExperimentInformationSent;

			CSignalDescription m_pSignalDescription;
			OpenViBE::boolean m_bSignalDescriptionSent;

			OpenViBE::uint64 m_ui64ClockFrequency;

			OpenViBE::boolean m_bTranslateByMinimum;


		private:

			void writeExperimentInformation();
			void writeSignalInformation();
			void writeEvents();

			OpenViBE::boolean initializeFile();

			template<class T> OpenViBE::float64 GDFTypeToFloat64(T val, OpenViBE::uint32 ui32Channel)
			{
				return m_pChannelScale[ui32Channel] * static_cast<OpenViBE::float64>(val) + m_pChannelTranslate[ui32Channel];
			}

			template<class T> void GDFTypeBufferToFloat64Buffer(OpenViBE::float64 * out, T * in, OpenViBE::uint64 inputBufferSize, OpenViBE::uint32 ui32Channel)
			{
				for(OpenViBE::uint64 i = 0 ; i<inputBufferSize ; i++)
				{
					out[i] = GDFTypeToFloat64<T>(in[i], ui32Channel);
				}
			}

			void GDFBufferToFloat64Buffer(OpenViBE::float64 * out, void * in, OpenViBE::uint64 inputBufferSize, OpenViBE::uint32 ui32Channel);
		};

		// template<> OpenViBE::float64 CGDFFileReader::GDFTypeToFloat64<OpenViBE::float32>(OpenViBE::float32 val, OpenViBE::uint32 ui32Channel);
		// template<> OpenViBE::float64 CGDFFileReader::GDFTypeToFloat64<OpenViBE::float64>(OpenViBE::float64 val, OpenViBE::uint32 ui32Channel);

		/**
		* Description of the GDF Reader plugin
		*/
		class CGDFFileReaderDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("GDF file reader"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Bruno Renier, Jussi T. Lindgren"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("GDF file reader"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Reads .GDF format files"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("File reading and writing/GDF"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.9"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-open"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_GDFFileReader; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::FileIO::CGDFFileReader(); }

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				// Adds box outputs
				rPrototype.addOutput("Experiment information", OV_TypeId_ExperimentInformation);
				rPrototype.addOutput("EEG stream", OV_TypeId_Signal);
				rPrototype.addOutput("Stimulations", OV_TypeId_Stimulations);

				// Adds settings
				rPrototype.addSetting("Filename", OV_TypeId_Filename, "");
				rPrototype.addSetting("Samples per buffer", OV_TypeId_Integer, "32");
				rPrototype.addSetting("Subtract physical minimum", OV_TypeId_Boolean, "False");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_GDFFileReaderDesc)
		};
	}
}

#endif
