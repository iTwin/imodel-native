# Microsoft Developer Studio Generated NMAKE File, Based on HPATST.DSP
!IF "$(CFG)" == ""
CFG=HPATST - Win32 Debug
!MESSAGE No configuration specified. Defaulting to HPATST - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "HPATST - Win32 Release" && "$(CFG)" != "HPATST - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "HPATST.MAK" CFG="HPATST - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "HPATST - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "HPATST - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "HPATST - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\HPATST.exe"

!ELSE

ALL : "HPA - Win32 Release" "$(OUTDIR)\HPATST.exe"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"HPA - Win32 ReleaseCLEAN"
!ELSE
CLEAN :
!ENDIF
        -@erase "$(INTDIR)\main.obj"
        -@erase "$(INTDIR)\TestParser.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(OUTDIR)\HPATST.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3  /O2 /EHsc /I "\dev\h" /I "\dev\all\h" /D "WIN32" /D\
 "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c
CPP_OBJS=.\Release/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HPATST.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\HPATST.pdb" /machine:I386 /out:"$(OUTDIR)\HPATST.exe"
LINK32_OBJS= \
        "$(INTDIR)\main.obj" \
        "$(INTDIR)\TestParser.obj" \
        "..\..\..\lib\HFC_d.lib" \
        "..\..\..\lib\HPA.lib"

"$(OUTDIR)\HPATST.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "HPATST - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\HPATST.exe"

!ELSE

ALL : "HPA - Win32 Debug" "$(OUTDIR)\HPATST.exe"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"HPA - Win32 DebugCLEAN"
!ELSE
CLEAN :
!ENDIF
        -@erase "$(INTDIR)\main.obj"
        -@erase "$(INTDIR)\TestParser.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(INTDIR)\vc50.pdb"
        -@erase "$(OUTDIR)\HPATST.exe"
        -@erase "$(OUTDIR)\HPATST.ilk"
        -@erase "$(OUTDIR)\HPATST.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3  /Zi /GS /Od /EHsc /I "\dev\h" /I "\dev\all\h" /D "WIN32" /D\
 "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c
CPP_OBJS=.\Debug/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HPATST.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib \dev\all\obj\dbg/hstdcpp.obj /nologo /subsystem:console\
 /incremental:yes /pdb:"$(OUTDIR)\HPATST.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\HPATST.exe" /pdbtype:sept
LINK32_OBJS= \
        "$(INTDIR)\main.obj" \
        "$(INTDIR)\TestParser.obj" \
        "..\..\..\lib\HFC_d.lib" \
        "..\..\..\lib\HPA_D.lib"

"$(OUTDIR)\HPATST.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "HPATST - Win32 Release" || "$(CFG)" == "HPATST - Win32 Debug"

!IF  "$(CFG)" == "HPATST - Win32 Release"

"HPA - Win32 Release" :
   cd "\dev\all\utl\hpa\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HPA.MAK CFG="HPA - Win32 Release"
   cd "..\tst"

"HPA - Win32 ReleaseCLEAN" :
   cd "\dev\all\utl\hpa\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HPA.MAK CFG="HPA - Win32 Release" RECURSE=1\

   cd "..\tst"

!ELSEIF  "$(CFG)" == "HPATST - Win32 Debug"

"HPA - Win32 Debug" :
   cd "\dev\all\utl\hpa\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HPA.MAK CFG="HPA - Win32 Debug"
   cd "..\tst"

"HPA - Win32 DebugCLEAN" :
   cd "\dev\all\utl\hpa\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HPA.MAK CFG="HPA - Win32 Debug" RECURSE=1
   cd "..\tst"

!ENDIF

SOURCE=.\main.cpp

!IF  "$(CFG)" == "HPATST - Win32 Release"

DEP_CPP_MAIN_=\
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
        "..\..\..\h\HFCPtr.h"\
        "..\..\..\h\HFCPtr.hpp"\
        "..\..\..\h\HPADynamicParser.h"\
        "..\..\..\h\HPADynamicParser.hpp"\
        "..\..\..\h\HPAGrammarObject.h"\
        "..\..\..\h\HPAGrammarObject.hpp"\
        "..\..\..\h\HPANode.h"\
        "..\..\..\h\HPANode.hpp"\
        "..\..\..\h\HPAParser.h"\
        "..\..\..\h\HPAParser.hpp"\
        "..\..\..\h\HPAProduction.h"\
        "..\..\..\h\HPAProduction.hpp"\
        "..\..\..\h\HPARule.h"\
        "..\..\..\h\HPARule.hpp"\
        "..\..\..\h\HPAToken.h"\
        "..\..\..\h\HPAToken.hpp"\
        "..\..\..\h\HPATokenizer.h"\
        "..\..\..\h\HPATokenizer.hpp"\
        "..\..\..\h\renew.h"\
        ".\TestParser.h"\


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HPATST - Win32 Debug"

DEP_CPP_MAIN_=\
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
        "..\..\..\h\HFCPtr.h"\
        "..\..\..\h\HFCPtr.hpp"\
        "..\..\..\h\HPADynamicParser.h"\
        "..\..\..\h\HPADynamicParser.hpp"\
        "..\..\..\h\HPAGrammarObject.h"\
        "..\..\..\h\HPAGrammarObject.hpp"\
        "..\..\..\h\HPANode.h"\
        "..\..\..\h\HPANode.hpp"\
        "..\..\..\h\HPAParser.h"\
        "..\..\..\h\HPAParser.hpp"\
        "..\..\..\h\HPAProduction.h"\
        "..\..\..\h\HPAProduction.hpp"\
        "..\..\..\h\HPARule.h"\
        "..\..\..\h\HPARule.hpp"\
        "..\..\..\h\HPAToken.h"\
        "..\..\..\h\HPAToken.hpp"\
        "..\..\..\h\HPATokenizer.h"\
        "..\..\..\h\HPATokenizer.hpp"\
        "..\..\..\h\renew.h"\
        ".\TestParser.h"\


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ENDIF

SOURCE=.\TestParser.cpp

!IF  "$(CFG)" == "HPATST - Win32 Release"

DEP_CPP_TESTP=\
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
        "..\..\..\h\HFCPtr.h"\
        "..\..\..\h\HFCPtr.hpp"\
        "..\..\..\h\HPAGrammarObject.h"\
        "..\..\..\h\HPAGrammarObject.hpp"\
        "..\..\..\h\HPANode.h"\
        "..\..\..\h\HPANode.hpp"\
        "..\..\..\h\HPAParser.h"\
        "..\..\..\h\HPAParser.hpp"\
        "..\..\..\h\HPAProduction.h"\
        "..\..\..\h\HPAProduction.hpp"\
        "..\..\..\h\HPARule.h"\
        "..\..\..\h\HPARule.hpp"\
        "..\..\..\h\HPAToken.h"\
        "..\..\..\h\HPAToken.hpp"\
        "..\..\..\h\HPATokenizer.h"\
        "..\..\..\h\HPATokenizer.hpp"\
        "..\..\..\h\renew.h"\
        ".\TestParser.h"\


"$(INTDIR)\TestParser.obj" : $(SOURCE) $(DEP_CPP_TESTP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HPATST - Win32 Debug"

DEP_CPP_TESTP=\
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
        "..\..\..\h\HFCPtr.h"\
        "..\..\..\h\HFCPtr.hpp"\
        "..\..\..\h\HPAGrammarObject.h"\
        "..\..\..\h\HPAGrammarObject.hpp"\
        "..\..\..\h\HPANode.h"\
        "..\..\..\h\HPANode.hpp"\
        "..\..\..\h\HPAParser.h"\
        "..\..\..\h\HPAParser.hpp"\
        "..\..\..\h\HPAProduction.h"\
        "..\..\..\h\HPAProduction.hpp"\
        "..\..\..\h\HPARule.h"\
        "..\..\..\h\HPARule.hpp"\
        "..\..\..\h\HPAToken.h"\
        "..\..\..\h\HPAToken.hpp"\
        "..\..\..\h\HPATokenizer.h"\
        "..\..\..\h\HPATokenizer.hpp"\
        "..\..\..\h\renew.h"\
        ".\TestParser.h"\


"$(INTDIR)\TestParser.obj" : $(SOURCE) $(DEP_CPP_TESTP) "$(INTDIR)"


!ENDIF


!ENDIF

