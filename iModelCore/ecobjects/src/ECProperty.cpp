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
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ClassCR Property::GetClass
(
) const
    {
    return m_class;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& Property::GetName
(
) const
    {
    return m_name;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Property::SetName
(
std::wstring const& name
)
    {        
    //NEEDSWORK name needs to be validated
    m_name = name;        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& Property::GetDescription
(
) const
    {
    return m_description;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Property::SetDescription
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
std::wstring const& Property::GetDisplayLabel
(
) const
    {
    return (m_displayLabel.empty()) ? Name : m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Property::SetDisplayLabel
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
bool Property::GetIsReadOnly
(
) const
    {
    return m_readOnly;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Property::SetIsReadOnly
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
ECObjectsStatus Property::SetIsReadOnly
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
bool Property::GetIsDisplayLabelDefined
(
) const
    {
    return (!m_displayLabel.empty());        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring Property::GetTypeName
(
) const
    {
    return this->_GetTypeName();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Property::SetTypeName
(
std::wstring typeName
)
    {
    return this->_SetTypeName (typeName);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool Property::GetIsPrimitive
(
) const
    {
    return this->_IsPrimitive();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitivePropertyP Property::GetAsPrimitiveProperty
(
) const
    {
    return dynamic_cast<PrimitivePropertyP>((PropertyP)this);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool Property::GetIsStruct
(
) const
    {
    return this->_IsStruct();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
StructPropertyP Property::GetAsStructProperty
(
) const
    {
    return dynamic_cast<StructPropertyP>((PropertyP)this);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool Property::GetIsArray
(
) const
    {
    return this->_IsArray();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayPropertyP Property::GetAsArrayProperty
(
) const
    {
    return dynamic_cast<ArrayPropertyP>((PropertyP)this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus Property::_ReadXML
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

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus Property::_Serialize
(
MSXML2::IXMLDOMElementPtr parentNode
)
    {
    return SCHEMA_SERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus PrimitiveProperty::_ReadXML
(
MSXML2::IXMLDOMNode& propertyNode
)
    {  
    SchemaDeserializationStatus status = __super::_ReadXML(propertyNode);
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
SchemaSerializationStatus PrimitiveProperty::_Serialize
(
MSXML2::IXMLDOMElementPtr parentNode
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    MSXML2::IXMLDOMAttributePtr attributePtr;

    CREATE_AND_ADD_TEXT_NODE("\n        ", parentNode);

    MSXML2::IXMLDOMElementPtr propertyPtr = parentNode->ownerDocument->createNode(NODE_ELEMENT, EC_PROPERTY_ELEMENT, ECXML_URI_2_0);;
    APPEND_CHILD_TO_PARENT(propertyPtr, parentNode);
    
    WRITE_XML_ATTRIBUTE(PROPERTY_NAME_ATTRIBUTE, this->GetName().c_str(), propertyPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(TYPE_NAME_ATTRIBUTE, TypeName, propertyPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(DESCRIPTION_ATTRIBUTE, Description, propertyPtr);
    if (IsDisplayLabelDefined)
        WRITE_OPTIONAL_XML_ATTRIBUTE(DISPLAY_LABEL_ATTRIBUTE, DisplayLabel, propertyPtr);
    WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(READONLY_ATTRIBUTE, IsReadOnly, propertyPtr);
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring PrimitiveProperty::_GetTypeName
(
) const
    {
    return ECXml::GetPrimitiveTypeName (m_primitiveType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveProperty::_SetTypeName 
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
PrimitiveType PrimitiveProperty::GetType
(
) const
    {
    return m_primitiveType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveProperty::SetType
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
std::wstring GetQualifiedClassName
(
SchemaCR primarySchema,
ClassCR  ecClass
)
    {
    std::wstring const* namespacePrefix = primarySchema.ResolveNamespacePrefix (ecClass.Schema);
    if (!EXPECTED_CONDITION (NULL != namespacePrefix))
        {
        Logger::GetLogger()->warningv (L"warning: Can not qualify an ECClass name with a namespace prefix unless the schema containing the ECClass is referenced by the primary schema.\n"
            L"The class name will remain unqualified.\n  Primary Schema: %s\n  ECClass: %s\n Schema containing ECClass: %s\n", primarySchema.Name.c_str(), ecClass.Name.c_str(), ecClass.Schema.Name.c_str());
        return ecClass.Name;
        }
    if (namespacePrefix->empty())
        return ecClass.Name;
    else
        return *namespacePrefix + L":" + ecClass.Name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus StructProperty::_ReadXML
(
MSXML2::IXMLDOMNode& propertyNode
)
    {  
    SchemaDeserializationStatus status = __super::_ReadXML(propertyNode);
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
SchemaSerializationStatus StructProperty::_Serialize
(
MSXML2::IXMLDOMElementPtr parentNode
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    MSXML2::IXMLDOMAttributePtr attributePtr;

    CREATE_AND_ADD_TEXT_NODE("\n        ", parentNode);

    MSXML2::IXMLDOMElementPtr propertyPtr = parentNode->ownerDocument->createNode(NODE_ELEMENT, EC_STRUCTPROPERTY_ELEMENT, ECXML_URI_2_0);;
    APPEND_CHILD_TO_PARENT(propertyPtr, parentNode);
    
    WRITE_XML_ATTRIBUTE(PROPERTY_NAME_ATTRIBUTE, this->GetName().c_str(), propertyPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(TYPE_NAME_ATTRIBUTE, TypeName, propertyPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(DESCRIPTION_ATTRIBUTE, Description, propertyPtr);
    if (IsDisplayLabelDefined)
        WRITE_OPTIONAL_XML_ATTRIBUTE(DISPLAY_LABEL_ATTRIBUTE, DisplayLabel, propertyPtr);
    WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(READONLY_ATTRIBUTE, IsReadOnly, propertyPtr);
    
    return status;    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring StructProperty::_GetTypeName
(
) const
    {
    if (!EXPECTED_CONDITION (NULL != m_structType))
        return EMPTY_STRING;
    return GetQualifiedClassName (this->Class.Schema, *m_structType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ResolveStructType
(
ClassP & structClass,
std::wstring const& typeName,
PropertyCR ecProperty
)
    {
    // typeName may potentially be qualified so we must parse into a namespace prefix and short class name
    std::wstring namespacePrefix;
    std::wstring className;
    ECObjectsStatus status = Class::ParseClassName (namespacePrefix, className, typeName);
    if (ECOBJECTS_STATUS_Success != status)
        {
        Logger::GetLogger()->warningv (L"Can not resolve the type name '%s' as a struct type because the typeName could not be parsed.\n", typeName.c_str());
        return status;
        }
    
    SchemaP resolvedSchema = ecProperty.Class.Schema.GetSchemaByNamespacePrefixP (namespacePrefix);
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
ECObjectsStatus StructProperty::_SetTypeName 
(
std::wstring const& typeName
)
    {
    ClassP structClass;
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
ClassCR StructProperty::GetType
(
) const
    {        
    DEBUG_EXPECT (NULL != m_structType);
    return *m_structType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StructProperty::SetType
(
ClassCR structType
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
SchemaDeserializationStatus ArrayProperty::_ReadXML
(
MSXML2::IXMLDOMNode& propertyNode
)
    {  
    SchemaDeserializationStatus status = __super::_ReadXML(propertyNode);
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
SchemaSerializationStatus ArrayProperty::_Serialize
(
MSXML2::IXMLDOMElementPtr parentNode
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    MSXML2::IXMLDOMAttributePtr attributePtr;

    CREATE_AND_ADD_TEXT_NODE("\n        ", parentNode);

    MSXML2::IXMLDOMElementPtr propertyPtr = parentNode->ownerDocument->createNode(NODE_ELEMENT, EC_ARRAYPROPERTY_ELEMENT, ECXML_URI_2_0);;
    APPEND_CHILD_TO_PARENT(propertyPtr, parentNode);
    
    WRITE_XML_ATTRIBUTE(PROPERTY_NAME_ATTRIBUTE, this->GetName().c_str(), propertyPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(TYPE_NAME_ATTRIBUTE, TypeName, propertyPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(DESCRIPTION_ATTRIBUTE, Description, propertyPtr);
    if (IsDisplayLabelDefined)
        WRITE_OPTIONAL_XML_ATTRIBUTE(DISPLAY_LABEL_ATTRIBUTE, DisplayLabel, propertyPtr);
    WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(READONLY_ATTRIBUTE, IsReadOnly, propertyPtr);
    
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

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring ArrayProperty::_GetTypeName
(
) const
    {    
    switch (ElementClassification)
        {
        case ELEMENTCLASSIFICATION_Primitive:
            return ECXml::GetPrimitiveTypeName (m_primitiveType);
        case ELEMENTCLASSIFICATION_Struct:
            return GetQualifiedClassName (this->Class.Schema, *m_structType);
        default:
            return EMPTY_STRING;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayProperty::_SetTypeName 
(
std::wstring const& typeName
)
    {
    PrimitiveType primitiveType;
    ECObjectsStatus status = ECXml::ParsePrimitiveType (primitiveType, typeName);
    if (ECOBJECTS_STATUS_Success == status)
        return SetPrimitiveElementType (primitiveType);
    
    ClassP structClass;
    status = ResolveStructType (structClass, typeName, *this);
    if (ECOBJECTS_STATUS_Success == status)
        return SetStructElementType (structClass);

    Logger::GetLogger()->errorv (L"Failed to set the type name of ArrayProperty '%s' to '%s' because the typeName could not be parsed into a resolvable type.\n", this->Name.c_str(), typeName.c_str());        
    return ECOBJECTS_STATUS_ParseError;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayElementClassification ArrayProperty::GetElementClassification
(
) const
    {
    return m_elementClassification;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType ArrayProperty::GetPrimitiveElementType
(
) const
    {
    return m_primitiveType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayProperty::SetPrimitiveElementType
(
PrimitiveType primitiveType
)
    {        
    m_elementClassification = ELEMENTCLASSIFICATION_Primitive;
    m_primitiveType = primitiveType;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ClassCP ArrayProperty::GetStructElementType
(
) const
    {
    return m_structType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayProperty::SetStructElementType
(
ClassCP structType
)
    {        
    PRECONDITION (NULL != structType, ECOBJECTS_STATUS_PreconditionViolated);
    PRECONDITION (structType->IsStruct, ECOBJECTS_STATUS_PreconditionViolated);

    // NEEDSWORK ensure the type is in a referenced schema of the class containing the property
    m_elementClassification = ELEMENTCLASSIFICATION_Struct;
    m_structType = structType;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ArrayProperty::GetMinOccurs
(
) const
    {
    return m_minOccurs;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayProperty::SetMinOccurs
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
ECObjectsStatus ArrayProperty::SetMinOccurs
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
UInt32 ArrayProperty::GetMaxOccurs
(
) const
    {
    return m_maxOccurs;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayProperty::SetMaxOccurs
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
ECObjectsStatus ArrayProperty::SetMaxOccurs
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