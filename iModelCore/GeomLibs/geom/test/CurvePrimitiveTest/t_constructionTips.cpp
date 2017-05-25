#include "testHarness.h"
struct CurveConstruction;
struct CurveConstraint;
typedef struct CurveConstraint const *ConstraintCP;
typedef struct CurveConstraint *ConstraintP;
typedef struct CurveConstraint &ConstraintCR;
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

bool IsType (CurveConstraint::Type t) { return t == m_type;}
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


struct ConstructionContext
{
bvector<CurveConstraint> &m_constraints;
ConstructionContext (bvector<CurveConstraint> &constraints) : m_constraints(constraints){}

ConstraintCP Deref (size_t index) const { return index < m_constraints.size () ? &m_constraints[index] : nullptr;}

size_t FindHint
(
CurveConstraint::Type type,
size_t firstIndex = 0,
size_t excludeIndexA = SIZE_MAX,
size_t excludeIndexB = SIZE_MAX
) const
    {
    for (size_t i = firstIndex; i < m_constraints.size (); i++)
        {
        if (i != excludeIndexA && i != excludeIndexB && m_constraints[i].IsType (type))
            return i;
        }
    return SIZE_MAX;
    }

bool Get2UnorderedHints
(
CurveConstraint::Type typeA,
CurveConstraint::Type typeB,
ConstraintCP &hintA,
ConstraintCP &hintB
) const
    {
    size_t indexA = FindHint (typeA, 0);
    size_t indexB = FindHint (typeB, 0, indexA);
    hintA = Deref (indexA);
    hintB = Deref (indexB);
    return hintA != nullptr && hintB != nullptr;
    }

bool Get3UnorderedHints
(
CurveConstraint::Type typeA,
CurveConstraint::Type typeB,
CurveConstraint::Type typeC,
ConstraintCP &hintA,
ConstraintCP &hintB,
ConstraintCP &hintC
) const
    {
    size_t indexA = FindHint (typeA, 0);
    size_t indexB = FindHint (typeB, 0, indexA);
    size_t indexC = FindHint (typeC, 0, indexA, indexB);
    hintA = Deref (indexA);
    hintB = Deref (indexB);
    hintC = Deref (indexC);
    return hintA != nullptr && hintB != nullptr && hintC != nullptr;
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
        ConstraintCP hintA, hintB;
        if (searcher.Get2UnorderedHints (CurveConstraint::Type::ThroughPoint, CurveConstraint::Type::ThroughPoint,
                hintA, hintB))
            {
            return ICurvePrimitive::CreateLine (DSegment3d::From (hintA->Point (), hintB->Point ()));
            }
        return nullptr;
        }
    };


struct FromPointClosestApproach : ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintCP hintA, hintB;
        if (searcher.Get2UnorderedHints (CurveConstraint::Type::ThroughPoint, CurveConstraint::Type::ClosestPoint,
                hintA, hintB))
            {
            CurveLocationDetail locationB;
            if (hintB->m_location.curve != nullptr
                && hintB->m_location.curve->ClosestPointBounded (hintA->Point (), locationB))
                {
                return ICurvePrimitive::CreateLine (DSegment3d::From (hintA->Point (), locationB.point));
                }
            }
        return nullptr;
        }
    };

struct FromClosestApproachClosestApproach : ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintCP hintA, hintB;
        if (searcher.Get2UnorderedHints (CurveConstraint::Type::ClosestPoint, CurveConstraint::Type::ClosestPoint,
                hintA, hintB))
            {
            CurveLocationDetail locationA, locationB;
            if (   hintB->m_location.curve != nullptr
                && hintA->m_location.curve != nullptr
                && CurveCurve::ClosestApproach (locationA, locationB,
                            (ICurvePrimitiveP)hintA->m_location.curve,
                            (ICurvePrimitiveP)hintB->m_location.curve
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
        ConstraintCP hintA, hintB;
        if (searcher.Get2UnorderedHints (
                CurveConstraint::Type::ThroughPoint,
                CurveConstraint::Type::PerpendicularNear,
                hintA, hintB))
            {
            CurveLocationDetail locationA, locationB;
            if (   hintB->m_location.curve != nullptr)
                {
                DPoint3d xyz;
                double fraction = hintB->m_location.fraction;
                if (ImprovePerpendicularProjection (hintB->m_location.curve, hintA->m_location.point, fraction, xyz)
                    && DoubleOps::IsAlmostIn01 (fraction))
                    {
                    fraction = DoubleOps::ClampFraction (fraction);
                    return ICurvePrimitive::CreateLine (DSegment3d::From (hintA->m_location.point, xyz));
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
        ConstraintCP hintA, hintB, hintC;
        if (searcher.Get3UnorderedHints (
                    CurveConstraint::Type::ThroughPoint,
                    CurveConstraint::Type::ThroughPoint,
                    CurveConstraint::Type::ThroughPoint,
                    hintA, hintB, hintC))
            {
            return ICurvePrimitive::CreateArc (
                    DEllipse3d::FromPointsOnArc (hintA->Point (), hintB->Point (), hintC->Point ()));
            }
        return nullptr;
        }
    };

struct FromCenterPointPoint: ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintCP hintA, hintB, hintC;
        if (searcher.Get3UnorderedHints (
                    CurveConstraint::Type::Center,
                    CurveConstraint::Type::ThroughPoint,
                    CurveConstraint::Type::ThroughPoint,
                    hintA, hintB, hintC))
            {
            return ICurvePrimitive::CreateArc (
                    DEllipse3d::FromArcCenterStartEnd (hintA->Point (), hintB->Point (), hintC->Point ()));
            }
        return nullptr;
        }
    };

struct FromPointDirectionPoint: ConstructionContext::ITryConstruction
    {
    ICurvePrimitivePtr TryConstruction (ConstructionContext const &searcher) override
        {
        ConstraintCP hintA, hintB;
        if (searcher.Get2UnorderedHints (CurveConstraint::Type::PointAndDirection, CurveConstraint::Type::ThroughPoint,
                hintA, hintB))
            {
            DEllipse3d arc;
            DRay3d ray = hintA->PointAndDirection ();
            if (arc.InitArcFromPointTangentPoint (ray.origin, ray.direction, hintB->Point ()))
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


TEST (Construction,Hello)
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
    Check::ClearGeometry ("Construction.Hello");
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

