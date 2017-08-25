/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/ECDbBasedCache.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct ECDbClosedNotifier;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct IECDbClosedListener
{
    friend struct ECDbClosedNotifier;

private:
    BeSQLite::Db::AppData::Key m_key;
    bset<ECDbClosedNotifier*> m_notifiers;

protected:
    virtual ~IECDbClosedListener();
    virtual void _OnConnectionClosed(BeSQLite::EC::ECDbCR) = 0;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct ECDbClosedNotifier : BeSQLite::Db::AppData
{
private:
    IECDbClosedListener* m_listener;
    BeSQLite::EC::ECDbCR m_db;
    ECDbClosedNotifier(IECDbClosedListener& listener, BeSQLite::EC::ECDbCR db, bool deleteOnClearCache) 
        : m_listener(&listener), m_db(db)
        {
        db.AddAppData(m_listener->m_key, this, deleteOnClearCache);
        }

public:
    ~ECDbClosedNotifier()
        {
        if (nullptr != m_listener)
            {
            m_listener->_OnConnectionClosed(m_db);
            m_listener->m_notifiers.erase(this);
            }
        }
    BeSQLite::EC::ECDbCR GetConnection() {return m_db;}
    static ECDbClosedNotifier* Register(IECDbClosedListener& listener, BeSQLite::EC::ECDbCR db, bool deleteOnClearCache)
        {
        if (nullptr != db.FindAppData(listener.m_key))
            return nullptr;

        ECDbClosedNotifier* notifier = new ECDbClosedNotifier(listener, db, deleteOnClearCache);
        listener.m_notifiers.insert(notifier);
        return notifier;
        }
    void Unregister()
        {
        IECDbClosedListener* listener = m_listener;
        m_listener = nullptr;
        m_db.DropAppData(listener->m_key);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECDbBasedCache : IECDbClosedListener
{
private:
    bset<BeSQLite::EC::ECDb const*> m_connections;
    bool m_clearOnECDbCacheClear;

protected:
    ECDbBasedCache(bool clearOnECDbCacheClear) : m_clearOnECDbCacheClear(clearOnECDbCacheClear) {}
    ECPRESENTATION_EXPORT void _OnConnectionClosed(BeSQLite::EC::ECDbCR) override;
    virtual void _ClearECDbCache(BeSQLite::EC::ECDbCR) = 0;
    ECPRESENTATION_EXPORT void OnConnection(BeSQLite::EC::ECDbCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
