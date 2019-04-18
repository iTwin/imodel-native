/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#include "NavigationBaseManager.h"

#include "../../Logging.h"
#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationBaseManager::NavigationBaseManager
(
ECDbAdapterR dbAdapter,
WebServices::ECSqlStatementCache& statementCache
) :
m_dbAdapter(dbAdapter),
m_statementCache(statementCache),
m_navigationBaseClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_NavigationBase))
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavigationBaseManager::IsNavigationBase(ECInstanceKeyCR instance)
    {
    return instance.GetClassId() == m_navigationBaseClass->GetId();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheNodeKey NavigationBaseManager::FindNavigationBase()
    {
    Utf8PrintfString key("NavigationBaseManager::FindNavigationBase");
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return "SELECT ECInstanceId FROM " ECSql_NavigationBase " LIMIT 1";
        });
    DbResult status = statement->Step();
    if (BE_SQLITE_ROW != status)
        {
        return CacheNodeKey();
        }
    return CacheNodeKey(m_navigationBaseClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheNodeKey NavigationBaseManager::FindOrCreateNavigationBase()
    {
    CacheNodeKey baseKey = FindNavigationBase();
    if (baseKey.IsValid())
        {
        return baseKey;
        }

    auto ecSql = "INSERT INTO " ECSql_NavigationBase " (ECInstanceId) VALUES (NULL)";
    ECSqlStatement statement;
    if (SUCCESS != m_dbAdapter.PrepareStatement(statement, ecSql))
        {
        return CacheNodeKey();
        }

    DbResult status = statement.Step(baseKey);
    if (BE_SQLITE_DONE != status)
        {
        return CacheNodeKey();
        }

    return baseKey;
    }
