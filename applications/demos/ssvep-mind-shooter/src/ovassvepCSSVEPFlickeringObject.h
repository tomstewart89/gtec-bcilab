#ifndef __OpenViBEApplication_CSSVEPFlickeringObject_H__
#define __OpenViBEApplication_CSSVEPFlickeringObject_H__

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <Ogre.h>

namespace OpenViBESSVEP
{
	class CSSVEPFlickeringObject
	{
		public:
//SP			CSSVEPFlickeringObject( Ogre::SceneNode* poObjectNode, OpenViBE::uint32 ui32LitFrames, OpenViBE::uint32 ui32DarkFrames );
			CSSVEPFlickeringObject( Ogre::SceneNode* poObjectNode, OpenViBE::uint64 ui64StimulationPattern );
			~CSSVEPFlickeringObject() {};

			virtual void setVisible( OpenViBE::boolean bVisibility );
			virtual void processFrame();

		protected:
			Ogre::SceneNode* m_poObjectNode;
			OpenViBE::uint32 m_ui32CurrentFrame;
//SP			OpenViBE::uint32 m_ui32LitFrames;
//SP			OpenViBE::uint32 m_ui32DarkFrames;
			OpenViBE::uint64 m_ui64StimulationPattern;

		private:

			OpenViBE::boolean m_bVisible;
			int m_iId;
	};
}


#endif // __OpenViBEApplication_CSSVEPFlickeringObject_H__
