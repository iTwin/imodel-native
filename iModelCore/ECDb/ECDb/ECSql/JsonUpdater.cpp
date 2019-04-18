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
JsonUpdater::Options::Options(JsonUpdater::SystemPropertiesOption systemPropertiesOption, JsonUpdater::ReadonlyPropertiesOption readonlyPropertiesOption, Utf8CP ecsqlOptions) : m_systemProps(systemPropertiesOption), m_readonlyProps(readonlyPropertiesOption), m_ecsqlOptions(ecsqlOptions)
    {
    if (m_ecsqlOptions.ContainsI(OptionsExp::READONLYPROPERTIESAREUPDATABLE_OPTION))
        {
        LOG.errorv("Invalid JsonUpdater::Option. The ECSQLOPTION '%s' may not be specified. Always use the JsonUpdater::ReadonlyPropertiesOption argument instead.", OptionsExp::READONLYPROPERTIESAREUPDATABLE_OPTION);
        BeAssert(!m_ecsqlOptions.ContainsI(OptionsExp::READONLYPROPERTIESAREUPDATABLE_OPTION) && "Invalid JsonUpdater::Option: The ECSQLOPTION 'ReadonlyPropertiesAreUpdatable' may not be specified. Always use the JsonUpdater::ReadonlyPropertiesOption argument instead.");
        m_isValid = false;
        return;
        }

    if (readonlyPropertiesOption == JsonUpdater::ReadonlyPropertiesOption::Update)
        {
        if (!m_ecsqlOptions.empty())
            m_ecsqlOptions.append(" ");

        m_ecsqlOptions.append(OptionsExp::READONLYPROPERTIESAREUPDATABLE_OPTION);
        }

    m_isValid = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle      09/2017
//+---------------+---------------+---------------+---------------+---------------+------
JsonUpdater::JsonUpdater(ECDbCR ecdb, ECClassCR ecClass, ECCrudWriteToken const* writeToken, Options const& options) : m_ecdb(ecdb), m_ecClass(ecClass), m_options(options)
    {
    m_isValid = Initialize(nullptr, writeToken) == SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle      09/2017
//+---------------+---------------+---------------+---------------+---------------+------
JsonUpdater::JsonUpdater(ECDbCR ecdb, ECClassCR ecClass, bvector<Utf8CP> const& props, ECCrudWriteToken const* writeToken, Options const& options) : m_ecdb(ecdb), m_ecClass(ecClass), m_options(options)
    {
    m_isValid = Initialize(&props, writeToken) == SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonUpdater::Initialize(bvector<Utf8CP> const* propNames, ECCrudWriteToken const* writeToken)
    {
    if (!m_options.IsValid())
        return ERROR;

    m_jsonClassName = ECJsonUtilities::FormatClassName(m_ecClass);

    Utf8String ecsql("UPDATE ONLY ");
    ecsql.append(m_ecClass.GetECSqlName()).append(" SET ");

    bvector<ECPropertyCP> props;
    if (propNames != nullptr)
        {
        if (propNames == nullptr)
            {
            LOG.errorv("JsonUpdater initialization failure. The ECClass '%s' does not have any properties.", m_ecClass.GetFullName());
            return ERROR;
            }

        for (Utf8CP propName : *propNames)
            {
            ECPropertyCP prop = m_ecClass.GetPropertyP(propName);
            if (prop == nullptr)
                return ERROR;

            props.push_back(prop);
            }
        }
    else
        {
        for (ECPropertyCP prop : m_ecClass.GetProperties(true))
            {
            props.push_back(prop);
            }

        if (props.empty())
            {
            LOG.errorv("JsonUpdater initialization failure. The list of ECProperties of ECClass '%s' is empty.", m_ecClass.GetFullName());
            return ERROR;
            }
        }

    uint32_t parameterIndex = 1;

    bool isFirstProp = true;
    for (ECPropertyCP prop : props)
        {
        //readonly props are ignored if the respective option is specified
        if (prop->GetIsReadOnly() && m_options.GetReadonlyPropertiesOption() == ReadonlyPropertiesOption::Ignore)
            {
            //add a "SkipBinding" BindingInfo so that at update time, the adapter knows to ignore a JSON member of this name
            m_bindingMap[prop->GetName().c_str()] = BindingInfo();
            continue;
            }

        if (!isFirstProp)
            ecsql.append(",");

        ecsql.append("[").append(prop->GetName()).append("]=?");
        m_bindingMap[prop->GetName().c_str()] = BindingInfo(parameterIndex, *prop);
        parameterIndex++;

        isFirstProp = false;
        }


    ecsql.append(" WHERE " ECDBSYS_PROP_ECInstanceId "=?");
    m_idParameterIndex = parameterIndex;

    if (!m_options.GetECSqlOptions().empty())
        ecsql.append(" ECSQLOPTIONS ").append(m_options.GetECSqlOptions());

    return ECSqlStatus::Success == m_statement.Prepare(m_ecdb, ecsql.c_str(), writeToken) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                09/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonUpdater::Update(ECInstanceId instanceId, JsonValueCR json) const
    {
    if (!m_isValid)
        return BE_SQLITE_ERROR;

    if (json.isNull() || !json.isObject() || json.size() == 0)
        {
        LOG.errorv("JsonUpdater failure. The JSON to update must be a non-empty object, but it was: %s", json.ToString().c_str());
        return BE_SQLITE_ERROR;
        }

    m_statement.ClearBindings();

    BeAssert(json.isObject());
    for (Json::Value::iterator it = json.begin(); it != json.end(); it++)
        {
        Utf8CP memberName = it.memberName();
        JsonValueCR memberJson = *it;

        auto bindingMapLookupIt = m_bindingMap.find(memberName);
        if (bindingMapLookupIt == m_bindingMap.end())
            {
            if (ECJsonSystemNames::IsTopLevelSystemMember(memberName))
                {
                if (m_options.GetSystemPropertiesOption() == JsonUpdater::SystemPropertiesOption::Ignore)
                    continue;

                LOG.errorv("JsonUpdater failure. ECJSON System member '%s' not allowed in the JSON. System properties cannot be updated. Input JSON: %s",
                           memberName, json.ToString().c_str());
                return BE_SQLITE_ERROR;
                }

            LOG.errorv("JsonUpdater failure. The JSON member '%s' does not match with a property in ECClass '%s'. Input JSON: %s",
                           memberName, m_ecClass.GetFullName(), json.ToString().c_str());
            return BE_SQLITE_ERROR;
            }

        BeAssert(!ECJsonSystemNames::IsTopLevelSystemMember(memberName));
        BindingInfo const& bindingInfo = bindingMapLookupIt->second;

        if (bindingInfo.SkipBinding())
            continue;

        if (ECSqlStatus::Success != JsonECSqlBinder::BindValue(m_statement.GetBinder((int) bindingInfo.GetParameterIndex()), memberJson, bindingInfo.GetProperty(), m_ecdb.GetClassLocater()))
            {
            LOG.errorv("JsonUpdater failure. Could not bind JSON member '%s' to parameter %" PRIu32 " for ECJSON %s. Underlying ECSQL: %s",
                       memberName, bindingInfo.GetParameterIndex(), json.ToString().c_str(), m_statement.GetECSql());
            return BE_SQLITE_ERROR;
            }
        }

    if (ECSqlStatus::Success != m_statement.BindId((int) m_idParameterIndex, instanceId))
        {
        LOG.errorv("JsonUpdater failure. Could not bind ECInstanceId %s to parameter %" PRIu32 ". Underlying ECSQL: %s",
                   instanceId.ToString().c_str(), m_idParameterIndex, m_statement.GetECSql());

        return BE_SQLITE_ERROR;
        }

    //now execute statement
    const DbResult stepStatus = m_statement.Step();

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_statement.Reset();
    m_statement.ClearBindings();

    return BE_SQLITE_DONE == stepStatus ? BE_SQLITE_OK : stepStatus;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Krischan.Eberle                09/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonUpdater::Update(ECInstanceId instanceId, RapidJsonValueCR json) const
    {
    if (!m_isValid)
        return BE_SQLITE_ERROR;

    if (json.IsNull() || !json.IsObject() || json.GetObject().ObjectEmpty())
        {
        rapidjson::StringBuffer jsonStr;
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
        json.Accept(writer);
        LOG.errorv("JsonUpdater failure. The JSON to update must be a non-empty object, but it was: %s", jsonStr.GetString());
        return BE_SQLITE_ERROR;
        }

    m_statement.ClearBindings();

    BeAssert(json.IsObject());
    for (rapidjson::Value::ConstMemberIterator it = json.MemberBegin(); it != json.MemberEnd(); ++it)
        {
        Utf8CP memberName = it->name.GetString();
        RapidJsonValueCR memberJson = it->value;

        auto bindingMapLookupIt = m_bindingMap.find(memberName);
        if (bindingMapLookupIt == m_bindingMap.end())
            {
            if (ECJsonSystemNames::IsTopLevelSystemMember(memberName))
                {
                if (m_options.GetSystemPropertiesOption() == JsonUpdater::SystemPropertiesOption::Ignore)
                    continue;

                rapidjson::StringBuffer jsonStr;
                rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
                memberJson.Accept(writer);

                LOG.errorv("JsonUpdater failure. ECJSON System member '%s' not allowed in the JSON. System properties cannot be updated. Input JSON: %s",
                           memberName, jsonStr.GetString());
                return BE_SQLITE_ERROR;
                }

            rapidjson::StringBuffer jsonStr;
            rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
            memberJson.Accept(writer);

            LOG.errorv("JsonUpdater failure. The JSON member '%s' does not match with a property in ECClass '%s'. Input JSON: %s",
                       memberName, m_ecClass.GetFullName(), jsonStr.GetString());
            return BE_SQLITE_ERROR;
            }

        BeAssert(!ECJsonSystemNames::IsTopLevelSystemMember(memberName));
        BindingInfo const& bindingInfo = bindingMapLookupIt->second;

        if (bindingInfo.SkipBinding())
            continue;

        if (ECSqlStatus::Success != JsonECSqlBinder::BindValue(m_statement.GetBinder((int) bindingInfo.GetParameterIndex()), memberJson, bindingInfo.GetProperty(), m_ecdb.GetClassLocater()))
            {
            rapidjson::StringBuffer jsonStr;
            rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
            memberJson.Accept(writer);

            LOG.errorv("JsonUpdater failure. Could not bind JSON member '%s' to parameter %" PRIu32 " for ECJSON %s. Underlying ECSQL: %s",
                       memberName, bindingInfo.GetParameterIndex(), jsonStr.GetString(), m_statement.GetECSql());
            return BE_SQLITE_ERROR;
            }
        }

    if (ECSqlStatus::Success != m_statement.BindId((int) m_idParameterIndex, instanceId))
        {
        LOG.errorv("JsonUpdater failure. Could not bind ECInstanceId %s to parameter %" PRIu32 ". Underlying ECSQL: %s",
                   instanceId.ToString().c_str(), m_idParameterIndex, m_statement.GetECSql());

        return BE_SQLITE_ERROR;
        }

    //now execute statement
    const DbResult stepStatus = m_statement.Step();

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_statement.Reset();
    m_statement.ClearBindings();

    return BE_SQLITE_DONE == stepStatus ? BE_SQLITE_OK : stepStatus;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
