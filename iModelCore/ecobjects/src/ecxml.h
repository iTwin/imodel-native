/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ecxml.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects\ECObjects.h>

#include <string>

#define ECXML_URI_2_0 L"http://www.bentley.com/schemas/Bentley.ECXML.2.0"
#define EC_NAMESPACE_PREFIX L"ec"

#define EC_SCHEMA_ELEMENT L"ECSchema"
#define EC_CUSTOM_ATTRIBUTES_ELEMENT L"ECCustomAttributes"
#define EC_BASE_CLASS_ELEMENT L"BaseClass"
#define EC_CLASS_ELEMENT L"ECClass"
#define EC_PROPERTY_ELEMENT L"ECProperty"
#define EC_ARRAYPROPERTY_ELEMENT L"ECArrayProperty"
#define EC_STRUCTPROPERTY_ELEMENT L"ECStructProperty"
#define EC_RELATIONSHIP_CLASS_ELEMENT L"ECRelationshipClass"
#define EC_SOURCECONSTRAINT_ELEMENT L"Source"
#define EC_TARGETCONSTRAINT_ELEMENT L"Target"
#define EC_CONSTRAINTCLASS_ELEMENT L"Class"
#define EC_CONSTRAINTKEY_ELEMENT L"Key"
#define EC_KEYPROPERTY_ELEMENT L"Property"
#define EC_SCHEMAREFERENCE_ELEMENT L"ECSchemaReference"

#define BASECLASS_ATTRIBUTE L"baseClass"
#define TYPE_NAME_ATTRIBUTE L"typeName"
#define IS_DOMAINCLASS_ATTRIBUTE L"isDomainClass"
#define IS_STRUCT_ATTRIBUTE L"isStruct"
#define IS_CUSTOMATTRIBUTE_ATTRIBUTE L"isCustomAttributeClass"
#define MIN_OCCURS_ATTRIBUTE L"minOccurs"
#define MAX_OCCURS_ATTRIBUTE L"maxOccurs"
#define PROPERTY_NAME_ATTRIBUTE L"propertyName"
#define DESCRIPTION_ATTRIBUTE L"description"
#define SCHEMA_NAME_ATTRIBUTE L"schemaName"
#define SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE L"nameSpacePrefix"
#define SCHEMA_VERSION_ATTRIBUTE L"version"
#define DISPLAY_LABEL_ATTRIBUTE L"displayLabel"
#define READONLY_ATTRIBUTE L"readOnly"
#define SCHEMAREF_NAME_ATTRIBUTE L"name"
#define SCHEMAREF_VERSION_ATTRIBUTE L"version"
#define SCHEMAREF_PREFIX_ATTRIBUTE L"prefix"
#define MAXIMUM_VALUE_ATTRIBUTE L"MaximumValue"
#define MINIMUM_VALUE_ATTRIBUTE L"MinimumValue"

#define STRENGTH_ATTRIBUTE L"strength"
#define STRENGTHDIRECTION_ATTRIBUTE L"strengthDirection"
#define CARDINALITY_ATTRIBUTE L"cardinality"
#define ROLELABEL_ATTRIBUTE L"roleLabel"
#define POLYMORPHIC_ATTRIBUTE L"polymorphic"
#define CONSTRAINTCLASSNAME_ATTRIBUTE L"class"
#define KEYPROPERTYNAME_ATTRIBUTE L"name"

#define ECXML_TRUE          L"True"
#define ECXML_FALSE         L"False"
#define ECXML_UNBOUNDED     L"unbounded"

// If you add any additional ECXML typenames you must update 
//    - enum PrimitiveType
//    - PrimitiveECProperty::_GetTypeName
static const WString ECXML_TYPENAME_BINARY             = L"binary";
static const WString ECXML_TYPENAME_BOOLEAN            = L"boolean";
static const WString ECXML_TYPENAME_BOOL               = L"bool";
static const WString ECXML_TYPENAME_DATETIME           = L"dateTime";
static const WString ECXML_TYPENAME_DOUBLE             = L"double";
static const WString ECXML_TYPENAME_INTEGER            = L"int";
static const WString ECXML_TYPENAME_LONG               = L"long";
static const WString ECXML_TYPENAME_POINT2D            = L"point2d";
static const WString ECXML_TYPENAME_POINT3D            = L"point3d";
static const WString ECXML_TYPENAME_STRING             = L"string";
static const WString ECXML_TYPENAME_IGEOMETRY          = L"Bentley.Geometry.Common.IGeometry";
static const WString EMPTY_STRING = L"";

static const WString ECXML_STRENGTH_REFERENCING        = L"referencing";
static const WString ECXML_STRENGTH_HOLDING            = L"holding";
static const WString ECXML_STRENGTH_EMBEDDING          = L"embedding";

static const WString ECXML_DIRECTION_FORWARD           = L"forward";
static const WString ECXML_DIRECTION_BACKWARD          = L"backward";

#define READ_OPTIONAL_XML_ATTRIBUTE(_xmlAttributeName, _setInPointer, _setInPropertyName)   \
    if ((NULL != (attributePtr = nodeAttributesPtr->getNamedItem (_xmlAttributeName))) &&   \
        (ECOBJECTS_STATUS_Success != _setInPointer->Set##_setInPropertyName ((WCharCP)attributePtr->text))) \
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;

#define READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS(_xmlAttributeName, _setInPointer, _setInPropertyName)   \
    if (NULL != (attributePtr = nodeAttributesPtr->getNamedItem (_xmlAttributeName)))   \
        setterStatus = _setInPointer->Set##_setInPropertyName ((WCharCP)attributePtr->text); \
    else \
        setterStatus = ECOBJECTS_STATUS_Success;

#define READ_REQUIRED_XML_ATTRIBUTE(_xmlAttributeName, _setInPointer, _setInPropertyName, _elementName)   \
    if (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (_xmlAttributeName)))     \
        {   \
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: %s element must contain a " _xmlAttributeName L" attribute", (WCharCP)_elementName);     \
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;        \
        }       \
    if (ECOBJECTS_STATUS_Success != _setInPointer->Set##_setInPropertyName ((WCharCP)attributePtr->text))       \
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;

#define READ_REQUIRED_XML_ATTRIBUTE_IGNORING_SET_ERRORS(_xmlAttributeName, _setInPointer, _setInPropertyName, _elementName)   \
    if (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (_xmlAttributeName)))     \
        {   \
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: %s element must contain a " _xmlAttributeName L" attribute", (WCharCP)_elementName);     \
        status = SCHEMA_READ_STATUS_InvalidECSchemaXml;        \
        }       \
    else if (ECOBJECTS_STATUS_ParseError == _setInPointer->Set##_setInPropertyName ((WCharCP)attributePtr->text)) \
        status = SCHEMA_READ_STATUS_FailedToParseXml;
            
#define APPEND_CHILD_TO_PARENT(_child, _parent) \
    {if (NULL == _child)\
        return SCHEMA_WRITE_STATUS_FailedToCreateXml;\
    if (NULL == _parent->appendChild(_child))\
        return SCHEMA_WRITE_STATUS_FailedToCreateXml;} 

#define WRITE_XML_ATTRIBUTE(_xmlAttributeName, _value, _parent) \
    {if (NULL == (attributePtr = _parent->ownerDocument->createAttribute(_xmlAttributeName))) \
        return SCHEMA_WRITE_STATUS_FailedToCreateXml; \
    attributePtr->Putvalue(_value); \
    _parent->setAttributeNode(attributePtr);}

#define WRITE_BOOL_XML_ATTRIBUTE(_xmlAttributeName, _getPropertyName, _parent) \
    if (this->Get##_getPropertyName() == true)\
        { \
        WRITE_XML_ATTRIBUTE(_xmlAttributeName, L"True", _parent); \
        } \
   else \
        { \
        WRITE_XML_ATTRIBUTE(_xmlAttributeName, L"False", _parent); \
        } 

#define WRITE_OPTIONAL_XML_ATTRIBUTE(_xmlAttributeName, _getPropertyName, _parent) \
    if (!this->Get##_getPropertyName().empty())\
        { \
        WRITE_XML_ATTRIBUTE(_xmlAttributeName, this->Get##_getPropertyName().c_str(), _parent); \
        }
        
#define WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(_xmlAttributeName, _getPropertyName, _parent) \
    if (this->Get##_getPropertyName() == true)\
        { \
        WRITE_XML_ATTRIBUTE(_xmlAttributeName, L"True", _parent); \
        }
    
BEGIN_BENTLEY_EC_NAMESPACE

struct ECXml /*abstract*/
{
    virtual void _Abstract() = 0;
public:
    static ECObjectsStatus ParseBooleanString(bool & booleanValue,WCharCP booleanString);
    static WStringCR GetPrimitiveTypeName (PrimitiveType primitiveType);
    static ECObjectsStatus ParsePrimitiveType (PrimitiveType& primitiveType,WStringCR typeName);
    static WStringCR StrengthToString (StrengthType strength);
    static ECObjectsStatus ParseStrengthType (StrengthType& strength, WStringCR strengthString);
    static WStringCR DirectionToString (ECRelatedInstanceDirection direction);
    static ECObjectsStatus ParseDirectionString (ECRelatedInstanceDirection& direction, WStringCR directionString);
    static ECObjectsStatus ParseCardinalityString (UInt32& lowerLimit, UInt32& upperLimit, WStringCR cardinalityString);
    static void FormatXml(MSXML2::IXMLDOMDocument2* pXmlDoc);
};

END_BENTLEY_EC_NAMESPACE

