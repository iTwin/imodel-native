/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/ECDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

struct ECDbCompatibilityTestFixture : CompatibilityTestFixture 
    {
    protected:
        Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::ECDb); }
        DbResult OpenTestFile(ECDb& ecdb, BeFileNameCR path) { return ecdb.OpenBeSQLiteDb(path, ECDb::OpenParams(ECDb::OpenMode::Readonly)); }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
struct CreateTestECDbTestFixture : CompatibilityTestFixture
    {
    private:
        Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::ECDb); }

    protected:
        DbResult CreateNewTestFile(ECDbR ecdb, Utf8CP fileName)
            {
            BeFileName filePath = Profile().GetPathForNewTestFile(fileName);
            BeFileName folder = filePath.GetDirectoryName();
            if (!folder.DoesPathExist())
                {
                if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(folder))
                    return BE_SQLITE_ERROR;
                }

            return ecdb.CreateNewDb(filePath);
            }


        BentleyStatus ImportSchema(ECDbCR ecdb, SchemaItem const& schema) const { return ImportSchemas(ecdb, {schema}); }
        BentleyStatus ImportSchemas(ECDbCR ecdb, std::vector<SchemaItem> const& schemas) const
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

    };

