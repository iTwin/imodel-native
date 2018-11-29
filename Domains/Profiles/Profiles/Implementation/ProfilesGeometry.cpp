/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ProfilesGeometry.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ProfilesDefinitions.h>
#include <Profiles\ProfilesGeometry.h>

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeomApi::CreateIShape(double overallWidth, double overallDepth, double flangeThickness, double webThickness)
    {
    double halfWidth = overallWidth / 2.0;
    double halfDepth = overallDepth / 2.0;

    DPoint3d ishapePts[] =
        {
        /*00*/ DPoint3d::From(-halfWidth, -halfDepth),
        /*01*/ DPoint3d::From(-halfWidth, -(halfDepth - flangeThickness)),
        /*02*/ DPoint3d::From(-(webThickness / 2.0), -(halfDepth - flangeThickness)),
        /*03*/ DPoint3d::From(-(webThickness / 2.0), halfDepth - flangeThickness),
        /*04*/ DPoint3d::From(-halfWidth, halfDepth - flangeThickness),
        /*05*/ DPoint3d::From(-halfWidth, halfDepth),
        /*06*/ DPoint3d::From(halfWidth, halfDepth),
        /*07*/ DPoint3d::From(halfWidth, halfDepth - flangeThickness),
        /*08*/ DPoint3d::From( webThickness / 2.0, halfDepth - flangeThickness),
        /*09*/ DPoint3d::From( webThickness / 2.0, -(halfDepth - flangeThickness)),
        /*10*/ DPoint3d::From(halfWidth, -(halfDepth - flangeThickness)),
        /*11*/ DPoint3d::From(halfWidth, -halfDepth),
        /*12*/ DPoint3d::From(-halfWidth, -halfDepth),
        };

    ICurvePrimitivePtr ishapeGeometry = ICurvePrimitive::CreateLineString(ishapePts, (sizeof(ishapePts) / sizeof((ishapePts)[0])));

    return IGeometry::Create(CurveVector::Create(ishapeGeometry, CurveVector::BOUNDARY_TYPE_Outer));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/

IGeometryPtr ProfilesGeomApi::CreateIShapeWithFillet(double overallWidth, double overallDepth, double flangeThickness, double webThickness, double filletRadius)
    {
    CurveVectorPtr ishapePath = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

    ishapePath->Add(ICurvePrimitive::CreateLine(DSegment3d::From(0, 0, 0, 100, 0, 0)));


    ICurvePrimitive::CreateLine(DSegment3d::From(-(overallWidth / 2.0), -(overallDepth / 2.0), 0.0,     
                                                 -(overallWidth / 2.0), -(overallDepth / 2.0 - flangeThickness), 0.0));

    ICurvePrimitive::CreateLine(DSegment3d::From(-(overallWidth / 2.0), -(overallDepth / 2.0 - flangeThickness), 0.0,
        -(webThickness / 2.0 + filletRadius), -(overallDepth / 2.0 - flangeThickness), 0.0));


    //ICurvePrimitive::CreateArc();

    return nullptr;
    }

END_BENTLEY_PROFILES_NAMESPACE