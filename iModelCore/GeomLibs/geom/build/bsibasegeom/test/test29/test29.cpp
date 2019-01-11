
//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <Geom/bezeval.fdf>
#include <printfuncs.h>
#include <stdlib.h>


//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <Geom/bezeval.fdf>
#include <printfuncs.h>
#include <stdlib.h>


#include <Geom/cluster.h>
#ifdef clusterTest
static void Test (void)
    {
    STDVectorDPoint3d xyzArray;
    DPoint3d pt;
    pt.init (-30.463187027241787, 30.485011349948369, 0); xyzArray.push_back (pt);
    pt.init (-9.3768666993411145, 30.485011349948369, 0); xyzArray.push_back (pt);
    pt.init (-9.3768666993411145, 30.485011349948369, 0); xyzArray.push_back (pt);
    pt.init (-9.3768666993411021, 9.3986910220476751, 0); xyzArray.push_back (pt);
    pt.init (-9.3768666993411021, 9.3986910220476751, 0); xyzArray.push_back (pt);
    pt.init (-30.463187027241801, 9.3986910220476787, 0); xyzArray.push_back (pt);
    pt.init (-30.463187027241801, 9.3986910220476787, 0); xyzArray.push_back (pt);
    pt.init (-30.463187027241787, 30.485011349948369, 0); xyzArray.push_back (pt);

    STDVectorInt rInd;
    bsiDPoint3dArray_findClusters (xyzArray, rInd, 0.12, true, false);
    for (unsigned int i = 0; i < rInd.size (); i++)
        printf ("%d\n", rInd[i]);
    }
#endif

void outputPolygons
(
DPoint3d *xyzOut,
int nXYZOut,
bool forceClosure,
double dz
)
    {
    if (nXYZOut <= 0)
        return;
    int numThisLoop = 0;
    printf ("place shape\n");
    DPoint3d xyz0;
    xyz0.zero ();
    for (int i = 0; i < nXYZOut; i++)
        {
        if (!xyzOut[i].isDisconnect())
            {
            if (numThisLoop == 0)
                xyz0 = xyzOut[i];
            printf ("xy=%.15g,%.15g,%.15g\n",
                    xyzOut[i].x, xyzOut[i].y, xyzOut[i].z + dz);
            numThisLoop ++;
            if (numThisLoop > 0 && i + 1 == nXYZOut || xyzOut[i+1].isDisconnect ())
                {
                if (forceClosure)
                    printf ("xy=%.15g,%.15g,%.15g\n",
                        xyz0.x, xyz0.y, xyz0.z + dz);
                printf ("close element\n");
                printf ("place shape\n");
                numThisLoop = 0;
                i++;
                }
            }
        }
    }


void Go
(
DPoint3d *pXYZIn,
int numXYZIn,
DPlane3dR plane,
DVec3dR shift
)
    {
    #define N_BUFF 400
    DPoint3d xyzOut[N_BUFF];

    DPoint3d xyz[1000];

    for (int i = 0; i < numXYZIn; i++)
        xyz[i].sumOf (&pXYZIn[i], &shift);

    int  nXYZOut  = 0;
    int  nNumLoop = 0;

    DRange3d range;
    range.init ();
    range.extend (xyz, numXYZIn);
    double y0 = range.low.y - 0.25;
    double y1 = range.high.y + 0.25;
    double dy = 0.25;
    int maxStep = 40;
    double dz = 1.0;
    int step0 = 0;
    int step1 = 60;//maxStep;
    for (int step = step0; step < step1; step ++)
        {
        plane.origin.y = y0 + step * dy;
        if (plane.origin.y > y1)
            break;
        bsiPolygon_clipToPlane
           (xyzOut, &nXYZOut, &nNumLoop,
            N_BUFF, xyz, numXYZIn,
            &plane
           );

        outputPolygons (xyzOut, nXYZOut, false,  step * dz);
        }
    }


void Test
(
double x0, // absolute origin
double dx, // shift between internal variations
double dy, // shfit between internal variations
DPoint3dP pXYZ, int numXYZ, DPlane3dR plane)
    {

    DVec3d shift0, shift1, shift2, shift3;
    DRange3d range;
    range.init ();
    range.extend (pXYZ, numXYZ);

    shift0.init (x0 + 0, 0,0);
    shift1.init (x0 + 0, dy,0);
    shift2.init (x0 + dx,0,0);
    shift3.init (x0 + dx,dy,0);


    DPlane3d plane1 = plane;
    plane1.normal.negate ();
    Go (pXYZ, numXYZ, plane1, shift1);
    Go (pXYZ, numXYZ, plane, shift0);


    // Shift the whole polygon to the right.
    // Add a little to one end of each horizontal (to force single-point touch)
    double ey = 0.125;
    for (int i = 0; i < numXYZ; i++)
        {
        int i0 = (i + numXYZ - 1) % numXYZ;
        if (pXYZ[i0].y == pXYZ[i].y)
            pXYZ[i0].y += ey;
        }

    Go (pXYZ, numXYZ, plane, shift2);
    Go (pXYZ, numXYZ, plane1, shift3);
    }

void Test2 (double x0)
{
    DPlane3d plane;
    plane.origin.init (0,0,0);
    plane.normal.init (0,-1,0);


    DPoint3d poly[] = {
        0,0,0,
        0,7,0,
        9,7,0,
        9,0,0,
        8,0,0,
        8,6,0,
        3,6,0,
        3,2,0,
        6,2,0,
        6,4,0,
        5,4,0,
        5,3,0,
        4,3,0,
        4,5,0,
        7,5,0,
        7,1,0,
        2,1,0,
        2,6,0,
        1,6,0,
        1,0,0,
        };

    int numXYZ = sizeof(poly) / sizeof (DPoint3d);
    Test (x0, 20, 20, poly, numXYZ, plane);
    }


void Test1 (double x0)
{
    //Test ();
    DPlane3d plane;
    plane.origin.init (3.488377487018484, 17.801941935971879, 0);
    plane.normal.init (0,  -1.004,  -0);
    DPoint3d poly[] = {
      27.064720679170168,  2.0312106116473814, 9.4999999999999929,
      26.064720679170204,  3.0312106116473787, 9.4999999999999929,
      3.5094730297222019,  3.0312106116473818, 9.4999999999999982,
      3.9729370225190639,  17.803941935971881, 9.4999999999999982,
      26.064720679170168,  17.803941935971885, 9.4999999999999929,
      26.064720679170204,  3.0312106116473787, 9.4999999999999929,
      27.064720679170168,  2.0312106116473814, 9.4999999999999929,
      27.064720679170165,  18.803941935971881, 9.4999999999999929,
      3.0038179515178953,  18.803941935971878, 9.5,
      2.4776080815204047,  2.0312106116473787, 9.5
#if defined (Crash2)
        99.873571858000901,   1.2338338177598227,    -2.2500000000000044,
        99.181614923566173,   1.6349984536195159,    -2.2500000000000044,
        99.181614923566173,   1.6349984536195159,  -0.010000000000000231,
        99.191614923566178,   1.6292009159998107,  -0.009999999999999792,
        99.191614923566178,   1.6292009159998107,                      0,
        99.200301017635766,   1.6241651202861396,                      0,
        99.200301017635766,   1.6241651202861396,                  -0.01,
        99.459844090232849,   1.4736940475547529, -0.0099999999999997868,
        99.459844090232849,   1.4736940475547529,                      0,
        99.873571858000872,   1.2338338177598278,                      0,
#elif defined abc
        0,-1,0,
        0,1,0,
        1,0,0,
        1,1,0,
        2,1,0,
        2,-1,0,
        3,0,0,
        4,0,0,
        4,1,0,
        5,1,0,
        5,0,0,
        6,0,0,
        7,1,0,
        7,-10,0,
        2,-9,0,
        1,-9,0,
        1.5,0.5,0,
        0,-10,0,
#endif
    };

    int numXYZ = sizeof(poly) / sizeof (DPoint3d);
    Test (x0, 30, 20, poly, numXYZ, plane);
    }

void main (void)
    {
    Test1 (0);
    Test2 (100);
//    Test3 ();

    }