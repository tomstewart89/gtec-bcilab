#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovexISharedMemoryReader__
#define __ovexISharedMemoryReader__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

using namespace boost::interprocess;

namespace OpenViBEApplications
{
	/**
	* An interface for reading the shared memory as written by the SharedMemoryWriterBox in the OpenViBE designer.
	* The interface should be implemented for each of the OpenViBE streaming types, such as stimulations and matrices (SharedStimulusReader and SharedMatrixReader).\n
	* Each reader object will correspond to a vector variable in shared memory that either contains stimulations or matrices.
	* As there can be a multitude of these vectors in shared memory we have a handler SharedVariableHandler that creates readers for each such vector variable in shared memory.\n
	* Note: we assume it is the writer, which constructs the shared memory, that should clean and destroy the variables and the shared memory.
	*/	
	class ISharedMemoryReader
	{
	public:
		
		/**
		* This nested class will handle/create the appropriate readers (implementations of ISharedMemoryReader) for all variables in the shared memory sharedMemoryName. 
		* The variables can either be vectors of stimulations or vectors of streamed matrices. 
		* In shared memory there will be a variable MetaInfo that indicates the name of the variables (so we can access it) and what the type of each variable is.
		* The values are accessed by calling front or pop_front on the correct index. 
		* The index identifies the reader corresponding to that variable in shared memory.
		* So if the SharedMemoryWriterBox in the designer has two inputs, the first for stimulations, the second for streamed matrices, then
		* the stimuli can be accessed by calling front(0) and the streamed matrices by calling front(1)
		*/			
		class SharedVariableHandler
		{
		private:
			std::vector<ISharedMemoryReader*>* m_vSharedMemoryVariables;
			OpenViBE::boolean m_bFailedToOpen;
			
		public:
			/**
			 * Based on the variable MetaInfo in shared memory with name sharedMemoryName approriate readers (either for stimuli or for streamed matrices)
			 * will be created and stored in m_vSharedMemoryVariables. Each of these readers will map a vector in shared memory of its corresponding type.
			 * @param sharedMemoryName name of the shared memory as indicated in the SharedMemoryWriterBox in the openvibe designer
			 */
			SharedVariableHandler(OpenViBE::CString sharedMemoryName);
			
			/**
			 * Destructor
			 */			
			~SharedVariableHandler();
			
			/**
			 * returns the first element of the vector at index inputIndex
			 * @param inputIndex
			 * @return OpenViBE StimulationSet or StreamedMatrix depending on the type of the variable at index inputIndex
			 */			
			OpenViBE::IObject* front(OpenViBE::uint32 inputIndex);
			
			/**
			 * returns the first element of the vector at index inputIndex and deletes it
			 * @param inputIndex
			 * @return OpenViBE's IStimulationSet or IMatrix depending on the type of the variable at index inputIndex
			 */			
			OpenViBE::IObject* pop_front(OpenViBE::uint32 inputIndex);
			
			/**
			 * clears the vector variable at index inputIndex
			 * @param inputIndex
			 */			
			void clear(OpenViBE::uint32 inputIndex);
		};		
		
		/**
		* 
		*/	
		ISharedMemoryReader();
		
		/**
		 * Constructs a reader that will provide access to the variable sharedVariableName in shared memory sharedMemoryName
		 * @param sharedMemoryName name of the shared memory
		 * @param sharedVariableName name of the variable in shared memory we want to access
		*/	
		ISharedMemoryReader(OpenViBE::CString sharedMemoryName, OpenViBE::CString sharedVariableName);
		
		/**
		* Destructor deletes the mutex
		*/	
		virtual ~ISharedMemoryReader();

	protected:

		/**
		* Finds and maps the shared variable specified by m_sSharedVariableName in the shared memory with name m_sSharedMemoryName
		*/			
		virtual OpenViBE::boolean open() = 0;
		
		/**
		* Finds and maps the shared variable specified by sharedVariableName in the shared memory with name sharedMemoryName
		* @param sharedMemoryName name of the shared memory
		* @param sharedVariableName name of the variable in shared memory we want to access
		*/			
		virtual OpenViBE::boolean open(OpenViBE::CString sharedMemoryName, OpenViBE::CString sharedVariableName) = 0;
		
		/**
		* Returns the first element of its corresponding vector variable that resides in shared memory
		* @return OpenViBE's IStimulationSet or IMatrix depending on the implementation of the interface
		*/			
		virtual OpenViBE::IObject* front() = 0;
		
		/**
		* Returns the first element of its corresponding vector variable that resides in shared memory and deletes it
		* @return OpenViBE's IStimulationSet or IMatrix depending on the implementation of the interface
		*/				
		virtual OpenViBE::IObject* pop_front() = 0;
		
		/**
		* clears the vector in shared memory
		*/			
		virtual void clear() = 0;
		
		/**
		* clears and destroys the variable in shared memory. It should not destroy the shared memory itself.
		* We also assume it is the writer, which constructs the shared memory, that should clean and destroy the variables and the shared memory.
		*/			
		virtual void close() = 0;

	protected:

		/**
		* Object refering to shared memory itself
		*/	
		managed_shared_memory m_oSharedMemory;
		
		/**
		* name by which we can identify the shared memory 
		*/	
		OpenViBE::CString m_sSharedMemoryName;
		
		/**
		* name by which we can identify a certain variable in shared memory
		*/	
		OpenViBE::CString m_sSharedVariableName;
		
		/**
		* whether it was able to open shared memory and its corresponding variable
		*/	
		OpenViBE::boolean m_bFailedToFind;
		
		/**
		* a named mutex in shared memory to make sure reading and writing is done exclusively
		*/	
		named_mutex* m_pMutex;
		
	private:
		OpenViBE::CString m_sMutexName;
	};
};
#endif
#endif
