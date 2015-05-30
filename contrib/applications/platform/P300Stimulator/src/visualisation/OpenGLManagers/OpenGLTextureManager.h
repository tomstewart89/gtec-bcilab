#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __OpenGLTextureManager_OV_H__
#define __OpenGLTextureManager_OV_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "OpenGLResourceManager.h"
//#include <set>

namespace OpenViBEApplications
{
	/**
	 * manages the texture resources used by glGPictureSymbol. The class is slightly different from
	 * the OpenGLDListManager and OpenGLVBOManager as it takes a string in the constructor
	 */
	class OpenGLTextureManager : public OpenGLResourceManager {
	public:
		/**
		 * The string sourceFile will be used to see if a texture for this file already exist (saved in m_mSourceFile2IdMap)
		 * A resource id will be created as well for this texture and will be mapped to its file name by m_mId2SourceFileMap
		 * @param sourceFile the file name of the png file that will serve as texture
		 */
		OpenGLTextureManager(OpenViBE::CString sourceFile);
		OpenGLTextureManager(OpenGLTextureManager* gl_manager) : OpenGLResourceManager(gl_manager)
		{ 
			//m_bSourceExists = gl_manager.m_bSourceExists; 
		}
		virtual ~OpenGLTextureManager();
		virtual OpenGLTextureManager* clone() const { return new OpenGLTextureManager((OpenGLTextureManager*)this); }
		
		//virtual void createResources();
		virtual void deleteResources();
		OpenViBE::boolean sourceExists() { return m_bSourceExists; }
		
	protected:
		virtual std::map<GLuint, OpenViBE::uint32>& getResourceMap();
		virtual void _createResources();
		virtual void _deleteResource(GLuint* resource_id);		
		//static void forwarder(OpenGLResourceManager* context);
		
	protected:
		static std::map<GLuint, OpenViBE::uint32> m_mIdCountMap;
		/**
		 * maps file name to a resource id
		 */
		static std::map<OpenViBE::CString, GLuint> m_mSourceFile2IdMap;
		/**
		 * maps a resource id to its file name (inverse of m_mSourceFile2IdMap)
		 * TODO not sure where this is exactly used
		 */
		static std::map<GLuint, OpenViBE::CString> m_mId2SourceFileMap;
		OpenViBE::CString m_sSourceFile;
		OpenViBE::boolean m_bSourceExists;
	};

};

#endif
#endif

#endif
