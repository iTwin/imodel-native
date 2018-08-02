/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/UsageDb.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <BeSQLite/BeSQLite.h>
#include "PolicyToken.h"

USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct UsageDb
{
private:
	Db m_db;

private:
	BentleyStatus OpenDb(BeFileNameCR filePath);

	BentleyStatus CreateDb(BeFileNameCR filePath);

    BentleyStatus SetUpTables();

    int64_t GetLastRowId();

public:
	LICENSING_EXPORT BentleyStatus OpenOrCreate(BeFileNameCR filePath);

    LICENSING_EXPORT void Close();

    LICENSING_EXPORT bool IsDbOpen();

    LICENSING_EXPORT BentleyStatus InsertNewRecord(int64_t startTime, int64_t endTime);
    
    LICENSING_EXPORT int64_t GetLastRecordEndTime();

    LICENSING_EXPORT BentleyStatus UpdateLastRecordEndTime(int64_t unixMilis);

    LICENSING_EXPORT int64_t GetRecordCount();

    LICENSING_EXPORT BentleyStatus WriteUsageToCSVFile(BeFileNameCR path);

	LICENSING_EXPORT std::list<Json::Value> GetPolicyFiles();
    LICENSING_EXPORT BentleyStatus AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken);

    LICENSING_EXPORT Json::Value GetPolicyFile();

    LICENSING_EXPORT BentleyStatus CleanUpUsages();

    LICENSING_EXPORT BentleyStatus RecordUsage(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
                                               Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
                                               Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
                                               Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
                                               Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType);
};

END_BENTLEY_LICENSING_NAMESPACE
