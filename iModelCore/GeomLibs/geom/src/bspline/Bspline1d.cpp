/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/Bspline1d.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

//BCurveSegment1d--------------------------------------

BCurveSegment1d::BCurveSegment1d ()
    {
    poles.clear ();
    knots.clear ();
    index = 0;
    uMin = uMax = 0.0;
    isNullU = true;
    }

DRange1d BCurveSegment1d::KnotRange () const
    {
    return DRange1d::From (uMin, uMax);
    }

void BCurveSegment1d::FractionToValue (double& value, double f) const
    {
    bsiBezier_evaluateUnivariate (&value, NULL, const_cast<double*>(&poles[0]), (int)poles.size (), f);
    }

void BCurveSegment1d::FractionToValue (double& value, double& der, double f, bool applyKnotScale) const
    {
    bsiBezier_evaluateUnivariate (&value, &der, const_cast<double*>(&poles[0]), (int)poles.size (), f);
    if (applyKnotScale && uMax != uMin)
        der /= uMax - uMin;
    }

double BCurveSegment1d::FractionToValue (double f) const
    {
    double value;
    FractionToValue (value, f);
    return value;
    }

double BCurveSegment1d::FractionToKnot (double fraction) const
    {
    return uMin + fraction * (uMax - uMin);
    }

//Bspline1d--------------------------------------

Bspline1d::Bspline1d ()
    {
    poles.clear ();
    knots.clear ();
    }

bvector<double>& Bspline1d::GetPolesR () { return poles;}
bvector<double>& Bspline1d::GetKnotsR () { return knots;}

int Bspline1d::GetOrder () const
    {
    return int (knots.size () - poles.size ());
    }

bool Bspline1d::GetSupport (bvector<double>& outPoles, bvector<double>& outKnots, size_t bezierSelect) const
    {
    size_t numPoles = poles.size ();
    size_t order    = (int)GetOrder ();
    outPoles.clear ();
    outKnots.clear ();

    if (bezierSelect <= numPoles - order)
        {
        for (size_t i = 0; i < order; i++)
            {
            size_t j = bezierSelect + i;
            outPoles.push_back (poles[j]);
            }

        size_t numKnotsOut = 2 * order - 2;
        for (size_t i = 0, j = bezierSelect + 1; i < numKnotsOut; i++, j++)
            outKnots.push_back (knots[j]);
        return true;
        }

    return false;
    }

bool Bspline1d::GetBezier (BCurveSegment1d& segment, size_t bezierSelect) const
    {
    size_t order = (int)GetOrder ();

    if (order > 1 && GetSupport (segment.poles, segment.knots, bezierSelect))
        {
        bsiBezier_saturateKnotsInInterval (&segment.poles[0], 1, &segment.knots[0], (int)order, segment.isNullU);
        segment.uMin = segment.knots[order - 2];
        segment.uMax = segment.knots[order - 1];
        segment.index = bezierSelect;
        return true;
        }

    return false;
    }

bool Bspline1d::FractionRoots (bvector<double> &rootFractions, double value) const
    {
    BCurveSegment1d segment;
    double roots[MAX_ORDER];
    int numRoot;

    rootFractions.clear ();
    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.isNullU && DRange1d::From (&segment.poles[0], segment.poles.size ()).Contains (value))
            {
            for (size_t i = 0; i < segment.poles.size (); i++)
                segment.poles[i] -= value;
            if (bsiBezier_univariateRoots (roots, &numRoot, &segment.poles[0], GetOrder ()) && numRoot > 0)
                {
                if (rootFractions.size () < 1)
                    {
                    for (size_t j = 0; j < (size_t)numRoot; j++)
                        rootFractions.push_back (segment.FractionToKnot (roots[j]));
                    }
                else
                    {
                    double t = segment.FractionToKnot (roots[0]);
                    if (!DoubleOps::AlmostEqual (rootFractions[rootFractions.size ()-1], t))
                        rootFractions.push_back (t);
                    for (size_t j = 1; j < (size_t)numRoot; j++)
                        rootFractions.push_back (segment.FractionToKnot (roots[j]));
                    }
                }
             }
        }

    return rootFractions.size () > 0;
    }

void Bspline1d::Populate (bvector<double> &poleVector, bvector<double> &knotVector)
    {
    poles.assign (poleVector.begin (), poleVector.end ());
    knots.assign (knotVector.begin (), knotVector.end ());
    }

void Bspline1d::Populate (double const * pVales, size_t numVales, double const * pKnots, size_t numKnots)
    {
    poles.assign (pVales, pVales + numVales);
    knots.assign (pKnots, pKnots + numKnots);
    }

void Bspline1d::Populate (double const * pVales, size_t numVales, size_t order)
    {
    poles.assign (pVales, pVales + numVales);
    knots.resize (numVales + order);
    size_t i, k = 1;
    for (i = 0; i < order; i++)
        knots[i] = 0.0;
    size_t num = knots.size ();
    while (k <= order)
        knots[num - (k++)]  = 1.0;

    double df = 1.0/(numVales - order + 1);
    for (i = order; i < num - order; i++)
        knots[i] = (i - order + 1)*df;
    }

DRange1d Bspline1d::KnotRange () const
    {
    double knot0 = knots[(int)GetOrder () - 1];
    size_t numKnots = knots.size ();
    double knot1 = knots[numKnots - (int)GetOrder ()];
    return DRange1d::From (knot0, knot1);
    }

double Bspline1d::FractionToKnot (double f) const
    {
    DRange1d range = KnotRange ();
    return range.low + f * (range.high - range.low);
    }

double Bspline1d::KnotToFraction (double knot) const
    {
    DRange1d range = KnotRange ();
    return (knot - range.low) / range.Length ();
    }

static void     knotToValueAndTangent
(
double          &value,
double          *der,
double          f,
bvector<double> poles,
bvector<double> knots
)
    {
    int         left, p, numKnots;
    double      b[MAX_BSORDER], dB[MAX_BSORDER], maxKnot;
    double      *bPtr, *bEnd, *dBLoc;

    dBLoc = der ? dB:NULL;

    numKnots = (int)knots.size ();
    int order = numKnots - (int)poles.size ();
    maxKnot = knots[numKnots - order];
    bsputil_knotToBlendingFuncs (b, dBLoc, &left, &knots[0], f, maxKnot, order, 0);

    value = 0.0;

    p = left - order;

    for (bPtr=b, bEnd=b+order; bPtr<bEnd; p++, bPtr++)
        value += *bPtr * poles[p];

    if (der)
        {
        *der = 0.0;
        p = left - order;
        for (bPtr=dB,bEnd=dB+order; bPtr<bEnd;  p++, bPtr++)
            *der += *bPtr * poles[p];
        }
    }

double Bspline1d::KnotToValue (double knot)
    {
    double value;
    knotToValueAndTangent (value, NULL, knot, GetPolesR (), GetKnotsR ());
    return value;
    }

void Bspline1d::KnotToValue (double& value, double& der, double knot)
    {
    knotToValueAndTangent (value, &der, knot, GetPolesR (), GetKnotsR ());
    }

double Bspline1d::FractionToValue (double f)
    {
    double knot = FractionToKnot (f);
    return KnotToValue (knot);
    }

void Bspline1d:: FractionToValue (double& value, double& der, double f)
    {
    double knot = FractionToKnot (f);
    KnotToValue (value, der, knot);
    DRange1d range = KnotRange ();
    der *= range.Length ();
    }
