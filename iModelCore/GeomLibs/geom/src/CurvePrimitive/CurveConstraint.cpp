/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/ccctangent.fdf>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
CurveConstraint::CurveConstraint (Type type, CurveLocationDetailCR detail) :
    m_type (type),
    m_location (detail),
    m_curve (const_cast<ICurvePrimitiveP>(detail.curve)),
    m_ray (DRay3d::FromOriginAndVector (detail.point, DVec3d::UnitX ()))
    {

    }

CurveConstraint::CurveConstraint (Type type, CurveLocationDetailCR detail, DRay3dCR ray) :
    m_type (type),
    m_location (detail),
    m_curve (const_cast<ICurvePrimitiveP>(detail.curve)),
    m_ray (ray)
    {

    }

CurveConstraint::CurveConstraint (Type type, TransformCR frame) :
    m_type (type),
    m_resultFrame (frame, true)
    {
    }

CurveConstraint::CurveConstraint (Type type, double distance) :
    m_type (type),
    m_distance (distance, true)
    {
    }



bool CurveConstraint::IsType (CurveConstraint::Type t) const { return t == m_type;}
DPoint3d CurveConstraint::Point () const{return m_location.point;}
DRay3d CurveConstraint::PointAndDirection () const {return m_ray;}
CurveConstraint::Type CurveConstraint::GetType () const {return m_type;}
ValidatedTransform CurveConstraint::GetResultFrame () const {return m_resultFrame;}
ValidatedDouble CurveConstraint::GetDistance () const {return m_distance;}

CurveLocationDetail const &CurveConstraint::Location () const {return m_location;}

CurveConstraint CurveConstraint::CreateThroughPoint (DPoint3dCR point)
    {
    return CurveConstraint (Type::ThroughPoint, CurveLocationDetail (nullptr, 0.0, point));
    }

CurveConstraint CurveConstraint::CreatePointAndDirection (DPoint3dCR point, DVec3dCR direction)
    {
    auto line = ICurvePrimitive::CreateLine (DSegment3d::From (point, point + direction));
    return CurveConstraint (Type::PointAndDirection, CurveLocationDetail (
                    line.get (), 0.0, point),
                    DRay3d::FromOriginAndVector (point, direction));
    }

CurveConstraint CurveConstraint::CreateResultFrame (TransformCR frame)
    {
    return CurveConstraint (Type::ResultFrame, frame);
    }

CurveConstraint CurveConstraint::CreateRadius (double radius) {return CurveConstraint (Type::Radius, radius);}

CurveConstraint CurveConstraint::CreateCenter (DPoint3dCR point) {return CurveConstraint (Type::Center, CurveLocationDetail (nullptr, 0.0, point));}

CurveConstraint CurveConstraint::CreateClosestPoint (ICurvePrimitiveCP curve)
    {
    DPoint3d xyz;
    curve->FractionToPoint (0.0, xyz);
    return CurveConstraint (Type::ClosestPoint, CurveLocationDetail (curve, 0.0, xyz));
    }

CurveConstraint CurveConstraint::CreatePerpendicularNear (ICurvePrimitiveCP curve, double fraction)
    {
    DPoint3d xyz;
    curve->FractionToPoint (fraction, xyz);
    return CurveConstraint (Type::PerpendicularNear, CurveLocationDetail (curve, fraction, xyz));
    }
    
CurveConstraint CurveConstraint::CreateTangent (ICurvePrimitiveCP curve, double fraction)
    {
    DPoint3d xyz;
    curve->FractionToPoint (fraction, xyz);
    return CurveConstraint (Type::Tangent, CurveLocationDetail (curve, fraction, xyz));
    }

struct ConstraintMatchTable
{
friend struct ConstructionContext;
private:
static const int s_maxTableIndex = 6;

uint32_t m_numRequested;
uint32_t m_numMatch;
struct RowData
{
CurveConstraint::Type m_type;
uint32_t m_sourceIndex;
CurveConstraintCP m_constraint;

RowData
    (
    CurveConstraint::Type type,
    uint32_t sourceIndex,
    CurveConstraintCP constraint,
    int sortGroup = 1000,
    double sortSize = 0.0
    )
    : m_type (type), m_sourceIndex (sourceIndex), m_constraint (constraint)
    {}

RowData (CurveConstraint::Type type)
    : m_type (type), m_sourceIndex (UINT32_MAX), m_constraint (nullptr)
    {}

RowData ()
    : m_type (CurveConstraint::Type::None), m_sourceIndex (UINT32_MAX)
    {}

};
RowData m_row[s_maxTableIndex];
public:

ConstraintMatchTable () : m_numRequested(0), m_numMatch (0) {}

ConstraintMatchTable (CurveConstraint::Type type0) : m_numRequested(1), m_numMatch (0)
    {
    m_numRequested = 1; // *** NEEDS WORK: This is to work around what looks like a bug in clang on Linux. If I don't do this, I get: CurveConstraint.cpp:76:10: error: private field 'm_numRequested' is not used [-Werror,-Wunused-private-field]
    m_row[0] = RowData (type0);
    }

ConstraintMatchTable (CurveConstraint::Type type0, CurveConstraint::Type type1 ) : m_numRequested(2), m_numMatch (0)
    {
    m_row[0] = RowData (type0);
    m_row[1] = RowData (type1);
    }

ConstraintMatchTable (CurveConstraint::Type type0, CurveConstraint::Type type1, CurveConstraint::Type type2) : m_numRequested(3), m_numMatch (0)
    {
    m_row[0] = RowData (type0);
    m_row[1] = RowData (type1);
    m_row[2] = RowData (type2);
    }

bool FindSourceIndex (uint32_t target)
    {
    for (uint32_t i = 0; i < m_numMatch; i++)
        {
        if (m_row[i].m_sourceIndex == target)
            return true;
        }
    return false;
    }
void Clear () {m_numMatch = 0;}
// Enter the next constraint and increment m_numMatch !!!
void Enter
(
uint32_t index,         // index in these arrays.  This must be identical to m_numMatch !!!!
uint32_t hintIndex,   // index in input data
CurveConstraintCP constraint    // the constraint
)
    {
    if (index == m_numMatch && index < s_maxTableIndex)
        {
        m_row[index].m_sourceIndex = hintIndex;
        m_row[index].m_constraint = constraint;
        m_numMatch++;
        }
    }

CurveConstraintCP GetCurveConstraintCP (uint32_t tableIndex){ return tableIndex < s_maxTableIndex ? m_row[tableIndex].m_constraint : nullptr;}
ValidatedTransform GetResultFrame (uint32_t tableIndex){return tableIndex < s_maxTableIndex ? m_row[tableIndex].m_constraint->GetResultFrame (): ValidatedTransform (Transform::FromIdentity ());}
ValidatedDouble GetDistance (uint32_t tableIndex){return tableIndex < s_maxTableIndex ? m_row[tableIndex].m_constraint->GetDistance () : ValidatedDouble (0.0, false);}

void GetPointsSegmentsArcs (bvector<DPoint3d> &points, bvector<DSegment3d> &segments, bvector<DEllipse3d> &arcs)
    {
    segments.clear ();
    arcs.clear ();
    for (uint32_t i = 0; i < m_numMatch; i++)
        {
        auto constraint = m_row[i].m_constraint;
        if (constraint->IsType (CurveConstraint::Type::ThroughPoint))
            {
            points.push_back (constraint->Point ());
            }
        else if (constraint->m_location.curve != nullptr)
            {
            DEllipse3d arc;
            DSegment3d segment;
            if (constraint->m_location.curve->TryGetArc (arc))
                {
                arcs.push_back (arc);
                }
            else if (constraint->m_location.curve->TryGetLine (segment))
                {
                segments.push_back (segment);
                }
            }
        }
    }
};


struct ConstructionContext
{

bvector<CurveConstraint> &m_constraints;
ConstructionContext (bvector<CurveConstraint> &constraints) : m_constraints(constraints){}

CurveConstraintCP Deref (size_t index) const { return index < m_constraints.size () ? &m_constraints[index] : nullptr;}

bool BuildConstraintMatchTable (ConstraintMatchTable &matchTable, bool ordered = false) const   
    {
    matchTable.Clear ();
    uint32_t nextK0 = 0;
    size_t numConstraints = m_constraints.size ();
    for (uint32_t requestIndex = 0; requestIndex < numConstraints; requestIndex++)
        {
        // note requestIndex is the index to the current target type.
        // It is also the count of accepted hints --- must avoid reuse of prior hints
        CurveConstraint::Type targetType = matchTable.m_row[requestIndex].m_type;
        bool found = false;
        for (uint32_t k = nextK0; k < numConstraints; k++)
            {
            if (m_constraints[k].IsType (targetType))
                {
                if (!matchTable.FindSourceIndex (k))
                    {
                    matchTable.Enter (requestIndex, k, Deref (k));
                    found = true;
                    if (ordered)
                        nextK0 = k + 1;
                    break;
                    }
                }
            }
        if (!found)
            return false;
        }
    return true;
    }
struct ITryConstruction
    {
    virtual void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) = 0;
    };

static void TryConstruction (bvector<CurveConstraint>&constraints, bvector<ITryConstruction*> methods, bvector<ICurvePrimitivePtr> &result)
    {
    for (auto &method : methods)
        {
        method->TryConstruction (constraints, result);
        if (result.size () > 0)
            return;
        }
    return;
    }
};


static bool MapToPlane
(
bvector<DPoint3d> &points,
bvector<DSegment3d> &segments,
bvector<DEllipse3d> &arcs,
TransformR localToWorld,
TransformR worldToLocal,
bool applyToAllGeometry,
double reltol = 1.0e-10
)
    {
    bvector<DPoint3d> allPoints = points;
    for (auto &s : segments)
        {
        allPoints.push_back (s.point[0]);
        allPoints.push_back (s.point[1]);
        }

    for (auto &arc: arcs)
        {
        allPoints.push_back (arc.center);
        allPoints.push_back (arc.center + arc.vector0);
        allPoints.push_back (arc.center + arc.vector90);
        }

    Transform originWithExtentVectors;
    if (DPoint3dOps::PrincipalExtents (allPoints, originWithExtentVectors, localToWorld, worldToLocal))
        {
        double ax = originWithExtentVectors.ColumnX ().Magnitude ();
        double ay = originWithExtentVectors.ColumnY ().Magnitude ();
        double az = originWithExtentVectors.ColumnZ ().Magnitude ();
        if (az <= reltol * (ax + ay))
            {
            localToWorld.SetTranslation (allPoints[0]); // make sure the origin is a real geometry point.
            worldToLocal.InverseOf (localToWorld);      // safe incidental fixup
            if (applyToAllGeometry)
                {
                worldToLocal.Multiply (points, points);
                for (auto &segment : segments)
                    worldToLocal.Multiply (segment);
                for (auto &arc : arcs)
                    worldToLocal.Multiply (arc);
                }
            return true;
            }
        return true;
        }
    return false;
    }

static void BranchToLineTangencySolver
(
bvector<ICurvePrimitivePtr> &result,
bvector<DPoint3d> &points,
bvector<DSegment3d> &segments,
bvector<DEllipse3d> &arcs,
TransformCR localToWorld
)
    {
    static const int MaxIn = 3;
    DPoint3d centerIn[3];
    double   radiusIn[3];
    DRay3d ray[3];
    int numRay = 0;
    int numCircle = 0;
    for (auto &point : points)
        {
        if (numCircle < MaxIn)
            {
            centerIn[numCircle] = point;
            radiusIn[numCircle] = 0.0;
            numCircle++;
            }
        }
    for (auto &arc : arcs)
        {
        double r;
        if (arc.IsCircularXY (r) && numCircle < MaxIn)
            {
            centerIn[numCircle] = arc.center;
            radiusIn[numCircle] = r;
            numCircle++;
            }
        }

    for (auto &segment : segments)
        {
        if (numRay < MaxIn)
            {
            ray[numRay] = DRay3d::FromOriginAndTarget (segment.point[0], segment.point[1]);
            numRay++;
            }
        }

    if (numCircle == 2)
        {
        for (double sB = -1.0; sB < 2.0; sB += 2.0)
            {
            double rB = sB * radiusIn[1];
            for (double sA = -1.0; sA < 2.0; sA += 2.0)
                {
                double rA = sA * radiusIn[0];
                auto segment = DSegment3d::ConstructTangent_CircleCircleXY (
                            centerIn[0], rA, centerIn[1], rB);
                if (segment.IsValid ())
                    {
                    auto worldSegment = segment.Value ();
                    localToWorld.Multiply (worldSegment);
                    result.push_back (ICurvePrimitive::CreateLine (worldSegment));
                    }
                }
            }
        }
    }



struct LineConstructions
{
struct FromPointPoint : ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (CurveConstraint::Type::ThroughPoint, CurveConstraint::Type::ThroughPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            result.push_back (ICurvePrimitive::CreateLine (DSegment3d::From (matchTable.GetCurveConstraintCP(0)->Point (), matchTable.GetCurveConstraintCP(1)->Point ())));
            }
        }
    };


struct FromPointClosestApproach : ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (CurveConstraint::Type::ThroughPoint, CurveConstraint::Type::ClosestPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            CurveLocationDetail locationB;
            if (matchTable.GetCurveConstraintCP(1)->Location ().curve != nullptr
                && matchTable.GetCurveConstraintCP(1)->Location ().curve->ClosestPointBounded (
                        matchTable.GetCurveConstraintCP(0)->Point (), locationB))
                {
                result.push_back (ICurvePrimitive::CreateLine (DSegment3d::From (matchTable.GetCurveConstraintCP(0)->Point (), locationB.point)));
                }
            }
        }
    };

struct FromClosestApproachClosestApproach : ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (CurveConstraint::Type::ClosestPoint, CurveConstraint::Type::ClosestPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            CurveLocationDetail locationA, locationB;
            if (   matchTable.GetCurveConstraintCP(1)->Location ().curve != nullptr
                && matchTable.GetCurveConstraintCP(0)->Location ().curve != nullptr
                && CurveCurve::ClosestApproach (locationA, locationB,
                            const_cast<ICurvePrimitiveP>(matchTable.GetCurveConstraintCP(0)->Location ().curve),
                            const_cast<ICurvePrimitiveP>(matchTable.GetCurveConstraintCP(1)->Location ().curve)
                            )
                )
                {
                result.push_back (ICurvePrimitive::CreateLine (DSegment3d::From (locationA.point, locationB.point)));
                }
            }
        }
    };
struct FromPointPerpendicularNear : ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (CurveConstraint::Type::ThroughPoint, CurveConstraint::Type::PerpendicularNear);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            CurveLocationDetail locationA, locationB;
            if (   matchTable.GetCurveConstraintCP(1)->Location ().curve != nullptr)
                {
                DPoint3d xyz;
                double fraction = matchTable.GetCurveConstraintCP(1)->Location ().fraction;
                if (ImprovePerpendicularProjection (matchTable.GetCurveConstraintCP(1)->Location ().curve, matchTable.GetCurveConstraintCP(0)->Location ().point, fraction, xyz)
                    && DoubleOps::IsAlmostIn01 (fraction))
                    {
                    fraction = DoubleOps::ClampFraction (fraction);
                    result.push_back (ICurvePrimitive::CreateLine (DSegment3d::From (matchTable.GetCurveConstraintCP(0)->Location ().point, xyz)));
                    }
                }
            }
        }
    };

struct FromPointTangent: ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::Tangent
                );
        bvector<DPoint3d> points;
        bvector<DEllipse3d>arcs;
        bvector<DSegment3d>segments;
        if (    searcher.BuildConstraintMatchTable (matchTable))
            {
            matchTable.GetPointsSegmentsArcs(points,segments, arcs);
            Transform localToWorld, worldToLocal;
            if (MapToPlane (points, segments, arcs, localToWorld, worldToLocal, true))
                BranchToLineTangencySolver (result, points, segments, arcs, localToWorld);
            }
        }
    };

struct FromTangentTangent: ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::Tangent,
                CurveConstraint::Type::Tangent
                );
        bvector<DPoint3d> points;
        bvector<DEllipse3d>arcs;
        bvector<DSegment3d>segments;
        if (    searcher.BuildConstraintMatchTable (matchTable))
            {
            matchTable.GetPointsSegmentsArcs(points,segments, arcs);
            Transform localToWorld, worldToLocal;
            if (MapToPlane (points, segments, arcs, localToWorld, worldToLocal, true))
                BranchToLineTangencySolver (result, points, segments, arcs, localToWorld);
            }
        }
    };

};


struct CircleConstructions
{

struct FromPointPointPoint : ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::ThroughPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            result.push_back (ICurvePrimitive::CreateArc (
                    DEllipse3d::FromPointsOnArc (matchTable.GetCurveConstraintCP(0)->Point (), matchTable.GetCurveConstraintCP(1)->Point (), matchTable.GetCurveConstraintCP(2)->Point ())));
            }
        }
    };

struct FromCenterPointPoint: ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::Center,
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::ThroughPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            result.push_back (ICurvePrimitive::CreateArc (
                    DEllipse3d::FromArcCenterStartEnd (matchTable.GetCurveConstraintCP(0)->Point (), matchTable.GetCurveConstraintCP(1)->Point (), matchTable.GetCurveConstraintCP(2)->Point ())));
            }
        }
    };

struct FromCenterPointFrame: ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::Center,
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::ResultFrame);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            auto localToWorld = matchTable.GetResultFrame (2);
            Transform worldToLocal;
            if (localToWorld.IsValid () && worldToLocal.InverseOf (localToWorld.Value ()))
                {
                DPoint3d worldCenter =  matchTable.GetCurveConstraintCP(0)->Point ();
                DPoint3d localCenter = worldToLocal * worldCenter;
                DPoint3d localEdge   = worldToLocal * matchTable.GetCurveConstraintCP(1)->Point ();
                DVec3d   localVector0 = localEdge - localCenter;
                localVector0.z = 0.0;
                DVec3d   localVector90 = DVec3d::FromCCWPerpendicularXY (localVector0);
                result.push_back (ICurvePrimitive::CreateArc (
                    DEllipse3d::FromVectors (worldCenter,
                            localToWorld * localVector0,
                            localToWorld * localVector90,
                            0.0, Angle::TwoPi ())));
                }
            }
        }
    };

struct FromCenterRadiusFrame: ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::Center,
                CurveConstraint::Type::Radius,
                CurveConstraint::Type::ResultFrame);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            DPoint3d worldCenter =  matchTable.GetCurveConstraintCP(0)->Point ();
            auto radius = matchTable.GetDistance (1).Value ();
            auto localToWorld = matchTable.GetResultFrame (2).Value ();
            result.push_back (ICurvePrimitive::CreateArc (
                DEllipse3d::FromVectors (worldCenter,
                        localToWorld.GetMatrixColumn (0) * radius,
                        localToWorld.GetMatrixColumn (1) * radius,
                        0.0, Angle::TwoPi ())));
            }
        }
    };


struct FromPointDirectionPoint: ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::PointAndDirection,
                CurveConstraint::Type::ThroughPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            DEllipse3d arc;
            DRay3d ray = matchTable.GetCurveConstraintCP(0)->PointAndDirection ();
            if (arc.InitArcFromPointTangentPoint (ray.origin, ray.direction, matchTable.GetCurveConstraintCP(1)->Point ()))
                result.push_back (ICurvePrimitive::CreateArc (arc));
            }
        }
    };


static void LoadArcs (bvector<ICurvePrimitivePtr> &result, DPoint3dCP center, double *radius, int numCircle, TransformCR localToWorld)
    {
    for (int i = 0; i < numCircle; i++)
        {
        DEllipse3d circle = DEllipse3d::FromCenterRadiusXY (center[i], radius[i]);
        localToWorld.Multiply (circle);
        result.push_back (ICurvePrimitive::CreateArc (circle));
        }
    }


static void BranchToCircleTangencySolver
(
bvector<ICurvePrimitivePtr> &result,
bvector<DPoint3d> &points,
bvector<DSegment3d> &segments,
bvector<DEllipse3d> &arcs,
TransformCR localToWorld
)
    {
    static const int MaxIn = 3;
    DPoint3d centerIn[3];
    double   radiusIn[3];
    DRay3d ray[3];
    int numRay = 0;
    int numCircle = 0;
    for (auto &point : points)
        {
        if (numCircle < MaxIn)
            {
            centerIn[numCircle] = point;
            radiusIn[numCircle] = 0.0;
            numCircle++;
            }
        }
    for (auto &arc : arcs)
        {
        double r;
        if (arc.IsCircularXY (r) && numCircle < MaxIn)
            {
            centerIn[numCircle] = arc.center;
            radiusIn[numCircle] = r;
            numCircle++;
            }
        }

    for (auto &segment : segments)
        {
        if (numRay < MaxIn)
            {
            ray[numRay] = DRay3d::FromOriginAndTarget (segment.point[0], segment.point[1]);
            numRay++;
            }
        }

    static const int MaxOut = 8;
    DPoint3d tangentPointA[MaxOut];
    DPoint3d tangentPointB[MaxOut];
    DPoint3d tangentPointC[MaxOut];
    DPoint3d centerOut[MaxOut];
    double   radiusOut[MaxOut];
    int numOut;
    
    if (numCircle == 3)
        {
        bsiGeom_circleTTTCircleConstruction
            (
            centerOut, radiusOut, &numOut, MaxOut,
            centerIn, radiusIn
            );
        LoadArcs (result, centerOut, radiusOut, numOut, localToWorld);
        }
    else if (numCircle == 2 && numRay == 1)
        {
        bsiGeom_circleTTTCircleCircleLineConstruction
            (
            centerOut, radiusOut, &numOut, MaxOut,
            centerIn, radiusIn,
            &ray[0].origin, &ray[0].direction);
        LoadArcs (result, centerOut, radiusOut, numOut, localToWorld);
        }
    else if (numCircle == 1 && numRay == 2)
        {
        bsiGeom_circleTTTLineLineCircleConstruction
            (
            centerOut, radiusOut,
            tangentPointA, tangentPointB, tangentPointC,
            numOut, MaxOut,
            ray[0].origin, ray[0].direction,
            ray[1].origin, ray[1].direction,
            centerIn[0], radiusIn[0]
            );
        LoadArcs (result, centerOut, radiusOut, numOut, localToWorld);
        }
    else if (numCircle == 0 && numRay == 3)
        {
        bsiGeom_circleTTTLineLineLineConstruction
            (
            centerOut, radiusOut,
            tangentPointA, tangentPointB, tangentPointC,
            numOut, MaxOut,
            ray[0].origin, ray[0].direction,
            ray[1].origin, ray[1].direction,
            ray[2].origin, ray[2].direction
            );
        LoadArcs (result, centerOut, radiusOut, numOut, localToWorld);
        }
    }

struct FromPointPointTangent: ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::Tangent
                );
        bvector<DPoint3d> points;
        bvector<DEllipse3d>arcs;
        bvector<DSegment3d>segments;
        if (    searcher.BuildConstraintMatchTable (matchTable))
            {
            matchTable.GetPointsSegmentsArcs(points,segments, arcs);
            Transform localToWorld, worldToLocal;
            if (MapToPlane (points, segments, arcs, localToWorld, worldToLocal, true))
                BranchToCircleTangencySolver (result, points, segments, arcs, localToWorld);
            }
        }
    };


struct FromPointTangentTangent: ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::Tangent,
                CurveConstraint::Type::Tangent
                );
        bvector<DPoint3d> points;
        bvector<DEllipse3d>arcs;
        bvector<DSegment3d>segments;
        if (    searcher.BuildConstraintMatchTable (matchTable))
            {
            matchTable.GetPointsSegmentsArcs(points,segments, arcs);
            Transform localToWorld, worldToLocal;
            if (MapToPlane (points, segments, arcs, localToWorld, worldToLocal, true))
                {
                BranchToCircleTangencySolver (result, points, segments, arcs, localToWorld);
                }
            }
        }
    };



struct FromTangentTangentTangent: ConstructionContext::ITryConstruction
    {
    void TryConstruction (ConstructionContext const &searcher, bvector<ICurvePrimitivePtr> &result) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::Tangent,
                CurveConstraint::Type::Tangent,
                CurveConstraint::Type::Tangent
                );
        bvector<DPoint3d> points;
        bvector<DEllipse3d>arcs;
        bvector<DSegment3d>segments;
        if (    searcher.BuildConstraintMatchTable (matchTable))
            {
            matchTable.GetPointsSegmentsArcs(points,segments, arcs);
            Transform localToWorld, worldToLocal;
            if (MapToPlane (points, segments, arcs, localToWorld, worldToLocal, true))
                {
                BranchToCircleTangencySolver (result, points, segments, arcs, localToWorld);
                }
            }
        }
    };


};



static bvector<ConstructionContext::ITryConstruction *> s_lineBuilders =
bvector<ConstructionContext::ITryConstruction *>
    {
    new LineConstructions::FromPointPoint (),
    new LineConstructions::FromPointClosestApproach (),
    new LineConstructions::FromClosestApproachClosestApproach (),
    new LineConstructions::FromPointPerpendicularNear (),
    new LineConstructions::FromTangentTangent (),
    new LineConstructions::FromPointTangent ()
    };

static bvector<ConstructionContext::ITryConstruction *> s_circleBuilders =
bvector<ConstructionContext::ITryConstruction *>
    {
    new CircleConstructions::FromPointPointPoint (),
    new CircleConstructions::FromCenterPointPoint (),
    new CircleConstructions::FromPointDirectionPoint (),
    new CircleConstructions::FromPointPointTangent (),
    new CircleConstructions::FromPointTangentTangent (),
    new CircleConstructions::FromTangentTangentTangent (),
    new CircleConstructions::FromCenterPointFrame (),
    new CircleConstructions::FromCenterRadiusFrame ()
    
    };

void ConstrainedConstruction::ConstructLines (bvector<CurveConstraint> &constraints, bvector<ICurvePrimitivePtr> &result)
    {
    result.clear ();
    ConstructionContext::TryConstruction  (constraints, s_lineBuilders, result);
    }

void ConstrainedConstruction::ConstructCircularArcs (bvector<CurveConstraint> &constraints, bvector<ICurvePrimitivePtr> &result)
    {
    result.clear ();
    ConstructionContext::TryConstruction  (constraints, s_circleBuilders, result);
    }



END_BENTLEY_GEOMETRY_NAMESPACE
