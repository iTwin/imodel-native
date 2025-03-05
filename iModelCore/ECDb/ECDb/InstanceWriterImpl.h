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

//---------------------------------------------------------------------------------------
// @bsistruct
//---------------------------------------------------------------------------------------
struct InstanceWriter::Impl final {
    using InsertOptions = InstanceWriter::InsertOptions;
    using UpdateOptions = InstanceWriter::UpdateOptions;
    using DeleteOptions = InstanceWriter::DeleteOptions;
    using Abortable = InstanceWriter::Abortable;
    using UnknownJsPropertyHandler = BaseInsertOrUpdateOptions::UnknownJsPropertyHandler;
    enum class WriterOp {
        Insert,
        Update,
        Delete,
    };
    //---------------------------------------------------------------------------------------
    // @bsistruct
    //---------------------------------------------------------------------------------------
    struct MruStatementCache final {
        //---------------------------------------------------------------------------------------
        // @bsistruct
        //---------------------------------------------------------------------------------------
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
        //---------------------------------------------------------------------------------------
        // @bsistruct
        //---------------------------------------------------------------------------------------
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

        //---------------------------------------------------------------------------------------
        // @bsistruct
        //---------------------------------------------------------------------------------------
        struct CachedWriteStatement final {
            friend struct MruStatementCache;
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
            CachedWriteStatement(ClassMap const& cls) : m_classMap(&cls) {}
            Utf8String GetCurrentTimeStampProperty() const;
            ECClassCR GetClass() const { return m_classMap->GetClass(); }
            ClassMap const& GetClassMap() const { return *m_classMap; }
            ECSqlStatement& GetStatement() { return m_stmt; }
            const CachedBinder* FindBinder(Utf8StringCR name) const;
            const std::vector<CachedBinder>& GetBinders() const { return m_propertyBinders; }
            std::vector<CachedBinder>& GetBinders() { return m_propertyBinders; }
            int GetInstanceIdParameterIndex() const { return m_instanceIdIndex; }
        };

    private:
        std::map<CacheKey, std::unique_ptr<CachedWriteStatement>> m_cache;
        std::vector<CacheKey> m_mru;
        ECDbCR m_ecdb;
        BeMutex m_mutex;
        uint32_t m_maxCache;
        bool m_addSupportForJsName = true;
        ECSqlStatus PrepareInsert(CachedWriteStatement& cachedStmt);
        ECSqlStatus PrepareUpdate(CachedWriteStatement& cachedStmt);
        ECSqlStatus PrepareDelete(CachedWriteStatement& cachedStmt);
        std::unique_ptr<CachedWriteStatement> Prepare(CacheKey key);
        CachedWriteStatement* TryGet(CacheKey key);
        SnappyToBlob m_snappyToBlob;
        SnappyFromBlob m_snappyFromBlob;

    public:
        MruStatementCache(ECDbCR ecdb, uint32_t maxCache) : m_ecdb(ecdb), m_maxCache(maxCache) {
            m_mru.reserve(maxCache);
        }
        void Reset();
        ECDbCR GetECDb() const { return m_ecdb; }
        DbResult WithInsert(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> fn);
        DbResult WithUpdate(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> fn);
        DbResult WithDelete(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> fn);
    };
    //---------------------------------------------------------------------------------------
    // @bsistruct
    //---------------------------------------------------------------------------------------
    struct BindContext final {
    private:
        WriterOp m_op;
        InstanceWriter::Impl& m_writer;
        const InsertOptions& m_insertOptions = InsertOptions();
        const UpdateOptions& m_updateOptions = UpdateOptions();
        const DeleteOptions& m_deleteOptions = DeleteOptions();
        Utf8String m_error;

    public:
        BindContext(InstanceWriter::Impl& writer, InsertOptions const& opt) : m_op(WriterOp::Insert), m_insertOptions(opt), m_writer(writer) {}
        BindContext(InstanceWriter::Impl& writer, UpdateOptions const& opt) : m_op(WriterOp::Update), m_updateOptions(opt), m_writer(writer) {}
        BindContext(InstanceWriter::Impl& writer, DeleteOptions const& opt) : m_op(WriterOp::Delete), m_deleteOptions(opt), m_writer(writer) {}
        ~BindContext() {
            if (HasError()) {
                m_writer.m_error = m_error;
                LOG.errorv("InstanceWriter error: %s", m_error.c_str());
            }
        }
        WriterOp GetOp() const { return m_op; }
        bool IsInsert() const { return m_op == WriterOp::Insert; }
        bool IsUpdate() const { return m_op == WriterOp::Update; }
        bool IsDelete() const { return m_op == WriterOp::Delete; }
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
        ECDbCR GetECDb() const { return m_writer.GetECDb(); }
        ECN::ECClassCP FindClass(Utf8StringCR name) const {
            return m_writer.GetECDb().Schemas().FindClass(name);
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
        void SetError(const char* fmt, ...);
        bool HasError() const { return !m_error.empty(); }
    };

private:
    MruStatementCache m_cache;
    Utf8String m_error;

    static ECSqlStatus BindDataProperty(BindContext& ctx, ECPropertyCR propMap, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindNavigationProperty(BindContext& ctx, NavigationECPropertyCR const& prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindPrimitive(BindContext& ctx, PrimitiveType type, IECSqlBinder& binder, BeJsConst val, Utf8CP propertyName, Utf8StringCR extendType);
    static ECSqlStatus BindPrimitiveArrayProperty(BindContext& ctx, PrimitiveArrayECProperty const& prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindPrimitiveProperty(BindContext& ctx, PrimitiveECPropertyCR prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindRootProperty(BindContext& ctx, PropertyMap const& propMap, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindStruct(BindContext& ctx, ECStructClassCR structClass, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindStructArrayProperty(BindContext& ctx, StructArrayECPropertyCR const& prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindStructProperty(BindContext& ctx, StructECPropertyCR prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindSystemProperty(BindContext& ctx, SystemPropertyMap const& prop, IECSqlBinder& binder, BeJsConst val);

    static bool TryGetECClassId(BindContext& ctx, BeJsConst val, ECClassId& id);
    static bool TryGetECInstanceId(BindContext& ctx, BeJsConst val, ECInstanceId& id);

public:
    Impl(ECDbCR ecdb, uint32_t cacheSize) : m_cache(ecdb, cacheSize) {}
    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;
    ~Impl() = default;
    ECDbCR GetECDb() const { return m_cache.GetECDb(); }
    Utf8StringCR GetLastError() const { return m_error; }
    DbResult Insert(BeJsConst inst, InsertOptions const& options, ECInstanceKey& key);
    DbResult Insert(BeJsConst inst, InsertOptions const& options);
    DbResult Update(BeJsConst inst, UpdateOptions const& options);
    DbResult Delete(BeJsConst inst, DeleteOptions const& options);

    ECDB_EXPORT void ToJson(BeJsValue out, ECInstanceId instanceId, ECClassId classId, bool useJsName = false) const;
    ECDB_EXPORT void ToJson(BeJsValue out, ECInstanceKeyCR key, bool useJsName = false) const;
    void Reset();
};

END_BENTLEY_SQLITE_EC_NAMESPACE