AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libCsmap.la

libCsmap_la_SOURCES = \
  Source/CS_alber.c \
  Source/CS_angle.c \
  Source/CS_ansi.c \
  Source/CS_ats77New.c \
  Source/CS_azmea.c \
  Source/CS_azmed.c \
  Source/CS_badekas.c \
  Source/CS_bonne.c \
  Source/CS_bpcnc.c \
  Source/CS_bursa.c \
  Source/CS_bynFile.c \
  Source/CS_category.c \
  Source/CS_csini.c \
  Source/CS_csio.c \
  Source/CS_csprm.c \
  Source/CS_csWktLoc.c \
  Source/CS_datum.c \
  Source/CS_defaults.c \
  Source/CS_defCmp.c \
  Source/CS_defCmpEx.c \
  Source/CS_dtcalc.c \
  Source/CS_dtio.c \
  Source/CS_dtmBridge.c \
  Source/CS_edcnc.c \
  Source/CS_edcyl.c \
  Source/CS_egm96.c \
  Source/CS_ekrt4.c \
  Source/CS_ekrt6.c \
  Source/CS_elCalc.c \
  Source/CS_elio.c \
  Source/CS_erpt.c \
  Source/CS_error.c \
  Source/CS_fips.c \
  Source/CS_frame.c \
  Source/CS_frnch.c \
  Source/CS_gauss.c \
  Source/CS_general.c \
  Source/CS_geoct.c \
  Source/CS_geoid96.c \
  Source/CS_geoid99.c \
  Source/CS_GeoidHeight.c \
  Source/CS_gissupprt.c \
  Source/CS_gnomc.c \
  Source/CS_gpio.c \
  Source/CS_gridi.c \
  Source/CS_groups.c \
  Source/CS_guiApi.c \
  Source/CS_gxIndex.c \
  Source/CS_gxio.c \
  Source/CS_gxprm.c \
  Source/CS_hlApi.c \
  Source/CS_hmlsn.c \
  Source/CS_hpApi.c \
  Source/CS_japanNew.c \
  Source/CS_krovk.c \
  Source/CS_lmbrt.c \
  Source/CS_lmtan.c \
  Source/CS_mfc.cpp \
  Source/CS_mgrs.c \
  Source/CS_millr.c \
  Source/CS_modpc.c \
  Source/CS_molod.c \
  Source/CS_molwd.c \
  Source/CS_mrcat.c \
  Source/CS_mstro.c \
  Source/CS_mulrg.c \
  Source/CS_nacyl.c \
  Source/CS_nadcn.c \
  Source/CS_nerth.c \
  Source/CS_ntv1.c \
  Source/CS_ntv2.c \
  Source/CS_nullx.c \
  Source/CS_nzlnd.c \
  Source/CS_oblqm.c \
  Source/CS_optional.c \
  Source/CS_ortho.c \
  Source/CS_osgm91.c \
  Source/CS_ost02.c \
  Source/CS_ost97.c \
  Source/CS_ostn02.c \
  Source/CS_ostn97.c \
  Source/CS_ostro.c \
  Source/CS_parm3.c \
  Source/CS_parm4.c \
  Source/CS_parm6.c \
  Source/CS_parm7.c \
  Source/CS_plycn.c \
  Source/CS_pstro.c \
  Source/CS_rlsUpdt.c \
  Source/CS_robin.c \
  Source/CS_sinus.c \
  Source/CS_sstro.c \
  Source/CS_supprt.c \
  Source/CS_swiss.c \
  Source/CS_sys34.c \
  Source/CS_system.c \
  Source/CS_tacyl.c \
  Source/CS_trmer.c \
  Source/CS_trmrs.c \
  Source/CS_units.c \
  Source/CS_unity.c \
  Source/CS_vdgrn.c \
  Source/CS_VertconUS.c \
  Source/CS_vrtcon.c \
  Source/cs_wellknowntext.cpp \
  Source/CS_wgs72.c \
  Source/CS_winkelTripel.c \
  Source/CS_zones.c \
  Source/csBrowser.cpp \
  Source/CScs2Wkt.cpp \
  Source/cscscomp.c \
  Source/csCsvFileSupport.cpp \
  Source/CSdata.c \
  Source/csDataDir.cpp \
  Source/CSdataDT.c \
  Source/CSdataPJ.c \
  Source/CSdataU.c \
  Source/CSdatumCatalog.c \
  Source/CSdictDiff.c \
  Source/CSdt2Wkt.cpp \
  Source/CSdtcomp.c \
  Source/csDualBrowse.cpp \
  Source/CSel2Wkt.cpp \
  Source/CSelcomp.c \
  Source/csEpsgStuff.cpp \
  Source/csEpsgSupport.cpp \
  Source/CSgeodeticSupport.c \
  Source/CSgpcomp.c \
  Source/CSgxcomp.c \
  Source/csKeyNm.cpp \
  Source/CSmrcomp.c \
  Source/csNameMapper.cpp \
  Source/csNameMapperSupport.cpp \
  Source/CSsys34KMS.c \
  Source/CSwinHlp.cpp \
  Source/CSwktFlavors.c \
  Source/rcWellKnownText.cpp \
  Source/rcWktKonstants.cpp \
  Source/csIoUtil.cpp

noinst_HEADERS = \
  Include/csIoUtilAutoPtr.hpp \
  Include/csIoUtilDirIterator.hpp \
  Include/csIoUtilDirSwitch.hpp \
  Include/csIoUtil.hpp \
  Include/cs_ioUtil.h \
  Include/csCsvFileSupport.hpp \
  Include/csCsvFileSupport.hpp \
  Include/csepsgstuff.h \
  Include/csNameMapper.hpp \
  Include/csNameMapperSupport.h \
  Include/cs_clientBeg.h \
  Include/cs_clientEnd.h \
  Include/cs_Geodetic.h \
  Include/cs_hlp.h \
  Include/cs_Legacy.h \
  Include/cs_map.h \
  Include/cs_mfc.h \
  Include/cs_Nadcon.h \
  Include/cs_NTv1.h \
  Include/cs_NTv2.h \
  Include/cs_OpenSource.h \
  Include/cs_res.h \
  Include/cs_Wchar.hpp \
  Include/cs_wkt.h \
  Include/rcWellKnownText.hpp

INCLUDES = -IInclude 

AM_CFLAGS = @CFLAGS@

AM_CXXFLAGS = @CXXFLAGS@ -DMG

AM_CPPFLAGS = -DCSCOMPARE -DCSAPI -DMG

libCsmap_la_LDFLAGS = -version-info 1:0:0

