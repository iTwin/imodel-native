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
    //! Prefix of the SQLite parameters carrying the "property was written" bitmasks.
    constexpr static Utf8CP kMaskParamPrefix = "_bulkwriter_mask";
    //! Number of root properties covered by a single bitmask parameter.
    constexpr static int kMaskBits = 64;
    //! Fraction of the SQLite variable limit a single statement is allowed to consume.
    constexpr static int kParamBudgetDivisor = 2;

    struct ClassWriter;

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
        //! sqlite parameter index of each bitmask word, empty for insert statements
        std::vector<int> m_maskParamIndices;
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

        std::vector<int> const& GetMaskParamIndices() const { return m_maskParamIndices; }
        void SetMaskParamIndices(std::vector<int> indices) { m_maskParamIndices = std::move(indices); }

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
            static Utf8String MakeMaskParamName(size_t wordIndex);

        public:
            static Ptr Create(ECDbCR, ClassMapCR, WriterOp, Utf8StringR error);
        };

    private:
        ECN::ECClassId m_classId;
        WriterOp m_op;
        std::vector<TableWriter::Ptr> m_tables;
        std::vector<PropertyWriter::Ptr> m_properties;
        //! indexed by PropertyWriter::GetIndex() (the bitmask bit position). May contain holes for
        //! properties that map to no physical column.
        std::vector<PropertyWriter const*> m_propertiesByIndex;
        std::map<Utf8CP, PropertyWriter const*, CompareIUtf8Ascii> m_propertyMap;
        //! binders that need OnBeforeFirstStep()/OnClearBindings(), mirroring ECSqlParameterMap
        std::vector<ECSqlBinder*> m_bindersToCallOnBeforeStep;
        std::vector<ECSqlBinder*> m_bindersToCallOnClearBindings;
        size_t m_maskWordCount = 0;

    public:
        ClassWriter(ECN::ECClassId classId, WriterOp op) : m_classId(classId), m_op(op) {}
        ClassWriter(ClassWriter const&) = delete;
        ClassWriter& operator=(ClassWriter const&) = delete;

        ECN::ECClassId GetClassId() const { return m_classId; }
        WriterOp GetOp() const { return m_op; }
        std::vector<TableWriter::Ptr> const& GetTables() const { return m_tables; }
        std::vector<PropertyWriter::Ptr> const& GetProperties() const { return m_properties; }
        size_t GetMaskWordCount() const { return m_maskWordCount; }
        void SetMaskWordCount(size_t v) { m_maskWordCount = v; }

        PropertyWriter const* FindProperty(Utf8CP name) const;
        int GetPropertyIndex(Utf8CP name) const;
        PropertyWriter const* GetProperty(int index) const;
        //! Size of the property index space. Indices are stable and are also the bitmask bit positions.
        int GetPropertyCount() const { return (int)m_propertiesByIndex.size(); }

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
    //+===============+===============+===============+===============+===============+======
    struct WriteContext final : BulkInstanceWriter::IBindContext {
    private:
        ClassWriter const& m_class;
        mutable std::vector<uint64_t> m_masks;

        IECSqlBinder& _GetBinder(int propertyIndex) const override;
        IECSqlBinder* _FindBinder(Utf8CP propertyName) const override;
        int _GetPropertyCount() const override { return m_class.GetPropertyCount(); }
        int _GetPropertyIndex(Utf8CP propertyName) const override { return m_class.GetPropertyIndex(propertyName); }
        ECN::ECPropertyCP _GetProperty(int propertyIndex) const override;

        void MarkWritten(int propertyIndex) const;

    public:
        explicit WriteContext(ClassWriter const& cls) : m_class(cls), m_masks(cls.GetMaskWordCount(), 0) {}
        WriteContext(WriteContext const&) = delete;
        WriteContext& operator=(WriteContext const&) = delete;

        bool IsWritten(int propertyIndex) const;
        std::vector<uint64_t> const& GetMasks() const { return m_masks; }
        bool HasAnyWritten() const;
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

        public:
            CacheKey(ECN::ECClassId classId, WriterOp op) : m_classId(classId), m_op(op) {}
            bool operator<(CacheKey const& rhs) const {
                return m_classId < rhs.m_classId || (m_classId == rhs.m_classId && (int)m_op < (int)rhs.m_op);
            }
            bool operator==(CacheKey const& rhs) const { return m_classId == rhs.m_classId && m_op == rhs.m_op; }
            ECN::ECClassId GetClassId() const { return m_classId; }
            WriterOp GetOp() const { return m_op; }
        };

    private:
        ECDbCR m_conn;
        mutable BeMutex m_mutex;
        mutable std::map<CacheKey, ClassWriter::Ptr> m_cache;
        mutable std::vector<CacheKey> m_mru;
        mutable Utf8String m_error;
        uint32_t m_maxCache;

        void _OnBeforeClearECDbCache() override { Clear(); }
        void _OnAfterClearECDbCache() override {}

        ClassWriter const* GetOrAdd(ECN::ECClassId classId, WriterOp op) const;
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
        DbResult BindMasks(TableWriter const& table, WriteContext const& ctx) const;
        bool IsTableDirty(TableWriter const& table, WriteContext const& ctx) const;
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
