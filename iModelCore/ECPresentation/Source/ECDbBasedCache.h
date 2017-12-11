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
    bset<ECDbClosedNotifier*> m_notifiers;

protected:
    ECPRESENTATION_EXPORT virtual ~IECDbClosedListener();
    virtual int _GetPriority() const {return 0;}
    virtual void _OnConnectionClosed(BeSQLite::EC::ECDbCR) = 0;

public:
    int GetPriority() const {return _GetPriority();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2016
+===============+===============+===============+===============+===============+======*/
struct ECDbClosedNotifier : RefCountedBase
{
private:
    int m_priority;
    IECDbClosedListener* m_listener;
    BeSQLite::EC::ECDbCR m_db;
    ECPRESENTATION_EXPORT ECDbClosedNotifier(IECDbClosedListener& listener, BeSQLite::EC::ECDbCR db);
public:
    ECPRESENTATION_EXPORT ~ECDbClosedNotifier();
    IECDbClosedListener& GetListener() const {return *m_listener;}
    BeSQLite::EC::ECDbCR GetConnection() {return m_db;}
    int GetPriority() const {return m_priority;}
    ECPRESENTATION_EXPORT static RefCountedPtr<ECDbClosedNotifier> Register(IECDbClosedListener&, BeSQLite::EC::ECDbCR, bool deleteOnClearCache);
    ECPRESENTATION_EXPORT void Unregister();
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
