/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/LicensingDb.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Utils/SCVWritter.h>

#include "LicensingDb.h"
#include "Logging.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_SQLITE

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::OpenOrCreate(BeFileNameCR filePath)
    {
    LOG.info("OpenorCreate");

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
BentleyStatus LicensingDb::OpenDb(BeFileNameCR filePath)
    {
    LOG.info("OpenDb");

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
bool LicensingDb::IsDbOpen()
    {
    LOG.debug("IsDbOpen");
    return m_db.IsDbOpen();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::CreateDb(BeFileNameCR filePath)
    {
    LOG.debug("CreateDb");

    DbResult result = m_db.CreateNewDb(filePath);

    if (result != DbResult::BE_SQLITE_OK)
        return ERROR;

    return SetUpTables();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::SetUpTables()
    {
    LOG.info("SetUpTables");

    // Create Policy table
    if (m_db.CreateTable("Policy",
                         "PolicyId NVARCHAR(20) PRIMARY KEY, "
                         "UserId NVARCHAR(40), "
                         "AccessKey NVARCHAR(40), "
                         "ExpirationDate NVARCHAR(20), "
                         "LastUpdateTime NVARCHAR(20), "
                         "PolicyFile NVARCHAR(900)") != DbResult::BE_SQLITE_OK)
        {
        LOG.error("Failed to create Policy table.");
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
        LOG.error("Failed to create Usage table.");
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
        LOG.error("Failed to create Feature table.");
        return ERROR;
        }

    // Create licensing schema table
    if (m_db.CreateTable("eimVersion",
                         "SchemaName NVARCHAR(20), "
                         "SchemaVersion REAL") != DbResult::BE_SQLITE_OK)
        {
        LOG.error("Failed to create eimVersion (schema version) table.");
        return ERROR;
        }

    if (SetEimVersion() != SUCCESS)
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
BentleyStatus LicensingDb::SetUpOfflineGraceTable()
    {
    if (m_db.CreateTable("OfflineGrace",
        "GraceId NVARCHAR(20) PRIMARY KEY, "
        "OfflineGracePeriodStart NVARCHAR(20)") != DbResult::BE_SQLITE_OK)
        {
        LOG.error("Failed to create OfflineGrace table.");
        return ERROR;
        }

    return ResetOfflineGracePeriod();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LicensingDb::Close()
    {
    LOG.debug("LicensingDb::Close");
    m_db.CloseDb();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t LicensingDb::GetLastUsageRecordRowId()
    {
    LOG.debug("GetLastUsageRecordRowId");

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
Utf8String LicensingDb::GetLastUsageRecordedTime()
    {
    LOG.debug("GetLastUsageRecordedTime");
    
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
int64_t LicensingDb::GetUsageRecordCount()
    {
    LOG.debug("GetUsageRecordCount");

    Statement stmt;
    stmt.Prepare(m_db, "SELECT COUNT(*) FROM Usage");

    DbResult result = stmt.Step();
    if (result != DbResult::BE_SQLITE_ROW)
        return -1;

    return stmt.GetValueInt64(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t LicensingDb::GetLastFeatureRowId()
    {
    LOG.debug("GetLastFeatureRowId");

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
int64_t LicensingDb::GetFeatureRecordCount()
    {
    LOG.debug("GetFeatureRecordCount");

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
BentleyStatus LicensingDb::WriteUsageToCSVFile(BeFileNameCR path)
    {
    LOG.info("WriteUsageToCSVFile");

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
        {
        LOG.errorv("Failed to copy usages to CVS file. BE_SQLITE error %d", result);
        return ERROR;
        }

    return writter.WriteToFile(path);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::WriteFeatureToCSVFile(BeFileNameCR path)
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
        {
        LOG.errorv("Failed to copy features to CVS file. BE_SQLITE error %d", result);
        return ERROR;
        }

    return writter.WriteToFile(path);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<Json::Value> LicensingDb::GetPolicyFiles()
    {
    LOG.info("GetPolicyFiles");

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
std::list<Json::Value> LicensingDb::GetPolicyFilesByUser(Utf8StringCR userId)
    {
    LOG.info("GetPolicyFilesByUser(userId)");

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
std::list<Json::Value> LicensingDb::GetPolicyFilesByKey(Utf8StringCR accessKey)
    {
    LOG.info("GetPolicyFilesByKey(accessKey)");

    std::list<Json::Value> policyList;

    Statement stmt;
    if (m_db.IsDbOpen())
        {
        stmt.Prepare(m_db, "SELECT PolicyFile FROM Policy WHERE AccessKey = ?");
        stmt.BindText(1, accessKey, Statement::MakeCopy::No);
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
BentleyStatus LicensingDb::AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR accessKey, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken)
    {
    LOG.debug("AddOrUpdatePolicyFile");

    Statement stmt;

    if (m_db.IsDbOpen())
        {
        Utf8String stringToken = Json::FastWriter::ToString(policyToken);
        stmt.Prepare(m_db, "INSERT INTO Policy VALUES (?, ?, ?, ?, ?, ?)");
        stmt.BindText(1, policyId, Statement::MakeCopy::No);
        stmt.BindText(2, userId, Statement::MakeCopy::No);
        stmt.BindText(3, accessKey, Statement::MakeCopy::No);
        stmt.BindText(4, expirationDate, Statement::MakeCopy::No);
        stmt.BindText(5, lastUpdateTime, Statement::MakeCopy::No);
        stmt.BindText(6, stringToken, Statement::MakeCopy::No);
        DbResult result = stmt.Step();
        if (result == DbResult::BE_SQLITE_DONE)
            return SUCCESS;
        }
    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::DeletePolicyFile(Utf8StringCR policyId)
    {
    LOG.debug("DeletePolicyFile");

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
BentleyStatus LicensingDb::DeleteAllOtherPolicyFilesByUser(Utf8StringCR policyId, Utf8StringCR userId)
    {
    LOG.debug("DeleteAllOtherPolicyFilesByUser");

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
BentleyStatus LicensingDb::DeleteAllOtherPolicyFilesByKey(Utf8StringCR policyId, Utf8StringCR accessKey)
    {
    LOG.debug("DeleteAllOtherPolicyFilesByKey");

    Statement stmt;

    if (m_db.IsDbOpen())
        {
        stmt.Prepare(m_db, "DELETE FROM Policy WHERE AccessKey = ? AND PolicyId != ?");
        stmt.BindText(1, accessKey, Statement::MakeCopy::No);
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
Json::Value LicensingDb::GetPolicyFile()
    {
    LOG.debug("GetPolicyFile");

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
Json::Value LicensingDb::GetPolicyFile(Utf8StringCR policyId)
    {
    LOG.debug("GetPolicyFile (policyId)");

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
BentleyStatus LicensingDb::SetOfflineGracePeriodStart(Utf8StringCR startTime)
    {
    LOG.debug("SetOfflineGracePeriodStart");

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
        LOG.errorv("SetOfflineGracePeriodStart - Failed to set offline grace period. BE_SQLITE error %d", result);
        }
    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String LicensingDb::GetOfflineGracePeriodStart()
    {
    LOG.debug("GetOfflineGracePeriodStart");

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
BentleyStatus LicensingDb::ResetOfflineGracePeriod()
    {
    LOG.debug("ResetOfflineGracePeriod");

    if (m_db.IsDbOpen())
        {
        return SetOfflineGracePeriodStart("");
        }
    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::CleanUpUsages()
    {
    LOG.info("CleanUpUsages");

    if (!m_db.IsDbOpen())
        {
        LOG.error("CleanUpUsages - Database is not open.");
        return ERROR;
        }

    auto maxRowId = GetLastUsageRecordRowId();

    Statement stmt;
    Utf8String sqlDeleteStatement;

    sqlDeleteStatement.Sprintf("DELETE FROM Usage WHERE rowid <= %lld", maxRowId);

    stmt.Prepare(m_db, sqlDeleteStatement.c_str());

    DbResult result = stmt.Step();

    if (result != DbResult::BE_SQLITE_DONE)
        {
        LOG.errorv("CleanUpUsages - Failed to remove usages. BE_SQLITE error %d", result);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::CleanUpFeatures()
    {
    LOG.info("CleanUpFeatures");

    if (!m_db.IsDbOpen())
        {
        LOG.error("CleanUpFeatures - Database is not open.");
        return ERROR;
        }

    auto maxRowId = GetLastFeatureRowId();

    Statement stmt;
    Utf8String sqlDeleteStatement;

    sqlDeleteStatement.Sprintf("DELETE FROM Feature WHERE rowid <= %lld", maxRowId);

    stmt.Prepare(m_db, sqlDeleteStatement.c_str());

    DbResult result = stmt.Step();

    if (result != DbResult::BE_SQLITE_DONE)
        {
        LOG.errorv("CleanUpFeatures - Failed to remove features. BE_SQLITE error %d", result);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::RecordUsage(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
                                   Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
                                   Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion, 
                                   Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
                                   Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType)
    {
    LOG.info("LicensingDb::RecordUsage");

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

        LOG.errorv("LicensingDb::RecordUsage - Failed to insert usage into the databse. BE_SQLITE error %d", result);
        }

    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::RecordFeature(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
                                     Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
                                     Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
                                     Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
                                     Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType, Utf8StringCR featureId,
                                     Utf8StringCR startDate, Utf8String endDate, Utf8String userData)
    {
    LOG.info("LicensingDb::RecordFeature");

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

        LOG.errorv("LicensingDb::RecordFeature - Failed to insert feature into the databse. BE_SQLITE error %d", result);
        }

    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::SetEimVersion()
    {
    LOG.debug("SetEimVersion");

    Statement stmt;

    if (m_db.IsDbOpen())
        {
        stmt.Prepare(m_db, "INSERT INTO eimVersion VALUES (?, ?)");
        stmt.BindText(1, LICENSE_CLIENT_SCHEMA_NAME, Statement::MakeCopy::No);
        stmt.BindDouble(2, LICENSE_CLIENT_SCHEMA_VERSION);

        DbResult result = stmt.Step();

        if (result == DbResult::BE_SQLITE_DONE)
            return SUCCESS;

        LOG.errorv("Failed to set eimVersion (schema version) value. BE_SQLITE error %d.", result);
        }

    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::UpdateDb()
    {
    LOG.debug("LicensingDb::UpdateDb");

    Statement stmt;
    DbResult result;

    if (m_db.IsDbOpen())
        {
        stmt.Prepare(m_db, "SELECT * FROM eimVersion");
        result = stmt.Step();

        if (result == DbResult::BE_SQLITE_ROW)
            {
            double schemaVersion = stmt.GetValueDouble(1);
            if (schemaVersion < LICENSE_CLIENT_SCHEMA_VERSION)
                {
                LOG.infov("UpdateDb - Updating schema version %f to %f...", schemaVersion, LICENSE_CLIENT_SCHEMA_VERSION);
                Utf8String updateStatement;
                //Statement updateStmt;
                stmt.Finalize();
                updateStatement.Sprintf("UPDATE eimVersion SET SchemaVersion = %f WHERE rowid = 1", LICENSE_CLIENT_SCHEMA_VERSION);
                stmt.Prepare(m_db, (Utf8CP) updateStatement.c_str());
                result = stmt.Step();
                if (result == DbResult::BE_SQLITE_DONE)
                    {
                    return UpdateDbTables();
                    }
                else
                    {
                    LOG.errorv("UpdateDb - Updating schema version failed. BE_SQLITE result  %d", result);
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
BentleyStatus LicensingDb::UpdateDbTables()
    {
    LOG.info("UpdateDbTables");
    return SUCCESS;
    }
