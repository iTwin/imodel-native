/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TestHelpers/TestConnectionCache.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECPresentationTest.h"
#include <ECPresentation/Connection.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct TestConnectionCache : IConnectionCache
{
    bmap<Utf8String, ECDb*> m_connections;
    ECDb* _GetConnection(Utf8CP connectionId) const override
        {
        auto iter = m_connections.find(connectionId);
        if (m_connections.end() == iter)
            return nullptr;
        return iter->second;
        }
    void Cache(ECDbR connection) {m_connections[connection.GetDbGuid().ToString()] = &connection;}
};

END_ECPRESENTATIONTESTS_NAMESPACE