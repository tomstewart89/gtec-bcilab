#ifndef __OpenViBEDesigner_CInterfacedScenario_H__
#define __OpenViBEDesigner_CInterfacedScenario_H__

#include "ovd_base.h"

#include "ovdCInterfacedObject.h"
#include "ovdCScenarioStateStack.h"

//moved here for modUI
#include <vector>
#include "ovdCBoxConfigurationDialog.h"

#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>

namespace OpenViBEDesigner
{
	class CApplication;
	class CDesignerVisualisation;
	class CPlayerVisualisation;
	class CBoxProxy;
	class CCommentProxy;

	class CInterfacedScenario
	{
	public:

		CInterfacedScenario(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBEDesigner::CApplication& rApplication, OpenViBE::Kernel::IScenario& rScenario, OpenViBE::CIdentifier& rScenarioIdentifier,
			::GtkNotebook& rNotebook, const char* sGUIFilename, const char* sGUISettingsFilename);
		virtual ~CInterfacedScenario(void);

		virtual OpenViBE::boolean isLocked(void) const;
		virtual void forceRedraw(void);
		virtual void redraw(void);
		virtual void redraw(OpenViBE::Kernel::IBox& rBox);
		virtual void redraw(OpenViBE::Kernel::IComment& rComment);
		virtual void redraw(OpenViBE::Kernel::ILink& rLink);
		virtual void updateScenarioLabel(void);
		OpenViBE::uint32 pickInterfacedObject(int x, int y);
		OpenViBE::boolean pickInterfacedObject(int x, int y, int iSizeX, int iSizeY);

		void undoCB(OpenViBE::boolean bManageModifiedStatusFlag=true);
		void redoCB(OpenViBE::boolean bManageModifiedStatusFlag=true);
		void snapshotCB(OpenViBE::boolean bManageModifiedStatusFlag=true);
		void addCommentCB(int x=-1, int y=-1);

		void scenarioDrawingAreaExposeCB(::GdkEventExpose* pEvent);
		void scenarioDrawingAreaDragDataReceivedCB(::GdkDragContext* pDragContext, gint iX, gint iY, ::GtkSelectionData* pSelectionData, guint uiInfo, guint uiT);
		void scenarioDrawingAreaMotionNotifyCB(::GtkWidget* pWidget, ::GdkEventMotion* pEvent);
		void scenarioDrawingAreaButtonPressedCB(::GtkWidget* pWidget, ::GdkEventButton* pEvent);
		void scenarioDrawingAreaButtonReleasedCB(::GtkWidget* pWidget, ::GdkEventButton* pEvent);
		void scenarioDrawingAreaKeyPressEventCB(::GtkWidget* pWidget, ::GdkEventKey* pEvent);
		void scenarioDrawingAreaKeyReleaseEventCB(::GtkWidget* pWidget, ::GdkEventKey* pEvent);
		void scenarioDrawingAreaLeaveNotifyCB(::GtkWidget* pWidget, ::GdkEventKey* pEvent);
		void scenarioDrawingAreaConfigureEventCB(::GtkWidget* pWidget, GdkRectangle *pEvent);


		void copySelection(void);
		void cutSelection(void);
		void pasteSelection(void);
		void deleteSelection(void);

		void setScale(OpenViBE::float64 rScale);
		OpenViBE::float64 getScale();

		void muteSelection(void);

		void contextMenuBoxRenameCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxRenameAllCB();
		void contextMenuBoxDeleteCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxAddInputCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditInputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxRemoveInputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxAddOutputCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditOutputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxRemoveOutputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxAddSettingCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxEditSettingCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxRemoveSettingCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxConfigureCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxAboutCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxMuteCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxDocumentationCB(OpenViBE::Kernel::IBox& rBox);

		void contextMenuBoxAddMessageInputCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxRemoveMessageInputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxAddMessageOutputCB(OpenViBE::Kernel::IBox& rBox);
		void contextMenuBoxRemoveMessageOutputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);

		void contextMenuBoxEditMessageInputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);
		void contextMenuBoxEditMessageOutputCB(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index);

		void contextMenuScenarioAboutCB(void);

		bool browseURL(OpenViBE::CString sURL);
		bool browseBoxDocumentation(OpenViBE::CIdentifier oBoxId);

		void toggleDesignerVisualisation();
		OpenViBE::boolean isDesignerVisualisationToggled();

		void showCurrentVisualisation();
		void hideCurrentVisualisation();

		void createPlayerVisualisation(void);
		void releasePlayerVisualisation(void);

		void onWindowClosed(const char* title);
		void onItemToggledOn(OpenViBE::uint32 ui32Index);
		void onItemToggledOff(OpenViBE::uint32 ui32Index);
		void hideWindowMenu(void);
		void showWindowMenu(void);


		OpenViBE::boolean hasSelection(void);

		OpenViBE::boolean deleteModifiableSettingsBoxes(void);
		OpenViBE::boolean setModifiableSettingsWidgets(void);
		OpenViBE::boolean centerOnBox(OpenViBE::CIdentifier rIdentifier);

		void alignBoxOnGrid(CBoxProxy& rProxy);
		void alignCommentOnGrid(CCommentProxy& rProxy);

/*
	private:

		void generateDisplayPluginName(OpenViBE::Kernel::IBox* pDisplayBox, OpenViBE::CString& rDisplayBoxName);*/

	public:

		OpenViBE::Kernel::EPlayerStatus m_ePlayerStatus;
		OpenViBE::CIdentifier m_oScenarioIdentifier;
		OpenViBE::CIdentifier m_oPlayerIdentifier;
		OpenViBE::CIdentifier m_oVisualisationTreeIdentifier;
		OpenViBE::CIdentifier m_oExporterIdentifier;
		OpenViBEDesigner::CApplication& m_rApplication;
		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IScenario& m_rScenario;
		OpenViBE::Kernel::IPlayer* m_pPlayer;
		OpenViBE::uint64 m_ui64LastLoopTime;
		::GtkNotebook& m_rNotebook;
		OpenViBE::Kernel::IVisualisationTree* m_pVisualisationTree;
		OpenViBE::boolean m_bDesignerVisualisationToggled;
		OpenViBEDesigner::CDesignerVisualisation* m_pDesignerVisualisation;
		OpenViBEDesigner::CPlayerVisualisation* m_pPlayerVisualisation;
		::GtkBuilder* m_pBuilderDummyScenarioNotebookTitle;
		::GtkBuilder* m_pBuilderDummyScenarioNotebookClient;
		::GtkBuilder* m_pBuilderTooltip;
		::GtkWidget* m_pNotebookPageTitle;
		::GtkWidget* m_pNotebookPageContent;
		::GtkViewport* m_pScenarioViewport;
		::GtkScrolledWindow* m_pScrolledWindow;//designer trim arrow scrolling
		::GtkDrawingArea* m_pScenarioDrawingArea;
		::GdkPixmap* m_pBufferedDrawingArea;
		::GdkPixmap* m_pStencilBuffer;
		::GtkWidget* m_pTooltip;
		OpenViBE::boolean m_bScenarioModified;
		OpenViBE::boolean m_bHasFileName;
		OpenViBE::boolean m_bHasBeenModified;
		OpenViBE::boolean m_bButtonPressed;
		OpenViBE::boolean m_bShiftPressed;
		OpenViBE::boolean m_bControlPressed;
		OpenViBE::boolean m_bAltPressed;
		OpenViBE::boolean m_bAPressed;
		OpenViBE::boolean m_bWPressed;
		OpenViBE::boolean m_bDebugCPUUsage;
		std::string m_sFileName;
		std::string m_sGUIFilename;
		std::string m_sGUISettingsFilename;
		OpenViBE::float64 m_f64PressMouseX;
		OpenViBE::float64 m_f64PressMouseY;
		OpenViBE::float64 m_f64ReleaseMouseX;
		OpenViBE::float64 m_f64ReleaseMouseY;
		OpenViBE::float64 m_f64CurrentMouseX;
		OpenViBE::float64 m_f64CurrentMouseY;
		OpenViBE::int32 m_i32ViewOffsetX;
		OpenViBE::int32 m_i32ViewOffsetY;
		OpenViBE::uint32 m_ui32CurrentMode;
		OpenViBE::float64 m_f64CurrentScale;
		OpenViBE::uint32 m_ui32NormalFontSize;

		gint m_iCurrentWidth;
		gint m_iCurrentHeight;


		OpenViBE::uint32 m_ui32BoxCount;
		OpenViBE::uint32 m_ui32CommentCount;
		OpenViBE::uint32 m_ui32LinkCount;

		OpenViBE::uint32 m_ui32InterfacedObjectId;
		std::map<OpenViBE::uint32, OpenViBEDesigner::CInterfacedObject> m_vInterfacedObject;
		std::map<OpenViBE::CIdentifier, OpenViBE::boolean> m_vCurrentObject;
		OpenViBEDesigner::CInterfacedObject m_oCurrentObject;

		//these variables do not appear anywhere (!?)
		// OpenViBE::float64 m_f64HPanInitialPosition;
		// OpenViBE::float64 m_f64VPanInitialPosition;

		typedef struct _BoxContextMenuCB
		{
			OpenViBE::uint32 ui32Command;
			OpenViBE::uint32 ui32Index;
			OpenViBE::Kernel::IBox* pBox;
			OpenViBEDesigner::CInterfacedScenario* pInterfacedScenario;
		} BoxContextMenuCB;
		std::map < OpenViBE::uint32, BoxContextMenuCB > m_vBoxContextMenuCB;

		OpenViBEDesigner::CScenarioStateStack m_oStateStack;

		std::vector<CBoxConfigurationDialog*> m_vBoxConfigurationDialog;
		// List of items in Menu->window->Show that appear while playing a scenario
		std::vector < ::GtkWidget* > m_vCheckItems;

	};
};

#endif // __OpenViBEDesigner_CInterfacedScenario_H__
