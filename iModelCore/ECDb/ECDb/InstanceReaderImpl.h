/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECSql/ECSqlPrepareContext.h"
#include "ECSql/ECSqlPreparer.h"
#include <ECDb/ECDb.h>
#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include <Bentley/BeTimeUtilities.h>
#include "ECSql/ECSqlPreparedStatement.h"
#include "ECSql/Exp.h"
#include  "ConcurrentQueryManagerImpl.h"
#include "ECDbLogger.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
//  This is high speed instance reader as it does not require preparing sqlite statement.
//  It only once prepare a value reader for a given class but later it only require a seek
//  operation to find the instance and make it available through the reader.
//
// @bsistruct
//---------------------------------------------------------------------------------------
struct InstanceReader::Impl final {
    //! Default number of cached class/property readers before the least recently used ones are dropped.
    constexpr static uint32_t kDefaultCacheSize = 40;

    //! Sorted, duplicate free set of columns a TableView must expose.
    //! An empty filter means "all columns of the table".
    using ColumnFilter = std::vector<DbColumnId>;

    //=======================================================================================
    //! Least recently used cache which never evicts entries that are still referenced by
    //! somebody else. Values are handed out as shared_ptr so an evicted but still used
    //! reader stays alive until its last user is gone.
    //+===============+===============+===============+===============+===============+======
    template <typename TKey, typename TValue>
    struct LruCache final {
        using ValuePtr = std::shared_ptr<TValue>;

        private:
            std::map<TKey, ValuePtr> m_map;
            std::vector<TKey> m_mru; // front = most recently used
            uint32_t m_maxSize;

            void Touch(TKey const& key) {
                const auto it = std::find(m_mru.begin(), m_mru.end(), key);
                if (it != m_mru.end()) {
                    if (it == m_mru.begin())
                        return;
                    m_mru.erase(it);
                }
                m_mru.insert(m_mru.begin(), key);
            }
            //! Drops the least recently used entries which are not referenced anywhere else.
            void Trim() {
                for (auto it = m_mru.end(); it != m_mru.begin() && m_map.size() > m_maxSize;) {
                    --it;
                    const auto mapIt = m_map.find(*it);
                    if (mapIt == m_map.end()) {
                        it = m_mru.erase(it);
                        continue;
                    }
                    if (mapIt->second.use_count() > 1)
                        continue; // still in use, keep it

                    m_map.erase(mapIt);
                    it = m_mru.erase(it);
                }
            }

        public:
            explicit LruCache(uint32_t maxSize) : m_maxSize(maxSize == 0 ? 1 : maxSize) {}
            //! Returns true if the key is cached. Also caches negative results (null values).
            bool TryGet(TKey const& key, ValuePtr& value) {
                const auto it = m_map.find(key);
                if (it == m_map.end())
                    return false;

                Touch(key);
                value = it->second;
                return true;
            }
            ValuePtr Find(TKey const& key) {
                ValuePtr value;
                TryGet(key, value);
                return value;
            }
            ValuePtr Insert(TKey const& key, ValuePtr value) {
                m_map[key] = value;
                Touch(key);
                Trim();
                return value;
            }
            void Clear() {
                m_map.clear();
                m_mru.clear();
            }
            size_t Size() const { return m_map.size(); }
    };

    //=======================================================================================
    //! Identifies a TableView. Two classes mapped to the same table share a TableView only
    //! if they need the exact same set of columns.
    //+===============+===============+===============+===============+===============+======
    struct TableViewKey final {
        private:
            DbTableId m_tableId;
            ColumnFilter m_columns;

        public:
            TableViewKey(DbTableId tableId, ColumnFilter columns) : m_tableId(tableId), m_columns(std::move(columns)) {}
            DbTableId GetTableId() const { return m_tableId; }
            ColumnFilter const& GetColumns() const { return m_columns; }
            bool operator<(TableViewKey const& rhs) const {
                if (m_tableId != rhs.m_tableId)
                    return m_tableId < rhs.m_tableId;
                return m_columns < rhs.m_columns;
            }
            bool operator==(TableViewKey const& rhs) const { return m_tableId == rhs.m_tableId && m_columns == rhs.m_columns; }
    };

    //=======================================================================================
    //! Identifies the reader of a single root property of a class.
    //+===============+===============+===============+===============+===============+======
    struct PropertyKey final {
        private:
            ECN::ECClassId m_classId;
            Utf8String m_accessString;

        public:
            PropertyKey(ECN::ECClassId classId, Utf8CP accessString) : m_classId(classId), m_accessString(accessString) {}
            bool operator<(PropertyKey const& rhs) const {
                if (m_classId != rhs.m_classId)
                    return m_classId < rhs.m_classId;
                return m_accessString.CompareToI(rhs.m_accessString) < 0;
            }
            bool operator==(PropertyKey const& rhs) const {
                return m_classId == rhs.m_classId && m_accessString.EqualsIAscii(rhs.m_accessString);
            }
    };

    //=======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct PropertyExists final {
        private:
            struct Entry {
                ECN::ECClassId m_classId;
                Utf8CP m_accessString;
            };
            struct NoCaseAsciiStrHash final {
                size_t operator ()(const Entry& val) const {
                    FNV1HashBuilder builder;
                    builder.UpdateUInt64(val.m_classId.GetValue());
                    if(val.m_accessString) builder.UpdateNoCaseAsciiCharCP(val.m_accessString);
                    return static_cast<size_t>(builder.GetHashCode());
                }
            };
            struct  NoCaseAsciiStrEqual final {
                bool operator ()(const Entry& lhs,const Entry& rhs ) const {
                    return (lhs.m_classId == rhs.m_classId)
                        && ((lhs.m_accessString == nullptr) != (rhs.m_accessString == nullptr))
                        && (rhs.m_accessString == nullptr || BeStringUtilities::StricmpAscii(lhs.m_accessString, rhs.m_accessString) == 0);
                }
            };
            mutable std::vector<std::unique_ptr<Utf8String>> m_props;
            mutable std::unordered_set<Entry, NoCaseAsciiStrHash, NoCaseAsciiStrEqual> m_propHashTable;
            ECDbCR m_conn;

        public:
            PropertyExists(ECDbCR conn):m_conn(conn){}
            void Clear() const;
            void Load() const;
            bool Exists(ECN::ECClassId classId, Utf8CP accessString) const;
            bool Exists(ECN::ECClassId classId) const { return Exists(classId, nullptr); }
    };
    //=======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct TableView final {
        using Ptr = std::shared_ptr<TableView> ;

        private:
            mutable ECSqlSelectPreparedStatement m_stmt;
            std::map<DbColumnId, int> m_colIndexMap;
            DbTableId m_id;
            int m_ecClassIdCol;
            int m_ecSourceClassIdCol;
            int m_ecTargetClassIdCol;
            static Ptr CreateNullTableView(ECDbCR, DbTable const&);
            static Ptr CreateTableView(ECDbCR, DbTable const&, ColumnFilter const&);
            static Ptr CreateLinkTableView(ECDbCR, DbTable const&, RelationshipClassLinkTableMap const&, ColumnFilter const&);
            static Ptr CreateEntityTableView(ECDbCR, DbTable const&, ClassMapCR, ColumnFilter const&);
        public:
            explicit TableView(ECDbCR conn): m_stmt(conn), m_ecClassIdCol(-1),m_ecSourceClassIdCol(-1),m_ecTargetClassIdCol(-1) {}
            TableView(TableView const&) = delete;
            TableView& operator =(TableView const&) = delete;
            Statement& GetSqliteStmt() const { return m_stmt.GetSqliteStatement(); }
            ECSqlSelectPreparedStatement& GetECSqlStmt() const { return m_stmt;}
            int GetColumnIndexOf(DbColumnId) const;
            int GetColumnIndexOf(DbColumn const& col) const { return GetColumnIndexOf(col.GetId()); }
            int GetClassIdCol() const { return m_ecClassIdCol; }
            int GetSourceClassIdCol() const { return m_ecSourceClassIdCol; }
            int GetTargetClassIdCol() const { return m_ecTargetClassIdCol; }

            size_t GetColumnCount() const { return m_colIndexMap.size(); }
            //! Creates a view over the given table exposing only the columns in the filter.
            //! An empty filter exposes every (non virtual) column of the table.
            static Ptr Create(ECDbCR, DbTable const&, ColumnFilter const& filter = ColumnFilter());
            DbTableId GetId() const { return m_id; }
            bool Seek(ECInstanceId rowId, ECN::ECClassId* classId = nullptr) const;
    };

    //======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct Property final {
        using Ptr = std::shared_ptr<Property> ;

        private:
            TableView::Ptr m_table;
            std::unique_ptr<ECSqlField> m_field;

        public:
            Property(TableView::Ptr table, std::unique_ptr<ECSqlField> field);
            Property(Property const&) = delete;
            Utf8StringCR GetName() const { return m_field->GetColumnInfo().GetProperty()->GetName() ;}
            Property& operator = (Property const&) = delete;
            const IECSqlValue& GetValue() const { return *m_field;}
            const TableView& GetTable() const {return *m_table; }
            TableView::Ptr const& GetTablePtr() const { return m_table; }
            ECSqlField& GetField() const { return *m_field; }
            bool Seek(ECInstanceId rowId, ECN::ECClassId& rowClassId) const;
            static Ptr Create(TableView::Ptr, std::unique_ptr<ECSqlField>);
            ECSqlStatus OnAfterStep() const { return m_field->OnAfterStep(); }
            ECSqlStatus OnAfterReset() const { return m_field->OnAfterReset(); }
    };

    //=======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct Class final {
        using Ptr = std::shared_ptr<Class> ;
        using GetTableFunc = std::function<TableView::Ptr(DbTable const&, ColumnFilter const&)>;

        struct Factory final {
            private:
                static DateTime::Info GetDateTimeInfo(PropertyMap const& propertyMap);
                static ECSqlPropertyPath GetPropertyPath (PropertyMap const&);
                static std::unique_ptr<ECSqlField> CreatePrimitiveField(ECSqlSelectPreparedStatement&, PropertyMap const&, TableView const&);
                static std::unique_ptr<ECSqlField> CreateSystemField(ECSqlSelectPreparedStatement&, PropertyMap const&, TableView const&);
                static std::unique_ptr<ECSqlField> CreateStructField(ECSqlSelectPreparedStatement&, PropertyMap const&, TableView const&);
                static std::unique_ptr<ECSqlField> CreateNavigationField(ECSqlSelectPreparedStatement&, PropertyMap const&, TableView const&);
                static std::unique_ptr<ECSqlField> CreateArrayField(ECSqlSelectPreparedStatement&, PropertyMap const&, TableView const&);
                static std::unique_ptr<ECSqlField> CreateField(ECSqlSelectPreparedStatement&, PropertyMap const&, TableView const&);
                static std::unique_ptr<ECSqlField> CreateClassIdField(ECSqlSelectPreparedStatement&, PropertyMap const&, ECN::ECClassId, TableView const&);

            public:
                //! Table a root property map is read from. Mirrors the mapping used when the fields are built.
                static DbTable const* GetPropertyTable(ClassMapCR, PropertyMap const&);
                //! Columns of the given class which live in the given table.
                static ColumnFilter CollectColumns(ClassMapCR, DbTable const&);
                //! Columns of a single root property map which live in the given table.
                static ColumnFilter CollectColumns(PropertyMap const&, DbTable const&);
                static std::vector<Property::Ptr> Create(ClassMapCR, GetTableFunc const&);
                //! Builds a reader for a single root property using a table view restricted to that property.
                static Property::Ptr CreateSingle(ClassMapCR, Utf8CP accessString, GetTableFunc const&);
        };

        private:
            std::vector<TableView::Ptr> m_tables;
            std::vector<Property::Ptr> m_properties;
            //! subset of m_properties whose fields need the step/reset notifications
            std::vector<Property const*> m_propertiesRequiringOnAfterStep;
            std::vector<Property const*> m_propertiesRequiringOnAfterReset;
            std::map<Utf8CP,  Property const*, CompareIUtf8Ascii> m_propertyMap;
            ECN::ECClassId m_id;

        public:
            Class(ECN::ECClassId, std::vector<Property::Ptr>);
            Class(Class const&) = delete;
            Class& operator =(Class const&) = delete;
            IECSqlValue const& GetValue(int index) const;
            size_t GetPropertyCount() const { return m_properties.size(); }
            Property const* FindProperty(Utf8CP) const;
            ECN::ECClassId GetClassId() const { return m_id; }
            std::vector<TableView::Ptr> const& GetTables() const { return m_tables; }
            bool Seek(ECInstanceId rowId, ECN::ECClassId& rowClassId) const;
            static Ptr Create(ECDbCR conn, ECN::ECClassId, GetTableFunc const&);
            static Ptr Create(ECDbCR conn, ClassMapCR , GetTableFunc const&);
    };

    //=======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct RowRender {
        using CrtAllocator = rapidjson::CrtAllocator;
        using MemoryPoolAllocator = rapidjson::MemoryPoolAllocator<CrtAllocator>;
        using Document = rapidjson::Document;

        private:
            ECDbCR m_conn;
            mutable MemoryPoolAllocator m_allocator;
            mutable CrtAllocator m_stackAllocator;
            mutable Document m_cachedJsonDoc;
            mutable ECInstanceKey m_instanceKey;
            mutable Utf8String m_accessString;
            mutable JsReadOptions m_jsonParam;
            Document& ClearAndGetCachedJsonDocument() const;

        public:
            RowRender(ECDbCR conn):m_conn(conn), m_cachedJsonDoc(&m_allocator, 1024, &m_stackAllocator){
                m_cachedJsonDoc.SetObject();
            }
            BeJsValue GetInstanceJsonObject(ECInstanceKeyCR instanceKey, IECSqlRow const& ecsqlRow, JsReadOptions const& param = JsReadOptions()) const;
            BeJsValue GetPropertyJsonValue(ECInstanceKeyCR instanceKey, Utf8StringCR accessString, IECSqlValue const& ecsqlValue, JsReadOptions const& param = JsReadOptions()) const;
            void Reset();
    };

    //=======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct SeekPos final : InstanceReader::IRowContext {
        enum class CompareResult {
            SameSchema,
            SameRowAndSchema,
            None,
        };

        private:
            mutable ECInstanceId m_rowId;
            mutable Class::Ptr m_class;
            mutable Property::Ptr m_prop;
            mutable Utf8String m_accessString;
            mutable RowRender m_rowRender;
            mutable ECN::ECClassId m_rowClassId;

        public:
            explicit SeekPos(ECDbCR conn):m_rowRender(conn){}
            bool HasRow() const {return m_rowId.IsValid();}
            bool Seek(ECInstanceId rowId);
            CompareResult Compare(InstanceReader::Position pos);
            ECInstanceId GetRowId() const {return m_rowId;}
            virtual IECSqlValue const& GetValue(int columnIndex) const override;
            virtual int GetColumnCount() const override;
            Class const* GetClass() const {return m_class.get();}
            Property const* GetProperty() const { return m_prop.get();}
            bool IsRowOfSubType() const {
                return !m_rowId.IsValid() ||
                       !m_rowClassId.IsValid() ||
                        m_rowClassId == m_class->GetClassId();
            }
            void Reset() const;
            void Reset(Class::Ptr queryClass, Property::Ptr queryProp, Utf8CP accessString) const;
            void Reset(Class::Ptr queryClass) const;
            virtual BeJsValue GetJson(JsReadOptions const& param = JsReadOptions()) const override;
    };

    //=======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct Reader final :  ECDb::IECDbCacheClearListener{
        private:
            struct LastClassResolved {
                Utf8String m_className;
                ECN::ECClassId m_classId;
            };
            ECDbCR m_conn;
            mutable BeMutex m_mutex;
            mutable LruCache<ECN::ECClassId, Class> m_queryClassMap;
            mutable LruCache<TableViewKey, TableView> m_queryTableMap;
            mutable LruCache<PropertyKey, Property> m_queryPropMap;
            mutable SeekPos m_seekPos;
            mutable LastClassResolved m_lastClassResolved;
            mutable PropertyExists m_propExists;
        private:
            void _OnBeforeClearECDbCache() override { Clear(); }
            void _OnAfterClearECDbCache() override {}
            TableView::Ptr GetOrAddTable(DbTable const& tbl, ColumnFilter const& filter) const;
            Class::Ptr GetOrAddClass(ECN::ECClassCR ecClass) const;
            Class::Ptr GetOrAddClass(ECN::ECClassId classId) const;
            Property::Ptr GetOrAddProperty(ECN::ECClassId classId, Utf8CP accessString) const;
            bool PrepareRowSchema(ECN::ECClassId classId, Utf8CP accessString) const;

        public:
            explicit Reader(ECDbCR conn, uint32_t cacheSize)
                :m_conn(conn), m_queryClassMap(cacheSize), m_queryTableMap(cacheSize * 2), m_queryPropMap(cacheSize * 2),
                 m_seekPos(conn), m_propExists(conn){}
            ~Reader() { }
            void Clear() const;
            bool Seek(InstanceReader::Position const& position, InstanceReader::RowCallback callback, InstanceReader::Options const& options) const;
            void InvalidateSeekPos(ECInstanceKey const& key);
    };

    private:
        Reader m_reader;
        InstanceReader& m_owner;

    public:
        Impl(InstanceReader& , ECDbCR, uint32_t cacheSize);
        ~Impl();
        bool Seek(Position const& position, RowCallback callback, InstanceReader::Options const& options) const {
            return m_reader.Seek(position, callback, options);
        }
        void Reset() { m_reader.Clear(); }
        void InvalidateSeekPos(ECInstanceKey const& key) { m_reader.InvalidateSeekPos(key); }
};
END_BENTLEY_SQLITE_EC_NAMESPACE