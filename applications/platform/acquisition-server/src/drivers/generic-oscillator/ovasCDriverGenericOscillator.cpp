#include "ovasCDriverGenericOscillator.h"
#include "ovasCConfigurationDriverGenericOscillator.h"

#include <toolkit/ovtk_all.h>

#include <system/ovCTime.h>

#include <cmath>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;

//___________________________________________________________________//
//                                                                   //

CDriverGenericOscillator::CDriverGenericOscillator(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_oSettings("AcquisitionServer_Driver_GenericOscillator", m_rDriverContext.getConfigurationManager())
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pSample(NULL)
	,m_ui32TotalSampleCount(0)
	,m_ui32StartTime(0)
	,m_bSendPeriodicStimulations(false)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::CDriverGenericOscillator\n";

	m_oHeader.setSamplingFrequency(512);
	m_oHeader.setChannelCount(4);
	for(uint32 i=0;i<4;i++)
	{
		std::stringstream ss; ss << "Oscillator " << (i+1);

		m_oHeader.setChannelUnits(i, OVTK_UNIT_Volts, OVTK_FACTOR_Base);
		m_oHeader.setChannelName(i, ss.str().c_str());
	}

	m_oSettings.add("Header", &m_oHeader);
	m_oSettings.add("SendPeriodicStimulations", &m_bSendPeriodicStimulations);
	m_oSettings.load();
}

void CDriverGenericOscillator::release(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::release\n";

	delete this;
}

const char* CDriverGenericOscillator::getName(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::getName\n";

	return "Generic Oscillator";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverGenericOscillator::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::initialize\n";

	if(m_rDriverContext.isConnected()) { return false; }

	if(!m_oHeader.isChannelCountSet()
	 ||!m_oHeader.isSamplingFrequencySet())
	{
		return false;
	}

	m_pSample=new float32[m_oHeader.getChannelCount()*ui32SampleCountPerSentBlock];
	if(!m_pSample)
	{
		delete [] m_pSample;
		m_pSample=NULL;
		return false;
	}

	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;

	return true;
}

boolean CDriverGenericOscillator::start(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::start\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

	m_ui32TotalSampleCount=0;
	m_ui32StartTime=System::Time::getTime();

	return true;
}

boolean CDriverGenericOscillator::loop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Debug << "CDriverGenericOscillator::loop\n";

	if(!m_rDriverContext.isConnected()) { return false; }

	if(m_rDriverContext.isStarted())
	{
		uint32 l_ui32CurrentTime=System::Time::getTime();

		if(l_ui32CurrentTime-m_ui32StartTime > (1000*(m_ui32TotalSampleCount+m_ui32SampleCountPerSentBlock))/m_oHeader.getSamplingFrequency())
		{
			CStimulationSet l_oStimulationSet;
			if (m_bSendPeriodicStimulations)
			{
				l_oStimulationSet.setStimulationCount(1);
				l_oStimulationSet.setStimulationIdentifier(0, 0);
				l_oStimulationSet.setStimulationDate(0, 0);
				l_oStimulationSet.setStimulationDuration(0, 0);
			}

			for(uint32 j=0; j<m_oHeader.getChannelCount(); j++)
			{
				for(uint32 i=0; i<m_ui32SampleCountPerSentBlock; i++)
				{
#if 1
					float64 l_f64Value=
						::sin(((i+m_ui32TotalSampleCount)*(j+1)*12.3)/m_oHeader.getSamplingFrequency())+
						::sin(((i+m_ui32TotalSampleCount)*(j+1)* 4.5)/m_oHeader.getSamplingFrequency())+
						::sin(((i+m_ui32TotalSampleCount)*(j+1)*67.8)/m_oHeader.getSamplingFrequency());
					m_pSample[j*m_ui32SampleCountPerSentBlock+i]=(float32)l_f64Value;
#else
					m_pSample[j*m_ui32SampleCountPerSentBlock+i]=j;
#endif
				}
			}

			m_ui32TotalSampleCount+=m_ui32SampleCountPerSentBlock;
			m_pCallback->setSamples(m_pSample);
			m_pCallback->setStimulationSet(l_oStimulationSet);
			m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());
		}
	}
	else
	{
		if(m_rDriverContext.isImpedanceCheckRequested())
		{
			for(uint32 j=0; j<m_oHeader.getChannelCount(); j++)
			{
				m_rDriverContext.updateImpedance(j, 1);
			}
		}
	}

	return true;
}

boolean CDriverGenericOscillator::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::stop\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }
	return true;
}

boolean CDriverGenericOscillator::uninitialize(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::uninitialize\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

	delete [] m_pSample;
	m_pSample=NULL;
	m_pCallback=NULL;

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverGenericOscillator::isConfigurable(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::isConfigurable\n";

	return true;
}

boolean CDriverGenericOscillator::configure(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "CDriverGenericOscillator::configure\n";

	CConfigurationDriverGenericOscillator m_oConfiguration(m_rDriverContext, OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-Generic-Oscillator.ui", 
		m_bSendPeriodicStimulations);

	if(m_oConfiguration.configure(m_oHeader)) 
	{
		m_oSettings.save();
		return true;
	}

	return false;
}
