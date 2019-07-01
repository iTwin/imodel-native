/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <string.h>
#include <mbstring.h>
#include "TerrainModel/TerrainModel.h"
#include "TerrainModel/Formats/Formats.h"
#include "TerrainModel/Core/bcDTMBaseDef.h"
#include "TerrainModel/Core/dtmdefs.h"
#include "TerrainModel/Core/DTMEvars.h"
#include "TerrainModel/Formats/InRoads.h"
#include <TerrainModel/Core/bcdtmInlines.h>

#define MAXAREAFORVOIDORISLANDS 0.001


/*
** InRoads Function Prototypes
*/

typedef int(*tinStatsCallBackFunctionDef)(long numRandomPoints, long numFeaturePoints, long numTriangles, long NumFeatures);
typedef int(*tinRandomPointsCallBackFunctionDef)(long pntIndex, double X, double Y, double Z);
typedef int(*tinFeaturePointsCallBackFunctionDef)(long pntIndex, double X, double Y, double Z);
typedef int(*tinTrianglesCallBackFunctionDef)(long trgIndex, long pntIndex1, long pntIndex2, long pntIndex3, long voidTriangle, long side1TrgIndex, long side2TrgIndex, long side3TrgIndex);
typedef int(*tinFeaturesCallBackFunctionDef)(long dtmFeatureType, __int64 dtmUsertag, __int64 dtmFeatureId, long *pointIndicesP, long numPointIndices);

int inroadsTM_setConvertGPKTinToDTMFunction(int(*callBackFunctionP)(WCharCP tinFileNameP, tinStatsCallBackFunctionDef tinStatsCallBackFunctionP, tinRandomPointsCallBackFunctionDef tinRandomPointsCallBackFunctionP, tinFeaturePointsCallBackFunctionDef tinFeaturePointsCallBackFunctionP, tinTrianglesCallBackFunctionDef tinTrianglesCallBackFunctionP, tinFeaturesCallBackFunctionDef tinFeaturesCallBackFunctionP));
int inroadsTM_convertGPKTinToDTM(WCharCP tinFileNameP, WCharCP dtmFileNameP, WCharCP nameP, WCharCP descriptionP, int version);

static BC_DTM_OBJ *glbDtmP = 0;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt bcdtmFormatInroads_insertRectangleAroundTinDtmObject
(
BC_DTM_OBJ *dtmP,
double  xdec,                     /* ==> Decrement Tin Xmin                   */
double  xinc,                     /* ==> Increment Tin Xmax                   */
double  ydec,                     /* ==> Decrement Tin Ymin                   */
double  yinc,                     /* ==> Increment Tin Ymax                   */
DTMFeatureId *islandFeatureIdP  /* <== Island Feature Id For Prior Tin Hull */
)
/*
** This Function Adds A Surrounding Rectangle To A Triangulated DTM
** To Simulate An Inroads Triangulation In bcLIB DTM. This Is Done Prior To
** Exporting bcLIB Triangles To An Inroads Triangulation
*/
    {
    DTMStatusInt ret = DTM_SUCCESS;
    int dbg = 0, cdbg = 0;
    long point, closePoint;
    long dtmFeature, numHullPts, hullFeature, rectangleFeature, tinPoints[4];
    long priorPnt, startPnt, nextPnt, endPnt, clkPnt;
    long cPriorPnt, cStartPnt, cNextPnt, cClkPnt;
    long hullPnt;
    DPoint3d  rectanglePts[5], *hullPtsP = nullptr;
    BC_DTM_OBJ *tempDtmP = nullptr;
    BC_DTM_FEATURE *dtmFeatureP;
    DPoint3d *pointP;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "Inserting Rectangle Around Tin");
        bcdtmWrite_message(0, 0, 0, "dtmP  = %p", dtmP);
        bcdtmWrite_message(0, 0, 0, "xdec  = %12.5lf", xdec);
        bcdtmWrite_message(0, 0, 0, "xinc  = %12.5lf", xinc);
        bcdtmWrite_message(0, 0, 0, "ydec  = %12.5lf", ydec);
        bcdtmWrite_message(0, 0, 0, "yinc  = %12.5lf", yinc);
        }
    /*
    ** Initialise
    */
    *islandFeatureIdP = DTM_NULL_FEATURE_ID;
    /*
    ** Test For Valid Dtm Object
    */
    if (bcdtmObject_testForValidDtmObject(dtmP)) goto errexit;
    /*
    ** Check DTM Is In Triangulated State
    */
    if (dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message(2, 0, 0, "Method Requires Triangulated Dtm");
        goto errexit;
        }
    /*
    ** Write Stats Prior To Adding Rectangle
    */
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "Before Adding External Rectangle");
        bcdtmWrite_message(0, 0, 0, "dtmP->numPoints        = %8ld", dtmP->numPoints);
        bcdtmWrite_message(0, 0, 0, "dtmP->numSortedPoints  = %8ld", dtmP->numSortedPoints);
        bcdtmWrite_message(0, 0, 0, "dtmP->numTriangles     = %8ld", dtmP->numTriangles);
        bcdtmWrite_message(0, 0, 0, "dtmP->numLines         = %8ld", dtmP->numLines);
        bcdtmWrite_message(0, 0, 0, "dtmP->numFeatures      = %8ld", dtmP->numFeatures);
        hullPnt = dtmP->hullPoint;
        numHullPts = 0;
        do
            {
            ++numHullPts;
            hullPnt = nodeAddrP(dtmP, hullPnt)->hPtr;
            } while (hullPnt != dtmP->hullPoint);
            ++numHullPts;
            bcdtmWrite_message(0, 0, 0, "numHullPts            = %8ld", numHullPts);
        }
    /*
    ** Add Old Tin Hull As Island Feature
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "dtmP->hullPoint = %8ld", dtmP->hullPoint);
    if (bcdtmList_copyHptrListToTptrListDtmObject(dtmP, dtmP->hullPoint)) goto errexit;
    if (dbg) bcdtmWrite_message(0, 0, 0, "BB dtmP->numFeatures = %8ld", dtmP->numFeatures);
    if (bcdtmInsert_addDtmFeatureToDtmObject(dtmP, nullptr, 0, DTMFeatureType::Island, -9999, dtmP->dtmFeatureIndex, dtmP->hullPoint, 1)) goto errexit;
    if (dbg) bcdtmWrite_message(0, 0, 0, "AA dtmP->numFeatures = %8ld", dtmP->numFeatures);
    *islandFeatureIdP = dtmP->dtmFeatureIndex;
    dtmFeatureP = ftableAddrP(dtmP, dtmP->numFeatures - 1);
    if (dbg) bcdtmWrite_message(0, 0, 0, "islandFeatureIdP = %8ld", *islandFeatureIdP);
    if (dbg) bcdtmWrite_message(0, 0, 0, "dtmFeatureState = %4ld dtmFeatureId = %8I64d dtmUserTag = %8I64d ** dtmFeatureType = %4ld", dtmFeatureP->dtmFeatureState, dtmFeatureP->dtmFeatureId, dtmFeatureP->dtmUserTag, dtmFeatureP->dtmFeatureType);
    if (dtmFeatureP->dtmFeatureType != DTMFeatureType::Island) dtmFeatureP->dtmFeatureType = DTMFeatureType::Island;
    ++dtmP->dtmFeatureIndex;
    /*
    ** Create Triangulation Of Existing Tin Hull And Surrounding Rectangle
    */
    if (bcdtmObject_createDtmObject(&tempDtmP) != DTM_SUCCESS) goto errexit;
    /*
    ** Extract Tin Hull
    */
    if (bcdtmList_extractHullDtmObject(dtmP, &hullPtsP, &numHullPts)) goto errexit;
    if (dbg) bcdtmWrite_message(0, 0, 0, "Number Of Hull Points = %6ld", numHullPts);
    /*
    ** Set Point Memory Allocation Parameters
    */
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP, numHullPts * 4 + 5, numHullPts);
    /*
    ** Store Tin Hull
    */
    if (bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP, DTMFeatureType::GraphicBreak, 1, 1, &tempDtmP->nullFeatureId, hullPtsP, numHullPts)) goto errexit;
    if (bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP, DTMFeatureType::GraphicBreak, 1, 1, &tempDtmP->nullFeatureId, hullPtsP, numHullPts)) goto errexit;
    if (bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP, DTMFeatureType::GraphicBreak, 1, 1, &tempDtmP->nullFeatureId, hullPtsP, numHullPts)) goto errexit;
    if (bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP, DTMFeatureType::Breakline, 1, 1, &tempDtmP->nullFeatureId, hullPtsP, numHullPts)) goto errexit;
    /*
    ** Store Surrounding Rectangle
    */
    rectanglePts[0].x = dtmP->xMin - xdec; rectanglePts[0].y = dtmP->yMin - ydec; rectanglePts[0].z = -999;
    rectanglePts[1].x = dtmP->xMax + xinc; rectanglePts[1].y = dtmP->yMin - ydec; rectanglePts[1].z = -999;
    rectanglePts[2].x = dtmP->xMax + xinc; rectanglePts[2].y = dtmP->yMax + yinc; rectanglePts[2].z = -999;
    rectanglePts[3].x = dtmP->xMin - xdec; rectanglePts[3].y = dtmP->yMax + yinc; rectanglePts[3].z = -999;
    rectanglePts[4].x = dtmP->xMin - xdec; rectanglePts[4].y = dtmP->yMin - ydec; rectanglePts[4].z = -999;
    if (bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP, DTMFeatureType::Breakline, 2, 1, &tempDtmP->nullFeatureId, rectanglePts, 5)) goto errexit;
    if (dbg) bcdtmWrite_geopakDatFileFromDtmObject(tempDtmP, L"padded.dat");
    /*
    ** Triangulate DTM
    */
    if (dbg)  bcdtmWrite_message(0, 0, 0, "Triangulating Temp Dtm Object ** tempDtmP->numPoints = %8ld", tempDtmP->numPoints); //
    dtmP->ppTol = 0.0;
    dtmP->plTol = 0.0;
    if (bcdtmObject_createTinDtmObject(tempDtmP, 1, 0.0, false, false)) goto errexit;
    if (dbg)  bcdtmWrite_message(0, 0, 0, "Triangulating Temp Dtm Object Completed ** tempDtmP->numPoints = %8ld", tempDtmP->numPoints);
    /*
    ** Check Triangulation
    */
    if (cdbg)
        {
        bcdtmWrite_message(0, 0, 0, "Checking Temp Triangulation");
        if (bcdtmCheck_tinComponentDtmObject(tempDtmP))
            {
            bcdtmWrite_message(2, 0, 0, "Triangulation Invalid");
            goto errexit;
            }
        bcdtmWrite_message(0, 0, 0, "Temp Triangulation Valid");
        }
    /*
    ** Find Entry In Tin For Hull And Rectangle Features
    */
    hullFeature = tempDtmP->nullPnt;
    rectangleFeature = tempDtmP->nullPnt;
    for (dtmFeature = 0; dtmFeature < tempDtmP->numFeatures; ++dtmFeature)
        {
        dtmFeatureP = ftableAddrP(tempDtmP, dtmFeature);
        if (dtmFeatureP->dtmUserTag == 1 && dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin) hullFeature = dtmFeature;
        if (dtmFeatureP->dtmUserTag == 2 && dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin) rectangleFeature = dtmFeature;
        }
    if (hullFeature == tempDtmP->nullPnt || rectangleFeature == tempDtmP->nullPnt)
        {
        bcdtmWrite_message(2, 0, 0, "Cannot Find Feature Entries");
        goto errexit;
        }
    if (dbg) bcdtmWrite_message(0, 0, 0, "hullFeature = %8ld ** rectangleFeature = %8ld", hullFeature, rectangleFeature);
    /*
    ** Add Rectangle Points To Tin
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Adding Rectangle Points To Tin");
    if (bcdtmInsert_addPointToDtmObject(dtmP, rectanglePts[0].x, rectanglePts[0].y, rectanglePts[0].z, &tinPoints[0])) goto errexit;
    if (bcdtmInsert_addPointToDtmObject(dtmP, rectanglePts[1].x, rectanglePts[1].y, rectanglePts[1].z, &tinPoints[1])) goto errexit;
    if (bcdtmInsert_addPointToDtmObject(dtmP, rectanglePts[2].x, rectanglePts[2].y, rectanglePts[2].z, &tinPoints[2])) goto errexit;
    if (bcdtmInsert_addPointToDtmObject(dtmP, rectanglePts[3].x, rectanglePts[3].y, rectanglePts[3].z, &tinPoints[3])) goto errexit;
    /*
    ** Add Rectangle Lines To Tin
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Adding Rectangle Lines To Tin");
    if (bcdtmList_insertLineDtmObject(dtmP, tinPoints[0], tinPoints[1])) goto errexit;
    if (bcdtmList_insertLineDtmObject(dtmP, tinPoints[1], tinPoints[2])) goto errexit;
    if (bcdtmList_insertLineDtmObject(dtmP, tinPoints[2], tinPoints[3])) goto errexit;
    if (bcdtmList_insertLineDtmObject(dtmP, tinPoints[3], tinPoints[0])) goto errexit;
    /*
    ** Scan Temp Tin And Set DTM Point Number In sPtr Array
    */
    for (point = 0; point < tempDtmP->numPoints; ++point)
        {
        pointP = pointAddrP(tempDtmP, point);
        bcdtmFind_closestPointDtmObject(dtmP, pointP->x, pointP->y, &closePoint);
        nodeAddrP(tempDtmP, point)->sPtr = closePoint;
        }
    /*
    ** Check For Duplicate Points
    */
    if (cdbg)
        {
        for (point = 0; point < tempDtmP->numPoints; ++point)
            {
            for (startPnt = point + 1; startPnt < tempDtmP->numPoints; ++startPnt)
                {
                if (nodeAddrP(tempDtmP, point)->sPtr == nodeAddrP(tempDtmP, startPnt)->sPtr)
                    {
                    bcdtmWrite_message(0, 0, 0, "Duplicate Points %8ld %8ld ** %8ld %8ld", point, startPnt, nodeAddrP(tempDtmP, point)->sPtr, nodeAddrP(tempDtmP, startPnt)->sPtr);
                    ret = DTM_ERROR;
                    }
                }
            }
        if (ret == DTM_ERROR)
            {
            bcdtmWrite_message(2, 0, 0, "Duplicate Points In Rectangle Infill");
            goto errexit;
            }
        }
    /*
    ** Fill Area External To Old Tin Hull
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Filling Area External To Hull");
    if (bcdtmList_copyDtmFeatureToTptrListDtmObject(tempDtmP, hullFeature, &startPnt)) goto errexit;
    priorPnt = startPnt;
    startPnt = nodeAddrP(tempDtmP, startPnt)->tPtr;
    endPnt = startPnt;
    do
        {
        nextPnt = nodeAddrP(tempDtmP, startPnt)->tPtr;
        cStartPnt = nodeAddrP(tempDtmP, startPnt)->sPtr;
        cNextPnt = nodeAddrP(tempDtmP, nextPnt)->sPtr;
        if ((clkPnt = bcdtmList_nextClkDtmObject(tempDtmP, startPnt, nextPnt)) < 0) goto errexit;
        while (clkPnt != priorPnt)
            {
            cClkPnt = nodeAddrP(tempDtmP, clkPnt)->sPtr;
            if (bcdtmList_insertLineAfterPointDtmObject(dtmP, cStartPnt, cClkPnt, cNextPnt)) goto errexit;
            cNextPnt = cClkPnt;
            if ((clkPnt = bcdtmList_nextClkDtmObject(tempDtmP, startPnt, clkPnt)) < 0) goto errexit;
            }
        priorPnt = startPnt;
        startPnt = nextPnt;
        } while (startPnt != endPnt);
        /*
        ** Null Tptr list
        */
        if (bcdtmList_nullTptrListDtmObject(dtmP, startPnt)) goto errexit;
        /*
        ** Fill Area Internal To Rectangle
        */
        if (dbg) bcdtmWrite_message(0, 0, 0, "Filling Area Internal To Reactangle");
        if (bcdtmList_copyDtmFeatureToTptrListDtmObject(tempDtmP, rectangleFeature, &startPnt)) goto errexit;
        priorPnt = startPnt;
        startPnt = nodeAddrP(tempDtmP, startPnt)->tPtr;
        endPnt = startPnt;
        do
            {
            nextPnt = nodeAddrP(tempDtmP, startPnt)->tPtr;
            cStartPnt = nodeAddrP(tempDtmP, startPnt)->sPtr;
            cPriorPnt = nodeAddrP(tempDtmP, priorPnt)->sPtr;
            if ((clkPnt = bcdtmList_nextClkDtmObject(tempDtmP, startPnt, priorPnt)) < 0) goto errexit;
            while (clkPnt != nextPnt)
                {
                cClkPnt = nodeAddrP(tempDtmP, clkPnt)->sPtr;
                if (bcdtmList_insertLineAfterPointDtmObject(dtmP, cStartPnt, cClkPnt, cPriorPnt)) goto errexit;
                cPriorPnt = cClkPnt;
                if ((clkPnt = bcdtmList_nextClkDtmObject(tempDtmP, startPnt, clkPnt)) < 0) goto errexit;
                }
            priorPnt = startPnt;
            startPnt = nextPnt;
            } while (startPnt != endPnt);
            /*
            ** Null Tptr list
            */
            if (bcdtmList_nullTptrListDtmObject(dtmP, startPnt)) goto errexit;
            /*
           ** Insert Void Feature Around Hull
           */
            if (bcdtmList_setConvexHullDtmObject(dtmP)) goto errexit;
            if (bcdtmList_copyHptrListToTptrListDtmObject(dtmP, dtmP->hullPoint)) goto errexit;
            if (bcdtmInsert_addDtmFeatureToDtmObject(dtmP, nullptr, 0, DTMFeatureType::Void, -9999, dtmP->dtmFeatureIndex, dtmP->hullPoint, 1)) goto errexit;
            ++dtmP->dtmFeatureIndex;
            /*
            ** Scan For Hull Island And Void
            */
            if (cdbg)
                {
                for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
                    {
                    dtmFeatureP = ftableAddrP(dtmP, dtmFeature);
                    if (dtmFeatureP->dtmUserTag == -9999) bcdtmWrite_message(0, 0, 0, "dtmFeatureState = %4ld dtmFeatureId = %8I64d dtmUserTag = %8I64d ** dtmFeatureType = %4ld", dtmFeatureP->dtmFeatureState, dtmFeatureP->dtmFeatureId, dtmFeatureP->dtmUserTag, dtmFeatureP->dtmFeatureType);
                    }
                }
            /*
            ** Clean Dtm
            */
            if (dbg) bcdtmWrite_message(0, 0, 0, "Cleaning DTM Object");
            if (bcdtmList_cleanDtmObject(dtmP)) goto errexit;
            /*
            ** Scan For Hull Island And Void
            */
            if (cdbg)
                {
                for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
                    {
                    dtmFeatureP = ftableAddrP(dtmP, dtmFeature);
                    if (dtmFeatureP->dtmUserTag == -9999) bcdtmWrite_message(0, 0, 0, "dtmFeatureState = %4ld dtmFeatureId = %8I64d dtmUserTag = %8I64d ** dtmFeatureType = %4ld", dtmFeatureP->dtmFeatureState, dtmFeatureP->dtmFeatureId, dtmFeatureP->dtmUserTag, dtmFeatureP->dtmFeatureType);
                    }
                }
            /*
             ** Check Tin
             */
            if (cdbg)
                {
                bcdtmWrite_message(0, 0, 0, "Checking Triangulation");
                if (bcdtmCheck_tinComponentDtmObject(dtmP))
                    {
                    bcdtmWrite_message(2, 0, 0, "Triangulation Invalid");
                    goto errexit;
                    }
                bcdtmWrite_message(0, 0, 0, "Triangulation Valid");
                }
            /*
            ** Write Stats After Adding Rectangle
            */
            if (dbg)
                {
                bcdtmWrite_message(0, 0, 0, "After Adding External Rectangle");
                bcdtmWrite_message(0, 0, 0, "dtmP->numPoints        = %8ld", dtmP->numPoints);
                bcdtmWrite_message(0, 0, 0, "dtmP->numSortedPoints  = %8ld", dtmP->numSortedPoints);
                bcdtmWrite_message(0, 0, 0, "dtmP->numTriangles     = %8ld", dtmP->numTriangles);
                bcdtmWrite_message(0, 0, 0, "dtmP->numLines         = %8ld", dtmP->numLines);
                if (dbg) bcdtmWrite_toFileDtmObject(dtmP, L"padded.tin");
                }
            /*
            ** Clean Up
            */
        cleanup:
            if (hullPtsP != nullptr)
                {
                free(hullPtsP); hullPtsP = nullptr;
                }
            if (tempDtmP != nullptr) bcdtmObject_destroyDtmObject(&tempDtmP);
            /*
            ** Job Completed
            */
            if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Inserting Rectangle Around Tin Completed");
            if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Inserting Rectangle Around Tin Error");
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
DTMStatusInt bcdtmFormatInroads_getInroadsTriangleNumberDtmObject
(
BC_DTM_OBJ       *dtmP,
DTM_MX_TRG_INDEX *trgIndexP,
long             trgPnt1,
long             trgPnt2,
long             trgPnt3,
long             *trgNumP
)
/*
** This Function Finds The Entry In The Triangle Index For Triangle <trgPnt1,trgPnt2,trgPnt3>
** And Returns The Triangle Number
**
** Note trgPnt1,trgPnt2,trgPnt3 Must Be In A Clockwise Direction
**
*/
    {
    DTMStatusInt ret = DTM_SUCCESS;
    long    sp, process;
    DTM_MX_TRG_INDEX *bIndexP, *tIndexP;
    /*
    ** Initialise
    */
    *trgNumP = dtmP->nullPnt;
    while (trgPnt1 > trgPnt2 || trgPnt1 > trgPnt3)
        {
        sp = trgPnt1; trgPnt1 = trgPnt2; trgPnt2 = trgPnt3; trgPnt3 = sp;
        }
    /*
    ** Get First Entry For trgPnt1
    */
    bIndexP = trgIndexP + (trgIndexP + trgPnt1)->index;
    tIndexP = trgIndexP + dtmP->numTriangles;
    /*
    ** Scan trgPnt1 Entries Looking For trgPnt2 && trgPnt3
    */
    process = 1;
    while (bIndexP < tIndexP && process && *trgNumP == dtmP->nullPnt)
        {
        if (bIndexP->trgPnt1 != trgPnt1) process = 0;
        else
            {
            if (bIndexP->trgPnt2 == trgPnt2 && bIndexP->trgPnt3 == trgPnt3)
                {
                *trgNumP = (long)(bIndexP - trgIndexP);
                }
            ++bIndexP;
            }
        }
    /*
    ** If Triangle Not Found Error Exit
    */
    if (*trgNumP == dtmP->nullPnt) goto errexit;
    /*
    ** Cleanup
    */
cleanup:
    /*
    ** Job Completed
    */
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
DTMStatusInt bcdtmFormatInroads_loadFeaturesFromDtmObject
(
BC_DTM_OBJ *dtmP,
DTMFeatureId hullFeatureId,
tinFeaturesCallBackFunctionDef loadFunctionP
)
/*
** This Function Loads Inroads Tin Features From A DTM Object
*/
    {
    DTMStatusInt   ret = DTM_SUCCESS;
    long  *indexP, *pointIndicesP = nullptr, memPointIndices = 1000, memPointIndicesInc = 1000;
    long  point, startPoint, dtmFeature, numPoints, closeFlag;
    DTMFeatureType dtmFeatureType;
    BC_DTM_FEATURE *dtmFeatureP;
    /*
    ** Allocate Initial Memory For Storing The Point Indices
    */
    pointIndicesP = (long *)malloc(memPointIndices * sizeof(long));
    if (pointIndicesP == nullptr)
        {
        bcdtmWrite_message(1, 0, 0, "Memory Allocation Failure");
        goto errexit;
        }
    /*
    ** Scan DTM Features
    */
    for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
        {
        dtmFeatureP = ftableAddrP(dtmP, dtmFeature);
        if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin)
            {
            /*
            **     Assign Feature Type
            */
            dtmFeatureType = dtmFeatureP->dtmFeatureType;
            /*
            **     Check For Island representing origional tin hull
            */
            if (dtmFeatureP->dtmFeatureId == hullFeatureId)
                {
                dtmFeatureType = DTMFeatureType::Hull;
                dtmFeatureP->dtmUserTag = dtmP->nullUserTag;
                }
            /*
            **     Check For Void Representing InRoads Bounding Rectangle
            */
            if (dtmFeature == dtmP->numFeatures - 1)
                {
                dtmFeatureType = DTMFeatureType::InroadsRectangle;
                dtmFeatureP->dtmUserTag = dtmP->nullUserTag;
                }
            /*
            **     Skip unsupported feature types)
            */
            if (dtmFeatureType != DTMFeatureType::InroadsRectangle && dtmFeatureType != DTMFeatureType::GroupSpots && dtmFeatureType != DTMFeatureType::Breakline &&
                dtmFeatureType != DTMFeatureType::ContourLine && dtmFeatureType != DTMFeatureType::Hull && dtmFeatureType != DTMFeatureType::Void && dtmFeatureType != DTMFeatureType::Island &&
                dtmFeatureType != DTMFeatureType::Hole && dtmFeatureType != DTMFeatureType::Polygon && dtmFeatureType != DTMFeatureType::ZeroSlopePolygon)
                continue;
            /*
            **     Count Number Of Feature Points
            */
            if (bcdtmList_countNumberOfDtmFeaturePointsDtmObject(dtmP, dtmFeature, &numPoints, &closeFlag)) goto errexit;
            /*
            **     Check Memory
            */
            if (numPoints > memPointIndices)
                {
                memPointIndices = memPointIndices + memPointIndicesInc;
                if (memPointIndices < numPoints) memPointIndices = numPoints;
                pointIndicesP = (long *)realloc(pointIndicesP, memPointIndices * sizeof(long));
                if (pointIndicesP == nullptr)
                    {
                    bcdtmWrite_message(1, 0, 0, "Memory Allocation Failure");
                    goto errexit;
                    }
                }
            /*
            **     Copy Feature Points To Index Array
            */
            indexP = pointIndicesP;
            startPoint = point = dtmFeatureP->dtmFeaturePts.firstPoint;
            do
                {
                *indexP = point;
                ++indexP;
                if (bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP, dtmFeature, point, &point)) goto errexit;
                } while (point != dtmP->nullPnt && point != startPoint);
                if (closeFlag) *indexP = *pointIndicesP;
                /*
                **     Call Inroads Call Back Function
                */
                if (loadFunctionP((long)dtmFeatureType, (__int64)dtmFeatureP->dtmUserTag, (__int64)dtmFeatureP->dtmFeatureId, pointIndicesP, numPoints)) goto errexit;
            }
        }
    /*
    ** Cleanup
    */
cleanup:
    if (pointIndicesP != nullptr)
        {
        free(pointIndicesP); pointIndicesP = nullptr;
        }
    /*
    ** Job Completed
    */
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

DTMStatusInt bcdtmClip_toTptrPolygonLeavingFeaturesDtmObject(BC_DTM_OBJ *dtmP, long startPnt, DTMClipOption clipOption)
    {
    DTMStatusInt    ret = DTM_SUCCESS;
    int dbg = 0, cdbg = 0;
    DTMDirection direction;//,dtmFeature ;
    double area;
    // BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "Clipping Dtm Object To Tptr Polygon");
        bcdtmWrite_message(0, 0, 0, "dtmP       = %p", dtmP);
        bcdtmWrite_message(0, 0, 0, "startPnt   = %8ld", startPnt);
        bcdtmWrite_message(0, 0, 0, "clipOption = %8ld", clipOption);
        }
    /*
    ** Test For Valid Clip Option
    */
    if (clipOption != DTMClipOption::Internal && clipOption != DTMClipOption::External)
        {
        bcdtmWrite_message(2, 0, 0, "Invalid Clip Option");
        goto errexit;
        }
    /*
    ** Test For Valid Dtm  Object
    */
    if (bcdtmObject_testForValidDtmObject(dtmP)) goto errexit;
    /*
    ** Check If DTM Is In Tin State
    */
    if (dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message(2, 0, 0, "DTM Object %p Not Triangulated", dtmP);
        goto errexit;
        }
    /*
    ** Check For Valid Tptr Polygon Start Point
    */
    if (startPnt < 0 || startPnt >= dtmP->numPoints || nodeAddrP(dtmP, startPnt)->tPtr < 0 || nodeAddrP(dtmP, startPnt)->tPtr >= dtmP->numPoints)
        {
        bcdtmWrite_message(2, 0, 0, "Invalid Start Point For Clip Tptr Polygon");
        goto errexit;
        }
    /*
    ** Check Connectivity Of Tptr Polygon - Development Only
    */
    if (cdbg)
        {
        if (dbg) bcdtmWrite_message(0, 0, 0, "Checking Connectivity Tptr Polygon");
        if (bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP, startPnt, 0)) goto errexit;
        }
    /*
    ** Check Direction Of Tptr Polygon And If Clockwise Set Direction Anti Clockwise
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Checking Direction Tptr Polygon");
    if (bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP, startPnt, &area, &direction)) goto errexit;
    if (dbg) bcdtmWrite_message(0, 0, 0, "Tptr Polygon Area = %15.4lf Direction = %2ld", area, direction);
    if (direction == DTMDirection::Clockwise)
        {
        if (bcdtmList_reverseTptrPolygonDtmObject(dtmP, startPnt)) goto errexit;
        }
    /*
    ** Check Integrity Of Dtm Object - Development Only
    */
    if (cdbg)
        {
        if (dbg) bcdtmWrite_message(0, 0, 0, "Checking Dtm");
        if (bcdtmCheck_tinComponentDtmObject(dtmP))
            {
            bcdtmWrite_message(0, 0, 0, "DTM Corrupted"); goto errexit;
            }
        else                                          bcdtmWrite_message(0, 0, 0, "DTM OK");
        }
    /*
    ** Clip Internal To Tptr Polygon
    */
    if (clipOption == DTMClipOption::Internal)
        {
        if (dbg) bcdtmWrite_message(0, 0, 0, "Clipping Dtm Object Internal");
        if (bcdtmClip_internalToTptrPolygonDtmObject(dtmP, startPnt, 1)) goto errexit;
        }
    /*
    ** Clip External To Tptr Polygon
    */
    if (clipOption == DTMClipOption::External)
        {
        if (dbg) bcdtmWrite_message(0, 0, 0, "Clipping Dtm Object External");
        if (bcdtmClip_externalToTptrPolygonDtmObject(dtmP, startPnt)) goto errexit;
        }
    /*
    ** Clean Dtm Object
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Cleaning Dtm Object ** dtmP->numFeatures = %8ld", dtmP->numFeatures);
    if (bcdtmList_cleanDtmObject(dtmP)) goto errexit;
    if (dbg) bcdtmWrite_message(0, 0, 0, "Cleaning Dtm Object Completed ** dtmP->numFeatures = %8ld", dtmP->numFeatures);
    /*
    ** Check Integrity Of Dtm Object - Development Only
    */
    if (cdbg)
        {
        bcdtmWrite_message(0, 0, 0, "Checking Clipped Dtm");
        if (bcdtmCheck_tinComponentDtmObject(dtmP))
            {
            bcdtmWrite_message(0, 0, 0, "DTM Corrupted After Clip");
            goto errexit;
            }
        else bcdtmWrite_message(0, 0, 0, "DTM OK");
        }
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Clipping Dtm Object To Tptr Polygon Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Clipping Dtm Object To Tptr Polygon Error");
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
DTMStatusInt bcdtmFormatInroads_clipUsingIslandFeatureIdDtmObject
(
BC_DTM_OBJ       *dtmP,
DTMFeatureId   dtmFeatureId  // Feature Id Of An Island Feature Used To Store The Old Tin Hull
)
/*
** This Is A Special Purpose Clean Up Function That Is Called After Exporting Inroads Triangles
** It Removes The External Void Triangles That Are Created To Simulate An Inroads Triangulation
**
*/
    {
    DTMStatusInt     ret = DTM_SUCCESS;
    int dbg = 0;
    long    dtmFeature, hullFeature, startPnt;
    BC_DTM_FEATURE *dtmFeatureP;
    /*
    ** Write Entry Message
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Clipping DTM To Island Feature Id = %10I64d", dtmFeatureId);
    /*
    ** Test For Valid Dtm Object
    */
    if (bcdtmObject_testForValidDtmObject(dtmP)) goto errexit;
    /*
    ** Check DTM Is In Triangulated State
    */
    if (dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message(2, 0, 0, "Method Requires Triangulated Dtm");
        goto errexit;
        }
    /*
    ** Write Stats Prior To Clipping
    */
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "Before Clipping To Island Feature");
        bcdtmWrite_message(0, 0, 0, "dtmP->numPoints        = %8ld", dtmP->numPoints);
        bcdtmWrite_message(0, 0, 0, "dtmP->numSortedPoints  = %8ld", dtmP->numSortedPoints);
        bcdtmWrite_message(0, 0, 0, "dtmP->numTriangles     = %8ld", dtmP->numTriangles);
        bcdtmWrite_message(0, 0, 0, "dtmP->numLines         = %8ld", dtmP->numLines);
        bcdtmWrite_message(0, 0, 0, "dtmP->numFeatures      = %8ld", dtmP->numFeatures);
        }
    /*
    ** Scan For Hull Feature
    */
    hullFeature = dtmP->nullPnt;
    for (dtmFeature = dtmP->numFeatures - 1; dtmFeature >= 0 && hullFeature == dtmP->nullPnt; --dtmFeature)
        {
        dtmFeatureP = ftableAddrP(dtmP, dtmFeature);
        if (dtmFeatureP->dtmFeatureId == dtmFeatureId && dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin)
            {
            hullFeature = dtmFeature;
            }
        }
    /*
    ** Check Hull Feature Found
    */
    if (hullFeature == dtmP->nullPnt)
        {
        bcdtmWrite_message(2, 0, 0, "No Island Feature With Required Feature Id Found");
        goto errexit;
        }
    /*
    ** Copy Feature Points To Tptr List
    */
    if (bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP, hullFeature, &startPnt)) goto errexit;
    /*
    ** Remove Island Feature
    */
    if (bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP, hullFeature)) goto errexit;
    /*
    ** Clip DTM To Tptr Polygon
    */
    if (bcdtmClip_toTptrPolygonLeavingFeaturesDtmObject(dtmP, startPnt, DTMClipOption::External)) goto errexit;
    /*
    ** Write Stats After Clipping
    */
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "After Clipping To Island Feature");
        bcdtmWrite_message(0, 0, 0, "dtmP->numPoints        = %8ld", dtmP->numPoints);
        bcdtmWrite_message(0, 0, 0, "dtmP->numSortedPoints  = %8ld", dtmP->numSortedPoints);
        bcdtmWrite_message(0, 0, 0, "dtmP->numTriangles     = %8ld", dtmP->numTriangles);
        bcdtmWrite_message(0, 0, 0, "dtmP->numLines         = %8ld", dtmP->numLines);
        bcdtmWrite_message(0, 0, 0, "dtmP->numFeatures      = %8ld", dtmP->numFeatures);
        }
    /*
    ** Cleanup
    */
cleanup:
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Clipping DTM To Island Feature Id Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Clipping DTM To Island Feature Id Error");
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
DTMStatusInt bcdtmFormatInroads_loadTrianglesFromDtmObject
(
BC_DTM_OBJ *dtmP,
int(*loadFunctionP)(long trgNum, long trgPnt1, long trgPnt2, long trgPnt3, long voidTriangle, long side1Trg, long side2Trg, long side3Trg)
)
/*
** This Function Loads Inroads Triangles From A DTM Object
*/
    {
    DTMStatusInt  ret = DTM_SUCCESS;
    int dbg = 0;
    long p1, p2, p3, clc, numTriangles;
    bool voidTriangle;
    long trgNum, side1Trg, side2Trg, side3Trg;
    DTM_CIR_LIST *clistP;
    DTM_MX_TRG_INDEX *indexP, *trgIndexP = nullptr;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "Loading Inroads Triangles From BcCivil DTM");
        bcdtmWrite_message(0, 0, 0, "dtmP           = %p", dtmP);
        bcdtmWrite_message(0, 0, 0, "loadFunctionP  = %p", loadFunctionP);
        }
    /*
    ** Test For Valid Dtm Object
    */
    if (bcdtmObject_testForValidDtmObject(dtmP)) goto errexit;
    /*
    ** Check DTM Is In Triangulated State
    */
    if (dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message(2, 0, 0, "Method Requires Triangulated Dtm");
        goto errexit;
        }
    /*
    ** Check Load Function Is Set
    */
    if (loadFunctionP == nullptr)
        {
        bcdtmWrite_message(1, 0, 0, "Load Function Not Set");
        goto errexit;
        }
    /*
    ** Allocate Memory For Triangle Index
    */
    trgIndexP = (DTM_MX_TRG_INDEX *)malloc(dtmP->numTriangles * sizeof(DTM_MX_TRG_INDEX));
    if (trgIndexP == nullptr)
        {
        bcdtmWrite_message(1, 0, 0, "Memory Allocation Failure");
        goto errexit;
        }
    /*
    ** Populate Triangle Index
    */
    numTriangles = 0;
    indexP = trgIndexP;
    for (p1 = 0; p1 < dtmP->numPoints; ++p1)
        {
        (trgIndexP + p1)->index = numTriangles;
        if (nodeAddrP(dtmP, p1)->cPtr != dtmP->nullPtr)
            {
            clc = nodeAddrP(dtmP, p1)->cPtr;
            if ((p2 = bcdtmList_nextAntDtmObject(dtmP, p1, clistAddrP(dtmP, clc)->pntNum)) < 0) return DTM_ERROR;
            while (clc != dtmP->nullPtr)
                {
                clistP = clistAddrP(dtmP, clc);
                p3 = clistP->pntNum;
                clc = clistP->nextPtr;
                if (p2 > p1 && p3 > p1 && nodeAddrP(dtmP, p3)->hPtr != p1)
                    {
                    if (numTriangles >= dtmP->numTriangles)
                        {
                        bcdtmWrite_message(2, 0, 0, "Too Many Index Triangles %6ld of %6ld ", numTriangles, dtmP->numTriangles);
                        goto errexit;
                        }
                    indexP->trgPnt1 = p1;
                    indexP->trgPnt2 = p2;
                    indexP->trgPnt3 = p3;
                    ++indexP;
                    ++numTriangles;
                    }
                p2 = p3;
                }
            }
        }
    /*
    ** Check Index Size
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "numTriangles = %8ld ** dtmP->numTriangles = %8ld", numTriangles, dtmP->numTriangles);
    if (numTriangles != dtmP->numTriangles)
        {
        bcdtmWrite_message(1, 0, 0, "Triangle Index Size Incorrect");
        goto errexit;
        }
    /*
    ** Scan Triangle Index And Load Triangles
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Loading Inroads Triangles");
    for (trgNum = 0, indexP = trgIndexP; indexP < trgIndexP + numTriangles; ++indexP, ++trgNum)
        {
        /*
        **  Get Adjancies For Each Triangle Side
        */
        side1Trg = -1;
        if (nodeAddrP(dtmP, indexP->trgPnt2)->hPtr != indexP->trgPnt1)
            {
            if ((p1 = bcdtmList_nextClkDtmObject(dtmP, indexP->trgPnt2, indexP->trgPnt1)) < 0) goto errexit;
            if (bcdtmFormatInroads_getInroadsTriangleNumberDtmObject(dtmP, trgIndexP, p1, indexP->trgPnt2, indexP->trgPnt1, &side1Trg))
                {
                bcdtmWrite_message(1, 0, 0, "Failed To Get Adjacency For Side1");
                goto errexit;
                }
            }
        side2Trg = -1;
        if (nodeAddrP(dtmP, indexP->trgPnt3)->hPtr != indexP->trgPnt2)
            {
            if ((p1 = bcdtmList_nextClkDtmObject(dtmP, indexP->trgPnt3, indexP->trgPnt2)) < 0) goto errexit;
            if (bcdtmFormatInroads_getInroadsTriangleNumberDtmObject(dtmP, trgIndexP, p1, indexP->trgPnt3, indexP->trgPnt2, &side2Trg))
                {
                bcdtmWrite_message(1, 0, 0, "Failed To Get Adjacency For Side2");
                goto errexit;
                }
            }
        side3Trg = -1;
        if (nodeAddrP(dtmP, indexP->trgPnt1)->hPtr != indexP->trgPnt3)
            {
            if ((p1 = bcdtmList_nextClkDtmObject(dtmP, indexP->trgPnt1, indexP->trgPnt3)) < 0) goto errexit;
            if (bcdtmFormatInroads_getInroadsTriangleNumberDtmObject(dtmP, trgIndexP, p1, indexP->trgPnt1, indexP->trgPnt3, &side3Trg))
                {
                bcdtmWrite_message(1, 0, 0, "Failed To Get Adjacency For Side3");
                goto errexit;
                }
            }
        /*
        **  Test For Void triangles
        */
        if (bcdtmList_testForVoidTriangleDtmObject(dtmP, indexP->trgPnt1, indexP->trgPnt2, indexP->trgPnt3, voidTriangle)) goto errexit;
        /*
        **  Call Load Function
        */
        if (loadFunctionP(trgNum, indexP->trgPnt1, indexP->trgPnt2, indexP->trgPnt3, voidTriangle, side1Trg, side2Trg, side3Trg)) goto errexit;
        }
    /*
    ** Clean Up
    */
cleanup:
    if (trgIndexP != nullptr)
        {
        free(trgIndexP); trgIndexP = nullptr;
        }
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Loading Inroads Triangles From BcCivil DTM Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Loading Inroads Triangles From BcCivil DTM Error");
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
DTMStatusInt bcdtmFormatInroads_importTinFromDtmObject
(
BC_DTM_OBJ *dtmP,
tinStatsCallBackFunctionDef tinStatsCallBackFunctionP,
tinRandomPointsCallBackFunctionDef tinRandomPointsCallBackFunctionP,
tinFeaturePointsCallBackFunctionDef tinFeaturePointsCallBackFunctionP,
tinTrianglesCallBackFunctionDef tinTrianglesCallBackFunctionP,
tinFeaturesCallBackFunctionDef tinFeaturesCallBackFunctionP)
    {
    DTMStatusInt ret = DTM_SUCCESS;
    int dbg = 0;
    long node, numRandomPoints = 0, numFeaturePoints = 0;
    double xinc, xdec, ydec, yinc, xrange, yrange;
    DTM_TIN_NODE *nodeP;
    DPoint3d *pntP;
    DTMFeatureId islandFeatureId;
    /*
    ** Write Entry Message
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Importing Tin From Dtm Object");
    /*
    ** Test For Valid Dtm Object
    */
    if (bcdtmObject_testForValidDtmObject(dtmP)) goto errexit;
    /*
    ** Check DTM Is In Triangulated State
    */
    if (dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message(2, 0, 0, "Method Requires Triangulated Dtm");
        goto errexit;
        }
    /*
    ** Place Rectangle Around Tin 5% larger than the Tin Hull bounding rectangle
    */
    xrange = (dtmP->xMax - dtmP->xMin) / 20.0;
    yrange = (dtmP->yMax - dtmP->yMin) / 20.0;
    xdec = xrange;
    ydec = yrange;
    xinc = xrange;
    yinc = yrange;
    if (dbg) bcdtmWrite_message(0, 0, 0, "Insert Rectangle Around Tin And Fill With Triangles");
    if (bcdtmFormatInroads_insertRectangleAroundTinDtmObject(dtmP, xdec, xinc, ydec, yinc, &islandFeatureId)) goto errexit;
    /*
    ** Count Number Of Random And Feature Points
    */
    for (node = 0; node < dtmP->numNodes; ++node)
        {
        nodeP = nodeAddrP(dtmP, node);
        if (nodeP->fPtr == dtmP->nullPtr) ++numRandomPoints;
        else                               ++numFeaturePoints;
        }
    if (dbg) bcdtmWrite_message(0, 0, 0, "numRandomPoints = %8ld numFeaturePoints = %8ld", numRandomPoints, numFeaturePoints);
    /*
    ** Adjust Point Counts For Inroads Bounding Rectangle Void Feature
    */
    numRandomPoints = numRandomPoints + 4;
    numFeaturePoints = numFeaturePoints - 4;
    if (numFeaturePoints < 0) numFeaturePoints = 0;
    /*
    ** Call Inroads Tin Stats Function
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Calling InRoads Tin Stats Call Back Function");
    if (tinStatsCallBackFunctionP(numRandomPoints, numFeaturePoints, dtmP->numTriangles, dtmP->numFeatures)) goto errexit;
    /*
    ** Call InRoads Random Points Function
    */
    numRandomPoints = 0;
    if (dbg) bcdtmWrite_message(0, 0, 0, "Calling InRoads Random Points Function");
    for (node = 0; node < dtmP->numNodes; ++node)
        {
        nodeP = nodeAddrP(dtmP, node);
        if (nodeP->fPtr == dtmP->nullPtr)
            {
            pntP = pointAddrP(dtmP, node);
            if (tinRandomPointsCallBackFunctionP(node, pntP->x, pntP->y, pntP->z)) goto errexit;
            ++numRandomPoints;
            }
        else                  // Check For Inroads Bounding Rectangle Void
            {
            if (flistAddrP(dtmP, nodeP->fPtr)->dtmFeature == dtmP->numFeatures - 1)
                {
                pntP = pointAddrP(dtmP, node);
                if (tinRandomPointsCallBackFunctionP(node, pntP->x, pntP->y, pntP->z)) goto errexit;
                ++numRandomPoints;
                }
            }
        }
    if (dbg) bcdtmWrite_message(0, 0, 0, "Number Of Random Points Written = %8ld", numRandomPoints);
    /*
    ** Call InRoads Feature Points Function
    */
    numFeaturePoints = 0;
    if (dbg) bcdtmWrite_message(0, 0, 0, "Calling InRoads Feature Points Call Back Function");
    for (node = 0; node < dtmP->numNodes; ++node)
        {
        nodeP = nodeAddrP(dtmP, node);
        if (nodeP->fPtr != dtmP->nullPtr) // Check For Inroads Bounding Rectangle Void
            {
            if (flistAddrP(dtmP, nodeP->fPtr)->dtmFeature != dtmP->numFeatures - 1)
                {
                pntP = pointAddrP(dtmP, node);
                if (tinFeaturePointsCallBackFunctionP(node, pntP->x, pntP->y, pntP->z)) goto errexit;
                ++numFeaturePoints;
                }
            }
        }
    if (dbg) bcdtmWrite_message(0, 0, 0, "Number Of Feature Points Written = %8ld", numFeaturePoints);
    /*
    ** Call Inroads Triangle Function
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Calling InRoads Triangles Call Back Function");
    if (bcdtmFormatInroads_loadTrianglesFromDtmObject(dtmP, tinTrianglesCallBackFunctionP)) goto errexit;
    /*
    ** Call Inroads Feature Function
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Calling InRoads Features Call Back Function");
    if (bcdtmFormatInroads_loadFeaturesFromDtmObject(dtmP, islandFeatureId, tinFeaturesCallBackFunctionP)) goto errexit;
    /*
    ** Remove InRoad's simulated triangles
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Removing Simulated Inroads Triangles");
    if (bcdtmFormatInroads_clipUsingIslandFeatureIdDtmObject(dtmP, islandFeatureId)) goto errexit;
    /*
    ** Clean Up
    */
cleanup:
    /*
    **	Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Importing Tin From Dtm Object Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Importing Tin From Dtm Object Error");
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
int bcdtmFormatInroads_importGeopakTinFromFile
(
WCharCP tinFileNameP,
tinStatsCallBackFunctionDef tinStatsCallBackFunctionP,
tinRandomPointsCallBackFunctionDef tinRandomPointsCallBackFunctionP,
tinFeaturePointsCallBackFunctionDef tinFeaturePointsCallBackFunctionP,
tinTrianglesCallBackFunctionDef tinTrianglesCallBackFunctionP,
tinFeaturesCallBackFunctionDef tinFeaturesCallBackFunctionP
)
    {
    DTMStatusInt ret = DTM_SUCCESS;
    int dbg = 0;
    BC_DTM_OBJ *dtmP = nullptr;
    /*
    ** Write Entry Message
    */
    if (dbg) bcdtmWrite_message(0, 0, 0, "Importing From Geopak Tin File");
    /*
    ** Read Geopak Tin File To bcLIB DTM
    */
    dtmP = glbDtmP;
    /*
    ** Import From Dtm Object
    */
    if (bcdtmFormatInroads_importTinFromDtmObject(dtmP, tinStatsCallBackFunctionP, tinRandomPointsCallBackFunctionP, tinFeaturePointsCallBackFunctionP, tinTrianglesCallBackFunctionP, tinFeaturesCallBackFunctionP)) goto errexit;
    /*
    ** Clean Up
    */
cleanup:
    /*
    **	Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Importing From Geopak Tin File Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Importing From Geopak Tin File Error");
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
BENTLEYDTMFORMATS_EXPORT DTMStatusInt bcdtmFormatInroads_exportBclibDtmToInroadsDtmFile(BC_DTM_OBJ *dtmP, const wchar_t *dtmFileNameP, const wchar_t *nameP, const wchar_t *descriptionP)
    {
    DTMStatusInt ret = DTM_SUCCESS;
    int status = 0, dbg = 0;
    wchar_t const*w_tinFileNameP = L"xxxx";
    /*
    **  Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message(0, 0, 0, "Exporting bcLIB DTM To Inroads DTM File");
        bcdtmWrite_message(0, 0, 0, "dtmP           = %p", dtmP);
        bcdtmWrite_message(0, 0, 0, "dtmFileNameP   = %s", dtmFileNameP);
        bcdtmWrite_message(0, 0, 0, "nameP          = %s", nameP);
        bcdtmWrite_message(0, 0, 0, "descriptionP   = %s", descriptionP);
        }
    /*
    ** Check For Valid Dtm Object
    */
    if (bcdtmObject_testForValidDtmObject(dtmP)) goto errexit;
    /*
    **  Check If DTM Is In Tin State
    */
    if (dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message(2, 0, 0, "Method Requires Triangulated DTM");
        goto errexit;
        }
    /*
    **  Call Inroads Function To Initiate Import Of A Geopak Tin File
    **  For The Purpose Of Importing Directly To An In Memory bcLIB DTM Object
    **  We Will Use A Dummy Tin File Name And Pass The Pointer To The DTM
    **
    */
    glbDtmP = dtmP;
    status = inroadsTM_setConvertGPKTinToDTMFunction(bcdtmFormatInroads_importGeopakTinFromFile);
    if (status < 0)
        {
        bcdtmWrite_message(1, 0, 0, "Failed To Load InRoads DLLs **");
        goto errexit;
        }
    // if( ( status = dtmLink_convertGPKTinToDTM(w_tinFileNameP, w_dtmFileNameP, w_nameP, w_descriptionP)) != DTM_SUCCESS )
    if ((status = inroadsTM_convertGPKTinToDTM (w_tinFileNameP, dtmFileNameP, nameP, descriptionP, 0)) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1, 0, 0, "Failed To Export Inroads DTM File ** Inroads Error = %6ld", status);
        goto errexit;
        }
    /*
    ** Clean Up
    */
cleanup:
    /*
    **	Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Exporting bcLIB DTM To Inroads DTM File Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0, 0, 0, "Exporting bcLIB DTM To Inroads DTM File Error");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }
