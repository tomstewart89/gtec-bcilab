#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovexSharedStimulusReader__
#define __ovexSharedStimulusReader__

#include "ovexISharedMemoryReader.h"

/*#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>*/

namespace OpenViBEApplications
{

	typedef allocator<OpenViBE::uint32, managed_shared_memory::segment_manager>  ShmemAllocatorStimulation;
	typedef vector<OpenViBE::uint32, ShmemAllocatorStimulation> MyVectorStimulation;
	
	class SharedStimulusReader : public ISharedMemoryReader
	{
		
	public:
		
		/**
		 * Constructor calling the super class constructor 
		 */		
		SharedStimulusReader() : ISharedMemoryReader() {}
		
		/**
		 * Constructor calling the super class constructor 
		 */		
		SharedStimulusReader(OpenViBE::CString sharedMemoryName, OpenViBE::CString sharedVariableName) 
		: ISharedMemoryReader(sharedMemoryName, sharedVariableName) {}
			
		/**
		 * Destructor. Note: the creating application should do all the cleanup
		 */				
		virtual ~SharedStimulusReader()
		{
			//ISharedMemoryReader::~ISharedMemoryReader();	
			//normally the client should not close/clean up the allocated memory, the creating application should 
		}
	protected:
		
		/**
		* Finds and maps the shared variable specified by m_sSharedVariableName in the shared memory with name m_sSharedMemoryName
		*/			
		virtual OpenViBE::boolean open();
		
		/**
		* Finds and maps the shared variable specified by sharedVariableName in the shared memory with name sharedMemoryName
		* @param sharedMemoryName name of the shared memory
		* @param sharedVariableName name of the variable in shared memory we want to access
		*/			
		virtual OpenViBE::boolean open(OpenViBE::CString sharedMemoryName, OpenViBE::CString sharedVariableName);
		
		/**
		* Returns the first stimulation of the corresponding vector variable that resides in shared memory
		* @return OpenViBE's IStimulationSet
		*/			
		virtual OpenViBE::IStimulationSet* front();
		
		/**
		* Returns the first stimulation of the corresponding vector variable that resides in shared memory and deletes it
		* @return OpenViBE's IStimulationSet
		*/				
		virtual OpenViBE::IStimulationSet* pop_front();
		
		/**
		* clears and destroys the variable in shared memory. It should not destroy the shared memory itself.
		* We also assume it is the writer, which constructs the shared memory, that should clean and destroy the variables and the shared memory.
		*/				
		virtual void close();	
		
		/**
		* clears the vector in shared memory
		*/				
		virtual void clear();
		
	private:
		/**
		 * This method is the core implementation of retrieving the contents of the shared variable. The method is called
		 * by front() and pop_front() and enclosed by a mutex to garantuee exclusive operation
		 */		
		OpenViBE::IStimulationSet* _front();
		
	protected:
		/**
		 * Vector of uint32 (representing the stimulation values) in shared memory
		 */		
		MyVectorStimulation* m_vStimulusVector;
	};
};

#endif
#endif
