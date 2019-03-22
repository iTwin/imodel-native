/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/vutriang.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/* special manipulations for triangles */

#define MAG2(Z) (Z.x*Z.x + Z.y*Z.y)


/************************************************************
** VU Node labeling in adjacent triangles being flipped
  +---------------------+
  |F                 E/A|
  |                 /   |
  |               /     |
  |             /       |
  |           /         |
  |         /           |
  |       /             |
  |     /               |
  |   /                 |
  |D/B                 C|
  +---------------------+
*************************************************************/

typedef struct
    {
    /* Scales to apply to vectors in area and perimeter calculations. */
    double xScale;
    double yScale;
    /* If nonzero, periods in respective directions */
    double xPeriod;
    double yPeriod;
    } VuScaleAndPeriod;



/*----------------------------------------------------------------------+
|                                                                       |
| name          scaledPeriodicDifference                                |
|                                                                       |
| author        earlinLutz                                      08/02   |
|                                                                       |
| FIRST, restrict delta to a half period on either side of zero.        |
| SECOND, apply scale factor.                                           |
+----------------------------------------------------------------------*/
static double scaledPeriodicDifference
(
double delta,
double scale,
double period
)
    {
    if (period != 0.0)
        {
        double halfPeriod = 0.5 * period;
        if (fabs (delta) >= halfPeriod)
            {
            delta = bsiTrig_normalizeToPeriod (delta, -halfPeriod, period);
            }
        }
    if (scale != 0.0)
        delta *= scale;

    return delta;
    }

static double vu_scaledPeriodicCross
(
VuP pNode0,
VuP pNode1,
VuP pNode2,
VuScaleAndPeriod *pData
)
    {
    if (pData)
        {
        DPoint3d U, V;
        DPoint3d point0, point1, point2;

        vu_getDPoint3d (&point0, pNode0);
        vu_getDPoint3d (&point1, pNode1);
        vu_getDPoint3d (&point2, pNode2);
        U.x = scaledPeriodicDifference (point1.x - point0.x, pData->xScale, pData->xPeriod);
        U.y = scaledPeriodicDifference (point1.y - point0.y, pData->yScale, pData->yPeriod);

        V.x = scaledPeriodicDifference (point2.x - point1.x, pData->xScale, pData->xPeriod);
        V.y = scaledPeriodicDifference (point2.y - point1.y, pData->yScale, pData->yPeriod);

        return U.x * V.y - U.y * V.x;
        }
    else
        {
        return vu_cross (pNode0, pNode1, pNode2);
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_scaledPeriodicQuadraticAspectRatio                   |
|                                                                       |
| author        earlinLutz                                      08/02   |
|                                                                       |
| Compute the aspect ratio of a triange, defined as                     |
|  mu = area/ sum of squared edge lengths                               |
| This is large and positive for good CCW triangles,                    |
| small positive for 'flat' (near degenerate) CCW triangles,            |
| zero for true degenerate triangle                                     |
| small negative for 'flat' (near degenerate) CW triangles,             |
| large negative for non-flat CW triangles.                             |
|
+----------------------------------------------------------------------*/
static int vu_scaledPeriodicQuadraticAspectRatio       /* <= ERROR if degenerate */
(
double  *ratioP,                /* <= returned aspect ratio.  0 if ERROR */
DPoint2d *point0P,
DPoint2d *point1P,
DPoint2d *point2P,
VuScaleAndPeriod *pData
)
    {
    DPoint2d U,V,W;
    double area;        /* SIGNED area of the triangle */
    double perim;       /* sum of SQUARED edge lengths */
    int status;
    /* Compute vectors along each edge */
    if (pData)
        {
        U.x = scaledPeriodicDifference (point1P->x - point0P->x, pData->xScale, pData->xPeriod);
        U.y = scaledPeriodicDifference (point1P->y - point0P->y, pData->yScale, pData->yPeriod);

        V.x = scaledPeriodicDifference (point2P->x - point1P->x, pData->xScale, pData->xPeriod);
        V.y = scaledPeriodicDifference (point2P->y - point1P->y, pData->yScale, pData->yPeriod);

        W.x = scaledPeriodicDifference (point0P->x - point2P->x, pData->xScale, pData->xPeriod);
        W.y = scaledPeriodicDifference (point0P->y - point2P->y, pData->yScale, pData->yPeriod);
        }
    else
        {
        U.x = point1P->x - point0P->x;
        U.y = point1P->y - point0P->y;

        V.x = point2P->x - point1P->x;
        V.y = point2P->y - point1P->y;

        W.x = point0P->x - point2P->x;
        W.y = point0P->y - point2P->y;
        }

    area = U.x*V.y - U.y*V.x;
    perim  = (MAG2(U) + MAG2(V) + MAG2(W));
    if (perim > 0.0)
        {
        status = SUCCESS;
        *ratioP = area / perim;
        }
    else
        {
        status = ERROR;
        *ratioP = 0.0;
        }
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_scaledPeriodicQuadraticAspectRatio                   |
|                                                                       |
| author        earlinLutz                                      08/02   |
|                                                                       |
| Compute the aspect ratio of a triange, defined as                     |
|  mu = area/ sum of squared edge lengths                               |
| This variant uses full 3D coordinates.                                |
|
+----------------------------------------------------------------------*/
static int vu_xyzAspectRatio /* <= ERROR if degenerate */
(
DPoint3dCR xyz0,
DPoint3dCR xyz1,
DPoint3dCR xyz2,
double &ratio
)
    {
    DVec3d U01, U12, U20, W;
    int status;
    U01.DifferenceOf (xyz1, xyz0);
    U12.DifferenceOf (xyz2, xyz1);
    U20.DifferenceOf (xyz0, xyz2);
    double perim = U01.MagnitudeSquared () + U12.MagnitudeSquared () + U20.MagnitudeSquared ();
    W.CrossProduct (U01, U12);
    double area = W.Magnitude ();
    if (perim > 0.0)
        {
        status = SUCCESS;
        ratio = area / perim;
        }
    else
        {
        status = ERROR;
        ratio = 0.0;
        }
    return status;
    }

template <typename UserDataType>
bool TryFlip
(
VuSetP graphP,
bool (*testFuncP)(VuSetP,VuP,UserDataType *),
UserDataType *userDataP,
VuMarkedEdgeSetP edgeSetP,
VuP AP                              // Edge to consider for flip.
)
    {
    VuP BP,CP,DP,EP,FP;
    bool flipped = false;
    if( (*testFuncP)(graphP,AP,userDataP) )
        {
        /* Extract the edge. */
        BP = VU_FSUCC(AP);
        DP = VU_VSUCC(BP);
        EP = VU_FSUCC(DP);
        CP = VU_FSUCC(BP);
        FP = VU_FSUCC(EP);

        vu_vertexTwist (graphP, AP,EP);
        vu_vertexTwist (graphP, BP,DP);
        /* Mark the boundaries of the face */
        VU_FACE_LOOP(currP,EP)
            vu_markedEdgeSetTestAndAdd(edgeSetP,currP);
        END_VU_FACE_LOOP(currP,EP)
        /* Reinsert the edge */
        vu_vertexTwist (graphP, AP,CP);
        vu_vertexTwist (graphP, DP,FP);
        vu_copyCoordinates(AP,CP);
        vu_copyCoordinates(DP,FP);
        vu_copyConditionalVertexData (graphP, CP, AP);
        vu_copyConditionalVertexData (graphP, FP, DP);
        flipped = true;
        }
    return flipped;
    }


template <typename UserDataType>
static int vu_flipEdgesFromEdgeSet
(
VuSetP graphP,
bool (*testFuncP)(VuSetP,VuP,UserDataType *),
UserDataType *userDataP,
VuMarkedEdgeSetP edgeSet,
size_t maxFlip
)
    {
    size_t numFlip = 0;
    int numCompletedFlip = 0;
    VuP seedNode;
    for(;
        numFlip++ < maxFlip && NULL != (seedNode = vu_markedEdgeSetChooseAny(edgeSet))
        ;)
        {
        if (TryFlip (graphP, testFuncP, userDataP, edgeSet, seedNode))
            numCompletedFlip++;
        }
    return numCompletedFlip;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_flipEdges                                            |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
template <typename UserDataType>
static int vu_flipEdges
(
VuSetP graphP,
bool (*testFuncP)(VuSetP,VuP,UserDataType *),
UserDataType *userDataP,
int maxFlipsPerEdge = 60
)
    {
    VuP AP;
    /* EDL: I observe convergence in large graphs in flip count of about 0.75 * number of vu nodes.
       (about 1.5 flips per edge)
       Extreme mismatches of edge lengths frustrate this.
       4 works fine for "most" situations.
      push it to 20
      EDL Sept 08: In a (nearly rectanglar) polygon with long sides divided into very short sticks,
        this needs far more flips.
        (edge count, flip factor) observations (1K,8)(1.3K, 12)(2K,25) (5K,55)
        (This is clearly an extreme configuration -- the 4X range numbers are still typical)
    */
    static double s_baseFlipFactor = 20;
    static double s_flipDivisor = 60;
    double numFlip = 0.0;
    int numFlip1 = 0;
    double maxFlip;
    double factor1;
    double factor2;
    if (graphP)
        {
        VuMarkedEdgeSetP edgeSetP = vu_markedEdgeSetNew(
            graphP, VU_ALL_FIXED_EDGES_MASK );

        /* Fill the new set with all allowable edges */
        maxFlip = 0;
        VU_SET_LOOP(nodeP,graphP)
            {
            vu_markedEdgeSetTestAndAdd(edgeSetP,nodeP);
            maxFlip++;
            }
        END_VU_SET_LOOP(nodeP,graphP)

        /* Remove and flip edges until the set is exhausted */
        factor1 = maxFlip / s_flipDivisor;
        factor2 = s_baseFlipFactor;
        if (factor1 > factor2)
            factor2 = factor1;
        if (factor2 > maxFlipsPerEdge)
            factor2 = maxFlipsPerEdge;
        maxFlip *= factor2;
        for(;
           numFlip++ < maxFlip && NULL != (AP = vu_markedEdgeSetChooseAny(edgeSetP))
           ;)
            {
            if (TryFlip (graphP, testFuncP, userDataP, edgeSetP, AP))
                numFlip1++;
            }
        vu_markedEdgeSetFree(edgeSetP);
        }
    return numFlip1;
    }
/*----------------------------------------------------------------------+
* @param graphP IN containing graph
* @param AP IN base node of edge candidate
* @param userDataP IN unused.
+----------------------------------------------------------------------*/
static bool    vu_mappedCoordinateFlipTest
(
VuSetP  graphP,
VuP     AP,
VuCoordinateMappingFunction *function
)
    {
    static double s_smallRatio = 1.0e-6;
    static double s_smallRatioImprovement = 1.25;
    static double s_zeroRatio = 1.0e-6;
    double muCurrent, muFlipped;
    
    /* Identify all nodes of the adjacent triangles, as labeled above */
    VuP BP = VU_FSUCC(AP);
    VuP CP = VU_FSUCC(BP);
    VuP DP = VU_VSUCC(BP);
    VuP EP = VU_FSUCC(DP);
    VuP FP = VU_FSUCC(EP);
    bool    swap_required = false;
    if(VU_FSUCC(CP) == AP && VU_FSUCC(FP) == DP)
        {
        DPoint2d xyAE = VU_UV(AP);
        DPoint2d xyBD = VU_UV(BP);
        DPoint2d xyC  = VU_UV(CP);
        DPoint2d xyF  = VU_UV(FP);
        DPoint3d xyzAE, xyzBD, xyzC, xyzF;
        
        /* Compute the asect ratios of each triangle under the
           current and flipped configuration.
        */
        double muABC, muDEF, muCAF, muCFB;
        if (
               function->TryMapCoordinates (graphP, AP, xyAE, xyzAE)
            && function->TryMapCoordinates (graphP, BP, xyBD, xyzBD)
            && function->TryMapCoordinates (graphP, CP, xyC, xyzC)
            && function->TryMapCoordinates (graphP, FP, xyF, xyzF)
            && SUCCESS == vu_xyzAspectRatio (xyzAE,xyzBD,xyzC, muABC)
            && SUCCESS == vu_xyzAspectRatio (xyzBD,xyzAE,xyzF, muDEF)
            && SUCCESS == vu_xyzAspectRatio (xyzC, xyzAE,xyzF, muCAF)
            && SUCCESS == vu_xyzAspectRatio (xyzC, xyzF, xyzBD, muCFB)
           )
            {
            DVec3d normalABC, normalDEF, normalFCA, normalCFD;
            normalABC.CrossProductToPoints (xyzAE, xyzBD, xyzC);
            normalDEF.CrossProductToPoints (xyzBD, xyzAE, xyzF);
            normalFCA.CrossProductToPoints (xyzF, xyzC, xyzAE);
            normalCFD.CrossProductToPoints (xyzC, xyzF, xyzBD);
            double dot0 = normalABC.DotProduct (normalDEF);
            double dot1 = normalFCA.DotProduct (normalCFD);
            if (dot0 < 0.0)
                {
                if (dot1 > 0.0)
                    return true;    // bad to good !!!
                return false;       // bad to bad??
                }
            else if (dot1 < 0.0)
                {
                return false;   // good to bad
                }
            /* Select the configuration with the better aspect ratio in its
               poorer triangle */
            if (fabs (muABC) < s_zeroRatio)
                muABC = 0.0;
            if (fabs (muDEF) < s_zeroRatio)
                muDEF = 0.0;
            if (fabs (muCAF) < s_zeroRatio)
                muCAF = 0.0;
            if (fabs (muCFB) < s_zeroRatio)
                muCFB = 0.0;

            muCurrent = ( muABC < muDEF ? muABC : muDEF );
            muFlipped = ( muCAF < muCFB ? muCAF : muCFB );
            if (muCurrent < s_smallRatio)
                {
                if (muFlipped > muCurrent * s_smallRatioImprovement)
                    swap_required = true;
                }
            else
                {
                if(muFlipped > muCurrent)
                    swap_required = true;
                }
            }
        }
    return swap_required;
    }


/*----------------------------------------------------------------------+
* @param graphP IN containing graph
* @param AP IN base node of edge candidate
* @param userDataP IN unused.
+----------------------------------------------------------------------*/
static bool    vu_incircleFlipTest
(
VuSetP  graphP,
VuP     AP,
VuScaleAndPeriod *pData
)
    {
    /* Identify all nodes of the adjacent triangles, as labeled above */
    VuP BP = VU_FSUCC(AP);
    VuP CP = VU_FSUCC(BP);
    VuP DP = VU_VSUCC(BP);
    VuP EP = VU_FSUCC(DP);
    VuP FP = VU_FSUCC(EP);
    bool    swap_required = false;
    if(VU_FSUCC(CP) == AP && VU_FSUCC(FP) == DP)
        {
        DPoint2d xyAE = VU_UV(AP);
        DPoint2d xyBD = VU_UV(BP);
        DPoint2d xyC  = VU_UV(CP);
        DPoint2d xyF  = VU_UV(FP);
        DVec2d uvF = DVec2d::FromStartEnd (xyAE, xyF);
        DVec2d uvB = DVec2d::FromStartEnd (xyAE, xyBD);
        DVec2d uvC = DVec2d::FromStartEnd (xyAE, xyC);
//                    *
//                   /|\
//                  /B|D\
//                 /  |  \
//                /   |   \
//               *C   |   F*
//                \   |   /
//                 \  |  /
//                  \A|E/
//                   \|/
//                    *
        RotMatrix Q = RotMatrix::FromRowValues
            (
            uvF.x, uvF.y, uvF.x * uvF.x + uvF.y * uvF.y,
            uvC.x, uvC.y, uvC.x * uvC.x + uvC.y * uvC.y,
            uvB.x, uvB.y, uvB.x * uvB.x + uvB.y * uvB.y
            );
        swap_required = Q.Determinant () > 0.0;
        }
    return swap_required;
    }

/*----------------------------------------------------------------------+
* @param graphP IN containing graph
* @param AP IN base node of edge candidate
* @param userDataP IN unused.
+----------------------------------------------------------------------*/
static bool    vu_scaledPeriodicQuadraticFlipTest
(
VuSetP  graphP,
VuP     AP,
VuScaleAndPeriod *pData
)
    {
    static double s_smallRatio = 1.0e-6;
    static double s_smallRatioImprovement = 1.25;
    static double s_zeroRatio = 1.0e-6;
    double muCurrent, muFlipped;
    /* Identify all nodes of the adjacent triangles, as labeled above */
    VuP BP = VU_FSUCC(AP);
    VuP CP = VU_FSUCC(BP);
    VuP DP = VU_VSUCC(BP);
    VuP EP = VU_FSUCC(DP);
    VuP FP = VU_FSUCC(EP);
    bool    swap_required = false;
    if(VU_FSUCC(CP) == AP && VU_FSUCC(FP) == DP)
        {
        DPoint2d xyAE = VU_UV(AP);
        DPoint2d xyBD = VU_UV(BP);
        DPoint2d xyC  = VU_UV(CP);
        DPoint2d xyF  = VU_UV(FP);
        /* Compute the asect ratios of each triangle under the
           current and flipped configuration.
        */
        double muABC, muDEF, muCAF, muCFB;
        if (
               SUCCESS == vu_scaledPeriodicQuadraticAspectRatio(&muABC, &xyAE,&xyBD,&xyC, pData)
            && SUCCESS == vu_scaledPeriodicQuadraticAspectRatio(&muDEF, &xyBD,&xyAE,&xyF, pData)
            && SUCCESS == vu_scaledPeriodicQuadraticAspectRatio(&muCAF, &xyC, &xyAE,&xyF, pData)
            && SUCCESS == vu_scaledPeriodicQuadraticAspectRatio(&muCFB, &xyC, &xyF, &xyBD, pData)
           )
            {
            /* Select the configuration with the better aspect ratio in its
               poorer triangle */
            if (fabs (muABC) < s_zeroRatio)
                muABC = 0.0;
            if (fabs (muDEF) < s_zeroRatio)
                muDEF = 0.0;
            if (fabs (muCAF) < s_zeroRatio)
                muCAF = 0.0;
            if (fabs (muCFB) < s_zeroRatio)
                muCFB = 0.0;

            muCurrent = ( muABC < muDEF ? muABC : muDEF );
            muFlipped = ( muCAF < muCFB ? muCAF : muCFB );
            if (muCurrent < s_smallRatio)
                {
                if (muFlipped > muCurrent * s_smallRatioImprovement)
                    swap_required = true;
                }
            else
                {
                if(muFlipped > muCurrent)
                    swap_required = true;
                }
            }
        }
#ifdef CompareToIncircle
    bool incircle = vu_incircleFlipTest (graphP, AP, pData);
    static size_t s_numDifferent = 0;
    static size_t s_numSame = 0;
    static bool s_useIncircle = true;
    if (incircle == swap_required)
        s_numSame++;
    else
        s_numDifferent++;
    return s_useIncircle ? incircle : swap_required;
#else
    return swap_required;
#endif
    }



#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
@description Test if the aspect ratios the two triangles at an edge would be improved
    if the edge were to flipped.   Intended for use as a callback by flip control logic.
    This callback uses coordinates exactly as given in the nodes -- no scaling, no periods.
@param graphP IN containing graph
@param AP IN base node of edge candidate
@param userDataP IN unused.
@return true if flipping improves triangle shape.
@group "VU Meshing"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    vu_quadraticFlipTest
(
VuSetP  graphP,
VuP     AP,
VuScaleAndPeriod    *userDataP
)
    {
    return vu_scaledPeriodicQuadraticFlipTest (graphP, AP, NULL);
    }
#endif
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
@description Test if the aspect ratios the two triangles at an edge would be improved
    if the edge were to flipped.   Intended for use as a callback by flip control logic.
    This callback uses coordinates applies an x scale factor.
@param graphP IN containing graph
@param AP IN base node of edge candidate
@param userDataP IN pointer to a double to be applied to x coordinates.
@return true if flipping improves triangle shape.
@group "VU Meshing"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    vu_scaledQuadraticFlipTest
(
VuSetP  graphP,
VuP     AP,
VuScaleAndPeriod    *userDataP
)
    {
    VuScaleAndPeriod data;
    data.xScale = *(double*)userDataP;
    data.yScale = data.xPeriod = data.yPeriod = 0.0;
    return vu_scaledPeriodicQuadraticFlipTest (graphP, AP, &data);
    }
#endif
/*---------------------------------------------------------------------------------**//**
@description Test if the aspect ratios the two triangles at an edge would be improved
    if the edge were to flipped.   Intended for use as a callback by flip control logic.
    This callback only considers U direction aspect ratio effects, with periodic coordinates.
@param graphP IN containing graph
@param AP IN base node of edge candidate
@param pPeriodData IN scale and period data.
@return true if flipping improves triangle shape.
@group "VU Meshing"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    vu_periodicUFlipTest
(
VuSetP  graphP,
VuP     AP,
VuScaleAndPeriod *pPeriodData
)
    {
    double muCurrent, muFlipped;
    double duA, duC;
    double halfPeriod;
    /* Identify all nodes of the adjacent triangles, as labeled above */
    VuP BP = VU_FSUCC(AP);
    VuP CP = VU_FSUCC(BP);
    VuP DP = VU_VSUCC(BP);
    VuP EP = VU_FSUCC(DP);
    VuP FP = VU_FSUCC(EP);
    bool    swap_required = false;
    if(VU_FSUCC(CP) == AP && VU_FSUCC(FP) == DP)
        {
        duA = VU_U(AP) - VU_U(BP);
        duC = VU_U(CP) - VU_U(FP);

        if (pPeriodData && pPeriodData->xPeriod != 0.0)
            {
            halfPeriod = 0.5 * pPeriodData->xPeriod;
            duA = bsiTrig_normalizeToPeriod (duA, -halfPeriod, pPeriodData->xPeriod);
            duC = bsiTrig_normalizeToPeriod (duC, -halfPeriod, pPeriodData->xPeriod);
            }

        muCurrent = fabs(duA);
        muFlipped = fabs(duC);

        if  (muFlipped < muCurrent
            && vu_scaledPeriodicCross (BP, CP, FP, pPeriodData) > 0.0
            && vu_scaledPeriodicCross (EP, FP, CP, pPeriodData) > 0.0
            )
            swap_required = true;
        }
    return swap_required;
    }
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
@description Test if the aspect ratios the two triangles at an edge would be improved
    if the edge were to flipped.   Intended for use as a callback by flip control logic.
    This callback only considers U direction aspect ratio effects, with non-periodic coordiantes
@param graphP IN containing graph
@param AP IN base node of edge candidate
@param userDataP IN unused.
@return true if flipping improves triangle shape.
@group "VU Meshing"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    vu_UFlipTest
(
VuSetP  graphP,
VuP     AP,
void    *userDataP
)
    {
    return vu_periodicUFlipTest (graphP, AP, NULL);
    }
#endif
/*---------------------------------------------------------------------------------**//**
@description Test if the aspect ratios the two triangles at an edge would be improved
    if the edge were to flipped.   Intended for use as a callback by flip control logic.
    This callback only considers V direction aspect ratio effects, with periodic coordiantes
@param graphP IN containing graph
@param AP IN base node of edge candidate
@param userDataP IN unused.
@return true if flipping improves triangle shape.
@group "VU Meshing"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    vu_periodicVFlipTest
(
VuSetP  graphP,
VuP     AP,
VuScaleAndPeriod *pPeriodData
)
    {
    double muCurrent, muFlipped;
    double dvA, dvC;
    double halfPeriod;
    /* Identify all nodes of the adjacent triangles, as labeled above */
    VuP BP = VU_FSUCC(AP);
    VuP CP = VU_FSUCC(BP);
    VuP DP = VU_VSUCC(BP);
    VuP EP = VU_FSUCC(DP);
    VuP FP = VU_FSUCC(EP);
    bool swap_required = false;
    if(VU_FSUCC(CP) == AP && VU_FSUCC(FP) == DP)
        {
        dvA = VU_V(AP) - VU_V(BP);
        dvC = VU_V(CP) - VU_V(FP);

        if (pPeriodData && pPeriodData->yPeriod != 0.0)
            {
            halfPeriod = 0.5 * pPeriodData->yPeriod;
            dvA = bsiTrig_normalizeToPeriod (dvA, -halfPeriod, pPeriodData->yPeriod);
            dvC = bsiTrig_normalizeToPeriod (dvC, -halfPeriod, pPeriodData->yPeriod);
            }

        muCurrent = fabs(dvA);
        muFlipped = fabs(dvC);

        if  (muFlipped < muCurrent
            && vu_scaledPeriodicCross (BP, CP, FP, pPeriodData) > 0.0
            && vu_scaledPeriodicCross (EP, FP, CP, pPeriodData) > 0.0
            )
            swap_required = true;
        }
    return swap_required;
    }
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
@description Test if the aspect ratios the two triangles at an edge would be improved
    if the edge were to flipped.   Intended for use as a callback by flip control logic.
    This callback only considers V direction aspect ratio effects, with non-periodic coordiantes
@param graphP IN containing graph
@param AP IN base node of edge candidate
@param userDataP IN unused.
@group "VU Meshing"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    vu_VFlipTest
(
VuSetP  graphP,
VuP     AP,
void    *userDataP
)
    {
    return vu_periodicVFlipTest (graphP, AP, NULL);
    }
#endif

/*======================================================================+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+======================================================================*/
/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to improve aspect ratio, using caller-supplied coordinate mapper for distance and area calculations.
@remarks This is a substantial full-graph modification function.
        The goal is to increase the ratio of triangle area divided by sum of squared edge lengths.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
int VuFlipFunctions::FlipTrianglesToImproveMappedCoordinateAspectRatio
(
VuSetP  graphP,
VuCoordinateMappingFunction *mapper,
int maxFlipsPerEdge
)
    {
    return vu_flipEdges (graphP, vu_mappedCoordinateFlipTest, mapper, maxFlipsPerEdge);
    }

int vu_flipTrianglesToImproveMappedCoordinateAspectRatio
(
VuSetP  graphP,
VuCoordinateMappingFunction *mapper,
int maxFlipPerEdge
)
    {
    return VuFlipFunctions::FlipTrianglesToImproveMappedCoordinateAspectRatio (graphP, mapper, maxFlipPerEdge);
    }

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to improve aspect ratio.
@remarks This is a substantial full-graph modification function.
        The goal is to increase the ratio of triangle area divided by sum of squared edge lengths.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveQuadraticAspectRatio
(
VuSetP  graphP
)
    {
    VuScaleAndPeriod data;
    DPoint3d periods;
    data.xScale = 0.0;
    data.yScale = 0.0;
    vu_getPeriods (graphP, &periods);
    data.xPeriod = periods.x;
    data.yPeriod = periods.y;
    return vu_flipEdges (graphP, vu_scaledPeriodicQuadraticFlipTest, &data);
    }

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to improve aspect ratio.
@remarks This is a substantial full-graph modification function.
        The goal is to increase the ratio of triangle area divided by sum of squared edge lengths.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveQuadraticAspectRatio
(
VuSetP  graphP,
VuMarkedEdgeSetP edgeSet
)
    {
    static size_t s_maxFlip = 1000000;
    VuScaleAndPeriod data;
    DPoint3d periods;
    data.xScale = 0.0;
    data.yScale = 0.0;
    vu_getPeriods (graphP, &periods);
    data.xPeriod = periods.x;
    data.yPeriod = periods.y;
    return vu_flipEdgesFromEdgeSet (graphP, vu_scaledPeriodicQuadraticFlipTest, &data, edgeSet, s_maxFlip);
    }



/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to correct incircle condition
@remarks This is a substantial full-graph modification function.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod                                                    Earlin.Lutz     03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesForIncircle
(
VuSetP  graphP,
VuMarkedEdgeSetP edgeSet
)
    {
    static size_t s_maxFlip = 1000000;
    VuScaleAndPeriod data;
    DPoint3d periods;
    data.xScale = 0.0;
    data.yScale = 0.0;
    vu_getPeriods (graphP, &periods);
    data.xPeriod = periods.x;
    data.yPeriod = periods.y;

    return edgeSet == nullptr
            ? vu_flipEdges (graphP, vu_incircleFlipTest, &data)
            : vu_flipEdgesFromEdgeSet (graphP, vu_incircleFlipTest, &data, edgeSet, s_maxFlip);
    }



/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to improve aspect ratio.
@remarks This is a substantial full-graph modification function.
        The goal is to increase the ratio of triangle area divided by sum of squared edge lengths, with area and edge length computed
        <em>after</em> applying scale factors to the coordinates.
@param graphP IN OUT graph header
@param xScale IN scale factor for x-coordinates, or zero for no scale
@param yScale IN scale factor for y-coordinates, or zero for no scale
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveScaledQuadraticAspectRatio
(
VuSetP  graphP,
double xScale,
double yScale
)
    {
    VuScaleAndPeriod data;
    DPoint3d periods;
    data.xScale = xScale;
    data.yScale = yScale;
    vu_getPeriods (graphP, &periods);
    data.xPeriod = periods.x;
    data.yPeriod = periods.y;
    return vu_flipEdges (graphP, vu_scaledPeriodicQuadraticFlipTest, &data);
    }

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges in the graph to improve aspect ratio.
@remarks This is a substantial full-graph modification function.
        The goal is to increase the ratio of triangle area divided by sum of squared edge lengths, with area and edge length computed
        <em>after</em> applying scale factors to the coordinates, and with coordinates interpreted periodically.
@param graphP IN OUT graph header
@param xScale IN scale factor for x-coordinates, or zero for no scale
@param yScale IN scale factor for y-coordinates, or zero for no scale
@param xPeriod IN period for x-coordinates
@param yPeriod IN period for y-coordinates
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveScaledPeriodicQuadraticAspectRatio
(
VuSetP  graphP,
double xScale,
double yScale,
double xPeriod,
double yPeriod
)
    {
    VuScaleAndPeriod data;
    data.xScale = xScale;
    data.yScale = yScale;
    data.xPeriod = xPeriod;
    data.yPeriod = yPeriod;
    return vu_flipEdges (graphP, vu_scaledPeriodicQuadraticFlipTest, &data);
    }

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges to improve aspect ratio in the u-parameter direction.
@remarks This is a substantial full-graph modification function.
@remarks This variant is for triangles on a surface ruled in the y-coordinate direction and curved in the x-coordinate direction, thus long
    triangles in the y-coordinate direction are preferred.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveUAspectRatio
(
VuSetP  graphP
)
    {
    VuScaleAndPeriod data;
    DPoint3d periods;
    data.xScale = 1.0;
    data.yScale = 1.0;
    vu_getPeriods (graphP, &periods);
    data.xPeriod = periods.x;
    data.yPeriod = periods.y;
    return vu_flipEdges (graphP, vu_periodicUFlipTest, &data);
    }

/*---------------------------------------------------------------------------------**//**
@description Flip triangle edges to improve aspect ratio in the v-parameter direction.
@remarks This is a substantial full-graph modification function.
@remarks This variant is for triangles on a surface ruled in the x-coordinate direction and curved in the y-coordinate direction, thus long
    triangles in the x-coordinate direction are preferred.
@param graphP IN OUT graph header
@group "VU Meshing"
@usealinkgroup TriangleFlipFunctions
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_flipTrianglesToImproveVAspectRatio
(
VuSetP  graphP
)
    {
    VuScaleAndPeriod data;
    DPoint3d periods;
    data.xScale = 1.0;
    data.yScale = 1.0;
    vu_getPeriods (graphP, &periods);
    data.xPeriod = periods.x;
    data.yPeriod = periods.y;
    return vu_flipEdges (graphP, vu_periodicVFlipTest, &data);
    }

/*---------------------------------------------------------------------------------**//**
@description Triangulate a single face by adding edges outward from a fan point.
@param graphP IN OUT graph header
@param P IN fixed point of fan
@group "VU Meshing"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_triangulateFromFixedPoint
(
VuSetP          graphP,
VuP             P
)
    {
    VuP             Q = VU_FSUCC (VU_FSUCC (P));
    VuP             T = VU_FPRED (P);
    VuP             A, B;
    if (Q != P)
        {
        /* This catches degenerate cases
            of n=1 and n=2 edges in face.
           Case n=3 fails the first loop test.
        */
        while (Q != T)
            {
            vu_join (graphP, P, Q, &A, &B);
            Q = VU_FSUCC (Q);
            }
        }
    }

/*PF*/
/*---------------------------------------------------------------------------------**//**
@description Unconditionally flip a single edge.
@remarks When the edge separates two triangular faces, this has the effect of flipping the diagonal of the quad.
@param graphP IN OUT graph header
@param AP IN base node of edge to flip
@group "VU Meshing"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_flipOneEdge
(
VuSetP  graphP,
VuP AP
)
    {
    VuP  BP, CP, DP, EP, FP;
    /* Extract the edge. */
    BP = VU_FSUCC(AP);
    DP = VU_VSUCC(BP);
    EP = VU_FSUCC(DP);
    CP = VU_FSUCC(BP);
    FP = VU_FSUCC(EP);
    vu_vertexTwist (graphP, AP,EP);
    vu_vertexTwist (graphP, BP,DP);
    /* Reinsert the edge and update coordinates */
    vu_vertexTwist (graphP, AP,CP);
    vu_vertexTwist (graphP, DP,FP);
    vu_copyCoordinates(AP,CP);
    vu_copyCoordinates(DP,FP);
    }

static int vu_countAndMarkAroundFace /* <= 0 normal, 1 if overflowed */
(
int *markedCountP,      /* <= number of marked nodes */
int *unMarkedCountP,    /* <= number of unmarked nodes */
VuP *markedNodePP,      /* <= array of marked nodes */
VuP *unMarkedNodePP,    /* => array of unmarked nodes */
int maxMarked,          /* => max marked nodes to return */
int maxUnMarked,        /* <= max unmarked nodes to return */
VuP startP,             /* => start node on face */
VuMask countMask,       /* => mask to check */
VuMask markMask         /* => mask to set on each node on the face */
)
    {
    int status = SUCCESS;
    *markedCountP = *unMarkedCountP = 0;
    VU_FACE_LOOP( currP, startP )
        {
        VU_SETMASK( currP, markMask);
        if( VU_GETMASK(currP, countMask) )
            {
            if ( *markedCountP < maxMarked )
                {
                markedNodePP[*markedCountP] = currP;
                *markedCountP += 1;
                }
            else
                {
                status = ERROR;
                }
            }
        else
            {
            if ( *unMarkedCountP < maxUnMarked )
                {
                unMarkedNodePP[*unMarkedCountP] = currP;
                *unMarkedCountP += 1;
                }
            else
                {
                status = ERROR;
                }
            }
        }
    END_VU_FACE_LOOP ( currP, startP )
    return status;
    }

/*---------------------------------------------------------------------------------**//**
@description Subdivide the edges of a triangulation using the given test function, and retriangulate the affected faces.
@param graphP       IN OUT  graph header
@param F            IN      edge test function
@param userDataP    IN      argument for test function
@group "VU Meshing"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_subdivideAndRetriangulate
(
VuSetP graphP,
VuSubdivideEdgeFuncP F,
void *userDataP
)
    {
    VuMask newVertexMask = vu_grabMask (graphP);
    VuMask edgeVisitedMask = vu_grabMask (graphP);
    VuMask faceVisitedMask = vu_grabMask (graphP);
    VuMask allMasks = newVertexMask | edgeVisitedMask | faceVisitedMask;
    VuMask currSkipMask, newNodeMask;
    VuMask mateSkipMask;
    VuP newLeftP, newRightP;
    VuP leftP, rightP;
    VuP markedNodeP[3], unMarkedNodeP[3];
    int markedCount, unMarkedCount;
    VuP mateP;
    VuP node0P, node1P, node2P, node3P;
    double d1,d2;
    int i,j;

    vu_clearMaskInSet ( graphP, allMasks );
    currSkipMask = VU_EXTERIOR_EDGE | VU_NULL_EDGE | VU_NULL_FACE | edgeVisitedMask;
    mateSkipMask = VU_NULL_EDGE | VU_NULL_FACE | edgeVisitedMask;
    newNodeMask = edgeVisitedMask | newVertexMask;

    /* Test and mark edges */
    VU_SET_LOOP(currP, graphP )
        {
        mateP = VU_EDGE_MATE(currP);
        if ( !VU_GETMASK(currP, currSkipMask ) )
            {
            if ( !VU_GETMASK(mateP, mateSkipMask ) )
                {
                (*F)( VU_MSG_TEST_EDGE_SUBDIVISION, &newLeftP, &newRightP, graphP, currP, userDataP );
                if ( newLeftP && newRightP )
                    {
                    VU_SETMASK( newLeftP, newNodeMask );
                    VU_SETMASK( newRightP, newNodeMask );
                    }

                VU_SETMASK( currP, edgeVisitedMask );
                VU_SETMASK( mateP, edgeVisitedMask );
                }
            }
        }
    END_VU_SET_LOOP(currP, graphP );

    currSkipMask = VU_EXTERIOR_EDGE | VU_NULL_EDGE | VU_NULL_FACE | faceVisitedMask ;
    /* Retriangulate faces */
    VU_SET_LOOP(currP, graphP)
        {
        if( !VU_GETMASK( currP, currSkipMask )
          && !vu_countAndMarkAroundFace ( &markedCount, &unMarkedCount, markedNodeP, unMarkedNodeP, 3, 3, currP,
                                        newVertexMask, faceVisitedMask )
          && markedCount > 0
          && unMarkedCount == 3
          )
            {
            /* This face was originally a triangle.  1,2, or 3 new nodes were
                added in the edge phase. Connect up the new nodes */
            switch ( markedCount )
                {
                case 1:
                    node0P = markedNodeP[0];
                    node1P = VU_FSUCC( VU_FSUCC( node0P ) );
                    vu_join ( graphP, node0P, node1P, &newLeftP, &newRightP );
                    VU_SETMASK( newLeftP, newNodeMask );
                    VU_SETMASK( newRightP, newNodeMask );
                    (*F)( VU_MSG_NEW_EDGE_NEWV_OLDV, NULL, NULL, graphP, newLeftP, userDataP );
                    break;

                case 2:
                    node0P = markedNodeP[0];
                    node1P = markedNodeP[1];
                    vu_join ( graphP, node0P, node1P, &newLeftP, &newRightP );
                    VU_SETMASK( newLeftP, newNodeMask );
                    VU_SETMASK( newRightP, newNodeMask );
                    (*F)( VU_MSG_NEW_EDGE_NEWV_NEWV, NULL, NULL, graphP, newLeftP, userDataP );

                    /* Find which side of the new edge is part of a quad.  Subdivide the quad on its short
                        diagonal */
                    for ( i = 0; i < 2; i++ )
                        {
                        node0P = markedNodeP[i];
                        node1P = VU_FSUCC(node0P);
                        node2P = VU_FSUCC(node1P);
                        node3P = VU_FSUCC(node2P);
                        if( node2P != node0P && node3P != node0P )
                            {
                            d1 = vu_distanceSquared ( node0P, node2P );
                            d2 = vu_distanceSquared ( node1P, node3P );
                            /* join so the left side of the new edge is at the new node */
                            if( d1 <= d2 )
                                {
                                vu_join( graphP, node0P, node2P, &leftP, &rightP );
                                }
                            else
                                {
                                vu_join( graphP, node3P, node1P, &leftP, &rightP );
                                }
                            VU_SETMASK( leftP, newNodeMask );
                            (*F)( VU_MSG_NEW_EDGE_NEWV_OLDV, NULL, NULL, graphP, leftP, userDataP );
                            }
                        }
                    break;

                case 3:
                    for ( i = 0 ; i < 3; i++ )
                        {
                        j = (i+1) % 3;
                        node0P = markedNodeP[i];
                        node1P = markedNodeP[j];
                        vu_join( graphP, node0P, node1P, &leftP, &rightP );
                        VU_SETMASK( leftP, newNodeMask );
                        VU_SETMASK( rightP, newNodeMask );
                        (*F)( VU_MSG_NEW_EDGE_NEWV_NEWV, NULL, NULL, graphP, leftP, userDataP );
                        markedNodeP[i] = leftP;         /* So the 3rd edge in gets to the right place */
                        }
                    break;
                }
            }
        }
    END_VU_SET_LOOP(currP, graphP)

    vu_returnMask ( graphP, faceVisitedMask );
    vu_returnMask ( graphP, edgeVisitedMask );
    vu_returnMask ( graphP, newVertexMask );
    }

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

typedef struct regularizationState
    {
    VuMask upEdgeMask;                /* During initialization, each edge is marked as
                                                up or down.  Just a minor optimization */
    VuMask visitedBySomeChannelMask;  /* This bit is cleared during initialization.  As channels
                                                creep upward, the edges that have been reached
                                                are marked.  When a channel spills over
                                                a peak into a neighboring channel, it continues
                                                setting these markers as it takes over the adjacent channel.
                                                Each channel dies if the bit is set at the start of
                                                the search upwards to seek containment of a downmin
                                      */
    VuMask downwardMinMask;             /* Distinguishes down mins from up mins in rsP->minArrayP */

    VuArrayP minArrayP; /* Array of vu's which are downward minima,
                                        eventually sorted by (lexical) y */
    VuArrayP channelArrayP;     /* Array of blocks of 3 pointers that defin
                                        each channel */
    VuSetP graphP;
    } RegularizationState;

/*
   A channel is stored as a block of 3 vu pointers in a vuarray.
   These access macros are used only by the channel access functions
*/
#define VUTRIANG_CHANNEL_NODES 3
#define VUTRIANG_CHANNEL_LEFT_VU(bufferP)  (bufferP[0])
#define VUTRIANG_CHANNEL_RIGHT_VU(bufferP) (bufferP[1])
#define VUTRIANG_CHANNEL_HIGHEST_VU(bufferP)  (bufferP[2])

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_openChannelScan                                |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void vutriang_openChannelScan
(
RegularizationState *rsP        /* => regularization scan state containing the channels to be scanned */
)
    {
    vu_arrayOpen( rsP->channelArrayP );
    }

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_readChannelScan                                |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static int vutriang_readChannelScan
(
RegularizationState *rsP,       /* => regularization scan state containing the channels to be scanned */
VuP *leftPP,                    /* => VU at the left of the channel */
VuP *rightPP,                   /* => VU at the right of the channel */
VuP *topPP                      /* => topmost VU in the channel */
)
    {
    VuP channelNodes[VUTRIANG_CHANNEL_NODES];
    int readStat = vu_arrayReadBlock( rsP->channelArrayP , channelNodes, VUTRIANG_CHANNEL_NODES);
    if ( readStat )
        {
        *leftPP  =  VUTRIANG_CHANNEL_LEFT_VU(channelNodes);
        *rightPP = VUTRIANG_CHANNEL_RIGHT_VU(channelNodes);
        *topPP   =   VUTRIANG_CHANNEL_HIGHEST_VU(channelNodes);
        }
    return readStat;
    }

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_updateCurrentChannel                           |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void vutriang_updateCurrentChannel
(
RegularizationState *rsP,       /* => regularization scan state whose current channel is being updated */
VuP leftP,                      /* => VU at the left of the channel */
VuP rightP,                     /* => VU at the right of the channel */
VuP topP                        /* => topmost VU in the channel */
)
    {
    VuP channelNodes[VUTRIANG_CHANNEL_NODES];
    VUTRIANG_CHANNEL_LEFT_VU(channelNodes) = leftP;
    VUTRIANG_CHANNEL_RIGHT_VU(channelNodes) = rightP;
    VUTRIANG_CHANNEL_HIGHEST_VU(channelNodes) = topP;

    vu_arrayReplaceBlock( rsP->channelArrayP , channelNodes, VUTRIANG_CHANNEL_NODES);
    }

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_addChannel                                     |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void vutriang_addChannel
(
RegularizationState *rsP,       /* => regularization scan state whose current channel is being updated */
VuP leftP,                      /* => VU at the left of the channel */
VuP rightP,                     /* => VU at the right of the channel */
VuP topP                        /* => topmost VU in the channel */
)
    {
    VuP channelNodes[VUTRIANG_CHANNEL_NODES];
    VUTRIANG_CHANNEL_LEFT_VU(channelNodes) = leftP;
    VUTRIANG_CHANNEL_RIGHT_VU(channelNodes) = rightP;
    VUTRIANG_CHANNEL_HIGHEST_VU(channelNodes) = topP;
    VU_SETMASK ( leftP, rsP->visitedBySomeChannelMask ) ;
    VU_SETMASK ( rightP, rsP->visitedBySomeChannelMask ) ;
    VU_SETMASK ( topP, rsP->visitedBySomeChannelMask ) ;

    vu_arrayAddBlock( rsP->channelArrayP , channelNodes, VUTRIANG_CHANNEL_NODES);
    }

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_deleteCurrentChannel                           |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void vutriang_deleteCurrentChannel
(
RegularizationState *rsP        /* => regularization scan state whose current channel is to be deleted */
)
    {
    vu_arrayDeleteBlock( rsP->channelArrayP, VUTRIANG_CHANNEL_NODES );
    }


/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_markUpEdges                                    |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void vutriang_markUpEdges
(
VuSetP graphP,          /* => graph whose upedges are to be marked */
VuMask  mask            /* => mask to install in each upedge ( and clear in downedge ) */
)
    {
    VuP nextP;
    VU_SET_LOOP( currP, graphP )
        {
        nextP = VU_FSUCC( currP );
        if ( VU_BELOW( currP, nextP ) )
            {
            VU_SETMASK( currP, mask );
            }
        else
            {
            VU_CLRMASK( currP, mask );
            }
        }
    END_VU_SET_LOOP( currP, graphP )
    }

static int s_downwardMinMaskForSorting;
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_compareLexicalUV                               |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static int vutriang_compareLexicalUV
(
const VuP * node0PP,
const VuP * node1PP
)
    {
    int result = vu_compareLexicalUV( node0PP, node1PP, NULL);
    if (  (result == 0 ) && (*node0PP != *node1PP ) )
        {
        if (VU_GETMASK ( *node0PP, s_downwardMinMaskForSorting ) )
            return -1;
        if (VU_GETMASK ( *node1PP, s_downwardMinMaskForSorting ) )
            return 1;
        }
    return result;
    }
/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_collectAndMarkLocalMinimaInMarkedGraph         |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static int vutriang_collectAndMarkLocalMinimaInMarkedGraph
(
RegularizationState *rsP
)
    {
    VuP nextP, secondP;
    VuMask upEdgeMask = rsP->upEdgeMask;
    double cross;
    int count = 0;
    /*** Min sorting needs to know what mask bit distinguishes downward mins. */
    vu_arrayClear( rsP->minArrayP );

    VU_SET_LOOP( currP, rsP->graphP )
        {
        nextP = VU_FSUCC( currP );
        if ( ! VU_GETMASK( currP , upEdgeMask ) && VU_GETMASK( nextP, upEdgeMask ) )
            {
            secondP = VU_FSUCC(nextP);
            cross = vu_cross( currP, nextP, secondP );
            vu_arrayAdd( rsP->minArrayP , nextP );
                if ( VU_VSUCC(nextP) == nextP ||  cross <= 0.0 )
                {
                VU_SETMASK( nextP, rsP->downwardMinMask );
                }
            }
        }
    END_VU_SET_LOOP ( currP, rsP->graphP );

    s_downwardMinMaskForSorting = rsP->downwardMinMask;
    vu_arraySort0 (rsP->minArrayP, vutriang_compareLexicalUV);
    return count;
    }

typedef enum
    {
    VUTRIANG_CHANNEL_DIED_BELOW_MIN,
    VUTRIANG_CHANNEL_BRACKETS_MIN,
    VUTRIANG_CHANNEL_BESIDE_MIN
    } VuTriangChannelUpdateStatus;

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_compareMinToChannel                            |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void vutriang_pinpointCrossing
(
double *uCrossP,        /* <= u coordinate where v crosses the edge */
VuP bottomP,            /* => VU at bottom of edge */
VuP topP,               /* => VU at top of edge */
double v,               /* => v of scan line */
double uError           /* => u to return if the edge is horizontal */
)
    {
    double v0 = VU_V( bottomP );
    double v1 = VU_V( topP );
    double dv = v1 - v0;

    if ( dv == 0.0 )
        {
        /* This is not supposed to happen. I'll return the error value just
                to be complete */
        *uCrossP = uError;
        }
    else
        {
        double s = ( v - v0 ) / dv;
        double u0 = VU_U( bottomP );
        double u1 = VU_U( topP );
        /* Interpolate with the nearer end as the 0 point.  */
        if ( s < 0.5 )
            {
            *uCrossP = u0 + s * (u1 - u0);
            }
        else
            {
            s = ( v1 - v ) / dv;
            *uCrossP = u1 + s * (u0 - u1);
            }
        }
    }

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_advanceChannelToMin                            |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static VuTriangChannelUpdateStatus vutriang_advanceChannelToMin
(
RegularizationState *rsP,
VuP *leftPP,
VuP *rightPP,
VuP *topPP,
VuP minP
)
    {
    VuP nextLeftP = NULL, nextRightP = NULL, testP;
    int walking;
    double uLeft, uRight, uMin, vMin;
    VuMask dropDeadMask = rsP->visitedBySomeChannelMask;
    VuMask upEdgeMask = rsP->upEdgeMask;
    double uError;
    /* Invariant onditions in a channel:
        (1) rightP is an upward oriented edges
        (2) VU_FPRED(leftP) to a downward oriented edge
        (3) leftP and rightP are strictly below the minP
        (4) all nodes at and below these nodes are marked with rsP->edgeInChannelMask

       Rules for advancing a channel:
        (A) If a marked node is encountered while creeping upward, the channel has been subsumed
                by another that crept up to a peak and down into this one.  KILL.
        (B) If a downward max is encountered before the height of the min, the channel dies.
                (If you trace past the downward max, you will come to a (lower min). But
                        the new min is claimed to be the lowest min. QED)
    */

    if (   VU_GETMASK( VU_FSUCC(*rightPP), dropDeadMask )
        || VU_GETMASK( VU_FPRED(*leftPP),  dropDeadMask ) )
        return VUTRIANG_CHANNEL_DIED_BELOW_MIN;

    /* Advance the left side so leftP..nextLeftP brackets the downmin */
    for ( walking = 1
        ;  walking
        ;
        )
        {
        nextLeftP = VU_FPRED ( *leftPP );

        if ( VU_GETMASK( nextLeftP, upEdgeMask ) )
            {
            /* leftP is a max. */
            testP = VU_FSUCC( *leftPP );
            if ( VU_VSUCC(*leftPP) == *leftPP || vu_cross( nextLeftP, *leftPP,  testP ) <= 0.0 )
                {
                /* We are crossing over a peak into the adjacent channel */
                /* Walk down to the bottom */
                if ( VU_BELOW( *topPP, *leftPP ) )
                    *topPP = *leftPP;
                do {
                    *leftPP = nextLeftP;
                    nextLeftP = VU_FPRED( nextLeftP );
                    VU_SETMASK( *leftPP, dropDeadMask );
                    } while ( VU_GETMASK( nextLeftP, upEdgeMask ));
                }
            else
                {
                /* Die at closed top of channel */
                return VUTRIANG_CHANNEL_DIED_BELOW_MIN;
                }
            }
        else if ( VU_GETMASK( nextLeftP, dropDeadMask ) )
            {
            return VUTRIANG_CHANNEL_DIED_BELOW_MIN;
            }
        else if ( VU_BELOW( nextLeftP, minP ) )
            {
            VU_SETMASK( nextLeftP , dropDeadMask );
            *leftPP = nextLeftP;
            }
        else
            {
            /* The channel has advanced to the level of the min */
            walking = 0;
            }
        }
    /* Advance the right side so rightP..nextrightP brackets the downmin */
    for ( walking = 1
        ;  walking
        ;
        )
        {
        nextRightP = VU_FSUCC ( *rightPP );

        if ( !VU_GETMASK( *rightPP, upEdgeMask ) )
            {
            /* rightP is a max. */
            testP = VU_FPRED( *rightPP );
            if ( VU_BELOW( *topPP, *rightPP ) )
                    *topPP = *rightPP;
            if ( VU_VSUCC(*rightPP) == *rightPP || vu_cross( testP, *rightPP, nextRightP ) <= 0.0 )
                {
                /* We are crossing over a peak into the adjacent channel */
                /* Walk down to the bottom */
                do {
                    *rightPP = nextRightP;
                    nextRightP = VU_FSUCC( nextRightP );
                    VU_SETMASK( *rightPP, dropDeadMask  );
                    } while ( !VU_GETMASK( *rightPP, upEdgeMask ));
                }
            else
                {
                /* Die at closed top of channel */
                return VUTRIANG_CHANNEL_DIED_BELOW_MIN;
                }
            }
        else if ( VU_GETMASK( nextRightP, dropDeadMask ) )
            {
            return VUTRIANG_CHANNEL_DIED_BELOW_MIN;
            }
        else if ( VU_BELOW( nextRightP, minP ) )
            {
            VU_SETMASK( nextRightP, dropDeadMask );
            *rightPP = nextRightP;
            }
        else
            {
            /* The channel has advanced to the level of the min */
            walking = 0;
            }
        }

    if ( VU_BELOW( *topPP, *leftPP ) )
        *topPP = *leftPP;
    if ( VU_BELOW( *topPP, *rightPP ) )
        *topPP = *rightPP;

    uMin = VU_U ( minP );
    vMin = VU_V ( minP );
    uError = uMin; /* Should never be used?   */
    /* See if minP is inside or outside the bracket */
    vutriang_pinpointCrossing( &uLeft, *leftPP, nextLeftP , vMin, uError );
    if ( uLeft >= uMin ) return VUTRIANG_CHANNEL_BESIDE_MIN;

    vutriang_pinpointCrossing( &uRight, *rightPP, nextRightP, vMin, uError);
    if ( uRight <= uMin ) return VUTRIANG_CHANNEL_BESIDE_MIN;


    return VUTRIANG_CHANNEL_BRACKETS_MIN;
    }
/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_connectMinToChannel                            |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void vutriang_connectMinToChannel
(
RegularizationState *rsP,
VuP minP,                       /* => Pointer to the downward min node */
VuP leftP,
VuP rightP,
VuP topP
)
    {
    VuP startP, endP;
    vu_join ( rsP->graphP, topP, minP, &startP, &endP );
    VU_SETMASK( startP, rsP->upEdgeMask );

    if ( topP == leftP )
        {
        vutriang_addChannel( rsP, startP, minP, minP );
        vutriang_updateCurrentChannel ( rsP, endP, rightP, endP );
        }
    else if ( topP == rightP )
        {
        vutriang_addChannel( rsP, endP, rightP, endP );
        vutriang_updateCurrentChannel ( rsP, leftP, minP, minP );
        }
    else /* connection to a peak within the interval */
        {
        vutriang_addChannel( rsP, endP, rightP, endP );
        vutriang_updateCurrentChannel ( rsP, leftP, minP, minP );
        }
    }

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_insertDownwardMin                              |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void vutriang_insertDownwardMin
(
RegularizationState *rsP,
VuP minP                        /* => Pointer to the downward min node */
)
    {
    VuP leftP, rightP, topP;    /* left, right and top of the 'channel' */
    VuTriangChannelUpdateStatus newState;
    //int join = 0;

    /* Search for the (unique or nonexistent) channel that brackets this min */
    for ( vutriang_openChannelScan( rsP );
          vutriang_readChannelScan( rsP, &leftP, &rightP, &topP ) ; )
        {
        newState = vutriang_advanceChannelToMin ( rsP, &leftP, &rightP, &topP, minP );
        switch ( newState )
            {
            case VUTRIANG_CHANNEL_DIED_BELOW_MIN:
                vutriang_deleteCurrentChannel( rsP );
                break;
            case VUTRIANG_CHANNEL_BRACKETS_MIN:
                vutriang_connectMinToChannel( rsP, minP, leftP, rightP, topP );
                return;
                break;
            case VUTRIANG_CHANNEL_BESIDE_MIN:
                vutriang_updateCurrentChannel( rsP, leftP, rightP, topP );
                break;
           }
        }

    return;
    }


/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_closeSweep                                     |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void vutriang_closeSweep
(
RegularizationState *rsP
)
    {
    vu_returnMask ( rsP->graphP, rsP->upEdgeMask );
    vu_returnMask ( rsP->graphP, rsP->downwardMinMask );
    vu_returnMask ( rsP->graphP, rsP->visitedBySomeChannelMask );
    vu_returnArray ( rsP->graphP, rsP->minArrayP );
    vu_returnArray ( rsP->graphP, rsP->channelArrayP );
    }
/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_openSweep                                      |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static int vutriang_openSweep
(
RegularizationState *rsP,
VuSetP graphP
)
    {
    int status;
    rsP->graphP = graphP;
    rsP->upEdgeMask = vu_grabMask (graphP);
    rsP->visitedBySomeChannelMask = vu_grabMask (graphP);
    rsP->downwardMinMask = vu_grabMask (graphP);
    rsP->minArrayP = vu_grabArray( graphP );

    rsP->channelArrayP = vu_grabArray ( graphP );
    vu_clearMaskInSet( graphP, rsP->visitedBySomeChannelMask | rsP->upEdgeMask | rsP->downwardMinMask );

    if ( rsP->upEdgeMask &&
         rsP->minArrayP &&
         rsP->channelArrayP
        )
        {
        status = SUCCESS;
        }
    else
        {
        vutriang_closeSweep ( rsP );
        status = ERROR;
        }
    return status;
    }

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_halfRegularizeGraph                            |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
| Sweep from bottom to top to connect each downward min to a node below |
| (without crossing any lines of the polygon)                           |
+----------------------------------------------------------------------*/
static void vutriang_halfRegularizeGraph
(
VuSetP graphP
)
    {

    RegularizationState regstate;
    VuP minP;

    if ( SUCCESS == vutriang_openSweep ( &regstate, graphP ) )
        {
        vutriang_markUpEdges( regstate.graphP , regstate.upEdgeMask );
        vutriang_collectAndMarkLocalMinimaInMarkedGraph( &regstate );
        vu_arrayOpen ( regstate.minArrayP );
        while ( vu_arrayRead ( regstate.minArrayP, &minP ) )
            {
            if ( VU_GETMASK ( minP, regstate.downwardMinMask ) )
                {
                vutriang_insertDownwardMin ( &regstate, minP );
                }
            else
                {
                vutriang_addChannel ( &regstate, minP, minP, minP );
                }
            }
        vutriang_closeSweep ( &regstate );
        }
    }

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vutriang_regularizeGraph                                |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
| regularize the y coordinates in a graph, i.e. insert edges so each    |
| face has a single local min and a single local max                    |
+----------------------------------------------------------------------*/
static void vutriang_regularizeGraph
(
VuSetP graphP   /* <=> graph to be regularized */
)
    {
    static int sweepUp = 1;
    static int sweepDown = 1;

    if ( sweepUp )
        {
        vutriang_halfRegularizeGraph( graphP);
        }

    if ( sweepDown )
        {
        vu_rotate180( graphP );
        vutriang_halfRegularizeGraph( graphP);
        vu_rotate180( graphP );
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Add edges so that holes are connected to outer boundaries, and each face
    has only one chain of continguous "up" edges and one chain of contiguous "down" edges.
@remarks This is a critical step in triangulation.   Once the faces are regularized, triangulation
    edges are easy to add in a single bottom-to-top sweep.
@remarks Currently, this function is a synonym for ~mvureg_regularizeGraph.
@param graphP IN OUT graph header
@group "VU Meshing"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_regularizeGraph
(
VuSetP graphP
)
    {
    static int regularizer = 0;
    if (regularizer == 0)
        {
        vureg_regularizeGraph (graphP);
        }
    else
        {
        vutriang_regularizeGraph (graphP);
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE
