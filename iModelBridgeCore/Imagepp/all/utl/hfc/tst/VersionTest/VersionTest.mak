# Microsoft Developer Studio Generated NMAKE File, Based on VersionTest.dsp
!IF "$(CFG)" == ""
CFG=VersionTest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to VersionTest - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "VersionTest - Win32 Release" && "$(CFG)" !=\
 "VersionTest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "VersionTest.mak" CFG="VersionTest - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "VersionTest - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "VersionTest - Win32 Debug" (based on\
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

!IF  "$(CFG)" == "VersionTest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\VersionTest.exe"

!ELSE

ALL : "$(OUTDIR)\VersionTest.exe"

!ENDIF

CLEAN :
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(INTDIR)\VersionTest.obj"
        -@erase "$(OUTDIR)\VersionTest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3  /O2 /EHsc /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp"$(INTDIR)\VersionTest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"\
  /c
CPP_OBJS=.\Release/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\VersionTest.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\VersionTest.pdb" /machine:I386 /out:"$(OUTDIR)\VersionTest.exe"\

LINK32_OBJS= \
        "$(INTDIR)\VersionTest.obj"

"$(OUTDIR)\VersionTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "VersionTest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\VersionTest.exe"

!ELSE

ALL : "$(OUTDIR)\VersionTest.exe"

!ENDIF

CLEAN :
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(INTDIR)\vc50.pdb"
        -@erase "$(INTDIR)\VersionTest.obj"
        -@erase "$(OUTDIR)\VersionTest.exe"
        -@erase "$(OUTDIR)\VersionTest.ilk"
        -@erase "$(OUTDIR)\VersionTest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm  /Zi /GS /Od /EHsc /I "\dev\h" /I "\dev\all\h" /D\
 "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\VersionTest.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c
CPP_OBJS=.\Debug/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\VersionTest.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\VersionTest.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\VersionTest.exe" /pdbtype:sept
LINK32_OBJS= \
        "$(INTDIR)\VersionTest.obj"

"$(OUTDIR)\VersionTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "VersionTest - Win32 Release" || "$(CFG)" ==\
 "VersionTest - Win32 Debug"
SOURCE=.\VersionTest.cpp

!IF  "$(CFG)" == "VersionTest - Win32 Release"

NODEP_CPP_VERSI=\
        ".\HFCVersion.h"\
        ".\hstdcpp.h"\


"$(INTDIR)\VersionTest.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "VersionTest - Win32 Debug"

DEP_CPP_VERSI=\
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
        "..\..\..\..\h\HFCVersion.h"\
        "..\..\..\..\h\HFCVersion.hpp"\
        "..\..\..\..\h\renew.h"\


"$(INTDIR)\VersionTest.obj" : $(SOURCE) $(DEP_CPP_VERSI) "$(INTDIR)"


!ENDIF


!ENDIF

