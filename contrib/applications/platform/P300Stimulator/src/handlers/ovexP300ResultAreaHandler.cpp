#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300ResultAreaHandler.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "ovexP300TargetAreaHandler.h"
#include "ovexUndoHandler.h"
#include "ovexBackspaceHandler.h"
#include "../visualisation/glGButton.h"
#include "../visualisation/glGSymbol.h"
#include "../visualisation/glGPictureSymbol.h"

#include <stdexcept>

#include <system/ovCTime.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

//class P300UndoHandler;

P300ResultAreaHandler::P300ResultAreaHandler(GTable* container, P300ScreenLayoutReader* propertyObject) 
: m_pSymbolContainer(container), m_pScreenLayoutObject(propertyObject), m_sSpelledLetters(""), m_ui32ResultCounter(0)
{
	#ifdef OUTPUT_TIMING
	timingFile = fopen(OpenViBE::Directories::getUserDataDir() + "/xP300-gl_result_move_timing.txt","w");
	//for testing purposes, make the result area dump the spleed letters in a file for comparison with a reference
	stackFile = fopen(OpenViBE::Directories::getUserDataDir() + "/xP300-resultStack.txt","w");
	#endif
	
	m_oLastAddedLabel = NULL;
	m_f32LastFontSize = 0;
}


P300ResultAreaHandler::~P300ResultAreaHandler()
{
	#ifdef OUTPUT_TIMING
	fclose(timingFile);
	fclose(stackFile);
	#endif
}

void P300ResultAreaHandler::update(GObservable* observable, const void * pUserData)
{
	P300UndoHandler* l_pUndoHandler = dynamic_cast<P300UndoHandler*>(observable);
	P300BackspaceHandler* l_pBackspaceHandler = dynamic_cast<P300BackspaceHandler*>(observable);
	P300TargetAreaHandler* l_pTargetHandler = dynamic_cast<P300TargetAreaHandler*>(observable);
	GButton* l_pObservedButton = dynamic_cast<GButton*>(observable);
	//if the notification comes from a key of the keyboard
	if (l_pObservedButton!=NULL)
	{
		const GButton* l_pNewButton = static_cast<const GButton*>(pUserData);
		GSymbol* l_pTextLabel = dynamic_cast<GSymbol*>(l_pNewButton->getLabel());
		//GPictureSymbol* l_pPictureLabel = dynamic_cast<GPictureSymbol*>(l_pNewButton->getLabel());

		std::vector<GLabel*>* l_pLabelVector = new std::vector<GLabel*>();
		std::string l_sTextLabel;
		boolean l_bUpdate = true;
		GLabel* l_pLabel;
		
		m_ui32State = l_pNewButton->getState();
		switch (m_ui32State)
		{
			case GButton_WrongClick:		
			case GButton_Clicked:
				if (l_pTextLabel!=NULL)
				{
					l_sTextLabel = l_pTextLabel->getTextLabel();
					//if the key selected is a predicted word, only add the letters to complete the word
					if (l_pObservedButton->getAction()==GButton_WordPrediction)
					{
						this->trimPrefix(l_sTextLabel);
						for (uint32 i=0; i<l_sTextLabel.length(); i++)
						{
							l_pLabel = l_pTextLabel->clone();
							dynamic_cast<GSymbol*>(l_pLabel)->setTextLabel(l_sTextLabel.substr(i,1).c_str());
							l_pLabelVector->push_back(l_pLabel);
						}
					}
					//adds all the symbols of the key that has been selected
					else
					{	
						l_pLabel = l_pTextLabel->clone();
						dynamic_cast<GSymbol*>(l_pLabel)->setTextLabel(l_sTextLabel.c_str());
						l_pLabelVector->push_back(l_pLabel);
					}
				}
				else
					//TODO if it is an icon then we should represent it by one entry on the undo stack
					l_sTextLabel = std::string(" "); 
				break;	
			default:
				l_bUpdate = false;
				break;
		}
		if (l_bUpdate)
		{
			//you should never add more symbols than the size of the result bar, remove some of the first
			//letters if necessary
			if (l_pLabelVector->size()>m_pSymbolContainer->getColumnDimension())
			{
				uint32 l_ui32SizeDifference = l_pLabelVector->size() - m_pSymbolContainer->getColumnDimension();
				l_pLabelVector->erase(l_pLabelVector->begin(), l_pLabelVector->begin()+l_ui32SizeDifference);
			}
			
			#ifdef OUTPUT_TIMING
			fprintf(timingFile, "%f\n",float64((System::Time::zgetTime()>>22)/1024.0));	
			#endif
			//move the symbols to the left if the symbols you add make your result bar overflow
			if (m_ui32ResultCounter+l_pLabelVector->size()>m_pSymbolContainer->getColumnDimension())
				moveSymbolsLeft(l_pLabelVector->size());
			#ifdef OUTPUT_TIMING
			fprintf(timingFile, "%f\n",float64((System::Time::zgetTime()>>22)/1024.0));
			#endif
			
			//add your symbols and set the background color in case it was the wrong symbol in copy mode
			for (uint32 i=0, j=m_ui32ResultCounter; i<l_pLabelVector->size();  i++,j++)
			{
				if (m_ui32State==GButton_WrongClick)
					l_pLabelVector->at(i)->setBackgroundColor(m_pScreenLayoutObject->getDefaultBackgroundColor(CENTRAL_FEEDBACK_WRONG));
				m_pSymbolContainer->addChild(l_pLabelVector->at(i),l_pLabel->getDepth());
			}
			m_oLastAddedLabel = l_pLabelVector->at(0)->clone();
			m_f32LastFontSize = m_oLastAddedLabel->getLabelScaleSize();
			updateResultBuffer();
			this->notifyObservers(l_sTextLabel.c_str());
		}
		delete l_pLabelVector;
	}
	//if the notification comes from the UNDO handler (to undo or redo an action)
	else if (l_pUndoHandler!=NULL)
	{
		std::pair<int32,std::string> l_oUndoData = *static_cast< const std::pair<int32,std::string>* >(pUserData);
		int32 l_i32UndoSize = l_oUndoData.first;

		//if we redo something
		if (l_i32UndoSize<0)
		{
			std::string l_sStringToRedo = l_oUndoData.second;

			//if we redo a backspace
			if (l_sStringToRedo.find("<")!=std::string::npos)
			{
				eraseLastCharacter();
			}
			else //
			{
				for (uint32 i=0; i<l_sStringToRedo.length(); i++)
				{
					GSymbol* l_oSymbol = dynamic_cast<GSymbol*>(m_oLastAddedLabel->clone());
					l_oSymbol->setTextLabel(l_sStringToRedo.substr(i,1).c_str());
					l_oSymbol->setChanged(true);
					m_pSymbolContainer->addChild(l_oSymbol, l_oSymbol->getDepth());
				}
			}
		}
		//if we undo a backspace
		else if (l_i32UndoSize==0)
		{
			std::string l_sCharToRestore = l_oUndoData.second;
			m_sSpelledLetters+=l_sCharToRestore;
			GSymbol* l_oSymbol = dynamic_cast<GSymbol*>(m_oLastAddedLabel);
			l_oSymbol->setTextLabel(l_sCharToRestore.c_str());
			l_oSymbol->setChanged(true);
			m_pSymbolContainer->addChild(l_oSymbol, l_oSymbol->getDepth());
		}
		else //we undo something that is not a backspace
		{
			if (!m_sSpelledLetters.empty())
			{
				int32 l_i32Diff = m_sSpelledLetters.length()-l_i32UndoSize;
				if (l_i32Diff>=0)
					m_sSpelledLetters.erase(l_i32Diff, l_i32UndoSize);
				else
					m_sSpelledLetters.clear();
			}

			l_i32UndoSize = (uint32)l_i32UndoSize<=m_ui32ResultCounter?l_i32UndoSize:m_ui32ResultCounter;
			for (uint32 i=1; i<=(uint32)l_i32UndoSize; i++)
			{
				m_pSymbolContainer->removeChild(m_ui32ResultCounter-i);
			}
		}
		
		updateResultBuffer();
	}

	//if the notification comes from the BACKSPACE handler (to erase a letter)
	else if (l_pBackspaceHandler!=NULL)
	{
		std::string l_sCharacterRemoved = eraseLastCharacter();
		updateResultBuffer();
		//notify the undo handler
		std::string l_sNotification = std::string("<") + l_sCharacterRemoved;
		this->notifyObservers(l_sNotification.c_str());

	}
	//in this case the result area for the targets notifies the result area of the predictions that it has to move left
	else if (l_pTargetHandler!=NULL)
	{
		if (m_pSymbolContainer->getNumberOfChildren()>0)
			moveSymbolsLeft(1);
		updateResultBuffer();
	}	
}

void P300ResultAreaHandler::trimPrefix(std::string& textLabel)
{
	size_t l_iLastIndexOfSpace = m_sSpelledLetters.find_last_of(" ");
	if (l_iLastIndexOfSpace!=std::string::npos)
	{
		try
		{
			std::string l_sPrefix = m_sSpelledLetters.substr(l_iLastIndexOfSpace+1);
			size_t l_iIndexOfPrefix = textLabel.find(l_sPrefix);
			
			//if the prefix, that is already spelled, is found in the predicted word
			if (l_iIndexOfPrefix!=std::string::npos && l_sPrefix.length()!=0)
			{
				l_iIndexOfPrefix += l_sPrefix.length();
				textLabel = textLabel.substr(l_iIndexOfPrefix);
			}
			//if it is not found something weird is going on
		}
		catch (const std::out_of_range&)
		{
			std::cout << "P300ResultAreaHandler:: Last letter was a space so we can add the entire word " << textLabel << " to the result.\n";
		}
	}
	//if no space is found, but we have already spelled some letters
	else if(m_sSpelledLetters.length()>0 && textLabel.length()>=m_sSpelledLetters.length())
	{
		textLabel = textLabel.substr(m_sSpelledLetters.length());
	}
	//in all other cases we add the entire word
	m_sSpelledLetters += textLabel;
}

void P300ResultAreaHandler::moveSymbolsLeft(OpenViBE::uint32 nshift)
{
	nshift = m_pSymbolContainer->getNumberOfChildren()+nshift-m_pSymbolContainer->getColumnDimension();
	//copy dimensions/position parameters from the labels that will be removed to the ones that will replace them
	for(int i=m_pSymbolContainer->getNumberOfChildren()-1-nshift;i>=0;i--)
	{
		m_pSymbolContainer->getChild(0, i+nshift)->setDimParameters(m_pSymbolContainer->getChild(0, i)->getDimParameters());	
	}
	
	//remove the labels at the beginning of the result bar
	for(uint32 i=0;i<nshift;i++)
		m_pSymbolContainer->removeChild(0);	

	m_ui32ResultCounter -= nshift;
}

void P300ResultAreaHandler::updateResultBuffer()
{
		//updates the result buffer
	m_sSpelledLetters.clear();
	m_ui32ResultCounter = 0;
	for(uint32 i=0;i<m_pSymbolContainer->getNumberOfChildren();i++)
	{
		//std::cout << "move left, number of children " << m_pSymbolContainer->getNumberOfChildren() << ", moving symbol " << i<< "\n";
		GSymbol* l_pSymbol = dynamic_cast<GSymbol*>(m_pSymbolContainer->getChild(0, i));
		//std::cout << "updateResultBuffer:: X of child " << i << " " << m_pSymbolContainer->getChild(0, i)->getX() << "\n";
		if (l_pSymbol!=NULL)
		{
			m_sSpelledLetters += l_pSymbol->getTextLabel();
			m_ui32ResultCounter++;
		}
	}
	std::cout << "P300ResultAreaHandler:: Spelled letters #" << m_sSpelledLetters << "#" << m_ui32ResultCounter << "\n";
	//save state of what has been spelled
	m_vStates.push_back(m_sSpelledLetters);

	#ifdef OUTPUT_TIMING
	fprintf(stackFile, "%s\n",m_sSpelledLetters.c_str());
	#endif
}

std::string P300ResultAreaHandler::eraseLastCharacter()
{
	std::string l_sCharacterRemoved = "";
	if (!m_sSpelledLetters.empty())
	{
		//erase last character
		l_sCharacterRemoved = m_sSpelledLetters.substr(m_sSpelledLetters.length()-1,1);
		std::cout <<"removing " << l_sCharacterRemoved << "\n";
		m_sSpelledLetters.erase(m_sSpelledLetters.length()-1);
	}
	m_pSymbolContainer->removeChild(m_ui32ResultCounter-1);
	return l_sCharacterRemoved;
}
#endif

#endif
