	SetCompressor /FINAL /SOLID lzma
	SetCompressorDictSize 16

	!include "MUI.nsh"
	!include "Sections.nsh"
	!include "zipdll.nsh"

	;Name and file
	Name "OpenViBE dependencies"
	OutFile "win32-install_dependencies.exe"

	;Default installation folder
	InstallDir "$EXEDIR\..\dependencies"

;Interface Settings

	!define MUI_ABORTWARNING
	!define MUI_COMPONENTSPAGE_NODESC

;Pages

	!insertmacro MUI_PAGE_COMPONENTS
	!insertmacro MUI_PAGE_INSTFILES

;  !insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES

;Languages

	!insertmacro MUI_LANGUAGE "English"

;Installer and uninstaller icons

	Icon "${NSISDIR}\Contrib\Graphics\Icons\box-install.ico"
	UninstallIcon "${NSISDIR}\Contrib\Graphics\Icons\box-uninstall.ico"

;VS90/VS100 suffix

	Var suffix

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "-base"

	;Finds Microsoft Platform SDK

	ReadRegStr $r0 HKLM "SOFTWARE\Microsoft\Win32SDK\Directories" "Install Dir"
	StrCmp $r0 "" base_failed_to_find_sdk_1 base_found_sdk
base_failed_to_find_sdk_1:
	ReadRegStr $r0 HKLM "SOFTWARE\Microsoft\MicrosoftSDK\Directories" "Install Dir"
	StrCmp $r0 "" base_failed_to_find_sdk_2 base_found_sdk
base_failed_to_find_sdk_2:
	ReadRegStr $r0 HKLM "SOFTWARE\Microsoft\Microsoft SDKs\Windows" "CurrentInstallFolder"
	StrCmp $r0 "" base_failed_to_find_sdk_3 base_found_sdk
base_failed_to_find_sdk_3:
	goto base_failed_to_find_sdk

base_failed_to_find_sdk:
	MessageBox MB_OK|MB_ICONEXCLAMATION "Failed to find Microsoft Platform SDK$\nPlease update your win32-dependencies.cmd script by hand" /SD IDOK
	goto base_go_on
base_found_sdk:
	MessageBox MB_OK "Microsoft Platform SDK found at :$\n$r0" /SD IDOK
	goto base_go_on

base_go_on:

	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	;clears dependencies file
	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" w
	FileWrite $0 "@echo off$\r$\n"
	FileWrite $0 "$\r$\n"
	FileWrite $0 "SET PATH=$r0\bin;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

SectionGroup "!Compilation platform"
Section /o "Visual C++ 2008" vs90
	StrCpy $suffix "vs90"
SectionEnd

Section "Visual C++ 2010" vs100
	StrCpy $suffix "vs100"
SectionEnd
SectionGroupEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

SectionGroup Required

Section "DirectX Runtime"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "$SYSDIR\d3dx9_42.dll" no_need_to_install_directx
	IfFileExists "arch\openvibe-directx.exe" no_need_to_download_directx
	NSISdl::download "http://www.microsoft.com/downloads/info.aspx?na=90&p=&SrcDisplayLang=en&SrcCategoryId=&SrcFamilyId=04ac064b-00d1-474e-b7b1-442d8712d553&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2fB%2f7%2f9%2fB79FC9D7-47B8-48B7-A75E-101DEBEB5AB4%2fdirectx_aug2009_redist.exe" "arch\openvibe-directx.exe"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_directx:
	ExecWait '"arch\openvibe-directx.exe" /T:"$INSTDIR\tmp" /Q'
	ExecWait '"$INSTDIR\tmp\DXSETUP.exe" /silent'
no_need_to_install_directx:

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "Visual Redistributable Packages"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\openvibe-vcredist-2010.exe" no_need_to_download_vcredist_2010
	NSISdl::download "http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe" "arch\openvibe-vcredist-2010.exe"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_vcredist_2010:
	ExecWait '"arch\openvibe-vcredist-2010.exe" /q'
;no_need_to_install_vcredist_2010:

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "CMake"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\cmake-2.8.7-win32-x86-ov2.zip" no_need_to_download_cmake
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/cmake-2.8.7-win32-x86-ov2.zip "arch\cmake-2.8.7-win32-x86-ov2.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_cmake:
	ZipDLL::extractall "arch\cmake-2.8.7-win32-x86-ov2.zip" ""

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\cmake\bin;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "eXpat"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\expat-2.0.1.zip" no_need_to_download_expat
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/expat-2.0.1.zip "arch\expat-2.0.1.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_expat:
	ZipDLL::extractall "arch\expat-2.0.1.zip" "expat"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\expat\bin;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "BOOST"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\boost-1.47.0-ov.zip" no_need_to_download_boost
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/boost-1.47.0-ov.zip "arch\boost-1.47.0-ov.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_boost:
	ZipDLL::extractall "arch\boost-1.47.0-ov.zip" ""


SectionEnd


;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "GTK+"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\gtk-2.22.1-dev.zip" no_need_to_download_gtk_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/gtk-2.22.1-dev.zip "arch\gtk-2.22.1-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_gtk_dev:
	ZipDLL::extractall "arch\gtk-2.22.1-dev.zip" "gtk"

	IfFileExists "arch\gtk-2.22.1-runtime.zip" no_need_to_download_gtk_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/gtk-2.22.1-runtime.zip "arch\gtk-2.22.1-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_gtk_runtime:
	ZipDLL::extractall "arch\gtk-2.22.1-runtime.zip" "gtk"

	FileOpen $0 "$INSTDIR\gtk\lib\pkgconfig\gtk+-win32-2.0.pc" w
	FileWrite $0 "prefix=$INSTDIR\gtk$\r$\n"
	FileWrite $0 "exec_prefix=$${prefix}$\r$\n"
	FileWrite $0 "libdir=$${exec_prefix}/lib$\r$\n"
	FileWrite $0 "includedir=$${prefix}/include$\r$\n"
	FileWrite $0 "target=win32$\r$\n"
	FileWrite $0 "$\r$\n"
	FileWrite $0 "Name: GTK+$\r$\n"
	FileWrite $0 "Description: GTK+ Graphical UI Library ($${target} target)$\r$\n"
	FileWrite $0 "Version: 2.22.1$\r$\n"
	FileWrite $0 "Requires: gdk-$${target}-2.0 atk cairo gdk-pixbuf-2.0 gio-2.0$\r$\n"
	FileWrite $0 "Libs: -L$${libdir} -lgtk-$${target}-2.0$\r$\n"
	FileWrite $0 "Cflags: -I$${includedir}/gtk-2.0$\r$\n"
	FileClose $0

	FileOpen $0 "$INSTDIR\gtk\lib\pkgconfig\gthread-2.0.pc" w
	FileWrite $0 "prefix=$INSTDIR\gtk$\r$\n"
	FileWrite $0 "exec_prefix=$${prefix}$\r$\n"
	FileWrite $0 "libdir=$${exec_prefix}/lib$\r$\n"
	FileWrite $0 "includedir=$${prefix}/include$\r$\n"
	FileWrite $0 "$\r$\n"
	FileWrite $0 "Name: GThread$\r$\n"
	FileWrite $0 "Description: Thread support for GLib$\r$\n"
	FileWrite $0 "Requires: glib-2.0$\r$\n"
	FileWrite $0 "Version: 2.26.0$\r$\n"
	FileWrite $0 "Libs: -L$${libdir} -lgthread-2.0$\r$\n"
	FileWrite $0 "Cflags:$\r$\n"
	FileClose $0

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\gtk\bin;%PATH%$\r$\n"	
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "IT++"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\itpp-4.0.7-$suffix-dev.zip" no_need_to_download_itpp_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/itpp-4.0.7-$suffix-dev.zip "arch\itpp-4.0.7-$suffix-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_itpp_dev:
	ZipDLL::extractall "arch\itpp-4.0.7-$suffix-dev.zip" "itpp"

	IfFileExists "arch\itpp-4.0.7-runtime.zip" no_need_to_download_itpp_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/itpp-4.0.7-runtime.zip "arch\itpp-4.0.7-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_itpp_runtime:
	ZipDLL::extractall "arch\itpp-4.0.7-runtime.zip" "itpp"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\itpp\bin;%PATH%$\r$\n"		
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "eigen"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\eigen-3.2.1-dev.zip" no_need_to_download_eigen
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/eigen-3.2.1-dev.zip "arch\eigen-3.2.1-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_eigen:
	ZipDLL::extractall "arch\eigen-3.2.1-dev.zip" ""

	; Eigen doesn't need path or vars set

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "Lua"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\lua-5.1.4-$suffix.zip" no_need_to_download_lua
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/lua-5.1.4-$suffix.zip "arch\lua-5.1.4-$suffix.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_lua:
	ZipDLL::extractall "arch\lua-5.1.4-$suffix.zip" ""

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\lua\lib;%PATH%$\r$\n"		
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "Ogre3D"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\ogre-1.7.1-$suffix-dev.zip" no_need_to_download_ogre_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/ogre-1.7.1-$suffix-dev.zip "arch\ogre-1.7.1-$suffix-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_ogre_dev:
	ZipDLL::extractall "arch\ogre-1.7.1-$suffix-dev.zip" "ogre"

	IfFileExists "arch\ogre-1.7.1-$suffix-runtime.zip" no_need_to_download_ogre_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/ogre-1.7.1-$suffix-runtime.zip "arch\ogre-1.7.1-$suffix-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_ogre_runtime:
	ZipDLL::extractall "arch\ogre-1.7.1-$suffix-runtime.zip" "ogre"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET OGRE_HOME=$INSTDIR\ogre$\r$\n"
	FileWrite $0 "SET PATH=%OGRE_HOME%\bin\release;%OGRE_HOME%\bin\debug;%PATH%$\r$\n"		
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "CEGUI"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\cegui-0.7.2-$suffix-dev.zip" no_need_to_download_cegui_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/cegui-0.7.2-$suffix-dev.zip "arch\cegui-0.7.2-$suffix-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_cegui_dev:
	ZipDLL::extractall "arch\cegui-0.7.2-$suffix-dev.zip" "cegui"

	IfFileExists "arch\cegui-0.7.2-$suffix-runtime.zip" no_need_to_download_cegui_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/cegui-0.7.2-$suffix-runtime.zip "arch\cegui-0.7.2-$suffix-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0"
			Quit
no_need_to_download_cegui_runtime:
	ZipDLL::extractall "arch\cegui-0.7.2-$suffix-runtime.zip" "cegui"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\cegui\bin;%PATH%$\r$\n"
	FileClose $0

	FileOpen $0 "$INSTDIR\cegui\resources.cfg" w
	FileWrite $0 "FileSystem=$INSTDIR\cegui\datafiles\configs$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\cegui\datafiles\fonts$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\cegui\datafiles\imagesets$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\cegui\datafiles\layouts$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\cegui\datafiles\looknfeel$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\cegui\datafiles\lua_scripts$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\cegui\datafiles\schemes$\r$\n"
	FileWrite $0 "FileSystem=$INSTDIR\cegui\datafiles\xml_schemes$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "VRPN"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\vrpn-7.31-$suffix-dev.zip" no_need_to_download_vrpn_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/vrpn-7.31-$suffix-dev.zip "arch\vrpn-7.31-$suffix-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_vrpn_dev:
	ZipDLL::extractall "arch\vrpn-7.31-$suffix-dev.zip" ""

	IfFileExists "arch\vrpn-7.31-$suffix-runtime.zip" no_need_to_download_vrpn_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/vrpn-7.31-$suffix-runtime.zip "arch\vrpn-7.31-$suffix-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_vrpn_runtime:
	ZipDLL::extractall "arch\vrpn-7.31-$suffix-runtime.zip" ""

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET VRPNROOT=$INSTDIR\vrpn$\r$\n"
	FileWrite $0 "SET PATH=%VRPNROOT%\bin;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "pthreads"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\pthreads-2.8.0-dev.zip" no_need_to_download_pthreads_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/pthreads-2.8.0-dev.zip "arch\pthreads-2.8.0-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_pthreads_dev:
	ZipDLL::extractall "arch\pthreads-2.8.0-dev.zip" "pthreads"

	IfFileExists "arch\pthreads-7.26-runtime.zip" no_need_to_download_pthreads_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/pthreads-2.8.0-runtime.zip "arch\pthreads-2.8.0-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_pthreads_runtime:
	ZipDLL::extractall "arch\pthreads-2.8.0-runtime.zip" "pthreads"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\pthreads\lib;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "OpenAL"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\openal-1.1-dev.zip" no_need_to_download_openal_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/openal-1.1-dev.zip "arch\openal-1.1-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_openal_dev:
	ZipDLL::extractall "arch\openal-1.1-dev.zip" "openal"

	IfFileExists "arch\openal-1.1-runtime.zip" no_need_to_download_openal_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/openal-1.1-runtime.zip "arch\openal-1.1-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_openal_runtime:
	ZipDLL::extractall "arch\openal-1.1-runtime.zip" "openal"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\openal\libs\win32;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "Alut"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\freealut-1.1.0-bin-dev.zip" no_need_to_download_alut_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/freealut-1.1.0-bin-dev.zip "arch\freealut-1.1.0-bin-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_alut_dev:
	ZipDLL::extractall "arch\freealut-1.1.0-bin-dev.zip" "freealut"

	IfFileExists "arch\freealut-1.1.0-bin-runtime.zip" no_need_to_download_alut_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/freealut-1.1.0-bin-runtime.zip "arch\freealut-1.1.0-bin-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_alut_runtime:
	ZipDLL::extractall "arch\freealut-1.1.0-bin-runtime.zip" "freealut"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\freealut\lib;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "Ogg"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\libogg-1.2.1-$suffix-dev.zip" no_need_to_download_ogg_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/libogg-1.2.1-$suffix-dev.zip "arch\libogg-1.2.1-$suffix-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_ogg_dev:
	ZipDLL::extractall "arch\libogg-1.2.1-$suffix-dev.zip" "libogg"

	IfFileExists "arch\libogg-1.2.1-$suffix-runtime.zip" no_need_to_download_ogg_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/libogg-1.2.1-$suffix-runtime.zip "arch\libogg-1.2.1-$suffix-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_ogg_runtime:
	ZipDLL::extractall "arch\libogg-1.2.1-$suffix-runtime.zip" "libogg"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\libogg\win32\bin\release;$INSTDIR\libogg\win32\bin\debug;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "Vorbis"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\libvorbis-1.3.2-$suffix-dev.zip" no_need_to_download_vorbis_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/libvorbis-1.3.2-$suffix-dev.zip "arch\libvorbis-1.3.2-$suffix-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_vorbis_dev:
	ZipDLL::extractall "arch\libvorbis-1.3.2-$suffix-dev.zip" "libvorbis"

	IfFileExists "arch\libvorbis-1.3.2-$suffix-runtime.zip" no_need_to_download_vorbis_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/libvorbis-1.3.2-$suffix-runtime.zip "arch\libvorbis-1.3.2-$suffix-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_vorbis_runtime:
	ZipDLL::extractall "arch\libvorbis-1.3.2-$suffix-runtime.zip" "libvorbis"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\libvorbis\win32\bin\release;$INSTDIR\libvorbis\win32\bin\debug;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "LSL"

	; LabStreamingLayer
	
	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\liblsl-1.04-$suffix-dev.zip" no_need_to_download_lsl_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/liblsl-1.04-$suffix-dev.zip "arch\liblsl-1.04-$suffix-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_lsl_dev:
	ZipDLL::extractall "arch\liblsl-1.04-$suffix-dev.zip" "liblsl"
	
	IfFileExists "arch\liblsl-1.04-$suffix-runtime.zip" no_need_to_download_lsl_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/liblsl-1.04-$suffix-runtime.zip "arch\liblsl-1.04-$suffix-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_lsl_runtime:
	ZipDLL::extractall "arch\liblsl-1.04-$suffix-runtime.zip" "liblsl"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\liblsl\lib\;%PATH%$\r$\n"
	FileClose $0

SectionEnd

SectionGroupEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

SectionGroup Optional optionalGroup

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section /o "GLFW"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\glfw-3.0.4-$suffix.zip" no_need_to_download_glfw
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/glfw-3.0.4-$suffix.zip "arch\glfw-3.0.4-$suffix.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_glfw:
	ZipDLL::extractall "arch\glfw-3.0.4-$suffix.zip" ""

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\glfw\lib;%PATH%$\r$\n"
	FileClose $0	

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section /o "GTK+ themes"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\gtk-themes-2009.09.07.zip" no_need_to_download_gtk_themes
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/gtk-themes-2009.09.07.zip "arch\gtk-themes-2009.09.07.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_gtk_themes:
	ZipDLL::extractall "arch\gtk-themes-2009.09.07.zip" "gtk"

	FileOpen $0 "$INSTDIR\gtk\etc\gtk-2.0\gtkrc" w
	FileWrite $0 "gtk-theme-name = $\"Redmond$\"$\r$\n"
	FileWrite $0 "style $\"user-font$\"$\r$\n"
	FileWrite $0 "{$\r$\n"
	FileWrite $0 "	font_name=$\"Sans 8$\"$\r$\n"
	FileWrite $0 "}$\r$\n"
	FileWrite $0 "widget_class $\"*$\" style $\"user-font$\"$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section /o "inpout32"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\inpout32-$suffix.zip" no_need_to_download_inpout32
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/inpout32-$suffix.zip "arch\inpout32-$suffix.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_inpout32:
	ZipDLL::extractall "arch\inpout32-$suffix.zip" ""

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\inpout32\lib;%PATH%$\r$\n"
	FileClose $0	

SectionEnd


;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section /o "presage"

; todo skip presage properly on vs2008
StrCmp $suffix "vs100" 0 nopresage
	
	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\presage-0.8.9-$suffix.zip" no_need_to_download_presage
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/presage-0.8.9-$suffix.zip "arch\presage-0.8.9-$suffix.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_presage:
	ZipDLL::extractall "arch\presage-0.8.9-$suffix.zip" ""

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\presage\lib;%PATH%$\r$\n"
	FileClose $0	
	goto presagepassed
	
nopresage:
	MessageBox MB_OK "Note: Presage not available for VS2008" /SD IDOK

presagepassed:	
	
SectionEnd


SectionGroupEnd


;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

SectionGroup Drivers DriverGroup

Section /o "Device SDK: MCS NVX"

	; For MCS NVX driver
	
	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\sdk-mcs-b-$suffix-dev.zip" no_need_to_download_mcs_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/sdk-mcs-b-$suffix-dev.zip "arch\sdk-mcs-b-$suffix-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_mcs_dev:
	ZipDLL::extractall "arch\sdk-mcs-b-$suffix-dev.zip" ""
	
SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section /o "Device SDK: Micromed"

	; For Micromed driver
	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\sdk-micromed-$suffix.zip" no_need_to_download_micromed_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/sdk-micromed-$suffix.zip "arch\sdk-micromed-$suffix.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_micromed_dev:
	ZipDLL::extractall "arch\sdk-micromed-$suffix.zip" ""

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section /o "Device SDK: MindMedia NeXus"

	; For NeXus driver
	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\sdk-nexus.zip" no_need_to_download_nexus_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/sdk-nexus.zip "arch\sdk-nexus.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_nexus_dev:
	ZipDLL::extractall "arch\sdk-nexus.zip" ""	

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section /o "Device SDK: Mitsar"

	; For mitsar driver
	
	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\sdk-mitsar.zip" no_need_to_download_mitsar_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/sdk-mitsar.zip "arch\sdk-mitsar.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_mitsar_dev:
	ZipDLL::extractall "arch\sdk-mitsar.zip" ""
	
SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section /o "Device SDK: NeuroElectrics Enobio3G"

	; For Neuroelectrics Enobio 3G driver
	
	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\enobio3g-1.2.1-$suffix-dev.zip" no_need_to_download_enobio_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/enobio3g-1.2.1-$suffix-dev.zip "arch\enobio3g-1.2.1-$suffix-dev.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_enobio_dev:
	ZipDLL::extractall "arch\enobio3g-1.2.1-$suffix-dev.zip" ""
	
	IfFileExists "arch\enobio3g-1.2.1-$suffix-runtime.zip" no_need_to_download_enobio_runtime
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/enobio3g-1.2.1-$suffix-runtime.zip "arch\enobio3g-1.2.1-$suffix-runtime.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_enobio_runtime:
	ZipDLL::extractall "arch\enobio3g-1.2.1-$suffix-runtime.zip" ""

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET PATH=$INSTDIR\enobio3g\MSVC\;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section /o "Device SDK: TMSi"

	; For TMSi universal driver
	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\sdk-tmsi.zip" no_need_to_download_tmsi_dev
	NSISdl::download http://openvibe.inria.fr/dependencies/win32/sdk-tmsi.zip "arch\sdk-tmsi.zip"
	Pop $R0 ; Get the return value
		StrCmp $R0 "success" +3
			MessageBox MB_OK "Download failed: $R0" /SD IDOK
			Quit
no_need_to_download_tmsi_dev:
	ZipDLL::extractall "arch\sdk-tmsi.zip" ""	

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################



SectionGroupEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Section "Uninstall"

	RMDir /r "$INSTDIR\gtk"
	RMDir /r "$INSTDIR\boost"
	RMDir /r "$INSTDIR\expat"
	RMDir /r "$INSTDIR\cmake"
	RMDir /r "$INSTDIR\itpp"
	RMDir /r "$INSTDIR\lua"
	RMDir /r "$INSTDIR\ogre"
	RMDir /r "$INSTDIR\vrpn"
	RMDir /r "$INSTDIR\openal"
	RMDir /r "$INSTDIR\alut"
	RMDir /r "$INSTDIR\libogg"
	RMDir /r "$INSTDIR\libvorbis"
	RMDir /r "$INSTDIR\liblsl"
	RMDir /r "$INSTDIR\tmp"
	RMDir /r "$INSTDIR\pthreads"
	RMDir /r "$INSTDIR\enobio3g"
	RMDir /r "$INSTDIR\mcs"
		
	Delete "$INSTDIR\..\scripts\win32-dependencies.cmd"

	Delete "$INSTDIR\Uninstall.exe"

	RMDir "$INSTDIR"

SectionEnd

;##########################################################################################################################################################
;##########################################################################################################################################################
;##########################################################################################################################################################

Function EnableOptionals

  SectionGetFlags ${optionalGroup} $0 
  IntOp $0 $0 | ${SF_SELECTED}
  SectionSetFlags ${optionalGroup} $0

  SectionGetFlags ${driverGroup} $0 
  IntOp $0 $0 | ${SF_SELECTED}
  SectionSetFlags ${driverGroup} $0

FunctionEnd

Function .onInit
  StrCpy $9 ${vs90}
  ; On silent install, we install all components
  IfSilent 0 +2
	Call EnableOptionals
FunctionEnd

Function .onSelChange
  !insertmacro StartRadioButtons $9
    !insertmacro RadioButton ${vs90}
    !insertmacro RadioButton ${vs100}
  !insertmacro EndRadioButtons
FunctionEnd
