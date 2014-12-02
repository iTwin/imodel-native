/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/TextFieldConstants.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace TextFieldConstants
{

extern  WCharCP XMLTAG_Enabler;
extern  WCharCP XMLTAG_Class;
extern  WCharCP XMLTAG_AccessString;
extern  WCharCP XMLTAG_Element;
extern  WCharCP XMLTAG_ID;
extern  WCharCP XMLTAG_Expression;
extern  WCharCP XMLTAG_Evaluator;
extern  WCharCP XMLTAG_ACEvaluator;
extern  WCharCP XMLTAG_ACFormat;
extern  WCharCP XMLTAG_ACAccessString;
extern  WCharCP XMLTAG_Value;
extern  WCharCP XMLTAG_Dependents;
extern  WCharCP XMLTAG_ModelName;
extern  WCharCP XMLTAG_FileName;
extern  WCharCP XMLTAG_InvalidDepLinkage;
extern  WCharCP XMLTAG_NoRootsInDepLinkage;
extern  WCharCP XMLTAG_HandlerData;
extern  WCharCP XMLTAG_HandlerKey;
extern  WCharCP XMLTAG_LinkAncestorKey;
extern  WCharCP XMLTAG_TargetFileSaveTime;

extern  WCharCP VALUETYPE_Double;
extern  WCharCP VALUETYPE_DateTime;
extern  WCharCP VALUETYPE_Int64;
extern  WCharCP VALUETYPE_Int;
extern  WCharCP VALUETYPE_String;
extern  WCharCP VALUETYPE_ID;

extern  Int64 TICKADJUSTMENT;     // ticks between 01/01/01 and 01/01/1601

extern  WCharCP HANDLERID_Element;
extern  WCharCP HANDLERID_File;
extern  WCharCP HANDLERID_Model;
extern  WCharCP HANDLERID_PrintSet;
extern  WCharCP HANDLERID_PrintDefinition;
extern  WCharCP HANDLERID_Link;
extern  WCharCP HANDLERID_PlaceHolderLink;
extern  WCharCP HANDLERID_PlaceHolderCell;
extern  WCharCP HANDLERID_PlaceHolderSignatureCell;
extern  WCharCP HANDLERID_DwgProxy;

extern  WCharCP INVALID_FIELD_INDICATOR;

extern  WCharCP FORMATTER_CASE;
extern  WCharCP FORMATTER_DECIMAL_SEPARATOR;
extern  WCharCP FORMATTER_LEADING_ZERO;
extern  WCharCP FORMATTER_TRAILING_ZEROS;
extern  WCharCP FORMATTER_THOUSANDS_SEPARATOR;
extern  WCharCP FORMATTER_LIST_SEPARATOR;
extern  WCharCP FORMATTER_SUFFIX;
extern  WCharCP FORMATTER_PREFIX;
extern  WCharCP FORMATTER_ZERO_MASTER_UNITS;
extern  WCharCP FORMATTER_ZERO_SECONDARY_UNITS;
extern  WCharCP FORMATTER_CONVERSION_FACTOR;
extern  WCharCP FORMATTER_ACCURACY;
extern  WCharCP FORMATTER_UNITS;
extern  WCharCP FORMATTER_MASTER_UNITS;
extern  WCharCP FORMATTER_SECONDARY_UNITS;
extern  WCharCP FORMATTER_ALLOW_NEGATIVE;
extern  WCharCP FORMATTER_SHOW_LABEL;
extern  WCharCP FORMATTER_MODE;
extern  WCharCP FORMATTER_LABEL_FORMAT;
extern  WCharCP FORMATTER_DATETIMEFORMAT;
extern  WCharCP FORMATTER_ROOT;
extern  WCharCP FORMATTER_PATH;
extern  WCharCP FORMATTER_EXTENSION;
extern  WCharCP FORMATTER_COORDINATES;
extern  WCharCP FORMATTER_FORMAT;
extern  WCharCP FORMATTER_BOOL_WORD;
extern  WCharCP FORMATTER_INSTANCE_ID;
extern  WCharCP FORMATTER_MASTERBASE;
extern  WCharCP FORMATTER_MASTERSYSTEM;
extern  WCharCP FORMATTER_MASTERNUMERATOR;
extern  WCharCP FORMATTER_MASTERDENOMINATOR;
extern  WCharCP FORMATTER_MASTERLABEL;
extern  WCharCP FORMATTER_SECONDARYBASE;
extern  WCharCP FORMATTER_SECONDARYSYSTEM;
extern  WCharCP FORMATTER_SECONDARYNUMERATOR;
extern  WCharCP FORMATTER_SECONDARYDENOMINATOR;
extern  WCharCP FORMATTER_SECONDARYLABEL;
extern  WCharCP FORMATTER_C_FORMAT_STRING;
extern  WCharCP FORMATTER_FORMAT_INDEX;
extern  WCharCP FORMATTER_LU_FORMAT;                  //  DWG LUNITS format

extern  WCharCP FORMATTER_ANGLE_CLASS;
extern  WCharCP FORMATTER_DWG_ANGLE_CLASS;
extern  WCharCP FORMATTER_DIRECTION_CLASS;
extern  WCharCP FORMATTER_AREA_CLASS;
extern  WCharCP FORMATTER_VOLUME_CLASS;
extern  WCharCP FORMATTER_DWG_AREA_CLASS;
extern  WCharCP FORMATTER_DISTANCE_CLASS;
extern  WCharCP FORMATTER_DWG_DISTANCE_CLASS;
extern  WCharCP FORMATTER_DWG_ENUMERATED_CLASS;
extern  WCharCP FORMATTER_FILE_NAME_CLASS;
extern  WCharCP FORMATTER_COORDINATE_CLASS;
extern  WCharCP FORMATTER_DWG_COORDINATE_CLASS;
extern  WCharCP FORMATTER_POINT3D_CLASS;
extern  WCharCP FORMATTER_DWG_POINT3D_CLASS;
extern  WCharCP FORMATTER_STRING_CLASS;
extern  WCharCP FORMATTER_BOOLEAN_CLASS;
extern  WCharCP FORMATTER_CLASS_FILESIZE;  //  Only used for DWG
extern  WCharCP FORMATTER_DOUBLE_CLASS;
extern  WCharCP FORMATTER_INTEGER_CLASS;
extern  WCharCP FORMATTER_DATETIME_CLASS;

extern  WCharCP FORMATTER_SQUARE_FEET_SUFFIX;
extern  WCharCP FORMATTER_ACRES;
extern  WCharCP FORMATTER_PROPERTY_FORMATTER;
extern  WCharCP FORMATTER_PROPERTY_FORMATTER2;

}   // namespace TextFieldConstants

END_BENTLEY_DGNPLATFORM_NAMESPACE

