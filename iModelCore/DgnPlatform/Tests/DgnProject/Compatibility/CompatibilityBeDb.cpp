/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/CompatibilityBeDb.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

struct CompatibilityBeDb : CompatibilityTestFixture
    {
    };
//===========================================================Files===========================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
BEDB_TESTFILE(EmptyFile)
    {
    Db db;
    db.CreateNewDb(GetResolvedFilePath());
    }

//===========================================================Test===========================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CompatibilityBeDb, OpenAllVersionInReadWriteMode)
    {
    BEDB_FOR_EACH(testFile, EmptyFile)
        {
        BEDB_OPEN_READOWRITE(db, testFile);
        if (HasNewProfile(testFile))
            {
            ASSERT_TRUE(db.TableExists("be_prop"));
            }
        else if (HasOldProfile(testFile))
            {
            ASSERT_TRUE(db.TableExists("be_prop"));
            }
        else if (HasCurrentProfile(testFile))
            {
            ASSERT_TRUE(db.TableExists("be_prop"));
            }
        }
    }

