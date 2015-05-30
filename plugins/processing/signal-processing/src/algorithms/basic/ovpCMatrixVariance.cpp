#if defined(TARGET_HAS_ThirdPartyITPP)

#include "ovpCMatrixVariance.h"

#include <system/ovCMemory.h>
#include <cmath>
#include <iostream>

// the boost version used at the moment of writing this caused 4800 by internal call to "int _isnan" in a bool-returning function.
#if defined(WIN32)
  #pragma warning (disable : 4800)
#endif

#include <boost/math/distributions/students_t.hpp>
#include <itpp/base/vec.h>
#include <itpp/base/math/elem_math.h>
#include <itpp/base/math/min_max.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

using namespace boost::math;

// ________________________________________________________________________________________________________________
//

boolean CMatrixVariance::initialize(void)
{
	ip_ui64AveragingMethod.initialize(getInputParameter(OVP_Algorithm_MatrixVariance_InputParameterId_AveragingMethod));
	ip_ui64MatrixCount.initialize(getInputParameter(OVP_Algorithm_MatrixVariance_InputParameterId_MatrixCount));
	ip_f64SignificanceLevel.initialize(getInputParameter(OVP_Algorithm_MatrixVariance_InputParameterId_SignificanceLevel));
	ip_pMatrix.initialize(getInputParameter(OVP_Algorithm_MatrixVariance_InputParameterId_Matrix));
	op_pAveragedMatrix.initialize(getOutputParameter(OVP_Algorithm_MatrixVariance_OutputParameterId_AveragedMatrix));
	op_pMatrixVariance.initialize(getOutputParameter(OVP_Algorithm_MatrixVariance_OutputParameterId_Variance));
	op_pConfidenceBound.initialize(getOutputParameter(OVP_Algorithm_MatrixVariance_OutputParameterId_ConfidenceBound));

	return true;
}

boolean CMatrixVariance::uninitialize(void)
{
	std::deque < Vec<float64>* >::iterator it;
	for(it=m_vHistory.begin(); it!=m_vHistory.end(); it++)
	{
		delete *it;
	}
	m_vHistory.clear();

	op_pAveragedMatrix.uninitialize();
	op_pMatrixVariance.uninitialize();
	op_pConfidenceBound.uninitialize();
	ip_pMatrix.uninitialize();
	ip_ui64MatrixCount.uninitialize();
	ip_ui64AveragingMethod.uninitialize();
	ip_f64SignificanceLevel.uninitialize();

	return true;
}

// ________________________________________________________________________________________________________________
//

boolean CMatrixVariance::process(void)
{
	IMatrix* l_pInputMatrix=ip_pMatrix;
	//IMatrix* l_pOutputMatrix=op_pAveragedMatrix;

	boolean l_bShouldPerformAverage=false;

	if(this->isInputTriggerActive(OVP_Algorithm_MatrixVariance_InputTriggerId_Reset))
	{
		m_vMean.set_size(ip_pMatrix->getBufferElementCount()); m_vMean.zeros();
		m_vM.set_size(ip_pMatrix->getBufferElementCount()); m_vM.zeros();
		m_f64Variance.set_size(ip_pMatrix->getBufferElementCount()); m_f64Variance.zeros(); 
		m_ui32InputCounter = 0;

		std::deque < Vec<float64>* >::iterator it;
		for(it=m_vHistory.begin(); it!=m_vHistory.end(); it++)
		{
			delete *it;
		}
		m_vHistory.clear();

		OpenViBEToolkit::Tools::Matrix::copyDescription(*op_pAveragedMatrix, *l_pInputMatrix);
		OpenViBEToolkit::Tools::Matrix::copyDescription(*op_pMatrixVariance, *l_pInputMatrix);
		OpenViBEToolkit::Tools::Matrix::copyDescription(*op_pConfidenceBound, *l_pInputMatrix);
	}

	if(this->isInputTriggerActive(OVP_Algorithm_MatrixVariance_InputTriggerId_FeedMatrix))
	{
		if(ip_ui64AveragingMethod==OVP_TypeId_EpochAverageMethod_MovingAverage.toUInteger())
		{
			//CMatrix* l_pSwapMatrix=NULL;

			if(m_vHistory.size()>=ip_ui64MatrixCount)
			{
				delete m_vHistory.front();
				m_vHistory.pop_front();
			}
			/*else
			{
				l_pSwapMatrix=new CMatrix();
				OpenViBEToolkit::Tools::Matrix::copyDescription(*l_pSwapMatrix, *l_pInputMatrix);
			}*/

			//OpenViBEToolkit::Tools::Matrix::copyContent(*l_pSwapMatrix, *l_pInputMatrix);

			Vec<float64>* l_vMatrix = new Vec<float64>(l_pInputMatrix->getBuffer(),l_pInputMatrix->getBufferElementCount());
			m_vHistory.push_back(l_vMatrix);
			l_bShouldPerformAverage=(m_vHistory.size()==ip_ui64MatrixCount);
		}
		else if(ip_ui64AveragingMethod==OVP_TypeId_EpochAverageMethod_MovingAverageImmediate.toUInteger())
		{
			//CMatrix* l_pSwapMatrix=NULL;

			if(m_vHistory.size()>=ip_ui64MatrixCount)
			{
				delete m_vHistory.front();
				m_vHistory.pop_front();
			}
			/*else
			{
				l_pSwapMatrix=new CMatrix();
				OpenViBEToolkit::Tools::Matrix::copyDescription(*l_pSwapMatrix, *l_pInputMatrix);
			}*/

			//OpenViBEToolkit::Tools::Matrix::copyContent(*l_pSwapMatrix, *l_pInputMatrix);

			Vec<float64>* l_vMatrix = new Vec<float64>(l_pInputMatrix->getBuffer(),l_pInputMatrix->getBufferElementCount());
			m_vHistory.push_back(l_vMatrix);
			l_bShouldPerformAverage=(m_vHistory.size()>0);
		}
		else if(ip_ui64AveragingMethod==OVP_TypeId_EpochAverageMethod_BlockAverage.toUInteger())
		{
			//CMatrix* l_pSwapMatrix=new CMatrix();

			if(m_vHistory.size()>=ip_ui64MatrixCount)
			{
				std::deque < Vec<float64>* >::iterator it;
				for(it=m_vHistory.begin(); it!=m_vHistory.end(); it++)
				{
					delete *it;
				}
				m_vHistory.clear();
			}

			//OpenViBEToolkit::Tools::Matrix::copyDescription(*l_pSwapMatrix, *l_pInputMatrix);
			//OpenViBEToolkit::Tools::Matrix::copyContent(*l_pSwapMatrix, *l_pInputMatrix);

			Vec<float64>* l_vMatrix = new Vec<float64>(l_pInputMatrix->getBuffer(),l_pInputMatrix->getBufferElementCount());
			m_vHistory.push_back(l_vMatrix);
			l_bShouldPerformAverage=(m_vHistory.size()==ip_ui64MatrixCount);
		}
		else if(ip_ui64AveragingMethod==OVP_TypeId_EpochAverageMethod_CumulativeAverage.toUInteger())
		{
			if(!m_vHistory.empty())
			{
				//std::cout << "size of history " << m_vHistory.size() << "\n";
				delete m_vHistory.front();
				m_vHistory.pop_front();
			}
			/*else
				std::cout << "history empty \n";*/
			//CMatrix* l_pSwapMatrix=new CMatrix();

			//OpenViBEToolkit::Tools::Matrix::copyDescription(*l_pSwapMatrix, *l_pInputMatrix);
			//OpenViBEToolkit::Tools::Matrix::copyContent(*l_pSwapMatrix, *l_pInputMatrix);

			Vec<float64>* l_vMatrix = new Vec<float64>(l_pInputMatrix->getBuffer(),l_pInputMatrix->getBufferElementCount());
			m_vHistory.push_back(l_vMatrix);
			l_bShouldPerformAverage=(m_vHistory.size()!=0);
		}
		else
		{
			l_bShouldPerformAverage=false;
		}
	}

	if(l_bShouldPerformAverage)
	{
		if(m_vHistory.size()!=0)
		{
			std::deque < Vec<float64>* >::iterator it;
			students_t_distribution<double> l_oStudentDistribution(2);
			if(ip_ui64AveragingMethod==OVP_TypeId_EpochAverageMethod_CumulativeAverage.toUInteger())
			{
				//incremental estimation of mean and variance
				for(it=m_vHistory.begin(); it!=m_vHistory.end(); it++)
				{
					m_ui32InputCounter++;
					Vec<float64> l_vInputBuffer = **it;
					Vec<float64> delta = l_vInputBuffer - m_vMean;
					m_vMean += delta/(float64)m_ui32InputCounter;
					m_vM +=  elem_mult(delta, (l_vInputBuffer - m_vMean));
					if (m_ui32InputCounter>1)
						m_f64Variance =  m_vM/(float64)(m_ui32InputCounter-1);
					
				}
				l_oStudentDistribution = students_t_distribution<double>(m_ui32InputCounter<=1?1:m_ui32InputCounter-1);
				//CMatrix l_mSwapMatrix();
				//getLogManager() << LogLevel_Info << "Variance first element " << m_f64Variance[0] << ", last element " << m_f64Variance[l_pInputMatrix->getBufferElementCount()-1] << "\n";

				System::Memory::copy(op_pAveragedMatrix->getBuffer(), m_vMean._data(), l_pInputMatrix->getBufferElementCount()*sizeof(float64));
				System::Memory::copy(op_pMatrixVariance->getBuffer(), m_f64Variance._data(), l_pInputMatrix->getBufferElementCount()*sizeof(float64));
			}
			else
			{
				l_oStudentDistribution = students_t_distribution<double>((double)ip_ui64MatrixCount-1);

				OpenViBEToolkit::Tools::Matrix::clearContent(*op_pMatrixVariance);
				OpenViBEToolkit::Tools::Matrix::clearContent(*op_pAveragedMatrix);

				uint32 l_ui32Count=op_pAveragedMatrix->getBufferElementCount();
				float64 l_f64Scale=1./m_vHistory.size();

				std::deque < Vec<float64>* >::iterator it;
				for(it=m_vHistory.begin(); it!=m_vHistory.end(); it++)
				{
					//batch computation of mean
					float64* l_pOutputAverageMatrixBuffer=op_pAveragedMatrix->getBuffer();
					Vec<float64> l_pInputMatrixBuffer=**it;
					for(uint32 i=0; i<l_ui32Count; i++)
					{
						*l_pOutputAverageMatrixBuffer+=l_pInputMatrixBuffer[i]*l_f64Scale;
						l_pOutputAverageMatrixBuffer++;
					}
					//batch computation of variance
					l_pOutputAverageMatrixBuffer=op_pAveragedMatrix->getBuffer();
					float64* l_pOutputMatrixVarianceBuffer=op_pMatrixVariance->getBuffer();
					for(uint32 i=0; i<l_ui32Count; i++)
					{
						*l_pOutputMatrixVarianceBuffer+=(l_pInputMatrixBuffer[i]-*(l_pOutputAverageMatrixBuffer+i))*(l_pInputMatrixBuffer[i]-*(l_pOutputAverageMatrixBuffer+i))/(m_vHistory.size()-1.0f);
						l_pOutputMatrixVarianceBuffer++;
					}
				}
				m_f64Variance = Vec<float64>(op_pAveragedMatrix->getBuffer(), l_ui32Count);
			}

			//computing confidence bounds
			float32 l_f32Quantile = (float32)quantile(complement(l_oStudentDistribution,ip_f64SignificanceLevel/2.0f));
			getLogManager() << LogLevel_Debug << "Quantile at " <<  ip_f64SignificanceLevel << " is " << l_f32Quantile << "\n";
			Vec<float64> l_f64Bound;
			if(ip_ui64AveragingMethod==OVP_TypeId_EpochAverageMethod_CumulativeAverage.toUInteger())
				l_f64Bound = (l_f32Quantile/sqrt((float64)m_ui32InputCounter))*itpp::sqrt(m_f64Variance);
			else
				l_f64Bound = (l_f32Quantile/(float64)ip_ui64MatrixCount)*itpp::sqrt(m_f64Variance);
			System::Memory::copy(op_pConfidenceBound->getBuffer(), l_f64Bound._data(), l_pInputMatrix->getBufferElementCount()*sizeof(float64));
				
		}

		this->activateOutputTrigger(OVP_Algorithm_MatrixVariance_OutputTriggerId_AveragePerformed, true);
	}

	return true;
}

#endif