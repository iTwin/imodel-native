# Microsoft Developer Studio Project File - Name="HRFCloning" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=HRFCloning - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "HRFCloning.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "HRFCloning.mak" CFG="HRFCloning - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "HRFCloning - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "HRFCloning - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "HRFCloni"
# PROP BASE Intermediate_Dir "HRFCloni"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "HRFCloni"
# PROP Intermediate_Dir "HRFCloni"
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

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "HRFClon0"
# PROP BASE Intermediate_Dir "HRFClon0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "HRFClon0"
# PROP Intermediate_Dir "HRFClon0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\dev\h" /I "\dev\all\h" /I "\dev\all\ext\hmr\h" /I "\dev\win\h" /I "\dev\all\ext\tiff\h" /I "\dev\all\ext\flashpix\jpeg" /I "\dev\all\ext\oracle\h" /I "\dev\all\ext\wavelet\h" /I "\dev\all\ext\htiff\h" /I "\dev\all\ext\jpeg\h" /I "\dev\all\ext\png\h" /I "\dev\all\ext\deflate\h" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF

# Begin Target

# Name "HRFCloning - Win32 Release"
# Name "HRFCloning - Win32 Debug"
# Begin Group "Lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\ext\lib\deflate.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\deflate_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hcd.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hcd_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hcs.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hcs_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\Hdp_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HDS.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HDS_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HDSC_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HFC.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HFC_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HGF.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HGF_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HIM.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HIM_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\his_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HMG.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HMG_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\HMRCustomFPX.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\HMRCustomFPX_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HOD.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HOD_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hot.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hot_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HPM.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HPM_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRA.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRA_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRF.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRF_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRFInternetImaging.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRFInternetImaging_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRP.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HRP_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hru_d.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\dbg\hstdcpp.obj
# End Source File
# Begin Source File

SOURCE=..\..\..\obj\rel\hstdcpp.obj

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\htiff.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\htiff_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HUC.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HUC_D.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HUT.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\HUT_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hve.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\lib\hve_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\Jpeg.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\Jpeg_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\LPIFpxLib.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\LPIFpxLib_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\LPIJpegLib.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\LPIJpegLib_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\png.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\png_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\wave.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\ext\lib\wave_d.lib

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# End Group
# Begin Group "External Objects"

# PROP Default_Filter ""
# Begin Group "Release"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\obj\rel\HUCURLHTTP.obj

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# End Group
# Begin Group "Debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\obj\dbg\HUCURLHTTP.obj

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

!ENDIF

# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\HRFCloning.cpp
# End Source File
# End Target
# End Project
