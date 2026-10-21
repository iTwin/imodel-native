/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommandHelpers.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
using namespace SchemaCommands;

//! Resolves a persisted property category or KoQ to its ID and its copy in the working graph, but only
//! when that graph is or directly references the item's schema, so no reference must be added.
//! Leaves referenced null when the caller must fall back to the schema reader.
Result FindDirectlyReferencedItem(SchemaEditContext& ctx, ECSchemaCR working, bool category, Utf8StringCR schemaName, Utf8StringCR item,
                                  ECSchemaId& schemaId, int64_t& itemId, ECSchemaCP& referenced) {
    referenced = nullptr;
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != ctx.m_db.GetCachedStatement(stmt, category
                                                              ? "SELECT i.Id,s.Id,s.Name FROM main.ec_PropertyCategory i JOIN main.ec_Schema s ON s.Id=i.SchemaId"
                                                                " WHERE i.Name=?2 COLLATE NOCASE AND (s.Name=?1 COLLATE NOCASE OR s.Alias=?1 COLLATE NOCASE)"
                                                                " ORDER BY s.Name=?1 COLLATE NOCASE DESC LIMIT 1"
                                                              : "SELECT i.Id,s.Id,s.Name FROM main.ec_KindOfQuantity i JOIN main.ec_Schema s ON s.Id=i.SchemaId"
                                                                " WHERE i.Name=?2 COLLATE NOCASE AND (s.Name=?1 COLLATE NOCASE OR s.Alias=?1 COLLATE NOCASE)"
                                                                " ORDER BY s.Name=?1 COLLATE NOCASE DESC LIMIT 1") ||
        BE_SQLITE_OK != stmt->BindText(1, schemaName, Statement::MakeCopy::No) ||
        BE_SQLITE_OK != stmt->BindText(2, item, Statement::MakeCopy::No))
        return DatabaseError("Preparing the schema item lookup failed.");
    if (stmt->Step() != BE_SQLITE_ROW)
        return {};
    Utf8String name = stmt->GetValueText(2);
    ECSchemaCP candidate = working.GetName().EqualsI(name) ? &working : nullptr;
    for (auto const& entry : working.GetReferencedSchemas())
        if (candidate == nullptr && entry.second->GetName().EqualsI(name))
            candidate = entry.second.get();
    if (candidate == nullptr ||
        (category ? candidate->GetPropertyCategoryCP(item.c_str()) == nullptr : candidate->GetKindOfQuantityCP(item.c_str()) == nullptr))
        return {};
    itemId = stmt->GetValueInt64(0);
    schemaId = stmt->GetValueId<ECSchemaId>(1);
    referenced = candidate;
    return {};
}

Result Reserved(ECClassCR ecClass, Utf8StringCR name) {
    if (name.EqualsI("ECInstanceId") || name.EqualsI("ECClassId"))
        return Rejected("name", "ECDb system property names are reserved.");
    for (auto schemaName : {"BisCore", "ECDbSchemaPolicies"}) {
        auto ca = ecClass.GetCustomAttributeLocal(schemaName, "ReservedPropertyNames");
        if (!ca.IsValid())
            continue;
        ECValue names;
        if (ECObjectsStatus::Success != ca->GetValue(names, "PropertyNames") || names.IsNull() || !names.IsArray())
            return Rejected("name", "The ReservedPropertyNames policy is invalid.");
        for (uint32_t i = 0; i < names.GetArrayInfo().GetCount(); ++i) {
            ECValue entry;
            if (ECObjectsStatus::Success != ca->GetValue(entry, "PropertyNames", i) || entry.IsNull() || !entry.IsString())
                return Rejected("name", "Cannot read the ReservedPropertyNames policy.");
            if (name.EqualsI(entry.GetUtf8CP()))
                return Rejected("name", "The property name is prohibited by ReservedPropertyNames.");
        }
    }
    for (auto base : ecClass.GetBaseClasses()) {
        auto result = Reserved(*base, name);
        if (!result.IsOk())
            return result;
    }
    return {};
}

Result PropertyInstances(SchemaEditContext& ctx, ECClassCR owner, Utf8CP property) {
    if (property == nullptr) {
        // New grouped classes have no instance mapping until the group is completed.
        if (ctx.IsCreatedInCurrentUnit(owner.GetId()))
            return {};
        return ctx.CheckInstances(owner);
    }
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != ctx.m_db.GetCachedStatement(stmt,
                                     "WITH RECURSIVE derived(Id) AS (SELECT ? UNION SELECT b.ClassId FROM main.ec_ClassHasBaseClasses b JOIN derived d ON b.BaseClassId=d.Id)"
                                     " SELECT c.Id FROM derived d JOIN main.ec_Class c ON c.Id=d.Id") ||
        BE_SQLITE_OK != stmt->BindId(1, owner.GetId()))
        return DatabaseError("Preparing inherited property data checks failed.");
    DbResult step;
    while ((step = stmt->Step()) == BE_SQLITE_ROW) {
        auto ecClass = ctx.m_db.Schemas().GetClass(stmt->GetValueId<ECClassId>(0));
        if (ecClass == nullptr)
            return DatabaseError("Resolving a property data-check class failed.");
        if (ecClass->IsMixin())
            continue;
        auto definition = ecClass->GetPropertyP(property);
        if (definition == nullptr)
            return Missing("property", "The inherited property was not found during its data check.");
        auto result = CheckPropertyData(ctx, *ecClass, {definition});
        if (!result.IsOk())
            return result;
    }
    return step == BE_SQLITE_DONE ? Result() : DatabaseError("Reading inherited property data-check classes failed.");
}

Result FindMemberAccesses(ECPropertyCR property, ECPropertyId member, PropertyAccess& path, bvector<PropertyAccess>& accesses) {
    if (path.size() >= 256)
        return Unsupported("property", "The nested struct access path exceeds 256 properties.");
    path.push_back(&property);
    if (property.GetId() == member)
        accesses.push_back(path);
    else {
        ECStructClassCP structure = nullptr;
        if (property.GetIsStruct())
            structure = &property.GetAsStructProperty()->GetType();
        else if (property.GetIsStructArray())
            structure = &property.GetAsStructArrayProperty()->GetStructElementType();
        if (structure != nullptr)
            for (auto child : structure->GetProperties(true)) {
                auto result = FindMemberAccesses(*child, member, path, accesses);
                if (!result.IsOk())
                    return result;
            }
    }
    path.pop_back();
    return {};
}

Result PrepareStructMemberRemoval(SchemaEditContext& ctx, ECPropertyCR property, PropertyMapRemoval& removal) {
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != ctx.m_db.GetCachedStatement(stmt,
                                     "WITH RECURSIVE users(Id) AS (SELECT ? UNION SELECT b.ClassId FROM main.ec_ClassHasBaseClasses b JOIN users u ON b.BaseClassId=u.Id"
                                     " UNION SELECT p.ClassId FROM main.ec_Property p JOIN users u ON p.StructClassId=u.Id)"
                                     " SELECT c.Id FROM main.ec_Class c JOIN users u ON c.Id=u.Id") ||
        BE_SQLITE_OK != stmt->BindId(1, property.GetClass().GetId()))
        return DatabaseError("Preparing struct-property usage discovery failed.");
    DbResult step;
    while ((step = stmt->Step()) == BE_SQLITE_ROW) {
        auto owner = ctx.m_db.Schemas().GetClass(stmt->GetValueId<ECClassId>(0));
        if (owner == nullptr)
            return DatabaseError("Resolving a struct-property user failed.");
        if (owner->IsStructClass() || owner->IsMixin())
            continue;
        bvector<PropertyAccess> accesses;
        PropertyAccess path;
        for (auto root : owner->GetProperties(true)) {
            auto result = FindMemberAccesses(*root, property.GetId(), path, accesses);
            if (!result.IsOk())
                return result;
        }
        for (auto const& access : accesses) {
            auto result = CheckPropertyData(ctx, *owner, access);
            if (!result.IsOk())
                return result;
            Utf8String name;
            for (auto part : access) {
                if (!name.empty())
                    name.append(".");
                name.append(part->GetName());
            }
            result = removal.Select(ctx, owner->GetId(), name);
            if (!result.IsOk())
                return result;
        }
    }
    return step == BE_SQLITE_DONE ? Result() : DatabaseError("Reading struct-property users failed.");
}

Result ValidatePrimitiveTypeUpgrade(SchemaEditContext& ctx, ECSchemaCR schemaToCheckVersion,
                                    ECPropertyCR oldProp, PrimitiveType newPrimType, ECEnumerationCP newEnum,
                                    Utf8CP argName) {
    if (!oldProp.GetIsPrimitive())
        return Rejected(argName, "Changing the type of a non-primitive ECProperty is not supported.");
    auto const* oldPrim = oldProp.GetAsPrimitiveProperty();
    auto const* oldEnum = oldPrim->GetEnumeration();
    PrimitiveType oldPrimType = oldEnum != nullptr ? oldEnum->GetType() : oldPrim->GetType();
    PrimitiveType targetPrimType = newEnum != nullptr ? newEnum->GetType() : newPrimType;
    bool sameEnum = (oldEnum == nullptr && newEnum == nullptr) ||
                    (oldEnum != nullptr && newEnum != nullptr && oldEnum->GetId() == newEnum->GetId());
    if (sameEnum && oldPrimType == targetPrimType)
        return {};
    if (oldPrimType == PRIMITIVETYPE_Point2d || oldPrimType == PRIMITIVETYPE_Point3d ||
        targetPrimType == PRIMITIVETYPE_Point2d || targetPrimType == PRIMITIVETYPE_Point3d)
        return Rejected(argName, "Changing to or from Point2d/Point3d primitive type is not supported.");
    if (newEnum != nullptr && newEnum->GetIsStrict() && !sameEnum)
        return Rejected(argName, "Type change to a Strict ECEnumeration is not supported.");
    if (oldPrimType != targetPrimType) {
        auto verRes = ctx.RequireMajorUpgrade(schemaToCheckVersion);
        if (!verRes.IsOk())
            return verRes;
    }
    return {};
}

}

namespace SchemaCommands {
Result CheckAndCollectDerivedPropertyOverrides(SchemaEditContext& ctx, ECClassCR baseClass, ECPropertyCR newBaseProp,
                                               ECEnumerationCP persistedBaseEnum, KindOfQuantityCP persistedBaseKoq,
                                               bvector<DerivedPropertyTypeUpgrade>& outUpgrades,
                                               bool includeSelf) {
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != ctx.m_db.GetCachedStatement(stmt,
                                     includeSelf
                                         ? "SELECT DISTINCT c.Id FROM main.ec_cache_ClassHierarchy h JOIN main.ec_Class c ON c.Id=h.ClassId WHERE h.BaseClassId=?"
                                         : "SELECT DISTINCT c.Id FROM main.ec_cache_ClassHierarchy h JOIN main.ec_Class c ON c.Id=h.ClassId WHERE h.BaseClassId=? AND h.ClassId<>?") ||
        BE_SQLITE_OK != stmt->BindId(1, baseClass.GetId()) ||
        (!includeSelf && BE_SQLITE_OK != stmt->BindId(2, baseClass.GetId())))
        return DatabaseError("Preparing derived class override query failed.");
    DbResult step;
    while ((step = stmt->Step()) == BE_SQLITE_ROW) {
        auto derivedClass = ctx.m_db.Schemas().GetClass(stmt->GetValueId<ECClassId>(0));
        if (derivedClass == nullptr)
            return DatabaseError("Resolving derived class failed.");
        auto derivedProp = derivedClass->GetPropertyP(newBaseProp.GetName().c_str(), false);
        if (derivedProp == nullptr)
            continue;
        if (!derivedProp->GetName().Equals(newBaseProp.GetName()))
            return Rejected("name", "A derived class declares a property with the same name in a different case.");
        auto derivedKoq = derivedProp->GetKindOfQuantity();
        if (persistedBaseKoq != nullptr && derivedKoq != nullptr &&
            !persistedBaseKoq->GetPersistenceUnit()->GetFullName().EqualsIAscii(derivedKoq->GetPersistenceUnit()->GetFullName()))
            return Rejected("koq", "A derived class declares a property with an incompatible KindOfQuantity.");
        if (newBaseProp.GetIsPrimitive()) {
            if (!derivedProp->GetIsPrimitive())
                return Rejected("kind", "A derived class declares a non-primitive property with the same name.");
            auto const* newPrim = newBaseProp.GetAsPrimitiveProperty();
            PrimitiveType targetPrim = persistedBaseEnum != nullptr ? persistedBaseEnum->GetType() : newPrim->GetType();
            auto const* derivedPrim = derivedProp->GetAsPrimitiveProperty();
            auto const* derivedEnum = derivedPrim->GetEnumeration();
            PrimitiveType derivedPrimType = derivedEnum != nullptr ? derivedEnum->GetType() : derivedPrim->GetType();
            bool sameEnum = (persistedBaseEnum == nullptr && derivedEnum == nullptr) ||
                            (persistedBaseEnum != nullptr && derivedEnum != nullptr && persistedBaseEnum->GetId() == derivedEnum->GetId());
            if (!sameEnum || derivedPrimType != targetPrim) {
                auto res = Editable(ctx, derivedClass->GetSchema());
                if (!res.IsOk())
                    return res;
                res = ValidatePrimitiveTypeUpgrade(ctx, derivedClass->GetSchema(), *derivedProp, targetPrim, persistedBaseEnum,
                                                   persistedBaseEnum != nullptr ? "enum" : "type");
                if (!res.IsOk())
                    return res;
                DerivedPropertyTypeUpgrade upg;
                upg.m_propertyId = derivedProp->GetId();
                upg.m_ownerSchemaId = derivedClass->GetSchema().GetId();
                if (persistedBaseEnum != nullptr) {
                    upg.m_enumSchemaId = persistedBaseEnum->GetSchema().GetId();
                    upg.m_row.push_back({"PrimitiveType", SchemaMetaValue(nullptr)});
                    upg.m_row.push_back({"EnumerationId", int64_t(persistedBaseEnum->GetId().GetValue())});
                } else {
                    upg.m_row.push_back({"PrimitiveType", int64_t(targetPrim)});
                    upg.m_row.push_back({"EnumerationId", SchemaMetaValue(nullptr)});
                }
                outUpgrades.push_back(std::move(upg));
            }
        } else if (newBaseProp.GetIsStruct()) {
            if (!derivedProp->GetIsStruct() ||
                !Utf8String(derivedProp->GetAsStructProperty()->GetType().GetFullName()).EqualsIAscii(newBaseProp.GetAsStructProperty()->GetType().GetFullName()))
                return Rejected("struct", "A derived class declares a property with the same name and an incompatible struct type.");
        } else if (newBaseProp.GetIsPrimitiveArray()) {
            if (!derivedProp->GetIsPrimitiveArray())
                return Rejected("kind", "A derived class declares a property with the same name and an incompatible kind.");
            auto const* newArr = newBaseProp.GetAsPrimitiveArrayProperty();
            auto const* derArr = derivedProp->GetAsPrimitiveArrayProperty();
            if (newArr->GetType() != derArr->GetType() ||
                ((persistedBaseEnum == nullptr) != (derArr->GetEnumeration() == nullptr)) ||
                (persistedBaseEnum != nullptr && derArr->GetEnumeration()->GetId() != persistedBaseEnum->GetId()))
                return Rejected("type", "A derived class declares an array property with the same name and an incompatible element type.");
        } else if (newBaseProp.GetIsStructArray()) {
            if (!derivedProp->GetIsStructArray() ||
                !Utf8String(derivedProp->GetAsStructArrayProperty()->GetStructElementType().GetFullName()).EqualsIAscii(newBaseProp.GetAsStructArrayProperty()->GetStructElementType().GetFullName()))
                return Rejected("struct", "A derived class declares a struct array property with the same name and an incompatible struct type.");
        } else if (newBaseProp.GetIsNavigation()) {
            if (!derivedProp->GetIsNavigation())
                return Rejected("kind", "A derived class declares a non-navigation property with the same name.");
            auto const* newNav = newBaseProp.GetAsNavigationProperty();
            auto const* derNav = derivedProp->GetAsNavigationProperty();
            if (newNav->GetDirection() != derNav->GetDirection() ||
                !derNav->GetRelationshipClass()->Is(newNav->GetRelationshipClass()))
                return Rejected("relationship", "A derived class declares a navigation property with an incompatible relationship or direction.");
        }
    }
    return step == BE_SQLITE_DONE ? Result() : DatabaseError("Reading derived class overrides failed.");
}

Result ValidateAndPrepareBaseClassPropertyOverrides(SchemaEditContext& ctx, ECSchemaR working, ECClassCR targetPersisted,
                                                    ECClassCR newBasePersisted, ECClassCP oldBasePersisted,
                                                    bvector<DerivedPropertyTypeUpgrade>& outUpgrades) {
    for (auto const* baseProp : newBasePersisted.GetProperties(true)) {
        if (oldBasePersisted != nullptr && oldBasePersisted->GetPropertyP(baseProp->GetName().c_str(), true) != nullptr)
            continue;
        ECEnumerationCP baseEnum = baseProp->GetIsPrimitive() ? baseProp->GetAsPrimitiveProperty()->GetEnumeration() : (baseProp->GetIsPrimitiveArray() ? baseProp->GetAsPrimitiveArrayProperty()->GetEnumeration() : nullptr);
        KindOfQuantityCP baseKoq = baseProp->GetKindOfQuantity();
        size_t beforeCount = outUpgrades.size();
        auto res = CheckAndCollectDerivedPropertyOverrides(ctx, targetPersisted, *baseProp, baseEnum, baseKoq, outUpgrades, true);
        if (!res.IsOk())
            return res;
        if (outUpgrades.size() > beforeCount && baseProp->GetIsPrimitive()) {
            auto const* newPrim = baseProp->GetAsPrimitiveProperty();
            PrimitiveType targetPrim = baseEnum != nullptr ? baseEnum->GetType() : newPrim->GetType();
            Statement dStmt;
            if (BE_SQLITE_OK == dStmt.Prepare(ctx.m_db,
                                              "SELECT DISTINCT c.Id FROM main.ec_cache_ClassHierarchy h JOIN main.ec_Class c ON c.Id=h.ClassId WHERE h.BaseClassId=?") &&
                BE_SQLITE_OK == dStmt.BindId(1, targetPersisted.GetId())) {
                while (dStmt.Step() == BE_SQLITE_ROW) {
                    auto dClass = ctx.m_db.Schemas().GetClass(dStmt.GetValueId<ECClassId>(0));
                    if (dClass == nullptr)
                        continue;
                    auto dProp = dClass->GetPropertyP(baseProp->GetName().c_str(), false);
                    if (dProp == nullptr || !dProp->GetIsPrimitive())
                        continue;
                    if (auto wDerivedClass = WorkingClass(working, *dClass))
                        if (auto wDerivedProp = wDerivedClass->GetPropertyP(baseProp->GetName().c_str(), false))
                            if (auto wDerivedPrim = wDerivedProp->GetAsPrimitivePropertyP()) {
                                if (baseEnum != nullptr) {
                                    auto refRes = Reference(ctx, working, baseEnum->GetSchema());
                                    if (!refRes.IsOk())
                                        return refRes;
                                    if (auto wEnumSchema = WorkingSchema(working, baseEnum->GetSchema().GetName()))
                                        if (auto wEnum = wEnumSchema->GetEnumerationCP(baseEnum->GetName().c_str()))
                                            wDerivedPrim->SetType(*wEnum);
                                } else
                                    wDerivedPrim->SetType(targetPrim);
                            }
                }
            }
        }
    }
    return {};
}
}

namespace {
using namespace SchemaCommands;

Result ParseAndSetMinMaxValue(SchemaCommandArgParser& parser, bool minimum, ECPropertyR property, SchemaMetaRow& row) {
    auto key = minimum ? "minValue" : "maxValue";
    if (!parser.Has(key))
        return {};
    auto raw = parser.Value(key);
    PrimitiveType primType = PRIMITIVETYPE_String;
    if (auto prim = property.GetAsPrimitiveProperty())
        primType = prim->GetEnumeration() != nullptr ? prim->GetEnumeration()->GetType() : prim->GetType();
    else if (auto arr = property.GetAsPrimitiveArrayProperty())
        primType = arr->GetEnumeration() != nullptr ? arr->GetEnumeration()->GetType() : arr->GetType();
    else
        return Invalid(key, "minValue/maxValue require a primitive or primitiveArray property.");
    ECValue val;
    if (raw.isNull() || (raw.isString() && Utf8String(raw.ToUtf8CP()).empty())) {
        val.SetToNull();
    } else {
        switch (primType) {
        case PRIMITIVETYPE_Integer:
            if (raw.isNumeric())
                val.SetInteger(raw.asInt());
            else if (raw.isString())
                val.SetInteger(static_cast<int32_t>(std::strtoll(raw.ToUtf8CP(), nullptr, 10)));
            else
                return Invalid(key, "Expected an integer value.");
            break;
        case PRIMITIVETYPE_Long:
            if (raw.isNumeric())
                val.SetLong(raw.asInt64());
            else if (raw.isString())
                val.SetLong(std::strtoll(raw.ToUtf8CP(), nullptr, 10));
            else
                return Invalid(key, "Expected a 64-bit integer value.");
            break;
        case PRIMITIVETYPE_Double:
            if (raw.isNumeric())
                val.SetDouble(raw.asDouble());
            else if (raw.isString())
                val.SetDouble(std::strtod(raw.ToUtf8CP(), nullptr));
            else
                return Invalid(key, "Expected a double value.");
            break;
        case PRIMITIVETYPE_String:
            if (!raw.isString())
                return Invalid(key, "Expected a string value.");
            val.SetUtf8CP(raw.ToUtf8CP());
            break;
        default:
            return Invalid(key, "minValue/maxValue are only supported for numeric or string primitive types.");
        }
    }
    if (val.IsNull()) {
        if (primType != PRIMITIVETYPE_Integer && primType != PRIMITIVETYPE_Long && primType != PRIMITIVETYPE_Double)
            return Invalid(key, "Value bounds require a numeric primitive type.");
        if (minimum)
            property.ResetMinimumValue();
        else
            property.ResetMaximumValue();
    } else if (ECObjectsStatus::Success != (minimum ? property.SetMinimumValue(val) : property.SetMaximumValue(val)))
        return Rejected(key, "ECObjects rejected the property value bound.");
    row.push_back({minimum ? "PrimitiveTypeMinValue" : "PrimitiveTypeMaxValue",
                   val.IsNull() ? SchemaMetaValue(nullptr) : SchemaMetaValue(Utf8String(val.ToString()))});
    return {};
}

//! Defines a new or altered property on a working class. persisted is the owning class as read
//! from the database; it is null when the class is created by the same command, in which case
//! inlineOrdinal is the property's position and the class id is supplied when applying.
Result DefineProperty(SchemaEditContext& ctx, BeJsConst args, bool create, ECClassCP persisted, ECSchemaR workingSchema,
                      ECClassR workingClass, ECSchemaId schemaId, int64_t inlineOrdinal, PreparedProperty& out) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    if (persisted == nullptr && !parser.CheckKeys({"name", "kind", "type", "enum", "struct", "relationship", "direction",
                                                   "extendedType", "minOccurs", "maxOccurs", "readOnly", "label", "description", "priority", "koq", "category", "minLength", "maxLength", "minValue", "maxValue",
                                                   "customAttributes", "columnName", "isNullable", "isUnique", "collation", "fkOnDelete"}))
        return result;
    Utf8String name, kind = "primitive", typeName, enumName, structName, relationshipName, direction;
    if (!parser.String(create ? "name" : "property", name, true) ||
        !parser.String("kind", kind) || !parser.String("type", typeName) || !parser.String("enum", enumName) ||
        !parser.String("struct", structName) || !parser.String("relationship", relationshipName) || !parser.String("direction", direction))
        return result;
    ECSchemaP working = &workingSchema;
    ECClassP ecClass = &workingClass;
    ECPropertyP oldProperty = create ? nullptr : persisted->GetPropertyP(name, false);
    bool overridesInherited = create && persisted != nullptr && persisted->GetPropertyP(name, true) != nullptr;
    // The reader may lack properties created earlier in this run; the working graph has them.
    if (create && ecClass->GetPropertyP(name, false) != nullptr)
        return Rejected("name", "The class already declares this property.");
    ECPropertyP property = create ? nullptr : ecClass->GetPropertyP(name, false);
    SchemaMetaRow row;
    ECClassId affectedRelationship;
    bvector<ECSchemaId> references;
    bvector<DerivedPropertyTypeUpgrade> derivedUpgrades;
    ECEnumerationCP persistedTargetEnum = nullptr;
    KindOfQuantityCP persistedTargetKoq = nullptr;
    auto reference = [&](ECSchemaCR schema) {
        auto refResult = Reference(ctx, *working, schema);
        if (refResult.IsOk())
            references.push_back(schema.GetId());
        return refResult;
    };
    if (create) {
        // Pre-validate and apply any compatible derived-property type upgrades in the working graph before CreatePrimitiveProperty
        if (persisted != nullptr && kind.EqualsI("primitive") && (!enumName.empty() || !typeName.empty())) {
            ECEnumerationCP targetEnum = !enumName.empty() ? FindEnumeration(ctx, enumName) : nullptr;
            PrimitiveType targetPrim = PRIMITIVETYPE_String;
            if (targetEnum != nullptr || Primitive(typeName, targetPrim)) {
                if (targetEnum != nullptr)
                    targetPrim = targetEnum->GetType();
                Statement dStmt;
                if (BE_SQLITE_OK == dStmt.Prepare(ctx.m_db,
                                                  "SELECT DISTINCT c.Id FROM main.ec_cache_ClassHierarchy h"
                                                  " JOIN main.ec_Class c ON c.Id=h.ClassId WHERE h.BaseClassId=? AND h.ClassId<>?") &&
                    BE_SQLITE_OK == dStmt.BindId(1, persisted->GetId()) &&
                    BE_SQLITE_OK == dStmt.BindId(2, persisted->GetId())) {
                    while (dStmt.Step() == BE_SQLITE_ROW) {
                        auto dClass = ctx.m_db.Schemas().GetClass(dStmt.GetValueId<ECClassId>(0));
                        if (dClass == nullptr)
                            continue;
                        auto dProp = dClass->GetPropertyP(name.c_str(), false);
                        if (dProp == nullptr || !dProp->GetIsPrimitive())
                            continue;
                        auto res = ValidatePrimitiveTypeUpgrade(ctx, dClass->GetSchema(), *dProp, targetPrim, targetEnum,
                                                                targetEnum != nullptr ? "enum" : "type");
                        if (!res.IsOk())
                            return res;
                        if (auto wDerivedClass = WorkingClass(*working, *dClass))
                            if (auto wDerivedProp = wDerivedClass->GetPropertyP(name.c_str(), false))
                                if (auto wDerivedPrim = wDerivedProp->GetAsPrimitivePropertyP()) {
                                    if (targetEnum != nullptr) {
                                        auto refRes = reference(targetEnum->GetSchema());
                                        if (!refRes.IsOk())
                                            return refRes;
                                        if (auto wEnum = WorkingSchema(*working, targetEnum->GetSchema().GetName())->GetEnumerationCP(targetEnum->GetName().c_str()))
                                            wDerivedPrim->SetType(*wEnum);
                                    } else
                                        wDerivedPrim->SetType(targetPrim);
                                }
                    }
                }
            }
        }
        row = {{"ClassId", int64_t(0)}, {"Name", name}, {"IsReadonly", int64_t(0)}};
        ECObjectsStatus created = ECObjectsStatus::OperationNotSupported;
        if (kind.EqualsI("primitive") || kind.EqualsI("primitiveArray")) {
            if (parser.Has("struct") || parser.Has("relationship") || parser.Has("direction"))
                return Invalid("kind", "Primitive properties cannot specify struct, relationship, or direction.");
            ECEnumerationCP enumeration = nullptr;
            PrimitiveType primitive;
            if (!enumName.empty()) {
                enumeration = FindEnumeration(ctx, enumName);
                if (enumeration == nullptr)
                    return Missing("enum", "Enumeration not found.");
                persistedTargetEnum = enumeration;
                result = reference(enumeration->GetSchema());
                if (!result.IsOk())
                    return result;
                primitive = enumeration->GetType();
                if (!typeName.empty()) {
                    PrimitiveType supplied;
                    if (!Primitive(typeName, supplied) || supplied != primitive)
                        return Rejected("type", "The primitive type must match the enumeration backing type.");
                }
                enumeration = WorkingSchema(*working, enumeration->GetSchema().GetName())->GetEnumerationCP(enumeration->GetName().c_str());
                row.push_back({"EnumerationId", int64_t(persistedTargetEnum->GetId().GetValue())});
            } else if (!Primitive(typeName, primitive))
                return Invalid("type", "Specify a supported primitive type or enumeration.");
            if (kind.EqualsI("primitive")) {
                PrimitiveECPropertyP prim = nullptr;
                created = ecClass->CreatePrimitiveProperty(prim, name, primitive);
                if (created == ECObjectsStatus::Success && enumeration != nullptr)
                    created = prim->SetType(*enumeration);
                property = prim;
                row.push_back({"Kind", int64_t(PropertyKind::Primitive)});
            } else {
                PrimitiveArrayECPropertyP array = nullptr;
                created = enumeration != nullptr ? ecClass->CreatePrimitiveArrayProperty(array, name, *enumeration) : ecClass->CreatePrimitiveArrayProperty(array, name, primitive);
                property = array;
                row.push_back({"Kind", int64_t(PropertyKind::PrimitiveArray)});
            }
            if (enumeration == nullptr)
                row.push_back({"PrimitiveType", int64_t(primitive)});
            if (typeName.EqualsI("idlong") && !parser.Has("extendedType")) {
                if (property != nullptr && property->GetAsPrimitivePropertyP() != nullptr) {
                    if (ECObjectsStatus::Success != property->GetAsPrimitivePropertyP()->SetExtendedTypeName("Id"))
                        return Rejected("type", "Cannot assign the Id extended type.");
                } else if (property != nullptr && property->GetAsPrimitiveArrayPropertyP() != nullptr) {
                    if (ECObjectsStatus::Success != property->GetAsPrimitiveArrayPropertyP()->SetExtendedTypeName("Id"))
                        return Rejected("type", "Cannot assign the Id extended type.");
                }
                row.push_back({"ExtendedTypeName", Utf8String("Id")});
            }
        } else if (kind.EqualsI("struct") || kind.EqualsI("structArray")) {
            if (parser.Has("enum") || parser.Has("relationship") || parser.Has("direction"))
                return Invalid("kind", "Struct properties cannot specify enum, relationship, or direction.");
            if (!structName.empty() && !typeName.empty() && !structName.EqualsI(typeName))
                return Invalid("struct", "Specify the struct type once.");
            auto structClass = FindClass(ctx, structName.empty() ? typeName : structName);
            if (structClass == nullptr || !structClass->IsStructClass())
                return Missing("struct", "Struct class not found.");
            result = reference(structClass->GetSchema());
            if (!result.IsOk())
                return result;
            auto persistedStruct = structClass->GetId();
            structClass = WorkingClass(*working, *structClass);
            if (structClass == nullptr)
                return Rejected("struct", "Struct class is missing from the isolated working graph.");
            if (kind.EqualsI("struct")) {
                StructECPropertyP structure = nullptr;
                created = ecClass->CreateStructProperty(structure, name, *structClass->GetStructClassCP());
                property = structure;
                row.push_back({"Kind", int64_t(PropertyKind::Struct)});
            } else {
                StructArrayECPropertyP array = nullptr;
                created = ecClass->CreateStructArrayProperty(array, name, *structClass->GetStructClassCP());
                property = array;
                row.push_back({"Kind", int64_t(PropertyKind::StructArray)});
            }
            row.push_back({"StructClassId", int64_t(persistedStruct.GetValue())});
        } else if (kind.EqualsI("navigation")) {
            if (parser.Has("enum") || parser.Has("struct") || parser.Has("type"))
                return Invalid("kind", "Navigation properties specify relationship and direction, not type, enum, or struct.");
            auto relationship = FindClass(ctx, relationshipName);
            if (relationship == nullptr || !relationship->IsRelationshipClass())
                return Missing("relationship", "Relationship class not found.");
            auto relationshipMap = ctx.m_db.Schemas().Main().GetClassMap(*relationship);
            if (relationshipMap == nullptr)
                return Rejected("relationship", "The persisted relationship has no class map.");
            if (!MapStrategyExtendedInfo::IsForeignKeyMapping(relationshipMap->GetMapStrategy()))
                return Rejected("relationship", "A link-table relationship cannot be changed to foreign-key mapping by adding a navigation property.");
            result = reference(relationship->GetSchema());
            if (!result.IsOk())
                return result;
            ECRelatedInstanceDirection navDirection;
            if (direction.EqualsI("Forward"))
                navDirection = ECRelatedInstanceDirection::Forward;
            else if (direction.EqualsI("Backward"))
                navDirection = ECRelatedInstanceDirection::Backward;
            else
                return Invalid("direction", "Expected Forward or Backward.");
            auto relationshipId = relationship->GetId();
            affectedRelationship = relationshipId;
            relationship = WorkingClass(*working, *relationship);
            if (relationship == nullptr)
                return Rejected("relationship", "Relationship is missing from the isolated working graph.");
            NavigationECPropertyP nav = nullptr;
            if (ecClass->IsEntityClass())
                created = ecClass->GetEntityClassP()->CreateNavigationProperty(nav, name, *relationship->GetRelationshipClassCP(), navDirection);
            else if (ecClass->IsRelationshipClass())
                created = ecClass->GetRelationshipClassP()->CreateNavigationProperty(nav, name, *relationship->GetRelationshipClassCP(), navDirection);
            property = nav;
            row.push_back({"Kind", int64_t(PropertyKind::Navigation)});
            row.push_back({"NavigationRelationshipClassId", int64_t(relationshipId.GetValue())});
            row.push_back({"NavigationDirection", int64_t(navDirection)});
        } else
            return Invalid("kind", "Expected primitive, primitiveArray, struct, structArray, or navigation.");
        if (created != ECObjectsStatus::Success || property == nullptr)
            return Rejected("name", "ECObjects rejected the property definition or inherited-property override.");
    }
    if (property == nullptr)
        return Rejected("property", "Property missing from working schema.");
    if (!create && (parser.Has("type") || parser.Has("enum"))) {
        if (!oldProperty->GetIsPrimitive())
            return Rejected(parser.Has("enum") ? "enum" : "type", "Changing the type of a non-primitive ECProperty is not supported.");
        auto primProp = property->GetAsPrimitivePropertyP();
        ECEnumerationCP targetEnum = nullptr;
        PrimitiveType targetPrim;
        if (parser.Has("enum") && !enumName.empty()) {
            persistedTargetEnum = FindEnumeration(ctx, enumName);
            if (persistedTargetEnum == nullptr)
                return Missing("enum", "Enumeration not found.");
            result = reference(persistedTargetEnum->GetSchema());
            if (!result.IsOk())
                return result;
            targetPrim = persistedTargetEnum->GetType();
            if (!typeName.empty()) {
                PrimitiveType supplied;
                if (!Primitive(typeName, supplied) || supplied != targetPrim)
                    return Rejected("type", "The primitive type must match the enumeration backing type.");
            }
            targetEnum = WorkingSchema(*working, persistedTargetEnum->GetSchema().GetName())->GetEnumerationCP(persistedTargetEnum->GetName().c_str());
        } else if (parser.Has("type") && !typeName.empty()) {
            if (!Primitive(typeName, targetPrim))
                return Invalid("type", "Specify a supported primitive type.");
        } else
            return Invalid(parser.Has("enum") ? "enum" : "type", "Specify a valid primitive type or enumeration.");
        result = ValidatePrimitiveTypeUpgrade(ctx, persisted->GetSchema(), *oldProperty, targetPrim, persistedTargetEnum,
                                              persistedTargetEnum != nullptr ? "enum" : "type");
        if (!result.IsOk())
            return result;
        if (targetEnum != nullptr) {
            if (ECObjectsStatus::Success != primProp->SetType(*targetEnum))
                return Rejected("enum", "ECObjects rejected changing the property enumeration type.");
            row.push_back({"PrimitiveType", SchemaMetaValue(nullptr)});
            row.push_back({"EnumerationId", int64_t(persistedTargetEnum->GetId().GetValue())});
        } else {
            if (ECObjectsStatus::Success != primProp->SetType(targetPrim))
                return Rejected("type", "ECObjects rejected changing the primitive property type.");
            row.push_back({"PrimitiveType", int64_t(targetPrim)});
            row.push_back({"EnumerationId", SchemaMetaValue(nullptr)});
        }
    }
    if (parser.Has("minOccurs") || parser.Has("maxOccurs")) {
        if (property->GetAsArrayPropertyP() == nullptr)
            return Invalid("minOccurs", "Array occurrence bounds apply only to array properties.");
        if (!create)
            return Rejected(parser.Has("minOccurs") ? "minOccurs" : "maxOccurs",
                            "Changing minOccurs or maxOccurs for an Array ECProperty is not supported.");
    }
    if (create && property->GetAsArrayPropertyP() != nullptr) {
        uint32_t minimum = 0, maximum = std::numeric_limits<uint32_t>::max();
        if (!parser.UInt32("minOccurs", minimum))
            return result;
        if (parser.Has("maxOccurs")) {
            auto max = parser.Value("maxOccurs");
            if (!(max.isString() && Utf8String(max.asCString()).Equals("unbounded")) && !parser.UInt32("maxOccurs", maximum))
                return result;
        }
        if (minimum > maximum)
            return Invalid("minOccurs", "minOccurs exceeds maxOccurs.");
        auto array = property->GetAsArrayPropertyP();
        if (ECObjectsStatus::Success != array->SetMinOccurs(minimum) || ECObjectsStatus::Success != array->SetMaxOccurs(maximum))
            return Rejected("minOccurs", "ECObjects rejected the array bounds.");
        row.push_back({"ArrayMinOccurs", int64_t(minimum)});
        row.push_back({"ArrayMaxOccurs", maximum == std::numeric_limits<uint32_t>::max() ? SchemaMetaValue(nullptr) : SchemaMetaValue(int64_t(maximum))});
    }
    if (parser.Has("readOnly")) {
        bool readOnly = false;
        if (!parser.Boolean("readOnly", readOnly))
            return result;
        if (!create && !readOnly) {
            Statement dStmt;
            if (BE_SQLITE_OK == dStmt.Prepare(ctx.m_db,
                                              "SELECT DISTINCT c.Id FROM main.ec_cache_ClassHierarchy h JOIN main.ec_Class c ON c.Id=h.ClassId WHERE h.BaseClassId=?") &&
                BE_SQLITE_OK == dStmt.BindId(1, persisted->GetId())) {
                while (dStmt.Step() == BE_SQLITE_ROW) {
                    auto dClass = ctx.m_db.Schemas().GetClass(dStmt.GetValueId<ECClassId>(0));
                    if (dClass == nullptr)
                        continue;
                    PrimitiveECPropertyCP currentTsProp = nullptr;
                    if (SUCCESS == CoreCustomAttributeHelper::GetCurrentTimeStampProperty(currentTsProp, *dClass) &&
                        currentTsProp != nullptr && currentTsProp->GetName().EqualsIAscii(oldProperty->GetName()))
                        return Rejected("readOnly", "The property referenced by ClassHasCurrentTimeStampProperty must remain readOnly=true.");
                }
            }
        }
        if (ECObjectsStatus::Success != property->SetIsReadOnly(readOnly))
            return Rejected("readOnly", "Invalid readOnly setting.");
        if (create)
            row[2].second = int64_t(readOnly);
        else
            row.push_back({"IsReadonly", int64_t(readOnly)});
    }
    if (parser.Has("priority")) {
        uint32_t priority = 0;
        if (!parser.UInt32("priority", priority))
            return result;
        if (priority > uint32_t(std::numeric_limits<int32_t>::max()))
            return Invalid("priority", "Priority exceeds int32.");
        if (ECObjectsStatus::Success != property->SetPriority(static_cast<int32_t>(priority)))
            return Rejected("priority", "Invalid property priority.");
        row.push_back({"Priority", int64_t(priority)});
    }
    if (parser.Has("extendedType")) {
        Utf8String extended;
        if (!parser.String("extendedType", extended))
            return result;
        ECObjectsStatus status;
        if (auto primitive = property->GetAsPrimitivePropertyP())
            status = primitive->SetExtendedTypeName(extended.c_str());
        else if (auto array = property->GetAsPrimitiveArrayPropertyP())
            status = array->SetExtendedTypeName(extended.c_str());
        else
            return Invalid("extendedType", "Extended types require a primitive property.");
        if (status != ECObjectsStatus::Success)
            return Rejected("extendedType", "ECObjects rejected the extended type.");
        row.push_back({"ExtendedTypeName", extended.empty() ? SchemaMetaValue(nullptr) : SchemaMetaValue(extended)});
    }
    for (bool minimum : {true, false}) {
        auto key = minimum ? "minLength" : "maxLength";
        if (!parser.Has(key))
            continue;
        bool clear = parser.Value(key).isNull();
        uint32_t length = 0;
        if (!clear && !parser.UInt32(key, length))
            return result;
        if (ECObjectsStatus::Success != (minimum ? property->SetMinimumLength(length) : property->SetMaximumLength(length)))
            return Rejected(key, "ECObjects rejected the property length bound.");
        if (clear) {
            if (minimum)
                property->ResetMinimumLength();
            else
                property->ResetMaximumLength();
        }
        row.push_back({minimum ? "PrimitiveTypeMinLength" : "PrimitiveTypeMaxLength",
                       clear ? SchemaMetaValue(nullptr) : SchemaMetaValue(int64_t(length))});
    }
    if (property->IsMinimumLengthDefined() && property->IsMaximumLengthDefined() && property->GetMinimumLength() > property->GetMaximumLength())
        return Rejected("minLength", "minLength exceeds maxLength.");
    for (bool minimum : {true, false}) {
        result = ParseAndSetMinMaxValue(parser, minimum, *property, row);
        if (!result.IsOk())
            return result;
    }
    for (bool category : {true, false}) {
        auto key = category ? "category" : "koq";
        if (!parser.Has(key))
            continue;
        Utf8String qualified, schemaName, item;
        if (!parser.String(key, qualified))
            return result;
        if (category && qualified.empty()) {
            if (!create && oldProperty->GetCategory() != nullptr) {
                if (ECObjectsStatus::Success != property->SetCategory(nullptr))
                    return Rejected(key, "ECObjects rejected removing the property category.");
                row.push_back({"CategoryId", SchemaMetaValue(nullptr)});
            }
            continue;
        }
        if (!category && qualified.empty()) {
            if (!create && oldProperty->GetKindOfQuantity() != nullptr) {
                auto allowUnitCA = oldProperty->GetCustomAttribute("SchemaUpgradeCustomAttributes", "AllowUnitChange");
                bool allowed = false;
                if (allowUnitCA.IsValid()) {
                    ECValue fromVal, toVal;
                    allowUnitCA->GetValue(fromVal, "From");
                    allowUnitCA->GetValue(toVal, "To");
                    auto oldKoq = oldProperty->GetKindOfQuantity();
                    if (!fromVal.IsNull() && !toVal.IsNull() &&
                        oldKoq->GetPersistenceUnit()->GetQualifiedName(oldKoq->GetSchema()).EqualsIAscii(fromVal.ToString().c_str()) &&
                        toVal.ToString().Equals(""))
                        allowed = true;
                }
                if (!allowed)
                    return Rejected(key, "Removing a KindOfQuantity from an existing property is not supported without AllowUnitChange.");
                if (ECObjectsStatus::Success != property->SetKindOfQuantity(nullptr))
                    return Rejected(key, "ECObjects rejected removing the kind of quantity.");
                row.push_back({"KindOfQuantityId", SchemaMetaValue(nullptr)});
            }
            continue;
        }
        if (!QualifiedName(qualified, schemaName, item))
            return Invalid(key, "Expected a qualified schema item name.");
        if (persisted == nullptr) {
            // A new owner has no derived classes to check against the persisted item. Avoid reloading the
            // item through the schema reader, cleared after every class creation, when the working graph
            // already holds the referenced schema.
            ECSchemaId itemSchemaId;
            int64_t itemId = 0;
            ECSchemaCP referenced = nullptr;
            result = FindDirectlyReferencedItem(ctx, *working, category, schemaName, item, itemSchemaId, itemId, referenced);
            if (!result.IsOk())
                return result;
            if (referenced != nullptr) {
                auto status = category ? property->SetCategory(referenced->GetPropertyCategoryCP(item.c_str()))
                                       : property->SetKindOfQuantity(referenced->GetKindOfQuantityCP(item.c_str()));
                if (ECObjectsStatus::Success != status)
                    return Rejected(key, category ? "ECObjects rejected the property category." : "ECObjects rejected the kind of quantity.");
                references.push_back(itemSchemaId);
                row.push_back({category ? "CategoryId" : "KindOfQuantityId", itemId});
                continue;
            }
        }
        if (category) {
            auto value = ctx.m_db.Schemas().GetPropertyCategory(schemaName, item, SchemaLookupMode::AutoDetect);
            if (value == nullptr)
                return Missing(key, "Property category not found.");
            result = reference(value->GetSchema());
            if (!result.IsOk())
                return result;
            auto id = value->GetId();
            value = WorkingSchema(*working, value->GetSchema().GetName())->GetPropertyCategoryCP(value->GetName().c_str());
            if (ECObjectsStatus::Success != property->SetCategory(value))
                return Rejected(key, "ECObjects rejected the property category.");
            row.push_back({"CategoryId", int64_t(id.GetValue())});
        } else {
            auto value = ctx.m_db.Schemas().GetKindOfQuantity(schemaName, item, SchemaLookupMode::AutoDetect);
            if (value == nullptr)
                return Missing(key, "Kind of quantity not found.");
            persistedTargetKoq = value;
            if (!create && oldProperty->GetKindOfQuantity() != nullptr &&
                !oldProperty->GetKindOfQuantity()->GetPersistenceUnit()->GetFullName().EqualsIAscii(value->GetPersistenceUnit()->GetFullName())) {
                auto allowUnitCA = oldProperty->GetCustomAttribute("SchemaUpgradeCustomAttributes", "AllowUnitChange");
                bool allowed = false;
                if (allowUnitCA.IsValid()) {
                    ECValue fromVal, toVal;
                    allowUnitCA->GetValue(fromVal, "From");
                    allowUnitCA->GetValue(toVal, "To");
                    auto oldKoq = oldProperty->GetKindOfQuantity();
                    if (!fromVal.IsNull() && !toVal.IsNull() &&
                        oldKoq->GetPersistenceUnit()->GetQualifiedName(oldKoq->GetSchema()).EqualsIAscii(fromVal.ToString().c_str()) &&
                        value->GetPersistenceUnit()->GetQualifiedName(value->GetSchema()).EqualsIAscii(toVal.ToString().c_str()))
                        allowed = true;
                }
                if (!allowed)
                    return Rejected(key, "Changing KindOfQuantity persistence unit is not supported.");
            }
            result = reference(value->GetSchema());
            if (!result.IsOk())
                return result;
            auto id = value->GetId();
            value = WorkingSchema(*working, value->GetSchema().GetName())->GetKindOfQuantityCP(value->GetName().c_str());
            if (ECObjectsStatus::Success != property->SetKindOfQuantity(value))
                return Rejected(key, "ECObjects rejected the kind of quantity.");
            row.push_back({"KindOfQuantityId", int64_t(id.GetValue())});
        }
    }
    if (create) {
        // A class created by the same command has no derived classes yet.
        if (persisted != nullptr) {
            result = CheckAndCollectDerivedPropertyOverrides(ctx, *persisted, *property, persistedTargetEnum, persistedTargetKoq, derivedUpgrades);
            if (!result.IsOk())
                return result;
        }
    } else if (parser.Has("type") || parser.Has("enum") || parser.Has("koq")) {
        ECEnumerationCP effectiveEnum = persistedTargetEnum;
        if (!parser.Has("enum") && !parser.Has("type") && oldProperty->GetIsPrimitive())
            effectiveEnum = oldProperty->GetAsPrimitiveProperty()->GetEnumeration();
        KindOfQuantityCP effectiveKoq = parser.Has("koq") ? persistedTargetKoq : oldProperty->GetKindOfQuantity();
        result = CheckAndCollectDerivedPropertyOverrides(ctx, *persisted, *property, effectiveEnum, effectiveKoq, derivedUpgrades);
        if (!result.IsOk())
            return result;
    }
    result = Labels(parser, *property, row);
    if (!result.IsOk())
        return result;
    bvector<CustomAttribute> attributes;
    if (create && parser.Has("customAttributes")) {
        result = PrepareCustomAttributes(ctx, *working, *property, parser.Value("customAttributes"), attributes);
        if (!result.IsOk())
            return result;
    }
    if (create && (parser.Has("columnName") || parser.Has("isNullable") || parser.Has("isUnique") || parser.Has("collation"))) {
        if (!property->GetIsPrimitive())
            return Invalid("columnName", "PropertyMap options require a primitive scalar property.");
        BeJsDocument json;
        json.SetEmptyObject();
        for (auto pair : {std::make_pair("columnName", "ColumnName"), std::make_pair("collation", "Collation")})
            if (parser.Has(pair.first)) {
                Utf8String value;
                if (!parser.String(pair.first, value))
                    return result;
                if (Utf8String(pair.first).Equals("columnName") && !ECNameValidation::IsValidName(value.c_str()))
                    return Invalid(pair.first, "A column name must be an EC identifier.");
                if (Utf8String(pair.first).Equals("collation")) {
                    Utf8CP canonical = nullptr;
                    for (auto allowed : {"Binary", "NoCase", "RTrim"})
                        if (value.EqualsI(allowed))
                            canonical = allowed;
                    if (canonical == nullptr)
                        return Invalid(pair.first, "Expected Binary, NoCase, or RTrim.");
                    value = canonical;
                }
                json[pair.second] = value;
            }
        for (auto pair : {std::make_pair("isNullable", "IsNullable"), std::make_pair("isUnique", "IsUnique")})
            if (parser.Has(pair.first)) {
                bool value = false;
                if (!parser.Boolean(pair.first, value))
                    return result;
                if (Utf8String(pair.first).Equals("isNullable") && !value) {
                    if (ecClass->IsStructClass())
                        return Unsupported("isNullable", "Non-null struct members require checking every mapped struct user.");
                    result = persisted == nullptr ? Result() : PropertyInstances(ctx, *persisted, nullptr);
                    if (!result.IsOk())
                        return result;
                }
                json[pair.second] = value;
            }
        result = PrepareGeneratedCustomAttribute(ctx, *working, *property, "PropertyMap", json, attributes);
        if (!result.IsOk())
            return result;
    }
    if (create && parser.Has("fkOnDelete")) {
        if (!property->GetIsNavigation())
            return Invalid("fkOnDelete", "Foreign key options require a navigation property.");
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
        result = PrepareGeneratedCustomAttribute(ctx, *working, *property, "ForeignKeyConstraint", json, attributes);
        if (!result.IsOk())
            return result;
    }
    if (create) {
        auto propertyMap = property->GetCustomAttributeLocal("ECDbMap", "PropertyMap");
        if (propertyMap.IsValid()) {
            ECValue nullable;
            if (ECObjectsStatus::Success != propertyMap->GetValue(nullable, "IsNullable"))
                return Rejected("customAttributes", "Cannot read PropertyMap.IsNullable.");
            if (!nullable.IsNull() && !nullable.GetBoolean()) {
                if (ecClass->IsStructClass())
                    return Unsupported("isNullable", "Non-null struct members require checking every mapped struct user.");
                result = persisted == nullptr ? Result() : PropertyInstances(ctx, *persisted, nullptr);
                if (!result.IsOk())
                    return result;
            }
        }
    }
    if (!create && row.empty())
        return Invalid("property", "At least one property attribute must change.");
    auto propertyId = create ? ECPropertyId() : oldProperty->GetId();
    bool newOwner = persisted == nullptr;
    out.m_property = property;
    out.m_upgradesDerived = !derivedUpgrades.empty();
    out.m_apply = [create, newOwner, overridesInherited, inlineOrdinal, schemaId, propertyId, name, row, references, attributes, affectedRelationship, derivedUpgrades](SchemaEditContext& apply, ECClassId classId) mutable {
        for (auto referenceId : references) {
            auto result = apply.AddReference(schemaId, referenceId);
            if (!result.IsOk())
                return result;
        }
        // A class created by the same command already affected itself and has no users.
        if (!newOwner) {
            auto affected = AffectClassAndUsers(apply, classId);
            if (!affected.IsOk())
                return affected;
        }
        ECPropertyId id = propertyId;
        if (create) {
            int64_t ordinal = inlineOrdinal;
            if (!newOwner) {
                auto result = NextOrdinal(apply, "SELECT COALESCE(MAX(Ordinal)+1,0) FROM main.ec_Property WHERE ClassId=?", classId, ordinal);
                if (!result.IsOk())
                    return result;
            }
            id = ECPropertyId(apply.m_db.GetImpl().GetIdFactory().Property().NextId().GetValue());
            // A new local override also replaces inherited mapping identities in this class and its descendants.
            bool requiresRemap = overridesInherited || !derivedUpgrades.empty() || (!newOwner && [&]() {
                Statement s;
                return BE_SQLITE_OK == s.Prepare(apply.m_db,
                                                 "SELECT 1 FROM main.ec_cache_ClassHierarchy h JOIN main.ec_Property p ON p.ClassId=h.ClassId"
                                                 " WHERE h.BaseClassId=? AND h.ClassId<>? AND p.Name=? COLLATE NOCASE LIMIT 1") &&
                       BE_SQLITE_OK == s.BindId(1, classId) &&
                       BE_SQLITE_OK == s.BindId(2, classId) &&
                       BE_SQLITE_OK == s.BindText(3, name.c_str(), Statement::MakeCopy::No) &&
                       s.Step() == BE_SQLITE_ROW;
            }());
            if (requiresRemap) {
                auto classPtr = apply.m_db.Schemas().GetClass(classId);
                if (classPtr != nullptr)
                    apply.m_mapping.RemapManager().CollectRemapInfosFromNewProperty(*classPtr, name, id);
            }
            row[0].second = int64_t(classId.GetValue());
            row.push_back({"Ordinal", ordinal});
            row.push_back({"Id", int64_t(id.GetValue())});
        } else if (!derivedUpgrades.empty() || [&]() {
                       for (auto const& col : row)
                           if (col.first.Equals("PrimitiveType") || col.first.Equals("EnumerationId"))
                               return true;
                       return false;
                   }()) {
            auto classPtr = apply.m_db.Schemas().GetClass(classId);
            if (classPtr != nullptr)
                apply.m_mapping.RemapManager().CollectRemapInfosFromNewProperty(*classPtr, name, id);
        }
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
        auto written = create ? apply.m_writer.Insert("ec_Property", row) : apply.m_writer.Update("ec_Property", id, row);
        if (written.IsOk() && create)
            written = InsertCustomAttributes(apply, schemaId, id, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property, attributes);
        if (written.IsOk())
            Affected(apply, schemaId, classId);
        if (written.IsOk() && affectedRelationship.IsValid())
            apply.m_affectedClasses.insert(affectedRelationship);
        return written;
    };
    return {};
}


Result PrepareProperty(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, bool create) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    if (!(create ? parser.CheckKeys({"class", "name", "kind", "type", "enum", "struct", "relationship", "direction",
                                     "extendedType", "minOccurs", "maxOccurs", "readOnly", "label", "description", "priority", "koq", "category", "minLength", "maxLength", "minValue", "maxValue",
                                     "customAttributes", "columnName", "isNullable", "isUnique", "collation", "fkOnDelete"})
                 : parser.CheckKeys({"class", "property", "type", "enum", "minOccurs", "maxOccurs", "readOnly", "label", "description", "priority", "extendedType", "koq", "category", "minLength", "maxLength", "minValue", "maxValue"})))
        return result;
    Utf8String className, name, kind = "primitive", typeName, enumName, structName, relationshipName, direction;
    if (!parser.String("class", className, true) || !parser.String(create ? "name" : "property", name, true) ||
        !parser.String("kind", kind) || !parser.String("type", typeName) || !parser.String("enum", enumName) ||
        !parser.String("struct", structName) || !parser.String("relationship", relationshipName) || !parser.String("direction", direction))
        return result;
    if (!create)
        ctx.RefreshReader();
    auto persisted = FindClass(ctx, className);
    if (persisted != nullptr && ctx.m_staleReaderClass.IsValid() && persisted->GetId() != ctx.m_staleReaderClass) {
        ctx.RefreshReader();
        persisted = FindClass(ctx, className);
    }
    if (persisted == nullptr)
        return Missing("class", "Class not found.");
    auto oldProperty = persisted->GetPropertyP(name, false);
    if (create && oldProperty != nullptr)
        return Rejected("name", "The class already declares this property.");
    if (!create && oldProperty == nullptr)
        return Missing("property", "The property must be declared locally on the class.");
    result = CheckReservedPropertyNameInHierarchy(ctx, *persisted, name);
    if (!result.IsOk())
        return result;
    // Creations and alterations that leave the property type and enumeration unchanged cannot affect other
    // classes, so they can reuse and publish the working graph. A KoQ change only rejects incompatible
    // overrides: ECObjects checks the base and working-graph derived properties, and
    // CheckAndCollectDerivedPropertyOverrides checks persisted derived classes.
    bool localChange = create || !(parser.Has("type") || parser.Has("enum"));
    ECSchemaPtr working;
    // A reused graph already passed full validation after the command that published it.
    bool validatedGraph = false;
    if (localChange && ctx.m_propertyCreationSchema.IsValid() &&
        ctx.m_propertyCreationSchema->GetSchemaKey().Matches(persisted->GetSchema().GetSchemaKey(), SchemaMatchType::Exact)) {
        working = ctx.m_propertyCreationSchema;
        validatedGraph = true;
    } else
        result = Copy(ctx, working, persisted->GetSchema());
    if (!result.IsOk())
        return result;
    auto ecClass = working->GetClassP(persisted->GetName().c_str());
    if (ecClass == nullptr)
        return Rejected("class", "Class missing from working schema.");
    PreparedProperty definition;
    result = DefineProperty(ctx, args, create, persisted, *working, *ecClass, persisted->GetSchema().GetId(), -1, definition);
    if (!result.IsOk())
        return result;
    auto classId = persisted->GetId();
    // Derived type upgrades edit other classes in the graph, so they need full validation.
    if (validatedGraph && !definition.m_upgradesDerived) {
        result = ValidateWorkingGates(ctx, *working);
        if (result.IsOk())
            result = ctx.ValidateWorkingPropertyCreation(*working, *ecClass, classId, *definition.m_property);
    } else
        result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    // Adding property metadata leaves inheritance and persisted class-to-table mappings unchanged.
    // The group still refreshes both cache tables before physical mapping.
    out.m_requiresCacheTableRefresh = !create;
    if (localChange && ctx.m_session.m_inGroup && !definition.m_upgradesDerived) {
        out.m_propertyCreationSchema = working;
        out.m_propertyCreationClass = classId;
    }
    out.m_apply = [apply = std::move(definition.m_apply), classId](SchemaEditContext& ctx) { return apply(ctx, classId); };
    return {};
}

Result DropProperty(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    Utf8String className, name;
    if (!parser.CheckKeys({"class", "property"}) || !parser.String("class", className, true) || !parser.String("property", name, true))
        return result;
    auto ecClass = FindClass(ctx, className);
    if (ecClass == nullptr)
        return Missing("class", "Class not found.");
    auto property = ecClass->GetPropertyP(name, false);
    if (property == nullptr)
        return Missing("property", "The property is not declared locally.");
    result = Editable(ctx, ecClass->GetSchema());
    if (!result.IsOk())
        return result;
    if (property->GetBaseProperty() == nullptr) {
        result = ctx.RequireMajorUpgrade(ecClass->GetSchema());
        if (!result.IsOk())
            return result;
    }
    // Preserve override-owned values through shared remapping or an identical inherited column.
    PropertyMapRemoval removal;
    if (ecClass->IsStructClass()) {
        // Struct members are represented under their containing root property's
        // ID, so the direct-property override proof below does not apply.
        result = PrepareStructMemberRemoval(ctx, *property, removal);
        if (!result.IsOk())
            return result;
    } else if (property->GetBaseProperty() != nullptr) {
        auto root = property->GetBaseProperty();
        while (root->GetBaseProperty() != nullptr)
            root = root->GetBaseProperty();
        Statement owned;
        if (BE_SQLITE_OK != owned.Prepare(ctx.m_db,
                                          "SELECT pm.Id FROM main.ec_PropertyMap pm JOIN main.ec_PropertyPath pp ON pp.Id=pm.PropertyPathId"
                                          " JOIN main.ec_Column col ON col.Id=pm.ColumnId WHERE pp.RootPropertyId=?1 AND col.ColumnKind<>?2"
                                          " AND NOT EXISTS(SELECT 1 FROM main.ec_PropertyMap inherited JOIN main.ec_PropertyPath ip ON ip.Id=inherited.PropertyPathId"
                                          " WHERE ip.RootPropertyId=?3 AND ip.AccessString=pp.AccessString AND inherited.ColumnId=pm.ColumnId AND inherited.ClassId=?4) LIMIT 1") ||
            BE_SQLITE_OK != owned.BindId(1, property->GetId()) || BE_SQLITE_OK != owned.BindInt64(2, int64_t(DbColumn::Kind::SharedData)) ||
            BE_SQLITE_OK != owned.BindId(3, root->GetId()) || BE_SQLITE_OK != owned.BindId(4, root->GetClass().GetId()))
            return DatabaseError("Preparing override column preservation verification failed.");
        auto ownedStep = owned.Step();
        if (ownedStep == BE_SQLITE_ROW)
            return Rejected("property", "The override owns a non-shared column that is not retained by the inherited property.");
        if (ownedStep != BE_SQLITE_DONE)
            return DatabaseError("Verifying override column preservation failed.");
        Statement preserved;
        if (BE_SQLITE_OK != preserved.Prepare(ctx.m_db,
                                              "WITH RECURSIVE derived(Id) AS (SELECT ?1 UNION SELECT b.ClassId FROM main.ec_ClassHasBaseClasses b JOIN derived d ON b.BaseClassId=d.Id)"
                                              " SELECT m.ClassId FROM derived d JOIN main.ec_ClassMap m ON m.ClassId=d.Id WHERE m.MapStrategy<>?3"
                                              " AND NOT EXISTS(SELECT 1 FROM main.ec_PropertyMap pm JOIN main.ec_PropertyPath pp ON pp.Id=pm.PropertyPathId"
                                              " WHERE pm.ClassId=m.ClassId AND pp.RootPropertyId IN (?2,?4)) LIMIT 1") ||
            BE_SQLITE_OK != preserved.BindId(1, ecClass->GetId()) || BE_SQLITE_OK != preserved.BindId(2, root->GetId()) ||
            BE_SQLITE_OK != preserved.BindInt64(3, int64_t(MapStrategy::NotMapped)) ||
            BE_SQLITE_OK != preserved.BindId(4, property->GetId()))
            return DatabaseError("Preparing inherited property preservation verification failed.");
        auto step = preserved.Step();
        if (step == BE_SQLITE_ROW)
            return Rejected("property", "The override's mapped classes do not all retain an inherited root-property mapping.");
        if (step != BE_SQLITE_DONE)
            return DatabaseError("Verifying inherited property preservation failed.");
    } else {
        result = PropertyInstances(ctx, *ecClass, name.c_str());
        if (!result.IsOk())
            return result;
    }
    if (property->GetIsNavigation() && property->GetBaseProperty() == nullptr)
        return Unsupported("property", "The relationship's foreign-key ec_ClassMap and end-table partitions depend on this navigation mapping. RemapManager supports added properties/bases and deleted overrides, but has no foreign-key relationship unmapping collector.");
    ECSchemaPtr working;
    result = Copy(ctx, working, ecClass->GetSchema());
    if (!result.IsOk())
        return result;
    auto workingClass = working->GetClassP(ecClass->GetName().c_str());
    auto workingProperty = workingClass->GetPropertyP(name, false);
    if (workingProperty == nullptr || ECObjectsStatus::Success != workingClass->DeleteProperty(*workingProperty))
        return Rejected("property", "ECObjects rejected deleting the property.");
    result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    auto id = property->GetId();
    auto classId = ecClass->GetId();
    auto schemaId = ecClass->GetSchema().GetId();
    bool isOverride = property->GetBaseProperty() != nullptr;
    out.m_apply = [id, classId, schemaId, isOverride, removal](SchemaEditContext& apply) {
        auto result = AffectClassAndUsers(apply, classId);
        if (!result.IsOk())
            return result;
        if (isOverride)
            apply.m_mapping.RemapManager().CollectRemapInfosFromDeletedPropertyOverride(id);
        // True deletions contain only null values; surviving override paths use the remapper.
        result = removal.Apply(apply);
        if (!result.IsOk())
            return result;
        result = DeletePropertyCAs(apply, id);
        if (!result.IsOk())
            return result;
        result = apply.m_writer.Delete("ec_Property", id);
        if (result.IsOk())
            Affected(apply, schemaId, classId);
        return result;
    };
    return {};
}
}

namespace SchemaCommands {
Result PrepareInlineProperties(SchemaEditContext& ctx, BeJsConst properties, ECSchemaR working, ECClassR ecClass,
                               ECSchemaId schemaId, bvector<PreparedProperty>& out) {
    if (!properties.isArray())
        return Invalid("properties", "Expected an array of property definitions.");
    for (BeJsConst::ArrayIndex index = 0; index < properties.size(); ++index) {
        Utf8PrintfString path("properties[%u]", index);
        auto element = properties[index];
        if (!element.isObject())
            return Invalid(path.c_str(), "Expected a property definition object.");
        PreparedProperty prepared;
        auto result = DefineProperty(ctx, element, true, nullptr, working, ecClass, schemaId, int64_t(index), prepared);
        if (result.IsOk())
            result = Reserved(ecClass, prepared.m_property->GetName());
        if (!result.IsOk()) {
            // Report the failure at the element's argument path, keeping status, codes and messages.
            auto argPath = [&](Utf8StringCR arg) { return arg.empty() ? Utf8String(path) : Utf8String(path + "." + arg); };
            auto const& issues = result.GetIssues();
            auto prefixed = issues.empty() ? Result::Failure(result.GetStatus(), "", path.c_str(), "Invalid property definition.")
                                           : Result::Failure(result.GetStatus(), issues.front().code.c_str(), argPath(issues.front().arg).c_str(), issues.front().message);
            for (size_t i = 1; i < issues.size(); ++i)
                prefixed.AddIssue({issues[i].code, argPath(issues[i].arg), issues[i].message});
            prefixed.SetBlockingInstances(result.GetBlockingInstances());
            return prefixed;
        }
        out.push_back(std::move(prepared));
    }
    return {};
}

Result ApplyInlineProperties(SchemaEditContext& apply, ECClassId classId, bvector<PreparedProperty> const& properties) {
    for (auto const& property : properties) {
        auto result = property.m_apply(apply, classId);
        if (!result.IsOk())
            return result;
    }
    return {};
}

Result CheckReservedPropertyName(ECClassCR ecClass, Utf8StringCR name) {
    return Reserved(ecClass, name);
}

Result CheckReservedPropertyNameInHierarchy(SchemaEditContext& ctx, ECClassCR ecClass, Utf8StringCR name) {
    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != ctx.m_db.GetCachedStatement(stmt,
                                     "WITH RECURSIVE derived(Id) AS (SELECT ? UNION SELECT b.ClassId FROM main.ec_ClassHasBaseClasses b JOIN derived d ON b.BaseClassId=d.Id)"
                                     " SELECT Id FROM derived") ||
        BE_SQLITE_OK != stmt->BindId(1, ecClass.GetId()))
        return DatabaseError("Preparing inherited reserved-name validation failed.");
    DbResult step;
    while ((step = stmt->Step()) == BE_SQLITE_ROW) {
        auto derived = ctx.m_db.Schemas().GetClass(stmt->GetValueId<ECClassId>(0));
        if (derived == nullptr)
            return DatabaseError("Resolving a class for reserved-name validation failed.");
        auto result = Reserved(*derived, name);
        if (!result.IsOk())
            return result;
    }
    return step == BE_SQLITE_DONE ? Result() : DatabaseError("Reading inherited reserved-name policies failed.");
}
}

void RegisterSchemaPropertyCommands(SchemaCommandRegistry& registry) {
    Register(registry, "AddProperty", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return PrepareProperty(ctx, args, out, true); });
    Register(registry, "AlterProperty", [](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) { return PrepareProperty(ctx, args, out, false); });
    Register(registry, "DropProperty", DropProperty);
}
END_BENTLEY_SQLITE_EC_NAMESPACE
