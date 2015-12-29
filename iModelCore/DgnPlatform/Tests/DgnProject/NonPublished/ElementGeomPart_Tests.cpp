/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementGeomPart_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Goem Parts
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementGeomPartTests : public DgnDbTestFixture
{
    static void ExpectEqualRange(DRange3dCR a, DRange3dCR b)
        {
        EXPECT_EQ(a.low.x, b.low.x);
        EXPECT_EQ(a.low.y, b.low.y);
        EXPECT_EQ(a.low.z, b.low.z);
        EXPECT_EQ(a.high.x, b.high.x);
        EXPECT_EQ(a.high.y, b.high.y);
        EXPECT_EQ(a.high.z, b.high.z);
        }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnGeomParts& geomPartTable = m_db->GeomParts();
    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*GeomHelper::computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));
    
    // Test the range
    ElementAlignedBox3d partBox = geomPartPtr->GetBoundingBox();
    EXPECT_TRUE(partBox.IsValid());
    ExpectEqualRange(partBox, builder->GetPlacement3d().GetElementBox());

    // Insert
    //
    ASSERT_EQ(SUCCESS, geomPartTable.InsertGeomPart(*geomPartPtr));
    DgnGeomPartId partId = geomPartPtr->GetId();
    ASSERT_TRUE(partId.IsValid());

    // Query
    EXPECT_TRUE(partId == geomPartTable.QueryGeomPartId("TestGeomPart"));
    DgnGeomPartPtr toFind = geomPartTable.LoadGeomPart(partId);
    GeomStream stream = toFind->GetGeomStream();
    EXPECT_TRUE(stream.HasGeometry());
    uint32_t size  = stream.GetSize();
    EXPECT_TRUE(geomPartPtr->GetGeomStream().GetSize() == size);
    ExpectEqualRange(geomPartPtr->GetBoundingBox(), partBox);
    
    // Query range
    DRange3d partRange;
    EXPECT_EQ(SUCCESS, geomPartTable.QueryGeomPartRange(partRange, partId));
    ExpectEqualRange(partRange, partBox);

    // Update
    builder->Append(*elGPtr);
    builder->SetGeomStream(*geomPartPtr);
    ASSERT_TRUE(partId == geomPartPtr->GetId());
    EXPECT_TRUE(geomPartPtr->GetGeomStream().GetSize() > size);
    ASSERT_TRUE(geomPartPtr->GetId().IsValid());
    ASSERT_EQ(SUCCESS  , geomPartTable.UpdateGeomPart(*geomPartPtr));
    
    EXPECT_GT(geomPartTable.LoadGeomPart(partId)->GetGeomStream().GetSize() , size);

    // Delete
    EXPECT_TRUE(SUCCESS == geomPartTable.DeleteGeomPart(partId) );
    EXPECT_FALSE( geomPartTable.QueryGeomPartId("TestGeomPart").IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CreateElements)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*GeomHelper::computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId1 = InsertElementUsingGeomPart(geomPartPtr->GetCode());
    EXPECT_TRUE(elementId1.IsValid());
    
    DgnElementId elementId2 = InsertElementUsingGeomPart(geomPartPtr->GetCode());
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId3.IsValid());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, GeomPartWithoutCode)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*GeomHelper::computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create();
    EXPECT_TRUE(geomPartPtr != NULL);
    ASSERT_STREQ("", geomPartPtr->GetCode());
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = geomPartPtr->GetId();
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId1 = InsertElementUsingGeomPart(existingPartId);
    EXPECT_TRUE(elementId1.IsValid());
    
    DgnElementId elementId2 = InsertElementUsingGeomPart(existingPartId);
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId3.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, ElementGeomUsesParts)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*GeomHelper::computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId("TestGeomPart");
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertElementGeomUsesParts(elementId, existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(elementId);
    
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementUsesGeomParts)));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1,stmt.GetValueInt(0));
    ASSERT_EQ(elementId.GetValue(), stmt.GetValueInt64(1));
    ASSERT_EQ(existingPartId.GetValue(), (int64_t)stmt.GetValueInt(2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, ElementGeomUsesParts_DeleteGeomPart)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*GeomHelper::computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId("TestGeomPart");
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertElementGeomUsesParts(elementId, existingPartId));
    DgnElementCPtr elem = m_db->Elements().GetElement(elementId);

    // Delete Geom Part
    EXPECT_EQ(SUCCESS, m_db->GeomParts().DeleteGeomPart(existingPartId));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementUsesGeomParts)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, ElementGeomUsesParts_DeleteElement)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*GeomHelper::computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId("TestGeomPart");
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertElementGeomUsesParts(elementId, existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(elementId);
    m_db->SaveChanges ();
    // Delete Element
    ASSERT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(*m_db->Elements().GetElement(elementId)));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementUsesGeomParts)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    EXPECT_TRUE(m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CreateElementsAndDeleteGemPart)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*GeomHelper::computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());

    //Add two elements using this GeomPart
    DgnElementId elementId1 = InsertElementUsingGeomPart(geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId1.IsValid());

    DgnElementId elementId2 = InsertElementUsingGeomPart(geomPartPtr->GetCode());
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId3.IsValid());

    // Delete Geom Part
    EXPECT_EQ(SUCCESS, m_db->GeomParts().DeleteGeomPart(existingPartId));
    EXPECT_FALSE(m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(elementId1).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(elementId2).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(elementId3).IsValid());
    //m_db->Get
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, GeomPart2d)
{
    SetupProject(L"2dMetricGeneral.idgndb", L"GeomParts2d.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*GeomHelper::computeShape2d());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, false);
    builder->Append(*elGPtr);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));
     
    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());

    //Add two elements using this GeomPart
    DgnElementId elementId1 = InsertElementUsingGeomPart2d( geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId1.IsValid());

    DgnElementId elementId2 = InsertElementUsingGeomPart2d( geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement2d( m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId3.IsValid());
    }
