/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestIModelCreators.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestIModelCreators.h"
#include "Profiles.h"

USING_NAMESPACE_BENTLEY_EC
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool TestIModelCreation::s_hasRun = false;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestIModelCreation::Run()
    {
    if (s_hasRun)
        return SUCCESS;

    s_hasRun = true;
    return TestFileCreation::Run(std::vector<std::shared_ptr<TestFileCreator>>(TESTIMODELCREATOR_LIST));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbPtr TestIModelCreator::CreateNewTestFile(Utf8CP fileName)
    {
    BeFileName filePath = DgnDbProfile::Get().GetPathForNewTestFile(fileName);
    BeFileName folder = filePath.GetDirectoryName();
    if (!folder.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(folder))
            return nullptr;
        }
    else if (filePath.DoesPathExist())
        {
        if (BeFileNameStatus::Success != filePath.BeDeleteFile())
            return nullptr;
        }

    CreateDgnDbParams createParam(fileName);
    return DgnDb::CreateDgnDb(nullptr, filePath, createParam);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestIModelCreator::ImportSchemas(DgnDbR dgndb, std::vector<SchemaItem> const& schemas)
    {
    dgndb.SaveChanges();
    ECSchemaReadContextPtr ctx = DeserializeSchemas(dgndb, schemas);
    if (ctx == nullptr)
        return ERROR;

    if (SchemaStatus::Success == dgndb.ImportSchemas(ctx->GetCache().GetSchemas()))
        {
        dgndb.SaveChanges();
        return SUCCESS;
        }

    dgndb.AbandonChanges();
    return ERROR;
    }

