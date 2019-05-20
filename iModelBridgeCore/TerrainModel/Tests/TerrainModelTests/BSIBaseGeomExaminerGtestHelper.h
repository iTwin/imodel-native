
/*================================================================================**//**
* @bsiclass                                                     KevinNyman      03/10
+===============+===============+===============+===============+===============+======*/
class BSIBaseGeomExaminer : public ::testing::EmptyTestEventListener
{
protected:
    Int64 baseAllocationCount;

public:
    BSIBaseGeomExaminer()
        {}
    virtual void OnTestStart (const ::testing::TestInfo& test_info) override
        {
        baseAllocationCount = BSIBaseGeom::GetAllocationDifference ();
        }
    virtual void OnTestEnd (const ::testing::TestInfo& test_info) override
        {
        Int64 finalAllocationCount = BSIBaseGeom::GetAllocationDifference ();
        if (finalAllocationCount != baseAllocationCount)
            printf ("BSIBaseGeom Allocation Difference %I64d\n", finalAllocationCount - baseAllocationCount);
        }        
};

#include <StackExaminer.h>
#include <StackExaminerGtestHelper.h>