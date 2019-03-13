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
    // Function return value is passed into mock method
    void MockOpenOrCreate(BentleyStatus mocked) { m_mockedOpenOrCreate = mocked; }
    void MockWriteUsageToCSVFile(BentleyStatus mocked) { m_mockedWriteUsageToCSVFile = mocked; }
    void MockWriteFeatureToCSVFile(BentleyStatus mocked) { m_mockedWriteFeatureToCSVFile = mocked; }
    void MockAddOrUpdatePolicyFile(BentleyStatus mocked) { m_mockedAddOrUpdatePolicyFile = mocked; }
    void MockDeletePolicyFile(BentleyStatus mocked) { m_mockedDeletePolicyFile = mocked; }
    void MockDeleteAllOtherPolicyFilesByUser(BentleyStatus mocked) { m_mockedDeleteAllOtherPolicyFilesByUser = mocked; }
    void MockDeleteAllOtherPolicyFilesByKey(BentleyStatus mocked) { m_mockedDeleteAllOtherPolicyFilesByKey = mocked; }
    void MockSetOfflineGracePeriodStart(BentleyStatus mocked) { m_mockedSetOfflineGracePeriodStart = mocked; }
    void MockGetOfflineGracePeriodStart(Utf8String mocked) { m_mockedGetOfflineGracePeriodStart = mocked; }
    void MockResetOfflineGracePeriod(BentleyStatus mocked) { m_mockedResetOfflineGracePeriod = mocked; }
    void MockGetLastUsageRecordedTime(Utf8String mocked) { m_mockedGetLastUsageRecordedTime = mocked; }
    void MockGetUsageRecordCount(int64_t mocked) { m_mockedGetUsageRecordCount = mocked; }
    void MockGetFeatureRecordCount(int64_t mocked) { m_mockedGetFeatureRecordCount = mocked; }
    void MockCleanUpUsages(BentleyStatus mocked) { m_mockedCleanUpUsages = mocked; }
    void MockCleanUpFeatures(BentleyStatus mocked) { m_mockedCleanUpFeatures = mocked; }
    void MockRecordUsage(BentleyStatus mocked) { m_mockedRecordUsage = mocked; }
    void MockRecordFeature(BentleyStatus mocked) { m_mockedFeatureUsage = mocked; }

    // get number of times the mocked functions are called
    int OpenOrCreateCount() const { return m_openOrCreateCalls; }
    int CloseCount() const { return m_closeCalls; }
    int WriteUsageToCSVFileCount() const { return m_writeUsageToCSVFileCalls; }
    int WriteFeatureToCSVFileCount() const { return m_writeFeatureToCSVFileCalls; }
    int GetPolicyFilesCount() const { return m_getPolicyFilesCalls; }
    int GetPolicyFilesByUserCount(Utf8StringCR userId);
    int GetPolicyFilesByKeyCount(Utf8StringCR accessKey);
    int AddOrUpdatePolicyFileCount() const { return m_addOrUpdatePolicyFileCalls; }
    int DeletePolicyFileCount() const { return m_deletePolicyFileCalls; }
    int DeleteAllOtherPolicyFilesByUserCount() const { return m_deleteAllOtherPolicyFilesByUserCalls; }
    int DeleteAllOtherPolicyFilesByKeyCount() const { return m_deleteAllOtherPolicyFilesByKeyCalls; }
    int GetPolicyFileCount() const { return m_getPolicyFileCalls; }
    int GetPolicyFileByIdCount(Utf8StringCR policyId);
    int SetOfflineGracePeriodStartCount() const { return m_setOfflineGracePeriodStartCalls; }
    int GetOfflineGracePeriodStartCount() const { return m_getOfflineGracePeriodStartCalls; }
    int ResetOfflineGracePeriod() const { return m_resetOfflineGracePeriodCalls; }
    int GetLastUsageRecordedTime() const { return m_getLastUsageRecordedTimeCalls; }
    int GetUsageRecordCountCount() const { return m_getUsageRecordCountCalls; }
    int GetFeatureRecordCountCount() const { return m_getFeatureRecordCountCalls; }
    int RecordUsageCount() const { return m_recordUsageCalls; }
    int RecordFeatureCount() const { return m_recordFeatureCalls; }

    // set mock values
    void MockPolicyFiles(std::list<Json::Value> policyFiles);
    void MockUserPolicyFiles(Utf8StringCR userId, std::list<Json::Value> policyFiles);
    void MockKeyPolicyFiles(Utf8StringCR accessKey, std::list<Json::Value> policyFiles);
    void MockIdPolicyFile(Utf8StringCR policyId, Json::Value policyFile);

private:
    // mock methods
    BentleyStatus m_mockedOpenOrCreate = BSIERROR; // just picked a status, if testing, need to explicitly set this with the mock call
    BentleyStatus m_mockedWriteUsageToCSVFile = BSIERROR;
    BentleyStatus m_mockedWriteFeatureToCSVFile = BSIERROR;
    BentleyStatus m_mockedAddOrUpdatePolicyFile = BSIERROR;
    BentleyStatus m_mockedDeletePolicyFile = BSIERROR;
    BentleyStatus m_mockedDeleteAllOtherPolicyFilesByUser = BSIERROR;
    BentleyStatus m_mockedDeleteAllOtherPolicyFilesByKey = BSIERROR;
    BentleyStatus m_mockedSetOfflineGracePeriodStart = BSIERROR;
    Utf8String m_mockedGetOfflineGracePeriodStart = "";
    BentleyStatus m_mockedResetOfflineGracePeriod = BSIERROR;
    Utf8String m_mockedGetLastUsageRecordedTime = "";
    int64_t m_mockedGetUsageRecordCount = 0;
    int64_t m_mockedGetFeatureRecordCount = 0;
    BentleyStatus m_mockedCleanUpUsages = BSIERROR;
    BentleyStatus m_mockedCleanUpFeatures = BSIERROR;
    BentleyStatus m_mockedRecordUsage = BSIERROR;
    BentleyStatus m_mockedFeatureUsage = BSIERROR;

    // number of times a mocked function is called
    int m_openOrCreateCalls = 0;
    int m_closeCalls = 0;
    int m_writeUsageToCSVFileCalls = 0;
    int m_writeFeatureToCSVFileCalls = 0;
    int m_getPolicyFilesCalls = 0;
    std::map<Utf8String, int> m_getPolicyFilesByUserCallsMap;
    std::map<Utf8String, int> m_getPolicyFilesByKeyCallsMap;
    int m_getPolicyFileCalls = 0;
    std::map<Utf8String, int> m_getPolicyFileByIdCallsMap;
    int m_addOrUpdatePolicyFileCalls = 0;
    int m_deletePolicyFileCalls = 0;
    int m_deleteAllOtherPolicyFilesByUserCalls = 0;
    int m_deleteAllOtherPolicyFilesByKeyCalls = 0;
    int m_setOfflineGracePeriodStartCalls = 0;
    int m_getOfflineGracePeriodStartCalls = 0;
    int m_resetOfflineGracePeriodCalls = 0;
    int m_getLastUsageRecordedTimeCalls = 0;
    int m_getUsageRecordCountCalls = 0;
    int m_getFeatureRecordCountCalls = 0;
    int m_cleanUpUsagesCalls = 0;
    int m_cleanUpFeaturesCalls = 0;
    int m_recordUsageCalls = 0;
    int m_recordFeatureCalls = 0;

    // mock values
    bool m_isOpen = false;
    std::list<Json::Value> m_policyList;
    std::map<Utf8String, std::list<Json::Value>> m_userPolicyListMap;
    std::map<Utf8String, std::list<Json::Value>> m_keyPolicyListMap;
    std::map<Utf8String, Json::Value> m_idPolicyFileMap;
    };

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE
