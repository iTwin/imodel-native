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
//  TableView   (SELECT per DbTable)  TableWriter  (INSERT/UPDATE per table)
//  Property    (ECSqlField)          PropertyWriter (ECSqlBinder)
//  Class                             ClassWriter (INSERT) / ClassUpdatePlan (UPDATE)
//  SeekPos                           WriteContext
//  Reader                            Writer
//
//  INSERT statements are class specific: the column set and the ECClassId literal depend on
//  the class map, so a ClassWriter is cached per ECClassId.
//
//  UPDATE statements are decomposed by hierarchy level. A level is the ECClass that declares
//  a property, so the writable properties of Foo : Goo : Base partition into a Base, a Goo
//  and a Foo segment, each with its own UPDATE statement. Only the levels a caller actually
//  wrote are stepped, so an update that touches derived properties never rewrites the base
//  columns and never maintains indexes over them.
//
//  Because ClassMap::MapProperties copies inherited property maps verbatim within a table
//  per hierarchy mapping, an inherited property resolves to the same table and the same
//  column in every subclass. A LevelWriter is therefore shareable between sibling classes.
//  Sharing is keyed on the level's declaring class plus the exact column signature of the
//  segment, which makes it correct without having to reason about map strategies: classes
//  that remap a property from scratch produce a different signature and transparently get
//  their own statement.
//
// @bsistruct
//---------------------------------------------------------------------------------------
struct BulkInstanceWriter::Impl final {
    using InsertOptions = BulkInstanceWriter::InsertOptions;
    using UpdateOptions = BulkInstanceWriter::UpdateOptions;

    //! Name of the SQLite parameter carrying the ECInstanceId in every generated statement.
    constexpr static Utf8CP kInstanceIdParamName = "_bulkwriter_id";
    //! Fraction of the SQLite variable limit a single statement is allowed to consume.
    constexpr static int kParamBudgetDivisor = 2;
    //! A class holds one segment per hierarchy level, so the segment cache needs to be a
    //! multiple of the class cache to hold the same number of classes.
    constexpr static uint32_t kLevelCacheFactor = 8;

    //=======================================================================================
    //! The shape of a class: the root properties that can be written, the index each of them
    //! occupies and the hierarchy level each of them belongs to. Cached per class and shared
    //! by the INSERT writer and the UPDATE plan, so the property index space a caller sees
    //! never depends on the operation.
    //+===============+===============+===============+===============+===============+======
    struct ClassSchema final {
        using Ptr = std::shared_ptr<ClassSchema>;

        //! One hierarchy level: the ECClass that declares the properties and their indices.
        struct Level final {
            ECN::ECClassCP m_class = nullptr;
            std::vector<int> m_propertyIndices;
        };

    private:
        ECN::ECClassId m_classId;
        //! root property maps in index order. The class map owns them.
        std::vector<PropertyMap const*> m_propertiesByIndex;
        std::map<Utf8CP, int, CompareIUtf8Ascii> m_indexByName;
        //! levels ordered root -> leaf
        std::vector<Level> m_levels;
        //! level slot of every property index, -1 if the property belongs to no level
        std::vector<int> m_levelByPropertyIndex;

    public:
        explicit ClassSchema(ECN::ECClassId classId) : m_classId(classId) {}
        ClassSchema(ClassSchema const&) = delete;
        ClassSchema& operator=(ClassSchema const&) = delete;

        ECN::ECClassId GetClassId() const { return m_classId; }
        int GetPropertyCount() const { return (int)m_propertiesByIndex.size(); }

        void Add(PropertyMap const& propMap) {
            m_indexByName.insert(std::make_pair(propMap.GetName().c_str(), (int)m_propertiesByIndex.size()));
            m_propertiesByIndex.push_back(&propMap);
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

        //! Groups the properties added so far into levels, ordered root -> leaf.
        void BuildLevels(ECN::ECClassCR concreteClass);

        int GetLevelCount() const { return (int)m_levels.size(); }
        std::vector<Level> const& GetLevels() const { return m_levels; }
        Level const* GetLevel(int slot) const {
            if (slot < 0 || slot >= (int)m_levels.size())
                return nullptr;

            return &m_levels[(size_t)slot];
        }
        ECN::ECClassCP GetLevelClass(int slot) const {
            auto level = GetLevel(slot);
            return level == nullptr ? nullptr : level->m_class;
        }
        int GetPropertyLevel(int propertyIndex) const {
            if (propertyIndex < 0 || propertyIndex >= (int)m_levelByPropertyIndex.size())
                return -1;

            return m_levelByPropertyIndex[(size_t)propertyIndex];
        }
    };

    //=======================================================================================
    //! Owns the raw SQLite statement for one table of one class or level and one operation.
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
    //! One writable root property of a class together with the table it is mapped to and the
    //! index it occupies in the ClassSchema.
    //+===============+===============+===============+===============+===============+======
    struct PendingProperty final {
        PropertyMap const* m_propMap = nullptr;
        DbTable const* m_table = nullptr;
        int m_index = -1;
    };

    //=======================================================================================
    //! Shared helpers that turn a set of PendingProperty into prepared statements and binders.
    //+===============+===============+===============+===============+===============+======
    struct StatementFactory final {
        //! Columns and matching sqlite parameter names of one root property in one table.
        struct ColumnBinding final {
            DbColumn const* m_column = nullptr;
            Utf8String m_paramName;
        };

        static bool IsWritable(PropertyMap const&, Utf8StringCR timestampPropName, WriterOp);
        static bool IsSystemIdPropertyMap(PropertyMap const&);
        static BentleyStatus CollectColumnBindings(std::vector<ColumnBinding>&, PropertyMap const&, DbTable const&, ECSqlBinder const&, WriterOp);
        static DbTable const* GetPropertyTable(ClassMapCR, PropertyMap const&);
        //! Root property maps that the writer may write, in the order that defines their index.
        //! Must produce the exact same order everywhere or indices would shift.
        static void CollectWritableProperties(ClassMapCR, WriterOp, std::vector<PropertyMap const*>&);
        //! Every writable property of the class map paired with its table and schema index.
        static void CollectPendingProperties(ClassMapCR, WriterOp, std::vector<PendingProperty>&);
        //! The class that owns the level @p prop belongs to, in the context of @p concreteClass.
        //! Mixins are never levels: their properties are merged into and mapped by the entity
        //! class that implements them.
        static ECN::ECClassCP GetDeclaringClass(ECN::ECClassCR concreteClass, ECN::ECPropertyCR prop);
        //! Number of steps from @p ecClass to the root of its base class chain.
        static int GetClassDepth(ECN::ECClassCR ecClass);
        //! "table.column," of every non virtual column of @p propMap, in property map order.
        static void AppendColumnSignature(Utf8StringR, ClassMapCR, PropertyMap const&);

        //! Builds one TableWriter per table touched by @p pending plus the matching
        //! PropertyWriters, and prepares the statements.
        static BentleyStatus Build(ECDbCR, ClassMapCR, WriterOp, std::vector<PendingProperty> const& pending,
                                   std::vector<TableWriter::Ptr>& outTables, std::vector<PropertyWriter::Ptr>& outProperties,
                                   Utf8StringR error);
    };

    //=======================================================================================
    //! All statements and binders needed to INSERT one instance of a class.
    //+===============+===============+===============+===============+===============+======
    struct ClassWriter final {
        using Ptr = std::shared_ptr<ClassWriter>;

        struct Factory final {
            static ClassSchema::Ptr CreateSchema(ClassMapCR, WriterOp, Utf8StringR error);
            static Ptr Create(ECDbCR, ClassMapCR, ClassSchema::Ptr, Utf8StringR error);
        };

    private:
        ECN::ECClassId m_classId;
        ClassSchema::Ptr m_schema;
        std::vector<TableWriter::Ptr> m_tables;
        std::vector<PropertyWriter::Ptr> m_properties;
        //! indexed by PropertyWriter::GetIndex(). Holes for properties with no physical column.
        std::vector<PropertyWriter const*> m_propertiesByIndex;
        //! binders that need OnBeforeFirstStep()/OnClearBindings(), mirroring ECSqlParameterMap
        std::vector<ECSqlBinder*> m_bindersToCallOnBeforeStep;
        std::vector<ECSqlBinder*> m_bindersToCallOnClearBindings;

    public:
        ClassWriter(ECN::ECClassId classId, ClassSchema::Ptr schema) : m_classId(classId), m_schema(std::move(schema)) {}
        ClassWriter(ClassWriter const&) = delete;
        ClassWriter& operator=(ClassWriter const&) = delete;

        ECN::ECClassId GetClassId() const { return m_classId; }
        ClassSchema const& GetSchema() const { return *m_schema; }
        std::vector<TableWriter::Ptr> const& GetTables() const { return m_tables; }

        PropertyWriter const* GetProperty(int index) const;

        void SetTables(std::vector<TableWriter::Ptr>&& tables) { m_tables = std::move(tables); }
        void SetProperties(std::vector<PropertyWriter::Ptr>&& properties) { m_properties = std::move(properties); }
        void BuildIndexes();
        void RegisterStepHooks();

        ECSqlStatus OnBeforeFirstStep() const;
        void ResetStatements() const;
    };

    //=======================================================================================
    //! The UPDATE statements of one hierarchy level of one class, i.e. of the properties that
    //! are declared by a single ECClass. Shared between every class that inherits the level's
    //! property maps unchanged, so it is reference counted and never owned by a single plan.
    //+===============+===============+===============+===============+===============+======
    struct LevelWriter final {
        using Ptr = std::shared_ptr<LevelWriter>;

        struct Factory final {
            //! Creates the segment for the properties of one level. @p pending must contain only
            //! properties declared by @p levelClass.
            static Ptr Create(ECDbCR, ClassMapCR, ECN::ECClassCR levelClass, std::vector<PendingProperty> const& pending, Utf8StringR error);
        };

    private:
        ECN::ECClassId m_levelClassId;
        std::vector<TableWriter::Ptr> m_tables;
        std::vector<PropertyWriter::Ptr> m_properties;
        std::map<Utf8CP, PropertyWriter const*, CompareIUtf8Ascii> m_propertiesByName;
        std::vector<ECSqlBinder*> m_bindersToCallOnBeforeStep;
        std::vector<ECSqlBinder*> m_bindersToCallOnClearBindings;

    public:
        explicit LevelWriter(ECN::ECClassId levelClassId) : m_levelClassId(levelClassId) {}
        LevelWriter(LevelWriter const&) = delete;
        LevelWriter& operator=(LevelWriter const&) = delete;

        ECN::ECClassId GetLevelClassId() const { return m_levelClassId; }
        std::vector<TableWriter::Ptr> const& GetTables() const { return m_tables; }
        bool IsEmpty() const { return m_tables.empty(); }

        void SetTables(std::vector<TableWriter::Ptr>&& tables) { m_tables = std::move(tables); }
        void SetProperties(std::vector<PropertyWriter::Ptr>&& properties) { m_properties = std::move(properties); }
        void BuildIndexes();
        void RegisterStepHooks();

        PropertyWriter const* FindProperty(Utf8CP name) const {
            const auto it = m_propertiesByName.find(name);
            return it == m_propertiesByName.end() ? nullptr : it->second;
        }

        ECSqlStatus OnBeforeFirstStep() const;
        void ResetStatements() const;
    };

    //=======================================================================================
    //! Identifies a shareable level segment. The column signature makes two segments equal
    //! only when they render byte identical SQL over the very same columns, which is what
    //! makes sharing between sibling classes sound.
    //+===============+===============+===============+===============+===============+======
    struct LevelKey final {
    private:
        ECN::ECClassId m_levelClassId;
        Utf8String m_columnSignature;

    public:
        LevelKey(ECN::ECClassId levelClassId, Utf8String columnSignature)
            : m_levelClassId(levelClassId), m_columnSignature(std::move(columnSignature)) {}
        bool operator<(LevelKey const& rhs) const {
            if (m_levelClassId != rhs.m_levelClassId)
                return m_levelClassId < rhs.m_levelClassId;

            return m_columnSignature.CompareTo(rhs.m_columnSignature) < 0;
        }
        bool operator==(LevelKey const& rhs) const {
            return m_levelClassId == rhs.m_levelClassId && m_columnSignature.Equals(rhs.m_columnSignature);
        }
        ECN::ECClassId GetLevelClassId() const { return m_levelClassId; }
        Utf8StringCR GetColumnSignature() const { return m_columnSignature; }
    };

    //=======================================================================================
    //! Everything needed to UPDATE one class: its schema and the level segments it is made of,
    //! ordered root -> leaf. Slots are parallel to ClassSchema::GetLevels(); a slot is null
    //! when the level has no physically writable column.
    //+===============+===============+===============+===============+===============+======
    struct ClassUpdatePlan final {
        using Ptr = std::shared_ptr<ClassUpdatePlan>;
        using LevelResolver = std::function<LevelWriter::Ptr(LevelKey const&, ECN::ECClassCR, std::vector<PendingProperty> const&)>;

        struct Factory final {
            static Ptr Create(ECDbCR, ClassMapCR, ClassSchema::Ptr, LevelResolver const&, Utf8StringR error);
        };

    private:
        ECN::ECClassId m_classId;
        ClassSchema::Ptr m_schema;
        std::vector<LevelWriter::Ptr> m_levels;
        //! per property index: the level slot it binds into, -1 if it binds nowhere
        std::vector<int> m_levelSlotByPropertyIndex;
        //! per property index: the binder to hand out, null if the property has no column
        std::vector<PropertyWriter const*> m_propertiesByIndex;

    public:
        ClassUpdatePlan(ECN::ECClassId classId, ClassSchema::Ptr schema) : m_classId(classId), m_schema(std::move(schema)) {}
        ClassUpdatePlan(ClassUpdatePlan const&) = delete;
        ClassUpdatePlan& operator=(ClassUpdatePlan const&) = delete;

        ECN::ECClassId GetClassId() const { return m_classId; }
        ClassSchema const& GetSchema() const { return *m_schema; }
        int GetLevelCount() const { return (int)m_levels.size(); }
        LevelWriter const* GetLevel(int slot) const {
            if (slot < 0 || slot >= (int)m_levels.size())
                return nullptr;

            return m_levels[(size_t)slot].get();
        }

        void SetLevels(std::vector<LevelWriter::Ptr>&& levels) { m_levels = std::move(levels); }
        void BuildIndexes();

        PropertyWriter const* GetProperty(int propertyIndex) const {
            if (propertyIndex < 0 || propertyIndex >= (int)m_propertiesByIndex.size())
                return nullptr;

            return m_propertiesByIndex[(size_t)propertyIndex];
        }
        int GetLevelSlotOf(int propertyIndex) const {
            if (propertyIndex < 0 || propertyIndex >= (int)m_levelSlotByPropertyIndex.size())
                return -1;

            return m_levelSlotByPropertyIndex[(size_t)propertyIndex];
        }

        void ResetStatements() const;
    };

    //=======================================================================================
    //! Handed to the caller during a write. Tracks which hierarchy levels were touched.
    //! Analogue of InstanceReader::Impl::SeekPos.
    //!
    //! The property index space and the name lookup come from the ClassSchema, so a caller
    //! always sees the same indices for a class no matter which operation is running. Because
    //! UPDATE statements no longer depend on the set of written properties, every binder the
    //! context hands out is a real binder and the callback is only ever invoked once.
    //+===============+===============+===============+===============+===============+======
    struct WriteContext final : BulkInstanceWriter::IBindContext {
    private:
        ClassSchema const& m_schema;
        ClassWriter const* m_insertWriter;
        ClassUpdatePlan const* m_plan;
        mutable std::vector<bool> m_dirtyLevels;
        mutable bool m_anyWritten = false;

        IECSqlBinder& _GetBinder(int propertyIndex) const override;
        IECSqlBinder* _FindBinder(Utf8CP propertyName) const override;
        int _GetPropertyCount() const override { return m_schema.GetPropertyCount(); }
        int _GetPropertyIndex(Utf8CP propertyName) const override { return m_schema.GetIndexOf(propertyName); }
        ECN::ECPropertyCP _GetProperty(int propertyIndex) const override;
        int _GetLevelCount() const override { return m_schema.GetLevelCount(); }
        ECN::ECClassCP _GetLevelClass(int levelIndex) const override { return m_schema.GetLevelClass(levelIndex); }
        int _GetPropertyLevel(int propertyIndex) const override { return m_schema.GetPropertyLevel(propertyIndex); }

        void MarkWritten(int propertyIndex) const;

    public:
        //! Context of an INSERT. Every property goes into the one and only statement set.
        WriteContext(ClassSchema const& schema, ClassWriter const& insertWriter)
            : m_schema(schema), m_insertWriter(&insertWriter), m_plan(nullptr) {}
        //! Context of an UPDATE. Binders are routed to the level segment they belong to.
        WriteContext(ClassSchema const& schema, ClassUpdatePlan const& plan)
            : m_schema(schema), m_insertWriter(nullptr), m_plan(&plan), m_dirtyLevels((size_t)plan.GetLevelCount(), false) {}
        WriteContext(WriteContext const&) = delete;
        WriteContext& operator=(WriteContext const&) = delete;

        std::vector<bool> const& GetDirtyLevels() const { return m_dirtyLevels; }
        bool IsLevelDirty(int slot) const { return slot >= 0 && slot < (int)m_dirtyLevels.size() && m_dirtyLevels[(size_t)slot]; }
        bool HasAnyWritten() const { return m_anyWritten; }
    };

    //=======================================================================================
    //! Owns the statement caches. Analogue of InstanceReader::Impl::Reader.
    //+===============+===============+===============+===============+===============+======
    struct Writer final : ECDb::IECDbCacheClearListener {
    private:
        ECDbCR m_conn;
        mutable BeMutex m_mutex;
        mutable std::map<ECN::ECClassId, ClassSchema::Ptr> m_schemaCache;
        mutable std::vector<ECN::ECClassId> m_schemaMru;
        mutable std::map<ECN::ECClassId, ClassWriter::Ptr> m_insertCache;
        mutable std::vector<ECN::ECClassId> m_insertMru;
        mutable std::map<ECN::ECClassId, ClassUpdatePlan::Ptr> m_planCache;
        mutable std::vector<ECN::ECClassId> m_planMru;
        //! shared level segments. Reference counted, so evicting one that a live plan still
        //! uses only stops further sharing, it never invalidates the plan.
        mutable std::map<LevelKey, LevelWriter::Ptr> m_levelCache;
        mutable std::vector<LevelKey> m_levelMru;
        //! guards against a nested write from inside a bind callback, which would corrupt the
        //! bindings of the shared level statements.
        mutable bool m_isWriting = false;
        mutable Utf8String m_error;
        uint32_t m_maxCache;

        void _OnBeforeClearECDbCache() override { Clear(); }
        void _OnAfterClearECDbCache() override {}

        ClassMap const* GetClassMap(ECN::ECClassId classId) const;
        ClassSchema::Ptr GetOrAddSchema(ECN::ECClassId classId, ClassMapCR classMap, WriterOp op) const;
        ClassWriter const* GetOrAddInsertWriter(ECN::ECClassId classId, ClassMapCR classMap) const;
        ClassUpdatePlan const* GetOrAddUpdatePlan(ECN::ECClassId classId, ClassMapCR classMap) const;
        LevelWriter::Ptr GetOrAddLevel(ClassMapCR, LevelKey const&, ECN::ECClassCR levelClass, std::vector<PendingProperty> const&) const;

        template <typename TKey, typename TMap>
        void TouchMru(TMap& cache, std::vector<TKey>& mru, TKey const& key, size_t limit) const {
            auto it = std::find(mru.begin(), mru.end(), key);
            if (it != mru.end())
                mru.erase(it);

            mru.push_back(key);
            while (mru.size() > limit) {
                cache.erase(mru.front());
                mru.erase(mru.begin());
            }
        }

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
