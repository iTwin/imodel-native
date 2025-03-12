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
        JsFormat m_format;
        ECInstanceKeyCR m_key;
        const BeJsConst& m_instance;

    public:
        Args(ECInstanceKeyCR key, JsFormat format, const BeJsConst& userOptions, const BeJsConst& instance)
            : m_format(format), m_userOptions(userOptions), m_key(key), m_instance(instance) {}
        ECInstanceKeyCR GetKey() const { return m_key; }
        JsFormat GetFormat() const { return m_format; }
        const BeJsConst& GetUserOptions() const { return m_userOptions; }
        BeJsConst GetInstance() const { return m_instance; }
    };

    struct ReadArgs : Args {
    private:
        IECSqlValue const& m_value;
        BeJsValue& m_out;

    public:
        ReadArgs(ECInstanceKeyCR key, JsFormat format, BeJsConst userOptions, IECSqlValue const& value, BeJsValue& out, const BeJsConst& instance)
            : Args(key, format, userOptions, instance), m_value(value), m_out(out) {}
        const IECSqlValue& GetValue() const { return m_value; }
        BeJsValue GetOut() { return m_out; }
        ECN::ECPropertyCR GetProperty() const {
            auto prop = m_value.GetColumnInfo().GetProperty();
            return *prop;
        }
    };

    struct WriteArgs : Args {
    private:
        IECSqlBinder& m_binder;
        const BeJsConst& m_in;
        ECN::ECPropertyCR m_prop;
        bool m_isInsert;

    public:
        WriteArgs(ECInstanceKeyCR key, JsFormat format, BeJsConst userOptions, IECSqlBinder& binder, const BeJsConst& in, ECN::ECPropertyCR prop, const BeJsConst& instance, bool isInsert)
            : Args(key, format, userOptions, instance), m_binder(binder), m_in(in), m_prop(prop) {}
        IECSqlBinder& GetBinder() const { return m_binder; }
        const BeJsConst& GetIn() const { return m_in; }
        ECN::ECPropertyCR GetProperty() const { return m_prop; }
        bool IsInsert() const { return m_isInsert; }
        bool IsUpdate() const { return !m_isInsert; }
    };

    struct DeleteArgs : Args {
    public:
        DeleteArgs(ECInstanceKeyCR key, JsFormat format, BeJsConst userOptions)
            : Args(key, format, userOptions, BeJsDocument::Null()) {}
    };

    struct IClassHandler {
    private:
        ECN::ECClassId m_classId;
        ECDbCR m_db;

    public:
        IClassHandler(ECDbCR db, ECN::ECClassId classId) : m_classId(classId), m_db(db) {}
        ECN::ECClassId GetClassId() const { return m_classId; }
        template <typename T>
        const T& GetDb() const {
            static_assert(std::is_base_of<ECDb, T>::value, "T must be derived from ECDb");
            return static_cast<const T&>(m_db);
        }

        virtual ~IClassHandler() = default;
        virtual PropertyHandlerResult OnNextId(ECInstanceId&) { return PropertyHandlerResult::Continue; };
        virtual PropertyHandlerResult OnBindProperty(WriteArgs&) { return PropertyHandlerResult::Continue; };
        virtual PropertyHandlerResult OnReadProperty(ReadArgs&) { return PropertyHandlerResult::Continue; };
        virtual void OnBeforeInsertInstance(BeJsValue&, const BeJsConst&) {};
        virtual void OnBeforeUpdateInstance(BeJsValue&, const BeJsConst&) {};
        virtual void OnAfterReadInstance(BeJsValue&, const BeJsConst&) {};
    };

    ECDbCR m_ecdb;
    mutable BeMutex m_mutex;
    mutable std::map<ECN::ECClassId, std::vector<IClassHandler*>> m_handlerMap;
    mutable std::vector<std::unique_ptr<IClassHandler>> m_handlers;
    mutable Utf8String m_lastError;
    std::vector<IClassHandler*>& TryGetHandlers(ECN::ECClassId classId) const;

public:
    InstanceRepository(ECDbCR ecdb) : m_ecdb(ecdb) {}
    ~InstanceRepository() = default;
    InstanceRepository(InstanceRepository const&) = delete;
    InstanceRepository(InstanceRepository&&) = delete;
    InstanceRepository& operator=(InstanceRepository const&) = delete;
    InstanceRepository& operator=(InstanceRepository&&) = delete;
    ECDB_EXPORT DbResult Insert(BeJsValue instance, BeJsConst userOptions, JsFormat inFmt, ECInstanceKeyR key) const;
    ECDB_EXPORT DbResult Update(BeJsValue instance, BeJsConst userOptions, JsFormat inFmt) const;
    ECDB_EXPORT DbResult Delete(BeJsConst key, BeJsConst userOptions, JsFormat inFmt) const;
    ECDB_EXPORT DbResult Delete(ECInstanceKeyCR key, BeJsConst userOptions, JsFormat inFmt) const;
    ECDB_EXPORT DbResult Read(BeJsConst key, BeJsValue out, BeJsConst userOptions, JsFormat fmt) const;
    ECDB_EXPORT DbResult Read(ECInstanceKeyCR key, BeJsValue out, BeJsConst userOptions, JsFormat fmt) const;
    Utf8StringCR GetLastError() const { return m_lastError; }
    ECDB_EXPORT void Reset();
    template <typename T>
    bool RegisterClassHandler(ECN::ECClassId classId) {
        static_assert(std::is_base_of<IClassHandler, T>::value, "T must be derived from IClassHandler");
        BeMutexHolder _(m_mutex);
        if (std::find_if(m_handlers.begin(), m_handlers.end(), [&](const auto& handler) { return handler->GetClassId() == classId; }) != m_handlers.end()) {
            return false;
        }

        auto ecClass = m_ecdb.Schemas().GetClass(classId);
        if (!ecClass) {
            return false;
        }
        m_handlers.emplace_back(std::make_unique<T>(m_ecdb, classId));
        m_handlerMap.clear();
        return true;
    }
    template <typename T>
    bool RegisterClassHandler(Utf8CP className) {
        auto classP = m_ecdb.Schemas().FindClass(className);
        if (!classP) {
            return false;
        }
        return RegisterClassHandler<T>(classP->GetId());
    }
    template <typename T>
    bool UnregisterClassHandler(ECN::ECClassId classId) {
        static_assert(std::is_base_of<IClassHandler, T>::value, "T must be derived from IClassHandler");
        BeMutexHolder _(m_mutex);
        auto it = std::find_if(m_handlers.begin(), m_handlers.end(), [&](const auto& handler) { return handler->GetClassId() == classId; });
        if (it == m_handlers.end()) {
            return false;
        }
        m_handlers.erase(it);
        m_handlerMap.clear();
        return true;
    }
    template <typename T>
    bool UnregisterClassHandler(Utf8CP className) {
        auto classP = m_ecdb.Schemas().FindClass(className);
        if (!classP) {
            return false;
        }
        return UnregisterClassHandler<T>(classP->GetId());
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
