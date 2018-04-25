/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/CompatibilityECDb.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "CompatibilityTestFixture.h"

struct CompatibilityECDb : CompatibilityTestFixture
    {};
//===========================================================Files===========================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
ECDB_TESTFILE(EmptyFile)
    {
    ECDb db;
    db.CreateNewDb(GetResolvedFilePath());
    }



//===========================================================Test===========================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CompatibilityECDb, OpenAllVersionInReadWriteMode)
    {
    ECDB_FOR_EACH(testFile, EmptyFile)
        {
        ECDB_OPEN_READOWRITE(ecdb, testFile);
        if (HasNewProfile(testFile))
            {
            ASSERT_TRUE(ecdb.TableExists("ec_Class"));
            }
        else if (HasOldProfile(testFile))
            {
            ASSERT_TRUE(ecdb.TableExists("ec_Class"));
            }
        else if (HasCurrentProfile(testFile))
            {
            ASSERT_TRUE(ecdb.TableExists("ec_Class"));
            }
        }
    }

