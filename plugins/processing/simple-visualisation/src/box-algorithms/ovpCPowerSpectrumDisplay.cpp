#include "ovpCPowerSpectrumDisplay.h"

#include <cstdlib>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;
using namespace OpenViBEToolkit;

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{

		CPowerSpectrumDisplay::CPowerSpectrumDisplay() :
			m_pPowerSpectrumDisplayView(NULL),
			m_pPowerSpectrumDisplayDatabase(NULL)
		{
		}

		OpenViBE::boolean CPowerSpectrumDisplay::initialize()
		{
			m_oSpectrumDecoder.initialize(*this,0);

			m_pPowerSpectrumDisplayDatabase = new CPowerSpectrumDatabase(*this);

			//retrieve displayed frequency range settings
			CString l_sMinDisplayedFrequencySettingValue;
			CString l_sMaxDisplayedFrequencySettingValue;
			getStaticBoxContext().getSettingValue(0, l_sMinDisplayedFrequencySettingValue);
			getStaticBoxContext().getSettingValue(1, l_sMaxDisplayedFrequencySettingValue);

			m_pPowerSpectrumDisplayView = new CPowerSpectrumDisplayView(*m_pPowerSpectrumDisplayDatabase,
				atof(l_sMinDisplayedFrequencySettingValue), atof(l_sMaxDisplayedFrequencySettingValue));

			m_pPowerSpectrumDisplayDatabase->setDrawable(m_pPowerSpectrumDisplayView);

			//parent visualisation box in visualisation tree
			::GtkWidget* l_pWidget=NULL;
			::GtkWidget* l_pToolbarWidget=NULL;
			dynamic_cast<CPowerSpectrumDisplayView*>(m_pPowerSpectrumDisplayView)->getWidgets(l_pWidget, l_pToolbarWidget);
			getBoxAlgorithmContext()->getVisualisationContext()->setWidget(l_pWidget);
			if(l_pToolbarWidget != NULL)
			{
				getBoxAlgorithmContext()->getVisualisationContext()->setToolbar(l_pToolbarWidget);
			}
			return true;
		}

		OpenViBE::boolean CPowerSpectrumDisplay::uninitialize()
		{
			m_oSpectrumDecoder.uninitialize();

			delete m_pPowerSpectrumDisplayView;
			delete m_pPowerSpectrumDisplayDatabase;

			return true;
		}

		OpenViBE::boolean CPowerSpectrumDisplay::processInput(OpenViBE::uint32 ui32InputIndex)
		{
			getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
			return true;
		}

		OpenViBE::boolean CPowerSpectrumDisplay::process()
		{
			IDynamicBoxContext* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();

			for(uint32 i=0; i<l_pDynamicBoxContext->getInputChunkCount(0); i++)
			{
				m_oSpectrumDecoder.decode(i);

				if(m_oSpectrumDecoder.isHeaderReceived())
				{
					const IMatrix *l_pMatrix = m_oSpectrumDecoder.getOutputMatrix();

					m_pPowerSpectrumDisplayDatabase->setChannelCount(l_pMatrix->getDimensionSize(0));
					for(uint32 c=0;c<l_pMatrix->getDimensionSize(0);c++)
					{
						m_pPowerSpectrumDisplayDatabase->setChannelName(c, l_pMatrix->getDimensionLabel(0,c));
					}

					const IMatrix *l_pBandMatrix = m_oSpectrumDecoder.getOutputMinMaxFrequencyBands();
					m_pPowerSpectrumDisplayDatabase->setFrequencyBandCount(l_pMatrix->getDimensionSize(1));
					for(uint32 c=0;c<l_pMatrix->getDimensionSize(1);c++)
					{
						m_pPowerSpectrumDisplayDatabase->setFrequencyBandName(c, l_pMatrix->getDimensionLabel(1,c));
						m_pPowerSpectrumDisplayDatabase->setFrequencyBandStart(c, l_pBandMatrix->getBuffer()[c*2+0]);
						m_pPowerSpectrumDisplayDatabase->setFrequencyBandStop(c, l_pBandMatrix->getBuffer()[c*2+1]);
					}
				}

				if(m_oSpectrumDecoder.isBufferReceived())
				{
					const IMatrix *l_pMatrix = m_oSpectrumDecoder.getOutputMatrix();

					m_pPowerSpectrumDisplayDatabase->setBuffer(l_pMatrix->getBuffer());
				}
			}

			return true;
		}
	};
};
