/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

static constexpr Utf8CP SESSIONS_INDEX_KEY = "ecdb.sessions";
static constexpr Utf8CP SESSION_SCHEMA_KEY_PREFIX = "ecdb.session.";

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SchemaSessionEntry::GetXml() const
    {
    Utf8String key = SESSION_SCHEMA_KEY_PREFIX;
    key += m_guid;
    Utf8String xml;
    m_ecdb.QueryBriefcaseLocalValue(xml, key.c_str());
    return xml;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaSessions::IsEnabled() const
    {
    return m_ecdb.GetBriefcaseId().IsBriefcase();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaSessions::RecordSession(bvector<ECN::ECSchemaCP> const& schemas)
    {
    if (!IsEnabled() || schemas.empty())
        return;

    // Load existing sessions index
    BeJsDocument index;
    Utf8String indexJson;
    if (BE_SQLITE_ROW == m_ecdb.QueryBriefcaseLocalValue(indexJson, SESSIONS_INDEX_KEY))
        index.Parse(indexJson.c_str());
    else
        index.SetEmptyArray();

    // Build new session record: an object with a "schemas" array
    BeJsValue newSession = index.appendObject();
    BeJsValue schemasArr = newSession["schemas"];
    schemasArr.SetEmptyArray();

    bool anyRecorded = false;
    for (ECN::ECSchemaCP schema : schemas)
        {
        if (schema == nullptr)
            continue;

        // Serialize schema to XML
        Utf8String xml;
        if (ECN::SchemaWriteStatus::Success != schema->WriteToXmlString(xml))
            continue;

        // Generate a unique GUID for this schema entry
        Utf8String guid = BeGuid(true).ToString();

        // Store XML blob in be_local
        Utf8String xmlKey = SESSION_SCHEMA_KEY_PREFIX;
        xmlKey += guid;
        m_ecdb.SaveBriefcaseLocalValue(xmlKey.c_str(), xml);

        // Append entry metadata to session record
        BeJsValue entry = schemasArr.appendObject();
        entry["g"] = guid;
        entry["n"] = schema->GetName();
        entry["r"] = schema->GetVersionRead();
        entry["w"] = schema->GetVersionWrite();
        entry["p"] = schema->GetVersionMinor();
        anyRecorded = true;
        }

    if (!anyRecorded)
        return;

    // Persist the updated sessions index
    m_ecdb.SaveBriefcaseLocalValue(SESSIONS_INDEX_KEY, index.Stringify());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaSessions::Clear()
    {
    Utf8String indexJson;
    if (BE_SQLITE_ROW == m_ecdb.QueryBriefcaseLocalValue(indexJson, SESSIONS_INDEX_KEY))
        {
        BeJsDocument index;
        index.Parse(indexJson.c_str());
        index.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst sessionVal) -> bool
            {
            BeJsConst schemasArr = sessionVal["schemas"];
            schemasArr.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst entryVal) -> bool
                {
                Utf8String guid = entryVal["g"].asString();
                Utf8String xmlKey = SESSION_SCHEMA_KEY_PREFIX;
                xmlKey += guid;
                m_ecdb.DeleteBriefcaseLocalValue(xmlKey.c_str());
                return false; // continue
                });
            return false; // continue
            });
        }
    m_ecdb.DeleteBriefcaseLocalValue(SESSIONS_INDEX_KEY);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bvector<SchemaImportSession> SchemaSessions::GetSessions() const
    {
    bvector<SchemaImportSession> result;

    Utf8String indexJson;
    if (BE_SQLITE_ROW != m_ecdb.QueryBriefcaseLocalValue(indexJson, SESSIONS_INDEX_KEY))
        return result;

    BeJsDocument index;
    index.Parse(indexJson.c_str());

    index.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst sessionVal) -> bool
        {
        bvector<SchemaSessionEntry> entries;
        BeJsConst schemasArr = sessionVal["schemas"];
        schemasArr.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst entryVal) -> bool
            {
            Utf8String guid = entryVal["g"].asString();
            Utf8String name = entryVal["n"].asString();
            uint32_t r = entryVal["r"].asUInt();
            uint32_t w = entryVal["w"].asUInt();
            uint32_t p = entryVal["p"].asUInt();
            ECN::SchemaKey key(name.c_str(), r, w, p);
            entries.push_back(SchemaSessionEntry(m_ecdb, key, guid));
            return false; // continue
            });
        result.push_back(SchemaImportSession(std::move(entries)));
        return false; // continue
        });

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECSchemaReadContextR SchemaSessions::GetOrCreateReadContext() const
    {
    if (!m_readContext.IsValid())
        m_readContext = ECN::ECSchemaReadContext::CreateContext();
    return *m_readContext;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
