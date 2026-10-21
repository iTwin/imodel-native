/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommandHelpers.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
using namespace SchemaCommands;

Result PrepareSchema(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, bool create) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    if (!(create ? parser.CheckKeys({"name", "alias", "version", "label", "description", "ecVersion", "customAttributes", "tablePrefix", "dynamic", "importRequiresVersion",
                                     "noAdditionalRootEntityClasses", "noAdditionalRootEntityClassesExceptions",
                                     "noAdditionalForeignKeyConstraints", "noAdditionalForeignKeyConstraintsExceptions",
                                     "noAdditionalLinkTables", "noAdditionalLinkTablesExceptions",
                                     "originalECXmlVersionMajor", "originalECXmlVersionMinor"})
                 : parser.CheckKeys({"schema", "alias", "version", "label", "description",
                                     "originalECXmlVersionMajor", "originalECXmlVersionMinor"})))
        return result;
    Utf8String name, alias, version, ecVersion = "3.2";
    if (!parser.String(create ? "name" : "schema", name, true) || !parser.String("alias", alias, create) ||
        !parser.String("version", version, create) || (create && !parser.String("ecVersion", ecVersion)))
        return result;
    ECSchemaPtr working;
    ECSchemaCP oldSchema = nullptr;
    ECSchemaId id;
    uint32_t digits[3] = {0, 0, 0};
    if (parser.Has("version")) {
        result = ParseVersion(version, digits);
        if (!result.IsOk())
            return result;
    }
    if (create) {
        if (FindSchema(ctx, name) != nullptr || FindSchema(ctx, alias) != nullptr)
            return Rejected("name", "The schema name or alias already exists.");
        ECVersion format;
        if (ecVersion.Equals("3.2"))
            format = ECVersion::V3_2;
        else if (ecVersion.Equals("3.1"))
            format = ECVersion::V3_1;
        else
            return Invalid("ecVersion", "Only ECXML 3.1 and 3.2 are supported.");
        if (format == ECVersion::V3_2 && !FeatureManager::IsEC32Available(ctx.m_db))
            return Rejected("ecVersion", "ECXML 3.2 requires a newer ECDb profile.");
        if (ECObjectsStatus::Success != ECSchema::CreateSchema(working, name, alias, digits[0], digits[1], digits[2], format))
            return Rejected("name", "Invalid schema name, alias, or version.");
    } else {
        oldSchema = FindSchema(ctx, name);
        if (oldSchema == nullptr)
            return Missing("schema", "Schema not found.");
        id = oldSchema->GetId();
        result = Copy(ctx, working, *oldSchema);
        if (!result.IsOk())
            return result;
    }
    SchemaMetaRow row;
    if (create)
        row = {{"Name", working->GetName()}, {"Alias", working->GetAlias()}, {"VersionDigit1", int64_t(digits[0])}, {"VersionDigit2", int64_t(digits[1])}, {"VersionDigit3", int64_t(digits[2])}};
    else {
        if (parser.Has("alias")) {
            auto duplicate = FindSchema(ctx, alias);
            if (duplicate != nullptr && duplicate->GetId() != id)
                return Rejected("alias", "The schema alias is already in use.");
            if (ECObjectsStatus::Success != working->SetAlias(alias))
                return Rejected("alias", "Invalid schema alias.");
            row.push_back({"Alias", alias});
        }
        if (parser.Has("version")) {
            if (digits[0] > oldSchema->GetVersionRead() &&
                Enum::Contains(ctx.m_options, SchemaImportOptions::DisallowMajorSchemaUpgrade) &&
                !(oldSchema->IsDynamicSchema() && Enum::Contains(ctx.m_options, SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas)))
                return Rejected("version", "Major schema upgrades are disabled by the session options.");
            if (digits[0] < oldSchema->GetVersionRead() ||
                (digits[0] == oldSchema->GetVersionRead() && digits[1] < oldSchema->GetVersionWrite()) ||
                (digits[0] == oldSchema->GetVersionRead() && digits[1] == oldSchema->GetVersionWrite() && digits[2] < oldSchema->GetVersionMinor()))
                return Rejected("version", "Schema versions cannot be downgraded.");
            if (ECObjectsStatus::Success != working->SetVersionRead(digits[0]) ||
                ECObjectsStatus::Success != working->SetVersionWrite(digits[1]) ||
                ECObjectsStatus::Success != working->SetVersionMinor(digits[2]))
                return Rejected("version", "Invalid schema version.");
            row.push_back({"VersionDigit1", int64_t(digits[0])});
            row.push_back({"VersionDigit2", int64_t(digits[1])});
            row.push_back({"VersionDigit3", int64_t(digits[2])});
        }
    }
    bool hasOriginalMajor = parser.Has("originalECXmlVersionMajor"), hasOriginalMinor = parser.Has("originalECXmlVersionMinor");
    if (hasOriginalMajor != hasOriginalMinor)
        return Invalid(hasOriginalMajor ? "originalECXmlVersionMinor" : "originalECXmlVersionMajor",
                       "originalECXmlVersionMajor and originalECXmlVersionMinor must be supplied together.");
    if (hasOriginalMajor) {
        uint32_t major = 0, minor = 0;
        if (!parser.UInt32("originalECXmlVersionMajor", major) || !parser.UInt32("originalECXmlVersionMinor", minor))
            return result;
        bool supportsECVersions = FeatureManager::IsAvailable(ctx.m_db, Feature::ECVersions);
        if (!supportsECVersions && (major > 3 || (major == 3 && minor >= 2)))
            return Rejected("originalECXmlVersionMajor", "Only ECXML 3.1 schemas can be persisted in a file that does not support ECXML versions.");
        if (!create) {
            // Mirrors ECDb_0432/ECDb_0433: the minor digit may only decrease together with a major increase.
            uint32_t oldMajor = oldSchema->GetOriginalECXmlVersionMajor(), oldMinor = oldSchema->GetOriginalECXmlVersionMinor();
            if (major < oldMajor)
                return Rejected("originalECXmlVersionMajor", "Decreasing 'OriginalECXmlVersionMajor' of an ECSchema is not supported.");
            if (major == oldMajor && minor < oldMinor)
                return Rejected("originalECXmlVersionMinor", "Decreasing 'OriginalECXmlVersionMinor' of an ECSchema is not supported.");
        }
        if (ECObjectsStatus::Success != working->SetOriginalECXmlVersion(major, minor))
            return Rejected("originalECXmlVersionMajor", "Invalid original ECXML version.");
        if (supportsECVersions && (create || major != oldSchema->GetOriginalECXmlVersionMajor() || minor != oldSchema->GetOriginalECXmlVersionMinor())) {
            if (create && major == 0 && minor == 0) {
                row.push_back({"OriginalECXmlVersionMajor", nullptr});
                row.push_back({"OriginalECXmlVersionMinor", nullptr});
            } else {
                row.push_back({"OriginalECXmlVersionMajor", int64_t(major)});
                row.push_back({"OriginalECXmlVersionMinor", int64_t(minor)});
            }
        }
    } else if (create && FeatureManager::IsAvailable(ctx.m_db, Feature::ECVersions)) {
        row.push_back({"OriginalECXmlVersionMajor", int64_t(3)});
        row.push_back({"OriginalECXmlVersionMinor", int64_t(ecVersion.Equals("3.2") ? 2 : 1)});
    }
    result = Labels(parser, *working, row);
    if (!result.IsOk())
        return result;
    result = Editable(ctx, *working);
    if (!result.IsOk())
        return result;
    bvector<CustomAttribute> attributes;
    if (create && parser.Has("customAttributes")) {
        result = PrepareCustomAttributes(ctx, *working, *working, parser.Value("customAttributes"), attributes);
        if (!result.IsOk())
            return result;
    }
    if (create && parser.Has("tablePrefix")) {
        Utf8String prefix;
        if (!parser.String("tablePrefix", prefix))
            return result;
        if (!ECNameValidation::IsValidName(prefix.c_str()))
            return Invalid("tablePrefix", "Expected an EC identifier.");
        BeJsDocument json;
        json["TablePrefix"] = prefix;
        result = PrepareGeneratedCustomAttribute(ctx, *working, *working, "SchemaMap", json, attributes);
        if (!result.IsOk())
            return result;
    }
    if (create && parser.Has("importRequiresVersion")) {
        Utf8String required;
        if (!parser.String("importRequiresVersion", required))
            return result;
        BeJsDocument json;
        json["ECDbRuntimeVersion"] = required;
        result = PrepareGeneratedCustomAttribute(ctx, *working, *working, "ImportRequiresVersion", json, attributes);
        if (!result.IsOk())
            return result;
    }
    if (create && parser.Has("dynamic")) {
        bool dynamic = false;
        if (!parser.Boolean("dynamic", dynamic))
            return result;
        if (working->GetCustomAttributeLocal("CoreCustomAttributes", "DynamicSchema").IsValid())
            return Invalid("dynamic", "DynamicSchema is supplied twice.");
        if (dynamic) {
            auto definition = FindClass(ctx, "CoreCustomAttributes:DynamicSchema");
            if (definition == nullptr)
                return Missing("dynamic", "The persisted DynamicSchema attribute definition is missing.");
            for (auto const& attribute : attributes)
                if (attribute.m_class == definition->GetId())
                    return Invalid("dynamic", "DynamicSchema is supplied twice.");
            CustomAttribute attribute;
            BeJsDocument json("{}");
            result = PrepareCustomAttribute(ctx, *working, *working, "CoreCustomAttributes:DynamicSchema", json, attribute);
            if (!result.IsOk())
                return result;
            attributes.push_back(attribute);
        }
    }
    if (create)
        for (auto pair : {std::make_pair("noAdditionalRootEntityClasses", "NoAdditionalRootEntityClasses"),
                          std::make_pair("noAdditionalForeignKeyConstraints", "NoAdditionalForeignKeyConstraints"),
                          std::make_pair("noAdditionalLinkTables", "NoAdditionalLinkTables")}) {
            Utf8String exceptionsKey(pair.first);
            exceptionsKey.append("Exceptions");
            if (!parser.Has(pair.first) && !parser.Has(exceptionsKey.c_str()))
                continue;
            bool enabled = false;
            bvector<Utf8String> exceptions;
            if (!parser.Boolean(pair.first, enabled, true) || !parser.StringList(exceptionsKey.c_str(), exceptions))
                return result;
            Utf8String qualified("ECDbSchemaPolicies:");
            qualified.append(pair.second);
            auto definition = FindClass(ctx, qualified);
            if (definition == nullptr)
                return Missing(pair.first, "The persisted schema policy definition is missing.");
            for (auto const& attribute : attributes)
                if (attribute.m_class == definition->GetId())
                    return Invalid(pair.first, "A schema policy is supplied twice.");
            if (!enabled) {
                if (parser.Has(exceptionsKey.c_str()))
                    return Invalid(exceptionsKey.c_str(), "Policy exceptions require the policy to be enabled.");
                continue;
            }
            BeJsDocument json("{}");
            if (!exceptions.empty()) {
                auto array = json["Exceptions"];
                array.SetEmptyArray();
                for (auto const& exception : exceptions)
                    array.appendValue() = exception;
            }
            CustomAttribute attribute;
            result = PrepareCustomAttribute(ctx, *working, *working, qualified, json, attribute);
            if (!result.IsOk())
                return result;
            attributes.push_back(attribute);
        }
    if (create)
        for (auto const& attribute : attributes) {
            auto definition = ctx.m_db.Schemas().GetClass(attribute.m_class);
            if (definition == nullptr)
                return Missing("customAttributes", "Custom attribute class is no longer persisted.");
            if (!definition->GetSchema().GetName().EqualsI("ECDbSchemaPolicies"))
                continue;
            result = NoReferences(ctx, "SELECT Id FROM main.ec_CustomAttribute WHERE ContainerType=1 AND ClassId=? LIMIT 1",
                                  attribute.m_class, "customAttributes", "Only one schema can opt in to a given schema policy.");
            if (!result.IsOk())
                return result;
        }
    if (row.empty())
        return Invalid("schema", "AlterSchema requires at least one change.");
    result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    out.m_apply = [create, id, row, attributes](SchemaEditContext& apply) mutable {
        ECSchemaId schemaId = id;
        if (create) {
            schemaId = ECSchemaId(apply.m_db.GetImpl().GetIdFactory().Schema().NextId().GetValue());
            row.push_back({"Id", int64_t(schemaId.GetValue())});
        }
        auto written = create ? apply.m_writer.Insert("ec_Schema", row) : apply.m_writer.Update("ec_Schema", schemaId, row);
        if (written.IsOk() && create)
            written = InsertCustomAttributes(apply, schemaId, schemaId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema, attributes);
        if (written.IsOk())
            Affected(apply, schemaId);
        return written;
    };
    return {};
}

Result ChangeSchemaReference(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, bool add) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    Utf8String ownerName, referenceName, version;
    if (!(add ? parser.CheckKeys({"schema", "reference", "version"}) : parser.CheckKeys({"schema", "reference"})) ||
        !parser.String("schema", ownerName, true) || !parser.String("reference", referenceName, true) ||
        (add && !parser.String("version", version)))
        return result;
    auto owner = FindSchema(ctx, ownerName);
    if (owner == nullptr)
        return Missing("schema", "Schema not found.");
    auto referenced = FindSchema(ctx, referenceName);
    if (referenced == nullptr)
        return Missing("reference", "The referenced schema does not exist in the file.");
    if (owner->GetId() == referenced->GetId())
        return Invalid("reference", "A schema cannot reference itself.");
    ECSchemaPtr working;
    result = Copy(ctx, working, *owner);
    if (!result.IsOk())
        return result;
    auto ownerId = owner->GetId(), referencedId = referenced->GetId();
    ECSchemaP workingReferenced = nullptr;
    for (auto const& entry : working->GetReferencedSchemas())
        if (entry.second->GetName().EqualsI(referenced->GetName()))
            workingReferenced = entry.second.get();
    bool exists = workingReferenced != nullptr;
    if (add) {
        if (parser.Has("version")) {
            uint32_t digits[3];
            result = ParseVersion(version, digits);
            if (!result.IsOk())
                return result;
            SchemaKey required(referenced->GetName().c_str(), digits[0], digits[1], digits[2]);
            // Mirrors ECDb_0371: the persisted referenced schema must satisfy the declared version.
            if (referenced->GetSchemaKey().LessThan(required, SchemaMatchType::Exact))
                return Rejected("version", Utf8PrintfString("The referenced schema version in the file (%s) is older than the declared reference (%s).",
                                                            referenced->GetSchemaKey().GetFullSchemaName().c_str(), required.GetFullSchemaName().c_str()));
        }
        if (!exists) {
            result = Reference(ctx, *working, *referenced);
            if (!result.IsOk())
                return result;
        }
    } else {
        if (!exists)
            return Missing("reference", "The schema does not reference this schema.");
        if (ECObjectsStatus::Success != working->RemoveReferencedSchema(*workingReferenced))
            return Rejected("reference", "The reference is still used by items of the schema; remove its consumers first.");
    }
    result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    out.m_apply = [add, ownerId, referencedId](SchemaEditContext& apply) {
        if (add) {
            apply.m_session.m_pinnedReferences.insert({ownerId, referencedId});
            auto added = apply.AddReference(ownerId, referencedId);
            if (added.IsOk())
                Affected(apply, ownerId);
            return added;
        }
        apply.m_session.m_pinnedReferences.erase({ownerId, referencedId});
        Utf8PrintfString sql("SELECT Id FROM main.ec_SchemaReference WHERE SchemaId=?1 AND ReferencedSchemaId=%" PRIu64, referencedId.GetValue());
        auto removed = DeleteSelected(apply, "ec_SchemaReference", sql.c_str(), ownerId);
        if (removed.IsOk())
            Affected(apply, ownerId);
        return removed;
    };
    return {};
}

Result DropSchema(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    Utf8String name;
    if (!parser.CheckKeys({"schema"}) || !parser.String("schema", name, true))
        return result;
    auto schema = FindSchema(ctx, name);
    if (schema == nullptr)
        return Missing("schema", "Schema not found.");
    result = Editable(ctx, *schema);
    if (!result.IsOk())
        return result;
    result = ctx.RequireMajorUpgrade(*schema);
    if (!result.IsOk())
        return result;
    result = NoReferences(ctx, "SELECT Id FROM main.ec_SchemaReference WHERE ReferencedSchemaId=? LIMIT 1",
                          schema->GetId(), "schema", "Another schema references this schema; remove the dependency first.");
    if (!result.IsOk())
        return result;
    for (auto ecClass : schema->GetClasses()) {
        if ((ecClass->IsEntityClass() && !ecClass->IsMixin()) || ecClass->IsRelationshipClass()) {
            result = ctx.CheckInstances(*ecClass);
            if (!result.IsOk())
                return result;
        }
    }
    auto id = schema->GetId();
    bvector<ECClassId> classes;
    for (auto ecClass : schema->GetClasses())
        classes.push_back(ecClass->GetId());
    out.m_apply = [id, classes](SchemaEditContext& apply) {
        apply.m_affectedClasses.insert(classes.begin(), classes.end());
        auto cleaned = DeleteSelected(apply, "ec_CustomAttribute",
                                      "SELECT ca.Id FROM main.ec_CustomAttribute ca WHERE"
                                      " (ca.ContainerType=1 AND ca.ContainerId=?1)"
                                      " OR (ca.ContainerType=30 AND ca.ContainerId IN (SELECT Id FROM main.ec_Class WHERE SchemaId=?1))"
                                      " OR (ca.ContainerType=992 AND ca.ContainerId IN (SELECT p.Id FROM main.ec_Property p JOIN main.ec_Class c ON c.Id=p.ClassId WHERE c.SchemaId=?1))"
                                      " OR (ca.ContainerType IN (1024,2048) AND ca.ContainerId IN (SELECT r.Id FROM main.ec_RelationshipConstraint r JOIN main.ec_Class c ON c.Id=r.RelationshipClassId WHERE c.SchemaId=?1))",
                                      id);
        if (!cleaned.IsOk())
            return cleaned;
        auto written = apply.m_writer.Delete("ec_Schema", id);
        if (written.IsOk())
            Affected(apply, id);
        return written;
    };
    return {};
}
}

void RegisterSchemaCommands(SchemaCommandRegistry& registry) {
    SchemaCommands::Register(registry, "CreateSchema", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return PrepareSchema(ctx, args, out, true); });
    SchemaCommands::Register(registry, "AlterSchema", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return PrepareSchema(ctx, args, out, false); });
    SchemaCommands::Register(registry, "DropSchema", DropSchema);
    SchemaCommands::Register(registry, "AddSchemaReference", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return ChangeSchemaReference(ctx, args, out, true); });
    SchemaCommands::Register(registry, "RemoveSchemaReference", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return ChangeSchemaReference(ctx, args, out, false); });
    RegisterSchemaClassCommands(registry);
    RegisterSchemaPropertyCommands(registry);
    RegisterSchemaEnumerationCommands(registry);
    RegisterSchemaCustomAttributeCommands(registry);
    RegisterSchemaRelationshipCommands(registry);
    RegisterSchemaUnitCommands(registry);
    RegisterSchemaQuantityCommands(registry);
}
END_BENTLEY_SQLITE_EC_NAMESPACE
