#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovexSharedMatrixReader__
#define __ovexSharedMatrixReader__

#include "ovexISharedMemoryReader.h"

/*#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>*/

namespace OpenViBEApplications
{
	/**
	 * Representation of the streamed matrix in shared memory
	 */
	struct SMatrix
	{
		int rowDimension;
		int columnDimension;
		offset_ptr<double> data; /**< special pointer to the data with offset in shared memory*/
	};	
	
	typedef allocator<offset_ptr<SMatrix>, managed_shared_memory::segment_manager>  ShmemAllocatorMatrix;
	typedef vector<offset_ptr<SMatrix>, ShmemAllocatorMatrix> MyVectorStreamedMatrix;	

	class SharedMatrixReader : public ISharedMemoryReader
	{
	public:
		/**
		 * Constructor calling the super class constructor 
		 */
		SharedMatrixReader() : ISharedMemoryReader() {}
		
		/**
		 * Constructor calling the super class constructor 
		 */
		SharedMatrixReader(OpenViBE::CString sharedMemoryName, OpenViBE::CString sharedVariableName) 
		: ISharedMemoryReader(sharedMemoryName, sharedVariableName) {}
			
		/**
		 * Destructor. Note: the creating application should do all the cleanup
		 */	
		virtual ~SharedMatrixReader()
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
		* Returns the first IMatrix object of the corresponding vector variable that resides in shared memory
		* @return OpenViBE's IMatrix
		*/			
		virtual OpenViBE::IMatrix* front();
		
		/**
		* Returns the first IMatrix object of the corresponding vector variable that resides in shared memory and deletes it
		* @return OpenViBE's IMatrix
		*/				
		virtual OpenViBE::IMatrix* pop_front();
		
		/**
		* clears the vector in shared memory
		*/				
		virtual void clear();
		
		/**
		* clears and destroys the variable in shared memory. It should not destroy the shared memory itself.
		* We also assume it is the writer, which constructs the shared memory, that should clean and destroy the variables and the shared memory.
		*/				
		virtual void close();	
		
	private:
		/**
		 * This method is the core implementation of retrieving the contents of the shared variable. The method is called
		 * by front() and pop_front() and enclosed by a mutex to garantuee exclusive operation
		 */
		OpenViBE::IMatrix* _front();
		
	protected:
		/**
		 * Vector of SMatrix in shared memory
		 */
		MyVectorStreamedMatrix* m_vMatrixVector;
	};
};

#endif
#endif
