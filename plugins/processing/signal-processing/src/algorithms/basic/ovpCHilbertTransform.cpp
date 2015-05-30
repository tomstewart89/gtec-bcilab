#if defined(TARGET_HAS_ThirdPartyEIGEN)

#include "ovpCHilbertTransform.h"
#include <complex>
#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>
#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

using namespace Eigen;

OpenViBE::boolean HilbertTransform::transform(const Eigen::VectorXcd& vecXcdInput, Eigen::VectorXcd& vecXcdOutput) 
{
	const uint32 l_ui32NSamples = vecXcdInput.size();

	// Resize our buffers if input size has changed
	if((uint32)m_vecXcdSignalFourier.size() != l_ui32NSamples) 
	{
		m_vecXcdSignalFourier = VectorXcd::Zero(l_ui32NSamples);
		m_vecXcdHilbert = VectorXcd::Zero(l_ui32NSamples);

		//Initialization of vector h used to compute analytic signal
		m_vecXcdHilbert(0) = 1.0;

		if(l_ui32NSamples%2 == 0)
		{
			m_vecXcdHilbert(l_ui32NSamples/2) = 1.0;

			m_vecXcdHilbert.segment(1,(l_ui32NSamples/2)-1).setOnes();
			m_vecXcdHilbert.segment(1,(l_ui32NSamples/2)-1) *= 2.0;

			m_vecXcdHilbert.tail((l_ui32NSamples/2)+1).setZero();
		}
		else
		{
			m_vecXcdHilbert((l_ui32NSamples+1)/2) = 1.0;

			m_vecXcdHilbert.segment(1,(l_ui32NSamples/2)).setOnes();
			m_vecXcdHilbert.segment(1,(l_ui32NSamples/2)) *= 2.0;

			m_vecXcdHilbert.tail(((l_ui32NSamples+1)/2)+1).setZero();
		}
	}

	// Always resize output for safety
	vecXcdOutput.resize(l_ui32NSamples);

	//Fast Fourier Transform of input signal
	m_oFFT.fwd(m_vecXcdSignalFourier, vecXcdInput);

	//Apply Hilbert transform by element-wise multiplying fft vector by h
	m_vecXcdSignalFourier = m_vecXcdSignalFourier.cwiseProduct(m_vecXcdHilbert);
		
	//Inverse Fast Fourier transform
	m_oFFT.inv(vecXcdOutput, m_vecXcdSignalFourier); // m_vecXcdSignalBuffer is now the analytical signal of the initial input signal

	return true;
}

boolean CAlgorithmHilbertTransform::initialize(void)
{
	ip_pMatrix.initialize(this->getInputParameter(OVP_Algorithm_HilbertTransform_InputParameterId_Matrix));
	op_pHilbertMatrix.initialize(this->getOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_HilbertMatrix));
	op_pEnvelopeMatrix.initialize(this->getOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_EnvelopeMatrix));
	op_pPhaseMatrix.initialize(this->getOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_PhaseMatrix));

	return true;
}

boolean CAlgorithmHilbertTransform::uninitialize(void)
{
	op_pHilbertMatrix.uninitialize();
	op_pEnvelopeMatrix.uninitialize();
	op_pPhaseMatrix.uninitialize();
	ip_pMatrix.uninitialize();

	return true;
}

boolean CAlgorithmHilbertTransform::process(void)
{

	const uint32 l_ui32ChannelCount = ip_pMatrix->getDimensionSize(0);
	const uint32 l_ui32SamplesPerChannel = ip_pMatrix->getDimensionSize(1);

	IMatrix* l_pInputMatrix = ip_pMatrix;
	IMatrix* l_pOutputHilbertMatrix = op_pHilbertMatrix;
	IMatrix* l_pOutputEnvelopeMatrix = op_pEnvelopeMatrix;
	IMatrix* l_pOutputPhaseMatrix = op_pPhaseMatrix;

	if(this->isInputTriggerActive(OVP_Algorithm_HilbertTransform_InputTriggerId_Initialize)) //Check if the input is correct
	{
		if( l_pInputMatrix->getDimensionCount() != 2)
		{
			this->getLogManager() << LogLevel_Error << "The input matrix must have 2 dimensions, here the dimension is "<<l_pInputMatrix->getDimensionCount()<<"\n";
			return false;
		}

		if( l_pInputMatrix->getDimensionSize(1) < 2)
		{
			this->getLogManager() << LogLevel_Error << "Can't compute Hilbert transform on data length "<<l_pInputMatrix->getDimensionSize(1)<<"\n";
			return false;
		}

		//Setting size of outputs

		l_pOutputHilbertMatrix->setDimensionCount(2);
		l_pOutputHilbertMatrix->setDimensionSize(0,l_ui32ChannelCount);
		l_pOutputHilbertMatrix->setDimensionSize(1,l_ui32SamplesPerChannel);

		l_pOutputEnvelopeMatrix->setDimensionCount(2);
		l_pOutputEnvelopeMatrix->setDimensionSize(0,l_ui32ChannelCount);
		l_pOutputEnvelopeMatrix->setDimensionSize(1,l_ui32SamplesPerChannel);

		l_pOutputPhaseMatrix->setDimensionCount(2);
		l_pOutputPhaseMatrix->setDimensionSize(0,l_ui32ChannelCount);
		l_pOutputPhaseMatrix->setDimensionSize(1,l_ui32SamplesPerChannel);

		for(uint32 i=0; i<l_ui32ChannelCount; i++)
		{
			l_pOutputHilbertMatrix->setDimensionLabel(0,i,l_pInputMatrix->getDimensionLabel(0,i));
			l_pOutputEnvelopeMatrix->setDimensionLabel(0,i,l_pInputMatrix->getDimensionLabel(0,i));
			l_pOutputPhaseMatrix->setDimensionLabel(0,i,l_pInputMatrix->getDimensionLabel(0,i));
		}

	}

	if(this->isInputTriggerActive(OVP_Algorithm_HilbertTransform_InputTriggerId_Process))
	{

		//Compute Hilbert transform for each channel separately
		for(uint32 channel=0; channel<l_ui32ChannelCount; channel++)
		{
			// We cannot do a simple ptr assignment here as we need to convert real input to a complex vector
			VectorXcd l_vecXcdSingleChannel = VectorXcd::Zero(l_ui32SamplesPerChannel);
			const float64 *l_pBuffer = &l_pInputMatrix->getBuffer()[channel*l_ui32SamplesPerChannel];
			for(uint32 samples=0; samples<l_ui32SamplesPerChannel; samples++) {
				l_vecXcdSingleChannel(samples) = l_pBuffer[samples];
				l_vecXcdSingleChannel(samples).imag(0.0);
			}

			VectorXcd l_vecXcdSingleChannelTransformed; 
			m_oHilbert.transform(l_vecXcdSingleChannel, l_vecXcdSingleChannelTransformed);

			//Compute envelope and phase and pass them to the corresponding outputs
			for(uint32 samples=0; samples<l_ui32SamplesPerChannel; samples++)
			{
				l_pOutputHilbertMatrix->getBuffer()[samples + channel*l_ui32SamplesPerChannel] = l_vecXcdSingleChannelTransformed(samples).imag();
				l_pOutputEnvelopeMatrix->getBuffer()[samples + channel*l_ui32SamplesPerChannel] = abs(l_vecXcdSingleChannelTransformed(samples));
				l_pOutputPhaseMatrix->getBuffer()[samples + channel*l_ui32SamplesPerChannel] = arg(l_vecXcdSingleChannelTransformed(samples));
			}

		}

	}
	return true;
}

#endif //TARGET_HAS_ThirdPartyEIGEN
