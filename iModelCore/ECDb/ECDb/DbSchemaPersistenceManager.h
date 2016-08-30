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
    static BentleyStatus RepopulateClassHierarchyCacheTable(ECDbCR);
    static BentleyStatus RepopulateClassHasTableCacheTable(ECDbCR);
 
    static bool IsTrue(int sqlInt) { return sqlInt != 0; }
    static int BoolToSqlInt(bool val) { return val ? 1 : 0; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE