#include "ovpCBoxAlgorithmStreamedMatrixSwitch.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Streaming;

using namespace OpenViBEToolkit;

using namespace std;

boolean CBoxAlgorithmStreamedMatrixSwitch::initialize(void)
{

	// Getting the settings to build the map Stim code / output index
	for(uint32 i = 1; i < this->getStaticBoxContext().getSettingCount(); i++)
	{
		const uint64 l_oStimCode  = FSettingValueAutoCast(*this->getBoxAlgorithmContext(),i);
		const uint32 l_ui32OutputIndex = i-1;
		if(!m_mStimulationOutputIndexMap.insert(make_pair(l_oStimCode, l_ui32OutputIndex)).second)
		{
			this->getLogManager() << LogLevel_Warning << "The stimulation code ["<<this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation,l_oStimCode) << "] for the output ["<< l_ui32OutputIndex << "] is already used by a previous output.\n";
		}
		else
		{
			this->getLogManager() << LogLevel_Trace << "The stimulation code ["<<this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation,l_oStimCode) << "] is registered for the output ["<< l_ui32OutputIndex << "]\n";
		}
	}

	boolean l_bDefaultToFirstOutput = FSettingValueAutoCast(*this->getBoxAlgorithmContext(),0);
	if(l_bDefaultToFirstOutput)
	{
		m_i32ActiveOutputIndex = 0;
	} 
	else
	{
		// At start, no output is active.
		m_i32ActiveOutputIndex = -1;
	}

	// Stimulation stream decoder
	m_oStimulationDecoder.initialize(*this,0);
	m_ui64LastStimulationInputChunkEndTime = 0;

	//initializing the decoder depending on the input type.
	CIdentifier l_oTypeIdentifier;
	this->getStaticBoxContext().getInputType(1,l_oTypeIdentifier);

	m_pStreamDecoder = NULL;

	if(l_oTypeIdentifier == OV_TypeId_StreamedMatrix)
	{
		m_pStreamDecoder = new TStreamedMatrixDecoder < CBoxAlgorithmStreamedMatrixSwitch >(*this,1);
	}
	else if(l_oTypeIdentifier == OV_TypeId_Signal)
	{
		m_pStreamDecoder = new TSignalDecoder < CBoxAlgorithmStreamedMatrixSwitch >(*this,1);
	}
	else if(l_oTypeIdentifier == OV_TypeId_Spectrum)
	{
		m_pStreamDecoder = new TSpectrumDecoder < CBoxAlgorithmStreamedMatrixSwitch >(*this,1);
	}
	else if(l_oTypeIdentifier == OV_TypeId_FeatureVector)
	{
		m_pStreamDecoder = new TFeatureVectorDecoder < CBoxAlgorithmStreamedMatrixSwitch >(*this,1);
	}
	else if(l_oTypeIdentifier == OV_TypeId_ChannelLocalisation)
	{
		m_pStreamDecoder = new TChannelLocalisationDecoder < CBoxAlgorithmStreamedMatrixSwitch >(*this,1);
	}
	else if(l_oTypeIdentifier == OV_TypeId_Stimulations)
	{
		m_pStreamDecoder = new TStimulationDecoder < CBoxAlgorithmStreamedMatrixSwitch >(*this,1);
	}
	else if(l_oTypeIdentifier == OV_TypeId_ExperimentInformation)
	{
		m_pStreamDecoder = new TExperimentInformationDecoder < CBoxAlgorithmStreamedMatrixSwitch >(*this,1);
	}
	else if(l_oTypeIdentifier == OV_TypeId_ChannelUnits)
	{
		m_pStreamDecoder = new TChannelUnitsDecoder < CBoxAlgorithmStreamedMatrixSwitch >(*this, 1);
	}
	else
	{
		this->getLogManager() << LogLevel_Error << "Unsupported stream type " << this->getTypeManager().getTypeName(l_oTypeIdentifier).toASCIIString() << " (" << l_oTypeIdentifier.toString() << ")\n";
		return false;
	}

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmStreamedMatrixSwitch::uninitialize(void)
{
	m_oStimulationDecoder.uninitialize();
	if(m_pStreamDecoder)
	{
		m_pStreamDecoder->uninitialize();
		delete m_pStreamDecoder;
	}

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmStreamedMatrixSwitch::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmStreamedMatrixSwitch::process(void)
{

	// the static box context describes the box inputs, outputs, settings structures
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	uint64 l_ui64StartTime=0;
	uint64 l_ui64EndTime=0;
	uint64 l_ui64ChunkSize=0;
	boolean l_bGotStimulation = false;
	const uint8* l_pChunkBuffer=NULL;

	//iterate over all chunk on input 0 (Stimulation)
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		m_oStimulationDecoder.decode(i);

		if(m_oStimulationDecoder.isHeaderReceived() || m_oStimulationDecoder.isEndReceived())
		{
			// nothing
		}
		if(m_oStimulationDecoder.isBufferReceived())
		{
			// we update the active output index and time if needed
			OpenViBE::IStimulationSet * l_pStimSet = m_oStimulationDecoder.getOutputStimulationSet();
			for(uint32 stim_index = 0 ; stim_index < l_pStimSet->getStimulationCount(); stim_index++)
			{
				if(m_mStimulationOutputIndexMap.find(l_pStimSet->getStimulationIdentifier(stim_index)) != m_mStimulationOutputIndexMap.end())
				{
					m_i32ActiveOutputIndex = m_mStimulationOutputIndexMap[l_pStimSet->getStimulationIdentifier(stim_index)];
					this->getLogManager() << LogLevel_Trace << "Switching with ["<<this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation,l_pStimSet->getStimulationIdentifier(stim_index)) << "] to output ["<< m_i32ActiveOutputIndex << "].\n";
				}
			}
			l_bGotStimulation = true;
			m_ui64LastStimulationInputChunkEndTime = l_rDynamicBoxContext.getInputChunkEndTime(0,i);
		}
	}

	for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(1); j++)
	{
		//We decode the chunk but we don't automatically mark it as deprecated, as we may need to keep it.
		m_pStreamDecoder->decode(j,false);
		{
			l_rDynamicBoxContext.getInputChunk(1, j, l_ui64StartTime, l_ui64EndTime, l_ui64ChunkSize, l_pChunkBuffer);
			if(m_pStreamDecoder->isHeaderReceived() || m_pStreamDecoder->isEndReceived())
			{
				for(uint32 k = 0; k < l_rStaticBoxContext.getOutputCount(); k++)
				{
					l_rDynamicBoxContext.appendOutputChunkData(k, l_pChunkBuffer, l_ui64ChunkSize);
					l_rDynamicBoxContext.markOutputAsReadyToSend(k, l_ui64StartTime, l_ui64EndTime);
				}
				l_rDynamicBoxContext.markInputAsDeprecated(1,j);
			}
			if(m_pStreamDecoder->isBufferReceived())
			{
				if(m_i32ActiveOutputIndex == -1)
				{
					// we drop every chunk when no output is activated
					l_rDynamicBoxContext.markInputAsDeprecated(1,j);
				}
				else
				{
					if(!l_bGotStimulation || (l_ui64StartTime < m_ui64LastStimulationInputChunkEndTime))
					{
						// the input chunk is in the good time range (we are sure that no stim has been received to change the active output)
						l_rDynamicBoxContext.appendOutputChunkData(m_i32ActiveOutputIndex, l_pChunkBuffer, l_ui64ChunkSize);
						l_rDynamicBoxContext.markOutputAsReadyToSend(m_i32ActiveOutputIndex, l_ui64StartTime, l_ui64EndTime);
						l_rDynamicBoxContext.markInputAsDeprecated(1,j);
					}
					// else : we keep the input chunk, no mark as deprecated !
				}
			}
		}
	}

	return true;
}
