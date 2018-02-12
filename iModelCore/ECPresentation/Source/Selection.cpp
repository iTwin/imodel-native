/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/Selection.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/SelectionManager.h>
#include <ECPresentation/IECPresentationManager.h>
#include "ECDbBasedCache.h"

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ISelectionManager::AddToSelection(ECDbCR ecdb, Utf8CP source, bool isSubSelection, NavNodeKeyCR key, RapidJsonValueCR extendedData)
    {
    AddToSelection(ecdb, source, isSubSelection, *KeySet::Create(key), extendedData);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ISelectionManager::RemoveFromSelection(ECDbCR ecdb, Utf8CP source, bool isSubSelection, NavNodeKeyCR key, RapidJsonValueCR extendedData)
    {
    RemoveFromSelection(ecdb, source, isSubSelection, *KeySet::Create(key), extendedData);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ISelectionManager::ChangeSelection(ECDbCR ecdb, Utf8CP source, bool isSubSelection, NavNodeKeyCR key, RapidJsonValueCR extendedData)
    {
    ChangeSelection(ecdb, source, isSubSelection, *KeySet::Create(key), extendedData);
    }

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                08/2016
//=======================================================================================
struct SelectionManager::SelectionStorage
{
private:
    NativeLogging::ILogger* m_logger;
    KeySetPtr m_keys;
    Utf8String m_lastSource;

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            08/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    SelectionStorage(NativeLogging::ILogger* logger) : m_logger(logger) {m_keys = KeySet::Create();}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            08/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    KeySetCPtr GetSelection() const {return m_keys;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            08/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    Utf8StringCR GetLastSelectionSource() const {return m_lastSource;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            08/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    bool AddToSelection(Utf8CP source, KeySetCR keys)
        {
        uint64_t inserted = m_keys->MergeWith(keys);
        if (0 != inserted)
            {
            if (nullptr != m_logger)
                m_logger->debugv("%s added %" PRIu64 " nodes to selection set", source, inserted);
            m_lastSource = source;
            return true;
            }
        return false;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            08/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    bool RemoveFromSelection(Utf8CP source, KeySetCR keys)
        {
        uint64_t removed = m_keys->Remove(keys);
        if (0 != removed)
            {
            if (nullptr != m_logger)
                m_logger->debugv("%s removed %" PRIu64 " nodes from selection set", source, removed);
            m_lastSource = source;
            return true;
            }
        return false;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            08/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    bool ChangeSelection(Utf8CP source, KeySetCR keys)
        {
        if (m_keys->Equals(keys))
            {
            // sets are equal
            return false;
            }

        m_keys->Clear();
        m_keys->MergeWith(keys);
        m_lastSource = source;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            08/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    bool ClearSelection(Utf8CP source)
        {
        if (m_keys->empty())
            return false;

        if (nullptr != m_logger)
            m_logger->debugv("%s cleared selection (was %" PRIu64 " nodes)", source, m_keys->size());

        m_keys->Clear();
        m_lastSource = source;
        return true;
        }
};

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                09/2016
//=======================================================================================
struct SelectionManager::ECDbSelection : ECDbBasedCache
{
private:
    SelectionManager const& m_manager;
    ECDbCR m_ecdb;
    SelectionStorage m_selection;
    SelectionStorage m_subSelection;

protected:
    void _ClearECDbCache(ECDbCR ecdb) override {m_manager.OnECDbClosed(m_ecdb);}

public:
    ECDbSelection(SelectionManager const& mgr, ECDbCR ecdb, NativeLogging::ILogger& logger) 
        : ECDbBasedCache(false), m_ecdb(ecdb), m_manager(mgr), m_selection(&logger), m_subSelection(nullptr)
        {
        OnConnection(ecdb);
        }
    SelectionStorage& GetSelection() {return m_selection;}
    SelectionStorage& GetSubSelection() {return m_subSelection;}
};

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionManager::~SelectionManager()
    {
    for (auto iter : m_selections)
        DELETE_AND_CLEAR(iter.second);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NativeLogging::ILogger& SelectionManager::GetLogger() const {return *NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_DGNCLIENTFX_SELECTION);}

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionManager::SelectionStorage& SelectionManager::GetStorage(IConnectionCR connection, bool isSubSelection) const
    {
    auto iter = m_selections.find(connection.GetId());
    if (m_selections.end() == iter)
        iter = m_selections.Insert(connection.GetId(), new ECDbSelection(*this, connection.GetDb(), GetLogger())).first;
    return isSubSelection ? iter->second->GetSubSelection() : iter->second->GetSelection();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionManager::OnECDbClosed(ECDbCR ecdb) const
    {
    IConnectionCPtr connection = m_connections.GetConnection(ecdb);
    if (connection.IsNull())
        {
        BeAssert(false);
        return;
        }
    
    BeMutexHolder lock(m_mutex);
    auto iter = m_selections.find(connection->GetId());
    if (m_selections.end() != iter)
        {
        m_selections.erase(iter);
        GetLogger().infov("Selection cleared due to connection close: '%s'", connection->GetId().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionManager::_AddListener(ISelectionChangesListener& listener) {m_listeners.push_back(&listener);}

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionManager::_RemoveListener(ISelectionChangesListener& listener) {m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), &listener));}

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void  SelectionManager::AddSyncHandler(SelectionSyncHandlerR handler)
    {
    m_syncHandlers.push_back(&handler);
    handler.OnRegistered(*this);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void  SelectionManager::RemoveSyncHandler(SelectionSyncHandlerR handler)
    {
    auto iter = std::remove(m_syncHandlers.begin(), m_syncHandlers.end(), &handler);
    if (m_syncHandlers.end() == iter)
        return;

    m_syncHandlers.erase(iter);
    handler.OnUnregistered(*this);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionManager::BroadcastSelectionChangedEvent(IConnectionCR connection, Utf8CP source, SelectionChangeType changeType, bool isSubSelection, KeySetCR keys, RapidJsonValueCR extendedData) const
    {
    // create the selection changed event
    SelectionChangedEvent evt(connection, source, changeType, isSubSelection, keys);
    evt.GetExtendedDataR().CopyFrom(extendedData, evt.GetExtendedDataAllocator());

    // notify listeners on the work thread
    bvector<ISelectionChangesListener*> listeners = m_listeners;
    for (ISelectionChangesListener* listener : listeners)
        listener->NotifySelectionChanged(evt);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionManager::_AddToSelection(ECDbCR ecdb, Utf8CP source, bool isSubSelection, KeySetCR keys, RapidJsonValueCR extendedData)
    {
    IConnectionPtr connection = m_connections.GetConnection(ecdb);
    if (connection.IsNull())
        {
        BeAssert(false);
        return;
        }
    
    BeMutexHolder lock(m_mutex);
    if (GetStorage(*connection, isSubSelection).AddToSelection(source, keys))
        {
        if (!isSubSelection)
            {
            GetStorage(*connection, true).ClearSelection(source);
            GetLogger().debug("Sub selection cleared due to main selection change");
            }
        lock.unlock();
        BroadcastSelectionChangedEvent(*connection, source, SelectionChangeType::Add, isSubSelection, keys, extendedData);
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionManager::_RemoveFromSelection(ECDbCR ecdb, Utf8CP source, bool isSubSelection, KeySetCR keys, RapidJsonValueCR extendedData)
    {
    IConnectionPtr connection = m_connections.GetConnection(ecdb);
    if (connection.IsNull())
        {
        BeAssert(false);
        return;
        }
    
    BeMutexHolder lock(m_mutex);
    if (GetStorage(*connection, isSubSelection).RemoveFromSelection(source, keys))
        {
        if (!isSubSelection)
            {
            GetStorage(*connection, true).ClearSelection(source);
            GetLogger().debug("Sub selection cleared due to main selection change");
            }
        lock.unlock();
        BroadcastSelectionChangedEvent(*connection, source, SelectionChangeType::Remove, isSubSelection, keys, extendedData);
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionManager::_ChangeSelection(ECDbCR ecdb, Utf8CP source, bool isSubSelection, KeySetCR keys, RapidJsonValueCR extendedData)
    {
    IConnectionPtr connection = m_connections.GetConnection(ecdb);
    if (connection.IsNull())
        {
        BeAssert(false);
        return;
        }
    
    BeMutexHolder lock(m_mutex);
    if (GetStorage(*connection, isSubSelection).ChangeSelection(source, keys))
        {
        if (!isSubSelection)
            {
            GetStorage(*connection, true).ClearSelection(source);
            GetLogger().debug("Sub selection cleared due to main selection change");
            }
        lock.unlock();
        BroadcastSelectionChangedEvent(*connection, source, SelectionChangeType::Replace, isSubSelection, keys, extendedData);
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionManager::_ClearSelection(ECDbCR ecdb, Utf8CP source, bool isSubSelection, RapidJsonValueCR extendedData)
    {
    IConnectionPtr connection = m_connections.GetConnection(ecdb);
    if (connection.IsNull())
        {
        BeAssert(false);
        return;
        }
    
    BeMutexHolder lock(m_mutex);
    if (!GetStorage(*connection, isSubSelection).ClearSelection(source))
        return;

    if (!isSubSelection)
        {
        GetStorage(*connection, true).ClearSelection(source);
        GetLogger().debug("Sub selection cleared due to main selection change");
        }
    
    lock.unlock();
    BroadcastSelectionChangedEvent(*connection, source, SelectionChangeType::Clear, isSubSelection, *KeySet::Create(), extendedData);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionManager::_RefreshSelection(ECDbCR ecdb, Utf8CP source, bool isSubSelection, RapidJsonValueCR extendedData)
    {
    IConnectionPtr connection = m_connections.GetConnection(ecdb);
    if (connection.IsNull())
        {
        BeAssert(false);
        return;
        }
    KeySetCPtr keys = isSubSelection ? GetSubSelection(ecdb) : GetSelection(ecdb);
    BroadcastSelectionChangedEvent(*connection, source, SelectionChangeType::Replace, isSubSelection, *keys, extendedData);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetCPtr SelectionManager::_GetSelection(ECDbCR ecdb) const
    {
    IConnectionPtr connection = m_connections.GetConnection(ecdb);
    if (connection.IsNull())
        {
        BeAssert(false);
        return KeySet::Create();
        }
    BeMutexHolder lock(m_mutex);
    return GetStorage(*connection, false).GetSelection();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetCPtr SelectionManager::_GetSubSelection(ECDbCR ecdb) const
    {
    IConnectionPtr connection = m_connections.GetConnection(ecdb);
    if (connection.IsNull())
        {
        BeAssert(false);
        return KeySet::Create();
        }
    BeMutexHolder lock(m_mutex);
    return GetStorage(*connection, true).GetSelection();
    }

const Utf8CP SelectionChangedEvent::JSON_MEMBER_Source = "Source";
const Utf8CP SelectionChangedEvent::JSON_MEMBER_ConnectionId = "ConnectionId";
const Utf8CP SelectionChangedEvent::JSON_MEMBER_IsSubSelection = "IsSubSelection";
const Utf8CP SelectionChangedEvent::JSON_MEMBER_ChangeType = "ChangeType";
const Utf8CP SelectionChangedEvent::JSON_MEMBER_Keys = "Keys";
const Utf8CP SelectionChangedEvent::JSON_MEMBER_ExtendedData = "ExtendedData";
/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document SelectionChangedEvent::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json;
    json.SetObject();

    if (nullptr != m_connection)
        json.AddMember(rapidjson::StringRef(JSON_MEMBER_ConnectionId), rapidjson::Value(m_connection->GetId().c_str(), json.GetAllocator()), json.GetAllocator());

    json.AddMember(rapidjson::StringRef(JSON_MEMBER_Source), rapidjson::StringRef(m_sourceName.c_str()), json.GetAllocator());
    json.AddMember(rapidjson::StringRef(JSON_MEMBER_IsSubSelection), m_isSubSelection, json.GetAllocator());
    json.AddMember(rapidjson::StringRef(JSON_MEMBER_ChangeType), (int)m_changeType, json.GetAllocator());
    json.AddMember(rapidjson::StringRef(JSON_MEMBER_Keys), m_keys->AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember(rapidjson::StringRef(JSON_MEMBER_ExtendedData), rapidjson::Value(GetExtendedData(), json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionChangedEvent::SelectionChangedEvent(IConnectionCacheCR connectionCache, JsonValueCR json)
    : m_connection(nullptr)
    {
    BeAssert(json.isMember(JSON_MEMBER_ConnectionId) && json[JSON_MEMBER_ConnectionId].isString());
    Utf8CP connectionId = json[JSON_MEMBER_ConnectionId].asCString();
    m_connection = connectionCache.GetConnection(connectionId);

    BeAssert(json.isMember(JSON_MEMBER_Source) && json[JSON_MEMBER_Source].isString());
    m_sourceName = json[JSON_MEMBER_Source].asCString();

    BeAssert(json.isMember(JSON_MEMBER_ChangeType) && json[JSON_MEMBER_ChangeType].isInt());
    m_changeType = (SelectionChangeType)json[JSON_MEMBER_ChangeType].asInt();

    BeAssert(json.isMember(JSON_MEMBER_IsSubSelection) && json[JSON_MEMBER_IsSubSelection].isBool());
    m_isSubSelection = json[JSON_MEMBER_IsSubSelection].asBool();

    NavNodeKeySet keys;
    if (json.isMember(JSON_MEMBER_Keys))
        {
        JsonValueCR keysJson = json[JSON_MEMBER_Keys];
        m_keys = KeySet::FromJson(keysJson);
        }

    if (json.isMember(JSON_MEMBER_ExtendedData) && json[JSON_MEMBER_ExtendedData].isObject() && !json[JSON_MEMBER_ExtendedData].empty())
        {
        Utf8String serializedExtendedData = Json::FastWriter().write(json[JSON_MEMBER_ExtendedData]);
        rapidjson::Document extendedData;
        extendedData.Parse(serializedExtendedData.c_str());
        GetExtendedDataR().CopyFrom(extendedData, GetExtendedDataAllocator());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    08/2016
//---------------------------------------------------------------------------------------
void SelectionSyncHandler::OnRegistered(SelectionManager& manager)
    {
    m_manager = &manager;
    m_manager->AddListener(*this);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    08/2016
//---------------------------------------------------------------------------------------
void SelectionSyncHandler::OnUnregistered(SelectionManager& manager)
    {
    BeAssert(m_manager == &manager);
    m_manager->RemoveListener(*this);
    m_manager = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    08/2016
//---------------------------------------------------------------------------------------
void SelectionSyncHandler::_OnSelectionChanged(SelectionChangedEventCR evt)
    {
    // can't use presentation-based unified selection without a presentation manager
    if (!IECPresentationManager::IsActive())
        return;

    // this handler only handles one-way synchronization to SelectionManager
    if (SyncDirection::Outside != _GetSyncDirection() && SyncDirection::Both != _GetSyncDirection())
        return;

    // return if the event notifies about subselection and this handler doesn't handle it
    if (evt.IsSubSelection() && !_HandlesSubSelection())
        return;

    // don't handle events fired by itself
    if (nullptr != _GetSelectionSourceName() && evt.GetSourceName().Equals(_GetSelectionSourceName()))
        return;

    // create content request options
    Json::Value contentOptions = _CreateContentOptionsForSelection(evt);
    Utf8CP contentDisplayType = _GetContentDisplayType();

    // create the selection info
    SelectionInfo selection(*m_manager, evt);
    KeySetCPtr inputKeys = evt.IsSubSelection() ? m_manager->GetSubSelection(evt.GetConnection().GetDb()) : m_manager->GetSelection(evt.GetConnection().GetDb());
    bvector<ECInstanceKey> selectedKeys;

    // get the default content descriptor
    ContentDescriptorCPtr defaultDescriptor = IECPresentationManager::GetManager().GetContentDescriptor(evt.GetConnection().GetDb(), contentDisplayType, *inputKeys, &selection, contentOptions).get();
    if (defaultDescriptor.IsNull())
        {
        _SelectInstances(evt, selectedKeys);
        return;
        }

    // we only care about keys, so ask to not return anything else
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*defaultDescriptor);
    descriptor->AddContentFlag(ContentFlags::KeysOnly);

    // request for content
    ContentCPtr content = IECPresentationManager::GetManager().GetContent(*descriptor, PageOptions()).get();
    if (content.IsNull())
        {
        _SelectInstances(evt, selectedKeys);
        return;
        }

    // create the list of selected instances and select it
    for (ContentSetItemCPtr const& record : content->GetContentSet())
        std::copy(record->GetKeys().begin(), record->GetKeys().end(), std::back_inserter(selectedKeys));
    _SelectInstances(evt, selectedKeys);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    08/2016
//---------------------------------------------------------------------------------------
void SelectionSyncHandler::AddToSelection(ECDbCR ecdb, bool isSubSelection, KeySetCR keys)
    {
    if (nullptr == m_manager)
        {
        BeAssert(false && "SelectionSyncHandler must be registered with the SelectionManager before calling AddToSelection");
        return;
        }

    BeAssert(nullptr != _GetSelectionSourceName() && 0 != *_GetSelectionSourceName());
    m_manager->AddToSelection(ecdb, _GetSelectionSourceName(), isSubSelection, keys, _CreateSelectionEventExtendedData());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    08/2016
//---------------------------------------------------------------------------------------
void SelectionSyncHandler::RemoveFromSelection(ECDbCR ecdb, bool isSubSelection, KeySetCR keys)
    {
    if (nullptr == m_manager)
        {
        BeAssert(false && "SelectionSyncHandler must be registered with the SelectionManager before calling RemoveFromSelection");
        return;
        }

    BeAssert(nullptr != _GetSelectionSourceName() && 0 != *_GetSelectionSourceName());
    m_manager->RemoveFromSelection(ecdb, _GetSelectionSourceName(), isSubSelection, keys, _CreateSelectionEventExtendedData());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    08/2016
//---------------------------------------------------------------------------------------
void SelectionSyncHandler::ChangeSelection(ECDbCR ecdb, bool isSubSelection, KeySetCR keys)
    {
    if (nullptr == m_manager)
        {
        BeAssert(false && "SelectionSyncHandler must be registered with the SelectionManager before calling ChangeSelection");
        return;
        }

    BeAssert(nullptr != _GetSelectionSourceName() && 0 != *_GetSelectionSourceName());
    m_manager->ChangeSelection(ecdb, _GetSelectionSourceName(), isSubSelection, keys, _CreateSelectionEventExtendedData());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    08/2016
//---------------------------------------------------------------------------------------
void SelectionSyncHandler::ClearSelection(ECDbCR ecdb, bool isSubSelection)
    {
    if (nullptr == m_manager)
        {
        BeAssert(false && "SelectionSyncHandler must be registered with the SelectionManager before calling ClearSelection");
        return;
        }

    BeAssert(nullptr != _GetSelectionSourceName() && 0 != *_GetSelectionSourceName());
    m_manager->ClearSelection(ecdb, _GetSelectionSourceName(), isSubSelection, _CreateSelectionEventExtendedData());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    09/2016
//---------------------------------------------------------------------------------------
void SelectionSyncHandler::HandleSelectionChangeEvent(SelectionChangedEventCR evt)
    {
    KeySetCPtr container = &evt.GetSelectedKeys();
    switch (evt.GetChangeType())
        {
        case SelectionChangeType::Add:
            AddToSelection(evt.GetDb(), evt.IsSubSelection(), *container);
            break;
        case SelectionChangeType::Remove:
            RemoveFromSelection(evt.GetDb(), evt.IsSubSelection(), *container);
            break;
        case SelectionChangeType::Replace:
            ChangeSelection(evt.GetDb(), evt.IsSubSelection(), *container);
            break;
        case SelectionChangeType::Clear:
            ClearSelection(evt.GetDb(), evt.IsSubSelection());
            break;
        default:
            BeAssert(false);
        }
    }
