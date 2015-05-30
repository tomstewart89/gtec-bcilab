#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "OpenGLVBOManager.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

std::map<GLuint, uint32> OpenGLVBOManager::m_mIdCountMap = std::map<GLuint, uint32>();

OpenGLVBOManager::OpenGLVBOManager(OpenViBE::uint32 n) : OpenGLResourceManager(n) 
{
	OpenGLResourceManager::createResources(this);
}

OpenGLVBOManager::~OpenGLVBOManager()
{
	deleteResources();
}

/*void OpenGLVBOManager::createResources()
{
	OpenGLResourceManager::createResources(this);
}*/

void OpenGLVBOManager::deleteResources()
{
	OpenGLResourceManager::deleteResources(this);	
}

std::map<GLuint, uint32>& OpenGLVBOManager::getResourceMap()
{
	return OpenGLVBOManager::m_mIdCountMap;
}

void OpenGLVBOManager::_createResources()
{
	//glGenBuffers(m_uiNumberOfGLResources,m_uiGLResourceIds);
	//for (uint32 i=0; i<m_uiNumberOfGLResources; i++)
	//	std::cout << "OpenGLVBOManager::requested openGL resource id " << m_uiGLResourceIds[i] << "\n";
}

void OpenGLVBOManager::_deleteResource(GLuint* resource_id)
{
	//std::cout << "OpenGLVBOManager:: Deleting opengl resource with id " << *resource_id << "\n";
	//glDeleteBuffers(1,resource_id);	
}

/*void forwarder(OpenGLResourceManager* context) {
    dynamic_cast<OpenGLVBOManager*>(context)->generateOpenGLResources();
}*/
#endif

#endif
