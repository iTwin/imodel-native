/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>

#define EC_NAMESPACE_PREFIX                 "ec"
#define EC_NAMESPACE_PREFIX3                "ec3"

#define ECXML_URI                           "http://www.bentley.com/schemas/Bentley.ECXML"
#define ECJSON_URI                          "https://dev.bentley.com/json_schemas/ec/32/ecschema"
#define ECJSON_SCHEMA_ITEM_URI              "https://dev.bentley.com/json_schemas/ec/32/schemaitem"

#define ECJSON_URI_SPEC_ATTRIBUTE           "$schema"
#define ECXML_SCHEMA_NAME_ATTRIBUTE         "schemaName"

#define ECJSON_SCHEMA_ITEMS_ATTRIBUTE       "items"
#define ECJSON_PARENT_VERSION_ATTRIBUTE     "schemaVersion"
#define ECJSON_PARENT_SCHEMA_ATTRIBUTE      "schema"
#define ECJSON_SCHEMA_ITEM_NAME_ATTRIBUTE   "name"
#define ECJSON_SCHEMA_ITEM_TYPE             "schemaItemType"

#define ECXML_ENTITYCLASS_ELEMENT           "ECEntityClass"
#define ECJSON_ENTITYCLASS_ELEMENT          "EntityClass"
#define ECJSON_MIXIN_ELEMENT                "Mixin"
#define ECXML_RELATIONSHIP_CLASS_ELEMENT    "ECRelationshipClass"
#define ECJSON_RELATIONSHIP_CLASS_ELEMENT   "RelationshipClass"
#define ECXML_STRUCTCLASS_ELEMENT           "ECStructClass"
#define ECJSON_STRUCTCLASS_ELEMENT          "StructClass"
#define ECXML_CUSTOMATTRIBUTECLASS_ELEMENT  "ECCustomAttributeClass"
#define ECJSON_CUSTOMATTRIBUTECLASS_ELEMENT "CustomAttributeClass"
#define KIND_OF_QUANTITY_ELEMENT            "KindOfQuantity"
#define ECXML_ENUMERATION_ELEMENT           "ECEnumeration"
#define ECJSON_ENUMERATION_ELEMENT          "Enumeration"
#define PROPERTY_CATEGORY_ELEMENT           "PropertyCategory"
#define UNIT_SYSTEM_ELEMENT                 "UnitSystem"
#define PHENOMENON_ELEMENT                  "Phenomenon"
#define UNIT_ELEMENT                        "Unit"
#define INVERTED_UNIT_ELEMENT               "InvertedUnit"
#define CONSTANT_ELEMENT                    "Constant"
#define FORMAT_ELEMENT                      "Format"
#define FORMAT_COMPOSITE_ELEMENT            "Composite"
#define FORMAT_JSON_COMPOSITE_ELEMENT       "composite"
#define FORMAT_COMPOSITE_UNIT_ELEMENT       "Unit"
#define FORMAT_COMPOSITE_UNITS_ELEMENT      "units"

#define ECXML_SCHEMAREFERENCE_ELEMENT       "ECSchemaReference"
#define ECJSON_REFERENCES_ATTRIBUTE         "references"

#define ECXML_CUSTOM_ATTRIBUTES_ELEMENT     "ECCustomAttributes"
#define ECJSON_CUSTOM_ATTRIBUTES_ELEMENT    "customAttributes"
#define ECXML_BASE_CLASS_ELEMENT            "BaseClass"
#define ECJSON_BASE_CLASS_ELEMENT           "baseClass"
#define ECXML_SOURCECONSTRAINT_ELEMENT      "Source"
#define ECXML_TARGETCONSTRAINT_ELEMENT      "Target"
#define ECJSON_SOURCECONSTRAINT_ELEMENT     "source"
#define ECJSON_TARGETCONSTRAINT_ELEMENT     "target"
#define EC_SCHEMA_ELEMENT                   "ECSchema"
#define EC_CLASS_ELEMENT                    "ECClass"
#define EC_PROPERTY_ELEMENT                 "ECProperty"
#define EC_ARRAYPROPERTY_ELEMENT            "ECArrayProperty"
#define EC_STRUCTPROPERTY_ELEMENT           "ECStructProperty"
#define EC_STRUCTARRAYPROPERTY_ELEMENT      "ECStructArrayProperty"
#define EC_NAVIGATIONPROPERTY_ELEMENT       "ECNavigationProperty"
#define EC_CONSTRAINTCLASS_ELEMENT          "Class"
#define EC_CONSTRAINTKEY_ELEMENT            "Key"
#define EC_KEYPROPERTY_ELEMENT              "Property"

#define FORMAT_ROUND_FACTOR_ATTRIBUTE           "roundFactor"
#define FORMAT_TYPE_ATTRIBUTE                   "type"
#define FORMAT_SIGN_OPTION_ATTRIBUTE            "showSignOption"
#define FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE        "scientificType"
#define FORMAT_TRAITS_ATTRIBUTE                 "formatTraits"
#define FORMAT_PRECISION_ATTRIBUTE              "precision"
#define FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE      "decimalSeparator"
#define FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE    "thousandSeparator"
#define FORMAT_UOM_SEPARATOR_ATTRIBUTE          "uomSeparator"
#define FORMAT_STAT_SEPARATOR_ATTRIBUTE         "stationSeparator"
#define FORMAT_STATION_SIZE_ATTRIBUTE           "stationOffsetSize"
#define FORMAT_MIN_WIDTH_ATTRIBUTE              "minWidth"

#define COMPOSITE_SPACER_ATTRIBUTE          "spacer"
#define COMPOSITE_INCLUDEZERO_ATTRIBUTE     "includeZero"
#define COMPOSITE_UNIT_LABEL_ATTRIBUTE      "label"

#define ECXML_DISPLAY_LABEL_ATTRIBUTE       "displayLabel"
#define ECJSON_DISPLAY_LABEL_ATTRIBUTE      "label"
#define SCHEMA_VERSION_ATTRIBUTE            "version"
#define ALIAS_ATTRIBUTE                     "alias"
#define NAME_ATTRIBUTE                      "name"
#define DESCRIPTION_ATTRIBUTE               "description"
#define NUMERATOR_ATTRIBUTE                 "numerator"
#define DENOMINATOR_ATTRIBUTE               "denominator"
#define OFFSET_ATTRIBUTE                    "offset"
#define IS_CONSTANT_ATTRIBUTE               "isConstant"
#define UNIT_NAME_ATTRIBUTE                 "unit"
#define PHENOMENON_NAME_ATTRIBUTE           "phenomenon"
#define UNIT_SYSTEM_NAME_ATTRIBUTE          "unitSystem"
#define INVERTS_UNIT_ATTRIBUTE              "invertsUnit"
#define APPLIES_TO                          "appliesTo"
#define CUSTOM_ATTRIBUTE_APPLIES_TO_ATTRIBUTE   APPLIES_TO
#define MIXIN_APPLIES_TO_ATTRIBUTE              APPLIES_TO
#define MULTIPLICITY_ATTRIBUTE              "multiplicity"
#define ROLELABEL_ATTRIBUTE                 "roleLabel"
#define POLYMORPHIC_ATTRIBUTE               "polymorphic"
#define ABSTRACTCONSTRAINT_ATTRIBUTE        "abstractConstraint"
#define ECJSON_CONSTRAINT_CLASSES           "constraintClasses"
#define TYPE_ATTRIBUTE                      "type"
#define ECXML_BACKING_TYPE_NAME_ATTRIBUTE   "backingTypeName" 
#define IS_STRICT_ATTRIBUTE                 "isStrict"
#define ENUMERATOR_VALUE_ATTRIBUTE          "value"
#define PERSISTENCE_UNIT_ATTRIBUTE          "persistenceUnit"
#define PRESENTATION_UNITS_ATTRIBUTE        "presentationUnits"
#define RELATIVE_ERROR                      "relativeError"
#define ECJSON_UNIT_FORMAT_UNIT             "unit"
#define ECJSON_UNIT_FORMAT_FORMAT           "format"
#define PRIORITY_ATTRIBUTE                  "priority"
#define ECJSON_ECPROPERTY_NAME              "name"
#define ECXML_READONLY_ATTRIBUTE            "readOnly"
#define ECJSON_READONLY_ATTRIBUTE           "isReadOnly"
#define CATEGORY_ATTRIBUTE                  "category"
#define DEFINITION_ATTRIBUTE                "definition"
#define TYPE_NAME_ATTRIBUTE                 "typeName"
#define EXTENDED_TYPE_NAME_ATTRIBUTE        "extendedTypeName"
#define ECXML_MINIMUM_VALUE_ATTRIBUTE       "minimumValue"
#define ECXML_MAXIMUM_VALUE_ATTRIBUTE       "maximumValue"
#define ECXML_MINIMUM_LENGTH_ATTRIBUTE      "minimumLength"
#define ECXML_MAXIMUM_LENGTH_ATTRIBUTE      "maximumLength"
#define ECJSON_MINIMUM_VALUE_ATTRIBUTE      "minValue"
#define ECJSON_MAXIMUM_VALUE_ATTRIBUTE      "maxValue"
#define ECJSON_MINIMUM_LENGTH_ATTRIBUTE     "minLength"
#define ECJSON_MAXIMUM_LENGTH_ATTRIBUTE     "maxLength"
#define MIN_OCCURS_ATTRIBUTE                "minOccurs"
#define MAX_OCCURS_ATTRIBUTE                "maxOccurs"
#define ECJSON_SCHEMA_ITEM_PROPERTIES_ATTRIBUTE     "properties"
#define ECJSON_INHERITED_ATTRIBUTE          "inherited"
#define ECJSON_MIXIN_REFERENCES_ATTRIBUTE   "mixins"
#define ECXML_ENUMERATOR_ELEMENT            "ECEnumerator"
#define ECJSON_ENUMERATOR_ELEMENT           "enumerators"
#define KIND_OF_QUANTITY_ATTRIBUTE          "kindOfQuantity"
#define RELATIONSHIP_NAME_ATTRIBUTE         "relationshipName"
#define DIRECTION_ATTRIBUTE                 "direction"
#define BASECLASS_ATTRIBUTE                 "baseClass"
#define IS_DOMAINCLASS_ATTRIBUTE            "isDomainClass"
#define IS_STRUCT_ATTRIBUTE                 "isStruct"
#define IS_CUSTOMATTRIBUTE_ATTRIBUTE        "isCustomAttributeClass"
#define IS_FINAL_ATTRIBUTE                  "isFinal"
#define MODIFIER_ATTRIBUTE                  "modifier"
#define PROPERTY_NAME_ATTRIBUTE             "propertyName"
#define SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE   "nameSpacePrefix"
#define SCHEMAREF_VERSION_ATTRIBUTE         "version"
#define SCHEMAREF_PREFIX_ATTRIBUTE          "prefix"
#define STRENGTH_ATTRIBUTE                  "strength"
#define STRENGTHDIRECTION_ATTRIBUTE         "strengthDirection"
#define CARDINALITY_ATTRIBUTE               "cardinality"
#define CONSTRAINTCLASSNAME_ATTRIBUTE       "class"

#define ECXML_TRUE                          "True"
#define ECXML_FALSE                         "False"

#define ECXML_UNBOUNDED                     "unbounded"

#define EC_SCHEMAS_DIRECTORY                L"ECSchemas"
#define EC_STANDARD_DIRECTORY               L"Standard"
#define EC_V3CONVERSION_DIRECTORY           L"V3Conversion"

// Class modifiers
#define MODIFIER_NONE                       "None"
#define MODIFIER_ABSTRACT                   "Abstract"
#define MODIFIER_SEALED                     "Sealed"

// Relationship Class strength
#define ECXML_STRENGTH_REFERENCING          "referencing"
#define ECXML_STRENGTH_HOLDING              "holding"
#define ECXML_STRENGTH_EMBEDDING            "embedding"
#define ECJSON_STRENGTH_REFERENCING         "Referencing"
#define ECJSON_STRENGTH_HOLDING             "Holding"
#define ECJSON_STRENGTH_EMBEDDING           "Embedding"

// Relationship Class strength direction
#define ECXML_DIRECTION_FORWARD             "forward"
#define ECXML_DIRECTION_BACKWARD            "backward"
#define ECJSON_DIRECTION_FORWARD            "Forward"
#define ECJSON_DIRECTION_BACKWARD           "Backward"

// ECProperty Types
#define ECJSON_ECPROPERTY_PRIMITIVE         "PrimitiveProperty"
#define ECJSON_ECPROPERTY_STRUCT            "StructProperty"
#define ECJSON_ECPROPERTY_PRIMITIVEARRAY    "PrimitiveArrayProperty"
#define ECJSON_ECPROPERTY_STRUCTARRAY       "StructArrayProperty"
#define ECJSON_ECPROPERTY_NAVIGATION        "NavigationProperty"

// EC Primitive Types
// If you add any additional typenames you must update
//    - enum PrimitiveType
//    - PrimitiveECProperty::_GetTypeName
#define EC_PRIMITIVE_TYPENAME_BINARY        "binary"
#define EC_PRIMITIVE_TYPENAME_BOOLEAN       "boolean"
#define EC_PRIMITIVE_TYPENAME_BOOL          "bool"
#define EC_PRIMITIVE_TYPENAME_DATETIME      "dateTime"
#define EC_PRIMITIVE_TYPENAME_DOUBLE        "double"
#define EC_PRIMITIVE_TYPENAME_INTEGER       "int"
#define EC_PRIMITIVE_TYPENAME_LONG          "long"
#define EC_PRIMITIVE_TYPENAME_POINT2D       "point2d"
#define EC_PRIMITIVE_TYPENAME_POINT3D       "point3d"
#define EC_PRIMITIVE_TYPENAME_STRING        "string"
#define EC_PRIMITIVE_TYPENAME_IGEOMETRY     "Bentley.GeometryNET.Common.IGeometry"
// This is used for matching incoming types to any common geometry type
#define EC_PRIMITIVE_TYPENAME_IGEOMETRY_GENERIC         "Bentley.GeometryNET.Common"
#define EC_PRIMITIVE_TYPENAME_IGEOMETRY_LEGACY          "Bentley.Geometry.Common"
#define EC_PRIMITIVE_TYPENAME_IGEOMETRY_LEGACY_GENERIC  "Bentley.Geometry.Common.IGeometry";
#define EMPTY_STRING ""

// Defines Instance attributes that are used in Xml and Json serialization
#define ECINSTANCE_XMLNS_ATTRIBUTE                  "xmlns"
#define ECINSTANCE_XSI_NIL_ATTRIBUTE                "nil"
#define ECINSTANCE_SCHEMA_ATTRIBUTE                 "ecSchema"
#define ECINSTANCE_CLASS_ATTRIBUTE                  "ecClass"
#define ECINSTANCE_ID_ATTRIBUTE                     "id"
#define ECINSTANCE_SOURCECLASS_ATTRIBUTE            "sourceClass"
#define ECINSTANCE_SOURCEINSTANCEID_ATTRIBUTE       "sourceInstanceID"
#define ECINSTANCE_TARGETCLASS_ATTRIBUTE            "targetClass"
#define ECINSTANCE_TARGETINSTANCEID_ATTRIBUTE       "targetInstanceID"
#define ECINSTANCE_RELATIONSHIPID_ATTTRIBUTE        "relECClassId"
#define ECINSTANCE_RELATIONSHIPNAME_ATTRIBUTE       "relECClassName"
// InstanceId for xml has to contain capitalized 'D' to support legacy data, but we want to use lower case going forward.
// Therefore separate attribute names for Json and Xml are needed.
#define ECJSON_ECINSTANCE_INSTANCEID_ATTRIBUTE      "instanceId"
#define ECXML_ECINSTANCE_INSTANCEID_ATTRIBUTE       "instanceID"

#define READ_OPTIONAL_XML_ATTRIBUTE(_nodeVar, _xmlAttributeName, _setInPointer, _setInPropertyName) \
    if ((BEXML_Success == _nodeVar.GetAttributeStringValue (value, _xmlAttributeName)) &&           \
        (ECObjectsStatus::Success != _setInPointer->Set##_setInPropertyName (value.c_str())))       \
            return SchemaReadStatus::InvalidECSchemaXml;

#define READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS(_nodeVar, _xmlAttributeName, _setInPointer, _setInPropertyName) \
    if (BEXML_Success == _nodeVar.GetAttributeStringValue (value, _xmlAttributeName))                                   \
        setterStatus = _setInPointer->Set##_setInPropertyName (value.c_str());                                          \
    else                                                                                                                \
        setterStatus = ECObjectsStatus::Success;

#define READ_REQUIRED_XML_ATTRIBUTE(_nodeVar, _xmlAttributeName, _setInPointer, _setInPropertyName, _elementName)       \
    if (BEXML_Success != _nodeVar.GetAttributeStringValue (value, _xmlAttributeName))                                   \
        {                                                                                                               \
        LOG.errorv ("Invalid ECSchemaXML: %s element must contain a %s attribute", _elementName, _xmlAttributeName);    \
        return SchemaReadStatus::InvalidECSchemaXml;                                                                    \
        }                                                                                                               \
    if (ECObjectsStatus::Success != _setInPointer->Set##_setInPropertyName (value.c_str()))                             \
        return SchemaReadStatus::InvalidECSchemaXml;

#define READ_REQUIRED_XML_ATTRIBUTE_IGNORING_SET_ERRORS(_nodeVar,_xmlAttributeName, _setInPointer, _setInPropertyName, _elementName)    \
    if (BEXML_Success != _nodeVar.GetAttributeStringValue (value, _xmlAttributeName))                                                   \
        {                                                                                                                               \
        LOG.errorv ("Invalid ECSchemaXML: %s element must contain a %s attribute", _xmlAttributeName, _elementName);                    \
        status = SchemaReadStatus::InvalidECSchemaXml;                                                                                  \
        }                                                                                                                               \
    else if (ECObjectsStatus::ParseError == _setInPointer->Set##_setInPropertyName (value.c_str()))                                     \
        status = SchemaReadStatus::FailedToParseXml;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct SchemaParseUtils /*abstract*/
{
    virtual void _Abstract() = 0;
public:
    ECOBJECTS_EXPORT static ECObjectsStatus ParseBooleanXmlString(bool& booleanValue, Utf8CP booleanString);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseCardinalityString(uint32_t& lowerLimit, uint32_t& upperLimit, Utf8StringCR cardinalityString);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseContainerString(CustomAttributeContainerType& containerType, Utf8StringCR typeString);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseDirectionString(ECRelatedInstanceDirection& direction, Utf8StringCR directionString);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseXmlFullyQualifiedName(Utf8StringR alias, Utf8StringR typeName, Utf8StringCR stringToParse);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseModifierXmlString(ECClassModifier& modifier, Utf8StringCR modifierString);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseLegacyMultiplicityString(uint32_t& lowerLimit, uint32_t& upperLimit, Utf8StringCR multiplicityString);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseMultiplicityString(uint32_t& lowerLimit, uint32_t& upperLimit, Utf8StringCR multiplicityString);
    ECOBJECTS_EXPORT static ECObjectsStatus ParsePrimitiveType(PrimitiveType& primitiveType, Utf8StringCR typeName);
    ECOBJECTS_EXPORT static ECObjectsStatus ParseStrengthType(StrengthType& strength, Utf8StringCR strengthString);
    ECOBJECTS_EXPORT static bool IsFullSchemaNameFormatValidForVersion(Utf8CP schemaFullName, uint32_t xmlMajorVersion, uint32_t xmlMinorVersion);
    ECOBJECTS_EXPORT static Utf8CP DirectionToXmlString(ECRelatedInstanceDirection direction);
    ECOBJECTS_EXPORT static Utf8CP DirectionToJsonString(ECRelatedInstanceDirection direction);
    ECOBJECTS_EXPORT static Utf8CP ModifierToString(ECClassModifier modifier);
    ECOBJECTS_EXPORT static Utf8CP PrimitiveTypeToString(PrimitiveType primitiveType);
    ECOBJECTS_EXPORT static Utf8CP StrengthToXmlString(StrengthType strength);
    ECOBJECTS_EXPORT static Utf8CP StrengthToJsonString(StrengthType strength);
    ECOBJECTS_EXPORT static Utf8CP SchemaElementTypeToString(ECSchemaElementType elementType);
    ECOBJECTS_EXPORT static Utf8String ContainerTypeToString(CustomAttributeContainerType containerType);
    ECOBJECTS_EXPORT static Utf8String MultiplicityToLegacyString(RelationshipMultiplicity multiplicity);
    ECOBJECTS_EXPORT static Utf8String GetJsonFormatString(NamedFormatCR format, ECSchemaCR primarySchema);

    //! Given a schema and a schema item, will return the fully qualified name.  If the schema item is part of the passed in schema, there
    //! is no alias.  Otherwise, the item's schema must be a referenced schema in the passed in schema
    //! @param[in]  primarySchema   The schema used to lookup the alias of the class's schema
    //! @param[in]  schemaItem      The schema item whose schema should be searched for
    //! @return Utf8String    The alias if the item's schema is not the primarySchema
    template<typename T>
    static Utf8String GetQualifiedName(ECSchemaCR primarySchema, T const& schemaItem)
        {
        Utf8String alias;
        Utf8StringCR name = schemaItem.GetName();
        if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias(schemaItem.GetSchema(), alias)))
            {
            NativeLogging::LoggingManager::GetLogger(L"ECObjectsNative")->warningv ("warning: Cannot qualify a SchemaItem name with an alias unless the schema containing the SchemaItem is referenced by the primary schema."
                "The name will remain unqualified.\n  Primary ECSchema: %s\n  SchemaItem: %s\n ECSchema containing SchemaItem: %s", primarySchema.GetName().c_str(), name.c_str(), schemaItem.GetSchema().GetName().c_str());
            return name;
            }

        if (alias.empty())
            return name;
        else
            return alias + ":" + name;
        }

    //! Given a qualified schema item name, will parse out the schema's alias and the item name.
    //!
    //! If anything other than a qualified item name is provided it will it will be split on the first `:` found.
    //! @param[out] alias       The alias of the schema
    //! @param[out] className   The name of the class
    //! @param[in]  qualifiedItemName  The qualified name of the item, in the format of ns:itemName
    //! @return A status code indicating whether the qualified name was successfully parsed or not
    ECOBJECTS_EXPORT static ECObjectsStatus ParseName(Utf8StringR alias, Utf8StringR itemName, Utf8StringCR qualifiedItemName);
};

END_BENTLEY_ECOBJECT_NAMESPACE
