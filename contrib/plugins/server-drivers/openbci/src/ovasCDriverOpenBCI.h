/*
 * OpenBCI driver for OpenViBE
 *
 * \author Jeremy Frey
 *
 * \note Based on OpenEEG code; inherits its AGPL3 license conditions
 *
 */
#ifndef __OpenViBE_AcquisitionServer_CDriverOpenBCI_H__
#define __OpenViBE_AcquisitionServer_CDriverOpenBCI_H__

#include "ovasIDriver.h"
#include "../ovasCHeader.h"

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

#if defined TARGET_OS_Windows
 typedef void * FD_TYPE;
#elif defined TARGET_OS_Linux
 typedef OpenViBE::int32 FD_TYPE;
#else
#endif

#include <vector>
 
// some constants related to the boardWriteAndPrint
#define ADS1299_VREF 4.5  // reference voltage for ADC in ADS1299.  set by its hardware
#define ADS1299_GAIN 24.0  //assumed gain setting for ADS1299.  set by its Arduino code

// wait a little before before new writings are initiated (ms)
#define SLEEP_BEFORE_WRITE 500

// start and stop bytes from OpenBCI protocl
#define SAMPLE_START_BYTE 0xA0
#define SAMPLE_STOP_BYTE 0xC0
 
namespace OpenViBEAcquisitionServer
{
	class CDriverOpenBCI : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverOpenBCI(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual void release(void);
		virtual const char* getName(void);

		virtual OpenViBE::boolean initialize(
			const OpenViBE::uint32 ui32SampleCountPerSentBlock,
			OpenViBEAcquisitionServer::IDriverCallback& rCallback);
		virtual OpenViBE::boolean uninitialize(void);

		virtual OpenViBE::boolean start(void);
		virtual OpenViBE::boolean stop(void);
		virtual OpenViBE::boolean loop(void);

		virtual OpenViBE::boolean isConfigurable(void);
		virtual OpenViBE::boolean configure(void);
		virtual const OpenViBEAcquisitionServer::IHeader* getHeader(void) { return &m_oHeader; }
		
		// we're not quite ready yet
		virtual OpenViBE::boolean isFlagSet(
			const OpenViBEAcquisitionServer::EDriverFlag eFlag) const
		{
			return eFlag==DriverFlag_IsUnstable;
		}

	protected:

		OpenViBE::int32 interpret24bitAsInt32(std::vector < OpenViBE::uint8 > byteBuffer);
		OpenViBE::int32 interpret16bitAsInt32(std::vector < OpenViBE::uint8 > byteBuffer);
		OpenViBE::int16 parseByte(OpenViBE::uint8 ui8Actbyte);
		OpenViBE::boolean initTTY(::FD_TYPE * pFileDescriptor, OpenViBE::uint32 ui32TtyNumber);
		
		OpenViBE::boolean boardWriteAndPrint(::FD_TYPE i32FileDescriptor, const char *cmd, OpenViBE::boolean waitForResponse, OpenViBE::uint32 sleepBetween);
		OpenViBE::boolean initBoard(::FD_TYPE i32FileDescriptor);
		void fastReco(::FD_TYPE i32FileDescriptor);
		OpenViBE::int32 readPacketFromTTY(::FD_TYPE i32FileDescriptor);
		void closeTTY(::FD_TYPE i32FileDescriptor);
		OpenViBE::boolean handleCurrentSample(OpenViBE::int32 packetNumber); // will take car of samples fetch from OpenBCI board, dropping/merging packets if necessary
		void updateDaisy(OpenViBE::boolean bBeQuiet); // update internal state regarding daisy module
		
	protected:

		SettingsHelper m_oSettings;

		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;
		OpenViBEAcquisitionServer::CHeader m_oHeader;

		OpenViBE::uint32 m_ui32ChannelCount;
		OpenViBE::uint32 m_ui32DeviceIdentifier;
		OpenViBE::CString m_sComInit; // string to send possibly upon initialisation
		OpenViBE::uint32 m_ui32ComDelay; // parameter com init string
		OpenViBE::boolean m_bDaisyModule; // daisy module attached or not
		OpenViBE::float32* m_pSample;

		::FD_TYPE  m_i32FileDescriptor;
		
		// OpenBCI protocol related
		OpenViBE::int16 m_i16SampleNumber; // returned by the board
		OpenViBE::uint16 m_ui16Readstate; // position in the sample (see doc)
		OpenViBE::uint8 m_ui8SampleBufferPosition;// position in the buffer
		std::vector < OpenViBE::uint8 > m_vEEGValueBuffer; // buffer for one EEG value (int24)
		const static OpenViBE::uint8 EEGValueBufferSize = 3; // int24 == 3 bytes
		std::vector < OpenViBE::uint8 > m_vAccValueBuffer; // buffer for one accelerometer value (int16)
		const static OpenViBE::uint8 AccValueBufferSize = 2; // int16 == 2 bytes
		const static OpenViBE::uint8 EEGNbValuesPerSample = 8; // the board send EEG values 8 by 8 (will concatenate 2 samples with daisy module)
		const static OpenViBE::uint8 AccNbValuesPerSample = 3; // 3 accelerometer data per sample
		const static OpenViBE::uint16 DefaultSamplingRate = 250; // sampling rate with no daisy module (divided by 2 with daisy module)
		
		OpenViBE::uint16 m_ui16ExtractPosition; // used to situate sample reading both with EEG and accelerometer data
		
		OpenViBE::float32 ScaleFacuVoltsPerCount; // convert from int32 to microvolt
		
		OpenViBE::int16  m_i16LastPacketNumber;
		OpenViBE::uint16 m_ui16Switches;

		std::vector < std::vector < OpenViBE::float32 > > m_vChannelBuffer; // buffer to store channels & chunks
		// buffer to store sample coming from OpenBCI -- filled by parseByteP2(), passed to handleCurrentSample()
		std::vector < OpenViBE::float32 > m_vSampleEEGBuffer;
		std::vector < OpenViBE::float32 > m_vSampleAccBuffer;
		// buffers to store & merge daisy samples
		std::vector < OpenViBE::float32 > m_vSampleEEGBufferDaisy; 
		std::vector < OpenViBE::float32 > m_vSampleAccBufferDaisy;
		
		bool m_bSeenPacketFooter; // extra precaution to sync packets

		OpenViBE::CString m_sTTYName;
		
		// mechanism to call fastReco() if no data are received
		const static OpenViBE::uint32 PollingDelay = 1000; // in ms
		OpenViBE::uint32 m_ui32tick; // last tick for polling
	};
};

#endif // __OpenViBE_AcquisitionServer_CDriverOpenBCI_H__
