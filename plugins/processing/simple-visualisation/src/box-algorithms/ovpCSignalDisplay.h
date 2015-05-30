#ifndef __OpenViBEPlugins_SimpleVisualisation_CSignalDisplay_H__
#define __OpenViBEPlugins_SimpleVisualisation_CSignalDisplay_H__

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <vector>
#include <string>

#include "../ovpCBufferDatabase.h"
#include "ovpCSignalDisplay/ovpCSignalDisplayView.h"

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		/**
		* This plugin opens a new GTK window and displays the incoming signals. The user may change the zoom level,
		* the width of the time window displayed, ...
		*/
		class CSignalDisplay : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CSignalDisplay(void);

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize();
			virtual OpenViBE::boolean uninitialize();
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process();

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_SignalDisplay)

		public:

			OpenViBEToolkit::TDecoder<CSignalDisplay>* m_pStreamDecoder;
			OpenViBEToolkit::TStimulationDecoder<CSignalDisplay> m_oStimulationDecoder;
			OpenViBEToolkit::TChannelUnitsDecoder<CSignalDisplay> m_oUnitDecoder;

			//The main object used for the display (contains all the GUI code)
			CSignalDisplayDrawable * m_pSignalDisplayView;

			//Contains all the data about the incoming signal
			CBufferDatabase * m_pBufferDatabase;

			OpenViBE::CIdentifier m_oInputTypeIdentifier;

		protected:

			OpenViBE::uint64 m_ui64LastScaleRefreshTime;

			OpenViBE::float64 m_f64RefreshInterval;

		};



		class CSignalDisplayListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			virtual OpenViBE::boolean onInputTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) 
			{
				if(ui32Index==1)
				{
					OpenViBE::CIdentifier l_oSettingType;
					rBox.getSettingType(ui32Index, l_oSettingType);
					if(l_oSettingType != OV_TypeId_Stimulations)
					{
						this->getLogManager() << OpenViBE::Kernel::LogLevel_Error << "Error: Only stimulation type supported for input 2\n";
						rBox.setInputType(ui32Index, OV_TypeId_Stimulations);
					}
				}
				else if(ui32Index==2)
				{
					OpenViBE::CIdentifier l_oSettingType;
					rBox.getSettingType(ui32Index, l_oSettingType);
					if(l_oSettingType != OV_TypeId_ChannelUnits)
					{
						this->getLogManager() << OpenViBE::Kernel::LogLevel_Error << "Error: Only measurement unit type supported for input 3\n";
						rBox.setInputType(ui32Index, OV_TypeId_ChannelUnits);
					}
				}

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};

		/**
		 * Signal Display plugin descriptor
		 */
		class CSignalDisplayDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Signal display"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Bruno Renier, Yann Renard, Alison Cellard, Jussi T. Lindgren"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Displays the incoming stream"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("This box can be used to visualize signal and matrix streams"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Visualisation/Basic"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.3"); }
			virtual void release(void)                                   { }
			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_SignalDisplay; }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-zoom-fit"); }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SimpleVisualisation::CSignalDisplay(); }
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CSignalDisplayListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual OpenViBE::boolean hasFunctionality(OpenViBE::Kernel::EPluginFunctionality ePF) const
			{
				return ePF == OpenViBE::Kernel::PluginFunctionality_Visualization;
			}

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addSetting("Display Mode", OVP_TypeId_SignalDisplayMode, "Scan");
				rPrototype.addSetting("Auto vertical scale", OVP_TypeId_SignalDisplayScaling, CSignalDisplayView::m_vScalingModes[0]);
                rPrototype.addSetting("Scale refresh interval (secs)", OV_TypeId_Float, "5");
				rPrototype.addSetting("Vertical Scale",OV_TypeId_Float, "100");
				rPrototype.addSetting("Vertical Offset",OV_TypeId_Float, "0");
				rPrototype.addSetting("Time Scale", OV_TypeId_Float, "10");
				rPrototype.addSetting("Bottom ruler", OV_TypeId_Boolean, "true");
				rPrototype.addSetting("Left ruler", OV_TypeId_Boolean, "false");
				rPrototype.addSetting("Multiview", OV_TypeId_Boolean, "false");

				rPrototype.addInput("Data",          OV_TypeId_Signal); 
				rPrototype.addInput("Stimulations",  OV_TypeId_Stimulations);
				rPrototype.addInput("Channel Units", OV_TypeId_ChannelUnits);

				rPrototype.addInputSupport(OV_TypeId_Signal);
				rPrototype.addInputSupport(OV_TypeId_StreamedMatrix);
				rPrototype.addInputSupport(OV_TypeId_ChannelUnits);

				rPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyInput);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_SignalDisplayDesc)
		};
	};
};

#endif
