#include "ovpCTopographicMapDatabase.h"

#include <algorithm>

#include <cmath>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;
using namespace OpenViBE::Kernel;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

using namespace OpenViBEToolkit;

using namespace std;


CTopographicMapDatabase::CTopographicMapDatabase(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& oPlugin, IAlgorithmProxy& rSphericalSplineInterpolation)
	:CBufferDatabase(oPlugin)
	,m_bFirstProcess(true)
	,m_rSphericalSplineInterpolation(rSphericalSplineInterpolation)
	,m_i64SplineOrder(4)
	,m_ui64InterpolationType(OVP_TypeId_SphericalLinearInterpolationType_Spline)
	,m_bElectrodeCoordsInitialized(false)
	,m_pElectrodeCoords(NULL)
	,m_pElectrodePotentials(NULL)
	,m_pSamplePointCoords(NULL)
	,m_ui64Delay(0)
{
	//map input parameters
	//--------------------

	//spline order
	m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SplineOrder)->setReferenceTarget(&m_i64SplineOrder);
	//number of channels (or electrodes)
	m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCount)->setReferenceTarget(&m_i64NbElectrodes);
	//matrix of pointers to electrode coordinates
	m_pElectrodeCoords = &m_oElectrodeCoords;
	m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsCoordinates)->setReferenceTarget(&m_pElectrodeCoords);
	//matrix of potentials measured at each electrode
	m_pElectrodePotentials = &m_oElectrodePotentials;
	m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_ControlPointsValues)->setReferenceTarget(&m_pElectrodePotentials);
	//matrix holding sample coordinates mapped at runtime (its size is not known a priori and may vary)
	//

	//map output parameters
	//---------------------
	m_oMinSamplePointValue.initialize(m_rSphericalSplineInterpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MinSamplePointValue));
	m_oMaxSamplePointValue.initialize(m_rSphericalSplineInterpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_MaxSamplePointValue));
}

CTopographicMapDatabase::~CTopographicMapDatabase()
{
}

void CTopographicMapDatabase::setMatrixDimensionSize(const uint32 ui32DimensionIndex, const uint32 ui32DimensionSize)
{
	CBufferDatabase::setMatrixDimensionSize(ui32DimensionIndex, ui32DimensionSize);

	if(ui32DimensionIndex == 0)
	{
		m_oElectrodePotentials.setDimensionCount(1);
		m_oElectrodePotentials.setDimensionSize(0, (uint32)m_i64NbElectrodes);
	}
}

boolean CTopographicMapDatabase::onChannelLocalisationBufferReceived(uint32 ui32ChannelLocalisationBufferIndex)
{
	CBufferDatabase::onChannelLocalisationBufferReceived(ui32ChannelLocalisationBufferIndex);

	if(m_bChannelLookupTableInitialized == false || m_oChannelLocalisationStreamedCoords.size() == 0 || m_i64NbElectrodes == 0)
	{
		m_oParentPlugin.getLogManager() << LogLevel_Warning
			<< "Channel localisation buffer received before channel lookup table was initialized! Can't process buffer!\n";
	}

	//static electrode coordinates
	if(m_bDynamicChannelLocalisation == false)
	{
		//if streamed coordinates are cartesian
		if(m_bCartesianStreamedCoords == true)
		{
			//fill electrode coordinates matrix
			m_oElectrodeCoords.setDimensionCount(1);
			m_oElectrodeCoords.setDimensionSize(0, (uint32)(3*m_i64NbElectrodes));
			const float64* l_pCoords = m_oChannelLocalisationStreamedCoords[0].first->getBuffer();
			uint32 l_ui32LookupIndex;
			for(uint32 i=0; i<(uint32)m_i64NbElectrodes; i++)
			{
				l_ui32LookupIndex = m_oChannelLookupIndices[i];
				m_oElectrodeCoords[3*i] = *(l_pCoords + 3 * l_ui32LookupIndex);
				m_oElectrodeCoords[3*i+1] = *(l_pCoords + 3 * l_ui32LookupIndex + 1);
				m_oElectrodeCoords[3*i+2] = *(l_pCoords + 3 * l_ui32LookupIndex + 2);
			}

			//electrode coordinates initialized : it is now possible to interpolate potentials
			m_bElectrodeCoordsInitialized = true;
		}
	}

	return true;
}

void CTopographicMapDatabase::getLastBufferInterpolatedMinMaxValue(OpenViBE::float64& f64Min, OpenViBE::float64& f64Max)
{
	f64Min = m_oMinSamplePointValue;
	f64Max = m_oMaxSamplePointValue;
}

boolean CTopographicMapDatabase::processValues()
{
	//wait for electrode coordinates
	if(m_bElectrodeCoordsInitialized == false)
	{
		return true;
	}

	if(m_bFirstProcess == true)
	{
		//done in CBufferDatabase::setMatrixBuffer
		//initialize the drawable object
		//m_pDrawable->init();

		if(checkElectrodeCoordinates() == false)
		{
			return false;
		}

		//precompute sin/cos tables
		m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_PrecomputeTables, true);

		m_bFirstProcess = false;
	}

	//retrieve electrode values
	//determine what buffer to use from delay
	uint32 l_ui32BufferIndex;
	uint64 l_ui64CurrentTime = m_oParentPlugin.getPlayerContext().getCurrentTime();
	uint64 l_ui64DisplayTime = l_ui64CurrentTime - m_ui64Delay;
	getBufferIndexFromTime(l_ui64DisplayTime, l_ui32BufferIndex);

	//determine what sample to use
	uint64 l_ui64SampleIndex;
	if(l_ui64DisplayTime <= m_oStartTime[l_ui32BufferIndex])
	{
		l_ui64SampleIndex = 0;
	}
	else if(l_ui64DisplayTime >= m_oEndTime[l_ui32BufferIndex])
	{
		l_ui64SampleIndex = m_pDimensionSizes[1] - 1;
	}
	else
	{
		l_ui64SampleIndex = (uint64)((float64)(l_ui64DisplayTime - m_oStartTime[l_ui32BufferIndex])/(float64)m_ui64BufferDuration * m_pDimensionSizes[1]);
	}

	for(int i=0; i<m_i64NbElectrodes; i++)
	{
		*(m_oElectrodePotentials.getBuffer()+i) = m_oSampleBuffers[l_ui32BufferIndex][i*m_pDimensionSizes[1] + l_ui64SampleIndex];
	}

	//interpolate spline values (potentials)
	if(m_ui64InterpolationType == OVP_TypeId_SphericalLinearInterpolationType_Spline)
	{
		m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeSplineCoefs, true);
	}
	else //interpolate spline laplacian (currents)
	{
		m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_ComputeLaplacianCoefs, true);
	}

	//retrieve up-to-date pointer to sample matrix
	m_pSamplePointCoords = dynamic_cast<CTopographicMapDrawable*>(m_pDrawable)->getSampleCoordinatesMatrix();

	if(m_pSamplePointCoords != NULL)
	{
		//map pointer to input parameter
		m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates)->setReferenceTarget(&m_pSamplePointCoords);

		if(m_ui64InterpolationType == OVP_TypeId_SphericalLinearInterpolationType_Spline)
		{
			m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateSpline, true);
		}
		else
		{
			m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateLaplacian, true);
		}
	}

	m_rSphericalSplineInterpolation.process();
	boolean l_bProcess = true;
	if(m_rSphericalSplineInterpolation.isOutputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error) == true)
	{
		m_oParentPlugin.getLogManager() << LogLevel_Warning	<< "An error occurred while interpolating potentials!\n";
		l_bProcess = false;
	}
	else
	{
		if(m_pSamplePointCoords != NULL)
		{
			//retrieve interpolation results
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > l_oSampleValuesMatrix;
			l_oSampleValuesMatrix.initialize(m_rSphericalSplineInterpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_SamplePointsValues));
			dynamic_cast<CTopographicMapDrawable*>(m_pDrawable)->setSampleValuesMatrix(l_oSampleValuesMatrix);

			//tells the drawable to redraw itself since the signal information has been updated
			m_pDrawable->redraw();
		}
	}

	return l_bProcess;
}

boolean CTopographicMapDatabase::setDelay(OpenViBE::float64 f64Delay)
{
	if(f64Delay > m_f64TotalDuration)
	{
		return false;
	}

	//convert delay to 32:32 format
	m_ui64Delay = (int64)(f64Delay*(1LL<<32)); // $$$ Casted in (int64) because of Ubuntu 7.10 crash !
	return true;
}

boolean CTopographicMapDatabase::setInterpolationType(OpenViBE::uint64 ui64InterpolationType)
{
	m_ui64InterpolationType = ui64InterpolationType;
	return true;
}

boolean CTopographicMapDatabase::interpolateValues()
{
	//can't interpolate before first buffer has been received
	if(m_bFirstProcess == true)
	{
		return false;
	}

	//retrieve up-to-date pointer to sample matrix
	m_pSamplePointCoords = dynamic_cast<CTopographicMapDrawable*>(m_pDrawable)->getSampleCoordinatesMatrix();

	if(m_pSamplePointCoords != NULL)
	{
		//map pointer to input parameter
		m_rSphericalSplineInterpolation.getInputParameter(OVP_Algorithm_SphericalSplineInterpolation_InputParameterId_SamplePointsCoordinates)->setReferenceTarget(&m_pSamplePointCoords);

		//interpolate using spline or laplacian coefficients depending on interpolation mode
		if(m_ui64InterpolationType == OVP_TypeId_SphericalLinearInterpolationType_Spline)
		{
			m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateSpline, true);
		}
		else
		{
			m_rSphericalSplineInterpolation.activateInputTrigger(OVP_Algorithm_SphericalSplineInterpolation_InputTriggerId_InterpolateLaplacian, true);
		}
	}

	m_rSphericalSplineInterpolation.process();

	if(m_rSphericalSplineInterpolation.isOutputTriggerActive(OVP_Algorithm_SphericalSplineInterpolation_OutputTriggerId_Error) == true)
	{
		m_oParentPlugin.getLogManager() << LogLevel_Warning	<< "An error occurred while interpolating potentials!\n";
	}
	else
	{
		if(m_pSamplePointCoords != NULL)
		{
			//retrieve interpolation results
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > l_oSampleValuesMatrix;
			l_oSampleValuesMatrix.initialize(m_rSphericalSplineInterpolation.getOutputParameter(OVP_Algorithm_SphericalSplineInterpolation_OutputParameterId_SamplePointsValues));
			dynamic_cast<CTopographicMapDrawable*>(m_pDrawable)->setSampleValuesMatrix(l_oSampleValuesMatrix);
		}
	}

	return true;
}

boolean CTopographicMapDatabase::getBufferIndexFromTime(uint64 ui64Time, uint32& rBufferIndex)
{
	if(m_oSampleBuffers.size() == 0)
	{
		return false;
	}

	if(ui64Time < m_oStartTime[0])
	{
		rBufferIndex = 0;
		return false;
	}
	else if(ui64Time > m_oEndTime.back())
	{
		rBufferIndex = m_oSampleBuffers.size() - 1;
		return false;
	}
	else
	{
		for(uint32 i=0; i<m_oSampleBuffers.size(); i++)
		{
			if(ui64Time <= m_oEndTime[i])
			{
				rBufferIndex = i;
				break;
			}
		}

		return true;
	}
}

boolean CTopographicMapDatabase::checkElectrodeCoordinates()
{
	uint64 l_ui64ChannelCount = getChannelCount();

	for(uint32 i=0; i<l_ui64ChannelCount; i++)
	{
		float64* l_pNormalizedChannelCoords = NULL;
		if(getChannelPosition(i, l_pNormalizedChannelCoords) == false)
		{
			CString l_sChannelLabel;
			getChannelLabel(i, l_sChannelLabel);
			m_oParentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager()
				<< LogLevel_Error
				<< "Couldn't retrieve coordinates of electrode #" << i
				<< "(" << l_sChannelLabel << "), aborting model frame electrode coordinates computation\n";
			return false;
		}

#define MY_THRESHOLD 0.01
		if(fabs(l_pNormalizedChannelCoords[0] * l_pNormalizedChannelCoords[0] +
		        l_pNormalizedChannelCoords[1] * l_pNormalizedChannelCoords[1] +
		        l_pNormalizedChannelCoords[2] * l_pNormalizedChannelCoords[2] - 1.) > MY_THRESHOLD)
#undef MY_THRESHOLD
		{
			CString l_sChannelLabel;
			getChannelLabel(i, l_sChannelLabel);
			m_oParentPlugin.getBoxAlgorithmContext()->getPlayerContext()->getLogManager()
				<< LogLevel_Error
				<< "Coordinates of electrode #" << i
				<< "(" << l_sChannelLabel << "), are not normalized, aborting model frame electrode coordinates computation\n";
			return false;
		}
	}

	return true;
}
