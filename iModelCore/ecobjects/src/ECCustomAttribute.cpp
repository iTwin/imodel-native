/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECCustomAttribute.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#if defined (_WIN32) // WIP_NONPORT
#include <objbase.h>
#include <comdef.h>
#endif //defined (_WIN32) // WIP_NONPORT

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
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeInternal
(
WStringCR className,
bool      includeBaseClasses
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

    if (!includeBaseClasses)
        return NULL;
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
WStringCR className
) const
    {
    return GetCustomAttributeInternal (className, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeLocal
(
WStringCR className
) const
    {
    return GetCustomAttributeInternal(className, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeLocal
(
ECClassCR   ecClass
) const
    {
    return GetCustomAttributeInternal(ecClass, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeInternal
(
ECClassCR classDefinition,
bool      includeBaseClasses
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

    if (!includeBaseClasses)
        return NULL;

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
IECInstancePtr IECCustomAttributeContainer::GetCustomAttribute
(
ECClassCR classDefinition
) const
    {
    return GetCustomAttributeInternal (classDefinition, true);
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
InstanceReadStatus IECCustomAttributeContainer::ReadCustomAttributes (BeXmlNodeR containerNode, ECSchemaReadContextR schemaContext)
    {
    InstanceReadStatus status = INSTANCE_READ_STATUS_Success;

    // allow for multiple <ECCustomAttributes> nodes, even though we only ever write one.
    BeXmlDom::IterableNodeSet customAttributeNodes;
    containerNode.SelectChildNodes (customAttributeNodes, EC_NAMESPACE_PREFIX ":" EC_CUSTOM_ATTRIBUTES_ELEMENT);
    FOR_EACH (BeXmlNodeP& customAttributeNode, customAttributeNodes)
        {
        for (BeXmlNodeP customAttributeClassNode = customAttributeNode->GetFirstChild(); NULL != customAttributeClassNode; customAttributeClassNode = customAttributeClassNode->GetNextSibling())
            {
            ECInstanceReadContextPtr context = ECInstanceReadContext::CreateContext (schemaContext, NULL);

            IECInstancePtr  customAttributeInstance;
            WString         customAttributeXmlString;
            customAttributeClassNode->GetXmlString (customAttributeXmlString);
            status = IECInstance::ReadFromBeXmlNode (customAttributeInstance, *customAttributeClassNode, *context);
            if ( (INSTANCE_READ_STATUS_Success != status) && (INSTANCE_READ_STATUS_CommentOnly != status) )
                return status;

            if (customAttributeInstance.IsValid())
                SetCustomAttribute (*customAttributeInstance);
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus IECCustomAttributeContainer::WriteCustomAttributes (BeXmlNodeR containerNode) const
    {
    if (m_customAttributes.size() < 1)
        return SCHEMA_WRITE_STATUS_Success;

    SchemaWriteStatus   status = SCHEMA_WRITE_STATUS_Success;
    WString             customAttributeXml;

    ECCustomAttributeCollection::const_iterator iter;
    // Add the <ECCustomAttributes> node.
    BeXmlNodeP          customAttributeNode = containerNode.AddEmptyElement (EC_CUSTOM_ATTRIBUTES_ELEMENT);
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        (*iter)->WriteToBeXmlNode (*customAttributeNode);
        }

    if (NULL == customAttributeNode->GetFirstChild())
        containerNode.RemoveChildNode (customAttributeNode);

    return status;
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
IECInstancePtr const& ECCustomAttributeInstanceIterable::const_iterator::operator*() const
    {
    static IECInstancePtr s_result;
    if (m_isEnd)
        return s_result;
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
