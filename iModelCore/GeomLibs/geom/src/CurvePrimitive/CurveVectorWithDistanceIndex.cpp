/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

void CurveVectorWithDistanceIndex::Init (RotMatrix const &worldToView)
    {
    m_worldToViewRotMatrix = worldToView;
    m_viewToWorldRotMatrix.TransposeOf (worldToView);
    m_worldToView.InitFrom (worldToView);
    m_flattenToView.ScaleRows (m_worldToViewRotMatrix, 1,1,0);
    ClearCurves ();
    }

/*--------------------------------------------------------------------------------**//**
* (REMARK: Constructors are private)
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorWithDistanceIndex::CurveVectorWithDistanceIndex (RotMatrix worldToView)
    {
    Init (worldToView);
    }

CurveVectorWithDistanceIndex::CurveVectorWithDistanceIndex ()
    {
    Init (RotMatrix::FromIdentity ());
    }

/*--------------------------------------------------------------------------------**//**
* Public create methods
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorWithDistanceIndexPtr CurveVectorWithDistanceIndex::Create (RotMatrix worldToView)
    {
    return new CurveVectorWithDistanceIndex (worldToView);
    }

CurveVectorWithDistanceIndexPtr CurveVectorWithDistanceIndex::Create ()
    {
    return new CurveVectorWithDistanceIndex ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double PathLocationDetail::DistanceSquaredToPoint (DPoint3dCR xyz) const {return xyz.DistanceSquared (m_curveDetail.point);}
double PathLocationDetail::DistanceToPoint (DPoint3dCR xyz) const {return xyz.Distance (m_curveDetail.point);}
double PathLocationDetail::DistanceFromPathStart () const
    {
    return m_pathDistance;
    }

double PathLocationDetail::DistanceToPoint (PathLocationDetail const &other) const
    {
    return m_curveDetail.point.Distance (other.m_curveDetail.point);
    }

void CurveVectorWithDistanceIndex::ClearCurves ()
    {
    m_curveVector = nullptr;
    m_locations.clear ();
    if (m_savedPrimitives.IsValid ())
        m_savedPrimitives->clear ();
    }

CurveVectorCPtr CurveVectorWithDistanceIndex::GetCurveVector () const
    {
    return m_curveVector;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveVectorWithDistanceIndex::SetPath (CurveVectorPtr &path)
    {
    ClearCurves ();
    m_curveVector = path.get ();
    AppendPrimitives(*path);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveVectorWithDistanceIndex::SetPath (CurveVectorCR path)
    {
    ClearCurves ();
    m_curveVector = &path;
    AppendPrimitives(path);
    }


// Scale a vector to achieve a projected length perpendicular to a view vector.
static bool ApplyExtension
(
double targetLength,
DVec3dCR vector,
DVec3dCR perpVector,
double maxFactor,
DVec3dR  extendedVector
)
    {
    DVec3d perpPart, parallelPart;
    double fraction;
    vector.GetPerpendicularParts (perpVector, fraction, parallelPart, perpPart);
    double multiplier;
    if (DoubleOps::SafeDivide (multiplier, targetLength * vector.Magnitude (), perpPart.Magnitude (), maxFactor))
        {
        extendedVector.Scale (vector, multiplier);
        return true;
        }
    extendedVector.Scale (vector, maxFactor);
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVectorWithDistanceIndex::SetExtendedPath
(
CurveVectorPtr &path,
double extensionDistance,
PathLocationDetail &boundedStart,
PathLocationDetail &boundedEnd,
bool measureExtensionInView,
double maxExtensionFactor
)
    {
    DPoint3d xyzA, xyzB;
    DVec3d tangentA, tangentB;
    ClearCurves ();
    m_savedPrimitives = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    m_curveVector = nullptr;
    static double s_maxExtensionFactor = 100.0;
    static double s_minExtensionDistance = 0.001; // ??? no good number
    maxExtensionFactor = DoubleOps::Min (maxExtensionFactor, s_maxExtensionFactor);
    maxExtensionFactor = DoubleOps::Max (1.0, maxExtensionFactor);
    extensionDistance = DoubleOps::Max (extensionDistance, s_minExtensionDistance);
    if (path->GetStartEnd (xyzA, xyzB, tangentA, tangentB))
        {
        m_curveVector = path;   // but note that the extenders are not part of it -- indexing does not carry over !!!
        if (measureExtensionInView)
            {
            DVec3d zVector;
            m_viewToWorldRotMatrix.GetColumn (zVector,2);
            ApplyExtension (extensionDistance, tangentA, zVector, maxExtensionFactor, tangentA);
            ApplyExtension (extensionDistance, tangentB, zVector, maxExtensionFactor, tangentB);
            }
        else
            {
            tangentA.Scale (extensionDistance);
            tangentB.Scale (extensionDistance);
            }

        DPoint3d xyzAX = DPoint3d::FromSumOf (xyzA, tangentA, -1.0);
        DPoint3d xyzBX = DPoint3d::FromSumOf (xyzB, tangentB, 1.0);
        ICurvePrimitivePtr segmentA = ICurvePrimitive::CreateLine (DSegment3d::From (xyzAX, xyzA));
        ICurvePrimitivePtr segmentB = ICurvePrimitive::CreateLine (DSegment3d::From (xyzB, xyzBX));
        m_savedPrimitives->push_back (segmentA);
        m_savedPrimitives->push_back (segmentB);
        AppendPrimitive (segmentA);
        boundedStart = AtEnd ();
        AppendPrimitives (*path);
        boundedEnd = AtEnd ();
        AppendPrimitive (segmentB);
        }
    return m_locations.size () > 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double CurveVectorWithDistanceIndex::DistanceBetweenPointsXY (DPoint3dCR xyzA, DPoint3dCR xyzB) const
    {
    DPoint3d xyzA1, xyzB1;
    m_worldToView.MultiplyAndRenormalize (xyzA1, xyzA);
    m_worldToView.MultiplyAndRenormalize (xyzB1, xyzB);
    return xyzA1.DistanceXY (xyzB1);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double CurveVectorWithDistanceIndex::TotalPathLength () const
    {
    if (m_locations.size () == 0)
        return 0.0;
    return m_locations.back ().m_pathDistance;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double CurveVectorWithDistanceIndex::TotalPathLengthXY () const
    {
    if (m_locations.size () == 0)
        return 0.0;
    return m_locations.back ().m_projectedDistance;
    }


bool CurveVectorWithDistanceIndex::AppendPrimitive (ICurvePrimitivePtr const &primitive)
    {
    double a;
    if (primitive->Length (a))
        {
        if (m_locations.size () == 0)
            {
            CurveLocationDetail curveDetail0;
            primitive->FractionToPoint (0.0, curveDetail0);
            DRange3d range0 = DRange3d::From (curveDetail0.point);
            PathEntry entry (PathLocationDetail (curveDetail0, 0, 0.0), 0.0, range0);
            m_locations.push_back (entry);
            }

        double globalDistance = TotalPathLength () + a;
        double b;
        primitive->Length (&m_flattenToView, b);

        double projectedDistance = TotalPathLengthXY () + b;
        CurveLocationDetail curveDetail1;
        primitive->FractionToPoint (1.0, curveDetail1);
        curveDetail1.a = a;
        DRange3d range;
        primitive->GetRange (range);
        PathEntry indexEntry (PathLocationDetail (curveDetail1, m_locations.size (), globalDistance),
                        projectedDistance, range);
        m_locations.push_back (indexEntry);
        return true;
        }
    return false;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveVectorWithDistanceIndex::AppendPrimitives (CurveVectorCR source)
    {
    for (ICurvePrimitivePtr const &primitive : source)
        {
        CurveVectorCP child = primitive->GetChildCurveVectorCP ();
        if (nullptr != child)
            {
            AppendPrimitives (*child);
            }
        else
            {
            AppendPrimitive (primitive);
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PathLocationDetail::PathLocationDetail ()
    {
    m_pathIndex    = -1;
    m_pathDistance = 0.0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PathLocationDetail::PathLocationDetail (PathLocationDetail const &other)
    {
    m_curveDetail  = other.m_curveDetail;
    m_pathIndex    = other.m_pathIndex;
    m_pathDistance = other.m_pathDistance;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PathLocationDetail CurveVectorWithDistanceIndex::AtStart () const
    {
    PathLocationDetail detail;
    if (!m_locations.empty ())
        {
        detail = m_locations[0].FractionToPathLocationDetail (0.0);
        }
    return detail;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PathLocationDetail CurveVectorWithDistanceIndex::AtEnd () const
    {
    if (!m_locations.empty ())
        {
        return m_locations.back ();
        }
    PathLocationDetail detail;
    return detail;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t PathLocationDetail::PathIndex () const {return m_pathIndex;}
void PathLocationDetail::SetPathIndex (size_t index) {m_pathIndex = (int32_t)index;}
void PathLocationDetail::SetPathIndex (int32_t index) {m_pathIndex = index;}



double PathLocationDetail::CurveFraction () const {return m_curveDetail.fraction;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PathLocationDetail PathLocationDetail::FractionToPathLocationDetail (double f, bool evaluateDistance) const
    {
    PathLocationDetail detail = *this;
    if (m_curveDetail.curve != nullptr)
        {
        if (m_curveDetail.curve->FractionToPoint (f, detail.m_curveDetail))
            {
            if (evaluateDistance)
                {
                double internalDistance = 0.0;
                if (f != m_curveDetail.fraction)
                    {
                    m_curveDetail.curve->SignedDistanceBetweenFractions (
                          m_curveDetail.fraction, f, internalDistance);

                    }
                detail.m_curveDetail.a += internalDistance;
                detail.m_pathDistance += internalDistance;
                }
            else
                {
                detail.m_curveDetail.a = 0.0;
                detail.m_pathDistance = 0.0;
                }
            }
        }
    return detail;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVectorWithDistanceIndex::PathEntry::IsLessThan_ByPathDistance (CurveVectorWithDistanceIndex::PathEntry const &dataA, CurveVectorWithDistanceIndex::PathEntry const &dataB)
    {
    return dataA.m_pathDistance < dataB.m_pathDistance;
    }

bool PathLocationDetail::IsLessThan_ByPathDistance (PathLocationDetail const &dataA, PathLocationDetail const &dataB)
    {
    return dataA.m_pathDistance < dataB.m_pathDistance;
    }


bool CurveVectorWithDistanceIndex::PathEntry::IsLessThan_ByPathDistanceXY (CurveVectorWithDistanceIndex::PathEntry const &dataA, CurveVectorWithDistanceIndex::PathEntry const &dataB)
    {
    return dataA.m_projectedDistance < dataB.m_projectedDistance;
    }

double CurveVectorWithDistanceIndex::PathEntry::GetDistance (bool projected) const
    {
    if (projected)
        return m_projectedDistance;
    else
        return m_pathDistance;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVectorWithDistanceIndex::IsEmpty () const {return m_locations.empty ();}


PathLocationDetail CurveVectorWithDistanceIndex::SearchClosestPointBounded
(
DPoint3dCR spacePoint,
bool computeDistanceAlong
) const
    {
    if (IsEmpty ())
        return AtStart ();
    PathLocationDetail closestPointDetail = AtStart ();
    double d2Min = closestPointDetail.DistanceSquaredToPoint (spacePoint);
    CurveLocationDetail primitiveDetail;
    // nb. m_locations[0] is always "fraction=0" on same primitive as m_locations[1]
    for (size_t i = 1; i < m_locations.size (); i++)
        {
        double d2Range = m_locations[i].m_range.DistanceSquaredOutside (spacePoint);
        if (d2Range < d2Min && m_locations[i].m_curveDetail.curve->ClosestPointBounded (spacePoint, primitiveDetail))
            {
            double d2 = spacePoint.DistanceSquared (primitiveDetail.point);
            if (d2 < d2Min)
                {
                d2Min = d2;
                closestPointDetail = PathLocationDetail (primitiveDetail, i);
                }
            }
        }
    if (computeDistanceAlong)
        {
        size_t pathIndex = (size_t)closestPointDetail.m_pathIndex;
        if (pathIndex > 0)
            {
            double globalDistanceA =m_locations[pathIndex-1].m_pathDistance;
            double globalDistanceB =m_locations[pathIndex].m_pathDistance;
            double distanceAB = globalDistanceB - globalDistanceA;
            double signedDistanceFromB;
            m_locations[pathIndex].m_curveDetail.curve->SignedDistanceBetweenFractions (
                      m_locations[pathIndex].m_curveDetail.fraction,
                      closestPointDetail.m_curveDetail.fraction,
                      signedDistanceFromB);
            closestPointDetail.m_curveDetail.a = distanceAB + signedDistanceFromB;
            closestPointDetail.m_pathDistance = globalDistanceB + signedDistanceFromB;
            }
        }
    return closestPointDetail;
    }

ValidatedDRay3d CurveVectorWithDistanceIndex::DistanceAlongToPointAndUnitTangent (double distanceAlong, bool extrapolate) const
    {
    PathLocationDetail detail;
    bool inbounds = SearchByDistanceFromPathStart (distanceAlong, detail);
    DPoint3d xyz;
    DVec3d tangent;
    xyz = detail.PointAndUnitTangent (tangent);
    if (inbounds)
        return ValidatedDRay3d (DRay3d::FromOriginAndVector (xyz, tangent), true);
    if (!extrapolate)
        return ValidatedDRay3d (DRay3d::FromOriginAndVector (xyz, tangent), false);
    PathLocationDetail atStart = AtStart ();
    PathLocationDetail atEnd   = AtEnd ();
    double u1 = atEnd.DistanceFromPathStart ();
    double u0 = atStart.DistanceFromPathStart ();
    // if before start or after end, extrapolate the respective tangent
    if (distanceAlong < u0)
        {
        DPoint3d xyz0 = atStart.PointAndUnitTangent (tangent);
        xyz = xyz0 + (distanceAlong - u0) * tangent;
        }
    else if (distanceAlong > u1) // beyond the end -- extrapolate the tangent.
        {
        DPoint3d xyz1 = atEnd.PointAndUnitTangent (tangent);
        xyz = xyz1 + (distanceAlong - u1) * tangent;
        }
    return ValidatedDRay3d (DRay3d::FromOriginAndVector (xyz, tangent), true);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVectorWithDistanceIndex::SearchByDistanceFromPathStart
(
RotMatrixCP flatten,
double targetDistance,
PathLocationDetail &data
) const
    {
    if (m_locations.empty ())
        {
        data = AtStart ();
        return false;
        }
    double maxTargetDistance = flatten == nullptr ? TotalPathLength () : TotalPathLengthXY ();

    if (DoubleOps::AlmostEqual(targetDistance, 0.0))
        {
        data = AtStart();
        return true;
        }
    if (DoubleOps::AlmostEqual(targetDistance, maxTargetDistance))
        {
        data = AtEnd ();
        return true;
        }

    if (targetDistance <= 0.0)
        {
        data = AtStart ();
        return false;
        }
    else if (targetDistance >= maxTargetDistance)
        {
        data = AtEnd ();
        return false;
        }
    // Binary interior search ... The callback only looks at the pathDistance ....
    PathEntry key (targetDistance);
    // Use end ()-1 as upper bound -- hence the "too far" result is a valid list member.
    auto memberB = std::upper_bound (m_locations.begin (), m_locations.end () - 1,
            key,
            flatten == nullptr
                ? PathEntry::IsLessThan_ByPathDistance
                : PathEntry::IsLessThan_ByPathDistanceXY
            );
    if (memberB == m_locations.begin())
        {
        data = AtStart ();
        return true;
        }
    bool isFlattened = flatten != nullptr;
    //double d0 = (memberB-1)->GetDistance (isFlattened);
    double d1 = memberB->GetDistance (isFlattened);
    double signedDistanceToMove = targetDistance - d1;
    CurveLocationDetail curveDetail;
    if (memberB->m_curveDetail.curve->PointAtSignedDistanceFromFraction
                (
                flatten,
                memberB->m_curveDetail.fraction,
                signedDistanceToMove,
                false,
                curveDetail
                ))
        {
        if (nullptr == flatten)
            {
            curveDetail.a = memberB->m_curveDetail.a - signedDistanceToMove;
            data = PathLocationDetail (curveDetail, memberB->m_pathIndex, targetDistance);
            }
        else
            {
            // PointAtSignedDistanceFromFraction used projected distance.
            // Fill in the xyz distance
            double xyzDistance;
            memberB->m_curveDetail.curve->SignedDistanceBetweenFractions (
                        memberB->m_curveDetail.fraction, curveDetail.fraction, xyzDistance);
            curveDetail.a = memberB->m_curveDetail.a + xyzDistance;
            double d1xyz = memberB->GetDistance (false);
            data = PathLocationDetail (curveDetail, memberB->m_pathIndex, d1xyz + xyzDistance);
            }
        return true;
        }
    // PointAtSignedDistance failed.  This should not happen ... just take the memberB data
    data = *memberB;
    return false;
    }

ValidatedValue <PathLocationDetail> IndexedFractionToPathLocationDetail (size_t index, double fraction, bool computeDistance)
    {
    return ValidatedValue <PathLocationDetail> ();
    }

void CurveVectorWithDistanceIndex::CloneAndAppenedPartialCurve(CurveVectorPtr &dest, int32_t locationIndex, double fraction0, double fraction1, bool allowExtrapolation) const
    {
    if (locationIndex >= 0 && locationIndex < (int32_t)m_locations.size ()
        && m_locations[(size_t)locationIndex].m_curveDetail.curve != nullptr)
        {
        auto fragment = m_locations[(size_t)locationIndex].m_curveDetail.curve->CloneBetweenFractions (fraction0, fraction1, allowExtrapolation);
        double a;
        if (fragment.IsValid() && fragment->Length (a) && a > DoubleOps::SmallMetricDistance ())
            {
            if (!dest.IsValid ())
                dest = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
            dest->push_back (fragment);
            }
        }
    }
static PathLocationDetail Clamp (CurveVectorWithDistanceIndex const & path, PathLocationDetail const &source)
    {
    PathLocationDetail detail = source;
    if (PathLocationDetail::IsLessThan_ByPathDistance(detail, path.AtStart()))
        detail = path.AtStart ();
    if (PathLocationDetail::IsLessThan_ByPathDistance(path.AtEnd(), detail))
        detail = path.AtEnd ();
    return detail;
    }

CurveVectorPtr CurveVectorWithDistanceIndex::CloneBetweenDistances (double distance0, double distance1) const
    {
    PathLocationDetail location0, location1;
    if (   SearchByDistanceFromPathStart (distance0, location0)
        && SearchByDistanceFromPathStart (distance1, location1)
        )
        {
        return CloneBetweenPathLocations (location0, location1);
        }
    return nullptr;
    }

CurveVectorPtr CurveVectorWithDistanceIndex::CloneBetweenDistancesXY (double distance0, double distance1) const
    {
    PathLocationDetail location0, location1;
    if (   SearchByDistanceFromPathStartXY (distance0, location0)
        && SearchByDistanceFromPathStartXY (distance1, location1)
        )
        {
        return CloneBetweenPathLocations (location0, location1);
        }
    return nullptr;
    }


CurveVectorPtr CurveVectorWithDistanceIndex::CloneBetweenPathLocations (PathLocationDetail const &location0, PathLocationDetail const &location1) const
    {
    CurveVectorPtr dest;
    size_t n = m_locations.size ();
    if (n == 0)
        return nullptr;
    PathLocationDetail loc0 = Clamp (*this, location0);
    PathLocationDetail loc1 = Clamp (*this, location1);
    int32_t index0 = location0.m_pathIndex;
    int32_t index1 = location1.m_pathIndex;
    // Simplistically, output (location0.fraction to 1) from location0 curve, then a sequence of complete curves, then (0 to location1.fraction)
    // But consider ....
    //  1) reversed order
    //  2) same curve
    //  3) m_locations may have multiple entries pointing at subsets a single curve.
    if (index0 == index1)
        {
        CloneAndAppenedPartialCurve(dest, loc0.m_pathIndex, loc0.m_curveDetail.fraction, loc1.m_curveDetail.fraction, false);
        }
    else if (index1 > index0)
        {
        double fraction0 = DoubleOps::Min(loc0.m_curveDetail.fraction, 1.0);
        int32_t indexA = index0;
        int32_t indexB = (int32_t)n;
        if (loc1.m_pathIndex < indexB)
            indexB = loc1.m_pathIndex + 1;
        // INV indexA.indexA1 is (inclusively) a contiguous set of intervals with the same curve.
        // All needed fragments prior to indexA1 (with some fraction fussing) is output already.
        int32_t indexA1 = indexA;
        for (; indexA1 < indexB;)
            {
            // indexA1 + 1 is valid ...
            if (indexA1 + 1 < indexB &&  m_locations[indexA1 + 1].m_curveDetail.curve == m_locations[indexA].m_curveDetail.curve)
               {
               // extend the indexA..indexA1 interval
               indexA1++;
               }
            else
                {
                // accept indexA through indexA1, up to smaller of (a) top fraction in indexA1 or (b) loc1 fraction if it is in the same curve.
                // We need to output some or all of it and move forward ...
                if (loc1.m_curveDetail.curve != m_locations[indexA1].m_curveDetail.curve)
                    {
                    CloneAndAppenedPartialCurve (dest, indexA, fraction0, m_locations[indexA1].m_curveDetail.fraction, false);
                    indexA = indexA1 + 1;
                    indexA1 = indexA;
                    fraction0 = 0.0;    // the active curve fragments in m_locations always start at 0
                    }
                else
                    {
                    CloneAndAppenedPartialCurve (dest, indexA, fraction0, loc1.m_curveDetail.fraction, false);
                    break;
                    }
                }
            }
        }
    else
        {
        dest = CurveVectorWithDistanceIndex::CloneBetweenPathLocations (location1, location0);
        dest->CloneReversed ();
        }
    return dest;
    }

bool CurveVectorWithDistanceIndex::DistanceXYFromPathStart (PathLocationDetail const &pathDetail, double &distance) const
    {
    distance = 0.0;
    size_t index = pathDetail.m_pathIndex;
    if (index >= m_locations.size ())
        return false;
    if (m_locations[index].m_curveDetail.curve != pathDetail.m_curveDetail.curve)
        return false;
    // m_locations[i] is the end of the curve fragment . .
    double backupDistance;
    if (!pathDetail.m_curveDetail.curve->SignedDistanceBetweenFractions (
            &m_flattenToView,
            pathDetail.m_curveDetail.fraction,
            m_locations[index].m_curveDetail.fraction,
            backupDistance))
        return false;
    distance = m_locations[index].m_projectedDistance - backupDistance;
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVectorWithDistanceIndex::SearchByDistanceFromPathStart
(
double targetDistance,
PathLocationDetail &data
) const
    {
    return SearchByDistanceFromPathStart (nullptr, targetDistance, data);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVectorWithDistanceIndex::SearchByDistanceFromPathStartXY
(
double targetDistance,
PathLocationDetail &data
) const
    {
    return SearchByDistanceFromPathStart (&m_flattenToView, targetDistance, data);
    }


// Return true iff index has not yet reached (or gone beyond) limit, considering signed step.
static bool IsSteppingIntAlive (int index, int step, int limit)
    {
    if (step >= 0)
        return index < limit;
    else
        return index > limit;
    }

bool CurveVectorWithDistanceIndex::SearchFirstIntersectionWithCircleXY
(
PathLocationDetail const &startLocation,
double signedRadius,
PathLocationDetail &intersectionLocation
) const
    {
    intersectionLocation = startLocation;
    if (IsEmpty ())
        return false;

    int firstIndex = startLocation.m_pathIndex;
    int endIndex = -1;
    int step = 1;
    if (signedRadius > 0.0)
        {
        if (firstIndex < 0)
            firstIndex = 0;
        step = 1;
        endIndex = (int)m_locations.size ();
        }
    else if (signedRadius < 0.0)
        {
        if (firstIndex >= (int)m_locations.size ())
            firstIndex = (int)m_locations.size () - 1;
        endIndex = -1;
        step = -1;
        }
    else
        {
        return true;      // radius == 0 ==> no change.  Strange but sensible.
        }

    double radius = fabs (signedRadius);
    ICurvePrimitiveCP oldCurve = nullptr;
    DEllipse3d arc = DEllipse3d::FromScaledRotMatrix (startLocation.Point (), m_viewToWorldRotMatrix, radius, radius, 0.0, Angle::TwoPi ());
    ICurvePrimitivePtr cutter = ICurvePrimitive::CreateArc (arc);
    intersectionLocation = startLocation;
    CurveVectorPtr intersectionA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr intersectionB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    double bigFraction = 2.0;   // clearly outside the 0.1 interval, but small enough for decent calculations.

    // look for intersections with each curve.  The same curve maybe referenced from successive m_locations entries, so
    //  be sure to only use each once.
    for (int intIndex = firstIndex; IsSteppingIntAlive (intIndex, step, endIndex); intIndex += step)
        {
        size_t i = (size_t)intIndex;
        ICurvePrimitiveCP newCurve = m_locations[i].m_curveDetail.curve;

        if (newCurve != oldCurve)
            {
            CurveCurve::IntersectionsXY (*intersectionA, *intersectionB,
                  const_cast <ICurvePrimitiveP> (newCurve), cutter.get (), &m_worldToView, false);
            if (intersectionA->size () > 0)
                {
                DSegment1d fractionInterval = step > 0 ? DSegment1d (0.0, bigFraction) : DSegment1d (1.0, -bigFraction);
                if (newCurve == startLocation.m_curveDetail.curve)
                    fractionInterval.SetStart (startLocation.m_curveDetail.fraction);
                DSegment1d fractionInterval0 = fractionInterval;
                // we expect the intersections will come out in fraction order, but check them all for the closest.
                for (ICurvePrimitivePtr const &intersection : *intersectionA)
                    {
                    PartialCurveDetailCP partialCurve = intersection->GetPartialCurveDetailCP ();
                    CurveLocationDetail curveLocation;
                    if (nullptr != partialCurve && partialCurve->parentCurve.get () == newCurve)
                        {
                        double f0 = partialCurve->fraction0;
                        if (fractionInterval.IsStrictInterior (f0))
                            fractionInterval.SetStart (f0);
                        if (!partialCurve->IsSingleFraction ())
                            {
                            double f1 = partialCurve->fraction1;
                            if (fractionInterval.IsStrictInterior (f1))
                                fractionInterval.SetStart (f1);
                            }
                        }
                    if (!fractionInterval.IsEqual (fractionInterval0))
                        {
                        intersectionLocation = m_locations[i].FractionToPathLocationDetail (fractionInterval.GetStart (), true);
                        return true;
                        }
                    }
                }
            oldCurve = newCurve;
            }
        }
    return false;
    }

PathLocationDetail CurveVectorWithDistanceIndex::PathEntry::GetPathLocationDetail () const
    {
    return *(PathLocationDetail*)this;
    }

bool CurveVectorWithDistanceIndex::PathEntry::RangeIntersectsXY (CurveVectorWithDistanceIndex::PathEntry const &other) const
    {
    return m_range.IntersectsWith (other.m_range, 2);
    }

void CurveVectorWithDistanceIndex::GetBreakPoints (bvector<PathLocationDetail> &locations, bool replicateEndStart)
    {
    locations.clear ();
    for (PathEntry &location : m_locations)
        {
        PathLocationDetail detailB = location.GetPathLocationDetail ();
        if (locations.size () > 0)
            {
            PathLocationDetail detailA = locations.back ();
            if (!detailA.IsSameCurve (detailB))
                {
                // (We know!!!) detail0 is the end of its curve.   (We know !!!) detail1 is somewhere beyond its beginning (probably at its end)
                // Make a separate entry  at start the next curve.
                BeAssert (DoubleOps::AlmostEqualFraction (detailA.m_curveDetail.fraction, 1.0));
                if (!DoubleOps::AlmostEqualFraction (detailB.m_curveDetail.fraction, 0.0))
                    {
                    PathLocationDetail detailB0 = detailB.FractionToPathLocationDetail (0.0, false);
                    detailB0.m_pathDistance = detailA.m_pathDistance;
                    locations.push_back (detailB0);
                    }
                else
                    {
                    }
                }
            }
        locations.push_back (detailB);
        }
    }

void CurveVectorWithDistanceIndex::GetBreakPointsBetweenDistancesXY (double distanceA, double distanceB, bvector<PathLocationDetail> &locations) const
    {
    locations.clear ();
    // Note that these return false if distance was clamped to the end, but have valid AtStart () or AtEnd ().
    PathLocationDetail detailA, detailB;
    SearchByDistanceFromPathStartXY (distanceA, detailA);
    SearchByDistanceFromPathStartXY (distanceB, detailB);
    if (detailA.IsSameCurve (detailB))
        {
        locations.push_back (detailA);
        locations.push_back (detailB);
        return;
        }
    bool isReversed = distanceB < distanceA;
    if (isReversed)
        std::swap (detailB, detailA);
    size_t indexA = detailA.PathIndex ();
    size_t indexB = detailB.PathIndex ();
    locations.push_back (detailA);
    if (indexA < indexB)
        {
        if (!DoubleOps::AlmostEqualFraction (detailA.CurveFraction (), 1.0))
            locations.push_back (detailA.FractionToPathLocationDetail (1.0, false));

        for (size_t index = indexA + 1; index < indexB; index++)
            {
            PathLocationDetail location = m_locations[index].GetPathLocationDetail ();
            locations.push_back (location.FractionToPathLocationDetail (0.0));
            locations.push_back (location.FractionToPathLocationDetail (1.0));
            }

        if (!DoubleOps::AlmostEqualFraction (detailB.CurveFraction (), 0.0))
            locations.push_back (detailB.FractionToPathLocationDetail (0.0, false));
        }
    locations.push_back (detailA);
    if (isReversed)
        std::reverse (locations.begin (), locations.end ());
    }

// create pairs for the gap from the recorded interval to new details.
static bool InsertFillerPair (
bool doInsert,  //!< @param [in] if false, do all logic but skip the actual push_back
CurveVectorWithDistanceIndex    &pathA,             //!< @param [in] first path
CurveVectorWithDistanceIndex    &pathB,             //!< @param [in] second path
bvector<PathLocationDetailPair> &pathAIntervals,    //!< @param [out] intervals of pathA
bvector<PathLocationDetailPair> &pathBIntervals,     //!< @param [out] corresponding intervals of pathB
PathLocationDetail const &detailA1,       //!< detail at upper end of potentially empty interval of path A
PathLocationDetail const &detailB1,       //!< detail at lower end of potentially empty interval of path B
ptrdiff_t tag,
double tolerance
)
    {
    PathLocationDetail detailA0 = pathAIntervals.empty () ? pathA.AtStart () : pathAIntervals.back ().DetailB ();
    PathLocationDetail detailB0 = pathBIntervals.empty () ? pathB.AtStart () : pathBIntervals.back ().DetailB ();
    double distanceA = detailA1.DistanceFromPathStart () - detailA0.DistanceFromPathStart ();
    double distanceB = detailB1.DistanceFromPathStart () - detailB0.DistanceFromPathStart ();
    if (distanceA > tolerance || distanceB > tolerance)
        {
        if (doInsert)
            {
            pathAIntervals.push_back (PathLocationDetailPair (detailA0, detailA1, tag, tag));
            pathBIntervals.push_back (PathLocationDetailPair (detailB0, detailB1, tag, tag));
            }
        return true;
        }
    return false;
    }

// Retract intervalA so it is only the common part with B.
static bool RetractInterval (DSegment1dCR intervalA, DSegment1dCR intervalB,
    DSegment1dR outA, DSegment1dR outB)
    {
    ValidatedDSegment1d validatedBonA;
    if (intervalA.GetStart() <= intervalA.GetEnd ())
        validatedBonA = DSegment1d(0, 1).DirectedOverlap(intervalA);
    else
        {
        validatedBonA = DSegment1d(0, 1).DirectedOverlap(intervalA.Reverse ());
        validatedBonA.Value().ReverseInPlace();
        }

    if (validatedBonA.IsValid())
        {
        DSegment1d intervalBonA = validatedBonA;
        DSegment1d intervalAonB(
            intervalB.FractionToPoint(intervalBonA.GetStart()),
            intervalB.FractionToPoint(intervalBonA.GetEnd())
            );
        if (intervalBonA.Delta() < 0.0)
            {
            intervalBonA.ReverseInPlace();
            intervalAonB.ReverseInPlace();
            }
        outA = intervalAonB;
        outB = intervalBonA;
        return outA.Length () >= DoubleOps::SmallCoordinateRelTol();
        }
    return false;
    }

static bool AlmostEqualProjection (DSegment3dCR segment, DPoint3dCR spacePoint, DPoint3dR projection, double &fraction)
    {
    return segment.ProjectPoint (projection, fraction, spacePoint)
        && spacePoint.AlmostEqual (projection);
    }

// Test if this is at the end of interval in A and beginning in B ....
bool IsEndToStart(double fractionA, double fractionB)
    {
    static double s_almostZero = 1.0e-8;
    return (fractionA > 1.0 - s_almostZero) && fractionB < s_almostZero;
    }
// Setup point and fraction for arcA in detailA, and (bounded) closest point on arcB in detailB.
static bool SetupDetailPair (DEllipse3dCR arcA, double fractionA, DEllipse3dCR arcB, CurveLocationDetailR detailA, CurveLocationDetailR detailB)
    {
    detailA.fraction = fractionA;
    detailA.point = arcA.FractionToPoint (fractionA);
    double radians, dSquared;
    arcB.ClosestPointBounded (radians, dSquared, detailB.point, detailA.point);
    detailB.fraction = arcB.AngleToFraction (radians);
    if (!detailA.point.AlmostEqual(detailB.point))
        return false;
    if (IsEndToStart(detailA.fraction, detailB.fraction))
        return false;
    if (IsEndToStart(detailB.fraction, detailA.fraction))
        return false;
    return true;
    }

static void CollectCloseEndPointPairs(bvector<CurveLocationDetailPair> &pairs, DEllipse3dCR arcA, DEllipse3dCR arcB)
    {
    CurveLocationDetailPair pair;
    if (SetupDetailPair(arcA, 0.0, arcB, pair.detailA, pair.detailB))
        pairs.push_back (pair);
    if (SetupDetailPair(arcA, 1.0, arcB, pair.detailA, pair.detailB))
        pairs.push_back(pair);
    if (SetupDetailPair(arcB, 0.0, arcA, pair.detailB, pair.detailA))
        pairs.push_back(pair);
    if (SetupDetailPair(arcB, 1.0, arcA, pair.detailB, pair.detailA))
        pairs.push_back(pair);
    }
bool cb_detailAFractionLT(CurveLocationDetailPair const &pair0, CurveLocationDetailPair const &pair1)
    {
    return pair0.detailA.fraction < pair1.detailA.fraction;
    }
// look for overlap interval.
// ASSUME dissimilar curves have no overlap (don't worry about line overlapping order 2 bspline!!!)
// ASSUME only one interval per curve (no arc segments that wrap strangely.)
static bool ComputeSimpleOvelap (ICurvePrimitiveCR curveA, ICurvePrimitiveCR curveB, DSegment1dR intervalA, DSegment1dR intervalB, double abstol)
    {

    DSegment3d segmentA, segmentB;
    if (curveA.TryGetLine (segmentA) && curveB.TryGetLine (segmentB))
        {
        DPoint3d projA0, projA1;  // projections of A on B
        DPoint3d projB0, projB1;  // projections of B on A
        double fA0, fA1, fB0, fB1;
        if (   AlmostEqualProjection (segmentA, segmentB.point[0], projB0, fB0)
            && AlmostEqualProjection (segmentA, segmentB.point[1], projB1, fB1)
            && AlmostEqualProjection (segmentB, segmentA.point[0], projA0, fA0)
            && AlmostEqualProjection (segmentB, segmentA.point[1], projA1, fA1)
            )
            {
            auto fullA = DSegment1d(fA0, fA1);
            auto fullB = DSegment1d(fB0, fB1);
            return RetractInterval (fullA, fullB, intervalA, intervalB);
            }
        return false;
        }
    auto placementA = curveA.GetSpiralPlacementCP ();
    auto placementB = curveB.GetSpiralPlacementCP();
    if (nullptr != placementA && nullptr != placementB)
        {
        if (placementA->AlmostEqualByActiveIntervalPoints (*placementB, abstol))
        // if (placementA->AlmostEqual01 (*placementB, abstol))
            {
            intervalA = DSegment1d(0.0, 1.0);
            intervalB = DSegment1d(0.0, 1.0);
            return true;
/*
            auto parentIntervalA = placementA->FractionInterval();
            auto parentIntervalB = placementB->FractionInterval();
            if (DSegment1d::NonZeroFractionalDirectedOverlap (parentIntervalA, parentIntervalB,
                        intervalA, intervalB))
                {
                return intervalA.Length() >= DoubleOps::SmallCoordinateRelTol();
                }
*/
            }
        }
    DEllipse3d arcA, arcB;
    if (curveA.TryGetArc (arcA) && curveB.TryGetArc (arcB))
        {
        // simple equivalence as stroked?
        if (arcA.IsAlmostEqual(arcB, abstol))
            {
            intervalA = DSegment1d(0.0, 1.0);
            intervalB = DSegment1d(0.0, 1.0);
            return true;
            }
        else if (arcA.IsAlmostEqualFullEllipsePointSet(arcB, abstol))
            {
            bvector<CurveLocationDetailPair> pairs;
            CollectCloseEndPointPairs(pairs, arcA, arcB);
            if (pairs.size () >= 2)
                {
                auto pair0 = *std::min_element (pairs.begin (), pairs.end (), cb_detailAFractionLT);
                auto pair1 = *std::max_element(pairs.begin (), pairs.end (), cb_detailAFractionLT);
                intervalA = DSegment1d (pair0.detailA.fraction, pair1.detailA.fraction);
                intervalB = DSegment1d(pair0.detailB.fraction, pair1.detailB.fraction);
                return true;
                }
            }
        }
    return false;
    }

void CurveVectorWithDistanceIndex::FindCommonSubPaths
(
CurveVectorWithDistanceIndex    &pathA,             //!< @param [in] first path
CurveVectorWithDistanceIndex    &pathB,             //!< @param [in] second path
bvector<PathLocationDetailPair> &pathAIntervals,    //!< @param [out] intervals of pathA
bvector<PathLocationDetailPair> &pathBIntervals,     //!< @param [out] corresponding intervals of pathB
bool                            includeGaps,         //!< @param [in] true to create gap intervals between common parts
bool                            compressIntervals    //!< @param [in] true to return only start end of common paths -- no internal break locations (e.g. between curves)
)
    {
    pathAIntervals.clear ();
    pathBIntervals.clear ();
    double tolerance = Angle::MediumAngle () * DoubleOps::Max (pathA.m_curveVector->FastMaxAbs (), pathB.m_curveVector->FastMaxAbs ());
    // ASSUME -- m_locations is <C0@0, C0@1, C1@1 ..> i.e. zero entry then one per curve.
    size_t indexB0 = 1;
    ICurvePrimitivePtr parentA, parentB;
    for (size_t indexA = 1; indexA < pathA.m_locations.size (); indexA++)
        {
        ICurvePrimitiveP curveA = const_cast <ICurvePrimitiveP> (pathA.m_locations[indexA].m_curveDetail.curve);
        for (size_t indexB = indexB0; indexB < pathB.m_locations.size (); indexB++)
            {
            if (pathA.m_locations[indexA].RangeIntersectsXY (pathB.m_locations[indexB]))
                {
                ICurvePrimitiveP curveB = const_cast <ICurvePrimitiveP> (pathB.m_locations[indexB].m_curveDetail.curve);
                DSegment1d intervalA, intervalB;
                bool atEndOfA = false;
                if (ComputeSimpleOvelap (*curveA, *curveB, intervalA, intervalB, tolerance))
                    {
                    if (DoubleOps::AlmostEqual (intervalA.GetEnd (), 1.0))
                        intervalA.SetEnd (1.0);
                    if (DoubleOps::AlmostEqual (intervalB.GetEnd (), 1.0))
                        intervalB.SetEnd (1.0);
                    if (DoubleOps::AlmostEqual (intervalA.GetEnd (), 1.0))
                        atEndOfA = true;
                    if (   !DoubleOps::AlmostEqual (intervalA.GetStart(), intervalA.GetEnd())
                        && !DoubleOps::AlmostEqual (intervalB.GetStart(), intervalB.GetEnd())
                        )
                        {
                        PathLocationDetail detailA0 = pathA.m_locations[indexA].FractionToPathLocationDetail (intervalA.GetStart (), true);
                        PathLocationDetail detailA1 = pathA.m_locations[indexA].FractionToPathLocationDetail (intervalA.GetEnd (), true);
                        PathLocationDetail detailB0 = pathB.m_locations[indexB].FractionToPathLocationDetail (intervalB.GetStart (), true);
                        PathLocationDetail detailB1 = pathB.m_locations[indexB].FractionToPathLocationDetail (intervalB.GetEnd (), true);
                        if (!InsertFillerPair (includeGaps,
                                pathA, pathB,
                                pathAIntervals, pathBIntervals,
                                detailA0, detailB0,
                                0, tolerance)
                            && compressIntervals
                            && !pathAIntervals.empty ()
                            // Don't need to check (or change) tagA -- "no insert" means there is something back there with the right tag???
                            )
                            {
                            pathAIntervals.back ().SetDetailB (detailA1);
                            pathBIntervals.back ().SetDetailB (detailB1);
                            }
                        else
                            {
                            pathAIntervals.push_back (PathLocationDetailPair (detailA0, detailA1, 1,1));
                            pathBIntervals.push_back (PathLocationDetailPair (detailB0, detailB1, 1,1));
                            }
                        }
                    indexB0 = indexB;   // !!! no later A can remerge earlier in B.
                    if (DoubleOps::AlmostEqual (intervalB.GetEnd (), 1.0))
                        indexB0++;
                    if (atEndOfA)  // Nothing of this curve in A is left for intersection with other B curves.
                        break;
                    }
                }
            }
        }
    InsertFillerPair (includeGaps, pathA, pathB, pathAIntervals, pathBIntervals, pathA.AtEnd (), pathB.AtEnd (), 0, tolerance);
    }


//! Compute strokes along the curves.
void CurveVectorWithDistanceIndex::Stroke
(
bvector<PathLocationDetail> &strokeDetail, //!< [out] stroked locations
IFacetOptionsCR options                 //!< [in] density controls
) const
    {
    strokeDetail.clear ();
    bvector<PathLocationDetail> pointsOnPrimitive;
    for (size_t i = 0; i + 1 < m_locations.size (); i++)
        {
        pointsOnPrimitive.clear ();
        CurveLocationDetail detailA = m_locations[i].m_curveDetail;
        CurveLocationDetail detailB = m_locations[i+1].m_curveDetail;
        double fractionA = detailA.curve == detailB.curve ? detailA.fraction : 0.0;
        double fractionB = m_locations[i+1].m_curveDetail.fraction;
        size_t num0 = strokeDetail.size();
        m_locations[i+1].m_curveDetail.curve->AddStrokes (strokeDetail, options, fractionA, fractionB);
        for (size_t k = num0; k < strokeDetail.size (); k++)
            strokeDetail[k].SetPathIndex (i);
        }
    }

/*--------------------------------------------------------------------------------**//**
//! Evaluate the curve at specified xValue, assuming that the distance index is really x index.
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVectorWithDistanceIndex::SearchByElevationMapXPlane
(
double targetDistance,
PathLocationDetail &data
) const
    {
    if (m_locations.empty ())
        {
        data = AtStart ();
        return false;
        }
    double distanceA = m_locations.front ().GetDistance (true);
    double distanceB = m_locations.back ().GetDistance (true);

    if (DoubleOps::AlmostEqual(targetDistance, distanceA))
        {
        data = AtStart();
        return true;
        }
    if (DoubleOps::AlmostEqual(targetDistance, distanceB))
        {
        data = AtEnd ();
        return true;
        }

    if (targetDistance <= distanceA)
        {
        data = AtStart ();
        return false;
        }
    else if (targetDistance >= distanceB)
        {
        data = AtEnd ();
        return false;
        }
    // Binary interior search ... The callback only looks at the pathDistance ....
    PathEntry key (targetDistance);
    // Use end ()-1 as upper bound -- hence the "too far" result is a valid list member.
    auto memberB = std::upper_bound (m_locations.begin (), m_locations.end () - 1,
            key,
            PathEntry::IsLessThan_ByPathDistanceXY
            );

    if (memberB == m_locations.begin())
        {
        data = AtStart ();
        return true;
        }
    DPlane3d plane = DPlane3d::FromOriginAndNormal (targetDistance, 0,0, 1,0,0);
    bvector<CurveLocationDetailPair> intersections;
    memberB->m_curveDetail.curve->AppendCurvePlaneIntersections (plane, intersections);
    //  hm.. if it is an elevation curve, there should be only one intersection, and
    // it should be a single point (not an "on" interval).
    // Just take the first result.
    if (intersections.size () >= 1)
        {
        data = PathLocationDetail (intersections[0].detailA, memberB->m_pathIndex, targetDistance);
        return true;
        }
    // Plane intersection failed.  This should not happen.  take memberB.
    data = *memberB;
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVectorWithDistanceIndex::StrokeHorizontalAndVerticalCurves
(
IFacetOptionsCR xyOptions,  //!< [in] required options for xy stroke.
IFacetOptionsCR zOptions,   //!< [in] optional options for z stroke.
CurveVectorWithDistanceIndex  &xyCurve, //!< [in] space curve, typically with z identically 0, indexed by distance along curve
CurveVectorWithDistanceIndex  &zCurve,  //!< [in] z curve, with x being distance along curve and elevation in z.
bvector<PathLocationDetailPair> &locations  //!> [out] complete details of locations on xyCurve, zCurve
)
    {
    bvector<PathLocationDetail> locationsA;
    xyCurve.Stroke (locationsA, xyOptions);
    size_t errors = 0;
    for (PathLocationDetail const &detailA : locationsA)
        {
        PathLocationDetail detailB;
        if (!zCurve.SearchByElevationMapXPlane (detailA.m_pathDistance, detailB))
            errors++;   // But the result is still pushed -- maybe off the end?
        locations.push_back (PathLocationDetailPair (detailA, detailB));
        }
    return errors == 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVectorWithDistanceIndex::StrokeHorizontalAndVerticalCurves
(
IFacetOptionsCR xyOptions,  //!< [in] required options for xy stroke.
IFacetOptionsCR zOptions,   //!< [in] optional options for z stroke.
CurveVectorWithDistanceIndex  &xyCurve, //!< [in] space curve, typically with z identically 0, indexed by distance along curve
CurveVectorWithXIndex  &zCurve,  //!< [in] z curve, with x being distance along curve and elevation in z.
bvector<PathLocationDetailPair> &locations  //!> [out] complete details of locations on xyCurve, zCurve
)
    {
    bvector<PathLocationDetail> locationsA;
    xyCurve.Stroke (locationsA, xyOptions);
    size_t errors = 0;
    for (PathLocationDetail const &detailA : locationsA)
        {
        ValidatedPathLocationDetail detailB =
            zCurve.XToPathLocationDetail (detailA.m_pathDistance);
        if (!detailB.IsValid ())
            errors++;   // But the result is still pushed -- maybe off the end?
        locations.push_back (PathLocationDetailPair (detailA, detailB));
        }
    return errors == 0;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void PathLocationDetailPair::Merge
(
bvector<PathLocationDetailPair> const &pairs,
bvector<DPoint3d> *mergedXYZ,   //! [out] optional vector of points with xy from DetailA(), z from DetailB().
bvector<double>   *distanceA    //! [out] optional vector of distances.
)
    {
    if (nullptr != mergedXYZ)
        {
        mergedXYZ->clear ();
        mergedXYZ->reserve (pairs.size ());
        for (size_t i = 0; i < pairs.size (); i++)
            {
            DPoint3d xyPoint = pairs[i].DetailA ().m_curveDetail.point;
            DPoint3d zPoint  = pairs[i].DetailB ().m_curveDetail.point;
            mergedXYZ->push_back (DPoint3d::From (xyPoint.x, xyPoint.y, zPoint.z));
            }
        }
    if (nullptr != distanceA)
        {
        distanceA->clear ();
        distanceA->reserve (pairs.size ());
        for (size_t i = 0; i < pairs.size (); i++)
            {
            distanceA->push_back (pairs[i].DetailA ().m_pathDistance);
            }
        }
    }





END_BENTLEY_GEOMETRY_NAMESPACE
