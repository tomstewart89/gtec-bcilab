#include "ovpCBoxAlgorithmLatencyEvaluation.h"

// @note by just repeatedly printing to the console, this box introduces significant latency by itself.
// @fixme If it makes sense to enable this box at all, it should be reimplemented as printing to a small GUI widget instead.

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Tools;

boolean CBoxAlgorithmLatencyEvaluation::initialize(void)
{
	CString l_sLogLevel;
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, l_sLogLevel);
	m_eLogLevel=static_cast<ELogLevel>(getBoxAlgorithmContext()->getPlayerContext()->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_LogLevel, l_sLogLevel));

	m_ui64StartTime=System::Time::zgetTime();

	return true;
}

boolean CBoxAlgorithmLatencyEvaluation::uninitialize(void)
{
	return true;
}

boolean CBoxAlgorithmLatencyEvaluation::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

boolean CBoxAlgorithmLatencyEvaluation::process(void)
{
	// FIXME is it necessary to keep next line uncomment ?
	//IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	const uint64 l_ui64CurrentTime=getPlayerContext().getCurrentTime();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		const uint64 l_ui64StartTime=l_rDynamicBoxContext.getInputChunkStartTime(0, i);
		const uint64 l_ui64EndTime=l_rDynamicBoxContext.getInputChunkEndTime(0, i);

		const float64 l_f64StartLatencyMilli=(((((int64)(l_ui64CurrentTime-l_ui64StartTime)) >> 22) * 1000) / 1024.0);
		const float64 l_f64EndLatencyMilli=(((((int64)(l_ui64CurrentTime-l_ui64EndTime)) >> 22) * 1000) / 1024.0);

		// getLogManager() << LogLevel_Debug << "Timing values [start:end:current]=[" << l_ui64StartTime << ":" << l_ui64EndTime << ":" << l_ui64Time << "]\n";
		getLogManager() << m_eLogLevel << "Current latency at chunk " << i << " [start:end]=[" << l_f64StartLatencyMilli << ":" << l_f64EndLatencyMilli << "] ms\n";

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
	}

	const uint64 l_ui64RealTimeElapsed = System::Time::zgetTime()-m_ui64StartTime;
	const float64 l_f64InnerLatencyMilli=(((int64(l_ui64RealTimeElapsed - l_ui64CurrentTime) >> 22) * 1000) / 1024.0);

	getLogManager() << m_eLogLevel << "Inner latency : " << l_f64InnerLatencyMilli << " ms\n";

	return true;
}
