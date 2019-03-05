/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ILicensingDb.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <BeSQLite/BeSQLite.h>

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

    virtual std::list<Json::Value> GetPolicyFiles() = 0;
    virtual std::list<Json::Value> GetPolicyFilesByUser(Utf8StringCR userId) = 0;
    virtual std::list<Json::Value> GetPolicyFilesByKey(Utf8StringCR accessKey) = 0;
    virtual BentleyStatus AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR accessKey, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken) = 0;
    virtual BentleyStatus DeletePolicyFile(Utf8StringCR policyId) = 0;
    virtual BentleyStatus DeleteAllOtherPolicyFilesByUser(Utf8StringCR policyId, Utf8StringCR userId) = 0;
    virtual BentleyStatus DeleteAllOtherPolicyFilesByKey(Utf8StringCR policyId, Utf8StringCR accessKey) = 0;

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

    virtual BentleyStatus RecordUsage(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
        Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
        Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
        Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
        Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType) = 0;

    virtual BentleyStatus RecordFeature(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
        Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
        Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
        Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
        Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType, Utf8StringCR featureId,
        Utf8StringCR startDate, Utf8String endDate, Utf8String userData) = 0;

    virtual ~ILicensingDb() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
