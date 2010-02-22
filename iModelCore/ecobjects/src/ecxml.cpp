/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ecxml.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECXml::StrengthToString
(
StrengthType strength
)
    {
    switch (strength)
        {
        case STRENGTHTYPE_Referencing :
            return ECXML_STRENGTH_REFERENCING;
        case STRENGTHTYPE_Holding:
            return ECXML_STRENGTH_HOLDING;
        case STRENGTHTYPE_Embedding:
            return ECXML_STRENGTH_EMBEDDING;
        default:
            return EMPTY_STRING;
           
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseStrengthType
(
StrengthType&          strength,
std::wstring const&    strengthString
)
    {
    if (0 == strengthString.length())
        return ECOBJECTS_STATUS_ParseError;
    if (0 == strengthString.compare(ECXML_STRENGTH_EMBEDDING))
        strength = STRENGTHTYPE_Embedding;
    else if (0 == strengthString.compare(ECXML_STRENGTH_HOLDING))
        strength = STRENGTHTYPE_Holding;
    else if (0 == strengthString.compare(ECXML_STRENGTH_REFERENCING))
        strength = STRENGTHTYPE_Referencing;
    else
        return ECOBJECTS_STATUS_ParseError;
        
    return ECOBJECTS_STATUS_Success;
    }
         
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECXml::DirectionToString
(
ECRelatedInstanceDirection direction
)
    {
    switch (direction)
        {
        case STRENGTHDIRECTION_Forward :
            return ECXML_DIRECTION_FORWARD;
        case STRENGTHDIRECTION_Backward:
            return ECXML_DIRECTION_BACKWARD;
        default:
            return EMPTY_STRING;
        }
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseDirectionString
(
ECRelatedInstanceDirection& direction,
std::wstring const&         directionString
)
    {
    if (0 == directionString.length())
        return ECOBJECTS_STATUS_ParseError;
    if (0 == directionString.compare(ECXML_DIRECTION_BACKWARD))
        direction = STRENGTHDIRECTION_Backward;
    else if (0 == directionString.compare(ECXML_DIRECTION_FORWARD))
        direction = STRENGTHDIRECTION_Forward;
    else
        return ECOBJECTS_STATUS_ParseError;
        
    return ECOBJECTS_STATUS_Success;
    }
        
END_BENTLEY_EC_NAMESPACE