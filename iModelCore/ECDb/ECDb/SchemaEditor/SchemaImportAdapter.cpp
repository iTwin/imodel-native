/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommandHelpers.h"
#include "SchemaImportPlanBuilder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
using Status = SchemaCommandResult::Status;

namespace {
void LogFailure(SchemaCommandResult const& result) {
    for (auto const& issue : result.GetIssues())
        LOG.errorv("SchemaImportAdapter: %s [%s%s%s]", issue.message.c_str(), issue.code.c_str(), issue.arg.empty() ? "" : " at ", issue.arg.c_str());
}

bool IsNavigation(BeJsConst property) {
    return property["kind"].asString().EqualsIAscii("navigation");
}

//! True for plan steps whose commands only create classes, root relationships (with their navigation
//! property) or non-navigation properties, or alter property metadata other than type and enumeration.
//! None of these reads a class map or instance data during preparation; a derived relationship reads
//! its base's map, a standalone navigation property its relationship's map, and type or enumeration
//! changes check instance data.
bool BatchesWithClassCreation(BeJsConst commands) {
    bool only = commands.size() != 0;
    commands.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst command) {
        Utf8String name = command["command"].asString();
        auto args = command["args"];
        if (name == "AddProperty")
            only = !IsNavigation(args);
        else if (name == "AlterProperty")
            only = !args.isMember("type") && !args.isMember("enum");
        else if (name == "CreateRelationshipClass")
            only = !args.isMember("baseClass") || args["baseClass"].asString().empty();
        else {
            only = name == "CreateEntityClass" || name == "CreateMixin" || name == "CreateStructClass" || name == "CreateCustomAttributeClass";
            if (only)
                args["properties"].ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst property) {
                    only = !IsNavigation(property);
                    return !only;
                });
        }
        return !only;
    });
    return only;
}
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaCommandResult SchemaImportAdapter::BuildPlan(ECDbR db, bvector<ECN::ECSchemaCP> const& schemas, SchemaImportPlan& plan,
                                                   SchemaImportOptions options, SchemaImportToken const* token) {
    plan = SchemaImportPlan();
    auto result = SchemaEditor::CheckEnvironment(db, options, token, false);
    if (result.IsOk())
        result = SchemaImportPlanBuilder::Build(db, schemas, plan.m_steps, plan.m_warnings);
    if (!result.IsOk()) {
        plan = SchemaImportPlan();
        LogFailure(result);
        return result;
    }
    plan.m_options = options;
    plan.m_baseline = SchemaImportPlanBuilder::ComputeBaseline(db);
    plan.m_built = true;
    return result;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaCommandResult SchemaImportAdapter::ExecutePlan(ECDbR db, SchemaImportPlan const& plan, SchemaImportCompletion* completion,
                                                     SchemaImportToken const* token) {
    if (completion != nullptr)
        completion->m_db = nullptr;
    auto fail = [](SchemaCommandResult result) { LogFailure(result); return result; };
    if (!plan.IsBuilt())
        return fail(SchemaCommandResult::Failure(Status::ERROR_INVALID_ARGS, "SchemaImport.PlanNotBuilt", "plan", "The plan was not built by SchemaImportAdapter::BuildPlan."));
    auto environment = SchemaEditor::CheckEnvironment(db, plan.m_options, token, true);
    if (!environment.IsOk())
        return fail(environment);
    if (!SchemaImportPlanBuilder::ComputeBaseline(db).Equals(plan.m_baseline))
        return fail(SchemaCommandResult::Failure(Status::ERROR_VALIDATION, "SchemaImport.StalePlan", "plan",
                                                 "The database schema changed after the plan was built (or the plan belongs to another database). Rebuild the plan."));
    for (auto const& warning : plan.GetWarnings())
        LOG.warningv("SchemaImportAdapter: %s", warning.c_str());
    if (plan.IsEmpty())
        return {};

    SchemaCommandResult beginResult;
    auto editor = SchemaEditor::Begin(db, plan.m_options, token, &beginResult);
    if (editor == nullptr)
        return fail(beginResult);
    editor->AllowStandardSchemaEdits();
    auto const& steps = plan.m_steps;
    for (size_t index = 0; index < steps.size();) {
        // Consecutive steps that only create classes, root relationships and non-navigation properties, or edit
        // property metadata, run as one group so the mapper maps their classes in a single pass, as native import
        // does, and the working graph is reused between them. Their validation reads no class maps, which do not
        // exist for classes created earlier in the same group.
        BeJsDocument batch;
        batch.SetEmptyArray();
        bvector<size_t> firstChild; // batch child index at which each merged step starts
        size_t end = index;
        for (; end < steps.size(); ++end) {
            BeJsDocument commands(steps[end].m_commands.c_str());
            if (!BatchesWithClassCreation(commands))
                break;
            firstChild.push_back(batch.size());
            commands.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst command) {
                batch.appendValue().From(command);
                return false;
            });
        }
        size_t failed = index;
        SchemaCommandResult result;
        if (end - index > 1) {
            result = editor->ExecuteGroup(batch);
            auto failedChild = result.GetFailedCommandIndex();
            if (failedChild != SIZE_MAX) {
                size_t merged = std::upper_bound(firstChild.begin(), firstChild.end(), failedChild) - firstChild.begin() - 1;
                failed = index + merged;
                // Report the child index within its own plan step, as an unmerged group would.
                result.SetFailedCommand(Utf8String(result.GetFailedCommand()), steps[failed].m_group ? failedChild - firstChild[merged] : SIZE_MAX);
            } else if (!result.IsOk()) {
                result.AddIssue({"SchemaImport.Step", "", Utf8PrintfString("Mapping plan steps %" PRIu64 "-%" PRIu64 " failed; the import was abandoned.", (uint64_t)index, (uint64_t)(end - 1))});
                return fail(result);
            }
        } else {
            BeJsDocument commands(steps[index].m_commands.c_str());
            result = steps[index].m_group ? editor->ExecuteGroup(commands) : editor->Execute(BeJsConst(commands[0]));
            end = index + 1;
        }
        if (!result.IsOk()) {
            result.AddIssue({"SchemaImport.Step", "", Utf8PrintfString("Plan step %" PRIu64 " (%s %s) failed; the import was abandoned.", (uint64_t)failed, steps[failed].m_facet.c_str(), steps[failed].m_item.c_str())});
            return fail(result);
        }
        index = end;
    }
    auto finish = editor->Finish();
    if (!finish.IsOk())
        return fail(finish);
    if (completion != nullptr)
        completion->m_db = &db;
    return finish;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaCommandResult SchemaImportAdapter::ImportSchemas(ECDbR db, bvector<ECN::ECSchemaCP> const& schemas, SchemaImportCompletion* completion,
                                                       SchemaImportOptions options, SchemaImportToken const* token) {
    SchemaImportPlan plan;
    auto result = BuildPlan(db, schemas, plan, options, token);
    if (!result.IsOk())
        return result;
    return ExecutePlan(db, plan, completion, token);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult SchemaImportCompletion::Commit() {
    if (m_db == nullptr)
        return BE_SQLITE_MISUSE;
    auto status = m_db->SaveChanges();
    if (BE_SQLITE_OK == status)
        m_db = nullptr;
    return status;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult SchemaImportCompletion::Abandon() {
    if (m_db == nullptr)
        return BE_SQLITE_MISUSE;
    auto status = m_db->AbandonChanges();
    m_db->ClearECDbCache();
    m_db->GetImpl().GetIdFactory().Reset();
    m_db = nullptr;
    return status;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
