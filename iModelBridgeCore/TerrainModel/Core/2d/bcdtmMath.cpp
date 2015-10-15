/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmMath.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 

//#pragma optimize( "p", on )
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmMath_normaliseCoordinatesDtmObject(BC_DTM_OBJ *dtmP)
{
 int    ret=DTM_SUCCESS ;
 long   ofs,point,pointPartition ;
 double large ;
 DTM_TIN_POINT *pointP ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Process For dtmState DTMState::Data
*/
 if( dtmP->dtmState == DTMState::Data )
   {
/*
**  Normalise Dtm Coordinates
*/
    large = fabs(dtmP->xMin) ;
    if( fabs(dtmP->xMax) > large ) large = fabs(dtmP->xMax) ;
    if( fabs(dtmP->yMin) > large ) large = fabs(dtmP->yMin) ;
    if( fabs(dtmP->yMax) > large ) large = fabs(dtmP->yMax) ;
    if( large == 0.0 ) large = 1.0 ;
    if( large <=  1000.0 )
      {
       large = large * 100000.0 ;
      }
    else
      {
       large = large * 100000.0 ;
      }
/*
**  Scan DTM Coordinates
*/
    ofs  = 0 ;
    pointPartition = 0 ;
    pointP = dtmP->pointsPP[pointPartition] ;
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       pointP->x = pointP->x / large ; pointP->x = pointP->x * large ;
       pointP->y = pointP->y / large ; pointP->y = pointP->y * large ;
       // Shouldn't be a need to normalize the Zs.
       ++ofs ;
       if( ofs == dtmP->pointPartitionSize )
         {
          ofs = 0 ;
          ++pointPartition ;
          pointP = dtmP->pointsPP[pointPartition] ;
         }
       else ++pointP ;
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
BENTLEYDTM_EXPORT int bcdtmMath_setBoundingCubeDtmObject(BC_DTM_OBJ *dtmP)
{
 int    ret=DTM_SUCCESS ;
 long   point       ;
 DTM_TIN_POINT *pointP ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Initialise
*/
 dtmP->xMin   = dtmP->yMin   = dtmP->zMin   = 0.0 ;
 dtmP->xMax   = dtmP->yMax   = dtmP->zMax   = 0.0 ;
 dtmP->xRange = dtmP->yRange = dtmP->zRange = 0.0 ;
/*
**  Scan DTM Coordinates
*/
 if( dtmP->numPoints > 0 )
   {
    pointP = dtmP->pointsPP[0] ;
    dtmP->xMin = dtmP->xMax = pointP->x ;
    dtmP->yMin = dtmP->yMax = pointP->y ;
    dtmP->zMin = dtmP->zMax = pointP->z ;
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       pointP = pointAddrP(dtmP,point) ;
       if( pointP->x < dtmP->xMin ) dtmP->xMin = pointP->x ;
       if( pointP->x > dtmP->xMax ) dtmP->xMax = pointP->x ;
       if( pointP->y < dtmP->yMin ) dtmP->yMin = pointP->y ;
       if( pointP->y > dtmP->yMax ) dtmP->yMax = pointP->y ;
       if( pointP->z < dtmP->zMin ) dtmP->zMin = pointP->z ;
       if( pointP->z > dtmP->zMax ) dtmP->zMax = pointP->z ;
      }
/*
**  Set Ranges
*/
    dtmP->xRange = dtmP->xMax - dtmP->xMin ;
    dtmP->yRange = dtmP->yMax - dtmP->yMin ;
    dtmP->zRange = dtmP->zMax - dtmP->zMin ;
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
BENTLEYDTM_EXPORT int bcdtmMath_calculateMachinePrecisionForDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Calculates The Machine Precision For A Dtm Object
*/
{
 int ret=DTM_SUCCESS ;
 double  ls,lm,large ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Get Largest Absolute Coordinate
*/
 large = fabs(dtmP->xMax) ;
 if( fabs(dtmP->xMin) > large )  large = fabs(dtmP->xMin) ;
 if( fabs(dtmP->yMax) > large )  large = fabs(dtmP->yMax) ;
 if( fabs(dtmP->yMin) > large )  large = fabs(dtmP->yMin) ;
/*
** Iteratively get Midpoint
*/
 ls = lm = 0.0 ;
 lm = (lm+large) / 2.0 ;
 while ( lm != ls && lm != large  )
   {
    ls  = lm ;
    lm  = ( lm + large ) / 2.0 ;
   }
/*
** Set Machine Precision
*/
 dtmP->mppTol = large - ls ;
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
BENTLEYDTM_Public int bcdtmMath_pointSideOfDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2,long p3)
{
 int    ret=0,dbg=DTM_TRACE_VALUE(0) ;
 double sd1,sd2;
 DTM_TIN_POINT *point1P, *point2P, *point3P ;
/*
** Get Point Coordinates
*/
 point1P = pointAddrP(dtmP,p1) ;
 point2P = pointAddrP(dtmP,p2) ;
 point3P = pointAddrP(dtmP,p3) ;
/*
** Calculate Side Of Function
*/
 sd1 = ((point1P->x-point3P->x) * (point2P->y-point3P->y)) - ((point1P->y-point3P->y) * (point2P->x-point3P->x)) ;

 if (sd1 == 0.0)
    return 0;
 sd2 = ((point1P->x-point2P->x) * (point3P->y-point2P->y)) - ((point1P->y-point2P->y) * (point3P->x-point2P->x)) ;

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"p1 = %8ld ** %12.5lf %12.5lf %10.4lf",p1,point1P->x,point1P->y,point1P->z) ;
    bcdtmWrite_message(0,0,0,"p2 = %8ld ** %12.5lf %12.5lf %10.4lf",p2,point2P->x,point2P->y,point2P->z) ;
    bcdtmWrite_message(0,0,0,"p3 = %8ld ** %12.5lf %12.5lf %10.4lf",p3,point3P->x,point3P->y,point3P->z) ;
    bcdtmWrite_message(0,0,0,"sd1 = %16.15lf sd2 = %16.15lf",sd1,sd2) ;
    if( sd1 == 0.0 ) bcdtmWrite_message(0,0,0,"sd1 is zero") ;
    else             bcdtmWrite_message(0,0,0,"sd1 is not zero") ;
    if( sd2 == 0.0 ) bcdtmWrite_message(0,0,0,"sd2 is zero") ;
    else             bcdtmWrite_message(0,0,0,"sd2 is not zero") ;
   }
 if( ( sd1 > 0.0 && sd2 < 0.0 ) || ( sd1 < 0.0 && sd2 > 0.0 ))
   {
    if     ( sd1 <  0.0 ) ret = -1 ; /* Right of Line */
    else if( sd1 == 0.0 ) ret =  0 ; /* On Line       */
    else                  ret =  1 ; /* Left of Line  */
   }
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_linePointSideOfDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2,double x3,double y3)
{
 int    ret ;
 double sd1,sd2,sd3,x1,y1,x2,y2 ;
 DTM_TIN_POINT *pointP ;
 pointP = pointAddrP(dtmP,p1) ;
 x1 = pointP->x ;
 y1 = pointP->y ;
 pointP = pointAddrP(dtmP,p2) ;
 x2 = pointP->x ;
 y2 = pointP->y ;
 sd1 = ((x1-x3) * (y2-y3)) - ((y1-y3) * (x2-x3)) ;
 sd2 = ((x2-x1) * (y3-y1)) - ((y2-y1) * (x3-x1)) ;
 if( ( sd1 < 0.0 && sd2 >= 0.0 ) || ( sd1 > 0.0 && sd2 <= 0.0) ) return(0) ;
 sd3 = ((x3-x2) * (y1-y2)) - ((y3-y2) * (x1-x2)) ;
 if( ( sd1 < 0.0 && sd3 >= 0.0 ) || ( sd1 > 0.0 && sd3 <= 0.0) ) return(0) ;
 if     ( sd1 <  0.0 ) ret = -1 ; /* Right of Line */
 else if( sd1 == 0.0 ) ret =  0 ; /* On Line       */
 else                  ret =  1 ; /* Left of Line  */
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_pointDistanceDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 double x,y ;
 DTM_TIN_POINT *p1P,*p2P ;
 p1P = pointAddrP(dtmP,p1) ;
 p2P = pointAddrP(dtmP,p2) ;
 x = p2P->x - p1P->x ;
 y = p2P->y - p1P->y ;
 return(sqrt(x*x+y*y)) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_pointDistanceSquaredDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 double x,y ;
 DTM_TIN_POINT *p1P,*p2P ;
 p1P = pointAddrP(dtmP,p1) ;
 p2P = pointAddrP(dtmP,p2) ;
 x = p2P->x - p1P->x ;
 y = p2P->y - p1P->y ;
 return(x*x+y*y) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_pointDistance3DDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 double x,y,z ;
 DTM_TIN_POINT *p1P,*p2P ;
 p1P = pointAddrP(dtmP,p1) ;
 p2P = pointAddrP(dtmP,p2) ;
 x = p2P->x - p1P->x ;
 y = p2P->y - p1P->y ;
 z = p2P->z - p1P->z ;
 return(sqrt(x*x+y*y+z*z)) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_allPointSideOfDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2,long p3)
{
 int    ret=0 ;
 double sd1,sd2,sd3,x1,y1,x2,y2,x3,y3 ;
 DTM_TIN_POINT *pP ;
 pP = pointAddrP(dtmP,p1) ;
 x1  = pP->x ; y1 = pP->y ;
 pP = pointAddrP(dtmP,p2) ;
 x2  = pP->x ; y2 = pP->y ;
 pP = pointAddrP(dtmP,p3) ;
 x3  = pP->x ; y3 = pP->y ;
 sd1 = ((x1-x3) * (y2-y3)) - ((y1-y3) * (x2-x3))  ;
 sd2 = ((x2-x1) * (y3-y1)) - ((y2-y1) * (x3-x1))  ;
 sd3 = ((x3-x2) * (y1-y2)) - ((y3-y2) * (x1-x2))  ;
 if( ( sd1 < 0.0 && sd2 >= 0.0 ) || ( sd1 > 0.0 && sd2 <= 0.0) ) return(0) ;
 if( ( sd1 < 0.0 && sd3 >= 0.0 ) || ( sd1 > 0.0 && sd3 <= 0.0) ) return(0) ;
 if( sd1 <  0.0 ) ret = -1 ; /* Right of Line */
 if( sd1 == 0.0 ) ret =  0 ; /* On Line       */
 if( sd1 >  0.0 ) ret =  1 ; /* Left of Line  */
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_pointInTriangleDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,double Xp,double Yp)
/*
** This function tests if a point lies in a triangle
*/
{
 long    swap ;
 double  x1,y1,x2,y2,x3,y3 ;
 DTM_TIN_POINT *p1P,*p2P,*p3P ;
/*
** Check And Set Triangle Direction Anti Clockwise
*/
 if( bcdtmMath_pointSideOfDtmObject(dtmP,P1,P2,P3) < 0 )  { swap = P2 ; P2 = P3 ; P3 = swap ; }
/*
** Get Point Adresses
*/
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 p3P = pointAddrP(dtmP,P3) ;
/*
** Set Point Coordinates
*/
 x1 = p1P->x ; y1 = p1P->y  ;
 x2 = p2P->x ; y2 = p2P->y  ;
 x3 = p3P->x ; y3 = p3P->y  ;
/*
** Use Side Of Function
*/
 if( bcdtmMath_sideOf(x1,y1,x2,y2,Xp,Yp) >= 0 )
   if( bcdtmMath_sideOf(x2,y2,x3,y3,Xp,Yp) >= 0 )
     if( bcdtmMath_sideOf(x3,y3,x1,y1,Xp,Yp) >= 0 ) return(1) ;
/*
** Point Not In Triangle
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_pointInsideTriangleDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,double Xp,double Yp)
/*
** This function tests if a point lies in a triangle
*/
{
 long    swap ;
 double  x1,y1,x2,y2,x3,y3 ;
 DTM_TIN_POINT *p1P,*p2P,*p3P ;
/*
** Check And Set Triangle Direction Anti Clockwise
*/
 if( bcdtmMath_pointSideOfDtmObject(dtmP,P1,P2,P3) < 0 )  { swap = P2 ; P2 = P3 ; P3 = swap ; }
/*
** Get Point Adresses
*/
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 p3P = pointAddrP(dtmP,P3) ;
/*
** Set Point Coordinates
*/
 x1 = p1P->x ; y1 = p1P->y  ;
 x2 = p2P->x ; y2 = p2P->y  ;
 x3 = p3P->x ; y3 = p3P->y  ;
/*
** Use Side Of Function
*/
 if( bcdtmMath_sideOf(x1,y1,x2,y2,Xp,Yp) > 0 )
   if( bcdtmMath_sideOf(x2,y2,x3,y3,Xp,Yp) > 0 )
     if( bcdtmMath_sideOf(x3,y3,x1,y1,Xp,Yp) > 0 ) return(1) ;
/*
** Point Not In Triangle
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_normalDistanceToLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2, double x, double y )
/*
** This Function Return the Normal Distance Of a Point To A Line
*/
{
 double r,d,dx,dy,a1,a2,a3 ;
/*
** Initialise Variables
*/
 dx = pointAddrP(dtmP,P1)->x - pointAddrP(dtmP,P2)->x ;
 dy = pointAddrP(dtmP,P1)->y - pointAddrP(dtmP,P2)->y ;
 r  = sqrt( dx*dx + dy*dy) ;
 if( r > 0.0 )
   {
    a1 =  dy / r ;
    a2 = -dx / r ;
    a3 = -a1 * pointAddrP(dtmP,P1)->x - a2 * pointAddrP(dtmP,P1)->y ;
    d  = a1 * x + a2 * y + a3 ;
   }
 else
   {
    bcdtmWrite_message(2,0,0,"Error In Normal Distance To Line") ;
    d = sqrt( (x-pointAddrP(dtmP,P1)->x)*(x-pointAddrP(dtmP,P1)->x) + (y-pointAddrP(dtmP,P1)->y)*(y-pointAddrP(dtmP,P2)->y) ) ;
   }
 if( d < 0.0 ) d = -d ;
/*
** Job Completed
*/
 return(d) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT double bcdtmMath_getPointAngleDtmObject(BC_DTM_OBJ *dtmP,long p1,long p2)
{
 double ang,dx,dy ;
 DTM_TIN_POINT *p1P,*p2P ;
 p1P = pointAddrP(dtmP,p1) ;
 p2P = pointAddrP(dtmP,p2) ;
 dx = p2P->x - p1P->x ;
 dy = p2P->y - p1P->y ;
 ang = atan2(dy,dx) ;
 if( ang < 0.0 ) ang += DTM_2PYE ;
 return( ang ) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,double *areaP,DTMDirection *directionP)
/*
** This Function Calculates The Area Of An Hptr Polygon
**
** directionP = 1 Clockwise
**            = 2 Anti Clockwise
*/
{
 long   sp,np ;
 double x,y ;
 DTM_TIN_POINT *spP,*npP ;
/*
** Determine Hptr Polygon Area
*/
 sp  = startPnt   ;
 spP = pointAddrP(dtmP,sp) ;
 *areaP = 0.0    ;
 *directionP = DTMDirection::Clockwise ;
 do
   {
    np  = nodeAddrP(dtmP,sp)->hPtr ;
    npP = pointAddrP(dtmP,np) ;
    x  = npP->x - spP->x ;
    y  = npP->y - spP->y ;
    *areaP = *areaP + x * y / 2.0 + x * spP->y ;
    spP = npP ;
    sp  = np  ;
   }  while ( sp != startPnt ) ;
/*
** Set Polygon directionP
*/
 if( *areaP < 0.0 ) { *directionP = DTMDirection::AntiClockwise ; *areaP = -*areaP ; }
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
BENTLEYDTM_Public int bcdtmMath_calculateAreaAndDirectionSptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,double *areaP,DTMDirection *directionP)
/*
** This Function Calculates The Area Of An Hptr Polygon
**
** directionP = 1 Clockwise
**            = 2 Anti Clockwise
*/
{
 long   sp,np ;
 double x,y ;
 DTM_TIN_POINT *spP,*npP ;
/*
** Determine Hptr Polygon Area
*/
 sp  = startPnt   ;
 spP = pointAddrP(dtmP,sp) ;
 *areaP = 0.0    ;
 *directionP = DTMDirection::Clockwise ;
 do
   {
    np  = nodeAddrP(dtmP,sp)->sPtr ;
    npP = pointAddrP(dtmP,np) ;
    x  = npP->x - spP->x ;
    y  = npP->y - spP->y ;
    *areaP = *areaP + x * y / 2.0 + x * spP->y ;
    spP = npP ;
    sp  = np  ;
   }  while ( sp != startPnt ) ;
/*
** Set Polygon directionP
*/
 if( *areaP < 0.0 ) { *directionP = DTMDirection::AntiClockwise ; *areaP = -*areaP ; }
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
BENTLEYDTM_Public int bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,double *areaP,DTMDirection *directionP)
/*
** This Function Calculates The Area Of An Hptr Polygon
**
** directionP = 1 Clockwise
**            = 2 Anti Clockwise
*/
{
 long   sp,np ;
 double x,y ;
 DTM_TIN_POINT *spP,*npP ;
/*
** Determine Tptr Polygon Area
*/
 sp  = startPnt   ;
 spP = pointAddrP(dtmP,sp) ;
 *areaP = 0.0    ;
 *directionP = DTMDirection::Clockwise ;
 do
   {
    np  = nodeAddrP(dtmP,sp)->tPtr ;
    npP = pointAddrP(dtmP,np) ;
    x  = npP->x - spP->x ;
    y  = npP->y - spP->y ;
    *areaP = *areaP + x * y / 2.0 + x * spP->y ;
    spP = npP ;
    sp  = np  ;
   }  while ( sp != startPnt ) ;
/*
** Set Polygon directionP
*/
 if( *areaP < 0.0 ) { *directionP = DTMDirection::AntiClockwise ; *areaP = -*areaP ; }
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
BENTLEYDTM_Public int bcdtmMath_calculateDirectionAreaAndPerimeterTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,DTMDirection *directionP,double *areaP,double *perimeterP)
/*
** This Function Calculates The areaP And perimeterP Of A Tptr Polygon
**
*/
{
 long   sp,np ;
 double x,y ;
/*
** Initialise
*/
 *directionP = DTMDirection::Clockwise ;
 *areaP = *perimeterP = 0.0 ;
/*
** Scan Tptr Polygon
*/
 sp = startPnt ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    x  = pointAddrP(dtmP,np)->x - pointAddrP(dtmP,sp)->x ;
    y  = pointAddrP(dtmP,np)->y - pointAddrP(dtmP,sp)->y ;
    *areaP = *areaP + x * y / 2.0 + x * pointAddrP(dtmP,sp)->y ;
    *perimeterP = *perimeterP + sqrt(x*x+y*y) ;
    sp = np ;
   }  while ( sp != startPnt ) ;
/*
** Set Polygon direction
*/
 if( *areaP < 0.0 )  { *directionP = DTMDirection::AntiClockwise ; *areaP = -*areaP ; }
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
BENTLEYDTM_Public int bcdtmMath_calculateDirectionAreaAndPerimeterHptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,DTMDirection *directionP,double *areaP,double *perimeterP)
/*
** This Function Calculates The areaP And perimeterP Of A Hptr Polygon
**
*/
{
 long   sp,np ;
 double x,y ;
/*
** Initialise
*/
 *directionP = DTMDirection::Clockwise ;
 *areaP = *perimeterP = 0.0 ;
/*
** Scan Tptr Polygon
*/
 sp = startPnt ;
 do
   {
    np = nodeAddrP(dtmP,sp)->hPtr ;
    x  = pointAddrP(dtmP,np)->x - pointAddrP(dtmP,sp)->x ;
    y  = pointAddrP(dtmP,np)->y - pointAddrP(dtmP,sp)->y ;
    *areaP = *areaP + x * y / 2.0 + x * pointAddrP(dtmP,sp)->y ;
    *perimeterP = *perimeterP + sqrt(x*x+y*y) ;
    sp = np ;
   }  while ( sp != startPnt ) ;
/*
** Set Polygon direction
*/
 if( *areaP < 0.0 )  { *directionP = DTMDirection::AntiClockwise; *areaP = -*areaP ; }
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
BENTLEYDTM_EXPORT int bcdtmMath_interpolatePointOnLineDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double *z,long P1,long P2 )
/*
** This Function Interpolates the Point x,y on Line P1,P2
*/
{
 double dx,dy,dz ;
 DTM_TIN_POINT *p1P,*p2P   ;
/*
** Initialise Varibles
*/
 *z = 0.0 ;
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 dx = p2P->x - p1P->x ;
 dy = p2P->y - p1P->y ;
 dz = p2P->z - p1P->z ;
/*
** Interpolate Point
*/
 if( fabs(dx) >= fabs(dy) ) *z = p1P->z  + (( x - p1P->x ) / dx ) * dz ;
 else                       *z = p1P->z  + (( y - p1P->y ) / dy ) * dz ;
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
BENTLEYDTM_Public int bcdtmMath_interpolatePointOnTriangleDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double *ZP,long P1,long P2,long P3)
{
 long p ;
 double trgX[3],trgY[3],trgZ[3] ;
 DTM_TIN_POINT *p1P,*p2P,*p3P   ;
/*
** Initialise Variables
*/
 while ( P1 > P2 || P1 > P3 ) { p = P1 ; P1 = P2 ; P2 = P3 ; P3 = p ; }
/*
** Needed For Legacy Reasons
*/
 if( P2 == P3 ) { *ZP = pointAddrP(dtmP,P1)->z ; return(DTM_SUCCESS) ; }
/*
** Set Triangle Coordinates
*/
 p1P = pointAddrP(dtmP,P1) ;
 p2P = pointAddrP(dtmP,P2) ;
 p3P = pointAddrP(dtmP,P3) ;
 trgX[0] = p1P->x ;
 trgX[1] = p2P->x ;
 trgX[2] = p3P->x ;
 trgY[0] = p1P->y ;
 trgY[1] = p2P->y ;
 trgY[2] = p3P->y ;
 trgZ[0] = p1P->z ;
 trgZ[1] = p2P->z ;
 trgZ[2] = p3P->z ;
/*
** Interpolate Point
*/
 bcdtmMath_interpolatePointOnTriangle(x,y,ZP,trgX,trgY,trgZ) ;
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
BENTLEYDTM_Public int bcdtmMath_interpolatePointOnTrianglePlane(long newTriangle,double x,double y,double *z,double xt[3],double yt[3],double zt[3] )
/*
** This routine finds the z value for a point on the triangle plane
*/
{
static thread_local bool   flatTriangle = false;
static thread_local double ca = 0.0, cb = 0.0, cc = 0.0, cd = 0.0;
static thread_local double xmin = 0.0, ymin = 0.0, zmin = 0.0, flatZ = 0.0;
/*
** Normalise Triangle
*/
 if( newTriangle )
   {
    xmin = xt[0] ;
    ymin = yt[0] ;
    zmin = zt[0] ;
    if( xt[1] < xmin ) xmin = xt[1] ;
    if( xt[2] < xmin ) xmin = xt[2] ;
    if( yt[1] < ymin ) ymin = yt[1] ;
    if( yt[2] < ymin ) ymin = yt[2] ;
    if( zt[1] < zmin ) zmin = zt[1] ;
    if( zt[2] < zmin ) zmin = zt[2] ;
    if( zt[0] == zt[1] && zt[0] == zt[2] ) { flatTriangle = true ; flatZ = zt[0] ; }
    else                                     flatTriangle = false ;
   }
/*
** Calculate Coefficients of Plane
*/
 if( newTriangle )
   {
    bcdtmMath_calculatePlaneCoefficients(xt[0]-xmin,yt[0]-ymin,zt[0]-zmin,xt[1]-xmin,yt[1]-ymin,zt[1]-zmin,xt[2]-xmin,yt[2]-ymin,zt[2]-zmin,&ca,&cb,&cc,&cd ) ;
   }
/*
** Calculate z value
*/
 if( flatTriangle ) *z = flatZ ;
 else               *z = - ( ca * (x-xmin) + cb * (y-ymin) + cd ) / cc  +  zmin ;
/*
/*
** Job Completed
*/
 return(0) ;
}/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_getPointOffsetPolygonDirectionAndAreaDtmObject (BC_DTM_OBJ *dtmP, long *polyPtsP, long numPts, DTMDirection *directionP, double *areaP)
/*
**  This Function Determines The Direction And Area
**  Of A Dtm Object Point Offset Polygon
*/
{
 double x,y   ;
 long   *pointNumP ;
/*
** Initialise Varaibles
*/
 *areaP = 0.0 ;
/*
** Sum Area Of Polygon Points
*/
 for ( pointNumP = polyPtsP + 1 ; pointNumP < polyPtsP + numPts ; ++pointNumP )
   {
    x = pointAddrP(dtmP,*pointNumP)->x - pointAddrP(dtmP,*(pointNumP-1))->x ;
    y = pointAddrP(dtmP,*pointNumP)->y - pointAddrP(dtmP,*(pointNumP-1))->y ;
    *areaP = *areaP + x * y / 2.0 + x * pointAddrP(dtmP,*(pointNumP-1))->y ;
   }
/*
** Set directionP
*/
 if (*areaP >= 0.0) *directionP = DTMDirection::Clockwise;          /* ClockWise      */
 else { *directionP = DTMDirection::AntiClockwise; *areaP = -*areaP; } /* Anti Clockwise */
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
BENTLEYDTM_Public int bcdtmMath_reversePointOffsetPolygonDirection(long *polyPtsP,long numPts)
/*
**  This Function Reverses The Direction Of a Point Offset Polygon
*/
{
 long *pnt1P,*pnt2P,*pnt3P,tempPnt ;
/*
** Swap Coordinates
*/
 pnt1P = polyPtsP ;
 pnt2P = polyPtsP + numPts - 1 ;
 pnt3P = &tempPnt ;
 while ( pnt1P < pnt2P )
   {
    *pnt3P = *pnt1P ;
    *pnt1P = *pnt2P ;
    *pnt2P = *pnt3P ;
    ++pnt1P ; --pnt2P ;
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
BENTLEYDTM_Public int bcdtmMath_calculatePartialDerivativesDtmObject(BC_DTM_OBJ *dtmP,double **partialDerivPP )
/*
** This Routine Calculates the Partial Derivatives At The Triange Vertices
*/
{
 int     ret=DTM_SUCCESS ;
 double  x,y,z,zx,zy ;
 double  nmx,nmy,nmz,nmxx,nmxy,nmyx,nmyy ;
 double  dx1,dy1,dz1,dx2,dy2,dz2,dzx1,dzy1,dzx2,dzy2 ;
 double  dnmy,dnmx,dnmz ;
 double  dnmxx,dnmxy,dnmyx,dnmyy ;
 long    p,cl,cp,feature ;
 DTM_TIN_POINT *pntP ;
/*
** Allocate Storage for Partial Derivatives
*/
 *partialDerivPP = ( double * ) malloc( dtmP->numPoints * 5 * sizeof(double) ) ;
 if( *partialDerivPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Estimate zx and zy
*/
 for( p = 0 ; p < dtmP->numPoints ; ++ p )
   {
    pntP = pointAddrP(dtmP,p) ;
    x = pntP->x ;
    y = pntP->y ;
    z = pntP->z ;
    nmx = nmy = nmz = 0.0 ;
    cl = nodeAddrP(dtmP,p)->cPtr ;
    while ( clistAddrP(dtmP,cl)->nextPtr != dtmP->nullPtr )
      {
       pntP = pointAddrP(dtmP,clistAddrP(dtmP,cl)->pntNum) ;
       dx1  = pntP->x - x ;
       dy1  = pntP->y - y ;
       dz1  = pntP->z - z ;
       cp   = clistAddrP(dtmP,cl)->nextPtr    ;
       while ( cp != dtmP->nullPtr )
         {
          pntP = pointAddrP(dtmP,clistAddrP(dtmP,cp)->pntNum) ;
          dx2  = pntP->x - x ;
          dy2  = pntP->y - y ;
          dnmz = dx1*dy2 - dy1*dx2 ;
          if( dnmz != 0.0 )
            {
             dz2  = pntP->z - z ;
             dnmx = dy1*dz2 - dz1*dy2 ;
             dnmy = dz1*dx2 - dx1*dz2 ;
             if( dnmz < 0.0 ) { dnmx = - dnmx ; dnmy = - dnmy ; dnmz = - dnmz ; }
             nmx = nmx + dnmx ; nmy = nmy + dnmy ; nmz = nmz + dnmz ;
            }
          cp = clistAddrP(dtmP,cp)->nextPtr ;
         }
       cl = clistAddrP(dtmP,cl)->nextPtr ;
      }
    if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,p,&feature) ) goto errexit ;
    if( feature == dtmP->nullPnt )
      {
       *(*partialDerivPP + p*5 )     = -nmx/nmz ;
       *(*partialDerivPP + p*5 + 1 ) = -nmy/nmz ;
      }
    else
      {
       *(*partialDerivPP + p*5)     = 0.0 ;
       *(*partialDerivPP + p*5 + 1) = 0.0 ;
      }
   }
/*
** Estimate ZXX ZXY ZYY
*/
 for( p = 0 ; p < dtmP->numPoints ; ++p )
   {
    pntP = pointAddrP(dtmP,p) ;
    x = pntP->x ;
    y = pntP->y ;
    zx = *(*partialDerivPP + p*5 ) ;
    zy = *(*partialDerivPP + p*5 + 1 ) ;
    nmxx = nmxy = nmyx = nmyy = nmz = 0.0 ;
    cl = nodeAddrP(dtmP,p)->cPtr ;
    while( clistAddrP(dtmP,cl)->nextPtr != dtmP->nullPtr )
      {
       pntP = pointAddrP(dtmP,clistAddrP(dtmP,cl)->pntNum) ;
       dx1  = pntP->x - x ;
       dy1  = pntP->y - y ;
       dzx1 = *(*partialDerivPP+clistAddrP(dtmP,cl)->pntNum * 5 )     - zx ;
       dzy1 = *(*partialDerivPP+clistAddrP(dtmP,cl)->pntNum * 5 + 1 ) - zy ;
       cp   = clistAddrP(dtmP,cl)->nextPtr    ;
       while ( cp != dtmP->nullPtr )
         {
          pntP = pointAddrP(dtmP,clistAddrP(dtmP,cp)->pntNum) ;
          dx2  = pntP->x - x ;
          dy2  = pntP->y - y ;
          dnmz = dx1*dy2 - dy1*dx2 ;
          if( dnmz != 0.0 )
            {
             dzx2 = *( *partialDerivPP +clistAddrP(dtmP,cp)->pntNum * 5 )     - zx ;
             dzy2 = *( *partialDerivPP +clistAddrP(dtmP,cp)->pntNum * 5 + 1 ) - zy ;
             dnmxx = dy1 * dzx2 - dzx1 * dy2 ;
             dnmxy = dzx1 * dx2 - dx1 * dzx2 ;
             dnmyx = dy1 * dzy2 - dzy1 * dy2 ;
             dnmyy = dzy1 * dx2 - dx1 * dzy2 ;
             if( dnmz < 0.0 )
               {
                dnmxx = - dnmxx ; dnmxy = - dnmxy ;
                dnmyx = - dnmyx ; dnmyy = - dnmyy ;
                dnmz  = - dnmz  ;
               }
             nmxx = nmxx + dnmxx ; nmxy = nmxy + dnmxy ;
             nmyx = nmyx + dnmyx ; nmyy = nmyy + dnmyy ;
             nmz = nmz + dnmz ;
            }
          cp = clistAddrP(dtmP,cp)->nextPtr ;
         }
       cl = clistAddrP(dtmP,cl)->nextPtr ;
      }
    if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,p,&feature) ) goto errexit ;
    if( feature == dtmP->nullPnt )
      {
       *(*partialDerivPP + p * 5 + 2 ) = - ( nmxx / nmz ) ;
       *(*partialDerivPP + p * 5 + 3 ) = - ((nmxy + nmyx) / (2.0 * nmz)) ;
       *(*partialDerivPP + p * 5 + 4 ) = - ( nmyy / nmz ) ;
      }
    else
      {
       *(*partialDerivPP + p * 5 + 2 ) = 0.0 ;
       *(*partialDerivPP + p * 5 + 3 ) = 0.0 ;
       *(*partialDerivPP + p * 5 + 4 ) = 0.0 ;
      }
   }
/*
** Cleanup
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
BENTLEYDTM_Public int bcdtmMath_interpolatePointOnPolynomial(long newTriangle,double xp,double yp,double *zp,double x[],double y[],double z[],double pd[] )
/*
**  This routine interpolates the z value for a point
**  within a triangle from a bivarate polynominal
*/
{
 long   i,jpd ;
 double a,b,c,d,dlt,dx,dy,u,v ;
 static thread_local double x0, y0, ap, bp, cp, dp;
 double act2,adbc,bdt2,lu,lv,thxu,thuv ;
 double aa,bb,cc,dd,ab,cd,ac,ad,bc,g1,g2,h1,h2,h3  ;
 double zu[3],zuu[3],zv[3],zvv[3],zuv[3] ;
 double p0,p1,p2,p3,p4,p5 ;
 static thread_local double p00,p01,p02,p03,p04,p05 ;
 static thread_local double p10,p11,p12,p13,p14 ;
 static thread_local double p20,p21,p22,p23 ;
 static thread_local double p30,p31,p32 ;
 static thread_local double p40,p41 ;
 static thread_local double p50 ;
 double csuv,thus,thsv ;
/*
** Determine the Coefficients for the Coordinate System
** Transformation from the x-y system to the U-V system
*/
  if( newTriangle )
    {
     x0 = x[0] ;
     y0 = y[0] ;
     a = x[1] - x0 ;
     b = x[2] - x0 ;
     c = y[1] - y0 ;
     d = y[2] - y0 ;
     ad = a * d ;
     bc = b * c ;
     dlt = ad - bc ;
     ap =  d / dlt ;
     bp = -b / dlt ;
     cp = -c / dlt ;
     dp =  a / dlt ;
/*
**  Convert the Partial Derivatives at the vertices of the triangle
**  for the U-V coordinate system
*/
     aa = a*a ;
     act2 = 2.0*a*c ;
     cc = c*c ;
     ab = a*b ;
     adbc = ad+bc ;
     cd = c*d ;
     bb = b*b ;
     bdt2 = 2.0*b*d ;
     dd = d*d ;
     for( i = 0 ; i < 3 ; ++i )
       {
        jpd = 5 * (i+1) - 1 ;
        zu[i]  = a*pd[jpd-4]  + c*pd[jpd-3] ;
        zv[i]  = b*pd[jpd-4]  + d*pd[jpd-3] ;
        zuu[i] = aa*pd[jpd-2] + act2*pd[jpd-1] + cc*pd[jpd] ;
        zuv[i] = ab*pd[jpd-2] + adbc*pd[jpd-1] + cd*pd[jpd] ;
        zvv[i] = bb*pd[jpd-2] + bdt2*pd[jpd-1] + dd*pd[jpd] ;
       }
/*
** Calculate the coefficients of the polynominal
*/
     p00 = z[0] ;
     p10 = zu[0] ;
     p01 = zv[0] ;
     p20 = 0.5*zuu[0] ;
     p11 = zuv[0] ;
     p02 = 0.5*zvv[0] ;
     h1 = z[1] - p00 - p10 - p20 ;
     h2 = zu[1] - p10 - zuu[0] ;
     h3 = zuu[1] - zuu[0] ;
     p30 =  10.0*h1 - 4.0*h2 + 0.5*h3 ;
     p40 = -15.0*h1 + 7.0*h2 -     h3 ;
     p50 =   6.0*h1 - 3.0*h2 + 0.5*h3 ;
     h1 = z[2] - p00 - p01 - p02 ;
     h2 = zv[2] - p01 - zvv[0]   ;
     h3 = zvv[2] - zvv[0] ;
     p03 =  10.0*h1 - 4.0*h2 + 0.5*h3 ;
     p04 = -15.0*h1 + 7.0*h2 -     h3 ;
     p05 =   6.0*h1 - 3.0*h2 + 0.5*h3 ;
     lu=sqrt(aa+cc) ;
     lv=sqrt(bb+dd) ;
     thxu=atan2(c,a) ;
     thuv=atan2(d,b) - thxu ;
     csuv = cos(thuv) ;
     p41 = 5.0*lv*csuv/lu*p50 ;
     p14 = 5.0*lu*csuv/lv*p05 ;
     h1 = zv[1] - p01 - p11 - p41 ;
     h2 = zuv[1] - p11 - 4.0*p41  ;
     p21 =  3.0*h1-h2 ;
     p31 = -2.0*h1+h2 ;
     h1 = zu[2] - p10 - p11 - p14 ;
     h2 = zuv[2] - p11 - 4.0*p14  ;
     p12 =  3.0*h1-h2 ;
     p13 = -2.0*h1+h2 ;
     thus = atan2(d-c,b-a) - thxu ;
     thsv = thuv - thus ;
     aa =  sin(thsv) / lu ;
     bb = -cos(thsv) / lu ;
     cc =  sin(thus) / lv ;
     dd =  cos(thus) / lv ;
     ac = aa * cc ;
     ad = aa * dd ;
     bc = bb * cc ;
     g1 = aa * ac * ( 3.0*bc + 2.0*ad) ;
     g2 = cc * ac * ( 3.0*ad + 2.0*bc) ;
     h1 = -aa*aa*aa*(5.0*aa*bb*p50+(4.0*bc+ad)*p41) -
       cc*cc*cc*(5.0*cc*dd*p05+(4.0*ad+bc)*p14) ;
     h2 = 0.5 * zvv[1] - p02 - p12 ;
     h3 = 0.5 * zuu[2] - p20 - p21 ;
     p22 = (g1*h2 + g2*h3 - h1) / ( g1 + g2 ) ;
     p32 = h2 - p22 ;
     p23 = h3 - p22 ;
    }
/*
** Convert x & y to U-V system
*/
  dx = xp - x0 ;
  dy = yp - y0 ;
  u = ap * dx + bp * dy ;
  v = cp * dx + dp * dy ;
/*
** Evaluate the Polynominal
*/
  p0 = p00+v*(p01+v*(p02+v*(p03+v*(p04+v*p05)))) ;
  p1 = p10+v*(p11+v*(p12+v*(p13+v*p14)))  ;
  p2 = p20+v*(p21+v*(p22+v*p23)) ;
  p3 = p30+v*(p31+v*p32) ;
  p4 = p40+v*p41 ;
  p5 = p50 ;
  *zp = p0+u*(p1+u*(p2+u*(p3+u*(p4+u*p5)))) ;
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
BENTLEYDTM_Public double bcdtmMath_calculateIncludedPointAngleDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,long pnt3)
/*
** This Function Calculates The Included Angle Between angle(pnt1-Pnt2) and angle(pnt1-pnt3)
** assuming a clockwise sense for pnt1,pnt2,pnt3
**
*/
{
 double ang1,ang2 ;
 ang1 = bcdtmMath_getPointAngleDtmObject(dtmP,pnt1,pnt2) ;
 ang2 = bcdtmMath_getPointAngleDtmObject(dtmP,pnt1,pnt3) ;
 if( ang1 < ang2 ) ang1 += DTM_2PYE ;
 return(ang1-ang2) ;
 }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_getTriangleAttributesDtmObject(BC_DTM_OBJ *dtmP,long trgPnt1,long trgPnt2,long trgPnt3,double *slopeDegreesP,double *slopePercentP,double *aspectP,double *heightP)
/*
** This Function Gets Triangle Attributes
*/
{
 double         X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3 ;
 double         c,ca=0.0,cb=0.0,cc=0.0,cd=0.0,axy=0.0 ;
 DTM_TIN_POINT  *pntP ;
 static thread_local long    lp, lp1 = 0, lp2 = 0, lp3 = 0;
 static thread_local double  slopeDeg = 0.0, slopePer = 0.0, aspect = 0.0, height = 0.0;
 static thread_local BC_DTM_OBJ  *lastDtmP = NULL;

/*
** Initialise
*/
 while ( trgPnt1 > trgPnt2 || trgPnt1 > trgPnt3 ) {  lp = trgPnt1 ; trgPnt1 = trgPnt2 ; trgPnt2 = trgPnt3 ; trgPnt3 = lp ; }
 if( lastDtmP == dtmP && lp1 == trgPnt1 && lp2 == trgPnt2 && lp3 == trgPnt3 )
      { *slopeDegreesP = slopeDeg ; *slopePercentP = slopePer ; *aspectP = aspect ; *heightP = height ; return(0) ; }
 else { lastDtmP = dtmP ; lp1 = trgPnt1 ; lp2 = trgPnt2 ; lp3 = trgPnt3 ; }
/*
** Get Triangle Attributes
*/
 pntP = pointAddrP(dtmP,trgPnt1) ;
 X1 = pntP->x ; Y1 = pntP->y ; Z1 = pntP->z ;
 pntP = pointAddrP(dtmP,trgPnt2) ;
 X2 = pntP->x ; Y2 = pntP->y ; Z2 = pntP->z ;
 pntP = pointAddrP(dtmP,trgPnt3) ;
 X3 = pntP->x ; Y3 = pntP->y ; Z3 = pntP->z ;
 if( Z1 == Z2 && Z2 == Z3 )
   {
    slopeDeg = slopePer = 0.0 ; aspect = 360.0 ; height = Z1 ;
    *slopeDegreesP = *slopePercentP = 0.0 ;
    *aspectP = 360.0 ;
    *heightP = Z1 ;
    return(0) ;
   }
/*
** Calculate Plane Coefficients
*/
 bcdtmMath_calculatePlaneCoefficients(X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,&ca,&cb,&cc,&cd) ;
/*
** Get Triangle heightP
*/
 height = (Z1 + Z2 + Z3) / 3.0 ;
/*
** Calulate Triangle Slope
*/
/*
 slopeDeg = acos(fabs( cc / sqrt( ca*ca + cb*cb + cc*cc ))) ;
 Modified 15/5/2002 To Overcome Problems With Low Precision Setting
 on Floating Point Unit
*/
 c = sqrt( ca*ca + cb*cb + cc*cc ) ;
 if( c == 0.0 ) c = 0.0000000001 ;
 slopeDeg = acos(fabs(cc/c)) ;
 slopePer = tan(slopeDeg)   * 100.0 ;
 slopeDeg = slopeDeg / DTM_2PYE * 360.0 ;
/*
** Get Angle Of Normal Projected On XY Plane
*/
 axy = atan2(cb,ca) ;
 if( axy < 0.0 ) axy += DTM_2PYE ;
/*
** Get Aspect Value
*/
 if( cc  >=  0.0 ) aspect = axy ;
 else              aspect = axy + DTM_2PYE /2.0 ;
 if( aspect > DTM_2PYE ) aspect = aspect - DTM_2PYE ;
/*
** Convert Aspect To Bearing Radians Anti Clockwise From North Axis
*/
 aspect = ( DTM_PYE / 2.0 - aspect ) ;
 if( aspect < 0.0 ) aspect = DTM_2PYE + aspect ;
/*
** Convert Aspect Radians To Degrees
*/
 aspect = aspect / DTM_2PYE * 360.0 ;
/*
** Set Attributes
*/
 *slopeDegreesP = slopeDeg ;
 *slopePercentP = slopePer ;
 *aspectP       = aspect ;
 *heightP       = height ;
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
BENTLEYDTM_Public int bcdtmMath_calculateNormalVectorsForTriangleVerticesDtmObject
(
 BC_DTM_OBJ *dtmP,                /* ==> Pointer To Dtm Object      */
 long   vectorOption,             /* ==> VectorOption <1=Surface Derivatives,2=Averaged Triangle Surface Normals> */
 DPoint3d    **normalVectorsPP,        /* <== Pointer To Normal Vectors  */
 long   *numNormalVectorsP        /* <== Number Of Normal Vectors   */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long p1,p2,p3,clPtr,numVectors ;
 long p1p2Brk,p1p3Brk,p2p3Brk ;
 double *partialDerivP=NULL ;
 DPoint3d  trgVector,avgVector ;
 DTM_TIN_NODE *nodeP ;
 DTM_CIR_LIST *clistP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Normal Vectors For Triangle Vertices") ;
/*
** Initialise
*/
 *numNormalVectorsP = 0 ;
 if( *normalVectorsPP != NULL ) { free(*normalVectorsPP) ; *normalVectorsPP = NULL ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Allocate Memory For Normal Vectors
*/
 *normalVectorsPP = ( DPoint3d * ) malloc (dtmP->numPoints * sizeof(DPoint3d)) ;
 if( *normalVectorsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Calculate Normals Vectors
*/
 switch( vectorOption )
   {
    case 1 :    /* From Surface Partial Derivatives */
/*
**    Calculate Partial Derivatives
*/
      if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Partial Derivatives For Triangle Vertices") ;
      if( bcdtmMath_calculatePartialDerivativesDtmObject(dtmP,&partialDerivP)) goto errexit ;
/*
**    Create Normal Vectors From First Partial Derivatives Of x and y
*/
      for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
        {
         avgVector.x = *(partialDerivP+p1*5) ;
         avgVector.y = *(partialDerivP+p1*5+1) ;
         avgVector.z = 1.0  ;
//         avgVector.z = *(partialDerivP+p1*5+3) ;
         *(*normalVectorsPP+*numNormalVectorsP) = avgVector ;
         ++*numNormalVectorsP ;
        }
     break ;

     case 2 : /* From Averaged Triangle Surface Normals   */
/*
**     Scan Tin
*/
       for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
         {
          numVectors  = 0 ;
          avgVector.x = 0.0 ;
          avgVector.y = 0.0 ;
          avgVector.z = 0.0 ;
          nodeP = nodeAddrP(dtmP,p1) ;
          if( ( clPtr = nodeP->cPtr ) != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistP->pntNum)) < 0 ) goto errexit ;
             while( clPtr != dtmP->nullPtr )
               {
                p3 = clistP->pntNum ;
                clPtr = clistP->nextPtr ;
                if( nodeP->hPtr != p2 )
                  {
/*
**                 Check For Break lines
*/
                   p1p2Brk = bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2) ;
                   p1p3Brk = bcdtmList_testForBreakLineDtmObject(dtmP,p1,p3) ;
                   p2p3Brk = bcdtmList_testForBreakLineDtmObject(dtmP,p2,p3) ;
/*
**                 Calculate Normal For Triangle
*/
                   bcdtmMath_calculateNormalVectorToPlaneDtmObject(dtmP,p1,p3,p2,&trgVector) ;
                   avgVector.x = avgVector.x + trgVector.x ;
                   avgVector.y = avgVector.y + trgVector.y ;
                   avgVector.z = avgVector.z + trgVector.z ;
                   ++numVectors ;
                  }
                p2 = p3 ;
                clistP = clistAddrP(dtmP,clPtr);
               }
/*
**           Average Vectors
*/
             avgVector.x = avgVector.x / (double) numVectors ;
             avgVector.y = avgVector.y / (double) numVectors ;
             avgVector.z = avgVector.z / (double) numVectors ;
            }
/*
**        Store Vector
*/
          *(*normalVectorsPP+*numNormalVectorsP) = avgVector ;
          ++*numNormalVectorsP ;
         }
     break ;
   }
/*
** Clean Up
*/
 cleanup :
 if( partialDerivP != NULL ) { free(partialDerivP) ; partialDerivP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Normal Vectors For Triangle Vertices Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Normal Vectors For Triangle Vertices Error") ;
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
BENTLEYDTM_Public int bcdtmMath_calculateNormalVectorForTriangleVertexDtmObject
(
 BC_DTM_OBJ *dtmP,                /* ==> Pointer To Dtm Object      */
 long   point,                    /* ==> Tin Point To Calculate Normal Vector For */
 long   vectorOption,             /* ==> VectorOption <1=Surface Derivatives,2=Averaged Triangle Surface Normals> */
 DPoint3d    *normalVectorP            /* <== Pointer To Normal Vector  */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long p2,p3,clPtr ;
 DPoint3d  avgVector,trgVector ;
 DTM_CIR_LIST *clistP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Normal Vector For Triangle Vertex") ;
/*
** Initialise
*/
 normalVectorP->x = 1.0 ;
 normalVectorP->y = 1.0 ;
 normalVectorP->z = 1.0 ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
**  Check For Point Range Error
*/
 if( point < 0 || point >= dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
**  Scan Triangles At Vertex
*/
  clPtr = nodeAddrP(dtmP,point)->cPtr ;
  if( clPtr != dtmP->nullPtr )
    {
     avgVector.x = 0.0 ;
     avgVector.y = 0.0 ;
     avgVector.z = 0.0 ;
     if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
     while( clPtr != dtmP->nullPtr )
       {
        clistP = clistAddrP(dtmP,clPtr) ;
        p3 = clistP->pntNum ;
        clPtr = clistP->nextPtr ;
        if( nodeAddrP(dtmP,p3)->hPtr != point )
          {
/*
**         Calculate Normal For Triangle Plane
*/
           bcdtmMath_calculateNormalVectorToPlaneDtmObject(dtmP,point,p3,p2,&trgVector) ;
           avgVector.x = avgVector.x + trgVector.x ;
           avgVector.y = avgVector.y + trgVector.y ;
           avgVector.z = avgVector.z + trgVector.z ;
          }
        p2 = p3 ;
       }
/*
**   Set Return Vector
*/
     *normalVectorP = avgVector;
    }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Normal Vector For Triangle Vertex Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Normal Vector For Triangle Vertex Error") ;
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
BENTLEYDTM_EXPORT int bcdtmMath_calculateNormalVectorToPlaneDtmObject
(
 BC_DTM_OBJ *dtmP,                    /* Pointer To Dtm Object      */
 long  pnt1,                          /* Tin Point 1                */
 long  pnt2,                          /* Tin Point 1                */
 long  pnt3,                          /* Tin Point 3                */
 DPoint3d *normalVectorP                   /* Pointer To Normal Vector   */ 
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DPoint3d  vector1,vector2 ;
 DTM_TIN_POINT *p1P,*p2P,*p3P ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Normal Vector For Triangle Vertices") ;
/*
** Calculate point Address
*/
 p1P = pointAddrP(dtmP,pnt1) ;
 p2P = pointAddrP(dtmP,pnt2) ;
 p3P = pointAddrP(dtmP,pnt3) ;
/*
** Calculate Vectors
*/
 vector1.x = p2P->x - p1P->x ;
 vector1.y = p2P->y - p1P->y ;
 vector1.z = p2P->z - p1P->z ;
 vector2.x = p3P->x - p1P->x ;
 vector2.y = p3P->y - p1P->y ;
 vector2.z = p3P->z - p1P->z ;
/*
** Calculate Normal Vector - Cross Product vector1 * vector2
*/
 normalVectorP->x = vector1.y * vector2.z - vector1.z * vector2.y ;
 normalVectorP->y = vector1.z * vector2.x - vector1.x * vector2.z ;
 normalVectorP->z = vector1.x * vector2.y - vector1.y * vector2.x ;

 /*
 ** Normalize the vector
 */
//    s = 1.0 / sqrt((normalVectorP->x * normalVectorP->x) + (normalVectorP->y * normalVectorP->y) + (normalVectorP->z * normalVectorP->z));
//    normalVectorP->x *= s;
//    normalVectorP->y *= s;
//    normalVectorP->z *= s;

/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Normal Vector For Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Normal Vector For Triangle Error") ;
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmMath_transformDtmObject
(
 BC_DTM_OBJ *dtmP,                    /* Pointer To Dtm Object      */
 double  transformMatrix[3][4]        /* Transformation Matrix      */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   n,point,dtmFeature,priorTinState=FALSE,cubeSet=FALSE ;
 double x,y,z,dx,dy,scaleFactor=1.0 ;
 DPoint3d    *p3dP,*ptsP,scalePts[2] ;
 double xMin = 0.0,yMin = 0.0,xMax = 0.0,yMax = 0.0,zMin = 0.0,zMax = 0.0;
 DTM_TIN_POINT *pointP ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Transforming DTM") ;
    bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
    for( n = 0 ; n < 3 ; ++n )
      {
       bcdtmWrite_message(0,0,0,"transformMatrix[%2ld] = %12.5lf ** %12.5lf ** %12.5lf ** %12.5lf",n,transformMatrix[n][0],transformMatrix[n][1],transformMatrix[n][2],transformMatrix[n][3]) ;
      }
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Check Transform Can Be Applied To DTM State
*/
 if( dtmP->dtmState != DTMState::Tin && dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Transform Can Not Be Applied To DTM State") ;
    goto errexit ;
   }
 if( dtmP->dtmState == DTMState::Tin ) priorTinState = TRUE ;

// Log DTM Tolerances Prior To Transform

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"DTM Tolerances Prior To Transform") ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol   = %20.15lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol   = %20.15lf",dtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol  = %20.15lf",dtmP->mppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->maxSide = %20.15lf",dtmP->maxSide) ;
   }

/*
** Determine Scale Factor From Transformation Matrix
*/
 scalePts[0].x = 0.0   ; scalePts[0].y = 0.0 ; scalePts[0].z = 0.0 ; 
 scalePts[1].x = 100.0 ; scalePts[1].y = 0.0 ; scalePts[1].z = 0.0 ; 
 x = scalePts[0].x  ;
 y = scalePts[0].y  ;    
 z = scalePts[0].z  ;
 scalePts[0].x = x * transformMatrix[0][0] + y * transformMatrix[0][1] + z * transformMatrix[0][2] + transformMatrix[0][3] ;
 scalePts[0].y = x * transformMatrix[1][0] + y * transformMatrix[1][1] + z * transformMatrix[1][2] + transformMatrix[1][3] ;
 scalePts[0].z = x * transformMatrix[2][0] + y * transformMatrix[2][1] + z * transformMatrix[2][2] + transformMatrix[2][3] ;
 x = scalePts[1].x  ;
 y = scalePts[1].y  ;    
 z = scalePts[1].z  ;
 scalePts[1].x = x * transformMatrix[0][0] + y * transformMatrix[0][1] + z * transformMatrix[0][2] + transformMatrix[0][3] ;
 scalePts[1].y = x * transformMatrix[1][0] + y * transformMatrix[1][1] + z * transformMatrix[1][2] + transformMatrix[1][3] ;
 scalePts[1].z = x * transformMatrix[2][0] + y * transformMatrix[2][1] + z * transformMatrix[2][2] + transformMatrix[2][3] ;
 dx = scalePts[1].x - scalePts[0].x ;
 dy = scalePts[1].y - scalePts[0].y ; 
 scaleFactor = sqrt(dx*dx+dy*dy) / 100.0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"ScaleFactor = %12.8lf",scaleFactor) ;
/*
** Adjust DTM Tolerances By The Scale Factor
*/
 dtmP->ppTol = dtmP->ppTol * scaleFactor ;
 dtmP->plTol = dtmP->plTol * scaleFactor ;
 dtmP->maxSide = dtmP->maxSide * scaleFactor ;
/*
** Transform Points Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Transforming Points Array") ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    pointP = pointAddrP(dtmP,point) ;
    x = pointP->x ;
    y = pointP->y ;    
    z = pointP->z ;
    pointP->x = x * transformMatrix[0][0] + y * transformMatrix[0][1] + z * transformMatrix[0][2] + transformMatrix[0][3] ;
    pointP->y = x * transformMatrix[1][0] + y * transformMatrix[1][1] + z * transformMatrix[1][2] + transformMatrix[1][3] ;
    pointP->z = x * transformMatrix[2][0] + y * transformMatrix[2][1] + z * transformMatrix[2][2] + transformMatrix[2][3] ;
/*
**  Set Bounding Cube Limits
*/
    if( cubeSet == FALSE )
      {
       xMin = xMax = pointP->x ;
       yMin = yMax = pointP->y ;
       zMin = zMax = pointP->z ;
       cubeSet = TRUE ;
      }
    else
      {
       if( pointP->x < xMin ) xMin = pointP->x ;
       if( pointP->x > xMax ) xMax = pointP->x ;
       if( pointP->y < yMin ) yMin = pointP->y ;
       if( pointP->y > yMax ) yMax = pointP->y ;
       if( pointP->z < zMin ) zMin = pointP->z ;
       if( pointP->z > zMax ) zMax = pointP->z ;
      }
   }
/*
** Transform Features Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Transforming Points Array") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback || dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
      {
       ptsP = bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI);
       for( p3dP = ptsP ; p3dP < ptsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
         {
          x = p3dP->x ;
          y = p3dP->y ;    
          z = p3dP->z ;
          p3dP->x = x * transformMatrix[0][0] + y * transformMatrix[0][1] + z * transformMatrix[0][2] + transformMatrix[0][3] ;
          p3dP->y = x * transformMatrix[1][0] + y * transformMatrix[1][1] + z * transformMatrix[1][2] + transformMatrix[1][3] ;
          p3dP->z = x * transformMatrix[2][0] + y * transformMatrix[2][1] + z * transformMatrix[2][2] + transformMatrix[2][3] ;
         }
      }
   }
/*
** Set DTM Bounding Cube
*/
 if( cubeSet == TRUE )
   {
    dtmP->xMin = xMin ; dtmP->xMax = xMax ;
    dtmP->yMin = yMin ; dtmP->yMax = yMax ;
    dtmP->zMin = zMin ; dtmP->zMax = zMax ;
    dtmP->xRange = xMax - xMin ;
    dtmP->yRange = yMax - yMin ;
    dtmP->zRange = zMax - zMin ;
   }
/*
** Check If DTM Has To Be Re-Triangulated
*/
 if( priorTinState == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking DTM Precision") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,0))
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Retriangulating") ;
       if( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
       if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
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
      }
    if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
   }

//  Update Modified Time

 bcdtmObject_updateLastModifiedTime (dtmP) ;

// Log Tolerances After Transform

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"DTM Tolerances After Transform") ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol   = %20.15lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol   = %20.15lf",dtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol  = %20.15lf",dtmP->mppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->maxSide = %20.15lf",dtmP->maxSide) ;
   }

/*
** Clean Up
*/
 cleanup :
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Transforming DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Transforming DTM Error") ;
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
BENTLEYDTM_EXPORT int bcdtmMath_transformViaCallbackDtmObject
(
 BC_DTM_OBJ *dtmP,                        // Pointer To DTM
 DTMTransformPointsCallback loadFunction, // Pointer To Call Back Function 
 void*      userP                         // Pointer To User Value
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   point,dtmFeature,priorTinState=FALSE ;
 DPoint3d    *p3dP,*ptsP ;
 double dx,dy,scaleFactor,priorTransformLength,postTransformLength ;
 DTM_TIN_POINT *pointP,tinPnt ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Transforming DTM Via Function") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunction = %p",loadFunction) ;
    bcdtmWrite_message(0,0,0,"userP        = %p",dtmP) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Check Transform Can Be Applied To DTM State
*/
 if( dtmP->dtmState != DTMState::Tin && dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Transform Can Not Be Applied To DTM State") ;
    goto errexit ;
   }

//  Set DTM Prior State

 if( dtmP->dtmState == DTMState::Tin ) priorTinState = TRUE ;


// Calculate Prior Transform Length

 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
 dx = dtmP->xMax - dtmP->xMin ;
 dy = dtmP->yMax - dtmP->yMin ;
 priorTransformLength = sqrt(dx*dx+dy*dy) ;

// Log DTM Tolerances Prior To Transform

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Coordinate Ranges Prior To Transforming") ;
    bcdtmWrite_message(0,0,0,"Number Of Points = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"x ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->xMin,dtmP->xMax,dtmP->xMax-dtmP->xMin) ;
    bcdtmWrite_message(0,0,0,"y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->yMin,dtmP->yMax,dtmP->yMax-dtmP->yMin) ;
    bcdtmWrite_message(0,0,0,"z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->zMin,dtmP->zMax,dtmP->zMax-dtmP->zMin) ;
    bcdtmWrite_message(0,0,0,"DTM Tolerances Prior To Transform") ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol   = %20.15lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol   = %20.15lf",dtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol  = %20.15lf",dtmP->mppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->maxSide = %20.15lf",dtmP->maxSide) ;
   }

// Transform Points Array

 if( dbg ) bcdtmWrite_message(0,0,0,"Transforming Points Array") ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    pointP = pointAddrP(dtmP, point) ;
    tinPnt = *pointP ;
    loadFunction ((DPoint3d*)pointP, 1, userP);

    if( dbg && tinPnt.x == pointP->x && tinPnt.y == pointP->y )
      {
       bcdtmWrite_message(0,0,0,"Point[%8ld] ** Not Transformed",point) ;
      }
   }

//  Reset Bounding Cube

 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;

/*
** Transform Features Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Transforming Points Array") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback || dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
      {
       ptsP = bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI);
       for( p3dP = ptsP ; p3dP < ptsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
         {
          loadFunction ((DPoint3d*)p3dP, 1, userP);
         }
      }
   }


//  Log Coordinate Ranges

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Coordinate Ranges After Transforming Points Array") ;
    bcdtmWrite_message(0,0,0,"Number Of DTM Points     = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"x ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->xMin,dtmP->xMax,dtmP->xMax-dtmP->xMin) ;
    bcdtmWrite_message(0,0,0,"y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->yMin,dtmP->yMax,dtmP->yMax-dtmP->yMin) ;
    bcdtmWrite_message(0,0,0,"z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->zMin,dtmP->zMax,dtmP->zMax-dtmP->zMin) ;
   }

/*
** Check If DTM Has To Be Re-Triangulated
*/
 if( priorTinState == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking DTM Precision") ;
    if( bcdtmCheck_precisionDtmObject(dtmP,0))
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Retriangulating") ;
       if( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
       if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
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
      }
    if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
   }

// Calculate Post Transform Length And Scale Tolerances

 if( priorTransformLength != 0.0 )
   {
    dx = dtmP->xMax - dtmP->xMin ;
    dy = dtmP->yMax - dtmP->yMin ;
    postTransformLength = sqrt(dx*dx+dy*dy) ;
    if( postTransformLength != 0.0 )
      {
       scaleFactor = postTransformLength / priorTransformLength ;
       dtmP->ppTol = dtmP->ppTol * scaleFactor ;
       dtmP->plTol = dtmP->plTol * scaleFactor ;
       dtmP->maxSide = dtmP->maxSide * scaleFactor ;
      }
   }

// Log DTM Tolerances After Transforming

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Coordinate Ranges After Transforming") ;
    bcdtmWrite_message(0,0,0,"x ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->xMin,dtmP->xMax,dtmP->xMax-dtmP->xMin) ;
    bcdtmWrite_message(0,0,0,"y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->yMin,dtmP->yMax,dtmP->yMax-dtmP->yMin) ;
    bcdtmWrite_message(0,0,0,"z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->zMin,dtmP->zMax,dtmP->zMax-dtmP->zMin) ;
    bcdtmWrite_message(0,0,0,"DTM Tolerances After Transform") ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol   = %20.15lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol   = %20.15lf",dtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol  = %20.15lf",dtmP->mppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->maxSide = %20.15lf",dtmP->maxSide) ;
   }

//  Update Modified Time

 bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Transforming DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Transforming DTM Error") ;
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
BENTLEYDTM_EXPORT int  bcdtmMath_calculatesSlopeAtGivenAngleFromPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double Angle,double *elevationP,long *drapeFlagP,double *slopeP)
/*
** This Function Calculates The slopeP On The Tin Surface At A Given Angle From A Point
** The slopeP Is Calculated To The First Break Point. If A Break Point Doesnt Exist The
** slopeP Is Calculated To The First Drape Point
**
**
** Arguements
**
** Tin         ==> Tin Object
** x           ==> x Coordinate Of Point
** y           ==> y Coordinate Of Point
** Angle       ==> Angle To Calculate slopeP expressed in Radians
** drapeFlagP  <== 0   Point Not In Tin
**             <== 1   Point Coincident with existing Tin Point P1
**             <== 2   Point On Line P1P2
**             <== 3   Point On Hull Line P1P2
**             <== 4   Point In Triangle P1,P2,P3
** slopeP      <== slopeP At Angle Expressed As  Vertical / Horizontal
**
** Arguement Validation
**
** 1. No Validity Checking Of The Tin Object
** 2. No validity Checking Of x
** 2. No validity Checking Of y
** 2. No validity Checking Of Angle
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected
**
** Author :  Rob Cormack
** Date   :  10th December 2001
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs1,ofs2,HullFlag ;
 long   np,pp,P1,P2,P3,lf1,lf2,lf3,Ptype,numDrapePts ,breakPointFound,LineFlag ;
 double d1,d2,d3,radius,z,Xi,Yi,ang,angn,angp ;
 DPoint3d    LinePts[2] ;
 DTM_DRAPE_POINT *drapeP,*drapePtsP=NULL ;
/*
**Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Calculating slopeP At Given Angle From Point") ;
    bcdtmWrite_message(0,0,0,"dtmP  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x     = %10.4lf",x) ;
    bcdtmWrite_message(0,0,0,"y     = %10.4lf",y) ;
    bcdtmWrite_message(0,0,0,"Angle = %10.8lf",Angle) ;
    bcdtmWrite_toFileDtmObject(dtmP,L"shape.tin") ;
   }
/*
** Check For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated Dtm") ;
    goto errexit ;
   }
/*
** Initialise
*/
 *drapeFlagP = 0 ;
 *slopeP     = 0.0 ;
 *elevationP = 0.0 ;
 HullFlag = 0 ;
 while( Angle < 0.0  ) Angle += DTM_2PYE ;
 while( Angle > DTM_2PYE ) Angle -= DTM_2PYE ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Normalized Angle = %10.8lf",Angle) ;
/*
** Check Given Point Is Internal To Tin
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&z,&Ptype,&P1,&P2,&P3)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"00 Ptype = %1ld",Ptype) ;
/*
** If Point External Test If Point Within ppTol Of Hull Line Or Hull Point
*/
 if( Ptype == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Point External To Tin") ;
    bcdtmEdit_findClosestHullLineDtmObject(dtmP,x,y,&P1,&P2) ;
    if( P1 != dtmP->nullPnt || P2 != dtmP->nullPnt )
      {
       if     ( P1 != dtmP->nullPnt && bcdtmMath_distance(x,y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) <= dtmP->ppTol ) 
         { Ptype = 1 ; x = pointAddrP(dtmP,P1)->x ; y = pointAddrP(dtmP,P1)->y ; z = pointAddrP(dtmP,P1)->z ; }
       else if( P2 != dtmP->nullPnt && bcdtmMath_distance(x,y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) <= dtmP->ppTol ) 
         { Ptype = 1 ; x = pointAddrP(dtmP,P2)->x ; y = pointAddrP(dtmP,P2)->y ; z = pointAddrP(dtmP,P2)->z ; }
       else if( P1 != dtmP->nullPnt && P2 != dtmP->nullPnt && bcdtmMath_distanceOfPointFromLine(&LineFlag,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,x,y,&Xi,&Yi) < dtmP->ppTol )
         {
          Ptype = 3 ;  x = Xi ; y = Yi ; 
          bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,&z,P1,P2) ;
         }
      }
   }
/*
** If Point In Triangle Snap To Triangle Edge
*/
 if( Ptype == 4 )
   {
    d1 = bcdtmMath_distanceOfPointFromLine(&lf1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,x,y,&Xi,&Yi) ;
    d2 = bcdtmMath_distanceOfPointFromLine(&lf2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,x,y,&Xi,&Yi) ;
    d3 = bcdtmMath_distanceOfPointFromLine(&lf3,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,x,y,&Xi,&Yi) ;
    if     ( lf1 && d1 <= d2 && d1 <= d3 && d1 < dtmP->ppTol ) { Ptype = 2 ; P3 = dtmP->nullPnt ; }
    else if( lf2 && d2 <= d3 && d2 <= d1 && d2 < dtmP->ppTol ) { Ptype = 2 ; P1 = P2 ; P2 = P3 ; P3 = dtmP->nullPnt ; }
    else if( lf3 && d3 <= d1 && d3 <= d2 && d3 < dtmP->ppTol ) { Ptype = 2 ; P2 = P1 ; P1 = P3 ; P3 = dtmP->nullPnt ; }
    if( Ptype == 2 )
      {
       if      ( nodeAddrP(dtmP,P1)->hPtr == P2 )   Ptype = 3 ;
       else if ( nodeAddrP(dtmP,P2)->hPtr == P1 ) { Ptype = 3 ; P3 = P1 ; P1 = P2 ; P2 = P3 ; P3 = dtmP->nullPnt ; }
      }
    if( Ptype == 2 || Ptype == 3 )
      {
       d1 = bcdtmMath_distance(x,y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) ;
       d2 = bcdtmMath_distance(x,y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) ;
       if     ( d1 <= d2 && d1 < dtmP->ppTol ) { Ptype = 1 ; P2 = dtmP->nullPnt ; }
       else if( d2 <= d1 && d2 < dtmP->ppTol ) { Ptype = 1 ; P1 = P2 ; P2 = dtmP->nullPnt ; }
      }
   }
/*
** Set Point Type And elevationP
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"01 Ptype = %1ld",Ptype) ;
 *drapeFlagP = Ptype ;
 *elevationP = z ;
 if( Ptype == 0 ) { bcdtmWrite_message(1,0,0,"Given Point External To Tin") ; goto nosolution ; ; }
/*
** Check Angle If Point On Hull Point
*/
 if( Ptype == 1 && nodeAddrP(dtmP,P1)->hPtr != dtmP->nullPnt )
   {
    np = nodeAddrP(dtmP,P1)->hPtr ;
    if( (pp = bcdtmList_nextClkDtmObject(dtmP,P1,np)) < 0 ) goto errexit ;
    angn = bcdtmMath_getAngle(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y) ;
    angp = bcdtmMath_getAngle(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,pp)->x,pointAddrP(dtmP,pp)->y) ;
    ang  = Angle ;
    if( angn < angp ) angn += DTM_2PYE ;
    if( ang  < angp ) ang  += DTM_2PYE ;
    if( dbg ) bcdtmWrite_message(0,0,0,"PNT00 angp = %12.10lf ang = %12.10lf angn = %12.10lf",angp,ang,angn) ;
    if( ang >= angp && ang <= angn )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Flipping Angle") ;
       Angle += DTM_PYE ;
       if( Angle > DTM_2PYE ) Angle -= DTM_2PYE ;
       ang = Angle ;
       if( ang  < angp ) ang  += DTM_2PYE ;
       if( dbg ) bcdtmWrite_message(0,0,0,"PNT01 angp = %12.10lf ang = %12.10lf angn = %12.10lf",angp,ang,angn) ;
       if( ang >= angp && ang <= angn ) goto nosolution ; ;
      }
    HullFlag = 1 ;
   }
/*
** Check Angle If Point On Hull Line
*/
 if( Ptype == 3  )
   {
    angn = bcdtmMath_getAngle(x,y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) ;
    angp = bcdtmMath_getAngle(x,y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) ;
    ang  = Angle ;
    if( angn < angp ) angn += DTM_2PYE ;
    if( ang  < angp ) ang  += DTM_2PYE ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Hull00 angp = %12.10lf ang = %12.10lf angn = %12.10lf",angp,ang,angn) ;
    if( ang >= angp && ang <= angn )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Flipping Angle") ;
       Angle += DTM_PYE ;
       if( Angle > DTM_2PYE ) Angle -= DTM_2PYE ;
       ang = Angle ;
       if( ang  < angp ) ang  += DTM_2PYE ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Hull01 angp = %12.10lf ang = %12.10lf angn = %12.10lf",angp,ang,angn) ;
       if( ang >= angp && ang <= angn ) goto nosolution ; ;
      }
    HullFlag = 1 ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"HullFlag = %1ld",HullFlag) ;
/*
** Set Coordiantes For Drape
*/
 radius = sqrt(dtmP->xRange*dtmP->xRange+dtmP->yRange*dtmP->yRange) ;
 LinePts[0].x = x ;
 LinePts[0].y = y ;
 LinePts[0].z = z ;
 LinePts[1].x = x + radius * cos(Angle) ;
 LinePts[1].y = y + radius * sin(Angle) ;
 LinePts[1].z = z  ;
/*
** Drape Line String On DTMFeatureState::Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Draping String") ;
 if( bcdtmDrape_stringDtmObject(dtmP,LinePts,2,FALSE,&drapePtsP,&numDrapePts )) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Drape Points = %6ld",numDrapePts ) ;
    for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts  ; ++drapeP )
      {
       bcdtmWrite_message(0,0,0,"Drape Point[%4ld] Type = %1ld ** %10.4lf %10.4lf %10.4lf",(long)(drapeP-drapePtsP),drapeP->drapeType,drapeP->drapeX,drapeP->drapeY,drapeP->drapeZ) ;
      }
   }
/*
** Find First Non External Point At Start Of Drape Line
*/
 ofs1 = 0 ;
 while (ofs1 < numDrapePts && (drapePtsP + ofs1)->drapeType == DTMDrapedLineCode::External) ++ofs1;
/*
** Find First Non External Point At End Of Drape Line
*/
 ofs2 = numDrapePts  ;
 while( ofs2 >= 0 && (drapePtsP+ofs2)->drapeType == DTMDrapedLineCode::External) --ofs2 ;
/*
** Check For No Solution
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"ofs1 = %4ld ofs2 = %4ld",ofs1,ofs2) ;
 if( ofs1 >= numDrapePts  ) goto nosolution ;
 if( ofs2 <     0         ) goto nosolution ;
 if( ofs1 >= ofs2         ) goto nosolution ;
/*
** Scan To First Drape break Point
*/
 breakPointFound = 0 ;
 for( drapeP = drapePtsP + ofs1 + 1 ; drapeP <= drapePtsP + ofs2 && ! breakPointFound ; ++drapeP )
   {
   if (drapeP->drapeType == DTMDrapedLineCode::Breakline)
      {
       if( bcdtmMath_distance(x,y,drapeP->drapeX,drapeP->drapeY) >= dtmP->ppTol ) breakPointFound = (long)(drapeP-drapePtsP) ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"00 breakPointFound = %4ld",breakPointFound) ;
 if( ! breakPointFound  ) goto nosolution ; ;
/*
** Calculate Slope
*/
 *slopeP = ((drapePtsP+breakPointFound)->drapeZ - z) / bcdtmMath_distance(x,y,(drapePtsP+breakPointFound)->drapeX,(drapePtsP+breakPointFound)->drapeY) ;
/*
** Clean Up
*/
 cleanup :
 if( drapePtsP != NULL ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Slope At Given Angle From Point Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Calculating Slope At Given Angle From Point Error") ;
 if( dbg && ret == 2           ) bcdtmWrite_message(0,0,0,"Calculating Slope At Given Angle From Point ** No Solution") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
/*
** No Solution Exit
*/
 nosolution :
 ret = 2 ;
 goto cleanup ;
}
BENTLEYDTM_EXPORT int bcdtmMath_translateCoordinatesDtmObject(BC_DTM_OBJ *dtmP,double Xinc,double Yinc,double Zinc)
/*
** This Function Translate dtmP Object Coordinates
**
**  Return Values  =  0  Succesfull
**                 =  1  User Or Sytem Error
**                 =  2  Precision Errors
**                 = 20  Pre 98 dtmP File
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  point ;
 DTM_TIN_POINT *pointP ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Translating Coordinates dtmP Object") ;
    bcdtmWrite_message(0,0,0,"dtmP Object        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x Translate  = %12.5lf",Xinc)  ;
    bcdtmWrite_message(0,0,0,"y Translate  = %12.5lf",Yinc)  ;
    bcdtmWrite_message(0,0,0,"z Translate  = %12.5lf",Zinc)  ;
   }
/*
** Test For Valid dtmP Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test For 98 Tin File
*/
 if( dtmP->ppTol == 0.0 )
   {
    bcdtmWrite_message(1,0,0,"Pre 98 Tin File") ;
    ret = 20 ;
    goto errexit ;
   }
/*
** Translate Coordinates
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Translating Dtm Coordinates") ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    pointP = pointAddrP(dtmP,point) ;
    pointP->x = pointP->x + Xinc ;
    pointP->y = pointP->y + Yinc ;
    pointP->z = pointP->z + Zinc ;
   }
/*
** Re Calculate Bounding Cube For Dtm
*/
 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
/*
** Check For Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
/*
**  Resort Dtm Structure
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Dtm") ;
    if( bcdtmList_resortTinStructureDtmObject(dtmP)) goto errexit ;
/*
**  Check Precision
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm Precision") ;
    if(( ret = bcdtmCheck_precisionDtmObject(dtmP,0)) != DTM_SUCCESS )
      {
       if( ret == 2 )
         {
          if( bcdtmMerge_checkAndFixPrecisionForTrianglesDtmObject(dtmP) ) goto errexit ;
          if(( ret = bcdtmCheck_precisionDtmObject(dtmP,0)) != DTM_SUCCESS )
            {
             if( ret == 2 )
               {
                bcdtmWrite_message(1,0,0,"Precision Errors In Translated DTM") ;
                goto errexit ;
               }
             else goto errexit ;
            }
         }
       else goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmMath_rotateCoordinatesDtmObject(BC_DTM_OBJ *dtmP,double Xorg,double Yorg,double Angle)
/*
** This Function Rotates Dtm Object Coordinates
**
** Xorg,Yorg  = Coordinates Of Rotation Point
** Angle      = Rotation Angle In Decimal Degrees
**
**  Return Values  =  0  Succesfull
**                 =  1  User Or Sytem Error
**                 =  2  Precision Errors
**                 = 20  Pre 98 Dtm File
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   point ;
 double Tx,Ty,Radius,Pangle ;
 DTM_TIN_POINT  *pointP ;
/*
** Write Status Message
*/
 if(  dbg )
   {
    bcdtmWrite_message(0,0,0,"Rotate Coordinates Dtm Object") ;
    bcdtmWrite_message(0,0,0,"Dtm Object     = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x Origon       = %12.5lf",Xorg)  ;
    bcdtmWrite_message(0,0,0,"y Origon       = %12.5lf",Yorg)  ;
    bcdtmWrite_message(0,0,0,"Rotation Angle = %12.8lf",Angle)  ;
   }
/*
** Test For Valid dtmP Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Test For 98 Tin File
*/
 if( dtmP->ppTol == 0.0 )
   {
    bcdtmWrite_message(1,0,0,"Pre 98 Tin File") ;
    ret = 20 ;
    goto errexit ;
   }
/*
** Convert Angle To Radians
*/
 if( Angle == 0.0 || Angle == 360.0 ) return(0) ;
 Angle = fmod(Angle,360.0) ;
 if( Angle == 0.0 ) return(0) ;
 Angle = Angle / 360.0 * DTM_2PYE ;
/*
** Get Translation Constants
*/
 Tx = Xorg - 0.0 ;
 Ty = Yorg - 0.0 ;
/*
** Rotate Coordinates
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Rotating Dtm Coordinates") ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    pointP = pointAddrP(dtmP,point) ;
    pointP->x  = pointP->x - Tx ;
    pointP->y  = pointP->y - Ty ;
    Radius = sqrt( pointP->x * pointP->x + pointP->y * pointP->y );
    Pangle = atan2(pointP->y,pointP->x) ;
    pointP->x  = Radius * cos(Pangle+Angle) + Tx ;
    pointP->y  = Radius * sin(Pangle+Angle) + Ty ;
   }
/*
** Re Calculate Bounding Cube For Dtm
*/
 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
/*
** Check For Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
/*
**  Resort Dtm Structure
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Dtm") ;
    if( bcdtmList_resortTinStructureDtmObject(dtmP)) goto errexit ;
/*
**  Check Precision
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm Precision") ;
    if(( ret = bcdtmCheck_precisionDtmObject(dtmP,0)) != DTM_SUCCESS )
      {
       if( ret == 2 )
         {
          if( bcdtmMerge_checkAndFixPrecisionForTrianglesDtmObject(dtmP) ) goto errexit ;
          if(( ret = bcdtmCheck_precisionDtmObject(dtmP,0)) != DTM_SUCCESS )
            {
             if( ret == 2 )
               {
                bcdtmWrite_message(1,0,0,"Precision Errors In Rotated DTM") ;
                goto errexit ;
               }
             else goto errexit ;
            }
         }
       else goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmMath_convertCoordinatesDtmObject(BC_DTM_OBJ *dtmP,double ConversionFactor)
/*
** This Function Converts Tin Object Coordinates
**
**  Return Values  =  0  Succesfull
**                 =  1  User Or Sytem Error
**                 =  2  Precision Errors
**                 = 20  Pre 98 Tin File
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  point ;
 DTM_TIN_POINT *pointP ;
/*
** Write Status Message
*/
 if(  dbg )
   {
    bcdtmWrite_message(0,0,0,"Converting Coordinates Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Conversion Factor = %12.5lf",ConversionFactor)     ;
   }
/*
** Test For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test Conversion Factor Is Not Zerorsion Factor
*/
 if( ConversionFactor == 0.0 )
   {
    bcdtmWrite_message(1,0,0,"Zero Conversion Factor") ; goto errexit ;
   }
 if( ConversionFactor == 1.0 )  goto cleanup ;
/*
** Convert Coordinates
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Converting Tin Coordinates") ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    pointP = pointAddrP(dtmP,point) ;
    pointP->x = pointP->x * ConversionFactor ;
    pointP->y = pointP->y * ConversionFactor ;
    pointP->z = pointP->z * ConversionFactor ;
   }
/*
** Convert Tolerances
*/
 dtmP->ppTol = dtmP->ppTol * ConversionFactor ;
 dtmP->plTol = dtmP->plTol * ConversionFactor ;
/*
** Resort Tin Structure
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Tin") ;
    if( bcdtmTin_resortTinStructureDtmObject(dtmP)) goto errexit ;
   }
/*
** Re Calculate Bounding Cube For DTMFeatureState::Tin
*/
 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
/*
** Check Precision
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
 if( bcdtmCheck_precisionDtmObject(dtmP,0) )
   {
    bcdtmWrite_message(0,0,0,"Fixing Tin Precision") ;
    if( bcdtmMerge_checkAndFixPrecisionForTrianglesDtmObject(dtmP) ) goto errexit ;
    if( bcdtmTin_resortTinStructureDtmObject(dtmP)) goto errexit ;
    if( bcdtmCheck_precisionDtmObject(dtmP,0))
      {
       bcdtmWrite_message(2,0,0,"Precision Errors In Converted Tin") ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Converting Coordinates Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Converting Coordinates Dtm Object Error") ;
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
BENTLEYDTM_EXPORT int bcdtmMath_convertUnitsDtmObject(BC_DTM_OBJ *dtmP, double xyFactor, double zFactor)
{
 int    ret=DTM_SUCCESS ;
 long   point ;
 DTM_TIN_POINT *pointP ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
**  Scan DTM Coordinates And Convert
*/
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    pointP = pointAddrP(dtmP,point) ;
    pointP->x = pointP->x * xyFactor ;
    pointP->y = pointP->y * xyFactor ;
    pointP->z = pointP->z * zFactor  ;
   }
/*
** Rset Bounding Cube
*/
 if( bcdtmMath_setBoundingCubeDtmObject(dtmP)) goto errexit ;
/*
**  Update Modified Time
 */
 bcdtmObject_updateLastModifiedTime (dtmP) ;
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
BENTLEYDTM_Public int bcdtmMath_getPointArrayExtents
(
 DPoint3d  *pointsP,
 long numPoints,
 double *xMinP,
 double *yMinP,
 double *zMinP,
 double *xMaxP,
 double *yMaxP,
 double *zMaxP
)
{
 int   ret=DTM_SUCCESS ;
 DPoint3d   *pointP ;
/*
** Initialise
*/
 *xMinP = *yMinP = *zMinP = 0.0 ;
 *xMaxP = *yMaxP = *zMaxP = 0.0 ;
/*
** Test For Valid Point Array
*/
 if( pointsP == NULL || numPoints < 0 )
   {
    bcdtmWrite_message(1,0,0,"Invalid Point Array") ;
    goto errexit ;
   }
/*
** Scan Point Array
*/
 *xMinP = *xMaxP = pointsP->x ;
 *yMinP = *yMaxP = pointsP->y ;
 *zMinP = *zMaxP = pointsP->z ;
 for( pointP = pointsP + 1 ; pointP < pointsP + numPoints ; ++pointP )
   {
    if( pointP->x < *xMinP ) *xMinP = pointP->x ;
    if( pointP->x > *xMaxP ) *xMaxP = pointP->x ;
    if( pointP->y < *yMinP ) *yMinP = pointP->y ;
    if( pointP->y > *yMaxP ) *yMaxP = pointP->y ;
    if( pointP->z < *zMinP ) *zMinP = pointP->z ;
    if( pointP->z > *zMaxP ) *zMaxP = pointP->z ;
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

//#pragma optimize( "p",on)
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT double bcdtmMath_getAngle(double x1,double y1,double x2,double y2)
{
 double dx,dy,ang ;
 dx  = x2 - x1 ;
 dy  = y2 - y1 ;
 ang = atan2(dy,dx) ;
 if( ang < 0.0 ) ang += DTM_2PYE ;
 return( ang ) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_distance(double X1,double Y1,double X2,double Y2)
{
 double x,y ;
 x = ( X2 - X1 ) * ( X2 - X1 ) ;
 y = ( Y2 - Y1 ) * ( Y2 - Y1) ;
 return(sqrt(x+y)) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT double bcdtmMath_distance3D(double X1,double Y1,double Z1,double X2,double Y2,double Z2)
{
 double x,y,z ;
 x = ( X2 - X1 ) * ( X2 - X1 ) ;
 y = ( Y2 - Y1 ) * ( Y2 - Y1 ) ;
 z = ( Z2 - Z1 ) * ( Z2 - Z1 ) ;
 return(sqrt(x+y+z)) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_roundToTolerance(double Value,double Tolerance)
{
 long  lval ;
 static thread_local double mval, dval, Ltolerance = -9999.99;
/*
** If Tolerance Changed Calculate New Values
*/
 if( Tolerance != Ltolerance )
   {
    Ltolerance = Tolerance ;
    if( Tolerance < 0.0 ) Tolerance = -Tolerance ;
    mval = 1.0 ;
    while( Tolerance < 1.0 )  { mval = mval * 10.0 ; Tolerance = Tolerance * 10.0 ; }
    dval = mval / 10.0 ;
   }
/*
**  Round Value
*/
 lval  = ((long)  ( Value * mval + 5.0 )) / 10 ;
 Value = ((double) lval ) / dval ;
/*
** Job Completed
*/
 return(Value) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_roundToDecimalPoints(double Value,long Ndd)
{
 long   lval ;
 double fval,frct ;
 static thread_local long  ndd = -99;
 static thread_local double mval, dval;
/*
** Check Number Of Decimal Points
*/
 if( Ndd < 0 ) Ndd = 0 ;
 if( Ndd > 8 ) Ndd = 8 ;
/*
** If Tolerance Changed Calculate New Values
*/
 if( ndd != Ndd )
   {
    ndd  = Ndd ; ;
    dval = pow(10.0,(double)Ndd) ;
    mval = pow(10.0,(double)(Ndd+1))  ;
   }
/*
**  Round Value
*/
 fval  = floor(Value) ;
 frct  = Value - fval ;
 lval  = ((long) ( frct * mval + 5.0 )) / 10  ;
 if( dval != 0 ) Value = fval + ((double) lval ) / dval ;
 else            Value = fval + ((double) lval )  ;
/*
** Job Completed
*/
 return(Value) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmMath_sideOf(double x1,double y1,double x2,double y2,double x3,double y3)
{
 int ret=0 ;
 double sd1,sd2 ;
 sd1 = (x1-x3) * (y2-y3) - (y1-y3) * (x2-x3) ;
 sd2 = (x1-x2) * (y3-y2) - (y1-y2) * (x3-x2) ;
 if( ( sd1 > 0.0 && sd2 < 0.0 ) || ( sd1 < 0.0 && sd2 > 0.0 ))
   {
    if     ( sd1 <  0.0 ) ret = -1 ; /* Right of Line */
    else if( sd1 == 0.0 ) ret =  0 ; /* On Line       */
    else                  ret =  1 ; /* Left of Line  */
   }
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_allSideOf(double x1,double y1,double x2,double y2,double x3,double y3)
{
 int ret=0 ;
 double sd1,sd2,sd3 ;
 sd1 = ((x1-x3) * (y2-y3)) - ((y1-y3) * (x2-x3))  ;
 sd2 = ((x2-x1) * (y3-y1)) - ((y2-y1) * (x3-x1)) ;
 sd3 = ((x3-x2) * (y1-y2)) - ((y3-y2) * (x1-x2)) ;
 if( ( sd1 < 0.0 && sd2 >= 0.0 ) || ( sd1 > 0.0 && sd2 <= 0.0) ) return(0) ;
 if( ( sd1 < 0.0 && sd3 >= 0.0 ) || ( sd1 > 0.0 && sd3 <= 0.0) ) return(0) ;
 if( sd1 <  0.0 ) ret = -1 ; /* Right of Line */
 if( sd1 == 0.0 ) ret =  0 ; /* On Line       */
 if( sd1 >  0.0 ) ret =  1 ; /* Left of Line  */
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_returnSideOf(double x1,double y1,double x2,double y2,double x3,double y3)
{
 return((x1-x3) * (y2-y3) - (y1-y3) * (x2-x3)) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_intersectCordLines(double x1,double y1,double x2,double y2,double x3,double y3,double x4,double y4,double *xc,double *yc)
/*
** This Function Intersects Two Lines
** Return Values  ==  0  No Intersection
**                ==  1  Intersection
**
*/
{
 int    sd1,sd2,sd3,sd4 ;
 double xn1,xm1,yn1,ym1,xn2,xm2,yn2,ym2 ;
/*
** Initialise Variables
*/
 *xc = *yc = 0.0 ;
 if( x1 <= x2 ) { xn1 = x1 ; xm1 = x2 ; } else { xn1 = x2 ; xm1 = x1 ; }
 if( y1 <= y2 ) { yn1 = y1 ; ym1 = y2 ; } else { yn1 = y2 ; ym1 = y1 ; }
 if( x3 <= x4 ) { xn2 = x3 ; xm2 = x4 ; } else { xn2 = x4 ; xm2 = x3 ; }
 if( y3 <= y4 ) { yn2 = y3 ; ym2 = y4 ; } else { yn2 = y4 ; ym2 = y3 ; }
 if( xn1 > xm2 || xm1 < xn2 || yn1 > ym2 || ym1 < yn2 ) return(0) ;
/*
** Calculate SideOf Values
*/
 sd1 = bcdtmMath_sideOf(x3,y3,x4,y4,x1,y1) ;
 sd2 = bcdtmMath_sideOf(x3,y3,x4,y4,x2,y2) ;
 if( sd1 == sd2 ) return(0) ;
 sd3 = bcdtmMath_sideOf(x1,y1,x2,y2,x3,y3) ;
 sd4 = bcdtmMath_sideOf(x1,y1,x2,y2,x4,y4) ;
 if( sd3 == sd4  ) return(0) ;
/*
** Calculate Intercept Point
*/
 bcdtmMath_normalIntersectCordLines(x1,y1,x2,y2,x3,y3,x4,y4,xc,yc) ;
/*
** Adjust Intercept Coordinates
*/
 if( xn1 == xm1 ) *xc = xn1 ;
 if( yn1 == ym1 ) *yc = yn1 ;
 if( xn2 == xm2 ) *xc = xn2 ;
 if( yn2 == ym2 ) *yc = yn2 ;
/*
** Job Completed
*/
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int  bcdtmMath_normalIntersectCordLines(double X1,double Y1,double X2,double Y2,double X3,double Y3,double X4,double Y4,double *Xi,double *Yi)
/*
** This Function Calculates The Intersect Point Of Two Lines
*/
{
 double  n1,n2 ;
/*
** Check For Coincident End Points
*/
 if     ( ( X1 == X3 && Y1 == Y3 ) || ( X1 == X4 && Y1 == Y4 ) ) { *Xi = X1 ; *Yi = Y1 ; }
 else if( ( X2 == X3 && Y2 == Y3 ) || ( X2 == X4 && Y2 == Y4 ) ) { *Xi = X2 ; *Yi = Y2 ; }
/*
** Normal Intersect
*/
 else
   {
    n1  = bcdtmMath_normalDistanceToCordLine(X1,Y1,X2,Y2,X3,Y3) ;
    n2  = bcdtmMath_normalDistanceToCordLine(X1,Y1,X2,Y2,X4,Y4) ;
    if( n1+n2 != 0.0 )
      {
       *Xi = X3 + (X4-X3) * (n1/(n1+n2)) ;
       *Yi = Y3 + (Y4-Y3) * (n1/(n1+n2)) ;
      }
    else                                         // Parallel Lines - Not Sure What To Do
      {
       n1 = bcdtmMath_distance(X1,Y1,X3,Y3) ;
       n2 = bcdtmMath_distance(X1,Y1,X4,Y4) ;
       if( n1 <= n2 ) { *Xi = X3 ; *Yi = Y3 ; }
       else           { *Xi = X4 ; *Yi = Y4 ; }
      }
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
| bcdtmMath_normalDistanceToCordLine()                                 |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_normalDistanceToCordLine(double X1,double Y1,double X2,double Y2, double x, double y )
/*
** This Function Return the Normal Distance Of a Point From A Line
*/
{
 double r,d,dx,dy,a1,a2,a3 ;
/*
** Initialise Variables
*/
 d = 0.0 ;
 dx = X1 - X2 ;
 dy = Y1 - Y2 ;
 r  = sqrt( dx*dx + dy*dy) ;
 if( r > 0.0 )
   {
    a1 =  dy / r ;
    a2 = -dx / r ;
    a3 = -a1 * X1 - a2 * Y1 ;
    d  = a1 * x + a2 * y + a3 ;
   }
 else d = sqrt( (x-X1)*(x-X1) + (y-Y1)*(y-Y1) ) ;
 if( d < 0.0 ) d = -d ;
/*
** Job Completed
*/
 return(d) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_getPolygonDirectionP3D(DPoint3d *Polygon,long numPts,DTMDirection* Direction,double *Area)
/*
**  This Function Determines The Direction Of A Polygon
*/
 {
  double x,y   ;
  DPoint3d    *p3d ;
/*
** Initialise Varaibles
*/
  *Area = 0.0 ;
/*
** Sum Area Of Polygon
*/
  for ( p3d = Polygon + 1 ; p3d < Polygon + numPts ; ++p3d )
    {
     x = p3d->x - (p3d-1)->x ;
     y = p3d->y - (p3d-1)->y ;
     *Area = *Area + x * y / 2.0 + x * (p3d-1)->y ;
    }
/*
** Set Direction
*/
 if( *Area >= 0.0 ) *Direction = DTMDirection::Clockwise ;         /* ClockWise */
 else  { *Direction = DTMDirection::AntiClockwise ; *Area = -*Area ; } /* Anti Clockwise */
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
BENTLEYDTM_Public int bcdtmMath_reversePolygonDirectionP3D(DPoint3d *Polygon,long numPts)
/*
**  This Function Reverses The Direction Of a Polygon
*/
{
 DPoint3d *p3d1,*p3d2,*p3d3,TmpPnt ;
/*
** Swap Coordinates
*/
 p3d1 = Polygon ;
 p3d2 = Polygon + numPts - 1 ;
 p3d3 = &TmpPnt ;
 while ( p3d1 < p3d2 )
   {
    *p3d3 = *p3d1 ;
    *p3d1 = *p3d2 ;
    *p3d2 = *p3d3 ;
    ++p3d1 ; --p3d2 ;
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
BENTLEYDTM_Public double bcdtmMath_calculateRadiusOfCircumscribedCircleDtmObject
(
 BC_DTM_OBJ *dtmP,
 long        P1,
 long        P2,
 long        P3
)
/*
** This Function Calculates The Radius Of the Circumscribed Circle Through
** The Three Points P1,P2,P3
**
**  Radius = c / 2.0  * sin(C)
**
*/
{
 double c,dx,dy,a1,a2,a3,x1,y1,x2,y2,x3,y3,radius,sinA3 ;
/*
** Set Point Coordinate Values
*/
 x1 = pointAddrP(dtmP,P1)->x ; y1 = pointAddrP(dtmP,P1)->y ;
 x2 = pointAddrP(dtmP,P2)->x ; y2 = pointAddrP(dtmP,P2)->y ;
 x3 = pointAddrP(dtmP,P3)->x ; y3 = pointAddrP(dtmP,P3)->y ;
/*
** Calculate Constants
*/
 dx = x1-x2 ; dy = y1-y2  ;
 c  = sqrt(dx*dx + dy*dy) ;
 dx = x1-x3 ; dy = y1-y3  ; a1 = atan2(dy,dx) ;
 dx = x2-x3 ; dy = y2-y3  ; a2 = atan2(dy,dx) ;
 a3 = a1 - a2 ;
 if( a3 < 0.0 ) a3 = -a3 ;
 if( a3 > DTM_PYE ) a3 = a3 - DTM_PYE ;
/*
** Calculate Radius
*/
 sinA3 = sin(a3) ;
 if( sinA3 != 0.0 ) radius = c / ( 2.0 * sinA3) ;
 else               radius = DTM_INFINITE ;
/*
** Job Completed
*/
 return(radius) ;
}


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_calculateRadiusCircumscribedCircleCord(double x1,double y1,double x2,double y2,double x3,double y3)
/*
** This Function Calculates The Radius Of the Circumscribed Circle Through
** The Three Points P1,P2,P3
**
**  Radius = c / 2.0  * sin(C)
**
*/
{
 double c,dx,dy,a1,a2,a3,radius ;
/*
** Calculate Constants
*/
 dx = x1-x2 ; dy = y1-y2  ;
 c  = sqrt(dx*dx + dy*dy) ;
 dx = x1-x3 ; dy = y1-y3  ; a1 = atan2(dy,dx) ;
 dx = x2-x3 ; dy = y2-y3  ; a2 = atan2(dy,dx) ;
 a3 = a1 - a2 ;
 if( a3 < 0.0 ) a3 = -a3 ;
 if( a3 > DTM_PYE ) a3 = a3 - DTM_PYE ;
/*
** Calculate Radius
*/
 radius = c / ( 2.0 * sin(a3)) ;
/*
** Job Completed
*/
 return(radius) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_calculatePrecisionFromLarge(double Large,double *Tpptol, double *Tpltol,long *Nsd,long *Nid,long *Ndd)
/*
** This Function Calculates Precision Tolerances
*/
{
 long   nsd,nid,ndd,Iceil ;
 double Dceil ;
/*
** Initialise
*/
 *Nsd = *Nid = *Ndd = 0 ;
 *Tpptol = *Tpltol  = 0.0 ;
/*
** Get Largest Interger Value
*/
 Dceil = ceil(Large) ;
 Iceil = (long) Dceil ;
/*
** Get Number Of Decimal Digits
*/
 nid = 0 ;
 while( Iceil > 0 ) { Iceil = Iceil / 10 ; ++nid ; }
/*
** Get Number Of Decimal Digits
*/
 nsd = DBL_DIG ;
 ndd = nsd - nid ;
/*
** Calculate Precision Tolerances
*/
 *Tpptol = *Tpltol =  1.0 / pow(10.0,(double)ndd) ;
 *Nsd = nsd ; *Nid = nid ; *Ndd = ndd ;
/*
**  Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmMath_interpolatePointOnLine(double X1,double Y1,double Z1,double X2,double Y2,double Z2,double X3,double Y3,double *Z3 )
/*
** This Function Interpolates the Point x,y on Line P1,P2
*/
{
 double dx,dy,dz ;
/*
** Initialise Varibles
*/
 *Z3 = 0.0 ;
 dx = X2 - X1 ;
 dy = Y2 - Y1 ;
 dz = Z2 - Z1 ;
 if( dz == 0.0 ) { *Z3 = Z1 ; return(0) ; }
 if( dx == 0.0 && dy == 0.0 ) { *Z3 = Z1 ; return(0) ; }
/*
** Interpolate Point
*/
 if( fabs(dx) >= fabs(dy) ) *Z3 = Z1  + (( X3 - X1 ) / dx ) * dz ;
 else                       *Z3 = Z1  + (( Y3 - Y1 ) / dy ) * dz ;
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
BENTLEYDTM_Public int bcdtmMath_interpolatePointOnTriangle(double x,double y,double *ZP,double trgX[],double trgY[],double trgZ[])
{
 bool    newTriangle=false ;
 static  thread_local double  sTrgX[3]={3*0.0} ;
 static  thread_local double  sTrgY[3] = {3 * 0.0};
 static  thread_local double  sTrgZ[3] = {3 * 0.0};
/*
** Test For New Triangle
*/
 if     ( trgX[0] != sTrgX[0] || trgY[0] != sTrgY[0] || trgZ[0] != sTrgZ[0] ) newTriangle = true ;
 else if( trgX[1] != sTrgX[1] || trgY[1] != sTrgY[1] || trgZ[1] != sTrgZ[1] ) newTriangle = true ;
 else if( trgX[2] != sTrgX[2] || trgY[2] != sTrgY[2] || trgZ[2] != sTrgZ[2] ) newTriangle = true ;
/*
** Set Up For New Triangle
*/
 if( newTriangle )
   {
    sTrgX[0] = trgX[0] ;
    sTrgY[0] = trgY[0] ;
    sTrgZ[0] = trgZ[0] ;
    sTrgX[1] = trgX[1] ;
    sTrgY[1] = trgY[1] ;
    sTrgZ[1] = trgZ[1] ;
    sTrgX[2] = trgX[2] ;
    sTrgY[2] = trgY[2] ;
    sTrgZ[2] = trgZ[2] ;
   }
/*
** Interpolate Point On Triangle
*/
 bcdtmMath_interpolateLinear(newTriangle,x,y,ZP,sTrgX,sTrgY,sTrgZ) ;
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
BENTLEYDTM_Private int bcdtmMath_interpolateLinear(long newTriangle,double x,double y,double *z,double xt[3],double yt[3],double zt[3] )
/*
** This routine finds the z value for a point on the triangle plane
*/
{
 static thread_local bool   flatTriangle=false ;
 static thread_local double ca=0.0,cb=0.0,cc=0.0,cd=0.0 ;
 static thread_local double xmin=0.0,ymin=0.0,zmin=0.0,flatZ=0.0 ;
/*
** Normalise Triangle
*/
 if( newTriangle )
   {
    xmin = xt[0] ;
    ymin = yt[0] ;
    zmin = zt[0] ;
    if( xt[1] < xmin ) xmin = xt[1] ;
    if( xt[2] < xmin ) xmin = xt[2] ;
    if( yt[1] < ymin ) ymin = yt[1] ;
    if( yt[2] < ymin ) ymin = yt[2] ;
    if( zt[1] < zmin ) zmin = zt[1] ;
    if( zt[2] < zmin ) zmin = zt[2] ;
    if( zt[0] == zt[1] && zt[0] == zt[2] ) { flatTriangle = true ; flatZ = zt[0] ; }
    else                                     flatTriangle = false ;
   }
/*
** Calculate Coefficients of Plane
*/
 if( newTriangle )
   {
    bcdtmMath_calculatePlaneCoefficients(xt[0]-xmin,yt[0]-ymin,zt[0]-zmin,xt[1]-xmin,yt[1]-ymin,zt[1]-zmin,xt[2]-xmin,yt[2]-ymin,zt[2]-zmin,&ca,&cb,&cc,&cd ) ;
   }
/*
** Calculate z value
*/
 if( flatTriangle ) *z = flatZ ;
 else               *z = - ( ca * (x-xmin) + cb * (y-ymin) + cd ) / cc  +  zmin ;
/*
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
BENTLEYDTM_Public int bcdtmMath_interpolatePointOnPlane(double x,double y,double *z,double Ca,double Cb,double Cc,double Cd )
/*
** This routine finds the z value for a point on the triangle plane
*/
{
/*
** Calculate z value
*/
 *z = - ( Ca * x + Cb * y + Cd ) / Cc ;
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
BENTLEYDTM_Public int bcdtmMath_calculatePlaneCoefficients(double X1,double Y1,double Z1,double X2,double Y2,double Z2,double X3,double Y3,double Z3,double *A,double *B,double *C,double *D )
/*
** This Function Calculates The Coefficients For A Plane
*/
{
 *A = Y1 * ( Z2 - Z3 ) + Y2 * ( Z3 - Z1 ) + Y3 * ( Z1 - Z2 ) ;
 *B = Z1 * ( X2 - X3 ) + Z2 * ( X3 - X1 ) + Z3 * ( X1 - X2 ) ;
 *C = X1 * ( Y2 - Y3 ) + X2 * ( Y3 - Y1 ) + X3 * ( Y1 - Y2 ) ;
 *D = - X1 * ( Y2 * Z3 - Y3 * Z2 )
      - X2 * ( Y3 * Z1 - Y1 * Z3 )
      - X3 * ( Y1 * Z2 - Y2 * Z1 ) ;
/*
** Check For z Singularity
*/
 if( *C == 0.0 ) *C = 0.000000001  ;
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
BENTLEYDTM_Public int bcdtmMath_validateStringP3D(DPoint3d *StringPts,long *NumStringPts,double Tolerance)
/*
** This Function Validates A Non Polygonal String
*/
{
 long   CloseFlag ;
 double Xi,Yi ;
 DPoint3d    *p3d1,*p3d2 ;
/*
** Test For Closure
*/
 CloseFlag = 0 ;
 p3d1 = StringPts ;
 p3d2 = StringPts + *NumStringPts - 1 ;
 if( p3d1->x == p3d2->x && p3d1->y == p3d2->y ) CloseFlag = 1 ;
 if( CloseFlag ) { bcdtmWrite_message(0,0,0,"String Closes") ; return(1) ; }
/*
** Eliminate String Points Within Tolerance
*/
 for( p3d1 = StringPts , p3d2 = StringPts + 1 ; p3d2 < StringPts + *NumStringPts  ; ++p3d2 )
   {
    if( bcdtmMath_distance(p3d1->x,p3d1->y,p3d2->x,p3d2->y) >= Tolerance )
      {
       ++p3d1 ;
       *p3d1 = *p3d2 ;
      }
   }
/*
** Set Number of String Points
*/
 *NumStringPts = (long)(p3d1-StringPts+1) ;
/*
** Check For Coincident Points On String
*/
 for( p3d1 = StringPts ; p3d1 < StringPts + *NumStringPts - 1 ; ++p3d1 )
   {
    for( p3d2 = p3d1 + 1 ; p3d2 < StringPts + *NumStringPts - 1 ; ++p3d2 )
      {
       if( p3d1->x == p3d2->x && p3d1->y == p3d2->y  )
         { bcdtmWrite_message(1,0,0,"Knot In String") ; return(1) ; }
      }
   }
/*
** Check For Crossing String Lines
*/
 for( p3d1 = StringPts ; p3d1 < StringPts + *NumStringPts - 1 ; ++p3d1 )
   {
    for( p3d2 = p3d1 + 2 ; p3d2 < StringPts + *NumStringPts - 1 ; ++p3d2 )
      {
       if(bcdtmMath_intersectCordLines(p3d1->x,p3d1->y,(p3d1+1)->x,(p3d1+1)->y,p3d2->x,p3d2->y,(p3d2+1)->x,(p3d2+1)->y,&Xi,&Yi))  
         { bcdtmWrite_message(1,0,0,"Knot In String") ; return(1) ; }
      }
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmMath_validatePointArrayPolygon()                                      |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_validatePointArrayPolygon(DPoint3d **Polygon,long *NumPolyPoints,double Pptol)
/*
** This Function Validates a Polygon
*/
{
 DTMDirection   Direction;
 long IntFlag ;
 double Area ;
/*
** Check Last Point is equal to First Point
*/
 if( (*Polygon)->x != (*Polygon+*NumPolyPoints-1)->x  ||
     (*Polygon)->y != (*Polygon+*NumPolyPoints-1)->y     )
   { bcdtmWrite_message(1,0,0,"Polygon Does Not Close") ; return(1) ; }
/*
** Eliminate Duplicate Points
*/
 bcdtmMath_deleteDuplicatePointsP3D(Polygon,NumPolyPoints,Pptol) ;
/*
** Check For Knots
*/
 bcdtmData_checkPolygonForKnots(*Polygon,*NumPolyPoints,&IntFlag) ;
 if( IntFlag ) { bcdtmWrite_message(1,0,0,"Knot Detected In Polygon") ; return(1) ; }
/*
** Get Polygon Direction
*/
 bcdtmMath_getPolygonDirectionP3D(*Polygon,*NumPolyPoints,&Direction,&Area) ;
/*
** Reverse Direction Of Polygon If Clockwise
*/
 if (Direction == DTMDirection::Clockwise) bcdtmMath_reversePolygonDirectionP3D (*Polygon, *NumPolyPoints);
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
BENTLEYDTM_Public int bcdtmMath_deleteDuplicatePointsP3D(DPoint3d **Points,long *numPts,double Pptol)
{
 long   flag ;
 double dx,dy ;
 DPoint3d   *p3d1,*p3d2   ;
/*
** Initialise Variables
*/
 if( Pptol < 0.0 ) Pptol = -Pptol ;
/*
** Eliminate Points
*/
 for( p3d1 = *Points, p3d2 = p3d1 + 1 ; p3d2 < *Points + *numPts ; ++p3d2 )
   {
    dx = p3d2->x-p3d1->x ; 
    dy = p3d2->y-p3d1->y ;
    flag = 0 ;
    if     ( Pptol > 0.0 ) { if( sqrt(dx*dx + dy*dy) > Pptol ) flag = 1 ; }
    else if( dx != 0.0 || dy != 0.0 ) flag = 1 ;
    if( flag ) { ++p3d1 ; if( p3d1 != p3d2 ) *p3d1 = *p3d2 ; }
   }
 *numPts = (long) (p3d1 - *Points) + 1 ;
/*
** Reallocate Memory
*/
 *Points = ( DPoint3d * ) realloc(*Points,*numPts * sizeof(DPoint3d)) ;
 if( *Points == NULL )
   { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return(1) ; }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmMath_deletePolygonKnotsP3D                                     |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_deletePolygonKnotsP3D(DPoint3d **Polygon,long *numPts,long *IntFlag,double Pptol )
/*
** This Function Detects and Deletes Crossing Polygon Lines
*/
{
 long   process ;
 DPoint3d   *p1,*p2,*p3,*p4,*pSp,*pEp ;
 double d1,d2,X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,X4,Y4,Z4,Xc,Yc,Zc ;
/*
** Initialise Variables
*/
 *IntFlag = 0 ;
/*
** Delete Duplicate Points
*/
 if( bcdtmMath_deleteDuplicatePointsP3D(Polygon,numPts,Pptol)) return(1) ;
/*
** Check Polygon Lines Dont Cross
*/
 process = 1 ;
 while ( process )
   {
    process = 0 ;
    pSp = *Polygon ;
    pEp = *Polygon + *numPts - 1  ;
    for ( p1 = pSp ; p1 < pEp - 1 ; ++p1 )
      {
       p2 = p1 + 1 ;
       X1 = p1->x ; Y1 = p1->y ; Z1 = p1->z ;
       X2 = p2->x ; Y2 = p2->y ; Z2 = p2->z ;
       for ( p3 = pSp ; p3 < pEp - 1 ; ++p3 )
         {
          p4 = p3 + 1 ;
          if( p4 == pEp - 1 ) p4 = p1 ;
          if( p3 != p1 && p3 != p2 && p4 != p1 )
            {
             X3 = p3->x ; Y3 = p3->y ; Z3 = p3->z ;
             X4 = p4->x ; Y4 = p4->y ; Z4 = p4->z ;
             if(bcdtmMath_intersectCordLines(X1,Y1,X2,Y2,X3,Y3,X4,Y4,&Xc,&Yc))
               {
                ++*IntFlag  ; process = 1 ;
                d1 = sqrt((X1-X2)*(X1-X2)+(Y1-Y2)*(Y1-Y2)) ;
                d2 = sqrt((Xc-X2)*(Xc-X2)+(Yc-Y2)*(Yc-Y2)) ;
                Z1 = Z2 + ( Z1 - Z2 ) * d2 /d1 ;
                d1 = sqrt((X3-X4)*(X3-X4)+(Y3-Y4)*(Y3-Y4)) ;
                d2 = sqrt((Xc-X4)*(Xc-X4)+(Yc-Y4)*(Yc-Y4)) ;
                Z3 = Z4 + ( Z3 - Z4 ) * d2 /d1 ;
                Zc = ( Z1 + Z2 ) / 2.0 ;
                if( bcdtmMath_deleteKnotP3D(p2,p3,p4,Xc,Yc,Zc,&*Polygon,numPts,Pptol)) return(1) ;
                p1 = p3 = pEp = *Polygon + *numPts - 1 ;
               }
            }
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
BENTLEYDTM_Public int bcdtmMath_deleteKnotP3D(DPoint3d *P2,DPoint3d *P3,DPoint3d *P4,double Xc,double Yc,double Zc, DPoint3d **Points,long *numPts,double Pptol)
{
 DPoint3d *p1,*p2 ;
/*
** Add New Point to Replace P2
*/
 P2->x = Xc ; P2->y = Yc ; P3->z = Zc ;
/*
** Delete and Copy Over Points
*/
 p1 = p2 = P2 + 1 ;
 while( p2 != P4 ) ++p2 ;
 while( p2 < *Points + *numPts ) { *p1 = *p2 ; ++p1 ; ++p2 ; }
 *numPts = (long) ( p1 - *Points ) ;
/*
** Reallocate Memory
*/
 *Points = ( DPoint3d * ) realloc(*Points,*numPts * sizeof(DPoint3d)) ;
 if( *Points == NULL )
   { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return(1) ; }
/*
** Delete Duplicate Points
*/
 bcdtmMath_deleteDuplicatePointsP3D(&*Points,numPts,Pptol) ;
/*
** Job Completed
*/
 return(0) ;
}

#ifdef OLD
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT double bcdtmMath_distanceOfPointFromLine(long *LineFlag, double X1,double Y1,double X2,double Y2,double X3,double Y3,double *X4, double *Y4)
{
 double ang1,ang2,ang3,dist1,dist2,dist3 ;
 double xmin,ymin,xmax,ymax,xlmin,ylmin,xlmax,ylmax ;
/*
** Initialise Line Extents
*/
 if( X1 <= X2 ) { xlmin = X1 ; xlmax = X2 ; }
 else           { xlmin = X2 ; xlmax = X1 ; }
 if( Y1 <= Y2 ) { ylmin = Y1 ; ylmax = Y2 ; }
 else           { ylmin = Y2 ; ylmax = Y1 ; }
/*
** Normalise Coordinates
*/
 xmin = xmax = X1 ;
 ymin = ymax = Y1 ;
 if( X2 < xmin ) xmin = X2 ;
 if( X2 > xmax ) xmax = X2 ;
 if( X3 < xmin ) xmin = X3 ;
 if( X3 > xmax ) xmax = X3 ;
 if( Y2 < ymin ) ymin = Y2 ;
 if( Y2 > ymax ) ymax = Y2 ;
 if( Y3 < ymin ) ymin = Y3 ;
 if( Y3 > ymax ) ymax = Y3 ;
 X1 = X1 - xmin ;
 X2 = X2 - xmin ;
 X3 = X3 - xmin ;
 Y1 = Y1 - ymin ;
 Y2 = Y2 - ymin ;
 Y3 = Y3 - ymin ;
/*
** Calculate Distance Of Point From Line
*/
 if( Y2-Y1 == 0.0 && X2-X1 == 0 ) { *LineFlag = 1 ; *X4 = X1 + xmin ; *Y4 = Y1 + ymin ; return(0.0) ; }
 ang1 = atan2(Y2-Y1,X2-X1) ;
 if( Y3-Y1 == 0.0 && X3-X1 == 0 ) { *LineFlag = 1 ; *X4 = X1 + xmin ; *Y4 = Y1 + ymin ; return(0.0) ; }
 ang2 = atan2(Y3-Y1,X3-X1) ;
 if ( ang1 <   0.0 ) ang1 += DTM_2PYE ;
 if ( ang2 <   0.0 ) ang2 += DTM_2PYE ;
 if ( ang1 >= ang2 ) ang3 = ang1 - ang2 ;
 else                ang3 = ang2 - ang1 ;
 if ( ang3 > DTM_PYE ) ang3 = DTM_2PYE - ang3 ;
 dist1 = sin(ang3) * sqrt((Y3-Y1)*(Y3-Y1) + (X3-X1)*(X3-X1)) ;
/*
** Calculate Point X4,Y4 On Line ((X1,Y1) (X2,Y2)) Normal to X3,Y3
*/
 dist2 = cos(ang3) * sqrt((Y3-Y1)*(Y3-Y1) + (X3-X1)*(X3-X1)) ;
 dist3 = sqrt((X1-X2)*(X1-X2) + (Y1-Y2)*(Y1-Y2)) ;
 *X4   = X1 + ( X2-X1 ) * dist2 / dist3 + xmin ;
 *Y4   = Y1 + ( Y2-Y1 ) * dist2 / dist3 + ymin ;
/*
** Adjust X4 & Y4 for Line || to Xaxis or Yaxis
*/
 if( X1 == X2 ) *X4 = X1 + xmin ;
 if( Y1 == Y2 ) *Y4 = Y1 + ymin ;
/*
** Test if X4 && Y4 are between Line EndPoints
*/
 *LineFlag = 1 ;
 if( *X4 < xlmin || *X4 > xlmax || *Y4 < ylmin || *Y4 > ylmax ) *LineFlag = 0 ;
/*
** Job Completed
*/
 return(dist1) ;
}

#else

BENTLEYDTM_EXPORT double bcdtmMath_distanceOfPointFromLineSquared (long *lineFlag, double X1, double Y1, double X2, double Y2, double X3, double Y3, double *X4, double *Y4)
    {
    double lx = X2 - X1;
    double ly = Y2 - Y1;

    double denominator = (lx * lx) + (ly * ly);

    if (denominator != 0)
        {
        double px = X3 - X1;
        double py = Y3 - Y1;
        double numerator = (px * lx) + (py * ly);
        double faction = (numerator / denominator);
        *lineFlag = faction >= 0 && faction <= 1 ? 1 : 0;
        *X4 = X1 + (lx * faction);
        *Y4 = Y1 + (ly * faction);
        }
    else
        {
        *lineFlag = 1;
        *X4 = X3;
        *Y4 = Y3;
        }
    return ((*Y4 - Y3)*(*Y4 - Y3) + (*X4 - X3)*(*X4 - X3));
    }

BENTLEYDTM_EXPORT double bcdtmMath_distanceOfPointFromLine (long *lineFlag, double X1, double Y1, double X2, double Y2, double X3, double Y3, double *X4, double *Y4)
    {
    return sqrt (bcdtmMath_distanceOfPointFromLineSquared (lineFlag, X1, Y1, X2, Y2, X3, Y3, X4, Y4));
    }
#endif
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmMath_getTriangleAttributes(DPoint3d TrgPts[],double *SlopeDegrees,double *SlopePercent,double *Aspect,double *Height)
/*
** This Function Gets Triangle Attributes
*/
{
 double  X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3 ;
 double  Sd,Sp,As,Ht,axy,c,ca,cb,cc,cd ;
/*
** Get Triangle Attributes
*/
 X1 = TrgPts[0].x ; Y1 = TrgPts[0].y ; Z1 = TrgPts[0].z ;
 X2 = TrgPts[1].x ; Y2 = TrgPts[1].y ; Z2 = TrgPts[1].z ;
 X3 = TrgPts[2].x ; Y3 = TrgPts[2].y ; Z3 = TrgPts[2].z ;
 if( Z1 == Z2 && Z2 == Z3 )
   {
    *SlopeDegrees = *SlopePercent = *Aspect = 360.0 ; *Height = Z1 ;
    return(0) ;
   }
/*
** Calculate Plane Coefficients
*/
 bcdtmMath_calculatePlaneCoefficients(X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,&ca,&cb,&cc,&cd) ;
/*
** Get Triangle Height
*/
 Ht = (Z1 + Z2 + Z3) / 3.0 ;
/*
** Calulate Triangle Slope
*/
 c = sqrt( ca*ca + cb*cb + cc*cc ) ;
 if( c == 0.0 ) c = 0.0000000001 ;
 Sd = acos(fabs(cc/c)) ;
 Sp = tan(Sd)   * 100.0 ;
 Sd = Sd / DTM_2PYE * 360.0 ;
/*
** Get Angle Of Normal Projected On XY Plane
*/
 axy = atan2(cb,ca) ;
 if( axy < 0.0 ) axy += DTM_2PYE ;
/*
** Get Aspect Value
*/
 if( cc  >= 0.0 ) As = axy ;
 else             As = axy + DTM_2PYE /2.0 ;
 if( As > DTM_2PYE ) As = As - DTM_2PYE ;
/*
** Convert Aspect To Bearing Radians Anti Clockwise From North Axis
*/
 As = ( DTM_PYE / 2.0 - As ) ;
 if( As < 0.0 ) As = DTM_2PYE + As ;
/*
** Convert Aspect Radians To Degrees
*/
 As = As / DTM_2PYE * 360.0 ;
/*
** Set Attributes
*/
 *SlopeDegrees = Sd ;
 *SlopePercent = Sp ;
 *Aspect       = As ;
 *Height       = Ht ;
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
BENTLEYDTM_Public int bcdtmMath_getTriangleAttributesJay(DPoint3d TrgPts[],double *SlopeDegrees,double *SlopePercent,double *Aspect,double *Height)
/*
** This Function Gets Triangle Attributes
*/
{
 double  X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3 ;
 double  Sd,Sp,As,Ht,axy,c,d,ca,cb,cc,cd ;
/*
** Get Triangle Attributes
*/
 X1 = TrgPts[0].x ; Y1 = TrgPts[0].y ; Z1 = TrgPts[0].z ;
 X2 = TrgPts[1].x ; Y2 = TrgPts[1].y ; Z2 = TrgPts[1].z ;
 X3 = TrgPts[2].x ; Y3 = TrgPts[2].y ; Z3 = TrgPts[2].z ;
 if( Z1 == Z2 && Z2 == Z3 )
   {
    Sd = Sp = As = -1.0 ; Ht = Z1 ;
    *SlopeDegrees = *SlopePercent = *Aspect = -1.0 ; *Height = Z1 ;
    return(0) ;
   }
/*
** Calculate Plane Coefficients
*/
 bcdtmMath_calculatePlaneCoefficients(X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,&ca,&cb,&cc,&cd) ;
 if( cd == 0.0 )  cd = 0.0000000001 ;
 /* jay removed */
 /* ca = ca / cd ; cb = cb / cd ; cc = cc / cd ; */
 d  = 1.0 / sqrt( ca*ca + cb*cb + cc*cc ) ;
 c  = cc * d ;
/*
** Get Triangle Height
*/
 Ht = (Z1 + Z2 + Z3) / 3.0 ;
/*
** Calulate Triangle Slope
*/
 Sd = acos(fabs(c))   ;
 /* jay added */
 Sd = Sd / DTM_2PYE * 360.0 ;
 Sp = tan(Sd) * 100.0 ;
/*
** Get Angle Of Normal Projected On XY Plane
*/
 axy = atan2(cb,ca) ;
 if( axy < 0.0 ) axy += DTM_2PYE ;
/*
** Get Aspect Value
*/
 if( c  >= 0.0 ) As = axy ;
 else            As = axy + DTM_2PYE /2.0 ;
 if( As > DTM_2PYE ) As = As - DTM_2PYE ;
/*
** Convert Aspect To Bearing Radians Anti Clockwise From North Axis
*/
 As = ( DTM_PYE / 2.0 - As ) ;
 if( As < 0.0 ) As = DTM_2PYE + As ;
/*
** Convert Aspect Radians To Degrees
*/
 As = As / DTM_2PYE * 360.0 ;
/*
** Set Attributes
*/
 *SlopeDegrees = Sd ;
 *SlopePercent = Sp ;
 *Aspect       = As ;
 *Height       = Ht ;
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
BENTLEYDTM_Public int bcdtmMath_getLatticeAttributes(DPoint3d LatPts[],double *Sd,double *Sp,double *Bd, double *Ht)
/*
**
** This Routine Calculates The Attributes For A Lattice Cell
** By Averaging The Four Triangles That Cover A Cell
**
*/
{
 long   i1,i2,i3 ;
 DPoint3d    TrgPts[3] ;
 double H,Spp,Spd,A ;
/*
** Initialise Variables
*/
 *Sd = *Sp = *Bd = *Ht = 0.0 ;
/*
** Calculate  Attributes For the Four Triangles In the Cell
*/
 for( i1 = 0 ; i1 < 4 ; ++i1 )
   {
    i2 = (i1 + 1)  % 4 ; i3 = (i1 + 2 ) % 4 ;
    TrgPts[0].x = LatPts[i1].x ; TrgPts[0].y = LatPts[i1].y ; TrgPts[0].z = LatPts[i1].z ;
    TrgPts[1].x = LatPts[i2].x ; TrgPts[1].y = LatPts[i2].y ; TrgPts[1].z = LatPts[i2].z ;
    TrgPts[2].x = LatPts[i3].x ; TrgPts[2].y = LatPts[i3].y ; TrgPts[2].z = LatPts[i3].z ;
    bcdtmMath_getTriangleAttributes(TrgPts,&Spd,&Spp,&A,&H) ;
    *Sd = *Sd + Spd ;
    *Sp = *Sp + Spp ;
    *Bd = *Bd + A   ;
    *Ht = *Ht + H   ;
   }
 *Sd = *Sd / 4.0 ;
 *Sp = *Sp / 4.0 ;
 *Bd = *Bd / 4.0 ;
 *Ht = *Ht / 4.0 ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmMath_interpolatePoly()                                         |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_interpolatePoly(long NewTrg,double xp,double yp,double *zp,double x[],double y[],double z[],double pd[] )
/*
**  This routine interpolates the z value for a point
**  within a triangle from a bivarate polynominal
*/
{
 long   i,jpd ;
 double a,b,c,d,dlt,dx,dy,u,v ;
 static thread_local double x0, y0, ap, bp, cp, dp;
 double act2,adbc,bdt2,lu,lv,thxu,thuv ;
 double aa,bb,cc,dd,ab,cd,ac,ad,bc,g1,g2,h1,h2,h3  ;
 double zu[3],zuu[3],zv[3],zvv[3],zuv[3] ;
 double p0,p1,p2,p3,p4,p5 ;
 static thread_local double p00,p01,p02,p03,p04,p05 ;
 static thread_local double p10,p11,p12,p13,p14 ;
 static thread_local double p20,p21,p22,p23 ;
 static thread_local double p30,p31,p32 ;
 static thread_local double p40,p41 ;
 static thread_local double p50 ;
 double csuv,thus,thsv ;
/*
** Determine the Coefficients for the Coordinate System
** Transformation from the x-y system to the U-V system
*/
  if( NewTrg )
    {
     x0 = x[0] ;
     y0 = y[0] ;
     a = x[1] - x0 ;
     b = x[2] - x0 ;
     c = y[1] - y0 ;
     d = y[2] - y0 ;
     ad = a * d ;
     bc = b * c ;
     dlt = ad - bc ;
     ap =  d / dlt ;
     bp = -b / dlt ;
     cp = -c / dlt ;
     dp =  a / dlt ;
/*
** Convert the Partial Derivatives at the vertices of the triangle
** for the U-V coordinate system
*/
     aa = a*a ;
     act2 = 2.0*a*c ;
     cc = c*c ;
     ab = a*b ;
     adbc = ad+bc ;
     cd = c*d ;
     bb = b*b ;
     bdt2 = 2.0*b*d ;
     dd = d*d ;
     for( i = 0 ; i < 3 ; ++i )
       {
    jpd = 5 * (i+1) - 1 ;
    zu[i]  = a*pd[jpd-4]  + c*pd[jpd-3] ;
    zv[i]  = b*pd[jpd-4]  + d*pd[jpd-3] ;
    zuu[i] = aa*pd[jpd-2] + act2*pd[jpd-1] + cc*pd[jpd] ;
    zuv[i] = ab*pd[jpd-2] + adbc*pd[jpd-1] + cd*pd[jpd] ;
    zvv[i] = bb*pd[jpd-2] + bdt2*pd[jpd-1] + dd*pd[jpd] ;
       }
/*
** Calculate the coefficients of the polynominal
*/
     p00 = z[0] ;
     p10 = zu[0] ;
     p01 = zv[0] ;
     p20 = 0.5*zuu[0] ;
     p11 = zuv[0] ;
     p02 = 0.5*zvv[0] ;
     h1 = z[1] - p00 - p10 - p20 ;
     h2 = zu[1] - p10 - zuu[0] ;
     h3 = zuu[1] - zuu[0] ;
     p30 =  10.0*h1 - 4.0*h2 + 0.5*h3 ;
     p40 = -15.0*h1 + 7.0*h2 -     h3 ;
     p50 =   6.0*h1 - 3.0*h2 + 0.5*h3 ;
     h1 = z[2] - p00 - p01 - p02 ;
     h2 = zv[2] - p01 - zvv[0]   ;
     h3 = zvv[2] - zvv[0] ;
     p03 =  10.0*h1 - 4.0*h2 + 0.5*h3 ;
     p04 = -15.0*h1 + 7.0*h2 -     h3 ;
     p05 =   6.0*h1 - 3.0*h2 + 0.5*h3 ;
     lu=sqrt(aa+cc) ;
     lv=sqrt(bb+dd) ;
     thxu=atan2(c,a) ;
     thuv=atan2(d,b) - thxu ;
     csuv = cos(thuv) ;
     p41 = 5.0*lv*csuv/lu*p50 ;
     p14 = 5.0*lu*csuv/lv*p05 ;
     h1 = zv[1] - p01 - p11 - p41 ;
     h2 = zuv[1] - p11 - 4.0*p41  ;
     p21 =  3.0*h1-h2 ;
     p31 = -2.0*h1+h2 ;
     h1 = zu[2] - p10 - p11 - p14 ;
     h2 = zuv[2] - p11 - 4.0*p14  ;
     p12 =  3.0*h1-h2 ;
     p13 = -2.0*h1+h2 ;
     thus = atan2(d-c,b-a) - thxu ;
     thsv = thuv - thus ;
     aa =  sin(thsv) / lu ;
     bb = -cos(thsv) / lu ;
     cc =  sin(thus) / lv ;
     dd =  cos(thus) / lv ;
     ac = aa * cc ;
     ad = aa * dd ;
     bc = bb * cc ;
     g1 = aa * ac * ( 3.0*bc + 2.0*ad) ;
     g2 = cc * ac * ( 3.0*ad + 2.0*bc) ;
     h1 = -aa*aa*aa*(5.0*aa*bb*p50+(4.0*bc+ad)*p41) -
       cc*cc*cc*(5.0*cc*dd*p05+(4.0*ad+bc)*p14) ;
     h2 = 0.5 * zvv[1] - p02 - p12 ;
     h3 = 0.5 * zuu[2] - p20 - p21 ;
     p22 = (g1*h2 + g2*h3 - h1) / ( g1 + g2 ) ;
     p32 = h2 - p22 ;
     p23 = h3 - p22 ;
    }
/*
** Convert x & y to U-V system
*/
  dx = xp - x0 ;
  dy = yp - y0 ;
  u = ap * dx + bp * dy ;
  v = cp * dx + dp * dy ;
/*
** Evaluate the Polynominal
*/
  p0 = p00+v*(p01+v*(p02+v*(p03+v*(p04+v*p05)))) ;
  p1 = p10+v*(p11+v*(p12+v*(p13+v*p14)))  ;
  p2 = p20+v*(p21+v*(p22+v*p23)) ;
  p3 = p30+v*(p31+v*p32) ;
  p4 = p40+v*p41 ;
  p5 = p50 ;
  *zp = p0+u*(p1+u*(p2+u*(p3+u*(p4+u*p5)))) ;
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
BENTLEYDTM_Public int bcdtmMath_getStringMaxMinP3D(long SetFlag,DPoint3d *StringPts,long NumStringPts,double *Xmin,double *Xmax,double *Ymin,double *Ymax,double *Zmin,double *Zmax )
/*
** This Function Returns the Maximum and Minimum Coordinate Values
** in The String
*/
{
 DPoint3d *p3d ;
/*
** Set Initial Values
*/
 if( SetFlag )
   {
    *Xmin = *Xmax = StringPts->x ;
    *Ymin = *Ymax = StringPts->y ;
    *Zmin = *Zmax = StringPts->z ;
   }
/*
** Scan String For Maxima and Minima Values
*/
 for( p3d = StringPts ; p3d < StringPts + NumStringPts ; ++p3d )
   {
    if( p3d->x < *Xmin ) *Xmin = p3d->x ;
    if( p3d->x > *Xmax ) *Xmax = p3d->x ;
    if( p3d->y < *Ymin ) *Ymin = p3d->y ;
    if( p3d->y > *Ymax ) *Ymax = p3d->y ;
    if( p3d->z < *Zmin ) *Zmin = p3d->z ;
    if( p3d->z > *Zmax ) *Zmax = p3d->z ;
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
BENTLEYDTM_Public double bcdtmMath_coordinateTriangleArea(double X1,double Y1,double X2,double Y2,double X3,double Y3)
{
 double Area ;
 Area = (X1*Y2 + Y1*X3 + Y3*X2 - Y2*X3 - Y1*X2 - X1*Y3) / 2.0 ;
 if( Area < 0.0 ) Area =- Area ;
 return(Area) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double  bcdtmMath_calculateTriangleAreaToPerimeterRatio(double X1,double Y1,double X2,double Y2,double X3,double Y3)
/*
** This Function Calculates the Ratio of Triangle's Area To Perimeter
*/
{
 double perimeter,area ;
/*
** Initialise
*/
 area = 0.5 * (X1*Y2 + Y1*X3 + Y3*X2 - Y2*X3 - Y1*X2 - X1*Y3) ;
 if( area < 0.0 ) area = -area ;
 perimeter  = sqrt ( (X1-X2) * (X1-X2) + (Y1-Y2) * (Y1-Y2)) ;
 perimeter += sqrt ( (X2-X3) * (X2-X3) + (Y2-Y3) * (Y2-Y3)) ;
 perimeter += sqrt ( (X3-X1) * (X3-X1) + (Y3-Y1) * (Y3-Y1)) ;
/*
** Calculate Ratio
*/
 return( area/perimeter ) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmMath_strokeArc()                                               |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmMath_strokeArc(double Sx,double Sy,double Ex,double Ey,double Cx,double Cy,double A1,double A2,double Ap,double As,double Ra,double Tolerance,DPoint3d **ArcPts,long *NumArcPts)
/*
** This Function Strokes An Arc In a manner that ensures the
** maximum distance of the midpoint of two consecutive stroked vertices is less than
** or equal to the tolerance from the arc.
**
**
**   Tin     = Tin Object
**   Sx,Sy   = Start Cordinates Of Arc
**   Ex,Ey   = End Coordinates Of Arc
**   Cx,Cy   = Centre Coordinates Of Arc
**   A1,A2   = Start Angle And Sweep Angle
**   Ap,As   = Primary And Secondary Axis Of Arc
**   Ra      = Rotation Angle
**
**   Author: Rob Cormack
**   Date  : 23 March 2002
**
*/
{
 long   dbg=DTM_TRACE_VALUE(0),process,ArcDirection,ReverseDirection=0,MemArcPts=0,MemArcInc=1000 ;
 double A3,Aa,Rs,Px,Py,Tx,Ty,Nx,Ny,Rx,Ry,Nd,Lrs,Trs   ;
 DPoint3d    *p3d ;
 double x,y,StartAngle,EndAngle,d2Arc,minD2Arc=0.0,maxD2Arc=0.0 ; 
 DPoint3d    *p3d1,*p3d2, P3d ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Stroking ARC") ;
    bcdtmWrite_message(0,0,0,"Sx           = %12.4lf",Sx) ;
    bcdtmWrite_message(0,0,0,"Sy           = %12.4lf",Sy) ;
    bcdtmWrite_message(0,0,0,"Ex           = %12.4lf",Ex) ;
    bcdtmWrite_message(0,0,0,"Ey           = %12.4lf",Ey) ;
    bcdtmWrite_message(0,0,0,"Cx           = %12.4lf",Cx) ;
    bcdtmWrite_message(0,0,0,"Cy           = %12.4lf",Cy) ;
    bcdtmWrite_message(0,0,0,"A1           = %12.4lf",A1) ;
    bcdtmWrite_message(0,0,0,"A2           = %12.4lf",A2) ;
    bcdtmWrite_message(0,0,0,"Ap           = %12.4lf",Ap) ;
    bcdtmWrite_message(0,0,0,"As           = %12.4lf",As) ;
    bcdtmWrite_message(0,0,0,"Ra           = %12.4lf",Ra) ;
    bcdtmWrite_message(0,0,0,"Tolerance    = %12.6lf",Tolerance) ;
    bcdtmWrite_message(0,0,0,"ArcPoints    = %p",*ArcPts) ;
    bcdtmWrite_message(0,0,0,"NumArcPoints = %4ld",*NumArcPts) ;
   }
/*
** Initialise Variables
*/
 if( Tolerance <= 0.0 ) Tolerance = 0.0001 ;
 if( *ArcPts != NULL ) { free(*ArcPts) ; *ArcPts = NULL ; }
 *NumArcPts = 0    ;
 StartAngle = bcdtmMath_getAngle(Cx,Cy,Sx,Sy) ;
 EndAngle   = bcdtmMath_getAngle(Cx,Cy,Ex,Ey) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Start Angle = %12.10lf",StartAngle) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"End   Angle = %12.10lf",EndAngle) ;
/*
** Check To Reverse Direction Of ARC So That Arcs Are all stroked in the same direction.
** This ensures an ARC common to two shapes has identical stroke points
*/
 ReverseDirection = 0 ;
 if( Ex < Sx || ( Ex == Sx && Ey < Sy ) )
   {
    x  = Ex ; y =  Ey ;
    Ex = Sx ; Ey = Sy ;
    Sx = x  ; Sy = y ;
    A2 = -A2 ;
    Ra = -Ra ;
    A1 = EndAngle - Ra ;
    ReverseDirection = 1 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Arc Direction Reversed") ;
   }

/*
** Initialise Arc  Variables
*/
 Ap = Ap * 2.0   ;
 As = As * 2.0   ;
/*
** Set Translation Constants
*/
 Tx =  Cx - 0.0  ;
 Ty =  Cy - 0.0  ;
/*
** Check Direction Of Arc Parameters
*/
 ArcDirection = 0 ;
 bcdtmInterpolate_getPointOnArc(&Px,&Py,Ap,As,A1) ;
 bcdtmInterpolate_rotateAndTranslatePoint(Px,Py,Tx,Ty,Ra,&Nx,&Ny) ;
 if( bcdtmMath_distance(Nx,Ny,Sx,Sy) > 0.001 ) ArcDirection = 1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"ArcDirection =%2ld",ArcDirection) ;
/*
 bcdtmWrite_message(0,0,0,"** Sx = %10.4lf Sy = %10.4lf ** Nx = %10.4lf Ny = %10.4lf",Sx,Sy,Nx,Ny) ;
 bcdtmInterpolate_getPointOnArc(&Px,&Py,Ap,As,A1+A2) ;
 bcdtmInterpolate_rotateAndTranslatePoint(Px,Py,Tx,Ty,Ra,&Nx,&Ny) ;
 bcdtmWrite_message(0,0,0,"** Ex = %10.4lf Ey = %10.4lf ** Nx = %10.4lf Ny = %10.4lf",Ex,Ey,Nx,Ny) ;
 bcdtmInterpolate_getPointOnArc(&Px,&Py,Ap,As,A1) ;
 bcdtmInterpolate_rotateAndTranslatePoint(Px,Py,Tx,Ty,-Ra,&Nx,&Ny) ;
 bcdtmWrite_message(0,0,0,"** Sx = %10.4lf Sy = %10.4lf ** Nx = %10.4lf Ny = %10.4lf",Sx,Sy,Nx,Ny) ;
 bcdtmInterpolate_getPointOnArc(&Px,&Py,Ap,As,A1-A2) ;
 bcdtmInterpolate_rotateAndTranslatePoint(Px,Py,Tx,Ty,-Ra,&Nx,&Ny) ;
 bcdtmWrite_message(0,0,0,"** Ex = %10.4lf Ey = %10.4lf ** Nx = %10.4lf Ny = %10.4lf",Ex,Ey,Nx,Ny) ;
*/
/*
** Negate Arc Parameters If Necessary
*/
 if( ArcDirection )  { Ra = -Ra ; A2 = -A2 ; }
/*
** Determine Angular Stroking Tolerance
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determining Angular Stroking Tolerance") ;
 bcdtmInterpolate_getPointOnArc(&Px,&Py,Ap,As,A1) ;
 bcdtmInterpolate_rotateAndTranslatePoint(Px,Py,Tx,Ty,Ra,&Rx,&Ry) ;
 Lrs = A1 ;
 Trs = A1 + A2  ;
 Rs  = (Lrs + Trs ) / 2.0 ;
 process = 1 ;
 while( process )
   {
    process = 0 ;
    Aa = Rs ;
    bcdtmInterpolate_getPointOnArc(&Px,&Py,Ap,As,Aa) ;
    bcdtmInterpolate_rotateAndTranslatePoint(Px,Py,Tx,Ty,Ra,&Nx,&Ny) ;
    Nx = ( Rx + Nx ) / 2.0 ;
    Ny = ( Ry + Ny ) / 2.0 ;
    bcdtmInterpolate_translateAndRotatePoint(Nx,Ny,-Tx,-Ty,-Ra,&Px,&Py) ;
    Aa = bcdtmMath_getAngle(0.0,0.0,Px,Py) ;
    bcdtmInterpolate_getPointOnArc(&Nx,&Ny,Ap,As,Aa) ;
    Nd = bcdtmMath_distance(Px,Py,Nx,Ny) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Nd = %15.10lf Rs = %15.10lf",Nd,Rs) ;
    if( Nd > Tolerance || ( Nd < Tolerance && ( Tolerance - Nd ) > Tolerance / 100.0 ))
      {
       if( Nd > Tolerance  )  Trs =  Rs   ;
       if( Nd < Tolerance  )  Lrs =  Rs   ;
       Rs = ( Trs + Lrs ) / 2.0 ;
       process = 1 ;
      }
   }
 Rs = Rs - A1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Angular Stroking Tolerance For Arc = %10.8lf",Rs) ;
/*
** Allocate Initial Memory For Arc Points
*/
 MemArcPts = MemArcPts + MemArcInc ;
 *ArcPts = (DPoint3d *) malloc(MemArcPts * sizeof(DPoint3d)) ;
 if( *ArcPts == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Store First Point
*/
 (*ArcPts)->x = Sx  ;
 (*ArcPts)->y = Sy  ;
 (*ArcPts)->z = 0.0 ;
 ++*NumArcPts ;
/*
** Stroke Arc
*/
 Aa = A1 ;
 A3 = A1 +  A2  ;
 while ( Aa != A3  )
   {
/*
**  Get Coordinates Of Next Arc Stoke Point
*/
    Aa = Aa + Rs ;
    if(( Aa < A1 || Aa > A3 ) && ( Aa > A1 || Aa < A3 ) ) Aa = A3 ;
    bcdtmInterpolate_getPointOnArc(&Px,&Py,Ap,As,Aa) ;
    bcdtmInterpolate_rotateAndTranslatePoint(Px,Py,Tx,Ty,Ra,&Nx,&Ny) ;
/*
**  Store Stroked Arc Point
*/
    if( *NumArcPts == MemArcPts )
      {
       MemArcPts = MemArcPts + MemArcInc ;
       if( *ArcPts == NULL ) *ArcPts = (DPoint3d *) malloc(MemArcPts * sizeof(DPoint3d)) ;
       else                  *ArcPts = (DPoint3d *) realloc(*ArcPts,MemArcPts * sizeof(DPoint3d)) ;
       if( *ArcPts == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
    (*ArcPts+*NumArcPts)->x = Nx  ;
    (*ArcPts+*NumArcPts)->y = Ny  ;
    (*ArcPts+*NumArcPts)->z = 0.0 ;
    ++*NumArcPts ;
   }
/*
** Set Last calculated Arc Point Equal To Last Arc Point
*/
 (*ArcPts+*NumArcPts-1)->x = Ex  ;
 (*ArcPts+*NumArcPts-1)->y = Ey  ;
 (*ArcPts+*NumArcPts-1)->z = 0.0 ;
/*
** Check Distance Of Mid Point Between Stroked Vertices To Arc
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Distance Of Vertice Midpoints To Arc") ;
    for( p3d = *ArcPts ; p3d < *ArcPts + *NumArcPts - 1 ; ++p3d )
      {
       Rx = (p3d->x + (p3d+1)->x) / 2.0 ;
       Ry = (p3d->y + (p3d+1)->y) / 2.0 ;
       bcdtmInterpolate_translateAndRotatePoint(Rx,Ry,-Tx,-Ty,-Ra,&Nx,&Ny) ;
       Aa = bcdtmMath_getAngle(0.0,0.0,Nx,Ny) ;
       bcdtmInterpolate_getPointOnArc(&Px,&Py,Ap,As,Aa) ;
       d2Arc = bcdtmMath_distance(Nx,Ny,Px,Py) ;
       bcdtmWrite_message(0,0,0,"Mid Point %10.4lf %10.4lf Distance To Arc = %12.10lf",Rx,Ry,d2Arc) ;
       if( p3d == *ArcPts ) minD2Arc = maxD2Arc = d2Arc ;
       else
         {
          if( d2Arc < minD2Arc ) minD2Arc = d2Arc ;
          if( d2Arc > maxD2Arc ) maxD2Arc = d2Arc ;
         }
      }
    bcdtmWrite_message(0,0,0,"Minimum Distance Of Mid Points To Arc = %10.6lf",minD2Arc) ;
    bcdtmWrite_message(0,0,0,"Maximum Distance Of Mid Points To Arc = %10.6lf",maxD2Arc) ;
   }
/*
** Reverse Coordinates
*/
 if( ReverseDirection )
   {
    p3d  = &P3d ;
    p3d1 = *ArcPts ;
    p3d2 = *ArcPts + *NumArcPts - 1 ;
    while ( p3d1 < p3d2 )
      {
       *p3d  = *p3d1 ;
       *p3d1 = *p3d2 ;
       *p3d2 = *p3d  ;
       ++p3d1 ;
       --p3d2 ;
      }
   }
/*
** Write Out Arc Stroke Points
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Stroked Arc Points = %6ld",*NumArcPts) ;
    for( p3d = *ArcPts ; p3d < *ArcPts + *NumArcPts ; ++p3d )
      {
       bcdtmWrite_message(0,0,0,"Arc Point[%6ld] = %12.6lf %12.6lf %10.4lf",(long)(p3d-*ArcPts),p3d->x,p3d->y,p3d->z) ; 
      }
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmMath_freeArcPoints()                                           |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmMath_freeArcPoints(DPoint3d **Points)
{
 if( Points != NULL ) { free(*Points) ; *Points = NULL ; }
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_checkIfLinesIntersect(double x1,double y1,double x2,double y2,double x3,double y3,double x4,double y4)
/*
** This Function Checks If Two Lines Intersect
** Return Values  ==  0  No Intersection
**                ==  1  Intersection
**
*/
{
 int    sd1,sd2 ;
 double xn1,xm1,yn1,ym1,xn2,xm2,yn2,ym2 ;
/*
** Check If Bounding Rectangles Overlap
*/
 if( x1 <= x2 ) { xn1 = x1 ; xm1 = x2 ; } else { xn1 = x2 ; xm1 = x1 ; }
 if( y1 <= y2 ) { yn1 = y1 ; ym1 = y2 ; } else { yn1 = y2 ; ym1 = y1 ; }
 if( x3 <= x4 ) { xn2 = x3 ; xm2 = x4 ; } else { xn2 = x4 ; xm2 = x3 ; }
 if( y3 <= y4 ) { yn2 = y3 ; ym2 = y4 ; } else { yn2 = y4 ; ym2 = y3 ; }
 if( xn1 > xm2 || xm1 < xn2 || yn1 > ym2 || ym1 < yn2 ) return(0) ;
/*
** Calculate SideOf Values
*/
 sd1 = bcdtmMath_sideOf(x3,y3,x4,y4,x1,y1) ;
 sd2 = bcdtmMath_sideOf(x3,y3,x4,y4,x2,y2) ;
 if( sd1 == sd2 && sd1 != 0 ) return(0) ;
 sd1 = bcdtmMath_sideOf(x1,y1,x2,y2,x3,y3) ;
 sd2 = bcdtmMath_sideOf(x1,y1,x2,y2,x4,y4) ;
 if( sd1 == sd2 && sd1 != 0 ) return(0) ;
/*
** Lines Intersect
*/
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmMath_normaliseAngle()                                          |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT double bcdtmMath_normaliseAngle(double Angle )
{
 while ( Angle >= DTM_2PYE ) Angle -= DTM_2PYE ;
 while ( Angle <  0.0      ) Angle += DTM_2PYE ;
 return (Angle);
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_calculateIncludedAngle(double X1,double Y1,double X2,double Y2,double X3,double Y3)
/*
** This Function Calculates The Included Angle At X2,Y2
*/
{
 double a,b,c ;
 a = bcdtmMath_distance(X1,Y1,X3,Y3) ;
 b = bcdtmMath_distance(X1,Y1,X2,Y2) ;
 c = bcdtmMath_distance(X2,Y2,X3,Y3) ;
 return(acos((b*b+c*c-a*a)/(2.0*b*c))) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_getBoundingCubeForPointArray(DPoint3d *ptsP,long numPts,double *xMinP,double *xMaxP,double *yMinP,double *yMaxP,double *zMinP,double *zMaxP)
{
 DPoint3d *p3dP ;
/*
** Initialise
*/
 *xMinP = *xMaxP = ptsP->x ;
 *yMinP = *yMaxP = ptsP->y ;
 *zMinP = *zMaxP = ptsP->z ;
/*
** Scan Points
*/
 for( p3dP = ptsP ; p3dP < ptsP + numPts ; ++p3dP )
   {
    if( p3dP->x < *xMinP ) *xMinP = p3dP->x ;  
    if( p3dP->x > *xMaxP ) *xMaxP = p3dP->x ;  
    if( p3dP->y < *yMinP ) *yMinP = p3dP->y ;  
    if( p3dP->y > *yMaxP ) *yMaxP = p3dP->y ;  
    if( p3dP->z < *zMinP ) *zMinP = p3dP->z ;  
    if( p3dP->z > *zMaxP ) *zMaxP = p3dP->z ;  
   }
/*
** Return
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private double bcdtmMath_angleDifference(double angle1,double angle2)
{
 double diffAngle ;
 if( angle1 < 0.0 ) angle1 += DTM_2PYE ;
 if( angle2 < 0.0 ) angle1 += DTM_2PYE ;
 diffAngle = fabs(angle1-angle2) ;
 if( DTM_2PYE - diffAngle < diffAngle ) diffAngle = DTM_2PYE - diffAngle ;
 return(diffAngle) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMath_getTriangleDescentAndAscentAnglesDtmObject
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
       slopePercentage = tan(slopeDegrees)        ;
       *trgSlopeP = slopePercentage ;
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
 return(DTM_SUCCESS) ;
}
