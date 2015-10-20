/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainagePond.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcdtmDrainage.h"
#include <TerrainModel/Core/bcdtmInlines.h>

//#pragma optimize('p', on )

int DrainageDebug=0 ;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_calculatePondDtmObject
(
BC_DTM_OBJ         *dtmP,
double             x,
double             y,
double             falseLowDepth,
DTMFeatureCallback loadFunctionP,
bool               drawPond,
bool*              pondDeterminedP,
double*            pondElevationP,
double*            pondDepthP,
double*            pondAreaP,
double*            pondVolumeP,
void*              userP
)
    {
    return bcdtmDrainage_calculatePondWithLowPointDtmObject (dtmP, x, y, falseLowDepth, loadFunctionP, drawPond, pondDeterminedP, pondElevationP, pondAreaP, pondAreaP, pondVolumeP, nullptr, userP);
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_calculatePondWithLowPointDtmObject
(
BC_DTM_OBJ         *dtmP,
double             x,
double             y,
double             falseLowDepth,
DTMFeatureCallback loadFunctionP,
bool               drawPond,
bool*              pondDeterminedP,
double*            pondElevationP,
double*            pondDepthP,
double*            pondAreaP,
double*            pondVolumeP,
DPoint3d*          lowPtP,
void*              userP
)
/*
** This Function Determines A Pond About A Low Point And Returns The Pond Volume And Elevation
*/
    {
    int       ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE(0), cdbg = DTM_CHECK_VALUE(0), tdbg = DTM_TIME_VALUE(0);
    long      p1, p2, p3, lp1, lp2, trace, fndType, onHullLine, zeroSlopeOption = 2;
    bool inVoid;
    long      exitPoint, priorPoint, nextPoint, numSumpLines = 0, numPolyPts, startTime = bcdtmClock ();
    double    z, cut, fill, balance, area;
    DPoint3d          *polyPtsP = nullptr;
    DTMDirection      polyDirection;
    DTM_POLYGON_OBJ   *polygonP = nullptr;
    DTM_SUMP_LINES    *sumpLinesP = nullptr;
    BC_DTM_OBJ        *pondDtmP = nullptr;
    DTMDrainageTables *drainageTablesP = nullptr;
    static int        pondCount = -1;
    /*
    ** Write Arguements For Development Purposes
    */
    ++pondCount;
    /*
     if( pondCount != 24 ) return(DTM_SUCCESS) ;
     bcdtmWrite_message(0,0,0,"Calculating Pond ** pondCount = %8d",pondCount) ;
     dbg=1 ;
     */
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Calculating Pond");
        bcdtmWrite_message (0, 0, 0, "dtmP          = %p", dtmP);
        bcdtmWrite_message (0, 0, 0, "x             = %12.5lf", x);
        bcdtmWrite_message (0, 0, 0, "y             = %12.5lf", y);
        bcdtmWrite_message (0, 0, 0, "falseLowDepth = %12.5lf", falseLowDepth);
        bcdtmWrite_message (0, 0, 0, "drawPond      = %2ld", drawPond);
        }
    /*
    ** Initialise
    */
    *pondDeterminedP = false;
    if (pondVolumeP) *pondVolumeP = 0.0;
    if (pondDepthP) *pondDepthP = 0.0;
    if (pondElevationP) *pondElevationP = 0.0;
    if (pondAreaP) *pondAreaP = 0.0;
    p1 = p2 = p3 = dtmP->nullPnt;
    /*
    ** Check For Valid DTM Object
    */
    if (bcdtmObject_testForValidDtmObject (dtmP)) goto errexit;
    /*
    ** Check DTM Is Triangulated
    */
    if (dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message (1, 0, 0, "Method Requires Triangulated DTM");
        goto errexit;
        }
    /*
    ** Check DTM
    */
    if (cdbg)
        {
        bcdtmWrite_message (0, 0, 0, "Checking Tin");
        if (bcdtmCheck_tinComponentDtmObject (dtmP))
            {
            bcdtmWrite_message (0, 0, 0, "Tin Invalid");
            goto errexit;
            }
        bcdtmWrite_message (0, 0, 0, "Tin Valid");
        }
    /*
    ** Find Triangle For Point
    */
    if (bcdtmFind_triangleForPointDtmObject (dtmP, x, y, &z, &fndType, &p1, &p2, &p3)) goto errexit;
    if (fndType == 0)
        {
        bcdtmWrite_message (1, 0, 0, "Start Point External To Tin");
        goto errexit;
        }
    /*
    ** Check Point To Point Tolerance
    */
    if (bcdtmMath_distance (x, y, pointAddrP (dtmP, p1)->x, pointAddrP (dtmP, p1)->y) <= dtmP->ppTol) { fndType = 1; p2 = p3 = dtmP->nullPnt; }
    if (p2 != dtmP->nullPnt) if (bcdtmMath_distance (x, y, pointAddrP (dtmP, p2)->x, pointAddrP (dtmP, p2)->y) <= dtmP->ppTol) { fndType = 1; p1 = p2; p2 = p3 = dtmP->nullPnt; }
    if (p3 != dtmP->nullPnt) if (bcdtmMath_distance (x, y, pointAddrP (dtmP, p3)->x, pointAddrP (dtmP, p3)->y) <= dtmP->ppTol) { fndType = 1; p1 = p3; p2 = p3 = dtmP->nullPnt; }
    /*
    ** Test For Point In Void
    */
    inVoid = false;
    if (fndType == 1 && bcdtmFlag_testVoidBitPCWD (&nodeAddrP (dtmP, p1)->PCWD)) inVoid = true;
    if (fndType == 2 || fndType == 3)  if (bcdtmList_testForVoidLineDtmObject (dtmP, p1, p2, inVoid)) goto errexit;
    if (fndType == 4) if (bcdtmList_testForVoidTriangleDtmObject (dtmP, p1, p2, p3, inVoid)) goto errexit;
    if (inVoid)
        {
        bcdtmWrite_message (1, 0, 0, "Start Point In Void");
        goto errexit;
        }
    /*
    ** Test For Flat Triangle
    */
    trace = 1;
    if (p2 != dtmP->nullPnt && p3 != dtmP->nullPnt)
        {
        if (pointAddrP (dtmP, p1)->z == pointAddrP (dtmP, p2)->z && pointAddrP (dtmP, p1)->z == pointAddrP (dtmP, p3)->z)
            {
            if (dbg) bcdtmWrite_message (0, 0, 0, "Start Point In Flat Triangle");
            trace = 0;
            nodeAddrP (dtmP, p1)->tPtr = p3;
            nodeAddrP (dtmP, p3)->tPtr = p2;
            nodeAddrP (dtmP, p2)->tPtr = p1;
            if (bcdtmDrainage_expandPondToExitPointDtmObject (dtmP, nullptr, nullptr, nullptr, p1, &exitPoint, &priorPoint, &nextPoint)) goto errexit;
            if (exitPoint == dtmP->nullPnt)
                {
                bcdtmList_nullTptrValuesDtmObject (dtmP);
                bcdtmList_nullSptrValuesDtmObject (dtmP);
                bcdtmWrite_message (1, 0, 0, "Exit Point For Zero Slope Pond Not Found");
                goto errexit;
                }
            if (dbg)
                {
                bcdtmWrite_message (0, 0, 0, "Exit Point = %8ld ** %12.5lf %12.5lf %10.4lf", exitPoint, pointAddrP (dtmP, exitPoint)->x, pointAddrP (dtmP, exitPoint)->y, pointAddrP (dtmP, exitPoint)->z);
                bcdtmWrite_message (0, 0, 0, "P1         = %8ld ** %12.5lf %12.5lf %10.4lf", p1, pointAddrP (dtmP, p1)->x, pointAddrP (dtmP, p1)->y, pointAddrP (dtmP, p1)->z);
                }
            if (pointAddrP (dtmP, exitPoint)->z == pointAddrP (dtmP, p1)->z)
                {
                trace = 1;
                p1 = exitPoint;
                p2 = p3 = dtmP->nullPnt;
                x = pointAddrP (dtmP, p1)->x;
                y = pointAddrP (dtmP, p1)->y;
                z = pointAddrP (dtmP, p1)->z;
                priorPoint = exitPoint = nextPoint = dtmP->nullPnt;
                bcdtmList_nullTptrListDtmObject (dtmP, p1);
                }
            else
                {
                if (bcdtmDrainage_extractPondBoundaryDtmObject (dtmP, pointAddrP (dtmP, exitPoint)->z, exitPoint, nextPoint, loadFunctionP, drawPond, true, &polygonP, userP)) goto errexit;
                *pondDeterminedP = true;
                if (pondElevationP) *pondElevationP = pointAddrP (dtmP, exitPoint)->z;
                if (pondDepthP) *pondDepthP = pointAddrP (dtmP, exitPoint)->z - pointAddrP (dtmP, p1)->z;
                bcdtmList_nullTptrListDtmObject (dtmP, exitPoint);
                if (lowPtP) *lowPtP = *pointAddrP (dtmP, p1);
                }
            }
        }
    /*
    ** Set Triangle Anti Clockwise
    */
    if (p2 != dtmP->nullPnt && p3 != dtmP->nullPnt)
        {
        if (bcdtmMath_pointSideOfDtmObject (dtmP, p1, p2, p3) < 0) { fndType = p2; p2 = p3; p3 = fndType; }
        }
    else if (p2 != dtmP->nullPnt && p3 == dtmP->nullPnt)
        {
        if ((lp1 = bcdtmList_nextAntDtmObject (dtmP, p1, p2)) < 0) goto errexit;
        if (!bcdtmList_testLineDtmObject (dtmP, lp1, p2))
            {
            if ((lp1 = bcdtmList_nextClkDtmObject (dtmP, p1, p2)) < 0) goto errexit;
            if (!bcdtmList_testLineDtmObject (dtmP, lp1, p2)) goto errexit;
            p3 = lp1; lp1 = p1; p1 = p2; p2 = lp1;
            }
        }
    /*
    ** Start Tracing To Low Point
    **
    ** Note
    **
    ** Code Modified RobC 6/1/2005 To Do Multiple Traces To The Low Point
    ** This is necessary to force a trace to the lowest point when the
    ** pond expansion returns an exit point at the same elevation as the low point
    **
    */
    while (trace)
        {
        exitPoint = dtmP->nullPnt;
        /*
        **  Trace To Low Point
        */
        if (dbg) bcdtmWrite_message (0, 0, 0, "Tracing To Low Point From ** %12.5lf %12.5lf %10.4lf", x, y, z);
        if (bcdtmDrainage_traceToLowPointDtmObject (dtmP, drainageTablesP, loadFunctionP, falseLowDepth, zeroSlopeOption, false, p1, p2, p3, x, y, z, nullptr, &lp1, &lp2)) goto errexit;
        if (dbg)
            {
            bcdtmWrite_message (0, 0, 0, "Drains To %9ld %9ld", lp1, lp2);
            if (lp1 != dtmP->nullPnt) bcdtmWrite_message (0, 0, 0, "lp1 = %6ld lp1->hPtr = %9ld ** %10.4lf %10.4lf %10.4lf", lp1, nodeAddrP (dtmP, lp1)->hPtr, pointAddrP (dtmP, lp1)->x, pointAddrP (dtmP, lp1)->y, pointAddrP (dtmP, lp1)->z);
            if (lp2 != dtmP->nullPnt) bcdtmWrite_message (0, 0, 0, "lp2 = %6ld lp2->hPtr = %9ld ** %10.4lf %10.4lf %10.4lf", lp2, nodeAddrP (dtmP, lp2)->hPtr, pointAddrP (dtmP, lp2)->x, pointAddrP (dtmP, lp2)->y, pointAddrP (dtmP, lp2)->z);
            }
        /*
        ** Determine Pond About Low Point
        */
        if (lp1 != dtmP->nullPnt && lp2 == dtmP->nullPnt)
            {
            if (bcdtmList_checkForPointOnHullLineDtmObject (dtmP, lp1, &onHullLine)) goto errexit;
            if (!onHullLine)
                {
                if (dbg) bcdtmWrite_message (0, 0, 0, "Determing Pond About Low Point %6ld", lp1);
                if (bcdtmDrainage_determinePondAboutLowPointDtmObject (dtmP, nullptr, nullptr, nullptr, loadFunctionP, lp1, drawPond, true, &exitPoint, &priorPoint, &nextPoint, &polygonP, userP)) goto errexit;
                if (dbg) bcdtmWrite_message (0, 0, 0, "Exit Point = %8ld ** %12.5lf %12.5lf %10.4lf", exitPoint, pointAddrP (dtmP, exitPoint)->x, pointAddrP (dtmP, exitPoint)->y, pointAddrP (dtmP, exitPoint)->z);
                if (exitPoint == dtmP->nullPnt)
                    {
                    bcdtmWrite_message (0, 0, 0, "Exit Point For Low Point Pond %8ld Not Found", lp1);
                    }
                }
            }
        /*
        ** Determine Pond About Zero Slope SumpLine
        */
        if (lp1 != dtmP->nullPnt && lp2 != dtmP->nullPnt && nodeAddrP (dtmP, lp1)->hPtr == dtmP->nullPnt && nodeAddrP (dtmP, lp2)->hPtr == dtmP->nullPnt)
            {
            if (bcdtmList_checkForLineOnHullLineDtmObject (dtmP, lp1, lp2, &onHullLine)) goto errexit;
            if (!onHullLine)
                {
                if (dbg) bcdtmWrite_message (0, 0, 0, "Determing Pond About Zero Slope Sump Line %6ld %6ld", lp1, lp2);
                if (bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject (dtmP, nullptr, nullptr, loadFunctionP, lp1, lp2, drawPond, true, &exitPoint, &priorPoint, &nextPoint, &sumpLinesP, &numSumpLines, &polygonP, userP, &area)) goto errexit;
                if (sumpLinesP != nullptr) { free (sumpLinesP); sumpLinesP = nullptr; }
                if (exitPoint == dtmP->nullPnt)
                    {
                    bcdtmWrite_message (0, 0, 0, "Exit Point For Zero Sump Line Pond %8ld %8ld Not Found", lp1, lp2);
                    }
                }
            }
        /*
        ** Check Exit Point Is Higher Than Low Point
        */
        if (exitPoint == dtmP->nullPnt) trace = 0;
        else if (pointAddrP (dtmP, exitPoint)->z > pointAddrP (dtmP, lp1)->z)
            {
            trace = 0;
            *pondDeterminedP = true;
            if (pondElevationP) *pondElevationP = pointAddrP (dtmP, exitPoint)->z;
            if (pondDepthP) *pondDepthP = pointAddrP (dtmP, exitPoint)->z - pointAddrP (dtmP, lp1)->z;
            if (lowPtP) *lowPtP = *pointAddrP (dtmP, p1);
            }
        else
            {
            if (bcdtmList_checkForPointOnHullLineDtmObject (dtmP, exitPoint, &onHullLine)) goto errexit;
            if (onHullLine) trace = 0;
            else
                {
                x = pointAddrP (dtmP, exitPoint)->x;
                y = pointAddrP (dtmP, exitPoint)->y;
                z = pointAddrP (dtmP, exitPoint)->z;
                p1 = exitPoint;
                p2 = p3 = dtmP->nullPnt;
                if (polygonP != nullptr) bcdtmPolygon_deletePolygonObject (&polygonP);
                }
            }
        }
    /*
    ** Get Pond Boundary For Volume Calculations
    */
    if (*pondDeterminedP && polygonP != nullptr && polygonP->numPolygons > 0)
        {
        if (bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon (polygonP, 0, &polyPtsP, &numPolyPts)) goto errexit;
        if (dbg)
            {
            bcdtmWrite_message (0, 0, 0, "Before Clean ** Number Of Pond Points = %6ld", numPolyPts);
            bcdtmMath_getPolygonDirectionP3D (polyPtsP, numPolyPts, &polyDirection, &area);
            bcdtmWrite_message (0, 0, 0, "Pond ** Area = %10.4lf polyDirection = %1ld", area, polyDirection);
            if (dbg == 2)
                {
                DPoint3d *p3dP;
                bcdtmWrite_message (0, 0, 0, "Number Of Pond Points = %8ld", numPolyPts);
                for (p3dP = polyPtsP; p3dP < polyPtsP + numPolyPts; ++p3dP)
                    {
                    bcdtmWrite_message (0, 0, 0, "PolyPoint[%4ld] = %12.5lf %12.5lf %10.4lf", (long)(p3dP - polyPtsP), p3dP->x, p3dP->y, p3dP->z);
                    }
                }
            }
        if (numPolyPts == 0)
            {
            *pondDeterminedP = false;
            goto cleanup;
            }

        /*
        **  Clean Polygon Points - Robc - Added 27 Oct 2010 To Remove Point Duplication From Updated Draiange Module
        */
        if (bcdtmClean_internalPointArrayPolygon (&polyPtsP, &numPolyPts, 0.0)) goto errexit;
        //   if( bcdtmClean_externalPointArrayPolygon(&polyPtsP,&numPolyPts,0.0)) goto errexit ;
        if (dbg)
            {
            bcdtmWrite_message (0, 0, 0, "After  Clean ** Number Of Pond Points = %6ld", numPolyPts);
            bcdtmMath_getPolygonDirectionP3D (polyPtsP, numPolyPts, &polyDirection, &area);
            bcdtmWrite_message (0, 0, 0, "Pond ** Area = %15.8lf polyDirection = %1ld", area, polyDirection);
            }

        if (pondVolumeP || pondAreaP)
            {
            /*
            **  RobC - 12Mar2012 - Calculate Surface To Surface To Avoid Clipping DTM
            */
            if (dbg) bcdtmWrite_message (0, 0, 0, "Calculating Pond Volume");
            if (bcdtmObject_createDtmObject (&pondDtmP)) goto errexit;
            if (bcdtmObject_setPointMemoryAllocationParametersDtmObject (pondDtmP, numPolyPts * 4, numPolyPts * 4)) goto errexit;
            if (bcdtmObject_storeDtmFeatureInDtmObject (pondDtmP, DTMFeatureType::Breakline, pondDtmP->nullUserTag, 1, &pondDtmP->nullFeatureId, polyPtsP, numPolyPts)) goto errexit;
            pondDtmP->ppTol = pondDtmP->plTol = 0.0;
            if (bcdtmObject_triangulateDtmObject (pondDtmP)) goto errexit;
            if (bcdtmList_removeNoneFeatureHullLinesDtmObject (pondDtmP)) goto errexit;
            // bcdtmWrite_toFileDtmObject(pondDtmP,L"pond.bcdtm") ;

            double cutArea, fillArea;
            if (bcdtmTinVolume_surfaceToSurfaceDtmObjects (dtmP, pondDtmP, nullptr, 0, nullptr, 0, nullptr, nullptr, cut, fill, balance, cutArea, fillArea)) goto errexit;
            area = cutArea + fillArea;
            if (dbg) bcdtmWrite_message (0, 0, 0, "**Cut = %10.4lf Fill = %10.4lf Balance = %10.4lf Area = %10.4lf", cut, fill, balance, area);
            if (pondVolumeP) *pondVolumeP = fill;
            if (pondAreaP) *pondAreaP = area;
            if (pondDtmP != nullptr) bcdtmObject_destroyDtmObject (&pondDtmP);
            /*
            ** Subtact Area Of Islands From Pond Area
            */
            for (p1 = 1; p1 < polygonP->numPolygons; ++p1)
                {
                if (pondAreaP) *pondAreaP = *pondAreaP - (polygonP->polyListP + p1)->area;
                if (dbg) bcdtmWrite_message (0, 0, 0, "Island %2ld Area = %10.4lf", p1, (polygonP->polyListP + p1)->area);
                }
            }
        }
    /*
   ** Log Time To Determine Pond
   */
    if (tdbg) bcdtmWrite_message (0, 0, 0, "Time To Determine Pond About Low Point = %8.3lf Seconds", bcdtmClock_elapsedTime (bcdtmClock (), startTime));
    /*
    ** Clean Up
    */
cleanup:
    if (polyPtsP != nullptr) free (polyPtsP);
    if (sumpLinesP != nullptr) { free (sumpLinesP); sumpLinesP = nullptr; }
    if (polygonP != nullptr) bcdtmPolygon_deletePolygonObject (&polygonP);
    if (pondDtmP != nullptr) bcdtmObject_destroyDtmObject (&pondDtmP);
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Pond Determined = %2ld Pond Elevation = %10.4lf Pond Depth = %10.4lf Pond Area = %10.4lf Pond Volume = %10.4lf", *pondDeterminedP, *pondElevationP, *pondDepthP, *pondAreaP, *pondVolumeP);
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Calculating Pond Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Calculating Pond Error");
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
int bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject
(
 BC_DTM_OBJ         *dtmP,
 long               dtmPnt1,
 long               dtmPnt2,
 long               dtmPnt3,
 DTMFeatureCallback loadFunctionP ,
 bool               loadFlag,
 bool               boundaryFlag,
 long               *exitPointP,
 long               *priorPointP,
 long               *nextPointP,
 void               *userP
)
/*
** This Function Determines The Pond About A Zero Slope Triangle
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    startPnt ;
 DTM_POLYGON_OBJ *polygonP=nullptr ;

// Log Entry Arguments

 if( dbg )
     {
      bcdtmWrite_message(0,0,0,"Determing Pond About Zero Slope Triangle") ;
      bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
      bcdtmWrite_message(0,0,0,"dtmPnt1       = %8ld",dtmPnt1) ;
      bcdtmWrite_message(0,0,0,"dtmPnt2       = %8ld",dtmPnt2) ;
      bcdtmWrite_message(0,0,0,"dtmPnt3       = %8ld",dtmPnt3) ;
      bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
      bcdtmWrite_message(0,0,0,"loadFlag      = %8ld",loadFlag) ;
      bcdtmWrite_message(0,0,0,"boundaryFlag  = %8ld",boundaryFlag) ;
     }

// Initialise

 *exitPointP = *priorPointP = *nextPointP = dtmP->nullPnt ;

// Set Triangle Points In Correct Direction

 if( bcdtmMath_pointSideOfDtmObject(dtmP,dtmPnt1,dtmPnt2,dtmPnt3) > 0 )
   {
    startPnt = dtmPnt2  ;
    dtmPnt2  = dtmPnt3  ;
    dtmPnt3  = startPnt ;
   }

// Place Tptr Polgon Around Zero Slope Triangle

 startPnt = dtmPnt1 ;
 nodeAddrP(dtmP,dtmPnt1)->tPtr = dtmPnt3 ;
 nodeAddrP(dtmP,dtmPnt3)->tPtr = dtmPnt2 ;
 nodeAddrP(dtmP,dtmPnt2)->tPtr = dtmPnt1 ;
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,startPnt) ;

// Check Connectivity Of Tptr Polygon

 if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,dtmPnt1,0))
     {
      bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
      goto errexit ;
     }

// Expand Pond Boundary To Exit Point

 if( dbg ) bcdtmWrite_message(0,0,0,"Expanding To Exit Point") ;
 if( bcdtmDrainage_expandPondToExitPointDtmObject(dtmP,nullptr,nullptr,nullptr,startPnt,exitPointP,priorPointP,nextPointP)) goto errexit ;
 if( *exitPointP == dtmP->nullPnt )
   {
    bcdtmWrite_message(2,0,0,"Pond Exit Point About Zero Slope Triangle Not Determined") ;
    goto errexit ;
   }

// Draw Pond Boundaries

 if( loadFlag || boundaryFlag )
   {
    if( bcdtmDrainage_extractPondBoundaryDtmObject(dtmP,pointAddrP(dtmP,*exitPointP)->z,*exitPointP,*nextPointP,loadFunctionP,loadFlag,boundaryFlag,&polygonP, userP)) goto errexit ;
   }

// Null Out Tptr Polygon

 bcdtmList_nullTptrListDtmObject(dtmP,*exitPointP) ;

// Clean Up

 cleanup :
 if( polygonP != nullptr ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determing Pond About Zero Slope Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determing Pond About Zero Slope Triangle Error") ;
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
int bcdtmDrainage_extractPondBoundaryDtmObject
(
 BC_DTM_OBJ          *dtmP,
 double              pondElevation,
 long                startPoint,
 long                nextPoint,
 DTMFeatureCallback  loadFunctionP,
 bool                loadFlag,
 bool                boundaryFlag,
 DTM_POLYGON_OBJ     **polygonPP,
 void*               userP
)
/*
** This Function Extracts The Pond Boundary At The Elevation Value Of The Exit Point
** Assumes The Pond Expansion Is Stored In A Tptr Polygon With Start Point
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    clc,lp,sp,minPntZ,minPntNum,maxPntNum,numLowPoints,numHighPoints ;
 long    mark=-98798798,numMarked,numPondPts,pointType,pnt1,pnt2,pnt3 ;
 DPoint3d     *pondPtsP=nullptr ;
 BC_DTM_OBJ  *tempDtmP=nullptr  ;
 BC_DTM_OBJ  *clipTinP=nullptr ;
 long     connect=FALSE ;
 double   zMin=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Extracting Pond Boundary") ;
    bcdtmWrite_message(0,0,0,"pondElevation = %10.4lf",pondElevation) ;
    bcdtmWrite_message(0,0,0,"startPoint    = %10ld ** %12.5lf %12.5lf %10.4lf",startPoint,pointAddrP(dtmP,startPoint)->x,pointAddrP(dtmP,startPoint)->y,pointAddrP(dtmP,startPoint)->z) ;
    bcdtmWrite_message(0,0,0,"nextPoint     = %10ld ** %12.5lf %12.5lf %10.4lf",nextPoint,pointAddrP(dtmP,nextPoint)->x,pointAddrP(dtmP,nextPoint)->y,pointAddrP(dtmP,nextPoint)->z) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"loadFlag      = %8ld",loadFlag) ;
    bcdtmWrite_message(0,0,0,"boundaryFlag  = %8ld",boundaryFlag) ;
    bcdtmWrite_message(0,0,0,"userP         = %p",userP) ;
    if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,startPoint) ;
   }
/*
** Initialise
*/
 if( loadFlag || boundaryFlag )
   {
    if( *polygonPP == nullptr )
      {
       if( bcdtmPolygon_createPolygonObject(polygonPP))
           goto errexit ;
      }
   }
/*
** Write Tptr Polygon Representing The Pond To File
*/
 if( dbg == 2 )
   {
    if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,1000,1000) ;
    if( bcdtmList_copyTptrListFromDtmObjectToDtmObject(dtmP,tempDtmP,startPoint,DTMFeatureType::Breakline,dtmP->nullUserTag,dtmP->nullFeatureId)) goto errexit ;
    bcdtmObject_triangulateDtmObject(tempDtmP) ;
    bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtmP) ;
    bcdtmWrite_toFileDtmObject(tempDtmP,L"pondExpansion.bcdtm") ;
    bcdtmObject_destroyDtmObject(&tempDtmP) ;
   }
/*
** Get Min And Max Point Numbers On Tptr Polygon And
** Check That Start Point Is Lowest Point
*/
 minPntZ   = startPoint ;
 minPntNum = startPoint ;
 maxPntNum = startPoint ;
 sp   = nodeAddrP(dtmP,startPoint)->tPtr ;
 do
   {
    if( sp < minPntNum ) minPntNum = sp ;
    if( sp > maxPntNum ) maxPntNum = sp ;
    if(pointAddrP(dtmP,sp)->z < pointAddrP(dtmP,minPntZ)->z ) minPntZ = sp ;
    sp = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPoint ) ;
/*
** Test If Start Point ( Exit Point ) Is Lowest Point On Tptr Polygon
*/
 if( minPntZ != startPoint )
   {
    bcdtmWrite_message(2,0,0,"Exit Point Is Not Lowest Point On Pond Tptr Polygon") ;
    goto errexit ;
   }
/*
** Trace External Pond Boundary And Create Sptr Polygon For Points Internal To Boundary
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Tracing External Pond Boundary") ;
 if( bcdtmDrainage_tracePondBoundaryDtmObject(dtmP,pondElevation,startPoint,nextPoint,loadFunctionP,loadFlag,boundaryFlag,*polygonPP,&pondPtsP,&numPondPts,userP)) goto errexit ;
 if( dbg == 2 )
   {
    if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,1000,1000) ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,pondPtsP,numPondPts)) goto errexit ;
    if( bcdtmObject_triangulateDtmObject(tempDtmP)) goto errexit ;
    if( bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Writing Pond Boundary ** pondElevation = %10.4lf",pondElevation) ;
    bcdtmWrite_toFileDtmObject(tempDtmP,L"pondBoundary.bcdtm") ;
    if( tempDtmP != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
   }
/*
** Mark All Points Internal To Pond Elevation Boundary Or Sptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 if( bcdtmMark_internalTptrPolygonPointsDtmObject(dtmP,startPoint,mark,&numMarked)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Internal Points = %6ld",numMarked) ;
/*
** Unmark Internal Points Lower Than Exit Point Elevation
*/
 numLowPoints  = 0 ;
 numHighPoints = 0 ;
 if( numMarked > 0 && numPondPts >= 3 )
   {
    for( sp = minPntNum ; sp <= maxPntNum ; ++sp )
      {
       if(nodeAddrP(dtmP,sp)->tPtr == mark )
         {
          if(pointAddrP(dtmP,sp)->z  <= pointAddrP(dtmP,startPoint)->z )
            {
             nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
             ++numLowPoints ;
            }
          else
            {
             if( clipTinP == nullptr )
               {
                if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipTinP,pondPtsP,numPondPts)) goto errexit ;
                if( dbg ) bcdtmWrite_toFileDtmObject(clipTinP,L"clipping.tin") ;
               }
             if( bcdtmFind_triangleDtmObject(clipTinP,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,&pointType,&pnt1,&pnt2,&pnt3)) goto errexit ;
             if( pointType )
               {
/*
**              Check Point Does Not Connect To Tptr Polygon
*/
                connect = FALSE ;
                zMin = pointAddrP(dtmP,sp)->z ;
                clc  = nodeAddrP(dtmP,sp)->cPtr ;
                while( clc != dtmP->nullPtr && ! connect )
                  {
                   lp  = clistAddrP(dtmP,clc)->pntNum ;
                   clc = clistAddrP(dtmP,clc)->nextPtr ;
                   if( pointAddrP(dtmP,lp)->z  < zMin ) zMin = pointAddrP(dtmP,lp)->z ;
                   if( nodeAddrP(dtmP,lp)->tPtr != mark && nodeAddrP(dtmP,lp)->tPtr != dtmP->nullPnt ) connect = TRUE ;
                  }
                if( connect == FALSE && zMin >= pondElevation ) connect = TRUE ;
                if( connect )
                  {
                   nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
                  }
                else
                  {
                   ++numHighPoints ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"Internal Pond High Point[%5ld] = %8ld ** %12.5lf %12.5lf %10.4lf",numHighPoints,sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                  }
               }
             else
               {
                nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
                ++numLowPoints ;
               }
            }
         }
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Internal Low  Points = %6ld",numLowPoints) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Internal High Points = %6ld",numHighPoints) ;
   }
/*
** Trace Internal Pond Boundaries Or Islands
*/
 if( numHighPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Tracing Island Boundaries") ;
/*
**  Plot Internal Pond Boundaries Or Islands
*/
    for( sp = minPntNum ; sp <= maxPntNum ; ++sp )
      {
       if( nodeAddrP(dtmP,sp)->tPtr == mark )
         {
/*
**        Scan Point Looking For Island Start
*/
          clc = nodeAddrP(dtmP,sp)->cPtr;
          while( clc != dtmP->nullPtr )
            {
             lp  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if( pointAddrP(dtmP,sp)->z >= pondElevation && pointAddrP(dtmP,lp)->z < pondElevation )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"pondElevation = %10.4lf ** sp->z = %10.4lf lp->z = %10.4lf",pondElevation,pointAddrP(dtmP,sp)->z,pointAddrP(dtmP,lp)->z) ;
/*
**              Trace Island
*/
                if( bcdtmDrainage_traceIslandBoundaryDtmObject(dtmP,pondElevation,sp,lp,mark,loadFunctionP,loadFlag,boundaryFlag,*polygonPP, userP) ) goto errexit ;
                clc = dtmP->nullPtr ;
               }
            }
         }
      }
   }
/*
** Null Out Marked points
*/
 for( sp = minPntNum ; sp <= maxPntNum ; ++sp )
   {
    if(nodeAddrP(dtmP,sp)->tPtr == mark ) nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
   }
/*
** Clean Up
*/
 cleanup :
 if( pondPtsP != nullptr ) free(pondPtsP) ;
 if( tempDtmP != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 if( clipTinP != nullptr ) bcdtmObject_destroyDtmObject(&clipTinP) ;
/*
** Job Completed
*/
// bcdtmWrite_message(0,0,0,"Pond Boundary Extracted Completed") ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Pond Boundary Extracted Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Pond Boundary Extracted Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_traceIslandBoundaryDtmObject
(
 BC_DTM_OBJ         *dtmP,
 double             pondElevation,
 long               startPoint,
 long               nextPoint,
 long               mark,
 DTMFeatureCallback loadFunctionP,
 bool               loadFlag,
 bool               boundaryFlag,
 DTM_POLYGON_OBJ    *polygonP,
 void               *userP
)
/*
** This Routine Traces The Island Boundary
*/
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long      sp1,sp2,lp1,lp2,ifin=0,numIslandPts=0 ;
 double    ra,xs,ys,xn,yn ;
 DPoint3d  *islandPtsP=nullptr ;
 DTMPointCache  pondCache ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Island Boundary") ;
    bcdtmWrite_message(0,0,0,"Z = %10.4lf",pondElevation) ;
    bcdtmWrite_message(0,0,0,"** startPoint = %6ld TPTR = %9ld ** %10.4lf %10.4lf %10.4lf",startPoint,nodeAddrP(dtmP,startPoint)->tPtr,pointAddrP(dtmP,startPoint)->x,pointAddrP(dtmP,startPoint)->y,pointAddrP(dtmP,startPoint)->z) ;
    bcdtmWrite_message(0,0,0,"** nextPoint  = %6ld TPTR = %9ld ** %10.4lf %10.4lf %10.4lf",nextPoint,nodeAddrP(dtmP,startPoint)->tPtr,pointAddrP(dtmP,nextPoint)->x,pointAddrP(dtmP,nextPoint)->y,pointAddrP(dtmP,nextPoint)->z) ;
   }
/*
** Initialise
*/
 sp1 = startPoint ;
 sp2 = nextPoint  ;
/*
** Calculate Start Point Coordinates
*/
 if( pointAddrP(dtmP,startPoint)->z == pondElevation )  {  xs = pointAddrP(dtmP,startPoint)->x ; ys = pointAddrP(dtmP,startPoint)->y ; }
 else
   {
    ra = (pondElevation - pointAddrP(dtmP,nextPoint)->z)/(pointAddrP(dtmP,startPoint)->z - pointAddrP(dtmP,nextPoint)->z) ;
    xs = pointAddrP(dtmP,nextPoint)->x + ( pointAddrP(dtmP,startPoint)->x - pointAddrP(dtmP,nextPoint)->x ) * ra ;
    ys = pointAddrP(dtmP,nextPoint)->y + ( pointAddrP(dtmP,startPoint)->y - pointAddrP(dtmP,nextPoint)->y ) * ra ;
   }
/*
** Write First Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"xs = %10.4lf ys = %10.4lf zs = %10.4lf",xs,ys,pondElevation) ;
 if( pondCache.StorePointInCache(xs,ys,pondElevation)) goto errexit ;
/*
** Scan Back To First Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Back To Start Point") ;
 while ( ifin == 0 )
   {
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"** startPoint = %6ld Tptr = %9ld ** %10.4lf %10.4lf %10.4lf",startPoint,nodeAddrP(dtmP,startPoint)->tPtr,pointAddrP(dtmP,startPoint)->x,pointAddrP(dtmP,startPoint)->y,pointAddrP(dtmP,startPoint)->z) ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"** nextPoint  = %6ld Tptr = %9ld ** %10.4lf %10.4lf %10.4lf",nextPoint,nodeAddrP(dtmP,nextPoint)->tPtr,pointAddrP(dtmP,nextPoint)->x,pointAddrP(dtmP,nextPoint)->y,pointAddrP(dtmP,nextPoint)->z) ;
    lp2 = nextPoint ; lp1 = startPoint ;
    if( ! bcdtmList_testLineDtmObject(dtmP,startPoint,nextPoint))
      {
       bcdtmWrite_message(0,0,0,"Island Trace Points Not Connected") ;
       goto errexit ;
      }
    if( ( nextPoint = bcdtmList_nextAntDtmObject(dtmP,startPoint,nextPoint)) < 0 ) goto errexit ;
    if( pointAddrP(dtmP,nextPoint)->z == pondElevation )
      {
       startPoint = nextPoint ; nextPoint = lp1 ;
       xn = pointAddrP(dtmP,startPoint)->x ;
       yn = pointAddrP(dtmP,startPoint)->y ;
       if( pondCache.StorePointInCache(xn,yn,pondElevation)) goto errexit ;
      }
    else
      {
       if(pointAddrP(dtmP,nextPoint)->z > pondElevation )
         {
          startPoint = nextPoint ;
          nextPoint  = lp2 ;
         }
       ra = (pondElevation - pointAddrP(dtmP,nextPoint)->z)/(pointAddrP(dtmP,startPoint)->z - pointAddrP(dtmP,nextPoint)->z) ;
       xn = pointAddrP(dtmP,nextPoint)->x + ( pointAddrP(dtmP,startPoint)->x - pointAddrP(dtmP,nextPoint)->x ) * ra ;
       yn = pointAddrP(dtmP,nextPoint)->y + ( pointAddrP(dtmP,startPoint)->y - pointAddrP(dtmP,nextPoint)->y ) * ra ;
       if( pondCache.StorePointInCache(xn,yn,pondElevation)) goto errexit ;
      }
    if(dbg == 2 ) bcdtmWrite_message(0,0,0,"Vertice = %10.4lf %10.4lf %10.4lf",xn,yn,pondElevation) ;
/*
** Test for end of closed contour
*/
    if( nodeAddrP(dtmP,startPoint)->tPtr == mark ) nodeAddrP(dtmP,startPoint)->tPtr = dtmP->nullPnt ;
    if( nodeAddrP(dtmP,nextPoint)->tPtr  == mark ) nodeAddrP(dtmP,nextPoint)->tPtr  = dtmP->nullPnt ;
    if ( sp1 == startPoint && sp2 == nextPoint ) ifin = 1 ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Island Boundary Traced") ;
/*
** Copy Pond Island Bounday To Polygon Object
*/
 if( boundaryFlag )
   {
    if( pondCache.CopyCachePointsToPointArray(&islandPtsP,&numIslandPts)) goto errexit ;
    if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(polygonP,islandPtsP,numIslandPts,1)) goto errexit ;
   }
/*
**  Plot Island
*/
 if( loadFlag )
   {
    if( pondCache.CallUserDelegateWithCachePoints( loadFunctionP,DTMFeatureType::PondIsland,(DTMUserTag)1,dtmP->nullFeatureId,userP)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( islandPtsP != nullptr ) free(islandPtsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Island Boundary Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Island Boundary Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_tracePondBoundaryDtmObject
(
 BC_DTM_OBJ         *dtmP,
 double             pondElevation,
 long               startPoint,
 long               nextPoint,
 DTMFeatureCallback loadFunctionP,
 bool               loadFlag,
 bool               boundaryFlag,
 DTM_POLYGON_OBJ    *polygonP,
 DPoint3d           **pondPtsPP,
 long               *numPondPtsP,
 void*              userP
)
/*
** This Function Traces The Pond Boundary
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(1),cdbg=DTM_CHECK_VALUE(0) ;
 long   ap,apn,np,sp,process,tracePond ;
 double X,Y,ratio,area ;
 DPoint3d    *p3dP,contourStart ;
 BC_DTM_OBJ *tempDtmP=nullptr ;
 DTM_TIN_POINT *pointP ;
 DTMDirection  direction ;
 DTMPointCache pointCache ;
 long mark=-987654,numMarked=0,minPntNum=0,maxPntNum=0 ;
/*
** Initialise
*/
 sp = startPoint ;
 np = nextPoint  ;
/*
** Write Start Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Tracing Pond Boundary ** Z = %10.4lf",pondElevation) ;
    bcdtmWrite_message(0,0,0,"sp = %8ld ** %12.5lf %12.5lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
    bcdtmWrite_message(0,0,0,"np = %8ld ** %12.5lf %12.5lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
   }
/*
** Check Connectivity Tptr Polygon
*/
 if( cdbg )
   {
    if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,startPoint,0))
      {
       bcdtmWrite_message(2,0,0,"Tptr Polygon Connectivity Error") ;
       goto errexit ;
      }
   }
/*
** Log Tptr Polygon Area Direction
*/
 if( dbg == 1 )
   {
    bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,sp,&area,&direction) ;
    bcdtmWrite_message(0,0,0,"Tptr Polygon ** Area = %15.8lf Direction = %2ld",area,direction) ;
   }
/*
**  Plot First Vertice
*/
 if( pointCache.StorePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pondElevation)) goto errexit ;
/*
** Clip DTM To Tptr Polygon And Write To File
*/
 if( dbg == 2 )
   {
    bcdtmObject_cloneDtmObject(dtmP,&tempDtmP);
    bcdtmClip_toTptrPolygonDtmObject (tempDtmP, startPoint, DTMClipOption::External);
    for( ap = 0 ; ap < tempDtmP->numPoints ; ++ap)
      {
       pointP = pointAddrP(tempDtmP,ap) ;
       pointP->x = pointP->x * 1000.0 ;
       pointP->y = pointP->y * 1000.0 ;
       pointP->z = pointP->z  ;
      }
    bcdtmMath_setBoundingCubeDtmObject(tempDtmP) ;
    bcdtmWrite_toFileDtmObject(tempDtmP,L"pondDtm.bcdtm") ;
    bcdtmObject_destroyDtmObject(&tempDtmP) ;
   }
/*
**  Get Tptr Polygon Bounding Points
*/
 sp = startPoint ;
 minPntNum = maxPntNum = sp ;
 do
   {
    sp = nodeAddrP(dtmP,sp)->tPtr ;
    if( sp < minPntNum ) minPntNum = sp ;
    if( sp > maxPntNum ) maxPntNum = sp ;
   } while( sp != startPoint) ;
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"minPntNum = %8ld maxPntNum = %8ld",minPntNum,maxPntNum) ;
/*
**  Mark All Points Internal To To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 if( bcdtmMark_internalTptrPolygonPointsDtmObject(dtmP,startPoint,mark,&numMarked)) goto errexit ;
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Internal Points = %6ld",numMarked) ;
/*
**  Mark Internal Points At Same Elevation As Pond Exit Point Elevation
*/
 numMarked = 0 ;
 for( sp = minPntNum ; sp <= maxPntNum ; ++sp )
   {
    if(nodeAddrP(dtmP,sp)->tPtr == mark )
      {
       if( pointAddrP(dtmP,sp)->z  == pondElevation )
         {
          pointAddrP(dtmP,sp)->z = pointAddrP(dtmP,sp)->z - ( dtmP->zMax - dtmP->zMin ) / 1000.0 ;
          ++numMarked ;
         }
       else
         {
          nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
         }
      }
   }
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Internal Points Adjusted = %6ld",numMarked) ;
/*
**  Scan From Exit Point And Trace Pond Boundary
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Tracing Pond Boundary") ;
 sp = startPoint ;
 np = nodeAddrP(dtmP,sp)->tPtr ;
 pointCache.ClearCache() ;
 contourStart.x = pointAddrP(dtmP,sp)->x ;
 contourStart.y = pointAddrP(dtmP,sp)->y ;
 contourStart.z = pointAddrP(dtmP,sp)->z ;
 if( pointCache.StorePointInCache(contourStart.x,contourStart.y,pondElevation)) goto errexit ;
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"X = %12.5lf Y = %12.5lf Z = %10.4lf  ** Contour Start",contourStart.x,contourStart.y,pondElevation) ;
 tracePond = TRUE ;
 while ( tracePond )
   {
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"sp  = %8ld ** %12.5lf %12.5lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
       bcdtmWrite_message(0,0,0,"np  = %8ld ** %12.5lf %12.5lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
      }
/*
**  Check For Intersection On Line Sp-Np
*/
    if( ( pointAddrP(dtmP,np)->z > pondElevation && pointAddrP(dtmP,sp)->z < pondElevation ) ||
        ( pointAddrP(dtmP,np)->z < pondElevation && pointAddrP(dtmP,sp)->z > pondElevation )    )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Intersection On Line Sp-Np") ;
       ratio = ( pondElevation - pointAddrP(dtmP,sp)->z ) / ( pointAddrP(dtmP,np)->z - pointAddrP(dtmP,sp)->z ) ;
       X = pointAddrP(dtmP,sp)->x + (pointAddrP(dtmP,np)->x - pointAddrP(dtmP,sp)->x ) * ratio ;
       Y = pointAddrP(dtmP,sp)->y + (pointAddrP(dtmP,np)->y - pointAddrP(dtmP,sp)->y ) * ratio ;
       if( pointCache.StorePointInCache(X,Y,pondElevation)) goto errexit ;
       if( X == contourStart.x && Y == contourStart.y ) tracePond = FALSE ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"X = %12.5lf Y = %12.5lf Z = %10.4lf ** 00",X,Y,pondElevation) ;
       if(( np = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
      }
/*
**     Check For Contour CoinCident With Line Sp-Np - Can Only Occur On Tptr Polygon
*/
    else if( pointAddrP(dtmP,np)->z == pondElevation && pointAddrP(dtmP,sp)->z == pondElevation )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Contour CoinCident With Sp-Np") ;
       X = pointAddrP(dtmP,np)->x  ;
       Y = pointAddrP(dtmP,np)->y  ;
       if( pointCache.StorePointInCache(X,Y,pondElevation)) goto errexit ;
       if( X == contourStart.x && Y == contourStart.y ) tracePond = FALSE ;
       sp = np ;
       np = nodeAddrP(dtmP,sp)->tPtr ;
       if( dbg == 2 )bcdtmWrite_message(0,0,0,"X = %12.5lf Y = %12.5lf Z = %10.4lf ** 01",X,Y,pondElevation) ;
      }
/*
**  Check For Contour CoinCident With Np - Can Only Occur On Tptr Polygon
*/
    else if( pointAddrP(dtmP,np)->z == pondElevation && pointAddrP(dtmP,sp)->z != pondElevation )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Contour CoinCident With Np") ;
       X = pointAddrP(dtmP,np)->x  ;
       Y = pointAddrP(dtmP,np)->y  ;
       if( pointCache.StorePointInCache(X,Y,pondElevation)) goto errexit ;
       if( X == contourStart.x && Y == contourStart.y ) tracePond = FALSE ;
       sp = np ;
       np = nodeAddrP(dtmP,sp)->tPtr ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"X = %12.5lf Y = %12.5lf Z = %10.4lf ** 01",X,Y,pondElevation) ;
      }
/*
**  Check For Contour CoinCident With Sp - Can Only Occur On Tptr Polygon
*/
    else if( pointAddrP(dtmP,sp)->z == pondElevation && pointAddrP(dtmP,np)->z != pondElevation )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Contour CoinCident With Sp") ;
       ap = np ;
       if( ( apn = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
       process = 1 ;
       while( process )
         {
          if( pointAddrP(dtmP,apn)->z == pondElevation )
            {
             process = 0 ;
             sp = apn ;
             np = nodeAddrP(dtmP,sp)->tPtr ;
             X = pointAddrP(dtmP,sp)->x  ;
             Y = pointAddrP(dtmP,sp)->y  ;
             if( pointCache.StorePointInCache(X,Y,pondElevation)) goto errexit ;
             if( X == contourStart.x && Y == contourStart.y ) tracePond = FALSE ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"X = %12.5lf Y = %12.5lf Z = %10.4lf ** 02",X,Y,pondElevation) ;
            }
          else if( ( pointAddrP(dtmP,ap)->z > pondElevation && pointAddrP(dtmP,apn)->z < pondElevation ) ||
                   ( pointAddrP(dtmP,ap)->z < pondElevation && pointAddrP(dtmP,apn)->z > pondElevation )    )
            {
             process = 0 ;
             sp = apn ;
             np = ap ;
             ratio = ( pondElevation - pointAddrP(dtmP,sp)->z ) / ( pointAddrP(dtmP,np)->z - pointAddrP(dtmP,sp)->z ) ;
             X = pointAddrP(dtmP,sp)->x + (pointAddrP(dtmP,np)->x - pointAddrP(dtmP,sp)->x ) * ratio ;
             Y = pointAddrP(dtmP,sp)->y + (pointAddrP(dtmP,np)->y - pointAddrP(dtmP,sp)->y ) * ratio ;
             if( pointCache.StorePointInCache(X,Y,pondElevation)) goto errexit ;
             if( X == contourStart.x && Y == contourStart.y ) tracePond = FALSE ;
             if( ( np = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"X = %12.5lf Y = %12.5lf Z = %10.4lf ** 03",X,Y,pondElevation) ;
            }
          else
            {
             ap = apn ;
             if( ( apn = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
            }
         }
      }
/*
**  Switch Direction
*/
    else
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Switching") ;
       if( ( ap = bcdtmList_nextAntDtmObject(dtmP,np,sp)) < 0 ) goto errexit ;
       sp = np ;
       np = ap ;
      }
   }
/*
**  Get Contour Points From Cache
*/
 if( pointCache.CopyCachePointsToPointArray(pondPtsPP,numPondPtsP)) goto errexit ;
 pointCache.ClearCache() ;
/*
** Log Pond Boundary To File
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Pond Points = %8ld",*numPondPtsP) ;
    for( p3dP = *pondPtsPP ; p3dP < *pondPtsPP + *numPondPtsP ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Pond Point[%8ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-*pondPtsPP),p3dP->x,p3dP->y,p3dP->z) ;
      }
    bcdtmObject_createDtmObject(&tempDtmP);
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,*pondPtsPP,*numPondPtsP)) goto errexit ;
    bcdtmWrite_geopakDatFileFromDtmObject(tempDtmP,L"pondBoundary.dat") ;
    bcdtmObject_destroyDtmObject(&tempDtmP) ;
   }
/*
** Unmark Internal Points AT  Exit Point Elevation
*/
 numMarked = 0 ;
 for( sp = minPntNum ; sp <= maxPntNum ; ++sp )
   {
    if(nodeAddrP(dtmP,sp)->tPtr == mark )
      {
       ++numMarked ;
       pointAddrP(dtmP,sp)->z = pondElevation ;
       nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
      }
   }
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Internal Points Adjusted = %6ld",numMarked) ;
/*
** Log Pond Polygon Area Direction
*/
 if( dbg == 1 )
   {
    bcdtmMath_getPolygonDirectionP3D(*pondPtsPP,*numPondPtsP,&direction,&area);
    bcdtmWrite_message(0,0,0,"Pond Polygon Before Clean ** Area = %15.8lf Direction = %2ld",area,direction) ;
   }
/*
**  Internally Clean Pond
*/
 if( cdbg == 2 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Internally Cleaning Pond Boundary") ;
    if( bcdtmDrainage_internallyCleanPointArrayPolygon(pondPtsPP,numPondPtsP,dtmP->mppTol))
      {
/*
**     As Clean Errors Result From Colinear Or Zero Error Ponds Simply Return
*/
       if( *pondPtsPP != nullptr )
         {
          free(*pondPtsPP ) ;
          *pondPtsPP = nullptr ;
          *numPondPtsP = 0 ;
         }
       goto cleanup ;
      }
   }
/*
** Check Pond Area
*/
 if( dbg == 1 )
   {
    bcdtmMath_getPolygonDirectionP3D(*pondPtsPP,*numPondPtsP,&direction,&area);
    bcdtmWrite_message(0,0,0,"Pond Polygon After  Clean ** Area = %15.8lf Direction = %2ld",area,direction) ;
   }
/*
**  Copy Pond Bounday To Polygon Object
*/
 if( boundaryFlag )
   {
    if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(polygonP,*pondPtsPP,*numPondPtsP,2)) goto errexit ;
   }
/*
** Restore Pond Points In cache
*/
 if( loadFlag )
   {
    pointCache.ClearCache() ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Of Pond Points = %8ld",*numPondPtsP) ;
    for( p3dP = *pondPtsPP ; p3dP < *pondPtsPP + *numPondPtsP ; ++p3dP )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Pond Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-*pondPtsPP),p3dP->x,p3dP->y,p3dP->z) ;
       if( pointCache.StorePointInCache(p3dP->x,p3dP->y,p3dP->z)) goto errexit ;
      }
   }
/*
** Load Low Point Pond From Cache
*/
 if( dbg && loadFlag ) bcdtmWrite_message(0,0,0,"Loading Pond Boundary") ;
 if( loadFlag )
   {
    if( pointCache.CallUserDelegateWithCachePoints(( DTMFeatureCallback)loadFunctionP,DTMFeatureType::LowPointPond,(DTMUserTag)2,dtmP->nullFeatureId,userP))
      {
       bcdtmWrite_message(1,0,0,"Error Return From Pond Browse Method") ;
       goto errexit ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int  bcdtmDrainage_determinePondsDtmObject
(
 BC_DTM_OBJ         *dtmP,                      // ==> Pointer To DTM Object
 DTMDrainageTables  *drainageTablesP,           // ==> Pointer To Drainage Tables
 DTMFeatureCallback loadFunctionP,              // ==> Pointer To Call Back Function
 bool               loadFlag,                   // ==> If True Pass The Pond Features Back To Calling Program
 bool               buildTable,                 // ==> If True Create Pond Tables
 void               *userP                      // ==> User Pointer Passed back In The Call Back
)
//
// This Function Determines Ponds About
//
//     1. Zero Slope Triangles
//     2. Zero Slope Sump Lines
//     3. Low Points
//
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long startTime,pondTime=bcdtmClock() ;
 int  numLowPointPonds=0,numSumpLinePonds=0,numZeroSlopeTrianglePonds=0 ;
 int  numZeroSlopePolygons,*intP,*zeroSlopePointsIndexP=nullptr ;
 DTMZeroSlopePolygonVector zeroSlopePolygons ;

// Log Function Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determining Ponds") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"loadFlag        = %2ld",loadFlag) ;
    bcdtmWrite_message(0,0,0,"buildTable      = %2ld",buildTable) ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
   }

// Check Drainage Tables Exist For Building Pond Tables

 if( buildTable )
   {
    if( drainageTablesP == nullptr ) buildTable = 0 ;
    else
      {
       drainageTablesP->ClearPondTables() ;
      }
   }

// Test For Valid DTM Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;

// Check DTM Is Triangulated

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }

// Check Triangulation

 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(1,0,0,"Triangulation Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
   }

// Null Dtm Lists

 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;

// Polygonise Zero Slope Triangles

 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Zero Slope Triangles") ;
 if( bcdtmDrainage_polygoniseZeroSlopeTrianglesDtmObject(dtmP,zeroSlopePolygons)) goto errexit ;
 numZeroSlopePolygons = (int)(zeroSlopePolygons.end()-zeroSlopePolygons.begin()) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Zero Slope Polygons = %8ld",numZeroSlopePolygons) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Polygonise Zero Slope Triangles = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

// Allocate Memory To Index Zero Slope Polygons From Zero Slope Triangle Points

 if( numZeroSlopePolygons > 0 )
   {
    zeroSlopePointsIndexP = ( int * ) malloc( dtmP->numPoints * sizeof(int)) ;
    if( zeroSlopePointsIndexP == nullptr )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( intP = zeroSlopePointsIndexP ; intP < zeroSlopePointsIndexP + dtmP->numPoints ; ++intP )
      {
       *intP = dtmP->nullPnt ;
      }
   }

// Determine Zero Slope Triangle Ponds

 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Ponds About Zero Slope Triangles") ;
 startTime = bcdtmClock() ;
 if( bcdtmDrainage_determineZeroSlopeTrianglePondsDtmObject(dtmP,drainageTablesP,&zeroSlopePolygons,zeroSlopePointsIndexP,loadFunctionP,loadFlag,buildTable,userP,numZeroSlopeTrianglePonds)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Zero Slope Triangle Ponds         = %8ld",numZeroSlopeTrianglePonds) ;
    bcdtmWrite_message(0,0,0,"Time To Determine Zero Slope Triangle Ponds = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }

// Determine Zero Slope Sump Line Ponds

 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Ponds About Zero Slope Sump Lines") ;
 startTime = bcdtmClock() ;
 if( bcdtmDrainage_determineZeroSlopeSumpLinePondsDtmObject(dtmP,drainageTablesP,&zeroSlopePolygons,zeroSlopePointsIndexP,loadFunctionP,loadFlag,buildTable,userP,numSumpLinePonds)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Sump Line Ponds         = %8ld",numSumpLinePonds) ;
    bcdtmWrite_message(0,0,0,"Time To Determine Sump Line Ponds = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }

// Determine Low Point Ponds

 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Ponds About Low Points") ;
 startTime = bcdtmClock() ;
 if( bcdtmDrainage_determineLowPointPondsDtmObject(dtmP,drainageTablesP,&zeroSlopePolygons,zeroSlopePointsIndexP,loadFunctionP,loadFlag,buildTable,userP,numLowPointPonds)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Low Point Ponds         = %8ld",numLowPointPonds) ;
    bcdtmWrite_message(0,0,0,"Time To Determine Low Point Ponds = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }

// Sort Pond Tables For Binary Searching Purposes

 if( buildTable && drainageTablesP != nullptr  )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Pond Tables") ;
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Low Point Pond Table ** Size = %8ld",drainageTablesP->SizeOfLowPointPondTable()) ;
    if( drainageTablesP->SortLowPointPondTable()) goto errexit ; ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Zero Sump Line Pond Table ** Size = %8ld",drainageTablesP->SizeOfZeroSlopeLinePondTable()) ;
    if( drainageTablesP->SortZeroSlopeLinePondTable()) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Sort Pond Tables = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }

// Clean Up

 cleanup :
 if( zeroSlopePointsIndexP != nullptr ) { free(zeroSlopePointsIndexP) ; zeroSlopePointsIndexP = nullptr ; }
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
/*
** Normal Exit
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Determine Ponds = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),pondTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Ponds Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Ponds Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_determineLowPointPondsDtmObject
(
 BC_DTM_OBJ                 *dtmP,                      // ==> Pointer To Dtm Objec
 DTMDrainageTables          *drainageTablesP,           // ==> Pointer To Drainage Tables
 DTMZeroSlopePolygonVector  *zeroSlopePolygonsP,        // ==> Pointer To Zero Slope Polygons
 int                        *zeroSlopePointsIndexP,     // ==> Point Index To Zero Slope Polygons
 DTMFeatureCallback         loadFunctionP,              // ==> Pointer To Call Back Function
 bool                       loadFlag,                   // ==> Load Pond Features
 bool                       buildTable,                 // ==> Build Pond Drainage Table
 void                       *userP,                     // ==> User Pointer Passed Back
 int&                       numLowPointPonds            // <== Number Of Low Point Ponds
)
{
/*
** Scan Tin For Internal Low Points
*/
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p1,p2,clc,node,exitPoint,priorPoint,nextPoint,expandStart ;
 DTM_TIN_NODE *nodeP ;
 DTM_POLYGON_OBJ *polygonP=nullptr ;
 bool  lowPoint=true ;
/*
** Write Entr Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determing Low Point Ponds") ;
    bcdtmWrite_message(0,0,0,"dtmP                  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP       = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePolygonsP    = %p",zeroSlopePolygonsP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePointsIndexP = %p",zeroSlopePointsIndexP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP         = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"loadFlag              = %8ld",loadFlag) ;
    bcdtmWrite_message(0,0,0,"buildTable            = %8ld",buildTable) ;
    bcdtmWrite_message(0,0,0,"userP                 = %p",userP) ;
   }
/*
** Test For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Null Out Tptr values
*/
 numLowPointPonds = 0 ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
/*
** Scan Data Points And Return Low Point Ponds
*/
 for( node = 0 ; node < dtmP->numPoints ; ++node )
   {
    nodeP = nodeAddrP(dtmP,node) ;
    if( ( clc = nodeP->cPtr) != dtmP->nullPtr && nodeP->hPtr == dtmP->nullPnt )
      {
       p1 = node;
/*
**     Test For Void Point
*/
       if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD) )
         {

//        Test For Low Point

          lowPoint = true ;
          while ( clc != dtmP->nullPtr && lowPoint )
            {
             p2  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if( pointAddrP(dtmP,p2)->z <= pointAddrP(dtmP,p1)->z ) lowPoint = false ;
            }

//        Calculate Pond About Low Point

          if( lowPoint == true )
            {
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Determing Low Point Pond About Point %8ld ** %12.5lf %12.5lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
             expandStart = bcdtmClock() ;
             if( bcdtmDrainage_determinePondAboutLowPointDtmObject(dtmP,drainageTablesP,zeroSlopePolygonsP,zeroSlopePointsIndexP,loadFunctionP,p1,loadFlag,false,&exitPoint,&priorPoint,&nextPoint,&polygonP, userP) != DTM_SUCCESS ) goto errexit ;
             ++numLowPointPonds ;
             if( tdbg && bcdtmClock_elapsedTime(bcdtmClock(),expandStart) > 0.001 )
               {
                bcdtmWrite_message(0,0,0,"Expansion Time For Low Point %8ld = %8.5lf",p1,bcdtmClock_elapsedTime(bcdtmClock(),expandStart)) ;
               }
             if( polygonP != nullptr ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
             if( buildTable && drainageTablesP != nullptr )
               {
                drainageTablesP->StoreLowPointPond(p1,exitPoint,priorPoint,nextPoint) ;
               }
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( polygonP != nullptr ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
/*
** Normal Exit
*/
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determing Low Point Ponds Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determing Low Point Ponds Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( cdbg ) bcdtmWrite_message(0,0,0,"Error Determining Pond About DTM Point %8ld",p1) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_determinePondAboutLowPointDtmObject
(
 BC_DTM_OBJ                *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables         *drainageTablesP,           // ==> Pointer To Drainage Tables
 DTMZeroSlopePolygonVector *zeroSlopePolygonsP,        // ==> Pointer To Zero Slope Polygons
 int                       *zeroSlopePointsIndexP,     // ==> Index To Zero Slope Polygons
 DTMFeatureCallback        loadFunctionP,              // ==> Pointer To Call Back Function
 long                      lowPoint,                   // ==> Low Point To Create Pond About
 bool                      loadFlag,                   // ==> Pass The Pond Boundaries Back
 bool                      boundaryFlag,               // ==> ???
 long                      *exitPointP,                // <== Pond Exit Point On Tptr Polygon
 long                      *priorPointP,               // <== Prior Point To Exit Point On Tptr Polygon
 long                      *nextPointP,                // <== Next Point After Exit Point On Tptr Polygon
 DTM_POLYGON_OBJ           **polygonPP,                // <== Polygon Object To Store Pond Boundary
 void                      *userP                      // ==> User Pointer Passed Back To Call back Function
)
/*
** This Function Determines The Pond About A Low Point
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    startPoint,ofs,node ;
 bool    pondValid=true ;
 DTM_TIN_NODE   *nodeP ;

 long numPondPts=0 ;
 DPoint3d *pondPtsP=nullptr ;
 BC_DTM_OBJ  *tempDtmP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determing Pond About Low Point") ;
    bcdtmWrite_message(0,0,0,"dtmP                  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP       = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePolygonsP    = %p",zeroSlopePolygonsP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePointsIndexP = %p",zeroSlopePointsIndexP) ;
    bcdtmWrite_message(0,0,0,"LoadFunctionP         = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"Low Point             = %8ld ** %10.4lf %10.4lf %10.4lf",lowPoint,pointAddrP(dtmP,lowPoint)->x,pointAddrP(dtmP,lowPoint)->y,pointAddrP(dtmP,lowPoint)->z) ;
    bcdtmWrite_message(0,0,0,"loadFlag              = %8ld",loadFlag) ;
    bcdtmWrite_message(0,0,0,"boundaryFlag          = %8ld",boundaryFlag) ;
    if( dbg == 2 ) bcdtmList_writeCircularListForPointDtmObject(dtmP,lowPoint) ;
   }
/*
** Initialise
*/
 *exitPointP  = dtmP->nullPnt ;
 *priorPointP = dtmP->nullPnt ;
 *nextPointP  = dtmP->nullPnt ;
 if( *polygonPP != nullptr ) bcdtmPolygon_deletePolygonObject(polygonPP) ;
/*
** Check For None Null Tptr Or Sptr Values
*/
 if( cdbg )
   {
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
    bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtmP,1) ;
   }
/*
** Place Pond Around Low Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Placing Tptr Polygon About Low Point") ;
 if( bcdtmList_insertTptrPolygonAroundPointDtmObject(dtmP,lowPoint,&startPoint)) goto errexit ;
/*
** Expand Pond Boundary To Exit Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Expanding Pond To Exit Point") ;
 if( bcdtmDrainage_expandPondToExitPointDtmObject(dtmP,drainageTablesP,zeroSlopePolygonsP,zeroSlopePointsIndexP,startPoint,exitPointP,priorPointP,nextPointP)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"priorPoint = %8ld exitPoint = %8ld nextPoint = %8ld",*priorPointP,*exitPointP,*nextPointP) ;
    if( *exitPointP != dtmP->nullPnt )
      {
       bcdtmWrite_message(0,0,0,"ExitPoint = %8ld ** %12.5lf %12.5lf %10.4lf",*exitPointP,pointAddrP(dtmP,*exitPointP)->x,pointAddrP(dtmP,*exitPointP)->y,pointAddrP(dtmP,*exitPointP)->z) ;
      }
   }
 if( *exitPointP == dtmP->nullPnt )
   {
    bcdtmWrite_message(2,0,0,"Pond Exit Point About About Low Point Not Determined") ;
    goto errexit ;
   }

/*
** Copy Pond Boundary To DTM
*/
 if( cdbg )
   {
    if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,*exitPointP,&pondPtsP,&numPondPts)) goto errexit ;
    if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,pondPtsP,numPondPts)) goto errexit ;
    if( bcdtmWrite_geopakDatFileFromDtmObject(tempDtmP,L"pondBoundary.dat")) goto errexit ;
    if( pondPtsP != nullptr ) { free(pondPtsP) ; pondPtsP = nullptr ; }
    if( tempDtmP != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
   }
/*
** Validate Pond
*/
 if( cdbg == 2 )
   {
    if( bcdtmDrainage_validatePondDtmObject(dtmP,*exitPointP,pondValid))
      {
       bcdtmWrite_message(0,0,0,"Pond Validation Error ** LowPoint = %8ld",lowPoint) ;
       goto errexit ;
      }
   }
/*
** Draw Pond Boundaries
*/
 if( loadFlag || boundaryFlag )
   {
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Drawing Pond Boundary") ;
       bcdtmWrite_message(0,0,0,"lowPoint   = %9ld ** %10.4lf %10.4lf %10.4lf",lowPoint,pointAddrP(dtmP,lowPoint)->x,pointAddrP(dtmP,lowPoint)->y,pointAddrP(dtmP,lowPoint)->z) ;
       bcdtmWrite_message(0,0,0,"priorPoint = %9ld ** %10.4lf %10.4lf %10.4lf",*priorPointP,pointAddrP(dtmP,*priorPointP)->x,pointAddrP(dtmP,*priorPointP)->y,pointAddrP(dtmP,*priorPointP)->z) ;
       bcdtmWrite_message(0,0,0,"exitPoint  = %9ld ** %10.4lf %10.4lf %10.4lf",*exitPointP,pointAddrP(dtmP,*exitPointP)->x,pointAddrP(dtmP,*exitPointP)->y,pointAddrP(dtmP,*exitPointP)->z) ;
       bcdtmWrite_message(0,0,0,"nextPoint  = %9ld ** %10.4lf %10.4lf %10.4lf",*nextPointP,pointAddrP(dtmP,*nextPointP)->x,pointAddrP(dtmP,*nextPointP)->y,pointAddrP(dtmP,*nextPointP)->z) ;
       if( dbg == 2 )bcdtmList_writeTptrListDtmObject(dtmP,*exitPointP) ;
      }
    if( bcdtmDrainage_extractPondBoundaryDtmObject(dtmP,pointAddrP(dtmP,*exitPointP)->z,*exitPointP,*nextPointP,loadFunctionP,loadFlag,boundaryFlag,polygonPP, userP)) goto errexit ;
   }
/*
** Null Out Tptr Polygon
*/
  bcdtmList_nullTptrListDtmObject(dtmP,*exitPointP) ;
/*
** Check For None Null Pointer Values
*/
 if( cdbg )
   {
    for( node = 0  ; node < dtmP->numPoints ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr != dtmP->nullPnt || nodeP->sPtr != dtmP->nullPnt )
         {
          ofs = node ;
          bcdtmWrite_message(0,0,0,"[%6ld]->tPtr = %9ld [%6ld]->sPtr = %9ld",ofs,nodeP->tPtr,ofs,nodeP->sPtr) ;
          ret=DTM_ERROR ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( pondPtsP != nullptr ) { free(pondPtsP) ; pondPtsP = nullptr ; }
 if( tempDtmP != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
/*
** Non Error Exit
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Determing Pond About Low Point Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Determing Pond About Low Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_scanPondForExitPointDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              startPoint,                 // ==> Tptr Polygon Start Point
 double            lowPointZ,                  // ==> Lowest Elevation Value On Tptr Polygon
 long              *exitPointFoundP,           // <== Exit Point Found
 long              *exitPointP,                // <== Exit Point
 long              *priorPointP,               // <== Prior Point To Exit Point On Tptr Polygon
 long              *nextPointP                 // <== Next Point After Exit Point On Tptr Polygon
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   cPnt,lpPnt,llPnt,lnPnt,hullPoint ;
 long   flowOutPoint,maxPpnt,maxFpnt,maxNpnt,descentType,descentPnt1,descentPnt2 ;
 long   cFlowPnt,nFlowPnt,pFlowPnt,numFlowOutPoints ;
 double area,descentAngle,descentSlope,maxDescentSlope ;
 DTMDirection direction ;

/*
** Initialise
*/
 *exitPointFoundP = 0 ;
 *exitPointP = dtmP->nullPnt ;
 *priorPointP = dtmP->nullPnt ;
 *nextPointP = dtmP->nullPnt ;
/*
**  Scan Pond For Low Points On Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Pond For Exit Point On Hull") ;
 lpPnt = startPoint ;
 llPnt = nodeAddrP(dtmP,lpPnt)->tPtr ;
 do
   {
    lnPnt = nodeAddrP(dtmP,llPnt)->tPtr ;
    if( pointAddrP(dtmP,llPnt)->z == lowPointZ )
      {
       if( bcdtmList_testForHullPointDtmObject(dtmP,llPnt,&hullPoint)) goto errexit ;
       if( hullPoint )
         {
          *priorPointP = lpPnt ;
          *exitPointP  = llPnt ;
          *nextPointP  = lnPnt ;
          *exitPointFoundP = 1 ;
         }
      }
    lpPnt = llPnt ;
    llPnt = lnPnt ;
   } while ( lpPnt != startPoint && ! *exitPointFoundP ) ;
/*
**   Scan Pond For Exit Point At Low Point
*/
  if( ! *exitPointFoundP )
    {
     if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Pond For Internal Exit Point") ;
     lpPnt = startPoint ;
     llPnt = nodeAddrP(dtmP,lpPnt)->tPtr ;
     do
       {
        lnPnt = nodeAddrP(dtmP,llPnt)->tPtr ;
        if( pointAddrP(dtmP,llPnt)->z == lowPointZ )
          {
           if( ( cPnt = bcdtmList_nextClkDtmObject(dtmP,llPnt,lnPnt)) < 0 ) goto errexit ;
           while( nodeAddrP(dtmP,cPnt)->tPtr != llPnt && ! *exitPointFoundP )
             {
              if( pointAddrP(dtmP,cPnt)->z < lowPointZ )
                {
                 *priorPointP = lpPnt ;
                 *exitPointP  = llPnt ;
                 *nextPointP  = lnPnt ;
                 *exitPointFoundP = 1 ;
                }
              if( ( cPnt = bcdtmList_nextClkDtmObject(dtmP,llPnt,cPnt)) < 0 ) goto errexit ;
             }
          }
        lpPnt = llPnt ;
        llPnt = lnPnt ;
       } while ( lpPnt != startPoint && ! *exitPointFoundP ) ;
    }
/*
**   Scan Pond For Exit Point At Low Point
*/
  if( ! *exitPointFoundP )
    {
     if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Pond For Exit Point") ;
     lpPnt = startPoint ;
     llPnt = nodeAddrP(dtmP,lpPnt)->tPtr ;
     do
       {
        lnPnt = nodeAddrP(dtmP,llPnt)->tPtr ;
        if( pointAddrP(dtmP,llPnt)->z == lowPointZ )
          {
           if( bcdtmDrainage_testForPondExitPointDtmObject(dtmP,llPnt,&lpPnt,&lnPnt,&flowOutPoint)) goto errexit ;
           if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Point %8ld ** flowOutPoint = %2ld",llPnt,flowOutPoint) ;
           if( flowOutPoint )
             {
/*
**            Get Maximum Descent Flow Out Point
*/
              maxPpnt = lpPnt ;
              maxFpnt = llPnt ;
              maxNpnt = lnPnt ;
              if( bcdtmDrainage_scanBetweenPointsForMaximumDescentDtmObject(dtmP,drainageTablesP,llPnt,lnPnt,lpPnt,&descentType,&descentPnt1,&descentPnt2,&descentSlope,&descentAngle)) goto errexit ;
              maxDescentSlope = descentSlope ;
/*
**            Scan Pond For Other Flow Out Points
*/
              numFlowOutPoints = 1 ;
              cFlowPnt = lnPnt ;
              do
                {
                 nFlowPnt = nodeAddrP(dtmP,cFlowPnt)->tPtr ;
                 if( pointAddrP(dtmP,cFlowPnt)->z == lowPointZ )
                   {
                    if( bcdtmDrainage_testForPondExitPointDtmObject(dtmP,cFlowPnt,&pFlowPnt,&nFlowPnt,&flowOutPoint)) goto errexit ;
                    if( flowOutPoint )
                      {
                       ++numFlowOutPoints ;
                       if( bcdtmDrainage_scanBetweenPointsForMaximumDescentDtmObject(dtmP,drainageTablesP,cFlowPnt,nFlowPnt,pFlowPnt,&descentType,&descentPnt1,&descentPnt2,&descentSlope,&descentAngle)) goto errexit ;
                       if( descentSlope < maxDescentSlope )
                         {
                          maxPpnt = pFlowPnt ;
                          maxFpnt = cFlowPnt ;
                          maxNpnt = nFlowPnt ;
                          maxDescentSlope = descentSlope ;
                         }
                      }
                   }
                 cFlowPnt = nFlowPnt ;
                } while( cFlowPnt != llPnt ) ;
/*
**            Report Stats To Log File
*/
              if( dbg )
                {
                 bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction) ;
                 bcdtmWrite_message(0,0,0,"Pond Area = %12.4lf ** Number Of Pond Flow Out Points = %8ld",area,numFlowOutPoints) ;
                 bcdtmWrite_message(0,0,0,"maxDescentSlope = %12.5lf ** priorPoint = %8ld exitPoint = %8ld nextPoint = %8ld",maxDescentSlope,maxPpnt,maxFpnt,maxNpnt) ;
                }
/*
**            Set Maximum Descent Exit Point
*/
              *priorPointP = maxPpnt ;
              *exitPointP  = maxFpnt ;
              *nextPointP  = maxNpnt ;
              *exitPointFoundP = 1 ;
             }
          }
        lpPnt = llPnt ;
        llPnt = lnPnt ;
       } while ( lpPnt != startPoint && ! *exitPointFoundP ) ;
    }
/*
** Clean Up
*/
 cleanup :
/*
** Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Pond Exit Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Pond Exit Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_testForPondExitPointDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       lowPoint,
 long       *priorPointP,
 long       *nextPointP,
 long       *exitFromPointP
 )
/*
** This Function Tests If The Low Point Is A Pond Exit Point
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Pond Exit Point") ;
/*
** Initialise
*/
 *priorPointP = dtmP->nullPnt ;
 *nextPointP  = dtmP->nullPnt ;
 *exitFromPointP = 0 ;
/*
** Get Next And Prior Points On Pond Boundary
*/
 *priorPointP = *nextPointP  = nodeAddrP(dtmP,lowPoint)->tPtr ;
 while ( nodeAddrP(dtmP,*priorPointP)->tPtr != lowPoint )
   {
    if( pointAddrP(dtmP,*priorPointP)->z < pointAddrP(dtmP,lowPoint)->z ) *exitFromPointP = 1 ;
    if(( *priorPointP = bcdtmList_nextClkDtmObject(dtmP,lowPoint,*priorPointP)) < 0 ) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Pond Exit Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Pond Exit Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_expandPondAboutPointDtmObject
(
 BC_DTM_OBJ *dtmP,                            // ==> Pointer To DTM Object
 long       lowPoint,                         // ==> Low Point To Expand About
 double     lastArea,                         // ==> Area For Expansion - Development Purposes
 long       *startPointP,                     // <== New Start Point On Pond Tptr Polygo
 double     *areaP,                           // <== Area Of Expanded Pond Tptr Polygon
 long       *extStartPntP,                    // <== Start Point Of Extended Section
 long       *extEndPntP                       // <== End Point Of Extended Section
)
/*
** This Function Expands A Pond About A Low Point
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(1) ;
 DTMDirection direction ;
 double area ;
 long   startTime ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Expanding Pond About Point") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"lowPoint    = %8ld",lowPoint) ;
    bcdtmWrite_message(0,0,0,"lastArea    = %12.4lf",lastArea) ;
    bcdtmWrite_message(0,0,0,"startPnt    = %8ld",*startPointP) ;
    bcdtmWrite_message(0,0,0,"LowPoint    = %8ld  ** %12.5lf %12.5lf %10.4lf",lowPoint,pointAddrP(dtmP,lowPoint)->x,pointAddrP(dtmP,lowPoint)->y,pointAddrP(dtmP,lowPoint)->z) ;
   }
/*
** Initialise
*/
 *areaP = 0.0 ;
/*
** Expand Tptr Polygon About Point
*/
 startTime = bcdtmClock() ;
 if( bcdtmDrainageList_expandTptrPolygonAtPointDtmObject(dtmP,&lowPoint,extStartPntP,extEndPntP)) goto errexit ;
/*
** Check Connectivity
*/
 if( cdbg )
   {
    if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,lowPoint,0))
      {
       bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
       goto errexit ;
      }
   }
/*
** Check And Reset Start Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Start Point") ;
 if( nodeAddrP(dtmP,*startPointP)->tPtr == dtmP->nullPnt ) *startPointP = lowPoint ;
/*
** Calculate Area And Direction Of Tptr Polygon
*/
 if( cdbg )
   {
    if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,*startPointP,&area,&direction)) goto errexit ;
    if( direction != DTMDirection::AntiClockwise || area < lastArea )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"lastArea = %12.4lf area = %12.4lf direction = %2ld",lastArea,area,direction) ;
       bcdtmWrite_message(1,0,0,"Area Of Tptr Polygon Has Decreased") ;
       goto errexit ;
      }
/*
**  Set Return Area
*/
    *areaP = area ;
   }
 /*
** Clean Up
*/
 cleanup :
/*
** Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Pond About Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Pond About Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_zeroSlopeSumpLineCompareFunction(const void* void1P , const void* void2P)
{
 DTM_ZERO_SLOPE_SUMP_LINE *line1P,*line2P ;
 line1P = ( DTM_ZERO_SLOPE_SUMP_LINE * ) void1P ;
 line2P = ( DTM_ZERO_SLOPE_SUMP_LINE * ) void2P ;
 if( line1P->point1 < line2P->point1 ) return(-1) ;
 if( line1P->point1 > line2P->point1 ) return( 1) ;
 if( line1P->point2 < line2P->point2 ) return(-1) ;
 if( line1P->point2 > line2P->point2 ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_determineZeroSlopeSumpLinePondsDtmObject
(
 BC_DTM_OBJ                *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables         *drainageTablesP,           // ==> Pointer To Drainage Tables
 DTMZeroSlopePolygonVector *zeroSlopePolygonsP,        // ==> Pointer To Zero Slope Polygons
 int                       *zeroSlopePointsIndexP,     // ==> Index To Zero Slope Polygons
 DTMFeatureCallback        loadFunctionP,              // ==> Pointer To Call Back Function
 bool                      loadFlag,                   // ==> Pass Pond Boundaries to User Call Back Function
 bool                      buildTable,                 // ==> Create Pond Tables For Later Use
 void                      *userP,                     // ==> User Pointer Passed Back To Call Back Function
 int&                      numSumpLinePonds            // <== Number Of Sump Line Ponds
)
{
/*
** Scan Tin For Internal Zero Slope Sump Lines
*/
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p1,p2,ap,cp,clc,exitPoint,priorPoint,nextPoint ;
 long   numZeroLines,numSumpLines=0,maxSumpLines=0 ;
 bool voidLine;
 long   startTime=bcdtmClock()  ;
 long   numZeroSlopeSumpLines=0,memZeroSlopeSumpLines=0,memZeroSlopeSumpLinesInc=1000 ;
 double elevation1,elevation2 ;
 DTM_TIN_NODE      *dP ;
 DTM_TIN_POINT     *pnt1P,*pnt2P ;
 DTM_POLYGON_OBJ   *polygonP=nullptr ;
 DTM_SUMP_LINES    *sumpLinesP=nullptr,*slP=nullptr ;
 DTM_ZERO_SLOPE_SUMP_LINE  *lineP,*zeroSlopeSumpLinesP=nullptr ;

// Log Function Parameters

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determing Zero Slope Sump Line Ponds") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP  = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP    = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"loadFlag         = %8ld",loadFlag) ;
    bcdtmWrite_message(0,0,0,"buildTable       = %8ld",buildTable) ;
   }

// Initialise

 numSumpLinePonds = 0 ;

// Test For Valid DTM Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

// Check DTM Is Triangulated

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }

// Null Tptr values  - Just In Case If Needed

 bcdtmList_nullTptrValuesDtmObject(dtmP) ;

// Create Table Of Zero Slope Lines

 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Zero Slope Line Table") ;
 startTime = bcdtmClock() ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    dP = nodeAddrP(dtmP,p1) ;
    if( ( clc = dP->cPtr) != dtmP->nullPtr )
      {
       pnt1P = pointAddrP(dtmP,p1) ;
       while ( clc != dtmP->nullPtr )
         {
          p2    = clistAddrP(dtmP,clc)->pntNum ;
          clc   = clistAddrP(dtmP,clc)->nextPtr ;
          pnt2P = pointAddrP(dtmP,p2) ;
          if( p2 > p1 && pnt1P->z == pnt2P->z )
            {
             if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,voidLine)) goto errexit ;
             if( ! voidLine )
               {
                if( ( ap = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                if( ( cp = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                if( ap != dtmP->nullPnt && cp != dtmP->nullPnt )
                  {
                   if( pointAddrP(dtmP,ap)->z > pnt1P->z && pointAddrP(dtmP,cp)->z > pnt1P->z )
                     {
                      if( memZeroSlopeSumpLines < numZeroSlopeSumpLines + 2 )
                        {
                         memZeroSlopeSumpLines = memZeroSlopeSumpLines + memZeroSlopeSumpLinesInc ;
                         if( zeroSlopeSumpLinesP == nullptr ) zeroSlopeSumpLinesP = ( DTM_ZERO_SLOPE_SUMP_LINE * ) malloc( memZeroSlopeSumpLines * sizeof( DTM_ZERO_SLOPE_SUMP_LINE )) ;
                         else                              zeroSlopeSumpLinesP = ( DTM_ZERO_SLOPE_SUMP_LINE * ) realloc( zeroSlopeSumpLinesP, memZeroSlopeSumpLines * sizeof( DTM_ZERO_SLOPE_SUMP_LINE )) ;
                         if( zeroSlopeSumpLinesP == nullptr )
                           {
                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                            goto errexit ;
                           }
                        }
                      (zeroSlopeSumpLinesP+numZeroSlopeSumpLines)->point1 = p1 ;
                      (zeroSlopeSumpLinesP+numZeroSlopeSumpLines)->point2 = p2 ;
                      (zeroSlopeSumpLinesP+numZeroSlopeSumpLines)->status = 1 ;
                      (zeroSlopeSumpLinesP+numZeroSlopeSumpLines)->flag   = 1 ;
                      ++numZeroSlopeSumpLines ;
                      (zeroSlopeSumpLinesP+numZeroSlopeSumpLines)->point1 = p2 ;
                      (zeroSlopeSumpLinesP+numZeroSlopeSumpLines)->point2 = p1 ;
                      (zeroSlopeSumpLinesP+numZeroSlopeSumpLines)->status = 1 ;
                      (zeroSlopeSumpLinesP+numZeroSlopeSumpLines)->flag   = 1 ;
                      ++numZeroSlopeSumpLines ;
                     }
                  }
               }
            }
         }
      }
   }

//  Resize Zero Sump Line Points Memory

 if( numZeroSlopeSumpLines > 0 && memZeroSlopeSumpLines > numZeroSlopeSumpLines )
   {
    memZeroSlopeSumpLines = numZeroSlopeSumpLines ;
    zeroSlopeSumpLinesP = ( DTM_ZERO_SLOPE_SUMP_LINE * ) realloc( zeroSlopeSumpLinesP, memZeroSlopeSumpLines * sizeof( DTM_ZERO_SLOPE_SUMP_LINE )) ;
   }

//  Sort Sump Line Points

 if( numZeroSlopeSumpLines > 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting %8ld Zero Slope Lines",numZeroSlopeSumpLines) ;
    qsort( ( DTM_ZERO_SLOPE_SUMP_LINE * ) zeroSlopeSumpLinesP,numZeroSlopeSumpLines,sizeof( DTM_ZERO_SLOPE_SUMP_LINE ), bcdtmDrainage_zeroSlopeSumpLineCompareFunction) ;
   }

//  Log Timing

 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Determine And Sort %8ld Zero Slope Lines = %8.3lf seconds",numZeroSlopeSumpLines,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

//  Scan And Concatenate Sump Lines

 numZeroLines = 0 ;
 startTime = bcdtmClock() ;
 for( lineP = zeroSlopeSumpLinesP ; lineP < zeroSlopeSumpLinesP + numZeroSlopeSumpLines ; ++lineP )
   {
    if( lineP->status )
      {
       if( cdbg )   // Development Only
         {
          elevation1 = pointAddrP(dtmP,lineP->point1)->z ;
          elevation2 = pointAddrP(dtmP,lineP->point2)->z ;
          if( elevation1 != elevation2 )
            {
             bcdtmWrite_message(1,0,0,"Different Elevations At Sump Line Ends") ;
             goto errexit ;
            }
         }

//     Concatenate Sump Lines

       if( bcdtmDrainage_concatenateZeroSlopeSumpLines(zeroSlopeSumpLinesP,numZeroSlopeSumpLines,(long)(lineP-zeroSlopeSumpLinesP),&sumpLinesP,&numSumpLines)) goto errexit ;

//     Check Sump Line End Points - Development Only

       if( cdbg )
         {
          for( slP = sumpLinesP ; slP < sumpLinesP + numSumpLines ; ++slP )
            {
             if( pointAddrP(dtmP,slP->sP1)->z != elevation1 || pointAddrP(dtmP,slP->sP2)->z != elevation1 )
               {
                bcdtmWrite_message(0,0,0,"Zero Line = %8ld ** numSumpLines = %8ld sumpLineOffset = %8ld",numZeroLines,numSumpLines,(long)(slP-sumpLinesP)) ;
                bcdtmWrite_message(0,0,0,"slP->sp1 = %8ld ** %12.5lf %12.5lf %10.4lf",slP->sP1,pointAddrP(dtmP,slP->sP1)->x,pointAddrP(dtmP,slP->sP1)->y,pointAddrP(dtmP,slP->sP1)->z) ;
                bcdtmWrite_message(0,0,0,"slP->sp2 = %8ld ** %12.5lf %12.5lf %10.4lf",slP->sP2,pointAddrP(dtmP,slP->sP2)->x,pointAddrP(dtmP,slP->sP2)->y,pointAddrP(dtmP,slP->sP2)->z) ;
                for( slP = sumpLinesP ; slP < sumpLinesP + numSumpLines ; ++slP )
                  {
                   bcdtmWrite_message(0,0,0,"SumpLine[%8ld] = %8ld %8ld",(long)(slP-sumpLinesP),slP->sP1,slP->sP2) ;
                  }
                bcdtmWrite_message(1,0,0,"SumP Lines elevations Not Correct") ;
                goto errexit ;
               }
            }
         }

//     Determine Pond About Sump Lines

       ++numZeroLines ;
       if( dbg && numSumpLines > maxSumpLines )
         {
          maxSumpLines = numSumpLines ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Max Sump Lines = %8ld",maxSumpLines) ;
         }
       if( bcdtmDrainage_determinePondAboutZeroSlopeSumpLinesDtmObject(dtmP,drainageTablesP,zeroSlopePolygonsP,zeroSlopePointsIndexP,sumpLinesP,numSumpLines,loadFunctionP,loadFlag,buildTable,&exitPoint,&priorPoint,&nextPoint,&polygonP,userP) ) goto errexit ;
       if( exitPoint != dtmP->nullPnt )
         {
          ++numSumpLinePonds ;
          if( buildTable )
            {
             for( slP = sumpLinesP ; slP < sumpLinesP + numSumpLines ; ++slP )
               {
                drainageTablesP->StoreZeroSlopeLinePond(slP->sP1,slP->sP2,exitPoint,priorPoint,nextPoint) ;
               }
            }
         }
       else
         {
          bcdtmWrite_message(2,0,0,"Zero Slope Line Pond Not Determined") ;
          goto errexit ;
         }
      }
   }

//  Log Times

 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Concatenate %8ld Zero Slope Lines = %8.3lf seconds",numZeroLines,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

// Cleanup ;

 cleanup :
 if( zeroSlopeSumpLinesP != nullptr ) { free(zeroSlopeSumpLinesP) ; zeroSlopeSumpLinesP = nullptr ; }
 if( polygonP            != nullptr ) bcdtmPolygon_deletePolygonObject(&polygonP) ;

// Normal Exit

 if( dbg && ret == 0 ) bcdtmWrite_message(0,0,0,"Determing Zero Slope Sump Line Ponds Completed") ;
 if( dbg && ret != 0 ) bcdtmWrite_message(0,0,0,"Determing Zero Slope Sump Line Ponds Error") ;
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
int bcdtmDrainage_nullZeroSumpLineEntry
(
 DTM_ZERO_SLOPE_SUMP_LINE  *zeroSumpLinesP,
 long                      numZeroSumpLines,
 long                      sumpPoint1,
 long                      sumpPoint2
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  ofs1,ofs2,ofs3 ;
 DTM_ZERO_SLOPE_SUMP_LINE *ofsP,*ofs1P,*ofs2P,*ofs3P ;

// Log Parameter Values

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Nulling Zero Sump Line Entry") ;
    bcdtmWrite_message(0,0,0,"zeroSumpLinesP   = %p",zeroSumpLinesP) ;
    bcdtmWrite_message(0,0,0,"numZeroSumpLines = %8ld",numZeroSumpLines) ;
    bcdtmWrite_message(0,0,0,"sumpPoint1       = %8ld",sumpPoint1) ;
    bcdtmWrite_message(0,0,0,"sumpPoint2       = %8ld",sumpPoint2) ;
   }

// Check For First

 ofsP  = nullptr ;
 ofs1P = zeroSumpLinesP ;
 ofs2P = zeroSumpLinesP + numZeroSumpLines - 1 ;
 if     ( ofs1P->point1 == sumpPoint1 && ofs1P->point2 == sumpPoint2 ) ofsP = ofs1P ;
 else if( ofs2P->point1 == sumpPoint1 && ofs2P->point2 == sumpPoint2 ) ofsP = ofs2P ;

// Binary Scan To Find Point Entry

 if( ofsP == nullptr )
   {
    ofs1 = 0 ;
    ofs2 = numZeroSumpLines - 1 ;
    while( ofs2 - ofs1 > 1 && ofsP == nullptr )
      {
       ofs3  = ( ofs2 + ofs1 ) / 2 ;
       ofs3P = zeroSumpLinesP + ofs3 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"ofs3 = %8ld // point1 = %8ld sp2 = %8ld",ofs3,ofs3P->point1,ofs3P->point2) ;
       if( sumpPoint1 == ofs3P->point1 && sumpPoint2 == ofs3P->point2 ) ofsP  = ofs3P ;
       else if( sumpPoint1 >  ofs3P->point1 ) ofs1  = ofs3  ;
       else if( sumpPoint1 <  ofs3P->point1 ) ofs2  = ofs3  ;
       else if( sumpPoint2 >  ofs3P->point2 ) ofs1  = ofs3  ;
       else if( sumpPoint2 <  ofs3P->point2 ) ofs2  = ofs3  ;
      }
   }

// Check For Entry Found

 if( ofsP != nullptr )
   {
    ofsP->status = 0 ;
   }
 else
   {
    bcdtmWrite_message(0,0,0,"sumpPoint1 = %8ld sumpPoint2 = %8ld",sumpPoint1,sumpPoint2) ;
    for( ofsP = zeroSumpLinesP ; ofsP < zeroSumpLinesP + numZeroSumpLines ; ++ofsP )
      {
       if( ofsP->point1 == sumpPoint1 )
         {
          bcdtmWrite_message(0,0,0,"ofs = %8ld // ofsP->point1 = %8ld ofsP->point2 = %8ld",(long)(ofsP-zeroSumpLinesP),ofsP->point1,ofsP->point2) ;
         }
      }
    bcdtmWrite_message(2,0,0,"Sump Line Entry Not Found") ;
    goto errexit ;
   }

// Clean Up

 cleanup :

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Nulling Sump Line Entry Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Nulling Sump Line Entry Error") ;
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
int bcdtmDrainage_getZeroSumpLineOffsets
(
 DTM_ZERO_SLOPE_SUMP_LINE    *zeroSumpLinesP,
 long                        numZeroSumpLines,
 long                        sumpPoint,
 long                        *startOffsetP,
 long                        *endOffsetP
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  ofs1,ofs2,ofs3 ;
 DTM_ZERO_SLOPE_SUMP_LINE *ofsP,*ofs1P,*ofs2P,*ofs3P ;

// Log Parameter Values

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Zero Sump Line Offsets") ;
    bcdtmWrite_message(0,0,0,"zeroSumpLinesP   = %p",zeroSumpLinesP) ;
    bcdtmWrite_message(0,0,0,"numZeroSumpLines = %8ld",numZeroSumpLines) ;
    bcdtmWrite_message(0,0,0,"sumpPoint        = %8ld",sumpPoint) ;
   }

// Initialise

 *startOffsetP = -1 ;
 *endOffsetP   = -1 ;

// Check For First

 ofsP  = nullptr ;
 ofs1P = zeroSumpLinesP ;
 ofs2P = zeroSumpLinesP + numZeroSumpLines - 1 ;
 if     ( ofs1P->point1 == sumpPoint ) ofsP = ofs1P ;
 else if( ofs2P->point1 == sumpPoint ) ofsP = ofs2P ;

// Binary Scan To Find Point Entry

 if( ofsP == nullptr )
   {
    ofs1 = 0 ;
    ofs2 = numZeroSumpLines - 1 ;
    while( ofs2 - ofs1 > 1 && ofsP == nullptr )
      {
       ofs3  = ( ofs2 + ofs1 ) / 2 ;
       ofs3P = zeroSumpLinesP + ofs3 ;
       if( sumpPoint == ofs3P->point1 ) ofsP  = ofs3P ;
       if( sumpPoint >  ofs3P->point1 ) ofs1  = ofs3  ;
       if( sumpPoint <  ofs3P->point1 ) ofs2  = ofs3  ;
      }
   }

// Check For Entry Found

 if( ofsP != nullptr )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Second Point Found") ;
    ofs1P = ofsP ;
    while( ofs1P >= zeroSumpLinesP && ofs1P->point1 == sumpPoint ) --ofs1P ;
    ++ofs1P ;
    ofs2P = ofsP ;
    while( ofs2P < zeroSumpLinesP + numZeroSumpLines && ofs2P->point1 == sumpPoint ) ++ofs2P ;
    --ofs2P ;
    *startOffsetP = (long)(ofs1P-zeroSumpLinesP) ;
    *endOffsetP   = (long)(ofs2P-zeroSumpLinesP) ;

//  Check Offset Sump Lines Are Correct - Developement Only

    if( cdbg )
      {
       for( ofsP = zeroSumpLinesP + *startOffsetP ; ofsP <= zeroSumpLinesP + *endOffsetP ; ++ofsP )
         {
          if( ofsP->point1 != sumpPoint || ofsP->status != 1 )
            {
             bcdtmWrite_message(0,0,0,"sumpPoint = %8ld // startOffset = %8ld endOffset = %8ld",sumpPoint,*startOffsetP,*endOffsetP) ;
             for( ofsP = zeroSumpLinesP + *startOffsetP ; ofsP <= zeroSumpLinesP + *endOffsetP ; ++ofsP )
               {
                bcdtmWrite_message(0,0,0,"point1 = %8ld point2 = %8ld status = %2ld",ofsP->point1,ofsP->point2,ofsP->status) ;
               }
             bcdtmWrite_message(2,0,0,"Sump Line Offsets Error") ;
             goto errexit ;
            }
         }
      }
   }

// Clean Up

 cleanup :

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Zero Sump Line Offsets Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Zero Sump Line Offsets Error") ;
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
+------------------------------------------------------------------- */
int bcdtmDrainage_concatenateZeroSlopeSumpLines
(
 DTM_ZERO_SLOPE_SUMP_LINE *zeroSumpLinesP,
 long                     numZeroSumpLines,
 long                     sumpPtsOffset,
 DTM_SUMP_LINES           **sumpLinesPP,
 long                     *numSumpLinesP
)

// This Function Concatenates Zero Slope Sump Lines

{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 char  *sumpFlagP=nullptr ;
 long  ofs,startOffset,endOffset ;
 long  numSumpLines=0,memSumpLines=100,memSumpLinesInc=100 ;
 DTM_ZERO_SLOPE_SUMP_LINE *sptP,*spt1P ;
 DTM_SUMP_LINES *stackPtrP,*slP ;

// Write Line Debug Only


if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Concatenating Zero Slope Sump Lines") ;
    bcdtmWrite_message(0,0,0,"zeroSumpLinesP   = %p",zeroSumpLinesP) ;
    bcdtmWrite_message(0,0,0,"numZeroSumpLines = %8ld",numZeroSumpLines) ;
    bcdtmWrite_message(0,0,0,"sumpPtsOffset    = %8ld",sumpPtsOffset) ;
   }

// Initialise

 *numSumpLinesP = 0 ;
 if( *sumpLinesPP != nullptr )
   {
    free(*sumpLinesPP) ;
    *sumpLinesPP = nullptr ;
   }

//  Allocate Initial Memory For Concatenated Sump Lines

 *sumpLinesPP = ( DTM_SUMP_LINES *) malloc(memSumpLines*sizeof(DTM_SUMP_LINES)) ;
 if( *sumpLinesPP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }

// Store The First Sump Line

 sptP = zeroSumpLinesP + sumpPtsOffset ;
 (*sumpLinesPP+*numSumpLinesP)->sP1 =  sptP->point1 ;
 (*sumpLinesPP+*numSumpLinesP)->sP2 =  sptP->point2 ;
 sptP->status = 0 ;
 ++*numSumpLinesP ;

// Copy All Sump Lines That Have The Same First Point

 spt1P = sptP + 1 ;
 while( spt1P < zeroSumpLinesP + sumpPtsOffset && spt1P->point1 == sptP->point1 )
   {
    if( spt1P->status )
      {
      if (*numSumpLinesP == memSumpLines)
          {
          memSumpLines *= 1.5;
          *sumpLinesPP = ( DTM_SUMP_LINES *)realloc (*sumpLinesPP, memSumpLines*sizeof(DTM_SUMP_LINES));
          }
       (*sumpLinesPP+*numSumpLinesP)->sP1 =  spt1P->point1 ;
       (*sumpLinesPP+*numSumpLinesP)->sP2 =  spt1P->point2 ;
       spt1P->status = 0 ;
       ++*numSumpLinesP ;
      }
   }

// Log Initial Sump Points

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Initial Number Of Concatenated Sump Lines = %8ld",*numSumpLinesP) ;
    for( slP = *sumpLinesPP ; slP < *sumpLinesPP + *numSumpLinesP ; ++slP )
      {
       bcdtmWrite_message(0,0,0,"Sump Line[%8ld] = %8ld %8ld",(long)(slP-*sumpLinesPP),slP->sP1,slP->sP2) ;
      }
   }

// Copy All Sump Lines That The Same Second Point

 stackPtrP =  *sumpLinesPP ;
 while( stackPtrP < *sumpLinesPP+*numSumpLinesP )
   {

    // Get Offsets For Second Point

    if( bcdtmDrainage_getZeroSumpLineOffsets(zeroSumpLinesP,numZeroSumpLines,stackPtrP->sP2,&startOffset,&endOffset)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"startOffset = %8ld endOffset = %8ld",startOffset,endOffset) ;
    if( startOffset >= 0 )
      {
       for( ofs = startOffset ; ofs <= endOffset ; ++ofs )
         {
          sptP = zeroSumpLinesP + ofs ;
          if( sptP->status )
            {
             if (*numSumpLinesP == memSumpLines)
                 {
                 memSumpLines *= 1.5;
                 long stackPtrIndex = stackPtrP - *sumpLinesPP;
                 *sumpLinesPP = ( DTM_SUMP_LINES *)realloc (*sumpLinesPP, memSumpLines*sizeof(DTM_SUMP_LINES));
                 stackPtrP = *sumpLinesPP + stackPtrIndex;
                 }
             (*sumpLinesPP+*numSumpLinesP)->sP1 =  sptP->point1 ;
             (*sumpLinesPP+*numSumpLinesP)->sP2 =  sptP->point2 ;
             sptP->status = 0 ;
             ++*numSumpLinesP ;
            }
          if( bcdtmDrainage_nullZeroSumpLineEntry(zeroSumpLinesP,numZeroSumpLines,sptP->point2,sptP->point1)) goto errexit ;
         }
      }
    ++stackPtrP ;
   }

// Log Number Of Concated Sump Lines

 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Concatenated Sump Lines = %8ld",*numSumpLinesP) ;

// Clean Up

 cleanup :

// Return

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
int bcdtmDrainage_determinePondAboutZeroSlopeSumpLinesDtmObject
(
 BC_DTM_OBJ                *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables         *drainageTablesP,           // ==> Pointer To Drainage Tables
 DTMZeroSlopePolygonVector *zeroSlopePolygonsP,        // ==> Pointer To Zero Slope Polygons
 int                       *zeroSlopePointsIndexP,     // ==> Index To Zero Slope Polygons
 DTM_SUMP_LINES            *sumpLinesP,                // ==> Pointer To List Of Connecting Zero Slope Sump Lines
 long                      numSumpLines,               // ==> Number Of Zero Slope Sump Lines
 DTMFeatureCallback        loadFunctionP,              // ==> Pointer To Call Back Function
 bool                      loadFlag,                   // ==> Flag To Pass Back Pond Boundary To Calling Method
 bool                      boundaryFlag,               // ==> Flag To Save Pond Boundary In Polygon Object
 long                      *exitPointP,                // <== Pond Exit Point
 long                      *priorPointP,               // <== Prior Point On Tptr Polygon To Exit Point
 long                      *nextPointP,                // <== Next Point On Tptr Polygon From Exit Point
 DTM_POLYGON_OBJ           **polygonPP,                // <== Pointer To Polygom Object With Pond Bounday Coordinates
 void                      *userP                      // ==> User Pointer Passed Back To Calling Method Via the Call Back Function
)
/*
** This Function Determines The Pond About A Sump Line
*/
{
 int            ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long           ap,np,sp,numRemoved,startPoint,belowSump=TRUE ;
 long           expansionTime ;
 double         elevation ;
 DTM_SUMP_LINES *slP ;
 static int pondCount=-1 ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determing Pond About Zero Slope Sump Lines") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"sumpLinesP    = %p",sumpLinesP) ;
    bcdtmWrite_message(0,0,0,"numSumpLines  = %8ld",numSumpLines) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"loadFlag      = %8ld",loadFlag) ;
    bcdtmWrite_message(0,0,0,"boundaryFlag  = %8ld",boundaryFlag) ;
    if( dbg == 1 )
      {
       for( slP = sumpLinesP ; slP < sumpLinesP + numSumpLines ; ++slP )
         {
          bcdtmWrite_message(0,0,0,"Sump Line[%8ld] = %8ld %8ld",(long)(slP-sumpLinesP),slP->sP1,slP->sP2) ;
          bcdtmWrite_message(0,0,0,"**** sP1 = %8ld ** %10.4lf %10.4lf %10.4lf",slP->sP1,pointAddrP(dtmP,slP->sP1)->x,pointAddrP(dtmP,slP->sP1)->y,pointAddrP(dtmP,slP->sP1)->z) ;
          bcdtmWrite_message(0,0,0,"**** sP2 = %8ld ** %10.4lf %10.4lf %10.4lf",slP->sP2,pointAddrP(dtmP,slP->sP2)->x,pointAddrP(dtmP,slP->sP2)->y,pointAddrP(dtmP,slP->sP2)->z) ;
         }
      }
   }
/*
** Initialise
*/
 *exitPointP  = dtmP->nullPnt ;
 *priorPointP = dtmP->nullPnt ;
 *nextPointP  = dtmP->nullPnt ;
 ++pondCount ;
/*
** Write Out Sump Lines
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"List Of Connecting Sump Lines") ;
    for( slP = sumpLinesP ; slP < sumpLinesP + numSumpLines ; ++slP )
      {
       bcdtmWrite_message(0,0,0,"Sump Line[%6ld] = %6ld %6ld",(long)(slP-sumpLinesP),slP->sP1,slP->sP2) ;
      }
   }

// Check For Zero Slope Sump Line Internal To A Zero Slope Polygon

 if( zeroSlopePointsIndexP != nullptr )
   {
    for( slP = sumpLinesP ; slP < sumpLinesP + numSumpLines && *exitPointP == dtmP->nullPnt ; ++slP )
      {
       if( *(zeroSlopePointsIndexP+slP->sP1) != dtmP->nullPnt )
         {
          DTMZeroSlopePolygonVector::iterator zsp = zeroSlopePolygonsP->begin() + *(zeroSlopePointsIndexP+slP->sP1) ;
          *priorPointP = zsp->priorPoint ;
          *exitPointP  = zsp->exitPoint  ;
          *nextPointP  = zsp->nextPoint  ;
         }
       else if( *(zeroSlopePointsIndexP+slP->sP2) != dtmP->nullPnt )
         {
          DTMZeroSlopePolygonVector::iterator zsp = zeroSlopePolygonsP->begin() + *(zeroSlopePointsIndexP+slP->sP2) ;
          *priorPointP = zsp->priorPoint ;
          *exitPointP  = zsp->exitPoint  ;
          *nextPointP  = zsp->nextPoint  ;
         }
      }
   }
/*
** Determine If There Is Flow Out From A The Zero Slope Sump Line Point
*/
 if( *exitPointP == dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Flow Out From A The Zero Slope Sump Line Point") ;
    if( bcdtmDrainage_checkForFlowOutFromSumpLinePointsDtmObject(dtmP,drainageTablesP,sumpLinesP,numSumpLines,exitPointP,priorPointP,nextPointP)) goto errexit ;
   }
/*
** No Flow Out Point So Determine Pond About Sump Lines
*/
 if( *exitPointP == dtmP->nullPnt )
   {
/*
** Place Polygon Around Sump Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Placing Polygon Around Sump Lines") ;
    if( bcdtmDrainage_placePolygonAroundSumpLinesDtmObject(dtmP,sumpLinesP,numSumpLines,&startPoint) ) goto errexit ;
    if( dbg  ) bcdtmList_writeTptrListDtmObject(dtmP,startPoint) ;
/*
**  Check Tptr Polygon Points Are Above Sump Points
*/
    if( cdbg )
      {
       belowSump = FALSE ;
       sp = startPoint ;
       do
         {
          if( pointAddrP(dtmP,sp)->z < pointAddrP(dtmP,sumpLinesP->sP1)->z ) belowSump = TRUE ;
          sp = nodeAddrP(dtmP,sp)->tPtr ;
         } while ( sp != startPoint && belowSump == FALSE ) ;
       if( belowSump == TRUE )
         {
          bcdtmWrite_message(2,0,0,"Tptr Polygon Points Are Below Sump Line Points") ;
          goto errexit ;
         }
      }

//  Remove Zero Slope Triangles On Tptr Polygon Edge

    numRemoved =  0 ;
    sp = startPoint ;
    do
      {
       np = nodeAddrP(dtmP,sp)->tPtr ;
       if( pointAddrP(dtmP,sp)->z == pointAddrP(dtmP,np)->z )
         {
          if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
          if( pointAddrP(dtmP,ap)->z == pointAddrP(dtmP,sp)->z )
            {
             if(  nodeAddrP(dtmP,ap)->tPtr == dtmP->nullPnt )
               {
                ++numRemoved ;
                nodeAddrP(dtmP,sp)->tPtr = ap ;
                nodeAddrP(dtmP,ap)->tPtr = np ;
               }
             else if ( nodeAddrP(dtmP,np)->tPtr == ap )
               {
                ++numRemoved ;
                nodeAddrP(dtmP,sp)->tPtr = ap ;
                nodeAddrP(dtmP,np)->tPtr = dtmP->nullPnt ;
                if( startPoint == np ) startPoint = sp ;
               }
             else if ( nodeAddrP(dtmP,ap)->tPtr == sp )
               {
                ++numRemoved ;
                nodeAddrP(dtmP,ap)->tPtr = np ;
                nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
                if( startPoint == sp ) startPoint = ap ;
               }
            }
         }
       sp = nodeAddrP(dtmP,sp)->tPtr ;
      } while ( sp != startPoint ) ;

    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Number Of Zero Slope Triangles Removed = %8ld",numRemoved) ;

//  Expand To Exit Point

    if( dbg ) bcdtmWrite_message(0,0,0,"Expanding Polygon To Exit Point") ;
    elevation = pointAddrP(dtmP,startPoint)->z ;
    expansionTime = bcdtmClock() ;
    if( bcdtmDrainage_expandPondToExitPointDtmObject(dtmP,drainageTablesP,zeroSlopePolygonsP,zeroSlopePointsIndexP,startPoint,exitPointP,priorPointP,nextPointP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Prior Point = %6ld Exit Point = %6ld  Next Point = %6ld",*priorPointP,*exitPointP,*nextPointP) ;
    if( *exitPointP == dtmP->nullPnt )
      {
       bcdtmWrite_message(2,0,0,"Pond Exit Point About Zero Slope Sump Line Not Determined") ;
       goto errexit ;
      }

//  Draw Pond Boundaries

    if( ( loadFlag || boundaryFlag ) && pointAddrP(dtmP,*exitPointP)->z > elevation )
      {
       if( bcdtmDrainage_extractPondBoundaryDtmObject(dtmP,pointAddrP(dtmP,*exitPointP)->z,*exitPointP,*nextPointP,loadFunctionP,loadFlag,boundaryFlag,polygonPP, userP)) goto errexit ;
      }

//  Null Out Tptr Polygon

    bcdtmList_nullTptrListDtmObject(dtmP,*exitPointP) ;
   }
/*
** Check For None Null Pointer Values
*/
 if( cdbg )
   {
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
    bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtmP,1) ;
   }
/*
** Log Exit Point
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Prior Point = %6ld Exit Point = %6ld  Next Point = %6ld",*priorPointP,*exitPointP,*nextPointP) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Normal Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determing Pond About Zero Slope Sump Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determing Pond About Zero Slope Sump Lines Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 bcdtmWrite_message(0,0,0,"pondCount = %8ld",pondCount) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_getSumpLineOffsetDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       sumpPoint1,
 long       sumpPoint2,
 long       *sumpLineOffsetP
)
/*
** This Function Gets The Tin Line Offset For A Sump Line In A Tin Object
** The Lowest Value Sump Point Number Is Placed First
*/
{
 int  ret=DTM_SUCCESS ;
 long sp,clPtr ;
/*
** Initialise
*/
 *sumpLineOffsetP = dtmP->nullPnt ;
 if( sumpPoint1 > sumpPoint2 ) { sp = sumpPoint1 ; sumpPoint1 = sumpPoint2 ; sumpPoint2 = sp ; }
 if( sumpPoint1 < 0 || sumpPoint1 >= dtmP->numPoints ) goto errexit ;
 if( ( clPtr = nodeAddrP(dtmP,sumpPoint1)->cPtr) == dtmP->nullPtr ) goto errexit ;
/*
** Scan Cyclic List For sumpPoint1 Looking For sumpPoint2
*/
 while ( clPtr != dtmP->nullPtr && *sumpLineOffsetP == dtmP->nullPnt )
   {
    if(clistAddrP(dtmP,clPtr)->pntNum == sumpPoint2 ) *sumpLineOffsetP = clPtr ;
    else                                        clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
   }
/*
** Check Sump Line Offset Found
*/
 if( *sumpLineOffsetP == dtmP->nullPnt )
   {
    bcdtmWrite_message(2,0,0,"Sump Line Offset Not Found") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Normal Exit
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 int               *zeroSlopePointsIndexP,     // ==> Index To Zero Slope Polygons
 DTMFeatureCallback loadFunctionP,             // ==> Pointer To Call Back Function
 long              sP1,                        // ==> Point At One End Of Zero Slope Sump Line
 long              sP2,                        // ==> Point At Other End Of Zero Slope Sump Line
 bool              loadFlag,                   // ==> Flag To Pass The Pond Boundary Back To The call Back Function
 bool              boundaryFlag,               // ==> Flag To Pass The Pond Cordinates Back In A Polygon Object
 long              *exitPointP,                // <== Exit Point On Tptr Pond Polygon
 long              *priorPointP,               // <== Prior Point To Exit Point On Tptr Pond Polygon
 long              *nextPointP,                // <== Next Point From Exit Point On Tptr Pond Polygon
 DTM_SUMP_LINES    **sumpLinesPP,              // <== Pointer To All Zero Slope Sump Lines In Pond
 long              *numSumpLinesP,             // <== Number Of Zero Slope Sump Lines In Pond
 DTM_POLYGON_OBJ   **polygonPP,                // <== Pointer To Polygon Object Containing The Pond Boundary
 void              *userP,                     // <== User Pointer Passed Backed To Call Back Function
 double            *areaP                      // <==  Pond Area
)
/*
** This Function Determines The Pond About A Sump Line
*/
{
 int            ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long           sp,startPoint,belowSump=TRUE ;
 DTMDirection   direction ;
 DTM_SUMP_LINES *sumpLineP ;
/*
** Log Function Parameters
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determing Pond About Zero Slope Sump Line") ;
    bcdtmWrite_message(0,0,0,"dtmp            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"loadFlag        = %8ld",loadFlag) ;
    bcdtmWrite_message(0,0,0,"boundaryFlag    = %8ld",boundaryFlag) ;
    bcdtmWrite_message(0,0,0,"sP1             = %8ld ** %10.4lf %10.4lf %10.4lf",sP1,pointAddrP(dtmP,sP1)->x,pointAddrP(dtmP,sP1)->y,pointAddrP(dtmP,sP1)->z) ;
    bcdtmWrite_message(0,0,0,"sP2             = %8ld ** %10.4lf %10.4lf %10.4lf",sP2,pointAddrP(dtmP,sP2)->x,pointAddrP(dtmP,sP2)->y,pointAddrP(dtmP,sP2)->z) ;
   }
/*
** Initialise
*/
 *areaP = 0.0 ;
 *exitPointP  = dtmP->nullPnt ;
 *priorPointP = dtmP->nullPnt ;
 *nextPointP  = dtmP->nullPnt ;
 *numSumpLinesP = 0 ;
 if( *sumpLinesPP != nullptr ) { free(*sumpLinesPP) ; *sumpLinesPP = nullptr ; }
 if( *polygonPP   != nullptr ) bcdtmPolygon_deletePolygonObject(polygonPP) ;
/*
** Concatenate Zero Slope Sump Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Concatening Zero Slope Sump Lines") ;
 if( bcdtmDrainage_concatenateZeroSlopeSumpLinesDtmObject(dtmP,sP1,sP2,sumpLinesPP,numSumpLinesP)) goto errexit ;
/*
** Write Out Sump Lines
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"List Of Connecting Sump Lines") ;
    for( sumpLineP = *sumpLinesPP ; sumpLineP < *sumpLinesPP + *numSumpLinesP ; ++sumpLineP )
      {
       bcdtmWrite_message(0,0,0,"Sump Line[%6ld] = %6ld %6ld",(long)(sumpLineP-*sumpLinesPP),sumpLineP->sP1,sumpLineP->sP2) ;
      }
   }
/*
** Report None Null Tptr Values
*/
/*
** Determine If There Is Flow Out From A The Zero Slope Sump Line Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Flow Out From A The Zero Slope Sump Line Point") ;
 if( bcdtmDrainage_checkForFlowOutFromSumpLinePointsDtmObject(dtmP,drainageTablesP,*sumpLinesPP,*numSumpLinesP,exitPointP,priorPointP,nextPointP)) goto errexit ;
/*
** If Flow Out And Load Flag Write Sump Lines As Pond
*/
 if( *exitPointP != dtmP->nullPnt && loadFlag )
   {
/*
**  RobC  16Mar2011 Commented Out As It Creates Zero Area Ponds
**
    DTMPointCache pointCache ;
    for( sumpLineP = *sumpLinesPP ; sumpLineP < *sumpLinesPP + *numSumpLinesP ; ++sumpLineP )
      {
       if( pointCache.StorePointInCache(pointAddrP(dtmP,sumpLineP->sP1)->x,pointAddrP(dtmP,sumpLineP->sP1)->y,pointAddrP(dtmP,sumpLineP->sP1)->z)) goto errexit ;
       if( pointCache.StorePointInCache(pointAddrP(dtmP,sumpLineP->sP2)->x,pointAddrP(dtmP,sumpLineP->sP2)->y,pointAddrP(dtmP,sumpLineP->sP2)->z)) goto errexit ;
       if( pointCache.CallUserDelegateWithCachePoints(loadFunctionP,DTMFeatureType::LowPointPond,(DTMUserTag)2,dtmP->nullFeatureId,userP)) goto errexit ;
       pointCache.ClearCache() ;
      }
*/
   }
/*
** No Flow Out Point So Determine Pond About Sump Lines
*/
 if( *exitPointP == dtmP->nullPnt )
   {
/*
** Place Polygon Around Sump Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Placing Polygon Around Sump Lines") ;
    if( bcdtmDrainage_placePolygonAroundSumpLinesDtmObject(dtmP,*sumpLinesPP,*numSumpLinesP,&startPoint) ) goto errexit ;
    if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,startPoint) ;
/*
**  Check Tptr Polygon Points Are Above Sump Points
*/
    belowSump = FALSE ;
    sp = startPoint ;
    do
      {
       if( pointAddrP(dtmP,sp)->z <= pointAddrP(dtmP,sP1)->z ) belowSump = TRUE ;
       sp = nodeAddrP(dtmP,sp)->tPtr ;
      } while ( sp != startPoint && belowSump == FALSE ) ;
/*
** Determine Exit Point From Polygon
*/
    if( belowSump == FALSE )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Expanding Polygon To Exit Point") ;
       if( bcdtmDrainage_expandPondToExitPointDtmObject(dtmP,drainageTablesP,nullptr,zeroSlopePointsIndexP,startPoint,exitPointP,priorPointP,nextPointP)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Prior Point = %6ld Exit Point = %6ld  Next Point = %6ld",*priorPointP,*exitPointP,*nextPointP) ;
       if( *exitPointP == dtmP->nullPnt )
         {
          bcdtmWrite_message(2,0,0,"Pond Exit Point About Zero Slope Sump Line Not Determined") ;
          goto errexit ;
         }
/*
**     Draw Pond Boundaries
*/
       if( loadFlag || boundaryFlag )
         {
          if( bcdtmDrainage_extractPondBoundaryDtmObject(dtmP,pointAddrP(dtmP,*exitPointP)->z,*exitPointP,*nextPointP,loadFunctionP,loadFlag,boundaryFlag,polygonPP, userP)) goto errexit ;
         }
/*
**     Mark Zero Slope Sump Lines Within Pond Boundary
*/
       if( bcdtmDrainage_markZeroSlopeSumpLinesWithinTptrPolygonDtmObject(dtmP,*exitPointP,sumpLinesPP,numSumpLinesP)) goto errexit ;
/*
**     Null Out Tptr Polygon
*/
       if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,*exitPointP) ;
       bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,*exitPointP,areaP,&direction) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Polygon Area = %15.5lf",*areaP) ;
       bcdtmList_nullTptrListDtmObject(dtmP,*exitPointP) ;
      }
    else bcdtmList_nullTptrListDtmObject(dtmP,startPoint) ;
   }

//  Check And Report None Null Tptr Values

 if( dbg )
   {
    int node ;
    DTM_TIN_NODE *nodeP ;
    for( node = 0 ; node < dtmP->numPoints ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr != dtmP->nullPnt || nodeP->sPtr != dtmP->nullPnt )
         {
          bcdtmWrite_message(0,0,0,"node->tPtr = %10ld node->sPtr = %10ld ** sP1 = %8ld sP2 = %8ld",nodeP->tPtr,nodeP->tPtr,sP1,sP2) ;
          goto errexit ;
         }
      }
   }
/*
** Check For None Null Pointer Values
*/
 if( cdbg )
   {
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
    bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtmP,1) ;
   }


/*
** Clean Up
*/
 cleanup :
/*
** Normal Exit
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"sP1 = %8ld sP2 = %8ld",sP1,sP2) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_sumpPointsCompareFunction(const void *c1P,const void *c2P)
/*
** Compare Function For Qsort Of Sump Points
*/
{
 struct SUMPPTS { long sP1,sP2,Flag,Status ; } *sumpPnt1P,*sumpPnt2P ;
 sumpPnt1P = ( struct SUMPPTS * ) c1P ;
 sumpPnt2P = ( struct SUMPPTS * ) c2P ;
 if     (  sumpPnt1P->sP1  <  sumpPnt2P->sP1  ) return(-1) ;
 else if(  sumpPnt1P->sP1  >  sumpPnt2P->sP1  ) return( 1) ;
 else if(  sumpPnt1P->sP2  <  sumpPnt2P->sP2  ) return(-1) ;
 else if(  sumpPnt1P->sP2  >  sumpPnt2P->sP2  ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_concatenateZeroSlopeSumpLinesDtmObject
(
 BC_DTM_OBJ        *dtmP,
 long              sP1,
 long              sP2,
 DTM_SUMP_LINES    **sumpLinesPP,
 long              *numSumpLinesTableP
)
/*
** This Function Concatenates Zero Slope Sump Lines
*/
{
 int            ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 unsigned char  *cP,*sumpFlagP=nullptr ;
 long           ap,cp,p1,p2,p3,offset,stackPtr,numSumpFlag=0,sumpPtsNum=0,sumPtsMem=0,sumpPtsMemInc=1000 ;
 DTM_SUMP_LINES *sumplinesP ;
 struct SUMPPTS { long sP1,sP2,flag,status ; } *sumpPtsP=nullptr,*sptsP,*spts1P ;
/*
** Write Line Debug Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Concatenating Zero Slope Sump Lines") ;
    bcdtmWrite_message(0,0,0,"dtmP  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"sP1   = %8ld ** %10.4lf %10.4lf %10.4lf",sP1,pointAddrP(dtmP,sP1)->x,pointAddrP(dtmP,sP1)->y,pointAddrP(dtmP,sP1)->z) ;
    bcdtmWrite_message(0,0,0,"sP2   = %8ld ** %10.4lf %10.4lf %10.4lf",sP2,pointAddrP(dtmP,sP2)->x,pointAddrP(dtmP,sP2)->y,pointAddrP(dtmP,sP2)->z) ;
   }
/*
** Initialise
*/
 *numSumpLinesTableP = 0 ;
 if( *sumpLinesPP != nullptr ) { bcdtmWrite_message(2,0,0,"Sump Lines Not nullptr") ; goto errexit ; }
/*
** Allocate And Initialise Memory For Sump Line Flags
*/
 numSumpFlag = dtmP->cListPtr / 8 + 1 ;
 sumpFlagP = ( unsigned char * ) malloc( numSumpFlag * sizeof(char)) ;
 if( sumpFlagP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( cP = sumpFlagP ; cP < sumpFlagP + numSumpFlag ; ++cP ) *cP = 0 ;
/*
** Allocate And Initialise Memory For Sump Line Points
*/
 sumPtsMem = sumpPtsMemInc ;
 sumpPtsP   = ( struct SUMPPTS * ) malloc ( sumPtsMem * sizeof(struct SUMPPTS)) ;
 if( sumpPtsP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( sptsP = sumpPtsP ; sptsP < sumpPtsP + sumPtsMem ; ++sptsP )
   {
    sptsP->sP1 = sptsP->sP2 = dtmP->nullPnt ;
    sptsP->flag = 0 ;
   }
/*
** Add Initial Sump Line To Stack
*/
 sumpPtsP->sP1    = sP1 ;
 sumpPtsP->sP2    = sP2 ;
 sumpPtsP->flag   = 1   ;
 sumpPtsP->status = 1   ;
 (sumpPtsP+1)->sP1    = sP2 ;
 (sumpPtsP+1)->sP2    = sP1 ;
 (sumpPtsP+1)->flag   = 2   ;
 (sumpPtsP+1)->status = 2   ;
 sumpPtsNum = 2 ;
 if( bcdtmDrainage_getSumpLineOffsetDtmObject(dtmP,sP1,sP2,&offset) ) goto errexit ;
 bcdtmFlag_setFlag(sumpFlagP,offset) ;
/*
** Scan Sump Lines On Stack And Look For Other Connecting Sump Lines
*/
 stackPtr = 0 ;
 while ( stackPtr < sumpPtsNum )
   {
    p1 = (sumpPtsP+stackPtr)->sP1 ;
    p2 = (sumpPtsP+stackPtr)->sP2 ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Scanning Point %8ld From Point %8ld",p1,p2) ;
    if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
    while ( p3 != p2 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"p3 = %8ld ** %12.5lf %12.5lf %10.4lf",p3,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p3)->z) ;
       if(pointAddrP(dtmP,p3)->z == pointAddrP(dtmP,p1)->z )
         {
          if( ( ap = bcdtmList_nextAntDtmObject(dtmP,p1,p3)) < 0 ) goto errexit ;
          if( ( cp = bcdtmList_nextClkDtmObject(dtmP,p1,p3)) < 0 ) goto errexit ;
          if( ! bcdtmList_testLineDtmObject(dtmP,ap,p3)) ap = dtmP->nullPnt ;
          if( ! bcdtmList_testLineDtmObject(dtmP,cp,p3)) cp = dtmP->nullPnt ;
          if( ap != dtmP->nullPnt && cp != dtmP->nullPnt )
            {
//             if( pointAddrP(dtmP,ap)->z > pointAddrP(dtmP,p1)->z && pointAddrP(dtmP,cp)->z > pointAddrP(dtmP,p1)->z )
//               {
/*
**              Check If Zero Sump Line Already Processed
*/
                if( bcdtmDrainage_getSumpLineOffsetDtmObject(dtmP,p1,p3,&offset) ) goto errexit ;
                if( ! bcdtmFlag_testFlag(sumpFlagP,offset) )
                  {
/*
**                 Check For Memory Realloaction
*/
                   if( sumpPtsNum == sumPtsMem )
                     {
                      sumPtsMem = sumPtsMem + sumpPtsMemInc ;
                      sumpPtsP   = ( struct SUMPPTS * ) realloc ( sumpPtsP,sumPtsMem * sizeof(struct SUMPPTS)) ;
                      if( sumpPtsP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
                      for( sptsP = sumpPtsP + sumpPtsNum ; sptsP < sumpPtsP + sumPtsMem ; ++sptsP )
                        {
                         sptsP->sP1 = dtmP->nullPnt ;
                         sptsP->sP2 = dtmP->nullPnt ;
                         sptsP->flag = 0 ;
                        }
                     }
/*
**                 Add Line To Stack
*/
                   (sumpPtsP+sumpPtsNum)->sP1    = p3 ;
                   (sumpPtsP+sumpPtsNum)->sP2    = p1 ;
                   (sumpPtsP+sumpPtsNum)->status = 1  ;
                   ++sumpPtsNum         ;
                   bcdtmFlag_setFlag(sumpFlagP,offset) ;
                  }
//               }
            }
         }
/*
** Get Next Point About Sump Line
*/
       if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p3)) < 0 ) goto errexit ;
      }
/*
** Reset For Next Sump Line
*/
    ++stackPtr ;
   }
/*
** Write Zero Slope Sump Lines On Stack
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"List Of Connected Zero Slope Sump Lines") ;
    for( sptsP = sumpPtsP ; sptsP < sumpPtsP + sumpPtsNum ; ++sptsP )
      {
       sP1 = sptsP->sP1 ;
       sP2 = sptsP->sP2 ;
       bcdtmWrite_message(0,0,0,"sP1 = %6ld ** %10.4lf %10.4lf %10.4lf sP2 = %6ld ** %10.4lf %10.4lf %10.4lf",sP1,pointAddrP(dtmP,sP1)->x,pointAddrP(dtmP,sP1)->y,pointAddrP(dtmP,sP1)->z,sP2,pointAddrP(dtmP,sP2)->x,pointAddrP(dtmP,sP2)->y,pointAddrP(dtmP,sP2)->z) ;
      }
   }
/*
** Delete Duplicate Sump Lines
*/
 if( sumpPtsNum > 1 )
   {
/*
**  Order Points So Lowest Point Is First
*/
    for( sptsP = sumpPtsP ; sptsP < sumpPtsP + sumpPtsNum ; ++sptsP )
      {
       if( sptsP->sP1 > sptsP->sP2 )
         {
          sP1 = sptsP->sP1 ;
          sptsP->sP1 = sptsP->sP2 ;
          sptsP->sP2 = sP1 ;
         }
      }
/*
**  Quick Sort
*/
    qsort(sumpPtsP,sumpPtsNum,sizeof(struct SUMPPTS),bcdtmDrainage_sumpPointsCompareFunction) ;
/*
**  Eliminate Duplicates
*/
   for ( sptsP = spts1P = sumpPtsP ; spts1P < sumpPtsP + sumpPtsNum ; ++spts1P )
     {
      if( spts1P->sP1 != sptsP->sP1 || spts1P->sP2 != sptsP->sP2 )
        {
         ++sptsP ;
         if( sptsP != spts1P ) *sptsP = *spts1P ;
        }
     }
   sumpPtsNum = (long)(sptsP-sumpPtsP) + 1 ;
  }
/*
** Copy Sump Lines To Sump Lines Array
*/
 *numSumpLinesTableP = sumpPtsNum ;
 *sumpLinesPP   = ( DTM_SUMP_LINES * ) malloc( *numSumpLinesTableP * sizeof(DTM_SUMP_LINES)) ;
 if( *sumpLinesPP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 for( sumplinesP = *sumpLinesPP , sptsP = sumpPtsP ; sumplinesP < *sumpLinesPP + *numSumpLinesTableP ; ++sumplinesP , ++sptsP )
   {
    sumplinesP->sP1 = sptsP->sP1 ;
    sumplinesP->sP2 = sptsP->sP2 ;
   }
/*
** Free Memory
*/
 cleanup :
 if( sumpFlagP != nullptr ) free(sumpFlagP) ;
 if( sumpPtsP  != nullptr ) free(sumpPtsP)  ;
/*
** Normal Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Concatenating Zero Slope Sump Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Concatenating Zero Slope Sump Lines Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_checkForFlowOutFromSumpLinePointsDtmObject
(
 BC_DTM_OBJ        *dtmP,
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 DTM_SUMP_LINES    *sumpLinesP,
 long              numSumpLines,
 long              *exitPointP,
 long              *priorPointP,
 long              *nextPointP
 )
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    np,pp,loop,sumpPoint,exitPoint,hullPoint ;
 double  slope,exitSlope=0.0 ;
 DTM_SUMP_LINES *sLineP ;
/*
** Initialise
*/
 *exitPointP  = dtmP->nullPnt ;
 *nextPointP  = dtmP->nullPnt ;
 *priorPointP = dtmP->nullPnt ;
/*
** Check For Sump Line Point On A Tin Feature Hull
*/
 for( sLineP = sumpLinesP ; sLineP < sumpLinesP + numSumpLines && *exitPointP == dtmP->nullPnt ; ++sLineP )
   {
    if     ( nodeAddrP(dtmP,sLineP->sP1)->hPtr != dtmP->nullPnt ) *exitPointP = sLineP->sP1 ;
    else if( nodeAddrP(dtmP,sLineP->sP2)->hPtr != dtmP->nullPnt ) *exitPointP = sLineP->sP2 ;
    else
      {
       if( bcdtmList_testForHullPointDtmObject(dtmP,sLineP->sP2,&hullPoint)) goto errexit ;
       if( hullPoint ) *exitPointP = sLineP->sP2 ;
       else
         {
          if( bcdtmList_testForHullPointDtmObject(dtmP,sLineP->sP1,&hullPoint)) goto errexit ;
          if( hullPoint ) *exitPointP = sLineP->sP1 ;
         }
      }
   }
/*
** Scan Sump Points
*/
 if( *exitPointP == dtmP->nullPnt )
   {
    for( sLineP = sumpLinesP ; sLineP < sumpLinesP + numSumpLines ; ++sLineP )
      {
       for( loop = 0 ; loop <= 1 ; ++loop )
         {
          if( loop == 0 ) sumpPoint = sLineP->sP1 ;
          else            sumpPoint = sLineP->sP2 ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Looking For Flow Out From %6ld",sumpPoint) ;
/*
**        Check For Flow Out Down A Non Zero Slope Sump Line
*/
          if( bcdtmDrainage_scanSumpPointForSumpLineDtmObject(dtmP,drainageTablesP,sumpPoint,&exitPoint,&slope)) goto errexit ;
          if( exitPoint != dtmP->nullPnt )
            {
             if(( pp = bcdtmList_nextClkDtmObject(dtmP,sumpPoint,exitPoint)) < 0 ) goto errexit ;
             if(( np = bcdtmList_nextAntDtmObject(dtmP,sumpPoint,exitPoint))   < 0 ) goto errexit ;
             if( *exitPointP == dtmP->nullPnt || slope > exitSlope )
               {
                *exitPointP  = sumpPoint ;
                *priorPointP = pp ;
                *nextPointP  = np ;
                exitSlope    = slope ;
               }
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"Sump Point = %6ld Exit Point = %6ld slope = %10.6lf",sumpPoint,exitPoint,slope ) ;
                bcdtmWrite_message(0,0,0,"sP1 = %6ld ** %10.4lf %10.4lf %10.4lf",sumpPoint,pointAddrP(dtmP,sumpPoint)->x,pointAddrP(dtmP,sumpPoint)->y,pointAddrP(dtmP,sumpPoint)->z) ;
                bcdtmWrite_message(0,0,0,"sP2 = %6ld ** %10.4lf %10.4lf %10.4lf",exitPoint,pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,pointAddrP(dtmP,exitPoint)->z) ;
               }
            }
/*
**        Check For Flow Out Over A Triangle
*/
          if( bcdtmDrainage_scanSumpPointForMaximumDescentTriangleDtmObject(dtmP,drainageTablesP,sumpPoint,&np,&pp,&slope) ) goto errexit ;
          if( np != dtmP->nullPnt )
            {
             np = np ;
             pp = pp ;
             if( *exitPointP == dtmP->nullPnt || slope > exitSlope )
               {
                *exitPointP  = sumpPoint ;
                *priorPointP = pp ;
                *nextPointP  = np ;
                exitSlope    = slope ;
               }
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"Sump Point = %6ld np = %6ld pp = %6ld slope = %10.6lf",sumpPoint,np,pp,slope ) ;
                bcdtmWrite_message(0,0,0,"Sp = %6ld ** %10.4lf %10.4lf %10.4lf",sumpPoint,pointAddrP(dtmP,sumpPoint)->x,pointAddrP(dtmP,sumpPoint)->y,pointAddrP(dtmP,sumpPoint)->z) ;
                bcdtmWrite_message(0,0,0,"np = %6ld ** %10.4lf %10.4lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
                bcdtmWrite_message(0,0,0,"pp = %6ld ** %10.4lf %10.4lf %10.4lf",pp,pointAddrP(dtmP,pp)->x,pointAddrP(dtmP,pp)->y,pointAddrP(dtmP,pp)->z) ;
               }
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_placePolygonAroundSumpLinesDtmObject
(
 BC_DTM_OBJ     *dtmP,
 DTM_SUMP_LINES *sumpLinesP,
 long           numSumpLines,
 long           *startPointP
)
{
 int            ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long           p1,p2,p3,p4,clc;
 DPoint3d       sumpLinePts[2] ;
 BC_DTM_OBJ     *tempDtmP=nullptr ;
 DTM_SUMP_LINES *sLineP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Placing Polygon Around Sump Lines") ;
/*
** Initialise
*/
 *startPointP = dtmP->nullPnt ;
/*
** Create Dtm object
*/
 if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,1000,1000) ;
/*
** Write Sump Lines To Data Object
*/
 if( dbg )
   {
    for( sLineP = sumpLinesP ; sLineP < sumpLinesP + numSumpLines ; ++sLineP )
      {
       bcdtmWrite_message(0,0,0,"SumpLine[%6ld] ** point = %6ld point = %6ld",(long)(sLineP-sumpLinesP),sLineP->sP1,sLineP->sP2) ;
       bcdtmWrite_message(0,0,0,"**** point = %6ld hPtr = %9ld ** %12.5lf %12.5lf %10.4lf",sLineP->sP1,nodeAddrP(dtmP,sLineP->sP1)->hPtr,pointAddrP(dtmP,sLineP->sP1)->x,pointAddrP(dtmP,sLineP->sP1)->y,pointAddrP(dtmP,sLineP->sP1)->z) ;
       bcdtmWrite_message(0,0,0,"**** point = %6ld hPtr = %9ld ** %12.5lf %12.5lf %10.4lf",sLineP->sP2,nodeAddrP(dtmP,sLineP->sP2)->hPtr,pointAddrP(dtmP,sLineP->sP2)->x,pointAddrP(dtmP,sLineP->sP2)->y,pointAddrP(dtmP,sLineP->sP2)->z) ;
      }
   }
/*
** Scan Around Sump Line End Points And Write Surrounding Lines To Data Object
*/
 for( sLineP = sumpLinesP ; sLineP < sumpLinesP + numSumpLines ; ++sLineP )
   {
    clc = nodeAddrP(dtmP,sLineP->sP1)->cPtr;
    if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,sLineP->sP1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while ( clc != dtmP->nullPtr )
      {
       p2  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( bcdtmList_testLineDtmObject(dtmP,p1,p2) )
         {
          sumpLinePts[0].x = pointAddrP(dtmP,p1)->x ; sumpLinePts[0].y = pointAddrP(dtmP,p1)->y ; sumpLinePts[0].z = (double) p1 ;
          sumpLinePts[1].x = pointAddrP(dtmP,p2)->x ; sumpLinePts[1].y = pointAddrP(dtmP,p2)->y ; sumpLinePts[1].z = (double) p2 ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,sumpLinePts,2)) goto errexit ;
         }
       p1 = p2 ;
      }
    clc = nodeAddrP(dtmP,sLineP->sP2)->cPtr;
    if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,sLineP->sP2,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while ( clc != dtmP->nullPtr )
      {
       p2  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( bcdtmList_testLineDtmObject(dtmP,p1,p2) )
         {
          sumpLinePts[0].x = pointAddrP(dtmP,p1)->x ; sumpLinePts[0].y = pointAddrP(dtmP,p1)->y ; sumpLinePts[0].z = (double) p1 ;
          sumpLinePts[1].x = pointAddrP(dtmP,p2)->x ; sumpLinePts[1].y = pointAddrP(dtmP,p2)->y ; sumpLinePts[1].z = (double) p2 ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,sumpLinePts,2)) goto errexit ;
         }
       p1 = p2 ;
      }
   }
/*
**  Triangulate DTM Object
*/
 tempDtmP->ppTol = tempDtmP->plTol = 0.0 ;
 if( bcdtmObject_createTinDtmObjectOverload (tempDtmP,1,0.0,false,false)) goto errexit ;
/*
** Remove None Feature Hull Lines
*/
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(tempDtmP,L"sumpLinesPolygon.bcdtm") ;
/*
** Copy Tin Hull To Tptr List
*/
 p1 = tempDtmP->hullPoint ;
 p3 = (long) bcdtmMath_roundToDecimalPoints(pointAddrP(tempDtmP,p1)->z,1) ;
 do
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"p1 = %8ld p3 = %8ld",p1,p3) ;
    p2 = nodeAddrP(tempDtmP,p1)->hPtr ;
    p4 = (long) bcdtmMath_roundToDecimalPoints(pointAddrP(tempDtmP,p2)->z,1) ;
    nodeAddrP(dtmP,p3)->tPtr = p4  ;
    if( *startPointP == dtmP->nullPnt ) *startPointP = p3 ;
    p1 = p2 ;
    p3 = p4 ;
   } while ( p1 != tempDtmP->hullPoint ) ;
/*
** Check Connectivity Tptr List
*/
 if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,*startPointP,1)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( tempDtmP != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
/*
** Normal Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Placing Polygon Around Sump Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Placing Polygon Around Sump Lines Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_markZeroSlopeSumpLinesWithinTptrPolygonDtmObject
(
 BC_DTM_OBJ     *dtmP,
 long           startPoint,
 DTM_SUMP_LINES **sumpLinesPP,
 long           *numSumpLinesTableP
 )
/*
** This Function Marks All Zero Slope Lines Within A Tptr Polygon
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long sp,np,pp,cp,lp,hp,mark=-98989898,clPtr,numMarked ;
 long memSumpLinesTable,memSumpLinesTableInc=1000 ;
 DTM_SUMP_LINES *sumpLinesP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Zero Slope Sump Lines") ;
/*
** Initialise
*/
 memSumpLinesTable = *numSumpLinesTableP ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Start Number Of Marked Zero Slope Sump Lines = %6ld",*numSumpLinesTableP) ;
/*
** Mark All Points Internal To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Tptr Polygon") ;
 if( bcdtmMark_internalTptrPolygonPointsDtmObject(dtmP,startPoint,mark,&numMarked) ) goto errexit ;
/*
** Get Lowest And Highest Numbered Points On Tptr Polygon
*/
 lp = hp = sp = startPoint ;
 do
   {
    sp = nodeAddrP(dtmP,sp)->tPtr ;
    if( sp < lp ) lp = sp ;
    if( sp > hp ) hp = sp ;
   } while ( sp != startPoint ) ;
/*
** Mark And Store Zero Slope Lines In Sump Line Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Zero Slope Sump Lines") ;
 for( sp = lp ; sp != hp  ; ++sp )
   {
    if( nodeAddrP(dtmP,sp)->tPtr == mark )
      {
       clPtr = nodeAddrP(dtmP,sp)->cPtr;
       while( clPtr != dtmP->nullPtr )
          {
          np    = clistAddrP(dtmP,clPtr)->pntNum ;
            clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          if( np > sp && nodeAddrP(dtmP,np)->tPtr == mark )
            {
             if( pointAddrP(dtmP,sp)->z == pointAddrP(dtmP,np)->z )
               {
                if( *numSumpLinesTableP == memSumpLinesTable )
                  {
                   memSumpLinesTable = memSumpLinesTable + memSumpLinesTableInc ;
                   if( *sumpLinesPP == nullptr ) *sumpLinesPP = ( DTM_SUMP_LINES * ) malloc ( memSumpLinesTable * sizeof(DTM_SUMP_LINES)) ;
                   else                       *sumpLinesPP = ( DTM_SUMP_LINES * ) realloc ( *sumpLinesPP,memSumpLinesTable * sizeof(DTM_SUMP_LINES)) ;
                   if( *sumpLinesPP == nullptr )
                     {
                      bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                      goto errexit ;
                     }
                  }
                (*sumpLinesPP+*numSumpLinesTableP)->sP1 = sp ;
                (*sumpLinesPP+*numSumpLinesTableP)->sP2 = np ;
                ++*numSumpLinesTableP ;
               }
            }
         }
      }
   }
/*
** Mark Lines On Internal Edge Of Tptr Polygon Not Connected To A Marked Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Edge Zero Slope Sump Lines") ;
 pp = startPoint ;
 sp = nodeAddrP(dtmP,pp)->tPtr ;
 np = nodeAddrP(dtmP,sp)->tPtr ;
 do
   {
    cp = np ;
    if(( cp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
    while ( cp != pp )
      {
       if( nodeAddrP(dtmP,cp)->tPtr != mark )
         {
          if( pointAddrP(dtmP,sp)->z == pointAddrP(dtmP,cp)->z )
            {
             if( *numSumpLinesTableP == memSumpLinesTable )
               {
                memSumpLinesTable = memSumpLinesTable + memSumpLinesTableInc ;
                if( *sumpLinesPP == nullptr ) *sumpLinesPP = ( DTM_SUMP_LINES * )malloc ( memSumpLinesTable * sizeof(DTM_SUMP_LINES)) ;
                else                       *sumpLinesPP = ( DTM_SUMP_LINES * )realloc ( *sumpLinesPP,memSumpLinesTable * sizeof(DTM_SUMP_LINES)) ;
                if( *sumpLinesPP == nullptr )
                  {
                   bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                   goto errexit ;
                  }
               }
             (*sumpLinesPP+*numSumpLinesTableP)->sP1 = sp ;
             (*sumpLinesPP+*numSumpLinesTableP)->sP2 = cp ;
             ++*numSumpLinesTableP ;
            }
         }
        if(( cp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
       }
    pp = sp ;
    sp = np ;
    np = nodeAddrP(dtmP,sp)->tPtr  ;
   } while ( pp != startPoint ) ;
/*
** Un Mark Marked Points
*/
 for( sp = lp ; sp <= hp ; ++sp )
   {
    if( nodeAddrP(dtmP,sp)->tPtr == mark ) nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
   }
/*
** Write Marked Zero Slope Sump Lines
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Marked Zero Slope Sump Lines = %6ld",*numSumpLinesTableP) ;
    for( sumpLinesP = *sumpLinesPP ; sumpLinesP < *sumpLinesPP + *numSumpLinesTableP ; ++sumpLinesP )
      {
       bcdtmWrite_message(0,0,0,"sumpLine[%6ld] = %8ld  %8ld",(long)(sumpLinesP-*sumpLinesPP),sumpLinesP->sP1,sumpLinesP->sP2) ;
      }
   }
/*
** Cleanup ;
*/
 cleanup :
/*
** Normal Exit
*/
 if( dbg && ret == 0 ) bcdtmWrite_message(0,0,0,"Marking Zero Slope Sump Lines Completed") ;
 if( dbg && ret != 0 ) bcdtmWrite_message(0,0,0,"Marking Zero Slope Sump Lines Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_scanSumpPointForSumpLineDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Point To Scan About For Sump Lines
 long              *sumpPointP,                // <== If Sump Line Found Point On Other End Of Sump Line
 double            *slopeP                     // <== Slope Of maximum Descent Sump Line
 )
/*
** This Function Scans A Point For A Sump Line
** Zero Slope Sump Lines Are Not Considered
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   pp,sp,np,clPtr ;
 double dx,dy,dz,dd,slope ;
 int    flowLeft,flowRght ;
 bool   leftVoid,rghtVoid ;

// Log Function Arguments
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scanning Sump Point For Sump Line") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"point           = %8ld",point) ;
   }

// Initialise

 *sumpPointP = dtmP->nullPnt ;
 if( point < 0 || point >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Point Range Error") ;
    goto errexit ;
   }
 if( ( clPtr = nodeAddrP(dtmP,point)->cPtr) == dtmP->nullPtr )
   {
    bcdtmWrite_message(2,0,0,"Point Has No Circular List") ;
    goto errexit ;
   }

// Log Sump Point And Connecting Points

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"SumpPoint[%8ld] = %12.5lf %12.5lf %10.4lf",point,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z) ;
    bcdtmList_writeCircularListForPointDtmObject(dtmP,point) ;
   }

// Scan Around Point And Look For Sump Line

 sp = clistAddrP(dtmP,clPtr)->pntNum ;
 if(( pp = bcdtmList_nextAntDtmObject(dtmP,point,sp)) < 0 ) goto errexit ;
 while ( clPtr != dtmP->nullPtr )
   {
    sp  = clistAddrP(dtmP,clPtr)->pntNum ;
    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
    if(( np = bcdtmList_nextClkDtmObject(dtmP,point,sp)) < 0 ) goto errexit ;
    if(nodeAddrP(dtmP,point)->hPtr != pp && nodeAddrP(dtmP,point)->hPtr != sp && nodeAddrP(dtmP,sp)->hPtr != point && nodeAddrP(dtmP,np)->hPtr != point )
      {
       if( pointAddrP(dtmP,point)->z > pointAddrP(dtmP,sp)->z )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Checking Flow Direction pp = %8ld sp = %8ld np = %8ld",pp,sp,np) ;

//        Calculate Flow Direction For Adjacent Triangles

          if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,sp,point,np,leftVoid,flowLeft)) goto errexit ;
          if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,point,sp,pp,rghtVoid,flowRght)) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"**** leftVoid = %2ld rightVoid = %2ld flowLeft = %2ld flowRght = %2ld",leftVoid,rghtVoid,flowLeft,flowRght) ;
          if( ! leftVoid && ! rghtVoid && flowLeft >= 0 && flowRght >= 0 )
            {
             dx = pointAddrP(dtmP,point)->x - pointAddrP(dtmP,sp)->x ;
             dy = pointAddrP(dtmP,point)->y - pointAddrP(dtmP,sp)->y ;
             dz = pointAddrP(dtmP,point)->z - pointAddrP(dtmP,sp)->z ;
             dd = sqrt(dx*dx + dy*dy) ;
             slope = dz / dd ;
             if ( *sumpPointP == dtmP->nullPnt || slope  > *slopeP )
               {
                *sumpPointP = sp ;
                *slopeP = slope ;
               }
            }
         }
      }
    pp = sp ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_scanSumpPointForMaximumDescentTriangleDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Point To Scan About
 long              *trgPnt1P,                  // <== Clockwise Point Of Maximum Descent Triangle Base
 long              *trgPnt2P,                  // <== Anti Clockwise Point Of Maximum Descent Triangle Base
 double            *trgSlopeP                  // <== Triangle Slope
)
/*
** This Function Scans The A Point For The Maximum Descent Triangle
** Zero Slope Triangles Are Not Considered
*/
{
 int    ret=DTM_SUCCESS ;
 long   clPtr,p1,p2 ;
 double a1,a2,a3,angp1,angp2,slope,descentAngle,ascentAngle  ;
 long useTables=0 ;
 bool   voidTriangle ;
/*
** Initialise
*/
 *trgPnt1P = dtmP->nullPnt ;
 *trgPnt2P = dtmP->nullPnt ;
 *trgSlopeP = 0.0 ;
/*
** Scan Around Point
*/
 clPtr = nodeAddrP(dtmP,point)->cPtr;
 if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
 angp1 = bcdtmMath_getPointAngleDtmObject(dtmP,point,p1) ;
 while ( clPtr != dtmP->nullPtr )
   {
    p2  = clistAddrP(dtmP,clPtr)->pntNum ;
    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
    angp2 = bcdtmMath_getPointAngleDtmObject(dtmP,point,p2) ;
    if(  nodeAddrP(dtmP,point)->hPtr != p1 )
      {
       if( bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP,drainageTablesP,point,p1,p2,voidTriangle,slope,descentAngle,ascentAngle)) goto errexit ;
       if( ! voidTriangle && slope > 0.0 )
         {
          a1 = angp1 ;
          a2 = descentAngle ;
          a3 = angp2 ;
          if( a1 < a3 ) a1 = a1 + DTM_2PYE ;
          if( a2 < a3 ) a2 = a2 + DTM_2PYE ;
          if( a2 <= a1 && a2 >= a3 )
            {
             if( *trgPnt1P == dtmP->nullPnt || slope > *trgSlopeP  )
               {
                *trgPnt1P = p1 ;
                *trgPnt2P = p2 ;
                *trgSlopeP = slope ;
               }
            }
         }
      }
    p1 = p2 ;
    angp1 = angp2 ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_determineZeroSlopeTrianglePondsDtmObject
(
 BC_DTM_OBJ                *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables         *drainageTablesP,           // ==> Pointer To Drainage Tables
 DTMZeroSlopePolygonVector *zeroSlopePolygonsP,        // ==> Pointer To Zero Slope Polygons
 int                       *zeroSlopePointsIndexP,     // ==> Index To Zero Slope Polygons
 DTMFeatureCallback        loadFunctionP,              // ==> Pointer To Call Back Function
 bool                      loadFlag,                   // ==> Pass Pond Boundaries to User Call Back Function
 bool                      buildTable,                 // ==> Create Pond Tables For Later Use
 void                      *userP,                     // ==> User Pointer Passed Back To Call Back Function
 int&                      numZeroSlopeTrianglePonds   // <== Number Of Zero Slope Triangle Ponds
)
{

// Determine Zero Slope Triangle Ponds

 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 int     zeroSlopePolygon,numNoExitPoint ;
 long    spnt,npnt,exitPoint,priorPoint,nextPoint ;
 long    startPnt,expandStart,maxFeature,numZeroSlopePlygons ;
 double  area,elevation,maxTime=-1.0 ;
 bool    isInternal=false ;
 DTMDirection direction ;
 DTM_POLYGON_OBJ *polygonP=nullptr ;
 DTMZeroSlopePolygonVector::iterator zsp,zspExternal ;

// Log Function Parameters

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determing Zero Slope Triangle Ponds") ;
    bcdtmWrite_message(0,0,0,"dtmP                  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP       = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePolygonsP    = %p",zeroSlopePolygonsP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePointsIndexP = %p",zeroSlopePointsIndexP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP         = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"loadFlag              = %8ld",loadFlag) ;
    bcdtmWrite_message(0,0,0,"buildTable            = %8ld",buildTable) ;
    bcdtmWrite_message(0,0,0,"userP                 = %p",userP) ;
   }

// Initialise

 numZeroSlopeTrianglePonds = 0 ;

// Test For Valid Tin Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

// Check For Tin State

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }

// Check For None Null Tptr And Sptr Values

 if( cdbg == 2 )
   {
    int err=0 ;
    for( spnt = 0 ; spnt < dtmP->numPoints && err <= 10 ; ++spnt )
      {
       if( nodeAddrP(dtmP,spnt)->tPtr != dtmP->nullPnt || nodeAddrP(dtmP,spnt)->sPtr != dtmP->nullPnt )
         {
          bcdtmWrite_message(1,0,0,"00 ** spnt = %8ld spnt->tPtr = %10ld spnt->sPtr = %10ld",spnt,nodeAddrP(dtmP,spnt)->tPtr,nodeAddrP(dtmP,spnt)->sPtr ) ;
          ++err ;
         }
      }
    if( err ) goto errexit ;
   }

// Set Number Of Zero Slope Polygons

 numZeroSlopePlygons =  ( long ) ( zeroSlopePolygonsP->end() - zeroSlopePolygonsP->begin() );
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Zero Slope Polygons = %8ld",numZeroSlopePlygons) ;

// Scan Zero Slope Polygons And Determine Exit Points

 for( zsp = zeroSlopePolygonsP->begin() , zeroSlopePolygon = 0  ; zsp < zeroSlopePolygonsP->end() ; ++zsp , ++zeroSlopePolygon )
   {

    if( dbg == 1 && zeroSlopePolygon % 10000 == 0 ) bcdtmWrite_message(0,0,0,"Expanding Zero Slope Polygon  = %8d of %8ld",zeroSlopePolygon,numZeroSlopePlygons ) ;
//    if( dbg == 1  ) bcdtmWrite_message(0,0,0,"Expanding Zero Slope Polygon = %8d of %8ld",zeroSlopePolygon,numZeroSlopePlygons ) ;

//  Only Expand Counter Clockwise Zero Slope Polygons

    if(  zsp->direction == DTMDirection::AntiClockwise )
      {

//     Copy Zero Slope Polygon To Tptr List

       if( bcdtmDrainageList_copyPointListToTptrListDtmObject(dtmP,zsp->pointList,&startPnt)) goto errexit ;
       if( dbg == 2 )
         {
          bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ;
          bcdtmWrite_message(0,0,0,"Area Of Zero Slope Polygon ** direction = %2ld ** area = %15.5lf",direction,area) ;
         }

//     Check Zero Slope Polygon Is Not Internal To Another Zero Slope Polygon

       isInternal = false ;
       spnt = startPnt ;
       do
         {
          if( *(zeroSlopePointsIndexP+spnt) != dtmP->nullPnt )
            {
             isInternal = true ;
             zspExternal = zeroSlopePolygonsP->begin() + *(zeroSlopePointsIndexP+spnt) ;
             zsp->priorPoint = zspExternal->priorPoint ;
             zsp->exitPoint  = zspExternal->exitPoint  ;
             zsp->nextPoint  = zspExternal->nextPoint  ;
            }
         } while( spnt != startPnt && isInternal == false ) ;

//     Expand Zero Slope Polygon To Exit Point

       if( isInternal == false )
         {

//        Set Zero Slope Elevation

          elevation = pointAddrP(dtmP,startPnt)->z ;
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"startPnt = %8ld %12.5lf %12.5lf %10.4lf",startPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,startPnt)->z) ;

//        Expand To Exit Point

          expandStart = bcdtmClock() ;
          if( dbg == 2 )
            {
             bcdtmWrite_message(0,0,0,"zeroSlopePond = %8d of %8ld ** Elevation = %10.4lf",zeroSlopePolygon,numZeroSlopePlygons,pointAddrP(dtmP,startPnt)->z ) ;
             bcdtmWrite_message(0,0,0,"startPnt = %8ld ** %12.5lf %12.5lf %10.4lf",startPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,startPnt)->z) ;
            }
          if( bcdtmDrainage_expandPondToExitPointDtmObject(dtmP,drainageTablesP,nullptr,nullptr,startPnt,&exitPoint,&priorPoint,&nextPoint)) goto errexit ;
          if( exitPoint == dtmP->nullPnt )
            {
             bcdtmWrite_message(2,0,0,"Pond Exit Point About About Zero Slope Polygon Not Determined") ;
             goto errexit ;
            }

//        Log Timing Stats

          if( tdbg && bcdtmClock_elapsedTime(bcdtmClock(),expandStart) > 0.05 )
            {
             bcdtmWrite_message(0,0,0,"Expansion Time For Zero Slope Pond %8ld = %8.3lf seconds",zeroSlopePolygon,bcdtmClock_elapsedTime(bcdtmClock(),expandStart)) ;
            }
          if( tdbg && bcdtmClock_elapsedTime(bcdtmClock(),expandStart) > maxTime )
            {
             maxTime = bcdtmClock_elapsedTime(bcdtmClock(),expandStart) ;
             maxFeature = (long)(zsp-zeroSlopePolygonsP->begin()) ;
             bcdtmWrite_message(0,0,0,"maxFeature = %8ld ** maxTime = %8.3lf Seconds",maxFeature,maxTime) ;
            }

//        Log Expanded Area

          if( dbg == 2 )
            {
             bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,exitPoint,&area,&direction) ;
             bcdtmWrite_message(0,0,0,"Expanded Area Of Zero Slope Polygon ** direction = %2ld ** area = %15.5lf",direction,area) ;
            }

//        Update Pond Exit Points

          zsp->priorPoint = priorPoint ;
          zsp->exitPoint  = exitPoint ;
          zsp->nextPoint  = nextPoint ;

//        Draw Pond Boundary At Exit Point Elevation

          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Drawing Pond Boundary") ;
          if( loadFlag && pointAddrP(dtmP,exitPoint)->z > elevation )
            {
             if( bcdtmDrainage_extractPondBoundaryDtmObject(dtmP,pointAddrP(dtmP,exitPoint)->z,exitPoint,nextPoint,loadFunctionP,loadFlag,false,&polygonP, userP)) goto errexit ;
            }
          if( polygonP != nullptr ) bcdtmPolygon_deletePolygonObject(&polygonP) ;

//        Null Tptr Polygon

          if( bcdtmList_nullTptrListDtmObject(dtmP,exitPoint)) goto errexit ;

//        Index All Points At The Zero Slope Elevation To The Zero Slope Polygon

          if( bcdtmDrainage_indexAllPointsAtZeroSlopeElevationDtmObject(dtmP,zeroSlopePolygonsP,zeroSlopePointsIndexP,zeroSlopePolygon,priorPoint,exitPoint,nextPoint)) goto errexit ;

//        Check For None Null Tptr And Sptr Values

          if( cdbg == 2 )
            {
             for( spnt = 0 ; spnt < dtmP->numPoints ; ++spnt )
               {
                if( nodeAddrP(dtmP,spnt)->tPtr != dtmP->nullPnt || nodeAddrP(dtmP,spnt)->sPtr != dtmP->nullPnt )
                  {
                   bcdtmWrite_message(1,0,0,"zeroSlopePolygon = %8d ** spnt = %8ld spnt->tPtr = %10ld spnt->sPtr = %10ld",zeroSlopePolygon,spnt,nodeAddrP(dtmP,spnt)->tPtr,nodeAddrP(dtmP,spnt)->sPtr ) ;
                   goto errexit ;
                  }
               }
            }
         }

//     Null Tptr List

       else
         {
          if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
         }
      }

//  Increment Number Of Zero Slope Triangle Ponds

    ++numZeroSlopeTrianglePonds ;
   }

// Log Number OF Zero Slope Ponds

 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numZeroSlopeTrianglePonds = %8ld",numZeroSlopeTrianglePonds) ;

// Populate The Zero Slope Triangle Drainage Table

 if( buildTable )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Populating Zero Slope Triangle Pond Table") ;
    expandStart = bcdtmClock() ;
    for( zsp = zeroSlopePolygonsP->begin() ; zsp < zeroSlopePolygonsP->end() ; ++zsp )
      {
       if( bcdtmDrainageList_copyPointListToTptrListDtmObject(dtmP,zsp->pointList,&startPnt)) goto errexit ;

//     Get Exit Point For Internal Zero Slope Polygons

       if( zsp->direction == DTMDirection::Clockwise )
         {
          isInternal = false ;
          spnt = startPnt ;
          do
            {
             if( *(zeroSlopePointsIndexP+spnt) != dtmP->nullPnt )
               {
                isInternal = true ;
                zspExternal = zeroSlopePolygonsP->begin() + *(zeroSlopePointsIndexP+spnt) ;
                zsp->priorPoint = zspExternal->priorPoint ;
                zsp->exitPoint  = zspExternal->exitPoint  ;
                zsp->nextPoint  = zspExternal->nextPoint  ;
               }
            } while( spnt != startPnt && isInternal == false ) ;
         }

//     Populate Table

       spnt = startPnt ;
       do
         {
          npnt = nodeAddrP(dtmP,spnt)->tPtr ;
          drainageTablesP->StoreZeroSlopeLinePond(spnt,npnt,zsp->exitPoint,zsp->priorPoint,zsp->nextPoint) ;
          spnt = npnt ;
         } while ( spnt != startPnt ) ;

//     Null Tptr List

       if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
      }

//  Log Time To Populate Table

    if( tdbg == 1 ) bcdtmWrite_message(0,0,0,"Time To Populate Zero Slope Triangle Table = %8.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),expandStart)) ;

//  Count Number Of Zero Slope Polygons Without An Exit Point

    if( cdbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Counting Number Of Zero Slope Polygons Without An Exit Point") ;
       numNoExitPoint = 0 ;
       for( zsp = zeroSlopePolygonsP->begin() , zeroSlopePolygon = 0  ; zsp < zeroSlopePolygonsP->end() ; ++zsp , ++zeroSlopePolygon )
         {
          if( zsp->exitPoint < 0 )
            {
             ++numNoExitPoint ;
            }
         }
       bcdtmWrite_message(0,0,0,"Number Of Zero Slope Polygons Without An Exit Point = %8ld",numNoExitPoint) ;
      }

   }

/*
** Cleanup ;
*/
 cleanup :
/*
** Write Stats
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Zero Slope Triangle Ponds = %6ld",numZeroSlopeTrianglePonds) ;
 if( polygonP   != nullptr ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
/*
** Normal Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determing Zero Slope Triangle Ponds Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determing Zero Slope Triangle Ponds Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_placePolygonAroundZeroSlopePolygonDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 long              *firstPointP                // <== First Point On Prior And Expanded Polygon
)
/*
** This Function Places A Polgon Around A Zero Slope Polygon
*/
{
 int           ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long          spnt,npnt,ppnt,cptr ;
 DPoint3d      breakPts[2] ;
 BC_DTM_OBJ    *tempDtmP=nullptr ;
 DTM_TIN_POINT *pntP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Placing Polygon Around A Zero Slope Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"firstPointP     = %8ld",firstPointP) ;
   }
/*
** Check Tptr Polygon Connectivity
*/
 if( cdbg )
   {
    if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,*firstPointP,0))
      {
       bcdtmWrite_message(1,0,0,"Connectivity Error In Tptr Polygon") ;
       goto errexit ;
      }
   }
/*
** Create Temporary DTM To Store Features
*/
 if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,10000,10000) ;
/*
** Store Tptr Polygon In DTM As Break Lines
*/
 spnt = *firstPointP ;
 do
   {
    pntP = pointAddrP(dtmP,spnt) ;
    breakPts[0].x = pntP->x ; breakPts[0].y = pntP->y ; breakPts[0].z = ( double ) spnt ;
    npnt = nodeAddrP(dtmP,spnt)->tPtr ;
    pntP = pointAddrP(dtmP,npnt) ;
    breakPts[1].x = pntP->x ; breakPts[1].y = pntP->y ; breakPts[1].z = ( double ) npnt ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,breakPts,2)) goto errexit ;
    spnt = npnt ;
   } while( spnt != *firstPointP ) ;
/*
** Store Triangle Edges In DTM As Break Lines
*/
 spnt = *firstPointP ;
 do
   {
    if( ( cptr = nodeAddrP(dtmP,spnt)->cPtr ) != dtmP->nullPtr )
      {
       if( ( ppnt = bcdtmList_nextAntDtmObject(dtmP,spnt,clistAddrP(dtmP,cptr)->pntNum)) < 0 ) goto errexit ;
       while( cptr != dtmP->nullPtr )
         {
          npnt = clistAddrP(dtmP,cptr)->pntNum ;
          cptr = clistAddrP(dtmP,cptr)->nextPtr ;
          if( bcdtmList_testLineDtmObject(dtmP,ppnt,npnt))
            {
             pntP = pointAddrP(dtmP,ppnt) ;
             breakPts[0].x = pntP->x ; breakPts[0].y = pntP->y ; breakPts[0].z = ( double ) ppnt ;
             pntP = pointAddrP(dtmP,npnt) ;
             breakPts[1].x = pntP->x ; breakPts[1].y = pntP->y ; breakPts[1].z = ( double ) npnt ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,breakPts,2)) goto errexit ;
            }
          ppnt = npnt ;
         }
      }
    spnt = nodeAddrP(dtmP,spnt)->tPtr ;
   } while( spnt != *firstPointP ) ;
/*
** Triangulate Dtm Object
*/
 tempDtmP->ppTol = 0.0 ;
 tempDtmP->plTol = 0.0 ;
 if( bcdtmObject_createTinDtmObjectOverload (tempDtmP,1,0.0,false,false) ) goto errexit ;
/*
** Remove None Feature Hull Lines
*/
if( bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtmP)) goto errexit ;
/*
** Check Hull Points Are Coincident With Tin Points
*/
 if( cdbg )
   {
    spnt = tempDtmP->hullPoint ;
    do
      {
       ppnt = ( long) pointAddrP(tempDtmP,spnt)->z ;
       npnt = ( long) pointAddrP(tempDtmP,nodeAddrP(tempDtmP,spnt)->hPtr)->z ;
       if( ! bcdtmList_testLineDtmObject(dtmP,ppnt,npnt))
         {
          bcdtmWrite_message(1,0,0,"Unconnected Hull Points") ;
          goto errexit ;
         }
       spnt = nodeAddrP(tempDtmP,spnt)->hPtr ;
      } while( spnt != tempDtmP->hullPoint ) ;
   }
/*
** Null Existing Tptr Polygon
*/
 if( bcdtmList_nullTptrListDtmObject(dtmP,*firstPointP)) goto errexit ;
/*
** Insert New Tptr Polygon
*/
 *firstPointP = ( long ) pointAddrP(tempDtmP,tempDtmP->hullPoint)->z ;
 spnt = tempDtmP->hullPoint ;
 do
   {
    ppnt = ( long) pointAddrP(tempDtmP,spnt)->z ;
    npnt = ( long) pointAddrP(tempDtmP,nodeAddrP(tempDtmP,spnt)->hPtr)->z ;
    nodeAddrP(dtmP,ppnt)->tPtr = npnt ;
    spnt = nodeAddrP(tempDtmP,spnt)->hPtr ;
   } while( spnt != tempDtmP->hullPoint ) ;
/*
** Check Tptr Polygon Connectivity
*/
 if( cdbg )
   {
    if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,*firstPointP,0))
      {
       bcdtmWrite_message(1,0,0,"Connectivity Error In Tptr Polygon") ;
       goto errexit ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( tempDtmP != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
/*
** Non Error Exit
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Determing Pond About Low Point Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Determing Pond About Low Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_placeTptrPolygonAroundTouchingZeroSlopePolygonDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 long              zeroSlopeFeature,           // ==> Zero Slope Feature
 long              *firstPointP                // <== First Point On Tptr Polygon
)
/*
** This Function Places All Touching Zero Slope Polygon
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    fPnt,fPtr,spnt,ppnt,npnt,startPnt,dtmFeature,process,numTouched ;
 long    *tfP,*touchingFeaturesP=nullptr,numTouchingFeatures=0,memTouchingFeatures=0,memTouchingFeaturesInc=100 ;
 double  area ;
 DTMDirection   direction ;
 DPoint3d       breakPts[2] ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ     *tempDtmP=nullptr ;
 DTM_TIN_POINT  *pntP ;
/*
** Log Entry Parameters
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Placing Tptr Polygon Around Touching Zero Slope Polygons") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopeFeature = %8ld",zeroSlopeFeature) ;
   }
/*
** Initialise
*/
 *firstPointP = dtmP->nullPnt ;

// Copy Zero Feature Points To Tptr Polygon

 if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,zeroSlopeFeature,&startPnt)) goto errexit ;

// Log Initial Area

 if( dbg )
   {
    bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ;
    bcdtmWrite_message(0,0,0,"Initial Tptr Polygon Area = %20.5lf Direction = %2ld",area,direction) ;
   }

// Allocate Initial Memory

 memTouchingFeatures = memTouchingFeatures + memTouchingFeaturesInc ;
 touchingFeaturesP = ( long * ) malloc(memTouchingFeatures*sizeof(long)) ;
 if( touchingFeaturesP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }

// Add Feature To List

 *(touchingFeaturesP+numTouchingFeatures) = zeroSlopeFeature ;
 ++numTouchingFeatures ;

// Scan Tptr List To Get Touching Features

 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Touching Zero Slope Polygons") ;
 fPnt = startPnt ;
 do
   {
    fPtr = nodeAddrP(dtmP,fPnt)->fPtr ;
    while( fPtr != dtmP->nullPtr )
      {
       dtmFeature = flistAddrP(dtmP,fPtr)->dtmFeature ;
       fPtr = flistAddrP(dtmP,fPtr)->nextPtr ;
       if( dtmFeature != zeroSlopeFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::ZeroSlopePolygon && dtmFeatureP->internalToDtmFeature == 0 )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Touching Zero Slope Polygon = %8ld",dtmFeature) ;

//           Check Memory

             if( numTouchingFeatures == memTouchingFeatures )
               {
                memTouchingFeatures = memTouchingFeatures + memTouchingFeaturesInc ;
                if( touchingFeaturesP == nullptr ) touchingFeaturesP = ( long * ) malloc(memTouchingFeatures*sizeof(long)) ;
                else                            touchingFeaturesP = ( long * ) realloc(touchingFeaturesP,memTouchingFeatures*sizeof(long)) ;
                if( touchingFeaturesP == nullptr )
                  {
                   bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                   goto errexit ;
                  }

//              Add Initial Zero Slope Feature To List

                if( numTouchingFeatures == 0 )
                  {
                   *(touchingFeaturesP+numTouchingFeatures) = zeroSlopeFeature ;
                   ++numTouchingFeatures ;
                  }
               }

//           Add Touching Feature To List

             *(touchingFeaturesP+numTouchingFeatures) = dtmFeature ;
             ++numTouchingFeatures ;

//           Mark Touching Feature As Being Used

             dtmFeatureP->internalToDtmFeature = 2 ;

            }
         }
      }
    fPnt = nodeAddrP(dtmP,fPnt)->tPtr ;
   } while ( fPnt != startPnt ) ;

// Log Number Of Touching Features

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Touching Zero Slope Polygons = %8ld",numTouchingFeatures) ;
    for( tfP = touchingFeaturesP ; tfP < touchingFeaturesP + numTouchingFeatures ; ++tfP )
      {
       bcdtmList_nullTptrListDtmObject(dtmP,startPnt) ;
       bcdtmWrite_message(0,0,0,"TouchingFeature[%8ld] = %8ld",(long)(tfP-touchingFeaturesP),*tfP) ;
      }
   }

// Get Remaing Touching Features

 if( numTouchingFeatures > 1 )
   {
    process = TRUE ;
    bcdtmList_nullTptrListDtmObject(dtmP,startPnt) ;
    while( process )
      {
       process = FALSE ;
       numTouched = numTouchingFeatures ;
       for(tfP = touchingFeaturesP ; tfP < touchingFeaturesP + numTouched ; ++tfP )
         {
          if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,*tfP,&startPnt)) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Getting Touching Zero Slope Polygons") ;
          fPnt = startPnt ;
          do
            {
             fPtr = nodeAddrP(dtmP,fPnt)->fPtr ;
             while( fPtr != dtmP->nullPtr )
               {
                dtmFeature = flistAddrP(dtmP,fPtr)->dtmFeature ;
                fPtr = flistAddrP(dtmP,fPtr)->nextPtr ;
                if( dtmFeature != zeroSlopeFeature )
                  {
                   dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                   if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::ZeroSlopePolygon && dtmFeatureP->internalToDtmFeature == 0 )
                     {

                      if( dbg ) bcdtmWrite_message(0,0,0,"Touching Zero Slope Polygon = %8ld",dtmFeature) ;
                      process = 1 ;

//                    Check Memory

                      if( numTouchingFeatures == memTouchingFeatures )
                        {
                         memTouchingFeatures = memTouchingFeatures + memTouchingFeaturesInc ;
                         if( touchingFeaturesP == nullptr ) touchingFeaturesP = ( long * ) malloc(memTouchingFeatures*sizeof(long)) ;
                         else                            touchingFeaturesP = ( long * ) realloc(touchingFeaturesP,memTouchingFeatures*sizeof(long)) ;
                         if( touchingFeaturesP == nullptr )
                           {
                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                            goto errexit ;
                           }
                        }

//                    Add Touching Feature To List

                      *(touchingFeaturesP+numTouchingFeatures) = dtmFeature ;
                      ++numTouchingFeatures ;

//                    Mark Touching Feature As Being Used

                      dtmFeatureP->internalToDtmFeature = 2 ;
                     }
                  }
               }
             fPnt = nodeAddrP(dtmP,fPnt)->tPtr ;
            } while ( fPnt != startPnt ) ;

//        Null Tptr Polygon

          bcdtmList_nullTptrListDtmObject(dtmP,startPnt) ;
         }
      }

//  Create Temporary DTM Object

    if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,10000,10000)) goto errexit ;

//  Get Tptr Polygon Surrounding All Touching Zero Slope Polygons

    for( tfP = touchingFeaturesP ; tfP < touchingFeaturesP + numTouchingFeatures ; ++tfP )
      {
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,*tfP,&startPnt)) goto errexit ;
       if( bcdtmDrainage_placePolygonAroundZeroSlopePolygonDtmObject(dtmP,&startPnt)) goto errexit ;
       spnt = startPnt ;
       do
         {
          npnt = nodeAddrP(dtmP,spnt)->tPtr ;
          pntP = pointAddrP(dtmP,spnt) ;
          breakPts[0].x = pntP->x ; breakPts[0].y = pntP->y ; breakPts[0].z = ( double ) spnt ;
          pntP = pointAddrP(dtmP,npnt) ;
          breakPts[1].x = pntP->x ; breakPts[1].y = pntP->y ; breakPts[1].z = ( double ) npnt ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,breakPts,2)) goto errexit ;
          spnt = npnt ;
         } while( spnt != startPnt ) ;
      }

//  Triangulate DTM Object

    tempDtmP->ppTol = 0.0 ;
    tempDtmP->plTol = 0.0 ;
    if( bcdtmObject_createTinDtmObjectOverload (tempDtmP,1,0.0,false,false) ) goto errexit ;

//  Remove None Feature Hull Lines

    if( bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtmP)) goto errexit ;

//  Check Hull Points Are Coincident With Tin Points

    if( cdbg )
      {
       spnt = tempDtmP->hullPoint ;
       do
         {
          ppnt = ( long) pointAddrP(tempDtmP,spnt)->z ;
          npnt = ( long) pointAddrP(tempDtmP,nodeAddrP(tempDtmP,spnt)->hPtr)->z ;
          if( ! bcdtmList_testLineDtmObject(dtmP,ppnt,npnt))
            {
             bcdtmWrite_message(1,0,0,"Unconnected Hull Points") ;
             goto errexit ;
            }
          spnt = nodeAddrP(tempDtmP,spnt)->hPtr ;
         } while( spnt != tempDtmP->hullPoint ) ;
      }

//  Insert New Tptr Polygon

    *firstPointP = ( long ) pointAddrP(tempDtmP,tempDtmP->hullPoint)->z ;
    spnt = tempDtmP->hullPoint ;
    do
      {
       ppnt = ( long) pointAddrP(tempDtmP,spnt)->z ;
       npnt = ( long) pointAddrP(tempDtmP,nodeAddrP(tempDtmP,spnt)->hPtr)->z ;
       nodeAddrP(dtmP,ppnt)->tPtr = npnt ;
       spnt = nodeAddrP(tempDtmP,spnt)->hPtr ;
      } while( spnt != tempDtmP->hullPoint ) ;

//  Check Tptr Polygon Connectivity

    if( cdbg )
      {
       if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,*firstPointP,0))
         {
          bcdtmWrite_message(1,0,0,"Connectivity Error In Tptr Polygon") ;
          goto errexit ;
         }
      }
   }

// Place Surrounding Polygon Around Initial Feature

 else
   {
    if( bcdtmDrainage_placePolygonAroundZeroSlopePolygonDtmObject(dtmP,&startPnt)) goto errexit ;
    *firstPointP = startPnt ;
   }

// Log Number Of Touching Features

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Touching Zero Slope Polygons = %8ld",numTouchingFeatures) ;
    for( tfP = touchingFeaturesP ; tfP < touchingFeaturesP + numTouchingFeatures ; ++tfP )
      {
       bcdtmWrite_message(0,0,0,"TouchingFeature[%8ld] = %8ld",(long)(tfP-touchingFeaturesP),*tfP) ;
      }

//  Log Final Area

     bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,*firstPointP,&area,&direction) ;
     bcdtmWrite_message(0,0,0,"Final   Tptr Polygon Area = %20.5lf Direction = %2ld",area,direction) ;
   }


// Clean Up

 cleanup :
 if( touchingFeaturesP != nullptr ) { free(touchingFeaturesP) ; touchingFeaturesP = nullptr ; }
 if( tempDtmP          != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;

// Return

 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Placing Tptr Polygon Around Touching Zero Slope Polygons Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Placing Tptr Polygon Around Touching Zero Slope Polygons Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_markInternalZeroSlopePolygonPointsDtmObject
(
 BC_DTM_OBJ *dtmP,                            // ==> Pointer To Dtm Object
 long       **zeroSlopePtsPP,                 // <== Points Internal To Zero Slope Polygons
 long       *numMarkedP,                      // <== Number Of Points Internal To Zero Slope Polygons
 long       *zeroSlopeFeatureStartP           // <== First Zero Slope Polygon Feature In DTM
)
/*
** This Function Marks All Points Internal To Zero Slope Polygons
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long *longP,pnt,sPnt,dtmFeature,numInternal ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Marking Internal Zero Slope Polygon Points") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"*zeroSlopePtsPP = %p",*zeroSlopePtsPP) ;
    bcdtmWrite_message(0,0,0,"*numMarkedP     = %8ld",*numMarkedP) ;
   }
/*
** Initialise
*/
 *numMarkedP = 0 ;
 if( *zeroSlopePtsPP != nullptr ) { free(*zeroSlopePtsPP ) ; *zeroSlopePtsPP = nullptr ; }
 *zeroSlopeFeatureStartP = -1 ;
/*
** Check For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM Is Triangulated
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
**   Scan For Zero Slope Polygon Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::ZeroSlopePolygon && dtmFeatureP->internalToDtmFeature == 0 )
      {
       if( *zeroSlopeFeatureStartP == -1 ) *zeroSlopeFeatureStartP = dtmFeature ;
/*
**     Allocate Mark Memory If Not Allocated
*/
       if( *zeroSlopePtsPP == nullptr )
         {
          *zeroSlopePtsPP = ( long * ) malloc(dtmP->numPoints * sizeof(long)) ;
          if( *zeroSlopePtsPP == nullptr )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
          for( longP = *zeroSlopePtsPP ; longP < *zeroSlopePtsPP + dtmP->numPoints ; ++longP ) *longP = dtmP->nullPnt ;
         }
/*
**     Copy DTM Feature To Tptr Array
*/
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&sPnt)) goto errexit ;
/*
**     Mark Internal Points
*/
       if( bcdtmDraiange_markPointsInternalToAZeroSlopeTptrPolygonDtmObject(dtmP,sPnt,*zeroSlopePtsPP,dtmFeature,&numInternal)) goto errexit ;
       *numMarkedP = *numMarkedP + numInternal ;
/*
**     Mark Points On Tptr Hull
*/
       pnt = sPnt ;
       do
         {
          *(*zeroSlopePtsPP+pnt) = dtmFeature ;
          pnt = nodeAddrP(dtmP,pnt)->tPtr ;
         } while( pnt != sPnt ) ;
/*
**     Null Tptr List
*/
       if( bcdtmList_nullTptrListDtmObject(dtmP,sPnt)) goto errexit ;
      }
   }
/*
** Report And Set To Null None Null Tptr Points
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking For None Null Tptr Values") ;
    if( bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,0))
      {
       bcdtmWrite_message(1,0,0,"None Null Tptr Values") ;
       goto errexit ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Internal Zero Slope Polygon Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Internal Zero Slope Polygon Points Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_testForZeroSlopePolygonPointDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       point
 )
/*
** This Function Tests If a Point is On A Zero Slope Polygon Hull
*/
{
 long clc ;
/*
** Check For Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
/*
**  Scan Feature List Points For Point
*/
    clc = nodeAddrP(dtmP,point)->fPtr ;
    while( clc != dtmP->nullPtr )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::ZeroSlopePolygon ) return(1) ;
       clc = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDraiange_markPointsInternalToAZeroSlopeTptrPolygonDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       startPnt,
 long       *zeroIndexP,
 long       mark,
 long       *numMarkedP
 )
/*
** This Function Marks Points Internal To A Zero Slope Tptr Polygon
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long nextPnt,priorPnt,listPntnt,listPnt,clc,pnt,firstPnt,lastPnt;
 double elevation ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Marking Internal Zero Slope Tptr Polygon Points") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPnt     = %p",startPnt) ;
    bcdtmWrite_message(0,0,0,"zeroIndexP   = %p",zeroIndexP) ;
    bcdtmWrite_message(0,0,0,"mark         = %8ld",mark) ;
    bcdtmWrite_message(0,0,0,"*numMarkedP  = %8ld",*numMarkedP) ;
   }
/*
** Initialise
*/
 *numMarkedP = 0 ;
 elevation = pointAddrP(dtmP,startPnt)->z ;
 firstPnt = lastPnt = dtmP->nullPnt ;
/*
** Check For Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,startPnt) ;
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt ) goto errexit ;
/*
** Scan Around Tptr Polygon And Mark Immediately Internal Points And Create Internal Tptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Immediately Internal To Tptr Polygon") ;
 priorPnt= startPnt ;
 pnt = nodeAddrP(dtmP,startPnt)->tPtr ;
 do
   {
    listPnt = nextPnt = nodeAddrP(dtmP,pnt)->tPtr ;
    if(( listPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,listPnt)) < 0 ) goto errexit ;
/*
**  Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
    while ( listPnt != priorPnt )
      {
       if( pointAddrP(dtmP,listPnt)->z == elevation )
         {
          if( nodeAddrP(dtmP,listPnt)->tPtr == dtmP->nullPnt && ! bcdtmDrainage_testForZeroSlopePolygonPointDtmObject(dtmP,listPnt)  )
            {
             if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = listPnt ;  }
             else                           { nodeAddrP(dtmP,lastPnt)->tPtr = -(listPnt+1) ; lastPnt = listPnt ; }
             nodeAddrP(dtmP,lastPnt)->tPtr = -(lastPnt+1) ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"** Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld",listPnt,pointAddrP(dtmP,listPnt)->x,pointAddrP(dtmP,listPnt)->y,pointAddrP(dtmP,listPnt)->z,pnt,firstPnt,lastPnt) ;
            }
         }
       if(( listPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,listPnt)) < 0 ) goto errexit ; ;
      }
/*
**   Reset For Next Point On Tptr Polygon
*/
    priorPnt = pnt ;
    pnt  = nextPnt ;
   } while ( priorPnt!= startPnt ) ;
/*
** Scan External Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
 if( firstPnt != dtmP->nullPnt )
   {
    pnt = firstPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld",pnt) ;
       listPntnt = pnt ;
       clc = nodeAddrP(dtmP,pnt)->cPtr ;
       while( clc != dtmP->nullPtr )
         {
          listPnt  = clistAddrP(dtmP,clc)->pntNum ;
          clc      = clistAddrP(dtmP,clc)->nextPtr ;
          if( pointAddrP(dtmP,listPnt)->z == elevation && nodeAddrP(dtmP,listPnt)->tPtr == dtmP->nullPnt && ! bcdtmDrainage_testForZeroSlopePolygonPointDtmObject(dtmP,listPnt)  )
            {
             nodeAddrP(dtmP,lastPnt)->tPtr = -(listPnt+1) ;
             lastPnt = listPnt ;
             nodeAddrP(dtmP,lastPnt)->tPtr = -(lastPnt+1) ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"## Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld",listPnt,pointAddrP(dtmP,listPnt)->x,pointAddrP(dtmP,listPnt)->y,pointAddrP(dtmP,listPnt)->z,pnt,firstPnt,lastPnt) ;
            }
         }
       nextPnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       pnt = nextPnt ;
      } while ( listPntnt != pnt  ) ;
   }
/*
** Mark And Null Out Internal Tptr List
*/
 if( firstPnt != dtmP->nullPnt )
   {
    pnt = firstPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Internal Tptr List") ;
    do
      {
       nextPnt = -nodeAddrP(dtmP,pnt)->tPtr - 1 ;
       *(zeroIndexP+pnt) = mark ;
       nodeAddrP(dtmP,pnt)->tPtr = dtmP->nullPnt ;
       ++*numMarkedP ;
       pnt = nextPnt ;
      } while ( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt ) ;
   }
/*
** Write Number Marked
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",*numMarkedP) ;
/*
** Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Points Internal To A Zero Slope Tptr Polygon Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Marking Points Internal To A Zero Slope Tptr Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_placeSptrPolygonAroundZeroSlopeTrianglesDtmObject
(
 BC_DTM_OBJ *dtmP,                                 // ==> Pointer To DTM Object
 int        point1,                                // ==> First Point Of Zero Slope Edge
 int        point2,                                // ==> Second Point Of Zero Slope Edge
 DTMDirection&       sPtrDirection,                         // <== Polygon Direction
 int&       startPoint                             // <== Sptr Polygon Start Point
)
//
// This Function Places A Sptr Polygon Around Zero Slope Triangles.
// It Starts From A Zero Edge - Point 1 and Point 2 Must Have the same Elevation
// And The Point Closwise From Point 1 and Point 2 Must Have The Same Elevation.
// The Point Counter Clockwise Must Have A Different Elevation
//
{
 int          ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 int          sp,np,cp,lcp,antPoint,clkPoint ;
 long         hullLine;
 DTMDirection direction ;
 double       elevation,area ;
 bool         zeroEdge=true ;

//  Log Function Arguments
//if( point1 == 170625 && point2 == 170626 ) dbg=1 ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Placing Sptr Polygon Around Zero Slope Triangles") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP ) ;
    bcdtmWrite_message(0,0,0,"point1         = %8ld",point1 ) ;
    bcdtmWrite_message(0,0,0,"point2         = %8ld",point2 ) ;
    bcdtmWrite_message(0,0,0,"sPtrDirection  = %8ld",sPtrDirection ) ;
    bcdtmWrite_message(0,0,0,"startPoint     = %8ld",startPoint ) ;
   }

//  Initialise Variables

 startPoint = dtmP->nullPnt ;
 elevation = pointAddrP(dtmP,point1)->z ;
 if( ( clkPoint = bcdtmList_nextClkDtmObject(dtmP,point1,point2)) < 0 ) goto errexit ;
 if( ( antPoint = bcdtmList_nextAntDtmObject(dtmP,point1,point2)) < 0 ) goto errexit ;

//  Log Variables

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"point1   = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",point1,nodeAddrP(dtmP,point1)->hPtr,pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y,pointAddrP(dtmP,point1)->z) ;
    bcdtmWrite_message(0,0,0,"point2   = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",point2,nodeAddrP(dtmP,point2)->hPtr,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y,pointAddrP(dtmP,point2)->z) ;
    bcdtmWrite_message(0,0,0,"clkPoint = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",clkPoint,nodeAddrP(dtmP,clkPoint)->hPtr,pointAddrP(dtmP,clkPoint)->x,pointAddrP(dtmP,clkPoint)->y,pointAddrP(dtmP,clkPoint)->z) ;
    bcdtmWrite_message(0,0,0,"antPoint = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",antPoint,nodeAddrP(dtmP,antPoint)->hPtr,pointAddrP(dtmP,antPoint)->x,pointAddrP(dtmP,antPoint)->y,pointAddrP(dtmP,antPoint)->z) ;
   }

//  Check For Zero Slope Edge

 if( cdbg )
   {
    if     ( nodeAddrP(dtmP,point1)->hPtr == point2 ) zeroEdge = false ;
    else if( pointAddrP(dtmP,point2)->z != elevation || pointAddrP(dtmP,clkPoint)->z != elevation || pointAddrP(dtmP,antPoint)->z == elevation ) zeroEdge = false ;
    if( zeroEdge == false )
      {
       bcdtmWrite_message(2,0,0,"Not A Zero Edge") ;
       goto errexit ;
      }
   }

// Zero Edge Detected

 if( zeroEdge )
   {

//  Extract Zero Slope Polygon

    sp = point1 ;
    np = point2 ;
    do
      {
if( nodeAddrP(dtmP,sp)->sPtr != dtmP->nullPnt )
{
 if( dbg ) bcdtmWrite_message(0,0,0,"********** Knot At Point sp = %8ld",sp) ;
 cp = sp ;
 do
   {
     lcp = nodeAddrP(dtmP,cp)->sPtr ;
     nodeAddrP(dtmP,cp)->sPtr = dtmP->nullPnt ;
     cp = lcp ;
   } while( cp != sp ) ;
}
       nodeAddrP(dtmP,sp)->sPtr = np ;
       lcp = cp = sp ;
       if( ( cp = bcdtmList_nextAntDtmObject(dtmP,np,cp)) < 0 ) goto errexit ;
       if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,np,cp,&hullLine)) goto errexit ;
       while( ! hullLine && pointAddrP(dtmP,cp)->z == pointAddrP(dtmP,sp)->z  )
         {
          lcp = cp ;
          if( ( cp = bcdtmList_nextAntDtmObject(dtmP,np,cp)) < 0 ) goto errexit ;
          if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,np,cp,&hullLine)) goto errexit ;
         }
       if( pointAddrP(dtmP,cp)->z != pointAddrP(dtmP,sp)->z  ) cp = lcp ;
       sp = np ;
       np = cp ;
     } while ( sp != point1 ) ;

//  Check Connectivity Sptr Polygon

    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of Sptr Polygon ** startPoint = %8ld",point1) ;
    if( bcdtmList_checkConnectivitySptrPolygonDtmObject(dtmP,point1,0))
      {
       bcdtmWrite_message(2,0,0,"Connectivity Error In Sptr Polygon") ;
       goto errexit ;
      }

//  Get Direction Of Sptr Polygon

    bcdtmMath_calculateAreaAndDirectionSptrPolygonDtmObject(dtmP,point1,&area,&direction) ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"sPtr Polygon ** area = %20.10lf direction = %2ld",area,direction) ;
       bcdtmWrite_message(0,0,0,"Internal Polygon Boundary Found") ;
      }

//  Set Start Point

    startPoint   = point1 ;
    sPtrDirection = direction ;
   }

//  Processing Completed

 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Placing Sptr Polygon Around Zero Slope Triangles Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Placing Sptr Polygon Around Zero Slope Triangles Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_expandPondToExitPointDtmObject
(
 BC_DTM_OBJ                *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables         *drainageTablesP,           // ==> Pointer To Drainage Tables
 DTMZeroSlopePolygonVector *zeroSlopePolygonsP,        // ==> Pointer To Zero Slope Polygons
 int                       *zeroSlopePointsIndexP,     // ==> Index To Zero Slope Polygons
 long                      startPoint,                 // ==> Start Point On Tptr Polygon
 long                      *exitPointP,                // <== Exit Point On Tptr Polygon
 long                      *priorPointP,               // <== Prior Point To Exit Point On Tptr Polygon
 long                      *nextPointP                 // <== Next Point After Exit Point On Tptr Polygon
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(1),tdbg=DTM_TIME_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 int    ap,cp,p1,p2,edgeType,edgePnt1,edgePnt2,numPondPts,numZeroEdges ;
 long   sPnt,nPnt,pPnt,lowPoint,exitPointFound,extStartPoint,extEndPoint ;
 long   saveStartPoint,hullPoint,startTime,numFeaturePts ;
 double area,lowPointZ,lastArea=0.0 ;
 bool   expandOut=false,expandPoints=false,expandAtPoint=false,pondExitPoint=false ;
 DTMDirection direction ;
 DPoint3d     *featurePtsP=nullptr ;
 BC_DTM_OBJ   *dataP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Expanding Pond To Exit Point") ;
    bcdtmWrite_message(0,0,0,"dtmP                  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP       = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePolygons     = %p",zeroSlopePolygonsP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePointsIndexP = %p",zeroSlopePointsIndexP) ;
    bcdtmWrite_message(0,0,0,"startPoint            = %8ld",startPoint) ;
    if( dbg == 1 ) bcdtmList_writeTptrListDtmObject(dtmP,startPoint) ;
   }
/*
** Initialise
*/
 *priorPointP   = dtmP->nullPnt ;
 *exitPointP    = dtmP->nullPnt ;
 *nextPointP    = dtmP->nullPnt ;
 exitPointFound = 0 ;
 saveStartPoint = startPoint ;
 startTime      = bcdtmClock() ;

// Log Start Polygon For Expansion

 if( dbg == 1 )
   {
    if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
    if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
    if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"expandStart.dat")) goto errexit ;
    if( bcdtmObject_initialiseDtmObject(dataP)) goto errexit ;
   }
/*
** Check Connectivity Of Tptr Polygon
*/
 if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPoint,0))
   {
    bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
    goto errexit ;
   }
/*
** Check Area And Direction Of Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Area And Direction Tptr Polygon") ;
 bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction) ;
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Start ** direction = %2ld ** area = %20.10lf",direction,area) ;
 if( direction == DTMDirection::Clockwise )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"direction = %2ld ** area = %20.10lf",direction,area) ;
    bcdtmWrite_message(2,0,0,"Direction Error In Tptr Polygon") ;
    goto errexit ;
   }
 lastArea = area ;
/*
** Iteratively Expand Out From Lowest Point On Pond Boundary
*/
 while ( ! exitPointFound )
   {

//  Get Lowest Point On Pond Boundary

    if( dbg ) bcdtmWrite_message(0,0,0,"Getting Lowest Point On Pond Boundary") ;
    lowPoint = sPnt = startPoint ;
    do
      {
       if( pointAddrP(dtmP,sPnt)->z < pointAddrP(dtmP,lowPoint)->z ) lowPoint = sPnt ;
       sPnt = nodeAddrP(dtmP,sPnt)->tPtr ;
      } while( sPnt != startPoint ) ;

//  Set Low Point Elevation

    lowPointZ = pointAddrP(dtmP,lowPoint)->z ;
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Lowest Pond Point = %8ld  ** %12.5lf %12.5lf %10.4lf",lowPoint,pointAddrP(dtmP,lowPoint)->x,pointAddrP(dtmP,lowPoint)->y,pointAddrP(dtmP,lowPoint)->z) ;

//  Expand Pond At Low Point Elevation Points

    expandOut = true ;
    while ( expandOut )
      {
       expandOut = false ;

//     Expand Pond Over Zero Slope Triangles At The Low Point Elevation

       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Expanding Pond Over Zero Slope Triangles") ;
       if( bcdtmDrainage_expandPondOverSlopeTrianglesDtmObject(dtmP,&startPoint,lowPointZ)) goto errexit ;
       if( dbg == 1 )
         {
          bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction) ;
          bcdtmWrite_message(0,0,0,"After Zero Slope Triangle Expansion ** direction = %2ld ** area = %20.10lf",direction,area) ;
         }

//     Log Starting Pond Polygon

       if( dbg == 2 )
         {
          if( dataP == nullptr ) if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
          if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"afterZeroSlope.dat")) goto errexit ;
          if( bcdtmObject_initialiseDtmObject(dataP)) goto errexit ;
         }

//     Expand Pond Over Zero Slope Triangles And Zero Slope Sump Lines At The Low Point Elevation

       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Expanding Pond Over Zero Slope Sump Lines") ;
       expandOut = true ;
       while( expandOut )
         {

//        Initialise Starting Conditions

          expandOut = false ;
          sPnt = startPoint ;
          pPnt = nodeAddrP(dtmP,sPnt)->tPtr ;
          while( nodeAddrP(dtmP,pPnt)->tPtr != sPnt )
            {
             if( ( pPnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,pPnt)) < 0 ) goto errexit ;
            }

//        Log Number Of Pond Points

          if( dbg == 2 )
            {
             numPondPts = 0 ;
             p1 = sPnt ;
             do
               {
                ++numPondPts ;
                p1 = nodeAddrP(dtmP,p1)->tPtr ;
               } while ( p1 != sPnt ) ;
             bcdtmWrite_message(0,0,0,"Zero Slope Expansion Start ** Number Of Pond Points = %8ld",numPondPts) ;
            }

//        Scan Pond Boundary And Expand Out Zero Slope

          do
            {
             nPnt = nodeAddrP(dtmP,sPnt)->tPtr ;
             if( pointAddrP(dtmP,sPnt)->z == lowPointZ )
               {

                if( dbg == 1 ) bcdtmWrite_message(0,0,0,"lowPoint[%8ld] ** %12.5lf %12.5lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;

//              Only Expand If Not A Pond Exit Point

                startTime = bcdtmClock() ;
                if( bcdtmDrainage_countNumberOfExternalZeroEdgesAtPointDtmObject(dtmP,pPnt,sPnt,nPnt,lowPointZ,numZeroEdges)) goto errexit ;
                if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Number Of External Zero Edges At %8ld Point = %8ld",sPnt,numZeroEdges) ;
                if( numZeroEdges > 0 )
                  {
                   expandPoints = true ;

//                 Check For Expansion To Zero Slope Triangles

                   edgeType = 0 ;
                   if( numZeroEdges > 1 )
                     {
                      if( pointAddrP(dtmP,pPnt)->z > lowPointZ && pointAddrP(dtmP,sPnt)->z == lowPointZ && pointAddrP(dtmP,nPnt)->z > lowPointZ )
                        {
                         if( bcdtmDrainage_scanPondBoundaryPointForAnExternalZeroEdgeDtmObject(dtmP,sPnt,lowPointZ,edgePnt1,edgePnt2,edgeType)) goto errexit ;
                         if( dbg == 2 && edgeType ) bcdtmWrite_message(0,0,0,"01 - edgeType = %2ld ** edgePnt1 = %10d edgePnt2 = %10d",edgeType,edgePnt1,edgePnt2) ;
                        }
                      else
                        {
                         if( bcdtmDrainage_scanPondBoundaryPointForAnExternalZeroEdgeDtmObjectTwo(dtmP,pPnt,sPnt,nPnt,lowPointZ,edgePnt1,edgePnt2,edgeType)) goto errexit ;
                         if( dbg == 2 && edgeType ) bcdtmWrite_message(0,0,0,"02 - edgeType = %2ld ** edgePnt1 = %10d edgePnt2 = %10d",edgeType,edgePnt1,edgePnt2) ;
                        }
                     }

//                 Expand Pond About Point

                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Expanding At Point %8ld ** %12.5lf %12.5lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                   if( bcdtmDrainage_expandPondAboutPointDtmObject(dtmP,sPnt,lastArea,&startPoint,&area,&extStartPoint,&extEndPoint)) goto errexit ;
                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Expanding At Point %8ld Completed ** area = %20.5lf",sPnt,area) ;
                   if( cdbg )      // Check Expanded Pond Area Is Larger
                     {
                      if( area < lastArea )
                        {
                         bcdtmWrite_message(2,0,0,"Pond Area Decreased ** lastArea = %12.5lf expandedArea = %12.5lf",lastArea,area) ;
                         goto errexit ;
                        }
                     }
                   lastArea = area ;

//                 Union Pond Boundary With Zero Slope Triangles

                   if( edgeType )
                     {
                      if( dbg == 1 ) bcdtmWrite_message(0,0,0,"** Expanding Pond From Zero Edge ** %8ld %8ld",edgePnt1,edgePnt2) ;
                      if( bcdtmDrainage_expandPondOverZeroSlopeTrianglesFromZeroEdgeDtmObject(dtmP,&startPoint,edgeType,edgePnt1,edgePnt2,lowPointZ)) goto errexit ;
                      if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction)) goto errexit ;
                      if( cdbg )      // Check Expanded Pond Area Is Larger
                        {
                         if( area < lastArea )
                           {
                            bcdtmWrite_message(2,0,0,"Pond Area Decreased ** lastArea = %12.5lf expandedArea = %12.5lf",lastArea,area) ;
                            goto errexit ;
                           }
                        }
                      lastArea = area ;
                     }

//                 Reset Scan Parameters

                   sPnt = nodeAddrP(dtmP,startPoint)->tPtr ;
                   nPnt = nodeAddrP(dtmP,sPnt)->tPtr ;
                  }
               }
             pPnt = sPnt ;
             sPnt = nPnt ;
            } while( sPnt != startPoint ) ;
         }

//        Log Number Of Pond Points

          if( dbg == 2 )
            {
             numPondPts = 0 ;
             p1 = sPnt ;
             do
               {
                ++numPondPts ;
                p1 = nodeAddrP(dtmP,p1)->tPtr ;
               } while ( p1 != sPnt ) ;
             bcdtmWrite_message(0,0,0,"Zero Slope Expansion End   ** Number Of Pond Points = %8ld",numPondPts) ;
            }

         }

//     Log Area After

       if( dbg == 1 )
         {
          bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction) ;
          bcdtmWrite_message(0,0,0,"After Sump Line Expansion ** direction = %2ld ** area = %20.10lf",direction,area) ;
          numPondPts = 0 ;
          p1 = startPoint ;
          do
            {
             ++numPondPts ;
             p1 = nodeAddrP(dtmP,p1)->tPtr ;
            } while ( p1 != startPoint ) ;
           bcdtmWrite_message(0,0,0,"After Sump Line Expansion ** Number Of Pond Points = %8ld",numPondPts) ;
         }

//     Check For Pond Exit Point After Zero Slope Expansion

       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Checking For Exit Point After Zero Slope Expansion") ;
       if( bcdtmDrainage_scanPondForExitPointDtmObject(dtmP,drainageTablesP,startPoint,lowPointZ,&exitPointFound,exitPointP,priorPointP,nextPointP)) goto errexit ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"After Zero Slope Expansion ** exitPointFound = %2ld ** priorPoint = %10ld exitPoint = %10ld nextPoint = %10ld",exitPointFound,*priorPointP,*exitPointP,*nextPointP) ;

//     Expand Pond At All Low Points That Are Not Exit Points

       if( ! exitPointFound )
         {

//        Check For A Zero Edge

          if( cdbg == 1 )
            {
             bcdtmWrite_message(0,0,0,"Checking For A Zero Edge") ;
             p1 = startPoint ;
             do
               {
                p2 = nodeAddrP(dtmP,p1)->tPtr ;
                if( pointAddrP(dtmP,p1)->z == lowPointZ && pointAddrP(dtmP,p2)->z == lowPointZ )
                  {
                   if( bcdtmList_testForHullPointDtmObject(dtmP,p1,&hullPoint)) goto errexit ;
                   if( ! hullPoint )
                     {
                      if( ( ap = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                      if( ( cp = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                      if( pointAddrP(dtmP,cp)->z == lowPointZ && pointAddrP(dtmP,ap)->z != lowPointZ )
                        {
                         bcdtmWrite_message(0,0,0,"Zero Edge Detected ** cp->z = %10.4lf ap->z = %10.4lf",pointAddrP(dtmP,cp)->z,pointAddrP(dtmP,ap)->z) ;
                        }
                      if( pointAddrP(dtmP,cp)->z == lowPointZ && pointAddrP(dtmP,ap)->z == lowPointZ )
                        {
                         bcdtmWrite_message(0,0,0,"Internal Zero Edge ** cp->z = %10.4lf ap->z = %10.4lf",pointAddrP(dtmP,cp)->z,pointAddrP(dtmP,ap)->z) ;
                        }
                     }
                  }
                p1 = p2 ;
               } while( p1 != startPoint ) ;
            }

//        Expand Pond

          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Expanding Pond At Low Points") ;
          expandPoints = true ;
          while( expandPoints )
            {
             expandPoints = false ;
             sPnt = startPoint ;
             pPnt = nodeAddrP(dtmP,sPnt)->tPtr ;
             while( nodeAddrP(dtmP,pPnt)->tPtr != sPnt )
               {
                if( ( pPnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,pPnt)) < 0 ) goto errexit ;
               }

//           Log Number Of Pond Points

             if( dbg == 2 )
               {
                numPondPts = 0 ;
                p1 = sPnt ;
                do
                 {
                  ++numPondPts ;
                  p1 = nodeAddrP(dtmP,p1)->tPtr ;
                 } while ( p1 != sPnt ) ;
               bcdtmWrite_message(0,0,0,"Low Point Expansion Start ** Number Of Pond Points = %8ld",numPondPts) ;
              }

//           Scan Pond

             do
               {
                nPnt = nodeAddrP(dtmP,sPnt)->tPtr ;
                if( pointAddrP(dtmP,sPnt)->z == lowPointZ )
                  {

                   if( dbg == 1 ) bcdtmWrite_message(0,0,0,"lowPoint[%8ld] ** %12.5lf %12.5lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;

//                 Only Expand If Not A Pond Exit Point

                   if( bcdtmDrainage_checkForPondExitPointDtmObject(dtmP,sPnt,lowPointZ,pondExitPoint)) goto errexit ;
                   if( ! pondExitPoint )
                     {

//                    Expand About Low Point

                      expandPoints = true ;
                      if( dbg == 1 )
                        {
                         bcdtmWrite_message(0,0,0,"Expanding At Point %8ld",sPnt) ;
                         bcdtmWrite_message(0,0,0,"pPnt[%8ld] pPnt->hPtr = %10ld ** %12.5lf %12.5lf %10.4lf",pPnt,nodeAddrP(dtmP,pPnt)->hPtr,pointAddrP(dtmP,pPnt)->x,pointAddrP(dtmP,pPnt)->y,pointAddrP(dtmP,pPnt)->z) ;
                         bcdtmWrite_message(0,0,0,"sPnt[%8ld] sPnt->hPtr = %10ld ** %12.5lf %12.5lf %10.4lf",sPnt,nodeAddrP(dtmP,sPnt)->hPtr,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                         bcdtmWrite_message(0,0,0,"nPnt[%8ld] nPnt->hPtr = %10ld ** %12.5lf %12.5lf %10.4lf",nPnt,nodeAddrP(dtmP,nPnt)->hPtr,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointAddrP(dtmP,nPnt)->z) ;
                        }
                      if( bcdtmDrainage_expandPondAboutPointDtmObject(dtmP,sPnt,lastArea,&startPoint,&area,&extStartPoint,&extEndPoint)) goto errexit ;
                      if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Expanding At Point %8ld Completed",sPnt) ;
                      if( cdbg )      // Check Expanded Pond Area Is Larger
                        {
                         if( area < lastArea )
                           {
                            bcdtmWrite_message(2,0,0,"Pond Area Decreased ** lastArea = %12.5lf expandedArea = %12.5lf",lastArea,area) ;
                            goto errexit ;
                           }
                        }
                      lastArea = area ;

//                    Reset Scan Parameters

                      sPnt = nodeAddrP(dtmP,startPoint)->tPtr ;
                      pPnt = nodeAddrP(dtmP,sPnt)->tPtr ;
                      while( nodeAddrP(dtmP,pPnt)->tPtr != sPnt )
                        {
                         if( ( pPnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,pPnt)) < 0 ) goto errexit ;
                        }

//                    Check For A Zero Edge

                      if( bcdtmDrainage_scanPondBoundarySectionForAZeroEdgeDtmObject(dtmP,extStartPoint,extEndPoint,lowPointZ,edgePnt1,edgePnt2,edgeType )) goto errexit ;
                      if( edgeType )
                        {
                         if( dbg == 1 )
                           {
                            bcdtmWrite_message(0,0,0,"extStartPoint = %8ld extEndPoint = %8ld",extStartPoint,extEndPoint) ;
                            p1 = extStartPoint ;
                            do
                              {
                               p2 = nodeAddrP(dtmP,p1)->tPtr ;
                               bcdtmWrite_message(0,0,0,"Pond Point[%8ld] ** tPtr = %8ld",p1,p2) ;
                               p1 = p2 ;
                              } while( p1 != nodeAddrP(dtmP,extEndPoint)->tPtr ) ;
                           }
                         if( dbg == 1 ) bcdtmWrite_message(0,0,0,"edgeType = %2d ** edgePnt1 = %8ld edgePnt2 = %8ld",edgeType,edgePnt1,edgePnt2) ;
                         if( bcdtmDrainage_expandPondOverZeroSlopeTrianglesFromZeroEdgeDtmObject(dtmP,&startPoint,edgeType,edgePnt1,edgePnt2,lowPointZ)) goto errexit ;
                        }

//                    Look For A False Internal Zero Edge At The Low Point Elevation

                      if( cdbg == 2 )
                        {
                         p1 = startPoint ;
                         do
                           {
                            p2 = nodeAddrP(dtmP,p1)->tPtr ;
                            if( pointAddrP(dtmP,p1)->z == lowPointZ && pointAddrP(dtmP,p2)->z == lowPointZ )
                              {
                               if( bcdtmList_testForHullPointDtmObject(dtmP,p1,&hullPoint)) goto errexit ;
                               if( ! hullPoint )
                                 {
                                  if( ( ap = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                                  if( ( cp = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                                  if( pointAddrP(dtmP,cp)->z == lowPointZ && pointAddrP(dtmP,ap)->z == lowPointZ )
                                    {
                                     bcdtmWrite_message(0,0,0,"False Zero Edge Detected ** cp->z = %10.4lf ap->z = %10.4lf",pointAddrP(dtmP,cp)->z,pointAddrP(dtmP,ap)->z) ;
                                     if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
                                     if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                                     if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"flatExpansion.dat")) goto errexit ;
                                     if( bcdtmObject_initialiseDtmObject(dataP)) goto errexit ;
                                     goto errexit ;
                                    }
                                 }
                              }
                            p1 = p2 ;
                           } while( p1 != startPoint ) ;
                        }
                     }
                  }
                pPnt = sPnt ;
                sPnt = nodeAddrP(dtmP,sPnt)->tPtr ;
               } while( sPnt != startPoint && expandPoints == false ) ;
            }
         }

//     Log Number Of Pond Points

       if( dbg == 1 )
         {
          numPondPts = 0 ;
          p1 = sPnt ;
          do
            {
             ++numPondPts ;
             p1 = nodeAddrP(dtmP,p1)->tPtr ;
            } while ( p1 != sPnt ) ;
          bcdtmWrite_message(0,0,0,"Low Point Expansion End   ** Number Of Pond Points = %8ld",numPondPts) ;
         }

//     Check For Pond Exit Point

       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Checking For Exit Point") ;
       if( bcdtmDrainage_scanPondForExitPointDtmObject(dtmP,drainageTablesP,startPoint,lowPointZ,&exitPointFound,exitPointP,priorPointP,nextPointP)) goto errexit ;
      }

// Log Pond Exit Point And Expanded Tptr Polygon Stats

 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"exitPointFound = %2ld ** priorPoint = %10ld exitPoint = %10ld nextPoint = %10ld",exitPointFound,*priorPointP,*exitPointP,*nextPointP) ;
    bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,*exitPointP,&area,&direction) ;
    bcdtmWrite_message(0,0,0,"Expanded Tptr Polygon ** area = %20.10lf direction = %2ld",area,direction) ;
   }

// Log Expanded Tptr Polygon

 if( dbg == 1 )
   {
    if( dataP == nullptr ) bcdtmObject_createDtmObject(&dataP) ;
    if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
    if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"finalPondExpansion.dat")) goto errexit ;
    if( bcdtmObject_initialiseDtmObject(dataP)) goto errexit ;
   }

// Check For Valid Pond

 if( cdbg == 1 )
   {
    bool pondValid=true ;
    if( bcdtmDrainage_validatePondDtmObject(dtmP,*exitPointP,pondValid)) goto errexit ;
    if( pondValid ) bcdtmWrite_message(0,0,0,"Valid Pond Determined") ;
    else            bcdtmWrite_message(0,0,0,"Valid Pond Not Determined") ;
   }
/*
** Log Time To Expand Pond
*/
 if( tdbg )
   {
    bcdtmWrite_message(0,0,0,"Time To Expand Pond To Exit Point = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    if( bcdtmClock_elapsedTime(bcdtmClock(),startTime) > 0.05 )
      {
       bcdtmWrite_message(0,0,0,"Time To Expand Pond To Exit Point = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( dataP         != nullptr ) bcdtmObject_destroyDtmObject(&dataP) ;
/*
** Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Pond To Exit Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Pond To Exit Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_expandPondOverSlopeTrianglesDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 long              *pondStartPointP,           // <=> Start Point On Tptr Polygon
 double            lowPointZ                   // ==> Low Point Z
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 int    edgePnt1, edgePnt2, zeroEdgeType, zeroStartPoint;
 DTMDirection expandDirection;
 long   numFeaturePts,startPoint,newStartPoint,startTime=bcdtmClock() ;
 double area,beforeArea,afterArea ;
 bool   iterate=true ;
 DTMDirection direction ;
 DPoint3d    *featurePtsP=nullptr ;
 BC_DTM_OBJ     *dataP=nullptr ;

// Log Function Arguements

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Expanding Pond Over Zero Slope Triangles") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"*pondStartPointP = %8ld ** %12.5lf %12.5lf %10.4lf",*pondStartPointP,pointAddrP(dtmP,*pondStartPointP)->x,pointAddrP(dtmP,*pondStartPointP)->y,pointAddrP(dtmP,*pondStartPointP)->z ) ;
    bcdtmWrite_message(0,0,0,"lowPointZ        = %10.4lf",lowPointZ) ;
   }

// Check Direction Of Tptr Polygon

  if( cdbg )
    {
     if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,*pondStartPointP,&beforeArea,&direction)) goto errexit ;
     if( dbg ) bcdtmWrite_message(0,0,0,"Before Expansion ** Tptr Polygon ** diection = %2ld area = %15.5lf",direction,beforeArea) ;
     if (direction != DTMDirection::AntiClockwise)
       {
        bcdtmWrite_message(2,0,0,"Tptr Polygon Is Not Counter Clockwise Before Pond Expansion") ;
        goto errexit ;
       }
    }

// Iteratively Expand Pond

 startPoint = *pondStartPointP ;
 while( iterate )
   {
    iterate = false ;

    if (cdbg)
        {
        if (bcdtmList_checkConnectivityTptrPolygonDtmObject (dtmP, startPoint, 0))
            {
            bcdtmWrite_message (2, 0, 0, "Tptr Polygon Connectivity Error");
            goto errexit;
            }

        }

//  Find A Zero Edge To Start Expansion From

    if( bcdtmDrainage_scanPondBoundaryForAZeroEdgeDtmObject(dtmP,startPoint,lowPointZ,edgePnt1,edgePnt2,zeroEdgeType)) goto errexit ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"zeroEdgeType = %8ld ** edgePnt1 = %8ld edgePnt2 = %8ld",zeroEdgeType,edgePnt1,edgePnt2) ;
       if( zeroEdgeType )
         {
          bcdtmWrite_message(0,0,0,"edgePnt1[%8ld] ** %12.5lf %12.5lf %10.4lf",edgePnt1,pointAddrP(dtmP,edgePnt1)->x,pointAddrP(dtmP,edgePnt1)->y,pointAddrP(dtmP,edgePnt1)->z) ;
          bcdtmWrite_message(0,0,0,"edgePnt2[%8ld] ** %12.5lf %12.5lf %10.4lf",edgePnt2,pointAddrP(dtmP,edgePnt2)->x,pointAddrP(dtmP,edgePnt2)->y,pointAddrP(dtmP,edgePnt2)->z) ;
         }
      }
    if( zeroEdgeType )
      {
       iterate = true ;

//     Polygonize Zero Slope Triangles

       if( dbg ) bcdtmWrite_message(0,0,0,"Placing Zero Slope Polygon Around Zero Slope Triangles") ;
       if( bcdtmDrainage_placeSptrPolygonAroundZeroSlopeTrianglesDtmObject(dtmP,edgePnt1,edgePnt2,expandDirection,zeroStartPoint)) goto errexit ;

//       bcdtmList_writeSptrListDtmObject(dtmP,zeroStartPoint) ;

//     Log Before Pond Boundary And Polygonised Zero Slope Triangles To File

       if( dbg )
         {
          if( dataP == nullptr ) if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
          if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,zeroStartPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,2,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( bcdtmList_copySptrListToPointArrayDtmObject(dtmP,zeroStartPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::ContourLine,2,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"intersectPolygons.dat")) goto errexit ;
          if( bcdtmObject_destroyDtmObject(&dataP)) goto errexit ;
         }

//     Expand Direction Counter Clockwise -  Represents An Internal Edge Of Zero Slope Triangles

       if (expandDirection == DTMDirection::AntiClockwise)
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Expanded Polygon Is Anti Clockwise") ;
          bcdtmList_nullTptrListDtmObject(dtmP,startPoint) ;
          bcdtmList_copySptrListToTptrListDtmObject(dtmP,zeroStartPoint) ;
          bcdtmList_nullSptrListDtmObject(dtmP,zeroStartPoint) ;

//        Expand To Outer Edge Of Zero Slope Triangles

          startPoint = zeroStartPoint ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Expanding To Outer Edge") ;
          if( bcdtmDrainage_expandPondToOuterEdgeOfZeroSlopeTrianglesDtmObject(dtmP,&startPoint,lowPointZ)) goto errexit ;
          if (startPoint == zeroStartPoint)
              iterate = false;
           }

//     Expand Direction Clockwise - Union Tptr And Sptr Polygons

       else if (expandDirection == DTMDirection::Clockwise)
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Expanded Polygon Is Clockwise") ;

         if (zeroStartPoint == startPoint)
             {
             iterate = false;

             if (bcdtmDrainage_getUnionOfPolygonsDtmObject (dtmP, startPoint, zeroStartPoint, newStartPoint)) goto errexit;
             startPoint = newStartPoint;

             if (cdbg)
                 {
                 if (bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject (dtmP, startPoint, &area, &direction)) goto errexit;

                 if (direction == DTMDirection::Clockwise)
                     {
                     bcdtmWrite_message (0, 0, 0, "Polygon Is Clockwise when it shouldn't be");
                     return DTM_ERROR;
                     }
                 }
             if( dbg ) bcdtmWrite_message(0,0,0, "Doing a union of itself so dont iterate anymore.") ;
             }
         else
             {
//        Get Union Of Polygons

              newStartPoint=0 ;
              if( dbg ) bcdtmWrite_message(0,0,0,"Getting Union Of Polygons") ;
              if( bcdtmDrainage_getUnionOfPolygonsDtmObject(dtmP,startPoint,zeroStartPoint,newStartPoint)) goto errexit ;
              if( dbg ) bcdtmWrite_message(0,0,0,"newStartPoint = %8ld",newStartPoint) ;
              if( newStartPoint == dtmP->nullPnt )
                {
                 bcdtmWrite_message(2,0,0,"Failed To Determine Polygon Union") ;
                 goto errexit ;
                }
              startPoint = newStartPoint ;

//        Log Union Polygon

              if( dbg )
                {
                 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction)) goto errexit ;
                 bcdtmWrite_message(0,0,0,"area = %12.3lf direction = %2ld",area,direction) ;
                 if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
                 if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
                 if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,2,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                 if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"unionPolygon.dat")) goto errexit ;
                 if( bcdtmObject_destroyDtmObject(&dataP)) goto errexit ;
                }
             }
         }
      }
  }

// Check Direction And Area Of Expanded Tptr Polygon

  if( cdbg )
    {
     if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&afterArea,&direction)) goto errexit ;
     if( dbg ) bcdtmWrite_message(0,0,0,"After  Expansion ** Tptr Polygon ** diection = %2ld area = %15.5lf",direction,afterArea) ;
     if (direction != DTMDirection::AntiClockwise)
       {
        bcdtmWrite_message(2,0,0,"Tptr Polygon Is Not Counter Clockwise After Pond Expansion") ;
        goto errexit ;
       }
     if( afterArea < beforeArea )
       {
        bcdtmWrite_message(0,0,0,"Area Before Expansion = %20.5lf Area After Expansion = %20.15lf",beforeArea,afterArea) ;
        bcdtmWrite_message(2,0,0,"Tptr Polygon Area Is Smaller After Pond Expansion") ;
        goto errexit ;
       }
    }


//  Reset Pond Start Point

*pondStartPointP = startPoint ;

// Clean Up

 cleanup :

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Pond Over Zero Slope Triangles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Pond Over Zero Slope Triangles Error") ;
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
int bcdtmDrainage_scanPondBoundaryForAZeroEdgeDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 long              pondStartPoint,             // ==> Start Point On Tptr Polygon
 double            lowPointZ,                  // ==> Low Point Z
 int&              edgePnt1,                   // <== Zero Edge Point 1
 int&              edgePnt2,                   // <== Zero Edge Point 2
 int&              zeroEdgeType                // <== Zero Edge Type < 0 = Not Found , 1 = External , 2 = Internal
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   sp,np,cp,ap,tp ;

// Log Function Arguements

 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Scanning Pond Boundary For A Zero Edge") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pondStartPoint  = %8ld",pondStartPoint) ;
    bcdtmWrite_message(0,0,0,"lowPointZ       = %8.3lf",lowPointZ) ;
   }

// Initialise

 zeroEdgeType = 0 ;
 edgePnt1 = dtmP->nullPnt ;
 edgePnt2 = dtmP->nullPnt ;

// Scan Pond Boundary For External Zero Edges

 sp = pondStartPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    if( pointAddrP(dtmP,sp)->z == lowPointZ && pointAddrP(dtmP,np)->z == lowPointZ )
      {
       if( ! bcdtmList_testForHullLineDtmObject(dtmP,sp,np) )
         {
          if( ( cp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
          if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
          if( pointAddrP(dtmP,cp)->z == lowPointZ && pointAddrP(dtmP,ap)->z != lowPointZ )
            {
             zeroEdgeType = 1 ;
             edgePnt1 = sp ;
             edgePnt2 = np ;
            }
         }
      }
    sp = np ;
   } while ( sp != pondStartPoint && edgePnt1 == dtmP->nullPnt ) ;

//  Scan Pond Boundary For Internal Zero Edges

 if( edgePnt1 == dtmP->nullPnt )
   {
    sp = pondStartPoint ;
    do
      {
       np = nodeAddrP(dtmP,sp)->tPtr ;
       if( pointAddrP(dtmP,sp)->z == lowPointZ && pointAddrP(dtmP,np)->z == lowPointZ )
         {
          if( ! bcdtmList_testForHullLineDtmObject(dtmP,sp,np) )
            {
             if( ( cp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
             if( pointAddrP(dtmP,sp)->z == lowPointZ && pointAddrP(dtmP,np)->z == lowPointZ && pointAddrP(dtmP,cp)->z == lowPointZ  )
               {
                cp = np ;
                if( ( tp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
                if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,tp)) < 0 ) goto errexit ;
                while( nodeAddrP(dtmP,tp)->tPtr != sp && nodeAddrP(dtmP,ap)->tPtr != sp && edgePnt1 == dtmP->nullPnt )
                  {
                   if( ( pointAddrP(dtmP,cp)->z == lowPointZ && pointAddrP(dtmP,tp)->z == lowPointZ && pointAddrP(dtmP,ap)->z != lowPointZ ) ||
                       ( pointAddrP(dtmP,cp)->z != lowPointZ && pointAddrP(dtmP,tp)->z == lowPointZ && pointAddrP(dtmP,ap)->z == lowPointZ )    )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"Zero Edge ** cp->z = %10.4lf tp->z = %10.4lf ap->z = %10.4lf",pointAddrP(dtmP,cp)->z,pointAddrP(dtmP,tp)->z,pointAddrP(dtmP,ap)->z ) ;
                      zeroEdgeType = 2 ;
                      edgePnt1 = sp ;
                      edgePnt2 = tp ;
                     }
                   cp = tp ;
                   tp = ap ;
                   if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,tp)) < 0 ) goto errexit ;
                  }
               }
            }
         }
       sp = np ;
      } while ( sp != pondStartPoint && edgePnt1 == dtmP->nullPnt ) ;
   }

// Log Zero Edge

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"zeroEdgeType = %2d ** edgePnt1 = %10d edgePnt2 = %10d",zeroEdgeType,edgePnt1,edgePnt2,edgePnt2) ;
    if( ( ap = bcdtmList_nextAntDtmObject(dtmP,edgePnt1,edgePnt2)) < 0 ) goto errexit ;
    if( ( cp = bcdtmList_nextClkDtmObject(dtmP,edgePnt1,edgePnt2)) < 0 ) goto errexit ;
    bcdtmWrite_message(0,0,0,"edgePnt1[%8ld] ** %12.5lf %12.5lf %10.4lf",edgePnt1,pointAddrP(dtmP,edgePnt1)->x,pointAddrP(dtmP,edgePnt1)->y,pointAddrP(dtmP,edgePnt1)->z) ;
    bcdtmWrite_message(0,0,0,"edgePnt2[%8ld] ** %12.5lf %12.5lf %10.4lf",edgePnt2,pointAddrP(dtmP,edgePnt2)->x,pointAddrP(dtmP,edgePnt2)->y,pointAddrP(dtmP,edgePnt2)->z) ;
    bcdtmWrite_message(0,0,0,"clkPnt  [%8ld] ** %12.5lf %12.5lf %10.4lf",cp,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y,pointAddrP(dtmP,cp)->z) ;
    bcdtmWrite_message(0,0,0,"antPnt  [%8ld] ** %12.5lf %12.5lf %10.4lf",ap,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,ap)->z) ;
   }

// Clean Up

 cleanup :

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Pond Boundary For A Zero Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Pond Boundary For A Zero Edge Error") ;
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
int bcdtmDrainage_expandPondToOuterEdgeOfZeroSlopeTrianglesDtmObjectOld
(
BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
long              *pondStartPointP,           // <=> Start Point On Tptr Polygon
double            lowPointZ                   // ==> Low Point Z
)
    {
    int    ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0), tdbg = DTM_TIME_VALUE (0), cdbg = DTM_CHECK_VALUE (0);
    int    numConnectedPoints, numLowPointElevation, newStartPoint, numZeroEdges;
    DTMDirection direction;
    long   sp, np, cp, clc, startPnt, firstPnt, lastPnt, stopPnt, numMarked, totalMarked;
    long   p1, p2, p3, numFeaturePts, startTime = bcdtmClock ();
    DPoint3d    *featurePtsP = nullptr;
    bool   outerEdgeFound = false;
    double areaTptr, areaSptr;
    DTMDirection directionTptr;
    DTM_CIR_LIST *clistP;
    BC_DTM_OBJ *dataP = nullptr;

    // Log Function Arguements

    if (dbg == 1)
        {
        bcdtmWrite_message (0, 0, 0, "Expanding Pond To Outer Edge Of Zero Slope Triangles");
        bcdtmWrite_message (0, 0, 0, "dtmP            = %p", dtmP);
        bcdtmWrite_message (0, 0, 0, "pondStartPointP = %8ld", pondStartPointP);
        bcdtmWrite_message (0, 0, 0, "lowPointZ       = %8.3lf", lowPointZ);
        }

    // Initialise

    startPnt = *pondStartPointP;
    firstPnt = dtmP->nullPnt;
    lastPnt = dtmP->nullPnt;
    if (dbg)
        {
        if (bcdtmObject_createDtmObject (&dataP)) goto errexit;
        if (bcdtmList_copyTptrListToPointArrayDtmObject (dtmP, *pondStartPointP, &featurePtsP, &numFeaturePts)) goto errexit;
        if (bcdtmObject_storeDtmFeatureInDtmObject (dataP, DTMFeatureType::Breakline, dtmP->nullUserTag, 1, &dtmP->nullFeatureId, featurePtsP, numFeaturePts)) goto errexit;
        }

    // Check Inner Edge Points Of The Pond Have The Same Elevation Value

    if (cdbg)
        {
        sp = startPnt;
        do
            {
            if (pointAddrP (dtmP, sp)->z != lowPointZ)
                {
                bcdtmWrite_message (2, 0, 0, "Inner Edge Has Incorrect Elevation Values");
                goto errexit;
                }
            sp = nodeAddrP (dtmP, sp)->tPtr;
            } while (sp != startPnt);
        }

    // Calculate Area Of Tptr Polygon

    if (bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject (dtmP, *pondStartPointP, &areaTptr, &directionTptr)) goto errexit;

    // Mark Points Immediately External To Tptr Polygon

    if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Points Immediately External To Tptr Polygon");
    numMarked = 0;
    sp = startPnt;
    do
        {
        np = nodeAddrP (dtmP, sp)->tPtr;
        p1 = np;
        if ((p2 = bcdtmList_nextAntDtmObject (dtmP, np, sp)) < 0) goto errexit;
        while (nodeAddrP (dtmP, np)->tPtr != p2)
            {
            if ((p3 = bcdtmList_nextAntDtmObject (dtmP, np, p2)) < 0) goto errexit;
            if (nodeAddrP (dtmP, p2)->tPtr == dtmP->nullPnt && pointAddrP (dtmP, p2)->z == lowPointZ && (pointAddrP (dtmP, p1)->z == lowPointZ || pointAddrP (dtmP, p3)->z == lowPointZ))
                {
                ++numMarked;
                if (lastPnt == dtmP->nullPnt)
                    {
                    firstPnt = lastPnt = p2;
                    }
                else
                    {
                    nodeAddrP (dtmP, lastPnt)->tPtr = -(p2 + 1);
                    lastPnt = p2;
                    }
                nodeAddrP (dtmP, lastPnt)->tPtr = -(lastPnt + 1);
                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "1-Marked Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld", p2, pointAddrP (dtmP, p2)->x, pointAddrP (dtmP, p2)->y, pointAddrP (dtmP, p2)->z, sp, firstPnt, lastPnt);
                }
            p1 = p2;
            p2 = p3;
            }
        sp = np;
        } while (sp != startPnt);

    // Log Number marked

    totalMarked = numMarked;
    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Marked Immediately External To Tptr Polygon = %8ld", numMarked);

    //  Mark Points Connected To Points

    if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Points Connected To Marked Points");
    numMarked = 0;
    if ((sp = firstPnt) != dtmP->nullPnt)
        {
        do
            {

            stopPnt = sp;

            //     Check For Outer Edge At Marked Point

            if (bcdtmDrainage_countNumberOfZeroSlopeEdgesAtPointDtmObject (dtmP, sp, lowPointZ, numZeroEdges)) goto errexit;
            if (numZeroEdges == 1)    //  Outer Edge Found
                {
                if (bcdtmDrainage_placeSptrPolygonAtPointAroundZeroSlopeTrianglesDtmObject (dtmP, sp, lowPointZ, areaSptr, direction, newStartPoint)) goto errexit;
                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "areaTptr = %15.5lf sPtr Area = %15.5lf", areaTptr, areaSptr);
                if (newStartPoint != dtmP->nullPnt)
                    {
                    if (direction == DTMDirection::Clockwise && areaSptr > areaTptr)            // Clockwise
                        {
                        if (dbg)
                            {
                            if (bcdtmList_copySptrListToPointArrayDtmObject (dtmP, newStartPoint, &featurePtsP, &numFeaturePts)) goto errexit;
                            if (bcdtmObject_storeDtmFeatureInDtmObject (dataP, DTMFeatureType::ContourLine, dtmP->nullUserTag, 1, &dtmP->nullFeatureId, featurePtsP, numFeaturePts)) goto errexit;
                            }
                        outerEdgeFound = true;
                        }
                    else                             //  Anti Clockwise
                        {
                        bcdtmList_nullSptrListDtmObject (dtmP, newStartPoint);
                        }
                    }
                }
            else                                   //  Mark Connected Points
                {
                clc = nodeAddrP (dtmP, sp)->cPtr;
                numConnectedPoints = 0;
                numLowPointElevation = 0;
                if (clc != dtmP->nullPtr)
                    {
                    clistP = clistAddrP (dtmP, clc);
                    p2 = clistP->pntNum;
                    if ((p1 = bcdtmList_nextAntDtmObject (dtmP, sp, p2)) < 0) goto errexit;
                    clc = clistP->nextPtr;
                    while (clc != dtmP->nullPtr)
                        {
                        clistP = clistAddrP (dtmP, clc);
                        cp = p3 = clistP->pntNum;
                        clc = clistP->nextPtr;
                        if (nodeAddrP (dtmP, p2)->tPtr == dtmP->nullPnt && pointAddrP (dtmP, p2)->z == lowPointZ && (pointAddrP (dtmP, p1)->z == lowPointZ || pointAddrP (dtmP, p3)->z == lowPointZ))
                            {
                            ++numMarked;
                            nodeAddrP (dtmP, lastPnt)->tPtr = -(p2 + 1);
                            lastPnt = p2;
                            nodeAddrP (dtmP, lastPnt)->tPtr = -(lastPnt + 1);
                            if (dbg == 2) bcdtmWrite_message (0, 0, 0, "2-Marked Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld", p2, pointAddrP (dtmP, p2)->x, pointAddrP (dtmP, p2)->y, pointAddrP (dtmP, p2)->z, sp, firstPnt, lastPnt);
                            }
                        p1 = p2;
                        p2 = p3;
                        }
                    }
                }

            //     Get Next Marked Point

            sp = -(nodeAddrP (dtmP, sp)->tPtr + 1);
            } while (sp != stopPnt && outerEdgeFound == false);
        }

    // Log Number marked

    if (dbg)
        {
        totalMarked = totalMarked + numMarked;
        bcdtmWrite_message (0, 0, 0, "Number Marked Connected To Marked Points = %8ld", numMarked);
        bcdtmWrite_message (0, 0, 0, "Total Number Marked                      = %8ld", totalMarked);
        }

    // Unmark Marked Points

    if (firstPnt != dtmP->nullPtr)
        {
        sp = firstPnt;
        do
            {
            stopPnt = sp;
            np = -(nodeAddrP (dtmP, sp)->tPtr + 1);
            if (numMarked == 0)
                nodeAddrP (dtmP, sp)->tPtr = np;
            else
                nodeAddrP (dtmP, sp)->tPtr = dtmP->nullPnt;
            sp = np;
            } while (sp != stopPnt);
        if (numMarked == 0)
            nodeAddrP (dtmP, sp)->tPtr = firstPnt;
        }
    else
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "No Point Marked to UnMark.");
        }

    if (numMarked == 0)
        {
        newStartPoint = firstPnt;
        if (bcdtmList_nullTptrListDtmObject (dtmP, *pondStartPointP)) goto errexit;
        if (bcdtmList_reverseTptrPolygonDtmObject (dtmP, newStartPoint)) goto errexit;
        *pondStartPointP = newStartPoint;
        }
    // Set New Outer Edge Tptr Polygon

    else if (outerEdgeFound == true)
        {
        if (bcdtmList_nullTptrListDtmObject (dtmP, *pondStartPointP)) goto errexit;
        if (bcdtmList_copySptrListToTptrListDtmObject (dtmP, newStartPoint)) goto errexit;
        if (bcdtmList_reverseTptrPolygonDtmObject (dtmP, newStartPoint)) goto errexit;
        if (bcdtmList_nullSptrListDtmObject (dtmP, newStartPoint)) goto errexit;
        *pondStartPointP = newStartPoint;
        }

    // Save Boundaries To File

    if (dbg)
        {
        bcdtmWrite_geopakDatFileFromDtmObject (dataP, L"outerBoundaries.dat");
        }

    // Clean Up

cleanup:
    if (featurePtsP != nullptr)
        {
        free (featurePtsP); featurePtsP = nullptr;
        }
    if (dataP != nullptr) bcdtmObject_destroyDtmObject (&dataP);

    // Exit

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Expanding Pond To Outer Edge Of Zero Slope Triangles Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Expanding Pond To Outer Edge Of Zero Slope Triangles Error");
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
DTMStatusInt bcdtmDrainage_placeSPtrAroundNonNullTPtr (BC_DTM_OBJ* dtmP, long startPnt, int& newStartPt)
    {
    int    cdbg = DTM_CHECK_VALUE (0);

    if (nodeAddrP (dtmP, startPnt)->tPtr == dtmP->nullPnt)
        return DTM_ERROR;

    // search round the start pnt till we find a non tPtr point.
    long np = -1;
    while (true)
        {
        long lowestPnt = dtmP->numPoints;
        long clc = nodeAddrP (dtmP, startPnt)->cPtr;
        DTM_CIR_LIST* clistP = clistAddrP (dtmP, clc);
        np = clistP->pntNum;
        while (nodeAddrP (dtmP, np)->tPtr != dtmP->nullPnt || nodeAddrP (dtmP,np)->hPtr != dtmP->nullPnt)
            {
            if (np < lowestPnt)
                lowestPnt = np;
            clc = clistP->nextPtr;
            if (clc == dtmP->nullPtr)
                break;
            clistP = clistAddrP (dtmP, clc);
            np = clistP->pntNum;
            }
        if (clc != dtmP->nullPtr)
            break;
        }

    // Now as we have found a the first link as the edge.
    long p = startPnt;
    do
        {
        long p1 = bcdtmList_nextAntDtmObject (dtmP, p, np);
        while (p1 != np && nodeAddrP (dtmP, p1)->tPtr == dtmP->nullPnt && nodeAddrP (dtmP, p1)->hPtr == dtmP->nullPnt)
            {
            p1 = bcdtmList_nextAntDtmObject (dtmP, p, p1);
            }
        if (p1 == np)
            return DTM_ERROR;
        nodeAddrP (dtmP, p)->sPtr = p1;
        np = p;
        p = p1;
        } while (p != startPnt);
    if (cdbg)
        {
        if (bcdtmList_checkConnectivitySptrPolygonDtmObject (dtmP, startPnt, 1))
            {
            bcdtmWrite_message (2, 0, 0, "Sptr Polygon Connectivity Error");
            return DTM_ERROR;
            }
        double area;
        DTMDirection direction;
        if (bcdtmMath_calculateAreaAndDirectionSptrPolygonDtmObject (dtmP, startPnt, &area, &direction))
            return DTM_ERROR;
        if (direction != DTMDirection::AntiClockwise)
            return DTM_ERROR;
        }
    newStartPt = startPnt;
    return DTM_SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_expandPondToOuterEdgeOfZeroSlopeTrianglesDtmObject
    (
    BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
    long              *pondStartPointP,           // <=> Start Point On Tptr Polygon
    double            lowPointZ                   // ==> Low Point Z
    )
    {
    int    ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0), tdbg = DTM_TIME_VALUE (0), cdbg = DTM_CHECK_VALUE (0);
    int    numConnectedPoints, numLowPointElevation, numZeroEdges, newStartPoint;
    DTMDirection direction;
    long   sp, np, cp, clc, startPnt, firstPnt, lastPnt, stopPnt, numMarked, totalMarked;
    long   p1, p2, p3, numFeaturePts, startTime = bcdtmClock ();
    DPoint3d    *featurePtsP = nullptr;
    bool   outerEdgeFound = false;
    double areaTptr, areaSptr;
    DTMDirection directionTptr;
    DTM_CIR_LIST *clistP;
    BC_DTM_OBJ *dataP = nullptr;

    // Log Function Arguements

    if (dbg == 1)
        {
        bcdtmWrite_message (0, 0, 0, "Expanding Pond To Outer Edge Of Zero Slope Triangles");
        bcdtmWrite_message (0, 0, 0, "dtmP            = %p", dtmP);
        bcdtmWrite_message (0, 0, 0, "pondStartPointP = %8ld", pondStartPointP);
        bcdtmWrite_message (0, 0, 0, "lowPointZ       = %8.3lf", lowPointZ);
        }

    // Initialise

    startPnt = *pondStartPointP;
    firstPnt = dtmP->nullPnt;
    lastPnt = dtmP->nullPnt;
    if (dbg)
        {
        if (bcdtmObject_createDtmObject (&dataP)) goto errexit;
        if (bcdtmList_copyTptrListToPointArrayDtmObject (dtmP, *pondStartPointP, &featurePtsP, &numFeaturePts)) goto errexit;
        if (bcdtmObject_storeDtmFeatureInDtmObject (dataP, DTMFeatureType::Breakline, dtmP->nullUserTag, 1, &dtmP->nullFeatureId, featurePtsP, numFeaturePts)) goto errexit;
        }

    // Check Inner Edge Points Of The Pond Have The Same Elevation Value

    if (cdbg)
        {
        sp = startPnt;
        do
            {
            if (pointAddrP (dtmP, sp)->z != lowPointZ)
                {
                bcdtmWrite_message (2, 0, 0, "Inner Edge Has Incorrect Elevation Values");
                goto errexit;
                }
            sp = nodeAddrP (dtmP, sp)->tPtr;
            } while (sp != startPnt);
        }

    // Calculate Area Of Tptr Polygon

    if (bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject (dtmP, *pondStartPointP, &areaTptr, &directionTptr)) goto errexit;

    // Mark Points Immediately External To Tptr Polygon

    if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Points Immediately External To Tptr Polygon");
    numMarked = 0;
    sp = startPnt;
    do
        {
        np = nodeAddrP (dtmP, sp)->tPtr;
        p1 = np;
        if ((p2 = bcdtmList_nextAntDtmObject (dtmP, np, sp)) < 0) goto errexit;
        while (nodeAddrP (dtmP, np)->tPtr != p2)
            {
            if ((p3 = bcdtmList_nextAntDtmObject (dtmP, np, p2)) < 0) goto errexit;
            if (nodeAddrP (dtmP, p2)->tPtr == dtmP->nullPnt && pointAddrP (dtmP, p2)->z == lowPointZ && (pointAddrP (dtmP, p1)->z == lowPointZ || pointAddrP (dtmP, p3)->z == lowPointZ))
                {
                ++numMarked;
                if (lastPnt == dtmP->nullPnt)
                    {
                    firstPnt = lastPnt = p2;
                    }
                else
                    {
                    nodeAddrP (dtmP, lastPnt)->tPtr = -(p2 + 1);
                    lastPnt = p2;
                    }
                nodeAddrP (dtmP, lastPnt)->tPtr = -(lastPnt + 1);
                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "1-Marked Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld", p2, pointAddrP (dtmP, p2)->x, pointAddrP (dtmP, p2)->y, pointAddrP (dtmP, p2)->z, sp, firstPnt, lastPnt);
                }
            p1 = p2;
            p2 = p3;
            }
        sp = np;
        } while (sp != startPnt);

    // Log Number marked

    totalMarked = numMarked;
    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Marked Immediately External To Tptr Polygon = %8ld", numMarked);

    //  Mark Points Connected To Points

    if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Points Connected To Marked Points");
    numMarked = 0;
    if ((sp = firstPnt) != dtmP->nullPnt)
        {
        do
            {

            stopPnt = sp;

            //     Check For Outer Edge At Marked Point

            if (bcdtmDrainage_countNumberOfZeroSlopeEdgesAtPointDtmObject (dtmP, sp, lowPointZ, numZeroEdges)) goto errexit;
            if (numZeroEdges == 1)    //  Outer Edge Found
                {
                if (bcdtmDrainage_placeSptrPolygonAtPointAroundZeroSlopeTrianglesDtmObject (dtmP, sp, lowPointZ, areaSptr, direction, newStartPoint)) goto errexit;
                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "areaTptr = %15.5lf sPtr Area = %15.5lf", areaTptr, areaSptr);
                if (newStartPoint != dtmP->nullPnt)
                    {
                    if (direction == DTMDirection::Clockwise && areaSptr > areaTptr)            // Clockwise
                        {
                        if (dbg)
                            {
                            if (bcdtmList_copySptrListToPointArrayDtmObject (dtmP, newStartPoint, &featurePtsP, &numFeaturePts)) goto errexit;
                            if (bcdtmObject_storeDtmFeatureInDtmObject (dataP, DTMFeatureType::ContourLine, dtmP->nullUserTag, 1, &dtmP->nullFeatureId, featurePtsP, numFeaturePts)) goto errexit;
                            }
                        outerEdgeFound = true;
                        }
                    else                             //  Anti Clockwise
                        {
                        bcdtmList_nullSptrListDtmObject (dtmP, newStartPoint);
                        }
                    }
                }
            else                                   //  Mark Connected Points
                {
                clc = nodeAddrP (dtmP, sp)->cPtr;
                numConnectedPoints = 0;
                numLowPointElevation = 0;
                if (clc != dtmP->nullPtr)
                    {
                    clistP = clistAddrP (dtmP, clc);
                    p2 = clistP->pntNum;
                    if ((p1 = bcdtmList_nextAntDtmObject (dtmP, sp, p2)) < 0) goto errexit;
                    clc = clistP->nextPtr;
                    while (clc != dtmP->nullPtr)
                        {
                        clistP = clistAddrP (dtmP, clc);
                        cp = p3 = clistP->pntNum;
                        clc = clistP->nextPtr;
                        if (nodeAddrP (dtmP, p2)->tPtr == dtmP->nullPnt && pointAddrP (dtmP, p2)->z == lowPointZ && (pointAddrP (dtmP, p1)->z == lowPointZ || pointAddrP (dtmP, p3)->z == lowPointZ))
                            {
                            ++numMarked;
                            nodeAddrP (dtmP, lastPnt)->tPtr = -(p2 + 1);
                            lastPnt = p2;
                            nodeAddrP (dtmP, lastPnt)->tPtr = -(lastPnt + 1);
                            if (dbg == 2) bcdtmWrite_message (0, 0, 0, "2-Marked Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld", p2, pointAddrP (dtmP, p2)->x, pointAddrP (dtmP, p2)->y, pointAddrP (dtmP, p2)->z, sp, firstPnt, lastPnt);
                            }
                        p1 = p2;
                        p2 = p3;
                        }
                    }
                }

            //     Get Next Marked Point

            sp = -(nodeAddrP (dtmP, sp)->tPtr + 1);
            } while (sp != stopPnt && outerEdgeFound == false);
        }

    // Log Number marked

    if (dbg)
        {
        totalMarked = totalMarked + numMarked;
        bcdtmWrite_message (0, 0, 0, "Number Marked Connected To Marked Points = %8ld", numMarked);
        bcdtmWrite_message (0, 0, 0, "Total Number Marked                      = %8ld", totalMarked);
        }

    if (!outerEdgeFound)
        {
        if (bcdtmDrainage_placeSPtrAroundNonNullTPtr (dtmP, lastPnt, newStartPoint)) goto errexit;
        }
    // Unmark Marked Points
    if (firstPnt != dtmP->nullPtr)
        {
        sp = firstPnt;
        do
            {
            stopPnt = sp;
            np = -(nodeAddrP (dtmP, sp)->tPtr + 1);
            nodeAddrP (dtmP, sp)->tPtr = dtmP->nullPnt;
            sp = np;
            } while (sp != stopPnt);
        }
    else
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "No Point Marked to UnMark.");
        }

    // Set New Outer Edge Tptr Polygon

    if (bcdtmList_nullTptrListDtmObject (dtmP, *pondStartPointP)) goto errexit;
    if (bcdtmList_copySptrListToTptrListDtmObject (dtmP, newStartPoint)) goto errexit;
    if (outerEdgeFound == true)
        if (bcdtmList_reverseTptrPolygonDtmObject (dtmP, newStartPoint)) goto errexit;
    if (bcdtmList_nullSptrListDtmObject (dtmP, newStartPoint)) goto errexit;
    *pondStartPointP = newStartPoint;

    // Save Boundaries To File

    if (dbg)
        {
        bcdtmWrite_geopakDatFileFromDtmObject (dataP, L"outerBoundaries.dat");
        }

    // Clean Up

cleanup:
    if (featurePtsP != nullptr)
        {
        free (featurePtsP); featurePtsP = nullptr;
        }
    if (dataP != nullptr) bcdtmObject_destroyDtmObject (&dataP);

    // Exit

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Expanding Pond To Outer Edge Of Zero Slope Triangles Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Expanding Pond To Outer Edge Of Zero Slope Triangles Error");
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
int bcdtmDrainage_placeSptrPolygonAtPointAroundZeroSlopeTrianglesDtmObject
(
 BC_DTM_OBJ *dtmP,                                 // ==> Pointer To DTM Object
 int        point,                                 // ==> Point To Start Placement Of Polygon
 double     lowPointZ,                             // ==> Elevation Of Zero Slope Triangles
 double&    sPtrArea,                              // <== Sptr Polygon Area
 DTMDirection&       sPtrDirection,                         // <== Sptr Polygon Direction
 int&       startPoint                             // <== Sptr Polygon Start Point
)
//
// This Function Places A Sptr Polygon Around Zero Slope Triangles.
// It Starts From A Zero Edge With The Low Point Elevation From Point.
// There Should Only Be One Zero Edge In A clockwise Sense
//
{
 int          ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 int          sp,np,cp,lcp,clc ;
 long         pnt,hullLine,edgePnt1,edgePnt2,antPnt,clkPnt ;
 DTMDirection direction ;
 double       elevation,area ;
 bool         zeroEdge=false ;

//  Log Function Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Placing Sptr Polygon At Point Around Zero Slope Triangles") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"point          = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"lowPointZ      = %8.3lf",lowPointZ) ;
    bcdtmWrite_message(0,0,0,"sPtrDirection  = %8ld",sPtrDirection ) ;
    bcdtmWrite_message(0,0,0,"startPoint     = %8ld",startPoint ) ;
   }

//  Initialise Variables

 startPoint = dtmP->nullPnt ;
 elevation  = pointAddrP(dtmP,point)->z ;
 sPtrArea   = 0.0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"point = %8ld ** %12.5lf %12.5lf %10.4lf",point,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z) ;
// bcdtmList_writeCircularListForPointDtmObject(dtmP,point) ;

//  Scan Point For Zero Edge

 edgePnt1 = edgePnt2 = dtmP->nullPnt ;
 if( ( clc = nodeAddrP(dtmP,point)->cPtr ) != dtmP->nullPtr )
   {
    if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while( clc != dtmP->nullPtr && edgePnt1 == dtmP->nullPnt )
      {
       pnt = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
//bcdtmWrite_message(0,0,0,"lowPoint->z = %10.4lf pnt->z = %10.4lf antPnt-Z = %10.4lf",lowPointZ,pointAddrP(dtmP,pnt)->z,pointAddrP(dtmP,antPnt)->z) ;
       if( nodeAddrP(dtmP,pnt)->hPtr != point && pointAddrP(dtmP,pnt)->z == lowPointZ && pointAddrP(dtmP,antPnt)->z != lowPointZ )
         {
          if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
//          if( pointAddrP(dtmP,clkPnt)->z == lowPointZ && nodeAddrP(dtmP,clkPnt)->tPtr != dtmP->nullPnt )
          if( pointAddrP(dtmP,clkPnt)->z == lowPointZ )
            {
             edgePnt1 = point ;
             edgePnt2 = pnt ;
             zeroEdge = true ;
            }
         }
       antPnt = pnt ;
      }
   }


//  Log Variables

 if( dbg && zeroEdge )
   {
    bcdtmWrite_message(0,0,0,"edgePnt1 = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",edgePnt1,nodeAddrP(dtmP,edgePnt1)->hPtr,pointAddrP(dtmP,edgePnt1)->x,pointAddrP(dtmP,edgePnt1)->y,pointAddrP(dtmP,edgePnt1)->z) ;
    bcdtmWrite_message(0,0,0,"edgePnt2 = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",edgePnt2,nodeAddrP(dtmP,edgePnt2)->hPtr,pointAddrP(dtmP,edgePnt2)->x,pointAddrP(dtmP,edgePnt2)->y,pointAddrP(dtmP,edgePnt2)->z) ;
    bcdtmWrite_message(0,0,0,"clkPnt   = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",clkPnt,nodeAddrP(dtmP,clkPnt)->hPtr,pointAddrP(dtmP,clkPnt)->x,pointAddrP(dtmP,clkPnt)->y,pointAddrP(dtmP,clkPnt)->z) ;
    bcdtmWrite_message(0,0,0,"antPnt   = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",antPnt,nodeAddrP(dtmP,antPnt)->hPtr,pointAddrP(dtmP,antPnt)->x,pointAddrP(dtmP,antPnt)->y,pointAddrP(dtmP,antPnt)->z) ;
   }

// Zero Edge Detected

 if( zeroEdge )
   {

//  Extract Zero Slope Polygon

    sp = edgePnt1 ;
    np = edgePnt2 ;
    do
      {
       if( nodeAddrP(dtmP,sp)->sPtr != dtmP->nullPnt )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Knot At Point %8ld",sp) ;
          cp = sp ;
          do
            {
             lcp = nodeAddrP(dtmP,cp)->sPtr ;
             nodeAddrP(dtmP,cp)->sPtr = dtmP->nullPnt ;
             cp  = lcp ;
            } while( cp != sp ) ;
         }
       nodeAddrP(dtmP,sp)->sPtr = np ;

       lcp = cp = sp ;
       if( ( cp = bcdtmList_nextAntDtmObject(dtmP,np,cp)) < 0 ) goto errexit ;
       if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,np,cp,&hullLine)) goto errexit ;
       while( ! hullLine && pointAddrP(dtmP,cp)->z == pointAddrP(dtmP,sp)->z  )
         {
          lcp = cp ;
          if( ( cp = bcdtmList_nextAntDtmObject(dtmP,np,cp)) < 0 ) goto errexit ;
          if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,np,cp,&hullLine)) goto errexit ;
         }
       if( pointAddrP(dtmP,cp)->z != pointAddrP(dtmP,sp)->z  ) cp = lcp ;
       sp = np ;
       np = cp ;
     } while ( sp != edgePnt1 ) ;

//  Check Connectivity Sptr Polygon

    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of Sptr Polygon ** startPoint = %8ld",pnt) ;
    if( bcdtmList_checkConnectivitySptrPolygonDtmObject(dtmP,edgePnt1,0))
      {
       bcdtmWrite_message(2,0,0,"Connectivity Error In Sptr Polygon") ;
       goto errexit ;
      }

//  Get Direction Of Sptr Polygon

    bcdtmMath_calculateAreaAndDirectionSptrPolygonDtmObject(dtmP,edgePnt1,&area,&direction) ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"sPtr Polygon ** area = %20.10lf direction = %2ld",area,direction) ;
       bcdtmWrite_message(0,0,0,"Internal Polygon Boundary Found") ;
      }

//  Set Start Point

    startPoint    = edgePnt1 ;
    sPtrArea      = area ;
    sPtrDirection = direction ;
   }

//  Processing Completed

 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Placing Sptr Polygon Around Zero Slope Triangles Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Placing Sptr Polygon Around Zero Slope Triangles Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_countNumberOfZeroSlopeEdgesAtPointDtmObject
(
 BC_DTM_OBJ *dtmP,                                 // ==> Pointer To DTM Object
 int        point,                                 // ==> Point To Test For Number Of Zero Slope Edges
 double     lowPointZ,                             // ==> Elevation Of Zero Slope Triangles
 int&       numZeroEdges                           // <== Number Of Zero Slope Edges
)
//
// This Function Counts The Number Of Zero Slope Edges At The Low Point Elevation For A Point
// The Edges Are Counted In A Clockwise Direction. The Edge And Clockwise Point Must Have The
// Same Elevation equal to lowPointZ. The Counter Clockwise Point Must Have A Different Elevation
//
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 int    clc,pnt,antPnt,clkPnt ;

//  Log Function Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Counting Number Of Zero Edges At Point") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"point          = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"lowPointZ      = %8.3lf",lowPointZ) ;
    bcdtmWrite_message(0,0,0,"numZeroEdges   = %8ld",numZeroEdges) ;
   }

//  Initialise

 numZeroEdges = 0 ;

//  Log Circular List For Point

 if( dbg == 2 ) bcdtmList_writeCircularListForPointDtmObject(dtmP,point) ;

//  Scan Point For Zero Edge

 if( ( clc = nodeAddrP(dtmP,point)->cPtr ) != dtmP->nullPtr )
   {
    if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while( clc != dtmP->nullPtr )
      {
       pnt = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( nodeAddrP(dtmP,pnt)->hPtr != point && nodeAddrP(dtmP,pnt)->tPtr == dtmP->nullPnt )    //  Test For Edge Not Coincident With The Tin Hull
         {
          if( pointAddrP(dtmP,pnt)->z == lowPointZ && pointAddrP(dtmP,antPnt)->z != lowPointZ )
            {
             if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt)) < 0 ) goto errexit ;
             if( dbg == 2 )
               {
                bcdtmWrite_message(0,0,0,"antPnt   = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",antPnt,nodeAddrP(dtmP,antPnt)->hPtr,pointAddrP(dtmP,antPnt)->x,pointAddrP(dtmP,antPnt)->y,pointAddrP(dtmP,antPnt)->z) ;
                bcdtmWrite_message(0,0,0,"pnt      = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",pnt,nodeAddrP(dtmP,pnt)->hPtr,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
                bcdtmWrite_message(0,0,0,"clkPnt   = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",clkPnt,nodeAddrP(dtmP,clkPnt)->hPtr,pointAddrP(dtmP,clkPnt)->x,pointAddrP(dtmP,clkPnt)->y,pointAddrP(dtmP,clkPnt)->z) ;
               }
             if( pointAddrP(dtmP,clkPnt)->z == lowPointZ )
               {
                ++numZeroEdges ;
               }
            }
         }
       antPnt = pnt ;
      }
   }

// Log Number Of Zero Edges

 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Zero Edges = %8d",numZeroEdges) ;

// Processing Completed

 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Counting Number Of Zero Edges At Point Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Counting Number Of Zero Edges At Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_getUnionOfPolygonsDtmObject
(
 BC_DTM_OBJ *dtmP,                                 // ==> Pointer To DTM Object
 int        tPtrStartPoint,                        // ==> Start Point For Tptr Polygon
 int        sPtrStartPoint,                        // ==> Start Point For Sptr Polygon
 long&      startPoint                             // <== Tptr Polygon Start Point
)
//
// This Function Places A Tptr Polygon Around The Union
// Of The Sptr And Tptr Polygons. The initial Polygons are nulled
//
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   sp,spnt,npnt,apnt,cpnt,leftMostPoint,nextLeftMostPoint ;
 bool   unionFound=false ;
 double area ;
 DTMDirection direction ;
 bvector < int > points ;
 bvector < int >::iterator ipnt ;
 DTM_TIN_POINT *pnt1P,*pnt2P ;

//  Log Function Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Union Of Polgons") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP ) ;
    bcdtmWrite_message(0,0,0,"tPtrStartPoint = %8ld",tPtrStartPoint ) ;
    bcdtmWrite_message(0,0,0,"sPtrStartPoint = %8ld",sPtrStartPoint ) ;
    bcdtmWrite_message(0,0,0,"startPoint     = %8ld",startPoint ) ;
   }

//  Initialise Variables

 startPoint = dtmP->nullPnt ;

//  Check Connectivity Of Polygons - Development Only

 if( cdbg )
   {
    if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,tPtrStartPoint,0))
      {
       bcdtmWrite_message(2,0,0,"Tptr Polygon Connectivity Error") ;
       goto errexit ;
      }
    if( bcdtmList_checkConnectivitySptrPolygonDtmObject(dtmP,sPtrStartPoint,1))
      {
       bcdtmWrite_message(2,0,0,"Sptr Polygon Connectivity Error") ;
       goto errexit ;
      }
    bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,tPtrStartPoint,&area,&direction) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Polygon ** Area = %15.5lf Direction = %2ld",area,direction) ;
    if (direction != DTMDirection::AntiClockwise)
      {
       bcdtmWrite_message(2,0,0,"Tptr Polygon Direction Is Clockwise") ;
       goto errexit ;
      }
    bcdtmMath_calculateAreaAndDirectionSptrPolygonDtmObject(dtmP,sPtrStartPoint,&area,&direction) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Sptr Polygon ** Area = %15.5lf Direction = %2ld",area,direction) ;
    if (direction != DTMDirection::Clockwise)
      {
       bcdtmWrite_message(2,0,0,"Sptr Polygon Direction Is Counter Clockwise") ;
       goto errexit ;
      }
   }

//  Reverse Direction Of Sptr Polygon

 if( bcdtmList_reverseSptrPolygonDtmObject(dtmP,sPtrStartPoint)) goto errexit ;

//  Log Before Tptr And Sptr Polygons Before Union

 if( dbg == 1 )
   {
    long numFeaturePts ;
    DPoint3d  *featurePtsP=nullptr ;
    BC_DTM_OBJ *dataP=nullptr ;
    if( dataP == nullptr ) if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
    if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,tPtrStartPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,2,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
    if( bcdtmList_copySptrListToPointArrayDtmObject(dtmP,sPtrStartPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::ContourLine,2,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
    if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"unionPolygons.dat")) goto errexit ;
    if( bcdtmObject_destroyDtmObject(&dataP)) goto errexit ;
    if( featurePtsP != nullptr ) free(featurePtsP) ;
   }

//  Log Polygons

 if( dbg == 1 )
   {
    bcdtmList_writeTptrListDtmObject(dtmP,tPtrStartPoint) ;
    bcdtmList_writeSptrListDtmObject(dtmP,sPtrStartPoint) ;

    sp = tPtrStartPoint ;
    do
      {
       bcdtmWrite_message(0,0,0,"sp = %8ld ** tPtr = %10ld sPtr = %10ld",sp,nodeAddrP(dtmP,sp)->tPtr,nodeAddrP(dtmP,sp)->sPtr) ;
       sp = nodeAddrP(dtmP,sp)->tPtr  ;
      } while( sp != tPtrStartPoint ) ;

   }


//  Scan Polygons To Get Left Most Point

 leftMostPoint = tPtrStartPoint ;
 pnt1P = pointAddrP(dtmP,leftMostPoint) ;
 sp = tPtrStartPoint ;
 do
   {
    pnt2P = pointAddrP(dtmP,sp) ;
    if( pnt2P->x < pnt1P->x || ( pnt2P->x == pnt1P->x && pnt2P->y < pnt1P->y))
      {
       leftMostPoint = sp ;
       pnt1P = pointAddrP(dtmP,leftMostPoint) ;

      }
    sp = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != tPtrStartPoint) ;

 sp = sPtrStartPoint ;
 do
   {
    pnt2P = pointAddrP(dtmP,sp) ;
    if( pnt2P->x < pnt1P->x || ( pnt2P->x == pnt1P->x && pnt2P->y < pnt1P->y))
      {
       leftMostPoint = sp ;
       pnt1P = pointAddrP(dtmP,leftMostPoint) ;

      }
    sp = nodeAddrP(dtmP,sp)->sPtr ;
   } while ( sp != sPtrStartPoint) ;

// Log Left Most Point

 if( dbg ) bcdtmWrite_message(0,0,0,"leftMostPoint[%8ld] =  %12.5lf %12.5lf %10.4lf ** sPtr = %10ld tPtr = %10ld",leftMostPoint,pnt1P->x,pnt1P->y,pnt1P->z,nodeAddrP(dtmP,leftMostPoint)->sPtr,nodeAddrP(dtmP,leftMostPoint)->tPtr) ;

// Get Next Left Most Point

 nextLeftMostPoint = dtmP->nullPnt ;
 if     ( nodeAddrP(dtmP,leftMostPoint)->tPtr != dtmP->nullPnt && nodeAddrP(dtmP,leftMostPoint)->sPtr == dtmP->nullPnt )
   {
    nextLeftMostPoint = nodeAddrP(dtmP,leftMostPoint)->tPtr ;
   }
 else if( nodeAddrP(dtmP,leftMostPoint)->tPtr == dtmP->nullPnt && nodeAddrP(dtmP,leftMostPoint)->sPtr != dtmP->nullPnt )
   {
    nextLeftMostPoint = nodeAddrP(dtmP,leftMostPoint)->sPtr ;
   }
 else if( nodeAddrP(dtmP,leftMostPoint)->tPtr == nodeAddrP(dtmP,leftMostPoint)->sPtr )
   {
    nextLeftMostPoint = nodeAddrP(dtmP,leftMostPoint)->sPtr ;
   }
 else
   {
    nextLeftMostPoint = nodeAddrP(dtmP,leftMostPoint)->tPtr ;
    if( ( cpnt = bcdtmList_nextClkDtmObject(dtmP,leftMostPoint,nextLeftMostPoint)) < 0 ) goto errexit ;
    while( nodeAddrP(dtmP,cpnt)->tPtr != leftMostPoint )
      {
       if( cpnt == nodeAddrP(dtmP,leftMostPoint)->sPtr ) nextLeftMostPoint = cpnt ;
       if( ( cpnt = bcdtmList_nextClkDtmObject(dtmP,leftMostPoint,cpnt)) < 0 ) goto errexit ;
      }
   }

// Log Next Left Most Point

 if( dbg ) bcdtmWrite_message(0,0,0,"nextLeftMostPoint[%8ld] =  %12.5lf %12.5lf %10.4lf ** sPtr = %10ld tPtr = %10ld",nextLeftMostPoint,pointAddrP(dtmP,nextLeftMostPoint)->x,pointAddrP(dtmP,nextLeftMostPoint)->y,pointAddrP(dtmP,nextLeftMostPoint)->z,nodeAddrP(dtmP,nextLeftMostPoint)->sPtr,nodeAddrP(dtmP,nextLeftMostPoint)->tPtr) ;

// Scan Around Outside Of tPtr and sPtr Polygons

 spnt = leftMostPoint ;
 points.push_back(spnt) ;
 npnt = nextLeftMostPoint ;
 points.push_back(npnt) ;
 do
   {
    if( ( apnt = bcdtmList_nextAntDtmObject(dtmP,npnt,spnt)) < 0 ) goto errexit ;
    while( nodeAddrP(dtmP,npnt)->tPtr != apnt && nodeAddrP(dtmP,npnt)->sPtr != apnt )
      {
       if( ( apnt = bcdtmList_nextAntDtmObject(dtmP,npnt,apnt)) < 0 ) goto errexit ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"aPnt = %8ld ** %12.5lf %12.5lf %10.4lf ** nPnt = %8ld nPnt->tPtr = %10ld nPnt->sPtr = %10ld",apnt,pointAddrP(dtmP,apnt)->x,pointAddrP(dtmP,apnt)->y,pointAddrP(dtmP,apnt)->z,npnt,nodeAddrP(dtmP,npnt)->tPtr,nodeAddrP(dtmP,npnt)->sPtr) ;
    points.push_back(apnt) ;
    spnt = npnt ;
    npnt = apnt ;
   } while( apnt != leftMostPoint ) ;

//  Log Union Points

 if( dbg == 2 )
   {
    for( ipnt = points.begin() ; ipnt != points.end() ; ++ipnt )
      {
       bcdtmWrite_message(0,0,0,"Union Point[%8ld] = %8ld",(long)(ipnt-points.begin()),*ipnt) ;
      }
   }

//  Set New Start Point

 if( points.size() > 0 )
   {
    startPoint = *points.begin() ;

//  Null Existing Polygons And Insert Tptr Polygon For Polygon Union

    if( bcdtmList_nullTptrListDtmObject(dtmP,tPtrStartPoint)) goto errexit ;
    if( bcdtmList_nullSptrListDtmObject(dtmP,sPtrStartPoint)) goto errexit ;
    spnt = startPoint ;
    for( ipnt = points.begin() + 1 ; ipnt < points.end()  ; ++ipnt )
      {
       nodeAddrP(dtmP,spnt)->tPtr = *ipnt ;
       spnt = *ipnt ;
      }

//  Check Connectivity Of Union Polygon - Development Only

    if( cdbg )
      {
       if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,startPoint,0))
         {
          bcdtmWrite_message(2,0,0,"Tptr Polygon Connectivity Error - Union Polygon") ;
          goto errexit ;
         }
      }
   }

//  Log Union Polygon

 if( dbg == 1 )
   {
    long numFeaturePts ;
    DPoint3d  *featurePtsP=nullptr ;
    BC_DTM_OBJ *dataP=nullptr ;
    if( dataP == nullptr ) if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
    if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,tPtrStartPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,2,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
    if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"unionPolygon.dat")) goto errexit ;
    if( bcdtmObject_destroyDtmObject(&dataP)) goto errexit ;
    if( featurePtsP != nullptr ) free(featurePtsP) ;
   }

//  Processing Completed

 cleanup :
 points.empty() ;

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Union Of Polgons Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Getting Union Of Polgons Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_validatePondDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 long              exitPoint,                  // ==> Pond Exit Point On Tptr Polygon
 bool&             pondValid                   // <==  Set To True For Valid Pond. Set To False For An Invalid Pond
)

// This Function Validates A Pond

{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    sp,tp,hullPoint ;
 double  lowPointZ ;

// Log Entry Parameters

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Validating Pond") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"exitPoint  = %8ld",exitPoint) ;
   }

//  Initialise

 pondValid = true ;

//  Check Connectivity Of Pond

 if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,exitPoint,0))
   {
    pondValid = false ;
    bcdtmWrite_message(2,0,0,"Pond Connectivity Error") ;
    goto errexit ;
   }

//  Get Lowest Elevation Value On Pond Boundary

 sp = exitPoint ;
 lowPointZ = pointAddrP(dtmP,sp)->z ;
 do
   {
    if( pointAddrP(dtmP,sp)->z < lowPointZ ) lowPointZ = pointAddrP(dtmP,sp)->z ;
    sp = nodeAddrP(dtmP,sp)->tPtr ;
   } while( sp != exitPoint ) ;

//  Check Exit Point Elevation

 if( lowPointZ < pointAddrP(dtmP,exitPoint)->z )
   {
    pondValid = false ;
    bcdtmWrite_message(2,0,0,"Exit Point Not Lowest Point On Pond Boundary") ;
    goto errexit ;
   }

// Check All Other Points At The Low Point Elevation On The Pond Boundary Connect
// To A External Point At A lower Elevation Than The Low Point Elevation

 sp = exitPoint ;
 do
   {
    if( pointAddrP(dtmP,sp)->z == lowPointZ )
      {
       if( bcdtmList_testForHullPointDtmObject(dtmP,sp,&hullPoint)) goto errexit ;
       if( ! hullPoint )
         {
          pondValid = true ;
          tp = nodeAddrP(dtmP,sp)->tPtr ;
          if( ( tp = bcdtmList_nextClkDtmObject(dtmP,sp,tp)) < 0 ) goto errexit ;
          while( nodeAddrP(dtmP,tp)->tPtr != sp && pondValid == true )
            {
             if( pointAddrP(dtmP,tp)->z == lowPointZ ) pondValid = false ;
             if( ( tp = bcdtmList_nextClkDtmObject(dtmP,sp,tp)) < 0 ) goto errexit ;
            }
          if( pondValid == false )
            {
             bcdtmWrite_message(0,0,0,"sp = %8ld ** %12.5lf %12.5lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
             bcdtmWrite_message(2,0,0,"No Flow Out From Pond Low Point") ;
             goto errexit ;
            }
         }
      }
    sp = nodeAddrP(dtmP,sp)->tPtr ;
   } while( sp != exitPoint ) ;

// Clean Up

 cleanup :

// Return

 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Determing Pond About Low Point Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Determing Pond About Low Point Error") ;
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
int bcdtmDrainage_checkForPondExitPointDtmObject
(
 BC_DTM_OBJ        *dtmP,                     // ==> Pointer To Dtm Object
 int               point,                     // ==> Point To Test For An Exit Point
 double            lowPointZ,                 // ==> Low Point Elevation
 bool&             exitPoint                  // <== Set To True For Exit Point Otherwise Set To False
)

// This Checks For An Exit Point

{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    cpnt,hullPoint ;

// Log Entry Parameters

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking For Exit Pond") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"point      = %8ld",exitPoint) ;
   }

//  Initialise

 exitPoint = false ;

//  Check For Hull Point

 if( bcdtmList_testForHullPointDtmObject(dtmP,point,&hullPoint)) goto errexit ;
 if( hullPoint )
   {
    exitPoint = true ;
   }
 else
   {

//  Scan External To Pond For Exit Point

    cpnt = nodeAddrP(dtmP,point)->tPtr ;
    if( ( cpnt = bcdtmList_nextClkDtmObject(dtmP,point,cpnt)) < 0 ) goto errexit ;
    while( nodeAddrP(dtmP,cpnt)->tPtr != point && exitPoint == false )
      {
       if( pointAddrP(dtmP,cpnt)->z < lowPointZ ) exitPoint = true ;
       if( ( cpnt = bcdtmList_nextClkDtmObject(dtmP,point,cpnt)) < 0 ) goto errexit ;
      }
   }

// Clean Up

 cleanup :

// Return

 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Determing Pond About Low Point Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Determing Pond About Low Point Error") ;
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
int bcdtmDrainage_scanPondBoundaryPointForAnExternalZeroEdgeDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 long              pondPoint,                  // ==> Pond Point To Scan
 double            lowPointZ,                  // ==> Low Point Z
 int&              edgePnt1,                   // <== Zero Edge Point 1
 int&              edgePnt2,                   // <== Zero Edge Point 2
 int&              zeroEdgeType                // <== Zero Edge Type < 0 = Not Found , 1 = External , 2 = Internal
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   antPnt,clkPnt,nextClkPnt ;

// Log Function Arguements

 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Scanning Pond Boundary Point For A Zero Edge") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pondPoint       = %8ld",pondPoint) ;
    bcdtmWrite_message(0,0,0,"lowPointZ       = %8.3lf",lowPointZ) ;
   }

// Initialise

 zeroEdgeType = 0 ;
 edgePnt1 = dtmP->nullPnt ;
 edgePnt2 = dtmP->nullPnt ;

// Scan External To Pond Point For A Zero Edge

 antPnt = nodeAddrP(dtmP,pondPoint)->tPtr ;
 if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,pondPoint,antPnt))  < 0 ) goto errexit ;
 while( nodeAddrP(dtmP,clkPnt)->tPtr != pondPoint && zeroEdgeType == 0 )
   {
    if( ( nextClkPnt = bcdtmList_nextClkDtmObject(dtmP,pondPoint,clkPnt)) < 0 ) goto errexit ;
    if( pointAddrP(dtmP,antPnt)->z != lowPointZ && pointAddrP(dtmP,clkPnt)->z == lowPointZ && pointAddrP(dtmP,nextClkPnt)->z == lowPointZ )
      {
       zeroEdgeType = 2 ;
       edgePnt1 = pondPoint ;
       edgePnt2 = clkPnt ;
      }
    if( pointAddrP(dtmP,antPnt)->z == lowPointZ && pointAddrP(dtmP,clkPnt)->z == lowPointZ && pointAddrP(dtmP,nextClkPnt)->z != lowPointZ )
      {
       zeroEdgeType = 2 ;
       edgePnt1 = clkPnt ;
       edgePnt2 = pondPoint ;
      }
    antPnt = clkPnt ;
    clkPnt = nextClkPnt ;
   }

// Log Zero Edge

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"zeroEdgeType = %2d ** edgePnt1 = %10d edgePnt2 = %10d",zeroEdgeType,edgePnt1,edgePnt2,edgePnt2) ;
    if( zeroEdgeType != 0 )
      {
       if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,edgePnt1,edgePnt2)) < 0 ) goto errexit ;
       if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,edgePnt1,edgePnt2)) < 0 ) goto errexit ;
       bcdtmWrite_message(0,0,0,"edgePnt1[%8ld] ** %12.5lf %12.5lf %10.4lf",edgePnt1,pointAddrP(dtmP,edgePnt1)->x,pointAddrP(dtmP,edgePnt1)->y,pointAddrP(dtmP,edgePnt1)->z) ;
       bcdtmWrite_message(0,0,0,"edgePnt2[%8ld] ** %12.5lf %12.5lf %10.4lf",edgePnt2,pointAddrP(dtmP,edgePnt2)->x,pointAddrP(dtmP,edgePnt2)->y,pointAddrP(dtmP,edgePnt2)->z) ;
       bcdtmWrite_message(0,0,0,"clkPnt  [%8ld] ** %12.5lf %12.5lf %10.4lf",clkPnt,pointAddrP(dtmP,clkPnt)->x,pointAddrP(dtmP,clkPnt)->y,pointAddrP(dtmP,clkPnt)->z) ;
       bcdtmWrite_message(0,0,0,"antPnt  [%8ld] ** %12.5lf %12.5lf %10.4lf",antPnt,pointAddrP(dtmP,antPnt)->x,pointAddrP(dtmP,antPnt)->y,pointAddrP(dtmP,antPnt)->z) ;
      }
   }

// Clean Up

 cleanup :

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Pond Boundary Point For A Zero Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Pond Boundary Point For A Zero Edge Error") ;
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
int bcdtmDrainage_expandPondOverZeroSlopeTrianglesFromZeroEdgeDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 long              *pondStartPointP,           // <=> Start Point On Tptr Polygon
 int               edgeType,                   // ==> Type Of Zero Edge
 int               edgePnt1,                   // ==> Zero Edge Point 1
 int               edgePnt2,                   // ==> Zero Edge Point 2
 double            lowPointZ                   // ==> Low Point Z
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 int    zeroStartPoint;
 DTMDirection expandDirection;
 long   numFeaturePts,startPoint,newStartPoint ;
 DTMDirection  direction ;
 double area,beforeArea,afterArea ;
 DPoint3d    *featurePtsP=nullptr ;
 BC_DTM_OBJ     *dataP=nullptr ;

// Log Function Arguements

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Expanding Pond Over Zero Slope Triangles From Zero Edge") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pondStartPointP = %8ld",*pondStartPointP) ;
    bcdtmWrite_message(0,0,0,"edgeType        = %8d",edgeType) ;
    bcdtmWrite_message(0,0,0,"edgePnt1        = %8d",edgePnt1) ;
    bcdtmWrite_message(0,0,0,"edgePnt2        = %8d",edgePnt2) ;
    bcdtmWrite_message(0,0,0,"lowPointZ       = %10.4lf",lowPointZ) ;
   }

// Log Edge Point Coordinates

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"edgePnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",edgePnt1,pointAddrP(dtmP,edgePnt1)->x,pointAddrP(dtmP,edgePnt1)->y,pointAddrP(dtmP,edgePnt1)->z) ;
    bcdtmWrite_message(0,0,0,"edgePnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",edgePnt2,pointAddrP(dtmP,edgePnt2)->x,pointAddrP(dtmP,edgePnt2)->y,pointAddrP(dtmP,edgePnt2)->z) ;
   }

// Check Direction Of Tptr Polygon

  if( cdbg )
    {
     if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,*pondStartPointP,&beforeArea,&direction)) goto errexit ;
     if( dbg ) bcdtmWrite_message(0,0,0,"Before Expansion ** Tptr Polygon ** diection = %2ld area = %15.5lf",direction,beforeArea) ;
     if (direction != DTMDirection::AntiClockwise)
       {
        bcdtmWrite_message(2,0,0,"Tptr Polygon Is Not Counter Clockwise Before Pond Expansion") ;
        goto errexit ;
       }
    }

//   Initialise

 startPoint = *pondStartPointP ;

//   Polygonize Zero Slope Triangles

 if( bcdtmDrainage_placeSptrPolygonAroundZeroSlopeTrianglesDtmObject(dtmP,edgePnt1,edgePnt2,expandDirection,zeroStartPoint)) goto errexit ;
 if( dbg )
   {
    if( bcdtmMath_calculateAreaAndDirectionSptrPolygonDtmObject(dtmP,zeroStartPoint,&area,&direction)) goto errexit ;
    bcdtmWrite_message(0,0,0,"zeroStartPoint = %10ld expandDirection = %2d expandArea = %20.5lf",zeroStartPoint,expandDirection,area) ;
   }

//  Log Before Pond Boundary And Polygonised Zero Slope Triangles To File

 if( dbg )
   {
    if( dataP == nullptr ) if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
    if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,2,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
    if( bcdtmList_copySptrListToPointArrayDtmObject(dtmP,zeroStartPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::ContourLine,2,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
    if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"intersectPolygons.dat")) goto errexit ;
    if( bcdtmObject_destroyDtmObject(&dataP)) goto errexit ;
   }

//  Expand Direction Counter Clockwise -  Represents An Internal Edge Of Zero Slope Triangles

 if (expandDirection == DTMDirection::AntiClockwise)
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Expanded Polygon Is Anti Clockwise") ;
    bcdtmList_nullTptrListDtmObject(dtmP,startPoint) ;
    bcdtmList_copySptrListToTptrListDtmObject(dtmP,zeroStartPoint) ;
    bcdtmList_nullSptrListDtmObject(dtmP,zeroStartPoint) ;

//  Expand To Outer Edge Of Zero Slope Triangles

    startPoint = zeroStartPoint ;
    if( bcdtmDrainage_expandPondToOuterEdgeOfZeroSlopeTrianglesDtmObject(dtmP,&startPoint,lowPointZ)) goto errexit ;
   }

// Expand Direction Clockwise - Union Tptr And Sptr Polygons

 if (expandDirection == DTMDirection::Clockwise)
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Expanded Polygon Is Clockwise") ;

//  Get Union Of Polygons

    newStartPoint=0 ;
    if( bcdtmDrainage_getUnionOfPolygonsDtmObject(dtmP,startPoint,zeroStartPoint,newStartPoint)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"newStartPoint = %8ld",newStartPoint) ;
    if( newStartPoint == dtmP->nullPnt )
      {
       bcdtmWrite_message(2,0,0,"Failed To Determine Polygon Union") ;
       goto errexit ;
      }
    startPoint = newStartPoint ;

//  Log Union Polygon

    if( dbg )
      {
       if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction)) goto errexit ;
       bcdtmWrite_message(0,0,0,"area = %12.3lf direction = %2ld",area,direction) ;
       if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPoint,&featurePtsP,&numFeaturePts)) goto errexit ;
       if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,2,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
       if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"unionPolygon.dat")) goto errexit ;
       if( bcdtmObject_destroyDtmObject(&dataP)) goto errexit ;
      }
   }

// Check Direction And Area Of Expanded Tptr Polygon

 if( cdbg )
   {
    if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&afterArea,&direction)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"After  Expansion ** Tptr Polygon ** diection = %2ld area = %15.5lf",direction,afterArea) ;
    if (direction != DTMDirection::AntiClockwise)
      {
       bcdtmWrite_message(2,0,0,"Tptr Polygon Is Not Counter Clockwise After Pond Expansion") ;
       goto errexit ;
      }
    if( afterArea < beforeArea )
      {
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"beforeArea - afterArea = %20.15lf",beforeArea-afterArea) ;
          bcdtmWrite_message(0,0,0,"Area Before Expansion = %20.5lf Area After Expansion = %20.5lf",beforeArea,afterArea) ;
         }
       if( beforeArea-afterArea > 0.001 )
         {
          bcdtmWrite_message(2,0,0,"Tptr Polygon Area Is Smaller After Pond Expansion") ;
          goto errexit ;
         }
      }
   }

//  Reset Pond Start Point

*pondStartPointP = startPoint ;

// Clean Up

 cleanup :

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Pond Over Zero Slope Triangles From Zero Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Pond Over Zero Slope Triangles From Zero Edge Error") ;
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
int bcdtmDrainage_scanPondBoundaryPointForAnExternalZeroEdgeDtmObjectTwo
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 long              priorPondPoint,             // ==> Prior Pond Point To Scan Point
 long              pondPoint,                  // ==> Pond Point To Scan
 long              nextPondPoint,              // ==> Next Pond Point To Scan Point Point
 double            lowPointZ,                  // ==> Low Point Z
 int&              edgePnt1,                   // <== Zero Edge Point 1
 int&              edgePnt2,                   // <== Zero Edge Point 2
 int&              zeroEdgeType                // <== Zero Edge Type < 0 = Not Found , 1 = External , 2 = Internal
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   antPnt,clkPnt,nextClkPnt,edgeClkPnt,edgeAntPnt ;

// Log Function Arguements

 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Scanning Pond Boundary Point For A Zero Edge") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"priorPondPoint  = %8ld",priorPondPoint) ;
    bcdtmWrite_message(0,0,0,"pondPoint       = %8ld",pondPoint) ;
    bcdtmWrite_message(0,0,0,"nextPondPoint   = %8ld",nextPondPoint) ;
    bcdtmWrite_message(0,0,0,"lowPointZ       = %8.3lf",lowPointZ) ;
   }

// Initialise

 zeroEdgeType = 0 ;
 edgePnt1 = dtmP->nullPnt ;
 edgePnt2 = dtmP->nullPnt ;

// Scan For Clockwise Ridge

 clkPnt = nodeAddrP(dtmP,pondPoint)->tPtr ;
 while( pointAddrP(dtmP,clkPnt)->z == pointAddrP(dtmP,pondPoint)->z && clkPnt != priorPondPoint )
   {
    if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,pondPoint,clkPnt)) < 0 ) goto errexit ;
   }

// Scan For Counter Clockwise Ridge

 if( clkPnt != priorPondPoint )
   {
    antPnt = priorPondPoint ;
    while( pointAddrP(dtmP,antPnt)->z == pointAddrP(dtmP,pondPoint)->z && antPnt != clkPnt )
      {
       if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,pondPoint,antPnt)) < 0 ) goto errexit ;
      }

//  Scan Between Ridges For Zero Edge

    if( antPnt != clkPnt )
      {
       edgeAntPnt = clkPnt ;
       if( ( edgeClkPnt = bcdtmList_nextClkDtmObject(dtmP,pondPoint,edgeAntPnt))  < 0 ) goto errexit ;
       while( edgeClkPnt != antPnt && zeroEdgeType == 0 )
         {
          if( ( nextClkPnt = bcdtmList_nextClkDtmObject(dtmP,pondPoint,edgeClkPnt)) < 0 ) goto errexit ;
          if( pointAddrP(dtmP,antPnt)->z != lowPointZ && pointAddrP(dtmP,clkPnt)->z == lowPointZ && pointAddrP(dtmP,nextClkPnt)->z == lowPointZ )
            {
             zeroEdgeType = 2 ;
             edgePnt1 = pondPoint ;
             edgePnt2 = clkPnt ;
            }
          if( pointAddrP(dtmP,antPnt)->z == lowPointZ && pointAddrP(dtmP,clkPnt)->z == lowPointZ && pointAddrP(dtmP,nextClkPnt)->z != lowPointZ )
            {
             zeroEdgeType = 2 ;
             edgePnt1 = clkPnt ;
             edgePnt2 = pondPoint ;
            }
          edgeAntPnt = edgeClkPnt ;
          edgeClkPnt = nextClkPnt ;
         }
      }
   }



// Scan External To Pond Point For A Zero Edge

 antPnt = nodeAddrP(dtmP,pondPoint)->tPtr ;
 if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,pondPoint,antPnt))  < 0 ) goto errexit ;
 while( nodeAddrP(dtmP,clkPnt)->tPtr != pondPoint && zeroEdgeType == 0 )
   {
    if( ( nextClkPnt = bcdtmList_nextClkDtmObject(dtmP,pondPoint,clkPnt)) < 0 ) goto errexit ;
    if( pointAddrP(dtmP,antPnt)->z != lowPointZ && pointAddrP(dtmP,clkPnt)->z == lowPointZ && pointAddrP(dtmP,nextClkPnt)->z == lowPointZ )
      {
       zeroEdgeType = 2 ;
       edgePnt1 = pondPoint ;
       edgePnt2 = clkPnt ;
      }
    if( pointAddrP(dtmP,antPnt)->z == lowPointZ && pointAddrP(dtmP,clkPnt)->z == lowPointZ && pointAddrP(dtmP,nextClkPnt)->z != lowPointZ )
      {
       zeroEdgeType = 2 ;
       edgePnt1 = clkPnt ;
       edgePnt2 = pondPoint ;
      }
    antPnt = clkPnt ;
    clkPnt = nextClkPnt ;
   }

// Log Zero Edge

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"zeroEdgeType = %2d ** edgePnt1 = %10d edgePnt2 = %10d",zeroEdgeType,edgePnt1,edgePnt2,edgePnt2) ;
    if( zeroEdgeType != 0 )
      {
       if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,edgePnt1,edgePnt2)) < 0 ) goto errexit ;
       if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,edgePnt1,edgePnt2)) < 0 ) goto errexit ;
       bcdtmWrite_message(0,0,0,"edgePnt1[%8ld] ** %12.5lf %12.5lf %10.4lf",edgePnt1,pointAddrP(dtmP,edgePnt1)->x,pointAddrP(dtmP,edgePnt1)->y,pointAddrP(dtmP,edgePnt1)->z) ;
       bcdtmWrite_message(0,0,0,"edgePnt2[%8ld] ** %12.5lf %12.5lf %10.4lf",edgePnt2,pointAddrP(dtmP,edgePnt2)->x,pointAddrP(dtmP,edgePnt2)->y,pointAddrP(dtmP,edgePnt2)->z) ;
       bcdtmWrite_message(0,0,0,"clkPnt  [%8ld] ** %12.5lf %12.5lf %10.4lf",clkPnt,pointAddrP(dtmP,clkPnt)->x,pointAddrP(dtmP,clkPnt)->y,pointAddrP(dtmP,clkPnt)->z) ;
       bcdtmWrite_message(0,0,0,"antPnt  [%8ld] ** %12.5lf %12.5lf %10.4lf",antPnt,pointAddrP(dtmP,antPnt)->x,pointAddrP(dtmP,antPnt)->y,pointAddrP(dtmP,antPnt)->z) ;
      }
   }

// Clean Up

 cleanup :

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Pond Boundary Point For A Zero Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Pond Boundary Point For A Zero Edge Error") ;
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
int bcdtmDrainage_countNumberOfExternalZeroEdgesAtPointDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 long              priorPoint,                 // ==> Prior Pond Point To Scan Point
 long              pondPoint,                  // ==> Pond Point To Scan
 long              nextPoint,                  // ==> Next Pond Point To Scan Point Point
 double            lowPointZ,                  // ==> Low Point Z
 int&              numZeroEdges                // <== Number Of External Zero Edges
)
{
 int ret=DTM_SUCCESS,cPnt ;
 numZeroEdges = 0 ;
 if( pointAddrP(dtmP,pondPoint)->z == lowPointZ && nodeAddrP(dtmP,pondPoint)->hPtr == dtmP->nullPnt )
   {
    cPnt = nextPoint ;
    if(( cPnt = bcdtmList_nextClkDtmObject(dtmP,pondPoint,cPnt)) < 0 ) goto errexit ;
    while( cPnt != priorPoint )
      {
       if( pointAddrP(dtmP,cPnt)->z <  lowPointZ )
         {
          cPnt = priorPoint ;
          numZeroEdges = 0 ;
         }
       else
         {
          if( pointAddrP(dtmP,cPnt)->z == lowPointZ ) ++numZeroEdges ;
          if(( cPnt = bcdtmList_nextClkDtmObject(dtmP,pondPoint,cPnt)) < 0 ) goto errexit ;
         }
      }
   }
// Clean Up

 cleanup :

// Exit

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
int bcdtmDrainage_scanPondBoundarySectionForAZeroEdgeDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 long              pondStartPoint,             // ==> Start Point On Tptr Polygon
 long              pondEndPoint,               // ==> End Point On Tptr Polygon
 double            lowPointZ,                  // ==> Low Point Z
 int&              edgePnt1,                   // <== Zero Edge Point 1
 int&              edgePnt2,                   // <== Zero Edge Point 2
 int&              zeroEdgeType                // <== Zero Edge Type < 0 = Not Found , 1 = External , 2 = Internal
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   sp,np,cp,ap,tp,scanEndPoint ;

// Log Function Arguements

 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Scanning Pond Boundary For A Zero Edge") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pondStartPoint  = %8ld",pondStartPoint) ;
    bcdtmWrite_message(0,0,0,"pondEndPoint    = %8ld",pondEndPoint) ;
    bcdtmWrite_message(0,0,0,"lowPointZ       = %8.3lf",lowPointZ) ;
   }

// Initialise

 zeroEdgeType = 0 ;
 edgePnt1 = dtmP->nullPnt ;
 edgePnt2 = dtmP->nullPnt ;
 scanEndPoint = nodeAddrP(dtmP,pondEndPoint)->tPtr ;

// Log Start And End Section Points

 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"pondStartPoint = %8ld ** %12.5lf %12.5lf %10.4lf",pondStartPoint,pointAddrP(dtmP,pondStartPoint)->x,pointAddrP(dtmP,pondStartPoint)->y,pointAddrP(dtmP,pondStartPoint)->z) ;
    bcdtmWrite_message(0,0,0,"pondEndPoint   = %8ld ** %12.5lf %12.5lf %10.4lf",pondEndPoint,pointAddrP(dtmP,pondEndPoint)->y,pointAddrP(dtmP,pondStartPoint)->y,pointAddrP(dtmP,pondStartPoint)->z) ;
   }

// Scan Pond Boundary For External Zero Edges

 sp = pondStartPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    if( pointAddrP(dtmP,sp)->z == lowPointZ && pointAddrP(dtmP,np)->z == lowPointZ )
      {
       if( ! bcdtmList_testForHullLineDtmObject(dtmP,sp,np) )
         {
          if( ( cp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
          if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
          if( pointAddrP(dtmP,cp)->z == lowPointZ && pointAddrP(dtmP,ap)->z != lowPointZ )
            {
             zeroEdgeType = 1 ;
             edgePnt1 = sp ;
             edgePnt2 = np ;
            }
         }
      }
    sp = np ;
   } while ( sp != scanEndPoint && ! zeroEdgeType ) ;

//  Scan Pond Boundary For Internal Zero Edges

 if( edgePnt1 == dtmP->nullPnt )
   {
    sp = pondStartPoint ;
    do
      {
       np = nodeAddrP(dtmP,sp)->tPtr ;
       if( pointAddrP(dtmP,sp)->z == lowPointZ && pointAddrP(dtmP,np)->z == lowPointZ )
         {
          if( ! bcdtmList_testForHullLineDtmObject(dtmP,sp,np) )
            {
             if( ( cp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
             if( pointAddrP(dtmP,sp)->z == lowPointZ && pointAddrP(dtmP,np)->z == lowPointZ && pointAddrP(dtmP,cp)->z == lowPointZ  )
               {
                cp = np ;
                if( ( tp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
                if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,tp)) < 0 ) goto errexit ;
                while( nodeAddrP(dtmP,tp)->tPtr != sp && nodeAddrP(dtmP,ap)->tPtr != sp && edgePnt1 == dtmP->nullPnt )
                  {
                   if( ( pointAddrP(dtmP,cp)->z == lowPointZ && pointAddrP(dtmP,tp)->z == lowPointZ && pointAddrP(dtmP,ap)->z != lowPointZ ) ||
                       ( pointAddrP(dtmP,cp)->z != lowPointZ && pointAddrP(dtmP,tp)->z == lowPointZ && pointAddrP(dtmP,ap)->z == lowPointZ )    )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"Zero Edge ** cp->z = %10.4lf tp->z = %10.4lf ap->z = %10.4lf",pointAddrP(dtmP,cp)->z,pointAddrP(dtmP,tp)->z,pointAddrP(dtmP,ap)->z ) ;
                      zeroEdgeType = 2 ;
                      edgePnt1 = sp ;
                      edgePnt2 = tp ;
                     }
                   cp = tp ;
                   tp = ap ;
                   if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,tp)) < 0 ) goto errexit ;
                  }
               }
            }
         }
       sp = np ;
      } while ( sp != scanEndPoint && ! zeroEdgeType ) ;
   }

// Log Zero Edge

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"zeroEdgeType = %2d ** edgePnt1 = %10d edgePnt2 = %10d",zeroEdgeType,edgePnt1,edgePnt2,edgePnt2) ;
    if( ( ap = bcdtmList_nextAntDtmObject(dtmP,edgePnt1,edgePnt2)) < 0 ) goto errexit ;
    if( ( cp = bcdtmList_nextClkDtmObject(dtmP,edgePnt1,edgePnt2)) < 0 ) goto errexit ;
    bcdtmWrite_message(0,0,0,"edgePnt1[%8ld] ** %12.5lf %12.5lf %10.4lf",edgePnt1,pointAddrP(dtmP,edgePnt1)->x,pointAddrP(dtmP,edgePnt1)->y,pointAddrP(dtmP,edgePnt1)->z) ;
    bcdtmWrite_message(0,0,0,"edgePnt2[%8ld] ** %12.5lf %12.5lf %10.4lf",edgePnt2,pointAddrP(dtmP,edgePnt2)->x,pointAddrP(dtmP,edgePnt2)->y,pointAddrP(dtmP,edgePnt2)->z) ;
    bcdtmWrite_message(0,0,0,"clkPnt  [%8ld] ** %12.5lf %12.5lf %10.4lf",cp,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y,pointAddrP(dtmP,cp)->z) ;
    bcdtmWrite_message(0,0,0,"antPnt  [%8ld] ** %12.5lf %12.5lf %10.4lf",ap,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,ap)->z) ;
   }

// Clean Up

 cleanup :

// Exit

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Pond Boundary For A Zero Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Pond Boundary For A Zero Edge Error") ;
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
int bcdtmDrainage_polygoniseZeroSlopeTrianglesDtmObject
(
 BC_DTM_OBJ *dtmP,                                 // ==> Pointer To DTM Object
 DTMZeroSlopePolygonVector& zeroSlopePolygons      // ==> Pointer To Zero Slope Polygons Vector
)
/*
** This Function Polygonises Zero Slope Triangles
**
** The polygonisation can create both clockwise and anticlockwise polygons.
** A clockwise polygon represent an internal edge ( island ) of a polygonised area.
** A anti clockwise polygon polygon represents an external edge of a polygonised area.
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 int    numLines ;
 long   p1,p2,p3,p4,sp,np,cp,lcp,clPtr,zeroEdge,offset ;
 long   numFeatures=0,hullLine,numFeaturePts ;
 bool voidTriangle;
 long   *pointListP = nullptr, numPointList;
 DTMFeatureType dtmFeatureType = DTMFeatureType::Breakline;
 unsigned char   *cP,*lineMarkP=nullptr ;
 double area ;
 DTMDirection direction ;
 DPoint3d     *featurePtsP=nullptr ;
 BC_DTM_OBJ   *dataP=nullptr ;
 DTMPointList *pointList ;
 DTMZeroSlopePolygon *zeroSlopePolygon ;
 DTMZeroSlopePolygonVector::iterator zsp ;

// Write Entry Message

 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Zero Slope Triangles") ;

// Create DTM To Store Zero Slope Boundaries For Development Purposes

 if( dbg == 2 ) if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;

// Null Tptr Values

 bcdtmList_nullTptrValuesDtmObject(dtmP) ;

// Allocate Memory For Marking Zero Slope Polygon Lines

 numLines = ( dtmP->cListPtr ) / 8 ;
 lineMarkP = ( unsigned char * ) malloc( numLines * sizeof(char)) ;
 if( lineMarkP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( cP = lineMarkP ; cP < lineMarkP + numLines ; ++cP ) *cP = ( char ) 0 ;

// Scan Tin Looking For A Zero Slope Start Line

 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    if( ( clPtr = nodeAddrP(dtmP,p1)->cPtr) != dtmP->nullPtr )
      {
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while ( clPtr != dtmP->nullPtr )
         {
          p3    = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          if( ( p4 = bcdtmList_nextClkDtmObject(dtmP,p1,p3)) < 0 ) goto errexit ;
          if( p2 > p1 && p3 > p1 && nodeAddrP(dtmP,p3)->hPtr != p1  )
            {

//           Check For Zero Slope

             if( pointAddrP(dtmP,p1)->z == pointAddrP(dtmP,p2)->z && pointAddrP(dtmP,p1)->z == pointAddrP(dtmP,p3)->z )
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Zero Slope Triangle Detected") ;

//              Check For A None Void Triangle

                if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidTriangle)) goto errexit ;
                if( ! voidTriangle )
                  {

//                 Check Edge Is Not Already Part Of Zero Slope Edge Feature

                   bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p1,p3) ;
                   if( ! bcdtmFlag_testFlag(lineMarkP,offset) )
                     {

//                    Check For Zero Slope Edge

                      zeroEdge = 0 ;
                      if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,p1,p3,&hullLine)) goto errexit ;
                      if( hullLine ) zeroEdge = 1 ;
                      else if( pointAddrP(dtmP,p1)->z != pointAddrP(dtmP,p4)->z ) zeroEdge = 1 ;
                      if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Zero Edge = %2d",zeroEdge) ;


//                    Zero Edge Detected

                      if( zeroEdge )
                        {
                         if( dbg == 2 )
                           {
                            bcdtmWrite_message(0,0,0,"Zero Edge Detected") ;
                            bcdtmWrite_message(0,0,0,"hullLine = %2ld",hullLine) ;
                            bcdtmWrite_message(0,0,0,"p1 = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",p1,nodeAddrP(dtmP,p1)->hPtr,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->z) ;
                            bcdtmWrite_message(0,0,0,"p2 = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",p2,nodeAddrP(dtmP,p2)->hPtr,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->z) ;
                            bcdtmWrite_message(0,0,0,"p3 = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",p3,nodeAddrP(dtmP,p3)->hPtr,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->z) ;
                            bcdtmWrite_message(0,0,0,"p4 = %6ld FPTR = %9ld ** %12.5lf %12.5lf %10.4lf",p4,nodeAddrP(dtmP,p4)->hPtr,pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->z) ;
                           }

//                       Extract Zero Slope Polygon

                         sp = p1 ;
                         np = p3 ;
                         do
                           {

//                          Check For And Remove Knot In Zero Slope Polygon

                            if( nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt )
                              {
                               p4 = nodeAddrP(dtmP,sp)->tPtr ;
                               while( p4 != sp )
                                 {
                                  p3 = nodeAddrP(dtmP,p4)->tPtr ;
                                  nodeAddrP(dtmP,p4)->tPtr = dtmP->nullPnt ;
                                  p4 = p3 ;
                                 }
                              }

//                          Set Next Point In Zero Slope Polygon

                            nodeAddrP(dtmP,sp)->tPtr = np ;
                            lcp = cp = sp ;
                            if( ( cp = bcdtmList_nextClkDtmObject(dtmP,np,cp)) < 0 ) goto errexit ;
                            if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,np,cp,&hullLine)) goto errexit ;
                            while( ! hullLine && pointAddrP(dtmP,cp)->z == pointAddrP(dtmP,sp)->z  )
                              {
                               lcp = cp ;
                               if( ( cp = bcdtmList_nextClkDtmObject(dtmP,np,cp)) < 0 ) goto errexit ;
                               if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,np,cp,&hullLine)) goto errexit ;
                              }
                            if( pointAddrP(dtmP,cp)->z != pointAddrP(dtmP,sp)->z  ) cp = lcp ;
                            sp = np ;
                            np = cp ;
                           } while ( sp != p1 ) ;

//                       Check Connectivity Tptr Polygon

                         if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,sp,0))
                           {
                            bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
                            goto errexit ;
                           }

//                       Get Direction Of Tptr Polygon

                         bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,sp,&area,&direction) ;
                         if( dbg == 2  && direction == DTMDirection::Clockwise )
                           {
                            bcdtmWrite_message(0,0,0,"area = %20.10lf direction = %2ld",area,direction) ;
                            bcdtmWrite_message(0,0,0,"Internal Polygon Boundary Found") ;
                           }

//                       Store Zero Slope Polygon Boundary

                         if( dataP != nullptr )
                           {
                            if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,sp,&featurePtsP,&numFeaturePts)) goto errexit ;
                            if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                            if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
                           }

//                       Store Zero Slope Polygon

                         if( bcdtmList_copyTptrListToPointListDtmObject(dtmP,sp,&pointListP,&numPointList)) goto errexit ;
                         pointList = new DTMPointList(pointListP,numPointList) ;
                         zeroSlopePolygon = new DTMZeroSlopePolygon(direction,*pointList) ;
                         zeroSlopePolygons.push_back(*zeroSlopePolygon) ;
                         delete pointList ;
                         delete zeroSlopePolygon ;

//                       Mark Zero Slope Polygon Lines

                         sp = p1 ;
                         do
                           {
                            np = nodeAddrP(dtmP,sp)->tPtr ;
                            bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,sp,np) ;
                            bcdtmFlag_setFlag(lineMarkP,offset) ;
                            sp = np ;
                           } while ( sp != p1 ) ;

//                       Null Tptr List

                         bcdtmList_nullTptrListDtmObject(dtmP,p1) ;

//                       Log Some Debug Stats

                         if( dbg == 2 ) bcdtmWrite_message(0,0,0,"ZeroSlopePolygon[%8ld] ** area = %15.8lf direction = %2ld",zeroSlopePolygons.size(),area,direction) ;

                        }
                     }
                  }
               }
            }
          p2 = p3 ;
         }
      }
   }

// Write Number Of Zero Slope Polygons

 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Zero Slope Polygons = %6ld",zeroSlopePolygons.size()) ;

// Write To Geopak Dat File Development Only

 if( dataP != nullptr ) bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"zeroSlopePolygons.dat") ;

// Check Validity Of Zero Slope Polygons

 if( cdbg )
   {

    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Zero Slope Polygons") ;

    // Check All Points On Zero Slope Polygon Have The Same Elevation Value

    if( dbg ) bcdtmWrite_message(0,0,0,"**** Checking Elevation Values Of Zero Slope Polygons") ;
    for( zsp = zeroSlopePolygons.begin() ; zsp < zeroSlopePolygons.end() ; ++zsp )
      {
       double elevation ;
       for( int n = 0 ; n < zsp->pointList.numPoints ; ++n )
         {
          if( n == 0 ) elevation = pointAddrP(dtmP,zsp->pointList.pointsP[n])->z ;
          else if( pointAddrP(dtmP,zsp->pointList.pointsP[n])->z != elevation )
            {
             bcdtmWrite_message(1,0,0,"Different Elevations Values On Zero Slope Polygon") ;
             goto errexit ;
            }
         }
      }

    // Check There Are No Zero Slope Triangles On Zero Slope Polygon Edge

    if( dbg ) bcdtmWrite_message(0,0,0,"**** Checking For Zero Slope Triangles On Zero Slope Polygons") ;
    for( zsp = zeroSlopePolygons.begin() ; zsp < zeroSlopePolygons.end() ; ++zsp )
      {
       for( int n = 0 ; n < zsp->pointList.numPoints - 1 ; ++n )
         {
          sp = zsp->pointList.pointsP[n] ;
          np = zsp->pointList.pointsP[n+1] ;
          if( zsp->direction == DTMDirection::Clockwise )
            {
             if(( cp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
             if( bcdtmList_testLineDtmObject(dtmP,np,cp) )
               {
                if( pointAddrP(dtmP,cp)->z == pointAddrP(dtmP,sp)->z )
                  {
                   bcdtmWrite_message(0,0,0,"sp = %8ld ** %12.5lf %12.5lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                   bcdtmWrite_message(0,0,0,"np = %8ld ** %12.5lf %12.5lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
                   bcdtmWrite_message(0,0,0,"cp = %8ld ** %12.5lf %12.5lf %10.4lf",cp,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y,pointAddrP(dtmP,cp)->z) ;
                   bcdtmWrite_message(1,0,0,"Zero Slope Triangle On Inside Of Clock Wise Zero Slope Polygon") ;
                   goto errexit ;
                  }
               }
            }
          else
            {
             if( ! bcdtmList_testForHullLineDtmObject(dtmP,sp,np) )
               {
                if(( cp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
                if( pointAddrP(dtmP,cp)->z == pointAddrP(dtmP,sp)->z )
                  {
                   bcdtmWrite_message(0,0,0,"sp = %8ld ** %12.5lf %12.5lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                   bcdtmWrite_message(0,0,0,"np = %8ld ** %12.5lf %12.5lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
                   bcdtmWrite_message(0,0,0,"cp = %8ld ** %12.5lf %12.5lf %10.4lf",cp,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y,pointAddrP(dtmP,cp)->z) ;
                   bcdtmWrite_message(1,0,0,"Zero Slope Triangle On Outside Of Counter Clock Wise Zero Slope Polygon") ;
                   goto errexit ;
                  }
               }
            }
         }
      }
   }

// Clean Up

 cleanup :
 if( dataP       != nullptr ) bcdtmObject_destroyDtmObject(&dataP) ;
 if( lineMarkP   != nullptr ) free(lineMarkP) ;
 if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Polygonising Zero Slope Triangles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Polygonising Zero Slope Triangles Error") ;
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
int bcdtmDrainage_markPointsInternalToZeroSlopePolygonsDtmObject
(
 BC_DTM_OBJ *dtmP,                                 // ==> Pointer To DTM Object
 DTMZeroSlopePolygonVector& zeroSlopePolygons,     // ==> Pointer To Zero Slope Polygons Vector
 int  **pointIndexPP                               // <== Point Index To Surrounding Zero Slope Polygon
)
//
//  This Function Marks Internal Connected Points At The Zero Slope Polygon Elevation
//  Within The Zero Slope Polygon. The Order The Zero Slope Polygons Are Processed Is Important.
//  This Function Assumes the Surrounding Zero Slope Polygons Come First. This order
//  Is Inherent With The Manner In Which The Zero Slope Polygons Are Created
//  by Function "bcdtmDrainage_polygoniseZeroSlopeTrianglesDtmObject"
{

 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 int    *intP ;
 long   clc,pnt,listPnt,firstPnt,lastPnt,nextPnt,priorPnt,startPnt,stopPnt ;
 long   numMarked=0,totalNumMarked=0,numZeroSlopePolygons=0,maxNumMarked=-1,maxZeroSlopePolygon=0 ;
 double area,elevation ;
 bool   internalTo=false ;
 DTMDirection  direction ;
 DTMZeroSlopePolygonVector::iterator zsp ;

// Log Entry Parameters

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Marking Points Internal To Zero Slope Polygons") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePolygons = %p",zeroSlopePolygons) ;
    bcdtmWrite_message(0,0,0,"*pointIndexPP     = %p",*pointIndexPP) ;
   }

// Initialise

 if( *pointIndexPP != nullptr )
   {
    free(*pointIndexPP) ;
    *pointIndexPP = nullptr ;
   }
 numZeroSlopePolygons = (long)( zeroSlopePolygons.end() - zeroSlopePolygons.begin()) ;

// Allocate Memory For Point Index

 *pointIndexPP = ( int * ) malloc( dtmP->numPoints * sizeof(int)) ;
 if( *pointIndexPP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( intP = *pointIndexPP ; intP < *pointIndexPP + dtmP->numPoints ; ++intP )
   {
    *intP = dtmP->nullPnt ;
   }

//  Scan The Zero Slope Polygons

 for( zsp =  zeroSlopePolygons.begin() ; zsp != zeroSlopePolygons.end() ; ++zsp )
   {

    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Zero Slope Polygon %8ld of %8ld",(long)(zsp-zeroSlopePolygons.begin()),numZeroSlopePolygons) ;

//  Copy Point List To Tptr List

    if( bcdtmDrainageList_copyPointListToTptrListDtmObject(dtmP,zsp->pointList,&startPnt)) goto errexit ;

//  Check Connectivity Of Tptr Polygon

    if( cdbg )
      {
       if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0))
         {
          bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
          goto errexit ;
         }
      if( cdbg == 2 )
        {
         if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction)) goto errexit ;
         bcdtmWrite_message(0,0,0,"Tptr Polygon ** Area = %15.5lf Direction = %2ld",area,direction) ;
        }
     }

//  Mark All Connected Points At The Zero Slope Polygon Elevation Internal To The Zero Slope Polygon

    if( zsp->direction == DTMDirection::AntiClockwise )
      {
       numMarked = 0 ;
       elevation = pointAddrP(dtmP,startPnt)->z ;
       firstPnt  = lastPnt = dtmP->nullPnt ;

//     Scan Tptr Polygon And Mark Immediately Internal Points And Create Internal Tptr List

       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Marking Points Immediately Internal To Tptr Polygon") ;
       priorPnt = startPnt ;
       pnt = nodeAddrP(dtmP,startPnt)->tPtr ;
       do
         {
          listPnt = nextPnt = nodeAddrP(dtmP,pnt)->tPtr ;
          if(( listPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,listPnt)) < 0 ) goto errexit ;

//        Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point

          while ( listPnt != priorPnt )
            {
             if( pointAddrP(dtmP,listPnt)->z == elevation && nodeAddrP(dtmP,listPnt)->tPtr == dtmP->nullPnt  )
               {
                ++numMarked ;
                if( lastPnt == dtmP->nullPnt ) { firstPnt = lastPnt = listPnt ;  }
                else                           { nodeAddrP(dtmP,lastPnt)->tPtr = -(listPnt+1) ; lastPnt = listPnt ; }
                nodeAddrP(dtmP,lastPnt)->tPtr = -(lastPnt+1) ;
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"1-Marked Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld",listPnt,pointAddrP(dtmP,listPnt)->x,pointAddrP(dtmP,listPnt)->y,pointAddrP(dtmP,listPnt)->z,pnt,firstPnt,lastPnt) ;
               }
             if(( listPnt = bcdtmList_nextAntDtmObject(dtmP,pnt,listPnt)) < 0 ) goto errexit ; ;
            }

//        Reset For Next Point On Tptr Polygon

          priorPnt = pnt ;
          pnt  = nextPnt ;
         } while ( priorPnt!= startPnt ) ;

       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numMarked = %8ld",numMarked) ;

//     Scan Internal Tptr List And Mark Points Connected To Marked Points

       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
       numMarked = 0 ;
       if( firstPnt != dtmP->nullPnt )
         {
          pnt = firstPnt ;
          do
            {
             stopPnt = pnt ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld",pnt) ;
             clc = nodeAddrP(dtmP,pnt)->cPtr ;
             while( clc != dtmP->nullPtr )
               {
                listPnt  = clistAddrP(dtmP,clc)->pntNum ;
                clc      = clistAddrP(dtmP,clc)->nextPtr ;
                if( pointAddrP(dtmP,listPnt)->z == elevation && nodeAddrP(dtmP,listPnt)->tPtr == dtmP->nullPnt )
                  {
                   ++numMarked ;
                   nodeAddrP(dtmP,lastPnt)->tPtr = -(listPnt+1) ;
                   lastPnt = listPnt ;
                   nodeAddrP(dtmP,lastPnt)->tPtr = -(lastPnt+1) ;
                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"2-Marked Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** firstPnt = %9ld lastPnt = %9ld",listPnt,pointAddrP(dtmP,listPnt)->x,pointAddrP(dtmP,listPnt)->y,pointAddrP(dtmP,listPnt)->z,pnt,firstPnt,lastPnt) ;
                  }
               }
             pnt = -(nodeAddrP(dtmP,pnt)->tPtr+1) ;
            } while ( pnt != stopPnt  ) ;
         }

       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numMarked = %8ld",numMarked) ;

//     Mark And Null Out Internal Tptr List

       if( firstPnt != dtmP->nullPnt )
         {
          numMarked = 0 ;
          pnt = firstPnt ;
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Marking And Nulling Out Internal Tptr List") ;
          do
            {
             stopPnt = pnt ;
             nextPnt = -(nodeAddrP(dtmP,pnt)->tPtr+1) ;
             if( *(*pointIndexPP+pnt) == dtmP->nullPnt )
               {
                *(*pointIndexPP+pnt) = (long)( zsp - zeroSlopePolygons.begin()) ;
                ++numMarked ;
               }
             nodeAddrP(dtmP,pnt)->tPtr = dtmP->nullPnt ;
             pnt = nextPnt ;
            } while ( pnt != stopPnt ) ;
          totalNumMarked = totalNumMarked + numMarked ;
          if( numMarked > maxNumMarked )
            {
             maxNumMarked = numMarked ;
             maxZeroSlopePolygon = (long)( zsp - zeroSlopePolygons.begin()) ;
            }
         }

       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numMarked = %8ld",numMarked) ;

//     Marked Points On Zero Slope Polygon

       pnt = startPnt  ;
       do
         {
          ++numMarked ;
          *(*pointIndexPP+pnt) = (long)( zsp - zeroSlopePolygons.begin()) ;
          pnt = nodeAddrP(dtmP,pnt)->tPtr ;
         } while( pnt != startPnt ) ;
      }

//  Null Tptr Polygon

    if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;

//  Check For None Null Tptr And Sptr Values

    if( cdbg == 2 )
      {
       int err=0 ;
       for( pnt = 0 ; pnt < dtmP->numPoints && err <= 10 ; ++pnt )
         {
          if( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt || nodeAddrP(dtmP,pnt)->sPtr != dtmP->nullPnt )
            {
             bcdtmWrite_message(1,0,0,"zeroSlopePolygon = %8ld ** pnt = %8ld pnt->tPtr = %10ld pnt->sPtr = %10ld",(int)(zsp-zeroSlopePolygons.begin()),pnt,nodeAddrP(dtmP,pnt)->tPtr,nodeAddrP(dtmP,pnt)->sPtr ) ;
             ++err ;
            }
         }
       if( err ) goto errexit ;
      }
   }


//  Check For None Null Tptr And Sptr Values

 if( cdbg == 2 )
   {
    int err=0 ;
    for( pnt = 0 ; pnt < dtmP->numPoints && err <= 10 ; ++pnt )
      {
       if( nodeAddrP(dtmP,pnt)->tPtr != dtmP->nullPnt || nodeAddrP(dtmP,pnt)->sPtr != dtmP->nullPnt )
         {
          bcdtmWrite_message(1,0,0,"pnt = %8ld pnt->tPtr = %10ld pnt->sPtr = %10ld",pnt,nodeAddrP(dtmP,pnt)->tPtr,nodeAddrP(dtmP,pnt)->sPtr ) ;
          ++err ;
         }
      }
    if( err ) goto errexit ;
   }

// Log Number Of Internal Points Marked

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Internal Points Marked = %10ld",totalNumMarked) ;
    bcdtmWrite_message(0,0,0,"maxNumMarked = %10ld ** maxZeroSlopePolygon = %8ld",maxNumMarked,maxZeroSlopePolygon) ;
   }


//  Report And Set To Null None Null Tptr Values

 if( cdbg )
   {
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
   }

// Clean Up

 cleanup :

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Zero Slope Polygons Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Zero Slope Polygons Error") ;
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
int bcdtmDrainage_indexAllPointsAtZeroSlopeElevationDtmObject
(
 BC_DTM_OBJ                 *dtmP,                 // ==> Pointer To DTM Object
 DTMZeroSlopePolygonVector  *zeroSlopePolygonsP,   // ==> Pointer To Zero Slope Polygons Vector
 int                        *pointIndexP,          // ==> Point Index To Surrounding Zero Slope Polygon
 int                        zeroSlopePolygon,      // ==> Pond Index
 int                        priorPoint,            // ==> Prior Point To Pond Exit Point
 int                        exitPoint,             // ==> Pond Exit Point
 int                        nextPoint              // ==> Next Point From Pond Exit Point
)
//
//  This Function Indexes All Points At The Zero Slope Polygon
//  Elevation To The Zero Slope Polygon
//
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   clc,pnt,nextPnt,listPnt,firstPnt,lastPnt,stopPnt ;
 long   numMarked=0,numZeroSlopePolygonsMarked=0 ;
 long   ppnt,npnt,islandFeature,voidFeature ;
 double elevation ;
 DTMZeroSlopePolygonVector::iterator zsp,zspBegin=zeroSlopePolygonsP->begin() ;

// Log Entry Parameters

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Indexing All Points At Zero Slope Polygon Elevation") ;
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePolygonsP = %p",zeroSlopePolygonsP) ;
    bcdtmWrite_message(0,0,0,"pointIndexP        = %p",pointIndexP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopePolygon   = %8ld",zeroSlopePolygon) ;
    bcdtmWrite_message(0,0,0,"priorPoint         = %8ld",priorPoint) ;
    bcdtmWrite_message(0,0,0,"exitPoint          = %8ld",exitPoint) ;
    bcdtmWrite_message(0,0,0,"nextPoint          = %8ld",nextPoint) ;
   }

// Initialise

 firstPnt = lastPnt = dtmP->nullPnt ;

// Mark Zero Slope Polygon Booundary

 zsp = zspBegin + zeroSlopePolygon ;
 for( int n = 0 ; n < zsp->pointList.numPoints - 1 ; ++n )
   {
    pnt = zsp->pointList.pointsP[n] ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Marking Point %6ld ** %12.5lf %12.5lf %10.4lf",pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
    if( lastPnt == dtmP->nullPnt )
      {
       firstPnt = lastPnt = pnt ;
      }
    else
      {
       nodeAddrP(dtmP,lastPnt)->sPtr = -(pnt+1) ;
       lastPnt = pnt ;
      }
    nodeAddrP(dtmP,lastPnt)->sPtr = -(lastPnt+1) ;
    ++numMarked ;
   }

// Check Points Have Been Marked

 if( dbg ) bcdtmWrite_message(0,0,0,"00 ** numMarked = %8ld",numMarked) ;
 if( numMarked == 0 )
   {
    bcdtmWrite_message(1,0,0,"No Points On Zero Slope Polygon Boundary") ;
    goto errexit ;
   }

// Mark Points Connected To Marked Points

 if( firstPnt != dtmP->nullPnt )
   {
    pnt = firstPnt ;
    elevation = pointAddrP(dtmP,firstPnt)->z ;
    do
      {
       stopPnt = pnt ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld ** %12.5lf %12.5lf %10.4lf",pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;

//     Check For Point Not On A Void Or Island Hull

       if( ! bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,pnt))
         {
          clc = nodeAddrP(dtmP,pnt)->cPtr ;
          while( clc != dtmP->nullPtr )
            {
             listPnt  = clistAddrP(dtmP,clc)->pntNum ;
             clc      = clistAddrP(dtmP,clc)->nextPtr ;
             if( pointAddrP(dtmP,listPnt)->z == elevation && nodeAddrP(dtmP,listPnt)->sPtr == dtmP->nullPnt )
               {
                ++numMarked ;
                nodeAddrP(dtmP,lastPnt)->sPtr = -(listPnt+1) ;
                lastPnt = listPnt ;
                nodeAddrP(dtmP,lastPnt)->sPtr = -(lastPnt+1) ;
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"2-Marked Point %6ld ** %10.4lf %10.4lf %10.4lf ** firstPnt = %9ld lastPnt = %9ld",listPnt,pointAddrP(dtmP,listPnt)->x,pointAddrP(dtmP,listPnt)->y,pointAddrP(dtmP,listPnt)->z,firstPnt,lastPnt) ;
               }
            }
         }

//     Test For Point On Island Hull

       else if( bcdtmList_testForPointOnIslandHullDtmObject(dtmP,pnt) )
         {
          bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,pnt,&islandFeature) ;
          if( islandFeature != dtmP->nullPnt )
            {
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,pnt,&npnt))  goto errexit  ;
             if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,islandFeature,pnt,&ppnt)) goto errexit  ;

//           Mark Points Internal To Island Hull

             if( ( listPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,ppnt)) < 0 ) goto errexit ;
             while( listPnt != npnt )
               {
                if( pointAddrP(dtmP,listPnt)->z == elevation && nodeAddrP(dtmP,listPnt)->sPtr == dtmP->nullPnt )
                  {
                   ++numMarked ;
                   nodeAddrP(dtmP,lastPnt)->sPtr = -(listPnt+1) ;
                   lastPnt = listPnt ;
                   nodeAddrP(dtmP,lastPnt)->sPtr = -(lastPnt+1) ;
                  }
                if( ( listPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,listPnt)) < 0 ) goto errexit ;
               }
            }
         }

//     Point On Void Hull

       else
         {
          bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void,pnt,&voidFeature) ;
          if( voidFeature == dtmP->nullPnt )
            {
             bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Hole,pnt,&voidFeature) ;
            }
          if( voidFeature != dtmP->nullPnt )
            {
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,voidFeature,pnt,&npnt))  goto errexit  ;
             if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,voidFeature,pnt,&ppnt)) goto errexit  ;

//           Mark Points External To Void Hull

             if( ( listPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,npnt)) < 0 ) goto errexit ;
             while( listPnt != ppnt )
               {
                if( pointAddrP(dtmP,listPnt)->z == elevation && nodeAddrP(dtmP,listPnt)->sPtr == dtmP->nullPnt )
                  {
                   ++numMarked ;
                   nodeAddrP(dtmP,lastPnt)->sPtr = -(listPnt+1) ;
                   lastPnt = listPnt ;
                   nodeAddrP(dtmP,lastPnt)->sPtr = -(lastPnt+1) ;
                  }
                if( ( listPnt = bcdtmList_nextClkDtmObject(dtmP,pnt,listPnt)) < 0 ) goto errexit ;
               }
            }
         }

//     Set Next Point

       pnt = -(nodeAddrP(dtmP,pnt)->sPtr+1) ;
     } while ( pnt != stopPnt  ) ;
  }

 if( dbg ) bcdtmWrite_message(0,0,0,"01 ** numMarked = %8ld",numMarked) ;

// Log Points To Geopak Dat File

 if( dbg == 2 )
   {
    if( firstPnt != dtmP->nullPnt )
      {
       DPoint3d dtmPoint ;
       BC_DTM_OBJ *dataP=nullptr ;
       if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
       pnt = firstPnt ;
       do
         {
          stopPnt = pnt ;
          nextPnt = -(nodeAddrP(dtmP,pnt)->sPtr+1) ;
          dtmPoint.x = pointAddrP(dtmP,pnt)->x ;
          dtmPoint.y = pointAddrP(dtmP,pnt)->y ;
          dtmPoint.z = pointAddrP(dtmP,pnt)->z ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::RandomSpots,dataP->nullUserTag,1,&dataP->nullFeatureId,&dtmPoint,1)) goto errexit ;
          pnt = nextPnt ;
         } while ( pnt != stopPnt ) ;
       bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"internalPoints.dat") ;
       bcdtmObject_destroyDtmObject(&dataP) ;
      }
   }

//  Index Points And Null Tptr List

 if( firstPnt != dtmP->nullPnt )
   {
    pnt = firstPnt ;
    do
      {
       stopPnt = pnt ;
       nextPnt = -(nodeAddrP(dtmP,pnt)->sPtr+1) ;
       *(pointIndexP+pnt) = zeroSlopePolygon ;
       nodeAddrP(dtmP,pnt)->sPtr = dtmP->nullPnt ;
       pnt = nextPnt ;
      } while ( pnt != stopPnt ) ;
   }

// Log Number Of Zero Slope Polygons Marked

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points Marked = %8ld",numMarked) ;
    bcdtmWrite_message(0,0,0,"Number Of Zero Slope Polygons Marked = %8ld",numZeroSlopePolygonsMarked) ;
   }

// Clean Up

 cleanup :

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Indexing All Points At Zero Slope Polygon Elevation Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Indexing All Points At Zero Slope Polygon Elevation Error") ;
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
int bcdtmDrainage_writeZeroSlopePolygonPointsDtmObject
(
 BC_DTM_OBJ                 *dtmP,                 // ==> Pointer To DTM Object
 DTMZeroSlopePolygonVector  *zeroSlopePolygonsP,   // ==> Pointer To Zero Slope Polygons Vector
 int                        zeroSlopePolygon       // ==> Zero Slope Polygon Index
)
{
 int ret=DTM_SUCCESS ;
 int pnt ;
 DTMZeroSlopePolygonVector::iterator zsp = zeroSlopePolygonsP->begin() + zeroSlopePolygon ;

// Log

 bcdtmWrite_message(0,0,0,"**** Zero Slope Polygon %8ld ** direction = %2ld priorPoint = %8ld exitPoint  = %8ld nextPoint  = %8ld",zeroSlopePolygon,zsp->direction,zsp->priorPoint,zsp->exitPoint,zsp->nextPoint) ;
 bcdtmWrite_message(0,0,0,"**** Number Of Points For Zero Slope Polygon %8ld = %8ld",zeroSlopePolygon,zsp->pointList.numPoints) ;
 for( int n = 0 ; n <  zsp->pointList.numPoints ; ++n )
   {
    pnt = zsp->pointList.pointsP[n] ;
    bcdtmWrite_message(0,0,0,"Point[%8ld] = %8ld ** %12.5lf %12.5lf %10.4lf",n,pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
   }

// Return

 return(ret) ;

}
struct PondCallBackInfo
    {
    DTMFeatureCallback ContourLoadFunctionP;
    void* ContourLoadFunctionUserArgP;
    BC_DTM_OBJ* dtmP;
    };
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int  bcdtmDrainage_depressionPondCallBackFunction
(
 DTMFeatureType dtmFeatureType,
 DTMUserTag     userTag,
 DTMFeatureId   featureId,
 DPoint3d       *featurePtsP,
 size_t         numFeaturePts,
 void           *userP2
)

// Call Back Function For Depression Ponds

{
 int          ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 char         dtmFeatureTypeName[100] ;
 long         numPondPts ;
 double       area ;
 DTMDirection direction ;
 DPoint3d     *p3dP,*pondPtsP=nullptr ;
 DTMFeatureId dtmFeatureId ;
 BC_DTM_OBJ   *dtmP=nullptr ;
 PondCallBackInfo* info = (PondCallBackInfo*)userP2;

// Log Record

 if( dbg )
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"DTM Feature = %s userTag = %10I64d featureId = %10I64d featurePtsP = %p numFeaturePts = %6ld userP = %p",dtmFeatureTypeName,userTag,featureId,featurePtsP,numFeaturePts,userP2) ;
    if( dbg == 2 )
      {
       for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Pond Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }

// Store Ponds In DTM

 if( info->dtmP != nullptr && dtmFeatureType == DTMFeatureType::LowPointPond )
   {
   dtmP = (BC_DTM_OBJ *)info->dtmP;

//  Check For Closed Pond

    if( numFeaturePts >= 4 && featurePtsP->x == (featurePtsP+numFeaturePts-1)->x && featurePtsP->y == (featurePtsP+numFeaturePts-1)->y )
      {
       numPondPts = (int)numFeaturePts ;
       bcdtmUtl_copy3DTo3D(featurePtsP, numPondPts, &pondPtsP);
       bcdtmMath_getPolygonDirectionP3D(pondPtsP,numPondPts,&direction,&area) ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"CallBack ** direction = %2ld area = %12.8lf",direction,area) ;
       if (direction != DTMDirection::Unknown)
         {
         if (direction == DTMDirection::Clockwise) bcdtmMath_reversePolygonDirectionP3D (pondPtsP, numPondPts);
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::GraphicBreak,dtmP->nullUserTag,3,&dtmFeatureId,pondPtsP,numPondPts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Region,dtmP->nullUserTag,3,&dtmFeatureId,pondPtsP,numPondPts)) goto errexit ;
          if( dtmP->numFeatures % 2000 == 0 )
            {
             if( info->ContourLoadFunctionP != nullptr )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Firing Off Check Stop") ;
                if (info->ContourLoadFunctionP(DTMFeatureType::CheckStop, dtmP->nullUserTag, dtmP->nullFeatureId, nullptr, 0, info->ContourLoadFunctionUserArgP)) goto errexit;
               }
            }
         }
      }
    else if( dbg ) bcdtmWrite_message(0,0,0,"Pond Does Not Close") ;
   }

// Clean Up

 cleanup :
 if( pondPtsP != nullptr ) { free(pondPtsP) ; pondPtsP = nullptr ; }

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Depression Pond Call Back Function Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Depression Pond Call Back Function Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_createDepressionDtmObject
(
 BC_DTM_OBJ          *dtmP,               // ==> Pointer to Dtm object
 BC_DTM_OBJ          **depressionDtmPP,   // <== Depression Dtm
 DTMFeatureCallback   loadFunctionP,      // <== Call Back Function For Check Stop Purposes
 void                 *userP              // <== Pointer To User Argument For Check Stop Purposes
)

//  This Function Creates A DTM Of The Pond ( Depression ) Boundaries

{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   dtmFeature,numHullPts ;
 DPoint3d    *hullPtsP=nullptr ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTMDrainageTables *drainageTablesP=nullptr ;

// Log Entry Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Depression DTM") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"depressionDtmPP = %p",*depressionDtmPP) ;
   }

// Check If DTM Is In Tin State

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
 if( dbg ) bcdtmObject_reportStatisticsDtmObject(dtmP) ;


// Destroy Depression Dtm If It Exists

 if( *depressionDtmPP != nullptr ) if( bcdtmObject_destroyDtmObject(depressionDtmPP)) goto errexit ;

// Set Global Variables For Check Stop Purposes

 //ContourLoadFunctionP = loadFunctionP ;
 //ContourLoadFunctionUserArgP = userP ;

// Create Depression DTM

 if( bcdtmObject_createDtmObject(depressionDtmPP)) goto errexit ;

// Populate With Tin Hull

 if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(*depressionDtmPP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
 if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }

// Scan For Ponds And Store In Depression DTM

 if (dbg) bcdtmWrite_message(0, 0, 0, "Loading Ponds");
     {
     PondCallBackInfo info;
     info.ContourLoadFunctionP = loadFunctionP;
     info.ContourLoadFunctionUserArgP = userP;
     info.dtmP = *depressionDtmPP;
     if (bcdtmDrainage_determinePondsDtmObject(dtmP, drainageTablesP, (DTMFeatureCallback)bcdtmDrainage_depressionPondCallBackFunction, true, false, &info)) goto errexit;
     }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Ponds Loaded = %8ld",(*depressionDtmPP)->numFeatures) ;
 if( dbg == 1 ) bcdtmWrite_toFileDtmObject(*depressionDtmPP,L"depressions.bcdtm") ;

// Triangulate Depression DTM

 if( (*depressionDtmPP)->numFeatures )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Depression DTM") ;
    if( bcdtmObject_triangulateDtmObject(*depressionDtmPP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Hard Breaks") ;
    if( bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject(*depressionDtmPP,DTMFeatureType::Breakline)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning DTM") ;
    if( bcdtmList_cleanDtmObject(*depressionDtmPP)) goto errexit ;
    if( dbg )
      {
       for( dtmFeature = 0 ; dtmFeature < (*depressionDtmPP)->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(*depressionDtmPP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Region )
            {
             dtmFeatureP->dtmFeatureType = DTMFeatureType::Breakline ;
            }
         }
       bcdtmWrite_toFileDtmObject(*depressionDtmPP,L"depressions.bcdtm") ;
       for( dtmFeature = 0 ; dtmFeature < (*depressionDtmPP)->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(*depressionDtmPP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline )
            {
             dtmFeatureP->dtmFeatureType = DTMFeatureType::Region ;
            }
         }
      }
    if( dbg == 2 ) bcdtmObject_reportStatisticsDtmObject(*depressionDtmPP) ;

//  Check All Pond Regions Close And Are Counter Clockwise

    if( cdbg )
      {
       long pondError=0 ;
       DTMDirection direction ;
       double area=0.0 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Ponds") ;
       for( dtmFeature = 0 ; dtmFeature < (*depressionDtmPP)->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(*depressionDtmPP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Region )
            {

//           Check Region Feature Connectivity And Closure

             if( bcdtmList_checkConnectivityOfDtmFeatureDtmObject(*depressionDtmPP,dtmFeature,1))
               {
                pondError = 1 ;
               }

//           Check Region Feature Direction

             else
               {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(*depressionDtmPP,dtmFeature,&hullPtsP,&numHullPts)) goto errexit ;
                if( bcdtmMath_getPolygonDirectionP3D(hullPtsP,numHullPts,&direction,&area)) goto errexit ;
                if (direction != DTMDirection::AntiClockwise) pondError = 1;
                if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
                if( dbg ) bcdtmWrite_message(0,0,0,"Pond[%8ld] ** numPts = %8ld area = %15.5lf direction = %2ld",dtmFeature,numHullPts,area,direction) ;
               }
            }
         }

//     Check For Pond Errors

       if( pondError )
         {
          bcdtmWrite_message(2,0,0,"Error In Depression Ponds") ;
          goto errexit ;
         }
      }
   }

// No Pond Features So Destroy DTM

 else bcdtmObject_destroyDtmObject(depressionDtmPP) ;

// Clean Up

 cleanup :
 //ContourLoadFunctionP = nullptr ;
 //ContourLoadFunctionUserArgP = nullptr ;
 if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }

// Log Function Outcome

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Depression DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Depression DTM Error") ;

// Return

 return(ret) ;

// Error Exit

 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
