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

static const DgnCode s_geomPartCode = DgnGeometryPart::CreateCode("GeomPart", "Test");

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CRUD)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"GeomParts.ibim", BeSQLite::Db::OpenMode::ReadWrite);

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
    ASSERT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());
    DgnGeometryPartId partId = geomPartPtr->GetId();
    ASSERT_TRUE(partId.IsValid());

    // Query
    EXPECT_TRUE(partId == DgnGeometryPart::QueryGeometryPartId(s_geomPartCode, *m_db));
    DgnGeometryPartCPtr toFind = m_db->Elements().Get<DgnGeometryPart>(partId);
    GeometryStream stream = toFind->GetGeometryStream();
    EXPECT_TRUE(stream.HasGeometry());
    uint32_t size  = stream.GetSize();
    EXPECT_TRUE(geomPartPtr->GetGeometryStream().GetSize() == size);
    ExpectEqualRange(geomPartPtr->GetBoundingBox(), partBox);
    
    // Query range
    DRange3d partRange;
    EXPECT_EQ(SUCCESS, DgnGeometryPart::QueryGeometryPartRange(partRange, *m_db, partId));
    ExpectEqualRange(partRange, partBox);

    // Update
    builder->Append(*elGPtr);
    builder->SetGeometryStream(*geomPartPtr);
    ASSERT_TRUE(partId == geomPartPtr->GetId());
    EXPECT_TRUE(geomPartPtr->GetGeometryStream().GetSize() > size);
    ASSERT_TRUE(geomPartPtr->GetId().IsValid());
    ASSERT_TRUE(m_db->Elements().Update<DgnGeometryPart>(*geomPartPtr).IsValid());
    
    EXPECT_GT(m_db->Elements().Get<DgnGeometryPart>(partId)->GetGeometryStream().GetSize(), size);

    // Delete
    EXPECT_TRUE(DgnDbStatus::Success == m_db->Elements().Delete(partId));
    EXPECT_FALSE(DgnGeometryPart::QueryGeometryPartId(s_geomPartCode, *m_db).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CreateElements)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"GeomParts.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());

    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(geomPartPtr->GetCode(), *m_db);
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
    SetupProject(L"3dMetricGeneral.ibim", L"GeomParts.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());
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
TEST_F(ElementGeomPartTests, ElementsUseGeometryParts)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"GeomParts.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());

    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(s_geomPartCode, *m_db);
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    EXPECT_EQ(SUCCESS, DgnGeometryPart::InsertElementsUseGeometryParts(*m_db, elementId, existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(elementId);
    
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_db, "SELECT SourceECInstanceId,TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_ElementsUseGeometryParts)));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(elementId.GetValue(), statement.GetValueInt64(0));
    ASSERT_EQ(existingPartId.GetValue(), (int64_t)statement.GetValueInt(1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, ElementsUseGeometryParts_DeleteGeomPart)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"GeomParts.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());

    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(s_geomPartCode, *m_db);
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    EXPECT_EQ(SUCCESS, DgnGeometryPart::InsertElementsUseGeometryParts(*m_db, elementId, existingPartId));
    DgnElementCPtr elem = m_db->Elements().GetElement(elementId);

    // Delete Geom Part
    EXPECT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(existingPartId));
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_db, "SELECT * FROM " BIS_SCHEMA(BIS_REL_ElementsUseGeometryParts)));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, ElementsUseGeometryParts_DeleteElement)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"GeomParts.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());

    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(s_geomPartCode, *m_db);
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    EXPECT_EQ(SUCCESS, DgnGeometryPart::InsertElementsUseGeometryParts(*m_db, elementId, existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(elementId);
    m_db->SaveChanges ();
    // Delete Element
    ASSERT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(*m_db->Elements().GetElement(elementId)));
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_db, "SELECT * FROM " BIS_SCHEMA(BIS_REL_ElementsUseGeometryParts)));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());

    EXPECT_TRUE(DgnGeometryPart::QueryGeometryPartId(geomPartPtr->GetCode(), *m_db).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CreateElementsAndDeleteGemPart)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"GeomParts.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());

    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(geomPartPtr->GetCode(), *m_db);
    EXPECT_TRUE(existingPartId.IsValid());

    //Add two elements using this GeometryPart
    DgnElementId elementId1 = InsertElementUsingGeometryPart(geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId1.IsValid());

    DgnElementId elementId2 = InsertElementUsingGeometryPart(geomPartPtr->GetCode());
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId3.IsValid());

    // Delete Geom Part
    EXPECT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(existingPartId));
    EXPECT_FALSE(DgnGeometryPart::QueryGeometryPartId(geomPartPtr->GetCode(), *m_db).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(elementId1).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(elementId2).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(elementId3).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, GeomPart2d)
{
    SetupProject(L"2dMetricGeneral.ibim", L"GeomParts2d.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape2d());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, false);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(*m_db, s_geomPartCode);
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeometryStream(*geomPartPtr));

    EXPECT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());
     
    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(geomPartPtr->GetCode(), *m_db);
    EXPECT_TRUE(existingPartId.IsValid());

    //Add two elements using this GeometryPart
    DgnElementId elementId1 = InsertElementUsingGeometryPart2d( geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId1.IsValid());

    DgnElementId elementId2 = InsertElementUsingGeometryPart2d( geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement2d( m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId3.IsValid());
    }
