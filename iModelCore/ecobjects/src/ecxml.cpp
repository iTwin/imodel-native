/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ecxml.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseBooleanString
(
bool & booleanValue,
const wchar_t * booleanString
)
    {
    if (0 == wcsicmp (booleanString, ECXML_TRUE))
        booleanValue = true;
    else if (0 == wcsicmp (booleanString, ECXML_FALSE))
        booleanValue = false;
    else
        return ECOBJECTS_STATUS_ParseError;

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECXml::GetPrimitiveTypeName
(
PrimitiveType primitiveType
)
    {
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Binary:
            return ECXML_TYPENAME_BINARY;
        case PRIMITIVETYPE_Boolean:
            return ECXML_TYPENAME_BOOLEAN;
        case PRIMITIVETYPE_DateTime:
            return ECXML_TYPENAME_DATETIME;
        case PRIMITIVETYPE_Double:
            return ECXML_TYPENAME_DOUBLE;
        case PRIMITIVETYPE_Integer:
            return ECXML_TYPENAME_INTEGER;
        case PRIMITIVETYPE_Long:
            return ECXML_TYPENAME_LONG;
        case PRIMITIVETYPE_Point2D:
            return ECXML_TYPENAME_POINT2D;
        case PRIMITIVETYPE_Point3D:
            return ECXML_TYPENAME_POINT3D;
        case PRIMITIVETYPE_String:
            return ECXML_TYPENAME_STRING;
        default:
            return EMPTY_STRING;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParsePrimitiveType
(
PrimitiveType&          primitiveType,
std::wstring const&     typeName
)
    {
    if (0 == typeName.length())
        return ECOBJECTS_STATUS_ParseError;

    if (0 == typeName.compare (ECXML_TYPENAME_STRING))
        primitiveType = PRIMITIVETYPE_String;
    else if (0 == typeName.compare (ECXML_TYPENAME_INTEGER))
        primitiveType = PRIMITIVETYPE_Integer;
    else if (0 == typeName.compare (ECXML_TYPENAME_LONG))
        primitiveType = PRIMITIVETYPE_Long;
    else if (0 == typeName.compare (ECXML_TYPENAME_BOOLEAN))
        primitiveType = PRIMITIVETYPE_Boolean;
    else if (0 == typeName.compare (ECXML_TYPENAME_DOUBLE))
        primitiveType = PRIMITIVETYPE_Double;
    else if (0 == typeName.compare (ECXML_TYPENAME_POINT2D))
        primitiveType = PRIMITIVETYPE_Point2D;
    else if (0 == typeName.compare (ECXML_TYPENAME_POINT3D))
        primitiveType = PRIMITIVETYPE_Point3D;
    else if (0 == typeName.compare (ECXML_TYPENAME_DATETIME))
        primitiveType = PRIMITIVETYPE_DateTime;
    else if (0 == typeName.compare (ECXML_TYPENAME_BINARY))
        primitiveType = PRIMITIVETYPE_Binary;
    else
        return ECOBJECTS_STATUS_ParseError;
        
    return ECOBJECTS_STATUS_Success;
    }

END_BENTLEY_EC_NAMESPACE