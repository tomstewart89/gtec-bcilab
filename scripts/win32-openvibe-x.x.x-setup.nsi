	SetCompressor /FINAL /SOLID lzma
	SetCompressorDictSize 16

	!include "MUI.nsh"
	!include "zipdll.nsh"

	;Name and file
	Name "OpenViBE 1.0.0"
	OutFile "openvibe-1.0.0-setup.exe"

	;Default installation folder
	InstallDir "$PROGRAMFILES\openvibe"
	Var OLDINSTDIR
	Var DIRECTX_MISSING

;Interface Settings

	!define MUI_ABORTWARNING

;Pages

	!insertmacro MUI_PAGE_WELCOME
	!insertmacro MUI_PAGE_LICENSE "..\COPYING"
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES
	!insertmacro MUI_PAGE_FINISH

	!insertmacro MUI_UNPAGE_WELCOME
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
	!insertmacro MUI_UNPAGE_FINISH

;Languages

	!insertmacro MUI_LANGUAGE "English"

;Installer and uninstaller icons

	Icon "${NSISDIR}\Contrib\Graphics\Icons\box-install.ico"
	UninstallIcon "${NSISDIR}\Contrib\Graphics\Icons\box-uninstall.ico"

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Function .onInit

	UserInfo::GetAccountType
	Pop $R1
	StrCmp $R1 "Admin" has_admin_rights 0
		MessageBox MB_OK "You must be administrator to install OpenViBE" /SD IDOK
		Quit
has_admin_rights:

	ReadRegStr $0 HKLM SOFTWARE\openvibe InstallDir

	${If} $0 != ""
		IfFileExists "$0\Uninstall.exe" +1 +5
			MessageBox MB_YESNO "A previous installation of OpenViBE is installed under $0.$\nContinuing the install procedure will remove previous installation of OpenViBE (including all files you eventually added in the installation directory).$\nWould you like to accept this removal and continue on installation process ?" /SD IDYES IDNO +1 IDYES +2
			Abort
		StrCpy $OLDINSTDIR $0
		StrCpy $INSTDIR $0
	${EndIf}

FunctionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "-OpenViBE"

	${If} $OLDINSTDIR != ""
		RMDir /r $OLDINSTDIR
		RMDir /r "$SMPROGRAMS\OpenViBE"
	${EndIf}

	SetOutPath $INSTDIR
	WriteRegStr HKLM "SOFTWARE\openvibe" "InstallDir" "$INSTDIR"
	WriteUninstaller Uninstall.exe

	CreateDirectory "$INSTDIR\dependencies\arch"
	StrCpy $DIRECTX_MISSING "false"

	SetOutPath "$INSTDIR\dependencies"
	IfFileExists "$SYSDIR\d3dx9_42.dll" no_need_to_install_directx
	NSISdl::download "http://www.microsoft.com/downloads/info.aspx?na=90&p=&SrcDisplayLang=en&SrcCategoryId=&SrcFamilyId=04ac064b-00d1-474e-b7b1-442d8712d553&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2fB%2f7%2f9%2fB79FC9D7-47B8-48B7-A75E-101DEBEB5AB4%2fdirectx_aug2009_redist.exe" "arch\openvibe-directx.exe"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +4
			MessageBox MB_OK "Download failed: $R0$\nCheck your Internet connection and your firewall settings.$\nDirect X won't be installed and 3D functionalities won't be available...$\nYou can install DirectX later to enable 3D functionalities !" /SD IDOK
			StrCpy $DIRECTX_MISSING "true"
			Goto no_need_to_install_directx ; Quit
	ExecWait '"arch\openvibe-directx.exe" /T:"$INSTDIR\tmp" /Q'
	ExecWait '"$INSTDIR\tmp\DXSETUP.exe" /silent'
no_need_to_install_directx:

	SetOutPath "$INSTDIR\dependencies\arch"
	File ..\dependencies\arch\openvibe-vcredist-2010.exe
	File ..\dependencies\arch\lua-5.1.4-vs100.zip
	File ..\dependencies\arch\expat-2.0.1.zip
	File ..\dependencies\arch\glfw-3.0.4-vs100.zip	
	File ..\dependencies\arch\gtk-2.22.1-runtime.zip
	File ..\dependencies\arch\gtk-themes-2009.09.07.zip
	File ..\dependencies\arch\inpout32-vs100.zip	
	File ..\dependencies\arch\itpp-4.0.7-runtime.zip
	File ..\dependencies\arch\ogre-1.7.1-vs100-runtime.zip
	File ..\dependencies\arch\cegui-0.7.2-vs100-runtime.zip
	File ..\dependencies\arch\vrpn-7.31-vs100-runtime.zip
	File ..\dependencies\arch\pthreads-2.8.0-runtime.zip
	File ..\dependencies\arch\presage-0.8.9-vs100.zip		
	File ..\dependencies\arch\openal-1.1-runtime.zip
	File ..\dependencies\arch\freealut-1.1.0-bin-runtime.zip
	File ..\dependencies\arch\libvorbis-1.3.2-vs100-runtime.zip
	File ..\dependencies\arch\libogg-1.2.1-vs100-runtime.zip
	File ..\dependencies\arch\liblsl-1.04-vs100-runtime.zip
	File ..\dependencies\arch\enobio3g-1.2.1-vs100-runtime.zip	
	
	SetOutPath "$INSTDIR\dependencies"
	ExecWait '"arch\openvibe-vcredist-2010.exe" /q'

	SetOutPath "$INSTDIR\dependencies"
	ZipDLL::extractall "arch\lua-5.1.4-vs100.zip" ""
	ZipDLL::extractall "arch\expat-2.0.1.zip" "expat"
	ZipDLL::extractall "arch\glfw-3.0.4-vs100.zip" ""	
	ZipDLL::extractall "arch\gtk-2.22.1-runtime.zip" "gtk"
	ZipDLL::extractall "arch\gtk-themes-2009.09.07.zip" "gtk"
	ZipDLL::extractall "arch\inpout32-vs100.zip" ""	
	ZipDLL::extractall "arch\itpp-4.0.7-runtime.zip" "itpp"
	ZipDLL::extractall "arch\ogre-1.7.1-vs100-runtime.zip" "ogre"
	ZipDLL::extractall "arch\cegui-0.7.2-vs100-runtime.zip" "cegui"
	ZipDLL::extractall "arch\vrpn-7.31-vs100-runtime.zip" ""
	ZipDLL::extractall "arch\pthreads-2.8.0-runtime.zip" "pthreads"
	ZipDLL::extractall "arch\presage-0.8.9-vs100.zip" ""
	ZipDLL::extractall "arch\openal-1.1-runtime.zip" "openal"
	ZipDLL::extractall "arch\freealut-1.1.0-bin-runtime.zip" "freealut"
	ZipDLL::extractall "arch\libvorbis-1.3.2-vs100-runtime.zip" "libvorbis"
	ZipDLL::extractall "arch\libogg-1.2.1-vs100-runtime.zip" "libogg"
	ZipDLL::extractall "arch\liblsl-1.04-vs100-runtime.zip" "liblsl"
	ZipDLL::extractall "arch\enobio3g-1.2.1-vs100-runtime.zip" ""
	
	SetOutPath "$INSTDIR"
	; Export binaries
	File /nonfatal /r ..\dist\bin
	; Export launch scripts
	File /nonfatal ..\dist\*.cmd
	; File /nonfatal /r ..\dist\doc
	; File /nonfatal /r ..\dist\etc
	; File /nonfatal /r ..\dist\include
	; File /nonfatal /r ..\dist\lib
	File /nonfatal /r ..\dist\log
	File /nonfatal /r ..\dist\share
	; File /nonfatal /r ..\dist\tmp

	StrCmp $DIRECTX_MISSING "false" no_need_to_patch_3d_functionnality
	FileOpen $0 "$INSTDIR\share\openvibe\kernel\openvibe.conf" a	
	FileSeek $0 0 END
	FileWrite $0 "$\r$\n"
	FileWrite $0 "#####################################################################################$\r$\n"
	FileWrite $0 "# Patched by installer because DirectX is missing$\r$\n"
	FileWrite $0 "#####################################################################################$\r$\n"
	FileWrite $0 "Kernel_3DVisualisationEnabled = false$\r$\n"
	FileClose $0
no_need_to_patch_3d_functionnality:

	; Overwrite the file that may be in share/, as it contains local definitions to the build machine
	FileOpen $0 "$INSTDIR\bin\openvibe-set-env.cmd" w
	FileWrite $0 "@echo off$\r$\n"
	FileWrite $0 "$\r$\n"

	FileWrite $0 "$\r$\n"
	FileWrite $0 "SET OGRE_HOME=$INSTDIR\dependencies\ogre$\r$\n"
	FileWrite $0 "SET VRPNROOT=$INSTDIR\dependencies\vrpn$\r$\n"
	FileWrite $0 "$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\lua\lib;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\itpp\bin;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\expat\bin;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\glfw\lib\;%PATH%$\r$\n"		
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\gtk\bin;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\inpout32\lib;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\itpp\bin;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\cegui\bin;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=%OGRE_HOME%\bin\release;%OGRE_HOME%\bin\debug;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=%VRPNROOT%\bin;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\pthreads\lib;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\presage\lib;%PATH%$\r$\n"	
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\openal\libs\Win32;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\freealut\lib;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\libvorbis\win32\bin\release;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\libogg\win32\bin\release\;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\liblsl\lib\;%PATH%$\r$\n"
	FileWrite $0 "SET PATH=$INSTDIR\dependencies\enobio3g\MSVC\;%PATH%$\r$\n"	
	FileClose $0
	
	FileOpen $0 "$INSTDIR\dependencies\cegui\resources.cfg" w
	FileWrite $0 "FileSystem=$INSTDIR\dependencies\cegui\datafiles\configs$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\dependencies\cegui\datafiles\fonts$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\dependencies\cegui\datafiles\imagesets$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\dependencies\cegui\datafiles\layouts$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\dependencies\cegui\datafiles\looknfeel$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\dependencies\cegui\datafiles\lua_scripts$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\dependencies\cegui\datafiles\schemes$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\dependencies\cegui\datafiles\xml_schemes$\r$\n"
	FileClose $0

	FileOpen $0 "$INSTDIR\dependencies\gtk\etc\gtk-2.0\gtkrc" w
	FileWrite $0 "gtk-theme-name = $\"Redmond$\"$\r$\n"
	FileWrite $0 "style $\"user-font$\"$\r$\n"
	FileWrite $0 "{$\r$\n"
	FileWrite $0 "	font_name=$\"Sans 8$\"$\r$\n"
	FileWrite $0 "}$\r$\n"
	FileWrite $0 "widget_class $\"*$\" style $\"user-font$\"$\r$\n"
	FileClose $0

	CreateDirectory "$SMPROGRAMS\OpenViBE"
	CreateDirectory "$SMPROGRAMS\OpenViBE\Developer tools"
	CreateShortCut "$SMPROGRAMS\OpenViBE\Developer tools\openvibe id generator.lnk"       "$INSTDIR\openvibe-id-generator.cmd"        "" "%SystemRoot%\system32\shell32.dll" 57
	CreateShortCut "$SMPROGRAMS\OpenViBE\Developer tools\openvibe plugin inspector.lnk"   "$INSTDIR\openvibe-plugin-inspector.cmd"    "" "%SystemRoot%\system32\shell32.dll" 57
	CreateShortCut "$SMPROGRAMS\OpenViBE\Developer tools\openvibe skeleton generator.lnk" "$INSTDIR\openvibe-skeleton-generator.cmd"  "" "%SystemRoot%\system32\shell32.dll" 57
	CreateShortCut "$SMPROGRAMS\OpenViBE\openvibe designer.lnk"                           "$INSTDIR\openvibe-designer.cmd"            "" "%SystemRoot%\system32\shell32.dll" 137
	CreateShortCut "$SMPROGRAMS\OpenViBE\openvibe acquisition server.lnk"                 "$INSTDIR\openvibe-acquisition-server.cmd"  "" "%SystemRoot%\system32\shell32.dll" 18
	CreateShortCut "$SMPROGRAMS\OpenViBE\openvibe vr-demo spaceship.lnk"                  "$INSTDIR\openvibe-vr-demo-spaceship.cmd"   "" "%SystemRoot%\system32\shell32.dll" 200
	CreateShortCut "$SMPROGRAMS\OpenViBE\openvibe vr-demo handball.lnk"                   "$INSTDIR\openvibe-vr-demo-handball.cmd"    "" "%SystemRoot%\system32\shell32.dll" 200
	CreateShortCut "$SMPROGRAMS\OpenViBE\uninstall.lnk"                                   "$INSTDIR\Uninstall.exe"

	
	; AccessControl::EnableFileInheritance "$INSTDIR"
	; AccessControl::GrantOnFile "$INSTDIR" "(BU)" "GenericRead + GenericWrite + GenericExecute + Delete" ; to ensure windows XP back compatibility
	; AccessControl::GrantOnFile "$INSTDIR" "(S-1-5-32-545)" "GenericRead + GenericWrite + GenericExecute + Delete" ; (BU) user group (builtin users) does not exist on win7. this SID replaces it.
SectionEnd

Section "Uninstall"

	RMDir /r $INSTDIR
	RMDir /r "$SMPROGRAMS\OpenViBE"

SectionEnd
