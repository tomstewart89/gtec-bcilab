#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __OpenGLVBOManager_OV_H__
#define __OpenGLVBOManager_OV_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "OpenGLResourceManager.h"

namespace OpenViBEApplications
{
	class OpenGLVBOManager : public OpenGLResourceManager {
	public:
		OpenGLVBOManager(OpenViBE::uint32 n);// : OpenGLResourceManager(n) {}
		OpenGLVBOManager(OpenGLVBOManager* gl_manager) : OpenGLResourceManager(gl_manager) {}
		virtual ~OpenGLVBOManager();
		virtual OpenGLVBOManager* clone() const { return new OpenGLVBOManager((OpenGLVBOManager*)this); }
		
		//virtual void createResources();
		virtual void deleteResources();
		
	protected:
		virtual std::map<GLuint, OpenViBE::uint32>& getResourceMap();
		virtual void _createResources();
		virtual void _deleteResource(GLuint* resource_id);		
		//static void forwarder(OpenGLResourceManager* context);
		
	protected:
		static std::map<GLuint, OpenViBE::uint32> m_mIdCountMap;
	};

};

#endif
#endif

#endif
