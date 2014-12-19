# Microsoft Developer Studio Project File - Name="HPSTST" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=HPSTST - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "HPSTST.MAK".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "HPSTST.MAK" CFG="HPSTST - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "HPSTST - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "HPSTST - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HPSTST - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\dev\h" /I "\dev\all\h" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 \dev\all\obj\dbg\hstdcpp.obj \dev\win\obj\dbg\whstdcpp.obj kernel32.lib user32.lib ole32.lib wininet.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF

# Begin Target

# Name "HPSTST - Win32 Release"
# Name "HPSTST - Win32 Debug"
# Begin Group "Source files"

# PROP Default_Filter "*.c;*.cpp"
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# End Group
# Begin Group "Libraries"

# PROP Default_Filter "*.lib;*.obj"
# Begin Source File

SOURCE=..\..\..\ext\lib\deflate_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hcd_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hcs_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HFC_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HGF_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HIM_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HMG_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\HMRCustomFPX_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HOD_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HPA_D.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HPM_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HPS_D.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRA_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRF_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRFInternetImaging_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRP_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\htiff_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HUC_D.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hve_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\Jpeg_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\LPIFpxLib_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\LPIJpegLib_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\png_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\swid32.dll
# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\wave_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\win\lib\winhcs_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\win\lib\winhfc_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\win\lib\WINHRA_d.lib
# End Source File
# End Group
# Begin Group "External objects"

# PROP Default_Filter "*.obj"
# Begin Group "Debug"

# PROP Default_Filter "*.obj"
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFCalsFile.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFHMR2File.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFHMRFile.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFIntergraphCITFile.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFIntergraphCOT29File.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFIntergraphCotFile.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFIntergraphFile.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFIntergraphRLEFile.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFIntergraphTG4File.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFInternetImagingFile.obj

!IF  "$(CFG)" == "HPSTST - Win32 Release"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFJpegFile.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFLocalRasterFile.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFRasterFile.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\HRFTiffFile.obj
# End Source File
# End Group
# End Group
# End Target
# End Project
