/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/UsageDb.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include <BeSQLite/BeSQLite.h>

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

    LICENSING_EXPORT BentleyStatus WriteUsageToSCVFile(BeFileNameCR path);
};

END_BENTLEY_LICENSING_NAMESPACE
