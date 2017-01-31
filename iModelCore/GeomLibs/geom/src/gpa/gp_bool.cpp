/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_bool.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static int s_debug = 0;

#define DEBUG_STATEMENT(s)

#define DEBUG_STATEMENT_notNow(s) if (s_debug){s}

/*---------------------------------------------------------------------------------**//**
* For each interval of the source as split by the split points,
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void buildTestPoint
(
GraphicsPointArray *pTestPoints,
GraphicsPointArray *pSource,
GraphicsPoint           *pGP0,
GraphicsPoint           *pGP1,
int                     breakIndex
)
    {
    GraphicsPoint testPoint;
    int intermediatePrimitive;
    double intermediateFraction;
    memset (&testPoint, 0, sizeof (testPoint));
    if (s_debug)
        {
        DEBUG_STATEMENT(
        printf  (
                " Test interval: (%d@%lf) (%d@%lf)\n",
                pGP0->userData, pGP0->a,
                pGP1->userData, pGP1->a
                );)
        }

    /* Regular interval within the source. */
    if (jmdlGraphicsPointArray_anyPrimitiveFractionInInterval (pSource,
                            &intermediatePrimitive, &intermediateFraction,
                            pGP0->userData, pGP0->a,
                            pGP1->userData, pGP1->a)
        && jmdlGraphicsPointArray_primitiveFractionToDPoint4d (pSource,
                            &testPoint.point, intermediatePrimitive, intermediateFraction))
        {
        testPoint.userData = breakIndex;
        if (s_debug)
            {
            printf  ("     test at: (%d@%lf)\n", intermediatePrimitive, intermediateFraction);
            }

        jmdlGraphicsPointArray_addGraphicsPoint (pTestPoints, &testPoint);
        }
    }

enum
    {
    BOOL_UNKNOWN = 0,
    BOOL_START = 1,
    BOOL_INTERSECTION = 2,
    BOOL_END = 3
    };

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int decodePointType
(
GraphicsPoint *pGP
)
    {
    if (pGP->mask & HPOINT_MASK_USER1)
        return BOOL_INTERSECTION;
    if (pGP->a == 0.0)
        return BOOL_START;
    if (pGP->a == 1.0)
        return BOOL_END;
    return BOOL_UNKNOWN;
    }
/*---------------------------------------------------------------------------------**//**
* For each interval of the source as split by the split points,
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void generateTestPoints
(
GraphicsPointArray *pTestPoints,
GraphicsPointArray *pSource,
GraphicsPointArray *pBreakPoints
)
    {
    GraphicsPoint gp0, gp1;
    int i, pointType;


    /* We expect that the break array contains sequences of form
            START intersection* END
       Distinguishible by:
            BOOL_START = no mask, parameter 0.
            BOOL_INTERSECTION = mask
            BOOL_END = no mask, parameter 1
    */
    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint
                    (pBreakPoints, &gp0, i); i++)
        {
        pointType = decodePointType (&gp0);
        if (pointType == BOOL_START)
            {
            for (;i++, jmdlGraphicsPointArray_getGraphicsPoint (pBreakPoints, &gp1, i);
                    gp0 = gp1)
                {
                pointType = decodePointType (&gp1);
                if (pointType == BOOL_INTERSECTION || pointType == BOOL_END)
                    {
                    buildTestPoint (pTestPoints, pSource, &gp0, &gp1, i - 1);
                    }
                else
                    {
                    printf (" ERROR -- unexpected point type %d at index %d \n", pointType, i);
                    }

                if (pointType == BOOL_END)
                    {
                    break;
                    }
                }
            }
        else
            {
            printf (" ERROR -- unexpected point type %d at index %d \n", pointType, i);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* For each interval of the source as split by the split points,
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void extractClassifiedSpans
(
GraphicsPointArray *pInside,
GraphicsPointArray *pOutside,
GraphicsPointArray *pSource,
GraphicsPointArray *pBreakPoints,
GraphicsPointArray *pTestPoints
)
    {
    GraphicsPoint breakGP0, breakGP1, testGP;
    int     testIndex;
    int     breakIndex0, breakIndex1;
    int     inBit, outBit;
    for (testIndex = 0; jmdlGraphicsPointArray_getGraphicsPoint
                    (pTestPoints, &testGP, testIndex); testIndex++)
        {
        breakIndex0 = testGP.userData;
        breakIndex1 = breakIndex0 + 1;
        if (   jmdlGraphicsPointArray_getGraphicsPoint
                    (pBreakPoints, &breakGP0, breakIndex0)
           &&  jmdlGraphicsPointArray_getGraphicsPoint
                    (pBreakPoints, &breakGP1, breakIndex1)
            )
            {
            bsiGraphicsPoint_getInOut (&testGP, &inBit, &outBit);
            if (inBit && !outBit)
                {
                if (pInside)
                    {
                    DEBUG_STATEMENT(
                    printf (" IN  (%d,%lf) (%d,%lf)\n",
                                breakGP0.userData, breakGP0.a,
                                breakGP1.userData, breakGP1.a);)
                    jmdlGraphicsPointArray_appendInterval (pInside, pSource,
                                breakGP0.userData, breakGP0.a,
                                breakGP1.userData, breakGP1.a);
                    }
                }
            else if (outBit && !inBit)
                {
                if (pOutside)
                    {
                    DEBUG_STATEMENT(
                    printf (" OUT (%d,%lf) (%d,%lf)\n",
                                breakGP0.userData, breakGP0.a,
                                breakGP1.userData, breakGP1.a);)
                    jmdlGraphicsPointArray_appendInterval (pOutside, pSource,
                                breakGP0.userData, breakGP0.a,
                                breakGP1.userData, breakGP1.a);
                    }
                }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Treat geometry in pSource as wireframe (curve) data.
* Clip the data to the boundary given pBoundary.  Inside and outside of pBoundary
*   are defined by parity rules.  This is only valid if the boundary array contains
*   closed loops!!
* All calculations are done in xy plane.
* @param pInside => GPA to receive geometry classified as "inside".  May be NULL.
* @param pOutside => GPA to receive geometry classified as "outside". May be NULL.
* @param pSource => source geometry (curves to be clipped)
* @param pBoundary => boundary geometry (clipper)
*
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_splitXYWireframe
(
        GraphicsPointArray      *pInside,
        GraphicsPointArray      *pOutside,
        GraphicsPointArray      *pSource,
        GraphicsPointArray      *pBoundary
)
    {
    double pointTolerance = 1.0e-8;
    double angleTolerance = -1.0;
    GraphicsPointArray *pTestPoints         = jmdlGraphicsPointArray_grab ();
    GraphicsPointArray *pBreakPoints = jmdlGraphicsPointArray_grab ();

    DEBUG_STATEMENT(printf (" Source \n");)

    DEBUG_STATEMENT(jmdlGraphicsPointArray_print (pSource, false);)
    DEBUG_STATEMENT(printf (" Boundary \n");)
    DEBUG_STATEMENT(jmdlGraphicsPointArray_print (pBoundary, false);)

    /* Collect up intersection and discontinuity points, marking the intersections
        so we can tell them apart. */
    jmdlGraphicsPointArray_xyIntersectionPoints
                    (
                    pBreakPoints, NULL,
                    pSource, pBoundary
                    );

    DEBUG_STATEMENT(printf (" Intersections \n");)
    DEBUG_STATEMENT(jmdlGraphicsPointArray_print (pBreakPoints, false);)
    jmdlGraphicsPointArray_writeMaskAllPoints (pBreakPoints, HPOINT_MASK_USER1, 1);
    jmdlGraphicsPointArray_addDiscontinuityPoints
                (pBreakPoints, pSource, NULL, 0.0, 2, pointTolerance, angleTolerance);
    DEBUG_STATEMENT(printf (" Plus breaks \n");)
    DEBUG_STATEMENT(jmdlGraphicsPointArray_print (pBreakPoints, false);)

    jmdlGraphicsPointArray_sortByUserDataAndA (pBreakPoints);
    DEBUG_STATEMENT(printf (" Sorted ... \n");)
    DEBUG_STATEMENT(jmdlGraphicsPointArray_print (pBreakPoints, false);)

    generateTestPoints (pTestPoints, pSource, pBreakPoints);

    jmdlGraphicsPointArray_testInOutMapped (pTestPoints, pBoundary, NULL);

    DEBUG_STATEMENT(printf (" Test Points \n");)
    DEBUG_STATEMENT(jmdlGraphicsPointArray_print (pTestPoints, false);)

    extractClassifiedSpans (pInside, pOutside, pSource, pBreakPoints, pTestPoints);

    jmdlGraphicsPointArray_drop (pBreakPoints);
    jmdlGraphicsPointArray_drop (pTestPoints);
    }

END_BENTLEY_GEOMETRY_NAMESPACE