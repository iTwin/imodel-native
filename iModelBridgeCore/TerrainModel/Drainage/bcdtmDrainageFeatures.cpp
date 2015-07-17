/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageFeatures.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcdtmDrainage.h"
#include <TerrainModel/Core/bcdtmInlines.h>

#ifdef WIP
// ToDo : Check for low low point is false low point
// Struct Pt1, Pt2, done?
// First add the edges around the point.
// Mark the point as done.
// go through the edges, mark pt2 as done. Loop round from the link to the edge.
// If any any point in the list is the higher (or lower, or in range) then add those links, it might just be a single link, there and back.
// after we have found all edges, remove the done links.
struct Edges
    {
    long pt1;
    long pt2;

    Edges (long pt1, long pt2) : pt1 (pt1), pt2 (pt2)
        {
        }
    };

int GetPointBoundary (BC_DTM_OBJ* dtmP, long ptNum, bool testLowPoint, unsigned char* markedPoints, double depth, DPoint3d& pt, bool& isPoint)
    {
    double testZ = pointAddrP (dtmP, ptNum)->z;
    double minZ = testZ - depth;
    double maxZ = testZ + depth;
    bvector<Edges> edges;

    pt = *pointAddrP (dtmP, ptNum);
    if (bcdtmFlag_testFlag (markedPoints, ptNum))
        {
        isPoint = false;
        return DTM_SUCCESS;
        }

    DTM_TIN_NODE* nodeP;
    long clPtr;
    bcdtmFlag_setFlag (markedPoints, ptNum);
    nodeP = nodeAddrP (dtmP, ptNum);
    if ((clPtr = nodeP->cPtr) != dtmP->nullPtr)
        {
        bool isDeep = true;
        bool isPondOrMound = true;
        long firstPnt = dtmP->nullPnt;
        long prevPnt = dtmP->nullPnt;
        while (clPtr != dtmP->nullPtr)
            {
            long pnt1 = clistAddrP (dtmP, clPtr)->pntNum;
            clPtr = clistAddrP (dtmP, clPtr)->nextPtr;

            double pntZ = pointAddrP (dtmP, pnt1)->z;

            if (testLowPoint)
                {
                if (pntZ < maxZ)
                    {
                    isDeep = false;
                    if (pntZ < testZ)
                        {
                        isPoint = false;
                        return DTM_SUCCESS;
                        }
                    }
                }
            else
                {
                if (pntZ > minZ)
                    {
                    isDeep = false;
                    if (pntZ > testZ)
                        {
                        isPoint = false;
                        return DTM_SUCCESS;
                        }
                    }
                }

            if (firstPnt == dtmP->nullPnt)
                firstPnt = prevPnt = pnt1;
            else
                {
                edges.push_back (Edges (prevPnt, pnt1));
                bcdtmFlag_setFlag (markedPoints, pnt1);
                prevPnt = pnt1;
                }

            if (clPtr == dtmP->nullPtr)
                {
                edges.push_back (Edges (prevPnt, firstPnt));
                bcdtmFlag_setFlag (markedPoints, firstPnt);
                }
            }

        if (isDeep)
            {
            isPoint = true;
            return DTM_SUCCESS;
            }
        }

    size_t edgeIndex = 0;

    for (edgeIndex = 0; edgeIndex < edges.size (); edgeIndex++)
        {
        long edgePt1 = edges[edgeIndex].pt1;
        long edgePt2 = edges[edgeIndex].pt2;
        long prevPt = edgePt1;
        long nextPt = bcdtmList_nextClkDtmObject (dtmP, edgePt2, edgePt1);
        double testZ = pointAddrP (dtmP, edgePt2)->z;

        // If this point it lower then the test point then this isnt a low point.
        if (testLowPoint)
            {
            if (testZ < pt.z)
                pt = *pointAddrP (dtmP, edgePt2);
            }
        else
            {
            if (testZ > pt.z)
                pt = *pointAddrP (dtmP, edgePt2);
            }
        bool usePrevPoint = true;

        while (nextPt != edgePt1)
            {
            bool pointOK = true;
            if (!bcdtmFlag_testFlag (markedPoints, nextPt))
                {
                double pntZ = pointAddrP (dtmP, prevPt)->z;

                if (testLowPoint)
                    pointOK = (pntZ >= testZ);
                else
                    pointOK = (pntZ <= testZ);
                }
            else
                {
                //pointOK
                //prevPt = nextPt;
                //nextPt = bcdtmList_nextClkDtmObject (dtmP, edgePt2, prevPt);
                //continue;
                }

            if (!pointOK)
                {
                bool exitUnderDepth = false;
                if (testLowPoint)
                    exitUnderDepth = testZ < maxZ;
                else
                    exitUnderDepth = testZ > maxZ;

                if (exitUnderDepth)
                    {
                    isPoint = false;
                    return DTM_SUCCESS;
                    }
                }
            if (pointOK)
                {
                if (usePrevPoint)
                    edges.push_back (Edges (edgePt2, nextPt));
                else
                    edges.push_back (Edges (prevPt, nextPt));
                bcdtmFlag_setFlag (markedPoints, nextPt);
                usePrevPoint = true;
                }
            else
                {
                if (usePrevPoint)
                    {
                    edges.push_back (Edges (prevPt, edgePt2));
                    usePrevPoint = false;
                    }
                }
            prevPt = nextPt;
            nextPt = bcdtmList_nextClkDtmObject (dtmP, edgePt2, prevPt);
            }
        }
    isPoint = true;
    return DTM_SUCCESS;
    }

#endif
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_returnLowPointsDtmObject
(
BC_DTM_OBJ         *dtmP,                 // ==> Pointer To Dtm Object  
DTMFeatureCallback loadFunctionP,         // ==> Pointer To Load Function
bool               useFence,              // ==> Load Feature Within Fence
DTMFenceType       fenceType,             // ==> Rectangular or Irregular Shape
DTMFenceOption     fenceOption,           // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
const DPoint3d     *fencePtsP,            // ==> DPoint3d Array Of Fence Points
int                numFencePts,           // ==> Number Of Fence Points
void               *userP,                // <==> User Pointer Passed Back To User
int&               numLowPts              // <== Number Of Low Points Found
)
    {
    return bcdtmDrainage_returnNoneFalseLowPointsDtmObject (dtmP, 0, loadFunctionP, useFence, fenceType, fenceOption, fencePtsP, numFencePts, userP, numLowPts);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_returnNoneFalseLowPointsDtmObject
(
BC_DTM_OBJ         *dtmP,                 // ==> Pointer To Dtm Object  
double             falseLowDepth,         // ==> Low Point Must Be Deeper Than This Value
DTMFeatureCallback loadFunctionP,         // ==> Pointer To Load Function
bool               useFence,              // ==> Load Feature Within Fence
DTMFenceType       fenceType,             // ==> Rectangular or Irregular Shape
DTMFenceOption     fenceOption,           // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
const DPoint3d     *fencePtsP,            // ==> DPoint3d Array Of Fence Points
int                numFencePts,           // ==> Number Of Fence Points
void               *userP,                // <==> User Pointer Passed Back To User
int&               numLowPts              // <== Number Of Low Points Found
)
    {
    int           ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long          pnt, pnt1, pnt2, pnt3, clPtr, fndPnt, startPnt, lastPnt, cacheSize = 1000;
    const DTMFeatureType dtmFeatureType = DTMFeatureType::LowPoint;
    bool          lowPointFound = true;
    BC_DTM_OBJ    *clipDtmP = NULL;
    DTM_TIN_NODE  *nodeP;
    DTMPointCache pointCache;
    std::vector<long> zeroPts;
    unsigned char* pointDone = nullptr;

    // Log Input Arguments

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Returning low Points");
        bcdtmWrite_message (0, 0, 0, "dtmP           = %p", dtmP);
        bcdtmWrite_message (0, 0, 0, "loadFunctionP  = %p", loadFunctionP);
        bcdtmWrite_message (0, 0, 0, "useFence       = %8d", useFence);
        bcdtmWrite_message (0, 0, 0, "fenceType      = %8d", fenceType);
        bcdtmWrite_message (0, 0, 0, "fenceOption    = %8d", fenceOption);
        bcdtmWrite_message (0, 0, 0, "fencePtsP      = %p", fencePtsP);
        bcdtmWrite_message (0, 0, 0, "numFencePts    = %8d", numFencePts);
        bcdtmWrite_message (0, 0, 0, "userP          = %p", userP);
        }

    // Validate Fence 

    numLowPts = 0;
    if (fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap) fenceOption = DTMFenceOption::Inside;
    if (useFence && (fencePtsP == NULL || numFencePts <= 2)) useFence = false;
    if (useFence && (fencePtsP->x != (fencePtsP + numFencePts - 1)->x || fencePtsP->y != (fencePtsP + numFencePts - 1)->y)) useFence = false;

    // Test For Valid DTM Object

    if (bcdtmObject_testForValidDtmObject (dtmP)) goto errexit;

    // Test For DTM Object In Tin State
    if (dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message (2, 0, 0, "Method Requires Triangulated DTM");
        goto errexit;
        }

    // Check Load Function Set

    if (loadFunctionP == NULL)
        {
        bcdtmWrite_message (2, 0, 0, "Call Back Function Not Set");
        goto errexit;
        }

    // Set Point Scan Range
    startPnt = 0;
    lastPnt = dtmP->numPoints - 1;

    // Build Clipping Tin For Fence
    if (useFence)
        {
        if (bcdtmClip_buildClippingTinFromFencePointsDtmObject (&clipDtmP, fencePtsP, numFencePts)) goto errexit;
        if (fenceOption == DTMFenceOption::Inside || fenceOption == DTMFenceOption::Overlap)
            {
            bcdtmFind_binaryScanDtmObject (dtmP, clipDtmP->xMin, &startPnt);
            while (startPnt > 0 && pointAddrP (dtmP, startPnt)->x >= clipDtmP->xMin) --startPnt;
            if (pointAddrP (dtmP, startPnt)->x < clipDtmP->xMin) ++startPnt;
            bcdtmFind_binaryScanDtmObject (dtmP, clipDtmP->xMax, &lastPnt);
            while (lastPnt < dtmP->numPoints - 1 && pointAddrP (dtmP, lastPnt)->x <= clipDtmP->xMin) ++lastPnt;
            }
        }

    //  Log Start And Endpoints
    if (dbg) bcdtmWrite_message (0, 0, 0, "startPnt = %8d lastPnt = %8d", startPnt, lastPnt);

    // Scan Tin Points And Accumulate low Points
    pointDone = (unsigned char *)calloc (dtmP->numPoints, sizeof(char));

    for (pnt = startPnt; pnt <= lastPnt; ++pnt)
        {
        if (!bcdtmFlag_testFlag (pointDone, pnt))
            {

#ifdef WIP
            if (true)
                {
                DPoint3d pt;
                GetPointBoundary (dtmP, pnt, true, pointDone, falseLowDepth, pt, lowPointFound);
                ++numLowPts;
                if (pointCache.StorePointInCache (pointAddrP (dtmP, pnt)->x, pointAddrP (dtmP, pnt)->y, pointAddrP (dtmP, pnt)->z)) goto errexit;
                if (pointCache.SizeOfCache () >= cacheSize)
                    {
                    if (pointCache.CallUserDelegateWithCachePoints (loadFunctionP, (DTMFeatureType)dtmFeatureType, DTM_NULL_USER_TAG, DTM_NULL_FEATURE_ID, userP) != DTM_SUCCESS) goto errexit;
                    }
                continue;
                }
#endif
            //  Test For Valid Point
            lowPointFound = true;
            bool deepLowPoint = true;

            zeroPts.clear();

            zeroPts.push_back (pnt);
            long zeroPtIndex = 0;
            //     Test For low Point
            double testZ = pointAddrP (dtmP, pnt)->z;
            double testFalseZ = testZ + falseLowDepth;

            bcdtmFlag_setFlag (pointDone, pnt);
            while (lowPointFound && !zeroPts.empty ())
                {
                long tpnt = zeroPts.back ();
                zeroPts.pop_back ();

                nodeP = nodeAddrP (dtmP, tpnt);
                if ((clPtr = nodeP->cPtr) != dtmP->nullPtr && nodeP->hPtr == dtmP->nullPnt)
                    {
                    while (clPtr != dtmP->nullPtr && lowPointFound)
                        {
                        pnt1 = clistAddrP (dtmP, clPtr)->pntNum;
                        clPtr = clistAddrP (dtmP, clPtr)->nextPtr;

                        double pntZ = pointAddrP (dtmP, pnt1)->z;
                        if (pntZ < testZ)
                            {
                            lowPointFound = false;
                            break;
                            }
                        if (pntZ == testZ)
                            {
                            if (!bcdtmFlag_testFlag (pointDone, pnt1))
                                {
                                bcdtmFlag_setFlag (pointDone, pnt1);
                                zeroPts.push_back (pnt1);
                                }
                            }
                        else if (pntZ < testFalseZ)
                            deepLowPoint = false;
                        }
                    }
                else
                    lowPointFound = false;
                zeroPtIndex++;
                }
            if (lowPointFound)
                {
                // Check for void.
                }
            // low Point Found
            if (lowPointFound)
                {
                // Test Point For Fence Option
                if (useFence)
                    {
                    if (fenceOption == DTMFenceOption::Inside || fenceOption == DTMFenceOption::Overlap)
                        {
                        if (pointAddrP (dtmP, pnt)->x < clipDtmP->xMin || pointAddrP (dtmP, pnt)->x > clipDtmP->xMax ||
                            pointAddrP (dtmP, pnt)->y < clipDtmP->yMin || pointAddrP (dtmP, pnt)->y > clipDtmP->yMax) lowPointFound = false;
                        else if (fenceType == DTMFenceType::Shape)
                            {
                            if (bcdtmFind_triangleDtmObject (clipDtmP, pointAddrP (dtmP, pnt)->x, pointAddrP (dtmP, pnt)->y, &fndPnt, &pnt1, &pnt2, &pnt3)) goto errexit;
                            if (!fndPnt) lowPointFound = false;
                            }
                        }
                    if (fenceOption == DTMFenceOption::Outside)
                        {
                        lowPointFound = false;
                        if (pointAddrP (dtmP, pnt)->x < clipDtmP->xMin || pointAddrP (dtmP, pnt)->x > clipDtmP->xMax ||
                            pointAddrP (dtmP, pnt)->y < clipDtmP->yMin || pointAddrP (dtmP, pnt)->y > clipDtmP->yMax) lowPointFound = true;
                        else if (fenceType == DTMFenceType::Shape)
                            {
                            if (bcdtmFind_triangleDtmObject (clipDtmP, pointAddrP (dtmP, pnt)->x, pointAddrP (dtmP, pnt)->y, &fndPnt, &pnt1, &pnt2, &pnt3)) goto errexit;
                            if (!fndPnt) lowPointFound = true;
                            }
                        }
                    }

#ifdef WIP
                // If we have a falseLowDepth check if this isn't a false low.
                if (lowPointFound && falseLowDepth != 0 && !deepLowPoint)
                    {
                    DPoint3d pt = *pointAddrP (dtmP, pnt);
                    bool pondDetermined;
                    double depth;
                    int status = bcdtmDrainage_calculatePondForLowPointDtmObject (dtmP, pt.x, pt.y, 0, loadFunctionP, false, &pondDetermined, nullptr, &depth, nullptr, nullptr, &pt, nullptr);
                    lowPointFound = pondDetermined && (pointAddrP (dtmP, pnt)->z == pt.z) && depth >= falseLowDepth;
                    }
#else
                lowPointFound &= (falseLowDepth == 0) || deepLowPoint;
#endif
                //       Store Point In Cache
                if (lowPointFound)
                    {
                    ++numLowPts;
                    if (pointCache.StorePointInCache (pointAddrP (dtmP, pnt)->x, pointAddrP (dtmP, pnt)->y, pointAddrP (dtmP, pnt)->z)) goto errexit;
                    if (pointCache.SizeOfCache () >= cacheSize)
                        {
                        if (pointCache.CallUserDelegateWithCachePoints (loadFunctionP, (DTMFeatureType)dtmFeatureType, DTM_NULL_USER_TAG, DTM_NULL_FEATURE_ID, userP) != DTM_SUCCESS) goto errexit;
                        }
                    }
                }
            }
        }
    //  Check For Remaining Cache Points

    if (pointCache.SizeOfCache () > 0)
        {
        if (pointCache.CallUserDelegateWithCachePoints (loadFunctionP, (DTMFeatureType)dtmFeatureType, DTM_NULL_USER_TAG, DTM_NULL_FEATURE_ID, userP) != DTM_SUCCESS) goto errexit;
        }

    // Log Number Of low Points Returned

    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of low Points = %8d", numLowPts);

    // Clean Up

cleanup:
    if (clipDtmP != NULL) bcdtmObject_destroyDtmObject (&clipDtmP);

    // Return

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Returning low Points Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Returning low Points Error");
    return(ret);

    // Error Exit

errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_returnHighPointsDtmObject
(
BC_DTM_OBJ         *dtmP,                 // ==> Pointer To Dtm Object  
DTMFeatureCallback loadFunctionP,         // ==> Pointer To Load Function
bool               useFence,              // ==> Load Feature Within Fence
DTMFenceType       fenceType,             // ==> Rectangular or Irregular Shape
DTMFenceOption    fenceOption,           // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
const DPoint3d     *fencePtsP,            // ==> DPoint3d Array Of Fence Points
int                numFencePts,           // ==> Number Of Fence Points
void               *userP,                // <==> User Pointer Passed Back To User
int&               numHighPts             // <== Number Of High Points Found
)
    {
    int           ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long          pnt, pnt1, pnt2, pnt3, clPtr, fndPnt, startPnt, lastPnt, cacheSize = 1000;
    const DTMFeatureType dtmFeatureType = DTMFeatureType::HighPoint;
    bool          highPointFound = true;
    BC_DTM_OBJ    *clipDtmP = NULL;
    DTM_TIN_NODE  *nodeP;
    DTMPointCache pointCache;
    std::stack<long> zeroPts;
    unsigned char* pointDone = nullptr;

    // Log Input Arguments

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Returning High Points");
        bcdtmWrite_message (0, 0, 0, "dtmP           = %p", dtmP);
        bcdtmWrite_message (0, 0, 0, "loadFunctionP  = %p", loadFunctionP);
        bcdtmWrite_message (0, 0, 0, "useFence       = %8d", useFence);
        bcdtmWrite_message (0, 0, 0, "fenceType      = %8d", fenceType);
        bcdtmWrite_message (0, 0, 0, "fenceOption    = %8d", fenceOption);
        bcdtmWrite_message (0, 0, 0, "fencePtsP      = %p", fencePtsP);
        bcdtmWrite_message (0, 0, 0, "numFencePts    = %8d", numFencePts);
        bcdtmWrite_message (0, 0, 0, "userP          = %p", userP);
        }

    // Validate Fence 

    numHighPts = 0;
    if (fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap) fenceOption = DTMFenceOption::Inside;
    if (useFence && (fencePtsP == NULL || numFencePts <= 2)) useFence = false;
    if (useFence && (fencePtsP->x != (fencePtsP + numFencePts - 1)->x || fencePtsP->y != (fencePtsP + numFencePts - 1)->y)) useFence = false;

    // Test For Valid DTM Object

    if (bcdtmObject_testForValidDtmObject (dtmP)) goto errexit;

    // Test For DTM Object In Tin State

    if (dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message (2, 0, 0, "Method Requires Triangulated DTM");
        goto errexit;
        }

    // Check Load Function Set

    if (loadFunctionP == NULL)
        {
        bcdtmWrite_message (2, 0, 0, "Call Back Function Not Set");
        goto errexit;
        }

    // Set Point Scan Range

    startPnt = 0;
    lastPnt = dtmP->numPoints - 1;

    // Build Clipping Tin For Fence

    if (useFence)
        {
        if (bcdtmClip_buildClippingTinFromFencePointsDtmObject (&clipDtmP, fencePtsP, numFencePts)) goto errexit;
        if (fenceOption == DTMFenceOption::Inside || fenceOption == DTMFenceOption::Overlap)
            {
            bcdtmFind_binaryScanDtmObject (dtmP, clipDtmP->xMin, &startPnt);
            while (startPnt > 0 && pointAddrP (dtmP, startPnt)->x >= clipDtmP->xMin) --startPnt;
            if (pointAddrP (dtmP, startPnt)->x < clipDtmP->xMin) ++startPnt;
            bcdtmFind_binaryScanDtmObject (dtmP, clipDtmP->xMax, &lastPnt);
            while (lastPnt < dtmP->numPoints - 1 && pointAddrP (dtmP, lastPnt)->x <= clipDtmP->xMin) ++lastPnt;
            }
        }

    //  Log Start And Endpoints

    if (dbg) bcdtmWrite_message (0, 0, 0, "startPnt = %8d lastPnt = %8d", startPnt, lastPnt);

    // Scan Tin Points And Accumulate High Points
    pointDone = (unsigned char *)calloc (dtmP->numPoints, sizeof(char));

    for (pnt = startPnt; pnt <= lastPnt; ++pnt)
        {
        if (!bcdtmFlag_testFlag (pointDone, pnt))
            {
            //  Test For Valid Point
            highPointFound = true;

            if (!zeroPts.empty ())
                {
                zeroPts = std::stack<long> ();
                }
            zeroPts.push (pnt);
            long zeroPtIndex = 0;
            //     Test For High Point
            double testZ = pointAddrP (dtmP, pnt)->z;

            bcdtmFlag_setFlag (pointDone, pnt);
            while (highPointFound && !zeroPts.empty())
                {
                long tpnt = zeroPts.top ();
                zeroPts.pop ();

                nodeP = nodeAddrP (dtmP, tpnt);
                if ((clPtr = nodeP->cPtr) != dtmP->nullPtr && nodeP->hPtr == dtmP->nullPnt)
                    {
                    while (clPtr != dtmP->nullPtr && highPointFound)
                        {
                        pnt1 = clistAddrP (dtmP, clPtr)->pntNum;
                        clPtr = clistAddrP (dtmP, clPtr)->nextPtr;

                        double pntZ = pointAddrP (dtmP, pnt1)->z;
                        if (pntZ > testZ)
                            {
                            highPointFound = false;
                            break;
                            }
                        if (pntZ == testZ)
                            {
                            if (!bcdtmFlag_testFlag (pointDone, pnt1))
                                {
                                bcdtmFlag_setFlag (pointDone, pnt1);
                                zeroPts.push (pnt1);
                                }
                            }
                        }
                    }
                else
                    highPointFound = false;
                zeroPtIndex++;
                }

            if (highPointFound)
                {
                // Check for void.
                }
            // High Point Found
            if (highPointFound)
                {

                //        Test Point For Fence Option

                if (useFence)
                    {
                    if (fenceOption == DTMFenceOption::Inside || fenceOption == DTMFenceOption::Overlap)
                        {
                        if (pointAddrP (dtmP, pnt)->x < clipDtmP->xMin || pointAddrP (dtmP, pnt)->x > clipDtmP->xMax ||
                            pointAddrP (dtmP, pnt)->y < clipDtmP->yMin || pointAddrP (dtmP, pnt)->y > clipDtmP->yMax) highPointFound = false;
                        else if (fenceType == DTMFenceType::Shape)
                            {
                            if (bcdtmFind_triangleDtmObject (clipDtmP, pointAddrP (dtmP, pnt)->x, pointAddrP (dtmP, pnt)->y, &fndPnt, &pnt1, &pnt2, &pnt3)) goto errexit;
                            if (!fndPnt) highPointFound = false;
                            }
                        }
                    if (fenceOption == DTMFenceOption::Outside)
                        {
                        highPointFound = false;
                        if (pointAddrP (dtmP, pnt)->x < clipDtmP->xMin || pointAddrP (dtmP, pnt)->x > clipDtmP->xMax ||
                            pointAddrP (dtmP, pnt)->y < clipDtmP->yMin || pointAddrP (dtmP, pnt)->y > clipDtmP->yMax) highPointFound = true;
                        else if (fenceType == DTMFenceType::Shape)
                            {
                            if (bcdtmFind_triangleDtmObject (clipDtmP, pointAddrP (dtmP, pnt)->x, pointAddrP (dtmP, pnt)->y, &fndPnt, &pnt1, &pnt2, &pnt3)) goto errexit;
                            if (!fndPnt) highPointFound = true;
                            }
                        }
                    }

                //       Store Point In Cache

                if (highPointFound)
                    {
                    ++numHighPts;
                    if (pointCache.StorePointInCache (pointAddrP (dtmP, pnt)->x, pointAddrP (dtmP, pnt)->y, pointAddrP (dtmP, pnt)->z)) goto errexit;
                    if (pointCache.SizeOfCache () >= cacheSize)
                        {
                        if (pointCache.CallUserDelegateWithCachePoints (loadFunctionP, (DTMFeatureType)dtmFeatureType, DTM_NULL_USER_TAG, DTM_NULL_FEATURE_ID, userP) != DTM_SUCCESS) goto errexit;
                        }
                    }
                }
            }
        }
    //  Check For Remaining Cache Points

    if (pointCache.SizeOfCache () > 0)
        {
        if (pointCache.CallUserDelegateWithCachePoints (loadFunctionP, (DTMFeatureType)dtmFeatureType, DTM_NULL_USER_TAG, DTM_NULL_FEATURE_ID, userP) != DTM_SUCCESS) goto errexit;
        }

    // Log Number Of High Points Returned

    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of High Points = %8d", numHighPts);

    // Clean Up

cleanup:
    if (clipDtmP != NULL) bcdtmObject_destroyDtmObject (&clipDtmP);

    // Return

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Returning High Points Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Returning High Points Error");
    return(ret);

    // Error Exit

errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_returnSumpLinesDtmObject
(
 BC_DTM_OBJ         *dtmP,                    // ==> Pointer To Dtm Object 
 DTMFeatureCallback loadFunctionP,            // ==> Pointer To Load Function
 bool               useFence,                 // ==> Load Feature Within Fence
 DTMFenceType       fenceType,                // ==> Type Of Fence Reactangular Or Shape
 DTMFenceOption     fenceOption,              // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
 const DPoint3d     *fencePtsP,               // ==> DPoint3d Array Of Fence Points 
 int                numFencePts,              // ==> Number Of Fence Points
 void               *userP,                   // ==> User Pointer Passed Back To User
 int&               numSumpLines              // <== Number Of Sump Lines Found 
)
{
 int               ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 int               cacheSize=10000 ;
 long              pnt,startPnt,lastPnt,smpPnt,clkPnt,antPnt,clPtr,voidLine ;
 DTMFeatureType lineType;
 DTMFenceOption  sumpExtent;
 long               sumpLineFound;
 DTMFeatureType  dtmFeatureType = DTMFeatureType::SumpLine;
 double            xSumpMin,xSumpMax,ySumpMin,ySumpMax ;
 bool              voidsInDtm=false ;
 DPoint3d          sumpPts[2] ;
 BC_DTM_OBJ        *clipDtmP=NULL ;
 DTM_TIN_NODE      *nodeP ;
 DTMLineCache      lineCache ; 
 DTMDrainageTables *drainageTablesP=nullptr ;

// Log Entry Arguments

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Returning Sump Lines") ; 
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
   }

// Validate Fence 

 numSumpLines = 0 ;
 if (fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap) fenceOption = DTMFenceOption::Inside;
 if (useFence && (fencePtsP == NULL || numFencePts <= 2)) useFence = false;
 if( useFence && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = false ;

// Test For Valid DTM Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;

// Test For DTM Object In Tin State

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ; 
   } 

// Check Load Function Set

 if( loadFunctionP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Call Back Function Not Set") ;
    goto errexit ;
   }

// Check For Voids

 voidsInDtm = bcdtmDrainageList_checkForVoidsInDtmObject(dtmP) ;

// Set Point Scan Range

 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;

// Build Clipping Tin For Fence

 if( useFence ) 
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
/*
    if( fenceOption == DTMFenceOption::Inside || fenceOption == DTMFenceOption::Overlap )
      {
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
       while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
       if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
       while( lastPnt < dtmP->numPoints - 1 && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
      }
*/
   }

// Scan Tin Points And Test For Sump Lines

 for( pnt = startPnt ; pnt <= lastPnt ; ++pnt )
   {
    nodeP = nodeAddrP(dtmP,pnt) ;

//  Test For Valid Point

    if( ( clPtr = nodeP->cPtr ) != dtmP->nullPtr )
      {
       if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while ( clPtr != dtmP->nullPtr )
         {
          smpPnt  = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr   = clistAddrP(dtmP,clPtr)->nextPtr ;
          if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,smpPnt)) < 0 ) goto errexit ;

//        Check For Valid Tin Line

          if( smpPnt > pnt )
            {

//           Check For Void Line

             voidLine = FALSE ; 
             if( voidsInDtm )
               {
                if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt,smpPnt,&voidLine)) goto errexit ;
               }

//           Only Process For None Void Lines

             if( ! voidLine )
               {  

//              Test Tin Line For Fence Option Against Bounding Rectangle

                sumpLineFound = true ;
                if( useFence )
                  {

//                 Get Bounding Rectangle For Sump Line

                   xSumpMin =  xSumpMax = pointAddrP(dtmP,pnt)->x ;
                   ySumpMin =  ySumpMax = pointAddrP(dtmP,pnt)->y ;
                   if( pointAddrP(dtmP,smpPnt)->x < xSumpMin ) xSumpMin = pointAddrP(dtmP,smpPnt)->x ;
                   if( pointAddrP(dtmP,smpPnt)->x > xSumpMax ) xSumpMax = pointAddrP(dtmP,smpPnt)->x ;
                   if( pointAddrP(dtmP,smpPnt)->y < ySumpMin ) ySumpMin = pointAddrP(dtmP,smpPnt)->y ;
                   if( pointAddrP(dtmP,smpPnt)->y > ySumpMax ) ySumpMax = pointAddrP(dtmP,smpPnt)->y ;

//                 Check Fence Option Against Bounding Rectangle

                   if( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )
                     { 
                      if( xSumpMax < clipDtmP->xMin ||  xSumpMin > clipDtmP->xMax || ySumpMax < clipDtmP->yMin ||  ySumpMin > clipDtmP->yMax ) sumpLineFound = true ;
                     }
                   if( fenceOption == DTMFenceOption::Outside )
                     { 
                      sumpLineFound = false ;
                      if( xSumpMax < clipDtmP->xMin ||  xSumpMin > clipDtmP->xMax || ySumpMax < clipDtmP->yMin ||  ySumpMin > clipDtmP->yMax ) sumpLineFound = true  ;
                     }
                  }

//              Process Further If Fence Option Staifies Bounding Rectangle

                if( sumpLineFound )
                  {

//                 Check For Sump Line 

                   if( bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP,drainageTablesP,pnt,smpPnt,antPnt,clkPnt,&lineType)) goto errexit ;
                   if( lineType == DTMFeatureType::SumpLine )
                     {

//                    Determine Sump Line Extent With Fence

                      if( useFence )
                        {
                         sumpPts[0].x = pointAddrP(dtmP,pnt)->x    ; sumpPts[0].y = pointAddrP(dtmP,pnt)->y    ;  
                         sumpPts[1].x = pointAddrP(dtmP,smpPnt)->x ; sumpPts[1].y = pointAddrP(dtmP,smpPnt)->y ;  
                         if( bcdtmClip_determineFeatureExtentWithFenceDtmObject(clipDtmP,sumpPts,2,&sumpExtent)) goto errexit ;
                         if( fenceOption != sumpExtent ) sumpLineFound = false ;
                        } 
 
//                    Store Sump Line In Cache
         
                      if( sumpLineFound ) 
                        {
                         ++numSumpLines ; 
                         if( lineCache.StoreLineInCache(pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z,pointAddrP(dtmP,smpPnt)->x,pointAddrP(dtmP,smpPnt)->y,pointAddrP(dtmP,smpPnt)->z)) goto errexit ;
                         if( lineCache.SizeOfCache() >= cacheSize )
                           {
                            if( lineCache.CallUserDelegateWithCacheLines(loadFunctionP,(DTMFeatureType)dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,userP) != DTM_SUCCESS ) goto errexit ;
                           }
                        }
                     }
                  }
               }
            }
          antPnt = smpPnt ; 
         }
      }
   }

//  Check For Remaining Cache Lines

 if( lineCache.SizeOfCache() > 0 )
   {
    if( lineCache.CallUserDelegateWithCacheLines(loadFunctionP,(DTMFeatureType)dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,userP) != DTM_SUCCESS ) goto errexit ;
   }

//  Log Number Of Sump Lines Returned

 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Sump Lines = %8d",numSumpLines) ;

// Clean Up

 cleanup :
 if( clipDtmP  != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Returning Sump Lines Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Returning Sump Lines Error") ; 
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_returnZeroSlopeSumpLinesDtmObject
(
 BC_DTM_OBJ         *dtmP,                    // ==> Pointer To Dtm Object 
 DTMFeatureCallback loadFunctionP,            // ==> Pointer To Load Function
 bool               useFence,                 // ==> Load Feature Within Fence
 DTMFenceType       fenceType,                // ==> Type Of Fence Reactangular Or Shape
 DTMFenceOption     fenceOption,              // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
 const DPoint3d     *fencePtsP,               // ==> DPoint3d Array Of Fence Points 
 int                numFencePts,              // ==> Number Of Fence Points
 void               *userP,                   // ==> User Pointer Passed Back To User
 int&               numSumpLines              // <== Number Of Sump Lines Found 
)
{
 int               ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 int               cacheSize=10000 ;
 long              pnt, startPnt, lastPnt, smpPnt, clkPnt, antPnt, clPtr;
 DTMFenceOption  sumpExtent;
 long              sumpLineFound,voidLine ;
 DTMFeatureType dtmFeatureType = DTMFeatureType::SumpLine, lineType;
 bool              voidsInDtm=false ;
 double            xSumpMin,xSumpMax,ySumpMin,ySumpMax,pntZ ;
 DPoint3d          sumpPts[2] ;
 BC_DTM_OBJ        *clipDtmP=NULL ;
 DTM_TIN_NODE      *nodeP ;
 DTMLineCache      lineCache ; 
 DTMDrainageTables *drainageTablesP=nullptr ;

// Log Entry Arguments

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Returning Sump Lines") ; 
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
   }

// Validate Fence 

 numSumpLines = 0 ;
 if (fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap) fenceOption = DTMFenceOption::Inside;
 if (useFence && (fencePtsP == NULL || numFencePts <= 2)) useFence = false;
 if( useFence && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = false ;

// Test For Valid DTM Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;

// Test For DTM Object In Tin State

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ; 
   } 

// Check Load Function Set

 if( loadFunctionP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Call Back Function Not Set") ;
    goto errexit ;
   }

// Check For Voids

 voidsInDtm = bcdtmDrainageList_checkForVoidsInDtmObject(dtmP) ;

// Set Point Scan Range

 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;

// Build Clipping Tin For Fence

 if( useFence ) 
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
/*
    if( fenceOption == DTMFenceOption::Inside || fenceOption == DTMFenceOption::Overlap )
      {
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
       while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
       if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
       while( lastPnt < dtmP->numPoints - 1 && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
      }
*/
   }

// Scan Tin Points And Test For Sump Lines

 for( pnt = startPnt ; pnt <= lastPnt ; ++pnt )
   {
    pntZ  = pointAddrP(dtmP,pnt)->z ;
    nodeP = nodeAddrP(dtmP,pnt) ;

//  Test For Valid Point

    if( ( clPtr = nodeP->cPtr ) != dtmP->nullPtr )
      {
       if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while ( clPtr != dtmP->nullPtr )
         {
          smpPnt  = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr   = clistAddrP(dtmP,clPtr)->nextPtr ;
          if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,smpPnt)) < 0 ) goto errexit ;

//        Check For Valid Tin Line

          if( smpPnt > pnt )
            {

//           Check For Zero Slope Sump Line

             if( pointAddrP(dtmP,smpPnt)->z == pntZ )
               { 

//              Check For Void Line

                voidLine = FALSE ; 
                if( voidsInDtm )
                  {
                   if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt,smpPnt,&voidLine)) goto errexit ;
                  }

//              Only Process For None Void Lines

                if( ! voidLine )
                  {  

//                 Test Tin Line For Fence Option Against Bounding Rectangle

                   sumpLineFound = true ;
                   if( useFence )
                     {

//                    Get Bounding Rectangle For Sump Line

                      xSumpMin =  xSumpMax = pointAddrP(dtmP,pnt)->x ;
                      ySumpMin =  ySumpMax = pointAddrP(dtmP,pnt)->y ;
                      if( pointAddrP(dtmP,smpPnt)->x < xSumpMin ) xSumpMin = pointAddrP(dtmP,smpPnt)->x ;
                      if( pointAddrP(dtmP,smpPnt)->x > xSumpMax ) xSumpMax = pointAddrP(dtmP,smpPnt)->x ;
                      if( pointAddrP(dtmP,smpPnt)->y < ySumpMin ) ySumpMin = pointAddrP(dtmP,smpPnt)->y ;
                      if( pointAddrP(dtmP,smpPnt)->y > ySumpMax ) ySumpMax = pointAddrP(dtmP,smpPnt)->y ;

//                    Check Fence Option Against Bounding Rectangle

                      if( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )
                        { 
                         if( xSumpMax < clipDtmP->xMin ||  xSumpMin > clipDtmP->xMax || ySumpMax < clipDtmP->yMin ||  ySumpMin > clipDtmP->yMax ) sumpLineFound = true ;
                        }
                      if( fenceOption == DTMFenceOption::Outside )
                        { 
                         sumpLineFound = false ;
                         if( xSumpMax < clipDtmP->xMin ||  xSumpMin > clipDtmP->xMax || ySumpMax < clipDtmP->yMin ||  ySumpMin > clipDtmP->yMax ) sumpLineFound = true  ;
                        }
                     }

//                 Process Further If Fence Option Staifies Bounding Rectangle

                   if( sumpLineFound )
                     {

//                    Check For Sump Line 

                      if( bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP,drainageTablesP,pnt,smpPnt,antPnt,clkPnt,&lineType)) goto errexit ;
                      if( lineType == DTMFeatureType::SumpLine )
                        {

//                       Determine Sump Line Extent With Fence

                         if( useFence )
                           {
                            sumpPts[0].x = pointAddrP(dtmP,pnt)->x    ; sumpPts[0].y = pointAddrP(dtmP,pnt)->y    ;  
                            sumpPts[1].x = pointAddrP(dtmP,smpPnt)->x ; sumpPts[1].y = pointAddrP(dtmP,smpPnt)->y ;  
                            if( bcdtmClip_determineFeatureExtentWithFenceDtmObject(clipDtmP,sumpPts,2,&sumpExtent)) goto errexit ;
                            if( fenceOption != sumpExtent ) sumpLineFound = false ;
                           } 
 
//                       Store Sump Line In Cache
         
                         if( sumpLineFound ) 
                           {
                            ++numSumpLines ; 
                            if( lineCache.StoreLineInCache(pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z,pointAddrP(dtmP,smpPnt)->x,pointAddrP(dtmP,smpPnt)->y,pointAddrP(dtmP,smpPnt)->z)) goto errexit ;
                            if( lineCache.SizeOfCache() >= cacheSize )
                              {
                               if( lineCache.CallUserDelegateWithCacheLines(loadFunctionP,(DTMFeatureType)dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,userP) != DTM_SUCCESS ) goto errexit ;
                              }
                           }
                        }
                     }
                  } 
               }
            }
          antPnt = smpPnt ; 
         }
      }
   }

//  Check For Remaining Cache Lines

 if( lineCache.SizeOfCache() > 0 )
   {
    if( lineCache.CallUserDelegateWithCacheLines(loadFunctionP,(DTMFeatureType)dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,userP) != DTM_SUCCESS ) goto errexit ;
   }

//  Log Number Of Sump Lines Returned

 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Sump Lines = %8d",numSumpLines) ;

// Clean Up

 cleanup :
 if( clipDtmP  != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Returning Sump Lines Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Returning Sump Lines Error") ; 
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_returnRidgeLinesDtmObject
(
 BC_DTM_OBJ         *dtmP,                    // ==> Pointer To Dtm Object 
 DTMFeatureCallback loadFunctionP,            // ==> Pointer To Load Function
 bool               useFence,                 // ==> Load Feature Within Fence
 DTMFenceType    fenceType,                // ==> Type Of Fence Reactangular Or Shape
 DTMFenceOption    fenceOption,              // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
 const DPoint3d     *fencePtsP,               // ==> DPoint3d Array Of Fence Points 
 int                numFencePts,              // ==> Number Of Fence Points
 void               *userP,                   // ==> User Pointer Passed Back To User
 int&               numRidgeLines             // <== Number Of Ridge Lines Found 
)
{
 int               ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 int               cacheSize=10000 ;
 long              pnt,startPnt,lastPnt,rdgPnt,clkPnt,antPnt,clPtr,voidLine ;
 DTMFenceOption    ridgeExtent;
 long              ridgeLineFound;
 DTMFeatureType  dtmFeatureType = DTMFeatureType::RidgeLine, lineType;
 double            xRidgeMin,xRidgeMax,yRidgeMin,yRidgeMax ;
 bool              voidsInDtm ;
 DPoint3d          ridgePts[2] ;
 BC_DTM_OBJ        *clipDtmP=NULL ;
 DTM_TIN_NODE      *nodeP ;
 DTMLineCache      lineCache ; 
 DTMDrainageTables *drainageTablesP=nullptr ;

// Log Entry Arguments

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Returning Ridge Lines") ; 
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
   }

// Validate Fence 

 numRidgeLines = 0 ;
  if( fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap) fenceOption = DTMFenceOption::Inside ;
 if( useFence && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = false ;
 if( useFence && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = false ;

// Test For Valid DTM Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;

// Test For DTM Object In Tin State

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ; 
   } 

// Check Load Function Set

 if( loadFunctionP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Call Back Function Not Set") ;
    goto errexit ;
   }

// Check For Voids

 voidsInDtm = bcdtmDrainageList_checkForVoidsInDtmObject(dtmP) ;

// Set Point Scan Range

 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;

// Build Clipping Tin For Fence

 if( useFence ) 
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
/*
    if( fenceOption == DTMFenceOption::Inside || fenceOption == DTMFenceOption::Overlap )
      {
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
       while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
       if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
       while( lastPnt < dtmP->numPoints - 1 && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
      }
*/
   }

// Scan Tin Points And Test For Ridge Lines

 for( pnt = startPnt ; pnt <= lastPnt ; ++pnt )
   {
    nodeP = nodeAddrP(dtmP,pnt) ;

//  Test For Valid Point

    if( ( clPtr = nodeP->cPtr ) != dtmP->nullPtr )
      {
       if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while ( clPtr != dtmP->nullPtr )
         {
          rdgPnt  = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr   = clistAddrP(dtmP,clPtr)->nextPtr ;
          if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,rdgPnt)) < 0 ) goto errexit ;

//        Check For Valid Tin Line

          if( rdgPnt > pnt )
            {


//           Check For Void Line

             voidLine = FALSE ; 
             if( voidsInDtm )
               {
                if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt,rdgPnt,&voidLine)) goto errexit ;
               }

//           Only Process For None Void Lines

             if( ! voidLine )
               {  

//              Test Tin Line For Fence Option Against Bounding Rectangle

                ridgeLineFound = true ;
                if( useFence )
                  {

//                 Get Bounding Rectangle For Ridge Line

                   xRidgeMin =  xRidgeMax = pointAddrP(dtmP,pnt)->x ;
                   yRidgeMin =  yRidgeMax = pointAddrP(dtmP,pnt)->y ;
                   if( pointAddrP(dtmP,rdgPnt)->x < xRidgeMin ) xRidgeMin = pointAddrP(dtmP,rdgPnt)->x ;
                   if( pointAddrP(dtmP,rdgPnt)->x > xRidgeMax ) xRidgeMax = pointAddrP(dtmP,rdgPnt)->x ;
                   if( pointAddrP(dtmP,rdgPnt)->y < yRidgeMin ) yRidgeMin = pointAddrP(dtmP,rdgPnt)->y ;
                   if( pointAddrP(dtmP,rdgPnt)->y > yRidgeMax ) yRidgeMax = pointAddrP(dtmP,rdgPnt)->y ;

//                 Check Fence Option Against Bounding Rectangle

                   if( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )
                     { 
                      if( xRidgeMax < clipDtmP->xMin ||  xRidgeMin > clipDtmP->xMax || yRidgeMax < clipDtmP->yMin ||  yRidgeMin > clipDtmP->yMax ) ridgeLineFound = true ;
                     }
                   if( fenceOption == DTMFenceOption::Outside )
                     { 
                      ridgeLineFound = false ;
                      if( xRidgeMax < clipDtmP->xMin ||  xRidgeMin > clipDtmP->xMax || yRidgeMax < clipDtmP->yMin ||  yRidgeMin > clipDtmP->yMax ) ridgeLineFound = true  ;
                     }
                  }

//              Process Further If Fence Option Staifies Bounding Rectangle

                if( ridgeLineFound )
                  {

//                 Check For Ridge Line 

                   if( bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP,drainageTablesP,pnt,rdgPnt,antPnt,clkPnt,&lineType)) goto errexit ;
                   if( lineType == DTMFeatureType::RidgeLine )
                     {

//                    Determine Ridge Line Extent With Fence

                      if( useFence )
                        {
                         ridgePts[0].x = pointAddrP(dtmP,pnt)->x    ; ridgePts[0].y = pointAddrP(dtmP,pnt)->y    ;  
                         ridgePts[1].x = pointAddrP(dtmP,rdgPnt)->x ; ridgePts[1].y = pointAddrP(dtmP,rdgPnt)->y ;  
                         if( bcdtmClip_determineFeatureExtentWithFenceDtmObject(clipDtmP,ridgePts,2,&ridgeExtent)) goto errexit ;
                         if( fenceOption != ridgeExtent ) ridgeLineFound = false ;
                        } 
 
//                    Store Ridge Line In Cache
         
                      if( ridgeLineFound ) 
                        {
                         ++numRidgeLines ; 
                         if( lineCache.StoreLineInCache(pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z,pointAddrP(dtmP,rdgPnt)->x,pointAddrP(dtmP,rdgPnt)->y,pointAddrP(dtmP,rdgPnt)->z)) goto errexit ;
                         if( lineCache.SizeOfCache() >= cacheSize )
                           {
                            if( lineCache.CallUserDelegateWithCacheLines(loadFunctionP,(DTMFeatureType)dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,userP) != DTM_SUCCESS ) goto errexit ;
                           }
                        }
                     }
                  }
               }
            }
          antPnt = rdgPnt ; 
         }
      }
   }

//  Check For Remaining Cache Lines

 if( lineCache.SizeOfCache() > 0 )
   {
    if( lineCache.CallUserDelegateWithCacheLines(loadFunctionP,(DTMFeatureType)dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,userP) != DTM_SUCCESS ) goto errexit ;
   }

//  Log Number Of Ridge Lines Returned

 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Ridge Lines = %8d",numRidgeLines) ;

// Clean Up

 cleanup :
 if( clipDtmP  != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Returning Ridge Lines Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Returning Ridge Lines Error") ; 
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_returnZeroSlopePolygonsDtmObject
(
 BC_DTM_OBJ         *dtmP,                    // ==> Pointer To Dtm Object 
 DTMFeatureCallback loadFunctionP,            // ==> Pointer To Load Function
 bool               useFence,                 // ==> Load Feature Within Fence
 DTMFenceType    fenceType,                // ==> Type Of Fence Reactangular Or Shape
 DTMFenceOption    fenceOption,              // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
 const DPoint3d     *fencePtsP,               // ==> DPoint3d Array Of Fence Points 
 int                numFencePts,              // ==> Number Of Fence Points
 void               *userP,                   // ==> User Pointer Passed Back To User
 int&               numZeroSlopePolygons      // <== Number Of Zero Slope Polygons Found Found 
)
{
 int               ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long              startTime=bcdtmClock(),polygonTime=bcdtmClock() ;
 bool              voidsInDtm ;
 BC_DTM_OBJ        *clipDtmP=NULL ;
 DTMPointCache     pointCache ; 
 DTMZeroSlopePolygonVector zeroSlopePolygons ;
 DTMZeroSlopePolygonVector::iterator zsp ;

// Log Entry Arguments

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Returning Zero Slope Polygons") ; 
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
   }

// Validate Fence 

 numZeroSlopePolygons = 0 ;
  if( fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap) fenceOption = DTMFenceOption::Inside ;
 if( useFence && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = false ;
 if( useFence && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = false ;

// Test For Valid DTM Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;

// Test For DTM Object In Tin State

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ; 
   } 

// Check Load Function Set

 if( loadFunctionP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Call Back Function Not Set") ;
    goto errexit ;
   }

// Check For Voids

 voidsInDtm = bcdtmDrainageList_checkForVoidsInDtmObject(dtmP) ;

//  Polygonize Zero Slope Triangles

 polygonTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Zero Slope Triangles") ; 
 if( bcdtmDrainage_polygoniseZeroSlopeTrianglesDtmObject(dtmP,zeroSlopePolygons)) goto errexit ;
 numZeroSlopePolygons = (int)(zeroSlopePolygons.end()-zeroSlopePolygons.begin()) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Zero Slope Polygons = %8ld",numZeroSlopePolygons) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Polygonise Zero Slope Triangles = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),polygonTime)) ;

//  Scan Zero Slope Polygons And Pass Back

 numZeroSlopePolygons = 0 ;
 for( zsp = zeroSlopePolygons.begin() ; zsp < zeroSlopePolygons.end() ; ++zsp )
   {
    for( int* pnt = zsp->pointList.pointsP ; pnt < zsp->pointList.pointsP + zsp->pointList.numPoints ; ++pnt )
      {
       if( pointCache.StorePointInCache(pointAddrP(dtmP,*pnt)->x,pointAddrP(dtmP,*pnt)->y,pointAddrP(dtmP,*pnt)->z) ) goto errexit ;
      } 
    ++numZeroSlopePolygons ; 
    if( pointCache.CallUserDelegateWithCachePoints(loadFunctionP,DTMFeatureType::ZeroSlopePolygon,(DTMUserTag)zsp->direction,dtmP->nullFeatureId,userP)) goto errexit ;
   }

//  Log Number Of Zero Slope Polygons Returned

 if( dbg  ) bcdtmWrite_message(0,0,0,"Number Of Zero Slope Polygons Returned = %8d",numZeroSlopePolygons) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Polygonise And Load Zero Slope Polygons = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

// Clean Up

 cleanup :
 if( clipDtmP  != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Returning Zero Slope Polygons Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Returning Zero Slope Polygons Error") ; 
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
