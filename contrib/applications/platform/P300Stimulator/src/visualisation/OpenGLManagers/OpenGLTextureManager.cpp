#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "OpenGLTextureManager.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

std::map<GLuint, uint32> OpenGLTextureManager::m_mIdCountMap = std::map<GLuint, uint32>();
std::map<CString, GLuint> OpenGLTextureManager::m_mSourceFile2IdMap = std::map<CString, GLuint>();
std::map<GLuint, CString> OpenGLTextureManager::m_mId2SourceFileMap = std::map<GLuint, CString>();

OpenGLTextureManager::OpenGLTextureManager(CString sourceFile) : OpenGLResourceManager(1), m_sSourceFile(sourceFile)
{
	std::map<CString, GLuint>::iterator l_FindResult = m_mSourceFile2IdMap.find(sourceFile);
	//OpenGLTexture manager created for a new source that did not already exist
	if (l_FindResult == m_mSourceFile2IdMap.end())
	{
		m_bSourceExists = false;
		OpenGLResourceManager::createResources(this);
		m_mSourceFile2IdMap.insert(std::pair<CString,GLuint>(sourceFile,m_uiGLResourceIds[0]));
		m_mId2SourceFileMap.insert(std::pair<GLuint,CString>(m_uiGLResourceIds[0],sourceFile));
	}
	//OpenGLTexture manager created for existing source
	else
	{
		m_bSourceExists = true;
		m_uiGLResourceIds[0] = l_FindResult->second;
		m_mIdCountMap[ l_FindResult->second ]++;
	}
}

OpenGLTextureManager::~OpenGLTextureManager()
{
	deleteResources();
	
	std::map<GLuint, uint32>::iterator l_FindResultIdCount = m_mIdCountMap.find(m_uiGLResourceIds[0]);
	if (l_FindResultIdCount == m_mIdCountMap.end())
	{
		std::cout << "Delete opengl texture manager for source " << m_sSourceFile.toASCIIString() << "\n";
		m_mId2SourceFileMap.erase(m_uiGLResourceIds[0]);
		m_mSourceFile2IdMap.erase(m_sSourceFile);
	}
	
}

void OpenGLTextureManager::deleteResources()
{
	OpenGLResourceManager::deleteResources(this);	
}

std::map<GLuint, uint32>& OpenGLTextureManager::getResourceMap()
{
	return OpenGLTextureManager::m_mIdCountMap;
}

void OpenGLTextureManager::_createResources()
{
	glGenTextures(m_uiNumberOfGLResources, m_uiGLResourceIds);
}

void OpenGLTextureManager::_deleteResource(GLuint* resource_id)
{
	glDeleteTextures(1,resource_id);	
}
#endif

#endif
