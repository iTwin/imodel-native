/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/SchemaEditor.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//! One ordered unit of a SchemaImportPlan: a single command envelope or an atomic dependency group.
struct SchemaImportPlanStep final {
    Utf8String m_schema;   //!< Schema the step belongs to.
    Utf8String m_item;     //!< Qualified item (or schema name) the step was generated for.
    Utf8String m_facet;    //!< Facet family, e.g. "Schema", "EntityClass", "Property", "CustomAttribute".
    bool m_group = false;  //!< True if m_commands must run through SchemaEditor::ExecuteGroup.
    Utf8String m_commands; //!< JSON array of {command, args} envelopes.
};

//! An owned, inspectable, deterministic list of SchemaEditor commands that turns the persisted schemas into the
//! incoming ones. Generated plans are not editable; use SchemaEditor directly for hand-written command streams.
struct SchemaImportPlan final {
    friend struct SchemaImportAdapter;

private:
    bvector<SchemaImportPlanStep> m_steps;
    bvector<Utf8String> m_warnings;
    Utf8String m_baseline;
    SchemaImportOptions m_options = SchemaImportOptions::None;
    bool m_built = false;

public:
    bvector<SchemaImportPlanStep> const& GetSteps() const { return m_steps; }
    //! Non-fatal notes such as skipped compatible downgrades. Warnings never turn a plan into a failure.
    bvector<Utf8String> const& GetWarnings() const { return m_warnings; }
    bool IsBuilt() const { return m_built; }
    bool IsEmpty() const { return m_steps.empty(); }
    ECDB_EXPORT size_t GetCommandCount() const;
    //! {"steps":[{"schema","item","facet","group","commands":[...]}],"warnings":[...]}
    ECDB_EXPORT void ToJson(BeJsValue) const;
};

//! Returned by a successful SchemaImportAdapter execution. Changes are pending in the default transaction.
//! Destroying the handle without Commit/Abandon leaves them pending for the caller. If the caller abandons
//! the transaction directly it must also call ECDb::ClearECDbCache.
struct SchemaImportCompletion final {
    friend struct SchemaImportAdapter;

private:
    ECDb* m_db = nullptr;

public:
    bool IsPending() const { return m_db != nullptr; }
    //! ECDb::SaveChanges.
    ECDB_EXPORT DbResult Commit();
    //! ECDb::AbandonChanges, clears ECDb caches and resets the metadata id sequences.
    ECDB_EXPORT DbResult Abandon();
};

//! Imports ECSchemas through SchemaEditor commands instead of the monolithic ImportSchemas path.
//! New schemas are created item by item; existing schemas receive only the commands required by the diff.
//! Every command maps immediately and runs the editor's validation; nothing is committed.
struct SchemaImportAdapter final {
private:
    SchemaImportAdapter() = delete;

public:
    //! Read-only: builds an ordered command plan. Input schemas are never mutated; they are deep-copied first.
    ECDB_EXPORT static SchemaCommandResult BuildPlan(ECDbR, bvector<ECN::ECSchemaCP> const&, SchemaImportPlan&,
                                                     SchemaImportOptions = SchemaImportOptions::None, SchemaImportToken const* = nullptr);
    //! Executes a plan built for the same database state. Rejects stale plans. On failure the editor session
    //! (and every earlier step) is abandoned. On success, changes stay pending; see SchemaImportCompletion.
    ECDB_EXPORT static SchemaCommandResult ExecutePlan(ECDbR, SchemaImportPlan const&, SchemaImportCompletion* = nullptr,
                                                       SchemaImportToken const* = nullptr);
    //! BuildPlan followed by ExecutePlan.
    ECDB_EXPORT static SchemaCommandResult ImportSchemas(ECDbR, bvector<ECN::ECSchemaCP> const&, SchemaImportCompletion* = nullptr,
                                                         SchemaImportOptions = SchemaImportOptions::None, SchemaImportToken const* = nullptr);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
