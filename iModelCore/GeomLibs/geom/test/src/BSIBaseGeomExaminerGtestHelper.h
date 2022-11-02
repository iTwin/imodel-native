/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/*================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class BSIBaseGeomExaminer : public ::testing::EmptyTestEventListener
{
protected:
    int64_t baseAllocationCount;

public:
    BSIBaseGeomExaminer()
        {}
    virtual void OnTestStart (const ::testing::TestInfo& test_info) override
        {
        baseAllocationCount = BSIBaseGeom::GetAllocationDifference ();
        }
    virtual void OnTestEnd (const ::testing::TestInfo& test_info) override
        {
        int64_t finalAllocationCount = BSIBaseGeom::GetAllocationDifference ();
        if (finalAllocationCount != baseAllocationCount)
            printf ("BSIBaseGeom Allocation Difference %ld\n", finalAllocationCount - baseAllocationCount);
        }        
};

#include "StackExaminer.h"
#include "StackExaminerGtestHelper.h"