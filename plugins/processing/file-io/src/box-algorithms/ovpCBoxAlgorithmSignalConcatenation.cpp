#include "ovpCBoxAlgorithmSignalConcatenation.h"
#include <openvibe/ovITimeArithmetics.h>

#include <system/ovCMemory.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::FileIO;

using namespace std;

boolean CBoxAlgorithmSignalConcatenation::initialize(void)
{
	m_vSignalChunkBuffers.resize(this->getStaticBoxContext().getInputCount() >> 1);
	m_vStimulationChunkBuffers.resize(this->getStaticBoxContext().getInputCount() >> 1);

	m_ui64TimeOut = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_ui64TimeOut = m_ui64TimeOut << 32;
	this->getLogManager() << LogLevel_Info << "Timeout set to "<< time64(m_ui64TimeOut) <<".\n";
	for(uint32 i = 0; i < this->getStaticBoxContext().getInputCount(); i+=2)
	{
		m_vEndOfFileStimulations.push_back(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), (i>>2)+1));
		m_vEndOfFileReached.push_back(false);
		m_vFileEndTimes.push_back(0);
		
	}

	for(uint32 i = 0; i < this->getStaticBoxContext().getInputCount(); i+=2)
	{
		OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmSignalConcatenation > * l_pSignalDecoder = new OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmSignalConcatenation >(*this,i);
		OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmSignalConcatenation > * l_pStimDecoder = new OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmSignalConcatenation >(*this,i+1);

		m_vSignalDecoders.push_back(l_pSignalDecoder);
		m_vStimulationDecoders.push_back(l_pStimDecoder);
		CStimulationSet * l_pStimSet = new CStimulationSet();
		m_vStimulationSets.push_back(l_pStimSet);
	}

	m_oStimulationEncoder.initialize(*this,1);
	m_oStimulationEncoder.getInputStimulationSet().setReferenceTarget(m_vStimulationDecoders[0]->getOutputStimulationSet());

	m_oSignalEncoder.initialize(*this,0);

	m_oTriggerEncoder.initialize(*this,2);
	m_oTriggerEncoder.getInputStimulationSet().setReferenceTarget(m_vStimulationDecoders[0]->getOutputStimulationSet());

	m_ui32HeaderReceivedCount = 0;
	m_ui32EndReceivedCount = 0;

	m_bHeaderSent = false;
	m_bEndSent = false;
	m_bStimHeaderSent = false;
	m_bConcatenationFinished = false;
	m_bResynchroDone = false;
	m_bStatsPrinted = false;

	m_sState.ui32CurrentFileIndex        = 0;
	m_sState.ui32CurrentChunkIndex       = 0;
	m_sState.ui32CurrentStimulationIndex = 0;

	m_ui64TriggerDate = 0;
	m_ui64LastChunkStartTime = 0;
	m_ui64LastChunkEndTime = 0;

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmSignalConcatenation::uninitialize(void)
{
	m_oStimulationEncoder.uninitialize();
	m_oSignalEncoder.uninitialize();
	m_oTriggerEncoder.uninitialize();

	for(uint32 i = 0; i < m_vSignalDecoders.size(); i++)
	{
		m_vSignalDecoders[i]->uninitialize();
		m_vStimulationDecoders[i]->uninitialize();
		delete m_vSignalDecoders[i];
		delete m_vStimulationDecoders[i];
	}

	for(uint32 i = 0; i < m_vSignalChunkBuffers.size(); i++)
	{
		for(uint32 j = 0; j < m_vSignalChunkBuffers[i].size(); j++)
		{
			delete m_vSignalChunkBuffers[i][j].m_pMemoryBuffer;
		}
	}

	for(uint32 i = 0; i < m_vStimulationChunkBuffers.size(); i++)
	{
		for(uint32 j = 0; j < m_vStimulationChunkBuffers[i].size(); j++)
		{
			delete m_vStimulationChunkBuffers[i][j].m_pStimulationSet;
		}
	}

	for(uint32 i = 0; i < m_vStimulationSets.size(); i++)
	{
		delete m_vStimulationSets[i];
	}
	
	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmSignalConcatenation::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmSignalConcatenation::processClock(CMessageClock& rMessageClock)
{
	if(!m_bHeaderSent || m_bConcatenationFinished)
	{
		return true;
	}

	uint64 l_ui64CurrentPlayerTime = this->getPlayerContext().getCurrentTime();
	
	for(uint32 i = 0; i < m_vFileEndTimes.size(); i++)
	{
		if(!m_vEndOfFileReached[i] && l_ui64CurrentPlayerTime > m_vFileEndTimes[i] + m_ui64TimeOut)
		{
			m_vEndOfFileReached[i] = true;
			this->getLogManager() << LogLevel_Info << "File #" << i+1 << "/" << (this->getStaticBoxContext().getInputCount()/2) << " has timed out (effective end time: "<< time64(m_vFileEndTimes[i]) <<").\n";
		}
	}

	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmSignalConcatenation::process(void)
{
	
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	//SIGNAL INPUTS
	for(uint32 input = 0 ; input < l_rStaticBoxContext.getInputCount() ; input+=2)
	{
		const uint32 l_ui32SignalDecoderIndex = input >> 1;

		for(uint32 chunk = 0; chunk < l_rDynamicBoxContext.getInputChunkCount(input); chunk++)
		{
			m_vSignalDecoders[l_ui32SignalDecoderIndex]->decode(chunk, true);

			if(m_vSignalDecoders[l_ui32SignalDecoderIndex]->isHeaderReceived())
			{
				// Not received all headers we expect? Decode and test ...
				if(m_ui32HeaderReceivedCount < l_rStaticBoxContext.getInputCount()/2)
				{
					const uint64 l_ui64SamplingFrequency = m_vSignalDecoders[l_ui32SignalDecoderIndex]->getOutputSamplingRate();
					const uint32 l_ui32ChannelCount = m_vSignalDecoders[l_ui32SignalDecoderIndex]->getOutputMatrix()->getDimensionSize(0);
					const uint32 l_ui32SampleCountPerBuffer = m_vSignalDecoders[l_ui32SignalDecoderIndex]->getOutputMatrix()->getDimensionSize(1);

					// Note that the stream may be decoded in any order, hence e.g. stream  2 header may be received before stream 1	 ...
					if(m_ui32HeaderReceivedCount == 0)
					{
						this->getLogManager() << LogLevel_Info << "Common sampling rate is " << l_ui64SamplingFrequency << ", channel count is " << l_ui32ChannelCount << " and sample count per buffer is " << l_ui32SampleCountPerBuffer <<".\n";
						
						// Set the encoder to follow the parameters of this first received input
						m_oSignalEncoder.getInputSamplingRate().setReferenceTarget(m_vSignalDecoders[l_ui32SignalDecoderIndex]->getOutputSamplingRate());
						m_oSignalEncoder.getInputMatrix().setReferenceTarget(m_vSignalDecoders[l_ui32SignalDecoderIndex]->getOutputMatrix());

						m_oSignalEncoder.encodeHeader();
						l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(input,chunk), l_rDynamicBoxContext.getInputChunkEndTime(input,chunk));
						m_bHeaderSent = true;
					}
					else
					{
						if(m_oSignalEncoder.getInputSamplingRate() != l_ui64SamplingFrequency)
						{
							this->getLogManager() << LogLevel_Error << "File #" 
								<< (l_ui32SignalDecoderIndex)+1 << "/" << (l_rStaticBoxContext.getInputCount()/2) 
								<< " has a different sampling rate (" << l_ui64SamplingFrequency 
								<< "Hz) than other file(s) ("<< m_oSignalEncoder.getInputSamplingRate() << "Hz).\n";
							return false;
						}
						if(m_oSignalEncoder.getInputMatrix()->getDimensionSize(0) != l_ui32ChannelCount)
						{
							this->getLogManager() << LogLevel_Error << "File #" 
								<< (l_ui32SignalDecoderIndex)+1 << "/" << (l_rStaticBoxContext.getInputCount()/2) 
								<< " has a different channel count (" << l_ui32ChannelCount
								<< ") than other file(s) ("<< m_oSignalEncoder.getInputMatrix()->getDimensionSize(0) << ").\n";
							return false;
						}
						if(m_oSignalEncoder.getInputMatrix()->getDimensionSize(1) != l_ui32SampleCountPerBuffer)
						{
							this->getLogManager() << LogLevel_Error << "File #" 
								<< (l_ui32SignalDecoderIndex)+1 << "/" << (l_rStaticBoxContext.getInputCount()/2) 
								<< " has a different sample count per buffer (" << l_ui32SampleCountPerBuffer 
								<< ") than other file(s) ("<< m_oSignalEncoder.getInputMatrix()->getDimensionSize(1) <<").\n";
							return false;
						}
					}

					m_ui32HeaderReceivedCount++;
				}
			}

//			if(m_vSignalDecoders[l_ui32SignalDecoderIndex]->isBufferReceived() && (!m_vEndOfFileReached[l_ui32SignalDecoderIndex] || l_rDynamicBoxContext.getInputChunkStartTime(input,chunk) <= m_vFileEndTimes[l_ui32SignalDecoderIndex]))
			if(m_vSignalDecoders[l_ui32SignalDecoderIndex]->isBufferReceived() && !m_vEndOfFileReached[l_ui32SignalDecoderIndex])
			{
				IMemoryBuffer * l_pBuffer = new CMemoryBuffer();
				l_pBuffer->setSize(l_rDynamicBoxContext.getInputChunk(input,chunk)->getSize(),true);
				System::Memory::copy(
					l_pBuffer->getDirectPointer(),
					l_rDynamicBoxContext.getInputChunk(input,chunk)->getDirectPointer(),
					l_pBuffer->getSize());
				Chunk l_oChunk;
				l_oChunk.m_pMemoryBuffer = l_pBuffer;
				l_oChunk.m_ui64StartTime = l_rDynamicBoxContext.getInputChunkStartTime(input,chunk);
				l_oChunk.m_ui64EndTime = l_rDynamicBoxContext.getInputChunkEndTime(input,chunk);
				m_vSignalChunkBuffers[l_ui32SignalDecoderIndex].push_back(l_oChunk);

				if(l_rDynamicBoxContext.getInputChunkEndTime(input,chunk) < m_vFileEndTimes[l_ui32SignalDecoderIndex])
				{
					this->getLogManager() << LogLevel_Warning << "Oops, added extra chunk  " 
						<< time64(l_rDynamicBoxContext.getInputChunkStartTime(input,chunk))
						<< " to " << time64(l_rDynamicBoxContext.getInputChunkEndTime(input,chunk))
						<< "\n";
				}

				m_vFileEndTimes[l_ui32SignalDecoderIndex] = l_rDynamicBoxContext.getInputChunkEndTime(input,chunk);

				//this->getLogManager() << LogLevel_Info << "Input " << input << " signal chunk " << time64(l_oChunk.m_ui64StartTime) << " " << time64(l_oChunk.m_ui64EndTime) << "\n";
			}

			if(m_vSignalDecoders[l_ui32SignalDecoderIndex]->isEndReceived())
			{
				// we assume the signal chunks must be continuous, so the end time is the end of the last buffer, don't set here

				//just discard it (automatic by decoder)
			}
		}
	}

	//STIMULATION INPUTS
	for(uint32 input = 1 ; input < l_rStaticBoxContext.getInputCount() ; input+=2)
	{
		const uint32 l_ui32StimulationDecoderIndex = input >> 1;

		for(uint32 chunk = 0; chunk < l_rDynamicBoxContext.getInputChunkCount(input); chunk++)
		{
			m_vStimulationDecoders[l_ui32StimulationDecoderIndex]->decode(chunk, true);
			if(m_vStimulationDecoders[l_ui32StimulationDecoderIndex]->isHeaderReceived() && !m_bStimHeaderSent)
			{
				m_oStimulationEncoder.encodeHeader();
				l_rDynamicBoxContext.markOutputAsReadyToSend(1,l_rDynamicBoxContext.getInputChunkStartTime(input,chunk),l_rDynamicBoxContext.getInputChunkEndTime(input,chunk));
				m_oTriggerEncoder.encodeHeader();
				l_rDynamicBoxContext.markOutputAsReadyToSend(2,l_rDynamicBoxContext.getInputChunkStartTime(input,chunk),l_rDynamicBoxContext.getInputChunkEndTime(input,chunk));
				m_bStimHeaderSent = true;
			}
			if(m_vStimulationDecoders[l_ui32StimulationDecoderIndex]->isBufferReceived() && !m_vEndOfFileReached[l_ui32StimulationDecoderIndex])
			{
				//this->getLogManager() << LogLevel_Info << "Input " << input << " stim chunk " << time64(l_rDynamicBoxContext.getInputChunkStartTime(input,chunk))
				//	<< " " << time64(l_rDynamicBoxContext.getInputChunkEndTime(input,chunk) )
				//	<< "\n";

				const IStimulationSet * l_pStimSet = m_vStimulationDecoders[l_ui32StimulationDecoderIndex]->getOutputStimulationSet();

				StimulationChunk l_oChunk;
				l_oChunk.m_ui64StartTime = l_rDynamicBoxContext.getInputChunkStartTime(input,chunk);
				l_oChunk.m_ui64EndTime = l_rDynamicBoxContext.getInputChunkEndTime(input,chunk);

				if(l_pStimSet->getStimulationCount()>0) {
					l_oChunk.m_pStimulationSet = new CStimulationSet();
				} else {
					l_oChunk.m_pStimulationSet = NULL;
				}
				m_vStimulationChunkBuffers[l_ui32StimulationDecoderIndex].push_back(l_oChunk); // we store even if empty to be able to retain the chunking structure of the stimulation input stream

				for(uint32 stim = 0; stim < l_pStimSet->getStimulationCount(); stim++)
				{
					l_oChunk.m_pStimulationSet->appendStimulation(
						l_pStimSet->getStimulationIdentifier(stim),
						l_pStimSet->getStimulationDate(stim),
						l_pStimSet->getStimulationDuration(stim));

					this->getLogManager() << LogLevel_Trace << "Input " << input 
						<< ": Discovered stim " << l_pStimSet->getStimulationIdentifier(stim) 
						<< " at date [" << time64(l_pStimSet->getStimulationDate(stim)) 
						<< "] in chunk [" << time64(l_oChunk.m_ui64StartTime)
						<< ", " << time64(l_oChunk.m_ui64EndTime)
						<< "]\n";

					if(l_pStimSet->getStimulationIdentifier(stim) == m_vEndOfFileStimulations[l_ui32StimulationDecoderIndex])
					{
						m_vEndOfFileReached[l_ui32StimulationDecoderIndex] = true;
						m_vFileEndTimes[l_ui32StimulationDecoderIndex] = l_oChunk.m_ui64EndTime;
						this->getLogManager() << LogLevel_Info << "File #" << (l_ui32StimulationDecoderIndex)+1 << "/" << (l_rStaticBoxContext.getInputCount()/2) << " is finished (end time: "<< time64(m_vFileEndTimes[l_ui32StimulationDecoderIndex]) <<"). Later signal chunks will be discarded.\n";

						break;
					}
				}
			}
			if(m_vStimulationDecoders[l_ui32StimulationDecoderIndex]->isEndReceived() && !m_bEndSent)
			{
				m_ui32EndReceivedCount++;
			}

			if(m_ui32EndReceivedCount == l_rStaticBoxContext.getInputCount()/2 - 1)
			{

				m_bEndSent = true;
			}
		}
	}

	boolean l_bShouldConcatenate = true;
	for(uint32 i = 0; i < m_vEndOfFileReached.size(); i++)
	{
		l_bShouldConcatenate &= m_vEndOfFileReached[i];
	}
	
	if(l_bShouldConcatenate && !m_bStatsPrinted)
	{
		for(uint32 i=0;i<m_vStimulationChunkBuffers.size();i++) {
			if(m_vSignalChunkBuffers[i].size() != 0)
			{
				this->getLogManager() << LogLevel_Trace << "File " << i
					<< " has 1st signal chunk at " << time64(m_vSignalChunkBuffers[i][0].m_ui64StartTime)
					<< " last at [" << time64(m_vSignalChunkBuffers[i].back().m_ui64EndTime)
					<< ", " << time64(m_vSignalChunkBuffers[i].back().m_ui64EndTime)
					<< "].\n";
			}
			this->getLogManager() << LogLevel_Trace << "File " << i 
				<< " has 1st stim chunk at " << time64(m_vStimulationChunkBuffers[i][0].m_ui64StartTime)
				<< " last at [" << time64(m_vStimulationChunkBuffers[i].back().m_ui64EndTime)
				<< ", " << time64(m_vStimulationChunkBuffers[i].back().m_ui64EndTime)
				<< "].\n";
			this->getLogManager() << LogLevel_Trace << "File " << i
				<< " EOF is at " << time64(m_vFileEndTimes[i])
				<< "\n";
		}
		m_bStatsPrinted = true;
	}

	if(l_bShouldConcatenate && !m_bConcatenationFinished)
	{
		if(!this->concate())
		{
			// concatenation not finished, we will resume on next process
			return true;
		}
		else
		{
			m_oStimulationEncoder.encodeEnd();
			l_rDynamicBoxContext.markOutputAsReadyToSend(1,m_ui64LastChunkEndTime, m_ui64LastChunkEndTime);
			m_oTriggerEncoder.encodeEnd();
			l_rDynamicBoxContext.markOutputAsReadyToSend(2,m_ui64LastChunkEndTime, m_ui64LastChunkEndTime);
			m_oSignalEncoder.encodeEnd();
			l_rDynamicBoxContext.markOutputAsReadyToSend(0,m_ui64LastChunkEndTime, m_ui64LastChunkEndTime);

			m_oTriggerEncoder.getInputStimulationSet()->appendStimulation(OVTK_StimulationId_EndOfFile, this->getPlayerContext().getCurrentTime(), 0);
			m_oTriggerEncoder.encodeBuffer();
			l_rDynamicBoxContext.markOutputAsReadyToSend(2,this->getPlayerContext().getCurrentTime(),this->getPlayerContext().getCurrentTime());
			m_bConcatenationFinished = true;
		}
	}

	return true;
}


boolean CBoxAlgorithmSignalConcatenation::concate(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	if(!m_bResynchroDone)
	{
		this->getLogManager() << LogLevel_Info << "Concatenation in progress...\n";
		
		
		this->getLogManager() << LogLevel_Trace << "Resynchronizing Chunks ...\n";


		// note: m_vStimulationSets and m_vSignalChunkBuffers should have the same size (== number of files)

		uint64 l_ui64Offset = m_vFileEndTimes[0];

		for(uint32 i = 1; i < m_vStimulationChunkBuffers.size(); i++)
		{
			for(uint32 j = 0; j < m_vStimulationChunkBuffers[i].size(); j++)
			{
				IStimulationSet* l_pStimSet = m_vStimulationChunkBuffers[i][j].m_pStimulationSet;
				if(l_pStimSet) {
					for(uint32 k=0;k<l_pStimSet->getStimulationCount();k++)
					{
						const uint64 l_ui64SynchronizedDate = l_pStimSet->getStimulationDate(k) + l_ui64Offset;
						l_pStimSet->setStimulationDate(k,l_ui64SynchronizedDate);
						//this->getLogManager() << LogLevel_Info << "Resynchronizing stim ["<<m_vStimulations[i][j].first<<"] from time ["<<m_vStimulations[i][j].second<<"] to ["<<l_ui64SynchronizedDate<<"]\n";
					}
				}
				m_vStimulationChunkBuffers[i][j].m_ui64StartTime += l_ui64Offset;
				m_vStimulationChunkBuffers[i][j].m_ui64EndTime += l_ui64Offset;
			}

			for(uint32 j = 0; j < m_vSignalChunkBuffers[i].size(); j++)
			{
				m_vSignalChunkBuffers[i][j].m_ui64StartTime += l_ui64Offset;
				m_vSignalChunkBuffers[i][j].m_ui64EndTime += l_ui64Offset;
			}

			l_ui64Offset = l_ui64Offset + m_vFileEndTimes[i];
		}
		
		this->getLogManager() << LogLevel_Trace << "Resynchronization finished.\n";
		m_bResynchroDone = true;
	}
	
	// When we get here, resynchro has been done

	// note that the iterators are references on purpose...

	for(uint32& i=m_sState.ui32CurrentFileIndex; i<m_vSignalChunkBuffers.size(); i++)
	{
		const std::vector<Chunk>& l_rChunkVector = m_vSignalChunkBuffers[i];
		const std::vector<StimulationChunk>& l_rStimulusChunkVector = m_vStimulationChunkBuffers[i];

		// Send a signal chunk
		uint32& l_rChunk = m_sState.ui32CurrentChunkIndex;
		if(l_rChunk<l_rChunkVector.size())
		{
			// we write the signal memory buffer
			const IMemoryBuffer * l_pBuffer = l_rChunkVector[l_rChunk].m_pMemoryBuffer;
			IMemoryBuffer * l_pOutputMemoryBuffer = l_rDynamicBoxContext.getOutputChunk(0);
			l_pOutputMemoryBuffer->setSize(l_pBuffer->getSize(), true);
			System::Memory::copy(
				l_pOutputMemoryBuffer->getDirectPointer(),
				l_pBuffer->getDirectPointer(),
				l_pBuffer->getSize());
			l_rDynamicBoxContext.markOutputAsReadyToSend(0,l_rChunkVector[l_rChunk].m_ui64StartTime,l_rChunkVector[l_rChunk].m_ui64EndTime);

			/*
			if(ITimeArithmetics::timeToSeconds(l_rChunkVector[l_rChunk].m_ui64StartTime)>236)

			{
				this->getLogManager() << LogLevel_Info << "Adding signalchunk " << i << "," << l_rChunk << " [" 
					<< time64(l_rChunkVector[l_rChunk].m_ui64StartTime)
					<< ", " << time64(l_rChunkVector[l_rChunk].m_ui64EndTime) 
					<< "\n";
			}
			*/
		
			const uint64 l_ui64SignalChunkEnd = l_rChunkVector[l_rChunk].m_ui64EndTime;
	
			// Write stimulations up to this point
			for(uint32& k=m_sState.ui32CurrentStimulationIndex; 
				k < l_rStimulusChunkVector.size() && l_rStimulusChunkVector[k].m_ui64EndTime <= l_ui64SignalChunkEnd; 
				k++)
			{
				const StimulationChunk& l_rStimChunk = l_rStimulusChunkVector[k];
				const IStimulationSet * l_pBufferedStimSet = l_rStimChunk.m_pStimulationSet;

				IStimulationSet * l_pStimSet = m_oStimulationEncoder.getInputStimulationSet();
				l_pStimSet->clear();

				if(l_pBufferedStimSet)
				{
					for(uint32 s=0;s<l_pBufferedStimSet->getStimulationCount();s++)
					{

						l_pStimSet->appendStimulation(l_pBufferedStimSet->getStimulationIdentifier(s),
							l_pBufferedStimSet->getStimulationDate(s),
							l_pBufferedStimSet->getStimulationDuration(s));

						this->getLogManager() << LogLevel_Trace << "Adding stimulation " << l_pBufferedStimSet->getStimulationIdentifier(s) 
							<< " at date [" << time64(l_pStimSet->getStimulationDate(s)) 
							<< "] to chunk [" << time64(l_rStimChunk.m_ui64StartTime)
							<< ", " << time64(l_rStimChunk.m_ui64EndTime)
							<< "]\n";
					}
				}

				// encode the stim memory buffer even if it is empty
				m_oStimulationEncoder.encodeBuffer();
				l_rDynamicBoxContext.markOutputAsReadyToSend(1,l_rStimChunk.m_ui64StartTime,l_rStimChunk.m_ui64EndTime);
				/*
				if(ITimeArithmetics::timeToSeconds(l_rStimChunk.m_ui64StartTime)>238 &&
					ITimeArithmetics::timeToSeconds(l_rStimChunk.m_ui64StartTime)<242)

				{
					this->getLogManager() << LogLevel_Info << "Adding stimchunk " << i << "," << k << " [" 
						<< time64(l_rStimChunk.m_ui64StartTime)
						<< ", " << time64(l_rStimChunk.m_ui64EndTime) 
						<< "\n";
				}
				*/
			}

			// Let the kernel send blocks up to now, prevent freezing up sending everything at once
			l_rChunk++;
			return false;
		} 

		// For now we don't support stimuli that don't correspond to signal data, these ones are after the last signal chunk
		for(uint32& k=m_sState.ui32CurrentStimulationIndex; 
			k < l_rStimulusChunkVector.size();
			k++)
		{
			const StimulationChunk& l_rStimChunk = l_rStimulusChunkVector[k];
			const IStimulationSet * l_pBufferedStimSet = l_rStimChunk.m_pStimulationSet;

			if(i==m_vSignalChunkBuffers.size()-1) {
				// last file, let pass

				IStimulationSet * l_pStimSet = m_oStimulationEncoder.getInputStimulationSet();
				l_pStimSet->clear();

				if(l_pBufferedStimSet)
				{
					for(uint32 s=0;s<l_pBufferedStimSet->getStimulationCount();s++)
					{

						l_pStimSet->appendStimulation(l_pBufferedStimSet->getStimulationIdentifier(s),
							l_pBufferedStimSet->getStimulationDate(s),
							l_pBufferedStimSet->getStimulationDuration(s));

						this->getLogManager() << LogLevel_Warning << "Stimulation " << l_pBufferedStimSet->getStimulationIdentifier(s) 
							<< " at date [" << time64(l_pStimSet->getStimulationDate(s)) 
							<< "] in chunk [" << time64(l_rStimChunk.m_ui64StartTime)
							<< ", " << time64(l_rStimChunk.m_ui64EndTime)
							<< "] is after signal ended, but last file, so adding.\n";
					}
				}

				// encode the stim memory buffer even if it is empty
				m_oStimulationEncoder.encodeBuffer();
				l_rDynamicBoxContext.markOutputAsReadyToSend(1,l_rStimChunk.m_ui64StartTime,l_rStimChunk.m_ui64EndTime);
			}
			else
			{
				if(l_pBufferedStimSet)
				{

					for(uint32 s=0;s<l_pBufferedStimSet->getStimulationCount();s++)
					{
						if(l_rChunkVector.size() != 0)
						{
							this->getLogManager() << LogLevel_Warning
								<< "Stimulation " << l_pBufferedStimSet->getStimulationIdentifier(s)
								<< "'s chunk at [" << time64(l_rStimChunk.m_ui64StartTime)
								<< ", " << time64(l_rStimChunk.m_ui64EndTime)
								<< "] is after the last signal chunk end time " << time64(l_rChunkVector.back().m_ui64EndTime)
								<< ", discarded.\n";
						}
					}
				}
			}
		}


		// Finished with the file

		//	if(l_rStimChunk.m_ui64EndTime < l_rChunkVector[ui32CurrentChunkIndex].m_ui64EndTime) 
		//	{
				// There is no corresponding signal anymore, skip the rest of the stimulations from this file
				//this->getLogManager() << LogLevel_Info << "Stimulus time " << time64(l_rStimulusChunkVector[j].m_ui64EndTime) 
				//	<< " exceeds the last signal buffer end time " << time64(l_rChunkVector[l_rChunkVector.size()-1].m_ui64EndTime) 
				//	<< "\n";
				//break;
			//}
		m_sState.ui32CurrentChunkIndex = 0;
		m_sState.ui32CurrentStimulationIndex = 0;

		this->getLogManager() << LogLevel_Info << "File #" << i+1 <<" Finished.\n";
	}

	//We search for the last file with data.
	for(uint32 l_ui32LastFile = m_vSignalChunkBuffers.size() ; l_ui32LastFile > 0 ; l_ui32LastFile--)
	{
		const uint32 l_ui32LastChunkOfLastFile = m_vSignalChunkBuffers[l_ui32LastFile-1].size();
		if(l_ui32LastChunkOfLastFile != 0)
		{
			m_ui64LastChunkEndTime = m_vSignalChunkBuffers[l_ui32LastFile-1][l_ui32LastChunkOfLastFile-1].m_ui64EndTime;
			break;
		}
	}

	this->getLogManager() << LogLevel_Info << "Concatenation finished !\n";
	


	return true;

}
