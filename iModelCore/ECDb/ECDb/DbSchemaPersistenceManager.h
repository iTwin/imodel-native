/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbSchemaPersistenceManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "DbSchema.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbClassMapLoadContext : public NonCopyableClass
    {
    private:
        MapStrategyExtendedInfo m_mapStrategyExtInfo;
        ClassMapId m_classMapId;
        ECN::ECClassId m_baseClassId;
        bool m_isValid;
        std::map<Utf8String, std::vector<DbColumn const*>> m_columnByAccessString;
        static BentleyStatus ReadPropertyMaps(DbClassMapLoadContext& ctx, ECDbCR ecdb);
        ClassMapCP m_baseClassMap;
    public:
        DbClassMapLoadContext() :m_isValid(false), m_baseClassMap(nullptr) {}
        ~DbClassMapLoadContext() {}
        MapStrategyExtendedInfo const& GetMapStrategy() const { return m_mapStrategyExtInfo; }
        ClassMapId GetClassMapId() const { return m_classMapId; }
        ECN::ECClassId GetBaseClassId() const { return m_baseClassId; }
        bool IsValid() const { return m_isValid; }
        ClassMapCP GetBaseClassMap() const { return m_baseClassMap; }
        BentleyStatus SetBaseClassMap(ClassMapCR classMap);
        bool HasMappedProperties() const { return !m_columnByAccessString.empty(); }
        std::vector<DbColumn const*> const* FindColumnByAccessString(Utf8CP accessString) const;
        std::map<Utf8String, std::vector<DbColumn const*>> const& GetPropertyMaps() const { return m_columnByAccessString; }
        static BentleyStatus Load(DbClassMapLoadContext& loadContext, ECDbCR ecdb, ECN::ECClassId classId);
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbMapSaveContext : public NonCopyableClass
    {
    private:
        std::map<ECN::ECClassId, ClassMapCP> m_savedClassMaps;
        std::stack<ClassMapCP> m_editStack;
        ECDbCR m_ecdb;
    public:
        DbMapSaveContext(ECDbCR ecdb) :m_ecdb(ecdb)
            {}
        ~DbMapSaveContext() {}
        ECDbCR GetECDb() const { return m_ecdb; }
        bool IsAlreadySaved(ClassMapCR classMap) const;
        void BeginSaving(ClassMapCR classMap);
        void EndSaving(ClassMapCR classMap);
        ClassMapCP GetCurrent() const { return m_editStack.top();}
        BentleyStatus InsertClassMap(ClassMapId&, ECN::ECClassId, MapStrategyExtendedInfo const&, ClassMapId baseClassMapId);
        BentleyStatus TryGetPropertyPathId(PropertyPathId&, ECN::ECPropertyId rootPropertyId, Utf8CP accessString, bool addIfDoesNotExist);
    };

    
//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbClassMapSaveContext : public NonCopyableClass
    {
    private:
        DbMapSaveContext& m_classMapContext;
        ClassMapCR  m_classMap;

    public:
        DbClassMapSaveContext(DbMapSaveContext& ctx);
        ~DbClassMapSaveContext(){}
        DbMapSaveContext const& GetMapSaveContext() const { return m_classMapContext; }
        ClassMapCR GetClassMap() const { return m_classMap; }
        BentleyStatus InsertPropertyMap(ECN::ECPropertyId rootPropertyId, Utf8CP accessString, DbColumnId columnId);
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbSchemaPersistenceManager : public NonCopyableClass
    {
public:
    enum class CreateOrUpdateTableResult
        {
        Created = 1,
        Updated = 2,
        WasUpToDate = 3,
        Skipped = 4,
        Error = 5
        };

private:
    DbSchemaPersistenceManager();
    ~DbSchemaPersistenceManager();


    static bool IsTableChanged(ECDbCR, DbTable const&);

    static BentleyStatus CreateTable(ECDbCR, DbTable const&);
    static BentleyStatus UpdateTable(ECDbCR, DbTable const&);
    static BentleyStatus AlterTable(ECDbCR, DbTable const&, std::vector<DbColumn const*> const& columnsToAdd);

    static BentleyStatus CreateTriggers(ECDbCR, DbTable const&, bool failIfExists);
    static bool TriggerExistsInDb(ECDbCR, DbTrigger const&);

    static BentleyStatus AppendColumnDdl(Utf8StringR ddl, DbColumn const&);
    //!@param[in] singleFkColumn must not be nullptr if @p embedInColumnDdl is true
    static BentleyStatus AppendForeignKeyToColumnDdl(Utf8StringR ddl, ForeignKeyDbConstraint const&, DbColumn const& singleFkColumn);
    static BentleyStatus AppendForeignKeyDdl(Utf8StringR ddl, ForeignKeyDbConstraint const&);
    static void DoAppendForeignKeyDdl(Utf8StringR ddl, ForeignKeyDbConstraint const&);
    static void AppendColumnNamesToDdl(Utf8StringR ddl, std::vector<DbColumn const*> const&);

    static BentleyStatus GenerateIndexWhereClause(Utf8StringR ddl, ECDbCR, DbIndex const&);

public:
    static BentleyStatus BuildCreateIndexDdl(Utf8StringR ddl, Utf8StringR comparableIndexDef, ECDbCR, DbIndex const&);

    static CreateOrUpdateTableResult CreateOrUpdateTable(ECDbCR, DbTable const&);

    static bool IsTrue(int sqlInt) { return sqlInt != 0; }
    static int BoolToSqlInt(bool val) { return val ? 1 : 0; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE