#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "OpenGLDListManager.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

std::map<GLuint, uint32> OpenGLDListManager::m_mIdCountMap = std::map<GLuint, uint32>();

OpenGLDListManager::OpenGLDListManager(OpenViBE::uint32 n) : OpenGLResourceManager(n) 
{
	OpenGLResourceManager::createResources(this);
}

OpenGLDListManager::~OpenGLDListManager()
{
	deleteResources();
}

/*void OpenGLDListManager::createResources()
{
	OpenGLResourceManager::createResources(this);
}*/

void OpenGLDListManager::deleteResources()
{
	OpenGLResourceManager::deleteResources(this);	
}

std::map<GLuint, uint32>& OpenGLDListManager::getResourceMap()
{
	return OpenGLDListManager::m_mIdCountMap;
}

void OpenGLDListManager::_createResources()
{
	uint32 l_ui32BaseIndex = glGenLists(m_uiNumberOfGLResources);
	for (uint32 i=0; i<m_uiNumberOfGLResources; i++)
	{
		m_uiGLResourceIds[i] = l_ui32BaseIndex+i;
		//std::cout << "OpenGLDListManager::requested openGL resource id " << m_uiGLResourceIds[i] << "\n";
	}
}

void OpenGLDListManager::_deleteResource(GLuint* resource_id)
{
	//std::cout << "OpenGLDListManager:: Deleting opengl resource with id " << *resource_id << "\n";
	glDeleteLists(*resource_id,1);
}
#endif

#endif
