#ifndef __OpenViBEPlugins_BoxAlgorithm_SharedMemoryWriter_H__
#define __OpenViBEPlugins_BoxAlgorithm_SharedMemoryWriter_H__

//You may have to change this path to match your folder organisation
#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

//#include <pair>
#include <vector>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/containers/string.hpp>

using namespace boost::interprocess;

// The unique identifiers for the box and its descriptor.
// Identifier are randomly chosen by the skeleton-generator.

namespace OpenViBEPlugins
{
	namespace FileReadingAndWriting
	{
		/**
		 * \class CBoxAlgorithmSharedMemoryWriter
		 * \author Dieter Devlaminck (INRIA)
		 * \date Thu Jan 17 13:34:58 2013
		 * \brief The class CBoxAlgorithmSharedMemoryWriter describes the box SharedMemoryWriter.
		 *
		 */
		
		struct SMatrix
		{
			OpenViBE::uint32 rowDimension;
			OpenViBE::uint32 columnDimension;
			offset_ptr<OpenViBE::float64> data;
		};

		typedef allocator<char, managed_shared_memory::segment_manager> CharAllocator;
		typedef basic_string<char, std::char_traits<char>, CharAllocator> ShmString;
		typedef allocator<ShmString, managed_shared_memory::segment_manager> StringAllocator;      
		//typedef vector<ShmString, StringAllocator> MyShmStringVector;		
		typedef allocator< std::pair<const ShmString, OpenViBE::CIdentifier > , managed_shared_memory::segment_manager>  ShmemAllocatorMetaInfo;
		typedef map<ShmString, OpenViBE::CIdentifier, std::less<ShmString>, ShmemAllocatorMetaInfo> MyVectorMetaInfo;
		
		typedef allocator<OpenViBE::uint32, managed_shared_memory::segment_manager>  ShmemAllocatorStimulation;
		typedef allocator<offset_ptr<SMatrix>, managed_shared_memory::segment_manager>  ShmemAllocatorMatrix;
		typedef vector<OpenViBE::uint32, ShmemAllocatorStimulation> MyVectorStimulation;
		typedef vector<offset_ptr<SMatrix>, ShmemAllocatorMatrix> MyVectorStreamedMatrix;	

		class CBoxAlgorithmSharedMemoryWriter : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
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
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_SharedMemoryWriter);

		protected:
			// Codec algorithms specified in the skeleton-generator:
			// Stimulation stream decoder
			//OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmSharedMemoryWriter > m_oAlgo0_StimulationDecoder;
			//OpenViBEToolkit::TStreamedMatrixDecoder < CBoxAlgorithmSharedMemoryWriter > m_oAlgo0_StreamedMatrixDecoder;
			std::vector<OpenViBEToolkit::TDecoder < CBoxAlgorithmSharedMemoryWriter >* > m_vDecoder;


			OpenViBE::CString m_sSharedMemoryName;
			managed_shared_memory m_oSharedMemoryArray;
			//OpenViBE::uint32 * m_pInputStimuliSet;
			//MyVector * m_pInputStimuliSet;
			//OpenViBE::uint32 m_ui32InputCounter;


		private:
			OpenViBE::CIdentifier m_TypeIdentifier;

			OpenViBE::CString m_sMutexName;
			named_mutex* m_oMutex;
			std::vector<MyVectorStimulation *> m_vStimuliSet;
			std::vector<MyVectorStreamedMatrix *> m_vStreamedMatrix;
		};


		// If you need to implement a box Listener, here is a sekeleton for you.
		// Use only the callbacks you need.
		// For example, if your box has a variable number of input, but all of them must be stimulation inputs.
		// The following listener callback will ensure that any newly added input is stimulations :
		/*		
		virtual OpenViBE::boolean onInputAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
		{
			rBox.setInputType(ui32Index, OV_TypeId_Stimulations);
		};
		*/
		
		
		// The box listener can be used to call specific callbacks whenever the box structure changes : input added, name changed, etc.
		// Please uncomment below the callbacks you want to use.
		class CBoxAlgorithmSharedMemoryWriterListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			//virtual OpenViBE::boolean onInitialized(OpenViBE::Kernel::IBox& rBox) { return true; };
			//virtual OpenViBE::boolean onNameChanged(OpenViBE::Kernel::IBox& rBox) { return true; };
			//virtual OpenViBE::boolean onInputConnected(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onInputDisconnected(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			virtual OpenViBE::boolean onInputAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			virtual OpenViBE::boolean onInputRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			virtual OpenViBE::boolean onInputTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) 
			{ 
			
				return true; 
			};
			//virtual OpenViBE::boolean onInputNameChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputConnected(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputDisconnected(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputNameChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingNameChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingDefaultValueChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingValueChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};
		

		/**
		 * \class CBoxAlgorithmSharedMemoryWriterDesc
		 * \author Dieter Devlaminck (INRIA)
		 * \date Thu Jan 17 13:34:58 2013
		 * \brief Descriptor of the box SharedMemoryWriter.
		 *
		 */
		class CBoxAlgorithmSharedMemoryWriterDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("SharedMemoryWriter"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Dieter Devlaminck"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Stream input to shared memory"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("The box writes input to shared memory so that it can be read by another process. Stimuli and streamed matrices are supported, and transformed into a format that can be written into shared memory. Based on the input types, a metainfo variable will be created in shared memory that will specify which variables have which type. This way the client can know what it will be reading."); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("File reading and writing"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString(""); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_SharedMemoryWriter; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::FileReadingAndWriting::CBoxAlgorithmSharedMemoryWriter; }
			
			
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmSharedMemoryWriterListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }
			
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("prediction1",OV_TypeId_StreamedMatrix);
				//rBoxAlgorithmPrototype.addInput("prediction2",OV_TypeId_Stimulations);


				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyInput);
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddInput);
				//rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanRemoveInput);
				
				rBoxAlgorithmPrototype.addSetting("SharedMemoryName",OV_TypeId_String,"SharedMemory_P300Stimulator");
				
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				
				rBoxAlgorithmPrototype.addInputSupport(OV_TypeId_StreamedMatrix);
				rBoxAlgorithmPrototype.addInputSupport(OV_TypeId_Stimulations);

				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_SharedMemoryWriterDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_SharedMemoryWriter_H__
