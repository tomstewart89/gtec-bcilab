#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __P300UndoHandler_H__
#define __P300UndoHandler_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <list>
#include <cstring>

#include "../visualisation/glGTable.h"
#include "../visualisation/glGLabel.h"

namespace OpenViBEApplications
{	
	//template<class TContainer>
	class P300UndoHandler : public GObserver, public GObservable
	{	

	public:
		
		P300UndoHandler()//(GTable* container) : m_pSymbolContainer(container)
		{
			m_lUndoStack = new std::list< std::string >();
			m_lRedoStack = new std::list< std::string >();
		}
		
		~P300UndoHandler()
		{
			/*for (OpenViBE::uint32 i=0; i<m_sUndoStack->size(); i++)
			{
				for (OpenViBE::uint32 j=0; j<m_sUndoStack->top()->size(); j++)
				{
					delete m_sUndoStack->top()->top();
					m_sUndoStack->top()->pop();
				}
				delete m_sUndoStack->top();
				m_sUndoStack->pop();
			}*/
			delete m_lUndoStack;
			delete m_lRedoStack;
		}
		
		//inherited from GObserver
		virtual void update(GObservable* observable, const void * pUserData);
		
	protected:
		//GTable* m_pSymbolContainer;
		//std::stack< std::stack<GLabel*>* >* m_sUndoStack;
		std::list< std::string >* m_lUndoStack;
		std::list< std::string >* m_lRedoStack;
	};
};
#endif
#endif

#endif
