#ifndef __OpenViBEPlugins_SimpleVisualisation_CSignalDisplayView_H__
#define __OpenViBEPlugins_SimpleVisualisation_CSignalDisplayView_H__

#include "../../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <gtk/gtk.h>

#include "ovpCSignalChannelDisplay.h"
#include "../../ovpCBufferDatabase.h"

#include "../../ovpCBottomTimeRuler.h"

#include <vector>
#include <string>
#include <map>

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		/**
		* This class contains everything necessary to setup a GTK window and display
		* a signal thanks to a CBufferDatabase's information.
		*/
		class CSignalDisplayView : public CSignalDisplayDrawable
		{

		public:


			/**
			 * \brief Constructor
			 * \param [in] rBufferDatabase Signal database
			 * \param [in] f64TimeScale Initial time scale value
			 * \param [in] oDisplayMode Initial signal display mode
			 * \param [in] bAutoVerticalScale Initial auto vertical scale value
			 * \param [in] f64VerticalScale Initial vertical scale value
			 */
			CSignalDisplayView(
				CBufferDatabase& rBufferDatabase,
				OpenViBE::CIdentifier oDisplayMode,
				OpenViBE::CIdentifier oScalingMode,
				OpenViBE::float64 f64VerticalScale,
				OpenViBE::float64 f64VerticalOffset,
				OpenViBE::float64 f64TimeScale,
				OpenViBE::boolean l_bHorizontalRuler,
				OpenViBE::boolean l_bVerticalRuler,
				OpenViBE::boolean l_bIsMultiview
			);

			/**
			 * \brief Base constructor
			 * \param [in] rBufferDatabase Signal database
			 * \param [in] f64TimeScale Initial time scale value
			 * \param [in] oDisplayMode Initial signal display mode
			 */
			void construct(
				CBufferDatabase& oBufferDatabase,
				OpenViBE::float64 f64TimeScale,
				OpenViBE::CIdentifier oDisplayMode);

			/**
			 * \brief Destructor
			 */
			virtual ~CSignalDisplayView(void);
			/**
			 * \brief Get pointers to plugin main widget and (optional) toolbar widget
			 * \param [out] pWidget Pointer to main widget
			 * \param [out] pToolbarWidget Pointer to (optional) toolbar widget
			 */
			void getWidgets(
				::GtkWidget*& pWidget,
				::GtkWidget*& pToolbarWidget);
			/**
			 * Initializes the window.
			 */
			virtual void init(void);
			/**
			 * Invalidates the window's content and tells it to redraw itself.
			 */
			virtual void redraw(void);
			/**
			* Toggle left rulers on/off
			* \param bActive Show rulers if true.
			*/
			void toggleLeftRulers(
				OpenViBE::boolean bActive);
			/**
			 * Toggle time ruler on/off
			 * \param bActive Show the ruler if true.
			 */
			void toggleBottomRuler(
				OpenViBE::boolean bActive);
			/**
			 * Toggle a channel on/off
			 * \param ui64ChannelIndex The index of the channel to toggle.
			 * \param bActive Show the channel if true.
			 */
			void toggleChannel(
				OpenViBE::uint32 ui32ChannelIndex,
				OpenViBE::boolean bActive);

			void toggleChannelMultiView(
				OpenViBE::boolean bActive);

			void changeMultiView(void);

			void removeOldWidgets(void);
			void recreateWidgets(OpenViBE::uint32 ui32ChannelCount);
			void updateMainTableStatus(void);

			void updateDisplayTableSize(void);

			void activateToolbarButtons(
				OpenViBE::boolean bActive);

			/**
			 * Callback called when display mode changes
			 * \param oDisplayMode New display mode
			 * \return True
			 */
			OpenViBE::boolean onDisplayModeToggledCB(
				OpenViBE::CIdentifier oDisplayMode);

			OpenViBE::boolean onUnitsToggledCB(OpenViBE::boolean active);


			/**
			 * Callback called when vertical scale mode changes
			 * \param pToggleButton Radio button toggled
			 * \return True
			 */
			OpenViBE::boolean onVerticalScaleModeToggledCB(
				GtkToggleButton* pToggleButton);

			/**
			 * Callback called when custom vertical scale is changed
			 * \param pSpinButton Custom vertical scale widget
			 * \return True if custom vertical scale value could be retrieved, false otherwise
			 */
			OpenViBE::boolean onCustomVerticalScaleChangedCB(
				::GtkSpinButton* pSpinButton);
			OpenViBE::boolean onCustomVerticalOffsetChangedCB(
				::GtkSpinButton* pSpinButton);

			/**
			 * \brief Get a channel display object
			 * \param ui32ChannelIndex Index of channel display
			 * \return Channel display object
			 */
			CSignalChannelDisplay* getChannelDisplay(
				OpenViBE::uint32 ui32ChannelIndex);

			OpenViBE::boolean isChannelDisplayVisible(
				OpenViBE::uint32 ui32ChannelIndex);

			void onStimulationReceivedCB(
				OpenViBE::uint64 ui64StimulationCode,
				const OpenViBE::CString& rStimulationName);

			OpenViBE::boolean setChannelUnits(const std::vector< std::pair<OpenViBE::CString, OpenViBE::CString> >& oChannelUnits);

			/**
			 * \brief Get a color from a stimulation code
			 * \remarks Only the lower 32 bits of the stimulation code are currently used to compute the color.
			 * \param[in] ui64StimulationCode Stimulation code
			 * \param[out] rColor Color computed from stimulation code
			 */
			void getStimulationColor(
				OpenViBE::uint64 ui64StimulationCode,
				::GdkColor& rColor);

			/**
			 * \brief Get a color from a signal
			 * \param[in] ui32SignalIndex channel index
			 * \param[out] rColor Color computed from stimulation code
			 */
			void getMultiViewColor(
				OpenViBE::uint32 ui32ChannelIndex,
				::GdkColor& rColor);

			void refreshScale(void);

		private:
			/**
			 * \brief Update stimulations color dialog with a new (stimulation, color) pair
			 * \param[in] rStimulationLabel Stimulation label
			 * \param[in] rStimulationColor Stimulation color
			 */
			void updateStimulationColorsDialog(
				const OpenViBE::CString& rStimulationLabel,
				const GdkColor& rStimulationColor);

		public:

			//! The Builder handler used to create the interface
			::GtkBuilder* m_pBuilderInterface;

			//! The table containing the CSignalChannelDisplays
			GtkWidget* m_pSignalDisplayTable;

			GtkWidget* m_pSeparator;

			//! Array of the channel's labels
			std::vector<GtkWidget*> m_oChannelLabel;

			//! Array of CSignalChannelDisplays (one per channel, displays the corresponding channel)
			std::vector < CSignalChannelDisplay* > m_oChannelDisplay;
			std::map < OpenViBE::uint32, ::GtkWidget* > m_vSeparator;

			//! Show left rulers when true
			OpenViBE::boolean m_bShowLeftRulers;

			//!Show bottom time ruler when true
			OpenViBE::boolean m_bShowBottomRuler;

			//! Time of displayed signals at the left of channel displays
			OpenViBE::uint64 m_ui64LeftmostDisplayedTime;

			//! Largest displayed value range, to be matched by all channels in global best fit mode
			OpenViBE::float64 m_f64LargestDisplayedValueRange;

			OpenViBE::float64 m_f64LargestDisplayedValue;
			OpenViBE::float64 m_f64SmallestDisplayedValue;

			//! Current value range margin, used to avoid redrawing signals every time the largest value range changes
			OpenViBE::float64 m_f64ValueRangeMargin;
			// OpenViBE::float64 m_f64ValueMaxMargin;
			/*! Margins added to largest and subtracted from smallest displayed values are computed as :
			m_f64MarginFactor * m_f64LargestDisplayedValueRange. If m_ui64MarginFactor = 0, there's no margin at all.
			If factor is 0.1, largest displayed value range is extended by 10% above and below its extremums at the time
			when margins are computed. */
			OpenViBE::float64 m_f64MarginFactor;

			//! Normal/zooming cursors
			GdkCursor* m_pCursor[2];

			/** \name Vertical scale */
			//@{
			//! Flag set to true when you'd like the display to *check* if the scale needs to change and possibly update
			OpenViBE::boolean m_bVerticalScaleRefresh;
			//! Flag set to true when you'd like the display to update in any case
			OpenViBE::boolean m_bVerticalScaleForceUpdate;
			//! Value of custom vertical scale
			OpenViBE::float64 m_f64CustomVerticalScaleValue;
			//! Value of custom vertical offset
			OpenViBE::float64 m_f64CustomVerticalOffset;
			//@}

			//! The database that contains the information to use to draw the signals
			CBufferDatabase * m_pBufferDatabase;

			//! Vector of gdk points. Used to draw the signals.
			std::vector<GdkPoint> m_pPoints;

			//! Vector of raw points. Stores the points' coordinates before cropping.
			std::vector<std::pair<OpenViBE::float64,OpenViBE::float64> > m_pRawPoints;

			std::vector< OpenViBE::CString > m_vChannelName;

			//! Vector of indexes of the channels to display
			std::map<OpenViBE::uint32, OpenViBE::boolean> m_vSelectedChannels;

			OpenViBE::uint32 m_ui32SelectedChannelCount; 

			std::map<OpenViBE::uint32, std::pair<OpenViBE::CString, OpenViBE::CString> > m_vChannelUnits;

			//! Flag set to true once multi view configuration dialog is initialized
			OpenViBE::boolean m_bMultiViewEnabled;

			//! Vector of indices of selected channels
			std::map<OpenViBE::uint32, OpenViBE::boolean> m_vMultiViewSelectedChannels;

			//Map of stimulation codes received so far, and their corresponding name and color
			std::map< OpenViBE::uint64, std::pair< OpenViBE::CString, ::GdkColor > > m_mStimulations;

			//Map of signal indices received so far, and their corresponding name and color
			std::map< OpenViBE::uint64, std::pair< OpenViBE::CString, ::GdkColor > > m_mSignals;

			//! Bottom box containing bottom ruler
			GtkBox* m_pBottomBox;

			//! Bottom time ruler
			CBottomTimeRuler * m_pBottomRuler;

			//! Widgets for left rulers
			std::vector <GtkWidget *> m_oLeftRulers;

			OpenViBE::CIdentifier m_oScalingMode;

			static const char* m_vScalingModes[];

			std::vector<OpenViBE::CString> m_vErrorState;

			OpenViBE::boolean m_bStimulationColorsShown;
		};
	}
}

#endif
