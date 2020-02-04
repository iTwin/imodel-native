/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <BeSQLite/BeSQLite.h>
#include "Policy.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef std::shared_ptr<struct ILicensingDb> ILicensingDbPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ILicensingDb
    {
public:
    virtual BentleyStatus OpenOrCreate(BeFileNameCR filePath) = 0;
    virtual void Close() = 0;
    virtual bool IsDbOpen() = 0;

    virtual BentleyStatus WriteUsageToCSVFile(BeFileNameCR path) = 0;
    virtual BentleyStatus WriteFeatureToCSVFile(BeFileNameCR path) = 0;

    virtual std::list<std::shared_ptr<Policy>> GetPolicyFiles() = 0;
    virtual std::list<std::shared_ptr<Policy>> GetValidPolicyFilesForUser(Utf8StringCR userId) = 0;
	virtual std::list<Json::Value> GetAllCheckouts() = 0;
    virtual std::list<Json::Value> GetPolicyFilesByKey(Utf8StringCR accessKey) = 0;
    virtual BentleyStatus AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR accessKey, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken, Utf8StringCR projectId) = 0;
	virtual BentleyStatus AddOrUpdateCheckout(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR accessKey, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken) = 0;
    virtual BentleyStatus DeletePolicyFile(Utf8StringCR policyId) = 0;
    virtual BentleyStatus DeleteAllOtherPolicyFilesByUser(Utf8StringCR policyId, Utf8StringCR userId) = 0;
    virtual BentleyStatus DeleteAllOtherPolicyFilesByKey(Utf8StringCR policyId, Utf8StringCR accessKey) = 0;
    virtual BentleyStatus DeleteAllOtherPolicyFilesByProject(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR projectId) = 0;

    virtual Json::Value GetPolicyFile() = 0;
    virtual Json::Value GetPolicyFile(Utf8StringCR policyId) = 0;

    virtual BentleyStatus SetOfflineGracePeriodStart(Utf8StringCR startTime) = 0;
    virtual Utf8String GetOfflineGracePeriodStart() = 0;
    virtual BentleyStatus ResetOfflineGracePeriod() = 0;

    virtual Utf8String GetLastUsageRecordedTime() = 0;
    virtual int64_t GetUsageRecordCount() = 0;
    virtual int64_t GetFeatureRecordCount() = 0;

    virtual BentleyStatus CleanUpUsages() = 0;
    virtual BentleyStatus CleanUpFeatures() = 0;

    virtual BentleyStatus RecordUsage
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
        ) = 0;

    virtual BentleyStatus RecordFeature
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
        ) = 0;

    virtual ~ILicensingDb() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
