/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/CompatibilityDgnDb.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "CompatibilityTestFixture.h"

struct CompatibilityDgnDb : CompatibilityTestFixture
    {};
//===========================================================Files===========================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
DGNDB_TESTFILE(EmptyFile)
    {}

//===========================================================Test===========================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CompatibilityDgnDb, OpenAllVersionInReadWriteMode)
    {
    //DGNDB_FOR_EACH(testFile, EmptyFile)
    //    {
    //    BEDB_OPEN_READOWRITE(db, testFile);
    //    if (hasNewProfile(testFile))
    //        {
    //        ASSERT_TRUE(db.TableExists("dgn_txns"));
    //        }
    //    else if (hasOldProfile(testFile))
    //        {
    //        ASSERT_TRUE(db.TableExists("dgn_txns"));
    //        }
    //    else if (hasCurrentProfile(testFile))
    //        {
    //        ASSERT_TRUE(db.TableExists("dgn_txns"));
    //        }
    //    }
    }

