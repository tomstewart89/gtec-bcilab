

#if defined(TARGET_HAS_ThirdPartyOgre3D)

#include "ovavrdCOgreVRApplication.h"

#include "HandballBCI/ovavrdCHandballBCI.h"
#include "SpaceshipBCI/ovavrdCSpaceshipBCI.h"

#if 0
#if defined TARGET_OS_Linux
namespace CEGUI
{
	Exception::Exception(const String& s)
	{
		Exception(s, "", "", 0);
	}
}
#endif
#endif

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("Syntax: %s demo-name\n", argv[0]);
		printf("\n");
		printf("where demo-name could be one of the following :\n");
		printf("  - spaceship\n");
		printf("  - handball\n");
		return 1;
	}

	OpenViBEVRDemos::COgreVRApplication * app;
	if(strcmp(argv[1],"spaceship") == 0)
	{
		std::string l_slocalization = "eng";
		if( argc != 3 )
		{
			printf("No language selected. Default is: english (eng).\n\n");
		}
		else
		{
			l_slocalization = argv[2];
			printf("User defined language: l_slocalization \n");
			printf("WARNING: if the language keyword is not found, default language will be loaded (eng).\n\n");
		}
		printf("\n");


		printf(" ___     _.'\\   /'._     ___ \n");
		printf("/  b__--- | .'\"'. | ---__d  \\\n");
		printf("\\  p\"\"---_| '._.' |_---\"\"q  /\n");
		printf("     / /   ./   \\.   \\ \\    \n");
		printf("    / /     '---'     \\ \\   \n");
		printf("   /_/                 \\_\\  \n");
		printf("                             \n\n");
		printf("Application started! Lift 'em!\n\n");
		
		app = new OpenViBEVRDemos::CSpaceshipBCI(l_slocalization);
	}
	else if(strcmp(argv[1],"handball") == 0)
	{
		printf("Handball application started !\n");
		app = new OpenViBEVRDemos::CHandballBCI();
	}
	else
	{
		printf("ERROR: the application specified does not exist (%s).\n",argv[1]);
		printf("Please use one of the following applications:\n");
		printf("  - spaceship\n");
		printf("  - handball\n");
		return 2;
	}

	app->go();
	delete app;

	return 0;
}

#else
#include <stdio.h>

int main(int argc, char** argv)
{
	printf("VR demo has not been compiled as it depends on Ogre (missing/disabled)\n");

	return -1;
}
#endif
