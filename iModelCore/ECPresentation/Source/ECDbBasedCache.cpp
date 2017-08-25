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
