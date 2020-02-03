//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTClassID.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#ifdef HUTEXPORT_LINK_MODE_LIB
#define HUTEXPORT_LINK_MODE
#else
#define HUTEXPORT_LINK_MODE AFX_EXT_CLASS
#endif

// The section index indicate the position into the container.
#define HUTEXPORT_DESTINATION_SECTION        0
#define HUTEXPORT_IMG_SIZE_SECTION           1
#define HUTEXPORT_PIXEL_TYPE_SECTION         2
#define HUTEXPORT_SUBRES_PIXEL_TYPE_SECTION  3
#define HUTEXPORT_SMALL_COMPRESSION_SECTION  4
#define HUTEXPORT_COMPRESSION_SECTION        5
#define HUTEXPORT_ENCODING_SECTION           6
#define HUTEXPORT_BLOCK_SECTION              7
#define HUTEXPORT_GEO_REF_SECTION            8

#define HUTEXPORT_MAX_SECTION                9

// The section indicate Dialog ID
#define IDD_DLG_EXPORT_DIALOG           131
#define IDD_DLG_COMPRESSION             132
#define IDD_DLG_PIXEL_TYPE              133
#define IDD_DLG_ENCODING                134
#define IDD_DLG_IMG_SIZE                135
#define IDD_DLG_BLOCK                   136
#define IDD_DLG_OPTION                  139
#define IDD_DLG_SMALL_COMPRESSION       140
#define IDD_DLG_GEO_REF_FORMAT          141
#define IDD_DLG_PROGRESS                142
#define IDD_DLG_CONVERT_TO              150
#define IDD_DLG_DESTINATION             151
#define IDD_DLG_EXPORT_INTEGRITY        152
#define IDB_BMP_BT_INTEGRITY_OFF_UP     153
#define IDB_BMP_BT_INTEGRITY_OFF_DOWN   154
#define IDD_DLG_SUBRES_PIXEL_TYPE       160

// The section indicate Dialog Control ID
#define ID_HUT_TIMER                    0x451
#define IDC_BT_OPTION                   1001

#define IDC_COMPRESSION_QUALITY_SLIDER  1002
#define IDC_SPIN_SET_QUALITY            1003
#define IDC_EDIT_QUALITY                1004
#define IDC_CMB_COMPRESSION             1005

#define IDC_EDIT_WIDTH                  1006
#define IDC_EDIT_HEIGHT                 1007
#define IDC_SPIN_WIDTH                  1008
#define IDC_SPIN_HEIGHT                 1009
#define IDC_COMBO_BLOCK_TYPE            1010

#define IDC_CMB_ENCODING                1011
#define IDC_CHK_RESAMPLE                1012
#define IDC_CMB_PIXEL_TYPE              1013
#define IDC_CMB_RESAMPLING              1014
#define IDC_CMB_GEO_REF_FORMAT          1015

#define IDC_PROGRESS_BAR                1020
#define IDC_STOP_PROGRESSION            1021
#define IDC_DURATION                    1022
#define IDC_LIFESIGNAL                  1023

#define IDC_BT_OK                       1030
#define IDC_BT_CANCEL                   1031
#define IDC_BT_RASTER_CHECK             1032

#define IDC_CMB_FILE_FORMAT             1041
#define IDC_CHK_BEST_OPTION             1042
#define IDC_CHK_KEEP_SRCPATH            1043
#define IDC_RD_RETAIN_EXT               1044
#define IDC_RD_REPLACE_EXTENSION        1045
#define IDC_EXPORTAS_SOURCE             1046
#define IDC_CHK_EXCLUDE_JPEG            2114

#define IDC_DRAWSIGNAL                  1050
#define IDC_BT_BROWSE                   1051
#define IDC_EDIT_NEW_PATH               1052
#define IDC_CHK_RESUME_IN_FILENAME      1056
#define IDC_CHK_ALL_POSSIBILITY         1057
#define IDC_CHK_GEOREF_COMP             1058
#define IDC_CHK_IMAGE_COMP              1059
#define IDC_COMBO_IMAGE_COMP_METHOD     1060

#define IDC_CHK_TRESHOLD                1068
#define IDC_EDIT_ERROR_TOLERENCE        1069
#define IDC_EDIT_THRESHLOD_MIN          1070
#define IDC_EDIT_THRESHLOD_MAX          1071
#define IDC_SPIN_ERROR_TOLERENCE        1072
#define IDC_SPINIDC_THRESHLOD_MIN       1073
#define IDC_SPINIDC_THRESHLOD_MAX       1074
#define IDC_CHK_EXPORT_INTEGRITY        1076
#define IDC_CHK_SAVE_IMAGE_RESULT       1078
#define IDC_EDIT_IMAGE_RESULT_PATH      1079

#define IDC_STATIC_ERROR_TOLERENCE      1080
#define IDC_STATIC_THRESHOLD_MIN        1081
#define IDC_STATIC_THRESHOLD_MAX        1082

#define IDC_EDIT_SCALE_X                1090
#define IDC_EDIT_SCALE_Y                1091
#define IDC_CHK_FIX_IMAGE_SIZE          1092
#define IDC_CHK_FIX_IMAGE_SCALE         1093
#define IDC_CHK_ASPECT_RATIO            1094
#define IDC_RESET                       1095

#define IDC_EXPORTAS_SELECT             2000
#define IDC_EXPORTAS_FOLDER             2001
#define IDC_EXPORTAS_FORMAT             2002
#define IDC_EXPORTAS_BESTOPTION         2003
#define IDC_EXPORTAS_EXPORT             2004
