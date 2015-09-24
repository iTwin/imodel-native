/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ecxml.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>
#include <string>

#define     ECXML_URI_2_0                       "http://www.bentley.com/schemas/Bentley.ECXML.2.0"
#define     EC_NAMESPACE_PREFIX                 "ec"

#define     EC_SCHEMA_ELEMENT                   "ECSchema"
#define     EC_CUSTOM_ATTRIBUTES_ELEMENT        "ECCustomAttributes"
#define     EC_BASE_CLASS_ELEMENT               "BaseClass"
#define     EC_CLASS_ELEMENT                    "ECClass"
#define     EC_PROPERTY_ELEMENT                 "ECProperty"
#define     EC_ARRAYPROPERTY_ELEMENT            "ECArrayProperty"
#define     EC_STRUCTPROPERTY_ELEMENT           "ECStructProperty"
#define     EC_RELATIONSHIP_CLASS_ELEMENT       "ECRelationshipClass"
#define     EC_SOURCECONSTRAINT_ELEMENT         "Source"
#define     EC_TARGETCONSTRAINT_ELEMENT         "Target"
#define     EC_CONSTRAINTCLASS_ELEMENT          "Class"
#define     EC_CONSTRAINTKEY_ELEMENT            "Key"
#define     EC_KEYPROPERTY_ELEMENT              "Property"
#define     EC_SCHEMAREFERENCE_ELEMENT          "ECSchemaReference"

#define     BASECLASS_ATTRIBUTE                 "baseClass"
#define     TYPE_NAME_ATTRIBUTE                 "typeName"
#define     IS_DOMAINCLASS_ATTRIBUTE            "isDomainClass"
#define     IS_FINAL_ATTRIBUTE                  "isFinal"
#define     IS_STRUCT_ATTRIBUTE                 "isStruct"
#define     IS_CUSTOMATTRIBUTE_ATTRIBUTE        "isCustomAttributeClass"
#define     MIN_OCCURS_ATTRIBUTE                "minOccurs"
#define     MAX_OCCURS_ATTRIBUTE                "maxOccurs"
#define     PROPERTY_NAME_ATTRIBUTE             "propertyName"
#define     DESCRIPTION_ATTRIBUTE               "description"
#define     SCHEMA_NAME_ATTRIBUTE               "schemaName"
#define     SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE   "nameSpacePrefix"
#define     SCHEMA_VERSION_ATTRIBUTE            "version"
#define     DISPLAY_LABEL_ATTRIBUTE             "displayLabel"
#define     READONLY_ATTRIBUTE                  "readOnly"
#define     SCHEMAREF_NAME_ATTRIBUTE            "name"
#define     SCHEMAREF_VERSION_ATTRIBUTE         "version"
#define     SCHEMAREF_PREFIX_ATTRIBUTE          "prefix"
#define     MAXIMUM_VALUE_ATTRIBUTE             "MaximumValue"
#define     MINIMUM_VALUE_ATTRIBUTE             "MinimumValue"

#define     STRENGTH_ATTRIBUTE                  "strength"
#define     STRENGTHDIRECTION_ATTRIBUTE         "strengthDirection"
#define     CARDINALITY_ATTRIBUTE               "cardinality"
#define     ROLELABEL_ATTRIBUTE                 "roleLabel"
#define     POLYMORPHIC_ATTRIBUTE               "polymorphic"
#define     CONSTRAINTCLASSNAME_ATTRIBUTE       "class"
#define     KEYPROPERTYNAME_ATTRIBUTE           "name"

#define     ECXML_TRUE                          "True"
#define     ECXML_FALSE                         "False"

#define     ECXML_TRUE_W                        L"True"
#define     ECXML_FALSE_W                       L"False"
#define     ECXML_UNBOUNDED                     L"unbounded"

// If you add any additional ECXML typenames you must update 
//    - enum PrimitiveType
//    - PrimitiveECProperty::_GetTypeName
static WCharCP ECXML_TYPENAME_BINARY             = L"binary";
static WCharCP ECXML_TYPENAME_BOOLEAN            = L"boolean";
static WCharCP ECXML_TYPENAME_BOOL               = L"bool";
static WCharCP ECXML_TYPENAME_DATETIME           = L"dateTime";
static WCharCP ECXML_TYPENAME_DOUBLE             = L"double";
static WCharCP ECXML_TYPENAME_INTEGER            = L"int";
static WCharCP ECXML_TYPENAME_LONG               = L"long";
static WCharCP ECXML_TYPENAME_POINT2D            = L"point2d";
static WCharCP ECXML_TYPENAME_POINT3D            = L"point3d";
static WCharCP ECXML_TYPENAME_STRING             = L"string";
static WCharCP ECXML_TYPENAME_IGEOMETRY          = L"Bentley.GeometryNET.Common.IGeometry";
static WCharCP ECXML_TYPENAME_IGEOMETRY_GENERIC  = L"Bentley.GeometryNET.Common"; // This is used for matching incoming types to any common geometry type
static WCharCP ECXML_TYPENAME_IGEOMETRY_LEGACY   = L"Bentley.Geometry.Common";
static WCharCP ECXML_TYPENAME_IGEOMETRY_LEGACY_GENERIC   = L"Bentley.Geometry.Common.IGeometry";
static WCharCP EMPTY_STRING = L"";

static WCharCP ECXML_STRENGTH_REFERENCING        = L"referencing";
static WCharCP ECXML_STRENGTH_HOLDING            = L"holding";
static WCharCP ECXML_STRENGTH_EMBEDDING          = L"embedding";

static WCharCP ECXML_DIRECTION_FORWARD           = L"forward";
static WCharCP ECXML_DIRECTION_BACKWARD          = L"backward";

#define READ_OPTIONAL_XML_ATTRIBUTE(_nodeVar, _xmlAttributeName, _setInPointer, _setInPropertyName)   \
    if ((BEXML_Success == _nodeVar.GetAttributeStringValue (value, _xmlAttributeName)) &&   \
        (ECOBJECTS_STATUS_Success != _setInPointer->Set##_setInPropertyName (value.c_str()))) \
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;

#define READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS(_nodeVar, _xmlAttributeName, _setInPointer, _setInPropertyName)   \
    if (BEXML_Success == _nodeVar.GetAttributeStringValue (value, _xmlAttributeName))   \
        setterStatus = _setInPointer->Set##_setInPropertyName (value.c_str()); \
    else \
        setterStatus = ECOBJECTS_STATUS_Success;

#define READ_REQUIRED_XML_ATTRIBUTE(_nodeVar, _xmlAttributeName, _setInPointer, _setInPropertyName, _elementName)   \
    if (BEXML_Success != _nodeVar.GetAttributeStringValue (value, _xmlAttributeName))   \
        {   \
        LOG.errorv (L"Invalid ECSchemaXML: %hs element must contain a %hs  attribute", _xmlAttributeName, _elementName);     \
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;        \
        }       \
    if (ECOBJECTS_STATUS_Success != _setInPointer->Set##_setInPropertyName (value.c_str())) \
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;

#define READ_REQUIRED_XML_ATTRIBUTE_IGNORING_SET_ERRORS(_nodeVar,_xmlAttributeName, _setInPointer, _setInPropertyName, _elementName)   \
    if (BEXML_Success != _nodeVar.GetAttributeStringValue (value, _xmlAttributeName))     \
        {   \
        LOG.errorv (L"Invalid ECSchemaXML: %hs element must contain a %hs attribute",  _xmlAttributeName, _elementName);     \
        status = SCHEMA_READ_STATUS_InvalidECSchemaXml;        \
        }       \
    else if (ECOBJECTS_STATUS_ParseError == _setInPointer->Set##_setInPropertyName (value.c_str())) \
        status = SCHEMA_READ_STATUS_FailedToParseXml;
            
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ECXml /*abstract*/
{
    virtual void _Abstract() = 0;
public:
    static ECObjectsStatus ParseBooleanString(bool & booleanValue,WCharCP booleanString);
    static WCharCP GetPrimitiveTypeName (PrimitiveType primitiveType);
    static ECObjectsStatus ParsePrimitiveType (PrimitiveType& primitiveType,WStringCR typeName);
    static WCharCP StrengthToString (StrengthType strength);
    static ECObjectsStatus ParseStrengthType (StrengthType& strength, WStringCR strengthString);
    static WCharCP DirectionToString (ECRelatedInstanceDirection direction);
    static ECObjectsStatus ParseDirectionString (ECRelatedInstanceDirection& direction, WStringCR directionString);
    static ECObjectsStatus ParseCardinalityString (uint32_t& lowerLimit, uint32_t& upperLimit, WStringCR cardinalityString);
    static void FormatXml(BeXmlDomR xmlDom);
};

END_BENTLEY_ECOBJECT_NAMESPACE

