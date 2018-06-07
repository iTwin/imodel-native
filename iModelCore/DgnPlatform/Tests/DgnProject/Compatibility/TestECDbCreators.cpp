/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestECDbCreators.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestECDbCreators.h"
#include "ProfileManager.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool TestECDbCreation::s_hasRun = false;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestECDbCreation::Run()
    {
    if (s_hasRun)
        return SUCCESS;

    s_hasRun = true;
    return TestFileCreation::Run(std::vector<std::shared_ptr<TestFileCreator>>(TESTECDBCREATOR_LIST));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
DbResult TestECDbCreator::CreateNewTestFile(ECDbR ecdb, Utf8CP fileName)
    {
    BeFileName filePath = ProfileManager::Get().GetProfile(ProfileType::ECDb).GetPathForNewTestFile(fileName);
    BeFileName folder = filePath.GetDirectoryName();
    if (!folder.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(folder))
            return BE_SQLITE_ERROR;
        }
    else if (filePath.DoesPathExist())
        {
        if (BeFileNameStatus::Success != filePath.BeDeleteFile())
            return BE_SQLITE_ERROR;
        }

    return ecdb.CreateNewDb(filePath);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestECDbCreator::ImportSchemas(ECDbCR ecdb, std::vector<SchemaItem> const& schemas)
    {
    ECN::ECSchemaReadContextPtr ctx = DeserializeSchemas(ecdb, schemas);
    if (ctx == nullptr)
        return ERROR;

    Savepoint sp(const_cast<ECDb&>(ecdb), "Schema Import");
    if (SUCCESS == ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()))
        {
        sp.Commit();
        return SUCCESS;
        }

    sp.Cancel();
    return ERROR;
    }
