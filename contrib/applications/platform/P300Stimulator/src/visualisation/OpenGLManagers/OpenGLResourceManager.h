#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __OpenGLResourceManager_OV_H__
#define __OpenGLResourceManager_OV_H__

#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "../../ova_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <map>

namespace OpenViBEApplications
{
	/**
	 * This class and its inheriting classes will create, delete and manage OpenGL resources such as display list, VBO buffers and
	 * textures.\n
	 * No objects of this class can be constructed only of the specific inheriting classes.
	 * Each graphical object should call a constructor from the derived class and specify how many resources it needs
	 */
	class OpenGLResourceManager {
	//protected:
	//	typedef void (OpenGLResourceManager::*createOpenGLResource)();
		
	protected:
		/**
		 * Creates an array to hold n OpenGL resource id's of type GLuint
		 * @param n number of resource id's that will need to be managed
		 */
		OpenGLResourceManager(OpenViBE::uint32 n);
		
		/**
		 * Copy constructor. It will copy the resource ids and increase their count by one.\n
		 * The static member variable m_mIdCountMap keeps track of all resource ids managed
		 * by all OpenGLResourceManager objects and their count, i.e. how many OpenGLResourceManager
		 * objects that are using the same resource.
		 */
		OpenGLResourceManager(OpenGLResourceManager* gl_manager);
	public:
		/**
		 * Destructor deletes the array holding the resource ids
		 */
		virtual ~OpenGLResourceManager();
		
		/**
		 * Pure virtual clone function calls the copy constructor
		 */
		virtual OpenGLResourceManager* clone() const = 0;
		
		/**
		 * TODO: I have the feeling this function should be hidden somehow\n
		 * Pure virtual function that in its derived class implementations calls the protected 
		 * OpenGLResourceManager::deleteResources(this)
		 * @see deleteResources(OpenGLResourceManager* context)
		 */
		virtual void deleteResources() = 0;
		
		/**
		 * @param gl_index the index for which you want to get the resource id. If OpenGLResourceManager manages two display lists
		 * for example then you can retrieve the resource id of the second one by specifying gl_index=1
		 * @return the resource id at index gl_index
		 */
		virtual GLuint getResource(OpenViBE::uint32 gl_index) { return *(m_uiGLResourceIds+gl_index); }
		
		/**
		 * @return the number of OpenGL resources OpenGLResourceManager manages
		 */
		virtual OpenViBE::uint32 getNumberOfResources() { return m_uiNumberOfGLResources; }
			
	protected:
		/**
		 * Calls context->_createResources() on the derived OpenGLResourceManager class object which creates a number of
		 * specific OpenGL resources and sets the resource ids. It then inserts the ids in the static member map m_mIdCountMap with count one.
		 * @param context the derived OpenGLResourceManager class object which will call the specific OpenGL code to request the resources
		 * @see _createResources()
		 */
		static void createResources(OpenGLResourceManager* context);
		
		/**
		 * It will iterate over all resource ids that are managed by the OpenGLResourceManager object context
		 * and decrease their count by one. In case the count reaches zero, the OpenGL resource will be deleted, if not
		 * the display list, vbo buffer or texture remains alive
		 * @param context the OpenGLResourceManager for which the resources can be released (if count reaches zero)
		 */
		static void deleteResources(OpenGLResourceManager* context);
		
		/**
		 * @return the static member map that keeps track of the resource ids and their counts
		 */
		virtual std::map<GLuint, OpenViBE::uint32>& getResourceMap() = 0;
		
		/**
		 * Pure virtual function that calls the specific OpenGL (depending on the derived class, e.g. glGenLists) code to create
		 * the resources
		 */
		virtual void _createResources() = 0;
		
		/**
		 * Pure virtual function that calls the specific OpenGL (depending on the derived class, e.g. glGenLists) code to delete
		 * the resources
		 * @param resource_id the resource id that needs to be deleted
		 */		
		virtual void _deleteResource(GLuint* resource_id) = 0;		
		
	protected:
		static std::map<GLuint, OpenViBE::uint32> m_mIdCountMap; /**< the map that keeps track of all the resource ids requested by all manager class objects of a specific type and the associated count of their use */
		OpenViBE::uint32 m_uiNumberOfGLResources;/**< the number of resource ids requested */
		GLuint* m_uiGLResourceIds;/**< the resource ids */				
	};

};

#endif//TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#endif

#endif
