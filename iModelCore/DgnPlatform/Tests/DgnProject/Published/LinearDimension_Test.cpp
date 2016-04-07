/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/LinearDimension_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/Dimension/Dimension.h>
#include <ECObjects/DesignByContract.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/
struct LinearDimensionTest : public GenericDgnModelTestFixture
{
private:
    const    Utf8CP m_modelName     = "TestModel";
    const    Utf8CP m_categoryName  = "TestCategory";

    typedef GenericDgnModelTestFixture T_Super;

    DgnModelId              m_modelId;
    DgnCategoryId           m_categoryId;
    DgnElementId            m_dimStyleId;

public:
LinearDimensionTest() : GenericDgnModelTestFixture (__FILE__, true /*2D*/, false /*needBriefcase*/)
    {
    }

void SetUp () override
    {
    T_Super::SetUp();

    // Create a category
    DgnCategory category(DgnCategory::CreateParams(GetDgnDb(), m_categoryName, DgnCategory::Scope::Physical));
    DgnSubCategory::Appearance appearance;
    category.Insert(appearance);

    m_categoryId = category.GetCategoryId();
    ASSERT_TRUE(m_categoryId.IsValid());

    // Create a text style
    AnnotationTextStylePtr textStyle = AnnotationTextStyle::Create(GetDgnDb());
    textStyle->SetName(GetTextStyleName());
    textStyle->SetHeight(GetTextStyleHeight());
    textStyle->SetFontId(GetDgnDb().Fonts().AcquireId(DgnFontManager::GetAnyLastResortFont()));
    textStyle->Insert();

    ASSERT_TRUE(textStyle->GetElementId().IsValid());

    // Create a dimension style
    DimensionStylePtr dimStyle = DimensionStyle::Create(textStyle->GetElementId(), GetDgnDb());
    dimStyle->SetName(GetDimensionStyleName());
    dimStyle->Insert();

    m_dimStyleId = dimStyle->GetElementId();
    ASSERT_TRUE(m_dimStyleId.IsValid());

    // Create a 2d model
    DgnModelPtr model = new GeometricModel2d(GeometricModel2d::CreateParams(GetDgnDb(), DgnClassId(GetDgnDb().Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_GeometricModel2d)), DgnModel::CreateModelCode(m_modelName)));
    ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

    m_modelId = model->GetModelId();
    ASSERT_TRUE(m_modelId.IsValid());

#define WANT_VIEW
#if defined (WANT_VIEW)
    // This is only here to aid in debugging so you can open the file in a viewer and see the element you just created.
    //.........................................................................................
    DrawingViewDefinition view(DrawingViewDefinition::CreateParams(GetDgnDb(), "LinearDimensionTest",
                ViewDefinition::Data(m_modelId, DgnViewSource::Generated)));
    EXPECT_TRUE(view.Insert().IsValid());

    DRange3d  madeUpRange = DRange3d::From (DPoint3d::From(-10.0, -10.0, -10.0), DPoint3d::From(10.0, 10.0, 10.0));

    ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);

    DrawingViewController viewController(GetDgnDb(), view.GetViewId());
    viewController.SetStandardViewRotation(StandardView::Top);
    viewController.LookAtVolume(madeUpRange, nullptr, &viewMargin);
    //viewController.LookAtVolume(insertedAnnotationElement->CalculateRange3d(), nullptr, &viewMargin);
    viewController.GetViewFlagsR().SetRenderMode(Render::RenderMode::Wireframe);
    viewController.ChangeCategoryDisplay(m_categoryId, true);
    viewController.ChangeModelDisplay(m_modelId, true);

    EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());
    GetDgnDb().SaveSettings();
#endif
    }

DgnDbR                  GetDgnDb()              { return *T_Super::GetDgnDb(); }
DgnModelId              GetModelId()            { return m_modelId; }
DgnCategoryId           GetCategoryId()         { return m_categoryId; }
Utf8CP                  GetDimensionStyleName() { return "TestDimensionStyle"; }
Utf8CP                  GetTextStyleName()      { return "TestTextStyle"; }
double                  GetTextStyleHeight()    { return 0.25; }
DgnElementId            GetDimensionStyleId()   { return m_dimStyleId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
LinearDimension2dPtr    CreateBasicDimension (bvector<DPoint3d> const& points, AngleInDegrees const& angle)
    {
    BeAssert (points.size() > 1);

    DgnDbR          db          = GetDgnDb();
    DgnModelId      modelId     = GetModelId();
    DgnCategoryId   categoryId  = GetCategoryId();

    Placement2d placement(DPoint2d::From(points[0]), angle, ElementAlignedBox2d(0,0,1,1));

    LinearDimension2d::CreateParams  createParams (db, modelId, LinearDimension2d::QueryClassId(db), categoryId, placement);
    LinearDimension2dPtr             dimension = LinearDimension2d::Create(createParams, GetDimensionStyleId(), points[1]);
    EXPECT_TRUE (dimension.IsValid());

    for (size_t iPoint = 2; iPoint < points.size(); iPoint++)
        dimension->AppendPoint (points[iPoint]);

    return dimension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    InsertElement (LinearDimension2dR element)
    {
    LinearDimension2dCPtr  insertedElement = element.Insert();
    EXPECT_TRUE(insertedElement.IsValid());

    DgnElementId elementId = insertedElement->GetElementId();
    EXPECT_TRUE(elementId.IsValid());

    return elementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void        UpdateElement (LinearDimension2dR element)
    {
    LinearDimension2dCPtr  updatedElement = element.Update();
    EXPECT_TRUE(updatedElement.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    CreateBasicDimensionPersisted (bvector<DPoint3d> const& points, AngleInDegrees const& angle)
    {
    LinearDimension2dPtr   dimension = CreateBasicDimension (points, angle);
    return InsertElement (*dimension);
    }

}; // LinearDimensionTest

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LinearDimensionTest, BasicPersist)
    {
    bvector<DPoint3d>   points;
    points.push_back (DPoint3d::From (0.0, 0.0, 0.0));
    points.push_back (DPoint3d::From (2.5, 0.0, 0.0));
    points.push_back (DPoint3d::From (10.0, 1.0, 0.0));

    DgnElementId elementId = CreateBasicDimensionPersisted (points, AngleInDegrees::FromDegrees (45));

    // Purge the cache so that we don't get a cached element.
    GetDgnDb().Elements().Purge(0);

    LinearDimension2dCPtr readDimension = LinearDimension2d::Get(GetDgnDb(), elementId);
    ASSERT_TRUE(readDimension.IsValid());

    printf ("Point Count %d\n", readDimension->GetPointCount());
    for (uint32_t index = 0; index < readDimension->GetPointCount(); index++)
        {
        DPoint3d point = readDimension->GetPoint(index);
        printf ("   Point[%d]: %f, %f\n", index, point.x, point.y);
        }
    }

