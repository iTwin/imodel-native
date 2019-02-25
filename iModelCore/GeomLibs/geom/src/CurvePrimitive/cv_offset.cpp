/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/cv_offset.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static double s_minTolerance = 1.0e-8;
static double s_defaultRelTol = 1.0e-6;
static double s_maxChamferFraction = 0.80;  //fraction of pi
static int s_noisy = 0;
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
CurveOffsetOptions::CurveOffsetOptions (double offsetDistance)
    {
    m_arcAngle  = -1.0;
    m_bCurveMethod = 1;
    m_chamferAngle = Angle::PiOver2 ();
    m_forceClosure = false;
    m_offsetDistance = offsetDistance;
    SetTolerance (s_defaultRelTol * fabs (offsetDistance));
    SetBCurvePointsPerKnot (2);
    for (int i = 0; i < _countof (m_unusedDouble); i++)
        m_unusedDouble[i] = 0.0;
    for (int i = 0; i < _countof (m_unusedInt); i++)
        m_unusedInt[i] = 0;
    for (int i = 0; i < _countof (m_unusedBool); i++)
        m_unusedBool[i] = false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
void CurveOffsetOptions::SetTolerance (double tolerance)
    {
    m_tolerance = tolerance;
    if (m_tolerance < s_minTolerance)
        m_tolerance = s_minTolerance;
    }

void CurveOffsetOptions::SetOffsetDistance (double distance){m_offsetDistance = distance;}
void CurveOffsetOptions::SetArcAngle (double radians)       {m_arcAngle = radians;}
void CurveOffsetOptions::SetChamferAngle (double radians)
    {
    m_chamferAngle = radians;
    double maxChamfer = s_maxChamferFraction * Angle::Pi ();
    if (m_chamferAngle > maxChamfer)
        m_chamferAngle = maxChamfer;
    }

void CurveOffsetOptions::SetForceClosure (bool value)       {m_forceClosure = value;}
void CurveOffsetOptions::SetBCurvePointsPerKnot (int  value)       {m_bCurvePointsPerKnot = value;}
void CurveOffsetOptions::SetBCurveMethod (int  value)       {m_bCurveMethod = value;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
double CurveOffsetOptions::GetTolerance () const       {return m_tolerance;}
double CurveOffsetOptions::GetOffsetDistance () const  {return m_offsetDistance;}
double CurveOffsetOptions::GetArcAngle () const        {return m_arcAngle;}
double CurveOffsetOptions::GetChamferAngle () const    {return m_chamferAngle;}
 bool  CurveOffsetOptions::GetForceClosure () const    {return m_forceClosure;}
int    CurveOffsetOptions::GetBCurvePointsPerKnot () const {return m_bCurvePointsPerKnot;}
int    CurveOffsetOptions::GetBCurveMethod () const {return m_bCurveMethod;}



struct OffsetItem
{
ICurvePrimitivePtr m_offsetCurve;
DPoint3d m_basePoint[2];
DRay3d   m_offsetTangent[2];
double   m_cutbackFraction[2];
int      m_connectState[2];   // CONNECT_UNDEFINED, etc
bool     m_approvedFractions;
DEllipse3d m_outboundArc;
int     m_numJointPoints;   // 1==>common extension. 2==> single chamfer, 0==> plain chord, -1 ==> arc

static const int CONNECT_UNDEFINED = 0;
static const int CONNECT_TENTATIVE_FRACTION = 1;
static const int CONNECT_CONFIRMED_FRACTION = 2;
static const int CONNECT_BY_JOINT_GEOMETRY = 3;

OffsetItem (ICurvePrimitivePtr baseCurve, ICurvePrimitivePtr offsetCurve)
    : m_offsetCurve (offsetCurve), m_approvedFractions (false)
    {
    for (int i = 0; i < 2; i++)
        {
        double f = (double)i;
        baseCurve->FractionToPoint (f, m_basePoint[i]);
        m_offsetCurve->FractionToPoint (f, m_offsetTangent[i].origin, m_offsetTangent[i].direction  );
        m_cutbackFraction[i] = f;
        m_connectState[i] = CONNECT_UNDEFINED;
        m_numJointPoints = 0;
        }
    }

void DefineOutputArc (int numJointPoints, OffsetItem & next, double offsetDistance)
    {
    double sweep = m_offsetTangent[1].direction.AngleToXY (next.m_offsetTangent[0].direction);
    DVec3d vector0 = DVec3d::FromStartEnd (m_basePoint[1], m_offsetTangent[1].origin);
    if (Angle::NearlyEqualAllowPeriodShift (sweep, Angle::Pi ()))
        {
        if (offsetDistance > 0.0)
            sweep = fabs (sweep);
        else
            sweep = - fabs (sweep);
        }
    DVec3d vector90 = vector0.FromCCWPerpendicularXY (vector0);
    m_outboundArc = DEllipse3d::FromVectors (m_basePoint[1], vector0, vector90, 0.0, sweep);
    m_numJointPoints = numJointPoints;
    m_connectState[1] = CONNECT_BY_JOINT_GEOMETRY;
    next.m_connectState[0] = CONNECT_BY_JOINT_GEOMETRY;
    }

int FixIndex (int index)
    {
    return index <= 0 ? 0 : 1;
    }

bool IsVerifyableState (int value)
    {
    return value == CONNECT_CONFIRMED_FRACTION || value == CONNECT_TENTATIVE_FRACTION;
    }

bool VerifyFractions ()
    {
    m_approvedFractions =
           IsVerifyableState (m_connectState[0])
        && IsVerifyableState (m_connectState[1])
        && m_cutbackFraction[0] < m_cutbackFraction[1];
    return m_approvedFractions;
    }
void SetFraction (int index, double fraction)
    {
    index = FixIndex (index);
    m_cutbackFraction[index] = fraction;
    m_connectState[index] = CONNECT_TENTATIVE_FRACTION;
    }
    
void SetState (int index, int value)
    {
    index = FixIndex (index);
    m_connectState[index] = value;
    }
// Set the fractions for extending offsetA to offsetB.
static bool SetCutbackFractions (OffsetItem &offsetA, OffsetItem &offsetB, CurveOffsetOptionsCR options)
    {
    DSegment3d segmentA, segmentB;
    double turnAngle = offsetA.m_offsetTangent[1].direction.AngleToXY (offsetB.m_offsetTangent[0].direction);
    double offsetDistance = options.GetOffsetDistance ();
    double arcAngle = options.GetArcAngle ();
    double chamferAngle = options.GetChamferAngle ();
    double signedTurnAngle = offsetDistance > 0.0 ? turnAngle : -turnAngle;
    if (arcAngle > 0.0 && signedTurnAngle > arcAngle)
        {
        offsetA.DefineOutputArc (-1, offsetB, offsetDistance);
        return true;
        }
    else if (chamferAngle > 0.0 && signedTurnAngle > chamferAngle)
        {
        int numPart = (int) ((signedTurnAngle / chamferAngle) + 0.999999999);
        offsetA.DefineOutputArc (numPart, offsetB, offsetDistance);
        return true;
        }
    else if (   offsetA.m_offsetCurve->TryGetLine (segmentA)
        && offsetB.m_offsetCurve->TryGetLine (segmentB)
        )
        {
        double fractionA, fractionB;
        DPoint3d pointA, pointB;
        if (DSegment3d::IntersectXY (fractionA, fractionB, pointA, pointB, segmentA, segmentB))
            {
            offsetA.SetFraction (1, fractionA);
            offsetB.SetFraction (0, fractionB);
            return true;
            }
        }

    offsetA.DefineOutputArc (0, offsetB, offsetDistance);
    return false;
    }


// Set the fractions for extending offsetA to offsetB.
static bool AppendSimplePrimitive (CurveVectorR collector, OffsetItem &offset)
    {
    if (offset.m_offsetCurve.IsValid ())
        {
        collector.push_back (offset.m_offsetCurve);
        return true;
        }
    return false;
    }

static bool AppendOuterArcStrokes (CurveVectorR collector, OffsetItem &offset, int numSector)
    {
    bvector<DPoint3d> points;
    if (numSector < 1)
        numSector = 1;
    double theta = offset.m_outboundArc.sweep;
    if (fabs (theta) > Angle::TwoPi () * 0.8 && numSector == 1)
        numSector = 2;
    double beta = 0.5 * theta / numSector;
    double a = cos (beta);
    DEllipse3d e1 = offset.m_outboundArc;
    e1.vector0.Scale (1.0 / a);
    e1.vector90.Scale (1.0 / a);
    DPoint3d xyz;
    offset.m_outboundArc.FractionParameterToPoint (xyz, 0.0);
    points.push_back (xyz);
    for (int i = 0; i < numSector; i++)
        {
        double f = (1 + 2 * i) / (double) (2.0 * numSector);
        e1.FractionParameterToPoint (xyz, f);
        points.push_back (xyz);
        }
    offset.m_outboundArc.FractionParameterToPoint (xyz, 1.0);
    points.push_back (xyz);
    if (numSector == 1 && DPoint3dOps::AlmostEqual (points.front (), points.back ()))
        {
        }
    else
        {
        ICurvePrimitivePtr polyline = ICurvePrimitive::CreateLineString (points);
        collector.push_back (polyline);
        }
    return true;
    }
    
static bool AppendSimpleJoint (CurveVectorR collector, OffsetItem &offsetA, OffsetItem &offsetB, CurveOffsetOptionsCR options)
    {
    double turnAngle = offsetA.m_offsetTangent[1].direction.AngleToXY (offsetB.m_offsetTangent[0].direction);
    double offsetDistance = options.GetOffsetDistance ();
    double arcAngle = options.GetArcAngle ();
    double chamferAngle = options.GetChamferAngle ();
    double signedTurnAngle = offsetDistance > 0.0 ? turnAngle : -turnAngle;
    bvector<DPoint3d>points;
#define Almost180 Angle::DegreesToRadians (179.0)
    if (fabs (turnAngle) > Almost180)
        {
        return false;
        }
    if (arcAngle > 0.0 && signedTurnAngle > arcAngle)
        {
        offsetA.DefineOutputArc (-1, offsetB, offsetDistance);
        ICurvePrimitivePtr curve = ICurvePrimitive::CreateArc (offsetA.m_outboundArc);
        collector.push_back (curve);        
        return true;
        }
    else if (chamferAngle > 0.0)
        {
        int numPart = 1;
        if (signedTurnAngle > chamferAngle)     // negative turn always makes the intersection point -- line consolidation will eliminate it !!
            numPart = (int) ((fabs (signedTurnAngle) / chamferAngle) + 0.999999999);
        offsetA.DefineOutputArc (numPart, offsetB, offsetDistance);
        return AppendOuterArcStrokes (collector, offsetA, numPart);
        }

    // hm...
    offsetA.DefineOutputArc (turnAngle > Angle::PiOver2 () ? 2 : 1, offsetB, offsetDistance);
    ICurvePrimitivePtr curve = ICurvePrimitive::CreateArc (offsetA.m_outboundArc);
    collector.push_back (curve);        
    return false;
    }


static bool IsOuterTurn (DVec3dCR tangentA, DVec3dCR tangentB, double offset)
    {
    return tangentA.CrossProductXY (tangentB) * offset > 0.0;
    }
ICurvePrimitivePtr CreateResolvedOffset ()
    {
    if (m_approvedFractions)
        return m_offsetCurve->CloneBetweenFractions (m_cutbackFraction[0], m_cutbackFraction[1], true);
    else
        return m_offsetCurve;
    }
    
DPoint3d ResolvedEndpoint (int index)
    {
    double f = m_cutbackFraction[FixIndex(index)];
    DPoint3d point;
    m_offsetCurve->FractionToPoint (f, point);
    return point;
    }
};

struct PathOffsetContext
{
bvector<OffsetItem> m_simpleOffsets;
CurveOffsetOptions m_options;
PathOffsetContext (CurveOffsetOptionsCR options)
    : m_options (options)   // just copy it in...
    {
    }
    
// Collect all simple offsets.  Return false if 
bool LoadSimpleOffsets (CurveVectorCR path)
    {
    bool stat = true;
    m_simpleOffsets.clear ();
    for (size_t i = 0; i < path.size (); i++)
        {
        ICurvePrimitivePtr simpleOffset = path[i]->CloneAsSingleOffsetPrimitiveXY (m_options);
        if (simpleOffset.IsValid ())
            m_simpleOffsets.push_back (OffsetItem (path[i], simpleOffset));
        else
            stat = false;
        }
    return stat;
    }

void AppendResolvedPrimitive (CurveVectorR collector, OffsetItem &data)
    {
    ICurvePrimitivePtr curve = data.CreateResolvedOffset ();
    if (curve.IsValid ())
        {
        collector.push_back (curve);
        }
    }

void AppendResolvedJoint (CurveVectorR collector, OffsetItem &dataA, OffsetItem &dataB, CurveOffsetOptionsCR options)
    {
    double r;
    static int s_default = 0;
    if (dataA.m_connectState[1] == OffsetItem::CONNECT_BY_JOINT_GEOMETRY)
        {
        if (dataA.m_numJointPoints == -1)
            {
            ICurvePrimitivePtr curve = ICurvePrimitive::CreateArc (dataA.m_outboundArc);
            collector.push_back (curve);
            return;
            }
        }
    if (dataA.m_connectState[1] == OffsetItem::CONNECT_CONFIRMED_FRACTION
            && dataB.m_connectState[0] == OffsetItem::CONNECT_CONFIRMED_FRACTION
            )
        {
        DPoint3d pointA = dataA.ResolvedEndpoint (1);
        DPoint3d pointB = dataB.ResolvedEndpoint(0);
        if (!DPoint3dOps::AlmostEqual (pointA, pointB))
            {
            ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB));
            collector.push_back (curve);
            }
        }
    else if (s_default == 1 && dataA.m_outboundArc.IsCircular (r))
        {
        ICurvePrimitivePtr curve = ICurvePrimitive::CreateArc (dataA.m_outboundArc);
        collector.push_back (curve);
        return;
        
        // ??
        }
    else
        {
        DPoint3d pointA = dataA.ResolvedEndpoint (1);
        DPoint3d pointB = dataB.ResolvedEndpoint(0);
        if (!DPoint3dOps::AlmostEqual (pointA, pointB))
            {
            ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB));
            collector.push_back (curve);
            }
        }
    }

CurveVectorPtr OutputWithSimpleJoints (CurveVector::BoundaryType btype, bool xyOnly)
    {
    CurveVectorPtr result = CurveVector::Create (btype);
    bool closed =  btype == CurveVector::BOUNDARY_TYPE_Outer
                || btype == CurveVector::BOUNDARY_TYPE_Inner;
    ptrdiff_t   numPrimitive = m_simpleOffsets.size ();
    if (numPrimitive == 0)
        return NULL;
    ptrdiff_t numBase = numPrimitive;
    if (!closed)
        numBase--;

    for (ptrdiff_t i = 0; i < numBase; i++)
        {
        ptrdiff_t j = i + 1;
        if (j >= numPrimitive)
            j = 0;
        OffsetItem::AppendSimplePrimitive (*result, m_simpleOffsets[i]);
        OffsetItem::AppendSimpleJoint (*result, m_simpleOffsets[i], m_simpleOffsets[j], m_options);
        }
    if (numBase < numPrimitive)
        OffsetItem::AppendSimplePrimitive (*result, m_simpleOffsets[numBase]);
    if (result.IsValid ())
        result->ConsolidateAdjacentPrimitives (true, xyOnly);        
    return result;
    }

CurveVectorPtr JoinOffsets (CurveVector::BoundaryType btype)
    {
    CurveVectorPtr result = CurveVector::Create (btype);
    bool closed =  btype == CurveVector::BOUNDARY_TYPE_Outer
                || btype == CurveVector::BOUNDARY_TYPE_Inner;
    ptrdiff_t   numPrimitive = m_simpleOffsets.size ();
    ptrdiff_t numBase = numPrimitive;
    if (!closed)
        numBase--;

    for (ptrdiff_t i = 0; i < numBase; i++)
        {
        ptrdiff_t j = i + 1;
        if (j >= numBase)
            j = 0;
        OffsetItem::SetCutbackFractions (m_simpleOffsets[i], m_simpleOffsets[j], m_options);
        }
    for (ptrdiff_t i = 0; i < numPrimitive; i++)
        m_simpleOffsets[i].VerifyFractions ();
    
    for (ptrdiff_t i = 0; i < numPrimitive; i++)
        {
        AppendResolvedPrimitive (*result, m_simpleOffsets[i]);
        if (i <= numBase)
            {
            ptrdiff_t j = i + 1;
            if (j >= numBase)
                j = 0;
            AppendResolvedJoint (*result, m_simpleOffsets[i], m_simpleOffsets[j], m_options);
            }
        }
    if (result.IsValid ())
        result->ConsolidateAdjacentPrimitives ();
    return result;
    }
};
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CloneOffsetCurvesXY (CurveOffsetOptionsCR options) const
    {
    BoundaryType btype = GetBoundaryType ();
    if (btype == BOUNDARY_TYPE_Open
        || btype == BOUNDARY_TYPE_Inner || btype == BOUNDARY_TYPE_Outer
        )
        {
        if (CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString) > 0)
            {
            CurveVectorPtr source1 = CloneWithExplodedLinestrings ();
            return source1->CloneOffsetCurvesXY (options);
            }
        PathOffsetContext context (options);
        context.LoadSimpleOffsets (*this);
        return context.OutputWithSimpleJoints(btype, true);
        }
    else
        {
        CurveVectorPtr clone = Create (btype);
        for (size_t i = 0; i < size (); i++)
            {
            ICurvePrimitiveR childPrimitive = *at (i);
            CurveVectorPtr childVector = childPrimitive.GetChildCurveVectorP ();
            if (childVector.IsValid ())
                {
                CurveVectorPtr childClone = childVector->CloneOffsetCurvesXY (options);
                if (childClone.IsValid ())
                    clone->push_back (ICurvePrimitive::CreateChildCurveVector (childClone));
                }
            else
                {
                ICurvePrimitivePtr childOffset = childPrimitive.CloneAsSingleOffsetPrimitiveXY (options);
                if (childOffset.IsValid ())
                    clone->push_back (childOffset);
                }
            }
        return clone;
        }
    }


static void CollectOffsetSegment (DRay3dCR tangentA1, DSegment3dCR segment, DRay3dCR tangentC0,
CurveOffsetOptionsCR options,
CurveVectorR collector
)
    {
    double r1 = options.GetOffsetDistance ();
    DVec3d tangent = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
    DVec3d perp;
    perp.UnitPerpendicularXY (tangent);
    perp.Scale (-r1);
    DPoint3d points[5];
    size_t n = 0;
    if (r1 > 0.0)
        {
        points[0] = points[4] = segment.point[0];
        points[1].SumOf (points[0], perp);
        points[3] = segment.point[1];
        points[2].SumOf (points[3], perp);
        n = 5;
        }
    else if (r1 < 0.0)
        {
        points[0] = points[4] = segment.point[0];
        points[1] = segment.point[1];
        points[2].SumOf (points[1], perp);
        points[3].SumOf (points[0], perp);
        n = 5;
        }
    if (n > 0)
        {
        collector.push_back (
            ICurvePrimitive::CreateChildCurveVector (
                    CurveVector::CreateLinear (points, n, CurveVector::BOUNDARY_TYPE_Outer)));
        }
    }

static bool CollectOffsetCircularArc (DRay3dCR tangentA1, DEllipse3d arc, DRay3dCR tangentC0,
CurveOffsetOptionsCR options,
CurveVectorR collector
)
    {
    double r0;
    double dr0 = options.GetOffsetDistance ();
    DPoint3d points[10];
    DPoint3d arcStart, arcEnd;
    arc.FractionParameterToPoint (arcStart, 0.0);
    arc.FractionParameterToPoint (arcEnd, 1.0);
    if (arc.IsCircularXY (r0))
        {
        DVec3d normal = DVec3d::FromNormalizedCrossProduct (arc.vector0, arc.vector90);
        double dr = dr0;
        double q = normal.z * arc.sweep;
        if (q < 0.0)
            dr = -dr;
        double r1 = r0 + dr;
        if (r1 < 0.0)
            {
            // just make pie slice to center ...
            if (q > 0.0)
                {
                points[0] = arcEnd;
                points[1] = arc.center;
                points[2] = arcStart;
                CurveVectorPtr region = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
                region->push_back (ICurvePrimitive::CreateArc (arc));
                region->push_back (ICurvePrimitive::CreateLineString (points, 3));
                collector.push_back (ICurvePrimitive::CreateChildCurveVector (region));
                return true;
                }
            else
                {
                points[0] = arcStart;
                points[1] = arc.center;
                points[2] = arcEnd;
                DEllipse3d arc1 = DEllipse3d::FromReversed (arc);
                CurveVectorPtr region = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
                region->push_back (ICurvePrimitive::CreateArc (arc1));
                region->push_back (ICurvePrimitive::CreateLineString (points, 3));
                collector.push_back (ICurvePrimitive::CreateChildCurveVector (region));
                return true;
                }
            }
        else
            {
            DEllipse3d arc1 = arc;
            arc1.vector0.ScaleToLength (r1);
            arc1.vector90.ScaleToLength (r1);
            DPoint3d arc1Start, arc1End;
            arc1.FractionParameterToPoint (arc1Start, 0.0);
            arc1.FractionParameterToPoint (arc1End, 1.0);
            if (q * dr > 0.0)
                {
                DEllipse3d arc2 = DEllipse3d::FromReversed (arc);
                CurveVectorPtr region = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
                region->push_back (ICurvePrimitive::CreateArc (arc1));
                region->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (arc1End, arcEnd)));
                region->push_back (ICurvePrimitive::CreateArc (arc2));
                region->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (arcStart, arc1Start)));
                collector.push_back (ICurvePrimitive::CreateChildCurveVector (region));
                return true;
                }
            else
                {
                DEllipse3d arc3 = DEllipse3d::FromReversed (arc1);
                CurveVectorPtr region = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
                region->push_back (ICurvePrimitive::CreateArc (arc));
                region->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (arcEnd, arc1End)));
                region->push_back (ICurvePrimitive::CreateArc (arc3));
                region->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (arc1Start, arcStart)));
                collector.push_back (ICurvePrimitive::CreateChildCurveVector (region));
                return true;
                }
            }
        }
    return false;
    }



static void CollectArcJoint (
DRay3dCR tangent0,
DRay3dCR tangent1,
CurveOffsetOptionsCR options,
CurveVectorR collector
)
    {
    static double s_sliverAngle = 1.0e-2;
    double r1 = options.GetOffsetDistance ();
    double theta = tangent0.direction.AngleToXY (tangent1.direction);
    DPoint3d points[10];
    DVec3d perp0, perp1, vector900;
    perp0.UnitPerpendicularXY (tangent0.direction);
    perp1.UnitPerpendicularXY (tangent1.direction);
    vector900.Normalize (tangent0.direction);
    perp0.Scale (-r1);
    perp1.Scale (-r1);
    vector900.Scale (r1);

    if (Angle::IsNearZeroAllowPeriodShift (theta))
        {
        // no gap to fill
        }
    else if (r1 * theta <= 0.0)
        {
        // joint is "inside" a corner..
        }
    else if (fabs (theta) < s_sliverAngle)
        {
        // NEEDS WORK -- sign of r ?
        points[0] = tangent0.origin;
        points[1].SumOf (points[0], perp0);
        points[2].SumOf (points[0], perp1);
        points[3] = points[0];
        collector.push_back (
                ICurvePrimitive::CreateChildCurveVector
                    (CurveVector::CreateLinear (points, 4, CurveVector::BOUNDARY_TYPE_Outer)));
        }
    else if (r1 > 0.0)
        {
        DPoint3d arcStart = DPoint3d::FromSumOf (tangent0.origin, perp0, 1.0);
        DPoint3d arcEnd = DPoint3d::FromSumOf (tangent0.origin, perp1, 1.0);
        points[0] = arcEnd;
        points[1] = tangent0.origin;
        points[2] = arcStart;
        ICurvePrimitivePtr lines = ICurvePrimitive::CreateLineString (points, 3);
        DEllipse3d arc = DEllipse3d::FromVectors
                (
                tangent0.origin,
                perp0,
                vector900,
                0.0,
                theta
                );
        CurveVectorPtr region = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
        region->push_back (ICurvePrimitive::CreateArc (arc));
        region->push_back (ICurvePrimitive::CreateLineString (points, 3));
        collector.push_back (ICurvePrimitive::CreateChildCurveVector (region));
        }
    else if (r1 < 0.0)
        {
        // theta is also less than zero !!!
        DPoint3d arcStart = DPoint3d::FromSumOf (tangent0.origin, perp1, 1.0);
        DPoint3d arcEnd = DPoint3d::FromSumOf (tangent0.origin, perp0, 1.0);
        points[0] = arcStart;
        points[1] = tangent0.origin;
        points[2] = arcEnd;
        ICurvePrimitivePtr lines = ICurvePrimitive::CreateLineString (points, 3);
        DEllipse3d arc = DEllipse3d::FromVectors
                (
                tangent0.origin,
                perp0,
                vector900,
                theta,
                -theta
                );
        CurveVectorPtr region = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
        region->push_back (ICurvePrimitive::CreateArc (arc));
        region->push_back (ICurvePrimitive::CreateLineString (points, 3));
        collector.push_back (ICurvePrimitive::CreateChildCurveVector (region));
        }
    }    


static void CollectJoint (
DRay3dCR tangent0,
DRay3dCR tangent1,
CurveOffsetOptionsCR options,
CurveVectorR collector
)
    {
    static double s_sliverAngle = 1.0e-2;
    double r1 = options.GetOffsetDistance ();
    double arcRadians = options.GetArcAngle ();
    double theta = tangent0.direction.AngleToXY (tangent1.direction);
    double absTheta = fabs (theta);

    DVec3d perp0, perp1;
    perp0.UnitPerpendicularXY (tangent0.direction);
    perp1.UnitPerpendicularXY (tangent1.direction);

    bvector<DPoint3d> points;
    points.reserve (10);
    if (Angle::IsNearZeroAllowPeriodShift (theta))
        {
        // no gap to fill
        }
    else if (r1 * theta <= 0.0)
        {
        // joint is "inside" a corner..
        }
    else if (absTheta < s_sliverAngle)
        {
        // NEEDS WORK -- sign of r ?
        points.push_back (tangent0.origin);
        points.push_back (DPoint3d::FromSumOf (points[0], perp0, -r1));
        points.push_back (DPoint3d::FromSumOf (points[0], perp1, -r1));
        points.push_back (tangent0.origin);
        collector.push_back (
                ICurvePrimitive::CreateChildCurveVector
                    (CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer)));
        }
    else if (arcRadians > 0.0 && absTheta >= arcRadians)
        {
        CollectArcJoint (tangent0, tangent1, options, collector);
        }
    else
        {
        double chamferRadians = options.GetChamferAngle ();
        double maxChamferAngle = Angle::PiOver2 ();
        if (chamferRadians <= 0.0 || chamferRadians > maxChamferAngle)
            chamferRadians = maxChamferAngle;
        static double roundoffShift = 0.99999;
        int numTurn = (int)(roundoffShift + absTheta / chamferRadians);
        double alpha = 0.5 * theta / (double)numTurn;
        double r2 = r1 / cos (alpha);   // radius of "outer" circle.
        if (s_noisy)
            printf (" (r1 %g) (r2 %g) (alpha %g) (numTurn %d)\n", r1, r2, alpha, numTurn);
        DPoint3d center = tangent0.origin;
        DVec3d radialVector;
        points.push_back (center);
        points.push_back (DPoint3d::FromSumOf (center, perp0, -r1));
        // there are {numTurns+1} chords around the total turn.
        // to make them tangent to the arc of radius r1, the interior chords cover twice the angle
        // of the ones at either end.
        // Angle from start to first chord point is alpha.
        // Angle to successive chord points is 2*alpha
        for (int k = 1; k < 2 * numTurn; k += 2)
            {
            double beta = k * alpha;
            radialVector.RotateXY (perp0, beta);
            points.push_back (DPoint3d::FromSumOf (center, radialVector, -r2));
            }
        points.push_back (DPoint3d::FromSumOf (center, perp1, -r1));
        points.push_back (center);

        if (r1 < 0.0)
            DPoint3dOps::Reverse (points);

        collector.push_back (
                ICurvePrimitive::CreateChildCurveVector
                    (CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer)));
        }
    }




static void CollectOffsetArea (
ICurvePrimitiveCP curveA,       //  may be used to clip at curveB.start
ICurvePrimitiveCR curveB,       // Create pure offset from here -- ends move perpendicular.
ICurvePrimitiveCP curveC,       // create joint solid from curveB.end to curveC.start
CurveOffsetOptionsCR options,
CurveVectorR collector
)
    {
    DRay3d tangentB0, tangentB1;
    DRay3d tangentC0, tangentA1;
    curveB.FractionToPoint (0.0, tangentB0);
    curveB.FractionToPoint (1.0, tangentB1);
    if (NULL == curveA)
        tangentA1 = tangentB0;
    else
        curveA->FractionToPoint (1.0, tangentA1);

    if (NULL == curveC)
        tangentC0 = tangentB1;
    else
        curveC->FractionToPoint (0.0, tangentC0);
    DSegment3d segment;
    DEllipse3d arc;
    bool done = false;
    if (curveA->TryGetLine (segment))
        {
        CollectOffsetSegment (tangentA1, segment, tangentC0, options, collector);
        done = true;
        }
    else if (curveA->TryGetArc (arc))
        {
        done = CollectOffsetCircularArc (tangentA1, arc, tangentC0, options, collector);
        }
#ifdef primitiveSupport
    else
        CollectOffsetPrimitive (tangentA1, arc, tangentC0, options, collector);
#endif
    if (NULL != curveC)
        CollectJoint (tangentB1, tangentC0, options, collector);
    }
static void CollectOffsetAreas (CurveVectorCR source, CurveOffsetOptionsCR options, CurveVectorR collector)
    {
    CurveVector::BoundaryType btype = source.GetBoundaryType ();
    if (btype == CurveVector::BOUNDARY_TYPE_Open
        || btype == CurveVector::BOUNDARY_TYPE_Inner
        || btype == CurveVector::BOUNDARY_TYPE_Outer
        )
        {
        if (source.CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString) > 0)
            {
            CurveVectorPtr source1 = source.CloneWithExplodedLinestrings ();
            CollectOffsetAreas (*source1, options, collector);
            return;
            }
        // simple primitive offset + joint to successor.
        if (btype == CurveVector::BOUNDARY_TYPE_Open)
            {
            ICurvePrimitiveCP curveA = NULL;
            ICurvePrimitiveCP curveB, curveC;
            for (size_t iB = 0, n = source.size (); iB < n; iB++, curveA = curveB)
                {
                curveB = source.at(iB).get ();
                curveC = iB + 1 == n ? NULL : source.at(iB+1).get ();
                CollectOffsetArea (curveA, *curveB, curveC, options, collector);
                }
            }
        else
            {
            for (size_t iB = 0, n = source.size (); iB < n; iB++)
                {
                size_t iA = (iB + n - 1) % n;
                size_t iC = (iB + 1) % n;
                CollectOffsetArea (source.at (iA).get (), *source.at (iB).get (), source.at(iC).get (),
                        options, collector);
                }
            }
        }
    else
        {
        for (size_t i = 0; i < source.size (); i++)
            {
            ICurvePrimitiveR childPrimitive = *source.at (i);
            CurveVectorPtr childVector = childPrimitive.GetChildCurveVectorP ();
            if (childVector.IsValid ())
                {
                CollectOffsetAreas (*childVector, options, collector);
                }
            else
                {
                CollectOffsetArea (NULL, childPrimitive, NULL, options, collector);
                }
            }
        }
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::AreaOffset (CurveOffsetOptionsCR options) const
    {
    CurveVectorPtr offsetCollection = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    CollectOffsetAreas (*this, options, *offsetCollection);
    CurveVectorPtr result;
    if (options.GetOffsetDistance () > 0.0)
        result = AreaUnion (*this, *offsetCollection);
    else
        result = AreaDifference (*this, *offsetCollection);
    if (result.IsValid ())
      result->ConsolidateAdjacentPrimitives ();
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr  CurveVector::ThickenXYPathToArea(CurveVectorPtr const& centerPath, double leftOffset, double rightOffset)
    {
    if (centerPath.IsNull ())
        return nullptr;

    double fastLength = centerPath->FastLength();
    double  pointTolerance = 0.001 *  DoubleOps::MaxAbs (leftOffset, rightOffset, 0.01 * fastLength);

    // move the curve towards left by half width
    CurveOffsetOptions  offsetOptions(leftOffset);
    offsetOptions.SetTolerance(pointTolerance);

    auto left = fabs (leftOffset) < pointTolerance ? centerPath->Clone () : centerPath->CloneOffsetCurvesXY(offsetOptions);
    if (!left.IsValid())
        return  nullptr;

    // move the curve towards right by half width
    offsetOptions.SetOffsetDistance(-rightOffset);

    auto right = fabs(rightOffset) < pointTolerance ? centerPath->Clone() : centerPath->CloneOffsetCurvesXY(offsetOptions);
    if (!right.IsValid())
        return  nullptr;

    if (centerPath->IsClosedPath())
        {
        auto parityRegion = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
        parityRegion->Add(right);
        parityRegion->Add(left);
        parityRegion->FixupXYOuterInner(false);
        return parityRegion;
        }
    else
        {
        auto shape = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);

        ICurvePrimitivePtr  top, bottom;
        // get start and end points
        DPoint3d    starts[2], ends[2];
        if (!left->GetStartEnd(starts[0], ends[0]) || !right->GetStartEnd(starts[1], ends[1]))
            return  nullptr;

        // bottom cap to connect left->right
        bottom = ICurvePrimitive::CreateLine(DSegment3d::From(ends[0], ends[1]));
        if (!bottom.IsValid())
            return  nullptr;

        // top cap to connect right->left
        top = ICurvePrimitive::CreateLine(DSegment3d::From(starts[1], starts[0]));
        if (!top.IsValid())
            return  nullptr;

        // reverse the right curve
        right->ReverseCurvesInPlace();

        // add and orient them to complete a loop:
        shape->AddPrimitives(*left);
        shape->Add(bottom);
        shape->AddPrimitives(*right);
        shape->Add(top);
        if (shape->IsClosedPath())
            return  shape;

        CurveGapOptions gapOptions(pointTolerance, 1.0e-4, 1.0e-4);
        return shape->CloneWithGapsClosed(gapOptions);
        }
    }


END_BENTLEY_GEOMETRY_NAMESPACE
