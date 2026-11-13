/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <ECObjects/ECJsonUtilities.h>

using namespace IModelJsNative;

//=======================================================================================
// Helpers for the bulk ElementAspect operations (insertElementAspects, updateElementAspects, deleteElementAspects).
//=======================================================================================
namespace {

using FailureReason = BulkElementAspectFailureReason;

//=======================================================================================
// Collects the validation failures, keyed by the index of the row in the input array.
//=======================================================================================
struct BulkAspectFailures {
    struct Entry {
        FailureReason m_reason;
        ECInstanceId m_id;
    };
    std::map<uint32_t, Entry> m_failures;

    // Only the first failure of a row is recorded. The detail is diagnostic only, so it is logged rather than returned.
    void Fail(uint32_t index, FailureReason reason, Utf8StringCR detail = Utf8String(), ECInstanceId id = ECInstanceId()) {
        if (!m_failures.emplace(index, Entry{reason, id}).second)
            return;
        JsInterop::GetNativeLogger().debugv("Bulk ElementAspect operation: row %u is invalid (reason %d)%s%s", index, (int) reason,
            detail.empty() ? "" : ": ", detail.c_str());
    }
    bool IsEmpty() const { return m_failures.empty(); }

    // Builds { status, failures: [{ index, aspectId?, reason }] }, mirroring the result of deleteElements.
    Napi::Object ToJs(Napi::Env env) const {
        auto status = m_failures.empty() ? BulkElementAspectsStatus::Success : BulkElementAspectsStatus::ValidationFailed;
        auto result = Napi::Object::New(env);
        result.Set("status", Napi::Number::New(env, (int) status));
        auto failures = Napi::Array::New(env, m_failures.size());
        uint32_t i = 0;
        for (auto const& failure : m_failures) {
            auto obj = Napi::Object::New(env);
            obj.Set("index", Napi::Number::New(env, failure.first));
            if (failure.second.m_id.IsValid())
                obj.Set("aspectId", Napi::String::New(env, failure.second.m_id.ToHexStr().c_str()));
            obj.Set("reason", Napi::Number::New(env, (int) failure.second.m_reason));
            failures.Set(i++, obj);
        }
        result.Set("failures", failures);
        return result;
    }
};

/*---------------------------------------------------------------------------------**//**
* Throws for a failure after validation, once the JS pre-handlers may have run and aspects may have been written.
* The caller must abandon the current transaction.
+---------------+---------------+---------------+---------------+---------------+------*/
[[noreturn]] static void throwBulkAspectWriteError(Napi::Env env, Utf8CP functionName, DgnDbStatus status, Utf8StringCR detail) {
    Utf8PrintfString msg("%s: %s. The iModel may have been partially modified; abandon the changes.", functionName, detail.c_str());
    THROW_JS_DGN_DB_EXCEPTION(env, msg.c_str(), status);
}

/*---------------------------------------------------------------------------------**//**
* Throws for an unexpected SQLite error.
+---------------+---------------+---------------+---------------+---------------+------*/
[[noreturn]] static void throwBulkAspectSqlError(Napi::Env env, Utf8CP functionName, DbResult result, Utf8StringCR detail) {
    Utf8PrintfString msg("%s: %s: rc=%d, %s", functionName, detail.c_str(), (int) result, BeSQLiteLib::GetLogError(result).c_str());
    THROW_JS_BE_SQLITE_EXCEPTION(env, msg.c_str(), result);
}

/*---------------------------------------------------------------------------------**//**
* Parses an id string as strictly as Id64.isValidId64: "0x" followed by lowercase hexadecimal digits without leading zeros.
* BeInt64Id::FromString alone accepts a numeric prefix (e.g. "0x1234garbage") and wraps values that do not fit in 64 bits, so the
* parsed id must format back to the same string. Returns an invalid id otherwise.
+---------------+---------------+---------------+---------------+---------------+------*/
static BeInt64Id parseBulkAspectId(Utf8StringCR idString) {
    BeInt64Id id = BeInt64Id::FromString(idString.c_str());
    return (id.IsValid() && id.ToHexStr() == idString) ? id : BeInt64Id();
}

static BeInt64Id parseBulkAspectId(BeJsConst val) {
    return val.isString() ? parseBulkAspectId(val.asString()) : BeInt64Id();
}

//=======================================================================================
// Calls JS handler methods for aspects, resolving each JS class only once.
// If the JS class has the bulk method (e.g. "onBulkInsert") it is called once with
// { iModel, aspects: [...] }. Otherwise the per-aspect method (e.g. "onInsert") is called once per aspect.
// Exceptions thrown by the JS methods propagate to the caller.
//=======================================================================================
struct BulkAspectJsHooks {
    DgnDbR m_db;
    Napi::Env m_env;
    Napi::Object m_jsDb;
    Napi::Function m_getJsClass;
    std::map<DgnClassId, Napi::Object> m_jsClasses;

    BulkAspectJsHooks(DgnDbR db, Napi::Env env) : m_db(db), m_env(env) {
        auto jsDb = db.GetJsIModelDb();
        if (nullptr == jsDb)
            return; // we don't have an JavaScript IModelDb object for this DgnDb

        auto jsDgnDb = jsDb->Value();
        auto func = jsDgnDb.Get("getJsClass");
        if (!func.IsFunction())
            return; // must be a test

        m_jsDb = jsDgnDb;
        m_getJsClass = func.As<Napi::Function>();
    }

    bool IsAvailable() const { return !m_getJsClass.IsEmpty(); }

    Napi::Object GetJsClass(DgnClassId classId) {
        auto found = m_jsClasses.find(classId);
        if (m_jsClasses.end() != found)
            return found->second;

        auto ecClass = m_db.Schemas().GetClass(classId);
        if (nullptr == ecClass) {
            Utf8String err("ecClass not found for classId=");
            err += classId.ToHexStr();
            BeNapi::ThrowJsException(m_env, err.c_str(), (int) DgnDbStatus::BadArg);
        }

        auto classVal = m_getJsClass.Call(m_jsDb, {Napi::String::New(m_env, ecClass->GetFullName())});
        if (!classVal.IsObject())
            BeNapi::ThrowJsException(m_env, "getJsClass did not return an object", (int) DgnDbStatus::BadArg);

        auto classObj = classVal.As<Napi::Object>();
        m_jsClasses[classId] = classObj;
        return classObj;
    }

    // Calls the hooks for each class, in order of first appearance of the class in calls.
    void Call(bvector<std::pair<DgnClassId, Napi::Object>> const& calls, Utf8CP bulkMethod, Utf8CP singleMethod) {
        if (!IsAvailable() || calls.empty())
            return;

        bvector<DgnClassId> classOrder;
        std::map<DgnClassId, bvector<Napi::Object>> argsByClass;
        for (auto const& call : calls) {
            auto& args = argsByClass[call.first];
            if (args.empty())
                classOrder.push_back(call.first);
            args.push_back(call.second);
        }

        for (auto classId : classOrder) {
            auto const& args = argsByClass[classId];
            for (auto arg : args)
                arg.Set("iModel", m_jsDb);

            auto jsClass = GetJsClass(classId);
            auto bulkFunc = jsClass.Get(bulkMethod);
            if (bulkFunc.IsFunction()) {
                auto aspects = Napi::Array::New(m_env, args.size());
                for (uint32_t i = 0; i < (uint32_t) args.size(); ++i)
                    aspects.Set(i, args[i]);

                auto bulkArg = Napi::Object::New(m_env);
                bulkArg.Set("iModel", m_jsDb);
                bulkArg.Set("aspects", aspects);
                bulkFunc.As<Napi::Function>().Call(jsClass, {bulkArg});
                continue;
            }

            auto singleFunc = jsClass.Get(singleMethod);
            if (!singleFunc.IsFunction()) {
                Utf8String err("method not found: ");
                err += singleMethod;
                BeNapi::ThrowJsException(m_env, err.c_str(), (int) DgnDbStatus::BadArg);
            }

            for (auto arg : args)
                singleFunc.As<Napi::Function>().Call(jsClass, {arg});
        }
    }
};

//=======================================================================================
// The owning element and class of one row of insertElementAspects/updateElementAspects.
//=======================================================================================
struct BulkAspectPropsRow {
    uint32_t m_index = 0;
    Napi::Object m_props;
    Napi::Object m_hookArg;
    DgnElementCPtr m_owner;
    DgnClassId m_classId;
    ECClassCP m_class = nullptr;
    bool m_isMulti = false;
};

/*---------------------------------------------------------------------------------**//**
* Resolves the owning element and aspect class of an aspect props row. Records a failure and returns false if the row is invalid.
+---------------+---------------+---------------+---------------+---------------+------*/
static bool resolveBulkAspectPropsRow(BulkAspectPropsRow& row, DgnDbR db, Napi::Value propsVal, uint32_t index, std::map<DgnElementId, DgnElementCPtr>& owners, BulkAspectFailures& failures) {
    row.m_index = index;
    if (!propsVal.IsObject()) {
        failures.Fail(index, FailureReason::InvalidProps, "aspect props must be an object");
        return false;
    }

    row.m_props = propsVal.As<Napi::Object>();
    BeJsConst props(row.m_props);

    // The owner is given by element.id (RelatedElementProps). It is parsed strictly, so that e.g. "0x1234garbage" is not taken as 0x1234.
    auto elementProps = props["element"];
    DgnElementId ownerId((elementProps.isObject() ? parseBulkAspectId(elementProps[ECJsonUtilities::json_navId()]) : BeInt64Id()).GetValue());
    if (!ownerId.IsValid()) {
        failures.Fail(index, FailureReason::InvalidId, "invalid owning element id");
        return false;
    }

    auto owner = owners.find(ownerId);
    if (owners.end() == owner)
        owner = owners.emplace(ownerId, db.Elements().GetElement(ownerId)).first;

    if (!owner->second.IsValid()) {
        failures.Fail(index, FailureReason::ElementNotFound, ownerId.ToHexStr());
        return false;
    }
    row.m_owner = owner->second;

    auto classFullName = props[DgnElement::json_classFullName()];
    if (!classFullName.isString() || classFullName.asString().empty()) {
        failures.Fail(index, FailureReason::InvalidProps, "aspect props must have a classFullName");
        return false;
    }

    row.m_classId = ECJsonUtilities::GetClassIdFromClassNameJson(classFullName, db.GetClassLocater());
    if (!row.m_classId.IsValid()) {
        failures.Fail(index, FailureReason::ClassNotFound, classFullName.asString());
        return false;
    }

    row.m_class = db.Schemas().GetClass(row.m_classId);
    if (nullptr == row.m_class) {
        failures.Fail(index, FailureReason::ClassNotFound, classFullName.asString());
        return false;
    }

    row.m_isMulti = row.m_class->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);
    if (!row.m_isMulti && !row.m_class->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementUniqueAspect)) {
        failures.Fail(index, FailureReason::NotAnAspectClass, row.m_class->GetFullName());
        return false;
    }

    row.m_hookArg = Napi::Object::New(row.m_props.Env());
    row.m_hookArg.Set("props", row.m_props);
    row.m_hookArg.Set(DgnElement::json_model(), Napi::String::New(row.m_props.Env(), row.m_owner->GetModelId().ToHexStr()));
    return true;
}

/*---------------------------------------------------------------------------------**//**
* Converts the aspect JSON props onto an ECInstance. classFullName is skipped. JSON members are matched to properties without
* regard to case, so any spelling of "element" sets the Element navigation property that links the aspect to its owner. The owner is
* resolved from the canonical "element" member only, so other spellings are always skipped, and when excludeElement is true
* (updates, which cannot move an aspect to another owner) every spelling is skipped.
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus bulkAspectJsonToInstance(DgnDbR db, IECInstanceR instance, BulkAspectPropsRow const& row, bool excludeElement) {
    ECClassCP aspectClass = row.m_class;
    std::function<bool(Utf8CP)> shouldConvertProperty = [aspectClass, excludeElement](Utf8CP propName) {
        if (0 == strcmp(propName, DgnElement::json_classFullName()))
            return false;
        if (0 == BeStringUtilities::StricmpAscii(propName, "element") && (excludeElement || 0 != strcmp(propName, "element")))
            return false;
        return nullptr != aspectClass->GetPropertyP(propName);
    };
    return ECN::JsonECInstanceConverter::JsonToECInstance(instance, BeJsConst(row.m_props), db.GetClassLocater(), shouldConvertProperty);
}

/*---------------------------------------------------------------------------------**//**
* Binds ids to the IdSet(?) parameter at index. Returns false, with the bindings cleared, if an id could not be bound.
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T_Ids>
static bool bindBulkAspectIdSet(ECSqlStatement& stmt, int index, T_Ids const& ids) {
    auto& binder = stmt.GetBinder(index);
    for (auto const& id : ids) {
        if (ECSqlStatus::Success != binder.AddArrayElement().BindId(id)) {
            stmt.ClearBindings();
            return false;
        }
    }
    return true;
}

/*---------------------------------------------------------------------------------**//**
* Runs a query whose only parameter is an IdSet(?), calling onRow for each row. Throws on a SQLite error.
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T_Ids, typename T_OnRow>
static void queryBulkAspectIdSet(DgnDbR db, Napi::Env env, Utf8CP functionName, Utf8StringCR ecsql, T_Ids const& ids, T_OnRow onRow) {
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());
    if (!stmt.IsValid())
        throwBulkAspectSqlError(env, functionName, BE_SQLITE_ERROR, Utf8PrintfString("unable to prepare %s", ecsql.c_str()));
    if (!bindBulkAspectIdSet(*stmt, 1, ids))
        throwBulkAspectSqlError(env, functionName, BE_SQLITE_ERROR, Utf8PrintfString("unable to bind the ids of %s", ecsql.c_str()));

    DbResult stat;
    while (BE_SQLITE_ROW == (stat = stmt->Step()))
        onRow(*stmt);
    if (BE_SQLITE_DONE != stat)
        throwBulkAspectSqlError(env, functionName, stat, Utf8PrintfString("unable to step %s", ecsql.c_str()));
    stmt->Reset();
    stmt->ClearBindings();
}

//=======================================================================================
// A stored aspect: its id, actual class and owning element.
//=======================================================================================
struct BulkStoredAspect {
    ECInstanceId m_id;
    ECClassId m_classId;
    DgnElementId m_ownerId;
};

/*---------------------------------------------------------------------------------**//**
* Whether existing aspects of the class are loaded as generic aspects: the class has no registered C++ aspect handler and no
* ClassHasHandler custom attribute. DgnElement::GenericUniqueAspect::GetAspectP and DgnElement::GenericMultiAspect::GetAspectP
* return nullptr for aspects of other classes. The result is cached per class in cache.
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isGenericBulkAspectClass(DgnDbR db, ECClassCR aspectClass, std::map<ECClassId, bool>& cache) {
    auto found = cache.find(aspectClass.GetId());
    if (cache.end() != found)
        return found->second;

    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(db, DgnClassId(aspectClass.GetId()));
    bool isGeneric = ((nullptr == handler) || (handler == &dgn_AspectHandler::Aspect::GetHandler())) && aspectClass.GetCustomAttributeLocal("ClassHasHandler").IsNull();
    cache[aspectClass.GetId()] = isGeneric;
    return isGeneric;
}

/*---------------------------------------------------------------------------------**//**
* Groups row indices by owning element, preserving the order in which owners first appear.
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T_Row, typename T_GetOwnerId>
static bvector<std::pair<DgnElementId, bvector<size_t>>> groupBulkAspectRowsByOwner(bvector<T_Row> const& rows, T_GetOwnerId getOwnerId) {
    bvector<std::pair<DgnElementId, bvector<size_t>>> groups;
    std::map<DgnElementId, size_t> groupIndex;
    for (size_t i = 0; i < rows.size(); ++i) {
        DgnElementId ownerId = getOwnerId(rows[i]);
        auto found = groupIndex.find(ownerId);
        if (groupIndex.end() == found) {
            found = groupIndex.emplace(ownerId, groups.size()).first;
            groups.push_back(std::make_pair(ownerId, bvector<size_t>()));
        }
        groups[found->second].second.push_back(i);
    }
    return groups;
}

} // anonymous namespace

/*---------------------------------------------------------------------------------**//**
* Insert many ElementAspects. The operation is all-or-nothing, like deleteElements:
* 1. Every row is validated. If any row is invalid, nothing is written, no JS handlers are called, and ValidationFailed is returned
*    with one failure for each invalid row.
* 2. The JS pre-insert handlers are called for all rows. An exception aborts the call before anything is written.
* 3. The aspects are written grouped by owning element, with a single update of each owning element.
* 4. The JS post-insert handlers are called for all rows, in input order.
* An error while writing (e.g. the update of an owning element is rejected, or a SQLite error) throws at once: the remaining rows are
* not written and no post-insert handlers are called. Like an exception thrown by a JS handler during or after the writes, this leaves
* the rows already written in the Txn, so the caller must abandon the changes.
* Unique aspects are upserted: when the same (element, class) appears more than once the last row wins, and all of those rows get the
* id of the resulting aspect.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object JsInterop::InsertElementAspects(DgnDbR db, Napi::Array propsArray) {
    Utf8CP functionName = "insertElementAspects";
    Napi::Env env = propsArray.Env();
    uint32_t count = propsArray.Length();
    BulkAspectFailures failures;

    struct InsertRow : BulkAspectPropsRow {
        StandaloneECInstancePtr m_instance;
    };

    // Phase 1: validate every row
    bvector<InsertRow> rows;
    std::map<DgnElementId, DgnElementCPtr> owners;
    for (uint32_t i = 0; i < count; ++i) {
        InsertRow row;
        if (!resolveBulkAspectPropsRow(row, db, propsArray[i], i, owners, failures))
            continue;

        row.m_instance = row.m_class->GetDefaultStandaloneEnabler()->CreateInstance();
        if (!row.m_instance.IsValid() || SUCCESS != bulkAspectJsonToInstance(db, *row.m_instance, row, false)) {
            failures.Fail(i, FailureReason::InvalidProps, "invalid aspect properties");
            continue;
        }

        rows.push_back(row);
    }

    if (!failures.IsEmpty()) {
        auto result = failures.ToJs(env);
        result.Set("ids", Napi::Array::New(env, 0));
        return result;
    }

    // Phase 2: JS pre-insert handlers for all rows, before anything is written
    BulkAspectJsHooks hooks(db, env);
    bvector<std::pair<DgnClassId, Napi::Object>> hookCalls;
    for (auto const& row : rows)
        hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    hooks.Call(hookCalls, "onBulkInsert", "onInsert");

    // Phase 3: write, grouped by owning element. The aspects of an owner are added to one editable copy and written by a single Update.
    bvector<ECInstanceId> insertedIds(count);
    auto ownerGroups = groupBulkAspectRowsByOwner(rows, [](InsertRow const& row) { return row.m_owner->GetElementId(); });
    for (auto const& group : ownerGroups) {
        auto const& rowIndices = group.second;
        DgnElementPtr elementEdit = rows[rowIndices.front()].m_owner->CopyForEdit();
        if (!elementEdit.IsValid())
            throwBulkAspectWriteError(env, functionName, DgnDbStatus::WriteError, Utf8PrintfString("unable to edit element %s", group.first.ToHexStr().c_str()));

        // GenericUniqueAspect::SetAspect replaces an earlier aspect of the same class on the copy, so when the same (element, class)
        // appears more than once the last row wins, and all of those rows get the id of the resulting aspect.
        bvector<RefCountedPtr<DgnElement::Aspect>> aspects(rowIndices.size());
        std::map<DgnClassId, size_t> uniqueAspects;
        for (size_t i = 0; i < rowIndices.size(); ++i) {
            auto& row = rows[rowIndices[i]];
            DgnDbStatus stat = DgnDbStatus::Success;
            if (row.m_isMulti) {
                aspects[i] = DgnElement::GenericMultiAspect::AddAspect(*elementEdit, *row.m_instance, &stat);
            } else {
                aspects[i] = DgnElement::GenericUniqueAspect::SetAspect(*elementEdit, *row.m_instance, nullptr, &stat);
                uniqueAspects[row.m_classId] = i;
            }
            if (DgnDbStatus::Success != stat || !aspects[i].IsValid())
                throwBulkAspectWriteError(env, functionName, DgnDbStatus::Success != stat ? stat : DgnDbStatus::WriteError, Utf8PrintfString("unable to add the aspect at index %u", row.m_index));
        }

        DgnDbStatus stat = elementEdit->Update();
        if (DgnDbStatus::Success != stat)
            throwBulkAspectWriteError(env, functionName, stat, Utf8PrintfString("unable to update element %s", group.first.ToHexStr().c_str()));

        // The update of the owner succeeds even when one of its aspects cannot be written, so check each aspect.
        for (size_t i = 0; i < rowIndices.size(); ++i) {
            auto& row = rows[rowIndices[i]];
            auto const& aspect = aspects[row.m_isMulti ? i : uniqueAspects[row.m_classId]];
            DgnDbStatus writeStatus = aspect->GetLastWriteStatus();
            ECInstanceId aspectId = aspect->GetAspectInstanceId();
            if (DgnDbStatus::Success != writeStatus || !aspectId.IsValid())
                throwBulkAspectWriteError(env, functionName, DgnDbStatus::Success != writeStatus ? writeStatus : DgnDbStatus::WriteError,
                    Utf8PrintfString("unable to write the aspect at index %u", row.m_index));
            insertedIds[row.m_index] = aspectId;
        }
    }

    // Phase 4: JS post-insert handlers for all rows, in input order
    hookCalls.clear();
    for (auto const& row : rows)
        hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    hooks.Call(hookCalls, "onBulkInserted", "onInserted");

    auto result = failures.ToJs(env);
    auto ids = Napi::Array::New(env, count);
    for (uint32_t i = 0; i < count; ++i)
        ids.Set(i, Napi::String::New(env, insertedIds[i].ToHexStr().c_str()));
    result.Set("ids", ids);
    return result;
}

/*---------------------------------------------------------------------------------**//**
* Update many ElementAspects. Same phases and all-or-nothing behavior as InsertElementAspects. Properties not present in the props
* are left unchanged. A row may name an aspect through a base class of its actual class; only the properties of the class given in
* the props are applied. Aspects that are not found, that are not owned by the given element, or whose class has a handler (either the
* class given in the props or the actual class, so they cannot be modified as generic aspects) are validation failures. Rows that name
* the same stored aspect are applied in order and the aspect is written once. A unique aspect row whose class matches more than one
* unique aspect of the element (through a base class) is an AmbiguousAspect failure.
* Like updateElementAspect, the aspects are loaded after the pre-update handlers ran. An aspect that a handler deleted, or props that
* can no longer be applied (e.g. a handler modified them), throw.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object JsInterop::UpdateElementAspects(DgnDbR db, Napi::Array propsArray) {
    Utf8CP functionName = "updateElementAspects";
    Napi::Env env = propsArray.Env();
    uint32_t count = propsArray.Length();
    BulkAspectFailures failures;

    struct UpdateRow : BulkAspectPropsRow {
        ECInstanceId m_aspectId; // multi: given in the props
        ECInstanceId m_storedId; // the id of the stored aspect
        ECClassCP m_actualClass = nullptr; // the actual class of the stored aspect
    };

    // Phase 1: validate every row
    bvector<UpdateRow> candidates;
    bvector<ECInstanceId> multiIds;
    std::map<ECClassId, bvector<DgnElementId>> uniqueOwnersByClass;
    std::map<DgnElementId, DgnElementCPtr> owners;
    for (uint32_t i = 0; i < count; ++i) {
        UpdateRow row;
        if (!resolveBulkAspectPropsRow(row, db, propsArray[i], i, owners, failures))
            continue;

        if (row.m_isMulti) {
            row.m_aspectId = ECInstanceId(parseBulkAspectId(BeJsConst(row.m_props)[DgnElement::json_id()]).GetValue());
            if (!row.m_aspectId.IsValid()) {
                failures.Fail(i, FailureReason::InvalidId, "invalid aspect id");
                continue;
            }
            multiIds.push_back(row.m_aspectId);
        } else {
            uniqueOwnersByClass[row.m_class->GetId()].push_back(row.m_owner->GetElementId());
        }
        candidates.push_back(row);
    }

    // Find the stored aspects with their actual classes and owners: multi aspects by id with one query, and unique aspects by owner
    // with one query per class given in the props, which may be a base class of the aspect.
    std::map<ECInstanceId, BulkStoredAspect> multiAspects;
    if (!multiIds.empty()) {
        queryBulkAspectIdSet(db, env, functionName,
            "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect) " WHERE ECInstanceId IN (SELECT id FROM IdSet(?))",
            multiIds, [&](ECSqlStatement& stmt) {
                ECInstanceId id = stmt.GetValueId<ECInstanceId>(0);
                multiAspects[id] = BulkStoredAspect{id, stmt.GetValueId<ECClassId>(1), stmt.GetValueId<DgnElementId>(2)};
            });
    }

    std::map<std::pair<ECClassId, DgnElementId>, BulkStoredAspect> uniqueAspects;
    std::set<std::pair<ECClassId, DgnElementId>> ambiguousUniqueAspects;
    for (auto const& entry : uniqueOwnersByClass) {
        ECClassId keyClassId = entry.first;
        Utf8PrintfString ecsql("SELECT ECInstanceId,ECClassId,Element.Id FROM %s WHERE Element.Id IN (SELECT id FROM IdSet(?))",
            db.Schemas().GetClass(keyClassId)->GetECSqlName().c_str());
        queryBulkAspectIdSet(db, env, functionName, ecsql, entry.second, [&](ECSqlStatement& stmt) {
            DgnElementId ownerId = stmt.GetValueId<DgnElementId>(2);
            auto key = std::make_pair(keyClassId, ownerId);
            // An element can have several unique aspects whose classes derive from a base class given in the props
            if (!uniqueAspects.emplace(key, BulkStoredAspect{stmt.GetValueId<ECInstanceId>(0), stmt.GetValueId<ECClassId>(1), ownerId}).second)
                ambiguousUniqueAspects.insert(key);
        });
    }

    // Validate each row against its stored aspect
    std::map<ECClassId, bool> genericClasses;
    bvector<UpdateRow> rows;
    for (auto& row : candidates) {
        DgnElementId ownerId = row.m_owner->GetElementId();
        BulkStoredAspect const* stored = nullptr;
        if (row.m_isMulti) {
            auto found = multiAspects.find(row.m_aspectId);
            if (multiAspects.end() != found)
                stored = &found->second;
        } else {
            auto key = std::make_pair(row.m_class->GetId(), ownerId);
            if (ambiguousUniqueAspects.end() != ambiguousUniqueAspects.find(key)) {
                failures.Fail(row.m_index, FailureReason::AmbiguousAspect, Utf8PrintfString("element %s has more than one aspect of class %s or its subclasses", ownerId.ToHexStr().c_str(), row.m_class->GetFullName()));
                continue;
            }
            auto found = uniqueAspects.find(key);
            if (uniqueAspects.end() != found)
                stored = &found->second;
        }

        if (nullptr == stored) {
            failures.Fail(row.m_index, FailureReason::AspectNotFound, row.m_isMulti ? Utf8String() : Utf8String("the element has no aspect of this class"), row.m_aspectId);
            continue;
        }
        if (stored->m_ownerId != ownerId) {
            failures.Fail(row.m_index, FailureReason::AspectNotFound, Utf8PrintfString("the aspect is not owned by element %s", ownerId.ToHexStr().c_str()), row.m_aspectId);
            continue;
        }

        ECClassCP actualClass = db.Schemas().GetClass(stored->m_classId);
        if (nullptr == actualClass) {
            failures.Fail(row.m_index, FailureReason::ClassNotFound, Utf8PrintfString("class id %s", stored->m_classId.ToHexStr().c_str()), row.m_aspectId);
            continue;
        }
        if (!actualClass->Is(row.m_class)) {
            failures.Fail(row.m_index, FailureReason::AspectNotFound, Utf8PrintfString("the aspect is of class %s", actualClass->GetFullName()), row.m_aspectId);
            continue;
        }

        // Aspects of classes with a handler cannot be modified as generic aspects. The actual class may have a handler even when
        // the class given in the props does not.
        if (!isGenericBulkAspectClass(db, *row.m_class, genericClasses) || !isGenericBulkAspectClass(db, *actualClass, genericClasses)) {
            failures.Fail(row.m_index, FailureReason::AspectNotFound, Utf8PrintfString("aspects of class %s have a handler and cannot be updated as generic aspects", actualClass->GetFullName()), row.m_aspectId);
            continue;
        }

        // Validate the props on a scratch instance of the actual class, so that an invalid row cannot leave a partially modified aspect
        // behind. The actual class is concrete, even when the class given in the props is abstract.
        auto scratch = actualClass->GetDefaultStandaloneEnabler()->CreateInstance();
        if (!scratch.IsValid() || SUCCESS != bulkAspectJsonToInstance(db, *scratch, row, true)) {
            failures.Fail(row.m_index, FailureReason::InvalidProps, "invalid aspect properties", row.m_aspectId);
            continue;
        }

        row.m_storedId = stored->m_id;
        row.m_actualClass = actualClass;
        rows.push_back(row);
    }

    if (!failures.IsEmpty())
        return failures.ToJs(env);

    // Phase 2: JS pre-update handlers for all rows, before anything is written
    BulkAspectJsHooks hooks(db, env);
    bvector<std::pair<DgnClassId, Napi::Object>> hookCalls;
    for (auto const& row : rows)
        hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    hooks.Call(hookCalls, "onBulkUpdate", "onUpdate");

    // Phase 3: write, grouped by owning element. Like updateElementAspect, the aspects are loaded after the pre-update handlers, which
    // may have changed or deleted them. They are loaded through their actual class, so that every property is loaded. Rows that name
    // the same stored aspect get the same instance, so their props are applied in order and the aspect is written once.
    auto ownerGroups = groupBulkAspectRowsByOwner(rows, [](UpdateRow const& row) { return row.m_owner->GetElementId(); });
    for (auto const& group : ownerGroups) {
        auto const& rowIndices = group.second;
        DgnElementPtr elementEdit = rows[rowIndices.front()].m_owner->CopyForEdit();
        if (!elementEdit.IsValid())
            throwBulkAspectWriteError(env, functionName, DgnDbStatus::WriteError, Utf8PrintfString("unable to edit element %s", group.first.ToHexStr().c_str()));

        bvector<DgnElement::Aspect*> aspects(rowIndices.size(), nullptr);
        for (size_t i = 0; i < rowIndices.size(); ++i) {
            auto& row = rows[rowIndices[i]];
            IECInstanceP instance = nullptr;
            if (row.m_isMulti) {
                instance = DgnElement::GenericMultiAspect::GetAspectP(*elementEdit, *row.m_actualClass, row.m_storedId);
                if (nullptr != instance)
                    aspects[i] = DgnElement::MultiAspect::GetAspectP(*elementEdit, *row.m_actualClass, row.m_storedId);
            } else {
                instance = DgnElement::GenericUniqueAspect::GetAspectP(*elementEdit, *row.m_actualClass);
                if (nullptr != instance)
                    aspects[i] = DgnElement::UniqueAspect::GetAspectP(*elementEdit, *row.m_actualClass);
            }
            if (nullptr == instance || nullptr == aspects[i])
                throwBulkAspectWriteError(env, functionName, DgnDbStatus::NotFound, Utf8PrintfString("aspect %s can no longer be updated after the onUpdate handlers", row.m_storedId.ToHexStr().c_str()));

            if (SUCCESS != bulkAspectJsonToInstance(db, *instance, row, true))
                throwBulkAspectWriteError(env, functionName, DgnDbStatus::BadRequest, Utf8PrintfString("the props at index %u cannot be applied after the onUpdate handlers", row.m_index));
        }

        DgnDbStatus stat = elementEdit->Update();
        if (DgnDbStatus::Success != stat)
            throwBulkAspectWriteError(env, functionName, stat, Utf8PrintfString("unable to update element %s", group.first.ToHexStr().c_str()));

        // The update of the owner succeeds even when one of its aspects cannot be written, so check each aspect.
        for (size_t i = 0; i < rowIndices.size(); ++i) {
            DgnDbStatus writeStatus = aspects[i]->GetLastWriteStatus();
            if (DgnDbStatus::Success != writeStatus)
                throwBulkAspectWriteError(env, functionName, writeStatus, Utf8PrintfString("unable to write the aspect at index %u", rows[rowIndices[i]].m_index));
        }
    }

    // Phase 4: JS post-update handlers for all rows, in input order
    hookCalls.clear();
    for (auto const& row : rows)
        hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    hooks.Call(hookCalls, "onBulkUpdated", "onUpdated");

    return failures.ToJs(env);
}

/*---------------------------------------------------------------------------------**//**
* Delete many ElementAspects by id. Same phases and all-or-nothing behavior as InsertElementAspects. DgnElements::DeleteElementAspects
* resolves the ids with a single query, calls the JS pre-delete handlers through its onDelete callback once every aspect is valid, then
* updates each owning element once, which deletes its aspects with one statement per class. Duplicate ids are deleted once, and the JS
* handlers are called once for them. Like DgnElements::DeleteElements, ids that do not identify an aspect are ignored; malformed ids are
* validation failures. An aspect that can no longer be deleted after the pre-delete handlers ran (e.g. a handler deleted it or its
* owning element) throws.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object JsInterop::DeleteElementAspects(DgnDbR db, Napi::Array aspectIds) {
    Utf8CP functionName = "deleteElementAspects";
    Napi::Env env = aspectIds.Env();
    uint32_t count = aspectIds.Length();
    BulkAspectFailures failures;

    // Phase 1: parse the ids. DeleteElementAspects validates them.
    bvector<ECInstanceId> ids;
    std::map<ECInstanceId, bvector<uint32_t>> indicesById;
    for (uint32_t i = 0; i < count; ++i) {
        Napi::Value val = aspectIds[i];
        if (!val.IsString()) {
            failures.Fail(i, FailureReason::InvalidId, "aspect id must be a string");
            continue;
        }

        Utf8String idString = val.As<Napi::String>().Utf8Value();
        ECInstanceId aspectId(parseBulkAspectId(idString).GetValue());
        if (!aspectId.IsValid()) {
            failures.Fail(i, FailureReason::InvalidId, idString);
            continue;
        }

        auto& indices = indicesById[aspectId];
        if (indices.empty())
            ids.push_back(aspectId);
        indices.push_back(i);
    }

    if (!failures.IsEmpty())
        return failures.ToJs(env);

    // Phases 2 and 3: JS pre-delete handlers for all aspects, called by DeleteElementAspects before anything is written; then the
    // update of each owning element, which deletes its aspects.
    BulkAspectJsHooks hooks(db, env);
    bvector<std::pair<DgnClassId, Napi::Object>> hookCalls;
    auto onDelete = [&](bvector<BulkDeleteElementAspect> const& aspects) {
        for (auto const& aspect : aspects) {
            auto hookArg = Napi::Object::New(env);
            hookArg.Set("aspectId", Napi::String::New(env, aspect.aspectId.ToHexStr().c_str()));
            hookArg.Set(DgnElement::json_model(), Napi::String::New(env, aspect.modelId.ToHexStr().c_str()));
            hookArg.Set("element", Napi::String::New(env, aspect.elementId.ToHexStr().c_str()));
            hookCalls.push_back(std::make_pair(aspect.classId, hookArg));
        }
        hooks.Call(hookCalls, "onBulkDelete", "onDelete");
    };

    BulkDeleteElementAspectsResult result = db.Elements().DeleteElementAspects(ids, onDelete);
    if (BulkElementAspectsStatus::ValidationFailed == result.status) {
        for (auto const& failure : result.failures) {
            auto found = indicesById.find(failure.aspectId);
            if (indicesById.end() == found)
                continue;
            for (auto index : found->second)
                failures.Fail(index, failure.reason, Utf8String(), failure.aspectId);
        }
        if (failures.IsEmpty())
            throwBulkAspectWriteError(env, functionName, DgnDbStatus::BadArg, "unable to validate the aspects");
        return failures.ToJs(env);
    }

    if (BulkElementAspectsStatus::Success != result.status) {
        if (BE_SQLITE_OK != result.sqlStatus)
            throwBulkAspectSqlError(env, functionName, result.sqlStatus, "unable to delete the aspects; abandon the changes");
        if (DgnDbStatus::NotFound == result.writeStatus)
            throwBulkAspectWriteError(env, functionName, DgnDbStatus::NotFound, "an aspect can no longer be deleted after the onDelete handlers, because it or its owning element no longer exists");
        throwBulkAspectWriteError(env, functionName, DgnDbStatus::Success != result.writeStatus ? result.writeStatus : DgnDbStatus::WriteError, "unable to delete the aspects");
    }

    // Phase 4: JS post-delete handlers for all aspects, in input order
    hooks.Call(hookCalls, "onBulkDeleted", "onDeleted");

    return failures.ToJs(env);
}