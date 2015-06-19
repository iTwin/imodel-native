/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/ElementInsertPerformance.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECDbApi.h>
#include "PerformanceTestFixture.h"
#include "..\TestFixture\DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE

//=======================================================================================
//! Test Fixtrue for tests
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct PerformanceElementItem : public DgnDbTestFixture
{
public:
    PerformanceTestingFrameWork     m_testObj;

};

/*---------------------------------------------------------------------------------**//**
* Test to measure time of Insert, Select, Update and Delete of an Element Item
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementItem, CRUD)
{
    //Read from ecdb: the start, maximum and increment number to run the test
    int startCount = m_testObj.getStartNum();
    int maxCount = m_testObj.getEndNum();
    int increment = m_testObj.getIncrement();

    StopWatch elementTimer("Insert Element", false);

    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceTests.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    int counter;
    double elementTime, elementItemTime, selectTime, updateTime, deleteTime, deleteElementTime;
    for (counter = startCount; counter <= maxCount; counter = counter + increment)
    {
        elementTime = elementItemTime = selectTime = updateTime = deleteTime = deleteElementTime = 0.0;
        for (int i = 1; i <= counter; i++)
        {
            //First insert the Element
            elementTimer.Start();
            auto key1 = InsertElement(Utf8PrintfString("E%d", i));
            //m_db->SaveChanges();
            elementTimer.Stop();
            elementTime = elementTime + elementTimer.GetElapsedSeconds();

            EXPECT_TRUE(key1.GetElementId().IsValid());
            GeometricElementCPtr el = m_db->Elements().GetElement(key1.GetElementId())->ToGeometricElement();
            EXPECT_TRUE(el != nullptr);
            EXPECT_EQ(&el->GetElementHandler(), &TestElementHandler::GetHandler());

            // ECSQL to add ElementItem
            elementTimer.Start();
            EXPECT_TRUE(InsertElementItem(el->GetElementId(), L"Test"));
            //m_db->SaveChanges();
            elementTimer.Stop();
            elementItemTime = elementItemTime + elementTimer.GetElapsedSeconds();

            //Time to select a single ElementItem
            elementTimer.Start();
            EXPECT_TRUE(SelectElementItem(el->GetElementId()));
            //m_db->SaveChanges();
            elementTimer.Stop();
            selectTime = selectTime + elementTimer.GetElapsedSeconds();

            //Now Update data and measure time for Update
            elementTimer.Start();
            EXPECT_TRUE(UpdateElementItem(el->GetElementId(), L"Test - New"));
            //m_db->SaveChanges();
            elementTimer.Stop();
            updateTime = updateTime + elementTimer.GetElapsedSeconds();

            //Now delete data and measure time for Delete
            elementTimer.Start();
            EXPECT_TRUE(DeleteElementItem(el->GetElementId()));
            //m_db->SaveChanges();
            elementTimer.Stop();
            deleteTime = deleteTime + elementTimer.GetElapsedSeconds();

            //Now delete the Element
            elementTimer.Start();
            EXPECT_EQ(DgnDbStatus::Success, TestElementHandler::GetHandler().DeleteElement(*m_db, el->GetElementId()));
            //m_db->SaveChanges();
            elementTimer.Stop();
            deleteElementTime = deleteElementTime + elementTimer.GetElapsedSeconds();

        }
        //m_db->SaveChanges();

        //Write results to Db for analysis
        m_testObj.writeTodb(elementTime, "ElementCRUDPerformance,InsertElementItem_Element", "", counter);
        m_testObj.writeTodb(elementItemTime, "ElementCRUDPerformance,InsertElementItem_Item", "", counter);
        m_testObj.writeTodb(elementTime + elementItemTime, "ElementCRUDPerformance,InsertElementItem_Total", "", counter);
        m_testObj.writeTodb(selectTime, "ElementCRUDPerformance,SelectSignleElementItem", "", counter);
        m_testObj.writeTodb(updateTime, "ElementCRUDPerformance,UpdateElementItem", "", counter);
        m_testObj.writeTodb(deleteTime, "ElementCRUDPerformance,DeleteElementItem", "", counter);
        m_testObj.writeTodb(deleteElementTime, "ElementCRUDPerformance,DeleteElement", "", counter);
        m_testObj.writeTodb(deleteTime + deleteElementTime, "ElementCRUDPerformance,DeleteElementItem_Total", "", counter);

    }

}

