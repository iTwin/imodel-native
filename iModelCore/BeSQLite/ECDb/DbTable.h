/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbTable.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbMap.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! DbTable is not intended as a general representation of a database table, but
//! is instead focused on the mapping of ECSchema to db schema
// @bsiclass                                                Affan.Khan  02/2012
//+===============+===============+===============+===============+===============+======
struct DbTable : public RefCountedBase
    {
    friend DbTablePtr ECDbMap::FindOrCreateTable (Utf8CP tableName, bool isVirtual, Utf8CP primaryKeyColumnName, bool mapToSecondaryTable, bool mapToExisitingTable, bool allowReplacingEmptyTableWithView) const;

public:
    //=======================================================================================
    //! Collection of DbColumns
    //! The collection owns the DbColumn
    // @bsiclass                                                Krischan.Eberle  05/2014
    //+===============+===============+===============+===============+===============+======
    struct DbColumnCollection : NonCopyableClass
        {
    private:
        typedef bmap<Utf8CP, DbColumnPtr, CompareIUtf8> ColumnMap;

    public:
        //=======================================================================================
        //! Collection iterator
        // @bsiclass                                                Krischan.Eberle  05/2014
        //+===============+===============+===============+===============+===============+======
        struct const_iterator : std::iterator<std::forward_iterator_tag, DbColumnCP>
            {
        private:
            ColumnMap::const_iterator m_innerIterator;

        public:
            const_iterator ()
                {}

            const_iterator (ColumnMap::const_iterator const& innerIterator)
                : m_innerIterator (innerIterator)
                {}

            DbColumnCP operator* () const
                {
                return m_innerIterator->second.get ();
                }

            const_iterator& operator++ ()
                {
                m_innerIterator++;
                return *this;
                }
            bool operator== (const_iterator const& rhs) const
                {
                return m_innerIterator == rhs.m_innerIterator;
                }

            bool operator!= (const_iterator const& rhs) const
                {
                return !(*this == rhs);
                }

            };
    private:
        ColumnMap m_columns;

        bool TryGet (ColumnMap::const_iterator& it, Utf8CP name) const;

    public:
        DbColumnCollection () {}

        void Add (DbColumnPtr& newColumn);
        bool Contains (Utf8CP name) const;
        DbColumnCP Get (Utf8CP name) const;
        DbColumnPtr GetPtr (Utf8CP name) const;
        bool IsEmpty () const;
        size_t Size () const;
        const_iterator begin () const;
        const_iterator end () const;
        };

    typedef std::function<void (DbColumnPtr&)> NewClassIdColumnHook;


private:
    static const std::map<ECN::PrimitiveType, Utf8String> s_ecsqliteTypeMap;
    Utf8String           m_tableName;
    bool                 m_isVirtual;
    DbColumnCollection   m_columns;
    DbColumnPtr          m_classIdColumn;
    bool                 m_mapToExistingTable;
    bool                 m_replaceEmptyTableWithEmptyView;
    bvector<DbIndexPtr>  m_tableIndices;
    bvector<DbTableConstraintPtr> m_tableConstraints;
    NewClassIdColumnHook m_afterSetClassIdHook;

    DbTable (Utf8CP tableName, bool isVirtual, bool mapToExistingTable, bool allowReplacingEmptyTableWithView);
    ~DbTable ();
    // non-copyable
    DbTable (DbTableCR);
    DbTableR operator= (DbTableCR);

    void AppendPrimaryKeyConstraintDDL (Utf8StringR sql) const;
    void AppendColumnDDL (Utf8StringR sql, DbColumnCR column, bool firstColumn = false) const;
    void GeneratePrimaryKeyColumns (Utf8CP primaryKeyColumnName, bool mapToSecondaryTable);

    //! Tries to assign the new values to the DbTable if they are stronger than the existing
    //! ones. 
    //! This method must only be called in cases where multiple classes map to the same table,
    //! and therefore want to reuse this DbTable. Properties of the DbTable like IsVirtual or
    //! IsAllowedToReplaceEmptyTableWithEmptyView are changed to the option which does not break
    //! any of the class mappings for that table.
    //! E.g. if one class is ideally mapped to a virtual table (because it is an abstract class)
    //! and the other class is ideally mapped to a non-virtual table, the shared table will
    //! be non-virtual. Otherwise no instances could be stored for the non-abstract class, while
    //! it does not affect the abstract class's mapping.
    void TryAssign (bool newIsVirtual, bool newAllowToReplaceEmptyClassByEmptyView);

    DbResult CopyRows (BeSQLiteDbR db, Utf8CP sourceTable, bvector<Utf8String>& sourceColumns, Utf8CP targetTable, bvector<Utf8String>& targetColumns);

    void AddClassIdColumn (DbColumnPtr classIdColumn);

    DbResult PersistNewColumns (std::vector<Utf8CP> const& newColumns, std::map<Utf8CP, DbColumnCP, CompareIUtf8> const& modifiedColumnSet, BeSQLiteDbR &db) const;

    static Utf8String GetSqliteType (DbColumnCR column);

public:
    static DbTablePtr Create (Utf8CP tableName, bool mapToSecondaryTable, bool isVirtual, Utf8CP primaryKeyColumnName, bool mapToExistingTable, bool allowReplacingEmptyTableWithView);
    Utf8CP GetName () const;
    bool HasPendingModifications (BeSQLiteDbR db);
    DbColumnCollection const& GetColumns () const { return m_columns; }
    DbColumnCollection& GetColumnsR () { return m_columns; }

    DbColumnCP GetClassIdColumn () const;
    DbColumnPtr GetClassIdColumnPtr () const;
    void SetAfterSetClassIdHook (NewClassIdColumnHook hook) { m_afterSetClassIdHook = hook; }

    DbPrimaryKeyTableConstraintR GetPrimaryKeyTableConstraint () const;
    DbColumnCP GenerateClassIdColumn ();
    DbResult CreateTableInDb (BeSQLiteDbR db);
    void GetCreateTableSQL (Utf8StringR sql);
    void AddConstraint (DbTableConstraintR constraint);
    BentleyStatus AddIndex (DbIndexR index);
    DbIndexCP GetIndexByName (Utf8CP indexName);
    DbResult CreateEmptyView (BeSQLiteDbR db) const;
    static void AppendColumnToCreateEmptyViewSql (Utf8StringR creteEmptyViewSql, DbColumnCR column);
    bool IsNullTable () const;
    //! Checks whether this table is empty or not.
    //! @param[in] db ECDb to check against
    //! @return treatAsEmptyOnError Pass true, if this method should return true if the table does
    //!         not exist (or any other error occurs). Pass false, if this method should return false, if the table doesn't exist
    //!         (or any other error occurs).
    bool IsEmpty (BeSQLiteDbR db, bool treatAsEmptyOnError) const;
    bool IsAllowedToReplaceEmptyTableWithEmptyView () const { return m_replaceEmptyTableWithEmptyView && !m_mapToExistingTable; }
    bool IsMappedToExistingTable () const { return m_mapToExistingTable; }
    //!Meta info require for incremental update.
    DbResult SyncWithDb (BeSQLiteDbR db);



    //! Virtual tables are not persisted   
    bool IsVirtual () const { return m_isVirtual; }

    void Dump (Utf8StringR output) const;

    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbMetaDataHelper
    {
    enum class ObjectType
        {
        None,
        Table,
        View,
        Index,
        };
    static ObjectType GetObjectType(Db& db, Utf8CP name)
        {
        BeSQLite::CachedStatementPtr stmt;
        db.GetCachedStatement(stmt, "SELECT type FROM sqlite_master WHERE name =?");
        stmt->BindText(1,name, BeSQLite::Statement::MAKE_COPY_No);
        if (stmt->Step() == BE_SQLITE_ROW)
            {
            if (BeStringUtilities::Stricmp (stmt->GetValueText(0), "table") == 0)
                return ObjectType::Table;
            if (BeStringUtilities::Stricmp (stmt->GetValueText (0), "view") == 0)
                return ObjectType::View;
            if (BeStringUtilities::Stricmp (stmt->GetValueText (0), "index") == 0)
                return ObjectType::Index;
            }
        return ObjectType::None;
        }
    };
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbConstraint : RefCountedBase
    {
private:
    Utf8String m_name;
protected:
    DbConstraint();
    void          AppendConstraintNameIfAvaliable(Utf8StringR sql);
    virtual bool  _BuildSqlClause(Utf8StringR sql) = 0;
public:
    Utf8StringCR  GetName () const;
    void          SetName (Utf8CP constraintName);
public: 
    bool          GenerateSQLClause(Utf8StringR sqlClause);
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbTableConstraint : DbConstraint
    {
    private:
        DbColumnPtrList m_columns;
    
    protected:
        void AppendColumnList(Utf8StringR sql);
        DbTableConstraint();
    public:
        void GetColumns(bvector<DbColumnP>& columns) const;
        void GetColumns(DbColumnPtrList& columns) const;

        bool AddColumn(DbColumnR column);
        bool RemoveColumn(DbColumnR column);
        size_t  GetColumnCount();
    };

//struct DbColumnConstraint : DbConstraint
//    {
//
//    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbPrimaryKeyTableConstraint : DbTableConstraint
    {
protected:
     virtual  bool _BuildSqlClause(Utf8StringR sql) override;
     DbPrimaryKeyTableConstraint();
public:
    static DbPrimaryKeyTableConstraintPtr Create();
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbUniqueTableConstraint : DbTableConstraint
    {
protected:
    DbUniqueTableConstraint();
     virtual bool _BuildSqlClause(Utf8StringR sql)override;
public:
        static DbUniqueTableConstraintPtr Create();
    };


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbIndex : RefCountedBase
    {
private:
    mutable Utf8String   m_name;
    DbTablePtr           m_table;
    bool                 m_isUnique;
    DbColumnPtrList      m_columns;

private:
    DbIndex (Utf8CP name, DbTableR table, bool isUnique);

    void GenerateIndexName (Utf8StringR name) const;
public:
    Utf8CP  GetName(bool generateName = false) const;
    void    AddColumn(DbColumnCR column);
    bool    BuildCreateIndexSql(Utf8StringR sql) const;
    bool    IsUnique() const {return m_isUnique;}

    //! Creates a new DbIndex object
    //! @param[in] name Index name. Pass nullptr if name is being generated when calling GetName (true) or
    //!            when the DbIndex is added to a DbTable through DbTable::AddIndex
    //! @param[in] table DbTable on which the index will operate. Note: The DbIndex object will not be added
    //!            to the DbTable's list of indices here.
    //! @param[in] isUnique true, if this is a unique index. false otherwise
    static DbIndexPtr Create (Utf8CP name, DbTableR table, bool isUnique);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

