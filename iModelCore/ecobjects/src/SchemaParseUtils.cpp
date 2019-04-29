/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ctype.h>
#include <regex>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
ECObjectsStatus SchemaParseUtils::ParseBooleanXmlString(bool& booleanValue, Utf8CP booleanString)
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
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
// static
ECObjectsStatus SchemaParseUtils::ParseCardinalityString(uint32_t &lowerLimit, uint32_t &upperLimit, Utf8StringCR cardinalityString)
    {
    // Note to future maintainers: the code in this function can lead to semantically incorrect parsing of a
    // cardinality string (see unit tests for examples). "Fixing" the function could potentially cause more
    // problems than it fixes so it has been decided that the function will be left alone.
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
        upperLimit = INT_MAX;
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
    upperLimit = INT_MAX;
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
// static
ECObjectsStatus SchemaParseUtils::ParseContainerString(CustomAttributeContainerType& containerType, Utf8StringCR typeString)
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
            containerType = containerType | CustomAttributeContainerType::PrimitiveArrayProperty;
        else if (typeToken.EqualsI("StructArrayProperty"))
            containerType = containerType | CustomAttributeContainerType::StructArrayProperty;
        else if (typeToken.EqualsI("NavigationProperty"))
            containerType = containerType | CustomAttributeContainerType::NavigationProperty;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
// static
ECObjectsStatus SchemaParseUtils::ParseDirectionString(ECRelatedInstanceDirection& direction, Utf8StringCR directionString)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
// static
ECObjectsStatus SchemaParseUtils::ParseXmlFullyQualifiedName(Utf8StringR alias, Utf8StringR typeName, Utf8StringCR stringToParse)
    {
    if (0 == stringToParse.length())
        {
        return ECObjectsStatus::ParseError;
        }

    Utf8String::size_type colonIndex = stringToParse.find(':');
    if (Utf8String::npos == colonIndex)
        {
        alias.clear();
        typeName = stringToParse;
        return ECObjectsStatus::Success;
        }

    if (stringToParse.length() == colonIndex + 1)
        {
        return ECObjectsStatus::ParseError;
        }

    if (0 == colonIndex)
        alias.clear();
    else
        alias = stringToParse.substr(0, colonIndex);

    typeName = stringToParse.substr(colonIndex + 1);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
// static
ECObjectsStatus SchemaParseUtils::ParseModifierXmlString(ECClassModifier& modifier, Utf8StringCR modifierString)
    {
    if (0 == modifierString.CompareToI(MODIFIER_ABSTRACT))
        modifier = ECClassModifier::Abstract;
    else if (0 == modifierString.CompareToI(MODIFIER_SEALED))
        modifier = ECClassModifier::Sealed;
    else if (0 == modifierString.CompareToI(MODIFIER_NONE))
        modifier = ECClassModifier::None;
    else
        {
        LOG.errorv("Invalid value for Modifier attribute: %s.", modifierString.c_str());
        return ECObjectsStatus::ParseError;
        }
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Caleb.Shafer                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
// static
//
// Prior to EC3.2 multiplicity strings were parsed with scanf using the format string "(%d..%d)" and would "pass" if the first integer was parsed
// successfully, irregardless of whether the entire string was valid or not. Using this multiplicity string parsing mechanism the strings
//      "(3..N)"
//      "(3,N)"
//      "(3banana)"
// Would all be parsed as if they were "(3..*)". For schemas with ECVersion 3.2 or later, this bug was fixed and the restrictions on valid
// multiplicity strings were tightened. The ParseLegacyMultiplicityString method exists solely for comparability with old EC versions.
ECObjectsStatus SchemaParseUtils::ParseLegacyMultiplicityString(uint32_t &lowerLimit, uint32_t &upperLimit, Utf8StringCR multiplicityString)
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
    upperLimit = INT_MAX;
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus SchemaParseUtils::ParseMultiplicityString(uint32_t& lowerLimit, uint32_t& upperLimit, Utf8StringCR multiplicityString)
    {
    static std::regex rgx("\\(([0-9]+)\\s*\\.\\.\\s*([0-9]+|\\*)\\)", std::regex::optimize);
    // For the multiplicity string in the form (lowerLimit..upperLimit) described by the above regex string.
    // match[0] : entire string
    // match[1] : lowerLimit
    // match[2] : upperLimit
    std::cmatch match;

    if (!std::regex_match(multiplicityString.c_str(), match, rgx))
        {
        LOG.errorv("Multiplicity string '%s' is invalid.", multiplicityString.c_str());
        return ECObjectsStatus::ParseError;
        }

    lowerLimit = (uint32_t)BeStringUtilities::ParseUInt64(match[1].str().c_str());
    upperLimit = match[2].str() == "*" ? INT_MAX : (uint32_t)BeStringUtilities::ParseUInt64(match[2].str().c_str());

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
ECObjectsStatus SchemaParseUtils::ParsePrimitiveType(PrimitiveType& primitiveType, Utf8StringCR typeName)
    {
    if (typeName.empty())
        return ECObjectsStatus::ParseError;

    if (typeName.EqualsIAscii (EC_PRIMITIVE_TYPENAME_STRING))
        primitiveType = PRIMITIVETYPE_String;
    else if (typeName.EqualsIAscii(EC_PRIMITIVE_TYPENAME_INTEGER))
        primitiveType = PRIMITIVETYPE_Integer;
    else if (typeName.EqualsIAscii(EC_PRIMITIVE_TYPENAME_LONG))
        primitiveType = PRIMITIVETYPE_Long;
    else if (typeName.EqualsIAscii(EC_PRIMITIVE_TYPENAME_BOOLEAN))
        primitiveType = PRIMITIVETYPE_Boolean;
    else if (typeName.EqualsIAscii(EC_PRIMITIVE_TYPENAME_BOOL))
        primitiveType = PRIMITIVETYPE_Boolean;
    else if (typeName.EqualsIAscii(EC_PRIMITIVE_TYPENAME_DOUBLE))
        primitiveType = PRIMITIVETYPE_Double;
    else if (typeName.EqualsIAscii(EC_PRIMITIVE_TYPENAME_POINT2D))
        primitiveType = PRIMITIVETYPE_Point2d;
    else if (typeName.EqualsIAscii(EC_PRIMITIVE_TYPENAME_POINT3D))
        primitiveType = PRIMITIVETYPE_Point3d;
    else if (typeName.EqualsIAscii(EC_PRIMITIVE_TYPENAME_DATETIME))
        primitiveType = PRIMITIVETYPE_DateTime;
    else if (typeName.EqualsIAscii(EC_PRIMITIVE_TYPENAME_BINARY))
        primitiveType = PRIMITIVETYPE_Binary;
    else if (typeName.StartsWithIAscii(EC_PRIMITIVE_TYPENAME_IGEOMETRY_GENERIC))
        primitiveType = PRIMITIVETYPE_IGeometry;
    else if (typeName.StartsWithIAscii(EC_PRIMITIVE_TYPENAME_IGEOMETRY_LEGACY))
        primitiveType = PRIMITIVETYPE_IGeometry;
    else
        return ECObjectsStatus::ParseError;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
// static
ECObjectsStatus SchemaParseUtils::ParseStrengthType(StrengthType& strength, Utf8StringCR strengthString)
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

//-------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                    05/2018
//---------------+---------------+---------------+---------------+---------------+-----
bool SchemaParseUtils::IsFullSchemaNameFormatValidForVersion(Utf8CP schemaFullName, uint32_t xmlMajorVersion, uint32_t xmlMinorVersion)
    {
    if (Utf8String::IsNullOrEmpty(schemaFullName))
        return false;

    Utf8CP firstDot = strchr (schemaFullName, '.');
    if (Utf8String::IsNullOrEmpty(firstDot))
        return false;

    static const auto countDots = [](Utf8CP versionString) -> int
        {
        int count = 0;
        const int maxVersionSize = 10; //If we somehow got a non terminated string the max a version can be is xx.xx.xx\0
        int iter = 0;
        while (versionString[iter] != '\0' && iter < maxVersionSize)
            {
            if (versionString[iter] == '.')
                ++count;
            ++iter;
            }
        return count;
        };

    SchemaKey key;
    auto dots = countDots(firstDot+1);
    if ((xmlMajorVersion >= 3 && xmlMinorVersion >= 2) || xmlMajorVersion > 3)
        {
        if (dots < 2)
            return false;
        }
    else
        {
        if (dots < 1)
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SchemaParseUtils::DirectionToXmlString(ECRelatedInstanceDirection direction)
    {
    switch (direction)
        {
        case ECRelatedInstanceDirection::Forward:
            return ECXML_DIRECTION_FORWARD;
        case ECRelatedInstanceDirection::Backward:
            return ECXML_DIRECTION_BACKWARD;
        default:
            return EMPTY_STRING;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2010
//---------------+---------------+---------------+---------------+---------------+-------
// static
Utf8CP SchemaParseUtils::DirectionToJsonString(ECRelatedInstanceDirection direction)
    {
    switch (direction)
        {
        case ECRelatedInstanceDirection::Forward:
            return ECJSON_DIRECTION_FORWARD;
        case ECRelatedInstanceDirection::Backward:
            return ECJSON_DIRECTION_BACKWARD;
        default:
            return EMPTY_STRING;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
// static
Utf8CP SchemaParseUtils::ModifierToString(ECClassModifier modifier)
    {
    if (ECClassModifier::Abstract == modifier)
        return MODIFIER_ABSTRACT;
    if (ECClassModifier::Sealed == modifier)
        return MODIFIER_SEALED;
    return MODIFIER_NONE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SchemaParseUtils::PrimitiveTypeToString(PrimitiveType primitiveType)
    {
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Binary:
            return EC_PRIMITIVE_TYPENAME_BINARY;
        case PRIMITIVETYPE_Boolean:
            return EC_PRIMITIVE_TYPENAME_BOOLEAN;
        case PRIMITIVETYPE_DateTime:
            return EC_PRIMITIVE_TYPENAME_DATETIME;
        case PRIMITIVETYPE_Double:
            return EC_PRIMITIVE_TYPENAME_DOUBLE;
        case PRIMITIVETYPE_Integer:
            return EC_PRIMITIVE_TYPENAME_INTEGER;
        case PRIMITIVETYPE_Long:
            return EC_PRIMITIVE_TYPENAME_LONG;
        case PRIMITIVETYPE_Point2d:
            return EC_PRIMITIVE_TYPENAME_POINT2D;
        case PRIMITIVETYPE_Point3d:
            return EC_PRIMITIVE_TYPENAME_POINT3D;
        case PRIMITIVETYPE_String:
            return EC_PRIMITIVE_TYPENAME_STRING;
        case PRIMITIVETYPE_IGeometry:
            return EC_PRIMITIVE_TYPENAME_IGEOMETRY_LEGACY_GENERIC;
        default:
            return EMPTY_STRING;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP SchemaParseUtils::StrengthToXmlString(StrengthType strength)
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

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP SchemaParseUtils::StrengthToJsonString(StrengthType strength)
    {
    switch (strength)
        {
        case StrengthType::Referencing :
            return ECJSON_STRENGTH_REFERENCING;
        case StrengthType::Holding:
            return ECJSON_STRENGTH_HOLDING;
        case StrengthType::Embedding:
            return ECJSON_STRENGTH_EMBEDDING;
        default:
            return EMPTY_STRING;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP SchemaParseUtils::SchemaElementTypeToString(ECSchemaElementType elementType)
    {
    switch (elementType)
        {
        case ECSchemaElementType::ECClass:
            return EC_CLASS_ELEMENT;
        case ECSchemaElementType::ECEnumeration:
            return ECJSON_ENUMERATION_ELEMENT;
        case ECSchemaElementType::KindOfQuantity:
            return KIND_OF_QUANTITY_ELEMENT;
        case ECSchemaElementType::PropertyCategory:
            return PROPERTY_CATEGORY_ELEMENT;
        case ECSchemaElementType::UnitSystem:
            return UNIT_SYSTEM_ELEMENT;
        case ECSchemaElementType::Phenomenon:
            return PHENOMENON_ELEMENT;
        case ECSchemaElementType::Unit:
            return UNIT_ELEMENT;
        case ECSchemaElementType::InvertedUnit:
            return INVERTED_UNIT_ELEMENT;
        case ECSchemaElementType::Constant:
            return CONSTANT_ELEMENT;
        }

    return EMPTY_STRING;
    }

static void SetOrAppendValue(Utf8StringR str, Utf8CP val)
    {
    if (Utf8String::IsNullOrEmpty(str.c_str()))
        str = val;
    else
        {
        str.append(", ");
        str.append(val);
        }
    }

static bool TestValue(CustomAttributeContainerType compareType, CustomAttributeContainerType& myType)
    {
    if (compareType == (compareType & myType))
        return true;
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
// static
Utf8String SchemaParseUtils::ContainerTypeToString(CustomAttributeContainerType containerType)
    {
    Utf8String str;

    if (TestValue(CustomAttributeContainerType::Any, containerType))
        {
        str = "Any";
        return str;
        }

    if (TestValue(CustomAttributeContainerType::Schema, containerType))
        SetOrAppendValue(str, "Schema");

    if (TestValue(CustomAttributeContainerType::AnyClass, containerType))
        SetOrAppendValue(str, "AnyClass");
    else
        {
        if (TestValue(CustomAttributeContainerType::EntityClass, containerType))
            SetOrAppendValue(str, "EntityClass");
        if (TestValue(CustomAttributeContainerType::CustomAttributeClass, containerType))
            SetOrAppendValue(str, "CustomAttributeClass");
        if (TestValue(CustomAttributeContainerType::StructClass, containerType))
            SetOrAppendValue(str, "StructClass");
        if (TestValue(CustomAttributeContainerType::RelationshipClass, containerType))
            SetOrAppendValue(str, "RelationshipClass");
        }

    if (TestValue(CustomAttributeContainerType::AnyProperty, containerType))
        SetOrAppendValue(str, "AnyProperty");
    else
        {
        if (TestValue(CustomAttributeContainerType::PrimitiveProperty, containerType))
            SetOrAppendValue(str, "PrimitiveProperty");
        if (TestValue(CustomAttributeContainerType::StructProperty, containerType))
            SetOrAppendValue(str, "StructProperty");
        if (TestValue(CustomAttributeContainerType::PrimitiveArrayProperty, containerType))
            SetOrAppendValue(str, "ArrayProperty");
        if (TestValue(CustomAttributeContainerType::StructArrayProperty, containerType))
            SetOrAppendValue(str, "StructArrayProperty");
        if (TestValue(CustomAttributeContainerType::NavigationProperty, containerType))
            SetOrAppendValue(str, "NavigationProperty");
        }

    if (TestValue(CustomAttributeContainerType::AnyRelationshipConstraint, containerType))
        SetOrAppendValue(str, "AnyRelationshipConstraint");
    else
        {
        if (TestValue(CustomAttributeContainerType::SourceRelationshipConstraint, containerType))
            SetOrAppendValue(str, "SourceRelationshipConstraint");
        if (TestValue(CustomAttributeContainerType::TargetRelationshipConstraint, containerType))
            SetOrAppendValue(str, "TargetRelationshipConstraint");
        }

    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                08/2016
//---------------+---------------+---------------+---------------+---------------+-------
// static
Utf8String SchemaParseUtils::MultiplicityToLegacyString(RelationshipMultiplicity multiplicity)
    {
    Utf8Char legacyString[32];

    if (multiplicity.IsUpperLimitUnbounded())
        BeStringUtilities::Snprintf(legacyString, "(%d,N)", multiplicity.GetLowerLimit());
    else
        BeStringUtilities::Snprintf(legacyString, "(%d,%d)", multiplicity.GetLowerLimit(), multiplicity.GetUpperLimit());

    return legacyString;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                   06/2018
//---------------+---------------+---------------+---------------+---------------+-------
// static
Utf8String SchemaParseUtils::GetJsonFormatString(NamedFormatCR format, ECSchemaCR primarySchema)
    {
    Utf8String name;
    Nullable<int32_t> precision;
    bvector<Utf8String> unitNames;
    bvector<Nullable<Utf8String>> unitLabels;

    // The name of a NamedFormat is a format string representing that NamedFormat.
    Formatting::Format::ParseFormatString(name, precision, unitNames, unitLabels, format.GetName());
    auto& parentSchema = format.GetParentFormat()->GetSchema();
    Utf8String qualifiedFormatName = parentSchema.GetName() + "." + name;

    if (precision.IsValid())
        {
        qualifiedFormatName += "(";
        qualifiedFormatName += std::to_string(precision.Value()).c_str();
        qualifiedFormatName += ")";
        }

    for(int i = 0; i < unitNames.size(); i++)
        { 
        qualifiedFormatName += "[";
        qualifiedFormatName += ECJsonUtilities::ECNameToJsonName(*primarySchema.LookupUnit(unitNames[i].c_str()));

        if (unitLabels[i].IsValid()) // We want to override a label
            {
            qualifiedFormatName += "|";
            qualifiedFormatName += unitLabels[i].Value();
            }
        qualifiedFormatName += "]";
        }
    return qualifiedFormatName;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2018
//--------------------------------------------------------------------------------------
// static 
ECObjectsStatus SchemaParseUtils::ParseName(Utf8StringR alias, Utf8StringR itemName, Utf8StringCR stringToParse)
    {
    if (0 == stringToParse.length())
        {
        LOG.error("Failed to parse an alias and schema item name from a qualified name because the string is empty.");
        return ECObjectsStatus::ParseError;
        }

    Utf8String::size_type colonIndex = stringToParse.find(':');
    if (Utf8String::npos == colonIndex)
        {
        alias.clear();
        itemName = stringToParse;
        return ECObjectsStatus::Success;
        }

    if (stringToParse.length() == colonIndex + 1)
        {
        LOG.errorv("Failed to parse an alias and schema item name from the qualified name '%s' because the string ends with a colon. There must be characters after the colon.",
            stringToParse.c_str());
        return ECObjectsStatus::ParseError;
        }

    if (0 == colonIndex)
        alias.clear();
    else
        alias = stringToParse.substr(0, colonIndex);

    itemName = stringToParse.substr(colonIndex + 1);

    return ECObjectsStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
