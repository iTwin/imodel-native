/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.h>
#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>


void print (MSBsplineCurveR curve, char *pMessage)
    {
    int numKnots = curve.params.NumberAllocatedKnots ();
    printf ("\n MSBsplineCurve %s\n", pMessage ? pMessage : "");
    printf ("     (order %d) (numPoles %d) (numKnots %d) (rational %d) (numAllocKnots %d)\n",
                        curve.params.order,
                        curve.params.numPoles,
                        curve.params.numKnots,
                        curve.rational,
                        numKnots);

    for (int i = 0; i < curve.params.numPoles; i++)
        {
        DPoint3d xyz = curve.poles[i];
        if (curve.rational)
            printf ("   (%lg, %lg, %lg)/ %lg\n",
                        xyz.x, xyz.y, xyz.z, curve.weights[i]);
        else
            printf ("   (%lg, %lg, %lg)\n",
                        xyz.x, xyz.y, xyz.z);
        }
    for (int i = 0; i < numKnots; i++)
        {
        printf ("  %lg", curve.knots[i]);
        if (i + 1 == numKnots || ((i+1) % 4 == 0))
            printf ("\n");
        }
    }

void testKnotSearch (MSBsplineCurveR curve)
    {
    double knotA, knotB;
    int    kA, kB;
    double tolerance;
    MSBsplineCurve::KnotPosition kpos;
    curve.GetKnotRange (knotA, knotB, kA, kB, tolerance);
    double bigStep = 1000.0 * tolerance;
    double smallStep = 0.5 * tolerance;
    for (int k = kA; k <= kB; k++)
        {
        double u = curve.knots[k];
        double uNext = curve.knots[k+1];
        double uu;
        int k0, k1;
        if (k == kA)
            {
            kpos = curve.SearchKnot (u - bigStep, k0, k1, uu);
            checkInt ((int)kpos, (int)MSBsplineCurve::KNOTPOS_BEFORE_START, "LeftOfStart");
            kpos = curve.SearchKnot (u, k0, k1, uu);
            checkInt ((int)kpos, (int)MSBsplineCurve::KNOTPOS_START, "AtStart");
            kpos = curve.SearchKnot (u + smallStep, k0, k1, uu);
            checkInt ((int)kpos, (int)MSBsplineCurve::KNOTPOS_START, "AtStart+e");
            }
        else if (k == kB)
            {
            kpos = curve.SearchKnot (u + bigStep, k0, k1, uu);
            checkInt ((int)kpos, (int)MSBsplineCurve::KNOTPOS_AFTER_FINAL, "LeftOfStart");
            kpos = curve.SearchKnot (u, k0, k1, uu);
            checkInt ((int)kpos, (int)MSBsplineCurve::KNOTPOS_FINAL, "AtFinal");
            kpos = curve.SearchKnot (u - smallStep, k0, k1, uu);
            checkInt ((int)kpos, (int)MSBsplineCurve::KNOTPOS_FINAL, "AtFinal-e");
            }
        else
            {
            kpos = curve.SearchKnot (u, k0, k1, uu);
            checkInt ((int)kpos, (int)MSBsplineCurve::KNOTPOS_INTERIOR, "AtInterior");
            kpos = curve.SearchKnot (u - smallStep, k0, k1, uu);
            checkInt ((int)kpos, (int)MSBsplineCurve::KNOTPOS_INTERIOR, "Interior-e");
            kpos = curve.SearchKnot (u + smallStep, k0, k1, uu);
            checkInt ((int)kpos, (int)MSBsplineCurve::KNOTPOS_INTERIOR, "Interior+e");
            }

        if (k != kB)
            {
            kpos = curve.SearchKnot (0.5 * (u + uNext), k0, k1, uu);
            checkInt ((int)kpos, (int)MSBsplineCurve::KNOTPOS_INTERVAL, "Interval");
            checkInt (k1, k0 + 1, "Interval Brackets");
            }
        }
    }

void testArcs ()
    {
    MSBsplineCurve unitCircle;
    DPoint3d center = {0,0,0};
    DPoint3d xyz;
    memset (&unitCircle, 0, sizeof (unitCircle));
    MSBspline::InitEllipticArc (unitCircle, center, 1.0, 1.0);
    for (double s = 0.0; s <= 1.0; s += 0.125)
        {
        unitCircle.FractionToPoint (xyz, s);
        checkDouble (1.0, xyz.x * xyz.x + xyz.y * xyz.y, "UnitCircle point");
        }
    print (unitCircle, "Unit Circle");
    selectTolerances (3);
    checkDouble (unitCircle.Length (), msGeomConst_2pi, "Unit Circle Arc Length");
    selectTolerances (0);

    //DPoint3d tangent;
    //bspcurv_getTangent (&tangent, 0.0, &unitCircle, 0);
    //testKnotSearch (unitCircle);
    }

void testBezierCubicLine (double minKnot, double maxKnot)
    {
    MSBsplineCurve cubic;
    cubic.rational = 0;
    cubic.params.order = 4;
    cubic.params.closed = 0;
    cubic.params.numPoles = 4;
    cubic.Allocate ();
    cubic.poles[0].init (0,0,0);
    cubic.poles[1].init (1,0,0);
    cubic.poles[2].init (2,0,0);
    cubic.poles[3].init (3,0,0);

    for (int i = 0; i < 4; i++)
        {
        cubic.knots[i] = minKnot;
        cubic.knots[i+4] = maxKnot;
        }

    DVec3d startToEnd;
    startToEnd.differenceOf (&cubic.poles[3], &cubic.poles[0]);
    for (double s = 0.0; s <= 1.0; s += 0.125)
        {
        DPoint3d xyz;
        DVec3d  dxyz;
        cubic.FractionToPoint (xyz, dxyz, s);
        checkDouble (3.0 * s, xyz.x, "line point");
        checkDVec3d (&startToEnd, &dxyz, "tangent");
        }
    print (cubic, "Cubic on line");
    selectTolerances (3);
    double l0 = cubic.Length ();
    cubic.AddKnot (0.45, 1);
    print (cubic, "Add knot");
    testKnotSearch (cubic);
    MSBsplineCurve cubic1, cubic2;
    cubic1.CopyReversed(cubic);
    print (cubic1, "REVERSED");
    double l1 = cubic.Length ();
    double l2 = cubic1.Length ();
    checkDouble (l0, l1, "Length before after knot insertion");
    checkDouble (l0, l1, "Length after knot and reversal");
    cubic2.CopySegment (cubic, 0.5, 1.2);
    selectTolerances (0);
    }

void testGeneralCubic (DPoint3d *pXYZ, int numPoles, double knot0, double knot1)
    {
    MSBsplineCurve cubic;
    cubic.rational = 0;
    cubic.params.order = 4;
    cubic.params.closed = 0;
    cubic.params.numPoles = numPoles;
    cubic.Allocate ();
    for (int i = 0; i < numPoles; i++)
        cubic.poles[i] = pXYZ[i];

    int numAllocatedKnots = cubic.NumberAllocatedKnots ();
    int numInteriorKnots  = numAllocatedKnots - 2 * cubic.params.order;
    for (int i = 0; i < cubic.params.order; i++)
        {
        cubic.knots[i] = knot0;
        cubic.knots[numAllocatedKnots - 1 - i] = knot1;
        }
    double delta = (knot1 - knot0) / (numInteriorKnots + 1);
    for (int i = 0; i < numInteriorKnots; i++)
        {
        cubic.knots[cubic.params.order + i] = knot0 + i * delta;
        }

    for (int i = 0; i < 4; i++)
        {
        cubic.knots[i] = knot0;
        cubic.knots[i+4] = knot1;
        }

    MSBsplineCurve cubic1, cubic2, cubic3;

    print (cubic, "General Cubic");
    cubic1.CopyReversed(cubic);
    print (cubic1, "REVERSED");
    double cutKnotA = cubic.FractionToKnot (0.3);
    double cutKnotB = cubic.FractionToKnot (0.56);
    cubic2.CopySegment (cubic, cutKnotA, cutKnotB);
    cubic3.CopySegment (cubic, cutKnotB, cutKnotA);

    selectTolerances (3);
    double l0 = cubic.Length ();
    double l1 = cubic1.Length ();
    double l2 = cubic2.Length ();
    double l3 = cubic3.Length ();
    checkDouble (l0, l1, "Length before after reversal");
    checkDouble (l2, l3, "Length of forward and reversed segment");


    MSBsplineCurve cubicA, cubicB;
    cubicA.CopySegment (cubic, knot0, cutKnotA);
    cubicB.CopySegment (cubic, cutKnotA, knot1);
    print (cubicA, "cut A");
    print (cubicB, "cut B");
    checkDouble (cubic.Length (), cubicA.Length () + cubicB.Length (), "Length::Length of cuts");
    selectTolerances (0);

    }


void testSubPatches ()
    {
    DRange2d patchA, patchB, patchC;
    patchA.low.setComponents (1,2);
    patchA.high.setComponents (3,5);
    for (int code = 0; code < 4; code++)
        {
        bspproc_setSubPatchParameter ((DVector2d*)&patchB, code, (DVector2d*)&patchA);
        bsputil_selectQuarterPatch (&patchC, code, &patchA);
        checkDPoint2d (&patchB.low, &patchC.low, "quadrant low");
        checkDPoint2d (&patchB.high, &patchC.high, "quadrant high");
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    testBezierCubicLine (0,1);
    testBezierCubicLine (2,6);
    DPoint3d cubicA[4] =
        {
        {1,2,3},
        {2,3,6},
        {3,2,9},
        {4,4,15}
        };
    testGeneralCubic (cubicA, 4, 2.0, 5.0);
    testArcs ();
    testSubPatches ();
    return getExitStatus();
    }
