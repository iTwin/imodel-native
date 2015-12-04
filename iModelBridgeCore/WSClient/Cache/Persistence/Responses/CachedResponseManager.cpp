/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Responses/CachedResponseManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachedResponseManager.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../../Util/StringHelper.h"
#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseManager::CachedResponseManager
(
ECDbAdapter& dbAdapter,
WebServices::ECSqlStatementCache& statementCache,
HierarchyManager& hierarchyManager,
RelationshipInfoManager& relationshipInfoManager,
ObjectInfoManager& objectInfoManager
) :
m_dbAdapter(&dbAdapter),
m_statementCache(&statementCache),
m_hierarchyManager(&hierarchyManager),
m_relationshipInfoManager(&relationshipInfoManager),
m_objectInfoManager(&objectInfoManager),

m_responseClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedResponseInfo)),
m_responsePageClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedResponsePageInfo)),

m_responseToParentClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponseInfoToParent)),
m_responseToHolderClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponseInfoToHolder)),
m_responseToResponsePageClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponseToResponsePage)),

m_responsePageToResultClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponsePageToResult)),
m_responsePageToResultWeakClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponsePageToResultWeak)),
m_responsePageToRelInfoClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponsePageToRelationshipInfo)),

m_responseInserter(dbAdapter.GetECDb(), *m_responseClass),
m_responseUpdater(dbAdapter.GetECDb(), *m_responseClass)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::OnBeforeDelete(ECN::ECClassCR ecClass, ECInstanceId ecInstanceId)
    {
    // TODO: can this be taken care of by ECDb with configuration that would tread referencing relationship as holding?
    if (&ecClass != m_responseToParentClass)
        {
        return SUCCESS;
        }

    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::_OnBeforeDelete", [=]
        {
        return
            "SELECT rel.SourceECClassId, rel.SourceECInstanceId "
            "FROM ONLY " ECSql_ResponseToParentClass " rel "
            "WHERE rel.ECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, ecInstanceId);
    statement->Step();

    ECInstanceKey responseInfoKey(statement->GetValueInt64(0), statement->GetValueId<ECInstanceId>(1));
    if (!responseInfoKey.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    m_responsesToDelete.insert(responseInfoKey);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::OnAfterDelete(bset<ECInstanceKey>& instancesToDeleteOut)
    {
    instancesToDeleteOut.insert(m_responsesToDelete.begin(), m_responsesToDelete.end());
    m_responsesToDelete.clear();
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseInfo CachedResponseManager::ReadInfo(CachedResponseKeyCR key)
    {
    ECClassCP parentClass = m_dbAdapter->GetECClass(key.GetParent());
    if (!key.GetParent().IsValid() || nullptr == parentClass)
        {
        return CachedResponseInfo();
        }

    Utf8PrintfString statementKey("CachedResponseManager::ReadInfo:%lld", key.GetParent().GetECClassId());
    auto statement = m_statementCache->GetPreparedStatement(statementKey, [=]
        {
        return 
            "SELECT response.* "
            "FROM ONLY " ECSql_CachedResponseInfoClass " response "
            "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*parentClass) + " parent "
            "USING " ECSql_ResponseToParentClass " FORWARD "
            "WHERE parent.ECInstanceId = ? AND response.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, key.GetParent().GetECInstanceId());
    statement->BindText(2, key.GetName().c_str(), IECSqlBinder::MakeCopy::No);

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return CachedResponseInfo(key.GetParent(), key.GetHolder(), key.GetName(), m_responseClass->GetId());
        }

    Json::Value infoJson;

    JsonECSqlSelectAdapter adapter(*statement, ECValueFormat::RawNativeValues);
    if (!adapter.GetRowInstance(infoJson, m_responseClass->GetId()))
        {
        return CachedResponseInfo();
        }

    return CachedResponseInfo(key.GetParent(), key.GetHolder(), infoJson, m_responseClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey CachedResponseManager::FindInfo(CachedResponseKeyCR responseKey)
    {
    ECClassCP parentClass = m_dbAdapter->GetECClass(responseKey.GetParent());
    if (!responseKey.IsValid() || nullptr == parentClass)
        {
        return ECInstanceKey();
        }

    Utf8PrintfString key("CachedResponseManager::FindInfo:%lld", responseKey.GetParent().GetECClassId());
    auto statement = m_statementCache->GetPreparedStatement(key, [=]
        {
        return
            "SELECT response.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponseInfoClass " response "
            "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*parentClass) + " parent "
            "USING " ECSql_ResponseToParentClass " FORWARD "
            "WHERE parent.ECInstanceId = ? AND response.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, responseKey.GetParent().GetECInstanceId());
    statement->BindText(2, responseKey.GetName().c_str(), IECSqlBinder::MakeCopy::No);

    DbResult status = statement->Step();
    if (status == BE_SQLITE_DONE)
        {
        // Nothing to delete
        return ECInstanceKey();
        }

    return ECInstanceKey(m_responseClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::DeleteInfo(CachedResponseKeyCR responseKey)
    {
    if (!responseKey.IsValid())
        {
        return ERROR;
        }

    ECInstanceKey infoKey = FindInfo(responseKey);
    if (!infoKey.IsValid())
        {
        // Nothing to delete
        return SUCCESS;
        };

    return m_hierarchyManager->DeleteInstance(infoKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::DeleteResponses(Utf8StringCR name, DateTimeCR accessedBeforeDateUtc, const ECInstanceKeyMultiMap& instancesToLeave)
    {
    if (accessedBeforeDateUtc.GetInfo().GetKind() != DateTime::Kind::Utc)
        {
        return ERROR;
        }

    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::DeleteResponsesByDate", [=]
        {
        return
            "SELECT ECInstanceId "
            "FROM ONLY " ECSql_CachedResponseInfoClass " "
            "WHERE [" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? AND "
            "     ([" CLASS_CachedResponseInfo_PROPERTY_AccessDate "] IS NULL OR "
            "      [" CLASS_CachedResponseInfo_PROPERTY_AccessDate "] < ?) ";
        });

    statement->BindText(1, name.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindDateTime(2, accessedBeforeDateUtc);

    bset<ECInstanceKey> responsesToDelete;
    while (BE_SQLITE_ROW == statement->Step())
        {
        ECInstanceKey responseKey(m_responseClass->GetId(), statement->GetValueId<ECInstanceId>(0));
        if (ECDbHelper::IsInstanceInMultiMap(responseKey, instancesToLeave))
            {
            continue;
            }
        responsesToDelete.insert(responseKey);
        }

    return m_hierarchyManager->DeleteInstances(responsesToDelete);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::DeleteResponses(Utf8StringCR name)
    {
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::DeleteResponses", [=]
        {
        return
            "SELECT GetECClassId(), ECInstanceId "
            "FROM ONLY " ECSql_CachedResponseInfoClass " "
            "WHERE [" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? ";
        });

    statement->BindText(1, name.c_str(), IECSqlBinder::MakeCopy::No);

    return m_hierarchyManager->DeleteInstances(*statement);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey CachedResponseManager::SavePage(CachedResponseInfoCR info, uint64_t page, Utf8StringCR cacheTag)
    {
    ECInstanceKey pageKey = FindPage(info, page);
    if (!pageKey.IsValid())
        {
        pageKey = InsertPage(info, page, cacheTag);
        }
    else
        {
        UpdatePage(pageKey.GetECInstanceId(), cacheTag);
        }
    return pageKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey CachedResponseManager::FindPage(CachedResponseInfoCR info, uint64_t page)
    {
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::FindPage", [&]
        {
        return
            "SELECT page.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponsePageInfoClass " page "
            "JOIN " ECSql_ResponseToResponsePageClass " rel ON rel.TargetECInstanceId = page.ECInstanceId "
            "WHERE rel.SourceECInstanceId = ? AND page.[" CLASS_CachedResponsePageInfo_PROPERTY_Index "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, info.GetKey().GetECInstanceId());
    statement->BindInt64(2, page);

    statement->Step();
    return ECInstanceKey(m_responsePageClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bvector<ECInstanceKey> CachedResponseManager::FindPages(ECInstanceKeyCR responseKey)
    {
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::FindPages", [&]
        {
        return
            "SELECT page.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponsePageInfoClass " page "
            "JOIN " ECSql_ResponseToResponsePageClass " rel ON rel.TargetECInstanceId = page.ECInstanceId "
            "WHERE rel.SourceECInstanceId = ? ";
        });

    statement->BindId(1, responseKey.GetECInstanceId());

    bvector<ECInstanceKey> pages;
    while (BE_SQLITE_ROW == statement->Step())
        {
        pages.push_back(ECInstanceKey(m_responsePageClass->GetId(), statement->GetValueId<ECInstanceId>(0)));
        }

    return pages;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey CachedResponseManager::InsertPage(CachedResponseInfoCR info, uint64_t page, Utf8StringCR cacheTag)
    {
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::InsertPage", [&]
        {
        return "INSERT INTO " ECSql_CachedResponsePageInfoClass " ("
            "[" CLASS_CachedResponsePageInfo_PROPERTY_Index "],"
            "[" CLASS_CachedResponsePageInfo_PROPERTY_CacheTag "],"
            "[" CLASS_CachedResponsePageInfo_PROPERTY_CacheDate "]"
            ") VALUES (?,?,?)";
        });

    statement->BindInt64(1, page);
    statement->BindText(2, cacheTag.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindDateTime(3, DateTime::GetCurrentTimeUtc());

    ECInstanceKey pageKey;
    statement->Step(pageKey);

    if (!m_hierarchyManager->RelateInstances(info.GetKey(), pageKey, m_responseToResponsePageClass).IsValid())
        {
        pageKey = ECInstanceKey();
        }

    return pageKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::UpdatePage(ECInstanceId pageId, Utf8StringCR cacheTag)
    {
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::UpdatePage", [&]
        {
        return
            "UPDATE ONLY " ECSql_CachedResponsePageInfoClass " "
            "SET " CLASS_CachedResponsePageInfo_PROPERTY_CacheTag " = ?, "
            "    " CLASS_CachedResponsePageInfo_PROPERTY_CacheDate " = ? "
            "WHERE ECInstanceId = ? ";
        });

    statement->BindText(1, cacheTag.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindDateTime(2, DateTime::GetCurrentTimeUtc());
    statement->BindId(3, pageId);

    if (BE_SQLITE_OK != statement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::UpdatePageCacheDate(ECInstanceId pageId)
    {
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::UpdatePageCacheDate", [&]
        {
        return
            "UPDATE ONLY " ECSql_CachedResponsePageInfoClass " SET " CLASS_CachedResponsePageInfo_PROPERTY_CacheDate " = ? "
            "WHERE ECInstanceId = ? ";
        });

    statement->BindDateTime(1, DateTime::GetCurrentTimeUtc());
    statement->BindId(2, pageId);

    if (BE_SQLITE_OK != statement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::ClearPageCacheTag(ECInstanceId pageId)
    {
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::ClearPageCacheTag", [&]
        {
        return
            "UPDATE ONLY " ECSql_CachedResponsePageInfoClass " SET " CLASS_CachedResponsePageInfo_PROPERTY_CacheTag " = NULL "
            "WHERE ECInstanceId = ? ";
        });

    statement->BindId(1, pageId);

    if (BE_SQLITE_OK != statement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::SaveInfo(CachedResponseInfoR info)
    {
    if (!info.IsCached())
        {
        return InsertInfo(info);
        }
    else
        {
        return m_responseUpdater.Get().Update(info.GetJsonData());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::InsertInfo(CachedResponseInfoR info)
    {
    if (info.IsCached())
        {
        return SUCCESS;
        }

    if (SUCCESS != m_responseInserter.Get().Insert(info.GetJsonData()) ||
        !m_hierarchyManager->RelateInstances(info.GetKey(), info.GetParent(), m_responseToParentClass).IsValid() ||
        !m_hierarchyManager->RelateInstances(info.GetKey(), info.GetHolder(), m_responseToHolderClass).IsValid())
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::UpdatePageCachedDate(CachedResponseKeyCR responseKey, uint64_t page)
    {
    auto info = ReadInfo(responseKey);
    ECInstanceKey pageKey = FindPage(info, page);
    return UpdatePageCacheDate(pageKey.GetECInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::SavePage
(
CachedResponseKeyCR responseKey,
uint64_t page,
Utf8StringCR cacheTag,
const InstanceCacheHelper::CachedInstances& instances
)
    {
    auto info = ReadInfo(responseKey);
    if (SUCCESS != InsertInfo(info))
        {
        return ERROR;
        }

    ECInstanceKey pageKey = SavePage(info, page, cacheTag);

    if (SUCCESS != RelateResultInstancesToPage(info.GetParent(), pageKey, instances) ||
        SUCCESS != RelateResultRelationshipInstancesToPage(pageKey, instances))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::RelateResultRelationshipInstancesToPage
(
ECInstanceKeyCR pageKey,
const InstanceCacheHelper::CachedInstances& instances
)
    {
    const bset<CachedRelationshipKey>& newCachedRelInfos = instances.GetCachedRelationshipInfos();
    bset<CachedRelationshipKey> oldCachedRelInfos;

    if (SUCCESS != m_relationshipInfoManager
        ->ReadCachedRelationshipsFromHolder(pageKey, m_responsePageToRelInfoClass, oldCachedRelInfos))
        {
        return ERROR;
        }

    bset<CachedRelationshipKey> outdatedRelInfos;
    bset<CachedRelationshipKey> newRelInfos;

    std::set_difference(oldCachedRelInfos.begin(), oldCachedRelInfos.end(),
        newCachedRelInfos.begin(), newCachedRelInfos.end(),
        std::inserter(outdatedRelInfos, outdatedRelInfos.end()));

    std::set_difference(newCachedRelInfos.begin(), newCachedRelInfos.end(),
        oldCachedRelInfos.begin(), oldCachedRelInfos.end(),
        std::inserter(newRelInfos, newRelInfos.end()));

    if (SUCCESS != m_relationshipInfoManager->RelateCachedRelationshipsToHolder(pageKey, m_responsePageToRelInfoClass, newRelInfos) ||
        SUCCESS != m_relationshipInfoManager->RemoveCachedRelationshipsFromHolder(pageKey, m_responsePageToRelInfoClass, outdatedRelInfos))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::RelateResultInstancesToPage
(
ECInstanceKeyCR responseParentKey,
ECInstanceKeyCR pageKey,
const InstanceCacheHelper::CachedInstances& instances
)
    {
    for (ECInstanceKeyCR resultInstance : instances.GetCachedInstances())
        {
        if (nullptr != m_dbAdapter->GetECRelationshipClass(resultInstance))
            {
            continue;
            }

        ECRelationshipClassCP resultRelClass = m_responsePageToResultClass;
        if (responseParentKey == resultInstance)
            {
            resultRelClass = m_responsePageToResultWeakClass;
            }

        if (!m_hierarchyManager->RelateInstances(pageKey, resultInstance, resultRelClass).IsValid())
            {
            return ERROR;
            }
        }

    // Remove old instances from query
    return m_hierarchyManager->ReleaseOldChildren(pageKey, instances.GetCachedInstances(), m_responsePageToResultClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::ReadResponse
(
CachedResponseInfoCR info,
ISelectProviderCR selectProvider,
const CacheQueryHelper::ReadCallback& readCallback
)
    {
    if (!info.IsCached())
        {
        return SUCCESS;
        }

    // Read result keys
    ECInstanceKeyMultiMap resultKeys;
    if (SUCCESS != ReadResponseInstanceKeys(info.GetKey(), resultKeys))
        {
        return ERROR;
        }

    CacheQueryHelper helper(selectProvider);
    return helper.ReadInstances(*m_dbAdapter, resultKeys, readCallback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::ReadResponseInstanceKeys(ECInstanceKeyCR responseKey, ECInstanceKeyMultiMap& keysOut)
    {
    for (auto& page : FindPages(responseKey))
        {
        if (SUCCESS != m_hierarchyManager->ReadTargetKeys(page, m_responsePageToResultClass, keysOut) ||
            SUCCESS != m_hierarchyManager->ReadTargetKeys(page, m_responsePageToResultWeakClass, keysOut))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::ReadResponseObjectIds
(
ECInstanceKeyCR responseKey,
bset<ObjectId>& objectIdsOut
)
    {
    for (auto& page : FindPages(responseKey))
        {
        if (SUCCESS != ReadPageObjectIds(page, m_responsePageToResultClass, objectIdsOut) ||
            SUCCESS != ReadPageObjectIds(page, m_responsePageToResultWeakClass, objectIdsOut))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::ReadPageObjectIds
(
ECInstanceKeyCR pageKey,
ECRelationshipClassCP relationshipClass,
bset<ObjectId>& objectIdsOut
)
    {
    Utf8PrintfString key("CachedResponseManager::ReadPageObjectIds:%lld", relationshipClass->GetId());
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        return CacheQueryHelper::ECSql::SelectRemoteIdsByRelatedSourceECInstanceId(*relationshipClass);
        });

    statement->BindId(1, pageKey.GetECInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement->Step()))
        {
        ECClassCP resultClass = m_dbAdapter->GetECClass(statement->GetValueInt64(0));
        if (nullptr == resultClass)
            {
            return ERROR;
            }

        ObjectId objectId
            {
            Utf8String(resultClass->GetSchema().GetName()),
            Utf8String(resultClass->GetName()),
            statement->GetValueText(1)
            };

        objectIdsOut.insert(objectId);
        }

    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CachedResponseManager::ReadResponsePageCacheTag(CachedResponseKeyCR responseKey, uint64_t page)
    {
    auto statement = GetSelectPagePropertyStatement(responseKey, page, CLASS_CachedResponsePageInfo_PROPERTY_CacheTag);
    statement->Step();
    return statement->GetValueText(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
DateTime CachedResponseManager::ReadResponsePageCachedDate(CachedResponseKeyCR responseKey, uint64_t page)
    {
    auto statement = GetSelectPagePropertyStatement(responseKey, page, CLASS_CachedResponsePageInfo_PROPERTY_CacheDate);
    if (BE_SQLITE_ROW != statement->Step())
        {
        return DateTime();
        }
    return statement->GetValueDateTime(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECSqlStatementPtr CachedResponseManager::GetSelectPagePropertyStatement
(
CachedResponseKeyCR responseKey,
uint64_t page,
Utf8StringCR propertyName
)
    {
    auto key = "CachedResponseManager::GetSelectPagePropertyStatement:" + propertyName;
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        return
            "SELECT page.[" + propertyName + "] "
            "FROM ONLY " ECSql_CachedResponsePageInfoClass " page "
            "JOIN " ECSql_ResponseToResponsePageClass " rel1 "
            "  ON rel1.TargetECInstanceId = page.ECInstanceId "
            "JOIN " ECSql_CachedResponseInfoClass " info "
            "  ON rel1.SourceECInstanceId = info.ECInstanceId "
            " AND info.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? "
            "JOIN " ECSql_ResponseToParentClass " rel2 "
            "  ON rel2.SourceECInstanceId = rel1.SourceECInstanceId "
            "WHERE rel2.TargetECInstanceId = ? "
            "  AND page.[" CLASS_CachedResponsePageInfo_PROPERTY_Index "] = ? "
            "LIMIT 1 ";
        });

    statement->BindText(1, responseKey.GetName().c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindId(2, responseKey.GetParent().GetECInstanceId());
    statement->BindInt64(3, page);

    return statement;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::TrimPages(CachedResponseKeyCR responseKey, uint64_t maxPageIndex)
    {
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::TrimPages", [&]
        {
        return
            "SELECT page.GetECClassId(), page.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponsePageInfoClass " page "
            "JOIN " ECSql_ResponseToResponsePageClass " rel1 "
            "  ON rel1.TargetECInstanceId = page.ECInstanceId "
            "JOIN " ECSql_CachedResponseInfoClass " info "
            "  ON rel1.SourceECInstanceId = info.ECInstanceId "
            " AND info.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? "
            "JOIN " ECSql_ResponseToParentClass " rel2 "
            "  ON rel2.SourceECInstanceId = rel1.SourceECInstanceId "
            "WHERE rel2.TargetECInstanceId = ? "
            "  AND page.[" CLASS_CachedResponsePageInfo_PROPERTY_Index "] > ? ";
        });

    statement->BindText(1, responseKey.GetName().c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindId(2, responseKey.GetParent().GetECInstanceId());
    statement->BindInt64(3, maxPageIndex);

    return m_hierarchyManager->DeleteInstances(*statement);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::SetResponseCompleted(CachedResponseKeyCR responseKey, bool isCompleted)
    {
    ECInstanceKey infoKey = FindInfo(responseKey);
    if (!infoKey.IsValid())
        {
        return ERROR;
        }

    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::SetResponseCompleted", [&]
        {
        return
            "UPDATE ONLY " ECSql_CachedResponseInfoClass " SET " CLASS_CachedResponseInfo_PROPERTY_IsCompleted " = ? "
            "WHERE ECInstanceId = ? ";
        });

    statement->BindBoolean(1, isCompleted);
    statement->BindId(2, infoKey.GetECInstanceId());

    if (BE_SQLITE_ROW != statement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CachedResponseManager::IsResponseCompleted(CachedResponseKeyCR responseKey)
    {
    if (!responseKey.IsValid())
        {
        return false;
        }

    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::IsResponseCompleted", [=]
        {
        return
            "SELECT info.[" CLASS_CachedResponseInfo_PROPERTY_IsCompleted "] "
            "FROM ONLY " ECSql_CachedResponseInfoClass " info "
            "JOIN ONLY " ECSql_ResponseToParentClass " rel ON rel.SourceECInstanceId = info.ECInstanceId "
            "WHERE rel.TargetECInstanceId = ? AND info.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, responseKey.GetParent().GetECInstanceId());
    statement->BindText(2, responseKey.GetName().c_str(), IECSqlBinder::MakeCopy::No);

    statement->Step();
    return statement->GetValueBoolean(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::MarkTemporaryInstancesAsPartial
(
const bset<ECInstanceId>& responseIds,
const ECInstanceKeyMultiMap& fullyPersistedInstances
)
    {
    if (SUCCESS != MarkTemporaryInstancesAsPartial(responseIds, m_responsePageToResultClass, fullyPersistedInstances) ||
        SUCCESS != MarkTemporaryInstancesAsPartial(responseIds, m_responsePageToResultWeakClass, fullyPersistedInstances))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::MarkTemporaryInstancesAsPartial
(
const bset<ECInstanceId>& responseIds,
ECRelationshipClassCP resultRelationshipClass,
const ECInstanceKeyMultiMap& fullyPersistedInstances
)
    {
    ECClassCP instanceInfoClass = m_objectInfoManager->GetInfoClass();
    ECClassCP instanceInfoRelationshipClass = m_objectInfoManager->GetInfoRelationshipClass();

    Utf8String ecsql =
        "SELECT infoRel.TargetECClassId, infoRel.TargetECInstanceId, info.* "
        "FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*instanceInfoClass) + " info "
        "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*instanceInfoRelationshipClass) + " infoRel ON infoRel.SourceECInstanceId = info.ECInstanceId "
        "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*resultRelationshipClass) + " pageToResultRel ON pageToResultRel.TargetECInstanceId = infoRel.TargetECInstanceId "
        "JOIN ONLY " + ECSql_ResponseToResponsePageClass + " responseToPageRel ON responseToPageRel.TargetECInstanceId = pageToResultRel.SourceECInstanceId "
        "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceState "] = ? "
        "  AND responseToPageRel.SourceECInstanceId IN (" + StringHelper::Join(responseIds.begin(), responseIds.end(), ',') + ")";

    ECSqlStatement statement;
    if (SUCCESS != m_dbAdapter->PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }
    statement.BindInt(1, static_cast<int> (CachedInstanceState::Full));

    JsonECSqlSelectAdapter adapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        ECInstanceKey instanceKey(statement.GetValueInt64(0), statement.GetValueId<ECInstanceId>(1));

        if (ECDbHelper::IsInstanceInMultiMap(instanceKey, fullyPersistedInstances))
            {
            // Not temporary
            continue;
            }

        Json::Value instanceInfoJson;
        adapter.GetRowInstance(instanceInfoJson, instanceInfoClass->GetId());
        ECClassCP instanceClass = m_dbAdapter->GetECClass(instanceKey);
        ObjectInfo instanceInfo(instanceInfoJson, instanceClass, m_objectInfoManager->GetInfoClass()->GetId());

        instanceInfo.SetObjectState(CachedInstanceState::Partial);

        if (SUCCESS != m_objectInfoManager->UpdateInfo(instanceInfo))
            {
            return ERROR;
            }
        }

    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::InvalidateResponsePagesContainingInstance(ECInstanceKeyCR instanceKey)
    {
    bvector<ECInstanceId> pageIds;

    // Result relationship
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::InvalidateResponsePagesContainingInstance:Rel", [&]
        {
        return
            "SELECT page.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponsePageInfoClass " page "
            "JOIN " ECSql_ResponsePageToResultClass " rel ON page.ECInstanceId = rel.SourceECInstanceId "
            "WHERE rel.TargetECClassId = ? AND rel.TargetECInstanceId = ? ";
        });

    statement->BindInt64(1, instanceKey.GetECClassId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    if (SUCCESS != m_dbAdapter->ExtractECIdsFromStatement(*statement, 0, pageIds))
        {
        return ERROR;
        }

    // Result weak relationship
    statement = m_statementCache->GetPreparedStatement("CachedResponseManager::InvalidateResponsePagesContainingInstance:Weak", [&]
        {
        return
            "SELECT page.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponsePageInfoClass " page "
            "JOIN " ECSql_ResponsePageToResultWeakClass " rel ON page.ECInstanceId = rel.SourceECInstanceId "
            "WHERE rel.TargetECClassId = ? AND rel.TargetECInstanceId = ? ";
        });

    statement->BindInt64(1, instanceKey.GetECClassId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    if (SUCCESS != m_dbAdapter->ExtractECIdsFromStatement(*statement, 0, pageIds))
        {
        return ERROR;
        }

    // Invalidate
    for (auto& pageId : pageIds)
        {
        if (SUCCESS != ClearPageCacheTag(pageId))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }
