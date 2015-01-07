/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSql.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"
#include <BeXml/BeXml.h>
#include <unordered_map>
#include <unordered_set>
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


#define ECDBSQL_VERSION_MAJOR 1
#define ECDBSQL_VERSION_MINOR 0
#define ECDBSQL_NULLTABLENAME "ECDbNotMapped"
#define ECDBSQL_PROPERTYNAME "ECDbSqlSchema"
#define ECDBSQL_NAMESPACE "ECDb"
#define ECDBSQL_SCHEMA_ID "ECDbPersistence"

enum class PersistenceType
    {
    Persisted, //! Persisted in DB
    Virtual //! Not persisted in db rather used as a view specification
    };

enum class OwnerType
    {
    ECDb, //! owned by ECDb.
    ExistingTable //! existing table
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct EditHandle : NonCopyableClass
    {
    private:
        bool m_canEdit : 1;
        bool m_isModified : 2;

    public:
        EditHandle ();
        bool BeginEdit ();
        bool IsModified () const;
        bool SetModified ();
        bool EndEdit ();
        bool CanEdit () const;
        bool AssertNotInEditMode ();
        bool AssertInEditMode () const;
        EditHandle (EditHandle const &&  eh);
        ~EditHandle ();
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct Identity
    {
    typedef uint32_t Id;
    private:
        Id m_id;
    protected:
        void SetId (Id id){ m_id = id; }
    public:
        Identity (uint32_t id) : m_id (id){}

        Id GetId () const { return m_id; }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
enum class ECDbSqlTypeAffinity
    {
    Integer, Real, Numeric, None, Text
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct NameGenerator
    {
    private:
        int m_uniqueIdGenerator;
        Utf8String m_format;

    public:
        NameGenerator (Utf8CP format = "ecdb_%s")
            :m_format (format), m_uniqueIdGenerator (1)
            {}

        void Generate (Utf8StringR generatedName)
            {
            generatedName.clear ();
            generatedName.Sprintf (m_format.c_str (), m_uniqueIdGenerator++);
            }
        ~NameGenerator () {}
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct StringPool : NonCopyableClass
    {
    private:
        std::set<Utf8CP, CompareUtf8> m_lookUp;

    public:
        StringPool ()
            {
            //Reserve (1024);
            }
        ~StringPool ()
            {
            for (auto str : m_lookUp)
                {
                free (const_cast<Utf8P>(str));
                }

            m_lookUp.clear ();
            }
        Utf8CP Set (Utf8CP str);
        bool Exists (Utf8CP accessString) const;
        Utf8CP Get (Utf8CP str) const;
    };

struct ECDbSqlColumn;
struct ECDbSqlTable;
struct ECDbSqlIndex;
struct ECDbVersion
    {
    private:
        uint32_t m_versionMajor, m_versionMinor;
    public:
        explicit ECDbVersion (uint32_t versionMajor, uint32_t versionMinor)
            : m_versionMajor (versionMajor), m_versionMinor (versionMinor)
            {
            }
        explicit ECDbVersion (uint32_t versionMajor)
            : m_versionMajor (versionMajor), m_versionMinor (0)
            {
            }
        explicit ECDbVersion ()
            : m_versionMajor (1), m_versionMinor (0)
            {
            }
        uint32_t GetVersionMajor () const { return m_versionMajor; }
        uint32_t GetVersionMinor () const{ return m_versionMinor; }
        Utf8String ToString () const 
            {
            Utf8String str;
            str.Sprintf ("%d.%d", m_versionMajor, m_versionMinor);
            return str;
            }
        static ECDbVersion Parse (Utf8CP version) 
            {
            ECDbVersion ver;
            sscanf (version, "%d.%d", &ver.m_versionMajor, &ver.m_versionMinor);
            return ver;
            }
    };
//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlDb : NonCopyableClass
    {
    //struct InstanceDependency : NonCopyableClass
    //    {
    //    private:
    //        std::set<ECDbSqlTable const*> m_tables;
    //        ECDbSqlTable const& m_primary;
    //    public:
    //        InstanceDependency (ECDbSqlTable const& primaryTable)
    //            :m_primary (primaryTable)
    //            {
    //            Add (primaryTable);
    //            }
    //        ~InstanceDependency (){}
    //        std::set<ECDbSqlTable const*> const& GetTables () const { return m_tables; }
    //        BentleyStatus Add (ECDbSqlTable const& table)
    //            {
    //            m_tables.insert (&table);
    //            }
    //        ECDbSqlTable const& GetPrimary () const { return m_primary; }
    //        bool Constains (ECDbSqlTable const& table) const { return m_tables.find (&table) != m_tables.end (); }
    //    };

    //struct ClassDependency : NonCopyableClass
    //    {
    //    private:
    //    ECN::ECClassId m_ecClassId;
    //    std::vector<std::unique_ptr<InstanceDependency>> m_instanceDependencyList;

    //    public:
    //        ClassDependency (ECN::ECClassId ecClassId)
    //            : m_ecClassId(ecClassId) 
    //            {
    //            }
    //        ~ClassDependency (){}
    //        ECN::ECClassId GetClassId () const { return m_ecClassId; }
    //        InstanceDependency* Find (ECDbSqlTable const& primaryTable)
    //            {
    //            for (auto& item : m_instanceDependencyList)
    //                {
    //                if (&item->GetPrimary () == &primaryTable)
    //                    return item.get ();
    //                }

    //            return nullptr;
    //            }
    //        InstanceDependency& Add (ECDbSqlTable const& primaryTable)
    //            {
    //            auto idp = Find (primaryTable);
    //            if (idp == nullptr)
    //                {
    //                idp = new InstanceDependency (primaryTable);
    //                m_instanceDependencyList.push_back (std::unique_ptr<InstanceDependency> (idp));
    //                }
    //            return *idp;
    //            }
    //        InstanceDependency& Add (ECDbSqlTable const& primaryTable, ECDbSqlTable const& secondaryTable)
    //            {
    //            auto idp = Find (primaryTable);
    //            if (idp == nullptr)
    //                {
    //                idp = new InstanceDependency (primaryTable);
    //                m_instanceDependencyList.push_back (std::unique_ptr<InstanceDependency> (idp));
    //                }

    //            idp->Add (secondaryTable);
    //            return *idp;
    //            }
    //        std::vector<InstanceDependency const*> const& GetInstanceDependencyList () const
    //            { 
    //            std::vector<InstanceDependency const*> tmp;
    //            for (auto& item : m_instanceDependencyList)
    //                {
    //                tmp.push_back(item.get());
    //                }
    //            return tmp;
    //            }
    //    };

    //struct DependencyManager
    //    {
    //    private:
    //        std::map<ECN::ECClassId, std::unique_ptr<ClassDependency>> m_dependency;
    //        ECDbSqlDb& m_ecdbSqlDb;
    //    public:
    //        DependencyManager (ECDbSqlDb& ecdbSqlDb) : m_ecdbSqlDb (ecdbSqlDb){}
    //        ~DependencyManager (){}
    //        ECDbSqlDb const& GetSqlDb () const { return m_ecdbSqlDb; }
    //        ECDbSqlDb & GetSqlDbR () const { return m_ecdbSqlDb; }
    //        ClassDependency& Add (ECN::ECClassId ecClassId, ECDbSqlTable const& primaryTable)
    //            {
    //            auto cd = Find (ecClassId);
    //            if (cd == nullptr)
    //                {
    //                cd = new ClassDependency (ecClassId);
    //                m_dependency[ecClassId] = (std::unique_ptr<ClassDependency> (cd));
    //                }

    //            cd->Add (primaryTable);
    //            return *cd;
    //            }
    //        ClassDependency& Add (ECN::ECClassId ecClassId, ECDbSqlTable const& primaryTable, ECDbSqlTable const& secondaryTable)
    //            {
    //            auto cd = Find (ecClassId);
    //            if (cd == nullptr)
    //                {
    //                cd = new ClassDependency (ecClassId);
    //                m_dependency[ecClassId] = (std::unique_ptr<ClassDependency> (cd));
    //                }

    //            cd->Add (primaryTable, secondaryTable);
    //            return *cd;
    //            }
    //        ClassDependency* Find (ECN::ECClassId ecClassId) const
    //            {
    //            auto itor = m_dependency.find (ecClassId);
    //            if (itor != m_dependency.end ())
    //                return itor->second.get();

    //            return nullptr;
    //            }
    //        std::vector<ClassDependency const*> const& GetClassDependencyList () const
    //            {
    //            std::vector<ClassDependency const*> tmp;
    //            for (auto& kvp : m_dependency)
    //                {
    //                tmp.push_back (kvp.second.get ());
    //                }

    //            return tmp;
    //            }
    //    };

    private:
        NameGenerator m_nameGenerator;
        std::map<Utf8CP, std::unique_ptr<ECDbSqlTable>, CompareIUtf8> m_tables;
        std::map<Utf8CP, std::unique_ptr<ECDbSqlIndex>, CompareIUtf8> m_index;

        //std::map<ECN::ECClassId, ClassReference>, CompareIUtf8> m_index;

        bool HasObject (Utf8CP name);
        uint32_t m_idGenerator;
        Utf8String m_name;
        ECDbVersion m_version;
        StringPool m_stringPool;
    private:
        uint32_t GenerateId () { m_idGenerator = m_idGenerator + 1; return m_idGenerator; }
    public:
        ECDbSqlDb (Utf8CP name, ECDbVersion ver)
            : m_nameGenerator ("ECDbObj_%03d"), m_idGenerator (0), m_name (name), m_version (ver)
            {
            }
        ECDbSqlDb ()
            : m_nameGenerator ("ECDbObj_%03d"), m_idGenerator (0), m_name ("ECDb"), m_version (ECDbVersion (1, 0))
            {
            }
        Utf8StringCR GetName () const { return m_name; }
        //! Create a table with a given name or if name is null a name will be generated
        ECDbSqlTable* CreateTable (Utf8CP name, PersistenceType type = PersistenceType::Persisted);
        ECDbSqlTable* CreateTableUsingExistingTableDefinition (ECDbCR ecdb, Utf8CP existingTableName);
        ECDbSqlTable* CreateTableUsingExistingTableDefinition (Utf8CP existingTableName);
        ECDbVersion const& GetVersion () const { return m_version; }
        //! Find a table with a given name
        ECDbSqlTable const* FindTable (Utf8CP name) const;
        ECDbSqlTable* FindTableP (Utf8CP name) const;
        ECDbSqlIndex const* FindIndex (Utf8CP name) const;
        ECDbSqlIndex* FindIndexP (Utf8CP name) ;
        ECDbSqlIndex* CreateIndex (Utf8CP tableName, Utf8CP indexName);
        const std::vector<ECDbSqlIndex const*> GetIndexes () const;
        std::vector<ECDbSqlIndex*> GetIndexesR ();
        const std::vector<ECDbSqlTable const*> GetTables () const;
        virtual  ~ECDbSqlDb () {}
        BentleyStatus WriteTo (BeXmlDom& xmlDom) const;
        BentleyStatus WriteTo (Utf8StringR xml) const;
        StringPool const& GetStringPool () const { return m_stringPool; }
        StringPool& GetStringPoolR ()  { return m_stringPool; }
        BentleyStatus DropIndex (Utf8CP name);
        BentleyStatus DropTable (Utf8CP name);
        static BentleyStatus ReadFrom (ECDbSqlDb& ecdbSqlDb, BeXmlDomR xmlDom);
        static BentleyStatus ReadFrom (ECDbSqlDb& ecdbSqlDb, Utf8StringCR xml);
        bool IsModified () const;
        void Reset ();        
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlIndex : NonCopyableClass, Identity
    {
    friend ECDbSqlIndex* ECDbSqlDb::CreateIndex (Utf8CP tableName, Utf8CP indexName);
    struct PersistenceManager : NonCopyableClass
        {
        private:
            ECDbSqlIndex const& m_index;
        public:
            PersistenceManager (ECDbSqlIndex const& table)
                :m_index (table)
                {
                }
            ~PersistenceManager (){}
            ECDbSqlIndex const& GetIndex () const{ return m_index; }
            BentleyStatus Create (ECDbR ecdb);
            BentleyStatus Drop (ECDbR ecdb);
            bool Exist (ECDbR ecdb) const;
        };
    private:
        Utf8String m_whereExpression;
        Utf8String m_name;
        ECDbSqlTable& m_table;
        std::vector<ECDbSqlColumn const*> m_columns;
        bool m_isUnique;
        PersistenceManager m_persistenceManager;

    private:
        ECDbSqlIndex (ECDbSqlTable& table, Utf8CP name, Identity::Id id)
            :m_isUnique (false), m_table (table), m_name (name), Identity (id), m_persistenceManager (*this)
            {}

    public:
        Utf8StringCR GetName () const { return m_name; }
        ECDbSqlTable const& GetTable () const { return m_table; }
        ECDbSqlTable & GetTableR () { return m_table; }
        bool GetIsUnique () const { return m_isUnique; }
        void SetIsUnique (bool isUnique);
        std::vector<ECDbSqlColumn const*> const& GetColumns () const { return m_columns; }
        void SetWhereExpression (Utf8CP expression) { m_whereExpression = expression; }
        Utf8StringCR GetWhereExpression () const { return m_whereExpression; }
        bool IsPartialIndex () const;
        bool Contains (Utf8CP column) const;
        BentleyStatus Add (Utf8CP column);
        BentleyStatus Remove (Utf8CP column);
        BentleyStatus WriteTo (BeXmlNodeR xmlNode) const;
        static BentleyStatus ReadFrom (ECDbSqlDb& ecdbSqlDb, BeXmlNodeR xmlNode);
        BentleyStatus Drop ();
        bool IsValid () const { return !m_columns.empty (); }
        PersistenceManager const& GetPersistenceManager () const { return m_persistenceManager; }
        PersistenceManager & GetPersistenceManagerR () { return m_persistenceManager; }

    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DependentPropertyCollection : NonCopyableClass
    {

    private:
        std::map<ECN::ECClassId, Utf8CP> m_map;
        ECDbSqlColumn& m_column;

    public:
        DependentPropertyCollection (ECDbSqlColumn& column)
            :m_column (column)
            {
            }

        ~DependentPropertyCollection ()
            {}
        ECDbSqlColumn& GetColumnR () { return m_column; }
        ECDbSqlColumn const& GetColumn () const { return m_column; }
        BentleyStatus Add (ECN::ECClassId ecClassId, Utf8CP accessString);
        BentleyStatus Add (ECN::ECClassId ecClassId, WCharCP accessString);
        BentleyStatus Remove (ECN::ECClassId ecClassId);
        Utf8CP Find (ECN::ECClassId ecClassId) const;
        bool Contains (ECN::ECClassId ecClassId) const;
        size_t Count () const { return m_map.size (); }
        std::vector<ECN::ECClassId> GetClasses () const
            {
            std::vector<ECN::ECClassId> tmp;
            for (auto& key : m_map)
                tmp.push_back (key.first);

            return tmp;
            }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlColumn 
    {
    enum class Type
        {
        Integer, Long, Double, DateTime, Binary, Boolean, String, Any
        };
    const uint32_t NullUserFlags = 0;
    struct Constraint : NonCopyableClass
        {
        enum class Collate
            {
            Default, // Default is really Binary in sqlite. But we will not provide collate for property to sqlite in this case and assume sqlite default.
            Binary, // Compares string data using memcmp(), regardless of text encoding
            NoCase, // The same as binary, except the 26 upper case characters of ASCII are folded to their lower case equivalents before the comparison is performed. Note that only ASCII characters are case folded. SQLite does not attempt to do full UTF case folding due to the size of the tables required.
            RTrim,  // The same as binary, except that trailing space characters are ignored.
            };

        private:
            bool m_constraintNotNull : 1;
            bool m_constraintIsUnique : 2;
            Utf8String m_constraintCheck;
            Utf8String m_constraintDefaultValue;
            Collate m_collate;
        public:
            Constraint () :m_constraintNotNull (false), m_constraintIsUnique (false), m_collate (Collate::Default)
                {
                }
            bool IsNotNull () const { return m_constraintNotNull; }
            bool IsUnique () const { return m_constraintIsUnique; }
            Utf8StringCR GetCheckExpression () const { return m_constraintCheck; }
            Utf8StringCR GetDefaultExpression () const { return m_constraintDefaultValue; }
            void SetIsNotNull (bool isNotNull) { m_constraintNotNull = isNotNull; }
            void SetIsUnique (bool isUnique) { m_constraintIsUnique = isUnique; }
            void SetCheckExpression (Utf8CP expression) { m_constraintCheck = expression; }
            void SetDefaultExpression (Utf8CP expression) { m_constraintDefaultValue = expression; }
            Collate GetCollate ()  const { return m_collate; }
            void SetCollate (Collate collate)  { m_collate = collate; }
            static Collate StringToCollate (Utf8CP typeName);
            static Utf8CP CollateToString (Collate type);
        };

    private:
        Type m_type;
        ECDbSqlTable& m_ownerTable;
        Constraint m_constraints;
        Utf8String m_name;
        DependentPropertyCollection m_references;
        PersistenceType m_persistenceType;
        uint32_t m_userFlags;
    public:
        ECDbSqlColumn (Utf8CP name, Type type, ECDbSqlTable& owner, PersistenceType persistenceType)
            : m_name (name), m_ownerTable (owner), m_type (type), m_references (*this), m_persistenceType (persistenceType), m_userFlags (NullUserFlags){}

        PersistenceType GetPersistenceType () const { return m_persistenceType; }
        Utf8StringCR GetName () const { return m_name; }
        Type GetType () const { return m_type; };
        ECDbSqlTable const& GetTable () const { return m_ownerTable; }
        ECDbSqlTable&  GetTableR ()  { return m_ownerTable; }

        Constraint const& GetConstraint () const { return m_constraints; };
        Constraint& GetConstraintR ()  { return m_constraints; };
        bool IsReusable () const { return m_type == Type::Any; }
        virtual ~ECDbSqlColumn () {}
        BentleyStatus WriteTo (BeXmlNodeR xmlNode) const;
        static BentleyStatus ReadFrom (ECDbSqlTable& ecdbSqlTable, BeXmlNodeR xmlNode);
        static Type StringToType (Utf8CP typeName);
        static Utf8CP TypeToString (Type type);
        BentleyStatus SetUserFlags (uint32_t userFlags);
        uint32_t GetUserFlags () const;
        DependentPropertyCollection const& GetDependentProperties () const{ return m_references; }
        DependentPropertyCollection & GetDependentPropertiesR (){ return m_references; }
        std::weak_ptr<ECDbSqlColumn> GetWeakPtr () const;
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlConstraint
    {
    enum class Type
        {
        PrimaryKey,
        ForiegnKey
        };

    private:
        Type m_type;
        ECDbSqlTable const& m_table;

    public:
        ECDbSqlConstraint (Type type, ECDbSqlTable const& table)
            :m_type (type), m_table (table)
            {}

        Type GetType () const { return m_type; }
        ECDbSqlTable const& GetTable () const { return m_table; }
        virtual ~ECDbSqlConstraint (){}
        BentleyStatus WriteTo (BeXmlNodeR xmlNode) const;
        static BentleyStatus ReadFrom (ECDbSqlTable& ecdbSqlTable, BeXmlNodeR xmlNode);

    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlPrimaryKeyConstraint : ECDbSqlConstraint
    {
    private:
        std::vector<ECDbSqlColumn const*> m_columns;

    public:
        ECDbSqlPrimaryKeyConstraint (ECDbSqlTable const& table)
            :ECDbSqlConstraint (ECDbSqlConstraint::Type::PrimaryKey, table)
            {}
        virtual ~ECDbSqlPrimaryKeyConstraint (){}
        BentleyStatus Add (Utf8CP columnName);
        BentleyStatus Remove (Utf8CP columnName);
        bool Contains (Utf8CP columnName) const
            {
            return std::find_if (m_columns.begin (), m_columns.end (), [columnName] (ECDbSqlColumn const* column){ return column->GetName ().EqualsI (columnName); }) != m_columns.end ();
            }       
        std::vector<ECDbSqlColumn const*> const& GetColumns () const { return m_columns; }        
        BentleyStatus WriteTo (BeXmlNodeR xmlNode) const;
        static BentleyStatus ReadFrom (ECDbSqlTable& ecdbSqlTable, BeXmlNodeR xmlNode);
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlForiegnKeyConstraint : ECDbSqlConstraint
    {
    enum class ActionType
        {
        NotSpecified,
        Cascade,
        NoAction,
        SetNull,
        SetDefault,
        Restrict,
        };

    enum class MatchType
        {
        NotSpecified,
        Simple,
        Full,
        Partial
        };
    private:
        ECDbSqlTable const& m_targetTable;
        std::vector<ECDbSqlColumn const*> m_sourceColumns;
        std::vector<ECDbSqlColumn const*> m_targetColumns;
        ActionType m_onDeleteAction;
        ActionType m_onUpdateAction;
        MatchType m_matchType;
    public:
        ECDbSqlForiegnKeyConstraint (ECDbSqlTable const& sourceTable, ECDbSqlTable const& targetTable)
            :ECDbSqlConstraint (ECDbSqlConstraint::Type::ForiegnKey, sourceTable), m_targetTable (targetTable), m_matchType (MatchType::NotSpecified), m_onDeleteAction (ActionType::NotSpecified), m_onUpdateAction (ActionType::NotSpecified)
            {}
        void SetOnDeleteAction (ActionType action) { m_onDeleteAction = action; }
        void SetOnUpdateAction (ActionType action) { m_onUpdateAction = action; }
        void SetMatchType (MatchType matchType) { m_matchType = matchType; }

        ActionType GetOnDeleteAction () const { return m_onDeleteAction; }
        ActionType GetOnUpdateAction () const { return m_onUpdateAction; }
        MatchType GetMatchType () const { return m_matchType; }

        BentleyStatus Add (Utf8CP sourceColumn, Utf8CP targetColumn);
        BentleyStatus Remove (size_t index);
        std::vector<ECDbSqlColumn const*> const& GetSourceColumns () const { return m_sourceColumns; }
        std::vector<ECDbSqlColumn const*> const& GetTargetColumns () const { return m_targetColumns; }
        ECDbSqlTable const& GetTargetTable () const { return m_targetTable; }
        ECDbSqlTable const& GetSourceTable () const { return GetTable (); }
        bool ContainsInSource (Utf8CP columnName) const;
        bool ContainsInTarget (Utf8CP columnName) const;
        BentleyStatus Remove (Utf8CP sourceColumn, Utf8CP targetColumn);
        BentleyStatus WriteTo (BeXmlNodeR xmlNode) const;
        static BentleyStatus ReadFrom (ECDbSqlTable& ecdbSqlTable, BeXmlNodeR xmlNode);
        static ActionType ParseActionType (WCharCP actionType);
        static MatchType ParseMatchType (WCharCP matchType);
        static Utf8CP ToSQL (ActionType actionType);
        static Utf8CP ToSQL (MatchType matchType);
        virtual ~ECDbSqlForiegnKeyConstraint (){}
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlTable :  Identity
    {
    enum class ColumnEvent
        {
        Created,
        Deleted
        };
    friend ECDbSqlTable* ECDbSqlDb::CreateTable (Utf8CP name, PersistenceType type);
    friend ECDbSqlTable* ECDbSqlDb::CreateTableUsingExistingTableDefinition (ECDbCR ecdb, Utf8CP existingTableName);
    friend ECDbSqlTable* ECDbSqlDb::CreateTableUsingExistingTableDefinition (Utf8CP existingTableName);
    friend std::weak_ptr<ECDbSqlColumn> ECDbSqlColumn::GetWeakPtr () const;
    struct PersistenceManager : NonCopyableClass
        {
        private:
            ECDbSqlTable& m_table;
        public:
            PersistenceManager (ECDbSqlTable& table)
                :m_table (table)
                {
                }
            ~PersistenceManager (){}
            ECDbSqlTable const& GetTable () const{ return m_table; }
            ECDbSqlTable& GetTableR () { return m_table; }
            BentleyStatus Syncronize (ECDbR ecdb);
                
            BentleyStatus Create (ECDbR ecdb, bool createIndexes);
            BentleyStatus Drop (ECDbR ecdb);
            bool Exist (ECDbR ecdb) const;
        };

    private:
        ECDbSqlDb& m_dbDef;
        NameGenerator m_nameGeneratorForColumn;
        std::map<Utf8CP, std::shared_ptr<ECDbSqlColumn>, CompareIUtf8> m_columns;
        std::vector<ECDbSqlColumn const*> m_orderedColumns;
        std::vector<std::unique_ptr<ECDbSqlConstraint>> m_constraints;
        EditHandle m_editInfo;
        PersistenceType m_type;
        Utf8String m_name;
        OwnerType m_ownerType;
        PersistenceManager m_persistenceManager;
        std::vector<std::function<void (ColumnEvent, ECDbSqlColumn&)>> m_columnEvents;

    private:
        ECDbSqlTable (Utf8CP name, ECDbSqlDb& sqlDbDef, Identity::Id id, PersistenceType type, OwnerType ownerType)
            : m_name (name), Identity (id), m_dbDef (sqlDbDef), m_nameGeneratorForColumn ("c%d"), m_type (type), m_ownerType (ownerType), m_persistenceManager (*this)
            {
            }
        std::weak_ptr<ECDbSqlColumn> GetColumnWeakPtr (Utf8CP name) const;
    public:
        Utf8StringCR GetName () const { return m_name; }
        PersistenceType GetPersistenceType () const { return m_type; }
        OwnerType GetOwnerType () const { return m_ownerType; }
        ECDbSqlDb const& GetDbDef () const{ return m_dbDef; }
        ECDbSqlDb & GetDbDefR () { return m_dbDef; }
        //! Any type will be mark as reusable column
        ECDbSqlColumn* CreateColumn (Utf8CP name, ECDbSqlColumn::Type type, uint32_t userFlag = 0, PersistenceType persistenceType = PersistenceType::Persisted);
        ECDbSqlColumn* CreateColumn (Utf8CP name, ECDbSqlColumn::Type type, size_t position, uint32_t userFlag = 0, PersistenceType persistenceType = PersistenceType::Persisted);

        ECDbSqlColumn const* FindColumnCP (Utf8CP name) const;
        ECDbSqlColumn* FindColumnP (Utf8CP name) const;
        std::vector<ECDbSqlColumn const*> const& GetColumns () const;
        EditHandle& GetEditHandleR () { return m_editInfo; }
        EditHandle const& GetEditHandle () const { return m_editInfo; }
        ECDbSqlIndex* CreateIndex (Utf8CP indexName);
        const std::vector<ECDbSqlIndex const*> GetIndexes () const;
        const std::vector<ECDbSqlIndex*> GetIndexesR ();
        ECDbSqlPrimaryKeyConstraint* GetPrimaryKeyConstraint (bool createIfDonotExist = true);
        ECDbSqlForiegnKeyConstraint* CreateForiegnKeyConstraint (ECDbSqlTable const& targetTable);
        std::vector<ECDbSqlConstraint const*> GetConstraints () const;   
        BentleyStatus GetFilteredColumnList (std::vector<ECDbSqlColumn const*>& columns, PersistenceType persistenceType) const;
        BentleyStatus GetFilteredColumnList (std::vector<ECDbSqlColumn const*>& columns, uint32_t userFlag) const;
        bool DeleteColumn (Utf8CP name);
        BentleyStatus FinishEditing ();
        virtual ~ECDbSqlTable (){}
        BentleyStatus WriteTo (BeXmlNodeR xmlNode) const;
        static BentleyStatus ReadFrom (ECDbSqlDb& ecdbSqlDb, BeXmlNodeR xmlNode);
        //! temp method
        void AddColumnEventHandler (std::function<void (ColumnEvent, ECDbSqlColumn&)> columnEventHandler){ m_columnEvents.push_back(columnEventHandler); }
        std::set<ECN::ECClassId> GetReferences () const;
        PersistenceManager const& GetPersistenceManager () const { return m_persistenceManager; }
        PersistenceManager& GetPersistenceManagerR () { return m_persistenceManager; }
        bool IsValid () const { return m_columns.size () > 0; }
      
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         10/2014
//======================================================================================
struct DDLGenerator
    {
    enum class CreateOption
        {
        Create,
        CreateOrReplace
        };

    private:
        static Utf8String GetColumnList (std::vector<ECDbSqlColumn const*> const& columns);
        static Utf8String GetForiegnKeyConstraintDDL (ECDbSqlForiegnKeyConstraint const& constraint);
        static Utf8String GetPrimarykeyConstraintDDL (ECDbSqlPrimaryKeyConstraint const& constraint);
        static Utf8String GetConstraintsDDL (std::vector<ECDbSqlConstraint const*> const& constraints);
        static Utf8String GetColumnsDDL (std::vector<ECDbSqlColumn const*> columns);
        static Utf8String GetColumnDDL (ECDbSqlColumn const& column);
    public:
        static Utf8String GetCreateIndexDDL (ECDbSqlIndex const& index, CreateOption createOption);
        static Utf8String GetCreateTableDDL (ECDbSqlTable const& table, CreateOption createOption);
        static Utf8String GetDropTableDDL (ECDbSqlTable const& table)
            {
            return "DROP TABLE [" + table.GetName () + "]";
            }
        static Utf8String GetDropIndexDDL (ECDbSqlIndex const& index)
            {
            return "DROP INDEX [" + index.GetName () + "]";
            }

        static BentleyStatus AddColumns (ECDbSqlTable const& table, std::vector<Utf8CP> const& newColumns, BeSQLiteDbR &db);
        static BentleyStatus CopyRows (BeSQLiteDbR db, Utf8CP sourceTable, bvector<Utf8String>& sourceColumns, Utf8CP targetTable, bvector<Utf8String>& targetColumns);

    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlPath
    {
private:
    ECDbSqlDb& m_dbDef;
    ECDbSqlTable* m_table;
    ECDbSqlColumn* m_column;
    ECDbSqlColumn::Type m_dataType;

    void ResolveTable (Utf8CP tableName);
    void ResolveColumn (Utf8CP columnName);

public:
    ECDbSqlPath (ECDbSqlDb& dbDef, Utf8CP tableName);

    Utf8StringCR GetTableName () const;
    Utf8StringCR GetColumnName () const;
    ECDbSqlTable const* GetTable () const;
    ECDbSqlColumn const* GetColumn () const;
    ECDbSqlTable* GetTableP () const
        {
        return m_table;
        }
    ECDbSqlColumn* GetColumnP () const
        {
        return m_column;
        }

    void SetColumnName (Utf8CP name, ECDbSqlColumn::Type type = ECDbSqlColumn::Type::Any);
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbDebugStringWriter : NonCopyableClass
    {
    struct Scope
        {
        private:
            ECDbDebugStringWriter& m_writer;

        public:
            Scope (ECDbDebugStringWriter& writer)
                : m_writer (writer)
                {
                m_writer.Indent ();
                }

            ECDbDebugStringWriter& GetWriter ()
                {
                return m_writer;
                }
            ~Scope ()
                {
                m_writer.Unindent ();
                }
        };

    typedef std::unique_ptr<Scope> ScopePtr;
    private:
        size_t m_currentIndent;
        size_t m_indentSize;
        bool m_needPrefix;
        Utf8String m_indentChar;
        Utf8String m_buffer;
        Utf8String m_eol;
        Utf8String m_prefix;
        Utf8CP BuildPrefix ();
    public:
        ECDbDebugStringWriter (int indentSize = 1, Utf8CP indentChar = " ");
        ~ECDbDebugStringWriter ()
            {}
        void Indent ();
        void Unindent ();
        void Reset ();
        void Write (Utf8StringCR buffer);
        void Write (Utf8CP fmt, ...);
        void WriteLine (Utf8CP fmt, ...);
        void WriteLine ();
        ScopePtr CreateScope ();
        Utf8StringCR GetString () const;
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlDbDebugWriter
    {
    private:
        ECDbSqlDbDebugWriter ()
            {}

    public:
        static void WriteDbDef (ECDbDebugStringWriter& writer, ECDbSqlDb const& obj);
        static void WriteTableDef (ECDbDebugStringWriter& writer, ECDbSqlTable const& obj);
        static void WriteColumnDef (ECDbDebugStringWriter& writer, ECDbSqlColumn const& obj);
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct IWriteDebugString
    {
    private:
        virtual void _WriteDebugString (ECDbDebugStringWriter& writer) = 0;

    public:
        void WriteDebugString (ECDbDebugStringWriter& writer)
            {
            _WriteDebugString (writer);
            }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSQLManager
    {    
    private:
        ECDbSqlDb m_defaultDb;
        ECDbR m_ecdb;
        PropertySpec m_propertySpec;
        ECDbSqlTable* m_nullTable;

    private:
        void SetupNullTable ();
    public:

        ECDbSQLManager (ECDbR ecdb)
            : m_ecdb (ecdb), m_defaultDb (ECDBSQL_SCHEMA_ID, ECDbVersion (ECDBSQL_VERSION_MAJOR, ECDBSQL_VERSION_MINOR)), m_propertySpec (ECDBSQL_PROPERTYNAME, ECDBSQL_NAMESPACE)
            {
            SetupNullTable ();
            }
        ~ECDbSQLManager () {}

        ECDbR GetECDbR () { return m_ecdb; }
        ECDbCR GetECDb () const{ return m_ecdb; }
        ECDbSqlDb& GetDbSchemaR () { return m_defaultDb; }
        ECDbSqlDb const& GetDbSchema () const { return m_defaultDb; }
        BentleyStatus Load ();
        BentleyStatus Save ();
        ECDbSqlTable const* GetNullTable () const { return m_nullTable; }
        bool IsNullTable (ECDbSqlTable const& table) const { return &table == GetNullTable (); }
        bool IsTableChanged (ECDbSqlTable const& table) const;
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlHelper
    {
    static ECDbSqlColumn::Type PrimitiveTypeToColumnType (ECN::PrimitiveType type);
    static bool IsCompatiable (ECDbSqlColumn::Type target, ECDbSqlColumn::Type source);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE