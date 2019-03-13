#include "LicensingDbMock.h"

#include <exception>

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

BentleyStatus LicensingDbMock::OpenOrCreate(BeFileNameCR filePath) {
    m_openOrCreateCalls++;

    if (m_mockedOpenOrCreate == SUCCESS) {
        m_isOpen = true;
    }

    return m_mockedOpenOrCreate;
}

void LicensingDbMock::Close() {
    m_closeCalls++;
    m_isOpen = false;
}

bool LicensingDbMock::IsDbOpen() {
    return m_isOpen;
}


BentleyStatus LicensingDbMock::WriteUsageToCSVFile(BeFileNameCR path) {
    throw std::exception("Not implemented");
}

BentleyStatus LicensingDbMock::WriteFeatureToCSVFile(BeFileNameCR path) {
    throw std::exception("Not implemented");
}


std::list<Json::Value> LicensingDbMock::GetPolicyFiles() {
    throw std::exception("Not implemented");
}

std::list<Json::Value> LicensingDbMock::GetPolicyFilesByUser(Utf8StringCR userId) {
    m_getPolicyFilesCountMap[userId]++;
    return m_userPolicyMap[userId];
}

std::list<Json::Value> LicensingDbMock::GetPolicyFilesByKey(Utf8StringCR accessKey) {
    throw std::exception("Not implemented");
}

BentleyStatus LicensingDbMock::AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR accessKey, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken) {
    throw std::exception("Not implemented");
}

BentleyStatus LicensingDbMock::DeletePolicyFile(Utf8StringCR policyId) {
    throw std::exception("Not implemented");
}

BentleyStatus LicensingDbMock::DeleteAllOtherPolicyFilesByUser(Utf8StringCR policyId, Utf8StringCR userId) {
    throw std::exception("Not implemented");
}

BentleyStatus LicensingDbMock::DeleteAllOtherPolicyFilesByKey(Utf8StringCR policyId, Utf8StringCR accessKey) {
    throw std::exception("Not implemented");
}


Json::Value LicensingDbMock::GetPolicyFile() {
    throw std::exception("Not implemented");
}

Json::Value LicensingDbMock::GetPolicyFile(Utf8StringCR policyId) {
    throw std::exception("Not implemented");
}


BentleyStatus LicensingDbMock::SetOfflineGracePeriodStart(Utf8StringCR startTime) {
    throw std::exception("Not implemented");
}

Utf8String LicensingDbMock::GetOfflineGracePeriodStart() {
    throw std::exception("Not implemented");
}

BentleyStatus LicensingDbMock::ResetOfflineGracePeriod() {
    throw std::exception("Not implemented");
}


Utf8String LicensingDbMock::GetLastUsageRecordedTime() {
    throw std::exception("Not implemented");
}

int64_t LicensingDbMock::GetUsageRecordCount() {
    throw std::exception("Not implemented");
}

int64_t LicensingDbMock::GetFeatureRecordCount() {
    throw std::exception("Not implemented");
}


BentleyStatus LicensingDbMock::CleanUpUsages() {
    throw std::exception("Not implemented");
}

BentleyStatus LicensingDbMock::CleanUpFeatures() {
    throw std::exception("Not implemented");
}


BentleyStatus LicensingDbMock::RecordUsage(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
    Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
    Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
    Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
    Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType) {
    
    m_recordUsageCalls++;
    // TODO add some tests for what happens if this fails
    
    return SUCCESS;
}


BentleyStatus LicensingDbMock::RecordFeature(
    int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
    Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
    Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
    Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
    Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType, Utf8StringCR featureId,
    Utf8StringCR startDate, Utf8String endDate, Utf8String userData) {
    throw std::exception("Not implemented");
}

// mock/testing functions

void LicensingDbMock::MockUserPolicyFiles(Utf8StringCR userId, std::list<Json::Value> policyFiles) {
    m_userPolicyMap[userId] = policyFiles;
}

int LicensingDbMock::GetPolicyFilesByUserCount(Utf8StringCR userId) {
    if (m_getPolicyFilesCountMap.find(userId) != m_getPolicyFilesCountMap.end()) {
        return m_getPolicyFilesCountMap[userId];
    }
    return 0;
}
