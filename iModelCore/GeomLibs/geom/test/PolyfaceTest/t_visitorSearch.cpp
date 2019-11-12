/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
#include <SampleGeometry.h>
#include <SampleGeometryCreator.h>
void checkStrokeSearch
(
PolyfaceHeaderPtr &mesh,
double x0, 
double y0,
double x1,
double y1,
size_t numExpected
)
    {

    }


// a write-once, read-many-time array of uv parameter values for use when searching for interior points
bvector<DPoint2d> s_classifiedRay_testParameters
    {
    DPoint2d::From (0.1,0.2),
    DPoint2d::From (0.2, 0.1),
    DPoint2d::From (0.3, 0.05),
    DPoint2d::From (0.123176789, 0.45234789237)
    };
// a smattering of directions for test probes.
bvector<DVec3d> s_searchRayCandidates
    {
    DVec3d::From (1.0, 0.162, 0.23423),
    DVec3d::From (1.0, 0.84262, 0.232423),
    DVec3d::From (1.0, 0.634262, 0.238423),
    DVec3d::From (1.0, 0.934262, 0.023423)
    };

bvector<DVec2d> s_unitSearchFlips
    {
    DVec2d::From (1,1),
    DVec2d::From (-1,1),
    DVec2d::From (-1,-1),
    DVec2d::From (1,-1),

    DVec2d::From (3,3),
    DVec2d::From (-3,3),
    DVec2d::From (-3,-3),
    DVec2d::From (3,-3)

    };
struct ClassifiedRay
{
DRay3d m_ray;
bvector<FacetLocationDetail> m_hits;
uint32_t m_numEdgeHit;
uint32_t m_numInteriorHit;
bvector<DSegment1d> m_intervals;
bvector<DSegment3d> m_segments;

bool m_intervalsValid;

bool HasValidIntervalSet () const {return m_intervalsValid;}
size_t NumInterval () const { return m_intervals.size ();}
ValidatedDouble IntervalFractionToValue (size_t intervalIndex, double fraction) const
    {
    if (intervalIndex < m_intervals.size ())
        return ValidatedDouble (m_intervals[intervalIndex].FractionToPoint (fraction), true);
    return ValidatedDouble (0.0, false);
    }

ValidatedDPoint3d IntervalFractionToPoint (size_t intervalIndex, double fraction) const
    {
    if (intervalIndex < m_intervals.size ())
        return ValidatedDPoint3d (m_segments[intervalIndex].FractionToPoint (fraction), true);
    return ValidatedDPoint3d ();
    }


ClassifiedRay (DRay3dCR ray) : m_ray (ray) {}
ClassifiedRay () {} // garbage in m_ray!!!
bool SetNewRay (DRay3dCR ray)
    {
    m_ray = ray;
    Clear ();
    return true;
    }
void Clear ()
    {
    m_hits.clear ();
    m_intervals.clear ();
    m_numEdgeHit = 0;
    m_numInteriorHit = 0;
    m_intervalsValid = false;
    }

void BuildIntervals (PolyfaceVisitorR visitor)
    {
    FacetLocationDetail detail;
    Clear ();
    for (visitor.Reset (); visitor.AdvanceToFacetBySearchRay (m_ray, detail);)
        {
        m_hits.push_back (detail);
        if (detail.isInteriorPoint)
            m_numInteriorHit++;
        else
            m_numEdgeHit++;
        }

    // all over with alternate implementation .  ..
    size_t numHitB = 0;
    DPoint3d facetXYZB;
    double rayFractionB;
    ptrdiff_t edgeIndexB;
    double edgeFractionB;
    DPoint3d edgeXYZB;
    double edgeDistanceB;
    double tolerance = 0.001;
    for (visitor.Reset (); visitor.AdvanceToFacetBySearchRay (m_ray,
                    tolerance,
                    facetXYZB, rayFractionB, edgeIndexB, edgeFractionB, edgeXYZB, edgeDistanceB
                    );)
        {
        numHitB++;
        }
    Check::Size (numHitB, m_hits.size ());


    // verify that there are an even number of edges hits with strictly alternating in/out normals
    // (no hits is valid)
    if (m_numInteriorHit > 1 && m_numEdgeHit == 0 && (m_hits.size () & 0x01) == 0)
        {
        FacetLocationDetail::SortA (m_hits);
        size_t errors = 0;
        for (size_t i = 0; i + 1 < m_hits.size (); i += 1)
            {
            double dotA = m_ray.direction.DotProduct (m_hits[i].normal);
            double dotB = m_ray.direction.DotProduct (m_hits[i+1].normal);
            if (dotA * dotB >= 0.0)
                errors++;
            }
        if (errors == 0)
            {
            m_intervalsValid = true;
            for (size_t i = 0; i + 1 < m_hits.size (); i += 2)
                {
                m_intervals.push_back (DSegment1d (m_hits[i].a, m_hits[i+1].a));
                m_segments.push_back (DSegment3d::From (m_hits[i].point, m_hits[i+1].point));
                }
            }
        }
    }

bool TryFindRayWithInteriorPoints (PolyfaceVisitorR visitor)
    {
    // loop through the facets.
    // test some parameters.
    // we really expect this to exit on the very first time -- later ones are highly unusual
    //   and probably have degenerate or duplicate facets.
    for (visitor.Reset (); visitor.AdvanceToNextFace ();)
        {
        for (auto &uv : s_classifiedRay_testParameters)
            {
            size_t numEdgesThisFace = (size_t)visitor.NumEdgesThisFace ();
            for (size_t i1 = 0; i1 + 1 < numEdgesThisFace; i1++)
                {
                auto ray = visitor.TryTriangleParamToPerpendicularRay (uv, i1);
                if (ray.IsValid ())
                    {
                    m_ray = ray;
                    BuildIntervals (visitor);
                    if (m_intervalsValid && m_intervals.size () > 0)
                        return true;
                    }
                }
            }
        }
    return false;
    }
// return true if xyz is inside or on the polyface.
// On return, the ray data contains the ray (and intervals) that provided the classification.
bool IsPointInOrOnPolyface (PolyfaceVisitorR visitor, DPoint3d xyz)
    {
    // try multiple directions, looking for one with no edge or vertex hits.
    for (auto &direction0 : s_searchRayCandidates)
        {
        for (auto &flip : s_unitSearchFlips)
            {
            DVec3d direction1 = DVec3d::From (direction0.x, direction0.y * flip.x, direction0.z * flip.y);
            direction1.Normalize ();
            m_ray = DRay3d::FromOriginAndVector (xyz, direction1);
            BuildIntervals (visitor);
            if (m_intervalsValid)
                {
                for (auto &interval : m_intervals)
                    {
                    if (interval.IsInteriorOrEnd (0.0))
                        return true;
                    }
                // fall out if no interval contains the start point.
                // (... The special case "no intervals found" also comes here)
                return false;
                }
            }
        }
    // lots of attempts all hit edges.   
    return false;
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,RayInclusion)
    {
    auto mesh = CreatePolyface_ExtrudedL (0,0,  10,5, 3,12, 7);
    auto  volume = mesh->ValidatedVolume ();
    if (Check::True (volume.IsValid (), "Valid volume ExtrudedL"))
        {
        Check::True (volume.Value () > 0.0, "Positive volume ExtrudedL");
        }
    auto visitor = PolyfaceVisitor::Attach (*mesh);
    ClassifiedRay rayData (DRay3d::FromOriginAndVector (DPoint3d::From (1,2,0), DVec3d::From (0,0,1)));
    rayData.BuildIntervals (*visitor);

    auto rayData1 = rayData;
    auto rayData2 = rayData;
    
    Check::True (rayData1.TryFindRayWithInteriorPoints (*visitor));
    if (rayData1.NumInterval () > 0)
        {
        auto insidePoint  = rayData1.IntervalFractionToPoint (0,  0.3);
        auto outsidePoint = rayData1.IntervalFractionToPoint (0, -0.1);
        if (Check::True (insidePoint.IsValid ()))
            Check::True (rayData2.IsPointInOrOnPolyface (*visitor, insidePoint.Value ()));
        if (Check::True (outsidePoint.IsValid ()))
            Check::False (rayData2.IsPointInOrOnPolyface (*visitor, outsidePoint.Value ()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceVisitor,StrokeSearch_scaledSheet)
    {
    int numEdge = 5;
    double scale = 0.5;
    // space mesh has edges with length (scale) ..
    auto mesh = Mesh_XYGrid (numEdge, numEdge, scale * (double)numEdge, scale * (double)numEdge, false);
    // local pick space points will be constructed fro edges with length 1.0 ...
    auto worldToLocal
        = DMatrix4d::FromRowValues
            (
            1.0 / scale, 0,0,0,
            0, 1.0 / scale, 0,0,
            0, 0,1,0,
            0,0,0,1
            );
    PolyfacePolygonPicker picker (*mesh, worldToLocal);


    // 
    bvector<size_t> strokePicks;
    bvector<PolyfacePolygonPicker::StrokePick> detailPicks;
    picker.AppendHitsByStroke (
            DPoint3d::From (.5, .5, 0),
            DPoint3d::From (1.5, .5, 0),
            &strokePicks,
            &detailPicks
            );
    picker.SetVisibilityBits (detailPicks);
    Check::Size (2, strokePicks.size ());
    picker.Reset ();
    strokePicks.clear ();
    detailPicks.clear ();


    double ay = 0.32;
    double by = 0.34;
    picker.AppendHitsByStroke (
            DPoint3d::From (-0.5, ay, 0),
            DPoint3d::From (1.5, by, 0),
            &strokePicks,
            &detailPicks
            );
    picker.SetVisibilityBits (detailPicks);


    Check::Size (2, strokePicks.size (), "forward stroke");
    picker.Reset ();
    strokePicks.clear ();
    detailPicks.clear ();

    picker.AppendHitsByStroke (
            DPoint3d::From (1.5, ay, 0),
            DPoint3d::From (-0.5, by, 0),
            &strokePicks,
            &detailPicks
            );
    picker.SetVisibilityBits (detailPicks);
    Check::Size (2, strokePicks.size (), "reverse stroke");


    bvector<size_t> allIn, allOut, edges;
    picker.Reset ();
    picker.AppendHitsByBox (
        DRange2d::From (DPoint2d::From (0.5, 0.5), DPoint2d::From (2.1, 3.1)),
        &allIn, &allOut, &edges);
    Check::Size (2, allIn.size ());
    Check::Size (13, allOut.size ());
    Check::Size (10, edges.size ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceVisitor,StrokeSearch_Closed)
    {
    DgnConeDetail coneDetail (DPoint3d::From (0,0,0), DPoint3d::From (0,10,0), 1, 0.9, true);
    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    builder->GetFacetOptionsR ().SetMaxPerFace (4);
    builder->GetFacetOptionsR ().SetAngleTolerance (atan (1.0));
    builder->AddSolidPrimitive (*ISolidPrimitive::CreateDgnCone (coneDetail));

    auto mesh = builder->GetClientMeshPtr ();

    DMatrix4d worldToLocal;
    worldToLocal.InitIdentity ();
    PolyfacePolygonPicker picker (*mesh, worldToLocal);


    // 
    bvector<size_t> strokePicks;
    bvector<PolyfacePolygonPicker::StrokePick> detailPicks;
    picker.AppendHitsByStroke (
            DPoint3d::From (15,5,0),
            DPoint3d::From (-15,5,0),
            &strokePicks,
            &detailPicks
            );
    picker.SetVisibilityBits (detailPicks);

    }


void AddQuad (PolyfaceHeaderR mesh, double x, double y, double z, DVec3dCR vectorA, DVec3dCR vectorB)
    {
    DPoint3d origin = DPoint3d::From (x,y,z);
    int index0 = 1 + (int)mesh.Point().size ();
    mesh.Point().push_back (origin);
    mesh.Point().push_back (origin + vectorA);
    mesh.Point().push_back (origin + vectorA + vectorB);
    mesh.Point().push_back (origin + vectorB);
    mesh.PointIndex ().push_back (index0);
    mesh.PointIndex ().push_back (index0 + 1);
    mesh.PointIndex ().push_back (index0 + 2);
    mesh.PointIndex ().push_back (index0 + 3);
    mesh.PointIndex ().push_back (0);
    }
bool CheckCounts ( bvector<PolyfacePolygonPicker::StrokePick> const &picks, size_t num00, size_t num10, size_t num01, size_t num11)
    {
    size_t n00 = 0;
    size_t n10 = 0;
    size_t n01 = 0;
    size_t n11 = 0;
    for (auto &pick : picks)
        {
        if (pick.isVisible)
            {
            if (pick.isHidden)
                n11++;
            else
                n10++;
            }
        else
            {
            if (pick.isHidden)
                n01++;
            else
                n00++;
            }
        }
    // (make sure all 4 get evaluated!!!)
    uint32_t numFail = 0;
    if (!Check::Size (num00, n00, "VisHidden 00 case count"))
        numFail++;
    if (!Check::Size (num10, n10, "VisHidden 10 case count"))
        numFail++;
    if (!Check::Size (num01, n01, "VisHidden 01 case count"))
        numFail++;
    if (!Check::Size (num11, n11, "VisHidden 11 case count"))
        numFail++;
    return numFail == 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceVisitor,StrokeSearch_PartiallyVisible)
    {
    auto mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    DVec3d x1 = DVec3d::From (1,0,0);
    DVec3d x3 = DVec3d::From (3,0,0);
    DVec3d x8 = DVec3d::From (8,0,0);

//         =====   =====   =====                   =====   =====   =====
// 0===1===2===3===4===5===6===7===8   9  10==11==12==13==14==15==16==17==18==19==20
//                                                 ============
    DVec3d y4 = DVec3d::From (0,4,0);
    DVec3d y5 = DVec3d::From (0,5,0);
    DVec3d y2 = DVec3d::From (0,2,0);
    double da = 10.0;
    double smallY = 0.1;
    for (double a = 0.0; a < 18.0; a+=10.0)
        {
        // Large quad with 3 narrow ones floating above.
        AddQuad (*mesh, a,0,0, x8, y5);
        AddQuad (*mesh, a+2,smallY, 1, x1, y4);
        AddQuad (*mesh, a+4,smallY, 1, x1, y4);
        AddQuad (*mesh, a+6,smallY, 1, x1, y4);
        }

    // and one fully hidden under the second cluster . . .
    AddQuad (*mesh, da + 2, 1.0, -1.0,    x3, y2);

    DMatrix4d worldToLocal;
    worldToLocal.InitIdentity ();
    PolyfacePolygonPicker picker (*mesh, worldToLocal);


    // Return all visibles . .
    bvector<size_t> visibleReadIndex;
    picker.SelectVisibleFacets (nullptr, visibleReadIndex);
    Check::Size (8, visibleReadIndex.size (), "all facet visible");
    picker.Reset ();


    // 
    bvector<size_t> strokePicks;
    bvector<PolyfacePolygonPicker::StrokePick> detailPicks;
    picker.AppendHitsByStroke (
            DPoint3d::From (-5,3,0),
            DPoint3d::From ( 9,2,0),
            &strokePicks,
            &detailPicks
            );
    picker.SetVisibilityBits (detailPicks);

    Check::Size (4, detailPicks.size (), "Stroke over first cluster");
    CheckCounts (detailPicks, 0, 3, 0, 1);
    picker.SelectVisibleFacets (&strokePicks, visibleReadIndex);
    Check::Size (4, visibleReadIndex.size (), "first cluster visible");




    picker.Reset ();
    detailPicks.clear ();
    strokePicks.clear ();
    picker.AppendHitsByStroke (
            DPoint3d::From ( 9,3,0),
            DPoint3d::From (21,2,0),
            &strokePicks,
            &detailPicks
            );
    picker.SetVisibilityBits (detailPicks);

    Check::Size (5, detailPicks.size (), "Stroke over second cluster");
    CheckCounts (detailPicks, 0, 3, 1, 1);
    picker.SelectVisibleFacets (&strokePicks, visibleReadIndex);
    Check::Size (4, visibleReadIndex.size (), "second cluster visible");

    // from underneath .. small pieces "above" are hidden
    DMatrix4d reverseView = DMatrix4d::FromScaleAndTranslation (DPoint3d::From (-1,-1,-1), DPoint3d::From (0,0,0));
    PolyfacePolygonPicker reversePicker (*mesh, reverseView);
    reversePicker.SelectVisibleFacets (nullptr, visibleReadIndex);
    Check::Size (3, visibleReadIndex.size (), "view from below");


    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceVisitor,SmallFacetA)
    {
    bvector<DPoint3d> points
        {
        DPoint3d::From (783708.52280560206, 191504.78227908278, 334.12211475864831),
        DPoint3d::From (783711.09831022169, 191506.52366021139, 334.18416969387607),
        DPoint3d::From (783708.52758241841, 191504.77489877801, 334.12151951666914)
        };

    auto ray = DRay3d::FromOriginAndVector (
                        DPoint3d::From (783752.23315180629, 191449.51577484963, 331.68015499682446),
                        DVec3d::From (0.00000000000000000, 0.00000000000000000, 6.4370673650085450)
                        );
    auto polyface = PolyfaceHeader::CreateVariableSizeIndexed ();
    polyface->AddPolygon (points);
    auto visitor = PolyfaceVisitor::Attach (*polyface);
    Check::SaveTransformed (*polyface);
    bvector<DSegment3d> segments;
    double a = 1000.0;
    segments.push_back (DSegment3d::From (ray.origin, ray.origin + 10.0 * a * ray.direction));
    DVec3d xVec, yVec, zVec;
    ray.direction.GetNormalizedTriad (xVec, yVec, zVec);
    DPoint3d facetPoint;
    double rayFraction;
    for (visitor->Reset ();
        visitor->AdvanceToFacetBySearchRay (ray, 0.01, facetPoint, rayFraction);
        )
        {
        segments.push_back (DSegment3d::From (facetPoint, ray.FractionParameterToPoint (rayFraction)));
        }

    Check::SaveTransformed (segments);
    Check::ShiftToLowerRight (10.0);

    Check::SaveTransformed (*polyface);
    segments.resize (1);    // go back to the segment
    for (visitor->Reset ();
        visitor->AdvanceToFacetBySearchRay (ray, 1000.0, facetPoint, rayFraction);
        )
        {
        segments.push_back (DSegment3d::From (facetPoint, ray.FractionParameterToPoint (rayFraction)));
        }
    Check::SaveTransformed (segments);


    Check::ClearGeometry ("PolyfaceVisitor.SmallFacetA");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceVisitor,EdgeSearch)
    {
    auto polyface = PolyfaceWithSinusoidalGrid (2,3, 0.1, 0.2, 0.1, 3, true);
    DMatrix4d matrix;
    matrix.InitIdentity ();
    Check::SaveTransformed (polyface);
    PolyfaceEdgeSearcher searcher (*polyface, matrix, 0.1, false);
    for (auto pickPoint : {DPoint3d::From (0.5,0.2,5),
                DPoint3d::From (0,0,3),
                DPoint3d::From (0.5, 0, 1),
                DPoint3d::From (0.5, 0.01, 1),
                DPoint3d::From (0.5, 0.08, 1),
                DPoint3d::From (0.5, 0.15, 1)
                })
        {
        searcher.Reset ();
        Check::SaveTransformedMarker (pickPoint);
        bvector<FacetLocationDetail> edgeHits;
        bvector<FacetLocationDetail> facetHits;
        searcher.AppendHits (pickPoint, &edgeHits, &facetHits);
        for (auto &eh : edgeHits)
            Check::SaveTransformed (DSegment3d::From (pickPoint, eh.point));
        for (auto &eh : facetHits)
            Check::SaveTransformed (DSegment3d::From (pickPoint, eh.point));
        }
    Check::ClearGeometry("PolyfaceVisitor.EdgeSearch");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceVisitor,StrokeSearch)
    {
    auto polyface = PolyfaceWithSinusoidalGrid (5,4, 0.1, 0.2, 0.1, 3, true);
    bvector<FacetLocationDetail> pickData;
    Check::SaveTransformed (polyface);
    DSegment3d segment = DSegment3d::From (-1,2,0, 10,3,0);
    Check::SaveTransformed (segment);
    polyface->PickFacetsByStroke (
        DPoint4d::From (4,5,10,0),
        segment.point[0],
        segment.point[1],
        pickData, false);

    for (auto &pick: pickData)
        {
        Check::SaveTransformedMarker (pick.point);
        }
    Check::ClearGeometry("PolyfaceVisitor.StrokeSearch");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceVisitor,ClosestApproachToLineString)
    {
    auto polyface = PolyfaceWithSinusoidalGrid (5,4, 0.1, 0.2, 0.1, 3, true);
    bvector<FacetLocationDetail> pickData;
    Check::SaveTransformed (polyface);
    bvector<DPoint3d> pointB {
            DPoint3d::From (1,0.5, 3),
            DPoint3d::From (2,0.5, 2),
            DPoint3d::From (5,3,5)
            };
    Check::SaveTransformed (pointB);
    DSegment3d segment;
    if (Check::True (PolyfaceQuery::SearchClosestApproachToLinestring (
        *polyface,
        pointB,
        segment)))
        {
        Check::SaveTransformed (segment);
        }

    Check::ClearGeometry("PolyfaceVisitor.ClosestApproachToLineString");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceVisitor,ClosestApproachMeshMesh)
    {
    auto polyfaceA = PolyfaceWithSinusoidalGrid (5,4, 2.1, 1.2, 0.1, 3, true);
    auto polyfaceB = PolyfaceWithSinusoidalGrid (8,6, 0.9, 0.8, 0.1, 3, true);

    Transform placementB = Transform::FromRowValues (
        1.0, 0.2, 0.2,  0.3,
        0.3, 1.0, 0,  -0.2,
        0, 0,   1,      2.0
        );
    polyfaceB->Transform (placementB);
    bvector<FacetLocationDetail> pickData;
    Check::SaveTransformed (polyfaceA);
    Check::SaveTransformed (polyfaceB);
    DSegment3d segment;
    if (Check::True (PolyfaceQuery::SearchClosestApproach (
        *polyfaceA, *polyfaceB,
        0, segment)))
        {
        Check::SaveTransformed (segment);
        DSegment3d segmentB;
        // verify same result from all pairs
        if (Check::True (PolyfaceQuery::SearchClosestApproach (
            *polyfaceA, *polyfaceB,
            0, segmentB,
            nullptr, nullptr)))
            {
            Check::Near (segment, segmentB);
            }

        }

    Check::ClearGeometry("PolyfaceVisitor.ClosestApproachMeshMesh");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceVisitor,ClosestApproachOneMesh)
    {
    auto polyfaceA = PolyfaceWithSinusoidalGrid (6,8, 2.1, 1.2, 0.1, 3, true);

    Check::SaveTransformed (polyfaceA);
    DSegment3d segment;
    if (Check::True (PolyfaceQuery::SearchClosestApproach (
        *polyfaceA,
        0, segment,
        0.0)))
        {
        Check::SaveTransformed (segment);
        }

    Check::ClearGeometry("PolyfaceVisitor.ClosestApproachOneMesh");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceVisitor,TryParamToDetail)
    {
    PolyfaceHeaderPtr polyfaceA = PolyfaceWithSinusoidalGrid (6,8, 2.1, 1.2, 0.1, 3, true, true);

    Check::SaveTransformed (polyfaceA);

    auto visitor = PolyfaceVisitor::Attach (*polyfaceA);
    size_t numHit = 0;
    size_t numBracket = 0;
    FacetLocationDetailPair horizontalBracket, verticalBracket;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        DPoint2d uv = DPoint2d::From (0.2, 0.01);
        FacetLocationDetail detailA;
        if (visitor->TryParamToFacetLocationDetail (uv, detailA))
            {
            numHit++;
            Check::SaveTransformed (
                    DSegment3d::From (
                        detailA.point,
                        detailA.point + DVec3d::From (0,0,3)));
            }
        DPoint2d uvBracket = DPoint2d::From (1.3, 2.4);
        if (visitor->TryParamToScanBrackets (uvBracket, &horizontalBracket, &verticalBracket))
            {
            numBracket++;
            Check::SaveTransformed (DSegment3d::From (
                        horizontalBracket.detailA.point, horizontalBracket.detailB.point));
            Check::SaveTransformed (DSegment3d::From (
                        verticalBracket.detailA.point, verticalBracket.detailB.point));
            }
        }
    Check::Size (1, numHit);
    Check::ClearGeometry("PolyfaceVisitor.TryParamToDetail");
    }
