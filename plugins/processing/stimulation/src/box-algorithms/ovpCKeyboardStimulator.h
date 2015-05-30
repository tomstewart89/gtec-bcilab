#ifndef __OpenViBEPlugins_Stimulation_CKeyboardStimulator_H__
#define __OpenViBEPlugins_Stimulation_CKeyboardStimulator_H__

#if defined(TARGET_HAS_ThirdPartyGTK)

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <vector>
#include <map>

namespace OpenViBEPlugins
{
	namespace Stimulation
	{
		class CKeyboardStimulator : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CKeyboardStimulator(void);

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize();
			virtual OpenViBE::boolean uninitialize();

			virtual OpenViBE::uint64 getClockFrequency(){ return (32LL<<32); }

			virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock &rMessageClock);

			virtual OpenViBE::boolean process();

			OpenViBE::boolean parseConfigurationFile(const char * pFilename);

			virtual void processKey(guint uiKey, bool bState);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_KeyboardStimulator)

		public:

			OpenViBEToolkit::TStimulationEncoder<CKeyboardStimulator> m_oEncoder;

			::GtkWidget * m_pWidget;

			typedef struct
			{
				OpenViBE::uint64 m_ui64StimulationPress;
				OpenViBE::uint64 m_ui64StimulationRelease;
				OpenViBE::boolean m_bStatus;
			} SKey;

			//! Stores keyvalue/stimulation couples
			std::map<guint, SKey > m_oKeyToStimulation;

			//! Vector of the stimulations to send when possible
			std::vector<OpenViBE::uint64> m_oStimulationToSend;

			//! Plugin's previous activation date
			OpenViBE::uint64 m_ui64PreviousActivationTime;

			OpenViBE::boolean m_bError;

		private:
			OpenViBE::boolean m_bUnknownKeyPressed;
			OpenViBE::uint32 m_ui32UnknownKeyCode;
		};

		/**
		* Plugin's description
		*/
		class CKeyboardStimulatorDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Keyboard stimulator"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Bruno Renier"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Stimulation generator"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Sends stimulations according to key presses"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Stimulation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }
			virtual void release(void)                                   { }
			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_KeyboardStimulator; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Stimulation::CKeyboardStimulator(); }

			virtual OpenViBE::boolean hasFunctionality(OpenViBE::Kernel::EPluginFunctionality ePF) const
			{
				return ePF == OpenViBE::Kernel::PluginFunctionality_Visualization;
			}

			virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addOutput("Outgoing Stimulations", OV_TypeId_Stimulations);

				rPrototype.addSetting("Filename", OV_TypeId_Filename, "${Path_Data}/plugins/stimulation/simple-keyboard-to-stimulations.txt");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_KeyboardStimulatorDesc)
		};
	};
};

#endif

#endif

