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

protected:
    GeometryTestCase();

    template<typename T>
    void InsertProfileGeometry (typename T::CreateParams const& createParams, bool placeInNewRow = false);

private:
    void InsertPhysicalElement (ProfilePtr profilePtr, bool placeInNewRow);

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
* Creates a Profile from given CreateParams and creates a PhysicalElement with profiles
* geometry assigned to it for visual testing.
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
void GeometryTestCase::InsertProfileGeometry (typename T::CreateParams const& createParams, bool placeInNewRow)
    {
    RefCountedPtr<T> profilePtr = typename T::Create (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    DgnDbStatus status;
    profilePtr->Insert (&status);
    ASSERT_EQ (DgnDbStatus::Success, status) << "Failed to insert Profile to DgnDb.";

    InsertPhysicalElement (profilePtr, placeInNewRow);
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
void GeometryTestCase::InsertPhysicalElement (ProfilePtr profilePtr, bool placeInNewRow)
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
TEST_F(GeometryTestCase, ProfilesGemetry)
    {
    InsertProfileGeometry<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "CShape_Plain", 6, 10, 1, 1, 0, 0, 0));
    InsertProfileGeometry<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "CShape_FilletAndRoundedEdge", 6, 10, 1, 1, 0.5, 0.5, 0.0));
    InsertProfileGeometry<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "CShape_MaxFillet", 6, 10, 1, 1, 2.5, 0.1, 0.0));
    InsertProfileGeometry<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "CShape_SlopeAndRoundings", 6, 10, 1, 1, 0.5, 0.5, PI / 18));

    InsertProfileGeometry<IShapeProfile> (IShapeProfile::CreateParams (GetModel(), "IShape_Plain", 6, 10, 1, 1, 0, 0, 0), true);
    InsertProfileGeometry<IShapeProfile> (IShapeProfile::CreateParams (GetModel(), "IShape_FilletAndRoundedEdge", 6, 10, 1, 1, 0.5, 0.5, 0.0));
    InsertProfileGeometry<IShapeProfile> (IShapeProfile::CreateParams (GetModel(), "IShape_MaxFillet", 6, 10, 1, 1, 2.5 / 2.0, 0.1, 0.));
    InsertProfileGeometry<IShapeProfile> (IShapeProfile::CreateParams (GetModel(), "IShape_SlopeAndRoundings", 6, 10, 1, 1, 0.5, 0.5, PI / 18));

    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_Plain", 3.5, 10, 1, 1, 0, 0, 0), true);
    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_FilletAndRoundedEdge", 3.5, 10, 1, 1, 0.5, 0.5, 0.0));
    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_MaxFillet", 3.5, 10, 1, 1, 1.0, 0.1, 0.));
    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_SlopeAndRoundings", 3.5, 10, 1, 1, 0.5, 0.5, PI / 18));

    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_Plain", 6, 10, 1, 1, 0, 0, 0, 0, 0), true);
    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_FilletAndRoundedEdge", 6, 10, 1, 1, 0.5, 0.5, 0, 0.5, 0));
    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_MaxFillet", 6, 10, 1, 1, 2.5 / 2.0, 0.1, 0, 0.1, 0));
    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_SlopeAndRoundings", 6, 10, 1, 1, 0.5, 0.5, PI / 12, 0.5, PI / 48));
    }
