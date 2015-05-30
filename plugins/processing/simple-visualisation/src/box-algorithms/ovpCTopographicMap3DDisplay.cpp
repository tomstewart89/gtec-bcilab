#include "ovpCTopographicMap3DDisplay.h"
#include "ovpCTopographicMap3DDisplay/ovpCTopographicMap3DView.h"
#include "../algorithms/ovpCAlgorithmSphericalSplineInterpolation.h"

#include <cstdlib>
#include <cmath>
#include <memory.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEToolkit;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;
using namespace std;

/*
   OV (Ogre)     Normalized space
   =========     ================
    Y                Zn
    |                |
    +-- X        Xn--+
   /                /
  Z                Yn
 => X = -Xn, Y = Zn, Z = Yn
*/

template<typename RealOv, typename RealSTD> void convert_std_to_ov( RealOv ov_vec[], const RealSTD std_vec[]) 
{
	ov_vec[0] = -(RealOv)std_vec[0]; 
	ov_vec[1] = (RealOv)std_vec[2]; 
	ov_vec[2] = (RealOv)std_vec[1];
}
template<typename RealSTD, typename RealOv> void convert_ov_to_std(RealSTD std_vec[] , const RealOv ov_vec[]) 
{
	std_vec[0] = -(RealSTD)ov_vec[0]; 
	std_vec[1] = (RealSTD)ov_vec[2]; 
	std_vec[2] = (RealSTD)ov_vec[1];
}

#define CROSS(dest,v1,v2) \
	dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
	dest[0]=v1[0]-v2[0]; \
	dest[1]=v1[1]-v2[1]; \
	dest[2]=v1[2]-v2[2];

#define NORMALIZE(v) { \
	float32 a = 1 / sqrtf(DOT(v,v)); \
	v[0] *= a; \
	v[1] *= a; \
	v[2] *= a; }

#define EPSILON 0.000001f

CTopographicMap3DDisplay::CTopographicMap3DDisplay(void) :
	m_bError(false),
	m_pChannelLocalisationStreamDecoder(NULL),
	m_pSphericalSplineInterpolation(NULL),
	m_pTopographicMapDatabase(NULL),
	m_pTopographicMap3DView(NULL),
	m_oResourceGroupIdentifier(OV_UndefinedIdentifier),
	m_bSkullCreated(false),
	m_bCameraPositioned(false),
	m_bScalpDataInitialized(false),
	m_bElectrodesCreated(false),
	m_bModelElectrodeCoordinatesInitialized(false),
	m_ui32NbColors(0),
	m_pColorScale(NULL),
	m_bNeedToggleElectrodes(true),
	m_bElectrodesToggleState(false),
	m_bNeedToggleSamplingPoints(false),
	m_bSamplingPointsToggleState(false),
	m_ui32NbScalpVertices(0),
	m_pScalpVertices(NULL),
	m_pScalpColors(NULL)
{
	m_f32ProjectionCenter[0] = 0.f;
	m_f32ProjectionCenter[1] = 0.f;
	m_f32ProjectionCenter[2] = 0.f;

	//TODO : read color scale from some database or flow header
	m_ui32NbColors = 13;
	m_pColorScale = new float32[m_ui32NbColors*3];

	m_pColorScale[0] = 255/255.f; m_pColorScale[1] = 0/255.f; m_pColorScale[2] = 0/255.f;
	m_pColorScale[3] = 234/255.f; m_pColorScale[4] = 1/255.f; m_pColorScale[5] = 0/255.f;
	m_pColorScale[6] = 205/255.f; m_pColorScale[7] = 0/255.f; m_pColorScale[8] = 101/255.f;
	m_pColorScale[9] = 153/255.f; m_pColorScale[10] = 0/255.f; m_pColorScale[11] = 178/255.f;
	m_pColorScale[12] = 115/255.f; m_pColorScale[13] = 1/255.f; m_pColorScale[14] = 177/255.f;
	m_pColorScale[15] = 77/255.f; m_pColorScale[16] = 0/255.f; m_pColorScale[17] = 178/255.f;
	m_pColorScale[18] = 0/255.f; m_pColorScale[19] = 0/255.f; m_pColorScale[20] = 152/255.f;
	m_pColorScale[21] = 0/255.f; m_pColorScale[22] = 97/255.f; m_pColorScale[23] = 121/255.f;
	m_pColorScale[24] = 0/255.f; m_pColorScale[25] = 164/255.f; m_pColorScale[26] = 100/255.f;
	m_pColorScale[27] = 0/255.f; m_pColorScale[28] = 225/255.f; m_pColorScale[29] = 25/255.f;
	m_pColorScale[30] = 150/255.f; m_pColorScale[31] = 255/255.f; m_pColorScale[32] = 0/255.f;
	m_pColorScale[33] = 200/255.f; m_pColorScale[34] = 255/255.f; m_pColorScale[35] = 0/255.f;
	m_pColorScale[36] = 255/255.f; m_pColorScale[37] = 255/255.f; m_pColorScale[38] = 0/255.f;

	m_oSampleCoordinatesMatrix.setDimensionCount(2);
}

uint64 CTopographicMap3DDisplay::getClockFrequency(void)
{
	return ((uint64)1LL)<<37;
}

boolean CTopographicMap3DDisplay::initialize(void)
{
	//initialize chanloc decoder
	m_pChannelLocalisationStreamDecoder = new TChannelLocalisationDecoder < CTopographicMap3DDisplay >;
	m_pChannelLocalisationStreamDecoder->initialize(*this,1);

	//initializes the ebml input
	m_pDecoder = new TStreamedMatrixDecoder < CTopographicMap3DDisplay >;
	m_pDecoder->initialize(*this,0);

	m_bFirstBufferReceived=false;

	//initialize spline interpolation algorithm
	m_pSphericalSplineInterpolation = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_SphericalSplineInterpolation));
	m_pSphericalSplineInterpolation->initialize();

	//create topographic map database
	m_pTopographicMapDatabase = new CTopographicMapDatabase(*this, *m_pSphericalSplineInterpolation);

	//retrieve settings
	CString l_sInterpolationModeSettingValue;
	getStaticBoxContext().getSettingValue(0, l_sInterpolationModeSettingValue);
	CString l_sDelaySettingValue;
	getStaticBoxContext().getSettingValue(1, l_sDelaySettingValue);
	getStaticBoxContext().getSettingValue(2, m_oFaceMeshFilename);
	getStaticBoxContext().getSettingValue(3, m_oScalpMeshFilename);
	getStaticBoxContext().getSettingValue(4, m_oProjectionSphereMeshFilename);

	//create topographic map view (handling GUI interaction)
	m_pTopographicMap3DView = new CTopographicMap3DView(
		*this,
		*m_pTopographicMapDatabase,
		getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_SphericalLinearInterpolationType, l_sInterpolationModeSettingValue),
		atof(l_sDelaySettingValue));

	//have database notify us when new data is available
	m_pTopographicMapDatabase->setDrawable(this);
	//ask not to be notified when new data is available (refresh is handled separately)
	m_pTopographicMapDatabase->setRedrawOnNewData(false);

	//send widget pointers to visualisation context for parenting
	::GtkWidget* l_pWidget=NULL;
	m_o3DWidgetIdentifier = getBoxAlgorithmContext()->getVisualisationContext()->create3DWidget(l_pWidget);
	if(!l_pWidget)
	{
		this->getLogManager() << LogLevel_Error << "Unable to create 3D rendering widget.\n";
		return false;
	}

	getBoxAlgorithmContext()->getVisualisationContext()->setWidget(l_pWidget);

	::GtkWidget* l_pToolbarWidget=NULL;
	m_pTopographicMap3DView->getToolbar(l_pToolbarWidget);
	if(l_pToolbarWidget != NULL)
	{
		getBoxAlgorithmContext()->getVisualisationContext()->setToolbar(l_pToolbarWidget);
	}

	//resource group
	getVisualisationContext().createResourceGroup(m_oResourceGroupIdentifier, "TopographicMap3DResources");
	getVisualisationContext().addResourceLocation(m_oResourceGroupIdentifier, OpenViBE::Directories::getDataDir() + "/plugins/simple-visualisation/topographicmap3D", ResourceType_Directory, false);
	getVisualisationContext().initializeResourceGroup(m_oResourceGroupIdentifier);

	return true;
}

boolean CTopographicMap3DDisplay::uninitialize(void)
{
	//delete decoder algorithm
	if(m_pChannelLocalisationStreamDecoder)
	{
		m_pChannelLocalisationStreamDecoder->uninitialize();
		delete m_pChannelLocalisationStreamDecoder;
	}


	if(m_pDecoder)
	{
		m_pDecoder->uninitialize();
		delete m_pDecoder;
	}

	//release algorithm
	m_pSphericalSplineInterpolation->uninitialize();
	getAlgorithmManager().releaseAlgorithm(*m_pSphericalSplineInterpolation);

	delete m_pTopographicMap3DView;
	m_pTopographicMap3DView = NULL;
	delete m_pTopographicMapDatabase;
	m_pTopographicMapDatabase = NULL;

	delete[] m_pColorScale;
	delete[] m_pScalpColors;
	if(m_pScalpVertices != NULL)
	{
		delete[] m_pScalpVertices;
	}

	//destroy resource group
	if(m_oResourceGroupIdentifier!=OV_UndefinedIdentifier)
	{
		getVisualisationContext().destroyResourceGroup(m_oResourceGroupIdentifier);
		m_oResourceGroupIdentifier = OV_UndefinedIdentifier;
	}

	return true;
}

boolean CTopographicMap3DDisplay::processInput(uint32 ui32InputIndex)
{
	if(!getBoxAlgorithmContext()->getVisualisationContext()->is3DWidgetRealized(m_o3DWidgetIdentifier))
	{
		return true;
	}
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CTopographicMap3DDisplay::processClock(IMessageClock& rMessageClock)
{
	if(!getBoxAlgorithmContext()->getVisualisationContext()->is3DWidgetRealized(m_o3DWidgetIdentifier))
	{
		return true;
	}
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CTopographicMap3DDisplay::process(void)
{
	IDynamicBoxContext* l_pDynamicBoxContext=getBoxAlgorithmContext()->getDynamicBoxContext();
	uint32 i;

	//decode signal data
	for(i=0; i<l_pDynamicBoxContext->getInputChunkCount(0); i++)
	{
		m_pDecoder->decode(i);
		if(m_pDecoder->isBufferReceived())
		{
			IMatrix* l_pInputMatrix=m_pDecoder->getOutputMatrix();

			//do we need to recopy this for each chunk?
			if(!m_bFirstBufferReceived)
			{
				m_pTopographicMapDatabase->setMatrixDimensionCount(l_pInputMatrix->getDimensionCount());
				for(uint32 dimension=0; dimension<l_pInputMatrix->getDimensionCount(); dimension++)
				{
					m_pTopographicMapDatabase->setMatrixDimensionSize(dimension, l_pInputMatrix->getDimensionSize(dimension));
					for(uint32 entryIndex=0; entryIndex<l_pInputMatrix->getDimensionSize(dimension); entryIndex++)
					{
						m_pTopographicMapDatabase->setMatrixDimensionLabel(dimension, entryIndex, l_pInputMatrix->getDimensionLabel(dimension, entryIndex));
					}
				}
				m_bFirstBufferReceived=true;
			}
			//

			if(!m_pTopographicMapDatabase->setMatrixBuffer(l_pInputMatrix->getBuffer(), l_pDynamicBoxContext->getInputChunkStartTime(0,i), l_pDynamicBoxContext->getInputChunkEndTime(0,i)))
			{
				return false;
			}

		}
	}

	//decode channel localisation data
	for(i=0; i<l_pDynamicBoxContext->getInputChunkCount(1); i++)
	{
		const IMemoryBuffer* l_pBuf = l_pDynamicBoxContext->getInputChunk(1, i);
		m_pTopographicMapDatabase->decodeChannelLocalisationMemoryBuffer(
			l_pBuf,
			l_pDynamicBoxContext->getInputChunkStartTime(1, i),
			l_pDynamicBoxContext->getInputChunkEndTime(1, i));
		l_pDynamicBoxContext->markInputAsDeprecated(1, i);
	}

	if(process3D() == true)
	{
		getBoxAlgorithmContext()->getVisualisationContext()->update3DWidget(m_o3DWidgetIdentifier);
		return true;
	}
	else
	{
		//disable plugin upon errors
		return false;
	}
}

//CSignalDisplayDrawable implementation
//-------------------------------------
void CTopographicMap3DDisplay::init()
{
	m_pTopographicMap3DView->init();
}

void CTopographicMap3DDisplay::redraw()
{
	//3D widgets refresh is handled by OpenMASK
}

//CTopographicMapDrawable implementation
//--------------------------------------
CMatrix* CTopographicMap3DDisplay::getSampleCoordinatesMatrix()
{
	return &m_oSampleCoordinatesMatrix;
}

boolean CTopographicMap3DDisplay::setSampleValuesMatrix(IMatrix* pSampleValuesMatrix)
{
	//ensure matrix has the right size
	if(pSampleValuesMatrix == NULL || pSampleValuesMatrix->getDimensionSize(0) < m_ui32NbScalpVertices)
	{
		return false;
	}

	//retrieve min/max potentials
	float64 l_f64MinPotential, l_f64MaxPotential;
	m_pTopographicMapDatabase->getLastBufferInterpolatedMinMaxValue(l_f64MinPotential, l_f64MaxPotential);
	float64 l_f64InvPotentialStep = 0;
	if(l_f64MinPotential < l_f64MaxPotential)
	{
		l_f64InvPotentialStep = m_ui32NbColors / (l_f64MaxPotential - l_f64MinPotential);
	}

	//determine color index of each vertex
	for(uint32 i=0; i<m_ui32NbScalpVertices; i++)
	{
		//determine color index to use
		float64 l_f64Value = *(pSampleValuesMatrix->getBuffer() + i);

		uint32 l_iColorIndex = (uint32)((l_f64Value - l_f64MinPotential) * l_f64InvPotentialStep);
		if(l_iColorIndex >= m_ui32NbColors)
		{
			l_iColorIndex = m_ui32NbColors-1;
		}

		m_pScalpColors[i*4] = m_pColorScale[l_iColorIndex*3];
		m_pScalpColors[i*4+1] = m_pColorScale[l_iColorIndex*3+1];
		m_pScalpColors[i*4+2] = m_pColorScale[l_iColorIndex*3+2];
		m_pScalpColors[i*4+3] = 1;
	}

	//set colors
	getVisualisationContext().setObjectVertexColorArray(m_oScalpId, m_ui32NbScalpVertices, m_pScalpColors);

	return true;
}

void CTopographicMap3DDisplay::toggleElectrodes(boolean bToggle)
{
	m_bNeedToggleElectrodes = true;
	m_bElectrodesToggleState = bToggle;
}

/*void CTopographicMap3DDisplay::toggleSamplingPoints(boolean bToggle)
{
	m_bNeedToggleSamplingPoints = true;
	m_bSamplingPointsToggleState = bToggle;
}*/

boolean CTopographicMap3DDisplay::initializeScalpData()
{
	if(m_ui32NbScalpVertices == 0)
	{
		//retrieve number of vertices in scalp mesh
		if(getVisualisationContext().getObjectVertexCount(m_oScalpId, m_ui32NbScalpVertices) == false)
		{
			getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error << "Couldn't retrieve number of vertices from scalp object\n";
			return false;
		}

		if(m_ui32NbScalpVertices > 0)
		{
			//allocate colors array
			m_pScalpColors = new float32[4*m_ui32NbScalpVertices];

			//retrieve vertices
			if(m_pScalpVertices == NULL)
			{
				m_pScalpVertices = new float32[3*m_ui32NbScalpVertices];

				if(getVisualisationContext().getObjectVertexPositionArray(m_oScalpId, m_ui32NbScalpVertices, m_pScalpVertices) == false)
				{
					getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error << "Couldn't retrieve vertex array from scalp object\n";
					return false;
				}
			}

			//allocate normalized sample coordinates matrix
			m_oSampleCoordinatesMatrix.setDimensionSize(0, m_ui32NbScalpVertices);
			m_oSampleCoordinatesMatrix.setDimensionSize(1, 3);

			//compute scalp vertices coordinates once projected onto a unit sphere
			float32 l_f32UnitVector[3];
			float32* l_pScalpVertexCoord = m_pScalpVertices;
			float64* l_pSampleCoordsBuffer = m_oSampleCoordinatesMatrix.getBuffer();

			for(uint32 i=0; i<m_ui32NbScalpVertices; i++, l_pScalpVertexCoord+=3)
			{
				//compute unit vector from center of unit sphere to scalp vertex
				convert_ov_to_std(l_f32UnitVector, l_pScalpVertexCoord);
				SUB(l_f32UnitVector, l_f32UnitVector, m_f32ProjectionCenter)
				NORMALIZE(l_f32UnitVector)

				//store vertex in matrix to be fed to interpolation algorithm
				*l_pSampleCoordsBuffer++ = l_f32UnitVector[0];
				*l_pSampleCoordsBuffer++ = l_f32UnitVector[1];
				*l_pSampleCoordsBuffer++ = l_f32UnitVector[2];
			}
		}
	}

	return true;
}

boolean CTopographicMap3DDisplay::computeModelFrameChannelCoordinates()
{
	uint64 l_ui64ChannelCount = m_pTopographicMapDatabase->getChannelCount();

	//transform normalized projection center to Ogre space
	float32 l_pProjectionCenter[3];
	convert_std_to_ov(l_pProjectionCenter, m_f32ProjectionCenter);

	//get scalp triangles
	uint32 l_ui32ScalpTriangleCount = 0;
	if(getVisualisationContext().getObjectTriangleCount(m_oScalpId, l_ui32ScalpTriangleCount) == false)
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error << "Couldn't retrieve number of triangles from scalp object\n";
		return false;
	}
	if(l_ui32ScalpTriangleCount == 0)
	{
		return false;
	}

	//store scalp triangle indices
	uint32* l_pScalpTriangleIndexArray = new uint32[3*l_ui32ScalpTriangleCount];
	if(getVisualisationContext().getObjectTriangleIndexArray(m_oScalpId, l_ui32ScalpTriangleCount, l_pScalpTriangleIndexArray) == false)
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error << "Couldn't retrieve index array from scalp object\n";
		delete[] l_pScalpTriangleIndexArray;
		return false;
	}

#define PROJECT_ON_FACE

#ifdef PROJECT_ON_FACE
	uint32 l_ui32NbFaceVertices = 0;
	if(getVisualisationContext().getObjectVertexCount(m_oFaceId, l_ui32NbFaceVertices) == false)
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << "Couldn't retrieve number of vertices from face object\n";
	}

	float32* l_pFaceVertices = new float32[3*l_ui32NbFaceVertices];
	if(getVisualisationContext().getObjectVertexPositionArray(m_oFaceId, l_ui32NbFaceVertices, l_pFaceVertices) == false)
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << "Couldn't retrieve vertex array from face object\n";
		delete[] l_pFaceVertices;
		l_pFaceVertices = NULL;
	}

	uint32 l_ui32FaceTriangleCount = 0;
	if(getVisualisationContext().getObjectTriangleCount(m_oFaceId, l_ui32FaceTriangleCount) == false)
	{
		getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << "Couldn't retrieve number of triangles from face object\n";
	}

	uint32* l_pFaceTriangleIndexArray = NULL;
	if(l_ui32FaceTriangleCount > 0)
	{
		//store scalp triangle indices
		l_pFaceTriangleIndexArray = new uint32[3*l_ui32FaceTriangleCount];
		if(getVisualisationContext().getObjectTriangleIndexArray(m_oFaceId, l_ui32FaceTriangleCount, l_pFaceTriangleIndexArray) == false)
		{
			getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << "Couldn't retrieve index array from face object\n";
			delete[] l_pFaceVertices;
			l_pFaceVertices = NULL;
			delete[] l_pFaceTriangleIndexArray;
			l_pFaceTriangleIndexArray = NULL;
		}
	}
#endif

	m_oElectrodeIds.resize((size_t)m_pTopographicMapDatabase->getChannelCount());

	//retrieve size of scalp to compute size to give to electrodes
	float32 l_oMin[3];
	float32 l_oMax[3];
	getVisualisationContext().getObjectAxisAlignedBoundingBox(m_oScalpId, l_oMin, l_oMax);
#define ELECTRODE_TO_SCALP_SIZE_RATIO 5e-4f
	float32 l_f32ScalpLength = l_oMax[2] - l_oMin[2];
	float32 l_f32ElectrodeScale = ELECTRODE_TO_SCALP_SIZE_RATIO * l_f32ScalpLength;

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#ifdef TILT_ELECTRODES
	//10-20 electrode mapping is tilted by 10% (18deg) on its X axis. Compute rotation matrix
	//that transforming coordinates back into "standard" frame (-18deg)
	//float32 l_oXTransform[3] = { 1, 0, 0}; //identity
	float32 l_oYTransform[3];
	l_oYTransform[0] = 0; l_oYTransform[1] = cosf(-0.1f * M_PI); l_oYTransform[2] = sinf(-0.1f * M_PI);
	float32 l_oZTransform[3];
	l_oZTransform[0] = 0; l_oZTransform[1] = -sinf(-0.1f * M_PI); l_oZTransform[2] = cosf(-0.1f * M_PI);
#endif

	//for each channel, look for intersection of projCenter->channelCoord ray with scalp triangles
	for(uint32 i=0; i<l_ui64ChannelCount; i++)
	{
		//get normalized channel coords from DB
		float64* l_pNormalizedChannelCoords = NULL;
		if(m_pTopographicMapDatabase->getChannelPosition(i, l_pNormalizedChannelCoords) == true)
		{
#ifdef TILT_ELECTRODES
			//transform them to "electrophysiological standard space" : x right, y front, z up
			float64 l_pStandardChannelCoords[3];
			l_pStandardChannelCoords[0] = l_pNormalizedChannelCoords[0];
			l_pStandardChannelCoords[1] = l_oYTransform[1] * l_pNormalizedChannelCoords[1] + l_oYTransform[2] * l_pNormalizedChannelCoords[2];
			l_pStandardChannelCoords[2] = l_oZTransform[1] * l_pNormalizedChannelCoords[1] + l_oZTransform[2] * l_pNormalizedChannelCoords[2];

			//transform them to Ogre space to get normalized ray direction
			float32 l_pRayDirection[3];
			convert_std_to_ov(l_pRayDirection, l_pStandardChannelCoords);
#else
			//transform them to Ogre space to get normalized ray direction
			float32 l_pRayDirection[3];
			convert_std_to_ov(l_pRayDirection, l_pNormalizedChannelCoords);
#endif
			//look for scalp triangle intersected by ray
			uint32 j = 0;
			float32 l_t;
			boolean l_bIntersectionFound = false;

			for(j=0; j<l_ui32ScalpTriangleCount; j++)
			{
				l_bIntersectionFound = false;

				if(findRayTriangleIntersection(l_pProjectionCenter, l_pRayDirection,
					m_pScalpVertices + 3*l_pScalpTriangleIndexArray[3*j],
					m_pScalpVertices + 3*l_pScalpTriangleIndexArray[3*j+1],
					m_pScalpVertices + 3*l_pScalpTriangleIndexArray[3*j+2],
					l_t) == true && l_t > 0)
				{
					l_bIntersectionFound = true;
					break;
				}
			}

#ifdef PROJECT_ON_FACE
				if(l_bIntersectionFound == false && l_pFaceVertices != NULL && l_pFaceTriangleIndexArray != NULL) //try and project it on face
				{
					for(j=0; j<l_ui32FaceTriangleCount; j++)
					{
						if(findRayTriangleIntersection(l_pProjectionCenter, l_pRayDirection,
							l_pFaceVertices + 3*l_pFaceTriangleIndexArray[3*j],
							l_pFaceVertices + 3*l_pFaceTriangleIndexArray[3*j+1],
							l_pFaceVertices + 3*l_pFaceTriangleIndexArray[3*j+2],
							l_t) == true && l_t > 0)
						{
							l_bIntersectionFound = true;
							break;
						}
					}
				}
#endif

			if(l_bIntersectionFound == false)
			{
				CString l_oChannelLabel;
				m_pTopographicMapDatabase->getChannelLabel(i, l_oChannelLabel);
				getLogManager() << LogLevel_Warning << "Channel " << (i+1) << "(" << l_oChannelLabel << ") couldn't be projected on scalp! "
					<< "No graphical object will be created for this channel!\n";
			}
			else
			{
				m_oElectrodeIds[i] = getVisualisationContext().createObject(Standard3DObject_Sphere);
				if(m_oElectrodeIds[i] == OV_UndefinedIdentifier)
				{
					getLogManager() << LogLevel_Warning << "Couldn't create electrode object!\n";
					break;
				}

				//should electrodes be shown initially?
				if(m_bNeedToggleElectrodes == false)
				{
					getVisualisationContext().setObjectVisible(m_oElectrodeIds[i], false);
				}

				//scale electrode so that it is homogeneous with skull model
				getVisualisationContext().setObjectScale(m_oElectrodeIds[i], l_f32ElectrodeScale);

				//position electrode
				getVisualisationContext().setObjectPosition(m_oElectrodeIds[i],
					l_pProjectionCenter[0] + l_t * l_pRayDirection[0],
					l_pProjectionCenter[1] + l_t * l_pRayDirection[1],
					l_pProjectionCenter[2] + l_t * l_pRayDirection[2]);
			}
		}
	}

	m_bModelElectrodeCoordinatesInitialized = true;

	delete[] l_pScalpTriangleIndexArray;

#ifdef PROJECT_ON_FACE
	if(l_pFaceVertices != NULL)
	{
		delete[] l_pFaceVertices;
	}
	if(l_pFaceTriangleIndexArray != NULL)
	{
		delete[] l_pFaceTriangleIndexArray;
	}
#endif

	return true;
}

boolean CTopographicMap3DDisplay::findRayTriangleIntersection(float32* pOrigin, float32* pDirection, float32* pV0, float32* pV1, float32* pV2, float32& rT)
{
	float32 u, v;
	float32 edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	float32 det, inv_det;

	/* find vectors for two edges sharing pV0 */
	SUB(edge1, pV1, pV0);
	SUB(edge2, pV2, pV0);

	/* begin calculating determinant - also used to calculate U parameter */
	CROSS(pvec, pDirection, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DOT(edge1, pvec);

#ifdef TEST_CULL           /* define TEST_CULL if culling is desired */
	if (det < EPSILON)
		return false;

	/* calculate distance from pV0 to ray origin */
	SUB(tvec, pOrigin, pV0);

	/* calculate U parameter and test bounds */
	u = DOT(tvec, pvec);
	if (u < 0.0 || u > det)
		return false;

	/* prepare to test V parameter */
	CROSS(qvec, tvec, edge1);

	/* calculate V parameter and test bounds */
	v = DOT(pDirection, qvec);
	if (v < 0.0 || u + v > det)
		return false;

	/* calculate t, scale parameters, ray intersects triangle */
	rT = DOT(edge2, qvec);
	inv_det = 1.0 / det;
	rT *= inv_det;
	u *= inv_det;
	v *= inv_det;
#else                    /* the non-culling branch */
	if (det > -EPSILON && det < EPSILON)
		return false;
	inv_det = (float32)(1.0 / det);

	/* calculate distance from pV0 to ray origin */
	SUB(tvec, pOrigin, pV0);

	/* calculate U parameter and test bounds */
	u = DOT(tvec, pvec) * inv_det;
	if (u < 0.0 || u > 1.0)
		return false;

	/* prepare to test V parameter */
	CROSS(qvec, tvec, edge1);

	/* calculate V parameter and test bounds */
	v = DOT(pDirection, qvec) * inv_det;
	if (v < 0.0 || u + v > 1.0)
		return false;

	/* calculate t, ray intersects triangle */
	rT = DOT(edge2, qvec) * inv_det;
#endif
	return true;
}

boolean CTopographicMap3DDisplay::process3D()
{
	//first pass : initialize 3D scene
	if(m_bSkullCreated == false)
	{
		boolean l_bRes = createSkull();
		m_bSkullCreated = true;
		return l_bRes;
	}
	//second pass : auto position camera
	else if(m_bCameraPositioned == false)
	{
		boolean l_bRes = getVisualisationContext().setCameraToEncompassObjects(m_o3DWidgetIdentifier);
		m_bCameraPositioned = true;
		return l_bRes;
	}
	else
	{
		//initialize scalp vertices
		if(m_bScalpDataInitialized == false)
		{
			boolean l_bRes = initializeScalpData();
			m_bScalpDataInitialized = true;
			if(l_bRes == false)
			{
				return false;
			}
		}

		//create electrode objects
		if(m_bElectrodesCreated == false)
		{
			//ensure normalized channel coords are available
			if(m_pTopographicMapDatabase->isFirstChannelLocalisationBufferProcessed() == true)
			{
				//compute channel coords in 3d scene
				bool l_bRes = computeModelFrameChannelCoordinates();
				m_bElectrodesCreated = true;
				return l_bRes;
			}
		}

		//should electrode objects be toggled on/off?
		if(m_bNeedToggleElectrodes == true)
		{
			for(uint32 i=0; i<m_oElectrodeIds.size(); i++)
			{
				getVisualisationContext().setObjectVisible(m_oElectrodeIds[i], m_bElectrodesToggleState);
			}

			m_bNeedToggleElectrodes = false;
		}

		//should sampling point objects be toggled on/off?
		if(m_bNeedToggleSamplingPoints == true)
		{
			for(uint32 i=0; i<m_oSamplingPointIds.size(); i++)
			{
				getVisualisationContext().setObjectVisible(m_oSamplingPointIds[i], m_bSamplingPointsToggleState);
			}

			m_bNeedToggleSamplingPoints = false;
		}

		//update map
		boolean l_bProcess = m_pTopographicMapDatabase->processValues();

		//disable plugin upon errors
		return l_bProcess == true;
	}
	return true;
}

boolean CTopographicMap3DDisplay::createSkull()
{
	//set background color
	getVisualisationContext().setBackgroundColor(m_o3DWidgetIdentifier, 0, 0, 0);

	//load face mesh
	m_oFaceId = getVisualisationContext().createObject(m_oFaceMeshFilename);
	if(m_oFaceId == OV_UndefinedIdentifier)
	{
		getLogManager() << LogLevel_Error << "Couldn't load face mesh!\n";
		m_bError = true;
		return false;
	}

	//load scalp mesh
	CNameValuePairList l_oParams;
	l_oParams.setValue("CloneMeshes", true); //clone scalp mesh so that it doesn't interfere with other maps
	l_oParams.setValue("VertexBufferUsage", "Dynamic"); //vertex colors will be updated regularly
	m_oScalpId = getVisualisationContext().createObject(m_oScalpMeshFilename, &l_oParams);
	if(m_oScalpId == OV_UndefinedIdentifier)
	{
		getLogManager() << LogLevel_Error << "Couldn't load scalp mesh!\n";
		m_bError = true;
		return false;
	}

	//load projection sphere mesh
	CIdentifier l_oDummyObject = getVisualisationContext().createObject(m_oProjectionSphereMeshFilename);
	if(l_oDummyObject == OV_UndefinedIdentifier)
	{
		getLogManager() << LogLevel_Error << "Couldn't load projection sphere mesh!\n";
		m_bError = true;
		return false;
	}
	else
	{
		float32 l_oMin[3];
		float32 l_oMax[3];
		getVisualisationContext().getObjectAxisAlignedBoundingBox(l_oDummyObject, l_oMin, l_oMax);

		float32 l_oModelSpaceProjectionCenter[3];
		l_oModelSpaceProjectionCenter[0] = (l_oMin[0] + l_oMax[0]) / 2;
		l_oModelSpaceProjectionCenter[1] = (l_oMin[1] + l_oMax[1]) / 2;
		l_oModelSpaceProjectionCenter[2] = (l_oMin[2] + l_oMax[2]) / 2;

		convert_ov_to_std(m_f32ProjectionCenter, l_oModelSpaceProjectionCenter);

		getVisualisationContext().removeObject(l_oDummyObject);
	}

	return true;
}

boolean CTopographicMap3DDisplay::createSamplingPoints()
{
	boolean l_bRes = true;

	//display a sphere at the vertex location
	m_oSamplingPointIds.resize(m_ui32NbScalpVertices);

	for(uint32 i=0; i<m_ui32NbScalpVertices; i++)
	{
		m_oSamplingPointIds[i] = getVisualisationContext().createObject(Standard3DObject_Sphere);

		if(m_oSamplingPointIds[i] == OV_UndefinedIdentifier)
		{
			getLogManager() << LogLevel_Warning << "process3D() : couldn't create electrode object!\n";
			l_bRes = false;
			break;
		}

		//hide objects by default
		getVisualisationContext().setObjectVisible(m_oSamplingPointIds[i], false);

		//position electrode at vertex location
		getVisualisationContext().setObjectPosition(m_oSamplingPointIds[i], m_pScalpVertices[3*i], m_pScalpVertices[3*i+1], m_pScalpVertices[3*i+2]);

		float64 l_f64VertexX, l_f64VertexY, l_f64VertexZ;
		l_f64VertexX = *(m_oSampleCoordinatesMatrix.getBuffer() + 3*i);
		l_f64VertexY = *(m_oSampleCoordinatesMatrix.getBuffer() + 3*i+1);
		l_f64VertexZ = *(m_oSampleCoordinatesMatrix.getBuffer() + 3*i+2);

		//map theta and phi to size/color
		float64 theta = acos(l_f64VertexZ);
		float64 phi;
		if(l_f64VertexX > 0.001)
		{
			phi = atan(l_f64VertexY / l_f64VertexX);

			if(phi < 0)
			{
				phi += 2 * M_PI;
			}
		}
		else if(l_f64VertexX < -0.001)
		{
			phi = atan(l_f64VertexY / l_f64VertexX) + M_PI;
		}
		else
		{
			phi = l_f64VertexY > 0 ? (M_PI / 2) : (3 * M_PI/2);
		}

		static float64 minScale = 0.0005f;
		static float64 maxScale = 0.002f;
#if 0 //reflect phi angle on size
		float64 scale = minScale + (maxScale - minScale) * phi / (2*M_PI);
		getVisualisationContext().setObjectScale(m_oSamplingPointIds[i], scale, scale, scale);

		getVisualisationContext().setObjectColor(m_oSamplingPointIds[i], theta / M_PI * 1.f, 0, 0);
		getVisualisationContext().setObjectTransparency(m_oSamplingPointIds[i], 0.5f);
#else //reflect theta angle on size
		float32 scale = (float32)(minScale + (maxScale - minScale) * theta / M_PI);
		getVisualisationContext().setObjectScale(m_oSamplingPointIds[i], scale, scale, scale);

		getVisualisationContext().setObjectColor(m_oSamplingPointIds[i], (float32)(phi / (2*M_PI) * 1.f), 0, 0);
		getVisualisationContext().setObjectTransparency(m_oSamplingPointIds[i], 0.5f);
	}
#endif
	return l_bRes;
}
