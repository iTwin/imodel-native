/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ExtendedDataAdapter.h>

#include <WebServices/Cache/Util/ECDbHelper.h>

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define CLASS_ExtendedData_PROPERTY_Content         "Content"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExtendedDataAdapter::ExtendedDataAdapter(ObservableECDb& db, IDelegate& edDelegate) :
m_dbAdapter(db),
m_statementCache(db),
m_delegate(edDelegate)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExtendedData ExtendedDataAdapter::GetData(ECInstanceKeyCR ownerKey)
    {
    auto edClass = m_delegate.GetExtendedDataClass(ownerKey);
    auto edRelClass = m_delegate.GetExtendedDataRelationshipClass(ownerKey);

    if (nullptr == edClass || nullptr == edRelClass)
        {
        return ExtendedData();
        }

    Utf8PrintfString key("GetData:%llu:%llu", edClass->GetId().GetValue(), edRelClass->GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT data.ECInstanceId, data.[" CLASS_ExtendedData_PROPERTY_Content "] "
            "FROM " + edClass->GetECSqlName() + " data "
            "JOIN " + edRelClass->GetECSqlName() + " relationship ON relationship.TargetECInstanceId = data.ECInstanceId "
            "WHERE relationship.SourceECClassId = ? AND relationship.SourceECInstanceId = ? "
            "LIMIT 1 ";
        });

    ECInstanceKey holderKey = m_delegate.GetHolderKey(ownerKey);

    statement->BindId(1, holderKey.GetClassId());
    statement->BindId(2, holderKey.GetInstanceId());

    ECInstanceId extendedDataId;
    Utf8String content;

    DbResult status;
    if (BE_SQLITE_ROW == (status = statement->Step()))
        {
        extendedDataId = statement->GetValueId<ECInstanceId>(0);
        content = statement->GetValueText(1);
        }

    auto extendedDataJson = std::make_shared<Json::Value>(Json::objectValue);
    Json::Reader::Parse(content, *extendedDataJson);

    return ExtendedData(ownerKey, {edClass->GetId(), extendedDataId}, extendedDataJson);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ExtendedDataAdapter::UpdateData(ExtendedData& data)
    {
    auto edClass = m_delegate.GetExtendedDataClass(data.m_ownerKey);
    auto edRelClass = m_delegate.GetExtendedDataRelationshipClass(data.m_ownerKey);

    if (nullptr == edClass || nullptr == edRelClass)
        {
        return ERROR;
        }

    Utf8String content = data.m_extendedData->toStyledString();

    if (data.m_extendedDataKey.IsValid())
        {
        Utf8PrintfString key("UpdateData:%llu", edClass->GetId().GetValue());
        auto statement = m_statementCache.GetPreparedStatement(key, [&]
            {
            return
                "UPDATE ONLY " + edClass->GetECSqlName() + " "
                "SET [" CLASS_ExtendedData_PROPERTY_Content "] = ? "
                "WHERE ECInstanceId = ? ";
            });

        statement->BindText(1, content.c_str(), IECSqlBinder::MakeCopy::No);
        statement->BindId(2, data.m_extendedDataKey.GetInstanceId());

        DbResult status;
        if (BE_SQLITE_DONE != (status = statement->Step()))
            {
            return ERROR;
            }
        return SUCCESS;
        }

    Utf8PrintfString key("InsertData:%llu", edClass->GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return "INSERT INTO " + edClass->GetECSqlName() + " ([" CLASS_ExtendedData_PROPERTY_Content "]) VALUES (?) ";
        });

    statement->BindText(1, content.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult status;
    if (BE_SQLITE_DONE != (status = statement->Step(data.m_extendedDataKey)))
        {
        return ERROR;
        }

    ECInstanceKey holderKey = m_delegate.GetHolderKey(data.m_ownerKey);
    if (!m_dbAdapter.RelateInstances(edRelClass, holderKey, data.m_extendedDataKey).IsValid())
        {
        return ERROR;
        }

    return SUCCESS;
    }
