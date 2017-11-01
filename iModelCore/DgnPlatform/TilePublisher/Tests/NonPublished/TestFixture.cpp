/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/TestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestFixture.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TestFixture::SetupDb(WCharCP name)
    {
    ASSERT_TRUE(m_db.IsNull());

    BeFileName filename;
    BeTest::GetHost().GetOutputRoot(filename);
    filename.AppendToPath(name);
    filename.append(L".bim");
    BeFileName::BeDeleteFile(filename);

    CreateDgnDbParams params;
    params.SetRootSubjectName("TilePublisherTests");
    params.SetOverwriteExisting(false);

    BeSQLite::DbResult status;
    m_db = DgnDb::CreateDgnDb(&status, filename, params);
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, status) << status;
    }

