/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/UsageDb.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UsageDb.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UsageDb::UsageDb()
{
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::OpenOrCreate(BeFileNameCR filePath)
    {
	if (filePath.DoesPathExist())
        return OpenDb(filePath);
     else
        return CreateDb(filePath);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::OpenDb(BeFileNameCR filePath)
    {
    DbResult result = m_db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite));

    if (result == DbResult::BE_SQLITE_OK)
        return SUCCESS;

    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::CreateDb(BeFileNameCR filePath)
    {
    DbResult result = m_db.CreateNewDb(filePath);

    if (result != DbResult::BE_SQLITE_OK)
        return ERROR;

    return SetUpTables();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::SetUpTables()
    {
    if (m_db.CreateTable("TestTable", 
        "COL1 TEXT, "
        "StartTimeUTC DATETIME, "
        "EndTimeUTC DATETIME") != DbResult::BE_SQLITE_OK)
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UsageDb::Close()
    {
    m_db.CloseDb();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::TestInsert()
    {
    DbResult result = m_db.ExecuteSql("INSERT INTO TestTable (COL1) VALUES ('test value')");
    
    if (result == DbResult::BE_SQLITE_OK)
            return SUCCESS;

    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int UsageDb::GetTestRecordCount()
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT COUNT(*) FROM TestTable");
    stmt.Step();
    
    return stmt.GetValueInt(0);
    }