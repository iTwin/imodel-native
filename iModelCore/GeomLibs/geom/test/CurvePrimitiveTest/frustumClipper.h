/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

// Lexical ordering of points on "any" frsutum -- F1 is "closer to eye"
//
//      2------------------3
//      | \     F4       / |
//      |   6----------7   |
//      |   |          |   |   (BOTTOM = F0)
//      |F5 |   F1     |F3 |
//      |   |          |   |
//      |   4----------5   |
//      | /     F2       \ |
//      0------------------1
//
// Call a set of points in this order a "LexicalFrustum"
//
static int s_cornerIndexCCW[6][4] =
{
    {0,2,3,1},
    {4,5,7,6},
    {0,1,5,4},
    {1,3,7,5},
    {3,2,6,7},
    {2,0,4,6}
};

/*----------------------------------------------------------------------------------*//**
Get points around an indexed face of a lexical frustum. 
If corner points are in the customary order (NPC_xxx) this works for both range cube corners
and perspective view corners.
* @bsimethod                                                    EarlinLutz 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* GetLexicalFrustumFacePoints (DPoint3d* outPts, DPoint3d* frustumCornerPoints, unsigned int faceIndex)
    {
    faceIndex = faceIndex % 6;  // really shouldn't change it, but we'll be safe ...
    for (int i = 0; i < 4; i++)
        {
        int k = s_cornerIndexCCW[faceIndex][i];
        outPts[i] = frustumCornerPoints[k];
        }
    outPts[4] = outPts[0];
    return outPts;
    }
/*----------------------------------------------------------------------------------*//**
Get a plane from a lexical frustum
 (1) origin is first point and of face,
 (2) normal is cross product of vectors to second and third points.
* @bsimethod                                                    EarlinLutz 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DPlane3d GetLexicalFrustumFacePlane (DPoint3d* frustumCornerPoints, unsigned int faceIndex)
    {
    faceIndex = faceIndex % 6;  // really shouldn't change it, but we'll be safe ..    
    DPlane3d plane;
    plane.origin = frustumCornerPoints[s_cornerIndexCCW[faceIndex][0]];
    plane.normal.CrossProductToPoints (
                frustumCornerPoints[s_cornerIndexCCW[faceIndex][0]],
                frustumCornerPoints[s_cornerIndexCCW[faceIndex][1]],
                frustumCornerPoints[s_cornerIndexCCW[faceIndex][2]]
                );
    plane.normal.Normalize ();
    return plane;
    }



/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ToddSouthen   2013/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPolygonWithFrustum(DPoint3d* outPts, int* outCount, int maxOut, DPoint3d* inPts, int numIn,
    DPoint3d* clipperCorners)
    {
    DPoint3d buffer1[100];
    DPoint3d buffer2[100];
    DPoint3d* currPts = buffer1;
    DPoint3d* clipPts = buffer2;
    int nClip = 0;
    int nCurr;

    //start with input points, then keep clipping them to the planes
    numIn = numIn > 100 ? 100: numIn;
    memcpy(currPts, inPts, numIn * sizeof(DPoint3d));
    nCurr = numIn;

    //clip with each plane
    for (int ii=0; nCurr > 0 && ii < 6; ii++)
        {
        int numLoop;
        DPlane3d plane = GetLexicalFrustumFacePlane (clipperCorners, ii);

        bsiPolygon_clipToPlane(clipPts, &nClip, &numLoop, maxOut, currPts, nCurr, &plane);
        
        //don't include disconnect points at end 
        while (nClip > 0 && clipPts[nClip-1].IsDisconnect ())
            nClip--;

        //swap currPts and clipPts
        DPoint3d* tmpPts = currPts;
        currPts = clipPts;
        clipPts = tmpPts;
        nCurr = nClip;
        nClip = 0;
        }

    //copy currPts to output
    nCurr = nCurr > maxOut ? maxOut: nCurr;
    memcpy(outPts, currPts, nCurr * sizeof(DPoint3d));
    *outCount = nCurr;
    }


/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ToddSouthen   2013/01
+---------------+---------------+---------------+---------------+---------------+------*/
void LexicalFrustumIntersection
(
DPoint3d intersections[12][9], //<= OUT intersection points
int intersectionCounts[12],     //<= OUT number of points in each intersection
int* nIntersectionsP,           //<= OUT number of intersecting polygons
DPoint3d frustumACorners[8], //=> IN 1st frustum corners in XYZ lexical order.
DPoint3d frustumBCorners[8] //=> IN 2nd frustum corners in XYZ lexical order.
)
    {
    DPoint3d currentFacePoints[5];
    DPlane3d planes[12];
    DPoint3d clipPts[12][9];

    int nClip[12];
    int nPolygon = 0;

    memset(clipPts, 0, sizeof(clipPts));
    memset(nClip, 0, sizeof(nClip));

    memset(intersections, 0, sizeof(clipPts));
    memset(intersectionCounts, 0, sizeof(nClip));
    *nIntersectionsP = 0;

    //intersect each view polygon with the range planes
    for (int ii=0; ii < 6; ii++)
        {
        //if ((noFrontClip && ii == FACE_FRONT) || (noBackClip && ii == FACE_BACK))
        //    continue;

        GetLexicalFrustumFacePoints (currentFacePoints, frustumACorners, ii);
        ClipPolygonWithFrustum (&clipPts[nPolygon][0], &nClip[nPolygon], 9, currentFacePoints, 5, frustumBCorners);
        if (nClip[nPolygon] > 0)
            {
            planes[nPolygon] = GetLexicalFrustumFacePlane (frustumACorners, ii);
            nPolygon++;
            }
        }

    //intersect each range polygon with the view planes
    for (int ii=0; ii < 6; ii++)
        {
        GetLexicalFrustumFacePoints (currentFacePoints, frustumBCorners, ii);
        ClipPolygonWithFrustum (&clipPts[nPolygon][0], &nClip[nPolygon], 9, currentFacePoints, 5, frustumACorners);
        if (nClip[nPolygon] > 0)
            {
            planes[nPolygon] = GetLexicalFrustumFacePlane (frustumBCorners, ii);
            nPolygon++;
            }
        }

    //since we have two convex shapes we're intersecting, compute the normals of each polygon and if we have any that are equal to others, skip all but the last one

    for (int ii=0; ii < nPolygon; ii++)
        {
        bool copyToOutput = true;

        if (nPolygon > 6)
            {
            bool found = false;
            for (int jj=ii+1; !found && jj < nPolygon; jj++)
                {
                if (planes[ii].normal.IsParallelTo(*(&planes[jj].normal)))
                    found = true;
                }
            copyToOutput = !found;
            }

        if (copyToOutput)
            {
            memcpy(&intersections[*nIntersectionsP][0], &clipPts[ii][0], nClip[ii] * sizeof(DPoint3d));
            intersectionCounts[*nIntersectionsP] = nClip[ii];
            *nIntersectionsP = *nIntersectionsP + 1;
            }
        }
    }
