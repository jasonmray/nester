# Microsoft Developer Studio Project File - Name="nester" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=nester - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nester.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nester.mak" CFG="nester - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nester - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "nester - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nester - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/nes/mapper" /I "src/win32" /I "src/win32/resource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 comctl32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /profile
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=.\src\win32\resource\postbuild.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "nester - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/win32/resource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386

!ENDIF 

# Begin Target

# Name "nester - Win32 Release"
# Name "nester - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "nes"

# PROP Default_Filter ""
# Begin Group "apu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Nes\Apu\nes_apu.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_apu_wrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\src\nes\apu\vrcvisnd.c
# End Source File
# End Group
# Begin Group "cpu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Nes\cpu\nes6502.c
# End Source File
# Begin Source File

SOURCE=.\src\nes\cpu\nes_6502.cpp
# End Source File
# End Group
# Begin Group "ppu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\nes\ppu\nes_ppu.cpp
# End Source File
# End Group
# Begin Group "libsnss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Nes\libsnss\libsnss.c
# End Source File
# End Group
# Begin Group "mapper"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\nes\mapper\nes_mapper.cpp
# End Source File
# Begin Source File

SOURCE=.\src\nes\mapper\NES_mapper_Konami.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\nes\nes.cpp
# End Source File
# Begin Source File

SOURCE=.\src\nes\nes_rom.cpp
# End Source File
# Begin Source File

SOURCE=.\src\nes\snss.cpp
# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\WIN32\iDDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\iDIDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\iDirectX.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_default_controls.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_input_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_key_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_keytable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directory.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directsound_sound_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_emu.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_fullscreen_NES_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_GUID.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_INPButtons.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_NES_pad.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_NES_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_settings.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_shellext.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_timing.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_windowed_NES_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\winmain.cpp
# End Source File
# End Group
# Begin Group "debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\debug\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\src\debug\HEX.cpp
# End Source File
# Begin Source File

SOURCE=.\src\debug\mono.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\recent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\settings.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "nes headers"

# PROP Default_Filter ""
# Begin Group "apu headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\nes\apu\nes_apu.h
# End Source File
# Begin Source File

SOURCE=.\Src\Nes\Apu\nes_apu_wrapper.h
# End Source File
# Begin Source File

SOURCE=.\src\nes\apu\vrcvisnd.h
# End Source File
# End Group
# Begin Group "cpu headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Nes\cpu\nes6502.h
# End Source File
# Begin Source File

SOURCE=.\src\nes\cpu\nes_6502.h
# End Source File
# End Group
# Begin Group "ppu headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\nes\ppu\nes_ppu.h
# End Source File
# End Group
# Begin Group "libsnss headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Nes\libsnss\libsnss.h
# End Source File
# End Group
# Begin Group "mapper headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\nes\mapper\nes_mapper.h
# End Source File
# Begin Source File

SOURCE=.\src\nes\mapper\NES_mapper_Konami.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\nes\nes.h
# End Source File
# Begin Source File

SOURCE=.\src\nes\nes_pad.h
# End Source File
# Begin Source File

SOURCE=.\src\nes\nes_pal.h
# End Source File
# Begin Source File

SOURCE=.\src\nes\nes_rom.h
# End Source File
# Begin Source File

SOURCE=.\src\nes\nes_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\nes\nes_settings.h
# End Source File
# Begin Source File

SOURCE=.\src\nes\snss.h
# End Source File
# End Group
# Begin Group "win32 headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\WIN32\iDDraw.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\iDIDevice.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\iDirectX.h
# End Source File
# Begin Source File

SOURCE=.\Src\Win32\OSD_ButtonSettings.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\OSD_NES_graphics_settings.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_default_controls.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_dialogs.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directinput_input_mgr.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_key_filter.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_keytable.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directory.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directsound_sound_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_emu.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_fullscreen_NES_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_globals.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_GUID.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_INPButtons.h
# End Source File
# Begin Source File

SOURCE=.\Src\Win32\win32_NES_pad.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_NES_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_shellext.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_timing.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_windowed_NES_screen_mgr.h
# End Source File
# End Group
# Begin Group "debug headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\debug\debug.h
# End Source File
# Begin Source File

SOURCE=.\src\debug\debuglog.h
# End Source File
# Begin Source File

SOURCE=.\src\debug\HEX.h
# End Source File
# Begin Source File

SOURCE=.\src\debug\mono.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\controller.h
# End Source File
# Begin Source File

SOURCE=.\src\emulator.h
# End Source File
# Begin Source File

SOURCE=.\SRC\INPButton.h
# End Source File
# Begin Source File

SOURCE=.\src\input_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\null_sound_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\pixmap.h
# End Source File
# Begin Source File

SOURCE=.\src\recent.h
# End Source File
# Begin Source File

SOURCE=.\src\screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\settings.h
# End Source File
# Begin Source File

SOURCE=.\src\sound_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\types.h
# End Source File
# Begin Source File

SOURCE=.\SRC\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Src\Win32\Resource\nester.ico
# End Source File
# Begin Source File

SOURCE=.\src\win32\resource\nester.rc
# End Source File
# Begin Source File

SOURCE=.\src\win32\resource\postbuild.bat
# End Source File
# Begin Source File

SOURCE=.\src\win32\resource\profile.bat
# End Source File
# Begin Source File

SOURCE=.\src\win32\resource\resource.h
# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\COPYING.txt
# End Source File
# Begin Source File

SOURCE=.\docs\issues.txt
# End Source File
# Begin Source File

SOURCE=".\docs\loopy-2005.txt"
# End Source File
# Begin Source File

SOURCE=.\docs\mappers.txt
# End Source File
# Begin Source File

SOURCE=.\docs\nessound.txt
# End Source File
# Begin Source File

SOURCE=.\docs\nestech.txt
# End Source File
# Begin Source File

SOURCE=.\docs\project.txt
# End Source File
# Begin Source File

SOURCE=.\docs\readme.txt
# End Source File
# Begin Source File

SOURCE=".\docs\snss-tff.txt"
# End Source File
# Begin Source File

SOURCE=.\docs\todo.txt
# End Source File
# End Group
# End Target
# End Project
