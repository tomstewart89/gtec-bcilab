#ifndef __OpenViBEAcquisitionServer_CHeaderBrainProductsVAmp_H__
#define __OpenViBEAcquisitionServer_CHeaderBrainProductsVAmp_H__

#if defined TARGET_HAS_ThirdPartyUSBFirstAmpAPI

#include "../ovasCHeader.h"

#include <windows.h>
#include <FirstAmp.h>

#include<map>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CHeaderBrainProductsVAmp
	 * \author Laurent Bonnet (INRIA)
	 * \date 16 nov 2009
	 * \erief The CHeaderBrainProductsVAmp is an Adaptator for the VAmp device.
	 *
	 * It contains basic functions using the basic header behaviour, and a set of specific functions to handle the Fast Mode data.
	 *
	 * \sa CDriverBrainProductsVAmp
	 */
	class CHeaderBrainProductsVAmp : public OpenViBEAcquisitionServer::IHeader
	{
	public:

		CHeaderBrainProductsVAmp(void);
		virtual ~CHeaderBrainProductsVAmp(void);
		virtual void reset(void);

		// Experiment information
		virtual OpenViBE::boolean setExperimentIdentifier(const OpenViBE::uint32 ui32ExperimentIdentifier);
		virtual OpenViBE::boolean setSubjectAge(const OpenViBE::uint32 ui32SubjectAge);
		virtual OpenViBE::boolean setSubjectGender(const OpenViBE::uint32 ui32SubjectGender);

		virtual OpenViBE::uint32 getExperimentIdentifier(void) const;
		virtual OpenViBE::uint32 getSubjectAge(void) const;
		virtual OpenViBE::uint32 getSubjectGender(void) const;

		virtual OpenViBE::boolean isExperimentIdentifierSet(void) const;
		virtual OpenViBE::boolean isSubjectAgeSet(void) const;
		virtual OpenViBE::boolean isSubjectGenderSet(void) const;

		// Channel information
		virtual OpenViBE::boolean setChannelCount(const OpenViBE::uint32 ui32ChannelCount);
		virtual OpenViBE::boolean setChannelName(const OpenViBE::uint32 ui32ChannelIndex, const char* sChannelName);
		virtual OpenViBE::boolean setChannelGain(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::float32 f32ChannelGain);
		virtual OpenViBE::boolean setChannelUnits(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::uint32 ui32ChannelUnit, const OpenViBE::uint32 ui32ChannelFactor);

		virtual OpenViBE::uint32 getChannelCount(void) const;
		virtual const char* getChannelName(const OpenViBE::uint32 ui32ChannelIndex) const;
		virtual OpenViBE::float32 getChannelGain(const OpenViBE::uint32 ui32ChannelIndex) const;
		virtual OpenViBE::boolean getChannelUnits(const OpenViBE::uint32 ui32ChannelIndex, OpenViBE::uint32& ui32ChannelUnit, OpenViBE::uint32& ui32ChannelFactor) const;

		virtual OpenViBE::boolean isChannelCountSet(void) const;
		virtual OpenViBE::boolean isChannelNameSet(void) const;
		virtual OpenViBE::boolean isChannelGainSet(void) const;
		virtual OpenViBE::boolean isChannelUnitSet(void) const;

		// Samples information
		virtual OpenViBE::boolean setSamplingFrequency(const OpenViBE::uint32 ui32SamplingFrequency);

		virtual OpenViBE::uint32 getSamplingFrequency(void) const;

		virtual OpenViBE::boolean isSamplingFrequencySet(void) const;

		//------------- SPECIFIC FUNCTIONS -------------

		virtual OpenViBE::uint32 getAcquisitionMode(void) { return m_ui32AcquisitionMode; }
		virtual OpenViBE::boolean setAcquisitionMode(OpenViBE::uint32 ui32AcquisitionMode) { m_ui32AcquisitionMode = ui32AcquisitionMode; return true; }

		virtual OpenViBE::uint32 getEEGChannelCount(OpenViBE::uint32 ui32AcquisitionMode);
		virtual OpenViBE::uint32 getAuxiliaryChannelCount(OpenViBE::uint32 ui32AcquisitionMode);
		virtual OpenViBE::uint32 getTriggerChannelCount(OpenViBE::uint32 ui32AcquisitionMode);

		// Pair information
		virtual OpenViBE::boolean setPairCount(const OpenViBE::uint32 ui32PairCount);
		virtual OpenViBE::boolean setPairName(const OpenViBE::uint32 ui32PairIndex, const char* sPairName);
		virtual OpenViBE::boolean setPairGain(const OpenViBE::uint32 ui32PairIndex, const OpenViBE::float32 f32PairGain);
		virtual OpenViBE::boolean setPairUnits(const OpenViBE::uint32 ui32PairIndex, const OpenViBE::uint32 ui32PairUnit, const OpenViBE::uint32 ui32PairFactor);
		virtual OpenViBE::boolean setDeviceId(OpenViBE::int32 i32DeviceId);
		virtual OpenViBE::boolean setFastModeSettings(t_faDataModeSettings tFastModeSettings);

		virtual OpenViBE::uint32 getPairCount(void) const;
		virtual const char* getPairName(const OpenViBE::uint32 ui32PairIndex) const;
		virtual OpenViBE::float32 getPairGain(const OpenViBE::uint32 ui32PairIndex) const;
		virtual OpenViBE::boolean getPairUnits(const OpenViBE::uint32 ui32PairIndex, OpenViBE::uint32& ui32PairUnit, OpenViBE::uint32& ui32PairFactor) const;
		virtual OpenViBE::int32 getDeviceId(void) const;
		virtual t_faDataModeSettings getFastModeSettings(void) const;

		virtual OpenViBE::boolean isPairCountSet(void) const;
		virtual OpenViBE::boolean isPairNameSet(void) const;
		virtual OpenViBE::boolean isPairGainSet(void) const;
		virtual OpenViBE::boolean isPairUnitSet(void) const;
		virtual OpenViBE::boolean isDeviceIdSet(void) const;
		virtual OpenViBE::boolean isFastModeSettingsSet(void) const;

		OpenViBEAcquisitionServer::CHeader* getBasicHeader() const { return m_pBasicHeader; } ;

	protected:

		OpenViBEAcquisitionServer::CHeader* m_pBasicHeader; // the basic header

		// additional information
		OpenViBE::int32 m_i32DeviceId;
		OpenViBE::uint32 m_ui32AcquisitionMode;
		t_faDataModeSettings m_tFastModeSettings;

		// Pair information
		OpenViBE::uint32 m_ui32PairCount;
		std::map<OpenViBE::uint32, std::string> m_vPairName;
		std::map<OpenViBE::uint32, OpenViBE::float32> m_vPairGain;
		std::map<OpenViBE::uint32, std::pair<OpenViBE::uint32, OpenViBE::uint32> > m_vPairUnit;
	};
};

#endif // TARGET_HAS_ThirdPartyGUSBampCAPI

#endif // __OpenViBEAcquisitionServer_CHeaderBrainProductsVAmp_H__
