/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageTrace.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
int bcdtmDrainage_scanPointForMaximumDescentDtmObject
(
 BC_DTM_OBJ        *dtmP,                    // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP,         // ==> Pointer To Drainage Tables
 long              point,                    // ==> Point To Scan About
 long              excludePoint,             // ==> Exclude Point
 long              *descentTypeP,            // <== Descent Type < 1 Sump, 2 Triangle>
 long              *descentPnt1P,            // <== Pnt1 Of Descent Feature
 long              *descentPnt2P,            // <== Pnt2 Of Descent Feature
 double            *descentSlopeP,           // <== Descent Slope
 double            *descentAngleP            // <== Descent Angle
)
/*
** This Function Scans A Point For Maximum Descent
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sumpPnt,trgPnt1,trgPnt2 ;
 double sumpSlope,trgSlope,sumpDescentAngle=0.0,trgDescentAngle=0.0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
	bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent") ;
	bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
	bcdtmWrite_message(0,0,0,"drainageTablesP = %8ld",drainageTablesP) ;
	bcdtmWrite_message(0,0,0,"point           = %8ld",point) ;
	bcdtmWrite_message(0,0,0,"excludePoint    = %8ld",excludePoint) ;
   }
/*
** Initialise
*/
 *descentTypeP  = 0 ;
 *descentSlopeP = 0.0 ;
 *descentAngleP = 0.0 ;
 *descentPnt1P  = dtmP->nullPnt ;
 *descentPnt2P  = dtmP->nullPnt ;
/*
** Validate Point
*/
 if( point < 0 || point >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Point Range Error") ;
    goto errexit ;
   }

// Log Point Coordinates

 if( dbg ) bcdtmWrite_message(0,0,0,"Point[%8ld] = %12.5lf %12.5lf %10.4lf",point,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z) ;

/*
**  Get Maximum Descent Sump Line
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Sump Line") ;
 if( bcdtmDrainage_scanPointForMaximumDescentSumpLineDtmObject(dtmP,drainageTablesP,point,excludePoint,&sumpPnt,&sumpDescentAngle,&sumpSlope) ) goto errexit ;
 if(  dbg )
     {
     bcdtmWrite_message(0,0,0,"sumpPnt = %10ld",sumpPnt) ;
     if( sumpPnt != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"sumpPnt = %8ld sumpPnt->z = %12.5lf sumpSlope = %8.5lf sumpDescentAngle = %12.10lf",sumpPnt,pointAddrP(dtmP,sumpPnt)->z,sumpSlope,sumpDescentAngle) ;
     }
/*
**  Get Maximum Descent Triangle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Triangle") ;
 if( bcdtmDrainage_scanPointForMaximumDescentTriangleDtmObject(dtmP,drainageTablesP,point,excludePoint,&trgPnt1,&trgPnt2,&trgDescentAngle,&trgSlope) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"trgPnt1 = %8ld trgPnt2 = %8ld trgSlope = %8.5lf trgDescentAngle = %12.10lf",trgPnt1,trgPnt2,trgSlope,trgDescentAngle) ;
/*
** Maximum Descent Down A Sump Line
*/
 if( sumpPnt != dtmP->nullPnt && trgPnt1 == dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Descent Down A Sump Line") ;
    *descentTypeP  = 1 ;
    *descentPnt1P  = sumpPnt ;
    *descentSlopeP = sumpSlope ;
    *descentAngleP = sumpDescentAngle ;
   }
/*
** Maximum Descent Down A Triangle
*/
 if( sumpPnt == dtmP->nullPnt && trgPnt1 != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Descent Down A Triangle Face") ;
    *descentTypeP  = 2 ;
    *descentPnt1P  = trgPnt1 ;
    *descentPnt2P  = trgPnt2 ;
    *descentSlopeP = trgSlope ;
    *descentAngleP = trgDescentAngle ;
   }
/*
** Maximum Descents Down A Sump Lines And Triangle
** If Both Sump And Triangle Slopes Are The Same Value Set The Maximum
** Descent To A Triangle
*/
  if( sumpPnt != dtmP->nullPnt && trgPnt1 != dtmP->nullPnt )
   {
    if( trgSlope <= sumpSlope )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Descent Down A Triangle Face") ;
       *descentTypeP  = 2 ;
       *descentPnt1P  = trgPnt1 ;
       *descentPnt2P  = trgPnt2 ;
       *descentSlopeP = trgSlope ;
       *descentAngleP = trgDescentAngle ;
      }
    else
      {
       *descentTypeP  = 1 ;
       *descentPnt1P  = sumpPnt ;
       *descentSlopeP = sumpSlope ;
       *descentAngleP = sumpDescentAngle ;
     }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Error") ;
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
int bcdtmDrainage_scanPointForMaximumDescentSumpLineDtmObject
(
 BC_DTM_OBJ        *dtmP,               // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP,    // ==> Pointer To Drainage Tables
 long              point,               // ==> Point To Scan About
 long              excludePoint,        // ==> Exclude Point
 long              *sumpPointP,         // <== Sump Line End Point
 double            *sumpAngleP,         // <== Sump Line Angle
 double            *sumpSlopeP          // <== Sump Line Slope
)
/*
** This Function Scans A Point For The Maximum Descent Sump Line
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   antPnt, scanPnt, clkPnt, clPtr;
 DTMFeatureType lineType;
 double dx,dy,dz,dd,slope ;
/*
** Write Entry Message
*/
 if( dbg )
   {
	bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Sump Line") ;
	bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
	bcdtmWrite_message(0,0,0,"drainageTablesP = %8ld",drainageTablesP) ;
	bcdtmWrite_message(0,0,0,"point           = %8ld",point) ;
	bcdtmWrite_message(0,0,0,"excludePoint    = %8ld",excludePoint) ;
   }
/*
** Initialise
*/
 *sumpPointP = dtmP->nullPnt ;
 *sumpSlopeP = 0.0 ;        ;
/*
** Scan Around Point For Sump Lines
*/
 if( ( clPtr = nodeAddrP(dtmP,point)->cPtr) != dtmP->nullPtr )
   {
    scanPnt = clistAddrP(dtmP,clPtr)->pntNum ;
    if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,point,scanPnt)) < 0 ) goto errexit ;
    while ( clPtr != dtmP->nullPtr )
      {
       scanPnt  = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr    = clistAddrP(dtmP,clPtr)->nextPtr ;
       if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point,scanPnt)) < 0 ) goto errexit ;
       if(  scanPnt != excludePoint )
         {
          if( pointAddrP(dtmP,point)->z >= pointAddrP(dtmP,scanPnt)->z )
            {
             if( bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP,drainageTablesP,point,scanPnt,antPnt,clkPnt,&lineType)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"lineType = %10ld ** point = %8ld scanPnt = %8ld ** antPnt = %8ld clkPnt = %8ld",lineType,point,scanPnt,antPnt,clkPnt) ;
             if( lineType == DTMFeatureType::SumpLine )
               {
                dx = pointAddrP(dtmP,point)->x - pointAddrP(dtmP,scanPnt)->x ;
                dy = pointAddrP(dtmP,point)->y - pointAddrP(dtmP,scanPnt)->y ;
                dz = pointAddrP(dtmP,point)->z - pointAddrP(dtmP,scanPnt)->z ;
                dd = sqrt(dx*dx + dy*dy) ;
                slope = -dz/dd ;
                if( *sumpPointP == dtmP->nullPnt || slope < *sumpSlopeP )
                  {
                   *sumpPointP = scanPnt ;
                   *sumpSlopeP = slope   ;
				   *sumpAngleP = bcdtmMath_getPointAngleDtmObject(dtmP,point,scanPnt) ;
                  }
               }
            }
         }
       antPnt = scanPnt ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Sump Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Sump Line Error") ;
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
int  bcdtmDrainage_scanPointForMaximumDescentTriangleDtmObject
(
 BC_DTM_OBJ *dtmP,                   /* ==> Pointer To Tin Object                */
 DTMDrainageTables *drainageTablesP, /* ==> Pointer To Drainage Tables           */
 long point,                         /* ==> Point To Scan About                  */
 long excludePoint,                  /* ==> exclude Point                        */
 long *trgBasePnt1P,                 /* <== Triangle Base Point 1                */
 long *trgBasePnt2P,                 /* <== Triangle Base Point 2                */
 double *trgDescentAngleP ,          /* <== Maximum Descent Angle                */
 double *trgSlopeP                   /* <== Maximum Descent Angle                */
)
/*
** This Function Scans The Triangles About A Point Looking For The Maximum Descent Triangle
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   clPtr,antPnt,clkPnt ;
 double a1,a2,a3,angleAntPnt,angleClkPnt,slope,ascentAngle,descentAngle  ;
 bool   voidTriangle=false ;
/*
** Write Entry Message
*/
 if( dbg )
   {
	bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Triangle") ;
	bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
	bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP) ;
	bcdtmWrite_message(0,0,0,"point           = %8ld",point) ;
	bcdtmWrite_message(0,0,0,"excludePoint    = %8ld",excludePoint) ;
   }
/*
** Initialise
*/
 *trgBasePnt1P = dtmP->nullPnt ;
 *trgBasePnt2P = dtmP->nullPnt ;
 *trgDescentAngleP = 0.0 ;
 *trgSlopeP        = 0.0 ;
/*
** Scan Around point
*/
 clPtr = nodeAddrP(dtmP,point)->cPtr;
 if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit  ;
 angleAntPnt = bcdtmMath_getPointAngleDtmObject(dtmP,point,antPnt) ;
 while ( clPtr != dtmP->nullPtr )
   {
    clkPnt = clistAddrP(dtmP,clPtr)->pntNum ;
    clPtr  = clistAddrP(dtmP,clPtr)->nextPtr ;
    angleClkPnt = bcdtmMath_getPointAngleDtmObject(dtmP,point,clkPnt) ;
    if(  nodeAddrP(dtmP,point)->hPtr != antPnt && antPnt != excludePoint && clkPnt != excludePoint )
      {

//     Get Triangle Slope And Descent Angle

       if( bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP,drainageTablesP,point,antPnt,clkPnt,voidTriangle,slope,descentAngle,ascentAngle) != DTM_SUCCESS ) goto errexit ;

//     Only Process For None Void Triangles

       if( voidTriangle == false )
         {
/*
**        Determine If Descent Angle Intersects Triangle Base
*/
          a1 = angleAntPnt ;
          a2 = descentAngle ;
          a3 = angleClkPnt ;
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"**** antPnt = %8ld clkPnt = %8ld",antPnt,clkPnt) ;
             bcdtmWrite_message(0,0,0,"antAngle = %12.10lf desAngle = %12.10lf clkAngle = %12.10lf",a1,a2,a3) ;
             bcdtmWrite_message(0,0,0,"descentAngle = %12.8lf",descentAngle) ;
             bcdtmWrite_message(0,0,0,"descentSlope = %12.8lf",slope) ;
             bcdtmWrite_message(0,0,0,"angleAntPnt  = %12.8lf",angleAntPnt) ;
             bcdtmWrite_message(0,0,0,"angleClkPnt  = %12.8lf",angleClkPnt) ;
            }
          if( a1 < a3 ) a1 = a1 + DTM_2PYE ;
          if( a2 < a3 ) a2 = a2 + DTM_2PYE ;
          if( dbg )   bcdtmWrite_message(0,0,0,"** antAngle = %12.10lf desAngle = %12.10lf clkAngle = %12.10lf",a1,a2,a3) ;

          if( slope == 0.0 || ( a2 <= a1 && a2 >= a3 ))
            {
			 if( slope != 0.0 ) slope = -slope ;
             if( *trgBasePnt1P == dtmP->nullPnt || slope < *trgSlopeP )
               {
                *trgBasePnt1P = antPnt ;
                *trgBasePnt2P = clkPnt ;
                *trgDescentAngleP = descentAngle ;
                *trgSlopeP = slope ;
               }
            }
         }
      }
/*
**  Reset For Next Triangle
*/
    antPnt = clkPnt ;
    angleAntPnt = angleClkPnt ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Triangle Error") ;
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
int bcdtmDrainage_traceMaximumDescentDtmObject
(
 BC_DTM_OBJ         *dtmP,                  // ==> Pointer To Tin Object
 DTMDrainageTables  *drainageTablesP,       // ==> Pointer To Drainage Tables
 DTMFeatureCallback loadFunctionP,          // ==> Pointer To Load Function
 double             falseLowDepth,          // ==> False Low Depth
 double             startX,                 // ==> Start X Coordinate
 double             startY,                 // ==> Start Y Coordinate
 void               *userP                  // ==> User Pointer Passed Back To User
)
/*
** This Function Traces The Maximum Descent From Point(startX,startY)
** To Inhibit False Low Processing Set falseLowDepth to zero
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 int    zeroSlopeOption=1 ;                               // Trace At Last Angle Over Zero Slope Triangles
 long   trgPnt1=0,trgPnt2=0,trgPnt3=0,lowPnt1=0,lowPnt2=0,pointType=0,pntInVoid=0 ;
 double z ;

// Log Entry Arguments

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Maximum Descent") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld",dtmP->numPoints)  ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP)  ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP)  ;
    bcdtmWrite_message(0,0,0,"falseLowDepth   = %10.4lf",falseLowDepth) ;
    bcdtmWrite_message(0,0,0,"startX          = %12.5lf",startX)  ;
    bcdtmWrite_message(0,0,0,"startY          = %12.5lf",startY)  ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP)  ;
   }

// Initialise

 if( falseLowDepth < 0.0 ) falseLowDepth = 0.0 ;

// Test For Valid DTM Object

 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

// Check DTM Is Triangulated

 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Find Triangle Containing Start Point On TIN
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,startX,startY,&z,&pointType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"pointType = %2ld ** trgPnt1 = %10ld trgPnt2 = %10ld trgPnt3 = %10ld",pointType,trgPnt1,trgPnt2,trgPnt3) ;
 if( pointType == 0 )
   {
    bcdtmWrite_message(1,0,0,"Maximum Descent Start Point External To Tin") ;
    goto errexit ;
   }
/*
** Check Point To Point Tolerance
*/
 if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) <= dtmP->ppTol ) { pointType = 1 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
 if( trgPnt2 != dtmP->nullPnt ) if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) <= dtmP->ppTol ) { pointType = 1 ; trgPnt1 = trgPnt2 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
 if( trgPnt3 != dtmP->nullPnt ) if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y) <= dtmP->ppTol ) { pointType = 1 ; trgPnt1 = trgPnt3 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
/*
** Test For Point In Void
*/
 pntInVoid = 0 ;
 if( pointType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,trgPnt1)->PCWD) ) pntInVoid = 1 ;
 if( pointType == 2 || pointType == 3 )  if( bcdtmList_testForVoidLineDtmObject(dtmP,trgPnt1,trgPnt2,&pntInVoid)) goto errexit ;
 if( pointType == 4 ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,&pntInVoid)) goto errexit ;
 if( pntInVoid )
   {
    bcdtmWrite_message(1,0,0,"Maximum Descent Start Point In Void") ;
    goto errexit ;
   }
/*
** Test For Zero Slope Triangle
*/
 if( trgPnt2 != dtmP->nullPnt && trgPnt3 != dtmP->nullPnt )
   {
    if( pointAddrP(dtmP,trgPnt1)->z == pointAddrP(dtmP,trgPnt2)->z && pointAddrP(dtmP,trgPnt1)->z == pointAddrP(dtmP,trgPnt3)->z )
      {
       bcdtmWrite_message(1,0,0,"Maximum Descent Start Point On Zero Slope Triangle") ;
       goto errexit ;
      }
   }
 if( trgPnt3 != dtmP->nullPnt ) if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y) <= dtmP->ppTol ) { trgPnt1 = trgPnt3 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
/*
** Set Triangle Anti Clockwise
*/
 if      ( trgPnt2 != dtmP->nullPnt && trgPnt3 != dtmP->nullPnt )
   {
    if( bcdtmMath_pointSideOfDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3) < 0 ) { pointType = trgPnt2 ; trgPnt2 = trgPnt3 ; trgPnt3 = pointType ; }
   }
 else if ( trgPnt2 != dtmP->nullPnt && trgPnt3 == dtmP->nullPnt )
   {
    if(( pointType = bcdtmList_nextAntDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ;
    if( ! bcdtmList_testLineDtmObject(dtmP,pointType,trgPnt2))
      {
       if(( pointType = bcdtmList_nextClkDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ;
       if( ! bcdtmList_testLineDtmObject(dtmP,pointType,trgPnt2) ) goto errexit ;
       trgPnt3 = pointType ; pointType = trgPnt1 ; trgPnt1 = trgPnt2 ; trgPnt2 = pointType ;
      }
   }
/*
** Start Tracing
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Commencing Maximum Descent Trace To Low Point") ;
 if( bcdtmDrainage_traceToLowPointDtmObject(dtmP,drainageTablesP,loadFunctionP,falseLowDepth,zeroSlopeOption,true,trgPnt1,trgPnt2,trgPnt3,startX,startY,z,userP,&lowPnt1,&lowPnt2) ) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Drains To %9ld %9ld",lowPnt1,lowPnt2) ;
    if( lowPnt1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"lowPnt1 = %6ld lowPnt1->FPTR = %9ld ** %10.4lf %10.4lf %10.4lf",lowPnt1,nodeAddrP(dtmP,lowPnt1)->hPtr,pointAddrP(dtmP,lowPnt1)->x,pointAddrP(dtmP,lowPnt1)->y,pointAddrP(dtmP,lowPnt1)->z ) ;
    if( lowPnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"lowPnt2 = %6ld lowPnt2->FPTR = %9ld ** %10.4lf %10.4lf %10.4lf",lowPnt2,nodeAddrP(dtmP,lowPnt2)->hPtr,pointAddrP(dtmP,lowPnt2)->x,pointAddrP(dtmP,lowPnt2)->y,pointAddrP(dtmP,lowPnt2)->z ) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Descent Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Descent Error") ;
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
int bcdtmDrainage_startTraceDtmObject
(
 BC_DTM_OBJ *dtmP,                   /* ==> Pointer To DTM Object             */
 long   startTraceType,              /* ==> Start Type <1 Ascent,2 Descent>   */
 long   pnt1,                        /* ==> Triangle Point 1                  */
 long   pnt2,                        /* ==> Triangle Point 2                  */
 long   pnt3,                        /* ==> Triangle Point 3                  */
 double x,                           /* ==> Start Trace Point x Coordinate    */
 double y,                           /* ==> Start Trace Point y Coordinate    */
 double z,                           /* ==> Start Trace Point z Coordinate    */
 long   *startPointTypeP,            /* <== Start Point Type                  */
 long   *nextPnt1P,                  /* <== Next Triangle Point 1             */
 long   *nextPnt2P,                  /* <== Next Triangle Point 2             */
 long   *nextPnt3P,                  /* <== Next Triangle Point 3             */
 double *nextXP,                     /* <== Next Trace Point x Coordinate     */
 double *nextYP,                     /* <== Next Trace Point y Coordinate     */
 double *nextZP,                     /* <== Next Trace Point z Coordinate     */
 double *startTraceAngleP            /* <== Start Trace Angle                 */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   antPnt,clkPnt,antFlow,clkFlow,voidTriangle ;
 double slope,antSlope,clkSlope,descentAngle,ascentAngle ;
/*
** Write Entry Message
*/
dbg=DrainageDebug ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Drainage Trace Start") ;
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"startTraceType     = %8ld",startTraceType) ;
    bcdtmWrite_message(0,0,0,"pnt1               = %8ld",pnt1) ;
    bcdtmWrite_message(0,0,0,"pnt2               = %8ld",pnt2) ;
    bcdtmWrite_message(0,0,0,"pnt3               = %8ld",pnt3) ;
    bcdtmWrite_message(0,0,0,"x                  = %12.5lf",x)  ;
    bcdtmWrite_message(0,0,0,"y                  = %12.5lf",y)  ;
    bcdtmWrite_message(0,0,0,"z                  = %12.5lf",z)  ;
   }

/*
** Initialise
*/
 *nextXP = *nextYP = *nextZP = 0.0 ;
 *startPointTypeP = *nextPnt1P = *nextPnt2P = *nextPnt3P = dtmP->nullPnt ;
/*
** Determine Start Point Type
*/
 if     ( pnt1 != dtmP->nullPnt && pnt2 == dtmP->nullPnt && pnt3 == dtmP->nullPnt ) *startPointTypeP = 1 ;
 else if( pnt1 != dtmP->nullPnt && pnt2 != dtmP->nullPnt && pnt3 == dtmP->nullPnt ) *startPointTypeP = 2 ;
 else if( pnt1 != dtmP->nullPnt && pnt2 != dtmP->nullPnt && pnt3 != dtmP->nullPnt ) *startPointTypeP = 3 ;
/*
** Start Point Coincident With A Triangle Point
*/
 if( *startPointTypeP == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Start Point Coincident With Triangle Point") ;
    *nextPnt1P = pnt1 ;
    *nextXP = x ;
    *nextYP = y ;
    *nextZP = z ;
   }
/*
** Start Point Coincident With Triangle Edge
*/
 if( *startPointTypeP == 2 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Start Point Coincident With Triangle Edge") ;
    if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
    if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
    if( ! bcdtmList_testLineDtmObject(dtmP,pnt2,antPnt)) antPnt = dtmP->nullPnt ;
    if( ! bcdtmList_testLineDtmObject(dtmP,pnt2,clkPnt)) clkPnt = dtmP->nullPnt ;
    if( antPnt != dtmP->nullPnt )
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,antPnt,pnt2,&voidTriangle)) goto errexit ;
       if( voidTriangle) antPnt = dtmP->nullPnt ;
      }
    if( clkPnt != dtmP->nullPnt )
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,clkPnt,&voidTriangle)) goto errexit ;
       if( voidTriangle) clkPnt = dtmP->nullPnt ;
      }
    pnt3 = antFlow = clkFlow = dtmP->nullPnt ;
    if( antPnt != dtmP->nullPnt ) antFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,pnt1,pnt2,antPnt) ;
    if( clkPnt != dtmP->nullPnt ) clkFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,pnt1,pnt2,clkPnt) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"antFlow = %2ld clkFlow = %2ld",antFlow,clkFlow) ;

    if( startTraceType == 1 )    // Ascent
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Ascent Start Point From Triangle Edge") ;
       if     ( antFlow == 1 && clkFlow != 1 ) pnt3 = antPnt  ;
       else if( antFlow != 1 && clkFlow == 1 ) pnt3 = clkPnt  ;
       else if( antFlow == 1 && clkFlow == 1 )
         {
          bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,antPnt,&descentAngle,&ascentAngle,&antSlope) ;
          bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,clkPnt,&descentAngle,&ascentAngle,&clkSlope) ;
          if( antSlope >= clkSlope ) pnt3 = antPnt ;
          else                       pnt3 = clkPnt ;
         }
      }

    if( startTraceType == 2 )    // Descent
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Descent Start Point From Triangle Edge") ;
       if     ( antFlow == 1 && clkFlow == 1 )
           {
           *startPointTypeP = 1 ;
           if( pointAddrP(dtmP,pnt1)->z <=  pointAddrP(dtmP,pnt2)->z )
               {
               *nextPnt1P = pnt1 ;
               *nextPnt2P = dtmP->nullPnt ;
               *nextPnt3P = dtmP->nullPnt ;
               *nextXP = pointAddrP(dtmP,pnt1)->x ;
               *nextYP = pointAddrP(dtmP,pnt1)->y ;
               *nextZP = pointAddrP(dtmP,pnt1)->z ;
               }
           else
               {
               *nextPnt1P = pnt2 ;
               *nextPnt2P = dtmP->nullPnt ;
               *nextPnt3P = dtmP->nullPnt ;
               *nextXP = pointAddrP(dtmP,pnt2)->x ;
               *nextYP = pointAddrP(dtmP,pnt2)->y ;
               *nextZP = pointAddrP(dtmP,pnt2)->z ;
               }
            goto cleanup ;
           }
       else if( antFlow == -1 && clkFlow != -1 ) pnt3 = antPnt  ;
       else if( antFlow != -1 && clkFlow == -1 ) pnt3 = clkPnt  ;
       else if( antFlow == -1 && clkFlow == -1 )
         {
          bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,antPnt,&descentAngle,&ascentAngle,&antSlope) ;
          bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,clkPnt,&descentAngle,&ascentAngle,&clkSlope) ;
          if( antSlope >= clkSlope ) pnt3 = antPnt ;
          else                       pnt3 = clkPnt ;
         }
      }

      //  Get Start Triangle In CCW Direction

     if( pnt3 == dtmP->nullPnt ) *startPointTypeP = dtmP->nullPnt ;
     else
        {
         if( bcdtmMath_pointSideOfDtmObject(dtmP,pnt1,pnt2,pnt3) > 0 )
             {
             *nextPnt1P = pnt1 ;
             *nextPnt2P = pnt2 ;
             *nextPnt3P = pnt3 ;
             }
         else
             {
             *nextPnt1P = pnt2 ;
             *nextPnt2P = pnt1 ;
             *nextPnt3P = pnt3 ;
             }

        //  Get Intersect With Opposite Triangle Edge

        if( bcdtmDrainage_getFirstTracePointFromTriangleEdgeDtmObject(dtmP,startTraceType,pnt1,pnt2,pnt3,x,y,nextPnt1P,nextPnt2P,nextPnt3P,nextXP,nextYP,nextZP)) goto errexit ;
       }
   }
/*
** Start Point Internal To Triangle
*/
 if( *startPointTypeP == 3 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Start Point Internal To Triangle") ;
    bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,pnt3,&descentAngle,&ascentAngle,&slope) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Slope = %10.5lf ** descentAngle = %12.10lf ascentAngle = %12.10lf",slope,descentAngle,ascentAngle) ;
    if     ( startTraceType == 1 ) *startTraceAngleP = ascentAngle ;
    else if( startTraceType == 2 ) *startTraceAngleP = descentAngle ;
    if( bcdtmDrainage_getFirstTracePointFromTriangleDtmObject(dtmP,startTraceType,pnt1,pnt2,pnt3,x,y,nextPnt1P,nextPnt2P,nextPnt3P,nextXP,nextYP,nextZP)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Drainage Trace Start Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Drainage Trace Start Error") ;
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
int bcdtmDrainage_getFirstTracePointFromTriangleDtmObjectOld
(
 BC_DTM_OBJ     *dtmP,
 long           flowDirection,             // 1 = Ascent , 2 = Descent
 long           pnt1,
 long           pnt2,
 long           pnt3,
 double         startX,
 double         startY,
 long           *nextPnt1P,
 long           *nextPnt2P,
 long           *nextPnt3P,
 double         *nextXP,
 double         *nextYP,
 double         *nextZP
)
/*
** Calculates The Intersection Of the Maximum Ascent or Descent Vector with the Triangle Edge
** Flowdirection **  1 = Ascent , 2 = Descent
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   mp1,mp2,minp,onLine,firstFound=1,edge1=TRUE,edge2=TRUE,edge3=TRUE ;
 double dx,dy,dz,angle,xr,yr,nd,nd1,nd2,nd3,xn,yn,mind=0.0;
 double slope,radius,descentAngle,ascentAngle ;

// Log Arguements
dbg=DrainageDebug ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting First Trace Point") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"flowDirection   = %8ld",flowDirection) ;
    bcdtmWrite_message(0,0,0,"pnt1            = %8ld",pnt1) ;
    bcdtmWrite_message(0,0,0,"pnt2            = %8ld",pnt2) ;
    bcdtmWrite_message(0,0,0,"pnt3            = %8ld",pnt3) ;
    bcdtmWrite_message(0,0,0,"startX          = %12.5lf",startX) ;
    bcdtmWrite_message(0,0,0,"startY          = %12.5lf",startY) ;
   }
/*
** Initialise Return Arguments
*/
 *nextXP = 0.0 ;
 *nextYP = 0.0 ;
 *nextZP = 0.0 ;
 *nextPnt1P = dtmP->nullPnt ;
 *nextPnt2P = dtmP->nullPnt ;
 *nextPnt3P = dtmP->nullPnt ;
/*
** Initialise
*/
 minp = dtmP->nullPnt ;
/*
** Check If Point In Triangle
*/
 if( ! bcdtmMath_pointInsideTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,startX,startY) )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Point External To Triangle") ;
    nd1 = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,startX,startY,&xn,&yn) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Line[1-2] ** nd1 = %20.15lf onLine = %2ld",nd1,onLine) ;
    nd2 = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,startX,startY,&xn,&yn) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Line[2-3] ** nd2 = %20.15lf onLine = %2ld",nd2,onLine) ;
    nd3 = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,startX,startY,&xn,&yn) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Line[3-1] ** nd3 = %20.15lf onLine = %2ld",nd3,onLine) ;
    if( nd1 < nd2 && nd1 < nd3 ) edge1 = FALSE ;
    if( nd2 < nd1 && nd2 < nd3 ) edge2 = FALSE ;
    if( nd3 < nd1 && nd3 < nd2 ) edge3 = FALSE ;
   }

// Check If Point Is Coincident With A Triangle Vertex
/*
 bcdtmWrite_message(0,0,0,"dtmP->pptol   = %20.15lf",dtmP->ppTol) ;
 bcdtmWrite_message(0,0,0,"Distance Pnt1 = %20.15lf",bcdtmMath_distance(startX,startY,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y)) ;
 bcdtmWrite_message(0,0,0,"Distance Pnt2 = %20.15lf",bcdtmMath_distance(startX,startY,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y)) ;
 bcdtmWrite_message(0,0,0,"Distance Pnt3 = %20.15lf",bcdtmMath_distance(startX,startY,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y)) ;
 if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) <= dtmP->ppTol )
     {
     *nextXP = pointAddrP(dtmP,pnt1)->x ;
     *nextYP = pointAddrP(dtmP,pnt1)->y ;
     *nextZP = pointAddrP(dtmP,pnt1)->z ;
     *nextPnt1P = pnt1 ;
     goto cleanup ;
     }
 if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) <= dtmP->ppTol )
     {
     *nextXP = pointAddrP(dtmP,pnt2)->x ;
     *nextYP = pointAddrP(dtmP,pnt2)->y ;
     *nextZP = pointAddrP(dtmP,pnt2)->z ;
     *nextPnt1P = pnt2 ;
     goto cleanup ;
     }
 if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) <= dtmP->ppTol )
     {
     *nextXP = pointAddrP(dtmP,pnt3)->x ;
     *nextYP = pointAddrP(dtmP,pnt3)->y ;
     *nextZP = pointAddrP(dtmP,pnt3)->z ;
     *nextPnt1P = pnt3 ;
     goto cleanup ;
     }
*/
/*
** Initialise Calculation Variables
*/
 bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,pnt3,&descentAngle,&ascentAngle,&slope) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Slope = %10.5lf ** descentAngle = %12.10lf ascentAngle = %12.10lf",slope,descentAngle,ascentAngle) ;
 angle = 0.0 ;
 if( flowDirection == 1 ) angle = ascentAngle  ;
 if( flowDirection == 2 ) angle = descentAngle ;
 if( dbg ) bcdtmWrite_message(0,0,0,"flowAngle = %12.10lf",angle) ;
 dx = dtmP->xMax - dtmP->xMin ;
 dy = dtmP->yMax - dtmP->yMin ;
 radius = sqrt(dx*dx + dy*dy)  ;
 xr = startX + radius * cos(angle) ;
 yr = startY + radius * sin(angle) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"radialAngle = %12.10lf",bcdtmMath_getAngle(startX,startY,xr,yr)) ;
/*
** Determine If Radial Intersects A Triangle Vertex
*/
 mp1 = mp2 = dtmP->nullPnt ;
 if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) == 0 ) mp1 = pnt1 ;
 if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) == 0 ) mp1 = pnt2 ;
 if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) == 0 ) mp1 = pnt3 ;
 if( mp1 != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Radial Intersects Triangle Vextex %8ld",mp1) ;
    *nextXP = pointAddrP(dtmP,mp1)->x ;
	*nextYP = pointAddrP(dtmP,mp1)->y ;
	*nextZP = pointAddrP(dtmP,mp1)->z ;
    *nextPnt1P = mp1 ;
   }
/*
** Determine Triangle Edge Intersected With Radial
*/
 else
   {

if( dbg )
{
   edge1 = edge2 = edge3 = FALSE ;
   if( bcdtmMath_checkIfLinesIntersect(startX,startY,xr,yr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y))
       {
       bcdtmWrite_message(0,0,0,"Intersect pnt1-pnt2 = %8ld-%8ld",pnt1,pnt2) ;
       mp2 = pnt1 ;
       mp1 = pnt2 ;
       }
   else if( bcdtmMath_checkIfLinesIntersect(startX,startY,xr,yr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y))
       {
       bcdtmWrite_message(0,0,0,"Intersect pnt2-pnt3 = %8ld-%8ld",pnt2,pnt3) ;
       mp2 = pnt2 ;
       mp1 = pnt3 ;
       }
   else if( bcdtmMath_checkIfLinesIntersect(startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y))
       {
       bcdtmWrite_message(0,0,0,"Intersect pnt3-pnt1 = %8ld-%8ld",pnt3,pnt1) ;
       mp2 = pnt3 ;
       mp1 = pnt1 ;
       }
}

    if( edge1 == TRUE )
      {
       if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) < 0 &&
           bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) > 0 &&
           bcdtmMath_sideOf(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,xr,yr) < 0 ) { mp1 = pnt1 ; mp2 = pnt2 ; }
      }
    if( edge2 == TRUE )
      {
       if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) < 0 &&
           bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) > 0 &&
           bcdtmMath_sideOf(pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,xr,yr) < 0 ) { mp1 = pnt2 ; mp2 = pnt3 ; }
       }
    if( edge3 == TRUE )
      {
       if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) < 0 &&
           bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) > 0 &&
          bcdtmMath_sideOf(pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,xr,yr) < 0 ) { mp1 = pnt3 ; mp2 = pnt1 ; }
      }
/*
**  Calculate Intersection Point If Edge Detected
*/
    if( mp1 != dtmP->nullPnt && mp2 != dtmP->nullPnt )
	  {
       if( dbg ) bcdtmWrite_message(0,0,0,"Radial Intersects Triangle Edge %8ld %8ld",mp1,mp2) ;
/*
**     Calculate Intercept Of Radial On Mp1,Mp2
*/
       bcdtmDrainage_intersectCordLines(startX,startY,xr,yr,pointAddrP(dtmP,mp1)->x,pointAddrP(dtmP,mp1)->y,pointAddrP(dtmP,mp2)->x,pointAddrP(dtmP,mp2)->y,nextXP,nextYP) ;
       dx = pointAddrP(dtmP,mp2)->x - pointAddrP(dtmP,mp1)->x ;
       dy = pointAddrP(dtmP,mp2)->y - pointAddrP(dtmP,mp1)->y ;
       dz = pointAddrP(dtmP,mp2)->z - pointAddrP(dtmP,mp1)->z ;
       if( fabs(dx) >= fabs(dy) ) *nextZP = pointAddrP(dtmP,mp1)->z +  dz * (*nextXP - pointAddrP(dtmP,mp1)->x) / dx ;
       else                       *nextZP = pointAddrP(dtmP,mp1)->z +  dz * (*nextYP - pointAddrP(dtmP,mp1)->y) / dy ;
       *nextPnt1P = mp2 ;
       *nextPnt2P = mp1 ;
       if( ( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,mp2,mp1)) < 0 ) goto errexit ;
       if( ! bcdtmList_testLineDtmObject(dtmP,mp1,*nextPnt3P))
           {
            *nextPnt3P = dtmP->nullPnt ;
           }
      }
/*
**  Edge Not Found - Find Closest Triangle Point To Radial
*/
    else
      {
       nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,&xn,&yn) ;
       if( onLine && ( firstFound || nd < mind )) { firstFound = 0 ; mind = nd ; minp = pnt1 ; }
       nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,&xn,&yn) ;
       if( onLine && ( firstFound || nd < mind )) { firstFound = 0 ; mind = nd ; minp = pnt2 ; }
       nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,&xn,&yn) ;
       if( onLine && ( firstFound || nd < mind )) { firstFound = 0 ; mind = nd ; minp = pnt3 ; }
       if( ! firstFound )
         {
          *nextXP = pointAddrP(dtmP,minp)->x ;
	      *nextYP = pointAddrP(dtmP,minp)->y ;
	      *nextZP = pointAddrP(dtmP,minp)->z ;
          *nextPnt1P = minp ;
         }
       else
         {
          bcdtmWrite_message(0,0,0,"Intersection Of Trace With Triangle Edge Not Found") ;
          goto errexit ;
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
int bcdtmDrainage_getFirstTracePointFromTriangleDtmObject
(
 BC_DTM_OBJ     *dtmP,
 long           flowDirection,             // 1 = Ascent , 2 = Descent
 long           pnt1,
 long           pnt2,
 long           pnt3,
 double         startX,
 double         startY,
 long           *nextPnt1P,
 long           *nextPnt2P,
 long           *nextPnt3P,
 double         *nextXP,
 double         *nextYP,
 double         *nextZP
)
/*
** Calculates The Intersection Of the Maximum Ascent or Descent Vector with the Triangle Edge
** Flowdirection **  1 = Ascent , 2 = Descent
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   mp1,mp2,minp,onLine,firstFound=1 ;
 double dx,dy,dz,angle,xr,yr,nd,nd1,nd2,nd3,xn,yn,mind=0.0;
 double slope,radius,descentAngle,ascentAngle ;
 double minRadX,maxRadX,minRadY,maxRadY ;
 DTMDirection trgDirection ;

// Log Arguements

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting First Trace Point") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"flowDirection   = %8ld",flowDirection) ;
    bcdtmWrite_message(0,0,0,"pnt1            = %8ld",pnt1) ;
    bcdtmWrite_message(0,0,0,"pnt2            = %8ld",pnt2) ;
    bcdtmWrite_message(0,0,0,"pnt3            = %8ld",pnt3) ;
    bcdtmWrite_message(0,0,0,"startX          = %12.5lf",startX) ;
    bcdtmWrite_message(0,0,0,"startY          = %12.5lf",startY) ;
   }
/*
** Initialise Return Arguments
*/
 *nextXP = 0.0 ;
 *nextYP = 0.0 ;
 *nextZP = 0.0 ;
 *nextPnt1P = dtmP->nullPnt ;
 *nextPnt2P = dtmP->nullPnt ;
 *nextPnt3P = dtmP->nullPnt ;
 trgDirection = DTMDirection::Clockwise ;
 if( bcdtmMath_pointSideOfDtmObject(dtmP,pnt1,pnt2,pnt3) > 0 )
     trgDirection = DTMDirection::AntiClockwise ;
/*
** Initialise
*/
 minp = dtmP->nullPnt ;
/*
** Check If Point In Triangle
*/
 if( ! bcdtmMath_pointInsideTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,startX,startY) )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Point External To Triangle") ;
    nd1 = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,startX,startY,&xn,&yn) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Line[1-2] ** nd1 = %20.15lf onLine = %2ld",nd1,onLine) ;
    nd2 = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,startX,startY,&xn,&yn) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Line[2-3] ** nd2 = %20.15lf onLine = %2ld",nd2,onLine) ;
    nd3 = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,startX,startY,&xn,&yn) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Line[3-1] ** nd3 = %20.15lf onLine = %2ld",nd3,onLine) ;
   }

/*
** Initialise Calculation Variables
*/
 bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,pnt3,&descentAngle,&ascentAngle,&slope) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Slope = %10.5lf ** descentAngle = %12.10lf ascentAngle = %12.10lf",slope,descentAngle,ascentAngle) ;
 angle = 0.0 ;
 if( flowDirection == 1 ) angle = ascentAngle  ;
 else if( flowDirection == 2 ) angle = descentAngle ;
 if( dbg ) bcdtmWrite_message(0,0,0,"flowAngle = %12.10lf",angle) ;
 dx = dtmP->xMax - dtmP->xMin ;
 dy = dtmP->yMax - dtmP->yMin ;
 radius = sqrt(dx*dx + dy*dy)  ;
 xr = startX + radius * cos(angle) ;
 yr = startY + radius * sin(angle) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"radialAngle = %12.10lf",bcdtmMath_getAngle(startX,startY,xr,yr)) ;
/*
** Determine If Radial Intersects A Triangle Vertex
*/
 mp1 = mp2 = dtmP->nullPnt ;
 if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) == 0 ) mp1 = pnt1 ;
 else if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) == 0 ) mp1 = pnt3 ;
 else if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) == 0 ) mp1 = pnt2 ;
 if( mp1 != dtmP->nullPnt )
   {

    // Check Vertex Is On Line

    if( startX <= xr ) { minRadX = startX ; maxRadX = xr     ; }
    else               { minRadX = xr     ; maxRadX = startX ; }
    if( startY <= yr ) { minRadY = startX ; maxRadY = yr     ; }
    else               { minRadY = yr     ; maxRadY = startY ; }
    if( pointAddrP(dtmP,mp1)->x >= minRadX && pointAddrP(dtmP,mp1)->x <= maxRadX &&
        pointAddrP(dtmP,mp1)->y >= minRadY && pointAddrP(dtmP,mp1)->y <= maxRadY      )
        {
        if( dbg ) bcdtmWrite_message(0,0,0,"Radial Intersects Triangle Vextex %8ld",mp1) ;
        *nextXP = pointAddrP(dtmP,mp1)->x ;
	    *nextYP = pointAddrP(dtmP,mp1)->y ;
	    *nextZP = pointAddrP(dtmP,mp1)->z ;
        *nextPnt1P = mp1 ;
        }
    else  mp1 = dtmP->nullPnt ;
   }
/*
** Determine Triangle Edge Intersected With Radial
*/
 else if( *nextPnt1P == dtmP->nullPnt )
   {
   if( bcdtmMath_checkIfLinesIntersect(startX,startY,xr,yr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y))
       {
       if( dbg ) bcdtmWrite_message(0,0,0,"Intersect pnt1-pnt2 = %8ld-%8ld",pnt1,pnt2) ;
       mp1 = pnt1 ;
       mp2 = pnt2 ;
       }
   else if( bcdtmMath_checkIfLinesIntersect(startX,startY,xr,yr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y))
       {
       if( dbg ) bcdtmWrite_message(0,0,0,"Intersect pnt2-pnt3 = %8ld %8ld",pnt2,pnt3) ;
       mp1 = pnt2 ;
       mp2 = pnt3 ;
       }
   else if( bcdtmMath_checkIfLinesIntersect(startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y))
       {
       if( dbg ) bcdtmWrite_message(0,0,0,"Intersect pnt3-pnt1 = %8ld %8ld",pnt3,pnt1) ;
       mp1 = pnt3 ;
       mp2 = pnt1 ;
       }
/*
**  Calculate Intersection Point If Edge Detected
*/
    if( mp1 != dtmP->nullPnt && mp2 != dtmP->nullPnt )
	  {
       if( dbg ) bcdtmWrite_message(0,0,0,"Radial Intersects Triangle Edge %8ld %8ld",mp1,mp2) ;
/*
**     Calculate Intercept Of Radial On Mp1,Mp2
*/
       bcdtmDrainage_intersectCordLines(startX,startY,xr,yr,pointAddrP(dtmP,mp1)->x,pointAddrP(dtmP,mp1)->y,pointAddrP(dtmP,mp2)->x,pointAddrP(dtmP,mp2)->y,nextXP,nextYP) ;
       dx = pointAddrP(dtmP,mp2)->x - pointAddrP(dtmP,mp1)->x ;
       dy = pointAddrP(dtmP,mp2)->y - pointAddrP(dtmP,mp1)->y ;
       dz = pointAddrP(dtmP,mp2)->z - pointAddrP(dtmP,mp1)->z ;
       if( fabs(dx) >= fabs(dy) ) *nextZP = pointAddrP(dtmP,mp1)->z +  dz * (*nextXP - pointAddrP(dtmP,mp1)->x) / dx ;
       else                       *nextZP = pointAddrP(dtmP,mp1)->z +  dz * (*nextYP - pointAddrP(dtmP,mp1)->y) / dy ;

       if( trgDirection == DTMDirection::AntiClockwise )
           {
           *nextPnt1P = mp2 ;
           *nextPnt2P = mp1 ;
           if( ( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,mp2,mp1)) < 0 ) goto errexit ;
           if( ! bcdtmList_testLineDtmObject(dtmP,mp1,*nextPnt3P))
               {
               *nextPnt3P = dtmP->nullPnt ;
               }
           }
       else
           {
           *nextPnt1P = mp1 ;
           *nextPnt2P = mp2 ;
           if( ( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,mp1,mp2)) < 0 ) goto errexit ;
           if( ! bcdtmList_testLineDtmObject(dtmP,mp1,*nextPnt3P))
               {
               *nextPnt3P = dtmP->nullPnt ;
               }
           }

      }
/*
**  Edge Not Found - Find Closest Triangle Point To Radial
*/
    else
      {
       nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,&xn,&yn) ;
       if( onLine && ( firstFound || nd < mind )) { firstFound = 0 ; mind = nd ; minp = pnt1 ; }
       nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,&xn,&yn) ;
       if( onLine && ( firstFound || nd < mind )) { firstFound = 0 ; mind = nd ; minp = pnt2 ; }
       nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,&xn,&yn) ;
       if( onLine && ( firstFound || nd < mind )) { firstFound = 0 ; mind = nd ; minp = pnt3 ; }
       if( ! firstFound )
         {
          *nextXP = pointAddrP(dtmP,minp)->x ;
	      *nextYP = pointAddrP(dtmP,minp)->y ;
	      *nextZP = pointAddrP(dtmP,minp)->z ;
          *nextPnt1P = minp ;
         }
       else
         {
          bcdtmWrite_message(0,0,0,"Intersection Of Trace With Triangle Edge Not Found") ;
          goto errexit ;
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
int bcdtmDrainage_getFirstTracePointFromTriangleEdgeDtmObject
(
 BC_DTM_OBJ   *dtmP,
 long         flowDirection,
 long         pnt1,
 long         pnt2,
 long         pnt3,
 double       startX,
 double       startY,
 long         *nextPnt1P,
 long         *nextPnt2P,
 long         *nextPnt3P,
 double       *nextXP,
 double       *nextYP,
 double       *nextZP
)
/*
** Calculates The Intersection Of the Maximum Ascent or Descent Vector with the Triangle Edge
** Note :- pnt1-pnt2-pnt3 - Must Form A Clockwise Triangle. Assumes This is prior validated
** flowDirection **  1 = Ascent , 2 = Descent
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   mp1,mp2,minp=0,onLine,firstFound=1,zeroSlopeTriangle=0 ;
 double dx,dy,dz,angle,xr,yr,nd,xn,yn,mind=0.0;
 double slope,radius,descentAngle,ascentAngle ;
 DTMDirection dtmDirection ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting First Trace Point From Triangle Edge") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"flowDirection  = %8ld",flowDirection) ;
    bcdtmWrite_message(0,0,0,"pnt1           = %8ld",pnt1)  ;
    bcdtmWrite_message(0,0,0,"pnt2           = %8ld",pnt2)  ;
    bcdtmWrite_message(0,0,0,"pnt3           = %8ld",pnt3)  ;
    bcdtmWrite_message(0,0,0,"startX         = %15.5lf",startX)  ;
    bcdtmWrite_message(0,0,0,"startY         = %15.5lf",startY)  ;
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"pnt1 = %9ld  **  %12.5lf %12.5lf %10.4lf",pnt1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z ) ;
       bcdtmWrite_message(0,0,0,"pnt2 = %9ld  **  %12.5lf %12.5lf %10.4lf",pnt2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z ) ;
       bcdtmWrite_message(0,0,0,"pnt3 = %9ld  **  %12.5lf %12.5lf %10.4lf",pnt3,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt3)->z ) ;
      }
   }
/*
** Initialise Return Arguments
*/
 *nextXP = 0.0 ;
 *nextYP = 0.0 ;
 *nextZP = 0.0 ;
 *nextPnt1P = dtmP->nullPnt ;
 *nextPnt2P = dtmP->nullPnt ;
 *nextPnt3P = dtmP->nullPnt ;
 dtmDirection =  DTMDirection::Clockwise ;            // Set To Clockwise
 if( bcdtmMath_pointSideOfDtmObject(dtmP,pnt1,pnt2,pnt3) > 0 )
     dtmDirection =  DTMDirection::AntiClockwise ;

/*
** Check For Zero Slope Triangle
*/
 if( pointAddrP(dtmP,pnt1)->z == pointAddrP(dtmP,pnt2)->z && pointAddrP(dtmP,pnt1)->z == pointAddrP(dtmP,pnt3)->z) zeroSlopeTriangle = 1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"zeroSlopeTriangle = %2ld",zeroSlopeTriangle) ;
/*
** Initialise Calculation Variables
*/
 if( ! zeroSlopeTriangle )
   {
    bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,pnt3,&descentAngle,&ascentAngle,&slope) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"ascentAngle = %12.10lf descentAngle = %12.10lf slope = %10.4lf",ascentAngle,descentAngle,slope) ;
    angle = 0.0 ;
    if( flowDirection == 1 ) angle = ascentAngle  ;
    if( flowDirection == 2 ) angle = descentAngle ;
   }
 else angle = bcdtmMath_normaliseAngle((bcdtmMath_getPointAngleDtmObject(dtmP,pnt1,pnt2)-DTM_PYE/2.0)) ;
 dx = dtmP->xMax - dtmP->xMin ;
 dy = dtmP->yMax - dtmP->yMin ;
 radius = sqrt(dx*dx + dy*dy)  ;
 xr = startX + radius * cos(angle) ;
 yr = startY + radius * sin(angle) ;
/*
** Determine If Radial Intersects Opposite Triangle Vertex
*/
 if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) == 0 ) *nextPnt1P = pnt3 ;
 else if ( bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,&xn,&yn) < dtmP->ppTol) *nextPnt1P = pnt3 ;
 if( *nextPnt1P != dtmP->nullPnt )
   {
    *nextXP = pointAddrP(dtmP,*nextPnt1P)->x ;
	*nextYP = pointAddrP(dtmP,*nextPnt1P)->y ;
	*nextZP = pointAddrP(dtmP,*nextPnt1P)->z ;
   }
/*
** Determine Triangle Edge Intersected With Radial
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Determining Triangle Edge Intersected With Radial") ;
    mp1 = dtmP->nullPnt ;
    mp2 = dtmP->nullPnt ;
    if     ( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) > 0 &&
             bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) < 0    ) { mp1 = pnt2 ; mp2 = pnt3 ; }
    else if( bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) > 0 &&
             bcdtmMath_sideOf(startX,startY,xr,yr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) < 0    ) { mp1 = pnt3 ; mp2 = pnt1 ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"mp1 = %9ld mp2 = %9ld",mp1,mp2) ;
/*
**  Calculate Intersection Point If Edge Detected
*/
    if( mp1 != dtmP->nullPnt && mp2 != dtmP->nullPnt )
	  {
/*
**     Check For Ascent Line Coincident With End Points
*/
       nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,mp1)->x,pointAddrP(dtmP,mp1)->y,&xn,&yn) ;
       if( onLine && nd < dtmP->ppTol ) *nextPnt1P = mp1 ;
       else
         {
          nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,mp2)->x,pointAddrP(dtmP,mp2)->y,&xn,&yn) ;
          if( onLine && nd < dtmP->ppTol ) *nextPnt1P = mp2 ;
         }
/*
**     Ascent Coincident With Tin Point
*/
       if( *nextPnt1P != dtmP->nullPnt )
         {
          *nextXP = pointAddrP(dtmP,*nextPnt1P)->x ;
	      *nextYP = pointAddrP(dtmP,*nextPnt1P)->y ;
	      *nextZP = pointAddrP(dtmP,*nextPnt1P)->z ;
         }

/*
**     Calculate Intercept Of Radial On Mp1,Mp2
*/
       else
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Triangle Edge Intercept") ;
          bcdtmDrainage_intersectCordLines(startX,startY,xr,yr,pointAddrP(dtmP,mp1)->x,pointAddrP(dtmP,mp1)->y,pointAddrP(dtmP,mp2)->x,pointAddrP(dtmP,mp2)->y,nextXP,nextYP) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"*nextXP = %12.5lf *nextYP = %12.5lf",*nextXP,*nextYP) ;
          dx = pointAddrP(dtmP,mp2)->x - pointAddrP(dtmP,mp1)->x ;
          dy = pointAddrP(dtmP,mp2)->y - pointAddrP(dtmP,mp1)->y ;
          dz = pointAddrP(dtmP,mp2)->z - pointAddrP(dtmP,mp1)->z ;
          if( fabs(dx) >= fabs(dy) ) *nextZP = pointAddrP(dtmP,mp1)->z +  dz * (*nextXP - pointAddrP(dtmP,mp1)->x) / dx ;
          else                       *nextZP = pointAddrP(dtmP,mp1)->z +  dz * (*nextYP - pointAddrP(dtmP,mp1)->y) / dy ;
          *nextPnt1P = mp1 ;
          *nextPnt2P = mp2 ;
          if( ( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,mp1,mp2)) < 0 ) goto errexit ;
         }
      }
/*
**  Edge Not Found - Find Closest Triangle Point To Radial
*/
    else
      {
       if( dbg ) bcdtmWrite_message(2,0,0,"Intersection Of Trace With Triangle Edge Not Found") ;
       nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,&xn,&yn) ;
       if( onLine && ( firstFound || nd < mind )) { firstFound = 0 ; mind = nd ; minp = pnt1 ; }
       nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,&xn,&yn) ;
       if( onLine && ( firstFound || nd < mind )) { firstFound = 0 ; mind = nd ; minp = pnt2 ; }
       nd = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,xr,yr,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,&xn,&yn) ;
       if( onLine && ( firstFound || nd < mind )) { firstFound = 0 ; mind = nd ; minp = pnt3 ; }
       if( ! firstFound )
         {
          *nextXP = pointAddrP(dtmP,minp)->x ;
	      *nextYP = pointAddrP(dtmP,minp)->y ;
	      *nextZP = pointAddrP(dtmP,minp)->z ;
          *nextPnt1P = minp ;
         }
       else
         {
          bcdtmWrite_message(2,0,0,"Intersection Of Trace With Triangle Edge Not Found") ;
          goto errexit ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting First Trace Point From Triangle Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting First Trace Point From Triangle Edge Completed Error") ;
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
int bcdtmDrainage_traceToLowPointDtmObject
(
 BC_DTM_OBJ *dtmP,                   // ==> Pointer To DTM Object
 DTMDrainageTables *drainageTablesP, // ==> Pointer To Drainage Tables
 DTMFeatureCallback loadFunctionP,   // ==> Pointer To Load Function
 double falseLowDepth,               // ==> False Low Depth
 int    zeroSlopeOption,             // ==> Zero Slope Option ( 1 == Trace At Last Angle , 2 == Create Pond )
 bool   loadFlag,                    // ==> Load Trace Points
 long   pnt1,                        // ==> Triangle Point 1
 long   pnt2,                        // ==> Triangle Point 2
 long   pnt3,                        // ==> Triangle Point 3
 double X,                           // ==> Start Trace Point X Coordinate
 double Y,                           // ==> Start Trace Point Y Coordinate
 double Z,                           // ==> Start Trace Point Z Coordinate
 void   *userP,                      // ==> User Pointer Passed Back To User
 long   *lowPoint1P,                 // <== Low Point 1
 long   *lowPoint2P                  // <== Low Point 2 ( sump line )
)
/*
** This Function Traces To A Low Point
**
** Notes :-
**
** 1. Triangle Points pnt1-pnt2-pnt3 Must Be Set Anti Clockwise
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   nextPnt1,nextPnt2,nextPnt3,priorPnt,nextPnt,savePnt1,savePnt2,savePnt3,iteration=0,startPointType ;
 long   process,lastPoint,exitPoint,priorPoint,nextPoint,isFalseLow,numSumpLines=0,previousExit,hullFlag ;
 double firstX,firstY,firstZ,startX,startY,startZ,nextX,nextY,nextZ ;
 double area,lastAngle=-99.99,saveLastAngle ;
 bool   trace=true ;
 DTM_SUMP_LINES   *sumpLinesP=NULL ;
 DTM_POLYGON_OBJ  *polygonP=NULL ;
 DTMUserTag nullUserTag = DTM_NULL_USER_TAG ;
 DTMFeatureId nullFeatureId = DTM_NULL_FEATURE_ID ;
 DTMDrainageTracePoints  tracePoints ;
 DTMPointCache           streamPoints ;
/*
** Write Entry Message
*/
//dbg=DrainageDebug ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing To Low Point") ;
    bcdtmWrite_message(0,0,0,"Dtm Object         = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"drainageTablesP    = %p",drainageTablesP)  ;
    bcdtmWrite_message(0,0,0,"loadFunctionP      = %p",loadFunctionP)  ;
    bcdtmWrite_message(0,0,0,"falseLowDepth      = %10.4lf",falseLowDepth) ;
    bcdtmWrite_message(0,0,0,"zeroSlopeOption    = %8d",zeroSlopeOption)  ;
    bcdtmWrite_message(0,0,0,"loadFlag           = %8ld",loadFlag)  ;
    bcdtmWrite_message(0,0,0,"pnt1               = %8ld",pnt1) ;
    bcdtmWrite_message(0,0,0,"pnt2               = %8ld",pnt2) ;
    bcdtmWrite_message(0,0,0,"pnt3               = %8ld",pnt3) ;
    bcdtmWrite_message(0,0,0,"X                  = %12.5lf",X)  ;
    bcdtmWrite_message(0,0,0,"Y                  = %12.5lf",Y)  ;
    bcdtmWrite_message(0,0,0,"Z                  = %12.5lf",Z)  ;
    bcdtmWrite_message(0,0,0,"userP              = %p",userP)  ;
   }


// Initialise

 saveLastAngle = 0.0  ;
 process = 0 ;
 firstX = startX = X ;
 firstY = startY = Y ;
 firstZ = startZ = Z ;
 priorPnt = dtmP->nullPnt ;
 nextPnt  = dtmP->nullPnt ;
 lastPoint  = dtmP->nullPnt ;
 isFalseLow = 0 ;
 if( falseLowDepth > 0.0 ) isFalseLow = 1 ;
 if( zeroSlopeOption < 1 || zeroSlopeOption > 2 ) zeroSlopeOption = 1 ;

// Check Triangle Start Points Are Counter Clockwise

 if( pnt3 != dtmP->nullPnt && bcdtmMath_pointSideOfDtmObject(dtmP,pnt1,pnt2,pnt3) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Stream Trace Start Triangle Is Not Counter Clockwise") ;
    goto errexit ;
   }

//  Store Starting Down Stream Point

 if( loadFlag )
     {
      if( dbg ) bcdtmWrite_message(0,0,0,"Storing Start Stream Point %12.5lf %12.5lf %12.5lf",startX,startY,startZ) ;
      if( streamPoints.StorePointInCache(startX,startY,startZ) ) goto errexit ;
     }

// Get Drainage Trace Start

 if( bcdtmDrainage_startTraceDtmObject(dtmP,2,pnt1,pnt2,pnt3,startX,startY,startZ,&startPointType,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&lastAngle)) goto errexit ;
 if( dbg )  bcdtmWrite_message(0,0,0,"TraceStartPointType = %3ld ** nextX = %12.5lf nextY = %12.5lf nextZ = %10.4lf",startPointType,nextX,nextY,nextZ) ;
/*
** Only Process For A Valid Start Point
*/
 if( startPointType >= 1 && startPointType <= 3 )
   {
    if( startPointType == 1 )
        {
        if( pnt1 != nextPnt1)
            {
            tracePoints.StoreTracePoint(nextX,nextY,nextZ,nextPnt1,nextPnt2) ;
            if( loadFlag )
                {
                if( dbg ) bcdtmWrite_message(0,0,0,"Storing Stream Point On Triangle Edge %12.5lf %12.5lf %12.5lf",nextX,nextY,nextZ) ;
                if( streamPoints.StorePointInCache(nextX,nextY,nextZ)) goto errexit ;
                }
            }
        pnt1 = nextPnt1 ;
        pnt2 = nextPnt2 ;
        pnt3 = nextPnt3 ;
        }
    if( startPointType == 2 )   // Coincident With Triangle Edge
      {
       pnt1 = nextPnt1 ;
       pnt2 = nextPnt2 ;
       pnt3 = nextPnt3 ;
      }
    if( startPointType == 3 )   // Internal To Triangle
      {

//     Store Trace Point For False Low Processing

       tracePoints.StoreTracePoint(nextX,nextY,nextZ,nextPnt1,nextPnt2) ;

//     Store Down Stream Trace Point

       if( loadFlag )
           {
            if( dbg ) bcdtmWrite_message(0,0,0,"Storing Stream Point On Triangle Edge %12.5lf %12.5lf %12.5lf",nextX,nextY,nextZ) ;
            if( streamPoints.StorePointInCache(nextX,nextY,nextZ)) goto errexit ;
           }

//     Initialise Iteration Variables

       pnt1 = nextPnt1 ;
       pnt2 = nextPnt2 ;
       pnt3 = nextPnt3 ;
       startX = nextX  ;
       startY = nextY  ;
       startZ = nextZ  ;
       saveLastAngle = lastAngle ;
       if( pnt3 == dtmP->nullPnt ) trace = false ;
      }
/*
**  Iteratively Get Next Trace Point
*/
    if( trace )
      {
       do
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"iteration[%6ld] ** pnt1 = %9ld pnt2 = %9ld pnt3 = %9ld  ** X = %12.5lf Y = %12.5lf Z = %12.5lf",iteration,pnt1,pnt2,pnt3,startX,startY,startZ) ;
          ++iteration ;

//        Save Points For Drawing Of Ponds

          savePnt1 = pnt1 ;
          savePnt2 = pnt2 ;
          savePnt3 = pnt3 ;

//        Trace To Next Low Point

          if( pnt2 != dtmP->nullPnt ){ if( bcdtmDrainage_traceMaximumDescentFromTriangleEdgeDtmObject(dtmP,drainageTablesP,zeroSlopeOption,isFalseLow,lastAngle,pnt1,pnt2,pnt3,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process,&exitPoint,&priorPoint,&nextPoint)) goto errexit ; }
          else                       { if( bcdtmDrainage_traceMaximumDescentFromTrianglePointDtmObject(dtmP,drainageTablesP,zeroSlopeOption,isFalseLow,lastAngle,lastPoint,pnt1,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process,&exitPoint,&priorPoint,&nextPoint)) goto errexit ; }
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"**** exitPoint = %10ld nextPnt1 = %10ld nextPnt2 = %10ld nextX = %12.5lf nextY = %12.5lf nextZ = %12.5lf",exitPoint,nextPnt1,nextPnt2,nextX,nextY,nextZ) ;

//        Check Trace Is Going Down

          if( cdbg &&  nextPnt1 != dtmP->nullPnt )
            {
             if( nextZ - startZ > 0.00000001 )  // Allow For Some Math Precision Problems
               {
                bcdtmWrite_message(1,0,0,"Downstream Trace Elevation Increased ** startZ = %15.10lf nextZ = %15.10lf",startZ,nextZ) ;
                goto errexit ;
               }
            }

//        Check Point On Zero SLope Sump Line Has Not Been Prior Processed

          if( pnt2 == dtmP->nullPnt && nextPnt2 == dtmP->nullPnt && startZ == nextZ )
            {
             if( tracePoints.CheckForPriorTracePoint(nextPnt1) ) process = 0   ;
            }

//        Set For Next Trace Point

          if( process )
            {
             tracePoints.StoreTracePoint(nextX,nextY,nextZ,nextPnt1,nextPnt2) ;
             if( loadFlag )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Storing Next Stream Point %12.5lf %12.5lf %12.5lf",nextX,nextY,nextZ) ;
                if( streamPoints.StorePointInCache(nextX,nextY,nextZ)) goto errexit ;
               }
             if( pnt2 == dtmP->nullPnt ) lastPoint = pnt1 ;
             else                        lastPoint = dtmP->nullPnt ;
             pnt1 = nextPnt1 ;
             pnt2 = nextPnt2 ;
             pnt3 = nextPnt3 ;
             if( startX != nextX || startY != nextY ) lastAngle = bcdtmMath_getAngle(startX,startY,nextX,nextY) ;
             else                                     lastAngle = saveLastAngle ;
             saveLastAngle = lastAngle ;
             startX = nextX  ;
             startY = nextY  ;
             startZ = nextZ  ;
             if( startX == firstX && startY == firstY ) process = 0 ;
            }
          else
            {

//           Draw Last Descent Trace Point

             if( loadFlag )
               {
                if( dbg ) streamPoints.LogCachePoints() ;
                if( streamPoints.CallUserDelegateWithCachePoints( (DTMFeatureCallback)loadFunctionP,DTMFeatureType::DescentTrace,dtmP->nullUserTag,dtmP->nullFeatureId,userP)) goto errexit ;
               }

//           Process Pond Exit Point If It Exists

             if( exitPoint != dtmP->nullPnt  )
               {
                if( dbg == 2 )
                  {
                   bcdtmWrite_message(0,0,0,"lowPoint   = %9ld ** %10.4lf %10.4lf %10.4lf",pnt1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
                   bcdtmWrite_message(0,0,0,"priorPoint = %9ld ** %10.4lf %10.4lf %10.4lf",priorPoint,pointAddrP(dtmP,priorPoint)->x,pointAddrP(dtmP,priorPoint)->y,pointAddrP(dtmP,priorPoint)->z) ;
                   bcdtmWrite_message(0,0,0,"exitPoint  = %9ld ** %10.4lf %10.4lf %10.4lf",exitPoint,pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,pointAddrP(dtmP,exitPoint)->z) ;
                   bcdtmWrite_message(0,0,0,"nextPoint  = %9ld ** %10.4lf %10.4lf %10.4lf",nextPoint,pointAddrP(dtmP,nextPoint)->x,pointAddrP(dtmP,nextPoint)->y,pointAddrP(dtmP,nextPoint)->z) ;
                  }

//              Draw Pond About Low Point

                if( loadFlag )
                  {
                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"***** Determining Pond About %8ld %8ld %8ld",savePnt1,savePnt2,savePnt3) ;

//                 Determine Pond About Low Point

                   if( savePnt2 == dtmP->nullPnt )
                     {
                      if( nextPnt2 == dtmP->nullPnt && nextPnt3 == dtmP->nullPnt )
                        {
                         if( bcdtmDrainage_determinePondAboutLowPointDtmObject(dtmP,NULL,NULL,NULL,loadFunctionP,savePnt1,true,false,&exitPoint,&priorPoint,&nextPoint,&polygonP, userP) ) goto errexit ;
                         if( polygonP   != NULL ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
                        }
                      else if( nextPnt2 != dtmP->nullPnt && nextPnt3 == dtmP->nullPnt )
                        {
//                       if( bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(dtmP,savePnt1,nextPnt2,(DTMFeatureCallback)loadFunctionP,true,false,&exitPoint,&priorPoint,&nextPoint,&sumpLinesP,&numSumpLines,&polygonP, userP) ) goto errexit ;
                         if( sumpLinesP != NULL )  { free(sumpLinesP) ; sumpLinesP = NULL ; }
                         if( polygonP   != NULL ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
                        }
                      else if( nextPnt2 != dtmP->nullPnt && nextPnt3 != dtmP->nullPnt )
                        {
                         if( bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject(dtmP,savePnt1,nextPnt2,nextPnt3,loadFunctionP,1,0,&exitPoint,&priorPoint,&nextPoint,userP) ) goto errexit ;
                        }
                     }

//                 Determine Pond About Zero Slope Triangle

                   else if( pointAddrP(dtmP,savePnt1)->z == pointAddrP(dtmP,savePnt2)->z  && pointAddrP(dtmP,savePnt1)->z == pointAddrP(dtmP,savePnt3)->z )
                     {
                      if( bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject(dtmP,savePnt1,savePnt2,savePnt3,loadFunctionP,1,0,&exitPoint,&priorPoint,&nextPoint,userP) ) goto errexit ;
                     }

//                 Determine Pond About Zero Slope Sump Line

                   else
                     {
                      if( bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(dtmP,nullptr,nullptr,loadFunctionP,savePnt1,savePnt2,true,false,&exitPoint,&priorPoint,&nextPoint,&sumpLinesP,&numSumpLines,&polygonP, userP, &area) ) goto errexit ;
                      if( sumpLinesP != NULL )  { free(sumpLinesP) ; sumpLinesP = NULL ; }
                      if( polygonP   != NULL ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
                     }
                  }

//              Check If Exit Point Pond Can Be Processed

                if( pointAddrP(dtmP,exitPoint)->z == startZ || ( falseLowDepth > 0.0  && pointAddrP(dtmP,exitPoint)->z - startZ <= falseLowDepth ))
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Checking If Exit Point Can Be Processesd") ;

//                 Check If Exit Point Has Been Previously Processed

                   previousExit = 0 ;

                   if( tracePoints.CheckForPriorExitPoint(exitPoint , dtmP->nullPnt) ) previousExit = 1 ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"Previous Exit = %2ld",previousExit) ;

//                 Continue Tracing From Exit Point

                   if( ! previousExit )
                     {

//                    Check For Exit Point On A Hull Boundary

                      bcdtmList_testForHullPointDtmObject(dtmP,exitPoint,&hullFlag) ;

//                    Continue Tracing If Exit Point Not On Hull Boundary

                      if( ! hullFlag )
                        {
                         if( bcdtmDrainage_traceMaximumDescentFromPondExitPointDtmObject(dtmP,drainageTablesP,priorPoint,exitPoint,nextPoint,pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process)) goto errexit ;
                         if( dbg == 2 ) bcdtmWrite_message(0,0,0,"process from pond exit point ** exitPoint = %9ld nextPnt1 = %9ld nextPnt2 = %9ld nextX = %12.5lf nextY = %12.5lf nextZ = %10.4lf",exitPoint,nextPnt1,nextPnt2,nextX,nextY,nextZ) ;
			             if( process )
				           {

//                          Mark Exit Point

                            tracePoints.StoreTracePoint(nextX,nextY,nextZ,nextPnt1,nextPnt2) ;

//                          Load Coordinates For Drawing Downstream Trace

                            if( loadFlag )
                              {
                               if( streamPoints.StorePointInCache(pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,pointAddrP(dtmP,exitPoint)->z)) goto errexit ;
                               if( streamPoints.StorePointInCache(nextX,nextY,nextZ)) goto errexit ;
			                  }

//                          Set Parameters For Next Maximum Descent Trace

                            startX = nextX ;
                            startY = nextY ;
                            startZ = nextZ ;
                            pnt1   = nextPnt1 ;
                            pnt2   = nextPnt2 ;
                            pnt3   = nextPnt3 ;
                            lastPoint = exitPoint ;
                            lastAngle = bcdtmMath_getAngle(pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,nextX,nextY) ;
                            saveLastAngle = lastAngle ;
				           }
                        }
                     }
                  }
               }
            }
         } while ( process ) ;
      }
   }
/*
** Log Trace Points
*/
 if( dbg == 1 ) streamPoints.LogCachePoints() ;

// Set Low Point Values

 *lowPoint1P = pnt1 ;
 *lowPoint2P = pnt2 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"lowPoint1 = %6ld lowPoint2 = %6ld",*lowPoint1P,*lowPoint2P) ;

// Clean Up

 cleanup :
 if( sumpLinesP != NULL ) { free(sumpLinesP) ; sumpLinesP = NULL ; }
 if( polygonP   != NULL ) bcdtmPolygon_deletePolygonObject(&polygonP) ;

//  Return

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing To Low Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing To Low Point Error") ;
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
int bcdtmDrainage_traceMaximumDescentFromTriangleEdgeDtmObject
(
 BC_DTM_OBJ *dtmP,                   // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP, // ==> Pointer To Drainage Tables
 int    zeroSlopeOption,             // ==> Zero Slope Option
 long   isFalseLow,                  // ==> Is False Low Processing
 double lastAngle,                   // ==> Last Trace Angle
 long   startPnt1,                   // ==> Point 1 Of Triangle Edge
 long   startPnt2,                   // ==> Point 2 Of Triangle Edge
 long   startPnt3,                   // ==> Apex Point Of Triangle From Triangle Edge
 double startX,                      // ==> X Coordinate Of Trace Start On Triangle Edge
 double startY,                      // ==> Y Coordinate Of Trace Start On Triangle Edge
 long   *nextPnt1P,                  // <== Point 1 Of Next Triangle Edge
 long   *nextPnt2P,                  // <== Point 2 Of Next Triangle Edge
 long   *nextPnt3P,                  // <== Next Apex Point Of Triangle From Next Triangle Edge
 double *nextXP,                     // <== Next Trace Point X Coordiante
 double *nextYP,                     // <== Next Trace Point Y Coordiante
 double *nextZP,                     // <== Next Trace Point Z Coordiante
 long   *tracePointFoundP,           // <== Next Trace Point Found
 long   *exitPointP,                 // <== Pond Exit Point Of Pond If Next Trace Point Not Found
 long   *priorPointP,                // <== Prior Pond Point To Pond Exit Point
 long   *nextPointP                  // <== Next Pond Point From Pond Exit Point
)
/*
** This Function Traces The Maximum Descent From A Triangle Edge
**
** Notes :-
**
** 1. Triangle <startPnt1,startPnt2,startPnt3> Has An Anti Clockwise Direction
** 2. Triangle <nextPnt1P,nextPnt2P,nextPnt3P> Has An Anti Clockwise Direction
** 3. If the next trace point is not found and false low processing is invoked then a pond
**    is placed around the triangle edge.
** 4. Tracing then recommences from the pond exit point.
** 5. If a pond is created then the pond prior,exit and next points have an anti clockwise sense.
**
*/
{
 int    ret=DTM_SUCCESS,sdof,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 int    flowDirection,exitPnt,priorPnt,nextPnt ;
 long   intPnt,hullLine,numSumpLines=0 ;
 double dx,dy,xRad,yRad,area,radius,descentAngle,ascentAngle,slope,zeroSlopeTriangle ;
 DTM_SUMP_LINES *sumpLinesP=NULL ;
 DTM_POLYGON_OBJ *polygonP=NULL ;
 long useTables=0 ;
 bool trgFound=false,voidTriangle=false ;
 DTMFeatureCallback callBackP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Triangle Edge") ;
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP    = %p",drainageTablesP)  ;
    bcdtmWrite_message(0,0,0,"zeroSlopeOption    = %8d",zeroSlopeOption)  ;
    bcdtmWrite_message(0,0,0,"lastAngle          = %12.5lf",lastAngle)  ;
    bcdtmWrite_message(0,0,0,"isFalseLow         = %8ld",isFalseLow) ;
    bcdtmWrite_message(0,0,0,"startPnt1          = %8ld ** %10.4lf %10.4lf %10.4lf",startPnt1,pointAddrP(dtmP,startPnt1)->x,pointAddrP(dtmP,startPnt1)->y,pointAddrP(dtmP,startPnt1)->z) ;
    bcdtmWrite_message(0,0,0,"startPnt2          = %8ld ** %10.4lf %10.4lf %10.4lf",startPnt2,pointAddrP(dtmP,startPnt2)->x,pointAddrP(dtmP,startPnt2)->y,pointAddrP(dtmP,startPnt2)->z) ;
    bcdtmWrite_message(0,0,0,"startPnt3          = %8ld ** %10.4lf %10.4lf %10.4lf",startPnt3,pointAddrP(dtmP,startPnt3)->x,pointAddrP(dtmP,startPnt3)->y,pointAddrP(dtmP,startPnt3)->z) ;
    bcdtmWrite_message(0,0,0,"startX             = %12.5lf",startX) ;
    bcdtmWrite_message(0,0,0,"startY             = %12.5lf",startY) ;
   }

// Initialise

 *tracePointFoundP = 0 ;
 *nextPnt1P   = dtmP->nullPnt ;
 *nextPnt2P   = dtmP->nullPnt ;
 *nextPnt3P   = dtmP->nullPnt ;
 *exitPointP  = dtmP->nullPnt ;
 *nextPointP  = dtmP->nullPnt ;
 *priorPointP = dtmP->nullPnt ;
 *nextXP      = 0.0           ;
 *nextYP      = 0.0           ;
 *nextZP      = 0.0           ;
 dx = dtmP->xMax - dtmP->xMin ;
 dy = dtmP->yMax - dtmP->yMin ;

//  Check For Termination On A Hull Edge

 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Hull Line") ;
 if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,startPnt1,startPnt2,&hullLine)) goto errexit ;
 if( dbg && hullLine ) bcdtmWrite_message(0,0,0,"Hull Line Detected") ;

//  Only Process If Start Edge Is Not A Hull Line

 if( ! hullLine )
   {

//  Check For Valid Triangle

   if( cdbg )
     {
     if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Triangle %8ld %8ld %8ld",startPnt1,startPnt3,startPnt2) ;
     if( bcdtmList_checkForValidTriangleDtmObject(dtmP,startPnt1,startPnt3,startPnt2))
         {
         bcdtmWrite_message(1,0,0,"Invalid Trace From Triangle Edge Triangle") ;
         goto errexit ;
         }
     }

//  Check For Zero Slope Triangle

    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Zero Slope Triangle") ;
    zeroSlopeTriangle = 0 ;
    if(pointAddrP(dtmP,startPnt1)->z == pointAddrP(dtmP,startPnt2)->z  && pointAddrP(dtmP,startPnt1)->z == pointAddrP(dtmP,startPnt3)->z ) zeroSlopeTriangle = 1 ;

//  Flow Over Zero Slope Triangle

    if( zeroSlopeTriangle )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Triangle From Triangle Edge") ;
       if( zeroSlopeOption == 1 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"** Tracing At Last Angle") ;

           // Check Last Angle has Been Initialised

          if( lastAngle == -99.99 )
            {
             lastAngle = bcdtmMath_getPointAngleDtmObject(dtmP,startPnt1,startPnt2) + DTM_2PYE / 4.0 ;
             while ( lastAngle > DTM_2PYE ) lastAngle = lastAngle - DTM_2PYE ;
            }
          if( bcdtmDrainage_calculateAngleIntersectOfRadialFromTriangleEdgeWithTriangleDtmObject(dtmP,startPnt1,startPnt2,startPnt3,startX,startY,lastAngle,nextXP,nextYP,nextZP,nextPnt1P,nextPnt2P,nextPnt3P) ) goto errexit ;
         }
       else
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"** Placing Pond Over Zero Slope Triangle") ;
          if( bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject(dtmP,startPnt1,startPnt3,startPnt2,callBackP,0,0,exitPointP,priorPointP,nextPointP,NULL)) goto errexit ;
         }
      }

//  Flow From Edge

    else
      {

      // Get Flow Direction For Triangle Edge

       if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,startPnt1,startPnt2,startPnt3,voidTriangle,flowDirection)) goto errexit ;
       if( dbg )
         {
          if( flowDirection ==  1 ) bcdtmWrite_message(0,0,0,"Flow Direction Towards Edge") ;
          if( flowDirection ==  0 ) bcdtmWrite_message(0,0,0,"Flow Direction Parallel To Edge") ;
          if( flowDirection == -1 ) bcdtmWrite_message(0,0,0,"Flow Direction Away From Edge") ;
         }
/*
**     Flow Direction Is Towards Edge
*/
       if( flowDirection >= 0  )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Sump Line Detected") ;
/*
**        Zero Slope Sump Line
*/
          if( pointAddrP(dtmP,startPnt1)->z == pointAddrP(dtmP,startPnt2)->z )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Sump Line Detected") ;
             if( isFalseLow )
               {
                if( drainageTablesP != NULL && drainageTablesP->SizeOfZeroSlopeLinePondTable() > 0 )
                  {
                   drainageTablesP->FindZeroSlopeLinePond(startPnt1,startPnt2,exitPnt,priorPnt,nextPnt) ;
                   *exitPointP  = exitPnt ;
                   *priorPointP = priorPnt ;
                   *nextPointP  = nextPnt  ;
                  }
                else
                  {
                   if( bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(dtmP,NULL,NULL,NULL,startPnt1,startPnt2,false,false,exitPointP,priorPointP,nextPointP,&sumpLinesP,&numSumpLines,&polygonP, NULL,&area)) goto errexit ;
                   if( sumpLinesP != NULL ) { free(sumpLinesP) ; sumpLinesP = NULL ; }
                   if( polygonP   != NULL ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
                  }
               }
/*
**          Get Exit Point From Zero Sump Lines ** Added 8/1/2007
*/
	         else
		       {
               if( dbg ) bcdtmWrite_message(0,0,0,"Determining Pond About Zero Slope Sump Line") ;
//                if( bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(dtmP,startPnt1,startPnt2,NULL,0,0,exitPointP,priorPointP,nextPointP,&sumpLinesP,&numSumpLines,&polygonP, NULL)) goto errexit ;
                if( sumpLinesP != NULL ) { free(sumpLinesP) ; sumpLinesP = NULL ; }
                if( polygonP   != NULL ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
		        if( *exitPointP != dtmP->nullPnt )
			      {
                   if( pointAddrP(dtmP,*exitPointP)->z == pointAddrP(dtmP,startPnt1)->z )
			         {
                      *nextPnt1P = *exitPointP ;
                      *nextXP = pointAddrP(dtmP,*exitPointP)->x ;
                      *nextYP = pointAddrP(dtmP,*exitPointP)->y ;
                      *nextZP = pointAddrP(dtmP,*exitPointP)->z ;
                      *tracePointFoundP = 1 ;
			         }
			      }
		       }
            }
/*
**        StartPnt1 Lowest Sump Line Point
*/
          else if( pointAddrP(dtmP,startPnt1)->z <  pointAddrP(dtmP,startPnt2)->z )
            {
             *nextPnt1P = startPnt1 ;
             *nextXP = pointAddrP(dtmP,startPnt1)->x ;
             *nextYP = pointAddrP(dtmP,startPnt1)->y ;
             *nextZP = pointAddrP(dtmP,startPnt1)->z ;
             *tracePointFoundP = 1 ;
            }
/*
**        StartPnt2 Lowest Sump Line Point
*/
          else if( pointAddrP(dtmP,startPnt2)->z <  pointAddrP(dtmP,startPnt1)->z )
            {
             *nextPnt1P = startPnt2 ;
             *nextXP = pointAddrP(dtmP,startPnt2)->x ;
             *nextYP = pointAddrP(dtmP,startPnt2)->y ;
             *nextZP = pointAddrP(dtmP,startPnt2)->z ;
             *tracePointFoundP = 1 ;
            }
         }
/*
**      Flow Direction Is Away From Edge
*/
       else
         {
          if( bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP,drainageTablesP,startPnt1,startPnt3,startPnt2,voidTriangle,slope,descentAngle,ascentAngle) != DTM_SUCCESS ) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"slope = %8.4lf ascentAngle = %12.10lf descentAngle = %12.10lf",slope,ascentAngle,descentAngle) ;
/*
**        Calculate Radial Out From Start X And Start Y At Descent Angle
*/
          radius = sqrt(dx * dx + dy * dy) ;
          xRad = startX + radius * cos(descentAngle) ;
          yRad = startY + radius * sin(descentAngle) ;
/*
**        Determine Triangle Flow Out Edge
*/
          *tracePointFoundP = 1 ;
          sdof = bcdtmMath_sideOf(startX,startY,xRad,yRad,pointAddrP(dtmP,startPnt3)->x,pointAddrP(dtmP,startPnt3)->y) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Flow Out Edge Side Of = %2d",sdof) ;
/*
**        Flow Passes Through P3
*/
          if( sdof == 0 )
            {
             *nextPnt1P = startPnt3 ;
             *nextXP = pointAddrP(dtmP,startPnt3)->x ;
             *nextYP = pointAddrP(dtmP,startPnt3)->y ;
             *nextZP = pointAddrP(dtmP,startPnt3)->z ;
            }
/*
**        Flow Intersects Edge P2-P3
*/
          else if( sdof >  0 )
            {
             if( bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP,startX,startY,xRad,yRad,startPnt3,startPnt2,nextXP,nextYP,nextZP,&intPnt)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"intPnt = %10ld",intPnt) ;
             if( intPnt != dtmP->nullPnt )
               {
                *nextPnt1P = intPnt ;
                *nextXP = pointAddrP(dtmP,*nextPnt1P)->x ;
                *nextYP = pointAddrP(dtmP,*nextPnt1P)->y ;
                *nextZP = pointAddrP(dtmP,*nextPnt1P)->z ;
               }
             else
               {
                *nextPnt1P = startPnt3 ;
                *nextPnt2P = startPnt2 ;
                if(( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,*nextPnt1P,*nextPnt2P)) < 0 ) goto errexit ;
               }
            }
/*
**        Flow Intersects Edge P1-P3
*/
          else if( sdof <  0 )
            {
             if( bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP,startX,startY,xRad,yRad,startPnt1,startPnt3,nextXP,nextYP,nextZP,&intPnt)) goto errexit ;
             if( intPnt != dtmP->nullPnt )
               {
                *nextPnt1P = intPnt ;
                *nextXP = pointAddrP(dtmP,*nextPnt1P)->x ;
                *nextYP = pointAddrP(dtmP,*nextPnt1P)->y ;
                *nextZP = pointAddrP(dtmP,*nextPnt1P)->z ;
               }
             else
               {
                *nextPnt1P = startPnt1 ;
                *nextPnt2P = startPnt3 ;
                if(( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,*nextPnt1P,*nextPnt2P)) < 0 ) goto errexit ;
               }
            }
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
 if( sumpLinesP != NULL ) { free(sumpLinesP) ; sumpLinesP = NULL ; }
 if( polygonP   != NULL ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Triangle Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Triangle Edge Error") ;
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
int bcdtmDrainage_traceMaximumDescentFromTrianglePointDtmObject
(
 BC_DTM_OBJ *dtmP,                   // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP, // ==> Pointer To Drainage Tables
 int    zeroSlopeOption,             // ==> Zero Slope Option
 long   isFalseLow,                  // ==> Is False Low Processing
 double lastAngle,                   // ==> Last Descent Angle
 long   lastPnt,                     // ==> Last Descent Point
 long   startPnt,                    // ==> Triangle Point To Start Trace From
 double startX,                      // ==> X Coordinate Of Triangle Point To Start Trace
 double startY,                      // ==> Y Coordinate Of Triangle Point To Start Trace
 long   *nextPnt1P,                  // <== Point 1 Of Next Triangle Edge
 long   *nextPnt2P,                  // <== Point 2 Of Next Triangle Edge
 long   *nextPnt3P,                  // <== Point 3 Of Next Triangle Edge
 double *nextXP,                     // <== Next Trace Point X Coordiante
 double *nextYP,                     // <== Next Trace Point Y Coordinate
 double *nextZP,                     // <== Next Trace Point Z Coordinate
 long   *processP,                   // <== Next Trace Point Found
 long   *exitPointP,                 // <== Pond Exit Point Of Pond If Next Trace Point Not Found
 long   *priorPointP,                // <== Prior Pond Point To Pond Exit Point
 long   *nextPointP                  // <== Next Pond Point To Pond Exit Point
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   intPnt,hullPoint,descentType,descentPnt1,descentPnt2,numSumpLines  ;
 int    exitPnt,priorPnt,nextPnt ;
 double area,descentAngle,descentSlope ;
 DTM_SUMP_LINES   *sumpLinesP=NULL ;
 DTM_POLYGON_OBJ *polygonP=NULL ;
 static long loop=0 ;
 DTMFeatureCallback callBackP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Triangle Point") ;
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP    = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"zeroSlopeOption    = %8d",zeroSlopeOption) ;
    bcdtmWrite_message(0,0,0,"isFalseLow         = %8ld",isFalseLow) ;
    bcdtmWrite_message(0,0,0,"lastAngle          = %8.6lf",lastAngle) ;
    bcdtmWrite_message(0,0,0,"lastPnt            = %8ld",lastPnt) ;
    bcdtmWrite_message(0,0,0,"startPnt           = %8ld",startPnt) ;
    bcdtmWrite_message(0,0,0,"startX             = %12.5lf",startX) ;
    bcdtmWrite_message(0,0,0,"startY             = %12.5lf",startY) ;
    bcdtmWrite_message(0,0,0,"startZ             = %12.5lf",pointAddrP(dtmP,startPnt)->z)  ;
   }
/*
** Initialise Variables
*/
 *processP = 0 ;
 *nextXP = 0.0 ;
 *nextYP = 0.0 ;
 *nextZP = 0.0 ;   ;
 *nextPnt1P = dtmP->nullPnt ;
 *nextPnt2P = dtmP->nullPnt ;
 *nextPnt3P = dtmP->nullPnt ;
 *exitPointP  = dtmP->nullPnt ;
 *priorPointP = dtmP->nullPnt ;
 *nextPointP  = dtmP->nullPnt ;
/*
** Scan Point For Maximum Descent
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent") ;
 if( bcdtmDrainage_scanPointForMaximumDescentDtmObject(dtmP,drainageTablesP,startPnt,lastPnt,&descentType,&descentPnt1,&descentPnt2,&descentSlope,&descentAngle)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"descentType = %2ld descentSlope = %8.3lf descentAngle = %12.10lf descentPnt1 = %9ld descentPnt2 = %9ld",descentType,descentSlope,descentAngle,descentPnt1,descentPnt2) ;
/*
**  Place Pond About Low Point
*/
 if( descentType ==  0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Placing Pond About Low Point") ;
    if( bcdtmList_checkForPointOnHullLineDtmObject(dtmP,startPnt,&hullPoint)) goto errexit ;
    if( ! hullPoint )
      {
       if( drainageTablesP != NULL && drainageTablesP->SizeOfLowPointPondTable() > 0 )
         {
          drainageTablesP->FindLowPointPond(startPnt,exitPnt,priorPnt,nextPnt) ;
          *exitPointP  = exitPnt ;
          *priorPointP = priorPnt ;
          *nextPointP  = nextPnt ;
         }
       else
         {
          if( bcdtmDrainage_determinePondAboutLowPointDtmObject(dtmP,drainageTablesP,NULL,NULL,NULL,startPnt,false,false,exitPointP,priorPointP,nextPointP,&polygonP, NULL)) goto errexit ;
         }
       if( *exitPointP == dtmP->nullPnt )
         {
          bcdtmWrite_message(1,0,0,"Low Point Pond Not Determined") ;
          goto errexit ;
         }
      }
   }
/*
** Determine Pond About Zero Slope Sump Line
*/
 if( descentType == 1 && descentSlope == 0.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Sump Line From Triangle Point") ;
    if( zeroSlopeOption == 1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Tracing Down Zero Sump Line") ;
       *nextPnt1P = descentPnt1 ;
       *nextXP    = pointAddrP(dtmP,descentPnt1)->x ;
       *nextYP    = pointAddrP(dtmP,descentPnt1)->y ;
       *nextZP    = pointAddrP(dtmP,descentPnt1)->z ;
       *processP  = 1 ;
      }
    else
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Placing Pond About Zero Slope Sump Line") ;
       if( drainageTablesP != NULL && drainageTablesP->SizeOfZeroSlopeLinePondTable() > 0 )
         {
          drainageTablesP->FindZeroSlopeLinePond(startPnt,descentPnt1,exitPnt,priorPnt,nextPnt) ;
          *exitPointP  = exitPnt ;
          *priorPointP = priorPnt ;
          *nextPointP  = nextPnt ;
         }
       else
         {
          if( bcdtmDrainage_determinePondAboutZeroSlopeSumpLineDtmObject(dtmP,drainageTablesP,NULL,NULL,startPnt,descentPnt1,false,false,exitPointP,priorPointP,nextPointP,&sumpLinesP,&numSumpLines,&polygonP,NULL,&area) ) goto errexit ;
          if( sumpLinesP != NULL )  { free(sumpLinesP) ; sumpLinesP = NULL ; }
          if( polygonP   != NULL ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
         }
       if( *exitPointP == dtmP->nullPnt )
         {
          bcdtmWrite_message(1,0,0,"Zero Sump Line Pond Not Determined") ;
          goto errexit ;
         }
      }
   }
/*
** Flow Over Zero Slope Triangle
*/
 if( descentType == 2 && descentSlope == 0.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Triangle From Triangle Point") ;
    if( zeroSlopeOption == 1 )
      {
       if( dbg )bcdtmWrite_message(0,0,0,"** Tracing At Last Angle") ;

       // Check Last Angle has Been Initialised

       if( lastAngle == -99.99 )
         {
          lastAngle = bcdtmMath_getAngle(startX,startY,(pointAddrP(dtmP,descentPnt1)->x+pointAddrP(dtmP,descentPnt2)->x)/2.0,(pointAddrP(dtmP,descentPnt1)->y+pointAddrP(dtmP,descentPnt2)->y)/2.0) ;
         }
       if( bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP,startPnt,descentPnt1,descentPnt2,lastAngle,nextXP,nextYP,nextZP,&intPnt)) goto errexit ;
       if( intPnt != dtmP->nullPnt ) *nextPnt1P = intPnt ;
       else
         {
          *nextPnt1P = descentPnt1 ;
          *nextPnt2P = descentPnt2 ;
          if( ( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,descentPnt1,descentPnt2)) < 0 ) goto errexit ;
         }
       *processP = 1 ;
      }
    else
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"** Placing Pond About Zero Slope Triangle") ;
       if( bcdtmDrainage_determinePondAboutZeroSlopeTriangleDtmObject(dtmP,startPnt,descentPnt1,descentPnt2,callBackP,0,0,exitPointP,priorPointP,nextPointP,NULL)) goto errexit ;
       *nextPnt1P = startPnt ;
       *nextPnt2P = descentPnt1 ;
       *nextPnt3P = descentPnt2 ;
      }
   }
/*
**  Maximum Descent Is Down A Sump Line
*/
 if( descentType == 1 && descentSlope != 0.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Descent Down A Sump Line") ;
    *nextPnt1P = descentPnt1 ;
    *nextXP = pointAddrP(dtmP,descentPnt1)->x ;
    *nextYP = pointAddrP(dtmP,descentPnt1)->y ;
    *nextZP = pointAddrP(dtmP,descentPnt1)->z ;
    *processP = 1 ;
   }
/*
**  Maximum Descent Is Down A Triangle Face
*/
 if( descentType == 2 && descentSlope != 0.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Descent Down A Triangle Face") ;
    if( bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP,startPnt,descentPnt1,descentPnt2,descentAngle,nextXP,nextYP,nextZP,&intPnt)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"intPnt = %9ld ** X = %12.5lf Y = %12.5lf Z = %10.4lf ** intAngle = %12.10lf",intPnt,*nextXP,*nextYP,*nextZP,bcdtmMath_getAngle(startX,startY,*nextXP,*nextYP)) ;
    if( intPnt != dtmP->nullPnt ) *nextPnt1P = intPnt ;
    else
      {
       *nextPnt1P = descentPnt1 ;
       *nextPnt2P = descentPnt2 ;
       if( ( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,descentPnt1,descentPnt2)) < 0 ) goto errexit ;
      }
    *processP = 1 ;
   }
/*
** Clean Up
*/
 cleanup :
 if( sumpLinesP != NULL )  { free(sumpLinesP) ; sumpLinesP = NULL ; }
 if( polygonP   != NULL ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Triangle Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Triangle Point Error") ;
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
int bcdtmDrainage_traceMaximumDescentFromPondExitPointDtmObject
(
 BC_DTM_OBJ        *dtmP,                  // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP,       // ==> Pointer To Drainage Tables
 long              priorPnt,               // ==> Prior Point On Pond Boundary
 long              exitPnt,                // ==> Exit  Point On Pond Boundary
 long              nextPnt,                // ==> Next  Point On Pond Boundary
 double            startX,                 // ==> X Coordinate Of Triangle Point To Start Trace
 double            startY,                 // ==> Y Coordinate Of Triangle Point To Start Trace
 long              *nextPnt1P,             // <== Point 1 Of Next Triangle Edge
 long              *nextPnt2P,             // <== Point 2 Of Next Triangle Edge
 long              *nextPnt3P,             // <== Point 3 Of Next Triangle Edge
 double            *nextXP,                // <== Next Trace Point X Coordinate
 double            *nextYP,                // <== Next Trace Point Y Coordinate
 double            *nextZP,                // <== Next Trace Point Z Coordinate
 long              *processP               // <== Next Trace Point Found
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   intPnt,descentType=0,descentPnt1=0,descentPnt2=0  ;
 double descentAngle=0.0,descentSlope=0.0 ;
 long useTables=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Pond Exit Point") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %pld",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"priorPnt        = %8ld",priorPnt) ;
    bcdtmWrite_message(0,0,0,"exitPnt         = %8ld",exitPnt) ;
    bcdtmWrite_message(0,0,0,"startX          = %12.5lf",startX) ;
    bcdtmWrite_message(0,0,0,"startY          = %12.5lf",startY) ;
   }
/*
** Initialise Variables
*/
 *processP = 0 ;
 *nextXP = 0.0 ;
 *nextYP = 0.0 ;
 *nextZP = 0.0 ;   ;
 *nextPnt1P = dtmP->nullPnt ;
 *nextPnt2P = dtmP->nullPnt ;
 *nextPnt3P = dtmP->nullPnt ;
/*
** Range Scan Point For Maximum Descent
*/
// if( bcdtmDrainage_rangeScanPointForMaximumDescentDtmObject(dtmP,useTables,exitPnt,priorPnt,nextPnt,&descentType,&descentPnt1,&descentPnt2,&descentSlope,&descentAngle)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"descentType = %2ld descentSlope = %8.3lf descentAngle = %12.10lf descentPnt1 = %9ld descentPnt2 = %9ld",descentType,descentSlope,descentAngle,descentPnt1,descentPnt2) ;
/*
** Check Descent Slope Is Not Zero
*/
 if( descentSlope == 0.0  ) descentType = 0 ;
/*
**  Maximum Descent Is Down A Sump Line
*/
 if( descentType == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Descent Down A Sump Line") ;
    *nextPnt1P = descentPnt1 ;
    *nextXP = pointAddrP(dtmP,descentPnt1)->x ;
    *nextYP = pointAddrP(dtmP,descentPnt1)->y ;
    *nextZP = pointAddrP(dtmP,descentPnt1)->z ;
    *processP = 1 ;
   }
/*
**  Maximum Descent Is Down A Triangle Face
*/
 if( descentType == 2 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Descent Down A Triangle Face") ;
    if( bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP,exitPnt,descentPnt1,descentPnt2,descentAngle,nextXP,nextYP,nextZP,&intPnt)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"intPnt = %9ld ** X = %12.5lf Y = %12.5lf Z = %10.4lf ** intAngle = %12.10lf",intPnt,*nextXP,*nextYP,*nextZP,bcdtmMath_getAngle(startX,startY,*nextXP,*nextYP)) ;
    if( intPnt != dtmP->nullPnt ) *nextPnt1P = intPnt ;
    else
      {
       *nextPnt1P = descentPnt1 ;
       *nextPnt2P = descentPnt2 ;
       if( ( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,descentPnt1,descentPnt2)) < 0 ) goto errexit ;
      }
    *processP = 1 ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Triangle Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Triangle Point Error") ;
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
int bcdtmDrainage_checkForSumpOrRidgeLineDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To DTM Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              linePoint1,                 // ==> Line End Point
 long              linePoint2,                 // ==> Line End Point
 long              antPoint,                   // ==> Next Point Anti Clockwise From linePoint2 About linePoint1
 long              clkPoint,                   // ==> Next Point Clockwise From linePoint2 About linePoint1
 DTMFeatureType    *lineTypeP                  // <== Line Type <DTMFeatureType::SumpLine,DTMFeatureType::RidgeLine>
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 int    antFlow=0,clkFlow=0,antFlat=0,clkFlat=0 ;
 bool   antVoid=true,clkVoid=true ;
/*
** Write Entry Message
*/
 if( dbg  )
   {
    bcdtmWrite_message(0,0,0,"Checking For Sump Or Ridge Line") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP)         ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %8ld",drainageTablesP)  ;
    bcdtmWrite_message(0,0,0,"linePoint1      = %8ld",linePoint1) ;
    bcdtmWrite_message(0,0,0,"linePoint2      = %8ld",linePoint2) ;
    bcdtmWrite_message(0,0,0,"antPoint        = %8ld",antPoint)   ;
    bcdtmWrite_message(0,0,0,"clkPoint        = %8ld",clkPoint)   ;
   }
/*
** Initialise
*/
 *lineTypeP = DTMFeatureType::None ;
/*
** Calculate Flow Direction For Anti Clockwise None Void Valid Triangle
*/
 if( nodeAddrP(dtmP,linePoint2)->hPtr != linePoint1 )
   {
    if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,linePoint1,linePoint2,antPoint,antVoid,antFlow)) goto errexit ;
    if( pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,linePoint2)->z && pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,antPoint)->z ) antFlat = 1 ;
   }
/*
**  Calculate Flow Direction For Clockwise None Void Valid Triangle
*/
 if( nodeAddrP(dtmP,linePoint1)->hPtr != linePoint2 )
   {
    if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,linePoint2,linePoint1,clkPoint,clkVoid,clkFlow)) goto errexit ;
    if( pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,linePoint2)->z && pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,clkPoint)->z ) clkFlat = 1 ;
   }
/*
** Write Flow Direcitions
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"antFlow = %2ld clkFlow = %2ld ** antFlat = %2ld clkFlat = %2ld",antFlow,clkFlow,antFlat,clkFlat) ;
/*
**   Check For Sump Line
*/
  if     ( ( antFlow  >= 0 && clkFlow >= 0 ) || ( ! antFlat && ! clkFlat && ! antFlow && ! clkFlow && pointAddrP(dtmP,linePoint1)->z > pointAddrP(dtmP,linePoint2)->z) )  *lineTypeP = DTMFeatureType::SumpLine ;
//  if     (  antFlow  > 0 && clkFlow > 0 )   *lineTypeP = DTMFeatureType::SumpLine ;
/*
**   Check For Ridge Line
*/
//  else if( ( antFlow  <= 0 && clkFlow <= 0 ) && ( ! antFlat && ! clkFlat ) )  *lineTypeP = DTMFeatureType::RidgeLine ;
  else if( antFlow  < 0 && clkFlow < 0 )  *lineTypeP = DTMFeatureType::RidgeLine ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Sump Or Ridge Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Sump Or Ridge Line Error") ;
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
int bcdtmDrainage_scanBetweenPointsForMaximumDescentDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Point To Scan About
 long              startPoint,                 // ==> Start Point Of Scan
 long              endPoint,                   // ==> End Point Of Scan
 long              *descentTypeP,              // <== Descent Type < 1 Sump, 2 Triangle>
 long              *descentPnt1P,              // <== Pnt1 Of Descent Feature
 long              *descentPnt2P,              // <== Pnt2 Of Descent Feature
 double            *descentSlopeP,             // <== Descent Slope
 double            *descentAngleP              // <== Descent Angle
)
/*
** This Function Scans About Point In ClockWise Direction
** From startPoint To endPoint Looking For The Maximum descent
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sumpPnt,trgPnt1,trgPnt2;
 double sumpSlope,trgSlope,sumpDescentAngle=0.0,trgDescentAngle ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Descent") ;
/*
** Initialise
*/
 *descentTypeP  = 0 ;
 *descentSlopeP = 0.0 ;
 *descentAngleP = 0.0 ;
 *descentPnt1P  = dtmP->nullPnt ;
 *descentPnt2P  = dtmP->nullPnt ;
/*
** Validate Point
*/
 if( point < 0 || point >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
**  Get Maximum Descent Sump Line
*/
 if( bcdtmDrainage_scanBetweenPointsForMaximumDescentSumpLineDtmObject(dtmP,drainageTablesP,point,startPoint,endPoint,&sumpPnt,&sumpSlope,&sumpDescentAngle) ) goto errexit ;
/*
**  Get Maximum Descent Triangle
*/
 if( bcdtmDrainage_scanBetweenPointsForMaximumDescentTriangleDtmObject(dtmP,drainageTablesP,point,startPoint,endPoint,&trgPnt1,&trgPnt2,&trgDescentAngle,&trgSlope) ) goto errexit ;
/*
** Maximum Descent Down A Sump Line
*/
 if( sumpPnt != dtmP->nullPnt && trgPnt1 == dtmP->nullPnt )
   {
    *descentTypeP  = 1 ;
    *descentPnt1P  = sumpPnt ;
    *descentSlopeP = sumpSlope ;
    *descentAngleP = sumpDescentAngle ;
   }
/*
** Maximum Descent Down A Triangle
*/
 if( sumpPnt == dtmP->nullPnt && trgPnt1 != dtmP->nullPnt )
   {
    *descentTypeP  = 2 ;
    *descentPnt1P  = trgPnt1 ;
    *descentPnt2P  = trgPnt2 ;
    *descentSlopeP = trgSlope ;
    *descentAngleP = trgDescentAngle ;
   }
/*
** Maximum Descents Down A Sump Lines And Triangle
*/
  if( sumpPnt != dtmP->nullPnt && trgPnt1 != dtmP->nullPnt )
   {
    if( sumpSlope <= trgSlope )
      {
       *descentTypeP  = 1 ;
       *descentPnt1P  = sumpPnt ;
       *descentSlopeP = sumpSlope ;
       *descentAngleP = sumpDescentAngle ;
      }
    else
      {
       *descentTypeP  = 2 ;
       *descentPnt1P  = trgPnt1 ;
       *descentPnt2P  = trgPnt2 ;
       *descentSlopeP = trgSlope ;
       *descentAngleP = trgDescentAngle ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Descent Completed") ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Descent Error") ;
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
int bcdtmDrainage_scanBetweenPointsForMaximumDescentSumpLineDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Point To Scan About
 long              startPoint,                 // ==> Start Point Of Scan
 long              endPoint,                   // ==> End Point Of Scan
 long              *sumpPointP,                // <== Sump Line End Point
 double            *sumpSlopeP,                // <== Sump Line Slope
 double            *sumpAngleP                 // <== Sump Line Angle
)
/*
** This Function Scans In A Clockwise Direction Between Points For The Maximum Descent Sump Line
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   scanPnt, antPnt, clkPnt;
 DTMFeatureType lineType;
 double dx,dy,dz,dd,slope ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Descent Sump Line") ;
/*
** Initialise
*/
 *sumpSlopeP = 0.0 ;
 *sumpAngleP = 0.0 ;
 *sumpPointP = dtmP->nullPnt ;
/*
** Validate Point
*/
 if( point < 0 || point >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
** Scan Around point And Look For Sump
*/
 antPnt = startPoint ;
 if(( scanPnt = bcdtmList_nextClkDtmObject(dtmP,point,antPnt)) < 0 ) goto errexit ;
 while ( scanPnt != endPoint )
   {
    if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point,scanPnt)) < 0 ) goto errexit ;
    if(nodeAddrP(dtmP,point)->hPtr != antPnt && nodeAddrP(dtmP,point)->hPtr != scanPnt && nodeAddrP(dtmP,scanPnt)->hPtr != point && nodeAddrP(dtmP,clkPnt)->hPtr != point  )
      {
/*
**     Check For Descent
*/
       if( pointAddrP(dtmP,point)->z >= pointAddrP(dtmP,scanPnt)->z )
         {
          if( bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP,drainageTablesP,point,scanPnt,antPnt,clkPnt,&lineType)) goto errexit ;
          if( lineType == DTMFeatureType::SumpLine )
            {
             dx = pointAddrP(dtmP,scanPnt)->x - pointAddrP(dtmP,point)->x ;
             dy = pointAddrP(dtmP,scanPnt)->y - pointAddrP(dtmP,point)->y ;
             dz = pointAddrP(dtmP,scanPnt)->z - pointAddrP(dtmP,point)->z ;
             dd = sqrt(dx*dx + dy*dy) ;
             slope = dz/dd ;
             if     ( *sumpPointP == dtmP->nullPnt ) { *sumpPointP = scanPnt ; *sumpSlopeP = slope ; }
             else if( slope       <  *sumpSlopeP   ) { *sumpPointP = scanPnt ; *sumpSlopeP = slope ; }
            }
         }
      }
    antPnt  = scanPnt ;
    scanPnt = clkPnt ;
   }
/*
** Calculate Sump Line Angle
*/
 if( *sumpPointP != dtmP->nullPnt ) *sumpAngleP = bcdtmMath_getPointAngleDtmObject(dtmP,point,*sumpPointP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Descent Sump Line Completed") ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Descent Sump Line Error") ;
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
int bcdtmDrainage_scanBetweenPointsForMaximumDescentTriangleDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Point To Scan About
 long              startPoint,                 // ==> Start Point Of Scan
 long              endPoint,                   // ==> End Point Of Scan
 long              *trgPnt1P,                  // <== Triangle Base Point 1
 long              *trgPnt2P,                  // <== Triangle Base Point 2
 double            *trgDescentAngleP,          // <== Maximum Descent Angle
 double            *trgSlopeP                  // <== Maximum Descent Slope
)
/*
** This Function Scans In A Clockwise Direction Between Points For The Maximum Descent Triangle
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   scanPnt,nextPnt ;
 double a1,a2,a3,angScanPnt,angNextPnt,slope,ascentAngle,descentAngle  ;
 bool   voidTriangle ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Descent Triangle") ;
/*
** Initialise
*/
 *trgPnt1P = dtmP->nullPnt ;
 *trgPnt2P = dtmP->nullPnt ;
 *trgDescentAngleP = 0.0 ;
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

//     Get Triangle Slope And Descent Angle

       if( bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP,drainageTablesP,point,scanPnt,nextPnt,voidTriangle,slope,descentAngle,ascentAngle) != DTM_SUCCESS ) goto errexit ;

//     Only Process For None Void Triangles

       if( voidTriangle == false )
         {

          if( slope > 0.0 ) slope = -slope ;
/*
**        Set Maximum Descent triangle
*/
          a1 = angScanPnt ;
          a2 = descentAngle ;
          a3 = angNextPnt ;
          if( a1 < a3 ) a1 = a1 + DTM_2PYE ;
          if( a2 < a3 ) a2 = a2 + DTM_2PYE ;
          if( a2 <= a1 && a2 >= a3 )
            {
             if( *trgPnt1P == dtmP->nullPnt || slope < *trgSlopeP )
               {
                *trgPnt1P  = scanPnt ;
                *trgPnt2P  = nextPnt ;
                *trgSlopeP = slope   ;
                *trgDescentAngleP = descentAngle ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Descent Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Between Points For Maximum Descent Triangle Error") ;
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
int bcdtmDrainage_traceMaximumAscentDtmObject
(
 BC_DTM_OBJ         *dtmP,               // ==> Pointer To Dtm Object
 DTMDrainageTables  *drainageTablesP,    // ==> Pointer To Drainage Tables
 DTMFeatureCallback loadFunctionP,       // ==> Pointer To Load Function
 double             falseHighElevation,  // ==> False High Elevation
 double             startX,              // ==> Start X Coordinate
 double             startY,              // ==> Start Y Coordinate
 void               *userP               // ==> User Pointer Passed Back To User
)
/*
** This Function Traces The Maximum Ascent From Point(startX,startY)
** To Inhibit False High Processing Set falseHighElevation to zero
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   trgPnt1=0,trgPnt2=0,trgPnt3=0,highPnt1=0,highPnt2=0,pointType=0,pntInVoid=0 ;
 long   useTables=0,traceOverZeroSlope=1,loadFlag=1,startType=3 ;
 double startZ,highX,highY,highZ ;
/*
** Write Entry Message
*/
falseHighElevation = 0 ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent") ;
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"drainageTablesP    = %p",drainageTablesP)  ;
    bcdtmWrite_message(0,0,0,"loadFunctionP      = %p",loadFunctionP)  ;
    bcdtmWrite_message(0,0,0,"falseHighElevation = %12.5lf",falseHighElevation) ;
    bcdtmWrite_message(0,0,0,"startX             = %12.5lf",startX)  ;
    bcdtmWrite_message(0,0,0,"startY             = %12.5lf",startY)  ;
    bcdtmWrite_message(0,0,0,"userP              = %p",userP)  ;
   }
/*
** Initialise
*/
 if( falseHighElevation < 0.0 ) falseHighElevation = 0.0 ;
/*
** Test For Valid DTM Object
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
** Find Triangle Containing Start Point On TIN
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,startX,startY,&startZ,&pointType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
 if( pointType == 0 )
   {
    bcdtmWrite_message(1,0,0,"Maximum Ascent Start Point External To Tin") ;
    goto errexit ;
   }
/*
** Check Point To Point Tolerance
*/
 if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) <= dtmP->ppTol ) { pointType = 1 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
 if( trgPnt2 != dtmP->nullPnt ) if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) <= dtmP->ppTol ) { pointType = 1 ; trgPnt1 = trgPnt2 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
 if( trgPnt3 != dtmP->nullPnt ) if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y) <= dtmP->ppTol ) { pointType = 1 ; trgPnt1 = trgPnt3 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
/*
** Test For Point In Void
*/
 pntInVoid = 0 ;
 if( pointType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,trgPnt1)->PCWD) ) pntInVoid = 1 ;
 if( pointType == 2 || pointType == 3 )  if( bcdtmList_testForVoidLineDtmObject(dtmP,trgPnt1,trgPnt2,&pntInVoid)) goto errexit ;
 if( pointType == 4 ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3,&pntInVoid)) goto errexit ;
 if( pntInVoid )
   {
    bcdtmWrite_message(1,0,0,"Maximum Ascent Start Point In Void") ;
    goto errexit ;
   }
/*
** Test For Zero Slope Triangle
*/
 if( trgPnt2 != dtmP->nullPnt && trgPnt3 != dtmP->nullPnt )
   {
    if( pointAddrP(dtmP,trgPnt1)->z == pointAddrP(dtmP,trgPnt2)->z && pointAddrP(dtmP,trgPnt1)->z == pointAddrP(dtmP,trgPnt3)->z )
      {
       bcdtmWrite_message(1,0,0,"Maximum Ascent Start Point On Zero Slope Triangle") ;
       goto errexit ;
      }
   }
 if( trgPnt3 != dtmP->nullPnt ) if( bcdtmMath_distance(startX,startY,pointAddrP(dtmP,trgPnt3)->x,pointAddrP(dtmP,trgPnt3)->y) <= dtmP->ppTol ) { trgPnt1 = trgPnt3 ; trgPnt2 = trgPnt3 = dtmP->nullPnt ; }
/*
** Set Triangle Anti Clockwise
*/
 if( trgPnt2 != dtmP->nullPnt && trgPnt3 != dtmP->nullPnt )
   {
    if( bcdtmMath_pointSideOfDtmObject(dtmP,trgPnt1,trgPnt2,trgPnt3) < 0 ) { pointType = trgPnt2 ; trgPnt2 = trgPnt3 ; trgPnt3 = pointType ; }
   }
 else if ( trgPnt2 != dtmP->nullPnt && trgPnt3 == dtmP->nullPnt )
   {
    if(( pointType = bcdtmList_nextAntDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ;
    if( ! bcdtmList_testLineDtmObject(dtmP,pointType,trgPnt2))
      {
       if(( pointType = bcdtmList_nextClkDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ;
       if( ! bcdtmList_testLineDtmObject(dtmP,pointType,trgPnt2) ) goto errexit ;
       trgPnt3 = pointType ; pointType = trgPnt1 ; trgPnt1 = trgPnt2 ; trgPnt2 = pointType ;
      }
   }
/*
** Set Start Type
*/
 if( trgPnt1 != dtmP->nullPnt && trgPnt2 == dtmP->nullPnt && trgPnt3 == dtmP->nullPnt ) startType = 1 ;
 if( trgPnt1 != dtmP->nullPnt && trgPnt2 != dtmP->nullPnt && trgPnt3 == dtmP->nullPnt ) startType = 2 ;
 if( trgPnt1 != dtmP->nullPnt && trgPnt2 != dtmP->nullPnt && trgPnt3 != dtmP->nullPnt ) startType = 3 ;
/*
** Start Tracing
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Commencing Maximun Ascent Trace To High Point") ;
 if( bcdtmDrainage_traceToHighPointDtmObject(dtmP,drainageTablesP,loadFunctionP,falseHighElevation,traceOverZeroSlope,loadFlag,startType,trgPnt1,trgPnt2,trgPnt3,startX,startY,startZ,userP,&highPnt1,&highPnt2,&highX,&highY,&highZ) ) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Climbs To %9ld %9ld",highPnt1,highPnt2) ;
    if( highPnt1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"highPnt1 = %6ld highPnt1->FPTR = %9ld ** %10.4lf %10.4lf %10.4lf",highPnt1,nodeAddrP(dtmP,highPnt1)->hPtr,pointAddrP(dtmP,highPnt1)->x,pointAddrP(dtmP,highPnt1)->y,pointAddrP(dtmP,highPnt1)->z ) ;
    if( highPnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"highPnt2 = %6ld highPnt2->FPTR = %9ld ** %10.4lf %10.4lf %10.4lf",highPnt2,nodeAddrP(dtmP,highPnt2)->hPtr,pointAddrP(dtmP,highPnt2)->x,pointAddrP(dtmP,highPnt2)->y,pointAddrP(dtmP,highPnt2)->z ) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent Error") ;
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
int bcdtmDrainage_traceToHighPointDtmObject
(
 BC_DTM_OBJ         *dtmP,                      // ==> Pointer To DTM Object
 DTMDrainageTables  *drainageTablesP,           // ==> Pointer To Drainage Tables
 DTMFeatureCallback loadFunctionP,              // ==> Pointer To Load Function
 double             falseHighElevation,         // ==> False High Elevation
 long               traceOverZeroSlope,         // ==> Trace Over Zero Slopes
 long               loadFlag,                   // ==> Load Trace To High Point
 long               trgStartType,               // ==> Triangle Start Type
 long               pnt1,                       // ==> Start Triangle Point 1
 long               pnt2,                       // ==> Start Triangle Point 2
 long               pnt3,                       // ==> Start Triangle Point 3
 double             X,                          // ==> Start X Coordinate
 double             Y,                          // ==> Start Y Coordinate
 double             Z,                          // ==> Start Z Coordinate
 void               *userP,                     // ==> User Pointer Passed Back To User
 long               *highPnt1P,                 // <== High Point 1
 long               *highPnt2P,                 // <== High Point 2 If Zero Slope Ridge
 double             *highXP,                    // <== High Point X Coordinate
 double             *highYP,                    // <== High Point Y Coordinate
 double             *highZP                     // <== High Point Z Coordinate
)
/*
**
** This Function Traces To A High Point
**
** Notes :-
**
** 1. Start Triangle pnt1-pnt2-pnt3 must be anti clockwise
** 2. trgStartType == 1  From A Triangle Point  pnt1
**                 == 2  From A Triangle Edge   pnt1-pnt2  on  pnt3 side of pnt1-pnt2
**                 == 3  From Inside A Triangle
** 3. False High Elevation Processing Not Yet Implemented
**
*/
{
 int            ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long           nextPnt1,nextPnt2,nextPnt3,tracePointFound,lastPoint,isFalseHigh=FALSE,startPointType ;
 double         firstX,firstY,firstZ,startX,startY,startZ,nextX,nextY,nextZ ;
 double         lastAngle,saveLastAngle ;
 DTMPointCache  streamPoints ;

 long useTables=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing To High Point") ;
    bcdtmWrite_message(0,0,0,"dtmP                        = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"drainageTablesP             = %p",drainageTablesP)  ;
    bcdtmWrite_message(0,0,0,"loadFunctionP               = %p",loadFunctionP)  ;
    bcdtmWrite_message(0,0,0,"falseHighElevation          = %8.4lf",falseHighElevation)  ;
    bcdtmWrite_message(0,0,0,"traceOverZeroSlope          = %8ld",traceOverZeroSlope)  ;
    bcdtmWrite_message(0,0,0,"loadFlag                    = %8ld",loadFlag)  ;
    bcdtmWrite_message(0,0,0,"trgStartType                = %8ld",trgStartType) ;
    bcdtmWrite_message(0,0,0,"pnt1                        = %8ld",pnt1)  ;
    bcdtmWrite_message(0,0,0,"pnt2                        = %8ld",pnt2)  ;
    bcdtmWrite_message(0,0,0,"pnt3                        = %8ld",pnt3)  ;
    bcdtmWrite_message(0,0,0,"X                           = %15.5lf",X)  ;
    bcdtmWrite_message(0,0,0,"Y                           = %15.5lf",Y)  ;
    bcdtmWrite_message(0,0,0,"Z                           = %15.5lf",Z)  ;
    bcdtmWrite_message(0,0,0,"highPnt1P                   = %8ld",*highPnt1P)  ;
    bcdtmWrite_message(0,0,0,"highPnt2P                   = %8ld",*highPnt2P)  ;
   }
/*
** Set High Point Return Values
*/
 *highXP = 0.0 ;
 *highYP = 0.0 ;
 *highZP = 0.0 ;
 *highPnt1P = dtmP->nullPnt ;
 *highPnt2P = dtmP->nullPnt ;
/*
** Check And Set Triangle Points Anti Clockwise
*/
 if( pnt1 != dtmP->nullPnt && pnt2 != dtmP->nullPnt && pnt3 != dtmP->nullPnt )
   {
    if( bcdtmMath_pointSideOfDtmObject(dtmP,pnt1,pnt2,pnt3) < 0 )
      {
       lastPoint = pnt1 ;
       pnt1 = pnt2 ;
       pnt2 = lastPoint ;
      }
   }
/*
** Validate Start Type
*/
 if( trgStartType < 1 || trgStartType > 3 )
   {
    bcdtmWrite_message(2,0,0,"Invalid Trace To High Point Start Type") ;
    goto errexit ;
   }
/*
** Initialise
*/
 firstX = startX = X ;
 firstY = startY = Y ;
 firstZ = startZ = Z ;
 lastPoint = dtmP->nullPnt ;
 lastAngle = -DTM_2PYE ;
 saveLastAngle = lastAngle ;
 if( falseHighElevation < 0.0 ) falseHighElevation = 0.0 ;
 if( falseHighElevation > 0.0 ) isFalseHigh = TRUE ;
/*
** Store Start Point
*/
 if( loadFlag )
   {
    if( streamPoints.StorePointInCache(startX,startY,startZ)) goto errexit ;
   }
/*
** Get Drainage Trace Start
*/
 if( bcdtmDrainage_startTraceDtmObject(dtmP,1,pnt1,pnt2,pnt3,startX,startY,startZ,&startPointType,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&lastAngle)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"StartPointType = %2ld ** nextPnt1 = %10ld nextPnt2 = %10ld nextPnt3 = %10ld",startPointType,nextPnt1,nextPnt2,nextPnt3) ;
    bcdtmWrite_message(0,0,0,"nextX = %12.5lf nextY = %12.5lf nextZ = %12.5lf",nextX,nextY,nextZ) ;
   }
/*
** Only Process For A Valid Start Point
*/
 if( startPointType >= 1 && startPointType <= 3 )
   {
    if( startPointType == 2 )   // Coincident With Triangle Edge
      {
       pnt1 = nextPnt1 ;
       pnt2 = nextPnt2 ;
       pnt3 = nextPnt3 ;
      }
    if( startPointType == 3 )   // Internal To Triangle
      {
       pnt1 = nextPnt1 ;
       pnt2 = nextPnt2 ;
       pnt3 = nextPnt3 ;
       startX = nextX  ;
       startY = nextY  ;
       startZ = nextZ  ;
       saveLastAngle = lastAngle ;
       if( loadFlag )
         {
          if( streamPoints.StorePointInCache(startX,startY,startZ)) goto errexit ;
         }
      }
/*
** Iteratively Get Next Maximum Ascent Point
*/
    tracePointFound = 1 ;
    while ( tracePointFound )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Tracing To Next High Point From %10ld %10ld %10ld",pnt1,pnt2,pnt3) ;
       if( pnt2 != dtmP->nullPnt ){ if( bcdtmDrainage_traceMaximumAscentFromTriangleEdgeDtmObject(dtmP,drainageTablesP,isFalseHigh,traceOverZeroSlope,lastAngle,pnt1,pnt2,pnt3,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&tracePointFound)) goto errexit ; }
       else                       { if( bcdtmDrainage_traceMaximumAscentFromTrianglePointDtmObject(dtmP,drainageTablesP,isFalseHigh,traceOverZeroSlope,lastAngle,lastPoint,pnt1,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&tracePointFound) ) goto errexit ; }
       if( dbg ) bcdtmWrite_message(0,0,0,"tracePointFound = %2ld ** %11ld %11ld %11ld ** nextX = %15.5lf nextY = %15.5lf nextZ = %15.5lf",tracePointFound,nextPnt1,nextPnt2,nextPnt3,nextX,nextY,nextZ) ;
/*
**     Check Point On Zero SLope Ridge Line Has Not Been Prior Processed
*/
       if( tracePointFound && nextZ == startZ && nextPnt2 == dtmP->nullPnt )
         {
          if( streamPoints.CheckIfPointInCache(nextX,nextY,nextZ)) tracePointFound = 0 ;
         }
/*
**     Set For Next Trace Point
*/
       if( tracePointFound )
         {
          if( pnt2 == dtmP->nullPnt ) lastPoint = pnt1 ;
          else                        lastPoint = dtmP->nullPnt ;
          pnt1 = nextPnt1 ;
          pnt2 = nextPnt2 ;
          pnt3 = nextPnt3 ;
          if( startX != nextX || startY != nextY ) lastAngle = bcdtmMath_getAngle(startX,startY,nextX,nextY) ;
          else                                     lastAngle = saveLastAngle ;
          saveLastAngle = lastAngle ;
          startX = nextX  ;
          startY = nextY  ;
          startZ = nextZ  ;
          if( loadFlag )
            {
             if( streamPoints.StorePointInCache(startX,startY,startZ)) goto errexit ;
            }
          if( startX == firstX && startY == firstY ) tracePointFound = 0 ;
         }
      }
   }
/*
** Load Feature
*/
 if( loadFunctionP != NULL )
   {
    if( streamPoints.CallUserDelegateWithCachePoints( (DTMFeatureCallback)loadFunctionP,DTMFeatureType::AscentTrace,dtmP->nullUserTag,dtmP->nullFeatureId,userP)) goto errexit ;
   }
/*
** Set Point Return Values
*/
 *highXP = startX ;
 *highYP = startY ;
 *highZP = startZ ;
 *highPnt1P = pnt1 ;
 *highPnt2P = pnt2 ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Tracing To High Point Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Tracing To High Point Error") ;
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
int bcdtmDrainage_traceMaximumAscentFromTriangleEdgeDtmObject
(
 BC_DTM_OBJ        *dtmP,
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              isFalseHigh,
 long              traceOverZeroSlope,
 double            lastAngle,
 long              pnt1,
 long              pnt2,
 long              pnt3,
 double            startX,
 double            startY,
 long              *nextPnt1P,
 long              *nextPnt2P,
 long              *nextPnt3P,
 double            *nextXP,
 double            *nextYP,
 double            *nextZP,
 long              *tracePointFoundP
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   hullLine,zeroSlopeTriangle=0,numAscentLines,edgeType,intPnt ;
 double dx,dy,radX,radY,radius,descentAngle=0.0,ascentAngle=0.0,slope ;
 int    sdof,flowDirection ;
 DTM_ASCENT_LINE *ascentLinesP=NULL;
 bool voidTriangle=false ;
 long useTables=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Triangle Edge") ;
    bcdtmWrite_message(0,0,0,"pnt1 = %9ld ** %12.5lf %12.5lf %10.4lf",pnt1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
    bcdtmWrite_message(0,0,0,"pnt2 = %9ld ** %12.5lf %12.5lf %10.4lf",pnt2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z) ;
    bcdtmWrite_message(0,0,0,"pnt3 = %9ld ** %12.5lf %12.5lf %10.4lf",pnt3,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt3)->z) ;
   }
/*
** Initialise Variables
*/
 *tracePointFoundP  = 0 ;
 *nextPnt1P = dtmP->nullPnt ;
 *nextPnt2P = dtmP->nullPnt ;
 *nextPnt3P = dtmP->nullPnt ;
 *nextXP  = 0.0 ;
 *nextYP  = 0.0 ;
 *nextZP  = 0.0 ;
 dx = dtmP->xMax - dtmP->xMin ;
 dy = dtmP->yMax - dtmP->yMin ;
/*
** Determine Edge Type
*/
 edgeType = 0 ;
/*
** Check For A DTM Hull Line
*/
 if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,pnt1,pnt2,&hullLine)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"hullLine = %2ld",hullLine) ;
 if( hullLine ) edgeType = 1 ;
/*
** Check For Zero Slope Triangle
*/
 else
   {
    zeroSlopeTriangle = 0 ;
    if( pointAddrP(dtmP,pnt1)->z == pointAddrP(dtmP,pnt2)->z && pointAddrP(dtmP,pnt1)->z == pointAddrP(dtmP,pnt3)->z) zeroSlopeTriangle = 1 ;
    if( zeroSlopeTriangle ) edgeType = 3 ;
/*
**  Check For A Ridge Line
*/
    else
      {
/*
**     Get Next Flow Direction
*/
       flowDirection = 0 ;

      // Get Flow Direction For Triangle Edge

       if( dbg ) bcdtmWrite_message(0,0,0,"Getting Flow Direction For Edge %10ld %10ld ** %10ld",pnt1,pnt2,pnt3) ;
       if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,pnt1,pnt2,pnt3,voidTriangle,flowDirection)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"flowDirection = %2ld",flowDirection) ;
       if( flowDirection <= 0 ) edgeType = 2 ;
       else                     edgeType = 4 ;
      }
   }
/*
**  Write Edge Type
*/
 if( dbg )
   {
    if( edgeType == 1 ) bcdtmWrite_message(0,0,0,"edgeType = hullLine") ;
    if( edgeType == 2 ) bcdtmWrite_message(0,0,0,"edgeType = ridge") ;
    if( edgeType == 3 ) bcdtmWrite_message(0,0,0,"edgeType = zeroSlopeTriangle") ;
    if( edgeType == 4 ) bcdtmWrite_message(0,0,0,"edgeType = ascentTriangle") ;
   }
/*
**  Process Edge Type
*/
  switch ( edgeType )
    {
     case  1 :               //  Hull Line
     break   ;

     case  2 :               // Ridge Line
/*
**   Ascent Ridge Line
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Ascent Ridge Line") ;
     if( pointAddrP(dtmP,pnt1)->z != pointAddrP(dtmP,pnt2)->z )
       {
        *tracePointFoundP = 1 ;
        if( pointAddrP(dtmP,pnt1)->z > pointAddrP(dtmP,pnt2)->z )
          {
           *nextPnt1P = pnt1 ;
           *nextXP = pointAddrP(dtmP,pnt1)->x ;
           *nextYP = pointAddrP(dtmP,pnt1)->y ;
           *nextZP = pointAddrP(dtmP,pnt1)->z ;
          }
        else
          {
           *nextPnt1P = pnt2 ;
           *nextXP = pointAddrP(dtmP,pnt2)->x ;
           *nextYP = pointAddrP(dtmP,pnt2)->y ;
           *nextZP = pointAddrP(dtmP,pnt2)->z ;
          }
       }
/*
**   Zero Slope Ridge Line
*/
     else
       {
        if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Ridge Line") ;
/*
**      Get Maximum Ascents For pnt1-pnt2
*/
        if( bcdtmDrainage_scanLineEndPointsForAscentLinesDtmObject(dtmP,drainageTablesP,pnt1,pnt2,&ascentLinesP,&numAscentLines)) goto errexit ;
        if( numAscentLines > 0 )
          {
           if( ascentLinesP->slope > 0.0 )
             {
              *tracePointFoundP = 1 ;
              *nextPnt1P = ascentLinesP->pnt1 ;
              *nextXP = pointAddrP(dtmP,ascentLinesP->pnt1)->x ;
              *nextYP = pointAddrP(dtmP,ascentLinesP->pnt1)->y ;
              *nextZP = pointAddrP(dtmP,ascentLinesP->pnt1)->z ;
             }
          }
        if( ascentLinesP != NULL ) { free(ascentLinesP) ; ascentLinesP = NULL ; }
	   }
     break   ;


     case  3 :               // Zero Slope Triangle

     if( traceOverZeroSlope && lastAngle != DTM_2PYE ) ascentAngle = lastAngle ;
     else     break   ;


     case  4 :               // Ascent Triangle
/*
**   Get Ascent Angle For Triangle
*/
     if( ! zeroSlopeTriangle )
       {
        if( dbg ) bcdtmWrite_message(0,0,0,"Getting Ascent Angle For Triangle %8ld %8ld %8ld",pnt1,pnt2,pnt3) ;
        if( bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP,drainageTablesP,pnt1,pnt3,pnt2,voidTriangle,slope,descentAngle,ascentAngle) != DTM_SUCCESS ) goto errexit ;
        if( dbg ) bcdtmWrite_message(0,0,0,"ascent angle = %12.10lf",ascentAngle) ;
       }
/*
**   Calculate Radial Out From startX,startY
*/
     radius = dx * dx + dy * dy ;
     radX = startX + radius * cos(ascentAngle) ;
     radY = startY + radius * sin(ascentAngle) ;
/*
**   Calculate Intercept Of Radial On pnt1-pnt3 Or pnt2-pnt3
*/
     *tracePointFoundP = 1 ;
     sdof = bcdtmMath_sideOf(startX,startY,radX,radY,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) ;
/*
**   Maximum Ascent Passes Throught pnt3
*/
     if( sdof == 0 )
       {
        if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Ascent Passes Through pnt3") ;
        *nextPnt1P = pnt3 ;
        *nextXP = pointAddrP(dtmP,pnt3)->x ;
        *nextYP = pointAddrP(dtmP,pnt3)->y ;
        *nextZP = pointAddrP(dtmP,pnt3)->z ;
       }
/*
**   Maximum Ascent Passes Throught pnt2-pnt3
*/
     if( sdof >  0 )
       {
        if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Ascent Passes Through pnt2-pnt3") ;
        if( bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP,startX,startY,radX,radY,pnt3,pnt2,nextXP,nextYP,nextZP,&intPnt)) goto errexit ;
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
**   Maximum Ascent Passes Throught pnt1-pnt3
*/
     if( sdof <  0 )
       {
        if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Ascent Passes Through pnt1-pnt3") ;
        if( bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP,startX,startY,radX,radY,pnt1,pnt3,nextXP,nextYP,nextZP,&intPnt)) goto errexit ;
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
     break   ;

     default :
     break   ;
    } ;
/*
** Clean Up
*/
 cleanup :
 if( ascentLinesP != NULL ) { free(ascentLinesP) ; ascentLinesP = NULL ; }
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Triangle Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Triangle Edge Error") ;
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
int bcdtmDrainage_traceMaximumAscentFromTrianglePointDtmObject
(
 BC_DTM_OBJ *dtmP,                    // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP,  // ==> Pointer To Drainage Tables
 long   isFalseHigh,                  // ==> Is False High Processing
 long   traceOverZeroSlope,           // ==> Trace Over Zero Slope
 double lastAngle,                    // ==> Last Ascent Angle
 long   lastPnt,                      // ==> Last Ascent Point
 long   startPnt,                     // ==> Triangle Point To Start Trace From
 double startX,                       // ==> X Coordinate Of Triangle Point To Start Trace
 double startY,                       // ==> Y Coordinate Of Triangle Point To Start Trace
 long   *nextPnt1P,                   // <== Point 1 Of Next Triangle Edge
 long   *nextPnt2P,                   // <== Point 2 Of Next Triangle Edge
 long   *nextPnt3P,                   // <== Point 3 Of Next Triangle Edge
 double *nextXP,                      // <== Next Trace Point X Coordiante
 double *nextYP,                      // <== Next Trace Point Y Coordiante
 double *nextZP,                      // <== Next Trace Point Z Coordiante
 long   *tracePointFoundP             // <== Next Trace Point Found
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   intPnt,ascentType,ascentPnt1,ascentPnt2  ;
 double ascentAngle,ascentSlope ;
 long useTables=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Triangle Point") ;
    bcdtmWrite_message(0,0,0,"dtmP               = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP    = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"traceOverZeroSlope = %8ld",traceOverZeroSlope) ;
    bcdtmWrite_message(0,0,0,"isFalseHigh        = %8ld",isFalseHigh) ;
    bcdtmWrite_message(0,0,0,"lastAngle          = %8.6lf",lastAngle) ;
    bcdtmWrite_message(0,0,0,"lastPnt            = %8ld",lastPnt) ;
    bcdtmWrite_message(0,0,0,"startPnt           = %8ld",startPnt) ;
    bcdtmWrite_message(0,0,0,"startX             = %12.5lf",startX) ;
    bcdtmWrite_message(0,0,0,"startY             = %12.5lf",startY) ;
   }
/*
** Initialise Variables
*/
 *tracePointFoundP = 0 ;
 *nextXP = 0.0 ;
 *nextYP = 0.0 ;
 *nextZP = 0.0 ;   ;
 *nextPnt1P = dtmP->nullPnt ;
 *nextPnt2P = dtmP->nullPnt ;
 *nextPnt3P = dtmP->nullPnt ;
 /*
** Scan Point For Maximum Ascent
*/
 if( bcdtmDrainage_scanPointForMaximumAscentDtmObject(dtmP,drainageTablesP,startPnt,lastPnt,&ascentType,&ascentPnt1,&ascentPnt2,&ascentSlope,&ascentAngle)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"ascentType = %2ld ascentSlope = %8.3lf ascentAngle = %12.10lf ascentPnt1 = %9ld ascentPnt2 = %9ld",ascentType,ascentSlope,ascentAngle,ascentPnt1,ascentPnt2) ;
/*
** Only Process Zero Slope Ascent If Prior Flow To Point And Zero Slope Processing Is Invoked
*/
 if( lastAngle < 0.0 && ascentSlope == 0.0 ) ascentType = 0 ;
 if( ascentType && ascentSlope == 0.0 && ! traceOverZeroSlope ) ascentType = 0 ;
/*
** If Zero Slope Ascent Find Zero Slope Triangle Or Sump Line To Preserve Flow Direction
*/
 if( ascentType && ascentSlope == 0.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Finding Zero Slope Triangle To Trace Over") ;
    ascentType  = 2 ;
    ascentAngle = lastAngle ;
//    if( bcdtmDrainage_getAscentZeroSlopeTriangleToTraceOverDtmObject(dtmP,useTables,startPnt,lastAngle,&ascentPnt1,&ascentPnt2)) goto errexit ;
    if     ( ascentPnt1 == dtmP->nullPnt ) ascentType = 0 ;
    else if( ascentPnt2 == dtmP->nullPnt ) ascentType = 1 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope ** ascentType = %2ld ascentSlope = %8.3lf ascentAngle = %12.10lf ascentPnt1 = %9ld ascentPnt2 = %9ld",ascentType,ascentSlope,ascentAngle,ascentPnt1,ascentPnt2) ;
   }
/*
**  Maximum Ascent Is Up A Ridge Line
*/
 if( ascentType == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Ascent Up A Ridge Line") ;
    *nextPnt1P = ascentPnt1 ;
    *nextXP = pointAddrP(dtmP,ascentPnt1)->x ;
    *nextYP = pointAddrP(dtmP,ascentPnt1)->y ;
    *nextZP = pointAddrP(dtmP,ascentPnt1)->z ;
    *tracePointFoundP = 1 ;
   }
/*
**  Maximum Ascent Is Up A Triangle Face
*/
 if( ascentType == 2 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Maximum Ascent Up A Triangle Face") ;
    if( bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject(dtmP,startPnt,ascentPnt1,ascentPnt2,ascentAngle,nextXP,nextYP,nextZP,&intPnt)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"intPnt = %9ld ** X = %12.5lf Y = %12.5lf Z = %10.4lf ** intAngle = %12.10lf",intPnt,*nextXP,*nextYP,*nextZP,bcdtmMath_getAngle(startX,startY,*nextXP,*nextYP)) ;
    if( intPnt != dtmP->nullPnt ) *nextPnt1P = intPnt ;
    else
      {
       *nextPnt1P = ascentPnt1 ;
       *nextPnt2P = ascentPnt2 ;
       if( ( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,ascentPnt1,ascentPnt2)) < 0 ) goto errexit ;
      }
    *tracePointFoundP = 1 ;
   }
/*
**  If No Flow From Point Check For False High Point
*/
 if( ! *tracePointFoundP && isFalseHigh  )
   {
/*
**  False High Processing Not Yet Implemented
*/
    bcdtmWrite_message(2,0,0,"False High Processing Not Implemented") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Triangle Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Triangle Point Error") ;
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
int bcdtmDrainage_scanLineEndPointsForAscentLinesDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To DTM Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point1,                     // ==> Point One Of Zero Slope Line
 long              point2,                     // ==> Point Two Of Zero Slope Line
 DTM_ASCENT_LINE   **ascentLinesPP,            // <== Ascent Lines Sorted On Decreasing Slope
 long              *numAscentLinesP            // <== Number Of Ascent Lines
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long numPnt1Lines=0,numPnt2Lines=0 ;
 DTM_ASCENT_LINE *pnt1LinesP=NULL,*pnt2LinesP=NULL,*alineP;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scanning Line End Points For Ascent Lines") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"point1          = %8ld",point1) ;
    bcdtmWrite_message(0,0,0,"point2          = %8ld",point2) ;
    bcdtmWrite_message(0,0,0,"ascentLinesPP   = %p",*ascentLinesPP) ;
    bcdtmWrite_message(0,0,0,"numAscentLinesP = %8ld",*numAscentLinesP) ;
   }
/*
** Initialise
*/
 *numAscentLinesP = 0 ;
 if( *ascentLinesPP != NULL )
   {
    free(*ascentLinesPP) ;
    *ascentLinesPP = NULL ;
   }
/*
** Get Ascent Lines About Point 1
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Ascent Lines About Point1") ;
 if( bcdtmDrainage_scanPointForAscentLinesDtmObject(dtmP,drainageTablesP,point1,&pnt1LinesP,&numPnt1Lines)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Ascent Lines About Point = %6ld",numPnt1Lines) ;
    for( alineP = pnt1LinesP ; alineP < pnt1LinesP + numPnt1Lines ; ++alineP )
      {
       bcdtmWrite_message(0,0,0,"Point1 Line[%6ld] = %2ld pnt1 = %9ld pnt2 = %9ld slope = %10.4lf angle = %12.10lf",(long)(alineP-pnt1LinesP),alineP->ascentType,alineP->pnt1,alineP->pnt2,alineP->slope,alineP->ascentAngle) ;
      }
   }
/*
** Get Ascent Triangles About Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Ascent Triangles For Point2") ;
 if( bcdtmDrainage_scanPointForAscentTrianglesDtmObject(dtmP,drainageTablesP,point2,&pnt2LinesP,&numPnt2Lines)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Ascent Triangles About Point = %6ld",numPnt2Lines) ;
    for( alineP = pnt2LinesP ; alineP < pnt2LinesP + numPnt2Lines ; ++alineP )
      {
       bcdtmWrite_message(0,0,0,"Point2 Line[%6ld] = %2ld pnt1 = %9ld pnt2 = %9ld pnt3 = %8ld slope = %10.4lf angle = %12.10lf",(long)(alineP-pnt2LinesP),alineP->ascentType,alineP->pnt1,alineP->pnt2,alineP->pnt3,alineP->slope,alineP->ascentAngle) ;
      }
   }
/*
** Set Number Of Ascent Lines
*/
 *numAscentLinesP = numPnt1Lines + numPnt2Lines ;
/*
** Copy Ascent Lines To Return Array
*/
 if( *numAscentLinesP > 0 )
   {
/*
**  Allocate Memory To Store Ascent Lines
*/
    *ascentLinesPP = ( DTM_ASCENT_LINE *) malloc ( *numAscentLinesP * sizeof(DTM_ASCENT_LINE)) ;
    if( *ascentLinesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Copy Ridge Lines And Ascent Triangles
*/
    if( numPnt1Lines > 0 ) memcpy(*ascentLinesPP,pnt1LinesP,numPnt1Lines*sizeof(DTM_ASCENT_LINE)) ;
    if( numPnt2Lines > 0 ) memcpy(*ascentLinesPP+numPnt1Lines,pnt2LinesP,numPnt2Lines*sizeof(DTM_ASCENT_LINE)) ;
/*
**  Sort Ascent Lines On Descending Slope
*/
    qsort(*ascentLinesPP,*numAscentLinesP,sizeof(DTM_ASCENT_LINE),bcdtmDrainage_ascentLinesSlopeCompareFunction) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( pnt1LinesP != NULL ) free(pnt1LinesP) ;
 if( pnt2LinesP != NULL ) free(pnt2LinesP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Line End Points For Ascent Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Line End Points For Ascent Lines Error") ;
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
int bcdtmDrainage_ascentLinesSlopeCompareFunction(const void * void1P,const void * void2P)
/*
** Compare Function For Qsort Of Descending Slope Ascent Lines
*/
{
 DTM_ASCENT_LINE *aline1P,*aline2P ;
 aline1P = (DTM_ASCENT_LINE *)void1P ;
 aline2P = (DTM_ASCENT_LINE *)void2P ;
 if     (  aline1P->slope   <  aline2P->slope  ) return( 1) ;
 else if(  aline1P->slope   >  aline2P->slope  ) return(-1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_scanPointForMaximumAscentDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Point To Scan About
 long              excludePoint,               // ==> Exclude Point
 long              *ascentTypeP,               // <== Ascent Type < 1 Ridge, 2 Triangle>
 long              *ascentPnt1P,               // <== Pnt1 Of Ascent Feature
 long              *ascentPnt2P,               // <== Pnt2 Of Ascent Feature
 double            *ascentSlopeP,              // <== Ascent Slope
 double            *ascentAngleP               // <== Ascent Angle
)
/*
** This Function Scans A Point For Maximum Ascent
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ridgePnt,trgPnt1,trgPnt2 ;
 double ridgeSlope,trgSlope,ridgeAscentAngle=0.0,trgAscentAngle ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Ascent") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"point           = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"excludePoint    = %8ld",excludePoint) ;
   }
/*
** Initialise
*/
 *ascentTypeP  = 0 ;
 *ascentSlopeP = 0.0 ;
 *ascentAngleP = 0.0 ;
 *ascentPnt1P  = dtmP->nullPnt ;
 *ascentPnt2P  = dtmP->nullPnt ;
/*
** Validate Point
*/
 if( point < 0 || point >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
**  Get Maximum Ascent Ridge Line
*/
 if( bcdtmDrainage_scanPointForMaximumAscentRidgeLineDtmObject(dtmP,drainageTablesP,point,excludePoint,&ridgePnt,&ridgeSlope) ) goto errexit ;
 if( ridgePnt != dtmP->nullPnt ) ridgeAscentAngle = bcdtmMath_getPointAngleDtmObject(dtmP,point,ridgePnt) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"ridgePnt = %10ld ** ridgeSlope = %10.4lf ascentAngle = %10.12lf",ridgePnt,ridgeSlope,ridgeAscentAngle) ;
/*
**  Get Maximum Ascent Triangle
*/
 if( bcdtmDrainage_scanPointForMaximumAscentTriangleDtmObject(dtmP,drainageTablesP,point,excludePoint,&trgPnt1,&trgPnt2,&trgAscentAngle,&trgSlope) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"point = %10ld trgPnt1 = %10ld trgPnt2 = %10ld ** trgSlope = %10.4lf trgAngle = %10.12lf",point,trgPnt1,trgPnt2,trgSlope,trgAscentAngle) ;
/*
** Maximum Ascent Up A Ridge Line
*/
 if( ridgePnt != dtmP->nullPnt && trgPnt1 == dtmP->nullPnt )
   {
    *ascentTypeP  = 1 ;
    *ascentPnt1P  = ridgePnt ;
    *ascentSlopeP = ridgeSlope ;
    *ascentAngleP = ridgeAscentAngle ;
   }
/*
** Maximum Ascent Up A Triangle
*/
 if( ridgePnt == dtmP->nullPnt && trgPnt1 != dtmP->nullPnt )
   {
    *ascentTypeP  = 2 ;
    *ascentPnt1P  = trgPnt1 ;
    *ascentPnt2P  = trgPnt2 ;
    *ascentSlopeP = trgSlope ;
    *ascentAngleP = trgAscentAngle ;
   }
/*
** Maximum Ascents Up A Ridge Lines And Triangle
** If Both Ridge And Triangle Slopes Are The Same Value Set The Maximum
** Ascent To A Triangle
*/
  if( ridgePnt != dtmP->nullPnt && trgPnt1 != dtmP->nullPnt )
   {
    if( ridgeSlope >= trgSlope )
      {
       *ascentTypeP  = 1 ;
       *ascentPnt1P  = ridgePnt ;
       *ascentSlopeP = ridgeSlope ;
       *ascentAngleP = ridgeAscentAngle ;
      }
    else
      {
       *ascentTypeP  = 2 ;
       *ascentPnt1P  = trgPnt1 ;
       *ascentPnt2P  = trgPnt2 ;
       *ascentSlopeP = trgSlope ;
       *ascentAngleP = trgAscentAngle ;
      }
   }
/*
** Write Results
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"ascentType = %2ld ascentP1 = %10ld ascentPnt2 = %10ld ascentSlope = %10.4lf ascentAngle = %10.12lf",*ascentTypeP,*ascentPnt1P,*ascentPnt2P,*ascentSlopeP,*ascentAngleP) ;

/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Ascent Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Ascent Error") ;
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
int bcdtmDrainage_scanPointForMaximumAscentRidgeLineDtmObject
(
 BC_DTM_OBJ        *dtmP,               // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP,    // ==> Pointer To Drainage Tables
 long              point,               // ==> Point To Scan About
 long              excludePoint,        // ==> Exclude Point
 long              *ridgePointP,        // <== Ridge Line End Point
 double            *ridgeSlopeP         // <== Ridge Line Slope
)
/*
** This Function Scans A Point For The Maximum Ascent Ridge Line
*/
{
 int    ret=DTM_SUCCESS ;
 long   antPnt, scanPnt, clkPnt, clPtr;
 DTMFeatureType  lineType;
 double dx,dy,dz,dd,slope ;
/*
** Initialise
*/
 *ridgePointP = dtmP->nullPnt ;
 *ridgeSlopeP = 0.0 ;        ;
/*
** Scan Around Point For Ridge Lines
*/
 if( ( clPtr = nodeAddrP(dtmP,point)->cPtr) != dtmP->nullPtr )
   {
    scanPnt = clistAddrP(dtmP,clPtr)->pntNum ;
    if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,point,scanPnt)) < 0 ) goto errexit ;
    while ( clPtr != dtmP->nullPtr )
      {
       scanPnt  = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr    = clistAddrP(dtmP,clPtr)->nextPtr ;
       if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point,scanPnt)) < 0 ) goto errexit ;
       if(  scanPnt != excludePoint )
         {
          if( pointAddrP(dtmP,scanPnt)->z >= pointAddrP(dtmP,point)->z )
            {
             if( bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP,drainageTablesP,point,scanPnt,antPnt,clkPnt,&lineType)) goto errexit ;
             if( lineType == DTMFeatureType::RidgeLine )
               {
                dx = pointAddrP(dtmP,scanPnt)->x - pointAddrP(dtmP,point)->x ;
                dy = pointAddrP(dtmP,scanPnt)->y - pointAddrP(dtmP,point)->y ;
                dz = pointAddrP(dtmP,scanPnt)->z - pointAddrP(dtmP,point)->z ;
                dd = sqrt(dx*dx + dy*dy) ;
                slope = dz/dd ;
                if( *ridgePointP == dtmP->nullPnt || slope > *ridgeSlopeP )
                  {
                   *ridgePointP = scanPnt ;
                   *ridgeSlopeP = slope   ;
                  }
               }
            }
         }
       antPnt = scanPnt ;
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
int  bcdtmDrainage_scanPointForMaximumAscentTriangleDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Point To Scan About
 long              excludePoint,               // ==> exclude Point
 long              *trgBasePnt1P,              // <== Triangle Base Point 1
 long              *trgBasePnt2P,              // <== Triangle Base Point 2
 double            *trgAscentAngleP,           // <== Maximum Ascent Angle
 double            *trgSlopeP                  // <== Maximum Ascent Slope
)
/*
** This Function Scans The Triangles About A Point Looking For The Maximum Ascent Triangle
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   clPtr,antPnt,clkPnt ;
 double a1,a2,a3,angleAntPnt,angleClkPnt,slope,ascentAngle,descentAngle  ;
 bool   voidTriangle=false ;
/*
** Initialise
*/
 *trgBasePnt1P = dtmP->nullPnt ;
 *trgBasePnt2P = dtmP->nullPnt ;
 *trgAscentAngleP = 0.0 ;
 *trgSlopeP       = 0.0 ;
/*
** Scan Around point
*/
 clPtr = nodeAddrP(dtmP,point)->cPtr;
 if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit  ;
 angleAntPnt = bcdtmMath_getPointAngleDtmObject(dtmP,point,antPnt) ;
 while ( clPtr != dtmP->nullPtr )
   {
    clkPnt = clistAddrP(dtmP,clPtr)->pntNum ;
    clPtr  = clistAddrP(dtmP,clPtr)->nextPtr ;
    angleClkPnt = bcdtmMath_getPointAngleDtmObject(dtmP,point,clkPnt) ;
    if(  nodeAddrP(dtmP,point)->hPtr != antPnt && antPnt != excludePoint && clkPnt != excludePoint )
      {
       if( bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP,drainageTablesP,point,antPnt,clkPnt,voidTriangle,slope,descentAngle,ascentAngle) != DTM_SUCCESS ) goto errexit ;
       if( voidTriangle == false )
         {
/*
**        Determine If Ascent Angle Intersects Triangle Base
*/
          a1 = angleAntPnt ;
          a2 = ascentAngle ;
          a3 = angleClkPnt ;
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"**** antPnt = %8ld clkPnt = %8ld",antPnt,clkPnt) ;
             bcdtmWrite_message(0,0,0,"ascentAngle = %12.10lf",ascentAngle) ;
             bcdtmWrite_message(0,0,0,"ascentSlope = %12.10lf",slope) ;
             bcdtmWrite_message(0,0,0,"angleAntPnt = %12.10lf",angleAntPnt) ;
             bcdtmWrite_message(0,0,0,"angleClkPnt = %12.10lf",angleClkPnt) ;
            }
          if( a1 < a3 ) a1 = a1 + DTM_2PYE ;
          if( a2 < a3 ) a2 = a2 + DTM_2PYE ;
          if( a2 <= a1 && a2 >= a3 )
            {
             if( *trgBasePnt1P == dtmP->nullPnt || slope > *trgSlopeP )
               {
                *trgBasePnt1P = antPnt ;
                *trgBasePnt2P = clkPnt ;
                *trgAscentAngleP = ascentAngle ;
                *trgSlopeP = slope ;
               }
            }
         }
      }
/*
**  Reset For Next Triangle
*/
    antPnt = clkPnt ;
    angleAntPnt = angleClkPnt ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Exit
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
int bcdtmDrainage_scanPointForAscentLinesDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Point To Scan For Ascent Lines
 DTM_ASCENT_LINE   **ascentLinesPP,            // <== Pointer To Ascent Lines
 long              *numAscentLinesP            // <== Number Of Ascent Lines
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long numRidgeLines=0,numAscentTriangles=0 ;
 DTM_ASCENT_LINE *ridgeLinesP=NULL,*ascentTrianglesP=NULL,*alineP;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Lines") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"point           = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"ascentLinesPP   = %p",*ascentLinesPP) ;
    bcdtmWrite_message(0,0,0,"numAscentLinesP = %8ld",*numAscentLinesP) ;
    bcdtmWrite_message(0,0,0,"pointAddrP(dtmP,point)->x = %12.5lf",pointAddrP(dtmP,point)->x) ;
    bcdtmWrite_message(0,0,0,"pointAddrP(dtmP,point)->y = %12.5lf",pointAddrP(dtmP,point)->y) ;
    bcdtmWrite_message(0,0,0,"pointAddrP(dtmP,point)->z = %12.5lf",pointAddrP(dtmP,point)->z) ;
   }
/*
** Initialise
*/
 *numAscentLinesP = 0 ;
 if( *ascentLinesPP != NULL ) { free(*ascentLinesPP) ; *ascentLinesPP = NULL ; }
/*
** Get Ridge Lines About Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Ridge Lines") ;
 if( bcdtmDrainage_scanPointForAscentRidgeLinesDtmObject(dtmP,drainageTablesP,point,&ridgeLinesP,&numRidgeLines)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Ridge Lines About Point = %6ld",numRidgeLines) ;
    for( alineP = ridgeLinesP ; alineP < ridgeLinesP + numRidgeLines ; ++alineP )
      {
       bcdtmWrite_message(0,0,0,"Ridge Line[%6ld] = %2ld pnt1 = %9ld pnt2 = %9ld slope = %10.4lf angle = %12.10lf",(long)(alineP-ridgeLinesP),alineP->ascentType,alineP->pnt1,alineP->pnt2,alineP->slope,alineP->ascentAngle) ;
      }
   }
/*
** Get Ascent Triangles About Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Triangles") ;
 if( bcdtmDrainage_scanPointForAscentTrianglesDtmObject(dtmP,drainageTablesP,point,&ascentTrianglesP,&numAscentTriangles)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Ascent Triangles About Point = %6ld",numAscentTriangles) ;
    for( alineP = ascentTrianglesP ; alineP < ascentTrianglesP + numAscentTriangles ; ++alineP )
      {
       bcdtmWrite_message(0,0,0,"Ascent Triangle[%6ld] = %2ld pnt1 = %9ld pnt2 = %9ld pnt3 = %8ld slope = %10.4lf angle = %12.10lf",(long)(alineP-ascentTrianglesP),alineP->ascentType,alineP->pnt1,alineP->pnt2,alineP->pnt3,alineP->slope,alineP->ascentAngle) ;
      }
   }
/*
** Set Number Of Ascent Lines
*/
 *numAscentLinesP = numRidgeLines +  numAscentTriangles ;
/*
** Copy Ascent Lines To Return Array
*/
 if( *numAscentLinesP > 0 )
   {
/*
**  Allocate Memory To Store Ascent Lines
*/
    *ascentLinesPP = ( DTM_ASCENT_LINE *) malloc ( *numAscentLinesP * sizeof(DTM_ASCENT_LINE)) ;
    if( *ascentLinesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Copy Ridge Lines And Ascent Triangles
*/
    if( numRidgeLines      > 0 ) memcpy(*ascentLinesPP,ridgeLinesP,numRidgeLines*sizeof(DTM_ASCENT_LINE)) ;
    if( numAscentTriangles > 0 ) memcpy(*ascentLinesPP+numRidgeLines,ascentTrianglesP,numAscentTriangles*sizeof(DTM_ASCENT_LINE)) ;
/*
**  Sort Ascent Lines On Descending Slope
*/
    qsort(*ascentLinesPP,*numAscentLinesP,sizeof(DTM_ASCENT_LINE),bcdtmDrainage_ascentLinesSlopeCompareFunction) ;
   }
/*
**  Write Ascent Lines
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Ascent Lines = %6ld",*numAscentLinesP) ;
    for( alineP = *ascentLinesPP ; alineP < *ascentLinesPP + *numAscentLinesP ; ++alineP )
      {
       bcdtmWrite_message(0,0,0,"ascentLine[%6ld] ** Type = %2ld pnt1 = %9ld pnt2 = %9ld pnt3 = %9ld slope = %10.4lf angle = %12.10lf",(long)(alineP-*ascentLinesPP),alineP->ascentType,alineP->pnt1,alineP->pnt2,alineP->pnt3,alineP->slope,alineP->ascentAngle) ;
      }
    bcdtmList_writeCircularListForPointDtmObject(dtmP,point) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( ridgeLinesP      != NULL ) free(ridgeLinesP) ;
 if( ascentTrianglesP != NULL ) free(ascentTrianglesP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Lines Error") ;
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
int bcdtmDrainage_scanPointForAscentSumpLinesDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To DTM Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Point To Scan
 DTM_ASCENT_LINE   **ascentSumpLinesPP,        // <== Pointer To Sump Lines
 long              *numAscentSumpLinesP        // <== Number Of Sump Lines
 )
/*
** This Function Scans A Point For Ascent Sump Lines
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   antPnt,scanPnt,clkPnt,clPtr ;
 long   memAscentSumpLines=0,memAscentSumpLinesInc=100 ;
 double dx,dy,dz,dd,slope ;
 bool   clkVoid,antVoid ;
 int    clkFlow,antFlow ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Sump Lines") ;
    bcdtmWrite_message(0,0,0,"dtmP                 = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP      = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"point                = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"*ascentSumpLinesPP   = %p",*ascentSumpLinesPP) ;
    bcdtmWrite_message(0,0,0,"*numAscentSumpLinesP = %8ld",*numAscentSumpLinesP) ;
   }
/*
** Initialise
*/
 *numAscentSumpLinesP = 0 ;
 if( *ascentSumpLinesPP != NULL ) { free(*ascentSumpLinesPP) ; *ascentSumpLinesPP = NULL ; }
/*
** Scan Around Point For Ridge Lines
*/
 if( ( clPtr = nodeAddrP(dtmP,point)->cPtr) != dtmP->nullPtr )
   {
    scanPnt = clistAddrP(dtmP,clPtr)->pntNum ;
    if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,point,scanPnt)) < 0 ) goto errexit ;
    while ( clPtr != dtmP->nullPtr )
      {
       scanPnt  = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr    = clistAddrP(dtmP,clPtr)->nextPtr ;
       if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point,scanPnt)) < 0 ) goto errexit ;
       if(nodeAddrP(dtmP,point)->hPtr != antPnt && nodeAddrP(dtmP,point)->hPtr != scanPnt && nodeAddrP(dtmP,scanPnt)->hPtr != point && nodeAddrP(dtmP,clkPnt)->hPtr != point  )
         {
          if( pointAddrP(dtmP,point)->z <= pointAddrP(dtmP,scanPnt)->z )
            {
			 antFlow = -99 ;
			 clkFlow = -99 ;
/*
**           Calculate Flow Direction For Adjacent Triangles
*/
             if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,scanPnt,point,antPnt,antVoid,antFlow)) goto errexit ;
             if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,point,clkPnt,scanPnt,clkVoid,clkFlow)) goto errexit ;
/*
**           Write Flow Directions
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"scanPnt = %8ld antPnt = %8ld clkPnt = %8ld ** antFlow = %2ld clkFlow = %2ld",scanPnt,antPnt,clkPnt,antFlow,clkFlow) ;
/*
**           Check Flow Directions Determined
*/
             if( antFlow == -99 || clkFlow == -99 )
			   {
			    bcdtmWrite_message(2,0,0,"Cannot Determine Ridge Line Flow") ;
				goto errexit ;
			   }
/*
**           Check For Ascent Sump Line - Not Both Can Be Zero
*/
             if( antFlow  >= 0 && clkFlow >= 0 && ( antFlow > 0 || clkFlow > 0 ))
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Ascent Sump Line Detected") ;
                dx = pointAddrP(dtmP,scanPnt)->x - pointAddrP(dtmP,point)->x ;
                dy = pointAddrP(dtmP,scanPnt)->y - pointAddrP(dtmP,point)->y ;
                dz = pointAddrP(dtmP,scanPnt)->z - pointAddrP(dtmP,point)->z ;
                dd = sqrt(dx*dx + dy*dy) ;
                slope = dz/dd ;
/*
**              Check Heap Memory
*/
                if( *numAscentSumpLinesP == memAscentSumpLines )
				  {
				   memAscentSumpLines = memAscentSumpLines + memAscentSumpLinesInc ;
				   if( *ascentSumpLinesPP == NULL ) *ascentSumpLinesPP = ( DTM_ASCENT_LINE * ) malloc( memAscentSumpLines * sizeof(DTM_ASCENT_LINE)) ;
				   else                             *ascentSumpLinesPP = ( DTM_ASCENT_LINE * ) realloc( *ascentSumpLinesPP,memAscentSumpLines * sizeof(DTM_ASCENT_LINE)) ;
				   if( *ascentSumpLinesPP == NULL )
				     {
					  bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
					  goto errexit ;
					 }
				  }
/*
**              Store Ascent Sump Line
*/
                (*ascentSumpLinesPP+*numAscentSumpLinesP)->ascentType  = 1 ;
                (*ascentSumpLinesPP+*numAscentSumpLinesP)->pnt1        = point ;
                (*ascentSumpLinesPP+*numAscentSumpLinesP)->pnt2        = scanPnt ;
                (*ascentSumpLinesPP+*numAscentSumpLinesP)->pnt3        = dtmP->nullPnt ;
                (*ascentSumpLinesPP+*numAscentSumpLinesP)->slope       = slope ;
                (*ascentSumpLinesPP+*numAscentSumpLinesP)->ascentAngle = bcdtmMath_getPointAngleDtmObject(dtmP,point,scanPnt) ;
                (*ascentSumpLinesPP+*numAscentSumpLinesP)->x           = pointAddrP(dtmP,point)->x ;
                (*ascentSumpLinesPP+*numAscentSumpLinesP)->y           = pointAddrP(dtmP,point)->y ;
                (*ascentSumpLinesPP+*numAscentSumpLinesP)->z           = pointAddrP(dtmP,point)->z ;
				++*numAscentSumpLinesP ;
               }
            }
         }
       antPnt = scanPnt ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Sump Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Sump Lines Error") ;
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
int bcdtmDrainage_scanPointForAscentRidgeLinesDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To Dtm Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Point To Scan
 DTM_ASCENT_LINE   **ridgeLinesPP,             // <== Pointer To Ridge Lines
 long              *numRidgeLinesP             // <== Number Of Ridge Lines
)
/*
** This Function Scans A Point For Ridge Lines
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   antPnt,scanPnt,clkPnt,clPtr ;
 long   memRidgeLines=0,memRidgeLinesInc=100 ;
 double dx,dy,dz,dd,slope ;
 int    antFlow,clkFlow ;
 bool   antVoid,clkVoid ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Ridge Lines") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"point           = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"ridgeLinesPP    = %8ld",ridgeLinesPP) ;
    bcdtmWrite_message(0,0,0,"numRidgeLinesP  = %8ld",numRidgeLinesP) ;
   }
/*
** Initialise
*/
 *numRidgeLinesP = 0 ;
 if( *ridgeLinesPP != NULL ) { free(*ridgeLinesPP) ; *ridgeLinesPP = NULL ; }
/*
** Scan Around Point For Ridge Lines
*/
 if( ( clPtr = nodeAddrP(dtmP,point)->cPtr) != dtmP->nullPtr )
   {
    scanPnt = clistAddrP(dtmP,clPtr)->pntNum ;
    if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,point,scanPnt)) < 0 ) goto errexit ;
    while ( clPtr != dtmP->nullPtr )
      {
       scanPnt  = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr    = clistAddrP(dtmP,clPtr)->nextPtr ;
       if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point,scanPnt)) < 0 ) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"antPnt = %9ld scanPnt = %9ld clkPnt = %9ld",antPnt,scanPnt,clkPnt) ;
/*
**     Check For Possible Ridge Line
*/
       if( pointAddrP(dtmP,point)->z <= pointAddrP(dtmP,scanPnt)->z )
         {
          antFlow = 0 ;
		  clkFlow = 0 ;
          antVoid = true ;
          clkVoid = true ;
/*
**        Calculate Flow Direction For Anti Clockwise None Void Triangle
*/
          if( nodeAddrP(dtmP,scanPnt)->hPtr != point )
		    {
             if( dbg ) bcdtmWrite_message(0,0,0,"Getting Ccw_wise Flow Direction For Edge %8ld %8ld ** %8ld",point,scanPnt,antPnt) ;
//           if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,scanPnt,point,antPnt,antVoid,antFlow)) goto errexit ;
             if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,point,scanPnt,antPnt,antVoid,antFlow)) goto errexit ;
            }
/*
**        Calculate Flow Direction For Clockwise None Void Triangle
*/
          if( nodeAddrP(dtmP,clkPnt)->hPtr != point )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Getting Clk_wise Flow Direction For Edge %8ld %8ld ** %8ld",point,scanPnt,clkPnt) ;
//           if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,point,clkPnt,scanPnt,clkVoid,clkFlow)) goto errexit ;
             if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,scanPnt,point,clkPnt,clkVoid,clkFlow)) goto errexit ;
            }
          if( dbg ) bcdtmWrite_message(0,0,0,"antFlow = %2ld  ** clkFlow = %2ld",antFlow,clkFlow) ;
/*
**        Check For Ridge Line - Not Both Can Be Zero
*/
//          if( ( ! antVoid || ! clkVoid ) && ( antFlow  <= 0 && clkFlow <= 0 ) && ( antFlow < 0 || clkFlow < 0 ))
          if( ( ! antVoid || ! clkVoid ) && ( antFlow  < 0 && clkFlow < 0 ) )
            {
             dx = pointAddrP(dtmP,scanPnt)->x - pointAddrP(dtmP,point)->x ;
             dy = pointAddrP(dtmP,scanPnt)->y - pointAddrP(dtmP,point)->y ;
             dz = pointAddrP(dtmP,scanPnt)->z - pointAddrP(dtmP,point)->z ;
             dd = sqrt(dx*dx + dy*dy) ;
             slope = dz/dd ;
/*
**           Check Heap Memory
*/
             if( *numRidgeLinesP == memRidgeLines )
               {
                memRidgeLines = memRidgeLines + memRidgeLinesInc ;
			    if( *ridgeLinesPP == NULL ) *ridgeLinesPP = ( DTM_ASCENT_LINE *) malloc( memRidgeLines * sizeof(DTM_ASCENT_LINE)) ;
			    else                        *ridgeLinesPP = ( DTM_ASCENT_LINE *) realloc( *ridgeLinesPP,memRidgeLines * sizeof(DTM_ASCENT_LINE)) ;
			    if( *ridgeLinesPP == NULL )
			      {
			       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
			       goto errexit ;
				  }
               }
/*
**            Store Ridge Line
*/
             (*ridgeLinesPP+*numRidgeLinesP)->ascentType  = 1 ;
             (*ridgeLinesPP+*numRidgeLinesP)->pnt1        = point ;
             (*ridgeLinesPP+*numRidgeLinesP)->pnt2        = scanPnt ;
             (*ridgeLinesPP+*numRidgeLinesP)->pnt3        = dtmP->nullPnt ;
             (*ridgeLinesPP+*numRidgeLinesP)->slope       = slope ;
             (*ridgeLinesPP+*numRidgeLinesP)->ascentAngle = bcdtmMath_getPointAngleDtmObject(dtmP,point,scanPnt) ;
             (*ridgeLinesPP+*numRidgeLinesP)->x           = pointAddrP(dtmP,point)->x ;
             (*ridgeLinesPP+*numRidgeLinesP)->y           = pointAddrP(dtmP,point)->y ;
             (*ridgeLinesPP+*numRidgeLinesP)->z           = pointAddrP(dtmP,point)->z ;
             ++*numRidgeLinesP ;
            }
         }
       antPnt = scanPnt ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Ridge Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Ridge Lines Error") ;
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
int bcdtmDrainage_scanPointForAscentTrianglesDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To DTM Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 long              point,                      // ==> Scan Point For Triangles
 DTM_ASCENT_LINE   **ascentTrianglesPP,        // <== Pointer To Ascent Triangles
 long              *numAscentTrianglesP        // <== Number Of Ascent Triangles
 )
/*
** This Function Scans A Point For Ascent Triangles
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   antPnt,clkPnt,clPtr,memAscentTriangles=0,memAscentTrianglesInc=100 ;
 double a1,a2,a3,angleAntPnt,angleClkPnt,slope,descentAngle,ascentAngle  ;
 bool   voidTriangle ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Triangles") ;
    bcdtmWrite_message(0,0,0,"dtmP                = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"drainageTablesP     = %p",drainageTablesP) ;
    bcdtmWrite_message(0,0,0,"point               = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"ascentTrianglesPP   = %p",ascentTrianglesPP) ;
    bcdtmWrite_message(0,0,0,"numAscentTrianglesP = %8ld",numAscentTrianglesP) ;
   }
/*
** Initialise
*/
 *numAscentTrianglesP = 0 ;
 if( *ascentTrianglesPP != NULL ) { free(*ascentTrianglesPP) ; *ascentTrianglesPP = NULL ; }
/*
** Scan Around point
*/
 clPtr = nodeAddrP(dtmP,point)->cPtr;
 if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
 angleAntPnt = bcdtmMath_getPointAngleDtmObject(dtmP,point,antPnt) ;
 while ( clPtr != dtmP->nullPtr )
   {
    clkPnt = clistAddrP(dtmP,clPtr)->pntNum ;
    clPtr  = clistAddrP(dtmP,clPtr)->nextPtr ;
    angleClkPnt = bcdtmMath_getPointAngleDtmObject(dtmP,point,clkPnt) ;
/*
**  Only Process Points Internal To Hull
*/
    if(  nodeAddrP(dtmP,point)->hPtr != antPnt  )
      {
       if( bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject(dtmP,drainageTablesP,point,antPnt,clkPnt,voidTriangle,slope,descentAngle,ascentAngle) != DTM_SUCCESS ) goto errexit ;
/*
**     Only Process None Void Triangles
*/
       if( voidTriangle == false )
         {
/*
**        Determine If Ascent Angle Intersects Triangle Base
*/
          a1 = angleAntPnt ;
          a2 = ascentAngle ;
          a3 = angleClkPnt ;
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"**** antPnt = %8ld clkPnt = %8ld",antPnt,clkPnt) ;
             bcdtmWrite_message(0,0,0,"ascentAngle = %12.8lf",ascentAngle) ;
             bcdtmWrite_message(0,0,0,"ascentSlope = %12.8lf",slope) ;
             bcdtmWrite_message(0,0,0,"angleAntPnt = %12.8lf",angleAntPnt) ;
             bcdtmWrite_message(0,0,0,"angleClkPnt = %12.8lf",angleClkPnt) ;
            }
          if( a1 < a3 ) a1 = a1 + DTM_2PYE ;
          if( a2 < a3 ) a2 = a2 + DTM_2PYE ;
          if( a2 <= a1 && a2 >= a3 )
            {
/*
**           Check Heap Memory
*/
             if( *numAscentTrianglesP == memAscentTriangles )
	           {
                memAscentTriangles = memAscentTriangles + memAscentTrianglesInc ;
		        if( *ascentTrianglesPP == NULL ) *ascentTrianglesPP = ( DTM_ASCENT_LINE *) malloc( memAscentTriangles * sizeof(DTM_ASCENT_LINE)) ;
		        else                             *ascentTrianglesPP = ( DTM_ASCENT_LINE *) realloc( *ascentTrianglesPP,memAscentTriangles * sizeof(DTM_ASCENT_LINE)) ;
		        if( *ascentTrianglesPP == NULL )
		          {
		           bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
			       goto errexit ;
			      }
	           }
/*
**           Store Ascent Triangle
*/
             (*ascentTrianglesPP+*numAscentTrianglesP)->ascentType  = 2 ;
             (*ascentTrianglesPP+*numAscentTrianglesP)->pnt1        = point ;
             (*ascentTrianglesPP+*numAscentTrianglesP)->pnt2        = antPnt ;
             (*ascentTrianglesPP+*numAscentTrianglesP)->pnt3        = clkPnt ;
             (*ascentTrianglesPP+*numAscentTrianglesP)->slope       = slope ;
             (*ascentTrianglesPP+*numAscentTrianglesP)->ascentAngle = ascentAngle ;
             (*ascentTrianglesPP+*numAscentTrianglesP)->x           = pointAddrP(dtmP,point)->x ;
             (*ascentTrianglesPP+*numAscentTrianglesP)->y           = pointAddrP(dtmP,point)->y ;
             (*ascentTrianglesPP+*numAscentTrianglesP)->z           = pointAddrP(dtmP,point)->z ;
             ++*numAscentTrianglesP ;
            }
         }
      }
    antPnt = clkPnt ;
    angleAntPnt = angleClkPnt ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Triangles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning Point For Ascent Triangles Error") ;
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
int bcdtmDrainage_checkForSumpOrRidgeLineDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To DTM Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 int               linePoint1,                 // ==> Line End Point
 int               linePoint2,                 // ==> Line End Point
 DTMFeatureType&              lineType                    // <== Line Type <DTMFeatureType::SumpLine,DTMFeatureType::RidgeLine>
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 int    antFlow=0,clkFlow=0,antFlat=0,clkFlat=0 ;
 int    antPoint,clkPoint ;
 bool   antVoid=true,clkVoid=true ;
/*
** Write Entry Message
*/
 if( dbg  )
   {
    bcdtmWrite_message(0,0,0,"Checking For Sump Or Ridge Line") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP)         ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %8ld",drainageTablesP)  ;
    bcdtmWrite_message(0,0,0,"linePoint1      = %8ld",linePoint1) ;
    bcdtmWrite_message(0,0,0,"linePoint2      = %8ld",linePoint2) ;
   }

// Initialise

 lineType = DTMFeatureType::None;

//   Get ClockWise And Counter Clock Wise Points

 if( ( antPoint = bcdtmList_nextAntDtmObject(dtmP,linePoint1,linePoint2)) < 0 ) goto errexit ;
 if( ( clkPoint = bcdtmList_nextClkDtmObject(dtmP,linePoint1,linePoint2)) < 0 ) goto errexit ;

//  Log Clockwise And Counter Clockwise Points

 if( dbg )
     {
     bcdtmWrite_message(0,0,0,"linePoint1 = %8ld ** %12.5lf %12.5lf %10.4lf",linePoint1,pointAddrP(dtmP,linePoint1)->x,pointAddrP(dtmP,linePoint1)->y,pointAddrP(dtmP,linePoint1)->z ) ;
     bcdtmWrite_message(0,0,0,"linePoint2 = %8ld ** %12.5lf %12.5lf %10.4lf",linePoint2,pointAddrP(dtmP,linePoint2)->x,pointAddrP(dtmP,linePoint2)->y,pointAddrP(dtmP,linePoint2)->z ) ;
     bcdtmWrite_message(0,0,0,"antPnt     = %8ld ** %12.5lf %12.5lf %10.4lf",antPoint,pointAddrP(dtmP,antPoint)->x,pointAddrP(dtmP,antPoint)->y,pointAddrP(dtmP,antPoint)->z ) ;
     bcdtmWrite_message(0,0,0,"clkPnt     = %8ld ** %12.5lf %12.5lf %10.4lf",clkPoint,pointAddrP(dtmP,clkPoint)->x,pointAddrP(dtmP,clkPoint)->y,pointAddrP(dtmP,clkPoint)->z ) ;
     }

/*
** Calculate Flow Direction For Anti Clockwise None Void Valid Triangle
*/
 if( nodeAddrP(dtmP,linePoint2)->hPtr != linePoint1 )
   {
    if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,linePoint1,linePoint2,antPoint,antVoid,antFlow)) goto errexit ;
    if( pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,linePoint2)->z && pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,antPoint)->z ) antFlat = 1 ;
   }
/*
**  Calculate Flow Direction For Clockwise None Void Valid Triangle
*/
 if( nodeAddrP(dtmP,linePoint1)->hPtr != linePoint2 )
   {
    if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,linePoint2,linePoint1,clkPoint,clkVoid,clkFlow)) goto errexit ;
    if( pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,linePoint2)->z && pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,clkPoint)->z ) clkFlat = 1 ;
   }
/*
** Write Flow Direcitions
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"antFlow = %2ld clkFlow = %2ld ** antFlat = %2ld clkFlat = %2ld",antFlow,clkFlow,antFlat,clkFlat) ;
/*
**   Check For Sump Line
*/
  if     ( antFlow  > 0 && clkFlow > 0 )   lineType = DTMFeatureType::SumpLine ;
/*
**   Check For Ridge Line
*/
  else if( antFlow  < 0 && clkFlow < 0 )  lineType = DTMFeatureType::RidgeLine ;
/*
**   Check For Cross Flow Line
*/
  else if( ( antFlow  == 1 && clkFlow != 1 ) ||
           ( antFlow  != 1 && clkFlow == 1 )    ) lineType = DTMFeatureType::CrossLine ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Sump Or Ridge Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Sump Or Ridge Line Error") ;
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
int bcdtmDrainage_checkForBarrierLineDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To DTM Object
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 int               linePoint1,                 // ==> Line End Point
 int               linePoint2,                 // ==> Line End Point
 DTMFeatureType&   lineType                    // <== Line Type <DTMFeatureType::SumpLine,DTMFeatureType::RidgeLine>
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 int    antFlow=0,clkFlow=0,antFlat=0,clkFlat=0 ;
 int    antPoint,clkPoint ;
 bool   antVoid=true,clkVoid=true ;
/*
** Write Entry Message
*/
 if( dbg  )
   {
    bcdtmWrite_message(0,0,0,"Checking For Sump Or Ridge Line") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP)         ;
    bcdtmWrite_message(0,0,0,"drainageTablesP = %8ld",drainageTablesP)  ;
    bcdtmWrite_message(0,0,0,"linePoint1      = %8ld",linePoint1) ;
    bcdtmWrite_message(0,0,0,"linePoint2      = %8ld",linePoint2) ;
   }

// Initialise

 lineType = DTMFeatureType::None;

//   Get ClockWise And Counter Clock Wise Points

 if( ( antPoint = bcdtmList_nextAntDtmObject(dtmP,linePoint1,linePoint2)) < 0 ) goto errexit ;
 if( ( clkPoint = bcdtmList_nextClkDtmObject(dtmP,linePoint1,linePoint2)) < 0 ) goto errexit ;

//  Log Clockwise And Counter Clockwise Points

 if( dbg )
     {
     bcdtmWrite_message(0,0,0,"linePoint1 = %8ld ** %12.5lf %12.5lf %10.4lf",linePoint1,pointAddrP(dtmP,linePoint1)->x,pointAddrP(dtmP,linePoint1)->y,pointAddrP(dtmP,linePoint1)->z ) ;
     bcdtmWrite_message(0,0,0,"linePoint2 = %8ld ** %12.5lf %12.5lf %10.4lf",linePoint2,pointAddrP(dtmP,linePoint2)->x,pointAddrP(dtmP,linePoint2)->y,pointAddrP(dtmP,linePoint2)->z ) ;
     bcdtmWrite_message(0,0,0,"antPnt     = %8ld ** %12.5lf %12.5lf %10.4lf",antPoint,pointAddrP(dtmP,antPoint)->x,pointAddrP(dtmP,antPoint)->y,pointAddrP(dtmP,antPoint)->z ) ;
     bcdtmWrite_message(0,0,0,"clkPnt     = %8ld ** %12.5lf %12.5lf %10.4lf",clkPoint,pointAddrP(dtmP,clkPoint)->x,pointAddrP(dtmP,clkPoint)->y,pointAddrP(dtmP,clkPoint)->z ) ;
     }

/*
** Calculate Flow Direction For Anti Clockwise None Void Valid Triangle
*/
 if( nodeAddrP(dtmP,linePoint2)->hPtr != linePoint1 )
   {
    if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,linePoint1,linePoint2,antPoint,antVoid,antFlow)) goto errexit ;
    if( pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,linePoint2)->z && pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,antPoint)->z ) antFlat = 1 ;
   }
/*
**  Calculate Flow Direction For Clockwise None Void Valid Triangle
*/
 if( nodeAddrP(dtmP,linePoint1)->hPtr != linePoint2 )
   {
    if( bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject(dtmP,drainageTablesP,linePoint2,linePoint1,clkPoint,clkVoid,clkFlow)) goto errexit ;
    if( pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,linePoint2)->z && pointAddrP(dtmP,linePoint1)->z == pointAddrP(dtmP,clkPoint)->z ) clkFlat = 1 ;
   }
/*
** Write Flow Direcitions
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"antFlow = %2ld clkFlow = %2ld ** antFlat = %2ld clkFlat = %2ld",antFlow,clkFlow,antFlat,clkFlat) ;
/*
**   Check For Sump Line
*/
  if     (  antFlow  > 0 && clkFlow > 0 )   lineType = DTMFeatureType::SumpLine ;
/*
**   Check For Ridge Line
*/
  else if( antFlow  < 0 && clkFlow < 0 )  lineType = DTMFeatureType::RidgeLine ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Sump Or Ridge Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Sump Or Ridge Line Error") ;
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
int bcdtmDrainage_traceToSumpLineDtmObject
(
 BC_DTM_OBJ *dtmP,                                // ==> Pointer To DTM Object
 long       startType,                            // ==> 1 Triangle Point , 2 Triangle Edge , 3  Inside Triangle
 long       pnt1,                                 // ==> Triangle Point 1
 long       pnt2,                                 // ==> Triangle Point 2
 long       pnt3,                                 // ==> Triangle Point 3
 double     x,                                    // ==> Start Point X Coordinate
 double     y,                                    // ==> Start Point Y Coordinate
 double     z,                                    // ==> Start Point Y Coordiante
 long       *sumpPnt1P,                           // <==  Point At One End Of Sump Line
 long       *sumpPnt2P,                           // <==  Point At Other End Of Sump Line
 double     *sumpXP,                              // <==  Sump Point X Coordiante
 double     *sumpYP,                              // <==  Sump Point Y Coordiante
 double     *sumpZP                               // <==  Sump Point Z Coordiante
)
/*
** This Function Traces To A Sump Line
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   nextPnt1,nextPnt2,nextPnt3,iteration=0 ;
 long   pnt,process,lastPoint;
 double firstX,firstY,firstZ,startX,startY,startZ,nextX,nextY,nextZ ;
 double lastAngle=-99.99,saveLastAngle,descentAngle,ascentAngle,slope ;
 /*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing To Sump Line") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startType   = %8ld",startType) ;
    bcdtmWrite_message(0,0,0,"pnt1        = %8ld",pnt1) ;
    bcdtmWrite_message(0,0,0,"pnt2        = %8ld",pnt2) ;
    bcdtmWrite_message(0,0,0,"pnt3        = %8ld",pnt3) ;
    bcdtmWrite_message(0,0,0,"x           = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y           = %12.5lf",y) ;
    bcdtmWrite_message(0,0,0,"z           = %12.5lf",z) ;
   }
/*
** Initialise
*/
 saveLastAngle = -DTM_2PYE ;
 process   = 0 ;
 firstX    = startX = x ;
 firstY    = startY = y ;
 firstZ    = startZ = z ;
 lastPoint = dtmP->nullPnt ;
 *sumpPnt1P = dtmP->nullPnt ;
 *sumpPnt2P = dtmP->nullPnt ;
 *sumpXP = 0.0 ;
 *sumpYP = 0.0 ;
 *sumpZP = 0.0 ;
/*
** Check Start Type
*/
 if(  startType < 1 || startType > 3 )
   {
    bcdtmWrite_message(2,0,0,"Illegal Trace To Sump Line Start Type") ;
    goto errexit ;
   }

 // Log Start Trace Point

 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Start Trace Point = %12.5lf %12.5lf %10.4lf",startX,startY,startZ) ;
   }

/*
** Trace Starts On Triangle Edge
*/
 if( startType == 2  )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing First Edge") ;
    if( bcdtmDrainage_getFirstTrianglePointFromTriangleEdgeDtmObject(dtmP,2,pnt1,pnt2,&pnt3) ) goto errexit ;
    if( pnt3 == dtmP->nullPnt )
      {
       bcdtmWrite_message(2,0,0,"First Triangle Point Out Error") ;
       goto errexit ;
      }
/*
**  Set Points Anti Clockwise
*/
    if( bcdtmMath_pointSideOfDtmObject(dtmP,pnt1,pnt2,pnt3) < 0 )
      {
       pnt  = pnt1 ;
       pnt1 = pnt2 ;
       pnt2 = pnt  ;
      }
   }
/*
** Trace Starts In Triangle
*/
 if( startType == 3  )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing First Triangle") ;
    bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,pnt3,&descentAngle,&ascentAngle,&slope) ;
    saveLastAngle = lastAngle = descentAngle ;
    if( bcdtmDrainage_getFirstTracePointFromTriangleDtmObject(dtmP,2,pnt1,pnt2,pnt3,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ)) goto errexit ;
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"First Trace Point = %12.5lf %12.5lf %10.4lf",nextX,nextY,nextZ) ;
      }

/*
**  Initialise Iteration Variables
*/
    pnt1 = nextPnt1 ;
    pnt2 = nextPnt2 ;
    pnt3 = nextPnt3 ;
    startX = nextX  ;
    startY = nextY  ;
    startZ = nextZ  ;
    if( firstX != startX || firstY != startY )
	  {
	   saveLastAngle = lastAngle = bcdtmMath_getAngle(firstX,firstY,startX,startY) ;
      }
   }
/*
** Iteratively Get Next Trace Point
*/
 do
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"iteration[%6ld] ** pnt1 = %9ld pnt2 = %9ld",iteration,pnt1,pnt2) ;
    ++iteration ;
/*
**  Trace To Next Low Point
*/
    if( pnt2 != dtmP->nullPnt ){ if( bcdtmDrainage_traceToSumpLineFromTriangleEdgeDtmObject(dtmP,lastAngle,pnt1,pnt2,pnt3,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process)) goto errexit ; }
    else                       { if( bcdtmDrainage_traceToSumpLineFromTrianglePointDtmObject(dtmP,lastAngle,lastPoint,pnt1,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process)) goto errexit ; }

    //  Log Trace Point

    if( dbg == 1 && process )
      {
       bcdtmWrite_message(0,0,0,"Next  Trace Point = %12.5lf %12.5lf %10.4lf",nextX,nextY,nextZ) ;
      }

    //

    if( process )
      {
       if( pnt2 == dtmP->nullPnt ) lastPoint = pnt1 ;
       else                        lastPoint = dtmP->nullPnt ;
       pnt1 = nextPnt1 ;
       pnt2 = nextPnt2 ;
       pnt3 = nextPnt3 ;
       if( startX != nextX || startY != nextY ) lastAngle = bcdtmMath_getAngle(startX,startY,nextX,nextY) ;
       else                                     lastAngle = saveLastAngle ;
       saveLastAngle = lastAngle ;
       startX = nextX  ;
       startY = nextY  ;
       startZ = nextZ  ;
      }
   } while ( process ) ;
/*
** Set Return Values
*/
 *sumpXP = startX ;
 *sumpYP = startY ;
 *sumpZP = startZ ;
 *sumpPnt1P = pnt1 ;
 *sumpPnt2P = pnt2 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"sumpPnt1 = %6ld sumpPnt2 = %6ld ** %12.5lf %12.5lf %10.4lf",*sumpPnt1P,*sumpPnt2P,*sumpXP,*sumpYP,*sumpZP) ;
/*
** Clean Up
*/
 cleanup :
/*
**  Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing To Sump Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing To Sump Line Error") ;
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
int bcdtmDrainage_traceToSumpLineFromTriangleEdgeDtmObject
(
 BC_DTM_OBJ *dtmP,
 double lastAngle,
 long startPnt1,
 long startPnt2,
 long startPnt3,
 double startX,
 double startY,
 long *nextPnt1P,
 long *nextPnt2P,
 long *nextPnt3P,
 double *nextXP,
 double *nextYP,
 double *nextZP,
 long *processP
)
{
 int    ret=DTM_SUCCESS,sdof,dbg=DTM_TRACE_VALUE(0) ;
 long   flowDirection,hullLine ;
 double dx,dy,dz,xRad,yRad,radius,descentAngle,ascentAngle,slope,zeroSlopeTriangle ;
 /*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Tracing To Sump Line From Triangle Edge") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"startPnt1 = %6ld ** %10.4lf %10.4lf %10.4lf",startPnt1,pointAddrP(dtmP,startPnt1)->x,pointAddrP(dtmP,startPnt1)->y,pointAddrP(dtmP,startPnt1)->z) ;
    bcdtmWrite_message(0,0,0,"startPnt2 = %6ld ** %10.4lf %10.4lf %10.4lf",startPnt2,pointAddrP(dtmP,startPnt2)->x,pointAddrP(dtmP,startPnt2)->y,pointAddrP(dtmP,startPnt2)->z) ;
    bcdtmWrite_message(0,0,0,"startPnt3 = %6ld ** %10.4lf %10.4lf %10.4lf",startPnt3,pointAddrP(dtmP,startPnt3)->x,pointAddrP(dtmP,startPnt3)->y,pointAddrP(dtmP,startPnt3)->z) ;
   }
/*
** Initialise
*/
 *processP    = 0 ;
 *nextPnt1P   = dtmP->nullPnt ;
 *nextPnt2P   = dtmP->nullPnt ;
 *nextPnt3P   = dtmP->nullPnt ;
 *nextXP  = 0.0 ;
 *nextYP  = 0.0 ;
 *nextZP  = 0.0 ;
 dx = dtmP->xMax - dtmP->xMin ;
 dy = dtmP->yMax - dtmP->yMin ;
/*
** Check For Termination On A Hull Edge
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Hull Line") ;
 hullLine = bcdtmList_testForHullLineDtmObject(dtmP,startPnt1,startPnt2) ;
/*
** Only Process If Start Edge Is Not A Hull Line
*/
 if( ! hullLine )
   {
/*
**  Calculate Flow Direction From Start Edge
**
**  + Direction Flows In To The Edge
**  0 Direction Flows Parallel To The Edge
**  - Direction Flows Out From The Edge
**
*/
    flowDirection = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,startPnt1,startPnt2,startPnt3) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"flow Direction = %2ld",flowDirection) ;
/*
** Check For Zero Slope Triangle
*/
    zeroSlopeTriangle = 0 ;
    if(pointAddrP(dtmP,startPnt1)->z == pointAddrP(dtmP,startPnt2)->z  && pointAddrP(dtmP,startPnt1)->z == pointAddrP(dtmP,startPnt3)->z ) zeroSlopeTriangle = 1 ;
    if( dbg && zeroSlopeTriangle ) bcdtmWrite_message(0,0,0,"Zero slope Triangle") ;
/*
**  Flow Direction Must Be Away From Edge Otherwise Current Line Is A Sump Line
*/
    if( flowDirection < 0  ||  zeroSlopeTriangle )
      {
/*
**     Sump Line Detected
*/
       *processP = 1 ;
/*
**     Get Descent Angle For Triangle
*/
       if( zeroSlopeTriangle ) descentAngle = lastAngle ;
       else                    bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,startPnt1,startPnt2,startPnt3,&descentAngle,&ascentAngle,&slope) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Descent Angle = %12.10lf",descentAngle) ;
/*
**     Calculate Radial Out From Start x And Start y At Descent Angle
*/
       radius = dx * dx + dy * dy ;
       xRad = startX + radius * cos(descentAngle) ;
       yRad = startY + radius * sin(descentAngle) ;
/*
**     Determine Triangle Flow Out Edge
*/
       sdof = bcdtmMath_sideOf(startX,startY,xRad,yRad,pointAddrP(dtmP,startPnt3)->x,pointAddrP(dtmP,startPnt3)->y) ;
/*
**     Flow Passes Through P3
*/
       if( sdof == 0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Flow Passes Through P3") ;
          *nextPnt1P = startPnt3 ;
          *nextXP = pointAddrP(dtmP,startPnt3)->x ;
          *nextYP = pointAddrP(dtmP,startPnt3)->y ;
          *nextZP = pointAddrP(dtmP,startPnt3)->z ;
         }
/*
**     Flow Intersects Edge P2-P3
*/
       if( sdof >  0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Flow Intersects Edge P2-P3") ;
          bcdtmDrainage_intersectCordLines(startX,startY,xRad,yRad,pointAddrP(dtmP,startPnt3)->x,pointAddrP(dtmP,startPnt3)->y,pointAddrP(dtmP,startPnt2)->x,pointAddrP(dtmP,startPnt2)->y,nextXP,nextYP) ;
          if     ( bcdtmMath_distance(pointAddrP(dtmP,startPnt3)->x,pointAddrP(dtmP,startPnt3)->y,*nextXP,*nextYP) <= dtmP->ppTol ) { *nextPnt1P = startPnt3 ; *nextPnt2P = *nextPnt3P = dtmP->nullPnt ; }
          else if( bcdtmMath_distance(pointAddrP(dtmP,startPnt2)->x,pointAddrP(dtmP,startPnt2)->y,*nextXP,*nextYP) <= dtmP->ppTol ) { *nextPnt1P = startPnt2 ; *nextPnt2P = *nextPnt3P = dtmP->nullPnt ; }
          else
            {
             dx = pointAddrP(dtmP,startPnt3)->x - pointAddrP(dtmP,startPnt2)->x ;
             dy = pointAddrP(dtmP,startPnt3)->y - pointAddrP(dtmP,startPnt2)->y ;
             dz = pointAddrP(dtmP,startPnt3)->z - pointAddrP(dtmP,startPnt2)->z ;
             if( fabs(dx) >= fabs(dy) ) *nextZP = pointAddrP(dtmP,startPnt2)->z +  dz * (*nextXP - pointAddrP(dtmP,startPnt2)->x) / dx ;
             else                       *nextZP = pointAddrP(dtmP,startPnt2)->z +  dz * (*nextYP - pointAddrP(dtmP,startPnt2)->y) / dy ;
             *nextPnt1P = startPnt3 ;
             *nextPnt2P = startPnt2 ;
             if(( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,*nextPnt1P,*nextPnt2P)) < 0 ) goto errexit ;
            }
          if( *nextPnt2P == dtmP->nullPnt )
            {
             *nextXP = pointAddrP(dtmP,*nextPnt1P)->x ;
             *nextYP = pointAddrP(dtmP,*nextPnt1P)->y ;
             *nextZP = pointAddrP(dtmP,*nextPnt1P)->z ;
            }
         }
/*
**     Flow Intersects Edge P1-P3
*/
       if( sdof <  0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Flow Intersects Edge P1-P3") ;
          bcdtmDrainage_intersectCordLines(startX,startY,xRad,yRad,pointAddrP(dtmP,startPnt3)->x,pointAddrP(dtmP,startPnt3)->y,pointAddrP(dtmP,startPnt1)->x,pointAddrP(dtmP,startPnt1)->y,nextXP,nextYP) ;
          if     ( bcdtmMath_distance(pointAddrP(dtmP,startPnt3)->x,pointAddrP(dtmP,startPnt3)->y,*nextXP,*nextYP) <= dtmP->ppTol ) { *nextPnt1P = startPnt3 ; *nextPnt2P = *nextPnt3P = dtmP->nullPnt ; }
          else if( bcdtmMath_distance(pointAddrP(dtmP,startPnt1)->x,pointAddrP(dtmP,startPnt1)->y,*nextXP,*nextYP) <= dtmP->ppTol ) { *nextPnt1P = startPnt1 ; *nextPnt2P = *nextPnt3P = dtmP->nullPnt ; }
          else
            {
             dx = pointAddrP(dtmP,startPnt3)->x - pointAddrP(dtmP,startPnt1)->x ;
             dy = pointAddrP(dtmP,startPnt3)->y - pointAddrP(dtmP,startPnt1)->y ;
             dz = pointAddrP(dtmP,startPnt3)->z - pointAddrP(dtmP,startPnt1)->z ;
             if( fabs(dx) >= fabs(dy) ) *nextZP = pointAddrP(dtmP,startPnt1)->z +  dz * (*nextXP - pointAddrP(dtmP,startPnt1)->x) / dx ;
             else                       *nextZP = pointAddrP(dtmP,startPnt1)->z +  dz * (*nextYP - pointAddrP(dtmP,startPnt1)->y) / dy ;
             *nextPnt1P = startPnt1 ;
             *nextPnt2P = startPnt3 ;
             if(( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,*nextPnt1P,*nextPnt2P)) < 0 ) goto errexit ;
            }
          if( *nextPnt2P == dtmP->nullPnt )
            {
             *nextXP = pointAddrP(dtmP,*nextPnt1P)->x ;
             *nextYP = pointAddrP(dtmP,*nextPnt1P)->y ;
             *nextZP = pointAddrP(dtmP,*nextPnt1P)->z ;
            }
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing To Sump Line From Triangle Edge Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing To Sump Line From Triangle Edge Error") ;
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
int bcdtmDrainage_getFirstTrianglePointFromTriangleEdgeDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       traceDirection,       /* 1 = Ascent , 2 = Descent */
 long       pnt1,
 long       pnt2,
 long       *nxtPntP
)
/*
** Get The Next Triangle Point Out From A Triangle Edge To Start A Trace
*/
{
 int    ret=DTM_SUCCESS ;
 long   antPnt,clkPnt,antFlow,clkFlow ;
 double antSlope,clkSlope,descentAngle,ascentAngle ;
/*
** Initialise
*/
 *nxtPntP = dtmP->nullPnt ;
/*
** Get Points Either Side Of Triangle Edge
*/
 if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2) )   < 0 ) goto errexit ;
 if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt2) ) < 0 ) goto errexit ;
 if( ! bcdtmList_testLineDtmObject(dtmP,pnt2,antPnt) ) antPnt = dtmP->nullPnt ;
 if( ! bcdtmList_testLineDtmObject(dtmP,pnt2,clkPnt) ) clkPnt = dtmP->nullPnt ;
/*
** Set Next Point Out
*/
 if     ( antPnt != dtmP->nullPnt &&  clkPnt == dtmP->nullPnt ) *nxtPntP = antPnt ;
 else if( antPnt == dtmP->nullPnt &&  clkPnt != dtmP->nullPnt ) *nxtPntP = clkPnt ;
/*
** Get Flow Directions
*/
 else
   {
    antFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,pnt1,pnt2,antPnt) ;
    clkFlow = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,pnt1,pnt2,clkPnt) ;
/*
**  Tracing Ascent
*/
    if( traceDirection == 1 )
      {
            if( antFlow ==  1 && clkFlow == -1 ) *nxtPntP = antPnt ;
       else if( antFlow == -1 && clkFlow ==  1 ) *nxtPntP = clkPnt ;
/*
**     Both In Ascent
*/
       else
         {
          bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,antPnt,&descentAngle,&ascentAngle,&antSlope) ;
          bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,clkPnt,&descentAngle,&ascentAngle,&clkSlope) ;
          if( antSlope >= clkSlope ) *nxtPntP = antPnt ;
          else                       *nxtPntP = clkPnt ;
         }
      }
/*
**  Tracing Descent
*/
    if( traceDirection == 2 )
      {
            if( antFlow == -1 && clkFlow ==  1 ) *nxtPntP = antPnt ;
       else if( antFlow ==  1 && clkFlow == -1 ) *nxtPntP = clkPnt ;
/*
**     Both In Descent
*/
       else
         {
          bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,antPnt,&descentAngle,&ascentAngle,&antSlope) ;
          bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,clkPnt,&descentAngle,&ascentAngle,&clkSlope) ;
          if( antSlope >= clkSlope ) *nxtPntP = antPnt ;
          else                       *nxtPntP = clkPnt ;
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
int bcdtmDrainage_traceToSumpLineFromTrianglePointDtmObject
(
 BC_DTM_OBJ *dtmP,
 double lastAngle,
 long   lastPnt,
 long   startPnt,
 double startX,
 double startY,
 long   *nextPnt1P,
 long   *nextPnt2P,
 long   *nextPnt3P,
 double *nextXP,
 double *nextYP,
 double *nextZP,
 long   *processP
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   trgPnt1,trgPnt2,sumpPnt,descentType  ;
 double  a1,a2,dx,dy,dz,xRad,yRad,radius,descentAngle,sumpSlope,sumpAngle,triangleSlope ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing To Sump Line From Triangle Point") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"lastAngle  = %12.10lf",lastAngle) ;
    bcdtmWrite_message(0,0,0,"startPnt   = %8ld",startPnt) ;
    bcdtmWrite_message(0,0,0,"lastPnt    = %8ld",lastPnt) ;
    bcdtmWrite_message(0,0,0,"startX     = %12.10lf",startX) ;
    bcdtmWrite_message(0,0,0,"startY     = %12.10lf",startY) ;
   }
/*
** Initialise Variables
*/
 *processP = 0 ;
 *nextXP = 0.0 ;
 *nextYP = 0.0 ;
 *nextZP = 0.0 ;   ;
 *nextPnt1P = dtmP->nullPnt ;
 *nextPnt2P = dtmP->nullPnt ;
 *nextPnt3P = dtmP->nullPnt ;
 sumpPnt = dtmP->nullPnt ;
 trgPnt1 = dtmP->nullPnt ;
 trgPnt2 = dtmP->nullPnt ;
 sumpSlope = 0.0 ;
 triangleSlope = 0.0 ;
/*
**  Scan Point For Maximum Descent Sump Line
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Sump Line") ;
 bcdtmDrainage_scanPointForMaximumDescentSumpLineDtmObject(dtmP,0,startPnt,lastPnt,&sumpPnt,&sumpAngle,&sumpSlope) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"sumpPnt = %9ld sumpSlope = %10.4lf",sumpPnt,sumpSlope) ;
/*
**  Scan Point For Maximum Descent Triangle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Point For Maximum Descent Triangle") ;
 if( bcdtmDrainage_scanPointForMaximumDescentTriangleDtmObject(dtmP,0,startPnt,lastPnt,&trgPnt1,&trgPnt2,&descentAngle,&triangleSlope) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"trgPnt1 = %9ld trgPnt2 = %9ld trgSlope = %10.4lf",trgPnt1,trgPnt2,triangleSlope) ;
/*
**  Check Descent Found
*/
 if( sumpPnt != dtmP->nullPnt || trgPnt1 != dtmP->nullPnt )
   {
/*
**  Determine Descent Type
*/
    descentType = 0 ;
    if     ( sumpPnt != dtmP->nullPnt && trgPnt1 == dtmP->nullPnt ) descentType = 1 ;
    else if( sumpPnt != dtmP->nullPnt && trgPnt1 != dtmP->nullPnt && sumpSlope >= triangleSlope ) descentType = 1 ;
    else if( sumpPnt == dtmP->nullPnt && trgPnt1 != dtmP->nullPnt ) descentType = 2 ;
    else if( sumpPnt != dtmP->nullPnt && trgPnt1 != dtmP->nullPnt && sumpSlope <  triangleSlope ) descentType = 2 ;
/*
**  Maximum Descent Is Down A Sump Line
*/
    if( descentType == 1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"descentType = %2ld",descentType) ;
       if( sumpSlope  > 0.0 )
         {
          *nextPnt1P = sumpPnt ;
          *nextXP = pointAddrP(dtmP,sumpPnt)->x ;
          *nextYP = pointAddrP(dtmP,sumpPnt)->y ;
          *nextZP = pointAddrP(dtmP,sumpPnt)->z ;
          *processP = 1 ;
         }
/*
**     Find Zero Slope Triangle To Trace Over
*/
       else
         {
          if( lastAngle >= 0.0 )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Finding Zero Slope Triangle To Trace Over") ;
             if( bcdtmDrainage_getDescentZeroSlopeTriangleToTraceOverDtmObject(dtmP,startPnt,lastAngle,&trgPnt1,&trgPnt2)) goto errexit ;
             if( trgPnt2 != dtmP->nullPnt )
               {
                descentType = 2 ;
                triangleSlope = 0.0 ;
               }
            }
         }
      }
/*
**  Maximum Descent Is Down A Triangle Face
*/
    if( descentType == 2 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"descentType = %2ld",descentType) ;
/*
**     Only Process Zero Slope Triangles When The lastAngle is valid
*/
       if( triangleSlope > 0.0 || ( triangleSlope == 0.0 && lastAngle >= 0.0 ))
         {
/*
**        Adjust Descent Angle For Zero Slope Triangle
*/
          if( triangleSlope == 0.0 ) descentAngle = lastAngle ;
/*
**        Calculate End Points For Radial At Maximum Descent Angle
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Calculating EndPoints") ;
          dx = dtmP->xMax - dtmP->xMin ;
          dy = dtmP->yMax - dtmP->yMin ;
          radius = sqrt(dx*dx + dy*dy) ;
          xRad = startX + radius * cos(descentAngle) ;
          yRad = startY + radius * sin(descentAngle) ;
/*
**        Check If Maximum Descent Radial Is Coincident With Triangle Base Points
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Checking Coincidence With Base Points") ;
          a1  = bcdtmMath_normalDistanceToCordLine(startX,startY,xRad,yRad,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y) ;
          a2  = bcdtmMath_normalDistanceToCordLine(startX,startY,xRad,yRad,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y) ;
          if( a1 <= a2 && a1 <= dtmP->ppTol )
            {
             *nextPnt1P = trgPnt1 ;
             *nextXP = pointAddrP(dtmP,trgPnt1)->x ;
             *nextYP = pointAddrP(dtmP,trgPnt1)->y ;
             *nextZP = pointAddrP(dtmP,trgPnt1)->z ;
             *processP = 1 ;
            }
          else if( a2 <= a1 && a2 <= dtmP->ppTol )
            {
             *nextPnt1P = trgPnt2 ;
             *nextXP = pointAddrP(dtmP,trgPnt2)->x ;
             *nextYP = pointAddrP(dtmP,trgPnt2)->y ;
             *nextZP = pointAddrP(dtmP,trgPnt2)->z ;
             *processP = 1 ;
            }
/*
**        Calculate Intercept Of Radial On Triangle Base
*/
          else
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Intercept On Triangle Base") ;
             bcdtmDrainage_intersectCordLines(startX,startY,xRad,yRad,pointAddrP(dtmP,trgPnt1)->x,pointAddrP(dtmP,trgPnt1)->y,pointAddrP(dtmP,trgPnt2)->x,pointAddrP(dtmP,trgPnt2)->y,nextXP,nextYP) ;
             dx = pointAddrP(dtmP,trgPnt2)->x - pointAddrP(dtmP,trgPnt1)->x ;
             dy = pointAddrP(dtmP,trgPnt2)->y - pointAddrP(dtmP,trgPnt1)->y ;
             dz = pointAddrP(dtmP,trgPnt2)->z - pointAddrP(dtmP,trgPnt1)->z ;
             if( fabs(dx) >= fabs(dy) ) *nextZP = pointAddrP(dtmP,trgPnt1)->z +  dz * (*nextXP - pointAddrP(dtmP,trgPnt1)->x) / dx ;
             else                       *nextZP = pointAddrP(dtmP,trgPnt1)->z +  dz * (*nextYP - pointAddrP(dtmP,trgPnt1)->y) / dy ;
             *nextPnt1P = trgPnt1 ;
             *nextPnt2P = trgPnt2 ;
             if( ( *nextPnt3P = bcdtmList_nextAntDtmObject(dtmP,trgPnt1,trgPnt2)) < 0 ) goto errexit ;
             *processP = 1 ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing To Sump Line From Triangle Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing To Sump Line From Triangle Point Error") ;
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
int bcdtmDrainage_getDescentZeroSlopeTriangleToTraceOverDtmObject
(
 BC_DTM_OBJ *dtmP,              /* ==> Pointer To Tin Object  */
 long   point,                   /* ==> Triangle Scan Point    */
 double lastAngle,               /* ==> Last Angle             */
 long   *trgPnt1P,               /* <== Triangle Base Point 1  */
 long   *trgPnt2P                /* <== Triangle Base Point 2  */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   pnt1, pnt2, clPtr, antPnt, clkPnt, voidTriangle;
 DTMFeatureType lineType;
 double angle1,angle2=0.0,angle3 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scanning For Descent Zero Slope Triangle") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"point           = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"lastAngle       = %12.10lf",lastAngle) ;
   }
/*
** Initialise
*/
 *trgPnt1P = dtmP->nullPnt ;
 *trgPnt2P = dtmP->nullPnt ;
/*
** Scan Point For Zero Slope Triangle
*/
 if(( clPtr = nodeAddrP(dtmP,point)->cPtr) != dtmP->nullPtr )
   {
    if(( pnt1 = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
    while ( clPtr != dtmP->nullPtr && *trgPnt1P == dtmP->nullPnt )
      {
       pnt2  = clistAddrP(dtmP,clPtr)->pntNum ;
       clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
/*
**     Check For Valid Triangle
*/
       if( nodeAddrP(dtmP,point)->hPtr != pnt1 )
         {
/*
**        Check For Zero Slope Triangle
*/
          if( pointAddrP(dtmP,point)->z == pointAddrP(dtmP,pnt1)->z && pointAddrP(dtmP,point)->z == pointAddrP(dtmP,pnt2)->z )
            {
/*
**           Check For Void Triangle
*/
             bcdtmList_testForVoidTriangleDtmObject(dtmP,point,pnt1,pnt2,&voidTriangle) ;
             if( ! voidTriangle )
               {
                angle1 = bcdtmMath_getPointAngleDtmObject(dtmP,point,pnt1) ;
                angle2 = lastAngle ;
                angle3 = bcdtmMath_getPointAngleDtmObject(dtmP,point,pnt2) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"angle3 = %12.10lf angle2 = %12.10lf angle1 = %12.10lf",angle3,angle2,angle1) ;
                if( angle1 < angle3 ) angle1 += DTM_2PYE ;
                if( angle2 < angle3 ) angle2 += DTM_2PYE ;
                if( angle2 >= angle3 && angle2 <= angle1 )
                  {
                   *trgPnt1P = pnt1 ;
                   *trgPnt2P = pnt2 ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Triangle Found") ;
                  }
               }
            }
         }
       pnt1  = pnt2 ;
      }
   }
/*
** If Triangle Not Found Scan For Closest In Direction Zero Slope Sump Line
*/
 if( *trgPnt1P == dtmP->nullPnt )
   {
    if(( clPtr = nodeAddrP(dtmP,point)->cPtr) != dtmP->nullPtr )
      {
       while ( clPtr != dtmP->nullPtr  )
         {
          pnt1  = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
            {
             if( pointAddrP(dtmP,point)->z == pointAddrP(dtmP,pnt1)->z )
               {
/*
**              Get Points On Opposite Sides Of Zero Slope Line
*/
                if(( clkPnt = bcdtmList_nextClkDtmObject(dtmP,point,pnt1)) < 0 ) goto errexit ;
                if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,point,pnt1)) < 0 ) goto errexit ;
/*
**              Check For Sump Line
*/
                if( bcdtmDrainage_checkForSumpOrRidgeLineDtmObject(dtmP,nullptr,point,pnt1,antPnt,clkPnt,&lineType)) goto errexit ;
                if( lineType == DTMFeatureType::SumpLine )
                  {
                   angle1 = fabs((lastAngle - bcdtmMath_getPointAngleDtmObject(dtmP,point,pnt1))) ;
                   if( DTM_2PYE - angle1 < angle1 ) angle1 = DTM_2PYE - angle1 ;
                   if( *trgPnt1P == dtmP->nullPnt || angle1 < angle2 )
                     {
                      *trgPnt1P = pnt1  ;
                      angle2 = angle1 ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"Zero Slope Sump Line Found ** z = %10.4lf Z1 = %10.4lf AZ = %10.4lf CZ = %10.4lf",pointAddrP(dtmP,point)->z,pointAddrP(dtmP,pnt1)->z,pointAddrP(dtmP,antPnt)->z,pointAddrP(dtmP,clkPnt)->z) ;
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
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning For Descent Zero Slope Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning For Descent Zero Slope Triangle Error") ;
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
int bcdtmDrainage_checkTraceToSumpLineDtmObject
(
 BC_DTM_OBJ *dtmP,                        // ==> Pointer To Tin Object
 DTMDrainageTables *drainageTablesP,      // ==> Use Drainage Tables
 long   checkOption,                      // ==> Check Option <1,2>
 double falseLowDepth,                    // ==> False Low Depth
 bool   traceOverZeroSlope,               // ==> Trace Over Zero slope Triangles
 double X,                                // ==> Start Trace Point X Coordinate
 double Y,                                // ==> Start Trace Point Y Coordinate
 double Z,                                // ==> Start Trace Point Z Coordinate
 double sumpX,                            // ==> Trace Must Pass Though This Point
 double sumpY,                            // ==> Trace Must Pass Though This Point
 double sumpZ,                            // ==> Trace Must Pass Though This Point
 long   sumpPoint1,                       // ==> Sump Point 1
 long   sumpPoint2,                       // ==> Sump Point 2 ( sump line )
 bool&  tracedToSump                      // <== <False = Does not Trace To Sump> <True = Traces To Sump>
)

    // This Function Traces To A Sump Point
    // Triangle Points pnt1-pnt2-pnt3 Must Be Set Anti Clockwise

    {
    int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   pnt1,pnt2,pnt3,fndType,nextPnt1,nextPnt2,nextPnt3,priorPnt,nextPnt,iteration=0,onLine ;
    long   process,lastPoint,exitPoint,priorPoint,nextPoint,isFalseLow,previousExit,hullFlag ;
    double x,y,nd,firstX,firstY,firstZ,startX,startY,startZ,nextX,nextY,nextZ,lastX,lastY,lastZ;
    double lastAngle=-99.99,saveLastAngle,descentAngle,ascentAngle,slope ;

    bool trgFound,voidTriangle ;
    DTMDrainageTracePoints  tracePoints ;
    long  sumpType ;

    // Log Arguments

    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Checking Trace To Sump Line") ;
        bcdtmWrite_message(0,0,0,"dtm Object         = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"drainageTablesP    = %8ld",drainageTablesP) ;
        bcdtmWrite_message(0,0,0,"checkOption        = %8ld",checkOption) ;
        bcdtmWrite_message(0,0,0,"falseLowDepth      = %10.4lf",falseLowDepth) ;
        bcdtmWrite_message(0,0,0,"traceOverZeroSlope = %8ld",traceOverZeroSlope) ;
        bcdtmWrite_message(0,0,0,"X                  = %12.5lf",X) ;
        bcdtmWrite_message(0,0,0,"Y                  = %12.5lf",Y) ;
        bcdtmWrite_message(0,0,0,"Z                  = %12.5lf",Z) ;
        bcdtmWrite_message(0,0,0,"sumpX              = %12.5lf",sumpX) ;
        bcdtmWrite_message(0,0,0,"sumpY              = %12.5lf",sumpY) ;
        bcdtmWrite_message(0,0,0,"sumpZ              = %12.5lf",sumpZ) ;
        bcdtmWrite_message(0,0,0,"sumpPoint1         = %8ld",sumpPoint1) ;
        bcdtmWrite_message(0,0,0,"sumpPoint2         = %8ld",sumpPoint2) ;
        }

    // Initialise

    tracedToSump = false ;
    saveLastAngle = -DTM_2PYE ;
    process = 0 ;
    firstX = startX = X ;
    firstY = startY = Y ;
    firstZ = startZ = Z ;
    priorPnt = dtmP->nullPnt ;
    nextPnt  = dtmP->nullPnt ;
    lastPoint = dtmP->nullPnt ;
    isFalseLow = 0 ;
    if( falseLowDepth > 0.0 ) isFalseLow = 1 ;

    // Set Sump Type

    sumpType = 0 ;
    if( sumpPoint1 != dtmP->nullPnt && sumpPoint2 == dtmP->nullPnt )
        sumpType = 1 ;
    if( sumpPoint1 != dtmP->nullPnt && sumpPoint2 != dtmP->nullPnt )
        sumpType = 2 ;
    if( ! sumpType )
        {
        bcdtmWrite_message(1,0,0,"No Sump Set") ;
        goto errexit ;
        }

    // Check Check Option Set

    if( checkOption != 1 && checkOption != 2 )
        {
        bcdtmWrite_message(2,0,0,"Check Option Incorrectly Set") ;
        goto errexit ;
        }

    //  Find Triangle Containing Start Point

    if( bcdtmFind_triangleDtmObject(dtmP,X,Y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;

    // Log Start Triangle Coordinates

    if( dbg == 1 )
        {
        bcdtmWrite_message(0,0,0,"Start Triangle Coordinates") ;
        bcdtmWrite_message(0,0,0,"fndType = %2ld",fndType) ;
        if( pnt1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"pnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",pnt1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
        if( pnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"pnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",pnt2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z) ;
        if( pnt3 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"pnt3 = %8ld ** %12.5lf %12.5lf %10.4lf",pnt3,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt3)->z) ;
        }

    // Check For Start Point External To Tin

    if( fndType == 0 )
        {
        bcdtmWrite_message(1,0,0,"Point External To Tin") ;
        goto errexit ;
        }

    // Start Point Internal To Triangle

    if( fndType == 2 )
        {
        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Processing Start Triangle") ;
        if( drainageTablesP != nullptr )
            {
            drainageTablesP->GetTriangleSlopeAndSlopeAngles(pnt1,pnt2,pnt3,trgFound,voidTriangle,ascentAngle,descentAngle,slope) ;
            if( ! trgFound )
                {
                bcdtmWrite_message(2,0,0,"Cannot Find Triangle Number In Drainage Tables") ;
                goto errexit ;
                }
            }
        else
            {
            bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,pnt3,&descentAngle,&ascentAngle,&slope) ;
            }
        saveLastAngle = lastAngle = descentAngle ;

        // Get Trace Point On Triangle Edge

        if( bcdtmDrainage_getFirstTracePointFromTriangleDtmObject(dtmP,2,pnt1,pnt2,pnt3,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ)) goto errexit ;
        if( dbg == 1 )
            {
            bcdtmWrite_message(0,0,0,"First Trace Point = %12.5lf %12.5lf %10.4lf",nextX,nextY,nextZ) ;
            bcdtmWrite_message(0,0,0,"nextPnt1 = %8ld nextPnt2 = %10ld nextPnt3 = %10ld",nextPnt1,nextPnt2,nextPnt3) ;
            }

        //  Store Trace Point For False Low Processing

        if( tracePoints.StoreTracePoint(nextX,nextY,nextZ,nextPnt1,nextPnt2)) goto errexit ;

        //  Initialise Iteration Variables

        pnt1 = nextPnt1 ;
        pnt2 = nextPnt2 ;
        pnt3 = nextPnt3 ;
        lastX = startX ;
        lastY = startY ;
        lastZ = startZ ;
        startX = nextX  ;
        startY = nextY  ;
        }


    // Iteratively Get Next Trace Point

    do
       {
       if( dbg == 2 )
           {
           bcdtmWrite_message(0,0,0,"Processing Next Triangle") ;
           bcdtmWrite_message(0,0,0,"lastX = %12.5lf lastY = %12.5lf ** startX = %12.5lf startY = %12.5lf",lastX,lastY,startX,startY) ;
           bcdtmWrite_message(0,0,0,"iteration[%6ld] ** pnt1 = %9ld pnt2 = %9ld pnt3 = %9ld  ** X = %12.5lf Y = %12.5lf Z = %12.5lf",iteration,pnt1,pnt2,pnt3,startX,startY,startZ) ;
           }
       ++iteration ;
       if( dbg && iteration > 1000 ) goto errexit ;

/*
       // Check If Trace Passes Through Sump Point

       if( sumpType == 1 && pnt1 == sumpPoint1 && pnt2 == dtmP->nullPnt )
           {
           tracedToSump = true ;
           }

       // Check If Trace Flows Down Sump Line

       if( sumpType == 2 && pnt1 != dtmP->nullPnt && pnt2 == dtmP->nullPnt )
          {
           if( ( lastPoint == sumpPoint1 && pnt1 == sumpPoint2 ) || ( lastPoint == sumpPoint2 && pnt1 == sumpPoint1 ))
               {
               bcdtmWrite_message(0,0,0,"Flows Down Sump Line") ;
               tracedToSump = true ;
               }
          }


       // Check If Trace Passes Onto Sump Line

       if( sumpType == 2 && (( pnt1 == sumpPoint1 && pnt2 ==  sumpPoint2 ) || ( pnt1 == sumpPoint2 && pnt2 == sumpPoint1 ) ) )
           {
           bcdtmWrite_message(0,0,0,"Flows Into Sump Line") ;
           tracedToSump = true ;
           }

*/


       if( checkOption == 99 )     // Flows Down Sump Line
           {
           if( sumpPoint1 != dtmP->nullPnt && sumpPoint2 == dtmP->nullPnt )
               {
               if( ( sumpPoint1 == pnt1 && pnt2 == dtmP->nullPnt ) || ( sumpPoint1 == dtmP->nullPnt && sumpPoint1 == pnt2 ) )
                   {
                   bcdtmWrite_message(0,0,0,"Flows Check Option Point") ;
                   tracedToSump = true ;
                   }
               }
           if( sumpPoint1 != dtmP->nullPnt && sumpPoint2 != dtmP->nullPnt )
               {
               if( ( sumpPoint1 == pnt1 && sumpPoint2 == pnt2 ) || ( sumpPoint1 == pnt2 && sumpPoint2 == pnt1 ) )
                   {
                    bcdtmWrite_message(0,0,0,"Flows Check Option Line") ;
                    tracedToSump = true ;
                   }
               }
           if( pnt2 == dtmP->nullPnt && ( pnt1 == sumpPoint1 || pnt1 == sumpPoint2 ))
               {
 //              tracedToSump = true ;
               bcdtmWrite_message(0,0,0,"Flows Check Option") ;
               }
           }

        if( checkOption == 2 )   // Flows Across Drain Point On Sump Line
            {
            nd = bcdtmMath_distanceOfPointFromLine(&onLine,lastX,lastY,startX,startY,sumpX,sumpY,&x,&y) ;
            if( dbg ) bcdtmWrite_message(0,0,0,"nd = %15.10lf ** onLine = %2ld",nd,onLine) ;
            if( onLine && nd < 0.000001 ) tracedToSump = true ;
            }

        //  Check For Sump


        //  Trace To Next Low Point

        if( pnt2 != dtmP->nullPnt ){ if( bcdtmDrainage_traceMaximumDescentFromTriangleEdgeDtmObject(dtmP,drainageTablesP,traceOverZeroSlope,isFalseLow,lastAngle,pnt1,pnt2,pnt3,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process,&exitPoint,&priorPoint,&nextPoint)) goto errexit ; }
        else                       { if( bcdtmDrainage_traceMaximumDescentFromTrianglePointDtmObject(dtmP,drainageTablesP,traceOverZeroSlope,isFalseLow,lastAngle,lastPoint,pnt1,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process,&exitPoint,&priorPoint,&nextPoint)) goto errexit ; }
        if( dbg == 2 ) bcdtmWrite_message(0,0,0,"exitPoint = %9ld nextPnt1 = %9ld nextPnt2 = %9ld nextX = %12.5lf nextY = %12.5lf nextZ = %10.4lf",exitPoint,nextPnt1,nextPnt2,nextX,nextY,nextZ) ;

        if( ( pnt1 == sumpPoint1 && pnt2 == sumpPoint2 ) || ( pnt1 == sumpPoint2 && pnt2 == sumpPoint1 ) )
            {
            tracedToSump = true ;
            }

        //  Set For Next Trace Point

        if( process )
            {
            if( tracePoints.StoreTracePoint(nextX,nextY,nextZ,nextPnt1,nextPnt2)) goto errexit ;
            if( pnt2 == dtmP->nullPnt ) lastPoint = pnt1 ;
            else                        lastPoint = dtmP->nullPnt ;
            pnt1 = nextPnt1 ;
            pnt2 = nextPnt2 ;
            pnt3 = nextPnt3 ;
            if( startX != nextX || startY != nextY ) lastAngle = bcdtmMath_getAngle(startX,startY,nextX,nextY) ;
            else                                     lastAngle = saveLastAngle ;
            saveLastAngle = lastAngle ;
            lastX = startX ;
            lastY = startY ;
            lastZ = startZ ;
            startX = nextX ;
            startY = nextY ;
            startZ = nextZ ;
            if( startX == firstX && startY == firstY ) process = 0 ;
            }
        else
            {

            //  Process Pond Exit Point If It Exists

            if( exitPoint != dtmP->nullPnt  )
                {
                if( dbg == 1 )
                    {
                    bcdtmWrite_message(0,0,0,"lowPoint   = %9ld ** %10.4lf %10.4lf %10.4lf",pnt1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
                    bcdtmWrite_message(0,0,0,"priorPoint = %9ld ** %10.4lf %10.4lf %10.4lf",priorPoint,pointAddrP(dtmP,priorPoint)->x,pointAddrP(dtmP,priorPoint)->y,pointAddrP(dtmP,priorPoint)->z) ;
                    bcdtmWrite_message(0,0,0,"exitPoint  = %9ld ** %10.4lf %10.4lf %10.4lf",exitPoint,pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,pointAddrP(dtmP,exitPoint)->z) ;
                    bcdtmWrite_message(0,0,0,"nextPoint  = %9ld ** %10.4lf %10.4lf %10.4lf",nextPoint,pointAddrP(dtmP,nextPoint)->x,pointAddrP(dtmP,nextPoint)->y,pointAddrP(dtmP,nextPoint)->z) ;
                    }

                //  Check If Exit Point Pond Can Be Processed

                if( pointAddrP(dtmP,exitPoint)->z == startZ || ( falseLowDepth > 0.0  && pointAddrP(dtmP,exitPoint)->z - startZ <= falseLowDepth ))
                    {

                    // Check If Exit Point Has Been Previously Processed

                    previousExit = 0 ;
                    if( tracePoints.CheckForPriorTracePoint(exitPoint)  )
                        previousExit = 1 ;

                    if( dbg ) bcdtmWrite_message(0,0,0,"Previous Exit = %2ld",previousExit) ;
                    if( ! previousExit )
                        {

                        //  Check For Exit Point On A Hull Boundary

                        bcdtmList_testForHullPointDtmObject(dtmP,exitPoint,&hullFlag) ;

                        //  Continue Tracing If Exit Point Not On Hull Boundary

                        if( ! hullFlag )
                            {
                            if( bcdtmDrainage_traceMaximumDescentFromPondExitPointDtmObject(dtmP,drainageTablesP,priorPoint,exitPoint,nextPoint,pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process)) goto errexit ;
				            if( process )
				                {

                                //   Mark Exit Point

                                if( tracePoints.StoreTracePoint(startX,startY,startZ,exitPoint,dtmP->nullPnt)) goto errexit ;

                                //   Set Parameters For Next Maximum Descent Trace

                                lastX = startX ;
                                lastY = startY ;
                                lastZ = startZ ;
                                startX = nextX ;
                                startY = nextY ;
                                startZ = nextZ ;
                                pnt1   = nextPnt1 ;
                                pnt2   = nextPnt2 ;
                                pnt3   = nextPnt3 ;
                                lastPoint = exitPoint ;
                                lastAngle = bcdtmMath_getAngle(pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,nextX,nextY) ;
                                saveLastAngle = lastAngle ;
				                }
                            }
                        }
                    }
                if( bcdtmList_nullTptrListDtmObject(dtmP,exitPoint)) goto errexit ;
                }
            }

        } while ( process && ! tracedToSump ) ;

    // Check For Trace To Sump

    if( sumpPoint1 != dtmP->nullPnt && sumpPoint2 == dtmP->nullPnt )
        {
        if( pnt1 == sumpPoint1 )
            tracedToSump = true ;
        }
    else if ( sumpPoint1 != dtmP->nullPnt && sumpPoint2 != dtmP->nullPnt )
        {
        if( ( pnt1 == sumpPoint1 && pnt2 == sumpPoint2 ) || ( pnt1 == sumpPoint2 && pnt2 == sumpPoint1 ) )
            tracedToSump = true ;
        }

    // Write Trace Result

    if( dbg )
        {
        if( tracedToSump ) bcdtmWrite_message(0,0,0,"Point Traces To Sump") ;
        else               bcdtmWrite_message(0,0,0,"Point Does Not Trace To Sump") ;
        }

    // Clean Up

 cleanup :

    //  Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Trace To Sump Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Trace To Sump Error") ;
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
int bcdtmDrainage_checkTraceToDrainPointDtmObject
(
 BC_DTM_OBJ        *dtmP,                        // ==> Pointer To Dtm Object
 DTMDrainageTables *drainageTablesP,             // ==> Drainage Tables
 double            falseLowDepth,                // ==> False Low Depth
 bool              traceOverZeroSlope,           // ==> Trace Over Zero slope Triangles
 double            X,                            // ==> Start Trace Point X Coordinate
 double            Y,                            // ==> Start Trace Point Y Coordinate
 double            Z,                            // ==> Start Trace Point Z Coordinate
 int               drainPoint,                   // ==> Drain Point
 bool&             tracedToDrain                 // <== < False , True >
)

    // This Function Traces To A Sump Point
    // Triangle Points pnt1-pnt2-pnt3 Must Be Set Clockwise

    {
    int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   pnt1,pnt2,pnt3,nextPnt1,nextPnt2,nextPnt3,priorPnt,nextPnt,iteration=0 ;
    long   process,lastPoint,exitPoint,priorPoint,nextPoint,isFalseLow,previousExit,hullFlag ;
    double firstX,firstY,firstZ,startX,startY,startZ,nextX,nextY,nextZ,lastX,lastY,lastZ;
    double lastAngle=-99.99,saveLastAngle,descentAngle,ascentAngle,slope ;

    bool trgFound,voidTriangle ;
    DTMDrainageTracePoints  tracePoints ;
    long fndType ;

    // Log Arguments

    dbg = DrainageDebug ;
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Checking Trace To Drain Point") ;
        bcdtmWrite_message(0,0,0,"dtm Object         = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"drainageTablesP    = %8ld",drainageTablesP) ;
        bcdtmWrite_message(0,0,0,"falseLowDepth      = %10.4lf",falseLowDepth) ;
        bcdtmWrite_message(0,0,0,"traceOverZeroSlope = %8ld",traceOverZeroSlope) ;
        bcdtmWrite_message(0,0,0,"X                  = %12.5lf",X) ;
        bcdtmWrite_message(0,0,0,"Y                  = %12.5lf",Y) ;
        bcdtmWrite_message(0,0,0,"Z                  = %12.5lf",Z) ;
        bcdtmWrite_message(0,0,0,"drainPoint         = %8ld",drainPoint) ;
        }

    // Initialise

    tracedToDrain = false ;
    saveLastAngle = -DTM_2PYE ;
    process = 0 ;
    firstX = startX = X ;
    firstY = startY = Y ;
    firstZ = startZ = Z ;
    priorPnt = dtmP->nullPnt ;
    nextPnt  = dtmP->nullPnt ;
    lastPoint = dtmP->nullPnt ;
    isFalseLow = 0 ;
    if( falseLowDepth > 0.0 ) isFalseLow = 1 ;

    //  Find Triangle Containing Start Point

    if( bcdtmFind_triangleDtmObject(dtmP,X,Y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;

    // Log Start Triangle Coordinates

    if( dbg == 1 )
        {
        bcdtmWrite_message(0,0,0,"Start Triangle Coordinates") ;
        bcdtmWrite_message(0,0,0,"fndType = %2ld",fndType) ;
        if( pnt1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"pnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",pnt1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
        if( pnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"pnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",pnt2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z) ;
        if( pnt3 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"pnt3 = %8ld ** %12.5lf %12.5lf %10.4lf",pnt3,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt3)->z) ;
        }

    // Check For Start Point External To Tin

    if( fndType == 0 )
        {
        bcdtmWrite_message(1,0,0,"Point External To Tin") ;
        goto errexit ;
        }

    //  Start Point Coincident With Tin Point

    if( fndType == 1 )
        {
        if( pnt1 == drainPoint )
            {
            tracedToDrain = true ;
            goto cleanup ;
            }
        }

    // Start Point Internal To Triangle

    if( fndType == 2 )
        {
        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Processing Start Triangle") ;
        if( drainageTablesP != nullptr )
            {
            drainageTablesP->GetTriangleSlopeAndSlopeAngles(pnt1,pnt2,pnt3,trgFound,voidTriangle,ascentAngle,descentAngle,slope) ;
            if( ! trgFound )
                {
                bcdtmWrite_message(2,0,0,"Cannot Find Triangle Number In Drainage Tables") ;
                goto errexit ;
                }
            }
        else
            {
            bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,pnt1,pnt2,pnt3,&descentAngle,&ascentAngle,&slope) ;
            }
        saveLastAngle = lastAngle = descentAngle ;

        // Get Trace Point On Triangle Edge

        if( bcdtmDrainage_getFirstTracePointFromTriangleDtmObject(dtmP,2,pnt1,pnt2,pnt3,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ)) goto errexit ;
        if( dbg == 1 )
            {
            bcdtmWrite_message(0,0,0,"First Trace Point = %12.5lf %12.5lf %10.4lf",nextX,nextY,nextZ) ;
            bcdtmWrite_message(0,0,0,"nextPnt1 = %8ld nextPnt2 = %10ld nextPnt3 = %10ld",nextPnt1,nextPnt2,nextPnt3) ;
            }

        //  Store Trace Point For False Low Processing

        if( tracePoints.StoreTracePoint(nextX,nextY,nextZ,nextPnt1,nextPnt2)) goto errexit ;

        //  Initialise Iteration Variables

        pnt1 = nextPnt1 ;
        pnt2 = nextPnt2 ;
        pnt3 = nextPnt3 ;
        lastX = startX ;
        lastY = startY ;
        lastZ = startZ ;
        startX = nextX  ;
        startY = nextY  ;
        }

    // Iteratively Get Next Trace Point

    process = 1 ;
    do
        {
        if( dbg == 1 )
            {
            bcdtmWrite_message(0,0,0,"Processing Next Triangle") ;
            bcdtmWrite_message(0,0,0,"lastX = %12.5lf lastY = %12.5lf ** startX = %12.5lf startY = %12.5lf",lastX,lastY,startX,startY) ;
            bcdtmWrite_message(0,0,0,"iteration[%6ld] ** pnt1 = %9ld pnt2 = %9ld pnt3 = %9ld  ** X = %12.5lf Y = %12.5lf Z = %12.5lf",iteration,pnt1,pnt2,pnt3,startX,startY,startZ) ;
            }
        ++iteration ;
        if( dbg && iteration > 1000 ) goto errexit ;

        // Check If Trace Passes Through Sump Point

        if( pnt1 == drainPoint && pnt2 == dtmP->nullPnt )
            {
            tracedToDrain = true ;
            goto cleanup ;
           }

        //  Trace To Next Low Point

        if( pnt2 != dtmP->nullPnt ){ if( bcdtmDrainage_traceMaximumDescentFromTriangleEdgeDtmObject(dtmP,drainageTablesP,traceOverZeroSlope,isFalseLow,lastAngle,pnt1,pnt2,pnt3,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process,&exitPoint,&priorPoint,&nextPoint)) goto errexit ; }
        else                       { if( bcdtmDrainage_traceMaximumDescentFromTrianglePointDtmObject(dtmP,drainageTablesP,traceOverZeroSlope,isFalseLow,lastAngle,lastPoint,pnt1,startX,startY,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process,&exitPoint,&priorPoint,&nextPoint)) goto errexit ; }
        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"process = %2ld ** exitPoint = %9ld nextPnt1 = %9ld nextPnt2 = %9ld nextX = %12.5lf nextY = %12.5lf nextZ = %10.4lf",process,exitPoint,nextPnt1,nextPnt2,nextX,nextY,nextZ) ;

        //  Set For Next Trace Point

        if( process )
            {
            if( tracePoints.StoreTracePoint(nextX,nextY,nextZ,nextPnt1,nextPnt2)) goto errexit ;
            if( pnt2 == dtmP->nullPnt ) lastPoint = pnt1 ;
            else                        lastPoint = dtmP->nullPnt ;
            pnt1 = nextPnt1 ;
            pnt2 = nextPnt2 ;
            pnt3 = nextPnt3 ;
            if( startX != nextX || startY != nextY ) lastAngle = bcdtmMath_getAngle(startX,startY,nextX,nextY) ;
            else                                     lastAngle = saveLastAngle ;
            saveLastAngle = lastAngle ;
            lastX = startX ;
            lastY = startY ;
            lastZ = startZ ;
            startX = nextX ;
            startY = nextY ;
            startZ = nextZ ;
            if( startX == firstX && startY == firstY ) process = 0 ;
            }
        else
            {

            //  Process Pond Exit Point If It Exists

            if( exitPoint != dtmP->nullPnt  )
                {
                if( dbg == 1 )
                    {
                    bcdtmWrite_message(0,0,0,"lowPoint   = %9ld ** %10.4lf %10.4lf %10.4lf",pnt1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
                    bcdtmWrite_message(0,0,0,"priorPoint = %9ld ** %10.4lf %10.4lf %10.4lf",priorPoint,pointAddrP(dtmP,priorPoint)->x,pointAddrP(dtmP,priorPoint)->y,pointAddrP(dtmP,priorPoint)->z) ;
                    bcdtmWrite_message(0,0,0,"exitPoint  = %9ld ** %10.4lf %10.4lf %10.4lf",exitPoint,pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,pointAddrP(dtmP,exitPoint)->z) ;
                    bcdtmWrite_message(0,0,0,"nextPoint  = %9ld ** %10.4lf %10.4lf %10.4lf",nextPoint,pointAddrP(dtmP,nextPoint)->x,pointAddrP(dtmP,nextPoint)->y,pointAddrP(dtmP,nextPoint)->z) ;
                    }

                //  Check If Exit Point Pond Can Be Processed

                if( pointAddrP(dtmP,exitPoint)->z == startZ || ( falseLowDepth > 0.0  && pointAddrP(dtmP,exitPoint)->z - startZ <= falseLowDepth ))
                    {

                    // Check If Exit Point Has Been Previously Processed

                    previousExit = 0 ;
                    if( tracePoints.CheckForPriorTracePoint(exitPoint)  )
                        previousExit = 1 ;

                    if( dbg ) bcdtmWrite_message(0,0,0,"Previous Exit = %2ld",previousExit) ;
                    if( ! previousExit )
                        {

                        //  Check For Exit Point On A Hull Boundary

                        bcdtmList_testForHullPointDtmObject(dtmP,exitPoint,&hullFlag) ;

                        //  Continue Tracing If Exit Point Not On Hull Boundary

                        if( ! hullFlag )
                            {
                            if( bcdtmDrainage_traceMaximumDescentFromPondExitPointDtmObject(dtmP,drainageTablesP,priorPoint,exitPoint,nextPoint,pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,&nextPnt1,&nextPnt2,&nextPnt3,&nextX,&nextY,&nextZ,&process)) goto errexit ;
				            if( process )
				                {

                                //   Mark Exit Point

                                if( tracePoints.StoreTracePoint(startX,startY,startZ,exitPoint,dtmP->nullPnt)) goto errexit ;

                                //   Set Parameters For Next Maximum Descent Trace

                                lastX = startX ;
                                lastY = startY ;
                                lastZ = startZ ;
                                startX = nextX ;
                                startY = nextY ;
                                startZ = nextZ ;
                                pnt1   = nextPnt1 ;
                                pnt2   = nextPnt2 ;
                                pnt3   = nextPnt3 ;
                                lastPoint = exitPoint ;
                                lastAngle = bcdtmMath_getAngle(pointAddrP(dtmP,exitPoint)->x,pointAddrP(dtmP,exitPoint)->y,nextX,nextY) ;
                                saveLastAngle = lastAngle ;
				                }
                            }
                        }
                    }
                if( bcdtmList_nullTptrListDtmObject(dtmP,exitPoint)) goto errexit ;
                }
            }

        } while ( process && ! tracedToDrain ) ;

    // Write Trace Result

    if( dbg )
        {
        if( tracedToDrain ) bcdtmWrite_message(0,0,0,"Point Traces To Drain Point") ;
        else                bcdtmWrite_message(0,0,0,"Point Does Not Trace To Drain Point") ;
        }

    // Clean Up

 cleanup :

    //  Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Trace To Sump Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Trace To Sump Error") ;
    return(ret) ;

    // Error Exit

 errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;

    }
