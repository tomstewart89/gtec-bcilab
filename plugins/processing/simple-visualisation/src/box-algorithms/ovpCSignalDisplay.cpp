/*
 * Note: The signal display and its subclasses (SignalChannelDisplay, SignalDisplayView) 
 * were rehauled to give a better user experience for different types of signal. However, 
 * the code could likely use significant refactoring for clarity and maintainability. 
 * If this is done, care should be taken that the code does not break.
 */

#include "ovpCSignalDisplay.h"

#include <cmath>
#include <iostream>
#include <cstdlib>

#include <sstream>

#include <openvibe/ovITimeArithmetics.h>
#include <system/ovCTime.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

using namespace OpenViBEToolkit;

using namespace std;

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		/**
		* Constructor
		*/
		CSignalDisplay::CSignalDisplay(void)
			:
			 m_pStreamDecoder(NULL)
		    ,m_pSignalDisplayView(NULL)
			,m_pBufferDatabase(NULL)
		{
		}

		boolean CSignalDisplay::initialize()
		{
			this->getStaticBoxContext().getInputType(0, m_oInputTypeIdentifier);

			if(m_oInputTypeIdentifier==OV_TypeId_StreamedMatrix)
			{
				m_pStreamDecoder = new OpenViBEToolkit::TStreamedMatrixDecoder < CSignalDisplay >(*this, 0);
			}
			else if(m_oInputTypeIdentifier==OV_TypeId_Signal)
			{
				m_pStreamDecoder = new OpenViBEToolkit::TSignalDecoder < CSignalDisplay >(*this, 0);
			}
			else 
			{
				this->getLogManager() << LogLevel_Error << "Unknown input stream type at stream 0\n";
				return false;
			}

			m_oStimulationDecoder.initialize(*this,1);
			m_oUnitDecoder.initialize(*this,2);

			m_pBufferDatabase = new CBufferDatabase(*this);

			//retrieve settings
			CIdentifier l_oDisplayMode                   = (uint64)FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
			CIdentifier l_oScalingMode                   = (uint64)FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
			m_f64RefreshInterval                         = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
			float64 l_f64VerticalScale                   = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 3);
			float64 l_f64VerticalOffset                  = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 4);
			float64 l_f64TimeScale                       = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 5);
			boolean l_bHorizontalRuler                   = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 6);
			boolean l_bVerticalRuler                     = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 7);
			boolean l_bMultiview                         = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 8);

			if(m_f64RefreshInterval<0) {
				this->getLogManager() << LogLevel_Error << "Refresh interval must be >= 0\n";
				return false;
			}
			if(l_f64VerticalScale<=0)
			{
				this->getLogManager() << LogLevel_Error << "Vertical scale must be > 0\n";
				return false;
			}
			if(l_f64TimeScale<=0) {
				this->getLogManager() << LogLevel_Error << "Time scale must be > 0\n";
				return false;
			}

			this->getLogManager() << LogLevel_Debug << "l_sVerticalScale=" << l_f64VerticalScale << ", offset " << l_f64VerticalOffset << "\n";
			this->getLogManager() << LogLevel_Trace << "l_sScalingMode=" << l_oScalingMode << "\n";

			//create GUI
			m_pSignalDisplayView = new CSignalDisplayView(
				*m_pBufferDatabase,
				l_oDisplayMode,
				l_oScalingMode,
				l_f64VerticalScale,
				l_f64VerticalOffset,
				l_f64TimeScale,
				l_bHorizontalRuler,
				l_bVerticalRuler,
				l_bMultiview
				);

			m_pBufferDatabase->setDrawable(m_pSignalDisplayView);

			//parent visualisation box in visualisation tree
			::GtkWidget* l_pWidget=NULL;
			::GtkWidget* l_pToolbarWidget=NULL;
			dynamic_cast<CSignalDisplayView*>(m_pSignalDisplayView)->getWidgets(l_pWidget, l_pToolbarWidget);
			getBoxAlgorithmContext()->getVisualisationContext()->setWidget(l_pWidget);
			if(l_pToolbarWidget != NULL)
			{
				getBoxAlgorithmContext()->getVisualisationContext()->setToolbar(l_pToolbarWidget);
			}

			m_ui64LastScaleRefreshTime = 0;

			return true;
		}

		boolean CSignalDisplay::uninitialize()
		{
			m_oUnitDecoder.uninitialize();
			m_oStimulationDecoder.uninitialize();
			if(m_pStreamDecoder)
			{
				m_pStreamDecoder->uninitialize();
				delete m_pStreamDecoder;
			}

			delete m_pSignalDisplayView;
			delete m_pBufferDatabase;
			m_pSignalDisplayView=NULL;
			m_pBufferDatabase=NULL;

			return true;
		}

		boolean CSignalDisplay::processInput(uint32 ui32InputIndex)
		{
			getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
			return true;
		}

		boolean CSignalDisplay::process()
		{
			IDynamicBoxContext* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();

			if(m_pBufferDatabase->getErrorStatus()) {
				this->getLogManager() << LogLevel_Error << "Buffer database reports an error. Its possible that the inputs given to the Signal Display are not supported by it.\n";
				return false;
			}

			// Subcomponents may generate errors while running e.g. in gtk callbacks, where its not safe/possible to call logmanager
			if(((CSignalDisplayView*)m_pSignalDisplayView)->m_vErrorState.size()>0) {
				for(uint32 i=0;i<((CSignalDisplayView*)m_pSignalDisplayView)->m_vErrorState.size();i++)
				{
					this->getLogManager() << LogLevel_Error << ((CSignalDisplayView*)m_pSignalDisplayView)->m_vErrorState[i];
				}
				return false;
			}

#ifdef DEBUG
			uint64 in = System::Time::zgetTime();
#endif

			// Channel units on input 2 
			for(uint32 c=0; c<l_pDynamicBoxContext->getInputChunkCount(2); c++)
			{
				m_oUnitDecoder.decode(c);
				if(m_oUnitDecoder.isBufferReceived()) 
				{
					std::vector< std::pair<CString, CString> > l_vChannelUnits;
					l_vChannelUnits.resize(m_oUnitDecoder.getOutputMatrix()->getDimensionSize(0));
					const float64 *l_pBuffer = m_oUnitDecoder.getOutputMatrix()->getBuffer();
					for(uint32 i=0;i<l_vChannelUnits.size();i++)
					{
						CString l_sUnit = this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_MeasurementUnit, 
							static_cast<uint64>(l_pBuffer[i*2+0]));
						CString l_sFactor = this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Factor, 
							static_cast<uint64>(l_pBuffer[i*2+1]));

						l_vChannelUnits[i] = std::pair<CString, CString>(l_sUnit, l_sFactor);
					}
					
					if(!((CSignalDisplayView*)m_pSignalDisplayView)->setChannelUnits(l_vChannelUnits))
					{
						this->getLogManager() << LogLevel_Warning << "Unable to set channel units properly\n";
					}
				}
			}

			// Stimulations in input 1
			for(uint32 c=0; c<l_pDynamicBoxContext->getInputChunkCount(1); c++)
			{
				m_oStimulationDecoder.decode(c);
				if(m_oStimulationDecoder.isBufferReceived())
				{
					const IStimulationSet* l_pStimulationSet = m_oStimulationDecoder.getOutputStimulationSet();
					const uint64 l_ui64StimulationCount = l_pStimulationSet->getStimulationCount();

					m_pBufferDatabase->setStimulationCount(static_cast<uint32>(l_ui64StimulationCount));

					for(uint32 s=0;s<l_ui64StimulationCount;s++)
					{
						const uint64 l_ui64StimulationIdentifier = l_pStimulationSet->getStimulationIdentifier(s);
						const uint64 l_ui64StimulationDate = l_pStimulationSet->getStimulationDate(s);
						CString l_oStimulationName = getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation, l_ui64StimulationIdentifier);

						if(l_oStimulationName==CString("")) 
						{
							std::stringstream ss; ss << "Id " << l_ui64StimulationIdentifier;
							l_oStimulationName = CString(ss.str().c_str());
						}
						((CSignalDisplayView*)m_pSignalDisplayView)->onStimulationReceivedCB(l_ui64StimulationIdentifier, l_oStimulationName);
						m_pBufferDatabase->setStimulation(s, l_ui64StimulationIdentifier, l_ui64StimulationDate);
					}
				}
			}

			// Streamed matrix in input 0
			for(uint32 c=0; c<l_pDynamicBoxContext->getInputChunkCount(0); c++)
			{
				m_pStreamDecoder->decode(c);
				if(m_pStreamDecoder->isHeaderReceived())
				{
					const IMatrix* l_pMatrix = static_cast< OpenViBEToolkit::TStreamedMatrixDecoder<CSignalDisplay>* >(m_pStreamDecoder)->getOutputMatrix();

					if(m_oInputTypeIdentifier == OV_TypeId_Signal)
					{
						const uint64 l_ui64Rate = static_cast< OpenViBEToolkit::TSignalDecoder<CSignalDisplay>* >(m_pStreamDecoder)->getOutputSamplingRate();

						m_pBufferDatabase->setSamplingFrequency(static_cast<uint32>(l_ui64Rate));
					}

					m_pBufferDatabase->setMatrixDimensionCount(l_pMatrix->getDimensionCount());
					for(uint32 i=0;i<l_pMatrix->getDimensionCount();i++)
					{
						m_pBufferDatabase->setMatrixDimensionSize(i, l_pMatrix->getDimensionSize(i));
						for(uint32 j=0;j<l_pMatrix->getDimensionSize(i);j++) 
						{
							m_pBufferDatabase->setMatrixDimensionLabel(i, j, l_pMatrix->getDimensionLabel(i,j));
						}
					}
				}

				if(m_pStreamDecoder->isBufferReceived())
				{
					const IMatrix* l_pMatrix = static_cast< OpenViBEToolkit::TStreamedMatrixDecoder<CSignalDisplay>* >(m_pStreamDecoder)->getOutputMatrix();

#ifdef DEBUG
					static int count = 0; 
					std::cout << "Push chunk " << (count++) << " at " << 	l_pDynamicBoxContext->getInputChunkStartTime(0,c) << "\n";
#endif

					bool l_bReturnValue = m_pBufferDatabase->setMatrixBuffer(l_pMatrix->getBuffer(),
						l_pDynamicBoxContext->getInputChunkStartTime(0,c),
						l_pDynamicBoxContext->getInputChunkEndTime(0,c));
					if(!l_bReturnValue) 
					{
						return false;
					}

				}
			}

			const uint64 l_ui64TimeNow = getPlayerContext().getCurrentTime();
			if(m_ui64LastScaleRefreshTime == 0 || l_ui64TimeNow - m_ui64LastScaleRefreshTime > ITimeArithmetics::secondsToTime(m_f64RefreshInterval)) 
			{
//				this->getLogManager() << LogLevel_Info << "Refresh at " << ITimeArithmetics::timeToSeconds(l_ui64TimeNow) << "s \n";
				((CSignalDisplayView*)m_pSignalDisplayView)->refreshScale();
				m_ui64LastScaleRefreshTime = l_ui64TimeNow;
			}

#ifdef DEBUG
			out = System::Time::zgetTime();
			std::cout << "Elapsed1 " << out-in << "\n";
#endif

			return true;
		}
	};
};


