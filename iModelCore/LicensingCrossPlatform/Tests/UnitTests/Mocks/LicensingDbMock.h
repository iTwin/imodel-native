/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Mocks/LicensingDbMock.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include "../../../Licensing/ILicensingDb.h"

#include "../TestsHelper.h"

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE

struct LicensingDbMock : ILicensingDb
    {
public:
    MOCK_METHOD1(OpenOrCreate, BentleyStatus(BeFileNameCR filePath));
    MOCK_METHOD0(Close, void());
    MOCK_METHOD0(IsDbOpen, bool());

    MOCK_METHOD1(WriteUsageToCSVFile, BentleyStatus(BeFileNameCR path));
    MOCK_METHOD1(WriteFeatureToCSVFile, BentleyStatus(BeFileNameCR path));

    MOCK_METHOD0(GetPolicyFiles, std::list<Json::Value>());
    MOCK_METHOD1(GetPolicyFilesByUser, std::list<Json::Value>(Utf8StringCR userId));
    MOCK_METHOD1(GetPolicyFilesByKey, std::list<Json::Value>(Utf8StringCR accessKey));
    MOCK_METHOD6(AddOrUpdatePolicyFile, BentleyStatus(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR accessKey, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken));
    MOCK_METHOD1(DeletePolicyFile, BentleyStatus(Utf8StringCR policyId));
    MOCK_METHOD2(DeleteAllOtherPolicyFilesByUser, BentleyStatus(Utf8StringCR policyId, Utf8StringCR userId));
    MOCK_METHOD2(DeleteAllOtherPolicyFilesByKey, BentleyStatus(Utf8StringCR policyId, Utf8StringCR accessKey));

    MOCK_METHOD0(GetPolicyFile, Json::Value());
    MOCK_METHOD1(GetPolicyFile, Json::Value(Utf8StringCR policyId));

    MOCK_METHOD1(SetOfflineGracePeriodStart, BentleyStatus(Utf8StringCR startTime));
    MOCK_METHOD0(GetOfflineGracePeriodStart, Utf8String());
    MOCK_METHOD0(ResetOfflineGracePeriod, BentleyStatus());

    MOCK_METHOD0(GetLastUsageRecordedTime, Utf8String());
    MOCK_METHOD0(GetUsageRecordCount, int64_t());
    MOCK_METHOD0(GetFeatureRecordCount, int64_t());

    MOCK_METHOD0(CleanUpUsages, BentleyStatus());
    MOCK_METHOD0(CleanUpFeatures, BentleyStatus());

    virtual BentleyStatus RecordUsage(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
        Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
        Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
        Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
        Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType)
        {
        return RecordUsageMock();
        }
    MOCK_METHOD0(RecordUsageMock, BentleyStatus());

    virtual BentleyStatus RecordFeature(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
        Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
        Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
        Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
        Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType, Utf8StringCR featureId,
        Utf8StringCR startDate, Utf8String endDate, Utf8String userData)
        {
        return RecordFeatureMock();
        }
    MOCK_METHOD0(RecordFeatureMock, BentleyStatus());
    };

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE
