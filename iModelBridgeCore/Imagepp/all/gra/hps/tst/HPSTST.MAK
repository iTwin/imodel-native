# Microsoft Developer Studio Generated NMAKE File, Based on HPSTST.DSP
!IF "$(CFG)" == ""
CFG=HPSTST - Win32 Debug
!MESSAGE No configuration specified. Defaulting to HPSTST - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "HPSTST - Win32 Release" && "$(CFG)" != "HPSTST - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "HPSTST.MAK" CFG="HPSTST - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "HPSTST - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "HPSTST - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "HPSTST - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\HPSTST.exe"

!ELSE

ALL : "HRFInternetImaging - Win32 Release" "him - Win32 Release"\
 "whfc - Win32 Release" "winhcs - Win32 Release" "HCS50 - Win32 Release"\
 "hve - Win32 Release" "hcd50 - Win32 Release" "HOD - Win32 Release"\
 "winhra - Win32 Release" "HUC - Win32 Release" "HRP - Win32 Release"\
 "hrf - Win32 Release" "HRA - Win32 Release" "HPS - Win32 Release"\
 "HPM - Win32 Release" "HPA - Win32 Release" "HGF - Win32 Release"\
 "hfc - Win32 Release" "$(OUTDIR)\HPSTST.exe"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"hfc - Win32 ReleaseCLEAN" "HGF - Win32 ReleaseCLEAN"\
 "HPA - Win32 ReleaseCLEAN" "HPM - Win32 ReleaseCLEAN"\
 "HPS - Win32 ReleaseCLEAN" "HRA - Win32 ReleaseCLEAN"\
 "hrf - Win32 ReleaseCLEAN" "HRP - Win32 ReleaseCLEAN"\
 "HUC - Win32 ReleaseCLEAN" "winhra - Win32 ReleaseCLEAN"\
 "HOD - Win32 ReleaseCLEAN" "hcd50 - Win32 ReleaseCLEAN"\
 "hve - Win32 ReleaseCLEAN" "HCS50 - Win32 ReleaseCLEAN"\
 "winhcs - Win32 ReleaseCLEAN" "whfc - Win32 ReleaseCLEAN"\
 "him - Win32 ReleaseCLEAN" "HRFInternetImaging - Win32 ReleaseCLEAN"
!ELSE
CLEAN :
!ENDIF
        -@erase "$(INTDIR)\main.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(OUTDIR)\HPSTST.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3  /O2 /EHsc /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp"$(INTDIR)\HPSTST.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c
CPP_OBJS=.\Release/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HPSTST.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\HPSTST.pdb" /machine:I386 /out:"$(OUTDIR)\HPSTST.exe"
LINK32_OBJS= \
        "$(INTDIR)\main.obj" \
        "..\..\..\..\win\lib\winhcs.lib" \
        "..\..\..\..\win\lib\winhcs_d.lib" \
        "..\..\..\..\win\lib\winhfc.lib" \
        "..\..\..\..\win\lib\winhfc_d.lib" \
        "..\..\..\..\win\lib\WINHRA.lib" \
        "..\..\..\..\win\lib\WINHRA_d.lib" \
        "..\..\..\ext\lib\deflate_d.lib" \
        "..\..\..\ext\lib\HMRCustomFPX_d.lib" \
        "..\..\..\ext\lib\htiff_d.lib" \
        "..\..\..\ext\lib\Jpeg_d.lib" \
        "..\..\..\ext\lib\LPIFpxLib_d.lib" \
        "..\..\..\ext\lib\LPIJpegLib_d.lib" \
        "..\..\..\ext\lib\png_d.lib" \
        "..\..\..\ext\lib\wave_d.lib" \
        "..\..\..\lib\hcd.lib" \
        "..\..\..\lib\hcd_d.lib" \
        "..\..\..\lib\hcs.lib" \
        "..\..\..\lib\hcs_d.lib" \
        "..\..\..\lib\HFC.lib" \
        "..\..\..\lib\HFC_d.lib" \
        "..\..\..\lib\HGF.lib" \
        "..\..\..\lib\HGF_d.lib" \
        "..\..\..\lib\HIM.lib" \
        "..\..\..\lib\HIM_d.lib" \
        "..\..\..\lib\HMG_d.lib" \
        "..\..\..\lib\HOD.lib" \
        "..\..\..\lib\HOD_d.lib" \
        "..\..\..\lib\HPA.lib" \
        "..\..\..\lib\HPA_D.lib" \
        "..\..\..\lib\HPM.lib" \
        "..\..\..\lib\HPM_d.lib" \
        "..\..\..\lib\HPS.lib" \
        "..\..\..\lib\HPS_D.lib" \
        "..\..\..\lib\HRA.lib" \
        "..\..\..\lib\HRA_d.lib" \
        "..\..\..\lib\HRF.lib" \
        "..\..\..\lib\HRF_d.lib" \
        "..\..\..\lib\HRFInternetImaging.lib" \
        "..\..\..\lib\HRFInternetImaging_d.lib" \
        "..\..\..\lib\HRP.lib" \
        "..\..\..\lib\HRP_d.lib" \
        "..\..\..\lib\Huc.lib" \
        "..\..\..\lib\HUC_D.lib" \
        "..\..\..\lib\hve.lib" \
        "..\..\..\lib\hve_d.lib" \
        "..\..\..\obj\dbg\HRFCalsFile.obj" \
        "..\..\..\obj\dbg\HRFHMR2File.obj" \
        "..\..\..\obj\dbg\HRFHMRFile.obj" \
        "..\..\..\obj\dbg\HRFIntergraphCITFile.obj" \
        "..\..\..\obj\dbg\HRFIntergraphCOT29File.obj" \
        "..\..\..\obj\dbg\HRFIntergraphCotFile.obj" \
        "..\..\..\obj\dbg\HRFIntergraphFile.obj" \
        "..\..\..\obj\dbg\HRFIntergraphRLEFile.obj" \
        "..\..\..\obj\dbg\HRFIntergraphTG4File.obj" \
        "..\..\..\obj\dbg\HRFInternetImagingFile.obj" \
        "..\..\..\obj\dbg\HRFJpegFile.obj" \
        "..\..\..\obj\dbg\HRFLocalRasterFile.obj" \
        "..\..\..\obj\dbg\HRFRasterFile.obj" \
        "..\..\..\obj\dbg\HRFTiffFile.obj"

"$(OUTDIR)\HPSTST.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\HPSTST.exe"

!ELSE

ALL : "HRFInternetImaging - Win32 Debug" "him - Win32 Debug"\
 "whfc - Win32 Debug" "winhcs - Win32 Debug" "HCS50 - Win32 Debug"\
 "hve - Win32 Debug" "hcd50 - Win32 Debug" "HOD - Win32 Debug"\
 "winhra - Win32 Debug" "HUC - Win32 Debug" "HRP - Win32 Debug"\
 "hrf - Win32 Debug" "HRA - Win32 Debug" "HPS - Win32 Debug" "HPM - Win32 Debug"\
 "HPA - Win32 Debug" "HGF - Win32 Debug" "hfc - Win32 Debug"\
 "$(OUTDIR)\HPSTST.exe"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"hfc - Win32 DebugCLEAN" "HGF - Win32 DebugCLEAN"\
 "HPA - Win32 DebugCLEAN" "HPM - Win32 DebugCLEAN" "HPS - Win32 DebugCLEAN"\
 "HRA - Win32 DebugCLEAN" "hrf - Win32 DebugCLEAN" "HRP - Win32 DebugCLEAN"\
 "HUC - Win32 DebugCLEAN" "winhra - Win32 DebugCLEAN" "HOD - Win32 DebugCLEAN"\
 "hcd50 - Win32 DebugCLEAN" "hve - Win32 DebugCLEAN" "HCS50 - Win32 DebugCLEAN"\
 "winhcs - Win32 DebugCLEAN" "whfc - Win32 DebugCLEAN" "him - Win32 DebugCLEAN"\
 "HRFInternetImaging - Win32 DebugCLEAN"
!ELSE
CLEAN :
!ENDIF
        -@erase "$(INTDIR)\main.obj"
        -@erase "$(INTDIR)\vc50.idb"
        -@erase "$(INTDIR)\vc50.pdb"
        -@erase "$(OUTDIR)\HPSTST.exe"
        -@erase "$(OUTDIR)\HPSTST.ilk"
        -@erase "$(OUTDIR)\HPSTST.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm  /Zi /GS /Od /EHsc /I "\dev\h" /I "\dev\all\h" /D\
 "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"\
  /c
CPP_OBJS=.\Debug/
CPP_SBRS=.
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HPSTST.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=\dev\all\obj\dbg\hstdcpp.obj \dev\win\obj\dbg\whstdcpp.obj\
 kernel32.lib user32.lib ole32.lib wininet.lib wsock32.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\HPSTST.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)\HPSTST.exe" /pdbtype:sept
LINK32_OBJS= \
        "$(INTDIR)\main.obj" \
        "..\..\..\..\win\lib\winhcs_d.lib" \
        "..\..\..\..\win\lib\winhfc_d.lib" \
        "..\..\..\..\win\lib\WINHRA_d.lib" \
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
        "..\..\..\lib\HFC_d.lib" \
        "..\..\..\lib\HGF_d.lib" \
        "..\..\..\lib\HIM_d.lib" \
        "..\..\..\lib\HMG_d.lib" \
        "..\..\..\lib\HOD_d.lib" \
        "..\..\..\lib\HPA_D.lib" \
        "..\..\..\lib\HPM_d.lib" \
        "..\..\..\lib\HPS_D.lib" \
        "..\..\..\lib\HRA_d.lib" \
        "..\..\..\lib\HRF_d.lib" \
        "..\..\..\lib\HRFInternetImaging_d.lib" \
        "..\..\..\lib\HRP_d.lib" \
        "..\..\..\lib\HUC_D.lib" \
        "..\..\..\lib\hve_d.lib" \
        "..\..\..\obj\dbg\HRFCalsFile.obj" \
        "..\..\..\obj\dbg\HRFHMR2File.obj" \
        "..\..\..\obj\dbg\HRFHMRFile.obj" \
        "..\..\..\obj\dbg\HRFIntergraphCITFile.obj" \
        "..\..\..\obj\dbg\HRFIntergraphCOT29File.obj" \
        "..\..\..\obj\dbg\HRFIntergraphCotFile.obj" \
        "..\..\..\obj\dbg\HRFIntergraphFile.obj" \
        "..\..\..\obj\dbg\HRFIntergraphRLEFile.obj" \
        "..\..\..\obj\dbg\HRFIntergraphTG4File.obj" \
        "..\..\..\obj\dbg\HRFJpegFile.obj" \
        "..\..\..\obj\dbg\HRFLocalRasterFile.obj" \
        "..\..\..\obj\dbg\HRFRasterFile.obj" \
        "..\..\..\obj\dbg\HRFTiffFile.obj"

"$(OUTDIR)\HPSTST.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "HPSTST - Win32 Release" || "$(CFG)" == "HPSTST - Win32 Debug"
SOURCE=.\main.cpp

!IF  "$(CFG)" == "HPSTST - Win32 Release"

NODEP_CPP_MAIN_=\
        ".\HFCPtr.h"\
        ".\HRARaster.h"\


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

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
        "..\..\..\h\HFCAccessMode.h"\
        "..\..\..\h\hfcaccessmode.hpp"\
        "..\..\..\h\HFCExclusiveKey.h"\
        "..\..\..\h\HFCExclusiveKey.hpp"\
        "..\..\..\h\hfcmatrix.h"\
        "..\..\..\h\hfcmatrix.hpp"\
        "..\..\..\h\HFCPtr.h"\
        "..\..\..\h\HFCPtr.hpp"\
        "..\..\..\h\HGF2DCoordSys.h"\
        "..\..\..\h\HGF2DCoordSys.hpp"\
        "..\..\..\h\HGF2DDisplacement.h"\
        "..\..\..\h\HGF2DDisplacement.hpp"\
        "..\..\..\h\HGF2DExtent.h"\
        "..\..\..\h\HGF2DExtent.hpp"\
        "..\..\..\h\HGF2DGridScanLines.h"\
        "..\..\..\h\HGF2DGridScanLines.hpp"\
        "..\..\..\h\HGF2DLocation.h"\
        "..\..\..\h\HGF2DLocation.hpp"\
        "..\..\..\h\HGF2DPosition.h"\
        "..\..\..\h\HGF2DPosition.hpp"\
        "..\..\..\h\HGF2DTransfoModel.h"\
        "..\..\..\h\HGF2DTransfoModel.hpp"\
        "..\..\..\h\HGF2DWorld.h"\
        "..\..\..\h\hgf2dworld.hpp"\
        "..\..\..\h\hgf2dworldcluster.h"\
        "..\..\..\h\hgf2dworldcluster.hpp"\
        "..\..\..\h\HGF_Dll.h"\
        "..\..\..\h\HGFAngle.h"\
        "..\..\..\h\HGFAngle.hpp"\
        "..\..\..\h\HGFAngularAcceleration.h"\
        "..\..\..\h\HGFAngularAcceleration.hpp"\
        "..\..\..\h\HGFAngularAccelerationUnit.h"\
        "..\..\..\h\HGFAngularAccelerationUnit.hpp"\
        "..\..\..\h\HGFAngularUnit.h"\
        "..\..\..\h\HGFAngularUnit.hpp"\
        "..\..\..\h\HGFArea.h"\
        "..\..\..\h\HGFArea.hpp"\
        "..\..\..\h\HGFAreaUnit.h"\
        "..\..\..\h\HGFAreaUnit.hpp"\
        "..\..\..\h\HGFBearing.h"\
        "..\..\..\h\HGFBearing.hpp"\
        "..\..\..\h\HGFDistance.h"\
        "..\..\..\h\HGFDistance.hpp"\
        "..\..\..\h\HGFDistanceUnit.h"\
        "..\..\..\h\HGFDistanceUnit.hpp"\
        "..\..\..\h\HGFGraphicObject.h"\
        "..\..\..\h\HGFGraphicObject.hpp"\
        "..\..\..\h\hgfhmrstdworldcluster.h"\
        "..\..\..\h\HGFMessages.h"\
        "..\..\..\h\HGFMessages.hpp"\
        "..\..\..\h\HGFRaster.h"\
        "..\..\..\h\hgfscanline.h"\
        "..\..\..\h\hgfscanline.hpp"\
        "..\..\..\h\HMGMacros.h"\
        "..\..\..\h\HMGMessage.h"\
        "..\..\..\h\HMGMessage.hpp"\
        "..\..\..\h\HMGMessageDuplex.h"\
        "..\..\..\h\HMGMessageDuplex.hpp"\
        "..\..\..\h\HMGMessageReceiver.h"\
        "..\..\..\h\HMGMessageReceiver.hpp"\
        "..\..\..\h\HMGMessageSender.h"\
        "..\..\..\h\HMGMessageSender.hpp"\
        "..\..\..\h\HMGSink.h"\
        "..\..\..\h\HMGSink.hpp"\
        "..\..\..\h\HMGThread.h"\
        "..\..\..\h\HMGThread.hpp"\
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
        "..\..\..\h\HPM_Dll.h"\
        "..\..\..\h\HPMArrayAdaptor.h"\
        "..\..\..\h\HPMArrayAdaptor.hpp"\
        "..\..\..\h\HPMBufferAdaptor.h"\
        "..\..\..\h\HPMBufferAdaptor.hpp"\
        "..\..\..\h\HPMClassDictionary.h"\
        "..\..\..\h\HPMClassDictionary.hpp"\
        "..\..\..\h\HPMClassFilter.h"\
        "..\..\..\h\HPMClassFilter.hpp"\
        "..\..\..\h\HPMFactory.h"\
        "..\..\..\h\HPMFactory.hpp"\
        "..\..\..\h\HPMGlobalDictionary.h"\
        "..\..\..\h\HPMMacros.h"\
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
        "..\..\..\h\HPMSTLAssociativeArrayAdaptor.h"\
        "..\..\..\h\HPMSTLAssociativeArrayAdaptor.hpp"\
        "..\..\..\h\HPMSTLSequenceAdaptor.h"\
        "..\..\..\h\HPMSTLSequenceAdaptor.hpp"\
        "..\..\..\h\HPMStoreDescriptor.h"\
        "..\..\..\h\HPMStoreDescriptor.hpp"\
        "..\..\..\h\HPMStoreReader.h"\
        "..\..\..\h\HPMStringAdaptor.h"\
        "..\..\..\h\HPMStringAdaptor.hpp"\
        "..\..\..\h\HPMTypeInfo.h"\
        "..\..\..\h\HPMTypeInfo.hpp"\
        "..\..\..\h\HPMVirtualPtr.h"\
        "..\..\..\h\HPMVirtualPtr.hpp"\
        "..\..\..\h\HPSObjectStore.h"\
        "..\..\..\h\HPSObjectStore.hpp"\
        "..\..\..\h\HPSParser.h"\
        "..\..\..\h\HPSParser.hpp"\
        "..\..\..\h\HRACopyFromOptions.h"\
        "..\..\..\h\HRACopyFromOptions.hpp"\
        "..\..\..\h\HRAMessages.h"\
        "..\..\..\h\HRAMessages.hpp"\
        "..\..\..\h\HRARaster.h"\
        "..\..\..\h\HRARaster.hpp"\
        "..\..\..\h\hrautils.h"\
        "..\..\..\h\HRPChannelOrg.h"\
        "..\..\..\h\HRPChannelOrg.hpp"\
        "..\..\..\h\HRPChannelType.h"\
        "..\..\..\h\HRPChannelType.hpp"\
        "..\..\..\h\HRPFilter.h"\
        "..\..\..\h\HRPFilter.hpp"\
        "..\..\..\h\HRPMessages.h"\
        "..\..\..\h\HRPMessages.hpp"\
        "..\..\..\h\HRPPixelBuffer.h"\
        "..\..\..\h\HRPPixelBuffer.hpp"\
        "..\..\..\h\HRPPixelConverter.h"\
        "..\..\..\h\HRPPixelConverter.hpp"\
        "..\..\..\h\HRPPixelNeighbourhood.h"\
        "..\..\..\h\HRPPixelNeighbourhood.hpp"\
        "..\..\..\h\HRPPixelPalette.h"\
        "..\..\..\h\HRPPixelPalette.hpp"\
        "..\..\..\h\HRPPixelType.h"\
        "..\..\..\h\HRPPixelType.hpp"\
        "..\..\..\h\HRPPixelTypeFingerPrint.h"\
        "..\..\..\h\HRPPixelTypeFingerPrint.hpp"\
        "..\..\..\h\HRPQuantizedPalette.h"\
        "..\..\..\h\HRPQuantizedPalette.hpp"\
        "..\..\..\h\HUCBlockStream.h"\
        "..\..\..\h\HUCBlockStream.hpp"\
        "..\..\..\h\HUCLocalBlockStream.h"\
        "..\..\..\h\HUCLocalBlockStream.hpp"\
        "..\..\..\h\HUCStreamBlockEditor.h"\
        "..\..\..\h\HUCStreamBlockEditor.hpp"\
        "..\..\..\h\HUCURL.h"\
        "..\..\..\h\HUCURL.hpp"\
        "..\..\..\h\HUCURLFile.h"\
        "..\..\..\h\HUCURLFile.hpp"\
        "..\..\..\h\hve2dshape.h"\
        "..\..\..\h\HVE2DShape.hpp"\
        "..\..\..\h\hve2dvector.h"\
        "..\..\..\h\HVE2DVector.hpp"\
        "..\..\..\h\HVEShape.h"\
        "..\..\..\h\HVEShape.hpp"\
        "..\..\..\h\renew.h"\


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"hfc - Win32 Release" :
   cd "\dev\all\utl\hfc\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hfc50.mak CFG="hfc - Win32 Release"
   cd "..\..\..\gra\hps\tst"

"hfc - Win32 ReleaseCLEAN" :
   cd "\dev\all\utl\hfc\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hfc50.mak CFG="hfc - Win32 Release"\
 RECURSE=1
   cd "..\..\..\gra\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"hfc - Win32 Debug" :
   cd "\dev\all\utl\hfc\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hfc50.mak CFG="hfc - Win32 Debug"
   cd "..\..\..\gra\hps\tst"

"hfc - Win32 DebugCLEAN" :
   cd "\dev\all\utl\hfc\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hfc50.mak CFG="hfc - Win32 Debug" RECURSE=1\

   cd "..\..\..\gra\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"HGF - Win32 Release" :
   cd "\dev\all\gra\hgf\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hgf50.mak CFG="HGF - Win32 Release"
   cd "..\..\hps\tst"

"HGF - Win32 ReleaseCLEAN" :
   cd "\dev\all\gra\hgf\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hgf50.mak CFG="HGF - Win32 Release"\
 RECURSE=1
   cd "..\..\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"HGF - Win32 Debug" :
   cd "\dev\all\gra\hgf\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hgf50.mak CFG="HGF - Win32 Debug"
   cd "..\..\hps\tst"

"HGF - Win32 DebugCLEAN" :
   cd "\dev\all\gra\hgf\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hgf50.mak CFG="HGF - Win32 Debug" RECURSE=1\

   cd "..\..\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"HPA - Win32 Release" :
   cd "\dev\all\utl\hpa\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HPA.MAK CFG="HPA - Win32 Release"
   cd "..\..\..\gra\hps\tst"

"HPA - Win32 ReleaseCLEAN" :
   cd "\dev\all\utl\hpa\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HPA.MAK CFG="HPA - Win32 Release" RECURSE=1\

   cd "..\..\..\gra\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"HPA - Win32 Debug" :
   cd "\dev\all\utl\hpa\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HPA.MAK CFG="HPA - Win32 Debug"
   cd "..\..\..\gra\hps\tst"

"HPA - Win32 DebugCLEAN" :
   cd "\dev\all\utl\hpa\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HPA.MAK CFG="HPA - Win32 Debug" RECURSE=1
   cd "..\..\..\gra\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"HPM - Win32 Release" :
   cd "\dev\all\utl\hpm\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HPM50.mak CFG="HPM - Win32 Release"
   cd "..\..\..\gra\hps\tst"

"HPM - Win32 ReleaseCLEAN" :
   cd "\dev\all\utl\hpm\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HPM50.mak CFG="HPM - Win32 Release"\
 RECURSE=1
   cd "..\..\..\gra\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"HPM - Win32 Debug" :
   cd "\dev\all\utl\hpm\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HPM50.mak CFG="HPM - Win32 Debug"
   cd "..\..\..\gra\hps\tst"

"HPM - Win32 DebugCLEAN" :
   cd "\dev\all\utl\hpm\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HPM50.mak CFG="HPM - Win32 Debug" RECURSE=1\

   cd "..\..\..\gra\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"HPS - Win32 Release" :
   cd "\dev\all\gra\hps\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HPS.MAK CFG="HPS - Win32 Release"
   cd "..\tst"

"HPS - Win32 ReleaseCLEAN" :
   cd "\dev\all\gra\hps\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HPS.MAK CFG="HPS - Win32 Release" RECURSE=1\

   cd "..\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"HPS - Win32 Debug" :
   cd "\dev\all\gra\hps\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HPS.MAK CFG="HPS - Win32 Debug"
   cd "..\tst"

"HPS - Win32 DebugCLEAN" :
   cd "\dev\all\gra\hps\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HPS.MAK CFG="HPS - Win32 Debug" RECURSE=1
   cd "..\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"HRA - Win32 Release" :
   cd "\dev\all\gra\hra\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hra50.mak CFG="HRA - Win32 Release"
   cd "..\..\hps\tst"

"HRA - Win32 ReleaseCLEAN" :
   cd "\dev\all\gra\hra\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hra50.mak CFG="HRA - Win32 Release"\
 RECURSE=1
   cd "..\..\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"HRA - Win32 Debug" :
   cd "\dev\all\gra\hra\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hra50.mak CFG="HRA - Win32 Debug"
   cd "..\..\hps\tst"

"HRA - Win32 DebugCLEAN" :
   cd "\dev\all\gra\hra\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hra50.mak CFG="HRA - Win32 Debug" RECURSE=1\

   cd "..\..\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"hrf - Win32 Release" :
   cd "\dev\all\gra\hrf\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hrf50.mak CFG="hrf - Win32 Release"
   cd "..\..\hps\tst"

"hrf - Win32 ReleaseCLEAN" :
   cd "\dev\all\gra\hrf\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hrf50.mak CFG="hrf - Win32 Release"\
 RECURSE=1
   cd "..\..\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"hrf - Win32 Debug" :
   cd "\dev\all\gra\hrf\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hrf50.mak CFG="hrf - Win32 Debug"
   cd "..\..\hps\tst"

"hrf - Win32 DebugCLEAN" :
   cd "\dev\all\gra\hrf\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hrf50.mak CFG="hrf - Win32 Debug" RECURSE=1\

   cd "..\..\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"HRP - Win32 Release" :
   cd "\dev\all\gra\hrp\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hrp50.mak CFG="HRP - Win32 Release"
   cd "..\..\hps\tst"

"HRP - Win32 ReleaseCLEAN" :
   cd "\dev\all\gra\hrp\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hrp50.mak CFG="HRP - Win32 Release"\
 RECURSE=1
   cd "..\..\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"HRP - Win32 Debug" :
   cd "\dev\all\gra\hrp\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hrp50.mak CFG="HRP - Win32 Debug"
   cd "..\..\hps\tst"

"HRP - Win32 DebugCLEAN" :
   cd "\dev\all\gra\hrp\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hrp50.mak CFG="HRP - Win32 Debug" RECURSE=1\

   cd "..\..\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"HUC - Win32 Release" :
   cd "\dev\all\utl\huc\src"
   $(MAKE) /$(MAKEFLAGS) /F .\Huc.mak CFG="HUC - Win32 Release"
   cd "..\..\..\gra\hps\tst"

"HUC - Win32 ReleaseCLEAN" :
   cd "\dev\all\utl\huc\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\Huc.mak CFG="HUC - Win32 Release" RECURSE=1\

   cd "..\..\..\gra\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"HUC - Win32 Debug" :
   cd "\dev\all\utl\huc\src"
   $(MAKE) /$(MAKEFLAGS) /F .\Huc.mak CFG="HUC - Win32 Debug"
   cd "..\..\..\gra\hps\tst"

"HUC - Win32 DebugCLEAN" :
   cd "\dev\all\utl\huc\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\Huc.mak CFG="HUC - Win32 Debug" RECURSE=1
   cd "..\..\..\gra\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"winhra - Win32 Release" :
   cd "\dev\win\gra\hra\src"
   $(MAKE) /$(MAKEFLAGS) /F .\winhra50.mak CFG="winhra - Win32 Release"
   cd "..\..\..\..\all\gra\hps\tst"

"winhra - Win32 ReleaseCLEAN" :
   cd "\dev\win\gra\hra\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\winhra50.mak CFG="winhra - Win32 Release"\
 RECURSE=1
   cd "..\..\..\..\all\gra\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"winhra - Win32 Debug" :
   cd "\dev\win\gra\hra\src"
   $(MAKE) /$(MAKEFLAGS) /F .\winhra50.mak CFG="winhra - Win32 Debug"
   cd "..\..\..\..\all\gra\hps\tst"

"winhra - Win32 DebugCLEAN" :
   cd "\dev\win\gra\hra\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\winhra50.mak CFG="winhra - Win32 Debug"\
 RECURSE=1
   cd "..\..\..\..\all\gra\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"HOD - Win32 Release" :
   cd "\dev\all\utl\hod\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HOD50.mak CFG="HOD - Win32 Release"
   cd "..\..\..\gra\hps\tst"

"HOD - Win32 ReleaseCLEAN" :
   cd "\dev\all\utl\hod\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HOD50.mak CFG="HOD - Win32 Release"\
 RECURSE=1
   cd "..\..\..\gra\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"HOD - Win32 Debug" :
   cd "\dev\all\utl\hod\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HOD50.mak CFG="HOD - Win32 Debug"
   cd "..\..\..\gra\hps\tst"

"HOD - Win32 DebugCLEAN" :
   cd "\dev\all\utl\hod\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HOD50.mak CFG="HOD - Win32 Debug" RECURSE=1\

   cd "..\..\..\gra\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"hcd50 - Win32 Release" :
   cd "\dev\all\utl\hcd\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HCD50.mak CFG="hcd50 - Win32 Release"
   cd "..\..\..\gra\hps\tst"

"hcd50 - Win32 ReleaseCLEAN" :
   cd "\dev\all\utl\hcd\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HCD50.mak CFG="hcd50 - Win32 Release"\
 RECURSE=1
   cd "..\..\..\gra\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"hcd50 - Win32 Debug" :
   cd "\dev\all\utl\hcd\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HCD50.mak CFG="hcd50 - Win32 Debug"
   cd "..\..\..\gra\hps\tst"

"hcd50 - Win32 DebugCLEAN" :
   cd "\dev\all\utl\hcd\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HCD50.mak CFG="hcd50 - Win32 Debug"\
 RECURSE=1
   cd "..\..\..\gra\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"hve - Win32 Release" :
   cd "\dev\all\gra\hve\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hve50.mak CFG="hve - Win32 Release"
   cd "..\..\hps\tst"

"hve - Win32 ReleaseCLEAN" :
   cd "\dev\all\gra\hve\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hve50.mak CFG="hve - Win32 Release"\
 RECURSE=1
   cd "..\..\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"hve - Win32 Debug" :
   cd "\dev\all\gra\hve\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hve50.mak CFG="hve - Win32 Debug"
   cd "..\..\hps\tst"

"hve - Win32 DebugCLEAN" :
   cd "\dev\all\gra\hve\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hve50.mak CFG="hve - Win32 Debug" RECURSE=1\

   cd "..\..\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"HCS50 - Win32 Release" :
   cd "\dev\all\utl\hcs\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hcs50.mak CFG="HCS50 - Win32 Release"
   cd "..\..\..\gra\hps\tst"

"HCS50 - Win32 ReleaseCLEAN" :
   cd "\dev\all\utl\hcs\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hcs50.mak CFG="HCS50 - Win32 Release"\
 RECURSE=1
   cd "..\..\..\gra\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"HCS50 - Win32 Debug" :
   cd "\dev\all\utl\hcs\src"
   $(MAKE) /$(MAKEFLAGS) /F .\hcs50.mak CFG="HCS50 - Win32 Debug"
   cd "..\..\..\gra\hps\tst"

"HCS50 - Win32 DebugCLEAN" :
   cd "\dev\all\utl\hcs\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\hcs50.mak CFG="HCS50 - Win32 Debug"\
 RECURSE=1
   cd "..\..\..\gra\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"winhcs - Win32 Release" :
   cd "\dev\win\utl\hcs\src"
   $(MAKE) /$(MAKEFLAGS) /F .\winhcs50.mak CFG="winhcs - Win32 Release"
   cd "..\..\..\..\all\gra\hps\tst"

"winhcs - Win32 ReleaseCLEAN" :
   cd "\dev\win\utl\hcs\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\winhcs50.mak CFG="winhcs - Win32 Release"\
 RECURSE=1
   cd "..\..\..\..\all\gra\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"winhcs - Win32 Debug" :
   cd "\dev\win\utl\hcs\src"
   $(MAKE) /$(MAKEFLAGS) /F .\winhcs50.mak CFG="winhcs - Win32 Debug"
   cd "..\..\..\..\all\gra\hps\tst"

"winhcs - Win32 DebugCLEAN" :
   cd "\dev\win\utl\hcs\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\winhcs50.mak CFG="winhcs - Win32 Debug"\
 RECURSE=1
   cd "..\..\..\..\all\gra\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"whfc - Win32 Release" :
   cd "\dev\win\utl\hfc\src"
   $(MAKE) /$(MAKEFLAGS) /F .\winhfc50.mak CFG="whfc - Win32 Release"
   cd "..\..\..\..\all\gra\hps\tst"

"whfc - Win32 ReleaseCLEAN" :
   cd "\dev\win\utl\hfc\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\winhfc50.mak CFG="whfc - Win32 Release"\
 RECURSE=1
   cd "..\..\..\..\all\gra\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"whfc - Win32 Debug" :
   cd "\dev\win\utl\hfc\src"
   $(MAKE) /$(MAKEFLAGS) /F .\winhfc50.mak CFG="whfc - Win32 Debug"
   cd "..\..\..\..\all\gra\hps\tst"

"whfc - Win32 DebugCLEAN" :
   cd "\dev\win\utl\hfc\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\winhfc50.mak CFG="whfc - Win32 Debug"\
 RECURSE=1
   cd "..\..\..\..\all\gra\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"him - Win32 Release" :
   cd "\dev\all\gra\him\src"
   $(MAKE) /$(MAKEFLAGS) /F .\him50.mak CFG="him - Win32 Release"
   cd "..\..\hps\tst"

"him - Win32 ReleaseCLEAN" :
   cd "\dev\all\gra\him\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\him50.mak CFG="him - Win32 Release"\
 RECURSE=1
   cd "..\..\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"him - Win32 Debug" :
   cd "\dev\all\gra\him\src"
   $(MAKE) /$(MAKEFLAGS) /F .\him50.mak CFG="him - Win32 Debug"
   cd "..\..\hps\tst"

"him - Win32 DebugCLEAN" :
   cd "\dev\all\gra\him\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\him50.mak CFG="him - Win32 Debug" RECURSE=1\

   cd "..\..\hps\tst"

!ENDIF

!IF  "$(CFG)" == "HPSTST - Win32 Release"

"HRFInternetImaging - Win32 Release" :
   cd "\dev\all\gra\hrf\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HRFInternetImaging.mak\
 CFG="HRFInternetImaging - Win32 Release"
   cd "..\..\hps\tst"

"HRFInternetImaging - Win32 ReleaseCLEAN" :
   cd "\dev\all\gra\hrf\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HRFInternetImaging.mak\
 CFG="HRFInternetImaging - Win32 Release" RECURSE=1
   cd "..\..\hps\tst"

!ELSEIF  "$(CFG)" == "HPSTST - Win32 Debug"

"HRFInternetImaging - Win32 Debug" :
   cd "\dev\all\gra\hrf\src"
   $(MAKE) /$(MAKEFLAGS) /F .\HRFInternetImaging.mak\
 CFG="HRFInternetImaging - Win32 Debug"
   cd "..\..\hps\tst"

"HRFInternetImaging - Win32 DebugCLEAN" :
   cd "\dev\all\gra\hrf\src"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\HRFInternetImaging.mak\
 CFG="HRFInternetImaging - Win32 Debug" RECURSE=1
   cd "..\..\hps\tst"

!ENDIF


!ENDIF

