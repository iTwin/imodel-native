/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "../FeatureManager.h"
#include "SchemaCommand.h"
#include "SchemaCommandArgParser.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
using Status = SchemaCommandResult::Status;

SchemaCommandResult SessionError(Utf8CP message) {
    return SchemaCommandResult::Failure(Status::ERROR_SESSION, "SchemaEditor.Session", "", message);
}

struct IssueCapture final {
    BeEventScope m_scope;
    bvector<SchemaCommandIssue> m_issues;
    explicit IssueCapture(ECDbCR db) {
        db.GetImpl().Issues().OnIssueObserved().AddListener(m_scope,
                                                            [this](ECN::IssueSeverity severity, ECN::IssueCategory, ECN::IssueType,
                                                                   ECN::IssueId id, Utf8CP message) {
                                                                if (severity == ECN::IssueSeverity::Warning)
                                                                    LOG.warningv("SchemaEditor [%s]: %s", id.m_issueId, message);
                                                                else if (severity == ECN::IssueSeverity::Error || severity == ECN::IssueSeverity::Fatal) {
                                                                    LOG.errorv("SchemaEditor [%s]: %s", id.m_issueId, message);
                                                                    m_issues.push_back({id.m_issueId, "", message});
                                                                }
                                                            });
    }
    void AppendTo(SchemaCommandResult& result) const {
        if (!result.IsOk())
            for (auto const& issue : m_issues)
                result.AddIssue(issue);
    }
};

void LogResultIssues(Utf8CP operation, Utf8StringCR command, SchemaCommandResult const& result) {
    if (result.IsOk())
        return;
    for (auto const& issue : result.GetIssues()) {
        if (issue.arg.empty())
            LOG.errorv("SchemaEditor::%s(%s) [%s]: %s", operation, command.c_str(), issue.code.c_str(), issue.message.c_str());
        else
            LOG.errorv("SchemaEditor::%s(%s) [%s arg=%s]: %s", operation, command.c_str(), issue.code.c_str(), issue.arg.c_str(), issue.message.c_str());
    }
}
}

struct SchemaEditor::Impl final {
    struct VersionSnapshot {
        uint32_t m_read = 0;
        uint32_t m_write = 0;
        uint32_t m_minor = 0;
    };
    enum class State { Active,
                       Executing,
                       Finished,
                       Failed };
    ECDbR m_db;
    SchemaImportOptions m_options;
    SchemaCommandRegistry m_registry;
    std::map<ECN::ECSchemaId, uint32_t> m_originalReadVersions;
    std::map<ECN::ECSchemaId, VersionSnapshot> m_originalVersions;
    std::set<ECN::ECSchemaId> m_modifiedSchemas;
    SchemaEditSessionState m_session;
    State m_state = State::Active;
    bool m_ownsSession = false;

    Impl(ECDbR db, SchemaImportOptions options) : m_db(db), m_options(options) {
        RegisterSchemaCommands(m_registry);
    }

    //! A class without derived classes adds only its own rows: itself plus its direct bases' cached ancestors.
    DbResult AppendLeafClassHierarchy(ECClassId leaf) {
        auto stmt = m_db.GetCachedStatement(
            "INSERT INTO main." TABLE_ClassHierarchyCache "(ClassId,BaseClassId) "
            "SELECT ?1,?1 UNION SELECT ?1,h.BaseClassId FROM main." TABLE_ClassHierarchyCache " h "
            "JOIN main." TABLE_ClassHasBaseClasses " b ON h.ClassId=b.BaseClassId WHERE b.ClassId=?1");
        if (stmt.IsNull())
            return BE_SQLITE_ERROR;
        stmt->BindId(1, leaf);
        auto status = stmt->Step();
        return status == BE_SQLITE_DONE ? BE_SQLITE_OK : status;
    }

    SchemaCommandResult Abort(SchemaCommandResult result) {
        m_state = State::Failed;
        m_session.m_reusableGraph = nullptr;
        auto status = m_db.AbandonChanges();
        m_db.ClearECDbCache();
        if (!m_db.GetImpl().GetIdFactory().Reset())
            result.AddIssue({"SchemaEditor.IdReset", "", "Failed to reset metadata ID sequences after abandoning the session."});
        if (status != BE_SQLITE_OK) {
            result.AddIssue({"SchemaEditor.Rollback", "", m_db.GetLastError()});
            LOG.error("SchemaEditor failed to abandon its transaction. Close and reopen the database before further schema editing.");
            // Keep the import guard set: the connection must not continue with partial metadata.
            m_ownsSession = false;
        } else
            Release();
        return result;
    }

    void Release() {
        m_session.m_reusableGraph = nullptr;
        if (m_ownsSession) {
            m_db.Schemas().Main().SetSchemaEditorActive(false);
            m_ownsSession = false;
        }
    }

    SchemaCommandResult Prepare(Utf8StringCR command, BeJsConst args,
                                SchemaEditContext& context, PreparedSchemaCommand& prepared) const {
        auto result = ValidateSchemaCommandJson(args);
        if (!result.IsOk())
            return result;
        auto entry = m_registry.find(command);
        if (entry == m_registry.end())
            return SchemaCommandResult::Failure(Status::ERROR_INVALID_ARGS,
                                                "SchemaEditor.UnknownCommand", "command", Utf8PrintfString("Unknown schema command '%s'.", command.c_str()));
        auto handler = entry->second();
        result = handler->Prepare(context, args, prepared);
        if (result.IsOk() && !prepared.m_apply)
            return SchemaCommandResult::Failure(Status::ERROR,
                                                "SchemaEditor.Command", "", "A schema command produced no executable mutation.");
        return result;
    }

    //! Applies the metadata of every child in order, then prunes references, maps the union of
    //! affected classes once and runs required data transforms. A single command is a group of one.
    //! Children of a group see the metadata of earlier children but not their physical mapping.
    SchemaCommandResult Run(bvector<std::pair<Utf8String, BeJsConst>> const& children, bool group) {
        Utf8CP operation = group ? "ExecuteGroup" : "Execute";
        if (m_state != State::Active) {
            auto result = SessionError("The schema editor is not active or is already executing a command.");
            LogResultIssues(operation, children.empty() ? Utf8String() : children.front().first, result);
            return result;
        }
        m_state = State::Executing;
        IssueCapture issues(m_db);
        SchemaEditContext context(m_db, m_options, m_originalReadVersions, m_session);
        context.m_propertyCreationSchema = std::move(m_session.m_reusableGraph);
        m_session.m_inGroup = group;
        m_session.m_groupCreatedClasses.clear();
        auto const& manager = m_db.Schemas().Main();
        bool notified = false;
        bool hierarchyStale = false;
        bool mapsClasses = false;
        SchemaCommandResult result;
        size_t failedIndex = SIZE_MAX;
        for (size_t index = 0; index < children.size() && result.IsOk(); ++index) {
            PreparedSchemaCommand prepared;
            // Only AddProperty knows how to detect and tolerate a stale reader for its target class.
            if (!children[index].first.EqualsIAscii("AddProperty"))
                context.RefreshReader();
            result = Prepare(children[index].first, children[index].second, context, prepared);
            auto hierarchyWrites = context.m_writer.m_hierarchyWrites;
            if (result.IsOk()) {
                if (!notified) {
                    manager.OnBeforeSchemaChanges().RaiseEvent(m_db, SchemaChangeType::SchemaImport);
                    notified = true;
                }
                result = prepared.m_apply(context);
            }
            if (!result.IsOk()) {
                failedIndex = index;
                break;
            }
            mapsClasses = mapsClasses || prepared.m_mapsClasses;
            if (!prepared.m_propertyCreationSchema.IsValid())
                context.m_workingSchemaPolicies.reset();
            auto propertyCreationClass = prepared.m_propertyCreationSchema.IsValid() ? prepared.m_propertyCreationClass : ECClassId();
            context.m_propertyCreationSchema = std::move(prepared.m_propertyCreationSchema);
            ECClassId createdLeafClass;
            if (prepared.m_createsLeafClass && context.m_createdClasses.size() == 1)
                createdLeafClass = *context.m_createdClasses.begin();
            m_session.m_groupCreatedClasses.insert(context.m_createdClasses.begin(), context.m_createdClasses.end());
            context.m_createdClasses.clear();
            // ClassMap retains references into SchemaReader. Invalidating just reader entries
            // would leave dangling references; keep this boundary coherent until cache
            // ownership can support selective replacement. A published property command only
            // adds or edits a local property of one class; the next child refreshes unless it is
            // an AddProperty on that same class, which reads the owned working graph instead.
            if (propertyCreationClass.IsValid())
                context.m_staleReaderClass = propertyCreationClass;
            else {
                m_db.ClearECDbCache();
                context.m_staleReaderClass.Invalidate();
            }
            // A new leaf class only adds its own hierarchy rows. Other hierarchy changes are
            // rebuilt before the next child or, after the last child, before mapping. Commands
            // that wrote no hierarchy rows leave that cache current, and Map refreshes the
            // class-has-table rows of every class it maps.
            if (createdLeafClass.IsValid()) {
                if (BE_SQLITE_OK != AppendLeafClassHierarchy(createdLeafClass)) {
                    result = SchemaCommandResult::Failure(Status::ERROR, "SchemaEditor.Cache", "", "Failed to update schema hierarchy cache.");
                    failedIndex = index;
                }
            } else if (prepared.m_requiresCacheTableRefresh && prepared.m_mapsClasses && context.m_writer.m_hierarchyWrites != hierarchyWrites) {
                if (group && index + 1 < children.size()) {
                    if (SUCCESS != manager.RepopulateCacheTables()) {
                        result = SchemaCommandResult::Failure(Status::ERROR, "SchemaEditor.Cache", "", "Failed to rebuild schema hierarchy caches.");
                        failedIndex = index;
                    }
                } else
                    hierarchyStale = true;
            }
        }
        context.RefreshReader();
        if (result.IsOk()) {
            m_modifiedSchemas.insert(context.m_affectedSchemas.begin(), context.m_affectedSchemas.end());
            bool referencesChanged = false;
            for (auto id : context.m_affectedSchemas) {
                bool changed = false;
                result = context.PruneReferences(id, &changed);
                referencesChanged = referencesChanged || changed;
                if (!result.IsOk())
                    break;
            }
            if (result.IsOk()) {
                if (referencesChanged)
                    m_db.ClearECDbCache();
                else
                    m_session.m_reusableGraph = std::move(context.m_propertyCreationSchema);
                // Map refreshes the class-has-table rows of the classes it maps.
                if (hierarchyStale && SUCCESS != manager.RepopulateCacheTables())
                    result = SchemaCommandResult::Failure(Status::ERROR, "SchemaEditor.Cache", "", "Failed to rebuild schema hierarchy caches.");
                else if (mapsClasses)
                    result = IncrementalSchemaMapper::Map(context);
            }
            m_db.ClearECDbCache();
        }
        m_session.m_inGroup = false;
        m_session.m_groupCreatedClasses.clear();
        issues.AppendTo(result);
        if (!result.IsOk()) {
            if (group) {
                if (failedIndex != SIZE_MAX)
                    result.SetFailedCommand(children[failedIndex].first, failedIndex);
                else
                    result.SetFailedCommand("ExecuteGroup");
            } else
                result.SetFailedCommand(children.empty() ? Utf8String() : children.front().first);
            LogResultIssues(operation, failedIndex != SIZE_MAX ? children[failedIndex].first : (group ? Utf8String("ExecuteGroup") : children.front().first), result);
            result = Abort(std::move(result));
            if (notified)
                manager.OnAfterSchemaChanges().RaiseEvent(m_db, SchemaChangeType::SchemaImport);
            return result;
        }
        if (notified)
            manager.OnAfterSchemaChanges().RaiseEvent(m_db, SchemaChangeType::SchemaImport);
        m_state = State::Active;
        return result;
    }

    //! Shared by SaveChanges and Finish: validates persisted mappings and reports unversioned edits.
    //! Leaves the session Executing on success; aborts the session on failure.
    SchemaCommandResult Finalize(Utf8CP operation) {
        if (m_state != State::Active)
            return SessionError("The schema editor is not active.");
        m_state = State::Executing;
        IssueCapture issues(m_db);
        m_db.ClearECDbCache();
        // Commands append and refresh cache rows incrementally; restore the native row order once.
        if (!m_modifiedSchemas.empty() && SUCCESS != m_db.Schemas().Main().RepopulateCacheTables()) {
            auto result = SchemaCommandResult::Failure(Status::ERROR, "SchemaEditor.Cache", "", "Failed to rebuild schema hierarchy caches.");
            issues.AppendTo(result);
            LogResultIssues(operation, operation, result);
            return Abort(std::move(result));
        }
        if (SUCCESS != m_db.Schemas().Main().ValidatePersistedMappings(m_options)) {
            auto result = SchemaCommandResult::Failure(Status::ERROR_VALIDATION,
                                                       "SchemaEditor.MappingValidation", "", "Persisted mappings failed validation; the session was abandoned.");
            issues.AppendTo(result);
            LogResultIssues(operation, operation, result);
            return Abort(std::move(result));
        }
        for (auto schemaId : m_modifiedSchemas) {
            auto original = m_originalVersions.find(schemaId);
            if (original == m_originalVersions.end())
                continue;
            Statement verStmt;
            if (BE_SQLITE_OK == verStmt.Prepare(m_db, "SELECT Name,VersionDigit1,VersionDigit2,VersionDigit3 FROM main.ec_Schema WHERE Id=?") &&
                BE_SQLITE_OK == verStmt.BindId(1, schemaId) && verStmt.Step() == BE_SQLITE_ROW) {
                Utf8String schemaName(verStmt.GetValueText(0));
                uint32_t v1 = uint32_t(verStmt.GetValueInt64(1));
                uint32_t v2 = uint32_t(verStmt.GetValueInt64(2));
                uint32_t v3 = uint32_t(verStmt.GetValueInt64(3));
                verStmt.Finalize();
                auto schema = m_db.Schemas().GetSchema(schemaName);
                if (schema != nullptr && !schema->IsDynamicSchema() &&
                    v1 == original->second.m_read && v2 == original->second.m_write && v3 == original->second.m_minor) {
                    LOG.warningv("Schema '%s' has changes, but its version was not incremented. Always increment the version when modifying a non-dynamic schema.",
                                 schema->GetFullSchemaName().c_str());
                }
            }
        }
        m_db.ClearECDbCache();
        return {};
    }
};

SchemaEditor::SchemaEditor(std::unique_ptr<Impl> impl) : m_impl(std::move(impl)) {}

SchemaEditor::~SchemaEditor() {
    if (m_impl->m_ownsSession)
        AbandonChanges();
}

SchemaCommandResult SchemaEditor::CheckEnvironment(ECDbR db, SchemaImportOptions options, SchemaImportToken const* token, bool requireCleanTransaction) {
    if (!db.IsDbOpen())
        return SessionError("The database must be open.");
    if (db.IsReadonly())
        return SchemaCommandResult::Failure(Status::ERROR_READONLY, "SchemaEditor.Readonly", "", "The database is read-only.");
    if (db.Schemas().Main().IsSchemaEditorActive())
        return SessionError("A schema editor session is already active on this database.");
    if (requireCleanTransaction && (db.GetCurrentSavepointDepth() != 1 || db.GetDbFile()->GetTxnState() == BE_SQLITE_TXN_WRITE))
        return SessionError("Begin requires a clean default transaction with no pending writes or nested savepoints.");
    if (!PolicyManager::GetPolicy(SchemaImportPermissionPolicyAssertion(db, token)).IsSupported())
        return SessionError("The schema import policy requires a valid SchemaImportToken.");
    if (!db.Schemas().GetSchemaSync().GetInfo().IsEmpty())
        return SessionError("SchemaEditor does not support databases configured for SchemaSync.");
    if (db.GetECDbProfileVersion().CompareTo(ECDb::CurrentECDbProfileVersion(), ProfileVersion::VERSION_MajorMinorSub1) != 0)
        return SessionError("SchemaEditor requires the current ECDb profile.");
    auto const unsupported = SchemaImportOptions::DoNotCreateOrUpdateDataTables |
                             SchemaImportOptions::DoNotFailForDeletionsOrModifications;
    if ((options & unsupported) != SchemaImportOptions::None)
        return SessionError("SchemaEditor requires physical mapping and strict deletion/modification validation.");
    return {};
}

std::unique_ptr<SchemaEditor> SchemaEditor::Begin(ECDbR db, SchemaImportOptions options,
                                                  SchemaImportToken const* token, SchemaCommandResult* out) {
    auto fail = [&](SchemaCommandResult result) -> std::unique_ptr<SchemaEditor> {
        for (auto const& issue : result.GetIssues())
            LOG.errorv("SchemaEditor::Begin: %s", issue.message.c_str());
        if (out != nullptr)
            *out = std::move(result);
        return nullptr;
    };
    if (!db.IsDbOpen())
        return fail(SessionError("The database must be open."));
    BeMutexHolder lock(db.GetImpl().GetMutex());
    auto environment = CheckEnvironment(db, options, token, true);
    if (!environment.IsOk())
        return fail(std::move(environment));
    if (!db.GetImpl().GetIdFactory().Reset())
        return fail(SessionError("Cannot initialize metadata ID sequences."));
    auto impl = std::make_unique<Impl>(db, options);
    Statement versions;
    auto versionSql = FeatureManager::IsAvailable(db, Feature::ECVersions)
        ? "SELECT Id,VersionDigit1,VersionDigit2,VersionDigit3,OriginalECXmlVersionMajor,OriginalECXmlVersionMinor FROM main.ec_Schema"
        : "SELECT Id,VersionDigit1,VersionDigit2,VersionDigit3,0,0 FROM main.ec_Schema";
    if (BE_SQLITE_OK != versions.Prepare(db, versionSql))
        return fail(SessionError("Cannot read the original schema versions."));
    DbResult status;
    while ((status = versions.Step()) == BE_SQLITE_ROW) {
        auto schemaId = versions.GetValueId<ECN::ECSchemaId>(0);
        uint32_t readVer = uint32_t(versions.GetValueInt64(1));
        impl->m_originalReadVersions.emplace(schemaId, readVer);
        impl->m_session.m_originalECXmlVersions.emplace(schemaId, std::make_pair(uint32_t(versions.GetValueInt64(4)), uint32_t(versions.GetValueInt64(5))));
        impl->m_originalVersions.emplace(schemaId, Impl::VersionSnapshot{
                                                       readVer, uint32_t(versions.GetValueInt64(2)), uint32_t(versions.GetValueInt64(3))});
    }
    if (status != BE_SQLITE_DONE)
        return fail(SessionError("Cannot read the original schema versions."));
    db.Schemas().Main().SetSchemaEditorActive(true);
    impl->m_ownsSession = true;
    if (out != nullptr)
        *out = {};
    return std::unique_ptr<SchemaEditor>(new SchemaEditor(std::move(impl)));
}

SchemaCommandResult SchemaEditor::Validate(Utf8StringCR command, SchemaCommandArgs const& args) const {
    BeJsDocument document;
    auto result = NormalizeSchemaCommandArgs(args, document);
    if (!result.IsOk()) {
        LogResultIssues("Validate", command, result);
        return result;
    }
    return Validate(command, document);
}

SchemaCommandResult SchemaEditor::Validate(Utf8StringCR command, BeJsConst args) const {
    BeMutexHolder lock(m_impl->m_db.GetImpl().GetMutex());
    if (m_impl->m_state != Impl::State::Active) {
        auto result = SessionError("The schema editor is not active.");
        LogResultIssues("Validate", command, result);
        return result;
    }
    IssueCapture issues(m_impl->m_db);
    SchemaEditContext context(m_impl->m_db, m_impl->m_options, m_impl->m_originalReadVersions, m_impl->m_session);
    PreparedSchemaCommand prepared;
    auto result = m_impl->Prepare(command, args, context, prepared);
    issues.AppendTo(result);
    LogResultIssues("Validate", command, result);
    return result;
}

SchemaCommandResult SchemaEditor::Execute(Utf8StringCR command, SchemaCommandArgs const& args) {
    BeJsDocument document;
    auto result = NormalizeSchemaCommandArgs(args, document);
    if (result.IsOk())
        return Execute(command, document);
    BeMutexHolder lock(m_impl->m_db.GetImpl().GetMutex());
    result.SetFailedCommand(command);
    LogResultIssues("Execute", command, result);
    return m_impl->m_state == Impl::State::Active ? m_impl->Abort(std::move(result))
                                                  : SessionError("The schema editor is not active.");
}

SchemaCommandResult SchemaEditor::Execute(Utf8StringCR command, BeJsConst args) {
    BeMutexHolder lock(m_impl->m_db.GetImpl().GetMutex());
    return m_impl->Run({{command, args}}, false);
}

SchemaCommandResult SchemaEditor::Execute(BeJsConst envelope) {
    SchemaCommandResult result;
    SchemaCommandArgParser parser(envelope, result);
    Utf8String command;
    parser.CheckKeys({"command", "args"});
    parser.String("command", command, true);
    if (!parser.Has("args") || !parser.Value("args").isObject())
        result = SchemaCommandResult::Failure(Status::ERROR_INVALID_ARGS,
                                              "SchemaEditor.InvalidArgs", "args", "The command envelope requires an argument object.");
    if (result.IsOk())
        return Execute(command, parser.Value("args"));
    BeMutexHolder lock(m_impl->m_db.GetImpl().GetMutex());
    result.SetFailedCommand(command);
    return m_impl->m_state == Impl::State::Active ? m_impl->Abort(std::move(result))
                                                  : SessionError("The schema editor is not active.");
}

SchemaCommandResult SchemaEditor::ExecuteBatch(BeJsConst commands) {
    BeMutexHolder lock(m_impl->m_db.GetImpl().GetMutex());
    if (m_impl->m_state != Impl::State::Active)
        return SessionError("The schema editor is not active.");
    if (!commands.isArray())
        return m_impl->Abort(SchemaCommandResult::Failure(Status::ERROR_INVALID_ARGS,
                                                          "SchemaEditor.InvalidArgs", "", "A command batch must be an array."));
    for (uint32_t index = 0; index < commands.size(); ++index) {
        auto result = Execute(commands[index]);
        if (!result.IsOk()) {
            Utf8String command;
            if (commands[index].isObject() && commands[index]["command"].isString())
                command = commands[index]["command"].asCString();
            result.SetFailedCommand(command, index);
            return result;
        }
    }
    return {};
}

SchemaCommandResult SchemaEditor::ExecuteGroup(BeJsConst commands) {
    BeMutexHolder lock(m_impl->m_db.GetImpl().GetMutex());
    if (m_impl->m_state != Impl::State::Active)
        return SessionError("The schema editor is not active.");
    SchemaCommandResult result;
    bvector<std::pair<Utf8String, BeJsConst>> children;
    if (!commands.isArray() || commands.size() == 0)
        result = SchemaCommandResult::Failure(Status::ERROR_INVALID_ARGS,
                                              "SchemaEditor.InvalidArgs", "", "A dependency group must be a non-empty array of command envelopes.");
    for (uint32_t index = 0; result.IsOk() && index < commands.size(); ++index) {
        auto envelope = commands[index];
        SchemaCommandArgParser parser(envelope, result);
        Utf8String command;
        if (!parser.CheckKeys({"command", "args"}) || !parser.String("command", command, true)) {
            result.SetFailedCommand(command, index);
            break;
        }
        if (!parser.Has("args") || !parser.Value("args").isObject()) {
            result = SchemaCommandResult::Failure(Status::ERROR_INVALID_ARGS,
                                                  "SchemaEditor.InvalidArgs", "args", "The command envelope requires an argument object.");
            result.SetFailedCommand(command, index);
            break;
        }
        if (command.EqualsI("ExecuteGroup")) {
            result = SchemaCommandResult::Failure(Status::ERROR_INVALID_ARGS,
                                                  "SchemaEditor.InvalidArgs", "command", "Dependency groups cannot be nested.");
            result.SetFailedCommand(command, index);
            break;
        }
        children.push_back({command, parser.Value("args")});
    }
    if (!result.IsOk()) {
        LogResultIssues("ExecuteGroup", "ExecuteGroup", result);
        return m_impl->Abort(std::move(result));
    }
    return m_impl->Run(children, true);
}

SchemaCommandResult SchemaEditor::SaveChanges() {
    auto& db = m_impl->m_db;
    BeMutexHolder lock(db.GetImpl().GetMutex());
    auto result = m_impl->Finalize("SaveChanges");
    if (!result.IsOk())
        return result;
    if (db.SaveChanges() != BE_SQLITE_OK) {
        result = SchemaCommandResult::Failure(Status::ERROR, "SchemaEditor.Commit", "", db.GetLastError());
        LogResultIssues("SaveChanges", "SaveChanges", result);
        return m_impl->Abort(std::move(result));
    }
    m_impl->m_state = Impl::State::Finished;
    m_impl->Release();
    return {};
}

SchemaCommandResult SchemaEditor::Finish() {
    BeMutexHolder lock(m_impl->m_db.GetImpl().GetMutex());
    auto result = m_impl->Finalize("Finish");
    if (!result.IsOk())
        return result;
    m_impl->m_state = Impl::State::Finished;
    m_impl->Release();
    return {};
}

void SchemaEditor::AllowStandardSchemaEdits() {
    m_impl->m_session.m_allowStandardSchemas = true;
}

void SchemaEditor::AbandonChanges() {
    BeMutexHolder lock(m_impl->m_db.GetImpl().GetMutex());
    if (m_impl->m_state == Impl::State::Executing) {
        LOG.error("Cannot abandon a SchemaEditor from a schema-change event callback.");
        return;
    }
    if (!m_impl->m_ownsSession)
        return;
    auto result = m_impl->Abort(SessionError("The schema editor session was abandoned."));
    for (auto const& issue : result.GetIssues())
        if (!issue.code.Equals("SchemaEditor.Session"))
            LOG.errorv("SchemaEditor::AbandonChanges: %s", issue.message.c_str());
}

bvector<Utf8String> SchemaEditor::GetCommandNames() const {
    bvector<Utf8String> names;
    for (auto const& entry : m_impl->m_registry)
        names.push_back(entry.first);
    return names;
}
END_BENTLEY_SQLITE_EC_NAMESPACE
