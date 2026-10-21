/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "SchemaCommand.h"
#include "SchemaCommandArgParser.h"
#include "../ECDbImpl.h"
#include "../ECDbMapSchemaHelper.h"
#include "../FeatureManager.h"
#include "../RemapManager.h"
#include "../SchemaPersistenceHelper.h"
#include <limits>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace SchemaCommands {
using namespace ECN;
using Result = SchemaCommandResult;
using Status = SchemaCommandResult::Status;
using PrepareFunction = std::function<Result(SchemaEditContext&, BeJsConst, PreparedSchemaCommand&)>;

inline Result Invalid(Utf8CP arg, Utf8StringCR message) {
    return Result::Failure(Status::ERROR_INVALID_ARGS, "SchemaEditor.InvalidArgs", arg, message);
}
inline Result Rejected(Utf8CP arg, Utf8StringCR message) {
    return Result::Failure(Status::ERROR_VALIDATION, "SchemaEditor.Validation", arg, message);
}
inline Result Missing(Utf8CP arg, Utf8StringCR message) {
    return Result::Failure(Status::ERROR_NOT_FOUND, "SchemaEditor.NotFound", arg, message);
}
inline Result DatabaseError(Utf8CP operation) {
    return Result::Failure(Status::ERROR, "SchemaEditor.Persistence", "", operation);
}
inline Result Unsupported(Utf8CP arg, Utf8StringCR message) {
    return Result::Failure(Status::ERROR_VALIDATION, "SchemaEditor.UnsupportedOperation", arg, message);
}

struct Command final : ISchemaCommand {
    PrepareFunction m_prepare;
    explicit Command(PrepareFunction prepare) : m_prepare(std::move(prepare)) {}
    Result Prepare(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) const override { return m_prepare(ctx, args, out); }
};

inline void Register(SchemaCommandRegistry& registry, Utf8CP name, PrepareFunction prepare) {
    registry[name] = [prepare]() { return std::make_unique<Command>(prepare); };
}

inline bool QualifiedName(Utf8StringCR value, Utf8StringR schema, Utf8StringR item) {
    auto separator = value.find_first_of(":.");
    if (separator == Utf8String::npos || separator == 0 || separator + 1 == value.size() ||
        value.find_first_of(":.", separator + 1) != Utf8String::npos)
        return false;
    schema = value.substr(0, separator);
    item = value.substr(separator + 1);
    return ECNameValidation::IsValidName(schema.c_str()) && ECNameValidation::IsValidName(item.c_str());
}

inline bool IsECDbProfileSchema(Utf8StringCR name) {
    for (auto profile : {"ECDbMap", "ECDbMeta", "ECDbSystem", "ECDbFileInfo"})
        if (name.EqualsI(profile))
            return true;
    return false;
}

inline bool IsReferenceGraphStandardSchema(Utf8StringCR name) {
    for (auto standard : {"ECDbSchemaPolicies", "CoreCustomAttributes", "SchemaCustomization", "Units", "Formats"})
        if (name.EqualsI(standard))
            return true;
    return false;
}

inline Result Editable(SchemaEditContext const& ctx, ECSchemaCR schema) {
    if (IsECDbProfileSchema(schema.GetName()))
        return Rejected("schema", "ECDb profile schemas cannot be edited.");
    // Standard schemas only change when they arrive in an imported reference graph.
    if (IsReferenceGraphStandardSchema(schema.GetName()) && !ctx.m_session.m_allowStandardSchemas)
        return Rejected("schema", "Standard system schemas cannot be edited.");
    return {};
}

inline ECSchemaCP FindSchema(SchemaEditContext& ctx, Utf8StringCR name) {
    return ctx.m_db.Schemas().GetSchema(name, true, SchemaLookupMode::AutoDetect);
}

inline ECClassCP FindClass(SchemaEditContext& ctx, Utf8StringCR name) {
    Utf8String schema, item;
    return QualifiedName(name, schema, item) ? ctx.m_db.Schemas().GetClass(schema, item, SchemaLookupMode::AutoDetect) : nullptr;
}

inline ECEnumerationCP FindEnumeration(SchemaEditContext& ctx, Utf8StringCR name) {
    Utf8String schema, item;
    return QualifiedName(name, schema, item) ? ctx.m_db.Schemas().GetEnumeration(schema, item, SchemaLookupMode::AutoDetect) : nullptr;
}

inline Result CopyPresentationFormats(ECSchemaR working, ECSchemaCR source) {
    // CopySchema rebinds format parents but leaves unit overrides pointing into the source graph.
    // Resolve the existing format strings against the owned graph before clearing the reader cache.
    auto resolveSchema = [&](Utf8StringCR alias) -> ECSchemaCP {
        if (alias.empty() || alias.EqualsI(working.GetAlias()) || alias.EqualsI(working.GetName()))
            return &working;
        return working.GetSchemaByAliasP(alias);
    };
    for (auto sourceKoq : source.GetKindOfQuantities()) {
        auto koq = working.GetKindOfQuantityP(sourceKoq->GetName().c_str());
        if (koq == nullptr)
            return Rejected("schema", "A kind of quantity is missing from the isolated working graph.");
        koq->RemoveAllPresentationFormats();
        for (auto const& format : sourceKoq->GetPresentationFormats()) {
            auto status = koq->AddPresentationFormatByString(format.GetQualifiedFormatString(source),
                [&](Utf8StringCR alias, Utf8StringCR name) -> ECFormatCP {
                    auto schema = resolveSchema(alias);
                    return schema == nullptr ? nullptr : schema->GetFormatCP(name.c_str());
                },
                [&](Utf8StringCR alias, Utf8StringCR name) -> ECUnitCP {
                    auto schema = resolveSchema(alias);
                    return schema == nullptr ? nullptr : schema->GetUnitCP(name.c_str());
                });
            if (status != ECObjectsStatus::Success)
                return Rejected("schema", Utf8PrintfString("Cannot resolve presentation format '%s' for '%s' in the isolated working graph.",
                    format.GetQualifiedFormatString(source).c_str(), sourceKoq->GetFullName().c_str()).c_str());
        }
    }
    return {};
}

inline Result CopyGraph(SchemaEditContext& ctx, ECSchemaPtr& working, ECSchemaCR persisted, ECSchemaReadContextR context, std::set<Utf8String>& visiting) {
    working = context.GetFoundSchema(persisted.GetSchemaKey(), SchemaMatchType::Exact);
    if (working.IsValid())
        return {};
    if (!visiting.insert(persisted.GetName()).second)
        return Rejected("schema", "A schema reference cycle prevents creation of an isolated working copy.");
    ctx.m_db.Schemas().GetSchema(persisted.GetName(), true, SchemaLookupMode::ByName);
    for (auto const& entry : persisted.GetReferencedSchemas()) {
        ECSchemaPtr referenced;
        auto result = CopyGraph(ctx, referenced, *entry.second, context, visiting);
        if (!result.IsOk())
            return result;
    }
    if (ECObjectsStatus::Success != persisted.CopySchema(working, &context) ||
        ECObjectsStatus::Success != working->SetOriginalECXmlVersion(persisted.GetOriginalECXmlVersionMajor(), persisted.GetOriginalECXmlVersionMinor()) ||
        ECObjectsStatus::Success != context.AddSchema(*working))
        return Rejected("schema", "Cannot create an isolated, validated working schema graph.");
    auto formats = CopyPresentationFormats(*working, persisted);
    if (!formats.IsOk())
        return formats;
    visiting.erase(persisted.GetName());
    return {};
}

inline Result Copy(SchemaEditContext& ctx, ECSchemaPtr& working, ECSchemaCR persisted) {
    auto editable = Editable(ctx, persisted);
    if (!editable.IsOk())
        return editable;
    auto context = ECSchemaReadContext::CreateContext();
    std::set<Utf8String> visiting;
    return CopyGraph(ctx, working, persisted, *context, visiting);
}

inline Result ValidateWorkingGates(SchemaEditContext& ctx, ECSchemaCR schema) {
    if (FeatureManager::SchemaRequiresProfileUpgrade(ctx.m_db, schema))
        return Rejected("schema", "The proposed schema requires a newer ECDb profile.");
    ImportRequiresVersionCustomAttribute gate;
    if (ECDbMapCustomAttributeHelper::TryGetImportRequiresVersion(gate, schema) &&
        (!gate.IsValid() || SUCCESS != gate.Verify(ctx.m_db.GetImpl().Issues(), schema.GetFullSchemaName().c_str())))
        return Rejected("importRequiresVersion", "The schema's ImportRequiresVersion requirement is not satisfied.");
    return {};
}

inline Result ValidateWorking(SchemaEditContext& ctx, ECSchemaR schema) {
    auto result = ValidateWorkingGates(ctx, schema);
    if (!result.IsOk())
        return result;
    return ctx.ValidateWorkingSchema(schema);
}

inline ECSchemaCP WorkingSchema(ECSchemaCR working, Utf8StringCR name) {
    if (working.GetName().EqualsI(name))
        return &working;
    for (auto const& entry : working.GetReferencedSchemas()) {
        if (entry.second->GetName().EqualsI(name))
            return entry.second.get();
        if (auto nested = WorkingSchema(*entry.second, name))
            return nested;
    }
    return nullptr;
}

inline ECClassCP WorkingClass(ECSchemaCR working, ECClassCR persisted) {
    auto schema = WorkingSchema(working, persisted.GetSchema().GetName());
    return schema == nullptr ? nullptr : schema->GetClassCP(persisted.GetName().c_str());
}

inline ECSchemaP WorkingSchema(ECSchemaR working, Utf8StringCR name) {
    if (working.GetName().EqualsI(name))
        return &working;
    for (auto const& entry : working.GetReferencedSchemas()) {
        if (entry.second->GetName().EqualsI(name))
            return entry.second.get();
        if (auto nested = WorkingSchema(*entry.second, name))
            return nested;
    }
    return nullptr;
}

inline ECClassP WorkingClass(ECSchemaR working, ECClassCR persisted) {
    auto schema = WorkingSchema(working, persisted.GetSchema().GetName());
    return schema == nullptr ? nullptr : schema->GetClassP(persisted.GetName().c_str());
}

inline Result CacheWorkingGraph(ECSchemaR schema, ECSchemaReadContextR context) {
    if (context.GetFoundSchema(schema.GetSchemaKey(), SchemaMatchType::Exact).IsValid())
        return {};
    for (auto const& entry : schema.GetReferencedSchemas()) {
        auto result = CacheWorkingGraph(*entry.second, context);
        if (!result.IsOk())
            return result;
    }
    return ECObjectsStatus::Success == context.AddSchema(schema) ? Result() : Rejected("schema", "Cannot retain an isolated working schema reference.");
}

inline Result Reference(SchemaEditContext& ctx, ECSchemaR working, ECSchemaCR referenced) {
    if (working.GetName().EqualsI(referenced.GetName()))
        return {};
    for (auto const& entry : working.GetReferencedSchemas())
        if (entry.second->GetName().EqualsI(referenced.GetName()))
            return {};
    auto context = ECSchemaReadContext::CreateContext();
    for (auto const& entry : working.GetReferencedSchemas()) {
        auto result = CacheWorkingGraph(*entry.second, *context);
        if (!result.IsOk())
            return result;
    }
    std::set<Utf8String> visiting{working.GetName()};
    ECSchemaPtr isolated;
    auto result = CopyGraph(ctx, isolated, referenced, *context, visiting);
    if (!result.IsOk())
        return result;
    if (ECObjectsStatus::Success != working.AddReferencedSchema(*isolated))
        return Rejected("schema", "Cannot add the required schema reference to the working copy.");
    return {};
}

inline Result ParseVersion(Utf8StringCR value, uint32_t (&digits)[3]) {
    size_t offset = 0;
    for (size_t part = 0; part < 3; ++part) {
        uint64_t digit = 0;
        size_t start = offset;
        while (offset < value.size() && value[offset] >= '0' && value[offset] <= '9') {
            digit = digit * 10 + value[offset++] - '0';
            if (digit > std::numeric_limits<uint32_t>::max())
                return Invalid("version", "Schema version components exceed uint32.");
        }
        if (start == offset)
            return Invalid("version", "Expected the complete version RR.WW.mm.");
        digits[part] = static_cast<uint32_t>(digit);
        if (part < 2 && (offset >= value.size() || value[offset++] != '.'))
            return Invalid("version", "Expected the complete version RR.WW.mm.");
    }
    return offset == value.size() ? Result() : Invalid("version", "Trailing characters in schema version.");
}

// SQL is a command-owned constant. All input is bound, including identifiers represented by IDs.
inline Result QueryId(SchemaEditContext& ctx, Utf8CP sql, BeInt64Id id, int64_t& value, bool& found) {
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != ctx.m_db.GetCachedStatement(stmt, sql) || BE_SQLITE_OK != stmt->BindId(1, id))
        return DatabaseError("Preparing metadata dependency query failed.");
    auto step = stmt->Step();
    if (step != BE_SQLITE_ROW && step != BE_SQLITE_DONE)
        return DatabaseError("Executing metadata dependency query failed.");
    found = step == BE_SQLITE_ROW;
    value = found && !stmt->IsColumnNull(0) ? stmt->GetValueInt64(0) : 0;
    return {};
}

inline Result NoReferences(SchemaEditContext& ctx, Utf8CP sql, BeInt64Id id, Utf8CP arg, Utf8CP message) {
    int64_t value;
    bool found;
    auto result = QueryId(ctx, sql, id, value, found);
    return !result.IsOk() ? result : found ? Rejected(arg, message)
                                           : Result();
}

inline Result NextOrdinal(SchemaEditContext& ctx, Utf8CP sql, BeInt64Id owner, int64_t& ordinal) {
    bool found;
    auto result = QueryId(ctx, sql, owner, ordinal, found);
    if (result.IsOk()) {
        if (!found || ordinal < 0)
            return DatabaseError("Reading the next metadata ordinal failed.");
    }
    return result;
}

inline Result DeleteSelected(SchemaEditContext& ctx, Utf8CP table, Utf8CP sql, BeInt64Id owner) {
    bvector<BeInt64Id> ids;
    {
        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != ctx.m_db.GetCachedStatement(stmt, sql) || BE_SQLITE_OK != stmt->BindId(1, owner))
            return DatabaseError("Preparing dependent metadata deletion failed.");
        DbResult step;
        while ((step = stmt->Step()) == BE_SQLITE_ROW)
            ids.push_back(BeInt64Id(stmt->GetValueUInt64(0)));
        if (step != BE_SQLITE_DONE)
            return DatabaseError("Reading dependent metadata for deletion failed.");
    }
    for (auto id : ids) {
        auto result = ctx.m_writer.Delete(table, id);
        if (!result.IsOk())
            return result;
    }
    return {};
}

inline Result AffectClassAndUsers(SchemaEditContext& ctx, ECClassId id) {
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != ctx.m_db.GetCachedStatement(stmt,
                                     "WITH RECURSIVE users(Id) AS (SELECT ? UNION SELECT b.ClassId FROM main.ec_ClassHasBaseClasses b JOIN users u ON b.BaseClassId=u.Id"
                                     " UNION SELECT p.ClassId FROM main.ec_Property p JOIN users u ON p.StructClassId=u.Id)"
                                     " SELECT c.Id,c.SchemaId FROM main.ec_Class c JOIN users u ON c.Id=u.Id") ||
        BE_SQLITE_OK != stmt->BindId(1, id))
        return DatabaseError("Preparing affected class discovery failed.");
    DbResult step;
    while ((step = stmt->Step()) == BE_SQLITE_ROW) {
        ctx.m_affectedClasses.insert(stmt->GetValueId<ECClassId>(0));
        ctx.m_affectedSchemas.insert(stmt->GetValueId<ECSchemaId>(1));
    }
    return step == BE_SQLITE_DONE ? Result() : DatabaseError("Reading affected classes failed.");
}

inline Result DeletePropertyCAs(SchemaEditContext& ctx, ECPropertyId id) {
    return DeleteSelected(ctx, "ec_CustomAttribute",
                          "SELECT Id FROM main.ec_CustomAttribute WHERE ContainerType=992 AND ContainerId=?", id);
}

inline Result DerivedClassIds(SchemaEditContext& ctx, ECClassId root, bvector<ECClassId>& ids) {
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != ctx.m_db.GetCachedStatement(stmt,
                                     "WITH RECURSIVE derived(Id) AS (SELECT ? UNION SELECT b.ClassId FROM main.ec_ClassHasBaseClasses b JOIN derived d ON b.BaseClassId=d.Id)"
                                     " SELECT Id FROM derived") ||
        BE_SQLITE_OK != stmt->BindId(1, root))
        return DatabaseError("Preparing derived class discovery failed.");
    DbResult step;
    while ((step = stmt->Step()) == BE_SQLITE_ROW)
        ids.push_back(stmt->GetValueId<ECClassId>(0));
    return step == BE_SQLITE_DONE ? Result() : DatabaseError("Reading derived classes failed.");
}

using PropertyAccess = bvector<ECPropertyCP>;

inline bool HasPropertyData(IECSqlValue const& value, PropertyAccess const& access, size_t index) {
    if (index + 1 == access.size())
        return !value.IsNull() || (access[index]->GetIsArray() && value.GetArrayLength() != 0);
    auto name = access[index + 1]->GetName().c_str();
    if (access[index]->GetIsStructArray()) {
        for (auto const& element : value.GetArrayIterable())
            if (HasPropertyData(element[name], access, index + 1))
                return true;
        return false;
    }
    return HasPropertyData(value[name], access, index + 1);
}

inline Result CheckPropertyData(SchemaEditContext& ctx, ECClassCR ecClass, PropertyAccess const& access) {
    if (!ecClass.IsEntityClass() && !ecClass.IsRelationshipClass())
        return Unsupported("property", "Custom-attribute XML values require a separate migration; their owner is not an ECSQL instance class.");
    auto map = ctx.m_db.Schemas().Main().GetClassMap(ecClass);
    if (map == nullptr)
        return Unsupported("property", "The property's owner has no instance mapping; custom-attribute XML values cannot be inspected through ECSQL.");
    if (map->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        return {};
    auto query = Utf8PrintfString("SELECT ECClassId,ECInstanceId,[%s] FROM ONLY [%s].[%s]",
                                  access.front()->GetName().c_str(), ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str());
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ctx.m_db, query.c_str()))
        return DatabaseError("Preparing the property-value inspection failed.");
    ECInstanceKeyMultiMap instances;
    DbResult step;
    while ((step = stmt.Step()) == BE_SQLITE_ROW)
        if (HasPropertyData(stmt.GetValue(2), access, 0))
            instances.insert(ECInstanceKeyMultiMapPair(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1)));
    if (step != BE_SQLITE_DONE)
        return DatabaseError("Reading property values failed.");
    if (instances.empty())
        return {};
    auto result = Result::Failure(Result::Status::ERROR_DATA_DELETION_REQUIRED,
                                  "SchemaEditor.DataDeletionRequired", "property", "Non-null property values must be cleared before removing their schema definition or inheritance.");
    result.SetBlockingInstances(std::move(instances));
    return result;
}

// Only remove the selected classes' mappings. Paths can also be used by unaffected
// mixin consumers, so delete a path only after its last property map is gone.
struct PropertyMapRemoval {
    std::set<BeInt64Id> m_maps;
    std::set<BeInt64Id> m_paths;

    Result Select(SchemaEditContext& ctx, ECClassId ecClass, Utf8StringCR access) {
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(ctx.m_db,
                                         "SELECT pm.Id,pp.Id,EXISTS(SELECT 1 FROM main.ec_IndexColumn ic WHERE ic.ColumnId=pm.ColumnId)"
                                         " FROM main.ec_PropertyMap pm JOIN main.ec_PropertyPath pp ON pp.Id=pm.PropertyPathId"
                                         " WHERE pm.ClassId=?1 AND (pp.AccessString=?2 COLLATE NOCASE"
                                         " OR substr(pp.AccessString,1,length(?2)+1)=(?2||'.') COLLATE NOCASE)") ||
            BE_SQLITE_OK != stmt.BindId(1, ecClass) || BE_SQLITE_OK != stmt.BindText(2, access.c_str(), Statement::MakeCopy::Yes))
            return DatabaseError("Preparing property-map removal failed.");
        DbResult step;
        while ((step = stmt.Step()) == BE_SQLITE_ROW) {
            if (stmt.GetValueInt(2) != 0)
                return Rejected("property", "A removed property mapping is referenced by an index; remove the index before removing the property.");
            m_maps.insert(stmt.GetValueId<BeInt64Id>(0));
            m_paths.insert(stmt.GetValueId<BeInt64Id>(1));
        }
        return step == BE_SQLITE_DONE ? Result() : DatabaseError("Reading property mappings failed.");
    }

    Result Apply(SchemaEditContext& ctx) const {
        for (auto id : m_maps) {
            auto result = ctx.m_writer.Delete("ec_PropertyMap", id);
            if (!result.IsOk())
                return result;
        }
        for (auto id : m_paths) {
            bool orphan;
            {
                Statement stmt;
                if (BE_SQLITE_OK != stmt.Prepare(ctx.m_db,
                                                 "SELECT Id FROM main.ec_PropertyPath p WHERE p.Id=? AND NOT EXISTS(SELECT 1 FROM main.ec_PropertyMap pm WHERE pm.PropertyPathId=p.Id)") ||
                    BE_SQLITE_OK != stmt.BindId(1, id))
                    return DatabaseError("Preparing orphan property-path cleanup failed.");
                auto step = stmt.Step();
                if (step != BE_SQLITE_ROW && step != BE_SQLITE_DONE)
                    return DatabaseError("Checking orphan property paths failed.");
                orphan = step == BE_SQLITE_ROW;
            }
            if (orphan) {
                auto result = ctx.m_writer.Delete("ec_PropertyPath", id);
                if (!result.IsOk())
                    return result;
            }
        }
        return {};
    }
};

inline Result DeleteClassCAs(SchemaEditContext& ctx, ECClassId id) {
    return DeleteSelected(ctx, "ec_CustomAttribute",
                          "SELECT Id FROM main.ec_CustomAttribute WHERE (ContainerType=30 AND ContainerId=?1)"
                          " OR (ContainerType=992 AND ContainerId IN (SELECT Id FROM main.ec_Property WHERE ClassId=?1))"
                          " OR (ContainerType IN (1024,2048) AND ContainerId IN (SELECT Id FROM main.ec_RelationshipConstraint WHERE RelationshipClassId=?1))",
                          id);
}

inline Result Modifier(Utf8StringCR text, ECClassModifier& modifier) {
    if (text.EqualsI("None"))
        modifier = ECClassModifier::None;
    else if (text.EqualsI("Abstract"))
        modifier = ECClassModifier::Abstract;
    else if (text.EqualsI("Sealed"))
        modifier = ECClassModifier::Sealed;
    else
        return Invalid("modifier", "Expected None, Abstract, or Sealed.");
    return {};
}

inline bool Primitive(Utf8StringCR text, PrimitiveType& type) {
    static const std::pair<Utf8CP, PrimitiveType> types[] = {
        {"string", PRIMITIVETYPE_String}, {"int", PRIMITIVETYPE_Integer}, {"long", PRIMITIVETYPE_Long}, {"double", PRIMITIVETYPE_Double}, {"bool", PRIMITIVETYPE_Boolean}, {"boolean", PRIMITIVETYPE_Boolean}, {"dateTime", PRIMITIVETYPE_DateTime}, {"point2d", PRIMITIVETYPE_Point2d}, {"point3d", PRIMITIVETYPE_Point3d}, {"binary", PRIMITIVETYPE_Binary}, {"geometry", PRIMITIVETYPE_IGeometry}, {"idlong", PRIMITIVETYPE_Long}};
    for (auto const& entry : types)
        if (text.EqualsI(entry.first)) {
            type = entry.second;
            return true;
        }
    return false;
}

template <typename T>
Result Labels(SchemaCommandArgParser& parser, T& target, SchemaMetaRow& row) {
    Utf8String label, description;
    if (!parser.String("label", label) || !parser.String("description", description))
        return Invalid("label", "Label and description must be strings.");
    if (parser.Has("label")) {
        if (ECObjectsStatus::Success != target.SetDisplayLabel(label))
            return Rejected("label", "Invalid display label.");
        row.push_back({"DisplayLabel", label});
    }
    if (parser.Has("description")) {
        if (ECObjectsStatus::Success != target.SetDescription(description))
            return Rejected("description", "Invalid description.");
        row.push_back({"Description", description});
    }
    return {};
}

inline void Affected(SchemaEditContext& ctx, ECSchemaId schema, ECClassId ecClass = ECClassId()) {
    ctx.m_affectedSchemas.insert(schema);
    if (ecClass.IsValid())
        ctx.m_affectedClasses.insert(ecClass);
}

struct DerivedPropertyTypeUpgrade {
    ECPropertyId m_propertyId;
    ECSchemaId m_ownerSchemaId;
    ECSchemaId m_enumSchemaId;
    SchemaMetaRow m_row;
};

Result CheckAndCollectDerivedPropertyOverrides(SchemaEditContext&, ECClassCR baseClass, ECPropertyCR newBaseProp,
                                               ECEnumerationCP persistedBaseEnum, KindOfQuantityCP persistedBaseKoq,
                                               bvector<DerivedPropertyTypeUpgrade>& outUpgrades,
                                               bool includeSelf = false);
Result ValidateAndPrepareBaseClassPropertyOverrides(SchemaEditContext&, ECSchemaR working, ECClassCR targetPersisted,
                                                    ECClassCR newBasePersisted, ECClassCP oldBasePersisted,
                                                    bvector<DerivedPropertyTypeUpgrade>& outUpgrades);

struct CustomAttribute {
    ECClassId m_class;
    ECSchemaId m_schema;
    Utf8String m_xml;
};
Result PrepareCustomAttribute(SchemaEditContext&, ECSchemaR, IECCustomAttributeContainerR,
                              Utf8StringCR className, BeJsConst instance, CustomAttribute&);
Result PrepareCustomAttributes(SchemaEditContext&, ECSchemaR, IECCustomAttributeContainerR,
                               BeJsConst instances, bvector<CustomAttribute>&);
Result InsertCustomAttributes(SchemaEditContext&, ECSchemaId ownerSchema, BeInt64Id container,
                              SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType,
                              bvector<CustomAttribute> const&);
Result PrepareGeneratedCustomAttribute(SchemaEditContext&, ECSchemaR, IECCustomAttributeContainerR,
                                       Utf8CP name, BeJsConst, bvector<CustomAttribute>&);
Result PrepareClassMappingAttributes(SchemaEditContext&, ECSchemaR, ECClassR, SchemaCommandArgParser&,
                                     bvector<CustomAttribute>&);
Result ParseIndexesValue(BeJsConst indexesValue, BeJsDocument& outJson);
Result ValidateWorkingClassDbIndexList(SchemaEditContext& ctx, ECClassR ecClass);
Result ValidateDbIndexListUpdate(ECClassCP persistedClass, IECInstanceCR newInstance);
Result UpsertClassCustomAttribute(SchemaEditContext&, ECSchemaId ownerSchema, ECClassId classId,
                                  CustomAttribute const&);
Result CheckReservedPropertyName(ECClassCR, Utf8StringCR);

//! A property definition prepared against a working class; applied once the owning class id is known.
struct PreparedProperty final {
    ECPropertyP m_property = nullptr;
    bool m_upgradesDerived = false; //!< True if applying also changes overriding properties of derived classes.
    std::function<Result(SchemaEditContext&, ECClassId)> m_apply;
};
//! Prepares a creation command's "properties" array (AddProperty arguments without "class") on the
//! command's new working class. Errors are reported at properties[i].<argument>.
Result PrepareInlineProperties(SchemaEditContext&, BeJsConst properties, ECSchemaR working, ECClassR ecClass,
                               ECSchemaId schemaId, bvector<PreparedProperty>&);
//! Writes inline properties after their class row exists.
Result ApplyInlineProperties(SchemaEditContext&, ECClassId, bvector<PreparedProperty> const&);
Result CheckReservedPropertyNameInHierarchy(SchemaEditContext&, ECClassCR, Utf8StringCR);
}

void RegisterSchemaClassCommands(SchemaCommandRegistry&);
void RegisterSchemaPropertyCommands(SchemaCommandRegistry&);
void RegisterSchemaEnumerationCommands(SchemaCommandRegistry&);
void RegisterSchemaCustomAttributeCommands(SchemaCommandRegistry&);
void RegisterSchemaRelationshipCommands(SchemaCommandRegistry&);
void RegisterSchemaUnitCommands(SchemaCommandRegistry&);
void RegisterSchemaQuantityCommands(SchemaCommandRegistry&);
END_BENTLEY_SQLITE_EC_NAMESPACE
