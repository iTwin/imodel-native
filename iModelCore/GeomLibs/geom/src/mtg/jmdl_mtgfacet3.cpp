/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_mtgfacet3.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include "../memory/jmdl_iarray.fdf"
#include "../memory/jmdl_dpnt3.fdf"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMesh_projectToXYEdge                                |
|                                                                       |
| author        EarlinLutz                              04/95           |
|                                                                       |
| Project to a bounded line segment, using only xy parts.               |
+----------------------------------------------------------------------*/
static void    jmdlMesh_projectToXYEdge
(
        DPoint3d    *pNearPoint,
        double      *pLambda,
        double    *pDistanceSquared,
const   DPoint3d    *pPoint,
const   DPoint3d    *pStart,
const   DPoint3d    *pEnd
)
    {
    double uX = pPoint->x - pStart->x;
    double uY = pPoint->y - pStart->y;
    double vX = pEnd->x - pStart->x;
    double vY = pEnd->y - pStart->y;

    double dotUV = uX * vX + uY * vY;
    double dotVV = vX * vX + vY * vY;


    if ( dotUV < 0.0)
        {
        *pLambda = 0.0;
        *pNearPoint = *pStart;
        *pDistanceSquared = uX * uX + uY * uY;
        }
    else if (dotUV >= dotVV)
        {
        *pLambda = 1.0;
        *pNearPoint = *pEnd;
        double dX = pPoint->x - pEnd->x;
        double dY = pPoint->y - pEnd->y;
        *pDistanceSquared = dX * dX + dY * dY;
        }
    else
        {
        *pLambda = dotUV / dotVV;
        pNearPoint->Interpolate (*pStart, *pLambda, *pEnd);
        double dX = pPoint->x - pNearPoint->x;
        double dY = pPoint->y - pNearPoint->y;
        *pDistanceSquared = dX * dX + dY * dY;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @param pFacetHeader => facet set to search.
* @param pProj <= nearest point
* @param pStartPoint <= edge start
* @param pEndPoint <= edge end
* @param pLambda <= fractional coordinate along edge
* @param pMinDist <= in-plane distance to point
* @param piNode <= base mtg node of picked edge
* @param pPickPoint => pick point
* @param aperture => maximum distance to consider
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_evaluateEdgePick
(
const   MTGFacets * pFacetHeader,
        DPoint3d    *pProj,
        DPoint3d    *pStartPoint,
        DPoint3d    *pEndPoint,
        double      *pLambda,
        double      *pMinDist,
        int         *piNode,
const   DPoint3d    *pPickPoint,
        double      aperture
)
    {
    const EmbeddedDPoint3dArray *pVertexArray = &pFacetHeader->vertexArrayHdr;
    DPoint3d nearPoint, startPoint, endPoint;
    double   lambda, distanceSquared;
    double   minDistanceSquared;

    MTGNodeId mateId;
    int iStartVertex, iEndVertex;
    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    bool     boolstat = false;

    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    minDistanceSquared = aperture * aperture;


    MTGARRAY_SET_LOOP (currId, pGraph)
        {
        mateId = jmdlMTGGraph_getEdgeMate (pGraph, currId);
        if (currId > mateId
            && jmdlMTGGraph_getLabel (pGraph, &iStartVertex, currId, vertexIdOffset)
            && jmdlMTGGraph_getLabel (pGraph, &iEndVertex, mateId, vertexIdOffset)
            && jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &startPoint, iStartVertex)
            && jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &endPoint, iEndVertex)
           )
            {
            jmdlMesh_projectToXYEdge (
                        &nearPoint, &lambda, &distanceSquared,
                        pPickPoint, &startPoint, &endPoint);

            if (  distanceSquared <= minDistanceSquared)
                {
                *pProj = nearPoint;
                *pStartPoint = startPoint;
                *pEndPoint   = endPoint;
                *pLambda = lambda;
                *piNode  = currId;
                minDistanceSquared = distanceSquared;
                boolstat = true;
                }
            }
        }
    MTGARRAY_END_SET_LOOP (currId, pGraph)

    if (boolstat)
        {
        *pMinDist = sqrt (minDistanceSquared);
        }
    return boolstat;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGFacets_facetZ                                    |
|                                                                       |
| author        EarlinLutz                              04/95           |
|                                                                       |
| Produce a z value for a point on a facet.                             |
| When n>3, there is the non-planar facet problem.  Pretty much ignore  |
| it -- take whatever plane origin and normal extractPolygonNormal      |
| returns, and put the z value at the pierce point with that plane.     |
| For pick purposes, this is fine.  Anybody who needs something else    |
| can write
+----------------------------------------------------------------------*/
static bool    jmdlMTGFacets_facetZ   // z coordinate
(
double          *pZ,
const DPoint3d  *pPointArray,       // => vertex coordinates
int              n,                 // => number of points
const DPoint3d  *pPickPoint         // => xy point
)
    {
    bool    boolstat = false;

    if (n >= 3)
        {
        DPoint3d planeOrigin, planeNormal;
        DPoint3d rayOrigin, rayVector;
        DPoint3d piercePoint;
        double lambda;

        rayOrigin.Init ( pPickPoint->x, pPickPoint->y, 0.0);
        rayVector.Init ( 0.0, 0.0, 1.0);
        if (   bsiGeom_polygonNormal (
                                &planeNormal, &planeOrigin, pPointArray, n)
            && bsiGeom_rayPlaneIntersection(&lambda, &piercePoint,
                                &rayOrigin, &rayVector, &planeOrigin, &planeNormal)
            )
            {
            *pZ = piercePoint.z;
            boolstat = true;
            }
        }
    else if (n == 2)
        {
        DPoint2d projectionPoint;
        double lambda;
        if (bsiVector2d_projectPointToLine (&projectionPoint, &lambda,
                                (DPoint2d *)pPickPoint,
                                (DPoint2d *)&pPointArray[0],
                                (DPoint2d *)&pPointArray[1]
                                )
            )
            {
            *pZ = pPointArray[0].z + lambda * (pPointArray[1].z - pPointArray[0].z);
            boolstat = true;
            }
        }
    return boolstat;
    }

static double testFunc
(
double value
)
    {
    return value - 1.0;
    }


/*---------------------------------------------------------------------------------**//**
* @param pFacetHeader    => mesh to pick
* @param pPierce        <= pierce point with max z
* @param pFaceNodeId    <= any node on the face
* @param pPickPoint     => pick point
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_evaluateConvexFacetPick
(
const   MTGFacets * pFacetHeader,
        DPoint3d    *pPierce,
        MTGNodeId  *pFaceNodeId,
const   DPoint3d    *pPickPoint
)
    {
    const EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);
    EmbeddedIntArray  *pFaceIdArray = jmdlEmbeddedIntArray_grab ();
    const DPoint3d          *pCoordinateArray, *pCurrPoint, *pPredPoint;
    EmbeddedDPoint3dArray *pFaceLoop = jmdlEmbeddedDPoint3dArray_grab ();
    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    double cross;
    MTGNodeId      startId;
    int         hitCount = 0;
    int         i, n;
    int         numPos, numNeg;
    bool        boolstat= false;
        const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    pPierce->Zero ();

    jmdlMTGGraph_collectAndNumberFaceLoops(pGraph, NULL, pFaceIdArray);

    while (SUCCESS == jmdlVArrayInt_pop (&startId, pFaceIdArray))
        {
        if (!jmdlMTGGraph_getMask (pGraph, startId, MTG_EXTERIOR_MASK)
            && jmdlMTGFacets_getFaceCoordinates (pGraph,
                        pFaceLoop, pVertexArray, startId, vertexIdOffset)
            )
            {
            numPos = numNeg = 0;
            pCoordinateArray = jmdlEmbeddedDPoint3dArray_getConstPtr (pFaceLoop, 0);
            n = jmdlEmbeddedDPoint3dArray_getCount (pFaceLoop);
            pPredPoint = pCoordinateArray + n - 1;

            for (i = 0; i < n && (numPos == 0 || numNeg == 0); i++, pPredPoint = pCurrPoint)
                {
                pCurrPoint = pCoordinateArray + i;
                cross = ((DPoint2d *)pPickPoint)->CrossProductToPoints (*((DPoint2d *)pPredPoint), *((DPoint2d *)pCurrPoint));
                if (0.0 < cross)
                    {
                    numPos++;
                    }
                else
                    {
                    numNeg++;
                    }
                }

            double zCurr;
            if (((numPos > 0 && numNeg == 0) || (numPos == 0 && numNeg > 0))
                && jmdlMTGFacets_facetZ (&zCurr, pCoordinateArray, n, pPickPoint))
                {
                double zTest = zCurr + 1.0;
                testFunc (zTest);
                if (hitCount == 0 || zCurr > pPierce->z)
                    {
                    *pPierce = *pPickPoint;
                    *pFaceNodeId = startId;
                    hitCount++;
                    pPierce->z = zCurr;
                    boolstat = true;
                    }
                }
            }
        }
    jmdlEmbeddedDPoint3dArray_drop (pFaceLoop);
    jmdlEmbeddedIntArray_drop (pFaceIdArray);

    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* @param pFacetHeader   => mesh to test
* @param pNearPoint <= nearest point
* @param pMinDist <= in-plane distance to point
* @param piVertex <= vertex index
* @param pPickPoint => pick point
* @param aperture => maximum distance to consider
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_evaluateVertexPick
(
const   MTGFacets * pFacetHeader,
        DPoint3d    *pNearPoint,
        double      *pMinDist,
        int         *piVertex,
const   DPoint3d    *pPickPoint,
        double      aperture
)
    {
    int iVertex;
    DPoint3d testPoint;
    DPoint3d nearPoint = *pPickPoint;
    int kVertex = -1;
    double minDist2 = 0.0;


    for (iVertex = 0;
        jmdlEmbeddedDPoint3dArray_getDPoint3d ((&pFacetHeader->vertexArrayHdr), &testPoint, iVertex
                        );
        iVertex++
        )
        {
        double dist2 = testPoint.DistanceSquaredXY (*pPickPoint);
        if (kVertex == -1 || dist2 < minDist2)
            {
            kVertex = iVertex;
            minDist2 = dist2;
            nearPoint = testPoint;
            }
        }
    *piVertex = kVertex;
    *pMinDist = sqrt (minDist2);
    *pNearPoint = nearPoint;

    return *piVertex >= 0;
    }
#define MAX_FLOAT_BUFFER 2048

/*---------------------------------------------------------------------------------**//**
* Output all interior face loops of the facet set as arrays of
* coordianates.
* @param pFacetHeader    => mesh to output
* @param pVertexFunc => output function for faces without normals
* @param pVertexAndNormalFunc => output function for faces with normal data
* @param useNormalsIfAvailable => true to output normal data when available
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_emitFloatLoops
(
const MTGFacets *    pFacetHeader,
MTG_floatFaceFunction   pVertexFunc,
MTG_floatFaceFunction   pVertexAndNormalFunc,
bool                    useNormalsIfAvailable
)
    {
    MTGNodeId startNodeId;
    int vertexIndex;
    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    float floatBuffer[MAX_FLOAT_BUFFER];
    float *pFloat;
    EmbeddedIntArray *pFaceStart = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pFaceId    = jmdlEmbeddedIntArray_grab ();
    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);

    const EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);
    const EmbeddedDPoint3dArray *pNormalArray = (&pFacetHeader->normalArrayHdr);

    DPoint3d currPoint, currNormal;
    int i;
    int numVertex, maxVertex;
    maxVertex = MAX_FLOAT_BUFFER / 3;   // NEEDS WORK: allow for normals
    int nFace = jmdlMTGGraph_collectAndNumberFaceLoops(pGraph, pFaceStart, pFaceId);

    MTGFacets_NormalMode normalMode = pFacetHeader->normalMode;
    if (!useNormalsIfAvailable)
        normalMode = MTG_Facets_VertexOnly;

    if  (   normalMode == MTG_Facets_VertexOnly
        ||  normalMode == MTG_Facets_NormalPerVertex
        )
        {
        bool    getNormals = (normalMode == MTG_Facets_NormalPerVertex);

        if (getNormals)
            maxVertex /= 2;

        for (i = 0; i < nFace; i++)
            {
            jmdlVArrayInt_getInt (pFaceStart, &startNodeId, i);
            // Collect vertex coordinates from this (nonExterior) face ...

            if (!jmdlMTGGraph_getMask (pGraph, startNodeId, MTG_EXTERIOR_MASK))
                {
                numVertex = 0;
                pFloat = floatBuffer;
                MTGARRAY_FACE_LOOP (currNodeId, pGraph, startNodeId)
                    {
                    if (    jmdlMTGGraph_getLabel (pGraph, &vertexIndex, currNodeId, vertexIdOffset)
                        &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &currPoint, vertexIndex)
                        )
                        {
                        numVertex++;
                        if (numVertex < maxVertex)
                            {
                            *pFloat++ = (float)currPoint.x;
                            *pFloat++ = (float)currPoint.y;
                            *pFloat++ = (float)currPoint.z;
                            if (getNormals)
                                {
                                jmdlVArrayDPoint3d_getDPoint3d (pNormalArray, &currNormal, vertexIndex);
                                *pFloat++ = (float)currNormal.x;
                                *pFloat++ = (float)currNormal.y;
                                *pFloat++ = (float)currNormal.z;
                                }
                            }
                        }
                    }
                MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, startNodeId)

                // and dump them to the appropriate receiver ...
                if (numVertex <= maxVertex)
                    {
                    if (getNormals)
                        pVertexAndNormalFunc (numVertex, floatBuffer);
                    else
                        pVertexFunc (numVertex, floatBuffer);
                    }
                }
            }
        }
    else
        {
        // NEEDS WORK: emit with normal data
        }

    jmdlEmbeddedIntArray_drop (pFaceId);
    jmdlEmbeddedIntArray_drop (pFaceStart);
    }

/*---------------------------------------------------------------------------------**//**
* Add a point to an evolving triangle list.  Assumes immediatly preceding numVertex
* points are from same face.
* If numVertex is 0,1, or 2 just add it.
* If higher, replicate numVertex-1, and 0, and then add the point.
* @param = number of vertices, including this point.
+---------------+---------------+---------------+---------------+---------------+------*/
static void jmdlMTGFacets_addTrianglePoint
(
EmbeddedDPoint3dArray *pDestArray,
int             numVertex,
DPoint3d        *pPoint
)
    {
    DPoint3d point0, point1;
    if (numVertex < 3)
        {
        jmdlVArrayDPoint3d_addPoint (pDestArray, pPoint);
        }
    else
        {
        int numTotal = jmdlEmbeddedDPoint3dArray_getCount (pDestArray);
        jmdlVArrayDPoint3d_getDPoint3d (pDestArray, &point0, numTotal - numVertex);
        jmdlVArrayDPoint3d_getDPoint3d (pDestArray, &point1, numTotal - 1);
        pDestArray->push_back (point1);
        pDestArray->push_back (point0);
        jmdlVArrayDPoint3d_addPoint (pDestArray, pPoint);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Visit all interior faces.  Subdivide each to triangles.  Pack coordiantes into
* output arrays.
* Warning: Triangulation may assume convex faces.
* @param pFacetHeader    => mesh to output
* @param pDestPointArray <= array of triangle vertex coordinates
* @param pDestNormalArray <= array of triangle normal coordinates
* @param pDestParamArray <= array of triangle vertex parameters
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_collectPackedCoordinateTriangles
(
const MTGFacets         *pFacetHeader,
      EmbeddedDPoint3dArray   *pDestPointArray,
      EmbeddedDPoint3dArray   *pDestNormalArray,
      EmbeddedDPoint3dArray   *pDestParam1Array,
      EmbeddedDPoint3dArray   *pDestParam2Array
)
    {
    MTGNodeId startNodeId;
    int vertexIndex;
    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    EmbeddedIntArray *pFaceStart = jmdlEmbeddedIntArray_grab ();
    const MTGGraph * pGraph = &pFacetHeader->graphHdr;

    const EmbeddedDPoint3dArray *pSourceVertexArray = (&pFacetHeader->vertexArrayHdr);
    const EmbeddedDPoint3dArray *pSourceNormalArray = (&pFacetHeader->normalArrayHdr);
    const EmbeddedDPoint3dArray *pSourceParam1Array = (&pFacetHeader->param1ArrayHdr);
    const EmbeddedDPoint3dArray *pSourceParam2Array = (&pFacetHeader->param2ArrayHdr);

    DPoint3d currPoint;
    int i;
    int numVertex;
    int nFace = jmdlMTGGraph_collectAndNumberFaceLoops(pGraph, pFaceStart, NULL);

    MTGFacets_NormalMode normalMode = pFacetHeader->normalMode;

    if  (   normalMode == MTG_Facets_VertexOnly
        ||  normalMode == MTG_Facets_NormalPerVertex
        )
        {
        int numSourceVertex =  jmdlEmbeddedDPoint3dArray_getCount (pSourceVertexArray);
        bool    getNormals =   pDestNormalArray != NULL
                            && normalMode == MTG_Facets_NormalPerVertex;
        bool    getParam1  =   pDestParam1Array != NULL
                            && numSourceVertex == jmdlEmbeddedDPoint3dArray_getCount (pSourceParam1Array);
        bool    getParam2  =   pDestParam2Array != NULL
                            && numSourceVertex == jmdlEmbeddedDPoint3dArray_getCount (pSourceParam2Array);

        for (i = 0; i < nFace; i++)
            {
            jmdlVArrayInt_getInt (pFaceStart, &startNodeId, i);
            // Collect vertex coordinates from this (nonExterior) face ...

            if (!jmdlMTGGraph_getMask (pGraph, startNodeId, MTG_EXTERIOR_MASK)
                && jmdlMTGGraph_countNodesAroundFace (pGraph, startNodeId) >= 3)
                {
                numVertex = 0;
                MTGARRAY_FACE_LOOP (currNodeId, pGraph, startNodeId)
                    {
                    if (    jmdlMTGGraph_getLabel (pGraph, &vertexIndex, currNodeId, vertexIdOffset)
                        &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pSourceVertexArray, &currPoint, vertexIndex)
                        )
                        {
                        if (pDestPointArray)
                            {
                            jmdlMTGFacets_addTrianglePoint (pDestPointArray, numVertex, &currPoint);
                            }
                        if (getNormals)
                            {
                            jmdlVArrayDPoint3d_getDPoint3d (pSourceNormalArray, &currPoint, vertexIndex);
                            jmdlMTGFacets_addTrianglePoint (pDestNormalArray, numVertex, &currPoint);
                            }
                        if (getParam1)
                            {
                            jmdlVArrayDPoint3d_getDPoint3d (pSourceParam1Array, &currPoint, vertexIndex);
                            jmdlMTGFacets_addTrianglePoint (pDestParam1Array, numVertex, &currPoint);
                            }
                        if (getParam2)
                            {
                            jmdlVArrayDPoint3d_getDPoint3d (pSourceParam2Array, &currPoint, vertexIndex);
                            jmdlMTGFacets_addTrianglePoint (pDestParam2Array, numVertex, &currPoint);
                            }
                        numVertex++;
                        }
                    }
                MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, startNodeId)
                }
            }
        }
    else
        {
        // NEEDS WORK: emit with independent normal data
        }

    jmdlEmbeddedIntArray_drop (pFaceStart);
    }


/*---------------------------------------------------------------------------------**//**
* Output all interior face loops of the facet set as arrays of
* coordianates with RGB's.
* @param pFacetHeader    => mesh to output
* @param pFunc => output function f(xyzArray, rgbArray, numVertex, pUserData)
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_emitFloatRGBLoops
(
const MTGFacets *    pFacetHeader,
MTG_floatXYZRGBFunction  pFunc,
void    *pUserData
)
    {
    MTGNodeId startNodeId;
    int vertexIndex;
    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    float xyzBuffer[MAX_FLOAT_BUFFER];
    float rgbBuffer[MAX_FLOAT_BUFFER];
    float *pXYZ;
    float *pRGB;
    EmbeddedIntArray *pFaceStart = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pFaceId    = jmdlEmbeddedIntArray_grab ();
    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);

    const EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);
    const EmbeddedDPoint3dArray *pRGBArray = (&pFacetHeader->param2ArrayHdr);

    DPoint3d currPoint, currRGB;
    int i;
    int numVertex, maxVertex;
    maxVertex = MAX_FLOAT_BUFFER / 3;   // NEEDS WORK: allow for normals
    int nFace = jmdlMTGGraph_collectAndNumberFaceLoops(pGraph, pFaceStart, pFaceId);


    for (i = 0; i < nFace; i++)
        {
        jmdlVArrayInt_getInt (pFaceStart, &startNodeId, i);
        // Collect vertex coordinates from this (nonExterior) face ...

        if (!jmdlMTGGraph_getMask (pGraph, startNodeId, MTG_EXTERIOR_MASK))
            {
            numVertex = 0;
            pXYZ = xyzBuffer;
            pRGB = rgbBuffer;

            MTGARRAY_FACE_LOOP (currNodeId, pGraph, startNodeId)
                {
                if (    jmdlMTGGraph_getLabel (pGraph, &vertexIndex, currNodeId, vertexIdOffset)
                    &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &currPoint, vertexIndex)
                    &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pRGBArray, &currRGB, vertexIndex)
                    )
                    {
                    numVertex++;
                    if (numVertex < maxVertex)
                        {
                        *pXYZ++ = (float)currPoint.x;
                        *pXYZ++ = (float)currPoint.y;
                        *pXYZ++ = (float)currPoint.z;

                        *pRGB++     = (float)currRGB.x;
                        *pRGB++     = (float)currRGB.y;
                        *pRGB++     = (float)currRGB.z;
                        }
                    }
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, startNodeId)

            // and dump them to the receiver ...
            if (numVertex <= maxVertex)
                {
                pFunc (xyzBuffer, rgbBuffer, numVertex, pUserData);
                }
            }
        }

    jmdlEmbeddedIntArray_drop (pFaceId);
    jmdlEmbeddedIntArray_drop (pFaceStart);
    }

#ifdef COMPILE_HPOINTS

/*---------------------------------------------------------------------------------**//**
* Collect specified edges in HPoints form.  Directly sequenced edges
* are chained (using vertex id to identify chaining opportunities)
* but no other searches are performed to enhance chaining.
* @param pFacetHeader               => source mesh
* @param pHPoints       <= collected edges
* @param pEdgeArray     => edges to collect
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
HPOINTSPublic GEOMDLLIMPEXP void jmdlMTGFacets_collectEdges
(
    const MTGFacets    *pFacetHeader,
    HPoints             *pHPoints,
const EmbeddedIntArray  *pEdgeArray
)
    {
    int i;
    MTGNodeId  tailNodeId, headNodeId;
    int         tailVertexId, headVertexId, prevVertexId;
    int vertexLabelOffset = pFacetHeader->vertexLabelOffset;
    DPoint3d tailCoordinates, headCoordinates;


    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    const EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);

    prevVertexId = -1;
    for (i = 0; jmdlEmbeddedIntArray_getInt (pEdgeArray, &tailNodeId, i); i++)
        {
        headNodeId = jmdlMTGGraph_getFSucc (pGraph, tailNodeId);

        if (    jmdlMTGGraph_getLabel (pGraph, &tailVertexId, tailNodeId, vertexLabelOffset)
            &&  jmdlMTGGraph_getLabel (pGraph, &headVertexId, headNodeId, vertexLabelOffset)
            &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &tailCoordinates, tailVertexId)
            &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &headCoordinates, headVertexId)
            )
            {

            if (tailVertexId != prevVertexId)
                {
                jmdlHPoints_markBreak (pHPoints);
                jmdlHPoints_addDPoint3d (pHPoints, &tailCoordinates);
                }

            jmdlHPoints_addDPoint3d (pHPoints, &headCoordinates);
            prevVertexId = headVertexId;
            }
        }

    jmdlHPoints_markBreak (pHPoints);
    }


/*---------------------------------------------------------------------------------**//**
*
* Extract masked edges from facets to HPoints.   Attempts to keep
* edges within a face in the same polyline.
*
* @param pFacetHeader    => source mesh
* @param pHPoints <= collected edges
* @param searchMask => search mask
* @param maskOn => target value of mask
*                    true collects masked edges.
*                    false collects unmasked edges.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
HPOINTSPublic GEOMDLLIMPEXP void jmdlMTGFacets_collectMaskedEdges
(
const MTGFacets    *pFacetHeader,
      HPoints       *pHPoints,
      MTGMask       searchMask,
      bool          maskOn
)
    {
    MTGNodeId currNodeId, lastNodeId;
    int vertexId;
    // We need a non-const graph pointer so we can use mask bits.
    MTGGraph * pGraph = (MTGGraph *)(&pFacetHeader->graphHdr);

    MTGMask visitMask;
    MTGMask bothMasks;
    MTGMask nodeMask;
    MTGMask nodeSearchMask;
    MTGMask nodeTestMask = maskOn ? searchMask : 0;    // This is the value that we WANT the mask in the node to be
    DPoint3d xyz;
    int vertexLabelOffset = pFacetHeader->vertexLabelOffset;
    const EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);

    visitMask = jmdlMTGGraph_grabMask (pGraph);
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    bothMasks = visitMask | searchMask;
    int chainLength;

    MTGARRAY_SET_LOOP (tailNodeId, pGraph)
        {
        for (currNodeId = tailNodeId, chainLength = 0;; currNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId))
            {
            nodeMask = jmdlMTGGraph_getMask (pGraph, currNodeId, bothMasks);
            nodeSearchMask = searchMask & nodeMask;
            jmdlMTGGraph_setMask (pGraph, currNodeId, visitMask);
            if (     (nodeMask & visitMask)
                ||  !((maskOn && nodeSearchMask) || (!maskOn && !nodeSearchMask))
                || !jmdlMTGGraph_getLabel (pGraph, &vertexId, currNodeId, vertexLabelOffset)
                || SUCCESS != jmdlVArrayDPoint3d_getDPoint3d (pVertexArray, &xyz, vertexId)
                )
                break;
            jmdlHPoints_addDPoint3d (pHPoints, &xyz);
            lastNodeId = currNodeId;
            chainLength++;
            }

        if (chainLength > 0)
            {
            MTGNodeId finalNodeId = jmdlMTGGraph_getFSucc (pGraph, lastNodeId);
            if (
                   jmdlMTGGraph_getLabel (pGraph, &vertexId, finalNodeId, vertexLabelOffset)
                && jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &xyz, vertexId)
               )
                {
                jmdlHPoints_addDPoint3d (pHPoints, &xyz);
                }
            jmdlHPoints_markBreak (pHPoints);
            }
        }
    MTGARRAY_END_SET_LOOP (tailNodeId, pGraph)
    jmdlMTGGraph_dropMask (pGraph, bothMasks);
    }
#endif
/*----------------------------------------------------------------------+
|                                                                       |
| name          reverseIntArray                                         |
|                                                                       |
| author        RayBentley                                  3/96        |
|                                                                       |
+----------------------------------------------------------------------*/
static void reverseIntArray
(
int     *pArray,
int     nItems
)
    {
    int     i, last, middle = nItems >> 1;

    for (i=0, last = nItems-1; i<middle; i++, last--)
        {
        int tmp = pArray[i];

        pArray[i] = pArray[last];
        pArray[last] = tmp;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Output all interior face loops of the facet set as arrays of
* integer indices taken from the "vertex label" from the nodes.
* @param pFacetHeader    => mesh to emit
* @param reverseLoops => true to emit loops in reverse order
* @param pIndices => array of alternate indices.
* @param numIndex => number of alternate indices
* @param pFunc => output function
* @param userData => extra arg for output function
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_emitReindexedLoops
(
const MTGFacets *           pFacetHeader,
      bool                  reverseLoops,
      int                   *pIndices,
      int                   numIndex,
MTG_indexedFaceFunction     pFunc,
      int                   userData
)
    {
    MTGNodeId startNodeId;
    int vertexIndex;
    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    EmbeddedIntArray *pFaceStart = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pFaceId    = jmdlEmbeddedIntArray_grab ();
    const   MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    int i;
    int numVertex;

    int nFace = jmdlMTGGraph_collectAndNumberFaceLoops(pGraph, pFaceStart, pFaceId);

    if (pFacetHeader->normalMode == MTG_Facets_VertexOnly
        || pFacetHeader->normalMode == MTG_Facets_NormalPerVertex)
        {

        for (i = 0; i < nFace; i++)
            {
            jmdlVArrayInt_getInt (pFaceStart, &startNodeId, i);
            jmdlEmbeddedIntArray_empty (pFaceId);
            numVertex = 0;
            MTGARRAY_FACE_LOOP (currNodeId, pGraph, startNodeId)
                {
                if (    ! jmdlMTGGraph_getMask (pGraph, currNodeId, MTG_EXTERIOR_MASK)
                    &&  jmdlMTGGraph_getLabel (pGraph, &vertexIndex, currNodeId, vertexIdOffset)
                    &&  vertexIndex >= 0 && vertexIndex < numIndex
                    )
                    {
                    numVertex++;
                    jmdlEmbeddedIntArray_addInt (pFaceId, pIndices[vertexIndex]);
                    }
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, startNodeId)

            if (numVertex > 2)
                {
                int *pIntArray = jmdlEmbeddedIntArray_getPtr (pFaceId, 0);
                if (reverseLoops)
                    {
                    reverseIntArray (pIntArray, numVertex);
                    }
                pFunc (pIntArray, numVertex, userData);
                }
            }
        }
    else
        {
        // Split normal case??
        }

    jmdlEmbeddedIntArray_drop (pFaceId);
    jmdlEmbeddedIntArray_drop (pFaceStart);
    }


/*---------------------------------------------------------------------------------**//**
* Output all interior face loops of the facet set as arrays of
* integer points, under callersupplied transformation.
* @param pFacetHeader    => mesh to emit
* @param pStartPolyFunc => function to start polygon
* @param pAddLoopFunc => function to add loop to polygon
* @param pFinishPolyFunc => function to end polygon
* @param pUserData1 => arg for 'finish' function
* @param pUserData2 => arg for 'finish' function
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_emitVArrayDPoint3dLoops
(
const MTGFacets *               pFacetHeader,
MTG_startPolygonFunction        pStartPolyFunc,
MTG_addLoopToPolygonFunction    pAddLoopFunc,
MTG_finishPolygonFunction       pFinishPolyFunc,
        void                    *pUserData1,
        void                    *pUserData2
)
    {
    MTGNodeId startNodeId;
    int vertexIndex;
    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    EmbeddedIntArray *pFaceStart = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pFaceId    = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pPointArray = jmdlEmbeddedDPoint3dArray_grab ();
    const EmbeddedDPoint3dArray *pVertexArray = (&pFacetHeader->vertexArrayHdr);
    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    DPoint3d point;
    int i;
    int numVertex;

    int nFace = jmdlMTGGraph_collectAndNumberFaceLoops(pGraph, pFaceId, pFaceId);

    for (i = 0; i < nFace; i++)
        {
        jmdlVArrayInt_getInt (pFaceStart, &startNodeId, i);
        jmdlEmbeddedDPoint3dArray_empty (pPointArray);
        numVertex = 0;
        MTGARRAY_FACE_LOOP (currNodeId, pGraph, startNodeId)
            {
            if (    ! jmdlMTGGraph_getMask (pGraph, currNodeId, MTG_EXTERIOR_MASK)
                &&  jmdlMTGGraph_getLabel (pGraph, &vertexIndex, currNodeId, vertexIdOffset)
                &&  jmdlEmbeddedDPoint3dArray_getDPoint3d (pVertexArray, &point, vertexIndex)
                )
                {
                numVertex++;
                pPointArray->push_back (point);
                }
            }
        MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, startNodeId)

        if (numVertex > 2)
            {
            pStartPolyFunc ();
            pAddLoopFunc (pPointArray);
            pFinishPolyFunc (pUserData1, pUserData2);
            }
        }

    jmdlEmbeddedDPoint3dArray_drop (pPointArray);
    jmdlEmbeddedIntArray_drop (pFaceId);
    jmdlEmbeddedIntArray_drop (pFaceStart);
    }

#ifdef COMPILE_HPOINTS

/*---------------------------------------------------------------------------------**//**
* Output all interior face loops of the facet set as arrays of
* integer points, under callersupplied transformation.
* @param pHPoints => header to receive added strokes
* @param pFacetHeader    => mesh to emit
* @param pEdgeStartArray => start nodes of edges to output
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
HPOINTSPublic GEOMDLLIMPEXP void jmdlMTGFacets_collectHPointsEdgeStrokes
(
const MTGFacets    *pFacetHeader,
      HPoints       *pHPoints,
      EmbeddedIntArray    *pEdgeStartArray
)
    {
    const MTGGraph * pGraph = (&pFacetHeader->graphHdr);
    const EmbeddedDPoint3dArray *pPointArray = (&pFacetHeader->vertexArrayHdr);
    int offset = pFacetHeader->vertexLabelOffset;
    int vertexIndex[2];
    DPoint3d vertexCoordinates[2];
    MTGNodeId startNodeId;
    MTGNodeId endNodeId;
    int nNode = jmdlEmbeddedIntArray_getCount (pEdgeStartArray);
    int i;


    for (i = 0; i < nNode; i++)
        {
        jmdlVArrayInt_getInt (pEdgeStartArray, &startNodeId, i);
        endNodeId = jmdlMTGGraph_getEdgeMate (pGraph, startNodeId);
        if (   (    !jmdlMTGGraph_getMask (pGraph, startNodeId, MTG_EXTERIOR_MASK)
                ||  !jmdlMTGGraph_getMask (pGraph, endNodeId, MTG_EXTERIOR_MASK)
               )
            && jmdlMTGGraph_getLabel (pGraph, &vertexIndex[0], startNodeId, offset)
            && jmdlMTGGraph_getLabel (pGraph, &vertexIndex[1], endNodeId, offset)
            && 2 == jmdlVArrayDPoint3d_getIndexedArray (pPointArray, vertexCoordinates, 2, vertexIndex, 2)
            )
            {
            jmdlHPoints_addDPoint3dArray (pHPoints, vertexCoordinates, 2);
            jmdlHPoints_markBreak (pHPoints);
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
*
* Search a veretx loop for an edge with a given vertex ids and outbound mask.
* If found, set mask1 and return the base node id.   If not found, create a dangling
* edge with mask0 set on the other side, mask1 on the near side, and the two indicated vertex indices.
* @param node0Id    => any known node at the start vertex
* @param vertex0    => vertex index at start
* @param node1Id    => any known node at the end vertex
* @param vertex1    => vertex index at end
* @param mask0       => optional mask which identifies edges to consider.  If mask0 is 0, no mask
*                       condition is enforced.
* @return           true if an existing node was found, false if a not (and a new one was created)
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool     findOrCreateMaskedEdgeInVertexLoop
(
MTGFacets *     pFacetHeader,
MTGNodeId      *pBaseNodeId,
MTGNodeId      node0Id,
int             vertex0,
MTGNodeId      node1Id,
int             vertex1,
MTGMask mask0,
MTGMask mask1
)
    {
    MTGGraph *pGraph = &pFacetHeader->graphHdr;
    int currVertex0, currVertex1;
    int vertexIdOffset = pFacetHeader->vertexLabelOffset;
    MTGNodeId mateNodeId;
    MTGNodeId newBaseNodeId, newMateNodeId;

    if (node0Id != MTG_NULL_NODEID && node1Id != MTG_NULL_NODEID)
        {
        MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, node0Id)
            {
            mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
            jmdlMTGGraph_getLabel (pGraph, &currVertex0, currNodeId, vertexIdOffset);
            jmdlMTGGraph_getLabel (pGraph, &currVertex1, mateNodeId, vertexIdOffset);
            if (   currVertex0 == vertex0
                && currVertex1 == vertex1
                && (!mask0 || jmdlMTGGraph_getMask (pGraph, currNodeId, mask0)))
                {
                if (mask1)
                    jmdlMTGGraph_setMask (pGraph, currNodeId, mask1);
                *pBaseNodeId = currNodeId;
                return true;
                }
            }
        MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, node0Id)
        }

    jmdlMTGGraph_createEdge (pGraph, &newBaseNodeId, &newMateNodeId);
    if (mask1)
        jmdlMTGGraph_setMask (pGraph, newBaseNodeId, mask1);
    if (mask0)
        jmdlMTGGraph_setMask (pGraph, newMateNodeId, mask0);

    jmdlMTGGraph_setLabel (pGraph, newBaseNodeId, vertexIdOffset, vertex0);
    jmdlMTGGraph_setLabel (pGraph, newMateNodeId, vertexIdOffset, vertex1);
    *pBaseNodeId = newBaseNodeId;
    return false;
    }

/*---------------------------------------------------------------------------------**//*
* Given an 'in face' index into a loop of vertex indices, extract the signed, one-based index
* and convert it to a zero-based index with display flag.
*
* @param        *pIndex <= zero-based index into vertex array.
* @param        *pDisplayed <= display flag
* @param        i => zero-based index into pVertexIndices, to be interpretted cyclically.
* @param        *pVertexIndices => array of signed, one-based vertex indices. Negative indices
*               indicated non-displayed edges.
* @param        numVertex => number of vertex indices given.   Do not duplicate first and last.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     decodeSignedIndex
(
        int     *pIndex,
        MTGNodeId *pNodeId,
        bool    *pDisplayed,
        int     i,
const   int     *pVertexIndexArray,
        int     numVertex,
        EmbeddedIntArray *pVertexToNodeArray_hdr
)
    {
    int index;
    /* Be prepared for small cyclic wraparound effects */
    if (i >= numVertex)
        i -= numVertex;
    else if (i < 0)
        i += numVertex;

    int rawIndex = pVertexIndexArray[i];
    if (pDisplayed)
        *pDisplayed = rawIndex > 0;
    index = abs (rawIndex) - 1;
    if (pIndex)
        *pIndex = index;

    if (index < jmdlEmbeddedIntArray_getCount (pVertexToNodeArray_hdr))
        {
        if (pNodeId)
            jmdlVArrayInt_getInt (pVertexToNodeArray_hdr, pNodeId, index);
        return true;

        }
    else
        {
        if (pNodeId)
            *pNodeId = MTG_NULL_NODEID;
        return false;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @param pTargetNodeid <= Node at start of next cluster.
* @param p
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool       probeCluster
(
MTGGraph   *pGraph,
MTGNodeId  *pTargetNodeId,
MTGNodeId  seedNodeId,
MTGMask    mask,
MTGNodeId  forbiddenNodeId
)
    {
    MTGNodeId currNodeId = seedNodeId;
    *pTargetNodeId = MTG_NULL_NODEID;

    if (seedNodeId != MTG_NULL_NODEID)
        {
        do
            {
            currNodeId = jmdlMTGGraph_getVSucc (pGraph, currNodeId);
            /* Note: This positive return may happen with either the seed or the barrier
                as the (masked) target.   This is fine. We just can't go past those as non-masks: */
            if (jmdlMTGGraph_getMask (pGraph, currNodeId, mask))
                {
                *pTargetNodeId = currNodeId;
                return true;
                }

            if (currNodeId == forbiddenNodeId)
                return false;

            } while (currNodeId != seedNodeId);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* A convenient place to put a breakpoint.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void jmdlMTGFacets_panic
(
)
    {
    }

/*---------------------------------------------------------------------------------**//**
* Final assembly of an indexed face.
*
* @param        pVertexToNodeId_hdr <=> indices from vertices to any nodeId in the vertex loops.
* @param        pVertexAroundFace => array of vertex indices around the face.
* @param        numVertexAroundFace => number of vertices around the face
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    assembleIndexedLoop
(
MTGFacets  *pFacetHeader,
EmbeddedIntArray  *pVertexToNodeId_hdr,
int         *pVertexIndexArray,
MTGNodeId  *pNodeIdArray,
bool        *pIsNewEdgeArray,
int         numVertexAroundFace
)
    {
    int i0, i1;
    MTGNodeId node0Id, node1Id, headNode0Id, node2Id, node3Id;
    int vertexIndex;
    bool       new0, new1;
    MTGGraph *pGraph = &pFacetHeader->graphHdr;
    bool    myStat = true;

    static int s_noisy = 0; /* Bit mask - 2 faces, 4 vertices, 8 local */

    if (s_noisy)
        {
        printf(" Assemble Face \n position vertex  node isNew\n");
        for (i0 = 0; i0 < numVertexAroundFace; i0++)
            {
            printf(" %8d  %5d %5d %5d\n",
                        i0,
                        pVertexIndexArray[i0],
                        pNodeIdArray[i0],
                        pIsNewEdgeArray[i0]
                        );
            }
        }

    for (i0 = 0; i0 < numVertexAroundFace ; i0++)
        {
        if (s_noisy & 0x10)
            jmdlMTGFacets_printFaceLoops (pFacetHeader);

        i1 = i0 + 1;
        if (i1 >= numVertexAroundFace)
            i1 = 0;
        node0Id = pNodeIdArray[i0];
        node1Id = pNodeIdArray[i1];
        headNode0Id = jmdlMTGGraph_getFSucc (pGraph, node0Id);
        new0    = pIsNewEdgeArray[i0];
        new1    = pIsNewEdgeArray[i1];
        vertexIndex = pVertexIndexArray[i1];


        if (!new0 && !new1)
            {
            /* This is ugly.  We are being told that two particular edges already at the same
                    vertex must be adjacent to each other. */
            if (headNode0Id == node1Id)
                {
                /* Got lucky - everything is as it should be */
                }
            else
                {
                /* The vertex consists of some number of clusters of adjacent edges with proper
                    ordering within each cluster.    The two nodes we are working with should be
                    the extremes of two clusters.  Clusters are delimited by MTG_EXTERIOR_MASK
                    edges.
                    Look for a delimiter:
                */
                if (probeCluster (pGraph, &node2Id, headNode0Id, node1Id, MTG_EXTERIOR_MASK))
                    {
                    /* Pull the intervening edges out, leaving node0..node1 as desire, and
                       headNode0Id the yanked edges: */
                    jmdlMTGGraph_vertexTwist (pGraph, headNode0Id, node1Id);
                    /* Put the yanked edges back in at the "other" available place: */
                    jmdlMTGGraph_vertexTwist (pGraph, headNode0Id, node2Id);
                    }
                else
                    {
                    myStat = false;
                    jmdlMTGFacets_panic ();
                    }
                }
            }
        else
            {
            jmdlMTGGraph_vertexTwist (pGraph, headNode0Id, node1Id);
            if (new0 && new1)
                {
                jmdlVArrayInt_getInt (pVertexToNodeId_hdr, &node2Id, vertexIndex);
                if (node2Id == MTG_NULL_NODEID)
                    {
                    jmdlVArrayInt_set (pVertexToNodeId_hdr, node1Id, vertexIndex);
                    }
                else
                    {
                    /* This is a new cluster at a prior vertex. Find an exterior sector to attach to: */
                    node3Id = jmdlMTGGraph_findMaskAroundVertex (pGraph, node2Id, MTG_EXTERIOR_MASK);
                    if (node3Id != MTG_NULL_NODEID)
                        {
                        jmdlMTGGraph_vertexTwist (pGraph, headNode0Id, node3Id);
                        }
                    else
                        {
                        myStat = false;
                        jmdlMTGFacets_panic ();
                        }
                    }
                }
            }
        }

    for (i0 = 0; i0 < numVertexAroundFace; i0++)
        {
        node0Id = pNodeIdArray[i0];
        jmdlMTGGraph_clearMask (pGraph, node0Id, MTG_EXTERIOR_MASK);
        }

    if (s_noisy & 0x12)
        jmdlMTGFacets_printFaceLoops (pFacetHeader);

    if (s_noisy & 0x04)
        jmdlMTGFacets_printVertexLoops (pFacetHeader);

    if (s_noisy & 0x08)
        jmdlMTGFacets_printConnectivity (pFacetHeader);

    return myStat;
    }
#define MAX_NODE_PER_LOOP 100


/*---------------------------------------------------------------------------------**//**
* Assemble a face from signed, one-based indices into a vertexToNode array.
*
* @param        pVertexToNodeId_hdr <=> indices from vertices to any nodeId in the vertex loops.
* @param        pVertexAroundFace => array of vertex indices around the face.
* @param        numVertexAroundFace => number of vertices around the face.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_addSignedIndexedFace
(
MTGFacets  *pFacetHeader,
EmbeddedIntArray  *pVertexToNodeId_hdr,
const int         *pVertexAroundFace,
int         numVertexAroundFace
)
    {
    int         i0;
    int         vert0Index, vert1Index, node0Id, node1Id;
    bool        displayed;

    MTGNodeId  savedNodeId[MAX_NODE_PER_LOOP];
    bool        savedBool[MAX_NODE_PER_LOOP];
    int         zeroBasedVertexIndex[MAX_NODE_PER_LOOP];

    if (numVertexAroundFace > MAX_NODE_PER_LOOP)
        return false;


    for (i0 = 0; i0 < numVertexAroundFace; i0++)
        {
        if (   decodeSignedIndex (&vert0Index, &node0Id, &displayed, i0,
                                    pVertexAroundFace, numVertexAroundFace, pVertexToNodeId_hdr)
            && decodeSignedIndex (&vert1Index, &node1Id, NULL,      i0 + 1,
                                    pVertexAroundFace, numVertexAroundFace, pVertexToNodeId_hdr))
            {
            zeroBasedVertexIndex[i0] = vert0Index;
            savedBool[i0] = !findOrCreateMaskedEdgeInVertexLoop (
                                            pFacetHeader,
                                            &savedNodeId[i0],
                                            node0Id,
                                            vert0Index,
                                            node1Id,
                                            vert1Index,
                                            MTG_EXTERIOR_MASK,
                                            displayed ? MTG_PRIMARY_EDGE_MASK : 0
                                            );
            }
        }

    return assembleIndexedLoop (
                        pFacetHeader,
                        pVertexToNodeId_hdr,
                        zeroBasedVertexIndex,
                        savedNodeId,
                        savedBool,
                        numVertexAroundFace
                        );
    }


/*---------------------------------------------------------------------------------**//**
* Search the param1 array for all vertices that fall within a specified parametric circle
* @param    pFacets <=> Containing graph
* @param    pIntArray <= array of integer vertex id's.
* @param        u => center u of search circle.
* @param        v => center v of search circle.
* @param        r => radius of search circle.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_searchVertexIdByParameterCircle
(
MTGFacets               *pFacets,
EmbeddedIntArray                *pIntArray,
double                  u,
double                  v,
double                  r
)
    {
    jmdlEmbeddedIntArray_empty (pIntArray);
    int numPoint = jmdlEmbeddedDPoint3dArray_getCount (&pFacets->param1ArrayHdr);
    DPoint3d *pParams = jmdlEmbeddedDPoint3dArray_getPtr (&pFacets->param1ArrayHdr, 0);
    double rr = r * r;
    double dx, dy;

    int i;
    for (i = 0; i < numPoint; i++)
        {
        dx = pParams[i].x - u;
        dy = pParams[i].y - v;
        if (dx * dx + dy * dy <= rr)
            {
            jmdlEmbeddedIntArray_addInt (pIntArray, i);
            }
        }

    }


/*---------------------------------------------------------------------------------**//**
* Search the param1 array for all vertices that fall within a specified
* spatial sphere.
* @param    pFacets <=> Containing graph
* @param    pIntArray <= array of integer vertex id's.
* @param    pCenter => center of sphere.
* @param        r => radius of search circle.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_searchVertexIdByXYZSphere
(
MTGFacets               *pFacets,
EmbeddedIntArray                *pIntArray,
const DPoint3d          *pCenter,
double                  r
)
    {
    jmdlEmbeddedIntArray_empty (pIntArray);
    int numPoint = jmdlEmbeddedDPoint3dArray_getCount (&pFacets->vertexArrayHdr);
    DPoint3d *pParams = jmdlEmbeddedDPoint3dArray_getPtr (&pFacets->vertexArrayHdr, 0);
    double rr = r * r;
    double dx, dy, dz;

    int i;
    for (i = 0; i < numPoint; i++)
        {
        dx = pParams[i].x - pCenter->x;
        dy = pParams[i].y - pCenter->y;
        dz = pParams[i].z - pCenter->z;

        if (dx * dx + dy * dy + dz * dz <= rr)
            {
            jmdlEmbeddedIntArray_addInt (pIntArray, i);
            }
        }

    }



/*---------------------------------------------------------------------------------**//**
* Return the index of any triangular interior face which contains the specified
* uv parametric coordinates.
* @param    pFacets <=> Containing graph
* @param    pBarycentric => barycentric coordinates within triangle.  May be NULL.
* @param    pNode0Id => first node id on triangle.  May be NULL.
* @param    pNode1Id => second node id on triangle.  May be NULL.
* @param    pNodeId2 => third node id on triangle.  May be NULL.
* @param    pParam0 => parametric coordinates of first vertex.  May be NULL.
* @param    pParam1 => parametric coordinates of second vertex.  May be NULL.
* @param    pParam2 => parametric coordinates of third vertex.  May be NULL.
* @param        u => center u of search point.
* @param        v => center v of search point.
* @return true if a triangle was located.  If false, output valuees are left uninitialized.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_searchTriangleByParameter
(
MTGFacets       *pFacets,
DPoint3d        *pBarycentric,
MTGNodeId       *pNode0Id,
MTGNodeId       *pNode1Id,
MTGNodeId       *pNode2Id,
DPoint3d        *pParam0,
DPoint3d        *pParam1,
DPoint3d        *pParam2,
int             *pVertex0Index,
int             *pVertex1Index,
int             *pVertex2Index,
double          u,
double          v
)
    {
    MTGGraph *pGraph = &pFacets->graphHdr;
    MTGMask mask = jmdlMTGGraph_grabMask (pGraph);
    MTGNodeId node1Id, node2Id, node3Id;
    int vertex0Index, vertex1Index, vertex2Index;
    DPoint3d uv0, uv1, uv2, barycentric;
    DPoint2d uv;
    int vertexOffset = pFacets->vertexLabelOffset;
    bool    boolStat = false;

    uv.x = u;
    uv.y = v;

    MTGARRAY_SET_LOOP (node0Id, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, node0Id, mask))
            {
            jmdlMTGGraph_setMaskAroundFace (pGraph, node0Id, mask);
            node1Id = jmdlMTGGraph_getFSucc (pGraph, node0Id);
            node2Id = jmdlMTGGraph_getFSucc (pGraph, node1Id);
            node3Id = jmdlMTGGraph_getFSucc (pGraph, node2Id);
            if (   node3Id == node0Id
                && node2Id != node0Id
                && !jmdlMTGGraph_getMask (pGraph, node0Id, MTG_EXTERIOR_MASK)
                && jmdlMTGGraph_getLabel (pGraph, &vertex0Index, node0Id, vertexOffset)
                && jmdlMTGGraph_getLabel (pGraph, &vertex1Index, node1Id, vertexOffset)
                && jmdlMTGGraph_getLabel (pGraph, &vertex2Index, node2Id, vertexOffset)
                && jmdlEmbeddedDPoint3dArray_getDPoint3d
                            (&pFacets->param1ArrayHdr, &uv0, vertex0Index)
                && jmdlEmbeddedDPoint3dArray_getDPoint3d
                            (&pFacets->param1ArrayHdr, &uv1, vertex1Index)
                && jmdlEmbeddedDPoint3dArray_getDPoint3d
                            (&pFacets->param1ArrayHdr, &uv2, vertex1Index)
                && bsiDPoint3d_barycentricFromDPoint2dTriangle (&barycentric, &uv,
                                (DPoint2d *)&uv0, (DPoint2d*)&uv1, (DPoint2d *)&uv2)
                )
                {
                if (barycentric.x >= 0.0 && barycentric.y >= 0.0 && barycentric.z >= 0.0)
                    {

                    if (pNode0Id)
                        *pNode0Id = node0Id;
                    if (pNode1Id)
                        *pNode1Id = node1Id;
                    if (pNode2Id)
                        *pNode2Id = node2Id;

                    if (pParam0)
                        *pParam0 = uv0;
                    if (pParam1)
                        *pParam1 = uv1;
                    if (pParam2)
                        *pParam2 = uv2;
                    if (pBarycentric)
                        *pBarycentric = barycentric;

                    if (pVertex0Index)
                        *pVertex0Index = vertex0Index;
                    if (pVertex1Index)
                        *pVertex1Index = vertex1Index;
                    if (pVertex2Index)
                        *pVertex2Index = vertex2Index;

                    boolStat = true;
                    goto cleanup;
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (node0Id, pGraph)

cleanup:
    jmdlMTGGraph_dropMask (pGraph, mask);
    return boolStat;
    }


/*---------------------------------------------------------------------------------**//**
* Regularize the graph using xy coordinates in the vertex array.
* @param   pFacets  <=> Containing facets
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_regularizeXY
(
MTGFacets               *pFacets
)
    {
    return jmdlMTGReg_regularize
                    (
                    &pFacets->graphHdr,
                    pFacets->vertexLabelOffset,
                    &pFacets->vertexArrayHdr
                    );
    }


/*---------------------------------------------------------------------------------**//**
* Regularize the graph using uv coordinates in the parameter array.
* @param   pFacets  <=> Containing facets
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_regularizeUV
(
MTGFacets               *pFacets
)
    {
    return jmdlMTGReg_regularize
                    (
                    &pFacets->graphHdr,
                    pFacets->vertexLabelOffset,
                    &pFacets->param1ArrayHdr
                    );
    }


/*---------------------------------------------------------------------------------**//**
* Triangulate the graph using xy coordinates in the vertex array.
* @param  pFacets   <=> Containing facets
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_triangulateXY
(
MTGFacets               *pFacets
)
    {
    return jmdlMTGReg_regularize
                    (
                    &pFacets->graphHdr,
                    pFacets->vertexLabelOffset,
                    &pFacets->vertexArrayHdr
                    )
        && jmdlMTGTri_triangulateAllCCWRegularFacesByVerticalSweep
                    (
                    &pFacets->graphHdr,
                    pFacets->vertexLabelOffset,
                    &pFacets->vertexArrayHdr
                    )
        && jmdlMTGParity_markFaceParityFromNegativeAreaFaces
                    (
                    &pFacets->graphHdr,
                    pFacets->vertexLabelOffset,
                    &pFacets->vertexArrayHdr,
                    MTG_PRIMARY_EDGE_MASK,
                    MTG_EXTERIOR_MASK
                    );
    }


/*---------------------------------------------------------------------------------**//**
* Triangulate the graph using uv coordinates in the parameter array.
* @param  pFacets   <=> Containing facets
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_triangulateUV
(
MTGFacets               *pFacets
)
    {
    return jmdlMTGReg_regularize
                    (
                    &pFacets->graphHdr,
                    pFacets->vertexLabelOffset,
                    &pFacets->param1ArrayHdr
                    )
        && jmdlMTGTri_triangulateAllCCWRegularFacesByVerticalSweep
                    (
                    &pFacets->graphHdr,
                    pFacets->vertexLabelOffset,
                    &pFacets->param1ArrayHdr
                    )
        && jmdlMTGParity_markFaceParityFromNegativeAreaFaces
                    (
                    &pFacets->graphHdr,
                    pFacets->vertexLabelOffset,
                    &pFacets->param1ArrayHdr,
                    MTG_PRIMARY_EDGE_MASK,
                    MTG_EXTERIOR_MASK
                    );
    }


/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to improve aspect ratio using xy coordinates
* in the vertex array.
* @param  pFacets   <=> Containing facets
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipXY
(
MTGFacets               *pFacets
)
    {
    return jmdlMTGSwap_flipTrianglesToImproveQuadraticAspectRatio
                (
                &pFacets->graphHdr,
                pFacets->vertexLabelOffset,
                &pFacets->vertexArrayHdr,
                MTG_PRIMARY_EDGE_MASK
                );
    }


/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to improve aspect ratio using xyz coordinates
* in the vertex array.
* @param  pFacets   <=> Containing facets
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipXYZ
(
MTGFacets               *pFacets
)
    {
    return jmdlMTGSwap_flipTrianglesToImproveQuadraticXYZAspectRatio
                (
                &pFacets->graphHdr,
                pFacets->vertexLabelOffset,
                &pFacets->vertexArrayHdr,
                MTG_PRIMARY_EDGE_MASK
                );
    }


/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to improve aspect ratio using xyz coordinates
* in the vertex array.
* @param  pFacets   <=> Containing facets
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipDihedral
(
MTGFacets               *pFacets
)
    {
    return jmdlMTGSwap_flipTrianglesToImproveDihedralAngle
                (
                &pFacets->graphHdr,
                pFacets->vertexLabelOffset,
                &pFacets->vertexArrayHdr,
                MTG_PRIMARY_EDGE_MASK
                );
    }



/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to improve aspect ratio using xyz coordinates
* in the vertex array.   Only "ruled" edges are flipped -- must run from one
* rail to another both before and after flip.
* @param  pFacets   <=> Containing facets
* @param railMask => mask identifying rail edges.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipRuledXYZ
(
MTGFacets               *pFacets,
MTGMask                 railMask
)
    {
    return jmdlMTGSwap_flipTrianglesToImproveQuadraticRuledXYZAspectRatio
                (
                &pFacets->graphHdr,
                pFacets->vertexLabelOffset,
                &pFacets->vertexArrayHdr,
                MTG_PRIMARY_EDGE_MASK,
                railMask
                );
    }


/*---------------------------------------------------------------------------------**//**
* Flip triangles in the the graph to improve aspect ratio using uv coordinates
* in the vertex array.
* @param  pFacets   <=> Containing facets
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t jmdlMTGFacets_flipUV
(
MTGFacets               *pFacets
)
    {
    return jmdlMTGSwap_flipTrianglesToImproveQuadraticAspectRatio
                (
                &pFacets->graphHdr,
                pFacets->vertexLabelOffset,
                &pFacets->param1ArrayHdr,
                MTG_PRIMARY_EDGE_MASK
                );
    }


/*---------------------------------------------------------------------------------**//**
* Use UV area to identify true exterior faces; start recursive traversal
* from each negative area face to set exterior masks according to parity
* rules as boundary edges are crossed.
* @param  pFacets   <=> Containing facets
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_setUVExteriorMasks
(
MTGFacets               *pFacets
)
    {
    return jmdlMTGParity_markFaceParityFromNegativeAreaFaces
                (
                &pFacets->graphHdr,
                pFacets->vertexLabelOffset,
                &pFacets->param1ArrayHdr,
                MTG_PRIMARY_EDGE_MASK,
                MTG_EXTERIOR_MASK
                );
    }

/*----------------------------------------------------------------------------------*//**
* Recursively search nodes at the vertex of the given node for an edge ending at
* the vertex with the given data (some of which may be NULL, with coordinates
* the most reliable).  If thruOffset is 0, recursion is not performed; the
* search only considers the immediate vertex loop; otherwise, recursion proceeds
* to adjacent vertex loops and stops after surpassing a nesting depth of
* maxDepth.
*
* @param        pFacets         => MTG Facets
* @param        nodeId          => node at base vertex
* @param        oppNodeId       => node at opposite vertex
* @param        oppVertexIndex  => index of opposite vertex (may not be unique)
* @param        pOppVertex      => coordinates of opposite vertex
* @param        tol2            => square of absolute tolerance for comparing opposite vertex coords
* @param        thruOffset      => label offset to "other side" of face (or 0)
* @param        maxDepth        => maximum nested calls in recursion (if thruOffset > 0)
* @param        depth           => level of recursion (set to 0 initially)
* @return base nodeId of edge if found, or MTG_NULL_NODEID
* @bsimethod                                                    DavidAssaf      04/01
+---------------+---------------+---------------+---------------+---------------+------*/
static MTGNodeId   recursive_findEdgeAtVertexCloud
(
const   MTGFacets*  pFacets,
        MTGNodeId   nodeId,
        MTGNodeId   oppNodeId,
        int         oppVertexIndex,
const   DPoint3d*   pOppVertex,
        double      tol2,
        int         thruOffset,
        int         maxDepth,
        int         depth
)
    {
    const EmbeddedDPoint3dArray*    pVerts = &pFacets->vertexArrayHdr;
    const MTGGraph*                 pGraph = &pFacets->graphHdr;
    DPoint3d    v;
    MTGNodeId   n;
    int         i;

    if (nodeId == MTG_NULL_NODEID || depth > maxDepth)
        return MTG_NULL_NODEID;

    // search v-loop of given node
    MTGARRAY_VERTEX_LOOP (baseId, pGraph, nodeId)
        {
        n = jmdlMTGGraph_getFSucc (pGraph, baseId);
        if (MTG_NULL_NODEID == n)
            return MTG_NULL_NODEID;

        // test opposite nodeID/vertexID/vertex
        if (n == oppNodeId ||
            (jmdlMTGFacets_getNodeVertexIndex (pFacets, &i, n) && (i == oppVertexIndex)) ||
            (jmdlEmbeddedDPoint3dArray_getDPoint3d (pVerts, &v, i) && (v.DistanceSquared (*pOppVertex) <= tol2)))
            return baseId;  // Now caller can loop over edgestar to find the desired edge
        }
    MTGARRAY_END_VERTEX_LOOP (baseId, pGraph, nodeId)

    // search adjacent v-loops
    if (thruOffset > 0)
        {
        MTGARRAY_VERTEX_LOOP (baseId, pGraph, nodeId)
            {
            n = jmdlMTGGraph_getFacePartner (pGraph, baseId, thruOffset);

            if (jmdlMTGGraph_isValidNodeId (pGraph, n))
                {
                n = recursive_findEdgeAtVertexCloud (pFacets, n, oppNodeId, oppVertexIndex, pOppVertex, tol2, thruOffset, maxDepth,
                                                     depth + 1);

                if (MTG_NULL_NODEID != n)
                    return n;
                }
            }
        MTGARRAY_END_VERTEX_LOOP (baseId, pGraph, nodeId)
        }

    return MTG_NULL_NODEID;
    }


/*----------------------------------------------------------------------------------*//**
* Search nodes at the vertex of the given node for an edge ending at the vertex
* at which oppNode lies.  If thruOffset is 0, the search only considers the
* immediate vertex loop; otherwise, adjacent vertex loops are also searched,
* up to the maximum recursion nesting level given in maxDepth.
*
* @param        pFacets         => MTG Facets
* @param        nodeId          => node at base vertex
* @param        oppNodeId       => node at opposite vertex
* @param        tol2            => square of absolute tolerance for comparing opposite vertex coords
* @param        thruOffset      => label offset to "other side" of face (or 0)
* @param        maxDepth        => maximum nested calls in recursion (if thruOffset > 0)
* @return base nodeId of edge if found, or MTG_NULL_NODEID
* @bsimethod                                                    DavidAssaf      04/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGFacets_findEdgeAtVertexCloud
(
const MTGFacets *pFacets,
MTGNodeId       nodeId,
MTGNodeId       oppNodeId,
double          tol2,
int             thruOffset,
int             maxDepth
)
    {
    DPoint3d    v;
    int         i;

    if (   jmdlMTGFacets_getNodeCoordinates (pFacets, &v, oppNodeId)
        && jmdlMTGFacets_getNodeVertexIndex (pFacets, &i, oppNodeId))
        {
        return recursive_findEdgeAtVertexCloud (pFacets, nodeId, oppNodeId, i,
                    &v, tol2, thruOffset, maxDepth, 0);
        }

    return MTG_NULL_NODEID;
    }


/*----------------------------------------------------------------------------------*//**
* Compares given vertices to those of both faces attached at this edge.  The
* nodeId output on a successful return corresponds to the first given vertex,
* even when those vertices match the MTG face in the opposite orientation.
*
* @param        pFacets     => MTG Facets
* @param        pbFlipped   <= true if match found to reversed vertex order (or NULL)
* @param        pNodeId     <=> edge node (on return, may be vertex predecessor)
* @param        pXYZ        => vertex coords to compare
* @param        numXYZ      => # vertices to compare
* @param        tol2        => square of absolute tolerance for comparing vertex coords
* @param        presentMask => one of these masks must be present on matched face (or zero)
* @param        absentMask  => none of these masks must be present on matched face (or zero)
* @return true if a match was found
* @bsimethod                                                    DavidAssaf      09/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_matchMaskedFaceAtEdge
(
const   MTGFacets*  pFacets,
        bool*       pbFlipped,
        MTGNodeId*  pNodeId,
const   DPoint3d*   pXYZ,
        int         numXYZ,
        double      tol2,
        MTGMask     presentMask,
        MTGMask     absentMask
)
    {
    EmbeddedDPoint3dArray*  pMTGArray = jmdlEmbeddedDPoint3dArray_grab();
    const MTGGraph*         pGraph;
    MTGNodeId               n = *pNodeId;
    DPoint3d*               pMTGXYZ;
    bool                    bFound = false;
    int                     i, j, k;

    if (!pFacets || !pNodeId || !pXYZ || !pMTGArray)
        return false;

    pGraph = &pFacets->graphHdr;

    for (i = 0; i < 2; i++)
        {
        jmdlMTGFacets_getFaceCoordinates (pGraph, pMTGArray, &pFacets->vertexArrayHdr, n, pFacets->vertexLabelOffset);

        if (numXYZ == jmdlEmbeddedDPoint3dArray_getCount (pMTGArray))
            {
            pMTGXYZ = jmdlEmbeddedDPoint3dArray_getPtr (pMTGArray, 0);

            // check ccw direction
            for (j = 0; j < numXYZ; j++)
                if (tol2 < pMTGXYZ[j].DistanceSquared (pXYZ[j]))
                    break;

            if (j == numXYZ &&
                (!presentMask ||  jmdlMTGGraph_getMask (pGraph, n, presentMask)) &&
                (!absentMask  || !jmdlMTGGraph_getMask (pGraph, n, absentMask)))
                {
                *pNodeId = n;
                if (pbFlipped)
                    *pbFlipped = false;
                bFound = true;
                goto wrapup;
                }

            // check cw direction
            for (j = k = 0; j < numXYZ; j++, k = numXYZ - j)
                if (tol2 < pMTGXYZ[j].DistanceSquared (pXYZ[k]))
                    break;

            if (j == numXYZ &&
                (!presentMask ||  jmdlMTGGraph_getMask (pGraph, n, presentMask)) &&
                (!absentMask  || !jmdlMTGGraph_getMask (pGraph, n, absentMask)))
                {
                *pNodeId = n;
                if (pbFlipped)
                    *pbFlipped = true;
                bFound = true;
                goto wrapup;
                }
            }

        // try other side of edge
        n = jmdlMTGGraph_getVPred (pGraph, n);
        }

wrapup:
    jmdlEmbeddedDPoint3dArray_drop (pMTGArray);
    return bFound;
    }


/*----------------------------------------------------------------------------------*//**
* Compares given vertices to those of both faces attached at this edge.  The
* nodeId output on a successful return corresponds to the first given vertex,
* even when those vertices match the MTG face in the opposite orientation.
*
* @param        pFacets     => MTG Facets
* @param        pbFlipped   <= true if match found to reversed vertex order
* @param        pNodeId     <=> edge node (on return, may be vertex predecessor)
* @param        pXYZ        => vertex coords to compare
* @param        numXYZ      => # vertices to compare
* @param        tol2        => square of absolute tolerance for comparing vertex coords
* @return true if a match was found
* @bsimethod                                                    DavidAssaf      04/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_matchFaceAtEdge
(
const MTGFacets *pFacets,
bool            *pbFlipped,
MTGNodeId       *pNodeId,
const DPoint3d  *pXYZ,
int             numXYZ,
double          tol2
)
    {
    return jmdlMTGFacets_matchMaskedFaceAtEdge (pFacets, pbFlipped, pNodeId, pXYZ, numXYZ, tol2, 0, 0);
    }

// -----------------------------------------------------------------------------------
static int sNoisyCut = 0;
/*----------------------------------------------------------------------------------*//**
"Cut the corner" at every vertex.
Introduces a new face at each vertex.  Each edge receives two new points, at specified
  fractional distance from the ends.
* @bsimethod                                                    EarlinLutz  07/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_cutCorners
(
MTGFacets *pFacets,
double fraction
)
    {
    MTGGraph * pGraph = &pFacets->graphHdr;
    EmbeddedDPoint3dArray *pXYZArray = &pFacets->vertexArrayHdr;
    int vertexLabelOffset = pFacets->vertexLabelOffset;
    MTGMask originalEdgeMask = jmdlMTGGraph_grabMask (pGraph);
    MTGMask visitMask        = jmdlMTGGraph_grabMask (pGraph);

    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    jmdlMTGGraph_setMaskInSet (pGraph, originalEdgeMask);


    if (sNoisyCut)
        {
        jmdlMTGFacets_printFaceLoops (pFacets);
        jmdlMTGFacets_printVertexLoops (pFacets);
        }

    // At each VERTEX LOOP ...
    MTGARRAY_SET_LOOP (vertexSeedNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, vertexSeedNodeId, visitMask)
            && jmdlMTGGraph_getMask (pGraph, vertexSeedNodeId, originalEdgeMask))
            {
            jmdlMTGGraph_setMaskAroundVertex (pGraph, vertexSeedNodeId, visitMask);
            int numEdge = jmdlMTGGraph_countNodesAroundVertex (pGraph, vertexSeedNodeId);
            MTGNodeId ringNodeId = MTG_NULL_NODEID;
            DPoint3d xyz;
            int oldVertexIndex;
            jmdlMTGFacets_getNodeCoordinates (pFacets, &xyz, vertexSeedNodeId);
            jmdlMTGFacets_getNodeVertexIndex (pFacets, &oldVertexIndex, vertexSeedNodeId);
            if (sNoisyCut)
                printf (" Split Vertex (node %d) (xyzIndex %d) (xyz %lg,%lg,%lg)\n",
                                        vertexSeedNodeId, oldVertexIndex, xyz.x, xyz.y, xyz.z);
            for (int i = 0; i < numEdge; i++)
                {
                MTGNodeId leftNodeId, rightNodeId;
                // Grow the "new face" loop ..
                jmdlMTGGraph_splitEdge (pGraph, &leftNodeId, &rightNodeId, ringNodeId);
                // Look ahead to the "next" edge ... (on final pass the vertexSeedNodeId
                //          is the only one there)
                MTGNodeId activeEdgeNodeId = jmdlMTGGraph_getVSucc (pGraph, vertexSeedNodeId);
                // Yank the edge out ...
                jmdlMTGGraph_vertexTwist (pGraph, vertexSeedNodeId, activeEdgeNodeId);
                // insert it in the growing "new face" loop.
                jmdlMTGGraph_vertexTwist (pGraph, rightNodeId, activeEdgeNodeId);
                if (sNoisyCut)
                    printf (" Edge Splice R=%d L%d Yank=%d\n", leftNodeId, rightNodeId, activeEdgeNodeId);

                ringNodeId = leftNodeId;
                int vertexId;
                if (activeEdgeNodeId == vertexSeedNodeId)
                    {
                    // Reuse old vertex id.
                    vertexId = oldVertexIndex;
                    }
                else
                    {
                    // Generate a new vertex id, same coordinates.
                    vertexId = jmdlEmbeddedDPoint3dArray_getCount (&pFacets->vertexArrayHdr);
                    jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYZArray, &xyz);
                    }
                jmdlMTGGraph_setLabelAroundVertex (pGraph, activeEdgeNodeId, vertexLabelOffset, vertexId);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (vertexSeedNodeId, pGraph)

    // NEEDS WORK: propagate boundary, external edge masks.
    // For each original edge ...
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    MTGARRAY_SET_LOOP (nodeIdA, pGraph)
        {
        if (jmdlMTGGraph_getMask (pGraph, nodeIdA, originalEdgeMask)
            && !jmdlMTGGraph_getMask (pGraph, nodeIdA, visitMask))
            {
            MTGNodeId nodeIdB = jmdlMTGGraph_getEdgeMate (pGraph, nodeIdA);
            jmdlMTGGraph_setMask (pGraph, nodeIdA, visitMask);
            jmdlMTGGraph_setMask (pGraph, nodeIdB, visitMask);
            DPoint3d xyzA0, xyzB0;
            DPoint3d xyzA1, xyzB1;
            jmdlMTGFacets_getNodeCoordinates (pFacets, &xyzA0, nodeIdA);
            jmdlMTGFacets_getNodeCoordinates (pFacets, &xyzB0, nodeIdB);
            xyzA1.Interpolate (xyzA0, fraction, xyzB0);
            xyzB1.Interpolate (xyzB0, fraction, xyzA0);
            jmdlMTGFacets_setNodeCoordinates (pFacets, &xyzA1, nodeIdA);
            jmdlMTGFacets_setNodeCoordinates (pFacets, &xyzB1, nodeIdB);
            if (sNoisyCut)
                printf (" Edge Retract %d %d\n", nodeIdA, nodeIdB);
            }
        }
    MTGARRAY_END_SET_LOOP (nodeIdA, pGraph);
    jmdlMTGGraph_dropMask (pGraph, originalEdgeMask);
    jmdlMTGGraph_dropMask (pGraph, visitMask);
    if (sNoisyCut)
        jmdlMTGFacets_printFaceLoops (pFacets);

    }

static void printNodeABCD
(
MTGGraph *pGraph,
MTGNodeId nodeId,
MTGMask maskAA,
MTGMask maskBB,
MTGMask maskCC,
MTGMask maskDD,
char const*pSuffix
)
    {
    int vertexIndex;
    jmdlMTGGraph_getLabel (pGraph, &vertexIndex, nodeId, 0);
    printf ("   (node %3d:%3d)", nodeId, vertexIndex);
    if (jmdlMTGGraph_getMask (pGraph, nodeId, maskAA))
        printf ("A");
    if (jmdlMTGGraph_getMask (pGraph, nodeId, maskBB))
        printf ("B");
    if (jmdlMTGGraph_getMask (pGraph, nodeId, maskCC))
        printf ("C");
    if (jmdlMTGGraph_getMask (pGraph, nodeId, maskDD))
        printf ("D");
    printf ("%s", pSuffix);
    }


/*----------------------------------------------------------------------------------*//**
Quadratic style subdivsion step -- build a parallel edge and its collapsed cap.
Return
* @bsimethod                                                    EarlinLutz  07/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool splitEdgeABCD
(
MTGFacets *pFacets,
MTGGraph *pGraph,
MTGNodeId nodeIdA0,
MTGMask maskAA,
MTGMask maskBB,
MTGMask maskCC,
MTGMask maskDD,
MTGMask maskABCD
)
    {
    // Already split?
    if (jmdlMTGGraph_getMask (pGraph, nodeIdA0, maskABCD))
        return false;

    int vertexLabelOffset = pFacets->vertexLabelOffset;

    // Far side of the existing edge will end up "inside" -- it is a B
    MTGNodeId nodeIdB1 = jmdlMTGGraph_getEdgeMate (pGraph, nodeIdA0);

    // If on the inside of an exterior edge, do it from the other end:
    if (jmdlMTGGraph_getMask (pGraph, nodeIdB1, MTG_EXTERIOR_MASK))
        {
        MTGNodeId tempNodeId = nodeIdA0;
        nodeIdA0 = nodeIdB1;
        nodeIdB1 = tempNodeId;
        }
    // nodeId<A,B,C,D><0,1> are the A,B,C,D nodes at near and far ends.
    MTGNodeId nodeIdB0, nodeIdA1, nodeIdC0, nodeIdC1, nodeIdD0, nodeIdD1;
    // Need the vertex predecessor at A to twist in the parallel edge ...
    // Get to it from B1 ...
    MTGNodeId nodeIdQ = jmdlMTGGraph_getFSucc (pGraph, nodeIdB1);
    //  Make the parallel edge ....
    jmdlMTGGraph_join (pGraph, &nodeIdB0, &nodeIdA1, nodeIdQ, nodeIdB1, MTG_NULL_MASK, MTG_NULL_MASK);
    // Insert a sling at each end.  The "inside" of the sling
    // will be exposed to the vertex later.
    jmdlMTGGraph_createSling (pGraph, &nodeIdC0, &nodeIdD0);
    jmdlMTGGraph_vertexTwist (pGraph,  nodeIdB0,  nodeIdD0);
    jmdlMTGGraph_createSling (pGraph, &nodeIdC1, &nodeIdD1);
    jmdlMTGGraph_vertexTwist (pGraph,  nodeIdB1,  nodeIdD1);

    jmdlMTGGraph_setMask (pGraph, nodeIdA0, maskAA);
    jmdlMTGGraph_setMask (pGraph, nodeIdA1, maskAA);

    jmdlMTGGraph_setMask (pGraph, nodeIdB0, maskBB);
    jmdlMTGGraph_setMask (pGraph, nodeIdB1, maskBB);

    jmdlMTGGraph_setMask (pGraph, nodeIdC0, maskCC);
    jmdlMTGGraph_setMask (pGraph, nodeIdC1, maskCC);

    jmdlMTGGraph_setMask (pGraph, nodeIdD0, maskDD);
    jmdlMTGGraph_setMask (pGraph, nodeIdD1, maskDD);

    // spread the old vertex id's around ..
    int vertexIndex0, vertexIndex1;
    jmdlMTGFacets_getNodeVertexIndex (pFacets, &vertexIndex0, nodeIdA0);
    jmdlMTGFacets_getNodeVertexIndex (pFacets, &vertexIndex1, nodeIdB1);

    jmdlMTGGraph_setLabel (pGraph, nodeIdB0, vertexLabelOffset, vertexIndex0);
    jmdlMTGGraph_setLabel (pGraph, nodeIdC0, vertexLabelOffset, vertexIndex0);
    jmdlMTGGraph_setLabel (pGraph, nodeIdD0, vertexLabelOffset, vertexIndex0);

    jmdlMTGGraph_setLabel (pGraph, nodeIdA1, vertexLabelOffset, vertexIndex1);
    jmdlMTGGraph_setLabel (pGraph, nodeIdC1, vertexLabelOffset, vertexIndex1);
    jmdlMTGGraph_setLabel (pGraph, nodeIdD1, vertexLabelOffset, vertexIndex1);

    if (sNoisyCut)
        {
        printf ("SPLIT EDGE (A0 %3d) (B0 %3d) (A1 %3d)(B1 %3d)\n", nodeIdA0, nodeIdB0, nodeIdA1, nodeIdB1);
        printf ("           (C0 %3d) (D0 %3d) (C1 %3d)(D1 %3d)\n", nodeIdC0, nodeIdD0, nodeIdC1, nodeIdD1);
        printf ("           (V0 %3d) (V1 %3d)\n", vertexIndex0, vertexIndex1);
        }
    return true;
    }

/*----------------------------------------------------------------------------------*//**
Quadratic style subdivsion
Expand each vertex to a face (same degree as vertex), and each edge to a quad face.
Place new vertices midway towards centroid of face.
* @bsimethod                                                    EarlinLutz  07/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_subdivideA
(
MTGFacets *pFacets
)
    {
    MTGGraph * pGraph = &pFacets->graphHdr;
    EmbeddedDPoint3dArray *pXYZArray = &pFacets->vertexArrayHdr;
    int vertexLabelOffset = pFacets->vertexLabelOffset;
    MTGMask visitMask        = jmdlMTGGraph_grabMask (pGraph);
    // nodes (half edges) will appear in 4 flavors:
    // -- AA parallel to orginal edge, on "old" face side.
    // -- BB mates of AA, parallel to original edge, on "quad" side.
    // -- CC around vertex on "inside" of new face.
    // -- DD mates of CC, on quad side.
    // Get a mask for each.  Could be encoded in two, but let's be direct.
    MTGMask maskAA = jmdlMTGGraph_grabMask (pGraph);
    MTGMask maskBB = jmdlMTGGraph_grabMask (pGraph);
    MTGMask maskCC = jmdlMTGGraph_grabMask (pGraph);
    MTGMask maskDD = jmdlMTGGraph_grabMask (pGraph);
    MTGMask maskABCD = maskAA | maskBB | maskCC | maskDD;
    MTGMask allMasks = visitMask | maskAA | maskBB | maskCC | maskDD;
    jmdlMTGGraph_clearMaskInSet (pGraph, allMasks);
    EmbeddedIntArray *pNodeArray = jmdlEmbeddedIntArray_grab ();

    if (sNoisyCut)
        {
        jmdlMTGFacets_printFaceLoops (pFacets);
        jmdlMTGFacets_printVertexLoops (pFacets);
        }

    // For each original edge ...
    //   Make a "parallel" partner.
    //   Stick a sling at each end "inside" the parallel pair.
    //   Mark all 8 nodes as AA,BB,CC,DD.
    //  ABSENCE of any ABCD mask indicates original unvisited edge.
    MTGARRAY_SET_LOOP (nodeIdA0, pGraph)
        {
        splitEdgeABCD (pFacets, pGraph, nodeIdA0, maskAA, maskBB, maskCC, maskDD, maskABCD);
        }
    MTGARRAY_END_SET_LOOP (nodeIdA0, pGraph)

    if (sNoisyCut)
        {
        printf ("\n*************  DOUBLE EDGES, VERTEX BUDS *****************\n");
        jmdlMTGFacets_printFaceLoops (pFacets);
        jmdlMTGFacets_printVertexLoops (pFacets);
        }

    // At each vertex with K original edges, there are K incident bundles of ABCD nodes.
    // Do K-1 twists among the C nodes to form the degree K face at the vertex.
    // REMEMBER: twist(p,q) for nodes in DIFFERENT faces merges the faces.
    //       K-1 twists among the C nodes (each of which is a sling face to start with)
    //           merges the K faces into 1.

    MTGARRAY_SET_LOOP (vertexSeedNodeId, pGraph)
        {
        if (jmdlMTGGraph_getMask (pGraph, vertexSeedNodeId, maskCC)
            && !jmdlMTGGraph_getMask (pGraph, vertexSeedNodeId, visitMask))
            {
            jmdlMTGGraph_setMaskAroundVertex (pGraph, vertexSeedNodeId, visitMask);
            if (sNoisyCut)
                printf ("OPEN VERTEX (seed %3d)\n", vertexSeedNodeId);
            // Hello unvisited vertex loop.
            // We know that as we walk around the mask sequence will be
            // C,B (both within the starting bundle) then A,D in the next bundle.
            // build up an array of all the C nodes around the vertex ..
            // (Usually a very low number, but use the rubber array because we are changing topology
            //      as we go around the loop ...)
            jmdlEmbeddedIntArray_empty (pNodeArray);
            MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, vertexSeedNodeId)
                {
                if (jmdlMTGGraph_getMask (pGraph, currNodeId, maskCC))
                    jmdlEmbeddedIntArray_addInt (pNodeArray, currNodeId);
                if (sNoisyCut)
                    printNodeABCD (pGraph, currNodeId, maskAA, maskBB, maskCC, maskDD, "\n");
                }
            MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, vertexSeedNodeId)
            // Splice n-1 pairs....
            MTGNodeId nodeId0, nodeId1;
            jmdlEmbeddedIntArray_getInt (pNodeArray, &nodeId0, 0);
            for (int i = 1; jmdlEmbeddedIntArray_getInt (pNodeArray, &nodeId1, i); i++)
                {
                if (sNoisyCut)
                    printf ("   Twist to open %d %d\n", nodeId0, nodeId1);
                jmdlMTGGraph_vertexTwist (pGraph, nodeId0, nodeId1);
                }


            // Make new vertex ids (with same coordinate) for all but one of the loop that
            // was just expanded into a face "around" the old vertex.
            int oldVertexIndex;
            jmdlMTGGraph_getLabel (pGraph, &oldVertexIndex, vertexSeedNodeId, vertexLabelOffset);
            DPoint3d xyz;
            jmdlMTGFacets_getNodeCoordinates (pFacets, &xyz, vertexSeedNodeId);
            if (sNoisyCut)
                printf ("   seed for cut vertex face (node %3d) (OLD vertex index %d)\n", vertexSeedNodeId, oldVertexIndex);
            MTGARRAY_FACE_LOOP (currNodeId, pGraph, vertexSeedNodeId)
                {
                if (sNoisyCut)
                    printf ("   face in vertex (node %3d)\n", currNodeId);
                if (currNodeId != vertexSeedNodeId)
                    {
                    int newVertexIndex = jmdlEmbeddedDPoint3dArray_getCount (pXYZArray);
                    jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYZArray, &xyz);
                    if (sNoisyCut)
                        printf ("   NEW VERTEX (node %d) (vertex %d)\n", currNodeId, newVertexIndex);
                    MTGARRAY_VERTEX_LOOP (nodeIdZ, pGraph, currNodeId)
                        {
                        jmdlMTGGraph_setLabel (pGraph, nodeIdZ, vertexLabelOffset, newVertexIndex);
                        if (sNoisyCut)
                            printNodeABCD (pGraph, nodeIdZ, maskAA, maskBB, maskCC, maskDD, "\n");
                        }
                    MTGARRAY_END_VERTEX_LOOP (nodeIdZ, pGraph, currNodeId)
                    //jmdlMTGGraph_setLabelAroundVertex (pGraph, currNodeId, vertexLabelOffset, newVertexIndex);
                    }
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, vertexSeedNodeId)
            }
        }
    MTGARRAY_END_SET_LOOP (vertexSeedNodeId, pGraph);

    if (sNoisyCut)
        {
        printf ("\n*************  VERTEX BLOOM *****************\n");
        jmdlMTGFacets_printFaceLoops (pFacets);
        jmdlMTGFacets_printVertexLoops (pFacets);
        }

    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);

    // On each original face ...
    // Compute the centroid.  Move each vertex midway there.
    MTGARRAY_SET_LOOP (faceSeedNodeId, pGraph)
        {
        if (jmdlMTGGraph_getMask (pGraph, faceSeedNodeId, maskAA)
            && !jmdlMTGGraph_getMask (pGraph, faceSeedNodeId, visitMask))
            {
            jmdlMTGGraph_setMaskAroundFace (pGraph, faceSeedNodeId, visitMask);
            if (jmdlMTGGraph_getMask (pGraph, faceSeedNodeId, MTG_EXTERIOR_MASK))
                {
                // Exterior edge.  Leave it in place.
                // Hm.. maybe need option to insert a CD edge at each node and move them along the edges
                }
            else
                {
                if (sNoisyCut)
                    printf ("SHRINK FACE (seed %3d)\n", faceSeedNodeId);
                DPoint3d xyzCentroid, xyz;
                xyzCentroid.Zero ();
                int n = 0;
                MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceSeedNodeId)
                    {
                    jmdlMTGFacets_getNodeCoordinates (pFacets, &xyz, currNodeId);
                    xyzCentroid.Add (xyz);
                    n++;
                    }
                MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceSeedNodeId)

                bsiDPoint3d_scaleInPlace (&xyzCentroid, 1.0 / (double)n);
                if (sNoisyCut)
                    printf ("       (centroid %lg,  %lg,  %lg)\n", xyzCentroid.x, xyzCentroid.y, xyzCentroid.z);

                MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceSeedNodeId)
                    {
                    jmdlMTGFacets_getNodeCoordinates (pFacets, &xyz, currNodeId);
                    xyz.Interpolate (xyz, 0.5, xyzCentroid);
                    jmdlMTGFacets_setNodeCoordinates (pFacets, &xyz, currNodeId);
                    if (sNoisyCut)
                        printf ("     MOVE %d (xyz %lg,  %lg,  %lg)\n", currNodeId, xyz.x, xyz.y, xyz.z);
                    }
                MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceSeedNodeId)
                }
            }
        }
    MTGARRAY_END_SET_LOOP (faceSeedNodeId, pGraph)

    jmdlMTGGraph_dropMask (pGraph, visitMask);
    jmdlMTGGraph_dropMask (pGraph, maskAA);
    jmdlMTGGraph_dropMask (pGraph, maskBB);
    jmdlMTGGraph_dropMask (pGraph, maskCC);
    jmdlMTGGraph_dropMask (pGraph, maskDD);
    jmdlEmbeddedIntArray_drop (pNodeArray);
    if (sNoisyCut)
        jmdlMTGFacets_printFaceLoops (pFacets);
    }

class MTGCoordinateAssistant
{
public:
MTGFacets   *mpFacets;
MTGGraph    *mpGraph;
int         mVertexLabelOffset;
DPoint3d    mXYZ;
double      mWeight;
MTGCoordinateAssistant (MTGFacets *pFacets)
    {
    mpFacets = pFacets;
    mpGraph = &pFacets->graphHdr;
    mVertexLabelOffset = pFacets->vertexLabelOffset;
    Zero ();
    }
void Zero ()
    {
    mXYZ.x = mXYZ.y = mXYZ.z = mWeight = 0.0;
    }

// Accumulate a point with weight.
bool Accumulate (DPoint3dCR xyz, double weight)
    {
    mXYZ.x += xyz.x * weight;
    mXYZ.y += xyz.y * weight;
    mXYZ.z += xyz.z * weight;
    mWeight += weight;
    return true;
    }

// Accumulate a point, with weight 1.
bool Accumulate (MTGNodeId nodeId)
    {
    DPoint3d xyzi;
    if (jmdlMTGFacets_getNodeCoordinates (mpFacets, &xyzi, nodeId))
        return Accumulate (xyzi, 1.0);
    return false;
    }

// Accumulate a point, with weight 1, accessing coordinates in given array
bool Accumulate (MTGNodeId nodeId, EmbeddedDPoint3dArray *pXYZArray)
    {
    DPoint3d xyz;
    int index;
    if (jmdlMTGFacets_getNodeVertexIndex (mpFacets, &index, nodeId)
        && jmdlEmbeddedDPoint3dArray_getDPoint3d (pXYZArray, &xyz, index))
        return Accumulate (xyz, 1.0);
    return false;
    }


// Accumulate a point with weight.
bool Accumulate (MTGNodeId nodeId, double weight)
    {
    DPoint3d xyzi;
    if (jmdlMTGFacets_getNodeCoordinates (mpFacets, &xyzi, nodeId))
        return Accumulate (xyzi, weight);
    return false;
    }

bool NormalizeWeight ()
    {
    if (mWeight > 0)
        {
        double a = 1.0 / mWeight;
        mXYZ.x *= a;
        mXYZ.y *= a;
        mXYZ.z *= a;
        mWeight = 1;
        return true;
        }
    return false;
    }

// Normalize coordinates.  Save the coordiantes in the vertex referenced by the given node.
bool SaveAsReplacement (MTGNodeId nodeId)
    {
    return NormalizeWeight ()
        && jmdlMTGFacets_setNodeCoordinates (mpFacets, &mXYZ, nodeId);
    }

// Normalize coordinates.  Create a new vertex in the vertex array.   Save the new vertex id around
//   a vertex loop.
bool SaveAsNewVertex (MTGNodeId nodeId)
    {
    if (NormalizeWeight ())
        {
        int vertexIndex = jmdlEmbeddedDPoint3dArray_getCount (&mpFacets->vertexArrayHdr);
        jmdlEmbeddedDPoint3dArray_addDPoint3d (&mpFacets->vertexArrayHdr, &mXYZ);
        jmdlMTGGraph_setLabelAroundVertex (mpGraph, nodeId, mVertexLabelOffset, vertexIndex);
        }
    return false;
    }

void AverageNeighbors (MTGNodeId seedNodeId)
    {
    Zero ();
    MTGARRAY_VERTEX_LOOP (nearNodeId, mpGraph, seedNodeId)
        {
        MTGNodeId farNodeId = jmdlMTGGraph_getFSucc (mpGraph, nearNodeId);
        Accumulate (farNodeId);
        }
    MTGARRAY_END_VERTEX_LOOP (nearNodeId, mpGraph, seedNodeId)
    NormalizeWeight ();
    }


};
/*----------------------------------------------------------------------------------*//**
Catmull Clark subdivision
Expand each vertex to a face (same degree as vertex), and each edge to a quad face.
Place new vertices midway towards centroid of face.
* @bsimethod                                                    EarlinLutz  07/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_subdivideCatmullClark
(
MTGFacets *pFacets
)
    {
    MTGGraph * pGraph = &pFacets->graphHdr;
    EmbeddedDPoint3dArray *pXYZArray = &pFacets->vertexArrayHdr;
    EmbeddedDPoint3dArray *pXYZSave = jmdlEmbeddedDPoint3dArray_grab ();
    MTGMask visitMask        = jmdlMTGGraph_grabMask (pGraph);
    MTGMask maskOriginalVertex = jmdlMTGGraph_grabMask (pGraph);
    MTGMask maskMidEdge = jmdlMTGGraph_grabMask (pGraph);
    MTGMask maskMidFace = jmdlMTGGraph_grabMask (pGraph);
    MTGMask maskEdgeToFace = jmdlMTGGraph_grabMask (pGraph);
    MTGMask allMasks = visitMask | maskOriginalVertex | maskMidEdge | maskMidFace | maskEdgeToFace;
    MTGCoordinateAssistant edgeXYZ (pFacets);

    jmdlMTGGraph_clearMaskInSet (pGraph, allMasks);

    if (sNoisyCut)
        jmdlMTGFacets_printFaceLoops (pFacets);

    // Find, split, and mark each edge...
    jmdlMTGGraph_setMaskInSet (pGraph, maskOriginalVertex);
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    MTGARRAY_SET_LOOP (nodeIdA0, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, nodeIdA0, visitMask)
            && !jmdlMTGGraph_getMask (pGraph, nodeIdA0, MTG_EXTERIOR_MASK))
            {
            // First touch of this edge.
            MTGNodeId nodeIdA1 = jmdlMTGGraph_getEdgeMate (pGraph, nodeIdA0);
            // New nodes for mid edge ...
            MTGNodeId nodeIdB0, nodeIdB1;
            jmdlMTGGraph_splitEdge (pGraph, &nodeIdB0, &nodeIdB1, nodeIdA0);
            jmdlMTGGraph_setMask (pGraph, nodeIdA0, visitMask);
            jmdlMTGGraph_setMask (pGraph, nodeIdA1, visitMask);
            jmdlMTGGraph_setMask (pGraph, nodeIdB0, visitMask | maskMidEdge);
            jmdlMTGGraph_setMask (pGraph, nodeIdB1, visitMask | maskMidEdge);
            edgeXYZ.AverageNeighbors (nodeIdB0);
            edgeXYZ.SaveAsNewVertex (nodeIdB0);
            }
        }
    MTGARRAY_END_SET_LOOP( nodeIdA0, pGraph)

    // Find, split, and mark each face ...

    if (sNoisyCut)
        {
        printf (" AFTER EDGE SPLIT (no edge coordinates)\n");
        jmdlMTGFacets_printFaceLoops (pFacets);
        }

    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    MTGCoordinateAssistant faceXYZ (pFacets);
    MTGARRAY_SET_LOOP (nodeIdA0, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, nodeIdA0, visitMask)
            && !jmdlMTGGraph_getMask (pGraph, nodeIdA0, MTG_EXTERIOR_MASK)
            && jmdlMTGGraph_getMask (pGraph, nodeIdA0, maskOriginalVertex))
            {
            jmdlMTGGraph_setMaskAroundFace (pGraph, nodeIdA0, visitMask);
            // We are at an original vertex.  The face loop will alternately have original and midedge vertices.
            // Along each edge, call the nodes A,B,C
            MTGNodeId nodeIdA = nodeIdA0;
            // Exposed midface node (to be joined to "next" mid edge)
            MTGNodeId nodeIdF = MTG_NULL_NODEID;
            // Nodes on edge from mid-edge to midface...
            MTGNodeId nodeIdB1, nodeIdF1;
            faceXYZ.Zero ();
            do {
                MTGNodeId nodeIdB = jmdlMTGGraph_getFSucc (pGraph, nodeIdA);
                MTGNodeId nodeIdC = jmdlMTGGraph_getFSucc (pGraph, nodeIdB);
                jmdlMTGGraph_join (pGraph, &nodeIdB1, &nodeIdF1, nodeIdB, nodeIdF,
                                maskEdgeToFace | visitMask,
                                maskMidFace    | visitMask);
                faceXYZ.Accumulate (nodeIdA);
                nodeIdF = nodeIdF1;
                nodeIdA = nodeIdC;
                } while (nodeIdA != nodeIdA0);
            faceXYZ.SaveAsNewVertex (nodeIdF);
            }
        }
    MTGARRAY_END_SET_LOOP( nodeIdA, pGraph)


    // Save the "old" vertex and mid-edge coordinates
    jmdlEmbeddedDPoint3dArray_copy (pXYZSave, pXYZArray);

    if (sNoisyCut)
        {
        printf (" QUADS IN ALL FACES\n");
        jmdlMTGFacets_printFaceLoops (pFacets);
        }

    // Edges can now see their vertex and face neighbors ...
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    MTGARRAY_SET_LOOP (nodeIdA0, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, nodeIdA0, visitMask)
            && !jmdlMTGGraph_getMask (pGraph, nodeIdA0, MTG_EXTERIOR_MASK)
            && jmdlMTGGraph_getMask (pGraph, nodeIdA0, maskMidEdge))
            {
            jmdlMTGGraph_setMaskAroundVertex (pGraph, nodeIdA0, visitMask);
            edgeXYZ.AverageNeighbors (nodeIdA0);
            edgeXYZ.SaveAsReplacement (nodeIdA0);
            }
        }
    MTGARRAY_END_SET_LOOP( nodeIdA, pGraph)


    if (sNoisyCut)
        {
        printf (" AFTER MIDEDGE XYZ UPDATE\n");
        jmdlMTGFacets_printFaceLoops (pFacets);
        }

    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    MTGCoordinateAssistant sumF (pFacets);
    MTGCoordinateAssistant sumR (pFacets);
    MTGCoordinateAssistant vertexXYZ (pFacets);
    MTGARRAY_SET_LOOP (nodeIdA0, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, nodeIdA0, visitMask)
            && !jmdlMTGGraph_getMask (pGraph, nodeIdA0, MTG_EXTERIOR_MASK)
            && jmdlMTGGraph_getMask (pGraph, nodeIdA0, maskOriginalVertex))
            {
            jmdlMTGGraph_setMaskAroundVertex (pGraph, nodeIdA0, visitMask);
            int n = 0;
            sumF.Zero ();
            sumR.Zero ();
            MTGARRAY_VERTEX_LOOP (nodeIdA, pGraph, nodeIdA0)
                {
                MTGNodeId nodeIdB = jmdlMTGGraph_getFSucc (pGraph, nodeIdA);
                MTGNodeId nodeIdC = jmdlMTGGraph_getFSucc (pGraph, nodeIdB);
                sumR.Accumulate (nodeIdB, pXYZSave);
                sumF.Accumulate (nodeIdC);
                n++;
                }
            MTGARRAY_END_VERTEX_LOOP (nodeIdA, pGraph, nodeIdA0)
            sumF.NormalizeWeight ();
            sumR.NormalizeWeight ();
            vertexXYZ.Zero ();
            vertexXYZ.Accumulate (sumF.mXYZ, 1.0);
            vertexXYZ.Accumulate (sumR.mXYZ, 2.0);
            vertexXYZ.Accumulate (nodeIdA0, (double)(n-3));
            vertexXYZ.SaveAsReplacement (nodeIdA0);
            }
        }
    MTGARRAY_END_SET_LOOP( nodeIdA, pGraph)
    jmdlMTGGraph_dropMask (pGraph, visitMask);
    jmdlMTGGraph_dropMask (pGraph, maskEdgeToFace);
    jmdlMTGGraph_dropMask (pGraph, maskOriginalVertex);
    jmdlMTGGraph_dropMask (pGraph, maskMidEdge);
    jmdlMTGGraph_dropMask (pGraph, maskMidFace);
    jmdlEmbeddedDPoint3dArray_drop (pXYZSave);

    if (sNoisyCut)
        {
        printf (" FINAL\n");
        jmdlMTGFacets_printFaceLoops (pFacets);
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
