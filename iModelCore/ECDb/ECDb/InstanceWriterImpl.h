/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include "ClassMap.h"
#include "ECDbLogger.h"
#include "PropertyMap.h"
#include <ECDb/ECDb.h>
#include <ECDb/ECSqlStatement.h>
#include <ECDb/IECSqlBinder.h>
#include <ECDb/InstanceWriter.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace Internal {
    struct CachedBinder final {
    private:
        PropertyMap const* m_prop = nullptr;
        IECSqlBinder* m_binder = nullptr;

    public:
        CachedBinder(PropertyMap const& prop, IECSqlBinder& binder) : m_prop(&prop), m_binder(&binder) {}
        CachedBinder(CachedBinder const&) = default;
        CachedBinder(CachedBinder&&) = default;
        CachedBinder& operator=(CachedBinder const&) = default;
        CachedBinder& operator=(CachedBinder&&) = default;
        ECPropertyCR GetProperty() const { return m_prop->GetProperty(); }
        PropertyMap const& GetPropertyMap() const { return *m_prop; }
        IECSqlBinder& GetBinder() const { return *m_binder; }
    };
    enum class WriterOp {
        Insert,
        Update,
        Delete,
    };

    struct StatementMruCache;
    struct CachedStatement final {
        friend struct StatementMruCache;
        using BinderList = std::vector<CachedBinder>;

    private:
        ClassMap const* m_classMap;
        ECSqlStatement m_stmt;
        BinderList m_propertyBinders = {};
        int m_instanceIdIndex = -1;

        // for now us hash table
        std::unordered_map<Utf8String, CachedBinder*> m_propertyIndexMap;
        void BuildPropertyIndexMap(bool addUseJsNameMap);

    public:
        CachedStatement(ClassMap const& cls) : m_classMap(&cls) {}
        Utf8String GetCurrentTimeStampProperty() const;
        ECClassCR GetClass() const { return m_classMap->GetClass(); }
        ClassMap const& GetClassMap() const { return *m_classMap; }
        ECSqlStatement& GetStatement() { return m_stmt; }
        const CachedBinder* FindBinder(Utf8StringCR name) const;
        const std::vector<CachedBinder>& GetBinders() const { return m_propertyBinders; }
        std::vector<CachedBinder>& GetBinders() { return m_propertyBinders; }
        int GetInstanceIdParameterIndex() const { return m_instanceIdIndex; }
    };

    struct CacheKey final {
    private:
        ECClassId m_classId;
        WriterOp m_op;

    public:
        CacheKey(ECClassId classId, WriterOp op) : m_classId(classId), m_op(op) {}
        CacheKey(CacheKey const&) = default;
        CacheKey(CacheKey&&) = default;
        CacheKey& operator=(CacheKey const&) = default;
        CacheKey& operator=(CacheKey&&) = default;
        ECClassId GetClassId() const { return m_classId; }
        WriterOp GetOp() const { return m_op; }

        bool operator()(CacheKey const& rhs) const {
            return m_classId < rhs.m_classId || (m_classId == rhs.m_classId && (int)m_op < (int)rhs.m_op);
        }
        bool operator<(CacheKey const& rhs) const {
            return m_classId < rhs.m_classId || (m_classId == rhs.m_classId && (int)m_op < (int)rhs.m_op);
        }
        bool operator>(CacheKey const& rhs) const {
            return m_classId > rhs.m_classId || (m_classId == rhs.m_classId && (int)m_op > (int)rhs.m_op);
        }
        bool operator==(CacheKey const& rhs) const {
            return m_classId == rhs.m_classId && m_op == rhs.m_op;
        }
        bool operator<=(CacheKey const& rhs) const {
            return *this < rhs || *this == rhs;
        }
        bool operator>=(CacheKey const& rhs) const {
            return *this > rhs || *this == rhs;
        }
        bool operator!=(CacheKey const& rhs) const {
            return !(*this == rhs);
        }
    };

    struct StatementMruCache final {
    private:
        std::map<CacheKey, std::unique_ptr<CachedStatement>> m_cache;
        std::vector<CacheKey> m_mru;
        ECDbCR m_ecdb;
        BeMutex m_mutex;
        uint32_t m_maxCache;
        bool m_addSupportForJsName = true;
        ECSqlStatus PrepareInsert(CachedStatement& cachedStmt);
        ECSqlStatus PrepareUpdate(CachedStatement& cachedStmt);
        ECSqlStatus PrepareDelete(CachedStatement& cachedStmt);
        std::unique_ptr<CachedStatement> Prepare(CacheKey key);
        CachedStatement* TryGet(CacheKey key);

    public:
        StatementMruCache(ECDbCR ecdb, uint32_t maxCache) : m_ecdb(ecdb), m_maxCache(maxCache) {
            m_mru.reserve(maxCache);
        }
        void Reset();
        ECDbCR GetECDb() const { return m_ecdb; }
        DbResult WithInsert(ECClassId classId, std::function<DbResult(CachedStatement&)> fn);
        DbResult WithUpdate(ECClassId classId, std::function<DbResult(CachedStatement&)> fn);
        DbResult WithDelete(ECClassId classId, std::function<DbResult(CachedStatement&)> fn);
    };
};
//---------------------------------------------------------------------------------------
// @bsistruct
//---------------------------------------------------------------------------------------
struct InstanceWriter::Impl final {
    using InsertOptions = InstanceWriter::InsertOptions;
    using UpdateOptions = InstanceWriter::UpdateOptions;
    using DeleteOptions = InstanceWriter::DeleteOptions;
    using UnknownJsPropertyHandler = BaseInsertOrUpdateOptions::UnknownJsPropertyHandler;
    using Abortable = InstanceWriter::Abortable;

    struct Context final {
    private:
        Internal::WriterOp m_op;
        ECDbCR m_ecdb;
        const InsertOptions& m_insertOptions = InsertOptions();
        const UpdateOptions& m_updateOptions = UpdateOptions();
        const DeleteOptions& m_deleteOptions = DeleteOptions();
        Utf8StringR m_error;

    public:
        Context(ECDbCR ecdb, InsertOptions const& opt, Utf8StringR err) : m_op(Internal::WriterOp::Insert), m_insertOptions(opt), m_ecdb(ecdb), m_error(err) {}
        Context(ECDbCR ecdb, UpdateOptions const& opt, Utf8StringR err) : m_op(Internal::WriterOp::Update), m_updateOptions(opt), m_ecdb(ecdb), m_error(err) {}
        Context(ECDbCR ecdb, DeleteOptions const& opt, Utf8StringR err) : m_op(Internal::WriterOp::Delete), m_deleteOptions(opt), m_ecdb(ecdb), m_error(err) {}
        Internal::WriterOp GetOp() const { return m_op; }
        bool IsInsert() const { return m_op == Internal::WriterOp::Insert; }
        bool IsUpdate() const { return m_op == Internal::WriterOp::Update; }
        bool IsDelete() const { return m_op == Internal::WriterOp::Delete; }
        Utf8StringCR GetLastError() const { return m_error; }
        const InsertOptions& GetInsertOptions() const {
            BeAssert(IsInsert());
            return m_insertOptions;
        }
        const UpdateOptions& GetUpdateOptions() const {
            BeAssert(IsUpdate());
            return m_updateOptions;
        }
        const DeleteOptions& GetDeleteOptions() const {
            BeAssert(IsDelete());
            return m_deleteOptions;
        }
        ECDbCR GetECDb() const { return m_ecdb; }
        ECN::ECClassCP FindClass(Utf8StringCR name) const {
            return m_ecdb.Schemas().FindClass(name);
        }
        bool IsUseJsName() const {
            if (IsInsert()) {
                return m_insertOptions.GetUseJsName();
            }
            if (IsUpdate()) {
                return m_updateOptions.GetUseJsName();
            }
            return false;
        }
        Abortable NotifyUnknownJsProperty(Utf8CP prop, BeJsConst val) const {
            if (IsInsert() && m_insertOptions.GetUnknownJsPropertyHandler() != nullptr) {
                m_insertOptions.GetUnknownJsPropertyHandler()(prop, val);
            }
            if (IsUpdate() && m_updateOptions.GetUnknownJsPropertyHandler() != nullptr) {
                m_updateOptions.GetUnknownJsPropertyHandler()(prop, val);
            }
            return Abortable::Continue;
        }
        void SetError(const char* fmt, ...) {
            va_list args;
            va_start(args, fmt);
            m_error.VSprintf(fmt, args);
            va_end(args);
        }
    };

private:
    Internal::StatementMruCache m_cache;
    Utf8String m_error;

    static ECSqlStatus BindDataProperty(Context& ctx, ECPropertyCR propMap, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindNavigationProperty(Context& ctx, NavigationECPropertyCR const& prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindPrimitive(Context& ctx, PrimitiveType type, IECSqlBinder& binder, BeJsConst val, Utf8CP propertyName, Utf8StringCR extendType);
    static ECSqlStatus BindPrimitiveArrayProperty(Context& ctx, PrimitiveArrayECProperty const& prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindPrimitiveProperty(Context& ctx, PrimitiveECPropertyCR prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindRootProperty(Context& ctx, PropertyMap const& propMap, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindStruct(Context& ctx, ECStructClassCR structClass, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindStructArrayProperty(Context& ctx, StructArrayECPropertyCR const& prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindStructProperty(Context& ctx, StructECPropertyCR prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindSystemProperty(Context& ctx, SystemPropertyMap const& prop, IECSqlBinder& binder, BeJsConst val);

    static bool TryGetECClassId(Context& ctx, BeJsConst val, ECClassId& id);
    static bool TryGetECInstanceId(Context& ctx, BeJsConst val, ECInstanceId& id);

public:
    Impl(ECDbCR ecdb, uint32_t cacheSize) : m_cache(ecdb, cacheSize) {}
    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;
    ~Impl() = default;
    Utf8StringCR GetLastError() const { return m_error; }
    DbResult Insert(BeJsConst inst, InsertOptions const& options,  ECInstanceKey& key);
    DbResult Insert(BeJsConst inst, InsertOptions const& options);
    DbResult Update(BeJsConst inst, UpdateOptions const& options);
    DbResult Delete(BeJsConst inst, DeleteOptions const& options);
    void Reset();
};

END_BENTLEY_SQLITE_EC_NAMESPACE