/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECInstanceFinder.h>
#include <ECDb/SchemaManager.h>
#include <BeRapidJson/BeJsValue.h>
#include <initializer_list>
#include <memory>
#include <utility>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//! Ordered arguments. Duplicate keys are retained so validation can report them.
struct SchemaCommandArgs final {
private:
    bvector<std::pair<Utf8String, Utf8String>> m_entries;

public:
    SchemaCommandArgs() = default;
    SchemaCommandArgs(std::initializer_list<std::pair<Utf8String, Utf8String>> entries) : m_entries(entries) {}
    bvector<std::pair<Utf8String, Utf8String>> const& GetEntries() const { return m_entries; }
};

struct SchemaCommandIssue final {
    Utf8String code;
    Utf8String arg;
    Utf8String message;
};

struct SchemaCommandResult final {
    enum class Status {
        OK,
        ERROR,
        ERROR_INVALID_ARGS,
        ERROR_VALIDATION,
        ERROR_NOT_FOUND,
        ERROR_DATA_TRANSFORM_REQUIRED,
        ERROR_DATA_DELETION_REQUIRED,
        ERROR_READONLY,
        ERROR_SESSION
    };

private:
    Status m_status = Status::OK;
    bvector<SchemaCommandIssue> m_issues;
    ECInstanceKeyMultiMap m_blockingInstances;
    Utf8String m_failedCommand;
    size_t m_failedCommandIndex = SIZE_MAX;

public:
    ECDB_EXPORT static SchemaCommandResult Failure(Status, Utf8CP code, Utf8CP arg, Utf8StringCR message);
    bool IsOk() const { return m_status == Status::OK; }
    Status GetStatus() const { return m_status; }
    bvector<SchemaCommandIssue> const& GetIssues() const { return m_issues; }
    //! Adding an issue to an otherwise successful result changes its status to ERROR.
    ECDB_EXPORT void AddIssue(SchemaCommandIssue);
    ECDB_EXPORT void SetFailedCommand(Utf8StringCR, size_t index = SIZE_MAX);
    Utf8StringCR GetFailedCommand() const { return m_failedCommand; }
    //! Index of the failed child within a batch or group; SIZE_MAX when not applicable.
    size_t GetFailedCommandIndex() const { return m_failedCommandIndex; }
    ECDB_EXPORT void SetBlockingInstances(ECInstanceKeyMultiMap);
    ECInstanceKeyMultiMap const& GetBlockingInstances() const { return m_blockingInstances; }
    //! Emits a symbolic status, issues, and blockingInstances[{classId,instanceId}].
    //! Failed commands additionally carry failedCommand and, for batches, failedCommandIndex.
    ECDB_EXPORT void ToJson(BeJsValue) const;
};

struct SchemaImportAdapter;

//! An exclusive, atomic schema editing session. A failed command abandons the entire session.
//! Destruction abandons any session that has been neither saved nor finished.
struct SchemaEditor final {
    friend struct SchemaImportAdapter;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    explicit SchemaEditor(std::unique_ptr<Impl>);
    //! Permits creating/upgrading the standard reference-graph schemas (Units, Formats, CoreCustomAttributes,
    //! SchemaCustomization, ECDbSchemaPolicies). Only SchemaImportAdapter enables this.
    void AllowStandardSchemaEdits();
    static SchemaCommandResult CheckEnvironment(ECDbR, SchemaImportOptions, SchemaImportToken const*, bool requireCleanTransaction);

public:
    SchemaEditor(SchemaEditor const&) = delete;
    SchemaEditor& operator=(SchemaEditor const&) = delete;
    ECDB_EXPORT static std::unique_ptr<SchemaEditor> Begin(ECDbR, SchemaImportOptions = SchemaImportOptions::None,
                                                           SchemaImportToken const* = nullptr, SchemaCommandResult* = nullptr);
    ECDB_EXPORT ~SchemaEditor();
    ECDB_EXPORT SchemaCommandResult Validate(Utf8StringCR command, SchemaCommandArgs const&) const;
    ECDB_EXPORT SchemaCommandResult Validate(Utf8StringCR command, BeJsConst args) const;
    ECDB_EXPORT SchemaCommandResult Execute(Utf8StringCR command, SchemaCommandArgs const&);
    ECDB_EXPORT SchemaCommandResult Execute(Utf8StringCR command, BeJsConst args);
    ECDB_EXPORT SchemaCommandResult Execute(BeJsConst envelope);
    //! Executes command envelopes one after another; each command is mapped before the next runs.
    ECDB_EXPORT SchemaCommandResult ExecuteBatch(BeJsConst commands);
    //! Executes an array of command envelopes as one atomic dependency group: every child's metadata is
    //! applied in order (later children see earlier metadata), then the union of affected classes is mapped
    //! once. Classes created in the group may still receive creation-time custom attributes (e.g. timestamp,
    //! DbIndexList) from later children. Groups cannot be nested. Failure abandons the whole session and
    //! reports the failing child's name and index.
    ECDB_EXPORT SchemaCommandResult ExecuteGroup(BeJsConst commands);
    //! Validates the persisted mappings, commits the default transaction and ends the session.
    ECDB_EXPORT SchemaCommandResult SaveChanges();
    //! Validates the persisted mappings and ends the session WITHOUT committing. The changes remain pending in the
    //! default transaction; the caller must call ECDb::SaveChanges, or ECDb::AbandonChanges followed by
    //! ECDb::ClearECDbCache. A failed validation abandons the session as usual.
    ECDB_EXPORT SchemaCommandResult Finish();
    ECDB_EXPORT void AbandonChanges();
    ECDB_EXPORT bvector<Utf8String> GetCommandNames() const;

    SchemaCommandResult CreateSchema(SchemaCommandArgs const& args) { return Execute("CreateSchema", args); }
    SchemaCommandResult AlterSchema(SchemaCommandArgs const& args) { return Execute("AlterSchema", args); }
    SchemaCommandResult DropSchema(SchemaCommandArgs const& args) { return Execute("DropSchema", args); }
    SchemaCommandResult AddSchemaReference(SchemaCommandArgs const& args) { return Execute("AddSchemaReference", args); }
    SchemaCommandResult RemoveSchemaReference(SchemaCommandArgs const& args) { return Execute("RemoveSchemaReference", args); }
    SchemaCommandResult CreateEntityClass(SchemaCommandArgs const& args) { return Execute("CreateEntityClass", args); }
    SchemaCommandResult AlterEntityClass(SchemaCommandArgs const& args) { return Execute("AlterEntityClass", args); }
    SchemaCommandResult DropEntityClass(SchemaCommandArgs const& args) { return Execute("DropEntityClass", args); }
    SchemaCommandResult CreateStructClass(SchemaCommandArgs const& args) { return Execute("CreateStructClass", args); }
    SchemaCommandResult AlterStructClass(SchemaCommandArgs const& args) { return Execute("AlterStructClass", args); }
    SchemaCommandResult DropStructClass(SchemaCommandArgs const& args) { return Execute("DropStructClass", args); }
    SchemaCommandResult CreateRelationshipClass(SchemaCommandArgs const& args) { return Execute("CreateRelationshipClass", args); }
    SchemaCommandResult AlterRelationshipClass(SchemaCommandArgs const& args) { return Execute("AlterRelationshipClass", args); }
    SchemaCommandResult DropRelationshipClass(SchemaCommandArgs const& args) { return Execute("DropRelationshipClass", args); }
    SchemaCommandResult CreateMixin(SchemaCommandArgs const& args) { return Execute("CreateMixin", args); }
    SchemaCommandResult AddMixin(SchemaCommandArgs const& args) { return Execute("AddMixin", args); }
    SchemaCommandResult RemoveMixin(SchemaCommandArgs const& args) { return Execute("RemoveMixin", args); }
    SchemaCommandResult CreateCustomAttributeClass(SchemaCommandArgs const& args) { return Execute("CreateCustomAttributeClass", args); }
    SchemaCommandResult AlterCustomAttributeClass(SchemaCommandArgs const& args) { return Execute("AlterCustomAttributeClass", args); }
    SchemaCommandResult DropCustomAttributeClass(SchemaCommandArgs const& args) { return Execute("DropCustomAttributeClass", args); }
    SchemaCommandResult AddProperty(SchemaCommandArgs const& args) { return Execute("AddProperty", args); }
    SchemaCommandResult AlterProperty(SchemaCommandArgs const& args) { return Execute("AlterProperty", args); }
    SchemaCommandResult DropProperty(SchemaCommandArgs const& args) { return Execute("DropProperty", args); }
    SchemaCommandResult CreateEnumeration(SchemaCommandArgs const& args) { return Execute("CreateEnumeration", args); }
    SchemaCommandResult AlterEnumeration(SchemaCommandArgs const& args) { return Execute("AlterEnumeration", args); }
    SchemaCommandResult DropEnumeration(SchemaCommandArgs const& args) { return Execute("DropEnumeration", args); }
    SchemaCommandResult AddEnumerator(SchemaCommandArgs const& args) { return Execute("AddEnumerator", args); }
    SchemaCommandResult DropEnumerator(SchemaCommandArgs const& args) { return Execute("DropEnumerator", args); }
    SchemaCommandResult AlterEnumerator(SchemaCommandArgs const& args) { return Execute("AlterEnumerator", args); }
    SchemaCommandResult SetCustomAttribute(SchemaCommandArgs const& args) { return Execute("SetCustomAttribute", args); }
    SchemaCommandResult RemoveCustomAttribute(SchemaCommandArgs const& args) { return Execute("RemoveCustomAttribute", args); }
    SchemaCommandResult CreateUnitSystem(SchemaCommandArgs const& args) { return Execute("CreateUnitSystem", args); }
    SchemaCommandResult AlterUnitSystem(SchemaCommandArgs const& args) { return Execute("AlterUnitSystem", args); }
    SchemaCommandResult CreatePhenomenon(SchemaCommandArgs const& args) { return Execute("CreatePhenomenon", args); }
    SchemaCommandResult AlterPhenomenon(SchemaCommandArgs const& args) { return Execute("AlterPhenomenon", args); }
    SchemaCommandResult CreateUnit(SchemaCommandArgs const& args) { return Execute("CreateUnit", args); }
    SchemaCommandResult AlterUnit(SchemaCommandArgs const& args) { return Execute("AlterUnit", args); }
    SchemaCommandResult CreateFormat(SchemaCommandArgs const& args) { return Execute("CreateFormat", args); }
    SchemaCommandResult AlterFormat(SchemaCommandArgs const& args) { return Execute("AlterFormat", args); }
    SchemaCommandResult CreateKindOfQuantity(SchemaCommandArgs const& args) { return Execute("CreateKindOfQuantity", args); }
    SchemaCommandResult AlterKindOfQuantity(SchemaCommandArgs const& args) { return Execute("AlterKindOfQuantity", args); }
    SchemaCommandResult DropKindOfQuantity(SchemaCommandArgs const& args) { return Execute("DropKindOfQuantity", args); }
    SchemaCommandResult CreatePropertyCategory(SchemaCommandArgs const& args) { return Execute("CreatePropertyCategory", args); }
    SchemaCommandResult AlterPropertyCategory(SchemaCommandArgs const& args) { return Execute("AlterPropertyCategory", args); }
    SchemaCommandResult DropPropertyCategory(SchemaCommandArgs const& args) { return Execute("DropPropertyCategory", args); }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
