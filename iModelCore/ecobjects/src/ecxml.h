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

#define     ECXML_UNBOUNDED                     "unbounded"

// If you add any additional ECXML typenames you must update 
//    - enum PrimitiveType
//    - PrimitiveECProperty::_GetTypeName
#define ECXML_TYPENAME_BINARY             "binary"
#define ECXML_TYPENAME_BOOLEAN            "boolean"
#define ECXML_TYPENAME_BOOL               "bool"
#define ECXML_TYPENAME_DATETIME           "dateTime"
#define ECXML_TYPENAME_DOUBLE             "double"
#define ECXML_TYPENAME_INTEGER            "int"
#define ECXML_TYPENAME_LONG               "long"
#define ECXML_TYPENAME_POINT2D            "point2d"
#define ECXML_TYPENAME_POINT3D            "point3d"
#define ECXML_TYPENAME_STRING             "string"
#define ECXML_TYPENAME_IGEOMETRY          "Bentley.GeometryNET.Common.IGeometry"
// This is used for matching incoming types to any common geometry type
#define ECXML_TYPENAME_IGEOMETRY_GENERIC  "Bentley.GeometryNET.Common"
#define ECXML_TYPENAME_IGEOMETRY_LEGACY   "Bentley.Geometry.Common"
#define ECXML_TYPENAME_IGEOMETRY_LEGACY_GENERIC "Bentley.Geometry.Common.IGeometry";
#define EMPTY_STRING ""

#define ECXML_STRENGTH_REFERENCING        "referencing"
#define ECXML_STRENGTH_HOLDING            "holding"
#define ECXML_STRENGTH_EMBEDDING          "embedding"

#define ECXML_DIRECTION_FORWARD           "forward"
#define ECXML_DIRECTION_BACKWARD          "backward"

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
        LOG.errorv ("Invalid ECSchemaXML: %s element must contain a %s attribute", _xmlAttributeName, _elementName);     \
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;        \
        }       \
    if (ECOBJECTS_STATUS_Success != _setInPointer->Set##_setInPropertyName (value.c_str())) \
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;

#define READ_REQUIRED_XML_ATTRIBUTE_IGNORING_SET_ERRORS(_nodeVar,_xmlAttributeName, _setInPointer, _setInPropertyName, _elementName)   \
    if (BEXML_Success != _nodeVar.GetAttributeStringValue (value, _xmlAttributeName))     \
        {   \
        LOG.errorv ("Invalid ECSchemaXML: %s element must contain a %s attribute", _xmlAttributeName, _elementName);     \
        status = SCHEMA_READ_STATUS_InvalidECSchemaXml;        \
        }       \
    else if (ECOBJECTS_STATUS_ParseError == _setInPointer->Set##_setInPropertyName (value.c_str())) \
        status = SCHEMA_READ_STATUS_FailedToParseXml;
            
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ECXml /*abstract*/
{
    virtual void _Abstract() = 0;
public:
    static ECObjectsStatus ParseBooleanString(bool & booleanValue,Utf8CP booleanString);
    static Utf8CP GetPrimitiveTypeName (PrimitiveType primitiveType);
    static ECObjectsStatus ParsePrimitiveType (PrimitiveType& primitiveType, Utf8StringCR typeName);
    static Utf8CP StrengthToString (StrengthType strength);
    static ECObjectsStatus ParseStrengthType (StrengthType& strength, Utf8StringCR strengthString);
    static Utf8CP DirectionToString (ECRelatedInstanceDirection direction);
    static ECObjectsStatus ParseDirectionString (ECRelatedInstanceDirection& direction, Utf8StringCR directionString);
    static ECObjectsStatus ParseCardinalityString (uint32_t& lowerLimit, uint32_t& upperLimit, Utf8StringCR cardinalityString);
    static void FormatXml(BeXmlDomR xmlDom);
};

END_BENTLEY_ECOBJECT_NAMESPACE

