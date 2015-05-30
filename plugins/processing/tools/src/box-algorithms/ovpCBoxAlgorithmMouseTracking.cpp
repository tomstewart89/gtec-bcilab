#if defined(TARGET_HAS_ThirdPartyGTK)

#include "ovpCBoxAlgorithmMouseTracking.h"

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Tools;
using namespace OpenViBEToolkit;


void motion_event_handler(GtkWidget* widget, GdkEventMotion* event, gpointer data);

boolean CBoxAlgorithmMouseTracking::initialize(void)
{
	// Feature vector stream encoder
	m_oAlgo0_SignalEncoder.initialize(*this,0);
	// Feature vector stream encoder
	m_oAlgo1_SignalEncoder.initialize(*this,1);

	m_oAlgo0_SignalEncoder.getInputMatrix().setReferenceTarget(m_pAbsoluteCoordinateBuffer);
	m_oAlgo1_SignalEncoder.getInputMatrix().setReferenceTarget(m_pRelativeCoordinateBuffer);

	// Allocates coordinates Matrix
	m_pAbsoluteCoordinateBuffer=new CMatrix();
	m_pRelativeCoordinateBuffer=new CMatrix();

	// Retrieves settings
	m_ui64Frequency = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_ui64GeneratedEpochSampleCount = FSettingValueAutoCast(*this->getBoxAlgorithmContext(),1);

	m_ui64SentSampleCount = 0;
	m_f64Mouse_x = 0;
	m_f64Mouse_y = 0;
	m_f64Previous_x = 0;
	m_f64Previous_y = 0;

	// Creates empty window to get mouse position
	m_pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_add_events(m_pWindow, GDK_POINTER_MOTION_MASK);
	gtk_window_maximize(GTK_WINDOW(m_pWindow));
	gtk_widget_show_all(m_pWindow);

	g_signal_connect(m_pWindow, "motion-notify-event", G_CALLBACK(motion_event_handler), this);

	m_bHeaderSent = false;
	
	m_oAlgo0_SignalEncoder.getInputSamplingRate() = m_ui64Frequency;
	m_oAlgo1_SignalEncoder.getInputSamplingRate() = m_ui64Frequency;

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmMouseTracking::uninitialize(void)
{
	m_oAlgo0_SignalEncoder.uninitialize();
	m_oAlgo1_SignalEncoder.uninitialize();

	delete m_pAbsoluteCoordinateBuffer;
	m_pAbsoluteCoordinateBuffer=NULL;

	delete m_pRelativeCoordinateBuffer;
	m_pRelativeCoordinateBuffer=NULL;

	gtk_widget_destroy(m_pWindow);
	m_pWindow = NULL;

	return true;
}
/*******************************************************************************/


boolean CBoxAlgorithmMouseTracking::processClock(IMessageClock& rMessageClock)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}
/*******************************************************************************/


uint64 CBoxAlgorithmMouseTracking::getClockFrequency(void)
{
	// Intentional parameter swap to get the frequency
	m_ui64ClockFrequency = ITimeArithmetics::sampleCountToTime(m_ui64GeneratedEpochSampleCount, m_ui64Frequency);
	
	return m_ui64ClockFrequency;
}

/*******************************************************************************/

boolean CBoxAlgorithmMouseTracking::process(void)
{

	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	
	//Send header and initialize Matrix
	if(!m_bHeaderSent)
	{
		m_pAbsoluteCoordinateBuffer->setDimensionCount(2);
		m_pAbsoluteCoordinateBuffer->setDimensionSize(0,2);
		m_pAbsoluteCoordinateBuffer->setDimensionSize(1, static_cast<uint32>(m_ui64GeneratedEpochSampleCount));
		m_pAbsoluteCoordinateBuffer->setDimensionLabel(0,0, "x");
		m_pAbsoluteCoordinateBuffer->setDimensionLabel(0,1, "y");

		m_pRelativeCoordinateBuffer->setDimensionCount(2);
		m_pRelativeCoordinateBuffer->setDimensionSize(0,2);
		m_pRelativeCoordinateBuffer->setDimensionSize(1, static_cast<uint32>(m_ui64GeneratedEpochSampleCount));
		m_pRelativeCoordinateBuffer->setDimensionLabel(0,0, "x");
		m_pRelativeCoordinateBuffer->setDimensionLabel(0,1, "y");


		m_oAlgo0_SignalEncoder.encodeHeader();
		m_oAlgo1_SignalEncoder.encodeHeader();

		m_bHeaderSent=true;

		l_rDynamicBoxContext.markOutputAsReadyToSend(0, 0, 0);
		l_rDynamicBoxContext.markOutputAsReadyToSend(1, 0, 0);
	}
	else //Do the process and send coordinates to output
	{
		uint64 l_ui64SentSampleCount=m_ui64SentSampleCount;


		for(uint64 i=0; i<m_ui64GeneratedEpochSampleCount; i++)
		{
			m_pAbsoluteCoordinateBuffer->getBuffer()[0*m_ui64GeneratedEpochSampleCount+i] = m_f64Mouse_x;
			m_pAbsoluteCoordinateBuffer->getBuffer()[1*m_ui64GeneratedEpochSampleCount+i] = m_f64Mouse_y;

			m_pRelativeCoordinateBuffer->getBuffer()[0*m_ui64GeneratedEpochSampleCount+i] = m_f64Mouse_x - m_f64Previous_x;
			m_pRelativeCoordinateBuffer->getBuffer()[1*m_ui64GeneratedEpochSampleCount+i] = m_f64Mouse_y - m_f64Previous_y;
		}


		m_f64Previous_x = m_f64Mouse_x;
		m_f64Previous_y = m_f64Mouse_y;

		m_ui64SentSampleCount+=m_ui64GeneratedEpochSampleCount;

		uint64 l_ui64StartTime = ITimeArithmetics::sampleCountToTime(m_ui64Frequency, l_ui64SentSampleCount);
		uint64 l_ui64EndTime = ITimeArithmetics::sampleCountToTime(m_ui64Frequency, m_ui64SentSampleCount);

		m_oAlgo0_SignalEncoder.encodeBuffer();
		m_oAlgo1_SignalEncoder.encodeBuffer();

		l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_ui64StartTime, l_ui64EndTime);
		l_rDynamicBoxContext.markOutputAsReadyToSend(1, l_ui64StartTime, l_ui64EndTime);
	}
	return true;
}

// CALLBACK
// Get mouse position
void motion_event_handler(GtkWidget* widget, GdkEventMotion* event, gpointer data)
{
	CBoxAlgorithmMouseTracking* l_pBox = static_cast < CBoxAlgorithmMouseTracking* >(data);
	l_pBox->m_f64Mouse_x = static_cast<float64>(event->x);
	l_pBox->m_f64Mouse_y = static_cast<float64>(event->y);
}



#endif
