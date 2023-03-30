/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
#include <ECDb/ConcurrentQueryManager.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

struct IntegrityCheckerFixture : ECDbTestFixture {

    DbResult OpenECDbTestDataFile(Utf8CP name) {
        auto getDataPath = []() {
            BeFileName docRoot;
            BeTest::GetHost().GetDocumentsRoot(docRoot);
            docRoot.AppendToPath(L"ECDb");
            return docRoot;
        };

        const auto bimPath = getDataPath().AppendToPath(WString(name, true).c_str());
        if (m_ecdb.IsDbOpen()) {
            m_ecdb.CloseDb();
        }
        return m_ecdb.OpenBeSQLiteDb(bimPath, Db::OpenParams(Db::OpenMode::Readonly));
    }

};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IntegrityCheckerFixture, integrity_check) {
    ECDb db;
    auto rc = db.OpenBeSQLiteDb("d:\\temp\\0d38e141-7705-4974-95da-1d4d3b762f31.bim", Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ(rc, BE_SQLITE_OK);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "PRAGMA integrity_check"));
    while (stmt.Step() == BE_SQLITE_ROW) {
        printf("%s %s\n", stmt.GetValueBoolean(1) ? "PASSED" : "FAILED", stmt.GetValueText(0));
    }
}



END_ECDBUNITTESTS_NAMESPACE
