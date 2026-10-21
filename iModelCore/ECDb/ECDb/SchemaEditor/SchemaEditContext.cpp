/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommand.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
SchemaCommandResult DatabaseFailure(ECDbCR db, Utf8CP arg) {
    return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                        "SchemaEditor.Database", arg, db.GetLastError());
}
}

SchemaCommandResult SchemaEditContext::RequireMajorUpgrade(ECSchemaCR schema) const {
    if (schema.IsDynamicSchema() &&
        Enum::Contains(m_options, SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas))
        return {};
    auto id = SchemaPersistenceHelper::GetSchemaId(m_db, DbTableSpace::Main(),
                                                   schema.GetName().c_str(), SchemaLookupMode::ByName);
    if (!id.IsValid())
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_NOT_FOUND,
                                            "SchemaEditor.SchemaNotFound", "schema", "The schema to upgrade is not persisted.");
    auto original = m_originalReadVersions.find(id);
    if (original == m_originalReadVersions.end())
        return {}; // The schema was created in this session.
    if (Enum::Contains(m_options, SchemaImportOptions::DisallowMajorSchemaUpgrade))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                            "SchemaEditor.MajorUpgradeDisabled", "schema", "Major schema upgrades are disabled by the session options.");
    // The version must already have been persisted by a preceding AlterSchema command.
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_db, "SELECT VersionDigit1 FROM main.ec_Schema WHERE Id=?") ||
        BE_SQLITE_OK != stmt.BindId(1, id))
        return DatabaseFailure(m_db, "schema");
    if (stmt.Step() != BE_SQLITE_ROW)
        return DatabaseFailure(m_db, "schema");
    if (stmt.GetValueInt64(0) <= original->second)
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                            "SchemaEditor.MajorUpgradeRequired", "schema",
                                            Utf8PrintfString("AlterSchema must increase the read version of '%s' above %" PRIu32 " before this operation.",
                                                             schema.GetName().c_str(), original->second));
    return {};
}

SchemaCommandResult SchemaEditContext::AddReference(ECSchemaId owner, ECSchemaId referenced) {
    if (!owner.IsValid() || !referenced.IsValid())
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_NOT_FOUND,
                                            "SchemaEditor.ReferenceNotFound", "schema", "Both referenced schemas must already exist in the database.");
    if (owner == referenced)
        return {};
    Statement check;
    if (BE_SQLITE_OK != check.Prepare(m_db,
                                      "SELECT EXISTS(SELECT 1 FROM main.ec_Schema WHERE Id=?),"
                                      "EXISTS(SELECT 1 FROM main.ec_Schema WHERE Id=?),"
                                      "EXISTS(SELECT 1 FROM main.ec_SchemaReference WHERE SchemaId=? AND ReferencedSchemaId=?)") ||
        BE_SQLITE_OK != check.BindId(1, owner) ||
        BE_SQLITE_OK != check.BindId(2, referenced) ||
        BE_SQLITE_OK != check.BindId(3, owner) ||
        BE_SQLITE_OK != check.BindId(4, referenced) || check.Step() != BE_SQLITE_ROW)
        return DatabaseFailure(m_db, "schema");
    if (!check.GetValueBoolean(0) || !check.GetValueBoolean(1))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_NOT_FOUND,
                                            "SchemaEditor.ReferenceNotFound", "schema", "Both referenced schemas must already exist in the database.");
    if (check.GetValueBoolean(2))
        return {};
    Statement cycle;
    if (BE_SQLITE_OK != cycle.Prepare(m_db,
                                      "WITH RECURSIVE reachable(Id) AS (SELECT ? UNION "
                                      "SELECT r.ReferencedSchemaId FROM main.ec_SchemaReference r JOIN reachable q ON q.Id=r.SchemaId) "
                                      "SELECT 1 FROM reachable WHERE Id=?") ||
        BE_SQLITE_OK != cycle.BindId(1, referenced) ||
        BE_SQLITE_OK != cycle.BindId(2, owner))
        return DatabaseFailure(m_db, "schema");
    auto status = cycle.Step();
    if (status == BE_SQLITE_ROW)
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                            "SchemaEditor.ReferenceCycle", "schema", "The cross-schema dependency would introduce a schema reference cycle.");
    if (status != BE_SQLITE_DONE)
        return DatabaseFailure(m_db, "schema");
    auto id = m_db.GetImpl().GetIdFactory().SchemaReference().NextId();
    return m_writer.Insert("ec_SchemaReference", {{"Id", int64_t(id.GetValue())}, {"SchemaId", int64_t(owner.GetValue())}, {"ReferencedSchemaId", int64_t(referenced.GetValue())}});
}

SchemaCommandResult SchemaEditContext::PruneReferences(ECSchemaId owner, bool* changed) {
    if (changed != nullptr)
        *changed = false;
    Statement ownerStatement;
    if (BE_SQLITE_OK != ownerStatement.Prepare(m_db, "SELECT Name FROM main.ec_Schema WHERE Id=?") ||
        BE_SQLITE_OK != ownerStatement.BindId(1, owner))
        return DatabaseFailure(m_db, "schema");
    auto ownerStatus = ownerStatement.Step();
    if (ownerStatus == BE_SQLITE_DONE)
        return {}; // DropSchema removed the owner.
    if (ownerStatus != BE_SQLITE_ROW)
        return DatabaseFailure(m_db, "schema");
    Utf8String name(ownerStatement.GetValueText(0));
    ownerStatement.Finalize();
    Statement select;
    if (BE_SQLITE_OK != select.Prepare(m_db,
                                       "SELECT Id,ReferencedSchemaId FROM main.ec_SchemaReference WHERE SchemaId=?") ||
        BE_SQLITE_OK != select.BindId(1, owner))
        return DatabaseFailure(m_db, "schema");
    bvector<std::pair<BeInt64Id, ECSchemaId>> candidates;
    DbResult status;
    while ((status = select.Step()) == BE_SQLITE_ROW) {
        auto referenced = select.GetValueId<ECSchemaId>(1);
        if (m_session.m_pinnedReferences.count({owner, referenced}) == 0)
            candidates.push_back({select.GetValueId<BeInt64Id>(0), referenced});
    }
    if (status != BE_SQLITE_DONE)
        return DatabaseFailure(m_db, "schema");
    select.Finalize();
    if (candidates.empty())
        return {};
    auto schema = m_db.Schemas().GetSchema(name);
    if (schema == nullptr)
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.Reload", "schema", "Cannot reload the edited schema to resolve its references.");
    ECSchemaPtr copy;
    if (ECObjectsStatus::Success != schema->CopySchema(copy))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.Copy", "schema", "Cannot copy the edited schema to resolve its references.");
    copy->RemoveUnusedSchemaReferences();
    std::set<ECSchemaId> used;
    for (auto const& entry : copy->GetReferencedSchemas()) {
        auto id = SchemaPersistenceHelper::GetSchemaId(m_db, DbTableSpace::Main(),
                                                       entry.second->GetName().c_str(), SchemaLookupMode::ByName);
        if (!id.IsValid())
            return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                                "SchemaEditor.ReferenceNotFound", "schema", "An in-memory schema reference has no persisted schema.");
        used.insert(id);
    }
    bvector<BeInt64Id> unused;
    for (auto const& candidate : candidates)
        if (used.count(candidate.second) == 0)
            unused.push_back(candidate.first);
    for (auto id : unused) {
        auto result = m_writer.Delete("ec_SchemaReference", id);
        if (!result.IsOk())
            return result;
    }
    if (changed != nullptr)
        *changed = !unused.empty();
    return {};
}

SchemaCommandResult SchemaEditContext::CheckInstances(ECClassCR ecClass, Utf8CP property) const {
    auto map = m_db.Schemas().Main().GetClassMap(ecClass);
    if (map == nullptr)
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.MappingNotFound", "class", "Cannot inspect instance data without a class map.");
    if (map->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        return {};
    Utf8String query = Utf8PrintfString("SELECT ECClassId,ECInstanceId FROM [%s].[%s]",
                                        ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str());
    if (property != nullptr) {
        if (ecClass.GetPropertyP(property) == nullptr || strchr(property, ']') != nullptr)
            return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_NOT_FOUND,
                                                "SchemaEditor.PropertyNotFound", "property", "The property to inspect does not exist.");
        query.append(Utf8PrintfString(" WHERE [%s] IS NOT NULL", property));
    }
    ECSqlStatement statement;
    if (ECSqlStatus::Success != statement.Prepare(m_db, query.c_str()))
        return DatabaseFailure(m_db, property == nullptr ? "class" : "property");
    ECInstanceKeyMultiMap instances;
    DbResult status;
    while ((status = statement.Step()) == BE_SQLITE_ROW)
        instances.insert(ECInstanceKeyMultiMapPair(statement.GetValueId<ECClassId>(0), statement.GetValueId<ECInstanceId>(1)));
    if (status != BE_SQLITE_DONE)
        return DatabaseFailure(m_db, property == nullptr ? "class" : "property");
    if (instances.empty())
        return {};
    auto result = SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_DATA_DELETION_REQUIRED,
                                               "SchemaEditor.DataDeletionRequired", property == nullptr ? "class" : "property",
                                               property == nullptr ? "Instances must be deleted before dropping the schema item."
                                                                   : "Non-null property values must be cleared before dropping the property.");
    result.SetBlockingInstances(std::move(instances));
    return result;
}

SchemaCommandResult SchemaEditContext::ValidateWorkingSchema(ECSchemaR schema) {
    if (!schema.Validate(false))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                            "SchemaEditor.SchemaValidation", "", "The proposed schema does not satisfy ECObjects validation.");
    if (!SchemaValidator::ValidateSchemas(m_mapping, m_db.GetImpl().Issues(), {&schema}))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                            "SchemaEditor.ECDbValidation", "", "The proposed schema does not satisfy ECDb schema validation.");
    SchemaPolicies const* policiesP = nullptr;
    auto result = WorkingSchemaPolicies(policiesP);
    if (!result.IsOk())
        return result;
    auto schemaId = AssignWorkingSchemaId(schema);
    for (auto ecClass : schema.GetClasses()) {
        auto classId = SchemaPersistenceHelper::GetClassId(m_db, DbTableSpace::Main(),
                                                           schemaId, ecClass->GetName().c_str());
        result = CheckClassPolicies(*policiesP, *ecClass, classId);
        if (!result.IsOk())
            return result;
    }
    return {};
}

ECSchemaId SchemaEditContext::AssignWorkingSchemaId(ECSchemaR schema) {
    auto schemaId = SchemaPersistenceHelper::GetSchemaId(m_db, DbTableSpace::Main(),
                                                         schema.GetName().c_str(), SchemaLookupMode::ByName);
    // Assign only identities that already exist. This lets policy exception lists work on
    // a detached copy without allocating IDs during a dry run.
    if (schemaId.IsValid() && !schema.HasId())
        schema.SetId(schemaId);
    return schemaId;
}

SchemaCommandResult SchemaEditContext::CheckClassPolicies(SchemaPolicies const& policies, ECClassR ecClass, ECClassId classId) {
    SchemaPolicy const* policy = nullptr;
    if (!classId.IsValid()) {
        if (auto entity = ecClass.GetEntityClassCP()) {
            if (policies.IsOptedIn(policy, SchemaPolicy::Type::NoAdditionalRootEntityClasses) &&
                SUCCESS != policy->GetAs<NoAdditionalRootEntityClassesPolicy>().Evaluate(m_db, *entity))
                return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                                    "SchemaEditor.Policy", "baseClass", "The new entity class violates NoAdditionalRootEntityClasses.");
        }
        if (auto rel = ecClass.GetRelationshipClassCP()) {
            if (rel->GetCustomAttributeLocal("ECDbMap", "LinkTableRelationshipMap").IsValid() &&
                policies.IsOptedIn(policy, SchemaPolicy::Type::NoAdditionalLinkTables) &&
                SUCCESS != policy->GetAs<NoAdditionalLinkTablesPolicy>().Evaluate(m_db, *rel))
                return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                                    "SchemaEditor.Policy", "mapAs", "The relationship class violates NoAdditionalLinkTables.");
        }
    }
    for (auto prop : ecClass.GetProperties(false)) {
        auto result = CheckForeignKeyPolicy(policies, ecClass, classId, *prop);
        if (!result.IsOk())
            return result;
    }
    return {};
}

SchemaCommandResult SchemaEditContext::ValidateWorkingClassCreation(ECSchemaR schema, ECClassR ecClass, ECClassId persisted) {
    // The graph passed full validation after the previous command, and a new class cannot
    // change the validity of other classes. Class-level ECObjects failures fall back to full
    // validation so ECObjects' version-dependent handling stays authoritative.
    if (!ecClass.Validate())
        return ValidateWorkingSchema(schema);
    if (!SchemaValidator::ValidateClass(m_mapping, m_db.GetImpl().Issues(), ecClass))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                            "SchemaEditor.ECDbValidation", "", "The proposed schema does not satisfy ECDb schema validation.");
    SchemaPolicies const* policies = nullptr;
    auto result = WorkingSchemaPolicies(policies);
    if (!result.IsOk())
        return result;
    AssignWorkingSchemaId(schema);
    return CheckClassPolicies(*policies, ecClass, persisted);
}

SchemaCommandResult SchemaEditContext::WorkingSchemaPolicies(SchemaPolicies const*& policies) {
    if (m_workingSchemaPolicies == nullptr) {
        auto read = std::make_unique<SchemaPolicies>();
        if (SUCCESS != read->ReadPolicies(m_db))
            return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                                "SchemaEditor.Policy", "schema", "The persisted schema policies are invalid.");
        m_workingSchemaPolicies = std::move(read);
    }
    policies = m_workingSchemaPolicies.get();
    return {};
}

SchemaCommandResult SchemaEditContext::CheckForeignKeyPolicy(SchemaPolicies const& policies, ECClassR ecClass,
                                                             ECClassId classId, ECPropertyCR prop) {
    SchemaPolicy const* policy = nullptr;
    if (!prop.GetIsNavigation() ||
        !prop.GetCustomAttributeLocal("ECDbMap", "ForeignKeyConstraint").IsValid() ||
        !policies.IsOptedIn(policy, SchemaPolicy::Type::NoAdditionalForeignKeyConstraints))
        return {};
    auto propId = classId.IsValid() ? SchemaPersistenceHelper::GetPropertyId(m_db, DbTableSpace::Main(),
                                                                             classId, prop.GetName().c_str())
                                    : ECPropertyId();
    if (propId.IsValid())
        return {};
    if (classId.IsValid() && !ecClass.HasId())
        ecClass.SetId(classId);
    if (SUCCESS != policy->GetAs<NoAdditionalForeignKeyConstraintsPolicy>().Evaluate(m_db, *prop.GetAsNavigationProperty()))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                            "SchemaEditor.Policy", "fkOnDelete", "The navigation property violates NoAdditionalForeignKeyConstraints.");
    return {};
}

SchemaCommandResult SchemaEditContext::ValidateWorkingPropertyCreation(ECSchemaR schema, ECClassR ecClass,
                                                                       ECClassId classId, ECPropertyCR prop) {
    // The graph passed full validation after the previous command. A new local property, or a change
    // that keeps a property's type and enumeration, cannot change base-class, mixin or
    // relationship-constraint validity; KoQ override compatibility is checked when the KoQ is set, so only the property
    // rules and its policy need evaluation. Any class-level failure falls back to full validation
    // so ECObjects' version-dependent handling stays authoritative.
    auto relationship = ecClass.GetRelationshipClassCP();
    if (relationship != nullptr ? !relationship->Verify() : !ecClass.Validate())
        return ValidateWorkingSchema(schema);
    if (!SchemaValidator::ValidateProperty(m_db.GetImpl().Issues(), ecClass, prop))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_VALIDATION,
                                            "SchemaEditor.ECDbValidation", "", "The proposed schema does not satisfy ECDb schema validation.");
    SchemaPolicies const* policies = nullptr;
    auto result = WorkingSchemaPolicies(policies);
    if (!result.IsOk())
        return result;
    return CheckForeignKeyPolicy(*policies, ecClass, classId, prop);
}
END_BENTLEY_SQLITE_EC_NAMESPACE
