/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Persistence/CacheQueryHelper.h>

#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include <WebServices/Cache/Util/ECDbHelper.h>
#include <WebServices/Cache/Util/ECCustomAttributeHelper.h>
#include <WebServices/Cache/Util/ECExpressionHelper.h>

#include "../Logging.h"
#include "Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheQueryHelper::CacheQueryHelper(const ISelectProvider& selectProvider) :
m_selectProvider(selectProvider)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CacheQueryHelper::ClassReadInfo> CacheQueryHelper::CreateReadInfos(ECClassCP ecClass) const
    {
    bvector<ECClassCP> ecClasses;
    ecClasses.push_back(ecClass);
    return CreateReadInfos(ecClasses);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CacheQueryHelper::ClassReadInfo> CacheQueryHelper::CreateReadInfos(const bvector<ECClassP>& ecClasses) const
    {
    bvector<ECClassCP> constClasses;
    for (ECClassCP ecClass : ecClasses)
        {
        constClasses.push_back(ecClass);
        }
    return CreateReadInfos(constClasses);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CacheQueryHelper::ClassReadInfo> CacheQueryHelper::CreateReadInfos(bvector<ECClassCP> ecClasses) const
    {
    bvector<CacheQueryHelper::ClassReadInfo> readInfos;

    std::sort(ecClasses.begin(), ecClasses.end(), [=] (ECClassCP a, ECClassCP b)
        {
        return m_selectProvider.GetSortPriority(*a) > m_selectProvider.GetSortPriority(*b);
        });

    for (ECClassCP ecClass : ecClasses)
        {
        auto selectProperties = m_selectProvider.GetSelectProperties(*ecClass);

        if (nullptr == selectProperties)
            {
            continue;
            }

        auto sortProperties = std::make_shared<ISelectProvider::SortProperties>(m_selectProvider.GetSortProperties(*ecClass));

        CacheQueryHelper::ClassReadInfo options(*ecClass, selectProperties, sortProperties);
        readInfos.push_back(options);
        }

    return readInfos;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CacheQueryHelper::CreateSelectClause
(
const ISelectProvider::SelectProperties& selectProperties,
Utf8CP classAlias,
Utf8CP infoAlias,
bool* pInfoNeedsSelecting
)
    {
    Utf8String selectClause;

    if (selectProperties.GetSelectAll())
        {
        selectClause.Sprintf("%s.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "], %s.*", infoAlias, classAlias);
        if (pInfoNeedsSelecting)
            {
            *pInfoNeedsSelecting = true;
            }
        }
    else
        {
        if (std::find(selectProperties.GetExtendedProperties().begin(), selectProperties.GetExtendedProperties().end(), DataSourceCache_PROPERTY_RemoteId)
            != selectProperties.GetExtendedProperties().end())
            {
            if (!selectClause.empty())
                {
                selectClause += ", ";
                }

            selectClause += Utf8PrintfString("%s.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "]", infoAlias);
            if (pInfoNeedsSelecting)
                {
                *pInfoNeedsSelecting = true;
                }
            }

        for (ECPropertyCP ecProperty : selectProperties.GetProperties())
            {
            if (!selectClause.empty())
                {
                selectClause += ", ";
                }

            selectClause += Utf8PrintfString("%s.[%s]", classAlias, Utf8String(ecProperty->GetName()).c_str());
            }

        if (selectProperties.GetSelectInstanceId())
            {
            if (!selectClause.empty())
                {
                selectClause += ", ";
                }

            selectClause += Utf8PrintfString("%s.[ECInstanceId], %s.[ECClassId]", classAlias, classAlias);
            }
        }

    if (selectClause.empty())
        {
        selectClause = "NULL";
        }

    return selectClause;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CacheQueryHelper::ECSql::CreateOrderByClause
(
const ISelectProvider::SortProperties& sortProperties,
ECClassCR ecClass,
Utf8CP classAlias,
bool reverseOrder
)
    {
    Utf8String orderByClause;
    if (sortProperties.empty())
        {
        return orderByClause;
        }

    for (const ISelectProvider::SortProperty& sortProperty : sortProperties)
        {
        if (!orderByClause.empty())
            {
            orderByClause += ", ";
            }

        Utf8CP direction = sortProperty.GetSortAscending() != reverseOrder ? "ASC" : "DESC";
        Utf8CP format = IsTypeString(sortProperty.GetProperty()) ? "LOWER (%s.[%s]) %s" : "%s.[%s] %s";

        orderByClause += Utf8PrintfString(format, classAlias, Utf8String(sortProperty.GetProperty().GetName()).c_str(), direction);
        }

    return orderByClause;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CacheQueryHelper::IsTypeString(ECPropertyCR property)
    {
    return nullptr != property.GetAsPrimitiveProperty() &&
        property.GetAsPrimitiveProperty()->GetType() == PrimitiveType::PRIMITIVETYPE_String;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CacheQueryHelper::ECSql::SelectPropertiesByRemoteIds
(
const ClassReadInfo& info,
Utf8StringCR commaSeperatedRemoteIds,
Utf8StringCR optionalWhereClause
)
    {
    ECClassCR ecClass = info.GetECClass();
    Utf8String selectClause = CreateSelectClause(info.GetSelectProperties(), "instance", "info", nullptr);

    Utf8String orderByClause = CreateOrderByClause(info.GetSortProperties(), ecClass, "instance");
    if (!orderByClause.empty())
        orderByClause = "ORDER BY " + orderByClause;

    Utf8String optionalAndClause;
    if (!optionalWhereClause.empty())
        {
        optionalAndClause.Sprintf("AND (%s) ", optionalWhereClause.c_str());
        }

    Utf8PrintfString ecSql
        (
        "SELECT %s FROM ONLY %s instance "
        "JOIN ONLY " ECSql_CachedObjectInfo " info ON "
        "   info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] = instance.ECInstanceId "
        "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] IN (%s) %s"
        "%s",
        selectClause.c_str(),
        ecClass.GetECSqlName().c_str(),
        commaSeperatedRemoteIds.c_str(),
        optionalAndClause.c_str(),
        orderByClause.c_str()
        );
    return ecSql;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CacheQueryHelper::ECSql::SelectPropertiesByRemoteId
(
const ClassReadInfo& info
)
    {
    ECClassCR ecClass = info.GetECClass();
    Utf8String selectClause = CreateSelectClause(info.GetSelectProperties(), "instance", "info", nullptr);

    Utf8String orderByClause = CreateOrderByClause(info.GetSortProperties(), ecClass, "instance");
    if (!orderByClause.empty())
        orderByClause = "ORDER BY " + orderByClause;

    Utf8PrintfString ecSql
        (
        "SELECT %s FROM ONLY %s instance "
        "JOIN ONLY " ECSql_CachedObjectInfo " info ON "
        "   info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] = instance.ECInstanceId "
        "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] = ? "
        "%s",
        selectClause.c_str(),
        ecClass.GetECSqlName().c_str(),
        orderByClause.c_str()
        );
    return ecSql;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CacheQueryHelper::ECSql::SelectPropertiesByWhereClause
(
const ClassReadInfo& info,
Utf8StringCR customWhereClause
)
    {
    bool infoNeedsSelecting = false;
    ECClassCR ecClass = info.GetECClass();
    Utf8String selectClause = CreateSelectClause(info.GetSelectProperties(), "instance", "info", &infoNeedsSelecting);

    Utf8String orderByClause = CreateOrderByClause(info.GetSortProperties(), ecClass, "instance");
    if (!orderByClause.empty())
        orderByClause = "ORDER BY " + orderByClause;

    Utf8String infoJoin;
    if (infoNeedsSelecting)
        {
        infoJoin =
            "JOIN ONLY " ECSql_CachedObjectInfo " info ON "
            "info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] = instance.ECInstanceId ";
        }

    Utf8PrintfString ecSql
        (
        "SELECT %s FROM ONLY %s instance "
        "%s"
        "WHERE %s "
        "%s",
        selectClause.c_str(),
        ecClass.GetECSqlName().c_str(),
        infoJoin.c_str(),
        customWhereClause.c_str(),
        orderByClause.c_str()
        );

    return ecSql;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CacheQueryHelper::ECSql::SelectRemoteIdsByRelatedSourceECInstanceId
(
ECRelationshipClassCR relationshipClass
)
    {
    Utf8PrintfString ecSql
        (
        "SELECT rel.TargetECClassId, targetInfo.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] "
        "FROM ONLY " ECSql_CachedObjectInfo " targetInfo "
        "JOIN ONLY %s rel ON rel.TargetECInstanceId = targetInfo.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] "
        "WHERE rel.SourceECInstanceId = ?",
        relationshipClass.GetECSqlName().c_str()
        );
    return ecSql;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CacheQueryHelper::ECSql::SelectAllPropertiesAndRemoteIdByECInstanceId(ECClassCR cachedInstanceClass)
    {
    Utf8PrintfString ecSql
        (
        "SELECT info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "], instance.* FROM ONLY %s instance "
        "JOIN ONLY " ECSql_CachedObjectInfo " info ON "
        "   info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] = instance.ECInstanceId "
        "WHERE instance.ECInstanceId = ? "
        "LIMIT 1 ",
        cachedInstanceClass.GetECSqlName().c_str()
        );
    return ecSql;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CacheQueryHelper::ECSql::SelectAllPropertiesByRemoteId(ECClassCR cachedInstanceClass)
    {
    Utf8PrintfString ecSql
        (
        "SELECT instance.* FROM ONLY %s instance "
        "JOIN ONLY " ECSql_CachedObjectInfo " info ON "
        "   info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] = instance.ECInstanceId "
        "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] = ? "
        "LIMIT 1 ",
        cachedInstanceClass.GetECSqlName().c_str()
        );
    return ecSql;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CacheQueryHelper::ECSql::SelectECInstanceIdByRemoteId(ECClassCR cachedInstanceClass)
    {
    Utf8PrintfString ecSql
        (
        "SELECT info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] "
        "FROM ONLY " ECSql_CachedObjectInfo " info "
        "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] = ? "
        "LIMIT 1 "
        );
    return ecSql;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CacheQueryHelper::ECSql::SelectRemoteIdsByECInstanceIds
(
ECClassCR cachedInstanceClass,
Utf8StringCR commaSeperatedECInstanceIds
)
    {
    Utf8PrintfString ecSql
        (
        "SELECT info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] "
        "FROM ONLY " ECSql_CachedObjectInfo " info "
        "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] IN (%s) ",
        commaSeperatedECInstanceIds.c_str()
        );
    return ecSql;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheQueryHelper::ClassReadInfo::ClassReadInfo
(
ECClassCR ecClass,
std::shared_ptr<ISelectProvider::SelectProperties> selectProperties,
std::shared_ptr<ISelectProvider::SortProperties> sortProperties
) :
m_ecClass(&ecClass),
m_selectProperties(selectProperties),
m_sortProperties(sortProperties)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR CacheQueryHelper::ClassReadInfo::GetECClass() const
    {
    return *m_ecClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const ISelectProvider::SelectProperties& CacheQueryHelper::ClassReadInfo::GetSelectProperties() const
    {
    return *m_selectProperties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const ISelectProvider::SortProperties& CacheQueryHelper::ClassReadInfo::GetSortProperties() const
    {
    return *m_sortProperties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int CacheQueryHelper::GetRemoteIdColumnIndex(const ClassReadInfo& info, bool& columnNeedsVerification)
    {
    columnNeedsVerification = false;

    if (info.GetSelectProperties().GetSelectAll() ||
        std::find(info.GetSelectProperties().GetExtendedProperties().begin(), info.GetSelectProperties().GetExtendedProperties().end(), DataSourceCache_PROPERTY_RemoteId)
        != info.GetSelectProperties().GetExtendedProperties().end())
        {
        // Remote id should be selected as first column as in CreateSelectClause()
        columnNeedsVerification = true;
        return 0;
        }

    // Remote id is not selected
    return -1;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CacheQueryHelper::ReadInstances
(
IECDbAdapter& dbAdapter,
const ECInstanceKeyMultiMap& instances,
const ReadCallback& readCallback
)
    {
    // Prepare read info
    bvector<ECClassCP> classes = dbAdapter.GetECClasses(instances);
    bvector<CacheQueryHelper::ClassReadInfo> resultClassesReadInfo = CreateReadInfos(classes);

    // Read result instances
    for (CacheQueryHelper::ClassReadInfo& info : resultClassesReadInfo)
        {
        auto range = instances.equal_range(info.GetECClass().GetId());

        Utf8String ecInstaceIdsList = ECDbHelper::ToECInstanceIdList(range.first, range.second);
        Utf8PrintfString whereClause("instance.ECInstanceId IN (%s)", ecInstaceIdsList.c_str());

        Utf8String ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(info, whereClause);

        ECSqlStatement statement;
        if (SUCCESS != dbAdapter.PrepareStatement(statement, ecSql))
            {
            return ERROR;
            }

        if (SUCCESS != readCallback(info, statement))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

int CacheQueryHelper::GetVerifiedRemoteIdColumnIndex(const ClassReadInfo& info, ECSqlStatement& statement)
    {
    bool remoteIdColumnNeedsVerification = false;

    int remoteIdColumn = GetRemoteIdColumnIndex(info, remoteIdColumnNeedsVerification);

    if (remoteIdColumnNeedsVerification)
        {
        ECPropertyCP remoteIdProperty = statement.GetColumnInfo(remoteIdColumn).GetProperty();
        if (nullptr == remoteIdProperty ||
            !remoteIdProperty->GetClass().GetName().Equals(CLASS_CachedObjectInfo) ||
            !remoteIdProperty->GetName().EqualsI(CLASS_CachedObjectInfo_PROPERTY_RemoteId))
            {
            BeAssert(false && "Read info does not match statement");
            remoteIdColumn = -1;
            }
        }
    return remoteIdColumn;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CacheQueryHelper::ReadJsonInstance
(
const ClassReadInfo& info,
ECSqlStatement& statement,
JsonValueR jsonInstanceOut,
ICancellationTokenPtr ct
)
    {
    if (ct && ct->IsCanceled())
        {
        return ERROR;
        }

    int remoteIdColumn = GetVerifiedRemoteIdColumnIndex(info, statement);

    ECClassId ecClassId = info.GetECClass().GetId();
    JsonECSqlSelectAdapter adapter(statement);
    if (SUCCESS != adapter.GetRowInstance(jsonInstanceOut, ecClassId))
        {
        return ERROR;
        }

    if (remoteIdColumn >= 0)
        {
        jsonInstanceOut[DataSourceCache_PROPERTY_RemoteId] = statement.GetValueText(remoteIdColumn);
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CacheQueryHelper::ReadJsonInstance
(
const ClassReadInfo& info,
ECSqlStatement& statement,
RapidJsonValueR jsonInstanceOut,
rapidjson::Document::AllocatorType& allocator,
ICancellationTokenPtr ct
)
    {
    if (ct && ct->IsCanceled())
        {
        return ERROR;
        }

    int remoteIdColumn = GetVerifiedRemoteIdColumnIndex(info, statement);

    ECClassId ecClassId = info.GetECClass().GetId();
    JsonECSqlSelectAdapter adapter(statement);
    if (SUCCESS != adapter.GetRowInstance(jsonInstanceOut, ecClassId, allocator))
        {
        return ERROR;
        }

    if (remoteIdColumn >= 0)
        {
        Utf8String remoteIdText = statement.GetValueText(remoteIdColumn);
        jsonInstanceOut.AddMember(DataSourceCache_PROPERTY_RemoteId, rapidjson::Value(remoteIdText.c_str(), allocator).Move(), allocator);
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CacheQueryHelper::ReadJsonInstances
(
const ClassReadInfo& info,
ECSqlStatement& statement,
JsonValueR jsonInstancesArrayOut,
ICancellationTokenPtr ct
)
    {
    if (ct && ct->IsCanceled())
        {
        return ERROR;
        }

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        if (ct && ct->IsCanceled())
            {
            return ERROR;
            }

        JsonValueR currentObj = jsonInstancesArrayOut.append(Json::objectValue);
        if (SUCCESS != ReadJsonInstance(info, statement, currentObj, ct))
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
