#ifndef __OpenViBEPlugins_FileIO_CGDFFileWriter_H__
#define __OpenViBEPlugins_FileIO_CGDFFileWriter_H__

#include "../ovp_defines.h"
#include "../ovp_gdf_helpers.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <system/ovCMemory.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <stack>

#include <cstdio>
#include <cerrno>

namespace OpenViBEPlugins
{
	namespace FileIO
	{

		/**
		* The plugin's main class
		*
		*/
		class CGDFFileWriter : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CGDFFileWriter(void);

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize();
			virtual OpenViBE::boolean uninitialize();
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process();

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_GDFFileWriter)

		public:
			void setChannelCount(const OpenViBE::uint32 ui32ChannelCount);
			void setChannelName(const OpenViBE::uint32 ui32ChannelIndex, const char* sChannelName);
			void setSampleCountPerBuffer(const OpenViBE::uint32 ui32SampleCountPerBuffer);
			void setSamplingRate(const OpenViBE::uint32 ui32SamplingFrequency);
			void setSampleBuffer(const OpenViBE::float64* pBuffer);

			void setExperimentInformation();

			void setStimulation(const OpenViBE::uint64 ui64StimulationIdentifier, const OpenViBE::uint64 ui64StimulationDate);

			void saveMatrixData();
			void saveEvents();

		public:
			std::ofstream m_oFile;
			OpenViBE::CString m_sFileName;

			OpenViBEToolkit::TSignalDecoder<CGDFFileWriter>* m_pSignalDecoder;
			OpenViBEToolkit::TExperimentInformationDecoder<CGDFFileWriter>* m_pExperimentInformationDecoder;
			OpenViBEToolkit::TStimulationDecoder<CGDFFileWriter>* m_pStimulationDecoder;

			//GDF structures
			GDF::CFixedGDF1Header m_oFixedHeader;
			//std::vector< GDF::CVariableGDF1HeaderPerChannel > m_vVariableHeader;
			GDF::CVariableGDF1Header  m_oVariableHeader;


			std::vector< std::vector< OpenViBE::float64 > > m_vSamples;
			std::vector< OpenViBE::int64 > m_vSampleCount;


			OpenViBE::uint32 m_ui32SamplesPerChannel;
			OpenViBE::uint64 m_ui64SamplingFrequency;

			std::vector<std::pair<OpenViBE::uint64, OpenViBE::uint64> > m_oEvents;

			OpenViBE::boolean m_bError;
			OpenViBE::float64 m_f64Precision; // because of GDF writing problem (no scaling)
		};

		/**
		* Plugin's description
		*/
		class CGDFFileWriterDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("GDF file writer"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Yann Renard"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("This algorithm records on disk what comes from a specific output"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("This algorithm dumps on disk a stream from a specific output in the standard GDF file format"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("File reading and writing/GDF"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.6"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-save"); }

			virtual void release(void)                                   { }
			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_GDFFileWriter; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::FileIO::CGDFFileWriter(); }

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				// Adds box inputs //swap order of the first two
				rPrototype.addInput("Experiment information", OV_TypeId_ExperimentInformation);
				rPrototype.addInput("Signal", OV_TypeId_Signal);
				rPrototype.addInput("Stimulation", OV_TypeId_Stimulations);

				// Adds box settings
				rPrototype.addSetting("Filename", OV_TypeId_Filename, "record-[$core{date}-$core{time}].gdf");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_GDFFileWriterDesc)
		};
	};
};

#endif // __OpenViBEPlugins_FileIO_CGDFFileWriter_H__
