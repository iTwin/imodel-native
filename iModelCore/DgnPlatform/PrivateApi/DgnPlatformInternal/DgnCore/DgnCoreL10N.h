/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>
#include <BeSQLite/L10N.h>

BEGIN_BENTLEY_DGN_NAMESPACE

BENTLEY_TRANSLATABLE_STRINGS_START(DgnCoreL10N,dgncore_msg)
    L10N_STRING(Undone)                                         // ==" Undone"== <<must preserve leading space>>
    L10N_STRING(Redone)                                         // ==" Redone"== <<must preserve leading space>>
    L10N_STRING(UNIT_LABEL_SurveyMiles)                         // =="sm"==
    L10N_STRING(UNIT_LABEL_Miles)                               // =="mi"==
    L10N_STRING(UNIT_LABEL_Furlongs)                            // =="fur"==
    L10N_STRING(UNIT_LABEL_Chains)                              // =="ch"==
    L10N_STRING(UNIT_LABEL_Rods)                                // =="rd"==
    L10N_STRING(UNIT_LABEL_Fathoms)                             // =="fat"==
    L10N_STRING(UNIT_LABEL_Yards)                               // =="yd"==
    L10N_STRING(UNIT_LABEL_SurveyFeet)                          // =="sf"==
    L10N_STRING(UNIT_LABEL_Feet)                                // =="'"==
    L10N_STRING(UNIT_LABEL_SurveyInches)                        // =="si"==
    L10N_STRING(UNIT_LABEL_Inches)                              // =="""==
    L10N_STRING(UNIT_LABEL_Picas)                               // =="pica"==
    L10N_STRING(UNIT_LABEL_Points)                              // =="pt"==
    L10N_STRING(UNIT_LABEL_Mils)                                // =="mil"==
    L10N_STRING(UNIT_LABEL_MicroInches)                         // =="ui"==
    L10N_STRING(UNIT_LABEL_Petameters)                          // =="Pm"==
    L10N_STRING(UNIT_LABEL_Terameters)                          // =="Tm"==
    L10N_STRING(UNIT_LABEL_Gigameters)                          // =="Gm"==
    L10N_STRING(UNIT_LABEL_Megameters)                          // =="Mm"==
    L10N_STRING(UNIT_LABEL_Kilometers)                          // =="km"==
    L10N_STRING(UNIT_LABEL_Hectometers)                         // =="hm"==
    L10N_STRING(UNIT_LABEL_Dekameters)                          // =="dam"==
    L10N_STRING(UNIT_LABEL_Meters)                              // =="m"==
    L10N_STRING(UNIT_LABEL_Decimeters)                          // =="dm"==
    L10N_STRING(UNIT_LABEL_Centimeters)                         // =="cm"==
    L10N_STRING(UNIT_LABEL_Millimeters)                         // =="mm"==
    L10N_STRING(UNIT_LABEL_Micrometers)                         // =="um"==
    L10N_STRING(UNIT_LABEL_Nanometers)                          // =="nm"==
    L10N_STRING(UNIT_LABEL_Picometers)                          // =="pm"==
    L10N_STRING(UNIT_LABEL_Femtometers)                         // =="fm"==
    L10N_STRING(UNIT_LABEL_Parsecs)                             // =="pc"==
    L10N_STRING(UNIT_LABEL_LightYears)                          // =="l.y."==
    L10N_STRING(UNIT_LABEL_AstronomicalUnits)                   // =="AU"==
    L10N_STRING(UNIT_LABEL_NauticalMiles)                       // =="nm"==
    L10N_STRING(UNIT_LABEL_Angstroms)                           // =="A"==
    L10N_STRING(UNIT_LABEL_Radians)                             // =="rad"==
    L10N_STRING(UNIT_LABEL_Degrees)                             // =="deg"==
    L10N_STRING(UNIT_LABEL_Grads)                               // =="grad"==
    L10N_STRING(UNIT_LABEL_Minutes)                             // =="min"==
    L10N_STRING(UNIT_LABEL_Seconds)                             // =="sec"==
    L10N_STRING(UNIT_LABEL_UnitlessWhole)                       // =="uu"==
    L10N_STRING(UNIT_LABEL_CUSTOM)                              // =="??"==
    L10N_STRING(UNIT_LABEL_SUFFIX_Area)                         // =="2"==
    L10N_STRING(UNIT_LABEL_SUFFIX_Volume)                       // =="3"==
    L10N_STRING(UNIT_LABEL_PREFIX_Area)                         // =="Sq."==
    L10N_STRING(UNIT_LABEL_PREFIX_Volume)                       // =="Cu."==
    L10N_STRING(UNIT_SINGULAR_NAME_None)                        // =="Unitless"==
    L10N_STRING(UNIT_SINGULAR_NAME_SurveyMiles)                 // =="US Survey Mile"==
    L10N_STRING(UNIT_SINGULAR_NAME_Miles)                       // =="Mile"==
    L10N_STRING(UNIT_SINGULAR_NAME_Furlongs)                    // =="Furlong"==
    L10N_STRING(UNIT_SINGULAR_NAME_Chains)                      // =="Chain"==
    L10N_STRING(UNIT_SINGULAR_NAME_Rods)                        // =="Rod"==
    L10N_STRING(UNIT_SINGULAR_NAME_Fathoms)                     // =="Fathom"==
    L10N_STRING(UNIT_SINGULAR_NAME_Yards)                       // =="Yard"==
    L10N_STRING(UNIT_SINGULAR_NAME_SurveyFeet)                  // =="US Survey Foot"==
    L10N_STRING(UNIT_SINGULAR_NAME_Feet)                        // =="Foot"==
    L10N_STRING(UNIT_SINGULAR_NAME_SurveyInches)                // =="US Survey Inch"==
    L10N_STRING(UNIT_SINGULAR_NAME_Inches)                      // =="Inch"==
    L10N_STRING(UNIT_SINGULAR_NAME_Picas)                       // =="Pica"==
    L10N_STRING(UNIT_SINGULAR_NAME_Points)                      // =="Point"==
    L10N_STRING(UNIT_SINGULAR_NAME_Mils)                        // =="Mil"==
    L10N_STRING(UNIT_SINGULAR_NAME_MicroInches)                 // =="MicroInch"==
    L10N_STRING(UNIT_SINGULAR_NAME_Petameters)                  // =="Petameter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Terameters)                  // =="Terameter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Gigameters)                  // =="Gigameter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Megameters)                  // =="Megameter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Kilometers)                  // =="Kilometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Hectometers)                 // =="Hectometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Dekameters)                  // =="Dekameter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Meters)                      // =="Meter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Decimeters)                  // =="Decimeter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Centimeters)                 // =="Centimeter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Millimeters)                 // =="Millimeter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Micrometers)                 // =="Micrometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Nanometers)                  // =="Nanometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Picometers)                  // =="Picometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Femtometers)                 // =="Femtometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Parsecs)                     // =="Parsec"==
    L10N_STRING(UNIT_SINGULAR_NAME_LightYears)                  // =="Light Year"==
    L10N_STRING(UNIT_SINGULAR_NAME_AstronomicalUnits)           // =="Astronomical Unit"==
    L10N_STRING(UNIT_SINGULAR_NAME_NauticalMiles)               // =="Nautical Mile"==
    L10N_STRING(UNIT_SINGULAR_NAME_Angstroms)                   // =="Angstrom"==
    L10N_STRING(UNIT_SINGULAR_NAME_Radians)                     // =="Radian"==
    L10N_STRING(UNIT_SINGULAR_NAME_Degrees)                     // =="Degree"==
    L10N_STRING(UNIT_SINGULAR_NAME_Grads)                       // =="Grad"==
    L10N_STRING(UNIT_SINGULAR_NAME_Minutes)                     // =="Minute"==
    L10N_STRING(UNIT_SINGULAR_NAME_Seconds)                     // =="Second"==
    L10N_STRING(UNIT_SINGULAR_NAME_UnitlessWhole)               // =="Unitless Unit"==
    L10N_STRING(UNIT_SINGULAR_NAME_CUSTOM)                      // =="Custom"==
    L10N_STRING(UNIT_PLURAL_NAME_None)                          // =="Unitless"==
    L10N_STRING(UNIT_PLURAL_NAME_SurveyMiles)                   // =="US Survey Miles"==
    L10N_STRING(UNIT_PLURAL_NAME_Miles)                         // =="Miles"==
    L10N_STRING(UNIT_PLURAL_NAME_Furlongs)                      // =="Furlongs"==
    L10N_STRING(UNIT_PLURAL_NAME_Chains)                        // =="Chains"==
    L10N_STRING(UNIT_PLURAL_NAME_Rods)                          // =="Rods"==
    L10N_STRING(UNIT_PLURAL_NAME_Fathoms)                       // =="Fathoms"==
    L10N_STRING(UNIT_PLURAL_NAME_Yards)                         // =="Yards"==
    L10N_STRING(UNIT_PLURAL_NAME_SurveyFeet)                    // =="US Survey Feet"==
    L10N_STRING(UNIT_PLURAL_NAME_Feet)                          // =="Feet"==
    L10N_STRING(UNIT_PLURAL_NAME_SurveyInches)                  // =="US Survey Inches"==
    L10N_STRING(UNIT_PLURAL_NAME_Inches)                        // =="Inches"==
    L10N_STRING(UNIT_PLURAL_NAME_Picas)                         // =="Picas"==
    L10N_STRING(UNIT_PLURAL_NAME_Points)                        // =="Points"==
    L10N_STRING(UNIT_PLURAL_NAME_Mils)                          // =="Mils"==
    L10N_STRING(UNIT_PLURAL_NAME_MicroInches)                   // =="MicroInches"==
    L10N_STRING(UNIT_PLURAL_NAME_Petameters)                    // =="Petameters"==
    L10N_STRING(UNIT_PLURAL_NAME_Terameters)                    // =="Terameters"==
    L10N_STRING(UNIT_PLURAL_NAME_Gigameters)                    // =="Gigameters"==
    L10N_STRING(UNIT_PLURAL_NAME_Megameters)                    // =="Megameters"==
    L10N_STRING(UNIT_PLURAL_NAME_Kilometers)                    // =="Kilometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Hectometers)                   // =="Hectometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Dekameters)                    // =="Dekameters"==
    L10N_STRING(UNIT_PLURAL_NAME_Meters)                        // =="Meters"==
    L10N_STRING(UNIT_PLURAL_NAME_Decimeters)                    // =="Decimeters"==
    L10N_STRING(UNIT_PLURAL_NAME_Centimeters)                   // =="Centimeters"==
    L10N_STRING(UNIT_PLURAL_NAME_Millimeters)                   // =="Millimeters"==
    L10N_STRING(UNIT_PLURAL_NAME_Micrometers)                   // =="Micrometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Nanometers)                    // =="Nanometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Picometers)                    // =="Picometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Femtometers)                   // =="Femtometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Parsecs)                       // =="Parsecs"==
    L10N_STRING(UNIT_PLURAL_NAME_LightYears)                    // =="Light Years"==
    L10N_STRING(UNIT_PLURAL_NAME_AstronomicalUnits)             // =="Astronomical Units"==
    L10N_STRING(UNIT_PLURAL_NAME_NauticalMiles)                 // =="Nautical Miles"==
    L10N_STRING(UNIT_PLURAL_NAME_Angstroms)                     // =="Angstroms"==
    L10N_STRING(UNIT_PLURAL_NAME_Radians)                       // =="Radians"==
    L10N_STRING(UNIT_PLURAL_NAME_Degrees)                       // =="Degrees"==
    L10N_STRING(UNIT_PLURAL_NAME_Grads)                         // =="Grads"==
    L10N_STRING(UNIT_PLURAL_NAME_Minutes)                       // =="Minutes"==
    L10N_STRING(UNIT_PLURAL_NAME_Seconds)                       // =="Seconds"==
    L10N_STRING(UNIT_PLURAL_NAME_UnitlessWhole)                 // =="Unitless Units"==
    L10N_STRING(UNIT_PLURAL_NAME_CUSTOM)                        // =="Custom"==

    L10N_STRING(ACS_TYPE_NONE)                                  // =="None"==
    L10N_STRING(ACS_TYPE_RECT)                                  // =="Rectangular"==
    L10N_STRING(ACS_TYPE_CYL)                                   // =="Cylindrical"==
    L10N_STRING(ACS_TYPE_SPHERE)                                // =="Spherical"==
    L10N_STRING(ACS_TYPE_EXTEND)                                // =="Extended"==

    L10N_STRING(VIEWTITLE_MessageID_Top)                        // =="Top"==
    L10N_STRING(VIEWTITLE_MessageID_Bottom)                     // =="Bottom"==
    L10N_STRING(VIEWTITLE_MessageID_Left)                       // =="Left"==
    L10N_STRING(VIEWTITLE_MessageID_Right)                      // =="Right"==
    L10N_STRING(VIEWTITLE_MessageID_Front)                      // =="Front"==
    L10N_STRING(VIEWTITLE_MessageID_Back)                       // =="Back"==
    L10N_STRING(VIEWTITLE_MessageID_Iso)                        // =="Isometric"==
    L10N_STRING(VIEWTITLE_MessageID_RightIso)                   // =="Right Isometric"==

    L10N_STRING(VIEWFRUST_Message_InvalidWindow)                // =="Invalid window"==
    L10N_STRING(VIEWFRUST_Message_MinWindow)                    // =="Minimum window"==
    L10N_STRING(VIEWFRUST_Message_MaxWindow)                    // =="Maximum window"==
    L10N_STRING(VIEWFRUST_Message_MaxZoom  )                    // =="Maximum Zoom"==

    L10N_STRING(DISPLAY_INFO_MessageID_Model)                   // =="Model: "==
    L10N_STRING(DISPLAY_INFO_MessageID_Category)                // =="Category: "==
    L10N_STRING(DATETIME_MONTH_0)                               // =="January"==
    L10N_STRING(DATETIME_MONTH_1)                               // =="February"==
    L10N_STRING(DATETIME_MONTH_2)                               // =="March"==
    L10N_STRING(DATETIME_MONTH_3)                               // =="April"==
    L10N_STRING(DATETIME_MONTH_4)                               // =="May"==
    L10N_STRING(DATETIME_MONTH_5)                               // =="June"==
    L10N_STRING(DATETIME_MONTH_6)                               // =="July"==
    L10N_STRING(DATETIME_MONTH_7)                               // =="August"==
    L10N_STRING(DATETIME_MONTH_8)                               // =="September"==
    L10N_STRING(DATETIME_MONTH_9)                               // =="October"==
    L10N_STRING(DATETIME_MONTH_10)                              // =="November"==
    L10N_STRING(DATETIME_MONTH_11)                              // =="December"==
    L10N_STRING(DATETIME_DAY_0)                                 // =="Sunday"==
    L10N_STRING(DATETIME_DAY_1)                                 // =="Monday"==
    L10N_STRING(DATETIME_DAY_2)                                 // =="Tuesday"==
    L10N_STRING(DATETIME_DAY_3)                                 // =="Wednesday"==
    L10N_STRING(DATETIME_DAY_4)                                 // =="Thursday"==
    L10N_STRING(DATETIME_DAY_5)                                 // =="Friday"==
    L10N_STRING(DATETIME_DAY_6)                                 // =="Saturday"==
    L10N_STRING(DATETIME_MONTH_SHORT_0)                         // =="Jan"==
    L10N_STRING(DATETIME_MONTH_SHORT_1)                         // =="Feb"==
    L10N_STRING(DATETIME_MONTH_SHORT_2)                         // =="Mar"==
    L10N_STRING(DATETIME_MONTH_SHORT_3)                         // =="Apr"==
    L10N_STRING(DATETIME_MONTH_SHORT_4)                         // =="May"==
    L10N_STRING(DATETIME_MONTH_SHORT_5)                         // =="Jun"==
    L10N_STRING(DATETIME_MONTH_SHORT_6)                         // =="Jul"==
    L10N_STRING(DATETIME_MONTH_SHORT_7)                         // =="Aug"==
    L10N_STRING(DATETIME_MONTH_SHORT_8)                         // =="Sep"==
    L10N_STRING(DATETIME_MONTH_SHORT_9)                         // =="Oct"==
    L10N_STRING(DATETIME_MONTH_SHORT_10)                        // =="Nov"==
    L10N_STRING(DATETIME_MONTH_SHORT_11)                        // =="Dec"==
    L10N_STRING(DATETIME_DAY_SHORT_0)                           // =="Sun"==
    L10N_STRING(DATETIME_DAY_SHORT_1)                           // =="Mon"==
    L10N_STRING(DATETIME_DAY_SHORT_2)                           // =="Tue"==
    L10N_STRING(DATETIME_DAY_SHORT_3)                           // =="Wed"==
    L10N_STRING(DATETIME_DAY_SHORT_4)                           // =="Thu"==
    L10N_STRING(DATETIME_DAY_SHORT_5)                           // =="Fri"==
    L10N_STRING(DATETIME_DAY_SHORT_6)                           // =="Sat"==
    L10N_STRING(DATETIME_AM)                                    // =="AM"==
    L10N_STRING(DATETIME_PM)                                    // =="PM"==
    L10N_STRING(DATETIME_AM_SHORT)                              // =="A"==
    L10N_STRING(DATETIME_PM_SHORT)                              // =="P"==
    L10N_STRING(DATETIME_UTC)                                   // =="UTC"==
    L10N_STRING(VIEW_MessageID_BaseName)                        // =="Untitled"==

    L10N_STRING(REVISION_Merged)                                // =="Merged"==

    L10N_STRING(ECTYPEADAPTER_True)                             // =="True"==
    L10N_STRING(ECTYPEADAPTER_False)                            // =="False"==
    L10N_STRING(ECTYPEADAPTER_ParensNull)                       // =="(null)"==
    L10N_STRING(ECTYPEADAPTER_ParensNone)                       // =="(None)"==
    L10N_STRING(ECTYPEADAPTER_ByLevel)                          // =="ByLevel"==
    L10N_STRING(ECTYPEADAPTER_ByCell)                           // =="ByCell"==
    L10N_STRING(ECTYPEADAPTER_Bytes)                            // =="bytes"==
    L10N_STRING(ECTYPEADAPTER_KB)                               // =="KB"==
    L10N_STRING(ECTYPEADAPTER_MB)                               // =="MB"==
    L10N_STRING(ECTYPEADAPTER_GB)                               // =="GB"==
    L10N_STRING(ECTYPEADAPTER_MemberOfSingle)                   // =="Member of 1 group"==
    L10N_STRING(ECTYPEADAPTER_MemberOfPlural)                   // =="Member of %d groups"==
    L10N_STRING(ECTYPEADAPTER_SheetScaleCustom)                 // =="CUSTOM"==
    L10N_STRING(ECTYPEADAPTER_ElemClassPrimary)                 // =="Primary"==
    L10N_STRING(ECTYPEADAPTER_ElemClassPatternComponent)        // =="Pattern Component"==
    L10N_STRING(ECTYPEADAPTER_ElemClassConstruction)            // =="Construction"==
    L10N_STRING(ECTYPEADAPTER_ElemClassDimension)               // =="Dimension"==
    L10N_STRING(ECTYPEADAPTER_ElemClassPrimaryRule)             // =="Primary Rule"==
    L10N_STRING(ECTYPEADAPTER_ElemClassLinearPatterned)         // =="Linear Patterned"==
    L10N_STRING(ECTYPEADAPTER_ElemClassConstructionRule)        // =="Construction Rule"==
    L10N_STRING(ECTYPEADAPTER_Yes)                              // =="Yes"==
    L10N_STRING(ECTYPEADAPTER_No)                               // =="No"==
    L10N_STRING(ECTYPEADAPTER_On)                               // =="On"==
    L10N_STRING(ECTYPEADAPTER_Off)                              // =="Off"==
    L10N_STRING(ECTYPEADAPTER_Enabled)                          // =="Enabled"==
    L10N_STRING(ECTYPEADAPTER_Disabled)                         // =="Disabled"==
    L10N_STRING(ECTYPEADAPTER_Acres)                            // =="Acres"==
    L10N_STRING(ECTYPEADAPTER_WorkingUnits)                     // =="Working Units"==
    L10N_STRING(ECTYPEADAPTER_PropertyUnused)                   // =="<Unused>"==
    L10N_STRING(ECTYPEADAPTER_PropertyNone)                     // =="<None>"==
    L10N_STRING(ECTYPEADAPTER_NotAvailable)                     // =="Not Available"==
    L10N_STRING(ECTYPEADAPTER_GeopriorityAttachment)            // =="Attachment"==
    L10N_STRING(ECTYPEADAPTER_GeopriorityHeader)                // =="Raster Header"==
    L10N_STRING(ECTYPEADAPTER_GeoprioritySisterFile)            // =="Sister File"==
    L10N_STRING(ECTYPEADAPTER_DisplayPriority_BackPlane)        // =="Background"==
    L10N_STRING(ECTYPEADAPTER_DisplayPriority_DesignPlane)      // =="Design"==
    L10N_STRING(ECTYPEADAPTER_DisplayPriority_FrontPlane)       // =="Foreground"==
    L10N_STRING(ECTYPEADAPTER_ColorMode_Unknown)                // =="Unknown"==
    L10N_STRING(ECTYPEADAPTER_ColorMode_RGB)                    // =="RGB"==
    L10N_STRING(ECTYPEADAPTER_ColorMode_Palette16)              // =="16 Colors"==
    L10N_STRING(ECTYPEADAPTER_ColorMode_Palette256)             // =="256 Colors"==
    L10N_STRING(ECTYPEADAPTER_ColorMode_Greyscale)              // =="Grayscale"==
    L10N_STRING(ECTYPEADAPTER_ColorMode_Monochrome)             // =="Monochrome"==
    L10N_STRING(ECTYPEADAPTER_ColorMode_RGBA)                   // =="RGB Alpha"==
    L10N_STRING(ECTYPEADAPTER_ColorMode_Palette256Alpha)        // =="256 Colors with Alpha"==
    L10N_STRING(ECTYPEADAPTER_ColorMode_Greyscale16)            // =="16 bits Grayscale"==
    L10N_STRING(ECTYPEADAPTER_ColorMode_Palette2)               // =="2 Colors"==
    L10N_STRING(ECTYPEADAPTER_VariesAcross)                     // =="Varies Across"==
    L10N_STRING(ECTYPEADAPTER_NoHistory)                        // =="No History"==
    L10N_STRING(ECTYPEADAPTER_FromView)                         // =="From View"==
    L10N_STRING(ECTYPEADAPTER_SolidFill)                        // =="Filled Hidden Line"==
    L10N_STRING(ECTYPEADAPTER_HiddenLine)                       // =="Hidden Line"==
    L10N_STRING(ECTYPEADAPTER_Wireframe)                        // =="Wireframe"==
    L10N_STRING(ECTYPEADAPTER_Smooth)                           // =="Smooth"==
    L10N_STRING(ECTYPEADAPTER_RenderModeSuffix)                 // =="Display"==

    L10N_STRING(ECPROPERTYCATEGORY_Miscellaneous)               // =="Miscellaneous"==
    L10N_STRING(ECPROPERTYCATEGORY_General)                     // =="General"==
    L10N_STRING(ECPROPERTYCATEGORY_Extended)                    // =="Extended"==
    L10N_STRING(ECPROPERTYCATEGORY_RawData)                     // =="Raw Data"==
    L10N_STRING(ECPROPERTYCATEGORY_Geometry)                    // =="Geometry"==
    L10N_STRING(ECPROPERTYCATEGORY_Groups)                      // =="Groups"==
    L10N_STRING(ECPROPERTYCATEGORY_Material)                    // =="Material"==
    L10N_STRING(ECPROPERTYCATEGORY_Relationships)               // =="Relationships"==

    L10N_STRING(RepositoryStatus_ServerUnavailable)             // =="The repository server did not respond to a request"==
    L10N_STRING(RepositoryStatus_LockAlreadyHeld)               // =="A requested lock was already held by another briefcase"==
    L10N_STRING(RepositoryStatus_SyncError)                     // =="Failed to sync briefcase manager with server"==
    L10N_STRING(RepositoryStatus_InvalidResponse)               // =="Response from server not understood"==
    L10N_STRING(RepositoryStatus_PendingTransactions)           // =="An operation requires local changes to be committed or abandoned"==
    L10N_STRING(RepositoryStatus_LockUsed)                      // =="A lock cannot be relinquished because the associated object has been modified"==
    L10N_STRING(RepositoryStatus_CannotCreateRevision)          // =="An operation required creation of a DgnRevision, which failed"==
    L10N_STRING(RepositoryStatus_InvalidRequest)                // =="Request to server not understood"==
    L10N_STRING(RepositoryStatus_RevisionRequired)              // =="A revision committed to the server must be integrated into the briefcase before the operation can be completed"==
    L10N_STRING(RepositoryStatus_CodeUnavailable)               // =="A requested DgnCode is reserved by another briefcase or in use"==
    L10N_STRING(RepositoryStatus_CodeNotReserved)               // =="A DgnCode cannot be released because it has not been reserved by the requesting briefcase"==
    L10N_STRING(RepositoryStatus_CodeUsed)                      // =="A DgnCode cannot be relinquished because it has been used locally"==
    L10N_STRING(RepositoryStatus_LockNotHeld)                   // =="A required lock is not held by this briefcase"==
    L10N_STRING(RepositoryStatus_RepositoryIsLocked)            // =="Repository is currently locked, no changes allowed"==

BENTLEY_TRANSLATABLE_STRINGS_END

END_BENTLEY_DGN_NAMESPACE

