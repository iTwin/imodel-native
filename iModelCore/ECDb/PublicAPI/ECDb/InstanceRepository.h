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
#include <ECDb/InstanceWriter.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct InstanceRepository final {
    enum class Operation {
        Insert,
        Update,
        Delete,
        Read
    };
    struct IClassHandler {
        friend struct InstanceRepository;

    private:
        bset<Utf8String> m_customHandledProperties;
        ECN::ECClassId m_classId;
        const ECDb* m_db = nullptr;
        BeJsConst* m_userOptions = nullptr;
        JsFormat format;
        BeJsConst* m_instance = nullptr;
        Operation m_operation;
        Utf8String m_error;
        ECN::ECClassCP m_class = nullptr;
        void SetContext(ECDbCR db, BeJsConst& instance, BeJsConst& userOptions, JsFormat fmt, Operation operation, ECSqlStatementCache& cache);
        void Reset() { m_class = nullptr; }
        ECSqlStatementCache* m_cache = nullptr;
    protected:
        ECN::ECClassId GetClassId() const { return m_classId; }
        ECDbCR GetECDb() const { return *m_db; }
        BeJsConst& GetUserOptions() const { return *m_userOptions; }
        JsFormat GetFormat() const { return format; }
        BeJsConst& GetInstance() const { return *m_instance; }
        Operation GetOperation() const { return m_operation; }
        Utf8StringCR GetError() const { return m_error; }
        ECN::ECClassCR GetClass() const { return *m_class; }
        ECDB_EXPORT ECInstanceKey ParseInstanceKey() const;
        ECDB_EXPORT ECN::ECClassId ParseClassId() const;
        ECDB_EXPORT ECInstanceId ParseInstanceId() const;
        ECDB_EXPORT CachedECSqlStatementPtr GetPreparedStatement(Utf8CP);
        ECDB_EXPORT void SetError(const char* fmt, ...);
        virtual void OnNextId(ECInstanceId&) {};
        virtual PropertyHandlerResult OnBindECProperty(ECN::ECPropertyCR property, BeJsConst val, IECSqlBinder& binder, ECSqlStatus& rc) { return PropertyHandlerResult::Continue; };
        virtual PropertyHandlerResult OnBindUserProperty(Utf8CP property, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) { return PropertyHandlerResult::Continue; };
        virtual PropertyHandlerResult OnReadECProperty(ECN::ECPropertyCR property, const IECSqlValue& valueReader, BeJsValue val, ECSqlStatus& rc) { return PropertyHandlerResult::Continue; };
        virtual ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder finder) { return ECSqlStatus::Success; };
        template <typename T>
        const T& GetDb() const {
            static_assert(std::is_base_of<ECDb, T>::value, "T must be derived from ECDb");
            return *static_cast<const T*>(m_db);
        }
        template <typename T>
        T& GetDbR() const {
            static_assert(std::is_base_of<ECDb, T>::value, "T must be derived from ECDb");
            return *static_cast<T*>(const_cast<ECDb*>(m_db));
        }

    public:
        IClassHandler() = default;
        virtual ~IClassHandler() = default;
    };

private:
    ECDbCR m_ecdb;
    mutable BeMutex m_mutex;
    mutable std::map<ECN::ECClassId, std::vector<IClassHandler*>> m_handlerMap;
    mutable std::vector<std::unique_ptr<IClassHandler>> m_handlers;
    mutable Utf8String m_lastError;
    std::vector<IClassHandler*>& TryGetHandlers(ECN::ECClassId classId) const;
    std::vector<IClassHandler*>& TryGetHandlers(ECN::ECClassId classId, JsFormat fmt, Operation operation, BeJsConst& instance, BeJsConst& userOptions) const;
    InstanceReader m_reader;
    mutable ECSqlStatementCache m_cache;
public:
    InstanceRepository(ECDbCR ecdb) : m_ecdb(ecdb), m_reader(ecdb),m_cache(20) {}
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
    ECDB_EXPORT void Reset(bool clearHandlers = false);
    template <typename T>
    bool RegisterClassHandler(ECN::ECClassId classId, std::vector<Utf8CP> customHandledProperties = {}) {
        static_assert(std::is_base_of<IClassHandler, T>::value, "T must be derived from IClassHandler");
        BeMutexHolder _(m_mutex);
        if (std::find_if(m_handlers.begin(), m_handlers.end(), [&](const auto& handler) { return handler->GetClassId() == classId; }) != m_handlers.end()) {
            return false;
        }

        auto ecClass = m_ecdb.Schemas().GetClass(classId);
        if (!ecClass) {
            return false;
        }
        auto newHandler = std::make_unique<T>();
        newHandler->m_classId = classId;
        for (auto& prop : customHandledProperties) {
            newHandler->m_customHandledProperties.insert(prop);
        }
        m_handlers.push_back(std::move(newHandler));
        m_handlerMap.clear();
        return true;
    }
    template <typename T>
    bool RegisterClassHandler(Utf8CP className, std::vector<Utf8CP> customHandledProperties = {}) {
        auto classP = m_ecdb.Schemas().FindClass(className);
        if (!classP) {
            BeAssert(false && "Class not found");
            return false;
        }
        return RegisterClassHandler<T>(classP->GetId(), customHandledProperties);
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
