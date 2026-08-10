/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ClassMap.h"
#include "ECDbLogger.h"
#include "PropertyMap.h"
#include "PropertyMapVisitor.h"
#include "ECSql/ECSqlBinder.h"
#include "ECSql/ECSqlPrepareContext.h"
#include "ECSql/ECSqlPreparedStatement.h"
#include "ECSql/NativeSqlBuilder.h"
#include <ECDb/BulkInstanceWriter.h>
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
//  High speed bulk instance writer. Mirrors InstanceReader::Impl:
//
//  InstanceReader::Impl              BulkInstanceWriter::Impl
//  --------------------------------  --------------------------------
//  TableView   (SELECT per DbTable)  TableWriter  (INSERT/UPDATE per class+table+op)
//  Property    (ECSqlField)          PropertyWriter (ECSqlBinder)
//  Class                             ClassWriter
//  SeekPos                           WriteContext
//  Reader                            Writer
//
//  Unlike the reader, statements are class specific (the column set and the ECClassId
//  literal depend on the class map), so they are cached per (ECClassId, WriterOp).
//
// @bsistruct
//---------------------------------------------------------------------------------------
struct BulkInstanceWriter::Impl final {
    using InsertOptions = BulkInstanceWriter::InsertOptions;
    using UpdateOptions = BulkInstanceWriter::UpdateOptions;

    //! Name of the SQLite parameter carrying the ECInstanceId in every generated statement.
    constexpr static Utf8CP kInstanceIdParamName = "_bulkwriter_id";
    //! Number of root properties covered by a single bitmask word.
    constexpr static int kMaskBits = 64;
    //! Fraction of the SQLite variable limit a single statement is allowed to consume.
    constexpr static int kParamBudgetDivisor = 2;

    //! Set of root property indices, one bit per index. UPDATE statements are specialized
    //! for the exact set of properties the caller writes, so that a partial update of a
    //! wide class only touches the columns it actually writes.
    using PropertyMask = std::vector<uint64_t>;

    static bool IsMaskBitSet(PropertyMask const& mask, int index) {
        if (index < 0)
            return false;

        const auto word = (size_t)(index / kMaskBits);
        return word < mask.size() && (mask[word] & ((uint64_t)1 << (uint64_t)(index % kMaskBits))) != 0;
    }

    static void SetMaskBit(PropertyMask& mask, int index) {
        if (index < 0)
            return;

        const auto word = (size_t)(index / kMaskBits);
        if (word >= mask.size())
            return;

        mask[word] |= (uint64_t)1 << (uint64_t)(index % kMaskBits);
    }

    static bool IsMaskEmpty(PropertyMask const& mask) {
        for (auto word : mask) {
            if (word != 0)
                return false;
        }
        return true;
    }

    struct ClassWriter;

    //=======================================================================================
    //! The mask independent shape of a class: the root properties that can be written and
    //! the index each of them occupies. Indices are also the bit positions of PropertyMask.
    //! Shared by every mask specialization of a class so that the property index space a
    //! caller sees never depends on which statement happens to be cached.
    //+===============+===============+===============+===============+===============+======
    struct ClassSchema final {
        using Ptr = std::shared_ptr<ClassSchema>;

    private:
        ECN::ECClassId m_classId;
        //! root property maps in index order. The class map owns them.
        std::vector<PropertyMap const*> m_propertiesByIndex;
        std::map<Utf8CP, int, CompareIUtf8Ascii> m_indexByName;
        size_t m_maskWordCount = 0;

    public:
        explicit ClassSchema(ECN::ECClassId classId) : m_classId(classId) {}
        ClassSchema(ClassSchema const&) = delete;
        ClassSchema& operator=(ClassSchema const&) = delete;

        ECN::ECClassId GetClassId() const { return m_classId; }
        int GetPropertyCount() const { return (int)m_propertiesByIndex.size(); }
        size_t GetMaskWordCount() const { return m_maskWordCount; }
        PropertyMask MakeEmptyMask() const { return PropertyMask(m_maskWordCount, 0); }
        //! Mask with every property selected, used by full updates.
        PropertyMask MakeFullMask() const {
            PropertyMask mask(m_maskWordCount, 0);
            for (int i = 0; i < GetPropertyCount(); ++i)
                Impl::SetMaskBit(mask, i);

            return mask;
        }

        void Add(PropertyMap const& propMap) {
            m_indexByName.insert(std::make_pair(propMap.GetName().c_str(), (int)m_propertiesByIndex.size()));
            m_propertiesByIndex.push_back(&propMap);
            m_maskWordCount = (m_propertiesByIndex.size() + (size_t)kMaskBits - 1) / (size_t)kMaskBits;
        }

        int GetIndexOf(Utf8CP name) const {
            if (name == nullptr)
                return -1;

            const auto it = m_indexByName.find(name);
            return it == m_indexByName.end() ? -1 : it->second;
        }

        PropertyMap const* GetPropertyMap(int index) const {
            if (index < 0 || index >= (int)m_propertiesByIndex.size())
                return nullptr;

            return m_propertiesByIndex[(size_t)index];
        }
    };

    //=======================================================================================
    //! Owns the raw SQLite statement for one table of one class and one operation.
    //! Analogue of InstanceReader::Impl::TableView.
    //+===============+===============+===============+===============+===============+======
    struct TableWriter final {
        using Ptr = std::unique_ptr<TableWriter>;

    private:
        mutable SingleContextTableECSqlPreparedStatement m_stmt;
        DbTable const* m_table;
        WriterOp m_op;
        //! index of the ECInstanceId sqlite parameter, -1 if the statement has none
        int m_instanceIdParamIndex = -1;
        //! root property indices whose columns are written by this statement
        std::vector<int> m_propertyIndices;
        //! true if this statement writes at least one non system column
        bool m_hasDataColumns = false;

    public:
        TableWriter(ECDbCR ecdb, DbTable const& table, WriterOp op)
            : m_stmt(ecdb, op == WriterOp::Insert ? ECSqlType::Insert : ECSqlType::Update, table), m_table(&table), m_op(op) {}
        TableWriter(TableWriter const&) = delete;
        TableWriter& operator=(TableWriter const&) = delete;

        SingleContextTableECSqlPreparedStatement& GetECSqlStmt() const { return m_stmt; }
        Statement& GetSqliteStmt() const { return m_stmt.GetSqliteStatement(); }
        DbTable const& GetTable() const { return *m_table; }
        WriterOp GetOp() const { return m_op; }

        int GetInstanceIdParamIndex() const { return m_instanceIdParamIndex; }
        void SetInstanceIdParamIndex(int ix) { m_instanceIdParamIndex = ix; }

        std::vector<int> const& GetPropertyIndices() const { return m_propertyIndices; }
        void AddPropertyIndex(int ix) { m_propertyIndices.push_back(ix); }

        bool HasDataColumns() const { return m_hasDataColumns; }
        void SetHasDataColumns(bool v) { m_hasDataColumns = v; }

        DbResult Prepare(ECDbCR ecdb, Utf8StringCR sql);
        void Reset() const;
    };

    //=======================================================================================
    //! Binder for a single root property together with the statement it belongs to.
    //! Analogue of InstanceReader::Impl::Property.
    //+===============+===============+===============+===============+===============+======
    struct PropertyWriter final {
        using Ptr = std::unique_ptr<PropertyWriter>;

    private:
        TableWriter const* m_table;
        PropertyMap const* m_propertyMap;
        std::unique_ptr<ECSqlBinder> m_binder;
        int m_index;

    public:
        PropertyWriter(TableWriter const& table, PropertyMap const& propertyMap, std::unique_ptr<ECSqlBinder> binder, int index)
            : m_table(&table), m_propertyMap(&propertyMap), m_binder(std::move(binder)), m_index(index) {}
        PropertyWriter(PropertyWriter const&) = delete;
        PropertyWriter& operator=(PropertyWriter const&) = delete;

        Utf8StringCR GetName() const { return m_propertyMap->GetName(); }
        ECN::ECPropertyCR GetProperty() const { return m_propertyMap->GetProperty(); }
        PropertyMap const& GetPropertyMap() const { return *m_propertyMap; }
        TableWriter const& GetTable() const { return *m_table; }
        ECSqlBinder& GetBinder() const { return *m_binder; }
        int GetIndex() const { return m_index; }
    };

    //=======================================================================================
    //! All statements and binders needed to write one class with one operation.
    //! Analogue of InstanceReader::Impl::Class.
    //+===============+===============+===============+===============+===============+======
    struct ClassWriter final {
        using Ptr = std::unique_ptr<ClassWriter>;

        //=======================================================================================
        //! Builds the per table SQL and the binders. Analogue of Class::Factory.
        //+===============+===============+===============+===============+===============+======
        struct Factory final {
        private:
            //! Columns and matching sqlite parameter names of one root property in one table.
            struct ColumnBinding final {
                DbColumn const* m_column = nullptr;
                Utf8String m_paramName;
            };

            static bool IsWritable(PropertyMap const&, Utf8StringCR timestampPropName, WriterOp);
            static bool IsSystemIdPropertyMap(PropertyMap const&);
            static BentleyStatus CollectColumnBindings(std::vector<ColumnBinding>&, PropertyMap const&, DbTable const&, ECSqlBinder const&, WriterOp);
            static DbTable const* GetPropertyTable(ClassMapCR, PropertyMap const&);

        public:
            //! Root property maps that the writer may write, in the order that defines their index.
            //! Must produce the exact same order for CreateSchema and Create or indices would shift.
            static void CollectWritableProperties(ClassMapCR, WriterOp, std::vector<PropertyMap const*>&);
            static ClassSchema::Ptr CreateSchema(ClassMapCR, WriterOp, Utf8StringR error);
            //! Creates the statements for exactly the properties selected by @p mask. For Insert the
            //! mask is ignored, an INSERT always writes every column of the class.
            static Ptr Create(ECDbCR, ClassMapCR, WriterOp, ClassSchema::Ptr, PropertyMask const& mask, Utf8StringR error);
        };

    private:
        ECN::ECClassId m_classId;
        WriterOp m_op;
        ClassSchema::Ptr m_schema;
        //! the property set this specialization writes. Empty (all bits clear) for Insert.
        PropertyMask m_mask;
        std::vector<TableWriter::Ptr> m_tables;
        std::vector<PropertyWriter::Ptr> m_properties;
        //! indexed by PropertyWriter::GetIndex() (the bitmask bit position). Holes for properties
        //! that map to no physical column or that this specialization does not write.
        std::vector<PropertyWriter const*> m_propertiesByIndex;
        //! binders that need OnBeforeFirstStep()/OnClearBindings(), mirroring ECSqlParameterMap
        std::vector<ECSqlBinder*> m_bindersToCallOnBeforeStep;
        std::vector<ECSqlBinder*> m_bindersToCallOnClearBindings;

    public:
        ClassWriter(ECN::ECClassId classId, WriterOp op, ClassSchema::Ptr schema, PropertyMask mask)
            : m_classId(classId), m_op(op), m_schema(std::move(schema)), m_mask(std::move(mask)) {}
        ClassWriter(ClassWriter const&) = delete;
        ClassWriter& operator=(ClassWriter const&) = delete;

        ECN::ECClassId GetClassId() const { return m_classId; }
        WriterOp GetOp() const { return m_op; }
        ClassSchema const& GetSchema() const { return *m_schema; }
        PropertyMask const& GetMask() const { return m_mask; }
        std::vector<TableWriter::Ptr> const& GetTables() const { return m_tables; }
        std::vector<PropertyWriter::Ptr> const& GetProperties() const { return m_properties; }

        PropertyWriter const* GetProperty(int index) const;

        TableWriter& AddTable(TableWriter::Ptr table);
        PropertyWriter& AddProperty(PropertyWriter::Ptr property);
        void BuildIndexes();
        void RegisterStepHooks();

        ECSqlStatus OnBeforeFirstStep() const;
        void OnClearBindings() const;
        void ResetStatements() const;
    };

    //=======================================================================================
    //! Handed to the caller during a write. Tracks which properties were written.
    //! Analogue of InstanceReader::Impl::SeekPos.
    //!
    //! The property index space and the name lookup come from the ClassSchema, so they are
    //! identical no matter which mask specialization (if any) is backing the context. A
    //! context without a ClassWriter is a discovery context: it hands out no-op binders and
    //! is only used to learn which properties a callback writes.
    //+===============+===============+===============+===============+===============+======
    struct WriteContext final : BulkInstanceWriter::IBindContext {
    private:
        ClassSchema const& m_schema;
        ClassWriter const* m_class;
        mutable PropertyMask m_mask;

        IECSqlBinder& _GetBinder(int propertyIndex) const override;
        IECSqlBinder* _FindBinder(Utf8CP propertyName) const override;
        int _GetPropertyCount() const override { return m_schema.GetPropertyCount(); }
        int _GetPropertyIndex(Utf8CP propertyName) const override { return m_schema.GetIndexOf(propertyName); }
        ECN::ECPropertyCP _GetProperty(int propertyIndex) const override;

        void MarkWritten(int propertyIndex) const { Impl::SetMaskBit(m_mask, propertyIndex); }

    public:
        explicit WriteContext(ClassSchema const& schema, ClassWriter const* cls = nullptr)
            : m_schema(schema), m_class(cls), m_mask(schema.MakeEmptyMask()) {}
        WriteContext(WriteContext const&) = delete;
        WriteContext& operator=(WriteContext const&) = delete;

        bool IsWritten(int propertyIndex) const { return Impl::IsMaskBitSet(m_mask, propertyIndex); }
        PropertyMask const& GetMask() const { return m_mask; }
        bool HasAnyWritten() const { return !Impl::IsMaskEmpty(m_mask); }
    };

    //=======================================================================================
    //! Owns the ClassWriter cache. Analogue of InstanceReader::Impl::Reader.
    //+===============+===============+===============+===============+===============+======
    struct Writer final : ECDb::IECDbCacheClearListener {
        //=======================================================================================
        //+===============+===============+===============+===============+===============+======
        struct CacheKey final {
        private:
            ECN::ECClassId m_classId;
            WriterOp m_op;
            PropertyMask m_mask;

        public:
            CacheKey(ECN::ECClassId classId, WriterOp op, PropertyMask mask) : m_classId(classId), m_op(op), m_mask(std::move(mask)) {}
            bool operator<(CacheKey const& rhs) const {
                if (m_classId != rhs.m_classId)
                    return m_classId < rhs.m_classId;
                if (m_op != rhs.m_op)
                    return (int)m_op < (int)rhs.m_op;
                return m_mask < rhs.m_mask;
            }
            bool operator==(CacheKey const& rhs) const { return m_classId == rhs.m_classId && m_op == rhs.m_op && m_mask == rhs.m_mask; }
            ECN::ECClassId GetClassId() const { return m_classId; }
            WriterOp GetOp() const { return m_op; }
            PropertyMask const& GetMask() const { return m_mask; }
        };

    private:
        ECDbCR m_conn;
        mutable BeMutex m_mutex;
        mutable std::map<CacheKey, ClassWriter::Ptr> m_cache;
        mutable std::vector<CacheKey> m_mru;
        mutable std::map<ECN::ECClassId, ClassSchema::Ptr> m_schemaCache;
        mutable std::vector<ECN::ECClassId> m_schemaMru;
        //! The property set the last Update of a class wrote. Callers typically write the very
        //! same set over and over, so this lets the next Update bind straight into the matching
        //! statement instead of running a separate discovery pass.
        mutable std::map<ECN::ECClassId, PropertyMask> m_lastUpdateMask;
        mutable Utf8String m_error;
        uint32_t m_maxCache;

        void _OnBeforeClearECDbCache() override { Clear(); }
        void _OnAfterClearECDbCache() override {}

        ClassMap const* GetClassMap(ECN::ECClassId classId) const;
        ClassSchema::Ptr GetOrAddSchema(ECN::ECClassId classId, ClassMapCR classMap, WriterOp op) const;
        ClassWriter const* GetOrAdd(ECN::ECClassId classId, WriterOp op, PropertyMask const& mask) const;
        void TouchMru(CacheKey const& key) const;

    public:
        Writer(ECDbCR conn, uint32_t maxCache);
        ~Writer();

        ECDbCR GetECDb() const { return m_conn; }
        BeMutex& GetMutex() const { return m_mutex; }
        Utf8StringCR GetLastError() const { return m_error; }
        void SetError(Utf8CP fmt, ...) const;
        void ClearError() const { m_error.clear(); }
        void Clear() const;

        DbResult Insert(ECN::ECClassId classId, BindCallback const& callback, InsertOptions const& options, ECInstanceKey& key) const;
        DbResult Update(ECInstanceKeyCR key, BindCallback const& callback, UpdateOptions const& options) const;

    private:
        DbResult CheckWritePermission() const;
    };

private:
    Writer m_writer;

public:
    Impl(ECDbCR ecdb, uint32_t cacheSize) : m_writer(ecdb, cacheSize) {}
    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;
    ~Impl() = default;

    ECDbCR GetECDb() const { return m_writer.GetECDb(); }
    Utf8StringCR GetLastError() const { return m_writer.GetLastError(); }

    DbResult Insert(ECN::ECClassId classId, BindCallback const& callback, InsertOptions const& options, ECInstanceKey& key) {
        return m_writer.Insert(classId, callback, options, key);
    }
    DbResult Update(ECInstanceKeyCR key, BindCallback const& callback, UpdateOptions const& options) {
        return m_writer.Update(key, callback, options);
    }
    void Reset() { m_writer.Clear(); }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
