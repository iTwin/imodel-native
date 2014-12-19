# Microsoft Developer Studio Project File - Name="HFCStatTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=HFCStatTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "HFCStatTest.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "HFCStatTest.mak" CFG="HFCStatTest - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "HFCStatTest - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "HFCStatTest - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HFCStatTest - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "\dev\h" /I "\dev\all\h" /I "\dev\win\h" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "HFCStatTest - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\dev\h" /I "\dev\all\h" /I "\dev\win\h" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib ws2_32.lib wininet.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF

# Begin Target

# Name "HFCStatTest - Win32 Release"
# Name "HFCStatTest - Win32 Debug"
# Begin Source File

SOURCE=..\..\..\..\ext\lib\deflate_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\hcd_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\hcs_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\hdb_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\Hdp_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HDS_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HDSC_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HFC_d.lib
# End Source File
# Begin Source File

SOURCE=.\HFCStatTest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\hfs_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HGF_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HGS_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HIM_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\his_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\hle_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HLM_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HMG_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HMRLicense_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HOD_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\hot_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HPA_D.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HPM_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HPS_D.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HRA_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HRF_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HRFInternetImaging_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\dbg\HRFInternetImagingFile.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HRP_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\hru_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\htiff_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\HUT_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\lib\hve_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\ext\lib\Jpeg_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\ext\lib\wave_d.lib
# End Source File
# End Target
# End Project
