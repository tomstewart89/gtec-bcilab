#include "ovasCAcquisitionServerGUI.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <gtk/gtk.h>

#include <iostream>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

typedef struct _SConfiguration
{
	_SConfiguration(void)
	{
	}

	// <name, value>
	std::map < std::string, std::string > m_oFlag;
	std::map < std::string, std::string > m_oTokenMap;
} SConfiguration;

boolean parse_arguments(int argc, char** argv, SConfiguration& rConfiguration)
{
	SConfiguration l_oConfiguration;

	int i;
	std::vector < std::string > l_vArgValue;
	std::vector < std::string >::const_iterator it;
	for(i=1; i<argc; i++)
	{
		l_vArgValue.push_back(argv[i]);
	}
	l_vArgValue.push_back("");

	for(it=l_vArgValue.begin(); it!=l_vArgValue.end(); it++)
	{
		if(*it=="")
		{
		}
		else if(*it=="-c" || *it=="--config")
		{
			if(*++it=="") { std::cout << "Error: Switch --config needs an argument\n"; return false; }
			l_oConfiguration.m_oFlag["config"] = *it;
		}
		else if(*it=="-d" || *it=="--define")
		{
			if(*++it=="") {
				std::cout << "Error: Need two arguments after -d / --define.\n";
				return false;
			}

			// Were not using = as a separator for token/value, as on Windows its a problem passing = to the cmd interpreter 
			// which is used to launch the actual designer exe.
			const std::string& l_rToken = *it;
			if(*++it=="") {
				std::cout << "Error: Need two arguments after -d / --define.\n";
				return false;
			}

			const std::string& l_rValue = *it;	// iterator will increment later
			
			l_oConfiguration.m_oTokenMap[l_rToken] = l_rValue;

		}
		else if(*it=="-k" || *it=="--kernel")
		{
			if(*++it=="") { std::cout << "Error: Switch --kernel needs an argument\n"; return false; }
			l_oConfiguration.m_oFlag["kernel"] = *it;
		}
		else if(*it=="-h" || *it=="--help")
		{
			return false;
		}
		else
		{
			std::cout << "Error: Unknown argument [" << *it << "]\n";
			return false;
		}
	}

	rConfiguration=l_oConfiguration;

	return true;
}


int main(int argc, char ** argv)
{
//___________________________________________________________________//
//                                                                   //

	SConfiguration l_oConfiguration;
	if(!parse_arguments(argc, argv, l_oConfiguration))
	{
		cout << "Syntax : " << argv[0] << " [ switches ]\n";
		cout << "Possible switches :\n";
		cout << "  --config filename       : path to config file\n";
		cout << "  --define token value    : specify configuration token with a given value\n";
		cout << "  --help                  : displays this help message and exits\n";
		cout << "  --kernel filename       : path to openvibe kernel library\n";
		return -1;
	}

	CKernelLoader l_oKernelLoader;

	cout<<"[  INF  ] Created kernel loader, trying to load kernel module"<<endl;
	CString l_sError;
#if defined TARGET_OS_Windows
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/openvibe-kernel.dll";
#else
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/libopenvibe-kernel.so";
#endif
	if(l_oConfiguration.m_oFlag.count("kernel")) 
	{
		l_sKernelFile = CString(l_oConfiguration.m_oFlag["kernel"].c_str());
	}
	if(!l_oKernelLoader.load(l_sKernelFile, &l_sError))
	{
		cout<<"[ FAILED ] Error loading kernel from [" << l_sKernelFile << "]: " << l_sError << "\n";
	}
	else
	{
		cout<<"[  INF  ] Kernel module loaded, trying to get kernel descriptor"<<endl;
		IKernelDesc* l_pKernelDesc=NULL;
		IKernelContext* l_pKernelContext=NULL;
		l_oKernelLoader.initialize();
		l_oKernelLoader.getKernelDesc(l_pKernelDesc);
		if(!l_pKernelDesc)
		{
			cout<<"[ FAILED ] No kernel descriptor"<<endl;
		}
		else
		{
			cout<<"[  INF  ] Got kernel descriptor, trying to create kernel"<<endl;


			CString l_sConfigFile = CString(OpenViBE::Directories::getDataDir() + "/kernel/openvibe.conf");
			if(l_oConfiguration.m_oFlag.count("config")) 
			{
				l_sConfigFile = CString(l_oConfiguration.m_oFlag["config"].c_str());
			}

			l_pKernelContext=l_pKernelDesc->createKernel("acquisition-server", l_sConfigFile);
			if(!l_pKernelContext)
			{
				cout<<"[ FAILED ] No kernel created by kernel descriptor"<<endl;
			}
			else
			{
				OpenViBEToolkit::initialize(*l_pKernelContext);

				IConfigurationManager& l_rConfigurationManager=l_pKernelContext->getConfigurationManager();

				l_pKernelContext->getPluginManager().addPluginsFromFiles(l_rConfigurationManager.expand("${Kernel_Plugins}"));

				std::map<std::string, std::string>::const_iterator itr;
				for(itr=l_oConfiguration.m_oTokenMap.begin();
					itr!=l_oConfiguration.m_oTokenMap.end();
					itr++)
				{
					l_pKernelContext->getLogManager() << LogLevel_Trace << "Adding command line configuration token [" << (*itr).first.c_str() << " = " << (*itr).second.c_str() << "]\n";
					l_rConfigurationManager.addOrReplaceConfigurationToken((*itr).first.c_str(), (*itr).second.c_str());
				}

				//initialise Gtk before 3D context
#if !GLIB_CHECK_VERSION(2,32,0)
				// although deprecated in newer GTKs (no more needed after (at least) 2.24.13, deprecated in 2.32), we need to use this on Windows with the older GTK (2.22.1), or acquisition server will crash on startup
				g_thread_init(NULL);
#endif
				gdk_threads_init();
				gtk_init(&argc, &argv);

				// gtk_rc_parse(OpenViBE::Directories::getDataDir() + "/applications/designer/interface.gtkrc");

#ifdef TARGET_OS_Linux
				// Replace the gtk signal handlers with the default ones. As a result, 
				// the following exits on terminating signals won't be graceful, 
				// but its better than not exiting at all (gtk default on Linux apparently)
				signal(SIGHUP, SIG_DFL);
				signal(SIGINT, SIG_DFL);
				signal(SIGQUIT, SIG_DFL);
#endif

#if 0 // This is not needed in the acquisition server
				if(l_rConfigurationManager.expandAsBoolean("${Kernel_3DVisualisationEnabled}"))
				{
					l_pKernelContext->getVisualisationManager().initialize3DContext();
				}
#endif

				{
					// If this is encapsulated by gdk_threads_enter() and gdk_threads_exit(), m_pThread->join() can hang when gtk_main() returns before destructor of app has been called.
					OpenViBEAcquisitionServer::CAcquisitionServerGUI app(*l_pKernelContext);

					try
					{
						gdk_threads_enter();	
						gtk_main();
						gdk_threads_leave();			
					}
					catch(...)
					{
						l_pKernelContext->getLogManager() << LogLevel_Fatal << "Catched top level exception\n";
					}
				}

				cout<<"[  INF  ] Application terminated, releasing allocated objects"<<endl;

				OpenViBEToolkit::uninitialize(*l_pKernelContext);

				l_pKernelDesc->releaseKernel(l_pKernelContext);
			}
		}
		l_oKernelLoader.uninitialize();
		l_oKernelLoader.unload();
	}

	return 0;
}
