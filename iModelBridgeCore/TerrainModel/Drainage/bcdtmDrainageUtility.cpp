/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainageUtility.cpp $
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
int bcdtmDrainage_getTriangleEdgeFlowDirectionDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To DTM Object 
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 int               trgPoint1,                  // ==> Triangle Point1 
 int               trgPoint2,                  // ==> Triangle Point2 
 int               trgPoint3,                  // ==> Triangle Point3 
 bool&             voidTriangle,               // <== True If A Void Triangle , False If Not A Void Triangle  
 int&              flowDirection               // <== Flow Direction
)
//
//  Gets The Flow Direction For The Triangle Edge trgPoint1-trgPoint2
//  The Triangle Direction trgPoint1-trgPoint2-trgPoint3 Must Be CCW
//  To Be Compatiable With The Drainage Trace Functions 
//  
//  The Triangle Entries In The Drainage Tables Are In A Clockwise Direction
//
   {
    int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
    long voidTrg ;
    bool trgFound=false ;

    // Log Parameters

    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Getting Triangle Edge Flow Direction") ;
       bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
       bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP) ;
       bcdtmWrite_message(0,0,0,"trgPoint1       = %8ld",trgPoint1) ;
       bcdtmWrite_message(0,0,0,"trgPoint2       = %8ld",trgPoint2) ;
       bcdtmWrite_message(0,0,0,"trgPoint3       = %8ld",trgPoint3) ;
      }
      
    //  Perform Validation Check For Development And Testing Purposes Only
    
    if( cdbg )
      {
       if( bcdtmMath_pointSideOfDtmObject(dtmP,trgPoint1,trgPoint2,trgPoint3) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Triangle Direction Is Not Counter ClockWise") ;
          goto errexit ;
         }
       if( bcdtmList_checkForValidTriangleDtmObject(dtmP,trgPoint1,trgPoint3,trgPoint2) != DTM_SUCCESS ) goto errexit ;
      } 
 
    // Get Flow Direction From Drainage Tables
      
    if( drainageTablesP != nullptr )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Getting Flow Direction From Drainage Tables") ;
       drainageTablesP->GetTriangleEdgeFlowDirection(trgPoint1,trgPoint3,trgPoint2,trgFound,voidTriangle,flowDirection) ;
       if( trgFound == false )
         {
          bcdtmWrite_message(2,0,0,"EFD ** Triangle %8d %8ld %8ld Not Found In Index",trgPoint1,trgPoint3,trgPoint2) ;
          goto errexit ;
         }
         
       //  Confirm Flow Direction For Development Purpose Only
         
       if( cdbg )
         {    
          int flowDirectionTrg = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,trgPoint1,trgPoint2,trgPoint3) ;
          if( flowDirection != flowDirectionTrg )
            {
             bcdtmWrite_message(0,0,0,"Error ** FlowDirection = %2d FlowDirectionTrg = %2ld ** Edge = %8ld %8ld",flowDirection,flowDirectionTrg,trgPoint1,trgPoint2) ;
             bcdtmWrite_message(0,0,0,"Error ** trgPoint1 = %8ld trgPoint3 = %8ld trgPoint3 = %8ld",trgPoint1,trgPoint2,trgPoint3) ;         
             goto errexit ;
            }
         } 
      } 
         
    // Calculate Flow Direction On The Fly
           
    else 
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Flow Direction On The Fly") ;
       voidTriangle = false ;
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,trgPoint1,trgPoint3,trgPoint2,&voidTrg) ) goto errexit ;
       if( voidTrg ) voidTriangle = true ;
       flowDirection = bcdtmDrainage_getTriangleFlowDirectionDtmObject(dtmP,trgPoint1,trgPoint2,trgPoint3) ;
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"flow Direction = %2ld",flowDirection) ;

    // Clean Up

    cleanup :

    // Return 

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Triangle Edge Flow Direction Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Triangle Edge Flow Direction Error") ;
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
int bcdtmDrainage_getTriangleSlopeAndSlopeAnglesDtmObject
(
 BC_DTM_OBJ        *dtmP,                      // ==> Pointer To DTM Object 
 DTMDrainageTables *drainageTablesP,           // ==> Pointer To Drainage Tables
 int               trgPoint1,                  // ==> Triangle Point1 
 int               trgPoint2,                  // ==> Triangle Point2 
 int               trgPoint3,                  // ==> Triangle Point3 
 bool&             voidTriangle,               // <== Void Triangle  
 double&           slope,                      // <== Triangle Slope
 double&           descentAngle,               // <== Triangle Descent Angle
 double&           ascentAngle                 // <== Triangle Ascent Angle
)

//
//  Gets The Triangle Slope And Ascent And Descent Angles
//  The Triangle Direction trgPoint1-trgPoint2-trgPoint3 Must Be Clockwise
//  
//  The Triangle Entries In The Drainage Tables Are In A Clockwise Direction
//
   {
    int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
    long   voidTrg ;
    double trgSlope,trgAscentAngle,trgDescentAngle ;
    bool trgFound=false ;

    // Log Parameters

    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Getting Triangle Slope And Slope Angles") ;
       bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
       bcdtmWrite_message(0,0,0,"drainageTablesP = %p",drainageTablesP) ;
       bcdtmWrite_message(0,0,0,"trgPoint1       = %8ld",trgPoint1) ;
       bcdtmWrite_message(0,0,0,"trgPoint2       = %8ld",trgPoint2) ;
       bcdtmWrite_message(0,0,0,"trgPoint3       = %8ld",trgPoint3) ;
      }
      
    //  Perform Validation Check - Development And Testing Purposes Only
    
    if( cdbg )
      {
       if( bcdtmList_checkForValidTriangleDtmObject(dtmP,trgPoint1,trgPoint2,trgPoint3) != DTM_SUCCESS ) goto errexit ;
      } 
 
    // Get Flow Direction From Drainage Tables
      
    if( drainageTablesP != NULL )
      {
       drainageTablesP->GetTriangleSlopeAndSlopeAngles(trgPoint1,trgPoint2,trgPoint3,trgFound,voidTriangle,ascentAngle,descentAngle,slope) ;
       if( trgFound == false )
         {
          bcdtmWrite_message(2,0,0,"SAA ** Triangle %8d %8ld %8ld Not Found In Index",trgPoint1,trgPoint2,trgPoint3) ;
          goto errexit ;
         }
         
       //  Confirm Slope And Slope Angles - Development And Testing Purposes Only
         
       if( cdbg )
         {
          if( bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,trgPoint1,trgPoint2,trgPoint3,&trgDescentAngle,&trgAscentAngle,&trgSlope) ) goto errexit ;
          if( slope        != trgSlope        ||
              ascentAngle  != trgAscentAngle  ||
              descentAngle != trgDescentAngle 
            ) 
            {
             bcdtmWrite_message(0,0,0,"Error ** With Triangle Slope Or Slope Angle") ;
             bcdtmWrite_message(0,0,0,"slope        = %15.12lf trgSlope        = %15.12lf",slope,trgSlope) ;
             bcdtmWrite_message(0,0,0,"ascentAngle  = %15.12lf trgAscentAngle  = %15.12lf",ascentAngle,trgAscentAngle) ;
             bcdtmWrite_message(0,0,0,"descentAngle = %15.12lf trgDescentAngle = %15.12lf",descentAngle,trgDescentAngle) ;
             bcdtmWrite_message(0,0,0,"Error ** trgPoint1 = %8ld trgPoint3 = %8ld trgPoint3 = %8ld",trgPoint1,trgPoint2,trgPoint3) ;         
             goto errexit ;
            }
         } 
      } 
         
    // Calculate Slope And Slope Angles On The Fly
           
    else 
      {
       voidTriangle = false ;
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,trgPoint1,trgPoint2,trgPoint3,&voidTrg) ) goto errexit ;
       if( voidTrg ) voidTriangle = true ;
       if( bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject(dtmP,trgPoint1,trgPoint2,trgPoint3,&trgDescentAngle,&trgAscentAngle,&trgSlope) ) goto errexit ;
       slope        = trgSlope ;
       ascentAngle  = trgAscentAngle ;
       descentAngle = trgDescentAngle ;
      } 
      
    //  Log Slope And Slope Angles
    
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"slope = %12.5lf ascentAngle = %15.12lf descentAngle = %15.12lf",slope,ascentAngle,descentAngle) ;
      }  

    // Clean Up

    cleanup :

    // Return 

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Triangle Edge Flow Direction Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Triangle Edge Flow Direction Error") ;
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
int bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject
(
 BC_DTM_OBJ *dtmP,      /* ==> Pointer To Tin Object                                        */
 double sRadX,          /* ==> Starting x Coordinate Of Radial                              */
 double sRadY,          /* ==> Starting y Coordinate Of Radial                              */
 double eRadX,          /* ==> Ending x Coordinate Of Radial                                */
 double eRadY,          /* ==> Ending y Coordinate Of Radial                                */
 long dtmPnt1,          /* ==> Triangle Line Point1                                         */
 long dtmPnt2,          /* ==> Triangle Line Point2                                         */
 double *xP,            /* <== x Intersect Coordinate                                       */
 double *yP,            /* <== y Intersect Coordinate                                       */
 double *zP,            /* <== z Intersect Coordinate                                       */
 long *intPntP          /* <== Set To Tin Point If Intersect Is Coincident with Tin Point   */
)
/*
** This Function Calulates The Intersection Of The Radial With Tin Line <dtmPnt1,dtmPnt2>
*/
{
 double d1,d2,dx,dy,dz ;
/*
** Initialise
*/
 *intPntP = dtmP->nullPnt ;
/*
** Calculate Intersection Of Radial <(sRadX,sRadY),(eRadX,eRadY)> With Tin Line <dtmPnt1,dtmPnt2>
*/
 bcdtmMath_normalIntersectCordLines(sRadX,sRadY,eRadX,eRadY,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y,xP,yP) ;
 dx = pointAddrP(dtmP,dtmPnt2)->x - pointAddrP(dtmP,dtmPnt1)->x ;
 dy = pointAddrP(dtmP,dtmPnt2)->y - pointAddrP(dtmP,dtmPnt1)->y ;
 dz = pointAddrP(dtmP,dtmPnt2)->z - pointAddrP(dtmP,dtmPnt1)->z ;
 if( fabs(dx) >= fabs(dy) ) *zP = pointAddrP(dtmP,dtmPnt1)->z +  dz * (*xP - pointAddrP(dtmP,dtmPnt1)->x) / dx ;
 else                       *zP = pointAddrP(dtmP,dtmPnt1)->z +  dz * (*yP - pointAddrP(dtmP,dtmPnt1)->y) / dy ;
/*
** Check For Closeness To Tin Points
*/
 d1 = bcdtmMath_distance(*xP,*yP,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y) ;
 d2 = bcdtmMath_distance(*xP,*yP,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y) ;
 if     ( d1 <= d2 && d1 < dtmP->mppTol )
   {
    *xP = pointAddrP(dtmP,dtmPnt1)->x ;
    *yP = pointAddrP(dtmP,dtmPnt1)->y ;
    *zP = pointAddrP(dtmP,dtmPnt1)->z ;
    *intPntP = dtmPnt1 ;
   }
 else if( d2 <  d1 && d2 < dtmP->mppTol )
   {
    *xP = pointAddrP(dtmP,dtmPnt2)->x ;
    *yP = pointAddrP(dtmP,dtmPnt2)->y ;
    *zP = pointAddrP(dtmP,dtmPnt2)->z ;
    *intPntP = dtmPnt2 ;
   }
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_intersectCordLines
(
 double X1,
 double Y1,
 double X2,
 double Y2,
 double X3,
 double Y3,
 double X4,
 double Y4,
 double *X5,
 double *Y5
)
/*
** This Function Intersect Cord Lines
*/
{
 double n1,n2 ;
/*
** Intersect Cord Lines
*/
 if( ! bcdtmMath_intersectCordLines(X1,Y1,X2,Y2,X3,Y3,X4,Y4,X5,Y5) )
   {
    n1 = bcdtmMath_normalDistanceToCordLine(X1,Y1,X2,Y2,X3,Y3) ;
    n2 = bcdtmMath_normalDistanceToCordLine(X1,Y1,X2,Y2,X4,Y4) ;
    if( n1 <= n2 ) { *X5 = X3 ; *Y5 = Y3 ; }
    else           { *X5 = X4 ; *Y5 = Y4 ; }
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
int bcdtmDrainage_getTriangleDescentAndAscentAnglesDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       P1,
 long       P2,
 long       P3,
 double     *descentAngleP,
 double     *ascentAngleP,
 double     *trgSlopeP
)
/*
** This Function Returns The Triangle Slope And Ascent And Descent Angles
*/
{
 double slopeDegrees,slopePercentage,X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,ca,cb,cc,cd,d,c,axy,xMin,yMin,zMin ;
/*
** Initialise
*/
 *trgSlopeP     = 0.0 ;
 *descentAngleP = 0.0 ;
 *ascentAngleP  = DTM_2PYE/2.0 ;
/*
** Get Triangle Attributes
*/
 X1 = pointAddrP(dtmP,P1)->x ; Y1 = pointAddrP(dtmP,P1)->y ; Z1 = pointAddrP(dtmP,P1)->z ;
 X2 = pointAddrP(dtmP,P2)->x ; Y2 = pointAddrP(dtmP,P2)->y ; Z2 = pointAddrP(dtmP,P2)->z ;
 X3 = pointAddrP(dtmP,P3)->x ; Y3 = pointAddrP(dtmP,P3)->y ; Z3 = pointAddrP(dtmP,P3)->z ;
 if( Z1 != Z2 || Z2 != Z3 ) 
   {
/*
**  Normalise Cordinates
*/
    xMin = X1 ; if( X2 < xMin ) xMin = X2 ; if( X3 < xMin ) xMin = X3 ;
    yMin = Y1 ; if( Y2 < yMin ) yMin = Y2 ; if( Y3 < yMin ) yMin = Y3 ;
    zMin = Z1 ; if( Z2 < zMin ) zMin = Z2 ; if( Z3 < zMin ) zMin = Z3 ;
    X1 -= xMin ; X2 -= xMin ; X3 -= xMin ;
    Y1 -= yMin ; Y2 -= yMin ; Y3 -= yMin ;
    Z1 -= zMin ; Z2 -= zMin ; Z3 -= zMin ;
/*
**  Calculate Plane Coefficients
*/
    bcdtmMath_calculatePlaneCoefficients(X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,&ca,&cb,&cc,&cd) ;
    if( cd == 0.0 )  cd = 0.0000000001 ;
    ca = ca / cd ; 
    cb = cb / cd ; 
    cc = cc / cd ; 
    d  = 1.0 / sqrt( ca*ca + cb*cb + cc*cc ) ;
    c  = cc * d ;       
    if(  ca != 0.0 || cb != 0.0 ) 
      {
/*
**     Calulate Triangle Slope
*/
       slopeDegrees    = acos(fabs(c))  ;
       slopePercentage = tan(slopeDegrees) ;
       *trgSlopeP      = slopePercentage ;
       if( *trgSlopeP < 0.0 ) *trgSlopeP = - *trgSlopeP ;
/*
**     Get Angle Of Normal Projected On XY Plane
*/
       axy = atan2(cb,ca) ;
       if( axy < 0.0 ) axy += DTM_2PYE ;
/*
**     Set Return Values
*/
       if( c >= 0.0 )
         {
          *descentAngleP = axy ;
          *ascentAngleP  = *descentAngleP + DTM_2PYE /2.0 ;
          if( *ascentAngleP > DTM_2PYE ) *ascentAngleP = *ascentAngleP - DTM_2PYE ;
         }
       else
         {
          *ascentAngleP  = axy ;
          *descentAngleP = *ascentAngleP + DTM_2PYE /2.0 ;
          if( *descentAngleP > DTM_2PYE ) *descentAngleP = *descentAngleP - DTM_2PYE ;
         }
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
int bcdtmDrainage_getTriangleFlowDirectionDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       P1,
 long       P2,
 long       P3
)
/*
** This Function Determines Flow Of A Triangle Edge
** Points P1-P2 Are The Edge And P3 Is The Triangle Apex
**
**  Return  = -1  Flow Direction Away From P1P2
**          =  0  Flow Parallel to P1P2
**          =  1  Flow Direction To  P1P2   
*/
{
 long   onLine ;
 double dx,dy,dz,nd,Xi,Yi,Zi ;
 
/*
** Initialise
*/
 dx = pointAddrP(dtmP,P2)->x - pointAddrP(dtmP,P1)->x ;
 dy = pointAddrP(dtmP,P2)->y - pointAddrP(dtmP,P1)->y ;
 dz = pointAddrP(dtmP,P2)->z - pointAddrP(dtmP,P1)->z ;
/*
** Calculate Orthogonal Intersection Of P3 on P1,P2
*/
 nd = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,&Xi,&Yi) ;
/*
** Interpolate z Value On Line P1P2
*/
 if( fabs(dx) > fabs(dy) ) Zi = pointAddrP(dtmP,P1)->z + (Xi-pointAddrP(dtmP,P1)->x) / dx * dz ;
 else                      Zi = pointAddrP(dtmP,P1)->z + (Yi-pointAddrP(dtmP,P1)->y) / dy * dz ;
/*
** Job Completed
*/
 if     ( fabs(Zi-pointAddrP(dtmP,P3)->z) < 0.00001 ) return( 0) ;
 else if( Zi >  pointAddrP(dtmP,P3)->z )              return(-1) ;
 else if( Zi <  pointAddrP(dtmP,P3)->z )              return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_calculateIntersectOfApexRadialWithTriangleBaseDtmObject
(
 BC_DTM_OBJ *dtmP,      /* ==> Pointer To Tin Object                                        */
 long apexPnt,          /* ==> Triangle Apex Point                                          */
 long basePnt1,         /* ==> Triangle Base Point                                          */
 long basePnt2,         /* ==> Triangle Base Point                                          */
 double angle,          /* ==> Angle Of Intersect Radial                                    */
 double *xP,            /* <== x Intersect Coordinate                                       */
 double *yP,            /* <== y Intersect Coordinate                                       */
 double *zP,            /* <== z Intersect Coordinate                                       */
 long *intPntP          /* <== Set To Base Point If Intersect Is Coincident with Base Point */
)
/*
** This Function Calculates The Intersection Of A Radial With Tin Line <basePnt1,basePnt2> 
*/
{
 double d1,d2,dx,dy,dz,radX,radY,radius ;
/*
** Initialise
*/
 *xP = 0.0 ;
 *yP = 0.0 ;
 *zP = 0.0 ;
 *intPntP = dtmP->nullPnt ;
/*
** Create Radial Intersect Vector
*/
 dx = dtmP->xMax - dtmP->xMin ;
 dy = dtmP->yMax - dtmP->yMin ;
 radius = sqrt(dx*dx + dy*dy) ;
 radX = pointAddrP(dtmP,apexPnt)->x + radius * cos(angle) ;
 radY = pointAddrP(dtmP,apexPnt)->y + radius * sin(angle) ;
/*
** Calculate Intersection Of Radial From apexPnt With basePnt1-basePnt2
*/
 bcdtmMath_normalIntersectCordLines(pointAddrP(dtmP,apexPnt)->x,pointAddrP(dtmP,apexPnt)->y,radX,radY,pointAddrP(dtmP,basePnt1)->x,pointAddrP(dtmP,basePnt1)->y,pointAddrP(dtmP,basePnt2)->x,pointAddrP(dtmP,basePnt2)->y,xP,yP) ;
 dx = pointAddrP(dtmP,basePnt2)->x - pointAddrP(dtmP,basePnt1)->x ;
 dy = pointAddrP(dtmP,basePnt2)->y - pointAddrP(dtmP,basePnt1)->y ;
 dz = pointAddrP(dtmP,basePnt2)->z - pointAddrP(dtmP,basePnt1)->z ;
 if( fabs(dx) >= fabs(dy) ) *zP = pointAddrP(dtmP,basePnt1)->z +  dz * (*xP - pointAddrP(dtmP,basePnt1)->x) / dx ;
 else                       *zP = pointAddrP(dtmP,basePnt1)->z +  dz * (*yP - pointAddrP(dtmP,basePnt1)->y) / dy ;
/*
** Check For Closeness To Base Points
*/
 d1 = bcdtmMath_distance(*xP,*yP,pointAddrP(dtmP,basePnt1)->x,pointAddrP(dtmP,basePnt1)->y) ;
 d2 = bcdtmMath_distance(*xP,*yP,pointAddrP(dtmP,basePnt2)->x,pointAddrP(dtmP,basePnt2)->y) ;
 if     ( d1 <= d2 && d1 < dtmP->ppTol )
   {
    *xP = pointAddrP(dtmP,basePnt1)->x ;
    *yP = pointAddrP(dtmP,basePnt1)->y ;
    *zP = pointAddrP(dtmP,basePnt1)->z ;
    *intPntP = basePnt1 ;
   }
 else if( d2 <  d1 && d2 < dtmP->ppTol )
   {
    *xP = pointAddrP(dtmP,basePnt2)->x ;
    *yP = pointAddrP(dtmP,basePnt2)->y ;
    *zP = pointAddrP(dtmP,basePnt2)->z ;
    *intPntP = basePnt2 ;
   }
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_calculateAngleIntersectOfRadialFromTriangleEdgeWithTriangleDtmObject
(
 BC_DTM_OBJ *dtmP,                   // ==> Pointer To DTM
 long       startPnt1,               // ==> Triangle Edge Point
 long       startPnt2,               // ==> Triangle Edge Point
 long       startPnt3,               // ==> Triangle Apex Point
 double     startX,                  // ==> X Coordinate Of Start Point On Edge startPnt1-startPnt2
 double     startY,                  // ==> Y Coordinate Of Start Point On Edge startPnt1-startPnt2
 double     angle,                   // ==> Angle Of Intersect Radial
 double     *xP,                     // <== x Intersect Coordinate
 double     *yP,                     // <== y Intersect Coordinate
 double     *zP,                     // <== z Intersect Coordinate
 long       *nextPnt1P,              // <==  Next Edge Point
 long       *nextPnt2P,              // <==  Next Edge Point
 long       *nextApexP               // <==  Next Apex Point
)
//
//  Direction Of Triangle startPnt1,startPnt2,startPnt3 Must Be Counter Clockwise
//
    {
    int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
	int sdof ;
    long intPnt ;
	double dx,dy,radius ;

    // Log Arguments

    if ( dbg )
       {
       bcdtmWrite_message(0,0,0,"Calculating Angle Intersect From Triangle Edge With Triangle") ; 
       bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ; 
       bcdtmWrite_message(0,0,0,"startPnt1   = %8ld",startPnt1) ; 
       bcdtmWrite_message(0,0,0,"startPnt2   = %8ld",startPnt2) ; 
       bcdtmWrite_message(0,0,0,"startPnt3   = %8ld",startPnt3) ; 
       bcdtmWrite_message(0,0,0,"startX      = %12.5lf",startX) ; 
       bcdtmWrite_message(0,0,0,"startY      = %12.5lf",startY) ; 
       bcdtmWrite_message(0,0,0,"angle       = %12.10lf",angle) ; 
       } 

	// Initialise

	*xP = *yP = *zP = 0.0 ;
	*nextPnt1P = *nextPnt2P = *nextApexP = dtmP->nullPnt ;
    dx = dtmP->xMax - dtmP->xMin ;
    dy = dtmP->yMax - dtmP->yMin ;

    //  Check For Valid Triangle

    if( cdbg )
        {
        if( bcdtmList_checkForValidTriangleDtmObject(dtmP,startPnt1,startPnt3,startPnt2))
            {
             bcdtmWrite_message(1,0,0,"Invalid Triangle") ;
             goto errexit ;
            }  
        }  

	//  Calculate Radial Out From Start Point At Angle

    radius = sqrt(dx * dx + dy * dy) ;
    dx = startX + radius * cos(angle) ;
    dy = startY + radius * sin(angle) ;

    //  Determine Triangle Intersect Edge

    sdof = bcdtmMath_sideOf(startX,startY,dx,dy,pointAddrP(dtmP,startPnt3)->x,pointAddrP(dtmP,startPnt3)->y) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersect Edge Side Of = %2d",sdof) ;

    //  Radial Colinear with Triangle Apex Point startPnt3

    if( sdof == 0 )                          
        { 
        *nextPnt1P = startPnt3 ; 
        *xP = pointAddrP(dtmP,startPnt3)->x ; 
        *yP = pointAddrP(dtmP,startPnt3)->y ; 
        *zP = pointAddrP(dtmP,startPnt3)->z ; 
        }

    // Radial Intersects Triangle Edge startPnt2 - startPnt3

    if( sdof >  0 ) 
        { 
        if( bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP,startX,startY,dx,dy,startPnt3,startPnt2,xP,yP,zP,&intPnt)) goto errexit ;
        if( dbg ) bcdtmWrite_message(0,0,0,"intPnt = %10ld",intPnt) ;
        if( intPnt != dtmP->nullPnt )
            {
            *nextPnt1P = intPnt ;
            }
        else
            {
            *nextPnt1P = startPnt3 ;
            *nextPnt2P = startPnt2 ; 
            if(( *nextApexP = bcdtmList_nextAntDtmObject(dtmP,*nextPnt1P,*nextPnt2P)) < 0 ) goto errexit ;
            }
        }

    // Radial Intersects Triangle Edge startPnt1 - startPnt3

    if( sdof <  0 ) 
        { 
        if( bcdtmDrainage_calculateIntersectOfRadialWithTinLineDtmObject(dtmP,startX,startY,dx,dy,startPnt1,startPnt3,xP,yP,zP,&intPnt)) goto errexit ;
        if( intPnt != dtmP->nullPnt )
            {
            *nextPnt1P = intPnt ;
            }
        else
            {
            *nextPnt1P = startPnt1 ;
            *nextPnt2P = startPnt3 ; 
            if(( *nextApexP = bcdtmList_nextAntDtmObject(dtmP,*nextPnt1P,*nextPnt2P)) < 0 ) goto errexit ;
            }
        }

    //  Clean Up

cleanup : ;

    //  Return

    if ( dbg && ret == DTM_SUCCESS )
        bcdtmWrite_message(0,0,0,"Calculating Angle Intersect From Triangle Edge With Triangle Completed") ; 
    if ( dbg && ret != DTM_SUCCESS )
        bcdtmWrite_message(0,0,0,"Calculating Angle Intersect From Triangle Edge With Triangle Error") ; 

	return ret ;

errexit : 

    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;

    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_internallyCleanPointArrayPolygon
(
 DPoint3d    **polyPtsPP,
 long        *numPolyPtsP,
 double      ppTol
)
/*
** This Function Internally Cleans A DPoint3d Array Of Points
*/
{
 int          ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long         sp,np,lp=0,cln,snp=0,spnt,numFeatures,scanDirection,polyFound ;
 DTMDirection direction;
 long         node,process,startPoint,startNextPoint=0,startDirection=0 ;
 double       area,featureArea=0.0 ;
 DPoint3d     *p3dP,*p3d1P,polyPoint[2] ;
 BC_DTM_OBJ   *dtmP=NULL ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Cleaning Internally DPoint3d Polygon") ;
    bcdtmWrite_message(0,0,0,"polyPtsPP      = %p",*polyPtsPP) ;
    bcdtmWrite_message(0,0,0,"numPolyPtsP    = %4ld",*numPolyPtsP) ;
    bcdtmWrite_message(0,0,0,"ppTol          = %12.10lf",ppTol) ;
   }
/*
** Check For Existence Of DPoint3d Points
*/
 if( *polyPtsPP == NULL || *numPolyPtsP < 3 ) 
   { 
    bcdtmWrite_message(1,0,0,"Less Than 3 Polygon Points") ;
    goto errexit ; 
   }
 if( (*polyPtsPP)->x != (*polyPtsPP+*numPolyPtsP-1)->x || (*polyPtsPP)->y != (*polyPtsPP+*numPolyPtsP-1)->y ) 
   { 
    bcdtmWrite_message(1,0,0,"Polygon Does Not Close") ;
    goto errexit ;
   }
/*
** Get direction Of Polygon
*/
 bcdtmMath_getPolygonDirectionP3D(*polyPtsPP,*numPolyPtsP,&direction,&area) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygon direction = %2ld area = %10.4lf",direction,area) ;
/*
** Set direction Of Polygon Anti Clockwise
*/
 if( direction == DTMDirection::Clockwise )  bcdtmMath_reversePolygonDirectionP3D(*polyPtsPP,*numPolyPtsP) ;
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
/*
** Set Memory Allocation Parameters For Data Object
*/
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,*numPolyPtsP*2,*numPolyPtsP) ) goto errexit ;
/*
**  Store Polygon Segments In DTM Object As Break Lines
*/
 for( p3dP = *polyPtsPP ; p3dP < *polyPtsPP + *numPolyPtsP - 1 ; ++p3dP )
   {
    p3d1P = p3dP + 1 ;
    polyPoint[0].x = p3dP->x  ; polyPoint[0].y = p3dP->y  ; polyPoint[0].z = p3dP->z ;
    polyPoint[1].x = p3d1P->x ; polyPoint[1].y = p3d1P->y ; polyPoint[1].z = p3d1P->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,polyPoint,2)) goto errexit ; 
   }
/*
** Triangulate Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ; 
 dtmP->ppTol = dtmP->plTol = ppTol ;
 if( bcdtmObject_createTinDtmObjectOverload (dtmP,1,0.0,false,false)) goto errexit ;
/*
** Find A Point That Has Only One Two Feature
*/
 startPoint = dtmP->nullPnt ;
 for( spnt = 0 ; spnt < dtmP->numPoints ; ++spnt ) nodeAddrP(dtmP,spnt)->PRGN = 1 ;
 for( spnt = 0 ; spnt < dtmP->numPoints ; ++spnt )
   {
    if( nodeAddrP(dtmP,spnt)->PRGN )
      {
/*
**     Count Number Of Features At Point
*/
       numFeatures = 0 ;
       cln = nodeAddrP(dtmP,spnt)->fPtr  ;
       while ( cln != dtmP->nullPtr )
         {
          ++numFeatures ;
          if( flistAddrP(dtmP,cln)->nextPnt != dtmP->nullPnt ) snp = flistAddrP(dtmP,cln)->nextPnt ;
          cln = flistAddrP(dtmP,cln)->nextPtr ;
         } 
/*
**     Get Next Point
*/
       if( numFeatures == 2 )
         {
/*
**        Scan Internal To Feature
*/
          process = 1 ;   
          polyFound = 0 ;  
          scanDirection = 1 ;
          while ( process )
            {
             sp = spnt ;  
             np = snp  ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Scan direction = %2ld Start Point = %6ld Next Point = %6ld Number Of Features = %2ld",scanDirection,spnt,np,numFeatures) ;
             do
               {
                nodeAddrP(dtmP,sp)->tPtr = np ;
                if( scanDirection == 1 ) { if(( lp = bcdtmList_nextClkDtmObject(dtmP,np,sp)) < 0  ) goto errexit ; }
                if( scanDirection == 2 ) { if(( lp = bcdtmList_nextAntDtmObject(dtmP,np,sp)) < 0  ) goto errexit ; }
                while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,np,lp) )
                  {
                   if( scanDirection == 1 ) { if(( lp = bcdtmList_nextClkDtmObject(dtmP,np,lp)) < 0 ) goto errexit ; }
                   if( scanDirection == 2 ) { if(( lp = bcdtmList_nextAntDtmObject(dtmP,np,lp)) < 0 ) goto errexit ; }
                  }
                sp = np ;
                np = lp ;
               } while ( sp != spnt && nodeAddrP(dtmP,sp)->tPtr == dtmP->nullPnt ) ;
/*
**           Check Polygon Closes Correctly
*/
             if( sp != spnt )
               {
                if( dbg ) 
                  {
                   bcdtmWrite_message(0,0,0,"Polygon Does Not Close Correctly ** scanDirection = %2ld",scanDirection) ;
                   bcdtmWrite_message(0,0,0,"Polygon Start Point = %7ld ** %10.4lf %10.4lf %10.4lf",spnt,pointAddrP(dtmP,spnt)->x,pointAddrP(dtmP,spnt)->y,pointAddrP(dtmP,spnt)->z) ;
                   bcdtmWrite_message(0,0,0,"Polygon End   Point = %7ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                  } 
                for( node = 0 ; node < dtmP->numPoints ; ++node ) nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
                if     ( scanDirection == 1 ) scanDirection = 2 ;
                else if( scanDirection == 2 ) process = 0 ;
               }
             else { polyFound = 1 ; process = 0 ; }
            } 
/*
**        Determine direction Of Tptr Polgon
*/
          if( polyFound )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Calculating area And direction Tptr Polygon") ;
             bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,spnt,&area,&direction) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Clean Polygon direction = %2ld area = %10.4lf",direction,area) ;
/*
**           Check For Feature Start Point
*/
             if( startPoint == dtmP->nullPnt || area > featureArea )
               {
                startPoint     = spnt ;
                startNextPoint = snp ;
                startDirection = scanDirection ;
                featureArea    = area ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Start Point = %6ld Next Point = %6ld",startPoint,startNextPoint,featureArea) ;
               }
/*
**           Mark Points And Null Out Tptr List
*/
             if(dbg) bcdtmWrite_message(0,0,0,"Marking Points On Feature") ;
             sp = spnt ;
             do
               {
                np = nodeAddrP(dtmP,sp)->tPtr ;
                nodeAddrP(dtmP,sp)->PRGN = 0  ; 
                nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt  ; 
                sp = np ;
               } while ( sp != spnt ) ; 
            }
         }
      } 
   }
/*
** Write Status
*/
 if( dbg && startPoint == dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"Polygon Not Cleaned") ;
/*
**  If Start Point Found Copy Feature Polygon
*/
 if( startPoint != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Polygon Cleaned Start Point = %6ld",startPoint) ;
/*
** Free Memory
*/
    free(*polyPtsPP) ; 
    *polyPtsPP = NULL ;
    *numPolyPtsP = 0 ;
    spnt = startPoint ;
    np   = startNextPoint ; 
    scanDirection = startDirection ;
    sp = spnt ;  
    do
      {
       nodeAddrP(dtmP,sp)->tPtr = np ;
       if( scanDirection == 1 ) { if(( lp = bcdtmList_nextClkDtmObject(dtmP,np,sp)) < 0  ) goto errexit ; }
       if( scanDirection == 2 ) { if(( lp = bcdtmList_nextAntDtmObject(dtmP,np,sp)) < 0  ) goto errexit ; }
       while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,lp,np) )
         {
          if( scanDirection == 1 ) { if(( lp = bcdtmList_nextClkDtmObject(dtmP,np,lp)) < 0 ) goto errexit ; }
          if( scanDirection == 2 ) { if(( lp = bcdtmList_nextAntDtmObject(dtmP,np,lp)) < 0 ) goto errexit ; }
         }
       sp = np ;
       np = lp ;
      } while ( sp != spnt && nodeAddrP(dtmP,sp)->tPtr == dtmP->nullPnt ) ;
/*
**  Copy Tptr List To Cleaned Feature Points
*/
    if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,spnt,polyPtsPP,numPolyPtsP)) goto errexit ;
   }
/*
** Free Memory
*/
 cleanup :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ; 
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Internally DPoint3d Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Internally DPoint3d Polygon Error") ;
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
int bcdtmDrainage_insertPointIntoTinLineDtmObject
(
 BC_DTM_OBJ *dtmP,                   // ==> Pointer To DTM
 long       point,                   // ==> Tin Point
 long       linePoint1,              // ==> Tin Line Point
 long       linePoint2               // ==> Tin Line Point
)
    {
    int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    int antPnt,clkPnt,savePoint ;
    long voidLine ;

    // Log Function Arguments

    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Inserting Point Into Tin Line") ;
        bcdtmWrite_message(0,0,0,"point        = %8ld",point) ;
        bcdtmWrite_message(0,0,0,"linePoint1   = %8ld",linePoint1) ;
        bcdtmWrite_message(0,0,0,"linePoint2   = %8ld",linePoint2) ;
        }

    //  Test For Void Line

    bcdtmList_testForVoidLineDtmObject(dtmP,linePoint1,linePoint2,&voidLine) ;
    if( voidLine )
        {
        bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,point)->PCWD) ; 
        }

    // Insert Into Internal Tin Line

    if( nodeAddrP(dtmP,linePoint1)->hPtr != linePoint2 && nodeAddrP(dtmP,linePoint2)->hPtr != linePoint1 )
        {
        if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,linePoint1,linePoint2)) < 0 ) goto errexit ;
        if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,linePoint1,linePoint2)) < 0 ) goto errexit ;
  	    if(bcdtmList_deleteLineDtmObject(dtmP,linePoint1,linePoint2)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,linePoint1,point,antPnt)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point,linePoint1,dtmP->nullPnt)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,linePoint2,point,clkPnt)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point,linePoint2,linePoint1)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,point,linePoint2)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point,antPnt,linePoint1)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,clkPnt,point,linePoint1)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point,clkPnt,linePoint2)) goto errexit ; 
        if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,linePoint1,linePoint2) )
            {
            if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,linePoint1,linePoint2,point)) goto errexit ; 
            }
        }

    //  Insert Into External Tin Line

    else
        { 
        if( nodeAddrP(dtmP,linePoint2)->hPtr == linePoint1 )
            {
            savePoint  = linePoint1 ;
            linePoint1 = linePoint2 ;
            linePoint2 = savePoint ; 
            } 
	    if( (antPnt = bcdtmList_nextAntDtmObject(dtmP,linePoint1,linePoint2))   < 0 ) goto errexit ; 
	    if(bcdtmList_deleteLineDtmObject(dtmP,linePoint1,linePoint2)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,linePoint1,point,antPnt)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point,linePoint1,dtmP->nullPnt)) goto errexit ; 
	    if(bcdtmList_insertLineBeforePointDtmObject(dtmP,linePoint2,point,antPnt)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point,linePoint2,linePoint1)) goto errexit ; 
        if(bcdtmList_insertLineAfterPointDtmObject(dtmP,antPnt,point,linePoint2)) goto errexit ; 
	    if(bcdtmList_insertLineAfterPointDtmObject(dtmP,point,antPnt,linePoint1)) goto errexit ; 
        if(bcdtmList_testForDtmFeatureLineDtmObject(dtmP,linePoint1,linePoint2) )
            {
            if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,linePoint1,linePoint2,point)) goto errexit ;
            }  
	    nodeAddrP(dtmP,linePoint1)->hPtr = point ;
	    nodeAddrP(dtmP,point)->hPtr = linePoint2 ;
        }

    // Clean Up

cleanup :

// Return

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Point Into Tin Line Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Point Into Tin Line Error") ;
    return(ret) ;

// Error Exit

 errexit : 
     if( ret == DTM_SUCCESS ) ret = DTM_ERROR ; 
     goto cleanup ;
}
