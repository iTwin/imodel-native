/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmLidarFilter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLidarFilter_coarseFilterTinPointsDtmObject (BC_DTM_OBJ *dtmP, double zTolerance, long *numFilteredP)
    {
    int ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long p1, p2, p3, p4, clPtr;
    double z;
    /*
    ** Write Entry Message
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Coarse Filtering Tin Points");
    /*
    ** Initialise
    */
    *numFilteredP = 0;
    /*
    ** Null Out Tptr Values
    */
    bcdtmList_nullTptrValuesDtmObject (dtmP);
    /*
    ** Scan Tin Triangles
    */
    for (p1 = 0; p1 < dtmP->numPoints; ++p1)
        {
        if (nodeAddrP (dtmP, p1)->tPtr == dtmP->nullPnt)
            {
            if ((clPtr = nodeAddrP (dtmP, p1)->cPtr) != dtmP->nullPtr)
                {
                p3 = clistAddrP (dtmP, clPtr)->pntNum;
                if ((p2 = bcdtmList_nextAntDtmObject (dtmP, p1, p3)) < 0) goto errexit;
                while (clPtr != dtmP->nullPtr)
                    {
                    p3 = clistAddrP (dtmP, clPtr)->pntNum;
                    clPtr = clistAddrP (dtmP, clPtr)->nextPtr;
                    if (nodeAddrP (dtmP, p1)->hPtr != p2)
                        {
                        if (p2 > p1 && p3 > p1 && nodeAddrP (dtmP, p2)->tPtr == dtmP->nullPnt && nodeAddrP (dtmP, p3)->tPtr == dtmP->nullPnt)
                            {
                            /*
                            **                 Filter Point External To P1-P2
                            */
                            if (nodeAddrP (dtmP, p2)->hPtr != p1)
                                {
                                if ((p4 = bcdtmList_nextAntDtmObject (dtmP, p1, p2)) < 0) goto errexit;
                                if (nodeAddrP (dtmP, p4)->hPtr == dtmP->nullPnt && nodeAddrP (dtmP, p4)->tPtr == dtmP->nullPnt)
                                    {
                                    bcdtmMath_interpolatePointOnTriangleDtmObject (dtmP, pointAddrP (dtmP, p4)->x, pointAddrP (dtmP, p4)->y, &z, p1, p2, p3);
                                    if (fabs (z - pointAddrP (dtmP, p4)->z) < zTolerance)
                                        {
                                        nodeAddrP (dtmP, p4)->tPtr = 1;
                                        ++*numFilteredP;
                                        }
                                    }
                                }
                            /*
                            **                 Filter Point External To P3-P2
                            */
                            if (nodeAddrP (dtmP, p3)->hPtr != p2)
                                {
                                if ((p4 = bcdtmList_nextAntDtmObject (dtmP, p2, p3)) < 0) goto errexit;
                                if (nodeAddrP (dtmP, p4)->hPtr == dtmP->nullPnt  && nodeAddrP (dtmP, p4)->tPtr == dtmP->nullPnt)
                                    {
                                    bcdtmMath_interpolatePointOnTriangleDtmObject (dtmP, pointAddrP (dtmP, p4)->x, pointAddrP (dtmP, p4)->y, &z, p1, p2, p3);
                                    if (fabs (z - pointAddrP (dtmP, p4)->z) < zTolerance)
                                        {
                                        nodeAddrP (dtmP, p4)->tPtr = 1;
                                        ++*numFilteredP;
                                        }
                                    }
                                }
                            /*
                            **                 Filter Point External To P1-P3
                            */
                            if (nodeAddrP (dtmP, p1)->hPtr != p3)
                                {
                                if ((p4 = bcdtmList_nextAntDtmObject (dtmP, p3, p1)) < 0) goto errexit;
                                if (nodeAddrP (dtmP, p4)->hPtr == dtmP->nullPnt  && nodeAddrP (dtmP, p4)->tPtr == dtmP->nullPnt)
                                    {
                                    bcdtmMath_interpolatePointOnTriangleDtmObject (dtmP, pointAddrP (dtmP, p4)->x, pointAddrP (dtmP, p4)->y, &z, p1, p2, p3);
                                    if (fabs (z - pointAddrP (dtmP, p4)->z) < zTolerance)
                                        {
                                        nodeAddrP (dtmP, p4)->tPtr = 1;
                                        ++*numFilteredP;
                                        }
                                    }
                                }
                            }
                        }
                    p2 = p3;
                    }
                }
            }
        }
    /*
    ** Write Results
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of Points Filtered = %6ld", *numFilteredP);
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Filtering Tin Points Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Filtering Tin Points Error");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLidarFilter_fineFilterTinPointsDtmObject (BC_DTM_OBJ *dtmP, double zTolerance, long *numFilteredP)
    {
    int ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long p1, p2, p3, p4, p5, p6, clPtr;
    double z;
    /*
    ** Write Entry Message
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Filtering Tin Points");
    /*
    ** Initialise
    */
    *numFilteredP = 0;
    /*
    ** Null Out Tptr Values
    */
    for (p1 = 0; p1 < dtmP->numPoints; ++p1) nodeAddrP (dtmP, p1)->tPtr = dtmP->nullPnt;
    /*
    ** Scan Tin Triangles
    */
    for (p1 = 0; p1 < dtmP->numPoints; ++p1)
        {
        if (nodeAddrP (dtmP, p1)->tPtr == dtmP->nullPnt)
            {
            if ((clPtr = nodeAddrP (dtmP, p1)->cPtr) != dtmP->nullPtr)
                {
                p3 = clistAddrP (dtmP, clPtr)->pntNum;
                if ((p2 = bcdtmList_nextAntDtmObject (dtmP, p1, p3)) < 0) goto errexit;
                while (clPtr != dtmP->nullPtr)
                    {
                    p3 = clistAddrP (dtmP, clPtr)->pntNum;
                    clPtr = clistAddrP (dtmP, clPtr)->nextPtr;
                    if (nodeAddrP (dtmP, p1)->hPtr != p2)
                        {
                        if (p2 > p1 && p3 > p1 && nodeAddrP (dtmP, p2)->tPtr == dtmP->nullPnt && nodeAddrP (dtmP, p3)->tPtr == dtmP->nullPnt)
                            {
                            /*
                            **                 Get Point External To P1-P2
                            */
                            p4 = dtmP->nullPnt;
                            if (nodeAddrP (dtmP, p2)->hPtr != p1)
                                {
                                if ((p4 = bcdtmList_nextAntDtmObject (dtmP, p1, p2)) < 0) goto errexit;
                                }
                            /*
                            **                 Get Point External To P3-P2
                            */
                            p5 = dtmP->nullPnt;
                            if (nodeAddrP (dtmP, p3)->hPtr != p2)
                                {
                                if ((p5 = bcdtmList_nextAntDtmObject (dtmP, p2, p3)) < 0) goto errexit;
                                }
                            /*
                            **                 Get Point External To P1-P3
                            */
                            p6 = dtmP->nullPnt;
                            if (nodeAddrP (dtmP, p1)->hPtr != p3)
                                {
                                if ((p6 = bcdtmList_nextAntDtmObject (dtmP, p3, p1)) < 0) goto errexit;
                                }
                            /*
                            **                 Check Points Have Not Been Previously Filtered
                            */
                            if (p4 != dtmP->nullPnt && nodeAddrP (dtmP, p4)->tPtr != dtmP->nullPnt) p4 = dtmP->nullPnt;
                            if (p5 != dtmP->nullPnt && nodeAddrP (dtmP, p5)->tPtr != dtmP->nullPnt) p5 = dtmP->nullPnt;
                            if (p6 != dtmP->nullPnt && nodeAddrP (dtmP, p6)->tPtr != dtmP->nullPnt) p6 = dtmP->nullPnt;
                            /*
                            **                 Check If Points Can Be Filtered
                            */
                            if (p4 != dtmP->nullPnt && p5 != dtmP->nullPnt && p6 != dtmP->nullPnt)
                                {
                                bcdtmMath_interpolatePointOnTriangleDtmObject (dtmP, pointAddrP (dtmP, p4)->x, pointAddrP (dtmP, p4)->y, &z, p1, p2, p3);
                                if (fabs (z - pointAddrP (dtmP, p4)->z) < zTolerance)
                                    {
                                    bcdtmMath_interpolatePointOnTriangleDtmObject (dtmP, pointAddrP (dtmP, p5)->x, pointAddrP (dtmP, p5)->y, &z, p1, p2, p3);
                                    if (fabs (z - pointAddrP (dtmP, p5)->z) < zTolerance)
                                        {
                                        bcdtmMath_interpolatePointOnTriangleDtmObject (dtmP, pointAddrP (dtmP, p6)->x, pointAddrP (dtmP, p6)->y, &z, p1, p2, p3);
                                        if (fabs (z - pointAddrP (dtmP, p6)->z) < zTolerance)
                                            {
                                            nodeAddrP (dtmP, p1)->tPtr = 1;
                                            nodeAddrP (dtmP, p2)->tPtr = 1;
                                            nodeAddrP (dtmP, p3)->tPtr = 1;
                                            *numFilteredP = *numFilteredP + 3;
                                            clPtr = dtmP->nullPtr;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    p2 = p3;
                    }
                }
            }
        }
    /*
    ** Write Results
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of Points Filtered = %6ld", *numFilteredP);
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Filtering Tin Points Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Filtering Tin Points Error");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }
