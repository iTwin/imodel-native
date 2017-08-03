/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/UsageDb.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UsageDb.h"
#include <Licensing/Utils/SCVWritter.h>

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::OpenOrCreate(BeFileNameCR filePath)
    {
    if (m_db.IsDbOpen())
        {
        if (filePath.GetNameUtf8().compare(m_db.GetDbFileName()) == 0)
            return SUCCESS;

        m_db.CloseDb();
        }

	if (filePath.DoesPathExist())
        return OpenDb(filePath);

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
bool UsageDb::IsDbOpen()
    {
    return m_db.IsDbOpen();
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
    if (m_db.CreateTable("Usage", 
        "StartTime INTEGER, "
        "EndTime INTEGER") != DbResult::BE_SQLITE_OK)
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
BentleyStatus UsageDb::InsertNewRecord(int64_t startTime, int64_t endTime)
    {
    Statement stmt;
    stmt.Prepare(m_db, "INSERT INTO Usage VALUES (?, ?)");
    stmt.BindInt64(1, startTime);
    stmt.BindInt64(2, endTime);
    
    DbResult result = stmt.Step();
    if (result == DbResult::BE_SQLITE_DONE)
        return SUCCESS;

    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t UsageDb::GetLastRowId()
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT MAX(rowid) FROM Usage");

    DbResult result = stmt.Step();
    if (result != DbResult::BE_SQLITE_ROW)
        return 0;

    return stmt.GetValueInt64(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t UsageDb::GetLastRecordEndTime()
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT EndTime FROM Usage ORDER BY rowid DESC LIMIT 1");
    
    DbResult result = stmt.Step();
    if (result != DbResult::BE_SQLITE_ROW)
        return 0;

    return stmt.GetValueInt64(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::UpdateLastRecordEndTime(int64_t endTime)
    {
    int64_t lastRowId = GetLastRowId();
    if (lastRowId < 1)
        return ERROR;

    Statement stmt;
    stmt.Prepare(m_db, "UPDATE Usage set EndTime = ? WHERE rowid = ?");
    stmt.BindInt64(1, endTime);
    stmt.BindInt64(2, lastRowId);

    DbResult result = stmt.Step();
    if (result == DbResult::BE_SQLITE_DONE)
        return SUCCESS;

    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t UsageDb::GetRecordCount()
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT COUNT(*) FROM Usage");

    DbResult result = stmt.Step();
    if (result != DbResult::BE_SQLITE_ROW)
        return -1;

    return stmt.GetValueInt(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::WriteUsageToSCVFile(BeFileNameCR path)
    {
    SCVWritter writter;
    writter.AddRow("StartTime", "EndTime");

    Statement stmt;
    stmt.Prepare(m_db, "SELECT StartTime, EndTime FROM Usage");

    DbResult result = stmt.Step();
    while (result == DbResult::BE_SQLITE_ROW)
        {
        writter.AddRow(stmt.GetValueInt64(0), stmt.GetValueInt64(1));
        result = stmt.Step();
        }
    if (result != DbResult::BE_SQLITE_DONE)
        return ERROR;

    return writter.WriteToFile(path);
    }