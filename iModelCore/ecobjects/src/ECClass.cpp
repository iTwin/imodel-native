/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECClass.cpp $
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
Class::~Class
(
)
    {
    // NEEDSWORK make sure everything is destroyed
    Logger::GetLogger()->tracev (L"~~~~ Destroying ECClass %s\n", this->Name.c_str());
    Logger::GetLogger()->tracev  (L"     Freeing memory for %d properties\n", m_propertyMap.size());
    
    m_propertyList.clear();
    
    for each (std::pair<const wchar_t * , PropertyP> entry in m_propertyMap)
        delete entry.second;
    
    m_propertyMap.clear();
    
    memset (this, 0xececdead, sizeof(this));
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& Class::GetName
(
) const
    {        
    return m_name;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::SetName
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
std::wstring const& Class::GetDescription
(
) const
    {
    return m_description;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::SetDescription
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
std::wstring const& Class::GetDisplayLabel
(
) const
    {
    return (m_displayLabel.empty()) ? Name : m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::SetDisplayLabel
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
bool Class::GetIsDisplayLabelDefined
(
) const
    {
    return (!m_displayLabel.empty());        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool Class::GetIsStruct
(
) const
    {
    return m_isStruct; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::SetIsStruct
(
bool isStruct
)
    {        
    m_isStruct = isStruct;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::SetIsStruct
(
const wchar_t * isStruct
)
    {        
    PRECONDITION (NULL != isStruct, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isStruct, isStruct);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->warningv  (L"Failed to parse the isStruct string '%s' for ECClass '%s'.  Expected values are " ECXML_TRUE L" or " ECXML_FALSE L"\n", isStruct, this->Name.c_str());
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool Class::GetIsCustomAttributeClass
(
) const
    {
    return m_isCustomAttributeClass; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::SetIsCustomAttributeClass
(
bool isCustomAttributeClass
)
    {        
    m_isCustomAttributeClass = isCustomAttributeClass;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::SetIsCustomAttributeClass
(
const wchar_t * isCustomAttributeClass
)
    {      
    PRECONDITION (NULL != isCustomAttributeClass, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isCustomAttributeClass, isCustomAttributeClass);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->warningv  (L"Failed to parse the isCustomAttributeClass string '%s' for ECClass '%s'.  Expected values are " ECXML_TRUE L" or " ECXML_FALSE L"\n", isCustomAttributeClass, this->Name.c_str());
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool Class::GetIsDomainClass
(
) const
    {
    return m_isDomainClass; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::SetIsDomainClass
(
bool isDomainClass
)
    {        
    m_isDomainClass = isDomainClass;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::SetIsDomainClass
(
const wchar_t * isDomainClass
)
    {        
    PRECONDITION (NULL != isDomainClass, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isDomainClass, isDomainClass);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->warningv  (L"Failed to parse the isDomainClass string '%s' for ECClass '%s'.  Expected values are " ECXML_TRUE L" or " ECXML_FALSE L"\n", isDomainClass, this->Name.c_str());
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaCR Class::GetSchema
(
) const
    {
    return m_schema;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::AddProperty
(
PropertyP&                 pProperty
)
    {
    // NEEDSWORK - need to account for property override behaviors if the property already exists in a base class
    std::pair < stdext::hash_map<const wchar_t *, PropertyP>::iterator, bool > resultPair;
    resultPair = m_propertyMap.insert (std::pair<const wchar_t *, PropertyP> (pProperty->Name.c_str(), pProperty));
    if (resultPair.second == false)
        {
        Logger::GetLogger()->warningv  (L"Can not create property '%s' because it already exists in the schema", pProperty->Name.c_str());
        delete pProperty;
        pProperty = NULL;        
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    m_propertyList.push_back(pProperty);
    return ECOBJECTS_STATUS_Success;
    }


/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyP Class::GetPropertyP
(
std::wstring const& propertyName
) const
    {
    stdext::hash_map<const wchar_t *, PropertyP>::const_iterator  propertyIterator;
    propertyIterator = m_propertyMap.find (propertyName.c_str());
    
    if ( propertyIterator != m_propertyMap.end() )
        return propertyIterator->second;
    else
        return NULL;


    // not found yet, search the inheritence hierarchy
    std::vector<ClassP>::const_iterator baseClassIterator;
    for each (const ClassP& baseClass in m_baseClasses)
        {
        PropertyP baseProperty = baseClass->GetPropertyP (propertyName);
        if (NULL != baseProperty)
            return baseProperty;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::AddBaseClass
(
ClassCR baseClass
)
    {
    // NEEDSWORK - ensure the base class does not already exist
    m_baseClasses.push_back ((ClassP)&baseClass);

    // NEEDSWORK - validate property overrides are correct

    // NEEDSWORK - what if the base class being set is just a stub and does not contain 
    // any properties.  How do we handle property overrides?
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
bool Class::HasBaseClasses
(
)
    {
    return (m_baseClasses.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus Class::ReadXMLAttributes
(
MSXML2::IXMLDOMNode& classNode
)
    {                
    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = classNode.attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;        

    READ_REQUIRED_XML_ATTRIBUTE (TYPE_NAME_ATTRIBUTE,           this, Name,     classNode.baseName)    
    
    // OPTIONAL attributes - If these attributes exist they MUST be valid    
    READ_OPTIONAL_XML_ATTRIBUTE (DESCRIPTION_ATTRIBUTE,         this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (DISPLAY_LABEL_ATTRIBUTE,       this, DisplayLabel)

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (IS_STRUCT_ATTRIBUTE,           this, IsStruct)
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (IS_CUSTOMATTRIBUTE_ATTRIBUTE,  this, IsCustomAttributeClass)
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (IS_DOMAINCLASS_ATTRIBUTE,      this, IsDomainClass)
    // when isDomainClass is not specified in the ECSchemaXML and isCustomAttributeClass is specified and set to true, we will default to a non-domain class
    // NEEDSWORK This behavior comes from managed ECObjects but is not specified in the ECXML specification.  Consider updating the specification.
    if ((NULL == attributePtr) && (this->IsCustomAttributeClass))
        this->SetIsDomainClass (false);

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus Class::ReadXMLContents
(
MSXML2::IXMLDOMNode& classNode
)
    {            
    // Build inheritance hierarchy 
    MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = classNode.selectNodes (EC_NAMESPACE_PREFIX L":" EC_BASE_CLASS_ELEMENT);
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {        
        std::wstring qualifiedClassName = xmlNodePtr->text;
        
        // Parse the potentially qualified class name into a namespace prefix and short class name
        std::wstring namespacePrefix;
        std::wstring className;
        if (ECOBJECTS_STATUS_Success != Class::ParseClassName (namespacePrefix, className, qualifiedClassName))
            {
            Logger::GetLogger()->warningv (L"Invalid ECSchemaXML: The ECClass '%s' contains a " EC_BASE_CLASS_ELEMENT L" element with the value '%s' that can not be parsed.", 
                this->Name.c_str(), qualifiedClassName.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXML;
            }
        
        SchemaP resolvedSchema = Schema.GetSchemaByNamespacePrefixP (namespacePrefix);
        if (NULL == resolvedSchema)
            {
            Logger::GetLogger()->warningv  (L"Invalid ECSchemaXML: The ECClass '%s' contains a " EC_BASE_CLASS_ELEMENT L" element with the namespace prefix '%s' that can not be resolved to a referenced schema.", 
                this->Name.c_str(), namespacePrefix.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXML;
            }

        ClassP baseClass = resolvedSchema->GetClassP (className);
        if (NULL == baseClass)
            {
            Logger::GetLogger()->warningv  (L"Invalid ECSchemaXML: The ECClass '%s' contains a " EC_BASE_CLASS_ELEMENT L" element with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'", 
                this->Name.c_str(), qualifiedClassName.c_str(), className.c_str(), resolvedSchema->Name.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXML;
            }

        if (ECOBJECTS_STATUS_Success != AddBaseClass(*baseClass))
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXML;
        }

    // Build properties
    xmlNodeListPtr = classNode.selectNodes (EC_NAMESPACE_PREFIX L":" EC_PROPERTY_ELEMENT L" | " EC_NAMESPACE_PREFIX L":" EC_ARRAYPROPERTY_ELEMENT L" | " EC_NAMESPACE_PREFIX L":" EC_STRUCTPROPERTY_ELEMENT);
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {   
        PropertyP pProperty = NULL;
        if (0 == wcscmp (xmlNodePtr->baseName, EC_PROPERTY_ELEMENT))
            pProperty = new PrimitiveProperty (*this);
        else if (0 == wcscmp (xmlNodePtr->baseName, EC_ARRAYPROPERTY_ELEMENT))
            pProperty = new ArrayProperty (*this);
        else if (0 == wcscmp (xmlNodePtr->baseName, EC_STRUCTPROPERTY_ELEMENT))
            pProperty = new StructProperty (*this);

        SchemaDeserializationStatus status = pProperty->_ReadXML(xmlNodePtr);
        if (status != SCHEMA_DESERIALIZATION_STATUS_Success)
            {
            Logger::GetLogger()->warningv  (L"Invalid ECSchemaXML: Failed to deserialize properties of ECClass '%s' in the ECSchema '%s'\n", this->Name.c_str(), this->Schema.Name.c_str());                
            return status;
            }
        
        if (ECOBJECTS_STATUS_Success != this->AddProperty (pProperty))
            {
            Logger::GetLogger()->warningv  (L"Invalid ECSchemaXML: Failed to deserialize ECClass '%s' in the ECSchema '%s' because a problem occurred while adding ECProperty '%s'\n", 
                this->Name.c_str(), this->Schema.Name.c_str(), pProperty->Name.c_str());                
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXML;
            }
        }

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus Class::Serialize
(
MSXML2::IXMLDOMElementPtr parentNode
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;
    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    MSXML2::IXMLDOMAttributePtr attributePtr;

    CREATE_AND_ADD_TEXT_NODE("\n    ", parentNode);
    CREATE_AND_ADD_TEXT_NODE("\n    ", parentNode);

    MSXML2::IXMLDOMElementPtr classPtr = NULL;
    
    if (NULL == dynamic_cast<RelationshipClassP>((ClassP)this))
        classPtr = parentNode->ownerDocument->createNode(NODE_ELEMENT, EC_CLASS_ELEMENT, ECXML_URI_2_0);
    else
        classPtr = parentNode->ownerDocument->createNode(NODE_ELEMENT, EC_RELATIONSHIP_CLASS_ELEMENT, ECXML_URI_2_0);
    
    APPEND_CHILD_TO_PARENT(classPtr, parentNode);
    
    WRITE_XML_ATTRIBUTE(TYPE_NAME_ATTRIBUTE, this->Name.c_str(), classPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(DESCRIPTION_ATTRIBUTE, Description, classPtr);
    if (IsDisplayLabelDefined)
        WRITE_OPTIONAL_XML_ATTRIBUTE(DISPLAY_LABEL_ATTRIBUTE, DisplayLabel, classPtr);
    WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(IS_STRUCT_ATTRIBUTE, IsStruct, classPtr);
    WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(IS_CUSTOMATTRIBUTE_ATTRIBUTE, IsCustomAttributeClass, classPtr);
    WRITE_BOOL_XML_ATTRIBUTE(IS_DOMAINCLASS_ATTRIBUTE, IsDomainClass, classPtr);
    
    for each (const ClassP& baseClass in m_baseClasses)
        {
        MSXML2::IXMLDOMElementPtr basePtr = parentNode->ownerDocument->createNode(NODE_ELEMENT, EC_BASE_CLASS_ELEMENT, ECXML_URI_2_0);
        basePtr->text = baseClass->Name.c_str();
        CREATE_AND_ADD_TEXT_NODE("\n        ", classPtr);
        APPEND_CHILD_TO_PARENT(basePtr, classPtr);
        }
        
    for each (PropertyP prop in Properties)
        {
        prop->_Serialize(classPtr);
        }
    CREATE_AND_ADD_TEXT_NODE("\n    ", classPtr);
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus Class::ParseClassName 
(
std::wstring & prefix, 
std::wstring & className, 
std::wstring const& qualifiedClassName
)
    {
    if (0 == qualifiedClassName.length())
        {
        Logger::GetLogger()->warningv  (L"Failed to parse a prefix and class name from a qualified class name because the string is empty.");
        return ECOBJECTS_STATUS_ParseError;
        }
        
    std::wstring::size_type colonIndex = qualifiedClassName.find (':');
    if (std::wstring::npos == colonIndex)
        {
        prefix.clear();
        className = qualifiedClassName;
        return ECOBJECTS_STATUS_Success;
        }

    if (qualifiedClassName.length() == colonIndex + 1)
        {
        Logger::GetLogger()->warningv  (L"Failed to parse a prefix and class name from the qualified class name '%s' because the string ends with a colon.  There must be characters after the colon.", 
            qualifiedClassName.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    if (0 == colonIndex)
        prefix.clear();
    else
        prefix = qualifiedClassName.substr (0, colonIndex);

    className = qualifiedClassName.substr (colonIndex + 1);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyContainerCR Class::GetProperties
(
) const
    {
    return m_propertyContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyContainer::const_iterator  PropertyContainer::begin () const
    {
    return PropertyContainer::const_iterator(m_propertyList.begin());        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyContainer::const_iterator  PropertyContainer::end () const
    {
    return PropertyContainer::const_iterator(m_propertyList.end());        
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyContainer::const_iterator& PropertyContainer::const_iterator::operator++()
    {
    m_state->m_listIterator++;    
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContainer::const_iterator::operator!= (const_iterator const& rhs) const
    {
    return (m_state->m_listIterator != rhs.m_state->m_listIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyP       PropertyContainer::const_iterator::operator*() const
    {
    PropertyP pProperty = *(m_state->m_listIterator);
    return pProperty;
    };

END_BENTLEY_EC_NAMESPACE
