/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/UsageDb.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Utils/SCVWritter.h>

#include "UsageDb.h"
#include "Logging.h"

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
    // Create Policy table
    if (m_db.CreateTable("Policy",
                         "PolicyId NVARCHAR(20) PRIMARY KEY, "
                         "ExpirationDate NVARCHAR(20), "
                         "LastUpdateTime NVARCHAR(20), "
                         "PolicyFile NVARCHAR(900)") != DbResult::BE_SQLITE_OK)
        {
        return ERROR;
        }

    // Create Usage Record table
    if (m_db.CreateTable("Usage",
                         "UltimateSAPId INTEGER, "
                         "PrincipalId NVARCHAR(20), "
                         "ImsId NVARCHAR(20), "
                         "MachineName NVARCHAR(255), "
                         "MachineSID NVARCHAR(255), "
                         "UserName NVARCHAR(255), "
                         "UserSID NVARCHAR(255), "
                         "PolicyId NVARCHAR(20), "
                         "SecurableId NVARCHAR(20), "
                         "ProductId INTEGER, "
                         "FeatureString NVARCHAR(255), "
                         "ProductVersion INTEGER, "
                         "ProjectId NVARCHAR(20), "
                         "CorrelationId NVARCHAR(20), "
                         "EventTime NVARCHAR(20), "
                         "SchemaVersion REAL, "
                         "LogPostingSource NVARCHAR(20), "
                         "Country NVARCHAR(20), "
                         "UsageType NVARCHAR(20)") != DbResult::BE_SQLITE_OK)
        {
        return ERROR;
        }

	// Create Offline Grace Period table
	if (SetUpOfflineGraceTable() != SUCCESS)
		{
			return ERROR;
		}

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::SetUpOfflineGraceTable()
	{
	if (m_db.CreateTable("OfflineGrace",
		"GraceId NVARCHAR(20) PRIMARY KEY, "
		"OfflineGracePeriodStart NVARCHAR(20)") != DbResult::BE_SQLITE_OK)
	{
		return ERROR;
	}
	return ResetOfflineGracePeriod();
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
    /*Statement stmt;
    stmt.Prepare(m_db, "INSERT INTO Usage VALUES (?, ?)");
    stmt.BindText(12, startTime, Statement::MakeCopy::No);
    //stmt.BindInt64(2, endTime);
    
    DbResult result = stmt.Step();
    if (result == DbResult::BE_SQLITE_DONE)*/
        return SUCCESS;

    //return ERROR;
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
    stmt.Prepare(m_db, "SELECT EventTime FROM Usage ORDER BY rowid DESC LIMIT 1");
    
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
    /*int64_t lastRowId = GetLastRowId();
    if (lastRowId < 1)
        return ERROR;

    Statement stmt;
    stmt.Prepare(m_db, "UPDATE Usage set EventTime = ? WHERE rowid = ?");
    stmt.BindInt64(1, endTime);
    stmt.BindInt64(2, lastRowId);

    DbResult result = stmt.Step();
    if (result == DbResult::BE_SQLITE_DONE)*/
        return SUCCESS;

    //return ERROR;
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
BentleyStatus UsageDb::WriteUsageToCSVFile(BeFileNameCR path)
    {
    SCVWritter writter;

    Statement stmt;
    stmt.Prepare(m_db, "SELECT * FROM Usage");


    DbResult result = stmt.Step();

    while (result == DbResult::BE_SQLITE_ROW)
        {        
        writter.AddRow(stmt.GetValueInt64(0),
                       stmt.GetValueText(1),
                       stmt.GetValueText(2),
                       stmt.GetValueText(3),
                       stmt.GetValueText(4),
                       stmt.GetValueText(5),
                       stmt.GetValueText(6),
                       stmt.GetValueText(7),
                       stmt.GetValueText(8),
                       stmt.GetValueInt(9),
                       stmt.GetValueText(10),
                       stmt.GetValueInt64(11),
                       stmt.GetValueText(12),
                       stmt.GetValueText(13),
                       stmt.GetValueText(14),
                       stmt.GetValueDouble(15),
                       stmt.GetValueText(16),
                       stmt.GetValueText(17),
                       stmt.GetValueText(18));

        result = stmt.Step();
        }

    if (result != DbResult::BE_SQLITE_DONE)
        return ERROR;

    return writter.WriteToFile(path);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<Json::Value> UsageDb::GetPolicyFiles()
	{
	std::list<Json::Value> policyList;

	Statement stmt;
	stmt.Prepare(m_db, "SELECT PolicyFile FROM Policy");
	bool isDone = false;
	while (!isDone)
		{
		DbResult result = stmt.Step();
		if (result == DbResult::BE_SQLITE_ROW)
			{
			Utf8String policyUtf8 = stmt.GetValueText(0);
			auto policyJson = Json::Reader::DoParse(policyUtf8);
			policyList.push_back(policyJson);
			}
		else
			{
			isDone = true;
			}
		}
	return policyList;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken)
    {
    Statement stmt;

    if (m_db.IsDbOpen())
        {
        Utf8String stringToken = Json::FastWriter::ToString(policyToken);
        stmt.Prepare(m_db, "INSERT INTO Policy VALUES (?, ?, ?, ?)");
        stmt.BindText(1, policyId, Statement::MakeCopy::No);
        stmt.BindText(2, expirationDate, Statement::MakeCopy::No);
        stmt.BindText(3, lastUpdateTime, Statement::MakeCopy::No);
        stmt.BindText(4, stringToken, Statement::MakeCopy::No);
        DbResult result = stmt.Step();
        if (result == DbResult::BE_SQLITE_DONE)
            return SUCCESS;
        }
    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::DeletePolicyFile(Utf8StringCR policyId)
	{
	Statement stmt;

	if (m_db.IsDbOpen())
		{
		stmt.Prepare(m_db, "DELETE FROM Policy WHERE PolicyId = ?");
		stmt.BindText(1, policyId, Statement::MakeCopy::No);
		DbResult result = stmt.Step();
		if (result == DbResult::BE_SQLITE_DONE)
			return SUCCESS;
		}
	return ERROR;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UsageDb::GetPolicyFile()
    {
    Statement stmt;

    stmt.Prepare(m_db, "SELECT * FROM Policy");

    DbResult result = stmt.Step();

    if (result != DbResult::BE_SQLITE_ROW)
        return Json::Value::GetNull();
    
    Utf8CP policy = stmt.GetValueText(3);
    
    auto jv = Json::Value(policy);

    return jv;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::SetOfflineGracePeriodStart(Utf8StringCR startTime)
	{
	Statement stmt;
	if (m_db.IsDbOpen())
		{
		stmt.Prepare(m_db, "INSERT OR REPLACE INTO OfflineGrace VALUES (?, ?)");
		stmt.BindText(1, GRACESTART, Statement::MakeCopy::No);
		stmt.BindText(2, startTime, Statement::MakeCopy::No);
		DbResult result = stmt.Step();
		if (result == DbResult::BE_SQLITE_DONE)
			{
			return SUCCESS;
			}
		}
	return ERROR;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UsageDb::GetOfflineGracePeriodStart()
	{
	// Return empty string if no grace period (nothing returned)
	Utf8String returnString = "";
	Statement stmt;
	if (m_db.IsDbOpen())
		{
		stmt.Prepare(m_db, "SELECT OfflineGracePeriodStart FROM OfflineGrace WHERE GraceId = ?");
		stmt.BindText(1, GRACESTART, Statement::MakeCopy::No);
		DbResult result = stmt.Step();
		if (result == DbResult::BE_SQLITE_ROW)
			{
			return stmt.GetValueText(0);
			}
		}
	return returnString;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::ResetOfflineGracePeriod()
	{
	if (m_db.IsDbOpen())
		{
		return SetOfflineGracePeriodStart("");
		}
	return ERROR;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::CleanUpUsages()
    {
    if (!m_db.IsDbOpen())
        return ERROR;

    auto maxRowId = GetLastRowId();

    Statement stmt;
    Utf8String sqlDeleteStatement;

    sqlDeleteStatement.Sprintf("DELETE FROM Usage WHERE rowid <= %lld", maxRowId);

    stmt.Prepare(m_db, sqlDeleteStatement.c_str());

    DbResult result = stmt.Step();

    if (result != DbResult::BE_SQLITE_DONE)
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::RecordUsage(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
                                   Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
                                   Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion, 
                                   Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
                                   Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType)
    {
    Statement stmt;

    if (m_db.IsDbOpen())
        {
        stmt.Prepare(m_db, "INSERT INTO Usage VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        stmt.BindInt64(1, ultimateSAPId);
        stmt.BindText(2, principalId, Statement::MakeCopy::No);
        stmt.BindText(3, imsId, Statement::MakeCopy::No);
        stmt.BindText(4, machineName, Statement::MakeCopy::No);
        stmt.BindText(5, machineSID, Statement::MakeCopy::No);
        stmt.BindText(6, userName, Statement::MakeCopy::No);
        stmt.BindText(7, userSID, Statement::MakeCopy::No);
        stmt.BindText(8, policyId, Statement::MakeCopy::No);
        stmt.BindText(9, securableId, Statement::MakeCopy::No);
        stmt.BindInt(10, productId);
        stmt.BindText(11, featureString, Statement::MakeCopy::No);
        stmt.BindInt64(12, productVersion);
        stmt.BindText(13, projectId, Statement::MakeCopy::No);
        stmt.BindText(14, correlationId, Statement::MakeCopy::No);
        stmt.BindText(15, eventTime, Statement::MakeCopy::No);
        stmt.BindDouble(16, schemaVersion);
        stmt.BindText(17, logPostingSource, Statement::MakeCopy::No);
        stmt.BindText(18, country, Statement::MakeCopy::No);
        stmt.BindText(19, usageType, Statement::MakeCopy::No);
        
        DbResult result = stmt.Step();

        if (result == DbResult::BE_SQLITE_DONE)
            return SUCCESS;
        }

    return ERROR;
    }
