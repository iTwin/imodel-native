/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommandHelpers.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
using namespace SchemaCommands;
struct Constraint {
    SchemaMetaRow m_values;
    bvector<ECClassId> m_classes;
};

Result Multiplicity(Utf8StringCR text, uint32_t& lower, uint32_t& upper) {
    if (text.size() < 6 || text.front() != '(' || text.back() != ')')
        return Invalid("multiplicity", "Expected the complete multiplicity (lower..upper) or (lower..*).");
    auto separator = text.find("..");
    if (separator == Utf8String::npos || separator < 2 || separator + 2 >= text.size() - 1)
        return Invalid("multiplicity", "Expected the complete multiplicity (lower..upper) or (lower..*).");
    auto number = [](Utf8StringCR digits, uint32_t& value) {
        uint64_t parsed = 0;
        if (digits.empty())
            return false;
        for (auto ch : digits) {
            if (ch < '0' || ch > '9')
                return false;
            parsed = parsed * 10 + ch - '0';
            if (parsed > INT_MAX)
                return false;
        }
        value = static_cast<uint32_t>(parsed);
        return true;
    };
    Utf8String high = text.substr(separator + 2, text.size() - separator - 3);
    if (!number(text.substr(1, separator - 1), lower) || (!high.Equals("*") && !number(high, upper)))
        return Invalid("multiplicity", "Multiplicity bounds must be integers in [0,INT_MAX].");
    if (high.Equals("*"))
        upper = INT_MAX;
    if (upper == 0 || lower > upper)
        return Invalid("multiplicity", "Invalid multiplicity bounds.");
    return {};
}

Result PrepareConstraint(SchemaEditContext& ctx, ECSchemaR working, ECRelationshipConstraintR constraint,
                         SchemaCommandArgParser& parser, bool source, Constraint& out, bvector<ECSchemaId>& references) {
    auto classesKey = source ? "source.classes" : "target.classes";
    auto multiplicityKey = source ? "source.multiplicity" : "target.multiplicity";
    auto polymorphicKey = source ? "source.polymorphic" : "target.polymorphic";
    auto labelKey = source ? "source.roleLabel" : "target.roleLabel";
    auto abstractKey = source ? "source.abstractConstraint" : "target.abstractConstraint";
    bvector<Utf8String> names;
    Utf8String multiplicity, label, abstractName;
    bool polymorphic = true;
    if (!parser.StringList(classesKey, names, true) || !parser.String(multiplicityKey, multiplicity, true) ||
        !parser.String(labelKey, label, true) || !parser.Boolean(polymorphicKey, polymorphic) || !parser.String(abstractKey, abstractName))
        return Invalid(source ? "source" : "target", "Relationship ends require classes, multiplicity, and roleLabel with valid types.");
    if (names.empty() || label.empty())
        return Invalid(classesKey, "A relationship end requires constraint classes and a role label.");
    uint32_t lower = 0, upper = 0;
    auto result = Multiplicity(multiplicity, lower, upper);
    if (!result.IsOk())
        return result;
    if (ECObjectsStatus::Success != constraint.SetMultiplicity(RelationshipMultiplicity(lower, upper)) ||
        ECObjectsStatus::Success != constraint.SetIsPolymorphic(polymorphic) ||
        ECObjectsStatus::Success != constraint.SetRoleLabel(label))
        return Rejected(multiplicityKey, "ECObjects rejected a relationship constraint.");
    auto resolve = [&](Utf8StringCR name, ECClassCP& ecClass) {
        ecClass = FindClass(ctx, name);
        if (ecClass == nullptr || (!ecClass->IsEntityClass() && !ecClass->IsRelationshipClass()))
            return Missing(classesKey, "Relationship constraint class not found or has an invalid kind.");
        auto result = Reference(ctx, working, ecClass->GetSchema());
        if (!result.IsOk())
            return result;
        references.push_back(ecClass->GetSchema().GetId());
        ecClass = WorkingClass(working, *ecClass);
        if (ecClass == nullptr)
            return Rejected(classesKey, "Constraint class is missing from the isolated working graph.");
        return Result();
    };
    if (!abstractName.empty()) {
        ECClassCP abstractClass;
        result = resolve(abstractName, abstractClass);
        if (!result.IsOk())
            return result;
        auto status = abstractClass->IsEntityClass() ? constraint.SetAbstractConstraint(*abstractClass->GetEntityClassCP()) : constraint.SetAbstractConstraint(*abstractClass->GetRelationshipClassCP());
        if (status != ECObjectsStatus::Success)
            return Rejected(abstractKey, "Invalid abstract relationship constraint.");
        auto persisted = FindClass(ctx, abstractName);
        out.m_values.push_back({"AbstractConstraintClassId", int64_t(persisted->GetId().GetValue())});
    }
    if (names.size() == 1 && abstractName.empty()) {
        ECClassCP abstractClass;
        result = resolve(names.front(), abstractClass);
        if (!result.IsOk())
            return result;
        auto status = abstractClass->IsEntityClass() ? constraint.SetAbstractConstraint(*abstractClass->GetEntityClassCP()) : constraint.SetAbstractConstraint(*abstractClass->GetRelationshipClassCP());
        if (status != ECObjectsStatus::Success)
            return Rejected(classesKey, "Invalid implicit abstract relationship constraint.");
    }
    std::set<ECClassId> seen;
    for (auto const& name : names) {
        auto persisted = FindClass(ctx, name);
        ECClassCP ecClass;
        result = resolve(name, ecClass);
        if (!result.IsOk())
            return result;
        if (!seen.insert(persisted->GetId()).second)
            return Invalid(classesKey, "Duplicate relationship constraint class.");
        auto status = ecClass->IsEntityClass() ? constraint.AddClass(*ecClass->GetEntityClassCP()) : constraint.AddClass(*ecClass->GetRelationshipClassCP());
        if (status != ECObjectsStatus::Success)
            return Rejected(classesKey, "ECObjects rejected the relationship constraint class.");
        out.m_classes.push_back(persisted->GetId());
    }
    out.m_values.push_back({"RelationshipEnd", int64_t(source ? 0 : 1)});
    out.m_values.push_back({"MultiplicityLowerLimit", int64_t(lower)});
    out.m_values.push_back({"MultiplicityUpperLimit", upper == INT_MAX ? SchemaMetaValue(nullptr) : SchemaMetaValue(int64_t(upper))});
    out.m_values.push_back({"IsPolymorphic", int64_t(polymorphic)});
    out.m_values.push_back({"RoleLabel", label});
    return {};
}

Result InsertConstraint(SchemaEditContext& ctx, ECClassId relationship, Constraint const& constraint) {
    auto id = ctx.m_db.GetImpl().GetIdFactory().RelationshipConstraint().NextId();
    auto row = constraint.m_values;
    row.push_back({"Id", int64_t(id.GetValue())});
    row.push_back({"RelationshipClassId", int64_t(relationship.GetValue())});
    auto result = ctx.m_writer.Insert("ec_RelationshipConstraint", row);
    if (!result.IsOk())
        return result;
    for (auto ecClass : constraint.m_classes) {
        auto classId = ctx.m_db.GetImpl().GetIdFactory().RelationshipConstraintClass().NextId();
        result = ctx.m_writer.Insert("ec_RelationshipConstraintClass",
                                     {{"Id", int64_t(classId.GetValue())}, {"ConstraintId", int64_t(id.GetValue())}, {"ClassId", int64_t(ecClass.GetValue())}});
        if (!result.IsOk())
            return result;
    }
    return {};
}

Result CreateRelationship(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    if (!parser.CheckKeys({"schema", "name", "modifier", "baseClass", "label", "description", "strength", "direction", "mapAs",
                           "source.classes", "source.multiplicity", "source.polymorphic", "source.roleLabel", "source.abstractConstraint",
                           "target.classes", "target.multiplicity", "target.polymorphic", "target.roleLabel", "target.abstractConstraint",
                           "navProperty.class", "navProperty.name", "navProperty.direction", "fkOnDelete",
                           "customAttributes", "mapStrategy", "tableName", "ecInstanceIdColumn", "shareColumns", "applyToSubclassesOnly",
                           "maxSharedColumnsBeforeOverflow", "indexes", "queryView", "foreignKeyView", "useRequiresVersion",
                           "linkTable.sourceColumn", "linkTable.targetColumn",
                           "linkTable.createForeignKeyConstraints", "linkTable.allowDuplicateRelationships"}))
        return result;
    Utf8String schemaName, name, strengthName, direction, mapAs, modifierText = "None", baseName;
    if (!parser.String("schema", schemaName, true) || !parser.String("name", name, true) ||
        !parser.String("strength", strengthName, true) || !parser.String("direction", direction, true) ||
        !parser.String("mapAs", mapAs) || !parser.String("modifier", modifierText) ||
        !parser.String("baseClass", baseName))
        return result;
    ECClassCP baseRelPersisted = nullptr;
    if (!baseName.empty()) {
        baseRelPersisted = FindClass(ctx, baseName);
        if (baseRelPersisted == nullptr || !baseRelPersisted->IsRelationshipClass())
            return Missing("baseClass", "Base relationship class not found.");
        auto baseMap = ctx.m_db.Schemas().Main().GetClassMap(*baseRelPersisted);
        if (baseMap == nullptr)
            return Rejected("baseClass", "The base relationship class is not mapped.");
        Utf8CP inherited = nullptr;
        if (baseMap->GetType() == ClassMap::Type::RelationshipEndTable)
            inherited = "foreignKey";
        else if (baseMap->GetType() == ClassMap::Type::RelationshipLinkTable)
            inherited = "linkTable";
        else
            return Rejected("baseClass", "The base relationship class is not mapped as foreign key or link table.");
        if (!mapAs.empty() && !mapAs.EqualsI(inherited))
            return Rejected("mapAs", "A derived relationship inherits the mapping of its base relationship class.");
        mapAs = inherited;
    } else if (mapAs.empty())
        return Missing("mapAs", "mapAs is required for a root relationship class.");
    bool foreignKey = mapAs.EqualsI("foreignKey");
    if (baseRelPersisted != nullptr && foreignKey && parser.Has("navProperty"))
        return Rejected("navProperty", "Navigation properties may only be defined for root relationship classes (ECDb_0082).");
    if (!foreignKey && !mapAs.EqualsI("linkTable"))
        return Invalid("mapAs", "Expected foreignKey or linkTable.");
    if (!foreignKey && (parser.Has("navProperty") || parser.Has("fkOnDelete") || parser.Has("foreignKeyView")))
        return Invalid("navProperty", "Navigation and foreignKeyView arguments require mapAs=foreignKey.");
    if (foreignKey && parser.Has("linkTable"))
        return Invalid("linkTable", "Link table options require mapAs=linkTable.");
    // Read only the schema header first; the full schema is needed only for a fresh copy.
    ECSchemaCP schema = ctx.m_db.Schemas().GetSchema(schemaName, false, SchemaLookupMode::AutoDetect);
    if (schema == nullptr)
        return Missing("schema", "Schema not found.");
    ECSchemaPtr working;
    // A reused graph already passed full validation after the command that published it.
    bool validatedGraph = false;
    if (ctx.m_propertyCreationSchema.IsValid() &&
        ctx.m_propertyCreationSchema->GetSchemaKey().Matches(schema->GetSchemaKey(), SchemaMatchType::Exact)) {
        result = Editable(ctx, *schema);
        if (!result.IsOk())
            return result;
        working = ctx.m_propertyCreationSchema;
        validatedGraph = true;
    } else {
        schema = FindSchema(ctx, schemaName);
        if (schema == nullptr)
            return Missing("schema", "Schema not found.");
        result = Copy(ctx, working, *schema);
        if (!result.IsOk())
            return result;
    }
    ECRelationshipClassP relationship = nullptr;
    if (ECObjectsStatus::Success != working->CreateRelationshipClass(relationship, name, false))
        return Rejected("name", "Invalid or duplicate relationship class name.");
    StrengthType strength;
    if (strengthName.EqualsI("referencing"))
        strength = StrengthType::Referencing;
    else if (strengthName.EqualsI("holding"))
        strength = StrengthType::Holding;
    else if (strengthName.EqualsI("embedding"))
        strength = StrengthType::Embedding;
    else
        return Invalid("strength", "Expected Referencing, Holding, or Embedding.");
    ECRelatedInstanceDirection strengthDirection;
    if (direction.EqualsI("forward"))
        strengthDirection = ECRelatedInstanceDirection::Forward;
    else if (direction.EqualsI("backward"))
        strengthDirection = ECRelatedInstanceDirection::Backward;
    else
        return Invalid("direction", "Expected Forward or Backward.");
    ECClassModifier modifier;
    result = Modifier(modifierText, modifier);
    if (!result.IsOk())
        return result;
    if (ECObjectsStatus::Success != relationship->SetStrength(strength) ||
        ECObjectsStatus::Success != relationship->SetStrengthDirection(strengthDirection) ||
        ECObjectsStatus::Success != relationship->SetClassModifier(modifier))
        return Rejected("strength", "ECObjects rejected the relationship class settings.");
    SchemaMetaRow row = {{"Name", name}, {"SchemaId", int64_t(schema->GetId().GetValue())}, {"Type", int64_t(ECClassType::Relationship)}, {"Modifier", int64_t(modifier)}, {"RelationshipStrength", int64_t(strength)}, {"RelationshipStrengthDirection", int64_t(strengthDirection)}};
    result = Labels(parser, *relationship, row);
    if (!result.IsOk())
        return result;
    Constraint source, target;
    bvector<ECSchemaId> references;
    ECClassId baseRelId;
    if (parser.Has("baseClass") && !baseName.empty()) {
        auto baseRel = FindClass(ctx, baseName);
        if (baseRel == nullptr || !baseRel->IsRelationshipClass())
            return Missing("baseClass", "Base relationship class not found.");
        result = Reference(ctx, *working, baseRel->GetSchema());
        if (!result.IsOk())
            return result;
        references.push_back(baseRel->GetSchema().GetId());
        auto workingBase = WorkingClass(*working, *baseRel);
        if (workingBase == nullptr || ECObjectsStatus::Success != relationship->AddBaseClass(*workingBase))
            return Rejected("baseClass", "ECObjects rejected the base relationship class.");
        baseRelId = baseRel->GetId();
    }
    result = PrepareConstraint(ctx, *working, relationship->GetSource(), parser, true, source, references);
    if (!result.IsOk())
        return result;
    result = PrepareConstraint(ctx, *working, relationship->GetTarget(), parser, false, target, references);
    if (!result.IsOk())
        return result;
    bvector<CustomAttribute> attributes, navigationAttributes;
    result = PrepareClassMappingAttributes(ctx, *working, *relationship, parser, attributes);
    if (!result.IsOk())
        return result;
    if (parser.Has("foreignKeyView")) {
        bool fkView = false;
        if (!parser.Boolean("foreignKeyView", fkView))
            return Invalid("foreignKeyView", "Expected a boolean.");
        if (fkView) {
            BeJsDocument json;
            json.SetEmptyObject();
            result = PrepareGeneratedCustomAttribute(ctx, *working, *relationship, "ForeignKeyView", json, attributes);
            if (!result.IsOk())
                return result;
        }
    }
    ECClassId navClassId;
    ECSchemaId navSchemaId;
    ECClassCP navPersisted = nullptr;
    Utf8String navName;
    SchemaMetaRow navigationRow;
    ECSchemaPtr workingNavSchema;
    ECClassP navOwner = nullptr;
    if (foreignKey && baseRelPersisted == nullptr) {
        Utf8String navClassName, navDirectionName;
        if (!parser.String("navProperty.class", navClassName, true) || !parser.String("navProperty.name", navName, true) ||
            !parser.String("navProperty.direction", navDirectionName, true))
            return result;
        navPersisted = FindClass(ctx, navClassName);
        if (navPersisted == nullptr || !navPersisted->IsEntityClass())
            return Missing("navProperty.class", "Navigation owner entity class not found.");
        result = CheckReservedPropertyNameInHierarchy(ctx, *navPersisted, navName);
        if (!result.IsOk())
            return result;
        navSchemaId = navPersisted->GetSchema().GetId();
        ECRelationshipClassCP navTargetRel = relationship;
        if (navSchemaId == schema->GetId()) {
            navOwner = working->GetClassP(navPersisted->GetName().c_str());
        } else {
            result = Editable(ctx, navPersisted->GetSchema());
            if (!result.IsOk())
                return result;
            if (WorkingSchema(*working, navPersisted->GetSchema().GetName()) != nullptr)
                return Rejected("navProperty.class", "Adding this navigation property would create a schema reference cycle.");
            auto navContext = ECSchemaReadContext::CreateContext();
            result = CacheWorkingGraph(*working, *navContext);
            if (!result.IsOk())
                return result;
            std::set<Utf8String> visiting;
            result = CopyGraph(ctx, workingNavSchema, navPersisted->GetSchema(), *navContext, visiting);
            if (!result.IsOk())
                return result;
            if (ECObjectsStatus::Success != workingNavSchema->AddReferencedSchema(*working))
                return Rejected("navProperty.class", "Cannot reference relationship schema from navigation owner schema.");
            navOwner = workingNavSchema->GetClassP(navPersisted->GetName().c_str());
        }
        ECRelatedInstanceDirection navDirection;
        if (navDirectionName.EqualsI("Forward"))
            navDirection = ECRelatedInstanceDirection::Forward;
        else if (navDirectionName.EqualsI("Backward"))
            navDirection = ECRelatedInstanceDirection::Backward;
        else
            return Invalid("navProperty.direction", "Expected Forward or Backward.");
        NavigationECPropertyP navigation = nullptr;
        if (navOwner == nullptr || ECObjectsStatus::Success != navOwner->GetEntityClassP()->CreateNavigationProperty(navigation, navName, *navTargetRel, navDirection))
            return Rejected("navProperty", "ECObjects rejected the navigation property or its relationship constraints.");
        navClassId = navPersisted->GetId();
        navigationRow = {{"ClassId", int64_t(navClassId.GetValue())}, {"Name", navName}, {"Kind", int64_t(PropertyKind::Navigation)}, {"IsReadonly", int64_t(0)}, {"NavigationDirection", int64_t(navDirection)}};
        if (parser.Has("fkOnDelete")) {
            Utf8String action;
            if (!parser.String("fkOnDelete", action))
                return result;
            Utf8CP canonical = nullptr;
            for (auto allowed : {"Default", "NoAction", "Cascade", "SetNull"})
                if (action.EqualsI(allowed))
                    canonical = allowed;
            if (canonical == nullptr)
                return Invalid("fkOnDelete", "Expected Default, NoAction, Cascade, or SetNull.");
            BeJsDocument json;
            json.SetEmptyObject();
            if (!action.EqualsI("Default"))
                json["OnDeleteAction"] = canonical;
            result = PrepareGeneratedCustomAttribute(ctx, workingNavSchema.IsValid() ? *workingNavSchema : *working, *navigation, "ForeignKeyConstraint", json, navigationAttributes);
            if (!result.IsOk())
                return result;
        }
        for (auto const& ca : attributes) {
            auto definition = ctx.m_db.Schemas().GetClass(ca.m_class);
            if (definition != nullptr && definition->GetSchema().GetName().EqualsI("ECDbMap") &&
                (definition->GetName().EqualsI("LinkTableRelationshipMap") || definition->GetName().EqualsI("ClassMap") || definition->GetName().EqualsI("ShareColumns")))
                return Rejected("customAttributes", "Link-table class mapping attributes conflict with mapAs=foreignKey.");
        }
    } else {
        if (relationship->GetCustomAttributeLocal("ECDbMap", "ForeignKeyView").IsValid())
            return Rejected("customAttributes", "ForeignKeyView conflicts with mapAs=linkTable.");
        auto classMap = relationship->GetCustomAttributeLocal("ECDbMap", "ClassMap");
        if (classMap.IsValid()) {
            ECValue strategy;
            if (ECObjectsStatus::Success != classMap->GetValue(strategy, "MapStrategy"))
                return Rejected("customAttributes", "Cannot read the relationship ClassMap strategy.");
            if (!strategy.IsNull() && Utf8String(strategy.GetUtf8CP()).EqualsI("NotMapped"))
                return Rejected("mapAs", "mapAs=linkTable conflicts with a NotMapped ClassMap.");
        }
        BeJsDocument json;
        json.SetEmptyObject();
        for (auto pair : {std::make_pair("linkTable.sourceColumn", "SourceECInstanceIdColumn"), std::make_pair("linkTable.targetColumn", "TargetECInstanceIdColumn")})
            if (parser.Has(pair.first)) {
                Utf8String value;
                if (!parser.String(pair.first, value))
                    return result;
                json[pair.second] = value;
            }
        for (auto pair : {std::make_pair("linkTable.createForeignKeyConstraints", "CreateForeignKeyConstraints"),
                          std::make_pair("linkTable.allowDuplicateRelationships", "AllowDuplicateRelationships")})
            if (parser.Has(pair.first)) {
                bool value = false;
                if (!parser.Boolean(pair.first, value))
                    return result;
                json[pair.second] = value;
            }
        if (relationship->GetCustomAttributeLocal("ECDbMap", "LinkTableRelationshipMap").IsValid()) {
            if (parser.Has("linkTable"))
                return Invalid("linkTable", "LinkTableRelationshipMap is supplied twice.");
            if (baseRelId.IsValid())
                return Rejected("customAttributes", "Only the root relationship class of a hierarchy can have LinkTableRelationshipMap.");
        } else if (baseRelId.IsValid()) {
            if (parser.Has("linkTable"))
                return Rejected("linkTable", "Only the root relationship class of a hierarchy can specify linkTable mapping options.");
        } else if (json.size() > 0) {
            result = PrepareGeneratedCustomAttribute(ctx, *working, *relationship, "LinkTableRelationshipMap", json, attributes);
            if (!result.IsOk())
                return result;
        }
    }
    if (foreignKey && baseRelPersisted != nullptr) {
        if (parser.Has("fkOnDelete"))
            return Rejected("fkOnDelete", "fkOnDelete belongs to the root relationship navigation property.");
        for (auto const& ca : attributes) {
            auto definition = ctx.m_db.Schemas().GetClass(ca.m_class);
            if (definition != nullptr && definition->GetSchema().GetName().EqualsI("ECDbMap") &&
                (definition->GetName().EqualsI("LinkTableRelationshipMap") || definition->GetName().EqualsI("ClassMap") || definition->GetName().EqualsI("ShareColumns")))
                return Rejected("customAttributes", "Link-table class mapping attributes conflict with foreign key mapping.");
        }
    }
    if (!relationship->Verify())
        return Rejected("source", "The relationship constraints failed ECObjects verification.");
    if (validatedGraph) {
        // Only the new relationship and a same-schema navigation owner changed in the validated graph.
        result = ValidateWorkingGates(ctx, *working);
        if (result.IsOk())
            result = ctx.ValidateWorkingClassCreation(*working, *relationship);
        if (result.IsOk() && navOwner != nullptr && !workingNavSchema.IsValid())
            result = ctx.ValidateWorkingClassCreation(*working, *navOwner, navPersisted->GetId());
    } else
        result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    if (workingNavSchema.IsValid()) {
        result = ValidateWorking(ctx, *workingNavSchema);
        if (!result.IsOk())
            return result;
    }
    auto schemaId = schema->GetId();
    out.m_createsLeafClass = true;
    // The validated graph equals the persisted schema after apply. A navigation owned by another
    // schema changes that schema's graph instead, so it is not published.
    if (ctx.m_session.m_inGroup && !workingNavSchema.IsValid())
        out.m_propertyCreationSchema = working;
    out.m_apply = [schemaId, row, source, target, references, baseRelId, attributes, navigationAttributes, navClassId, navSchemaId, navigationRow, navName](SchemaEditContext& apply) mutable {
        for (auto referenced : references) {
            auto result = apply.AddReference(schemaId, referenced);
            if (!result.IsOk())
                return result;
        }
        ECClassId id(apply.m_db.GetImpl().GetIdFactory().Class().NextId().GetValue());
        row.push_back({"Id", int64_t(id.GetValue())});
        auto result = apply.m_writer.Insert("ec_Class", row);
        if (!result.IsOk())
            return result;
        apply.m_createdClasses.insert(id);
        if (baseRelId.IsValid()) {
            auto relationId = apply.m_db.GetImpl().GetIdFactory().ClassHasBaseClasses().NextId();
            result = apply.m_writer.Insert("ec_ClassHasBaseClasses",
                                           {{"Id", int64_t(relationId.GetValue())}, {"ClassId", int64_t(id.GetValue())}, {"BaseClassId", int64_t(baseRelId.GetValue())}, {"Ordinal", int64_t(0)}});
            if (!result.IsOk())
                return result;
        }
        result = InsertConstraint(apply, id, source);
        if (!result.IsOk())
            return result;
        result = InsertConstraint(apply, id, target);
        if (!result.IsOk())
            return result;
        result = InsertCustomAttributes(apply, schemaId, id, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class, attributes);
        if (!result.IsOk())
            return result;
        if (navClassId.IsValid()) {
            if (navSchemaId.IsValid() && navSchemaId != schemaId) {
                result = apply.AddReference(navSchemaId, schemaId);
                if (!result.IsOk())
                    return result;
            }
            int64_t ordinal;
            result = NextOrdinal(apply, "SELECT COALESCE(MAX(Ordinal)+1,0) FROM main.ec_Property WHERE ClassId=?", navClassId, ordinal);
            if (!result.IsOk())
                return result;
            ECPropertyId propertyId(apply.m_db.GetImpl().GetIdFactory().Property().NextId().GetValue());
            navigationRow.push_back({"Id", int64_t(propertyId.GetValue())});
            navigationRow.push_back({"Ordinal", ordinal});
            navigationRow.push_back({"NavigationRelationshipClassId", int64_t(id.GetValue())});
            result = apply.m_writer.Insert("ec_Property", navigationRow);
            if (!result.IsOk())
                return result;
            result = InsertCustomAttributes(apply, navSchemaId.IsValid() ? navSchemaId : schemaId, propertyId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property, navigationAttributes);
            if (!result.IsOk())
                return result;
            result = AffectClassAndUsers(apply, navClassId);
            if (!result.IsOk())
                return result;
        }
        Affected(apply, schemaId, id);
        return Result();
    };
    return {};
}

Result ChangeRelationship(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, bool drop) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    if (!parser.CheckKeys(drop ? std::initializer_list<Utf8CP>{"class"} : std::initializer_list<Utf8CP>{"class", "label", "description", "strength", "direction", "source.classes", "source.multiplicity", "source.polymorphic", "source.roleLabel", "source.abstractConstraint", "target.classes", "target.multiplicity", "target.polymorphic", "target.roleLabel", "target.abstractConstraint", "allowDuplicateRelationships", "indexes"}))
        return result;
    Utf8String name;
    if (!parser.String("class", name, true))
        return result;
    auto ecClass = FindClass(ctx, name);
    if (ecClass == nullptr || !ecClass->IsRelationshipClass())
        return Missing("class", "Relationship class not found.");
    result = Editable(ctx, ecClass->GetSchema());
    if (!result.IsOk())
        return result;
    auto id = ecClass->GetId();
    auto schemaId = ecClass->GetSchema().GetId();
    if (drop) {
        result = ctx.RequireMajorUpgrade(ecClass->GetSchema());
        if (!result.IsOk())
            return result;
        result = NoReferences(ctx,
                              "SELECT Id FROM main.ec_Property WHERE NavigationRelationshipClassId=?1"
                              " UNION ALL SELECT Id FROM main.ec_ClassHasBaseClasses WHERE BaseClassId=?1"
                              " UNION ALL SELECT Id FROM main.ec_RelationshipConstraintClass WHERE ClassId=?1"
                              " UNION ALL SELECT Id FROM main.ec_RelationshipConstraint WHERE AbstractConstraintClassId=?1 LIMIT 1",
                              id, "class", "The relationship is referenced by navigation properties, derived classes, or relationship constraints.");
        if (!result.IsOk())
            return result;
        result = ctx.CheckInstances(*ecClass);
        if (!result.IsOk())
            return result;
        out.m_apply = [id, schemaId](SchemaEditContext& apply) {
            auto result = DeleteClassCAs(apply, id);
            if (!result.IsOk())
                return result;
            result = apply.m_writer.Delete("ec_Class", id);
            if (result.IsOk())
                Affected(apply, schemaId, id);
            return result;
        };
        return {};
    }
    auto oldRel = ecClass->GetRelationshipClassCP();
    if (parser.Has("strength")) {
        Utf8String strengthName;
        if (!parser.String("strength", strengthName, true))
            return result;
        StrengthType strength;
        if (strengthName.EqualsI("referencing"))
            strength = StrengthType::Referencing;
        else if (strengthName.EqualsI("holding"))
            strength = StrengthType::Holding;
        else if (strengthName.EqualsI("embedding"))
            strength = StrengthType::Embedding;
        else
            return Invalid("strength", "Expected Referencing, Holding, or Embedding.");
        if (strength != oldRel->GetStrength())
            return Rejected("strength", "Changing the 'Strength' of an ECRelationshipClass is not supported.");
    }
    if (parser.Has("direction")) {
        Utf8String direction;
        if (!parser.String("direction", direction, true))
            return result;
        ECRelatedInstanceDirection strengthDirection;
        if (direction.EqualsI("forward"))
            strengthDirection = ECRelatedInstanceDirection::Forward;
        else if (direction.EqualsI("backward"))
            strengthDirection = ECRelatedInstanceDirection::Backward;
        else
            return Invalid("direction", "Expected Forward or Backward.");
        if (strengthDirection != oldRel->GetStrengthDirection())
            return Rejected("direction", "Changing the 'StrengthDirection' of an ECRelationshipClass is not supported.");
    }
    ECSchemaPtr working;
    result = Copy(ctx, working, ecClass->GetSchema());
    if (!result.IsOk())
        return result;
    auto relationship = working->GetClassP(ecClass->GetName().c_str());
    auto workingRel = relationship->GetRelationshipClassP();
    SchemaMetaRow row;
    result = Labels(parser, *relationship, row);
    if (!result.IsOk())
        return result;
    struct ConstraintUpdate {
        BeInt64Id m_constraintId;
        SchemaMetaRow m_row;
        BeInt64Id m_constraintClassRowId;
        ECClassId m_newClassId;
    };
    bvector<ConstraintUpdate> constraintUpdates;
    bvector<ECSchemaId> references;
    auto relMap = ctx.m_db.Schemas().Main().GetClassMap(*ecClass);
    for (bool isSource : {true, false}) {
        auto classesKey = isSource ? "source.classes" : "target.classes";
        auto multiplicityKey = isSource ? "source.multiplicity" : "target.multiplicity";
        auto polymorphicKey = isSource ? "source.polymorphic" : "target.polymorphic";
        auto labelKey = isSource ? "source.roleLabel" : "target.roleLabel";
        auto abstractKey = isSource ? "source.abstractConstraint" : "target.abstractConstraint";
        if (!parser.Has(classesKey) && !parser.Has(multiplicityKey) && !parser.Has(polymorphicKey) &&
            !parser.Has(labelKey) && !parser.Has(abstractKey))
            continue;
        auto const& oldConstraint = isSource ? oldRel->GetSource() : oldRel->GetTarget();
        auto& newConstraint = isSource ? workingRel->GetSource() : workingRel->GetTarget();
        Statement cStmt;
        if (BE_SQLITE_OK != cStmt.Prepare(ctx.m_db,
                                          "SELECT Id FROM main.ec_RelationshipConstraint WHERE RelationshipClassId=? AND RelationshipEnd=?") ||
            BE_SQLITE_OK != cStmt.BindId(1, id) ||
            BE_SQLITE_OK != cStmt.BindInt(2, isSource ? 0 : 1) ||
            cStmt.Step() != BE_SQLITE_ROW)
            return DatabaseError("Reading relationship constraint failed.");
        ConstraintUpdate cu;
        cu.m_constraintId = cStmt.GetValueId<BeInt64Id>(0);
        if (parser.Has(abstractKey)) {
            Utf8String abstractName;
            if (!parser.String(abstractKey, abstractName))
                return result;
            auto oldAbs = oldConstraint.GetAbstractConstraint();
            if (oldAbs == nullptr || !Utf8String(oldAbs->GetFullName()).EqualsIAscii(abstractName.c_str()))
                return Rejected(abstractKey, "Changing the abstract constraint of an existing relationship constraint is not supported.");
        }
        if (parser.Has(multiplicityKey)) {
            Utf8String multText;
            if (!parser.String(multiplicityKey, multText, true))
                return result;
            uint32_t lower = 0, upper = 0;
            result = Multiplicity(multText, lower, upper);
            if (!result.IsOk())
                return result;
            if (oldConstraint.GetMultiplicity().GetLowerLimit() != lower)
                return Rejected(multiplicityKey, "Changing 'Multiplicity' lower limit of an ECRelationshipConstraint is not supported.");
            if (oldConstraint.GetMultiplicity().GetUpperLimit() > upper)
                return Rejected(multiplicityKey, "Changing 'Multiplicity' upper limit to be less than its current value is not supported.");
            if (oldConstraint.GetMultiplicity().GetUpperLimit() < upper) {
                if (relMap != nullptr && relMap->GetType() == ClassMap::Type::RelationshipEndTable) {
                    if (isSource && relMap->GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInTargetTable)
                        return Rejected(multiplicityKey, "Changing 'Multiplicity' of a Source ECRelationshipConstraint is not supported when mapped as ForeignKeyRelationshipInTargetTable.");
                    if (!isSource && relMap->GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable)
                        return Rejected(multiplicityKey, "Changing 'Multiplicity' of a Target ECRelationshipConstraint is not supported when mapped as ForeignKeyRelationshipInSourceTable.");
                }
                if (ECObjectsStatus::Success != newConstraint.SetMultiplicity(RelationshipMultiplicity(lower, upper)))
                    return Rejected(multiplicityKey, "ECObjects rejected the relationship multiplicity.");
                cu.m_row.push_back({"MultiplicityUpperLimit", upper == INT_MAX ? SchemaMetaValue(nullptr) : SchemaMetaValue(int64_t(upper))});
            }
        }
        if (parser.Has(polymorphicKey)) {
            bool poly = false;
            if (!parser.Boolean(polymorphicKey, poly))
                return result;
            if (oldConstraint.GetIsPolymorphic() && !poly)
                return Rejected(polymorphicKey, "Changing flag 'IsPolymorphic' from 'true' to 'false' is not supported.");
            if (!oldConstraint.GetIsPolymorphic() && poly) {
                if (ECObjectsStatus::Success != newConstraint.SetIsPolymorphic(true))
                    return Rejected(polymorphicKey, "ECObjects rejected setting IsPolymorphic.");
                cu.m_row.push_back({"IsPolymorphic", int64_t(1)});
            }
        }
        if (parser.Has(labelKey)) {
            Utf8String roleLabel;
            if (!parser.String(labelKey, roleLabel, true))
                return result;
            if (!roleLabel.Equals(oldConstraint.GetRoleLabel())) {
                if (ECObjectsStatus::Success != newConstraint.SetRoleLabel(roleLabel.c_str()))
                    return Rejected(labelKey, "ECObjects rejected the relationship roleLabel.");
                cu.m_row.push_back({"RoleLabel", roleLabel});
            }
        }
        if (parser.Has(classesKey)) {
            bvector<Utf8String> names;
            if (!parser.StringList(classesKey, names, true) || names.empty())
                return Invalid(classesKey, "Constraint classes cannot be empty.");
            if (names.size() != 1)
                return Rejected(classesKey, "Updating constraint classes requires a single persisted constraint class.");
            auto newConstraintClass = FindClass(ctx, names.front());
            if (newConstraintClass == nullptr || (!newConstraintClass->IsEntityClass() && !newConstraintClass->IsRelationshipClass()))
                return Missing(classesKey, "New constraint class not found in ECDb.");
            result = Reference(ctx, *working, newConstraintClass->GetSchema());
            if (!result.IsOk())
                return result;
            references.push_back(newConstraintClass->GetSchema().GetId());
            auto workingConstraintClass = WorkingClass(*working, *newConstraintClass);
            if (workingConstraintClass == nullptr)
                return Rejected(classesKey, "Constraint class is missing from the isolated working graph.");
            bvector<ECClassCP> existingClasses(newConstraint.GetConstraintClasses().begin(), newConstraint.GetConstraintClasses().end());
            for (auto existingClass : existingClasses)
                newConstraint.RemoveClass(*existingClass);
            if (!parser.Has(abstractKey) || parser.Value(abstractKey).isNull() ||
                (parser.Value(abstractKey).isString() && Utf8String(parser.Value(abstractKey).ToUtf8CP()).empty())) {
                auto status = workingConstraintClass->IsEntityClass() ? newConstraint.SetAbstractConstraint(*workingConstraintClass->GetEntityClassCP()) : newConstraint.SetAbstractConstraint(*workingConstraintClass->GetRelationshipClassCP());
                if (status != ECObjectsStatus::Success)
                    return Rejected(classesKey, "ECObjects rejected the abstract constraint for the updated relationship constraint.");
                cu.m_row.push_back({"AbstractConstraintClassId", int64_t(newConstraintClass->GetId().GetValue())});
            }
            auto status = workingConstraintClass->IsEntityClass() ? newConstraint.AddClass(*workingConstraintClass->GetEntityClassCP()) : newConstraint.AddClass(*workingConstraintClass->GetRelationshipClassCP());
            if (status != ECObjectsStatus::Success)
                return Rejected(classesKey, "ECObjects rejected the new relationship constraint class.");
            for (auto oldConstraintClass : oldConstraint.GetConstraintClasses()) {
                auto workingOldClass = WorkingClass(*working, *oldConstraintClass);
                if (workingOldClass == nullptr) {
                    result = Reference(ctx, *working, oldConstraintClass->GetSchema());
                    if (!result.IsOk())
                        return result;
                    workingOldClass = WorkingClass(*working, *oldConstraintClass);
                }
                if (workingOldClass == nullptr || !newConstraint.SupportsClass(*workingOldClass))
                    return Rejected(classesKey, Utf8PrintfString("New constraints must support old constraint class '%s'.", oldConstraintClass->GetFullName()));
            }
            Statement ccStmt;
            if (BE_SQLITE_OK != ccStmt.Prepare(ctx.m_db,
                                               "SELECT Id,ClassId FROM main.ec_RelationshipConstraintClass WHERE ConstraintId=?") ||
                BE_SQLITE_OK != ccStmt.BindId(1, cu.m_constraintId) ||
                ccStmt.Step() != BE_SQLITE_ROW)
                return DatabaseError("Reading relationship constraint class failed.");
            if (ccStmt.GetValueId<ECClassId>(1) != newConstraintClass->GetId()) {
                cu.m_constraintClassRowId = ccStmt.GetValueId<BeInt64Id>(0);
                cu.m_newClassId = newConstraintClass->GetId();
            }
        }
        if (!cu.m_row.empty() || cu.m_constraintClassRowId.IsValid())
            constraintUpdates.push_back(std::move(cu));
    }
    bvector<CustomAttribute> attributes;
    if (parser.Has("allowDuplicateRelationships")) {
        bool allowDup = false;
        if (!parser.Boolean("allowDuplicateRelationships", allowDup))
            return result;
        auto oldLinkCA = ecClass->GetCustomAttributeLocal("ECDbMap", "LinkTableRelationshipMap");
        if (!oldLinkCA.IsValid())
            return Rejected("allowDuplicateRelationships", "allowDuplicateRelationships is only supported on link-table relationships.");
        BeJsDocument json;
        json.SetEmptyObject();
        for (Utf8CP propName : {"SourceECInstanceIdColumn", "TargetECInstanceIdColumn"}) {
            ECValue v;
            if (ECObjectsStatus::Success == oldLinkCA->GetValue(v, propName) && !v.IsNull() && v.IsString())
                json[propName] = v.GetUtf8CP();
        }
        ECValue fkVal;
        if (ECObjectsStatus::Success == oldLinkCA->GetValue(fkVal, "CreateForeignKeyConstraints") && !fkVal.IsNull() && fkVal.IsBoolean())
            json["CreateForeignKeyConstraints"] = fkVal.GetBoolean();
        json["AllowDuplicateRelationships"] = allowDup;
        result = PrepareGeneratedCustomAttribute(ctx, *working, *relationship, "LinkTableRelationshipMap", json, attributes);
        if (!result.IsOk())
            return result;
    }
    if (parser.Has("indexes")) {
        BeJsDocument json;
        result = ParseIndexesValue(parser.Value("indexes"), json);
        if (!result.IsOk())
            return result;
        result = PrepareGeneratedCustomAttribute(ctx, *working, *relationship, "DbIndexList", json, attributes);
        if (!result.IsOk())
            return result;
        auto appliedCA = relationship->GetCustomAttributeLocal("ECDbMap", "DbIndexList");
        if (appliedCA.IsValid()) {
            result = ValidateDbIndexListUpdate(ecClass, *appliedCA);
            if (!result.IsOk())
                return result;
        }
        result = ValidateWorkingClassDbIndexList(ctx, *relationship);
        if (!result.IsOk())
            return result;
    }
    if (row.empty() && attributes.empty() && constraintUpdates.empty())
        return Invalid("class", "At least one relationship attribute must change.");
    if (!workingRel->Verify())
        return Rejected("class", "The updated relationship constraints failed ECObjects verification.");
    result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    out.m_apply = [id, schemaId, row, attributes, constraintUpdates, references](SchemaEditContext& apply) {
        Result result;
        for (auto referenced : references) {
            result = apply.AddReference(schemaId, referenced);
            if (!result.IsOk())
                return result;
        }
        if (!row.empty()) {
            result = apply.m_writer.Update("ec_Class", id, row);
            if (!result.IsOk())
                return result;
        }
        for (auto const& cu : constraintUpdates) {
            if (!cu.m_row.empty()) {
                result = apply.m_writer.Update("ec_RelationshipConstraint", cu.m_constraintId, cu.m_row);
                if (!result.IsOk())
                    return result;
            }
            if (cu.m_constraintClassRowId.IsValid()) {
                result = apply.m_writer.Update("ec_RelationshipConstraintClass", cu.m_constraintClassRowId,
                                               {{"ClassId", int64_t(cu.m_newClassId.GetValue())}});
                if (!result.IsOk())
                    return result;
            }
        }
        for (auto const& attr : attributes) {
            result = UpsertClassCustomAttribute(apply, schemaId, id, attr);
            if (!result.IsOk())
                return result;
        }
        Affected(apply, schemaId, id);
        return AffectClassAndUsers(apply, id);
    };
    return {};
}
}

void RegisterSchemaRelationshipCommands(SchemaCommandRegistry& registry) {
    Register(registry, "CreateRelationshipClass", CreateRelationship);
    Register(registry, "AlterRelationshipClass", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return ChangeRelationship(ctx, args, out, false); });
    Register(registry, "DropRelationshipClass", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return ChangeRelationship(ctx, args, out, true); });
}
END_BENTLEY_SQLITE_EC_NAMESPACE
