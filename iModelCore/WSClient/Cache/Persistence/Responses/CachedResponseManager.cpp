/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Responses/CachedResponseManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
m_dbAdapter(dbAdapter),
m_statementCache(statementCache),
m_hierarchyManager(hierarchyManager),
m_relationshipInfoManager(relationshipInfoManager),
m_objectInfoManager(objectInfoManager),

m_responseClass(m_dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedResponseInfo)),
m_responsePageClass(m_dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedResponsePageInfo)),

m_responseToParentClass(m_dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponseToParent)),
m_responseToHolderClass(m_dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponseToHolder)),
m_responseToResponsePageClass(m_dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponseToResponsePage)),
m_responseToAdditionalInstanceClass(m_dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponseToAdditionalInstance)),

m_responsePageToResultClass(m_dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponsePageToResult)),
m_responsePageToResultWeakClass(m_dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ResponsePageToResultWeak)),

m_responseInserter(m_dbAdapter.GetECDb(), *m_responseClass),
m_responseUpdater(m_dbAdapter.GetECDb(), *m_responseClass)
    {
    dbAdapter.RegisterDeleteListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ResponseKey CachedResponseManager::ConvertResponseKey(CachedResponseKeyCR key)
    {
    Utf8StringCR name = key.GetName();
    CacheNodeKey parent = GetCacheNodeKey(key.GetParent());
    CacheNodeKey holder = parent;

    if (key.GetParent() != key.GetHolder())
        {
        holder = GetCacheNodeKey(key.GetHolder());
        }

    return ResponseKey(parent, holder, name);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CacheNodeKey CachedResponseManager::GetCacheNodeKey(ECInstanceKeyCR instanceKey)
    {
    // WIP06
    ECClassCP ecClass = m_dbAdapter.GetECClass(instanceKey);
    if (nullptr == ecClass)
        {
        return CacheNodeKey();
        }

    ECSchemaId cacheSchemaId = m_responseClass->GetSchema().GetId();
    if (cacheSchemaId == ecClass->GetSchema().GetId())
        {
        return CacheNodeKey(instanceKey.GetECClassId(), instanceKey.GetECInstanceId());
        }

    return m_objectInfoManager.ReadInfoKey(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::OnBeforeDelete(ECN::ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalInstancesOut)
    {
    // TODO: can this be taken care of by ECDb with configuration that would tread referencing relationship as holding?
    if (&ecClass != m_responseToParentClass)
        {
        return SUCCESS;
        }

    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::_OnBeforeDelete", [=]
        {
        return
            "SELECT rel.SourceECClassId, rel.SourceECInstanceId "
            "FROM ONLY " ECSql_ResponseToParent " rel "
            "WHERE rel.ECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, ecInstanceId);
    statement->Step();

    ECInstanceKey responseInfoKey(statement->GetValueId<ECClassId>(0), statement->GetValueId<ECInstanceId>(1));
    if (!responseInfoKey.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    additionalInstancesOut.insert(responseInfoKey);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseInfo CachedResponseManager::ReadInfo(ResponseKeyCR key)
    {
    if (!key.IsValid())
        {
        return CachedResponseInfo();
        }

    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::ReadInfo", [=]
        {
        return
            "SELECT response.* "
            "FROM ONLY " ECSql_CachedResponseInfo " response "
            "JOIN ONLY " ECSql_ResponseToParent " rel ON rel.SourceECInstanceId = response.ECInstanceId "
            "WHERE rel.TargetECInstanceId = ? AND response.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, key.GetParent().GetECInstanceId());
    statement->BindText(2, key.GetName().c_str(), IECSqlBinder::MakeCopy::No);

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return CachedResponseInfo(key, m_responseClass->GetId());
        }

    Json::Value infoJson;

    JsonECSqlSelectAdapter adapter(*statement, ECValueFormat::RawNativeValues);
    if (!adapter.GetRowInstance(infoJson, m_responseClass->GetId()))
        {
        return CachedResponseInfo();
        }

    return CachedResponseInfo(key, infoJson, m_responseClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CacheNodeKey CachedResponseManager::FindInfo(ResponseKeyCR key)
    {
    if (!key.IsValid())
        {
        return CacheNodeKey();
        }

    CacheNodeKey parent = GetCacheNodeKey(key.GetParent());
    Utf8StringCR name = key.GetName();

    if (!parent.IsValid())
        {
        return CacheNodeKey();
        }

    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::FindInfo", [=]
        {
        return
            "SELECT response.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponseInfo " response "
            "JOIN ONLY " ECSql_ResponseToParent " rel ON rel.SourceECInstanceId = response.ECInstanceId "
            "WHERE rel.TargetECInstanceId = ? AND response.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, parent.GetECInstanceId());
    statement->BindText(2, name.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult status = statement->Step();
    if (status == BE_SQLITE_DONE)
        {
        // Nothing to delete
        return CacheNodeKey();
        }

    return CacheNodeKey(m_responseClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKeyMultiMap CachedResponseManager::FindInfosContainingInstance(CachedObjectInfoKeyCR instanceInfoKey)
    {
    ECInstanceKeyMultiMap instances;

    for (auto page : FindPagesContainingInstance(instanceInfoKey))
        {
        if (SUCCESS != m_hierarchyManager.ReadSourceKeys(page, m_responseToResponsePageClass, instances))
            {
            BeAssert(false);
            return ECInstanceKeyMultiMap();
            }
        }

    if (SUCCESS != m_hierarchyManager.ReadSourceKeys(instanceInfoKey, m_responseToAdditionalInstanceClass, instances))
        {
        BeAssert(false);
        return ECInstanceKeyMultiMap();
        }

    return instances;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::DeleteInfo(ResponseKeyCR responseKey)
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

    return m_hierarchyManager.DeleteInstance(infoKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::DeleteResponses(Utf8StringCR name, DateTimeCR accessedBeforeDateUtc, const ECInstanceKeyMultiMap& nodesToLeave)
    {
    if (accessedBeforeDateUtc.GetInfo().GetKind() != DateTime::Kind::Utc)
        {
        return ERROR;
        }

    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::DeleteResponsesByDate", [=]
        {
        return
            "SELECT ECInstanceId "
            "FROM ONLY " ECSql_CachedResponseInfo " "
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
        if (ECDbHelper::IsInstanceInMultiMap(responseKey, nodesToLeave))
            {
            continue;
            }
        responsesToDelete.insert(responseKey);
        }

    return m_hierarchyManager.DeleteInstances(responsesToDelete);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::DeleteResponses(Utf8StringCR name)
    {
    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::DeleteResponses", [=]
        {
        return
            "SELECT GetECClassId(), ECInstanceId "
            "FROM ONLY " ECSql_CachedResponseInfo " "
            "WHERE [" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? ";
        });

    statement->BindText(1, name.c_str(), IECSqlBinder::MakeCopy::No);

    return m_hierarchyManager.DeleteInstances(*statement);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CacheNodeKey CachedResponseManager::SavePage(CachedResponseInfoCR info, uint64_t page, Utf8StringCR cacheTag)
    {
    CacheNodeKey pageKey = FindPage(info, page);
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
CacheNodeKey CachedResponseManager::FindPage(CachedResponseInfoCR info, uint64_t page)
    {
    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::FindPage", [&]
        {
        return
            "SELECT page.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponsePageInfo " page "
            "JOIN " ECSql_ResponseToResponsePage " rel ON rel.TargetECInstanceId = page.ECInstanceId "
            "WHERE rel.SourceECInstanceId = ? AND page.[" CLASS_CachedResponsePageInfo_PROPERTY_Index "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, info.GetInfoKey().GetECInstanceId());
    statement->BindInt64(2, page);

    statement->Step();
    return CacheNodeKey(m_responsePageClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bvector<CacheNodeKey> CachedResponseManager::FindPages(CacheNodeKeyCR responseKey)
    {
    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::FindPages", [&]
        {
        return
            "SELECT page.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponsePageInfo " page "
            "JOIN " ECSql_ResponseToResponsePage " rel ON rel.TargetECInstanceId = page.ECInstanceId "
            "WHERE rel.SourceECInstanceId = ? ";
        });

    statement->BindId(1, responseKey.GetECInstanceId());

    bvector<CacheNodeKey> pages;
    while (BE_SQLITE_ROW == statement->Step())
        {
        pages.push_back(CacheNodeKey(m_responsePageClass->GetId(), statement->GetValueId<ECInstanceId>(0)));
        }

    return pages;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bvector<ECInstanceKey> CachedResponseManager::FindPagesContainingInstance(CachedObjectInfoKeyCR instanceInfoKey)
    {
    bvector<ECInstanceKey> pages;

    if (SUCCESS != m_hierarchyManager.ReadSourceKeys(instanceInfoKey, m_responsePageToResultClass, pages) ||
        SUCCESS != m_hierarchyManager.ReadSourceKeys(instanceInfoKey, m_responsePageToResultWeakClass, pages))
        {
        BeAssert(false);
        }

    return pages;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CacheNodeKey CachedResponseManager::InsertPage(CachedResponseInfoCR info, uint64_t page, Utf8StringCR cacheTag)
    {
    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::InsertPage", [&]
        {
        return "INSERT INTO " ECSql_CachedResponsePageInfo " ("
            "[" CLASS_CachedResponsePageInfo_PROPERTY_Index "],"
            "[" CLASS_CachedResponsePageInfo_PROPERTY_CacheTag "],"
            "[" CLASS_CachedResponsePageInfo_PROPERTY_CacheDate "]"
            ") VALUES (?,?,?)";
        });

    statement->BindInt64(1, page);
    statement->BindText(2, cacheTag.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindDateTime(3, DateTime::GetCurrentTimeUtc());

    CacheNodeKey pageKey;
    statement->Step(pageKey);

    if (!m_hierarchyManager.RelateInstances(info.GetInfoKey(), pageKey, m_responseToResponsePageClass).IsValid())
        {
        pageKey = CacheNodeKey();
        }

    return pageKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::UpdatePage(ECInstanceId pageId, Utf8StringCR cacheTag)
    {
    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::UpdatePage", [&]
        {
        return
            "UPDATE ONLY " ECSql_CachedResponsePageInfo " "
            "SET " CLASS_CachedResponsePageInfo_PROPERTY_CacheTag " = ?, "
            "    " CLASS_CachedResponsePageInfo_PROPERTY_CacheDate " = ? "
            "WHERE ECInstanceId = ? ";
        });

    statement->BindText(1, cacheTag.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindDateTime(2, DateTime::GetCurrentTimeUtc());
    statement->BindId(3, pageId);

    if (BE_SQLITE_DONE != statement->Step())
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::UpdatePageCacheDate(ECInstanceId pageId)
    {
    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::UpdatePageCacheDate", [&]
        {
        return
            "UPDATE ONLY " ECSql_CachedResponsePageInfo " SET " CLASS_CachedResponsePageInfo_PROPERTY_CacheDate " = ? "
            "WHERE ECInstanceId = ? ";
        });

    statement->BindDateTime(1, DateTime::GetCurrentTimeUtc());
    statement->BindId(2, pageId);

    if (BE_SQLITE_DONE != statement->Step())
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::ClearPageCacheTag(ECInstanceId pageId)
    {
    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::ClearPageCacheTag", [&]
        {
        return
            "UPDATE ONLY " ECSql_CachedResponsePageInfo " SET " CLASS_CachedResponsePageInfo_PROPERTY_CacheTag " = NULL "
            "WHERE ECInstanceId = ? ";
        });

    statement->BindId(1, pageId);

    if (BE_SQLITE_DONE != statement->Step())
        {
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
        !m_hierarchyManager.RelateInstances(info.GetInfoKey(), info.GetKey().GetParent(), m_responseToParentClass).IsValid() ||
        !m_hierarchyManager.RelateInstances(info.GetInfoKey(), info.GetKey().GetHolder(), m_responseToHolderClass).IsValid())
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::UpdatePageCachedDate(ResponseKeyCR responseKey, uint64_t page)
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
ResponseKeyCR responseKey,
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

    CacheNodeKey pageKey = SavePage(info, page, cacheTag);

    if (SUCCESS != RelateResultInstancesToPage(info.GetKey().GetHolder(), pageKey, instances))
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
CacheNodeKeyCR responseHolderNodeKey,
CacheNodeKeyCR pageKey,
const InstanceCacheHelper::CachedInstances& instances
)
    {
    const bset<CachedInstanceKey>& newCached = instances.GetCachedInstances();
    bset<CachedInstanceKey> oldCached;
    
    if (SUCCESS != m_objectInfoManager.ReadCachedInstanceKeys(pageKey, *m_responsePageToResultClass, oldCached) ||
        SUCCESS != m_relationshipInfoManager.ReadCachedRelationshipsFromHolder(pageKey, m_responsePageToResultClass, oldCached))
        {
        return ERROR;
        }

    bset<CachedInstanceKey> instancesToAdd, instancesToRemove;

    std::set_difference(newCached.begin(), newCached.end(),
                        oldCached.begin(), oldCached.end(),
                        std::inserter(instancesToAdd, instancesToAdd.end()));

    std::set_difference(oldCached.begin(), oldCached.end(),
                        newCached.begin(), newCached.end(),
                        std::inserter(instancesToRemove, instancesToRemove.end()));

    // Avoid holding cycles
    CachedInstanceKey holderKey = m_objectInfoManager.ReadCachedInstanceKey(responseHolderNodeKey);
    if (0 != instancesToAdd.erase(holderKey))
        {
        bset<CachedInstanceKey> oldWeakCached, newWeakCached;
        newWeakCached.insert(holderKey);

        if (SUCCESS != m_objectInfoManager.ReadCachedInstanceKeys(pageKey, *m_responsePageToResultWeakClass, oldWeakCached))
            {
            return ERROR;
            }

        if (oldWeakCached != newWeakCached)
            {
            if (SUCCESS != m_hierarchyManager.RemoveCachedInstancesFromHolder(pageKey, m_responsePageToResultWeakClass, oldWeakCached) ||
                SUCCESS != m_hierarchyManager.RelateCachedInstancesToHolder(pageKey, m_responsePageToResultWeakClass, newWeakCached))
                {
                return ERROR;
                }
            }
        }

    if (SUCCESS != m_hierarchyManager.RelateCachedInstancesToHolder(pageKey, m_responsePageToResultClass, instancesToAdd) ||
        SUCCESS != m_hierarchyManager.RemoveCachedInstancesFromHolder(pageKey, m_responsePageToResultClass, instancesToRemove))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::AddAdditionalInstance(ResponseKeyCR responseKey, CachedObjectInfoKeyCR instanceInfoKey)
    {
    auto info = ReadInfo(responseKey);
    if (!info.IsCached())
        {
        if (SUCCESS != InsertInfo(info))
            return ERROR;
        if (SUCCESS != SetResponseCompleted(responseKey, true))
            return ERROR;
        }

    if (!m_dbAdapter.RelateInstances(m_responseToAdditionalInstanceClass, info.GetInfoKey(), instanceInfoKey).IsValid())
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::RemoveAdditionalInstance(ResponseKeyCR responseKey, CachedObjectInfoKeyCR instanceInfoKey)
    {
    auto info = ReadInfo(responseKey);
    if (!info.IsCached())
        return ERROR;

    return m_dbAdapter.DeleteRelationship(m_responseToAdditionalInstanceClass, info.GetInfoKey(), instanceInfoKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::RemoveAdditionalInstance(CachedObjectInfoKeyCR instanceInfoKey)
    {
    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::RemoveAdditionalInstance", [&]
        {
        return "DELETE FROM ONLY " + m_responseToAdditionalInstanceClass->GetECSqlName() + " WHERE TargetECInstanceId = ?";
        });

    statement->BindId(1, instanceInfoKey.GetECInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement->Step()))
        {}

    if (BE_SQLITE_DONE != status)
        return ERROR;

    return SUCCESS;
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
    if (SUCCESS != ReadResponseInstanceKeys(info.GetInfoKey(), resultKeys))
        {
        return ERROR;
        }

    CacheQueryHelper helper(selectProvider);
    return helper.ReadInstances(m_dbAdapter, resultKeys, readCallback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::ReadResponseInstanceKeys(CacheNodeKeyCR responseKey, ECInstanceKeyMultiMap& keysOut)
    {
    for (CacheNodeKeyCR page : FindPages(responseKey))
        {
        if (SUCCESS != m_objectInfoManager.ReadCachedInstanceKeys(page, *m_responsePageToResultClass, keysOut) ||
            SUCCESS != m_objectInfoManager.ReadCachedInstanceKeys(page, *m_responsePageToResultWeakClass, keysOut))
            {
            return ERROR;
            }
        }

    if (SUCCESS != m_objectInfoManager.ReadCachedInstanceKeys(responseKey, *m_responseToAdditionalInstanceClass, keysOut))
        return ERROR;

    return SUCCESS;
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::GetResponsesContainingInstance
(
CachedObjectInfoKeyCR instanceInfoKey,
bset<CachedResponseKey>& keysOut,
Utf8StringCR responseName
)
    {
    ECInstanceKeyMultiMap infos = FindInfosContainingInstance(instanceInfoKey);
    if (infos.empty())
        return SUCCESS;

    Utf8PrintfString statementKey("CachedResponseManager::GetResponsesContainingInstance:%d", responseName.empty());
    auto statement = m_statementCache.GetPreparedStatement(statementKey, [=]
        {
        Utf8String sql("SELECT ECInstanceId, " CLASS_CachedResponseInfo_PROPERTY_Name " "
                       "FROM " ECSql_CachedResponseInfo " "
                       "WHERE InVirtualSet(?, ECInstanceId) ");

        if (!responseName.empty())
            sql += "AND " CLASS_CachedResponseInfo_PROPERTY_Name " = ? ";

        return sql;
        });

    ECInstanceIdSet idSet;
    for (auto pair : infos)
        idSet.insert(pair.second);

    statement->BindVirtualSet(1, idSet);

    if (!responseName.empty())
        statement->BindText(2, responseName.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement->Step()))
        {
        ECInstanceKey infoInstance(m_responseClass->GetId(), statement->GetValueId<ECInstanceId>(0));
        Utf8String name(statement->GetValueText(1));

        bvector<ECInstanceKey> parents;
        if (SUCCESS != m_hierarchyManager.ReadTargetKeys(infoInstance, m_responseToParentClass, parents) || parents.size() != 1)
            return ERROR;

        bvector<ECInstanceKey> holders;
        if (SUCCESS != m_hierarchyManager.ReadTargetKeys(infoInstance, m_responseToHolderClass, holders) || holders.size() != 1)
            return ERROR;

        auto parent = m_objectInfoManager.ConvertToInstanceKey(*parents.begin());
        auto holder = m_objectInfoManager.ConvertToInstanceKey(*holders.begin());

        keysOut.insert({parent, name, holder});
        }
       
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::ReadResponseObjectIds(CacheNodeKeyCR responseKey, bset<ObjectId>& objectIdsOut)
    {
    for (CacheNodeKeyCR page : FindPages(responseKey))
        {
        if (SUCCESS != m_objectInfoManager.ReadCachedInstanceIds(page, *m_responsePageToResultClass, objectIdsOut) ||
            SUCCESS != m_objectInfoManager.ReadCachedInstanceIds(page, *m_responsePageToResultWeakClass, objectIdsOut))
            {
            return ERROR;
            }
        }
        
    if (SUCCESS != m_objectInfoManager.ReadCachedInstanceIds(responseKey, *m_responseToAdditionalInstanceClass, objectIdsOut))
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
Utf8String CachedResponseManager::ReadResponsePageCacheTag(ResponseKeyCR responseKey, uint64_t page)
    {
    auto statement = GetSelectPagePropertyStatement(responseKey, page, CLASS_CachedResponsePageInfo_PROPERTY_CacheTag);
    statement->Step();
    return statement->GetValueText(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
DateTime CachedResponseManager::ReadResponsePageCachedDate(ResponseKeyCR responseKey, uint64_t page)
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
ResponseKeyCR responseKey,
uint64_t page,
Utf8StringCR propertyName
)
    {
    auto key = "CachedResponseManager::GetSelectPagePropertyStatement:" + propertyName;
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT page.[" + propertyName + "] "
            "FROM ONLY " ECSql_CachedResponsePageInfo " page "
            "JOIN " ECSql_ResponseToResponsePage " rel1 "
            "  ON rel1.TargetECInstanceId = page.ECInstanceId "
            "JOIN " ECSql_CachedResponseInfo " info "
            "  ON rel1.SourceECInstanceId = info.ECInstanceId "
            " AND info.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? "
            "JOIN " ECSql_ResponseToParent " rel2 "
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
BentleyStatus CachedResponseManager::TrimPages(ResponseKeyCR responseKey, uint64_t maxPageIndex)
    {
    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::TrimPages", [&]
        {
        return
            "SELECT page.GetECClassId(), page.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponsePageInfo " page "
            "JOIN " ECSql_ResponseToResponsePage " rel1 "
            "  ON rel1.TargetECInstanceId = page.ECInstanceId "
            "JOIN " ECSql_CachedResponseInfo " info "
            "  ON rel1.SourceECInstanceId = info.ECInstanceId "
            " AND info.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? "
            "JOIN " ECSql_ResponseToParent " rel2 "
            "  ON rel2.SourceECInstanceId = rel1.SourceECInstanceId "
            "WHERE rel2.TargetECInstanceId = ? "
            "  AND page.[" CLASS_CachedResponsePageInfo_PROPERTY_Index "] > ? ";
        });

    statement->BindText(1, responseKey.GetName().c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindId(2, responseKey.GetParent().GetECInstanceId());
    statement->BindInt64(3, maxPageIndex);

    return m_hierarchyManager.DeleteInstances(*statement);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::SetResponseCompleted(ResponseKeyCR responseKey, bool isCompleted)
    {
    ECInstanceKey infoKey = FindInfo(responseKey);
    if (!infoKey.IsValid())
        {
        return ERROR;
        }

    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::SetResponseCompleted", [&]
        {
        return
            "UPDATE ONLY " ECSql_CachedResponseInfo " SET " CLASS_CachedResponseInfo_PROPERTY_IsCompleted " = ? "
            "WHERE ECInstanceId = ? ";
        });

    statement->BindBoolean(1, isCompleted);
    statement->BindId(2, infoKey.GetECInstanceId());

    DbResult status = statement->Step();
    if (BE_SQLITE_DONE != status) // WIP06
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CachedResponseManager::IsResponseCompleted(ResponseKeyCR responseKey)
    {
    if (!responseKey.IsValid())
        {
        return false;
        }

    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::IsResponseCompleted", [=]
        {
        return
            "SELECT info.[" CLASS_CachedResponseInfo_PROPERTY_IsCompleted "] "
            "FROM ONLY " ECSql_CachedResponseInfo " info "
            "JOIN ONLY " ECSql_ResponseToParent " rel ON rel.SourceECInstanceId = info.ECInstanceId "
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
const ECInstanceKeyMultiMap& fullyPersistedNodes
)
    {
    // Get all fully cached instances for this response
    Utf8String ecsql =
        "SELECT info.ECInstanceId "
        "FROM ONLY " ECSql_CachedObjectInfo " info "
        "JOIN ONLY " + resultRelationshipClass->GetECSqlName() + " pageToResultRel ON pageToResultRel.TargetECInstanceId = info.ECInstanceId "
        "JOIN ONLY " ECSql_ResponseToResponsePage " responseToPageRel ON responseToPageRel.TargetECInstanceId = pageToResultRel.SourceECInstanceId "
        "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceState "] = ? "
        "  AND responseToPageRel.SourceECInstanceId IN (" + StringHelper::Join(responseIds.begin(), responseIds.end(), ',') + ")";

    ECSqlStatement statement;
    if (SUCCESS != m_dbAdapter.PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }
    statement.BindInt(1, static_cast<int> (CachedInstanceState::Full));

    ECClassCP infoClass = m_objectInfoManager.GetInfoClass();

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        CacheNodeKey infoKey(infoClass->GetId(), statement.GetValueId<ECInstanceId>(0));

        if (ECDbHelper::IsInstanceInMultiMap(infoKey, fullyPersistedNodes))
            {
            // Not temporary
            continue;
            }

        Utf8PrintfString key("CachedResponseManager::MarkTemporaryInstancesAsPartial");
        auto updateStatement = m_statementCache.GetPreparedStatement(key, [&]
            {
            return
                "UPDATE ONLY " ECSql_CachedObjectInfo " SET " CLASS_CachedObjectInfo_PROPERTY_InstanceState " = ? "
                "WHERE ECInstanceId = ? ";
            });

        updateStatement->BindInt(1, static_cast<int> (CachedInstanceState::Partial));
        updateStatement->BindId(2, infoKey.GetECInstanceId());

        if (BE_SQLITE_DONE != updateStatement->Step())
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
BentleyStatus CachedResponseManager::InvalidateResponsePagesContainingInstance(CachedInstanceKeyCR cachedKey)
    {
    ECInstanceKey nodeKey = cachedKey.GetInfoKey();

    bvector<ECInstanceId> pageIds;

    // Result relationship
    auto statement = m_statementCache.GetPreparedStatement("CachedResponseManager::InvalidateResponsePagesContainingInstance:Rel", [&]
        {
        return
            "SELECT page.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponsePageInfo " page "
            "JOIN " ECSql_ResponsePageToResult " rel ON page.ECInstanceId = rel.SourceECInstanceId "
            "WHERE rel.TargetECClassId = ? AND rel.TargetECInstanceId = ? ";
        });

    statement->BindId(1, nodeKey.GetECClassId());
    statement->BindId(2, nodeKey.GetECInstanceId());

    if (SUCCESS != m_dbAdapter.ExtractECIdsFromStatement(*statement, 0, pageIds))
        {
        return ERROR;
        }

    // Result weak relationship
    statement = m_statementCache.GetPreparedStatement("CachedResponseManager::InvalidateResponsePagesContainingInstance:Weak", [&]
        {
        return
            "SELECT page.ECInstanceId "
            "FROM ONLY " ECSql_CachedResponsePageInfo " page "
            "JOIN " ECSql_ResponsePageToResultWeak " rel ON page.ECInstanceId = rel.SourceECInstanceId "
            "WHERE rel.TargetECClassId = ? AND rel.TargetECInstanceId = ? ";
        });

    statement->BindId(1, nodeKey.GetECClassId());
    statement->BindId(2, nodeKey.GetECInstanceId());

    if (SUCCESS != m_dbAdapter.ExtractECIdsFromStatement(*statement, 0, pageIds))
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
