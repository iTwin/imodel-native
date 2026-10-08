/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECSqlStatement.h>
#include <ECDb/ChangesetReader.h>
#include <memory>
#include <vector>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ChangeUnifierImpl;

//=======================================================================================
//! ChangeUnifier drains one or more ChangesetReaders and merges their per-table rows into
//! one change per EC instance and stage.
//! @remarks
//! - Rows are merged on (root ECClassId of the table's class family, ECInstanceId, stage). Rows coming from the
//!   main table, joined tables and overflow tables of the same instance and stage therefore end up in one instance.
//! - An insert produces a New instance, a delete an Old instance, an update one Old and one New instance.
//! - The ECClassId of a merged instance is the most derived class id seen among the contributing rows.
//! - Property values: a later row wins over an earlier one.
//! - The opcode and the indirect flag come from the row of the main (non-overflow) table. If only overflow
//!   tables contributed, the opcode is reported as Update. Without a main table row, the indirect flag of the first row is used.
//! - Rows of non-EC tables are skipped.
//! - Instances are returned sorted numerically by (root ECClassId, ECInstanceId, stage) with Old before New.
//! - Only the properties listed in Options::m_propNames (plus ECInstanceId and ECClassId) are kept. They are dropped
//!   before any value is stored. A name also matches the relationship end properties derived from it, e.g. "Source"
//!   keeps SourceECInstanceId and SourceECClassId, "Target" keeps TargetECInstanceId and TargetECClassId.
//! - Merged data is held in a hash map. When its estimated size exceeds Options::m_memoryBudgetBytes, the map is sorted
//!   and written as a run to a temporary file next to the ECDb file. The runs and the in-memory remainder are k-way merged
//!   when stepping starts. Spill files are deleted by Close() and by the destructor.
//! - Property values are rendered with ECSqlRowAdaptor using the JsReadOptions passed to AppendFrom, exactly as
//!   the rows of a ChangesetReader are rendered to JavaScript.
//! - Not thread-safe.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangeUnifier final {
public:
    static constexpr uint64_t DefaultMemoryBudgetBytes = 64ull * 1024ull * 1024ull;

    struct Options final {
        //! EC property names to keep. ECInstanceId and ECClassId are always kept. An empty list keeps all properties.
        std::vector<Utf8String> m_propNames;
        //! Bytes of merged instance data held in memory before sorted runs spill to temporary files. 0 means never spill.
        uint64_t m_memoryBudgetBytes = DefaultMemoryBudgetBytes;
    };

    //! Receives one merged instance from Step() as JSON-like events, in the order a JSON writer
    //! would produce them: StartObject, a Key and a value per property and for "$meta", then EndObject.
    //! @remarks Lets a caller build its own value representation, e.g. JavaScript objects, without an intermediate document.
    //! Names and strings from the unifier's string table (property names, "$meta" member names, table names, op and stage)
    //! carry a non-zero @p stringId that identifies the same text for the unifier's lifetime, so a writer can convert each
    //! text once. Other strings have stringId 0. @p name and @p value are not NUL-terminated.
    struct IInstanceWriter {
        virtual ~IInstanceWriter() {}
        virtual void StartObject() = 0;
        virtual void Key(Utf8CP name, size_t length, uint32_t stringId) = 0;
        virtual void EndObject() = 0;
        virtual void StartArray() = 0;
        virtual void EndArray() = 0;
        virtual void Null() = 0;
        virtual void Bool(bool value) = 0;
        virtual void Int64(int64_t value) = 0;
        virtual void UInt64(uint64_t value) = 0;
        virtual void Double(double value) = 0;
        virtual void String(Utf8CP value, size_t length, uint32_t stringId) = 0;
        virtual void Binary(Byte const* data, size_t size) = 0;
    };

private:
    std::unique_ptr<ChangeUnifierImpl> m_impl;

    ChangeUnifier(ChangeUnifier const&) = delete;
    ChangeUnifier& operator=(ChangeUnifier const&) = delete;
    ChangeUnifier(ChangeUnifier&&) = delete;
    ChangeUnifier& operator=(ChangeUnifier&&) = delete;

public:
    //! Constructs a ChangeUnifier that keeps all properties and uses the default memory budget.
    ECDB_EXPORT ChangeUnifier();
    //! Constructs a ChangeUnifier with the specified options.
    ECDB_EXPORT explicit ChangeUnifier(Options const& options);
    //! Destructor. Deletes any spill files that are left.
    ECDB_EXPORT ~ChangeUnifier();

    //! Steps @p reader until it is exhausted and merges all of its rows.
    //! @remarks May be called several times (e.g. once per changeset) before the first call to Step().
    //! The reader stays owned by the caller. Only rows returned by the reader's Step() are seen, so the reader should not have been stepped before.
    //! @param[in] reader An open ChangesetReader.
    //! @param[in] rowOptions Options used to render property values.
    //! All readers must belong to the same ECDb, which must stay open until the last AppendFrom returns.
    //! @return BE_SQLITE_OK on success, BE_SQLITE_MISUSE if called after Step() or Close() or after a previous failure,
    //! or with a reader of another ECDb, or an error code. On other errors the unifier cannot be used anymore; see GetLastError().
    ECDB_EXPORT DbResult AppendFrom(ChangesetReader& reader, JsReadOptions const& rowOptions);

    //! Sends the next merged instance to @p writer as one object.
    //! @remarks The first call finalizes the merge (sort and k-way merge of spilled runs). After that, AppendFrom fails.
    //! The instance contains the rendered property values plus a "$meta" object with the members
    //! tables, op, stage, changeIndexes, instanceKey, changeFetchedPropNames and isIndirectChange.
    //! @return BE_SQLITE_ROW if an instance was written, BE_SQLITE_DONE when all instances were returned,
    //! BE_SQLITE_MISUSE after Close() or after a previous failure, or an error code.
    ECDB_EXPORT DbResult Step(IInstanceWriter& writer);

    //! Frees all memory and deletes spill files. Idempotent.
    ECDB_EXPORT void Close();

    //! Gets a description of the last error, or an empty string.
    ECDB_EXPORT Utf8StringCR GetLastError() const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
