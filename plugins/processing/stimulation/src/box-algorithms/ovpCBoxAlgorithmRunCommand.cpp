#include "ovpCBoxAlgorithmRunCommand.h"
#include <cstdlib>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Stimulation;

boolean CBoxAlgorithmRunCommand::initialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	m_pStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pStimulationDecoder->initialize();
	ip_pMemoryBuffer.initialize(m_pStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pStimulationSet.initialize(m_pStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	for(uint32 i=0; i<l_rStaticBoxContext.getSettingCount(); i+=2)
	{
		m_vCommand[(uint64)FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i)].push_back(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i+1));
	}

	return true;
}

boolean CBoxAlgorithmRunCommand::uninitialize(void)
{
	op_pStimulationSet.uninitialize();
	ip_pMemoryBuffer.uninitialize();
	m_pStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pStimulationDecoder);

	return true;
}

boolean CBoxAlgorithmRunCommand::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

boolean CBoxAlgorithmRunCommand::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		ip_pMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, i);
		m_pStimulationDecoder->process();
		if(m_pStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
		}
		if(m_pStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			IStimulationSet* l_pStimulationSet=op_pStimulationSet;
			for(uint32 j=0; j<l_pStimulationSet->getStimulationCount(); j++)
			{
				uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);
				if(m_vCommand.find(l_ui64StimulationIdentifier)!=m_vCommand.end())
				{
					std::vector < CString >& l_rCommand=m_vCommand[l_ui64StimulationIdentifier];
					std::vector < CString >::const_iterator itCommand=l_rCommand.begin();
					while(itCommand!=l_rCommand.end())
					{
						CString l_sCommand = *itCommand;
#if defined(WIN32)
						// On Windows, we pad the call with quotes. This addresses the situation where both the
						// command and some of its arguments have spaces, e.g. the call being like
						//
						// "C:\Program Files\blah.exe" --something "C:\Program Files\duh.dat"
						//
						// Without padding the whole line with quotes, it doesn't seem work. 
						l_sCommand = CString("\"") + l_sCommand + CString ("\"");
#else
						// On other platforms, we pass the call as-is. We cannot pad the call in the scenario as the
						// Linux shell - on the other hand - doesn't like calls like ""blah blah" "blah""
#endif
						this->getLogManager() << LogLevel_Debug << "Running command [" << l_sCommand << "]\n";
						if(::system(l_sCommand.toASCIIString())<0)
						{
							this->getLogManager() << LogLevel_Warning << "Could not run command " << l_sCommand << "\n";
						}
						itCommand++;
					}
				}
			}
		}
		if(m_pStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
		}
		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}

	return true;
}
