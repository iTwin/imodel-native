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

};

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

    auto key1 = InsertElementUsingGeomPart(geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key1.GetElementId().IsValid());
    
    auto key2 = InsertElementUsingGeomPart(geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key2.GetElementId().IsValid());

    auto key3 = InsertElement(DgnElement::Code(), m_defaultModelId, m_defaultCategoryId)->GetElementKey();
    EXPECT_TRUE(key3.GetElementId().IsValid());
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

    auto key1 = InsertElementUsingGeomPart(existingPartId, m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key1.GetElementId().IsValid());
    
    auto key2 = InsertElementUsingGeomPart(existingPartId, m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key2.GetElementId().IsValid());

    auto key3 = InsertElement(DgnElement::Code(), m_defaultModelId, m_defaultCategoryId)->GetElementKey();
    EXPECT_TRUE(key3.GetElementId().IsValid());
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

    auto key1 = InsertElement(DgnElement::Code(), m_defaultModelId, m_defaultCategoryId)->GetElementKey();
    EXPECT_TRUE(key1.GetElementId().IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertElementGeomUsesParts(key1.GetElementId(), existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(key1.GetElementId());
    
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK ,stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGeomUsesParts)));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1,stmt.GetValueInt(0));
    ASSERT_EQ(key1.GetElementId().GetValue(), (int64_t)stmt.GetValueInt(1));
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

    auto key1 = InsertElement(DgnElement::Code(), m_defaultModelId, m_defaultCategoryId)->GetElementKey();
    EXPECT_TRUE(key1.GetElementId().IsValid());

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertElementGeomUsesParts(key1.GetElementId(), existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(key1.GetElementId());

    // Delete Geom Part
    EXPECT_EQ(SUCCESS, m_db->GeomParts().DeleteGeomPart(existingPartId));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGeomUsesParts)));
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

    auto key1 = InsertElement(DgnElement::Code(), m_defaultModelId, m_defaultCategoryId)->GetElementKey();
    EXPECT_TRUE(key1.GetElementId().IsValid());


    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertElementGeomUsesParts(key1.GetElementId(), existingPartId) );
    DgnElementCPtr elem = m_db->Elements().GetElement(key1.GetElementId());
    m_db->SaveChanges ();
    // Delete Element
    ASSERT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(*m_db->Elements().GetElement(key1.GetElementId())));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT * FROM " DGN_TABLE(DGN_RELNAME_ElementGeomUsesParts)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
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
    auto key1 = InsertElementUsingGeomPart(geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key1.GetElementId().IsValid());

    auto key2 = InsertElementUsingGeomPart(geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key2.GetElementId().IsValid());

    auto key3 = InsertElement(DgnElement::Code(), m_defaultModelId, m_defaultCategoryId)->GetElementKey();
    EXPECT_TRUE(key3.GetElementId().IsValid());

    // Delete Geom Part
    EXPECT_EQ(SUCCESS, m_db->GeomParts().DeleteGeomPart(existingPartId));
    EXPECT_FALSE(m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(key1.GetElementId()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(key2.GetElementId()).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(key3.GetElementId()).IsValid());
    //m_db->Get
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, GeomPartWitoutGeometry)
{
    SetupProject(L"3dMetricGeneral.idgndb", L"GeomParts.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*GeomHelper::computeShape());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, true);

    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());

    //Add two elements using this GeomPart
    auto key1 = InsertElementUsingGeomPart(geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key1.GetElementId().IsValid());

    auto key2 = InsertElementUsingGeomPart(geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key2.GetElementId().IsValid());

    auto key3 = InsertElement(DgnElement::Code(), m_defaultModelId, m_defaultCategoryId)->GetElementKey();
    EXPECT_TRUE(key3.GetElementId().IsValid());
    
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomPartTests, GeomPart2d)
{
    SetupProject(L"2dMetricGeneral.idgndb", L"GeomParts2d.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    //Create a GeomPart
    ElementGeometryPtr elGPtr = ElementGeometry::Create(*GeomHelper::computeShape2d());
    TextString textStringElem;
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateGeomPart(*m_db, false);
    builder->Append(textStringElem);
    DgnGeomPartPtr geomPartPtr = DgnGeomPart::Create("TestGeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    EXPECT_EQ(SUCCESS, builder->SetGeomStream(*geomPartPtr));

    EXPECT_EQ(SUCCESS, m_db->GeomParts().InsertGeomPart(*geomPartPtr));
     
    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());

    //Add two elements using this GeomPart
    auto key1 = InsertElementUsingGeomPart2d( geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key1.GetElementId().IsValid());

    auto key2 = InsertElementUsingGeomPart2d( geomPartPtr->GetCode(), m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key2.GetElementId().IsValid());

    auto key3 = InsertElement2d( m_defaultModelId, m_defaultCategoryId, DgnElement::Code());
    EXPECT_TRUE(key3.GetElementId().IsValid());
    
}
