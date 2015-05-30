#include "ovpCTopographicMap2DDisplay.h"
#include "../algorithms/ovpCAlgorithmSphericalSplineInterpolation.h"
#include <cstdlib>
#include <cmath>
#include <memory.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

CTopographicMap2DDisplay::CTopographicMap2DDisplay(void) :
	m_pSphericalSplineInterpolation(NULL),
	m_pTopographicMapDatabase(NULL),
	m_pTopographicMap2DView(NULL)
{
}

uint64 CTopographicMap2DDisplay::getClockFrequency(void)
{
	return ((uint64)1LL)<<37;
}

boolean CTopographicMap2DDisplay::initialize(void)
{
	m_bFirstBufferReceived=false;
	m_pDecoder = new OpenViBEToolkit::TStreamedMatrixDecoder < CTopographicMap2DDisplay >;
	m_pDecoder->initialize(*this,0);

	m_pSphericalSplineInterpolation = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_SphericalSplineInterpolation));
	m_pSphericalSplineInterpolation->initialize();

	//create topographic map database
	m_pTopographicMapDatabase = new CTopographicMapDatabase(*this, *m_pSphericalSplineInterpolation);

	//retrieve settings
	CString l_sInterpolationModeSettingValue;
	getStaticBoxContext().getSettingValue(0, l_sInterpolationModeSettingValue);
	CString l_sDelaySettingValue;
	getStaticBoxContext().getSettingValue(1, l_sDelaySettingValue);

	//create topographic map view (handling GUI interaction)
	m_pTopographicMap2DView = new CTopographicMap2DView(
		*m_pTopographicMapDatabase,
		getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_SphericalLinearInterpolationType, l_sInterpolationModeSettingValue),
		atof(l_sDelaySettingValue));

	//have database notify us when new data is available
	m_pTopographicMapDatabase->setDrawable(m_pTopographicMap2DView);
	//ask not to be notified when new data is available (refresh is handled separately)
	m_pTopographicMapDatabase->setRedrawOnNewData(false);

	//send widget pointers to visualisation context for parenting
	::GtkWidget* l_pWidget=NULL;
	::GtkWidget* l_pToolbarWidget=NULL;
	dynamic_cast<CTopographicMap2DView*>(m_pTopographicMap2DView)->getWidgets(l_pWidget, l_pToolbarWidget);
	getBoxAlgorithmContext()->getVisualisationContext()->setWidget(l_pWidget);
	if(l_pToolbarWidget != NULL)
	{
		getBoxAlgorithmContext()->getVisualisationContext()->setToolbar(l_pToolbarWidget);
	}

	return true;
}

boolean CTopographicMap2DDisplay::uninitialize(void)
{
	if(m_pDecoder)
	{
		m_pDecoder->uninitialize();
		delete m_pDecoder;
	}

	delete m_pTopographicMap2DView;
	m_pTopographicMap2DView = NULL;
	delete m_pTopographicMapDatabase;
	m_pTopographicMapDatabase = NULL;

	m_pSphericalSplineInterpolation->uninitialize();

	getAlgorithmManager().releaseAlgorithm(*m_pSphericalSplineInterpolation);

	return true;
}

boolean CTopographicMap2DDisplay::processInput(OpenViBE::uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CTopographicMap2DDisplay::processClock(IMessageClock& rMessageClock)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CTopographicMap2DDisplay::process(void)
{
	IDynamicBoxContext* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();
	uint32 i;

	//decode signal data
	for(i=0; i<l_pDynamicBoxContext->getInputChunkCount(0); i++)
	{
		m_pDecoder->decode(i);
		if(m_pDecoder->isBufferReceived())
		{
			IMatrix* l_pInputMatrix=m_pDecoder->getOutputMatrix();

			//do we need to recopy this for each chunk?
			if(!m_bFirstBufferReceived)
			{
				m_pTopographicMapDatabase->setMatrixDimensionCount(l_pInputMatrix->getDimensionCount());
				for(uint32 dimension=0; dimension<l_pInputMatrix->getDimensionCount(); dimension++)
				{
					m_pTopographicMapDatabase->setMatrixDimensionSize(dimension, l_pInputMatrix->getDimensionSize(dimension));
					for(uint32 entryIndex=0; entryIndex<l_pInputMatrix->getDimensionSize(dimension); entryIndex++)
					{
						m_pTopographicMapDatabase->setMatrixDimensionLabel(dimension, entryIndex, l_pInputMatrix->getDimensionLabel(dimension, entryIndex));
					}
				}
				m_bFirstBufferReceived=true;
			}
			//

			if(!m_pTopographicMapDatabase->setMatrixBuffer(l_pInputMatrix->getBuffer(), l_pDynamicBoxContext->getInputChunkStartTime(0,i), l_pDynamicBoxContext->getInputChunkEndTime(0,i)))
			{
				return false;
			}

		}
	}

	//decode channel localisation data
	for(i=0; i<l_pDynamicBoxContext->getInputChunkCount(1); i++)
	{
		const IMemoryBuffer* l_pBuf = l_pDynamicBoxContext->getInputChunk(1, i);
		m_pTopographicMapDatabase->decodeChannelLocalisationMemoryBuffer(
			l_pBuf,
			l_pDynamicBoxContext->getInputChunkStartTime(1, i),
			l_pDynamicBoxContext->getInputChunkEndTime(1, i));
		l_pDynamicBoxContext->markInputAsDeprecated(1, i);
	}

	boolean l_bProcessValues = m_pTopographicMapDatabase->processValues();

	//disable plugin upon errors
	return l_bProcessValues;
}
