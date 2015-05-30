#include "ovpCTemporalFilterBoxAlgorithm.h"
#include <cstdlib>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace std;

boolean CTemporalFilterBoxAlgorithm::initialize(void)
{
	CIdentifier l_oInputTypeIdentifier;
	getStaticBoxContext().getInputType(0, l_oInputTypeIdentifier);
	if(l_oInputTypeIdentifier==OV_TypeId_Signal)
	{
		m_pStreamDecoder=new OpenViBEToolkit::TSignalDecoder < CTemporalFilterBoxAlgorithm >(*this,0);
		m_pStreamEncoder=new OpenViBEToolkit::TSignalEncoder < CTemporalFilterBoxAlgorithm >(*this,0);
	}
	else
	{
		return false;
	}

	// Compute filter coeff algorithm
	m_pComputeTemporalFilterCoefficients=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_ComputeTemporalFilterCoefficients));
	m_pComputeTemporalFilterCoefficients->initialize();

	// Apply filter to signal input buffer
	m_pApplyTemporalFilter=&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_ApplyTemporalFilter));
	m_pApplyTemporalFilter->initialize();

	m_ui64LastEndTime = 0;

	// compute filter coefs settings
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

	TParameterHandler<uint64> ip_ui64NameFilter(m_pComputeTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_FilterMethod));
	ip_ui64NameFilter=this->getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_FilterMethod, l_oNameFilter);

	TParameterHandler<uint64> ip_ui64KindFilter(m_pComputeTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_FilterType));
	ip_ui64KindFilter=this->getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_FilterType, l_oKindFilter);

	TParameterHandler<uint64> ip_ui64FilterOrder(m_pComputeTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_FilterOrder));
	ip_ui64FilterOrder=atoi(l_oFilterOrder);

	TParameterHandler<float64> ip_f64LowCutFrequency(m_pComputeTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_LowCutFrequency));
	ip_f64LowCutFrequency=atof(l_oLowPassBandEdge);

	TParameterHandler<float64> ip_f64HighCutFrequency(m_pComputeTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_HighCutFrequency));
	ip_f64HighCutFrequency=atof(l_oHighPassBandEdge);

	TParameterHandler<float64> ip_f64PassBandRipple(m_pComputeTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_BandPassRipple));
	ip_f64PassBandRipple=atof(l_oPassBandRipple);

	TParameterHandler<uint64> ip_ui64SamplingFrequency(m_pComputeTemporalFilterCoefficients->getInputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputParameterId_SamplingFrequency));
	ip_ui64SamplingFrequency.setReferenceTarget(m_pStreamDecoder->getOutputSamplingRate());

	// apply filter settings
	m_pApplyTemporalFilter->getInputParameter(OVP_Algorithm_ApplyTemporalFilter_InputParameterId_FilterCoefficientsMatrix)->setReferenceTarget(m_pComputeTemporalFilterCoefficients->getOutputParameter(OVP_Algorithm_ComputeTemporalFilterCoefficients_OutputParameterId_Matrix));

	m_pStreamEncoder->getInputMatrix().setReferenceTarget(m_pApplyTemporalFilter->getOutputParameter(OVP_Algorithm_ApplyTemporalFilter_OutputParameterId_FilteredSignalMatrix));
	m_pStreamEncoder->getInputSamplingRate().setReferenceTarget(m_pStreamDecoder->getOutputSamplingRate());

	return true;
}

boolean CTemporalFilterBoxAlgorithm::uninitialize(void)
{
	m_pApplyTemporalFilter->uninitialize();
	m_pComputeTemporalFilterCoefficients->uninitialize();
	getAlgorithmManager().releaseAlgorithm(*m_pApplyTemporalFilter);
	getAlgorithmManager().releaseAlgorithm(*m_pComputeTemporalFilterCoefficients);

	//codecs
	m_pStreamEncoder->uninitialize();
	delete m_pStreamEncoder;
	m_pStreamDecoder->uninitialize();
	delete m_pStreamDecoder;

	return true;
}

boolean CTemporalFilterBoxAlgorithm::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CTemporalFilterBoxAlgorithm::process(void)
{
	IBoxIO& l_rDynamicBoxContext=getDynamicBoxContext();
	IBox& l_rStaticBoxContext=getStaticBoxContext();

	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(i); j++)
		{
			uint64 l_ui64StartTime=l_rDynamicBoxContext.getInputChunkStartTime(i, j);
			uint64 l_ui64EndTime=l_rDynamicBoxContext.getInputChunkEndTime(i, j);

			if(!m_pStreamDecoder->decode(j)) return false;

			//this has to be done here as it does not work if done once in initialize()
			IMatrix* l_pInputMatrix = m_pStreamDecoder->getOutputMatrix();
			TParameterHandler<IMatrix*> l_oMatrixToFilter = m_pApplyTemporalFilter->getInputParameter(OVP_Algorithm_ApplyTemporalFilter_InputParameterId_SignalMatrix);
			l_oMatrixToFilter.setReferenceTarget(l_pInputMatrix);

			if(m_pStreamDecoder->isHeaderReceived())
			{
				if(!m_pComputeTemporalFilterCoefficients->process(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputTriggerId_Initialize)) return false;
				if(!m_pComputeTemporalFilterCoefficients->process(OVP_Algorithm_ComputeTemporalFilterCoefficients_InputTriggerId_ComputeCoefficients)) return false;
				if(!m_pApplyTemporalFilter->process(OVP_Algorithm_ApplyTemporalFilter_InputTriggerId_Initialize)) return false;
				if(!m_pStreamEncoder->encodeHeader()) return false;

				l_rDynamicBoxContext.markOutputAsReadyToSend(i, l_ui64StartTime, l_ui64EndTime);
			}
			if(m_pStreamDecoder->isBufferReceived())
			{
				if (m_ui64LastEndTime==l_ui64StartTime)
				{
					if(!m_pApplyTemporalFilter->process(OVP_Algorithm_ApplyTemporalFilter_InputTriggerId_ApplyFilterWithHistoric)) return false;
				}
				else
				{
					if(!m_pApplyTemporalFilter->process(OVP_Algorithm_ApplyTemporalFilter_InputTriggerId_ApplyFilter)) return false;
				}
				if(!m_pStreamEncoder->encodeBuffer()) return false;
				l_rDynamicBoxContext.markOutputAsReadyToSend(i, l_ui64StartTime, l_ui64EndTime);
			}
			if(m_pStreamDecoder->isEndReceived())
			{
				if(!m_pStreamEncoder->encodeEnd()) return false;
				l_rDynamicBoxContext.markOutputAsReadyToSend(i, l_ui64StartTime, l_ui64EndTime);
			}

//			m_ui64LastStartTime=l_ui64StartTime;
			m_ui64LastEndTime=l_ui64EndTime;
			l_rDynamicBoxContext.markInputAsDeprecated(i, j);
		}
	}

	return true;
}
