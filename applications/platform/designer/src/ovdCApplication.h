#ifndef __OpenViBEDesigner_CApplication_H__
#define __OpenViBEDesigner_CApplication_H__

#include "ovd_base.h"

#include <vector>


namespace OpenViBEDesigner
{
	class CInterfacedScenario;

	class CLogListenerDesigner;

	class CApplication
	{
	public:
		/**
		 * \brief Replay modes
		 */
		enum EReplayMode
		{
			EReplayMode_None,
			EReplayMode_Play,
			EReplayMode_Forward
		};

		CApplication(const OpenViBE::Kernel::IKernelContext& rKernelContext);
		~CApplication(void);

		void initialize(OpenViBEDesigner::ECommandLineFlag eCommandLineFlags);

		OpenViBE::boolean openScenario(const char* sFileName);

		/** \name Drag and drop management */
		//@{

		void dragDataGetCB(
			::GtkWidget* pWidget,
			::GdkDragContext* pDragContex,
			::GtkSelectionData* pSelectionData,
			guint uiInfo,
			guint uiT);

		//@}

		/** \name Selection management */
		//@{

		void undoCB(void);
		void redoCB(void);

		void copySelectionCB(void);
		void cutSelectionCB(void);
		void pasteSelectionCB(void);
		void deleteSelectionCB(void);
		void preferencesCB(void);

		//@}

		/** \name Scenario management */
		//@{

		OpenViBE::CString getWorkingDirectory(void);

		// changes the working dir config token to reflect the current working directory of the scenario
		void updateWorkingDirectoryToken(const OpenViBE::CIdentifier &oScenarioIdentifier);	
		void removeScenarioDirectoryToken(const OpenViBE::CIdentifier &oScenarioIdentifier);
		void resetVolatileScenarioDirectoryToken();

		OpenViBE::boolean hasRunningScenario(void);
		OpenViBE::boolean hasUnsavedScenario(void);

		OpenViBEDesigner::CInterfacedScenario* getCurrentInterfacedScenario(void);

		void testCB(void);
		void newScenarioCB(void);
		void openScenarioCB(void);
		OpenViBE::boolean saveScenarioCB(OpenViBEDesigner::CInterfacedScenario* pInterfacedScenario=NULL); // defaults to current scenario if NULL
		OpenViBE::boolean saveScenarioAsCB(OpenViBEDesigner::CInterfacedScenario* pInterfacedScenario=NULL); // defaults to current scenario if NULL
		void closeScenarioCB(
			OpenViBEDesigner::CInterfacedScenario* pInterfacedScenario);

		void zoomInCB(void);//Call when a zoom in is required
		void zoomOutCB(void);//Call when a zoom out is required
		void spinnerZoomChangedCB(OpenViBE::uint32 scaleDelta);

		void windowItemToggledCB(GtkCheckMenuItem* pCheckMenuItem);
		void toggleOnWindowItem(OpenViBE::uint32 ui32Index, OpenViBE::int32 i32PageIndex);
		void toggleOffWindowItem(OpenViBE::uint32 ui32Index);

		void stopScenarioCB(void);
		void pauseScenarioCB(void);
		void nextScenarioCB(void);
		OpenViBE::boolean playScenarioCB(void);
		OpenViBE::boolean forwardScenarioCB(void);

		void keyPressEventCB(::GtkWidget* pWidget, ::GdkEventKey* pEvent);

		void addCommentCB(
			OpenViBEDesigner::CInterfacedScenario* pScenario);

		void changeCurrentScenario(
			OpenViBE::int32 i32PageIndex);
		void reorderCurrentScenario(
			OpenViBE::uint32 i32NewPageIndex);

		//@}

		/** \name Designer visualisation management */
		//@{

		void deleteDesignerVisualisationCB();

		void toggleDesignerVisualisationCB();

		//@}

		/** \name Player management */
		//@{

		OpenViBE::Kernel::IPlayer* getPlayer(void);

		OpenViBE::boolean createPlayer(void);
		OpenViBE::boolean isPlayerExisting(void);
		void releasePlayer(void);

		void destroyWindowMenu(void);

		//@}

		/** \name Application management */
		//@{

		OpenViBE::boolean quitApplicationCB(void);
		void aboutOpenViBECB(void);
		void aboutScenarioCB(OpenViBEDesigner::CInterfacedScenario* pScenario);
		void aboutLinkClickedCB(const gchar *url);

		void browseDocumentationCB(void);

		//@}

		/** \name Log management */
		//@{

		void logLevelCB(void);
		void logLevelMessagesCB(void);
		void logLevelRestore(GObject* ToolButton, OpenViBE::Kernel::ELogLevel level, const char* configName);
		OpenViBE::uint32 getLogState(const char* sButtonName);


		//@}

		/** \name CPU usage */
		//@{

		void CPUUsageCB(void);

		//@}

		// to know which GtkEntry should be fu=ocused on when doing CTRL+F
		OpenViBE::boolean isLogAreaClicked();

		OpenViBE::boolean isNoGuiActive();

	public:

		const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
		OpenViBE::Kernel::IPluginManager* m_pPluginManager;
		OpenViBE::Kernel::IScenarioManager* m_pScenarioManager;
		OpenViBE::Kernel::IVisualisationManager* m_pVisualisationManager;
		OpenViBE::Kernel::IScenario* m_pClipboardScenario;

		OpenViBEDesigner::CLogListenerDesigner* m_pLogListenerDesigner;

		OpenViBEDesigner::ECommandLineFlag m_eCommandLineFlags;

		::GtkBuilder* m_pBuilderInterface;
		::GtkWidget* m_pMainWindow;
		::GtkNotebook* m_pScenarioNotebook;
		::GtkNotebook* m_pResourceNotebook;
		::GtkTreeStore* m_pBoxAlgorithmTreeModel;
		::GtkTreeModel* m_pBoxAlgorithmTreeModelFilter;
		::GtkTreeModel* m_pBoxAlgorithmTreeModelFilter2;
		::GtkTreeModel* m_pBoxAlgorithmTreeModelFilter3;
		::GtkTreeModel* m_pBoxAlgorithmTreeModelFilter4;
		::GtkTreeView* m_pBoxAlgorithmTreeView;
		::GtkTreeStore* m_pAlgorithmTreeModel;
		::GtkTreeView* m_pAlgorithmTreeView;
		::GtkEntry* m_pSearchEntry;//for search in log
		::GtkTextView* m_pTextView;//for search log
		::GtkTextTag* m_pCIdentifierTag;
		::GtkSpinButton* m_pZoomSpinner;
		gint m_giFilterTimeout;

		const gchar* m_sSearchTerm;
		const gchar* m_sLogSearchTerm;
		
		OpenViBE::float64 m_f64LastTimeRefresh;
		OpenViBE::boolean m_bIsQuitting;
		OpenViBE::int32 m_i32CurrentScenarioPage;

		::GtkToggleButton* m_pInitAlert;

		std::vector < OpenViBEDesigner::CInterfacedScenario* > m_vInterfacedScenario;

		// Keeps track of the used playing mode so the playback can be restarted in the same mode.
		EReplayMode m_eReplayMode;
	};
};

#endif // __OpenViBEDesigner_CApplication_H__
