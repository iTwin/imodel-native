/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECProperty.cpp $
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
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECProperty::GetBaseProperty
(
) const
    {
    return m_baseProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetBaseProperty
(
ECPropertyCP baseProperty
)
    {
    m_baseProperty = baseProperty;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR ECProperty::GetClass
(
) const
    {
    return m_class;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECProperty::GetName
(
) const
    {
    return m_name;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetName
(
std::wstring const& name
)
    {        
    if (!NameValidator::Validate(name))
        return ECOBJECTS_STATUS_InvalidName;
 
    m_name = name;        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECProperty::GetDescription
(
) const
    {
    return m_description;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetDescription
(
std::wstring const& description
)
    {        
    m_description = description;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECProperty::GetDisplayLabel
(
) const
    {
    return (m_displayLabel.empty()) ? Name : m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetDisplayLabel
(
std::wstring const& displayLabel
)
    {        
    m_displayLabel = displayLabel;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsReadOnly
(
) const
    {
    return m_readOnly;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetIsReadOnly
(
bool readOnly
)
    {        
    m_readOnly = readOnly;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetIsReadOnly
(
const wchar_t * isReadOnly
)
    {        
    PRECONDITION (NULL != isReadOnly, ECOBJECTS_STATUS_PreconditionViolated);

    bool bReadOnly;
    ECObjectsStatus status = ECXml::ParseBooleanString (bReadOnly, isReadOnly);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->errorv (L"Failed to parse the isReadOnly string '%s' for ECProperty '%s'.\n", isReadOnly, this->Name.c_str());
    else
        SetIsReadOnly (bReadOnly);
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsDisplayLabelDefined
(
) const
    {
    return (!m_displayLabel.empty());        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring ECProperty::GetTypeName
(
) const
    {
    return this->_GetTypeName();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetTypeName
(
std::wstring typeName
)
    {
    return this->_SetTypeName (typeName);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsPrimitive
(
) const
    {
    return this->_IsPrimitive();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveECPropertyP ECProperty::GetAsPrimitiveProperty
(
) const
    {
    return dynamic_cast<PrimitiveECPropertyP>((ECPropertyP)this);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsStruct
(
) const
    {
    return this->_IsStruct();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
StructECPropertyP ECProperty::GetAsStructProperty
(
) const
    {
    return dynamic_cast<StructECPropertyP>((ECPropertyP)this);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsArray
(
) const
    {
    return this->_IsArray();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayECPropertyP ECProperty::GetAsArrayProperty
(
) const
    {
    return dynamic_cast<ArrayECPropertyP>((ECPropertyP)this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::_GetBaseContainers
(
bvector<IECCustomAttributeContainerP>& returnList
) const
    {
    if (NULL != m_baseProperty)
        returnList.push_back((const_cast<ECPropertyP>(m_baseProperty)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECProperty::_ReadXml
(
MSXML2::IXMLDOMNode& propertyNode
)
    {  
    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = propertyNode.attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;        

    READ_REQUIRED_XML_ATTRIBUTE (PROPERTY_NAME_ATTRIBUTE,       this, Name,     propertyNode.baseName)        
    
    // OPTIONAL attributes - If these attributes exist they MUST be valid    
    READ_OPTIONAL_XML_ATTRIBUTE (DESCRIPTION_ATTRIBUTE,         this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (DISPLAY_LABEL_ATTRIBUTE,       this, DisplayLabel)    

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (READONLY_ATTRIBUTE,            this, IsReadOnly)

    ReadCustomAttributes(propertyNode, (ECSchemaP) (&(m_class.Schema)));
    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECProperty::_WriteXml
(
MSXML2::IXMLDOMElement& parentNode
)
    {
    return _WriteXml(parentNode, EC_PROPERTY_ELEMENT);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECProperty::_WriteXml
(
MSXML2::IXMLDOMElement& parentNode,
const wchar_t *elementName
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    MSXML2::IXMLDOMAttributePtr attributePtr;

    MSXML2::IXMLDOMElementPtr propertyPtr = parentNode.ownerDocument->createNode(NODE_ELEMENT, elementName, ECXML_URI_2_0);;
    APPEND_CHILD_TO_PARENT(propertyPtr, (&parentNode));
    
    WRITE_XML_ATTRIBUTE(PROPERTY_NAME_ATTRIBUTE, this->GetName().c_str(), propertyPtr);
    WRITE_XML_ATTRIBUTE(TYPE_NAME_ATTRIBUTE, this->GetTypeName().c_str(), propertyPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(DESCRIPTION_ATTRIBUTE, Description, propertyPtr);
    if (IsDisplayLabelDefined)
        WRITE_OPTIONAL_XML_ATTRIBUTE(DISPLAY_LABEL_ATTRIBUTE, DisplayLabel, propertyPtr);
    WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(READONLY_ATTRIBUTE, IsReadOnly, propertyPtr);
    
    WriteCustomAttributes(propertyPtr);
    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus PrimitiveECProperty::_ReadXml
(
MSXML2::IXMLDOMNode& propertyNode
)
    {  
    SchemaDeserializationStatus status = __super::_ReadXml(propertyNode);
    if (status != SCHEMA_DESERIALIZATION_STATUS_Success)
        return status;

    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = propertyNode.attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;        
    
    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    // For Primitive & Array properties we ignore parse errors and default to string.  Struct properties will require a resolvable typename.
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (TYPE_NAME_ATTRIBUTE,           this, TypeName)  
    if (ECOBJECTS_STATUS_Success != setterStatus)
        Logger::GetLogger()->warningv (L"Defaulting the type of ECProperty '%s' to '%s' in reaction to non-fatal parse error.\n", this->Name.c_str(), this->TypeName.c_str());        

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus PrimitiveECProperty::_WriteXml
(
MSXML2_IXMLDOMElement& parentNode
)
    {
    return __super::_WriteXml(parentNode, EC_PROPERTY_ELEMENT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrimitiveECProperty::_CanOverride
(
ECPropertyCR baseProperty
) const
    {
    PrimitiveType basePrimitiveType;
    
    // normally, we do not allow a primitive property to override an array property.  However, there is a set of schemas that
    // have been delivered that allow this behavior.  If the primitive property type is the same as the type used in the array, then
    // we allow it to be overridden.
    if (baseProperty.IsArray)
        {
        ArrayECPropertyP arrayProperty = baseProperty.GetAsArrayProperty();
        if (ARRAYKIND_Struct == arrayProperty->Kind)
            return false;
        basePrimitiveType = arrayProperty->GetPrimitiveElementType();
        }
    else if (baseProperty.IsStruct)
        return false;
    else
        {
        basePrimitiveType = baseProperty.GetAsPrimitiveProperty()->Type;
        }
        
    return (basePrimitiveType == m_primitiveType);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring PrimitiveECProperty::_GetTypeName
(
) const
    {
    return ECXml::GetPrimitiveTypeName (m_primitiveType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveECProperty::_SetTypeName 
(
std::wstring const& typeName
)
    {
    PrimitiveType primitiveType;
    ECObjectsStatus status = ECXml::ParsePrimitiveType (primitiveType, typeName);
    if (ECOBJECTS_STATUS_Success != status)
        {            
        Logger::GetLogger()->errorv (L"Failed to set the type name of ECProperty '%s' to '%s' because the typeName could not be parsed into a primitive type.\n", this->Name.c_str(), typeName.c_str());        
        return status;
        }

    return SetType (primitiveType);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType PrimitiveECProperty::GetType
(
) const
    {
    return m_primitiveType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveECProperty::SetType
(
PrimitiveType primitiveType
)
    {        
    m_primitiveType = primitiveType;        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus StructECProperty::_ReadXml
(
MSXML2::IXMLDOMNode& propertyNode
)
    {  
    SchemaDeserializationStatus status = __super::_ReadXml(propertyNode);
    if (status != SCHEMA_DESERIALIZATION_STATUS_Success)
        return status;

    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = propertyNode.attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;        

    // For Primitive & Array properties we ignore parse errors and default to string.  Struct properties will require a resolvable typename.
    READ_REQUIRED_XML_ATTRIBUTE (TYPE_NAME_ATTRIBUTE,       this, TypeName,     propertyNode.baseName)        

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus StructECProperty::_WriteXml
(
MSXML2::IXMLDOMElement& parentNode
)
    {
    return __super::_WriteXml(parentNode, EC_STRUCTPROPERTY_ELEMENT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool StructECProperty::_CanOverride
(
ECPropertyCR baseProperty
) const
    {

    if (baseProperty.IsPrimitive)
        return false;
        
    if (baseProperty.IsArray)
        {
        ArrayECPropertyP arrayProp = baseProperty.GetAsArrayProperty();
        if (ARRAYKIND_Struct != arrayProp->Kind)
            return false;
        }

    // if the struct type hasn't been set yet, we will say it can override
    if (NULL == m_structType)
        return true;

    return (TypeName == baseProperty.TypeName);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring StructECProperty::_GetTypeName
(
) const
    {
    if (!EXPECTED_CONDITION (NULL != m_structType))
        return EMPTY_STRING;
    return ECClass::GetQualifiedClassName (this->Class.Schema, *m_structType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ResolveStructType
(
ECClassP & structClass,
std::wstring const& typeName,
ECPropertyCR ecProperty
)
    {
    // typeName may potentially be qualified so we must parse into a namespace prefix and short class name
    std::wstring namespacePrefix;
    std::wstring className;
    ECObjectsStatus status = ECClass::ParseClassName (namespacePrefix, className, typeName);
    if (ECOBJECTS_STATUS_Success != status)
        {
        Logger::GetLogger()->warningv (L"Can not resolve the type name '%s' as a struct type because the typeName could not be parsed.\n", typeName.c_str());
        return status;
        }
    
    ECSchemaP resolvedSchema = ecProperty.Class.Schema.GetSchemaByNamespacePrefixP (namespacePrefix);
    if (NULL == resolvedSchema)
        {
        Logger::GetLogger()->warningv (L"Can not resolve the type name '%s' as a struct type because the namespacePrefix '%s' can not be resolved to the primary or a referenced schema.\n", 
            typeName.c_str(), namespacePrefix.c_str());
        return ECOBJECTS_STATUS_SchemaNotFound;
        }

    structClass = resolvedSchema->GetClassP (className);
    if (NULL == structClass)
        {
        Logger::GetLogger()->warningv (L"Can not resolve the type name '%s' as a struct type because ECClass '%s' does not exist in the schema '%s'.\n", 
            typeName.c_str(), className.c_str(), resolvedSchema->Name.c_str());
        return ECOBJECTS_STATUS_ClassNotFound;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StructECProperty::_SetTypeName 
(
std::wstring const& typeName
)
    {
    ECClassP structClass;
    ECObjectsStatus status = ResolveStructType (structClass, typeName, *this);
    if (ECOBJECTS_STATUS_Success != status)
        {
        Logger::GetLogger()->errorv (L"Failed to set the type name of ECStructProperty '%s' to '%s' because the typeName could not be parsed into a resolvable ECClass.\n", this->Name.c_str(), typeName.c_str());        
        return status;
        }
    else
        return SetType (*structClass);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR StructECProperty::GetType
(
) const
    {        
    DEBUG_EXPECT (NULL != m_structType);
    return *m_structType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StructECProperty::SetType
(
ECClassCR structType
)
    {            
    PRECONDITION (structType.IsStruct, ECOBJECTS_STATUS_PreconditionViolated);

    // NEEDSWORK ensure the type is in a referenced schema of the class containing the property
    
    m_structType = &structType;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ArrayECProperty::_ReadXml
(
MSXML2::IXMLDOMNode& propertyNode
)
    {  
    SchemaDeserializationStatus status = __super::_ReadXml(propertyNode);
    if (status != SCHEMA_DESERIALIZATION_STATUS_Success)
        return status;

    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = propertyNode.attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;        
    
    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (MIN_OCCURS_ATTRIBUTE,          this, MinOccurs)    
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (MAX_OCCURS_ATTRIBUTE,          this, MaxOccurs)
    // For Primitive & Array properties we ignore parse errors and default to string.  Struct properties will require a resolvable typename.
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (TYPE_NAME_ATTRIBUTE,           this, TypeName)  
    if (ECOBJECTS_STATUS_Success != setterStatus)
        Logger::GetLogger()->warningv (L"Defaulting the type of ECArrayProperty '%s' to '%s' in reaction to non-fatal parse error.\n", this->Name.c_str(), this->TypeName.c_str());        

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ArrayECProperty::_WriteXml
(
MSXML2::IXMLDOMElement& parentNode
)
    {
    SchemaSerializationStatus status = __super::_WriteXml(parentNode, EC_ARRAYPROPERTY_ELEMENT);
    if (status != SCHEMA_SERIALIZATION_STATUS_Success)
        return status;
        
    MSXML2::IXMLDOMAttributePtr attributePtr;

    MSXML2::IXMLDOMElementPtr propertyPtr = parentNode.lastChild;
    if (NULL == propertyPtr)
        return SCHEMA_SERIALIZATION_STATUS_FailedToCreateXml;
        
    // verify that this really is the current array property element
    if (wcscmp(propertyPtr->nodeName, EC_ARRAYPROPERTY_ELEMENT) != 0)
        return SCHEMA_SERIALIZATION_STATUS_FailedToCreateXml;

    wchar_t buf[64];
    swprintf(buf, sizeof(buf), L"%u", m_minOccurs);
    WRITE_XML_ATTRIBUTE(MIN_OCCURS_ATTRIBUTE, buf, propertyPtr);

    if (m_maxOccurs != UINT_MAX)
        {
        swprintf(buf, sizeof(buf), L"%u", m_maxOccurs);
        WRITE_XML_ATTRIBUTE(MAX_OCCURS_ATTRIBUTE, buf, propertyPtr);
        }
    else
        {
        WRITE_XML_ATTRIBUTE(MAX_OCCURS_ATTRIBUTE, ECXML_UNBOUNDED, propertyPtr);
        }

    if (m_arrayKind == ARRAYKIND_Struct)
        {
        WRITE_XML_ATTRIBUTE(IS_STRUCT_ATTRIBUTE, L"True", propertyPtr);
        }
        
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArrayECProperty::_CanOverride
(
ECPropertyCR baseProperty
) const
    {
    return (TypeName == EMPTY_STRING) || (TypeName == baseProperty.TypeName);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring ArrayECProperty::_GetTypeName
(
) const
    {    
    switch (Kind)
        {
        case ARRAYKIND_Primitive:
            return ECXml::GetPrimitiveTypeName (m_primitiveType);
        case ARRAYKIND_Struct:
            return ECClass::GetQualifiedClassName (this->Class.Schema, *m_structType);
        default:
            return EMPTY_STRING;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::_SetTypeName 
(
std::wstring const& typeName
)
    {
    PrimitiveType primitiveType;
    ECObjectsStatus status = ECXml::ParsePrimitiveType (primitiveType, typeName);
    if (ECOBJECTS_STATUS_Success == status)
        return SetPrimitiveElementType (primitiveType);
    
    ECClassP structClass;
    status = ResolveStructType (structClass, typeName, *this);
    if (ECOBJECTS_STATUS_Success == status)
        return SetStructElementType (structClass);

    Logger::GetLogger()->errorv (L"Failed to set the type name of ArrayECProperty '%s' to '%s' because the typeName could not be parsed into a resolvable type.\n", this->Name.c_str(), typeName.c_str());        
    return ECOBJECTS_STATUS_ParseError;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayKind ArrayECProperty::GetKind
(
) const
    {
    return m_arrayKind;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType ArrayECProperty::GetPrimitiveElementType
(
) const
    {
    return m_primitiveType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetPrimitiveElementType
(
PrimitiveType primitiveType
)
    {        
    m_arrayKind = ARRAYKIND_Primitive;
    m_primitiveType = primitiveType;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ArrayECProperty::GetStructElementType
(
) const
    {
    return m_structType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetStructElementType
(
ECClassCP structType
)
    {        
    PRECONDITION (NULL != structType, ECOBJECTS_STATUS_PreconditionViolated);
    PRECONDITION (structType->IsStruct, ECOBJECTS_STATUS_PreconditionViolated);

    // NEEDSWORK ensure the type is in a referenced schema of the class containing the property
    m_arrayKind = ARRAYKIND_Struct;
    m_structType = structType;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ArrayECProperty::GetMinOccurs
(
) const
    {
    return m_minOccurs;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMinOccurs
(
UInt32 minOccurs
)
    {
    PRECONDITION (minOccurs <= m_maxOccurs, ECOBJECTS_STATUS_PreconditionViolated);
    m_minOccurs = minOccurs;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMinOccurs
(
std::wstring const& minOccurs
)
    {    
    UInt32 iMinOccurs;
    int count = swscanf (minOccurs.c_str(), L"%u", &iMinOccurs);
    if (count != 1)
        {
        Logger::GetLogger()->errorv (L"Failed to set MinOccurs of ECProperty '%s' to '%s' because the value could not be parsed.  It must be a valid unsigned integer.",
                 this->Name.c_str(), minOccurs.c_str());        
        return ECOBJECTS_STATUS_ParseError;
        }    
    SetMinOccurs (iMinOccurs);
    return ECOBJECTS_STATUS_Success;
    }


/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ArrayECProperty::GetMaxOccurs
(
) const
    {
    return m_maxOccurs;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMaxOccurs
(
UInt32 maxOccurs
)
    {
    PRECONDITION (maxOccurs >= m_minOccurs, ECOBJECTS_STATUS_PreconditionViolated);
    m_maxOccurs = maxOccurs;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMaxOccurs
(
std::wstring const& maxOccurs
)
    {    
    UInt32 iMaxOccurs;
    int count = swscanf (maxOccurs.c_str(), L"%u", &iMaxOccurs);
    if (count != 1)
        {
        if (0 == wcscmp (maxOccurs.c_str(), ECXML_UNBOUNDED))
            iMaxOccurs = UINT_MAX;
        else
            {
            Logger::GetLogger()->errorv (L"Failed to set MaxOccurs of ECProperty '%s' to '%s' because the value could not be parsed.  It must be a valid unsigned integer or the string 'unbounded'.",
                     this->Name.c_str(), maxOccurs.c_str());        
            return ECOBJECTS_STATUS_ParseError;
            }
        }
    SetMaxOccurs (iMaxOccurs);
    return ECOBJECTS_STATUS_Success;
    }

END_BENTLEY_EC_NAMESPACE