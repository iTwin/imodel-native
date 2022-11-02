/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

static double s_noGapTol = 1.0e-10;
static double s_bigGapDirect = 100.0;
//static double s_smallGapDirect = 0.01;
static double s_gapAlong = 100.0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GapCleanup, Test1)
    {

    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (1,0,0, 99,0,0)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (100,1,0, 100,99,0)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (99,100,0, 1,100,0)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (0,99,0, 0,1,0)));
    CurveGapOptions options (s_noGapTol, s_bigGapDirect, s_gapAlong);
    CurveVectorPtr fixed = sticks->CloneWithGapsClosed (options);

    if (!Check::Near (0.0, fixed->MaxGapWithinPath (), "LineLine gap cleanup"))
        {
        Check::Print (*sticks, "sticks");
        Check::Print (*fixed, "fixed");
        }
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GapCleanup, Test2)
    {
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (1,0,0, 99,0,0)));
    sticks->push_back (
        ICurvePrimitive::CreateArc (DEllipse3d::FromPointsOnArc (
                        DPoint3d::From (100,1,0),
                        DPoint3d::From (110,50,0),
                        DPoint3d::From (100,99,0)
                        )));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (99,100,0, 1,100,0)));
    sticks->push_back (
        ICurvePrimitive::CreateArc (DEllipse3d::FromPointsOnArc (
                        DPoint3d::From (0,99,0),
                        DPoint3d::From (5,50,0),
                        DPoint3d::From (0,1,0)
                        )));
    CurveGapOptions options (s_noGapTol, s_bigGapDirect, s_gapAlong);
    CurveVectorPtr fixed = sticks->CloneWithGapsClosed (options);
    if (!Check::Near (0.0, fixed->MaxGapWithinPath (), "LineArc gap cleanup"))
        {
        Check::Print (*sticks, "sticks");
        Check::Print (*fixed, "fixed");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GapCleanup, Test3)
    {
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (100,0,0));
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (0,100,0));
    points.push_back (DPoint3d::From (100,100,0));
    
    sticks->push_back (ICurvePrimitive::CreateLineString (points));
    double d0 = 93.0;
    sticks->push_back (
        ICurvePrimitive::CreateArc (
            DEllipse3d::FromVectors (
                        DPoint3d::From (100,50,0),
                        DVec3d::From (49,0,0),
                        DVec3d::From (0,49,0),
                        Angle::DegreesToRadians (d0),
                        Angle::DegreesToRadians (-2.0 * d0)
                        )));

    CurveGapOptions options (s_noGapTol, s_bigGapDirect, s_gapAlong);
    CurveVectorPtr fixed = sticks->CloneWithGapsClosed (options);
    if (!Check::Near (0.0, fixed->MaxGapWithinPath (), "LineArc gap cleanup"))
        {
        Check::Print (*sticks, "sticks");
        Check::Print (*fixed, "fixed");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GapCleanup, Test4)
    {
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d pointA = DPoint3d::From (0,0,0);
    DPoint3d pointB = DPoint3d::From (99,0,0);
    DPoint3d pointC = DPoint3d::From (100,1,0);
    DPoint3d pointD = DPoint3d::From (100,100,0);
    
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointC)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointC, pointD)));
        
    sticks->at(1)->SetMarkerBit (ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve, true);
    CurveGapOptions options (s_noGapTol, s_bigGapDirect, s_gapAlong);
    CurveVectorPtr fixed = sticks->CloneWithGapsClosed (options);

    if (!Check::Near (0.0, fixed->MaxGapWithinPath (), "LineLine gap cleanup"))
        {
        Check::Print (*sticks, "sticks");
        Check::Print (*fixed, "fixed");
        }

    Check::Size (3, sticks->size (), "Size with gap");
    Check::Size (2, fixed->size (), "Size after extend");
    Check::True (fixed->Length () > sticks->Length () + 0.5, "Length with extension instead of gap");
    }


size_t CountBoundaryType (CurveVectorCR source, CurveVector::BoundaryType targetType)
    {
    size_t count = 0;
    for (size_t i = 0; i < source.size (); i++)
        {
        CurveVector::BoundaryType btype;
        if (source.GetChildBoundaryType (i, btype) && btype == targetType)
            count++;
        }
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Chaining, Test0)
    {
    static bool s_doPrint = false;
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d pointA = DPoint3d::From (0,0,0);
    DPoint3d pointB = DPoint3d::From (100,0,0);
    DPoint3d pointC = DPoint3d::From (100,100,0);
    
    DPoint3d pointD = DPoint3d::From (200,0,0);
    DPoint3d pointE = DPoint3d::From (300,0,0);
    
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointC)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointD, pointE)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointC, pointA)));

    CurveVectorPtr chains = sticks->AssembleChains ();
    Check::Size (2, chains->size (), "Triangle + Line = 2 chains");
    Check::Size (1, CountBoundaryType (*chains, CurveVector::BOUNDARY_TYPE_Outer), "Triangle+Line has one outer");
    Check::Size (1, CountBoundaryType (*chains, CurveVector::BOUNDARY_TYPE_Open), "Triangle+Line has one open");
    if (s_doPrint)
        Check::Print (chains, "Triangle + islandLine");
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointD)));
    chains = sticks->AssembleChains ();
    if (s_doPrint)
        Check::Print (chains, "Triangle + penninsulaLine");

    Check::Size (2, chains->size (), "Triangle + Line = 2 chains");
    Check::Size (2, CountBoundaryType (*chains, CurveVector::BOUNDARY_TYPE_Open), "Triangle+LineConnected has two chain");
    Check::Size (0, CountBoundaryType (*chains, CurveVector::BOUNDARY_TYPE_Outer), "Triangle+Line has one outer");
    }
