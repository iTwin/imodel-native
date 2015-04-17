/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/DgnPlatformInternal/DgnHandlers/DgnHandlersMessage.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>
#include <BeSQLite/L10N.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

BENTLEY_TRANSLATABLE_STRINGS_START(DgnHandlersMessage,dgnhandler_msg)
    {
    IDS_TYPENAMES_GRAPHIC_ELM = 153,                        // =="Graphic"==
    IDS_TYPENAMES_ANNOTATION_ELM = 154,                     // =="Annotation"==
    IDS_TYPENAMES_MISSING_HANDLER = 163,                    // =="Missing Handler"==
    IDS_DsigDescription = 500,                              // =="Digital Signature"==
    IDS_DsigInfo_SignedBy = 501,                            // =="By: %0.120s %0.120s"==
    IDS_DsigInfo_EmailAndName = 502,                        // =="(%0.120s %0.120s)"==
    IDS_DsigInfo_Location = 503,                            // =="Location: %0.120s"==
    IDS_DsigInfo_Purpose = 504,                             // =="Purpose: %0.120s"==
    IDS_DsigInfo_PrereqCount = 506,                         // =="Prerequisites: %d"==
    IDS_DsigInfo_IncludesRefs = 507,                        // =="Includes References"==
    IDS_DsigInfo__NOT_VERIFIED = 508,                       // =="<NOT VERIFIED>"==
    IDS_DsigInfo_Verified = 509,                            // =="(Verified)"==
    IDS_DsigInfo__NOT_TRUSTED = 510,                        // =="<NOT TRUSTED>"==
    IDS_DsigInfo_SignedOn = 511,                            // =="Signed: %ls %ls"==
    IDS_DsigInfo_ExpiresOn = 512,                           // =="Expires: %ls %ls"==
    IDS_DsigInfo__EXPIRED = 513,                            // =="<EXPIRED>"==
    IDS_DsigPropCategory_Dsig = 600,                        // =="Digital Signature"==
    IDS_DsigProp_Verified = 601,                            // =="Verified"==
    IDS_DsigProp_Signer = 602,                              // =="Signer"==
    IDS_DsigProp_SignerEmail = 603,                         // =="Email"==
    IDS_DsigProp_InformalName = 604,                        // =="Informal Name"==
    IDS_DsigProp_Location = 605,                            // =="Location"==
    IDS_DsigProp_Purpose = 606,                             // =="Purpose"==
    IDS_DsigProp_Date = 607,                                // =="Date"==
    IDS_DsigProp_ExpiryDate = 608,                          // =="Expiration"==
    IDS_DsigProp_Trusted = 609,                             // =="Trusted"==
    IDS_DsigProp_Expired = 610,                             // =="Expired"==
    IDS_DsigProp_IncludesReferences = 611,                  // =="Includes References"==
    IDS_DsigProp_Scope = 612,                               // =="Scope"==
    IDS_DsigProp_ScopeModel = 613,                          // =="Model"==
    IDS_DsigProp_ScopeFile = 614,                           // =="File"==
    MSGID_MHEname = 1000,                                   // =="Name"==
    MSGID_MHEdescription = 1001,                            // =="Description"==
    MSGID_MHEunitLock = 1002,                               // =="Unit Lock"==
    MSGID_MHEgridLock = 1003,                               // =="Grid Lock"==
    MSGID_MHEisoGrid = 1004,                                // =="Iso Grid"==
    MSGID_MHEisoLock = 1005,                                // =="Iso Lock"==
    MSGID_MHEisoPlane = 1006,                               // =="Iso Plane"==
    MSGID_MHEuseAnnScale = 1007,                            // =="Use Annotation Scale"==
    MSGID_MHEacsActiveLock = 1008,                          // =="ACS Active Lock"==
    MSGID_MHEmodelIs3D = 1009,                              // =="Is3D"==
    MSGID_MHEhidden = 1010,                                 // =="Hidden"==
    MSGID_MHElocked = 1011,                                 // =="Locked"==
    MSGID_MHEnotInCellList = 1012,                          // =="Can be placed as a cell"==
    MSGID_MHEuseBackgroundColor = 1013,                     // =="Use Background Color"==
    MSGID_MHElSScaleIsAnnScale = 1014,                      // =="LSScaleIsAnnScale"==
    MSGID_MHEisAnnotationCell = 1015,                       // =="Is Annotation Cell"==
    MSGID_MHEautoUpdateFields = 1016,                       // =="Auto-update Fields"==
    MSGID_MHEnonIndexed = 1017,                             // =="Non-indexed"==
    MSGID_MHEisMarkup = 1018,                               // =="Is Markup"==
    MSGID_MHEtype = 1019,                                   // =="Type"==
    MSGID_MHEuorPerStorage = 1020,                          // =="UOR per Storage"==
    MSGID_MHEstorageUnit = 1021,                            // =="Storage Unit"==
    MSGID_MHEmasterUnit = 1022,                             // =="Master Unit"==
    MSGID_MHEsubUnit = 1023,                                // =="Sub Unit"==
    MSGID_MHEglobalOrigin = 1024,                           // =="Global Origin"==
    MSGID_MHEtypeData = 1025,                               // =="Cell Type"==
    MSGID_MHEsolidExtent = 1026,                            // =="Solid Extent"==
    MSGID_MHEinsertBase = 1027,                             // =="Insert Base"==
    MSGID_MHEacsElementId = 1028,                           // =="ACS ElementId"==
    MSGID_MHEacsType = 1029,                                // =="ACS Type"==
    MSGID_MHEacsOrigin = 1030,                              // =="ACS Origin"==
    MSGID_MHEacsScale = 1031,                               // =="ACS Scale"==
    MSGID_MHEacsRotation = 1032,                            // =="ACS Rotation"==
    MSGID_MHEroundoffUnit = 1033,                           // =="Roundoff Unit"==
    MSGID_MHEuorPerGrid = 1034,                             // =="UOR Per Grid"==
    MSGID_MHErefGrid = 1035,                                // =="Ref Grid"==
    MSGID_MHEgridRatio = 1036,                              // =="Grid Ratio"==
    MSGID_MHEgridBase = 1037,                               // =="Grid Base"==
    MSGID_MHEgridAngle = 1038,                              // =="Grid Angle"==
    MSGID_MHEroundoffRatio = 1039,                          // =="Roundoff Ratio"==
    MSGID_MHElinestyleScale = 1040,                         // =="Linestyle Scale"==
    MSGID_MHEbackgroundColor = 1041,                        // =="Background Color"==
    MSGID_MHEdefaultRefLogical = 1042,                      // =="Default Ref Logical"==
    MSGID_MHEtransparency = 1043,                           // =="Transparency"==
    MSGID_MHEazimuth = 1044,                                // =="Azimuth"==
    MSGID_MHEShtorigin = 1045,                              // =="Origin"==
    MSGID_MHEShtrotation = 1046,                            // =="Rotation"==
    MSGID_MHEShtwidth = 1047,                               // =="Width"==
    MSGID_MHEShtheight = 1048,                              // =="Height"==
    MSGID_MHEShtcolor = 1049,                               // =="Color"==
    MSGID_MHEShtunitDef = 1050,                             // =="Unit Def"==
    MSGID_MHEShtpaperOrigin = 1051,                         // =="Paper Origin"==
    MSGID_MHEShtpaperRotation = 1052,                       // =="Paper Rotation"==
    MSGID_MHEShtpaperWidth = 1053,                          // =="Paper Width"==
    MSGID_MHEShtpaperHeight = 1054,                         // =="Paper Height"==
    MSGID_MHEShtpaperUnitDef = 1055,                        // =="Paper Unit Def"==
    MSGID_MHEShttopPaperMargin = 1056,                      // =="Top Paper Margin"==
    MSGID_MHEShtleftPaperMargin = 1057,                     // =="Left Paper Margin"==
    MSGID_MHEShtbottomPaperMargin = 1058,                   // =="Bottom Paper Margin"==
    MSGID_MHEShtrightPaperMargin = 1059,                    // =="Right Paper Margin"==
    MSGID_MHEShtplotScale = 1060,                           // =="Plot Scale"==
    MSGID_MHEShtformName = 1061,                            // =="Form Name"==
    MSGID_MHEShtpaperFormName = 1063,                       // =="Paper Form Name"==
    MSGID_MHEShtwindowsPrinterName = 1064,                  // =="Windows Printer Name"==
    MSGID_MHEShtpltFileName = 1065,                         // =="Plt File Name"==
    MSGID_MHEShtpstFileName = 1066,                         // =="Pst File Name"==
    MSGID_MHEShtsheetNumber = 1067,                         // =="Sheet Number"==
    MSGID_MHEShtborderAttachmentId = 1068,                  // =="Border Attachment Id"==
    MSGID_MHEShtsheetName = 1069,                           // =="Sheet Name"==
    MSGID_MHEShtdrawingScale = 1071,                        // =="Drawing Scale"==
    MSGID_MHEShtgeometryScale = 1072,                       // =="Geometry Scale"==
    MSGID_MHEShtannotationScale = 1073,                     // =="Annotation Scale"==
    MSGID_MHEShtisEnabled = 1074,                           // =="Display Sheet Boundary"==
    MSGID_MHEShtgeometryAtDrawingScale = 1075,              // =="Geometry At Drawing Scale"==
    MSGID_MHEShtprintStyleName = 1076,                      // =="Print Style Name"==
    MSGID_MHEtypeStringsDesign = 1080,                      // =="Design"==
    MSGID_MHEtypeStringsSheet = 1081,                       // =="Sheet"==
    MSGID_MHEtypeStringsExtraction = 1082,                  // =="Extraction"==
    MSGID_MHEacsTypeStringsRectangular = 1085,              // =="Rectangular"==
    MSGID_MHEacsTypeStringsCylindrical = 1086,              // =="Cylindrical"==
    MSGID_MHEacsTypeStringsSpherical = 1087,                // =="Spherical"==
    MSGID_MHEModelCategory = 1092,                          // =="Model"==
    MSGID_MHESheetCategory = 1093,                          // =="Sheet"==
    MSGID_NAMEDVIEW_BaseName = 1300,                        // =="Untitled"==
    MSGID_OleTypeLinked = 2000,                             // =="Link to: "==
    MSGID_OleTypeEmbedded = 2001,                           // =="Embedded "==
    MSGID_OleTypePictureOf = 2002,                          // =="Picture of "==
    MSGID_UniqueNameRoot = 2003,                            // =="Untitled"==
    MSGID_SolarLockOn = 2004,                               // =="Solar Lock On"==
    MSGID_SolarLightOff = 2005,                             // =="Solar Light Off"==
    IDS_DEFAULT_SHEET_NAME = 2100,                          // =="Sheet Model"==
    IDS_DEFAULT_SHEET_DESCRIPTION = 2101,                   // =="Sheet model from V7 Sheet View"==
    IDS_DEFAULT_VIEWGROUP_NAME = 2102,                      // =="Default Views"==
    IDS_DEFAULT_VIEWGROUP_DESCRIPTION = 2103,               // =="Default view group for V7 conversion."==
    IDS_LEVELTABLEMSGS_LevelName = 2104,                    // =="Level "==
    IDS_DEFAULT_MODEL_NAME = 2105,                          // =="Default"==
    IDS_DEFAULT_MODEL_DESCRIPTION = 2106,                   // =="Master Model"==
    IDS_DEPENDENCY_FMTS_LINEAR_FMT = 2202,                  // =="Linear (vertex: %d, frac: %d/%d)"==
    IDS_DEPENDENCY_FMTS_PROJECTION_FMT = 2203,              // =="Projection (vertex: %d, ratio: %f)"==
    IDS_DEPENDENCY_FMTS_ARC_ANGLE_FMT = 2204,               // =="Arc (angle: %f)"==
    IDS_DEPENDENCY_FMTS_ARC_TYPE_FMT = 2205,                // =="Arc (%ls)"==
    IDS_DEPENDENCY_FMTS_ARC_TYPE_CENTER = 2206,             // =="center"==
    IDS_DEPENDENCY_FMTS_ARC_TYPE_START = 2207,              // =="start"==
    IDS_DEPENDENCY_FMTS_ARC_TYPE_END = 2208,                // =="end"==
    IDS_DEPENDENCY_FMTS_MLINE_FMT = 2209,                   // =="Mline (pt: %d, line %d, offset %f)"==
    IDS_DEPENDENCY_FMTS_ORIGIN_FMT = 2210,                  // =="Origin"==
    IDS_DEPENDENCY_FMTS_BCURVE_FMT = 2211,                  // =="BspCurve (u: %f)"==
    IDS_DEPENDENCY_FMTS_BSURF_FMT = 2212,                   // =="BspSurf (u: %f, v: %f)"==
    IDS_DEPENDENCY_FMTS_MESH_VERTEX_FMT = 2213,             // =="Mesh Vertex (vertex: %d)"==
    IDS_DEPENDENCY_FMTS_MESH_EDGE_FMT = 2214,               // =="Mesh Edge (edge: %d, u: %f)"==
    IDS_DEPENDENCY_FMTS_INTERSECT_FMT = 2215,               // =="Old Intersect (index %d)"==
    IDS_DEPENDENCY_FMTS_INTERSECT2_FMT = 2216,              // =="Intersect (index: %d, segs %d, %d)"==
    IDS_DEPENDENCY_FMTS_SINGLE_ELM_FMT = 2217,              // =="Elm: %I64d"==
    IDS_DEPENDENCY_FMTS_SINGLE_ELM_REF_FMT = 2218,          // =="Elm: %I64d, Ref: %I64d"==
    IDS_DEPENDENCY_FMTS_TWO_ELM_FMT = 2219,                 // =="Elms: %I64d, %I64d"==
    IDS_DEPENDENCY_FMTS_TWO_ELM_REF_FMT = 2220,             // =="Elms: %I64d, %I64d, Refs: %I64d, %I64d"==
    MSGID_DGNLINKS_LINKSET_Untitled = 2301,                 // =="Untitled"==
    MSGID_DGNLINKS_LINKTYPE_File = 2302,                    // =="File"==
    MSGID_DGNLINKS_LINKTYPE_Folder = 2303,                  // =="Folder"==
    MSGID_DGNLINKS_LINKTYPE_DesignModel = 2304,             // =="Design Model"==
    MSGID_DGNLINKS_LINKTYPE_SheetModel = 2305,              // =="Sheet Model"==
    MSGID_DGNLINKS_LINKTYPE_DrawingModel = 2306,            // =="Drawing Model"==
    MSGID_DGNLINKS_LINKTYPE_SavedView = 2307,               // =="Saved View"==
    MSGID_DGNLINKS_LINKTYPE_Reference = 2308,               // =="Reference Attachment"==
    MSGID_DGNLINKS_LINKTYPE_Drawing = 2309,                 // =="Drawing Region"==
    MSGID_DGNLINKS_LINKTYPE_LinkLinkSet = 2310,             // =="Link to Linkset or Link Folder"==
    MSGID_DGNLINKS_LINKTYPE_ConfigurationVariable = 2311,   // =="Link to Configuration Variable"==
    MSGID_DGNLINKS_LINKTYPE_Keyin = 2312,                   // =="Key-in Link"==
    MSGID_DGNLINKS_LINKTYPE_Url = 2313,                     // =="URL Link"==
    MSGID_DGNLINKS_BRANCH_SAVEDVIEWS = 2314,                // =="Saved Views"==
    MSGID_DGNLINKS_BRANCH_ATTACHMENTS = 2315,               // =="References"==
    MSGID_DGNLINKS_BRANCH_DRAWINGS = 2316,                  // =="Drawings"==
    MSGID_DYNAMICVIEWS_VIEWTYPE_Section = 2401,             // =="Section View"==
    MSGID_DYNAMICVIEWS_VIEWTYPE_Detail = 2402,              // =="Detail View"==
    MSGID_DYNAMICVIEWS_VIEWTYPE_Elevation = 2403,           // =="Elevation View"==
    MSGID_DYNAMICVIEWS_VIEWTYPE_Plan = 2404,                // =="Plan View"==
    MS_MSGID_OriginalElementHasBeenModified = 2601,         // =="Cached Display - Original Element Modified"==
    MS_MSGID_OriginalElementHasBeenDeleted = 2602,          // =="Cached Display - Original Element Deleted"==
    MS_MSGID_CachedCutGeometry = 2603,                      // =="Cached Cut Geometry"==
    MS_MSGID_CachedVisibleEdge = 2604,                      // =="Cached Visible Edge"==
    MS_MSGID_CachedHiddenEdge = 2605,                       // =="Cached Hidden Edge"==
    MS_MSGID_CachedVisibleWire = 2606,                      // =="Cached Visible Wire"==
    MS_MSGID_CachedHiddenWire = 2607,                       // =="Cached Hidden Wire"==
    MS_MSGID_OriginalElementUnavailable = 2608,             // =="Cached Display - Original Model Unavailable"==
    MS_MSGID_CachedAnnotation = 2609,                       // =="Cached Annotation"==
    MS_MSGID_CachedUnderlay = 2610,                         // =="Cached Underlay"==
    MS_MSGID_SectionGraphicsCut = 2611,                     // =="Section cut graphics"==
    MS_MSGID_SectionGraphicsForward = 2612,                 // =="Section forward graphics"==
    MS_MSGID_SectionGraphicsBackward = 2613,                // =="Section backward graphics"==
    MS_MSGID_SectionGraphicsOutside = 2614,                 // =="Clip volume outside graphics"==
    MS_MSGID_CachedVisibleEdgesName = 2615,                 // =="Cached Visible Edges"==
    MSGID_ECTYPEADAPTER_True = 2701,                        // =="True"==
    MSGID_ECTYPEADAPTER_False = 2702,                       // =="False"==
    MSGID_ECTYPEADAPTER_ParensNull = 2703,                  // =="(null)"==
    MSGID_ECTYPEADAPTER_ParensNone = 2704,                  // =="(None)"==
    MSGID_ECTYPEADAPTER_ByLevel = 2705,                     // =="ByLevel"==
    MSGID_ECTYPEADAPTER_ByCell = 2706,                      // =="ByCell"==
    MSGID_ECTYPEADAPTER_Bytes = 2707,                       // =="bytes"==
    MSGID_ECTYPEADAPTER_KB = 2708,                          // =="KB"==
    MSGID_ECTYPEADAPTER_MB = 2709,                          // =="MB"==
    MSGID_ECTYPEADAPTER_GB = 2710,                          // =="GB"==
    MSGID_ECTYPEADAPTER_MemberOfSingle = 2711,              // =="Member of 1 group"==
    MSGID_ECTYPEADAPTER_MemberOfPlural = 2712,              // =="Member of %d groups"==
    MSGID_ECTYPEADAPTER_SheetScaleCustom = 2713,            // =="CUSTOM"==
    MSGID_ECTYPEADAPTER_ElemClassPrimary = 2714,            // =="Primary"==
    MSGID_ECTYPEADAPTER_ElemClassPatternComponent = 2715,   // =="Pattern Component"==
    MSGID_ECTYPEADAPTER_ElemClassConstruction = 2716,       // =="Construction"==
    MSGID_ECTYPEADAPTER_ElemClassDimension = 2717,          // =="Dimension"==
    MSGID_ECTYPEADAPTER_ElemClassPrimaryRule = 2718,        // =="Primary Rule"==
    MSGID_ECTYPEADAPTER_ElemClassLinearPatterned = 2719,    // =="Linear Patterned"==
    MSGID_ECTYPEADAPTER_ElemClassConstructionRule = 2720,   // =="Construction Rule"==
    MSGID_ECTYPEADAPTER_Yes = 2721,                         // =="Yes"==
    MSGID_ECTYPEADAPTER_No = 2722,                          // =="No"==
    MSGID_ECTYPEADAPTER_On = 2723,                          // =="On"==
    MSGID_ECTYPEADAPTER_Off = 2724,                         // =="Off"==
    MSGID_ECTYPEADAPTER_Enabled = 2725,                     // =="Enabled"==
    MSGID_ECTYPEADAPTER_Disabled = 2726,                    // =="Disabled"==
    MSGID_ECTYPEADAPTER_Acres = 2727,                       // =="Acres"==
    MSGID_ECTYPEADAPTER_WorkingUnits = 2728,                // =="Working Units"==
    MSGID_ECTYPEADAPTER_PropertyUnused = 2729,              // =="<Unused>"==
    MSGID_ECTYPEADAPTER_PropertyNone = 2730,                // =="<None>"==
    MSGID_ECTYPEADAPTER_NotAvailable = 2731,                // =="Not Available"==
    MSGID_ECTYPEADAPTER_GeopriorityAttachment = 2732,       // =="Attachment"==
    MSGID_ECTYPEADAPTER_GeopriorityHeader = 2733,           // =="Raster Header"==
    MSGID_ECTYPEADAPTER_GeoprioritySisterFile = 2734,       // =="Sister File"==
    MSGID_ECTYPEADAPTER_DisplayPriority_BackPlane = 2735,   // =="Background"==
    MSGID_ECTYPEADAPTER_DisplayPriority_DesignPlane = 2736, // =="Design"==
    MSGID_ECTYPEADAPTER_DisplayPriority_FrontPlane = 2737,  // =="Foreground"==
    MSGID_ECTYPEADAPTER_ColorMode_Unknown = 2738,           // =="Unknown"==
    MSGID_ECTYPEADAPTER_ColorMode_RGB = 2739,               // =="RGB"==
    MSGID_ECTYPEADAPTER_ColorMode_Palette16 = 2740,         // =="16 Colors"==
    MSGID_ECTYPEADAPTER_ColorMode_Palette256 = 2741,        // =="256 Colors"==
    MSGID_ECTYPEADAPTER_ColorMode_Greyscale = 2742,         // =="Grayscale"==
    MSGID_ECTYPEADAPTER_ColorMode_Monochrome = 2743,        // =="Monochrome"==
    MSGID_ECTYPEADAPTER_ColorMode_RGBA = 2744,              // =="RGB Alpha"==
    MSGID_ECTYPEADAPTER_ColorMode_Palette256Alpha = 2745,   // =="256 Colors with Alpha"==
    MSGID_ECTYPEADAPTER_ColorMode_Greyscale16 = 2746,       // =="16 bits Grayscale"==
    MSGID_ECTYPEADAPTER_ColorMode_Palette2 = 2747,          // =="2 Colors"==
    MSGID_ECTYPEADAPTER_VariesAcross = 2748,                // =="Varies Across"==
    MSGID_ECTYPEADAPTER_NoHistory = 2749,                   // =="No History"==
    MSGID_ECTYPEADAPTER_FromView = 2750,                    // =="From View"==
    MSGID_ECTYPEADAPTER_SolidFill = 2751,                   // =="Filled Hidden Line"==
    MSGID_ECTYPEADAPTER_HiddenLine = 2752,                  // =="Hidden Line"==
    MSGID_ECTYPEADAPTER_Wireframe = 2753,                   // =="Wireframe"==
    MSGID_ECTYPEADAPTER_Smooth = 2754,                      // =="Smooth"==
    MSGID_ECTYPEADAPTER_RenderModeSuffix = 2755,            // =="Display"==
    MSGID_LIGHTNAME_AREA = 2801,                            // =="Area Light"==
    MSGID_LIGHTNAME_DISTANT = 2802,                         // =="Directional Light"==
    MSGID_LIGHTNAME_POINT = 2803,                           // =="Point Light"==
    MSGID_LIGHTNAME_SPOT = 2804,                            // =="Spot Light"==
    MSGID_LIGHTNAME_OPENSKY = 2805,                         // =="Sky Opening"==
    MSGID_THEMATIC_HeightName = 2901,                       // =="Height"==
    MSGID_THEMATIC_HeightX = 2902,                          // =="X"==
    MSGID_THEMATIC_HeightY = 2903,                          // =="Y"==
    MSGID_THEMATIC_HeightZ = 2904,                          // =="Z"==
    MSGID_THEMATIC_SlopeName = 2905,                        // =="Slope"==
    MSGID_THEMATIC_Angle = 2906,                            // =="Angle"==
    MSGID_THEMATIC_Percent = 2907,                          // =="Percent"==
    MSGID_THEMATIC_AspectAngleName = 2908,                  // =="Aspect Angle"==
    MSGID_THEMATIC_HillShadeName = 2909,                    // =="HillShade"==
    MSGID_VIEWNAMEPREFIX_Section = 3001,                    // =="Section"==
    MSGID_VIEWNAMEPREFIX_Detail = 3002,                     // =="Detail"==
    MSGID_VIEWNAMEPREFIX_Elevation = 3003,                  // =="Elevation"==
    MSGID_VIEWNAMEPREFIX_Plan = 3004,                       // =="Plan"==
    IDS_LEVEL_CACHE_DEFAULT_LEVEL_NAME = 3050,              // =="Default"==
    IDS_DefaultLevelNameW = 3051,                           // =="Default"==
    IDS_LevelDescriptionNone = 3052,                        // =="(none)"==
    ATTACHMENT_BASE_NAME = 3060,                            // =="Ref"==
    FILE_PROGRESS_OpeningFile = 3061,                       // =="Opening file"==

    MSGID_ECPROPERTYCATEGORY_Miscellaneous = 3062,          // =="Miscellaneous"==
    MSGID_ECPROPERTYCATEGORY_General = 3063,                // =="General"==
    MSGID_ECPROPERTYCATEGORY_Extended = 3064,               // =="Extended"==
    MSGID_ECPROPERTYCATEGORY_RawData = 3065,                // =="Raw Data"==
    MSGID_ECPROPERTYCATEGORY_Geometry = 3066,               // =="Geometry"==
    MSGID_ECPROPERTYCATEGORY_Groups = 3067,                 // =="Groups"==
    MSGID_ECPROPERTYCATEGORY_Material = 3068,               // =="Material"==
    MSGID_ECPROPERTYCATEGORY_Relationships = 3069,          // =="Relationships"==

    };
BENTLEY_TRANSLATABLE_STRINGS_END

END_BENTLEY_DGNPLATFORM_NAMESPACE

