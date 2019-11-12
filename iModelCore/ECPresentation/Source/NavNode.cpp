/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/NavNode.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::FromJson(IConnectionCR connection, RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetNavNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::FromJson(IConnectionCR connection, JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetNavNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::Create(JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetBaseNavNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr NavNodeKey::Create(RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetBaseNavNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNodeKey::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 NavNodeKey::_ComputeHash() const
    {
    MD5 h;
    h.Add(m_type.c_str(), m_type.SizeInBytes());
    for (Utf8StringCR pathElement : m_pathFromRoot)
        h.Add(pathElement.c_str(), pathElement.SizeInBytes());
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR NavNodeKey::GetHash() const
    {
    if (m_keyHash.empty())
        m_keyHash = _ComputeHash().GetHashString();
    return m_keyHash;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
int NavNodeKey::_Compare(NavNodeKey const& other) const
    {
    int typeCompareResult = m_type.compare(other.m_type);
    if (0 != typeCompareResult)
        return typeCompareResult;

    if (m_pathFromRoot < other.m_pathFromRoot)
        return -1;
    if (m_pathFromRoot > other.m_pathFromRoot)
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECInstanceNodeKey> ECInstanceNodeKey::Create(IConnectionCR connection, JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECInstanceNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECInstanceNodeKey> ECInstanceNodeKey::Create(IConnectionCR connection, RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECInstanceNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECInstanceNodeKey::_IsSimilar(NavNodeKey const& other) const
    {
    if (!NavNodeKey::_IsSimilar(other))
        return false;

    BeAssert(nullptr != dynamic_cast<ECInstanceNodeKey const*>(&other));
    ECInstanceNodeKey const& otherKey = static_cast<ECInstanceNodeKey const&>(other);
    return m_instanceKey == otherKey.m_instanceKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECInstanceNodeKey::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ECInstanceNodeKey::_ComputeHash() const
    {
    MD5 h = NavNodeKey::_ComputeHash();
    h.Add(&m_instanceKey, sizeof(ECInstanceKey));
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECClassGroupingNodeKey> ECClassGroupingNodeKey::Create(IConnectionCR connection, JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECClassGroupingNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECClassGroupingNodeKey> ECClassGroupingNodeKey::Create(IConnectionCR connection, RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECClassGroupingNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECClassGroupingNodeKey::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ECClassGroupingNodeKey::_ComputeHash() const
    {
    MD5 h = NavNodeKey::_ComputeHash();
    h.Add(m_class->GetFullName(), strlen(m_class->GetFullName()));
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECPropertyGroupingNodeKey> ECPropertyGroupingNodeKey::Create(IConnectionCR connection, JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECPropertyGroupingNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECPropertyGroupingNodeKey> ECPropertyGroupingNodeKey::Create(IConnectionCR connection, RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetECPropertyGroupingNodeKeyFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECPropertyGroupingNodeKey::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ECPropertyGroupingNodeKey::_ComputeHash() const
    {
    MD5 h = NavNodeKey::_ComputeHash();
    h.Add(m_class->GetFullName(), strlen(m_class->GetFullName()));
    h.Add(m_propertyName.c_str(), m_propertyName.SizeInBytes());
    // wip: do we also absolutely need grouping value?
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<LabelGroupingNodeKey> LabelGroupingNodeKey::Create(JsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetLabelGroupingNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<LabelGroupingNodeKey> LabelGroupingNodeKey::Create(RapidJsonValueCR json)
    {
    return IECPresentationManager::GetSerializer().GetLabelGroupingNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document LabelGroupingNodeKey::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 LabelGroupingNodeKey::_ComputeHash() const
    {
    MD5 h = NavNodeKey::_ComputeHash();
    h.Add(m_label.c_str(), m_label.SizeInBytes());
    return h;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NavNode::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonAccessor NavNode::GetUsersExtendedData() const
    {
    rapidjson::Value const* ptr = _GetUsersExtendedData();
    if (!ptr)
        return RapidJsonAccessor();
    return RapidJsonAccessor(*ptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document NodesPathElement::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeySetContainer::Create() {return new NavNodeKeySetContainer(NavNodeKeySet());}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeySetContainer::Create(NavNodeKeySet set) {return new NavNodeKeySetContainer(set);}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeySetContainer::Create(NavNodeKeySet const* set) {return new NavNodeKeySetContainer(set);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeyListContainer::Create() {return new NavNodeKeyListContainer(NavNodeKeyList());}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeyListContainer::Create(NavNodeKeyList list) {return new NavNodeKeyListContainer(list);}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodeKeysContainerCPtr NavNodeKeyListContainer::Create(NavNodeKeyList const* list) {return new NavNodeKeyListContainer(list);}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR INavNodeKeysContainer::GetHash() const
    {
    if (m_hash.empty())
        {
        MD5 h;
        for (NavNodeKeyCPtr const& key : *this)
            h.Add(key->GetHash().c_str(), key->GetHash().SizeInBytes());
        m_hash = h.GetHashString();
        }
    return m_hash;
    }
