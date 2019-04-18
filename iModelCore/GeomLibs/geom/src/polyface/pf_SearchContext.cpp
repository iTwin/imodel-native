/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
DrapeSegment::DrapeSegment (size_t facetReadIndex, size_t edgeIndex)
    {
    m_facetReadIndex = facetReadIndex;
    m_drapeEdgeIndex = edgeIndex;
    m_drapeEdgeFraction[0]      =  DBL_MAX;
    m_drapeEdgeFraction[1]      = -DBL_MAX;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
DrapeSegment::DrapeSegment ()
    {
    m_facetReadIndex = SIZE_MAX;
    m_drapeEdgeIndex = SIZE_MAX;
    m_drapeEdgeFraction[0]      =  DBL_MAX;
    m_drapeEdgeFraction[1]      = -DBL_MAX;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
// sort comparison using m_drapeEdgeFraction[0] as sort coordinate . . .
bool DrapeSegment::operator< (DrapeSegment const &other) const
    {
    if (m_drapeEdgeIndex < other.m_drapeEdgeIndex)
        return true;
    if (m_drapeEdgeIndex > other.m_drapeEdgeIndex)
        return false;
    return m_drapeEdgeFraction[0] < other.m_drapeEdgeFraction[0];
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
bool DrapeSegment::IsEmptyInterval () const
    {
    return m_drapeEdgeFraction[1] < m_drapeEdgeFraction[0];
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
double DrapeSegment::FractionToDistance (double f) const
    {
    return DoubleOps::Interpolate (m_drapeDistance[0], f, m_drapeDistance[1]);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void DrapeSegment::SetPoint0 (double edgeFractionA, DPoint3dCR xyzA, FacetEdgeLocationDetail edgeDetail)
    {
    m_segment.point[0] = xyzA;
    m_drapeEdgeFraction[0] = edgeFractionA;
    m_edgeDetail[0] = edgeDetail;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void DrapeSegment::SetPoint1 (double edgeFractionA, DPoint3dCR xyzA, FacetEdgeLocationDetail edgeDetail)
    {
    m_segment.point[1] = xyzA;
    m_drapeEdgeFraction[1] = edgeFractionA;
    m_edgeDetail[1] = edgeDetail;
    }

/// Set m_drapeDistance by multiplying transform "y" row times segment points
void DrapeSegment::SetDrapeDistancesFromLocalY (TransformCR worldToLocal)
    {
    m_drapeDistance[0] = worldToLocal.MultiplyY (m_segment.point[0]);
    m_drapeDistance[1] = worldToLocal.MultiplyY (m_segment.point[1]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void DrapeSegment::ExpandInterval (double edgeFractionA, DPoint3dCR xyzA, FacetEdgeLocationDetail edgeDetail)
    {
    if (IsEmptyInterval ())
        {
        SetPoint0 (edgeFractionA, xyzA, edgeDetail);
        SetPoint1 (edgeFractionA, xyzA, edgeDetail);
        }
    else if (edgeFractionA < m_drapeEdgeFraction[0])
        {
        SetPoint0 (edgeFractionA, xyzA, edgeDetail);
        }
    else if (edgeFractionA > m_drapeEdgeFraction[1])
        {
        SetPoint1 (edgeFractionA, xyzA, edgeDetail);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
bool DrapeSegment::RestrictTo01 ()
    {
    if (IsEmptyInterval () || m_drapeEdgeFraction[1] <= 0.0 || m_drapeEdgeFraction[0] >= 1.0)
        return false;
    if (m_drapeEdgeFraction[0] < 0.0)
        {
        double u = -m_drapeEdgeFraction[0] / (m_drapeEdgeFraction[1] - m_drapeEdgeFraction[0]);
        DPoint3d xyz = DPoint3d::FromInterpolate (m_segment.point[0], u, m_segment.point[1]);
        SetPoint0 (0.0, xyz, m_edgeDetail[0]);
        }
    if (m_drapeEdgeFraction[1] > 1.0)
        {
        double u = (1.0-m_drapeEdgeFraction[0]) / (m_drapeEdgeFraction[1] - m_drapeEdgeFraction[0]);
        DPoint3d xyz = DPoint3d::FromInterpolate (m_segment.point[0], u, m_segment.point[1]);
        SetPoint1 (1.0, xyz, m_edgeDetail[0]);
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
ValidatedDrapeSegment DrapeSegment::Clip (double f0, double f1) const
    {
    double g0 = DoubleOps::Min (f0, f1);
    double g1 = DoubleOps::Max (f0, f1);
    DrapeSegment result = *this;
    bool valid = false;
    if (!IsEmptyInterval ())
        {
        double h0 = DoubleOps::Max (m_drapeEdgeFraction[0], g0);
        double h1 = DoubleOps::Min (m_drapeEdgeFraction[1], g1);
        if (h1 > h0)    // also ensures nonzero interval length !!!
            {
            double divDelta = 1.0 / (m_drapeEdgeFraction[1] - m_drapeEdgeFraction[0]);
            double u[2] = { (h0 - m_drapeEdgeFraction[0]) * divDelta,
                            (h1 - m_drapeEdgeFraction[0]) * divDelta };

            for (size_t i = 0; i < 2; i++)
                {
                result.m_drapeEdgeFraction[i] = DoubleOps::Interpolate (m_drapeEdgeFraction[0], u[i], m_drapeEdgeFraction[1] );
                result.m_drapeDistance[i] = DoubleOps::Interpolate     (m_drapeDistance[0],     u[i], m_drapeDistance[1] );
                result.m_segment.point[i] = DPoint3d::FromInterpolate  (m_segment.point[0],     u[i], m_segment.point[1] );
                }
            valid = true;
            }
        }
    return ValidatedDrapeSegment (result, valid);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
bool DrapeSegment::Extend (DrapeSegmentCR candidate)
    {
    if (m_facetReadIndex == candidate.m_facetReadIndex && m_drapeEdgeIndex == candidate.m_drapeEdgeIndex)
        {
        if (DoubleOps::AlmostEqualFraction (m_drapeEdgeFraction[1], candidate.m_drapeEdgeFraction[0]))
            {
            m_drapeEdgeFraction[1] = candidate.m_drapeEdgeFraction[1];
            m_segment.point[1] = candidate.m_segment.point[1];
            m_drapeDistance[1] = candidate.m_drapeDistance[1];
            return true;
            }
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
bool DrapeSegment::AlmostEqualXYZEndToStart (DrapeSegment const &candidate) const
    {
    return m_segment.point[1].AlmostEqual (candidate.m_segment.point[0]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void DrapeSegment::ExtendOrAppend (bvector<DrapeSegment> &data, DrapeSegmentCR candidate, bool compress)
    {
    if (!compress || data.empty () || !data.back ().Extend (candidate))
        data.push_back (candidate);
    }

double DrapeSegment::SignedSweptAreaSegmentToPlaneWithUnitNormal (DPlane3dCR plane) const
    {
    double dot0 = plane.normal.DotProduct (m_segment.point[0] - plane.origin);
    double dot1 = plane.normal.DotProduct (m_segment.point[1] - plane.origin);
    DPoint3d xyz0 = m_segment.point[0] - dot0 * plane.normal;
    DPoint3d xyz1 = m_segment.point[1] - dot1 * plane.normal;
    return 0.5 * (dot0 + dot1) * xyz0.Distance (xyz1);
    }

double DrapeSegment::SumSignedSweptAreaToPlaneWithUnitNormal (bvector<DrapeSegment> const &data, DPlane3dCR plane)
    {
    double sum = 0.0;
    for (DrapeSegment const &segment : data)
        sum += segment.SignedSweptAreaSegmentToPlaneWithUnitNormal (plane);
    return sum;
    }

void DrapeSegment::ExtractLinestrings (bvector<DrapeSegment> const &data, bvector<bvector<DPoint3d>> &chains)
    {
    chains.clear ();
    for (size_t i = 0; i < data.size (); i++)
        PolylineOps::AppendToChains (chains, data[i].m_segment);
    }



// allDrape will be sorted . . .
void DrapeSegment::SelectMinMaxVisibleDrape
(
bvector<DrapeSegment> &allDrape,
bvector<DrapeSegment> &minDrape,
bvector<DrapeSegment> &maxDrape,
bool compress
)
    {
    // ASSUME allDrape has forward-moving fractions . . .
    // DrapeSegment has a comparison operator . . .
    std::sort (allDrape.begin (), allDrape.end ());
    size_t numDrape = allDrape.size ();
    bvector<double> splitFractions;
    minDrape.clear ();
    maxDrape.clear ();
    for (size_t drapeIndex1 = 0, drapeIndex0 = 0; drapeIndex0 < numDrape; drapeIndex0 = drapeIndex1)
        {
        drapeIndex1 = drapeIndex0;
        size_t drapeEdgeIndex = allDrape[drapeIndex0].m_drapeEdgeIndex;
        // Collect all start and end fractions within the same original segment . .
        splitFractions.clear ();
        for (;drapeIndex1 < numDrape && allDrape[drapeIndex1].m_drapeEdgeIndex == drapeEdgeIndex; drapeIndex1++)
            {
            splitFractions.push_back (allDrape[drapeIndex1].m_drapeEdgeFraction[0]);
            splitFractions.push_back (allDrape[drapeIndex1].m_drapeEdgeFraction[1]);
            }
        if (drapeIndex0 + 1 == drapeIndex1)
            {
            minDrape.push_back (allDrape[drapeIndex0]);
            maxDrape.push_back (allDrape[drapeIndex0]);
            }
        else
            {
            std::sort (splitFractions.begin (), splitFractions.end ());

            // clip all the segments to the split intervals.
            // WARNING: We ignore the possiblity of overlapping facets
            for (size_t i = 0; i + 1 < splitFractions.size (); i++)
                {
                double f0 = splitFractions[i];
                double f1 = splitFractions[i+1];
                double testFraction = 0.5;
                if (f0 < f1)        // IGNORE -- what about almost-identical fractions?
                    {
                    ValidatedDrapeSegment minSegment, maxSegment;
                    double minDistance = 0.0, maxDistance = 0.0;
                    for (size_t drapeIndex = drapeIndex0; drapeIndex < drapeIndex1; drapeIndex++)
                        {
                        ValidatedDrapeSegment candidate = allDrape[drapeIndex].Clip (f0, f1);
                        if (candidate.IsValid ())
                            {
                            double candidateDistance = candidate.Value().FractionToDistance (testFraction);
                            if (!minSegment.IsValid ())
                                {
                                minSegment = candidate;
                                minDistance = candidateDistance;
                                }
                            else
                                {
                                if (candidateDistance < minDistance)
                                    {
                                    minSegment = candidate;
                                    minDistance = candidateDistance;
                                    }
                                }

                            if (!maxSegment.IsValid ())
                                {
                                maxSegment = candidate;
                                maxDistance = candidateDistance;
                                }
                            else
                                {
                                if (candidateDistance > maxDistance)
                                    {
                                    maxSegment = candidate;
                                    maxDistance = candidateDistance;
                                    }
                                }




                            }
                        }
                    if (minSegment.IsValid ())
                        DrapeSegment::ExtendOrAppend (minDrape, minSegment, compress);
                    if (maxSegment.IsValid ())
                        DrapeSegment::ExtendOrAppend (maxDrape, maxSegment, compress);
                    }
                }
            }
        }
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
static bool SweepLineSegmentToMeshXY
(
bvector<DrapeSegment> &segments,
DSegment3dCR segment,
PolyfaceIndexedHeapRangeTree & meshHeap,
PolyfaceQueryCR mesh,
PolyfaceVisitorR visitor
)
    {
    DRange3d segmentRange;
    segmentRange.Init ();
    segmentRange.Extend (segment.point[0]);
    segmentRange.Extend (segment.point[1]);
    segmentRange.low.z = -DBL_MAX;
    segmentRange.high.z = DBL_MAX;
    bvector<size_t>readIndices;
    meshHeap.CollectInRange (readIndices, segmentRange, 0.0);
    segments.clear ();
    visitor.SetNumWrap (0);
    visitor.Reset ();
    DVec3d segmentVector = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
    segmentVector.z = 0.0;
    double uu = segmentVector.DotProductXY (segmentVector);
    double divisor;
    DVec3d perpVector;
    if (    !perpVector.UnitPerpendicularXY (segmentVector)
        ||  !DoubleOps::SafeDivide (divisor, 1.0, uu, 0.0))
        return false;
    DVec3d scaledXYSegmentVector = DVec3d::FromScale (segmentVector, divisor);
    DPoint3d edgePoint;
    bvector<DPoint3d> &facetPoints = visitor.Point ();
    DPoint3d xyz0, xyz1;  // start and end of current facet edge
    for (size_t readIndex : readIndices)
        {
        if (visitor.MoveToFacetByReadIndex (readIndex))
            {
            DrapeSegment candidate (readIndex);
            size_t numFacetPoint = facetPoints.size ();
            size_t edgeReadIndex = readIndex + numFacetPoint - 1;
            xyz0 = facetPoints.back ();
            double h1, h0 = xyz0.DotDifference (segment.point[0], perpVector);
            size_t numEdgeHit = 0;
            for (size_t i = 0; i < numFacetPoint;
                  edgeReadIndex = readIndex + i,
                        i++, h0 = h1, xyz0 = xyz1)
                {
                xyz1 = facetPoints[i];
                h1 = xyz1.DotDifference (segment.point[0], perpVector);
                if (h1 != 0.0 && h0 * h1 <= 0.0)
                    {
                    double edgeFraction = -h0 / (h1 - h0);
                    edgePoint.SumOf (xyz0, 1.0 - edgeFraction, xyz1, edgeFraction);
                    double segmentFraction = edgePoint.DotDifference (segment.point[0], scaledXYSegmentVector);
                    candidate.ExpandInterval (segmentFraction, edgePoint, FacetEdgeLocationDetail (edgeReadIndex, edgeFraction));
                    numEdgeHit++;
                    }
                }
            if (numEdgeHit > 0 && candidate.RestrictTo01 ())
                segments.push_back (candidate);
            }
        }
    if (segments.size () > 0)
        {
        std::sort (segments.begin (), segments.end ());
        }
    return false;

    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
static bool AddSweepLineSegmentToMesh
(
bvector<DrapeSegment> &segments,
DSegment3dCR segment,
size_t segmentIndex,
DVec3d sweepVector,
PolyfaceVisitorR visitor
)
    {
    visitor.SetNumWrap (0);
    visitor.Reset ();
    DVec3d segmentVector = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
    segmentVector.z = 0.0;
    auto unitPerp = DVec3d::FromCrossProduct (segmentVector, sweepVector).ValidatedNormalize ();
    if (!unitPerp.IsValid ())
        return false;
    auto unitSweep = sweepVector.ValidatedNormalize ();
    Transform localToWorld = Transform::FromOriginAndVectors (segment.point[0], segmentVector, unitSweep, unitPerp);
    Transform worldToLocal;
    // worldToLocal produces (fractionAlongEdge, distanceInSweepDirection, altitudeAbovePlane)
    if (!worldToLocal.InverseOf (localToWorld))
        return false;

    DPoint3d edgePoint;
    bvector<DPoint3d> &facetPoints = visitor.Point ();
    for (visitor.Reset (); visitor.AdvanceToNextFace ();)
        {
        size_t readIndex = visitor.GetReadIndex ();
        DrapeSegment candidate (readIndex, segmentIndex);
        DPoint3d xyz0, xyz1;  // start and end of current facet edge
        size_t numFacetPoint = facetPoints.size ();
        size_t edgeReadIndex = readIndex + numFacetPoint - 1;
        xyz0 = facetPoints.back ();
        double h1, h0 = worldToLocal.MultiplyZ (xyz0);
        size_t numEdgeHit = 0;
        for (size_t i = 0; i < numFacetPoint;
                edgeReadIndex = readIndex + i,
                    i++, h0 = h1, xyz0 = xyz1)
            {
            xyz1 = facetPoints[i];
            h1 = worldToLocal.MultiplyZ (xyz1);
            if (h1 != 0.0 && h0 * h1 <= 0.0)
                {
                double edgeFraction = -h0 / (h1 - h0);
                edgePoint.SumOf (xyz0, 1.0 - edgeFraction, xyz1, edgeFraction);
                double segmentFraction = worldToLocal.MultiplyX (edgePoint);
                candidate.ExpandInterval (segmentFraction, edgePoint, FacetEdgeLocationDetail (edgeReadIndex, edgeFraction));
                numEdgeHit++;
                }
            }
        if (numEdgeHit > 0 && candidate.RestrictTo01 ())
            {
            candidate.SetDrapeDistancesFromLocalY (worldToLocal);
            segments.push_back (candidate);
            }
        }

    return true;

    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
static bool DoPointPickXY
(
bvector<DPoint3dSizeSize> &pickPoints,
DPoint3d spacePoint,
size_t spacePointTag,
PolyfaceIndexedHeapRangeTree & meshHeap,
PolyfaceQueryCR mesh,
PolyfaceVisitorR visitor
)
    {
    DRange3d pointRange;
    pointRange.Init ();
    pointRange.Extend (spacePoint);
    pointRange.low.z = -DBL_MAX;
    pointRange.high.z = DBL_MAX;
    bvector<size_t>readIndices;
    meshHeap.CollectInRange (readIndices, pointRange, 0.0);
    pickPoints.clear ();
    DRange3d heapRange;
    if (!meshHeap.TryGetRange (0, heapRange))
        return false;
    double tolerance = DoubleOps::SmallCoordinateRelTol () * heapRange.LargestCoordinateXY ();
    visitor.SetNumWrap (0);
    visitor.Reset ();
    DRay3d ray = DRay3d::FromOriginAndVector (spacePoint, DVec3d::From (0,0,-1));
    DPoint3d facetPoint;
    double rayFraction;

    for (size_t readIndex : readIndices)
        {
        if (visitor.MoveToFacetByReadIndex (readIndex))
            {
            if (visitor.TryFindFacetRayIntersection (ray, tolerance, facetPoint, rayFraction))
                {
                pickPoints.push_back (DPoint3dSizeSize (facetPoint, spacePointTag, readIndex));
                }
            }
        }
    return pickPoints.size () > 0;

    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void PolyfaceQuery::DrapeLinestring (bvector<DPoint3d> const &spacePoints, DVec3dCR direction, bvector<DrapeSegment> &drapeSegments) const
    {
    drapeSegments.clear ();
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this);
    for (size_t i = 0; i + 1 < spacePoints.size (); i++)
        {
        DSegment3d segment = DSegment3d::From (spacePoints[i], spacePoints[i+1]);
        AddSweepLineSegmentToMesh (drapeSegments, segment, i, direction, *visitor);
        }
    }

bool cb_compareLT_tagB_Z (DPoint3dSizeSize const &dataA, DPoint3dSizeSize const &dataB)
    {
    if (dataA.GetTagB () < dataB.GetTagB ())
        return true;
    if (dataA.GetTagB () > dataB.GetTagB ())
        return false;
    DPoint3d xyzA = dataA.GetCR ();
    DPoint3d xyzB = dataB.GetCR ();        
    if (xyzA.z < xyzB.z)
        return true;
    return false;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void PolyfaceQuery::DrapePointsXY (bvector<DPoint3d> const &spacePoints, bvector<DPoint3dSizeSize> &drapePoints) const
    {
    drapePoints.clear ();
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this);
    DRay3d zRay;
    DPoint3d facetPoint;
    double tolerance = 1.0e-8;
    double rayFraction;
    zRay.direction = DVec3d::From (0,0,-1);
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        DRange3d range = visitor->PointRange ();
        for (size_t i = 0; i < spacePoints.size (); i++)
            {
            if (range.IsContainedXY (spacePoints[i]))
                {
                zRay.origin = spacePoints[i];
                if (visitor->TryFindFacetRayIntersection (zRay, tolerance, facetPoint, rayFraction))
                    {
                    drapePoints.push_back (DPoint3dSizeSize (facetPoint, visitor->GetReadIndex (), i));
                    }
                }
            }
        }
    std::sort (drapePoints.begin (), drapePoints.end (), cb_compareLT_tagB_Z);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
PolyfaceSearchContext::PolyfaceSearchContext (PolyfaceHeaderPtr & mesh, bool sortX, bool sortY, bool sortZ)
    : m_mesh(mesh)
    {
    m_rangeTree = PolyfaceIndexedHeapRangeTree::CreateForPolyface (*mesh, sortX, sortY, sortZ);
    m_visitor = PolyfaceVisitor::Attach (*mesh, false);
    }


static void TransferDrapeSegmentsToImprint
(
bvector<DrapeSegment> &drapeSegments,
size_t tagA,
bvector<DSegment3dSizeSize> &imprintSegments
)
    {
    for (size_t i = 0; i < drapeSegments.size (); i++)
        {
        DSegment3dSizeSize imprint (drapeSegments[i].m_segment, i, drapeSegments[i].m_facetReadIndex);
        imprintSegments.push_back (imprint);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void PolyfaceSearchContext::DoDrapeXY_go
(
DSegment3dCR segment,
size_t tagA,
bvector<DrapeSegment> &workDrapeSegments,
bvector<DSegment3dSizeSize> &imprintSegments
)
    {
    SweepLineSegmentToMeshXY (workDrapeSegments, segment, *m_rangeTree, *m_mesh, *m_visitor);
    TransferDrapeSegmentsToImprint (workDrapeSegments, tagA, imprintSegments);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void PolyfaceSearchContext::DoDrapeXY (DSegment3dCR segment, bvector<DrapeSegment> &drapeSegments)
    {
    SweepLineSegmentToMeshXY (drapeSegments, segment, *m_rangeTree, *m_mesh, *m_visitor);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void PolyfaceSearchContext::DoDrapeXY (bvector<DPoint3d> const &linestring, bvector<DSegment3dSizeSize> &imprintSegments)
    {
    bvector<DrapeSegment> drapeSegments;
    DSegment3d segment;
    imprintSegments.clear ();
    for (size_t i = 0; i + 1 < linestring.size (); i++)
        {
        segment.Init (linestring[i], linestring[i+1]);
        DoDrapeXY_go (segment, i, drapeSegments, imprintSegments);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void PolyfaceSearchContext::DoDrapeXY (bvector<DSegment3dSizeSize> const &sourceSegments, bvector<DSegment3dSizeSize> &imprintSegments)
    {
    bvector<DrapeSegment> drapeSegments;
    imprintSegments.clear ();
    for (size_t i = 0; i < sourceSegments.size (); i++)
        {
        DoDrapeXY_go (sourceSegments[i].GetCR (), i, drapeSegments, imprintSegments);
        }
    }

void PolyfaceSearchContext::DoDrapeXY_recurse
(
CurveVectorCR curves,
size_t &segmentCounter,
bvector<DrapeSegment> &workDrapeSegments,
bvector<DSegment3dSizeSize> &imprintSegments
)
    {
    for (size_t i = 0; i < curves.size (); i++)
        {
        switch (curves[i]->GetCurvePrimitiveType ())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                bvector<DPoint3d> const * linestring = curves[i]->GetLineStringCP ();
                DSegment3d segment;
                for (size_t k = 0; k + 1 < linestring->size (); k++)
                    {
                    segment.Init (linestring->at(k), linestring->at(k+1));
                    DoDrapeXY_go (segment, segmentCounter++, workDrapeSegments, imprintSegments);
                    }
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                DSegment3d segment;
                curves[i]->TryGetLine (segment);
                DoDrapeXY_go (segment, segmentCounter++, workDrapeSegments, imprintSegments);
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                DoDrapeXY_recurse (*curves[i]->GetChildCurveVectorP (), segmentCounter, workDrapeSegments, imprintSegments);
                break;
                }
            }
        }
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void PolyfaceSearchContext::DoDrapeXY (CurveVectorCR curves, bvector<DSegment3dSizeSize> &imprintSegments)
    {
    bvector<DrapeSegment> workDrapeSegments;
    imprintSegments.clear ();
    size_t counter = 0;
    DoDrapeXY_recurse (curves, counter, workDrapeSegments, imprintSegments);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2015
+--------------------------------------------------------------------------------------*/
void PolyfaceSearchContext::SelectFacetsXY (DPoint3dCR spacePoint, size_t pointTag, bvector<DPoint3dSizeSize> &pickPoints)
    {
    DoPointPickXY (pickPoints, spacePoint, pointTag, *m_rangeTree, *m_mesh, *m_visitor);
    }


END_BENTLEY_GEOMETRY_NAMESPACE