/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <printfuncs.h>
#include <stdlib.h>

typedef enum
    {
    VREL_Components,
    VREL_UnorientedAngle,
    VREL_OrientedAngle
    } VectorRelationship;

void checkDPlane3d
(
DPlane3d const *pPlaneA,
DPlane3d const *pPlaneB,
char *pDescr,
bool bSameOrigin,
VectorRelationship vrel
)
    {
    char descr[2048];
    if (bSameOrigin)
        {
        sprintf (descr, "DPlane3d.origin.identical::%s", pDescr);
        checkDPoint3d (&pPlaneA->origin, &pPlaneB->origin, descr);
        }
    else
        {
        double dA, dB;
        dA = pPlaneA->evaluate (&pPlaneB->origin);
        dB = pPlaneB->evaluate (&pPlaneA->origin);
        sprintf (descr, "DPlane3d.originB.onPlaneA::%s", pDescr);
        checkDouble (dA, 0.0, descr);
        sprintf (descr, "DPlane3d.originA.onPlaneB::%s", pDescr);
        checkDouble (dB, 0.0, descr);
        }

    if (vrel == VREL_Components)
        {
        sprintf (descr, "DPlane3d.normal.identical::%s", pDescr);
        checkDVec3d (&pPlaneA->normal, &pPlaneB->normal, descr);
        }
    else if (vrel == VREL_OrientedAngle)
        {
        double theta = bsiDVec3d_angleBetweenVectors (&pPlaneA->normal, &pPlaneB->normal);
        sprintf (descr, "DPlane3d.angleBetweenNormals::%s", pDescr);
        checkDouble (theta, 0.0, descr);
        }
    else
        {
        double theta = bsiDVec3d_smallerAngleBetweenUnorientedVectors (&pPlaneA->normal, &pPlaneB->normal);
        sprintf (descr, "DPlane3d.angleBetweenUnorientedNormals::%s", pDescr);
        checkDouble (theta, 0.0, descr);
        }
    }

void test_DPlane3d ()
    {
    DPlane3d planeA, planeB, planeC, planeD;
    DPoint3d origin;
    DVec3d normal;
    origin.init (1,2,3);
    normal.init (0.2, 0.3, 0.4);
    planeA.initFromOriginAndNormal (origin.x, origin.y, origin.z, normal.x, normal.y, normal.z);
    planeB.initFromOriginAndNormal (&origin, &normal);
    checkDPlane3d (&planeA, &planeB, "DPlane3d.initFromOriginAndNormal");
    checkDPlane3d (&planeA, &planeB, "DPlane3d.initFromOriginAndNormal", true, VREL_Components);

    planeB.normalize ();
    checkDPlane3d (&planeA, &planeB, "DPlane3d.normalize", true, VREL_OrientedAngle);
    checkDouble (1.0, planeB.normal.magnitude (), "DPlane3d.normalize");

    DPoint4d hPlaneA;
    planeA.getDPoint4d (&hPlaneA);
    planeC.init (&hPlaneA);
    checkDPlane3d (&planeA, &planeC, "DPlane3d HPlane RT", false, VREL_UnorientedAngle);

    double aa, bb, cc, dd;
    planeA.getCoefficients (&aa, &bb, &cc, &dd);

    planeD.init (aa, bb, cc, dd);
    checkDPlane3d (&planeA, &planeD, "DPlane3d Coffs RT", false, VREL_UnorientedAngle);

    checkFalse (planeD.isZero (), "DPlane3d.isZero");
    planeD.zero ();
    checkTrue  (planeD.isZero (), "DPlane3d.isZero");

    DPoint3d points[3];
    points[0].init (1,2,3);
    points[1].init (4,3,9);
    points[2].init (-2,5,3);
    DPlane3d planeE, planeF;
    planeE.initFrom3Points (&points[0], &points[1], &points[2]);
    planeF.initFromArray   (points, 3);
    checkDPlane3d (&planeE, &planeF, "DPlane3d 3pts, array", false, VREL_UnorientedAngle);

    for (int i = 0; i < 3; i++)
        {
        checkDouble (planeE.evaluate (&points[i]), 0.0, "DPlane3d evaluate (E)");
        checkDouble (planeF.evaluate (&points[i]), 0.0, "DPlane3d evaluate (F)");
        }
    DPoint3d pointQ, pointR;
    double a = 1.23123;
    pointQ.sumOf (&planeE.origin, &planeE.normal, a);
    checkDouble (planeE.evaluate (&pointQ), a * planeE.normal.magnitudeSquared (), "DPlane3d.evaluate");

    pointQ.sumOf (&points[2], &planeE.normal, a);
    planeE.projectPoint (&pointR, &pointQ);
    checkDPoint3d (&pointR, &points[2], "DPlane3d.projectPoint");


#ifdef abc

bool    DPlane3d::productOf
(

const   DTransform3d  *pTransform,
const   DPlane3d      *pSource

);
#endif
    }

int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    test_DPlane3d ();
    return getExitStatus();
    }
