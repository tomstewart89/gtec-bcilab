#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexP300CSVReader.h"

#include <locale>
#include <fstream>
#include <sstream>
using namespace OpenViBEApplications;
using namespace std;

struct csvReader: ctype<char> {
	csvReader(): ctype<char>(get_table()) {}
	static ctype_base::mask const* get_table() {
		static vector<ctype_base::mask> rc(table_size, ctype_base::mask());

		rc[' '] = ctype_base::space;
		rc[';'] = ctype_base::space;
	  rc[','] = ctype_base::space;
	  rc['\t'] = ctype_base::space;
		rc['\n'] = ctype_base::space;
		return &rc[0];
	}
};

ovexP300CSVReader::ovexP300CSVReader(OpenViBE::uint32 numberOfSymbols, OpenViBE::uint32 numberOfGroups, OpenViBE::uint32 nrOfRepetitions):	P300SequenceGenerator(numberOfSymbols,numberOfGroups,nrOfRepetitions)
{
	std::cout << "creating CSV reader\n";
	m_uiFlashIndex = 0;
	m_bFileRead = false;
	m_uiTrialIndex = 0;


}

void ovexP300CSVReader::readFile()
{
	std::string sFileName = std::string(m_wSequenceWriter->getFilename().toASCIIString());
	std::ifstream m_sStream(sFileName.c_str());
	std::string line;


	if (m_sStream.is_open())
	{
		std::cout << "file openend\n";
		while ( getline (m_sStream,line) )
		{
			if(line.size()/2==m_ui32NumberOfSymbols)
			{
				std::vector<unsigned int>* currentGroup = new std::vector<unsigned int>();
				//std::stringstream iss(line);

				unsigned int value;

				std::istringstream l_LineBuffer(line);
				l_LineBuffer.imbue(std::locale(std::locale(), new csvReader()));

				while(l_LineBuffer >> value)
				{
					//std::cout << value << '\n';
					currentGroup->push_back(value);
				}
				m_lFlashes.push_back(currentGroup);
				m_uiFlashIndex++;
			}
			else
			{
				std::cout << "invalid line size " << line.size() << " instead of " << m_ui32NumberOfSymbols << '\n';
			}

		}
		m_sStream.close();
		std::cout << "read " << m_lFlashes.size() <<" lines\n";
	}
	else
		std::cout << "Unable to open file";

	m_uiFlashIndex = 0;

}

std::vector< std::vector<OpenViBE::uint32>* >* ovexP300CSVReader::generateSequence()
{
	m_uiFlashIndex+=m_ui32FlashCount;
	P300SequenceGenerator::generateSequence();
	if(!m_bFileRead)
	{
		std::cout << "\n			READING FILE\n";
		readFile();
		m_bFileRead = true;
	}
	for(unsigned int i=m_uiFlashIndex; i<m_lFlashes.size(); i++)
	{
		m_lSequence->push_back(m_lFlashes.at(i));
	}


	//m_lSequence->clear();


	return NULL;
}

ovexP300CSVReader::~ovexP300CSVReader()
{

}
/*
std::vector<unsigned int> ovexP300CSVReader::getNextFlashGroup()
{
}
//*/

#endif
