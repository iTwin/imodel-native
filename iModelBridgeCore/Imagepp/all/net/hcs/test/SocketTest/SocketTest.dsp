# Microsoft Developer Studio Project File - Name="SocketTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SocketTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "SocketTest.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "SocketTest.mak" CFG="SocketTest - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "SocketTest - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "SocketTest - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SocketTest - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "\dev\h" /I "\dev\all\h" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /I /dev/h" /I /dev/allh" /I /dev/h" /I /dev/all/h" /I /dev/win/h" " " " " " /c
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\dev\h" /I "\dev\all\h" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /I /dev/h" /I /dev/all/h" /I /dev/h" /I /dev/all/h" /I /dev/win/h" " " " " " /c
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/SocketTest_d.exe" /pdbtype:sept

!ENDIF

# Begin Target

# Name "SocketTest - Win32 Release"
# Name "SocketTest - Win32 Debug"
# Begin Group "Release"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\..\..\out\obj\rel\all\HFCURLInternetImagingSocket.obj

!IF  "$(CFG)" == "SocketTest - Win32 Release"

!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\out\lib\NetLibs.lib

!IF  "$(CFG)" == "SocketTest - Win32 Release"

!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\out\lib\UtlLibs.lib

!IF  "$(CFG)" == "SocketTest - Win32 Release"

!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF

# End Source File
# End Group
# Begin Group "Debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\..\..\out\obj\dbg\all\HFCURLInternetImagingSocket.obj

!IF  "$(CFG)" == "SocketTest - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\out\lib\NetLibs_D.lib

!IF  "$(CFG)" == "SocketTest - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\out\lib\UtlLibs_D.lib

!IF  "$(CFG)" == "SocketTest - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

!ENDIF

# End Source File
# End Group
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SocketTest.cpp
# End Source File
# End Group
# Begin Group "Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ClientThread.h
# End Source File
# Begin Source File

SOURCE=.\KeyboardThread.h
# End Source File
# Begin Source File

SOURCE=.\NothingThread.h
# End Source File
# Begin Source File

SOURCE=.\ServerThread.h
# End Source File
# Begin Source File

SOURCE=.\ServerThread2.h
# End Source File
# End Group
# Begin Group "Publisher"

# PROP Default_Filter ""
# Begin Group "Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\PUBEngineThread.cpp
# End Source File
# Begin Source File

SOURCE=.\PUBRequestDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\PUBRequestProcessor.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp"
# Begin Source File

SOURCE=.\PUBCache.h
# End Source File
# Begin Source File

SOURCE=.\PUBCache.hpp
# End Source File
# Begin Source File

SOURCE=.\PUBCacheEntry.h
# End Source File
# Begin Source File

SOURCE=.\PUBDumbAnalyzer.h
# End Source File
# Begin Source File

SOURCE=.\PUBDumbAnalyzer.hpp
# End Source File
# Begin Source File

SOURCE=.\PUBEngineThread.h
# End Source File
# Begin Source File

SOURCE=.\PUBQueryAnalyzer.h
# End Source File
# Begin Source File

SOURCE=.\PUBQueryAnalyzer.hpp
# End Source File
# Begin Source File

SOURCE=.\PUBRequest.h
# End Source File
# Begin Source File

SOURCE=.\PUBRequest.hpp
# End Source File
# Begin Source File

SOURCE=.\PUBRequestDispatcher.h
# End Source File
# Begin Source File

SOURCE=.\PUBRequestProcessor.h
# End Source File
# End Group
# End Group
# End Target
# End Project
