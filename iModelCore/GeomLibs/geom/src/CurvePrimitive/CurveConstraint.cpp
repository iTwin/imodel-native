/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/CurveConstraint.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
CurveConstraint::CurveConstraint (Type type, CurveLocationDetailCR detail) :
    m_type (type),
    m_location (detail),
    m_curve ((ICurvePrimitiveP)detail.curve),
    m_ray (DRay3d::FromOriginAndVector (detail.point, DVec3d::UnitX ()))
    {

    }

CurveConstraint::CurveConstraint (Type type, CurveLocationDetailCR detail, DRay3dCR ray) :
    m_type (type),
    m_location (detail),
    m_curve ((ICurvePrimitiveP)detail.curve),
    m_ray (ray)
    {

    }


bool CurveConstraint::IsType (CurveConstraint::Type t) const { return t == m_type;}
DPoint3d CurveConstraint::Point () const{return m_location.point;}
DRay3d CurveConstraint::PointAndDirection () const {return m_ray;}
CurveConstraint::Type CurveConstraint::GetType () const {return m_type;}

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

CurveConstraint CurveConstraint::CreateCenter (DPoint3dCR point)
    {
    return CurveConstraint (Type::Center, CurveLocationDetail (nullptr, 0.0, point));
    }

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
    

struct ConstraintMatchTable
{
friend struct ConstructionContext;
private:
static const int s_maxTableIndex = 6;

uint32_t m_numRequested;
uint32_t m_numMatch;
CurveConstraint::Type m_type[s_maxTableIndex];
uint32_t m_sourceIndex[s_maxTableIndex];
CurveConstraintCP m_constraint[s_maxTableIndex];
public:

ConstraintMatchTable () : m_numRequested(0), m_numMatch (0) {}

ConstraintMatchTable (CurveConstraint::Type type0) : m_numRequested(1), m_numMatch (0)
    {
    m_type[0] = type0;
    }

ConstraintMatchTable (CurveConstraint::Type type0, CurveConstraint::Type type1 ) : m_numRequested(2), m_numMatch (0)
    {
    m_type[0] = type0;
    m_type[1] = type1;
    }

ConstraintMatchTable (CurveConstraint::Type type0, CurveConstraint::Type type1, CurveConstraint::Type type2) : m_numRequested(3), m_numMatch (0)
    {
    m_type[0] = type0;
    m_type[1] = type1;
    m_type[2] = type2;
    }

bool FindSourceIndex (uint32_t target)
    {
    for (uint32_t i = 0; i < m_numMatch; i++)
        {
        if (m_sourceIndex[i] == target)
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
        m_sourceIndex[index] = hintIndex;
        m_constraint[index] = constraint;
        m_numMatch++;
        }
    }

CurveConstraintCP GetCurveConstraintCP (uint32_t tableIndex){ return tableIndex < s_maxTableIndex ? m_constraint[tableIndex] : nullptr;}
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
        CurveConstraint::Type targetType = matchTable.m_type[requestIndex];
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
                            (ICurvePrimitiveP)matchTable.GetCurveConstraintCP(0)->Location ().curve,
                            (ICurvePrimitiveP)matchTable.GetCurveConstraintCP(1)->Location ().curve
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
};



static bvector<ConstructionContext::ITryConstruction *> s_lineBuilders =
bvector<ConstructionContext::ITryConstruction *>
    {
    new LineConstructions::FromPointPoint (),
    new LineConstructions::FromPointClosestApproach (),
    new LineConstructions::FromClosestApproachClosestApproach (),
    new LineConstructions::FromPointPerpendicularNear ()
    };

static bvector<ConstructionContext::ITryConstruction *> s_circleBuilders =
bvector<ConstructionContext::ITryConstruction *>
    {
    new CircleConstructions::FromPointPointPoint (),
    new CircleConstructions::FromCenterPointPoint (),
    new CircleConstructions::FromPointDirectionPoint ()
    };

void ConstrainedConstruction::ConstructLines (bvector<CurveConstraint> &constraints, bvector<ICurvePrimitivePtr> &result)
    {
    result.clear ();
    ConstructionContext::TryConstruction  (constraints, s_lineBuilders, result);
    }

void ConstrainedConstruction::ConstructCircularArcs (bvector<CurveConstraint> &constraints, bvector<ICurvePrimitivePtr> &result)
    {
    ConstructionContext::TryConstruction  (constraints, s_circleBuilders, result);
    }



END_BENTLEY_GEOMETRY_NAMESPACE
