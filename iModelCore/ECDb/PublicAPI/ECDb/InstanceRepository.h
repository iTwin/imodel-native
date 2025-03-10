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

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct InstanceRepository final {

    struct Args {
    private:
        const BeJsConst& m_userOptions;
        ECDbCR m_ecdb;
        JsFormat m_format;
        ECInstanceKeyCR m_key;
        const BeJsConst& m_instance;

    public:
        Args(ECDbCR ecdb, ECInstanceKeyCR key, JsFormat format, const BeJsConst& userOptions, const BeJsConst& instance)
            : m_ecdb(ecdb), m_format(format), m_userOptions(userOptions), m_key(key), m_instance(instance) {}
        ECInstanceKeyCR GetKey() const { return m_key; }
        ECDbCR GetECDb() const { return m_ecdb; }
        JsFormat GetFormat() const { return m_format; }
        const BeJsConst& GetUserOptions() const { return m_userOptions; }
        BeJsConst GetInstance() const { return m_instance; }
    };

    struct ReadArgs : Args {
    private:
        IECSqlValue const& m_value;
        BeJsValue& m_out;

    public:
        ReadArgs(ECDbCR ecdb, ECInstanceKeyCR key, JsFormat format, BeJsConst userOptions, IECSqlValue const& value, BeJsValue& out, const BeJsConst& instance)
            : Args(ecdb, key, format, userOptions, instance), m_value(value), m_out(out) {}
        const IECSqlValue& GetValue() const { return m_value; }
        BeJsValue GetOut() { return m_out; }
        ECN::ECPropertyCR GetProperty() const {
            auto prop = m_value.GetColumnInfo().GetProperty();
            return *prop;
        }
    };

    struct InsertArgs : Args {
    private:
        IECSqlBinder& m_binder;
        const BeJsConst& m_in;
        ECN::ECPropertyCR m_prop;

    public:
        InsertArgs(ECDbCR ecdb, ECInstanceKeyCR key, JsFormat format, BeJsConst userOptions, IECSqlBinder& binder, const BeJsConst& in, ECN::ECPropertyCR prop, const BeJsConst& instance)
            : Args(ecdb, key, format, userOptions, instance), m_binder(binder), m_in(in), m_prop(prop) {}
        IECSqlBinder& GetBinder() const { return m_binder; }
        const BeJsConst& GetIn() const { return m_in; }
        ECN::ECPropertyCR GetProperty() const { return m_prop; }
    };

    struct UpdateArgs : Args {
    private:
        IECSqlBinder& m_binder;
        const BeJsConst& m_in;
        ECN::ECPropertyCR m_prop;

    public:
        UpdateArgs(ECDbCR ecdb, ECInstanceKeyCR key, JsFormat format, BeJsConst userOptions, IECSqlBinder& binder, const BeJsConst& in, ECN::ECPropertyCR prop, const BeJsConst& instance)
            : Args(ecdb, key, format, userOptions, instance), m_binder(binder), m_in(in), m_prop(prop) {}
        IECSqlBinder& GetBinder() const { return m_binder; }
        const BeJsConst& GetIn() const { return m_in; }
        ECN::ECPropertyCR GetProperty() const { return m_prop; }
    };

    struct DeleteArgs : Args {
    public:
        DeleteArgs(ECDbCR ecdb, ECInstanceKeyCR key, JsFormat format, BeJsConst userOptions)
            : Args(ecdb, key, format, userOptions, BeJsDocument::Null()) {}
    };

    struct IClassHandler {
    private:
        ECN::ECClassId m_classId;

    public:
        IClassHandler(ECN::ECClassId classId) : m_classId(classId) {}
        ECN::ECClassId GetClassId() const { return m_classId; }
        virtual ~IClassHandler() = default;
        virtual PropertyHandlerResult OnNextId(ECInstanceId&) { return PropertyHandlerResult::Continue; };
        virtual PropertyHandlerResult OnInsert(InsertArgs&) { return PropertyHandlerResult::Continue; };
        virtual PropertyHandlerResult OnUpdate(UpdateArgs&) { return PropertyHandlerResult::Continue; };
        virtual PropertyHandlerResult OnRead(ReadArgs&) { return PropertyHandlerResult::Continue; };
    };

    ECDbCR m_ecdb;
    mutable BeMutex m_mutex;
    mutable std::map<ECN::ECClassId, std::vector<IClassHandler*>> m_handlerMap;
    mutable std::vector<std::unique_ptr<IClassHandler>> m_handlers;

    std::vector<IClassHandler*>& TryGetHandlers(ECN::ECClassId classId) const;

public:
    InstanceRepository(ECDbCR ecdb) : m_ecdb(ecdb) {}
    ~InstanceRepository() = default;
    InstanceRepository(InstanceRepository const&) = delete;
    InstanceRepository(InstanceRepository&&) = delete;
    InstanceRepository& operator=(InstanceRepository const&) = delete;
    InstanceRepository& operator=(InstanceRepository&&) = delete;
    ECDB_EXPORT DbResult Insert(BeJsConst in, BeJsConst userOptions, JsFormat inFmt, ECInstanceKeyR key) const;
    ECDB_EXPORT DbResult Update(BeJsConst in, BeJsConst userOptions, JsFormat inFmt) const;
    ECDB_EXPORT DbResult Delete(BeJsConst in, BeJsConst userOptions, JsFormat inFmt) const;
    ECDB_EXPORT DbResult Read(BeJsConst in, BeJsValue out, BeJsConst userOptions, JsFormat fmt) const;
    ECDB_EXPORT void Reset();
    template <typename T>
    bool RegisterClassHandler(ECN::ECClassId classId) {
        static_assert(std::is_base_of<IClassHandler, T>::value, "T must be derived from IClassHandler");
        BeMutexHolder _(m_mutex);
        auto ecClass = m_ecdb.Schemas().GetClass(classId);
        if (!ecClass) {
            return false;
        }
        m_handlers[classId] = std::make_unique<T>(classId);
        m_handlerMap.clear();
    }
    template <typename T>
    bool RegisterClassHandler(Utf8CP className) {
        static_assert(std::is_base_of<IClassHandler, T>::value, "T must be derived from IClassHandler");
        BeMutexHolder _(m_mutex);
        auto ecClass = m_ecdb.Schemas().FindClass(className);
        if (!ecClass) {
            return false;
        }
        m_handlers.emplace_back(std::make_unique<T>(ecClass->GetId()));
        m_handlerMap.clear();
        return true;
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
