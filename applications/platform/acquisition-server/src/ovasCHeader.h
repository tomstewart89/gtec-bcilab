#ifndef __OpenViBEAcquisitionServer_CHeader_H__
#define __OpenViBEAcquisitionServer_CHeader_H__

#include "ovasIHeader.h"

namespace OpenViBEAcquisitionServer
{
	class CHeader : public OpenViBEAcquisitionServer::IHeader
	{
	public:

		CHeader(void);
		virtual ~CHeader(void);
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

		// Chanel information
		virtual OpenViBE::boolean setChannelCount(const OpenViBE::uint32 ui32ChannelCount);
		virtual OpenViBE::boolean setChannelName(const OpenViBE::uint32 ui32ChannelIndex, const char* sChannelName);
		virtual OpenViBE::boolean setChannelGain(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::float32 f32ChannelGain);
		virtual OpenViBE::boolean setChannelUnits(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::uint32 ui32ChannelUnit, const OpenViBE::uint32 ui32ChannelFactor);
		// virtual OpenViBE::boolean setChannelLocation(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::float32 ui32ChannelLocationX, const OpenViBE::float32 ui32ChannelLocationY, const OpenViBE::float32 ui32ChannelLocationZ);

		virtual OpenViBE::uint32 getChannelCount(void) const;
		virtual const char* getChannelName(const OpenViBE::uint32 ui32ChannelIndex) const;
		virtual OpenViBE::float32 getChannelGain(const OpenViBE::uint32 ui32ChannelIndex) const;
		virtual OpenViBE::boolean getChannelUnits(const OpenViBE::uint32 ui32ChannelIndex, OpenViBE::uint32& ui32ChannelUnit, OpenViBE::uint32& ui32ChannelFactor) const;
		// virtual getChannelLocation(const OpenViBE::uint32 ui32ChannelIndex) const;

		virtual OpenViBE::boolean isChannelCountSet(void) const;
		virtual OpenViBE::boolean isChannelNameSet(void) const;
		virtual OpenViBE::boolean isChannelGainSet(void) const;
		// virtual OpenViBE::boolean isChannelLocationSet(void) const;
		virtual OpenViBE::boolean isChannelUnitSet(void) const;

		// Samples information
		virtual OpenViBE::boolean setSamplingFrequency(const OpenViBE::uint32 ui32SamplingFrequency);

		virtual OpenViBE::uint32 getSamplingFrequency(void) const;

		virtual OpenViBE::boolean isSamplingFrequencySet(void) const;

	protected:

		OpenViBEAcquisitionServer::IHeader* m_pHeaderImpl;
	};

	class CHeaderAdapter : public OpenViBEAcquisitionServer::IHeader
	{
	public:

		CHeaderAdapter(IHeader& rAdaptedHeader) : m_rAdaptedHeader(rAdaptedHeader) { }

		virtual void reset(void) { return m_rAdaptedHeader.reset(); }

		// Experiment information
		virtual OpenViBE::boolean setExperimentIdentifier(const OpenViBE::uint32 ui32ExperimentIdentifier) { return m_rAdaptedHeader.setExperimentIdentifier(ui32ExperimentIdentifier); }
		virtual OpenViBE::boolean setSubjectAge(const OpenViBE::uint32 ui32SubjectAge) { return m_rAdaptedHeader.setSubjectAge(ui32SubjectAge); }
		virtual OpenViBE::boolean setSubjectGender(const OpenViBE::uint32 ui32SubjectGender) { return m_rAdaptedHeader.setSubjectGender(ui32SubjectGender); }

		virtual OpenViBE::uint32 getExperimentIdentifier(void) const { return m_rAdaptedHeader.getExperimentIdentifier(); }
		virtual OpenViBE::uint32 getSubjectAge(void) const { return m_rAdaptedHeader.getSubjectAge(); }
		virtual OpenViBE::uint32 getSubjectGender(void) const { return m_rAdaptedHeader.getSubjectGender(); }

		virtual OpenViBE::boolean isExperimentIdentifierSet(void) const { return m_rAdaptedHeader.isExperimentIdentifierSet(); }
		virtual OpenViBE::boolean isSubjectAgeSet(void) const { return m_rAdaptedHeader.isSubjectAgeSet(); }
		virtual OpenViBE::boolean isSubjectGenderSet(void) const { return m_rAdaptedHeader.isSubjectGenderSet(); }

		// Channel information
		virtual OpenViBE::boolean setChannelCount(const OpenViBE::uint32 ui32ChannelCount) { return m_rAdaptedHeader.setChannelCount(ui32ChannelCount); }
		virtual OpenViBE::boolean setChannelName(const OpenViBE::uint32 ui32ChannelIndex, const char* sChannelName) { return m_rAdaptedHeader.setChannelName(ui32ChannelIndex, sChannelName); }
		virtual OpenViBE::boolean setChannelGain(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::float32 f32ChannelGain) { return m_rAdaptedHeader.setChannelGain(ui32ChannelIndex, f32ChannelGain); }
		virtual OpenViBE::boolean setChannelUnits(const OpenViBE::uint32 ui32ChannelIndex, const OpenViBE::uint32 ui32ChannelUnit, const OpenViBE::uint32 ui32ChannelFactor) { return m_rAdaptedHeader.setChannelUnits(ui32ChannelIndex, ui32ChannelUnit, ui32ChannelFactor); } ;

		virtual OpenViBE::uint32 getChannelCount(void) const { return m_rAdaptedHeader.getChannelCount(); }
		virtual const char* getChannelName(const OpenViBE::uint32 ui32ChannelIndex) const { return m_rAdaptedHeader.getChannelName(ui32ChannelIndex); }
		virtual OpenViBE::float32 getChannelGain(const OpenViBE::uint32 ui32ChannelIndex) const { return m_rAdaptedHeader.getChannelGain(ui32ChannelIndex); }
		virtual OpenViBE::boolean getChannelUnits(const OpenViBE::uint32 ui32ChannelIndex, OpenViBE::uint32& ui32ChannelUnit, OpenViBE::uint32& ui32ChannelFactor) const { return m_rAdaptedHeader.getChannelUnits(ui32ChannelIndex, ui32ChannelUnit, ui32ChannelFactor); } ;

		virtual OpenViBE::boolean isChannelCountSet(void) const { return m_rAdaptedHeader.isChannelCountSet(); }
		virtual OpenViBE::boolean isChannelNameSet(void) const { return m_rAdaptedHeader.isChannelNameSet(); }
		virtual OpenViBE::boolean isChannelGainSet(void) const { return m_rAdaptedHeader.isChannelGainSet(); }
		virtual OpenViBE::boolean isChannelUnitSet(void) const { return m_rAdaptedHeader.isChannelUnitSet(); }

		// Samples information
		virtual OpenViBE::boolean setSamplingFrequency(const OpenViBE::uint32 ui32SamplingFrequency) { return m_rAdaptedHeader.setSamplingFrequency(ui32SamplingFrequency); }
		virtual OpenViBE::uint32 getSamplingFrequency(void) const { return m_rAdaptedHeader.getSamplingFrequency(); }
		virtual OpenViBE::boolean isSamplingFrequencySet(void) const { return m_rAdaptedHeader.isSamplingFrequencySet(); }

	protected:

		IHeader& m_rAdaptedHeader;
	};
};

#endif // __OpenViBEAcquisitionServer_CHeader_H__
