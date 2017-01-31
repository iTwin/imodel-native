/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_developable.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../mtg/mtgintrn.h"

/* @dllName mtg */

#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static int s_debug = 0;

typedef struct
    {
    GraphicsPointArray *pBoundary0;
    GraphicsPointArray *pBoundary1;
    GraphicsPointArray *pSpaceMesh;
    GraphicsPointArray *pFlatMesh;
    Transform putdownTransform;
    double curveChordTol;
    double curveAngleTol;
    double quadrilateralRefinementTol;
    double quadrilateralTriangulationTol;
    double  minParameterStep;
    double shortEdgeTol;
    DPoint3d previousNormal;
    int previousNormalValid;
    } RuledPatternContext;


static void validateContext
(
RuledPatternContext *pContext
)
    {
    DRange3d range;
    double dataSize;
    double minTol;
    static double s_chordTolFactor = 1000.0;
    static double s_quadrilateralRefinementFactor = 1000.0;
    static double s_quadrilateralTriangulationFactor = 1.0;
    static double s_minParameterStep = 0.01;
    static double s_minAngleTol = 0.05;
    double smallAngle = bsiTrig_smallAngle ();

    bsiDRange3d_init (&range);
    jmdlGraphicsPointArray_extendDRange3d (pContext->pBoundary0, &range);
    jmdlGraphicsPointArray_extendDRange3d (pContext->pBoundary1, &range);
    dataSize = bsiDRange3d_getLargestCoordinate (&range);

    minTol = dataSize * bsiTrig_smallAngle ();

    pContext->shortEdgeTol = minTol;

    if (pContext->curveChordTol < s_chordTolFactor * minTol)
        pContext->curveChordTol = s_chordTolFactor * minTol;

    if (pContext->quadrilateralRefinementTol < s_quadrilateralRefinementFactor * smallAngle)
        pContext->quadrilateralRefinementTol = s_quadrilateralRefinementFactor * smallAngle;

    if (pContext->quadrilateralTriangulationTol < s_quadrilateralTriangulationFactor * smallAngle)
        pContext->quadrilateralTriangulationTol = s_quadrilateralTriangulationFactor * smallAngle;

    if (pContext->minParameterStep > s_minParameterStep)
        pContext->minParameterStep = s_minParameterStep;

    if (pContext->curveAngleTol < s_minAngleTol)
        pContext->curveAngleTol = s_minAngleTol;
    }

typedef enum
    {
    TRIEDGE_NONE = 0,
    TRIEDGE_BOUNDARY = 1,   /* A true boundary edge */
    TRIEDGE_FRONT = 2,      /* Front edge of advancing fold.  May be duplicated by subsequent
                                triangle. */
    TRIEDGE_REAR = 3    /* Trailing edge of fold. */
    } TriEdgeFoldCode;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addWithFoldAngle
(
GraphicsPointArray *pGPA,
DPoint3d *pXYZBuffer,
int numPoint,
double angle,
int    angleIndex,
TriEdgeFoldCode      *pLabel                    /* Nominal edge positions -- front, rear, boundary */
)
    {
    int i;
    int userData;
    for (i = 0; i < numPoint; i++)
        {
        userData = pLabel[i];
        jmdlGraphicsPointArray_addComplete (
                    pGPA,
                    pXYZBuffer[i].x, pXYZBuffer[i].y, pXYZBuffer[i].z, 1.0,
                    i == angleIndex ? angle : 0.0,
                    0, userData);
        }
    jmdlGraphicsPointArray_markBreak (pGPA);
    jmdlGraphicsPointArray_markMajorBreak (pGPA);
    }

/*---------------------------------------------------------------------------------**//**
* Emit a triangle.  Transform designated edge to pattern and reset pattern transform to
*       new base.

* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void emitTriangle
(
RuledPatternContext *pContext,
const DPoint3d *pPoint0,
const DPoint3d *pPoint1,
const DPoint3d *pPoint2,
int indexOrigin,
int indexX,
int indexY,
int reset0,
int reset1,
int foldIndex,
TriEdgeFoldCode label0,
TriEdgeFoldCode label1,
TriEdgeFoldCode label2
)
    {
    DPoint3d spacePoint[4];
    DPoint3d flatPoint[4];
    Transform frame, inverseFrame;
    Transform fullTransform;
    DPoint3d currNormal;
    double angle;
    TriEdgeFoldCode label[4];
    bool    hasFoldEdge;

    DPoint3d xVector, yVector, zVector;

    int numPoint = 4;

    spacePoint[0] = *pPoint0;
    spacePoint[1] = *pPoint1;
    spacePoint[2] = *pPoint2;
    spacePoint[3] = *pPoint0;
    label[0] = label0;
    label[1] = label1;
    label[2] = label2;
    label[3] = TRIEDGE_NONE;
    bsiDPoint3d_crossProduct3DPoint3d (&currNormal, pPoint0, pPoint1, pPoint2);
    bsiDPoint3d_normalizeInPlace (&currNormal);

    angle = 0.0;
    hasFoldEdge = false;
    if (pContext->previousNormalValid)
        {
        angle = bsiDPoint3d_angleBetweenVectors (&pContext->previousNormal, &currNormal);
        hasFoldEdge = true;
        }
    pContext->previousNormal = currNormal;
    pContext->previousNormalValid = true;

    addWithFoldAngle (pContext->pSpaceMesh, spacePoint, numPoint, angle, foldIndex, label);

    RotMatrix matrix;
    bsiRotMatrix_initRotationFromOriginXY
                    (
                    &matrix,
                    &spacePoint[indexOrigin],
                    &spacePoint[indexX],
                    &spacePoint[indexY]
                    );
    frame = Transform::From (matrix, spacePoint[indexOrigin]);

    bsiTransform_invertTransform (&inverseFrame, &frame);
    bsiTransform_multiplyTransformTransform (&fullTransform, &pContext->putdownTransform, &inverseFrame);

    bsiTransform_multiplyDPoint3dArray (&fullTransform, flatPoint, spacePoint, numPoint);

    addWithFoldAngle (pContext->pFlatMesh, flatPoint, numPoint, angle, foldIndex, label);

    bsiDPoint3d_computeNormal (&xVector, &flatPoint[reset1], &flatPoint[reset0]);
    bsiDPoint3d_rotateXY (&yVector, &xVector, msGeomConst_piOver2);
    bsiDPoint3d_crossProduct (&zVector, &xVector, &yVector);
    bsiTransform_initFromOriginAndVectors
                    (&pContext->putdownTransform, &flatPoint[reset0], &xVector, &yVector, &zVector);
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute the angle between normals of two triangles
* between specified fractions of curves.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static double bilinearTwistAngle
(
GraphicsPointArrayCP pSource0,
            int         index0,
            double      a0,
            double      a1,
GraphicsPointArrayCP pSource1,
            int         index1,
            double      b0,
            double      b1
)
    {
    DPoint3d pointa0, pointa1, pointb0, pointb1;
    DPoint3d normal0, normal1;
    jmdlGraphicsPointArray_primitiveFractionToDPoint3d (pSource0, &pointa0, index0, a0);
    jmdlGraphicsPointArray_primitiveFractionToDPoint3d (pSource0, &pointa1, index0, a1);

    jmdlGraphicsPointArray_primitiveFractionToDPoint3d (pSource1, &pointb0, index1, b0);
    jmdlGraphicsPointArray_primitiveFractionToDPoint3d (pSource1, &pointb1, index1, b1);

    bsiDPoint3d_crossProduct3DPoint3d (&normal0, &pointa0, &pointb0, &pointa1);
    bsiDPoint3d_crossProduct3DPoint3d (&normal1, &pointb0, &pointb1, &pointb1);
    return bsiDPoint3d_angleBetweenVectors (&normal0, &normal1);
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute the turn angle between tangents on a primitive at two
*   specified fractions.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static double primitiveTurnAngle
(
GraphicsPointArrayCP pSource,
            int         index,
            double      a0,
            double      a1
)
    {
    DPoint3d point0,  point1;
    DPoint3d tangent0, tangent1;

    jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent
                (pSource, &point0, &tangent0, index, a0);
    jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent
                (pSource, &point1, &tangent1, index, a1);

    return bsiDPoint3d_angleBetweenVectors (&tangent0, &tangent1);
    }

static double primitiveSubdivisionFractionalStep
(
GraphicsPointArrayCP pSource,
            int         index,
            double      curveAngleTol
)
    {
    double fraction;
    double df = 0.125;
    double sum = 0.0;
    DPoint3d tangent0, tangent1, point0, point1;
    static double s_minCurveAngleTol = 0.01;
    int numInterval = 1;
    fraction = 0.0;
    jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent
                (pSource, &point0, &tangent0, index, fraction);
    for (fraction = 0;fraction <= 1; fraction += df, point0 = point1, tangent0 = tangent1)
        {
        jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent
                (pSource, &point1, &tangent1, index, fraction);
        sum += fabs (bsiDPoint3d_angleBetweenVectors (&tangent0, &tangent1));
        }

    if (curveAngleTol < s_minCurveAngleTol)
        curveAngleTol = s_minCurveAngleTol;

    if (sum < curveAngleTol)
        numInterval = 1;
    else
        numInterval = (int) (sum / curveAngleTol + 0.99999);

    return 1.0 / (double)numInterval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool cb_processRuledPrimitivePair
(
RuledPatternContext     *pContext,
GraphicsPointArrayCP pSource0,
            int         index0,
GraphicsPointArrayCP pSource1,
            int         index1
)
    {
    DPoint3d point00, point10, point01, point11;
    double s_nearOneFactor = 1.01;
    double df, f0, f1;
    double df0, df1;
    static double dfMin = 1.0 / 64.0;
    double dfStart;

    /* Brute force -- fixed chords per curve */
    jmdlGraphicsPointArray_primitiveFractionToDPoint3d (pContext->pBoundary0, &point00, index0, 0.0);
    jmdlGraphicsPointArray_primitiveFractionToDPoint3d (pContext->pBoundary1, &point10, index1, 0.0);

    df0 = primitiveSubdivisionFractionalStep (pSource0, index0, pContext->curveAngleTol);
    df1 = primitiveSubdivisionFractionalStep (pSource1, index1, pContext->curveAngleTol);
    if (df1 < df0)
        dfStart = df1;
    else
        dfStart = df0;

    f0 = 0.0;
    while (f0 < 1.0)
        {
        df = dfStart;
        f1 = f0 + df;
        if (f1 > 1.0)
            f1 = 1.0;
        df = f1 - f0;
        while (
                df > dfMin
                && primitiveTurnAngle (pSource0, index0, f0, f1)
                                > pContext->curveAngleTol
                && primitiveTurnAngle (pSource1, index1, f0, f1)
                                > pContext->curveAngleTol
                && bilinearTwistAngle (pSource0, index0, f0, f1, pSource1, index1, f0, f1)
                                > pContext->curveAngleTol
                )
                {
                df *= 0.5;
                f1 = f0 + df;
                }
        if (f1 > 1.0)
            f1 = 1.0;
        /* If we lunged almost to 1, go all the way */
        if (f0 + s_nearOneFactor * df > 1.0)
            f1 = 1.0;

        jmdlGraphicsPointArray_primitiveFractionToDPoint3d
                    (pContext->pBoundary0, &point01, index0, f1);
        jmdlGraphicsPointArray_primitiveFractionToDPoint3d
                    (pContext->pBoundary1, &point11, index1, f1);

        if (bsiDPoint3d_distance (&point00, &point01) > pContext->shortEdgeTol)
            {
            emitTriangle (pContext, &point00, &point10, &point01, 0, 1, 2, 2, 1, 0,
                            TRIEDGE_REAR, TRIEDGE_FRONT, TRIEDGE_BOUNDARY
                            );
            point00 = point01;
            }

        if (bsiDPoint3d_distance (&point10, &point11) > pContext->shortEdgeTol)
            {
            emitTriangle (pContext, &point01, &point10, &point11, 0, 1, 2, 0, 2, 0,
                            TRIEDGE_REAR, TRIEDGE_BOUNDARY, TRIEDGE_FRONT
                            );
            point10 = point11;
            }

        f0 = f1;
        }
    return true;
    }

static void markLeadingAndTrailingBoundary
(
GraphicsPointArray  *pGPA
)
    {
    GraphicsPoint gp0, gp1;
    int n;
    if (   pGPA
        && ((n = jmdlGraphicsPointArray_getCount (pGPA)) >= 4)
        && jmdlGraphicsPointArray_getGraphicsPoint (pGPA, &gp0, 0)
        && jmdlGraphicsPointArray_getGraphicsPoint (pGPA, &gp1, n - 2)
        && gp0.userData == TRIEDGE_REAR
        && gp1.userData == TRIEDGE_FRONT
        )
        {
        gp0.userData = TRIEDGE_BOUNDARY;
        gp1.userData = TRIEDGE_BOUNDARY;
        jmdlGraphicsPointArray_setGraphicsPoint (pGPA, &gp0, 0);
        jmdlGraphicsPointArray_setGraphicsPoint (pGPA, &gp1, n - 2);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Examine pairs of geometry items in two arrays.
* Construct a "ruled" surface between corresponding primitives, triangulating as
* needed to generate a planar approximation.
* @param pSpaceMesh <= receives 3d mesh on the ruled surface
* @param pFlatMesh <= receives flattened mesh.
* @param pBoundary0 => first boundary
* @param pBoundary1 => second boundary
* @param curveChordTol => tol for chordal approxmation of curves.
* @param curveAngleTol => tol for angular turn between successive chords on curve approximation.
*                                   (radians)
* @param quadrilateralRefinementTol => accept a quadrilateral element when twist is
*               smaller than this angle.  (radians)
* @param quadrilateralTriangulationTol => when an accepted quad has twist larger than this
*               angle, triangulate for output.  This number should typically be
*               1000X smaller than the quadrilateral refinement tol.
* @param minParameterStep => smallest parameter step for a chord on a primitive.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_ruledPattern
(
        GraphicsPointArray  *pSpaceMesh,
        GraphicsPointArray  *pFlatMesh,
        GraphicsPointArray  *pBoundary0,
        GraphicsPointArray  *pBoundary1,
        double curveChordTol,
        double curveAngleTol,
        double quadrilateralRefinementTol,
        double quadrilateralTriangulationTol,
        double minParameterStep
)
    {
    RuledPatternContext context;

    memset (&context, 0, sizeof context);
    context.curveChordTol = curveChordTol;
    context.curveAngleTol = curveAngleTol;
    context.quadrilateralRefinementTol = quadrilateralRefinementTol;
    context.quadrilateralTriangulationTol = quadrilateralTriangulationTol;
    context.minParameterStep = minParameterStep;
    context.pBoundary0 = pBoundary0;
    context.pBoundary1 = pBoundary1;
    context.pSpaceMesh = pSpaceMesh;
    context.pFlatMesh  = pFlatMesh;

    bsiTransform_initIdentity (&context.putdownTransform);
    validateContext (&context);

    jmdlGraphicsPointArray_processCorrespondingIndexPairs
            (
            &context,
            pBoundary0,
            pBoundary1,
            (GPAPairFunc_IndexIndex)cb_processRuledPrimitivePair
            );
    markLeadingAndTrailingBoundary (pSpaceMesh);
    markLeadingAndTrailingBoundary (pFlatMesh);
    }

//==============================================================================
/*----------------------------------------------------------------------+
|                                                                       |
+----------------------------------------------------------------------*/
static int jmdlMTGGraph_countFaceSteps
(
MTGGraph *pGraph,
MTGNodeId   startNodeId,
MTGNodeId   endNodeId
)
    {
    int count = 0;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, startNodeId)
        {
        if (currNodeId == endNodeId)
            return count;
        count++;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, startNodeId)
    return -1;
    }

/*----------------------------------------------------------------------+
|                                                                       |
+----------------------------------------------------------------------*/
static bool    buildLadder
(
MTGFacets   *pFacets,
MTGNodeId   cornerNodeId0,
MTGNodeId   cornerNodeId1
)
    {
    MTGGraph *pGraph = jmdlMTGFacets_getGraph (pFacets);
    int step0, step1, sum0, sum1;
    int n0 = jmdlMTGGraph_countFaceSteps (pGraph, cornerNodeId0, cornerNodeId1);
    int n1 = jmdlMTGGraph_countFaceSteps (pGraph, cornerNodeId1, cornerNodeId0);
    MTGNodeId newLeftNodeId, newRightNodeId;
    MTGNodeId leftNodeId, rightNodeId;
    MTGNodeId baseNodeId = cornerNodeId0;

    if (n0 < 1 || n1 < 1)
        return false;

    n0--;
    n1--;
    step0 = n1;
    step1 = n0;
    sum0 = step0;
    sum1 = step1;

    while (n0 + n1 > 1)
        {
        if (sum0 < sum1
            || (sum0 == sum1 && step0 <= step1))
            {
            leftNodeId = baseNodeId;
            rightNodeId = jmdlMTGGraph_getFSucc (pGraph,
                          jmdlMTGGraph_getFSucc (pGraph, baseNodeId));
            sum0 += step0;
            n0--;
            }
        else
            {
            leftNodeId = jmdlMTGGraph_getFPred (pGraph, baseNodeId);
            rightNodeId = jmdlMTGGraph_getFSucc (pGraph, baseNodeId);
            sum1 += step1;
            n1--;
            }
            jmdlMTGGraph_join ( pGraph,
                                &newLeftNodeId, &newRightNodeId,
                                leftNodeId, rightNodeId,
                                MTG_NULL_MASK, MTG_NULL_MASK);
            baseNodeId = newLeftNodeId;
        }
    return true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
+----------------------------------------------------------------------*/
bool    constructTrianglesBetweenLineStrings
(
MTGFacets   *pFacets,
bvector<DPoint3d> &xyzA,
bvector<DPoint3d> &xyzB,
bool        flip
)
    {
    MTGGraph *pGraph = jmdlMTGFacets_getGraph (pFacets);
    size_t numA = xyzA.size ();
    size_t numB = xyzB.size ();
    if (numA < 1 || numB < 1 || numA + numB < 3)
        return false;

    MTGNodeId oldNodeId;
    MTGNodeId leftNodeId, rightNodeId;
    MTGNodeId topBarNodeId, bottomBarNodeId;
    DPoint3d xyzA1 = xyzA.back ();
    DPoint3d xyzB1 = xyzB.back ();
    bool    singular0, singular1;
    MTGMask railMask = MTG_CONSTU_MASK;

    struct NodeAndVertexIndex
        {
        MTGNodeId m_nodeId;
        ptrdiff_t m_vertexIndex;

        NodeAndVertexIndex ()
            : m_nodeId (MTG_NULL_NODEID), m_vertexIndex(-1)
            {}

        NodeAndVertexIndex
            (
            MTGNodeId nodeId,
            size_t vertexIndex
            )
            : m_nodeId (nodeId), m_vertexIndex(vertexIndex)
            {

            }
        };

    bvector<NodeAndVertexIndex> indexA (numA), indexB (numB);

    static double s_relTol = 1.0e-4;
    static int debugFlags = 0;

    double tolerance = s_relTol * ( PolylineOps::Length (xyzA) + PolylineOps::Length (xyzB) );

    /* Assign vertex indices */
    for (size_t i = 0; i < numA; i++)
        {
        indexA[i].m_vertexIndex = jmdlMTGFacets_addVertex (pFacets, &xyzA[i], NULL);
        }

    // iA0,iB0 are first candidates for joining.
    // iA1, iB1 are last candiadates for joining.
    // Both are "normally" at ends but moved inward by 1 of ends have matching coordinates.
    ptrdiff_t iA0 = 0;
    ptrdiff_t iA1 = numA - 1;
    ptrdiff_t iB0 = 0;
    ptrdiff_t iB1 = numB - 1;
    singular0 = singular1 = false;
    for (size_t i = 0; i < numB; i++)
        {
        if (i == 0 && xyzA.front ().Distance (xyzB.front ()) <= tolerance)
            {
            indexB[0].m_vertexIndex = indexA[0].m_vertexIndex;
            iA0 = iB0 = 1;
            singular0 = true;
            }
        else if (i + 1 == numB && xyzA1.Distance (xyzB1) <= tolerance)
            {
            indexB[i].m_vertexIndex = indexA.back ().m_vertexIndex;
            iA1 = numA - 2;
            iB1 = numB - 2;
            singular1 = true;
            }
        else
            {
            indexB[i].m_vertexIndex = jmdlMTGFacets_addVertex (pFacets, &xyzB[i], NULL);
            }
        }

    if (iA0 >= iA1 || iB0 >= iB1)
        return false;


    /* Build non-singular part of topology */
    /* string A goes up the right, so its left side is interior. */
    /* string B goes up the left, so its right side is interior. */
    oldNodeId = MTG_NULL_NODEID;
    for (size_t i = 0; i +1 < numA; i++)
        {
        jmdlMTGFacets_addIndexedEdge (pFacets,
                            &leftNodeId, &rightNodeId,
                            oldNodeId, MTG_NULL_NODEID,
                            MTG_PRIMARY_EDGE_MASK | railMask,
                            MTG_EXTERIOR_MASK | MTG_PRIMARY_EDGE_MASK | railMask,
                            (int)indexA[i].m_vertexIndex, (int)indexA[i+1].m_vertexIndex,
                            -1, -1);
        indexA[i].m_nodeId = leftNodeId;
        indexA[i+1].m_nodeId = rightNodeId; /* All but last will be replaced next time around */
        oldNodeId = rightNodeId;
        }



    oldNodeId = MTG_NULL_NODEID;
    for (size_t i = 0; i + 1 < numB; i++)
        {
        jmdlMTGFacets_addIndexedEdge (pFacets,
                            &leftNodeId, &rightNodeId,
                            oldNodeId, MTG_NULL_NODEID,
                            MTG_EXTERIOR_MASK | MTG_PRIMARY_EDGE_MASK | railMask,
                            MTG_PRIMARY_EDGE_MASK | railMask,
                            (int)indexB[i].m_vertexIndex, (int)indexB[i+1].m_vertexIndex,
                            -1, -1);
        indexB[i].m_nodeId = jmdlMTGGraph_getVSucc (pGraph, leftNodeId);
        indexB[i+1].m_nodeId = rightNodeId; /* All but last will be replaced next time around */
        oldNodeId = rightNodeId;
        }

    /* Build caps
        bottomBar goes from left to right at bottom.
       topBar goes from right to left at top
    */
    if (debugFlags)
        jmdlMTGFacets_printFaceLoops (pFacets);
    jmdlMTGFacets_addIndexedEdge (pFacets,
                            &leftNodeId, &rightNodeId,
                            indexA[iA0].m_nodeId, indexB[iB0].m_nodeId,
                            singular0 ? MTG_NULL_MASK : MTG_EXTERIOR_MASK | MTG_PRIMARY_EDGE_MASK,
                            singular0 ? MTG_NULL_MASK : MTG_PRIMARY_EDGE_MASK,
                            (int)indexA[iA0].m_vertexIndex, (int)indexB[iB0].m_vertexIndex,
                            -1, -1);
    bottomBarNodeId = rightNodeId;
    if (singular0)
        pGraph->VertexTwist (indexA[0].m_nodeId, indexB[0].m_nodeId);

    jmdlMTGFacets_addIndexedEdge (pFacets,
                            &leftNodeId, &rightNodeId,
                            indexA[iA1].m_nodeId, indexB[iB1].m_nodeId,
                            singular1 ? MTG_NULL_MASK : MTG_PRIMARY_EDGE_MASK,
                            singular1 ? MTG_NULL_MASK : MTG_EXTERIOR_MASK | MTG_PRIMARY_EDGE_MASK,
                            (int)indexA[iA1].m_vertexIndex, (int)indexB[iB1].m_vertexIndex,
                            -1, -1);
    topBarNodeId = leftNodeId;
    if (singular1)
        jmdlMTGGraph_vertexTwist (pGraph, indexA[numA-1].m_nodeId, indexB[numB-1].m_nodeId);

    if (debugFlags)
        {
        jmdlMTGFacets_printFaceLoops (pFacets);
        printf (" bottom %d    top %d\n", bottomBarNodeId, topBarNodeId);
        }

    buildLadder (pFacets, bottomBarNodeId, topBarNodeId);
    if (flip)
        jmdlMTGFacets_flipRuledXYZ (pFacets, railMask);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
Copy pPointBuffer1 to pPointBuffer2.  If closed, shift so start/end point is closest point
to first point in pPointBuffer0.
Reverse pPointBuffer2 if first chord vector is opposite the first chord vector in pPointBuffer0.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void     correctCyclicAlignment
(
        DPoint3d    *pPointBuffer2,
        int         *pNum2,
        DPoint3d    *pPointBuffer0,
        int         num0,
        DPoint3d    *pPointBuffer1,
        int         num1
)
    {
    DPoint3d xyzBase = pPointBuffer0[0];
    double bigNum = bsiDPoint3d_getLargestCoordinate (pPointBuffer1, num1);
    double tol = 1.0e-10 * bigNum;
    double gap0, gap1;
    int i, i0, k;
    int cycleCount;
    DVec3d chord0, chord2;
    *pNum2 = num1;
    memcpy (pPointBuffer2, pPointBuffer1, num1 * sizeof (DPoint3d));

    gap0 = bsiDPoint3d_distance (&pPointBuffer0[0], &pPointBuffer0[num0-1]);
    gap1 = bsiDPoint3d_distance (&pPointBuffer1[0], &pPointBuffer1[num1-1]);

    if (num1 > 2 && gap1 <= tol)
        {
        cycleCount = num1 - 1;
        i0 = bsiGeom_nearestPointinDPoint3dArray (NULL, pPointBuffer1, num1, &xyzBase);
        for (i = 0; i < cycleCount; i++)
            {
            k = i0 + i;
            if (k >= cycleCount)
                k -= cycleCount;
            pPointBuffer2[i] = pPointBuffer1[k];
            }
        pPointBuffer2[num1-1] = pPointBuffer2[0];
        }

    bsiDVec3d_subtractDPoint3dDPoint3d (&chord0, &pPointBuffer0[1], &pPointBuffer0[0]);
    bsiDVec3d_subtractDPoint3dDPoint3d (&chord2, &pPointBuffer2[1], &pPointBuffer2[0]);
    if (bsiDVec3d_dotProduct (&chord0, &chord2) < 0.0)
        {
        int i, j;
        for (i = 0, j = *pNum2 - 1; j > i; j--, i++)
            {
            DPoint3d xyz = pPointBuffer2[i];
            pPointBuffer2[i] = pPointBuffer2[j];
            pPointBuffer2[j] = xyz;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_ruledPatternDPoint3dArrayBoundaries
(
        GraphicsPointArray  *pSpaceMesh,
        GraphicsPointArray  *pFlatMesh,
        DPoint3d    *pPointBuffer0,
        int         num0,
        DPoint3d    *pPointBuffer1,
        int         num1
)
    {
    DPoint3d *pPointBuffer2 = (DPoint3d*)_alloca (num1 * sizeof (DPoint3d));
    int num2;
    static bool s_flip = true;
    MTGFacets *pFacets = jmdlMTGFacets_grab ();
    jmdlMTGFacets_setNormalMode (pFacets, MTG_Facets_VertexOnly, 0, 0);
    correctCyclicAlignment (pPointBuffer2, &num2, pPointBuffer0, num0, pPointBuffer1, num1);
    bvector<DPoint3d> point0, point1;
    DPoint3dOps::Append (&point0, pPointBuffer0, (size_t)num0);
    DPoint3dOps::Append (&point1, pPointBuffer1, (size_t)num1);
    if (constructTrianglesBetweenLineStrings (pFacets, point0, point1, s_flip))
        {
        jmdlMTGFacets_collectInteriorFacesToGPA (pFacets, pSpaceMesh);
        }
    jmdlMTGFacets_drop (pFacets);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlMTGFacets_ruledPatternDPoint3dArrayBoundaries
(
    MTGFacets *pFacets,
    DPoint3d    *pPointBuffer0,
    int         num0,
    DPoint3d    *pPointBuffer1,
    int         num1
)
    {
    DPoint3d *pPointBuffer2 = (DPoint3d*)_alloca (num1 * sizeof (DPoint3d));
    int num2;
    correctCyclicAlignment (pPointBuffer2, &num2, pPointBuffer0, num0, pPointBuffer1, num1);
    bvector<DPoint3d> point0, point1;
    DPoint3dOps::Append (&point0, pPointBuffer0, (size_t)num0);
    DPoint3dOps::Append (&point1, pPointBuffer1, (size_t)num1);
    return constructTrianglesBetweenLineStrings (pFacets, point0, point1, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_ruledPatternFromGPABoundaries
(
        GraphicsPointArray  *pSpaceMesh,
        GraphicsPointArray  *pFlatMesh,
        GraphicsPointArray  *pBoundary0,
        GraphicsPointArray  *pBoundary1
)
    {
    int num0 = jmdlGraphicsPointArray_getCount (pBoundary0);
    int num1 = jmdlGraphicsPointArray_getCount (pBoundary1);
    int numGot0, numGot1;

    DPoint3d *pBuffer0 = (DPoint3d*)_alloca(num0 * sizeof (DPoint3d));
    DPoint3d *pBuffer1 = (DPoint3d*)_alloca(num1 * sizeof (DPoint3d));

    if (   jmdlGraphicsPointArray_getDPoint3dArray (pBoundary0, pBuffer0, &numGot0, 0, num0)
        && jmdlGraphicsPointArray_getDPoint3dArray (pBoundary1, pBuffer1, &numGot1, 0, num1)
        )
        {
        jmdlGraphicsPointArray_ruledPatternDPoint3dArrayBoundaries
                    (
                    pSpaceMesh,
                    pFlatMesh,
                    pBuffer0, numGot0,
                    pBuffer1, numGot1
                    );
        }
    }

/*
Search a target curve for points which may be joined to source curve at a particular source
    point and tangent.
@param pTargetPoints OUT target points for developable lines starting from a curve passing
    through source point with given source tangent.
@param pTargetParams OUT parameters at target points
@param pNumTargetPoints OUT the number of target points
@param maxTargetPont IN maximum number of target ponts permitted.  If more are computed,
    NONE are returned!!!
@param pSourcePoint IN point on source curve.
@param pSourceTangetn IN curve tangent at source point
@param pTargetPoles IN poles of (rational) bezier target curve
@param targetOrder IN order of target curve

@remark Why is the source point weighted?  It makes sense at infinity.
*/
static void computeDevelopableTargets_pointToCurve
(
DPoint3d *pTargetPoints,
double   *pTargetParams,
int      *pNumTargetPoints,
int      maxTargetPoint,
const DPoint4d *pSourcePoint,
const DPoint3d *pSourceTangent,
BCurveSegmentR target
)
    {
    int targetOrder = (int)target.GetOrder ();
    if (targetOrder < 2)
        targetOrder = 2;
    DPoint4dCP pTargetPoles = target.GetPoleP ();
    int maxOrder = 8 * targetOrder; // Well, we really think 3 * targetOrder would do it.
    /* Poles of a bezier function for the (possibly pseudo-length) vectors along
            candidate rule lines from source to parameter along target curve */
    DPoint3d *pRulePoles       = (DPoint3d*)_alloca (maxOrder * sizeof (DPoint3d));
    /* Poles of a bezier function for the (possibly pseudo-length) target curve tangent */
    DPoint3d *pTangentPoles    = (DPoint3d*)_alloca (maxOrder * sizeof (DPoint3d));
    /* Poles of a bezier function which is cross product of the pRulePoles vector function
        with the (fixed) source point tangent */
    DPoint3d *pCrossPoles      = (DPoint3d*)_alloca (maxOrder * sizeof (DPoint3d));
    /* Poles of the (scalar) polynomial whose roots are valid rule lines on the developable surface. */
    double   *pFPoles          = (double*)_alloca (maxOrder * sizeof (double));
    /* Roots of the scalar polynomial */
    double   *pRoots           = (double*)_alloca (maxOrder * sizeof (double));
    int tangentOrder, ruleOrder, fOrder;
    int numRoot;
    int spaceDim = 3;
    int i;
    *pNumTargetPoints = 0;


    if (   bsiBezierDPoint4d_pseudoTangent (pTangentPoles, &tangentOrder, maxOrder,
                    pTargetPoles, targetOrder, spaceDim)
        && bsiBezierDPoint4d_pseudoVectorFromDPoint4d (pRulePoles, &ruleOrder, maxOrder,
                    pTargetPoles, targetOrder, pSourcePoint, spaceDim)
        )
        {
        /* Let X0, T0 be the source point and curve tangent.
           Let T1(s) be the target curve's tangent vector at parameter s.
           Let R(s) be the vector along the rule line from source point to target curve at
                        parameter s.
           Geometrically, we need the 3 vectors to be coplanar.
           Algebraically, three vectors are coplanar if their (scalar!!) triple product is zero:
                f(s) = T1(s) dot R(s) cross T0 = 0
           This is a scalar polynomial in s.
           If the target curve has non-uniform weights, we don't care about the magnitude
            of either the tangent or rule vector, and can use the pseudo tangent and
            pseudo difference vectors.
           (Remark:  The order of the scalar polynomial is handled by the various bezier
                functions.  We can loosely describe it as follows:
               If the target curve is degree D and has uniform weights, f(s) is degree 2*d.
               If the target curve is degree D with non-uniform weights, the pseudo tangent
                    has order (2D-1) and the pseudo rule vector has order D, so the
                    degree of f is (3D-1)
            )
        */

        /* Since T0 is a fixed vector, R cross T0 is just cross products with all the poles. */
        for (i = 0; i < ruleOrder; i++)
            bsiDPoint3d_crossProduct (&pCrossPoles[i], &pRulePoles[i], pSourceTangent);

        /* The dot product is a 3D bezier vector function dotted with another 3D bezier vector */
        bsiBezier_dotProduct
                (
                pFPoles, &fOrder, maxOrder,
                (double*)pCrossPoles, ruleOrder, 0, 3,
                (double*)pTangentPoles, tangentOrder, 0, 3, 3
                );

        /* Pull out the roots .. */
        bsiBezier_univariateRoots
                (pRoots, &numRoot, pFPoles, fOrder);
        /* And evaluate target points */
        if (numRoot > 0 && numRoot < fOrder && numRoot)
            {
            if (pNumTargetPoints)
                *pNumTargetPoints = numRoot;
            if (pTargetParams)
                memcpy (pTargetParams, pRoots, numRoot * sizeof (double));
            if (pTargetPoints)
                bsiBezierDPoint4d_evaluateDPoint3dArray (pTargetPoints, NULL,
                            pTargetPoles, targetOrder, pRoots, numRoot);
            }
        }
    }
/*
@param pTargetParameter OUT the parameter of the closest developable point on the target curve.
@param pTargetPoint OUT     the coordinates of the closest developable point on the target curve.
@param pSourceXYZ IN start coordinates
@param sourceTangent IN start tangent
@param pTargetPoles IN poles of target curve
@param targetOrder order of target curve
*/
static bool    computeDevelopable_searchSingleBezier
(
DPoint3d *pTargetPoint,
double *pTargetParameter,
const DPoint3d *pSourceXYZ,
const DPoint3d *pSourceTangent,
BCurveSegmentR target
)
    {
    int j, k;
    DPoint4d sourceXYZW;
#define MAX_RESULT 20
    DPoint3d targetPoints[MAX_RESULT];
    double targetParams[MAX_RESULT];
    int numTargetPoint;
    bool    bSolutionFound = false;
    double d0, d1;

    bsiDPoint4d_initFromDPoint3dAndWeight (&sourceXYZW, pSourceXYZ, 1.0);
    computeDevelopableTargets_pointToCurve
                    (
                    targetPoints, targetParams, &numTargetPoint, MAX_RESULT,
                    &sourceXYZW, pSourceTangent, target
                    );

    if (numTargetPoint > 0)
        {

        if (s_debug > 0)
            {
            printf (" (Raw root params %d) ", numTargetPoint);
            for (j = 0; j < numTargetPoint; j++)
                {
                printf (" (f=%lf d=%lf)", targetParams[j],
                        bsiDPoint3d_distance (pSourceXYZ, &targetPoints[j]));
                }
            printf ("\n");
            }
        d0 = bsiDPoint3d_distance (pSourceXYZ, &targetPoints[0]);
        k = 0;
        for (j = 1; j < numTargetPoint; j++)
            {
            d1 = bsiDPoint3d_distance (pSourceXYZ, &targetPoints[j]);
            if (d1 < d0)
                {
                k = j;
                d1 = d0;
                }
            }
        bSolutionFound = true;
        *pTargetPoint = targetPoints[k];
        *pTargetParameter = targetParams[k];
        }
    return bSolutionFound;
    }


/*
At a single fractional position on a source curve, search far GPA for best developable point.
@param pXYZA INPUT known point
@param pTangentA INPUT and its tangent
*/
static bool    computeDevelopable_searchFarCurve
(
DPoint3dCR pointA,
DPoint3dCR tangentA,
MSBsplineCurveCR curveB,
double &knotB,
DPoint3dR pointB,
DVec3dR tangentB
)
    {
    DPoint3d xyzB0;
    DPoint3d xyzBSave;
    DVec3d tangentBSave;
    double fractionB0, knotBSave = 0.0;
    bool    bSolutionFound = false;
    double dist0, distSave;

    xyzBSave.Zero ();
    tangentBSave.Zero ();
    distSave = DBL_MAX;

    // For each target bezier ..
    BCurveSegment segmentB;
    for (size_t i = 0; curveB.AdvanceToBezier (segmentB, i, true);)
        {
        if (s_debug > 0)
            printf (" GPAB bezier index %d\n", (int)i);

        if (computeDevelopable_searchSingleBezier
                (
                &xyzB0, &fractionB0,
                &pointA, &tangentA,
                segmentB
                ))
            {
            dist0 = pointA.Distance (xyzB0);
             if (!bSolutionFound  || dist0 < distSave)
                {
                DVec3d tangentB2;
                DPoint3d xyzB2;
                segmentB.FractionToPoint (xyzB2, tangentB2, fractionB0);
                if (s_debug > 0)
                    printf (" update to <B,%d,%lf> distance %lf\n",
                            (int)i, fractionB0, dist0);
                bSolutionFound = true;
                knotBSave = segmentB.FractionToKnot (fractionB0);
                distSave = dist0;
                xyzBSave = xyzB0;
                tangentBSave = tangentB2;
                }
            }
        }

    if (bSolutionFound)
        {
        knotB = knotBSave;
        pointB = xyzBSave;
        tangentB = tangentBSave;
        }
    return bSolutionFound;
    }


/*---------------------------------------------------------------------------------**//**
@description Sample pCurvesA at specified density per bezier.
For each sampled point on curve A, search curve B for points such that the two tangents and
    the line between the points are coplanar.
Return all computed line segments in pRuleLines, with point from A always first.
Each graphics point is labeled with (read index, bezier parameter).
Any non-bezier in either curve is skipped.  The calculation continues using whatever bezier curves
are present, but the function return value is false.
@return false if any non-beziers are encountered.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_developableRuleLines
(
MSBsplineCurve &curveA,
MSBsplineCurve &curveB,
GraphicsPointArray *pRuleLines,
int numSamplePerPrimitive0
)
    {
    double fractionStep;
    DPoint3d xyzA, xyzB;
    DVec3d tangentA, tangentB;
    int candidateCounter = 0;
    static double s_fractionTol = 1.0e-6;

    int primaryCounter = 0;
    int matchABCounter   = 0;
    int matchBACounter   = 0;
    int matchCounter     = 0;
    static int s_firstActive = -1;
    static int s_lastActive  = -1;

    fractionStep = 1.0 / numSamplePerPrimitive0;
    int fractionIndex0 = 0;
    // For each source bezier ...
    BCurveSegment segment;
    for (size_t i = 0; curveA.AdvanceToBezier (segment, i, true); fractionIndex0 = 1)
        {

        // For each fraction along this bezier ...
        for (int fractionIndex = fractionIndex0; fractionIndex <= numSamplePerPrimitive0; fractionIndex++)
            {
            candidateCounter++;
            if (s_firstActive >= 0 && s_lastActive >= s_firstActive
                && (   candidateCounter < s_firstActive
                    || candidateCounter > s_lastActive)
                )
                continue;

            double fractionA = fractionIndex * fractionStep;
            segment.FractionToPoint (xyzA, tangentA, fractionA);
            double knotA = segment.FractionToKnot (fractionA);
            double knotB;
            bsiDVec3d_normalizeInPlace (&tangentA);
            if (s_debug)
                printf (" ***** Primary search <A:%d:%lf> to B\n", (int)i, fractionA);
            primaryCounter++;
            if (computeDevelopable_searchFarCurve (xyzA, tangentA, curveB, knotB, xyzB, tangentB))
                {
                // Now compute from B back to A.  Don't accept of it's not where we started.
                DPoint3d xyzA2;
                DVec3d tangentA2;
                double knotA2;
                matchABCounter++;
                if (s_debug)
                    printf (" *****  secondary search from <B:%d:%lf> to A\n", (int)i, knotB);
                if (computeDevelopable_searchFarCurve (xyzB, tangentB, curveA, knotA2, xyzA2, tangentA2))
                    {
                    matchBACounter++;
                    if (fabs (knotA2 - knotA) < s_fractionTol)
                        {
                        matchCounter++;
                        jmdlGraphicsPointArray_addComplete (pRuleLines,
                                        xyzA.x, xyzA.y, xyzA.z, 1.0, curveA.KnotToFraction (knotA), 0, 0);
                        jmdlGraphicsPointArray_addComplete (pRuleLines,
                                        xyzB.x, xyzB.y, xyzB.z, 1.0, curveB.KnotToFraction (knotB), 0, 0);
                        jmdlGraphicsPointArray_markBreak (pRuleLines);
                        }
                    }
                }
            }
        }
    if (s_debug)
        printf (" (primary %d) (AB %d) (BA %d) (ABA %d)\n",
                    primaryCounter, matchABCounter, matchBACounter, matchCounter);
    return true;
    }

/*

@description Compute local orientation, tangent, and second derivative of developable surface.
   The curve and segment input arrays are as computed by jmdlGraphicsPointArray_developableRuleLines.

@param pAxes OUT Coordinate directions (unit)
        Column X is along rule line.
        Column Y is tangent to surface in direction of unroll
        Column Z is surface normal

@param pXYZA OUT global coordiantes on curve A
@param pLocalTangentA OUT local coordinate tangent vector on curve A.
@param pLocalKA OUT local coordinate second derivative vector on curve B

@param pXYZA OUT global coordiantes on curve A
@param pLocalTangentA OUT local coordinate tangent vector on curve A.
@param pLocalKA OUT local coordinate second derivative vector on curve B

@param pSegments IN paired segment endpoints with primtive and fraction reference to curves.
@param readIndex IN read index in segment array.
@param pCurvesA IN curves on A side
@param pCurvesB IN curves on B side

*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_localDevelopableFrame
(
RotMatrix *pAxes,
DPoint3d *pXYZA,
DVec3d *pLocalTangentA,
DVec3d *pLocalKA,
DPoint3d *pXYZB,
DVec3d *pLocalTangentB,
DVec3d *pLocalKB,
GraphicsPointArray *pSegments,
int readIndex,
MSBsplineCurve &curveA,
MSBsplineCurve &curveB
)
    {
    GraphicsPoint gpA, gpB;
    bool    bResult = false;
    DPoint3d xyzA, xyzB;
    DVec3d   txyzA, txyzB;
    DVec3d   kxyzA, kxyzB;
    RotMatrix localToGlobal;

    if (   jmdlGraphicsPointArray_getGraphicsPoint (pSegments, &gpA, readIndex)
        && jmdlGraphicsPointArray_getGraphicsPoint (pSegments, &gpB, readIndex + 1)
        )
        {
        curveA.FractionToPoint (xyzA, txyzA, kxyzA, gpA.a);
        curveB.FractionToPoint (xyzB, txyzB, kxyzB, gpB.a);
        
        DVec3d uVec, vVec, wVec;
        double aa;
        // U vec is along curve ...
        bsiDVec3d_subtractDPoint3dDPoint3d (&uVec, &xyzB, &xyzA);
        bsiDVec3d_normalizeInPlace (&uVec);
        // If the developable is properly constructed, the two tangents and U vec should be coplanar.
        //  (and the two tangents should not be parallel to U)
        // We'll use the sum of the two tangents as representative.
        bsiDVec3d_add (&vVec, &txyzA, &txyzB);
        bsiDVec3d_normalizedCrossProduct (&wVec, &uVec, &vVec);
        aa = bsiDVec3d_normalizedCrossProduct (&vVec, &wVec, &uVec);
        bsiRotMatrix_initFromColumnVectors (&localToGlobal, &uVec, &vVec, &wVec);

        if (pAxes)
            *pAxes = localToGlobal;

        if (pXYZA)
            *pXYZA = xyzA;
        if (pXYZB)
            *pXYZB = xyzB;

        if (pLocalTangentA)
            bsiDVec3d_multiplyRotMatrixTransposeDVec3d (pLocalTangentA, &localToGlobal, &txyzA);
        if (pLocalTangentB)
            bsiDVec3d_multiplyRotMatrixTransposeDVec3d (pLocalTangentB, &localToGlobal, &txyzB);

        if (pLocalKA)
            bsiDVec3d_multiplyRotMatrixTransposeDVec3d (pLocalKA, &localToGlobal, &kxyzA);
        if (pLocalKB)
            bsiDVec3d_multiplyRotMatrixTransposeDVec3d (pLocalKB, &localToGlobal, &kxyzB);

        bResult = (aa != 0.0);
        }
    return bResult;
    }

/*

@description Compute the two axis centers for a cone touching a developable line.

@param pCenterA OUT cone center at curve A
double pRadiusA OUT cone radius at curve A
@param pCenterB OUT cone center at curve B
@param pRadiusB OUT cone radius at curve B
@param pTheta   OUT angle between cone axis and rule line

@param pSegments IN paired segment endpoints with primtive and fraction reference to curves.
@param readIndex IN read index in segment array.
@param pCurvesA IN curves on A side
@param pCurvesB IN curves on B side

*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_developableCone
(
DPoint3d *pCenterA,
double   *pRadiusA,
DPoint3d *pCenterB,
double   *pRadiusB,
double   *pTheta,
GraphicsPointArray *pSegments,
int readIndex,
MSBsplineCurve &curveA,
MSBsplineCurve &curveB
)
    {
    DPoint3d xyzA, xyzB;
    // Local tangents
    DVec3d   localTA, localTB;
    // Local 2nd derivatives
    DVec3d   localKA, localKB;
    RotMatrix localToGlobal;
    double rA = 0.0;
    double rB = 0.0;
    double theta = 0.0;
    DPoint3d centerA, centerB;

    bool    bResult = false;

    bsiDPoint3d_zero (&centerA);
    bsiDPoint3d_zero (&centerB);

    if (jmdlGraphicsPointArray_localDevelopableFrame
                    (
                    &localToGlobal,
                    &xyzA, &localTA, &localKA,
                    &xyzB, &localTB, &localKB,
                    pSegments, readIndex,
                    curveA, curveB
                    ))
        {
        // Consider a curve passing through the origin of a uv 2d coordinate system, with (parametric) tangent
        // vector (tu,0), i.e. tangent along u axis.
        // The 2nd derivative vector is (ku,kv).
        // The radius of curvature is tu^2/kv.
        //
        // In the local developable coordinates, u is the y axis and v is the z axis rotated by theta around -y.
        // The projection on localK onto v is
        // kv = -localK.x * sin(theta) + localK.z * cos(theta)
        // Apply this to local radius of curvature in plane perpendicular to the rule line.
        // The local radius of the axis at A is tuA^2 / kvA
        // The local radius of the axis at B is tuB^2 / kvB

        // The ratio of these radii are like the ratio of cone cross sections.
        double dAB = bsiDPoint3d_distance (&xyzA, &xyzB);
        double rhoA, rhoB, cc, ss;
        double tuA2 = localTA.y * localTA.y;
        double tuB2 = localTB.y * localTB.y;
        DVec3d vectorX, vectorZ;

        bool    bOKA = bsiTrig_safeDivide (&rhoA, tuA2, localKA.z, 0.0);

        bool    bOKB = bsiTrig_safeDivide (&rhoB, tuB2, localKB.z, 0.0);

        theta = bsiTrig_atan2 (rhoA - rhoB, dAB);

        cc = cos (theta);
        ss = sin (theta);
        rA = rhoA * cc;
        rB = rhoB * cc;
        bsiDVec3d_fromRotMatrixColumn (&vectorX, &localToGlobal, 0);
        bsiDVec3d_fromRotMatrixColumn (&vectorZ, &localToGlobal, 2);

        bsiDPoint3d_add2ScaledDVec3d (&centerA, &xyzA, &vectorX, ss * rA, &vectorZ, cc * rA);
        bsiDPoint3d_add2ScaledDVec3d (&centerB, &xyzB, &vectorX, ss * rB, &vectorZ, cc * rB);

            {
            double angle0, distA, distB;
            double dotA, dotB, dotZ;
            DVec3d radVecA, radVecB;
            angle0 = bsiDPoint3d_dotProduct (&vectorX, &vectorZ);
            dotZ = bsiDPoint3d_dotDifference (&xyzB, &xyzA, &vectorZ);
            distA = bsiDPoint3d_distance (&centerA, &xyzA);
            distB = bsiDPoint3d_distance (&centerB, &xyzB);
            bsiDVec3d_subtractDPoint3dDPoint3d (&radVecA, &xyzA, &centerA);
            bsiDVec3d_subtractDPoint3dDPoint3d (&radVecB, &xyzB, &centerB);
            dotA = bsiDPoint3d_dotDifference (&centerB, &centerA, &radVecA);
            dotB = bsiDPoint3d_dotDifference (&centerB, &centerA, &radVecB);

            dotB = bsiDPoint3d_dotDifference (&centerB, &centerA, &radVecB);

            }

        bResult = bOKA && bOKB;
        }

    if (pCenterA)
        *pCenterA = centerA;
    if (pCenterB)
        *pCenterB = centerB;
    if (pRadiusA)
        *pRadiusA = rA;
    if (pRadiusB)
        *pRadiusB = rB;
    if (pTheta)
        *pTheta = theta;

    return bResult;
    }

/**
@description Create a flat pattern which approximately represents a strip of quad faces.
In both the space and pattern faces, the strip is represented by pairs of points.
Each point pair is a single rule line.
Nonplanar effects are projected out to local planes.
@param pXY0Array OUT array of rule lines at constant z.
@param pXYZArray IN array of space rule lines
@param pXYZOrigin IN putdown point for first pattern rule edge.
*/
Public GEOMDLLIMPEXP bool    jmdlEmbeddedDPoint3dArray_unfoldQuadStrip
(
EmbeddedDPoint3dArray *pXY0Array,
EmbeddedDPoint3dArray *pXYZArray,
DPoint3d const *pXYZOrigin
)
    {
    DPoint3d xyzA[2], xyzB[2];
    DPoint3d uvwA[2], uvwB[2];
    int numGot;
    int iSegment;
    int numSegment = jmdlEmbeddedDPoint3dArray_getCount (pXYZArray) / 2;
    if (numSegment < 2)
        return false;

    // Place first segment parallel to x axis and from first point of space mesh.
    jmdlEmbeddedDPoint3dArray_getDPoint3dArray (pXYZArray, xyzA, &numGot, 0, 2);
    uvwA[0] = uvwA[1] = *pXYZOrigin;
    uvwA[1].x += bsiDPoint3d_distance (&xyzA[0], &xyzA[1]);

    jmdlEmbeddedDPoint3dArray_addDPoint3dArray (pXY0Array, uvwA, 2);

    for (iSegment = 1; iSegment < numSegment; iSegment++)
        {
        DVec3d xAxis, yAxis;
        DVec3d uAxis, vAxis;
        DVec3d vector0, vector1;
        DVec3d midVector;
        double dotUV, dotUU;
        double s;
        double u0, u1, v0, v1;
        jmdlEmbeddedDPoint3dArray_getDPoint3dArray (pXYZArray, xyzB, &numGot, iSegment * 2, 2);
        // Compute x,y vectors so x is along line A, y perpendicular and towards midpoint of line B
        //
        bsiDVec3d_subtractDPoint3dDPoint3d (&xAxis, &xyzA[1], &xyzA[0]);
        bsiDVec3d_subtractDPoint3dDPoint3d(&vector0, &xyzB[0], &xyzA[0]);
        bsiDVec3d_subtractDPoint3dDPoint3d (&vector1, &xyzB[1], &xyzA[0]);
        bsiDVec3d_interpolate (&midVector, &vector0, 0.5, &vector1);

        dotUU = bsiDVec3d_dotProduct (&xAxis, &xAxis);
        dotUV = bsiDVec3d_dotProduct (&xAxis, &midVector);

        if (bsiTrig_safeDivide (&s, dotUV, dotUU, 0.5))
            {
            bsiDVec3d_addScaled (&yAxis, &midVector, &xAxis, -s);
            bsiDVec3d_normalizeInPlace (&xAxis);
            bsiDVec3d_normalizeInPlace (&yAxis);

            // Compute u,v vectors parallel to global xy plane. u is along unfolded B line, v is perpendicular.
            bsiDVec3d_subtractDPoint3dDPoint3d (&uAxis, &uvwA[1], &uvwA[0]);
            bsiDVec3d_normalizeInPlace (&uAxis);
            bsiDVec3d_unitPerpendicularXY (&vAxis, &uAxis);

            u0 = bsiDVec3d_dotProduct (&vector0, &xAxis);
            u1 = bsiDVec3d_dotProduct (&vector1, &xAxis);

            v0 = bsiDVec3d_dotProduct (&vector0, &yAxis);
            v1 = bsiDVec3d_dotProduct (&vector1, &yAxis);

            bsiDPoint3d_add2ScaledDVec3d (&uvwB[0], &uvwA[0], &uAxis, u0, &vAxis, v0);
            bsiDPoint3d_add2ScaledDVec3d (&uvwB[1], &uvwA[0], &uAxis, u1, &vAxis, v1);
            jmdlEmbeddedDPoint3dArray_addDPoint3dArray (pXY0Array, uvwB, 2);

            uvwA[0] = uvwB[0];
            uvwA[1] = uvwB[1];

            xyzA[0] = xyzB[0];
            xyzA[1] = xyzB[1];
            }
        else
            return false;
        }
    return true;
    }

/**
@description Compute centers of a rolling cone, placed relative to the cone's tangency line
    (on-surface rule line) in pattern position.
@param pCenterC OUT center point at cone base
@param pCenterD OUT center point at cone top.
@param pXYZ0Array IN array with rule lines.
@param iSegmentStart IN index for reading rule line
@param rC IN radius at cone base
@param rD IN radius at cone top
@param theta IN cone angle
*/
static bool    placeLayoutCone
(
DPoint3d *pCenterC,
DPoint3d *pCenterD,
EmbeddedDPoint3dArray *pXY0Array,
int iSegmentStart,
double rC,
double rD,
double theta
)
    {
    DPoint3d xyzLine[2];
    DPoint3d xyzC, xyzD;
    DVec3d vectorU, vectorW;
    int numGot;
    double cc = cos (theta);
    double ss = sin (theta);
    bool    bStat = false;

    if (jmdlEmbeddedDPoint3dArray_getDPoint3dArray (pXY0Array, xyzLine, &numGot, iSegmentStart, 2))
        {
        bsiDVec3d_subtractDPoint3dDPoint3d (&vectorU, &xyzLine[1], &xyzLine[0]);
        bsiDVec3d_normalizeInPlace (&vectorU);
        bsiDVec3d_setXYZ (&vectorW, 0.0, 0.0, 1.0);
        bsiDPoint3d_add2ScaledDVec3d (&xyzC, &xyzLine[0], &vectorU, ss * rC, &vectorW, cc * rC);
        bsiDPoint3d_add2ScaledDVec3d (&xyzD, &xyzLine[1], &vectorU, ss * rD, &vectorW, cc * rD);
        if (pCenterC)
            *pCenterC = xyzC;
        if (pCenterD)
            *pCenterD = xyzD;
        bStat = true;
        }
    return bStat;
    }

/*
@descriptions Compute rule lines of a developable surface.
Rules lines are returned as start-end pairs in the various arrays.
@param pSpaceGPA OUT (REQUIRED) start and end point pairs for each rule line.  In each graphics point,
    the "userData" and "a" fields are the primitive id and fractional parameter.
@param pSpaceRuleLines OUT (REQUIRED) xyz start and end points of rule lines in space.
@param pSpaceConeCenters OUT (OPTIONAL) array of axis points for spatial tangent cones
@param pConeRadiusArray OUT (OPTIONAL) array of cone radii.
@param pPatternRuleLines OUT (OPTIONAL) array of rule lines of the pattern, parallel to xy plane.
@param PatternConeCenters OUT (OPTIONAL) array of axis points for cones tangent to the pattern.
@param pMeshIndexArray OUT (OPTIONAL) mesh indices.  The same indices are applicable to the space and pattern rule lines.
@param curveA IN first reference curve
@param curveB IN second reference curve
@param pXYZPatternOrigin IN origin point for the pattern.  The first rule line of the pattern is placed along the x axis.
@param numPerprimitive IN number of sample points to be placed on each primitive of curve A.
*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_developableRulingsFromCurves
(
GraphicsPointArray *pSpaceGPA,
EmbeddedDPoint3dArray *pSpaceRuleLines,
EmbeddedDPoint3dArray *pSpaceConeCenters,
EmbeddedDoubleArray   *pConeRadiusArray,
EmbeddedDPoint3dArray *pPatternRuleLines,
EmbeddedDPoint3dArray *pPatternConeCenters,
EmbeddedIntArray      *pMeshIndexArray,
MSBsplineCurve &curveA,
MSBsplineCurve &curveB,
DPoint3d const *pXYZPatternOrigin,
int numPerPrimitive
)
    {
    DPoint3d xyzSegment[2];
    int iSegmentStart = 0;
    int numVertex = 0;
    int numSegmentPoint;
    int numThisSegment;

    jmdlGraphicsPointArray_developableRuleLines
            (curveA, curveB, pSpaceGPA, numPerPrimitive);

    for (iSegmentStart = 0;
            jmdlGraphicsPointArray_getDPoint3dArray (pSpaceGPA, xyzSegment, &numThisSegment, iSegmentStart, 2)
        &&  numThisSegment == 2;
        iSegmentStart += 2
        )
        {
        jmdlEmbeddedDPoint3dArray_addDPoint3dArray (pSpaceRuleLines, xyzSegment, 2);
        numVertex += 2;
        if (iSegmentStart > 0)
            {
            // One based indices to points on this and previous segment.
            jmdlEmbeddedIntArray_addInt (pMeshIndexArray, numVertex);
            jmdlEmbeddedIntArray_addInt (pMeshIndexArray, numVertex-2);
            jmdlEmbeddedIntArray_addInt (pMeshIndexArray, numVertex-3);
            jmdlEmbeddedIntArray_addInt (pMeshIndexArray, numVertex-1);
            jmdlEmbeddedIntArray_addInt (pMeshIndexArray, 0);
            }
        }

    if (pPatternRuleLines)
        jmdlEmbeddedDPoint3dArray_unfoldQuadStrip (pPatternRuleLines, pSpaceRuleLines, pXYZPatternOrigin);

    numSegmentPoint = jmdlGraphicsPointArray_getCount (pSpaceGPA);
    for  (
         iSegmentStart = 0;
         iSegmentStart < numSegmentPoint;
         iSegmentStart += 2
         )
        {
        DPoint3d centerA, centerB;
        DPoint3d centerC, centerD;
        double rA, rB;
        double theta;
        if (jmdlGraphicsPointArray_developableCone (&centerA, &rA, &centerB, &rB, &theta,
                        pSpaceGPA, iSegmentStart, curveA, curveB))
            {
            if (pSpaceConeCenters)
                {
                jmdlEmbeddedDPoint3dArray_addDPoint3d (pSpaceConeCenters, &centerA);
                jmdlEmbeddedDPoint3dArray_addDPoint3d (pSpaceConeCenters, &centerB);
                }
            if (pConeRadiusArray)
                {
                jmdlEmbeddedDoubleArray_addDouble (pConeRadiusArray, rA);
                jmdlEmbeddedDoubleArray_addDouble (pConeRadiusArray, rB);
                }

            if (   pPatternRuleLines
                && pPatternConeCenters
                && placeLayoutCone (&centerC, &centerD, pPatternRuleLines, iSegmentStart, rA, rB, theta))
                {
                if (pPatternConeCenters)
                    {
                    jmdlEmbeddedDPoint3dArray_addDPoint3d (pPatternConeCenters, &centerC);
                    jmdlEmbeddedDPoint3dArray_addDPoint3d (pPatternConeCenters, &centerD);
                    }
                }
            }
        }
    return true;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
