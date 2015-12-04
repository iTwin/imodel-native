/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderFromV9ToV10.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "UpgraderFromV9ToV10.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderFromV9ToV10::UpgraderFromV9ToV10(ECDbAdapter& adapter) :
    UpgraderBase(adapter)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV9ToV10::Upgrade()
    {
    if (SUCCESS != UpgradeCacheSchema(1, 7))
        {
        return ERROR;
        }

    // Read response infos
    ECSqlStatement statement;
    if (SUCCESS != m_adapter.PrepareStatement(statement,
        "SELECT GetECClassId(), ECInstanceId, [CacheDate], [CacheTag] FROM ONLY [DSC].[CachedResponseInfo]"))
        {
        return ERROR;
        }

    bvector<std::shared_ptr<Response>> responses;
    while (BE_SQLITE_ROW == statement.Step())
        {
        auto response = std::make_shared<Response>();

        response->key = ECInstanceKey(statement.GetValueId<ECClassId>(0), statement.GetValueId<ECInstanceId>(1));
        response->cacheDate = statement.GetValueDateTime(2);
        response->cacheTag = statement.GetValueText(3);

        responses.push_back(response);
        }

    // Get required classes
    auto responseToRelInfoClass = m_adapter.GetECRelationshipClass("DSCacheSchema", "CachedResponseInfoToCachedRelationshipInfo");
    auto responseToResultClass = m_adapter.GetECRelationshipClass("DSCacheSchema", "CachedResponseInfoToResultRelationship");
    auto responseToResultWeakClass = m_adapter.GetECRelationshipClass("DSCacheSchema", "CachedResponseInfoToResultWeakRelationship");

    auto responseToPageClass = m_adapter.GetECRelationshipClass("DSCacheSchema", "ResponseToResponsePage");

    auto pageToRelInfoClass = m_adapter.GetECRelationshipClass("DSCacheSchema", "ResponsePageToRelationshipInfo");
    auto pageToResultClass = m_adapter.GetECRelationshipClass("DSCacheSchema", "ResponsePageToResult");
    auto pageToResultWeakClass = m_adapter.GetECRelationshipClass("DSCacheSchema", "ResponsePageToResultWeak");

    // Create page for each cached response    
    statement.Finalize();
    if (SUCCESS != m_adapter.PrepareStatement(statement,
        "INSERT INTO [DSC].[CachedResponsePageInfo] ([Index], [CacheTag], [CacheDate]) VALUES (0,?,?)"))
        {
        return ERROR;
        }

    for (auto response : responses)
        {
        statement.Reset();

        statement.BindText(1, response->cacheTag.c_str(), IECSqlBinder::MakeCopy::No);
        statement.BindDateTime(2, response->cacheDate);

        ECInstanceKey pageKey;
        statement.Step(pageKey);
        if (!pageKey.IsValid())
            {
            return ERROR;
            }

        // Relate page to response
        if (!m_adapter.RelateInstances(responseToPageClass, response->key, pageKey).IsValid())
            {
            return ERROR;
            }

        // Relate results to page
        if (SUCCESS != RelateResponseResultToPage(response->key, pageKey, responseToRelInfoClass, pageToRelInfoClass) ||
            SUCCESS != RelateResponseResultToPage(response->key, pageKey, responseToResultClass, pageToResultClass) ||
            SUCCESS != RelateResponseResultToPage(response->key, pageKey, responseToResultWeakClass, pageToResultWeakClass))
            {
            return ERROR;
            }
        }
    
    statement.Finalize();

    // Remove deprecated data and set new IsCompleted
    if (SUCCESS != ExecuteStatement("UPDATE ONLY [DSC].[CachedResponseInfo] SET [CacheDate] = NULL, [CacheTag] = NULL, [IsCompleted] = TRUE") ||
        SUCCESS != ExecuteStatement("DELETE FROM ONLY [DSC].[CachedResponseInfoToCachedRelationshipInfo] WHERE TRUE") ||
        SUCCESS != ExecuteStatement("DELETE FROM ONLY [DSC].[CachedResponseInfoToResultRelationship] WHERE TRUE") ||
        SUCCESS != ExecuteStatement("DELETE FROM ONLY [DSC].[CachedResponseInfoToResultWeakRelationship] WHERE TRUE"))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV9ToV10::RelateResponseResultToPage
(
ECInstanceKeyCR responseKey,
ECInstanceKeyCR pageKey,
ECRelationshipClassCP responseRelClass,
ECRelationshipClassCP pageRelClass
)
    {
    if (!responseKey.IsValid() || !pageKey.IsValid() || responseRelClass == nullptr || pageRelClass == nullptr)
        {
        return ERROR;
        }

    ECInstanceKeyMultiMap responseResults;
    if (SUCCESS != m_adapter.GetRelatedTargetKeys(responseRelClass, responseKey, responseResults))
        {
        return ERROR;
        }

    for (auto& pair : responseResults)
        {
        ECInstanceKey resultKey (pair.first, pair.second);
        if (!m_adapter.RelateInstances(pageRelClass, pageKey, resultKey).IsValid())
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }