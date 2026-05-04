/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

static Utf8CP const KEY_SESSIONS = "ec_schemaSessions";
static Utf8CP const KEY_SCHEMA_XML_PREFIX = "ec_schemaXml_";

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static Utf8String SchemaXmlKey(Utf8StringCR fullSchemaName, bool isDynamic, int sessionId)
    {
    Utf8String key(KEY_SCHEMA_XML_PREFIX);
    key.append(fullSchemaName);
    if (isDynamic)
        key.append(Utf8PrintfString("_%d", sessionId));
    return key;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static Utf8String SchemaImportOptionsToString(SchemaManager::SchemaImportOptions options)
    {
    Utf8String str;
    auto val = static_cast<int>(options);
    if (val == 0)
        return "None";

    bool first = true;
    auto append = [&](Utf8CP name) { if (!first) str.append("|"); str.append(name); first = false; };

    if (val & static_cast<int>(SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues))
        append("DoNotFailSchemaValidationForLegacyIssues");
    if (val & static_cast<int>(SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade))
        append("DisallowMajorSchemaUpgrade");
    if (val & static_cast<int>(SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications))
        append("DoNotFailForDeletionsOrModifications");
    if (val & static_cast<int>(SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade))
        append("AllowDataTransformDuringSchemaUpgrade");
    if (val & static_cast<int>(SchemaManager::SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas))
        append("AllowMajorSchemaUpgradeForDynamicSchemas");

    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static SchemaManager::SchemaImportOptions SchemaImportOptionsFromString(Utf8StringCR str)
    {
    if (str.empty() || str.Equals("None"))
        return SchemaManager::SchemaImportOptions::None;

    int val = 0;
    if (str.Contains("DoNotFailSchemaValidationForLegacyIssues"))
        val |= static_cast<int>(SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues);
    if (str.Contains("DisallowMajorSchemaUpgrade"))
        val |= static_cast<int>(SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade);
    if (str.Contains("DoNotFailForDeletionsOrModifications"))
        val |= static_cast<int>(SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications);
    if (str.Contains("AllowDataTransformDuringSchemaUpgrade"))
        val |= static_cast<int>(SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
    if (str.Contains("AllowMajorSchemaUpgradeForDynamicSchemas"))
        val |= static_cast<int>(SchemaManager::SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas);

    return static_cast<SchemaManager::SchemaImportOptions>(val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaManager::Sessions::Record(bvector<ECSchemaCP> const& schemas, SchemaImportOptions options)
    {
    if (!m_enabled)
        return SUCCESS;

    // Load existing session log
    Utf8String sessionsJson;
    BeJsDocument sessionsDoc;
    if (BE_SQLITE_ROW == m_ecdb.QueryBriefcaseLocalValue(sessionsJson, KEY_SESSIONS))
        sessionsDoc.Parse(sessionsJson);

    if (!sessionsDoc.isArray())
        sessionsDoc.SetEmptyArray();

    // Determine next session id
    int nextId = 1;
    sessionsDoc.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst entry)
        {
        if (entry.isMember("id"))
            {
            int id = entry["id"].GetInt();
            if (id >= nextId)
                nextId = id + 1;
            }
        return false;
        });

    // Build new session entry
    auto session = sessionsDoc.appendValue();
    session.SetEmptyObject();
    session["id"] = nextId;
    session["dateTime"] = DateTime::GetCurrentTime().ToTimestampString();
    session["importFlags"] = SchemaImportOptionsToString(options);

    auto schemasArr = session["schemas"];
    schemasArr.SetEmptyArray();

    // Store schema XML and record schema names
    for (auto schemaCP : schemas)
        {
        if (schemaCP == nullptr)
            continue;

        ECSchemaCP schema = schemaCP;
        Utf8String fullName = schema->GetFullSchemaName();
        bool isDynamic = schema->IsDynamicSchema();

        // Add schema name to session entry
        schemasArr.appendValue() = fullName;

        // Store XML — always for dynamic, only if not already stored for non-dynamic
        Utf8String xmlKey = SchemaXmlKey(fullName, isDynamic, nextId);

        if (isDynamic)
            {
            // Always store XML for dynamic schemas (may differ even at same version)
            Utf8String xml;
            if (SchemaWriteStatus::Success == schema->WriteToXmlString(xml))
                m_ecdb.SaveBriefcaseLocalValue(xmlKey.c_str(), xml);
            }
        else
            {
            // Only store if not already present
            Utf8String existing;
            if (BE_SQLITE_ROW != m_ecdb.QueryBriefcaseLocalValue(existing, xmlKey.c_str()))
                {
                Utf8String xml;
                if (SchemaWriteStatus::Success == schema->WriteToXmlString(xml))
                    m_ecdb.SaveBriefcaseLocalValue(xmlKey.c_str(), xml);
                }
            }
        }

    // Save updated session log
    m_ecdb.SaveBriefcaseLocalValue(KEY_SESSIONS, sessionsDoc.Stringify());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaImportResult SchemaManager::Sessions::Replay(ECDbR target) const
    {
    // Load session log
    Utf8String sessionsJson;
    if (BE_SQLITE_ROW != m_ecdb.QueryBriefcaseLocalValue(sessionsJson, KEY_SESSIONS))
        return SchemaImportResult(SchemaImportResult::Status::OK);

    BeJsDocument sessionsDoc;
    sessionsDoc.Parse(sessionsJson);
    if (!sessionsDoc.isArray() || sessionsDoc.size() == 0)
        return SchemaImportResult(SchemaImportResult::Status::OK);

    SchemaImportResult lastResult(SchemaImportResult::Status::OK);

    sessionsDoc.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst sessionEntry)
        {
        if (!sessionEntry.isObject())
            return false;

        int sessionId = sessionEntry["id"].GetInt();
        SchemaImportOptions options = SchemaImportOptionsFromString(Utf8String(sessionEntry["importFlags"].asCString()));
        BeJsConst schemasArr = sessionEntry["schemas"];
        if (!schemasArr.isArray() || schemasArr.size() == 0)
            return false;

        // Create read context with target ECDb as locater
        ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
        readContext->AddSchemaLocater(const_cast<SchemaManager&>(target.Schemas()));

        bvector<ECSchemaCP> schemasToImport;

        // Deserialize each schema from stored XML
        schemasArr.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst schemaNameVal)
            {
            Utf8String fullName(schemaNameVal.asCString());

            // Try dynamic key first, then non-dynamic
            Utf8String xmlKey = SchemaXmlKey(fullName, true, sessionId);
            Utf8String xml;
            if (BE_SQLITE_ROW != m_ecdb.QueryBriefcaseLocalValue(xml, xmlKey.c_str()))
                {
                xmlKey = SchemaXmlKey(fullName, false, sessionId);
                if (BE_SQLITE_ROW != m_ecdb.QueryBriefcaseLocalValue(xml, xmlKey.c_str()))
                    {
                    // Schema XML not found — this should not happen
                    lastResult = SchemaImportResult(SchemaImportResult::Status::ERROR);
                    return true; // stop iteration
                    }
                }

            ECSchemaPtr schema;
            if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(schema, xml.c_str(), *readContext))
                {
                lastResult = SchemaImportResult(SchemaImportResult::Status::ERROR);
                return true; // stop iteration
                }

            schemasToImport.push_back(schema.get());
            return false;
            });

        if (!lastResult.IsOk())
            return true; // stop outer iteration

        if (!schemasToImport.empty())
            {
            lastResult = target.Schemas().ImportSchemas(schemasToImport, options);
            if (!lastResult.IsOk())
                return true; // stop on first error
            }

        return false;
        });

    return lastResult;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaManager::Sessions::Clear()
    {
    // Load session log to find all stored schema XML keys
    Utf8String sessionsJson;
    if (BE_SQLITE_ROW == m_ecdb.QueryBriefcaseLocalValue(sessionsJson, KEY_SESSIONS))
        {
        BeJsDocument sessionsDoc;
        sessionsDoc.Parse(sessionsJson);
        if (sessionsDoc.isArray())
            {
            sessionsDoc.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst sessionEntry)
                {
                if (!sessionEntry.isObject())
                    return false;

                int sessionId = sessionEntry["id"].GetInt();
                BeJsConst schemasArr = sessionEntry["schemas"];
                if (!schemasArr.isArray())
                    return false;

                schemasArr.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst schemaNameVal)
                    {
                    Utf8String fullName(schemaNameVal.asCString());
                    // Delete both dynamic and non-dynamic keys
                    Utf8String dynamicKey = SchemaXmlKey(fullName, true, sessionId);
                    Utf8String staticKey = SchemaXmlKey(fullName, false, sessionId);
                    m_ecdb.DeleteBriefcaseLocalValue(dynamicKey.c_str());
                    m_ecdb.DeleteBriefcaseLocalValue(staticKey.c_str());
                    return false;
                    });
                return false;
                });
            }
        }

    // Delete the session log itself
    m_ecdb.DeleteBriefcaseLocalValue(KEY_SESSIONS);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int SchemaManager::Sessions::GetCount() const
    {
    Utf8String sessionsJson;
    if (BE_SQLITE_ROW != m_ecdb.QueryBriefcaseLocalValue(sessionsJson, KEY_SESSIONS))
        return 0;

    BeJsDocument sessionsDoc;
    sessionsDoc.Parse(sessionsJson);
    if (!sessionsDoc.isArray())
        return 0;

    return (int)sessionsDoc.size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaManager::Sessions& SchemaManager::GetSessions() const
    {
    if (m_sessions == nullptr)
        {
        ECDb& ecdb = const_cast<ECDb&>(Main().GetECDb());
        m_sessions = new Sessions(ecdb);
        }
    return *m_sessions;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
