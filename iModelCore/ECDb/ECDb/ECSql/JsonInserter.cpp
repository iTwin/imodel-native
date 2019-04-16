/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECObjects/ECJsonUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle      09/2017
//+---------------+---------------+---------------+---------------+---------------+------
JsonInserter::JsonInserter(ECDbCR ecdb, ECClassCR ecClass, ECCrudWriteToken const* writeToken) : m_ecdb(ecdb), m_ecClass(ecClass)
    {
    Initialize(writeToken);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/2017
//+---------------+---------------+---------------+---------------+---------------+------
void JsonInserter::Initialize(ECCrudWriteToken const* writeToken)
    {
    m_jsonClassName = ECJsonUtilities::FormatClassName(m_ecClass);

    Utf8String ecsql("INSERT INTO ");
    //add ECInstanceId. If NULL is bound to it, ECDb will auto-generate one
    ecsql.append(m_ecClass.GetECSqlName()).append("(" ECDBSYS_PROP_ECInstanceId);
    Utf8String valuesClause(") VALUES(?");

    int parameterIndex = 1;
    //cache the binding info as we later need to set the user provided instance id (if available)
    m_bindingMap[ECJsonSystemNames::Id()] = BindingInfo(parameterIndex);
    parameterIndex++;

    PrimitiveECPropertyCP currentTimeStampProp = nullptr;
    if (SUCCESS != CoreCustomAttributeHelper::GetCurrentTimeStampProperty(currentTimeStampProp, m_ecClass))
        {
        LOG.errorv("JsonInserter failure: Could not retrieve the 'ClassHasCurrentTimeStampProperty' custom attribute from the ECClass '%s'.",
                   m_ecClass.GetFullName());
        m_isValid = false;
        return;
        }

    for (ECPropertyCP ecProperty : m_ecClass.GetProperties(true))
        {
        //Current time stamp props are populated by SQLite, so ignore them here.
        if (currentTimeStampProp != nullptr && ecProperty == currentTimeStampProp)
            continue;

        ecsql.append(",[").append(ecProperty->GetName()).append("]");
        valuesClause.append(",?");
        m_bindingMap[ecProperty->GetName().c_str()] = BindingInfo(parameterIndex, *ecProperty);
        parameterIndex++;
        }

    if (m_ecClass.IsRelationshipClass())
        {
        //SourceECClassId and TargetECClassId are not needed during insert
        ecsql.append("," ECDBSYS_PROP_SourceECInstanceId);
        valuesClause.append(",?");
        m_bindingMap[ECJsonSystemNames::SourceId()] = BindingInfo(parameterIndex);

        parameterIndex++;
        ecsql.append("," ECDBSYS_PROP_TargetECInstanceId);
        valuesClause.append(",?");
        m_bindingMap[ECJsonSystemNames::TargetId()] = BindingInfo(parameterIndex);
        }

    ecsql.append(valuesClause).append(")");
    m_isValid = ECSqlStatus::Success == m_statement.Prepare(m_ecdb, ecsql.c_str(), writeToken);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonInserter::Insert(ECInstanceKey& key, JsonValueCR json) const
    {
    if (!m_isValid)
        return BE_SQLITE_ERROR;

    if (!json.isNull() && !json.isObject())
        {
        LOG.errorv("JsonInserter failure. The JSON to insert must be an object or null, but was: %s", json.ToString().c_str());
        return BE_SQLITE_ERROR;
        }

    m_statement.ClearBindings();

    if (!json.isNull())
        {
        BeAssert(json.isObject());
        for (Json::Value::iterator it = json.begin(); it != json.end(); it++)
            {
            Utf8CP memberName = it.memberName();
            JsonValueCR memberJson = *it;

            auto bindingMapLookupIt = m_bindingMap.find(memberName);
            if (bindingMapLookupIt == m_bindingMap.end())
                {
                if (strcmp(ECJsonSystemNames::ClassName(), memberName) == 0)
                    {
                    if (memberJson.isNull() || !memberJson.isString() || !m_jsonClassName.EqualsIAscii(memberJson.asCString()))
                        {
                        LOG.errorv("JsonInserter failure. The '%s' member of the JSON to insert has an invalid value: %s",
                                   ECJsonSystemNames::ClassName(), memberJson.ToString().c_str());
                        return BE_SQLITE_ERROR;
                        }

                    continue;
                    }

                if (strcmp(ECJsonSystemNames::SourceClassName(), memberName) == 0 ||
                    strcmp(ECJsonSystemNames::TargetClassName(), memberName) == 0)
                    continue; //those two are ignored as ECSQL doesn't validate them anyways

                LOG.errorv("JsonInserter failure. The JSON member '%s' does not match with a property in ECClass '%s' or it maps to a 'CurrentTimeStamp' property (which is handled by ECDb).",
                           memberName, m_ecClass.GetFullName());
                return BE_SQLITE_ERROR;
                }

            BindingInfo const& bindingInfo = bindingMapLookupIt->second;

            if (bindingInfo.IsSystemProperty())
                {
                BeAssert(strcmp(memberName, ECJsonSystemNames::Id()) == 0 ||
                         strcmp(memberName, ECJsonSystemNames::SourceId()) == 0 ||
                         strcmp(memberName, ECJsonSystemNames::TargetId()) == 0);

                BeInt64Id id = ECJsonUtilities::JsonToId<BeInt64Id>(memberJson);
                if (!id.IsValid())
                    return BE_SQLITE_ERROR;

                if (ECSqlStatus::Success != m_statement.BindId(bindingInfo.GetParameterIndex(), id))
                    return BE_SQLITE_ERROR;

                continue;
                }

            if (ECSqlStatus::Success != JsonECSqlBinder::BindValue(m_statement.GetBinder(bindingInfo.GetParameterIndex()), memberJson, *bindingInfo.GetProperty(), m_ecdb.GetClassLocater()))
                {
                LOG.errorv("JsonInserter failure. Could not bind JSON member '%s' to parameter %d for EC JSON %s,",
                           memberName, bindingInfo.GetParameterIndex(), json.ToString().c_str());
                return BE_SQLITE_ERROR;
                }
            }
        }

    //now execute statement
    const DbResult stepStatus = m_statement.Step(key);

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_statement.Reset();
    m_statement.ClearBindings();

    return BE_SQLITE_DONE == stepStatus ? BE_SQLITE_OK : stepStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonInserter::Insert(JsonValueR json) const
    {
    ECInstanceKey key;
    const DbResult stat = Insert(key, json);
    if (BE_SQLITE_OK != stat)
        return stat;

    //this overload expects the ECInstanceId of the new row to be written into the input JSON
    if (!json.isMember(ECJsonUtilities::json_id()))
        {
        if (SUCCESS != ECJsonUtilities::IdToJson(json[ECJsonUtilities::json_id()], key.GetInstanceId()))
            return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Shaun.Sewall                    01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonInserter::Insert(ECInstanceKey& key, RapidJsonValueCR json) const
    {
    if (!m_isValid)
        return BE_SQLITE_ERROR;

    if (!json.IsNull() && !json.IsObject())
        {
        rapidjson::StringBuffer jsonStr;
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
        json.Accept(writer);
        LOG.errorv("JsonInserter failure. The JSON to insert must be an object or null, but was: %s", jsonStr.GetString());
        return BE_SQLITE_ERROR;
        }

    if (!json.IsNull())
        {
        BeAssert(json.IsObject());
        for (rapidjson::Value::ConstMemberIterator it = json.MemberBegin(); it != json.MemberEnd(); ++it)
            {
            Utf8CP memberName = it->name.GetString();
            RapidJsonValueCR memberJson = it->value;

            if (strcmp(ECJsonSystemNames::ClassName(), memberName) == 0)
                {
                if (memberJson.IsNull() || !memberJson.IsString() || !m_jsonClassName.EqualsIAscii(memberJson.GetString()))
                    {
                    rapidjson::StringBuffer jsonStr;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
                    memberJson.Accept(writer);
                    LOG.errorv("JsonInserter failure. The '%s' member of the JSON to insert has an invalid value: %s",
                               ECJsonSystemNames::ClassName(), jsonStr.GetString());
                    return BE_SQLITE_ERROR;
                    }

                continue;
                }

            auto bindingMapLookupIt = m_bindingMap.find(memberName);
            if (bindingMapLookupIt == m_bindingMap.end())
                {
                if (strcmp(ECJsonSystemNames::ClassName(), memberName) == 0)
                    {
                    if (memberJson.IsNull() || !memberJson.IsString() || !m_jsonClassName.EqualsIAscii(memberJson.GetString()))
                        {
                        rapidjson::StringBuffer jsonStr;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
                        memberJson.Accept(writer);

                        LOG.errorv("JsonInserter failure. The '%s' member of the JSON to insert has an invalid value: %s",
                                   ECJsonSystemNames::ClassName(), jsonStr.GetString());
                        return BE_SQLITE_ERROR;
                        }

                    continue;
                    }

                if (strcmp(ECJsonSystemNames::SourceClassName(), memberName) == 0 ||
                    strcmp(ECJsonSystemNames::TargetClassName(), memberName) == 0)
                    continue; //those two are ignored as ECSQL doesn't validate them anyways

                LOG.errorv("JsonInserter failure. The JSON member '%s' does not match with a property in ECClass '%s'.",
                           memberName, m_ecClass.GetFullName());
                return BE_SQLITE_ERROR;
                }

            BindingInfo const& bindingInfo = bindingMapLookupIt->second;

            if (bindingInfo.IsSystemProperty())
                {
                BeAssert(strcmp(memberName, ECJsonSystemNames::Id()) == 0 ||
                         strcmp(memberName, ECJsonSystemNames::SourceId()) == 0 ||
                         strcmp(memberName, ECJsonSystemNames::TargetId()) == 0);

                BeInt64Id id = ECJsonUtilities::JsonToId<BeInt64Id>(memberJson);
                if (!id.IsValid())
                    return BE_SQLITE_ERROR;

                if (ECSqlStatus::Success != m_statement.BindId(bindingInfo.GetParameterIndex(), id))
                    return BE_SQLITE_ERROR;

                continue;
                }

            if (ECSqlStatus::Success != JsonECSqlBinder::BindValue(m_statement.GetBinder(bindingInfo.GetParameterIndex()), memberJson, *bindingInfo.GetProperty(), m_ecdb.GetClassLocater()))
                {
                rapidjson::StringBuffer jsonStr;
                rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
                memberJson.Accept(writer);
                LOG.errorv("JsonInserter failure. Could not bind JSON member '%s' to parameter %d for EC JSON %s,",
                           memberName, bindingInfo.GetParameterIndex(), jsonStr.GetString());
                return BE_SQLITE_ERROR;
                }
            }
        }

    //now execute statement
    const DbResult stepStatus = m_statement.Step(key);

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_statement.Reset();
    m_statement.ClearBindings();

    return BE_SQLITE_DONE == stepStatus ? BE_SQLITE_OK : stepStatus;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
