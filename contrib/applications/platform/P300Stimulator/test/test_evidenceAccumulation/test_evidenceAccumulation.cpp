
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "../../src/evidence-accumulation/ovexP300CEvidenceAccumulator.h"

#include "../../src/sequence/ovexP300RipRandSequenceGenerator.h"
#include "../../src/sequence/ovexP300RowColumnSequenceGenerator.h"
#include "../../src/sequence/ovexP300CSVReader.h"

#include <iostream>
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <system/ovCTime.h>

#include <openvibe/ovITimeArithmetics.h>

#include "../../src/properties/ovexP300InterfacePropertyReader.h"
#include "../../src/properties/ovexP300StimulatorPropertyReader.h"
#include "../../src/properties/ovexP300ScreenLayoutReader.h"

#include "../../src/ovexP300SequenceFileWriter.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

using namespace std;


class test_evidenceAccumulation
{
	public:

	test_evidenceAccumulation()
	{
		initializeOpenViBEKernel();
		//this will read all configuration files, interface-properties.xml, stimulator-properties.xml and the keyboard layout in share/openvibe/applications/externalP300Stimulator/
		CString l_sPathRoot = m_pKernelContext->getConfigurationManager().expand(CString("${Path_Root}"));
		this->m_pInterfacePropReader = new P300InterfacePropertyReader(this->m_pKernelContext);
		this->m_pInterfacePropReader->readPropertiesFromFile(l_sPathRoot + "/share/openvibe/applications/CoAdaptP300Stimulator/interface-properties.xml");
		this->m_pScreenLayoutReader = new P300ScreenLayoutReader(this->m_pKernelContext);
		this->m_pScreenLayoutReader->readPropertiesFromFile(this->m_pInterfacePropReader->getScreenDefinitionFile());
		this->m_pStimulatorPropReader = new P300StimulatorPropertyReader(this->m_pKernelContext, this->m_pScreenLayoutReader->getSymbolList());
		this->m_pStimulatorPropReader->readPropertiesFromFile(this->m_pInterfacePropReader->getStimulatorConfigFile());


		if(this->m_pStimulatorPropReader->getEarlyStopping())
			m_pKernelContext->getLogManager() << LogLevel_Info << "Early Stop true\n";
		else
			m_pKernelContext->getLogManager() << LogLevel_Info << "Early Stop false\n";

		m_pSequenceWriter = new P300SequenceFileWriter(this->m_pInterfacePropReader->getFlashGroupDefinitionFile());

		//sequence generator
		if (this->m_pInterfacePropReader->getFlashMode()==CString("rowcol"))
			m_pSequenceGenerator = new P300RowColumnSequenceGenerator(
				this->m_pScreenLayoutReader->getNumberOfKeys(),
				this->m_pStimulatorPropReader->getNumberOfGroups(),
				this->m_pStimulatorPropReader->getNumberOfRepetitions());
		else if(this->m_pInterfacePropReader->getFlashMode()==CString("file"))
			m_pSequenceGenerator = new ovexP300CSVReader(
						this->m_pScreenLayoutReader->getNumberOfKeys(),
						this->m_pStimulatorPropReader->getNumberOfGroups(),
						this->m_pStimulatorPropReader->getNumberOfRepetitions());
		else
			m_pSequenceGenerator = new P300RipRandSequenceGenerator(
				this->m_pScreenLayoutReader->getNumberOfKeys(),
				this->m_pStimulatorPropReader->getNumberOfGroups(),
				this->m_pStimulatorPropReader->getNumberOfRepetitions());

		//register the file write with the sequence generator
		m_pSequenceGenerator->setSequenceWriter(m_pSequenceWriter);

	}
		void initializeOpenViBEKernel();


	public:
		/**
		 * OpenViBE's kernel context
		 */
		OpenViBE::Kernel::IKernelContext* m_pKernelContext;

		/**
		 * ExternalP300PropertyReader that reads the interface-properties file in share/openvibe/applications/externalP300Stimulator/
		 */
		P300InterfacePropertyReader* m_pInterfacePropReader;

		/**
		 * ExternalP300PropertyReader that reads one of the keyboard layout files such as 5by10grid-abc-gray.xml in share/openvibe/applications/externalP300Stimulator/
		 */
		P300ScreenLayoutReader* m_pScreenLayoutReader;

		/**
		 * ExternalP300PropertyReader that reads the stimulator-properties file in share/openvibe/applications/externalP300Stimulator/
		 */
		P300StimulatorPropertyReader* m_pStimulatorPropReader;

		P300SequenceGenerator* m_pSequenceGenerator;

		P300SequenceWriter* m_pSequenceWriter;

};

void test_evidenceAccumulation::initializeOpenViBEKernel()
{
	CKernelLoader l_oKernelLoader;

	cout<<"[  INF  ] Created kernel loader, trying to load kernel module"<<"\n";
	CString m_sError;
	#if defined TARGET_OS_Windows
	if(!l_oKernelLoader.load(OpenViBE::Directories::getLibDir() + "/openvibe-kernel.dll", &m_sError))
	#elif defined TARGET_OS_Linux
	if(!l_oKernelLoader.load(OpenViBE::Directories::getLibDir() + "/libopenvibe-kernel.so", &m_sError))
	#endif
	{
			cout<<"[ FAILED ] Error loading kernel ("<<m_sError<<")"<<"\n";
	}
	else
	{
		cout<<"[  INF  ] Kernel module loaded, trying to get kernel descriptor"<<"\n";
		IKernelDesc* l_pKernelDesc=NULL;
		m_pKernelContext=NULL;
		l_oKernelLoader.initialize();
		l_oKernelLoader.getKernelDesc(l_pKernelDesc);
		if(!l_pKernelDesc)
		{
			cout<<"[ FAILED ] No kernel descriptor"<<"\n";
		}
		else
		{
			cout<<"[  INF  ] Got kernel descriptor, trying to create kernel"<<"\n";
			m_pKernelContext=l_pKernelDesc->createKernel("externalP300Stimulator", OpenViBE::Directories::getDataDir() + "/kernel/openvibe.conf");
			if(!m_pKernelContext)
			{
				cout<<"[ FAILED ] No kernel created by kernel descriptor"<<"\n";
			}
			else
			{
				OpenViBEToolkit::initialize(*m_pKernelContext);
			}
		}
	}
}



int main(int argc, char *argv[])
{
	test_evidenceAccumulation* l_oTest = new test_evidenceAccumulation();

	CoAdaptP300CEvidenceAccumulator* l_oEvidenceAccumulator = new CoAdaptP300CEvidenceAccumulator(l_oTest->m_pStimulatorPropReader, l_oTest->m_pSequenceGenerator);
	l_oTest->m_pSequenceGenerator->generateSequence();


	boolean l_bQuit=false;

	uint64 l_ui64TimeDataReceived = System::Time::zgetTime();

	while(!l_bQuit)
	{
		boolean l_bUpdated = l_oEvidenceAccumulator->update();
		if(l_bUpdated)
		{
			l_ui64TimeDataReceived = System::Time::zgetTime();
		}
		uint64 l_ui64Prediction = l_oEvidenceAccumulator->getPrediction();
		if(l_ui64Prediction!=0)
		{
			l_oTest->m_pKernelContext->getLogManager() << LogLevel_Info << "Predicted " << l_ui64Prediction << "\n";
			l_oEvidenceAccumulator->flushEvidence();
			l_oTest->m_pSequenceGenerator->generateSequence();
		}

		uint64 l_ui64Time = System::Time::zgetTime();
		if(ITimeArithmetics::timeToSeconds(l_ui64Time-l_ui64TimeDataReceived)>15)
		{
			l_oTest->m_pKernelContext->getLogManager() << LogLevel_Info << "Timeout " << ITimeArithmetics::timeToSeconds(l_ui64Time-l_ui64TimeDataReceived) << "\n";
			l_bQuit=true;
		}


	}
	
	

}

#else

#include <iostream>
int main(int argc, char *argv[])
{
	std::cout << "Compiler did not have the required libraries for the CoAdapt stimulator." << std::endl;
	return 1;
}
#endif
