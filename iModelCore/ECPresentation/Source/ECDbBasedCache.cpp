/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/ECDbBasedCache.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ECDbBasedCache.h"
#include <queue>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct NotifiersComparer
    {
    bool operator()(RefCountedPtr<ECDbClosedNotifier> const& lhs, RefCountedPtr<ECDbClosedNotifier> const& rhs) const
        {
        return lhs->GetPriority() < rhs->GetPriority();
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct PriorityAppData : BeSQLite::Db::AppData
{
private:
    static Key s_key;
    mutable BeMutex m_mutex;
    bmap<int, bset<RefCountedPtr<ECDbClosedNotifier>>> m_priorityMap;

public:
    ~PriorityAppData() {Clear();}

    static PriorityAppData* Get(ECDbCR db)
        {
        BeSQLite::Db::AppData* appdata = db.FindAppData(s_key);
        return (nullptr != appdata) ? static_cast<PriorityAppData*>(appdata) : nullptr;
        }

    static PriorityAppData& GetOrCreate(ECDbCR db, bool deleteOnClearCache)
        {
        BeSQLite::Db::AppData* appdata = db.FindAppData(s_key);
        if (nullptr != appdata)
            return static_cast<PriorityAppData&>(*appdata);

        PriorityAppData* thisAppData = new PriorityAppData();
        db.AddAppData(s_key, thisAppData, deleteOnClearCache);
        return *thisAppData;
        }

    void Add(ECDbClosedNotifier& notifier)
        {
        BeMutexHolder lock(m_mutex);
        int priority = notifier.GetPriority();
        m_priorityMap[priority].insert(&notifier);
        }

    void Remove(ECDbClosedNotifier& notifier)
        {
        BeMutexHolder lock(m_mutex);
        int priority = notifier.GetPriority();
        auto iter = m_priorityMap.find(priority);
        if (m_priorityMap.end() != iter)
            {
            RefCountedPtr<ECDbClosedNotifier> notifierPtr = &notifier;
            bset<RefCountedPtr<ECDbClosedNotifier>>& notifiers = iter->second;
            notifiers.erase(&notifier);
            lock.unlock();
            }
        }

    void Clear()
        {
        BeMutexHolder lock(m_mutex);
        bmap<int, bset<RefCountedPtr<ECDbClosedNotifier>>> copy = m_priorityMap;
        m_priorityMap.clear();
        lock.unlock();

        // make sure each priority set is cleared in correct order:
        // smallest priority notifiers are removed first, largest priority notifiers are removed last
        for (auto& prioritySet : copy)
            prioritySet.second.clear();
        }
};
BeSQLite::Db::AppData::Key PriorityAppData::s_key;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbClosedNotifier::ECDbClosedNotifier(IECDbClosedListener& listener, BeSQLite::EC::ECDbCR db) 
    : m_listener(&listener), m_db(db), m_priority(listener.GetPriority())
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbClosedNotifier::~ECDbClosedNotifier()
    {
    if (nullptr != m_listener)
        {
        m_listener->m_notifiers.erase(this);
        m_listener->_OnConnectionClosed(m_db);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ECDbClosedNotifier> ECDbClosedNotifier::Register(IECDbClosedListener& listener, BeSQLite::EC::ECDbCR db, bool deleteOnClearCache)
    {
    RefCountedPtr<ECDbClosedNotifier> notifier = new ECDbClosedNotifier(listener, db);
    PriorityAppData::GetOrCreate(db, deleteOnClearCache).Add(*notifier);
    listener.m_notifiers.insert(notifier.get());
    return notifier;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbClosedNotifier::Unregister()
    {
    m_listener = nullptr;
    PriorityAppData::Get(m_db)->Remove(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IECDbClosedListener::~IECDbClosedListener()
    {
    bset<ECDbClosedNotifier*> notifiers = m_notifiers;
    m_notifiers.clear();

    for (ECDbClosedNotifier* notifier : notifiers)
        notifier->Unregister();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbBasedCache::OnConnection(ECDbCR connection)
    {
    if (m_connections.end() != m_connections.find(&connection))
        return;

    m_connections.insert(&connection);
    ECDbClosedNotifier::Register(*this, connection, m_clearOnECDbCacheClear);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbBasedCache::_OnConnectionClosed(ECDbCR connection)
    {
    BeAssert(m_connections.end() != m_connections.find(&connection));
    m_connections.erase(&connection);
    _ClearECDbCache(connection);
    }
