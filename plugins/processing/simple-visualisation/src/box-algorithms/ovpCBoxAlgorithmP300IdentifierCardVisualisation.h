#ifndef __OpenViBEPlugins_BoxAlgorithm_P300IdentifierCardVisualisation_H__
#define __OpenViBEPlugins_BoxAlgorithm_P300IdentifierCardVisualisation_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <gtk/gtk.h>
#include <map>


namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		class CBoxAlgorithmP300IdentifierCardVisualisation : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32Index);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_P300IdentifierCardVisualisation);

		private:

			typedef struct
			{
				int iIndex;
				::GdkColor oBackgroundColor;
				::GtkWidget* pParent;
				::GtkWidget* pWidget;
				::GtkWidget* pImage;
			} SWidgetStyle;

			typedef void (CBoxAlgorithmP300IdentifierCardVisualisation::*_cache_callback_)(CBoxAlgorithmP300IdentifierCardVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData);

			void _cache_build_from_table_(::GtkTable* pTable);
			void _cache_for_each_(_cache_callback_ fpCallback, void* pUserData);
			void _cache_for_each_if_(int iCard, _cache_callback_ fpIfCallback, _cache_callback_ fpElseCallback, void* pIfUserData, void* pElseUserData);
			void _cache_change_null_cb_(CBoxAlgorithmP300IdentifierCardVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData);
			void _cache_change_image_cb_(CBoxAlgorithmP300IdentifierCardVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData);
			void _cache_change_background_cb_(CBoxAlgorithmP300IdentifierCardVisualisation::SWidgetStyle& rWidgetStyle, void* pUserData);

		protected:

			OpenViBE::CString m_sInterfaceFilename;
			OpenViBE::uint64 m_ui64CardStimulationBase;

		private:

			OpenViBE::Kernel::IAlgorithmProxy* m_pSequenceStimulationDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pTargetStimulationDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pTargetFlaggingStimulationEncoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pCardSelectionStimulationDecoder;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pSequenceMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pTargetMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IStimulationSet*> ip_pTargetFlaggingStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pSequenceStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pTargetStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IMemoryBuffer*> op_pTargetFlaggingMemoryBuffer;
			OpenViBE::uint64 m_ui64LastTime;

			::GtkBuilder* m_pMainWidgetInterface;
			::GtkBuilder* m_pToolbarWidgetInterface;
			::GtkWidget* m_pMainWindow;
			::GtkWidget* m_pToolbarWidget;
			::GtkTable* m_pTable;
			::GtkLabel* m_pResult;
			::GtkLabel* m_pTarget;
			::GdkColor m_oBackgroundColor;
			::GdkColor m_oTargetBackgroundColor;
			::GdkColor m_oSelectedBackgroundColor;
			::GtkLabel* m_oTargetLabel;
			::GtkLabel* m_oSelectedLabel;
			OpenViBE::uint64 m_ui64CardCount;

			int m_iTargetCard;

			std::vector < ::GtkWidget* > m_vForegroundImageTarget;
			std::vector < ::GtkWidget* > m_vForegroundImageWork;
			std::vector < ::GtkWidget* > m_vForegroundImageResult;
			::GtkWidget* m_pBackgroundImageTarget;
			::GtkWidget* m_pBackgroundImageWork;
			::GtkWidget* m_pBackgroundImageResult;

			OpenViBE::boolean m_bTableInitialized;

			std::vector <CBoxAlgorithmP300IdentifierCardVisualisation::SWidgetStyle > m_vCache;
		};

		class CBoxAlgorithmP300IdentifierCardVisualisationDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("P300 Identifier Card Visualisation"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Baptiste Payan"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Displays images to the user based on received stimulations with some additional P300 related functionality"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Visualisation/Presentation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-select-font"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_P300IdentifierCardVisualisation; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SimpleVisualisation::CBoxAlgorithmP300IdentifierCardVisualisation; }

			virtual OpenViBE::boolean hasFunctionality(OpenViBE::Kernel::EPluginFunctionality ePF) const { return ePF == OpenViBE::Kernel::PluginFunctionality_Visualization; }
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput ("Sequence stimulations",            OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput ("Target stimulations",              OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput ("Card selection stimulations",      OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addOutput("Target / Non target flagging",     OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addSetting("Interface filename",              OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-identifier-card.ui");
				rBoxAlgorithmPrototype.addSetting("Background color",                OV_TypeId_Color,       "0,0,0");
				rBoxAlgorithmPrototype.addSetting("Target background color",         OV_TypeId_Color,       "10,40,10");
				rBoxAlgorithmPrototype.addSetting("Selected background color",       OV_TypeId_Color,       "70,20,20");
				rBoxAlgorithmPrototype.addSetting("Card stimulation base",           OV_TypeId_Stimulation, "OVTK_StimulationId_Label_01");
				rBoxAlgorithmPrototype.addSetting("Background Card filename",        OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/openvibe-logo.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/01.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/02.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/03.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/04.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/05.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/06.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/07.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/08.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/09.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/10.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/11.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualisation/p300-magic-card/12.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "");

				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_P300IdentifierCardVisualisationDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_P300IdentifierCardVisualisation_H__
