/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/DomainTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesTestCase.h"
#include <DgnPlatform/GenericDomain.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct DomainTestCase : ProfilesTestCase
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTestCase, EnsureDomainsAreRegistered)
    {
    BentleyStatus registrationStatus = DgnDomains::RegisterDomain (ProfilesDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    ASSERT_TRUE (BentleyStatus::SUCCESS == registrationStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bssimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTestCase, EnsureProfilesDomainIsPresentInBim)
    {
    DgnDomainCP profilesDomain = GetDb().Domains().FindDomain (ProfilesDomain::GetDomain().GetDomainName());
    ASSERT_TRUE (NULL != profilesDomain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DomainTestCase, ValidateSchema)
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext (true, true);
    context->AddSchemaLocater (GetDb().GetSchemaLocater());

    ECN::SchemaKey refKey (PRF_SCHEMA_NAME, 1, 0);

    ECN::ECSchemaPtr refSchema = context->LocateSchema (refKey, ECN::SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE (refSchema.IsValid());

    ASSERT_TRUE (refSchema->Validate());
    }


static IGeometryPtr CreateCShape (CShapeProfileCPtr profile);


TEST_F(DomainTestCase, IshapeGraphics)
    {
    PhysicalElementPtr el = GenericPhysicalObject::Create(GetPhysicalModel(), GetCategoryId());
    el->SetUserLabel("Petras");

    //test geometry
    Placement3d placement;
    placement.GetOriginR() = DPoint3d::From(10.0, 10.0, -50.0);
    el->SetPlacement(placement);

    GeometryBuilderPtr builder = GeometryBuilder::Create(*el->ToGeometrySourceP());

    GeometrySourceP geomElem = el->ToGeometrySourceP();
    geomElem->SetCategoryId(el->GetCategoryId());

    builder->Append(el->GetCategoryId(), GeometryBuilder::CoordSystem::World);

    Dgn::Render::GeometryParams params;

    DgnCategoryId c = el->GetCategoryId();

    params.SetCategoryId(el->GetCategoryId());
    params.SetFillDisplay(Render::FillDisplay::Always);
    params.SetLineColor(ColorDef::Red());
    params.SetFillColor(ColorDef::Black());
    params.SetWeight(1);
    builder->Append(params, GeometryBuilder::CoordSystem::World);

    CShapeProfile::CreateParams profileParams (GetModel(), "C", 6, 10, 1, 1, 0.0, 0.5, PI / 18);
    auto profilePtr = CShapeProfile::Create (profileParams);

    IGeometryPtr blockGeom = CreateCShape (profilePtr);

    builder->Append(*blockGeom, GeometryBuilder::CoordSystem::World);
    builder->Finish(*el->ToGeometrySourceP());

    DgnDbStatus status;
    el->Insert(&status);
    ASSERT_TRUE(status == DgnDbStatus::Success);


    //create a view, it is neccessary if you like to see geoemetry with Gist
    DefinitionModelR dictionary = GetDb().GetDictionaryModel();
    CategorySelectorPtr categorySelector = new CategorySelector(dictionary, "Default");

    ModelSelectorPtr modelSelector = new ModelSelector(dictionary, "Default");
    modelSelector->AddModel(GetPhysicalModel().GetModelId());

    DisplayStyle3dPtr displayStyle = new DisplayStyle3d(dictionary, "Default");

    displayStyle->SetBackgroundColor(ColorDef::DarkYellow());
    displayStyle->SetSkyBoxEnabled(false);
    displayStyle->SetGroundPlaneEnabled(false);

    Render::ViewFlags viewFlags = displayStyle->GetViewFlags();
    viewFlags.SetRenderMode(Render::RenderMode::SolidFill);
    viewFlags.SetShowTransparency(true);
    viewFlags.ShowTransparency();

    displayStyle->SetViewFlags(viewFlags);

    //create view 
    OrthographicViewDefinition view(dictionary, "Structure View", *categorySelector, *displayStyle, *modelSelector);
    view.SetStandardViewRotation(StandardView::Iso); // Default to a rotated view
    view.LookAtVolume(GetDb().GeoLocation().GetProjectExtents());
    view.Insert();
    DgnViewId viewId = view.GetViewId();
    GetDb().SaveProperty(DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));
    }


//static IGeometryPtr CreateCShapeTest (CShapeProfileCPtr profile)
//    {
//    double const halfWidth = profile->GetFlangeWidth() / 2.0;
//    double const halfDepth = profile->GetDepth() / 2.0;
//    double const flangeThickness = profile->GetFlangeThickness();
//    double const flangeSlope = profile->GetFlangeSlope();
//
//    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
//
//    ICurvePrimitivePtr curvePtr (nullptr);
//
//    DPoint3d const topLeft     = { -halfWidth,  halfDepth, 0.0 };
//    DPoint3d const topRight    = {  halfWidth,  halfDepth, 0.0 };
//    DPoint3d const bottomRight = {  halfWidth, -halfDepth, 0.0 };
//    DPoint3d const bottomLeft  = { -halfWidth, -halfDepth, 0.0 };
//
//    double const slopeDepth = (10.0 / std::cos (profile->GetFlangeSlope())) * std::sin (profile->GetFlangeSlope());
//
//    auto line1 = ICurvePrimitive::CreateLine (topLeft - DPoint3d::From (0.0, -slopeDepth), topRight);
//    auto line2 = ICurvePrimitive::CreateLine (topRight, bottomRight);
//
//    double angle = (PI / 2.0 + profile->GetFlangeSlope()) / 2.0;
//    double c = flangeThickness / std::sin (angle);
//    double y = c * std::cos (angle);
//
//    DPoint3d center = topRight - DPoint3d::From (flangeThickness, y);
//    DVec3d ray = DVec3d::FromStartEnd (center, center - DPoint3d::From (-flangeThickness, 0.0));
//
//    double otherAngle = PI / 2 - angle;
//    ray.RotateXY (2.0 * otherAngle);
//    
//    DPoint3d top = DPoint3d::From (ray.x + center.x, ray.y + center.y);
//
//    DEllipse3d ellipse = DEllipse3d::FromArcCenterStartEnd (center,
//                                                            top,
//                                                            center - DPoint3d::From (-flangeThickness, 0.0));
//
//    ICurvePrimitivePtr curves[] =
//        {
//        line1,
//        ICurvePrimitive::CreateLine (topRight, center),
//        ICurvePrimitive::CreateLine (center, topRight),
//        ICurvePrimitive::CreateArc (ellipse),
//        line2,
//        ICurvePrimitive::CreateLine (bottomRight, bottomLeft),
//        ICurvePrimitive::CreateLine (bottomLeft, topLeft)
//        };
//
//    for (auto const& curve : curves)
//        curveVector->Add (curve);
//
//    return IGeometry::Create (curveVector);
//    }

static IGeometryPtr CreateCShape (CShapeProfileCPtr profile)
    {
    double const flangeWidth = profile->GetFlangeWidth();
    double const depth = profile->GetDepth();
    double const flangeThickness = profile->GetFlangeThickness();
    double const webThickness = profile->GetWebThickness();

    double const filletRadius = profile->GetFilletRadius();
    double const flangeEdgeRadius = profile->GetFlangeEdgeRadius();
    double const flangeSlope = profile->GetFlangeSlope();

    double const halfWidth = profile->GetFlangeWidth() / 2.0;
    double const halfDepth = profile->GetDepth() / 2.0;

    double const slopeHeight = profile->GetSlopeHeight();
    double const innerFlangeFaceLength = profile->GetInnerFlangeFaceLength();

    DPoint3d const topLeft     = { -halfWidth,  halfDepth, 0.0 };
    DPoint3d const topRight    = {  halfWidth,  halfDepth, 0.0 };
    DPoint3d const bottomRight = {  halfWidth, -halfDepth, 0.0 };
    DPoint3d const bottomLeft  = { -halfWidth, -halfDepth, 0.0 };

    DPoint3d const topFlangeEdge = topRight - DPoint3d { 0.0, flangeThickness };
    DPoint3d const topInnerCorner = topLeft - DPoint3d { -webThickness, flangeThickness + slopeHeight };
    DPoint3d const bottomInnerCorner = bottomLeft - DPoint3d { -webThickness, -flangeThickness - slopeHeight };
    DPoint3d const bottomFlangEdge = bottomRight - DPoint3d { 0.0, -flangeThickness };

    /*ICurvePrimitivePtr curves[] =
        {
        ICurvePrimitive::CreateLine (topLeft, topRight),
        ICurvePrimitive::CreateLine (topRight, topFlangeEdge),
        ICurvePrimitive::CreateLine (topFlangeEdge, topInnerCorner),
        ICurvePrimitive::CreateLine (topInnerCorner, bottomInnerCorner),
        ICurvePrimitive::CreateLine (bottomInnerCorner, bottomFlangEdge),
        ICurvePrimitive::CreateLine (bottomFlangEdge, bottomRight),
        ICurvePrimitive::CreateLine (bottomRight, bottomLeft),
        ICurvePrimitive::CreateLine (bottomLeft, topLeft)
        };*/
    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

    curveVector->Add (ICurvePrimitive::CreateLine (topLeft, topRight));

    if (profile->GetFlangeEdgeRadius() > 0.0)
        {

        double bisectorAngle = (PI / 2.0 + profile->GetFlangeSlope()) / 2.0;
        double bisectorLength = flangeEdgeRadius / std::sin (bisectorAngle);
        double y = bisectorLength * std::cos (bisectorAngle);

        DPoint3d center = topFlangeEdge - DPoint3d::From (flangeEdgeRadius, -y);
        DVec3d ray = DVec3d::FromStartEnd (center, center - DPoint3d::From (-flangeEdgeRadius, 0.0));

        double otherAngle = PI / 2 - bisectorAngle;
        ray.RotateXY (-2.0 * otherAngle);

        DPoint3d top = DPoint3d::From (center.x + ray.x, center.y + ray.y);

        DPoint3d ellipseStart = top;
        DPoint3d ellipseEnd = center - DPoint3d::From (-flangeEdgeRadius, 0.0);
        DEllipse3d ellipse = DEllipse3d::FromArcCenterStartEnd (center, ellipseStart, ellipseEnd);

        curveVector->Add (ICurvePrimitive::CreateLine (topRight, topFlangeEdge));
        curveVector->Add (ICurvePrimitive::CreateArc (ellipse));
        curveVector->Add (ICurvePrimitive::CreateLine (topFlangeEdge, topInnerCorner));
        }
    else
        {
        curveVector->Add (ICurvePrimitive::CreateLine (topRight, topFlangeEdge));
        curveVector->Add (ICurvePrimitive::CreateLine (topFlangeEdge, topInnerCorner));
        }

    curveVector->Add (ICurvePrimitive::CreateLine (topInnerCorner, bottomInnerCorner));
    curveVector->Add (ICurvePrimitive::CreateLine (bottomInnerCorner, bottomFlangEdge));
    curveVector->Add (ICurvePrimitive::CreateLine (bottomFlangEdge, bottomRight));
    curveVector->Add (ICurvePrimitive::CreateLine (bottomRight, bottomLeft));
    curveVector->Add (ICurvePrimitive::CreateLine (bottomLeft, topLeft));

    // for (auto const& curve : curves)
    //     curveVector->Add (curve);

    return IGeometry::Create (curveVector);
    }