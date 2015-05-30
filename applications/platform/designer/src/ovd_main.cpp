#include "ovd_base.h"

#include <system/ovCTime.h>
#include <system/ovCMath.h>

#include <stack>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>

#include "ovdCDesignerVisualisation.h"
#include "ovdCPlayerVisualisation.h"
#include "ovdCInterfacedObject.h"
#include "ovdCInterfacedScenario.h"
#include "ovdCApplication.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEDesigner;
using namespace std;

map<uint32, ::GdkColor> g_vColors;

class CPluginObjectDescEnum
{
public:

	CPluginObjectDescEnum(const IKernelContext& rKernelContext)
		:m_rKernelContext(rKernelContext)
	{
	}

	virtual ~CPluginObjectDescEnum(void)
	{
	}

	virtual boolean enumeratePluginObjectDesc(void)
	{
		CIdentifier l_oIdentifier;
		while((l_oIdentifier=m_rKernelContext.getPluginManager().getNextPluginObjectDescIdentifier(l_oIdentifier))!=OV_UndefinedIdentifier)
		{
			this->callback(*m_rKernelContext.getPluginManager().getPluginObjectDesc(l_oIdentifier));
		}
		return true;
	}

	virtual boolean enumeratePluginObjectDesc(
		const CIdentifier& rParentClassIdentifier)
	{
		CIdentifier l_oIdentifier;
		while((l_oIdentifier=m_rKernelContext.getPluginManager().getNextPluginObjectDescIdentifier(l_oIdentifier, rParentClassIdentifier))!=OV_UndefinedIdentifier)
		{
			this->callback(*m_rKernelContext.getPluginManager().getPluginObjectDesc(l_oIdentifier));
		}
		return true;
	}

	virtual boolean callback(
		const IPluginObjectDesc& rPluginObjectDesc)=0;

protected:

	const IKernelContext& m_rKernelContext;
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

class CPluginObjectDescCollector : public CPluginObjectDescEnum
{
public:

	CPluginObjectDescCollector(const IKernelContext& rKernelContext)
		:CPluginObjectDescEnum(rKernelContext)
	{
	}

	virtual boolean callback(
		const IPluginObjectDesc& rPluginObjectDesc)
	{
		string l_sFullName=string(rPluginObjectDesc.getCategory())+"/"+string(rPluginObjectDesc.getName());
		map<string, const IPluginObjectDesc* >::iterator itPluginObjectDesc=m_vPluginObjectDesc.find(l_sFullName);
		if(itPluginObjectDesc!=m_vPluginObjectDesc.end())
		{
			m_rKernelContext.getLogManager() << LogLevel_ImportantWarning << "Duplicate plugin object name " << CString(l_sFullName.c_str()) << " " << itPluginObjectDesc->second->getCreatedClass() << " and " << rPluginObjectDesc.getCreatedClass() << "\n";
		}
		m_vPluginObjectDesc[l_sFullName]=&rPluginObjectDesc;
		return true;
	}

	map<string, const IPluginObjectDesc*>& getPluginObjectDescMap(void)
	{
		return m_vPluginObjectDesc;
	}

private:

	map<string, const IPluginObjectDesc*> m_vPluginObjectDesc;
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

class CPluginObjectDescLogger : public CPluginObjectDescEnum
{
public:

	CPluginObjectDescLogger(const IKernelContext& rKernelContext)
		:CPluginObjectDescEnum(rKernelContext)
	{
	}

	virtual boolean callback(
		const IPluginObjectDesc& rPluginObjectDesc)
	{
		// Outputs plugin info to console
		m_rKernelContext.getLogManager() << LogLevel_Trace << "Plugin <" << rPluginObjectDesc.getName() << ">\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Plugin category        : " << rPluginObjectDesc.getCategory() << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Class identifier       : " << rPluginObjectDesc.getCreatedClass() << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Author name            : " << rPluginObjectDesc.getAuthorName() << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Author company name    : " << rPluginObjectDesc.getAuthorCompanyName() << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Short description      : " << rPluginObjectDesc.getShortDescription() << "\n";
		m_rKernelContext.getLogManager() << LogLevel_Debug << " | Detailed description   : " << rPluginObjectDesc.getDetailedDescription() << "\n";

		return true;
	}
};

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

static void insertPluginObjectDesc_to_GtkTreeStore(const IKernelContext& rKernelContext, map<string, const IPluginObjectDesc*>& vPluginObjectDesc, ::GtkTreeStore* pTreeStore)
{
	map<string, const IPluginObjectDesc*>::iterator itPluginObjectDesc;
	for(itPluginObjectDesc=vPluginObjectDesc.begin(); itPluginObjectDesc!=vPluginObjectDesc.end(); itPluginObjectDesc++)
	{
		const IPluginObjectDesc* l_pPluginObjectDesc=itPluginObjectDesc->second;

		CString l_sStockItemName;

		const IBoxAlgorithmDesc* l_pBoxAlgorithmDesc=dynamic_cast<const IBoxAlgorithmDesc*>(l_pPluginObjectDesc);
		if(l_pBoxAlgorithmDesc)
		{
			l_sStockItemName=l_pBoxAlgorithmDesc->getStockItemName();
		}

		boolean l_bShouldShow=true;

		if  (rKernelContext.getPluginManager().isPluginObjectFlaggedAsDeprecated(l_pPluginObjectDesc->getCreatedClass())
		 && !rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowDeprecated}", false))
		{
			l_bShouldShow=false;
		}

		/*
		if  (rKernelContext.getPluginManager().isPluginObjectFlaggedAsUnstable(l_pPluginObjectDesc->getCreatedClass())
		 && !rKernelContext.getConfigurationManager().expandAsBoolean("${Designer_ShowUnstable}", false))
		{
			l_bShouldShow=false;
		}
		*/

		if(l_bShouldShow)
		{
			::GtkStockItem l_oStockItem;
			if(!gtk_stock_lookup(l_sStockItemName, &l_oStockItem))
			{
				l_sStockItemName=GTK_STOCK_NEW;
			}

			// Splits the plugin category
			vector<string> l_vCategory;
			string l_sCategory=string(l_pPluginObjectDesc->getCategory());
			size_t j, i=(size_t)-1;
			while((j=l_sCategory.find('/', i+1))!=string::npos)
			{
				string l_sSubCategory=string(l_sCategory, i+1, j-i-1);
				if(l_sSubCategory!=string(""))
				{
					l_vCategory.push_back(l_sSubCategory);
				}
				i=j;
			}
			if(i+1!=l_sCategory.length())
			{
				l_vCategory.push_back(string(l_sCategory, i+1, l_sCategory.length()-i-1));
			}

			// Fills plugin in the tree
			vector<string>::iterator it;
			::GtkTreeIter l_oGtkIter1;
			::GtkTreeIter l_oGtkIter2;
			::GtkTreeIter* l_pGtkIterParent=NULL;
			::GtkTreeIter* l_pGtkIterChild=&l_oGtkIter1;
			for(it=l_vCategory.begin(); it!=l_vCategory.end(); it++)
			{
				boolean l_bFound=false;
				boolean l_bValid=gtk_tree_model_iter_children(
					GTK_TREE_MODEL(pTreeStore),
					l_pGtkIterChild,
					l_pGtkIterParent)?true:false;
				while(l_bValid && !l_bFound)
				{
					gchar* l_sName=NULL;
					gboolean l_bIsPlugin;
					gtk_tree_model_get(
						GTK_TREE_MODEL(pTreeStore),
						l_pGtkIterChild,
						Resource_StringName, &l_sName,
						Resource_BooleanIsPlugin, &l_bIsPlugin,
						-1);
					if(!l_bIsPlugin && l_sName==*it)
					{
						l_bFound=true;
					}
					else
					{
						l_bValid=gtk_tree_model_iter_next(
							GTK_TREE_MODEL(pTreeStore),
							l_pGtkIterChild)?true:false;
					}
				}
				if(!l_bFound)
				{
					gtk_tree_store_append(
						GTK_TREE_STORE(pTreeStore),
						l_pGtkIterChild,
						l_pGtkIterParent);
					gtk_tree_store_set(
						GTK_TREE_STORE(pTreeStore),
						l_pGtkIterChild,
						Resource_StringName, it->c_str(),
						Resource_StringShortDescription, "",
						Resource_StringStockIcon, "gtk-directory",
						Resource_StringColor, "#000000",
						Resource_BooleanIsPlugin, (gboolean)FALSE,
						-1);
				}
				if(!l_pGtkIterParent)
				{
					l_pGtkIterParent=&l_oGtkIter2;
				}
				::GtkTreeIter* l_pGtkIterSwap=l_pGtkIterChild;
				l_pGtkIterChild=l_pGtkIterParent;
				l_pGtkIterParent=l_pGtkIterSwap;
			}
			gtk_tree_store_append(
				GTK_TREE_STORE(pTreeStore),
				l_pGtkIterChild,
				l_pGtkIterParent);
			gtk_tree_store_set(
				GTK_TREE_STORE(pTreeStore),
				l_pGtkIterChild,
				Resource_StringName, (const char*)l_pPluginObjectDesc->getName(),
				Resource_StringShortDescription, (const char*)l_pPluginObjectDesc->getShortDescription(),
				Resource_StringIdentifier, (const char*)l_pPluginObjectDesc->getCreatedClass().toString(),
				Resource_StringStockIcon, (const char*)l_sStockItemName,
				Resource_StringColor,
					rKernelContext.getPluginManager().isPluginObjectFlaggedAsDeprecated(l_pPluginObjectDesc->getCreatedClass())?"#3f7f7f":
					rKernelContext.getPluginManager().isPluginObjectFlaggedAsUnstable(l_pPluginObjectDesc->getCreatedClass())?"#6f6f6f":
					"black",
				Resource_BooleanIsPlugin, (gboolean)TRUE,
				Resource_BooleanIsUnstable, rKernelContext.getPluginManager().isPluginObjectFlaggedAsUnstable(l_pPluginObjectDesc->getCreatedClass()),
				-1);
		};
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

typedef struct _SConfiguration
{
	_SConfiguration(void)
		:m_eNoGui(CommandLineFlag_None)
		,m_eNoCheckColorDepth(CommandLineFlag_None)
		,m_eNoManageSession(CommandLineFlag_None)
	{
	}

	OpenViBEDesigner::ECommandLineFlag getFlags(void)
	{
		return OpenViBEDesigner::ECommandLineFlag(m_eNoGui|m_eNoCheckColorDepth|m_eNoManageSession);
	}

	std::map < ECommandLineFlag, std::string > m_oFlag;
	OpenViBEDesigner::ECommandLineFlag m_eNoGui;
	OpenViBEDesigner::ECommandLineFlag m_eNoCheckColorDepth;
	OpenViBEDesigner::ECommandLineFlag m_eNoManageSession;
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
			l_oConfiguration.m_oFlag[CommandLineFlag_Config] = *it;
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
		else if(*it=="-h" || *it=="--help")
		{
			return false;
		}
		else if(*it=="-k" || *it=="--kernel")
		{
			if(*++it=="") { std::cout << "Error: Switch --kernel needs an argument\n"; return false; }
			l_oConfiguration.m_oFlag[CommandLineFlag_Kernel] = *it;
		}
		else if(*it=="-o" || *it=="--open")
		{
			if(*++it=="") { std::cout << "Error: Switch --open needs an argument\n"; return false; }
			l_oConfiguration.m_oFlag[CommandLineFlag_Open] = *it;
		}
		else if(*it=="-p" || *it=="--play")
		{
			if(*++it=="") { std::cout << "Error: Switch --play needs an argument\n"; return false; }
			l_oConfiguration.m_oFlag[CommandLineFlag_Play] = *it;
		}
		else if(*it=="-pf" || *it=="--play-fast")
		{
			if(*++it=="") { std::cout << "Error: Switch --play-fast needs an argument\n"; return false; }
			l_oConfiguration.m_oFlag[CommandLineFlag_PlayFast] = *it;
		}
		else if(*it=="--no-gui")
		{
			l_oConfiguration.m_eNoGui=CommandLineFlag_NoGui;
			l_oConfiguration.m_eNoCheckColorDepth=CommandLineFlag_NoCheckColorDepth;
			l_oConfiguration.m_eNoManageSession=CommandLineFlag_NoManageSession;
		}
		else if(*it=="--no-check-color-depth")
		{
			l_oConfiguration.m_eNoCheckColorDepth=CommandLineFlag_NoCheckColorDepth;
		}
		else if(*it=="--no-session-management")
		{
			l_oConfiguration.m_eNoManageSession=CommandLineFlag_NoManageSession;
		}
		else if(*it=="--random-seed")
		{
			if(*++it=="") { std::cout << "Error: Switch --random-seed needs an argument\n"; return false; }
			l_oConfiguration.m_oFlag[CommandLineFlag_RandomSeed] = *it;
		}
		else
		{
			std::cout << "Error: Unknown argument [" << *it << "]\n";
			return false;
		}
	}

#if 0
	rConfiguration.m_oFlag=l_oConfiguration.m_oFlag;
	rConfiguration.m_bCheckColorDepth=l_oConfiguration.m_bCheckColorDepth;
	rConfiguration.m_bShowGui=l_oConfiguration.m_bShowGui;
#else
	rConfiguration=l_oConfiguration;
#endif
	return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

int go(int argc, char ** argv)
{
	boolean errorWhileLoadingScenario = false;
	/*
		{ 0,     0,     0,     0 },
		{ 0, 16383, 16383, 16383 },
		{ 0, 32767, 32767, 32767 },
		{ 0, 49151, 49151, 49151 },
		{ 0, 65535, 65535, 65535 },
	*/

	SConfiguration l_oConfiguration;
	if(!parse_arguments(argc, argv, l_oConfiguration))
	{
		cout << "Syntax : " << argv[0] << " [ switches ]\n";
		cout << "Possible switches :\n";
		cout << "  --config filename       : path to config file\n";
		cout << "  --define token value    : specify configuration token with a given value\n";
		cout << "  --help                  : displays this help message and exits\n";
		cout << "  --kernel filename       : path to openvibe kernel library\n";
		cout << "  --no-gui                : hides the designer graphical user interface (assumes --no-check-color-depth)\n";
		cout << "  --no-check-color-depth  : does not check 24/32 bits color depth\n";
		cout << "  --no-session-management : neither restore last used scenarios nor saves them at exit\n";
		cout << "  --open filename         : opens a scenario (see also --no-session-management)\n";
		cout << "  --play filename         : plays the opened scenario (see also --no-session-management)\n";
		cout << "  --play-fast filename    : plays fast forward the opened scenario (see also --no-session-management)\n";
		cout << "  --random-seed uint      : initialize random number generator with value, default=time(NULL)\n";
//					l_rLogManager << LogLevel_Info << "  --define                : defines a variable in the configuration manager\n";
		return -1;
	}

	#define gdk_color_set(c, r, g, b) { c.pixel=0; c.red=r; c.green=g; c.blue=b; }
	gdk_color_set(g_vColors[Color_BackgroundPlayerStarted],  32767, 32767, 32767);
	gdk_color_set(g_vColors[Color_BoxBackgroundSelected],    65535, 65535, 49151);
	gdk_color_set(g_vColors[Color_BoxBackgroundMissing],     49151, 32767, 32767);
	gdk_color_set(g_vColors[Color_BoxBackgroundDeprecated],  24575, 32767, 32767);
	gdk_color_set(g_vColors[Color_BoxBackgroundNeedsUpdate], 57343, 57343, 57343);
	gdk_color_set(g_vColors[Color_BoxBackgroundUnstable],    49151, 49151, 49151);
	gdk_color_set(g_vColors[Color_BoxBackgroundMuted],       65535, 57343, 57343);
	gdk_color_set(g_vColors[Color_BoxBackground],            65535, 65535, 65535);
	gdk_color_set(g_vColors[Color_BoxBorderSelected],            0,     0,     0);
	gdk_color_set(g_vColors[Color_BoxBorder],                    0,     0,     0);
	gdk_color_set(g_vColors[Color_BoxInputBackground],       65535, 49151, 32767);
	gdk_color_set(g_vColors[Color_BoxInputBorder],           16383, 16383, 16383);
	gdk_color_set(g_vColors[Color_BoxOutputBackground],      32767, 65535, 49151);
	gdk_color_set(g_vColors[Color_BoxOutputBorder],          16383, 16383, 16383);
	gdk_color_set(g_vColors[Color_BoxSettingBackground],     49151, 32767, 65535);
	gdk_color_set(g_vColors[Color_BoxSettingBorder],         16383, 16383, 16383);

	gdk_color_set(g_vColors[Color_CommentBackground],        65535, 65535, 57343);
	gdk_color_set(g_vColors[Color_CommentBackgroundSelected],65535, 65535, 49151);
	gdk_color_set(g_vColors[Color_CommentBorder],            32767, 32767, 32767);
	gdk_color_set(g_vColors[Color_CommentBorderSelected],    32767, 32767, 32767);

	gdk_color_set(g_vColors[Color_Link],                         0,     0,     0);
	gdk_color_set(g_vColors[Color_LinkSelected],             49151, 49151, 16383);
	gdk_color_set(g_vColors[Color_LinkUpCast],               32767, 16383, 16383);
	gdk_color_set(g_vColors[Color_LinkDownCast],             16383, 32767, 16383);
	gdk_color_set(g_vColors[Color_LinkInvalid],              49151, 16383, 16383);
	gdk_color_set(g_vColors[Color_SelectionArea],           0x3f00,0x3f00,0x3f00);
	gdk_color_set(g_vColors[Color_SelectionAreaBorder],          0,     0,     0);
	#undef gdk_color_set

//___________________________________________________________________//
//                                                                   //

	CKernelLoader l_oKernelLoader;


	cout<<"[  INF  ] Created kernel loader, trying to load kernel module"<<"\n";
	CString l_sError;
#if defined TARGET_OS_Windows
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/openvibe-kernel.dll";
#else
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/libopenvibe-kernel.so";
#endif
	if(l_oConfiguration.m_oFlag.count(CommandLineFlag_Kernel)) 
	{
		l_sKernelFile = CString(l_oConfiguration.m_oFlag[CommandLineFlag_Kernel].c_str());
	}
	if(!l_oKernelLoader.load(l_sKernelFile, &l_sError))
	{
		cout<<"[ FAILED ] Error loading kernel from [" << l_sKernelFile << "]: " << l_sError << "\n";
	}
	else
	{
		cout<<"[  INF  ] Kernel module loaded, trying to get kernel descriptor"<<"\n";
		IKernelDesc* l_pKernelDesc=NULL;
		IKernelContext* l_pKernelContext=NULL;
		l_oKernelLoader.initialize();
		l_oKernelLoader.getKernelDesc(l_pKernelDesc);
		if(!l_pKernelDesc)
		{
			cout<<"[ FAILED ] No kernel descriptor"<<"\n";
		}
		else
		{
			cout<<"[  INF  ] Got kernel descriptor, trying to create kernel"<<"\n";

			CString l_sConfigFile = CString(OpenViBE::Directories::getDataDir() + "/kernel/openvibe.conf");
			if(l_oConfiguration.m_oFlag.count(CommandLineFlag_Config)) 
			{
				l_sConfigFile = CString(l_oConfiguration.m_oFlag[CommandLineFlag_Config].c_str());
			}

			l_pKernelContext=l_pKernelDesc->createKernel("designer", l_sConfigFile);
			if(!l_pKernelContext)
			{
				cout<<"[ FAILED ] No kernel created by kernel descriptor"<<"\n";
			}
			else
			{
				OpenViBEToolkit::initialize(*l_pKernelContext);

				IConfigurationManager& l_rConfigurationManager=l_pKernelContext->getConfigurationManager();
				ILogManager& l_rLogManager=l_pKernelContext->getLogManager();

				l_pKernelContext->getPluginManager().addPluginsFromFiles(l_rConfigurationManager.expand("${Kernel_Plugins}"));

				std::map<std::string, std::string>::const_iterator itr;
				for(itr=l_oConfiguration.m_oTokenMap.begin();
					itr!=l_oConfiguration.m_oTokenMap.end();
					itr++)
				{
					l_rLogManager << LogLevel_Trace << "Adding command line configuration token [" << (*itr).first.c_str() << " = " << (*itr).second.c_str() << "]\n";
					l_rConfigurationManager.addOrReplaceConfigurationToken((*itr).first.c_str(), (*itr).second.c_str());
				}

				//FIXME : set locale only when needed
				CString l_sLocale = l_rConfigurationManager.expand("${Designer_Locale}");
				if(l_sLocale == CString(""))
				{
					l_sLocale = "C";
				}
				setlocale( LC_ALL, l_sLocale.toASCIIString() );

				//initialise Gtk before 3D context
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
				if(l_rConfigurationManager.expandAsBoolean("${Kernel_3DVisualisationEnabled}"))
				{
					l_pKernelContext->getVisualisationManager().initialize3DContext();
				}

				if(l_oConfiguration.m_oFlag.count(CommandLineFlag_RandomSeed)) 
				{
					System::Math::initializeRandomMachine(atol(l_oConfiguration.m_oFlag[CommandLineFlag_RandomSeed].c_str()));
				} 
				else
				{
					System::Math::initializeRandomMachine(time(NULL));
				}

				{
					::CApplication app(*l_pKernelContext);
					app.initialize(l_oConfiguration.getFlags());
					// FIXME is it necessary to keep next line uncomment ?
					//boolean l_bIsScreenValid=true;
					if(!l_oConfiguration.m_eNoCheckColorDepth)
					{
						if(GDK_IS_DRAWABLE(GTK_WIDGET(app.m_pMainWindow)->window))
						{
							// FIXME is it necessary to keep next line uncomment ?
							//l_bIsScreenValid=false;
							switch(gdk_drawable_get_depth(GTK_WIDGET(app.m_pMainWindow)->window))
							{
								case 24:
								case 32:
									// FIXME is it necessary to keep next line uncomment ?
									//l_bIsScreenValid=true;
									break;
								default:
									l_rLogManager << LogLevel_Error << "Please change the color depth of your screen to either 24 or 32 bits\n";
									// TODO find a way to break
									break;
							}
						}
					}
					std::map < ECommandLineFlag, std::string >::iterator it;
					for(it=l_oConfiguration.m_oFlag.begin(); it!=l_oConfiguration.m_oFlag.end(); it++)
					{

						switch(it->first)
						{
							case CommandLineFlag_Open:
								l_rLogManager << LogLevel_Info << "Opening scenario [" << CString(it->second.c_str()) << "]\n";
								app.openScenario(it->second.c_str());
								break;
							case CommandLineFlag_Play:
								l_rLogManager << LogLevel_Info << "Opening and playing scenario [" << CString(it->second.c_str()) << "]\n";
								if(app.openScenario(it->second.c_str()))
								{
									if(!app.playScenarioCB())
									{
										errorWhileLoadingScenario = true;
									}
								}
								break;
							case CommandLineFlag_PlayFast:
								l_rLogManager << LogLevel_Info << "Opening and fast playing scenario [" << CString(it->second.c_str()) << "]\n";
								if(app.openScenario(it->second.c_str()))
								{
									if(!app.forwardScenarioCB())
									{
										errorWhileLoadingScenario = true;
									}
								}
								break;
//								case CommandLineFlag_Define:
//									break;
							default:
								break;
						}
					}
					if(app.m_vInterfacedScenario.empty())
					{
						app.newScenarioCB();
					}

					{
						CPluginObjectDescCollector cb_collector1(*l_pKernelContext);
						CPluginObjectDescCollector cb_collector2(*l_pKernelContext);
						CPluginObjectDescLogger cb_logger(*l_pKernelContext);
						cb_logger.enumeratePluginObjectDesc();
						cb_collector1.enumeratePluginObjectDesc(OV_ClassId_Plugins_BoxAlgorithmDesc);
						cb_collector2.enumeratePluginObjectDesc(OV_ClassId_Plugins_AlgorithmDesc);
						insertPluginObjectDesc_to_GtkTreeStore(*l_pKernelContext, cb_collector1.getPluginObjectDescMap(), app.m_pBoxAlgorithmTreeModel);
						insertPluginObjectDesc_to_GtkTreeStore(*l_pKernelContext, cb_collector2.getPluginObjectDescMap(), app.m_pAlgorithmTreeModel);
	
						l_pKernelContext->getLogManager() << LogLevel_Info << "Initialization took " << l_pKernelContext->getConfigurationManager().expand("$Core{real-time}") << " ms\n";
	
						try
						{
							gtk_main();
						}
						catch(...)
						{
							l_pKernelContext->getLogManager() << LogLevel_Fatal << "Catched top level exception\n";
						}
					}
				}

				l_rLogManager << LogLevel_Info << "Application terminated, releasing allocated objects\n";

				OpenViBEToolkit::uninitialize(*l_pKernelContext);

				l_pKernelDesc->releaseKernel(l_pKernelContext);
			}
		}
		l_oKernelLoader.uninitialize();
		l_oKernelLoader.unload();
	}
	if(errorWhileLoadingScenario && l_oConfiguration.m_eNoGui == CommandLineFlag_NoGui)
	{
		return -1;
	}

	return 0;
}

int main(int argc, char ** argv)
{
	int l_iRet=-1;
	try
	{
		l_iRet=go(argc, argv);
	}
	catch (...)
	{
		std::cout << "Caught an exception at the very top...\nLeaving application!\n";
	}
	return l_iRet;
}
