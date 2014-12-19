# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=HPMTST - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to HPMTST - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "HPMTST - Win32 Release" && "$(CFG)" != "HPMTST - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE
!MESSAGE NMAKE /f "HPMTST.mak" CFG="HPMTST - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "HPMTST - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "HPMTST - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
################################################################################
# Begin Project
# PROP Target_Last_Scanned "HPMTST - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HPMTST - Win32 Release"

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
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\HPMTST.exe"

CLEAN :
        -@erase "$(INTDIR)\main.obj"
        -@erase "$(OUTDIR)\HPMTST.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3  /O2 /EHsc /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MD /W3  /O2 /EHsc /I "\dev\h" /I "..\..\..\h" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MD /W3  /O2 /EHsc /I "\dev\h" /I "..\..\..\h" /D "WIN32" /D\
 "NDEBUG" /D "_CONSOLE" /Fp"$(INTDIR)/HPMTST.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/HPMTST.bsc"
BSC32_SBRS= \

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/HPMTST.pdb" /machine:I386 /out:"$(OUTDIR)/HPMTST.exe"
LINK32_OBJS= \
        "$(INTDIR)\main.obj" \
        "..\..\..\lib\HPM_D.lib"

"$(OUTDIR)\HPMTST.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "HPMTST - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\HPMTST.exe"

CLEAN :
        -@erase "$(INTDIR)\main.obj"
        -@erase "$(INTDIR)\vc40.idb"
        -@erase "$(INTDIR)\vc40.pdb"
        -@erase "$(OUTDIR)\HPMTST.exe"
        -@erase "$(OUTDIR)\HPMTST.ilk"
        -@erase "$(OUTDIR)\HPMTST.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm  /Zi /GS /Od /EHsc /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm  /Zi /GS /Od /EHsc /I "\dev\h" /I "..\..\..\h" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MDd /W3 /Gm  /Zi /GS /Od /EHsc /I "\dev\h" /I "..\..\..\h" /D\
 "WIN32" /D "_DEBUG" /D "_CONSOLE" /Fp"$(INTDIR)/HPMTST.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/HPMTST.bsc"
BSC32_SBRS= \

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/HPMTST.pdb" /debug /machine:I386 /out:"$(OUTDIR)/HPMTST.exe"
LINK32_OBJS= \
        "$(INTDIR)\main.obj" \
        "..\..\..\lib\HPM_D.lib"

"$(OUTDIR)\HPMTST.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<

################################################################################
# Begin Target

# Name "HPMTST - Win32 Release"
# Name "HPMTST - Win32 Debug"

!IF  "$(CFG)" == "HPMTST - Win32 Release"

!ELSEIF  "$(CFG)" == "HPMTST - Win32 Debug"

!ENDIF

################################################################################
# Begin Source File

SOURCE=.\main.cpp

!IF  "$(CFG)" == "HPMTST - Win32 Release"

DEP_CPP_MAIN_=\
        "..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\h\htypes.h"\
        "..\..\..\h\HPMArrayAdaptor.h"\
        "..\..\..\h\HPMArrayAdaptor.hpp"\
        "..\..\..\h\HPMBufferAdaptor.h"\
        "..\..\..\h\HPMBufferAdaptor.hpp"\
        "..\..\..\h\HPMClassDictionary.h"\
        "..\..\..\h\HPMClassDictionary.hpp"\
        "..\..\..\h\HPMFactory.h"\
        "..\..\..\h\HPMFactory.hpp"\
        "..\..\..\h\HPMGlobalDictionary.h"\
        "..\..\..\h\HPMNativeArrayAdaptor.h"\
        "..\..\..\h\HPMNativeArrayAdaptor.hpp"\
        "..\..\..\h\HPMObjectLog.h"\
        "..\..\..\h\HPMObjectLog.hpp"\
        "..\..\..\h\HPMObjectStore.h"\
        "..\..\..\h\HPMPersistentObject.h"\
        "..\..\..\h\HPMPersistentObject.hpp"\
        "..\..\..\h\HPMPointerAdaptor.h"\
        "..\..\..\h\HPMPointerAdaptor.hpp"\
        "..\..\..\h\HPMStoreFactory.h"\
        "..\..\..\h\HPMStoreFactory.hpp"\
        "..\..\..\h\HPMTypeInfo.h"\
        "..\..\..\h\HPMTypeInfo.hpp"\
        "..\..\..\h\HPMVirtualPtr.h"\
        "..\..\..\h\HPMVirtualPtr.hpp"\
        "\dev\h\hstdcpp.h"\


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HPMTST - Win32 Debug"

DEP_CPP_MAIN_=\
        "..\..\..\..\h\hmrconst.h"\
        "..\..\..\..\h\hmrerror.h"\
        "..\..\..\..\h\hmrmacro.h"\
        "..\..\..\..\h\hmrpltfm.h"\
        "..\..\..\..\h\hmrtypes.h"\
        "..\..\..\..\h\htypes.h"\
        "..\..\..\h\HPMArrayAdaptor.h"\
        "..\..\..\h\HPMArrayAdaptor.hpp"\
        "..\..\..\h\HPMBufferAdaptor.h"\
        "..\..\..\h\HPMBufferAdaptor.hpp"\
        "..\..\..\h\HPMClassDictionary.h"\
        "..\..\..\h\HPMClassDictionary.hpp"\
        "..\..\..\h\HPMFactory.h"\
        "..\..\..\h\HPMFactory.hpp"\
        "..\..\..\h\HPMGlobalDictionary.h"\
        "..\..\..\h\HPMNativeArrayAdaptor.h"\
        "..\..\..\h\HPMNativeArrayAdaptor.hpp"\
        "..\..\..\h\HPMObjectLog.h"\
        "..\..\..\h\HPMObjectLog.hpp"\
        "..\..\..\h\HPMObjectStore.h"\
        "..\..\..\h\HPMObjectStore.hpp"\
        "..\..\..\h\HPMPersistentObject.h"\
        "..\..\..\h\HPMPersistentObject.hpp"\
        "..\..\..\h\HPMPointerAdaptor.h"\
        "..\..\..\h\HPMPointerAdaptor.hpp"\
        "..\..\..\h\HPMStoreFactory.h"\
        "..\..\..\h\HPMStoreManipulator.h"\
        "..\..\..\h\HPMStoreManipulator.hpp"\
        "..\..\..\h\HPMTypeInfo.h"\
        "..\..\..\h\HPMTypeInfo.hpp"\
        "..\..\..\h\HPMVirtualPtr.h"\
        "..\..\..\h\HPMVirtualPtr.hpp"\
        "\dev\h\hstdcpp.h"\


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=\dev\all\lib\HPM_D.lib

!IF  "$(CFG)" == "HPMTST - Win32 Release"

!ELSEIF  "$(CFG)" == "HPMTST - Win32 Debug"

!ENDIF

# End Source File
# End Target
# End Project
################################################################################
