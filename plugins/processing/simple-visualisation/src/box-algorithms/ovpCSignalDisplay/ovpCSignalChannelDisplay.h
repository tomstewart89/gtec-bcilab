#ifndef __OpenViBEPlugins_SimpleVisualisation_CSignalChannelDisplay_H__
#define __OpenViBEPlugins_SimpleVisualisation_CSignalChannelDisplay_H__

#include "../../ovp_defines.h"
#include "ovpCSignalDisplayLeftRuler.h"

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <openvibe/ov_all.h>

#include <toolkit/ovtk_all.h>

#include <memory.h>
#include <cmath>

#include <vector>
#include <map>

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{

	class CSignalDisplayView;
	class CBufferDatabase;

	class CSignalChannelDisplay
	{
	public:
		/**
		 * \brief Constructor
		 * \param pDisplayView Parent view
		 * \param i32ChannelDisplayWidthRequest Width to be requested by widget
		 * \param i32ChannelDisplayHeightRequest Height to be requested by widget
		 * \param i32LeftRulerWidthRequest Width to be requested by left ruler
		 * \param i32LeftRulerHeightRequest Height to be requested by left ruler
		 */
		CSignalChannelDisplay(
			CSignalDisplayView* pDisplayView,
			OpenViBE::int32 i32ChannelDisplayWidthRequest,
			OpenViBE::int32 i32ChannelDisplayHeightRequest,
			OpenViBE::int32 i32LeftRulerWidthRequest,
			OpenViBE::int32 i32LeftRulerHeightRequest);

		/**
		 * \brief Destructor
		 */
		~CSignalChannelDisplay();

		/**
		 * \brief Get ruler widget
		 * \return Pointer to ruler widget
		 */
        ::GtkWidget* getRulerWidget(OpenViBE::uint32 ui32Index) const;

		/**
		 * \brief Get signal display widget
		 * \return Pointer to signal display widget
		 */
		::GtkWidget* getSignalDisplayWidget() const;

		/**
		 * \brief Callback notified upon resize events
		 * \param i32Width New window width
		 * \param i32Height New window height
		 */
		void onResizeEventCB(
			gint i32Width,
			gint i32Height);

		/**
		 * \brief Updates scale following a resize event or a time scale change
		 */
		void updateScale();

		// Updates some drawing limits, i.e. to limit drawing to [chn_i,...,chn_j]
		void updateLimits(void);

		/**
		 * \brief Reset list of channels displayed by this object
		 */
        void resetChannelList();

		/**
		 * \brief Add a channel to the list of channels to be displayed
		 * \param ui32Channel Index of channel to be displayed
		 */
		void addChannel(
			OpenViBE::uint32 ui32Channel);

        void addChannelList(
            OpenViBE::uint32 ui32Channel);

		/**
		 * \brief Get rectangle to clear and redraw based on latest signal data received
		 * \param[out] rRect Rectangle holding part of drawing area to clear and update
		 */
		void getUpdateRectangle(
			GdkRectangle& rRect);

		/**
		 * \brief Flag widget so that its whole window is redrawn at next refresh
	   */
		void redrawAllAtNextRefresh();

		/**
		 * \brief Check whether the whole window must be redrawn
		 * \return True if the whole window must be redrawn, false otherwise
		 */
		OpenViBE::boolean mustRedrawAll();

		/**
		 * \brief Draws the signal on the signal's drawing area.
		 * \param rExposedArea Exposed area that needs to be redrawn
		 */
		void draw(
			const GdkRectangle& rExposedArea);

		/**
		 * \brief Clip signals to drawing area
		 * Computes the list of points used to draw the lines (m_pParentDisplayView->m_pPoints) using the raw points list
		 * (m_pParentDisplayView->m_pRawPoints) and by cropping the lines when they go out of the window.
		 * \param ui64PointCount Number of points to clip
		 * \return The number of points to display.
		 */
		OpenViBE::uint64 cropCurve(
			OpenViBE::uint64 ui64PointCount);

		/**
		 * \brief Computes the parameters necessary for the signal to be zoomed at the selected coordinates.
		 * \param bZoomIn If true, the operation is a zoom In, if false it's a zoom Out.
		 * \param f64XClick The X-coordinate of the center of the area we want to zoom in.
		 * \param f64YClick The Y-coordinate of the center of the area we want to zoom in.
		 */
		void computeZoom(
			OpenViBE::boolean bZoomIn,
			OpenViBE::float64 f64XClick,
			OpenViBE::float64 f64YClick);

		/**
		 * \brief Returns empiric y min and maxes of the currently shown signal chunks for all subchannels.
		 * Note that the actually used display limits may be different. This function can be used
		 * to get the data extremal values and then use these to configure the display appropriately.
		 */
        void getDisplayedValueRange(
			std::vector<OpenViBE::float64>& rDisplayedValueMin,
			std::vector<OpenViBE::float64>& rDisplayedValueMax) const;

		/*
		 * \brief Sets scale for all subchannels. 
		 */
		void setGlobalScaleParameters(
			const OpenViBE::float64 f64Min,
			const OpenViBE::float64 f64Max,
			const OpenViBE::float64 f64Margin);

		/*
		 * \brief Sets scale for a single subchannel.
		 */
		void setLocalScaleParameters(
			const OpenViBE::uint32 subChannelIndex,
			const OpenViBE::float64 rMin,
			const OpenViBE::float64 rMax,
			const OpenViBE::float64 f64Margin);

		/**
		 * \brief Updates signal scale and translation based on latest global range and margin
		 */
		void updateDisplayParameters();

	private:
		/**
		 * \brief Get first buffer to display index and position and first sample to display index
		 * \param[out] rFirstBufferToDisplay Index of first buffer to display
		 * \param[out] rFirstSampleToDisplay Index of first sample to display
		 * \param[out] rFirstBufferToDisplayPosition Position of first buffer to display (0-based, from left edge)
		 */
		void getFirstBufferToDisplayInformation(
			OpenViBE::uint32& rFirstBufferToDisplay,
			OpenViBE::uint32& rFirstSampleToDisplay,
			OpenViBE::uint32& rFirstBufferToDisplayPosition);

		/**
		 * \brief Get start X coord of a buffer
		 * \param ui32Position Position of buffer on screen (0-based, from left edge)
		 * \return Floored X coordinate of buffer
		 */
		OpenViBE::int32 getBufferStartX(
			OpenViBE::uint32 ui32Position);

		/**
		 * \brief Get X coordinate of a sample
		 * \param ui32BufferPosition Position of buffer on screen (0-based, from left edge)
		 * \param ui32SampleIndex Index of sample in buffer
		 * \param f64XOffset X offset from which to start drawing. Used in scroll mode only.
		 * \return X coordinate of sample
		 */
		OpenViBE::float64 getSampleXCoordinate(
			OpenViBE::uint32 ui32BufferPosition,
			OpenViBE::uint32 ui32SampleIndex,
			OpenViBE::float64 f64XOffset);

		/**
		 * \brief Get Y coordinate of a sample
         * \param f64Value Sample value and index of channel
		 * \return Y coordinate of sample
		 */
		OpenViBE::float64 getSampleYCoordinate(
            OpenViBE::float64 f64Value, OpenViBE::uint32 ui32ChannelIndex);

        /**
         * \brief Get Y coordinate of a sample in Multiview mode
         * \param f64Value Sample value and index of channel
         * \return Y coordinate of sample
         */
        OpenViBE::float64 getSampleYMultiViewCoordinate(
            OpenViBE::float64 f64Value);

		/**
		 * \brief Draw signals (and stimulations, if any) displayed by this channel
		 * \param ui32FirstBufferToDisplay Index of first buffer to display
		 * \param ui32LastBufferToDisplay Index of last buffer to display
		 * \param ui32FirstSampleToDisplay Index of first sample to display in first buffer (subsequent buffers will start at sample 0)
		 * \param f64BaseX X offset to apply to signals (can be non null in scroll mode)
		 * \return True if all went ok, false otherwise
		 */
		OpenViBE::boolean drawSignals(
			OpenViBE::uint32 ui32FirstBufferToDisplay,
			OpenViBE::uint32 ui32LastBufferToDisplay,
			OpenViBE::uint32 ui32FirstSampleToDisplay,
			OpenViBE::float64 f64FirstBufferStartX,
			OpenViBE::uint32 ui32FirstChannelToDisplay, 
			OpenViBE::uint32 ui32LastChannelToDisplay);

		/**
		 * \brief Draw vertical line highlighting where data was last drawn
		 * \param ui32X X coordinate of line
		 */
		void drawProgressLine(
			OpenViBE::uint32 ui32FirstBufferToDisplay,
			OpenViBE::uint32 ui32FirstBufferToDisplayPosition);

		/**
		 * \brief Draw Y=0 line
		 */
		void drawZeroLine();

	public:
        //! Vector of Left rulers displaying signal scale. Indexed by channel id. @note This is a map as the active number of channels 
		// may change by the toolbar whereas this total set of rulers doesn't...
        std::map<OpenViBE::uint32, CSignalDisplayLeftRuler* > m_oLeftRuler;
		//! The drawing area where the signal is to be drawn
		GtkWidget * m_pDrawingArea;
		//! Drawing area dimensions, in pixels
		OpenViBE::uint32 m_ui32Width, m_ui32Height;
		//! Available width per buffer, in pixels
		OpenViBE::float64 m_f64WidthPerBuffer;
		//! Available width per sample point, in pixels
		OpenViBE::float64 m_f64PointStep;
		//! The index list of the channels to display
		std::vector<OpenViBE::uint32> m_oChannelList;
		//! The "parent" view (which uses this widget)
		CSignalDisplayView * m_pParentDisplayView;
		//! The database from which the information are to be read
		CBufferDatabase * m_pDatabase;

        /** \ name Extrema of displayed values for all channel in this display */
		//@{
        std::vector<OpenViBE::float64> m_vLocalMaximum;
        std::vector<OpenViBE::float64> m_vLocalMinimum;
		//@}

		/** \name Auto scaling parameters */
		//@{
//		OpenViBE::float64 m_f64ScaleX;
//		OpenViBE::float64 m_f64TranslateX;

        std::vector<OpenViBE::float64> m_vScaleY;
        std::vector<OpenViBE::float64> m_vTranslateY;
		//@}

		/** \name Zooming parameters (user controlled) */
		//@{
		OpenViBE::float64 m_f64ZoomTranslateX;
		OpenViBE::float64 m_f64ZoomTranslateY;
		OpenViBE::float64 m_f64ZoomScaleX;
		OpenViBE::float64 m_f64ZoomScaleY;
		//! The zoom factor step
		const OpenViBE::float64 m_f64ZoomFactor;
		//@}

		/** \name Scale margin parameters */
		//@{
        std::vector<OpenViBE::float64> m_vMaximumTopMargin;
        std::vector<OpenViBE::float64> m_vMaximumBottomMargin;
        std::vector<OpenViBE::float64> m_vMinimumTopMargin;
        std::vector<OpenViBE::float64> m_vMinimumBottomMargin;
		//@}

		OpenViBE::uint32 m_i32LeftRulerWidthRequest, m_i32LeftRulerHeightRequest;

		//! Current signal display mode
		OpenViBEPlugins::SimpleVisualisation::EDisplayMode m_eCurrentSignalMode;
		//! Time of latest displayed data
		OpenViBE::uint64 m_ui64LatestDisplayedTime;
		//! Should the whole window be redrawn at next redraw?
		OpenViBE::boolean m_bRedrawAll;

        //! Is it a multiview display ?
        OpenViBE::boolean m_bMultiView;

		// These parameters control that we don't unnecessarily draw parts of the signal which are not in view
		
		// Currently visible y segment in the drawing area
		OpenViBE::uint32 m_ui32StartY;
		OpenViBE::uint32 m_ui32StopY;

		// First and last channel to draw
		OpenViBE::uint32 m_ui32FirstChannelToDisplay;
		OpenViBE::uint32 m_ui32LastChannelToDisplay;

	};

	}
}

#endif
