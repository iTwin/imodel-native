#include "LicensingDbMock.h"

#include <exception>

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

BentleyStatus LicensingDbMock::OpenOrCreate(BeFileNameCR filePath)
    {
    m_openOrCreateCalls++;

    if (m_mockedOpenOrCreate == SUCCESS)
        {
        m_isOpen = true;
        }

    return m_mockedOpenOrCreate;
    }

void LicensingDbMock::Close()
    {
    m_closeCalls++;
    m_isOpen = false;
    }

bool LicensingDbMock::IsDbOpen()
    {
    return m_isOpen;
    }

BentleyStatus LicensingDbMock::WriteUsageToCSVFile(BeFileNameCR path)
    {
    m_writeUsageToCSVFileCalls++;
    return m_mockedWriteUsageToCSVFile;
    }

BentleyStatus LicensingDbMock::WriteFeatureToCSVFile(BeFileNameCR path)
    {
    m_writeFeatureToCSVFileCalls++;
    return m_mockedWriteFeatureToCSVFile;
    }

std::list<Json::Value> LicensingDbMock::GetPolicyFiles()
    {
    m_getPolicyFilesCalls++;
    return m_policyList;
    }

std::list<Json::Value> LicensingDbMock::GetPolicyFilesByUser(Utf8StringCR userId)
    {
    m_getPolicyFilesByUserCallsMap[userId]++;
    return m_userPolicyListMap[userId];
    }

std::list<Json::Value> LicensingDbMock::GetPolicyFilesByKey(Utf8StringCR accessKey)
    {
    m_getPolicyFilesByKeyCallsMap[accessKey]++;
    return m_keyPolicyListMap[accessKey];
    }

BentleyStatus LicensingDbMock::AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR accessKey, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken)
    {
    m_addOrUpdatePolicyFileCalls++;
    return m_mockedAddOrUpdatePolicyFile;
    }

BentleyStatus LicensingDbMock::DeletePolicyFile(Utf8StringCR policyId)
    {
    m_deletePolicyFileCalls++;
    return m_mockedDeletePolicyFile;
    }

BentleyStatus LicensingDbMock::DeleteAllOtherPolicyFilesByUser(Utf8StringCR policyId, Utf8StringCR userId)
    {
    m_deleteAllOtherPolicyFilesByUserCalls++;
    return m_mockedDeleteAllOtherPolicyFilesByUser;
    }

BentleyStatus LicensingDbMock::DeleteAllOtherPolicyFilesByKey(Utf8StringCR policyId, Utf8StringCR accessKey)
    {
    m_deleteAllOtherPolicyFilesByKeyCalls++;
    return m_mockedDeleteAllOtherPolicyFilesByKey;
    }

Json::Value LicensingDbMock::GetPolicyFile()
    {
    m_getPolicyFileCalls++;
    return m_policyList.size() > 0 ? m_policyList.front() : Json::Value::GetNull();
    }

Json::Value LicensingDbMock::GetPolicyFile(Utf8StringCR policyId)
    {
    m_getPolicyFileByIdCallsMap[policyId]++;
    return m_idPolicyFileMap[policyId];
    }


BentleyStatus LicensingDbMock::SetOfflineGracePeriodStart(Utf8StringCR startTime)
    {
    m_setOfflineGracePeriodStartCalls++;
    return m_mockedSetOfflineGracePeriodStart;
    }

Utf8String LicensingDbMock::GetOfflineGracePeriodStart()
    {
    m_getOfflineGracePeriodStartCalls++;
    return m_mockedGetOfflineGracePeriodStart;
    }

BentleyStatus LicensingDbMock::ResetOfflineGracePeriod()
    {
    m_resetOfflineGracePeriodCalls++;
    return m_mockedResetOfflineGracePeriod;
    }

Utf8String LicensingDbMock::GetLastUsageRecordedTime()
    {
    m_getLastUsageRecordedTimeCalls++;
    return m_mockedGetLastUsageRecordedTime;
    }

int64_t LicensingDbMock::GetUsageRecordCount()
    {
    m_getUsageRecordCountCalls++;
    return m_mockedGetUsageRecordCount;
    }

int64_t LicensingDbMock::GetFeatureRecordCount()
    {
    m_getFeatureRecordCountCalls++;
    return m_mockedGetFeatureRecordCount;
    }

BentleyStatus LicensingDbMock::CleanUpUsages()
    {
    m_cleanUpUsagesCalls++;
    return m_mockedCleanUpUsages;
    }

BentleyStatus LicensingDbMock::CleanUpFeatures()
    {
    m_cleanUpFeaturesCalls++;
    return m_mockedCleanUpFeatures;
    }

BentleyStatus LicensingDbMock::RecordUsage
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
    )
    {
    m_recordUsageCalls++;
    return m_mockedRecordUsage;
    }

BentleyStatus LicensingDbMock::RecordFeature
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
    )
    {
    m_recordFeatureCalls++;
    return m_mockedFeatureUsage;
    }

// mock/testing functions
void LicensingDbMock::MockPolicyFiles(std::list<Json::Value> policyFiles)
    {
    m_policyList = policyFiles;
    }

void LicensingDbMock::MockUserPolicyFiles(Utf8StringCR userId, std::list<Json::Value> policyFiles)
    {
    m_userPolicyListMap[userId] = policyFiles;
    m_getPolicyFilesByUserCallsMap[userId] = 0; // initialize count for this user id
    }

void LicensingDbMock::MockKeyPolicyFiles(Utf8StringCR accessKey, std::list<Json::Value> policyFiles)
    {
    m_keyPolicyListMap[accessKey] = policyFiles;
    m_getPolicyFilesByKeyCallsMap[accessKey] = 0; // initialize count for this key
    }

void LicensingDbMock::MockIdPolicyFile(Utf8StringCR policyId, Json::Value policyFile)
    {
    m_idPolicyFileMap[policyId] = policyFile;
    m_getPolicyFileByIdCallsMap[policyId] = 0; // initialize count for this id
    }

int LicensingDbMock::GetPolicyFilesByUserCount(Utf8StringCR userId)
    {
    if (m_getPolicyFilesByUserCallsMap.find(userId) != m_getPolicyFilesByUserCallsMap.end())
        {
        return m_getPolicyFilesByUserCallsMap[userId];
        }
    return 0;
    }

int LicensingDbMock::GetPolicyFilesByKeyCount(Utf8StringCR accessKey)
    {
    if (m_getPolicyFilesByKeyCallsMap.find(accessKey) != m_getPolicyFilesByKeyCallsMap.end())
        {
        return m_getPolicyFilesByKeyCallsMap[accessKey];
        }
    return 0;
    }

int LicensingDbMock::GetPolicyFileByIdCount(Utf8StringCR policyId)
    {
    if (m_getPolicyFileByIdCallsMap.find(policyId) != m_getPolicyFileByIdCallsMap.end())
        {
        return m_getPolicyFileByIdCallsMap[policyId];
        }
    return 0;
    }

