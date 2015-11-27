/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/DgnDbTestFixtures.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DPTEST

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(DgnModelId mid, DgnCategoryId categoryId, DgnDbStatus* result, DgnElement::Code elementCode)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el, result);
}

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement with Display Properties
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(ElemDisplayParamsCR ep, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, ep, mid, categoryId, elementCode,100);
    return m_db->Elements().Insert(*el);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(Utf8CP elementCode, DgnModelId mid, DgnCategoryId categoryId)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el);
     }

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .dgndb project file after copying it to out
* baseProjFile is the existing file and testProjFile is what we get
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::SetupProject(WCharCP baseProjFile, WCharCP testProjFile, BeSQLite::Db::OpenMode mode)
{
    BeFileName outFileName;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseProjFile, testProjFile, __FILE__));
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, outFileName, DgnDb::OpenParams(mode));
    ASSERT_TRUE(m_db.IsValid());
    ASSERT_TRUE(result == BE_SQLITE_OK);

    TestDataManager::MustBeBriefcase(m_db, mode);
    ASSERT_TRUE(m_db->IsBriefcase());
    ASSERT_TRUE((Db::OpenMode::ReadWrite != mode) || m_db->Txns().IsTracking());

    auto status = DgnPlatformTestDomain::GetDomain().ImportSchema(*m_db);
    ASSERT_TRUE(DgnDbStatus::Success == status);

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    m_defaultModelP = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_TRUE(m_defaultModelP.IsValid());
    m_defaultModelP->FillModel();

    m_defaultCategoryId = DgnCategory::QueryFirstCategoryId(*m_db);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey DgnDbTestFixture::InsertElement2d(DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    DgnElementPtr el = TestElement2d::Create(*m_db, mid, categoryId, elementCode,100);

    return m_db->Elements().Insert(*el)->GetElementKey();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey DgnDbTestFixture::InsertElementUsingGeomPart2d(Utf8CP gpCode, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElement2dPtr el = TestElement2d::Create(*m_db, mid, categoryId, elementCode,100);

    DgnModelP model = m_db->Models().GetModel(mid).get();
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, categoryId, DPoint2d::From(0.0, 0.0));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(gpCode);
    EXPECT_TRUE(existingPartId.IsValid());

    if (!(builder->Append(existingPartId, Transform::From(0.0, 0.0, 0.0))))
        return DgnElementKey();

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
        return DgnElementKey();

    return m_db->Elements().Insert(*el)->GetElementKey();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey DgnDbTestFixture::InsertElementUsingGeomPart(Utf8CP gpCode, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    DgnElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);

    DgnModelP model = m_db->Models().GetModel(mid).get();
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, categoryId, DPoint3d::From(0.0, 0.0,0.0));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(gpCode);
    EXPECT_TRUE(existingPartId.IsValid());

    if (!(builder->Append(existingPartId, Transform::From(0.0, 0.0, 0.0))))
        return DgnElementKey();

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
        return DgnElementKey();

    return m_db->Elements().Insert(*el)->GetElementKey();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey DgnDbTestFixture::InsertElementUsingGeomPart(DgnGeomPartId gpId, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    DgnElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);

    DgnModelP model = m_db->Models().GetModel(mid).get();
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, categoryId, DPoint3d::From(0.0, 0.0,0.0));

    if (!(builder->Append(gpId, Transform::From(0.0, 0.0, 0.0))))
        return DgnElementKey();

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
        return DgnElementKey();

    return m_db->Elements().Insert(*el)->GetElementKey();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::setUpPhysicalView(DgnDbR dgnDb, DgnModelR model, ElementAlignedBox3d elementBox, DgnCategoryId categoryId)
    {
    CameraViewDefinition view(CameraViewDefinition::CreateParams(dgnDb, "TestView", ViewDefinition::Data(model.GetModelId(), DgnViewSource::Generated)));
    EXPECT_TRUE(view.Insert().IsValid());

    ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);

    PhysicalViewController viewController (dgnDb, view.GetViewId());
    viewController.SetStandardViewRotation(StandardView::Iso);
    viewController.LookAtVolume(elementBox, nullptr, &viewMargin);
    viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::SmoothShade);
    viewController.ChangeCategoryDisplay(categoryId, true);
    viewController.ChangeModelDisplay(model.GetModelId(), true);

    EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());
    }


