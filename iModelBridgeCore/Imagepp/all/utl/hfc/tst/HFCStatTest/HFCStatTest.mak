# Microsoft Developer Studio Generated NMAKE File, Based on HFCStatTest.dsp
!IF "$(CFG)" == ""
CFG=HFCStatTest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to HFCStatTest - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "HFCStatTest - Win32 Release" && "$(CFG)" !=\
 "HFCStatTest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HFCStatTest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\HFCStatTest.exe"

!ELSE

ALL : "$(OUTDIR)\HFCStatTest.exe"

!ENDIF

CLEAN :
        -@erase "$(INTDIR)\HFCStatTest.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(OUTDIR)\HFCStatTest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3  /O2 /EHsc /I "\dev\h" /I "\dev\all\h" /I "\dev\win\h" /D\
 "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"\
  /c
CPP_OBJS=.\Release/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HFCStatTest.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\HFCStatTest.pdb" /machine:I386 /out:"$(OUTDIR)\HFCStatTest.exe"\

LINK32_OBJS= \
        "$(INTDIR)\HFCStatTest.obj" \
        "..\..\..\..\ext\lib\deflate_d.lib" \
        "..\..\..\..\ext\lib\Jpeg_d.lib" \
        "..\..\..\..\ext\lib\wave_d.lib" \
        "..\..\..\..\lib\hcd_d.lib" \
        "..\..\..\..\lib\hcs_d.lib" \
        "..\..\..\..\lib\hdb_d.lib" \
        "..\..\..\..\lib\Hdp_d.lib" \
        "..\..\..\..\lib\HDS_d.lib" \
        "..\..\..\..\lib\HDSC_d.lib" \
        "..\..\..\..\lib\HFC_d.lib" \
        "..\..\..\..\lib\hfs_d.lib" \
        "..\..\..\..\lib\HGF_d.lib" \
        "..\..\..\..\lib\HGS_d.lib" \
        "..\..\..\..\lib\HIM_d.lib" \
        "..\..\..\..\lib\his_d.lib" \
        "..\..\..\..\lib\hle_d.lib" \
        "..\..\..\..\lib\HLM_d.lib" \
        "..\..\..\..\lib\HMG_d.lib" \
        "..\..\..\..\lib\HMRLicense_d.lib" \
        "..\..\..\..\lib\HOD_d.lib" \
        "..\..\..\..\lib\hot_d.lib" \
        "..\..\..\..\lib\HPA_D.lib" \
        "..\..\..\..\lib\HPM_d.lib" \
        "..\..\..\..\lib\HPS_D.lib" \
        "..\..\..\..\lib\HRA_d.lib" \
        "..\..\..\..\lib\HRF_d.lib" \
        "..\..\..\..\lib\HRFInternetImaging_d.lib" \
        "..\..\..\..\lib\HRP_d.lib" \
        "..\..\..\..\lib\hru_d.lib" \
        "..\..\..\..\lib\htiff_d.lib" \
        "..\..\..\..\lib\HUT_d.lib" \
        "..\..\..\..\lib\hve_d.lib" \
        "..\..\..\..\obj\dbg\HRFInternetImagingFile.obj"

"$(OUTDIR)\HFCStatTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "HFCStatTest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\HFCStatTest.exe"

!ELSE

ALL : "$(OUTDIR)\HFCStatTest.exe"

!ENDIF

CLEAN :
        -@erase "$(INTDIR)\HFCStatTest.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(INTDIR)\vc50.pdb"
        -@erase "$(OUTDIR)\HFCStatTest.exe"
        -@erase "$(OUTDIR)\HFCStatTest.ilk"
        -@erase "$(OUTDIR)\HFCStatTest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm  /Zi /GS /Od /EHsc /I "\dev\h" /I "\dev\all\h" /I\
 "\dev\win\h" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\"  /c
CPP_OBJS=.\Debug/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HFCStatTest.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib wsock32.lib ws2_32.lib wininet.lib /nologo /subsystem:console\
 /incremental:yes /pdb:"$(OUTDIR)\HFCStatTest.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\HFCStatTest.exe" /pdbtype:sept
LINK32_OBJS= \
        "$(INTDIR)\HFCStatTest.obj" \
        "..\..\..\..\ext\lib\deflate_d.lib" \
        "..\..\..\..\ext\lib\Jpeg_d.lib" \
        "..\..\..\..\ext\lib\wave_d.lib" \
        "..\..\..\..\lib\hcd_d.lib" \
        "..\..\..\..\lib\hcs_d.lib" \
        "..\..\..\..\lib\hdb_d.lib" \
        "..\..\..\..\lib\Hdp_d.lib" \
        "..\..\..\..\lib\HDS_d.lib" \
        "..\..\..\..\lib\HDSC_d.lib" \
        "..\..\..\..\lib\HFC_d.lib" \
        "..\..\..\..\lib\hfs_d.lib" \
        "..\..\..\..\lib\HGF_d.lib" \
        "..\..\..\..\lib\HGS_d.lib" \
        "..\..\..\..\lib\HIM_d.lib" \
        "..\..\..\..\lib\his_d.lib" \
        "..\..\..\..\lib\hle_d.lib" \
        "..\..\..\..\lib\HLM_d.lib" \
        "..\..\..\..\lib\HMG_d.lib" \
        "..\..\..\..\lib\HMRLicense_d.lib" \
        "..\..\..\..\lib\HOD_d.lib" \
        "..\..\..\..\lib\hot_d.lib" \
        "..\..\..\..\lib\HPA_D.lib" \
        "..\..\..\..\lib\HPM_d.lib" \
        "..\..\..\..\lib\HPS_D.lib" \
        "..\..\..\..\lib\HRA_d.lib" \
        "..\..\..\..\lib\HRF_d.lib" \
        "..\..\..\..\lib\HRFInternetImaging_d.lib" \
        "..\..\..\..\lib\HRP_d.lib" \
        "..\..\..\..\lib\hru_d.lib" \
        "..\..\..\..\lib\htiff_d.lib" \
        "..\..\..\..\lib\HUT_d.lib" \
        "..\..\..\..\lib\hve_d.lib" \
        "..\..\..\..\obj\dbg\HRFInternetImagingFile.obj"

"$(OUTDIR)\HFCStatTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "HFCStatTest - Win32 Release" || "$(CFG)" ==\
 "HFCStatTest - Win32 Debug"
SOURCE=.\HFCStatTest.cpp
DEP_CPP_HFCST=\
        "..\..\..\..\..\h\HArrayAutoPtr.h"\
        "..\..\..\..\..\h\HArrayAutoPtr.hpp"\
        "..\..\..\..\..\h\HAutoPtr.h"\
        "..\..\..\..\..\h\HAutoPtr.hpp"\
        "..\..\..\..\..\h\HmrConst.h"\
        "..\..\..\..\..\h\HmrError.h"\
        "..\..\..\..\..\h\HmrMacro.h"\
        "..\..\..\..\..\h\HmrPltfm.h"\
        "..\..\..\..\..\h\HmrTypes.h"\
        "..\..\..\..\..\h\hstdcpp.h"\
        "..\..\..\..\..\h\HTypes.h"\
        "..\..\..\..\h\HFCAccessMode.h"\
        "..\..\..\..\h\HFCAccessMode.hpp"\
        "..\..\..\..\h\HFCExclusiveKey.h"\
        "..\..\..\..\h\HFCExclusiveKey.hpp"\
        "..\..\..\..\h\HFCPtr.h"\
        "..\..\..\..\h\HFCPtr.hpp"\
        "..\..\..\..\h\HFCStat.h"\
        "..\..\..\..\h\HFCStat.hpp"\
        "..\..\..\..\h\HFCURL.h"\
        "..\..\..\..\h\HFCURL.hpp"\
        "..\..\..\..\h\HFCURLFile.h"\
        "..\..\..\..\h\HFCURLFile.hpp"\
        "..\..\..\..\h\HFCVersion.h"\
        "..\..\..\..\h\HFCVersion.hpp"\
        "..\..\..\..\h\renew.h"\


"$(INTDIR)\HFCStatTest.obj" : $(SOURCE) $(DEP_CPP_HFCST) "$(INTDIR)"



!ENDIF

