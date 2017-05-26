#include "testHarness.h"
struct CurveConstruction;
struct CurveConstraint;
typedef struct CurveConstraint const *CurveConstraintCP;
typedef struct CurveConstraint *CurveConstraintP;
typedef struct CurveConstraint &CurveConstraintCR;
typedef struct CurveConstraint &ConstraintR;


struct CurveConstraint
{
enum class Type
    {
    ThroughPoint,
    PointAndDirection,
    Center,
    PerpendicularNear,
    ClosestPoint,
    Tangent
    };
Type m_type;
CurveLocationDetail m_location;
ICurvePrimitivePtr m_curve; // redundant of the CurveLocationDetail -- but as a Ptr so the lifecycle management works
DRay3d m_ray;
private:
CurveConstraint (Type type, CurveLocationDetailCR detail) :
    m_type (type),
    m_location (detail),
    m_curve ((ICurvePrimitiveP)detail.curve),
    m_ray (DRay3d::FromOriginAndVector (detail.point, DVec3d::UnitX ()))
    {

    }

CurveConstraint (Type type, CurveLocationDetailCR detail, DRay3dCR ray) :
    m_type (type),
    m_location (detail),
    m_curve ((ICurvePrimitiveP)detail.curve),
    m_ray (ray)
    {

    }

public:

bool IsType (CurveConstraint::Type t) const { return t == m_type;}
DPoint3d Point () const{return m_location.point;}
DRay3d PointAndDirection () const {return m_ray;}

static CurveConstraint CreateThroughPoint (DPoint3dCR point)
    {
    return CurveConstraint (Type::ThroughPoint, CurveLocationDetail (nullptr, 0.0, point));
    }

static CurveConstraint CreatePointAndDirection (DPoint3dCR point, DVec3dCR direction)
    {
    auto line = ICurvePrimitive::CreateLine (DSegment3d::From (point, point + direction));
    return CurveConstraint (Type::PointAndDirection, CurveLocationDetail (
                    line.get (), 0.0, point),
                    DRay3d::FromOriginAndVector (point, direction));
    }

static CurveConstraint CreateCenter (DPoint3dCR point)
    {
    return CurveConstraint (Type::Center, CurveLocationDetail (nullptr, 0.0, point));
    }

static CurveConstraint CreateClosestPoint (ICurvePrimitiveCP curve)
    {
    DPoint3d xyz;
    curve->FractionToPoint (0.0, xyz);
    return CurveConstraint (Type::ClosestPoint, CurveLocationDetail (curve, 0.0, xyz));
    }

static CurveConstraint CreatePerpendicularNear (ICurvePrimitiveCP curve, double fraction)
    {
    DPoint3d xyz;
    curve->FractionToPoint (fraction, xyz);
    return CurveConstraint (Type::PerpendicularNear, CurveLocationDetail (curve, fraction, xyz));
    }
};

struct ConstrainedConstruction
{
static ICurvePrimitivePtr ConstructLines (bvector<CurveConstraint> &constraints);
static ICurvePrimitivePtr ConstructArcs  (bvector<CurveConstraint> &constraints);
};

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
    virtual ICurvePrimitivePtr TryConstruction (ConstructionContext const &) = 0;
    };

static ICurvePrimitivePtr TryConstruction (bvector<CurveConstraint>&constraints, bvector<ITryConstruction*> methods)
    {
    for (auto &method : methods)
        {
        auto cp = method->TryConstruction (constraints);
        if (cp.IsValid ())
            return cp;
        }
    return nullptr;
    }
};



struct LineConstructions
{
struct FromPointPoint : ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintMatchTable matchTable (CurveConstraint::Type::ThroughPoint, CurveConstraint::Type::ThroughPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            return ICurvePrimitive::CreateLine (DSegment3d::From (matchTable.GetCurveConstraintCP(0)->Point (), matchTable.GetCurveConstraintCP(1)->Point ()));
            }
        return nullptr;
        }
    };


struct FromPointClosestApproach : ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintMatchTable matchTable (CurveConstraint::Type::ThroughPoint, CurveConstraint::Type::ClosestPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            CurveLocationDetail locationB;
            if (matchTable.GetCurveConstraintCP(1)->m_location.curve != nullptr
                && matchTable.GetCurveConstraintCP(1)->m_location.curve->ClosestPointBounded (
                        matchTable.GetCurveConstraintCP(0)->Point (), locationB))
                {
                return ICurvePrimitive::CreateLine (DSegment3d::From (matchTable.GetCurveConstraintCP(0)->Point (), locationB.point));
                }
            }
        return nullptr;
        }
    };

struct FromClosestApproachClosestApproach : ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintMatchTable matchTable (CurveConstraint::Type::ClosestPoint, CurveConstraint::Type::ClosestPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            CurveLocationDetail locationA, locationB;
            if (   matchTable.GetCurveConstraintCP(1)->m_location.curve != nullptr
                && matchTable.GetCurveConstraintCP(0)->m_location.curve != nullptr
                && CurveCurve::ClosestApproach (locationA, locationB,
                            (ICurvePrimitiveP)matchTable.GetCurveConstraintCP(0)->m_location.curve,
                            (ICurvePrimitiveP)matchTable.GetCurveConstraintCP(1)->m_location.curve
                            )
                )
                {
                return ICurvePrimitive::CreateLine (DSegment3d::From (locationA.point, locationB.point));
                }
            }
        return nullptr;
        }
    };
struct FromPointPerpendicularNear : ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintMatchTable matchTable (CurveConstraint::Type::ThroughPoint, CurveConstraint::Type::PerpendicularNear);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            CurveLocationDetail locationA, locationB;
            if (   matchTable.GetCurveConstraintCP(1)->m_location.curve != nullptr)
                {
                DPoint3d xyz;
                double fraction = matchTable.GetCurveConstraintCP(1)->m_location.fraction;
                if (ImprovePerpendicularProjection (matchTable.GetCurveConstraintCP(1)->m_location.curve, matchTable.GetCurveConstraintCP(0)->m_location.point, fraction, xyz)
                    && DoubleOps::IsAlmostIn01 (fraction))
                    {
                    fraction = DoubleOps::ClampFraction (fraction);
                    return ICurvePrimitive::CreateLine (DSegment3d::From (matchTable.GetCurveConstraintCP(0)->m_location.point, xyz));
                    }
                }
            }
        return nullptr;
        }
    };
};


struct CircleConstructions
{

struct FromPointPointPoint : ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::ThroughPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            return ICurvePrimitive::CreateArc (
                    DEllipse3d::FromPointsOnArc (matchTable.GetCurveConstraintCP(0)->Point (), matchTable.GetCurveConstraintCP(1)->Point (), matchTable.GetCurveConstraintCP(2)->Point ()));
            }
        return nullptr;
        }
    };

struct FromCenterPointPoint: ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::Center,
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::ThroughPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            return ICurvePrimitive::CreateArc (
                    DEllipse3d::FromArcCenterStartEnd (matchTable.GetCurveConstraintCP(0)->Point (), matchTable.GetCurveConstraintCP(1)->Point (), matchTable.GetCurveConstraintCP(2)->Point ()));
            }
        return nullptr;
        }
    };

struct FromPointDirectionPoint: ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintMatchTable matchTable (
                CurveConstraint::Type::PointAndDirection,
                CurveConstraint::Type::ThroughPoint);
        if (searcher.BuildConstraintMatchTable (matchTable))
            {
            DEllipse3d arc;
            DRay3d ray = matchTable.GetCurveConstraintCP(0)->PointAndDirection ();
            if (arc.InitArcFromPointTangentPoint (ray.origin, ray.direction, matchTable.GetCurveConstraintCP(1)->Point ()))
                return ICurvePrimitive::CreateArc (arc);
            }
        return nullptr;
        }
    };


};



void SaveHints (bvector<CurveConstraint> &constraints)
    {
    static double s_markerSize = 0.25;
    for (auto &constraint : constraints)
        {
        switch (constraint.m_type)
            {
            case CurveConstraint::Type::ThroughPoint:
                {
                Check::SaveTransformedMarkers (bvector<DPoint3d>{constraint.m_location.point}, s_markerSize);
                }
                break;
            case CurveConstraint::Type::Center:
                {
                Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (constraint.m_location.point, s_markerSize * 0.25));
                }
                break;
            case CurveConstraint::Type::ClosestPoint:
                {
                Check::SaveTransformed (*constraint.m_location.curve);
                }
                break;
            case CurveConstraint::Type::PointAndDirection:
                {
                Check::SaveTransformed (*constraint.m_location.curve);
                Check::SaveTransformedMarkers (bvector<DPoint3d>{constraint.m_location.point}, s_markerSize);
                }
                break;
            case CurveConstraint::Type::PerpendicularNear:
                {
                Check::SaveTransformed (*constraint.m_location.curve);
                Check::SaveTransformedMarkers (bvector<DPoint3d>{constraint.m_location.point}, s_markerSize);
                }
                break;
            }
        }
    }

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


TEST (Construction,HelloLines)
    {

    DPoint3d point0 = DPoint3d::From (1,1,1);
    DPoint3d point1 = DPoint3d::From (5,2,-1);
    ICurvePrimitivePtr line0 = ICurvePrimitive::CreateLine (DSegment3d::From (3,1,0, 1,8,1));
    ICurvePrimitivePtr  arc0 = ICurvePrimitive::CreateArc (DEllipse3d::From (2,2,0,   1,0,0,  0,2,0, 0.0, Angle::DegreesToRadians (280)));
    ICurvePrimitivePtr  bsp0 = ICurvePrimitive::CreateBsplineCurve (
            MSBsplineCurve::CreateFromPolesAndOrder (
                bvector<DPoint3d> {
                    DPoint3d::From (2,0, -1),
                    DPoint3d::From (2,5, 2),
                    DPoint3d::From (5,8, 3),
                    DPoint3d::From (8,8)
                    },
                nullptr, nullptr, 3, false, true));
                
    double a = 10.0;
    auto hPoint0 = CurveConstraint::CreateThroughPoint (point0);
    auto hPoint1 = CurveConstraint::CreateThroughPoint (point1);
    auto hLine0 = CurveConstraint::CreateClosestPoint (line0.get ());
    auto hArc0 = CurveConstraint::CreateClosestPoint (arc0.get ());
    auto hBsp0 = CurveConstraint::CreateClosestPoint (bsp0.get ());
    bvector<CurveConstraint> allHints {hPoint0, hPoint1, hLine0, hArc0, hBsp0};

    for (size_t i1 = 0; i1 < allHints.size (); i1++)
        {
        SaveAndRestoreCheckTransform shifter (0,a,0);
        for (size_t i0 = 0; i0 < allHints.size (); i0++)
            {
            SaveAndRestoreCheckTransform shifter (a,0,0);
            if (i0 != i1)
                {
                bvector<CurveConstraint> constraints {allHints[i0], allHints[i1]};
                SaveHints (constraints);
                auto cp0 = ConstructionContext::TryConstruction  (constraints, s_lineBuilders);
                if (cp0.IsValid ())
                    Check::SaveTransformed (*cp0);
                }
            }
        }
    Check::ClearGeometry ("Construction.HelloLines");
    }

TEST (Construction,PointPerpendicularNear)
    {


    DPoint3d point0 = DPoint3d::From (0.1, 0.1, 0.1);
    ICurvePrimitivePtr  arc0 = ICurvePrimitive::CreateArc (DEllipse3d::From (0,0,0,
                                3,0,0,
                                0,8,0,
                                Angle::DegreesToRadians (-20),
                                Angle::DegreesToRadians (235)));
                
    double a = 10.0;
    auto hPoint0 = CurveConstraint::CreateThroughPoint (point0);

    double df = 1.0 / 16.0;
    for (double f = 0.0; f < 1.0; f += df)
        {
        SaveAndRestoreCheckTransform shifter (a,0,0);
        bvector<CurveConstraint> constraints;
        constraints.push_back (hPoint0);
        constraints.push_back (CurveConstraint::CreatePerpendicularNear (arc0.get (), f));
        SaveHints (constraints);
        auto cp0 = ConstructionContext::TryConstruction  (constraints, s_lineBuilders);
        if (cp0.IsValid ())
            Check::SaveTransformed (*cp0);
        }
    Check::ClearGeometry ("Construction.PointPerpendicularNear");
    }

TEST (Construction,HelloCircles)
    {
    double a = 10.0;

    auto hPoint0 = CurveConstraint::CreateThroughPoint (DPoint3d::From (0.1, 0.1));
    auto hPoint1 = CurveConstraint::CreateThroughPoint (DPoint3d::From (1,0));
    auto hPoint2 = CurveConstraint::CreateThroughPoint (DPoint3d::From (0,2));
    auto hPointTangent0 = CurveConstraint::CreatePointAndDirection (DPoint3d::From (0.1, 0.1), DVec3d::From (1,0,0));

    auto hCenter0 = CurveConstraint::CreateCenter (DPoint3d::From (1,1));
    bvector<CurveConstraint> constraints;
    constraints.push_back (hPoint0);
    constraints.push_back (hPoint1);
    constraints.push_back (hPoint2);
    SaveHints (constraints);
    auto cp0 = ConstructionContext::TryConstruction (constraints, s_circleBuilders);
    if (cp0.IsValid ())
        Check::SaveTransformed (*cp0);

    Check::Shift (a,0,0);

    constraints.clear ();
    constraints.push_back (hPoint0);
    constraints.push_back (hCenter0);
    constraints.push_back (hPoint2);
    SaveHints (constraints);
    cp0 = ConstructionContext::TryConstruction  (constraints, s_circleBuilders);
    if (cp0.IsValid ())
        Check::SaveTransformed (*cp0);

    Check::Shift (a,0,0);

    constraints.clear ();
    constraints.push_back (hPointTangent0);
    constraints.push_back (hPoint2);
    SaveHints (constraints);
    cp0 = ConstructionContext::TryConstruction  (constraints, s_circleBuilders);
    if (cp0.IsValid ())
        Check::SaveTransformed (*cp0);



    Check::ClearGeometry ("Construction.HelloCircles");
    }

