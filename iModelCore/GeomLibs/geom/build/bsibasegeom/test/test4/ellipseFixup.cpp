#include <msgeomstructs.h>
#include <ellipseFixup.fdf>

//#define DEBUG_PRINT
//#define COMPILE_FOR_JMDL
#ifdef DEBUG_PRINT
extern void showDouble (char *, double);
extern void showInt (char *, int);
#endif

#ifdef COMPILE_FOR_JMDL
#define bsiDPoint3d_distance jmdlDPoint3d_distance
#define bsiDEllipse3d_initFrom3DPoint3dOnArc jmdlDEllipse3d_initFrom3DPoint3dOnArc
#define bsiDEllipse3d_fractionParameterToDPoint3d  jmdlDEllipse3d_fractionParameterToDPoint3d
#endif

/*---------------------------------------------------------------------------------**//**
@description Input is 3 points on an arc, plus a function which will be applied to
    convert points to a truncated format.   This function computes the center of the
    circle through the 3 true points, and then returns three truncated points which can
    serve as start-middle-end of an arc whose center is close to the base center in spite of
    noise introduced by the truncation.
@param pStartOut OUT the (truncated) start point.
@param pMiddleOut OUT the (truncated) along-arc point. This will be in the specified fractional
     part of the arc.
@param pEndOut OUT the (truncated) end point.
@param pTrueCenterOut OUT the computed center for the three input points.
@param pBestCenterOut OUT the computed center for the three output (truncated) points.
@param pStartIn IN the full-precision start point.
@param pMiddleIn IN the full precision along-arc point.
@param pEndIn IN the full precision end point.
@param coordinateFunction IN a function pointer.  This function may be called repeatedly with signature
            coordinateFunction (DPoint3d *pOut, DPoint3d const *pIn, void *pContext)
        to convert (truncate?) a full precision point pIn to a storage-format point pOut.
@param pContext IN caller's data context for truncation function.
@param f0 IN start fraction for search interval.   (Suggested value: 0.49, maybe as low as 0.4)
@param f1 IN end fraction for search interval.  (Suggested value: 0.51, maybe as high as 0.6)
@param maxTest IN number of mid points to test (Suggested value: 101 to 1001)
@param centerTol IN tolerance to trigger (desired) early termination.   If a center is found
        within this distance of the true center, the search is termianted.
* @bsimethod                                    Earlin.Lutz                     12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
Public void searchTruncatedCoordinateMidpointsToMatchCenter
(
DPoint3d *pStartOut,
DPoint3d *pMiddleOut,
DPoint3d *pEndOut,
DPoint3d *pTrueCenterOut,
DPoint3d *pBestCenterOut,
DPoint3d const *pStartIn,
DPoint3d const *pMiddleIn,
DPoint3d const *pEndIn,
void (*coordinateFixupFunction)(DPoint3d *pOut, DPoint3d const *pIn, void *pContext),
void *pContext,
double f0,
double f1,
int maxTest,
double centerTol
)
    {
    DPoint3d startTRUNC, currMiddleTRUNC, bestMiddleTRUNC, endTRUNC;
    DPoint3d currMiddle, bestCenter;
    double currDist, bestDist, baseDist;
    if (maxTest < 3)
        maxTest = 3;
    DEllipse3d baseEllipse;
    DEllipse3d currEllipse;
    // Truncate the base points
    coordinateFixupFunction (&startTRUNC, pStartIn, pContext);
    coordinateFixupFunction (&currMiddleTRUNC, pMiddleIn, pContext);
    coordinateFixupFunction (&endTRUNC, pEndIn, pContext);
    bestDist = 1.0e100;
    double bestFraction = 0.5;

    // Calculate the "true" center ...
    bsiDEllipse3d_initFrom3DPoint3dOnArc (&baseEllipse, pStartIn, pMiddleIn, pEndIn);
    // Truncate the full precision midpoint as first guess at the output midpoint:
    bsiDEllipse3d_fractionParameterToDPoint3d (&baseEllipse, &currMiddle, bestFraction);
    coordinateFixupFunction (&currMiddleTRUNC, &currMiddle, pContext);
    bsiDEllipse3d_initFrom3DPoint3dOnArc (&currEllipse, &startTRUNC, &currMiddleTRUNC, &endTRUNC);
    bestMiddleTRUNC = currMiddleTRUNC;
    bestDist = bsiDPoint3d_distance (&baseEllipse.center, &currEllipse.center);
    baseDist = bestDist;
    bestCenter = currEllipse.center;

    // Try other midpoint candidates ...
    double df = (f1 - f0) / (maxTest - 1);
    int numTest;
    for (numTest = 0; bestDist > centerTol && numTest <= maxTest; numTest++)
        {
        double f = f0 + numTest * df;
        bsiDEllipse3d_fractionParameterToDPoint3d (&baseEllipse, &currMiddle, f);
        // Calculate center for this set of truncated coordinates ...
        coordinateFixupFunction (&currMiddleTRUNC, &currMiddle, pContext);
        bsiDEllipse3d_initFrom3DPoint3dOnArc (&currEllipse, &startTRUNC, &currMiddleTRUNC, &endTRUNC);
        currDist = bsiDPoint3d_distance (&currEllipse.center, &baseEllipse.center);
        if (currDist < bestDist)
            {
            bestDist = currDist;
            bestCenter = currEllipse.center;
            bestMiddleTRUNC = currMiddleTRUNC;
            bestFraction = f;
            }
        }
    *pStartOut  = startTRUNC;
    *pMiddleOut = bestMiddleTRUNC;
    *pEndOut    = endTRUNC;
    *pTrueCenterOut = baseEllipse.center;
    *pBestCenterOut = bestCenter;

#ifdef DEBUG_PRINT
    DPoint3d baseCenterTRUNC;
    coordinateFixupFunction (&baseCenterTRUNC, &baseEllipse.center, pContext);

    showDouble ("centerShift", bsiDPoint3d_distance (&baseEllipse.center, &baseCenterTRUNC));
    showDouble ("startShift", bsiDPoint3d_distance (pStartIn, pStartOut));
    showDouble ("endShift", bsiDPoint3d_distance (pEndIn, pEndOut));
    showDouble ("baseDist", baseDist);
    showDouble ("bestDist", bestDist);
    showDouble ("ratio", bestDist / (baseDist == 0.0 ? 1.0 : baseDist));
    showDouble ("bestFraction", bestFraction);
    showDouble ("f0", f1);
    showDouble ("f1", f0);
    showDouble ("centerTol", centerTol);
    showInt ("maxTest", maxTest);
    showInt ("numTest", numTest);
#endif
    }
