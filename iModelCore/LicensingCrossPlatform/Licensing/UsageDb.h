/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/UsageDb.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <BeSQLite/BeSQLite.h>

#define LICENSE_CLIENT_SCHEMA_NAME      "LICENSINGSCHEMA"
#define LICENSE_CLIENT_SCHEMA_VERSION   1.0

USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct UsageDb
{
private:
	Db m_db;

	const Utf8String GRACESTART = "GRACESTART";

	BentleyStatus OpenDb(BeFileNameCR filePath);

	BentleyStatus CreateDb(BeFileNameCR filePath);

	BentleyStatus SetUpOfflineGraceTable();
    BentleyStatus SetUpTables();

    BentleyStatus SetEimVersion();
    BentleyStatus UpdateDb();
    BentleyStatus UpdateDbTables();

    int64_t GetLastUsageRecordRowId();
    int64_t GetLastFeatureRowId();

public:
	LICENSING_EXPORT BentleyStatus OpenOrCreate(BeFileNameCR filePath);

    LICENSING_EXPORT void Close();

    LICENSING_EXPORT bool IsDbOpen();

    LICENSING_EXPORT BentleyStatus WriteUsageToCSVFile(BeFileNameCR path);
    LICENSING_EXPORT BentleyStatus WriteFeatureToCSVFile(BeFileNameCR path);

	LICENSING_EXPORT std::list<Json::Value> GetPolicyFiles();
	LICENSING_EXPORT std::list<Json::Value> GetPolicyFiles(Utf8String userId);
    LICENSING_EXPORT BentleyStatus AddOrUpdatePolicyFile(Utf8StringCR policyId, Utf8StringCR userId, Utf8StringCR expirationDate, Utf8StringCR lastUpdateTime, Json::Value policyToken);
	LICENSING_EXPORT BentleyStatus DeletePolicyFile(Utf8StringCR policyId);
	LICENSING_EXPORT BentleyStatus DeleteAllOtherUserPolicyFiles(Utf8StringCR policyId, Utf8StringCR userId);

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

    LICENSING_EXPORT BentleyStatus RecordUsage(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
                                               Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
                                               Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
                                               Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
                                               Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType);

    LICENSING_EXPORT BentleyStatus RecordFeature(int64_t ultimateSAPId, Utf8StringCR principalId, Utf8StringCR imsId, Utf8String machineName,
                                                 Utf8StringCR machineSID, Utf8StringCR userName, Utf8StringCR userSID, Utf8StringCR policyId,
                                                 Utf8StringCR securableId, int productId, Utf8String featureString, int64_t productVersion,
                                                 Utf8StringCR projectId, Utf8String correlationId, Utf8StringCR eventTime, double schemaVersion,
                                                 Utf8StringCR logPostingSource, Utf8StringCR country, Utf8StringCR usageType, Utf8StringCR featureId,
                                                 Utf8StringCR startDate, Utf8String endDate, Utf8String userData);

};

END_BENTLEY_LICENSING_NAMESPACE
