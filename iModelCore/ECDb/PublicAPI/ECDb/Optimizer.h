/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Performs optional destructive maintenance on an ECDb.
//!
//! Optimizer never starts, commits, or abandons a transaction. The caller must have an
//! active transaction and decides whether to call SaveChanges or AbandonChanges. If Optimize
//! returns an error, some earlier operations may already have changed the caller's transaction.
//!
//! BisCore and ECDb system schema definitions and their mappings are never modified.
//! Schema versions are preserved.
//! Successful schema or mapping cleanup invalidates ECDb schema caches. Callers must discard
//! schema, class, property, and mapping pointers acquired before Optimize.
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct Optimizer final
    {
    enum class Options : uint32_t
        {
        None = 0,
        DropUnusedSchemas = 1 << 0,
        DropEmptyDynamicClasses = 1 << 1,
        DropEmptyDynamicProperties = 1 << 2,
        CompactSharedColumns = 1 << 3,
        DeleteOrphanRelationships = 1 << 4,
        NullifyOrphanNavProps = 1 << 5,
        PurgeInvalidClassIds = 1 << 6,
        DropUnmappedTables = 1 << 7,
        DropUnmappedSharedColumns = 1 << 8,
        RunAnalyze = 1 << 9,

        SchemaCleanup = 0x007,
        DataCleanup = 0x070,
        StorageCleanup = 0x188,
        All = 0x3ff,
        };

    struct Result final
        {
        struct Phase final
            {
            Options m_option = Options::None;
            uint64_t m_candidates = 0;
            uint64_t m_changed = 0;
            uint64_t m_skipped = 0;
            BeDuration m_elapsed;
            };

    private:
        bvector<Phase> m_phases;
        bvector<Utf8String> m_messages;
        Options m_failedOption = Options::None;
        DbResult m_status = BE_SQLITE_OK;
        bool m_isDryRun = false;

        friend struct Optimizer;

    public:
        bool IsDryRun() const { return m_isDryRun; }
        bool IsSuccess() const { return m_status == BE_SQLITE_OK; }
        DbResult GetStatus() const { return m_status; }
        Options GetFailedOption() const { return m_failedOption; }
        bvector<Phase> const& GetPhases() const { return m_phases; }
        bvector<Utf8String> const& GetMessages() const { return m_messages; }
        ECDB_EXPORT void ToJson(BeJsValue) const;
        };

private:
    ECDbR m_ecdb;
    Utf8String m_lastError;

    DbResult Run(Options, Result&, bool isDryRun, SchemaImportToken const*, ECCrudWriteToken const*);

public:
    explicit Optimizer(ECDbR ecdb) : m_ecdb(ecdb) {}

    //! Runs the selected maintenance operations in the caller's active transaction. Pass the
    //! authorization tokens required by the ECDb's settings for selected data and metadata writes.
    ECDB_EXPORT DbResult Optimize(Options, Result&, SchemaImportToken const* = nullptr, ECCrudWriteToken const* = nullptr);
    //! Builds the same optimization plan without modifying the main database.
    ECDB_EXPORT DbResult DryRun(Options, Result&);

    Utf8StringCR GetLastError() const { return m_lastError; }
    ECDB_EXPORT static Utf8CP GetOptionName(Options);
    };

ENUM_IS_FLAGS(Optimizer::Options);

END_BENTLEY_SQLITE_EC_NAMESPACE
