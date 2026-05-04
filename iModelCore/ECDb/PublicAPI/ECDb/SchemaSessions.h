/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECObjects/ECContext.h>
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! A single entry in a schema import session, representing one ECSchema that was imported.
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaSessionEntry final
    {
private:
    ECDbCR m_ecdb;
    ECN::SchemaKey m_key;
    Utf8String m_guid;

public:
    SchemaSessionEntry(ECDbCR ecdb, ECN::SchemaKey const& key, Utf8StringCR guid)
        : m_ecdb(ecdb), m_key(key), m_guid(guid) {}

    //! Get the SchemaKey identifying this schema (name and version).
    ECN::SchemaKey const& GetSchemaKey() const { return m_key; }

    //! Lazily load and return the serialized XML for this schema from be_local storage.
    //! @return The schema XML string, or an empty string if not found.
    ECDB_EXPORT Utf8String GetXml() const;
    };

//=======================================================================================
//! Represents one call to SchemaManager::ImportSchemas, holding the schemas that were
//! newly mapped during that call.
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaImportSession final
    {
private:
    bvector<SchemaSessionEntry> m_entries;

public:
    explicit SchemaImportSession(bvector<SchemaSessionEntry>&& entries)
        : m_entries(std::move(entries)) {}

    //! Get all schema entries recorded in this session.
    bvector<SchemaSessionEntry> const& GetEntries() const { return m_entries; }
    };

//=======================================================================================
//! Manages recording and retrieval of ECSchema import sessions stored in be_local.
//! When IsEnabled() returns true (i.e. the connection is a briefcase connection),
//! every successful ImportSchemas call is automatically recorded.
//! Use Clear() to remove all session records, and GetSessions() to retrieve them.
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaSessions final : NonCopyableClass
    {
private:
    ECDbCR m_ecdb;
    mutable ECN::ECSchemaReadContextPtr m_readContext;

public:
    explicit SchemaSessions(ECDbCR ecdb) : m_ecdb(ecdb) {}

    //! Returns true if sessions are automatically recorded.
    //! Sessions are enabled when the connection is a briefcase (has a valid BriefcaseId).
    ECDB_EXPORT bool IsEnabled() const;

    //! Record a new import session for the given list of schemas.
    //! @remarks Called internally by SchemaManager::ImportSchemas on successful import.
    //! @param[in] schemas The schemas that were newly mapped in this import call.
    ECDB_EXPORT void RecordSession(bvector<ECN::ECSchemaCP> const& schemas);

    //! Delete all session records and their associated schema XML blobs from be_local.
    ECDB_EXPORT void Clear();

    //! Retrieve all previously recorded import sessions, in the order they were recorded.
    ECDB_EXPORT bvector<SchemaImportSession> GetSessions() const;

    //! Get or create a shared ECSchemaReadContext used when loading schemas from sessions.
    //! All schemas loaded from sessions share this single context.
    ECDB_EXPORT ECN::ECSchemaReadContextR GetOrCreateReadContext() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
