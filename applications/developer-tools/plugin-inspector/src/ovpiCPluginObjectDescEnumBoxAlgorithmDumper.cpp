#include "ovpiCPluginObjectDescEnumBoxAlgorithmDumper.h"

#include <system/ovCTime.h>

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;


// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

CPluginObjectDescEnumBoxAlgorithmDumper::CPluginObjectDescEnumBoxAlgorithmDumper(const IKernelContext& rKernelContext, const CString& sOutFile)
	:CPluginObjectDescEnum(rKernelContext)
	,m_bWriteToFile(true)
{
	if(sOutFile == CString("")) 
	{
		m_bWriteToFile = false;
	}

	if(m_bWriteToFile)
	{
		m_oFile.open(sOutFile.toASCIIString());
	} 

}

CPluginObjectDescEnumBoxAlgorithmDumper::~CPluginObjectDescEnumBoxAlgorithmDumper(void)
{
	if(m_bWriteToFile)
	{
		m_oFile.close();
	}
}

boolean CPluginObjectDescEnumBoxAlgorithmDumper::callback(const IPluginObjectDesc& rPluginObjectDesc)
{
	
	const CIdentifier l_oBoxIdentifier = rPluginObjectDesc.getCreatedClassIdentifier();

	if(m_bWriteToFile)
	{
		m_oFile << "BoxAlgorithm " << transform(rPluginObjectDesc.getName().toASCIIString()) << " " << l_oBoxIdentifier.toString().toASCIIString() << "\n";
	}
	else
	{
		std::cout << "BoxAlgorithm " << transform(rPluginObjectDesc.getName().toASCIIString()) << " " << l_oBoxIdentifier.toString().toASCIIString() << "\n";	
	}

	return true;
}
