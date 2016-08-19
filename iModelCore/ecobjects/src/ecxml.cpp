/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ecxml.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ctype.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseBooleanString (bool & booleanValue, Utf8CP booleanString)
    {
    if (0 == BeStringUtilities::StricmpAscii(booleanString, ECXML_TRUE))
        booleanValue = true;
    else if (0 == BeStringUtilities::StricmpAscii(booleanString, ECXML_FALSE))
        booleanValue = false;
    else
        return ECObjectsStatus::ParseError;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECXml::GetPrimitiveTypeName (PrimitiveType primitiveType)
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
        case PRIMITIVETYPE_IGeometry:
            return ECXML_TYPENAME_IGEOMETRY_LEGACY_GENERIC;
        default:
            return EMPTY_STRING;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParsePrimitiveType (PrimitiveType& primitiveType, Utf8StringCR typeName)
    {
    if (typeName.empty())
        return ECObjectsStatus::ParseError;

    if (typeName.EqualsIAscii (ECXML_TYPENAME_STRING))
        primitiveType = PRIMITIVETYPE_String;
    else if (typeName.EqualsIAscii(ECXML_TYPENAME_INTEGER))
        primitiveType = PRIMITIVETYPE_Integer;
    else if (typeName.EqualsIAscii(ECXML_TYPENAME_LONG))
        primitiveType = PRIMITIVETYPE_Long;
    else if (typeName.EqualsIAscii(ECXML_TYPENAME_BOOLEAN))
        primitiveType = PRIMITIVETYPE_Boolean;
    else if (typeName.EqualsIAscii(ECXML_TYPENAME_BOOL))
        primitiveType = PRIMITIVETYPE_Boolean;
    else if (typeName.EqualsIAscii(ECXML_TYPENAME_DOUBLE))
        primitiveType = PRIMITIVETYPE_Double;
    else if (typeName.EqualsIAscii(ECXML_TYPENAME_POINT2D))
        primitiveType = PRIMITIVETYPE_Point2D;
    else if (typeName.EqualsIAscii(ECXML_TYPENAME_POINT3D))
        primitiveType = PRIMITIVETYPE_Point3D;
    else if (typeName.EqualsIAscii(ECXML_TYPENAME_DATETIME))
        primitiveType = PRIMITIVETYPE_DateTime;
    else if (typeName.EqualsIAscii(ECXML_TYPENAME_BINARY))
        primitiveType = PRIMITIVETYPE_Binary;
    else if (typeName.StartsWithIAscii(ECXML_TYPENAME_IGEOMETRY_GENERIC))
        primitiveType = PRIMITIVETYPE_IGeometry; 
    else if (typeName.StartsWithIAscii(ECXML_TYPENAME_IGEOMETRY_LEGACY))
        primitiveType = PRIMITIVETYPE_IGeometry; 
    else
        return ECObjectsStatus::ParseError;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECXml::StrengthToString (StrengthType strength)
    {
    switch (strength)
        {
        case StrengthType::Referencing :
            return ECXML_STRENGTH_REFERENCING;
        case StrengthType::Holding:
            return ECXML_STRENGTH_HOLDING;
        case StrengthType::Embedding:
            return ECXML_STRENGTH_EMBEDDING;
        default:
            return EMPTY_STRING;
           
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseStrengthType (StrengthType& strength, Utf8StringCR strengthString)
    {
    if (0 == strengthString.length())
        return ECObjectsStatus::ParseError;
    if (0 == strengthString.CompareToI(ECXML_STRENGTH_EMBEDDING))
        strength = StrengthType::Embedding;
    else if (0 == strengthString.CompareToI(ECXML_STRENGTH_HOLDING))
        strength = StrengthType::Holding;
    else if (0 == strengthString.CompareToI(ECXML_STRENGTH_REFERENCING))
        strength = StrengthType::Referencing;
    else
        return ECObjectsStatus::ParseError;
        
    return ECObjectsStatus::Success;
    }
         
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECXml::DirectionToString (ECRelatedInstanceDirection direction)
    {
    switch (direction)
        {
        case ECRelatedInstanceDirection::Forward :
            return ECXML_DIRECTION_FORWARD;
        case ECRelatedInstanceDirection::Backward:
            return ECXML_DIRECTION_BACKWARD;
        default:
            return EMPTY_STRING;
        }
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseDirectionString (ECRelatedInstanceDirection& direction, Utf8StringCR directionString)
    {
    if (0 == directionString.length())
        return ECObjectsStatus::ParseError;
    if (0 == directionString.CompareToI(ECXML_DIRECTION_BACKWARD))
        direction = ECRelatedInstanceDirection::Backward;
    else if (0 == directionString.CompareToI(ECXML_DIRECTION_FORWARD))
        direction = ECRelatedInstanceDirection::Forward;
    else
        return ECObjectsStatus::ParseError;
        
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseCardinalityString (uint32_t &lowerLimit, uint32_t &upperLimit, const Utf8String &cardinalityString)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    if (0 == cardinalityString.compare("1"))
        {
        LOG.debugv("Legacy cardinality of '1' interpreted as '(1,1)'");
        lowerLimit = 1;
        upperLimit = 1;
        return status;
        }
        
    if ((0 == cardinalityString.compare("UNBOUNDED")) || (0 == cardinalityString.compare("Unbounded")) ||
             (0 == cardinalityString.compare("unbounded")) || (0 == cardinalityString.compare("n")) ||
             (0 == cardinalityString.compare("N")))
        {
        LOG.debugv("Legacy cardinality of '%s' interpreted as '(0,n)'", cardinalityString.c_str());
        lowerLimit = 0;
        upperLimit = UINT_MAX;
        return status;
        }
    
    Utf8String cardinalityWithoutSpaces = cardinalityString;
    cardinalityWithoutSpaces.erase(std::remove_if(cardinalityWithoutSpaces.begin(), cardinalityWithoutSpaces.end(), ::isspace), cardinalityWithoutSpaces.end());
    size_t openParenIndex = cardinalityWithoutSpaces.find('(');
    if (openParenIndex == std::string::npos)
        {
        if (0 == BE_STRING_UTILITIES_UTF8_SSCANF(cardinalityWithoutSpaces.c_str(), "%d", &upperLimit))
            return ECObjectsStatus::ParseError;
        LOG.debugv("Legacy cardinality of '%d' interpreted as '(0,%d)'", upperLimit, upperLimit);
        lowerLimit = 0;
        return status;
        }
        
    if (openParenIndex != 0 && cardinalityWithoutSpaces.find(')') != cardinalityWithoutSpaces.length() - 1)
        {
        LOG.errorv("Cardinality string '%s' is invalid.", cardinalityString.c_str());
        return ECObjectsStatus::ParseError;
        }
     
    int scanned = BE_STRING_UTILITIES_UTF8_SSCANF(cardinalityWithoutSpaces.c_str(), "(%d,%d)", &lowerLimit, &upperLimit);
    if (2 == scanned)
        return ECObjectsStatus::Success;
        
    if (0 == scanned)
        {
        LOG.errorv("Cardinality string '%s' is invalid.", cardinalityString.c_str());
        return ECObjectsStatus::ParseError;
        }
    
    // Otherwise, we just assume the upper limit is 'n' or 'N' and is unbounded
    upperLimit = UINT_MAX;
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Caleb.Shafer                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseMultiplicityString(uint32_t &lowerLimit, uint32_t &upperLimit, const Utf8String &multiplicityString)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;

    Utf8String multiplicityWithoutSpaces = multiplicityString;
    multiplicityWithoutSpaces.erase(std::remove_if(multiplicityWithoutSpaces.begin(), multiplicityWithoutSpaces.end(), ::isspace), multiplicityWithoutSpaces.end());
    
    size_t openParenIndex = multiplicityWithoutSpaces.find('(');
    if (openParenIndex == std::string::npos
            || (openParenIndex != 0 && multiplicityWithoutSpaces.find(')') != multiplicityWithoutSpaces.length() - 1))
        {
        LOG.errorv("Multiplicity string '%s' is invalid.", multiplicityString.c_str());
        return ECObjectsStatus::ParseError;
        }

    int scanned = BE_STRING_UTILITIES_UTF8_SSCANF(multiplicityWithoutSpaces.c_str(), "(%d..%d)", &lowerLimit, &upperLimit);
    if (2 == scanned)
        return ECObjectsStatus::Success;
        
    if (0 == scanned)
        {
        LOG.errorv("Multiplicity string '%s' is invalid.", multiplicityString.c_str());
        return ECObjectsStatus::ParseError;
        }

    // Otherwise, we just assume the upper limit is '*' and is unbounded
    upperLimit = UINT_MAX;
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                08/2016
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String ECXml::MultiplicityToLegacyString(RelationshipMultiplicity multiplicity)
    {
    Utf8Char legacyString[32];

    if (multiplicity.IsUpperLimitUnbounded())
        BeStringUtilities::Snprintf(legacyString, "(%d,N)", multiplicity.GetLowerLimit());
    else
        BeStringUtilities::Snprintf(legacyString, "(%d,%d)", multiplicity.GetLowerLimit(), multiplicity.GetUpperLimit());

    return legacyString;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
Utf8CP ECXml::ModifierToString(ECClassModifier modifier)
    {
    if (ECClassModifier::Abstract == modifier)
        return ECXML_MODIFIER_ABSTRACT;
    if (ECClassModifier::Sealed == modifier)
        return ECXML_MODIFIER_SEALED;
    return ECXML_MODIFIER_NONE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECXml::ParseModifierString(ECClassModifier& modifier, Utf8StringCR modifierString)
    {
    if (0 == modifierString.CompareToI(ECXML_MODIFIER_ABSTRACT))
        modifier = ECClassModifier::Abstract;
    else if (0 == modifierString.CompareToI(ECXML_MODIFIER_SEALED))
        modifier = ECClassModifier::Sealed;
    else if (0 == modifierString.CompareToI(ECXML_MODIFIER_NONE))
        modifier = ECClassModifier::None;
    else
        {
        LOG.errorv("Invalid value for Modifier attribute: %s.", modifierString.c_str());
        return ECObjectsStatus::ParseError;
        }
    return ECObjectsStatus::Success;
    }

void SetOrAppendValue(Utf8StringR str, Utf8CP val)
    {
    if (Utf8String::IsNullOrEmpty(str.c_str()))
        str = val;
    else
        {
        str.append(", ");
        str.append(val);
        }
    }

bool TestValue(CustomAttributeContainerType compareType, CustomAttributeContainerType& myType)
    {
    if (compareType == (compareType & myType))
        return true;
    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String ECXml::ContainerTypeToString(CustomAttributeContainerType containerType)
    {
    Utf8String str;
    if (TestValue(CustomAttributeContainerType::Schema, containerType))
        SetOrAppendValue(str, "Schema");
    if (TestValue(CustomAttributeContainerType::EntityClass, containerType))
        SetOrAppendValue(str, "EntityClass");
    if (TestValue(CustomAttributeContainerType::CustomAttributeClass, containerType))
        SetOrAppendValue(str, "CustomAttributeClass");
    if (TestValue(CustomAttributeContainerType::StructClass, containerType))
        SetOrAppendValue(str, "StructClass");
    if (TestValue(CustomAttributeContainerType::RelationshipClass, containerType))
        SetOrAppendValue(str, "RelationshipClass");
    if (TestValue(CustomAttributeContainerType::AnyClass, containerType))
        SetOrAppendValue(str, "AnyClass");
    if (TestValue(CustomAttributeContainerType::PrimitiveProperty, containerType))
        SetOrAppendValue(str, "PrimitiveProperty");
    if (TestValue(CustomAttributeContainerType::StructProperty, containerType))
        SetOrAppendValue(str, "StructProperty");
    if (TestValue(CustomAttributeContainerType::ArrayProperty, containerType))
        SetOrAppendValue(str, "ArrayProperty");
    if (TestValue(CustomAttributeContainerType::StructArrayProperty, containerType))
        SetOrAppendValue(str, "StructArrayProperty");
    if (TestValue(CustomAttributeContainerType::AnyProperty, containerType))
        SetOrAppendValue(str, "AnyProperty");
    if (TestValue(CustomAttributeContainerType::SourceRelationshipConstraint, containerType))
        SetOrAppendValue(str, "SourceRelationshipConstraint");
    if (TestValue(CustomAttributeContainerType::TargetRelationshipConstraint, containerType))
        SetOrAppendValue(str, "TargetRelationshipConstraint");
    if (TestValue(CustomAttributeContainerType::AnyRelationshipConstraint, containerType))
        SetOrAppendValue(str, "AnyRelationshipConstraint");
    if (TestValue(CustomAttributeContainerType::Any, containerType))
        str = "Any";

    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECXml::ParseContainerString(CustomAttributeContainerType& containerType, Utf8StringCR typeString)
    {
    if (Utf8String::IsNullOrEmpty(typeString.c_str()))
        {
        LOG.error("Unable to parse CustomAttributeContianerType from input string because it is empty");
        return ECObjectsStatus::ParseError;
        }

    bvector<Utf8String> typeTokens;
    BeStringUtilities::Split(typeString.c_str(), ",;|", typeTokens);
    
    containerType = static_cast<CustomAttributeContainerType>(0);

    for (Utf8StringCR typeToken : typeTokens)
        {
        if (typeToken.empty())
            continue;

        if (typeToken.EqualsI("Schema"))
            containerType = containerType | CustomAttributeContainerType::Schema;
        else if (typeToken.EqualsI("EntityClass"))
            containerType = containerType | CustomAttributeContainerType::EntityClass;
        else if (typeToken.EqualsI("CustomAttributeClass"))
            containerType = containerType | CustomAttributeContainerType::CustomAttributeClass;
        else if (typeToken.EqualsI("StructClass"))
            containerType = containerType | CustomAttributeContainerType::StructClass;
        else if (typeToken.EqualsI("RelationshipClass"))
            containerType = containerType | CustomAttributeContainerType::RelationshipClass;
        else if (typeToken.EqualsI("AnyClass"))
            containerType = containerType | CustomAttributeContainerType::AnyClass;
        else if (typeToken.EqualsI("PrimitiveProperty"))
            containerType = containerType | CustomAttributeContainerType::PrimitiveProperty;
        else if (typeToken.EqualsI("StructProperty"))
            containerType = containerType | CustomAttributeContainerType::StructProperty;
        else if (typeToken.EqualsI("ArrayProperty"))
            containerType = containerType | CustomAttributeContainerType::ArrayProperty;
        else if (typeToken.EqualsI("StructArrayProperty"))
            containerType = containerType | CustomAttributeContainerType::StructArrayProperty;
        else if (typeToken.EqualsI("AnyProperty"))
            containerType = containerType | CustomAttributeContainerType::AnyProperty;
        else if (typeToken.EqualsI("SourceRelationshipConstraint"))
            containerType = containerType | CustomAttributeContainerType::SourceRelationshipConstraint;
        else if (typeToken.EqualsI("TargetRelationshipConstraint"))
            containerType = containerType | CustomAttributeContainerType::TargetRelationshipConstraint;
        else if (typeToken.EqualsI("AnyRelationshipConstraint"))
            containerType = containerType | CustomAttributeContainerType::AnyRelationshipConstraint;
        else if (typeToken.EqualsI("Any"))
            containerType = CustomAttributeContainerType::Any;
        else
            {
            LOG.errorv("'%s' is not a valid CustomAttributeContainerType value.", typeToken.c_str());
            return ECObjectsStatus::ParseError;
            }
        }
    return ECObjectsStatus::Success;
    }

void ECXml::FormatXml (BeXmlDomR pXmlDoc)
    {
    // For now, do nothing. I think this can be controlled using xmlSaveCtxt
    // FormatXmlNode(pXmlDoc->documentElement, 0);
    }


END_BENTLEY_ECOBJECT_NAMESPACE