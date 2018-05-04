/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/CompatibilityDgnDb.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

struct CompatibilityDgnDb : CompatibilityTestFixture
    {
    ScopedDgnHost m_host;
    };

//===========================================================Files===========================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
DGNDB_TESTFILE(EmptyFile)
    {
    CreateDgnDbParams createParam(GetFileName());
    DbResult status;
    DgnDb::CreateDgnDb(&status, GetResolvedFilePath(), createParam);
    }

//===========================================================Test===========================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CompatibilityDgnDb, OpenAllVersionInReadWriteMode)
    {
    DGNDB_FOR_EACH(testFile, EmptyFile)
        {
        DGNDB_OPEN_READOWRITE(db, testFile);
        if (HasNewProfile(testFile))
            {
            ASSERT_TRUE(db->TableExists("dgn_txns"));
            }
        else if (HasOldProfile(testFile))
            {
            ASSERT_TRUE(db->TableExists("dgn_txns"));
            }
        else if (HasCurrentProfile(testFile))
            {
            ASSERT_TRUE(db->TableExists("dgn_txns"));
            }
        }
    }

