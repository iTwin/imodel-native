# Microsoft Developer Studio Generated NMAKE File, Based on HRFCloning.dsp
!IF "$(CFG)" == ""
CFG=HRFCloning - Win32 Debug
!MESSAGE No configuration specified. Defaulting to HRFCloning - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "HRFCloning - Win32 Release" && "$(CFG)" !=\
 "HRFCloning - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

OUTDIR=.\HRFCloni
INTDIR=.\HRFCloni
# Begin Custom Macros
OutDir=.\HRFCloni
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\HRFCloning.exe"

!ELSE

ALL : "$(OUTDIR)\HRFCloning.exe"

!ENDIF

CLEAN :
        -@erase "$(INTDIR)\HRFCloning.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(OUTDIR)\HRFCloning.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3  /O2 /EHsc /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp"$(INTDIR)\HRFCloning.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" \
 /c
CPP_OBJS=.\HRFCloni/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HRFCloning.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\HRFCloning.pdb" /machine:I386 /out:"$(OUTDIR)\HRFCloning.exe"
LINK32_OBJS= \
        "$(INTDIR)\HRFCloning.obj" \
        "..\..\..\ext\lib\deflate.lib" \
        "..\..\..\ext\lib\HMRCustomFPX.lib" \
        "..\..\..\ext\lib\htiff.lib" \
        "..\..\..\ext\lib\Jpeg.lib" \
        "..\..\..\ext\lib\LPIFpxLib.lib" \
        "..\..\..\ext\lib\LPIJpegLib.lib" \
        "..\..\..\ext\lib\png.lib" \
        "..\..\..\ext\lib\wave.lib" \
        "..\..\..\lib\hcd.lib" \
        "..\..\..\lib\hcs.lib" \
        "..\..\..\lib\HDS.lib" \
        "..\..\..\lib\HFC.lib" \
        "..\..\..\lib\HGF.lib" \
        "..\..\..\lib\HIM.lib" \
        "..\..\..\lib\HMG.lib" \
        "..\..\..\lib\HOD.lib" \
        "..\..\..\lib\hot.lib" \
        "..\..\..\lib\HPM.lib" \
        "..\..\..\lib\HRA.lib" \
        "..\..\..\lib\HRF.lib" \
        "..\..\..\lib\HRFInternetImaging.lib" \
        "..\..\..\lib\HRP.lib" \
        "..\..\..\lib\hru_d.lib" \
        "..\..\..\lib\HUC.lib" \
        "..\..\..\lib\HUT.lib" \
        "..\..\..\lib\hve.lib" \
        "..\..\..\obj\dbg\hstdcpp.obj" \
        "..\..\..\obj\rel\hstdcpp.obj" \
        "..\..\..\obj\rel\HUCURLHTTP.obj"

"$(OUTDIR)\HRFCloning.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

OUTDIR=.\HRFClon0
INTDIR=.\HRFClon0
# Begin Custom Macros
OutDir=.\HRFClon0
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\HRFCloning.exe"

!ELSE

ALL : "$(OUTDIR)\HRFCloning.exe"

!ENDIF

CLEAN :
        -@erase "$(INTDIR)\HRFCloning.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(INTDIR)\vc50.pdb"
        -@erase "$(OUTDIR)\HRFCloning.exe"
        -@erase "$(OUTDIR)\HRFCloning.ilk"
        -@erase "$(OUTDIR)\HRFCloning.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm  /Zi /GS /Od /EHsc /I "\dev\h" /I "\dev\all\h" /I\
 "\dev\all\ext\hmr\h" /I "\dev\win\h" /I "\dev\all\ext\tiff\h" /I\
 "\dev\all\ext\flashpix\jpeg" /I "\dev\all\ext\oracle\h" /I\
 "\dev\all\ext\wavelet\h" /I "\dev\all\ext\htiff\h" /I "\dev\all\ext\jpeg\h" /I\
 "\dev\all\ext\png\h" /I "\dev\all\ext\deflate\h" /D "WIN32" /D "_DEBUG" /D\
 "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\HRFCloning.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\"  /c
CPP_OBJS=.\HRFClon0/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HRFCloning.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\HRFCloning.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\HRFCloning.exe" /pdbtype:sept
LINK32_OBJS= \
        "$(INTDIR)\HRFCloning.obj" \
        "..\..\..\ext\lib\deflate_d.lib" \
        "..\..\..\ext\lib\HMRCustomFPX_d.lib" \
        "..\..\..\ext\lib\htiff_d.lib" \
        "..\..\..\ext\lib\Jpeg_d.lib" \
        "..\..\..\ext\lib\LPIFpxLib_d.lib" \
        "..\..\..\ext\lib\LPIJpegLib_d.lib" \
        "..\..\..\ext\lib\png_d.lib" \
        "..\..\..\ext\lib\wave_d.lib" \
        "..\..\..\lib\hcd_d.lib" \
        "..\..\..\lib\hcs_d.lib" \
        "..\..\..\lib\Hdp_d.lib" \
        "..\..\..\lib\HDS_d.lib" \
        "..\..\..\lib\HDSC_d.lib" \
        "..\..\..\lib\HFC_d.lib" \
        "..\..\..\lib\HGF_d.lib" \
        "..\..\..\lib\HIM_d.lib" \
        "..\..\..\lib\his_d.lib" \
        "..\..\..\lib\HMG_d.lib" \
        "..\..\..\lib\HOD_d.lib" \
        "..\..\..\lib\hot_d.lib" \
        "..\..\..\lib\HPM_d.lib" \
        "..\..\..\lib\HRA_d.lib" \
        "..\..\..\lib\HRF_d.lib" \
        "..\..\..\lib\HRFInternetImaging_d.lib" \
        "..\..\..\lib\HRP_d.lib" \
        "..\..\..\lib\hru_d.lib" \
        "..\..\..\lib\HUC_D.lib" \
        "..\..\..\lib\HUT_d.lib" \
        "..\..\..\lib\hve_d.lib" \
        "..\..\..\obj\dbg\hstdcpp.obj" \
        "..\..\..\obj\dbg\HUCURLHTTP.obj"

"$(OUTDIR)\HRFCloning.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "HRFCloning - Win32 Release" || "$(CFG)" ==\
 "HRFCloning - Win32 Debug"
SOURCE=.\HRFCloning.cpp

!IF  "$(CFG)" == "HRFCloning - Win32 Release"

NODEP_CPP_HRFCL=\
        ".\HFCURL.h"\
        ".\HFCURLFile.h"\
        ".\HRFBmpFile.h"\
        ".\HRFCalsFile.h"\
        ".\HRFGeoTiffFile.h"\
        ".\HRFGifFile.h"\
        ".\HRFHMRFile.h"\
        ".\HRFIntergraphCITFile.h"\
        ".\HRFIntergraphCOT29File.h"\
        ".\HRFIntergraphCotFile.h"\
        ".\HRFIntergraphRGBFile.h"\
        ".\HRFIntergraphRLEFile.h"\
        ".\HRFIntergraphTG4File.h"\
        ".\HRFiTiffFile.h"\
        ".\HRFJpegFile.h"\
        ".\HRFPngFile.h"\
        ".\HRFRasterFile.h"\
        ".\HRFRasterFileCapabilities.h"\
        ".\HRFRasterFileFactory.h"\
        ".\HRFResolutionEditor.h"\
        ".\HRFTiffFile.h"\
        ".\HRUPRJFile.h"\
        ".\hstdcpp.h"\


"$(INTDIR)\HRFCloning.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HRFCloning - Win32 Debug"

DEP_CPP_HRFCL=\
        "..\..\..\..\h\hstdcpp.h"\
        "..\..\..\h\HFCURL.h"\
        "..\..\..\h\HFCURLFile.h"\
        "..\..\..\h\HRFBmpFile.h"\
        "..\..\..\h\HRFCalsFile.h"\
        "..\..\..\h\HRFGeoTiffFile.h"\
        "..\..\..\h\HRFGifFile.h"\
        "..\..\..\h\HRFHMRFile.h"\
        "..\..\..\h\HRFIntergraphCITFile.h"\
        "..\..\..\h\HRFIntergraphCOT29File.h"\
        "..\..\..\h\HRFIntergraphCotFile.h"\
        "..\..\..\h\HRFIntergraphRGBFile.h"\
        "..\..\..\h\HRFIntergraphRLEFile.h"\
        "..\..\..\h\HRFIntergraphTG4File.h"\
        "..\..\..\h\HRFiTiffFile.h"\
        "..\..\..\h\HRFJpegFile.h"\
        "..\..\..\h\HRFPngFile.h"\
        "..\..\..\h\HRFRasterFile.h"\
        "..\..\..\h\HRFRasterFileCapabilities.h"\
        "..\..\..\h\HRFRasterFileFactory.h"\
        "..\..\..\h\HRFResolutionEditor.h"\
        "..\..\..\h\HRFTiffFile.h"\
        "..\..\..\h\HRUPRJFile.h"\


"$(INTDIR)\HRFCloning.obj" : $(SOURCE) $(DEP_CPP_HRFCL) "$(INTDIR)"


!ENDIF


!ENDIF

