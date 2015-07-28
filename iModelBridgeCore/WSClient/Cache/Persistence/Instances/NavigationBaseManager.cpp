/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Instances/NavigationBaseManager.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
ECSqlStatementCache& statementCache
) :
m_dbAdapter(&dbAdapter),
m_statementCache(&statementCache),
m_navigationBaseClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_NavigationBase))
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey NavigationBaseManager::FindNavigationBase()
    {
    Utf8PrintfString key("NavigationBaseManager::FindNavigationBase");
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        return "SELECT ECInstanceId FROM " ECSql_NavigationBaseClass " LIMIT 1";
        });
    ECSqlStepStatus status = statement->Step();
    if (ECSqlStepStatus::HasRow != status)
        {
        return ECInstanceKey();
        }
    return ECInstanceKey(m_navigationBaseClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey NavigationBaseManager::FindOrCreateNavigationBase()
    {
    ECInstanceKey instanceKey = FindNavigationBase();
    if (instanceKey.IsValid())
        {
        return instanceKey;
        }

    auto ecSql = "INSERT INTO " ECSql_NavigationBaseClass " (ECInstanceId) VALUES (NULL)";
    ECSqlStatement statement;
    if (SUCCESS != m_dbAdapter->PrepareStatement(statement, ecSql))
        {
        return ECInstanceKey();
        }

    ECSqlStepStatus status = statement.Step(instanceKey);
    if (ECSqlStepStatus::Done != status)
        {
        return ECInstanceKey();
        }

    return instanceKey;
    }
