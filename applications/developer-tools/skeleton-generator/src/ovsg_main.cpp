#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <iostream>

#include "ovsgCDriverSkeletonGenerator.h"
#include "ovsgCBoxAlgorithmSkeletonGenerator.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBESkeletonGenerator;

using namespace std;

int main(int argc, char** argv)
{

	CKernelLoader l_oKernelLoader;

	cout<<"[  INF  ] Created kernel loader, trying to load kernel module"<<endl;
	CString l_sError;
#if defined TARGET_OS_Windows
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/openvibe-kernel.dll";
#else
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/libopenvibe-kernel.so";
#endif
	if(!l_oKernelLoader.load(l_sKernelFile, &l_sError))
	{
		cout<<"[ FAILED ] Error loading kernel ("<<l_sError<<")" << " from [" << l_sKernelFile << "]\n";
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
			l_pKernelContext=l_pKernelDesc->createKernel("skeleton-generator", OpenViBE::Directories::getDataDir() + "/kernel/openvibe.conf");
			if(!l_pKernelContext)
			{
				cout<<"[ FAILED ] No kernel created by kernel descriptor"<<endl;
			}
			else
			{
				OpenViBEToolkit::initialize(*l_pKernelContext);
				IConfigurationManager& l_rConfigurationManager=l_pKernelContext->getConfigurationManager();
				l_pKernelContext->getPluginManager().addPluginsFromFiles(l_rConfigurationManager.expand("${Kernel_Plugins}"));
				
				gtk_init(&argc, &argv);

				::GtkBuilder * l_pBuilderInterface = gtk_builder_new();
				const OpenViBE::CString l_sFilename = OpenViBE::Directories::getDataDir() + "/applications/skeleton-generator/generator-interface.ui";
				if(!gtk_builder_add_from_file(l_pBuilderInterface, l_sFilename, NULL))
				{
					std::cout << "Problem loading [" << l_sFilename << "]\n";
					return -1;
				}

				//gtk_builder_connect_signals(l_pBuilderInterface, NULL);

				::GtkWidget * l_pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "sg-selection-dialog"));

				gtk_dialog_add_button (GTK_DIALOG (l_pDialog),
					GTK_STOCK_OK,
					GTK_RESPONSE_OK);

				gtk_dialog_add_button (GTK_DIALOG (l_pDialog),
					GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL);

				::GtkWidget * l_pRadioDriver = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "sg-driver-selection-radio-button"));
				::GtkWidget * l_pRadioAlgo = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "sg-algo-selection-radio-button"));
				::GtkWidget * l_pRadioBox = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "sg-box-selection-radio-button"));


				CBoxAlgorithmSkeletonGenerator l_BoxGenerator(*l_pKernelContext,l_pBuilderInterface);
				CDriverSkeletonGenerator l_DriverGenerator(*l_pKernelContext,l_pBuilderInterface);

				gint resp = gtk_dialog_run(GTK_DIALOG(l_pDialog));
				
				if(resp== GTK_RESPONSE_OK)
				{
					gtk_widget_hide(GTK_WIDGET(l_pDialog));

					if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pRadioDriver)))
					{
						l_DriverGenerator.initialize();
					}
					if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pRadioAlgo)))
					{
						std::cout<< "NOT YET AVAILABLE." <<std::endl;
						return 0;
					}
					if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pRadioBox)))
					{
						l_BoxGenerator.initialize();
					}
					gtk_main();
				}
				else
				{
					std::cout<< "User cancelled. Exit." <<std::endl;
					return 0 ;
				}
			}
		}
	}

	l_oKernelLoader.uninitialize();
	l_oKernelLoader.unload();
	return 0;
}
