/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECCustomAttribute.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECCustomAttributeContainer::~IECCustomAttributeContainer() 
    {
    m_primaryCustomAttributes.clear();
    m_supplementedCustomAttributes.clear();
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
    for (IECInstancePtr instance: GetCustomAttributes(false))
        {
        bool alreadyFound = false;
        ECClassCR classDefinition = instance->GetClass();

        // only add the instance if there isn't already one with the same classDefinition in the list
        for (IECInstancePtr testInstance: returnList)
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
    for (IECCustomAttributeContainerP container: baseContainers)
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
    for (IECInstancePtr instance: GetPrimaryCustomAttributes(false))
        {
        bool alreadyFound = false;
        ECClassCR classDefinition = instance->GetClass();

        // only add the instance if there isn't already one with the same classDefinition in the list
        for (IECInstancePtr testInstance: returnList)
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
    for (IECCustomAttributeContainerP container: baseContainers)
        {
        container->AddUniquePrimaryCustomAttributesToList(returnList);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::IsDefined (Utf8StringCR schemaName, Utf8StringCR className) const
    {
    ECCustomAttributeCollection::const_iterator iter;
    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        ECSchemaCR classSchema = currentClass.GetSchema();
        if (0 == className.compare(currentClass.GetName()) &&
            0 == schemaName.compare(classSchema.GetName()))
            return true;
        }

    // check base containers
    bvector<IECCustomAttributeContainerP> baseContainers;
    _GetBaseContainers(baseContainers);
    for (IECCustomAttributeContainerP container: baseContainers)
        {
        if (container->IsDefined(schemaName, className))
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
) const
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
    for (IECCustomAttributeContainerP container: baseContainers)
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
Utf8StringCR schemaName,
Utf8StringCR className,
bool      includeBaseClasses,
bool      includeSupplementalAttributes
) const
    {
    IECInstancePtr result;
    ECCustomAttributeCollection::const_iterator iter;

    if (includeSupplementalAttributes)
        {
        for (iter = m_supplementedCustomAttributes.begin(); iter != m_supplementedCustomAttributes.end(); iter++)
            {
            ECClassCR currentClass = (*iter)->GetClass();
            ECSchemaCR classSchema = currentClass.GetSchema();

            if (0 == className.compare(currentClass.GetName()) &&
                0 == schemaName.compare(classSchema.GetName()))
                {
                return *iter;
                }
            }
        }

    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        ECSchemaCR classSchema = currentClass.GetSchema();

        if (0 == className.compare(currentClass.GetName()) &&
            0 == schemaName.compare(classSchema.GetName()))
            {
            return *iter;
            }
        }

    if (!includeBaseClasses)
        return NULL;
    bvector<IECCustomAttributeContainerP> baseContainers;
    _GetBaseContainers(baseContainers);
    for (IECCustomAttributeContainerP container: baseContainers)
        {
        result = container->GetCustomAttribute(schemaName, className);
        if (result.IsValid())
            return result;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Colin.Kerr                      05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetLocalAttributeAsSupplemented(Utf8StringCR schemaName, Utf8StringCR className)
    {
    for(auto const& caIter : m_supplementedCustomAttributes)
        {
        ECClassCR caClass = caIter->GetClass();
        ECSchemaCR caSchema = caClass.GetSchema();

        if (0 == className.compare(caClass.GetName()) &&
            0 == schemaName.compare(caSchema.GetName()))
            return caIter;
        }
    
    IECInstancePtr customAttribute;
    for(auto const& caIter : m_primaryCustomAttributes)
        {
        ECClassCR caClass = caIter->GetClass();
        ECSchemaCR caSchema = caClass.GetSchema();

        if(0 == className.compare(caClass.GetName()) &&
           0 == schemaName.compare(caSchema.GetName()))
            {
            customAttribute = caIter;
            break;
            }
        }

    if (!customAttribute.IsValid())
        return customAttribute;

    IECInstancePtr caCopy = customAttribute->CreateCopyThroughSerialization();
    SetSupplementedCustomAttribute(*caCopy);
    return caCopy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttribute
(
Utf8StringCR schemaName,
Utf8StringCR className
) const
    {
    return GetCustomAttributeInternal (schemaName, className, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetPrimaryCustomAttribute
(
Utf8StringCR schemaName,
Utf8StringCR className
) const
    {
    return GetCustomAttributeInternal (schemaName, className, true, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                01/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr IECCustomAttributeContainer::GetPrimaryCustomAttributeLocal
(
Utf8StringCR schemaName,
Utf8StringCR className
) const
    {
    return GetCustomAttributeInternal(schemaName, className, false, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeLocal
(
Utf8StringCR schemaName,
Utf8StringCR className
) const
    {
    return GetCustomAttributeInternal(schemaName, className, false, true);
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
        for (iter = m_supplementedCustomAttributes.begin(); iter != m_supplementedCustomAttributes.end(); iter++)
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
    for (IECCustomAttributeContainerP container: baseContainers)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Basanta.Kharel                01/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr IECCustomAttributeContainer::GetPrimaryCustomAttributeLocal
(
    ECClassCR classDefinition
    ) const
    {
    return GetCustomAttributeInternal(classDefinition, false, false);
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
    ECCustomAttributeClassCP caClass = classDefinition.GetCustomAttributeClassCP();
    if (nullptr == caClass)
        {
        BeAssert (false);
        LOG.errorv("Cannot add instance of class %s:%s as a custom attribute because the class is not a custom attribute class",
                   classDefinition.GetSchema().GetName().c_str(), classDefinition.GetName().c_str());
        return ECObjectsStatus::NotCustomAttributeClass;
        }

    CustomAttributeContainerType containerType = _GetContainerType();
    if (!caClass->CanBeAppliedTo(containerType))
        {
        BeAssert(false);
        Utf8String caContainerTypeString = ECXml::ContainerTypeToString(caClass->GetContainerType());
        Utf8String containerTypeString = ECXml::ContainerTypeToString(containerType);
        LOG.errorv("Cannot add custom attribute of class %s:%s to a container of type %s because the custom attribute has an incompatible 'appliesTo' attribute of %s",
                   classDefinition.GetSchema().GetName().c_str(), classDefinition.GetName().c_str(), containerTypeString.c_str(), caContainerTypeString.c_str());
        return ECObjectsStatus::CustomAttributeContainerTypesNotCompatible;
        }

    // first need to verify that this custom attribute instance is from either the current schema or a referenced schema
    ECSchemaCP containerSchema = _GetContainerSchema();
    if (containerSchema != &(classDefinition.GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(*containerSchema, classDefinition.GetSchema()))
            {
            if (requireSchemaReference)
                {
                LOG.errorv("%s (used in ECSchema %s) requires a (missing) reference to ECSchema %s", 
                    classDefinition.GetFullName(), 
                    containerSchema->GetFullSchemaName().c_str(), 
                    classDefinition.GetSchema().GetFullSchemaName().c_str());
                BeAssert (false);
                return ECObjectsStatus::SchemaNotFound;
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
    return ECObjectsStatus::Success;
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
ECObjectsStatus IECCustomAttributeContainer::SetSupplementedCustomAttribute
(
IECInstanceR customAttributeInstance
)
    {
    return SetCustomAttributeInternal(m_supplementedCustomAttributes, customAttributeInstance, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::RemoveCustomAttribute
(
Utf8StringCR schemaName,
Utf8StringCR className
)
    {
    ECCustomAttributeCollection::iterator iter;
    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        ECSchemaCR classSchema = currentClass.GetSchema();

        if (0 == className.compare(currentClass.GetName()) &&
            0 == schemaName.compare(classSchema.GetName()))
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
bool IECCustomAttributeContainer::RemoveSupplementedCustomAttribute
(
Utf8StringCR schemaName,
Utf8StringCR className
)
    {
    ECCustomAttributeCollection::iterator iter;
    for (iter = m_supplementedCustomAttributes.begin(); iter != m_supplementedCustomAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        ECSchemaCR classSchema = currentClass.GetSchema();

        if (0 == className.compare(currentClass.GetName()) &&
            0 == schemaName.compare(classSchema.GetName()))
            {
            m_supplementedCustomAttributes.erase(iter);
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::RemoveSupplementedCustomAttribute
(
ECClassCR classDefinition
)
    {
    ECCustomAttributeCollection::iterator iter;
    for (iter = m_supplementedCustomAttributes.begin(); iter != m_supplementedCustomAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (&classDefinition == &currentClass || ECClass::ClassesAreEqualByName(&classDefinition, &currentClass))
            {
            m_supplementedCustomAttributes.erase(iter);
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeReadStatus IECCustomAttributeContainer::ReadCustomAttributes (BeXmlNodeR containerNode, ECSchemaReadContextR schemaContext, ECSchemaCR fallBackSchema, int ecXmlVersionMajor)
    {
    CustomAttributeReadStatus status = CustomAttributeReadStatus::Success;

    // allow for multiple <ECCustomAttributes> nodes, even though we only ever write one.
    for (BeXmlNodeP customAttributeNode = containerNode.GetFirstChild (); NULL != customAttributeNode; customAttributeNode = customAttributeNode->GetNextSibling ())
        {
        if (0 != strcmp (customAttributeNode->GetName (), EC_CUSTOM_ATTRIBUTES_ELEMENT))
            continue;

        for (BeXmlNodeP customAttributeClassNode = customAttributeNode->GetFirstChild(); NULL != customAttributeClassNode; customAttributeClassNode = customAttributeClassNode->GetNextSibling())
            {
            ECInstanceReadContextPtr context = ECInstanceReadContext::CreateContext (schemaContext, fallBackSchema, NULL);

            IECInstancePtr  customAttributeInstance;
            Utf8String         customAttributeXmlString;
            customAttributeClassNode->GetXmlString (customAttributeXmlString);
            InstanceReadStatus thisStatus = InstanceReadStatus::Success;
            ICustomAttributeDeserializerP CustomAttributeDeserializerP = CustomAttributeDeserializerManager::GetManager ().GetCustomDeserializer (customAttributeClassNode->GetName());
            if (CustomAttributeDeserializerP)
                thisStatus = CustomAttributeDeserializerP->LoadCustomAttributeFromString (customAttributeInstance, *customAttributeClassNode, *context, schemaContext, *this);
            else
                thisStatus = IECInstance::ReadFromBeXmlNode (customAttributeInstance, *customAttributeClassNode, *context);
            if (InstanceReadStatus::Success != thisStatus && InstanceReadStatus::CommentOnly != thisStatus)
                {
                // In EC3 we will fail to load the schema if any invalid custom attributes are found, for EC2 schemas we will skip the invalid attributes and continue to load the schema
                if (ecXmlVersionMajor >= 3)
                    status = CustomAttributeReadStatus::InvalidCustomAttributes;
                else if (CustomAttributeReadStatus::Success == status) 
                    status = CustomAttributeReadStatus::SkippedCustomAttributes;
                }
            if (customAttributeInstance.IsValid())
                {
                ECSchemaP containerSchema = const_cast<ECSchemaP>(_GetContainerSchema());
                ECClassP customAttribClass = const_cast<ECClassP>(&customAttributeInstance->GetClass());
                if ((_GetContainerSchema() != &(customAttributeInstance->GetClass().GetSchema()) && !ECSchema::IsSchemaReferenced(*containerSchema, customAttribClass->GetSchemaR())))
                    containerSchema->AddReferencedSchema(customAttribClass->GetSchemaR());
                if (ECObjectsStatus::CustomAttributeContainerTypesNotCompatible == SetCustomAttribute(*customAttributeInstance))
                    {
                    status = CustomAttributeReadStatus::InvalidCustomAttributes;
                    }
                }
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus IECCustomAttributeContainer::WriteCustomAttributes 
(
BeXmlWriterR xmlWriter
) const
    {
    if (m_primaryCustomAttributes.size() < 1)
        return SchemaWriteStatus::Success;

    SchemaWriteStatus   status = SchemaWriteStatus::Success;
    ECCustomAttributeCollection::const_iterator iter;
    if (m_primaryCustomAttributes.begin() == m_primaryCustomAttributes.end())
        return status;

    // Add the <ECCustomAttributes> node.
    xmlWriter.WriteElementStart(EC_CUSTOM_ATTRIBUTES_ELEMENT);
    for (iter = m_primaryCustomAttributes.begin(); iter != m_primaryCustomAttributes.end(); iter++)
        {
        Utf8CP className = (*iter)->GetClass().GetName().c_str();
        
        if (0 == BeStringUtilities::StricmpAscii(className, "UnitSpecificationAttr"))
            className = "UnitSpecification";
        else if (0 == BeStringUtilities::StricmpAscii(className, "DisplayUnitSpecificationAttr"))
            className = "DisplayUnitSpecification";

        (*iter)->WriteToBeXmlNode (xmlWriter, className);
        }
    xmlWriter.WriteElementEnd();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECCustomAttributeContainer::CopyCustomAttributesTo
(
IECCustomAttributeContainerR destContainer
) const
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    for (IECInstancePtr customAttribute: GetPrimaryCustomAttributes(false))
        {
        status = destContainer.SetPrimaryCustomAttribute(*(customAttribute->CreateCopyThroughSerialization()));
        if (ECObjectsStatus::Success != status)
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

static const IECInstancePtr s_nullInstancePtr;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr const& ECCustomAttributeInstanceIterable::const_iterator::operator*() const
    {
    if (m_isEnd)
        return s_nullInstancePtr;

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
    if (includeSupplementalAttributes && container.m_supplementedCustomAttributes.size() > 0)
        {
        for (IECInstancePtr ptr : container.m_supplementedCustomAttributes)
            m_customAttributes->push_back(ptr);
        }
    else
        {
        for (IECInstancePtr ptr : container.m_primaryCustomAttributes)
            m_customAttributes->push_back(ptr);
        }

    if (includeBase)
        {
        bvector<IECCustomAttributeContainerP> baseContainers;
        container._GetBaseContainers(baseContainers);
        for (IECCustomAttributeContainerP baseContainer: baseContainers)
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
			
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::IsDefined (Utf8StringCR className) const
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
    for (IECCustomAttributeContainerP container: baseContainers)
        {
        if (container->IsDefined(className))
            return true;
        }
    return false;
    }
		
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeInternal
(
Utf8StringCR className,
bool      includeBaseClasses,
bool      includeSupplementalAttributes
) const
    {
    IECInstancePtr result;
    ECCustomAttributeCollection::const_iterator iter;

    if (includeSupplementalAttributes)
        {
        for (iter = m_supplementedCustomAttributes.begin(); iter != m_supplementedCustomAttributes.end(); iter++)
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
    for (IECCustomAttributeContainerP container: baseContainers)
        {
        result = container->GetCustomAttribute(className);
        if (result.IsValid())
            return result;
        }
    return result;
    }
		
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Colin.Kerr                      05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetLocalAttributeAsSupplemented
(
Utf8StringCR className
)
    {
    for(auto const& caIter : m_supplementedCustomAttributes)
        {
        ECClassCR caClass = caIter->GetClass();
        if (0 == className.compare(caClass.GetName()))
            return caIter;
        }
    
    IECInstancePtr customAttribute;
    for(auto const& caIter : m_primaryCustomAttributes)
        {
        ECClassCR caClass = caIter->GetClass();
        if(0 == className.compare(caClass.GetName()))
            {
            customAttribute = caIter;
            break;
            }
        }

    if (!customAttribute.IsValid())
        return customAttribute;

    IECInstancePtr caCopy = customAttribute->CreateCopyThroughSerialization();
    SetSupplementedCustomAttribute(*caCopy);
    return caCopy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttribute
(
Utf8StringCR className
) const
    {
    return GetCustomAttributeInternal (className, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetPrimaryCustomAttribute
(
Utf8StringCR className
) const
    {
    return GetCustomAttributeInternal (className, true, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeLocal
(
Utf8StringCR className
) const
    {
    return GetCustomAttributeInternal(className, false, true);
    }
		
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::RemoveCustomAttribute
(
Utf8StringCR className
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
END_BENTLEY_ECOBJECT_NAMESPACE
