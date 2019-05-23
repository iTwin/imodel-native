/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLiteNonPublishedTests.h"
#include "BeSQLite/ChangeSet.h"
#include <vector>
#include <limits>
#include <string>

//---------------------------------------------------------------------------------------
// @bsimethod                                Taslim.Murad                   05/17
//---------------------------------------------------------------------------------------
struct SnappyToBlobTests : public ::testing::Test
{
};

//---------------------------------------------------------------------------------------
// @bsimethod                                Taslim.Murad                   05/17
//---------------------------------------------------------------------------------------
TEST_F (SnappyToBlobTests, WriteGetUncompSize)
{
    SnappyToBlob sblob;
    Byte values[] = {1,0,1,0,0,1};
    Byte* datap = &values[0];
    sblob.Init();
    sblob.Write(datap,6);
    uint32_t size = sblob.GetUnCompressedSize();
    ASSERT_EQ(size,6);
}