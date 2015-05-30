#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __P300BackspaceHandler_H__
#define __P300BackspaceHandler_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <list>
#include <cstring>

#include "../visualisation/glGTable.h"
#include "../visualisation/glGLabel.h"

namespace OpenViBEApplications
{	
	//template<class TContainer>
	class P300BackspaceHandler : public GObserver, public GObservable
	{	

	public:
		
		P300BackspaceHandler()//(GTable* container) : m_pSymbolContainer(container)
		{
			m_sBackspaceStack = new std::list< std::string >();
		}
		
		~P300BackspaceHandler()
		{
			/*for (OpenViBE::uint32 i=0; i<m_sBackspaceStack->size(); i++)
			{
				for (OpenViBE::uint32 j=0; j<m_sBackspaceStack->top()->size(); j++)
				{
					delete m_sBackspaceStack->top()->top();
					m_sBackspaceStack->top()->pop();
				}
				delete m_sBackspaceStack->top();
				m_sBackspaceStack->pop();
			}*/
			delete m_sBackspaceStack;
		}
		
		//inherited from GObserver
		virtual void update(GObservable* observable, const void * pUserData);
		
	protected:
		//GTable* m_pSymbolContainer;
		//std::stack< std::stack<GLabel*>* >* m_sBackspaceStack;
		std::list< std::string >* m_sBackspaceStack;
		std::string m_sSpeltLetters;
	};
};
#endif
#endif

#endif
