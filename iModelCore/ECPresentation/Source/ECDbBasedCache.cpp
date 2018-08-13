/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/ECDbBasedCache.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
        return lhs->GetPriority() < rhs->GetPriority()
            || lhs->GetPriority() == rhs->GetPriority() && lhs.get() < rhs.get();
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
template<typename TAppData>
struct PriorityAppData : BeSQLite::Db::AppData
{
protected:
    static BeSQLite::Db::AppData::Key s_key;
    mutable BeMutex m_mutex;
    bmap<int, bset<RefCountedPtr<ECDbClosedNotifier>>> m_priorityMap;

public:
    static TAppData* Get(ECDbCR db)
        {
        // NB: Use the version returning a raw pointer to work around circular dependency wherein destructors of
        // ECDbClosedNotifier's want to find this app data while it is in the process of being destroyed.
        BeSQLite::Db::AppData* appdata = db.FindRawAppData(s_key);
        return (nullptr != appdata) ? static_cast<TAppData*>(appdata) : nullptr;
        }

    static TAppData& GetOrCreate(ECDbCR db)
        {
        BeSQLite::Db::AppDataPtr appdata = db.FindAppData(s_key);
        if (appdata.IsValid())
            return static_cast<TAppData&>(*appdata);

        TAppData* thisAppData = new TAppData();
        db.AddAppData(s_key, thisAppData, TAppData::DeleteOnClearCache());
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
};
template<typename TAppData> BeSQLite::Db::AppData::Key PriorityAppData<TAppData>::s_key;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2017
+===============+===============+===============+===============+===============+======*/
struct CloseAppData : PriorityAppData<CloseAppData>
    {
    static bool DeleteOnClearCache() {return false;}
    ~CloseAppData()
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
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2017
+===============+===============+===============+===============+===============+======*/
struct ReloadAppData : PriorityAppData<ReloadAppData>
    {
    static bool DeleteOnClearCache() {return true;}
    ~ReloadAppData()
        {
        BeMutexHolder lock(m_mutex);

        // make sure each priority set is notified in correct order:
        // smallest priority notifiers are removed first, largest priority notifiers are removed last
        for (auto& prioritySet : m_priorityMap)
            {
            for (RefCountedPtr<ECDbClosedNotifier> const& notifier : prioritySet.second)
                notifier->GetListener().NotifyConnectionReloaded(notifier->GetConnection());
            }
        }
    };

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
        m_listener->NotifyConnectionClosed(m_db);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbClosedNotifier::Register(IECDbClosedListener& listener, BeSQLite::EC::ECDbCR db, bool deleteOnClearCache)
    {
    RefCountedPtr<ECDbClosedNotifier> closeNotifier = new ECDbClosedNotifier(listener, db);
    CloseAppData::GetOrCreate(db).Add(*closeNotifier);
    listener.m_notifiers.insert(closeNotifier.get());

    if (deleteOnClearCache)
        {
        RefCountedPtr<ECDbClosedNotifier> reloadNotifier = new ECDbClosedNotifier(listener, db);
        ReloadAppData::GetOrCreate(db).Add(*reloadNotifier);
        listener.m_notifiers.insert(reloadNotifier.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbClosedNotifier::Unregister()
    {
    RefCountedPtr<ECDbClosedNotifier> notifier = this;
    m_listener = nullptr;

    CloseAppData* closeAppData = CloseAppData::Get(m_db);
    if (nullptr != closeAppData)
        closeAppData->Remove(*this);

    ReloadAppData* reloadAppData = ReloadAppData::Get(m_db);
    if (nullptr != reloadAppData)
        reloadAppData->Remove(*this);
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
