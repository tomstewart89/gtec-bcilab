#ifndef __OpenViBEPlugins_BoxAlgorithm_ErpPlot_H__
#define __OpenViBEPlugins_BoxAlgorithm_ErpPlot_H__

//You may have to change this path to match your folder organisation
#include "ovp_defines.h"


#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo.h>
#include <list>
#include <vector>
#include <iostream>
#include <cmath>
#include <system/ovCMemory.h>
#include <utility>
#include <stdio.h>
#include <string.h>
#include <cfloat>

namespace OpenViBEPlugins
{
		namespace SimpleVisualisation
	{

		class Graph
		{

			public:
				//should be a list of colors;
				Graph(std::vector< ::GdkColor>& lineColor, std::vector<OpenViBE::CString>& lineText, int rowIndex, int colIndex, int curveSize) : m_cLineColor(lineColor), m_sLineText(lineText)
				{
					m_lCurves.resize(lineColor.size());
                                        m_pStandardDeviation.resize(lineColor.size());
					for (unsigned int i=0; i<m_lCurves.size(); i++)
					{
						m_lCurves[i] = new OpenViBE::float64[curveSize];
						memset(m_lCurves[i], 0, sizeof(OpenViBE::float64) * curveSize);
                                                m_pStandardDeviation[i] = new OpenViBE::float64[curveSize];
                                                memset(m_pStandardDeviation[i], 0, sizeof(OpenViBE::float64) * curveSize);
					}
					this->rowIndex = rowIndex;
					this->colIndex = colIndex;
					this->curveSize = curveSize;
					pointCounter = new int[lineColor.size()];
					m_pMaximum = -FLT_MAX;
					m_pMinimum = FLT_MAX;
					for (unsigned int i=0; i<lineColor.size(); i++)
					{ 
						pointCounter[i] = 0;
					}
				}
				~Graph()
				{
					delete [] pointCounter;
					while(!m_lCurves.empty()) delete [] m_lCurves.back(), m_lCurves.pop_back();
				}
				void resizeAxis(gint width, gint height, int nrOfGraphs);
								 void draw(GtkWidget * widget);
				void drawAxis(cairo_t * cairoContext);
								void drawLine(cairo_t * cairoContext, double* Xo, double* Yo, double* Xe, double* Ye);
								 void drawAxisLabels(cairo_t * cairoContext);
				void drawCurves(cairo_t * cairoContext);
								void drawLegend(cairo_t * cairoContext);
								 void drawVar(cairo_t * cairoContext);
				void updateCurves(OpenViBE::float64* curve, unsigned int curveIndex);
								void setVariance(OpenViBE::float64* Variance, unsigned int curveIndex);
								 void snapCoords(cairo_t * cairoContext, double* x, double* y);
				OpenViBE::float64* begin(int curveIndex);
								 OpenViBE::float64 adjustValueToScale(OpenViBE::float64 value);

				std::vector <OpenViBE::float64*> m_lCurves; //private
				std::vector< ::GdkColor>& m_cLineColor; //private
				std::vector<OpenViBE::CString>& m_sLineText; //private
				OpenViBE::float64 m_pMaximum;
				OpenViBE::float64 m_pMinimum;
                                std::pair<int, int> m_pArgMaximum;
                                std::pair<int, int> m_pArgMinimum;
                                std::vector <OpenViBE::float64*> m_pStandardDeviation;
				double m_dGraphWidth;
				double m_dGraphHeight;
				double m_dGraphOriginX;
				double m_dGraphOriginY;
				int rowIndex; //private
				int colIndex; //private 
				int curveSize; //private
				int* pointCounter;
                                OpenViBE::uint64 StartTime;

                                OpenViBE::uint64 EndTime;

                                double fontSize;
				
		};

		/**
		 * \class CBoxAlgorithmErpPlot
		 * \author Dieter Devlaminck (INRIA)
		 * \date Fri Nov 16 10:50:43 2012
		 * \brief The class CBoxAlgorithmErpPlot describes the box ERP plot.
		 *
		 */
		class CBoxAlgorithmErpPlot : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);
                        virtual OpenViBE::boolean save(void);
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_ErpPlot);

		protected:
			OpenViBE::CString m_sFigureFileName;
			std::vector< ::GdkColor > m_oLegendColors;
			std::vector<OpenViBE::CString> m_oLegend;
			GtkWidget * m_pDrawWindow;
                        std::list<Graph*>* m_lGraphList;
                        OpenViBE::boolean m_bFirstHeaderReceived;
			std::vector< OpenViBEToolkit::TStreamedMatrixDecoder<CBoxAlgorithmErpPlot>* > m_vDecoders;
                        std::vector< OpenViBEToolkit::TStreamedMatrixDecoder<CBoxAlgorithmErpPlot>* > m_vVarianceDecoders;
			OpenViBEToolkit::TStimulationDecoder<CBoxAlgorithmErpPlot>* m_oStimulationDecoder;
			OpenViBE::uint64 m_sTriggerToSave;

		};		
		
		// The box listener can be used to call specific callbacks whenever the box structure changes : input added, name changed, etc.
		// Please uncomment below the callbacks you want to use.
		class CBoxAlgorithmErpPlotListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			//virtual OpenViBE::boolean onInitialized(OpenViBE::Kernel::IBox& rBox) { return true; };
			//virtual OpenViBE::boolean onNameChanged(OpenViBE::Kernel::IBox& rBox) { return true; };
			//virtual OpenViBE::boolean onInputConnected(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onInputDisconnected(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			virtual OpenViBE::boolean onInputAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) 
			{ 
				rBox.setInputType(ui32Index, OV_TypeId_StreamedMatrix);
                                OpenViBE::uint32 l_ui32Class = ui32Index/2 +1;
                                char l_sIndex[21];
                                sprintf(l_sIndex,"%d",l_ui32Class);


                                char l_pInputLabel[25] = "ERP";
                                rBox.setInputName(ui32Index, strcat(l_pInputLabel,l_sIndex));



				char l_pColorLabel[25] = "Line color ";
				char l_pTextLabel[25] = "Line label ";
				rBox.addSetting(strcat(l_pColorLabel,l_sIndex),OV_TypeId_Color,"0,0,0");
				rBox.addSetting(strcat(l_pTextLabel,l_sIndex),OV_TypeId_String,"curve");
                                //add the corresponding variance input
                                char l_pVarianceLabel[25] = "Variance";
                                rBox.addInput(strcat(l_pVarianceLabel,l_sIndex), OV_TypeId_StreamedMatrix);

				return true; 
			};
			virtual OpenViBE::boolean onInputRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) 
			{ 
				rBox.removeSetting(ui32Index*2+2);
				rBox.removeSetting(ui32Index*2+1);
				return true; 
			};
			//virtual OpenViBE::boolean onInputTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onInputNameChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputConnected(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputDisconnected(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onOutputNameChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingNameChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingDefaultValueChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingValueChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};
		

		/**
		 * \class CBoxAlgorithmErpPlotDesc
		 * \author Dieter Devlaminck (INRIA)
		 * \date Fri Nov 16 10:50:43 2012
		 * \brief Descriptor of the box ERP plot.
		 *
		 */
		class CBoxAlgorithmErpPlotDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("ERP plot"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Dieter Devlaminck"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Plots event-related potentials"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("plots target ERP versus non-target ERP"); }
                        virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Visualisation/Presentation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString(""); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_ErpPlot; }
						virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SimpleVisualisation::CBoxAlgorithmErpPlot; }
                        virtual OpenViBE::boolean hasFunctionality(OpenViBE::Kernel::EPluginFunctionality ePF) const { return ePF == OpenViBE::Kernel::PluginFunctionality_Visualization; }
			
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmErpPlotListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }
			
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("Trigger",OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput("ERP1",OV_TypeId_StreamedMatrix);
                                rBoxAlgorithmPrototype.addInput("Variance1",OV_TypeId_StreamedMatrix);

				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddInput);
					
				rBoxAlgorithmPrototype.addSetting("Filename final figure",OV_TypeId_Filename,"");
				rBoxAlgorithmPrototype.addSetting("Trigger to save figure",OV_TypeId_Stimulation,"OVTK_StimulationId_ExperimentStop");
				rBoxAlgorithmPrototype.addSetting("Line color 1",OV_TypeId_Color,"0,0,0");
				rBoxAlgorithmPrototype.addSetting("Line label 1",OV_TypeId_String,"curve 1");
				
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_ErpPlotDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_ErpPlot_H__
