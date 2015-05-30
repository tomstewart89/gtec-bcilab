
#if defined(TARGET_HAS_ThirdPartyMCS)

#include "ovasCDriverMCSNVXDriver.h"
#include "ovasCConfigurationMCSNVXDriver.h"

#include <toolkit/ovtk_all.h>
#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

//___________________________________________________________________//
//                                                                   //

void setNVXDataSettinsDefaults(t_NVXDataSettins& ds) {
	ds.DataRate = 2;
	for (size_t i = 0; i < NVX_SELECT_CHANNELS_COUNT; ++i) {
		ds.NVXChannelsSelect.MainChannels[i] = static_cast<short>(i);
		ds.NVXChannelsSelect.DiffChannels[i] = 255;
	}
}

CDriverMKSNVXDriver::CDriverMKSNVXDriver(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_oSettings("AcquisitionServer_Driver_MKSNVXDriver", m_rDriverContext.getConfigurationManager())
	,m_pCallback(NULL)
	,m_ui32SampleCountPerSentBlock(0)
	,nvxDeviceId_(-1)
	, nvxDataModel_(NVX_DM_NORMAL)
	, dataMode_(0)
	, samplesCounter_(0)
	, triggerStates_(0)
	, showAuxChannels_(false)
{
	setNVXDataSettinsDefaults(nvxDataSettings_);
	m_oHeader.setSamplingFrequency(10000);
	m_oHeader.setChannelCount(maxNumberOfChannels);
	
	// The following class allows saving and loading driver settings from the acquisition server .conf file
	m_oSettings.add("Header", &m_oHeader);
	// To save your custom driver settings, register each variable to the SettingsHelper
	m_oSettings.add("dataMode", &dataMode_);
	m_oSettings.add("showAuxChannels", &showAuxChannels_);

	m_oSettings.load();	
}

CDriverMKSNVXDriver::~CDriverMKSNVXDriver(void)
{
}

const char* CDriverMKSNVXDriver::getName(void)
{
	return "MCS NVX amplifier";
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverMKSNVXDriver::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) return false;
	if(!m_oHeader.isChannelCountSet()||!m_oHeader.isSamplingFrequencySet()) return false;
	
	static const char* configuration = "<Configuration version=\"100\"></Configuration>";
	if (NVXAPIInit(configuration) != NVX_ERR_OK) {
		m_rDriverContext.getLogManager() << LogLevel_Error << "Cannot load NVX library.\n";
		return false;
	}
	// ...
	// initialize hardware and get
	// available header information
	// from it
	// Using for example the connection ID provided by the configuration (m_ui32ConnectionID)
	// ...
	const size_t deviceCount = NVXGetCount();
	if (deviceCount != 1) {
		m_rDriverContext.getLogManager() << LogLevel_Error << (deviceCount == 0 ? "No MKS device found.\n" : "Only one MKS device is supported.\n");
		return false;
	}
	nvxDeviceId_ = NVXGetId(0);
	if (NVXGetInformation(nvxDeviceId_, &nvxInfo_) != NVX_ERR_OK) {
		m_rDriverContext.getLogManager() << LogLevel_Error << "NVXGetInformation error.\n";
		return false;
	}
	OpenViBE::uint32 realChannelsCount = 0;
	switch (nvxInfo_.Model) {
	case NVX_MODEL_16:
		realChannelsCount = showAuxChannels_ ? 16 : 10;
		break;
	case NVX_MODEL_24:
		realChannelsCount = 24;
		break;
	case NVX_MODEL_36:
		realChannelsCount = showAuxChannels_ ? 36 : 32;
		break;
	case NVX_MODEL_52:
		realChannelsCount = showAuxChannels_ ? 52 : 48;
		break;
	default:
		m_rDriverContext.getLogManager() << LogLevel_Error << "Unsupported device model: ." << nvxInfo_.Model << "\n";
		return false;
	}
	nvxDataModel_ = NVX_DM_NORMAL;
	switch (m_oHeader.getSamplingFrequency()) {
	case 50000:
		nvxDataSettings_.DataRate = 0; // it is not used
		realChannelsCount = 4;
		nvxDataModel_ = NVX_DM_50_KHZ;
		break;
	case 10000:
		nvxDataSettings_.DataRate = 0;
		realChannelsCount = 16;
		break;
	case 5000:
		nvxDataSettings_.DataRate = 1;
		realChannelsCount = 24;
		break;
	case 2000:
		nvxDataSettings_.DataRate = 2;
		break;
	case 1000:
		nvxDataSettings_.DataRate = 3;
		break;
	case 500:
		nvxDataSettings_.DataRate = 4;
		break;
	case 250:
		nvxDataSettings_.DataRate = 5;
		break;
	case 125:
		nvxDataSettings_.DataRate = 6;
		break;
	default:
		m_rDriverContext.getLogManager() << LogLevel_Error << "Unsupported sampling frequency.\n";
		return false;
	}
	m_oHeader.setChannelCount(realChannelsCount);
	samplesCounter_ = 0;
	triggerStates_ = 0;
	// Builds up a buffer to store
	// acquired samples. This buffer
	// will be sent to the acquisition
	// server later...
	sampleData_.resize(getDeviceBufferSamplesCapacity());

	if (NVXSetDataMode(nvxDeviceId_, nvxDataModel_, &nvxDataSettings_) != NVX_ERR_OK) {
		m_rDriverContext.getLogManager() << LogLevel_Error << "NVXSetDataMode error.\n";
		return false;
	}

	if (NVXGetProperty(nvxDeviceId_, &nvxProperty_) != NVX_ERR_OK) {
		m_rDriverContext.getLogManager() << LogLevel_Error << "NVXGetProperty error.\n";
		return false;
	}

	if (NVXOpen(nvxDeviceId_) != NVX_ERR_OK) {
		m_rDriverContext.getLogManager() << LogLevel_Error << "NVXSetDataMode error.\n";
		return false;
	}
	// Saves parameters
	m_pCallback=&rCallback;
	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;
	if (m_rDriverContext.isImpedanceCheckRequested()) {
		if (NVXStartImpedance(nvxDeviceId_) != NVX_ERR_OK) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStartImpedance error.\n";
			return false;
		}
		if (NVXStart(nvxDeviceId_) != NVX_ERR_OK) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStart error.\n";
			return false;
		}
	}
	return true;
}

boolean CDriverMKSNVXDriver::start(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) return false;

	// ...
	// request hardware to start
	// sending data
	// ...
	samplesCounter_ = 0;
	if (m_rDriverContext.isImpedanceCheckRequested()) {
		if (NVXStop(nvxDeviceId_) != NVX_ERR_OK) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStop error.\n";
			return false;
		}
		if (NVXStopImpedance(nvxDeviceId_) != NVX_ERR_OK) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStopImpedance error.\n";
			return false;
		}
	}
	switch (dataMode_) {
	case 1: // test
		if (NVXStartTest(nvxDeviceId_, 0) != NVX_ERR_OK) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStartTest error.\n";
			return false;
		}
		// break; no break on purpose
	case 0: // normal
		if (NVXStart(nvxDeviceId_) != NVX_ERR_OK) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStart error.\n";
			return false;
		}
		break;
	default:
		return false;
	}
	return true;
}

const float floatNaN = std::numeric_limits<float>::quiet_NaN();
#define NVXValueToFloat(v, resolution) ((v) == INT_MAX ? floatNaN : ((v)*(resolution)))
// #define NVXValueToFloat(v, resolution) ((v) == INT_MAX ? 0.f : ((v)*(resolution)))

boolean CDriverMKSNVXDriver::loop(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if(m_rDriverContext.isStarted()) {
		const size_t bufferSize = sizeof(deviceBuffer_);
		int res = NVXGetData(nvxDeviceId_, deviceBuffer_, bufferSize);
		size_t sizeOfNVXDataModelStructure = nvxDataModel_ == NVX_DM_NORMAL ? (nvxInfo_.Model == NVX_MODEL_16 ? sizeof(t_NVXDataModel16)
			: (nvxInfo_.Model == NVX_MODEL_24 ? sizeof(t_NVXDataModel24)
			: (nvxInfo_.Model == NVX_MODEL_36 ? sizeof(t_NVXDataModel36) : sizeof(t_NVXDataModel52))))
			: sizeof(t_NVXDataMode50kHz);
		if (res < 0 || res > bufferSize || res % sizeOfNVXDataModelStructure) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXGetData returned " << res << " This is unexpected.\n";
			return false;
		}
		if (res) {

			OpenViBE::CStimulationSet l_oStimulationSet;

			// ...
			// receive samples from hardware
			// put them the correct way in the sample array
			// whether the buffer is full, send it to the acquisition server
			//...
			const size_t channelsAmount = m_oHeader.getChannelCount();
			const size_t sampblesPerChannel = res / sizeOfNVXDataModelStructure;
			if (sampblesPerChannel * channelsAmount > getDeviceBufferSamplesCapacity()) {
				m_rDriverContext.getLogManager() << LogLevel_Error << "Internal error: the amount of data exceeds the size of sampleData storage. This is internal error due to sampleData storage miscalculation.\n";
				return false;
			}
			float *sampleDataPtr = &sampleData_.at(0);
			const float eegResolution = nvxProperty_.ResolutionEeg * 1000000.f, auxResolution = nvxProperty_.ResolutionAux * 1000000.f;
			for (size_t j = 0; j < sampblesPerChannel; ++j) {
				for (size_t i = 0; i < channelsAmount; ++i) {
					if (nvxDataModel_ == NVX_DM_NORMAL) {
						switch (nvxInfo_.Model) {
						case NVX_MODEL_16:
							if (i < NVX_MODEL_16_CHANNELS_MAIN) {
								sampleDataPtr[i*sampblesPerChannel + j] = NVXValueToFloat(reinterpret_cast<t_NVXDataModel16*>(deviceBuffer_)[j].Main[i], eegResolution);
							}
							else {
								const size_t auxChannelNumber = i - NVX_MODEL_16_CHANNELS_MAIN;
								sampleDataPtr[i*sampblesPerChannel + j] = NVXValueToFloat(reinterpret_cast<t_NVXDataModel16*>(deviceBuffer_)[j].Aux[auxChannelNumber], auxResolution);
							}
							break;
						case NVX_MODEL_24:
							sampleDataPtr[i*sampblesPerChannel + j] = NVXValueToFloat(reinterpret_cast<t_NVXDataModel24*>(deviceBuffer_)[j].Main[i], eegResolution);
							break;
						case NVX_MODEL_36:
							if (i < NVX_MODEL_36_CHANNELS_MAIN) {
								sampleDataPtr[i*sampblesPerChannel + j] = NVXValueToFloat(reinterpret_cast<t_NVXDataModel36*>(deviceBuffer_)[j].Main[i], eegResolution);
							}
							else {
								const size_t auxChannelNumber = i - NVX_MODEL_36_CHANNELS_MAIN;
								sampleDataPtr[i*sampblesPerChannel + j] = NVXValueToFloat(reinterpret_cast<t_NVXDataModel36*>(deviceBuffer_)[j].Aux[auxChannelNumber], auxResolution);
							}
							break;
						case NVX_MODEL_52:
							if (i < NVX_MODEL_52_CHANNELS_MAIN) {
								sampleDataPtr[i*sampblesPerChannel + j] = NVXValueToFloat(reinterpret_cast<t_NVXDataModel52*>(deviceBuffer_)[j].Main[i], eegResolution);
							}
							else {
								const size_t auxChannelNumber = i - NVX_MODEL_52_CHANNELS_MAIN;
								sampleDataPtr[i*sampblesPerChannel + j] = NVXValueToFloat(reinterpret_cast<t_NVXDataModel52*>(deviceBuffer_)[j].Aux[auxChannelNumber], auxResolution);
							}
							break;
						default:
							m_rDriverContext.getLogManager() << LogLevel_Error << "Unsupported device model: ." << nvxInfo_.Model << "\n";
							return false;
						}
					}
					else {
						sampleDataPtr[i*sampblesPerChannel + j] = NVXValueToFloat(reinterpret_cast<t_NVXDataMode50kHz*>(deviceBuffer_)[j].Main[i], eegResolution);
					}
				}
				// check data integrity
				unsigned int correctSamplesCounter = 0;
				if (nvxDataModel_ == NVX_DM_NORMAL) {
					switch (nvxInfo_.Model) {
					case NVX_MODEL_16:
						correctSamplesCounter = reinterpret_cast<t_NVXDataModel16*>(deviceBuffer_)[j].Counter;
						break;
					case NVX_MODEL_24:
						correctSamplesCounter = reinterpret_cast<t_NVXDataModel24*>(deviceBuffer_)[j].Counter;
						break;
					case NVX_MODEL_36:
						correctSamplesCounter = reinterpret_cast<t_NVXDataModel36*>(deviceBuffer_)[j].Counter;
						break;
					case NVX_MODEL_52:
						correctSamplesCounter = reinterpret_cast<t_NVXDataModel52*>(deviceBuffer_)[j].Counter;
						break;
					default:
						m_rDriverContext.getLogManager() << LogLevel_Error << "Unsupported device model: ." << nvxInfo_.Model << "\n";
						return false;
					}
				}
				else {
					correctSamplesCounter = reinterpret_cast<t_NVXDataMode50kHz*>(deviceBuffer_)[j].Counter;
				}
				if (samplesCounter_ != correctSamplesCounter) {
					m_rDriverContext.getLogManager() << LogLevel_Error << "Data loss: expected sample number is " << samplesCounter_ 
						<< " and the hardware reports " << correctSamplesCounter << "\n";
					samplesCounter_ = correctSamplesCounter;
					l_oStimulationSet.appendStimulation(OVTK_StimulationId_Label_00, OpenViBE::ITimeArithmetics::sampleCountToTime(m_oHeader.getSamplingFrequency(), j), 0);
				}
				++samplesCounter_;
				// set triggers
				OpenViBE::uint32 currentTriggersState = 0;
				if (nvxDataModel_ == NVX_DM_NORMAL) {
					switch (nvxInfo_.Model) {
					case NVX_MODEL_16:
						currentTriggersState = ~reinterpret_cast<t_NVXDataModel16*>(deviceBuffer_)[j].Status & 0x1ff;
						break;
					case NVX_MODEL_24:
						currentTriggersState = ~reinterpret_cast<t_NVXDataModel24*>(deviceBuffer_)[j].Status & 0x1ff;
						break;
					case NVX_MODEL_36:
						currentTriggersState = ~reinterpret_cast<t_NVXDataModel36*>(deviceBuffer_)[j].Status & 0x1ff;
						break;
					case NVX_MODEL_52:
						currentTriggersState = ~reinterpret_cast<t_NVXDataModel52*>(deviceBuffer_)[j].Status & 0x1ff;
						break;
					default:
						m_rDriverContext.getLogManager() << LogLevel_Error << "Unsupported device model: ." << nvxInfo_.Model << "\n";
						return false;
					}
				}
				else {
					currentTriggersState = ~reinterpret_cast<t_NVXDataMode50kHz*>(deviceBuffer_)[j].Status & 0x1ff;
				}
				for (OpenViBE::uint32 triggerBit = 1, ovtkLabel = OVTK_StimulationId_Label_01; triggerBit <= 256; triggerBit <<= 1, ++ovtkLabel) {
					if (currentTriggersState & triggerBit) {
						if (!(triggerStates_ & triggerBit)) {
							triggerStates_ |= triggerBit;
							l_oStimulationSet.appendStimulation(ovtkLabel, OpenViBE::ITimeArithmetics::sampleCountToTime(m_oHeader.getSamplingFrequency(), j), 0);
						}
					}
					else {
						triggerStates_ &= ~triggerBit;
					}
				}
			}
			m_pCallback->setSamples(sampleDataPtr, sampblesPerChannel);

			// When your sample buffer is fully loaded, 
			// it is advised to ask the acquisition server 
			// to correct any drift in the acquisition automatically.
			m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());

			// ...
			// receive events from hardware
			// and put them the correct way in a CStimulationSet object
			//...
			if (l_oStimulationSet.getStimulationCount() != 0) {
				m_pCallback->setStimulationSet(l_oStimulationSet);
			}
		}
	}
	else if (m_rDriverContext.isImpedanceCheckRequested()) { // impedance
		const size_t impedanceBubberCapacityAmount = 48;
		OpenViBE::uint32 impedanceBuffer[impedanceBubberCapacityAmount];
		size_t impedanceChannelsAmount = m_oHeader.getChannelCount();
		if (impedanceChannelsAmount == 36 || impedanceChannelsAmount == 52) {
			impedanceChannelsAmount -= 4;
		}
		if (impedanceBubberCapacityAmount < impedanceChannelsAmount) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "Incorrect channels amount in impedance mode. This is internal error which shall never happen.\n";
			return false;
		}
		if (NVXGetImpedance(nvxDeviceId_, impedanceBuffer, impedanceChannelsAmount*sizeof(impedanceBuffer[0])) != NVX_ERR_OK) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXGetImpedance error.\n";
			return false;
		}
		for (size_t i = 0; i < impedanceChannelsAmount; ++i) {
			m_rDriverContext.updateImpedance(i, impedanceBuffer[i]);
		}
	}
	return true;
}

boolean CDriverMKSNVXDriver::stop(void)
{
	if (!m_rDriverContext.isConnected()) return false;
	if (!m_rDriverContext.isStarted()) return false;

	// ...
	// request the hardware to stop
	// sending data
	// ...
	switch (dataMode_) {
	case 0: // normal
		if (NVXStop(nvxDeviceId_) != NVX_ERR_OK) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStop error.\n";
			return false;
		}
		break;
	case 1: // test
		if (NVXStop(nvxDeviceId_) != NVX_ERR_OK) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStop error.\n";
			return false;
		}
		if (NVXStopTest(nvxDeviceId_) != NVX_ERR_OK) {
			m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStopTest error.\n";
			return false;
		}
		break;
	default:
		return false;
	}

	return true;
}

boolean CDriverMKSNVXDriver::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) return false;
	if (!m_rDriverContext.isStarted()) {
		if (m_rDriverContext.isImpedanceCheckRequested()) {
			if (NVXStop(nvxDeviceId_) != NVX_ERR_OK) {
				m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStop error.\n";
				return false;
			}
			if (NVXStopImpedance(nvxDeviceId_) != NVX_ERR_OK) {
				m_rDriverContext.getLogManager() << LogLevel_Error << "NVXStopImpedance error.\n";
				return false;
			}
			if (NVXClose(nvxDeviceId_) != NVX_ERR_OK) {
				m_rDriverContext.getLogManager() << LogLevel_Error << "NVXClose error.\n";
				return false;
			}
			return true;
		}
		return false;
	}

	// ...
	// uninitialize hardware here
	// ...
	if (NVXClose(nvxDeviceId_) != NVX_ERR_OK) {
		m_rDriverContext.getLogManager() << LogLevel_Error << "NVXClose error.\n";
		return false;
	}

	m_pCallback=NULL;
	NVXAPIStop();

	return true;
}

//___________________________________________________________________//
//                                                                   //
boolean CDriverMKSNVXDriver::isConfigurable(void)
{
	return true; // change to false if your device is not configurable
}

boolean CDriverMKSNVXDriver::configure(void)
{
	// Change this line if you need to specify some references to your driver attribute that need configuration, e.g. the connection ID.
	CConfigurationMKSNVXDriver m_oConfiguration(m_rDriverContext, 
		OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-MCSNVXDriver.ui",
		dataMode_, showAuxChannels_);
	
	if(!m_oConfiguration.configure(m_oHeader))
	{
		return false;
	}
	m_oSettings.save();
	
	return true;
}

#endif