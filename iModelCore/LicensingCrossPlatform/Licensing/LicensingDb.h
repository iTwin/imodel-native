/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ILicensingDb.h"

#include <Licensing/Licensing.h>
#include <BeSQLite/BeSQLite.h>

#define LICENSE_CLIENT_SCHEMA_NAME      "LICENSINGSCHEMA"
#define LICENSE_CLIENT_SCHEMA_VERSION   3.0  //DB version 2.0 handles checkouts, 3.0 handles project policies
#define LOG_VERSION 1
#define PARTITION_ID 1

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct LicensingDb : ILicensingDb
    {
private:
    BeSQLite::Db m_db;

    const Utf8String GRACESTART = "GRACESTART";

    BentleyStatus OpenDb(BeFileNameCR filePath);

    BentleyStatus CreateDb(BeFileNameCR filePath);

    BentleyStatus SetUpOfflineGraceTable();
    BentleyStatus SetUpTables();

    BentleyStatus SetEimVersion();
    BentleyStatus UpdateDb();
    BentleyStatus UpdateDbTables(double startingSchema);

    int64_t GetLastUsageRecordRowId();
    int64_t GetLastFeatureRowId();

public:
    LICENSING_EXPORT BentleyStatus OpenOrCreate(BeFileNameCR filePath);

    LICENSING_EXPORT void Close();

    LICENSING_EXPORT bool IsDbOpen();

    LICENSING_EXPORT BentleyStatus WriteUsageToCSVFile(BeFileNameCR path);
    LICENSING_EXPORT BentleyStatus WriteFeatureToCSVFile(BeFileNameCR path);

    LICENSING_EXPORT std::list<std::shared_ptr<Policy>> GetPolicyFiles();
    LICENSING_EXPORT std::list<std::shared_ptr<Policy>> GetValidPolicyFilesForUser(Utf8StringCR userId);
	LICENSING_EXPORT std::list<Json::Value> GetAllCheckouts();
    LICENSING_EXPORT std::list<Json::Value> GetPolicyFilesByKey(Utf8StringCR accessKey);
    LICENSING_EXPORT BentleyStatus AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR accessKey, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken, Utf8StringCR projectId);
	LICENSING_EXPORT BentleyStatus AddOrUpdateCheckout(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR accessKey, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken);
    LICENSING_EXPORT BentleyStatus DeletePolicyFile(Utf8StringCR policyId);
    LICENSING_EXPORT BentleyStatus DeleteAllOtherPolicyFilesByUser(Utf8StringCR policyId, Utf8StringCR userId);
    LICENSING_EXPORT BentleyStatus DeleteAllOtherPolicyFilesByKey(Utf8StringCR policyId, Utf8StringCR accessKey);
    LICENSING_EXPORT BentleyStatus DeleteAllOtherPolicyFilesByProject(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR projectId);
    LICENSING_EXPORT BentleyStatus DeleteLocalCheckout(Utf8StringCR productId);

    LICENSING_EXPORT Json::Value GetPolicyFile();
    LICENSING_EXPORT Json::Value GetPolicyFile(Utf8StringCR policyId);

    LICENSING_EXPORT BentleyStatus SetOfflineGracePeriodStart(Utf8StringCR startTime);
    LICENSING_EXPORT Utf8String GetOfflineGracePeriodStart();
    LICENSING_EXPORT BentleyStatus ResetOfflineGracePeriod();

    LICENSING_EXPORT Utf8String GetLastUsageRecordedTime();
    LICENSING_EXPORT int64_t GetUsageRecordCount();
    LICENSING_EXPORT int64_t GetFeatureRecordCount();

    LICENSING_EXPORT BentleyStatus CleanUpUsages();
    LICENSING_EXPORT BentleyStatus CleanUpFeatures();

    LICENSING_EXPORT BentleyStatus RecordUsage
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
        Utf8StringCR entryDate, // aka eventTime
        int partitionId // 1
        );

    LICENSING_EXPORT BentleyStatus RecordFeature
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
        Utf8StringCR startTime,
        Utf8String endTime,
        bool durationTracked,
        Utf8StringCR userData // aka metaData
        );

    };

END_BENTLEY_LICENSING_NAMESPACE
