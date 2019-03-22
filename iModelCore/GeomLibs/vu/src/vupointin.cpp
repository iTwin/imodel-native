/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/vupointin.cpp $
|    $RCSfile: vupointin.c,v $
|   $Revision: 1.2 $
|       $Date: 2006/09/14 14:56:12 $
|     $Author: DavidAssaf $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
static bool unitNormal
(
DPoint2d *normalP,
DPoint2d *point0P,
DPoint2d *point1P
)
    {
    double length, a;
    normalP->x = - (point1P->y - point0P->y);
    normalP->y = point1P->x - point0P->x;
    length = sqrt (normalP->x * normalP->x + normalP->y * normalP->y);
    if (length == 0.0)
        return false;
    a = 1.0 / length;
    normalP->x *= a;
    normalP->y *= a;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
static double dotVectorDifference
(
DPoint2d *vectorP,
DPoint2d *point0P,
DPoint2d *point1P
)
    {
    return vectorP->x * (point1P->x - point0P->x)
        +  vectorP->y * (point1P->y - point0P->y);
    }

/*-----------------------------------------------------------------*//**
* @description Over all edges of the face, search for the edge with <em>smallest</em> max distance to any vertex.
* @remarks For a triangle, this distance is the smallest of the altitudes.  For a convex face, this is the smallest vertical height as the
*       shape is rolled from one edge to another along a horizontal line.
* @param seedP  IN  node in face
* @bsimethod                            EarlinLutz      07/01
+----------------------------------------------------------------------*/
Public double vu_minEdgeVertexDistanceAroundFace
(
VuP seedP
)
    {
    DPoint2d baseXY, normal, currXY;
    VuP currP;
    double currDist, currBaseMaxDist, result = 1000000.0;
    int numBase = 0;
    VU_FACE_LOOP (baseP, seedP)
        {
        vu_getXY (&baseXY.x, &baseXY.y, baseP);
        currP = vu_fsucc (baseP);
        vu_getXY (&currXY.x, &currXY.y, currP);
        if (unitNormal (&normal, &baseXY, &currXY))
            {
            currBaseMaxDist = 0.0;
            for (currP = vu_fsucc(currP); currP != baseP; currP = vu_fsucc(currP))
                {
                vu_getXY (&currXY.x, &currXY.y, currP);
                currDist = fabs (dotVectorDifference (&normal, &baseXY, &currXY));
                if (currDist > currBaseMaxDist)
                    currBaseMaxDist = currDist;
                }

            if (numBase == 0 || currBaseMaxDist < result)
                {
                result = currBaseMaxDist;
                }
            numBase++;
            }
        }
    END_VU_FACE_LOOP (baseP, seedP)
    return result;
    }

/*-----------------------------------------------------------------*//**
* @description Find any point strictly interior to a polygon.  Only xy coordinates are examined.
* @param pXYOut <= coordinates of point in polygon.
* @param pLoopPoints => array of points in polygon.   Multiple loops
*               may be entered with the value DISCONNECT as x and y parts
*               of a separator point.
* @param numLoopPoints => number of points in array.
* @return SUCCESS if point found.
* @bsimethod                            EarlinLutz      07/01
+----------------------------------------------------------------------*/
Public StatusInt vu_anyInteriorPointInPolygon
(
DPoint3d                *pXYOut,
DPoint3dCP              pLoopPoints,
int                     numLoopPoints
)
    {
    StatusInt status = SUCCESS;
    VuSetP      graphP;
    VuArrayP    faceArrayP;
    VuP         faceP;
    int         i0, i1;
    int         numThisLoop;
    double      maxDist, minDist;
    DPoint3d    xySave;
    int numFace = 0;

    if ( ! (graphP = vu_newVuSet (0)) )
        return ERROR;


    for ( i0 = 0 ; i0 < numLoopPoints ;)
        {
        for (i1 = i0; i1 < numLoopPoints && pLoopPoints[i1].x != DISCONNECT;)
            {
            i1++;
            }
        numThisLoop = i1 - i0;
        if (numThisLoop > 2)
            vu_makeLoopFromArray3d (graphP, const_cast <DPoint3d*> (pLoopPoints + i0), numThisLoop, true, true);
        i0 = i1 + 1;
        }

    /*-------------------------------------------------------------------
    Merge loops is supposed to fix up all the criss-crosses and whatnot
    that might be here.  Sometimes it does, sometimes it has tolerance
    problems.  So we run it twice -- the second time cleans up what the
    first time doesn't catch.  Yup, that's strange.  But it works. (EDL)
    -------------------------------------------------------------------*/
    vu_mergeLoops (graphP);
    vu_mergeLoops (graphP);

    vu_regularizeGraph (graphP);
    vu_markAlternatingExteriorBoundaries(graphP,true);

    faceArrayP = vu_grabArray (graphP);

    vu_triangulateMonotoneInteriorFaces (graphP, false);
    vu_flipTrianglesToImproveQuadraticAspectRatio (graphP);
    vu_collectInteriorFaceLoops (faceArrayP, graphP);

    vu_arrayOpen (faceArrayP);

    maxDist = -1.0;
    memset (&xySave, 0, sizeof (xySave));
    for (;vu_arrayRead (faceArrayP, &faceP);)
        {
        double xSum = 0.0;
        double ySum = 0.0;
        numThisLoop = 0;
        VU_FACE_LOOP (P, faceP)
            {
            xSum += vu_getX (P);
            ySum += vu_getY (P);
            numThisLoop++;
            }
        END_VU_FACE_LOOP (P, faceP)
        xSum /= (double)numThisLoop;
        ySum /= (double)numThisLoop;

        minDist = vu_minEdgeVertexDistanceAroundFace (faceP);
        if (minDist > maxDist)
            {
            xySave.x = xSum;
            xySave.y = ySum;
            maxDist = minDist;
            }
        numFace++;
        }

    if (graphP)
        {
        if (faceArrayP)
            vu_returnArray (graphP, faceArrayP);
        vu_freeVuSet (graphP);
        }

    if (status == SUCCESS && numFace > 0 && pXYOut)
        *pXYOut = xySave;
    return status;
    }
END_BENTLEY_GEOMETRY_NAMESPACE