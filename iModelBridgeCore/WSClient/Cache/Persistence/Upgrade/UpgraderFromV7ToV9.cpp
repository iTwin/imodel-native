/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderFromV7ToV9.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "UpgraderFromV7ToV9.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderFromV7ToV9::UpgraderFromV7ToV9(ECDbAdapter& adapter) :
UpgraderBase(adapter)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV7ToV9::Upgrade()
    {
    if (SUCCESS != UpgradeCacheSchema(1, 6) ||
        SUCCESS != ExecuteStatement("UPDATE ONLY [DSC].[CachedObjectInfo]       SET [IsLocal] = TRUE WHERE [ChangeStatus] = 1") ||
        SUCCESS != ExecuteStatement("UPDATE ONLY [DSC].[CachedRelationshipInfo] SET [IsLocal] = TRUE WHERE [ChangeStatus] = 1"))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV7ToV9::ExecuteStatement(Utf8CP ecSql)
    {
    ECSqlStatement statement;
    if (SUCCESS != m_adapter.PrepareStatement(statement, ecSql))
        {
        return ERROR;
        }

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()));

    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }