/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECCustomAttributeContainer::~IECCustomAttributeContainer()
    {
    m_customAttributes.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IECCustomAttributeContainer::_GetBaseContainers
(
bvector<IECCustomAttributeContainerP>& returnList
)const
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::IsDefined (Utf8StringCR schemaName, Utf8StringCR className) const
    {
    return IsDefinedInternal(schemaName, className, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::IsDefined (ECClassCR classDefinition) const
    {
    return IsDefinedInternal(classDefinition, true);
    }

//-------------------------------------------------------------------------------------//
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-----//
bool IECCustomAttributeContainer::IsDefinedLocal(Utf8StringCR schemaName, Utf8StringCR className) const
    {
    return IsDefinedInternal(schemaName, className, false);
    }

//-------------------------------------------------------------------------------------//
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-----//
bool IECCustomAttributeContainer::IsDefinedLocal(ECClassCR classDefinition) const
    {
    return IsDefinedInternal(classDefinition, false);
    }

//-------------------------------------------------------------------------------------//
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-----//
bool IECCustomAttributeContainer::IsDefinedInternal (Utf8StringCR schemaName, Utf8StringCR className, bool includeBaseClasses) const
    {
    ECCustomAttributeCollection::const_iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        ECSchemaCR classSchema = currentClass.GetSchema();
        if (0 == className.compare(currentClass.GetName()) &&
            0 == schemaName.compare(classSchema.GetName()))
            return true;
        }

    if (!includeBaseClasses)
        return false;

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

//-------------------------------------------------------------------------------------//
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-----//
bool IECCustomAttributeContainer::IsDefinedInternal (ECClassCR classDefinition, bool includeBaseClasses) const
    {
    ECCustomAttributeCollection::const_iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        if (&classDefinition == &currentClass || ECClass::ClassesAreEqualByName(&classDefinition, &currentClass))
            return true;
        }

    if (!includeBaseClasses)
        return false;

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeInternal
(
Utf8StringCR schemaName,
Utf8StringCR className,
bool      includeBaseClasses
) const
    {
    IECInstancePtr result;
    ECCustomAttributeCollection::const_iterator iter;

    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttribute
(
Utf8StringCR schemaName,
Utf8StringCR className
) const
    {
    return GetCustomAttributeInternal (schemaName, className, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeLocal
(
Utf8StringCR schemaName,
Utf8StringCR className
) const
    {
    return GetCustomAttributeInternal(schemaName, className, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeLocal
(
ECClassCR   ecClass
) const
    {
    return GetCustomAttributeInternal(ecClass, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    for (IECCustomAttributeContainerP container: baseContainers)
        {
        result = container->GetCustomAttribute(classDefinition);
        if (result.IsValid())
            return result;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttribute
(
ECClassCR classDefinition
) const
    {
    return GetCustomAttributeInternal (classDefinition, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable IECCustomAttributeContainer::GetCustomAttributes
(
bool includeBase
) const
    {
    return ECCustomAttributeInstanceIterable(*this, includeBase);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable::ECPropertyIterable(ECClassCR ecClass, bool includeBaseProperties)
    : m_ecClass(ecClass), m_includeBaseProperties(includeBaseProperties)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ECPropertyIterable::const_iterator::operator==(const_iterator const& rhs) const
    {
    return !(*this != rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
        Utf8String caContainerTypeString = SchemaParseUtils::ContainerTypeToString(caClass->GetContainerType());
        Utf8String containerTypeString = SchemaParseUtils::ContainerTypeToString(containerType);
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECCustomAttributeContainer::SetCustomAttribute
(
IECInstanceR customAttributeInstance
)
    {
    return SetCustomAttributeInternal(m_customAttributes, customAttributeInstance, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::RemoveCustomAttribute
(
Utf8StringCR schemaName,
Utf8StringCR className
)
    {
    ECCustomAttributeCollection::iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        ECClassCR currentClass = (*iter)->GetClass();
        ECSchemaCR classSchema = currentClass.GetSchema();

        if (0 == className.compare(currentClass.GetName()) &&
            0 == schemaName.compare(classSchema.GetName()))
            {
            m_customAttributes.erase(iter);
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeReadStatus IECCustomAttributeContainer::ReadCustomAttributes (pugi::xml_node containerNode, ECSchemaReadContextR schemaContext, ECSchemaCR fallBackSchema)
    {
    CustomAttributeReadStatus status = CustomAttributeReadStatus::Success;

    // allow for multiple <ECCustomAttributes> nodes, even though we only ever write one.
    for (pugi::xml_node customAttributeNode : containerNode.children(ECXML_CUSTOM_ATTRIBUTES_ELEMENT))
        {
        for (pugi::xml_node customAttributeClassNode : customAttributeNode.children())
            {
            if(customAttributeClassNode.type() != pugi::xml_node_type::node_element)
                continue;
            ECInstanceReadContextPtr context = ECInstanceReadContext::CreateContext (schemaContext, fallBackSchema, NULL);

            IECInstancePtr  customAttributeInstance;
            InstanceReadStatus thisStatus = InstanceReadStatus::Success;
            ICustomAttributeDeserializerP CustomAttributeDeserializerP = CustomAttributeDeserializerManager::GetManager ().GetCustomDeserializer (customAttributeClassNode.name());
            Utf8String ns = customAttributeClassNode.attribute("xmlns").as_string();
            if (!Utf8String::IsNullOrEmpty(ns.c_str()) && !SchemaParseUtils::IsFullSchemaNameFormatValidForVersion(ns.c_str(), _GetContainerSchema()->GetOriginalECXmlVersionMajor(), _GetContainerSchema()->GetOriginalECXmlVersionMinor()))
                {
                LOG.errorv("Custom attribute namespaces must contain a valid 3.2 full schema name in the form <schemaName>.RR.ww.mm.  xmlns '%s' for custom attribute class '%s'", ns.c_str(), customAttributeClassNode.name());
                return CustomAttributeReadStatus::InvalidCustomAttributes;
                }
            if (CustomAttributeDeserializerP)
                thisStatus = CustomAttributeDeserializerP->LoadCustomAttributeFromString (customAttributeInstance, customAttributeClassNode, *context, schemaContext, *this);
            else
                thisStatus = IECInstance::ReadFromBeXmlNode (customAttributeInstance, customAttributeClassNode, *context);
            if (InstanceReadStatus::Success != thisStatus && InstanceReadStatus::CommentOnly != thisStatus)
                {
                // In EC3 we will fail to load the schema if any invalid custom attributes are found, for EC2 schemas we will skip the invalid attributes and continue to load the schema
                if (fallBackSchema.OriginalECXmlVersionAtLeast(ECVersion::V3_0))
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
                ECObjectsStatus caSetStatus = SetCustomAttribute(*customAttributeInstance);
                if (ECObjectsStatus::Success != caSetStatus)
                    {
                    // In EC3 we will fail to load the schema if any invalid custom attributes are found, for EC2 schemas we will skip the invalid attributes and continue to load the schema
                    if (fallBackSchema.OriginalECXmlVersionAtLeast(ECVersion::V3_0))
                        status = CustomAttributeReadStatus::InvalidCustomAttributes;
                    else if (CustomAttributeReadStatus::Success == status)
                        status = CustomAttributeReadStatus::SkippedCustomAttributes;
                    }
                }
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus IECCustomAttributeContainer::WriteCustomAttributes
(
BeXmlWriterR xmlWriter,
ECVersion ecXmlVersion
) const
    {
    if (m_customAttributes.size() == 0)
        return SchemaWriteStatus::Success;

    if (m_customAttributes.begin() == m_customAttributes.end())
        return SchemaWriteStatus::Success;

    // Add the <ECCustomAttributes> node.
    xmlWriter.WriteElementStart(ECXML_CUSTOM_ATTRIBUTES_ELEMENT);
    ECCustomAttributeCollection::const_iterator iter;
    for (iter = m_customAttributes.begin(); iter != m_customAttributes.end(); iter++)
        {
        Utf8CP className = (*iter)->GetClass().GetName().c_str();

        if (0 == BeStringUtilities::StricmpAscii(className, "UnitSpecificationAttr"))
            className = "UnitSpecification";
        else if (0 == BeStringUtilities::StricmpAscii(className, "DisplayUnitSpecificationAttr"))
            className = "DisplayUnitSpecification";

        if (ecXmlVersion == ECVersion::V2_0)
            (*iter)->WriteToBeXmlNode(xmlWriter, className);
        else
            (*iter)->WriteToBeXmlNodeLatestVersion(xmlWriter, className);
        }
    xmlWriter.WriteElementEnd();

    return SchemaWriteStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
template <typename ITERATOR_TYPE>
void IECCustomAttributeContainer::_WriteCustomAttributes(BeJsValue& parentNode, ITERATOR_TYPE& iterator) const
    {
    auto caNode = parentNode[ECJSON_CUSTOM_ATTRIBUTES_ELEMENT];
    caNode.toArray();
    
    for (auto pInstance : iterator)
        {
        auto instanceJson = caNode.appendValue();
        JsonEcInstanceWriter::WriteInstanceToSchemaJson(instanceJson, *pInstance);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void IECCustomAttributeContainer::WriteFilteredCustomAttributes(BeJsValue& parentNode, bool(*skipClassPredicate)(Utf8CP)) const
    {
    if(skipClassPredicate == nullptr)
        {
        WriteCustomAttributes(parentNode);
        return;
        }

    auto customAttributes = GetCustomAttributes(false);
    auto begin = customAttributes.begin();
    auto end = customAttributes.end();

    if(begin == end)
        return;
    
    std::vector<IECInstancePtr> filteredCAs;
    for (auto it = begin; it != end; ++it)
        {
        IECInstancePtr pInstance = *it;

        if(skipClassPredicate(pInstance->GetClass().GetFullName()))
            continue;
        
        filteredCAs.push_back(pInstance);
        }

    if(!filteredCAs.empty())
        _WriteCustomAttributes(parentNode, filteredCAs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void IECCustomAttributeContainer::WriteCustomAttributes(BeJsValue& parentNode) const
    {
    auto customAttributes = GetCustomAttributes(false);
    auto begin = customAttributes.begin();
    auto end = customAttributes.end();

    if(begin == end)
        return;
    _WriteCustomAttributes(parentNode, customAttributes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECCustomAttributeContainer::CopyCustomAttributesTo
(
IECCustomAttributeContainerR destContainer,
bool copyReferences
) const
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    for (auto&& sourceCustomAttribute: GetCustomAttributes(false))
        {
        auto&& sourceCustomAttributeSchema = sourceCustomAttribute->GetClass().GetSchema();
        if (_GetContainerSchema()->GetSchemaKey().Matches(sourceCustomAttributeSchema.GetSchemaKey(), SchemaMatchType::Exact))
            {
            if (nullptr == destContainer.GetContainerSchema()->GetClassCP(sourceCustomAttribute->GetClass().GetName().c_str()))
                {
                if (copyReferences)
                    {
                    ECClassP ecClass;
                    status = destContainer.GetContainerSchema()->CopyClass(ecClass, sourceCustomAttribute->GetClass(), copyReferences);
                    if (ECObjectsStatus::Success != status)
                        return status;
                    }
                else if (!ECSchema::IsSchemaReferenced(*destContainer.GetContainerSchema(), sourceCustomAttributeSchema))
                    {
                    status = destContainer.GetContainerSchema()->AddReferencedSchema(const_cast<ECSchemaR>(sourceCustomAttributeSchema));
                    if (ECObjectsStatus::Success != status)
                        return status;
                    }
                }
            }
        else if (!ECSchema::IsSchemaReferenced(*destContainer.GetContainerSchema(), sourceCustomAttributeSchema))
            {
            status = destContainer.GetContainerSchema()->AddReferencedSchema(const_cast<ECSchemaR>(sourceCustomAttributeSchema));
            if (ECObjectsStatus::Success != status)
                return status;
            }

        auto&& destCustomAttribute = sourceCustomAttribute->CreateCopyThroughSerialization(*destContainer._GetContainerSchema());

        if (destCustomAttribute.IsNull())
            return ECObjectsStatus::Error;

        status = destContainer.SetCustomAttribute(*destCustomAttribute);
        if (ECObjectsStatus::Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP IECCustomAttributeContainer::GetContainerSchema()
    {
    return const_cast<ECSchemaP> (_GetContainerSchema());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::const_iterator ECCustomAttributeInstanceIterable::begin() const
    {
    return ECCustomAttributeInstanceIterable::const_iterator(m_container, m_includeBaseContainers);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::const_iterator ECCustomAttributeInstanceIterable::end () const
    {
    return ECCustomAttributeInstanceIterable::const_iterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::const_iterator& ECCustomAttributeInstanceIterable::const_iterator::operator++()
    {
    m_state->m_customAttributesIterator++;
    if (m_state->m_customAttributesIterator == m_state->m_customAttributes->end())
        m_isEnd = true;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ECCustomAttributeInstanceIterable::const_iterator::operator==(const_iterator const& rhs) const
    {
    return !(*this != rhs);
    }

static const IECInstancePtr s_nullInstancePtr;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr const& ECCustomAttributeInstanceIterable::const_iterator::operator*() const
    {
    if (m_isEnd)
        return s_nullInstancePtr;

    return *(m_state->m_customAttributesIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::IteratorState::IteratorState
(
IECCustomAttributeContainerCR container,
bool includeBase
)
    {
    m_customAttributes = new ECCustomAttributeCollection();
    for (IECInstancePtr ptr : container.m_customAttributes)
        m_customAttributes->push_back(ptr);

    if (includeBase)
        {
        bvector<IECCustomAttributeContainerP> baseContainers;
        container._GetBaseContainers(baseContainers);
        for (IECCustomAttributeContainerP baseContainer: baseContainers)
            {
            baseContainer->AddUniqueCustomAttributesToList(*m_customAttributes);
            }
        }
    m_customAttributesIterator = m_customAttributes->begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECCustomAttributeInstanceIterable::IteratorState::~IteratorState()
    {
    delete m_customAttributes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::IsDefined (Utf8StringCR className) const
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
    for (IECCustomAttributeContainerP container: baseContainers)
        {
        if (container->IsDefined(className))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeInternal
(
Utf8StringCR className,
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
    for (IECCustomAttributeContainerP container: baseContainers)
        {
        result = container->GetCustomAttribute(className);
        if (result.IsValid())
            return result;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttribute
(
Utf8StringCR className
) const
    {
    return GetCustomAttributeInternal (className, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECCustomAttributeContainer::GetCustomAttributeLocal(Utf8StringCR className) const
    {
    return GetCustomAttributeInternal(className, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECCustomAttributeContainer::RemoveCustomAttribute(Utf8StringCR className)
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

END_BENTLEY_ECOBJECT_NAMESPACE
