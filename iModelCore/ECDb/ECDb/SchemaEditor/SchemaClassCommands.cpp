/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommandHelpers.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
using namespace SchemaCommands;

Result PrepareClass(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, bool create, bool entity) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    if (!(create ? parser.CheckKeys({"schema", "name", "modifier", "baseClass", "mixins", "label", "description", "customAttributes",
                                     "mapStrategy", "tableName", "ecInstanceIdColumn", "shareColumns", "applyToSubclassesOnly",
                                     "maxSharedColumnsBeforeOverflow", "joinedTablePerDirectSubclass", "queryView", "useRequiresVersion", "indexes", "properties"})
                 : parser.CheckKeys(entity ? std::initializer_list<Utf8CP>{"class", "modifier", "baseClass", "label", "description", "indexes"} : std::initializer_list<Utf8CP>{"class", "modifier", "baseClass", "label", "description"})))
        return result;
    Utf8String owner, name, modifierText, baseName;
    if (!parser.String(create ? "schema" : "class", owner, true) ||
        (create && !parser.String("name", name, true)) || !parser.String("modifier", modifierText) ||
        !parser.String("baseClass", baseName))
        return result;
    ECSchemaCP schema;
    ECClassCP oldClass = nullptr;
    ECSchemaPtr working;
    // A reused graph already passed full validation after the command that published it.
    bool validatedGraph = false;
    if (create) {
        // Read only the schema header first; the full schema is needed only for a fresh copy.
        schema = ctx.m_db.Schemas().GetSchema(owner, false, SchemaLookupMode::AutoDetect);
        if (schema == nullptr)
            return Missing("schema", "Schema not found.");
        if (ctx.m_propertyCreationSchema.IsValid() &&
            ctx.m_propertyCreationSchema->GetSchemaKey().Matches(schema->GetSchemaKey(), SchemaMatchType::Exact)) {
            result = Editable(ctx, *schema);
            if (!result.IsOk())
                return result;
            working = ctx.m_propertyCreationSchema;
            validatedGraph = true;
        } else {
            schema = FindSchema(ctx, owner);
            if (schema == nullptr)
                return Missing("schema", "Schema not found.");
        }
    } else {
        oldClass = FindClass(ctx, owner);
        if (oldClass == nullptr)
            return Missing("class", "Class not found; use Schema:Class.");
        if (entity ? !oldClass->IsEntityClass() || oldClass->IsMixin() : !oldClass->IsStructClass())
            return Rejected("class", "The command does not match the persisted class kind.");
        schema = &oldClass->GetSchema();
        name = oldClass->GetName();
    }
    if (!validatedGraph)
        result = Copy(ctx, working, *schema);
    if (!result.IsOk())
        return result;
    ECClassP ecClass = nullptr;
    if (create) {
        if (entity) {
            ECEntityClassP created = nullptr;
            if (ECObjectsStatus::Success != working->CreateEntityClass(created, name))
                return Rejected("name", "Invalid or duplicate entity class name.");
            ecClass = created;
        } else {
            ECStructClassP created = nullptr;
            if (ECObjectsStatus::Success != working->CreateStructClass(created, name))
                return Rejected("name", "Invalid or duplicate struct class name.");
            ecClass = created;
        }
    } else
        ecClass = working->GetClassP(name.c_str());
    if (ecClass == nullptr)
        return Rejected("class", "Class is missing from the working schema.");
    SchemaMetaRow row;
    if (create)
        row = {{"SchemaId", int64_t(schema->GetId().GetValue())}, {"Name", name}, {"Type", int64_t(ecClass->GetClassType())}, {"Modifier", int64_t(ecClass->GetClassModifier())}};
    if (parser.Has("modifier")) {
        ECClassModifier modifier;
        result = Modifier(modifierText, modifier);
        if (!result.IsOk())
            return result;
        if (!create && modifier != oldClass->GetClassModifier()) {
            if (modifier == ECClassModifier::Sealed) {
                result = NoReferences(ctx, "SELECT Id FROM main.ec_ClassHasBaseClasses WHERE BaseClassId=? LIMIT 1",
                                      oldClass->GetId(), "modifier", "A class with derived classes cannot be sealed.");
                if (!result.IsOk())
                    return result;
            }
            if (modifier == ECClassModifier::Abstract && entity) {
                result = ctx.RequireMajorUpgrade(*schema);
                if (!result.IsOk())
                    return result;
                result = ctx.CheckInstances(*oldClass);
                if (!result.IsOk())
                    return result;
            }
            if (oldClass->GetClassModifier() == ECClassModifier::Abstract && modifier != ECClassModifier::Abstract && entity) {
                auto classMap = ctx.m_db.Schemas().Main().GetClassMap(*oldClass);
                if (classMap == nullptr || classMap->GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy)
                    return Rejected("modifier", "Changing modifier from Abstract requires MapStrategy::TablePerHierarchy.");
                Statement subStmt;
                if (BE_SQLITE_OK != subStmt.Prepare(ctx.m_db,
                                                    "SELECT c.Id FROM main.ec_cache_ClassHierarchy h JOIN main.ec_Class c ON c.Id=h.ClassId"
                                                    " WHERE h.BaseClassId=? AND h.ClassId<>? AND c.Modifier=1 LIMIT 1") ||
                    BE_SQLITE_OK != subStmt.BindId(1, oldClass->GetId()) ||
                    BE_SQLITE_OK != subStmt.BindId(2, oldClass->GetId()))
                    return DatabaseError("Preparing derived abstract class query failed.");
                if (subStmt.Step() == BE_SQLITE_ROW)
                    return Rejected("modifier", "Cannot change modifier from Abstract when an abstract derived class exists.");
            }
        }
        if (ECObjectsStatus::Success != ecClass->SetClassModifier(modifier))
            return Rejected("modifier", "The class modifier is invalid.");
        if (create)
            row.back().second = int64_t(modifier);
        else
            row.push_back({"Modifier", int64_t(modifier)});
    }
    ECClassCP base = nullptr;
    BeInt64Id replacedBaseId;
    int64_t baseOrdinal = 0;
    bvector<DerivedPropertyTypeUpgrade> derivedUpgrades;
    if (parser.Has("baseClass")) {
        if (baseName.empty())
            return Unsupported("baseClass", "Removing an entity base class is not supported; its inherited storage must be retained.");
        base = FindClass(ctx, baseName);
        if (base == nullptr)
            return Missing("baseClass", "Base class not found; use Schema:Class.");
        if (entity ? !base->IsEntityClass() || base->IsMixin() : !base->IsStructClass())
            return Rejected("baseClass", "Base class has an incompatible kind.");
        result = Reference(ctx, *working, base->GetSchema());
        if (!result.IsOk())
            return result;
        if (!create) {
            ECClassCP previous = nullptr;
            for (auto candidate : oldClass->GetBaseClasses())
                if (!candidate->IsMixin())
                    previous = candidate;
            if (!entity || previous == nullptr || base->GetId() == previous->GetId() || !base->Is(previous))
                return Unsupported("baseClass", "An existing entity base may only be specialized to one of its derived classes.");
            result = ValidateAndPrepareBaseClassPropertyOverrides(ctx, *working, *oldClass, *base, previous, derivedUpgrades);
            if (!result.IsOk())
                return result;
            auto workingPrevious = WorkingClass(*working, *previous);
            if (workingPrevious == nullptr)
                return Rejected("baseClass", "Previous base is missing from the isolated working graph.");
            if (ECObjectsStatus::Success != ecClass->RemoveBaseClass(*workingPrevious))
                return Rejected("baseClass", "ECObjects rejected removing the previous base from the working copy.");
            Statement stmt;
            if (BE_SQLITE_OK != stmt.Prepare(ctx.m_db,
                                             "SELECT Id,Ordinal FROM main.ec_ClassHasBaseClasses WHERE ClassId=? AND BaseClassId=?") ||
                BE_SQLITE_OK != stmt.BindId(1, oldClass->GetId()) || BE_SQLITE_OK != stmt.BindId(2, previous->GetId()) ||
                stmt.Step() != BE_SQLITE_ROW)
                return DatabaseError("Reading the previous base-class relationship failed.");
            replacedBaseId = BeInt64Id(stmt.GetValueUInt64(0));
            baseOrdinal = stmt.GetValueInt64(1);
        }
        ECClassCP workingBase = WorkingClass(*working, *base);
        if (workingBase == nullptr)
            return Rejected("baseClass", "Base is missing from the isolated working graph.");
        if (ECObjectsStatus::Success != ecClass->AddBaseClass(*workingBase, !create))
            return Rejected("baseClass", "Base class violates inheritance constraints.");
    }
    bvector<std::pair<ECClassId, ECSchemaId>> mixins;
    if (create && parser.Has("mixins")) {
        if (!entity)
            return Invalid("mixins", "Only entity classes support mixins.");
        bvector<Utf8String> names;
        if (!parser.StringList("mixins", names, true))
            return result;
        std::set<ECClassId> seen;
        for (auto const& mixinName : names) {
            auto mixin = FindClass(ctx, mixinName);
            if (mixin == nullptr || !mixin->IsMixin())
                return Missing("mixins", "Mixin class not found.");
            if (!seen.insert(mixin->GetId()).second)
                return Invalid("mixins", "A mixin cannot be specified twice.");
            result = Reference(ctx, *working, mixin->GetSchema());
            if (!result.IsOk())
                return result;
            auto workingMixin = WorkingClass(*working, *mixin);
            if (workingMixin == nullptr)
                return Rejected("mixins", "Mixin is missing from the isolated working graph.");
            if (ECObjectsStatus::Success != ecClass->AddBaseClass(*workingMixin))
                return Rejected("mixins", "A mixin is incompatible with the class or its base class.");
            mixins.push_back({mixin->GetId(), mixin->GetSchema().GetId()});
        }
    }
    result = Labels(parser, *ecClass, row);
    if (!result.IsOk())
        return result;
    bvector<PreparedProperty> properties;
    if (create && parser.Has("properties")) {
        result = PrepareInlineProperties(ctx, parser.Value("properties"), *working, *ecClass, schema->GetId(), properties);
        if (!result.IsOk())
            return result;
    }
    bvector<CustomAttribute> attributes;
    if (create) {
        result = PrepareClassMappingAttributes(ctx, *working, *ecClass, parser, attributes);
        if (!result.IsOk())
            return result;
    } else if (parser.Has("indexes")) {
        BeJsDocument json;
        result = ParseIndexesValue(parser.Value("indexes"), json);
        if (!result.IsOk())
            return result;
        result = PrepareGeneratedCustomAttribute(ctx, *working, *ecClass, "DbIndexList", json, attributes);
        if (!result.IsOk())
            return result;
        auto appliedCA = ecClass->GetCustomAttributeLocal("ECDbMap", "DbIndexList");
        if (appliedCA.IsValid()) {
            result = ValidateDbIndexListUpdate(oldClass, *appliedCA);
            if (!result.IsOk())
                return result;
        }
        result = ValidateWorkingClassDbIndexList(ctx, *ecClass);
        if (!result.IsOk())
            return result;
    }
    for (auto property : ecClass->GetProperties()) {
        result = CheckReservedPropertyName(*ecClass, property->GetName());
        if (!result.IsOk())
            return result;
    }
    if (!create && row.empty() && !replacedBaseId.IsValid() && attributes.empty())
        return Invalid("class", "At least one class attribute must change.");
    if (validatedGraph) {
        result = ValidateWorkingGates(ctx, *working);
        if (result.IsOk())
            result = ctx.ValidateWorkingClassCreation(*working, *ecClass);
    } else
        result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    auto schemaId = schema->GetId();
    auto classId = create ? ECClassId() : oldClass->GetId();
    auto baseId = base == nullptr ? ECClassId() : base->GetId();
    auto baseSchemaId = base == nullptr ? ECSchemaId() : base->GetSchema().GetId();
    auto oldClassId = oldClass == nullptr ? ECClassId() : oldClass->GetId();
    out.m_createsLeafClass = create;
    if (create && ctx.m_session.m_inGroup) {
        // The validated graph equals the persisted schema after apply; following AddProperty
        // commands reuse it instead of reading and copying the whole schema again.
        out.m_propertyCreationSchema = working;
    }
    out.m_apply = [create, schemaId, classId, baseId, baseSchemaId, row, attributes, replacedBaseId, baseOrdinal, oldClassId, mixins, derivedUpgrades, properties](SchemaEditContext& apply) mutable {
        if (baseSchemaId.IsValid()) {
            auto reference = apply.AddReference(schemaId, baseSchemaId);
            if (!reference.IsOk())
                return reference;
        }
        for (auto const& mixin : mixins) {
            auto reference = apply.AddReference(schemaId, mixin.second);
            if (!reference.IsOk())
                return reference;
        }
        ECClassId id = classId;
        if (create) {
            id = ECClassId(apply.m_db.GetImpl().GetIdFactory().Class().NextId().GetValue());
            row.push_back({"Id", int64_t(id.GetValue())});
        }
        Result written;
        if (replacedBaseId.IsValid()) {
            written = AffectClassAndUsers(apply, id);
            if (!written.IsOk())
                return written;
            for (auto const& upg : derivedUpgrades) {
                if (upg.m_enumSchemaId.IsValid()) {
                    auto refRes = apply.AddReference(upg.m_ownerSchemaId, upg.m_enumSchemaId);
                    if (!refRes.IsOk())
                        return refRes;
                }
                auto upgRes = apply.m_writer.Update("ec_Property", upg.m_propertyId, upg.m_row);
                if (!upgRes.IsOk())
                    return upgRes;
            }
            auto oldClassPtr = apply.m_db.Schemas().GetClass(oldClassId);
            auto basePtr = apply.m_db.Schemas().GetClass(baseId);
            if (oldClassPtr != nullptr && basePtr != nullptr)
                apply.m_mapping.RemapManager().CollectRemapInfosFromModifiedBaseClass(*oldClassPtr, *basePtr);
            written = apply.m_writer.Delete("ec_ClassHasBaseClasses", replacedBaseId);
            if (!written.IsOk())
                return written;
        }
        if (create)
            written = apply.m_writer.Insert("ec_Class", row);
        else if (!row.empty())
            written = apply.m_writer.Update("ec_Class", id, row);
        if (!written.IsOk())
            return written;
        if (create)
            apply.m_createdClasses.insert(id);
        if (create) {
            written = InsertCustomAttributes(apply, schemaId, id, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class, attributes);
            if (!written.IsOk())
                return written;
        } else {
            for (auto const& attr : attributes) {
                written = UpsertClassCustomAttribute(apply, schemaId, id, attr);
                if (!written.IsOk())
                    return written;
            }
        }
        if (baseId.IsValid()) {
            auto relationId = apply.m_db.GetImpl().GetIdFactory().ClassHasBaseClasses().NextId();
            written = apply.m_writer.Insert("ec_ClassHasBaseClasses",
                                            {{"Id", int64_t(relationId.GetValue())}, {"ClassId", int64_t(id.GetValue())}, {"BaseClassId", int64_t(baseId.GetValue())}, {"Ordinal", baseOrdinal}});
            if (!written.IsOk())
                return written;
        }
        int64_t ordinal = baseId.IsValid() ? 1 : 0;
        for (auto const& mixin : mixins) {
            auto relationId = apply.m_db.GetImpl().GetIdFactory().ClassHasBaseClasses().NextId();
            written = apply.m_writer.Insert("ec_ClassHasBaseClasses",
                                            {{"Id", int64_t(relationId.GetValue())}, {"ClassId", int64_t(id.GetValue())}, {"BaseClassId", int64_t(mixin.first.GetValue())}, {"Ordinal", ordinal++}});
            if (!written.IsOk())
                return written;
        }
        written = ApplyInlineProperties(apply, id, properties);
        if (!written.IsOk())
            return written;
        Affected(apply, schemaId, id);
        return AffectClassAndUsers(apply, id);
    };
    return {};
}

Result DropClass(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, bool entity) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    Utf8String name;
    if (!parser.CheckKeys({"class"}) || !parser.String("class", name, true))
        return result;
    auto ecClass = FindClass(ctx, name);
    if (ecClass == nullptr)
        return Missing("class", "Class not found.");
    if (entity ? !ecClass->IsEntityClass() || ecClass->IsMixin() : !ecClass->IsStructClass())
        return Rejected("class", "The command does not match the persisted class kind.");
    result = Editable(ctx, ecClass->GetSchema());
    if (!result.IsOk())
        return result;
    result = ctx.RequireMajorUpgrade(ecClass->GetSchema());
    if (!result.IsOk())
        return result;
    if (entity) {
        result = ctx.CheckInstances(*ecClass);
        if (!result.IsOk())
            return result;
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(ctx.m_db,
                                         "SELECT DISTINCT ca.ContainerId FROM main.ec_CustomAttribute ca JOIN main.ec_Class c ON c.Id=ca.ClassId"
                                         " JOIN main.ec_Schema s ON s.Id=c.SchemaId WHERE ca.ContainerType=30 AND s.Name='CoreCustomAttributes' AND c.Name='IsMixin'"))
            return DatabaseError("Preparing mixin appliesTo dependency query failed.");
        DbResult step;
        while ((step = stmt.Step()) == BE_SQLITE_ROW) {
            auto candidate = ctx.m_db.Schemas().GetClass(stmt.GetValueId<ECClassId>(0));
            if (candidate == nullptr || !candidate->IsMixin())
                return DatabaseError("Resolving a persisted mixin dependency failed.");
            auto appliesTo = candidate->GetEntityClassCP()->GetAppliesToClass();
            if (appliesTo == nullptr)
                return Rejected("class", "A persisted mixin has an unresolved appliesTo dependency.");
            if (appliesTo->GetId() == ecClass->GetId())
                return Rejected("class", "A mixin's appliesTo constraint references this entity class.");
        }
        if (step != BE_SQLITE_DONE)
            return DatabaseError("Reading mixin appliesTo dependencies failed.");
    }
    result = NoReferences(ctx, "SELECT ClassId FROM main.ec_ClassHasBaseClasses WHERE BaseClassId=? LIMIT 1",
                          ecClass->GetId(), "class", "The class has derived classes. Remove them first.");
    if (!result.IsOk())
        return result;
    result = NoReferences(ctx,
                          "SELECT Id FROM main.ec_Property WHERE StructClassId=?1 OR NavigationRelationshipClassId=?1"
                          " UNION ALL SELECT Id FROM main.ec_RelationshipConstraint WHERE AbstractConstraintClassId=?1"
                          " UNION ALL SELECT Id FROM main.ec_RelationshipConstraintClass WHERE ClassId=?1 LIMIT 1",
                          ecClass->GetId(), "class", "The class is referenced by a property or relationship constraint.");
    if (!result.IsOk())
        return result;
    auto id = ecClass->GetId();
    auto schemaId = ecClass->GetSchema().GetId();
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

Result CreateMixin(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    Utf8String owner, name, appliesToName, baseName;
    if (!parser.CheckKeys({"schema", "name", "appliesTo", "baseClass", "label", "description", "properties"}) ||
        !parser.String("schema", owner, true) || !parser.String("name", name, true) ||
        !parser.String("appliesTo", appliesToName, true) || !parser.String("baseClass", baseName))
        return result;
    auto schema = FindSchema(ctx, owner);
    auto appliesTo = FindClass(ctx, appliesToName);
    if (schema == nullptr)
        return Missing("schema", "Schema not found.");
    if (appliesTo == nullptr || !appliesTo->IsEntityClass() || appliesTo->IsMixin())
        return Missing("appliesTo", "The appliesTo entity class was not found.");
    ECSchemaPtr working;
    result = Copy(ctx, working, *schema);
    if (!result.IsOk())
        return result;
    result = Reference(ctx, *working, appliesTo->GetSchema());
    if (!result.IsOk())
        return result;
    ECEntityClassP mixin = nullptr;
    if (ECObjectsStatus::Success != working->CreateEntityClass(mixin, name) ||
        ECObjectsStatus::Success != mixin->SetClassModifier(ECClassModifier::Abstract))
        return Rejected("name", "Invalid or duplicate mixin class name.");
    ECClassCP base = nullptr;
    if (parser.Has("baseClass")) {
        base = FindClass(ctx, baseName);
        if (base == nullptr || !base->IsMixin())
            return Missing("baseClass", "A mixin can derive only from an existing mixin.");
        result = Reference(ctx, *working, base->GetSchema());
        if (!result.IsOk())
            return result;
    }
    BeJsDocument json;
    auto workingAppliesTo = WorkingClass(*working, *appliesTo);
    if (workingAppliesTo == nullptr)
        return Rejected("appliesTo", "The appliesTo class is missing from the isolated working graph.");
    json["AppliesToEntityClass"] = ECClass::GetQualifiedClassName(*working, *workingAppliesTo);
    CustomAttribute attribute;
    result = PrepareCustomAttribute(ctx, *working, *mixin, "CoreCustomAttributes:IsMixin", json, attribute);
    if (!result.IsOk())
        return result;
    if (base != nullptr) {
        auto workingBase = WorkingClass(*working, *base);
        if (workingBase == nullptr)
            return Rejected("baseClass", "Mixin base is missing from the isolated working graph.");
        if (ECObjectsStatus::Success != mixin->AddBaseClass(*workingBase))
            return Rejected("baseClass", "The mixin base class is incompatible with appliesTo.");
    }
    SchemaMetaRow row = {{"Name", name}, {"SchemaId", int64_t(schema->GetId().GetValue())}, {"Type", int64_t(ECClassType::Entity)}, {"Modifier", int64_t(ECClassModifier::Abstract)}};
    result = Labels(parser, *mixin, row);
    if (!result.IsOk())
        return result;
    bvector<PreparedProperty> properties;
    if (parser.Has("properties")) {
        result = PrepareInlineProperties(ctx, parser.Value("properties"), *working, *mixin, schema->GetId(), properties);
        if (!result.IsOk())
            return result;
    }
    result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    auto schemaId = schema->GetId();
    auto appliesSchemaId = appliesTo->GetSchema().GetId();
    auto baseId = base == nullptr ? ECClassId() : base->GetId();
    auto baseSchemaId = base == nullptr ? ECSchemaId() : base->GetSchema().GetId();
    out.m_createsLeafClass = true;
    out.m_apply = [schemaId, appliesSchemaId, baseId, baseSchemaId, attribute, row, properties](SchemaEditContext& apply) mutable {
        auto result = apply.AddReference(schemaId, appliesSchemaId);
        if (!result.IsOk())
            return result;
        if (baseSchemaId.IsValid()) {
            result = apply.AddReference(schemaId, baseSchemaId);
            if (!result.IsOk())
                return result;
        }
        ECClassId id(apply.m_db.GetImpl().GetIdFactory().Class().NextId().GetValue());
        row.push_back({"Id", int64_t(id.GetValue())});
        result = apply.m_writer.Insert("ec_Class", row);
        if (!result.IsOk())
            return result;
        apply.m_createdClasses.insert(id);
        if (baseId.IsValid()) {
            auto relation = apply.m_db.GetImpl().GetIdFactory().ClassHasBaseClasses().NextId();
            result = apply.m_writer.Insert("ec_ClassHasBaseClasses",
                                           {{"Id", int64_t(relation.GetValue())}, {"ClassId", int64_t(id.GetValue())}, {"BaseClassId", int64_t(baseId.GetValue())}, {"Ordinal", int64_t(0)}});
            if (!result.IsOk())
                return result;
        }
        result = InsertCustomAttributes(apply, schemaId, id, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class, {attribute});
        if (result.IsOk())
            result = ApplyInlineProperties(apply, id, properties);
        if (result.IsOk())
            Affected(apply, schemaId, id);
        return result;
    };
    return {};
}

ECPropertyCP RootProperty(ECPropertyCP property) {
    while (property != nullptr && property->GetBaseProperty() != nullptr)
        property = property->GetBaseProperty();
    return property;
}

bool SamePropertyDefinition(ECPropertyCR first, ECPropertyCR second) {
    return first.GetName().EqualsI(second.GetName().c_str()) &&
           first.GetClass().GetName().EqualsI(second.GetClass().GetName().c_str()) &&
           first.GetClass().GetSchema().GetName().EqualsI(second.GetClass().GetSchema().GetName().c_str());
}

Result PrepareMixinPropertyRemoval(SchemaEditContext& ctx, ECClassCR target, ECClassCR mixin, PropertyMapRemoval& removal) {
    bvector<ECClassId> descendants;
    auto result = DerivedClassIds(ctx, target.GetId(), descendants);
    if (!result.IsOk())
        return result;
    for (auto id : descendants) {
        auto owner = ctx.m_db.Schemas().GetClass(id);
        if (owner == nullptr)
            return DatabaseError("Resolving a mixin's descendant failed.");
        ECSchemaPtr working;
        result = Copy(ctx, working, owner->GetSchema());
        if (!result.IsOk())
            return result;
        auto workingTarget = WorkingClass(*working, target);
        auto workingMixin = WorkingClass(*working, mixin);
        auto workingOwner = WorkingClass(*working, *owner);
        if (workingTarget == nullptr || workingMixin == nullptr || workingOwner == nullptr ||
            ECObjectsStatus::Success != workingTarget->RemoveBaseClass(*workingMixin))
            return Rejected("mixin", "The isolated descendant schema cannot remove the mixin.");
        for (auto property : mixin.GetProperties(true)) {
            auto previous = owner->GetPropertyP(property->GetName().c_str());
            if (previous == nullptr)
                return Rejected("mixin", "A mixin property is missing from its consumer.");
            auto retained = workingOwner->GetPropertyP(property->GetName().c_str());
            if (retained != nullptr && SamePropertyDefinition(*RootProperty(previous), *RootProperty(retained)))
                continue;
            result = CheckPropertyData(ctx, *owner, {previous});
            if (!result.IsOk())
                return result;
            if (previous->GetIsNavigation() && retained == nullptr)
                return Unsupported("mixin", "Removing this inherited navigation mapping would orphan the relationship's foreign-key class map; RemapManager has no foreign-key relationship unmapping collector.");
            result = removal.Select(ctx, id, previous->GetName());
            if (!result.IsOk())
                return result;
        }
        result = ValidateWorking(ctx, *working);
        if (!result.IsOk())
            return result;
    }
    return {};
}

Result ChangeMixin(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, bool remove) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    Utf8String className, mixinName;
    if (!parser.CheckKeys({"class", "mixin"}) || !parser.String("class", className, true) || !parser.String("mixin", mixinName, true))
        return result;
    auto ecClass = FindClass(ctx, className);
    auto mixin = FindClass(ctx, mixinName);
    if (ecClass == nullptr || !ecClass->IsEntityClass() || ecClass->IsMixin())
        return Missing("class", "Entity class not found.");
    if (mixin == nullptr || !mixin->IsMixin())
        return Missing("mixin", "Mixin class not found.");
    BeInt64Id relationId;
    {
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(ctx.m_db, "SELECT Id FROM main.ec_ClassHasBaseClasses WHERE ClassId=? AND BaseClassId=?") ||
            BE_SQLITE_OK != stmt.BindId(1, ecClass->GetId()) || BE_SQLITE_OK != stmt.BindId(2, mixin->GetId()))
            return DatabaseError("Preparing mixin lookup failed.");
        auto step = stmt.Step();
        if (step == BE_SQLITE_ROW)
            relationId = BeInt64Id(stmt.GetValueUInt64(0));
        else if (step != BE_SQLITE_DONE)
            return DatabaseError("Reading mixin lookup failed.");
    }
    if (remove && !relationId.IsValid())
        return Missing("mixin", "Mixin is not applied directly to this class.");
    if (!remove && relationId.IsValid())
        return Rejected("mixin", "Mixin is already applied directly to this class.");
    ECSchemaPtr working;
    result = Copy(ctx, working, ecClass->GetSchema());
    if (!result.IsOk())
        return result;
    result = Reference(ctx, *working, mixin->GetSchema());
    if (!result.IsOk())
        return result;
    auto workingClass = working->GetClassP(ecClass->GetName().c_str());
    auto workingMixin = WorkingClass(*working, *mixin);
    if (workingClass == nullptr || workingMixin == nullptr)
        return Rejected("mixin", "Mixin is missing from the isolated working graph.");
    PropertyMapRemoval removal;
    bvector<DerivedPropertyTypeUpgrade> derivedUpgrades;
    if (remove) {
        result = ctx.RequireMajorUpgrade(ecClass->GetSchema());
        if (!result.IsOk())
            return result;
        if (mixin->GetPropertyCount() != 0) {
            result = PrepareMixinPropertyRemoval(ctx, *ecClass, *mixin, removal);
            if (!result.IsOk())
                return result;
        }
        if (ECObjectsStatus::Success != workingClass->RemoveBaseClass(*workingMixin))
            return Rejected("mixin", "ECObjects rejected removing the mixin.");
    } else {
        result = ValidateAndPrepareBaseClassPropertyOverrides(ctx, *working, *ecClass, *mixin, nullptr, derivedUpgrades);
        if (!result.IsOk())
            return result;
        if (ECObjectsStatus::Success != workingClass->AddBaseClass(*workingMixin))
            return Rejected("mixin", "The mixin is not compatible with the entity class or its existing bases.");
    }
    if (!remove)
        for (auto property : mixin->GetProperties()) {
            result = CheckReservedPropertyNameInHierarchy(ctx, *ecClass, property->GetName());
            if (!result.IsOk())
                return result;
        }
    result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    auto classId = ecClass->GetId();
    auto schemaId = ecClass->GetSchema().GetId();
    auto mixinId = mixin->GetId();
    auto mixinSchema = mixin->GetSchema().GetId();
    out.m_apply = [remove, relationId, ecClass, mixin, classId, schemaId, mixinId, mixinSchema, removal, derivedUpgrades](SchemaEditContext& apply) {
        auto result = AffectClassAndUsers(apply, classId);
        if (!result.IsOk())
            return result;
        if (remove) {
            // Snapshot surviving bases before the edge disappears. Disappearing
            // paths contain no data and must not enter the override restore queue.
            for (auto base : ecClass->GetBaseClasses())
                if (base->GetId() != mixinId)
                    apply.m_mapping.RemapManager().CollectRemapInfosFromModifiedBaseClass(*ecClass, *base);
            result = removal.Apply(apply);
            if (!result.IsOk())
                return result;
            result = apply.m_writer.Delete("ec_ClassHasBaseClasses", relationId);
        } else {
            for (auto const& upg : derivedUpgrades) {
                if (upg.m_enumSchemaId.IsValid()) {
                    auto refRes = apply.AddReference(upg.m_ownerSchemaId, upg.m_enumSchemaId);
                    if (!refRes.IsOk())
                        return refRes;
                }
                auto upgRes = apply.m_writer.Update("ec_Property", upg.m_propertyId, upg.m_row);
                if (!upgRes.IsOk())
                    return upgRes;
            }
            result = apply.AddReference(schemaId, mixinSchema);
            if (!result.IsOk())
                return result;
            int64_t ordinal;
            result = NextOrdinal(apply, "SELECT COALESCE(MAX(Ordinal)+1,0) FROM main.ec_ClassHasBaseClasses WHERE ClassId=?", classId, ordinal);
            if (!result.IsOk())
                return result;
            apply.m_mapping.RemapManager().CollectRemapInfosFromModifiedBaseClass(*ecClass, *mixin);
            auto id = apply.m_db.GetImpl().GetIdFactory().ClassHasBaseClasses().NextId();
            result = apply.m_writer.Insert("ec_ClassHasBaseClasses",
                                           {{"Id", int64_t(id.GetValue())}, {"ClassId", int64_t(classId.GetValue())}, {"BaseClassId", int64_t(mixinId.GetValue())}, {"Ordinal", ordinal}});
        }
        if (result.IsOk())
            Affected(apply, schemaId, classId);
        return result;
    };
    return {};
}
Result ParseCustomAttributeContainerMask(SchemaCommandArgParser& parser, CustomAttributeContainerType& outMask) {
    outMask = CustomAttributeContainerType::Any;
    if (!parser.Has("appliesTo"))
        return {};
    bvector<Utf8String> tokens;
    auto val = parser.Value("appliesTo");
    if (val.isString()) {
        Utf8String raw(val.ToUtf8CP());
        size_t start = 0;
        for (size_t i = 0; i <= raw.size(); ++i)
            if (i == raw.size() || raw[i] == ',' || raw[i] == '|') {
                Utf8String part = raw.substr(start, i - start);
                part.Trim();
                if (!part.empty())
                    tokens.push_back(part);
                start = i + 1;
            }
    } else if (!parser.StringList("appliesTo", tokens, true))
        return Invalid("appliesTo", "Expected a container type string or array.");
    if (tokens.empty())
        return Invalid("appliesTo", "appliesTo cannot be empty.");
    int mask = 0;
    for (auto const& token : tokens) {
        if (token.EqualsI("Schema"))
            mask |= int(CustomAttributeContainerType::Schema);
        else if (token.EqualsI("EntityClass"))
            mask |= int(CustomAttributeContainerType::EntityClass);
        else if (token.EqualsI("CustomAttributeClass"))
            mask |= int(CustomAttributeContainerType::CustomAttributeClass);
        else if (token.EqualsI("StructClass"))
            mask |= int(CustomAttributeContainerType::StructClass);
        else if (token.EqualsI("RelationshipClass"))
            mask |= int(CustomAttributeContainerType::RelationshipClass);
        else if (token.EqualsI("AnyClass"))
            mask |= int(CustomAttributeContainerType::AnyClass);
        else if (token.EqualsI("PrimitiveProperty"))
            mask |= int(CustomAttributeContainerType::PrimitiveProperty);
        else if (token.EqualsI("StructProperty"))
            mask |= int(CustomAttributeContainerType::StructProperty);
        else if (token.EqualsI("PrimitiveArrayProperty"))
            mask |= int(CustomAttributeContainerType::PrimitiveArrayProperty);
        else if (token.EqualsI("StructArrayProperty"))
            mask |= int(CustomAttributeContainerType::StructArrayProperty);
        else if (token.EqualsI("NavigationProperty"))
            mask |= int(CustomAttributeContainerType::NavigationProperty);
        else if (token.EqualsI("AnyProperty"))
            mask |= int(CustomAttributeContainerType::AnyProperty);
        else if (token.EqualsI("SourceRelationshipConstraint"))
            mask |= int(CustomAttributeContainerType::SourceRelationshipConstraint);
        else if (token.EqualsI("TargetRelationshipConstraint"))
            mask |= int(CustomAttributeContainerType::TargetRelationshipConstraint);
        else if (token.EqualsI("AnyRelationshipConstraint"))
            mask |= int(CustomAttributeContainerType::AnyRelationshipConstraint);
        else if (token.EqualsI("Any"))
            mask |= int(CustomAttributeContainerType::Any);
        else
            return Invalid("appliesTo", "Unknown CustomAttributeContainerType token.");
    }
    outMask = static_cast<CustomAttributeContainerType>(mask);
    return {};
}

Result PrepareCustomAttributeClass(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, bool create) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    if (!(create ? parser.CheckKeys({"schema", "name", "modifier", "appliesTo", "baseClass", "label", "description", "customAttributes", "properties"}) : parser.CheckKeys({"class", "modifier", "appliesTo", "label", "description"})))
        return result;
    Utf8String owner, name, modifierText, baseName;
    if (!parser.String(create ? "schema" : "class", owner, true) ||
        (create && !parser.String("name", name, true)) || !parser.String("modifier", modifierText) ||
        (create && !parser.String("baseClass", baseName)))
        return result;
    ECSchemaCP schema = nullptr;
    ECClassCP oldClass = nullptr;
    if (create) {
        schema = FindSchema(ctx, owner);
        if (schema == nullptr)
            return Missing("schema", "Schema not found.");
    } else {
        oldClass = FindClass(ctx, owner);
        if (oldClass == nullptr)
            return Missing("class", "Class not found; use Schema:Class.");
        if (!oldClass->IsCustomAttributeClass())
            return Rejected("class", "The command does not match the persisted class kind.");
        schema = &oldClass->GetSchema();
        name = oldClass->GetName();
    }
    result = Editable(ctx, *schema);
    if (!result.IsOk())
        return result;
    ECSchemaPtr working;
    result = Copy(ctx, working, *schema);
    if (!result.IsOk())
        return result;
    ECCustomAttributeClassP caClass = nullptr;
    if (create) {
        if (ECObjectsStatus::Success != working->CreateCustomAttributeClass(caClass, name))
            return Rejected("name", "Invalid or duplicate custom attribute class name.");
    } else {
        auto existing = working->GetClassP(name.c_str());
        caClass = existing != nullptr ? existing->GetCustomAttributeClassP() : nullptr;
    }
    if (caClass == nullptr)
        return Rejected("class", "Custom attribute class missing from working schema.");
    SchemaMetaRow row;
    if (create)
        row = {{"SchemaId", int64_t(schema->GetId().GetValue())}, {"Name", name}, {"Type", int64_t(ECClassType::CustomAttribute)}, {"Modifier", int64_t(caClass->GetClassModifier())}};
    if (parser.Has("modifier")) {
        ECClassModifier modifier;
        result = Modifier(modifierText, modifier);
        if (!result.IsOk())
            return result;
        if (!create && modifier != oldClass->GetClassModifier()) {
            if (modifier == ECClassModifier::Sealed) {
                result = NoReferences(ctx, "SELECT Id FROM main.ec_ClassHasBaseClasses WHERE BaseClassId=? LIMIT 1",
                                      oldClass->GetId(), "modifier", "A class with derived classes cannot be sealed.");
                if (!result.IsOk())
                    return result;
            }
            if (modifier == ECClassModifier::Abstract) {
                result = ctx.RequireMajorUpgrade(*schema);
                if (!result.IsOk())
                    return result;
            }
        }
        if (ECObjectsStatus::Success != caClass->SetClassModifier(modifier))
            return Rejected("modifier", "The class modifier is invalid.");
        if (create)
            row.back().second = int64_t(modifier);
        else
            row.push_back({"Modifier", int64_t(modifier)});
    }
    if (create || parser.Has("appliesTo")) {
        CustomAttributeContainerType mask = CustomAttributeContainerType::Any;
        result = ParseCustomAttributeContainerMask(parser, mask);
        if (!result.IsOk())
            return result;
        if (!create && oldClass->GetCustomAttributeClassCP() != nullptr) {
            int oldMask = int(oldClass->GetCustomAttributeClassCP()->GetContainerType());
            int newMask = int(mask);
            if ((oldMask & newMask) != oldMask) {
                result = ctx.RequireMajorUpgrade(*schema);
                if (!result.IsOk())
                    return result;
            }
        }
        caClass->SetContainerType(mask);
        if (mask != CustomAttributeContainerType::Any)
            row.push_back({"CustomAttributeContainerType", int64_t(mask)});
        else if (!create)
            row.push_back({"CustomAttributeContainerType", SchemaMetaValue(nullptr)});
    }
    ECClassCP base = nullptr;
    if (create && parser.Has("baseClass")) {
        base = FindClass(ctx, baseName);
        if (base == nullptr || !base->IsCustomAttributeClass())
            return Missing("baseClass", "Base custom attribute class not found.");
        result = Reference(ctx, *working, base->GetSchema());
        if (!result.IsOk())
            return result;
        auto workingBase = WorkingClass(*working, *base);
        if (workingBase == nullptr || ECObjectsStatus::Success != caClass->AddBaseClass(*workingBase))
            return Rejected("baseClass", "Base class violates inheritance constraints.");
    }
    result = Labels(parser, *caClass, row);
    if (!result.IsOk())
        return result;
    bvector<PreparedProperty> properties;
    if (create && parser.Has("properties")) {
        result = PrepareInlineProperties(ctx, parser.Value("properties"), *working, *caClass, schema->GetId(), properties);
        if (!result.IsOk())
            return result;
    }
    bvector<CustomAttribute> attributes;
    if (create && parser.Has("customAttributes")) {
        result = PrepareCustomAttributes(ctx, *working, *caClass, parser.Value("customAttributes"), attributes);
        if (!result.IsOk())
            return result;
    }
    if (!create && row.empty())
        return Invalid("class", "At least one custom attribute class attribute must change.");
    result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    auto schemaId = schema->GetId();
    auto classId = create ? ECClassId() : oldClass->GetId();
    auto baseId = base == nullptr ? ECClassId() : base->GetId();
    auto baseSchemaId = base == nullptr ? ECSchemaId() : base->GetSchema().GetId();
    out.m_createsLeafClass = create;
    out.m_apply = [create, schemaId, classId, baseId, baseSchemaId, row, attributes, properties](SchemaEditContext& apply) mutable {
        if (baseSchemaId.IsValid()) {
            auto reference = apply.AddReference(schemaId, baseSchemaId);
            if (!reference.IsOk())
                return reference;
        }
        ECClassId id = classId;
        if (create) {
            id = ECClassId(apply.m_db.GetImpl().GetIdFactory().Class().NextId().GetValue());
            row.push_back({"Id", int64_t(id.GetValue())});
        }
        auto written = create ? apply.m_writer.Insert("ec_Class", row) : apply.m_writer.Update("ec_Class", id, row);
        if (!written.IsOk())
            return written;
        if (create)
            apply.m_createdClasses.insert(id);
        if (create && !attributes.empty()) {
            written = InsertCustomAttributes(apply, schemaId, id, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class, attributes);
            if (!written.IsOk())
                return written;
        }
        if (baseId.IsValid()) {
            auto relationId = apply.m_db.GetImpl().GetIdFactory().ClassHasBaseClasses().NextId();
            written = apply.m_writer.Insert("ec_ClassHasBaseClasses",
                                            {{"Id", int64_t(relationId.GetValue())}, {"ClassId", int64_t(id.GetValue())}, {"BaseClassId", int64_t(baseId.GetValue())}, {"Ordinal", int64_t(0)}});
            if (!written.IsOk())
                return written;
        }
        written = ApplyInlineProperties(apply, id, properties);
        if (!written.IsOk())
            return written;
        Affected(apply, schemaId, id);
        return Result();
    };
    return {};
}

Result DropCustomAttributeClass(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    Utf8String name;
    if (!parser.CheckKeys({"class"}) || !parser.String("class", name, true))
        return result;
    auto ecClass = FindClass(ctx, name);
    if (ecClass == nullptr)
        return Missing("class", "Class not found.");
    if (!ecClass->IsCustomAttributeClass())
        return Rejected("class", "The command does not match the persisted class kind.");
    result = Editable(ctx, ecClass->GetSchema());
    if (!result.IsOk())
        return result;
    result = ctx.RequireMajorUpgrade(ecClass->GetSchema());
    if (!result.IsOk())
        return result;
    result = NoReferences(ctx, "SELECT ClassId FROM main.ec_ClassHasBaseClasses WHERE BaseClassId=? LIMIT 1",
                          ecClass->GetId(), "class", "The custom attribute class has derived classes. Remove them first.");
    if (!result.IsOk())
        return result;
    result = NoReferences(ctx, "SELECT Id FROM main.ec_CustomAttribute WHERE ClassId=? LIMIT 1",
                          ecClass->GetId(), "class", "The custom attribute class is applied to one or more schema items; remove those custom attributes first.");
    if (!result.IsOk())
        return result;
    auto id = ecClass->GetId();
    auto schemaId = ecClass->GetSchema().GetId();
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
}

void RegisterSchemaClassCommands(SchemaCommandRegistry& registry) {
    Register(registry, "CreateMixin", CreateMixin);
    Register(registry, "AddMixin", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return ChangeMixin(ctx, args, out, false); });
    Register(registry, "RemoveMixin", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return ChangeMixin(ctx, args, out, true); });
    Register(registry, "CreateCustomAttributeClass", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return PrepareCustomAttributeClass(ctx, args, out, true); });
    Register(registry, "AlterCustomAttributeClass", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return PrepareCustomAttributeClass(ctx, args, out, false); });
    Register(registry, "DropCustomAttributeClass", DropCustomAttributeClass);
    for (bool entity : {true, false}) {
        Register(registry, entity ? "CreateEntityClass" : "CreateStructClass",
                 [entity](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return PrepareClass(ctx, args, out, true, entity); });
        Register(registry, entity ? "AlterEntityClass" : "AlterStructClass",
                 [entity](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return PrepareClass(ctx, args, out, false, entity); });
        Register(registry, entity ? "DropEntityClass" : "DropStructClass",
                 [entity](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return DropClass(ctx, args, out, entity); });
    }
}
END_BENTLEY_SQLITE_EC_NAMESPACE
