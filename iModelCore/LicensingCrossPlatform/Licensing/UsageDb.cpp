/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/UsageDb.cpp $
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
        {
        if (UpdateDb() != SUCCESS)
            return ERROR;

        return SUCCESS;
        }

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
                         "UserId NVARCHAR(20), "
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

    // Create Feature table
    if (m_db.CreateTable("Feature",
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
                         "UsageType NVARCHAR(20), "
                         "FeatureId NVARCHAR(20), "
                         "StartDate NVARCHAR(20), "
                         "EndDate NVARCHAR(20), "
                         "UserData NVARCHAR(900)") != DbResult::BE_SQLITE_OK)
        {
        return ERROR;
        }

    // Create licensing schema table
    if (m_db.CreateTable("eimVersion",
                         "SchemaName NVARCHAR(20), "
                         "SchemaVersion REAL") != DbResult::BE_SQLITE_OK)
        {
        return ERROR;
        }

    if (SetEimVerion() != SUCCESS)
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
int64_t UsageDb::GetLastUsageRecordRowId()
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
Utf8String UsageDb::GetLastUsageRecordedTime()
    {
    Statement stmt;

    stmt.Prepare(m_db, "SELECT MAX(EventTime) FROM Usage");

    DbResult result = stmt.Step();
    if (result != DbResult::BE_SQLITE_ROW)
        return NULL;

    return stmt.GetValueText(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t UsageDb::GetUsageRecordCount()
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
int64_t UsageDb::GetLastFeatureRowId()
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT MAX(rowid) FROM Feature");

    DbResult result = stmt.Step();
    if (result != DbResult::BE_SQLITE_ROW)
        return 0;

    return stmt.GetValueInt64(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t UsageDb::GetFeatureRecordCount()
    {
    Statement stmt;
    stmt.Prepare(m_db, "SELECT COUNT(*) FROM Feature");

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
BentleyStatus UsageDb::WriteFeatureToCSVFile(BeFileNameCR path)
    {
    SCVWritter writter;
    Statement stmt;

    if (GetFeatureRecordCount() == 0)
        return SUCCESS;

    stmt.Prepare(m_db, "SELECT * FROM Feature");

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
                       stmt.GetValueText(18),
                       stmt.GetValueText(19),
                       stmt.GetValueText(20),
                       stmt.GetValueText(21),
                       stmt.GetValueText(22));

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
std::list<Json::Value> UsageDb::GetPolicyFiles(Utf8String userId)
	{
	std::list<Json::Value> policyList;

	Statement stmt;
	if (m_db.IsDbOpen())
		{
		stmt.Prepare(m_db, "SELECT PolicyFile FROM Policy WHERE UserID = ?");
		stmt.BindText(1, userId, Statement::MakeCopy::No);
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
		}
	return policyList;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken)
    {
    Statement stmt;

    if (m_db.IsDbOpen())
        {
        Utf8String stringToken = Json::FastWriter::ToString(policyToken);
        stmt.Prepare(m_db, "INSERT INTO Policy VALUES (?, ?, ?, ?, ?)");
        stmt.BindText(1, policyId, Statement::MakeCopy::No);
		stmt.BindText(2, userId, Statement::MakeCopy::No);
        stmt.BindText(3, expirationDate, Statement::MakeCopy::No);
        stmt.BindText(4, lastUpdateTime, Statement::MakeCopy::No);
        stmt.BindText(5, stringToken, Statement::MakeCopy::No);
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
BentleyStatus UsageDb::DeleteAllOtherUserPolicyFiles(Utf8StringCR policyId, Utf8StringCR userId)
	{
	Statement stmt;

	if (m_db.IsDbOpen())
		{
		stmt.Prepare(m_db, "DELETE FROM Policy WHERE UserId = ? AND PolicyId != ?");
		stmt.BindText(1, userId, Statement::MakeCopy::No);
		stmt.BindText(2, policyId, Statement::MakeCopy::No);
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

    stmt.Prepare(m_db, "SELECT PolicyFile FROM Policy");

    DbResult result = stmt.Step();

    if (result != DbResult::BE_SQLITE_ROW)
        return Json::Value::GetNull();
    
    Utf8CP policy = stmt.GetValueText(0);
    
    auto jv = Json::Value(policy);

    return jv;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UsageDb::GetPolicyFile(Utf8StringCR policyId)
	{
	Statement stmt;

	if (m_db.IsDbOpen())
		{
		stmt.Prepare(m_db, "SELECT PolicyFile FROM Policy WHERE PolicyId = ?");
		stmt.BindText(1, policyId, Statement::MakeCopy::No);
		DbResult result = stmt.Step();
		if (result == DbResult::BE_SQLITE_ROW)
			{
			Utf8String policyUtf8 = stmt.GetValueText(0);
			Json::Value policyJson = Json::Reader::DoParse(policyUtf8);
			return policyJson;
			}
		}
	return Json::Value::GetNull();
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

    auto maxRowId = GetLastUsageRecordRowId();

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
BentleyStatus UsageDb::CleanUpFeatures()
    {
    if (!m_db.IsDbOpen())
        return ERROR;

    auto maxRowId = GetLastFeatureRowId();

    Statement stmt;
    Utf8String sqlDeleteStatement;

    sqlDeleteStatement.Sprintf("DELETE FROM Feature WHERE rowid <= %lld", maxRowId);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::RecordFeature(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
                                     Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
                                     Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
                                     Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
                                     Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType, Utf8StringCR featureId,
                                     Utf8StringCR startDate, Utf8String endDate, Utf8String userData)
    {
    Statement stmt;

    if (m_db.IsDbOpen())
        {
        stmt.Prepare(m_db, "INSERT INTO Feature VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
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
        stmt.BindText(20, featureId, Statement::MakeCopy::No);
        stmt.BindText(21, startDate, Statement::MakeCopy::No);
        stmt.BindText(22, endDate, Statement::MakeCopy::No);
        stmt.BindText(23, userData, Statement::MakeCopy::No);

        DbResult result = stmt.Step();

        if (result == DbResult::BE_SQLITE_DONE)
            return SUCCESS;
        }

    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::SetEimVerion()
    {
    Statement stmt;

    if (m_db.IsDbOpen())
        {
        stmt.Prepare(m_db, "INSERT INTO eimVersion VALUES (?, ?)");
        stmt.BindText(1, LICENSE_CLIENT_SCHEMA_NAME, Statement::MakeCopy::No);
        stmt.BindDouble(2, LICENSE_CLIENT_SCHEMA_VERSION);

        DbResult result = stmt.Step();

        if (result == DbResult::BE_SQLITE_DONE)
            return SUCCESS;
        }

    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::UpdateDb()
    {
    Statement stmt;
    DbResult result;

    if (m_db.IsDbOpen())
        {
        stmt.Prepare(m_db, "SELECT * FROM eimVersion");
        result = stmt.Step();

        if (result == DbResult::BE_SQLITE_ROW)
            {
            if (stmt.GetValueDouble(1) < LICENSE_CLIENT_SCHEMA_VERSION)
                {
                Utf8String updateStatement;
                stmt.Finalize();
                updateStatement.Sprintf("UPDATE eimVersion SET SchemaVersion = %f", LICENSE_CLIENT_SCHEMA_VERSION);
                stmt.Prepare(m_db, (Utf8CP) updateStatement.c_str());
                result = stmt.Step();
                if (result == DbResult::BE_SQLITE_DONE)
                    {
                    return UpdateDbTables();
                    }
                else
                    {
                    return ERROR;
                    }
                }
            }
        }
    
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UsageDb::UpdateDbTables()
    {
    LOG.info("UpdateDbTables");
    return SUCCESS;
    }
