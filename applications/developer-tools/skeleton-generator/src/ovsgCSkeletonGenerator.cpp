#include "ovsgCSkeletonGenerator.h"

#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>

#include <boost/regex.hpp>

using namespace std;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBESkeletonGenerator;

// Wraps system() call to handle spaces on Windows
int systemWrapper(const CString& command)
{
#if defined(TARGET_OS_Windows)
	// If both command and its arguments have spaces on Windows, the whole thing needs to be padded with double quotes
	CString l_sPadded = "\"" + command + "\"";
	return system(l_sPadded.toASCIIString());
#else
	// On Linux we leave it as-is.
	return system(command.toASCIIString());
#endif
}

CSkeletonGenerator::CSkeletonGenerator(IKernelContext & rKernelContext, ::GtkBuilder * pBuilderInterface)
	:m_rKernelContext(rKernelContext)
	,m_pBuilderInterface(pBuilderInterface)
	,m_bConfigurationFileLoaded(false)
{
	m_sConfigurationFile = m_rKernelContext.getConfigurationManager().expand("${CustomConfigurationApplication}");
	m_rKernelContext.getLogManager() << LogLevel_Trace << "Configuration file [" << m_sConfigurationFile << "]\n";
	loadCommonParameters(m_sConfigurationFile);
}

CSkeletonGenerator::~CSkeletonGenerator(void)
{
}

void CSkeletonGenerator::getCommonParameters()
{
	//Author and Company
	::GtkWidget* l_pEntryCompany = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "entry_company_name"));
	m_sCompany = gtk_entry_get_text(GTK_ENTRY(l_pEntryCompany));

	::GtkWidget* l_pEntryAuthor = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "entry_author_name"));
	m_sAuthor=gtk_entry_get_text(GTK_ENTRY(l_pEntryAuthor));

}

boolean CSkeletonGenerator::saveCommonParameters(CString sFileName)
{
	// we get the latest values
	getCommonParameters();

	FILE* l_pFile=::fopen(sFileName.toASCIIString(), "ab");
	if(!l_pFile)
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Saving the common entries in [" << sFileName << "] failed !\n";
		return false;
	}

	// generator selected
	CString l_sActive;
	::GtkWidget* l_pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "sg-driver-selection-radio-button"));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pWidget)))
	{
		l_sActive = "0";
	}
	l_pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "sg-algo-selection-radio-button"));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pWidget)))
	{
		l_sActive = "1";
	}
	l_pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "sg-box-selection-radio-button"));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pWidget)))
	{
		l_sActive = "2";
	}

	::fprintf(l_pFile, "SkeletonGenerator_GeneratorSelected = %s\n", l_sActive.toASCIIString());
	::fprintf(l_pFile, "SkeletonGenerator_Common_Author = %s\n", m_sAuthor.toASCIIString());
	::fprintf(l_pFile, "SkeletonGenerator_Common_Company = %s\n", m_sCompany.toASCIIString());
	::fclose(l_pFile);
	m_rKernelContext.getLogManager() << LogLevel_Info << "Common entries saved in [" << sFileName << "]\n";

	//we can reload the file, it may have changed
	m_bConfigurationFileLoaded = false;

	return true;
}

boolean CSkeletonGenerator::cleanConfigurationFile(CString sFileName)
{
	FILE* l_pFile=::fopen(sFileName.toASCIIString(), "wb");
	if(!l_pFile)
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Failed to clean [" << sFileName << "]\n";
		return false;
	}

	m_rKernelContext.getLogManager() << LogLevel_Info << "Configuration file [" << sFileName << "] cleaned.\n";
	::fclose(l_pFile);
	return true;
}

boolean CSkeletonGenerator::loadCommonParameters(CString sFileName)
{
	if(!m_bConfigurationFileLoaded && !m_rKernelContext.getConfigurationManager().addConfigurationFromFile(sFileName))
	{
		m_rKernelContext.getLogManager() << LogLevel_Warning << "Common: Configuration file [" << sFileName << "] could not be loaded. \n";
		return false;
	}

	::GtkWidget* l_pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "sg-driver-selection-radio-button"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pWidget), (m_rKernelContext.getConfigurationManager().expandAsUInteger("${SkeletonGenerator_GeneratorSelected}") == 0));
	l_pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "sg-algo-selection-radio-button"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pWidget), (m_rKernelContext.getConfigurationManager().expandAsUInteger("${SkeletonGenerator_GeneratorSelected}") == 1));
	l_pWidget = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "sg-box-selection-radio-button"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pWidget), (m_rKernelContext.getConfigurationManager().expandAsUInteger("${SkeletonGenerator_GeneratorSelected}") == 2));
	
	::GtkWidget* l_pEntryCompany = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "entry_company_name"));
	gtk_entry_set_text(GTK_ENTRY(l_pEntryCompany),m_rKernelContext.getConfigurationManager().expand("${SkeletonGenerator_Common_Company}"));

	::GtkWidget * l_pEntryAuthorName = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "entry_author_name"));
	gtk_entry_set_text(GTK_ENTRY(l_pEntryAuthorName),m_rKernelContext.getConfigurationManager().expand("${SkeletonGenerator_Common_Author}"));

	m_rKernelContext.getLogManager() << LogLevel_Info << "Common entries from [" << sFileName << "] loaded.\n";

	m_bConfigurationFileLoaded = true;

	return true;
}

CString CSkeletonGenerator::ensureSedCompliancy(CString sExpression)
{
	string l_sExpression((const char*)sExpression);
	string::iterator it=l_sExpression.begin();
	while(it<l_sExpression.end())
	{
		if((*it)=='\\')
		{
			it = l_sExpression.insert(it,'\\');
			it++;
			it = l_sExpression.insert(it,'\\');
			it++;
			it = l_sExpression.insert(it,'\\');
			it++;
#ifdef TARGET_OS_Linux
			it = l_sExpression.insert(it,'\\');
			it = l_sExpression.insert(it,'\\');
			it+=2;
			it = l_sExpression.insert(it,'\\');
			it = l_sExpression.insert(it,'\\');
			it+=2;
#endif
		}
		else if((*it)=='/')
		{
			it = l_sExpression.insert(it,'\\');
			it++;
		}
		else if((*it)=='"')
		{
			it = l_sExpression.insert(it,'\\');
			it++;
			it = l_sExpression.insert(it,'\\');
			it++;
			it = l_sExpression.insert(it,'\\');
			it++;
			it = l_sExpression.insert(it,'\\');
			it++;
			it = l_sExpression.insert(it,'\\');
			it++;
		}
		else if((*it)=='\n')
		{
			it = l_sExpression.erase(it);
#ifdef TARGET_OS_Linux
			it = l_sExpression.insert(it,'\\');
			it = l_sExpression.insert(it,'\\');
			it+=2;
#endif
			it = l_sExpression.insert(it,'\\');
			it = l_sExpression.insert(it,'\\');
			it+=2;
			it = l_sExpression.insert(it,'n');
			//it++;
		}
		it++;
	}

	return CString(l_sExpression.c_str());
}

boolean CSkeletonGenerator::regexReplace(const CString& sTemplateFile, const CString& sRegEx, const CString& sSubstitute, const CString& sDestinationFile)
{
	try {
		// Read file to memory
		std::ifstream l_oInStream(sTemplateFile.toASCIIString());
		std::string l_sBuffer((std::istreambuf_iterator<char>(l_oInStream)), std::istreambuf_iterator<char>());
		l_oInStream.close();

		// Open output stream and set an iterator to it
		CString l_sRealDestination(sDestinationFile);
		if(l_sRealDestination==CString(""))
		{
			l_sRealDestination = sTemplateFile;
		}
#if 0
		std::cout << "Replace from " << sTemplateFile << " to " << l_sRealDestination << "\n";
		std::cout << "  regEx  : " << sRegEx << "\n";
		std::cout << "  newText: " << sSubstitute << "\n";
#endif

		std::ofstream l_oOutStream(l_sRealDestination.toASCIIString());

		std::ostream_iterator<char> l_oOutIterator(l_oOutStream);
	
		// Do regex magic on the iterator
		boost::regex l_oExpression;
		l_oExpression.assign(sRegEx.toASCIIString());

		boost::regex_replace(l_oOutIterator, l_sBuffer.begin(), l_sBuffer.end(), l_oExpression, sSubstitute.toASCIIString(), boost::match_default | boost::format_sed);

		l_oOutStream.close();
	} 
	catch(...)
	{
		std::cout << "Error occurred processing " << sTemplateFile << " to " << sDestinationFile << "\n";
		return false;
	}
	return true;
}

CString CSkeletonGenerator::getDate()
{
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	stringstream ssTime;
	string string_time(asctime (timeinfo));
	string_time = string_time.substr(0,string_time.size()-1); // the ascitime ends with a "\n"
	CString l_sDate(string_time.c_str());
	
	return l_sDate;
}

boolean CSkeletonGenerator::generate(CString sTemplateFile, CString sDestinationFile, map<CString,CString> mSubstitutions, CString& rLog)
{
	// we check if the template file is in place.
	if(! g_file_test(sTemplateFile, G_FILE_TEST_EXISTS))
	{
		rLog = rLog + "[FAILED] the template file '"+sTemplateFile+"' is missing.\n";
		m_rKernelContext.getLogManager() << LogLevel_Error << "The template file '"<<sTemplateFile<<"' is missing.\n";
		return false;
	}
	
	// we check the map
	if(mSubstitutions.size() == 0)
	{
		rLog = rLog + "[WARNING] No substitution provided.\n";
		m_rKernelContext.getLogManager() << LogLevel_Warning << "No substitution provided.\n";
		return false;
	}

	boolean l_bSuccess = true;

	rLog = rLog +  "[   OK   ] -- template file '"+sTemplateFile+"' found.\n";
	m_rKernelContext.getLogManager() << LogLevel_Info << " -- template file '" << sTemplateFile << "' found.\n";

	//we need to create the destination file by copying the template file, then do the first substitution
	map<CString,CString>::const_iterator it = mSubstitutions.begin();
	l_bSuccess &= regexReplace(sTemplateFile, it->first, ensureSedCompliancy(it->second), sDestinationFile);
	it++;
	
	//next substitutions are done on the - incomplete - destination file itself
	while(it != mSubstitutions.end() && l_bSuccess)
	{
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Executing substitution ["<<it->first<<"] ->["<<it->second<<"]\n";
		l_bSuccess &= regexReplace(sDestinationFile, it->first, ensureSedCompliancy(it->second));
		it++;
	}

	if(! l_bSuccess)
	{
		rLog = rLog + "[FAILED] -- " + sDestinationFile + " cannot be written.\n";
		m_rKernelContext.getLogManager() << LogLevel_Error << " -- " << sDestinationFile << " cannot be written.\n";
		return false;

	}

	rLog = rLog + "[   OK   ] -- " + sDestinationFile + " written.\n";
	m_rKernelContext.getLogManager() << LogLevel_Info << " -- " << sDestinationFile << " written.\n";
	return true;
}
