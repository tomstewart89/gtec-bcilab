#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __OpenGLDListManager_OV_H__
#define __OpenGLDListManager_OV_H__

#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "OpenGLResourceManager.h"

namespace OpenViBEApplications
{
	/**
	 * Resource manager for OpenGL display lists (this is deprecated in OpenGL, everything should be using vbo buffers)
	 */
	class OpenGLDListManager : public OpenGLResourceManager {
	public:
		/**
		 * Calls the constructor of the super class OpenGLResourceManager and then calls the static createResources of the super
		 * class given itself as argument
		 */
		OpenGLDListManager(OpenViBE::uint32 n);// : OpenGLResourceManager(n) {}
		OpenGLDListManager(OpenGLDListManager* gl_manager) : OpenGLResourceManager(gl_manager) {}
		
		/**
		 * Destructor will call deleteResources which in turn will call the static deleteResources of the super class
		 * OpenGLResourceManager with itself as argument
		 */
		virtual ~OpenGLDListManager();
		virtual OpenGLDListManager* clone() const { return new OpenGLDListManager((OpenGLDListManager*)this); }
		
		//virtual void createResources();
		/**
		 * calls the static deleteResources of the super class OpenGLResourceManager with itself as argument
		 * TODO this method should be hidden
		 */
		virtual void deleteResources();
		
	protected:
		/**
		 * @return returns the static resource id map from the super class
		 */
		virtual std::map<GLuint, OpenViBE::uint32>& getResourceMap();
		/**
		 * specific code for creating the display list resource
		 */
		virtual void _createResources();
		/**
		 * specific code for deleting the display list resource
		 */
		virtual void _deleteResource(GLuint* resource_id);		
		//static void forwarder(OpenGLResourceManager* context);
		
	protected:
		static std::map<GLuint, OpenViBE::uint32> m_mIdCountMap;
	};

};

#endif
#endif

#endif
