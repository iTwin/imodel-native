/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/DgnPlatformInternal/DgnCore/DgnCoreL10N.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>
#include <BeSQLite/L10N.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

BENTLEY_TRANSLATABLE_STRINGS_START(DgnCoreL10N,dgncore_msg)
    {
    IDS_SHEETPROPERTIES_SheetLayout = 121,                          // =="Sheet Layout"==
    IDS_SectionGraphicsCut = 123,                                   // =="Section Cut"==
    IDS_SectionGraphicsForward = 124,                               // =="Forward"==
    IDS_SectionGraphicsOutside = 125,                               // =="Outside"==
    IDS_SectionGraphicsBackward = 126,                              // =="Backward"==
    IDS_UniqueNameRoot = 127,                                       // =="Untitled"==
    IDS_DISPLAYMODE_WIREFRAME = 132,                                // =="Wireframe"==
    IDS_DISPLAYMODE_HIDDENLINE = 133,                               // =="Hidden Line"==
    IDS_DISPLAYMODE_FILLEDHIDDENLINE = 134,                         // =="Filled Hidden Line"==
    IDS_DISPLAYMODE_SMOOTH = 135,                                   // =="Smooth"==
    UNDOMSG_FMT_UNDONE = 136,                                       // ==" Undone"== <<must preserve leading space>>
    UNDOMSG_FMT_REDONE = 137,                                       // ==" Redone"== <<must preserve leading space>>
    IDS_VIEWGROUP_VIEWS = 160,                                      // =="Views"==
    IDS_VIEWGROUP_TEMPVIEWS = 161,                                  // =="Temp Views"==
    IDS_NAMEDGROUP_BASE = 170,                                      // =="Group"==
    UNIT_LABEL_None = 200,                                          // =="??"==
    UNIT_LABEL_SurveyMiles = 201,                                   // =="sm"==
    UNIT_LABEL_Miles = 202,                                         // =="mi"==
    UNIT_LABEL_Furlongs = 203,                                      // =="fur"==
    UNIT_LABEL_Chains = 204,                                        // =="ch"==
    UNIT_LABEL_Rods = 205,                                          // =="rd"==
    UNIT_LABEL_Fathoms = 206,                                       // =="fat"==
    UNIT_LABEL_Yards = 207,                                         // =="yd"==
    UNIT_LABEL_SurveyFeet = 208,                                    // =="sf"==
    UNIT_LABEL_Feet = 209,                                          // =="'"==
    UNIT_LABEL_SurveyInches = 210,                                  // =="si"==
    UNIT_LABEL_Inches = 211,                                        // =="""==
    UNIT_LABEL_Picas = 212,                                         // =="pica"==
    UNIT_LABEL_Points = 213,                                        // =="pt"==
    UNIT_LABEL_Mils = 214,                                          // =="mil"==
    UNIT_LABEL_MicroInches = 215,                                   // =="ui"==
    UNIT_LABEL_Petameters = 216,                                    // =="Pm"==
    UNIT_LABEL_Terameters = 217,                                    // =="Tm"==
    UNIT_LABEL_Gigameters = 218,                                    // =="Gm"==
    UNIT_LABEL_Megameters = 219,                                    // =="Mm"==
    UNIT_LABEL_Kilometers = 220,                                    // =="km"==
    UNIT_LABEL_Hectometers = 221,                                   // =="hm"==
    UNIT_LABEL_Dekameters = 222,                                    // =="dam"==
    UNIT_LABEL_Meters = 223,                                        // =="m"==
    UNIT_LABEL_Decimeters = 224,                                    // =="dm"==
    UNIT_LABEL_Centimeters = 225,                                   // =="cm"==
    UNIT_LABEL_Millimeters = 226,                                   // =="mm"==
    UNIT_LABEL_Micrometers = 227,                                   // =="um"==
    UNIT_LABEL_Nanometers = 228,                                    // =="nm"==
    UNIT_LABEL_Picometers = 229,                                    // =="pm"==
    UNIT_LABEL_Femtometers = 230,                                   // =="fm"==
    UNIT_LABEL_Parsecs = 231,                                       // =="pc"==
    UNIT_LABEL_LightYears = 232,                                    // =="l.y."==
    UNIT_LABEL_AstronomicalUnits = 233,                             // =="AU"==
    UNIT_LABEL_NauticalMiles = 234,                                 // =="nm"==
    UNIT_LABEL_Angstroms = 235,                                     // =="A"==
    UNIT_LABEL_Radians = 236,                                       // =="rad"==
    UNIT_LABEL_Degrees = 237,                                       // =="deg"==
    UNIT_LABEL_Grads = 238,                                         // =="grad"==
    UNIT_LABEL_Minutes = 239,                                       // =="min"==
    UNIT_LABEL_Seconds = 240,                                       // =="sec"==
    UNIT_LABEL_UnitlessWhole = 241,                                 // =="uu"==
    UNIT_LABEL_CUSTOM = 242,                                        // =="??"==
    UNIT_LABEL_SUFFIX_Area = 290,                                   // =="2"==
    UNIT_LABEL_SUFFIX_Volume = 291,                                 // =="3"==
    UNIT_LABEL_PREFIX_Area = 292,                                   // =="Sq."==
    UNIT_LABEL_PREFIX_Volume = 293,                                 // =="Cu."==
    UNIT_SINGULAR_NAME_None = 300,                                  // =="Unitless"==
    UNIT_SINGULAR_NAME_SurveyMiles = 301,                           // =="US Survey Mile"==
    UNIT_SINGULAR_NAME_Miles = 302,                                 // =="Mile"==
    UNIT_SINGULAR_NAME_Furlongs = 303,                              // =="Furlong"==
    UNIT_SINGULAR_NAME_Chains = 304,                                // =="Chain"==
    UNIT_SINGULAR_NAME_Rods = 305,                                  // =="Rod"==
    UNIT_SINGULAR_NAME_Fathoms = 306,                               // =="Fathom"==
    UNIT_SINGULAR_NAME_Yards = 307,                                 // =="Yard"==
    UNIT_SINGULAR_NAME_SurveyFeet = 308,                            // =="US Survey Foot"==
    UNIT_SINGULAR_NAME_Feet = 309,                                  // =="Foot"==
    UNIT_SINGULAR_NAME_SurveyInches = 310,                          // =="US Survey Inch"==
    UNIT_SINGULAR_NAME_Inches = 311,                                // =="Inch"==
    UNIT_SINGULAR_NAME_Picas = 312,                                 // =="Pica"==
    UNIT_SINGULAR_NAME_Points = 313,                                // =="Point"==
    UNIT_SINGULAR_NAME_Mils = 314,                                  // =="Mil"==
    UNIT_SINGULAR_NAME_MicroInches = 315,                           // =="MicroInch"==
    UNIT_SINGULAR_NAME_Petameters = 316,                            // =="Petameter"==
    UNIT_SINGULAR_NAME_Terameters = 317,                            // =="Terameter"==
    UNIT_SINGULAR_NAME_Gigameters = 318,                            // =="Gigameter"==
    UNIT_SINGULAR_NAME_Megameters = 319,                            // =="Megameter"==
    UNIT_SINGULAR_NAME_Kilometers = 320,                            // =="Kilometer"==
    UNIT_SINGULAR_NAME_Hectometers = 321,                           // =="Hectometer"==
    UNIT_SINGULAR_NAME_Dekameters = 322,                            // =="Dekameter"==
    UNIT_SINGULAR_NAME_Meters = 323,                                // =="Meter"==
    UNIT_SINGULAR_NAME_Decimeters = 324,                            // =="Decimeter"==
    UNIT_SINGULAR_NAME_Centimeters = 325,                           // =="Centimeter"==
    UNIT_SINGULAR_NAME_Millimeters = 326,                           // =="Millimeter"==
    UNIT_SINGULAR_NAME_Micrometers = 327,                           // =="Micrometer"==
    UNIT_SINGULAR_NAME_Nanometers = 328,                            // =="Nanometer"==
    UNIT_SINGULAR_NAME_Picometers = 329,                            // =="Picometer"==
    UNIT_SINGULAR_NAME_Femtometers = 330,                           // =="Femtometer"==
    UNIT_SINGULAR_NAME_Parsecs = 331,                               // =="Parsec"==
    UNIT_SINGULAR_NAME_LightYears = 332,                            // =="Light Year"==
    UNIT_SINGULAR_NAME_AstronomicalUnits = 333,                     // =="Astronomical Unit"==
    UNIT_SINGULAR_NAME_NauticalMiles = 334,                         // =="Nautical Mile"==
    UNIT_SINGULAR_NAME_Angstroms = 335,                             // =="Angstrom"==
    UNIT_SINGULAR_NAME_Radians = 336,                               // =="Radian"==
    UNIT_SINGULAR_NAME_Degrees = 337,                               // =="Degree"==
    UNIT_SINGULAR_NAME_Grads = 338,                                 // =="Grad"==
    UNIT_SINGULAR_NAME_Minutes = 339,                               // =="Minute"==
    UNIT_SINGULAR_NAME_Seconds = 340,                               // =="Second"==
    UNIT_SINGULAR_NAME_UnitlessWhole = 341,                         // =="Unitless Unit"==
    UNIT_SINGULAR_NAME_CUSTOM = 342,                                // =="Custom"==
    UNIT_PLURAL_NAME_None = 400,                                    // =="Unitless"==
    UNIT_PLURAL_NAME_SurveyMiles = 401,                             // =="US Survey Miles"==
    UNIT_PLURAL_NAME_Miles = 402,                                   // =="Miles"==
    UNIT_PLURAL_NAME_Furlongs = 403,                                // =="Furlongs"==
    UNIT_PLURAL_NAME_Chains = 404,                                  // =="Chains"==
    UNIT_PLURAL_NAME_Rods = 405,                                    // =="Rods"==
    UNIT_PLURAL_NAME_Fathoms = 406,                                 // =="Fathoms"==
    UNIT_PLURAL_NAME_Yards = 407,                                   // =="Yards"==
    UNIT_PLURAL_NAME_SurveyFeet = 408,                              // =="US Survey Feet"==
    UNIT_PLURAL_NAME_Feet = 409,                                    // =="Feet"==
    UNIT_PLURAL_NAME_SurveyInches = 410,                            // =="US Survey Inches"==
    UNIT_PLURAL_NAME_Inches = 411,                                  // =="Inches"==
    UNIT_PLURAL_NAME_Picas = 412,                                   // =="Picas"==
    UNIT_PLURAL_NAME_Points = 413,                                  // =="Points"==
    UNIT_PLURAL_NAME_Mils = 414,                                    // =="Mils"==
    UNIT_PLURAL_NAME_MicroInches = 415,                             // =="MicroInches"==
    UNIT_PLURAL_NAME_Petameters = 416,                              // =="Petameters"==
    UNIT_PLURAL_NAME_Terameters = 417,                              // =="Terameters"==
    UNIT_PLURAL_NAME_Gigameters = 418,                              // =="Gigameters"==
    UNIT_PLURAL_NAME_Megameters = 419,                              // =="Megameters"==
    UNIT_PLURAL_NAME_Kilometers = 420,                              // =="Kilometers"==
    UNIT_PLURAL_NAME_Hectometers = 421,                             // =="Hectometers"==
    UNIT_PLURAL_NAME_Dekameters = 422,                              // =="Dekameters"==
    UNIT_PLURAL_NAME_Meters = 423,                                  // =="Meters"==
    UNIT_PLURAL_NAME_Decimeters = 424,                              // =="Decimeters"==
    UNIT_PLURAL_NAME_Centimeters = 425,                             // =="Centimeters"==
    UNIT_PLURAL_NAME_Millimeters = 426,                             // =="Millimeters"==
    UNIT_PLURAL_NAME_Micrometers = 427,                             // =="Micrometers"==
    UNIT_PLURAL_NAME_Nanometers = 428,                              // =="Nanometers"==
    UNIT_PLURAL_NAME_Picometers = 429,                              // =="Picometers"==
    UNIT_PLURAL_NAME_Femtometers = 430,                             // =="Femtometers"==
    UNIT_PLURAL_NAME_Parsecs = 431,                                 // =="Parsecs"==
    UNIT_PLURAL_NAME_LightYears = 432,                              // =="Light Years"==
    UNIT_PLURAL_NAME_AstronomicalUnits = 433,                       // =="Astronomical Units"==
    UNIT_PLURAL_NAME_NauticalMiles = 434,                           // =="Nautical Miles"==
    UNIT_PLURAL_NAME_Angstroms = 435,                               // =="Angstroms"==
    UNIT_PLURAL_NAME_Radians = 436,                                 // =="Radians"==
    UNIT_PLURAL_NAME_Degrees = 437,                                 // =="Degrees"==
    UNIT_PLURAL_NAME_Grads = 438,                                   // =="Grads"==
    UNIT_PLURAL_NAME_Minutes = 439,                                 // =="Minutes"==
    UNIT_PLURAL_NAME_Seconds = 440,                                 // =="Seconds"==
    UNIT_PLURAL_NAME_UnitlessWhole = 441,                           // =="Unitless Units"==
    UNIT_PLURAL_NAME_CUSTOM = 442,                                  // =="Custom"==

    ACS_TYPE_BASE_ = 540,                                           // ==NoMessage==
    ACS_TYPE_NONE = ACS_TYPE_BASE_+0,                               // =="None"==
    ACS_TYPE_RECT = ACS_TYPE_BASE_+1,                               // =="Rectangular"==
    ACS_TYPE_CYL = ACS_TYPE_BASE_+2,                                // =="Cylindrical"==
    ACS_TYPE_SPHERE = ACS_TYPE_BASE_+3,                             // =="Spherical"==

    VIEWTITLE_MessageID_Top = 550,                                  // =="Top"==
    VIEWTITLE_MessageID_Bottom = 551,                               // =="Bottom"==
    VIEWTITLE_MessageID_Left = 552,                                 // =="Left"==
    VIEWTITLE_MessageID_Right = 553,                                // =="Right"==
    VIEWTITLE_MessageID_Front = 554,                                // =="Front"==
    VIEWTITLE_MessageID_Back = 555,                                 // =="Back"==
    VIEWTITLE_MessageID_Iso = 556,                                  // =="Isometric"==
    VIEWTITLE_MessageID_RightIso = 557,                             // =="Right Isometric"==

    VIEWFRUST_MessageBase = 560,                                    // ==NoMessage==
    VIEWFRUST_Message_InvalidWindow    = VIEWFRUST_MessageBase + 1, // =="Invalid window"==
    VIEWFRUST_Message_MinWindow = VIEWFRUST_MessageBase + 2,        // =="Minimum window"==
    VIEWFRUST_Message_MaxWindow = VIEWFRUST_MessageBase + 3,        // =="Maximum window"==
    VIEWFRUST_Message_MaxZoom   = VIEWFRUST_MessageBase + 4,        // =="Maximum Zoom"==
    
    DISPLAY_INFO_MessageID_Level = 600,                             // =="Level: "==
    DISPLAY_INFO_MessageID_Model = 601,                             // =="Model: "==
    DISPLAY_INFO_MessageID_GG = 602,                                // =="GG"==
    DISPLAY_INFO_MessageID_Groups = 603,                            // =="Groups= "==
    DISPLAY_INFO_MessageID_Transient = 604,                         // =="<transient>"==
    HARD_CODED_DGN_FONT_NAMED_SYMBOL_Unknown = 605,                 // =="(Unnamed)"==
    HARD_CODED_DGN_FONT_NAMED_SYMBOL_Degree = 606,                  // =="Degree"==
    HARD_CODED_DGN_FONT_NAMED_SYMBOL_Diameter = 607,                // =="Diameter"==
    HARD_CODED_DGN_FONT_NAMED_SYMBOL_PlusMinus = 608,               // =="Plus/Minus"==
    HARD_CODED_DGN_FONT_NAMED_SYMBOL_CenterLine = 609,              // =="Center Line"==
    SCALEDEFINITION_MSGID_SheetScaleCustom = 618,                   // =="CUSTOM"==
    DATETIME_MONTH_0 = 620,                                         // =="January"==
    DATETIME_MONTH_1 = 621,                                         // =="February"==
    DATETIME_MONTH_2 = 622,                                         // =="March"==
    DATETIME_MONTH_3 = 623,                                         // =="April"==
    DATETIME_MONTH_4 = 624,                                         // =="May"==
    DATETIME_MONTH_5 = 625,                                         // =="June"==
    DATETIME_MONTH_6 = 626,                                         // =="July"==
    DATETIME_MONTH_7 = 627,                                         // =="August"==
    DATETIME_MONTH_8 = 628,                                         // =="September"==
    DATETIME_MONTH_9 = 629,                                         // =="October"==
    DATETIME_MONTH_10 = 630,                                        // =="November"==
    DATETIME_MONTH_11 = 631,                                        // =="December"==
    DATETIME_DAY_0 = 632,                                           // =="Sunday"==
    DATETIME_DAY_1 = 633,                                           // =="Monday"==
    DATETIME_DAY_2 = 634,                                           // =="Tuesday"==
    DATETIME_DAY_3 = 635,                                           // =="Wednesday"==
    DATETIME_DAY_4 = 636,                                           // =="Thursday"==
    DATETIME_DAY_5 = 637,                                           // =="Friday"==
    DATETIME_DAY_6 = 638,                                           // =="Saturday"==
    DATETIME_MONTH_SHORT_0 = 639,                                   // =="Jan"==
    DATETIME_MONTH_SHORT_1 = 640,                                   // =="Feb"==
    DATETIME_MONTH_SHORT_2 = 641,                                   // =="Mar"==
    DATETIME_MONTH_SHORT_3 = 642,                                   // =="Apr"==
    DATETIME_MONTH_SHORT_4 = 643,                                   // =="May"==
    DATETIME_MONTH_SHORT_5 = 644,                                   // =="Jun"==
    DATETIME_MONTH_SHORT_6 = 645,                                   // =="Jul"==
    DATETIME_MONTH_SHORT_7 = 646,                                   // =="Aug"==
    DATETIME_MONTH_SHORT_8 = 647,                                   // =="Sep"==
    DATETIME_MONTH_SHORT_9 = 648,                                   // =="Oct"==
    DATETIME_MONTH_SHORT_10 = 649,                                  // =="Nov"==
    DATETIME_MONTH_SHORT_11 = 650,                                  // =="Dec"==
    DATETIME_DAY_SHORT_0 = 651,                                     // =="Sun"==
    DATETIME_DAY_SHORT_1 = 652,                                     // =="Mon"==
    DATETIME_DAY_SHORT_2 = 653,                                     // =="Tue"==
    DATETIME_DAY_SHORT_3 = 654,                                     // =="Wed"==
    DATETIME_DAY_SHORT_4 = 655,                                     // =="Thu"==
    DATETIME_DAY_SHORT_5 = 656,                                     // =="Fri"==
    DATETIME_DAY_SHORT_6 = 657,                                     // =="Sat"==
    DATETIME_AM = 658,                                              // =="AM"==
    DATETIME_PM = 659,                                              // =="PM"==
    DATETIME_AM_SHORT = 660,                                        // =="A"==
    DATETIME_PM_SHORT = 661,                                        // =="P"==
    DATETIME_UTC = 662,                                             // =="UTC"==
    VIEW_MessageID_BaseName = 663,                                  // =="Untitled"==
    IDS_LevelName = 664,                                            // =="Level "==
    IDS_LevelDescriptionNone = 665,                                 // =="(none)"==
    IDS_SectionGraphicsOther = 666,                                 // ==""==
    };
BENTLEY_TRANSLATABLE_STRINGS_END

END_BENTLEY_DGNPLATFORM_NAMESPACE

