# Microsoft Developer Studio Generated NMAKE File, Based on SQLList.dsp
!IF "$(CFG)" == ""
CFG=SQLList - Win32 Debug
!MESSAGE No configuration specified. Defaulting to SQLList - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "SQLList - Win32 Release" && "$(CFG)" !=\
 "SQLList - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "SQLList.mak" CFG="SQLList - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "SQLList - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SQLList - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "SQLList - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\SQLList.exe"

!ELSE

ALL : "hfs - Win32 Release" "$(OUTDIR)\SQLList.exe"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"hfs - Win32 ReleaseCLEAN"
!ELSE
CLEAN :
!ENDIF
        -@erase "$(INTDIR)\SQLList.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(OUTDIR)\SQLList.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3  /O2 /EHsc /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp"$(INTDIR)\SQLList.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c\

CPP_OBJS=.\Release/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SQLList.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\SQLList.pdb" /machine:I386 /out:"$(OUTDIR)\SQLList.exe"
LINK32_OBJS= \
        "$(INTDIR)\SQLList.obj" \
        "..\..\..\lib\HFC_d.lib" \
        "..\..\..\lib\hfs.lib" \
        "..\..\..\lib\hfs_d.lib" \
        "..\..\..\obj\dbg\HFCURLCommonInternet.obj" \
        "..\..\..\obj\dbg\HFCURLHTTP.obj" \
        "..\..\..\obj\dbg\HFCURLInternetImagingSocket.obj"

"$(OUTDIR)\SQLList.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SQLList - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\SQLList.exe"

!ELSE

ALL : "hfs - Win32 Debug" "$(OUTDIR)\SQLList.exe"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"hfs - Win32 DebugCLEAN"
!ELSE
CLEAN :
!ENDIF
        -@erase "$(INTDIR)\SQLList.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(INTDIR)\vc50.pdb"
        -@erase "$(OUTDIR)\SQLList.exe"
        -@erase "$(OUTDIR)\SQLList.ilk"
        -@erase "$(OUTDIR)\SQLList.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm  /Zi /GS /Od /EHsc /I "\dev\h" /I "\dev\all\h" /D\
 "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\SQLList.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c
CPP_OBJS=.\Debug/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SQLList.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib wininet.lib ws2_32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\SQLList.pdb" /debug /machine:I386 /out:"$(OUTDIR)\SQLList.exe"\
 /pdbtype:sept
LINK32_OBJS= \
        "$(INTDIR)\SQLList.obj" \
        "..\..\..\lib\HFC_d.lib" \
        "..\..\..\lib\hfs_d.lib" \
        "..\..\..\obj\dbg\HFCURLCommonInternet.obj" \
        "..\..\..\obj\dbg\HFCURLHTTP.obj" \
        "..\..\..\obj\dbg\HFCURLInternetImagingSocket.obj"

"$(OUTDIR)\SQLList.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "SQLList - Win32 Release" || "$(CFG)" ==\
 "SQLList - Win32 Debug"

!IF  "$(CFG)" == "SQLList - Win32 Release"

"hfs - Win32 Release" :
   cd "\dev\all\utl\hfs\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hfs.mak CFG="hfs - Win32 Release"
   cd "..\tst"

"hfs - Win32 ReleaseCLEAN" :
   cd "\dev\all\utl\hfs\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hfs.mak CFG="hfs - Win32 Release" RECURSE=1\

   cd "..\tst"

!ELSEIF  "$(CFG)" == "SQLList - Win32 Debug"

"hfs - Win32 Debug" :
   cd "\dev\all\utl\hfs\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hfs.mak CFG="hfs - Win32 Debug"
   cd "..\tst"

"hfs - Win32 DebugCLEAN" :
   cd "\dev\all\utl\hfs\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hfs.mak CFG="hfs - Win32 Debug" RECURSE=1
   cd "..\tst"

!ENDIF

SOURCE=.\SQLList.cpp

!IF  "$(CFG)" == "SQLList - Win32 Release"

NODEP_CPP_SQLLI=\
        ".\HFCHTTPConnection.h"\
        ".\HFCInternetConnection.h"\
        ".\HFCSocketConnection.h"\
        ".\HFSSQLListLister.h"\


"$(INTDIR)\SQLList.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SQLList - Win32 Debug"

DEP_CPP_SQLLI=\
        "..\..\..\..\h\harrayautoptr.h"\
        "..\..\..\..\h\harrayautoptr.hpp"\
        "..\..\..\..\h\hautoptr.h"\
        "..\..\..\..\h\hautoptr.hpp"\
        "..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\h\hstdcpp.h"\
        "..\..\..\..\h\htypes.h"\
        "..\..\..\h\hfcaccessmode.h"\
        "..\..\..\h\hfcaccessmode.hpp"\
        "..\..\..\h\hfcbuffer.h"\
        "..\..\..\h\hfcbuffer.hpp"\
        "..\..\..\h\hfcconnection.h"\
        "..\..\..\h\hfcconnection.hpp"\
        "..\..\..\h\hfcevent.h"\
        "..\..\..\h\hfcevent.hpp"\
        "..\..\..\h\hfcexception.h"\
        "..\..\..\h\hfcexception.hpp"\
        "..\..\..\h\hfcexclusivekey.h"\
        "..\..\..\h\hfcexclusivekey.hpp"\
        "..\..\..\h\hfchandle.h"\
        "..\..\..\h\hfchttpconnection.h"\
        "..\..\..\h\hfchttpconnection.hpp"\
        "..\..\..\h\hfcinterlockedvalue.h"\
        "..\..\..\h\hfcinterlockedvalue.hpp"\
        "..\..\..\h\hfcinternetconnection.h"\
        "..\..\..\h\hfcinternetconnection.hpp"\
        "..\..\..\h\hfcmonitor.h"\
        "..\..\..\h\hfcmonitor.hpp"\
        "..\..\..\h\hfcptr.h"\
        "..\..\..\h\hfcptr.hpp"\
        "..\..\..\h\hfcsocketconnection.h"\
        "..\..\..\h\hfcsynchro.h"\
        "..\..\..\h\hfcsynchro.hpp"\
        "..\..\..\h\hfcurl.h"\
        "..\..\..\h\hfcurl.hpp"\
        "..\..\..\h\hfcurlcommoninternet.h"\
        "..\..\..\h\hfcurlcommoninternet.hpp"\
        "..\..\..\h\hfcurlhttp.h"\
        "..\..\..\h\hfcurlhttp.hpp"\
        "..\..\..\h\hfcversion.h"\
        "..\..\..\h\hfcversion.hpp"\
        "..\..\..\h\hfssqllistlister.h"\
        "..\..\..\h\renew.h"\


"$(INTDIR)\SQLList.obj" : $(SOURCE) $(DEP_CPP_SQLLI) "$(INTDIR)"


!ENDIF


!ENDIF

