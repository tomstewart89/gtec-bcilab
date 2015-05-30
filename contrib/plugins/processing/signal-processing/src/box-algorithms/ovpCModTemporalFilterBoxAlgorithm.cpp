#include "ovpCModTemporalFilterBoxAlgorithm.h"
#include <cstdlib>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace std;

boolean CModTemporalFilterBoxAlgorithm::initialize(void)
{
	CIdentifier l_oInputTypeIdentifier;
	getStaticBoxContext().getInputType(0, l_oInputTypeIdentifier);
	if(l_oInputTypeIdentifier==OV_TypeId_Signal)
	{
		m_pStreamDecoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamDecoder));
		m_pStreamEncoder=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
	}
	else
	{
		return false;
	}
	m_pStreamDecoder->initialize();
	m_pStreamEncoder->initialize();

	ip_pMemoryBufferToDecode.initialize(m_pStreamDecoder->getInputParameter(OVP_GD_Algorithm_SignalStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pEncodedMemoryBuffer.initialize(m_pStreamEncoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	// Compute filter coeff algorithm
	m_pComputeModTemporalFilterCoefficients=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_ComputeTemporalFilterCoefficients));
	m_pComputeModTemporalFilterCoefficients->initialize();

	// Apply filter to signal input buffer
	m_pApplyModTemporalFilter=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_ApplyTemporalFilter));
	m_pApplyModTemporalFilter->initialize();

	m_ui64LastEndTime = 0;

	m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));

	m_sFilterMethod=CString("");
	m_sFilterType=CString("");
	m_sFilterOrder=CString("");
	m_sLowBand=CString("");
	m_sHighBand=CString("");
	m_sPassBandRiple=CString("");
	updateSettings();

	m_pComputeModTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_SamplingFrequency)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));


	// apply filter settings
	m_pApplyModTemporalFilter->getInputParameter(OVP_Algorithm_ApplyTemporalFilter_InputParameterId_SignalMatrix)->setReferenceTarget(m_pStreamDecoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix));
	m_pApplyModTemporalFilter->getInputParameter(OVP_Algorithm_ApplyTemporalFilter_InputParameterId_FilterCoefficientsMatrix)->setReferenceTarget(m_pComputeModTemporalFilterCoefficients->getOutputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_OutputParameterId_Matrix));

	m_pStreamEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix)->setReferenceTarget(m_pApplyModTemporalFilter->getOutputParameter(OVP_Algorithm_ApplyTemporalFilter_OutputParameterId_FilteredSignalMatrix));
	return true;
}

bool CModTemporalFilterBoxAlgorithm::updateSettings()
{
	bool retVal = false;
	//get the settings
	CString l_oNameFilter;
	CString l_oKindFilter;
	CString l_oFilterOrder;
	CString l_oLowPassBandEdge;
	CString l_oHighPassBandEdge;
	CString l_oPassBandRipple;

	getStaticBoxContext().getSettingValue(0, l_oNameFilter);
	getStaticBoxContext().getSettingValue(1, l_oKindFilter);
	getStaticBoxContext().getSettingValue(2, l_oFilterOrder);
	getStaticBoxContext().getSettingValue(3, l_oLowPassBandEdge);
	getStaticBoxContext().getSettingValue(4, l_oHighPassBandEdge);
	getStaticBoxContext().getSettingValue(5, l_oPassBandRipple);

	if(m_sFilterMethod!=l_oNameFilter)
	{
		TParameterHandler<uint64> ip_ui64NameFilter(m_pComputeModTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_FilterMethod));
		ip_ui64NameFilter=this->getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_FilterMethod, l_oNameFilter);
		m_sFilterMethod=l_oNameFilter;
		retVal=true;
	}

	if(m_sFilterType!=l_oKindFilter)
	{
		TParameterHandler<uint64> ip_ui64KindFilter(m_pComputeModTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_FilterType));
		ip_ui64KindFilter=this->getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_FilterType, l_oKindFilter);
		m_sFilterType=l_oKindFilter;
		retVal=true;
	}

	if(m_sFilterOrder!=l_oFilterOrder)
	{
		TParameterHandler<uint64> ip_ui64FilterOrder(m_pComputeModTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_FilterOrder));
		ip_ui64FilterOrder=atoi(l_oFilterOrder);
		m_sFilterOrder=l_oFilterOrder;
		retVal=true;
	}

	if(m_sLowBand!=l_oLowPassBandEdge)
	{
		TParameterHandler<float64> ip_f64LowCutFrequency(m_pComputeModTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_LowCutFrequency));
		ip_f64LowCutFrequency=atof(l_oLowPassBandEdge);
		m_sLowBand=l_oLowPassBandEdge;
		retVal=true;
	}

	if(m_sHighBand!=l_oHighPassBandEdge)
	{
		TParameterHandler<float64> ip_f64HighCutFrequency(m_pComputeModTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_HighCutFrequency));
		ip_f64HighCutFrequency=atof(l_oHighPassBandEdge);
		m_sHighBand=l_oHighPassBandEdge;
		retVal=true;
	}

	if(m_sPassBandRiple!=l_oPassBandRipple)
	{
		TParameterHandler<float64> ip_f64PassBandRipple(m_pComputeModTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_BandPassRipple));
		ip_f64PassBandRipple=atof(l_oPassBandRipple);
		m_sPassBandRiple=l_oPassBandRipple;
		retVal=true;
	}

	return retVal;
}

bool CModTemporalFilterBoxAlgorithm::compute()
{
	bool retVal = true;
	//compute filter coeff

	if(!m_pComputeModTemporalFilterCoefficients->process(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputTriggerId_Initialize)) return false;
	if(!m_pComputeModTemporalFilterCoefficients->process(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputTriggerId_ComputeCoefficients)) return false;
	if(!m_pApplyModTemporalFilter->process(OVP_Algorithm_ApplyTemporalFilter_InputTriggerId_Initialize)) return false;

	return retVal;
}

boolean CModTemporalFilterBoxAlgorithm::uninitialize(void)
{
	m_pApplyModTemporalFilter->uninitialize();
	m_pComputeModTemporalFilterCoefficients->uninitialize();
	m_pStreamEncoder->uninitialize();
	m_pStreamDecoder->uninitialize();

	getAlgorithmManager().releaseAlgorithm(*m_pApplyModTemporalFilter);
	getAlgorithmManager().releaseAlgorithm(*m_pComputeModTemporalFilterCoefficients);
	getAlgorithmManager().releaseAlgorithm(*m_pStreamEncoder);
	getAlgorithmManager().releaseAlgorithm(*m_pStreamDecoder);

	return true;
}

boolean CModTemporalFilterBoxAlgorithm::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CModTemporalFilterBoxAlgorithm::process(void)
{
	IBoxIO& l_rDynamicBoxContext=getDynamicBoxContext();
	IBox& l_rStaticBoxContext=getStaticBoxContext();

	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(i); j++)
		{
//			TParameterHandler < const IMemoryBuffer* > l_oInputMemoryBufferHandle(m_pStreamDecoder->getInputParameter(OVP_GD_Algorithm_SignalStreamDecoder_InputParameterId_MemoryBufferToDecode));
//			TParameterHandler < IMemoryBuffer* > l_oOutputMemoryBufferHandle(m_pStreamEncoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
//			l_oInputMemoryBufferHandle=l_rDynamicBoxContext.getInputChunk(i, j);
//			l_oOutputMemoryBufferHandle=l_rDynamicBoxContext.getOutputChunk(i);
			ip_pMemoryBufferToDecode=l_rDynamicBoxContext.getInputChunk(i, j);
			op_pEncodedMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(i);
			uint64 l_ui64StartTime=l_rDynamicBoxContext.getInputChunkStartTime(i, j);
			uint64 l_ui64EndTime=l_rDynamicBoxContext.getInputChunkEndTime(i, j);

			if(!m_pStreamDecoder->process()) return false;
			if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedHeader))
			{
				compute();
				if(!m_pStreamEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeHeader)) return false;

				l_rDynamicBoxContext.markOutputAsReadyToSend(i, l_ui64StartTime, l_ui64EndTime);
			}
			if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedBuffer))
			{

				if(updateSettings())
				{
					//recompute if the settings have changed only
					bool rVar = compute();
					if(!rVar)
					{
						this->getLogManager() << LogLevel_Error << "error during computation\n";
					}
				}

				if (m_ui64LastEndTime==l_ui64StartTime)
				{
					if(!m_pApplyModTemporalFilter->process(OVP_Algorithm_ApplyTemporalFilter_InputTriggerId_ApplyFilterWithHistoric)) return false;
				}
				else
				{
					if(!m_pApplyModTemporalFilter->process(OVP_Algorithm_ApplyTemporalFilter_InputTriggerId_ApplyFilter)) return false;
				}
				if(!m_pStreamEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer)) return false;
				l_rDynamicBoxContext.markOutputAsReadyToSend(i, l_ui64StartTime, l_ui64EndTime);
			}
			if(m_pStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedEnd))
			{
				if(!m_pStreamEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeEnd)) return false;
				l_rDynamicBoxContext.markOutputAsReadyToSend(i, l_ui64StartTime, l_ui64EndTime);
			}

			m_ui64LastEndTime=l_ui64EndTime;
			l_rDynamicBoxContext.markInputAsDeprecated(i, j);
		}
	}

	return true;
}
