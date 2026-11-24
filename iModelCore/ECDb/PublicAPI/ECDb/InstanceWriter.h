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
        bool m_convertClassIdsToClassNames = false;
        ECN::ECClassId m_classId;
        UserPropertyHandler m_userPropertyHandler;
        CustomBindHandler m_customBindHandler;
        CanBindHanlder m_canBindHandler;
        WriterOp m_op;

    public:
        Options(WriterOp op) : m_useJsName(false), m_op(op) {}
        bool GetUseJsNames() const { return m_useJsName; }
        bool GetConvertClassIdsToClassNames() const { return m_convertClassIdsToClassNames; }
        //! When valid, the class to write. The ECClassId, className or classFullName of the instance are then ignored and
        //! need not be supplied. This avoids looking up the class by name for each instance.
        ECN::ECClassId GetClassId() const { return m_classId; }
        Options& UseClassId(ECN::ECClassId v) {
            m_classId = v;
            return *this;
        }
        CustomBindHandler GetCustomBindHandler() const { return m_customBindHandler; }
        CanBindHanlder GetCanBindHandler() const { return m_canBindHandler; }
        UserPropertyHandler GetUserPropertyHandler() const { return m_userPropertyHandler; }
        Options& UseJsNames(bool v) {
            m_useJsName = v;
            return *this;
        }
        Options& ConvertClassIdsToClassNames(bool v) {
            m_convertClassIdsToClassNames = v;
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
        bool m_exactClass = false;
        ECInstanceId m_instanceId;

    public:
        UpdateOptions() : Options(WriterOp::Update) {}
        bool GetUseIncrementalUpdate() const { return m_useIncrementalUpdate; }
        //! When valid, the id of the instance to update. The ECInstanceId or id of the instance is then ignored and need not be supplied.
        ECInstanceId GetInstanceId() const { return m_instanceId; }
        UpdateOptions& UseInstanceId(ECInstanceId v) {
            m_instanceId = v;
            return *this;
        }
        //! When true, only the properties that the instance supplies are changed, and the others keep their stored values.
        //! Otherwise, every property that the instance does not supply is set to null.
        //! A supplied struct object is merged with the stored struct, so that its omitted members are kept.
        //! @remarks When no UserPropertyHandler or CanBindHandler is set and no struct object is supplied, the update sets
        //! only the supplied properties and does not read the stored instance, and a CustomBindHandler is called only for
        //! the supplied properties. Otherwise, the update reads the stored instance and writes every property, and a
        //! CustomBindHandler is also called for the stored values that are written back.
        UpdateOptions& UseIncrementalUpdate(bool v) {
            m_useIncrementalUpdate = v;
            return *this;
        }
        bool GetExactClass() const { return m_exactClass; }
        //! When true, only an instance of exactly the class to write is updated, using UPDATE ONLY. This is faster, because
        //! it does not filter by the class hierarchy. The caller must know that the class is the class of the instance:
        //! an instance of a subclass is not updated, and no error is returned.
        UpdateOptions& UseExactClass(bool v) {
            m_exactClass = v;
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
        bool m_exactClass = false;

    public:
        DeleteOptions() : Options(WriterOp::Delete) {}
        bool GetExactClass() const { return m_exactClass; }
        //! When true, only an instance of exactly the class to delete is deleted, using DELETE FROM ONLY. This is faster,
        //! because it does not filter by the class hierarchy. The caller must know that the class is the class of the
        //! instance: an instance of a subclass is not deleted, and no error is returned.
        DeleteOptions& UseExactClass(bool v) {
            m_exactClass = v;
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
    ECDB_EXPORT DbResult Insert(BeJsConst inst, InsertOptions const& options);
    ECDB_EXPORT DbResult Insert(BeJsConst inst, InsertOptions const& options, ECInstanceKey& key);
    ECDB_EXPORT DbResult Update(BeJsConst inst, UpdateOptions const& options);
    ECDB_EXPORT DbResult Delete(BeJsConst inst, DeleteOptions const& options);
    ECDB_EXPORT DbResult Delete(ECInstanceKeyCR key, DeleteOptions const& options);

    ECDB_EXPORT void ToJson(BeJsValue out, ECInstanceId instanceId, ECN::ECClassId classId, JsFormat jsFmt = JsFormat::Standard) const;
    ECDB_EXPORT void ToJson(BeJsValue out, ECInstanceKeyCR key, JsFormat jsFmt = JsFormat::Standard) const;
    ECDB_EXPORT bool TryGetId(ECInstanceId& instanceId, BeJsConst in, JsFormat jsFmt = JsFormat::Standard) const;
    ECDB_EXPORT bool TryGetClassId(ECN::ECClassId& classId, BeJsConst in, JsFormat jsFmt = JsFormat::Standard, bool convertClassIdsToClassNames = false) const;
    ECDB_EXPORT bool TryGetInstanceKey(ECInstanceKeyR key, BeJsConst in, JsFormat jsFmt = JsFormat::Standard, bool convertClassIdsToClassNames = false) const;

    ECDB_EXPORT void Reset();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
