/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ecxml.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>

#define     ECJSON_URI                          "https://dev.bentley.com/json_schemas/ec/31/draft-01/ecschema"
#define     ECJSON_SCHEMA_CHILD_URI             "https://dev.bentley.com/json_schemas/ec/31/draft-01/schemachild"

#define     ECJSON_URI_SPEC_ATTRIBUTE           "$schema"
#define     ECJSON_NAME_ATTRIBUTE               "name"
#define     ECJSON_DISPLAY_LABEL_ATTRIBUTE      "label"
#define     ECJSON_REFERENCES_ATTRIBUTE         "references"
#define     ECJSON_SCHEMA_CHILDREN_ATTRIBUTE    "children"
#define     ECJSON_SCHEMA_NAME_ATTRIBUTE        "schema"
#define     ECJSON_SCHEMA_VERSION_ATTRIBUTE     "schemaVersion"
#define     ECJSON_SCHEMA_CHILD_NAME_ATTRIBUTE  "name"
#define     ECJSON_SCHEMA_CHILD_PROPERTIES_ATTRIBUTE "properties"
#define     ECJSON_MINIMUM_VALUE_ATTRIBUTE      "minValue"
#define     ECJSON_MAXIMUM_VALUE_ATTRIBUTE      "maxValue"
#define     ECJSON_MINIMUM_LENGTH_ATTRIBUTE     "minLength"
#define     ECJSON_MAXIMUM_LENGTH_ATTRIBUTE     "maxLength"
#define     ECJSON_SCHEMA_CHILD_TYPE            "schemaChildType"

#define     ECJSON_CUSTOM_ATTRIBUTES_ELEMENT    "customAttributes"
#define     ECJSON_MIXIN_ELEMENT                "Mixin"
#define     ECJSON_ENTITYCLASS_ELEMENT          "EntityClass"
#define     ECJSON_RELATIONSHIP_CLASS_ELEMENT   "RelationshipClass"
#define     ECJSON_STRUCTCLASS_ELEMENT          "StructClass"
#define     ECJSON_CUSTOMATTRIBUTECLASS_ELEMENT "CustomAttributeClass"
#define     ECJSON_BASE_CLASS_ELEMENT           "baseClass"
#define     ECJSON_MIXIN_ATTRIBUTE              "mixins"
#define     ECJSON_SOURCECONSTRAINT_ELEMENT     "source"
#define     ECJSON_TARGETCONSTRAINT_ELEMENT     "target"
#define     ECJSON_ENUMERATION_ELEMENT          "Enumeration"
#define     ECJSON_ENUMERATOR_ELEMENT           "enumerators"
#define     ECJSON_PRECISION_ATTRIBUTE          "precision"
#define     ECJSON_PRESENTATION_UNIT_OBJECT_UNIT   "unit"
#define     ECJSON_PRESENTATION_UNIT_OBJECT_FORMAT "format"

#define     ECJSON_ECPROPERTY_NAME              "name"
#define     ECJSON_ECPROPERTY_PRIMITIVE         "PrimitiveProperty"
#define     ECJSON_ECPROPERTY_STRUCT            "StructProperty"
#define     ECJSON_ECPROPERTY_PRIMITIVEARRAY    "PrimitiveArrayProperty"
#define     ECJSON_ECPROPERTY_STRUCTARRAY       "StructArrayProperty"
#define     ECJSON_ECPROPERTY_TYPE              "propertyType"
#define     ECJSON_ECPROPERTY_NAVIGATION        "NavigationProperty"
#define     ECJSON_CONSTRAINT_CLASSES           "constraintClasses"

#define     ECJSON_DIRECTION_FORWARD            "forward"
#define     ECJSON_DIRECTION_BACKWARD           "backward"

#define     ECJSON_MODIFIER_NONE                "none"
#define     ECJSON_MODIFIER_ABSTRACT            "abstract"
#define     ECJSON_MODIFIER_SEALED              "sealed"

#define     ECXML_URI                           "http://www.bentley.com/schemas/Bentley.ECXML"
#define     EC_NAMESPACE_PREFIX                 "ec"
#define     EC_NAMESPACE_PREFIX3                "ec3"

#define     EC_SCHEMAS_DIRECTORY                L"ECSchemas"
#define     EC_STANDARD_DIRECTORY               L"Standard"
#define     EC_V3CONVERSION_DIRECTORY           L"V3Conversion"

#define     EC_SCHEMA_ELEMENT                   "ECSchema"
#define     EC_CUSTOM_ATTRIBUTES_ELEMENT        "ECCustomAttributes"
#define     EC_BASE_CLASS_ELEMENT               "BaseClass"
#define     EC_CLASS_ELEMENT                    "ECClass"
#define     EC_ENTITYCLASS_ELEMENT              "ECEntityClass"
#define     EC_STRUCTCLASS_ELEMENT              "ECStructClass"
#define     EC_CUSTOMATTRIBUTECLASS_ELEMENT     "ECCustomAttributeClass"
#define     EC_PROPERTY_ELEMENT                 "ECProperty"
#define     EC_ARRAYPROPERTY_ELEMENT            "ECArrayProperty"
#define     EC_STRUCTPROPERTY_ELEMENT           "ECStructProperty"
#define     EC_STRUCTARRAYPROPERTY_ELEMENT      "ECStructArrayProperty"
#define     EC_NAVIGATIONPROPERTY_ELEMENT       "ECNavigationProperty"
#define     EC_RELATIONSHIP_CLASS_ELEMENT       "ECRelationshipClass"
#define     EC_SOURCECONSTRAINT_ELEMENT         "Source"
#define     EC_TARGETCONSTRAINT_ELEMENT         "Target"
#define     EC_CONSTRAINTCLASS_ELEMENT          "Class"
#define     EC_CONSTRAINTKEY_ELEMENT            "Key"
#define     EC_KEYPROPERTY_ELEMENT              "Property"
#define     EC_SCHEMAREFERENCE_ELEMENT          "ECSchemaReference"
#define     EC_ENUMERATION_ELEMENT              "ECEnumeration"
#define     EC_ENUMERATOR_ELEMENT               "ECEnumerator"
#define     KIND_OF_QUANTITY_ELEMENT            "KindOfQuantity"
#define     PROPERTY_CATEGORY_ELEMENT           "PropertyCategory"

#define     BASECLASS_ATTRIBUTE                 "baseClass"
#define     TYPE_NAME_ATTRIBUTE                 "typeName"
#define     EXTENDED_TYPE_NAME_ATTRIBUTE        "extendedTypeName"
#define     BACKING_TYPE_NAME_ATTRIBUTE         "backingTypeName"
#define     IS_STRICT_ATTRIBUTE                 "isStrict"
#define     IS_DOMAINCLASS_ATTRIBUTE            "isDomainClass"
#define     IS_STRUCT_ATTRIBUTE                 "isStruct"
#define     IS_CUSTOMATTRIBUTE_ATTRIBUTE        "isCustomAttributeClass"
#define     IS_FINAL_ATTRIBUTE                  "isFinal"
#define     MODIFIER_ATTRIBUTE                  "modifier"
#define     CUSTOM_ATTRIBUTE_APPLIES_TO         "appliesTo"
#define     MIN_OCCURS_ATTRIBUTE                "minOccurs"
#define     MAX_OCCURS_ATTRIBUTE                "maxOccurs"
#define     PROPERTY_NAME_ATTRIBUTE             "propertyName"
#define     RELATIONSHIP_NAME_ATTRIBUTE         "relationshipName"
#define     DIRECTION_ATTRIBUTE                 "direction"
#define     DESCRIPTION_ATTRIBUTE               "description"
#define     SCHEMA_NAME_ATTRIBUTE               "schemaName"
#define     SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE   "nameSpacePrefix"
#define     SCHEMA_VERSION_ATTRIBUTE            "version"
#define     DISPLAY_LABEL_ATTRIBUTE             "displayLabel"
#define     READONLY_ATTRIBUTE                  "readOnly"
#define     SCHEMAREF_NAME_ATTRIBUTE            "name"
#define     SCHEMAREF_VERSION_ATTRIBUTE         "version"
#define     SCHEMAREF_PREFIX_ATTRIBUTE          "prefix"
#define     ALIAS_ATTRIBUTE                     "alias"
#define     MINIMUM_VALUE_ATTRIBUTE             "minimumValue"
#define     MAXIMUM_VALUE_ATTRIBUTE             "maximumValue"
#define     MINIMUM_LENGTH_ATTRIBUTE            "minimumLength"
#define     MAXIMUM_LENGTH_ATTRIBUTE            "maximumLength"
#define     KIND_OF_QUANTITY_ATTRIBUTE          "kindOfQuantity"
#define     PERSISTENCE_UNIT_ATTRIBUTE          "persistenceUnit"
#define     PRESENTATION_UNITS_ATTRIBUTE        "presentationUnits"
#define     RELATIVE_ERROR_ATTRIBUTE            "relativeError"
#define     CATEGORY_ATTRIBUTE                  "category"
#define     PRIORITY_ATTRIBUTE                  "priority"

#define     STRENGTH_ATTRIBUTE                  "strength"
#define     STRENGTHDIRECTION_ATTRIBUTE         "strengthDirection"
#define     CARDINALITY_ATTRIBUTE               "cardinality"
#define     MULTIPLICITY_ATTRIBUTE              "multiplicity"
#define     ROLELABEL_ATTRIBUTE                 "roleLabel"
#define     POLYMORPHIC_ATTRIBUTE               "polymorphic"
#define     ABSTRACTCONSTRAINT_ATTRIBUTE        "abstractConstraint"
#define     CONSTRAINTCLASSNAME_ATTRIBUTE       "class"
#define     KEYPROPERTYNAME_ATTRIBUTE           "name"
#define     ENUMERATOR_VALUE_ATTRIBUTE          "value"

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

#define ECXML_MODIFIER_NONE                 "None"
#define ECXML_MODIFIER_ABSTRACT             "Abstract"
#define ECXML_MODIFIER_SEALED               "Sealed"

// Defines Instance attributes that are used in Xml and Json serialization
#define ECINSTANCE_XMLNS_ATTRIBUTE              "xmlns"
#define ECINSTANCE_XSI_NIL_ATTRIBUTE            "nil"
#define ECINSTANCE_SCHEMA_ATTRIBUTE             "ecSchema"
#define ECINSTANCE_CLASS_ATTRIBUTE              "ecClass"
#define ECINSTANCE_ID_ATTRIBUTE                 "id"
#define ECINSTANCE_SOURCECLASS_ATTRIBUTE        "sourceClass"
#define ECINSTANCE_SOURCEINSTANCEID_ATTRIBUTE   "sourceInstanceID"
#define ECINSTANCE_TARGETCLASS_ATTRIBUTE        "targetClass"
#define ECINSTANCE_TARGETINSTANCEID_ATTRIBUTE   "targetInstanceID"
#define ECINSTANCE_RELATIONSHIPID_ATTTRIBUTE    "relECClassId"
#define ECINSTANCE_RELATIONSHIPNAME_ATTRIBUTE   "relECClassName"
// InstanceId for xml has to contain capitalized 'D' to support legacy data, but we want to use lower case going forward.
// Therefore separate attribute names for Json and Xml are needed.
#define ECINSTANCE_INSTANCEID_JSON_ATTRIBUTE    "instanceId"
#define ECINSTANCE_INSTANCEID_XML_ATTRIBUTE     "instanceID"

#define READ_OPTIONAL_XML_ATTRIBUTE(_nodeVar, _xmlAttributeName, _setInPointer, _setInPropertyName)   \
    if ((BEXML_Success == _nodeVar.GetAttributeStringValue (value, _xmlAttributeName)) &&   \
        (ECObjectsStatus::Success != _setInPointer->Set##_setInPropertyName (value.c_str()))) \
            return SchemaReadStatus::InvalidECSchemaXml;

#define READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS(_nodeVar, _xmlAttributeName, _setInPointer, _setInPropertyName)   \
    if (BEXML_Success == _nodeVar.GetAttributeStringValue (value, _xmlAttributeName))   \
        setterStatus = _setInPointer->Set##_setInPropertyName (value.c_str()); \
    else \
        setterStatus = ECObjectsStatus::Success;

#define READ_REQUIRED_XML_ATTRIBUTE(_nodeVar, _xmlAttributeName, _setInPointer, _setInPropertyName, _elementName)   \
    if (BEXML_Success != _nodeVar.GetAttributeStringValue (value, _xmlAttributeName))   \
        {   \
        LOG.errorv ("Invalid ECSchemaXML: %s element must contain a %s attribute", _elementName, _xmlAttributeName);     \
        return SchemaReadStatus::InvalidECSchemaXml;        \
        }       \
    if (ECObjectsStatus::Success != _setInPointer->Set##_setInPropertyName (value.c_str())) \
        return SchemaReadStatus::InvalidECSchemaXml;

#define READ_REQUIRED_XML_ATTRIBUTE_IGNORING_SET_ERRORS(_nodeVar,_xmlAttributeName, _setInPointer, _setInPropertyName, _elementName)   \
    if (BEXML_Success != _nodeVar.GetAttributeStringValue (value, _xmlAttributeName))     \
        {   \
        LOG.errorv ("Invalid ECSchemaXML: %s element must contain a %s attribute", _xmlAttributeName, _elementName);     \
        status = SchemaReadStatus::InvalidECSchemaXml;        \
        }       \
    else if (ECObjectsStatus::ParseError == _setInPointer->Set##_setInPropertyName (value.c_str())) \
        status = SchemaReadStatus::FailedToParseXml;
            
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ECXml /*abstract*/
{
    virtual void _Abstract() = 0;
public:
    static ECObjectsStatus ParseBooleanString(bool & booleanValue,Utf8CP booleanString);
    static Utf8CP GetPrimitiveTypeName (PrimitiveType primitiveType);
    static ECObjectsStatus ParsePrimitiveType (PrimitiveType& primitiveType, Utf8StringCR typeName);
    static Utf8CP StrengthToString (StrengthType strength);
    static Utf8CP StrengthToJsonString(StrengthType strength);
    static ECObjectsStatus ParseStrengthType (StrengthType& strength, Utf8StringCR strengthString);
    static Utf8CP DirectionToString (ECRelatedInstanceDirection direction);
    static Utf8CP DirectionToJsonString(ECRelatedInstanceDirection direction);
    static ECObjectsStatus ParseDirectionString (ECRelatedInstanceDirection& direction, Utf8StringCR directionString);
    static ECObjectsStatus ParseCardinalityString (uint32_t& lowerLimit, uint32_t& upperLimit, Utf8StringCR cardinalityString);
    static ECObjectsStatus ParseMultiplicityString (uint32_t& lowerLimit, uint32_t& upperLimit, Utf8StringCR multiplicityString);
    static Utf8String MultiplicityToLegacyString (RelationshipMultiplicity multiplicity);
    static Utf8CP ModifierToString(ECClassModifier modifier);
    static Utf8CP ModifierToJsonString(ECClassModifier modifier);
    static ECObjectsStatus ParseModifierString(ECClassModifier& modifier, Utf8StringCR modifierString);
    static Utf8String ContainerTypeToString(CustomAttributeContainerType containerType);
    static ECObjectsStatus ParseContainerString(CustomAttributeContainerType& containerType, Utf8StringCR typeString);
    static ECObjectsStatus ParseFullyQualifiedName(Utf8StringR alias, Utf8StringR typeName, Utf8StringCR stringToParse);
    static void FormatXml(BeXmlDomR xmlDom);
};

END_BENTLEY_ECOBJECT_NAMESPACE

