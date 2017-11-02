/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/TestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestFixture.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TestFixture::SetupDb(WCharCP name)
    {
    ASSERT_TRUE(m_db.IsNull());

    BeFileName filename;
    BeTest::GetHost().GetOutputRoot(filename);
    filename.AppendToPath(name);
    filename.append(L".bim");
    BeFileName::BeDeleteFile(filename);

    CreateDgnDbParams params;
    params.SetRootSubjectName("TilePublisherTests");
    params.SetOverwriteExisting(false);

    BeSQLite::DbResult status;
    m_db = DgnDb::CreateDgnDb(&status, filename, params);
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, status) << status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSelectorCPtr TestFixture::InsertModelSelector(DgnModelIdSet const& modelIds, Utf8CP name)
    {
    DgnDbR db = GetDb();
    ModelSelector sel(db.GetDictionaryModel(), name);
    sel.GetModelsR() = modelIds;
    
    auto ret = db.Elements().Insert(sel);
    EXPECT_TRUE(ret.IsValid());

    return ret;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
CategorySelectorCPtr TestFixture::InsertCategorySelector(DgnCategoryIdSet const& catIds, Utf8CP name)
    {
    DgnDbR db = GetDb();
    CategorySelector sel(db.GetDictionaryModel(), name);
    sel.GetCategoriesR() = catIds;

    auto ret = db.Elements().Insert(sel);
    EXPECT_TRUE(ret.IsValid());

    return ret;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyle3dCPtr TestFixture::InsertDisplayStyle3d(Utf8CP name, ColorDef bgColor, bool groundPlane, ViewFlags vf)
    {
    DisplayStyle3d style(GetDb().GetDictionaryModel(), name);
    style.SetBackgroundColor(bgColor);
    style.SetGroundPlaneEnabled(groundPlane);
    style.SetViewFlags(vf);

    auto ret = GetDb().Elements().Insert(style);
    EXPECT_TRUE(ret.IsValid());
    return ret;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialViewDefinitionCPtr TestFixture::InsertSpatialView(Utf8CP name, ModelSelectorCR models, CategorySelectorCR categories, DisplayStyle3dCR style, DRange3dCP viewedVolume, SpatialViewDefinition::Camera const* camera)
    {
    SpatialViewDefinition view(GetDb().GetDictionaryModel(), name, *DgnElement::MakeCopy(categories), *DgnElement::MakeCopy(style), *DgnElement::MakeCopy(models), camera);
    auto ret = GetDb().Elements().Insert(view);
    EXPECT_TRUE(ret.IsValid());
    if (nullptr != viewedVolume)
        {
        ViewControllerPtr vc = view.LoadViewController();
        vc->GetViewDefinitionR().LookAtVolume(*viewedVolume);
        ret = static_cast<SpatialViewDefinitionCP>(vc->GetViewDefinitionR().Update().get());
        EXPECT_TRUE(ret.IsValid());
        }

    return ret;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr TestFixture::CreateRectangle(DPoint3dCR ll, double w, double h)
    {
    double top = ll.y + h,
           right = ll.x + w;

    DPoint3d pts[5] =
        {
        ll,
        DPoint3d::From(ll.x, top, ll.z),
        DPoint3d::From(right, top, ll.z),
        DPoint3d::From(right, ll.y, ll.z),
        ll
        };

    return CreateShape(pts, 5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr TestFixture::CreateTriangle(DPoint3dCR ll, double w, double h)
    {
    auto curve = CreateRectangle(ll, w, h);
    auto& shape = *(curve->at(0)->GetLineStringP());
    shape.erase(shape.begin() + 2);
    return curve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalElementCPtr TestFixture::InsertPhysicalElement(PhysicalModelR model, GeometryBuilderR builder, DPoint3dCR origin)
    {
    auto elem = GenericPhysicalObject::Create(model, builder.GetGeometryParams().GetCategoryId());
    EXPECT_TRUE(elem.IsValid());
    Placement3d placement = elem->GetPlacement();
    placement.GetOriginR() = origin;
    elem->SetPlacement(placement);

    auto ret = GetDb().Elements().Insert(*elem);
    EXPECT_TRUE(ret.IsValid());
    return ret.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr TestFixture::CreateGeometryBuilder(DgnModelR model, DgnCategoryId categoryId, ColorDef color)
    {
    auto geom = CreateGeometryBuilder(model, categoryId);
    GeometryParams params;
    params.SetFillColor(color);
    params.SetLineColor(color);
    geom->Append(params);
    return geom;
    }

