/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/MemoryManager_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestConsumer : IMemoryConsumer
{
private:
    Priority m_priority;
    int64_t m_allocation;

    virtual int64_t _CalculateBytesConsumed() const override { return m_allocation; }
    virtual int64_t _Purge(int64_t target) override
        {
        m_allocation = 0;
        return 0;
        }
public:
    TestConsumer(Priority priority, int64_t allocation) : m_priority(priority), m_allocation(allocation) { }

    Priority GetPriority() const { return m_priority; }
    void SetAllocation(int64_t allocation) { m_allocation = allocation; }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct MemoryManagerTests : public DgnDbTestFixture
{
    typedef IMemoryConsumer::Priority Priority;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MemoryManagerTests, CalculateAndPurge)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"MemoryManager_Test.idgndb", BeSQLite::Db::OpenMode::ReadWrite); // because ImportSchemas() fails our test if Readonly...

    DgnDb& db = *m_db;
    MemoryManager& mgr = db.Memory();

    TestConsumer consumers[] =
        {
        TestConsumer(Priority::Moderate, 100),
        TestConsumer(Priority::VeryLow, 100),
        TestConsumer(Priority::High, 100),
        TestConsumer(Priority::Low, 100),
        TestConsumer(Priority::Critical, 100)
        };

    for (auto& consumer : consumers)
        mgr.AddConsumer(consumer, consumer.GetPriority());

    EXPECT_EQ(500, mgr.CalculateBytesConsumed());

    EXPECT_TRUE(mgr.Purge(350));
    EXPECT_EQ(300, mgr.CalculateBytesConsumed());

#define EXPECT_CONSUMED(INDEX, VALUE) EXPECT_EQ(consumers[INDEX].CalculateBytesConsumed(), VALUE)

    EXPECT_CONSUMED(1, 0);
    EXPECT_CONSUMED(3, 0);
    EXPECT_CONSUMED(0, 100);
    EXPECT_CONSUMED(2, 100);
    EXPECT_CONSUMED(4, 100);

    consumers[1].SetAllocation(100);
    EXPECT_TRUE(mgr.Purge(250));
    EXPECT_EQ(200, mgr.CalculateBytesConsumed());

    EXPECT_CONSUMED(1, 0);
    EXPECT_CONSUMED(3, 0);
    EXPECT_CONSUMED(0, 0);
    EXPECT_CONSUMED(2, 100);
    EXPECT_CONSUMED(4, 100);

    EXPECT_TRUE(mgr.Purge(500));
    EXPECT_EQ(200, mgr.CalculateBytesConsumed());

    EXPECT_TRUE(mgr.Purge(0));
    EXPECT_EQ(0, mgr.CalculateBytesConsumed());

    for (auto& consumer : consumers)
        {
        EXPECT_EQ(0, consumer.CalculateBytesConsumed());
        mgr.DropConsumer(consumer);
        }
    }

