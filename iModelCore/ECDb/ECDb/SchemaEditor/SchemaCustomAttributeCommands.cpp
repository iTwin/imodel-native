/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommandHelpers.h"
#include <ECObjects/ECJsonUtilities.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace SchemaCommands {
Result PrepareGeneratedCustomAttribute(SchemaEditContext& ctx, ECSchemaR working, IECCustomAttributeContainerR container,
                                       Utf8CP name, BeJsConst json, bvector<CustomAttribute>& attributes) {
    Utf8String qualified("ECDbMap:");
    qualified.append(name);
    auto definition = FindClass(ctx, qualified);
    if (definition == nullptr)
        return Missing("customAttributes", "The required ECDbMap attribute class is not persisted.");
    for (auto const& attribute : attributes)
        if (attribute.m_class == definition->GetId())
            return Invalid("customAttributes", "The same custom attribute is supplied both explicitly and through a first-class option.");
    CustomAttribute attribute;
    auto result = PrepareCustomAttribute(ctx, working, container, qualified, json, attribute);
    if (result.IsOk())
        attributes.push_back(attribute);
    return result;
}

Result ValidateWorkingClassDbIndexList(SchemaEditContext& ctx, ECClassR ecClass) {
    DbIndexListCustomAttribute dbIndexList;
    if (ECDbMapCustomAttributeHelper::TryGetDbIndexList(dbIndexList, ecClass)) {
        bvector<DbIndexListCustomAttribute::DbIndex> indices;
        if (SUCCESS != dbIndexList.GetIndexes(indices))
            return Rejected("indexes", "The DbIndexList definition is invalid for this class.");
        for (auto const& idx : indices)
            for (auto const& propName : idx.GetProperties()) {
                bvector<Utf8String> tokens;
                BeStringUtilities::Split(propName.c_str(), ".", tokens);
                if (tokens.empty())
                    return Rejected("indexes", "Index property access string cannot be empty.");
                ECClassCP currentClass = &ecClass;
                for (size_t i = 0; i < tokens.size(); ++i) {
                    if (currentClass == nullptr)
                        return Rejected("indexes", "Index property path traverses a non-struct property.");
                    ECPropertyCP prop = currentClass->GetPropertyP(tokens[i].c_str(), true);
                    if (prop == nullptr) {
                        if (i == tokens.size() - 1 && prop != nullptr && prop->GetIsNavigation() && tokens[i].EqualsIAscii("Id"))
                            break;
                        return Rejected("indexes", Utf8PrintfString("Indexed property '%s' does not exist on class '%s'.", propName.c_str(), ecClass.GetFullName()));
                    }
                    if (i + 1 < tokens.size()) {
                        if (prop->GetIsStruct())
                            currentClass = &prop->GetAsStructProperty()->GetType();
                        else if (prop->GetIsNavigation() && i + 2 == tokens.size() && tokens[i + 1].EqualsIAscii("Id"))
                            break;
                        else
                            return Rejected("indexes", Utf8PrintfString("Indexed property '%s' is invalid on class '%s'.", propName.c_str(), ecClass.GetFullName()));
                    }
                }
            }
    }
    return {};
}

Result PrepareClassMappingAttributes(SchemaEditContext& ctx, ECSchemaR working, ECClassR ecClass,
                                     SchemaCommandArgParser& parser, bvector<CustomAttribute>& attributes) {
    Result result;
    if (parser.Has("customAttributes")) {
        result = PrepareCustomAttributes(ctx, working, ecClass, parser.Value("customAttributes"), attributes);
        if (!result.IsOk())
            return result;
    }
    if (parser.Has("mapStrategy") || parser.Has("tableName") || parser.Has("ecInstanceIdColumn")) {
        Utf8String strategy, table, idColumn;
        if (!parser.String("mapStrategy", strategy, true) || !parser.String("tableName", table) || !parser.String("ecInstanceIdColumn", idColumn))
            return Invalid("mapStrategy", "Class mapping requires a mapStrategy string.");
        Utf8CP canonical = nullptr;
        for (auto allowed : {"OwnTable", "TablePerHierarchy", "ExistingTable", "NotMapped"})
            if (strategy.EqualsI(allowed))
                canonical = allowed;
        if (canonical == nullptr)
            return Invalid("mapStrategy", "Unknown class map strategy.");
        if (parser.Has("tableName") && !strategy.EqualsI("ExistingTable"))
            return Invalid("tableName", "tableName is only supported with ExistingTable.");
        if (strategy.EqualsI("ExistingTable") && table.empty())
            return Invalid("tableName", "ExistingTable requires a table name.");
        BeJsDocument json;
        json["MapStrategy"] = canonical;
        if (parser.Has("tableName"))
            json["TableName"] = table;
        if (parser.Has("ecInstanceIdColumn"))
            json["ECInstanceIdColumn"] = idColumn;
        result = PrepareGeneratedCustomAttribute(ctx, working, ecClass, "ClassMap", json, attributes);
        if (!result.IsOk())
            return result;
    }
    if (parser.Has("shareColumns") || parser.Has("applyToSubclassesOnly") || parser.Has("maxSharedColumnsBeforeOverflow")) {
        if (ecClass.GetCustomAttributeLocal("ECDbMap", "ShareColumns").IsValid())
            return Invalid("shareColumns", "ShareColumns is supplied both explicitly and through first-class options.");
        bool share = false, subclasses = false;
        uint32_t maximum = 0;
        if (!parser.Boolean("shareColumns", share, true) || !parser.Boolean("applyToSubclassesOnly", subclasses) ||
            !parser.UInt32("maxSharedColumnsBeforeOverflow", maximum))
            return Invalid("shareColumns", "ShareColumns options have invalid types.");
        if (!share && (parser.Has("applyToSubclassesOnly") || parser.Has("maxSharedColumnsBeforeOverflow")))
            return Invalid("shareColumns", "ShareColumns options require shareColumns=true.");
        if (share) {
            if (maximum > uint32_t(std::numeric_limits<int32_t>::max()))
                return Invalid("maxSharedColumnsBeforeOverflow", "The shared-column limit exceeds int32.");
            BeJsDocument json;
            json["ApplyToSubclassesOnly"] = subclasses;
            if (parser.Has("maxSharedColumnsBeforeOverflow"))
                json["MaxSharedColumnsBeforeOverflow"] = maximum;
            result = PrepareGeneratedCustomAttribute(ctx, working, ecClass, "ShareColumns", json, attributes);
            if (!result.IsOk())
                return result;
        }
    }
    if (parser.Has("joinedTablePerDirectSubclass")) {
        if (ecClass.GetCustomAttributeLocal("ECDbMap", "JoinedTablePerDirectSubclass").IsValid())
            return Invalid("joinedTablePerDirectSubclass", "JoinedTablePerDirectSubclass is supplied twice.");
        bool joined = false;
        if (!parser.Boolean("joinedTablePerDirectSubclass", joined))
            return Invalid("joinedTablePerDirectSubclass", "Expected a boolean.");
        if (joined) {
            BeJsDocument json;
            json.SetEmptyObject();
            result = PrepareGeneratedCustomAttribute(ctx, working, ecClass, "JoinedTablePerDirectSubclass", json, attributes);
            if (!result.IsOk())
                return result;
        }
    }
    if (parser.Has("queryView")) {
        Utf8String query;
        if (!parser.String("queryView", query, true) || query.empty())
            return Invalid("queryView", "A query view requires a nonempty ECSQL query.");
        BeJsDocument json;
        json["Query"] = query;
        result = PrepareGeneratedCustomAttribute(ctx, working, ecClass, "QueryView", json, attributes);
        if (!result.IsOk())
            return result;
    }
    if (parser.Has("useRequiresVersion")) {
        Utf8String required;
        if (!parser.String("useRequiresVersion", required, true))
            return Invalid("useRequiresVersion", "Expected the required ECDb runtime version string.");
        BeJsDocument json;
        json["ECDbRuntimeVersion"] = required;
        result = PrepareGeneratedCustomAttribute(ctx, working, ecClass, "UseRequiresVersion", json, attributes);
        if (!result.IsOk())
            return result;
    }
    if (parser.Has("indexes")) {
        BeJsDocument json;
        result = ParseIndexesValue(parser.Value("indexes"), json);
        if (!result.IsOk())
            return result;
        result = PrepareGeneratedCustomAttribute(ctx, working, ecClass, "DbIndexList", json, attributes);
        if (!result.IsOk())
            return result;
        result = ValidateWorkingClassDbIndexList(ctx, ecClass);
        if (!result.IsOk())
            return result;
    }
    return {};
}

Result ParseIndexesValue(BeJsConst indexes, BeJsDocument& json) {
    auto output = json["Indexes"];
    output.SetEmptyArray();
    std::set<Utf8String, CompareIUtf8Ascii> names;
    auto addEntry = [&](Utf8StringCR name, bvector<Utf8String> const& properties, bool unique, Utf8StringCR where) -> Result {
        if (!ECNameValidation::IsValidName(name.c_str()) || !names.insert(name).second || properties.empty())
            return Invalid("indexes", "Index names must be unique EC identifiers and each index requires properties.");
        if (!where.empty() && !where.EqualsI("IndexedColumnsAreNotNull"))
            return Invalid("indexes.where", "Only IndexedColumnsAreNotNull is supported.");
        auto index = output.appendObject();
        index["Name"] = name;
        index["IsUnique"] = unique;
        auto propertyNames = index["Properties"];
        propertyNames.SetEmptyArray();
        std::set<Utf8String, CompareIUtf8Ascii> seen;
        for (auto const& property : properties) {
            if (property.empty() || !seen.insert(property).second)
                return Invalid("indexes.properties", "Index property names cannot be empty or duplicated.");
            propertyNames.appendValue() = property;
        }
        if (!where.empty())
            index["Where"] = "IndexedColumnsAreNotNull";
        return {};
    };

    if (indexes.isArray()) {
        for (uint32_t i = 0; i < indexes.size(); ++i) {
            Result indexResult;
            SchemaCommandArgParser indexParser(indexes[i], indexResult);
            Utf8String name, where;
            bvector<Utf8String> properties;
            bool unique = false;
            if (!indexParser.CheckKeys({"name", "properties", "isUnique", "where"}) ||
                !indexParser.String("name", name, true) || !indexParser.StringList("properties", properties, true) ||
                !indexParser.Boolean("isUnique", unique) || !indexParser.String("where", where))
                return indexResult;
            auto r = addEntry(name, properties, unique, where);
            if (!r.IsOk())
                return r;
        }
        return {};
    }
    if (indexes.isString()) {
        Utf8String text(indexes.ToUtf8CP());
        text.Trim();
        if (text.empty())
            return Invalid("indexes", "Indexes string cannot be empty.");
        size_t offset = 0;
        while (offset < text.size()) {
            auto end = text.find(';', offset);
            if (end == Utf8String::npos)
                end = text.size();
            Utf8String entry = text.substr(offset, end - offset);
            entry.Trim();
            if (entry.empty())
                return Invalid("indexes", "Indexes string contains an empty entry.");
            bvector<Utf8String> parts;
            BeStringUtilities::Split(entry.c_str(), ":", parts);
            if (parts.size() < 2 || parts.size() > 4)
                return Invalid("indexes", "Expected index format name:prop1+prop2[:unique][:where=IndexedColumnsAreNotNull].");
            Utf8String name = parts[0];
            name.Trim();
            bvector<Utf8String> properties;
            BeStringUtilities::Split(parts[1].c_str(), "+", properties);
            for (auto& p : properties)
                p.Trim();
            bool unique = false;
            Utf8String where;
            for (size_t k = 2; k < parts.size(); ++k) {
                Utf8String opt = parts[k];
                opt.Trim();
                if (opt.EqualsI("unique"))
                    unique = true;
                else if (opt.StartsWithIAscii("where="))
                    where = opt.substr(6);
                else
                    return Invalid("indexes", "Unknown index option; expected 'unique' or 'where=IndexedColumnsAreNotNull'.");
            }
            auto r = addEntry(name, properties, unique, where);
            if (!r.IsOk())
                return r;
            if (end == text.size())
                break;
            offset = end + 1;
            if (offset == text.size())
                return Invalid("indexes", "Trailing semicolon in indexes string.");
        }
        return {};
    }
    return Invalid("indexes", "Expected a JSON index array or 'name:prop1+prop2[:unique][:where=IndexedColumnsAreNotNull];...' string.");
}

Result ValidateDbIndexListUpdate(ECClassCP persistedClass, IECInstanceCR newInstance) {
    if (persistedClass == nullptr)
        return {};
    auto oldCA = persistedClass->GetCustomAttributeLocal("ECDbMap", "DbIndexList");
    if (!oldCA.IsValid())
        return {};
    struct IndexSpec {
        bool m_isUnique = false;
        Utf8String m_where;
    };
    std::map<Utf8String, IndexSpec, CompareIUtf8Ascii> oldIndexes;
    ECValue oldArr;
    if (ECObjectsStatus::Success == oldCA->GetValue(oldArr, "Indexes") && !oldArr.IsNull() && oldArr.IsArray()) {
        for (uint32_t i = 0; i < oldArr.GetArrayInfo().GetCount(); ++i) {
            ECValue elem;
            if (ECObjectsStatus::Success != oldCA->GetValue(elem, "Indexes", i) || elem.IsNull() || !elem.IsStruct())
                continue;
            auto structInst = elem.GetStruct();
            if (!structInst.IsValid())
                continue;
            ECValue nameVal, uniqueVal, whereVal;
            structInst->GetValue(nameVal, "Name");
            structInst->GetValue(uniqueVal, "IsUnique");
            structInst->GetValue(whereVal, "Where");
            if (nameVal.IsNull() || !nameVal.IsString())
                continue;
            IndexSpec spec;
            spec.m_isUnique = !uniqueVal.IsNull() && uniqueVal.IsBoolean() && uniqueVal.GetBoolean();
            spec.m_where = (!whereVal.IsNull() && whereVal.IsString()) ? whereVal.GetUtf8CP() : "";
            oldIndexes[nameVal.GetUtf8CP()] = spec;
        }
    }
    std::map<Utf8String, IndexSpec, CompareIUtf8Ascii> newIndexes;
    ECValue newArr;
    if (ECObjectsStatus::Success == newInstance.GetValue(newArr, "Indexes") && !newArr.IsNull() && newArr.IsArray()) {
        for (uint32_t i = 0; i < newArr.GetArrayInfo().GetCount(); ++i) {
            ECValue elem;
            if (ECObjectsStatus::Success != newInstance.GetValue(elem, "Indexes", i) || elem.IsNull() || !elem.IsStruct())
                continue;
            auto structInst = elem.GetStruct();
            if (!structInst.IsValid())
                continue;
            ECValue nameVal, uniqueVal, whereVal;
            structInst->GetValue(nameVal, "Name");
            structInst->GetValue(uniqueVal, "IsUnique");
            structInst->GetValue(whereVal, "Where");
            if (nameVal.IsNull() || !nameVal.IsString())
                continue;
            IndexSpec spec;
            spec.m_isUnique = !uniqueVal.IsNull() && uniqueVal.IsBoolean() && uniqueVal.GetBoolean();
            spec.m_where = (!whereVal.IsNull() && whereVal.IsString()) ? whereVal.GetUtf8CP() : "";
            newIndexes[nameVal.GetUtf8CP()] = spec;
        }
    }
    for (auto const& [oldName, oldSpec] : oldIndexes) {
        auto it = newIndexes.find(oldName);
        if (it == newIndexes.end())
            return Rejected("indexes", "Removing or renaming an existing index in DbIndexList is not supported.");
        if (it->second.m_isUnique != oldSpec.m_isUnique || !it->second.m_where.EqualsI(oldSpec.m_where))
            return Rejected("indexes", "Changing IsUnique or Where on an existing index in DbIndexList is not supported; only adding indexes or updating Properties is allowed.");
    }
    return {};
}

Result UpsertClassCustomAttribute(SchemaEditContext& ctx, ECSchemaId ownerSchema, ECClassId classId,
                                  CustomAttribute const& attribute) {
    auto result = ctx.AddReference(ownerSchema, attribute.m_schema);
    if (!result.IsOk())
        return result;
    BeInt64Id existingId;
    int64_t ordinal = 0;
    {
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(ctx.m_db,
                                         "SELECT Id,Ordinal FROM main.ec_CustomAttribute WHERE ContainerId=? AND ContainerType=30 AND ClassId=?") ||
            BE_SQLITE_OK != stmt.BindId(1, classId) || BE_SQLITE_OK != stmt.BindId(2, attribute.m_class))
            return DatabaseError("Preparing class custom attribute lookup failed.");
        auto step = stmt.Step();
        if (step == BE_SQLITE_ROW) {
            existingId = BeInt64Id(stmt.GetValueUInt64(0));
            ordinal = stmt.GetValueInt64(1);
        } else if (step != BE_SQLITE_DONE)
            return DatabaseError("Reading class custom attribute failed.");
    }
    if (existingId.IsValid())
        return ctx.m_writer.Update("ec_CustomAttribute", existingId, {{"Instance", attribute.m_xml}});
    Statement ordStmt;
    if (BE_SQLITE_OK != ordStmt.Prepare(ctx.m_db,
                                        "SELECT COALESCE(MAX(Ordinal)+1,0) FROM main.ec_CustomAttribute WHERE ContainerId=? AND ContainerType=30") ||
        BE_SQLITE_OK != ordStmt.BindId(1, classId) || ordStmt.Step() != BE_SQLITE_ROW)
        return DatabaseError("Reading class custom attribute ordinal failed.");
    ordinal = ordStmt.GetValueInt64(0);
    ordStmt.Finalize();
    auto id = ctx.m_db.GetImpl().GetIdFactory().CustomAttribute().NextId();
    return ctx.m_writer.Insert("ec_CustomAttribute",
                               {{"Id", int64_t(id.GetValue())}, {"ContainerId", int64_t(classId.GetValue())}, {"ContainerType", int64_t(SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class)}, {"ClassId", int64_t(attribute.m_class.GetValue())}, {"Ordinal", ordinal}, {"Instance", attribute.m_xml}});
}

Result PrepareCustomAttribute(SchemaEditContext& ctx, ECSchemaR working, IECCustomAttributeContainerR container,
                              Utf8StringCR className, BeJsConst json, CustomAttribute& out) {
    if (!json.isObject())
        return Invalid("instance", "A custom attribute instance must be an ECJson object.");
    auto caClass = FindClass(ctx, className);
    if (caClass == nullptr || !caClass->IsCustomAttributeClass())
        return Missing("caClass", "Custom attribute class not found.");
    auto result = Reference(ctx, working, caClass->GetSchema());
    if (!result.IsOk())
        return result;
    SchemaCommandArgParser identity(json, result);
    if (identity.Has("className")) {
        Utf8String supplied;
        if (!identity.String("className", supplied))
            return result;
        auto suppliedClass = FindClass(ctx, supplied);
        if (suppliedClass == nullptr || suppliedClass->GetId() != caClass->GetId())
            return Invalid("instance.className", "The ECJson className must identify caClass.");
    }
    auto workingClass = WorkingClass(working, *caClass);
    if (workingClass == nullptr)
        return Rejected("caClass", "Custom attribute class missing from isolated working graph.");
    IECInstancePtr instance = workingClass->GetDefaultStandaloneEnabler()->CreateInstance();
    if (!instance.IsValid())
        return Rejected("instance", "Cannot create a custom attribute instance.");
    if (SUCCESS != JsonECInstanceConverter::JsonToECInstance(*instance, json, const_cast<SchemaManager&>(ctx.m_db.Schemas()), false))
        return Rejected("instance", "Invalid ECJson custom attribute property or value.");
    if (caClass->GetSchema().GetName().EqualsI("ECDbMap") &&
        (caClass->GetName().EqualsI("ImportRequiresVersion") || caClass->GetName().EqualsI("UseRequiresVersion")))
        for (auto property : {"ECDbRuntimeVersion", "ECSqlVersion"}) {
            if (workingClass->GetPropertyP(property) == nullptr)
                continue;
            ECValue value;
            if (ECObjectsStatus::Success != instance->GetValue(value, property))
                return Rejected("instance", "Cannot read the version-gate custom attribute.");
            if (value.IsNull()) {
                if (caClass->GetName().EqualsI("ImportRequiresVersion"))
                    return Invalid("instance.ECDbRuntimeVersion", "ImportRequiresVersion requires ECDbRuntimeVersion.");
                continue;
            }
            if (!value.IsString())
                return Invalid("instance", "Version-gate values must be strings.");
            Utf8String version(value.GetUtf8CP());
            size_t offset = 0;
            for (size_t part = 0; part < 4; ++part) {
                auto begin = offset;
                uint64_t component = 0;
                while (offset < version.size() && version[offset] >= '0' && version[offset] <= '9') {
                    component = component * 10 + version[offset++] - '0';
                    if (component > std::numeric_limits<uint32_t>::max())
                        return Invalid("instance", "Version-gate components exceed uint32.");
                }
                if (begin == offset || (part < 3 && (offset >= version.size() || version[offset++] != '.')))
                    return Invalid("instance", "Version-gate values must contain four unsigned integer components.");
            }
            if (offset != version.size())
                return Invalid("instance", "Trailing characters in a version-gate value.");
        }
    if (ECObjectsStatus::Success != container.SetCustomAttribute(*instance))
        return Rejected("container", "The custom attribute does not support this container.");
    Utf8String xml;
    if (InstanceWriteStatus::Success != instance->WriteToXmlString(xml, false, true))
        return Rejected("instance", "Cannot serialize the custom attribute.");
    out = {caClass->GetId(), caClass->GetSchema().GetId(), xml};
    return {};
}

Result PrepareCustomAttributes(SchemaEditContext& ctx, ECSchemaR working, IECCustomAttributeContainerR container,
                               BeJsConst instances, bvector<CustomAttribute>& out) {
    if (!instances.isArray())
        return Invalid("customAttributes", "Expected an ECJson custom attribute array.");
    std::set<ECClassId> seen;
    for (uint32_t i = 0; i < instances.size(); ++i) {
        Result result;
        SchemaCommandArgParser parser(instances[i], result);
        Utf8String className;
        if (!parser.String("className", className, true))
            return result;
        CustomAttribute ca;
        result = PrepareCustomAttribute(ctx, working, container, className, instances[i], ca);
        if (!result.IsOk())
            return result;
        if (!seen.insert(ca.m_class).second)
            return Invalid("customAttributes", "A custom attribute class cannot appear twice.");
        out.push_back(ca);
    }
    return {};
}

Result InsertCustomAttributes(SchemaEditContext& ctx, ECSchemaId ownerSchema, BeInt64Id container,
                              SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType type,
                              bvector<CustomAttribute> const& attributes) {
    int64_t ordinal = 0;
    for (auto const& attribute : attributes) {
        auto result = ctx.AddReference(ownerSchema, attribute.m_schema);
        if (!result.IsOk())
            return result;
        auto id = ctx.m_db.GetImpl().GetIdFactory().CustomAttribute().NextId();
        result = ctx.m_writer.Insert("ec_CustomAttribute",
                                     {{"Id", int64_t(id.GetValue())}, {"ContainerId", int64_t(container.GetValue())}, {"ContainerType", int64_t(type)}, {"ClassId", int64_t(attribute.m_class.GetValue())}, {"Ordinal", ordinal++}, {"Instance", attribute.m_xml}});
        if (!result.IsOk())
            return result;
    }
    return {};
}
}
namespace {
using namespace SchemaCommands;

Result ValidateCustomAttributeRule(SchemaEditContext& ctx, ECClassCR caClass, bool remove,
                                   ECSchemaCP persistedSchema, ECClassCP persistedClass, ECPropertyCP persistedProperty,
                                   IECInstanceCP newInstance) {
    auto const& caSchemaName = caClass.GetSchema().GetName();
    auto const& caName = caClass.GetName();
    // A class created earlier in the same dependency group has not been mapped yet, so its
    // creation-time attributes (including property-referencing ones) may still be completed.
    if (!remove && persistedClass != nullptr && ctx.IsCreatedInCurrentUnit(persistedClass->GetId()))
        return {};
    if (caSchemaName.EqualsI("ECDbMap")) {
        if (caName.EqualsI("ImportRequiresVersion") || caName.EqualsI("UseRequiresVersion"))
            return {};
        if (remove)
            return Rejected("caClass", "ECDbMap custom attributes cannot be removed after item creation.");
        if (caName.EqualsI("DbIndexList"))
            return newInstance != nullptr ? ValidateDbIndexListUpdate(persistedClass, *newInstance) : Result();
        if (caName.EqualsI("LinkTableRelationshipMap")) {
            if (persistedClass == nullptr)
                return Rejected("caClass", "LinkTableRelationshipMap can only be updated on an existing relationship class.");
            auto oldCA = persistedClass->GetCustomAttributeLocal("ECDbMap", "LinkTableRelationshipMap");
            if (!oldCA.IsValid())
                return Rejected("caClass", "LinkTableRelationshipMap cannot be added after relationship creation.");
            if (newInstance != nullptr) {
                for (Utf8CP propName : {"SourceECInstanceIdColumn", "TargetECInstanceIdColumn", "CreateForeignKeyConstraints"}) {
                    ECValue oldVal, newVal;
                    oldCA->GetValue(oldVal, propName);
                    newInstance->GetValue(newVal, propName);
                    if (!oldVal.Equals(newVal))
                        return Rejected("instance", "Only AllowDuplicateRelationships on LinkTableRelationshipMap may be changed after creation.");
                }
            }
            return {};
        }
        if (caName.EqualsI("QueryView")) {
            if (persistedClass == nullptr || !persistedClass->GetCustomAttributeLocal("ECDbMap", "QueryView").IsValid())
                return Rejected("caClass", "QueryView cannot be added to an existing class; only modifying an existing QueryView is supported.");
            return persistedSchema != nullptr ? ctx.RequireMajorUpgrade(*persistedSchema) : Result();
        }
        return Rejected("caClass", "ECDbMap custom attributes cannot be added or modified after item creation.");
    }
    if (caSchemaName.EqualsI("CoreCustomAttributes") && caName.EqualsI("ClassHasCurrentTimeStampProperty"))
        return Rejected("caClass", "CoreCustomAttributes:ClassHasCurrentTimeStampProperty cannot be added, modified, or removed after class creation.");
    if (caSchemaName.EqualsI("CoreCustomAttributes") && caName.EqualsI("IsMixin")) {
        if (remove)
            return Rejected("caClass", "CoreCustomAttributes:IsMixin cannot be removed.");
        if (persistedClass == nullptr || !persistedClass->GetCustomAttributeLocal("CoreCustomAttributes", "IsMixin").IsValid())
            return Rejected("caClass", "CoreCustomAttributes:IsMixin cannot be added after class creation.");
        if (newInstance != nullptr) {
            auto oldCA = persistedClass->GetCustomAttributeLocal("CoreCustomAttributes", "IsMixin");
            auto resolveAppliesTo = [&](IECInstanceCR inst) -> ECClassCP {
                ECValue v;
                if (ECObjectsStatus::Success != inst.GetValue(v, "AppliesToEntityClass") || v.IsNull() || !v.IsString())
                    return nullptr;
                Utf8String targetAlias, targetClassName;
                if (ECClass::ParseClassName(targetAlias, targetClassName, v.GetUtf8CP()) != ECObjectsStatus::Success)
                    return nullptr;
                if (targetAlias.empty())
                    targetAlias = persistedClass->GetSchema().GetName();
                return ctx.m_db.Schemas().GetClass(targetAlias, targetClassName, SchemaLookupMode::AutoDetect);
            };
            ECClassCP oldApplyTo = resolveAppliesTo(*oldCA);
            ECClassCP newApplyTo = resolveAppliesTo(*newInstance);
            if (newApplyTo == nullptr || !newApplyTo->IsEntityClass() || newApplyTo->IsMixin())
                return Rejected("instance", "The new AppliesToEntityClass on IsMixin must reference an existing entity class.");
            if (oldApplyTo == nullptr || !oldApplyTo->Is(newApplyTo))
                return Rejected("instance", "Modifying IsMixin.AppliesToEntityClass is only supported when the old class is derived from the new class.");
            for (uint32_t i = 0; i < caClass.GetPropertyCount(); ++i) {
                ECValue oldVal, newVal;
                oldCA->GetValue(oldVal, i);
                newInstance->GetValue(newVal, i);
                if (!oldVal.Equals(newVal)) {
                    uint32_t appliesIdx = 0;
                    if (newInstance->GetEnablerR().GetPropertyIndex(appliesIdx, "AppliesToEntityClass") == ECObjectsStatus::Success && i != appliesIdx)
                        return Rejected("instance", "Only AppliesToEntityClass on CoreCustomAttributes:IsMixin may be modified.");
                }
            }
        }
        return {};
    }
    return {};
}

Result PrepareAttribute(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, bool remove) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    if (!parser.CheckKeys(remove ? std::initializer_list<Utf8CP>{"container", "caClass"} : std::initializer_list<Utf8CP>{"container", "caClass", "instance", "ordinal"}))
        return result;
    bool reorder = parser.Has("ordinal");
    uint32_t requestedOrdinal = 0;
    if (reorder && !parser.UInt32("ordinal", requestedOrdinal))
        return result;
    Utf8String containerName, className;
    if (!parser.String("container", containerName, true) || !parser.String("caClass", className, true))
        return result;
    auto caClass = FindClass(ctx, className);
    if (caClass == nullptr || !caClass->IsCustomAttributeClass())
        return Missing("caClass", "Custom attribute class not found.");
    ECSchemaCP schema = FindSchema(ctx, containerName);
    ECClassCP ecClass = nullptr;
    ECPropertyCP property = nullptr;
    Utf8String propertyName;
    bool isSourceConstraint = false;
    bool isTargetConstraint = false;
    auto type = SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema;
    BeInt64Id containerId;
    if (schema != nullptr)
        containerId = schema->GetId();
    else {
        ecClass = FindClass(ctx, containerName);
        if (ecClass == nullptr) {
            auto separator = containerName.find_last_of('.');
            if (separator == Utf8String::npos)
                return Missing("container", "Expected schema, Schema:Class, Schema:Class.Property, or Schema:RelClass.source|target.");
            ecClass = FindClass(ctx, containerName.substr(0, separator));
            propertyName = containerName.substr(separator + 1);
            if (ecClass == nullptr)
                return Missing("container", "Custom attribute container class not found.");
            property = ecClass->GetPropertyP(propertyName, false);
            if (property != nullptr) {
                containerId = property->GetId();
                type = SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property;
            } else if (ecClass->IsRelationshipClass() && (propertyName.EqualsI("source") || propertyName.EqualsI("target"))) {
                isSourceConstraint = propertyName.EqualsI("source");
                isTargetConstraint = !isSourceConstraint;
                type = isSourceConstraint
                           ? SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint
                           : SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint;
                Statement cStmt;
                if (BE_SQLITE_OK != cStmt.Prepare(ctx.m_db,
                                                  "SELECT Id FROM main.ec_RelationshipConstraint WHERE RelationshipClassId=? AND RelationshipEnd=?") ||
                    BE_SQLITE_OK != cStmt.BindId(1, ecClass->GetId()) ||
                    BE_SQLITE_OK != cStmt.BindInt(2, isSourceConstraint ? int(ECRelationshipEnd_Source) : int(ECRelationshipEnd_Target)) ||
                    cStmt.Step() != BE_SQLITE_ROW)
                    return DatabaseError("Resolving relationship constraint container failed.");
                containerId = cStmt.GetValueId<BeInt64Id>(0);
            } else
                return Missing("container", "Custom attribute container property must be declared locally.");
        } else {
            containerId = ecClass->GetId();
            type = SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class;
        }
        schema = &ecClass->GetSchema();
    }
    if (!reorder) {
        result = ValidateCustomAttributeRule(ctx, *caClass, remove, schema, ecClass, property, nullptr);
        if (!result.IsOk())
            return result;
    }
    ECSchemaPtr working;
    result = Copy(ctx, working, *schema);
    if (!result.IsOk())
        return result;
    IECCustomAttributeContainerP container = working.get();
    if (ecClass != nullptr) {
        auto workingClass = working->GetClassP(ecClass->GetName().c_str());
        if (workingClass == nullptr)
            return Rejected("container", "Class missing from working schema.");
        if (isSourceConstraint && workingClass->GetRelationshipClassP() != nullptr)
            container = &workingClass->GetRelationshipClassP()->GetSource();
        else if (isTargetConstraint && workingClass->GetRelationshipClassP() != nullptr)
            container = &workingClass->GetRelationshipClassP()->GetTarget();
        else
            container = property == nullptr ? static_cast<IECCustomAttributeContainerP>(workingClass) : static_cast<IECCustomAttributeContainerP>(workingClass->GetPropertyP(propertyName, false));
    }
    if (container == nullptr)
        return Rejected("container", "Container missing from working schema.");
    BeInt64Id existingId;
    int64_t ordinal = 0;
    Utf8String existingXml;
    {
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(ctx.m_db,
                                         "SELECT Id,Ordinal,Instance FROM main.ec_CustomAttribute WHERE ContainerId=? AND ContainerType=? AND ClassId=?") ||
            BE_SQLITE_OK != stmt.BindId(1, containerId) || BE_SQLITE_OK != stmt.BindInt64(2, int64_t(type)) ||
            BE_SQLITE_OK != stmt.BindId(3, caClass->GetId()))
            return DatabaseError("Preparing custom attribute lookup failed.");
        auto step = stmt.Step();
        if (step == BE_SQLITE_ROW) {
            existingId = BeInt64Id(stmt.GetValueUInt64(0));
            ordinal = stmt.GetValueInt64(1);
            existingXml = stmt.GetValueText(2);
        } else if (step != BE_SQLITE_DONE)
            return DatabaseError("Reading the custom attribute failed.");
    }
    CustomAttribute attribute;
    BeInt64Id displacedId;
    int64_t previousOrdinal = ordinal, temporaryOrdinal = 0;
    if (remove) {
        if (!existingId.IsValid())
            return Missing("caClass", "The container does not define this custom attribute locally.");
        if (!container->RemoveCustomAttribute(caClass->GetSchema().GetName(), caClass->GetName()))
            return Rejected("caClass", "Cannot remove the custom attribute from the working copy.");
    } else {
        result = PrepareCustomAttribute(ctx, *working, *container, className, parser.Value("instance"), attribute);
        if (!result.IsOk())
            return result;
        auto appliedCA = container->GetCustomAttributeLocal(caClass->GetSchema().GetName(), caClass->GetName());
        if (reorder) {
            if (!existingId.IsValid() || existingXml != attribute.m_xml)
                return Rejected("ordinal", "Reordering requires an existing, unchanged custom attribute instance.");
            Statement displaced;
            if (BE_SQLITE_OK != displaced.Prepare(ctx.m_db,
                                                  "SELECT Id,(SELECT MAX(Ordinal)+1 FROM main.ec_CustomAttribute WHERE ContainerId=?1 AND ContainerType=?2)"
                                                  " FROM main.ec_CustomAttribute WHERE ContainerId=?1 AND ContainerType=?2 AND Ordinal=?3") ||
                BE_SQLITE_OK != displaced.BindId(1, containerId) ||
                BE_SQLITE_OK != displaced.BindInt64(2, int64_t(type)) ||
                BE_SQLITE_OK != displaced.BindInt64(3, requestedOrdinal))
                return DatabaseError("Preparing custom attribute order lookup failed.");
            auto status = displaced.Step();
            if (status == BE_SQLITE_DONE)
                return Invalid("ordinal", "The requested ordinal must belong to an existing local custom attribute.");
            if (status != BE_SQLITE_ROW)
                return DatabaseError("Reading custom attribute order failed.");
            displacedId = displaced.GetValueId<BeInt64Id>(0);
            temporaryOrdinal = displaced.GetValueInt64(1);
            ordinal = requestedOrdinal;
        } else {
            result = ValidateCustomAttributeRule(ctx, *caClass, false, schema, ecClass, property, appliedCA.get());
            if (!result.IsOk())
                return result;
        }
        if (caClass->GetName().EqualsI("ReservedPropertyNames") && ecClass != nullptr) {
            for (auto cls : working->GetClasses())
                for (auto prop : cls->GetProperties(false)) {
                    result = CheckReservedPropertyName(*cls, prop->GetName());
                    if (!result.IsOk())
                        return result;
                }
        }
        if (!existingId.IsValid()) {
            Statement stmt;
            if (BE_SQLITE_OK != stmt.Prepare(ctx.m_db,
                                             "SELECT COALESCE(MAX(Ordinal)+1,0) FROM main.ec_CustomAttribute WHERE ContainerId=? AND ContainerType=?") ||
                BE_SQLITE_OK != stmt.BindId(1, containerId) || BE_SQLITE_OK != stmt.BindInt64(2, int64_t(type)) ||
                stmt.Step() != BE_SQLITE_ROW)
                return DatabaseError("Reading the custom attribute ordinal failed.");
            ordinal = stmt.GetValueInt64(0);
        }
    }
    result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    auto schemaId = schema->GetId();
    auto classId = ecClass == nullptr ? ECClassId() : ecClass->GetId();
    out.m_apply = [remove, reorder, schemaId, classId, containerId, type, existingId, ordinal, attribute,
                   displacedId, previousOrdinal, temporaryOrdinal](SchemaEditContext& apply) {
        Result result;
        if (reorder) {
            if (displacedId == existingId)
                return Result();
            result = apply.m_writer.Update("ec_CustomAttribute", existingId, {{"Ordinal", temporaryOrdinal}});
            if (!result.IsOk())
                return result;
            result = apply.m_writer.Update("ec_CustomAttribute", displacedId, {{"Ordinal", previousOrdinal}});
            if (!result.IsOk())
                return result;
            result = apply.m_writer.Update("ec_CustomAttribute", existingId, {{"Ordinal", ordinal}});
            if (result.IsOk())
                apply.m_affectedSchemas.insert(schemaId);
            return result;
        }
        if (remove)
            result = apply.m_writer.Delete("ec_CustomAttribute", existingId);
        else {
            result = apply.AddReference(schemaId, attribute.m_schema);
            if (!result.IsOk())
                return result;
            if (existingId.IsValid())
                result = apply.m_writer.Update("ec_CustomAttribute", existingId, {{"Instance", attribute.m_xml}});
            else {
                auto id = apply.m_db.GetImpl().GetIdFactory().CustomAttribute().NextId();
                result = apply.m_writer.Insert("ec_CustomAttribute",
                                               {{"Id", int64_t(id.GetValue())}, {"ContainerId", int64_t(containerId.GetValue())}, {"ContainerType", int64_t(type)}, {"ClassId", int64_t(attribute.m_class.GetValue())}, {"Ordinal", ordinal}, {"Instance", attribute.m_xml}});
            }
        }
        if (!result.IsOk())
            return result;
        Affected(apply, schemaId, classId);
        return classId.IsValid() ? AffectClassAndUsers(apply, classId) : Result();
    };
    return {};
}
}

void RegisterSchemaCustomAttributeCommands(SchemaCommandRegistry& registry) {
    Register(registry, "SetCustomAttribute", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return PrepareAttribute(ctx, args, out, false); });
    Register(registry, "RemoveCustomAttribute", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return PrepareAttribute(ctx, args, out, true); });
}
END_BENTLEY_SQLITE_EC_NAMESPACE
