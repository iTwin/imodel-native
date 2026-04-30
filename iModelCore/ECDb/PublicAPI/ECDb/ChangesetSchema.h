/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDb.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>
#include <BeRapidJson/BeJsValue.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Captures the EC-level interpretation (class identity + property↔column mapping per
//! table segment) of every EC row that appears in a changeset.
//!
//! The schema is computed once from the changeset against the ECDb that produced it,
//! then carried alongside the changeset so that — after subsequent schema upgrades —
//! the changeset can be diffed against the evolved ECDb and transformed if required.
//!
//! Only entity-class rows are captured. Relationship class rows (link tables and
//! end-table FK columns) are not included in v1.
//!
//! Streams are consumed once per call. Callers that need both Validate and a separate
//! consumer (e.g. Transform or ApplyChanges) must provide a fresh stream per call.
//!
//! @see ChangesetSchemaDiff
// @bsiclass
//=======================================================================================
struct ChangesetSchema
    {
    //! Maps one EC property access string to one SQLite column in one specific table.
    //! @p sourceColumnIndex is the column position as it appeared in the changeset's
    //! source table — used by Transform to read raw values from change rows without
    //! needing the originating ECDb to be open.
    struct PropertyColumnMap
        {
        Utf8String accessString;
        Utf8String tableName;
        Utf8String columnName;
        int        sourceColumnIndex = -1;

        bool IsValid() const { return !accessString.empty() && !tableName.empty() && !columnName.empty(); }
        };

    //! One physical table segment (primary, overflow, or joined partition) for a class
    //! as it appears in the changeset.
    struct TableSegment
        {
        Utf8String tableName;
        int        idColumnIndex      = -1;  //!< position of ECInstanceId column in source rows
        int        classIdColumnIndex = -1;  //!< position of ECClassId column; -1 if virtual / single-class table
        int        sourceColumnCount  = 0;   //!< total column count in source table
        bmap<Utf8String /*accessString*/, PropertyColumnMap> columns;
        };

    //! Per-class snapshot. A class with overflow / joined tables produces multiple
    //! TableSegment entries — one per physical table.
    struct ClassEntry
        {
        ECN::ECClassId classId;
        Utf8String     fullName;
        bmap<Utf8String /*tableName*/, TableSegment> tableSegments;
        };

    //! Extract the schema from a changeset, interpreted against the supplied ECDb.
    //! @param ecdb The ECDb that the changeset originated against (or that has the
    //!             schemas needed to interpret the rows).
    //! @param changes The changeset to walk. Consumed.
    ECDB_EXPORT static ChangesetSchema ExtractFrom(ECDbCR ecdb, BeSQLite::ChangeStream& changes);

    //! Serialize the schema as JSON to the given value.
    ECDB_EXPORT void To(BeJsValue value) const;

    //! Deserialize a schema previously written via To. Returns an empty schema on
    //! malformed input (with a warning logged).
    ECDB_EXPORT static ChangesetSchema From(BeJsConst value);

    //! Validate that @p changes is consistent with this schema, interpreted against
    //! @p ecdb. Returns SUCCESS only when every mapped row's class and column mappings
    //! match the stored schema. Optional @p outErrors receives diagnostic strings.
    ECDB_EXPORT BentleyStatus Validate(ECDbCR ecdb, BeSQLite::ChangeStream& changes, bvector<Utf8String>* outErrors = nullptr) const;

    bmap<ECN::ECClassId, ClassEntry> const& GetClasses() const { return m_classes; }
    bool IsEmpty() const { return m_classes.empty(); }

private:
    bmap<ECN::ECClassId, ClassEntry> m_classes;
    };

//=======================================================================================
//! Describes how a stored ChangesetSchema differs from the current state of an ECDb,
//! and (when transformable) drives the transformation of a source changeset into one
//! that matches the current ECDb's mapping.
//!
//! Diffs are classified per (class, column):
//!   - ClassIdRemapped — same fullName, different ECClassId (transformable)
//!   - ColumnRemapped  — same access string and table, different SQLite column name (transformable)
//!   - ColumnMoved     — same access string, different SQLite table (transformable, complex)
//!   - PropertyLost    — access string no longer present in ECDb (NOT transformable)
//!   - ClassLost       — class fullName no longer present in ECDb (NOT transformable)
// @bsiclass
//=======================================================================================
struct ChangesetSchemaDiff
    {
    enum class ChangeKind
        {
        ClassIdRemapped,
        ColumnRemapped,
        ColumnMoved,
        PropertyLost,
        ClassLost,
        };

    struct ColumnDiff
        {
        Utf8String                            accessString;
        ChangeKind                            kind;
        ChangesetSchema::PropertyColumnMap    oldMap;
        ChangesetSchema::PropertyColumnMap    newMap; //!< empty when kind == PropertyLost
        };

    struct ClassDiff
        {
        ChangesetSchema::ClassEntry  sourceEntry;
        ECN::ECClassId               targetClassId;        //!< invalid when kind == ClassLost
        bool                         classLost = false;
        bvector<ColumnDiff>          columnDiffs;
        ECDB_EXPORT bool             IsTransformable() const;
        };

    //! Compute the diff between @p source (a previously extracted schema) and the
    //! current state of @p current (an open ECDb).
    ECDB_EXPORT static ChangesetSchemaDiff Compute(ChangesetSchema const& source, ECDbCR current);

    //! True iff every class diff is transformable (no ClassLost, no PropertyLost).
    //! When false, Transform will fail.
    ECDB_EXPORT bool IsTransformable() const;

    //! Transform @p source into @p target, applying the diffs against @p targetDb's
    //! current table layout. Non-EC rows (be_*, ec_*, dgn_*, …) are passed through
    //! unchanged. Returns BE_SQLITE_OK on success.
    //!
    //! Note: vertical partitioning means a single ECInstance may produce rows in
    //! multiple SQLite tables. ColumnMoved diffs may synthesise rows in target tables
    //! that had no source row for the affected instance.
    ECDB_EXPORT BeSQLite::DbResult Transform(BeSQLite::ChangeStream& source, BeSQLite::ChangeSet& target, ECDbCR targetDb) const;

    bvector<ClassDiff> const& GetClassDiffs() const { return m_classDiffs; }

private:
    bvector<ClassDiff> m_classDiffs;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
