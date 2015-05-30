
#include <vector>
#include <openvibe/ov_all.h>

// @BEGIN CICIT-GARCHES
#include "box-algorithms/ovpCBoxAlgorithmEDFFileWriter.h"
// @END CICIT-GARCHES

// @BEGIN GIPSA
#include "box-algorithms/ovpCBoxAlgorithmBrainampFileWriter.h"
// @END GIPSA

OVP_Declare_Begin();

// @BEGIN CICIT-GARCHES
	OVP_Declare_New(OpenViBEPlugins::FileIO::CBoxAlgorithmEDFFileWriterDesc)
// @END CICIT_GARCHES

// @BEGIN GIPSA
	//Register dropdowns
	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_BinaryFormat, "Binary format select");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_BinaryFormat, "INT_16",            OVP_TypeId_BinaryFormat_int16.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_BinaryFormat, "UINT_16",           OVP_TypeId_BinaryFormat_uint16.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_BinaryFormat, "IEEE_FLOAT_32",     OVP_TypeId_BinaryFormat_float32.toUInteger());

	OVP_Declare_New(OpenViBEPlugins::FileIO::CBoxAlgorithmBrainampFileWriterDesc)
// @END GIPSA

OVP_Declare_End();
	