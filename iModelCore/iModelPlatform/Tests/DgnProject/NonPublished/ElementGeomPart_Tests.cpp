/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Goem Parts
* @bsimethod
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

    DefinitionModelR GetGeomPartModel() {return GetDgnDb().GetDictionaryModel();}
    static constexpr Utf8CP GetGeomPartName() {return "TestGeomPart";}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CRUD)
    {
    SetupSeedProject();

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(GetGeomPartModel(), GetGeomPartName());
    EXPECT_TRUE(geomPartPtr != nullptr);
    EXPECT_EQ(SUCCESS, builder->Finish(*geomPartPtr));

    // Test the range
    ElementAlignedBox3d partBox = geomPartPtr->GetBoundingBox();
    EXPECT_TRUE(partBox.IsValid());
    ExpectEqualRange(partBox, builder->GetPlacement3d().GetElementBox());

    // Insert
    ASSERT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());
    DgnGeometryPartId partId = geomPartPtr->GetId();
    ASSERT_TRUE(partId.IsValid());

    // Query
    EXPECT_TRUE(partId == DgnGeometryPart::QueryGeometryPartId(GetGeomPartModel(), GetGeomPartName()));
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
    builder->Finish(*geomPartPtr);
    ASSERT_TRUE(partId == geomPartPtr->GetId());
    EXPECT_TRUE(geomPartPtr->GetGeometryStream().GetSize() > size);
    ASSERT_TRUE(geomPartPtr->GetId().IsValid());
    ASSERT_EQ(DgnDbStatus::Success, m_db->Elements().Update(*geomPartPtr));

    EXPECT_GT(m_db->Elements().Get<DgnGeometryPart>(partId)->GetGeometryStream().GetSize(), size);

    // Delete
    EXPECT_NE(DgnDbStatus::Success, m_db->Elements().Delete(partId)); // must delete via "purge"
    DgnDb::PurgeOperation purgeOperation(*m_db); // Give test permission to delete GeometryPart (normally reserved for "purge" operations)
    EXPECT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(partId));
    EXPECT_FALSE(DgnGeometryPart::QueryGeometryPartId(GetGeomPartModel(), GetGeomPartName()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CreateElements)
    {
    SetupSeedProject();

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(GetGeomPartModel(), GetGeomPartName());
    EXPECT_TRUE(geomPartPtr != nullptr);
    EXPECT_EQ(SUCCESS, builder->Finish(*geomPartPtr));

    EXPECT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());

    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(GetGeomPartModel(), GetGeomPartName());
    EXPECT_TRUE(existingPartId.IsValid());

    DgnElementId elementId1 = InsertElementUsingGeometryPart(existingPartId);
    EXPECT_TRUE(elementId1.IsValid());

    DgnElementId elementId2 = InsertElementUsingGeometryPart(existingPartId);
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId3.IsValid());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, GeomPartWithoutCode)
    {
    SetupSeedProject();

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(GetGeomPartModel());
    EXPECT_TRUE(geomPartPtr != nullptr);
    EXPECT_EQ(SUCCESS, builder->Finish(*geomPartPtr));

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, CreateElementsAndDeleteGeomPart)
    {
    SetupSeedProject();

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(GetGeomPartModel(), GetGeomPartName());
    EXPECT_TRUE(geomPartPtr != nullptr);
    EXPECT_EQ(SUCCESS, builder->Finish(*geomPartPtr));

    EXPECT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());

    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(GetGeomPartModel(), GetGeomPartName());
    EXPECT_TRUE(existingPartId.IsValid());

    //Add two elements using this GeometryPart
    DgnElementId elementId1 = InsertElementUsingGeometryPart(existingPartId, m_defaultModelId, m_defaultCategoryId);
    EXPECT_TRUE(elementId1.IsValid());

    DgnElementId elementId2 = InsertElementUsingGeometryPart(existingPartId);
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement()->GetElementId();
    EXPECT_TRUE(elementId3.IsValid());

    // Delete Geom Part
    EXPECT_NE(DgnDbStatus::Success, m_db->Elements().Delete(existingPartId)); // must delete via "purge" operation
    DgnDb::PurgeOperation purgeOperation(*m_db); // Give test permission to delete GeometryPart (normally reserved for "purge" operations)
    EXPECT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(existingPartId));
    EXPECT_FALSE(DgnGeometryPart::QueryGeometryPartId(GetGeomPartModel(), GetGeomPartName()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(elementId1).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(elementId2).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(elementId3).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, GeomPart2d)
    {
    SetupSeedProject();

    //Create a GeometryPart
    GeometricPrimitivePtr elGPtr = GeometricPrimitive::Create(*GeomHelper::computeShape2d());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, false);
    builder->Append(*elGPtr);
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(GetGeomPartModel(), GetGeomPartName());
    EXPECT_TRUE(geomPartPtr != nullptr);
    EXPECT_EQ(SUCCESS, builder->Finish(*geomPartPtr));

    EXPECT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());

    DgnGeometryPartId existingPartId = DgnGeometryPart::QueryGeometryPartId(GetGeomPartModel(), GetGeomPartName());
    EXPECT_TRUE(existingPartId.IsValid());

    DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(*m_db, "MyDrawingCategory");
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "TestDrawing");
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    DgnModelId modelId = drawingModel->GetModelId();

    //Add two elements using this GeometryPart
    DgnElementId elementId1 = InsertElementUsingGeometryPart2d(geomPartPtr->GetCode(), modelId, categoryId);
    EXPECT_TRUE(elementId1.IsValid());

    DgnElementId elementId2 = InsertElementUsingGeometryPart2d(geomPartPtr->GetCode(), modelId, categoryId);
    EXPECT_TRUE(elementId2.IsValid());

    DgnElementId elementId3 = InsertElement2d(modelId, categoryId);
    EXPECT_TRUE(elementId3.IsValid());
    }
