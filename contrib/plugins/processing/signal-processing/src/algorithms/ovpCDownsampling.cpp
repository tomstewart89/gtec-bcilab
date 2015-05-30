#include "ovpCDownsampling.h"

#include <cmath> //floor, ceil
#include <cstdlib>


using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;
using namespace std;
// ________________________________________________________________________________________________________________
//

boolean CDownsampling::initialize(void)
{
	ip_ui64SamplingFrequency.initialize(getInputParameter(OVP_Algorithm_Downsampling_InputParameterId_SamplingFrequency));
	ip_ui64NewSamplingFrequency.initialize(getInputParameter(OVP_Algorithm_Downsampling_InputParameterId_NewSamplingFrequency));
	ip_pSignalMatrix.initialize(getInputParameter(OVP_Algorithm_Downsampling_InputParameterId_SignalMatrix));
	op_pSignalMatrix.initialize(getOutputParameter(OVP_Algorithm_Downsampling_OutputParameterId_SignalMatrix));

	m_f64LastValueOrigSignal = NULL;
	
	m_bFirst = true;
	
	return true;
}

boolean CDownsampling::uninitialize(void)
{
	free(m_f64LastValueOrigSignal);
	m_f64LastValueOrigSignal = NULL;
	
	op_pSignalMatrix.uninitialize();
	ip_pSignalMatrix.uninitialize();
	ip_ui64NewSamplingFrequency.uninitialize();
	ip_ui64SamplingFrequency.uninitialize();

	return true;
}

// ________________________________________________________________________________________________________________
//

boolean CDownsampling::process(void)
{
	uint32 l_ui32SignalOutputMatrixDimensionSizeEpoch, l_ui32IndexBegOutput;
	float64 l_f64BlocDuration, l_f64EndTime;
				
	// signal input vars
	IMatrix* l_pSignalInputMatrix=ip_pSignalMatrix;
	float64* l_pSignalInput = l_pSignalInputMatrix->getBuffer();
	uint32 l_ui32SignalInputMatrixDimensionNbChannels=ip_pSignalMatrix->getDimensionSize(0);
	uint32 l_ui32SignalInputMatrixDimensionSizeEpoch=ip_pSignalMatrix->getDimensionSize(1);

	const float64 l_f64InputSamplingFrequency = static_cast<float64>(ip_ui64SamplingFrequency);
	const float64 l_f64OutputSamplingFrequency = static_cast<float64>(ip_ui64NewSamplingFrequency);
	
	// signal output vars
	IMatrix* l_pSignalOutputMatrix=op_pSignalMatrix;
	l_pSignalOutputMatrix->setDimensionCount(ip_pSignalMatrix->getDimensionCount());
	uint32 l_ui32SignalOutputMatrixDimensionNbChannels=l_ui32SignalInputMatrixDimensionNbChannels;
	if ((m_bFirst)||(isInputTriggerActive(OVP_Algorithm_Downsampling_InputTriggerId_Resample)))
	{
		l_ui32SignalOutputMatrixDimensionSizeEpoch= (uint32)floor(l_ui32SignalInputMatrixDimensionSizeEpoch * (l_f64OutputSamplingFrequency/l_f64InputSamplingFrequency));
		
		l_f64BlocDuration = (float64)(l_ui32SignalInputMatrixDimensionSizeEpoch-1)/l_f64InputSamplingFrequency;
		l_f64EndTime = l_f64BlocDuration;
		
		m_f64LastTimeOrigSignal = 0;
		m_f64LastTimeNewSignal = 0;

		if (m_bFirst)
		{
			m_f64LastValueOrigSignal = (float64 *)calloc(l_ui32SignalInputMatrixDimensionNbChannels, sizeof(float64));
			if (m_f64LastValueOrigSignal == NULL)
			{
				this->getLogManager() << LogLevel_Error << "Memory allocation : last values of original signal.\n";
			}
		}
	}
	else
	{
		l_f64BlocDuration = (float64)l_ui32SignalInputMatrixDimensionSizeEpoch/l_f64InputSamplingFrequency;
		l_f64EndTime = m_f64LastTimeOrigSignal+l_f64BlocDuration;
		float64 l_f64TimePassed = l_f64EndTime - m_f64LastTimeNewSignal;
		l_ui32SignalOutputMatrixDimensionSizeEpoch = (uint32)floor(l_f64TimePassed*l_f64OutputSamplingFrequency);
	}
	
	if(l_ui32SignalOutputMatrixDimensionSizeEpoch==0) {
		this->getLogManager() << LogLevel_Error << "Output epoch size is 0. Increase input epoch size.\n";
		return false;
	}

	// this->getLogManager() << LogLevel_Info << "blockDur " << l_f64BlocDuration << " et " << l_f64EndTime << " lt " << m_f64LastTimeNewSignal << " td " << l_f64TimeDiff << " dim " << l_ui32SignalOutputMatrixDimensionSizeEpoch << "\n";

	l_pSignalOutputMatrix->setDimensionSize(0, l_ui32SignalOutputMatrixDimensionNbChannels);
	l_pSignalOutputMatrix->setDimensionSize(1, l_ui32SignalOutputMatrixDimensionSizeEpoch);
	float64* l_pSignalOutput = l_pSignalOutputMatrix->getBuffer();

	if(isInputTriggerActive(OVP_Algorithm_Downsampling_InputTriggerId_Initialize))
	{
	}

	if((isInputTriggerActive(OVP_Algorithm_Downsampling_InputTriggerId_ResampleWithHistoric))
		||(isInputTriggerActive(OVP_Algorithm_Downsampling_InputTriggerId_Resample)))
	{
		float64 l_f64CountOrig, l_f64CountNew=0, l_f64ValuePrev, l_f64TimePrev, l_f64ValueCur;
		int32 l_i32IndexInput;
				
		for (uint64 i=0;  i < l_ui32SignalOutputMatrixDimensionNbChannels; i++)
		{
			l_f64CountOrig = m_f64LastTimeOrigSignal;
			l_f64CountNew = m_f64LastTimeNewSignal+(1.0/(float64)ip_ui64NewSamplingFrequency);
			l_f64TimePrev = m_f64LastTimeOrigSignal;
			if ((m_bFirst)||(isInputTriggerActive(OVP_Algorithm_Downsampling_InputTriggerId_Resample)))
			{
				l_f64ValuePrev = l_pSignalInput[i*l_ui32SignalInputMatrixDimensionSizeEpoch];
				l_pSignalOutput[i*l_ui32SignalOutputMatrixDimensionSizeEpoch] = l_f64ValuePrev;
				l_ui32IndexBegOutput = 1;
				l_i32IndexInput = 0;
			}
			else
			{
				l_f64ValuePrev = m_f64LastValueOrigSignal[i];
				l_ui32IndexBegOutput = 0;
				l_i32IndexInput = -1;
			}
			for (uint64 j=l_ui32IndexBegOutput ; j<l_ui32SignalOutputMatrixDimensionSizeEpoch; j++)
			{
				while ((l_i32IndexInput < (int32)l_ui32SignalInputMatrixDimensionSizeEpoch) && (l_f64CountOrig < l_f64CountNew))
				{
					l_f64CountOrig += 1.0/(float64)ip_ui64SamplingFrequency;
					l_i32IndexInput++;
				}
				
				if (l_i32IndexInput == -1)
				{
					this->getLogManager() << LogLevel_Warning << "Downsampling problem : index value=-1\n";
				}
				else if (l_i32IndexInput < (int32)l_ui32SignalInputMatrixDimensionSizeEpoch)
				{
					l_f64ValueCur = l_pSignalInput[(i*l_ui32SignalInputMatrixDimensionSizeEpoch)+l_i32IndexInput];
					
					l_pSignalOutput[(i*l_ui32SignalOutputMatrixDimensionSizeEpoch)+j] = ((l_f64ValueCur-l_f64ValuePrev)*(l_f64CountNew-l_f64TimePrev)/(l_f64CountOrig-l_f64TimePrev))+l_f64ValuePrev;
					
					l_f64ValuePrev = l_f64ValueCur;
					l_f64TimePrev = l_f64CountOrig;
				}
				else
				{
					this->getLogManager() << LogLevel_Warning << "Downsampling problem : sample #"<<j<<"/"<<l_ui32SignalOutputMatrixDimensionSizeEpoch<<" time original signal="<<l_f64CountOrig<<" time new signal="<<l_f64CountNew<<" new signal sample #"<<l_i32IndexInput<<" /"<<l_ui32SignalInputMatrixDimensionSizeEpoch<<"\n";
					j = l_ui32SignalOutputMatrixDimensionSizeEpoch;
				}
				
				l_f64CountNew += 1.0/l_f64OutputSamplingFrequency;
			}
			
			if (l_ui32SignalOutputMatrixDimensionSizeEpoch > 0)
			{
				m_f64LastValueOrigSignal[i] = l_pSignalInput[(i*l_ui32SignalInputMatrixDimensionSizeEpoch)+l_ui32SignalInputMatrixDimensionSizeEpoch-1];
			}
		}
		if (l_ui32SignalOutputMatrixDimensionSizeEpoch > 0)
		{
			m_f64LastTimeNewSignal = l_f64CountNew - (1.0/l_f64OutputSamplingFrequency);
			m_f64LastTimeOrigSignal = l_f64EndTime;
		}
		if (m_bFirst)
		{
			m_bFirst=false;
		}
	}
	

	return true;
}
