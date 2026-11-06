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

    // Builds { status, failures: [{ index, id?, reason }] }, mirroring the result of deleteElements.
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
                obj.Set("id", Napi::String::New(env, failure.second.m_id.ToHexStr().c_str()));
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

    DgnElement::RelatedElement relatedElement;
    relatedElement.FromJson(db, props["element"]);
    if (!relatedElement.IsValid()) {
        failures.Fail(index, FailureReason::InvalidId, "invalid owning element id");
        return false;
    }

    auto owner = owners.find(relatedElement.m_id);
    if (owners.end() == owner)
        owner = owners.emplace(relatedElement.m_id, db.Elements().GetElement(relatedElement.m_id)).first;

    if (!owner->second.IsValid()) {
        failures.Fail(index, FailureReason::ElementNotFound, relatedElement.m_id.ToHexStr());
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

//=======================================================================================
// An existing aspect to update. Rows of updateElementAspects that name the same aspect (multi: class and id; unique: owner and
// class) share one, so their props are applied in order and the aspect is written once.
//=======================================================================================
struct BulkAspectLoad {
    ECClassCP m_class = nullptr; // the class given in the props
    bool m_isMulti = false;
    DgnElementId m_ownerId;
    ECInstanceId m_aspectId; // multi: given in the props; unique: set when loaded
    IECInstancePtr m_instance; // set when loaded
};

/*---------------------------------------------------------------------------------**//**
* Loads the aspects with one query per class, getting the same instances as GenericMultiAspect::_LoadProperties and
* GenericUniqueAspect::_LoadProperties. An aspect that is not found is left without an instance. Throws on a SQLite error.
+---------------+---------------+---------------+---------------+---------------+------*/
static void loadBulkAspectsForUpdate(DgnDbR db, Napi::Env env, bvector<BulkAspectLoad>& loads) {
    std::map<ECClassId, bvector<size_t>> multiByClass, uniqueByClass;
    for (size_t i = 0; i < loads.size(); ++i)
        (loads[i].m_isMulti ? multiByClass : uniqueByClass)[loads[i].m_class->GetId()].push_back(i);

    // Selects the aspects of a class by id and sets their instances. The class may be a base class of the aspects (polymorphic select).
    auto selectInstances = [&](ECClassCR cls, std::map<ECInstanceId, size_t> const& loadById) {
        bvector<ECInstanceId> ids;
        for (auto const& entry : loadById)
            ids.push_back(entry.first);

        Utf8PrintfString ecsql("SELECT * FROM %s WHERE ECInstanceId IN (SELECT id FROM IdSet(?))", cls.GetECSqlName().c_str());
        CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());
        if (!stmt.IsValid())
            throwBulkAspectSqlError(env, "updateElementAspects", BE_SQLITE_ERROR, Utf8PrintfString("unable to prepare the select of %s", cls.GetFullName()));
        if (!bindBulkAspectIdSet(*stmt, 1, ids))
            throwBulkAspectSqlError(env, "updateElementAspects", BE_SQLITE_ERROR, "unable to bind the aspect ids");

        ECInstanceECSqlSelectAdapter adapter(*stmt);
        DbResult stat;
        while (BE_SQLITE_ROW == (stat = stmt->Step())) {
            ECInstanceId id;
            adapter.GetInstanceId(id);
            auto found = loadById.find(id);
            if (loadById.end() != found)
                loads[found->second].m_instance = adapter.GetInstance();
        }
        if (BE_SQLITE_DONE != stat)
            throwBulkAspectSqlError(env, "updateElementAspects", stat, Utf8PrintfString("unable to select the aspects of %s", cls.GetFullName()));
        stmt->Reset();
        stmt->ClearBindings();
    };

    // Multi aspects: SELECT * FROM [class] WHERE ECInstanceId=?
    for (auto const& entry : multiByClass) {
        std::map<ECInstanceId, size_t> loadById;
        for (auto i : entry.second)
            loadById[loads[i].m_aspectId] = i;
        selectInstances(*loads[entry.second.front()].m_class, loadById);
    }

    // Unique aspects: find the actual class of the owner's aspect of the class (or of a subclass), then SELECT * FROM [actual class]
    for (auto const& entry : uniqueByClass) {
        ECClassCR keyClass = *loads[entry.second.front()].m_class;
        std::map<DgnElementId, size_t> loadByOwner;
        for (auto i : entry.second)
            loadByOwner[loads[i].m_ownerId] = i;

        bvector<DgnElementId> ownerIds;
        for (auto const& owner : loadByOwner)
            ownerIds.push_back(owner.first);

        Utf8PrintfString ecsql("SELECT ECInstanceId,ECClassId,Element.Id FROM %s WHERE Element.Id IN (SELECT id FROM IdSet(?))", keyClass.GetECSqlName().c_str());
        CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());
        if (!stmt.IsValid())
            throwBulkAspectSqlError(env, "updateElementAspects", BE_SQLITE_ERROR, Utf8PrintfString("unable to prepare the select of %s", keyClass.GetFullName()));
        if (!bindBulkAspectIdSet(*stmt, 1, ownerIds))
            throwBulkAspectSqlError(env, "updateElementAspects", BE_SQLITE_ERROR, "unable to bind the owning element ids");

        std::map<ECClassId, std::map<ECInstanceId, size_t>> loadByActualClass;
        DbResult stat;
        while (BE_SQLITE_ROW == (stat = stmt->Step())) {
            auto found = loadByOwner.find(stmt->GetValueId<DgnElementId>(2));
            if (loadByOwner.end() == found || loads[found->second].m_aspectId.IsValid())
                continue;
            auto& load = loads[found->second];
            load.m_aspectId = stmt->GetValueId<ECInstanceId>(0);
            loadByActualClass[stmt->GetValueId<ECClassId>(1)][load.m_aspectId] = found->second;
        }
        if (BE_SQLITE_DONE != stat)
            throwBulkAspectSqlError(env, "updateElementAspects", stat, Utf8PrintfString("unable to select the aspects of %s", keyClass.GetFullName()));
        stmt->Reset();
        stmt->ClearBindings();

        for (auto const& actual : loadByActualClass) {
            ECClassCP actualClass = db.Schemas().GetClass(actual.first);
            if (nullptr == actualClass)
                throwBulkAspectWriteError(env, "updateElementAspects", DgnDbStatus::BadSchema, Utf8PrintfString("aspect class %s not found", actual.first.ToHexStr().c_str()));
            selectInstances(*actualClass, actual.second);
        }
    }
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

    // Phase 3: write, grouped by owning element. The aspects of an owner are written by a single Update of one editable copy, the same
    // way as GenericMultiAspect::AddAspect and GenericUniqueAspect::SetAspect would write them.
    BulkElementAspectWriter writer(db);
    bvector<ECInstanceId> insertedIds(count);
    auto ownerGroups = groupBulkAspectRowsByOwner(rows, [](InsertRow const& row) { return row.m_owner->GetElementId(); });
    for (auto const& group : ownerGroups) {
        auto const& rowIndices = group.second;
        DgnElementPtr elementEdit = rows[rowIndices.front()].m_owner->CopyForEdit();
        if (!elementEdit.IsValid())
            throwBulkAspectWriteError(env, functionName, DgnDbStatus::WriteError, Utf8PrintfString("unable to edit element %s", group.first.ToHexStr().c_str()));

        bvector<BulkElementAspectWrite> writes;
        bvector<size_t> writeIndices;
        std::map<DgnClassId, size_t> uniqueWrites;
        for (auto r : rowIndices) {
            auto& row = rows[r];
            if (!row.m_isMulti) {
                auto found = uniqueWrites.find(row.m_classId);
                if (uniqueWrites.end() != found) {
                    writes[found->second].instance = row.m_instance;
                    writeIndices.push_back(found->second);
                    continue;
                }
                uniqueWrites[row.m_classId] = writes.size();
            }
            writeIndices.push_back(writes.size());
            writes.push_back(BulkElementAspectWrite());
            writes.back().instance = row.m_instance;
        }

        DgnDbStatus stat = writer.UpdateElement(*elementEdit, writes);
        if (DgnDbStatus::Success != stat)
            throwBulkAspectWriteError(env, functionName, stat, Utf8PrintfString("unable to update element %s", group.first.ToHexStr().c_str()));

        // The update of the owner succeeds even when one of its aspects cannot be written, so check each aspect.
        for (size_t i = 0; i < rowIndices.size(); ++i) {
            auto& row = rows[rowIndices[i]];
            auto const& write = writes[writeIndices[i]];
            if (DgnDbStatus::Success != write.status || !write.aspectId.IsValid())
                throwBulkAspectWriteError(env, functionName, DgnDbStatus::Success != write.status ? write.status : DgnDbStatus::WriteError,
                    Utf8PrintfString("unable to write the aspect at index %u", row.m_index));
            insertedIds[row.m_index] = write.aspectId;
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
* are left unchanged. Aspects that are not found, that are not owned by the given element, or whose class has a handler (so they
* cannot be modified as generic aspects) are validation failures. Rows that name the same aspect are applied in order and the aspect
* is written once. Props that cannot be applied after the pre-update handlers ran (e.g. a handler modified them) throw.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object JsInterop::UpdateElementAspects(DgnDbR db, Napi::Array propsArray) {
    Utf8CP functionName = "updateElementAspects";
    Napi::Env env = propsArray.Env();
    uint32_t count = propsArray.Length();
    BulkAspectFailures failures;

    struct UpdateRow : BulkAspectPropsRow {
        ECInstanceId m_aspectId;
        size_t m_loadIndex = 0;
    };

    // Phase 1: validate every row
    bvector<UpdateRow> candidates;
    bvector<ECInstanceId> multiIds;
    std::map<DgnElementId, DgnElementCPtr> owners;
    for (uint32_t i = 0; i < count; ++i) {
        UpdateRow row;
        if (!resolveBulkAspectPropsRow(row, db, propsArray[i], i, owners, failures))
            continue;

        if (row.m_isMulti) {
            row.m_aspectId = BeJsConst(row.m_props)[DgnElement::json_id()].GetId64<ECInstanceId>();
            if (!row.m_aspectId.IsValid()) {
                failures.Fail(i, FailureReason::InvalidId, "invalid aspect id");
                continue;
            }
            multiIds.push_back(row.m_aspectId);
        }

        // Validate the props on a scratch instance so that an invalid row cannot leave a partially modified aspect behind.
        auto scratch = row.m_class->GetDefaultStandaloneEnabler()->CreateInstance();
        if (!scratch.IsValid() || SUCCESS != bulkAspectJsonToInstance(db, *scratch, row, true)) {
            failures.Fail(i, FailureReason::InvalidProps, "invalid aspect properties", row.m_aspectId);
            continue;
        }

        candidates.push_back(row);
    }

    // Multi aspects are loaded by id alone, so look up their owners and only update those owned by the given element.
    std::map<ECInstanceId, DgnElementId> multiOwners;
    if (!multiIds.empty()) {
        CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(
            "SELECT ECInstanceId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect) " WHERE ECInstanceId IN (SELECT id FROM IdSet(?))");
        if (!statement.IsValid())
            throwBulkAspectSqlError(env, functionName, BE_SQLITE_ERROR, "unable to prepare the aspect lookup");
        if (!bindBulkAspectIdSet(*statement, 1, multiIds))
            throwBulkAspectSqlError(env, functionName, BE_SQLITE_ERROR, "unable to bind the aspect ids");

        DbResult stat;
        while (BE_SQLITE_ROW == (stat = statement->Step()))
            multiOwners[statement->GetValueId<ECInstanceId>(0)] = statement->GetValueId<DgnElementId>(1);
        if (BE_SQLITE_DONE != stat)
            throwBulkAspectSqlError(env, functionName, stat, "unable to look up the aspects");
        statement->Reset();
        statement->ClearBindings();
    }

    // Find the aspects to update. Rows that name the same aspect (multi: class and id; unique: owner and class) share one load.
    BulkElementAspectWriter writer(db);
    bvector<UpdateRow> loadRows;
    bvector<BulkAspectLoad> loaded;
    std::map<std::tuple<bool, uint64_t, uint64_t>, size_t> loadIndices;
    for (auto& row : candidates) {
        DgnElementId ownerId = row.m_owner->GetElementId();
        if (row.m_isMulti) {
            auto found = multiOwners.find(row.m_aspectId);
            if (multiOwners.end() == found) {
                failures.Fail(row.m_index, FailureReason::AspectNotFound, Utf8String(), row.m_aspectId);
                continue;
            }
            if (found->second != ownerId) {
                failures.Fail(row.m_index, FailureReason::AspectNotFound, Utf8PrintfString("the aspect is not owned by element %s", ownerId.ToHexStr().c_str()), row.m_aspectId);
                continue;
            }
        }

        // Aspects of classes with a handler cannot be modified as generic aspects
        if (!writer.IsGenericAspectClass(*row.m_class)) {
            failures.Fail(row.m_index, FailureReason::AspectNotFound, Utf8PrintfString("aspects of class %s have a handler and cannot be updated as generic aspects", row.m_class->GetFullName()), row.m_aspectId);
            continue;
        }

        size_t loadIndex = loaded.size();
        auto loadKey = row.m_isMulti ?
            std::make_tuple(true, row.m_class->GetId().GetValue(), row.m_aspectId.GetValue()) :
            std::make_tuple(false, ownerId.GetValue(), row.m_class->GetId().GetValue());
        auto inserted = loadIndices.emplace(loadKey, loadIndex);
        if (inserted.second) {
            BulkAspectLoad aspect;
            aspect.m_class = row.m_class;
            aspect.m_isMulti = row.m_isMulti;
            aspect.m_ownerId = ownerId;
            aspect.m_aspectId = row.m_aspectId;
            loaded.push_back(aspect);
        }
        row.m_loadIndex = inserted.first->second;
        loadRows.push_back(row);
    }

    loadBulkAspectsForUpdate(db, env, loaded);

    bvector<UpdateRow> rows;
    for (auto& row : loadRows) {
        if (!loaded[row.m_loadIndex].m_instance.IsValid()) {
            failures.Fail(row.m_index, FailureReason::AspectNotFound, row.m_isMulti ? Utf8String() : Utf8String("the element has no aspect of this class"), row.m_aspectId);
            continue;
        }
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

    // Phase 3: write, grouped by owning element
    auto ownerGroups = groupBulkAspectRowsByOwner(rows, [](UpdateRow const& row) { return row.m_owner->GetElementId(); });
    for (auto const& group : ownerGroups) {
        auto const& rowIndices = group.second;
        DgnElementPtr elementEdit = rows[rowIndices.front()].m_owner->CopyForEdit();
        if (!elementEdit.IsValid())
            throwBulkAspectWriteError(env, functionName, DgnDbStatus::WriteError, Utf8PrintfString("unable to edit element %s", group.first.ToHexStr().c_str()));

        bvector<BulkElementAspectWrite> writes;
        std::map<size_t, size_t> writeByLoad;
        for (auto r : rowIndices) {
            auto& row = rows[r];
            auto& load = loaded[row.m_loadIndex];
            if (SUCCESS != bulkAspectJsonToInstance(db, *load.m_instance, row, true))
                throwBulkAspectWriteError(env, functionName, DgnDbStatus::BadRequest, Utf8PrintfString("the props at index %u cannot be applied after the onUpdate handlers", row.m_index));

            if (writeByLoad.emplace(row.m_loadIndex, writes.size()).second) {
                writes.push_back(BulkElementAspectWrite());
                writes.back().instance = load.m_instance;
                writes.back().keyClass = load.m_isMulti ? nullptr : load.m_class;
                writes.back().aspectId = load.m_aspectId;
            }
        }

        DgnDbStatus stat = writer.UpdateElement(*elementEdit, writes);
        if (DgnDbStatus::Success != stat)
            throwBulkAspectWriteError(env, functionName, stat, Utf8PrintfString("unable to update element %s", group.first.ToHexStr().c_str()));

        // The update of the owner succeeds even when one of its aspects cannot be written, so check each aspect.
        for (auto r : rowIndices) {
            DgnDbStatus writeStatus = writes[writeByLoad[rows[r].m_loadIndex]].status;
            if (DgnDbStatus::Success != writeStatus)
                throwBulkAspectWriteError(env, functionName, writeStatus, Utf8PrintfString("unable to write the aspect at index %u", rows[r].m_index));
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
* Delete many ElementAspects by id. Same phases and all-or-nothing behavior as InsertElementAspects. Ids are resolved with a single
* query; each owning element is updated once and the aspects of each class are deleted with a single statement
* (DgnElements::DeleteElementAspects). Duplicate ids are deleted once, and the JS handlers are called once for them.
* Malformed ids, and aspects that are not found, are validation failures. An aspect that can no longer be deleted after the
* pre-delete handlers ran (e.g. a handler deleted it) throws.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object JsInterop::DeleteElementAspects(DgnDbR db, Napi::Array aspectIds) {
    Utf8CP functionName = "deleteElementAspects";
    Napi::Env env = aspectIds.Env();
    uint32_t count = aspectIds.Length();
    BulkAspectFailures failures;

    struct DeleteRow {
        ECInstanceId m_id;
        bvector<uint32_t> m_indices;
        DgnClassId m_classId;
        DgnElementId m_ownerId;
        DgnElementCPtr m_owner;
        Napi::Object m_hookArg;
        bool m_found = false;
    };

    // Phase 1: parse, resolve and validate the ids
    bvector<DeleteRow> rows;
    std::map<ECInstanceId, size_t> rowById;
    for (uint32_t i = 0; i < count; ++i) {
        Napi::Value val = aspectIds[i];
        if (!val.IsString()) {
            failures.Fail(i, FailureReason::InvalidId, "aspect id must be a string");
            continue;
        }

        Utf8String idString = val.As<Napi::String>().Utf8Value();
        ECInstanceId aspectId(BeInt64Id::FromString(idString.c_str()).GetValue());
        if (!aspectId.IsValid()) {
            failures.Fail(i, FailureReason::InvalidId, idString);
            continue;
        }

        auto found = rowById.find(aspectId);
        if (rowById.end() == found) {
            found = rowById.emplace(aspectId, rows.size()).first;
            rows.push_back(DeleteRow());
            rows.back().m_id = aspectId;
        }
        rows[found->second].m_indices.push_back(i);
    }

    auto failRow = [&](DeleteRow const& row, FailureReason reason, Utf8StringCR detail) {
        for (auto index : row.m_indices)
            failures.Fail(index, reason, detail, row.m_id);
    };

    if (!rows.empty()) {
        CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(
            "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementUniqueAspect) " WHERE ECInstanceId IN (SELECT id FROM IdSet(?)) UNION ALL "
            "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect)  " WHERE ECInstanceId IN (SELECT id FROM IdSet(?))");
        if (!statement.IsValid())
            throwBulkAspectSqlError(env, functionName, BE_SQLITE_ERROR, "unable to prepare the aspect lookup");

        bvector<ECInstanceId> ids;
        for (auto const& row : rows)
            ids.push_back(row.m_id);
        if (!bindBulkAspectIdSet(*statement, 1, ids) || !bindBulkAspectIdSet(*statement, 2, ids))
            throwBulkAspectSqlError(env, functionName, BE_SQLITE_ERROR, "unable to bind the aspect ids");

        DbResult stat;
        while (BE_SQLITE_ROW == (stat = statement->Step())) {
            auto found = rowById.find(statement->GetValueId<ECInstanceId>(0));
            if (rowById.end() == found)
                continue;

            auto& row = rows[found->second];
            row.m_found = true;
            row.m_classId = statement->GetValueId<DgnClassId>(1);
            row.m_ownerId = statement->GetValueId<DgnElementId>(2);
        }
        if (BE_SQLITE_DONE != stat)
            throwBulkAspectSqlError(env, functionName, stat, "unable to look up the aspects");
        statement->Reset();
        statement->ClearBindings();
    }

    std::map<DgnElementId, DgnElementCPtr> owners;
    for (auto& row : rows) {
        if (!row.m_found) {
            failRow(row, FailureReason::AspectNotFound, Utf8String());
            continue;
        }

        if (nullptr == db.Schemas().GetClass(row.m_classId)) {
            failRow(row, FailureReason::ClassNotFound, Utf8PrintfString("class id %s", row.m_classId.ToHexStr().c_str()));
            continue;
        }

        auto owner = owners.find(row.m_ownerId);
        if (owners.end() == owner)
            owner = owners.emplace(row.m_ownerId, db.Elements().GetElement(row.m_ownerId)).first;
        if (!owner->second.IsValid()) {
            failRow(row, FailureReason::ElementNotFound, row.m_ownerId.ToHexStr());
            continue;
        }
        row.m_owner = owner->second;

        row.m_hookArg = Napi::Object::New(env);
        row.m_hookArg.Set("aspectId", Napi::String::New(env, row.m_id.ToHexStr().c_str()));
        row.m_hookArg.Set(DgnElement::json_model(), Napi::String::New(env, row.m_owner->GetModelId().ToHexStr().c_str()));
        row.m_hookArg.Set("element", Napi::String::New(env, row.m_ownerId.ToHexStr().c_str()));
    }

    if (!failures.IsEmpty())
        return failures.ToJs(env);

    // Phase 2: JS pre-delete handlers for all aspects, before anything is written
    BulkAspectJsHooks hooks(db, env);
    bvector<std::pair<DgnClassId, Napi::Object>> hookCalls;
    for (auto const& row : rows)
        hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    hooks.Call(hookCalls, "onBulkDelete", "onDelete");

    // Phase 3: update each owning element once, then delete the aspects of each class with one statement (see DgnElements::DeleteElementAspects).
    // The aspects are validated again, because the pre-delete handlers may have changed them.
    if (!rows.empty()) {
        bvector<ECInstanceId> ids;
        for (auto const& row : rows)
            ids.push_back(row.m_id);

        BulkDeleteElementAspectsResult result = db.Elements().DeleteElementAspects(ids);
        if (BulkElementAspectsStatus::ValidationFailed == result.status && !result.failures.empty()) {
            auto const& failure = result.failures.front();
            DgnDbStatus status = FailureReason::ElementNotFound == failure.reason ? DgnDbStatus::MissingId :
                FailureReason::ClassNotFound == failure.reason ? DgnDbStatus::BadSchema : DgnDbStatus::NotFound;
            throwBulkAspectWriteError(env, functionName, status, Utf8PrintfString("aspect %s can no longer be deleted after the onDelete handlers", failure.aspectId.ToHexStr().c_str()));
        }
        if (BulkElementAspectsStatus::Success != result.status) {
            if (BE_SQLITE_OK != result.sqlStatus)
                throwBulkAspectSqlError(env, functionName, result.sqlStatus, "unable to delete the aspects; abandon the changes");
            throwBulkAspectWriteError(env, functionName, DgnDbStatus::Success != result.writeStatus ? result.writeStatus : DgnDbStatus::WriteError, "unable to delete the aspects");
        }
    }

    // Phase 4: JS post-delete handlers for all aspects, in input order
    hookCalls.clear();
    for (auto const& row : rows)
        hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    hooks.Call(hookCalls, "onBulkDeleted", "onDeleted");

    return failures.ToJs(env);
}