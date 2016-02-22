/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementGeomPart_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGN
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

static const DgnCode s_geomPartCode = DgnGeometryPart::CreateCode("Test", "GeomPart");

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnGeometryParts& geomPartTable = m_db->GeometryParts();
    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));
    
    // Test the range
    ElementAlignedBox3d partBox = geomPartPtr->GetBoundingBox();
    EXPECT_TRUE(partBox.IsValid());
    ExpectEqualRange(partBox, builder->GetPlacement3d().GetElementBox());

    // Insert
    //
    ASSERT_EQ(SUCCESS, geomPartTable.InsertGeometryPart(*geomPartPtr));
    DgnGeometryPartId partId = geomPartPtr->GetId();
    ASSERT_TRUE(partId.IsValid());

    // Query
    EXPECT_TRUE(partId == geomPartTable.QueryGeometryPartId(s_geomPartCode));
    DgnGeometryPartPtr toFind = geomPartTable.LoadGeometryPart(partId);
    GeometryStream stream = toFind->GetGeometryStream();
    EXPECT_TRUE(stream.HasGeometry());
    uint32_t size  = stream.GetSize();
    EXPECT_TRUE(geomPartPtr->GetGeometryStream().GetSize() == size);
    ExpectEqualRange(geomPartPtr->GetBoundingBox(), partBox);
    
    // Query range
    DRange3d partRange;
    EXPECT_EQ(SUCCESS, geomPartTable.QueryGeometryPartRange(partRange, partId));
    ExpectEqualRange(partRange, partBox);

    // Update
    builder->Append(*elGPtr);
    builder->SetGeometryStream(*geomPartPtr);
    ASSERT_TRUE(partId == geomPartPtr->GetId());
    EXPECT_TRUE(geomPartPtr->GetGeometryStream().GetSize() > size);
    ASSERT_TRUE(geomPartPtr->GetId().IsValid());
    ASSERT_EQ(SUCCESS  , geomPartTable.UpdateGeometryPart(*geomPartPtr));
    
    EXPECT_GT(geomPartTable.LoadGeometryPart(partId)->GetGeometryStream().GetSize() , size);

    // Delete
    EXPECT_TRUE(SUCCESS == geomPartTable.DeleteGeometryPart(partId) );
    EXPECT_FALSE( geomPartTable.QueryGeometryPartId(s_geomPartCode).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CreateElements)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeometryParts().InsertGeometryPart(*geomPartPtr));

    DgnGeometryPartId existingPartId = m_db->GeometryParts().QueryGeometryPartId(geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId1 = InsertElementUsingGeometryPart(geomPartPtr->GetCode());
    EXPECT_TRUE(elementId1.IsValid());
    
    DgnElementId elementId2 = InsertElementUsingGeometryPart(geomPartPtr->GetCode());
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

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeometryParts().InsertGeometryPart(*geomPartPtr));
    EXPECT_TRUE(geomPartPtr->GetCode().IsValid());
    EXPECT_TRUE(geomPartPtr->GetCode().IsEmpty());

    DgnGeometryPartId existingPartId = geomPartPtr->GetId();
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId1 = InsertElementUsingGeometryPart(existingPartId);
    EXPECT_TRUE(elementId1.IsValid());
    
    DgnElementId elementId2 = InsertElementUsingGeometryPart(existingPartId);
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

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeometryParts().InsertGeometryPart(*geomPartPtr));

    DgnGeometryPartId existingPartId = m_db->GeometryParts().QueryGeometryPartId(s_geomPartCode);
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeometryParts().InsertElementGeomUsesParts(elementId, existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(elementId);
    
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementUsesGeometryParts)));
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

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeometryParts().InsertGeometryPart(*geomPartPtr));

    DgnGeometryPartId existingPartId = m_db->GeometryParts().QueryGeometryPartId(s_geomPartCode);
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeometryParts().InsertElementGeomUsesParts(elementId, existingPartId));
    DgnElementCPtr elem = m_db->Elements().GetElement(elementId);

    // Delete Geom Part
    EXPECT_EQ(SUCCESS, m_db->GeometryParts().DeleteGeometryPart(existingPartId));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementUsesGeometryParts)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, ElementGeomUsesParts_DeleteElement)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeometryParts().InsertGeometryPart(*geomPartPtr));

    DgnGeometryPartId existingPartId = m_db->GeometryParts().QueryGeometryPartId(s_geomPartCode);
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeometryParts().InsertElementGeomUsesParts(elementId, existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(elementId);
    m_db->SaveChanges ();
    // Delete Element
    ASSERT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(*m_db->Elements().GetElement(elementId)));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementUsesGeometryParts)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    EXPECT_TRUE(m_db->GeometryParts().QueryGeometryPartId(geomPartPtr->GetCode()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CreateElementsAndDeleteGemPart)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeometryParts().InsertGeometryPart(*geomPartPtr));

    DgnGeometryPartId existingPartId = m_db->GeometryParts().QueryGeometryPartId(geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());

    //Add two elements using this GeometryPart
    DgnElementId elementId1 = InsertElementUsingGeometryPart(geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId1.IsValid());

    DgnElementId elementId2 = InsertElementUsingGeometryPart(geomPartPtr->GetCode());
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId3.IsValid());

    // Delete Geom Part
    EXPECT_EQ(SUCCESS, m_db->GeometryParts().DeleteGeometryPart(existingPartId));
    EXPECT_FALSE(m_db->GeometryParts().QueryGeometryPartId(geomPartPtr->GetCode()).IsValid());
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

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape2d());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, false);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeometryParts().InsertGeometryPart(*geomPartPtr));
     
    DgnGeometryPartId existingPartId = m_db->GeometryParts().QueryGeometryPartId(geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());

    //Add two elements using this GeometryPart
    DgnElementId elementId1 = InsertElementUsingGeometryPart2d( geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId1.IsValid());

    DgnElementId elementId2 = InsertElementUsingGeometryPart2d( geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement2d( m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId3.IsValid());
    }
