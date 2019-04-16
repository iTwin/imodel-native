/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "bcdtmDrainage.h"
#include <TerrainModel/Core/bcdtminlines.h>

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

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
    BC_DTM_OBJ    *clipDtmP = nullptr;
    DTM_TIN_NODE  *nodeP;
    DTMPointCache pointCache;
    std::vector<long> zeroPts;
    unsigned char* pointDone = nullptr;

    // Log Input Arguments

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Returning low Points");
        bcdtmWrite_message (0, 0, 0, "dtmP           = %p", dtmP);
     //   bcdtmWrite_message (0, 0, 0, "loadFunctionP  = %p", loadFunctionP);
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
    if (useFence && (fencePtsP == nullptr || numFencePts <= 2)) useFence = false;
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

    if (loadFunctionP == nullptr)
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
    if (clipDtmP != nullptr) bcdtmObject_destroyDtmObject (&clipDtmP);

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
    BC_DTM_OBJ    *clipDtmP = nullptr;
    DTM_TIN_NODE  *nodeP;
    DTMPointCache pointCache;
    std::stack<long> zeroPts;
    unsigned char* pointDone = nullptr;

    // Log Input Arguments

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Returning High Points");
        bcdtmWrite_message (0, 0, 0, "dtmP           = %p", dtmP);
  //      bcdtmWrite_message (0, 0, 0, "loadFunctionP  = %p", loadFunctionP);
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
    if (useFence && (fencePtsP == nullptr || numFencePts <= 2)) useFence = false;
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

    if (loadFunctionP == nullptr)
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
    if (clipDtmP != nullptr) bcdtmObject_destroyDtmObject (&clipDtmP);

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
int bcdtmDrainage_returnRidgeOrSumpLinesDtmObject
(
    BC_DTM_OBJ         *dtmP,                    // ==> Pointer To Dtm Object 
    DTMFeatureType     dtmFeatureType,           // == Ridge or Sump
    bool               zeroSlopeOnly,         // ==> Return Zero Slope lines only
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
    int               ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE(0);
    int               cacheSize = 10000;
    long              startPnt, lastPnt, clPtr;
    DTMFeatureType  lineType;
    bool              voidsInDtm;
    BC_DTM_OBJ        *clipDtmP = nullptr;
    DTMLineCache      lineCache;
    DTMDrainageTables *drainageTablesP = nullptr;

    // Log Entry Arguments

    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "Returning Ridge Lines");
        bcdtmWrite_message(0, 0, 0, "dtmP              = %p", dtmP);
    //    bcdtmWrite_message(0, 0, 0, "loadFunctionP     = %p", loadFunctionP);
        bcdtmWrite_message(0, 0, 0, "useFence          = %8ld", useFence);
        bcdtmWrite_message(0, 0, 0, "fenceOption       = %8ld", fenceOption);
        bcdtmWrite_message(0, 0, 0, "fencePtsP         = %p", fencePtsP);
        bcdtmWrite_message(0, 0, 0, "numFencePts       = %8ld", numFencePts);
        bcdtmWrite_message(0, 0, 0, "userP             = %p", userP);
        }

    // Validate Fence 
    numRidgeLines = 0;
    if (useFence && (fencePtsP == nullptr || numFencePts <= 2)) useFence = false;
    if (useFence && (fencePtsP->x != (fencePtsP + numFencePts - 1)->x || fencePtsP->y != (fencePtsP + numFencePts - 1)->y)) useFence = false;

    if (useFence)
        {
        if (fenceType != DTMFenceType::Block && fenceType != DTMFenceType::Shape) fenceType = DTMFenceType::Block;
        if (fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap) fenceOption = DTMFenceOption::Overlap;
        }


    // Test For Valid DTM Object
    if (bcdtmObject_testForValidDtmObject(dtmP)) goto errexit;

    // Test For DTM Object In Tin State

    if (dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message(1, 0, 0, "Method Requires Triangulated DTM");
        goto errexit;
        }

    // Check Load Function Set
    if (loadFunctionP == nullptr)
        {
        bcdtmWrite_message(1, 0, 0, "Call Back Function Not Set");
        goto errexit;
        }

    // Check For Voids
    voidsInDtm = bcdtmDrainageList_checkForVoidsInDtmObject(dtmP);

    // Set Point Scan Range
    startPnt = 0;
    lastPnt = dtmP->numPoints - 1;

    // Build Clipping Tin For Fence
    if (useFence)
        {
        if (dbg) bcdtmWrite_message(0, 0, 0, "Building Clipping Tin");
        if (bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP, fencePtsP, numFencePts)) goto errexit;
        if (dbg) bcdtmWrite_message(0, 0, 0, "Building Clipping Tin Completed");
        if (fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax) useFence = FALSE;
        }

    /*
    **    Find First Point Before And Last Point After Fence
    */
    startPnt = 0 ;
    lastPnt  = dtmP->numPoints ;
    numRidgeLines = 0 ;

    if (useFence && fenceOption == DTMFenceOption::Overlap)
        {
        if (dbg) bcdtmWrite_message(0, 0, 0, "Scanning For Overlap Triangle Edges");
        bcdtmFind_binaryScanDtmObject(dtmP, clipDtmP->xMin, &startPnt);
        while (startPnt > 0 && pointAddrP(dtmP, startPnt)->x >= clipDtmP->xMin) --startPnt;
        if (pointAddrP(dtmP, startPnt)->x < clipDtmP->xMin) ++startPnt;
        bcdtmFind_binaryScanDtmObject(dtmP, clipDtmP->xMax, &lastPnt);
        while (lastPnt < dtmP->numPoints - 1 && pointAddrP(dtmP, lastPnt)->x <= clipDtmP->xMin) ++lastPnt;
        /*
        **      Mark Points Within Fence Block
        */
        if (fenceType == DTMFenceType::Block)
            {
            for (long p1 = startPnt; p1 <= lastPnt; ++p1)
                {
                DPoint3dP pntP = pointAddrP(dtmP, p1);
                if (pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax) nodeAddrP(dtmP, p1)->sPtr = 1;
                }
            }
        /*
        **      Mark Points Within Fence Shape
        */
        if (fenceType == DTMFenceType::Shape)
            {
            for (long p1 = startPnt; p1 <= lastPnt; ++p1)
                {
                DPoint3dP pntP = pointAddrP(dtmP, p1);
                long findType = 0;
                if (pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax)
                    {
                    long trgPnt1, trgPnt2, trgPnt3;
                    if (bcdtmFind_triangleDtmObject(clipDtmP, pntP->x, pntP->y, &findType, &trgPnt1, &trgPnt2, &trgPnt3)) goto errexit;
                    }
                if (findType) nodeAddrP(dtmP, p1)->sPtr = 1;
                }
            }
            /*
            **      Scan And Load Triangle Edges
            */
            for (long p1 = startPnt; p1 <= lastPnt; ++p1)
                {
                auto node1P = nodeAddrP(dtmP, p1);
                if (node1P->sPtr == 1 && (clPtr = node1P->cPtr) != dtmP->nullPtr)
                    {
                    while (clPtr != dtmP->nullPtr)
                        {
                        auto clistP = clistAddrP(dtmP, clPtr);
                        auto p2 = clistP->pntNum;
                        clPtr = clistP->nextPtr;
                        auto node2P = nodeAddrP(dtmP, p2);
                        if (p2 > p1 && node2P->sPtr == 1)
                            {
                            bool voidFlag = false;
                            if (voidsInDtm)
                                {
                                if (bcdtmList_testForVoidLineDtmObject(dtmP, p1, p2, voidFlag)) goto errexit;
                                }
                            if (voidFlag == false && (!zeroSlopeOnly || (pointAddrP(dtmP, p1)->z == pointAddrP(dtmP, p2)->z)))
                                {
                                /*
                                **                     Set Point Addresses And Coordinates
                                */
                                long antPnt = bcdtmList_nextAntDtmObject(dtmP, p1, p2);
                                long clkPnt = bcdtmList_nextClkDtmObject(dtmP, p1, p2);
                                if (bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP, drainageTablesP, p1, p2, antPnt, clkPnt, &lineType)) goto errexit;

                                if (lineType == dtmFeatureType)
                                    {
                                    numRidgeLines++;
                                    if (lineCache.StoreLineInCache(*pointAddrP(dtmP, p1), *pointAddrP(dtmP, p2))) goto errexit;
                                    if (lineCache.SizeOfCache() >= cacheSize &&
                                        lineCache.CallUserDelegateWithCacheLines(loadFunctionP, (DTMFeatureType)dtmFeatureType, DTM_NULL_USER_TAG, DTM_NULL_FEATURE_ID, true, userP) != DTM_SUCCESS) goto errexit;
                                    }
                                }
                            }
                        }
                    }
                }
        }
    /*
    **    Scan And Load Triangle Edges
    */
    else
        {
        if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Triangle Edges") ;
        for( long p1 = startPnt ; p1 < lastPnt ; ++p1 )
            {
            auto nodeP = nodeAddrP(dtmP,p1) ;
            if( ( clPtr = nodeP->cPtr) != dtmP->nullPtr )
                {
                while( clPtr != dtmP->nullPtr )
                    {
                    auto clistP = clistAddrP(dtmP,clPtr) ;
                    auto p2     = clistP->pntNum ;
                    clPtr  = clistP->nextPtr ;
                    if( p2 > p1 )
                        {
                        bool voidFlag = false;
                        if( voidsInDtm ) { if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,voidFlag)) goto errexit ; }
                        if (voidFlag == false && (!zeroSlopeOnly || (pointAddrP(dtmP, p1)->z == pointAddrP(dtmP, p2)->z)))
                            {
                            long antPnt = bcdtmList_nextAntDtmObject(dtmP, p1, p2);
                            long clkPnt = bcdtmList_nextClkDtmObject(dtmP, p1, p2);
                            if (bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP, drainageTablesP, p1, p2, antPnt, clkPnt, &lineType)) goto errexit;

                            if (lineType == dtmFeatureType)
                                {
                                numRidgeLines++;

                                /*
                                **                      Load Triangle Edge
                                */
                                if (!useFence)
                                    {
                                    if (lineCache.StoreLineInCache(*pointAddrP(dtmP, p1), *pointAddrP(dtmP, p2))) goto errexit;
                                    }
                                /*
                                **                      Check If Triangle Edge Lies In Fence
                                */
                                else
                                    {
                                    DPoint3d ridgePts[2] = {*pointAddrP(dtmP, p1), *pointAddrP(dtmP, p2)};
                                    long numClipArrays, clipResult;
                                    DTM_POINT_ARRAY** clipArraysPP = nullptr;

                                    if (bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP, fenceOption, ridgePts, 2, &clipResult, &clipArraysPP, &numClipArrays)) goto errexit;
                                    if (clipResult == 1)
                                        {
                                        if (lineCache.StoreLineInCache(*pointAddrP(dtmP, p1), *pointAddrP(dtmP, p2))) goto errexit;
                                        }
                                    else if (clipResult == 2)
                                        {
                                        for (long n = 0; n < numClipArrays; ++n)
                                            {
                                            BeAssert(clipArraysPP[n]->numPoints == 2);
                                            if (lineCache.StoreLineInCache(clipArraysPP[n]->pointsP[0], clipArraysPP[n]->pointsP[1])) goto errexit;
                                            }
                                        bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP, numClipArrays);
                                        }
                                    }

                                if (lineCache.SizeOfCache() >= cacheSize && lineCache.CallUserDelegateWithCacheLines(loadFunctionP, dtmFeatureType, DTM_NULL_USER_TAG, DTM_NULL_FEATURE_ID, true, userP) != DTM_SUCCESS) goto errexit;

                                }
                            }
                        }
                    }
                }
            }
        }
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Triangle Edges Loaded = %10ld",numRidgeLines) ;

    //  Check For Remaining Cache Lines
    if (lineCache.SizeOfCache() > 0)
        {
        if (lineCache.CallUserDelegateWithCacheLines(loadFunctionP, dtmFeatureType, DTM_NULL_USER_TAG, DTM_NULL_FEATURE_ID, true, userP) != DTM_SUCCESS) goto errexit;
        }

    //  Log Number Of Ridge Lines Returned

    if (dbg) bcdtmWrite_message(0, 0, 0, "Number Of Ridge Lines = %8d", numRidgeLines);

    // Clean Up

cleanup:
    if (clipDtmP != nullptr) bcdtmObject_destroyDtmObject(&clipDtmP);

    // Return

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Returning Ridge Lines Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Returning Ridge Lines Error");
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
        return bcdtmDrainage_returnRidgeOrSumpLinesDtmObject(dtmP, DTMFeatureType::SumpLine, true, loadFunctionP, useFence, fenceType, fenceOption, fencePtsP, numFencePts, userP, numSumpLines);
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
    DTMFenceType    fenceType,                // ==> Type Of Fence Reactangular Or Shape
    DTMFenceOption    fenceOption,              // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
    const DPoint3d     *fencePtsP,               // ==> DPoint3d Array Of Fence Points 
    int                numFencePts,              // ==> Number Of Fence Points
    void               *userP,                   // ==> User Pointer Passed Back To User
    int&               numSumpLines             // <== Number Of Ridge Lines Found 
)
    {
    return bcdtmDrainage_returnRidgeOrSumpLinesDtmObject(dtmP, DTMFeatureType::SumpLine, false, loadFunctionP, useFence, fenceType, fenceOption, fencePtsP, numFencePts, userP, numSumpLines);
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
    return bcdtmDrainage_returnRidgeOrSumpLinesDtmObject(dtmP, DTMFeatureType::RidgeLine, false, loadFunctionP, useFence, fenceType, fenceOption, fencePtsP, numFencePts, userP, numRidgeLines);
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
 BC_DTM_OBJ        *clipDtmP=nullptr ;
 DTMPointCache     pointCache ; 
 DTMZeroSlopePolygonVector zeroSlopePolygons ;

// Log Entry Arguments

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Returning Zero Slope Polygons") ; 
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
  //  bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
   }

// Validate Fence 

 numZeroSlopePolygons = 0 ;
  if( fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap) fenceOption = DTMFenceOption::Inside ;
 if( useFence && ( fencePtsP == nullptr || numFencePts <= 2 ) ) useFence = false ;
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

 if( loadFunctionP == nullptr )
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
 numZeroSlopePolygons = (int)zeroSlopePolygons.size() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Zero Slope Polygons = %8ld",numZeroSlopePolygons) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Polygonise Zero Slope Triangles = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),polygonTime)) ;

//  Scan Zero Slope Polygons And Pass Back

 numZeroSlopePolygons = 0 ;
 for(auto&& zsp : zeroSlopePolygons)
   {
    for(const auto pnt : zsp.pointList)
      {
       if( pointCache.StorePointInCache(*pointAddrP(dtmP,pnt)) ) goto errexit ;
      } 
    ++numZeroSlopePolygons ; 
    if( pointCache.CallUserDelegateWithCachePoints(loadFunctionP,DTMFeatureType::ZeroSlopePolygon,(DTMUserTag)zsp.direction,dtmP->nullFeatureId,userP)) goto errexit ;
   }

//  Log Number Of Zero Slope Polygons Returned

 if( dbg  ) bcdtmWrite_message(0,0,0,"Number Of Zero Slope Polygons Returned = %8d",numZeroSlopePolygons) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Polygonise And Load Zero Slope Polygons = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

// Clean Up

 cleanup :
 if( clipDtmP  != nullptr ) bcdtmObject_destroyDtmObject(&clipDtmP) ;

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Returning Zero Slope Polygons Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Returning Zero Slope Polygons Error") ; 
 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

END_BENTLEY_TERRAINMODEL_NAMESPACE
