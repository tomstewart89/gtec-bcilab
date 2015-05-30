#include "ovpCBoxAlgorithmSharedMemoryWriter.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::FileReadingAndWriting;



//using namespace std;

//struct timeval currentLTime;

#define time2ms(x,y) ((x) * 1000 + y/1000.0) + 0.5

boolean CBoxAlgorithmSharedMemoryWriter::initialize(void)
{
	//m_oAlgo0_StimulationDecoder.initialize(*this);
	//m_oAlgo0_StreamedMatrixDecoder.initialize(*this);

	//remove and create shared memory
	m_sSharedMemoryName = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0); // can be local variable
	shared_memory_object::remove(m_sSharedMemoryName.toASCIIString());
	m_oSharedMemoryArray = managed_shared_memory(create_only, m_sSharedMemoryName.toASCIIString(),  655360);
	
	//remove and create mutex
	m_sMutexName = m_sSharedMemoryName+CString("_Mutex");
	named_mutex::remove(m_sMutexName.toASCIIString());
	m_oMutex = new named_mutex(open_or_create, m_sMutexName.toASCIIString());
	
	//create shared vector for meta info (type and name)
	const ShmemAllocatorMetaInfo alloc_inst_metainfo(m_oSharedMemoryArray.get_segment_manager());
	MyVectorMetaInfo* l_vMetaInfoVector  = m_oSharedMemoryArray.construct<MyVectorMetaInfo>("MetaInfo")(std::less<ShmString>(),alloc_inst_metainfo);
	const StringAllocator alloc_inst_string(m_oSharedMemoryArray.get_segment_manager());
	
	//fill meta info vector and create shared vector variable for the appropriate types
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{	
		CIdentifier l_oTypeIdentifier;
		std::ostringstream convert;   // stream used for the conversion
		convert << i; 		
		
		l_rStaticBoxContext.getInputType(i,l_oTypeIdentifier);
		if (l_oTypeIdentifier==OVTK_TypeId_StreamedMatrix)
		{
			m_vDecoder.push_back(new OpenViBEToolkit::TStreamedMatrixDecoder < CBoxAlgorithmSharedMemoryWriter >());
			ShmString l_sShmVariableName("Matrix", alloc_inst_string);
			l_sShmVariableName += ShmString(convert.str().c_str(), alloc_inst_string);
			l_vMetaInfoVector->insert(std::make_pair<const ShmString,CIdentifier>(l_sShmVariableName,l_oTypeIdentifier));

			const ShmemAllocatorMatrix alloc_inst(m_oSharedMemoryArray.get_segment_manager());
			m_vStreamedMatrix.push_back(m_oSharedMemoryArray.construct<MyVectorStreamedMatrix>(l_sShmVariableName.c_str())(alloc_inst));
			
			this->getLogManager() << LogLevel_Info  << "Constructed variable in shared memory of type matrix with name " << l_sShmVariableName.c_str() << "\n";
		}
		else if (l_oTypeIdentifier==OVTK_TypeId_Stimulations)
		{
			m_vDecoder.push_back(new OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmSharedMemoryWriter >());
			ShmString l_sShmVariableName("Stimuli", alloc_inst_string);
			l_sShmVariableName += ShmString(convert.str().c_str(), alloc_inst_string);
			l_vMetaInfoVector->insert(std::make_pair<const ShmString,CIdentifier>(l_sShmVariableName,l_oTypeIdentifier));
			
			const ShmemAllocatorStimulation alloc_inst(m_oSharedMemoryArray.get_segment_manager());
			m_vStimuliSet.push_back(m_oSharedMemoryArray.construct<MyVectorStimulation>(l_sShmVariableName.c_str())(alloc_inst));
			
			this->getLogManager() << LogLevel_Info  << "Constructed variable in shared memory of type stimulation with name " << l_sShmVariableName.c_str() << "\n"; 
		} else {
			this->getLogManager() << LogLevel_Warning  << "Input type " << l_oTypeIdentifier << " is not supported\n";
		}
		m_vDecoder.back()->initialize(*this,i);
	}

	//m_ui32InputCounter = 0;

	return true;
}

boolean CBoxAlgorithmSharedMemoryWriter::uninitialize(void)
{
	m_oSharedMemoryArray.destroy<MyVectorMetaInfo>("MetaInfo");
	
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	for(int i=l_rStaticBoxContext.getInputCount()-1; i>=0; i--)
	{
		CIdentifier l_oTypeIdentifier;
		l_rStaticBoxContext.getInputType(i,l_oTypeIdentifier);
		std::ostringstream convert;   // stream used for the conversion
		convert << i; 		
		
		if (l_oTypeIdentifier==OVTK_TypeId_StreamedMatrix)
		{
			this->getLogManager() << LogLevel_Debug << "Uninitialize shared memory variable associated with input " << i << "\n";
			for (uint32 it=0; it<m_vStreamedMatrix.back()->size(); it++)
			{
				m_oSharedMemoryArray.deallocate(m_vStreamedMatrix.back()->at(it)->data.get());
				m_oSharedMemoryArray.deallocate(m_vStreamedMatrix.back()->at(it).get());
			}
			this->getLogManager() << LogLevel_Debug << "Deallocated shared memory for variable with name " << (std::string("Matrix")+convert.str()).c_str() << "\n";
			m_vStreamedMatrix.back()->clear();	
			//this->getLogManager() << LogLevel_Info << "1\n";
			m_oSharedMemoryArray.destroy<MyVectorStreamedMatrix>((std::string("Matrix")+convert.str()).c_str());
			//this->getLogManager() << LogLevel_Info << "2\n";
			
			//TODO: pop_back()?
		}
		else if (l_oTypeIdentifier==OVTK_TypeId_Stimulations)
		{
			m_vStimuliSet.back()->clear();	
			m_oSharedMemoryArray.destroy<MyVectorStimulation>((std::string("Stimuli")+convert.str()).c_str());
		}
	}
	this->getLogManager() << LogLevel_Debug << "Destroyed all shared variables associated with input" << "\n";
	shared_memory_object::remove(m_sSharedMemoryName.toASCIIString());	
	named_mutex::remove(m_sMutexName.toASCIIString());
	delete m_oMutex;
	//m_oAlgo0_StimulationDecoder.uninitialize();
	//m_oAlgo0_StreamedMatrixDecoder.uninitialize();
	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		m_vDecoder[i]->uninitialize();
		delete m_vDecoder[i];
	}

	
	return true;
}

boolean CBoxAlgorithmSharedMemoryWriter::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

boolean CBoxAlgorithmSharedMemoryWriter::process(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	uint32 l_ui32StimulusInputCounter = 0;
	uint32 l_ui32MatrixInputCounter = 0;
	
	for(uint32 j=0; j<l_rStaticBoxContext.getInputCount(); j++)
	{
		CIdentifier l_oTypeIdentifier;
		l_rStaticBoxContext.getInputType(j,l_oTypeIdentifier);		
		if (l_oTypeIdentifier==OVTK_TypeId_Stimulations)
		{		
			for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(j); i++)
			{			
				//m_oAlgo0_StimulationDecoder.decode(j,i, false);
				m_vDecoder[j]->decode(i,false);
				//CStimulationSet l_oStimSet;
				IStimulationSet* l_pStimSet = ((OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmSharedMemoryWriter >*)m_vDecoder[j])->getOutputStimulationSet();
				//OpenViBEToolkit::Tools::StimulationSet::copy(l_oStimSet, *m_oAlgo0_StimulationDecoder.getOutputStimulationSet());
				if(m_vDecoder[j]->isHeaderReceived())
				{
					l_rDynamicBoxContext.markInputAsDeprecated(j,i);
				}
				if(m_vDecoder[j]->isBufferReceived())
				{
					if (l_pStimSet->getStimulationCount()>0)
					{
						scoped_lock<named_mutex> lock(*m_oMutex, try_to_lock);
						if (lock)
						{
							for (uint32 si=0; si<l_pStimSet->getStimulationCount(); si++)
							{
								m_vStimuliSet[l_ui32StimulusInputCounter]->push_back(l_pStimSet->getStimulationIdentifier(si)); 					
								this->getLogManager() << LogLevel_Info << "Added stimulus with id " << m_vStimuliSet[l_ui32StimulusInputCounter]->back() << " to shared memory variable\n";
							}
							l_rDynamicBoxContext.markInputAsDeprecated(j,i);
						}
						else
						{
							//this->getLogManager() << LogLevel_Warning  << "At time " << (uint32)currentLTime.tv_sec << "," << (uint32)currentLTime.tv_usec  << " shared memory writer could not lock mutex\n";		
							this->getLogManager() << LogLevel_Warning  << "Shared memory writer could not lock mutex\n";	
						}							
					}	
					else
					{
						l_rDynamicBoxContext.markInputAsDeprecated(j,i);
						//std::cout << "no stimuli in chunk for shared memory writer\n";
					}
				}
				if(m_vDecoder[j]->isEndReceived())
				{
					l_rDynamicBoxContext.markInputAsDeprecated(j,i);
				}
			}
			l_ui32StimulusInputCounter++;
		}
		else if (l_oTypeIdentifier==OVTK_TypeId_StreamedMatrix)
		{
			for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(j); i++)
			{				
				m_vDecoder[j]->decode(i, false);
				IMatrix* l_pMatrix = ((OpenViBEToolkit::TStreamedMatrixDecoder < CBoxAlgorithmSharedMemoryWriter >*)m_vDecoder[j])->getOutputMatrix();
				if(m_vDecoder[j]->isHeaderReceived())
				{
					l_rDynamicBoxContext.markInputAsDeprecated(j,i);
				}
				if(m_vDecoder[j]->isBufferReceived())
				{
					scoped_lock<named_mutex> lock(*m_oMutex, try_to_lock);
					if (lock)
					{
						/*for (uint32 it=0; it<m_pInputStreamedMatrix->size(); it++)
						{
							m_oSharedMemoryArray.deallocate(m_pInputStreamedMatrix->at(it)->data.get());
							m_oSharedMemoryArray.deallocate(m_pInputStreamedMatrix->at(it).get());
						}
						m_pInputStreamedMatrix->clear();*/
						
						offset_ptr<SMatrix> l_ShmMatrix= static_cast<SMatrix*>(m_oSharedMemoryArray.allocate(sizeof(SMatrix)));

						//if we receive a vector (second dimension to 0) we force to one otherwise no memory will be allocated
						uint32 row = (l_pMatrix->getDimensionSize(1)==0)?1:l_pMatrix->getDimensionSize(1);

						l_ShmMatrix->rowDimension = row;
						l_ShmMatrix->columnDimension = l_pMatrix->getDimensionSize(0);

						this->getLogManager() << LogLevel_Trace  << "dimensions " << l_pMatrix->getDimensionCount()
											  << "row " << l_pMatrix->getDimensionSize(1) << " by "
												<< " columns " << l_pMatrix->getDimensionSize(0) << "\n";
						
						l_ShmMatrix->data = static_cast<float64*>(m_oSharedMemoryArray.allocate(
							l_pMatrix->getDimensionSize(0)*row*sizeof(float64)));
						for (uint32 di=0; di<l_pMatrix->getBufferElementCount(); di++)
						{
							*(l_ShmMatrix->data+di) = *(l_pMatrix->getBuffer()+di);
							//std::cout << " " << *(l_ShmMatrix->data+di);
						}
						//std::cout << "\n";
						
						m_vStreamedMatrix[l_ui32MatrixInputCounter]->push_back(l_ShmMatrix);	
						
						l_rDynamicBoxContext.markInputAsDeprecated(j,i);
					}
				}
				if(m_vDecoder[j]->isEndReceived())
				{
					l_rDynamicBoxContext.markInputAsDeprecated(j,i);
				}
			}
			l_ui32MatrixInputCounter++;			
		}
	}	

	return true;
}
