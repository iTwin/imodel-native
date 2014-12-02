/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/TextFieldConstants.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "TextFieldConstants.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

 WCharCP TextFieldConstants::XMLTAG_Enabler          = L"Enabler";
 WCharCP TextFieldConstants::XMLTAG_Class            = L"Class";
 WCharCP TextFieldConstants::XMLTAG_AccessString     = L"Access";
 WCharCP TextFieldConstants::XMLTAG_Element          = L"Element";
 WCharCP TextFieldConstants::XMLTAG_ID               = L"ID";
 WCharCP TextFieldConstants::XMLTAG_Expression       = L"Expression";
 WCharCP TextFieldConstants::XMLTAG_Evaluator        = L"Evaluator";
 WCharCP TextFieldConstants::XMLTAG_ACEvaluator      = L"AC_Evaluator";
 WCharCP TextFieldConstants::XMLTAG_ACFormat         = L"AC_Format";
 WCharCP TextFieldConstants::XMLTAG_ACAccessString   = L"AC_Access";
 WCharCP TextFieldConstants::XMLTAG_Value            = L"Value";
 WCharCP TextFieldConstants::XMLTAG_Dependents       = L"Dependents";
 WCharCP TextFieldConstants::XMLTAG_ModelName        = L"ModelName";
 WCharCP TextFieldConstants::XMLTAG_FileName         = L"FileName";
 WCharCP TextFieldConstants::XMLTAG_InvalidDepLinkage = L"InvalidDepLinkage";
 WCharCP TextFieldConstants::XMLTAG_NoRootsInDepLinkage = L"NoRootsInDepLinkage";
 WCharCP TextFieldConstants::XMLTAG_HandlerData      = L"HandlerData";
 WCharCP TextFieldConstants::XMLTAG_HandlerKey       = L"HandlerKey";
 WCharCP TextFieldConstants::XMLTAG_LinkAncestorKey  = L"DemoTargetLinkAncestorKey";
 WCharCP TextFieldConstants::XMLTAG_TargetFileSaveTime = L"TargetFileSaveTime" ;

 WCharCP TextFieldConstants::VALUETYPE_Double       = L"Double";
 WCharCP TextFieldConstants::VALUETYPE_DateTime     = L"DateTime";
 WCharCP TextFieldConstants::VALUETYPE_Int64        = L"Int64";
 WCharCP TextFieldConstants::VALUETYPE_Int          = L"Int";
 WCharCP TextFieldConstants::VALUETYPE_String       = L"String";
 WCharCP TextFieldConstants::VALUETYPE_ID           = L"ID";

 Int64   TextFieldConstants::TICKADJUSTMENT = 504911232000000000LL;     // ticks between 01/01/01 and 01/01/1601

 WCharCP TextFieldConstants::HANDLERID_Element              = L"ElemProp";
 WCharCP TextFieldConstants::HANDLERID_File                 = L"ECFileProp";
 WCharCP TextFieldConstants::HANDLERID_Model                = L"ModelProp";
 WCharCP TextFieldConstants::HANDLERID_PrintSet             = L"ECPrintSetProp";
 WCharCP TextFieldConstants::HANDLERID_PrintDefinition      = L"ECPrintDefProp";
 WCharCP TextFieldConstants::HANDLERID_Link                 = L"LinkProp";
 WCharCP TextFieldConstants::HANDLERID_PlaceHolderLink      = L"PlaceHolderLinkProp";
 WCharCP TextFieldConstants::HANDLERID_PlaceHolderCell      = L"PlaceHolderCellProp";
 WCharCP TextFieldConstants::HANDLERID_PlaceHolderSignatureCell = L"PlaceholderDgnSignatureCellProp";
 WCharCP TextFieldConstants::HANDLERID_DwgProxy             = L"DwgProxy";

 WCharCP TextFieldConstants::INVALID_FIELD_INDICATOR        = L"####";

 WCharCP TextFieldConstants::FORMATTER_CASE                     = L"Case";
 WCharCP TextFieldConstants::FORMATTER_DECIMAL_SEPARATOR        = L"DecimalSeparator";
 WCharCP TextFieldConstants::FORMATTER_LEADING_ZERO             = L"LeadingZero";
 WCharCP TextFieldConstants::FORMATTER_TRAILING_ZEROS           = L"TrailingZeros";
 WCharCP TextFieldConstants::FORMATTER_THOUSANDS_SEPARATOR      = L"TS";
 WCharCP TextFieldConstants::FORMATTER_LIST_SEPARATOR           = L"LS";             // For points
 WCharCP TextFieldConstants::FORMATTER_SUFFIX                   = L"SX";
 WCharCP TextFieldConstants::FORMATTER_PREFIX                   = L"PX";
 WCharCP TextFieldConstants::FORMATTER_ZERO_MASTER_UNITS        = L"ZMU";
 WCharCP TextFieldConstants::FORMATTER_ZERO_SECONDARY_UNITS     = L"ZSU";
 WCharCP TextFieldConstants::FORMATTER_CONVERSION_FACTOR        = L"CF";
 WCharCP TextFieldConstants::FORMATTER_ACCURACY                = L"Accuracy";
 WCharCP TextFieldConstants::FORMATTER_UNITS                   = L"Units";
 WCharCP TextFieldConstants::FORMATTER_MASTER_UNITS            = L"MasterUnits";
 WCharCP TextFieldConstants::FORMATTER_SECONDARY_UNITS         = L"SecondaryUnits";
 WCharCP TextFieldConstants::FORMATTER_ALLOW_NEGATIVE          = L"AllowNegative";
 WCharCP TextFieldConstants::FORMATTER_SHOW_LABEL              = L"ShowLabel";
 WCharCP TextFieldConstants::FORMATTER_MODE                    = L"Mode";
 WCharCP TextFieldConstants::FORMATTER_LABEL_FORMAT            = L"LabelFormat";
 WCharCP TextFieldConstants::FORMATTER_DATETIMEFORMAT          = L"DateTimeFormat";
 WCharCP TextFieldConstants::FORMATTER_ROOT                    = L"Root";
 WCharCP TextFieldConstants::FORMATTER_PATH                    = L"Path";
 WCharCP TextFieldConstants::FORMATTER_EXTENSION               = L"Extension";
 WCharCP TextFieldConstants::FORMATTER_COORDINATES             = L"Coordinates";
 WCharCP TextFieldConstants::FORMATTER_FORMAT                  = L"Format";
 WCharCP TextFieldConstants::FORMATTER_BOOL_WORD               = L"BoolWord";
 WCharCP TextFieldConstants::FORMATTER_INSTANCE_ID             = L"instanceId";
 WCharCP TextFieldConstants::FORMATTER_MASTERBASE              = L"MasterBase";
 WCharCP TextFieldConstants::FORMATTER_MASTERSYSTEM            = L"MasterSystem";
 WCharCP TextFieldConstants::FORMATTER_MASTERNUMERATOR         = L"MasterNumerator";
 WCharCP TextFieldConstants::FORMATTER_MASTERDENOMINATOR       = L"MasterDenominator";
 WCharCP TextFieldConstants::FORMATTER_MASTERLABEL             = L"MasterLabel";
 WCharCP TextFieldConstants::FORMATTER_SECONDARYBASE           = L"SecondaryBase";
 WCharCP TextFieldConstants::FORMATTER_SECONDARYSYSTEM         = L"SecondarySystem";
 WCharCP TextFieldConstants::FORMATTER_SECONDARYNUMERATOR      = L"SecondaryNumerator";
 WCharCP TextFieldConstants::FORMATTER_SECONDARYDENOMINATOR    = L"SecondaryDenominator";
 WCharCP TextFieldConstants::FORMATTER_SECONDARYLABEL          = L"SecondaryLabel";
 WCharCP TextFieldConstants::FORMATTER_C_FORMAT_STRING         = L"CFormatString";
 WCharCP TextFieldConstants::FORMATTER_FORMAT_INDEX            = L"FormatIndex";
 WCharCP TextFieldConstants::FORMATTER_LU_FORMAT               = L"LuFmt";                  //  DWG LUNITS format

 WCharCP TextFieldConstants::FORMATTER_ANGLE_CLASS             = L"AngleClass";
 WCharCP TextFieldConstants::FORMATTER_DWG_ANGLE_CLASS         = L"DAng";
 WCharCP TextFieldConstants::FORMATTER_DIRECTION_CLASS         = L"DirectionClass";
 WCharCP TextFieldConstants::FORMATTER_AREA_CLASS              = L"AreaClass";
 WCharCP TextFieldConstants::FORMATTER_VOLUME_CLASS            = L"Vol";
 WCharCP TextFieldConstants::FORMATTER_DWG_AREA_CLASS          = L"DArea";
 WCharCP TextFieldConstants::FORMATTER_DISTANCE_CLASS          = L"DistanceClass";
 WCharCP TextFieldConstants::FORMATTER_DWG_DISTANCE_CLASS      = L"DDist";
 WCharCP TextFieldConstants::FORMATTER_DWG_ENUMERATED_CLASS    = L"DEn";
 WCharCP TextFieldConstants::FORMATTER_FILE_NAME_CLASS         = L"FileNameClass";
 WCharCP TextFieldConstants::FORMATTER_COORDINATE_CLASS        = L"CoordinateClass";
 WCharCP TextFieldConstants::FORMATTER_DWG_COORDINATE_CLASS    = L"DCoord";
 WCharCP TextFieldConstants::FORMATTER_POINT3D_CLASS           = L"Point3dClass";
 WCharCP TextFieldConstants::FORMATTER_DWG_POINT3D_CLASS       = L"DP3d";
 WCharCP TextFieldConstants::FORMATTER_STRING_CLASS            = L"StringClass";
 WCharCP TextFieldConstants::FORMATTER_BOOLEAN_CLASS           = L"BooleanClass";
 WCharCP TextFieldConstants::FORMATTER_CLASS_FILESIZE          = L"FilesizeClass";  //  Only used for DWG
 WCharCP TextFieldConstants::FORMATTER_DOUBLE_CLASS            = L"DoubleClass";
 WCharCP TextFieldConstants::FORMATTER_INTEGER_CLASS           = L"IntegerClass";
 WCharCP TextFieldConstants::FORMATTER_DATETIME_CLASS          = L"DateTimeFormat";

 WCharCP TextFieldConstants::FORMATTER_SQUARE_FEET_SUFFIX      = L"SquareFeet";
 WCharCP TextFieldConstants::FORMATTER_ACRES                   = L"Acres";
 WCharCP TextFieldConstants::FORMATTER_PROPERTY_FORMATTER      = L"Bentley.PropertyFormatter.01.00";
 WCharCP TextFieldConstants::FORMATTER_PROPERTY_FORMATTER2              = L"Bentley.PropertyFormatter.02.00";

END_BENTLEY_DGNPLATFORM_NAMESPACE

