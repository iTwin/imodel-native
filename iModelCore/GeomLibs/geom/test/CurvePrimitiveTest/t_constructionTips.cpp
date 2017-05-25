#include "testHarness.h"

struct ConstructionHint;
typedef struct ConstructionHint const *ConstructionHintCP;
typedef struct ConstructionHint *ConstructionHintP;
typedef struct ConstructionHint &ConstructionHintCR;
typedef struct ConstructionHint &ConstructionHintR;

struct ConstructionHint
{
enum class Type
    {
    ThroughPoint,
    Center,
    PerpendicularNear,
    ClosestPoint,
    Tangent
    };
Type m_type;
CurveLocationDetail m_location;
ICurvePrimitivePtr m_curve; // redundant of the CurveLocationDetail -- but as a Ptr so the lifecycle management works
private:
ConstructionHint (Type type, CurveLocationDetailCR detail) :
    m_type (type),
    m_location (detail),
    m_curve ((ICurvePrimitiveP)detail.curve)
    {

    }
public:

bool IsType (ConstructionHint::Type t) { return t == m_type;}
DPoint3d Point (){return m_location.point;}

static ConstructionHint CreateThroughPoint (DPoint3dCR point)
    {
    return ConstructionHint (Type::ThroughPoint, CurveLocationDetail (nullptr, 0.0, point));
    }

static ConstructionHint CreateCenter (DPoint3dCR point)
    {
    return ConstructionHint (Type::Center, CurveLocationDetail (nullptr, 0.0, point));
    }

static ConstructionHint CreateClosestPoint (ICurvePrimitiveCP curve)
    {
    DPoint3d xyz;
    curve->FractionToPoint (0.0, xyz);
    return ConstructionHint (Type::ClosestPoint, CurveLocationDetail (curve, 0.0, xyz));
    }

static ConstructionHint CreatePerpendicularNear (ICurvePrimitiveCP curve, double fraction)
    {
    DPoint3d xyz;
    curve->FractionToPoint (fraction, xyz);
    return ConstructionHint (Type::PerpendicularNear, CurveLocationDetail (curve, fraction, xyz));
    }
};


struct IConstructFrom2Hints
{
virtual ICurvePrimitivePtr DoConstruction (ConstructionHintCR hintA, ConstructionHintCR hintB) = 0;
};

struct IConstructFrom3Hints
{
virtual ICurvePrimitivePtr DoConstruction (ConstructionHintCR hintA, ConstructionHintCR hintB, ConstructionHintCR hintC) = 0;
};


struct LineConstructions
{
struct FromPointPoint : IConstructFrom2Hints
    {
    ICurvePrimitivePtr DoConstruction (ConstructionHintCR hintA, ConstructionHintCR hintB) override
        {
        if (hintA.IsType (ConstructionHint::Type::ThroughPoint)
            && hintB.IsType (ConstructionHint::Type::ThroughPoint))
            {
            return ICurvePrimitive::CreateLine (DSegment3d::From (hintA.Point (), hintB.Point ()));
            }
        return nullptr;
        }
    };

struct FromPointClosestApproach : IConstructFrom2Hints
    {
    ICurvePrimitivePtr DoConstruction (ConstructionHintCR hintA, ConstructionHintCR hintB) override
        {
        if (hintA.IsType (ConstructionHint::Type::ThroughPoint)
            && hintB.IsType (ConstructionHint::Type::ClosestPoint)
            )
            {
            CurveLocationDetail locationB;
            if (hintB.m_location.curve != nullptr
                && hintB.m_location.curve->ClosestPointBounded (hintA.m_location.point, locationB))
                {
                return ICurvePrimitive::CreateLine (DSegment3d::From (hintA.Point (), locationB.point));
                }
            }
        return nullptr;
        }
    };

struct FromClosestApproachClosestApproach : IConstructFrom2Hints
    {
    ICurvePrimitivePtr DoConstruction (ConstructionHintCR hintA, ConstructionHintCR hintB) override
        {
        if (hintA.IsType (ConstructionHint::Type::ClosestPoint)
            && hintB.IsType (ConstructionHint::Type::ClosestPoint)
            )
            {
            CurveLocationDetail locationA, locationB;
            if (   hintB.m_location.curve != nullptr
                && hintA.m_location.curve != nullptr
                && CurveCurve::ClosestApproach (locationA, locationB, (ICurvePrimitiveP)hintA.m_location.curve, (ICurvePrimitiveP)hintB.m_location.curve)
                )
                {
                return ICurvePrimitive::CreateLine (DSegment3d::From (locationA.point, locationB.point));
                }
            }
        return nullptr;
        }
    };
struct FromPointPerpendicularNear : IConstructFrom2Hints
    {
    ICurvePrimitivePtr DoConstruction (ConstructionHintCR hintA, ConstructionHintCR hintB) override
        {
        if (hintA.IsType (ConstructionHint::Type::ThroughPoint)
            && hintB.IsType (ConstructionHint::Type::PerpendicularNear)
            )
            {
            CurveLocationDetail locationA, locationB;
            if (   hintB.m_location.curve != nullptr)
                {
                DPoint3d xyz;
                double fraction = hintB.m_location.fraction;
                if (ImprovePerpendicularProjection (hintB.m_location.curve, hintA.m_location.point, fraction, xyz)
                    && DoubleOps::IsAlmostIn01 (fraction))
                    {
                    fraction = DoubleOps::ClampFraction (fraction);
                    return ICurvePrimitive::CreateLine (DSegment3d::From (hintA.m_location.point, xyz));
                    }
                }
            }
        return nullptr;
        }
    };

};


struct CircleConstructions
{
struct FromPointPointPoint : IConstructFrom3Hints
    {
    ICurvePrimitivePtr DoConstruction (ConstructionHintCR hintA, ConstructionHintCR hintB, ConstructionHintCR hintC) override
        {
        if (hintA.IsType (ConstructionHint::Type::ThroughPoint)
            && hintB.IsType (ConstructionHint::Type::ThroughPoint)
            && hintC.IsType (ConstructionHint::Type::ThroughPoint))
            {
            return ICurvePrimitive::CreateArc (
                    DEllipse3d::FromPointsOnArc (hintA.Point (), hintB.Point (), hintC.Point ()));
            }
        return nullptr;
        }
    };

struct FromCenterPointPoint: IConstructFrom3Hints
    {
    ICurvePrimitivePtr DoConstruction (ConstructionHintCR hintA, ConstructionHintCR hintB, ConstructionHintCR hintC) override
        {
        if (hintA.IsType (ConstructionHint::Type::Center)
            && hintB.IsType (ConstructionHint::Type::ThroughPoint)
            && hintC.IsType (ConstructionHint::Type::ThroughPoint))
            {
            return ICurvePrimitive::CreateArc (
                    DEllipse3d::FromArcCenterStartEnd (hintA.Point (), hintB.Point (), hintC.Point ()));
            }
        return nullptr;
        }
    };
};


struct ConstructionSearcher
{
static ICurvePrimitivePtr ConstructByPairs (bvector<ConstructionHint> &hints, bvector<IConstructFrom2Hints *> const &builders)
    {
    for (size_t lastIndex = 1; lastIndex < hints.size (); lastIndex++)
        {
        for (size_t firstIndex = 0; firstIndex < lastIndex; firstIndex++)
            {
            for (auto builder : builders)
                {
                auto cp = builder->DoConstruction (hints[firstIndex], hints[lastIndex]);
                if (cp.IsValid ())
                    return cp;
                cp = builder->DoConstruction (hints[lastIndex], hints[firstIndex]);
                if (cp.IsValid ())
                    return cp;
                }
            }
        }
    return nullptr;
    }

static ICurvePrimitivePtr ConstructByTriples (bvector<ConstructionHint> &hints, bvector<IConstructFrom3Hints *> const &builders)
    {
    for (size_t lastIndex = 1; lastIndex < hints.size (); lastIndex++)
        {
        for (size_t middleIndex = 1; middleIndex < lastIndex; middleIndex++)
            {
            for (size_t firstIndex = 0; firstIndex < middleIndex; firstIndex++)
                {
                for (auto builder : builders)
                    {
                    auto cp = builder->DoConstruction (hints[firstIndex], hints[middleIndex], hints[lastIndex]);
                    if (cp.IsValid ())
                        return cp;
                    cp = builder->DoConstruction (hints[firstIndex], hints[lastIndex], hints[middleIndex]);
                    if (cp.IsValid ())
                        return cp;

                    cp = builder->DoConstruction (hints[middleIndex], hints[firstIndex], hints[lastIndex]);
                    if (cp.IsValid ())
                        return cp;
                    cp = builder->DoConstruction (hints[middleIndex], hints[lastIndex], hints[firstIndex]);
                    if (cp.IsValid ())
                        return cp;

                    cp = builder->DoConstruction (hints[lastIndex], hints[middleIndex], hints[firstIndex]);
                    if (cp.IsValid ())
                        return cp;
                    cp = builder->DoConstruction (hints[lastIndex], hints[firstIndex], hints[middleIndex]);
                    if (cp.IsValid ())
                        return cp;
                    }
                }
            }
        }
    return nullptr;
    }
};

void SaveHints (bvector<ConstructionHint> &hints)
    {
    static double s_markerSize = 0.25;
    for (auto &hint : hints)
        {
        switch (hint.m_type)
            {
            case ConstructionHint::Type::ThroughPoint:
                {
                Check::SaveTransformedMarkers (bvector<DPoint3d>{hint.m_location.point}, s_markerSize);
                }
                break;
            case ConstructionHint::Type::Center:
                {
                Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (hint.m_location.point, s_markerSize * 0.25));
                }
                break;
            case ConstructionHint::Type::ClosestPoint:
                {
                Check::SaveTransformed (*hint.m_location.curve);
                }
                break;
            case ConstructionHint::Type::PerpendicularNear:
                {
                Check::SaveTransformed (*hint.m_location.curve);
                Check::SaveTransformedMarkers (bvector<DPoint3d>{hint.m_location.point}, s_markerSize);
                }
                break;
            }
        }
    }

static bvector<IConstructFrom2Hints *> s_lineBuilders =
bvector<IConstructFrom2Hints *>
    {
    new LineConstructions::FromPointPoint (),
    new LineConstructions::FromPointClosestApproach (),
    new LineConstructions::FromClosestApproachClosestApproach (),
    new LineConstructions::FromPointPerpendicularNear ()
    };

static bvector<IConstructFrom3Hints *> s_circleBuilders =
bvector<IConstructFrom3Hints *>
    {
    new CircleConstructions::FromPointPointPoint (),
    new CircleConstructions::FromCenterPointPoint ()
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
    auto hPoint0 = ConstructionHint::CreateThroughPoint (point0);
    auto hPoint1 = ConstructionHint::CreateThroughPoint (point1);
    auto hLine0 = ConstructionHint::CreateClosestPoint (line0.get ());
    auto hArc0 = ConstructionHint::CreateClosestPoint (arc0.get ());
    auto hBsp0 = ConstructionHint::CreateClosestPoint (bsp0.get ());
    bvector<ConstructionHint> allHints {hPoint0, hPoint1, hLine0, hArc0, hBsp0};

    for (size_t i1 = 0; i1 < allHints.size (); i1++)
        {
        SaveAndRestoreCheckTransform shifter (0,a,0);
        for (size_t i0 = 0; i0 < allHints.size (); i0++)
            {
            SaveAndRestoreCheckTransform shifter (a,0,0);
            if (i0 != i1)
                {
                bvector<ConstructionHint> hints {allHints[i0], allHints[i1]};
                SaveHints (hints);
                auto cp0 = ConstructionSearcher::ConstructByPairs (hints, s_lineBuilders);
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
    auto hPoint0 = ConstructionHint::CreateThroughPoint (point0);

    double df = 1.0 / 16.0;
    for (double f = 0.0; f < 1.0; f += df)
        {
        SaveAndRestoreCheckTransform shifter (a,0,0);
        bvector<ConstructionHint> hints;
        hints.push_back (hPoint0);
        hints.push_back (ConstructionHint::CreatePerpendicularNear (arc0.get (), f));
        SaveHints (hints);
        auto cp0 = ConstructionSearcher::ConstructByPairs (hints, s_lineBuilders);
        if (cp0.IsValid ())
            Check::SaveTransformed (*cp0);
        }
    Check::ClearGeometry ("Construction.PointPerpendicularNear");
    }

TEST (Construction,HelloCircles)
    {
    double a = 10.0;


    auto hPoint0 = ConstructionHint::CreateThroughPoint (DPoint3d::From (0.1, 0.1));
    auto hPoint1 = ConstructionHint::CreateThroughPoint (DPoint3d::From (1,0));
    auto hPoint2 = ConstructionHint::CreateThroughPoint (DPoint3d::From (0,2));
    auto hCenter0 = ConstructionHint::CreateCenter (DPoint3d::From (1,1));
    bvector<ConstructionHint> hints;
    hints.push_back (hPoint0);
    hints.push_back (hPoint1);
    hints.push_back (hPoint2);
    SaveHints (hints);
    auto cp0 = ConstructionSearcher::ConstructByTriples (hints, s_circleBuilders);
    if (cp0.IsValid ())
        Check::SaveTransformed (*cp0);

    Check::Shift (a,0,0);

    hints.clear ();
    hints.push_back (hPoint0);
    hints.push_back (hCenter0);
    hints.push_back (hPoint2);
    SaveHints (hints);
    cp0 = ConstructionSearcher::ConstructByTriples (hints, s_circleBuilders);
    if (cp0.IsValid ())
        Check::SaveTransformed (*cp0);



    Check::ClearGeometry ("Construction.HelloCircles");
    }

