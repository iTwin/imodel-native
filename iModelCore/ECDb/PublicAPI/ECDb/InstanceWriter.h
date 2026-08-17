/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECInstanceId.h>
#include <ECDb/ECSqlStatement.h>
#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include <ECDb/SchemaManager.h>
#include <list>
#include <optional>
#include <vector>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct PropertyBinder final {
    using Finder = std::function<std::optional<PropertyBinder>(Utf8CP)>;

private:
    ECN::ECPropertyCP m_property;
    IECSqlBinder* m_binder;

public:
    PropertyBinder(ECN::ECPropertyCR prop, IECSqlBinder& binder) : m_property(&prop), m_binder(&binder) {}
    PropertyBinder(PropertyBinder const&) = default;
    PropertyBinder& operator=(PropertyBinder const&) = default;
    ECN::ECPropertyCR GetProperty() const { return *m_property; }
    IECSqlBinder& GetBinder() const { return *m_binder; }
};

//=======================================================================================
//! @internal Allow fast reading of full instance by its classid and instanceId
//=======================================================================================
struct InstanceWriter final {
public:
    enum class WriterOp {
        Insert,
        Update,
        Delete,
    };

    struct InsertOptions;
    struct UpdateOptions;
    struct DeleteOptions;

    struct Options {
        using CustomBindHandler = std::function<PropertyHandlerResult(ECN::ECPropertyCR, IECSqlBinder&, BeJsConst, BeJsConst, ECSqlStatus&)>;
        using UserPropertyHandler = std::function<PropertyHandlerResult(Utf8CP, BeJsConst, PropertyBinder::Finder, ECSqlStatus&)>;
        using CanBindHanlder = std::function<bool(ECN::ECPropertyCR)>;

    protected:
        bool m_useJsName;
        UserPropertyHandler m_userPropertyHandler;
        CustomBindHandler m_customBindHandler;
        CanBindHanlder m_canBindHandler;
        WriterOp m_op;

    public:
        Options(WriterOp op) : m_useJsName(false), m_op(op) {}
        bool GetUseJsNames() const { return m_useJsName; }
        CustomBindHandler GetCustomBindHandler() const { return m_customBindHandler; }
        CanBindHanlder GetCanBindHandler() const { return m_canBindHandler; }
        UserPropertyHandler GetUserPropertyHandler() const { return m_userPropertyHandler; }
        Options& UseJsNames(bool v) {
            m_useJsName = v;
            return *this;
        }
        Options& SetUserPropertyHandler(UserPropertyHandler handler) {
            m_userPropertyHandler = handler;
            return *this;
        }
        Options& SetCustomBindHandler(CustomBindHandler handler) {
            m_customBindHandler = handler;
            return *this;
        }
        Options& SetCanBindHandler(CanBindHanlder handler) {
            m_canBindHandler = handler;
            return *this;
        }
        InsertOptions& asInsert() {
            BeAssert(m_op == WriterOp::Insert);
            return static_cast<InsertOptions&>(*this);
        }
        UpdateOptions& asUpdate() {
            BeAssert(m_op == WriterOp::Update);
            return static_cast<UpdateOptions&>(*this);
        }
        DeleteOptions& asDelete() {
            BeAssert(m_op == WriterOp::Delete);
            return static_cast<DeleteOptions&>(*this);
        }
        bool isInsert() const { return m_op == WriterOp::Insert; }
        bool isUpdate() const { return m_op == WriterOp::Update; }
        bool isDelete() const { return m_op == WriterOp::Delete; }
    };

    struct UpdateOptions final : public Options {
    private:
        bool m_useIncrementalUpdate = false;
        std::optional<BeJsConst> m_expectedOldValues;

    public:
        UpdateOptions() : Options(WriterOp::Update) {}
        bool GetUseIncrementalUpdate() const { return m_useIncrementalUpdate; }
        // This can cause slow performance as it will read the existing instance from the database override
        // the properties that are not specified in the input JSON.
        UpdateOptions& UseIncrementalUpdate(bool v) {
            m_useIncrementalUpdate = v;
            return *this;
        }
        // Optional "expected old values" JSON (same shape as the instance being written; only the properties to be
        // verified need be present). When set, the row is only updated if its current values still match these
        // values (a null-safe comparison), so the caller can detect optimistic-concurrency conflicts. Identity
        // members (id/className/classFullName/ECInstanceId/ECClassId) are ignored if present, since the row is
        // already located via the primary instance/key argument. Array-valued properties are not supported.
        bool HasExpectedOldValues() const { return m_expectedOldValues.has_value(); }
        BeJsConst GetExpectedOldValues() const { return *m_expectedOldValues; }
        UpdateOptions& CompareBeforeUpdate(BeJsConst expectedOldValues) {
            m_expectedOldValues.emplace(expectedOldValues);
            return *this;
        }
    };

    struct InsertOptions final : public Options {
        enum class InstanceIdMode {
            Auto,   // generate a new instanceId as generated by ECDb
            Manual, // use the instanceId from InsertOptions set using SetNewInstanceId()
            FromJs, // use the instanceId from the input JSON
        };

    private:
        ECInstanceId m_newInstanceId;
        InstanceIdMode m_instanceIdMode = InstanceIdMode::Auto;

    public:
        InsertOptions() : Options(WriterOp::Insert) {}
        ECInstanceId GetInstanceId() const { return m_newInstanceId; }
        ECDB_EXPORT InsertOptions& UseInstanceId(ECInstanceId id);
        ECDB_EXPORT InsertOptions& UseInstanceIdFromJs();
        ECDB_EXPORT InsertOptions& UseAutoECInstanceId();
        InstanceIdMode GetInstanceIdMode() const { return m_instanceIdMode; }
    };

    struct DeleteOptions final : public Options {
    private:
        std::optional<BeJsConst> m_expectedOldValues;

    public:
        DeleteOptions() : Options(WriterOp::Delete) {}
        // See UpdateOptions::CompareBeforeUpdate. When set, the row is only deleted if its current values still
        // match these values.
        bool HasExpectedOldValues() const { return m_expectedOldValues.has_value(); }
        BeJsConst GetExpectedOldValues() const { return *m_expectedOldValues; }
        DeleteOptions& CompareBeforeDelete(BeJsConst expectedOldValues) {
            m_expectedOldValues.emplace(expectedOldValues);
            return *this;
        }
    };
    struct Impl;

private:
    Impl* m_pImpl;
    /**
     * 1. TimeStampProperty is skipped during insert & update.
     */
public:
    ECDB_EXPORT InstanceWriter(ECDbCR ecdb, uint32_t cacheSize = 40);
    ECDB_EXPORT ~InstanceWriter();
    ECDB_EXPORT Utf8StringCR GetLastError() const;
    //! When the most recent Insert/Update failed with a UNIQUE constraint violation, populates \p out with
    //! { "kind": "UniqueConstraint", "uniqueConstraintProperties": [...], "conflictingRow": {...} } and returns
    //! true. "uniqueConstraintProperties" are the JS-cased access strings of the properties making up the
    //! violated index; "conflictingRow" is the pre-existing instance the write collided with, and is omitted if
    //! it could not be identified. Returns false for any other result, or if the violated index could not be
    //! mapped back to EC properties. Only valid until the next write on this writer.
    ECDB_EXPORT bool TryGetLastWriteConflictDetail(BeJsValue out) const;
    ECDB_EXPORT DbResult Insert(BeJsConst inst, InsertOptions const& options);
    ECDB_EXPORT DbResult Insert(BeJsConst inst, InsertOptions const& options, ECInstanceKey& key);
    ECDB_EXPORT DbResult Update(BeJsConst inst, UpdateOptions const& options);
    //! Like Update(BeJsConst, UpdateOptions const&), but when options.HasExpectedOldValues() and the row was not
    //! affected (i.e. 0 rows modified), \p conflictingProperties is populated with the JSON member names (as
    //! given in options.GetExpectedOldValues()) of the properties whose current value no longer matches the
    //! expected value; if the row itself does not exist, every checked property is reported instead. Otherwise
    //! (a successful update, or no expected old values were given) \p conflictingProperties is left empty.
    ECDB_EXPORT DbResult Update(BeJsConst inst, UpdateOptions const& options, std::vector<Utf8String>& conflictingProperties);
    ECDB_EXPORT DbResult Delete(BeJsConst inst, DeleteOptions const& options);
    ECDB_EXPORT DbResult Delete(BeJsConst inst, DeleteOptions const& options, std::vector<Utf8String>& conflictingProperties);
    ECDB_EXPORT DbResult Delete(ECInstanceKeyCR key, DeleteOptions const& options);
    ECDB_EXPORT DbResult Delete(ECInstanceKeyCR key, DeleteOptions const& options, std::vector<Utf8String>& conflictingProperties);

    ECDB_EXPORT void ToJson(BeJsValue out, ECInstanceId instanceId, ECN::ECClassId classId, JsFormat jsFmt = JsFormat::Standard) const;
    ECDB_EXPORT void ToJson(BeJsValue out, ECInstanceKeyCR key, JsFormat jsFmt = JsFormat::Standard) const;
    ECDB_EXPORT bool TryGetId(ECInstanceId& instanceId, BeJsConst in, JsFormat jsFmt = JsFormat::Standard) const;
    ECDB_EXPORT bool TryGetClassId(ECN::ECClassId& classId, BeJsConst in, JsFormat jsFmt = JsFormat::Standard) const;
    ECDB_EXPORT bool TryGetInstanceKey(ECInstanceKeyR key, BeJsConst in, JsFormat jsFmt = JsFormat::Standard) const;

    ECDB_EXPORT void Reset();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
