#if defined(TARGET_HAS_ThirdPartyEIGEN)

#include "ovpCAlgorithmMagnitudeSquaredCoherence.h"
#include <cmath>
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
using namespace std;

// Debug
#if 0
void dumpMatrix(const char *desc, const IMatrix& mat)
{
	const float64 *buf = mat.getBuffer();
	uint32 dim0size = mat.getDimensionSize(0);
	uint32 dim1size = mat.getDimensionSize(1);

	cout << desc << "\n";
	for(uint32 i=0;i<dim0size;i++) {
		cout << "Row " << i << ": ";
		for(uint32 j=0;j<dim1size;j++) {
			cout << buf[i*dim1size+j] << " ";
		}
		cout << "\n";
	}
}
#endif

// Let s=w.*x(k) be the windowed input signal x at segment k. The column k of the output matrix will be fft(s).
boolean CAlgorithmMagnitudeSquaredCoherence::computePeriodogram(const VectorXd& vecXdInput, Eigen::MatrixXcd& matXcdSignalFourier, const Eigen::VectorXd& vecXdWindow, const uint32& ui32NSegments, const uint32& ui32LSegments, const uint32& ui32NOverlap)
{
	matXcdSignalFourier = MatrixXcd::Zero(ui32LSegments, ui32NSegments);

	// Segment input vector and apply window to segment each segment
	for(uint32 k = 0; k<ui32NSegments; k++)
	{
		VectorXd l_vecXdWindowedSegments(ui32LSegments);

		for(uint32 i = 0; i<ui32LSegments; i++)
		{
			l_vecXdWindowedSegments(i) = vecXdInput(i+k*ui32NOverlap)*vecXdWindow(i);
		}

		// Remove the DC component of each channel. Without this we get really high coherences spread all over the 
		// spectrum when using e.g. the motor imagery dataset bundled with OpenViBE. @todo before or after windowing? here after.
		// l_vecXdWindowedSegments = l_vecXdWindowedSegments - VectorXd::Ones(ui32LSegments) * l_vecXdWindowedSegments.mean();

		VectorXcd l_vecXcdFourier = VectorXcd::Zero(ui32LSegments);

		// FFT of windowed segments
		m_oFFT.fwd(l_vecXcdFourier,l_vecXdWindowedSegments);

		// cout << "fi " << l_vecXdWindowedSegments.transpose() << "\n";
		// cout << "fo " << l_vecXcdFourier.transpose() << "\n";


		matXcdSignalFourier.col(k) = l_vecXcdFourier;

	}
	return true;
}


boolean CAlgorithmMagnitudeSquaredCoherence::powerSpectralDensity(const VectorXd& vecXdInput, VectorXd& vecXdOutput, const VectorXd& vecXdWindow, const uint32& ui32NSegments, const uint32& ui32LSegments, const uint32& ui32NOverlap)
{
	// Compute periodograms
	MatrixXcd l_matXcdSignalFourier;

	CAlgorithmMagnitudeSquaredCoherence::computePeriodogram(vecXdInput, l_matXcdSignalFourier, vecXdWindow, ui32NSegments, ui32LSegments, ui32NOverlap);
	//cout << "sf " << l_matXcdSignalFourier << "\n";

	// vecXdOutput(i) will be the power for the band i across segments (time) as summed from the periodogram
	vecXdOutput = VectorXd::Zero(ui32LSegments);
	for(uint32 k = 0; k<ui32NSegments; k++)
	{
		vecXdOutput += (l_matXcdSignalFourier.col(k).cwiseProduct(l_matXcdSignalFourier.col(k).conjugate())).real()/m_f64U;
	}
	vecXdOutput /= ui32NSegments;

 //   cout << "vxO " << vecXdOutput.transpose() << "\n";

	return true;

}


boolean CAlgorithmMagnitudeSquaredCoherence::crossSpectralDensity(const VectorXd& vecXdInput1, const VectorXd& vecXdInput2, VectorXcd& vecXcdOutput, const VectorXd& vecXdWindow, const uint32& ui32NSegments, const uint32& ui32LSegments, const uint32& ui32NOverlap)
{
	MatrixXcd l_matXcdSignalFourier1;
	MatrixXcd l_matXcdSignalFourier2;

	//Compute periodograms for input 1 and 2
	CAlgorithmMagnitudeSquaredCoherence::computePeriodogram(vecXdInput1, l_matXcdSignalFourier1, vecXdWindow, ui32NSegments, ui32LSegments, ui32NOverlap);
	CAlgorithmMagnitudeSquaredCoherence::computePeriodogram(vecXdInput2, l_matXcdSignalFourier2, vecXdWindow, ui32NSegments, ui32LSegments, ui32NOverlap);

	vecXcdOutput = VectorXcd::Zero(ui32LSegments);

	for(uint32 k = 0; k<ui32NSegments; k++)
	{
		vecXcdOutput += l_matXcdSignalFourier2.col(k).cwiseProduct(l_matXcdSignalFourier1.col(k).conjugate())/m_f64U;
	}

	vecXcdOutput = vecXcdOutput / ui32NSegments;

	// cout << "vxX " << vecXcdOutput.transpose() << "\n";

	return true;

}

boolean CAlgorithmMagnitudeSquaredCoherence::initialize(void)
{

	ip_pSignal1.initialize(this->getInputParameter(OVP_Algorithm_Connectivity_InputParameterId_InputMatrix1));
	ip_pSignal2.initialize(this->getInputParameter(OVP_Algorithm_Connectivity_InputParameterId_InputMatrix2));
	ip_ui64SamplingRate1.initialize(this->getInputParameter(OVP_Algorithm_Connectivity_InputParameterId_ui64SamplingRate1));

	ip_pChannelPairs.initialize(this->getInputParameter(OVP_Algorithm_Connectivity_InputParameterId_LookupMatrix));
	op_pMatrixMean.initialize(this->getOutputParameter(OVP_Algorithm_Connectivity_OutputParameterId_OutputMatrix));
	op_pMatrixSpectrum.initialize(this->getOutputParameter(OVP_Algorithm_MagnitudeSquaredCoherence_OutputParameterId_OutputMatrixSpectrum));
	op_pFrequencyBandVector.initialize(this->getOutputParameter(OVP_Algorithm_MagnitudeSquaredCoherence_OutputParameterId_FreqVector));

	ip_ui64WindowType.initialize(this->getInputParameter(OVP_Algorithm_MagnitudeSquaredCoherence_InputParameterId_Window));
	ip_ui64SegmentLength.initialize(this->getInputParameter(OVP_Algorithm_MagnitudeSquaredCoherence_InputParameterId_SegLength));
	ip_ui64Overlap.initialize(this->getInputParameter(OVP_Algorithm_MagnitudeSquaredCoherence_InputParameterId_Overlap));

	// Set default values
	ip_ui64WindowType = OVP_TypeId_WindowType_Welch.toUInteger();
	ip_ui64SegmentLength = 32;
	ip_ui64Overlap = 50;

	return true;
}

boolean CAlgorithmMagnitudeSquaredCoherence::uninitialize(void)
{

	ip_pSignal1.uninitialize();
	ip_pSignal2.uninitialize();
	ip_ui64SamplingRate1.uninitialize();

	ip_pChannelPairs.uninitialize();
	op_pMatrix.uninitialize();
	op_pMatrixSpectrum.uninitialize();
	op_pFrequencyBandVector.uninitialize();

	ip_ui64WindowType.uninitialize();
	ip_ui64SegmentLength.uninitialize();
	ip_ui64Overlap.uninitialize();

	return true;
}


boolean CAlgorithmMagnitudeSquaredCoherence::process(void)
{
	// Inputs
	const IMatrix* l_pInputMatrix1 = ip_pSignal1;
	const IMatrix* l_pInputMatrix2 = ip_pSignal2;
	const IMatrix* l_pChannelPairs = ip_pChannelPairs;
	// dumpMatrix("s1", *l_pInputMatrix1);
	// dumpMatrix("s2", *l_pInputMatrix2);

	const uint64 l_ui64SamplingRate = ip_ui64SamplingRate1;
	const uint32 l_ui32SamplesPerChannel1 = l_pInputMatrix1->getDimensionSize(1);
	const uint32 l_ui32SamplesPerChannel2 = l_pInputMatrix2->getDimensionSize(1);
	const uint32 l_ui32PairsCount = ip_pChannelPairs->getDimensionSize(0)/2;

	const float64* l_pMatrixBuffer1 = l_pInputMatrix1->getBuffer();
	const float64* l_pMatrixBuffer2 = l_pInputMatrix2->getBuffer();
	const float64* l_pChannelBuffer = l_pChannelPairs->getBuffer();

	const uint32 l_ui32SegmentsLength = (uint32) ip_ui64SegmentLength;
	const uint64 l_ui64WindowType = ip_ui64WindowType;
	const uint32 l_ui32OverlapPercent = (uint32) ip_ui64Overlap;
	// cout << "OP : " << l_ui32OverlapPercent << "\n";

	// Outputs
	IMatrix* l_pOutputMatrixMeanCoherence = op_pMatrixMean;
	IMatrix* l_pOutputMatrixCoherenceSpectrum = op_pMatrixSpectrum;
	IMatrix* l_pFrequencyVector = op_pFrequencyBandVector;

	if(this->isInputTriggerActive(OVP_Algorithm_Connectivity_InputTriggerId_Initialize))
	{

		// Do some checks
		if(l_ui32SamplesPerChannel1 != l_ui32SamplesPerChannel2)
		{
			this->getLogManager() << LogLevel_Error << "Can't compute MSCoherence on two signals with different lengths\n";
			return false;
		}

		if(l_ui32SamplesPerChannel1==0||l_ui32SamplesPerChannel2==0)
		{
			this->getLogManager() << LogLevel_Error << "Can't compute MSCoherence, input signal size = 0\n";
			return false;
		}

		if(l_ui32OverlapPercent < 0 || l_ui32OverlapPercent > 100)
		{
			this->getLogManager() << LogLevel_Error << "Overlap must be a value between 0 and 100\n";
			return false;
		}

		if(l_ui32SegmentsLength == 0)
		{
			this->getLogManager() << LogLevel_Error << "Segments must have a strictly positive length (>0)\n";
			return false;
		}

		// Setting window vector. We do this only once after we know the real segment length.
		m_vecXdWindow = VectorXd::Zero(l_ui32SegmentsLength);

		// Getting window vector
		if (l_ui64WindowType == OVP_TypeId_WindowType_Bartlett)
		{
			m_oWindow.bartlett(m_vecXdWindow, l_ui32SegmentsLength);
		}
		else if (l_ui64WindowType == OVP_TypeId_WindowType_Hamming)
		{
			m_oWindow.hamming(m_vecXdWindow, l_ui32SegmentsLength);
		}
		else if (l_ui64WindowType == OVP_TypeId_WindowType_Hann)
		{
			m_oWindow.hann(m_vecXdWindow, l_ui32SegmentsLength);
		}
		else if (l_ui64WindowType == OVP_TypeId_WindowType_Parzen)
		{
			m_oWindow.parzen(m_vecXdWindow, l_ui32SegmentsLength);
		}
		else if (l_ui64WindowType == OVP_TypeId_WindowType_Welch)
		{
			m_oWindow.welch(m_vecXdWindow, l_ui32SegmentsLength);
		}
		else // else rectangular window
		{
			m_vecXdWindow = VectorXd::Ones(l_ui32SegmentsLength);
		}
		// cout<<"window = "<<m_vecXdWindow.transpose()<<endl;

		// Calculate window normalization constant m_f64U. A 1/ui32LSegments factor has been removed since it will be canceled
		m_f64U = 0;
		for(uint32 i = 0; i<l_ui32SegmentsLength; i++)
		{
			m_f64U += pow(m_vecXdWindow(i),2);
		}
		// cout << "norm " << m_f64U << "\n";

		// Setting size of outputs
		l_pOutputMatrixMeanCoherence->setDimensionCount(2); // the output matrix will have 2 dimensions
		l_pOutputMatrixMeanCoherence->setDimensionSize(0,l_ui32PairsCount);
		l_pOutputMatrixMeanCoherence->setDimensionSize(1,1); // Compute the mean so only one value

		l_pOutputMatrixCoherenceSpectrum->setDimensionCount(2); // the output matrix will have 2 dimensions
		l_pOutputMatrixCoherenceSpectrum->setDimensionSize(0,l_ui32PairsCount);
		l_pOutputMatrixCoherenceSpectrum->setDimensionSize(1, l_ui32SegmentsLength);

		l_pFrequencyVector->setDimensionCount(2);
		l_pFrequencyVector->setDimensionSize(0,2);
		l_pFrequencyVector->setDimensionSize(1, l_ui32SegmentsLength); // the frequency vector is a matrix of size 2*l_ui32SegmentsLength to match the spectrum encoder requirements


		// Setting name of output channels for visualization
		for(uint32 i=0;i<l_ui32PairsCount;i++)
		{
			const uint32 l_ui32Index=2*i;
			CString l_name1 = l_pInputMatrix1->getDimensionLabel(0,(uint32)l_pChannelBuffer[l_ui32Index+0]);
			CString l_name2 = l_pInputMatrix2->getDimensionLabel(0,(uint32)l_pChannelBuffer[l_ui32Index+1]);
			CString l_name = l_name1+" "+l_name2;

			l_pOutputMatrixMeanCoherence->setDimensionLabel(0,i,l_name);
			l_pOutputMatrixCoherenceSpectrum->setDimensionLabel(0,i,l_name);
		}

		// Create frequency vector for the spectrum encoder
		float64* l_pFrequencyVectorBuffer = l_pFrequencyVector->getBuffer(); 
		for(uint32 i = 0; i<l_ui32SegmentsLength; i++)
		{
			const float64 l_f64FrequencyBandStart = i * l_ui64SamplingRate/2.0/l_ui32SegmentsLength;
			const float64 l_f64FrequencyBandStop = (i+1) * l_ui64SamplingRate/2.0/l_ui32SegmentsLength;
			l_pFrequencyVectorBuffer[2*i+0] = l_f64FrequencyBandStart;
			l_pFrequencyVectorBuffer[2*i+1] = l_f64FrequencyBandStop;
		}
	}

	if(this->isInputTriggerActive(OVP_Algorithm_Connectivity_InputTriggerId_Process))
	{
		const uint32 l_ui32NOverlap = l_ui32OverlapPercent*l_ui32SegmentsLength/100; // Convert percentage in samples

		uint32 l_ui32NbSegments = 0;
		float64 l_f64MeanCohere = 0;

		// Calculate number of segment on data set giving segment's length and overlap
		if(l_ui32NOverlap != 0)
		{
			l_ui32NbSegments = (l_ui32SamplesPerChannel1 - l_ui32SegmentsLength)/l_ui32NOverlap + 1;
		}
		else
		{
			l_ui32NbSegments = l_ui32SamplesPerChannel1/l_ui32SegmentsLength;
		}


		//_______________________________________________________________________________________
		//
		// Compute MSC for each pair
		//_______________________________________________________________________________________
		//

		for(uint32 channel = 0; channel < l_ui32PairsCount; channel++)
		{
			VectorXd l_vecXdChannelToCompare1(l_ui32SamplesPerChannel1);
			VectorXd l_vecXdChannelToCompare2(l_ui32SamplesPerChannel2);

			//_______________________________________________________________________________________
			//
			// Form pairs with the lookup matrix given
			//_______________________________________________________________________________________
			//

			const uint32 l_ui32ChannelIndex = 2*channel; //Index of a single channel
			const uint32 l_ui32Channel1 = (uint32)l_pChannelBuffer[l_ui32ChannelIndex+0];
			const uint32 l_ui32Channel2 = (uint32)l_pChannelBuffer[l_ui32ChannelIndex+1];

			for(uint32 sample = 0; sample < l_ui32SamplesPerChannel1; sample++)
			{
				l_vecXdChannelToCompare1(sample) = l_pMatrixBuffer1[sample+l_ui32Channel1*l_ui32SamplesPerChannel1];
			}
			for(uint32 sample = 0; sample < l_ui32SamplesPerChannel2; sample++)
			{
				l_vecXdChannelToCompare2(sample) = l_pMatrixBuffer2[sample+l_ui32Channel2*l_ui32SamplesPerChannel2];
			}

			// Remove the DC component of each channel. Without this we get really high coherences spread all over the 
			// spectrum when using e.g. the motor imagery dataset bundled with OpenViBE. @todo before or after windowing? here before.
			l_vecXdChannelToCompare1 = l_vecXdChannelToCompare1 - VectorXd::Ones(l_ui32SamplesPerChannel1) * l_vecXdChannelToCompare1.mean();
			l_vecXdChannelToCompare2 = l_vecXdChannelToCompare2 - VectorXd::Ones(l_ui32SamplesPerChannel2) * l_vecXdChannelToCompare2.mean();

			//	cout << "c1 " << l_vecXdChannelToCompare1 << "\n";
			//	cout << "c2 " << l_vecXdChannelToCompare2 << "\n";

			/* Compute MSC */
 			CAlgorithmMagnitudeSquaredCoherence::powerSpectralDensity(l_vecXdChannelToCompare1, m_vecXdPowerSpectrum1, m_vecXdWindow, l_ui32NbSegments, l_ui32SegmentsLength, l_ui32NOverlap);
 			// cout << "s1 " << m_vecXdPowerSpectrum1.transpose() << "\n";
  			CAlgorithmMagnitudeSquaredCoherence::powerSpectralDensity(l_vecXdChannelToCompare2, m_vecXdPowerSpectrum2, m_vecXdWindow, l_ui32NbSegments, l_ui32SegmentsLength, l_ui32NOverlap);
			// cout << "s2 " << m_vecXdPowerSpectrum2.transpose() << "\n";
			CAlgorithmMagnitudeSquaredCoherence::crossSpectralDensity(l_vecXdChannelToCompare1, l_vecXdChannelToCompare2, m_vecXcdCrossSpectrum, m_vecXdWindow, l_ui32NbSegments, l_ui32SegmentsLength, l_ui32NOverlap);
			// cout << "xs " << m_vecXcdCrossSpectrum.transpose() << "\n";

			const float64 l_f64Epsilon = 10e-3;	// discard bands with autospectral power smaller than this

			const VectorXd l_vecXdCoherenceNum = (m_vecXcdCrossSpectrum.real().cwiseProduct(m_vecXcdCrossSpectrum.conjugate())).real();
			const VectorXd l_vecXdCoherenceDen = m_vecXdPowerSpectrum1.cwiseProduct(m_vecXdPowerSpectrum2);
			const VectorXd l_vecXdCoherence = l_vecXdCoherenceNum.cwiseQuotient(l_vecXdCoherenceDen);

			// cout << "vCohNum: " << l_vecXdCoherenceNum.transpose() << "\n";
			// cout << "vCohDen: " << l_vecXdCoherenceDen.transpose() << "\n";
			// cout << "vCoh: " << l_vecXdCoherence.transpose() << "\n";

			uint32 l_ui32NumValids = 0;
			for (int32 i = 0; i<l_vecXdCoherence.size()/2; i++) // skip symmetric part
			{
				// Write coherence to output
				float64 val = l_vecXdCoherence(i);
				// if( (m_vecXdPowerSpectrum1(i)<l_f64Epsilon || m_vecXdPowerSpectrum2(i)<l_f64Epsilon)
				if( (l_vecXdCoherenceDen(i)<l_f64Epsilon)
					|| val!=val) 
				{
					// Set to zero if both channels have very low autospectral power or if the result is NaN 
					val = 0;
				}
				else 
				{
					// Only include valid results in the mean
					l_ui32NumValids++;
				}

				l_pOutputMatrixCoherenceSpectrum->getBuffer()[i*2+0+channel*l_vecXdCoherence.size()] = val;
				l_pOutputMatrixCoherenceSpectrum->getBuffer()[i*2+1+channel*l_vecXdCoherence.size()] = val;

				// Compute MSC mean over frequencies
				l_f64MeanCohere += val;

			}

			// Write coherence mean over valid frequencies to output. 
			// The validity test might not matter much for real data that may have power on all bands, but its critical for artificial examples.
			if(l_ui32NumValids) 
			{
				l_pOutputMatrixMeanCoherence->getBuffer()[channel] = l_f64MeanCohere/l_ui32NumValids;
			} 
			else 
			{
				l_pOutputMatrixMeanCoherence->getBuffer()[channel] = 0;
			}
			// cout << "Coh " << l_f64MeanCohere << "\n";
		}
		this->activateOutputTrigger(OVP_Algorithm_Connectivity_OutputTriggerId_ProcessDone, true);
	}

	return true;
}
#endif //TARGET_HAS_ThirdPartyEIGEN
