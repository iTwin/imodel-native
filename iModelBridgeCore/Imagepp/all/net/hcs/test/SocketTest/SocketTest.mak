# Microsoft Developer Studio Generated NMAKE File, Based on SocketTest.dsp
!IF "$(CFG)" == ""
CFG=SocketTest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to SocketTest - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "SocketTest - Win32 Release" && "$(CFG)" !=\
 "SocketTest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SocketTest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\SocketTest.exe"

!ELSE

ALL : "UtlLibs - Win32 Release" "NetLibs - Win32 Release"\
 "$(OUTDIR)\SocketTest.exe"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"NetLibs - Win32 ReleaseCLEAN" "UtlLibs - Win32 ReleaseCLEAN"
!ELSE
CLEAN :
!ENDIF
        -@erase "$(INTDIR)\PUBEngineThread.obj"
        -@erase "$(INTDIR)\PUBRequestDispatcher.obj"
        -@erase "$(INTDIR)\PUBRequestProcessor.obj"
        -@erase "$(INTDIR)\SocketTest.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(OUTDIR)\SocketTest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3  /O2 /EHsc /I "\dev\h" /I "\dev\all\h" /D "WIN32" /D\
 "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\SocketTest.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /I /dev/h" /I /dev/allh" /I\
 /dev/h" /I /dev/all/h" /I /dev/win/h" " " " " " /c
CPP_OBJS=.\Release/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SocketTest.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib wsock32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\SocketTest.pdb" /machine:I386 /out:"$(OUTDIR)\SocketTest.exe"
LINK32_OBJS= \
        "$(INTDIR)\PUBEngineThread.obj" \
        "$(INTDIR)\PUBRequestDispatcher.obj" \
        "$(INTDIR)\PUBRequestProcessor.obj" \
        "$(INTDIR)\SocketTest.obj" \
        "..\..\..\..\..\..\out\lib\NetLibs.lib" \
        "..\..\..\..\..\..\out\lib\UtlLibs.lib" \
        "..\..\..\..\..\..\out\obj\rel\all\HFCURLInternetImagingSocket.obj"

"$(OUTDIR)\SocketTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\SocketTest_d.exe"

!ELSE

ALL : "UtlLibs - Win32 Debug" "NetLibs - Win32 Debug"\
 "$(OUTDIR)\SocketTest_d.exe"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"NetLibs - Win32 DebugCLEAN" "UtlLibs - Win32 DebugCLEAN"
!ELSE
CLEAN :
!ENDIF
        -@erase "$(INTDIR)\PUBEngineThread.obj"
        -@erase "$(INTDIR)\PUBRequestDispatcher.obj"
        -@erase "$(INTDIR)\PUBRequestProcessor.obj"
        -@erase "$(INTDIR)\SocketTest.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(INTDIR)\vc50.pdb"
        -@erase "$(OUTDIR)\SocketTest_d.exe"
        -@erase "$(OUTDIR)\SocketTest_d.ilk"
        -@erase "$(OUTDIR)\SocketTest_d.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm  /Zi /GS /Od /EHsc /I "\dev\h" /I "\dev\all\h" /D\
 "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\SocketTest.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /I /dev/h" /I /dev/all/h" /I\
 /dev/h" /I /dev/all/h" /I /dev/win/h" " " " " " /c
CPP_OBJS=.\Debug/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SocketTest.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib wsock32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\SocketTest_d.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\SocketTest_d.exe" /pdbtype:sept
LINK32_OBJS= \
        "$(INTDIR)\PUBEngineThread.obj" \
        "$(INTDIR)\PUBRequestDispatcher.obj" \
        "$(INTDIR)\PUBRequestProcessor.obj" \
        "$(INTDIR)\SocketTest.obj" \
        "..\..\..\..\..\..\out\lib\NetLibs_D.lib" \
        "..\..\..\..\..\..\out\lib\UtlLibs_D.lib" \
        "..\..\..\..\..\..\out\obj\dbg\all\HFCURLInternetImagingSocket.obj"

"$(OUTDIR)\SocketTest_d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<


!IF "$(CFG)" == "SocketTest - Win32 Release" || "$(CFG)" ==\
 "SocketTest - Win32 Debug"
SOURCE=.\SocketTest.cpp

!IF  "$(CFG)" == "SocketTest - Win32 Release"

DEP_CPP_SOCKE=\
        "..\..\..\..\..\h\harrayautoptr.h"\
        "..\..\..\..\..\h\harrayautoptr.hpp"\
        "..\..\..\..\..\h\hautoptr.h"\
        "..\..\..\..\..\h\hautoptr.hpp"\
        "..\..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\..\h\hstdcpp.h"\
        "..\..\..\..\..\h\htypes.h"\
        "..\..\..\..\H\HCSBufferedConnectionPool.h"\
        "..\..\..\..\H\HCSConnectionPool.h"\
        "..\..\..\..\H\HCSGroupeableConnectionPool.h"\
        "..\..\..\..\H\HCSNamedPipeConnection.h"\
        "..\..\..\..\H\HCSNamedPipeConnectionPool.h"\
        "..\..\..\..\H\HCSNamedPipeServerConnection.h"\
        "..\..\..\..\H\HCSNamedPipeServerConnection.hpp"\
        "..\..\..\..\H\HCSRequestProcessor.h"\
        "..\..\..\..\H\HCSRequestProcessor.hpp"\
        "..\..\..\..\H\HCSServerConnection.h"\
        "..\..\..\..\H\HCSServerConnection.hpp"\
        "..\..\..\..\H\HCSServerListener.h"\
        "..\..\..\..\H\HCSSocketConnection.h"\
        "..\..\..\..\H\HCSSocketConnectionPool.h"\
        "..\..\..\..\H\HCSSocketServerConnection.h"\
        "..\..\..\..\H\HCSSocketServerConnection.hpp"\
        "..\..\..\..\h\HFCAccessMode.h"\
        "..\..\..\..\h\HFCAccessMode.hpp"\
        "..\..\..\..\h\HFCBuffer.h"\
        "..\..\..\..\h\HFCBuffer.hpp"\
        "..\..\..\..\H\HFCConnection.h"\
        "..\..\..\..\H\HFCConnection.hpp"\
        "..\..\..\..\h\HFCEvent.h"\
        "..\..\..\..\h\HFCEvent.hpp"\
        "..\..\..\..\h\HFCException.h"\
        "..\..\..\..\h\HFCException.hpp"\
        "..\..\..\..\h\HFCExclusiveKey.h"\
        "..\..\..\..\h\HFCExclusiveKey.hpp"\
        "..\..\..\..\h\hfchandle.h"\
        "..\..\..\..\h\HFCInterlockedValue.h"\
        "..\..\..\..\h\HFCInterlockedValue.hpp"\
        "..\..\..\..\H\HFCInternetConnection.h"\
        "..\..\..\..\H\HFCInternetConnection.hpp"\
        "..\..\..\..\h\HFCMonitor.h"\
        "..\..\..\..\h\HFCMonitor.hpp"\
        "..\..\..\..\H\HFCMutex.h"\
        "..\..\..\..\H\HFCMutex.hpp"\
        "..\..\..\..\h\HFCPtr.h"\
        "..\..\..\..\h\HFCPtr.hpp"\
        "..\..\..\..\h\HFCSemaphore.h"\
        "..\..\..\..\h\HFCSemaphore.hpp"\
        "..\..\..\..\h\HFCSynchro.h"\
        "..\..\..\..\h\HFCSynchro.hpp"\
        "..\..\..\..\h\HFCThread.h"\
        "..\..\..\..\h\HFCThread.hpp"\
        "..\..\..\..\h\HFCURL.h"\
        "..\..\..\..\h\HFCURL.hpp"\
        "..\..\..\..\h\HFCURLCommonInternet.h"\
        "..\..\..\..\h\HFCURLCommonInternet.hpp"\
        "..\..\..\..\h\renew.h"\
        ".\ClientThread.h"\
        ".\KeyboardThread.h"\
        ".\PUBCache.h"\
        ".\PUBCache.hpp"\
        ".\PUBCacheEntry.h"\
        ".\PUBEngineThread.h"\
        ".\PUBRequestDispatcher.h"\
        ".\PUBRequestProcessor.h"\
        ".\ServerThread.h"\


"$(INTDIR)\SocketTest.obj" : $(SOURCE) $(DEP_CPP_SOCKE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

DEP_CPP_SOCKE=\
        "..\..\..\..\..\h\harrayautoptr.h"\
        "..\..\..\..\..\h\harrayautoptr.hpp"\
        "..\..\..\..\..\h\hautoptr.h"\
        "..\..\..\..\..\h\hautoptr.hpp"\
        "..\..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\..\h\hstdcpp.h"\
        "..\..\..\..\..\h\htypes.h"\
        "..\..\..\..\H\HCSBufferedConnectionPool.h"\
        "..\..\..\..\H\HCSConnectionPool.h"\
        "..\..\..\..\H\HCSGroupeableConnectionPool.h"\
        "..\..\..\..\H\HCSNamedPipeConnection.h"\
        "..\..\..\..\H\HCSNamedPipeConnectionPool.h"\
        "..\..\..\..\H\HCSNamedPipeServerConnection.h"\
        "..\..\..\..\H\HCSNamedPipeServerConnection.hpp"\
        "..\..\..\..\H\HCSRequestProcessor.h"\
        "..\..\..\..\H\HCSRequestProcessor.hpp"\
        "..\..\..\..\H\HCSServerConnection.h"\
        "..\..\..\..\H\HCSServerConnection.hpp"\
        "..\..\..\..\H\HCSServerListener.h"\
        "..\..\..\..\H\HCSSocketConnection.h"\
        "..\..\..\..\H\HCSSocketConnectionPool.h"\
        "..\..\..\..\H\HCSSocketServerConnection.h"\
        "..\..\..\..\H\HCSSocketServerConnection.hpp"\
        "..\..\..\..\h\HFCAccessMode.h"\
        "..\..\..\..\h\HFCAccessMode.hpp"\
        "..\..\..\..\h\HFCBuffer.h"\
        "..\..\..\..\h\HFCBuffer.hpp"\
        "..\..\..\..\H\HFCConnection.h"\
        "..\..\..\..\H\HFCConnection.hpp"\
        "..\..\..\..\h\HFCEvent.h"\
        "..\..\..\..\h\HFCEvent.hpp"\
        "..\..\..\..\h\HFCException.h"\
        "..\..\..\..\h\HFCException.hpp"\
        "..\..\..\..\h\HFCExclusiveKey.h"\
        "..\..\..\..\h\HFCExclusiveKey.hpp"\
        "..\..\..\..\h\hfchandle.h"\
        "..\..\..\..\h\HFCInterlockedValue.h"\
        "..\..\..\..\h\HFCInterlockedValue.hpp"\
        "..\..\..\..\H\HFCInternetConnection.h"\
        "..\..\..\..\H\HFCInternetConnection.hpp"\
        "..\..\..\..\h\HFCMonitor.h"\
        "..\..\..\..\h\HFCMonitor.hpp"\
        "..\..\..\..\H\HFCMutex.h"\
        "..\..\..\..\H\HFCMutex.hpp"\
        "..\..\..\..\h\HFCPtr.h"\
        "..\..\..\..\h\HFCPtr.hpp"\
        "..\..\..\..\h\HFCSemaphore.h"\
        "..\..\..\..\h\HFCSemaphore.hpp"\
        "..\..\..\..\h\HFCSynchro.h"\
        "..\..\..\..\h\HFCSynchro.hpp"\
        "..\..\..\..\h\HFCThread.h"\
        "..\..\..\..\h\HFCThread.hpp"\
        "..\..\..\..\h\HFCURL.h"\
        "..\..\..\..\h\HFCURL.hpp"\
        "..\..\..\..\h\HFCURLCommonInternet.h"\
        "..\..\..\..\h\HFCURLCommonInternet.hpp"\
        "..\..\..\..\h\renew.h"\
        ".\ClientThread.h"\
        ".\KeyboardThread.h"\
        ".\PUBCache.h"\
        ".\PUBCache.hpp"\
        ".\PUBCacheEntry.h"\
        ".\PUBEngineThread.h"\
        ".\PUBRequestDispatcher.h"\
        ".\PUBRequestProcessor.h"\
        ".\ServerThread.h"\


"$(INTDIR)\SocketTest.obj" : $(SOURCE) $(DEP_CPP_SOCKE) "$(INTDIR)"


!ENDIF

SOURCE=.\PUBEngineThread.cpp

!IF  "$(CFG)" == "SocketTest - Win32 Release"

DEP_CPP_PUBEN=\
        "..\..\..\..\..\h\harrayautoptr.h"\
        "..\..\..\..\..\h\harrayautoptr.hpp"\
        "..\..\..\..\..\h\hautoptr.h"\
        "..\..\..\..\..\h\hautoptr.hpp"\
        "..\..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\..\h\hstdcpp.h"\
        "..\..\..\..\..\h\htypes.h"\
        "..\..\..\..\H\HCSConnectionPool.h"\
        "..\..\..\..\h\HFCAccessMode.h"\
        "..\..\..\..\h\HFCAccessMode.hpp"\
        "..\..\..\..\h\HFCBinStream.h"\
        "..\..\..\..\h\HFCBinStream.hpp"\
        "..\..\..\..\h\HFCBuffer.h"\
        "..\..\..\..\h\HFCBuffer.hpp"\
        "..\..\..\..\H\HFCConnection.h"\
        "..\..\..\..\H\HFCConnection.hpp"\
        "..\..\..\..\h\HFCEvent.h"\
        "..\..\..\..\h\HFCEvent.hpp"\
        "..\..\..\..\h\HFCException.h"\
        "..\..\..\..\h\HFCException.hpp"\
        "..\..\..\..\h\HFCExclusiveKey.h"\
        "..\..\..\..\h\HFCExclusiveKey.hpp"\
        "..\..\..\..\h\hfchandle.h"\
        "..\..\..\..\h\HFCHTTPHeader.h"\
        "..\..\..\..\h\HFCInterlockedValue.h"\
        "..\..\..\..\h\HFCInterlockedValue.hpp"\
        "..\..\..\..\H\HFCInternetConnection.h"\
        "..\..\..\..\H\HFCInternetConnection.hpp"\
        "..\..\..\..\h\HFCLocalBinStream.h"\
        "..\..\..\..\h\HFCLocalBinStream.hpp"\
        "..\..\..\..\h\HFCMonitor.h"\
        "..\..\..\..\h\HFCMonitor.hpp"\
        "..\..\..\..\h\HFCPtr.h"\
        "..\..\..\..\h\HFCPtr.hpp"\
        "..\..\..\..\h\HFCSemaphore.h"\
        "..\..\..\..\h\HFCSemaphore.hpp"\
        "..\..\..\..\h\HFCSynchro.h"\
        "..\..\..\..\h\HFCSynchro.hpp"\
        "..\..\..\..\h\HFCThread.h"\
        "..\..\..\..\h\HFCThread.hpp"\
        "..\..\..\..\h\HFCURL.h"\
        "..\..\..\..\h\HFCURL.hpp"\
        "..\..\..\..\h\HFCURLCommonInternet.h"\
        "..\..\..\..\h\HFCURLCommonInternet.hpp"\
        "..\..\..\..\h\renew.h"\
        ".\PUBCacheEntry.h"\
        ".\PUBEngineThread.h"\
        ".\PUBRequest.h"\
        ".\PUBRequest.hpp"\
        ".\PUBRequestDispatcher.h"\


"$(INTDIR)\PUBEngineThread.obj" : $(SOURCE) $(DEP_CPP_PUBEN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

DEP_CPP_PUBEN=\
        "..\..\..\..\..\h\harrayautoptr.h"\
        "..\..\..\..\..\h\harrayautoptr.hpp"\
        "..\..\..\..\..\h\hautoptr.h"\
        "..\..\..\..\..\h\hautoptr.hpp"\
        "..\..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\..\h\hstdcpp.h"\
        "..\..\..\..\..\h\htypes.h"\
        "..\..\..\..\H\HCSConnectionPool.h"\
        "..\..\..\..\h\HFCAccessMode.h"\
        "..\..\..\..\h\HFCAccessMode.hpp"\
        "..\..\..\..\h\HFCBinStream.h"\
        "..\..\..\..\h\HFCBinStream.hpp"\
        "..\..\..\..\h\HFCBuffer.h"\
        "..\..\..\..\h\HFCBuffer.hpp"\
        "..\..\..\..\H\HFCConnection.h"\
        "..\..\..\..\H\HFCConnection.hpp"\
        "..\..\..\..\h\HFCEvent.h"\
        "..\..\..\..\h\HFCEvent.hpp"\
        "..\..\..\..\h\HFCException.h"\
        "..\..\..\..\h\HFCException.hpp"\
        "..\..\..\..\h\HFCExclusiveKey.h"\
        "..\..\..\..\h\HFCExclusiveKey.hpp"\
        "..\..\..\..\h\hfchandle.h"\
        "..\..\..\..\h\HFCHTTPHeader.h"\
        "..\..\..\..\h\HFCInterlockedValue.h"\
        "..\..\..\..\h\HFCInterlockedValue.hpp"\
        "..\..\..\..\H\HFCInternetConnection.h"\
        "..\..\..\..\H\HFCInternetConnection.hpp"\
        "..\..\..\..\h\HFCLocalBinStream.h"\
        "..\..\..\..\h\HFCLocalBinStream.hpp"\
        "..\..\..\..\h\HFCMonitor.h"\
        "..\..\..\..\h\HFCMonitor.hpp"\
        "..\..\..\..\h\HFCPtr.h"\
        "..\..\..\..\h\HFCPtr.hpp"\
        "..\..\..\..\h\HFCSemaphore.h"\
        "..\..\..\..\h\HFCSemaphore.hpp"\
        "..\..\..\..\h\HFCSynchro.h"\
        "..\..\..\..\h\HFCSynchro.hpp"\
        "..\..\..\..\h\HFCThread.h"\
        "..\..\..\..\h\HFCThread.hpp"\
        "..\..\..\..\h\HFCURL.h"\
        "..\..\..\..\h\HFCURL.hpp"\
        "..\..\..\..\h\HFCURLCommonInternet.h"\
        "..\..\..\..\h\HFCURLCommonInternet.hpp"\
        "..\..\..\..\h\renew.h"\
        ".\PUBCacheEntry.h"\
        ".\PUBEngineThread.h"\
        ".\PUBRequest.h"\
        ".\PUBRequest.hpp"\
        ".\PUBRequestDispatcher.h"\


"$(INTDIR)\PUBEngineThread.obj" : $(SOURCE) $(DEP_CPP_PUBEN) "$(INTDIR)"


!ENDIF

SOURCE=.\PUBRequestDispatcher.cpp

!IF  "$(CFG)" == "SocketTest - Win32 Release"

DEP_CPP_PUBRE=\
        "..\..\..\..\..\h\harrayautoptr.h"\
        "..\..\..\..\..\h\harrayautoptr.hpp"\
        "..\..\..\..\..\h\hautoptr.h"\
        "..\..\..\..\..\h\hautoptr.hpp"\
        "..\..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\..\h\hstdcpp.h"\
        "..\..\..\..\..\h\htypes.h"\
        "..\..\..\..\H\HCSConnectionPool.h"\
        "..\..\..\..\h\HFCAccessMode.h"\
        "..\..\..\..\h\HFCAccessMode.hpp"\
        "..\..\..\..\h\HFCBuffer.h"\
        "..\..\..\..\h\HFCBuffer.hpp"\
        "..\..\..\..\H\HFCConnection.h"\
        "..\..\..\..\H\HFCConnection.hpp"\
        "..\..\..\..\h\HFCEvent.h"\
        "..\..\..\..\h\HFCEvent.hpp"\
        "..\..\..\..\h\HFCException.h"\
        "..\..\..\..\h\HFCException.hpp"\
        "..\..\..\..\h\HFCExclusiveKey.h"\
        "..\..\..\..\h\HFCExclusiveKey.hpp"\
        "..\..\..\..\h\hfchandle.h"\
        "..\..\..\..\h\HFCHTTPHeader.h"\
        "..\..\..\..\h\HFCInterlockedValue.h"\
        "..\..\..\..\h\HFCInterlockedValue.hpp"\
        "..\..\..\..\H\HFCInternetConnection.h"\
        "..\..\..\..\H\HFCInternetConnection.hpp"\
        "..\..\..\..\h\HFCMonitor.h"\
        "..\..\..\..\h\HFCMonitor.hpp"\
        "..\..\..\..\h\HFCPtr.h"\
        "..\..\..\..\h\HFCPtr.hpp"\
        "..\..\..\..\h\HFCSemaphore.h"\
        "..\..\..\..\h\HFCSemaphore.hpp"\
        "..\..\..\..\h\HFCSynchro.h"\
        "..\..\..\..\h\HFCSynchro.hpp"\
        "..\..\..\..\h\HFCThread.h"\
        "..\..\..\..\h\HFCThread.hpp"\
        "..\..\..\..\h\HFCURL.h"\
        "..\..\..\..\h\HFCURL.hpp"\
        "..\..\..\..\h\HFCURLCommonInternet.h"\
        "..\..\..\..\h\HFCURLCommonInternet.hpp"\
        "..\..\..\..\h\renew.h"\
        ".\PUBCacheEntry.h"\
        ".\PUBRequest.h"\
        ".\PUBRequest.hpp"\
        ".\PUBRequestDispatcher.h"\


"$(INTDIR)\PUBRequestDispatcher.obj" : $(SOURCE) $(DEP_CPP_PUBRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

DEP_CPP_PUBRE=\
        "..\..\..\..\..\h\harrayautoptr.h"\
        "..\..\..\..\..\h\harrayautoptr.hpp"\
        "..\..\..\..\..\h\hautoptr.h"\
        "..\..\..\..\..\h\hautoptr.hpp"\
        "..\..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\..\h\hstdcpp.h"\
        "..\..\..\..\..\h\htypes.h"\
        "..\..\..\..\H\HCSConnectionPool.h"\
        "..\..\..\..\h\HFCAccessMode.h"\
        "..\..\..\..\h\HFCAccessMode.hpp"\
        "..\..\..\..\h\HFCBuffer.h"\
        "..\..\..\..\h\HFCBuffer.hpp"\
        "..\..\..\..\H\HFCConnection.h"\
        "..\..\..\..\H\HFCConnection.hpp"\
        "..\..\..\..\h\HFCEvent.h"\
        "..\..\..\..\h\HFCEvent.hpp"\
        "..\..\..\..\h\HFCException.h"\
        "..\..\..\..\h\HFCException.hpp"\
        "..\..\..\..\h\HFCExclusiveKey.h"\
        "..\..\..\..\h\HFCExclusiveKey.hpp"\
        "..\..\..\..\h\hfchandle.h"\
        "..\..\..\..\h\HFCHTTPHeader.h"\
        "..\..\..\..\h\HFCInterlockedValue.h"\
        "..\..\..\..\h\HFCInterlockedValue.hpp"\
        "..\..\..\..\H\HFCInternetConnection.h"\
        "..\..\..\..\H\HFCInternetConnection.hpp"\
        "..\..\..\..\h\HFCMonitor.h"\
        "..\..\..\..\h\HFCMonitor.hpp"\
        "..\..\..\..\h\HFCPtr.h"\
        "..\..\..\..\h\HFCPtr.hpp"\
        "..\..\..\..\h\HFCSemaphore.h"\
        "..\..\..\..\h\HFCSemaphore.hpp"\
        "..\..\..\..\h\HFCSynchro.h"\
        "..\..\..\..\h\HFCSynchro.hpp"\
        "..\..\..\..\h\HFCThread.h"\
        "..\..\..\..\h\HFCThread.hpp"\
        "..\..\..\..\h\HFCURL.h"\
        "..\..\..\..\h\HFCURL.hpp"\
        "..\..\..\..\h\HFCURLCommonInternet.h"\
        "..\..\..\..\h\HFCURLCommonInternet.hpp"\
        "..\..\..\..\h\renew.h"\
        ".\PUBCacheEntry.h"\
        ".\PUBRequest.h"\
        ".\PUBRequest.hpp"\
        ".\PUBRequestDispatcher.h"\


"$(INTDIR)\PUBRequestDispatcher.obj" : $(SOURCE) $(DEP_CPP_PUBRE) "$(INTDIR)"


!ENDIF

SOURCE=.\PUBRequestProcessor.cpp

!IF  "$(CFG)" == "SocketTest - Win32 Release"

DEP_CPP_PUBREQ=\
        "..\..\..\..\..\h\harrayautoptr.h"\
        "..\..\..\..\..\h\harrayautoptr.hpp"\
        "..\..\..\..\..\h\hautoptr.h"\
        "..\..\..\..\..\h\hautoptr.hpp"\
        "..\..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\..\h\hstdcpp.h"\
        "..\..\..\..\..\h\htypes.h"\
        "..\..\..\..\H\HCSConnectionPool.h"\
        "..\..\..\..\H\HCSRequestProcessor.h"\
        "..\..\..\..\H\HCSRequestProcessor.hpp"\
        "..\..\..\..\h\HFCAccessMode.h"\
        "..\..\..\..\h\HFCAccessMode.hpp"\
        "..\..\..\..\h\HFCBuffer.h"\
        "..\..\..\..\h\HFCBuffer.hpp"\
        "..\..\..\..\H\HFCConnection.h"\
        "..\..\..\..\H\HFCConnection.hpp"\
        "..\..\..\..\h\HFCEvent.h"\
        "..\..\..\..\h\HFCEvent.hpp"\
        "..\..\..\..\h\HFCException.h"\
        "..\..\..\..\h\HFCException.hpp"\
        "..\..\..\..\h\HFCExclusiveKey.h"\
        "..\..\..\..\h\HFCExclusiveKey.hpp"\
        "..\..\..\..\h\hfchandle.h"\
        "..\..\..\..\h\HFCHTTPHeader.h"\
        "..\..\..\..\h\HFCHTTPParser.h"\
        "..\..\..\..\h\HFCHTTPParser.hpp"\
        "..\..\..\..\h\HFCInterlockedValue.h"\
        "..\..\..\..\h\HFCInterlockedValue.hpp"\
        "..\..\..\..\H\HFCInternetConnection.h"\
        "..\..\..\..\H\HFCInternetConnection.hpp"\
        "..\..\..\..\h\HFCMonitor.h"\
        "..\..\..\..\h\HFCMonitor.hpp"\
        "..\..\..\..\H\HFCMutex.h"\
        "..\..\..\..\H\HFCMutex.hpp"\
        "..\..\..\..\h\HFCPtr.h"\
        "..\..\..\..\h\HFCPtr.hpp"\
        "..\..\..\..\h\HFCSemaphore.h"\
        "..\..\..\..\h\HFCSemaphore.hpp"\
        "..\..\..\..\h\HFCSynchro.h"\
        "..\..\..\..\h\HFCSynchro.hpp"\
        "..\..\..\..\h\HFCThread.h"\
        "..\..\..\..\h\HFCThread.hpp"\
        "..\..\..\..\h\HFCURL.h"\
        "..\..\..\..\h\HFCURL.hpp"\
        "..\..\..\..\h\HFCURLCommonInternet.h"\
        "..\..\..\..\h\HFCURLCommonInternet.hpp"\
        "..\..\..\..\h\renew.h"\
        ".\PUBCache.h"\
        ".\PUBCache.hpp"\
        ".\PUBCacheEntry.h"\
        ".\PUBDumbAnalyzer.h"\
        ".\PUBDumbAnalyzer.hpp"\
        ".\PUBQueryAnalyzer.h"\
        ".\PUBQueryAnalyzer.hpp"\
        ".\PUBRequest.h"\
        ".\PUBRequest.hpp"\
        ".\PUBRequestDispatcher.h"\
        ".\PUBRequestProcessor.h"\


"$(INTDIR)\PUBRequestProcessor.obj" : $(SOURCE) $(DEP_CPP_PUBREQ) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

DEP_CPP_PUBREQ=\
        "..\..\..\..\..\h\harrayautoptr.h"\
        "..\..\..\..\..\h\harrayautoptr.hpp"\
        "..\..\..\..\..\h\hautoptr.h"\
        "..\..\..\..\..\h\hautoptr.hpp"\
        "..\..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\..\h\hstdcpp.h"\
        "..\..\..\..\..\h\htypes.h"\
        "..\..\..\..\H\HCSConnectionPool.h"\
        "..\..\..\..\H\HCSRequestProcessor.h"\
        "..\..\..\..\H\HCSRequestProcessor.hpp"\
        "..\..\..\..\h\HFCAccessMode.h"\
        "..\..\..\..\h\HFCAccessMode.hpp"\
        "..\..\..\..\h\HFCBuffer.h"\
        "..\..\..\..\h\HFCBuffer.hpp"\
        "..\..\..\..\H\HFCConnection.h"\
        "..\..\..\..\H\HFCConnection.hpp"\
        "..\..\..\..\h\HFCEvent.h"\
        "..\..\..\..\h\HFCEvent.hpp"\
        "..\..\..\..\h\HFCException.h"\
        "..\..\..\..\h\HFCException.hpp"\
        "..\..\..\..\h\HFCExclusiveKey.h"\
        "..\..\..\..\h\HFCExclusiveKey.hpp"\
        "..\..\..\..\h\hfchandle.h"\
        "..\..\..\..\h\HFCHTTPHeader.h"\
        "..\..\..\..\h\HFCHTTPParser.h"\
        "..\..\..\..\h\HFCHTTPParser.hpp"\
        "..\..\..\..\h\HFCInterlockedValue.h"\
        "..\..\..\..\h\HFCInterlockedValue.hpp"\
        "..\..\..\..\H\HFCInternetConnection.h"\
        "..\..\..\..\H\HFCInternetConnection.hpp"\
        "..\..\..\..\h\HFCMonitor.h"\
        "..\..\..\..\h\HFCMonitor.hpp"\
        "..\..\..\..\H\HFCMutex.h"\
        "..\..\..\..\H\HFCMutex.hpp"\
        "..\..\..\..\h\HFCPtr.h"\
        "..\..\..\..\h\HFCPtr.hpp"\
        "..\..\..\..\h\HFCSemaphore.h"\
        "..\..\..\..\h\HFCSemaphore.hpp"\
        "..\..\..\..\h\HFCSynchro.h"\
        "..\..\..\..\h\HFCSynchro.hpp"\
        "..\..\..\..\h\HFCThread.h"\
        "..\..\..\..\h\HFCThread.hpp"\
        "..\..\..\..\h\HFCURL.h"\
        "..\..\..\..\h\HFCURL.hpp"\
        "..\..\..\..\h\HFCURLCommonInternet.h"\
        "..\..\..\..\h\HFCURLCommonInternet.hpp"\
        "..\..\..\..\h\renew.h"\
        ".\PUBCache.h"\
        ".\PUBCache.hpp"\
        ".\PUBCacheEntry.h"\
        ".\PUBDumbAnalyzer.h"\
        ".\PUBDumbAnalyzer.hpp"\
        ".\PUBQueryAnalyzer.h"\
        ".\PUBQueryAnalyzer.hpp"\
        ".\PUBRequest.h"\
        ".\PUBRequest.hpp"\
        ".\PUBRequestDispatcher.h"\
        ".\PUBRequestProcessor.h"\


"$(INTDIR)\PUBRequestProcessor.obj" : $(SOURCE) $(DEP_CPP_PUBREQ) "$(INTDIR)"


!ENDIF

!IF  "$(CFG)" == "SocketTest - Win32 Release"

"NetLibs - Win32 Release" :
   cd "\dev\all\net"
   $(MAKE) /$(MAKEFLAGS) /F .\NetLibs.mak CFG="NetLibs - Win32 Release"
   cd ".\hcs\test\SocketTest"

"NetLibs - Win32 ReleaseCLEAN" :
   cd "\dev\all\net"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\NetLibs.mak CFG="NetLibs - Win32 Release"\
 RECURSE=1
   cd ".\hcs\test\SocketTest"

!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

"NetLibs - Win32 Debug" :
   cd "\dev\all\net"
   $(MAKE) /$(MAKEFLAGS) /F .\NetLibs.mak CFG="NetLibs - Win32 Debug"
   cd ".\hcs\test\SocketTest"

"NetLibs - Win32 DebugCLEAN" :
   cd "\dev\all\net"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\NetLibs.mak CFG="NetLibs - Win32 Debug"\
 RECURSE=1
   cd ".\hcs\test\SocketTest"

!ENDIF

!IF  "$(CFG)" == "SocketTest - Win32 Release"

"UtlLibs - Win32 Release" :
   cd "\dev\all\utl"
   $(MAKE) /$(MAKEFLAGS) /F .\UtlLibs.mak CFG="UtlLibs - Win32 Release"
   cd "..\net\hcs\test\SocketTest"

"UtlLibs - Win32 ReleaseCLEAN" :
   cd "\dev\all\utl"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\UtlLibs.mak CFG="UtlLibs - Win32 Release"\
 RECURSE=1
   cd "..\net\hcs\test\SocketTest"

!ELSEIF  "$(CFG)" == "SocketTest - Win32 Debug"

"UtlLibs - Win32 Debug" :
   cd "\dev\all\utl"
   $(MAKE) /$(MAKEFLAGS) /F .\UtlLibs.mak CFG="UtlLibs - Win32 Debug"
   cd "..\net\hcs\test\SocketTest"

"UtlLibs - Win32 DebugCLEAN" :
   cd "\dev\all\utl"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\UtlLibs.mak CFG="UtlLibs - Win32 Debug"\
 RECURSE=1
   cd "..\net\hcs\test\SocketTest"

!ENDIF


!ENDIF

