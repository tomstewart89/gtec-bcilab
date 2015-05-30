#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovCoAdaptP300SharedMemoryReader__
#define __ovCoAdaptP300SharedMemoryReader__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

#include "iocommunication/ovexISharedMemoryReader.h"

using namespace boost::interprocess;

namespace OpenViBEApplications
{
	typedef allocator<OpenViBE::uint32, managed_shared_memory::segment_manager>  ShmemAllocator;
	typedef allocator<OpenViBE::uint32, managed_shared_memory::segment_manager>  ShmemAllocatorStimulation;
	typedef allocator<OpenViBE::CMatrix, managed_shared_memory::segment_manager>  ShmemAllocatorMatrix;
	typedef vector<OpenViBE::uint32, ShmemAllocator> MyVector;
	typedef vector<OpenViBE::uint32, ShmemAllocatorStimulation> MyVectorStimulation;
	typedef vector<OpenViBE::CMatrix, ShmemAllocatorMatrix> MyVectorStreamedMatrix;

	/**
	 * This class uses the ISharedMemoryReader 'library'/interface to read predictions and letter probabilities
	 * as computed by the P300 scenario from the shared memory
	 */
	class CoAdaptP300SharedMemoryReader
	{
		public:
			/**
			 * Simply constructs a SharedVariableHandler to provide access to all the variables in shared memory\n
			 * It is assumed that the first variable will be the stimulus (corresponding to the letter that is predicted by the P300 scenario)
			 * and the second variable could be the matrix with letter probabilities.\n
			 * So the shared memory writer box in the scenario should have at least a stimulation input and optionally a secondary streamed matrix input.
			 */
			void openSharedMemory(OpenViBE::CString sharedMemoryName);
			
			/**
			 * returns the index of the letter predicted, starting from 1 corresponding to the first letter specified in the keyboard layout xml file
			 * @return the index of the letter predicted
			 */
			OpenViBE::uint64 readNextPrediction();

			/**
			 * returns the stimulations currently in the shared memory and erase them
			 * @return StimulationSet with all the stimulation currently in the shared memory
			 */
			OpenViBE::IStimulationSet* readStimulation();
			
			/** 
			 * returns the matrix with probabilities for all symbols in the keyboard, in order of appearance in the keyboard layout xml file 
			 * @return the matrix with probabilities for all symbols in the keyboard
			 */
			OpenViBE::IMatrix* readNextSymbolProbabilities();
			
			/**
			 * clears the vector with the symbol probabilities
			 */
			void clearSymbolProbabilities();
			
			/**
			 * deletes the SharedVariableHandler. This has not much effect as it is the creating application that has to clean up the shared memory
			 */
			void closeSharedMemory();

		protected:

			/**
			 * The shared variable handler that has a reader of appropriate type for each variable in shared memory
			 */
			ISharedMemoryReader::SharedVariableHandler* m_pSharedVariableHandler;
	};
};
#endif

#endif
