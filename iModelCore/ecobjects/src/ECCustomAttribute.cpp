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

BEGIN_BENTLEY_EC_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECCustomAttributeContainer::~IECCustomAttributeContainer() 
    {
    m_primaryCustomAttributes.clear();
    m_consolidatedCustomAttributes.clear();
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
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void IECCustomAttributeContainer::AddUniquePrimaryCustomAttributesToList
(
ECCustomAttributeCollection& returnList
) 
    {
    FOR_EACH (IECInstancePtr instance, GetPrimaryCustomAttributes(false))
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
        container->AddUniquePrimaryCustomAttributesToList(returnList);
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
    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
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
    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
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
bool      includeBaseClasses,
bool      includeSupplementalAttributes
) const
    {
    IECInstancePtr result;
    ECCustomAttributeCollection::const_iterator iter;

    if (includeSupplementalAttributes)
        {
        for (iter = m_consolidatedCustomAttributes.begin(); iter != m_consolidatedCustomAttributes.end(); iter++)
            {
            ECClassCR currentClass = (*iter)->GetClass();
            if (0 == className.compare(currentClass.GetName()))
                {
                return *iter;
                }
            }
        }

    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
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
    return GetCustomAttributeInternal (className, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetPrimaryCustomAttribute
(
WStringCR className
) const
    {
    return GetCustomAttributeInternal (className, true, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeLocal
(
WStringCR className
) const
    {
    return GetCustomAttributeInternal(className, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeLocal
(
ECClassCR   ecClass
) const
    {
    return GetCustomAttributeInternal(ecClass, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeInternal
(
ECClassCR classDefinition,
bool      includeBaseClasses,
bool      includeSupplementalAttributes
) const
    {
    IECInstancePtr result;
    ECCustomAttributeCollection::const_iterator iter;

    if (includeSupplementalAttributes)
        {
        for (iter = m_consolidatedCustomAttributes.begin(); iter != m_consolidatedCustomAttributes.end(); iter++)
            {
            ECClassCR currentClass = (*iter)->GetClass();
            if (&classDefinition == &currentClass || ECClass::ClassesAreEqualByName(&classDefinition, &currentClass))
                {
                return *iter;
                }
            }
        }

    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
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
    return GetCustomAttributeInternal (classDefinition, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetPrimaryCustomAttribute
(
ECClassCR classDefinition
) const
    {
    return GetCustomAttributeInternal (classDefinition, true, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable IECCustomAttributeContainer::GetCustomAttributes
(
bool includeBase
) const
    {
    return ECCustomAttributeInstanceIterable(*this, includeBase, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable::ECPropertyIterable(ECClassCR ecClass, bool includeBaseProperties)
    : m_ecClass(ecClass), m_includeBaseProperties(includeBaseProperties)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ECPropertyIterable::const_iterator::operator==(const_iterator const& rhs) const
    {
    return !(*this != rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable IECCustomAttributeContainer::GetPrimaryCustomAttributes
(
bool includeBase
) const
    {
    return ECCustomAttributeInstanceIterable(*this, includeBase, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECCustomAttributeContainer::SetCustomAttributeInternal
(
ECCustomAttributeCollection& customAttributeCollection,
IECInstanceR customAttributeInstance,
bool requireSchemaReference
)
    {
    ECClassCR classDefinition = customAttributeInstance.GetClass();
    if (!classDefinition.GetIsCustomAttributeClass())
        {
        BeAssert (false);
        return ECOBJECTS_STATUS_NotCustomAttributeClass;
        }

    // first need to verify that this custom attribute instance is from either the current schema or a referenced schema
    ECSchemaCP containerSchema = _GetContainerSchema();
    if (containerSchema != &(classDefinition.GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(*containerSchema, classDefinition.GetSchema()))
            {
            if (requireSchemaReference)
                {
                BeAssert (false);
                return ECOBJECTS_STATUS_SchemaNotFound;
                }
            }
        }

    // remove existing custom attributes with matching class
    ECCustomAttributeCollection::iterator iter;
    for (iter = customAttributeCollection.begin(); iter != customAttributeCollection.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (&classDefinition == &currentClass || ECClass::ClassesAreEqualByName(&classDefinition, &currentClass))
            {
            customAttributeCollection.erase(iter);
            break;
            }
        }

    customAttributeCollection.push_back(&customAttributeInstance);
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECCustomAttributeContainer::SetCustomAttribute
(
IECInstanceR customAttributeInstance
)
    {
    return SetCustomAttributeInternal(m_primaryCustomAttributes, customAttributeInstance, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECCustomAttributeContainer::SetPrimaryCustomAttribute
(
IECInstanceR customAttributeInstance
)
    {
    return SetCustomAttributeInternal(m_primaryCustomAttributes, customAttributeInstance, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECCustomAttributeContainer::SetConsolidatedCustomAttribute
(
IECInstanceR customAttributeInstance
)
    {
    return SetCustomAttributeInternal(m_consolidatedCustomAttributes, customAttributeInstance, false);
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
    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (0 == className.compare(currentClass.GetName()))
            {
            m_primaryCustomAttributes.erase(iter);
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
    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (&classDefinition == &currentClass || ECClass::ClassesAreEqualByName(&classDefinition, &currentClass))
            {
            m_primaryCustomAttributes.erase(iter);
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus IECCustomAttributeContainer::ReadCustomAttributes (BeXmlNodeR containerNode, ECSchemaReadContextR schemaContext, ECSchemaCR fallBackSchema)
    {
    InstanceReadStatus status = INSTANCE_READ_STATUS_Success;

    // allow for multiple <ECCustomAttributes> nodes, even though we only ever write one.
    for (BeXmlNodeP customAttributeNode = containerNode.GetFirstChild (); NULL != customAttributeNode; customAttributeNode = customAttributeNode->GetNextSibling ())
        {
        if (0 != strcmp (customAttributeNode->GetName (), EC_CUSTOM_ATTRIBUTES_ELEMENT))
            continue;
        for (BeXmlNodeP customAttributeClassNode = customAttributeNode->GetFirstChild(); NULL != customAttributeClassNode; customAttributeClassNode = customAttributeClassNode->GetNextSibling())
            {
            
            ECInstanceReadContextPtr context = ECInstanceReadContext::CreateContext (schemaContext, fallBackSchema, NULL);

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
SchemaWriteStatus IECCustomAttributeContainer::WriteCustomAttributes 
(
BeXmlNodeR containerNode
) const
    {
    if (m_primaryCustomAttributes.size() < 1)
        return SCHEMA_WRITE_STATUS_Success;

    SchemaWriteStatus   status = SCHEMA_WRITE_STATUS_Success;
    WString             customAttributeXml;

    ECCustomAttributeCollection::const_iterator iter;
    // Add the <ECCustomAttributes> node.
    BeXmlNodeP          customAttributeNode = containerNode.AddEmptyElement (EC_CUSTOM_ATTRIBUTES_ELEMENT);
    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
        {
        (*iter)->WriteToBeXmlNode (*customAttributeNode);
        }

    if (NULL == customAttributeNode->GetFirstChild())
        containerNode.RemoveChildNode (customAttributeNode);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECCustomAttributeContainer::CopyCustomAttributesTo
(
IECCustomAttributeContainerR destContainer
)
    {
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    FOR_EACH (IECInstancePtr customAttribute, GetPrimaryCustomAttributes(false))
        {
        status = destContainer.SetPrimaryCustomAttribute(*(customAttribute->CreateCopyThroughSerialization()));
        if (ECOBJECTS_STATUS_Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP IECCustomAttributeContainer::GetContainerSchema()
    {
    return const_cast<ECSchemaP> (_GetContainerSchema());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::const_iterator::const_iterator
(
IECCustomAttributeContainerCR container,
bool includeBase,
bool includeSupplementalAttributes
)
    {
    m_state = IteratorState::Create(container, includeBase, includeSupplementalAttributes);
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
    return ECCustomAttributeInstanceIterable::const_iterator(m_container, m_includeBaseContainers, m_includeSupplementalAttributes);
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
* @bsimethod                                    Deepak.Malkan                   04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ECCustomAttributeInstanceIterable::const_iterator::operator==(const_iterator const& rhs) const
    {
    return !(*this != rhs);
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
bool includeBase,
bool includeSupplementalAttributes
)
    {
    m_customAttributes = new ECCustomAttributeCollection();
    FOR_EACH (IECInstancePtr ptr, container.m_primaryCustomAttributes)
        m_customAttributes->push_back(ptr);

    if (includeSupplementalAttributes)
        {
        FOR_EACH (IECInstancePtr ptr, container.m_consolidatedCustomAttributes)
            m_customAttributes->push_back(ptr);
        }

    if (includeBase)
        {
        bvector<IECCustomAttributeContainerP> baseContainers;
        container._GetBaseContainers(baseContainers);
        FOR_EACH (IECCustomAttributeContainerP baseContainer, baseContainers)
            {
            if (includeSupplementalAttributes)
                baseContainer->AddUniqueCustomAttributesToList(*m_customAttributes);
            else
                baseContainer->AddUniquePrimaryCustomAttributesToList(*m_customAttributes);
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
