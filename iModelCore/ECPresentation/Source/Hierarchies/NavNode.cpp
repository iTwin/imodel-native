/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/NavNode.h>
#include "../Shared/ValueHelpers.h"
#include "../Shared/ExtendedData.h"
#include "../Shared/Queries/PresentationQuery.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<Utf8String> CombineHashes(bvector<Utf8String> hashPath, Utf8String hash)
    {
    hashPath.push_back(hash);
    return hashPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddStringToHash(MD5& hash, Utf8StringCR str, bool useByteLength = true)
    {
    hash.Add(str.c_str(), useByteLength ? str.SizeInBytes() : str.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::FromJson(IConnectionCR connection, BeJsConst json)
    {
    return ECPresentationManager::GetSerializer().GetNavNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNodeKey::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNodeKey::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR NavNodeKey::GetHash() const
    {
    if (m_hashPath.empty())
        {
        static Utf8String const s_empty;
        return s_empty;
        }
    return m_hashPath.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int NavNodeKey::_Compare(NavNodeKey const& other) const
    {
    int typeCompareResult = m_type.compare(other.m_type);
    if (0 != typeCompareResult)
        return typeCompareResult;

    if (m_hashPath < other.m_hashPath)
        return -1;
    if (m_hashPath > other.m_hashPath)
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> NavNodeKey::CreateHashPath(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
    Utf8StringCR type, Utf8StringCR label, PresentationQueryCP instanceKeysSelectQuery)
    {
    MD5 h;
    AddStringToHash(h, specificationIdentifier, false);
    AddStringToHash(h, type);
    AddStringToHash(h, connectionIdentifier);
    if (!label.empty())
        AddStringToHash(h, label);
    if (instanceKeysSelectQuery && !instanceKeysSelectQuery->GetQueryString().empty())
        AddStringToHash(h, instanceKeysSelectQuery->GetQueryString());
    return CombineHashes(parentKey ? parentKey->GetHashPath() : bvector<Utf8String>(), h.GetHashString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECInstancesNodeKey::_IsSimilar(NavNodeKey const& other) const
    {
    if (!NavNodeKey::_IsSimilar(other))
        return false;

    ECInstancesNodeKey const* otherKey = dynamic_cast<ECInstancesNodeKey const*>(&other);
    return nullptr != otherKey && m_instanceKeys == otherKey->m_instanceKeys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECInstancesNodeKey::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> ECInstancesNodeKey::CreateHashPath(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
    bvector<ECClassInstanceKey> const& instanceKeys)
    {
    MD5 h;
    AddStringToHash(h, specificationIdentifier, false);
    AddStringToHash(h, NAVNODE_TYPE_ECInstancesNode);
    AddStringToHash(h, connectionIdentifier);
    for (auto const& classInstanceKey : instanceKeys)
        {
        ECInstanceKey instanceKey(classInstanceKey.GetClass()->GetId(), classInstanceKey.GetId());
        h.Add(&instanceKey, sizeof(instanceKey));
        }
    return CombineHashes(parentKey ? parentKey->GetHashPath() : bvector<Utf8String>(), h.GetHashString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECClassGroupingNodeKey> ECClassGroupingNodeKey::FromJson(IConnectionCR connection, BeJsConst json)
    {
    return ECPresentationManager::GetSerializer().GetECClassGroupingNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECClassGroupingNodeKey::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClassGroupingNodeKey::_IsSimilar(NavNodeKey const& other) const
    {
    return GroupingNodeKey::_IsSimilar(other)
        && m_class == static_cast<ECClassGroupingNodeKey const&>(other).m_class;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> ECClassGroupingNodeKey::CreateHashPath(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
    ECClassCR groupingClass, bool isPolymorphic, PresentationQueryCP instanceKeysSelectQuery)
    {
    MD5 h;
    AddStringToHash(h, specificationIdentifier, false);
    AddStringToHash(h, NAVNODE_TYPE_ECClassGroupingNode);
    AddStringToHash(h, connectionIdentifier);
    ECClassId classId = groupingClass.GetId();
    h.Add(&classId, sizeof(classId));
    if (isPolymorphic)
        h.Add(&isPolymorphic, sizeof(isPolymorphic));
    if (instanceKeysSelectQuery && !instanceKeysSelectQuery->GetQueryString().empty())
        AddStringToHash(h, instanceKeysSelectQuery->GetQueryString());
    return CombineHashes(parentKey ? parentKey->GetHashPath() : bvector<Utf8String>(), h.GetHashString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECPropertyGroupingNodeKey> ECPropertyGroupingNodeKey::FromJson(IConnectionCR connection, BeJsConst json)
    {
    return ECPresentationManager::GetSerializer().GetECPropertyGroupingNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECPropertyGroupingNodeKey::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPropertyGroupingNodeKey::_IsSimilar(NavNodeKey const& other) const
    {
    return GroupingNodeKey::_IsSimilar(other)
        && m_class == static_cast<ECPropertyGroupingNodeKey const&>(other).m_class
        && m_propertyName.Equals(static_cast<ECPropertyGroupingNodeKey const&>(other).m_propertyName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> ECPropertyGroupingNodeKey::CreateHashPath(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
    ECClassCR propertyClass, Utf8StringCR propertyName, RapidJsonValueCR groupedValuesJson, PresentationQueryCP instanceKeysSelectQuery)
    {
    MD5 h;
    AddStringToHash(h, specificationIdentifier, false);
    AddStringToHash(h, NAVNODE_TYPE_ECPropertyGroupingNode);
    AddStringToHash(h, connectionIdentifier);
    if (!groupedValuesJson.IsNull())
        AddStringToHash(h, BeRapidJsonUtilities::ToString(groupedValuesJson));
    ECClassId classId = propertyClass.GetId();
    h.Add(&classId, sizeof(classId));
    h.Add(propertyName.c_str(), propertyName.size());
    if (instanceKeysSelectQuery && !instanceKeysSelectQuery->GetQueryString().empty())
        AddStringToHash(h, instanceKeysSelectQuery->GetQueryString());
    return CombineHashes(parentKey ? parentKey->GetHashPath() : bvector<Utf8String>(), h.GetHashString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<LabelGroupingNodeKey> LabelGroupingNodeKey::FromJson(BeJsConst json)
    {
    return ECPresentationManager::GetSerializer().GetLabelGroupingNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document LabelGroupingNodeKey::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LabelGroupingNodeKey::_IsSimilar(NavNodeKey const& other) const
    {
    return GroupingNodeKey::_IsSimilar(other)
        && m_label.Equals(static_cast<LabelGroupingNodeKey const&>(other).m_label);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> LabelGroupingNodeKey::CreateHashPath(Utf8StringCR connectionIdentifier, Utf8StringCR specificationIdentifier, NavNodeKeyCP parentKey,
    Utf8StringCR label, PresentationQueryCP instanceKeysSelectQuery)
    {
    MD5 h;
    AddStringToHash(h, specificationIdentifier, false);
    AddStringToHash(h, NAVNODE_TYPE_DisplayLabelGroupingNode);
    AddStringToHash(h, connectionIdentifier);
    AddStringToHash(h, label);
    if (instanceKeysSelectQuery && !instanceKeysSelectQuery->GetQueryString().empty())
        AddStringToHash(h, instanceKeysSelectQuery->GetQueryString());
    return CombineHashes(parentKey ? parentKey->GetHashPath() : bvector<Utf8String>(), h.GetHashString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKey::NavNodeKey(NavNodeKey const& other)
    {
    m_type = other.m_type;
    m_specificationIdentifier = other.m_specificationIdentifier;
    m_hashPath = other.m_hashPath; 
    if (other.m_instanceKeysSelectQuery)
        m_instanceKeysSelectQuery = other.m_instanceKeysSelectQuery->Clone();
    }

#define NAVNODE_JSON_CHUNK_SIZE 256
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNode::NavNode()
    : m_allocator(NAVNODE_JSON_CHUNK_SIZE), m_internalExtendedData(&m_allocator)
    {
    m_internalExtendedData.SetObject();
    m_nodeId.Invalidate();
    m_determinedChildren = false;
    m_hasChildren = false;
    m_isChecked = false;
    m_isCheckboxVisible = false;
    m_isCheckboxEnabled = false;
    m_shouldAutoExpand = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNode::NavNode(NavNodeCR other)
    : m_allocator(NAVNODE_JSON_CHUNK_SIZE), m_internalExtendedData(&m_allocator)
    {
    m_internalExtendedData.CopyFrom(other.m_internalExtendedData, m_allocator);
    if (other.m_usersExtendedData != nullptr)
        InitUsersExtendedData(other.m_usersExtendedData.get());
    m_nodeId = other.m_nodeId;
    m_nodeKey = other.m_nodeKey;
    m_labelDefinition = other.m_labelDefinition;
    m_description = other.m_description;
    m_imageId = other.m_imageId;
    m_foreColor = other.m_foreColor;
    m_backColor = other.m_backColor;
    m_fontStyle = other.m_fontStyle;
    m_type = other.m_type;
    m_determinedChildren = other.m_determinedChildren;
    m_hasChildren = other.m_hasChildren;
    m_isChecked = other.m_isChecked;
    m_isCheckboxVisible = other.m_isCheckboxVisible;
    m_isCheckboxEnabled = other.m_isCheckboxEnabled;
    m_shouldAutoExpand = other.m_shouldAutoExpand;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNode::InitUsersExtendedData(rapidjson::Value const* source)
    {
    m_usersExtendedData = std::make_unique<rapidjson::Document>(&m_allocator);
    if (source)
        m_usersExtendedData->CopyFrom(*source, m_allocator);
    else
        m_usersExtendedData->SetObject();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNode::AddUsersExtendedData(Utf8CP key, ECValueCR value)
    {
    if (nullptr == m_usersExtendedData)
        InitUsersExtendedData();
    m_usersExtendedData->AddMember(rapidjson::Value(key, m_allocator), ValueHelpers::GetJsonFromECValue(value, &m_allocator), m_allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNode::HasChildren() const
    {
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, m_determinedChildren, "Returning 'has children' flag without having it determined");
    return m_hasChildren;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNode::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNode::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonAccessor NavNode::GetUsersExtendedData() const
    {
    if (!m_usersExtendedData)
        return RapidJsonAccessor();
    return RapidJsonAccessor(*m_usersExtendedData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NodesPathElement::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NodesPathElement::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeySetContainer::Create() {return new NavNodeKeySetContainer(NavNodeKeySet());}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeySetContainer::Create(NavNodeKeySet set) {return new NavNodeKeySetContainer(set);}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeySetContainer::Create(NavNodeKeySet const* set) {return new NavNodeKeySetContainer(set);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeyListContainer::Create() {return new NavNodeKeyListContainer(NavNodeKeyList());}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeyListContainer::Create(NavNodeKeyList list) {return new NavNodeKeyListContainer(list);}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeyListContainer::Create(NavNodeKeyList const* list) {return new NavNodeKeyListContainer(list);}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR INavNodeKeysContainer::GetHash() const
    {
    if (m_hash.empty())
        {
        MD5 h;
        for (NavNodeKeyCPtr const& key : *this)
            {
            for (Utf8StringCR partialPathHash : key->GetHashPath())
                h.Add(partialPathHash.c_str(), partialPathHash.SizeInBytes());
            }
        m_hash = h.GetHashString();
        }
    return m_hash;
    }
