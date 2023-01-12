/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
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
    //=======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======    
    struct PropertyExists final {
        private:
            struct NoCaseAsciiStrHash final {
                size_t operator ()(const std::string& val) const {
                    FNV1HashBuilder builder;
                    builder.UpdateNoCaseAsciiString(val);
                    return static_cast<size_t>(builder.GetHashCode());
                }
                size_t operator ()(const char* val) const {
                    FNV1HashBuilder builder;
                    builder.UpdateNoCaseAsciiCharCP(val);
                    return static_cast<size_t>(builder.GetHashCode());
                }
            };
            struct  NoCaseAsciiStrEqual final {
                bool operator ()(const std::string& lhs,const std::string& rhs ) const {
                    return lhs.size() == rhs.size() && BeStringUtilities::StricmpAscii(lhs.c_str(), rhs.c_str()) == 0;
                }
                bool operator ()(const char* lhs, const char* rhs ) const {
                    return BeStringUtilities::StricmpAscii(lhs, rhs) == 0;
                }
            };
            mutable std::vector<std::unique_ptr<std::string>> m_props;
            mutable std::unordered_map<const char*, std::set<ECN::ECClassId>, NoCaseAsciiStrHash, NoCaseAsciiStrEqual> m_propMap;
            mutable std::set<ECN::ECClassId> m_classIds;
            ECDbCR m_conn;
        public:
            PropertyExists(ECDbCR conn):m_conn(conn){}
            void Clear() const ;
            bool Exists(ECN::ECClassId classId, Utf8CP accessString) const;
    };
    //=======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct Table final {
        using Ptr = std::unique_ptr<Table> ;

        private:
            mutable ECSqlSelectPreparedStatement m_stmt;
            std::map<DbColumnId, int> m_colIndexMap;
            DbTableId m_id;
            int m_ecClassIdCol;

        public:
            explicit Table(ECDbCR conn): m_stmt(conn), m_ecClassIdCol(-1){}
            Table(Table const&) = delete;
            Table& operator =(Table const&) = delete;
            Statement& GetSqliteStmt() const { return m_stmt.GetSqliteStatement(); }
            ECSqlSelectPreparedStatement& GetECSqlStmt() const { return m_stmt;}
            int GetColumnIndexOf(DbColumnId) const;
            int GetColumnIndexOf(DbColumn const& col) const { return GetColumnIndexOf(col.GetId()); }
            size_t GetColumnCount() const { return m_colIndexMap.size(); }
            static Ptr Create(ECDbCR, DbTable const&);
            DbTableId GetId() const { return m_id; }
            bool Seek(ECInstanceId rowId, ECN::ECClassId* classId = nullptr) const;
    };

    //=======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct Property final {
        using Ptr = std::unique_ptr<Property> ;

        private:
            Table const* m_table;
            std::unique_ptr<ECSqlField> m_field;

        public:
            Property(Table const& table, std::unique_ptr<ECSqlField> field);
            Property(Property const&) = delete;
            Utf8StringCR GetName() const { return m_field->GetColumnInfo().GetProperty()->GetName() ;}
            Property& operator = (Property const&) = delete;
            const IECSqlValue& GetValue() const { return *m_field;}
            const Table& GetTable() const {return *m_table; }
            bool Seek(ECInstanceId rowId, ECN::ECClassId& rowClassId) const;
            static Ptr Create(Table const&, std::unique_ptr<ECSqlField>);
    };

    //=======================================================================================
    //! @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct Class final {
        using Ptr = std::unique_ptr<Class> ;
        struct Factory final {
            private:
                static ECSqlPropertyPath GetPropertyPath (PropertyMap const&);
                static std::unique_ptr<ECSqlField> CreatePrimitiveField(ECSqlSelectPreparedStatement&, PropertyMap const&, Table const&);
                static std::unique_ptr<ECSqlField> CreateSystemField(ECSqlSelectPreparedStatement&, PropertyMap const&, Table const&);
                static std::unique_ptr<ECSqlField> CreateStructField(ECSqlSelectPreparedStatement&, PropertyMap const&, Table const&);
                static std::unique_ptr<ECSqlField> CreateNavigationField(ECSqlSelectPreparedStatement&, PropertyMap const&, Table const&);
                static std::unique_ptr<ECSqlField> CreateArrayField(ECSqlSelectPreparedStatement&, PropertyMap const&, Table const&);
                static std::unique_ptr<ECSqlField> CreateField(ECSqlSelectPreparedStatement&, PropertyMap const&, Table const&);
                static std::unique_ptr<ECSqlField> CreateClassIdField(ECSqlSelectPreparedStatement&, PropertyMap const&, ECN::ECClassId, Table const&);

            public:
                static std::vector<Property::Ptr> Create(ClassMapCR, std::function<Table const*(DbTable const&)>);
        };

        private:
            std::vector<Table const*> m_tables;
            std::vector<Property::Ptr> m_properties;
            ECN::ECClassId m_id;

        public:
            Class(ECN::ECClassId, std::vector<Property::Ptr>);
            Class(Class const&) = delete;
            Class& operator =(Class const&) = delete;
            IECSqlValue const& GetValue(int index) const;
            size_t GetPropertyCount() const { return m_properties.size(); }
            Property const* FindProperty(Utf8CP) const;
            ECN::ECClassId GetClassId() const { return m_id; }
            std::vector<Table const*> const& GetTables() const { return m_tables; }
            bool Seek(ECInstanceId rowId, ECN::ECClassId& rowClassId) const;
            static Ptr Create(ECDbCR conn, ECN::ECClassId, std::function<Table const*(DbTable const&)>);
            static Ptr Create(ECDbCR conn, ClassMapCR , std::function<Table const*(DbTable const&)>);
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
            mutable Document m_cachedXmlDoc;
            mutable ECInstanceKey m_instanceKey;
            mutable Utf8String m_accessString;
            mutable InstanceReader::JsonParams m_jsonParam;
            Document& ClearAndGetCachedXmlDocument() const;

        public:
            RowRender(ECDbCR conn):m_conn(conn), m_cachedXmlDoc(&m_allocator, 1024, &m_stackAllocator){
                m_cachedXmlDoc.SetObject();
            }
            BeJsValue GetInstanceJsonObject(ECInstanceKeyCR instanceKey, IECSqlRow const& ecsqlRow, InstanceReader::JsonParams const& param = InstanceReader::JsonParams()) const;
            BeJsValue GetPropertyJsonValue(ECInstanceKeyCR instanceKey, Utf8StringCR accessString, IECSqlValue const& ecsqlValue, InstanceReader::JsonParams const& param = InstanceReader::JsonParams()) const;
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
            mutable Class const* m_class;
            mutable Property const* m_prop;
            mutable Utf8String m_accessString;
            mutable RowRender m_rowRender;
            mutable ECN::ECClassId m_rowClassId;

        public:
            explicit SeekPos(ECDbCR conn):m_rowRender(conn),m_class(nullptr), m_prop(nullptr){}
            bool HasRow() const {return m_rowId.IsValid();}
            bool Seek(ECInstanceId rowId);
            CompareResult Compare(InstanceReader::Position pos);
            ECInstanceId GetRowId() const {return m_rowId;}
            virtual IECSqlValue const& GetValue(int columnIndex) const override;
            virtual int GetColumnCount() const override;
            Class const* GetClass() const {return m_class;}
            Property const* GetProperty() const { return m_prop;}
            bool IsRowOfSubType() const {
                return !m_rowId.IsValid() ||
                       !m_rowClassId.IsValid() ||
                        m_rowClassId == m_class->GetClassId();
            }
            void Reset() const;
            void Reset(Class const& queryClass, Property const& queryProp, Utf8CP accessString) const;
            void Reset(Class const& queryClass) const;
            virtual BeJsValue GetJson(InstanceReader::JsonParams const& param = InstanceReader::JsonParams()) const override;
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
            mutable std::map<ECN::ECClassId, Class::Ptr> m_queryClassMap;
            mutable std::map<DbTableId, Table::Ptr> m_queryTableMap;
            mutable SeekPos m_seekPos;
            mutable LastClassResolved m_lastClassResolved;
            mutable PropertyExists m_propExists;
        private:
            void _OnBeforeClearECDbCache() override { Clear(); }
            void _OnAfterClearECDbCache() override {}
            Table const* GetOrAddTable(DbTableId tableId) const;
            Table const* GetOrAddTable(DbTable const& tbl) const;
            Class const* GetOrAddClass(ECN::ECClassCR ecClass) const;
            Class const* GetOrAddClass(ECN::ECClassId classId) const;
            bool PrepareRowSchema(ECN::ECClassId classId, Utf8CP accessString) const;

        public:
            explicit Reader(ECDbCR conn):m_conn(conn),m_seekPos(conn),m_propExists(conn){}
            ~Reader() { }
            void Clear() const;
            bool Seek(InstanceReader::Position const& position, InstanceReader::RowCallback callback) const;
    };

    private:
        Reader m_reader;
        InstanceReader& m_owner;

    public:
        Impl(InstanceReader& , ECDbCR);
        ~Impl();
        bool Seek(Position const& position, RowCallback callback) const {
            return m_reader.Seek(position, callback);
        }
};
END_BENTLEY_SQLITE_EC_NAMESPACE