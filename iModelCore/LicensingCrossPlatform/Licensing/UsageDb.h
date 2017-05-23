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

	BentleyStatus OpenDb(BeFileNameCR filePath);

	BentleyStatus CreateDb(BeFileNameCR filePath);
    BentleyStatus SetUpTables();

public:
	LICENSING_EXPORT UsageDb();

	LICENSING_EXPORT BentleyStatus OpenOrCreate(BeFileNameCR filePath);

    LICENSING_EXPORT void Close();




    LICENSING_EXPORT BentleyStatus TestInsert();
    LICENSING_EXPORT int GetTestRecordCount();
};

END_BENTLEY_LICENSING_NAMESPACE
