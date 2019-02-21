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
    Render::GeometryParams m_shapeGeometryParams;
    Render::GeometryParams m_centerLineGeometryParams;
protected:
    GeometryTestCase();

    template<typename T>
    RefCountedPtr<T> InsertProfileGeometry (typename T::CreateParams const& createParams, bool placeInNewRow = false);
    template<typename T>
    void InsertCenterlineProfileGeometry (typename T::CreateParams const& createParams, bool placeInNewRow = false);
    DgnCategoryId GetCategoryId() { return m_categoryId; }
    PhysicalModel& GetPhysicalModel() { return *m_physicalModelPtr; }

private:
    void InsertPhysicalElement (ProfilePtr profilePtr, bool placeInNewRow);
    void InsertPhysicalElementForCenterLineProfile (ProfilePtr profilePtr, IGeometryPtr centerLineGeom, bool placeInNewRow);

    Render::GeometryParams const& GetGeometryParams() const { return m_shapeGeometryParams; }
    Render::GeometryParams const& GetGeometryParamsForCenterLine() const { return m_centerLineGeometryParams; }
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
    m_shapeGeometryParams.SetCategoryId (m_categoryId);
    m_shapeGeometryParams.SetFillDisplay (Render::FillDisplay::Always);
    m_shapeGeometryParams.SetLineColor (ColorDef::White());
    m_shapeGeometryParams.SetFillColor (ColorDef::DarkGrey());
    m_shapeGeometryParams.SetWeight (1);

    m_centerLineGeometryParams.SetCategoryId (m_categoryId);
    m_centerLineGeometryParams.SetLineColor (ColorDef::Green());
    m_centerLineGeometryParams.SetWeight (1);

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
* Creates a CenterLineCShapeProfilePtr from given CreateParams and creates a PhysicalElement with profiles
* geometry assigned to it for visual testing.
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
void GeometryTestCase::InsertCenterlineProfileGeometry (typename T::CreateParams const& createParams, bool placeInNewRow)
    {
    RefCountedPtr<T> profilePtr = typename T::Create (createParams);

    BeAssert (profilePtr.IsValid());

    DgnDbStatus status;
    profilePtr->Insert (&status);
    BeAssert (status == DgnDbStatus::Success && "Failed to insert Profile");

    InsertPhysicalElementForCenterLineProfile (profilePtr, profilePtr->GetCenterLine(), placeInNewRow);
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

    GeometrySource* pGeometrySource = physicaleElementPtr->ToGeometrySourceP();
    pGeometrySource->SetCategoryId(GetCategoryId());

    IGeometryPtr profileGeometryPtr = profilePtr->GetShape();

    Placement3d elementPlacement;
    elementPlacement.GetOriginR() = offsetProfilePlacement (profileGeometryPtr, placeInNewRow);
    physicaleElementPtr->SetPlacement (elementPlacement);

    GeometryBuilderPtr builder = GeometryBuilder::Create (*pGeometrySource);
    builder->Append (GetGeometryParams());
    builder->Append (*profileGeometryPtr);
    builder->Finish (*pGeometrySource);

    DgnDbStatus insertStatus;
    physicaleElementPtr->Insert (&insertStatus);
    ASSERT_TRUE (insertStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* Creates a PhysicalElement, assings CenterLine Profile geometry to it and offsets it in world
* space.
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryTestCase::InsertPhysicalElementForCenterLineProfile (ProfilePtr profilePtr, IGeometryPtr centerLineGeom, bool placeInNewRow)
    {
    PhysicalElementPtr physicaleElementPtr = GenericPhysicalObject::Create (GetPhysicalModel(), GetCategoryId());
    physicaleElementPtr->SetUserLabel (profilePtr->GetName().c_str());

    GeometrySource* pGeometrySource = physicaleElementPtr->ToGeometrySourceP();
    IGeometryPtr profileGeometryPtr = profilePtr->GetShape();
    DRange3d range = {0};
    BeAssert (profileGeometryPtr->TryGetRange (range));

    Placement3d elementPlacement;
    elementPlacement.GetOriginR() = offsetProfilePlacement (profileGeometryPtr, placeInNewRow);
    physicaleElementPtr->SetPlacement (elementPlacement);

    GeometryBuilderPtr builder = GeometryBuilder::Create (*pGeometrySource);
    builder->Append (GetGeometryParams());
    builder->Append (*profileGeometryPtr);

    if (centerLineGeom.IsValid())
        {
        Transform translation = Transform::From (DPoint3d::From (range.low.x * -1.0, range.low.y * -1.0, 0.01));
        BeAssert (centerLineGeom->TryTransformInPlace (translation));

        builder->Append (GetGeometryParamsForCenterLine());
        builder->Append (*centerLineGeom);
        }

    builder->Finish (*pGeometrySource);

    DgnDbStatus insertStatus;
    physicaleElementPtr->Insert (&insertStatus);
    ASSERT_TRUE (insertStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeometryTestCase, ProfilesGeometry)
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

    InsertProfileGeometry<SchifflerizedLShapeProfile> (SchifflerizedLShapeProfile::CreateParams (GetModel(), "Schifflerized L - plain", 6.0, 0.5), true);
    InsertProfileGeometry<SchifflerizedLShapeProfile> (SchifflerizedLShapeProfile::CreateParams (GetModel(), "Schifflerized L", 6.0, 0.5, 0.5, 0.25, 0.5));
    InsertProfileGeometry<SchifflerizedLShapeProfile> (SchifflerizedLShapeProfile::CreateParams (GetModel(), "Schifflerized L - max fillet", 6.0, 0.5, 1.0, 0.5, 0.25));

    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_Plain",
                                          6, 10, 1, 1, 0, 0, Angle::FromRadians (0), 0, Angle::FromRadians (0)), true);
    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_FilletAndRoundedEdge",
                                          6, 10, 1, 1, 0.5, 0.5, Angle::FromRadians (0), 0.5, Angle::FromRadians (0)));
    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_MaxFillet",
                                          6, 10, 1, 1, 2.5 / 2.0, 0.1, Angle::FromRadians (0), 0.1, Angle::FromRadians (0)));
    InsertProfileGeometry<TShapeProfile> (TShapeProfile::CreateParams (GetModel(), "TShape_SlopeAndRoundings",
                                          6, 10, 1, 1, 0.5, 0.5, Angle::FromRadians (PI / 12), 0.5, Angle::FromRadians (PI / 48)));

    InsertProfileGeometry<TTShapeProfile> (TTShapeProfile::CreateParams (GetModel(), "TTShape plain",
                                          10, 10, 1, 1, 2, 0, 0, Angle::FromRadians (0), 0, Angle::FromRadians (0)), true);
    InsertProfileGeometry<TTShapeProfile> (TTShapeProfile::CreateParams (GetModel(), "TTShape fillet and rounded edges",
                                          10, 10, 1, 1, 2, 0.5, 0.5, Angle::FromRadians (0), 0.5, Angle::FromRadians (0)));
    InsertProfileGeometry<TTShapeProfile> (TTShapeProfile::CreateParams (GetModel(), "TTShape max fillet (fillet between webs cliped)",
                                          10, 10, 1, 1, 2, 1.5, 0.1, Angle::FromRadians (0), 0.1, Angle::FromRadians (0)));
    InsertProfileGeometry<TTShapeProfile> (TTShapeProfile::CreateParams (GetModel(), "TTShape with slopes",
                                          10, 10, 1, 1, 2, 0, 0, Angle::FromRadians (PI / 32), 0, Angle::FromRadians (PI / 64)));
    InsertProfileGeometry<TTShapeProfile> (TTShapeProfile::CreateParams (GetModel(), "TTShape max web slope",
                                          10, 10, 1, 1, 2, 0, 0, Angle::FromRadians (0), 0, Angle::FromRadians (0.11)));
    InsertProfileGeometry<TTShapeProfile> (TTShapeProfile::CreateParams (GetModel(), "TTShape max web slope with flange slope present",
                                          10, 10, 1, 1, 2, 0, 0, Angle::FromRadians (0.3217), 0, Angle::FromRadians (0.124)));
    InsertProfileGeometry<TTShapeProfile> (TTShapeProfile::CreateParams (GetModel(), "TTShape slopes and roundings",
                                          10, 10, 1, 1, 2, 0.25, 0.5, Angle::FromRadians (PI / 24), 0.3, Angle::FromRadians (PI / 64)));

    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_Plain", 3.5, 10, 1, 1, 0, 0), true);
    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_FilletAndRoundedEdge", 3.5, 10, 1, 1, 0.5, 0.5));
    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_MaxFillet", 3.5, 10, 1, 1, 1.0, 0.1));
    InsertProfileGeometry<ZShapeProfile> (ZShapeProfile::CreateParams (GetModel(), "ZShape_SlopeAndRoundings", 3.5, 10, 1, 1, 0.5, 0.5, Angle::FromRadians (PI / 18)));

    InsertCenterlineProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams (GetModel(), "CenterLineCShape filletRadius 0, girth 0",
                                                              6.0, 10.0, 1.0, 0.0, 0.0), true);
    InsertCenterlineProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams (GetModel(), "CenterLineCShape girth 0",
                                                              6.0, 10.0, 1.0, 0.0, 0.25));
    InsertCenterlineProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams (GetModel(), "CenterLineCShape filletRadius 0",
                                                              6.0, 10.0, 1.0, 2.0, 0.0));
    InsertCenterlineProfileGeometry<CenterLineCShapeProfile> (CenterLineCShapeProfile::CreateParams (GetModel(), "CenterLineCShape",
                                                              6.0, 10.0, 1.0, 2.0, 0.25));

    InsertCenterlineProfileGeometry<CenterLineLShapeProfile> (CenterLineLShapeProfile::CreateParams (GetModel(), "CenterLineLShape filletRadius 0, girth 0",
                                                              6.0, 10.0, 1.0, 0.0, 0.0), true);
    InsertCenterlineProfileGeometry<CenterLineLShapeProfile> (CenterLineLShapeProfile::CreateParams (GetModel(), "CenterLineLShape rounded, without girth",
                                                              6.0, 10.0, 1.0, 0.0, 0.25));
    InsertCenterlineProfileGeometry<CenterLineLShapeProfile> (CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape filletRadius 0",
                                                              6.0, 10.0, 1.0, 2.0, 0.0));
    InsertCenterlineProfileGeometry<CenterLineLShapeProfile> (CenterLineLShapeProfile::CreateParams(GetModel(), "CenterLineLShape",
                                                              6.0, 10.0, 1.0, 2.0, 0.25));

    InsertCenterlineProfileGeometry<CenterLineZShapeProfile> (CenterLineZShapeProfile::CreateParams (GetModel(), "CenterLineZShape filletRadius 0, girth 0",
                                                              3.5, 10.0, 1.0, 0.0, 0.0), true);
    InsertCenterlineProfileGeometry<CenterLineZShapeProfile> (CenterLineZShapeProfile::CreateParams (GetModel(), "CenterLineZShape girth 0",
                                                              3.5, 10.0, 1.0, 0.25, 0.0));
    InsertCenterlineProfileGeometry<CenterLineZShapeProfile> (CenterLineZShapeProfile::CreateParams (GetModel(), "CenterLineZShape filletRadius 0",
                                                              3.5, 10.0, 1.0, 0.0, 2.0));
    InsertCenterlineProfileGeometry<CenterLineZShapeProfile> (CenterLineZShapeProfile::CreateParams (GetModel(), "CenterLineZShape",
                                                              3.5, 10.0, 1.0, 0.25, 2.0));

    InsertCenterlineProfileGeometry<BentPlateProfile> (BentPlateProfile::CreateParams (GetModel(), "BentPlate", 6.0, 0.5, Angle::FromDegrees (135), 2.0, 0.5), true);
    InsertCenterlineProfileGeometry<BentPlateProfile> (BentPlateProfile::CreateParams (GetModel(), "BentPlate", 6.0, 0.5, Angle::FromDegrees (60), 3.0, 0.1));
    InsertCenterlineProfileGeometry<BentPlateProfile> (BentPlateProfile::CreateParams (GetModel(), "BentPlate", 6.0, 0.5, Angle::FromDegrees (45), 2.0, 0.5));
    InsertCenterlineProfileGeometry<BentPlateProfile> (BentPlateProfile::CreateParams (GetModel(), "BentPlate", 4.0, 0.5, Angle::FromDegrees (90), 1.0, 0.05));

    InsertProfileGeometry<CircleProfile> (CircleProfile::CreateParams (GetModel(), "Circle", 3.0), true);
    InsertProfileGeometry<HollowCircleProfile> (HollowCircleProfile::CreateParams (GetModel(), "HollowCircle", 3.0, 0.5));
    InsertProfileGeometry<EllipseProfile> (EllipseProfile::CreateParams (GetModel(), "Ellipse_BiggerXRadius", 3.0, 2.0));
    InsertProfileGeometry<EllipseProfile> (EllipseProfile::CreateParams (GetModel(), "Ellipse_BiggerYRadius", 2.0, 3.0));

    InsertProfileGeometry<RectangleProfile> (RectangleProfile::CreateParams (GetModel(), "Rectangle", 6.0, 4.0), true);
    InsertProfileGeometry<RoundedRectangleProfile> (RoundedRectangleProfile::CreateParams (GetModel(), "RoundedRectangle", 4.0, 6.0, 0.5));
    InsertProfileGeometry<CapsuleProfile> (CapsuleProfile::CreateParams (GetModel(), "Capsule 6x4", 6.0, 4.0));
    InsertProfileGeometry<CapsuleProfile> (CapsuleProfile::CreateParams (GetModel(), "Capsule 4x6", 4.0, 6.0));

    InsertProfileGeometry<HollowRectangleProfile> (HollowRectangleProfile::CreateParams (GetModel(), "HollowRectangle", 4.0, 6.0, 0.5), true);
    InsertProfileGeometry<HollowRectangleProfile> (HollowRectangleProfile::CreateParams (GetModel(), "HollowRectangle_OuterRadius", 4.0, 6.0, 0.5, 0.0, 0.5));
    InsertProfileGeometry<HollowRectangleProfile> (HollowRectangleProfile::CreateParams (GetModel(), "HollowRectangle_InnerRadius", 4.0, 6.0, 0.5, 0.5, 0.0));
    InsertProfileGeometry<HollowRectangleProfile> (HollowRectangleProfile::CreateParams (GetModel(), "HollowRectangle_bothRadiuses", 4.0, 6.0, 0.5, 0.5, 0.5));

    InsertProfileGeometry<TrapeziumProfile> (TrapeziumProfile::CreateParams (GetModel(), "Trapezium", 4.0, 6.0, 4.0, 1.0), true);
    InsertProfileGeometry<TrapeziumProfile> (TrapeziumProfile::CreateParams (GetModel(), "Trapezium wider top", 6.0, 4.0, 4.0, 0.0));
    InsertProfileGeometry<TrapeziumProfile> (TrapeziumProfile::CreateParams (GetModel(), "Trapezium wider top centered", 6.0, 4.0, 4.0, -1.0));
    InsertProfileGeometry<TrapeziumProfile> (TrapeziumProfile::CreateParams (GetModel(), "Trapezium bigger top offset", 2.0, 4.0, 4.0, 6.0));

    InsertProfileGeometry<RegularPolygonProfile> (RegularPolygonProfile::CreateParams (GetModel(), "Regular polygon - 3 side", 3, 4.0), true);
    InsertProfileGeometry<RegularPolygonProfile> (RegularPolygonProfile::CreateParams (GetModel(), "Regular polygon - 6 side", 6, 4.0));
    InsertProfileGeometry<RegularPolygonProfile> (RegularPolygonProfile::CreateParams (GetModel(), "Regular polygon - 16 side", 16, 4.0));

        {
        LShapeProfilePtr wideL = InsertProfileGeometry<LShapeProfile> (LShapeProfile::CreateParams (GetModel(), "L 6x3 (for DoubleL)", 6.0, 3.0, 1.0, 0.5), true);
        InsertProfileGeometry<DoubleLShapeProfile> (DoubleLShapeProfile::CreateParams (GetModel(), "DoubleL 6x3 LLBB", 0.25, wideL->GetElementId(), DoubleLShapeProfileType::LLBB));
        InsertProfileGeometry<DoubleLShapeProfile> (DoubleLShapeProfile::CreateParams (GetModel(), "DoubleL 6x3 SLBB", 0.25, wideL->GetElementId(), DoubleLShapeProfileType::SLBB));
        }

        {
        LShapeProfilePtr deepL = InsertProfileGeometry<LShapeProfile> (LShapeProfile::CreateParams (GetModel(), "L 3x6 (for DoubleL)", 3.0, 6.0, 1.0, 0.5));
        InsertProfileGeometry<DoubleLShapeProfile> (DoubleLShapeProfile::CreateParams (GetModel(), "DoubleL 3x6 LLBB", 0.25, deepL->GetElementId(), DoubleLShapeProfileType::LLBB));
        InsertProfileGeometry<DoubleLShapeProfile> (DoubleLShapeProfile::CreateParams (GetModel(), "DoubleL 3x6 SLBB", 0.25, deepL->GetElementId(), DoubleLShapeProfileType::SLBB));
        InsertProfileGeometry<DoubleLShapeProfile> (DoubleLShapeProfile::CreateParams (GetModel(), "DoubleL 3x6 SLBB (0 spacing)", 0.0, deepL->GetElementId(), DoubleLShapeProfileType::SLBB));
        }

        {
        CShapeProfilePtr cShape = InsertProfileGeometry<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "C (for DoubleL)", 3.0, 6.0, 0.5, 0.5, 0.25), true);
        InsertProfileGeometry<DoubleCShapeProfile> (DoubleCShapeProfile::CreateParams (GetModel(), "DoubleC", 0.25, cShape->GetElementId()));
        InsertProfileGeometry<DoubleCShapeProfile> (DoubleCShapeProfile::CreateParams (GetModel(), "DoubleC (0 spacing)", 0.0, cShape->GetElementId()));
        }

        { // Arbitrary composite profile - I shape with plates on flanges
        IShapeProfilePtr iShapePtr = InsertElement<IShapeProfile> (IShapeProfile::CreateParams (GetModel(), "I shape for composite",
            6.0, 10.0, 0.5, 0.75, 0.5, 0.25, Angle::FromRadians (PI / 32)));
        RectangleProfilePtr platePtr = InsertElement<RectangleProfile> (RectangleProfile::CreateParams (GetModel(), "plate for composite", 5.0, 0.75));
        ArbitraryCompositeProfile::ComponentVector components =
            {
            ArbitraryCompositeProfileComponent (*iShapePtr, DPoint2d::From (0.0, 0.0)),
            ArbitraryCompositeProfileComponent (*platePtr, DPoint2d::From (0.0, 5.0 + 0.75 / 2.0)),
            ArbitraryCompositeProfileComponent (*platePtr, DPoint2d::From (0.0, -5.0 - 0.75 / 2.0))
            };
        InsertProfileGeometry<ArbitraryCompositeProfile> (ArbitraryCompositeProfile::CreateParams (GetModel(), "I shape beam with plates", components), true);
        }

        { // Arbitrary composite profile - 4 angle shape
        LShapeProfilePtr anglePtr = InsertElement<LShapeProfile> (LShapeProfile::CreateParams (GetModel(), "L shape for composite", 5.0, 5.0, 0.5, 0.2, 0.2));
        ArbitraryCompositeProfile::ComponentVector components =
            {
            ArbitraryCompositeProfileComponent (*anglePtr, DPoint2d::From (2.6, 2.6), Angle::FromRadians (0.0)),
            ArbitraryCompositeProfileComponent (*anglePtr, DPoint2d::From (-2.6, 2.6), Angle::FromRadians (PI / 2.0)),
            ArbitraryCompositeProfileComponent (*anglePtr, DPoint2d::From (-2.6, -2.6), Angle::FromRadians (PI)),
            ArbitraryCompositeProfileComponent (*anglePtr, DPoint2d::From (2.6, -2.6), Angle::FromRadians (-PI / 2.0))
            };
        InsertProfileGeometry<ArbitraryCompositeProfile> (ArbitraryCompositeProfile::CreateParams (GetModel(), "4 Angle shape", components));
        }

        { // Arbitrary shape profile - X shape
        bvector<DPoint3d> points =
            {
            DPoint3d::From (-4.0, -0.5), DPoint3d::From (-4.0, 0.5), DPoint3d::From (-0.5, 0.5), DPoint3d::From (-0.5, 4.0),
            DPoint3d::From (0.5, 4.0), DPoint3d::From (0.5, 0.5), DPoint3d::From (4.0, 0.5), DPoint3d::From (4.0, -0.5),
            DPoint3d::From (0.5, -0.5), DPoint3d::From (0.5, -4.0), DPoint3d::From (-0.5, -4.0), DPoint3d::From (-0.5, -0.5)
            };
        CurveVectorPtr curveVectorPtr = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString (points));
        IGeometryPtr geometryPtr = IGeometry::Create (curveVectorPtr);

        DMatrix4d rotation = DMatrix4d::From (RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0.0, 0.0, 1.0), PI / 4.0));
        Transform transform;
        transform.InitFrom (rotation);
        geometryPtr->TryTransformInPlace (transform);

        InsertProfileGeometry<ArbitraryShapeProfile> (ArbitraryShapeProfile::CreateParams (GetModel(), "Arbitrary shape - X shape", geometryPtr));
        }

        { // ARbitrary CenterrLine profile - Half pipe (Single curve primitive)
        DEllipse3d halfArc = DEllipse3d::FromPointsOnArc (DPoint3d::From (-2.0, 2.0), DPoint3d::From (0.0, 0.0), DPoint3d::From (2.0, 2.0));
        ICurvePrimitivePtr curvePtr = ICurvePrimitive::CreateArc (halfArc);

        ArbitraryCenterLineProfile::CreateParams params (GetModel(), "Arbitrary CenterLine - Half pipe", IGeometry::Create (curvePtr), 0.5);
        InsertCenterlineProfileGeometry<ArbitraryCenterLineProfile> (params);
        }

        { // Arbitrary CenterLine profile - S shape (Curve vector)
        DEllipse3d topArc = DEllipse3d::FromPointsOnArc (DPoint3d::From (1.5, 1.5), DPoint3d::From (-1.5, 1.5), DPoint3d::From (0.0, 0.0));
        ICurvePrimitivePtr topCurvePtr = ICurvePrimitive::CreateArc (topArc);

        DEllipse3d bottomArc = DEllipse3d::FromPointsOnArc (DPoint3d::From (0.0, 0.0), DPoint3d::From (1.5, -1.5), DPoint3d::From (-1.5, -1.5));
        ICurvePrimitivePtr bottomCurvePtr = ICurvePrimitive::CreateArc (bottomArc);

        CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        curves->Add (topCurvePtr);
        curves->Add (bottomCurvePtr);

        ArbitraryCenterLineProfile::CreateParams params (GetModel(), "Arbitrary CenterLine - S shape", IGeometry::Create (curves), 0.5);
        InsertCenterlineProfileGeometry<ArbitraryCenterLineProfile> (params);
        }

        { // Derived profile examples
        CShapeProfilePtr baseProfilePtr = InsertElement<CShapeProfile> (CShapeProfile::CreateParams (GetModel(), "C shape for derived profile", 6.0, 6.0, 0.75, 0.75));

        InsertProfileGeometry<DerivedProfile> (DerivedProfile::CreateParams (GetModel(), "Derived - offseted C (2.0 xAxis)", *baseProfilePtr,
            DPoint2d::From (2.0, 0.0), DPoint2d::From (1.0, 1.0), Angle::FromRadians (0.0)), true);

        InsertProfileGeometry<DerivedProfile> (DerivedProfile::CreateParams (GetModel(), "Derived - Uniformly scaled C (0.5)", *baseProfilePtr,
            DPoint2d::From (0.0, 0.0), DPoint2d::From (0.5, 0.5), Angle::FromRadians (0.0)));

        InsertProfileGeometry<DerivedProfile> (DerivedProfile::CreateParams (GetModel(), "Derived - Rotated C (90 deg)", *baseProfilePtr,
            DPoint2d::From (0.0, 0.0), DPoint2d::From (1.0, 1.0), Angle::FromRadians (PI / 2.0)));

        InsertProfileGeometry<DerivedProfile> (DerivedProfile::CreateParams (GetModel(), "Derived - mirrored C", *baseProfilePtr,
            DPoint2d::From (0.0, 0.0), DPoint2d::From (1.0, 1.0), Angle::FromRadians (0.0), true));
        }
    }
