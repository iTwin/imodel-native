
//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <printfuncs.h>
#include <stdlib.h>

static double s_pi = 4.0 * atan (1.0);

static int cb_qsort_DPoint3d_x
(
void const *vpA,
void const *vpB
)
    {
    DPoint3d const *pA = (DPoint3d const *)vpA;
    DPoint3d const *pB = (DPoint3d const *)vpB;
    if (pA->x < pB->x)
        return -1;
    if (pA->x > pB->x)
        return 1;
    return 0;
    }

static int cb_qsort_DSegment3d_x0
(
void const *vpA,
void const *vpB
)
    {
    DSegment3d const *pA = (DSegment3d const *)vpA;
    DSegment3d const *pB = (DSegment3d const *)vpB;
    if (pA->point[0].x < pB->point[0].x)
        return -1;
    if (pA->point[0].x > pB->point[0].x)
        return 1;
    return 0;
    }

void testPolygonTransverseIntersect ()
    {
    DPoint3d xyzA[100];
    DPoint3d xyzB[100];
    DSegment3d segmentAB[100];
    int numSegment;
    int numA = 0, numB = 0;
    DVec3d normalA, normalB;
    bool    bParallel;

    bsiDPoint3d_setXYZ (xyzA + numA++, 0,0,0);
    bsiDPoint3d_setXYZ (xyzA + numA++, 1,0,0);
    bsiDPoint3d_setXYZ (xyzA + numA++, 1,2,0);
    bsiDPoint3d_setXYZ (xyzA + numA++, 0,2,0);

    bsiDPoint3d_setXYZ (xyzB + numB++, -1, 1,-1);
    bsiDPoint3d_setXYZ (xyzB + numB++,  2, 1,-1);
    bsiDPoint3d_setXYZ (xyzB + numB++,  2, 1, 1);
    bsiDPoint3d_setXYZ (xyzB + numB++, -1, 1, 1);

    bsiPolygon_transverseIntersection
                (
                segmentAB, &numSegment,
                &bParallel, &normalA, &normalB,
                100,
                xyzA, numA, xyzB, numB);

    checkInt (numSegment, 1, "A intersect B segments");
    checkFalse (bParallel, "not parallel");
    qsort (segmentAB, 1, sizeof (DPoint3d), cb_qsort_DPoint3d_x);
    checkDPoint3dXYZ (&segmentAB[0].point[0], 0,1,0, "intersect enter");
    checkDPoint3dXYZ (&segmentAB[0].point[1], 1,1,0, "intersect leave");
    }


void testPolygonClip ()
    {
    DPoint3d xyzArray[100];
    DPoint3d xyzClip[100];
    double   paramClip[100];
    int numClip = 0;

    int numXYZ = 0;
    bsiDPoint3d_setXYZ (&xyzArray[numXYZ++], 0,0,0);
    bsiDPoint3d_setXYZ (&xyzArray[numXYZ++], 1,0,0);
    bsiDPoint3d_setXYZ (&xyzArray[numXYZ++], 1,1,0);
    bsiDPoint3d_setXYZ (&xyzArray[numXYZ++], 0,1,0);
    DVec3d perp;
    bsiDVec3d_setXYZ (&perp, 0,0,1);
    DRay3d ray;

    bsiDPoint3d_setXYZ (&ray.origin, -0.5, 0.5, 0);
    bsiDPoint3d_setXYZ (&ray.direction, 2, 0, 0);

    bsiPolygon_clipDRay3d (xyzClip, paramClip, &numClip, 100, xyzArray, numXYZ, &perp, &ray);

    checkInt (numClip, 2, "Unit Square clip @y=0.5");
    checkDouble (paramClip[0], 0.25, "Enter");
    checkDouble (paramClip[1], 0.75, "Leave");

    bsiDPoint3d_setXYZ (&xyzArray[numXYZ++], 0,2,0);
    bsiDPoint3d_setXYZ (&xyzArray[numXYZ++], 2,2,0);
    bsiDPoint3d_setXYZ (&xyzArray[numXYZ++], 2,-2,0);
    bsiDPoint3d_setXYZ (&xyzArray[numXYZ++], 0,-2,0);

    bsiPolygon_clipDRay3d (xyzClip, paramClip, &numClip, 100, xyzArray, numXYZ, &perp, &ray);

    checkInt (numClip, 2, "Concave @y=0.5");
    checkDouble (paramClip[0], 0.75, "Enter");
    checkDouble (paramClip[1], 1.25, "Leave");

    bsiDPoint3d_setXYZ (&ray.origin,    0.25, 0.5, 0);
    bsiDPoint3d_setXYZ (&ray.direction, 0.05, 1, 0);

    bsiPolygon_clipDRay3d (xyzClip, paramClip, &numClip, 100, xyzArray, numXYZ, &perp, &ray);

    checkInt (numClip, 4, "Concave @y=0.5");
    checkDouble (paramClip[0], -2.5, "Enter");
    checkDouble (paramClip[1], -0.5, "Leave");

    checkDouble (paramClip[2],  0.5, "Enter");
    checkDouble (paramClip[3],  1.5, "Leave");

    bsiDPoint3d_setXYZ (&ray.direction, 0.05, -1, 0);

    bsiPolygon_clipDRay3d (xyzClip, paramClip, &numClip, 100, xyzArray, numXYZ, &perp, &ray);

    checkInt (numClip, 4, "Concave @y=0.5");
    checkDouble (paramClip[0],  -1.5, "Enter");
    checkDouble (paramClip[1], -0.5, "Leave");

    checkDouble (paramClip[2],  0.5, "Enter");
    checkDouble (paramClip[3],  2.5, "Leave");

    bsiDPoint3d_setXYZ (&ray.origin, 0,1,1);
    bsiDPoint3d_setXYZ (&ray.direction, 1,0,0);
    // Don't really know how an "on" edge is handled, but we know the midpoint..
    bsiPolygon_clipDRay3d (xyzClip, paramClip, &numClip, 100, xyzArray, numXYZ, &perp, &ray);

    checkInt (numClip, 2, "Concave @y=0.5");
    checkDouble (fabs (paramClip[0] - 0.5),  0.5, "MidPoint of ON edge");
    checkDouble (paramClip[1],  2.0, "Leave");


    bsiDPoint3d_setXYZ (&ray.direction, 1,0,0);
    // Don't really know how an "on" edge is handled, but we know the midpoint..
    bsiPolygon_clipDRay3d (xyzClip, paramClip, &numClip, 100, xyzArray, numXYZ, &perp, &ray);

    checkInt (numClip, 2, "Concave @y=0.5");
    checkDouble (fabs (paramClip[0] - 0.5),  0.5, "MidPoint of ON edge");
    checkDouble (paramClip[1],  2.0, "Leave");

    }

void testPolygon2dInOut ()
    {
    DPoint2d xyArray[100];

    int numXYZ = 0;
    bsiDPoint2d_setComponents (&xyArray [numXYZ++], 0,0);
    bsiDPoint2d_setComponents (&xyArray [numXYZ++], 1,0);
    bsiDPoint2d_setComponents (&xyArray [numXYZ++], 1,1);
    bsiDPoint2d_setComponents (&xyArray [numXYZ++], 0,1);
    double tol = 1.0e-5;
    DPoint2d xy;
    for (double x = -0.5; x < 2.0; x += 0.5)
        for (double y = -0.5; y < 2.0; y+= 0.5)
            {
            int code;
            if (x > 1.0
                || x < 0.0
                || y > 1.0
                || y < 0.0
                )
                code = -1;
            else if (x > 0.0 && x < 1.0 && y > 0.0 && y < 1.0)
                code = 1;
            else
                code = 0;
            char s[100];
            sprintf (s, "Unit Square bsiDPoint2d_PolygonParity (%lf,%lf)", x,y);
            bsiDPoint2d_setComponents (&xy, x, y);
            checkInt (code, bsiDPoint2d_PolygonParity (&xy, xyArray, 4, tol), s);
            }
    }


void testPolygon3dXYParity ()
    {
    DPoint3d     xyzArray[100];

    int numXYZ = 0;
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], 0,0, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], 1,0, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], 1,1, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], 0,1, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], 0,0, 0);
    double tol = 1.0e-5;
    DPoint3d xyz;
    for (double x = -0.5; x < 2.0; x += 0.5)
        for (double y = -0.5; y < 2.0; y+= 0.5)
            {
            int code;
            if (x > 1.0
                || x < 0.0
                || y > 1.0
                || y < 0.0
                )
                code = -1;
            else if (x > 0.0 && x < 1.0 && y > 0.0 && y < 1.0)
                code = 1;
            else
                code = 0;
            char s[100];
            sprintf (s, "Unit Square bsiGeom_XYPolygonParity (%lf,%lf)", x,y);
            bsiDPoint3d_setXYZ (&xyz, x, y, 0);
            checkInt (code, bsiGeom_XYPolygonParity (&xyz, xyzArray, 4, tol), s);
            }


    double delta = 0.1;
    for (int depth = 1; depth < 3; depth++)
        {
        double a = delta * depth;
        double b = 1.0 - a  ;
        bsiDPoint3d_setXYZ (&xyzArray[numXYZ++], DISCONNECT, DISCONNECT, DISCONNECT);
        bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], a,a, 0);
        bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], b,a, 0);
        bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], b,b, 0);
        bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], a,b, 0);
        bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], a,a, 0);

        //printf (" subsquare %lg,%lg\n", a, b);
        for (double x = -0.05; x < 1.1; x += 0.1)
            {
            DPoint3d xyz;
            xyz.init (x, 0.5, 0);
            int code1 = bsiGeom_XYPolygonParity (&xyz, xyzArray, numXYZ, tol);
            double dx = fabs (0.5 - x);
            double xx = 0.5 + dx;
            int numIn = 0;
            for (double bb = b; bb < 1.01; bb += delta)
                {
                if (xx < bb)
                    numIn++;
                }
            int code0 = (numIn & 0x01) ? 1 : -1;
            char s[200];
            sprintf (s, " x=%lg b=%lg numIn =%d", x, b, numIn);
            checkInt (code0, code1, s);
            }
        }
    }

void testPolygon3dXYParityDiamonds ()
    {
    DPoint3d     xyzArray[100];
    double tol = 1e-8;
    int numXYZ = 0;
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], -2, 0, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  0,-2, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  2, 0, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  0, 2, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], DISCONNECT, DISCONNECT, DISCONNECT);

    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], -1, 0, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  0,-1, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  1, 0, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  0, 1, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], DISCONNECT, DISCONNECT, DISCONNECT);

    DPoint3d xyz;
    bsiDPoint3d_setXYZ (&xyz, 0,0,0);   // x,y tests hit vertices
    checkInt (-1, bsiGeom_XYPolygonParity (&xyz, xyzArray, numXYZ, tol), "2 Diamonds 000");
    double f = 0.1;
    bsiDPoint3d_setXYZ (&xyz, f,0,0);   // x tests hit vertices
    checkInt (-1, bsiGeom_XYPolygonParity (&xyz, xyzArray, numXYZ, tol), "2 Diamonds f00");

    bsiDPoint3d_setXYZ (&xyz, 1+f,0,0);   // x tests hit vertices
    checkInt (1, bsiGeom_XYPolygonParity (&xyz, xyzArray, numXYZ, tol), "2 Diamonds (1+f)00");

    bsiDPoint3d_setXYZ (&xyz, f,f,0);
    checkInt (-1, bsiGeom_XYPolygonParity (&xyz, xyzArray, numXYZ, tol), "2 Diamonds ff0");

    bsiDPoint3d_setXYZ (&xyz, 0,1+f,0);
    checkInt (1, bsiGeom_XYPolygonParity (&xyz, xyzArray, numXYZ, tol), "2 Diamonds 0(1+f)0");
    }

void testPolygon3dXYParityOverlap ()
    {
    CGWriter cgwriter(stdout);

    DPoint3d     xyzArray[100];
    int startIndex[20];
    startIndex[0] = 0;
    int numLoop = 0;
    double tol = 1e-8;
    int numXYZ = 0;
    double radius = 0.02;
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], -2, 0, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  0,-2, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  2, 0, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  0, 2, 0);
    xyzArray[numXYZ++] = xyzArray[startIndex[numLoop]];
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], DISCONNECT, DISCONNECT, DISCONNECT);
    startIndex[++numLoop] = numXYZ;

    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], -1, 0, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  0,-1, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  1, 0, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  0, 1, 0);
    xyzArray[numXYZ++] = xyzArray[startIndex[numLoop]];
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], DISCONNECT, DISCONNECT, DISCONNECT);
    startIndex[++numLoop] = numXYZ;

    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], -1, -1, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  -1, 1, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  4, 1, 0);
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++],  4, -1, 0);
    xyzArray[numXYZ++] = xyzArray[startIndex[numLoop]];
    bsiDPoint3d_setXYZ (&xyzArray [numXYZ++], DISCONNECT, DISCONNECT, DISCONNECT);
    startIndex[++numLoop] = numXYZ;

    double x0 = -2.2  ;
    double xPeriod = 3.0;
    double df = 0.02;
    double y0 = 0.1;
    double dy = 2.1;
    double twoPi = 8.0 * atan(1.0);
    for (double f = 0.0; f < 2.3; f+= df)
        {
        DPoint3d xyz;
        xyz.x = x0 + f * xPeriod;
        xyz.y = y0 + dy * sin (f * twoPi);
        xyz.z = 0;
        bool in0 = bsiGeom_XYPolygonParity (&xyz, xyzArray, numXYZ, tol) > 0;
        bool in1 = false;
        char loopChar[20];
        for (int loop = 0; loop < numLoop; loop++)
            {
            int n = startIndex[loop+1] - startIndex[loop];
            bool in = bsiGeom_XYPolygonParity (&xyz, xyzArray + startIndex[loop], n, tol) > 0;
            in1 ^= in;
            loopChar[loop] = in ? '1' : '0';
            }
        loopChar[numLoop] = 0;
        char s[200];

        sprintf (s, "Overlaps: %10lg,%10lg (%s::%s)",
                    xyz.x, xyz.y,
                    in0 ? "1" : "0",
                    loopChar
                    );
        checkBool (in0, in1, s);
        if (in0)
            cgwriter.circle (xyz, radius);
        else
            cgwriter.coordinate (xyz);
        }

    cgwriter.polygon (xyzArray, numXYZ);
    }



void testPolygon3d_isConvex ()
    {
    DPoint3d box0[5] =
        {
            {0,0,0},
            {2,0,0},
            {2,2,0},
            {0,2,0},
            {0,0,0},
        };

    DPoint3d box1[6] =
        {
            {0,0,0},
            {2,0,0},
            {1.8,1,0},
            {2,2,0},
            {0,2,0},
            {0,0,0},
        };

    DPoint3d box[40];
    int num0 = 5;
    int num1 = 6;
    Transform T;
    checkInt (1, bsiGeom_testXYPolygonConvex (box0, num0), "Convex box");
    checkInt (0, bsiGeom_testXYPolygonConvex (box1, num1), "Convex box");

    //  Skew along x lines....
    bsiTransform_initFromRowValues (&T,
                1,1,0,0,
                0,1,0,0,
                0,0,0,1);
    bsiTransform_multiplyDPoint3dArray (&T, box, box0, num0);
    checkInt (1, bsiGeom_testXYPolygonConvex (box, 5), "Convex box");
    bsiTransform_multiplyDPoint3dArray (&T, box, box1, num1);
    checkInt (0, bsiGeom_testXYPolygonConvex (box1, 5), "Convex box");

    //  Flip, Skew along x lines....
    bsiTransform_initFromRowValues (&T,
                -1,-1,0,0,
                0,1,0,0,
                0,0,0,1);

    bsiTransform_multiplyDPoint3dArray (&T, box, box0, num0);
    checkInt (-1, bsiGeom_testXYPolygonConvex (box, 5), "Convex box");
    bsiTransform_multiplyDPoint3dArray (&T, box, box1, num1);
    checkInt (0, bsiGeom_testXYPolygonConvex (box1, 5), "Convex box");

    // Regular polygons.  Pull each point inwards.
    DPoint3d xyz[100];
    int numSides = 6;
    double dTheta = 8.0 * atan(1.0) / numSides;
    double r = 1.3;
    DPoint3d center = {4.2, 6.6, 0.2};
    char message[100];
    for (int i = 0; i < numSides; i++)
        {
        for (int j = 0; j < numSides; j++)
            {
            double theta = j * dTheta;
            xyz[j].x = center.x + r * cos (theta);
            xyz[j].y = center.y + r * sin (theta);
            xyz[j].z = center.z;
            }
        xyz[numSides] = xyz[0];
        int n = numSides+1;
        // The regular polygon is convex ....
        sprintf (message, "Convex (numSides %d)", numSides);
        checkInt (1, bsiGeom_testXYPolygonConvex (xyz, n), message);
        xyz[i] = center;
        xyz[numSides] = xyz[0];
        // but not after pulling a point to the center ...
        sprintf (message, "Krinkle (numSides %d) (move vertex %d)", numSides, i);
        checkInt (0, bsiGeom_testXYPolygonConvex (xyz, n), message);
        }

    // This one always turns left, but wraps twice ...
    DPoint3d wrap[9] =
        {
            {0,4,0},
            {4,4,0},
            {4,6,0},
            {2,6,0},
            {2,2,0},
            {8,2,0},
            {8,8,0},
            {0,8,0},
            {0,4,0},

        };
    checkInt (1, bsiGeom_testXYPolygonTurningDirections (wrap, 9), "Double wrap turns left");
    checkInt (0, bsiGeom_testXYPolygonConvex (wrap, 9), "Double wrap is not convex");
    }

void testDisconnects ()
    {
    DPoint3d polygon[] =
      {
        {288.62890507280827,-75.765466198325157, 0},
        {267.42718051373959,-200.52946069836617, 0},
        {143.47863695025444,-220.91573429107666, 0},
        {-9.8261405974626541,-149.15605117380619, 0},
        {30.130955681204796,-47.224683135747910, 0},
        {288.62890507280827,-75.765466198325157, 0},
        {DISCONNECT, DISCONNECT, 0},
        {86.397070854902267,-132.03158134222031, 0},
        {129.61597089469433,-162.20326630771160, 0},
        {170.38851810991764,-135.29338511824608, 0},
        {155.71040111780167,-99.413543581962585, 0},
        {100.25973691046238,-108.38350397348404, 0},
        {86.397070854902267,-132.03158134222031, 0},
      };

    DPoint3d testPoint []
      =
      {
        {48.8723523875939, -146.457779176371, 0},
        {225.382519745816, -171.754727477308, 0},
        {163.987678925565, -191.935439043224, 0},
        {80.4224789202231, -73.1250526410709, 0},
        {214.297340153271, -81.9363492402736, 0},
        {122.2051, -132.5302, 0},
        {122, -128, 0},
        {350, -128, 0},
      };

    int testCode []
        =
        {
        1,
        1,
        1,
        1,
        1,
        -1,
        -1,
        -1,
        };

    int polygonCount = sizeof (polygon) / sizeof (DPoint3d);
    int testCount = sizeof (testPoint) / sizeof (DPoint3d);
    double tol = 1.0e-10;
    for (int i = 0; i < testCount; i++)
        {
        char s[1024];
        sprintf (s, " XYParityTestPoint %g,%g", testPoint[i].x, testPoint[i].y);
        checkInt (testCode[i], bsiGeom_XYPolygonParity (&testPoint[i], polygon, polygonCount, tol), s);
        }
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);

    //printStandardViews ();
    testPolygonClip ();
    testPolygonTransverseIntersect ();
    testPolygon2dInOut ();
    testPolygon3d_isConvex ();
    testPolygon3dXYParity ();
    testPolygon3dXYParityDiamonds ();
    testPolygon3dXYParityOverlap ();
    testDisconnects ();
    return getExitStatus();
    }
