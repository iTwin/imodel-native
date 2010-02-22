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
ECClass::~ECClass
(
)
    {
    // NEEDSWORK make sure everything is destroyed
    Logger::GetLogger()->tracev (L"~~~~ Destroying ECClass %s\n", this->Name.c_str());
    Logger::GetLogger()->tracev  (L"     Freeing memory for %d properties\n", m_propertyMap.size());
    
    m_propertyList.clear();
    
    for each (std::pair<const wchar_t * , ECPropertyP> entry in m_propertyMap)
        delete entry.second;
    
    m_propertyMap.clear();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring const& ECClass::GetName
(
) const
    {        
    return m_name;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetName
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
std::wstring const& ECClass::GetDescription
(
) const
    {
    return m_description;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetDescription
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
std::wstring const& ECClass::GetDisplayLabel
(
) const
    {
    return (m_displayLabel.empty()) ? Name : m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetDisplayLabel
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
bool ECClass::GetIsDisplayLabelDefined
(
) const
    {
    return (!m_displayLabel.empty());        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::GetIsStruct
(
) const
    {
    return m_isStruct; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsStruct
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
ECObjectsStatus ECClass::SetIsStruct
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
bool ECClass::GetIsCustomAttributeClass
(
) const
    {
    return m_isCustomAttributeClass; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsCustomAttributeClass
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
ECObjectsStatus ECClass::SetIsCustomAttributeClass
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
bool ECClass::GetIsDomainClass
(
) const
    {
    return m_isDomainClass; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsDomainClass
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
ECObjectsStatus ECClass::SetIsDomainClass
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
ECSchemaCR ECClass::GetSchema
(
) const
    {
    return m_schema;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddProperty
(
ECPropertyP&                 pProperty
)
    {
    // NEEDSWORK - need to account for property override behaviors if the property already exists in a base class
    std::pair < stdext::hash_map<const wchar_t *, ECPropertyP>::iterator, bool > resultPair;
    resultPair = m_propertyMap.insert (std::pair<const wchar_t *, ECPropertyP> (pProperty->Name.c_str(), pProperty));
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
ECPropertyP ECClass::GetPropertyP
(
std::wstring const& propertyName
) const
    {
    stdext::hash_map<const wchar_t *, ECPropertyP>::const_iterator  propertyIterator;
    propertyIterator = m_propertyMap.find (propertyName.c_str());
    
    if ( propertyIterator != m_propertyMap.end() )
        return propertyIterator->second;
    else
        return NULL;


    // not found yet, search the inheritence hierarchy
    for each (const ECClassP& baseClass in m_baseClasses)
        {
        ECPropertyP baseProperty = baseClass->GetPropertyP (propertyName);
        if (NULL != baseProperty)
            return baseProperty;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddProperty
(
ECPropertyP ecProperty,
const std::wstring &name
)
    {
    ECObjectsStatus status = ecProperty->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return AddProperty (ecProperty);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreatePrimitiveProperty
(
PrimitiveECPropertyP &ecProperty, 
const std::wstring &name
)
    {
    ecProperty = new PrimitiveECProperty(*this);
    return AddProperty(ecProperty, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreatePrimitiveProperty
(
PrimitiveECPropertyP &ecProperty, 
const std::wstring &name,
PrimitiveType primitiveType
)
    {
    ecProperty = new PrimitiveECProperty(*this);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECOBJECTS_STATUS_Success)
        return status;
    ecProperty->Type = primitiveType;
    return ECOBJECTS_STATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateStructProperty
(
StructECPropertyP &ecProperty, 
const std::wstring &name
)
    {
    ecProperty = new StructECProperty(*this);
    return AddProperty(ecProperty, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateStructProperty
(
StructECPropertyP &ecProperty, 
const std::wstring &name,
ECClassCR structType
)
    {
    ecProperty = new StructECProperty(*this);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECOBJECTS_STATUS_Success)
        return status;
    ecProperty->Type = structType;
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateArrayProperty
(
ArrayECPropertyP &ecProperty, 
const std::wstring &name
)
    {
    ecProperty = new ArrayECProperty(*this);
    return AddProperty(ecProperty, name);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateArrayProperty
(
ArrayECPropertyP &ecProperty, 
const std::wstring &name,
PrimitiveType primitiveType
)
    {
    ecProperty = new ArrayECProperty(*this);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECOBJECTS_STATUS_Success)
        return status;
    ecProperty->PrimitiveElementType = primitiveType;
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateArrayProperty
(
ArrayECPropertyP &ecProperty, 
const std::wstring &name,
ECClassCP structType
)
    {
    ecProperty = new ArrayECProperty(*this);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECOBJECTS_STATUS_Success)
        return status;
    ecProperty->StructElementType = structType;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::CheckBaseClassCycles
(
ECClassCP thisClass, 
ECClassCP proposedParentClass
)
    {
    if (thisClass == proposedParentClass || ClassesAreEqualByName(thisClass, proposedParentClass))
        return true;
    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddBaseClass
(
ECClassCR baseClass
)
    {
    if (&(baseClass.Schema) != &(this->Schema))
        {
        bool foundRefSchema = false;
        ECSchemaReferenceList referencedSchemas = m_schema.GetReferencedSchemas();
        ECSchemaReferenceList::const_iterator schemaIterator;
        for (schemaIterator = referencedSchemas.begin(); schemaIterator != referencedSchemas.end(); schemaIterator++)
            {
            ECSchemaP refSchema = *schemaIterator;
            if (refSchema == &(baseClass.Schema))
                {
                foundRefSchema = true;
                break;
                }
            }
        if (foundRefSchema == false)
            {
            return ECOBJECTS_STATUS_SchemaNotFound;
            }
        }

    if (this == &baseClass || ClassesAreEqualByName(this, &baseClass) || TraverseBaseClasses(&CheckBaseClassCycles, true, this))
        return ECOBJECTS_STATUS_BaseClassUnacceptable;
        
    ECBaseClassesVector::const_iterator baseClassIterator;
    for (baseClassIterator = m_baseClasses.begin(); baseClassIterator != m_baseClasses.end(); baseClassIterator++)
        {
        if (*baseClassIterator == (ECClassP)&baseClass)
            {
            Logger::GetLogger()->warningv (L"Can not add class '%s' as a base class to '%s' because it already exists as a base class", baseClass.Name.c_str(), m_name.c_str());
            return ECOBJECTS_STATUS_NamedItemAlreadyExists;
            }
        }
    m_baseClasses.push_back((ECClassP)&baseClass);

    // NEEDSWORK - validate property overrides are correct

    // NEEDSWORK - what if the base class being set is just a stub and does not contain 
    // any properties.  How do we handle property overrides?
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::HasBaseClasses
(
)
    {
    return (m_baseClasses.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::Is
(
ECClassCP targetClass
)
    {
    if (NULL == targetClass)
        return false;
    
    if (ClassesAreEqualByName(this, targetClass))
        return true;
            
    return TraverseBaseClasses(&ClassesAreEqualByName, true, targetClass);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::ClassesAreEqualByName
(
ECClassCP thisClass, 
ECClassCP thatClass
)
    {
    return ((thisClass == thatClass) ||
            ( (0 == thisClass->Name.compare(thatClass->Name)) &&
              (0 == thisClass->Schema.Name.compare(thatClass->Schema.Name)) &&
              (thisClass->Schema.VersionMajor == thatClass->Schema.VersionMajor) &&
              (thisClass->Schema.VersionMinor == thatClass->Schema.VersionMinor)));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::TraverseBaseClasses
(
TraversalDelegate traverseMethod, 
bool recursive,
ECClassCP arg
)
    {
    if (m_baseClasses.size() == 0)
        return false;
        
    for each (const ECClassP& baseClass in m_baseClasses)
        {
        if (traverseMethod(baseClass, arg))
            return true;
            
        if (recursive)
            {
            if (baseClass->TraverseBaseClasses(traverseMethod, recursive, arg))
                return true;
            }
        }
        
    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECClass::ReadXmlAttributes
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
SchemaDeserializationStatus ECClass::ReadXmlContents
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
        if (ECOBJECTS_STATUS_Success != ECClass::ParseClassName (namespacePrefix, className, qualifiedClassName))
            {
            Logger::GetLogger()->warningv (L"Invalid ECSchemaXML: The ECClass '%s' contains a " EC_BASE_CLASS_ELEMENT L" element with the value '%s' that can not be parsed.", 
                this->Name.c_str(), qualifiedClassName.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }
        
        ECSchemaP resolvedSchema = Schema.GetSchemaByNamespacePrefixP (namespacePrefix);
        if (NULL == resolvedSchema)
            {
            Logger::GetLogger()->warningv  (L"Invalid ECSchemaXML: The ECClass '%s' contains a " EC_BASE_CLASS_ELEMENT L" element with the namespace prefix '%s' that can not be resolved to a referenced schema.", 
                this->Name.c_str(), namespacePrefix.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }

        ECClassP baseClass = resolvedSchema->GetClassP (className);
        if (NULL == baseClass)
            {
            Logger::GetLogger()->warningv  (L"Invalid ECSchemaXML: The ECClass '%s' contains a " EC_BASE_CLASS_ELEMENT L" element with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'", 
                this->Name.c_str(), qualifiedClassName.c_str(), className.c_str(), resolvedSchema->Name.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }

        if (ECOBJECTS_STATUS_Success != AddBaseClass(*baseClass))
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
        }

    // Build properties
    xmlNodeListPtr = classNode.selectNodes (EC_NAMESPACE_PREFIX L":" EC_PROPERTY_ELEMENT L" | " EC_NAMESPACE_PREFIX L":" EC_ARRAYPROPERTY_ELEMENT L" | " EC_NAMESPACE_PREFIX L":" EC_STRUCTPROPERTY_ELEMENT);
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {   
        ECPropertyP pProperty = NULL;
        if (0 == wcscmp (xmlNodePtr->baseName, EC_PROPERTY_ELEMENT))
            pProperty = new PrimitiveECProperty (*this);
        else if (0 == wcscmp (xmlNodePtr->baseName, EC_ARRAYPROPERTY_ELEMENT))
            pProperty = new ArrayECProperty (*this);
        else if (0 == wcscmp (xmlNodePtr->baseName, EC_STRUCTPROPERTY_ELEMENT))
            pProperty = new StructECProperty (*this);

        SchemaDeserializationStatus status = pProperty->_ReadXml(xmlNodePtr);
        if (status != SCHEMA_DESERIALIZATION_STATUS_Success)
            {
            Logger::GetLogger()->warningv  (L"Invalid ECSchemaXML: Failed to deserialize properties of ECClass '%s' in the ECSchema '%s'\n", this->Name.c_str(), this->Schema.Name.c_str());                
            return status;
            }
        
        if (ECOBJECTS_STATUS_Success != this->AddProperty (pProperty))
            {
            Logger::GetLogger()->warningv  (L"Invalid ECSchemaXML: Failed to deserialize ECClass '%s' in the ECSchema '%s' because a problem occurred while adding ECProperty '%s'\n", 
                this->Name.c_str(), this->Schema.Name.c_str(), pProperty->Name.c_str());                
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }
        }

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECClass::WriteXml
(
MSXML2::IXMLDOMElement &parentNode, 
const wchar_t *elementName
) const
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;
    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    MSXML2::IXMLDOMAttributePtr attributePtr;

    CREATE_AND_ADD_TEXT_NODE("\n    ", (&parentNode));
    CREATE_AND_ADD_TEXT_NODE("\n    ", (&parentNode));

    MSXML2::IXMLDOMElementPtr classPtr = NULL;
    
    classPtr = parentNode.ownerDocument->createNode(NODE_ELEMENT, elementName, ECXML_URI_2_0);
    
    APPEND_CHILD_TO_PARENT(classPtr, (&parentNode));
    
    WRITE_XML_ATTRIBUTE(TYPE_NAME_ATTRIBUTE, this->Name.c_str(), classPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(DESCRIPTION_ATTRIBUTE, Description, classPtr);
    if (IsDisplayLabelDefined)
        WRITE_OPTIONAL_XML_ATTRIBUTE(DISPLAY_LABEL_ATTRIBUTE, DisplayLabel, classPtr);
    WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(IS_STRUCT_ATTRIBUTE, IsStruct, classPtr);
    WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(IS_CUSTOMATTRIBUTE_ATTRIBUTE, IsCustomAttributeClass, classPtr);
    WRITE_BOOL_XML_ATTRIBUTE(IS_DOMAINCLASS_ATTRIBUTE, IsDomainClass, classPtr);
    
    for each (const ECClassP& baseClass in m_baseClasses)
        {
        MSXML2::IXMLDOMElementPtr basePtr = parentNode.ownerDocument->createNode(NODE_ELEMENT, EC_BASE_CLASS_ELEMENT, ECXML_URI_2_0);
        basePtr->text = (ECClass::GetQualifiedClassName(Schema, *baseClass)).c_str();
        
        CREATE_AND_ADD_TEXT_NODE("\n        ", classPtr);
        APPEND_CHILD_TO_PARENT(basePtr, classPtr);
        }
        
    // NEEDSWORK: Serialize Custom Attributes
    
    for each (ECPropertyP prop in Properties)
        {
        prop->_WriteXml(classPtr);
        }
    CREATE_AND_ADD_TEXT_NODE("\n    ", classPtr);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECClass::WriteXml
(
MSXML2::IXMLDOMElement& parentNode
) const
    {
    return WriteXml(parentNode, EC_CLASS_ELEMENT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::ParseClassName 
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
std::wstring ECClass::GetQualifiedClassName
(
ECSchemaCR primarySchema,
ECClassCR  ecClass
)
    {
    std::wstring const* namespacePrefix = primarySchema.ResolveNamespacePrefix (ecClass.Schema);
    if (!EXPECTED_CONDITION (NULL != namespacePrefix))
        {
        Logger::GetLogger()->warningv (L"warning: Can not qualify an ECClass name with a namespace prefix unless the schema containing the ECClass is referenced by the primary schema.\n"
            L"The class name will remain unqualified.\n  Primary ECSchema: %s\n  ECClass: %s\n ECSchema containing ECClass: %s\n", primarySchema.Name.c_str(), ecClass.Name.c_str(), ecClass.Schema.Name.c_str());
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
const ECBaseClassesVector& ECClass::GetBaseClasses
(
) const
    {
    return m_baseClasses;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyContainerCR ECClass::GetProperties
(
) const
    {
    return m_propertyContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyContainer::const_iterator  ECPropertyContainer::begin () const
    {
    return ECPropertyContainer::const_iterator(m_propertyList.begin());        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyContainer::const_iterator  ECPropertyContainer::end () const
    {
    return ECPropertyContainer::const_iterator(m_propertyList.end());        
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyContainer::const_iterator& ECPropertyContainer::const_iterator::operator++()
    {
    m_state->m_listIterator++;    
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECPropertyContainer::const_iterator::operator!= (const_iterator const& rhs) const
    {
    return (m_state->m_listIterator != rhs.m_state->m_listIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP       ECPropertyContainer::const_iterator::operator*() const
    {
    ECPropertyP pProperty = *(m_state->m_listIterator);
    return pProperty;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StrengthType ECRelationshipClass::GetStrength
(
) const
    {
    return m_strength;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrength
(
StrengthType strength
)
    {
    m_strength = strength;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrength
(
const wchar_t *strength
)
    {
    PRECONDITION (NULL != strength, ECOBJECTS_STATUS_PreconditionViolated);

    StrengthType strengthType;
    ECObjectsStatus status = ECXml::ParseStrengthType(strengthType, strength);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->errorv (L"Failed to parse the Strength string '%s' for ECRelationshipClass '%s'.\n", strength, this->Name.c_str());
    else
        SetStrength (strengthType);
        
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelatedInstanceDirection ECRelationshipClass::GetStrengthDirection
(
) const
    {
    return m_strengthDirection;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrengthDirection
(
ECRelatedInstanceDirection direction
)
    {
    m_strengthDirection = direction;
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrengthDirection
(
const wchar_t *directionString
)
    {
    PRECONDITION (NULL != directionString, ECOBJECTS_STATUS_PreconditionViolated);

    ECRelatedInstanceDirection direction;
    ECObjectsStatus status = ECXml::ParseDirectionString(direction, directionString);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->errorv (L"Failed to parse the ECRelatedInstanceDirection string '%s' for ECRelationshipClass '%s'.\n", directionString, this->Name.c_str());
    else
        SetStrengthDirection (direction);
        
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECRelationshipClass::WriteXml
(
MSXML2::IXMLDOMElement& parentNode
) const
    {
    // NEEDSWORK: Serialize constraint dependencies
    
    SchemaSerializationStatus status = __super::WriteXml(parentNode, EC_RELATIONSHIP_CLASS_ELEMENT);
    
    if (status != SCHEMA_SERIALIZATION_STATUS_Success)
        return status;
        
    MSXML2::IXMLDOMAttributePtr attributePtr;

    MSXML2::IXMLDOMElementPtr propertyPtr = parentNode.lastChild;
    if (NULL == propertyPtr)
        return SCHEMA_SERIALIZATION_STATUS_FailedToCreateXml;
        
    // verify that this really is the current relationship class element
    if (wcscmp(propertyPtr->nodeName, EC_RELATIONSHIP_CLASS_ELEMENT) != 0)
        return SCHEMA_SERIALIZATION_STATUS_FailedToCreateXml;
        
    // NEEDSWORK: Full implementation
    WRITE_XML_ATTRIBUTE(STRENGTH_ATTRIBUTE, ECXml::StrengthToString(m_strength).c_str(), propertyPtr);
    WRITE_XML_ATTRIBUTE(STRENGTHDIRECTION_ATTRIBUTE, ECXml::DirectionToString(m_strengthDirection).c_str(), propertyPtr);
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECRelationshipClass::ReadXmlAttributes
(
MSXML2::IXMLDOMNode &classNode
)
    {
    SchemaDeserializationStatus status = __super::ReadXmlAttributes(classNode);
    if (status != SCHEMA_DESERIALIZATION_STATUS_Success)
        return status;
        
    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = classNode.attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;
    
    READ_OPTIONAL_XML_ATTRIBUTE (STRENGTH_ATTRIBUTE, this, Strength)
    READ_OPTIONAL_XML_ATTRIBUTE (STRENGTHDIRECTION_ATTRIBUTE, this, StrengthDirection)
    
    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECRelationshipClass::ReadXmlContents
(
MSXML2::IXMLDOMNode &classNode
)
    {
    SchemaDeserializationStatus status = __super::ReadXmlContents(classNode);
    if (status != SCHEMA_DESERIALIZATION_STATUS_Success)
        return status;
        
    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }
    
END_BENTLEY_EC_NAMESPACE
