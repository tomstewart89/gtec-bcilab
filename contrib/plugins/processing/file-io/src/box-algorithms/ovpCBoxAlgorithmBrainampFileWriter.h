/* Project: Gipsa-lab plugins for OpenVibe
 * AUTHORS AND CONTRIBUTORS: Andreev A., Barachant A., Congedo M., Ionescu,Gelu,

 * This file is part of "Gipsa-lab plugins for OpenVibe".
 * You can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with Brain Invaders. If not, see http://www.gnu.org/licenses/.*/
 
#ifndef __OpenViBEPlugins_BoxAlgorithm_BrainampFileWriter_H__
#define __OpenViBEPlugins_BoxAlgorithm_BrainampFileWriter_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <cstdlib>
#include <cstdio>
#include <fstream>

#include <boost/date_time/posix_time/posix_time.hpp>

#define OVP_ClassId_BoxAlgorithm_BrainampFileWriter     OpenViBE::CIdentifier(0x0C7E0BDE, 0x4EC90F95)
#define OVP_ClassId_BoxAlgorithm_BrainampFileWriterDesc OpenViBE::CIdentifier(0x0A77142C, 0x316B6E47)

namespace OpenViBEPlugins
{
	namespace FileIO
	{
		class CBoxAlgorithmBrainampFileWriter : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			CBoxAlgorithmBrainampFileWriter(void);
			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			OpenViBE::boolean writeHeaderFile(void);
			std::string getShortName(std::string fullpath); //of a file
			std::string FormatTime(boost::posix_time::ptime now);
	        
	
			template <class type> bool	saveBuffer(const type myDummy)
			{
				std::vector<type> output(m_pMatrix->getBufferElementCount());

				if(output.size() != m_pMatrix->getBufferElementCount())
					return false;

				OpenViBE::uint32 l_uint32ChannelCount     = m_pMatrix->getDimensionSize(0);
				OpenViBE::uint32 l_uint32SamplesPerChunk  = m_pMatrix->getDimensionSize(1);
				OpenViBE::float64* input                  = m_pMatrix->getBuffer();

				for (OpenViBE::uint32 k=0; k < l_uint32ChannelCount; k++) 
				{
					for (OpenViBE::uint32 j=0; j < l_uint32SamplesPerChunk; j++)
					{
						OpenViBE::uint32 index = (k * l_uint32SamplesPerChunk) + j;
						output[j * l_uint32ChannelCount + k] = type(input[index]);
					}
				}
					
				m_oDataFile.write ((char*) &output[0], m_pMatrix->getBufferElementCount() * sizeof(type));

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_BrainampFileWriter);
	
		protected:

			enum EBinaryFormat
			{
				BinaryFormat_Integer16,
				BinaryFormat_UnsignedInteger16,
				BinaryFormat_Float32,
			};

			//input signal 1
			OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmBrainampFileWriter > * m_pStreamDecoder;
			OpenViBE::IMatrix* m_pMatrix;
			OpenViBE::uint64 m_ui64SamplingFrequency;
			
			//input stimulation 1 
			OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmBrainampFileWriter > * m_pStimulationDecoderTrigger;
			//OpenViBE::Kernel::TParameterHandler <const OpenViBE::IMemoryBuffer* > ip_pMemoryBufferToDecodeTrigger;
			//OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* > op_pStimulationSetTrigger;

			std::string m_sHeaderFilename;
			std::string m_sDataFilename;
			std::string m_sMarkerFilename;

			std::ofstream m_oHeaderFile;
			std::ofstream m_oDataFile;
			std::ofstream m_oMarkerFile;

			OpenViBE::uint32 m_ui32BinaryFormat;

			OpenViBE::uint32 m_uint32StimulationCounter;
		};

		class CBoxAlgorithmBrainampFileWriterListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			virtual OpenViBE::boolean onInputTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};

		class CBoxAlgorithmBrainampFileWriterDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Brainamp file writer"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Anton Andreev"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Gipsa-lab"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Writes signal in the Brainamp file format. Stimulations are outputed as OpenVibe interger codes."); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("You must select the location of the output header file .vhdr. The .eeg and .vmrk files will be created with the same name and in the same folder."); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("File reading and writing/Brainamp"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-save"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_BrainampFileWriter; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::FileIO::CBoxAlgorithmBrainampFileWriter; }
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmBrainampFileWriterListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput  ("Streamed matrix",  OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addInput  ("Input stimulation channel",	   OV_TypeId_Stimulations); 

				rBoxAlgorithmPrototype.addSetting("Header filename",         OV_TypeId_Filename, "record-[$core{date}-$core{time}].vhdr");
				rBoxAlgorithmPrototype.addSetting("Binary format",           OVP_TypeId_BinaryFormat, "INT_16");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_BrainampFileWriterDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_BrainampFileWriter_H__
