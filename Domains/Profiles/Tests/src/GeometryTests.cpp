/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/GeometryTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"
#include <DgnPlatform/GenericDomain.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* Test case for Profiles geometry generation/visualization tests.
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct GeometryTestCase : ProfilesTestCase
    {
private:
    Render::GeometryParams m_geometryParams;

public:
    GeometryTestCase();

    void InsertProfileGeometry (ProfilePtr profilePtr, bool placeInNewRow = false);

    Render::GeometryParams const& GetGeometryParams() const { return m_geometryParams; }
    };

/*---------------------------------------------------------------------------------**//**
* Setup view related platfrom structures so physical elements with geometry could be
* rendered and viewed in applications (e.g. Gist).
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryTestCase::GeometryTestCase()
    {
    m_geometryParams.SetCategoryId (GetCategoryId());
    m_geometryParams.SetFillDisplay (Render::FillDisplay::Always);
    m_geometryParams.SetLineColor (ColorDef::White());
    m_geometryParams.SetFillColor (ColorDef::DarkGrey());
    m_geometryParams.SetWeight (1);

    DefinitionModelR dictionaryModel = GetDb().GetDictionaryModel();
    CategorySelectorPtr categorySelectorPtr = new CategorySelector (dictionaryModel, "ProfilesTest");

    ModelSelectorPtr modelSelectorPtr = new ModelSelector (dictionaryModel, "ProfilesTest");
    modelSelectorPtr->AddModel (GetPhysicalModel().GetModelId());

    DisplayStyle3dPtr displayStylePtr = new DisplayStyle3d (dictionaryModel, "ProfilesTest");
    displayStylePtr->SetBackgroundColor (ColorDef::LightGrey());
    displayStylePtr->SetSkyBoxEnabled (false);
    displayStylePtr->SetGroundPlaneEnabled (false);

    Render::ViewFlags viewFlags = displayStylePtr->GetViewFlags();
    viewFlags.SetRenderMode (Render::RenderMode::Wireframe);
    viewFlags.SetShowTransparency (false);
    viewFlags.SetShowGrid (true);
    viewFlags.SetShowAcsTriad (true);
    displayStylePtr->SetViewFlags (viewFlags);

    OrthographicViewDefinition view (dictionaryModel, "Structure View", *categorySelectorPtr, *displayStylePtr, *modelSelectorPtr);
    view.SetCategorySelector (*categorySelectorPtr);
    view.SetStandardViewRotation (StandardView::Top);
    view.LookAtVolume (GetDb().GeoLocation().GetProjectExtents());
    view.Insert();
    }

/*---------------------------------------------------------------------------------**//**
* Callculates an offset to translate profile geometry in world space. Keeps track of
* last profile placement point and offsets other profiles according to that.
* Used to position profiles in rows at world space.
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static DPoint3d offsetProfilePlacement (IGeometryPtr& geometryPtr, bool placeInNewRow)
    {
    static double xPlacement = 0.0;
    static double yPlacement = 0.0;
    static double maxHeight = 0.0;

    if (placeInNewRow)
        {
        xPlacement = 0.0;
        yPlacement += maxHeight + 1.0;
        maxHeight = 0.0;
        }

    DPoint3d offset = DPoint3d::From (xPlacement, yPlacement, 0.0);

    DRange3d range;
    BeAssert (geometryPtr->TryGetRange (range));
    double width = range.high.x - range.low.x;
    double height = range.high.y - range.low.y;

    xPlacement += width + 1.0;
    if (height > maxHeight)
        maxHeight = height;

    return offset;
    }

/*---------------------------------------------------------------------------------**//**
* Creates a PhysicalElement, assings Profile geometry to it and offsets it in world
* space.
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryTestCase::InsertProfileGeometry (ProfilePtr profilePtr, bool placeInNewRow)
    {
    PhysicalElementPtr physicaleElementPtr = GenericPhysicalObject::Create (GetPhysicalModel(), GetCategoryId());
    physicaleElementPtr->SetUserLabel (profilePtr->GetName().c_str());

    GeometrySourceP pGeometrySource = physicaleElementPtr->ToGeometrySourceP();
    pGeometrySource->SetCategoryId (GetCategoryId());

    IGeometryPtr profileGeometryPtr = profilePtr->GetShape();

    Placement3d elementPlacement;
    elementPlacement.GetOriginR() = offsetProfilePlacement (profileGeometryPtr, placeInNewRow);
    physicaleElementPtr->SetPlacement (elementPlacement);

    GeometryBuilderPtr builder = GeometryBuilder::Create (*pGeometrySource);
    builder->Append (GetCategoryId());
    builder->Append (GetGeometryParams());
    builder->Append (*profileGeometryPtr);
    builder->Finish (*pGeometrySource);

    DgnDbStatus insertStatus;
    physicaleElementPtr->Insert (&insertStatus);
    ASSERT_TRUE (insertStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeometryTestCase, CShapeGeometry)
    {
    CShapeProfile::CreateParams params1 (GetModel(), "CShape_Plain", 6, 10, 1, 1, 0, 0, 0);
    CShapeProfilePtr profilePtr1 = CShapeProfile::Create (params1);
    BeAssert (profilePtr1->Insert().IsValid());
    InsertProfileGeometry (profilePtr1);

    CShapeProfile::CreateParams profileParams2 (GetModel(), "CShape_FilletAndRoundedEdge", 6, 10, 1, 1, 0.5, 0.5, 0.0);
    CShapeProfilePtr profilePtr2 = CShapeProfile::Create (profileParams2);
    BeAssert (profilePtr2->Insert().IsValid());
    InsertProfileGeometry (profilePtr2);

    CShapeProfile::CreateParams profileParams3 (GetModel(), "CShape_MaxFillet", 6, 10, 1, 1, 2.5, 0.1, 0.0);
    CShapeProfilePtr profilePtr3 = CShapeProfile::Create (profileParams3);
    BeAssert (profilePtr3->Insert().IsValid());
    InsertProfileGeometry (profilePtr3);

    CShapeProfile::CreateParams profileParams4 (GetModel(), "CShape_SlopeAndRoundings", 6, 10, 1, 1, 0.5, 0.5, PI / 18);
    CShapeProfilePtr profilePtr4 = CShapeProfile::Create (profileParams4);
    BeAssert (profilePtr4->Insert().IsValid());
    InsertProfileGeometry (profilePtr4);

    IShapeProfile::CreateParams params5 (GetModel(), "IShape_Plain", 6, 10, 1, 1, 0, 0, 0);
    IShapeProfilePtr profilePtr5 = IShapeProfile::Create (params5);
    BeAssert (profilePtr5->Insert().IsValid());
    InsertProfileGeometry (profilePtr5, true);

    IShapeProfile::CreateParams params6 (GetModel(), "IShape_FilletAndRoundedEdge", 6, 10, 1, 1, 0.5, 0.5, 0.0);
    IShapeProfilePtr profilePtr6 = IShapeProfile::Create (params6);
    BeAssert (profilePtr6->Insert().IsValid());
    InsertProfileGeometry (profilePtr6);

    IShapeProfile::CreateParams params7 (GetModel(), "IShape_MaxFillet", 6, 10, 1, 1, 2.5 / 2.0, 0.1, 0.0);
    IShapeProfilePtr profilePtr7 = IShapeProfile::Create (params7);
    BeAssert (profilePtr7->Insert().IsValid());
    InsertProfileGeometry (profilePtr7);

    IShapeProfile::CreateParams profileParams8 (GetModel(), "IShape_SlopeAndRoundings", 6, 10, 1, 1, 0.5, 0.5, PI / 18);
    IShapeProfilePtr profilePtr8 = IShapeProfile::Create (profileParams8);
    BeAssert (profilePtr8->Insert().IsValid());
    InsertProfileGeometry (profilePtr8);
    }
