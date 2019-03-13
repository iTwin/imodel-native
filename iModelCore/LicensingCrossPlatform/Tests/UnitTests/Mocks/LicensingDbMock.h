/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Mocks/LicensingDbMock.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <map>
#include <Licensing/Licensing.h>
#include "../../../Licensing/ILicensingDb.h"

#include "../TestsHelper.h"

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE

struct LicensingDbMock : ILicensingDb
    {
public:
    // ILicensingDb API
    BentleyStatus OpenOrCreate(BeFileNameCR filePath) override;
    void Close() override;
    bool IsDbOpen() override;

    BentleyStatus WriteUsageToCSVFile(BeFileNameCR path) override;
    BentleyStatus WriteFeatureToCSVFile(BeFileNameCR path) override;

    std::list<Json::Value> GetPolicyFiles() override;
    std::list<Json::Value> GetPolicyFilesByUser(Utf8StringCR userId) override;
    std::list<Json::Value> GetPolicyFilesByKey(Utf8StringCR accessKey) override;
    BentleyStatus AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR accessKey, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken) override;
    BentleyStatus DeletePolicyFile(Utf8StringCR policyId) override;
    BentleyStatus DeleteAllOtherPolicyFilesByUser(Utf8StringCR policyId, Utf8StringCR userId) override;
    BentleyStatus DeleteAllOtherPolicyFilesByKey(Utf8StringCR policyId, Utf8StringCR accessKey) override;

    Json::Value GetPolicyFile() override;
    Json::Value GetPolicyFile(Utf8StringCR policyId) override;

    BentleyStatus SetOfflineGracePeriodStart(Utf8StringCR startTime) override;
    Utf8String GetOfflineGracePeriodStart() override;
    BentleyStatus ResetOfflineGracePeriod() override;

    Utf8String GetLastUsageRecordedTime() override;
    int64_t GetUsageRecordCount() override;
    int64_t GetFeatureRecordCount() override;

    BentleyStatus CleanUpUsages() override;
    BentleyStatus CleanUpFeatures() override;

    BentleyStatus RecordUsage(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
        Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
        Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
        Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
        Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType) override;

    BentleyStatus RecordFeature(
        int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
        Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
        Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
        Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
        Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType, Utf8StringCR featureId,
        Utf8StringCR startDate, Utf8String endDate, Utf8String userData) override;

    // Mocking/Testing API
    void MockOpenOrCreate(BentleyStatus mocked) { m_mockedOpenOrCreate = mocked; }
    int OpenOrCreateCount() const { return m_openOrCreateCalls; }
    int CloseCount() const { return m_closeCalls; }
    int RecordUsageCount() const { return m_recordUsageCalls; }
    void MockUserPolicyFiles(Utf8StringCR userId, std::list<Json::Value> policyFiles);
    int GetPolicyFilesByUserCount(Utf8StringCR userId);

private:
    int m_openOrCreateCalls = 0;
    BentleyStatus m_mockedOpenOrCreate = BSIERROR; // just picked a status, if testing, need to explicitly set this with the mock call
    bool m_isOpen = false;
    int m_closeCalls = 0;
    int m_recordUsageCalls = 0;
    std::map<Utf8String, std::list<Json::Value>> m_userPolicyMap;
    std::map<Utf8String, int> m_getPolicyFilesCountMap;
    };

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE
