/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECCustomAttribute.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include <objbase.h>
#include <comdef.h>

BEGIN_BENTLEY_EC_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECCustomAttributeContainer::~IECCustomAttributeContainer() 
    {
    m_customAttributes.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void IECCustomAttributeContainer::_GetBaseContainers
(
bvector<IECCustomAttributeContainerP>& returnList
)const
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void IECCustomAttributeContainer::AddUniqueCustomAttributesToList
(
ECCustomAttributeCollection& returnList
) 
    {
    FOR_EACH (IECInstancePtr instance, GetCustomAttributes(false))
        {
        bool alreadyFound = false;
        ECClassCR classDefinition = instance->GetClass();

        // only add the instance if there isn't already one with the same classDefinition in the list
        FOR_EACH (IECInstancePtr testInstance, returnList)
            {
            ECClassCR testClass = testInstance->GetClass();
            if (&classDefinition == &testClass || ECClass::ClassesAreEqualByName(&classDefinition, &testClass))
                {
                alreadyFound = true;
                break;
                }
            }
        if (!alreadyFound)
            returnList.push_back(instance);
        }

    // do base containers
    bvector<IECCustomAttributeContainerP> baseContainers;
    _GetBaseContainers(baseContainers);
    FOR_EACH (IECCustomAttributeContainerP container, baseContainers)
        {
        container->AddUniqueCustomAttributesToList(returnList);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::IsDefined
(
WStringCR className
) 
    {
    ECCustomAttributeCollection::const_iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (0 == className.compare(currentClass.GetName()))
            return true;
        }

    // check base containers
    bvector<IECCustomAttributeContainerP> baseContainers;
    _GetBaseContainers(baseContainers);
    FOR_EACH (IECCustomAttributeContainerP container, baseContainers)
        {
        if (container->IsDefined(className))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::IsDefined
(
ECClassCR classDefinition
) 
    {
    ECCustomAttributeCollection::const_iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (&classDefinition == &currentClass || ECClass::ClassesAreEqualByName(&classDefinition, &currentClass))
            return true;
        }
    // check base containers
    bvector<IECCustomAttributeContainerP> baseContainers;
    _GetBaseContainers(baseContainers);
    FOR_EACH (IECCustomAttributeContainerP container, baseContainers)
        {
        if (container->IsDefined(classDefinition))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttribute
(
WStringCR className
) const
    {
    IECInstancePtr result;
    ECCustomAttributeCollection::const_iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (0 == className.compare(currentClass.GetName()))
            {
            return *iter;
            }
        }
    bvector<IECCustomAttributeContainerP> baseContainers;
    _GetBaseContainers(baseContainers);
    FOR_EACH (IECCustomAttributeContainerP container, baseContainers)
        {
        result = container->GetCustomAttribute(className);
        if (result.IsValid())
            return result;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttribute
(
ECClassCR classDefinition
) const
    {
    IECInstancePtr result;
    ECCustomAttributeCollection::const_iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (&classDefinition == &currentClass || ECClass::ClassesAreEqualByName(&classDefinition, &currentClass))
            {
            return *iter;
            }
        }
    bvector<IECCustomAttributeContainerP> baseContainers;
    _GetBaseContainers(baseContainers);
    FOR_EACH (IECCustomAttributeContainerP container, baseContainers)
        {
        result = container->GetCustomAttribute(classDefinition);
        if (result.IsValid())
            return result;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable IECCustomAttributeContainer::GetCustomAttributes
(
bool includeBase
) const
    {
    return ECCustomAttributeInstanceIterable(*this, includeBase);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECCustomAttributeContainer::SetCustomAttribute
(
IECInstanceR customAttributeInstance
)
    {
    ECClassCR classDefinition = customAttributeInstance.GetClass();
    if (!classDefinition.GetIsCustomAttributeClass())
        {
        assert (false);
        return ECOBJECTS_STATUS_NotCustomAttributeClass;
        }

    // first need to verify that this custom attribute instance is from either the current schema or a referenced schema
    ECSchemaCP containerSchema = _GetContainerSchema();
    if (containerSchema != &(classDefinition.GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(*containerSchema, classDefinition.GetSchema()))
            {
            assert (false);
            return ECOBJECTS_STATUS_SchemaNotFound;
            }
        }

    // remove existing custom attributes with matching class
    ECCustomAttributeCollection::iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (&classDefinition == &currentClass || ECClass::ClassesAreEqualByName(&classDefinition, &currentClass))
            {
            m_customAttributes.erase(iter);
            break;
            }
        }

    m_customAttributes.push_back(&customAttributeInstance);
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::RemoveCustomAttribute
(
WStringCR className
)
    {
    ECCustomAttributeCollection::iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (0 == className.compare(currentClass.GetName()))
            {
            m_customAttributes.erase(iter);
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::RemoveCustomAttribute
(
ECClassCR classDefinition
)
    {
    ECCustomAttributeCollection::iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (&classDefinition == &currentClass || ECClass::ClassesAreEqualByName(&classDefinition, &currentClass))
            {
            m_customAttributes.erase(iter);
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus IECCustomAttributeContainer::ReadCustomAttributes
(
MSXML2::IXMLDOMNode&       containerNode,
ECSchemaCR                 schema,
IStandaloneEnablerLocatorR standaloneEnablerLocator
)
    {
    InstanceDeserializationStatus status = INSTANCE_DESERIALIZATION_STATUS_Success;
    MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr;
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    MSXML2::IXMLDOMNodeListPtr attributeInstances;
    MSXML2::IXMLDOMNodePtr instancePtr;

    xmlNodeListPtr = containerNode.selectNodes (EC_NAMESPACE_PREFIX L":" EC_CUSTOM_ATTRIBUTES_ELEMENT);
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {
        MSXML2::IXMLDOMNodeListPtr attributeInstances = xmlNodePtr->childNodes;
        while (NULL != (instancePtr = attributeInstances->nextNode()))
            {
            ECInstanceDeserializationContextPtr context = ECInstanceDeserializationContext::CreateContext (schema, standaloneEnablerLocator);

            IECInstancePtr ptr;
            status = IECInstance::ReadXmlFromString(ptr, (WCharCP) instancePtr->Getxml(), *context);
            if ( (INSTANCE_DESERIALIZATION_STATUS_Success != status) && (INSTANCE_DESERIALIZATION_STATUS_CommentOnly != status) )
                return status;
            if (ptr.IsValid())
                SetCustomAttribute(*ptr);
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus IECCustomAttributeContainer::WriteCustomAttributes
(
MSXML2::IXMLDOMNode& propertyNode
) const
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;
    WString customAttributeXml;
    MSXML2::IXMLDOMAttributePtr attributePtr;
    if (m_customAttributes.size() < 1)
        return status;

    ECCustomAttributeCollection::const_iterator iter;
    MSXML2::IXMLDOMNodePtr customAttributesNodePtr = propertyNode.ownerDocument->createNode(NODE_ELEMENT, EC_CUSTOM_ATTRIBUTES_ELEMENT, ECXML_URI_2_0);
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        (*iter)->WriteXmlToString(customAttributeXml, false);
        MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
        if (S_OK != xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)))
            return SCHEMA_SERIALIZATION_STATUS_FailedToInitializeMsmxl;

        xmlDocPtr->loadXML(customAttributeXml.c_str());
        MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = xmlDocPtr->childNodes;
        MSXML2::IXMLDOMNodePtr xmlNodePtr;
        _bstr_t baseName;
        while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
            {
            if (xmlNodePtr->nodeType != NODE_ELEMENT)
                continue;

            MSXML2::IXMLDOMElementPtr caPtr = customAttributesNodePtr->ownerDocument->createNode(NODE_ELEMENT, xmlNodePtr->baseName, ECXML_URI_2_0);
            APPEND_CHILD_TO_PARENT(caPtr, customAttributesNodePtr);
            MSXML2::IXMLDOMNamedNodeMapPtr attributes = xmlNodePtr->attributes;
            long numAttributes;
            attributes->get_length(&numAttributes);
            for (int index = 0; index < numAttributes; index++)
                {
                MSXML2::IXMLDOMNodePtr attrib;
                attributes->get_item(index, &attrib);
                WRITE_XML_ATTRIBUTE(attrib->nodeName, attrib->nodeValue, caPtr);
                }
            AddCustomAttributeProperties(xmlNodePtr, caPtr);
            }
        }
    if (customAttributesNodePtr->hasChildNodes())
        APPEND_CHILD_TO_PARENT(customAttributesNodePtr, (&propertyNode));
    return status;
    }

SchemaSerializationStatus IECCustomAttributeContainer::AddCustomAttributeProperties
(
MSXML2::IXMLDOMNode& oldNode, 
MSXML2::IXMLDOMNode& newNode
) const
    {
    if (!oldNode.hasChildNodes())
        return SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = oldNode.childNodes;
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {
        if (xmlNodePtr->nodeType != NODE_ELEMENT)
            continue;

        MSXML2::IXMLDOMNodePtr propertyPtr = newNode.ownerDocument->createNode(NODE_ELEMENT, xmlNodePtr->baseName, xmlNodePtr->namespaceURI);
        APPEND_CHILD_TO_PARENT(propertyPtr, (&newNode));
        MSXML2::IXMLDOMNodePtr childTextNodePtr = xmlNodePtr->firstChild;
        WString nodeType = childTextNodePtr->nodeTypeString.GetBSTR();
        if (0 == nodeType.compare(L"text"))
            propertyPtr->text = childTextNodePtr->text;
        AddCustomAttributeProperties(xmlNodePtr, propertyPtr);
        }

    return SCHEMA_SERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::const_iterator::const_iterator
(
IECCustomAttributeContainerCR container,
bool includeBase
)
    {
    m_state = IteratorState::Create(container, includeBase);
    if (m_state->m_customAttributesIterator == m_state->m_customAttributes->end())
        m_isEnd = true;
    else
        m_isEnd = false;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::const_iterator ECCustomAttributeInstanceIterable::begin() const
    {
    return ECCustomAttributeInstanceIterable::const_iterator(m_container, m_includeBaseContainers);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::const_iterator ECCustomAttributeInstanceIterable::end () const
    {
    return ECCustomAttributeInstanceIterable::const_iterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::const_iterator& ECCustomAttributeInstanceIterable::const_iterator::operator++()
    {
    m_state->m_customAttributesIterator++;
    if (m_state->m_customAttributesIterator == m_state->m_customAttributes->end())
        m_isEnd = true;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECCustomAttributeInstanceIterable::const_iterator::operator!= (const_iterator const& rhs) const
    {
    if (m_isEnd && rhs.m_isEnd)
        return false;
    if (m_state.IsNull() && !(rhs.m_state.IsNull()))
        return true;
    if (!(m_state.IsNull()) && rhs.m_state.IsNull())
        return true;
    return (m_state->m_customAttributesIterator != rhs.m_state->m_customAttributesIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECCustomAttributeInstanceIterable::const_iterator::operator*() const
    {
    IECInstancePtr result;
    if (m_isEnd)
        return result;
    return *(m_state->m_customAttributesIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::IteratorState::IteratorState
(
IECCustomAttributeContainerCR container,
bool includeBase
)
    {
    m_customAttributes = new ECCustomAttributeCollection();
    FOR_EACH (IECInstancePtr ptr, container.m_customAttributes)
        m_customAttributes->push_back(ptr);
    if (includeBase)
        {
        bvector<IECCustomAttributeContainerP> baseContainers;
        container._GetBaseContainers(baseContainers);
        FOR_EACH (IECCustomAttributeContainerP baseContainer, baseContainers)
            {
            baseContainer->AddUniqueCustomAttributesToList(*m_customAttributes);
            }
        }
    m_customAttributesIterator = m_customAttributes->begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::IteratorState::~IteratorState()
    {
    delete m_customAttributes;
    }
END_BENTLEY_EC_NAMESPACE
