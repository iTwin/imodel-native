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
#include <string_view>
#include <unordered_map>
#include <vector>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsistruct
//---------------------------------------------------------------------------------------
struct InstanceWriter::Impl final {
    using InsertOptions = InstanceWriter::InsertOptions;
    using UpdateOptions = InstanceWriter::UpdateOptions;
    using DeleteOptions = InstanceWriter::DeleteOptions;
    using CustomBindHandler = Options::CustomBindHandler;

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
        // Transparent (heterogeneous) hash/equality for the property-name index so that a lookup
        // by Utf8CP does not have to materialize a temporary Utf8String. The global
        // std::hash<Utf8String> specialization additionally copies through c_str(), so the naive
        // lookup allocated twice for every property of every row. That cost dominates bulk writes.
        //---------------------------------------------------------------------------------------
        struct PropertyNameHash final {
            using is_transparent = void;
            size_t operator()(Utf8CP name) const { return std::hash<std::string_view>{}(std::string_view(name == nullptr ? "" : name)); }
            size_t operator()(std::string_view name) const { return std::hash<std::string_view>{}(name); }
            size_t operator()(Utf8StringCR name) const { return std::hash<std::string_view>{}(std::string_view(name.data(), name.size())); }
        };

        struct PropertyNameEqual final {
            using is_transparent = void;
            static std::string_view View(Utf8CP name) { return std::string_view(name == nullptr ? "" : name); }
            static std::string_view View(std::string_view name) { return name; }
            static std::string_view View(Utf8StringCR name) { return std::string_view(name.data(), name.size()); }
            template <typename L, typename R>
            bool operator()(L const& lhs, R const& rhs) const { return View(lhs) == View(rhs); }
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
            std::unordered_map<Utf8String, CachedBinder*, PropertyNameHash, PropertyNameEqual> m_propertyIndexMap;
            void BuildPropertyIndexMap(bool addUseJsNameMap);

        public:
            CachedWriteStatement(ClassMap const& cls) : m_classMap(&cls) {}
            Utf8String GetCurrentTimeStampProperty() const;
            ECClassCR GetClass() const { return m_classMap->GetClass(); }
            ClassMap const& GetClassMap() const { return *m_classMap; }
            ECSqlStatement& GetStatement() { return m_stmt; }
            const CachedBinder* FindBinder(Utf8CP name) const;
            const CachedBinder* FindBinder(Utf8StringCR name) const { return FindBinder(name.c_str()); }
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
        DbResult WithOp(CacheKey key, std::function<DbResult(CachedWriteStatement&)> const& fn);
        SnappyToBlob m_snappyToBlob;
        SnappyFromBlob m_snappyFromBlob;

    public:
        MruStatementCache(ECDbCR ecdb, uint32_t maxCache) : m_ecdb(ecdb), m_maxCache(maxCache) {
            m_mru.reserve(maxCache);
        }
        void Reset();
        ECDbCR GetECDb() const { return m_ecdb; }
        //! The mutex guarding this cache.
        BeMutex& GetMutex() { return m_mutex; }
        DbResult WithInsert(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> fn);
        DbResult WithUpdate(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> fn);
        DbResult WithDelete(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> fn);
        //! Variants that assume the caller already holds GetMutex(). Used by the batch APIs to avoid
        //! re-acquiring the mutex once per row.
        DbResult WithInsertNoLock(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> const& fn) { return WithOp(CacheKey(classId, WriterOp::Insert), fn); }
        DbResult WithUpdateNoLock(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> const& fn) { return WithOp(CacheKey(classId, WriterOp::Update), fn); }
        DbResult WithDeleteNoLock(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> const& fn) { return WithOp(CacheKey(classId, WriterOp::Delete), fn); }
    };
    //---------------------------------------------------------------------------------------
    // @bsistruct
    //---------------------------------------------------------------------------------------
    struct BindContext final {
    private:
        InstanceWriter::Impl& m_writer;
        const Options& m_options;
        Utf8String m_error;
        BeJsConst m_instance;

    public:
        BindContext(InstanceWriter::Impl& writer, BeJsConst instance, Options const& opt) : m_options(opt), m_writer(writer), m_instance(instance) {}
        ~BindContext() {
            if (HasError()) {
                m_writer.m_error = m_error;
                LOG.errorv("InstanceWriter error: %s", m_error.c_str());
            }
        }
        Options const& GetOptions() const { return m_options; }
        Utf8StringCR GetLastError() const { return m_error; }
        ECDbCR GetECDb() const { return m_writer.GetECDb(); }
        bool TryFindClassId(Utf8StringCR name, ECN::ECClassId& id) const { return m_writer.TryFindClassIdForBatch(name, id); }
        bool UseJsNames() const { return m_options.GetUseJsNames(); }
        ECSqlStatus NotifyUserProperty(Utf8CP prop, BeJsConst val, InstanceWriter::Impl::MruStatementCache::CachedWriteStatement& stmt) const;
        void SetError(const char* fmt, ...);
        void PrependError(const char* fmt, ...);
        bool HasError() const { return !m_error.empty(); }
        BeJsConst GetInstance() const { return m_instance; }
    };

private:
    MruStatementCache m_cache;
    Utf8String m_error;
    bool m_inBatch = false;
    std::map<Utf8String, ECN::ECClassId, std::less<>> m_batchClassIds;

    static ECSqlStatus BindDataProperty(BindContext& ctx, ECPropertyCR propMap, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindNavigationProperty(BindContext& ctx, NavigationECPropertyCR prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindPrimitive(BindContext& ctx, PrimitiveType type, IECSqlBinder& binder, BeJsConst val, Utf8CP propertyName, Utf8StringCR extendType);
    static ECSqlStatus BindPrimitiveArrayProperty(BindContext& ctx, PrimitiveArrayECProperty const& prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindPrimitiveProperty(BindContext& ctx, PrimitiveECPropertyCR prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindRootProperty(BindContext& ctx, PropertyMap const& propMap, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindStruct(BindContext& ctx, ECStructClassCR structClass, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindStructArrayProperty(BindContext& ctx, StructArrayECPropertyCR prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindStructProperty(BindContext& ctx, StructECPropertyCR prop, IECSqlBinder& binder, BeJsConst val);
    static ECSqlStatus BindSystemProperty(BindContext& ctx, SystemPropertyMap const& prop, IECSqlBinder& binder, BeJsConst val);

    static bool TryGetECClassId(BindContext& ctx, BeJsConst val, ECClassId& id);
    static bool TryGetECInstanceId(BindContext& ctx, BeJsConst val, ECInstanceId& id);

    DbResult RejectReentrantWrite(Utf8CP opName);
    DbResult InsertNoLock(BeJsConst inst, InsertOptions const& options, ECInstanceKey& key);
    DbResult UpdateNoLock(BeJsConst inst, UpdateOptions const& options);
    DbResult DeleteNoLock(BeJsConst inst, DeleteOptions const& options);
    DbResult DeleteNoLock(ECInstanceKeyCR key, DeleteOptions const& options);
    DbResult RunBatch(BeJsConst instances, Utf8CP opName, std::function<DbResult(BeJsConst)> const& rowFn, int& failedIndex);

public:
    Impl(ECDbCR ecdb, uint32_t cacheSize) : m_cache(ecdb, cacheSize) {}
    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;
    ~Impl() = default;
    ECDbCR GetECDb() const { return m_cache.GetECDb(); }
    bool TryFindClassIdForBatch(Utf8StringCR name, ECN::ECClassId& id) {
        if (m_inBatch) {
            auto it = m_batchClassIds.find(name);
            if (it != m_batchClassIds.end()) {
                id = it->second;
                return true;
            }
        }

        auto classP = GetECDb().Schemas().FindClass(name);
        if (classP == nullptr)
            return false;

        id = classP->GetId();
        if (m_inBatch)
            m_batchClassIds.emplace(name, id);
        return true;
    }
    Utf8StringCR GetLastError() const { return m_error; }
    DbResult Insert(BeJsConst inst, InsertOptions const& options, ECInstanceKey& key);
    DbResult Insert(BeJsConst inst, InsertOptions const& options);
    DbResult Update(BeJsConst inst, UpdateOptions const& options);
    DbResult Delete(BeJsConst inst, DeleteOptions const& options);
    DbResult Delete(ECInstanceKeyCR key, DeleteOptions const& options);

    //! Batch variants. `instances` must be a JSON array. The whole batch runs inside a single
    //! savepoint and a single lock of the statement cache; if any row fails the savepoint is
    //! cancelled so the batch is all-or-nothing. `failedIndex` receives the index of the offending
    //! row on failure (-1 if the failure was not row specific).
    DbResult InsertBatch(BeJsConst instances, InsertOptions const& options, std::vector<ECInstanceKey>* keys, int& failedIndex);
    DbResult UpdateBatch(BeJsConst instances, UpdateOptions const& options, uint64_t& affectedRows, int& failedIndex);
    DbResult DeleteBatch(BeJsConst instances, DeleteOptions const& options, uint64_t& affectedRows, int& failedIndex);

    void ToJson(BeJsValue out, ECInstanceId instanceId, ECClassId classId, JsFormat jsFmt) const;
    void ToJson(BeJsValue out, ECInstanceKeyCR key, JsFormat jsFmt) const;
    bool TryGetId(ECInstanceId& instanceId, BeJsConst in, JsFormat jsFmt = JsFormat::Standard) const;
    bool TryGetClassId(ECClassId& classId, BeJsConst in, JsFormat jsFmt = JsFormat::Standard) const;
    bool TryGetInstanceKey(ECInstanceKeyR key, BeJsConst in, JsFormat jsFmt = JsFormat::Standard) const;
    void Reset();
};

END_BENTLEY_SQLITE_EC_NAMESPACE