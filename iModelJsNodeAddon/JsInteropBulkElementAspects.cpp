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

enum class BulkElementAspectsStatus : int { Success = 0, PartialSuccess = 1, Failed = 2, ValidationFailed = 3 };

// BestEffort writes the valid rows and reports the invalid ones. FailFast writes nothing, and calls no JS handlers, if any row is invalid.
enum class BulkElementAspectsMode : int { BestEffort = 0, FailFast = 1 };

/*---------------------------------------------------------------------------------**//**
* Reads the mode from the optional options object ({ mode?: BulkElementAspectsMode }). Throws a JS TypeError for an invalid mode.
+---------------+---------------+---------------+---------------+---------------+------*/
static BulkElementAspectsMode parseBulkAspectsMode(Napi::Value options, Utf8CP functionName) {
    if (options.IsEmpty() || options.IsUndefined() || options.IsNull())
        return BulkElementAspectsMode::BestEffort;

    Utf8String err("Invalid options given to ");
    err += functionName;
    if (!options.IsObject())
        BeNapi::ThrowJsTypeException(options.Env(), err.c_str());

    Napi::Value mode = options.As<Napi::Object>().Get("mode");
    if (mode.IsUndefined())
        return BulkElementAspectsMode::BestEffort;

    if (mode.IsNumber()) {
        double value = mode.As<Napi::Number>().DoubleValue();
        if (value == (double) BulkElementAspectsMode::BestEffort)
            return BulkElementAspectsMode::BestEffort;
        if (value == (double) BulkElementAspectsMode::FailFast)
            return BulkElementAspectsMode::FailFast;
    }

    err = "Invalid options.mode given to ";
    err += functionName;
    BeNapi::ThrowJsTypeException(options.Env(), err.c_str());
}

//=======================================================================================
// Collects per-row failures, keyed by the index of the row in the input array.
//=======================================================================================
struct BulkAspectFailures {
    uint32_t m_total;
    std::map<uint32_t, std::pair<DgnDbStatus, Utf8String>> m_failures;

    explicit BulkAspectFailures(uint32_t total) : m_total(total) {}

    // Only the first failure of a row is recorded.
    void Fail(uint32_t index, DgnDbStatus status, Utf8StringCR message) { m_failures.emplace(index, std::make_pair(status, message)); }
    bool HasFailed(uint32_t index) const { return m_failures.end() != m_failures.find(index); }
    bool IsEmpty() const { return m_failures.empty(); }

    // validationFailed: nothing was written because FailFast found invalid rows.
    Napi::Object ToJs(Napi::Env env, bool validationFailed = false) const {
        BulkElementAspectsStatus status = validationFailed ? BulkElementAspectsStatus::ValidationFailed :
            m_failures.empty() ? BulkElementAspectsStatus::Success :
            (m_failures.size() >= m_total ? BulkElementAspectsStatus::Failed : BulkElementAspectsStatus::PartialSuccess);

        auto result = Napi::Object::New(env);
        result.Set("status", Napi::Number::New(env, (int) status));
        auto failures = Napi::Array::New(env, m_failures.size());
        uint32_t i = 0;
        for (auto const& failure : m_failures) {
            auto obj = Napi::Object::New(env);
            obj.Set("index", Napi::Number::New(env, failure.first));
            obj.Set("status", Napi::Number::New(env, (int) failure.second.first));
            obj.Set("message", Napi::String::New(env, failure.second.second.c_str()));
            failures.Set(i++, obj);
        }
        result.Set("failures", failures);
        return result;
    }
};

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
        failures.Fail(index, DgnDbStatus::BadArg, "aspect props must be an object");
        return false;
    }

    row.m_props = propsVal.As<Napi::Object>();
    BeJsConst props(row.m_props);

    DgnElement::RelatedElement relatedElement;
    relatedElement.FromJson(db, props["element"]);
    if (!relatedElement.IsValid()) {
        failures.Fail(index, DgnDbStatus::InvalidId, "invalid owning element id");
        return false;
    }

    auto owner = owners.find(relatedElement.m_id);
    if (owners.end() == owner)
        owner = owners.emplace(relatedElement.m_id, db.Elements().GetElement(relatedElement.m_id)).first;

    if (!owner->second.IsValid()) {
        failures.Fail(index, DgnDbStatus::MissingId, "owning element not found");
        return false;
    }
    row.m_owner = owner->second;

    auto classFullName = props[DgnElement::json_classFullName()];
    if (!classFullName.isString() || classFullName.asString().empty()) {
        failures.Fail(index, DgnDbStatus::BadArg, "aspect props must have a classFullName");
        return false;
    }

    // BadSchema (rather than WrongClass) lets callers tell a class missing from the iModel apart from a class that is not an aspect.
    row.m_classId = ECJsonUtilities::GetClassIdFromClassNameJson(classFullName, db.GetClassLocater());
    if (!row.m_classId.IsValid()) {
        failures.Fail(index, DgnDbStatus::BadSchema, "aspect class not found");
        return false;
    }

    row.m_class = db.Schemas().GetClass(row.m_classId);
    if (nullptr == row.m_class) {
        failures.Fail(index, DgnDbStatus::BadSchema, "aspect class not found");
        return false;
    }

    row.m_isMulti = row.m_class->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);
    if (!row.m_isMulti && !row.m_class->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementUniqueAspect)) {
        failures.Fail(index, DgnDbStatus::WrongClass, "class is not an ElementUniqueAspect or ElementMultiAspect");
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
* Insert many ElementAspects. Rows are validated first; then the JS pre-insert handlers are called for all valid rows
* (an exception aborts the whole call before anything is written); then the aspects are written grouped by owning
* element, with a single update of each owning element; finally the JS post-insert handlers are called for the rows that succeeded.
* The aspects of an owner are written by DgnElement::_OnUpdated, after the owner's row has been written, so an owner update that
* fails leaves none of its aspects written. An aspect whose write fails while its owner is updated is reported as failed; the owner
* and its other aspects are still written. As with the single-aspect operations, nothing is rolled back: the failed aspect may be
* partially written (e.g. a row without its properties), so the caller should abandon the changes. Exceptions thrown by JS handlers
* during or after the writes (the element's onUpdated, or the post-insert handlers) propagate to the caller and leave the rows that
* were already written in the Txn, the same as the single-aspect operations; the caller can abandon the changes.
* In FailFast mode, if any row fails validation nothing is written, no JS handlers are called, and ValidationFailed is returned
* with the invalid rows. Failures found while writing are still reported as PartialSuccess/Failed, with the other rows written.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object JsInterop::InsertElementAspects(DgnDbR db, Napi::Array propsArray, Napi::Value options) {
    Napi::Env env = propsArray.Env();
    auto mode = parseBulkAspectsMode(options, "insertElementAspects");
    uint32_t count = propsArray.Length();
    BulkAspectFailures failures(count);
    bvector<ECInstanceId> insertedIds(count);

    auto makeResult = [&](bool validationFailed) {
        auto result = failures.ToJs(env, validationFailed);
        auto ids = Napi::Array::New(env, count);
        for (uint32_t i = 0; i < count; ++i)
            ids.Set(i, Napi::String::New(env, insertedIds[i].IsValid() ? insertedIds[i].ToHexStr().c_str() : "0"));
        result.Set("ids", ids);
        return result;
    };

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
            failures.Fail(i, DgnDbStatus::BadRequest, "invalid aspect properties");
            continue;
        }

        rows.push_back(row);
    }

    if (BulkElementAspectsMode::FailFast == mode && !failures.IsEmpty())
        return makeResult(true);

    // Phase 2: JS pre-insert handlers for all valid rows, before anything is written
    BulkAspectJsHooks hooks(db, env);
    bvector<std::pair<DgnClassId, Napi::Object>> hookCalls;
    for (auto const& row : rows)
        hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    hooks.Call(hookCalls, "onBulkInsert", "onInsert");

    // Phase 3: write, grouped by owning element. All aspects of an owner are added to one editable copy and written by a single Update.
    ECClassId multiAspectRelClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects).GetValue());
    hookCalls.clear();

    auto ownerGroups = groupBulkAspectRowsByOwner(rows, [](InsertRow const& row) { return row.m_owner->GetElementId(); });
    for (auto const& group : ownerGroups) {
        auto const& rowIndices = group.second;
        auto failGroup = [&](DgnDbStatus status, Utf8StringCR message) {
            for (auto r : rowIndices)
                failures.Fail(rows[r].m_index, status, message);
        };

        DgnElementPtr elementEdit = rows[rowIndices.front()].m_owner->CopyForEdit();
        if (!elementEdit.IsValid()) {
            failGroup(DgnDbStatus::WriteError, "unable to edit owning element");
            continue;
        }

        // SetAspect behaves as an upsert for unique aspects, so when the same (element, class) appears more than once,
        // the last row wins and all of those rows report the resulting aspect id.
        std::map<DgnClassId, RefCountedPtr<DgnElement::UniqueAspect>> uniqueAspects;
        std::map<size_t, RefCountedPtr<DgnElement::MultiAspect>> multiAspects;
        for (auto r : rowIndices) {
            auto& row = rows[r];
            DgnDbStatus stat;
            if (row.m_isMulti) {
                ECValue elementVal;
                row.m_instance->GetValue(elementVal, "Element");
                if (elementVal.IsNull() || !elementVal.IsNavigation() || !elementVal.GetNavigationInfo().GetRelationshipClassId().IsValid())
                    row.m_instance->SetValue("Element", ECValue(group.first, multiAspectRelClassId));

                auto aspect = DgnElement::GenericMultiAspect::AddAspect(*elementEdit, *row.m_instance, &stat);
                if (DgnDbStatus::Success != stat || !aspect.IsValid()) {
                    failures.Fail(row.m_index, stat, "unable to add multi aspect");
                    continue;
                }
                multiAspects[r] = aspect;
                continue;
            }

            auto aspect = DgnElement::GenericUniqueAspect::SetAspect(*elementEdit, *row.m_instance, nullptr, &stat);
            if (DgnDbStatus::Success != stat || !aspect.IsValid()) {
                failures.Fail(row.m_index, stat, "unable to set unique aspect");
                continue;
            }
            uniqueAspects[row.m_classId] = aspect;
        }

        DgnDbStatus stat = elementEdit->Update();
        if (DgnDbStatus::Success != stat) {
            failGroup(stat, "unable to update owning element");
            continue;
        }

        // The update of the owner succeeds even when one of its aspects cannot be written, so check each aspect.
        for (auto r : rowIndices) {
            auto& row = rows[r];
            if (failures.HasFailed(row.m_index))
                continue;

            DgnElement::Aspect* aspect = row.m_isMulti ? static_cast<DgnElement::Aspect*>(multiAspects[r].get()) : uniqueAspects[row.m_classId].get();
            ECInstanceId aspectId = aspect->GetAspectInstanceId();
            DgnDbStatus writeStatus = aspect->GetLastWriteStatus();
            if (DgnDbStatus::Success != writeStatus || !aspectId.IsValid()) {
                failures.Fail(row.m_index, DgnDbStatus::Success != writeStatus ? writeStatus : DgnDbStatus::WriteError, "unable to write aspect");
                continue;
            }
            insertedIds[row.m_index] = aspectId;
        }
    }

    // Phase 4: JS post-insert handlers for the rows that succeeded, in input order
    for (auto const& row : rows) {
        if (!failures.HasFailed(row.m_index))
            hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    }
    hooks.Call(hookCalls, "onBulkInserted", "onInserted");

    return makeResult(false);
}

/*---------------------------------------------------------------------------------**//**
* Update many ElementAspects. Same phases as InsertElementAspects. Properties not present in the props are left unchanged.
* A multi aspect is only updated when it is owned by the element given in its props.
* If the props of an aspect cannot be applied after the pre-update handlers ran, the owning element and all of its aspects in
* the batch are left unchanged. An aspect whose write fails while its owner is updated is reported as failed; the owner and its
* other aspects are still written. FailFast mode behaves as for InsertElementAspects; aspects that are not found, or not owned by the
* given element, are validation failures.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object JsInterop::UpdateElementAspects(DgnDbR db, Napi::Array propsArray, Napi::Value options) {
    Napi::Env env = propsArray.Env();
    auto mode = parseBulkAspectsMode(options, "updateElementAspects");
    uint32_t count = propsArray.Length();
    BulkAspectFailures failures(count);

    struct UpdateRow : BulkAspectPropsRow {
        ECInstanceId m_aspectId;
        IECInstanceP m_aspect = nullptr;
        RefCountedPtr<DgnElement::Aspect> m_platformAspect;
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
                failures.Fail(i, DgnDbStatus::InvalidId, "invalid aspect id");
                continue;
            }
            multiIds.push_back(row.m_aspectId);
        }

        // Validate the props on a scratch instance so that an invalid row cannot leave a partially modified aspect behind.
        auto scratch = row.m_class->GetDefaultStandaloneEnabler()->CreateInstance();
        if (!scratch.IsValid() || SUCCESS != bulkAspectJsonToInstance(db, *scratch, row, true)) {
            failures.Fail(i, DgnDbStatus::BadRequest, "invalid aspect properties");
            continue;
        }

        candidates.push_back(row);
    }

    // Multi aspects are loaded by id alone, so look up their owners and only update those owned by the given element.
    std::map<ECInstanceId, DgnElementId> multiOwners;
    if (!multiIds.empty()) {
        CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(
            "SELECT ECInstanceId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect) " WHERE ECInstanceId IN (SELECT id FROM IdSet(?))");
        if (!statement.IsValid() || !bindBulkAspectIdSet(*statement, 1, multiIds))
            throwSqlError();

        while (BE_SQLITE_ROW == statement->Step())
            multiOwners[statement->GetValueId<ECInstanceId>(0)] = statement->GetValueId<DgnElementId>(1);
        statement->Reset();
        statement->ClearBindings();
    }

    // Load the aspects to update onto one editable copy per owning element
    bvector<UpdateRow> rows;
    std::map<DgnElementId, DgnElementPtr> elementEdits;
    for (auto& row : candidates) {
        DgnElementId ownerId = row.m_owner->GetElementId();
        if (row.m_isMulti) {
            auto found = multiOwners.find(row.m_aspectId);
            if (multiOwners.end() == found) {
                failures.Fail(row.m_index, DgnDbStatus::NotFound, "aspect not found");
                continue;
            }
            if (found->second != ownerId) {
                failures.Fail(row.m_index, DgnDbStatus::NotFound, "aspect not found on the owning element");
                continue;
            }
        }

        auto& elementEdit = elementEdits[ownerId];
        if (elementEdit.IsNull())
            elementEdit = row.m_owner->CopyForEdit();
        if (!elementEdit.IsValid()) {
            failures.Fail(row.m_index, DgnDbStatus::WriteError, "unable to edit owning element");
            continue;
        }

        if (row.m_isMulti) {
            row.m_aspect = DgnElement::GenericMultiAspect::GetAspectP(*elementEdit, *row.m_class, row.m_aspectId);
            if (nullptr != row.m_aspect)
                row.m_platformAspect = DgnElement::MultiAspect::GetAspectP(*elementEdit, *row.m_class, row.m_aspectId);
        } else {
            row.m_aspect = DgnElement::GenericUniqueAspect::GetAspectP(*elementEdit, *row.m_class);
            if (nullptr != row.m_aspect)
                row.m_platformAspect = DgnElement::UniqueAspect::GetAspectP(*elementEdit, *row.m_class);
        }
        if (nullptr == row.m_aspect || row.m_platformAspect.IsNull()) {
            failures.Fail(row.m_index, DgnDbStatus::NotFound, "aspect not found");
            continue;
        }

        rows.push_back(row);
    }

    if (BulkElementAspectsMode::FailFast == mode && !failures.IsEmpty())
        return failures.ToJs(env, true);

    // Phase 2: JS pre-update handlers for all valid rows, before anything is written
    BulkAspectJsHooks hooks(db, env);
    bvector<std::pair<DgnClassId, Napi::Object>> hookCalls;
    for (auto const& row : rows)
        hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    hooks.Call(hookCalls, "onBulkUpdate", "onUpdate");

    // Phase 3: write, grouped by owning element
    hookCalls.clear();
    auto ownerGroups = groupBulkAspectRowsByOwner(rows, [](UpdateRow const& row) { return row.m_owner->GetElementId(); });
    for (auto const& group : ownerGroups) {
        auto const& rowIndices = group.second;
        auto failGroup = [&](DgnDbStatus status, Utf8StringCR message) {
            for (auto r : rowIndices)
                failures.Fail(rows[r].m_index, status, message);
        };

        bool applied = true;
        for (auto r : rowIndices) {
            auto& row = rows[r];
            if (SUCCESS != bulkAspectJsonToInstance(db, *row.m_aspect, row, true)) {
                failures.Fail(row.m_index, DgnDbStatus::BadRequest, "invalid aspect properties");
                applied = false;
                break;
            }
        }

        if (!applied) {
            failGroup(DgnDbStatus::WriteError, "another aspect of the same owning element failed");
            continue;
        }

        DgnDbStatus stat = elementEdits[group.first]->Update();
        if (DgnDbStatus::Success != stat) {
            failGroup(stat, "unable to update owning element");
            continue;
        }

        // The update of the owner succeeds even when one of its aspects cannot be written, so check each aspect.
        for (auto r : rowIndices) {
            DgnDbStatus writeStatus = rows[r].m_platformAspect->GetLastWriteStatus();
            if (DgnDbStatus::Success != writeStatus)
                failures.Fail(rows[r].m_index, writeStatus, "unable to write aspect");
        }
    }

    // Phase 4: JS post-update handlers for the rows that succeeded, in input order
    for (auto const& row : rows) {
        if (!failures.HasFailed(row.m_index))
            hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    }
    hooks.Call(hookCalls, "onBulkUpdated", "onUpdated");

    return failures.ToJs(env);
}

/*---------------------------------------------------------------------------------**//**
* Delete many ElementAspects by id. Ids are resolved with a single query; each owning element is updated once and the aspects of
* each class are deleted with a single statement (DgnElements::DeleteElementAspects). Duplicate ids are deleted once.
* An aspect whose delete fails after its owner is updated is reported as failed; the owner and its other aspects are still written.
* FailFast mode behaves as for InsertElementAspects; malformed ids and aspects that are not found are validation failures.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object JsInterop::DeleteElementAspects(DgnDbR db, Napi::Array aspectIds, Napi::Value options) {
    Napi::Env env = aspectIds.Env();
    auto mode = parseBulkAspectsMode(options, "deleteElementAspects");
    uint32_t count = aspectIds.Length();
    BulkAspectFailures failures(count);

    struct DeleteRow {
        ECInstanceId m_id;
        bvector<uint32_t> m_indices;
        DgnClassId m_classId;
        DgnElementId m_ownerId;
        DgnElementCPtr m_owner;
        Napi::Object m_hookArg;
        bool m_found = false;
        bool m_failed = false;
    };

    // Phase 1: parse and resolve the ids
    bvector<DeleteRow> rows;
    std::map<ECInstanceId, size_t> rowById;
    for (uint32_t i = 0; i < count; ++i) {
        Napi::Value val = aspectIds[i];
        if (!val.IsString()) {
            failures.Fail(i, DgnDbStatus::BadArg, "aspect id must be a string");
            continue;
        }

        ECInstanceId aspectId(BeInt64Id::FromString(val.As<Napi::String>().Utf8Value().c_str()).GetValue());
        if (!aspectId.IsValid()) {
            failures.Fail(i, DgnDbStatus::InvalidId, "invalid aspect id");
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

    auto failRow = [&](DeleteRow& row, DgnDbStatus status, Utf8StringCR message) {
        row.m_failed = true;
        for (auto index : row.m_indices)
            failures.Fail(index, status, message);
    };

    if (!rows.empty()) {
        CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(
            "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementUniqueAspect) " WHERE ECInstanceId IN (SELECT id FROM IdSet(?)) UNION ALL "
            "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect)  " WHERE ECInstanceId IN (SELECT id FROM IdSet(?))");
        if (!statement.IsValid())
            throwSqlError();

        bvector<ECInstanceId> ids;
        for (auto const& row : rows)
            ids.push_back(row.m_id);
        if (!bindBulkAspectIdSet(*statement, 1, ids) || !bindBulkAspectIdSet(*statement, 2, ids))
            throwSqlError();

        while (BE_SQLITE_ROW == statement->Step()) {
            auto found = rowById.find(statement->GetValueId<ECInstanceId>(0));
            if (rowById.end() == found)
                continue;

            auto& row = rows[found->second];
            row.m_found = true;
            row.m_classId = statement->GetValueId<DgnClassId>(1);
            row.m_ownerId = statement->GetValueId<DgnElementId>(2);
        }
        statement->Reset();
        statement->ClearBindings();
    }

    std::map<DgnElementId, DgnElementCPtr> owners;
    for (auto& row : rows) {
        if (!row.m_found) {
            failRow(row, DgnDbStatus::NotFound, "aspect not found");
            continue;
        }

        if (nullptr == db.Schemas().GetClass(row.m_classId)) {
            failRow(row, DgnDbStatus::BadSchema, "aspect class not found");
            continue;
        }

        auto owner = owners.find(row.m_ownerId);
        if (owners.end() == owner)
            owner = owners.emplace(row.m_ownerId, db.Elements().GetElement(row.m_ownerId)).first;
        if (!owner->second.IsValid()) {
            failRow(row, DgnDbStatus::MissingId, "owning element not found");
            continue;
        }
        row.m_owner = owner->second;

        row.m_hookArg = Napi::Object::New(env);
        row.m_hookArg.Set("aspectId", Napi::String::New(env, row.m_id.ToHexStr().c_str()));
        row.m_hookArg.Set(DgnElement::json_model(), Napi::String::New(env, row.m_owner->GetModelId().ToHexStr().c_str()));
        row.m_hookArg.Set("element", Napi::String::New(env, row.m_ownerId.ToHexStr().c_str()));
    }

    bvector<DeleteRow> validRows;
    for (auto const& row : rows) {
        if (!row.m_failed)
            validRows.push_back(row);
    }

    if (BulkElementAspectsMode::FailFast == mode && !failures.IsEmpty())
        return failures.ToJs(env, true);

    // Phase 2: JS pre-delete handlers for all valid rows, before anything is written
    BulkAspectJsHooks hooks(db, env);
    bvector<std::pair<DgnClassId, Napi::Object>> hookCalls;
    for (auto const& row : validRows)
        hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    hooks.Call(hookCalls, "onBulkDelete", "onDelete");

    // Phase 3: update each owning element once, then delete the aspects of each class with one statement (see DgnElements::DeleteElementAspects).
    // The aspects are looked up again, so ones that the pre-delete handlers removed are reported as not found.
    hookCalls.clear();
    if (!validRows.empty()) {
        bvector<ECInstanceId> validIds;
        std::map<ECInstanceId, size_t> validRowById;
        for (size_t r = 0; r < validRows.size(); ++r) {
            validIds.push_back(validRows[r].m_id);
            validRowById[validRows[r].m_id] = r;
        }

        BulkDeleteElementAspectsResult result = db.Elements().DeleteElementAspects(validIds);
        if (BE_SQLITE_OK != result.sqlStatus)
            throwSqlError();

        for (auto const& failure : result.failures) {
            auto found = validRowById.find(failure.aspectId);
            if (validRowById.end() != found)
                failRow(validRows[found->second], failure.status, failure.message);
        }
    }

    // Phase 4: JS post-delete handlers for the rows that succeeded, in input order
    for (auto const& row : validRows) {
        if (!row.m_failed)
            hookCalls.push_back(std::make_pair(row.m_classId, row.m_hookArg));
    }
    hooks.Call(hookCalls, "onBulkDeleted", "onDeleted");

    return failures.ToJs(env);
}
