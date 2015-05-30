#ifndef __OpenViBEPlugins_BoxAlgorithm_CSVFileReader_H__
#define __OpenViBEPlugins_BoxAlgorithm_CSVFileReader_H__

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>

namespace OpenViBEPlugins
{
	namespace FileIO
	{
		class CBoxAlgorithmCSVFileReader : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			CBoxAlgorithmCSVFileReader(void);

			virtual void release(void) { delete this; }

			virtual OpenViBE::uint64 getClockFrequency(void);
			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock& rMessageClock);
			virtual OpenViBE::boolean process(void);

			OpenViBE::boolean process_streamedMatrix(void);
			OpenViBE::boolean process_stimulation(void);
			OpenViBE::boolean process_signal(void);
			OpenViBE::boolean process_channelLocalisation(void);
			OpenViBE::boolean process_featureVector(void);
			OpenViBE::boolean process_spectrum(void);
			OpenViBE::boolean convertVectorDataToMatrix(OpenViBE::IMatrix* matrix);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_CSVFileReader);


		protected:
			OpenViBE::boolean initializeFile();

			::FILE* m_pFile;
			std::string m_sSeparator;
			OpenViBE::boolean m_bDoNotUseFileTime;
			OpenViBE::CString m_sFilename;
			OpenViBE::boolean m_bUseCompression;

			OpenViBE::CIdentifier m_oTypeIdentifier;
			OpenViBE::uint32 m_ui32NbColumn;
			OpenViBE::uint64 m_ui64SamplingRate;
			OpenViBE::uint32 m_ui32SamplesPerBuffer;
			OpenViBE::uint32 m_ui32ChannelNumberPerBuffer;

			OpenViBE::boolean (OpenViBEPlugins::FileIO::CBoxAlgorithmCSVFileReader::*m_fpRealProcess)(void);

			OpenViBEToolkit::TEncoder < CBoxAlgorithmCSVFileReader >* m_pAlgorithmEncoder;

			OpenViBE::boolean m_bHeaderSent;
			std::vector<std::string> m_vLastLineSplit;
			std::vector<std::string> m_vHeaderFile;
			std::vector<std::vector<std::string> > m_vDataMatrix;

			OpenViBE::float64 m_f64NextTime;

			OpenViBE::uint64 m_ui64ChunkStartTime;
			OpenViBE::uint64 m_ui64ChunkEndTime;

			static const OpenViBE::uint32 m_ui32bufferLen = 16384; // Side-effect: a maximum allowed length for a line of a CSV file
		};

		class CBoxAlgorithmCSVFileReaderListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			virtual OpenViBE::boolean onOutputTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				OpenViBE::CIdentifier l_oTypeIdentifier;
				rBox.getOutputType(ui32Index, l_oTypeIdentifier);
				if(l_oTypeIdentifier == OV_TypeId_Spectrum)
				{
					rBox.setSettingName(3,"Unused parameter");
					rBox.setSettingValue(3,"0");
				}
				else if(l_oTypeIdentifier == OV_TypeId_ChannelLocalisation)
				{
					rBox.setSettingName(3,"Channels number");
					rBox.setSettingValue(3,"32");
				}
				else if(l_oTypeIdentifier == OV_TypeId_FeatureVector)
				{
					rBox.setSettingName(3,"Unused parameter");
					rBox.setSettingValue(3,"0");
				}
				else if(l_oTypeIdentifier == OV_TypeId_StreamedMatrix)
				{
					rBox.setSettingName(3,"Samples per buffer");
					rBox.setSettingValue(3,"32");
				}
				else if(l_oTypeIdentifier==OV_TypeId_Stimulations)
				{
					rBox.setSettingName(3,"Unused parameter");
					rBox.setSettingValue(3,"0");
				}
				else
				{
					this->getLogManager() << OpenViBE::Kernel::LogLevel_Error << "Unsupported stream type " << l_oTypeIdentifier << "\n";
					rBox.setOutputType(ui32Index, OV_TypeId_Signal);
					rBox.setSettingName(3,"Samples per buffer");
					rBox.setSettingValue(3,"32");
					return false;
				}
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier)
		};

		class CBoxAlgorithmCSVFileReaderDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("CSV File Reader"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Baptiste Payan"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Read signal in a CSV (text based) file"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("File reading and writing/CSV"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-open"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_CSVFileReader; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::FileIO::CBoxAlgorithmCSVFileReader; }
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmCSVFileReaderListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addOutput ("Output stream", OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addSetting("Filename", OV_TypeId_Filename, "");
				rBoxAlgorithmPrototype.addSetting("Column separator", OV_TypeId_String, ";");
				rBoxAlgorithmPrototype.addSetting("Don't use the file time",OV_TypeId_Boolean, "false");
				rBoxAlgorithmPrototype.addSetting("Samples per buffer", OV_TypeId_Integer,"32");

				rBoxAlgorithmPrototype.addFlag   (OpenViBE::Kernel::BoxFlag_CanModifyOutput);

				rBoxAlgorithmPrototype.addFlag   (OpenViBE::Kernel::BoxFlag_IsUnstable);

				rBoxAlgorithmPrototype.addOutputSupport(OV_TypeId_StreamedMatrix);
				rBoxAlgorithmPrototype.addOutputSupport(OV_TypeId_FeatureVector);
				rBoxAlgorithmPrototype.addOutputSupport(OV_TypeId_ChannelLocalisation);
				rBoxAlgorithmPrototype.addOutputSupport(OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addOutputSupport(OV_TypeId_Spectrum);
				rBoxAlgorithmPrototype.addOutputSupport(OV_TypeId_Stimulations);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_CSVFileReaderDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_CSVFileReader_H__
