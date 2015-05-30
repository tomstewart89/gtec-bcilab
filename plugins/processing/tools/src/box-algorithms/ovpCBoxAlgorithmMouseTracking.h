#ifndef __OpenViBEPlugins_BoxAlgorithm_MouseTracking_H__
#define __OpenViBEPlugins_BoxAlgorithm_MouseTracking_H__

#if defined(TARGET_HAS_ThirdPartyGTK)

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <iostream>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// The unique identifiers for the box and its descriptor.
// Identifier are randomly chosen by the skeleton-generator.
#define OVP_ClassId_BoxAlgorithm_MouseTracking OpenViBE::CIdentifier(0x1E386EE5, 0x203E13C6)
#define OVP_ClassId_BoxAlgorithm_MouseTrackingDesc OpenViBE::CIdentifier(0x7A31C11B, 0xF522262E)

namespace OpenViBEPlugins
{
	namespace Tools
	{
		/**
		 * \class CBoxAlgorithmMouseTracking
		 * \author Alison Cellard (Inria)
		 * \date Mon Mar 10 15:07:21 2014
		 * \brief The class CBoxAlgorithmMouseTracking describes the box Mouse tracking.
		 *
		 */
		class CBoxAlgorithmMouseTracking : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

//			CBoxAlgorithmMouseTracking(void);
			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock& rMessageClock);		
			virtual OpenViBE::uint64 getClockFrequency(void);
			
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_MouseTracking);

		protected:
			// Feature vector stream encoder
			OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmMouseTracking > m_oAlgo0_SignalEncoder;
			// Feature vector stream encoder
			OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmMouseTracking > m_oAlgo1_SignalEncoder;

			// To check if the header was sent or not
			OpenViBE::boolean m_bHeaderSent;

			// Requested Sampling frequency
			OpenViBE::uint64 m_ui64Frequency;
			// Process clock frequency
			OpenViBE::uint64 m_ui64ClockFrequency;

			// Length of output chunks
			OpenViBE::uint64 m_ui64GeneratedEpochSampleCount;
			// Absolute coordinates of the mouse pointer, that is, relative to the window in fullscreen
			OpenViBE::IMatrix* m_pAbsoluteCoordinateBuffer;
			// Relative coordinates of the mouse pointer, the coordinates is relative to the previous point
			OpenViBE::IMatrix* m_pRelativeCoordinateBuffer;

			OpenViBE::uint64 m_ui64SentSampleCount;

			// Gtk window to track mouse position
			::GtkWidget* m_pWindow;

			// X coordinate from the previous position (in pixel, reference is upper left corner of window)
			OpenViBE::float64 m_f64Previous_x;
			// Y coordinate from the previous position (in pixel, reference is upper left corner of window)
			OpenViBE::float64 m_f64Previous_y;


		public:

			// X coordinate of mouse current position
			OpenViBE::float64 m_f64Mouse_x ;
			// Y coordinate of mouse current position
			OpenViBE::float64 m_f64Mouse_y ;

		};


		class CBoxAlgorithmMouseTrackingListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			virtual OpenViBE::boolean onSettingValueChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};
		

		/**
		 * \class CBoxAlgorithmMouseTrackingDesc
		 * \author Alison Cellard (Inria)
		 * \date Mon Mar 10 15:07:21 2014
		 * \brief Descriptor of the box Mouse tracking.
		 *
		 */
		class CBoxAlgorithmMouseTrackingDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Mouse Tracking"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Alison Cellard"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Track mouse position within the screen"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Return absolute and relative to the previous one mouse position"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Tools"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-index"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_MouseTracking; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Tools::CBoxAlgorithmMouseTracking; }
			
			
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmMouseTrackingListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }
			
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				
				rBoxAlgorithmPrototype.addOutput("Absolute coordinate",OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addOutput("Previous relative coordinate",OV_TypeId_Signal);

				rBoxAlgorithmPrototype.addSetting("Sampling Frequency",OV_TypeId_Integer,"16");
				rBoxAlgorithmPrototype.addSetting("Generated epoch sample count",OV_TypeId_Integer,"1");
				
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_MouseTrackingDesc);
		};
	};
};


#endif

#endif // __OpenViBEPlugins_BoxAlgorithm_MouseTracking_H__

