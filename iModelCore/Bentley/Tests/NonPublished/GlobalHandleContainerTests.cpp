/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/GlobalHandleContainer.h>
#include <vector>
//=======================================================================================
// @bsiclass                                        Farhad.Kabir            11/16
//=======================================================================================
class GlobalHandleContainerTest : public ::testing::Test
    {
    public:
        uint32_t offset;
        void* ptr;
        void* ptr2;
        void* ptr3;
        virtual void SetUp()
            {
            offset = 16777215;//  0xFFFFFF
            //  Don't let any other tests effect this.
            GlobalHandleContainer::Destroy();
            }
        virtual void TearDown()
            {
            //  Don't effect any other tests.
            GlobalHandleContainer::Destroy();
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Farhad.Kabir                    11/16
//---------------------------------------------------------------------------------------
TEST_F(GlobalHandleContainerTest, AllocateHandle)
    {
    uint32_t handle = offset;
    ASSERT_TRUE(0 == GlobalHandleContainer::IsHandleValid(handle));
    GlobalHandleContainer::ReleaseHandle(offset + 1);
    EXPECT_EQ(NULL, GlobalHandleContainer::GetPointer(offset + 3));

    uint32_t dum1 = 21;//dum* give dummy values, they mean nothing, just giving a reference to pointers
    ptr = &dum1;
    ASSERT_EQ((uint32_t)(offset + 1), GlobalHandleContainer::AllocateHandle(ptr));
    uint32_t dum2 = 3222;
    ptr2 = &dum2;
    ASSERT_EQ((uint32_t)(offset + 2), GlobalHandleContainer::AllocateHandle(ptr2));
    uint32_t dum3 = 222;
    ptr3 = &dum3;
    ASSERT_EQ((uint32_t)(offset + 3), GlobalHandleContainer::AllocateHandle(ptr3));
    uint32_t handlein = offset + 1;
    void* received = GlobalHandleContainer::GetPointer(handlein);
    EXPECT_EQ(ptr, received);
    handlein = offset + 2;
    received = GlobalHandleContainer::GetPointer(handlein);
    EXPECT_EQ(ptr2, received);
    handlein = offset + 3;
    received = GlobalHandleContainer::GetPointer(handlein);
    EXPECT_EQ(ptr3, received);
    ASSERT_TRUE(1 == GlobalHandleContainer::IsHandleValid((uint32_t)(offset + 1)));

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Farhad.Kabir                    11/16
//---------------------------------------------------------------------------------------
TEST_F(GlobalHandleContainerTest, ReallocateHandle)
    {
    uint32_t handleRelease = offset + 1;
    GlobalHandleContainer::ReleaseHandle(handleRelease);
    ASSERT_EQ(NULL, GlobalHandleContainer::GetPointer(handleRelease));

    //reallocating 
    uint32_t dum4 = 2122;
    void* ptr4 = &dum4;
    ASSERT_EQ((uint32_t)(1 * handleRelease), GlobalHandleContainer::AllocateHandle(ptr4));

    handleRelease = 1 * (offset + 1);
    GlobalHandleContainer::ReleaseHandle(handleRelease);
    ASSERT_EQ(NULL, GlobalHandleContainer::GetPointer(handleRelease));

    //reallocating 
    uint32_t dum5 = 2122;
    void* ptr5 = &dum5;
    ASSERT_EQ((uint32_t)(2 * (offset + 1)), GlobalHandleContainer::AllocateHandle(ptr5));
    }
