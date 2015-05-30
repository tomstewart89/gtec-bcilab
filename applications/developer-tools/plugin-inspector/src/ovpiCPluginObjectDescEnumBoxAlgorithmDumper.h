#ifndef __OpenViBEPluginInspector_CPluginObjectDescEnumBoxAlgorithmDumper_H__
#define __OpenViBEPluginInspector_CPluginObjectDescEnumBoxAlgorithmDumper_H__

// 
// Dumps all registered box algorithms to a text file or cout. The intended use case
// is to check the current scenarios to see if all boxes have at least one 
// scenario where it is used. n.b. The checking was done by a shell script in 
//
// test/check-scenario-coverage.sh
//

#include "ovpiCPluginObjectDescEnum.h"

#include <map>
#include <vector>
#include <string>
#include <fstream>

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

class CPluginObjectDescEnumBoxAlgorithmDumper : public CPluginObjectDescEnum
{
public:

	CPluginObjectDescEnumBoxAlgorithmDumper(const OpenViBE::Kernel::IKernelContext& rKernelContext, const OpenViBE::CString& sOutFile);
	virtual ~CPluginObjectDescEnumBoxAlgorithmDumper(void);
	virtual OpenViBE::boolean callback(const OpenViBE::Plugins::IPluginObjectDesc& rPluginObjectDesc);

protected:

	std::ofstream m_oFile;
	bool m_bWriteToFile;	// if false, to console

};

#endif // __OpenViBEPluginInspector_CPluginObjectDescEnumBoxAlgorithmDumper_H__
