/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/UserSettings.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include "LocalizationHelper.h"
#include "LoggingHelper.h"

#ifdef USER_SETTINGS_DB
#define USER_SETTINGS_DB_NAME           "RulesEngine.UserSettings.db"
#define USER_SETTINGS_DB_VERSION_MAJOR  1
#define USER_SETTINGS_DB_VERSION_MINOR  0
#define USER_SETTINGS_TABLE_NAME        "UserSettings"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettings::HasValue(Utf8CP settingId) const
    {
    if (!m_settingsDb.IsDbOpen())
        {
        BeAssert(false);
        return false;
        }

    Utf8CP sql = "SELECT 1 FROM [" USER_SETTINGS_TABLE_NAME "] "
                 "WHERE [RulesetId] = ? AND [SettingId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_settingsDb.GetDbFile(), sql))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindText(1, m_rulesetId.c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, settingId, Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value GetJsonFromDbValue(DbValue const& value)
    {
    switch (value.GetValueType())
        {
        case DbValueType::TextVal: return value.GetValueText();
        case DbValueType::IntegerVal: return value.GetValueInt64();
        case DbValueType::FloatVal: return value.GetValueDouble();
        }
    return Json::Value();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UserSettings::GetValue(Utf8CP settingId, bool isText) const
    {
    if (!m_settingsDb.IsDbOpen())
        {
        BeAssert(false);
        return Json::Value();
        }

    Utf8String sql = "SELECT [";
    sql.append(isText ? "StrValue" : "IntValue").append("] ");
    sql.append("FROM [" USER_SETTINGS_TABLE_NAME "] ");
    sql.append("WHERE [RulesetId] = ? AND [SettingId] = ?");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_settingsDb.GetDbFile(), sql.c_str()))
        {
        BeAssert(false);
        return Json::Value();
        }

    stmt->BindText(1, m_rulesetId.c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, settingId, Statement::MakeCopy::No);

    Json::Value json;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        Json::Value value = GetJsonFromDbValue(stmt->GetDbValue(0));
        if (json.isNull())
            {
            // swap to avoid copy
            json.swap(value);
            }
        else
            {
            if (!json.isArray())
                {
                // convert json to array
                Json::Value jsonArray(Json::arrayValue);
                jsonArray.append(json);
                jsonArray.swap(json);
                }            
            json.append(value);
            }
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::DeleteValues(Utf8CP settingId)
    {
    if (!m_settingsDb.IsDbOpen())
        {
        BeAssert(false);
        return;
        }

    Utf8CP sql = "DELETE FROM [" USER_SETTINGS_TABLE_NAME "] WHERE [RulesetId] = ? AND [SettingId] = ?";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_settingsDb.GetDbFile(), sql))
        {
        BeAssert(false);
        return;
        }

    stmt->BindText(1, m_rulesetId.c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, settingId, Statement::MakeCopy::No);
    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void BindValues(Statement& stmt, Utf8CP rulesetId, Utf8CP settingId, JsonValueCR value)
    {
    if (value.isBool())
        stmt.BindBoolean(1, value.asBool());
    else if (value.isDouble())
        stmt.BindDouble(1, value.asDouble());
    else if (value.isString())
        stmt.BindText(1, value.asCString(), Statement::MakeCopy::No);
    else if (value.isIntegral())
        stmt.BindInt64(1, value.asInt64());

    stmt.BindText(2, rulesetId, Statement::MakeCopy::No);
    stmt.BindText(3, settingId, Statement::MakeCopy::No);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::InsertValue(Utf8CP settingId, JsonValueCR settingValue, bool updateIfExists, bool notifyListeners)
    {
    if (!m_settingsDb.IsDbOpen())
        {
        BeAssert(false);
        return;
        }
    
    if (!m_isInitializing)
        {
        Json::Value currentValue = GetValue(settingId, settingValue.isString());
        if (!currentValue.isNull() && currentValue == settingValue)
            return;
        }

    Utf8CP columnName = settingValue.isString() ? "StrValue" : "IntValue";

    Utf8String updateSql = "UPDATE [" USER_SETTINGS_TABLE_NAME "] SET [";
    updateSql.append(columnName).append("] = ?) ");
    updateSql.append("WHERE [RulesetId] = ? AND [SettingId] = ?");

    Utf8String insertSql = "INSERT INTO [" USER_SETTINGS_TABLE_NAME "] ([";
    insertSql.append(columnName).append("], [RulesetId], [SettingId]) VALUES (?, ?, ?)");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_settingsDb.GetDbFile(), insertSql.c_str()))
        {
        BeAssert(false);
        return;
        }
    BindValues(*stmt, m_rulesetId.c_str(), settingId, settingValue);
    stmt->Step();

    if (updateIfExists && 0 == m_settingsDb.GetLastInsertRowId())
        {
        // no value inserted - have to update existing one
        if (BE_SQLITE_OK != m_statements.GetPreparedStatement(stmt, *m_settingsDb.GetDbFile(), updateSql.c_str()))
            {
            BeAssert(false);
            return;
            }
        BindValues(*stmt, m_rulesetId.c_str(), settingId, settingValue);
        stmt->Step();
        }    
    
    if (notifyListeners && !m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), settingId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingValue(Utf8CP id, Utf8CP value)
    {
    if (!m_isInitializing)
        {
        Utf8String currentValue = GetSettingValue(id);
        if (currentValue.Equals(value))
            return;
        }
    InsertValue(id, value, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingIntValue(Utf8CP id, int64_t value)
    {    
    if (!m_isInitializing)
        {
        int64_t currentValue = GetSettingIntValue(id);
        if (currentValue == value)
            return;
        }
    InsertValue(id, value, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values)
    {    
    if (!m_isInitializing)
        {
        bvector<int64_t> currentValues = GetSettingIntValues(id);
        if (currentValues == values)
            return;
        }

    DeleteValues(id);

    for (int64_t value : values)
        InsertValue(id, value, false, false);
    
    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingBoolValue(Utf8CP id, bool value)
    {
    if (!m_isInitializing)
        {
        bool currentValue = GetSettingBoolValue(id);
        if (currentValue == value)
            return;
        }
    InsertValue(id, value, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UserSettings::_GetSettingValue(Utf8CP id) const
    {
    static Utf8String s_defaultValue = "";
    if (!m_settingsDb.IsDbOpen())
        {
        BeAssert(false);
        return s_defaultValue;
        }
    Json::Value jsonValue = GetValue(id, true);
    return jsonValue.isString() ? jsonValue.asCString() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t UserSettings::_GetSettingIntValue(Utf8CP id) const
    {
    static int64_t s_defaultValue = 0;
    if (!m_settingsDb.IsDbOpen())
        {
        BeAssert(false);
        return s_defaultValue;
        }    
    Json::Value value = GetValue(id, false);
    return (value.isConvertibleTo(Json::intValue) || value.isConvertibleTo(Json::uintValue)) ? value.asInt64() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<int64_t> UserSettings::_GetSettingIntValues(Utf8CP id) const
    {
    bvector<int64_t> values;
    if (!m_settingsDb.IsDbOpen())
        {
        BeAssert(false);
        return values;
        }
    
    Json::Value jsonValues = GetValue(id, false);
    if (!jsonValues.isArray())
        return values;

    for (Json::ArrayIndex i = 0; i < jsonValues.size(); ++i)
        values.push_back(jsonValues[i].asInt64());
    return values;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettings::_GetSettingBoolValue(Utf8CP id) const
    {
    static bool s_defaultValue = false;
    if (!m_settingsDb.IsDbOpen())
        {
        BeAssert(false);
        return s_defaultValue;
        }
    Json::Value value = GetValue(id, false);
    return value.isConvertibleTo(Json::booleanValue) ? value.asBool() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettings::_HasSetting(Utf8CP id) const {return HasValue(id);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::AddValues(JsonValueR groupListJson) const
    {
    for (Json::ArrayIndex i = 0; i < groupListJson.size(); i++)
        {
        JsonValueR groupJson = groupListJson[i];
        if (groupJson.isMember("Items"))
            {
            for (Json::ArrayIndex j = 0; j < groupJson["Items"].size(); j++)
                {
                JsonValueR itemJson = groupJson["Items"][j];
                Utf8CP options = itemJson["Options"].asCString();
                if (0 == strcmp("StringValue", options))
                    itemJson["Value"] = GetSettingValue(itemJson["Id"].asCString()).c_str();
                else if (0 == strcmp("IntValue", options))
                    itemJson["Value"] = GetSettingIntValue(itemJson["Id"].asCString());
                else
                    itemJson["Value"] = GetSettingBoolValue(itemJson["Id"].asCString());
                }
            }
        if (groupJson.isMember("NestedGroups"))
            AddValues(groupJson["NestedGroups"]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UserSettings::_GetPresentationInfo() const
    {
    if (!m_settingsDb.IsDbOpen())
        {
        BeAssert(false);
        return Json::Value::null;
        }

    Json::Value json = m_presentationInfo;
    AddValues(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UserSettings::GetLocalizedLabel(Utf8StringCR nonLocalizedLabel) const
    {
    if (nullptr == m_localizationProvider)
        {
        LoggingHelper::LogMessage(Log::Localization, "Localization is not available as the localization provider is not set", NativeLogging::LOG_WARNING, true);
        return nonLocalizedLabel;
        }

    Utf8String localizedLabel = nonLocalizedLabel;
    LocalizationHelper helper(*m_localizationProvider);
    return helper.LocalizeString(localizedLabel) ? localizedLabel : nonLocalizedLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::InitFrom(UserSettingsGroupList const& rules)
    {
    if (!rules.empty() && !m_settingsDb.IsDbOpen())
        {
        BeAssert(false);
        return;
        }

    m_isInitializing = true;
    m_presentationInfo.clear();
    InitFrom(rules, m_presentationInfo);
    m_isInitializing = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::InitFrom(UserSettingsGroupList const& rules, JsonValueR presentationInfo)
    {
    presentationInfo = Json::Value(Json::arrayValue);
    for (UserSettingsGroupCP group : rules)
        {
        Json::Value groupJson(Json::objectValue);
        groupJson["Label"] = GetLocalizedLabel(group->GetCategoryLabel());
        if (!group->GetSettingsItems().empty())
            groupJson["Items"] = Json::Value(Json::arrayValue);

        for (UserSettingsItemCP item : group->GetSettingsItems())
            {
            if (!HasValue(item->GetId().c_str()))
                {
                // save the value in local state
                if (item->GetOptions().Equals("StringValue"))
                    {
                    SetSettingValue(item->GetId().c_str(), item->GetDefaultValue().c_str());
                    }
                else if (item->GetOptions().Equals("IntValue"))
                    {
                    int value = atoi(item->GetDefaultValue().c_str());
                    SetSettingIntValue(item->GetId().c_str(), value);
                    }
                else
                    {
                    bool value = item->GetDefaultValue().EqualsI("true") || item->GetDefaultValue().Equals("1");
                    SetSettingBoolValue(item->GetId().c_str(), value);
                    }
                }

            // create the presentation info
            Json::Value itemPresentationInfo(Json::objectValue);
            itemPresentationInfo["Id"] = item->GetId();
            itemPresentationInfo["Label"] = GetLocalizedLabel(item->GetLabel());
            itemPresentationInfo["Options"] = item->GetOptions().empty() ? "TrueFalse" : item->GetOptions();
            groupJson["Items"].append(itemPresentationInfo);
            }

        if (!group->GetNestedSettings().empty())
            InitFrom(group->GetNestedSettings(), groupJson["NestedGroups"]);

        presentationInfo.append(groupJson);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsManager::UserSettingsManager(BeFileNameCR temporaryDirectory, IUserSettingsChangeListener const* changeListener)
    : m_statements(10), m_localizationProvider(nullptr), m_changeListener(changeListener)
    {
    BeSQLiteLib::Initialize(temporaryDirectory);
    InitSettingsDb(temporaryDirectory);
    InitSettingsTable();
    }

static PropertySpec s_versionPropertySpec("Version", "UserSettings");
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static BeVersion GetSettingsDbVersion(BeSQLite::Db const& db)
    {
    Utf8String versionStr;
    if (BE_SQLITE_ROW != db.QueryProperty(versionStr, s_versionPropertySpec))
        return BeVersion();

    return BeVersion(versionStr.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::InitSettingsDb(BeFileNameCR localStateDirectory)
    {
    m_settingsDbPath = BeFileName(localStateDirectory).AppendToPath(WString(USER_SETTINGS_DB_NAME, true).c_str());

    DbResult result = BE_SQLITE_ERROR;
    if (m_settingsDbPath.DoesPathExist())
        {
        result = m_settingsDb.OpenBeSQLiteDb(m_settingsDbPath, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes, new BusyRetry()));
        if (BE_SQLITE_OK == result && GetSettingsDbVersion(m_settingsDb).GetMajor() < USER_SETTINGS_DB_VERSION_MAJOR)
            {
            // if the existing cache version is too old, simply delete the old cache and create a new one
            m_settingsDb.CloseDb();
            m_settingsDbPath.BeDeleteFile();
            result = BE_SQLITE_ERROR_ProfileTooOld;
            }
        }
    if (BE_SQLITE_OK != result)
        {
        result = m_settingsDb.CreateNewDb(m_settingsDbPath, BeGuid(), Db::CreateParams(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8, 
            true, DefaultTxn::Yes, new BusyRetry()));
    
        // save the cache version
        static BeVersion s_cacheVersion(USER_SETTINGS_DB_VERSION_MAJOR, USER_SETTINGS_DB_VERSION_MINOR);
        m_settingsDb.SaveProperty(s_versionPropertySpec, s_cacheVersion.ToString(), nullptr, 0);
        }
    if (BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::InitSettingsTable()
    {    
    // create the table
    if (!m_settingsDb.TableExists(USER_SETTINGS_TABLE_NAME))
        {
        Utf8CP ddl = "[Id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
                     "[RulesetId] TEXT NOT NULL, "
                     "[SettingId] TEXT NOT NULL, "
                     "[StrValue] TEXT, "
                     "[IntValue] INTEGER ";
        m_settingsDb.CreateTable(USER_SETTINGS_TABLE_NAME, ddl);
        m_settingsDb.ExecuteSql("CREATE INDEX [IX_UserSettings] ON [" USER_SETTINGS_TABLE_NAME "]([RulesetId],[SettingId])");
        }
    m_settingsDb.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsManager::~UserSettingsManager()
    {
    for (auto iter : m_settings)
        delete iter.second;

    m_statements.Empty();
    //m_settingsDb.SaveChanges();
    m_settingsDb.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::SetLocalizationProvider(ILocalizationProvider const* provider)
    {
    m_localizationProvider = provider;
    for (auto iter : m_settings)
        iter.second->SetLocalizationProvider(m_localizationProvider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettings const& UserSettingsManager::GetSettings(Utf8StringCR rulesetId) const {return GetSettingsR(rulesetId);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettings& UserSettingsManager::GetSettingsR(Utf8StringCR rulesetId) const
    {
    auto iter = m_settings.find(rulesetId);
    if (m_settings.end() == iter)
        {
        UserSettings* settings = new UserSettings(const_cast<Db&>(m_settingsDb), const_cast<StatementCache&>(m_statements),
            rulesetId.c_str(), this);
        settings->SetLocalizationProvider(m_localizationProvider);
        iter = m_settings.Insert(rulesetId, settings).first;
        }
    return *iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::_OnRulesetDispose(PresentationRuleSetCR ruleset)
    {
    auto iter = m_settings.find(ruleset.GetRuleSetId());
    if (m_settings.end() != iter)
        {
        delete iter->second;
        m_settings.erase(iter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    if (nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(rulesetId, settingId);
    }

#define ECSCHEMA_UserSettings "" \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
    "<ECSchema schemaName=\"PresentationRulesEngine_UserSettings\" alias=\"settings\" version=\"1.0\" " \
    "          xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">" \
    "    <ECEntityClass typeName=\"UserSettings\">" \
    "        <ECCustomAttributes>" \
    "            <ClassMap xmlns=\"ECDbMap.02.00\">" \
    "                <MapStrategy>ExistingTable</MapStrategy>" \
    "                <TableName>PresentationRulesEngine_UserSettings." USER_SETTINGS_TABLE_NAME "</TableName>" \
    "            </ClassMap>" \
    "        </ECCustomAttributes>" \
    "        <ECProperty propertyName=\"RulesetId\" typeName=\"string\" />" \
    "        <ECProperty propertyName=\"SettingId\" typeName=\"string\" />" \
    "        <ECProperty propertyName=\"StrValue\" typeName=\"string\" />" \
    "        <ECProperty propertyName=\"IntValue\" typeName=\"long\" />" \
    "    </ECEntityClass>" \
    "</ECSchema>"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::InitForConnection(ECDbR connection)
    {
    if (m_attachedConnections.end() != m_attachedConnections.find(connection.GetDbGuid()))
        return;

    DbResult result = connection.AttachDb(m_settingsDbPath.GetNameUtf8().c_str(), "PresentationRulesEngine_UserSettings");
    if (BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return;
        }
    m_attachedConnections.insert(connection.GetDbGuid());
    
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(const_cast<IECSchemaLocaterR>((IECSchemaLocaterCR)connection.Schemas()));

    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(schema, ECSCHEMA_UserSettings, *schemaContext)
        || SUCCESS != connection.Schemas().ImportSchemas(schemaContext->GetCache().GetSchemas()))
        {
        BeAssert(false);
        TerminateForConnection(connection);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::TerminateForConnection(ECDbR connection)
    {
    if (m_attachedConnections.end() == m_attachedConnections.find(connection.GetDbGuid()))
        return;

    connection.DetachDb("PresentationRulesEngine_UserSettings");
    m_attachedConnections.erase(connection.GetDbGuid());
    }
#endif

#ifdef USER_SETTINGS_LOCALSTATE
#define USER_SETTINGS_NAMESPACE "RulesEngine.UserSettings"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingValue(Utf8CP id, Utf8CP value)
    {
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return;
        }

    if (!m_isInitializing)
        {
        Utf8String currentValue = GetSettingValue(id);
        if (currentValue.Equals(value))
            return;
        }

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), value);

    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingIntValue(Utf8CP id, int64_t value)
    {
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return;
        }
    
    if (!m_isInitializing)
        {
        int64_t currentValue = GetSettingIntValue(id);
        if (currentValue == value)
            return;
        }

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), value);
    
    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<int64_t> VectorFromJsonArray(JsonValueCR json)
    {
    BeAssert(json.isArray());
    bvector<int64_t> vec;
    for (Json::ArrayIndex i = 0; i < json.size(); ++i)
        vec.push_back(json[i].asInt64());
    return vec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value JsonArrayFromVector(bvector<int64_t> const& vec)
    {
    Json::Value json(Json::arrayValue);
    for (int64_t v : vec)
        json.append(v);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values)
    {    
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return;
        }
    
    if (!m_isInitializing)
        {
        bvector<int64_t> currentValues = GetSettingIntValues(id);
        if (currentValues == values)
            return;
        }

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), JsonArrayFromVector(values));
    
    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingBoolValue(Utf8CP id, bool value)
    {
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return;
        }

    
    if (!m_isInitializing)
        {
        bool currentValue = GetSettingBoolValue(id);
        if (currentValue == value)
            return;
        }

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), value);
    
    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UserSettings::_GetSettingValue(Utf8CP id) const
    {
    static Utf8String s_defaultValue = "";
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return s_defaultValue;
        }
    
    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    Json::Value value = m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    return value.isString() ? value.asCString() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t UserSettings::_GetSettingIntValue(Utf8CP id) const
    {
    static int64_t s_defaultValue = 0;
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return s_defaultValue;
        }
    
    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    Json::Value value = m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    return (value.isConvertibleTo(Json::intValue) || value.isConvertibleTo(Json::uintValue)) ? value.asInt64() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<int64_t> UserSettings::_GetSettingIntValues(Utf8CP id) const
    {
    bvector<int64_t> values;
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return values;
        }
    
    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    Json::Value jsonValues = m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    if (!jsonValues.isArray())
        return values;

    return VectorFromJsonArray(jsonValues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettings::_GetSettingBoolValue(Utf8CP id) const
    {
    static bool s_defaultValue = false;
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return s_defaultValue;
        }
    
    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    Json::Value value = m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    return value.isConvertibleTo(Json::booleanValue) ? value.asBool() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettings::_HasSetting(Utf8CP id) const
    {
    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    return !m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str()).isNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::AddValues(JsonValueR groupListJson) const
    {
    for (Json::ArrayIndex i = 0; i < groupListJson.size(); i++)
        {
        JsonValueR groupJson = groupListJson[i];
        if (groupJson.isMember("Items"))
            {
            for (Json::ArrayIndex j = 0; j < groupJson["Items"].size(); j++)
                {
                JsonValueR itemJson = groupJson["Items"][j];
                Utf8CP options = itemJson["Options"].asCString();
                if (0 == strcmp("StringValue", options))
                    itemJson["Value"] = GetSettingValue(itemJson["Id"].asCString()).c_str();
                else if (0 == strcmp("IntValue", options))
                    itemJson["Value"] = GetSettingIntValue(itemJson["Id"].asCString());
                else
                    itemJson["Value"] = GetSettingBoolValue(itemJson["Id"].asCString());
                }
            }
        if (groupJson.isMember("NestedGroups"))
            AddValues(groupJson["NestedGroups"]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UserSettings::_GetPresentationInfo() const
    {
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return Json::Value::GetNull();
        }

    Json::Value json = m_presentationInfo;
    AddValues(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UserSettings::GetLocalizedLabel(Utf8StringCR nonLocalizedLabel) const
    {
    if (nullptr == m_localizationProvider)
        {
        LoggingHelper::LogMessage(Log::Localization, "Localization is not available as the localization provider is not set", NativeLogging::LOG_WARNING, true);
        return nonLocalizedLabel;
        }

    Utf8String localizedLabel = nonLocalizedLabel;
    LocalizationHelper helper(*m_localizationProvider);
    return helper.LocalizeString(localizedLabel) ? localizedLabel : nonLocalizedLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::InitFrom(UserSettingsGroupList const& rules)
    {
    if (!rules.empty() && nullptr == m_localState)
        {
        BeAssert(false);
        return;
        }

    m_isInitializing = true;
    m_presentationInfo.clear();
    InitFrom(rules, m_presentationInfo);
    m_isInitializing = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::InitFrom(UserSettingsGroupList const& rules, JsonValueR presentationInfo)
    {
    presentationInfo = Json::Value(Json::arrayValue);
    for (UserSettingsGroupCP group : rules)
        {
        Json::Value groupJson(Json::objectValue);
        groupJson["Label"] = GetLocalizedLabel(group->GetCategoryLabel());
        if (!group->GetSettingsItems().empty())
            groupJson["Items"] = Json::Value(Json::arrayValue);

        for (UserSettingsItemCP item : group->GetSettingsItems())
            {
            Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), item->GetId().c_str());
            if (m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str()).isNull())
                {
                // save the value in local state
                if (item->GetOptions().Equals("StringValue"))
                    {
                    SetSettingValue(item->GetId().c_str(), item->GetDefaultValue().c_str());
                    }
                else if (item->GetOptions().Equals("IntValue"))
                    {
                    int value = atoi(item->GetDefaultValue().c_str());
                    SetSettingIntValue(item->GetId().c_str(), value);
                    }
                else
                    {
                    bool value = item->GetDefaultValue().EqualsI("true") || item->GetDefaultValue().Equals("1");
                    SetSettingBoolValue(item->GetId().c_str(), value);
                    }
                }

            // create the presentation info
            Json::Value itemPresentationInfo(Json::objectValue);
            itemPresentationInfo["Id"] = item->GetId();
            itemPresentationInfo["Label"] = GetLocalizedLabel(item->GetLabel());
            itemPresentationInfo["Options"] = item->GetOptions().empty() ? "TrueFalse" : item->GetOptions();
            groupJson["Items"].append(itemPresentationInfo);
            }

        if (!group->GetNestedSettings().empty())
            InitFrom(group->GetNestedSettings(), groupJson["NestedGroups"]);

        presentationInfo.append(groupJson);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsManager::UserSettingsManager(BeFileNameCR, IUserSettingsChangeListener const* changeListener) 
    : m_localState(nullptr), m_localizationProvider(nullptr), m_changeListener(changeListener)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsManager::~UserSettingsManager()
    {
    for (auto iter : m_settings)
        delete iter.second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::SetLocalState(IJsonLocalState* localState)
    {
    m_localState = localState;
    for (auto iter : m_settings)
        iter.second->SetLocalState(m_localState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::SetLocalizationProvider(ILocalizationProvider const* provider)
    {
    m_localizationProvider = provider;
    for (auto iter : m_settings)
        iter.second->SetLocalizationProvider(m_localizationProvider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettings& UserSettingsManager::GetSettings(Utf8StringCR rulesetId) const
    {
    auto iter = m_settings.find(rulesetId);
    if (m_settings.end() == iter)
        {
        UserSettings* settings = new UserSettings(rulesetId.c_str(), this);
        settings->SetLocalState(m_localState);
        settings->SetLocalizationProvider(m_localizationProvider);
        iter = m_settings.Insert(rulesetId, settings).first;
        }
    return *iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::_OnRulesetDispose(PresentationRuleSetCR ruleset)
    {
    auto iter = m_settings.find(ruleset.GetRuleSetId());
    if (m_settings.end() != iter)
        {
        delete iter->second;
        m_settings.erase(iter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    if (nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(rulesetId, settingId);
    }
#endif
