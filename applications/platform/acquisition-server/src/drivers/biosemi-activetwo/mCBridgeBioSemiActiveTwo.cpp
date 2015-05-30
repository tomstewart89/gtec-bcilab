/*
 * mCBioSemiActiveTwoBridge.cpp
 *
 * Copyright (c) 2012, Mensia Technologies SA. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */


/*
This code implements the procedure described her:
http://www.biosemi.com/faq/make_own_acquisition_software.htm
*/

#ifdef TARGET_HAS_ThirdPartyBioSemiAPI
#include "mCBridgeBioSemiActiveTwo.h"

#include <iostream>
//debug
#include <fstream>

#ifdef WIN32
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
 #endif
 #include <windows.h>
#else
// linux, mac
#include <unistd.h>
#endif	// WIN32

#include <labview_dll.h>

using namespace Mensia;
using namespace std;

//___________________________________________________________________//
//                                                                   //
#ifdef _DEBUG
	#define __BioSemiBridgeLogConsole__(msg, ...) printf(msg, __VA_ARGS__) 
#else
	#define __BioSemiBridgeLogConsole__(msg, ...)
#endif

//___________________________________________________________________//
//                                                                   //
// Ring buffer must be large enough to hold at least a complete sample round
// otherwise we wont be able to detect the length between 2 sync bytes and deduce channel count.
// test gives 131072 bytes (32768 long) per read
// it needs to be a multiple of 512
#define BIOSEMI_ACTIVETWO_RINGBUFFERLONGCOUNT (32768*512)
#define BIOSEMI_ACTIVETWO_RINGBUFFERBYTES (BIOSEMI_ACTIVETWO_RINGBUFFERLONGCOUNT*sizeof(int)) //67108864 bytes

// The control buffer is sent through USB_WRITE. It is a 64 bytes buffer.
// First byte controls the start/stop filling of ring buffer; if Odd, it starts.
// 2nd and 3rd byte are used for outgoing triggers.
// byte 4-64 unused.
#define BIOSEMI_ACTIVETWO_CONTROLBUFFERBYTES 64

// SYNC bytes pads the stream that sends a fixed number of bytes per read() independently of the channel count.
#define BIOSEMI_ACTIVETWO_SYNCBYTES 0xffffff00

// the device can be configured by changing the speedmode "button" in front of the amplifier 0-9
#define BIOSEMI_ACTIVETWO_SPEEDMODECOUNT 10

namespace BioSemi
{
	/*
	AD-box model MK1

	AD-box switch  | Sample-rate     | Pin channels + EX channels + Sensor channels
	***************|*****************|*********************************************
	0              | 2048 (2 kHz)    | 256+0+0
	1              | 4096 (4 kHz)    | 128+0+0
	2              | 8192 (8 kHz)    | 64+0+0
	3              | 16384 (16 kHz)  | 32+0+0
	4              | 2048 (2 kHz)    | 232+8+7
	5              | 4096 (4 kHz)    | 104+8+7
	6              | 8192 (8 kHz)    | 40+8+7
	7              | 16384 (16 kHz)  | 8+8+7
	8 (AIB-mode)   | 2048 (2 kHz)    | AIB-mode
	9              | Reserved        | Reserved
	*/
	static unsigned int g_ActiveTwoMarkI_SpeedModeFrequency[BIOSEMI_ACTIVETWO_SPEEDMODECOUNT] = { 2048, 4096, 8192, 16384, 2048, 4096, 8192, 16384, 2048, 0};

	/*
	AD-box model MK2

	AD-box switch  | Sample-rate     | Pin channels + EX channels + Sensor channels
	***************|*****************|*********************************************
	0              | 2048 (2 kHz)    | >> Daisy-chain mode:
	1              | 4096 (4 kHz)    | >> In speedmode 0 to 3, the AD-boxes work as up to 4 optical fiber 'daisy chained' boxes,
	2              | 8192 (8 kHz)    | >> each with a maximum of 128+8 channels+sensors @ 2 kHz, speedmode switch = box number. (0=Box1, 1=Box2, 2=Box3, 3=Box4).
	3              | 16384 (16 kHz)  | >> Daisy chain possibilities are not standard included in the base system price
	4              | 2048 (2 kHz)    | 256+8+7
	5              | 4096 (4 kHz)    | 128+8+7
	6              | 8192 (8 kHz)    | 64+8+7
	7              | 16384 (16 kHz)  | 32+8+7
	8 (AIB-mode)   | 2048 (2 kHz)    | 256+8+7+32AIB
	9 (ABR-mode)   | 16384 (16 kHz)  | 5
	*/
	static unsigned int g_ActiveTwoMarkII_SpeedModeFrequency[BIOSEMI_ACTIVETWO_SPEEDMODECOUNT] = { 2048, 4096, 8192, 16384, 2048, 4096, 8192, 16384, 2048, 16384};

	static unsigned int * g_ActiveTwo_SpeedModeFrequency[2] = {g_ActiveTwoMarkI_SpeedModeFrequency, g_ActiveTwoMarkII_SpeedModeFrequency};

	/*
	The ActiveTwo sends the following number of 32-bit words per sample:
	
	Mk1: 
	Speedmode 0 and 4: 258 
	Speedmode 1 and 5: 130 
	Speedmode 2 and 6: 66 
	Speedmode 3 and 7: 34 
	Speedmode 8: 290 (2+256+32)
	*/
	static unsigned int g_ActiveTwoMarkI_LongPerSample[BIOSEMI_ACTIVETWO_SPEEDMODECOUNT] = {258,130,66,34,258,130,66,34,290,0};
	
	/*
	Mk2: 
	Speedmode 0, 1, 2 and 3: 610 (2+4*152) 
	Speedmode 4: 282 
	Speedmode 5: 154 
	Speedmode 6: 90 
	Speedmode 7: 58 
	Speedmode 8: 314 (2+280+32)
	*/
	static unsigned int g_ActiveTwoMarkII_LongPerSample[BIOSEMI_ACTIVETWO_SPEEDMODECOUNT] = {610,610,610,610,282,154,90,58,314,0};
	
	static unsigned int * g_ActiveTwoMark_LongPerSample[2] = {g_ActiveTwoMarkI_LongPerSample, g_ActiveTwoMarkII_LongPerSample};
	/*
	The ActiveTwo sends the following number of 32-bit words per sample:
	
	Mk1: 
	Speedmode 0 and 4: 258 
	Speedmode 1 and 5: 130 
	Speedmode 2 and 6: 66 
	Speedmode 3 and 7: 34 
	Speedmode 8: 290 (2+256+32)
	*/
	static unsigned int g_ActiveTwoMarkI_ElectrodeChannelPerSample[BIOSEMI_ACTIVETWO_SPEEDMODECOUNT] = {256,128,256,34,232,104,40,8,290,0};

	/*
	The ActiveTwo sends the following number of 32-bit words per sample:
	
	Mk2: 
	Speedmode 0 and 4: 258 
	Speedmode 1 and 5: 130 
	Speedmode 2 and 6: 66 
	Speedmode 3 and 7: 34 
	Speedmode 8: 290 (2+256+32)
	*/
	static unsigned int g_ActiveTwoMarkII_ElectrodeChannelPerSample[BIOSEMI_ACTIVETWO_SPEEDMODECOUNT] = {256,128,256,128,256,128,64,32,314,0};
	
	static unsigned int * g_ActiveTwoMark_ElectrodeChannelPerSample[2] = {g_ActiveTwoMarkI_ElectrodeChannelPerSample, g_ActiveTwoMarkII_ElectrodeChannelPerSample};

	/*
	Mk2: 
	Speedmode 0, 1, 2 and 3: 610 (2+4*152) 
	Speedmode 4: 282 
	Speedmode 5: 154 
	Speedmode 6: 90 
	Speedmode 7: 58 
	Speedmode 8: 314 (2+280+32)
	*/
	static unsigned int g_ActiveTwoMarkI_EXChannelPerSample[BIOSEMI_ACTIVETWO_SPEEDMODECOUNT] = {0,0,0,0,8,8,8,8,8,8};
	
	/*
	Mk2: 
	Speedmode 0, 1, 2 and 3: 610 (2+4*152) 
	Speedmode 4: 282 
	Speedmode 5: 154 
	Speedmode 6: 90 
	Speedmode 7: 58 
	Speedmode 8: 314 (2+280+32)
	*/
	static unsigned int g_ActiveTwoMarkII_EXChannelPerSample[BIOSEMI_ACTIVETWO_SPEEDMODECOUNT] = {8,8,8,8,8,8,8,8,8,8};
		
	static unsigned int * g_ActiveTwoMark_EXChannelPerSample[2] = {g_ActiveTwoMarkI_EXChannelPerSample, g_ActiveTwoMarkII_EXChannelPerSample};

};

//___________________________________________________________________//
//                                                                   //

CBridgeBioSemiActiveTwo::CBridgeBioSemiActiveTwo()
{
	m_vBuffer.resize(BIOSEMI_ACTIVETWO_RINGBUFFERBYTES,0);
	m_vControlBuffer.resize(BIOSEMI_ACTIVETWO_CONTROLBUFFERBYTES);
	m_uiLastError = BioSemiError_NoError;
	m_bUseEXChannels = false;
}

CBridgeBioSemiActiveTwo::~CBridgeBioSemiActiveTwo()
{
}

//___________________________________________________________________//
//                                                                   //
bool CBridgeBioSemiActiveTwo::isDeviceConnected(void)
{
	m_hDevice = OPEN_DRIVER();
	if(m_hDevice  == INVALID_HANDLE_VALUE)
	{
		__BioSemiBridgeLogConsole__("Failed to open driver!\n");
		m_uiLastError = BioSemiError_OSOpenFailed;
		return false;
	}

	Sleep(100);

	if(!CLOSE_DRIVER(m_hDevice))
	{
		__BioSemiBridgeLogConsole__("device driver cannot be closed!!\n");
		m_uiLastError = BioSemiError_OSCloseFailed;
		return false;
	}

	return true;
}


bool CBridgeBioSemiActiveTwo::open()
{
	m_uiLastRingBufferByteIndex = 0;
	m_uiConsumptionByteIndex = 0;
	m_bFirstRead = true;

	m_uiTotalByteReadCount = 0;
	
	m_uiChannelCount = 0;
	m_uiInitialChannelCount = 0;
	m_bBridgeSyncedWithDevice = false;

	m_vTriggers.clear();
	m_vTriggers.resize(16,false);
	m_bEpochStarted = false;
	m_bCMSInRange = false;
	m_bBatteryLow = false;
	m_bActiveTwoMarkII = false;
	m_bInitialActiveTwoMarkII = false;
	m_uiSpeedMode = -1;
	m_uiInitialSpeedmode = 0;

	m_hDevice = OPEN_DRIVER();
	if(m_hDevice  == INVALID_HANDLE_VALUE)
	{
		__BioSemiBridgeLogConsole__("Failed to open driver!\n");
		m_uiLastError = BioSemiError_OSOpenFailed;
		return false;
	}

	__BioSemiBridgeLogConsole__("Handle created, configuring...\n");
		
	// TEMP for debug: log file written along with the LabView_DLL
	//BSIF_SET_LOG(true);

	// default config
	//BSIF_SET_SYNC(true);
	//BSIF_SET_DEBUG(false);

	// No stride
	//BSIF_SET_STRIDE_KB(0);
	//BSIF_SET_STRIDE_MS(0);

	char l_pBufferInfo[256];
	GET_DRIVER_INFO(l_pBufferInfo, 256);
	__BioSemiBridgeLogConsole__("Driver Info: %s", l_pBufferInfo);

	__BioSemiBridgeLogConsole__("connecting ring buffer ...\n");
	READ_MULTIPLE_SWEEPS(m_hDevice, &m_vBuffer[0], BIOSEMI_ACTIVETWO_RINGBUFFERBYTES);

	__BioSemiBridgeLogConsole__("device driver now opened.\n");
	return true;
}

bool CBridgeBioSemiActiveTwo::start()
{
	m_vControlBuffer.clear();
	m_vControlBuffer.resize(BIOSEMI_ACTIVETWO_CONTROLBUFFERBYTES);
	m_vControlBuffer[0] = static_cast<char>(-1); // Odd number to start the filling of ring buffer

	BOOL l_bStatus = false;
	
	l_bStatus = USB_WRITE(m_hDevice, &m_vControlBuffer[0]);
	if(!l_bStatus)
	{
		__BioSemiBridgeLogConsole__("usb_write for enable handshake failed with [%i]\n", GetLastError());
		m_uiLastError = BioSemiError_EnableUSBHandshakeFailed;
		return false;
	}

	// Past this point, the ring buffer is constantly filled.
	return true;
}

int CBridgeBioSemiActiveTwo::read()
{
	int l_iBytesRead;
	int l_iCurrentRingBufferByteIndex;
	bool l_bStatus = (READ_POINTER(m_hDevice, &l_iCurrentRingBufferByteIndex) != FALSE);
	
	if(!l_bStatus)
	{
		__BioSemiBridgeLogConsole__("READ_POINTER failed !!\n");
		m_uiLastError = BioSemiError_ReadPointerFailed;
		return -1;
	}

	if(l_iCurrentRingBufferByteIndex != m_uiLastRingBufferByteIndex)
	{
		l_iBytesRead = l_iCurrentRingBufferByteIndex - m_uiLastRingBufferByteIndex;
		
		// ring buffer: we can loop indices
		if(l_iBytesRead < 0)
		{
			l_iBytesRead += BIOSEMI_ACTIVETWO_RINGBUFFERBYTES;
			__BioSemiBridgeLogConsole__("RING BUFFER IS ROLLING\n");
		}
		
		//__BioSemiBridgeLogConsole__("READ_POINTER read %i bytes\n",l_iBytesRead);

		m_uiTotalByteReadCount += static_cast<unsigned int>(l_iBytesRead);
		m_uiLastRingBufferByteIndex = l_iCurrentRingBufferByteIndex;
		
		// determine number of channels according to sync bytes
		// Two extra channels are leading the ADC data: before: channel 1 = sync (check for FFFFFF00) and channel 2 = Status
		// (see http://www.biosemi.com/faq/trigger_signals.htm), channels 3-258 are ADCs 1-256.

		unsigned int l_iFirstSyncByte = -1;
		unsigned int l_iNextSyncByte = -1;

		// for loop over LONG values, not bytes
		for (int i=0; i<l_iBytesRead && !m_bBridgeSyncedWithDevice; i++)
		{
			if(*(reinterpret_cast<int*>(&m_vBuffer[i])) == BIOSEMI_ACTIVETWO_SYNCBYTES)
			{
				__BioSemiBridgeLogConsole__("sync detected at byte %i\n",i);
				if(l_iFirstSyncByte == -1)
				{
					l_iFirstSyncByte = i;
				}
				else
				{
					//second sync found, we can deduce the channel count: it's the number of longs in between
					l_iNextSyncByte = i;
					m_uiChannelCount = l_iNextSyncByte - l_iFirstSyncByte;
					m_uiChannelCount /= sizeof(int);
					m_uiChannelCount -= 2; 
					

					// we also initialize the status values for getSamplingFrequency
					int l_iStatusChannelValue = *(reinterpret_cast<int*>(&m_vBuffer[l_iFirstSyncByte + sizeof(int)])); // status integer just after sync integer
					if(!this->updateStatusFromValue(l_iStatusChannelValue))
					{
						__BioSemiBridgeLogConsole__("something bad in latest status value !\n");
						return -1;
					}

					// Bridge is synced, user can call status getters
					m_bBridgeSyncedWithDevice = true;
					
					//consuming all sync samples
					//this->consumeBytes(sizeof(int) * (getSampleCount() - m_uiChannelCount - 1));
					
					// The Handsake is complete; we can safely consume the corresponding data
					// channel# + SYNC (Status is consumed in updateStatusFromValue function)
					this->consumeBytes((m_uiChannelCount + 1) * sizeof(int));

					// fix number of channel
					
				}
			}
		}

		if(!m_bBridgeSyncedWithDevice)
		{
			__BioSemiBridgeLogConsole__("Cannot synchronize on current data, waiting for more.\n");
			return 0;
		}
		
		//debug
		/*fstream fs("C:/biosemi-buffer.data",fstream::app);
		fs<< "START----------------------------------------------";
		for(int i=0; i<l_iBytesRead; i+=4)
			fs << hex << *(reinterpret_cast<int*>(&m_vBuffer[i]))  << endl;
		fs.close();
		fs<< "END----------------------------------------------";*/
		
		if(m_bFirstRead)
		{
			m_uiInitialChannelCount =  m_bUseEXChannels ? m_uiChannelCount + getEXChannelCount(): m_uiChannelCount;
			m_uiInitialSpeedmode = m_uiSpeedMode;
			m_bInitialActiveTwoMarkII = m_bActiveTwoMarkII;
			m_bFirstRead = false;

			// Consume what remains of data
			// samples# - channel# - SYNC
			this->consumeBytes(sizeof(int) * (getSampleCount() - m_uiChannelCount - 1));

			m_uiChannelCount = m_uiChannelCount < getElectrodeChannelCount() ? m_uiChannelCount : getElectrodeChannelCount();
			__BioSemiBridgeLogConsole__("Bridge sync! Initial configuration is: %u channels | speedmode %u | Mark2 %u | Sample count %u | electrode channel count %u | EX channel count: %u \n", 
				m_uiInitialChannelCount, m_uiInitialSpeedmode, m_bInitialActiveTwoMarkII, getSampleCount(), getElectrodeChannelCount(), getEXChannelCount() );
		}
		return l_iBytesRead;
	}
	
	// nothing read
	//__BioSemiBridgeLogConsole__("READ_POINTER has nothing for us.\n)";
	return 0;
	
}

//___________________________________________________________________//
//                                                                   //

bool CBridgeBioSemiActiveTwo::updateStatusFromValue(int iStatusChannelValue)
{
	/*
	Bit 00 (LSB)    Trigger Input 1 (High = trigger on)
	Bit 01          Trigger Input 2 (High = trigger on)
	Bit 02          Trigger Input 3 (High = trigger on)
	Bit 03          Trigger Input 4 (High = trigger on)
	Bit 04          Trigger Input 5 (High = trigger on)
	Bit 05          Trigger Input 6 (High = trigger on)
	Bit 06          Trigger Input 7 (High = trigger on)
	Bit 07          Trigger Input 8 (High = trigger on)
	Bit 08          Trigger Input 9 (High = trigger on)
	Bit 09          Trigger Input 10 (High = trigger on)
	Bit 10          Trigger Input 11 (High = trigger on)
	Bit 11          Trigger Input 12 (High = trigger on)
	Bit 12          Trigger Input 13 (High = trigger on)
	Bit 13          Trigger Input 14 (High = trigger on)
	Bit 14          Trigger Input 15 (High = trigger on)
	Bit 15          Trigger Input 16 (High = trigger on)
	Bit 16          High when new Epoch is started
	Bit 17          Speed bit 0
	Bit 18          Speed bit 1
	Bit 19          Speed bit 2
	Bit 20          High when CMS is within range ---> error in the documentation, it is the opposite
	Bit 21          Speed bit 3
	Bit 22          High when battery is low
	Bit 23 (MSB)    High if ActiveTwo MK2
	*/
	//__BioSemiBridgeLogConsole__("status line value is [%#x]\n",iStatusChannelValue);
	// the status channel has 2 bytes padding (zeros), so we start at 0x100
	for(unsigned int t = 0; t<16; t++)
	{
		m_vTriggers[t] = (iStatusChannelValue & (0x100 << t)) != 0;
	}
						
	m_bEpochStarted = (iStatusChannelValue & (0x100 << 16)) != 0;
	m_bCMSInRange = !(iStatusChannelValue & (0x100 << 20)) != 0; // bit low = CM in range !
	m_bBatteryLow = (iStatusChannelValue & (0x100 << 22)) != 0;
	m_bActiveTwoMarkII = (iStatusChannelValue & (0x100 << 23)) != 0;
	m_uiSpeedMode = ((iStatusChannelValue & (0x100 << 17))!= 0)
		+ (((iStatusChannelValue & (0x100 << 18))!= 0) << 1)
		+ (((iStatusChannelValue & (0x100 << 19))!= 0) << 2);
//		+ (((iStatusChannelValue & (0x100 << 21))!= 0) << 3);
	
	this->consumeBytes(sizeof(int));
	
	// fail cases:
	if(!m_bFirstRead && m_bActiveTwoMarkII != m_bInitialActiveTwoMarkII)
	{
		__BioSemiBridgeLogConsole__("Device type changed during acquisition! Stream may have lost synchronization.\n");
		m_uiLastError = BioSemiError_DeviceTypeChanged;
		return false;
	}
	if(!m_bFirstRead && m_uiSpeedMode != m_uiInitialSpeedmode)
	{
		__BioSemiBridgeLogConsole__("Speedmode changed during acquisition (%u > %u). Stream may have lost synchronization.\n", m_uiInitialSpeedmode, m_uiSpeedMode);
		m_uiLastError = BioSemiError_SpeedmodeChanged;
		return false;
	}
	if(m_uiSpeedMode >= BIOSEMI_ACTIVETWO_SPEEDMODECOUNT)
	{
		__BioSemiBridgeLogConsole__("Invalid speedmode [%u].\n",m_uiSpeedMode);
		m_uiLastError = BioSemiError_InvalidSpeedmode;
		return false;
	}

	return true;
}

unsigned int CBridgeBioSemiActiveTwo::getSamplingFrequency()
{
	return BioSemi::g_ActiveTwo_SpeedModeFrequency[(m_bActiveTwoMarkII ? 1 : 0)][m_uiSpeedMode];
}

//___________________________________________________________________//
//                                                                   //

bool CBridgeBioSemiActiveTwo::discard(void)
{
	unsigned int l_uiSampleCount = this->getAvailableSampleCount();
	for(unsigned int i = 0; i < l_uiSampleCount; i++)
	{
		if(!this->consumeOneSamplePerChannel(NULL, 0))
		{
			__BioSemiBridgeLogConsole__("the sample %u/%u [%u] cannot be discarded ! stopping discard.\n", i, l_uiSampleCount, m_uiConsumptionByteIndex);
			return false;
		}
	}
	return true;
}

unsigned int CBridgeBioSemiActiveTwo::getAvailableByteCount(void) 
{
	if(m_uiLastRingBufferByteIndex> m_uiConsumptionByteIndex)
	{
		return m_uiLastRingBufferByteIndex - m_uiConsumptionByteIndex;
	}
	else
	{
		return m_uiLastRingBufferByteIndex + (BIOSEMI_ACTIVETWO_RINGBUFFERBYTES - m_uiConsumptionByteIndex);
	}
}

unsigned int CBridgeBioSemiActiveTwo::getAvailableSampleCount(void)
{
	if(!m_bBridgeSyncedWithDevice) return 0;
	// this will be rounded
	return (this->getAvailableByteCount() / (sizeof(int) * getSampleCount()));
}

void CBridgeBioSemiActiveTwo::consumeBytes(unsigned int uiByteToConsume) 
{
	/*fstream fs("C:/biosemi-consume.txt",fstream::app);
	fs <<dec<< uiByteToConsume<< " : " << m_uiConsumptionByteIndex << endl;
	fs.close();*/

	m_uiConsumptionByteIndex = (m_uiConsumptionByteIndex + uiByteToConsume) % BIOSEMI_ACTIVETWO_RINGBUFFERBYTES;
}

bool CBridgeBioSemiActiveTwo::consumeOneSamplePerChannel(float* pSampleBuffer, unsigned int uiBufferValueCount)
{
	// fail case: we can't consume if not yet sync'ed
	if(!m_bBridgeSyncedWithDevice)
	{
		__BioSemiBridgeLogConsole__("Bridge is not synced with the device, cannot consume.\n");
		m_uiLastError = BioSemiError_NoSync;
		return false;
	}

	//fail case: we do nothing if we cannot completly fill the user buffer
	if(this->getAvailableByteCount() < uiBufferValueCount * sizeof(int))
	{
		__BioSemiBridgeLogConsole__("Not enough data to complete user buffer. Waiting for next read & consume run.\n");
		m_uiLastError = BioSemiError_NotEnoughDataInBuffer;
		return false;
	}

/*	if(*(reinterpret_cast<int*>(&m_vBuffer[m_uiConsumptionByteIndex])) != BIOSEMI_ACTIVETWO_SYNCBYTES)
	{
		__BioSemiBridgeLogConsole__("Lost sync at consume time ! byte index is [%i]\n",m_uiConsumptionByteIndex);
		m_uiLastError = BioSemiError_SyncLost;
		return false;
	}
*/	
	/*
	the LongPerSample value gives the number of integers that the device sends per sample round.
	For example with the Mark II and a speedmode = 4, we have 282 Integers per sample.
	Say we have 8 channels connected, we end up with 282 integers with SYNC forward padding:
	- 273 SYNC
	- 1 Integer for the Status
	- 8 Integers for the 8 channels
	*/	
	//this->consumeBytes(sizeof(int) * (getSampleCount() - m_uiChannelCount - 1));

	// We  consume the integer from STATUS channel.
	int l_iStatusChannelValue = *(reinterpret_cast<int*>(&m_vBuffer[m_uiConsumptionByteIndex]));
	if(!this->updateStatusFromValue(l_iStatusChannelValue))
	{
		__BioSemiBridgeLogConsole__("something bad in latest status value !\n");
		return false;
	}

	// EEG channels fill the user buffer.
	if(pSampleBuffer != NULL)
	{
		/*
		The receiver converts every 24-bit word from the AD-box into a 32-bit Signed Integer,
		by adding an extra zero Least Significant Byte to the ADC data.
		The 24-bit ADC output has an LSB value of 1/32th uV.
		The 32-bit Integer received for the USB interface has an LSB value of 1/32*1/256 = 1/8192th uV
		*/
		float l_fFactor = 1.f/8192;

		unsigned int l_uiSampleLongConsumed = 0;
		unsigned int l_uiSampleInBuffer = 0;
		unsigned int l_uiMaxEEGChannel = isUseEXChannels() ? uiBufferValueCount - getEXChannelCount() : uiBufferValueCount;
		while(l_uiSampleInBuffer < l_uiMaxEEGChannel)
		{
			int l_iValue = *(reinterpret_cast<int*>(&m_vBuffer[m_uiConsumptionByteIndex]));
		
			// We don't add more samples that the number that should be displayed, 
			// to avoid displaying an electrode channel instaed of an EX channel
			if(l_uiSampleInBuffer < m_uiChannelCount)
			{
				pSampleBuffer[l_uiSampleInBuffer] = l_iValue * l_fFactor;
			}
			else
			{
				// If the number of requested channel is superior to the number of available channels, pad with 0
				pSampleBuffer[l_uiSampleInBuffer] = 0;
			}

			l_uiSampleInBuffer++;
			l_uiSampleLongConsumed++;
			this->consumeBytes(sizeof(int));
		}
		if(m_bUseEXChannels)
		{
			// Consuming all sync samples until EX channels
			// ELECTRODE CHANNEL # - CHANNEL#
			this->consumeBytes(sizeof(int) * (getElectrodeChannelCount() - l_uiMaxEEGChannel) );
			while(l_uiSampleLongConsumed < l_uiMaxEEGChannel + getEXChannelCount())
			{
				int l_iValue = *(reinterpret_cast<int*>(&m_vBuffer[m_uiConsumptionByteIndex]));
				if(l_uiSampleInBuffer < uiBufferValueCount)
				{
					pSampleBuffer[l_uiSampleInBuffer] = l_iValue * l_fFactor;
					l_uiSampleInBuffer++;
				}
				l_uiSampleLongConsumed++;
				this->consumeBytes(sizeof(int));
			}
			// Consuming all remaining samples (sensor channels)
			// SAMPLE# - ELECTRODE CHANNEL# - EX CHANNEL# - SYNC
			this->consumeBytes(sizeof(int) * (getSampleCount() - (getElectrodeChannelCount() + getEXChannelCount() + 1)));
		} 
		else 
		{
			// Consuming all sync samples plus EX and sensor channels
			// SAMPLE# - CHANNEL# - SYNC
			this->consumeBytes(sizeof(int) * (getSampleCount() - l_uiMaxEEGChannel - 1));
		}
	}
	//if pSample == NULL
	else
	{
		// Consuming all sync samples plus EX and sensor channels
		// SAMPLE# - CHANNEL# - SYNC
		this->consumeBytes(sizeof(int) * (getSampleCount() - 1));
	}
	/*if(*(reinterpret_cast<int*>(&m_vBuffer[m_uiConsumptionByteIndex])) != BIOSEMI_ACTIVETWO_SYNCBYTES)
	{
		__BioSemiBridgeLogConsole__("Lost sync at consume time ! byte index is [%i]\n",m_uiConsumptionByteIndex);
		m_uiLastError = BioSemiError_SyncLost;
		return false;
	}*/
	//this->consumeBytes(sizeof(int) * m_uiChannelCount);

	//__BioSemiBridgeLogConsole__("consumed "<<l_uiByteConsumed<<" bytes this round.\n";
	return true;
}


unsigned int CBridgeBioSemiActiveTwo::getElectrodeChannelCount(void)
{
	if(!m_bBridgeSyncedWithDevice) return 0;
	// this will be rounded
	return (BioSemi::g_ActiveTwoMark_ElectrodeChannelPerSample[(m_bActiveTwoMarkII ? 1 : 0)][m_uiSpeedMode]);
}

unsigned int CBridgeBioSemiActiveTwo::getEXChannelCount(void)
{
	if(!m_bUseEXChannels) return 0;
	else if(!m_bBridgeSyncedWithDevice) return 0;
	// this will be rounded
	return (BioSemi::g_ActiveTwoMark_EXChannelPerSample[(m_bActiveTwoMarkII ? 1 : 0)][m_uiSpeedMode]);
}

unsigned int CBridgeBioSemiActiveTwo::getSampleCount(void)
{
	if(!m_bBridgeSyncedWithDevice) return 0;
	// this will be rounded
	return (BioSemi::g_ActiveTwoMark_LongPerSample[(m_bActiveTwoMarkII ? 1 : 0)][m_uiSpeedMode]);
}

//___________________________________________________________________//
//                                                                   //

bool CBridgeBioSemiActiveTwo::stop()
{
	//float bytesPerMsec = GET_BYTES_PER_MSEC();
	//__BioSemiBridgeLogConsole__("STATS: Bytes per ms: " << bytesPerMsec << endl;

	// To stop, we disable the handshake
	m_vControlBuffer.clear();
	m_vControlBuffer.resize(BIOSEMI_ACTIVETWO_CONTROLBUFFERBYTES);
	m_vControlBuffer[0] = 0; // stop byte

	BOOL l_bStatus =  USB_WRITE(m_hDevice, &m_vControlBuffer[0]);
	if(!l_bStatus)
	{
		__BioSemiBridgeLogConsole__("usb_write for disabling handshake failed with [%i]\n", GetLastError());
		m_uiLastError = BioSemiError_DisableUSBHandshakeFailed;
		return false;
	}

	return true;
}

bool CBridgeBioSemiActiveTwo::close()
{

	if(!CLOSE_DRIVER(m_hDevice))
	{
		__BioSemiBridgeLogConsole__("device driver cannot be closed!!\n");
		m_uiLastError = BioSemiError_OSCloseFailed;
		return false;
	}

	__BioSemiBridgeLogConsole__("device driver now closed.\n");
	return true;
}


#endif
