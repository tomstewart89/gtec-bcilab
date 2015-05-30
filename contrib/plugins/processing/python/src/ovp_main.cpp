#if defined(WIN32) && defined(TARGET_BUILDTYPE_Debug)
// Windows debug build doesn't typically link as most people don't have the python debug library.
#else

#if defined TARGET_HAS_ThirdPartyPython

#include "box-algorithms/ovpCBoxAlgorithmPython.h"

#ifdef TARGET_OS_Windows
#include "windows.h"
#endif

#include <string>
#include <iostream>

class CPythonInitializer
{
public:
	CPythonInitializer(void);
	~CPythonInitializer(void);
	OpenViBE::boolean IsPythonAvailable(void);
private:
#ifdef TARGET_OS_Windows
	bool checkPythonPath();
#endif
	//		PyThreadState *m_pMainPyThreadState;
	OpenViBE::boolean m_bPythonAvailable;
};

#ifdef TARGET_OS_Windows
bool CPythonInitializer::checkPythonPath()
{
	std::string l_sPath = Py_GetPath();

	int found;
	found = l_sPath.find_first_of(";");
	while(found != std::string::npos)
	{
		if(found > 0)
		{
			std::string l_sFilename = l_sPath.substr(0,found);
			bool l_bExists = (_access(l_sFilename.c_str(), 0) == 0);
			if (l_bExists)
			{
				// std::cout << "Found Python in : " << l_sPath.substr(0,found) << std::endl;
				return true;
			}
			else
			{
				//std::cout << "NOT found : " << l_sPath.substr(0,found) << std::endl;
			}
		}
		l_sPath = l_sPath.substr(found+1);
		found = l_sPath.find_first_of(";");
	}
	
	std::cout << "Python directory not found. You probably have a corrupted python installation!" << std::endl;
	std::cout << "The tried path from Py_GetPath() was [" << Py_GetPath() << "]\n";

	return false;
}
#endif

CPythonInitializer::CPythonInitializer(void) : 
	m_bPythonAvailable(false)
{
	//m_pMainPyThreadState = NULL;

#ifdef TARGET_OS_Windows
		__try
		{
			if (!Py_IsInitialized())
			{
				// We do not care about the last file, since it is the OpenViBE runtime path
				if (checkPythonPath())
				{
					Py_Initialize();
					m_bPythonAvailable = true;
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
#else
		if (!Py_IsInitialized())
		{
			Py_Initialize();
			m_bPythonAvailable = true;
		}
#endif



}

CPythonInitializer::~CPythonInitializer(void)
{
	if (m_bPythonAvailable)
	{
		m_bPythonAvailable = false;
		Py_Finalize();
	}
}

OpenViBE::boolean CPythonInitializer::IsPythonAvailable(void)
{
	return m_bPythonAvailable;
}


OVP_Declare_Begin();

	static CPythonInitializer l_oPythonInitializer;
	if (l_oPythonInitializer.IsPythonAvailable())
	{
		OVP_Declare_New(OpenViBEPlugins::Python::CBoxAlgorithmPythonDesc);
	}

OVP_Declare_End();

#endif // TARGET_HAS_ThirdPartyPython

#endif
