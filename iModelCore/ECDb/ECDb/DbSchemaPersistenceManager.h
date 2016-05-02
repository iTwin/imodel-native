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

    static DbResult ReadTables(DbSchema&, ECDbCR);
    static DbResult ReadColumns(DbTable&, ECDbCR);
    static BentleyStatus ReadIndexes(DbSchema&, ECDbCR);
    static DbResult ReadClassMappings(DbSchema&, ECDbCR);
    static DbResult ReadPropertyMappings(ClassDbMapping&, ECDbCR, DbSchema const&);
    static DbResult ReadPropertyPaths(DbSchema&, ECDbCR);

    static DbResult InsertTable(ECDbCR, DbTable const&);
    static DbResult InsertColumn(ECDbCR, DbColumn const&, int columnOrdinal, int primaryKeyOrdinal);
    static DbResult InsertConstraint(ECDbCR, DbConstraint const&);
    static DbResult InsertForeignKeyConstraint(ECDbCR, ForeignKeyDbConstraint const&);
    static DbResult InsertIndex(ECDbCR, DbIndex const&);

    static DbResult InsertClassMapping(ECDbCR, ClassDbMapping const&);
    static DbResult InsertPropertyMapping(ECDbCR, PropertyDbMapping const&);
    static DbResult InsertPropertyPath(ECDbCR, PropertyDbMapping::Path const&);

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

    static BentleyStatus BuildCreateIndexDdl(Utf8StringR ddl, Utf8StringR comparableIndexDef, ECDbCR, DbIndex const&);
    static BentleyStatus GenerateIndexWhereClause(Utf8StringR ddl, ECDbCR, DbIndex const&);


public:
    static BentleyStatus Load(DbSchema&, ECDbCR, DbSchema::LoadState loadMode);
    static BentleyStatus Save(ECDbCR, DbSchema const&);

    static CreateOrUpdateTableResult CreateOrUpdateTable(ECDbCR, DbTable const&);
    static BentleyStatus CreateOrUpdateIndexes(ECDbCR, DbSchema const&);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE