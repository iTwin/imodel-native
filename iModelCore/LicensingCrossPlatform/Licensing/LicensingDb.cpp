/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Utils/SCVWritter.h>

#include "LicensingDb.h"
#include "Logging.h"
#include <mutex>

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_SQLITE

std::mutex dbchangelocker;

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::OpenOrCreate(BeFileNameCR filePath)
    {
    LOG.trace("LicensingDb::OpenOrCreate");

    if (m_db.IsDbOpen())
        {
        if (filePath.GetNameUtf8().compare(m_db.GetDbFileName()) == 0)
            return SUCCESS;

        m_db.CloseDb();
        }

    if (filePath.DoesPathExist())
        {
        auto result = OpenDb(filePath);
        if (result == ERROR)
            {
            LOG.info("Database file is invalid or corrupted, attempting to delete file and re-create as a Database");

            // attempt to delete .db and .db-journal and recreate the db
            if (filePath.BeDeleteFile() == BeFileNameStatus::Success)
                {
                LOG.info("Successfully deleted .db file");

                BeFileName journalPath(filePath.GetDirectoryName());
                journalPath.AppendToPath(filePath.GetFileNameWithoutExtension().c_str());
                journalPath.AppendExtension(L"db-journal");
                if (journalPath.DoesPathExist())
                    {
                    if (journalPath.BeDeleteFile() == BeFileNameStatus::Success)
                        {
                        LOG.info("Successfully deleted .db-journal file");
                        }
                    else
                        {
                        LOG.info("Failed to delete .db-journal file");
                        return ERROR;
                        }
                    }

                // attempt to create database after deleting .db and .db-journal files
                return CreateDb(filePath);
                }
            else
                {
                LOG.info("Failed to delete the .db file");
                return ERROR;
                }
            }

        return result;
        }

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
    LOG.debugv("CreateDb result: %d", (int)result);

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

    // Create offline Usage record table
    // These values are listed as they must appear in the offline usage CSV, and in the correct order
    if (m_db.CreateTable("Usage",
                         "UltimateId BIGINT, "
                         "PrincipalId NVARCHAR(20), "
                         "ProductId INTEGER, "
                         "UsageCountryISO NVARCHAR(20), "
                         "FeatureString NVARCHAR(255), "
                         "IMSId NVARCHAR(20), "
                         "MachineName NVARCHAR(255), "
                         "ComputerSID NVARCHAR(255), "
                         "Username NVARCHAR(255), "
                         "UserSID NVARCHAR(255), "
                         "PolicyId NVARCHAR(255), "
                         "ProductVersion BIGINT, "
                         "ProjectId NVARCHAR(20), "
                         "CorrelationId NVARCHAR(20), "
                         "LogVersion BIGINT, "
                         "LogPostingSource NVARCHAR(20), "
                         "UsageType NVARCHAR(20), "
                         "StartTimeUTC NVARCHAR(20), "
                         "EndTimeUTC NVARCHAR(20), "
                         "EntryDate NVARCHAR(20), "
                         "PartitionId INTEGER") != DbResult::BE_SQLITE_OK)
        {
        LOG.error("Failed to create Usage table.");
        return ERROR;
        }

    // Create Offline Feature record table
    // NOTE: If we want to add realtime feature tracking, we need to add the following values to the DB:
    // "PrincipalId NVARCHAR(20)"
    // "PolicyId NVARCHAR(20), "
    // "SecurableId NVARCHAR(20), "
    // "EventTime NVARCHAR(20), "
    // "SchemaVersion REAL, "
    // "LogPostingSource NVARCHAR(20), "
    // "UsageType NVARCHAR(20), "

    if (m_db.CreateTable("Feature",
                         "UltimateId INTEGER, "
                         "CountryIso NVARCHAR(20), "
                         "ProductId INTEGER, "
                         "FeatureString NVARCHAR(255), "
                         "VersionNumber INTEGER, "
                         "MachineName NVARCHAR(255), "
                         "ComputerSid NVARCHAR(255), "
                         "UserName NVARCHAR(255), "
                         "UserSid NVARCHAR(255), "
                         "ImsId NVARCHAR(20), "
                         "ProjectId NVARCHAR(20), "
                         "SessionId NVARCHAR(20), "
                         "FeatureId NVARCHAR(20), "
                         "StartTime NVARCHAR(20), "
                         "EndTime NVARCHAR(20), "
                         "DurationTracked NVARCHAR(20), "
                         "MetaData NVARCHAR(900)") != DbResult::BE_SQLITE_OK)
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

    stmt.Prepare(m_db, "SELECT MAX(EntryDate) FROM Usage");

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

    // add header row
    writter.AddRow("UltimateId",
                   "PrincipalId",
                   "ProductId",
                   "UsageCountryISO",
                   "FeatureString",
                   "IMSId",
                   "MachineName",
                   "ComputerSID",
                   "Username",
                   "UserSID",
                   "PolicyId",
                   "ProductVersion",
                   "ProjectId",
                   "CorrelationId",
                   "LogVersion",
                   "LogPostingSource",
                   "UsageType",
                   "StartTimeUTC",
                   "EndTimeUTC",
                   "EntryDate",
                   "PartitionId");

    while (result == DbResult::BE_SQLITE_ROW)
        {
        writter.AddRow(stmt.GetValueInt64(0),
                       stmt.GetValueText(1),
                       stmt.GetValueInt(2),
                       stmt.GetValueText(3),
                       stmt.GetValueText(4),
                       stmt.GetValueText(5),
                       stmt.GetValueText(6),
                       stmt.GetValueText(7),
                       stmt.GetValueText(8),
                       stmt.GetValueText(9),
                       stmt.GetValueText(10),
                       stmt.GetValueInt64(11),
                       stmt.GetValueText(12),
                       stmt.GetValueText(13),
                       stmt.GetValueInt64(14),
                       stmt.GetValueText(15),
                       stmt.GetValueText(16),
                       stmt.GetValueText(17),
                       stmt.GetValueText(18),
                       stmt.GetValueText(19),
                       stmt.GetValueInt(20));

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

    // add header row
    writter.AddRow("UltimateId",
                   "CountryIso",
                   "ProductId",
                   "FeatureString",
                   "VersionNumber",
                   "MachineName",
                   "ComputerSid",
                   "UserName",
                   "UserSid",
                   "ImsId",
                   "ProjectId",
                   "SessionId",
                   "FeatureId",
                   "StartTime",
                   "EndTime",
                   "DurationTracked",
                   "MetaData");

    while (result == DbResult::BE_SQLITE_ROW)
        {
        writter.AddRow(stmt.GetValueInt64(0),
                       stmt.GetValueText(1),
                       stmt.GetValueInt(2),
                       stmt.GetValueText(3),
                       stmt.GetValueInt64(4),
                       stmt.GetValueText(5),
                       stmt.GetValueText(6),
                       stmt.GetValueText(7),
                       stmt.GetValueText(8),
                       stmt.GetValueText(9),
                       stmt.GetValueText(10),
                       stmt.GetValueText(11),
                       stmt.GetValueText(12),
                       stmt.GetValueText(13),
                       stmt.GetValueText(14),
                       stmt.GetValueText(15),
                       stmt.GetValueText(16));

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
        dbchangelocker.lock();
        Utf8String stringToken = Json::FastWriter::ToString(policyToken);
        stmt.Prepare(m_db, "INSERT INTO Policy VALUES (?, ?, ?, ?, ?, ?)");
        stmt.BindText(1, policyId, Statement::MakeCopy::No);
        stmt.BindText(2, userId, Statement::MakeCopy::No);
        stmt.BindText(3, accessKey, Statement::MakeCopy::No);
        stmt.BindText(4, expirationDate, Statement::MakeCopy::No);
        stmt.BindText(5, lastUpdateTime, Statement::MakeCopy::No);
        stmt.BindText(6, stringToken, Statement::MakeCopy::No);
        DbResult result = stmt.Step();
        dbchangelocker.unlock();
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
        dbchangelocker.lock();
        stmt.Prepare(m_db, "DELETE FROM Policy WHERE PolicyId = ?");
        stmt.BindText(1, policyId, Statement::MakeCopy::No);
        DbResult result = stmt.Step();
        dbchangelocker.unlock();
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
        dbchangelocker.lock();
        stmt.Prepare(m_db, "DELETE FROM Policy WHERE UserId = ? AND PolicyId != ?");
        stmt.BindText(1, userId, Statement::MakeCopy::No);
        stmt.BindText(2, policyId, Statement::MakeCopy::No);
        DbResult result = stmt.Step();
        dbchangelocker.unlock();
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
        dbchangelocker.lock();
        stmt.Prepare(m_db, "DELETE FROM Policy WHERE AccessKey = ? AND PolicyId != ?");
        stmt.BindText(1, accessKey, Statement::MakeCopy::No);
        stmt.BindText(2, policyId, Statement::MakeCopy::No);
        DbResult result = stmt.Step();
        dbchangelocker.unlock();
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
        dbchangelocker.lock();
        stmt.Prepare(m_db, "INSERT OR REPLACE INTO OfflineGrace VALUES (?, ?)");
        stmt.BindText(1, GRACESTART, Statement::MakeCopy::No);
        stmt.BindText(2, startTime, Statement::MakeCopy::No);
        DbResult result = stmt.Step();
        dbchangelocker.unlock();
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
    dbchangelocker.lock();
    sqlDeleteStatement.Sprintf("DELETE FROM Usage WHERE rowid <= %lld", maxRowId);

    stmt.Prepare(m_db, sqlDeleteStatement.c_str());

    DbResult result = stmt.Step();
    dbchangelocker.unlock();
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
    dbchangelocker.lock();
    sqlDeleteStatement.Sprintf("DELETE FROM Feature WHERE rowid <= %lld", maxRowId);

    stmt.Prepare(m_db, sqlDeleteStatement.c_str());

    DbResult result = stmt.Step();
    dbchangelocker.unlock();
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
BentleyStatus LicensingDb::RecordUsage
    (
    int64_t ultimateId,
    Utf8StringCR principalId,
    int productId,
    Utf8StringCR usageCountryIso,
    Utf8String featureString,
    Utf8StringCR imsId,
    Utf8String machineName,
    Utf8StringCR machineSID, // aka deviceId aka computerSID
    Utf8StringCR userName,
    Utf8StringCR userSID,
    Utf8StringCR policyId,
    int64_t productVersion,
    Utf8StringCR projectId,
    Utf8StringCR correlationId, // aka sessionId
    int64_t logVersion, // aka schemaVersion
    Utf8StringCR logPostingSource,
    Utf8StringCR usageType,
    Utf8StringCR startTimeUtc,
    Utf8StringCR endTimeUtc,
    Utf8StringCR entryDate, // aka eventTimeZ
    int partitionId // 1
    )
    {
    LOG.info("LicensingDb::RecordUsage");

    // NOTE: if we want to add realtime feature log posting, we must add the following fields:
    // Utf8StringCR securableId

    Statement stmt;

    if (m_db.IsDbOpen())
        {
        dbchangelocker.lock();
        stmt.Prepare(m_db, "INSERT INTO Usage VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        stmt.BindInt64(1, ultimateId);
        stmt.BindText(2, principalId, Statement::MakeCopy::No);
        stmt.BindInt(3, productId);
        stmt.BindText(4, usageCountryIso, Statement::MakeCopy::No);
        stmt.BindText(5, featureString, Statement::MakeCopy::No);
        stmt.BindText(6, imsId, Statement::MakeCopy::No);
        stmt.BindText(7, machineName, Statement::MakeCopy::No);
        stmt.BindText(8, machineSID, Statement::MakeCopy::No);
        stmt.BindText(9, userName, Statement::MakeCopy::No);
        stmt.BindText(10, userSID, Statement::MakeCopy::No);
        stmt.BindText(11, policyId, Statement::MakeCopy::No);
        stmt.BindInt64(12, productVersion);
        stmt.BindText(13, projectId, Statement::MakeCopy::No);
        stmt.BindText(14, correlationId, Statement::MakeCopy::No);
        stmt.BindInt64(15, logVersion);
        stmt.BindText(16, logPostingSource, Statement::MakeCopy::No);
        stmt.BindText(17, usageType, Statement::MakeCopy::No);
        stmt.BindText(18, startTimeUtc, Statement::MakeCopy::No);
        stmt.BindText(19, endTimeUtc, Statement::MakeCopy::No);
        stmt.BindText(20, entryDate, Statement::MakeCopy::No);
        stmt.BindInt(21, partitionId);

        DbResult result = stmt.Step();
        dbchangelocker.unlock();
        if (result == DbResult::BE_SQLITE_DONE)
            return SUCCESS;

        LOG.errorv("LicensingDb::RecordUsage - Failed to insert usage into the databse. BE_SQLITE error %d", result);
        }

    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LicensingDb::RecordFeature
    (
    int64_t ultimateId, // aka ultimateSAPId
    Utf8StringCR countryIso,
    int productId,
    Utf8String featureString,
    int64_t productVersion, // aka versionNumber
    Utf8String machineName, // aka deviceId aka computerSID
    Utf8StringCR machineSID,
    Utf8StringCR userName,
    Utf8StringCR userSID,
    Utf8StringCR imsId,
    Utf8StringCR projectId,
    Utf8String correlationId, // aka sessionId
    Utf8StringCR featureId,
    Utf8StringCR startTime, // aka startTimeUtc
    Utf8String endTime, // aka endTimeUtc
    bool durationTracked,
    Utf8StringCR userData // aka metaData
    )
    {
    LOG.info("LicensingDb::RecordFeature");

    // NOTE: if we want to add realtime feature log posting, we must add the following fields:
    // Utf8StringCR principalId,
    // Utf8StringCR policyId,
    // Utf8StringCR securableId,
    // Utf8StringCR eventTime,
    // double schemaVersion,
    // Utf8StringCR logPostingSource,
    // Utf8StringCR usageType,

    Statement stmt;

    if (m_db.IsDbOpen())
        {
        dbchangelocker.lock();
        stmt.Prepare(m_db, "INSERT INTO Feature VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        stmt.BindInt64(1, ultimateId);
        stmt.BindText(2, countryIso, Statement::MakeCopy::No);
        stmt.BindInt(3, productId);
        stmt.BindText(4, featureString, Statement::MakeCopy::No);
        stmt.BindInt64(5, productVersion);
        stmt.BindText(6, machineName, Statement::MakeCopy::No);
        stmt.BindText(7, machineSID, Statement::MakeCopy::No);
        stmt.BindText(8, userName, Statement::MakeCopy::No);
        stmt.BindText(9, userSID, Statement::MakeCopy::No);
        stmt.BindText(10, imsId, Statement::MakeCopy::No);
        stmt.BindText(11, projectId, Statement::MakeCopy::No);
        stmt.BindText(12, correlationId, Statement::MakeCopy::No);
        stmt.BindText(13, featureId, Statement::MakeCopy::No);
        stmt.BindText(14, startTime, Statement::MakeCopy::No);
        stmt.BindText(15, endTime, Statement::MakeCopy::No);
        durationTracked ? stmt.BindText(16, "true", Statement::MakeCopy::No) : stmt.BindText(16, "false", Statement::MakeCopy::No);
        stmt.BindText(17, userData, Statement::MakeCopy::No);

        DbResult result = stmt.Step();
        dbchangelocker.unlock();
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
        dbchangelocker.lock();
        stmt.Prepare(m_db, "INSERT INTO eimVersion VALUES (?, ?)");
        stmt.BindText(1, LICENSE_CLIENT_SCHEMA_NAME, Statement::MakeCopy::No);
        stmt.BindDouble(2, LICENSE_CLIENT_SCHEMA_VERSION);

        DbResult result = stmt.Step();
        dbchangelocker.unlock();
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
                dbchangelocker.lock();
                //Statement updateStmt;
                stmt.Finalize();
                updateStatement.Sprintf("UPDATE eimVersion SET SchemaVersion = %f WHERE rowid = 1", LICENSE_CLIENT_SCHEMA_VERSION);
                stmt.Prepare(m_db, (Utf8CP) updateStatement.c_str());
                result = stmt.Step();
                dbchangelocker.unlock();
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
