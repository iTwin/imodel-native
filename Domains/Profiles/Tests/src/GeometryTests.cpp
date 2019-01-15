/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/GeometryTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"
#include <DgnPlatform\GenericDomain.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* Test case for Profiles geometry generation/visualization tests.
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct GeometryTestCase : ProfilesTestCase
    {
private:
    Dgn::PhysicalModelPtr m_physicalModelPtr;
    DgnCategoryId m_categoryId;
    Render::GeometryParams m_geometryParams;

protected:
    GeometryTestCase();

    template<typename T>
    RefCountedPtr<T> InsertProfileGeometry (typename T::CreateParams const& createParams, bool placeInNewRow = false);

    DgnCategoryId GetCategoryId() { return m_categoryId; }
    PhysicalModel& GetPhysicalModel() { return *m_physicalModelPtr; }

private:
    void InsertPhysicalElement (ProfilePtr profilePtr, bool placeInNewRow);

    Render::GeometryParams const& GetGeometryParams() const { return m_geometryParams; }
    };

/*---------------------------------------------------------------------------------**//**
* Setup view related platform structures so physical elements with geometry could be
* rendered and viewed in applications (e.g. Gist).
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryTestCase::GeometryTestCase()
    : m_physicalModelPtr (nullptr)
    {
    // Create a PhysicalModel used for creating PhysicalElements
    m_physicalModelPtr = InsertDgnModel<PhysicalModel, PhysicalPartition> ("ProfilesTestPartition_Physical");
    BeAssert (m_physicalModelPtr.IsValid());

    // Create a SpatialCategory used for creating PhysicalElements
    SpatialCategory category (GetDb().GetDictionaryModel(), "ProfilesTestCategory", DgnCategory::Rank::Application);

    DgnDbStatus status;
    SpatialCategoryCPtr categoryPtr = category.Insert (DgnSubCategory::Appearance(), &status);
    BeAssert (status == DgnDbStatus::Success);
    m_categoryId = categoryPtr->GetCategoryId();

    // Setup view and render related structures
    m_geometryParams.SetCategoryId (m_categoryId);
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
RefCountedPtr<T> GeometryTestCase::InsertProfileGeometry (typename T::CreateParams const& createParams, bool placeInNewRow)
    {
    RefCountedPtr<T> profilePtr = typename T::Create (createParams);
    BeAssert (profilePtr.IsValid());

    DgnDbStatus status;
    profilePtr->Insert (&status);
    BeAssert (status == DgnDbStatus::Success && "Failed to insert Profile");

    InsertPhysicalElement (profilePtr, placeInNewRow);

    return profilePtr;
    }

/*---------------------------------------------------------------------------------**//**
* Function used to position multiple profiles in rows at world space.
* Returns an offset to translate profile geometry in world space.
* Transforms profile geometry in its local space to be positioned at bottom left corner
* of its bounding box. Keeps track of last profile placement point and offsets other
* profiles according to that.
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
    double const width = range.XLength();
    double const height = range.YLength();

    Transform translation = Transform::From (DPoint3d::From (range.low.x * -1.0, range.low.y * -1.0));
    BeAssert (geometryPtr->TryTransformInPlace (translation));

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
    InsertProfileGeometry<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "CShape_Plain", 6, 10, 1, 1, 0, 0));
    InsertProfileGeometry<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "CShape_FilletAndRoundedEdge", 6, 10, 1, 1, 0.5, 0.5));
    InsertProfileGeometry<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "CShape_MaxFillet", 6, 10, 1, 1, 2.5, 0.1));
    InsertProfileGeometry<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "CShape_SlopeAndRoundings", 6, 10, 1, 1, 0.5, 0.5, Angle::FromRadians (PI / 18)));

    InsertProfileGeometry<IShapeProfile> (IShapeProfile::CreateParams (GetModel(), "IShape_Plain", 6, 10, 1, 1, 0, 0), true);
    InsertProfileGeometry<IShapeProfile> (IShapeProfile::CreateParams (GetModel(), "IShape_FilletAndRoundedEdge", 6, 10, 1, 1, 0.5, 0.5));
    InsertProfileGeometry<IShapeProfile> (IShapeProfile::CreateParams (GetModel(), "IShape_MaxFillet", 6, 10, 1, 1, 2.5 / 2.0, 0.1));
    InsertProfileGeometry<IShapeProfile> (IShapeProfile::CreateParams (GetModel(), "IShape_SlopeAndRoundings", 6, 10, 1, 1, 0.5, 0.5, Angle::FromRadians (PI / 18)));

    InsertProfileGeometry<AsymmetricIShapeProfile> (AsymmetricIShapeProfile::CreateParams (GetModel(), "AsymmetricI_Plain",
                                                    4, 6, 10, 0.5, 1, 1), true);
    InsertProfileGeometry<AsymmetricIShapeProfile> (AsymmetricIShapeProfile::CreateParams (GetModel(), "AsymmetricI_TopFlangeRoundings",
                                                    4, 6, 10, 0.5, 1, 1, 0.5, 0.25));
    InsertProfileGeometry<AsymmetricIShapeProfile> (AsymmetricIShapeProfile::CreateParams (GetModel(), "AsymmetricI_BottomFlangeRoundings",
                                                    4, 6, 10, 0.5, 1, 1, 0, 0, Angle::FromRadians (0), 0.5, 0.5));
    InsertProfileGeometry<AsymmetricIShapeProfile> (AsymmetricIShapeProfile::CreateParams (GetModel(), "AsymmetricI_MaxFillets",
                                                    4, 6, 10, 0.5, 1, 1, 0.75, 0.1, Angle::FromRadians (0), 1.25, 0.1, Angle::FromRadians (0)));
    InsertProfileGeometry<AsymmetricIShapeProfile> (AsymmetricIShapeProfile::CreateParams (GetModel(), "AsymmetricI_Slopes",
                                                    4, 6, 10, 0.5, 1, 1, 0, 0, Angle::FromRadians (PI / 18), 0, 0, Angle::FromRadians (PI / 18)));
    InsertProfileGeometry<AsymmetricIShapeProfile> (AsymmetricIShapeProfile::CreateParams (GetModel(), "AsymmetricI_SlopesAndRoundings",
                                                    4, 6, 10, 0.5, 1, 1, 0.5, 0.25, Angle::FromRadians (PI / 18), 0.5, 0.5, Angle::FromRadians (PI / 18)));

    InsertProfileGeometry<LShapeProfile> (LShapeProfile::CreateParams (GetModel(), "LShape_Plain", 6, 10, 1, 0, 0), true);
    InsertProfileGeometry<LShapeProfile> (LShapeProfile::CreateParams (GetModel(), "LShape_FilletAndRoundedEdge", 6, 10, 1, 0.5, 0.5));
    InsertProfileGeometry<LShapeProfile> (LShapeProfile::CreateParams (GetModel(), "LShape_MaxFillet", 6, 10, 1, 2.5, 0.1));
    InsertProfileGeometry<LShapeProfile> (LShapeProfile::CreateParams (GetModel(), "LShape_SlopeAndRoundings", 6, 10, 1, 0.5, 0.5, Angle::FromRadians (PI / 32)));

    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_Plain",
                                          6, 10, 1, 1, 0, 0, Angle::FromRadians (0), 0, Angle::FromRadians (0)), true);
    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_FilletAndRoundedEdge",
                                          6, 10, 1, 1, 0.5, 0.5, Angle::FromRadians (0), 0.5, Angle::FromRadians (0)));
    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_MaxFillet",
                                          6, 10, 1, 1, 2.5 / 2.0, 0.1, Angle::FromRadians (0), 0.1, Angle::FromRadians (0)));
    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_SlopeAndRoundings",
                                          6, 10, 1, 1, 0.5, 0.5, Angle::FromRadians (PI / 12), 0.5, Angle::FromRadians (PI / 48)));

    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_Plain", 3.5, 10, 1, 1, 0, 0), true);
    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_FilletAndRoundedEdge", 3.5, 10, 1, 1, 0.5, 0.5));
    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_MaxFillet", 3.5, 10, 1, 1, 1.0, 0.1));
    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_SlopeAndRoundings", 3.5, 10, 1, 1, 0.5, 0.5, Angle::FromRadians (PI / 18)));

    InsertProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams(GetModel(), "CenterLineCShape not rounded, with girth", 3.5, 10.0, 1.0, 1.3, 0.0), true);
    InsertProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams(GetModel(), "CenterLineCShape not rounded, with max girth", 3.5, 10.0, 1.0, 10.0 / 2 - 0.01, 0.0));
    InsertProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams(GetModel(), "CenterLineCShape not rounded, without girth", 3.5, 10.0, 1.0, 0.0, 0.0));
    InsertProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams(GetModel(), "CenterLineCShape rounded, with min girth", 3.5, 10.0, 1.0, 1.0 + PI / 18, PI / 18));
    InsertProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams(GetModel(), "CenterLineCShape rounded, without girth", 3.5, 10.0, 1.0, 0.0, PI / 18));
    InsertProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams(GetModel(), "CenterLineCShape rounded, with max girth", 3.5, 10.0, 1.0, 10.0 / 2 - 0.01, PI / 18));
    InsertProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams(GetModel(), "CenterLineCShape rounded, without girth", 3.5, 10.0, 1.0, 3.5, PI / 18));
    
    InsertProfileGeometry<CenterLineLShapeProfile> (CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape not rounded, without girth", 3.5, 10.00, 1.0, 0.0, 0.0), true);
    InsertProfileGeometry<CenterLineLShapeProfile> (CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape not rounded, with girth", 3.5, 10.00, 1.0, 2.0, 0.0));
    InsertProfileGeometry<CenterLineLShapeProfile> (CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape rounded, with girth", 3.5, 10.00, 1.0, 2.0, PI / 18));
    InsertProfileGeometry<CenterLineLShapeProfile> (CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape rounded, without girth", 3.5, 10.00, 1.0, 0.0, PI / 18));

    InsertProfileGeometry<CenterLineLShapeProfile>(CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape rounded, with long girth", 3.5, 10.00, 1.0, 7.0, PI / 18));
    InsertProfileGeometry<CenterLineLShapeProfile>(CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape rounded, with max girth", 3.5, 10.00, 1.0, 9.0 - 0.01, PI / 18));

    InsertProfileGeometry<CenterLineLShapeProfile>(CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape rounded, with max girth", 10, 10.00, 1.0, 9.0 - 0.01, PI / 18));
    InsertProfileGeometry<CenterLineLShapeProfile>(CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape rounded, with max girth", 10, 10.00, 3.0, 6.9 - 0.01, PI / 18));
    InsertProfileGeometry<CenterLineLShapeProfile>(CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape rounded, with max girth", 10, 10.00, 4.0, 3.0, PI / 18));

    InsertProfileGeometry<CircleProfile> (CircleProfile::CreateParams (GetModel(), "Circle", 3.0), true);
    InsertProfileGeometry<HollowCircleProfile> (HollowCircleProfile::CreateParams (GetModel(), "HollowCircle", 3.0, 0.5));
    InsertProfileGeometry<EllipseProfile> (EllipseProfile::CreateParams (GetModel(), "Ellipse_BiggerXRadius", 3.0, 2.0));
    InsertProfileGeometry<EllipseProfile> (EllipseProfile::CreateParams (GetModel(), "Ellipse_BiggerYRadius", 2.0, 3.0));

    InsertProfileGeometry<RectangleProfile> (RectangleProfile::CreateParams (GetModel(), "Rectangle", 6.0, 4.0), true);
    InsertProfileGeometry<RoundedRectangleProfile> (RoundedRectangleProfile::CreateParams (GetModel(), "RoundedRectangle", 4.0, 6.0, 0.5));
    InsertProfileGeometry<RoundedRectangleProfile> (RoundedRectangleProfile::CreateParams (GetModel(), "RoundedRectangle_MaxRadius", 6.0, 4.0, 2.0));
    InsertProfileGeometry<RoundedRectangleProfile> (RoundedRectangleProfile::CreateParams (GetModel(), "RoundedRectangle_MaxRadius", 4.0, 6.0, 2.0));

    InsertProfileGeometry<HollowRectangleProfile> (HollowRectangleProfile::CreateParams (GetModel(), "HollowRectangle", 4.0, 6.0, 0.5), true);
    InsertProfileGeometry<HollowRectangleProfile> (HollowRectangleProfile::CreateParams (GetModel(), "HollowRectangle_OuterRadius", 4.0, 6.0, 0.5, 0.0, 0.5));
    InsertProfileGeometry<HollowRectangleProfile> (HollowRectangleProfile::CreateParams (GetModel(), "HollowRectangle_InnerRadius", 4.0, 6.0, 0.5, 0.5, 0.0));
    InsertProfileGeometry<HollowRectangleProfile> (HollowRectangleProfile::CreateParams (GetModel(), "HollowRectangle_bothRadiuses", 4.0, 6.0, 0.5, 0.5, 0.5));
    InsertProfileGeometry<HollowRectangleProfile> (HollowRectangleProfile::CreateParams (GetModel(), "HollowRectangle_MaxOuterRadiusAgainstInnerRadius",
                                                   6.0, 6.0, 0.25, 0.5, 0.25 * (2.0 + std::sqrt (2.0)) + 0.49));

    InsertProfileGeometry<TrapeziumProfile> (TrapeziumProfile::CreateParams (GetModel(), "Trapezium", 4.0, 6.0, 4.0, 1.0), true);
    InsertProfileGeometry<TrapeziumProfile> (TrapeziumProfile::CreateParams (GetModel(), "Trapezium wider top", 6.0, 4.0, 4.0, 0.0));
    InsertProfileGeometry<TrapeziumProfile> (TrapeziumProfile::CreateParams (GetModel(), "Trapezium wider top centered", 6.0, 4.0, 4.0, -1.0));
    InsertProfileGeometry<TrapeziumProfile> (TrapeziumProfile::CreateParams (GetModel(), "Trapezium bigger top offset", 2.0, 4.0, 4.0, 6.0));

    LShapeProfilePtr wideL = InsertProfileGeometry<LShapeProfile> (LShapeProfile::CreateParams (GetModel(), "L 6x3 (for DoubleL)", 6.0, 3.0, 1.0, 0.5), true);
    InsertProfileGeometry<DoubleLShapeProfile> (DoubleLShapeProfile::CreateParams (GetModel(), "DoubleL 6x3 LLBB", 0.25, wideL->GetElementId(), DoubleLShapeProfileType::LLBB));
    InsertProfileGeometry<DoubleLShapeProfile> (DoubleLShapeProfile::CreateParams (GetModel(), "DoubleL 6x3 SLBB", 0.25, wideL->GetElementId(), DoubleLShapeProfileType::SLBB));
    LShapeProfilePtr deepL = InsertProfileGeometry<LShapeProfile> (LShapeProfile::CreateParams (GetModel(), "L 3x6 (for DoubleL)", 3.0, 6.0, 1.0, 0.5));
    InsertProfileGeometry<DoubleLShapeProfile> (DoubleLShapeProfile::CreateParams (GetModel(), "DoubleL 3x6 LLBB", 0.25, deepL->GetElementId(), DoubleLShapeProfileType::LLBB));
    InsertProfileGeometry<DoubleLShapeProfile> (DoubleLShapeProfile::CreateParams (GetModel(), "DoubleL 3x6 SLBB", 0.25, deepL->GetElementId(), DoubleLShapeProfileType::SLBB));
    InsertProfileGeometry<DoubleLShapeProfile> (DoubleLShapeProfile::CreateParams (GetModel(), "DoubleL 3x6 SLBB (0 spacing)", 0.0, deepL->GetElementId(), DoubleLShapeProfileType::SLBB));

    CShapeProfilePtr cShape = InsertProfileGeometry<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "C (for DoubleL)", 3.0, 6.0, 0.5, 0.5, 0.25), true);
    InsertProfileGeometry<DoubleCShapeProfile> (DoubleCShapeProfile::CreateParams (GetModel(), "DoubleC", 0.25, cShape->GetElementId()));
    InsertProfileGeometry<DoubleCShapeProfile> (DoubleCShapeProfile::CreateParams (GetModel(), "DoubleC (0 spacing)", 0.0, cShape->GetElementId()));

    HollowCircleProfilePtr leftEyePtr = InsertElement<HollowCircleProfile> (HollowCircleProfile::CreateParams (GetModel(), "Left eye", 1.0, 0.15));
    CircleProfilePtr leftEyeBallPtr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "Left eye ball", 0.2));
    HollowCircleProfilePtr rightEyePtr = InsertElement<HollowCircleProfile> (HollowCircleProfile::CreateParams (GetModel(), "Right eye", 1.0, 0.15));
    CircleProfilePtr rightEyeBallPtr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "Right eye ball", 0.2));
    TrapeziumProfilePtr nosePtr = InsertElement<TrapeziumProfile> (TrapeziumProfile::CreateParams (GetModel(), "Nose", 0.0001, 1, 2.0, 0.5));
    CShapeProfilePtr mouthPtr = InsertElement<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "Mouth", 1.5, 4.0, 0.15, 0.15, 0.5));
    bvector<CompositeProfileComponent> faceComponents =
        {
        CompositeProfileComponent (*leftEyePtr, false, DPoint2d::From (-2.0, 0.0), Angle::FromRadians (0.0)),
        CompositeProfileComponent (*leftEyeBallPtr, false, DPoint2d::From (-2.0, 0.0), Angle::FromRadians (0.0)),
        CompositeProfileComponent (*rightEyePtr, false, DPoint2d::From (2.0, 0.0), Angle::FromRadians (0.0)),
        CompositeProfileComponent (*rightEyeBallPtr, false, DPoint2d::From (2.0, 0.0), Angle::FromRadians (0.0)),
        CompositeProfileComponent (*nosePtr, false, DPoint2d::From (0.0, -2.0), Angle::FromRadians (0.0)),
        CompositeProfileComponent (*mouthPtr, false, DPoint2d::From (0.0, -3.5), Angle::FromRadians (PI / 2.0))
        };
    InsertProfileGeometry<ArbitraryCompositeProfile> (ArbitraryCompositeProfile::CreateParams (GetModel(), "Smiley face", faceComponents), true);
    }
