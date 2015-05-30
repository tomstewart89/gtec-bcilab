#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef OVEXP300CSVREADER_H
#define OVEXP300CSVREADER_H

#include <string>
#include <vector>
#include <iostream>

#include <cstdio>
#include <cstdlib>

#include "ovexP300SequenceGenerator.h"

namespace OpenViBEApplications
{

class ovexP300CSVReader :  public P300SequenceGenerator
{
public:
	ovexP300CSVReader(OpenViBE::uint32 numberOfSymbols, OpenViBE::uint32 numberOfGroups, OpenViBE::uint32 nrOfRepetitions);

	~ovexP300CSVReader();

	//std::vector<unsigned int> getNextFlashGroup();

	virtual void changeNumberOfSymbols(OpenViBE::uint32 numberOfSymbols) {m_ui32NumberOfSymbols = numberOfSymbols;}
	virtual void changeNumberOfGroups(OpenViBE::uint32 numberOfGroups) {m_ui32NumberOfGroups = numberOfGroups;}

	virtual std::vector< std::vector<OpenViBE::uint32>* >* generateSequence();

private :
	void readFile();

	unsigned int m_uiFlashIndex;
	std::vector< std::vector <unsigned int>* > m_lFlashes;
	bool m_bFileRead;
	unsigned int m_uiTrialIndex;
};
}
#endif // OVEXP300CSVREADER_H

#endif
