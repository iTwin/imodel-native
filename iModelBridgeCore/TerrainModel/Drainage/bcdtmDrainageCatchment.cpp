/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageCatchment.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcdtmDrainage.h"
#include <TerrainModel/Core/bcdtmInlines.h>

extern int DrainageDebug ;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
 int  CatchmentLineCompareFunction(const void* line1P , const void *line2P )
    {
    DTMCatchmentLine* pLine1 = ( DTMCatchmentLine* ) line1P ;
    DTMCatchmentLine* pLine2 = ( DTMCatchmentLine* ) line2P ;

    if( pLine1->startPoint  <   pLine2->startPoint ) return(-1) ;
    if( pLine1->startPoint  >   pLine2->startPoint ) return( 1) ;
    if( pLine1->startPoint  ==  pLine2->startPoint && pLine1->endPoint < pLine2->endPoint ) return( -1) ;
    if( pLine1->startPoint  ==  pLine2->startPoint && pLine1->endPoint > pLine2->endPoint ) return(  1) ;
    return(0) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
 int  CatchmentLineElevationCompareFunction(const void* line1P , const void *line2P )
    {
    DTMCatchmentLine* pLine1 = ( DTMCatchmentLine* ) line1P ;
    DTMCatchmentLine* pLine2 = ( DTMCatchmentLine* ) line2P ;

    if( pLine1->lowElevation  <   pLine2->lowElevation ) return(-1) ;
    if( pLine1->lowElevation  >   pLine2->lowElevation ) return( 1) ;
    return(0) ;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_traceCatchmentForPointDtmObject
(
 BC_DTM_OBJ *dtmP,                          /* ==> Pointer To Tin Object                             */
 double     x,                              /* ==> X Coordinate Of Point                             */
 double     y,                              /* ==> Y Coordinate Of Point                             */
 double     maxPondDepth,                   /* ==> Maximum Depth Of Ponds To Flow Out Of             */
 bool*      catchmentDeterminedP,           /* <== Catchment Determined < TRUE,FALSE>                */
 long       *catchmentClosureP,             /* <== Catchment Closes < TRUE,FALSE>                    */
 DPoint3d   **catchmentPtsPP,               /* <== Catchment Points                                  */
 long       *numCatchmentPtsP,              /* <== Number Of Catchment Points                        */
 DPoint3d   *sumpPointP                     /* <== Sump Point For Which The Catchment Was Determined */
)

    // This Function Traces The Catchment For A Point And Returns The Catchment Points

    {
    int        ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long       ap, cp, p1, p2, p3, sumpPnt1 = 0, sumpPnt2 = 0, fndType;
    bool inVoid;
    long       flatTriangle,traceStartType=0,numFeatures=0,useTables=0 ;
    double     z,sumpX=0.0,sumpY=0.0,sumpZ=0.0 ;
    DPoint3d   *p3dP ;
    BC_DTM_OBJ *dataP=NULL ;

    // Log Arguments

    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Tracing Catchment For Point") ;
        bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP)  ;
        bcdtmWrite_message(0,0,0,"x                = %12.5lf",x)  ;
        bcdtmWrite_message(0,0,0,"y                = %12.5lf",y)  ;
        bcdtmWrite_message(0,0,0,"maxPondDepth     = %8.4lf",maxPondDepth)  ;
        bcdtmWrite_message(0,0,0,"catchmentPtsPP   = %p",*catchmentPtsPP) ;
        bcdtmWrite_message(0,0,0,"numCatchmentPtsP = %8ld",*numCatchmentPtsP) ;
        }

    // Initialise

    *catchmentDeterminedP = false ;
    *catchmentClosureP    = 0 ;
    *numCatchmentPtsP     = 0 ;
    if( *catchmentPtsPP != NULL )
        {
        free(*catchmentPtsPP) ;
        *catchmentPtsPP = NULL ;
        }
    sumpPointP->x = 0.0 ;
    sumpPointP->y = 0.0 ;
    sumpPointP->z = 0.0 ;
    p1 = p2 = p3 = dtmP->nullPnt ;

    // Check For Valid DTM Object

    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

    // Check For Tin State

    if( dtmP->dtmState != DTMState::Tin )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
        goto errexit ;
        }

    // Find Triangle For Point

    if( dbg ) bcdtmWrite_message(0,0,0,"Finding Triangle For Point") ;
    bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&z,&fndType,&p1,&p2,&p3)  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"fndType = %2ld",fndType) ;
    if( fndType == 0 )
        {
        bcdtmWrite_message(0,0,0,"Point External To Tin") ;
        goto errexit ;
        }

    // Check Point To Point Tolerance

    if ( bcdtmMath_distance(x,y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) <= dtmP->ppTol ) { fndType = 1 ; p2 = p3 = dtmP->nullPnt ; }
    if( p2 != dtmP->nullPnt )
        {
        if( bcdtmMath_distance(x,y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) <= dtmP->ppTol )
            {
            fndType = 1 ; p1 = p2 ; p2 = p3 = dtmP->nullPnt ;
            }
        }
    if( p3 != dtmP->nullPnt )
        {
        if( bcdtmMath_distance(x,y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y) <= dtmP->ppTol )
            {
            fndType = 1 ; p1 = p3 ; p2 = p3 = dtmP->nullPnt ;
            }
        }


    // Test For Point In Void

    if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Point In Void") ;
    inVoid = false ;
    if( fndType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD) ) inVoid = true;
    if( fndType == 2 || fndType == 3 )
        {
        if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,inVoid)) goto errexit ;
        }
    if( fndType == 4 )
        {
        if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,inVoid)) goto errexit ;
        }
    if( inVoid )
        {
        bcdtmWrite_message(0,0,0,"Start Point In Void") ;
        goto errexit ;
        }

    // Test For Triangle Edge In Flat Triangles

    if( fndType == 2 || fndType == 3 )
        {
        if( pointAddrP(dtmP,p1)->z == pointAddrP(dtmP,p2)->z )
            {
            flatTriangle = 1 ;
            if( ( ap = bcdtmList_nextAntDtmObject(dtmP,p1,p2) )   < 0 ) goto errexit ;
            if( ( cp = bcdtmList_nextClkDtmObject(dtmP,p1,p2) ) < 0 ) goto errexit ;
            if( ! bcdtmList_testLineDtmObject(dtmP,p2,ap) ) ap = dtmP->nullPnt ;
            if( ! bcdtmList_testLineDtmObject(dtmP,p2,cp) ) cp = dtmP->nullPnt ;
            if( ap != dtmP->nullPnt && pointAddrP(dtmP,p1)->z != pointAddrP(dtmP,ap)->z )
                flatTriangle = 0 ;
            if( cp != dtmP->nullPnt && pointAddrP(dtmP,p1)->z != pointAddrP(dtmP,cp)->z )
                flatTriangle = 0 ;
            if( flatTriangle )
                {
                bcdtmWrite_message(1,0,0,"Start Point In Zero Slope Triangle") ;
                goto errexit ;
                }
            }
        }

    // Test For Flat Triangle

    if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Zero Slope Triangle") ;
    if( fndType == 4  )
        {
        if( pointAddrP(dtmP,p1)->z == pointAddrP(dtmP,p2)->z && pointAddrP(dtmP,p1)->z == pointAddrP(dtmP,p3)->z )
            {
            bcdtmWrite_message(0,0,0,"Start Point In Zero Slope Triangle") ;
            goto errexit ;
            }
        }

    // Set Triangle Anti Clockwise

    if( fndType == 4 )
        {
        if( bcdtmMath_pointSideOfDtmObject(dtmP,p1,p2,p3) < 0 )
            { ap = p2 ; p2 = p3 ; p3 = ap ; }
        }

    //  Set Trace Start Type

    if ( fndType == 1 )
        traceStartType = 1 ;
    else if( fndType == 2 || fndType == 3 )
        traceStartType = 2 ;
    else if( fndType == 4 )
        traceStartType = 3 ;

    // Calculate Drainage Tables

    if( useTables )
        {
        useTables = false ;
        if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Drainage Tables") ;
        //    if( bcdtmDrainage_calculateDrainageTablesDtmObject(dtmP,maxPondDepth)) goto errexit ;
        }

    //  Trace To Low Point

    if( dbg == 2 )
        {
        if( bcdtmDrainage_traceToLowPointDtmObject(dtmP,nullptr,nullptr,0.0,1,nullptr,p1,p2,p3,x,y,z,nullptr,&sumpPnt1,&sumpPnt2)) goto errexit ;
        bcdtmWrite_message(0,0,0,"Sump Line = %9ld %9ld",sumpPnt1,sumpPnt2) ;
        if( sumpPnt1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"sumpPnt1 = %6ld sumpPnt1->hPtr = %9ld ** %10.4lf %10.4lf %10.4lf",sumpPnt1,nodeAddrP(dtmP,sumpPnt1)->hPtr,pointAddrP(dtmP,sumpPnt1)->x,pointAddrP(dtmP,sumpPnt1)->y,pointAddrP(dtmP,sumpPnt1)->z ) ;
        if( sumpPnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"sumpPnt2 = %6ld sumpPnt2->hPtr = %9ld ** %10.4lf %10.4lf %10.4lf",sumpPnt2,nodeAddrP(dtmP,sumpPnt2)->hPtr,pointAddrP(dtmP,sumpPnt2)->x,pointAddrP(dtmP,sumpPnt2)->y,pointAddrP(dtmP,sumpPnt2)->z ) ;
        }

    //  Trace To Sump Line

    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Tracing To Sump Line From ** %12.5lf %12.5lf %10.4lf",x,y,z) ;
    if( bcdtmDrainage_traceToSumpLineDtmObject(dtmP,traceStartType,p1,p2,p3,x,y,z,&sumpPnt1,&sumpPnt2,&sumpX,&sumpY,&sumpZ)) goto errexit ;
    if( dbg == 1 )
        {
        bcdtmWrite_message(0,0,0,"Sump Line = %9ld %9ld",sumpPnt1,sumpPnt2) ;
        if( sumpPnt1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"sumpPnt1 = %6ld sumpPnt1->hPtr = %9ld ** %10.4lf %10.4lf %10.4lf",sumpPnt1,nodeAddrP(dtmP,sumpPnt1)->hPtr,pointAddrP(dtmP,sumpPnt1)->x,pointAddrP(dtmP,sumpPnt1)->y,pointAddrP(dtmP,sumpPnt1)->z ) ;
        if( sumpPnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"sumpPnt2 = %6ld sumpPnt2->hPtr = %9ld ** %10.4lf %10.4lf %10.4lf",sumpPnt2,nodeAddrP(dtmP,sumpPnt2)->hPtr,pointAddrP(dtmP,sumpPnt2)->x,pointAddrP(dtmP,sumpPnt2)->y,pointAddrP(dtmP,sumpPnt2)->z ) ;
        bcdtmWrite_message(0,0,0,"sumpPoint = %12.5lf, %12.5lf, %10.4lf",sumpX,sumpY,sumpZ) ;
        }

    //  Set Sump Point For Which Catchment Will Be Determined

    sumpPointP->x = sumpX ;
    sumpPointP->y = sumpY ;
    sumpPointP->z = sumpZ ;

    //  Trace Catchment From Both Sides Of Sump Line

    if( dbg ) bcdtmWrite_message(0,0,0,"Tracing Catchment For Sump Line") ;
    if( bcdtmDrainage_traceCatchmentForSumpLineDtmObject(dtmP,sumpX,sumpY,sumpZ,sumpPnt1,sumpPnt2,maxPondDepth,useTables,catchmentPtsPP,numCatchmentPtsP)) goto errexit ;

    // Set Additional Return Arguments

    if( *numCatchmentPtsP > 0 )
        {
        *catchmentDeterminedP = true ;
        if( (*catchmentPtsPP)->x == (*catchmentPtsPP+*numCatchmentPtsP-1)->x && (*catchmentPtsPP)->y == (*catchmentPtsPP+*numCatchmentPtsP-1)->y ) *catchmentClosureP = 1 ;
        if( dbg == 2 )
            {
            if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
            if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Breakline,dataP->nullUserTag,1,&dataP->nullFeatureId,*catchmentPtsPP,*numCatchmentPtsP)) goto errexit ;
            if( bcdtmWrite_toFileDtmObject(dataP,L"catchmentPts.dtm")) goto errexit ;
            if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"catchmentPts.dat")) goto errexit ;
            }
        if( dbg == 2 )
            {
            bcdtmWrite_message(0,0,0,"Number Of Catchment Points = %8ld",*numCatchmentPtsP) ;
            for( p3dP = *catchmentPtsPP ; p3dP < *catchmentPtsPP + *numCatchmentPtsP ; ++p3dP )
                {
                bcdtmWrite_message(0,0,0,"Catchment Point[%4ld] = %12.5lf %12.5lf %12.5lf",(long)(p3dP-*catchmentPtsPP),p3dP->x,p3dP->y,p3dP->z) ;
                }
            }
        }

// Clean Up

cleanup :
    if( dataP != NULL ) bcdtmObject_destroyDtmObject(&dataP) ;
    if( bcdtmList_nullTptrValuesDtmObject(dtmP)) goto errexit ;
// bcdtmDrainage_destroyDrainageTablesDtmObject(dtmP) ;

// Return

    if( *catchmentDeterminedP ) bcdtmWrite_message(0,0,0,"Catchment Determined") ;
    else                        bcdtmWrite_message(0,0,0,"Catchment Not Determined") ;
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Catchment For Point Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Catchment For Point Error") ;
    return(ret) ;

// Error Exit

errexit :
    if( *catchmentPtsPP != NULL ) free(*catchmentPtsPP) ;
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }

    enum class SumpLineType
        {
        None = 0,
        Point = 1,// Internal SumpPoint
        PointOnHull = 2,
        PointOnVoid = 3,
        PointOnHole = 4,
        PointOnIsland = 5,

        Line = 6,
        LineOnHull = 7,
        LineOnVoid = 8,
        LineOnHole = 9,
        LineOnIsland = 10,
        DrainPoint = 11
        };
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
    int bcdtmDrainage_traceCatchmentForSumpLineDtmObject
        (
        BC_DTM_OBJ  *dtmP,
        double      sumpX,
        double      sumpY,
        double      sumpZ,
        long        sumpPnt1,
        long        sumpPnt2,
        double      maxPondDepth,
        long        useTables,
        DPoint3d    **catchmentPtsPP,
        long        *numCatchmentPtsP
        )

        // Assumes Sump Line Is Valid

        {
        int  ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
        SumpLineType sumpLineType = SumpLineType::None;
        long clPtr, antPnt = 0, clkPnt = 0, antFlow, clkFlow, dtmFeature, hullPnt;

        // Log Parameters

        if (dbg)
            {
            bcdtmWrite_message (0, 0, 0, "Tracing Catchment For Sump Line");
            bcdtmWrite_message (0, 0, 0, "dtmP             = %p", dtmP);
            bcdtmWrite_message (0, 0, 0, "sumpX            = %15.5lf", sumpX);
            bcdtmWrite_message (0, 0, 0, "sumpY            = %15.5lf", sumpY);
            bcdtmWrite_message (0, 0, 0, "sumpZ            = %15.5lf", sumpZ);
            bcdtmWrite_message (0, 0, 0, "sumpPnt1         = %8ld", sumpPnt1);
            bcdtmWrite_message (0, 0, 0, "sumpPnt2         = %8ld", sumpPnt2);
            bcdtmWrite_message (0, 0, 0, "maxPondDepth     = %8.3lf", maxPondDepth);
            bcdtmWrite_message (0, 0, 0, "catchmentPtsPP   = %p", *catchmentPtsPP);
            bcdtmWrite_message (0, 0, 0, "numCatchmentPtsP = %8ld", *numCatchmentPtsP);
            }

        // Set Direction Of Sump Line So That SumPnt1 is the Lowest Point

        if (sumpPnt2 != dtmP->nullPnt)
            {
            if (pointAddrP (dtmP, sumpPnt2)->z < pointAddrP (dtmP, sumpPnt1)->z)
                {
                antPnt = sumpPnt1;
                sumpPnt1 = sumpPnt2;
                sumpPnt2 = antPnt;
                }
            }

        // Set Sump Line Type

        sumpLineType = SumpLineType::None;
        if (sumpPnt1 != dtmP->nullPnt && sumpPnt2 == dtmP->nullPnt) sumpLineType = SumpLineType::Point;  //  Sump Point
        else if (sumpPnt1 != dtmP->nullPnt && sumpPnt2 != dtmP->nullPnt) sumpLineType = SumpLineType::Line;  //  Sump Line
        if (sumpLineType == SumpLineType::None)
            {
            bcdtmWrite_message (2, 0, 0, "Invalid Sump Line [%8ld,%8ld]", sumpPnt1, sumpPnt2);
            goto errexit;
            }
        if (sumpLineType == SumpLineType::Point)
            {
            if (bcdtmList_testForPointOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Hull, sumpPnt1, &dtmFeature)) sumpLineType = SumpLineType::PointOnHull;
            else if (bcdtmList_testForPointOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Void, sumpPnt1, &dtmFeature)) sumpLineType = SumpLineType::PointOnVoid;
            else if (bcdtmList_testForPointOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Hole, sumpPnt1, &dtmFeature)) sumpLineType = SumpLineType::PointOnHole;
            else if (bcdtmList_testForPointOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Island, sumpPnt1, &dtmFeature)) sumpLineType = SumpLineType::PointOnIsland;
            }
        else if (sumpLineType == SumpLineType::Line)
            {
            if (bcdtmList_testForLineOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Hull, sumpPnt1, sumpPnt2)) sumpLineType = SumpLineType::LineOnHull;
            else if (bcdtmList_testForLineOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Void, sumpPnt1, sumpPnt2)) sumpLineType = SumpLineType::LineOnVoid;
            else if (bcdtmList_testForLineOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Hole, sumpPnt1, sumpPnt2)) sumpLineType = SumpLineType::LineOnHole;
            else if (bcdtmList_testForLineOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Island, sumpPnt1, sumpPnt2)) sumpLineType = SumpLineType::LineOnIsland;
            }

        // Log Sump Line Type

        if (dbg) bcdtmWrite_message (0, 0, 0, "sumpLineType = %4ld", sumpLineType);

        //  Validate Sump Point

        if (sumpLineType >= SumpLineType::Point && sumpLineType <= SumpLineType::PointOnIsland)
            {

            //  Check If Point Is On Zero Slope Polygon

            if (bcdtmList_testForPointOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::ZeroSlopePolygon, sumpPnt1, &dtmFeature))
                {
                if (dbg) bcdtmWrite_message (0, 0, 0, "Point On Zero Slope Polygon");
                }

            //  Check For Valid Sump Point

            if (sumpLineType == SumpLineType::Point)
                {
                clPtr = nodeAddrP (dtmP, sumpPnt1)->cPtr;
                while (clPtr != dtmP->nullPtr)
                    {
                    clkPnt = clistAddrP (dtmP, clPtr)->pntNum;
                    clPtr = clistAddrP (dtmP, clPtr)->nextPtr;
                    if (pointAddrP (dtmP, sumpPnt1)->z > pointAddrP (dtmP, clkPnt)->z)
                        {
                        bcdtmWrite_message (2, 0, 0, "Invalid Sump Point");
                        if (dbg)
                            {
                            bcdtmWrite_message (0, 0, 0, "sumpPnt1 = %9ld clkPnt = %9ld", sumpPnt1, clkPnt);
                            bcdtmList_writeCircularListForPointDtmObject (dtmP, sumpPnt1);
                            }
                        sumpLineType = SumpLineType::DrainPoint;
                        goto errexit;
                        }
                    }
                }
            }

        // Get Triangle Points Either Side Of Sump Line

        else if (sumpLineType >= SumpLineType::Line && sumpLineType <= SumpLineType::LineOnIsland)
            {
            if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, sumpPnt1, sumpPnt2)) < 0) goto errexit;
            if ((clkPnt = bcdtmList_nextClkDtmObject (dtmP, sumpPnt1, sumpPnt2)) < 0) goto errexit;
            if (dbg)
                {
                bcdtmWrite_message (0, 0, 0, "antPnt = %6ld ** %12.5lf %12.5lf %12.5lf", antPnt, pointAddrP (dtmP, antPnt)->x, pointAddrP (dtmP, antPnt)->y, pointAddrP (dtmP, antPnt)->z);
                bcdtmWrite_message (0, 0, 0, "clkPnt = %6ld ** %12.5lf %12.5lf %12.5lf", clkPnt, pointAddrP (dtmP, clkPnt)->x, pointAddrP (dtmP, clkPnt)->y, pointAddrP (dtmP, clkPnt)->z);
                }

            //  Test For Valid triangles

            if (!bcdtmList_testLineDtmObject (dtmP, sumpPnt2, antPnt)) antPnt = dtmP->nullPnt;
            if (!bcdtmList_testLineDtmObject (dtmP, sumpPnt2, clkPnt)) clkPnt = dtmP->nullPnt;
            }

        // Trace Catchment For Sump Line

        if (dbg && sumpLineType == SumpLineType::Point)
            {
            bcdtmWrite_message (0, 0, 0, "*************** Tracing From Sump Point");
            bcdtmWrite_message (0, 0, 0, "sumpX            = %15.5lf", sumpX);
            bcdtmWrite_message (0, 0, 0, "sumpY            = %15.5lf", sumpY);
            bcdtmWrite_message (0, 0, 0, "sumpZ            = %15.5lf", sumpZ);
            bcdtmWrite_message (0, 0, 0, "sumpPnt1         = %8ld", sumpPnt1);
            bcdtmWrite_message (0, 0, 0, "sumpPnt2         = %8ld", sumpPnt2);
            bcdtmWrite_message (0, 0, 0, "sumpPnt1 = %6ld ** %12.5lf %12.5lf %12.5lf", sumpPnt1, pointAddrP (dtmP, sumpPnt1)->x, pointAddrP (dtmP, antPnt)->y, pointAddrP (dtmP, sumpPnt1)->z);
            }
        if (sumpLineType == SumpLineType::Point) sumpLineType = SumpLineType::DrainPoint;


        switch (sumpLineType)
            {
            case  SumpLineType::Point:                  // Internal Sump Point
                if (dbg) bcdtmWrite_message (0, 0, 0, "Creating Catchment From Internal Sump Point");
                /*
                **    Test Point Is An Internal Sump Point
                */
                if (bcdtmList_testForHullPointDtmObject (dtmP, sumpPnt1, &hullPnt)) goto errexit;
                if (hullPnt)
                    {
                    bcdtmWrite_message (2, 0, 0, "Not An Internal Sump Point");
                    goto errexit;
                    }
                /*
                **    Trace Catchment From Internal Sump Point
                */
                if (bcdtmDrainage_traceCatchmentFromInternalSumpPointDtmObject (dtmP, sumpX, sumpY, sumpZ, sumpPnt1, useTables, catchmentPtsPP, numCatchmentPtsP)) goto errexit;
                break;

            case  SumpLineType::PointOnHull:                  // Sump Point On Tin Hull
                if (dbg) bcdtmWrite_message (0, 0, 0, "Catchment From Tin Hull Sump Point Not Implemented");
                break;

            case  SumpLineType::PointOnVoid:                  // Sump Point On Void Hull
                bcdtmWrite_message (0, 0, 0, "Catchment From Void Hull Sump Point Not Implemented");
                break;

            case  SumpLineType::PointOnHole:                  // Sump Point On Hole Hull
                bcdtmWrite_message (0, 0, 0, "Catchment From Island Hull Sump Point Not Implemented");
                break;

            case  SumpLineType::PointOnIsland:                  // Sump Point On Island Hull
                bcdtmWrite_message (0, 0, 0, "Catchment From Internal Sump Point Not Implemented");
                break;

            case  SumpLineType::Line:                  // Internal Sump Line
                /*
                **    Test Sump Line Is Internal
                */
                if (dbg) bcdtmWrite_message (0, 0, 0, "Creating Catchment From Internal Sump Line");
                if (dbg) bcdtmWrite_message (0, 0, 0, "Checking Sump Line Is Internal");
                if (antPnt == dtmP->nullPnt || clkPnt == dtmP->nullPnt)
                    {
                    bcdtmWrite_message (2, 0, 0, "Not An Internal Sump Line");
                    bcdtmWrite_message (0, 0, 0, "sumpPnt1 = %8ld hPtr = %9ld ** %12.5lf %12.5lf %10.4lf", sumpPnt1, nodeAddrP (dtmP, sumpPnt1)->hPtr, pointAddrP (dtmP, sumpPnt1)->x, pointAddrP (dtmP, sumpPnt1)->y, pointAddrP (dtmP, sumpPnt1)->z);
                    bcdtmWrite_message (0, 0, 0, "sumpPnt2 = %8ld hPtr = %9ld ** %12.5lf %12.5lf %10.4lf", sumpPnt2, nodeAddrP (dtmP, sumpPnt2)->hPtr, pointAddrP (dtmP, sumpPnt2)->x, pointAddrP (dtmP, sumpPnt2)->y, pointAddrP (dtmP, sumpPnt2)->z);
                    goto errexit;
                    }
                /*
                **    Test Flow Directions From Sump Line
                */
                if (dbg) bcdtmWrite_message (0, 0, 0, "Checking Flow Directions From Internal Sump Line");
                antFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject (dtmP, sumpPnt1, sumpPnt2, antPnt);
                clkFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject (dtmP, sumpPnt1, sumpPnt2, clkPnt);
                if (antFlow < 0 || clkFlow < 0)
                    {
                    bcdtmWrite_message (2, 0, 0, "Illegal Flow Direction For Internal Sump Line ** ccwFlow = %2ld clkFlow = %2ld", antFlow, clkFlow);
                    goto errexit;
                    }
                /*
                **    Trace Catchment From Point On Sump Line
                */
                if (bcdtmDrainage_traceCatchmentFromPointOnInternalSumpLineDtmObject (dtmP, sumpX, sumpY, sumpZ, sumpPnt1, sumpPnt2, maxPondDepth, useTables, catchmentPtsPP, numCatchmentPtsP)) goto errexit;
                break;

            case  SumpLineType::LineOnHull:                  // Sump Line On Tin Hull
                if (dbg) bcdtmWrite_message (0, 0, 0, "Catchment From Tin Hull Sump Line Not Implemented");
                break;

            case  SumpLineType::LineOnVoid:                  // Sump Line On Void Hull
                bcdtmWrite_message (0, 0, 0, "Catchment From Void Hull Sump Line Not Implemented");
                break;

            case  SumpLineType::LineOnHole:                  // Sump Line On Hole Hull
                bcdtmWrite_message (0, 0, 0, "Catchment From Hole Hull Sump Line Not Implemented");
                break;

            case  SumpLineType::LineOnIsland:                 // Sump Line On Island Hull
                bcdtmWrite_message (0, 0, 0, "Catchment From Island Hull Sump Line Not Implemented");
                break;

            case  SumpLineType::DrainPoint:                 // A Drain Point Not A Sump Point
                if (dbg) bcdtmWrite_message (0, 0, 0, "Creating Catchment From An Internal Drain Point");
                if (bcdtmDrainage_traceCatchmentFromPointOnInternalSumpLineDtmObject (dtmP, sumpX, sumpY, sumpZ, sumpPnt1, sumpPnt2, maxPondDepth, useTables, catchmentPtsPP, numCatchmentPtsP)) goto errexit;
                break;

            default:
                break;

            };

        /*
        ** Clean Up
        */
    cleanup:
        /*
        ** Job Completed
        */
        if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Tracing Catchment For Sump Line Completed");
        if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Tracing Catchment For Sump Line Error");
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
int bcdtmDrainage_traceCatchmentFromPointOnInternalSumpLineDtmObject
(
 BC_DTM_OBJ   *dtmP,
 double       sumpX,
 double       sumpY,
 double       sumpZ,
 long         sumpPoint1,
 long         sumpPoint2,
 double       maxPondDepth,
 long         useTables,
 DPoint3d     **catchmentPtsPP,
 long         *numCatchmentPtsP
)

// This Function Traces The Catchment From A Point On An Internal Sump Line
// It Assumes The Internal Sump Line Has Been Totally Validated

    {
    int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
    long   n,process,numPntList ;
    long   antPoint,clkPoint,startPoint,scanPoint,nextPoint,listPoint ;
    double x,y,z ;
    DPoint3d    tracePoints[4],*tempPtsP=nullptr ;
    enum class SumpType
        {
        Point = 1, Line = 2
        } sumpType;
    long   *pntListP = NULL, saveSumpPoint1, saveSumpPoint2;
    long   chkPoint,startPointOnPolygon,numTptrPoints ;
    double area,lastArea=0.0 ;
    DTMDirection direction ;
    DTMDrainageTables *drainageTablesP=nullptr ;
    bool  traceOverZeroSlope=false,traceToSump ;
    BC_DTM_OBJ *tempDtmP=NULL ;

    // Log Arguments

    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Tracing Catchment From Internal Sump Line") ;
        bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP)  ;
        bcdtmWrite_message(0,0,0,"sumpX            = %12.5lf",sumpX)  ;
        bcdtmWrite_message(0,0,0,"sumpY            = %12.5lf",sumpY)  ;
        bcdtmWrite_message(0,0,0,"sumpZ            = %12.5lf",sumpZ)  ;
        bcdtmWrite_message(0,0,0,"sumpPoint1       = %8ld",sumpPoint1) ;
        bcdtmWrite_message(0,0,0,"sumpPoint2       = %8ld",sumpPoint2) ;
        bcdtmWrite_message(0,0,0,"maxPondDepth     = %8.3lf",maxPondDepth) ;
        bcdtmWrite_message(0,0,0,"catchmentPtsPP   = %p",*catchmentPtsPP) ;
        bcdtmWrite_message(0,0,0,"numCatchmentPtsP = %8ld",*numCatchmentPtsP) ;
        }

    // Determine Sump Type ( Point Or Line )

    sumpType = SumpType::Point;
    if (sumpPoint2 != dtmP->nullPnt) sumpType = SumpType::Line;

    // Find Low Point From Sump Line

    saveSumpPoint1 = sumpPoint1 ;
    saveSumpPoint2 = sumpPoint2 ;
    if (sumpType == SumpType::Line && pointAddrP (dtmP, sumpPoint1)->z != pointAddrP (dtmP, sumpPoint2)->z)
        {
        if( bcdtmDrainage_traceToLowPointDtmObject(dtmP,nullptr,nullptr,0.0,0,false,sumpPoint1,sumpPoint2,dtmP->nullPnt,sumpX,sumpY,sumpZ,nullptr,&sumpPoint1,&sumpPoint2)) goto errexit ;
        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Low Point From Sump = %10ld ** %10ld",sumpPoint1,sumpPoint2) ;
        sumpType = SumpType::Point;
        if (sumpPoint2 != dtmP->nullPnt) sumpType = SumpType::Line;
        }

    // Place Tptr Polygon Around Point

    if (sumpType == SumpType::Point)
        {
        if( bcdtmList_insertTptrPolygonAroundPointDtmObject(dtmP,sumpPoint1,&startPoint)) goto errexit ;
        }

    // Place Tptr Polygon Around Sump Line

    else if (sumpType == SumpType::Line)
        {
        if( ( antPoint = bcdtmList_nextAntDtmObject(dtmP,sumpPoint1,sumpPoint2)) < 0 ) goto errexit ;
        if( ( clkPoint = bcdtmList_nextClkDtmObject(dtmP,sumpPoint1,sumpPoint2)) < 0 ) goto errexit ;
        if( dbg ) bcdtmWrite_message(0,0,0,"antPoint = %8ld clkPoint = %8ld",antPoint,clkPoint) ;
        nodeAddrP(dtmP,sumpPoint1)->tPtr = clkPoint   ;
        nodeAddrP(dtmP,clkPoint)->tPtr   = sumpPoint2 ;
        nodeAddrP(dtmP,sumpPoint2)->tPtr = antPoint   ;
        nodeAddrP(dtmP,antPoint)->tPtr   = sumpPoint1 ;
        startPoint = sumpPoint1 ;
        }

    // Log Tptr Polygon

    if( dbg == 2 )
        {
        bcdtmList_writeTptrListDtmObject(dtmP,startPoint) ;
        }

    // Scan Around Tptr Polygon And Include Triangles That Flow To Sump

    process = 1 ;
    while( process )
       {
       process = 0 ;
       scanPoint = startPoint ;
       if( dbg == 2) bcdtmWrite_message(0,0,0,"********** startPoint = %8ld",startPoint) ;

       //  Check Connectivity And Direction Of Tptr Polgon

       if( cdbg )
           {
           if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,scanPoint,0)) goto errexit ;
           bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction) ;
           if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Area = %10.3lf ** Direction = %2ld",area,direction) ;
           if (direction == DTMDirection::Clockwise)
               {
               bcdtmWrite_message(1,0,0,"Tptr Polygon Changes Direction") ;
               goto errexit ;
               }
           }

        //  Scan And Expand Tptr Polygon

        if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Expanding Tptr Polygon") ;
        do
            {
            nextPoint = nodeAddrP(dtmP,scanPoint)->tPtr ;
            if( ( listPoint = bcdtmList_nextClkDtmObject(dtmP,scanPoint,nextPoint)) < 0 ) goto errexit ;
            if( dbg == 2 )bcdtmWrite_message(0,0,0,"startPoint = %8ld scanPoint = %8ld listPoint = %8ld nextPoint = %8ld",startPoint,scanPoint,listPoint,nextPoint) ;
            if( dbg == 2 )
                {
                bcdtmWrite_message(0,0,0,"******* Triangle = %8ld %8ld %8ld",scanPoint,nextPoint,listPoint) ;
                bcdtmWrite_message(0,0,0,"scanPoint = %8ld ** %12.5lf %12.5lf %10.4lf",scanPoint,pointAddrP(dtmP,scanPoint)->x,pointAddrP(dtmP,scanPoint)->y,pointAddrP(dtmP,scanPoint)->z) ;
                bcdtmWrite_message(0,0,0,"nextPoint = %8ld ** %12.5lf %12.5lf %10.4lf",nextPoint,pointAddrP(dtmP,nextPoint)->x,pointAddrP(dtmP,nextPoint)->y,pointAddrP(dtmP,nextPoint)->z) ;
                bcdtmWrite_message(0,0,0,"listPoint = %8ld ** %12.5lf %12.5lf %10.4lf",listPoint,pointAddrP(dtmP,listPoint)->x,pointAddrP(dtmP,listPoint)->y,pointAddrP(dtmP,listPoint)->z) ;
                bcdtmWrite_message(0,0,0,"Centriod  = %12.5lf %12.5lf %10.4lf",(pointAddrP(dtmP,scanPoint)->x+nextPoint,pointAddrP(dtmP,nextPoint)->x+nextPoint,pointAddrP(dtmP,listPoint)->x)/3.0,
                                                                               (pointAddrP(dtmP,scanPoint)->y+nextPoint,pointAddrP(dtmP,nextPoint)->y+nextPoint,pointAddrP(dtmP,listPoint)->y)/3.0,
                                                                               (pointAddrP(dtmP,scanPoint)->z+nextPoint,pointAddrP(dtmP,nextPoint)->z+nextPoint,pointAddrP(dtmP,listPoint)->z)/3.0  ) ;
                }

            if( dbg == 2 )bcdtmWrite_message(0,0,0,"startPoint = %8ld scanPoint = %8ld listPoint = %8ld nextPoint = %8ld",startPoint,scanPoint,listPoint,nextPoint) ;
            if( nodeAddrP(dtmP,scanPoint)->hPtr != nextPoint && bcdtmList_testLineDtmObject(dtmP,nextPoint,listPoint ))
                {

                 //   Check Inclusion Of Triangle Into Tptr Polygon Will Maintain Anti Clockwise Direction

                if( nodeAddrP(dtmP,listPoint)->tPtr == dtmP->nullPnt || ( nodeAddrP(dtmP,listPoint)->tPtr != dtmP->nullPnt && ( nodeAddrP(dtmP,listPoint)->tPtr == scanPoint || nodeAddrP(dtmP,listPoint)->tPtr == nextPoint )))
                    {

                    //   Save And Clear Tptr Polygon

                    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Saving And Clearing Tptr Polygon") ;
                    if( bcdtmList_copyTptrListToPointListDtmObject(dtmP,scanPoint,&pntListP,&numPntList)) goto errexit ;
                    if( bcdtmList_nullTptrListDtmObject(dtmP,scanPoint)) goto errexit ;

                    //   Calculate Trace Points For Triangle

                    if( bcdtmDrainage_calculateTracePointsForTriangleDtmObject(dtmP,scanPoint,listPoint,nextPoint,tracePoints)) goto errexit ;

                    //  Trace To Sump

                    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Tracing To Sump") ;
                    traceToSump = 0 ;

//                    for( n = 0 ; n < 4 && ! traceToSump ; ++n )
                   for( n = 3 ; n == 3 && ! traceToSump ; ++n )
                        {
                        x = tracePoints[n].x ;
                        y = tracePoints[n].y ;
                        z = tracePoints[n].z ;
                        if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Tracing To Sump From ** %12.5lf %12.5lf %10.4lf",x,y,z) ;
                        if( bcdtmDrainage_checkTraceToSumpLineDtmObject(dtmP,drainageTablesP,1,maxPondDepth,traceOverZeroSlope,x,y,z,sumpX,sumpY,sumpZ,sumpPoint1,sumpPoint2,traceToSump)) goto errexit ;
                        }

                    // Log Trace Result

                    if( dbg == 2 )
                        {
                        if( traceToSump ) bcdtmWrite_message(0,0,0,"Traced To Sump") ;
                        else              bcdtmWrite_message(0,0,0,"Did Not Trace To Sump") ;
                        }

                    // Re Create Tptr Polygon

                    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Re Creating Tptr Polygon") ;
                    bcdtmList_copyPointListToTptrListDtmObject(dtmP,pntListP,numPntList,&scanPoint) ;
                    if( pntListP != NULL ) { free(pntListP) ; pntListP = NULL ; }

                    //  Add Triangle To Tptr Polygon Representing Catchment

                    if (traceToSump)
                        {
                        if (dbg == 2)
                            {
                            bcdtmWrite_message (0, 0, 0, "Adding Triangle To Catchment ** %8ld %8ld %8ld", scanPoint, listPoint, nextPoint);
                            bcdtmWrite_message (0, 0, 0, "startPoint = %8ld startPoint->tPtr = %8ld", startPoint, nodeAddrP (dtmP, startPoint)->tPtr);
                            bcdtmWrite_message (0, 0, 0, "scanPoint  = %8ld scanPoint->tPtr  = %8ld", scanPoint, nodeAddrP (dtmP, scanPoint)->tPtr);
                            bcdtmWrite_message (0, 0, 0, "listPoint  = %8ld listPoint->tPtr  = %8ld", listPoint, nodeAddrP (dtmP, listPoint)->tPtr);
                            bcdtmWrite_message (0, 0, 0, "nextPoint  = %8ld nextPoint->tPtr  = %8ld", nextPoint, nodeAddrP (dtmP, nextPoint)->tPtr);
                            }
                        process = 1;


                        if (nodeAddrP (dtmP, listPoint)->tPtr == scanPoint)
                            {
                            if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Condition 1");
                            if (dbg == 2) bcdtmList_writeTptrListDtmObject (dtmP, startPoint);
                            nodeAddrP (dtmP, listPoint)->tPtr = nextPoint;
                            nodeAddrP (dtmP, scanPoint)->tPtr = dtmP->nullPnt;
                            if (startPoint == scanPoint) startPoint = listPoint;
                            if (cdbg) if (bcdtmList_checkConnectivityTptrPolygonDtmObject (dtmP, startPoint, 0)) goto errexit;
                            }
                        else if (nodeAddrP (dtmP, nextPoint)->tPtr == listPoint)
                            {
                            if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Condition 2");
                            nodeAddrP (dtmP, scanPoint)->tPtr = listPoint;
                            nodeAddrP (dtmP, nextPoint)->tPtr = dtmP->nullPnt;
                            if (startPoint == nextPoint) startPoint = scanPoint;
                            nextPoint = scanPoint;
                            if (cdbg) if (bcdtmList_checkConnectivityTptrPolygonDtmObject (dtmP, startPoint, 0)) goto errexit;
                            }
                        else
                            {
                            if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Condition 3");
                            nodeAddrP (dtmP, scanPoint)->tPtr = listPoint;
                            nodeAddrP (dtmP, listPoint)->tPtr = nextPoint;
                            if (cdbg) if (bcdtmList_checkConnectivityTptrPolygonDtmObject (dtmP, startPoint, 0)) goto errexit;
                            }

                        //   Check Tptr Polygon Start Point is On Tptr Polygon

                        if( cdbg )
                            {
                            numTptrPoints = 0 ;
                            startPointOnPolygon = 0 ;
                            chkPoint = listPoint ;
                            if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Checking Start Point %8ld Is On Tptr Polygon",startPoint) ;
                            do
                               {
                               ++numTptrPoints ;
                               if( dbg == 2 ) bcdtmWrite_message(0,0,0,"chkPoint = %8ld listPoint =%8ld",chkPoint,listPoint) ;
                               if( chkPoint == startPoint ) startPointOnPolygon = 1 ;
                               chkPoint = nodeAddrP(dtmP,chkPoint)->tPtr ;
                               } while( chkPoint != listPoint && ! startPointOnPolygon && numTptrPoints < 1000 ) ;
                            if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** chkPoint = %8ld listPoint =%8ld",chkPoint,listPoint) ;
                            if( ! startPointOnPolygon )
                                {
                                bcdtmWrite_message(0,0,0,"startPoint = %8ld listPoint = %8ld startPointOnPolygon = %2ld numTptrPoints = %8ld",startPoint,listPoint,startPointOnPolygon,numTptrPoints) ;
                                goto errexit ;
                                }
                            }

                        //  Check Direction Of Tptr Polygon Has Not Changed To Clockwise

                        if( cdbg )
                            {
                            bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction) ;
                            if( dbg == 2 ) bcdtmWrite_message(0,0,0," Area = %10.5lf ** Direction = %2ld",area,direction) ;
                            if (direction == DTMDirection::Clockwise)
                                {
                                bcdtmWrite_message(0,0,0,"Tptr Polygon Changes Direction") ;
                                goto errexit ;
                                }
                            if( area < lastArea )
                                {
                                bcdtmWrite_message(0,0,0,"Tptr Polygon area is smaller") ;
                                goto errexit ;
                            }

                        lastArea = area ;
                        }
                    }
                }
            }

        //  Reset Tptr Polygon Points

        scanPoint  = nextPoint ;
        nextPoint  = nodeAddrP(dtmP,scanPoint)->tPtr ;

        //  Check For End Of Scan

        } while ( scanPoint != startPoint )  ;

     // Reset Start Point

     startPoint = nextPoint ;
     }

    //  Log Tptr Polygon Points And Save To File

    if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,startPoint) ;
    if( dbg == 1 )
        {
        if( tempDtmP != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
        if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
        if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPoint,&tempPtsP,&n)) goto errexit ;
        if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,tempPtsP,n)) goto errexit ;
        if( bcdtmWrite_geopakDatFileFromDtmObject(tempDtmP,L"unrefinedPointCatchment.dat")) goto errexit ;
        }

    // Refine Catchment Boundary

    if( dbg ) bcdtmWrite_message(0,0,0,"Refining Tptr Catchment") ;
    sumpPoint1 = saveSumpPoint1 ;
    sumpPoint2 = saveSumpPoint2 ;
    if( bcdtmDrainage_refineTptrCatchmentPolygonDtmObject(dtmP,sumpX,sumpY,sumpZ,sumpPoint1,sumpPoint2,startPoint,maxPondDepth,catchmentPtsPP,numCatchmentPtsP)) goto errexit ;
    if( dbg == 1 )
        {
        bcdtmWrite_message(0,0,0,"Number Of Catchment Points = %8ld",*numCatchmentPtsP) ;
        if( *numCatchmentPtsP > 0 )
            {
            if( tempDtmP != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
            if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
            if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,*catchmentPtsPP,*numCatchmentPtsP)) goto errexit ;
            bcdtmWrite_geopakDatFileFromDtmObject(tempDtmP,L"refinedPointCatchment.dat") ;
            }
        }


    // Null Tptr List

    bcdtmList_nullTptrListDtmObject(dtmP,startPoint) ;

    // Clean Up

 cleanup :

    if( tempPtsP != nullptr ) free(tempPtsP) ;
    if( tempDtmP != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
    bcdtmList_nullSptrValuesDtmObject(dtmP) ;

    // Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Catchment From Internal Sump Line Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Catchment From Internal Sump Line Error") ;
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
int bcdtmDrainage_calculateTracePointsForTriangleDtmObject
(
 BC_DTM_OBJ *dtmP,
 long pnt1,
 long pnt2,
 long pnt3,
 DPoint3d tracePoints[]
 )
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 double x1,y1,z1,x2,y2,z2,x3,y3,z3,angle,pptol ;
 DTM_TIN_POINT *pntP ;
/*
** Write Trace Triangle
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Trace Triangle") ;
    bcdtmWrite_message(0,0,0,"pnt1 = %8ld pnt1->tPtr = %10ld ** %12.5lf %12.5lf %12.5lf",pnt1,nodeAddrP(dtmP,pnt1)->tPtr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
    bcdtmWrite_message(0,0,0,"pnt2 = %8ld pnt2->tPtr = %10ld ** %12.5lf %12.5lf %12.5lf",pnt2,nodeAddrP(dtmP,pnt2)->tPtr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z) ;
    bcdtmWrite_message(0,0,0,"pnt3 = %8ld pnt3->tPtr = %10ld ** %12.5lf %12.5lf %12.5lf",pnt3,nodeAddrP(dtmP,pnt3)->tPtr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt3)->z) ;
   }
/*
** Initialise
*/
 pptol = dtmP->ppTol * 2.0 ;
 if( pptol == 0.0 ) pptol = 0.002 ;
 pntP = pointAddrP(dtmP,pnt1) ;
 x1 = pntP->x ;
 y1 = pntP->y ;
 z1 = pntP->z ;
 pntP = pointAddrP(dtmP,pnt2) ;
 x2 = pntP->x ;
 y2 = pntP->y ;
 z2 = pntP->z ;
 pntP = pointAddrP(dtmP,pnt3) ;
 x3 = pntP->x ;
 y3 = pntP->y ;
 z3 = pntP->z ;
/*
** Calculate Trace Point One
*/
 angle  = bcdtmMath_getAngle(x1,y1,(x2+x3)/2.0,(y2+y3)/2.0) ;
 tracePoints[0].x = x1 + pptol * cos(angle) ;
 tracePoints[0].y = y1 + pptol * sin(angle) ;
 bcdtmMath_interpolatePointOnLine(x1,y1,z1,(x2+x3)/2.0,(y2+y3)/2.0,(z2+z3)/2.0,tracePoints[0].x,tracePoints[0].y,&tracePoints[0].z ) ;
/*
** Calculate Trace Point Two
*/
 angle = bcdtmMath_getAngle(x2,y2,(x1+x3)/2.0,(y1+y3)/2.0) ;
 tracePoints[1].x = x2 + pptol * cos(angle) ;
 tracePoints[1].y = y2 + pptol * sin(angle) ;
 bcdtmMath_interpolatePointOnLine(x2,y2,z2,(x1+x3)/2.0,(y1+y3)/2.0,(z1+z3)/2.0,tracePoints[1].x,tracePoints[1].y,&tracePoints[1].z ) ;
/*
** Calculate Trace Point Three
*/
 angle = bcdtmMath_getAngle(x3,y3,(x1+x2)/2.0,(y1+y2)/2.0) ;
 tracePoints[2].x = x3 + pptol * cos(angle) ;
 tracePoints[2].y = y3 + pptol * sin(angle) ;
 bcdtmMath_interpolatePointOnLine(x3,y3,z3,(x1+x2)/2.0,(y1+y2)/2.0,(z1+z2)/2.0,tracePoints[2].x,tracePoints[2].y,&tracePoints[2].z ) ;
/*
** Calculate Centroid Trace Point
*/
 tracePoints[3].x = ( x1 + x2 + x3 ) / 3.0 ;
 tracePoints[3].y = ( y1 + y2 + y3 ) / 3.0 ;
 tracePoints[3].z = ( z1 + z2 + z3 ) / 3.0 ;

/*
** Return
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_refineTptrCatchmentPolygonDtmObjectOld
(
 BC_DTM_OBJ            *dtmP,
 double                sumpX,
 double                sumpY,
 double                sumpZ,
 long                  sumpPoint1,
 long                  sumpPoint2,
 long                  firstPoint,
 double                maxPondDepth,
 DPoint3d              **catchmentPtsPP,
 long                  *numCatchmentPtsP
 )
/*
** This Function Refines A Tptr Catchment Polygon
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   point,lowPoint,antPoint,clkPoint,scanPoint,nextPoint,intPoint,nxtPoint,sumpAnt,sumpClk ;
 long   onTptrPolygon,scan,traceToSump,p1,p2,p3,findType,process,canBeDeleted ;
 long   ascentTraced=0,highPoint1,highPoint2,numTracePts=0,numCatchPts,priorPoint ;
 long   clPtr,trgp1,trgp2,trgp3,fptr,danglingPoint,dtmFeature ;
 DPoint3d    *p3dP=NULL,*p3d1P,*catchPtsP=NULL,*tracePtsP=NULL,trgPts[4];
 double x,y,z,xn,yn,angP2,angP3,ascentAngle,descentAngle,ascentSlope ;
 BC_DTM_OBJ    *catchmentDtmP=NULL,*catchDtmP=NULL ;
 DTMUserTag  catchmentTag=0 ;
 DTM_TIN_POINT *pntP,*pnt1P,*pnt2P ;


/*
** Write Entry message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Refining Tptr Catchment Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"sumpX            = %12.5lf",sumpX)  ;
    bcdtmWrite_message(0,0,0,"sumpY            = %12.5lf",sumpY)  ;
    bcdtmWrite_message(0,0,0,"sumpZ            = %12.5lf",sumpZ)  ;
    bcdtmWrite_message(0,0,0,"sumpPoint1       = %8ld",sumpPoint1) ;
    bcdtmWrite_message(0,0,0,"sumpPoint2       = %8ld",sumpPoint2) ;
    bcdtmWrite_message(0,0,0,"firstPoint       = %8ld",firstPoint) ;
    bcdtmWrite_message(0,0,0,"maxPondDepth     = %8.3lf",maxPondDepth) ;
    bcdtmWrite_message(0,0,0,"catchmentPtsPP   = %p",*catchmentPtsPP) ;
    bcdtmWrite_message(0,0,0,"numCatchmentPtsP = %8ld",*numCatchmentPtsP) ;
   }
/*
** Find Lowest Point On Tptr Polygon
*/
 lowPoint = scanPoint = firstPoint ;
 do
   {
    if( pointAddrP(dtmP,scanPoint)->z <= pointAddrP(dtmP,lowPoint)->z )
      {
       lowPoint = scanPoint ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Low Point = %8ld ** %12.5lf %12.5lf %10.4lf",lowPoint,pointAddrP(dtmP,lowPoint)->x,pointAddrP(dtmP,lowPoint)->y,pointAddrP(dtmP,lowPoint)->z) ;
      }
    scanPoint = nodeAddrP(dtmP,scanPoint)->tPtr ;
   } while( scanPoint != firstPoint ) ;
/*
** Set First Point To Low Point
*/
 firstPoint = lowPoint ;
/*
** Create Dtm Object For Storing Catchment Boundary
*/
 if( bcdtmObject_createDtmObject(&catchmentDtmP)) goto errexit ;
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(catchmentDtmP,*numCatchmentPtsP*5,*numCatchmentPtsP)) goto errexit ;
/*
** Copr Tptr Polygon To Points Array
*/
 if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,firstPoint,&catchPtsP,&numCatchPts)) goto errexit ;
/*
** Store Catchment Boundary In DTM As A Series Of Break Lines
*/
 for( p3dP = catchPtsP ; p3dP < catchPtsP + numCatchPts - 1 ; ++p3dP )
   {
    if( bcdtmObject_storeDtmFeatureInDtmObject(catchmentDtmP,DTMFeatureType::Breakline,catchmentTag,1,&catchmentDtmP->nullFeatureId,p3dP,2)) goto errexit ;
   }
 ++catchmentTag ;
 if( catchPtsP != NULL ) { free(catchPtsP) ; catchPtsP = NULL ; }
/*
** Triangulate
*/
 if( bcdtmObject_triangulateDtmObject(catchmentDtmP)) goto errexit ;
/*
** Remove Triangles On The Tin Hull
*/
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(catchmentDtmP)) goto errexit ;
 if( dbg == 1 ) if( bcdtmWrite_toFileDtmObject(catchmentDtmP,L"catchment00.tin")) goto errexit ;
/*
** Fire Off Maximum Ascent From Sump Lines About Sump Point
*/
 if( sumpPoint1 != dtmP->nullPnt && sumpPoint2 == dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Firing Off Maximum Ascents From Sump Point") ;
/*
**  Fire Off Maximum Ascents In All Triangles About Point
*/
    clPtr = nodeAddrP(dtmP,sumpPoint1)->cPtr ;
    if( clPtr != dtmP->nullPtr )
      {
       if( ( trgp2 = bcdtmList_nextAntDtmObject(dtmP,sumpPoint1,clistAddrP(dtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
       while( clPtr != dtmP->nullPtr )
         {
          trgp3 = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          if( bcdtmList_testLineDtmObject(dtmP,trgp2,trgp3))
            {
             if( bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,sumpPoint1,trgp2,trgp3,&descentAngle,&ascentAngle,&ascentSlope)) goto errexit ;
             angP2 = bcdtmMath_getPointAngleDtmObject(dtmP,sumpPoint1,trgp2) ;
             angP3 = bcdtmMath_getPointAngleDtmObject(dtmP,sumpPoint1,trgp3) ;
             if( angP2 < angP3 ) angP2 += DTM_2PYE ;
             if( ascentAngle < angP3 ) ascentAngle += DTM_2PYE ;
             if( ascentAngle >= angP3 && ascentAngle <= angP2 )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"**** Firing Off Maximum Ascent From Sump Point") ;
//                if( bcdtmDrainage_traceMaximumAscentFromDtmPointBetweenDtmPointsDtmObject(dtmP,sumpPoint1,trgp2,trgp3,&ascentTraced,&highPoint1,&highPoint2,&tracePtsP,&numTracePts))  goto errexit ;
                if ( ascentTraced )if( bcdtmObject_storeDtmFeatureInDtmObject(catchmentDtmP,DTMFeatureType::Breakline,catchmentTag,1,&catchmentDtmP->nullFeatureId,tracePtsP,numTracePts)) goto errexit ;
               }
            }
          trgp2 = trgp3 ;
         }
      }
   }
/*
** Fire Off maximum Ascents From Drain Point On Sump Line
*/
 if( sumpPoint1 != dtmP->nullPnt && sumpPoint2 != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Firing Off Maximum Ascents From Sump Line") ;
    if( ( sumpAnt = bcdtmList_nextAntDtmObject(dtmP,sumpPoint1,sumpPoint2)) < 0 ) goto errexit ;
    if( ( sumpClk = bcdtmList_nextClkDtmObject(dtmP,sumpPoint1,sumpPoint2)) < 0 ) goto errexit ;
    if( ! bcdtmList_testLineDtmObject(dtmP,sumpAnt,sumpPoint2)) sumpAnt = dtmP->nullPnt ;
    if( ! bcdtmList_testLineDtmObject(dtmP,sumpClk,sumpPoint2)) sumpClk = dtmP->nullPnt ;
    if( sumpAnt != dtmP->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"**** Firing Off Maximum Ascents From Sump Line To Ant Point") ;
       if( bcdtmDrainage_traceMaximumAscentFromPointOnTriangleEdgeDtmObject(dtmP,sumpX,sumpY,sumpZ,sumpPoint1,sumpPoint2,sumpAnt,&ascentTraced,&highPoint1,&highPoint2,&tracePtsP,&numTracePts)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"ascentTraced = %2ld numTracePts = %8ld",ascentTraced,numTracePts) ;
       if ( ascentTraced )if( bcdtmObject_storeDtmFeatureInDtmObject(catchmentDtmP,DTMFeatureType::Breakline,catchmentTag,1,&catchmentDtmP->nullFeatureId,tracePtsP,numTracePts)) goto errexit ;
      }
    if( sumpClk != dtmP->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"**** Firing Off Maximum Ascents From Sump Line To Clk Point") ;
       if( bcdtmDrainage_traceMaximumAscentFromPointOnTriangleEdgeDtmObject(dtmP,sumpX,sumpY,sumpZ,sumpPoint2,sumpPoint1,sumpClk,&ascentTraced,&highPoint1,&highPoint2,&tracePtsP,&numTracePts)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"ascentTraced = %2ld numTracePts = %8ld",ascentTraced,numTracePts) ;
       if( ascentTraced ) if( bcdtmObject_storeDtmFeatureInDtmObject(catchmentDtmP,DTMFeatureType::Breakline,catchmentTag,1,&catchmentDtmP->nullFeatureId,tracePtsP,numTracePts)) goto errexit ;
      }
   }
/*
** Scan Around Tptr Polygon And Fire Off Maximum Ascents
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Firing Off Maximum Ascents From Tptr Polygon") ;
 for( scan = 0 ; scan < 2 ; ++scan )
   {
    scanPoint = firstPoint ;
    do
      {
       nextPoint = nodeAddrP(dtmP,scanPoint)->tPtr ;
       if( nodeAddrP(dtmP,nextPoint)->sPtr == dtmP->nullPnt || scanPoint == firstPoint )
         {
          if( ( antPoint = bcdtmList_nextAntDtmObject(dtmP,scanPoint,nextPoint)) < 0 ) goto errexit ;
          if( ( clkPoint = bcdtmList_nextClkDtmObject(dtmP,scanPoint,nextPoint)) < 0 ) goto errexit ;
          if( ! bcdtmList_testLineDtmObject(dtmP,antPoint,nextPoint)) antPoint = dtmP->nullPnt ;
          if( ! bcdtmList_testLineDtmObject(dtmP,clkPoint,nextPoint)) clkPoint = dtmP->nullPnt ;
          if( antPoint != dtmP->nullPnt && clkPoint != dtmP->nullPnt )
            {
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"********** Firing Off Maximum Ascent ** scanPoint = %8ld antPoint = %8ld clkPoint = %8ld",scanPoint,antPoint,clkPoint) ;
//             if( bcdtmDrainage_traceMaximumAscentFromDtmPointBetweenDtmPointsDtmObject(dtmP,scanPoint,antPoint,clkPoint,&ascentTraced,&highPoint1,&highPoint2,&tracePtsP,&numTracePts))  goto errexit ;
             if( ascentTraced )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"highPoint1 = %10ld highPoint2 = %10ld",highPoint1,highPoint2) ;
/*
**              Store maximum Ascent As A Series Of Break Lines
*/
                for( p3dP = tracePtsP ; p3dP < tracePtsP + numTracePts - 1 ; ++p3dP )
                  {
                   if( bcdtmObject_storeDtmFeatureInDtmObject(catchmentDtmP,DTMFeatureType::Breakline,catchmentTag,1,&catchmentDtmP->nullFeatureId,p3dP,2)) goto errexit ;
                  }
                ++catchmentTag ;
/*
**              Check If High Points Are On Tptr Catchment Polygon
*/
                onTptrPolygon = 0 ;
                if( nodeAddrP(dtmP,highPoint1)->tPtr != dtmP->nullPnt )
                  {
                   nodeAddrP(dtmP,highPoint1)->sPtr = scanPoint ;
                   onTptrPolygon = 1 ;
                  }
                if( highPoint2 != dtmP->nullPnt && nodeAddrP(dtmP,highPoint2)->tPtr != dtmP->nullPnt )
                  {
                   nodeAddrP(dtmP,highPoint2)->sPtr = scanPoint ;
                   onTptrPolygon = 1 ;
                  }
                if( dbg )
                  {
                   if( ! onTptrPolygon ) bcdtmWrite_message(0,0,0,"High Point Not On Tptr Polygon") ;
                   else                  bcdtmWrite_message(0,0,0,"High Point On Tptr Polygon") ;
                  }
 /*
**              Scan To High Points On Tptr Polygon
*/
                if( onTptrPolygon )
                  {
                   point = nextPoint = scanPoint ;
                   do
                     {
                      if( point == highPoint1 || point == highPoint2 ) nextPoint = point ;
                      point = nodeAddrP(dtmP,point)->tPtr ;
                     } while ( point != scanPoint )  ;
                   point = scanPoint ;
                   while ( point != nextPoint )
                     {
                      nodeAddrP(dtmP,point)->sPtr = scanPoint ;
                      point = nodeAddrP(dtmP,point)->tPtr ;
                     }
                  }
/*
**              Determine Last Intersection With Tptr Polygon
*/
                else
                  {
                   for( p3dP = tracePtsP + numTracePts - 2 ; p3dP >= tracePtsP && ! onTptrPolygon ; --p3dP )
                     {
                      p3d1P = p3dP + 1 ;
                      intPoint = scanPoint ;
                      pnt1P = pointAddrP(dtmP,intPoint) ;
                      do
                        {
                         nxtPoint = nodeAddrP(dtmP,intPoint)->tPtr ;
                         pnt2P = pointAddrP(dtmP,nxtPoint) ;
                         if( bcdtmMath_checkIfLinesIntersect(p3dP->x,p3dP->y,p3d1P->x,p3d1P->y,pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y))
                           {
                            bcdtmMath_normalIntersectCordLines(p3dP->x,p3dP->y,p3d1P->x,p3d1P->y,pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y,&xn,&yn) ;
                            if( dbg ) bcdtmWrite_message(0,0,0,"Intersection Found = %12.5lf %12.5lf",xn,yn) ;
                           }
                         intPoint = nxtPoint ;
                         pnt1P    = pnt2P ;
                        } while( intPoint != scanPoint ) ;
                     }
                  }
               }
            }
         }
       scanPoint = nextPoint ;
      }while( scanPoint != firstPoint ) ;
/*
**  Reverse Tptr Polygon
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Reversing Tptr Polygon") ;
    if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,firstPoint)) goto errexit ;
   }
/*
**  Remove Tptr Polygon
*/
 if( bcdtmList_nullTptrListDtmObject(dtmP,firstPoint)) goto errexit ;
/*
** Triangulate And Remove Triangles On The Tin Hull
*/
 if( bcdtmObject_triangulateDtmObject(catchmentDtmP)) goto errexit ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(catchmentDtmP)) goto errexit ;
 if( dbg == 1 ) if( bcdtmWrite_toFileDtmObject(catchmentDtmP,L"catchment01.tin")) goto errexit ;
/*
** Remove Dangling Ascent Lines - Those External to The Initial Catchment Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dangling Ascents") ;
 if( cdbg ) if( bcdtmCheck_tinComponentDtmObject(catchmentDtmP)) goto errexit ;
 process = 1 ;
 while ( process )
   {
    process = 0 ;
    priorPoint = catchmentDtmP->hullPoint ;
    scanPoint  = nodeAddrP(catchmentDtmP,priorPoint)->hPtr ;
    do
      {
       nextPoint = nodeAddrP(catchmentDtmP,scanPoint)->hPtr ;
       if( ! bcdtmList_testForBreakLineDtmObject(catchmentDtmP,priorPoint,scanPoint) || ! bcdtmList_testForBreakLineDtmObject(catchmentDtmP,scanPoint,nextPoint) )
         {
          danglingPoint = 1 ;
          fptr = nodeAddrP(catchmentDtmP,scanPoint)->fPtr ;
          while( fptr != catchmentDtmP->nullPtr && danglingPoint )
            {
             dtmFeature = flistAddrP(catchmentDtmP,fptr)->dtmFeature ;
             if( ftableAddrP(catchmentDtmP,dtmFeature)->dtmUserTag == 0 ) danglingPoint = 0 ;
             fptr = flistAddrP(catchmentDtmP,fptr)->nextPtr ;
            }
          if( danglingPoint )
            {
             if( bcdtmDrainage_checkPointOnTinHullCanBeDeletedDtmObject(catchmentDtmP,scanPoint,&canBeDeleted)) goto errexit ;
             if( canBeDeleted )
               {

                if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Deleting Dangling Point = %10ld ** %12.5lf %12.5lf %10.4lf",scanPoint,pointAddrP(catchmentDtmP,scanPoint)->x,pointAddrP(catchmentDtmP,scanPoint)->y,pointAddrP(catchmentDtmP,scanPoint)->z) ;
//                if( bcdtmDrainage_deletePointOnTinHullDtmObject(catchmentDtmP,scanPoint)) goto errexit ;
                scanPoint = catchmentDtmP->hullPoint ;
                process = 1 ;
                if( cdbg ) if( bcdtmCheck_tinComponentDtmObject(catchmentDtmP)) goto errexit ;
               }
            }
         }
       priorPoint = scanPoint ;
       scanPoint  = nextPoint ;
      } while( priorPoint != catchmentDtmP->hullPoint  ) ;
   }
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"01 Checking Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(catchmentDtmP))
      {
       bcdtmWrite_message(1,0,0,"Triangulation Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"01 Triangulation Valid") ;
   }
 if( dbg == 1 ) if( bcdtmWrite_toFileDtmObject(catchmentDtmP,L"catchment02.tin")) goto errexit ;
/*
**  Remove All DTM Features And Clean Tin
*/
 if( bcdtmData_deleteAllDtmFeaturesDtmObject(catchmentDtmP))goto errexit ;
 if( bcdtmList_cleanDtmObject(catchmentDtmP)) goto errexit ;
 if( dbg == 1 ) if( bcdtmWrite_toFileDtmObject(catchmentDtmP,L"catchment03.tin")) goto errexit ;
 if( cdbg ) if( bcdtmCheck_tinComponentDtmObject(catchmentDtmP)) goto errexit ;
/*
** Get Extent Of Catchment Area
**
**
**  Create DTM Object To Store Triangles That Flow To Sump Point
*/
 if( bcdtmObject_createDtmObject(&catchDtmP)) goto errexit ;
/*
**  Scan All Triangles And See If They Drain To Sump Point
*/
 for( p1 = 0 ; p1 < catchmentDtmP->numPoints ; ++p1 )
   {
    clPtr = nodeAddrP(catchmentDtmP,p1)->cPtr ;
    if( clPtr != catchmentDtmP->nullPtr )
      {
       if( ( p2 = bcdtmList_nextAntDtmObject(catchmentDtmP,p1,clistAddrP(catchmentDtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while( clPtr != catchmentDtmP->nullPtr )
         {
          p3 = clistAddrP(catchmentDtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(catchmentDtmP,clPtr)->nextPtr ;
          if( nodeAddrP(catchmentDtmP,p3)->hPtr != p1 )
            {
             pntP  = pointAddrP(catchmentDtmP,p1) ;
             pnt1P = pointAddrP(catchmentDtmP,p2) ;
             pnt2P = pointAddrP(catchmentDtmP,p3)  ;
             x = ( pntP->x + pnt1P->x + pnt2P->x ) / 3.0 ;
             y = ( pntP->y + pnt1P->y + pnt2P->y ) / 3.0 ;
             z = ( pntP->z + pnt1P->z + pnt2P->z ) / 3.0 ;
             traceToSump = 0 ;
             if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&z,&findType,&trgp1,&trgp2,&trgp3)) goto errexit ;
//             if( findType ) if( bcdtmDrainage_checkTraceToSumpLineDtmObject(dtmP,2,maxPondDepth,0,0,trgp1,trgp3,trgp2,x,y,z,sumpX,sumpY,sumpZ,sumpPoint1,sumpPoint2,&traceToSump)) goto errexit ;
             if( ! traceToSump )
               {
                if( dbg == 2 )
                  {
                   bcdtmWrite_message(0,0,0,"Triangle Does Not Trace To Sump") ;
                   bcdtmWrite_message(0,0,0,"p1 = %8ld ** %12.5lf %12.5lf %10.4lf",p1,pointAddrP(catchmentDtmP,p1)->x,pointAddrP(catchmentDtmP,p1)->y,pointAddrP(catchmentDtmP,p1)->z) ;
                   bcdtmWrite_message(0,0,0,"p2 = %8ld ** %12.5lf %12.5lf %10.4lf",p2,pointAddrP(catchmentDtmP,p2)->x,pointAddrP(catchmentDtmP,p2)->y,pointAddrP(catchmentDtmP,p2)->z) ;
                   bcdtmWrite_message(0,0,0,"p3 = %8ld ** %12.5lf %12.5lf %10.4lf",p3,pointAddrP(catchmentDtmP,p3)->x,pointAddrP(catchmentDtmP,p3)->y,pointAddrP(catchmentDtmP,p3)->z) ;
                  }
               }
             else
               {
                if( dbg == 2 )
                  {
                   bcdtmWrite_message(0,0,0,"Triangle Traces To Sump") ;
                   bcdtmWrite_message(0,0,0,"p1 = %8ld ** %12.5lf %12.5lf %10.4lf",p1,pointAddrP(catchmentDtmP,p1)->x,pointAddrP(catchmentDtmP,p1)->y,pointAddrP(catchmentDtmP,p1)->z) ;
                   bcdtmWrite_message(0,0,0,"p2 = %8ld ** %12.5lf %12.5lf %10.4lf",p2,pointAddrP(catchmentDtmP,p2)->x,pointAddrP(catchmentDtmP,p2)->y,pointAddrP(catchmentDtmP,p2)->z) ;
                   bcdtmWrite_message(0,0,0,"p3 = %8ld ** %12.5lf %12.5lf %10.4lf",p3,pointAddrP(catchmentDtmP,p3)->x,pointAddrP(catchmentDtmP,p3)->y,pointAddrP(catchmentDtmP,p3)->z) ;
                  }
                trgPts[0].x = pntP->x  ; trgPts[0].y = pntP->y  ; trgPts[0].z = pntP->z ;
                trgPts[1].x = pnt1P->x ; trgPts[1].y = pnt1P->y ; trgPts[1].z = pnt1P->z ;
                trgPts[2].x = pnt2P->x ; trgPts[2].y = pnt2P->y ; trgPts[2].z = pnt2P->z ;
                trgPts[3].x = pntP->x  ; trgPts[3].y = pntP->y  ; trgPts[3].z = pntP->z ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(catchDtmP,DTMFeatureType::Breakline,catchDtmP->nullUserTag,1,&catchDtmP->nullFeatureId,trgPts,4)) goto errexit ;
               }
            }
          p2 = p3 ;
         }
      }
   }
/*
**  Triangulate
*/
 if( catchDtmP->numPoints < 3 ) goto cleanup ;
 if( bcdtmObject_triangulateDtmObject(catchDtmP)) goto errexit ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(catchDtmP)) goto errexit ;
 if( dbg == 1 ) if( bcdtmWrite_toFileDtmObject(catchDtmP,L"catchment04.tin")) goto errexit ;
 if( bcdtmList_extractHullDtmObject(catchDtmP,catchmentPtsPP,numCatchmentPtsP)) goto errexit  ;
/*
** Clean Up
*/
 cleanup :
 if( catchPtsP     != NULL ) { free(catchPtsP) ; catchPtsP = NULL ; }
 if( catchDtmP     != NULL ) bcdtmObject_destroyDtmObject(&catchDtmP) ;
 if( catchmentDtmP != NULL ) bcdtmObject_destroyDtmObject(&catchmentDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Refining Tptr Catchment Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Refining Tptr Catchment Polygon Error") ;
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
int bcdtmDrainage_traceMaximumAscentFromPointOnTriangleEdgeDtmObject
(
 BC_DTM_OBJ *dtmP,                   // ==> Dtm Object
 double     pointX,                  // ==> X Coordinate Of Point
 double     pointY,                  // ==> Y Coordinate Of Point
 double     pointZ,                  // ==> Z Coordinate Of Point
 long       point1,                  // ==> Tin Point Of Triangle Edge
 long       point2,                  // ==> Tin Point Of Triangle Edge  ( direction of point1-point2-point3 is ) clockwise
 long       point3,                  // ==> Tin Point Of Triangle Vertex Opposite Edge ( direction of point1-point2-point3 is ) clockwise
 long       *ascentTracedP,          // <== Set To True If Maximum Ascent Traced Else False
 long       *highPoint1P,            // <== Terminating Point Or Line Point For Trace
 long       *highPoint2P,            // <== Terminating Line Point For Trace
 DPoint3d        **tracePtsPP,            // <== Maximum Ascent Trace Points
 long       *numTracePtsP            // <== Number Of Trace Points
)
/*
** This Function Traces A Maximum Ascent Line Starting From A Point On Triangle Edge point1-point2
** The direction of point1-point2-point3 must be Set clockwise
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,ascentType,traceAscent,lastPoint,memPoints=0,memPointsInc=1000  ;
 double dx,dy,ascentAngle=0.0,descentAngle,ascentSlope=0.0,radius  ;
 double sx,sy,sz,rx,ry,nx,ny,nz,fx,fy,fz,lastAngle ;
 DTM_TIN_POINT *pnt1P,*pnt2P,*pnt3P ;
 DTMDrainageTables *drainageTablesP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Point On Triangle Edge") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pointX     = %12.5lf",pointX) ;
    bcdtmWrite_message(0,0,0,"pointY     = %12.5lf",pointY) ;
    bcdtmWrite_message(0,0,0,"pointZ     = %12.5lf",pointZ) ;
    bcdtmWrite_message(0,0,0,"point1     = %8ld ** %12.5lf %12.5lf %10.4lf",point1,pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y,pointAddrP(dtmP,point1)->z) ;
    bcdtmWrite_message(0,0,0,"point2     = %8ld ** %12.5lf %12.5lf %10.4lf",point2,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y,pointAddrP(dtmP,point2)->z) ;
    bcdtmWrite_message(0,0,0,"point3     = %8ld ** %12.5lf %12.5lf %10.4lf",point3,pointAddrP(dtmP,point3)->x,pointAddrP(dtmP,point3)->y,pointAddrP(dtmP,point3)->z) ;
   }
/*
** Initialise
*/
 *ascentTracedP = FALSE ;
 *highPoint1P   = dtmP->nullPnt ;
 *highPoint2P   = dtmP->nullPnt ;
 if( *tracePtsPP != NULL ) free(*tracePtsPP) ;
 *tracePtsPP    = NULL ;
 *numTracePtsP  = 0 ;
 lastPoint      = dtmP->nullPnt ;
/*
** Check Triangle Direction
*/
 if( bcdtmMath_pointSideOfDtmObject(dtmP,point1,point2,point3) > 0 )
   {
    p1 = point1 ;
    point1 = point2 ;
    point2 = p1 ;
   }
/*
** Initialise
*/
 p1 = p2 = p3 = dtmP->nullPnt ;
 fx = sx = pointX ;
 fy = sy = pointY ;
 fz = sz = pointZ ;
 dx = dtmP->xMax - dtmP->xMin ;
 dy = dtmP->yMax - dtmP->yMin ;
 radius = sqrt(dx*dx + dy*dy) ;
 ascentSlope = -1.0 ;
 ascentAngle = 0.0 ;
/*
** Calculate Triangle Ascent and Descent Angles And Slope
*/
 if( bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,point1,point2,point3,&descentAngle,&ascentAngle,&ascentSlope)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"ascentAngle = %12.10lf ascentSlope = %12.5lf",ascentAngle,ascentSlope) ;
/*
** Trace Maximum Ascent Slope
*/
 if( ascentSlope >= 0.0 )
   {
    *ascentTracedP = TRUE ;
    sx = pointX ;
    sy = pointY ;
    sz = pointZ ;
/*
**  Allocate memory To Store Maximum Ascent Points
*/
    memPoints = memPointsInc ;
    *tracePtsPP = ( DPoint3d * ) malloc(memPoints*sizeof(DPoint3d)) ;
    if( *tracePtsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Store Start Point
*/
    (*tracePtsPP)->x = sx ;
    (*tracePtsPP)->y = sy ;
    (*tracePtsPP)->z = sz ;
    *numTracePtsP = 1 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Trace Point[%5ld] = %12.5lf %12.5lf %10.4lf",*numTracePtsP,sx,sy,sz) ;
/*
**  Calculate Intersection Of Maximum Ascent With Triangle Edge
*/
    rx = sx + radius * cos(ascentAngle) ;
    ry = sy + radius * sin(ascentAngle) ;
    if( bcdtmDrainage_calculateRadialIntersectOnOppositeTriangleEdgeDtmObject(dtmP,sx,sy,rx,ry,point1,point2,point3,&nx,&ny,&nz,&p1,&p2,&p3)) goto errexit ;
/*
**  Store Next Trace Point
*/
    (*tracePtsPP+*numTracePtsP)->x = nx ;
    (*tracePtsPP+*numTracePtsP)->y = ny ;
    (*tracePtsPP+*numTracePtsP)->z = nz ;
    ++*numTracePtsP ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Trace Point[%5ld] = %12.5lf %12.5lf %10.4lf",*numTracePtsP,nx,ny,nz) ;
/*
**  Scan To Highest Point
*/
    traceAscent = 1 ;
    while ( traceAscent )
      {
       traceAscent = 0 ;
/*
**     Write Trace Triangle
*/
       if( dbg == 1 )
         {
          bcdtmWrite_message(0,0,0,"p1 = %10ld p2 = %10ld p3 = %10ld",p1,p2,p3) ;
          if( p1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"p1 = %10ld ** %12.5lf %12.5lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
          if( p2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"p2 = %10ld ** %12.5lf %12.5lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
          if( p3 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"p3 = %10ld ** %12.5lf %12.5lf %10.4lf",p3,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p3)->z) ;
         }
/*
**     Set High Points
*/
       *highPoint1P = p1 ;
       *highPoint2P = p2 ;
/*
**     Initialise Some Values
*/
       lastAngle = bcdtmMath_getAngle(sx,sy,nx,ny) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"lastAngle = %12.10lf",lastAngle) ;
       sx = nx ;
       sy = ny ;
       sz = nz ;
/*
**     Determine Next Trace Triangle From A Triangle Point
*/
       if( p2 == dtmP->nullPnt )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Determining Next Triangle Trace From Point %8ld",p1) ;
          if( bcdtmDrainage_scanPointForMaximumAscentDtmObject(dtmP,drainageTablesP,p1,lastPoint,&ascentType,&p2,&p3,&ascentSlope,&ascentAngle) ) goto errexit ;
          lastPoint = p1 ;
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"ascentType = %2ld ascentAngle = %12.10lf ascentSlope = %10.4lf ** p1 = %10ld p2 = %10ld p3 = %10ld",ascentType,ascentAngle,ascentSlope,p1,p2,p3) ;
          if( ascentType == dtmP->nullPnt ) ascentType = 0 ;
          if( ascentType )
            {
             if( ascentType == 1 ) traceAscent = 2 ;
             else
               {
                pnt2P = pointAddrP(dtmP,p2) ;
                pnt3P = pointAddrP(dtmP,p3) ;
                rx = sx + radius * cos(ascentAngle) ;
                ry = sy + radius * sin(ascentAngle) ;
                if     ( bcdtmMath_sideOf(sx,sy,rx,ry,pnt2P->x,pnt2P->y) == 0 ) traceAscent = 2 ;
                else if( bcdtmMath_sideOf(sx,sy,rx,ry,pnt3P->x,pnt3P->y) == 0 ) traceAscent = 3 ;
                else if( bcdtmMath_sideOf(sx,sy,rx,ry,pnt2P->x,pnt2P->y) > 0 && bcdtmMath_sideOf(sx,sy,rx,ry,pnt3P->x,pnt3P->y) < 0 ) traceAscent = 4 ;
               }
            }
         }
       else
         {
/*
**        Determine Next Trace Triangle From A Triangle Edge
*/
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Determining Next Triangle Trace From Edge %8ld %8ld",p1,p2) ;
          if( nodeAddrP(dtmP,p1)->hPtr != p2 )
            {
             if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
             if( bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,p1,p2,p3,&descentAngle,&ascentAngle,&ascentSlope)) goto errexit ;
             rx = sx + radius * cos(ascentAngle) ;
             ry = sy + radius * sin(ascentAngle) ;
             pnt1P = pointAddrP(dtmP,p1) ;
             pnt2P = pointAddrP(dtmP,p2) ;
             pnt3P = pointAddrP(dtmP,p3) ;
/*
**           Check For Intersection With Triangle Vertices And Edges
*/
             if     ( bcdtmMath_sideOf(sx,sy,rx,ry,pnt1P->x,pnt1P->y) == 0 ) traceAscent = 1 ;
             else if( bcdtmMath_sideOf(sx,sy,rx,ry,pnt2P->x,pnt2P->y) == 0 ) traceAscent = 2 ;
             else if( bcdtmMath_sideOf(sx,sy,rx,ry,pnt3P->x,pnt3P->y) == 0 ) traceAscent = 3 ;
             else if( bcdtmMath_sideOf(sx,sy,rx,ry,pnt2P->x,pnt2P->y) > 0 && bcdtmMath_sideOf(sx,sy,rx,ry,pnt3P->x,pnt3P->y) < 0 ) traceAscent = 4 ;
             else if( bcdtmMath_sideOf(sx,sy,rx,ry,pnt3P->x,pnt3P->y) > 0 && bcdtmMath_sideOf(sx,sy,rx,ry,pnt1P->x,pnt1P->y) < 0 ) traceAscent = 5 ;
             else if( pointAddrP(dtmP,p1)->z > pointAddrP(dtmP,p2)->z ) traceAscent = 1 ;
             else if( pointAddrP(dtmP,p2)->z > pointAddrP(dtmP,p1)->z ) traceAscent = 2 ;
            }
         }
/*
**     Calculate Trace Intercept With Triangle
*/
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"******** traceAscent = %2ld",traceAscent) ;
       if( traceAscent )
         {
          switch ( traceAscent )
            {
             case 1 :         // Maximum Ascent Passes Through P1
                pnt1P = pointAddrP(dtmP,p1) ;
                nx = pnt1P->x ;
                ny = pnt1P->y ;
                nz = pnt1P->z ;
                p2 = p3 = dtmP->nullPnt ;
             break ;

             case 2 :         // Maximum Ascent Passes Through P2
                pnt2P = pointAddrP(dtmP,p2) ;
                nx = pnt2P->x ;
                ny = pnt2P->y ;
                nz = pnt2P->z ;
                p1 = p2 ;
                p2 = p3 = dtmP->nullPnt ;
             break ;

             case 3 :         // Maximum Ascent Passes Through P3
                pnt3P = pointAddrP(dtmP,p3) ;
                nx = pnt3P->x ;
                ny = pnt3P->y ;
                nz = pnt3P->z ;
                p1 = p3 ;
                p2 = p3 = dtmP->nullPnt ;
            break ;

             case 4 :         // Maximum Ascent Intersects P2-P3
                pnt2P = pointAddrP(dtmP,p2) ;
                pnt3P = pointAddrP(dtmP,p3) ;
                bcdtmDrainage_intersectCordLines(sx,sy,rx,ry,pnt2P->x,pnt2P->y,pnt3P->x,pnt3P->y,&nx,&ny) ;
                p1 = p3 ;
                if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
             break ;

             case 5 :         // Maximum Ascent Intersects P3-P1
                pnt3P = pointAddrP(dtmP,p3) ;
                pnt1P = pointAddrP(dtmP,p1) ;
                bcdtmDrainage_intersectCordLines(sx,sy,rx,ry,pnt3P->x,pnt3P->y,pnt1P->x,pnt1P->y,&nx,&ny) ;
                p2 = p3 ;
                if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
             break ;

             case 6 :       // Ridge Line To P1
             break ;

             default :
                bcdtmWrite_message(2,0,0,"No Maximum Ascent Intersection Found With Triangle") ;
                goto errexit ;
             break ;
            }
/*
**        Check Memory
*/
          if( *numTracePtsP >= memPoints )
            {
             memPoints = memPoints + memPointsInc ;
             *tracePtsPP = ( DPoint3d *) realloc( *tracePtsPP , memPoints * sizeof(DPoint3d)) ;
             if( *tracePtsPP == NULL )
               {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               }
            }
/*
**        Store Point
*/
          (*tracePtsPP+*numTracePtsP)->x = nx ;
          (*tracePtsPP+*numTracePtsP)->y = ny ;
          (*tracePtsPP+*numTracePtsP)->z = nz ;
          ++*numTracePtsP ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Trace Point[%5ld] = %12.5lf %12.5lf %10.4lf",*numTracePtsP,nx,ny,nz) ;
         }
      }
   }

/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent Between Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent Between Points Error") ;
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
int bcdtmDrainage_checkPointOnTinHullCanBeDeletedDtmObject(BC_DTM_OBJ *dtmP,long point,long *canBeDeletedP)
{
 int ret=DTM_SUCCESS ;
 long priorPoint,nextPoint ;
/*
** Check Point Can Be Deleted
*/
 *canBeDeletedP = 1 ;
 nextPoint = nodeAddrP(dtmP,point)->hPtr ;
 if( ( priorPoint = bcdtmList_nextClkDtmObject(dtmP,point,nextPoint))  < 0 ) goto errexit ;
 if( ( priorPoint = bcdtmList_nextClkDtmObject(dtmP,point,priorPoint)) < 0 ) goto errexit ;
 while( priorPoint != nextPoint && *canBeDeletedP )
   {
    if( nodeAddrP(dtmP,priorPoint)->hPtr != dtmP->nullPnt ) *canBeDeletedP = 0 ;
    if( ( priorPoint = bcdtmList_nextClkDtmObject(dtmP,point,priorPoint)) < 0 ) goto errexit ;
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
int bcdtmDrainage_calculateRadialIntersectOnOppositeTriangleEdgeDtmObject
(
 BC_DTM_OBJ *dtmP ,
 double r1X ,
 double r1Y ,
 double r2X ,
 double r2Y ,
 long   pnt1 ,
 long   pnt2 ,
 long   pnt3 ,
 double *nextXP ,
 double *nextYP ,
 double *nextZP ,
 long   *nextPnt1P ,
 long   *nextPnt2P ,
 long   *nextPnt3P
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long sdof=0,intPnt ;
 DTM_TIN_POINT *pnt1P,*pnt2P,*pnt3P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Calculating Radial Intersect On Triangle Edge") ;
    bcdtmWrite_message(0,0,0,"r1X         = %12.5lf",r1X) ;
    bcdtmWrite_message(0,0,0,"r1Y         = %12.5lf",r1Y) ;
    bcdtmWrite_message(0,0,0,"r2X         = %12.5lf",r2X) ;
    bcdtmWrite_message(0,0,0,"r2Y         = %12.5lf",r2Y) ;
    bcdtmWrite_message(0,0,0,"pnt1        = %8ld",pnt1) ;
    bcdtmWrite_message(0,0,0,"pnt2        = %8ld",pnt2) ;
    bcdtmWrite_message(0,0,0,"pnt3        = %8ld",pnt3) ;
   }
/*
** Initialise
*/
 *nextXP = *nextYP = *nextZP = 0.0 ;
 *nextPnt1P = *nextPnt1P = *nextPnt1P = dtmP->nullPnt ;
 pnt1P = pointAddrP(dtmP,pnt1) ;
 pnt2P = pointAddrP(dtmP,pnt2) ;
 pnt3P = pointAddrP(dtmP,pnt3) ;
/*
** Calculate Intercept Of Radial On pnt1-pnt3 Or pnt2-pnt3
*/
 sdof = bcdtmMath_sideOf(r1X,r1Y,r2X,r2Y,pnt3P->x,pnt3P->y) ;
/*
**  Maximum Ascent Passes Throught pnt3
*/
 if( sdof == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Radial Passes Through pnt3") ;
    *nextPnt1P = pnt3 ;
    *nextXP = pointAddrP(dtmP,pnt3)->x ;
    *nextYP = pointAddrP(dtmP,pnt3)->y ;
    *nextZP = pointAddrP(dtmP,pnt3)->z ;
   }
/*
** Maximum Ascent Passes Throught pnt2-pnt3
*/
 if( sdof <  0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Ascent Passes Through pnt2-pnt3") ;
    if( bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP,r1X,r1Y,r2X,r2Y,pnt3,pnt2,nextXP,nextYP,nextZP,&intPnt)) goto errexit ;
    if( intPnt != dtmP->nullPnt )
      {
       *nextPnt1P = intPnt ;
       *nextXP = pointAddrP(dtmP,*nextPnt1P)->x ;
       *nextYP = pointAddrP(dtmP,*nextPnt1P)->y ;
       *nextZP = pointAddrP(dtmP,*nextPnt1P)->z ;
      }
    else
      {
       *nextPnt1P = pnt3 ;
       *nextPnt2P = pnt2 ;
       if(( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,*nextPnt1P,*nextPnt2P)) < 0 ) goto errexit ;
      }
   }
/*
**  Maximum Ascent Passes Throught pnt1-pnt3
*/
 if( sdof >  0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Ascent Passes Through pnt1-pnt3") ;
    if( bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP,r1X,r1Y,r2X,r2Y,pnt1,pnt3,nextXP,nextYP,nextZP,&intPnt)) goto errexit ;
    if( intPnt != dtmP->nullPnt )
      {
       *nextPnt1P = intPnt ;
       *nextXP = pointAddrP(dtmP,*nextPnt1P)->x ;
       *nextYP = pointAddrP(dtmP,*nextPnt1P)->y ;
       *nextZP = pointAddrP(dtmP,*nextPnt1P)->z ;
      }
    else
      {
       *nextPnt1P = pnt1 ;
       *nextPnt2P = pnt3 ;
       if(( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,*nextPnt1P,*nextPnt2P)) < 0 ) goto errexit ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Radial Intersect On Triangle Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Radial Intersect On Triangle Edge Error") ;
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
int bcdtmDrainage_traceCatchmentFromInternalSumpPointDtmObject
(
 BC_DTM_OBJ *dtmP,
 double x,
 double y,
 double z,
 long   sumpPnt,
 long   useTables,
 DPoint3d    **catchmentPtsPP,
 long   *numCatchmentPtsP
)
/*
** This Function Traces The Catchment From An Internal Sump Point
** It Assumes The Internal Sump Point Has Been Totally Validated
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   noneNullTptr,numPondPts=0 ;
 long   startPnt,priorPnt,exitPnt,nextPnt,ascentType=0,ascentPnt1=0,ascentPnt2=0,ascentPnt3=0;
 long   numPointArrays=0,numTracePoints=0,dtmFeature ;
 double xMin,yMin,xMax,yMax,ascentSlope=0.0,ascentAngle=0.0 ;
 DPoint3d    *p3dP,*pondPtsP=NULL,randomSpot[4] ;
 BC_DTM_OBJ *tempDtmP=NULL ;
 DTM_ASCENT_LINE  ascentLine ;
 DTM_POINT_ARRAY  **pointArraysPP=NULL,**pointArrayPP ;
 DTMUserTag nullUserTag = DTM_NULL_USER_TAG ;
 DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID ;
/*
** Write Entry message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Catchment From Internal Sump Point") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"x                = %15.5lf",x)  ;
    bcdtmWrite_message(0,0,0,"y                = %15.5lf",y)  ;
    bcdtmWrite_message(0,0,0,"z                = %15.5lf",z)  ;
    bcdtmWrite_message(0,0,0,"sumpPnt          = %8ld",sumpPnt) ;
    bcdtmWrite_message(0,0,0,"catchmentPtsPP   = %p",*catchmentPtsPP) ;
    bcdtmWrite_message(0,0,0,"numCatchmentPtsP = %8ld",*numCatchmentPtsP) ;
   }
/*
** Check Point Is Not On Zero Slope Polygon
*/
 if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::ZeroSlopePolygon,sumpPnt,&dtmFeature) ) goto cleanup ;
/*
** Check For None Null Tptr Values
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For None Null Tptr Values") ;
 if( bcdtmList_checkForNoneNullTptrValuesDtmObject(dtmP,&noneNullTptr) ) goto errexit ;
 if( noneNullTptr )
   {
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
    bcdtmWrite_message(2,0,0,"01 None Null Tptr Values") ;
    goto errexit ;
   }
/*
** Check For None Null Sptr Values
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For None Null Sptr Values") ;
 if( bcdtmList_checkForNoneNullSptrValuesDtmObject(dtmP,&noneNullTptr) )  goto errexit ;
 if( noneNullTptr )
   {
    bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtmP,1) ;
    bcdtmWrite_message(2,0,0,"01 None Null Tptr Values") ;
    goto errexit ;
   }
/*
** Place Tptr Polygon About Sump Point
*/
 if( bcdtmList_insertTptrPolygonAroundPointDtmObject(dtmP,sumpPnt,&startPnt)) goto errexit ;
/*
** Expand Tptr Polygon To Pond Exit Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Expanding Pond To Exit Point") ;
 if( bcdtmDrainage_expandPondToExitPointDtmObject(dtmP,NULL,NULL,NULL,startPnt,&exitPnt,&priorPnt,&nextPnt)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"exitPnt = %9ld",exitPnt) ;
 if( exitPnt == dtmP->nullPnt )
   {
    bcdtmWrite_message(2,0,0,"Pond Exit Point About About Low Point Not Determined") ;
    goto errexit ;
   }
/*
** Count Number Of Pond Points
*/
 numPondPts = 0 ;
 startPnt = exitPnt ;
 do
   {
    ++numPondPts ;
    startPnt = nodeAddrP(dtmP,startPnt)->tPtr ;
   } while ( startPnt != exitPnt) ;
 ++numPondPts ;
/*
** Write Pond Boundary
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Pond Exit Point = %9ld",exitPnt) ;
    startPnt = exitPnt ;
    do
      {
       bcdtmWrite_message(0,0,0,"Pond Point[%9ld] = %12.5lf %12.5lf %10.4lf",startPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,startPnt)->z) ;
       startPnt = nodeAddrP(dtmP,startPnt)->tPtr ;
      } while ( startPnt != exitPnt ) ;
    bcdtmWrite_message(0,0,0,"Pond Point[%9ld] = %12.5lf %12.5lf %10.4lf",startPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,startPnt)->z) ;
    if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,10000,10000) ;
    if( bcdtmList_copyTptrListFromDtmObjectToDtmObject(dtmP,tempDtmP,exitPnt,DTMFeatureType::Breakline,nullUserTag,nullFeatureId)) goto errexit ;
    bcdtmWrite_geopakDatFileFromDtmObject(tempDtmP,L"sumpPond.dat") ;
    bcdtmObject_destroyDtmObject(&tempDtmP) ;
   }
/*
** Scan Sump Point Pond In Anti Clockwise Direction For Ascent Lines
*/
 startPnt = exitPnt ;
 do
   {
    nextPnt = nodeAddrP(dtmP,startPnt)->tPtr ;
/*
**  Test For Ridge Line
*/
//    if( bcdtmDrainage_getMaximumAscentForLineDtmObject(dtmP,startPnt,nextPnt,&ascentType,&ascentAngle,&ascentSlope,&ascentPnt1,&ascentPnt2,&ascentPnt3)) goto errexit ;
/*
**  Push Ascent Line Onto Stack
*/
    if( ascentType )
      {
       ascentLine.flowSide    = 2 ;
       ascentLine.ascentType  = ascentType ;
       ascentLine.ascentAngle = ascentAngle ;
       ascentLine.slope       = ascentSlope ;
       ascentLine.pnt1        = ascentPnt1 ;
       ascentLine.pnt2        = ascentPnt2 ;
       ascentLine.pnt3        = ascentPnt3 ;
       ascentLine.x           = pointAddrP(dtmP,startPnt)->x ;
       ascentLine.y           = pointAddrP(dtmP,startPnt)->y ;
       ascentLine.z           = pointAddrP(dtmP,startPnt)->z ;
//       if( bcdtmDrainage_pushAscentStackFiFo(&ascentLine)) goto errexit ;
      }
/*
**  Get Next Pond Line
*/
     startPnt = nextPnt ;
   } while ( startPnt != exitPnt ) ;
/*
** Scan Sump Point Pond In Clockwise Direction For Ascent Lines
*/
 bcdtmList_reverseTptrPolygonDtmObject(dtmP,exitPnt) ;
 startPnt = exitPnt ;
 do
   {
    nextPnt = nodeAddrP(dtmP,startPnt)->tPtr ;
/*
**  Test For Ridge Line
*/
//    if( bcdtmDrainage_getMaximumAscentForLineDtmObject(dtmP,startPnt,nextPnt,&ascentType,&ascentAngle,&ascentSlope,&ascentPnt1,&ascentPnt2,&ascentPnt3)) goto errexit ;
/*
**  Push Ascent Line Onto Stack
*/
    if( ascentType )
      {
       ascentLine.flowSide    = 1 ;
       ascentLine.ascentType  = ascentType ;
       ascentLine.ascentAngle = ascentAngle ;
       ascentLine.slope       = ascentSlope ;
       ascentLine.pnt1        = ascentPnt1 ;
       ascentLine.pnt2        = ascentPnt2 ;
       ascentLine.pnt3        = ascentPnt3 ;
       ascentLine.x           = pointAddrP(dtmP,startPnt)->x ;
       ascentLine.y           = pointAddrP(dtmP,startPnt)->y ;
       ascentLine.z           = pointAddrP(dtmP,startPnt)->z ;
//       if( bcdtmDrainage_pushAscentStackFiFo(&ascentLine)) goto errexit ;
      }
/*
**  Get Next Pond Line
*/
     startPnt = nextPnt ;
   } while ( startPnt != exitPnt ) ;
/*
** Save Tptr Pond Points
*/
 if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,exitPnt,&pondPtsP,&numPondPts)) goto errexit ;
 bcdtmList_nullTptrListDtmObject(dtmP,exitPnt) ;
/*
** Process Ascent Lines Stack
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Processing Ascent Stack") ;
// if( bcdtmDrainage_processAscentStackFiFoDtmObject(dtmP,&pointArraysPP,&numPointArrays)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Point Arrays = %6ld",numPointArrays) ;
/*
** Count Number Of Trace Points
*/
 numTracePoints = 0 ;
 for( pointArrayPP = pointArraysPP ; pointArrayPP < pointArraysPP + numPointArrays ; ++pointArrayPP )
   {
    numTracePoints = numTracePoints + (*pointArraysPP)->numPoints ;
   }
/*
** Create Data Object
*/
 if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,numTracePoints+4,numTracePoints+4) ;
/*
** Populate Data Object With Pond Points
*/
 if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,nullUserTag,1,&nullFeatureId,pondPtsP,numPondPts)) goto errexit ;
/*
** Populate Data Object With Trace Points
*/
 for( pointArrayPP = pointArraysPP ; pointArrayPP < pointArraysPP + numPointArrays ; ++pointArrayPP )
   {
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,(long)(pointArrayPP-pointArraysPP),1,&nullFeatureId,(*pointArrayPP)->pointsP,(*pointArrayPP)->numPoints)) goto errexit ;
   }
/*
** Write Trace Data
*/
 if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(tempDtmP,L"ascentTrace.dat") ;
/*
** Store Bounding Rectangle In Data Object To Ensure No Problems With
** Less Than 3 Data Points And Collinear Points
*/
 xMin = tempDtmP->yMin - 1.0 ;
 yMin = tempDtmP->yMin - 1.0 ;
 xMax = tempDtmP->yMax + 1.0 ;
 yMax = tempDtmP->yMax + 1.0 ;
 randomSpot[0].x = xMin ; randomSpot[0].y = yMin ; randomSpot[0].z = 0.0 ;
 randomSpot[1].x = xMax ; randomSpot[1].y = yMin ; randomSpot[1].z = 0.0 ;
 randomSpot[2].x = xMax ; randomSpot[2].y = yMax ; randomSpot[2].z = 0.0 ;
 randomSpot[3].x = xMin ; randomSpot[3].y = yMax ; randomSpot[3].z = 0.0 ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,nullUserTag,1,&nullFeatureId,randomSpot,4)) goto errexit ;
/*
** Triangulate Dtm Object
*/
 if( bcdtmObject_createTinDtmObjectOverload (tempDtmP,1,0.0,false,false)) goto errexit ;
/*
** Extract Catchment Outline From Tin
*/
// if( bcdtmDrainage_extractPointCatchmentFromAscentTracesDtmObject(tempDtmP,pointAddrP(dtmP,exitPnt)->x,pointAddrP(dtmP,exitPnt)->y,catchmentPtsPP,numCatchmentPtsP)) goto errexit ;
/*
** Write Out Catchment Boundary
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Catchment Boundary Points = %6ld",*numCatchmentPtsP) ;
    for( p3dP = *catchmentPtsPP ; p3dP < *catchmentPtsPP + *numCatchmentPtsP ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Catchment Point[%6ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-*catchmentPtsPP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( pondPtsP != NULL ) { free(pondPtsP) ; pondPtsP = NULL ; }
 if( tempDtmP != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 if( pointArraysPP != NULL ) bcdtmMem_freePointerArrayToPointArrayMemory(&pointArraysPP,numPointArrays) ;
 if( bcdtmList_checkForNoneNullTptrValuesDtmObject(dtmP,&noneNullTptr) ) goto errexit ;
 if( noneNullTptr )
   {
    bcdtmList_reportAndSetToNullNoneNullTptrValuesDtmObject(dtmP,1) ;
    bcdtmWrite_message(2,0,0,"None Null Tptr Values") ;
    ret = DTM_ERROR ;
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Catchment From Internal Sump Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Catchment From Internal Sump Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_refineTinForDrainageDtmObject
(
 BC_DTM_OBJ         *dtmP,                    // ==> Pointer To Dtm Object
 bool               useFence,                 // ==> Load Feature Within Fence
 DTMFenceType       fenceType,                // ==> Type Of Fence Reactangular Or Shape
 DTMFenceOption     fenceOption,              // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
 const DPoint3d     *fencePtsP,               // ==> DPoint3d Array Of Fence Points
 int                numFencePts               // ==> Number Of Fence Points
)
{
 int               ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long              numHullPts,startTime=bcdtmClock(),polygonTime=bcdtmClock() ;
 BC_DTM_OBJ        *clipDtmP=NULL ;
 DPoint3d          *hullPtsP=NULL ;

// Log Entry Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Refining Tin For Drainage") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
   }

// Validate Fence

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

// Clip Tin To Fence

 if( useFence )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
    if( bcdtmList_extractHullDtmObject(clipDtmP,&hullPtsP,&numHullPts)) goto errexit ;
    if( bcdtmClip_toPolygonDtmObject(dtmP,hullPtsP,numHullPts,DTMClipOption::External)) goto errexit ;
   }

// Clean Up

 cleanup :
 if( clipDtmP  != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Refining Tin For Drainage Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Refining Tin For Drainage Error") ;
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
int bcdtmDrainage_determineCatchmentsDtmObject
(
 BC_DTM_OBJ              *dtmP,                 // ==> Pointer To Dtm
 DTMFeatureCallback      loadFunctionP,         // ==> Pointer To Load Function
 DTMDrainageTables       *drainageTablesP,      // ==> Pointer To Drainage Tables
 double                  falseLowDepth,         // ==> False Low Depth
 bool                    useFence,              // ==> Load Feature Within Fence
 DTMFenceType            fenceType,             // ==> Rectangular DTMFenceType::Block or Irregular Shape DTMFenceType::Shape
 DTMFenceOption          fenceOption,           // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
 DPoint3dCP              fencePtsP,             // ==> DPoint3d Array Of Fence Points
 int                     numFencePts,           // ==> Number Of Fence Points
 void                    *userP,                // ==> User Pointer Passed Back To User
 int&                    numCatchments          // <== Number Of Catchments Determined
)
/*
** This Function Determines The Tin Catchments
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 int     numScanned=-1,numTraced=0,zeroSlopeOption=1 ;
 long    startTime=bcdtmClock() ;
 long    pnt1,pnt2,pnt3,index,trgPnt1,trgPnt2,trgPnt3,fndPnt,lowPnt1,lowPnt2 ;
 double  startX,startY,startZ ;
 bool    traceToLowPoint=false ;
 DTM_TIN_POINT *pointP ;
 BC_DTM_OBJ    *clipTinP=NULL ;
 DTMTriangleIndex*  triangleIndexP=nullptr ;

 double time=0.0,maxTime=0.0 ;
 long   maxTriangle=-1 ;

//  Log Entry Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determining Catchments") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP)  ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP)  ;
    bcdtmWrite_message(0,0,0,"falseLowDepth   = %8.4lf",falseLowDepth) ;
    bcdtmWrite_message(0,0,0,"useFence        = %8d",useFence)  ;
    bcdtmWrite_message(0,0,0,"fenceType       = %8d",fenceType)  ;
    bcdtmWrite_message(0,0,0,"fenceOption     = %8d",fenceOption)  ;
    bcdtmWrite_message(0,0,0,"fencePtsP       = %p",fencePtsP)  ;
    bcdtmWrite_message(0,0,0,"numFencePts     = %8d",numFencePts)  ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP)  ;
   }

// Initialise

 numCatchments = 0 ;
 if( falseLowDepth < 0.0 ) falseLowDepth = 0.0 ;

// Validate Fence

 if( useFence )
   {
   if (fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap) fenceOption = DTMFenceOption::Inside;
   if (useFence && (fencePtsP == NULL || numFencePts <= 2)) useFence = false;
    if( useFence && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = false ;
   }

// Test For Valid Tin Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;

// Test For DTM Object In Tin State

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires A Triangulated DTM") ;
    goto errexit ;
   }

// Round Tin Elevation Values To Eight Decimal Places

 if( dbg ) bcdtmWrite_message(0,0,0,"Rounding Elevation Values To Eight Decimal Places") ;
 for( pnt1 = 0 ; pnt1 < dtmP->numPoints ; ++pnt1 )
   {
    pointP = pointAddrP(dtmP,pnt1) ;
    pointP->z = bcdtmMath_roundToDecimalPoints(pointP->z,8) ;
   }

// Build Clipping Tin For Fence

 if( useFence )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Clipping Tin") ;
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipTinP,fencePtsP,numFencePts)) goto errexit ;
   }

// Create Triangle Index

 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Index") ;
 startTime = bcdtmClock() ;
 triangleIndexP = new DTMTriangleIndex (dtmP) ;
 if( dbg  ) bcdtmWrite_message(0,0,0,"Size Of Triangle Index = %8d",triangleIndexP->TriangleIndexSize() ) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Calculate Triangle Index = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

// Log Number Of Void And Flat Triangles

 if( dbg )
   {
    triangleIndexP->LogTriangleCounts() ;
   }

//  Scan Triangle Index And Downstream Trace From Each Triangle Centroid

 if( dbg ) bcdtmWrite_message(0,0,0,"Down Stream Tracing Tin Streams") ;
 for(DtmTriangleIndex::iterator triangle = triangleIndexP->FirstTriangle() ; triangle <= triangleIndexP->LastTriangle() ; triangle++)
   {

//     Log Trace Status

       ++numScanned ;
       if( dbg && numScanned % 100000 == 0 ) bcdtmWrite_message(0,0,0,"Number Of Triangles Scanned = %8ld of %8ld",numScanned,triangleIndexP->TriangleIndexSize()) ;

//     Set Null Colour For Triangle

       triangle->index = -dtmP->nullPnt ;

//     Dont Trace For Void Or Zero Slope Triangles

       if( ! triangle->voidTriangle && ! triangle->flatTriangle )
         {
          trgPnt1 = triangle->trgPnt1 ;
          trgPnt2 = triangle->trgPnt2 ;
          trgPnt3 = triangle->trgPnt3 ;
          startX = ( pointAddrP(dtmP,trgPnt1)->x + pointAddrP(dtmP,trgPnt2)->x + pointAddrP(dtmP,trgPnt3)->x) / 3.0 ;
          startY = ( pointAddrP(dtmP,trgPnt1)->y + pointAddrP(dtmP,trgPnt2)->y + pointAddrP(dtmP,trgPnt3)->y) / 3.0 ;
          startZ = ( pointAddrP(dtmP,trgPnt1)->z + pointAddrP(dtmP,trgPnt2)->z + pointAddrP(dtmP,trgPnt3)->z) / 3.0 ;

//        Check Fence Criteria

          traceToLowPoint = true ;
          if( useFence )
            {
             if( fenceOption == DTMFenceOption::Inside || fenceOption == DTMFenceOption::Overlap )
               {
                if ( startX < clipTinP->xMin || startX > clipTinP->xMax ||
                     startY < clipTinP->yMin || startY > clipTinP->yMax    ) traceToLowPoint = false ;
                else
                  {
                   if( ! bcdtmFind_triangleDtmObject(clipTinP,startX,startY,&fndPnt,&pnt1,&pnt2,&pnt3) ) goto errexit ;
                   if( ! fndPnt ) traceToLowPoint = false ;
                  }
               }
             if( fenceOption == DTMFenceOption::Outside )
               {
                traceToLowPoint = false ;
                if ( startX < clipTinP->xMin || startX > clipTinP->xMax ||
                     startY < clipTinP->yMin || startY > clipTinP->yMax    ) traceToLowPoint = true ;
                else
                  {
                   if( ! bcdtmFind_triangleDtmObject(clipTinP,startX,startY,&fndPnt,&pnt1,&pnt2,&pnt3) ) goto errexit ;
                   if( ! fndPnt ) traceToLowPoint = true ;
                  }
               }
            }

//      Trace To Low Point

         if( traceToLowPoint  )
           {
            ++numTraced ;
            startTime=bcdtmClock() ;
            if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Tracing From %12.5lf %12.5lf %10.4lf",startX,startY,startZ) ;
            if( bcdtmDrainage_traceToLowPointDtmObject(dtmP,drainageTablesP,nullptr,falseLowDepth,zeroSlopeOption,false,trgPnt1,trgPnt3,trgPnt2,startX,startY,startZ,userP,&lowPnt1,&lowPnt2) ) goto errexit ;
            if( dbg == 2 )
              {
               bcdtmWrite_message(0,0,0,"lowPnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",lowPnt1,pointAddrP(dtmP,lowPnt1)->x,pointAddrP(dtmP,lowPnt1)->y,pointAddrP(dtmP,lowPnt1)->z) ;
               if( lowPnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"lowPnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",lowPnt2,pointAddrP(dtmP,lowPnt2)->x,pointAddrP(dtmP,lowPnt2)->y,pointAddrP(dtmP,lowPnt2)->z) ;
              }

//          Set Triangle Index

            if( lowPnt2 == dtmP->nullPnt )
              {
               triangle->index = lowPnt1 ;
              }
            else
              {
               if( lowPnt2 < lowPnt1 )
                 {
                  index   = lowPnt1 ;
                  lowPnt1 = lowPnt2 ;
                  lowPnt2 = index   ;
                 }
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&index,lowPnt1,lowPnt2)) goto errexit ;
             triangle->index = dtmP->numPoints + index ;

             if( dbg && ( time = bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) > maxTime )
               {
                maxTime = time ;
                maxTriangle = numScanned ;
                bcdtmWrite_message(0,0,0,"maxTime = %8.3lf ** maxTriangle = %8ld",maxTime,maxTriangle) ;
               }
            }
        }
      }
   }

// Log Downstrean Trace Stats

 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Triangles Scanned = %8d ** Number Of Triangles Traced = %8ld",numScanned,numTraced) ;

// Polygonise And Load Catchment Polygons

 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Catchment Polygons") ;
 if( bcdtmDrainage_polygoniseAndLoadTriangleIndexPolygonsDtmObject(dtmP,loadFunctionP,triangleIndexP,userP,numCatchments)) goto errexit ;

// Clean Up

 cleanup :
 if( clipTinP   != NULL ) bcdtmObject_destroyDtmObject(&clipTinP) ;

// Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Catchments Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Catchments Error") ;
 return(ret) ;

// Error Exit

 errexit :
 ret = DTM_ERROR ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numScanned = %8ld",numScanned) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_polygoniseAndLoadTriangleIndexPolygonsDtmObject
(
 BC_DTM_OBJ          *dtmP,                         // ==> Pointer To Tin Object
 DTMFeatureCallback  loadFunctionP,                 // ==> Pointer To Load Function
 DTMTriangleIndex*   triangleIndexP,                // ==> Pointer To Triangle Index
 void                *userP,                        // ==> User Pointer Passed Back To User
 int&                numPolygons                    // <== Number Of Polygons Determined
)

// This Function Polygonises Triangle Colours In The Triangle Index Table

{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 int     numBefore,numAfter,numTrace=0 ;
 long    pnt1,pnt2,pnt3,pnt4,clPtr,lineOffset1,lineOffset2 ;
 long    *tinLineP=NULL,*lineP,lineOffset,lineValue,nullValue=-9999999 ;
 double  x,y,sx,sy,area ;
 DTMFeatureType dtmFeatureType=DTMFeatureType::Catchment ;
 DTMPointCache pointCache ;
 DTMUserTag   catchmentId ;

// Write Entry Message

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Polygonising Triangle Index") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP)  ;
    bcdtmWrite_message(0,0,0,"triangleIndexP  = %p",triangleIndexP)  ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP)  ;
    bcdtmWrite_message(0,0,0,"numPolygons     = %8ld",numPolygons)  ;
   }

// Initialise

 numPolygons = 0 ;

// Allocate Memory To Hold Values For Tin Lines

 tinLineP  = ( long * ) malloc (dtmP->cListPtr * sizeof(long)) ;
 if( tinLineP  == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit  ;
   }

// Set Tin Line Values To Null

 for( lineP = tinLineP  ; lineP < tinLineP  + dtmP->cListPtr ; ++lineP ) *lineP = nullValue ;

// Set Tin Line Values For Each Triangle

 for(DtmTriangleIndex::iterator triangle = triangleIndexP->FirstTriangle() ; triangle <= triangleIndexP->LastTriangle() ; triangle++)
   {
    if( triangle->index != -dtmP->nullPnt )
      {
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset,triangle->trgPnt1,triangle->trgPnt2) ) goto errexit ;
       *(tinLineP+lineOffset) = triangle->index ;
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset,triangle->trgPnt2,triangle->trgPnt3) ) goto errexit ;
       *(tinLineP+lineOffset) = triangle->index ;
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset,triangle->trgPnt3,triangle->trgPnt1) ) goto errexit ;
       *(tinLineP+lineOffset) = triangle->index ;
      }
   }

// Extract Polygons On the Tin Hull

 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Polygons On Tin Hull") ;
 pnt1 = dtmP->hullPoint ;
 do
   {
    pnt2 = nodeAddrP(dtmP,pnt1)->hPtr ;
    bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset1,pnt2,pnt1) ;
    if( ( lineValue = *(tinLineP+lineOffset1) ) != nullValue )
      {
       pnt3 = pnt2 ;
       pnt2 = pnt1 ;
       lineOffset = lineOffset1 ;
       catchmentId = lineValue ;
       if( pointCache.StorePointInCache(pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z)) goto errexit ;

//     Scan Back To First Point

       ++numTrace ;

       do
         {
          if( (pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt2,pnt3)) < 0 ) goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset1,pnt2,pnt3)) goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset2,pnt3,pnt2)) goto errexit ;
          while( *(tinLineP+lineOffset1) == lineValue && *(tinLineP+lineOffset2) == lineValue )
            {
             if( (pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt2,pnt3)) < 0 ) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset1,pnt2,pnt3)) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset2,pnt3,pnt2)) goto errexit ;
            }
          *(tinLineP+lineOffset1) = nullValue ;
          pnt4 = pnt2 ;
          pnt2 = pnt3 ;
          pnt3 = pnt4 ;
          if( pointCache.StorePointInCache(pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z)) goto errexit ;
         } while ( pnt2 != pnt1 ) ;
       *(tinLineP+lineOffset) = nullValue ;


//     Check For Duplicate Polygon Points

       if( cdbg )
         {
          numBefore = pointCache.SizeOfCache() ;
          pointCache.RemoveDuplicatePoints() ;
          numAfter  = pointCache.SizeOfCache() ;
          if( numBefore != numAfter )
            {
             bcdtmWrite_message(1,0,0,"Corrupt Polygon") ;
             goto errexit ;
            }
         }

//     Load Catchment

       if( pointCache.SizeOfCache()  >= 4 )
         {
          if( pointCache.CallUserDelegateWithCachePoints(loadFunctionP,dtmFeatureType,catchmentId,dtmP->nullFeatureId,userP)) goto errexit ;
          ++numPolygons ;
         }
       else
         {
          bcdtmWrite_message(1,0,0,"Not Enough Points In Polygon") ;
          goto errexit ;
         }
      }
    pnt1 = nodeAddrP(dtmP,pnt1)->hPtr  ;
   } while( pnt1 != dtmP->hullPoint ) ;


// Extract Internal Polygons

 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Polygons Internal To Tin Hull") ;
 for( pnt1 = 0 ; pnt1 < dtmP->numPoints ; ++pnt1 )
   {
    clPtr = nodeAddrP(dtmP,pnt1)->cPtr;
    while ( clPtr != dtmP->nullPtr )
      {
       pnt2 = clistAddrP(dtmP,clPtr)->pntNum ;
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset1,pnt1,pnt2) )  goto errexit ;
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset2,pnt2,pnt1) )  goto errexit ;
       if( *(tinLineP+lineOffset1) != *(tinLineP+lineOffset2)  && *(tinLineP+lineOffset1) != nullValue  && bcdtmList_testLineDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum) && ( nodeAddrP(dtmP,pnt1)->hPtr != pnt2 || nodeAddrP(dtmP,pnt2)->hPtr != pnt1 ) )
         {
          lineValue = *(tinLineP+lineOffset1) ;
          catchmentId = lineValue ;
          pnt3 = pnt1 ;

//        Get Polygon Direction

          area = 0.0 ;
          sx = pointAddrP(dtmP,pnt1)->x ;
          sy = pointAddrP(dtmP,pnt1)->y ;
          x = pointAddrP(dtmP,pnt2)->x - sx ;
          y = pointAddrP(dtmP,pnt2)->y - sy  ;
          area = area + ( x * y ) / 2.0 + x * sy ;
          sx = pointAddrP(dtmP,pnt2)->x ;
          sy = pointAddrP(dtmP,pnt2)->y ;
          do
            {
             if( ( pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt2,pnt3)) < 0 ) goto errexit  ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset1,pnt2,pnt3)) goto errexit  ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset2,pnt3,pnt2)) goto errexit  ;
             while( *(tinLineP+lineOffset1) == lineValue && *(tinLineP+lineOffset2) == lineValue )
               {
                if( ( pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt2,pnt3)) < 0 ) goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset1,pnt2,pnt3)) goto errexit  ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset2,pnt3,pnt2)) goto errexit  ;
               }
             x = pointAddrP(dtmP,pnt3)->x - sx ; y = pointAddrP(dtmP,pnt3)->y - sy  ;
             area = area + ( x * y ) / 2.0 + x * sy ;
             sx = pointAddrP(dtmP,pnt3)->x ;
             sy = pointAddrP(dtmP,pnt3)->y ;
              pnt4 = pnt2 ; pnt2 = pnt3 ; pnt3 = pnt4 ;
             } while ( pnt2 != pnt1 ) ;

//        If Polygon Is Clockwise Write Polygon If It Is Not A Void

          if( area > 0.0  )
            {
             pnt3 = pnt1 ;
             pnt2 = clistAddrP(dtmP,clPtr)->pntNum ;
             if( pointCache.StorePointInCache(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z)) goto errexit ;

//           Scan Back To First Point

             do
               {
                if( (pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt2,pnt3)) < 0 ) goto errexit  ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset1,pnt2,pnt3)) goto errexit  ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset2,pnt3,pnt2)) goto errexit  ;
                while( *(tinLineP+lineOffset1) == lineValue && *(tinLineP+lineOffset2) == lineValue )
                  {
                   if( (pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt2,pnt3)) < 0 ) goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset1,pnt2,pnt3)) goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset2,pnt3,pnt2)) goto errexit ;
                  }
                *(tinLineP+lineOffset1)  = nullValue ;
                if( pointCache.StorePointInCache(pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z)) goto errexit ;
                pnt4 = pnt2 ;
                pnt2 = pnt3 ;
                pnt3 = pnt4 ;
               } while ( pnt2 != pnt1 ) ;
               if( pointCache.StorePointInCache(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z)) goto errexit ;

//             Check For Duplicate Polygon Points

               if( cdbg )
                 {
                  numBefore = pointCache.SizeOfCache() ;
                  pointCache.RemoveDuplicatePoints() ;
                  numAfter  = pointCache.SizeOfCache() ;
                  if( numBefore != numAfter )
                    {
                     bcdtmWrite_message(1,0,0,"Corrupt Polygon") ;
                     goto errexit ;
                    }
                 }

//             Load Polygon

               if( pointCache.SizeOfCache()  >= 4 )
                 {
                  if( pointCache.CallUserDelegateWithCachePoints(loadFunctionP,dtmFeatureType,catchmentId,dtmP->nullFeatureId,userP)) goto errexit ;
                  ++numPolygons ;
                 }
               else
                 {
                  bcdtmWrite_message(1,0,0,"Not Enough Points In Catchment Polygon") ;
                  goto errexit ;
                 }
            }
         }
       clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
      }
   }

// Log Number Of Polygons

 if( dbg ) bcdtmWrite_message(0,0,0,"numPolygons = %8ld",numPolygons) ;

/*
** Clean Up
*/
 cleanup :
 if( tinLineP    != NULL ) free(tinLineP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Polygonising Triangle Index Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Polygonising Triangle Index Error") ;
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
int  bcdtmDrainage_catchmentCallBackFunction
(
 DTMFeatureType dtmFeatureType,
 DTMUserTag     userTag,
 DTMFeatureId   userFeatureId,
 DPoint3d       *featurePtsP,
 size_t         numFeaturePts,
 void           *userP
)
/*
**  This Function Recieves The Polygonised Catchments And Stores Them In A DTM Object
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 char dtmFeatureTypeName[100] ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Write Record
*/
 if( dtmFeatureType == DTMFeatureType::Catchment )
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Feature[%8ld] ** %s ** userTag = %10I64d userFeatureId = %10I64d featurePtsP = %p numFeaturePts = %6ld userP = %p",dtmFeatureType,dtmFeatureTypeName,userTag,userFeatureId,featurePtsP,numFeaturePts,userP) ;
/*
**  Store Catchment Polygon
*/
    dtmP = ( BC_DTM_OBJ *) userP ;
    if( dtmP != NULL )
      {
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,userTag,2,&userFeatureId,featurePtsP,(long)numFeaturePts)) goto errexit ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_traceCatchmentsDtmObject
(
 BC_DTM_OBJ          *dtmP,                   // ==> Pointer To Dtm Object
 DTMFeatureCallback  loadFunctionP,           // ==> Pointer To Load Function
 DTMDrainageTables   *drainageTablesP,        // ==> Pointer To Drainage Tables
 double              falseLowDepth,           // ==> False Low Depth
 bool                refineOption,            // ==> Refine The Catchment <true,false>
 bool                useFence,                // ==> Load Feature Within Fence
 DTMFenceType        fenceType,               // ==> Fence Type < 1 Rectangular , 2 Irregular Shape >
 DTMFenceOption      fenceOption,             // ==> Fence Option <INSIDE(1),OVERLAP(2),OUTSIDE(3)>
 DPoint3dCP          fencePtsP,               // ==> DPoint3d Array Of Fence Points
 int                 numFencePts,             // ==> Number Of Fence Points
 void                *userP,                  // ==> User Pointer Passed Back To User
 int&                numCatchments            // <== Number Of Catchments Determined
)

{
 int          ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
 long         numCatchPts,dtmFeature,startCatchment=0;
 DTMDirection direction ;
 long         descentTraceOverZeroSlope=FALSE,maxSpots=10000 ;
 long         startPnt=0,insert=0  ;
 long         catchmentNum=0 ;
 DPoint3d     *catchPtsP=nullptr ;
 double       area,largestArea=0.0 ;
 BC_DTM_OBJ   *catchmentDtmP=NULL,*cloneDtmP=NULL ;
 DTMUserTag   catchmentNumber=0 ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
 DTMDrainageTables* refinedTablesP=nullptr ;

// Log Entry Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Catchments") ;
    bcdtmWrite_message(0,0,0,"DTM Object      = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"falseLowDepth   = %8.4lf",falseLowDepth) ;
    bcdtmWrite_message(0,0,0,"refineOption    = %8d",refineOption) ;
    bcdtmWrite_message(0,0,0,"useFence        = %8d",useFence)  ;
    bcdtmWrite_message(0,0,0,"fenceType       = %8d",fenceType)  ;
    bcdtmWrite_message(0,0,0,"fenceOption     = %8d",fenceOption)  ;
    bcdtmWrite_message(0,0,0,"fencePtsP       = %p",fencePtsP)  ;
    bcdtmWrite_message(0,0,0,"numFencePts     = %8d",numFencePts)  ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP)  ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol     = %15.12lf",dtmP->ppTol)  ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol     = %15.12lf",dtmP->plTol)  ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol    = %15.12lf",dtmP->mppTol)  ;
   }

// Initialise

 numCatchments = 0 ;

// Test For Valid Dtm Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;

// Check If DTM Is In Tin State

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check DTM Tolerances
*/
 if( dtmP->ppTol < dtmP->mppTol * 10000.0 || dtmP->plTol < dtmP->mppTol * 10000.0 )
   {
    dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 1000.0 ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"dtmP->ppTol  = %15.12lf",dtmP->ppTol)  ;
       bcdtmWrite_message(0,0,0,"dtmP->plTol  = %15.12lf",dtmP->plTol)  ;
       bcdtmWrite_message(0,0,0,"dtmP->mppTol = %15.12lf",dtmP->mppTol) ;
      }
   }

// During Devlopement Read The alreay Created Cathment Boundaries From File

 bool refineOnly=true ;

 if( ! refineOnly )
     {

//   Create DTM To Store Catchment Polygons

     if( bcdtmObject_createDtmObject(&catchmentDtmP)) goto errexit ;

//   Get Initial Catchment Boundaries

     if( dbg ) bcdtmWrite_message(0,0,0,"Determining Initial Catchment Polygons") ;
     if( bcdtmDrainage_determineCatchmentsDtmObject(dtmP,(DTMFeatureCallback) bcdtmDrainage_catchmentCallBackFunction,drainageTablesP,falseLowDepth,useFence,fenceType,fenceOption,fencePtsP,numFencePts,catchmentDtmP,numCatchments)) goto errexit ;
     if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Initial Catchment Polygons = %6ld",numCatchments) ;

//   Save The Catchments To File

     if( bcdtmWrite_toFileDtmObject(catchmentDtmP,L"testCatchment.bcdtm") ) goto errexit ;
     }
 else
     {
      if( bcdtmRead_fromFileDtmObject(&catchmentDtmP,L"testCatchment.bcdtm")) goto errexit ;
     }
/*
**  Refine Catchment Polygons
*/
 if( refineOption )
   {

//  Clone DTM

    if( bcdtmObject_cloneDtmObject(dtmP,&cloneDtmP)) goto errexit ;

//  Refine Catchment Boundaries

    if( dbg )  bcdtmWrite_message(0,0,0,"Refining Catchments") ;
    if( bcdtmDrainage_refineCatchmentBoundariesDtmObject(cloneDtmP,catchmentDtmP,0.0,1)) goto errexit ;

//  Round Elevation Values To 8 Decimal Places

    int pnt ;
    for( pnt = 0 ; pnt < cloneDtmP->numPoints ; ++pnt )
        {
         pointAddrP(cloneDtmP,pnt)->z = bcdtmMath_roundToDecimalPoints(pointAddrP(cloneDtmP,pnt)->z,8) ;
        }

//  Recalculate Catchments From Refined Tin

    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Drainage Tables For Refined Tin") ;
    refinedTablesP = new DTMDrainageTables(cloneDtmP) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Size Of Drainage Tables Triangle Index = %8d",refinedTablesP->SizeOfTriangleIndex() ) ;
    bcdtmObject_initialiseDtmObject(catchmentDtmP) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Determining Refined Catchment Polygons") ;
    if( bcdtmDrainage_determineCatchmentsDtmObject(cloneDtmP,(DTMFeatureCallback) bcdtmDrainage_catchmentCallBackFunction,refinedTablesP,falseLowDepth,useFence,fenceType,fenceOption,fencePtsP,numFencePts,catchmentDtmP,numCatchments)) goto errexit ;
 //   if( bcdtmDrainage_determineCatchmentsDtmObject(cloneDtmP,(DTMFeatureCallback) bcdtmDrainage_catchmentCallBackFunction,nullptr,falseLowDepth,useFence,fenceType,fenceOption,fencePtsP,numFencePts,catchmentDtmP,numCatchments)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Refined Catchment Polygons = %6ld",numCatchments) ;
    bcdtmObject_destroyDtmObject(&cloneDtmP) ;
   }
/*
**  Load The Catchment Polygons
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Catchment Polygons ** Number Of Catchments = %8ld",catchmentDtmP->numFeatures) ;
 for( dtmFeature = 0 ; dtmFeature < catchmentDtmP->numFeatures ; ++dtmFeature )
   {
    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(catchmentDtmP,dtmFeature,&catchPtsP,&numCatchPts)) goto errexit ;
    bcdtmMath_getPolygonDirectionP3D(catchPtsP,numCatchPts,&direction,&area) ;
    if( direction == DTMDirection::Clockwise ) bcdtmMath_reversePolygonDirectionP3D(catchPtsP,numCatchPts) ;
    if( loadFunctionP(DTMFeatureType::Catchment,catchmentNumber,DTM_NULL_FEATURE_ID,catchPtsP,numCatchPts,userP)) goto errexit ;
    ++catchmentNumber ;
   }
/*
** Load Zero Slope Polygons
*/
// if( dbg ) bcdtmWrite_message(0,0,0,"Loading Zero Slope Polygons") ;
// if( bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject(dtmP,DTMFeatureType::ZeroSlopePolygon,maxSpots,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP)) goto errexit ;
/*
** Load Voids
*/
// if( dbg ) bcdtmWrite_message(0,0,0,"Loading Voids") ;
// if( bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject(dtmP,DTMFeatureType::Void,maxSpots,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( catchPtsP     != NULL ) { free(catchPtsP) ; catchPtsP = NULL ; }
 if( catchmentDtmP != NULL ) bcdtmObject_destroyDtmObject(&catchmentDtmP) ;
 if( cloneDtmP     != NULL ) bcdtmObject_destroyDtmObject(&cloneDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Catchments Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Catchments Error") ;
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
int bcdtmDrainage_refineCatchmentBoundariesDtmObject
(
 BC_DTM_OBJ   *dtmP,                      // ==> Pointer To DTM Object
 BC_DTM_OBJ   *catchmentDtmP,             // ==> Pointer To DTM Object With The Un Refined Catchment Boundaries
 double       falseLowDepth,              // ==> False Low Depth
 long         descentTraceOverZeroSlope   // ==> Trace Over Zero Slope Triangles
)
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long     dtmFeature,startPnt,numCatchments=-1,pnt1,pnt2,numCatchPts ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DPoint3d     *p3dP,*catchPtsP=NULL ;

 long maxCatchment=0 ;
 double x,y,z,area,maxArea=0.0 ;
 DTMDirection direction ;
 int numRidgeLines=0,numSumpLines=0,numLines=0,numEdgeLines=0,numCrossLines=0,numFlowLines=0,numLowPoints=0 ;
 int pnt,antPnt,clkPnt,insertStartPoint,numAscentsInserted=0 ;
 DTMFeatureType lineType;
 DTMRidgeLineCache ridgeLineCache,edgeLineCache ;
 DTMCatchmentLine  *catchmentLineP,*pLineP,*nLineP=NULL ;
 DTMCatchmentLineCache  catchmentLineCache,tempLineCache ;
 long lowPnt1,lowPnt2,testFeature ;
 int catchmentId ;
 double xLow,yLow,zLow,xHigh,yHigh,zHigh,xMid,yMid,zMid ;
 long catchmentPoint,noneCatchmentPoint,numCatchmentPoints ;
 bool flowLine=false,maxAscent=false,lowPoint=false ;
 bool processRidgeLines=false,processFlowLines=false,processLowPoints=false,processCrossFlow=false ;

 DPoint3d  dtmPnt ;

// Log Arguments

 if( dbg )
     {
     bcdtmWrite_message(0,0,0,"Refining Catchment Boundaries") ;
     bcdtmWrite_message(0,0,0,"dtmP                      = %p",dtmP) ;
     bcdtmWrite_message(0,0,0,"catchmentDtmP             = %p",catchmentDtmP) ;
     bcdtmWrite_message(0,0,0,"falseLowDepth             = %8.4lf",falseLowDepth) ;
     bcdtmWrite_message(0,0,0,"descentTraceOverZeroSlope = %8ld",descentTraceOverZeroSlope) ;
     }

//  Insert Unrefined Catchment Boundaries Into Tin

    bcdtmWrite_message(0,0,0,"Inserting Unrefined Catchments Into Tin") ;
    for( dtmFeature = 0 ; dtmFeature < catchmentDtmP->numFeatures ; ++dtmFeature )
      {
       if( dbg == 1 && dtmFeature % 1000 == 0) bcdtmWrite_message(0,0,0,"**** Processing Unrefined Catchment %8ld of %8ld",dtmFeature,catchmentDtmP->numFeatures) ;
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(catchmentDtmP,dtmFeature,&catchPtsP,&numCatchPts)) goto errexit ;
       bcdtmMath_getPolygonDirectionP3D(catchPtsP,numCatchPts,&direction,&area) ;
       if( direction == DTMDirection::Clockwise ) bcdtmMath_reversePolygonDirectionP3D(catchPtsP,numCatchPts) ;

//     Insert Feature Into Tin As A Tptr Polygon

       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Inserting catchment %8ld of %8ld",dtmFeature+1,catchmentDtmP->numFeatures) ;
       bcdtmFind_closestPointDtmObject(dtmP,catchPtsP->x,catchPtsP->y,&pnt1) ;
       startPnt = pnt1 ;
       for( p3dP = catchPtsP + 1 ; p3dP < catchPtsP + numCatchPts  ; ++p3dP )
         {
          bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&pnt2) ;
          nodeAddrP(dtmP,pnt1)->tPtr = pnt2 ;
          pnt1 = pnt2 ;
         }

//     Check Connectivity

       if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0))
         {
          bcdtmWrite_message(1,0,0,"Connectivity Error In Tptr Polygon") ;
          goto errexit ;
         }

//     Add Catchment Feature To DTM

       if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Catchment,ftableAddrP(catchmentDtmP,dtmFeature)->dtmUserTag,dtmP->nullFeatureId,startPnt,1)) goto errexit ;

      }

//  Get Largest Area Catchment - Development Purposes Only

 testFeature = dtmP->nullPnt ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
     {
     dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
     if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Catchment )
         {

         //  Copy Catchment Boundary To Tptr Polygon

         if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&startPnt)) goto errexit ;

         //  Scan Tptr Looking For Line On Unrefined Catchment

         int nxt=0 ;
         pnt = startPnt ;
         DPoint3d findLine[2] ;
         findLine[0].x = 2138937.22  ; findLine[0].y = 222505.12 ; findLine[0].z = 0.0 ;
         findLine[1].x = 2138943.24  ; findLine[1].y = 222511.73 ; findLine[1].z = 0.0 ;
         findLine[0].x = 2138768.41  ; findLine[0].y = 222563.49 ; findLine[0].z = 0.0 ;
         findLine[1].x = 2138771.65  ; findLine[1].y = 222569.22 ; findLine[1].z = 0.0 ;
         findLine[0].x = 2139165.22  ; findLine[0].y = 222757.78 ; findLine[0].z = 0.0 ;
         findLine[1].x = 2139161.62  ; findLine[1].y = 222755.00 ; findLine[1].z = 0.0 ;
         do
            {
             nxt = nodeAddrP(dtmP,pnt)->tPtr ;
            if(  bcdtmMath_distance(findLine[0].x,findLine[0].y,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y) < 0.001 ||
                 bcdtmMath_distance(findLine[1].x,findLine[1].y,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y) < 0.001     )
              {
                 bcdtmWrite_message(0,0,0,"dtmFeature = %8ld ** pnt = %8ld ** %12.4lf %12.4lf %10.4lf",dtmFeature,pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
 //                bcdtmList_writeTptrListDtmObject(dtmP,pnt) ;
              }
            if( ( bcdtmMath_distance(findLine[0].x,findLine[0].y,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y) < 0.001 &&
                  bcdtmMath_distance(findLine[1].x,findLine[1].y,pointAddrP(dtmP,nxt)->x,pointAddrP(dtmP,nxt)->y) < 0.001     ) ||
                ( bcdtmMath_distance(findLine[0].x,findLine[0].y,pointAddrP(dtmP,nxt)->x,pointAddrP(dtmP,nxt)->y) < 0.001 &&
                  bcdtmMath_distance(findLine[1].x,findLine[1].y,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y) < 0.001     )
               )

                 {
                 bcdtmWrite_message(0,0,0,"dtmFeature = %8ld ** pnt = %8ld tPtr = %8ld ** %12.4lf %12.4lf %10.4lf",dtmFeature,pnt,nodeAddrP(dtmP,pnt)->tPtr,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z) ;
                 bcdtmWrite_message(0,0,0,"                      ** nxt = %8ld tPtr = %8ld ** %12.4lf %12.4lf %10.4lf",nxt,nodeAddrP(dtmP,nxt)->tPtr,pointAddrP(dtmP,nxt)->x,pointAddrP(dtmP,nxt)->y,pointAddrP(dtmP,nxt)->z) ;
                 if( testFeature == dtmP->nullPnt ) testFeature = dtmFeature ;
                 bcdtmWrite_message(0,0,0,"Test Feature = %8ld",testFeature) ;
                 }
             pnt = nxt ;
            }  while ( pnt != startPnt ) ;

  //       bcdtmWrite_message(0,0,0,"testFeature = %8ld",testFeature) ;

         bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ;

         if( area > maxArea )
             {
              maxArea = area ;
              maxCatchment = dtmFeature ;
              bcdtmWrite_message(0,0,0,"maxArea = %12.4lf ** largestArea Catchment = %8ld ** Catchment Id = %10I64d ** startPnt = %12.5lf %12.5lf %10.4lf",area,maxCatchment,dtmFeatureP->dtmUserTag,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,startPnt)->z) ;
             }

          bcdtmList_nullTptrListDtmObject(dtmP,startPnt) ;

         }
    }

// if( 1) goto errexit ;
 if( testFeature == dtmP->nullPnt ) goto errexit ;

//  Scan Unrefined Catchment Boundaries
bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld",dtmP->numPoints) ;
//  for( dtmFeature = testFeature ; dtmFeature <= testFeature ; ++dtmFeature )
  for( dtmFeature = 0 ; dtmFeature <= dtmP->numFeatures ; ++dtmFeature )
     {
     dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
     if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Catchment )
         {

         ++numCatchments ;
         if( dbg == 2 && numCatchments % 100 == 0 ) bcdtmWrite_message(0,0,0,"Refining Catchment %8ld of %8ld  ** dtmP->numPoints = %8ld",numCatchments,dtmP->numFeatures,dtmP->numPoints) ;

         //  Copy Catchment Boundary To Tptr Polygon

         if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&startPnt)) goto errexit ;

         //  Get Stats On Catchment Boundary

         pnt1 = startPnt ;
         do
             {
              ++numLines ;
              pnt2 = nodeAddrP(dtmP,pnt1)->tPtr ;

              //  Dont Process Hull Lines

              if( ! bcdtmList_testForHullLineDtmObject(dtmP,pnt1,pnt2) )
                  {

                  // Get Points On Oppsite Sides Of Line

                  if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
                  if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;

                  // Determine Draiange Line Type

                  if( bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP,nullptr,pnt1,pnt2,lineType)) goto errexit ;

                  if( lineType != DTMFeatureType::None )
                      {
                      if( tempLineCache.StoreLineInCache(dtmP,( DTMFeatureType) lineType,dtmFeatureP->dtmUserTag,pnt1,pnt2,antPnt,clkPnt)) goto errexit ;
                      }

                  // Accumulate Stats On Different Line Types

                  if( lineType == DTMFeatureType::RidgeLine )
                      {
                      ++numRidgeLines ;
                      if( dbg == 2 )
                          {
                          bcdtmWrite_message(0,0,0,"Ridge Line   ****  %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
                          bcdtmWrite_message(0,0,0,"                   %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z) ;
                          }
                      }
                  if( lineType == DTMFeatureType::SumpLine  )
                      {
                      ++numSumpLines ;
                      if( dbg == 2 )
                          {
                          bcdtmWrite_message(0,0,0,"SumP  Line   ****  %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
                          bcdtmWrite_message(0,0,0,"                   %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z) ;
                          }
                      }

                  if( lineType == DTMFeatureType::CrossLine )
                      {
                      ++numCrossLines ;
                      if( dbg == 2 )
                          {
                          bcdtmWrite_message(0,0,0,"Cross Line ****  %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
                          bcdtmWrite_message(0,0,0,"                 %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z) ;
                          }
                      }

                  }

              pnt1 = pnt2 ;

             }  while( pnt1 != startPnt ) ;

          //  Log Temp Lines

          if( dbg == 2 ) tempLineCache.LogCacheLines() ;

          //  Scan Temp Line Cache And Copy All Ridge Lines

          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Ridge Lines") ;
          for( catchmentLineP = tempLineCache.FirstLine() ; catchmentLineP <= tempLineCache.LastLine() ; ++catchmentLineP )
             {
             if( catchmentLineP->edgeType == DTMFeatureType::RidgeLine )
                 {
                 if( catchmentLineCache.StoreLineInCache(dtmP,DTMFeatureType::RidgeLine,catchmentLineP->catchmentId,catchmentLineP->startPoint,catchmentLineP->endPoint,catchmentLineP->ccwPoint,catchmentLineP->clkPoint)) goto errexit ;
                 }
             }

          //  Scan Temp Line Cache And Change Cross Flow Lines That Connect To A Ridge Or Sump Line To Flow Lines

          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Flow Lines") ;
          for( catchmentLineP = tempLineCache.FirstLine() ; catchmentLineP <= tempLineCache.LastLine() ; ++catchmentLineP )
             {
             if( catchmentLineP->edgeType == DTMFeatureType::CrossLine )
                 {

                 if( dbg == 2 )
                     {
                     bcdtmWrite_message(0,0,0,"Cross Flow Line  = %8ld",(long)(catchmentLineP-tempLineCache.FirstLine())) ;
                     bcdtmWrite_message(0,0,0,"startPoint = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->startPoint,pointAddrP(dtmP,catchmentLineP->startPoint)->x,pointAddrP(dtmP,catchmentLineP->startPoint)->y,pointAddrP(dtmP,catchmentLineP->startPoint)->z) ;
                     bcdtmWrite_message(0,0,0,"endPoint   = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->endPoint,pointAddrP(dtmP,catchmentLineP->endPoint)->x,pointAddrP(dtmP,catchmentLineP->endPoint)->y,pointAddrP(dtmP,catchmentLineP->endPoint)->z) ;
                     }


                 //  Check For Prior Or Next Ridge Or Sump Line

                 flowLine = false ;
                 pLineP = catchmentLineP - 1 ;
                 if( pLineP < tempLineCache.FirstLine() ) pLineP = tempLineCache.LastLine() ;
                 nLineP = catchmentLineP + 1 ;
                 if( nLineP > tempLineCache.LastLine()  ) nLineP = tempLineCache.FirstLine() ;
                 if( pLineP->edgeType == DTMFeatureType::RidgeLine || nLineP->edgeType == DTMFeatureType::RidgeLine ||
                     pLineP->edgeType == DTMFeatureType::SumpLine  || nLineP->edgeType == DTMFeatureType::SumpLine     )
                     {

                     // Check For Maximum Ascent Flow From Triangle

                     if( bcdtmDrainage_checkForMaximumAscentFlowLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->endPoint,maxAscent)) goto errexit ;
                     if( maxAscent )
                         {
                         flowLine = true ;
                         if( catchmentLineCache.StoreLineInCache(dtmP,DTMFeatureType::FlowLine,catchmentLineP->catchmentId,catchmentLineP->startPoint,catchmentLineP->endPoint,catchmentLineP->ccwPoint,catchmentLineP->clkPoint)) goto errexit ;
                         if( dbg == 2 )  bcdtmWrite_message(0,0,0,"**** Flow Line Detected") ;
                         catchmentLineP->edgeStatus = 0 ;
                         }
                     }
                 }
             }

          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Flow Lines Completed") ;

          //  Scan Cross Flow Lines InTemp Line Cache And Store Low Points

          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Catchment Low Points") ;
          for( catchmentLineP = tempLineCache.FirstLine() ; catchmentLineP <= tempLineCache.LastLine() ; ++catchmentLineP )
             {
             if( catchmentLineP->edgeStatus && catchmentLineP->edgeType == DTMFeatureType::CrossLine )
                 {
                 if( dbg == 2 )
                     {
                     bcdtmWrite_message(0,0,0,"Cross Flow Line = %8ld",(long)(catchmentLineP-tempLineCache.FirstLine())) ;
                     bcdtmWrite_message(0,0,0,"startPoint = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->startPoint,pointAddrP(dtmP,catchmentLineP->startPoint)->x,pointAddrP(dtmP,catchmentLineP->startPoint)->y,pointAddrP(dtmP,catchmentLineP->startPoint)->z) ;
                     bcdtmWrite_message(0,0,0,"endPoint   = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->endPoint,pointAddrP(dtmP,catchmentLineP->endPoint)->x,pointAddrP(dtmP,catchmentLineP->endPoint)->y,pointAddrP(dtmP,catchmentLineP->endPoint)->z) ;
                     }

                 //  Only Process None Flat Line

                 if( pointAddrP(dtmP,catchmentLineP->startPoint)->z < pointAddrP(dtmP,catchmentLineP->endPoint)->z )
                   {
                   lowPoint = false ;
                   pLineP = catchmentLineP - 1 ;
                   if( pLineP < tempLineCache.FirstLine() ) pLineP = tempLineCache.LastLine() ;
                   if( pLineP->edgeType == DTMFeatureType::CrossLine && pointAddrP(dtmP,nLineP->startPoint)->z > pointAddrP(dtmP,catchmentLineP->startPoint)->z )
                       {
                       lowPoint = true ;
                       if( catchmentLineCache.StoreLineInCache(dtmP,DTMFeatureType::LowPoint,catchmentLineP->catchmentId,catchmentLineP->startPoint,catchmentLineP->endPoint,pLineP->startPoint,pLineP->startPoint)) goto errexit ;
                       if( dbg == 2 )  bcdtmWrite_message(0,0,0,"**** Low Point Detected") ;
                       }
                   }

                 //  Store As Cross Flow Line If Not A Low Point

                 if( ! lowPoint )
                     {
                     if( catchmentLineCache.StoreLineInCache(dtmP,catchmentLineP->edgeType,catchmentLineP->catchmentId,catchmentLineP->startPoint,catchmentLineP->endPoint,catchmentLineP->ccwPoint,catchmentLineP->clkPoint)) goto errexit ;
                     }
                 }
             }

          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Catchment Low Points Completed") ;


          //  Release Tempoary Catchment Lines

          tempLineCache.ClearCache() ;

          //  Null Tptr polygon

          if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
         }
      }

   // Log Stats On Line Types

   if( dbg == 1  )
       {
       numLines = numRidgeLines = numSumpLines = numFlowLines = numCrossLines = numLowPoints = 0 ;
       for( catchmentLineP = catchmentLineCache.FirstLine() ; catchmentLineP <= catchmentLineCache.LastLine() ; ++catchmentLineP )
           {
           ++numLines ;
           if( catchmentLineP->edgeType == DTMFeatureType::RidgeLine ) ++numRidgeLines ;
           if( catchmentLineP->edgeType == DTMFeatureType::SumpLine  ) ++numSumpLines  ;
           if( catchmentLineP->edgeType == DTMFeatureType::FlowLine  ) ++numFlowLines  ;
           if( catchmentLineP->edgeType == DTMFeatureType::CrossLine ) ++numCrossLines ;
           if( catchmentLineP->edgeType == DTMFeatureType::LowPoint  ) ++numLowPoints  ;
           }
       bcdtmWrite_message(0,0,0,"numLines = %8ld ** numRidgeLines = %8ld numSumpLines = %8ld numFlowLines = %8ld numCrossLines = %8d numLowPoints = %8ld",numLines,numRidgeLines,numSumpLines,numFlowLines,numCrossLines,numLowPoints) ;
       }

   //  Log Catchment lines

   if( dbg == 2 ) catchmentLineCache.LogCacheLines() ;

   //  Sort And Remove Duplicate Catchment Lines

   if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Sorting And Removing Duplicates")  ;
   catchmentLineCache.SortAndRemoveDuplicates() ;
   if( dbg == 2 ) catchmentLineCache.LogCacheLines() ;

   // Sort On Increasing Elevation

   if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Sorting On Increasing Elevation")  ;
   catchmentLineCache.SortOnAscendingElevation() ;
   if( dbg == 2 ) catchmentLineCache.LogCacheLines() ;

   //  Scan Catchment Lines And Insert Maximum Ascents From Ridge Line High Points

   processRidgeLines = false ;
   if( processRidgeLines )
       {
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Ridge Lines") ;
       numAscentsInserted = 0 ;
       for( catchmentLineP = catchmentLineCache.FirstLine() ; catchmentLineP <= catchmentLineCache.LastLine() ; ++catchmentLineP )
            {
            if( catchmentLineP->edgeType == DTMFeatureType::RidgeLine  && bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->endPoint ))
                {
                insertStartPoint = catchmentLineP->startPoint ;
                if( pointAddrP(dtmP,catchmentLineP->startPoint)->z < pointAddrP(dtmP,catchmentLineP->endPoint)->z )
                    insertStartPoint = catchmentLineP->endPoint ;
                if( insertStartPoint != dtmP->nullPnt )
                    {
                    if( dbg && numAscentsInserted % 5000 == 0 ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent From Ridge Line ** %8ld %8ld ** dtmP->numPoints = %8ld dtmP->numSortedPoints = %8ld",catchmentLineP->startPoint,catchmentLineP->endPoint,dtmP->numPoints,dtmP->numSortedPoints) ;
                    if( bcdtmDrainage_insertMaximumAscentLineFromTinPointDtmObject(dtmP,insertStartPoint)) goto errexit ;
                    ++numAscentsInserted ;
                    }
                 catchmentLineP->edgeStatus = 0 ;
                 }
             }
        if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Maximum Inserts Inserted From Ridge Lines = %8ld",numAscentsInserted) ;
        }

   //  Log Flow Lines

   processFlowLines = true ;
   if( processFlowLines )
       {
       if( dbg == 2 )
           {
           for( catchmentLineP = catchmentLineCache.FirstLine() ; catchmentLineP <= catchmentLineCache.LastLine() ; ++catchmentLineP )
               {
               if(  catchmentLineP->edgeType == DTMFeatureType::FlowLine )
                   {
                   bcdtmWrite_message(0,0,0,"Catchment Flow Line = %8ld",(long)(catchmentLineP-catchmentLineCache.FirstLine())) ;
                   bcdtmWrite_message(0,0,0,"startPoint = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->startPoint,pointAddrP(dtmP,catchmentLineP->startPoint)->x,pointAddrP(dtmP,catchmentLineP->startPoint)->y,pointAddrP(dtmP,catchmentLineP->startPoint)->z) ;
                   bcdtmWrite_message(0,0,0,"endPoint   = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->endPoint,pointAddrP(dtmP,catchmentLineP->endPoint)->x,pointAddrP(dtmP,catchmentLineP->endPoint)->y,pointAddrP(dtmP,catchmentLineP->endPoint)->z) ;
                   }
               }
           }

       //  Scan Catchment Lines And Insert Maximum Ascents From Flow Lines

       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Flow Lines") ;
       numAscentsInserted = 0 ;
       for( catchmentLineP = catchmentLineCache.FirstLine() ; catchmentLineP <= catchmentLineCache.LastLine() ; ++catchmentLineP )
           {
            if(  catchmentLineP->edgeType == DTMFeatureType::FlowLine )
                 {

if( (long)(catchmentLineP-catchmentLineCache.FirstLine()) == 28 )
{
bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent From Flow Line[%8ld] ** %8ld %8ld ** Status = %2d",(long)(catchmentLineP-catchmentLineCache.FirstLine()),catchmentLineP->startPoint,catchmentLineP->endPoint,catchmentLineP->edgeStatus) ;
bcdtmList_writeCircularListForPointDtmObject(dtmP,catchmentLineP->startPoint) ;
bcdtmList_writeCircularListForPointDtmObject(dtmP,catchmentLineP->endPoint) ;
//goto errexit ;
}



                 if( catchmentLineP->edgeStatus )
                     {

                     // Check For Prior Inserted Maximum Ascent Line
/*
                     if( bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->endPoint) &&
                         bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->clkPoint) &&
                         bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->ccwPoint) &&
                         bcdtmList_testLineDtmObject(dtmP,catchmentLineP->endPoint  ,catchmentLineP->clkPoint) &&
                         bcdtmList_testLineDtmObject(dtmP,catchmentLineP->endPoint  ,catchmentLineP->ccwPoint)       )
*/

                       if( bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->endPoint))
                          {

                          // Log Flow Line

                          if( dbg == 2 )
                              {
                              bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent From Flow Line[%8ld] ** %8ld %8ld",(long)(catchmentLineP-catchmentLineCache.FirstLine()),catchmentLineP->startPoint,catchmentLineP->endPoint) ;
                              bcdtmWrite_message(0,0,0,"startPoint = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->startPoint,pointAddrP(dtmP,catchmentLineP->startPoint)->x,pointAddrP(dtmP,catchmentLineP->startPoint)->y,pointAddrP(dtmP,catchmentLineP->startPoint)->z) ;
                              bcdtmWrite_message(0,0,0,"endPoint   = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->endPoint,pointAddrP(dtmP,catchmentLineP->endPoint)->x,pointAddrP(dtmP,catchmentLineP->endPoint)->y,pointAddrP(dtmP,catchmentLineP->endPoint)->z) ;
                              }

                          // Insert Maximum Ascent Line

                          if( bcdtmDrainage_insertMaximumAscentLineFromTriangleEdgeDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->endPoint)) goto errexit ;
                          ++numAscentsInserted ;
                          }
                     }
                 catchmentLineP->edgeStatus = 0 ;
                 }
             }
         if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Maximum Ascents Inserted From Flow Lines = %8ld",numAscentsInserted) ;
        }

    // Maximum Ascents From Low Points On Unrefinded Catchment Boundary

    processLowPoints = false ;
    if( processLowPoints )
        {

        //  Log Low Points

        if( dbg == 2 )
            {
            bcdtmWrite_message(0,0,0,"Low Points On Catchment Lines For Inserting Maximum Ascents") ;
            for( catchmentLineP = catchmentLineCache.FirstLine() ; catchmentLineP <= catchmentLineCache.LastLine() ; ++catchmentLineP )
                {
                if(  catchmentLineP->edgeType == DTMFeatureType::LowPoint )
                    {
                    bcdtmWrite_message(0,0,0,"Catchment Low Point Line = %8ld",(long)(catchmentLineP-catchmentLineCache.FirstLine())) ;
                    bcdtmWrite_message(0,0,0,"startPoint = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->startPoint,pointAddrP(dtmP,catchmentLineP->startPoint)->x,pointAddrP(dtmP,catchmentLineP->startPoint)->y,pointAddrP(dtmP,catchmentLineP->startPoint)->z) ;
                    bcdtmWrite_message(0,0,0,"endPoint   = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->endPoint,pointAddrP(dtmP,catchmentLineP->endPoint)->x,pointAddrP(dtmP,catchmentLineP->endPoint)->y,pointAddrP(dtmP,catchmentLineP->endPoint)->z) ;
                    }
                }
            }

        //  Scan Catchment Lines And Insert Maximum Ascents From Low Points

        if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Low Points") ;
        numAscentsInserted = 0 ;
        for( catchmentLineP = catchmentLineCache.FirstLine() ; catchmentLineP <= catchmentLineCache.LastLine() ; ++catchmentLineP )
            {
            if( catchmentLineP->edgeStatus && catchmentLineP->edgeType == DTMFeatureType::LowPoint )
                {
                if( bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->endPoint) &&
                    bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->ccwPoint)      )
                    {

                    // Log Low Point Lines

                    if( dbg == 2 )
                        {
                        bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent From Low Point Line[%8ld] ** %8ld %8ld",(long)(catchmentLineP-catchmentLineCache.FirstLine()),catchmentLineP->startPoint,catchmentLineP->endPoint) ;
                        bcdtmWrite_message(0,0,0,"priorPoint = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->ccwPoint,pointAddrP(dtmP,catchmentLineP->ccwPoint)->x,pointAddrP(dtmP,catchmentLineP->ccwPoint)->y,pointAddrP(dtmP,catchmentLineP->ccwPoint)->z) ;
                        bcdtmWrite_message(0,0,0,"startPoint = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->startPoint,pointAddrP(dtmP,catchmentLineP->startPoint)->x,pointAddrP(dtmP,catchmentLineP->startPoint)->y,pointAddrP(dtmP,catchmentLineP->startPoint)->z) ;
                        bcdtmWrite_message(0,0,0,"endPoint   = %8ld ** %12.4lf %12.4lf %10.4lf",catchmentLineP->endPoint,pointAddrP(dtmP,catchmentLineP->endPoint)->x,pointAddrP(dtmP,catchmentLineP->endPoint)->y,pointAddrP(dtmP,catchmentLineP->endPoint)->z) ;
                        }

                    // Insert Maximum Ascents From The Prior And Next Triangle Edges About The Low Point

                    if( bcdtmDrainage_insertMaximumAscentLineFromTriangleEdgeDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->endPoint)) goto errexit ;
                    if( bcdtmDrainage_insertMaximumAscentLineFromTriangleEdgeDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->ccwPoint)) goto errexit ;
                    ++numAscentsInserted ;
                    }

                 // Mark Low Point As Processed

                 catchmentLineP->edgeStatus = 0 ;
                 }
            }

        if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Maximum Ascents Inserted From Low Points = %8ld",numAscentsInserted) ;
        }


    //  Maximum Ascents From Cross Flow Lines

    processCrossFlow = false ;
    if( processCrossFlow )
        {

        // Log Number Of Cross Flow Lines To Be Inserted

        if( dbg == 1 )
            {
            numAscentsInserted = 0 ;
            for( catchmentLineP = catchmentLineCache.FirstLine() ; catchmentLineP <= catchmentLineCache.LastLine() ; ++catchmentLineP )
                {
                if( catchmentLineP->edgeStatus  &&  catchmentLineP->edgeType == DTMFeatureType::CrossLine )
                    {

                    // Check For Prior Inserted Maximum Ascent Line

                    if( bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->endPoint) &&
                        bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->clkPoint) &&
                        bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->ccwPoint) &&
                        bcdtmList_testLineDtmObject(dtmP,catchmentLineP->endPoint  ,catchmentLineP->clkPoint) &&
                        bcdtmList_testLineDtmObject(dtmP,catchmentLineP->endPoint  ,catchmentLineP->ccwPoint)       )
                        {
                        ++numAscentsInserted ;
                        }
                    }
                }
            bcdtmWrite_message(0,0,0,"Number Of Cross Flow Lines To Be Inserted = %8ld",numAscentsInserted) ;
            }

           // Insert Maximum Ascent Lines From Cross Flow Lines

           if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Cross Flow Lines") ;
           numAscentsInserted = 0 ;
           for( catchmentLineP = catchmentLineCache.FirstLine() ; catchmentLineP <= catchmentLineCache.LastLine() ; ++catchmentLineP )
               {

               // Only Process If Line Has Not Been Prior Processed

               if( catchmentLineP->edgeStatus  &&  catchmentLineP->edgeType == DTMFeatureType::CrossLine )
                   {

                   // Check For Prior Inserted Maximum Ascent Line

                   if( bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->endPoint) &&
                       bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->clkPoint) &&
                       bcdtmList_testLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->ccwPoint) &&
                       bcdtmList_testLineDtmObject(dtmP,catchmentLineP->endPoint  ,catchmentLineP->clkPoint) &&
                       bcdtmList_testLineDtmObject(dtmP,catchmentLineP->endPoint  ,catchmentLineP->ccwPoint)       )
                       {

                       // Initialise Counters For Binary Searching For Maximum Ascent Intersect Point

                       catchmentPoint     = dtmP->nullPnt ;
                       noneCatchmentPoint = dtmP->nullPnt ;
                       numCatchmentPoints = 0 ;

                      //  Check If Line Start Point Traces To Catchment

                      x = pointAddrP(dtmP,catchmentLineP->startPoint)->x ;
                      y = pointAddrP(dtmP,catchmentLineP->startPoint)->y ;
                      z = pointAddrP(dtmP,catchmentLineP->startPoint)->z ;
                      if( bcdtmDrainage_traceToLowPointDtmObject(dtmP,nullptr,nullptr,falseLowDepth,descentTraceOverZeroSlope,nullptr,catchmentLineP->startPoint,dtmP->nullPnt,dtmP->nullPnt,x,y,z,nullptr,&lowPnt1,&lowPnt2)) goto errexit ;
                      catchmentId = lowPnt1 ;
                      if( lowPnt2 != dtmP->nullPnt )
                          {
                          long index ;
                          if( lowPnt1 > lowPnt2 )
                             {
                             index   = lowPnt1 ;
                             lowPnt1 = lowPnt2 ;
                             lowPnt2 = index   ;
                             }
                          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&index,lowPnt1,lowPnt2)) goto errexit ;
                          catchmentId = dtmP->numPoints + index ;
                          }
                      if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Start Point Catchment Id = %10d",catchmentId) ;
                      if( catchmentId == catchmentLineP->catchmentId )
                          {
                          catchmentPoint = catchmentLineP->startPoint ;
                          ++numCatchmentPoints ;
                          }
                      else
                          {
                          noneCatchmentPoint = catchmentLineP->startPoint ;
                          }

                      //  Check If Line End Point Line Traces To Catchment

                      x = pointAddrP(dtmP,catchmentLineP->endPoint)->x ;
                      y = pointAddrP(dtmP,catchmentLineP->endPoint)->y ;
                      z = pointAddrP(dtmP,catchmentLineP->endPoint)->z ;
                      if( bcdtmDrainage_traceToLowPointDtmObject(dtmP,nullptr,nullptr,falseLowDepth,descentTraceOverZeroSlope,nullptr,catchmentLineP->endPoint,dtmP->nullPnt,dtmP->nullPnt,x,y,z,nullptr,&lowPnt1,&lowPnt2)) goto errexit ;
                      catchmentId = lowPnt1 ;
                      if( lowPnt2 != dtmP->nullPnt )
                          {
                          long index ;
                          if( lowPnt1 > lowPnt2 )
                              {
                              index   = lowPnt1 ;
                              lowPnt1 = lowPnt2 ;
                              lowPnt2 = index   ;
                              }
                          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&index,lowPnt1,lowPnt2)) goto errexit ;
                          catchmentId = dtmP->numPoints + index ;
                         }
                     if( catchmentId == catchmentLineP->catchmentId )
                         {
                         catchmentPoint = catchmentLineP->endPoint ;
                         ++numCatchmentPoints ;
                         }
                     else
                         {
                         noneCatchmentPoint = catchmentLineP->endPoint ;
                         }

                     // Log Results

                     if( dbg == 2 )
                         {
                         bcdtmWrite_message(0,0,0,"End   Point Catchment Id = %10d",catchmentId) ;
                         bcdtmWrite_message(0,0,0,"Catchment Line %8ld %8ld ** numCatchmentPoints = %2d",catchmentLineP->startPoint,catchmentLineP->endPoint,numCatchmentPoints) ;
                         }

                     //  Binary Search To Find Location Between Points To Start Maximum Ascent Insert

                     if( numCatchmentPoints == 1 )
                         {

                         // Log Line

                         if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Binary Searching For Maximum Ascent Start Insert ** catchMentPoint = %8ld noneCatchmentPoint = %8ld",catchmentPoint,noneCatchmentPoint) ;

                         // Set Variables For Binary Searching to Locate Maximum Ascent Insert Point

                         int loop = 0 ;
                         xLow  = pointAddrP(dtmP,catchmentPoint)->x ;
                         yLow  = pointAddrP(dtmP,catchmentPoint)->y ;
                         zLow  = pointAddrP(dtmP,catchmentPoint)->z ;
                         xHigh = pointAddrP(dtmP,noneCatchmentPoint)->x ;
                         yHigh = pointAddrP(dtmP,noneCatchmentPoint)->y ;
                         zHigh = pointAddrP(dtmP,noneCatchmentPoint)->z ;
                         while( bcdtmMath_distance(xLow,yLow,xHigh,yHigh) > dtmP->ppTol )
                             {
                             ++loop ;
                             if( loop > 100 ) goto errexit ;
                             xMid  = ( xLow + xHigh ) / 2.0 ;
                             yMid  = ( yLow + yHigh ) / 2.0 ;
                             zMid  = ( zLow + zHigh ) / 2.0 ;
                             if( bcdtmDrainage_traceToLowPointDtmObject(dtmP,nullptr,nullptr,falseLowDepth,descentTraceOverZeroSlope,nullptr,catchmentPoint,noneCatchmentPoint,dtmP->nullPnt,xMid,yMid,zMid,nullptr,&lowPnt1,&lowPnt2)) goto errexit ;
                             catchmentId = lowPnt1 ;
                             if( lowPnt2 != dtmP->nullPnt )
                                 {
                                 long index ;
                                 if( lowPnt1 > lowPnt2 )
                                     {
                                     index   = lowPnt1 ;
                                     lowPnt1 = lowPnt2 ;
                                     lowPnt2 = index   ;
                                     }
                                 if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&index,lowPnt1,lowPnt2)) goto errexit ;
                                 catchmentId = dtmP->numPoints + index ;
                                 }
                             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"** %12.5lf %12.5lf %10.4lf ** lowPointId = %10d catchmentId = %10d",xMid,yMid,zMid,catchmentId,catchmentLineP->catchmentId) ;
                             if( catchmentId == catchmentLineP->catchmentId )
                                 {
                                 xLow = xMid ;
                                 yLow = yMid ;
                                 zLow = zMid ;
                                 }
                             else
                                 {
                                 xHigh = xMid ;
                                 yHigh = yMid ;
                                 zHigh = zMid ;
                                 }
                             }

                         //  Log Maximum Ascent Start Point

                        if( dbg == 2 )
                            {
                            bcdtmWrite_message(0,0,0,"                    Line Start Point = %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,catchmentPoint)->x,pointAddrP(dtmP,catchmentPoint)->y,pointAddrP(dtmP,catchmentPoint)->z) ;
                            bcdtmWrite_message(0,0,0,"********* Maximum Ascent Start Point = %12.5lf %12.5lf %10.4lf",xLow,yLow,zLow) ;
                            bcdtmWrite_message(0,0,0,"                    Line End   Point = %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,noneCatchmentPoint)->x,pointAddrP(dtmP,noneCatchmentPoint)->y,pointAddrP(dtmP,noneCatchmentPoint)->z) ;
                            }

                        //  Insert Maximum Ascents Into Tin

                        dtmPnt.x = xLow ;
                        dtmPnt.y = yLow ;
                        dtmPnt.z = zLow ;
                        if( dbg && numAscentsInserted % 1000 == 0 ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent From Catchment Line ** %8ld %8ld ** dtmP->numPoints = %8ld dtmP->numSortedPoints = %8ld",catchmentLineP->startPoint,catchmentLineP->endPoint,dtmP->numPoints,dtmP->numSortedPoints) ;
                        if( bcdtmDrainage_insertMaximumAscentLineFromPointOnTinLineDtmObject(dtmP,catchmentLineP->startPoint,catchmentLineP->endPoint,dtmPnt)) goto errexit ;
                        ++numAscentsInserted ;
                        }
                    }
                catchmentLineP->edgeStatus = 0 ;
                }
            }

        if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Maximum Ascents Inserted From Cross Flow Lines = %8ld",numAscentsInserted) ;
        }

 //  Log Lines Not Used

    if( dbg == 2 )
        {
        bcdtmWrite_message(0,0,0,"None Processed Catchment Lines") ;
        for( catchmentLineP = catchmentLineCache.FirstLine() ; catchmentLineP <= catchmentLineCache.LastLine() ; ++catchmentLineP )
            {
            if( catchmentLineP->edgeStatus )
                {
                bcdtmWrite_message(0,0,0,"**** Offset = %8ld **  Catchment Line Type = %4ld",(int)(catchmentLineP-catchmentLineCache.FirstLine()),catchmentLineP->edgeType) ;
                bcdtmWrite_message(0,0,0,"** Start Point =  %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,catchmentLineP->startPoint)->x,pointAddrP(dtmP,catchmentLineP->startPoint)->y,pointAddrP(dtmP,catchmentLineP->startPoint)->z) ;
                bcdtmWrite_message(0,0,0,"** End   Point =  %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,catchmentLineP->endPoint)->x,pointAddrP(dtmP,catchmentLineP->endPoint)->y,pointAddrP(dtmP,catchmentLineP->endPoint)->z) ;
                }
            }
        }


//  Clean Dtm Object

    if( dbg ) bcdtmWrite_message(0,0,0,"After Inserting Maximum Ascents ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
    if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
    if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"refinedCatchment.tin") ;

//  Check DTM

    if( cdbg )
        {
         if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid DTM") ;
         if( bcdtmCheck_tinComponentDtmObject(dtmP))
             {
              bcdtmWrite_message(1,0,0,"DTM Invalid") ;
             }
         if( dbg ) bcdtmWrite_message(0,0,0,"DTM Valid") ;
        }

//  Clean Up

cleanup :

// Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Refining Catchment Boundaries Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Refining Catchment Boundaries Error") ;
    return(ret) ;

// Error Exit

errexit :
    ret = DTM_ERROR ;
    bcdtmList_nullTptrValuesDtmObject(dtmP) ;
    bcdtmList_nullSptrValuesDtmObject(dtmP) ;
    goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_determineRefinedCatchmentBoundaryDtmObject
(
 BC_DTM_OBJ   *dtmP,               /* ==> Pointer To Tin Object                           */
 BC_DTM_OBJ   *catchmentDtmP,      /* ==> Pointer To Un Refined Catchment DTM             */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                        */
 double       falseLowDepth,       /* ==> False Low Depth                                 */
 long         catchmentID,         /* ==> Catchment ID                                    */
 void         *userP,              /* ==> User Pointer Passed Back To User                */
 long         *numCatchmentsP      /* <== Number Of Catchments Determined                 */
)
/*
** This Function Determines The Tin Catchments
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    pnt,pnt1,pnt2,pnt3,pnt4,clPtr,trgPnt1,trgPnt2,trgPnt3,fndPnt,lowPnt1=0,lowPnt2=0;
 long    useTables=TRUE,dtmFeature,trgNum=0,traceOverZeroSlope=0  ;
 long    clkPnt,lineOffset,lineOffset1,lineOffset2,lineOffset3,flowPoint,numPolygons=0 ;
 long    *lineP,*tinLineP=NULL,nullValue=-9999999,lineValue,traceToLowPoint ;
 long    numFeaturePts=0,memFeaturePts=0,memFeaturePtsInc=1000,numBefore,numAfter ;
 double  x,y,sx,sy,area,startX,startY,startZ ;
 DPoint3d     *p3d1P,*p3d2P,*featurePtsP=NULL ;
 DTM_TIN_POINT *pnt1P,*pnt2P,*pnt3P ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Entry Message
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Determining Refined Catchments") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"catchmentDtmP   = %p",catchmentDtmP)  ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP)  ;
    bcdtmWrite_message(0,0,0,"falseLowDepth   = %8.4lf",falseLowDepth) ;
    bcdtmWrite_message(0,0,0,"catchmentID     = %8ld",catchmentID) ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP)  ;
   }
/*
** Initialise
*/
 *numCatchmentsP = 0 ;
 if( falseLowDepth < 0.0 ) falseLowDepth = 0.0 ;
/*
** Allocate Memory To Hold Values For Tin Lines
*/
 tinLineP  = ( long * ) malloc (catchmentDtmP->cListPtr * sizeof(long)) ;
 if( tinLineP  == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit  ;
   }
/*
** Set Tin Line Values To Null
*/
 for( lineP = tinLineP  ; lineP < tinLineP  + catchmentDtmP->cListPtr ; ++lineP ) *lineP = nullValue ;
/*
** Set Catchment Id For Triangles That Have Not Been Sub Divided By The Insertion Of Maximum Ascents
*/
 if( true) goto traceDownstream ;   // Following Code Has still To be perfected
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Catchment IDs For Mone Subdivided Triangles") ;
 for( pnt1 = 0 ; pnt1 < catchmentDtmP->numPoints ; ++pnt1 )
   {
    if( ( clPtr = nodeAddrP(catchmentDtmP,pnt1)->cPtr ) != catchmentDtmP->nullPtr )
      {
       pnt1P = pointAddrP(catchmentDtmP,pnt1) ;
       bcdtmFind_closestPointDtmObject(dtmP,pnt1P->x,pnt1P->y,&trgPnt1) ;
       pnt3 = clistAddrP(catchmentDtmP,clPtr)->pntNum ;
       if( ( pnt2 = bcdtmList_nextAntDtmObject(catchmentDtmP,pnt1,pnt3)) < 0 ) goto errexit ;
       pnt2P = pointAddrP(catchmentDtmP,pnt2) ;
       bcdtmFind_closestPointDtmObject(dtmP,pnt2P->x,pnt2P->y,&trgPnt2) ;
       while( clPtr != catchmentDtmP->nullPtr )
         {
          pnt3  = clistAddrP(catchmentDtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(catchmentDtmP,clPtr)->nextPtr ;
          pnt3P = pointAddrP(catchmentDtmP,pnt3) ;
          bcdtmFind_closestPointDtmObject(dtmP,pnt3P->x,pnt3P->y,&trgPnt3) ;
          if( pnt2 > pnt1 && pnt3 > pnt1 && nodeAddrP(catchmentDtmP,pnt3)->hPtr != pnt1 )
            {
             if( bcdtmList_testLineDtmObject(dtmP,trgPnt1,trgPnt2))
               {
                if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ;
                if( clkPnt == trgPnt3 )
                  {
                   if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset,pnt1,pnt2) ) goto errexit ;
                   *(tinLineP+lineOffset) = catchmentID ;
                   if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset,pnt2,pnt3) ) goto errexit ;
                   *(tinLineP+lineOffset) = catchmentID ;
                   if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset,pnt3,pnt1) ) goto errexit ;
                   *(tinLineP+lineOffset) = catchmentID ;
                  }
               }
            }
          pnt2 = pnt3 ;
          pnt2P = pnt3P ;
          trgPnt2 = trgPnt3 ;
         }
      }
   }
/*
**  Downstream Trace From Each Triangle Centroid Of The Unrefined Catchment DTM
*/
 traceDownstream :
 if( dbg ) bcdtmWrite_message(10,0,0,"Tracing Tin Streams") ;
 for( pnt1 = 0 ; pnt1 < catchmentDtmP->numPoints ; ++pnt1 )
   {
    if( ( clPtr = nodeAddrP(catchmentDtmP,pnt1)->cPtr ) != catchmentDtmP->nullPtr )
      {
       pnt1P = pointAddrP(catchmentDtmP,pnt1) ;
       pnt3 = clistAddrP(catchmentDtmP,clPtr)->pntNum ;
       if( ( pnt2 = bcdtmList_nextAntDtmObject(catchmentDtmP,pnt1,pnt3)) < 0 ) goto errexit ;
       pnt2P = pointAddrP(catchmentDtmP,pnt2) ;
       while( clPtr != catchmentDtmP->nullPtr )
         {
          pnt3  = clistAddrP(catchmentDtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(catchmentDtmP,clPtr)->nextPtr ;
          pnt3P = pointAddrP(catchmentDtmP,pnt3) ;
          if( pnt2 > pnt1 && pnt3 > pnt1 && nodeAddrP(catchmentDtmP,pnt3)->hPtr != pnt1 )
            {
             if( trgNum % 10000 == 0 ) bcdtmWrite_message(0,0,0,"Processing Triangle %8ld of %8ld",trgNum,catchmentDtmP->numTriangles) ;
             ++trgNum ;
             traceToLowPoint = TRUE ;
             if( true) goto trace ;    // Following Code Still Needs To Be perfected

/*
**           Check If Catchment Has Been Set For Triangle
*/
             traceToLowPoint = FALSE ;
             if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset1,pnt1,pnt2) ) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset2,pnt2,pnt3) ) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset3,pnt3,pnt1) ) goto errexit ;
             if( *(tinLineP+lineOffset1) != catchmentID || *(tinLineP+lineOffset2) != catchmentID || *(tinLineP+lineOffset3) != catchmentID ) traceToLowPoint = TRUE ;
/*
**           Trace To LowPoint
*/
             trace :
             if( traceToLowPoint == TRUE )
               {
                startX = ( pnt1P->x + pnt2P->x + pnt3P->x ) / 3.0 ;
                startY = ( pnt1P->y + pnt2P->y + pnt3P->y ) / 3.0 ;
                startZ = ( pnt1P->z + pnt2P->z + pnt3P->z ) / 3.0 ;
                if( bcdtmFind_triangleForPointDtmObject(dtmP,startX,startY,&startZ,&fndPnt,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                if( fndPnt )
                  {
                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Tracing To Low Point From %12.5lf %12.5lf %10.4lf",startX,startY,startZ) ;
//                   if( bcdtmDrainage_traceToLowPointDtmObject(dtmP,NULL,falseLowDepth,traceOverZeroSlope,useTables,FALSE,trgPnt1,trgPnt3,trgPnt2,startX,startY,startZ,userP,&lowPnt1,&lowPnt2) ) goto errexit ;
                   if( dbg == 2 )
                     {
                      bcdtmWrite_message(0,0,0,"lowPnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",lowPnt1,pointAddrP(dtmP,lowPnt1)->x,pointAddrP(dtmP,lowPnt1)->y,pointAddrP(dtmP,lowPnt1)->z) ;
                      if( lowPnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"lowPnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",lowPnt2,pointAddrP(dtmP,lowPnt2)->x,pointAddrP(dtmP,lowPnt2)->y,pointAddrP(dtmP,lowPnt2)->z) ;
                     }
/*
**                 Trace Terminates At A Sump Point
*/
                   flowPoint = dtmP->nullPnt ;
                   if( lowPnt2 == dtmP->nullPnt )
                     {
                      if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::ZeroSlopePolygon,lowPnt1,&dtmFeature))
                        {
                         flowPoint = -dtmFeature ;
                        }
                      else flowPoint = lowPnt1 ;
                     }
/*
**                 Trace Terminates At A Sump Line
*/
                   if( lowPnt2 != dtmP->nullPnt )
                     {
                      if( lowPnt1 > lowPnt2 ) { pnt = lowPnt1 ; lowPnt1 = lowPnt2 ; lowPnt2 = pnt ; }
                      if( bcdtmList_testForLineOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::ZeroSlopePolygon,lowPnt1,lowPnt2))
                        {
                         bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::ZeroSlopePolygon,lowPnt1,&dtmFeature) ;
                         flowPoint = -dtmFeature ;
                        }
                      else
                        {
                         if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&lineOffset,lowPnt1,lowPnt2) ) goto errexit ;
                         flowPoint = dtmP->numPoints + lineOffset ;
                        }
                     }
/*
**                 Mark Tin Lines
*/
                   if( flowPoint == catchmentID )
                     {
                      if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset,pnt1,pnt2) ) goto errexit ;
                      *(tinLineP+lineOffset) = flowPoint ;
                      if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset,pnt2,pnt3) ) goto errexit ;
                      *(tinLineP+lineOffset) = flowPoint ;
                      if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset,pnt3,pnt1) ) goto errexit ;
                      *(tinLineP+lineOffset) = flowPoint ;
                     }
                  }
               }
            }
          pnt2  = pnt3  ;
          pnt2P = pnt3P ;
         }
      }
   }
 bcdtmWrite_message(10,0,0,"Triangles Processed %8ld of %8ld",catchmentDtmP->numTriangles,catchmentDtmP->numTriangles) ;
 bcdtmWrite_message(10,0,0,"") ;
/*
** Extract Polygons On the Tin Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Polygons On Tin Hull") ;
 pnt1 = catchmentDtmP->hullPoint ;
 do
   {
    pnt2 = nodeAddrP(catchmentDtmP,pnt1)->hPtr ;
    bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset1,pnt2,pnt1) ;
    if( ( lineValue = *(tinLineP+lineOffset1) ) != nullValue )
      {
       pnt3 = pnt2 ;
       pnt2 = pnt1 ;
       lineOffset = lineOffset1 ;
       if( bcdtmLoad_storeFeaturePoint(pointAddrP(catchmentDtmP,pnt2)->x,pointAddrP(catchmentDtmP,pnt2)->y,pointAddrP(catchmentDtmP,pnt2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
/*
**     Scan Back To First Point
*/
       do
         {
          if( (pnt3 = bcdtmList_nextAntDtmObject(catchmentDtmP,pnt2,pnt3)) < 0 ) goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset1,pnt2,pnt3)) goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset2,pnt3,pnt2)) goto errexit ;
          while( *(tinLineP+lineOffset1) == lineValue && *(tinLineP+lineOffset2) == lineValue )
            {
             if( (pnt3 = bcdtmList_nextAntDtmObject(catchmentDtmP,pnt2,pnt3)) < 0 ) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset1,pnt2,pnt3)) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset2,pnt3,pnt2)) goto errexit ;
            }
          *(tinLineP+lineOffset1) = nullValue ;
          pnt4 = pnt2 ;
          pnt2 = pnt3 ;
          pnt3 = pnt4 ;
          if( bcdtmLoad_storeFeaturePoint(pointAddrP(catchmentDtmP,pnt2)->x,pointAddrP(catchmentDtmP,pnt2)->y,pointAddrP(catchmentDtmP,pnt2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
         } while ( pnt2 != pnt1 ) ;
       *(tinLineP+lineOffset) = nullValue ;
/*
**     Remove Duplicate Feature Points
*/
       numBefore = numFeaturePts ;
       p3d1P = featurePtsP ;
       for( p3d2P = featurePtsP + 1 ; p3d2P < featurePtsP + numFeaturePts ; ++p3d2P )
         {
          if( p3d2P->x != p3d1P->x || p3d2P->y != p3d1P->y )
            {
             ++p3d1P ;
             if( p3d1P != p3d2P ) *p3d1P = *p3d2P ;
            }
         }
       numFeaturePts = (long)(p3d1P-featurePtsP) + 1 ;
       numAfter = numFeaturePts ;
       if( numBefore != numAfter )
         {
          bcdtmWrite_message(1,0,0,"Points Filtered From Catchment Polygon ** numBefore = %8ld numAfter = %8ld",numBefore,numAfter) ;
          goto errexit ;
         }
/*
**     Clean Catchment Polygon
*/
//       if( bcdtmDrainage_cleanCatchmentPolygonDtmObject(catchmentDtmP,&featurePtsP,&numFeaturePts)) goto errexit ;
       memFeaturePts = numFeaturePts ;
/*
**     Load Catchment
*/
       if( loadFunctionP(DTMFeatureType::Catchment,(DTMUserTag)numPolygons%8,(DTMFeatureId)nullFeatureId,(DPoint3d *)featurePtsP,(long)numFeaturePts,(void *)userP)) goto errexit ;
       numFeaturePts = 0 ;
       ++numPolygons ;
      }
    pnt1 = nodeAddrP(catchmentDtmP,pnt1)->hPtr  ;
   } while( pnt1 != catchmentDtmP->hullPoint ) ;
/*
** Extract Internal Polygons
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Polygons Internal To Tin Hull") ;
 for( pnt1 = 0 ; pnt1 < catchmentDtmP->numPoints ; ++pnt1 )
   {
    clPtr = nodeAddrP(catchmentDtmP,pnt1)->cPtr;
    while ( clPtr != catchmentDtmP->nullPtr )
      {
       pnt2 = clistAddrP(catchmentDtmP,clPtr)->pntNum ;

       if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset1,pnt1,pnt2) )  goto errexit ;
       if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset2,pnt2,pnt1) )  goto errexit ;
       if( *(tinLineP+lineOffset1) != *(tinLineP+lineOffset2)  && *(tinLineP+lineOffset1) != nullValue  && bcdtmList_testLineDtmObject(catchmentDtmP,pnt1,clistAddrP(catchmentDtmP,clPtr)->pntNum) && ( nodeAddrP(catchmentDtmP,pnt1)->hPtr != pnt2 || nodeAddrP(catchmentDtmP,pnt2)->hPtr != pnt1 ) )
         {
          lineValue = *(tinLineP+lineOffset1) ;
          pnt3 = pnt1 ;
/*
**        Get Polygon Direction
*/
          area = 0.0 ;
          sx = pointAddrP(catchmentDtmP,pnt1)->x ;
          sy = pointAddrP(catchmentDtmP,pnt1)->y ;
          x = pointAddrP(catchmentDtmP,pnt2)->x - sx ;
          y = pointAddrP(catchmentDtmP,pnt2)->y - sy  ;
          area = area + ( x * y ) / 2.0 + x * sy ;
          sx = pointAddrP(catchmentDtmP,pnt2)->x ;
          sy = pointAddrP(catchmentDtmP,pnt2)->y ;
          do
            {
             if( ( pnt3 = bcdtmList_nextAntDtmObject(catchmentDtmP,pnt2,pnt3)) < 0 ) goto errexit  ;
             if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset1,pnt2,pnt3)) goto errexit  ;
             if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset2,pnt3,pnt2)) goto errexit  ;
             while( *(tinLineP+lineOffset1) == lineValue && *(tinLineP+lineOffset2) == lineValue )
               {
                if( ( pnt3 = bcdtmList_nextAntDtmObject(catchmentDtmP,pnt2,pnt3)) < 0 ) goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset1,pnt2,pnt3)) goto errexit  ;
                if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset2,pnt3,pnt2)) goto errexit  ;
               }
             x = pointAddrP(catchmentDtmP,pnt3)->x - sx ; y = pointAddrP(catchmentDtmP,pnt3)->y - sy  ;
             area = area + ( x * y ) / 2.0 + x * sy ;
             sx = pointAddrP(catchmentDtmP,pnt3)->x ;
             sy = pointAddrP(catchmentDtmP,pnt3)->y ;
              pnt4 = pnt2 ; pnt2 = pnt3 ; pnt3 = pnt4 ;
             } while ( pnt2 != pnt1 ) ;
/*
**        If Polygon Is Clockwise Write Polygon If It Is Not A Void
*/
          if( area > 0.0 && lineValue != -10 )
            {
             pnt3 = pnt1 ;
             pnt2 = clistAddrP(catchmentDtmP,clPtr)->pntNum ;
             if( bcdtmLoad_storeFeaturePoint(pointAddrP(catchmentDtmP,pnt1)->x,pointAddrP(catchmentDtmP,pnt1)->y,pointAddrP(catchmentDtmP,pnt1)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
/*
**           Scan Back To First Point
*/
             do
               {
                if( (pnt3 = bcdtmList_nextAntDtmObject(catchmentDtmP,pnt2,pnt3)) < 0 ) goto errexit  ;
                if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset1,pnt2,pnt3)) goto errexit  ;
                if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset2,pnt3,pnt2)) goto errexit  ;
                while( *(tinLineP+lineOffset1) == lineValue && *(tinLineP+lineOffset2) == lineValue )
                  {
                   if( (pnt3 = bcdtmList_nextAntDtmObject(catchmentDtmP,pnt2,pnt3)) < 0 ) goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset1,pnt2,pnt3)) goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(catchmentDtmP,&lineOffset2,pnt3,pnt2)) goto errexit ;
                  }
                *(tinLineP+lineOffset1)  = nullValue ;
                if( bcdtmLoad_storeFeaturePoint(pointAddrP(catchmentDtmP,pnt2)->x,pointAddrP(catchmentDtmP,pnt2)->y,pointAddrP(catchmentDtmP,pnt2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
                pnt4 = pnt2 ;
                pnt2 = pnt3 ;
                pnt3 = pnt4 ;
               } while ( pnt2 != pnt1 ) ;
             if( bcdtmLoad_storeFeaturePoint(pointAddrP(catchmentDtmP,pnt1)->x,pointAddrP(catchmentDtmP,pnt1)->y,pointAddrP(catchmentDtmP,pnt1)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
/*
**           Remove Duplicate Feature Points
*/
             numBefore = numFeaturePts ;
             p3d1P = featurePtsP ;
             for( p3d2P = featurePtsP + 1 ; p3d2P < featurePtsP + numFeaturePts ; ++p3d2P )
               {
                if( p3d2P->x != p3d1P->x || p3d2P->y != p3d1P->y )
                  {
                   ++p3d1P ;
                   if( p3d1P != p3d2P ) *p3d1P = *p3d2P ;
                  }
               }
             numFeaturePts = (long)(p3d1P-featurePtsP) + 1 ;
/*
**           Check For Filtered Points
*/
             numAfter = numFeaturePts ;
             if( numBefore != numAfter )
               {
                bcdtmWrite_message(1,0,0,"Points Filtered From Catchment Polygon ** numBefore = %8ld numAfter = %8ld",numBefore,numAfter) ;
                goto errexit ;
               }
/*
**           Clean Catchment Polygon
*/
 //            if( bcdtmDrainage_cleanCatchmentPolygonDtmObject(catchmentDtmP,&featurePtsP,&numFeaturePts)) goto errexit ;
             memFeaturePts = numFeaturePts ;
/*
**           Load Catchment
*/
             if( loadFunctionP(DTMFeatureType::Catchment,(DTMUserTag)numPolygons%8,(DTMFeatureId)nullFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
             numFeaturePts = 0 ;
             ++numPolygons ;
            }
         }
       clPtr = clistAddrP(catchmentDtmP,clPtr)->nextPtr ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( tinLineP    != NULL ) { free(tinLineP)    ; tinLineP    = NULL ; }
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(10,0,0,"Determining Catchments Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(10,0,0,"Determining Catchments Error") ;
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
int bcdtmDrainage_expandTptrPolygonDtmObject
(
 BC_DTM_OBJ  *dtmP,                /* ==>  Pointer To DTM Object                  */
 long        *startPointP          /* <=>  Pointer To Start Point Of Tptr Polygon */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long pnt1,pnt2,pnt3,pnt4,process ;
 DPoint3d  polygonLine[2] ;
 double dd ;
 DTM_TIN_POINT *pntP ;
 BC_DTM_OBJ *polygonDtmP=NULL ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Expanding Tptr Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPointP = %8ld",*startPointP) ;
   }
/*
** Create DTM To Store Polygon
*/
 if( bcdtmObject_createDtmObject(&polygonDtmP)) goto errexit ;
/*
** Store Tptr Polygon In Polygon DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Tptr Polygon In DTM") ;
 pnt1 = *startPointP ;
 do
   {
    pnt2 = nodeAddrP(dtmP,pnt1)->tPtr ;
    pntP = pointAddrP(dtmP,pnt1) ;
    polygonLine[0].x = pntP->x ;
    polygonLine[0].y = pntP->y ;
    polygonLine[0].z = pntP->z ;
    pntP = pointAddrP(dtmP,pnt2) ;
    polygonLine[1].x = pntP->x ;
    polygonLine[1].y = pntP->y ;
    polygonLine[1].z = pntP->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(polygonDtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,polygonLine,2)) goto errexit ;
    pnt1 = pnt2 ;
   } while( pnt1 != *startPointP ) ;
/*
** Store Triangles External To Tptr Polygon In Polygon DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Triangles External To Tptr Polygon In DTM") ;
 pnt1 = *startPointP ;
 do
   {
    pnt2 = pnt3 = nodeAddrP(dtmP,pnt1)->tPtr ;
    if( nodeAddrP(dtmP,pnt1)->hPtr != pnt2 && nodeAddrP(dtmP,pnt2)->hPtr != pnt1 )
      {
       process = 1 ;
       if(( pnt4 = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt3)) < 0 ) goto errexit ;
       while( nodeAddrP(dtmP,pnt3)->tPtr != pnt1 && process )
         {
          pntP = pointAddrP(dtmP,pnt3) ;
          polygonLine[0].x = pntP->x ;
          polygonLine[0].y = pntP->y ;
          polygonLine[0].z = pntP->z ;
          pntP = pointAddrP(dtmP,pnt4) ;
          polygonLine[1].x = pntP->x ;
          polygonLine[1].y = pntP->y ;
          polygonLine[1].z = pntP->z ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(polygonDtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,polygonLine,2)) goto errexit ;
          if( nodeAddrP(dtmP,pnt1)->hPtr == pnt4 )
            {
             process = 0 ;
             pntP = pointAddrP(dtmP,pnt1) ;
             polygonLine[0].x = pntP->x ;
             polygonLine[0].y = pntP->y ;
             polygonLine[0].z = pntP->z ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(polygonDtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,polygonLine,2)) goto errexit ;
            }
          pnt3 = pnt4 ;
          if(( pnt4 = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt3)) < 0 ) goto errexit ;
         }
      }
    pnt1 = pnt2 ;
   } while( pnt1 != *startPointP ) ;
/*
** Triangulate Polygon DTM Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating ** polygonDtmP->numPoints = %8ld",polygonDtmP->numPoints) ;
 polygonDtmP->ppTol = polygonDtmP->plTol = 0.0 ;
 if( bcdtmObject_triangulateDtmObject(polygonDtmP)) goto errexit ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(polygonDtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"After Triangulating ** polygonDtmP->numPoints = %8ld",polygonDtmP->numPoints) ;
 if( dbg ) bcdtmWrite_toFileDtmObject(polygonDtmP,L"expandedTptrPolygon.bcdtm") ;
/*
** Scan Polygon DTM And Determine Point Numbers From DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determining Point Numbers") ;
 pnt1 = polygonDtmP->hullPoint ;
 do
   {
    pntP = pointAddrP(polygonDtmP,pnt1) ;
    bcdtmFind_closestPointDtmObject(dtmP,pntP->x,pntP->y,&pnt2);
    if( cdbg )
      {
       dd = bcdtmMath_distance(pntP->x,pntP->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
       if( dd != 0.0 )
         {
          bcdtmWrite_message(0,0,0,"Find Error   ** dd = %12.5lf",dd) ;
          bcdtmWrite_message(0,0,0,"Actual Point ** %12.5lf %12.5lf %10.5lf",pntP->x,pntP->y,pntP->z) ;
          bcdtmWrite_message(0,0,0,"Find   Point ** %12.5lf %12.5lf %10.5lf",pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z) ;
         }
      }
    pntP->z = ( double ) pnt2 ;
    pnt1 = nodeAddrP(polygonDtmP,pnt1)->hPtr ;
   } while ( pnt1 != polygonDtmP->hullPoint ) ;
/*
** Check Connectivity Of Expanded Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity") ;
 pnt1 = polygonDtmP->hullPoint ;
 pnt3 = (long )pointAddrP(polygonDtmP,pnt1)->z ;
 do
   {
    pnt2 = nodeAddrP(polygonDtmP,pnt1)->hPtr ;
    pnt4 = (long )pointAddrP(polygonDtmP,pnt2)->z ;
    if( ! bcdtmList_testLineDtmObject(dtmP,pnt3,pnt4))
      {
       bcdtmWrite_message(2,0,0,"Tptr Polygon Connectivity Error") ;
       goto errexit ;
      }
    pnt1 = pnt2 ;
    pnt3 = pnt4 ;
   } while ( pnt1 != polygonDtmP->hullPoint ) ;
/*
** Null Current Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Current Tptr Polygon") ;
 if( bcdtmList_nullTptrListDtmObject(dtmP,*startPointP)) goto errexit ;
/*
** Insert Expanded Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Expanded Tptr Polygon") ;
 pnt1 = polygonDtmP->hullPoint ;
 pnt3 = (long )pointAddrP(polygonDtmP,pnt1)->z ;
 *startPointP = pnt3 ;
 do
   {
    pnt2 = nodeAddrP(polygonDtmP,pnt1)->hPtr ;
    pnt4 = (long )pointAddrP(polygonDtmP,pnt2)->z ;
    nodeAddrP(dtmP,pnt3)->tPtr = pnt4 ;
    pnt1 = pnt2 ;
    pnt3 = pnt4 ;
   } while ( pnt1 != polygonDtmP->hullPoint ) ;
/*
** Clean Up
*/
 cleanup :
 if( polygonDtmP != NULL ) bcdtmObject_destroyDtmObject(&polygonDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Tptr Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Expanding Tptr Polygon Error") ;
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
int bcdtmDrainage_insertCatchmentPolygonIntoDtmObject
(
 BC_DTM_OBJ   *dtmP,                    /* ==> Pointer To DTM Object               */
 DPoint3d     *polygonPtsP,             /* ==> Pointer To Catchment  Polygon       */
 long         numPolygonPts,            /* ==> Number Of Polygon Points            */
 long         *startPointP              /* <== Start Point For Tptr Polygon        */
)
{
/*
** Insert A Catchment Polygon Into The DTM
** Assumes The Catchment Polygon Is Clean
*/
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   pnt1,pnt2,startPnt,fixFlag,fixPnt,nxtPnt ;
 DPoint3d    *p3dP ;
 double dd ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Catchment Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"polygonPtsPP       = %p",polygonPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolygonPtsP     = %8ld",numPolygonPts) ;
    bcdtmWrite_message(0,0,0,"startPointP        = %8ld",*startPointP) ;
   }
/*
** Initialise
*/
 *startPointP = dtmP->nullPnt ;
/*
** Insert First Point
*/
 bcdtmFind_closestPointDtmObject(dtmP,polygonPtsP->x,polygonPtsP->y,&startPnt);
 if( cdbg )
   {
    dd = bcdtmMath_distance(polygonPtsP->x,polygonPtsP->y,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y) ;
    if( dd != 0.0 )
      {
       bcdtmWrite_message(1,0,0,"Closest Point Find Error") ;
       goto errexit ;
      }
   }
/*
**  Creat Tptr Polygon Of Catchment Polygon
*/
 fixFlag = 0 ;
 pnt1 = startPnt ;
 for( p3dP = polygonPtsP ; p3dP < polygonPtsP + numPolygonPts - 1 ; ++p3dP )
   {
/*
**  Get Next DTM Point For Polygon
*/
    bcdtmFind_closestPointDtmObject(dtmP,(p3dP+1)->x,(p3dP+1)->y,&pnt2);
    if( cdbg )
      {
       dd = bcdtmMath_distance((p3dP+1)->x,(p3dP+1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
       if( dd != 0.0 )
         {
          bcdtmWrite_message(1,0,0,"Closest Point Find Error") ;
          goto errexit ;
         }
      }
/*
**  Check Next Point Is Not Already Included In Tptr Catchment Polygon
*/
    if( nodeAddrP(dtmP,pnt2)->tPtr != dtmP->nullPnt && pnt2 != startPnt  )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Fixing Connectivity Error At Point %8ld",pnt2) ;
       fixPnt = pnt2 ;
       do
         {
          nxtPnt = nodeAddrP(dtmP,fixPnt)->tPtr ;
          nodeAddrP(dtmP,fixPnt)->tPtr = dtmP->nullPnt ;
          fixPnt = nxtPnt ;
         } while( nodeAddrP(dtmP,fixPnt)->tPtr != dtmP->nullPnt ) ;
       fixFlag = 1 ;
      }
    else nodeAddrP(dtmP,pnt1)->tPtr = pnt2 ;
/*
**  Reset For Next Segment
*/
    pnt1 = pnt2 ;
   }
/*
** Check Connectivity Tptr Polygon
*/
 if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,startPnt,0))
   {
    bcdtmWrite_message(0,0,0,"Connectivity Error In Tptr Polygon") ;
    goto errexit ;
   }
/*
** Set Start Point
*/
 *startPointP = startPnt ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Catchment Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Catchment Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_insertMaximumAscentLinesFromCatchmentDtmObject
(
 BC_DTM_OBJ   *dtmP,                    // ==> Pointer To DTM Object
 long         startPoint                // ==> Start Point For Tptr Catchment Polygon
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   pnt1,pnt2,antPnt,clkPnt  ;
 double trgDescentAngle,trgAscentAngle,trgSlope ;

// Write Entry Message

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Catchment Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint = %8ld",startPoint) ;
   }

// Report Number Of Points In Catchment DTM After Inserting Maximum Ascent Lines

 if( dbg ) bcdtmWrite_message(0,0,0,"Before Inserting Maximum Ascent Lines ** dtmP->numPoints = %8ld",dtmP->numPoints) ;

// Scan Catchment Polygon And Fire Off Maximum Ascent Lines

 pnt1 = startPoint ;
 do
   {
    pnt2 = nodeAddrP(dtmP,pnt1)->tPtr ;
    if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
    if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
    if( ! bcdtmList_testLineDtmObject(dtmP,pnt2,antPnt)) antPnt = dtmP->nullPnt ;
    if( ! bcdtmList_testLineDtmObject(dtmP,pnt2,clkPnt)) clkPnt = dtmP->nullPnt ;
/*
**  Fire Off Maximum Ascent On Counter Clockwise Triangle
*/
    if( antPnt != dtmP->nullPnt )
      {
       if( bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,antPnt,pnt2,&trgDescentAngle,&trgAscentAngle,&trgSlope)) goto errexit ;
       if( bcdtmDrainage_checkForAngleBetweenTrianglePointsDtmObject(dtmP,pnt1,antPnt,pnt2,trgAscentAngle))
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Firing Off Maximum Ascent Fron Pnt1 - ant ** ascentAngle = %12.10lf",trgAscentAngle) ;
          if( bcdtmDrainage_insertMaximumAscentLineBetweenPointsDtmObject(dtmP,pnt1,antPnt,pnt2)) goto errexit ;
         }
       if( bcdtmDrainage_checkForAngleBetweenTrianglePointsDtmObject(dtmP,pnt2,pnt1,antPnt,trgAscentAngle))
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Firing Off Maximum Ascent Fron Pnt1 - ant ** ascentAngle = %12.10lf",trgAscentAngle) ;
          if( bcdtmDrainage_insertMaximumAscentLineBetweenPointsDtmObject(dtmP,pnt2,pnt1,antPnt)) goto errexit ;
         }
      }
/*
**  Fire Off Maximum Ascent On Clockwise Triangle
*/
    if( clkPnt != dtmP->nullPnt )
      {
       if( bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,clkPnt,&trgDescentAngle,&trgAscentAngle,&trgSlope)) goto errexit ;
       if( bcdtmDrainage_checkForAngleBetweenTrianglePointsDtmObject(dtmP,pnt1,pnt2,clkPnt,trgAscentAngle))
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Firing Off Maximum Ascent Fron Pnt1 - clk ** ascentAngle = %12.10lf",trgAscentAngle) ;
          if( bcdtmDrainage_insertMaximumAscentLineBetweenPointsDtmObject(dtmP,pnt1,pnt2,clkPnt)) goto errexit ;
         }
       if( bcdtmDrainage_checkForAngleBetweenTrianglePointsDtmObject(dtmP,pnt2,clkPnt,pnt1,trgAscentAngle))
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Firing Off Maximum Ascent Fron Pnt1 - clk ** ascentAngle = %12.10lf",trgAscentAngle) ;
          if( bcdtmDrainage_insertMaximumAscentLineBetweenPointsDtmObject(dtmP,pnt2,clkPnt,pnt1)) goto errexit ;
         }
      }
    pnt1 = pnt2 ;
   } while( pnt1 != startPoint) ;

/*
** Report Number Of Points In Catchment DTM After Inserting Maximum Ascent Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"After  Inserting Maximum Ascent Lines ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Clean And Check Triangulation
*/
 if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
/*
** Clean And Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation After Inserting Maximum Ascent Lines") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Triangulation Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Catchment Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Catchment Polygon Error") ;
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
int bcdtmDrainage_checkForAngleBetweenTrianglePointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 long p1,
 long p2,
 long p3,
 double angle
)
{
 int dbg=DTM_TRACE_VALUE(0) ;
/*
** Triangle Points P1 P2 P3 Must Be In A Clockwise Direction
*/
 double p1p2Angle,p1p3Angle,angleIncrement=0.0 ;
 int sd1,sd2 ;
 double xr,yr,radius ;
/*
** Get Angles
*/
 if( true ) goto radiusTest ;
 p1p2Angle = bcdtmMath_getPointAngleDtmObject(dtmP,p1,p2) ;
 p1p3Angle = bcdtmMath_getPointAngleDtmObject(dtmP,p1,p3) ;
 if( p1p2Angle <= p1p3Angle ) angleIncrement = DTM_2PYE ;
 p1p2Angle += angleIncrement ;
 angle     += angleIncrement ;
 if( dbg ) bcdtmWrite_message(0,0,0,"p1p3Angle = %12.10lf angle = %12.10lf p1p2Angle = %12.10lf",p1p3Angle,angle,p1p2Angle) ;
 if( angle >= p1p3Angle && angle <= p1p2Angle ) return(1) ;
 return(0) ;
/*
** Radius
*/
 radiusTest :
 radius = sqrt(dtmP->xRange*dtmP->xRange+dtmP->yRange*dtmP->yRange) ;
 xr = pointAddrP(dtmP,p1)->x + radius*cos(angle) ;
 yr = pointAddrP(dtmP,p1)->y + radius*sin(angle) ;
 sd1 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,xr,yr,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
 sd2 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,xr,yr,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y) ;
 if( sd1 > 0 && sd2 < 0 ) return(1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_insertMaximumAscentLineBetweenPointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       point,
 long       startPoint,
 long       endPoint
)
/*
** This Function Inserts A Maximum Line At Starting At Point.
** The Maximum Ascent Is determined By Scanning Clockwise From Start Point To End Point
** If startPoint is set to dtmP->nullPnt The Maximum Ascent Is Determined By Scanning All Triangles
** Connected To startPoint
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,p4,np,np1,np2,np3,process,insertLine,ridgePoint  ;
 double d1,d2,dx,dy,dz,ascentAngle=0.0,slope,radius  ;
 double sx,sy,sz,xr,yr,nx,ny,nz,fx,fy,lastZ ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line Between Points") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"point           = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"startPoint      = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"endPoint        = %8ld",endPoint) ;
   }
/*
** Check points are Connected
*/
 if( startPoint != dtmP->nullPnt )
   {
    if( ! bcdtmList_testLineDtmObject(dtmP,point,startPoint)) goto cleanup ;
    if( ! bcdtmList_testLineDtmObject(dtmP,point,endPoint))   goto cleanup ;
   }
/*
** Initialise
*/
 p1 = p2 = p3 = dtmP->nullPnt ;
 fx = sx = pointAddrP(dtmP,point)->x ;
 fy = sy = pointAddrP(dtmP,point)->y ;
/*
**  Check For Ridge Lines
*/
 if( bcdtmDrainage_scanBetweenPointsForRidgeLineDtmObject(dtmP,point,startPoint,endPoint,&ridgePoint) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"ridgePoint = %10ld",ridgePoint) ;
/*
**  If No Ridge Lines Get Maximum Ascent Triangle
*/
 if( ridgePoint == dtmP->nullPnt )
   {
    if( bcdtmDrainage_scanBetweenPointsForMaximumAscentTriangleDtmObject(dtmP,0,point,startPoint,endPoint,&p3,&p2,&ascentAngle,&slope)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"p2 = %10ld p3 = %10ld ** ascentAngle = %12.10lf slope = %10.5lf",p2,p3,ascentAngle,slope) ;
    if( p2 == dtmP->nullPnt ) goto cleanup ;
   }
/*
** If Maximum Ascent Triangle Insert Maximum Ascent Line
*/
 if( p2 != dtmP->nullPnt )
   {
/*
** Set Starting Coordinates For Trace
*/
    fx = sx = pointAddrP(dtmP,point)->x ;
    fy = sy = pointAddrP(dtmP,point)->y ;
    sz = pointAddrP(dtmP,point)->z ;
    dx = dtmP->xMax - dtmP->xMin ;
    dy = dtmP->yMax - dtmP->yMin ;
    radius = sqrt(dx*dx + dy*dy) ;
    xr = sx + radius * cos(ascentAngle) ;
    yr = sy + radius * sin(ascentAngle) ;
/*
**  Get Intersection With p2-p3
*/
    bcdtmDrainage_intersectCordLines(sx,sy,xr,yr,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,&nx,&ny) ;
    if     ( bcdtmMath_distance(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,nx,ny) <= dtmP->ppTol ) { p1 = p2 ; p2 = p3 = dtmP->nullPnt ; }
    else if( bcdtmMath_distance(pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,nx,ny) <= dtmP->ppTol ) { p1 = p3 ; p2 = p3 = dtmP->nullPnt ; }
    if( p2 != dtmP->nullPnt )
      {
       d1  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,nx,ny) ;
       d2  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,nx,ny) ;
       if      ( d1 <= d2 && d1 <= dtmP->ppTol ) { p1 = p3 ; p2 = p3 = dtmP->nullPnt ; }
       else if ( d2 <= d1 && d2 <= dtmP->ppTol ) { p1 = p2 ; p2 = p3 = dtmP->nullPnt ; }
      }

    if( p2 != dtmP->nullPnt )
      {
       if(( p4 = bcdtmList_nextClkDtmObject(dtmP,p2,p3)) < 0 ) goto errexit ;
       if( ! bcdtmList_testLineDtmObject(dtmP,p4,p3) || p4 == point ) p4 = dtmP->nullPnt  ;
       if( p4 != dtmP->nullPnt )
         {
          d1  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,nx,ny) ;
          d2  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p4)->x,pointAddrP(dtmP,p4)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,nx,ny) ;
          if      ( d1 <= d2 && d1 <= dtmP->ppTol ) { p1 = p3 ; p2 = p3 = dtmP->nullPnt ; }
          else if ( d2 <= d1 && d2 <= dtmP->ppTol ) { p1 = p2 ; p2 = p3 = dtmP->nullPnt ; }
         }
      }

/*
** If Maximum Ascent Cuts p3-p2 Calculate Intercept And Store point
*/
    if( p2 != dtmP->nullPnt )
      {
       dx = pointAddrP(dtmP,p3)->x - pointAddrP(dtmP,p2)->x ;
       dy = pointAddrP(dtmP,p3)->y - pointAddrP(dtmP,p2)->y ;
       dz = pointAddrP(dtmP,p3)->z - pointAddrP(dtmP,p2)->z ;
       if( fabs(dx) >= fabs(dy)) nz = pointAddrP(dtmP,p2)->z +  dz * (nx - pointAddrP(dtmP,p2)->x) / dx ;
       else                      nz = pointAddrP(dtmP,p2)->z +  dz * (ny - pointAddrP(dtmP,p2)->y) / dy ;
/*
**  Get Next Clock
*/
       p1 = point ;
       if(( p4 = bcdtmList_nextClkDtmObject(dtmP,p2,p3)) < 0 ) goto errexit ;
       if( ! bcdtmList_testLineDtmObject(dtmP,p4,p3) || p4 == p1 ) p4 = dtmP->nullPnt  ;
/*
**  Insert Line
*/
       insertLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p2,p3) ;
       if( bcdtmInsert_addPointToDtmObject(dtmP,nx,ny,nz,&np) ) goto errexit ;
       if( bcdtmList_deleteLineDtmObject(dtmP,p2,p3)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,np,p3)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,p1,dtmP->nullPnt)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,np,p1)) goto errexit ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,np,p2,p1)) goto errexit ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p3,np,p1)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,p3,p1)) goto errexit ;
       if( p4 != dtmP->nullPnt )
         {
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,np,p2)) goto errexit ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,p4,p3)) goto errexit ;
         }
       else
         {
          if(nodeAddrP(dtmP,p2)->hPtr == p3 ) { nodeAddrP(dtmP,p2)->hPtr = np ; nodeAddrP(dtmP,np)->hPtr = p3 ; }
          if(nodeAddrP(dtmP,p3)->hPtr == p2 ) { nodeAddrP(dtmP,p3)->hPtr = np ; nodeAddrP(dtmP,np)->hPtr = p2 ; }
         }
       if(nodeAddrP(dtmP,p2)->tPtr == p3 ) { nodeAddrP(dtmP,p2)->tPtr = np ; nodeAddrP(dtmP,np)->tPtr = p3 ; }
       if(nodeAddrP(dtmP,p3)->tPtr == p2 ) { nodeAddrP(dtmP,p3)->tPtr = np ; nodeAddrP(dtmP,np)->tPtr = p2 ; }
       if( insertLine ) if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,p2,p3,np)) goto errexit ;
       p1 = np ;
      }
   }
 else  p1 = ridgePoint ;
/*
** Re-iteratively Get Next point
*/
 lastZ = pointAddrP(dtmP,point)->z ;
 startPoint = point ;
 do
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"p1 = %10ld z = %12.5lf",p1,pointAddrP(dtmP,p1)->z) ;
    if( bcdtmDrainage_getNextMaximumAscentLineDtmObject(dtmP,startPoint,p1,&np1,&np2,&np3,&nx,&ny,&nz,&process) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"00 ** process = %2ld ** np1 = %10ld np2 = %10ld np3 = %10ld ** nx = %12.5lf ny = %12.5lf nz = %10.5lf",process,np1,np2,np3,nx,ny,nz) ;
    if( process && nz < lastZ ) process = 0 ;
    if( process )
      {
    lastZ = nz ;
       startPoint = p1 ;
       if( process )
         {
/*
**        Check point To point Tolerance
*/
          if( np2 != dtmP->nullPnt )
            {
             if     ( bcdtmMath_distance(nx,ny,pointAddrP(dtmP,np1)->x,pointAddrP(dtmP,np1)->y) <= dtmP->ppTol ) { np2 = np3 = dtmP->nullPnt ; }
             else if( bcdtmMath_distance(nx,ny,pointAddrP(dtmP,np2)->x,pointAddrP(dtmP,np2)->y) <= dtmP->ppTol ) { np1 = np2 ; np2 = np3 = dtmP->nullPnt ; }
             else if( bcdtmMath_distance(nx,ny,pointAddrP(dtmP,p1)->x, pointAddrP(dtmP,p1)->y)  <= dtmP->ppTol ) goto cleanup ;
            }
    if( dbg ) bcdtmWrite_message(0,0,0,"01 ** process = %2ld ** np1 = %10ld np2 = %10ld np3 = %10ld ** nx = %12.5lf ny = %12.5lf nz = %10.5lf",process,np1,np2,np3,nx,ny,nz) ;
/*
**        Check point To Line Tolerance
*/
          if( np2 != dtmP->nullPnt )
            {
             d1  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,nx,ny,pointAddrP(dtmP,np1)->x,pointAddrP(dtmP,np1)->y) ;
             d2  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,nx,ny,pointAddrP(dtmP,np2)->x,pointAddrP(dtmP,np2)->y) ;
             if      ( d1 <= d2 && d1 <= dtmP->ppTol ) { np2 = np3 = dtmP->nullPnt ; }
             else if ( d2 <= d1 && d2 <= dtmP->ppTol ) { np1 = np2 ; np2 = np3 = dtmP->nullPnt ; }
            }

    if( dbg ) bcdtmWrite_message(0,0,0,"02 ** process = %2ld ** np1 = %10ld np2 = %10ld np3 = %10ld ** nx = %12.5lf ny = %12.5lf nz = %10.5lf",process,np1,np2,np3,nx,ny,nz) ;
/*
**        Check point To Line Tolerance
*/
          if( np2 != dtmP->nullPnt && np3 != dtmP->nullPnt )
            {
             d1  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,np1)->x,pointAddrP(dtmP,np1)->y,pointAddrP(dtmP,np3)->x,pointAddrP(dtmP,np3)->y,nx,ny) ;
             d2  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,np2)->x,pointAddrP(dtmP,np2)->y,pointAddrP(dtmP,np3)->x,pointAddrP(dtmP,np3)->y,nx,ny) ;
             if      ( d1 <= d2 && d1 <= dtmP->ppTol ) { np2 = np3 = dtmP->nullPnt ; }
             else if ( d2 <= d1 && d2 <= dtmP->ppTol ) { np1 = np2 ; np2 = np3 = dtmP->nullPnt ; }
            }
    if( dbg ) bcdtmWrite_message(0,0,0,"03 ** process = %2ld ** np1 = %10ld np2 = %10ld np3 = %10ld ** nx = %12.5lf ny = %12.5lf nz = %10.5lf",process,np1,np2,np3,nx,ny,nz) ;
/*
**        Insert Line
*/
          if( np2 != dtmP->nullPnt )
            {
             dx = pointAddrP(dtmP,np2)->x - pointAddrP(dtmP,np1)->x ;
             dy = pointAddrP(dtmP,np2)->y - pointAddrP(dtmP,np1)->y ;
             dz = pointAddrP(dtmP,np2)->z - pointAddrP(dtmP,np1)->z ;
             if( fabs(dx) >= fabs(dy) ) nz = pointAddrP(dtmP,np1)->z +  dz * ( nx - pointAddrP(dtmP,np1)->x) / dx ;
             else                       nz = pointAddrP(dtmP,np1)->z +  dz * ( ny - pointAddrP(dtmP,np1)->y) / dy ;
/*
**           Insert Line
*/
             insertLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,np1,np2) ;
             if( bcdtmInsert_addPointToDtmObject(dtmP,nx,ny,nz,&np) ) goto errexit ;
             if( bcdtmList_deleteLineDtmObject(dtmP,np1,np2)) goto errexit ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,np,np1)) goto errexit ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,p1,dtmP->nullPnt)) goto errexit ;
             if( bcdtmList_insertLineBeforePointDtmObject(dtmP,np1,np,p1)) goto errexit ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,np1,p1)) goto errexit ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np2,np,p1)) goto errexit ;
             if( bcdtmList_insertLineBeforePointDtmObject(dtmP,np,np2,p1)) goto errexit ;
             if( np3 != dtmP->nullPnt )
               {
                if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np3,np,np2)) goto errexit ;
                if( bcdtmList_insertLineBeforePointDtmObject(dtmP,np,np3,np2)) goto errexit ;
               }
             else
               {
                if(nodeAddrP(dtmP,np1)->hPtr == np2 ) { nodeAddrP(dtmP,np1)->hPtr = np ; nodeAddrP(dtmP,np)->hPtr = np2 ; }
                if(nodeAddrP(dtmP,np2)->hPtr == np1 ) { nodeAddrP(dtmP,np2)->hPtr = np ; nodeAddrP(dtmP,np)->hPtr = np1 ; }
               }
             if(nodeAddrP(dtmP,np1)->tPtr == np2 ) { nodeAddrP(dtmP,np1)->tPtr = np ; nodeAddrP(dtmP,np)->tPtr = np2 ; }
             if(nodeAddrP(dtmP,np2)->tPtr == np1 ) { nodeAddrP(dtmP,np2)->tPtr = np ; nodeAddrP(dtmP,np)->tPtr = np1 ; }
             if( insertLine ) if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,np1,np2,np)) goto errexit ;

             p1 = np ;
            }
          else  p1 = np1 ;
          if( nx == fx && ny == fy ) process = 0 ;
         }
      }
   } while ( process ) ;
/*
** CleanUp
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
int bcdtmDrainage_scanBetweenPointsForRidgeLineDtmObject(BC_DTM_OBJ *dtmP,long Point,long Sp,long Ep,long *RidgePoint)
/*
** This Function Scans Between Points For A Ridge Line
*/
{
 long   pp,sp,np,clc,fd1,fd2 ;
 double dx,dy,dz,dd,slope,mslope=0.0 ;
/*
** Initialise
*/
 *RidgePoint = dtmP->nullPnt ;
 if( Point < 0 || Point >= dtmP->numPoints ) return(0) ;
 if( ( clc = nodeAddrP(dtmP,Point)->cPtr) == dtmP->nullPtr ) return(0) ;
 sp = clistAddrP(dtmP,clc)->pntNum ;
/*
** Scan Around Point And Look For Ridge
*/
 pp = Sp ;  np = Ep ;
 if(( sp = bcdtmList_nextClkDtmObject(dtmP,Point,pp)) < 0 ) return(1) ;
 while ( sp != Ep )
   {
    if(( np = bcdtmList_nextClkDtmObject(dtmP,Point,sp)) < 0 ) return(1) ;
    if(nodeAddrP(dtmP,Point)->hPtr != pp && nodeAddrP(dtmP,Point)->hPtr != sp && nodeAddrP(dtmP,sp)->hPtr != Point && nodeAddrP(dtmP,np)->hPtr != Point  )
      {
       if( pointAddrP(dtmP,Point)->z <= pointAddrP(dtmP,sp)->z )
         {
          fd1 = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,sp,Point,pp) ;
          fd2 = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,Point,sp,np) ;
          if( fd1  <= 0 && fd2 <= 0 )
            {
             dx = pointAddrP(dtmP,sp)->x - pointAddrP(dtmP,Point)->x ;
             dy = pointAddrP(dtmP,sp)->y - pointAddrP(dtmP,Point)->y ;
             dz = pointAddrP(dtmP,sp)->z - pointAddrP(dtmP,Point)->z ;
             dd = sqrt(dx*dx + dy*dy) ;
             slope = dz/dd ;
             if     ( *RidgePoint == dtmP->nullPnt ) { *RidgePoint = sp ; mslope = slope ; }
             else if( slope  > mslope )              { *RidgePoint = sp ; mslope = slope ; }
            }
         }
      }
    pp = sp ; sp = np ;
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
int bcdtmDrainage_scanBetweenPointsForMaximumAscentTriangleDtmObject
(
 BC_DTM_OBJ *dtmP,          /* ==> Pointer To Tin Object                */
 long useTables,             /* ==> Use Drainage Tables                  */
 long point,                 /* ==> Point To Scan About                  */
 long startPoint,            /* ==> Start Point Of Scan                  */
 long endPoint,              /* ==> End Point Of Scan                    */
 long *trgPnt1P,             /* <== Triangle Base Point 1                */
 long *trgPnt2P,             /* <== Triangle Base Point 2                */
 double *trgAscentAngleP,    /* <== Maximum Ascent Angle                 */
 double *trgSlopeP           /* <== Maximum Ascent Slope                 */
)
/*
** This Function Scans In A Clockwise Direction Between Points For The Maximum Ascent Triangle
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   scanPnt,nextPnt,trgNum=0 ;
 bool voidTrg;
 double a1,a2,a3,angScanPnt,angNextPnt,slope,descentAngle,ascentAngle  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Ascent Triangle") ;
/*
** Initialise
*/
 *trgPnt1P = dtmP->nullPnt ;
 *trgPnt2P = dtmP->nullPnt ;
 *trgAscentAngleP = 0.0 ;
 *trgSlopeP  = 0.0 ;
/*
** Scan Around point
*/
 scanPnt = startPoint ;  ;
 angScanPnt = bcdtmMath_getPointAngleDtmObject(dtmP,point,scanPnt) ;
 if(( nextPnt = bcdtmList_nextClkDtmObject(dtmP,point,scanPnt)) < 0 ) goto errexit ;
 while ( scanPnt != endPoint )
   {
    angNextPnt = bcdtmMath_getPointAngleDtmObject(dtmP,point,nextPnt) ;
/*
**  Check For Internal Tin Triangle
*/
    if(  nodeAddrP(dtmP,point)->hPtr != scanPnt )
      {
/*
**     Test For Void Triangle
*/
       if( useTables )
         {
//          if( bcdtmDrainage_getTriangleNumberDtmObject(dtmP,trgIdxTabP,point,scanPnt,nextPnt,&trgNum)) goto errexit ;
//          voidTrg = ( trgIdxTabP+trgNum)->voidTriangle ;
         }
       else if( bcdtmList_testForVoidTriangleDtmObject(dtmP,point,scanPnt,nextPnt,voidTrg)) goto errexit ;
/*
**     Get Descent And Ascent Angles
*/
       if( ! voidTrg )
         {
          if( useTables )
            {
//             ascentAngle = (trgHydTabP+trgNum)->ascentAngle ;
//             slope       = (trgHydTabP+trgNum)->slope ;
            }
          else bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,point,scanPnt,nextPnt,&descentAngle,&ascentAngle,&slope) ;
          if( slope < 0.0 ) slope = -slope ;
          a1 = angScanPnt ;
          a2 = ascentAngle ;
          a3 = angNextPnt ;
          if( a1 < a3 ) a1 = a1 + DTM_2PYE ;
          if( a2 < a3 ) a2 = a2 + DTM_2PYE ;
          if( a2 <= a1 && a2 >= a3 )
            {
             if( *trgPnt1P == dtmP->nullPnt || slope > *trgSlopeP )
               {
                *trgPnt1P  = scanPnt ;
                *trgPnt2P  = nextPnt ;
                *trgSlopeP = slope   ;
                *trgAscentAngleP = ascentAngle ;
               }
            }
         }
      }
/*
**  Set For Next Triangle
*/
    scanPnt    = nextPnt ;
    angScanPnt = angNextPnt ;
    if(( nextPnt = bcdtmList_nextClkDtmObject(dtmP,point,nextPnt)) < 0 ) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Ascent Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Ascent Triangle Error") ;
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
int bcdtmDrainage_getNextMaximumAscentLineDtmObject
(
 BC_DTM_OBJ *dtmP,
 long   lastPoint,
 long   point,
 long   *nextPnt1P,
 long   *nextPnt2P,
 long   *nextPnt3P,
 double *nextXP,
 double *nextYP,
 double *nextZP,
 long   *processP
)
{
 int    ret=DTM_SUCCESS ;
 long   ridgePnt,trgPnt1,trgPnt2,hullLine=0 ;
 long   ascentType,precisionError,nextPnt=0,fixType=0 ;
 double dx,dy,dz,ridgeSlope,trgSlope,radX,radY,radius,ascentAngle ;
/*
** Initialise Variables
*/
 *processP = 0 ;
 *nextPnt1P = dtmP->nullPnt ;
 *nextPnt2P = dtmP->nullPnt ;
 *nextPnt3P = dtmP->nullPnt ;
 *nextXP  = 0.0 ;
 *nextYP  = 0.0 ;
 *nextZP  = 0.0 ;
 ridgePnt = trgPnt1 = trgPnt2 = dtmP->nullPnt ;
/*
** Check For Termination On A DTM Hull Line
*/
// if( bcdtmDrainage_checkForPointOnHullLineDtmObject(dtmP,point,&hullLine))goto errexit ;
 if( ! hullLine )
   {
/*
**  Test Last Point And Current Point Are Connected
*/
   if( lastPoint == dtmP->nullPnt || bcdtmList_testLineDtmObject(dtmP,point,lastPoint))
     {
/*
**    Scan Point For Maximum Ascent Ridge Line
*/
      bcdtmDrainage_scanPointForMaximumAscentRidgeLineDtmObject(dtmP,0,point,lastPoint,&ridgePnt,&ridgeSlope) ;
      if( ridgeSlope == 0.0 ) ridgePnt = dtmP->nullPnt ;
/*
**    Scan Point And Get Maximum Ascent Triangle
*/
      if( bcdtmDrainage_scanPointForMaximumAscentTriangleDtmObject(dtmP,0,point,lastPoint,&trgPnt1,&trgPnt2,&ascentAngle,&trgSlope) )goto errexit ;
      if( trgSlope == 0.0 ) trgPnt1 = trgPnt2 = dtmP->nullPnt ;
/*
**    Only Process If Maximum Ascent Found
*/
      if( ridgePnt != dtmP->nullPnt || trgPnt1 != dtmP->nullPnt )
        {
/*
**       Determine Ascent Type
*/
         ascentType = 0 ;
         if( ridgePnt != dtmP->nullPnt && trgPnt1 == dtmP->nullPnt ) ascentType = 1 ;
         if( ridgePnt != dtmP->nullPnt && trgPnt1 != dtmP->nullPnt && ridgeSlope >= trgSlope ) ascentType = 1 ;
         if( ridgePnt == dtmP->nullPnt && trgPnt1 != dtmP->nullPnt ) ascentType = 2 ;
         if( ridgePnt != dtmP->nullPnt && trgPnt1 != dtmP->nullPnt && ridgeSlope <  trgSlope ) ascentType = 2 ;
/*
**       Maximum Ascent Up Ridge
*/
         if( ascentType == 1 )
           {
            *nextPnt1P = ridgePnt ;
            *nextXP = pointAddrP(dtmP,ridgePnt)->x ;
            *nextYP = pointAddrP(dtmP,ridgePnt)->x ;
            *nextZP = pointAddrP(dtmP,ridgePnt)->z ;
            *processP = 1 ;
           }
/*
**        Maximum Ascent Up Triangle
*/
          if( ascentType == 2 )
            {
/*
**           Caculate Radial Out From Point
*/
             radius = sqrt((dtmP->xMax-dtmP->xMin)*(dtmP->xMax-dtmP->xMin)+(dtmP->yMax-dtmP->yMin)*(dtmP->yMax-dtmP->yMin)) ;
             radX = pointAddrP(dtmP,point)->x + radius * cos(ascentAngle) ;
             radY = pointAddrP(dtmP,point)->y + radius * sin(ascentAngle) ;
/*
**           Calculate Radial Intercept On trgPnt1-trgPnt2
*/
             bcdtmDrainage_intersectCordLines(pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,radX,radY,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,nextXP,nextYP)  ;
             dx = pointAddrP(dtmP,trgPnt2)->x - pointAddrP(dtmP,trgPnt1)->x ;
             dy = pointAddrP(dtmP,trgPnt2)->y - pointAddrP(dtmP,trgPnt1)->y ;
             dz = pointAddrP(dtmP,trgPnt2)->z - pointAddrP(dtmP,trgPnt1)->z ;
             if( fabs(dx) >= fabs(dy) ) *nextZP = pointAddrP(dtmP,trgPnt1)->z +  dz * (*nextXP - pointAddrP(dtmP,trgPnt1)->x) / dx ;
             else                       *nextZP = pointAddrP(dtmP,trgPnt1)->z +  dz * (*nextYP - pointAddrP(dtmP,trgPnt1)->y) / dy ;
/*
**           Test Point To Point Tolerance
*/
             if     ( bcdtmMath_distance(pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,*nextXP,*nextYP) <= dtmP->ppTol ) *nextPnt1P = trgPnt1 ;
             else if( bcdtmMath_distance(pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,*nextXP,*nextYP) <= dtmP->ppTol ) *nextPnt1P = trgPnt2 ;
             if( *nextPnt1P != dtmP->nullPnt )
               {
                *nextXP = pointAddrP(dtmP,*nextPnt1P)->x ;
                *nextYP = pointAddrP(dtmP,*nextPnt1P)->x ;
                *nextZP = pointAddrP(dtmP,*nextPnt1P)->z ;
                *processP = 1 ;
               }
             else
               {
                *nextPnt1P = trgPnt1 ;
                *nextPnt2P = trgPnt2 ;
                if( ( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 )goto errexit ;
                if( ! bcdtmList_testLineDtmObject(dtmP,*nextPnt3P,trgPnt2)) *nextPnt3P = dtmP->nullPnt   ;
                *processP = 1 ;
/*
**              Check For Potential Precision Error
*/
                if( *nextPnt3P != dtmP->nullPnt )
                  {
                   if( bcdtmInsert_checkPointQuadrilateralPrecisionDtmObject(dtmP,point,trgPnt1,*nextPnt3P,trgPnt2,*nextXP,*nextYP,&precisionError)) goto errexit ;
                   if( precisionError )
                     {
//                      if( bcdtmDrainage_fixDrainagePrecisonDtmObject(dtmP,ascentAngle,point,trgPnt1,*nextPnt3P,trgPnt2,*nextXP,*nextYP,&nextPnt,nextXP,nextYP,&fixType,processP)) goto errexit ;
                      if( fixType == 2 )
                        {
                         *nextPnt1P = nextPnt ;
                         *nextPnt2P = dtmP->nullPnt ;
                         *nextPnt3P = dtmP->nullPnt ;
                         *nextXP  = pointAddrP(dtmP,nextPnt)->x ;
                         *nextYP  = pointAddrP(dtmP,nextPnt)->y ;
                         *nextZP  = pointAddrP(dtmP,nextPnt)->z ;
                        }
                     }
                  }
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
int bcdtmDrainage_copyCatchmentTrianglesDtmObject
(
 BC_DTM_OBJ   *dtmP,                      /* ==> Pointer To DTM Object                       */
 long         startPoint,                 /* ==> StartPoint Of Catchment Tptr Polygon        */
 BC_DTM_OBJ   **catchDtmPP                /* ==> DTM To Store Catchment Triangles            */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p1,firstPnt,nextPnt,numTrgPts;
 DTMDirection direction ;
 long   pnt1,pnt2,pnt3,clPtr,addPnt,listPnt,*pointsP=NULL ;
 double area ;
 DPoint3d    dtmPoint[4] ;
 DTM_TIN_POINT *pntP ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Copying Catchment Triangles") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPoint       = %8ld",startPoint) ;
    bcdtmWrite_message(0,0,0,"*catchDtmPP      = %p",*catchDtmPP) ;
   }
/*
** Initialise
*/
 if( *catchDtmPP != NULL ) bcdtmObject_destroyDtmObject(catchDtmPP) ;
/*
** Check Direction Of Tptr Polygon
*/
 if( cdbg )
   {
    if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction)) goto errexit ;
    if( direction == DTMDirection::Clockwise )
      {
       bcdtmWrite_message(2,0,0,"Tptr Polygon Has A Clockwise Direction") ;
       goto errexit ;
      }
   }
/*
** Create Internal List Of Catchment Points
*/
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 if( bcdtmTinVolume_createSptrListOfInternalAndTptrPolygonPointsDtmObject(dtmP,startPoint,&firstPnt) ) goto errexit ;
/*
** Count Number Of Points
*/
 if( dbg == 1 )
   {
    numTrgPts = 0 ;
    nextPnt = firstPnt ;
    while( nextPnt != dtmP->nullPnt )
      {
       ++numTrgPts ;
       if( ( p1 = nodeAddrP(dtmP,nextPnt)->sPtr) == nextPnt )  nextPnt = dtmP->nullPnt ;
       else                                                    nextPnt = p1 ;
      }
    bcdtmWrite_message(0,0,0,"Number Of Triangle Pts = %8ld",numTrgPts) ;
   }
/*
** Allocate Memory For Point Offsets
*/
 pointsP = ( long * ) malloc(dtmP->numPoints*sizeof(long)) ;
 if( pointsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Create Catchment DTM
*/
 if( bcdtmObject_createDtmObject(catchDtmPP)) goto errexit ;
/*
** Store Points In DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Points") ;
 addPnt = firstPnt ;
 while( addPnt != dtmP->nullPnt )
   {
    pntP = pointAddrP(dtmP,addPnt) ;
    dtmPoint[0].x = pntP->x ;
    dtmPoint[0].y = pntP->y ;
    dtmPoint[0].z = pntP->z ;
    *(pointsP+addPnt) = (*catchDtmPP)->numPoints ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(*catchDtmPP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,dtmPoint,1)) goto errexit ;
    nextPnt = nodeAddrP(dtmP,addPnt)->sPtr ;
    if( nextPnt == addPnt ) addPnt = dtmP->nullPnt ;
    else                    addPnt = nextPnt ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"*catchDtmPP->numPoints = %8ld",(*catchDtmPP)->numPoints) ;
/*
**  Resize memory
*/
 if( bcdtmObject_resizeMemoryDtmObject(*catchDtmPP)) goto errexit ;
/*
**  Allocate Nodes Memory For Dtm Object
*/
 if( bcdtmObject_allocateNodesMemoryDtmObject(*catchDtmPP)) goto errexit ;
/*
** Initialise Circular List Parameters And Allocate Memory
*/
 (*catchDtmPP)->cListPtr = 0 ;
 (*catchDtmPP)->cListDelPtr = (*catchDtmPP)->nullPtr ;
 (*catchDtmPP)->numSortedPoints = 0 ;
 (*catchDtmPP)->numLines = 0 ;
 if( bcdtmObject_allocateCircularListMemoryDtmObject(*catchDtmPP) ) goto errexit ;
/*
** Store Lines In DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Lines") ;
 addPnt = firstPnt ;
 while( addPnt != dtmP->nullPnt )
   {
    pnt1 = *(pointsP+addPnt) ;
    pnt3 = (*catchDtmPP)->nullPnt ;
    clPtr = nodeAddrP(dtmP,addPnt)->cPtr ;
    while( clPtr != dtmP->nullPtr )
      {
       listPnt = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr   = clistAddrP(dtmP,clPtr)->nextPtr ;
       if( nodeAddrP(dtmP,listPnt)->sPtr != dtmP->nullPnt )
         {
          pnt2 = *(pointsP+listPnt) ;
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Inserting Line %8ld %8ld %10ld",pnt1,pnt2,pnt3) ;
          if( bcdtmList_insertLineAfterPointDtmObject(*catchDtmPP,pnt1,pnt2,pnt3)) goto errexit ;
          pnt3 = pnt2 ;
         }
      }
    nextPnt = nodeAddrP(dtmP,addPnt)->sPtr ;
    if( nextPnt == addPnt ) addPnt = dtmP->nullPnt ;
    else                    addPnt = nextPnt ;
  }
/*
** Store Hull In DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Hull") ;
 addPnt = startPoint ;
 pnt1 = *(pointsP+addPnt) ;
 do
   {
    nextPnt = nodeAddrP(dtmP,addPnt)->tPtr ;
    pnt2 = *(pointsP+nextPnt) ;
    if( ! bcdtmList_testLineDtmObject(*catchDtmPP,pnt1,pnt2))
      {
       bcdtmWrite_message(1,0,0,"Hull Points %8ld %8ld Not Connected",pnt1,pnt2) ;
       goto errexit ;
      }
    nodeAddrP(*catchDtmPP,pnt1)->hPtr = pnt2 ;
    addPnt = nextPnt ;
    pnt1 = pnt2 ;
   } while ( addPnt != startPoint ) ;
/*
** Set Hull Point Header Values
*/
 (*catchDtmPP)->hullPoint = *(pointsP+startPoint) ;
 (*catchDtmPP)->nextHullPoint = nodeAddrP(*catchDtmPP,(*catchDtmPP)->hullPoint)->hPtr ;
/*
** Remove Triangles External To Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Triangles External To Hull") ;
 pnt1 = (*catchDtmPP)->hullPoint ;
 do
   {
    pnt2 = nodeAddrP(*catchDtmPP,pnt1)->hPtr ;
    if( ( pnt3 = bcdtmList_nextClkDtmObject(*catchDtmPP,pnt1,pnt2)) < 0 ) goto errexit ;
    while( nodeAddrP(*catchDtmPP,pnt3)->hPtr != pnt1 )
      {
       if( ( listPnt = bcdtmList_nextClkDtmObject(*catchDtmPP,pnt1,pnt3)) < 0 ) goto errexit ;
       if( bcdtmList_deleteLineDtmObject(*catchDtmPP,pnt1,pnt3)) goto errexit ;
       pnt3 = listPnt ;
      }
    pnt1 = pnt2 ;
   } while ( pnt1 != (*catchDtmPP)->hullPoint ) ;
/*
** Set DTM State
*/
 (*catchDtmPP)->dtmState = DTMState::Tin ;
/*
** Clean Catchment DTM
*/
 if( bcdtmList_cleanDtmObject(*catchDtmPP)) goto errexit ;
/*
** Check Triangulation
*/
  if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(*catchDtmPP))
      {
       bcdtmWrite_message(0,0,0,"Triangulation Invalid") ;
       goto errexit ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
   }
/*
** Clear Sptr list
*/
 while( firstPnt != dtmP->nullPnt )
   {
    nextPnt = nodeAddrP(dtmP,firstPnt)->sPtr ;
    nodeAddrP(dtmP,firstPnt)->sPtr = dtmP->nullPnt ;
    if( nextPnt == firstPnt ) firstPnt = dtmP->nullPnt ;
    else                      firstPnt = nextPnt ;
   }
/*
** Clean Up
*/
 cleanup :
 if( pointsP != NULL ) { free(pointsP) ; pointsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Catchment Triangles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Catchment Triangles Error") ;
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
int bcdtmDrainage_insertMaximumAscentLineFromTinPointDtmObject
    (
    BC_DTM_OBJ *dtmP,                       // Pointer To DTM
    long       point                        // Tin Point To Start The Insertion From
    )

    // This Function Inserts A Maximum Line Into The Tin From A Starting Point

   {
   int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
   long   pnt1,nextPnt,lastPnt,nextPnt1,nextPnt2,nextPnt3,process,insertLine ;
   double d1,d2,dx,dy,dz,nextX,nextY,nextZ,lastZ ;

   // Log Arguments

   if( dbg )
       {
       bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Tin Point") ;
       bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
       bcdtmWrite_message(0,0,0,"point     = %8ld",point) ;
       }

   //  Iteratively Get And Insert Next Maximum Ascent Line

   pnt1 = point ;
   lastPnt = dtmP->nullPnt ;
   lastZ   = pointAddrP(dtmP,point)->z ;
   do
       {
    if( bcdtmDrainage_getNextMaximumAscentLineDtmObject(dtmP,lastPnt,pnt1,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process) ) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"process = %2ld ** nextPnt1 = %8ld nextPnt2 = %8ld nextPnt3 = %8ld ** %12.5lf %12.5lf %10.4lf",process,nextPnt1,nextPnt2,nextPnt3,nextX,nextY,nextZ) ;
    if( lastPnt != dtmP->nullPnt && nextZ <= pointAddrP(dtmP,lastPnt)->z ) process=0 ;
    if( process )
      {

//     Check Ascent Is Going Up

       if( cdbg )
           {
           if( nextZ < lastZ )
               {
               if( dbg == 0 ) bcdtmWrite_message(0,0,0,"Last Elevation = %15.8lf Current Elevation = %15.8lf",lastZ,nextZ) ;
               bcdtmWrite_message(2,0,0,"From Tin Point ** Ascent Elevation Is Lower") ;
//               goto errexit ;
               }
           }

//     Reset Variables

       lastPnt = pnt1 ;
       lastZ = nextZ ;

       if( process )
         {
/*
**        Check Point To Point Tolerance
*/
          if( nextPnt2 != dtmP->nullPnt )
            {
             if     ( bcdtmMath_distance(nextX,nextY,pointAddrP(dtmP,nextPnt1)->x,pointAddrP(dtmP,nextPnt1)->y) <= dtmP->ppTol ) { nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
             else if( bcdtmMath_distance(nextX,nextY,pointAddrP(dtmP,nextPnt2)->x,pointAddrP(dtmP,nextPnt2)->y) <= dtmP->ppTol ) { nextPnt1 = nextPnt2 ; nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
             else if( bcdtmMath_distance(nextX,nextY,pointAddrP(dtmP,pnt1)->x, pointAddrP(dtmP,pnt1)->y)  <= dtmP->ppTol ) return(0) ;
            }
/*
**        Check Point To Line Tolerance
*/
          if( nextPnt2 != dtmP->nullPnt )
            {
             d1  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,nextX,nextY,pointAddrP(dtmP,nextPnt1)->x,pointAddrP(dtmP,nextPnt1)->y) ;
             d2  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,nextX,nextY,pointAddrP(dtmP,nextPnt2)->x,pointAddrP(dtmP,nextPnt2)->y) ;
             if      ( d1 <= d2 && d1 <= dtmP->ppTol ) { nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
             else if ( d2 <= d1 && d2 <= dtmP->ppTol ) { nextPnt1 = nextPnt2 ; nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
            }
/*
**        Check Point To Line Tolerance
*/
          if( nextPnt2 != dtmP->nullPnt && nextPnt3 != dtmP->nullPnt )
            {
             d1  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,nextPnt1)->x,pointAddrP(dtmP,nextPnt1)->y,pointAddrP(dtmP,nextPnt3)->x,pointAddrP(dtmP,nextPnt3)->y,nextX,nextY) ;
             d2  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,nextPnt2)->x,pointAddrP(dtmP,nextPnt2)->y,pointAddrP(dtmP,nextPnt3)->x,pointAddrP(dtmP,nextPnt3)->y,nextX,nextY) ;
             if      ( d1 <= d2 && d1 <= dtmP->ppTol ) { nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
             else if ( d2 <= d1 && d2 <= dtmP->ppTol ) { nextPnt1 = nextPnt2 ; nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
            }
/*
**        Insert Line
*/
          if( nextPnt2 != dtmP->nullPnt )
            {
             dx = pointAddrP(dtmP,nextPnt2)->x - pointAddrP(dtmP,nextPnt1)->x ;
             dy = pointAddrP(dtmP,nextPnt2)->y - pointAddrP(dtmP,nextPnt1)->y ;
             dz = pointAddrP(dtmP,nextPnt2)->z - pointAddrP(dtmP,nextPnt1)->z ;
             if( fabs(dx) >= fabs(dy) ) nextZ = pointAddrP(dtmP,nextPnt1)->z +  dz * ( nextX - pointAddrP(dtmP,nextPnt1)->x) / dx ;
             else                       nextZ = pointAddrP(dtmP,nextPnt1)->z +  dz * ( nextY - pointAddrP(dtmP,nextPnt1)->y) / dy ;
/*
**          Insert Line
*/
             insertLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,nextPnt1,nextPnt2) ;
             if( bcdtmInsert_addPointToDtmObject(dtmP,nextX,nextY,nextZ,&nextPnt) ) goto errexit  ;
             if( bcdtmList_deleteLineDtmObject(dtmP,nextPnt1,nextPnt2)) goto errexit  ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt1,nextPnt,nextPnt1)) goto errexit  ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nextPnt,pnt1,dtmP->nullPnt)) goto errexit  ;
             if( bcdtmList_insertLineBeforePointDtmObject(dtmP,nextPnt1,nextPnt,pnt1)) goto errexit  ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nextPnt,nextPnt1,pnt1)) goto errexit  ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nextPnt2,nextPnt,pnt1)) goto errexit  ;
             if( bcdtmList_insertLineBeforePointDtmObject(dtmP,nextPnt,nextPnt2,pnt1)) goto errexit  ;
             if( nextPnt3 != dtmP->nullPnt )
               {
                if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nextPnt3,nextPnt,nextPnt2)) goto errexit  ;
                if( bcdtmList_insertLineBeforePointDtmObject(dtmP,nextPnt,nextPnt3,nextPnt2)) goto errexit  ;
               }
             else
               {
                if(nodeAddrP(dtmP,nextPnt1)->hPtr == nextPnt2 ) { nodeAddrP(dtmP,nextPnt1)->hPtr = nextPnt ; nodeAddrP(dtmP,nextPnt)->hPtr = nextPnt2 ; }
                if(nodeAddrP(dtmP,nextPnt2)->hPtr == nextPnt1 ) { nodeAddrP(dtmP,nextPnt2)->hPtr = nextPnt ; nodeAddrP(dtmP,nextPnt)->hPtr = nextPnt1 ; }
               }
             if(nodeAddrP(dtmP,nextPnt1)->tPtr == nextPnt2 ) { nodeAddrP(dtmP,nextPnt1)->tPtr = nextPnt ; nodeAddrP(dtmP,nextPnt)->tPtr = nextPnt2 ; }
             if(nodeAddrP(dtmP,nextPnt2)->tPtr == nextPnt1 ) { nodeAddrP(dtmP,nextPnt2)->tPtr = nextPnt ; nodeAddrP(dtmP,nextPnt)->tPtr = nextPnt1 ; }
             if( insertLine ) if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,nextPnt1,nextPnt2,nextPnt)) goto errexit  ;
             pnt1 = nextPnt ;
            }
          else  pnt1 = nextPnt1 ;
         }
      }
   } while ( process ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Tin Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Tin Point Error") ;
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
int bcdtmDrainage_insertMaximumAscentLineFromPointOnTinLineDtmObject
    (
    BC_DTM_OBJ *dtmP,                       // Pointer To DTM
    int        point1,                      // Point At One End Of Tin Line
    int        point2,                      // Point At Other End Of Tin Line
    DPoint3d   point                        // Start Point For Inserting Maximum Ascent Line
    )

    // This Function Inserts A Maximum Ascent Line Into The Tin
    // From A Starting Point On A Tin Line

    {
    int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
    long   dtmPnt=dtmP->nullPnt,antPnt,clkPnt,lineType ;
    bool voidPnt;
    double d1,d2 ;

    // Log Arguments

    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Point On Tin Line") ;
        bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"point1    = %8ld",point1) ;
        bcdtmWrite_message(0,0,0,"point2    = %8ld",point2) ;
        bcdtmWrite_message(0,0,0,"point.x   = %12.5lf",point.x) ;
        bcdtmWrite_message(0,0,0,"point.y   = %12.5lf",point.y) ;
        bcdtmWrite_message(0,0,0,"point.z   = %12.5lf",point.z) ;
        }

    //  Test Tin Line Exists

    if( ! bcdtmList_testLineDtmObject(dtmP,point1,point2))
        {
        bcdtmWrite_message(2,0,0,"Not A Tin Line") ;
        goto errexit ;
        }

    //  Check If Point Is Within Point To Point Tolerance Of The Line End Points

    d1 = bcdtmMath_distance(point.x,point.y,pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y) ;
    d2 = bcdtmMath_distance(point.x,point.y,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y) ;
    if      ( d1 <= d2 && d1 < dtmP->ppTol ) dtmPnt = point1 ;
    else if ( d2 < dtmP->ppTol ) dtmPnt = point2 ;

    // Only Insert Point Into Line If It Is Not Coincident With One Of The Line End Points

    if( dtmPnt == dtmP->nullPnt )
        {

        //  Determine Line Type ( Internal = 1 , On Tin Hull = 2 )

        if( nodeAddrP(dtmP,point1)->hPtr == dtmP->nullPnt ||  nodeAddrP(dtmP,point2)->hPtr == dtmP->nullPnt )
            {
            lineType = 1 ;
            }
        else
            {
            lineType = 2 ;
            if( nodeAddrP(dtmP,point1)->hPtr != point2 )
                {
                dtmPnt = point1 ;
                point1 = point2 ;
                point2 = dtmPnt ;
                }
            }

        //  Check For Void Line

        if( bcdtmList_testForVoidLineDtmObject(dtmP,point1,point2,voidPnt)) goto errexit ;

        // Add Point To Tin

        if( bcdtmInsert_addPointToDtmObject(dtmP,point.x,point.y,point.z,&dtmPnt) ) goto errexit  ;

        //  Insert Point Into Line

        switch ( lineType )
            {

            case  1 :      /* Coincident With Internal Tin Line  */

                bcdtmList_testForVoidLineDtmObject(dtmP,point1,point2,voidPnt) ;
                if( (antPnt = bcdtmList_nextAntDtmObject(dtmP,point1,point2)) < 0 ) goto errexit ;
                if( (clkPnt = bcdtmList_nextClkDtmObject(dtmP,point1,point2)) < 0 ) goto errexit ;
                if(bcdtmList_deleteLineDtmObject(dtmP,point1,point2)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point1,dtmPnt,antPnt)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,point1,dtmP->nullPnt)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point2,dtmPnt,clkPnt)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,point2,point1)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,dtmPnt,point2)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,antPnt,point1)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,clkPnt,dtmPnt,point1)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,clkPnt,point2)) goto errexit ;
                if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,point1,point2) )
                    {
                    if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,point1,point2,dtmPnt)) goto errexit ;
                    }
                break ;

            case  2 :      /* Coincident With External Tin Line  */
                bcdtmList_testForVoidLineDtmObject(dtmP,point1,point2,voidPnt) ;
                if( (antPnt = bcdtmList_nextAntDtmObject(dtmP,point1,point2))   < 0 ) goto errexit ;
                if(bcdtmList_deleteLineDtmObject(dtmP,point1,point2)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point1,dtmPnt,antPnt)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,point1,dtmP->nullPnt)) goto errexit ;
                if(bcdtmList_insertLineBeforePointDtmObject(dtmP,point2,dtmPnt,antPnt)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,point2,point1)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,dtmPnt,point2)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,antPnt,point1)) goto errexit ;
                if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,point1,point2) )
                    {
                    if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,point1,point2,dtmPnt)) goto errexit ;
                    }
                nodeAddrP(dtmP,point1)->hPtr = dtmPnt ;
                nodeAddrP(dtmP,dtmPnt)->hPtr = point2 ;
                break ;

            default :
               bcdtmWrite_message(1,0,0,"Unknown Line Type") ;
               goto errexit ;
               break ;

            }

        //  Check Tin After Inserting Point

        if( cdbg )
            {
            if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin After Inserting Point Into Line") ;
            if( bcdtmCheck_tinComponentDtmObject(dtmP))
                {
                if( dbg ) bcdtmWrite_message(0,0,0,"Tin Invalid") ;
                goto errexit ;
                }
            if( dbg ) bcdtmWrite_message(0,0,0,"Tin Valid") ;
            }
       }

   //  Insert MaximumAscent Line From Tin Point

   if( bcdtmDrainage_insertMaximumAscentLineFromTinPointDtmObject(dtmP,dtmPnt)) goto errexit ;

// Clean Up

 cleanup :

// Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Point On Tin Line Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Point On Tin Point Error") ;
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
int bcdtmDrainage_insertMaximumAscentLineFromTriangleBasePointDtmObject
    (
    BC_DTM_OBJ *dtmP,                       // Pointer To DTM
    long       point1,                      // Tin Point Of Triangle Base
    long       point2,                      // Tin Point Of Triangle Base
    long       point3                       // Tin Point Of triangle Apex
    )

    // This Function Inserts A Maximum Line Into The Tin From A Starting Triangle
    // The Maximum Ascent Is Started From Point1 And Intersects The Triangle Base Point2 - Point3

   {
   int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
   int    trgFlow ;
   long   pnt1, nextPnt, lastPnt, nextPnt1, nextPnt2, nextPnt3, process, insertLine;
   bool   voidTrg;
   double d1,d2,dx,dy,dz,nextX,nextY,nextZ,lastZ,slope,descentAngle,ascentAngle ;
   bool   voidTriangle=false ;
   DTMDirection triangleDirection ;

   // Log Arguments

   int startPnt1=point1  ;
   int startPnt2=point2  ;

   if( dbg )
       {
       bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Triangle Base Point") ;
       bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
       bcdtmWrite_message(0,0,0,"point1    = %8ld ** %12.5lf %12.5lf %10.4lf",point1,pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y,pointAddrP(dtmP,point1)->z) ;
       bcdtmWrite_message(0,0,0,"point2    = %8ld ** %12.5lf %12.5lf %10.4lf",point2,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y,pointAddrP(dtmP,point2)->z) ;
       bcdtmWrite_message(0,0,0,"point3    = %8ld ** %12.5lf %12.5lf %10.4lf",point3,pointAddrP(dtmP,point3)->x,pointAddrP(dtmP,point3)->y,pointAddrP(dtmP,point3)->z) ;
       }

   //  Check For Valid Triangle

   if( cdbg )
       {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Triangle") ;
       if( ! bcdtmList_testLineDtmObject(dtmP,point1,point2) ||
           ! bcdtmList_testLineDtmObject(dtmP,point1,point3) ||
           ! bcdtmList_testLineDtmObject(dtmP,point2,point3)      )
           {
           bcdtmWrite_message(1,0,0,"Invalid Triangle ") ;
           goto errexit ;
           }
       }

   //  Check And Set Triangle Direction Clockwise

   triangleDirection = DTMDirection::Clockwise ;
   if( bcdtmMath_pointSideOfDtmObject(dtmP,point1,point2,point3) > 0 ) triangleDirection = DTMDirection::AntiClockwise ;
   if( dbg ) bcdtmWrite_message(0,0,0,"triangleDirection = %2ld",triangleDirection) ;
   if( triangleDirection == DTMDirection::AntiClockwise )
       {
       pnt1   = point1 ;
       point1 = point2 ;
       point2 = pnt1   ;
       triangleDirection = DTMDirection::Clockwise ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Reversed Triangle Direction ** point1 = %8ld point2 = %8ld point3 = %8ld",point1,point2,point3) ;
       }

   //  Check For Void Triangle

   if( cdbg )
       {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Void Triangle") ;
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,point1,point2,point3,voidTrg))
           {
           bcdtmWrite_message(1,0,0,"Cannot Calculate The Triangle Flow For A Void Triangle") ;
           goto errexit ;
           }
       }

   //  Check Triangle Flows To Base ( point1 - point2 )

   if( cdbg )
       {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Triangle Flows To Base") ;
       if( ( trgFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,point1,point2,point3)) != 1 )
           {
           bcdtmWrite_message(1,0,0,"Triangle Flow %2d Does Not Flow To Base Line",trgFlow) ;
           goto errexit ;
           }
       }

   //  Get Triangle Ascent Angle - Triangle Must Be Clockwise

   if( bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP,nullptr,point1,point2,point3,voidTriangle,slope,descentAngle,ascentAngle)) goto errexit ;
   if( dbg ) bcdtmWrite_message(0,0,0,"slope = %10.5lf ** descentAngle = %12.10lf ascentAngle = %12.10lf",slope,descentAngle,ascentAngle) ;
   dx = dtmP->xMax - dtmP->xMin ;
   dy = dtmP->yMax - dtmP->yMin ;
   d1 = sqrt(dx*dx+dy*dy) ;
   dx = pointAddrP(dtmP,point1)->x + d1 * cos(ascentAngle) ;
   dy = pointAddrP(dtmP,point1)->y + d1 * sin(ascentAngle) ;

   //  Check For Intersect With Triangle Edge Point2 - Point3

   if( ! bcdtmMath_checkIfLinesIntersect (pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y,dx,dy,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y,pointAddrP(dtmP,point3)->x,pointAddrP(dtmP,point3)->y))
       {

       if( dbg ) bcdtmWrite_message(0,0,0,"Intersect Not Found With Edge point2 = %8ld point3 = %8ld",point2,point3) ;

       //  Reverse Triangle Base Direction And Recalculate Ascent Angle

       pnt1   = point1 ;
       point1 = point2 ;
       point2 = pnt1 ;
       dx = pointAddrP(dtmP,point1)->x + d1 * cos(ascentAngle) ;
       dy = pointAddrP(dtmP,point1)->y + d1 * sin(ascentAngle) ;
       if( ! bcdtmMath_checkIfLinesIntersect (pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y,dx,dy,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y,pointAddrP(dtmP,point3)->x,pointAddrP(dtmP,point3)->y))
           {

           //  Check If Maximum Ascent Has Been Prior Inserted

           double angleP1P3,angleP2P3,diffAngle1,diffAngle2 ;
           angleP1P3 = bcdtmMath_getPointAngleDtmObject(dtmP,point1,point3) ;
           angleP2P3 = bcdtmMath_getPointAngleDtmObject(dtmP,point2,point3) ;
           diffAngle1 = fabs(ascentAngle-angleP1P3) ;
           if( DTM_2PYE - diffAngle1 < diffAngle1 ) diffAngle1 = DTM_2PYE - diffAngle1 ;
           diffAngle2 = fabs(ascentAngle-angleP2P3) ;
           if( DTM_2PYE - diffAngle2 < diffAngle2 ) diffAngle2 = DTM_2PYE - diffAngle1 ;
           if( diffAngle2 < 0.0000001 )
             {
              pnt1 = point1 ;
              point1 = point2 ;
              point2 = point1 ;
             }
           else if( diffAngle1 > 0.0000001 )
             {
             if( dbg == 1 )
                 {
                 bcdtmWrite_message(0,0,0,"Maximum Ascent From Vertex Does Not Intersect Triangle Base") ;
                 bcdtmWrite_message(0,0,0,"slope = %10.5lf ** descentAngle = %12.10lf ascentAngle = %12.10lf",slope,descentAngle,ascentAngle) ;
                 bcdtmWrite_message(0,0,0,"point1    = %8ld ** %12.5lf %12.5lf %10.4lf",point1,pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y,pointAddrP(dtmP,point1)->z) ;
                 bcdtmWrite_message(0,0,0,"point2    = %8ld ** %12.5lf %12.5lf %10.4lf",point2,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y,pointAddrP(dtmP,point2)->z) ;
                 bcdtmWrite_message(0,0,0,"point3    = %8ld ** %12.5lf %12.5lf %10.4lf",point3,pointAddrP(dtmP,point3)->x,pointAddrP(dtmP,point3)->y,pointAddrP(dtmP,point3)->z) ;
                 bcdtmWrite_message(0,0,0,"angleP1P3 = %12.10lf diffAngle1 = %12.10lf",angleP1P3,diffAngle1) ;
                 bcdtmWrite_message(0,0,0,"angleP2P3 = %12.10lf diffAngle2 = %12.10lf",angleP2P3,diffAngle2) ;
                 bcdtmWrite_message(0,0,0,"angle(point1,point2) = %12.10lf",bcdtmMath_getPointAngleDtmObject(dtmP,point1,point2));
                 bcdtmWrite_message(0,0,0,"angle(point1,point3) = %12.10lf",bcdtmMath_getPointAngleDtmObject(dtmP,point1,point3));
                 bcdtmWrite_message(0,0,0,"angle(point2,point1) = %12.10lf",bcdtmMath_getPointAngleDtmObject(dtmP,point2,point1));
                 bcdtmWrite_message(0,0,0,"angle(point2,point3) = %12.10lf",bcdtmMath_getPointAngleDtmObject(dtmP,point2,point3));
                 goto cleanup ;
                 }
            }
            goto cleanup ;  // Maximum Ascent Has been Prior Inserted
            }
       }

   // Calculate Intersect Of Triangle Ascent Radial With Triangle Edge

   bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP,point1,point2,point3,ascentAngle,&nextX,&nextY,&nextZ,&nextPnt) ;
   if( dbg ) bcdtmWrite_message(0,0,0,"Intersect Point = %12.5lf %12.5lf %10.4lf",nextX,nextY,nextZ) ;

   // Insert Point Into Triangle Edge

   if( nextPnt == dtmP->nullPnt )
       {
       if( bcdtmInsert_addPointToDtmObject(dtmP,nextX,nextY,nextZ,&nextPnt) ) goto errexit  ;
if( nextPnt == 288493 ) bcdtmWrite_message(0,0,0,"Inserted Point %8ld ** startPnt1 = %8ld startPnt2 = %8ld",nextPnt,startPnt1,startPnt2) ;
       if( bcdtmDrainage_insertPointIntoTinLineDtmObject( dtmP,nextPnt,point2,point3)) goto errexit ;
       }

   //  Iteratively Get And Insert Next Maximum Ascent Line

   pnt1    = nextPnt ;
   lastPnt = point1 ;
   lastZ   = pointAddrP(dtmP,point1)->z ;
   do
        {
        if( bcdtmDrainage_getNextMaximumAscentLineDtmObject(dtmP,lastPnt,pnt1,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process) ) goto errexit  ;
        if( dbg == 2 ) bcdtmWrite_message(0,0,0,"process = %2ld ** nextPnt1 = %8ld nextPnt2 = %8ld nextPnt3 = %8ld ** %12.5lf %12.5lf %10.4lf",process,nextPnt1,nextPnt2,nextPnt3,nextX,nextY,nextZ) ;
        if( lastPnt != dtmP->nullPnt && nextZ <= pointAddrP(dtmP,lastPnt)->z ) process=0 ;
        if( process )
            {

            //   Check Ascent Is Going Up

            if( cdbg )
                {
                if( nextZ < lastZ )
                    {
                    if( dbg == 0 ) bcdtmWrite_message(0,0,0,"Last Elevation = %15.8lf Current Elevation = %15.8lf",lastZ,nextZ) ;
                    bcdtmWrite_message(2,0,0,"From Triangle Base ** Ascent Elevation Is Lower") ;
                    goto cleanup ;
//                    goto errexit ;
                    }
                }

            //     Reset Variables

            lastPnt = pnt1 ;
            lastZ = nextZ ;

            if( process )
                {
/*
**        Check Point To Point Tolerance
*/
          if( nextPnt2 != dtmP->nullPnt )
            {
             if     ( bcdtmMath_distance(nextX,nextY,pointAddrP(dtmP,nextPnt1)->x,pointAddrP(dtmP,nextPnt1)->y) <= dtmP->ppTol ) { nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
             else if( bcdtmMath_distance(nextX,nextY,pointAddrP(dtmP,nextPnt2)->x,pointAddrP(dtmP,nextPnt2)->y) <= dtmP->ppTol ) { nextPnt1 = nextPnt2 ; nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
             else if( bcdtmMath_distance(nextX,nextY,pointAddrP(dtmP,pnt1)->x, pointAddrP(dtmP,pnt1)->y)  <= dtmP->ppTol ) return(0) ;
            }
/*
**        Check Point To Line Tolerance
*/
          if( nextPnt2 != dtmP->nullPnt )
            {
             d1  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,nextX,nextY,pointAddrP(dtmP,nextPnt1)->x,pointAddrP(dtmP,nextPnt1)->y) ;
             d2  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,nextX,nextY,pointAddrP(dtmP,nextPnt2)->x,pointAddrP(dtmP,nextPnt2)->y) ;
             if      ( d1 <= d2 && d1 <= dtmP->ppTol ) { nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
             else if ( d2 <= d1 && d2 <= dtmP->ppTol ) { nextPnt1 = nextPnt2 ; nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
            }
/*
**        Check Point To Line Tolerance
*/
          if( nextPnt2 != dtmP->nullPnt && nextPnt3 != dtmP->nullPnt )
            {
             d1  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,nextPnt1)->x,pointAddrP(dtmP,nextPnt1)->y,pointAddrP(dtmP,nextPnt3)->x,pointAddrP(dtmP,nextPnt3)->y,nextX,nextY) ;
             d2  = bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,nextPnt2)->x,pointAddrP(dtmP,nextPnt2)->y,pointAddrP(dtmP,nextPnt3)->x,pointAddrP(dtmP,nextPnt3)->y,nextX,nextY) ;
             if      ( d1 <= d2 && d1 <= dtmP->ppTol ) { nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
             else if ( d2 <= d1 && d2 <= dtmP->ppTol ) { nextPnt1 = nextPnt2 ; nextPnt2 = nextPnt3 = dtmP->nullPnt ; }
            }
/*
**        Insert Line
*/
          if( nextPnt2 != dtmP->nullPnt )
            {
             dx = pointAddrP(dtmP,nextPnt2)->x - pointAddrP(dtmP,nextPnt1)->x ;
             dy = pointAddrP(dtmP,nextPnt2)->y - pointAddrP(dtmP,nextPnt1)->y ;
             dz = pointAddrP(dtmP,nextPnt2)->z - pointAddrP(dtmP,nextPnt1)->z ;
             if( fabs(dx) >= fabs(dy) ) nextZ = pointAddrP(dtmP,nextPnt1)->z +  dz * ( nextX - pointAddrP(dtmP,nextPnt1)->x) / dx ;
             else                       nextZ = pointAddrP(dtmP,nextPnt1)->z +  dz * ( nextY - pointAddrP(dtmP,nextPnt1)->y) / dy ;
/*
**          Insert Line
*/
             insertLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,nextPnt1,nextPnt2) ;
             if( bcdtmInsert_addPointToDtmObject(dtmP,nextX,nextY,nextZ,&nextPnt) ) goto errexit  ;
if( nextPnt == 288493 ) bcdtmWrite_message(0,0,0,"** Inserted Point %8ld ** startPnt1 = %8ld startPnt2 = %8ld",nextPnt,startPnt1,startPnt2) ;

             if( bcdtmList_deleteLineDtmObject(dtmP,nextPnt1,nextPnt2)) goto errexit  ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,pnt1,nextPnt,nextPnt1)) goto errexit  ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nextPnt,pnt1,dtmP->nullPnt)) goto errexit  ;
             if( bcdtmList_insertLineBeforePointDtmObject(dtmP,nextPnt1,nextPnt,pnt1)) goto errexit  ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nextPnt,nextPnt1,pnt1)) goto errexit  ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nextPnt2,nextPnt,pnt1)) goto errexit  ;
             if( bcdtmList_insertLineBeforePointDtmObject(dtmP,nextPnt,nextPnt2,pnt1)) goto errexit  ;
             if( nextPnt3 != dtmP->nullPnt )
               {
                if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nextPnt3,nextPnt,nextPnt2)) goto errexit  ;
                if( bcdtmList_insertLineBeforePointDtmObject(dtmP,nextPnt,nextPnt3,nextPnt2)) goto errexit  ;
               }
             else
               {
                if(nodeAddrP(dtmP,nextPnt1)->hPtr == nextPnt2 ) { nodeAddrP(dtmP,nextPnt1)->hPtr = nextPnt ; nodeAddrP(dtmP,nextPnt)->hPtr = nextPnt2 ; }
                if(nodeAddrP(dtmP,nextPnt2)->hPtr == nextPnt1 ) { nodeAddrP(dtmP,nextPnt2)->hPtr = nextPnt ; nodeAddrP(dtmP,nextPnt)->hPtr = nextPnt1 ; }
               }
             if(nodeAddrP(dtmP,nextPnt1)->tPtr == nextPnt2 ) { nodeAddrP(dtmP,nextPnt1)->tPtr = nextPnt ; nodeAddrP(dtmP,nextPnt)->tPtr = nextPnt2 ; }
             if(nodeAddrP(dtmP,nextPnt2)->tPtr == nextPnt1 ) { nodeAddrP(dtmP,nextPnt2)->tPtr = nextPnt ; nodeAddrP(dtmP,nextPnt)->tPtr = nextPnt1 ; }
             if( insertLine ) if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,nextPnt1,nextPnt2,nextPnt)) goto errexit  ;
             pnt1 = nextPnt ;
            }
          else  pnt1 = nextPnt1 ;
         }
      }
   } while ( process ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Triangle Base Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Triangle Base Point Error") ;
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
int bcdtmDrainage_insertMaximumAscentLineFromTriangleEdgeDtmObject
    (
    BC_DTM_OBJ *dtmP,                       // Pointer To DTM
    long       point1,                      // Tin Point Of Triangle Base
    long       point2                       // Tin Point Of Triangle Base
    )

   // This Function Inserts A Maximum Ascent Line Into The Tin From A Triangle Edge
   // It Maximum Ascent Commences From The End Of The Edge Line

   {
   int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
   int    antFlow,clkFlow ;
   long   antPnt,clkPnt ;

   // Log Arguments

   if( dbg )
       {
       bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Triangle Base") ;
       bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
       bcdtmWrite_message(0,0,0,"point1    = %8ld ** %12.5lf %12.5lf %10.4lf",point1,pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y,pointAddrP(dtmP,point1)->z) ;
       bcdtmWrite_message(0,0,0,"point2    = %8ld ** %12.5lf %12.5lf %10.4lf",point2,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y,pointAddrP(dtmP,point2)->z) ;
       }

   //  Only Process If Traingle Edge Points Are Connected

   if( bcdtmList_testLineDtmObject(dtmP,point1,point2 ) )
       {

   //  Only Process If Triangle Edge Is not A Hull Line

       if( ! bcdtmList_testForHullLineDtmObject(dtmP,point1,point2) )
           {

           //  Get Points Either Side Of Triangle Edge

           if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,point1,point2 )) < 0 ) goto errexit ;
           if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point1,point2 )) < 0 ) goto errexit ;

           // Get Triangle Flow Directions

           antFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,point1,point2,antPnt) ;
           clkFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,point1,point2,clkPnt) ;

           if( dbg == 1 ) bcdtmWrite_message(0,0,0,"antFlow = %2d clkFlow = %2d",antFlow,clkFlow) ;

           if( clkFlow == 1 && antFlow != 1 )
               {
               if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent From Clock Flow") ;
               if( bcdtmDrainage_insertMaximumAscentLineFromTriangleBasePointDtmObject(dtmP,point1,point2,clkPnt)) goto errexit ;
               }
           if( antFlow == 1 && clkFlow != 1 )
               {
               if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent From Counter Ant Flow") ;
               if( bcdtmDrainage_insertMaximumAscentLineFromTriangleBasePointDtmObject(dtmP,point1,point2,antPnt)) goto errexit ;
               }
           }
       }

// Clean Up

cleanup :

//  Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Triangle Base Point Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Line From Triangle Base Point Error") ;
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
int  bcdtmDrainage_genericCallBackFunction
    (
    DTMFeatureType   dtmFeatureType,
    DTMUserTag       userTag,
    DTMFeatureId     featureId,
    DPoint3d         *featurePtsP,
    size_t           numFeaturePts,
    void             *userP
   )
    /*
    ** Sample DTM Interrupt Load Function
    **
    ** This Function Receives The Load Features From The DTM
    ** As The DTM Reuses The Feature Points Memory Do Not Free It
    ** If You Require The Feature Points Then Make A Copy
    **
    */
    {
    int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    char  dtmFeatureTypeName[100] ;
    BC_DTM_OBJ *dtmP=NULL ;
    DPoint3d  *p3dP ;
    /*
    ** Check For DTMFeatureType::CheckStop
    */
    // if(dtmFeatureType == DTMFeatureType::CheckStop) bcdtmWrite_message(0,0,0,"DTMFeatureType::CheckStop") ;
    /*
    ** Initialise
    */
    // if(numDtmFeatures % 1000 == 0) bcdtmWrite_message(0,0,0,"numDtmFeatures = %8ld",numDtmFeatures) ;
    /*
    ** Write Record
    */
    if(dbg == 1)
        {
        bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
        bcdtmWrite_message(0,0,0,"Feature %s ** userTag = %10I64d featureId = %10I64d featurePtsP = %p numFeaturePts = %6ld userP = %p",dtmFeatureTypeName,userTag,featureId,featurePtsP,numFeaturePts,userP) ;
        }
    /*
    ** Write Points
    */
    if(dbg == 2)
        {
        bcdtmWrite_message(0,0,0,"Number Of Feature Points = %6ld",numFeaturePts) ;
        for(p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP)
            {
            bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.4lf %12.4lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
            }
        }
    /*
    ** Store DTM Features In DTM
    */
//    if( userP != NULL && ( dtmFeatureType == DTMFeatureType::LowPointPond || dtmFeatureType == DTMFeatureType::DescentTrace ))
    if( userP != NULL && dtmFeatureType != DTMFeatureType::CheckStop )
        {
        dtmP = (BC_DTM_OBJ *) userP ;
        if(bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

        if( dtmFeatureType ==  DTMFeatureType::SumpLine || dtmFeatureType == DTMFeatureType::RidgeLine )
          {
           DPoint3d *lineP =  featurePtsP ;
           for( lineP = featurePtsP ; lineP < featurePtsP + numFeaturePts * 2 ; lineP = lineP + 2 )
              {
               if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,lineP,2)) goto errexit ;
              }
          }
        else
          {
           if(bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,featurePtsP,(long)numFeaturePts)) goto errexit ;
          }
       }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if(dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0,0,0,"Call Back Function Completed") ;
    if(dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0,0,0,"Call Back Function Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if(ret == DTM_SUCCESS) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_checkForMaximumAscentFlowLineDtmObject
    (
    BC_DTM_OBJ *dtmP,                       // ==> Pointer To DTM
    long       point1,                      // ==> Tin Point Of Flow Line
    long       point2,                      // ==> Tin Point Of Flow Line
    bool&      maxAscent                    // <== True If A Maximum Ascent Flow Line Else Flow
    )

    // This Function Checks If A Maximum Ascent Line Can Be Inserted From A Flow Line
   {
   int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
   long   pnt1,antPnt,clkPnt,tstPnt,antFlow,clkFlow;
   double d1,dx,dy,slope,descentAngle,ascentAngle ;
   bool   voidTriangle=false ;

   // Log Arguments

   if( dbg )
       {
       bcdtmWrite_message(0,0,0,"Checking For Maximum Ascent Flow Line") ;
       bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
       bcdtmWrite_message(0,0,0,"point1    = %8ld ** %12.5lf %12.5lf %10.4lf",point1,pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y,pointAddrP(dtmP,point1)->z) ;
       bcdtmWrite_message(0,0,0,"point2    = %8ld ** %12.5lf %12.5lf %10.4lf",point2,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y,pointAddrP(dtmP,point2)->z) ;
       }

   //  Set Max Ascent Parameter

   maxAscent = false ;

   //  Check If point2 and Point 3 are connected

   if( dbg ) bcdtmWrite_message(0,0,0,"Checking If Point1 And Point2 Are Connected") ;
   if( ! bcdtmList_testLineDtmObject(dtmP,point1,point2))
       {
       bcdtmWrite_message(1,0,0,"Unconnected Flow Line") ;
       goto cleanup ;
       }

   //  Only Test For None Hull Line

   if( ! bcdtmList_testForHullLineDtmObject(dtmP,point1,point2))
       {

       //  Get Clockwise And Anti Clockwise Points

       if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,point1,point2 )) < 0 ) goto errexit ;
       if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point1,point2 )) < 0 ) goto errexit ;
       if( dbg == 2 )
           {
           bcdtmWrite_message(0,0,0,"antPnt    = %8ld ** %12.5lf %12.5lf %10.4lf",antPnt,pointAddrP(dtmP,antPnt)->x,pointAddrP(dtmP,antPnt)->y,pointAddrP(dtmP,antPnt)->z) ;
           bcdtmWrite_message(0,0,0,"clkPnt    = %8ld ** %12.5lf %12.5lf %10.4lf",clkPnt,pointAddrP(dtmP,clkPnt)->x,pointAddrP(dtmP,clkPnt)->y,pointAddrP(dtmP,clkPnt)->z) ;
           }

       // Get Triangle Flow Directions

       antFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,point1,point2,antPnt) ;
       clkFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,point1,point2,clkPnt) ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"antFlow = %2d clkFlow = %2d",antFlow,clkFlow) ;

       // Check For Cross Flow

       tstPnt = dtmP->nullPnt ;
       if( clkFlow == 1 && antFlow != 1 )  tstPnt = clkPnt ;
       if( clkFlow != 1 && antFlow == 1 )
           {
           tstPnt = antPnt ;
           pnt1   = point1 ;
           point1 = point2 ;
           point2 = pnt1   ;
          }

       // If Cross Flow Check For Maximum Ascent

       if( tstPnt != dtmP->nullPnt )
           {

           //  Get Triangle Ascent Angle

           if( bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP,nullptr,point1,point2,tstPnt,voidTriangle,slope,descentAngle,ascentAngle)) goto errexit ;
           if( dbg ) bcdtmWrite_message(0,0,0,"slope = %10.5lf ** descentAngle = %12.10lf ascentAngle = %12.10lf",slope,descentAngle,ascentAngle) ;

           //  Calculate Ascent Radial

           dx = dtmP->xMax - dtmP->xMin ;
           dy = dtmP->yMax - dtmP->yMin ;
           d1 = sqrt(dx*dx+dy*dy) ;
           dx = pointAddrP(dtmP,point1)->x + d1 * cos(ascentAngle) ;
           dy = pointAddrP(dtmP,point1)->y + d1 * sin(ascentAngle) ;

           //  Check For Intersect With Triangle Edge point2 - tstPnt

           if( ! bcdtmMath_checkIfLinesIntersect (pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y,dx,dy,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y,pointAddrP(dtmP,tstPnt)->x,pointAddrP(dtmP,tstPnt)->y))
                {

                if( dbg ) bcdtmWrite_message(0,0,0,"Intersect Not Found With Edge point2 = %8ld point3 = %8ld",point2,tstPnt) ;

                //  Reverse Triangle Base Direction And Recalculate Ascent Radial

                pnt1   = point1 ;
                point1 = point2 ;
                point2 = pnt1 ;
                dx = pointAddrP(dtmP,point1)->x + d1 * cos(ascentAngle) ;
                dy = pointAddrP(dtmP,point1)->y + d1 * sin(ascentAngle) ;
                if( ! bcdtmMath_checkIfLinesIntersect (pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y,dx,dy,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y,pointAddrP(dtmP,tstPnt)->x,pointAddrP(dtmP,tstPnt)->y))
                    {
                    if( dbg ) bcdtmWrite_message(0,0,0,"Reversed Intersect Not Found With Edge point2 = %8ld tstPoint = %8ld",point2,tstPnt) ;
                    }
                else
                    maxAscent = true ;
                }
             else
                 maxAscent = true ;
             }
         }

     // Clean Up

cleanup :

    // Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Maximum Ascent Flow Line Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Maximum Ascent Flow Line Error") ;
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
int bcdtmDrainage_refineTptrCatchmentPolygonDtmObject
(
 BC_DTM_OBJ            *dtmP,
 double                sumpX,
 double                sumpY,
 double                sumpZ,
 long                  sumpPoint1,
 long                  sumpPoint2,
 long                  firstPoint,
 double                maxPondDepth,
 DPoint3d              **catchmentPtsPP,
 long                  *numCatchmentPtsP
 )

    //  This Function Refines A Tptr Catchment Polygon

    {
    int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
    long   p1,p2,p3;
    BC_DTM_OBJ    *catchmentDtmP=NULL,*catchDtmP=NULL,*catchmentPolygonsDtmP=NULL,*tempDtmP=NULL ;

    long   pnt,npnt,smpPnt1,smpPnt2,smpType=0,numTriangles ;
    DPoint3d sumpPoint,tracePoints[4],*catchPtsP=NULL  ;
    long    drainPoint,numTracedToSump ;
    bool   refine=true,tracedToSump,traceOverZeroSlope=true ;
    double x,y,z,drainPointX,drainPointY,drainPointZ ;
    DTMTriangleIndex *triangleIndexP=nullptr ;
    long startTime ;
    int numCatchments,dtmFeature,numZero,numOne,numTwo,numThree,count ;

// Log Entry Arguments

    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Refining Tptr Catchment Polygon") ;
        bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP)  ;
        bcdtmWrite_message(0,0,0,"sumpX            = %12.5lf",sumpX)  ;
        bcdtmWrite_message(0,0,0,"sumpY            = %12.5lf",sumpY)  ;
        bcdtmWrite_message(0,0,0,"sumpZ            = %12.5lf",sumpZ)  ;
        bcdtmWrite_message(0,0,0,"sumpPoint1       = %8ld",sumpPoint1) ;
        bcdtmWrite_message(0,0,0,"sumpPoint2       = %8ld",sumpPoint2) ;
        bcdtmWrite_message(0,0,0,"firstPoint       = %8ld",firstPoint) ;
        bcdtmWrite_message(0,0,0,"maxPondDepth     = %8.3lf",maxPondDepth) ;
        bcdtmWrite_message(0,0,0,"catchmentPtsPP   = %p",*catchmentPtsPP) ;
        bcdtmWrite_message(0,0,0,"numCatchmentPtsP = %8ld",*numCatchmentPtsP) ;
        }

    // Check Connectivity Of Tptr Polygon

    if( cdbg )
        {
        if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,firstPoint,0))
            {
            bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
            goto errexit ;
            }
        }

    //  Clone DTM To Create A Catchment DTM

    if( bcdtmObject_cloneDtmObject(dtmP,&catchmentDtmP)) goto errexit ;

    //  Insert Tptr Polygon Into Catchment DTM

    pnt = firstPoint ;
    do
        {
         npnt = nodeAddrP(dtmP,pnt)->tPtr ;
         nodeAddrP(catchmentDtmP,pnt)->tPtr = npnt ;
         pnt = npnt ;
        } while ( pnt != firstPoint ) ;

   // Check Connectivity Of Tptr Polygon

    if( cdbg )
        {
        if( bcdtmList_checkConnectivityTptrPolygonDtmObject(catchmentDtmP,firstPoint,0))
            {
            bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
            goto errexit ;
            }
        }

    //  Clip To Tptr Catchment Polygon

    if (bcdtmClip_toTptrPolygonDtmObject (catchmentDtmP, firstPoint, DTMClipOption::External)) goto errexit;

    //  Check Clipped Catchment DTM

    if( cdbg )
        {
         if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid DTM") ;
         if( bcdtmCheck_tinComponentDtmObject(catchmentDtmP))
             {
             bcdtmWrite_message(2,0,0,"DTM Invalid") ;
             goto errexit ;
             }
         if( dbg ) bcdtmWrite_message(0,0,0,"DTM Valid") ;
        }

    // Null Drain Point

    drainPoint = catchmentDtmP->nullPnt ;

    // Log Catchment Tin

    if( dbg == 1 )
        {
        if( bcdtmWrite_toFileDtmObject(catchmentDtmP,L"clippedCatchment.tin")) goto errexit ;
        }

    //  Find Corresponding Sump Points In The Clipped DTM

    smpPnt1 = smpPnt2 = catchmentDtmP->nullPnt ;
    if( sumpPoint1 != dtmP->nullPnt )
        {
        bcdtmFind_closestPointDtmObject(catchmentDtmP,pointAddrP(dtmP,sumpPoint1)->x,pointAddrP(dtmP,sumpPoint1)->y,&smpPnt1) ;
        if( cdbg == 1 )
            {
            if( pointAddrP(catchmentDtmP,smpPnt1)->x != pointAddrP(dtmP,sumpPoint1)->x || pointAddrP(catchmentDtmP,smpPnt1)->y != pointAddrP(dtmP,sumpPoint1)->y )
                {
                bcdtmWrite_message(2,0,0,"Closest Point Error") ;
                goto errexit ;
                }
            }
        }
    if( sumpPoint2 != dtmP->nullPnt )
        {
        bcdtmFind_closestPointDtmObject(catchmentDtmP,pointAddrP(dtmP,sumpPoint2)->x,pointAddrP(dtmP,sumpPoint2)->y,&smpPnt2) ;
        if( cdbg == 1 )
            {
            if( pointAddrP(catchmentDtmP,smpPnt2)->x != pointAddrP(dtmP,sumpPoint2)->x || pointAddrP(catchmentDtmP,smpPnt2)->y != pointAddrP(dtmP,sumpPoint2)->y )
                {
                bcdtmWrite_message(2,0,0,"Closest Point Error") ;
                goto errexit ;
                }
            }
        }
     smpType = 2 ;                                             //  Sump Line
     if( smpPnt2 == catchmentDtmP->nullPnt ) smpType = 1 ;     //  Sump Point

     // Log New Sump Points

     if( dbg == 1 )
         {
         bcdtmWrite_message(0,0,0,"smpType = %2d",smpType) ;
         if( smpPnt1 != catchmentDtmP->nullPnt ) bcdtmWrite_message(0,0,0,"smpPnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",smpPnt1,pointAddrP(catchmentDtmP,smpPnt1)->x,pointAddrP(catchmentDtmP,smpPnt1)->y,pointAddrP(catchmentDtmP,smpPnt1)->z) ;
         if( smpPnt2 != catchmentDtmP->nullPnt ) bcdtmWrite_message(0,0,0,"smpPnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",smpPnt2,pointAddrP(catchmentDtmP,smpPnt2)->x,pointAddrP(catchmentDtmP,smpPnt2)->y,pointAddrP(catchmentDtmP,smpPnt2)->z) ;
         }

     //  Set Sump Point

     sumpPoint.x = sumpX ;
     sumpPoint.y = sumpY ;
     sumpPoint.z = sumpZ ;

     //  From Sump Point

     if( smpType == 1 )
         {
         if( bcdtmDrainage_insertMaximumAscentLinesFromSumpPointDtmObject(catchmentDtmP,smpPnt1)) goto errexit ;
         drainPoint = smpPnt1 ;
         }

     //  From Sump Line

     if( smpType == 2 )
         {
         if( bcdtmDrainage_insertMaximumAscentLinesFromPointOnSumpLineDtmObject(catchmentDtmP,smpPnt1,smpPnt2,sumpPoint,drainPoint)) goto errexit ;
         }

     //  Log Tin

    if( dbg == 1 )
        {
        if( bcdtmWrite_toFileDtmObject(catchmentDtmP,L"ascentCatchment00.tin")) goto errexit ;
        }

    //  Check Drain Point Has Been Determined As All Catchment Flow Must Pass Through The Drain Point

    if( drainPoint == catchmentDtmP->nullPnt )
        {
        bcdtmWrite_message(2,0,0,"Could Not Determine Drain Point For Point Catchment") ;
        goto errexit ;
        }
    drainPointX = pointAddrP(catchmentDtmP,drainPoint)->x ;
    drainPointY = pointAddrP(catchmentDtmP,drainPoint)->y ;
    drainPointZ = pointAddrP(catchmentDtmP,drainPoint)->z ;
    if( dbg == 1 )  bcdtmWrite_message(0,0,0,"drainPoint = %10d ** %12.5lf %12.5lf %10.4lf",drainPoint,drainPointX,drainPointY,drainPointZ) ;

    //  Iteratively Refine Point Catchment Boundary

    refine = true ;
    while( refine )
        {

        // Initialise

        refine = false ;
        if( bcdtmList_cleanDtmObject(catchmentDtmP) ) goto errexit ;

        // Check Triangulation After Clean

        // Find New Point Number For Drain Point

        bcdtmFind_closestPointDtmObject(catchmentDtmP,drainPointX,drainPointY,&drainPoint) ;
        if( dbg == 1 )  bcdtmWrite_message(0,0,0,"drainPoint = %10d ** %12.5lf %12.5lf %10.4lf",drainPoint,pointAddrP(catchmentDtmP,drainPoint)->x,pointAddrP(catchmentDtmP,drainPoint)->y,pointAddrP(catchmentDtmP,drainPoint)->z) ;
        if( cdbg == 1 )
            {
            if( pointAddrP(catchmentDtmP,drainPoint)->x != drainPointX || pointAddrP(catchmentDtmP,drainPoint)->y != drainPointY )
                {
                bcdtmWrite_message(1,0,0,"Drain Point Has Changed") ;
                goto errexit ;
                }
            }

        // Create Triangle Index

        if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Index") ;
        if( triangleIndexP != nullptr )
            triangleIndexP->~DTMTriangleIndex() ;
        startTime = bcdtmClock() ;
        triangleIndexP = new DTMTriangleIndex (catchmentDtmP) ;
        if( dbg  ) bcdtmWrite_message(0,0,0,"Size Of Triangle Index = %8d",triangleIndexP->TriangleIndexSize() ) ;
        if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Calculate Triangle Index = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

        // Log Number Of Void And Flat Triangles

        if( dbg ) triangleIndexP->LogTriangleCounts() ;

        //  Scan Triangle Index And Downstream Trace From Each Triangle Centroid

        numTriangles = 0 ;
        if( dbg ) bcdtmWrite_message(0,0,0,"Down Stream Tracing Tin Streams") ;
        for( DtmTriangleIndex::iterator triangle = triangleIndexP->FirstTriangle() ; triangle <= triangleIndexP->LastTriangle() ; triangle++)
            {

            //  Log Trace Status

            if( dbg && numTriangles % 100000 == 0 ) bcdtmWrite_message(0,0,0,"Number Of Triangles Scanned = %8ld of %8ld",numTriangles,triangleIndexP->TriangleIndexSize()) ;

            //  Set Null Colour For Triangle

            triangle->index = -dtmP->nullPnt ;

            //  Dont Trace For Void Or Zero Slope Triangles

            if( ! triangle->voidTriangle && ! triangle->flatTriangle )
                {

                p1 = triangle->trgPnt1 ;
                p2 = triangle->trgPnt2 ;
                p3 = triangle->trgPnt3 ;

                //   Calculate Trace Points For Triangle

                if( bcdtmDrainage_calculateTracePointsForTriangleDtmObject(catchmentDtmP,p1,p2,p3,tracePoints)) goto errexit ;

                //   Check Trace To Drain Point

                numTracedToSump = 0 ;
                for( int n = 0 ; n < 3 ; ++n )
                    {
                     x = tracePoints[n].x ;
                     y = tracePoints[n].y ;
                     z = tracePoints[n].z ;
                     if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numTriangles = %8ld **** Tracing To Sump From ** %12.5lf %12.5lf %10.4lf",numTriangles,x,y,z) ;
                     if( bcdtmDrainage_checkTraceToDrainPointDtmObject(catchmentDtmP,nullptr,maxPondDepth,traceOverZeroSlope,x,y,z,drainPoint,tracedToSump)) goto errexit ;
                     if( tracedToSump ) ++numTracedToSump ;
                     if( dbg == 2 ) bcdtmWrite_message(0,0,0,"tracedToSump = %d",tracedToSump) ;
                     }
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numTracedToSump = %3d",numTracedToSump) ;
                triangle->index = numTracedToSump ;
                ++numTriangles ;
                }
            }
        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Number Of Triangles = %8ld",numTriangles) ;

        // Polygonise And Load Catchment Polygons

        if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Catchment Polygons") ;
        if( bcdtmObject_createDtmObject(&catchmentPolygonsDtmP)) goto errexit ;
        if( bcdtmDrainage_polygoniseAndLoadTriangleIndexPolygonsDtmObject(catchmentDtmP,(DTMFeatureCallback) bcdtmDrainage_catchmentCallBackFunction,triangleIndexP,catchmentPolygonsDtmP,numCatchments)) goto errexit ;
        if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Catchments = %8ld",numCatchments) ;

        //  Count Number Of Full And Partial Polygons That Flow To Drain Point

        if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
        numZero = numOne = numTwo = numThree = 0 ;
        for( dtmFeature = 0 ; dtmFeature < catchmentPolygonsDtmP->numFeatures ; ++dtmFeature )
             {
             count = ( int ) ftableAddrP(catchmentPolygonsDtmP,dtmFeature)->dtmUserTag ;
             if     ( count == 0 ) ++numZero ;
             else if( count == 1 ) ++numOne ;
             else if( count == 2 ) ++numTwo ;
             else if( count == 3 ) ++numThree ;
             if( count == 0 )
                 {
                 if( bcdtmInsert_removeDtmFeatureFromDtmObject(catchmentPolygonsDtmP,dtmFeature)) goto errexit ;
                 }
             if( count > 0 )
                 {
                 ftableAddrP(catchmentPolygonsDtmP,dtmFeature)->dtmFeatureType = DTMFeatureType::Breakline ;
                 }
             }

        //  Log Polygonisation Results

        if( dbg == 1 )
            {
             bcdtmWrite_message(0,0,0,"numZero = %4ld ** numOne = %4ld numTwo = %4ld numThree = %4ld",numZero,numOne,numTwo,numThree) ;
             bcdtmWrite_geopakDatFileFromDtmObject(catchmentPolygonsDtmP,L"refinedCatchmentPolygons.dat") ;
            }

         //  Check For Termination Of Refinement

        if( numThree == 1 && numOne == 0 && numTwo == 0 )
            {
            for( dtmFeature = 0 ; dtmFeature < catchmentPolygonsDtmP->numFeatures ; ++dtmFeature )
                {
                if( ftableAddrP(catchmentPolygonsDtmP,dtmFeature)->dtmUserTag == 3 )
                    {
                    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(catchmentPolygonsDtmP,dtmFeature,catchmentPtsPP,numCatchmentPtsP)) goto errexit ;
                    }
                }
            }

        //  Continue Refinement

       else
           {
           }



        }

    // Clean Up

 cleanup :
    if( catchPtsP             != NULL ) { free(catchPtsP) ; catchPtsP = NULL ; }
    if( catchDtmP             != NULL ) bcdtmObject_destroyDtmObject(&catchDtmP) ;
    if( catchmentDtmP         != NULL ) bcdtmObject_destroyDtmObject(&catchmentDtmP) ;
    if( catchmentPolygonsDtmP != NULL ) bcdtmObject_destroyDtmObject(&catchmentPolygonsDtmP) ;

    // Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Refining Tptr Catchment Polygon Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Refining Tptr Catchment Polygon Error") ;
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
int bcdtmDrainage_insertMaximumAscentLinesFromPointOnSumpLineDtmObject
    (
    BC_DTM_OBJ *dtmP,                       // ==> Pointer To DTM
    int        point1,                      // ==> Point At One End Of Tin Line
    int        point2,                      // ==> Point At Other End Of Tin Line
    DPoint3d   point,                       // ==> Start Point For Inserting Maximum Ascent Line
    long&      drainPoint                   // <== Point From Where The Maximum Ascents Were Inserted
    )

    // This Function Inserts A Maximum Ascent Line Into The Tin
    // From A Starting Point On A Tin Line

    {
    int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
    long   dtmPnt=dtmP->nullPnt,antPnt,clkPnt,lineType ;
    bool voidPnt;
    double d1,d2 ;

    int fd1,fd2,sideOf ;
    bool newPnt=false ;
    double ascentAngle1,descentAngle1,trgSlope1 ;
    double ascentAngle2,descentAngle2,trgSlope2 ;
    double xr,yr,radius ;

    // Log Arguments

    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Point On Sump Line") ;
        bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"point1    = %8ld",point1) ;
        bcdtmWrite_message(0,0,0,"point2    = %8ld",point2) ;
        bcdtmWrite_message(0,0,0,"point.x   = %12.5lf",point.x) ;
        bcdtmWrite_message(0,0,0,"point.y   = %12.5lf",point.y) ;
        bcdtmWrite_message(0,0,0,"point.z   = %12.5lf",point.z) ;
        }

    // Initialise

    drainPoint = dtmP->nullPnt ;

    //  Test Tin Line Exists

    if( ! bcdtmList_testLineDtmObject(dtmP,point1,point2))
        {
        if( dbg )
            {
            bcdtmList_writeCircularListForPointDtmObject(dtmP,point1) ;
            bcdtmList_writeCircularListForPointDtmObject(dtmP,point2) ;
            }
        bcdtmWrite_message(2,0,0,"Not A Tin Line") ;
        goto errexit ;
        }

    //  Get Triangles Either Side Of Sump Line

    if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,point1,point2)) < 0 ) goto errexit ;
    if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point1,point2)) < 0 ) goto errexit ;
    if( ! bcdtmList_testLineDtmObject(dtmP,antPnt,point2)) antPnt = dtmP->nullPnt ;
    if( ! bcdtmList_testLineDtmObject(dtmP,clkPnt,point2)) clkPnt = dtmP->nullPnt ;
    if( antPnt == dtmP->nullPnt || clkPnt == dtmP->nullPnt )
        {
         bcdtmWrite_message(1,0,0,"Not A Sump Line") ;
         goto errexit ;
        }

    //  Check For Sump Line

    if( cdbg == 1 )
        {
        fd1 = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,point1,point2,antPnt) ;
        fd2 = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,point1,point2,clkPnt) ;
        if( fd1 < 1 || fd2 < 1 )
            {
            bcdtmWrite_message(1,0,0,"Not A Sump Line") ;
            goto errexit ;
            }
        }

    //  Check If Point Is Within Point To Point Tolerance Of The Line End Points

    d1 = bcdtmMath_distance(point.x,point.y,pointAddrP(dtmP,point1)->x,pointAddrP(dtmP,point1)->y) ;
    d2 = bcdtmMath_distance(point.x,point.y,pointAddrP(dtmP,point2)->x,pointAddrP(dtmP,point2)->y) ;
    if      ( d1 <= d2 && d1 < dtmP->ppTol ) dtmPnt = point1 ;
    else if ( d2 < dtmP->ppTol ) dtmPnt = point2 ;

    // Only Insert Point Into Line If It Is Not Coincident With One Of The Line End Points

    if( dtmPnt == dtmP->nullPnt )
        {

        //  Determine Line Type ( Internal = 1 , On Tin Hull = 2 )

        if( nodeAddrP(dtmP,point1)->hPtr == dtmP->nullPnt ||  nodeAddrP(dtmP,point2)->hPtr == dtmP->nullPnt )
            {
            lineType = 1 ;
            }
        else
            {
            lineType = 2 ;
            if( nodeAddrP(dtmP,point1)->hPtr != point2 )
                {
                dtmPnt = point1 ;
                point1 = point2 ;
                point2 = dtmPnt ;
                }
            }

        //  Check For Void Line

        if( bcdtmList_testForVoidLineDtmObject(dtmP,point1,point2,voidPnt)) goto errexit ;

        // Add Point To Tin

        newPnt = true ;
        if( bcdtmInsert_addPointToDtmObject(dtmP,point.x,point.y,point.z,&dtmPnt) ) goto errexit  ;

        //  Insert Point Into Line

        switch ( lineType )
            {

            case  1 :      /* Coincident With Internal Tin Line  */

                bcdtmList_testForVoidLineDtmObject(dtmP,point1,point2,voidPnt) ;
                if(bcdtmList_deleteLineDtmObject(dtmP,point1,point2)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point1,dtmPnt,antPnt)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,point1,dtmP->nullPnt)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point2,dtmPnt,clkPnt)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,point2,point1)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,dtmPnt,point2)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,antPnt,point1)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,clkPnt,dtmPnt,point1)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,clkPnt,point2)) goto errexit ;
                if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,point1,point2) )
                    {
                    if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,point1,point2,dtmPnt)) goto errexit ;
                    }
                break ;

            case  2 :      /* Coincident With External Tin Line  */
                bcdtmList_testForVoidLineDtmObject(dtmP,point1,point2,voidPnt) ;
                if( (antPnt = bcdtmList_nextAntDtmObject(dtmP,point1,point2))   < 0 ) goto errexit ;
                if(bcdtmList_deleteLineDtmObject(dtmP,point1,point2)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point1,dtmPnt,antPnt)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,point1,dtmP->nullPnt)) goto errexit ;
                if(bcdtmList_insertLineBeforePointDtmObject(dtmP,point2,dtmPnt,antPnt)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,point2,point1)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,dtmPnt,point2)) goto errexit ;
                if(bcdtmList_insertLineAfterPointDtmObject(dtmP,dtmPnt,antPnt,point1)) goto errexit ;
                if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,point1,point2) )
                    {
                    if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,point1,point2,dtmPnt)) goto errexit ;
                    }
                nodeAddrP(dtmP,point1)->hPtr = dtmPnt ;
                nodeAddrP(dtmP,dtmPnt)->hPtr = point2 ;
                break ;

            default :
               bcdtmWrite_message(1,0,0,"Unknown Line Type") ;
               goto errexit ;
               break ;

            }

        //  Check Tin After Inserting Point

        if( cdbg )
            {
            if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin After Inserting Point Into Line") ;
            if( bcdtmCheck_tinComponentDtmObject(dtmP))
                {
                if( dbg ) bcdtmWrite_message(0,0,0,"Tin Invalid") ;
                goto errexit ;
                }
            if( dbg ) bcdtmWrite_message(0,0,0,"Tin Valid") ;
            }
       }

   //  Insert Maximum Ascent Line From Tin Point

   if( bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,point1,point2,antPnt,&descentAngle1,&ascentAngle1,&trgSlope1)) goto errexit ;
   if( bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,point1,point2,clkPnt,&descentAngle2,&ascentAngle2,&trgSlope2)) goto errexit ;
   if( newPnt )
       {

       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascents From Sump Line") ;

       radius = sqrt(dtmP->xRange*dtmP->xRange+dtmP->yRange*dtmP->yRange) ;

       //  Insert Maximum Ascent To Left Of Sump Line

       xr = pointAddrP(dtmP,dtmPnt)->x + radius*cos(ascentAngle1) ;
       yr = pointAddrP(dtmP,dtmPnt)->y + radius*sin(ascentAngle1) ;
       sideOf = bcdtmMath_sideOf(pointAddrP(dtmP,dtmPnt)->x,pointAddrP(dtmP,dtmPnt)->y,pointAddrP(dtmP,antPnt)->x,pointAddrP(dtmP,antPnt)->y,xr,yr) ;
       if     ( sideOf < 0 )
           {
           if( bcdtmDrainage_insertMaximumAscentLineFromTriangleBasePointDtmObject(dtmP,dtmPnt,antPnt,point2)) goto errexit ;
           }
       else if( sideOf == 0 )
           {
           if( bcdtmDrainage_insertMaximumAscentLineFromTinPointDtmObject(dtmP,antPnt)) goto errexit ;
           }
       else if( sideOf > 0 )
           {
           if( bcdtmDrainage_insertMaximumAscentLineFromTriangleBasePointDtmObject(dtmP,dtmPnt,point1,antPnt)) goto errexit ;
           }

       //  Insert Maximum Ascent To Right Of Sump Line

       xr = pointAddrP(dtmP,dtmPnt)->x + radius*cos(ascentAngle2) ;
       yr = pointAddrP(dtmP,dtmPnt)->y + radius*sin(ascentAngle2) ;
       sideOf = bcdtmMath_sideOf(pointAddrP(dtmP,dtmPnt)->x,pointAddrP(dtmP,dtmPnt)->y,pointAddrP(dtmP,clkPnt)->x,pointAddrP(dtmP,clkPnt)->y,xr,yr) ;
       if     ( sideOf < 0 )
           {
           if( bcdtmDrainage_insertMaximumAscentLineFromTriangleBasePointDtmObject(dtmP,dtmPnt,clkPnt,point1)) goto errexit ;
           }
       else if( sideOf == 0 )
           {
           if( bcdtmDrainage_insertMaximumAscentLineFromTinPointDtmObject(dtmP,clkPnt)) goto errexit ;
           }
       else if( sideOf > 0 )
           {
           if( bcdtmDrainage_insertMaximumAscentLineFromTriangleBasePointDtmObject(dtmP,dtmPnt,point2,clkPnt)) goto errexit ;
           }
       }

   // Insert Maximum Ascents From Sump Point

   else
       {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascents From Sump Point") ;
       if( bcdtmDrainage_insertMaximumAscentLinesFromSumpPointDtmObject(dtmP,dtmPnt)) goto errexit ;
       }

   //  Set Drain Point

   drainPoint = dtmPnt ;

   // Check Triangulation After Inserting Maximum Ascent Lines

   if( cdbg == 1 )
       {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Triangulation After Inserting Maximum Ascent Lines From Sump Line") ;
       if( bcdtmCheck_tinComponentDtmObject(dtmP))
           {
           bcdtmWrite_message(1,0,0,"Triangulation Invalid") ;
           goto errexit ;
           }
        if( dbg ) bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
        }

// Clean Up

 cleanup :

// Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Point On Sump Line Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Point On Sump Point Error") ;
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
int bcdtmDrainage_insertMaximumAscentLinesFromSumpPointDtmObject
    (
    BC_DTM_OBJ *dtmP,                       // Pointer To DTM
    int        sumpPoint                    // Sump Point
    )

    // This Function Inserts Maximum Maximum Ascent Lines For A Sump Point Into The Tin

    {
    int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
    int    clPnt,clPtr,sPnt,nPnt,pPnt ;
    long   startPnt ;
    bool   lowPoint=true ;

    // Log Arguments

    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Sump Point") ;
        bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"sumpPoint = %8ld",sumpPoint) ;
        }

    //  Check Sum Point Validity

    if( cdbg == 1 )
        {
         if( sumpPoint < 0 || sumpPoint >= dtmP->numPoints )
             {
             bcdtmWrite_message(2,0,0,"Sump Point Range Error") ;
             goto errexit ;
             }

         if(( clPtr = nodeAddrP(dtmP,sumpPoint)->cPtr ) == dtmP->nullPtr )
             {
             bcdtmWrite_message(2,0,0,"Sump Point Not A Tin Point") ;
             goto errexit ;
             }
         while( clPtr != dtmP->nullPtr && lowPoint )
             {
             clPnt = clistAddrP(dtmP,clPtr)->pntNum ;
             clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
             if( pointAddrP(dtmP,clPnt)->z <= pointAddrP(dtmP,sumpPoint)->z )
                 lowPoint = false ;
             }
         if( ! lowPoint )
             {
             bcdtmWrite_message(2,0,0,"Sump Point Not A Low Point") ;
             goto errexit ;
             }
        }

    //  Place Tptr Polygon Around Sump Point

    if( bcdtmList_insertTptrPolygonAroundPointDtmObject(dtmP,sumpPoint,&startPnt)) goto errexit ;

    //  Scan Around Tptr Polygon And Fire Off And Insert Maximum Ascents

    pPnt = startPnt ;                      // Prior Tptr Polygon Point
    sPnt = nodeAddrP(dtmP,pPnt)->tPtr ;    // Scan Point
    do
        {
        nPnt = nodeAddrP(dtmP,sPnt)->tPtr ; // Next Tptr Polygon Point

        //  Insert Maximum Ascent Line

        if( bcdtmDrainage_insertMaximumAscentLineBetweenPointsDtmObject(dtmP,sPnt,nPnt,pPnt)) goto errexit  ;

        //  Reset Prior And Scan Point

        pPnt = sPnt ;
        sPnt = nPnt ;
        } while( sPnt != nodeAddrP(dtmP,startPnt)->tPtr ) ;

    // Check Triangulation After Inserting Maximum Ascent Lines

    if( cdbg == 1 )
        {
        if( dbg ) bcdtmWrite_message(0,0,0,"Checking Triangulation After Inserting Maximum Ascent Lines From Sump Point") ;
        if( bcdtmCheck_tinComponentDtmObject(dtmP))
            {
            bcdtmWrite_message(1,0,0,"Triangulation Invalid") ;
            goto errexit ;
            }
        if( dbg ) bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
        }

// Clean Up

 cleanup :

// Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Sump Point Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Maximum Ascent Lines From Sump Point Error") ;
    return(ret) ;

// Error Exit

 errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }




