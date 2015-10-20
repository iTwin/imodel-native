/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmFnd.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
BENTLEYDTM_EXPORT int bcdtmFind_triangleForPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double *ZP,long *fndTypeP,long *pnt1P,long *pnt2P,long *pnt3P )
/*
**
** Note :- fndTypeP ( Point Find Type ) Return Values
**
**  == 0   Point External To Dtm
**  == 1   Point Coincident with Point pnt1P
**  == 2   Point On Line pnt1-Ppnt2P
**  == 3   Point On Hull Line pnt1P-pnt2P
**  == 4   Point In Triangle pnt1P-pnt2P-pnt3P
**
*/
{
 int  ret=DTM_SUCCESS ;
 long p ;
/*
** Initialise Variables
*/
 *ZP       = 0.0 ;  
 *fndTypeP = 0   ;
 *pnt1P = *pnt2P = *pnt3P = dtmP->nullPnt ;
/*
** Find Triangle
*/
 if( bcdtmFind_triangleDtmObject(dtmP,x,y,fndTypeP,pnt1P,pnt2P,pnt3P) ) goto errexit ;
/*
** If Point Inside Tin Hull Interpolate Point
*/
 if( *fndTypeP  )  
   {
/*
**  Point Coincident With Existing dtmP Point 
*/
    if( *fndTypeP == 1 ) *ZP = pointAddrP(dtmP,*pnt1P)->z ;
    else
      {
/*
**     Set Find Type To Triangle
*/
       *fndTypeP = 4 ;
/*
**     Set Points Clockwise
*/
       if( bcdtmMath_pointSideOfDtmObject(dtmP,*pnt1P,*pnt2P,*pnt3P) > 0 ) { p = *pnt2P ; *pnt2P = *pnt3P ; *pnt3P = p ; }
/*
**     Test If Point On Tin Line
*/
       if     ( bcdtmMath_sideOf(pointAddrP(dtmP,*pnt1P)->x,pointAddrP(dtmP,*pnt1P)->y,pointAddrP(dtmP,*pnt2P)->x,pointAddrP(dtmP,*pnt2P)->y,x,y)  == 0 ) { *fndTypeP = 2 ; *pnt3P = dtmP->nullPnt ; }
       else if( bcdtmMath_sideOf(pointAddrP(dtmP,*pnt2P)->x,pointAddrP(dtmP,*pnt2P)->y,pointAddrP(dtmP,*pnt3P)->x,pointAddrP(dtmP,*pnt3P)->y,x,y)  == 0 ) { *fndTypeP = 2 ; *pnt1P = *pnt2P ; *pnt2P = *pnt3P ; *pnt3P = dtmP->nullPnt ; }
       else if( bcdtmMath_sideOf(pointAddrP(dtmP,*pnt3P)->x,pointAddrP(dtmP,*pnt3P)->y,pointAddrP(dtmP,*pnt1P)->x,pointAddrP(dtmP,*pnt1P)->y,x,y)  == 0 ) { *fndTypeP = 2 ; *pnt2P = *pnt3P ; *pnt3P = dtmP->nullPnt ; }
/*
**     Test If Point On Hull Line
*/
       if( *fndTypeP == 2 ) 
         {
          if      ( nodeAddrP(dtmP,*pnt1P)->hPtr == *pnt2P )    *fndTypeP = 3 ;
          else if ( nodeAddrP(dtmP,*pnt2P)->hPtr == *pnt1P )  { *fndTypeP = 3 ;p = *pnt1P ; *pnt1P = *pnt2P ; *pnt2P = p ; }
         }
/*
**     Set Lowest Point Number First
*/
       if( *fndTypeP == 4 ) while ( *pnt1P > *pnt2P || *pnt1P > *pnt3P ) { p = *pnt1P ; *pnt1P = *pnt2P ; *pnt2P = *pnt3P ; *pnt3P = p ; }
       if( *fndTypeP == 2 && *pnt1P > *pnt2P )  { p = *pnt1P ; *pnt1P = *pnt2P ; *pnt2P = p ; }
/*
**     Interpolate Point 
*/
       if( *fndTypeP  < 4 ) bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,ZP,*pnt1P,*pnt2P) ; 
       if( *fndTypeP == 4 ) bcdtmMath_interpolatePointOnTriangleDtmObject(dtmP,x,y,ZP,*pnt1P,*pnt2P,*pnt3P) ;
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
BENTLEYDTM_EXPORT int bcdtmFind_binaryScanDtmObject(BC_DTM_OBJ *dtmP,double x,long *cPointP)
/*
** Binary scan x axis to find first data point equal to or less than x
*/
{
 long bp,tp ;
 DPoint3d *pntP ;	  
/*
** Initialise
*/
 if( x <= pointAddrP(dtmP,0)->x ) { *cPointP = 0 ; return(0) ; }
 if( x >= pointAddrP(dtmP,dtmP->numSortedPoints-1)->x ) { *cPointP = dtmP->numSortedPoints - 1 ; return(0) ; }
 bp = 0 ;
 tp = dtmP->numSortedPoints - 1 ; 
 *cPointP = ( tp + bp ) / 2 ;
 while( tp - bp > 1 )
   {
    pntP = pointAddrP(dtmP,*cPointP) ;
    if( x == pntP->x )  bp = tp = *cPointP ;
    if( x  > pntP->x )  bp = *cPointP ; 
    else                tp = *cPointP ;
    *cPointP = ( tp + bp ) / 2 ;
   }
/*
** Job Completed
*/
 return(0) ;
}

BENTLEYDTM_Public double bcdtmMath_distanceSquared(double X1,double Y1,double X2,double Y2)
{
 double x,y ;
 x = ( X2 - X1 );
 y = ( Y2 - Y1 );
 return (x * x) + (y * y) ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFind_closestPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *cPointP)
/*
** This routine finds the closest Dtm point to p(x,y)
*/
{
 long    dbg=DTM_TRACE_VALUE(0) ;
 long    cpnt,spnt,process ;
 double  dn = 0.0,dns,dd ;
 DPoint3d *cpntP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Finding Closest Point") ;
    bcdtmWrite_message(0,0,0,"dtmP                  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x                     = %12.5lf",x) ; 
    bcdtmWrite_message(0,0,0,"y                     = %12.5lf",y) ; 
    bcdtmWrite_message(0,0,0,"dtmP->dtmState        = %8ld",dtmP->dtmState) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints       = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints = %8ld",dtmP->numSortedPoints) ;
   }
/*
** Initialise
*/
 *cPointP = dtmP->nullPnt ;
/*
** Only Find If DTM Is In Tin State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
/*
**  Binary Scan x-Axis and find first x point value equal x or less than x
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Binary Scanning DTM Object") ;
    bcdtmFind_binaryScanDtmObject(dtmP,x,&cpnt) ;
    spnt = *cPointP = cpnt ;
    if( nodeAddrP(dtmP,cpnt)->cPtr != dtmP->nullPtr ) dns = bcdtmMath_distanceSquared(pointAddrP(dtmP,cpnt)->x,pointAddrP(dtmP,cpnt)->y,x,y) ;
    else                                              dns = /*sqrt*/(dtmP->xRange*dtmP->xRange + dtmP->yRange*dtmP->yRange) ; 
    if( dns == 0 )  goto cleanup ;
    dn = sqrt (dns);
/*
** Scan Back Until x - x point value  > dn
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Backwards") ;
    process = 1 ;
    for( cpnt = spnt - 1 ; cpnt >= 0 && process  ; --cpnt )
      {
       cpntP = pointAddrP(dtmP,cpnt) ;
       if( x - cpntP->x >= dn ) process = 0 ;
       else
         {
          if( fabs(y-cpntP->y ) < dn && nodeAddrP(dtmP,cpnt)->cPtr != dtmP->nullPtr )
            {
             dd = bcdtmMath_distanceSquared(cpntP->x,cpntP->y,x,y) ;
             if( dd < dns ) 
               {
                dns = dd ;
                dn = sqrt (dns);

                *cPointP = cpnt ;
               }
            }
         }
      }
/*
**  Scan Forwards Until x point value - x > dn
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Forwards") ;
    if( dn > 0.0 ) process = 1 ;
    for( cpnt = spnt + 1 ; cpnt < dtmP->numSortedPoints && process ; ++cpnt )
      {
       cpntP = pointAddrP(dtmP,cpnt) ;
       if( cpntP->x - x >= dn ) process = 0 ;
       else 
         {
          if( fabs(y-cpntP->y) < dn && nodeAddrP(dtmP,cpnt)->cPtr != dtmP->nullPtr )
            {
             dd = bcdtmMath_distanceSquared(cpntP->x,cpntP->y,x,y) ;
             if( dd < dns ) 
               { 
                dns = dd ;
                dn = sqrt (dns);
                *cPointP = cpnt ;
               }
            }
         }
      }
/*
**  Scan Inserted 
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Inserted") ;
    if( dtmP->numSortedPoints != dtmP->numPoints && dn > 0.0 )
      {
       for( cpnt = dtmP->numSortedPoints ; cpnt < dtmP->numPoints ; ++cpnt )
         {
          cpntP = pointAddrP(dtmP,cpnt) ;
          if( fabs(x-cpntP->x) <= dn && fabs(y-cpntP->y) <= dn && nodeAddrP(dtmP,cpnt)->cPtr != dtmP->nullPtr )
            {
             dd = bcdtmMath_distanceSquared(cpntP->x,cpntP->y,x,y) ;
             if( dd < dns ) { dns = dd ; dn = sqrt (dns); *cPointP = cpnt ; }
            }
         }
      }
   }
/*
** Job Completed
*/
 cleanup :
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Point Completed ** cp = %8ld ** dn = %15.10lf",*cPointP,dn) ;
 if( dn == 0.0  ) return(1) ; 
 else             return(2) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFind_triangleDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *fndTypeP,long *pnt1P,long *pnt2P,long *pnt3P)
/*
** This routine finds the triangle the point x,y lies in
**
** Find Type ( fndTypeP ) Return Values
**
**   0 - Data Point Outside Data Set Area
**   1 - Point on Triangle Vertex p1
**   2 - Triangle Found vertices are p1,p2,p3
*/
{
 int   ret=DTM_SUCCESS;  
 long  closestPnt  ;
/*
** Initialise
*/
 *fndTypeP = 0 ;
 *pnt1P = *pnt2P = *pnt3P = dtmP->nullPnt ;
 
/*
** Check Dtm In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated") ;
    goto errexit ;
   }
/*
** Check If Point Is External To Minimum Bounding Rectangle Of DTMFeatureState::Tin
*/
 if( x >= dtmP->xMin - 1.0  && x <= dtmP->xMax + 1.0 && y >= dtmP->yMin - 1.0  && y <= dtmP->yMax + 1.0 )
   {
/*
**  Find Closest Tin Point to p(x,y)
*/
    if( bcdtmFind_closestPointDtmObject(dtmP,x,y,&closestPnt) == 1 ) 
      { 
       *pnt1P = closestPnt ; 
       *fndTypeP = 1 ; 
      }
/*
** Scan Tin Structure For Point
*/
   else if( bcdtmFind_triangleForPointFromPointDtmObject(dtmP,x,y,closestPnt,fndTypeP,pnt1P,pnt2P,pnt3P)) goto errexit ;
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
BENTLEYDTM_Public int bcdtmFind_triangleForPointFromPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long closestPnt,long *pntTypeP,long *pnt1P,long *pnt2P,long *pnt3P)
/*
** This Function Finds The Triangle Containing Point x,y From closestPnt
** 
** *pntTypeP = 0  Point External To DTMFeatureState::Tin
**           = 1  Point In Triangle pnt1P-pnt2P-pnt3P
**  
*/
{
 int  ret=DTM_SUCCESS,sdof1,sdof2 ;
 long p0,p1,p2,p3,clc,hullIntFnd,scan=1 ;
/*
** Initialise
*/
 *pntTypeP = 0 ;
 *pnt1P = *pnt2P = *pnt3P = dtmP->nullPnt ;
/*
** Scan Circular List about Closest Point and determine if p lies within a Triangle
*/
 p0  = closestPnt ;
 while ( scan )
   {
    p1  = p0 ;
    clc = nodeAddrP(dtmP,p1)->cPtr ;
    p3  = clistAddrP(dtmP,clc)->pntNum ;
    if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,p3)) < 0 ) goto errexit ;
    sdof1 = bcdtmMath_linePointSideOfDtmObject(dtmP,p1,p2,x,y) ;
    while ( clc != dtmP->nullPtr )
      {
       p3  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       sdof2 = bcdtmMath_linePointSideOfDtmObject(dtmP,p1,p3,x,y) ;
       if( nodeAddrP(dtmP,p1)->hPtr != p2 )
         {
          if( sdof1 <= 0 && sdof2 >= 0 )
            {
/*
**           Test For Point In Triangle
*/
             if( bcdtmMath_linePointSideOfDtmObject(dtmP,p2,p3,x,y) <= 0 )
               {
                if( x == pointAddrP(dtmP,p1)->x && y == pointAddrP(dtmP,p1)->y ) { *pnt1P = p1 ; *pntTypeP = 1 ; goto cleanup ; }    
                if( x == pointAddrP(dtmP,p2)->x && y == pointAddrP(dtmP,p2)->y ) { *pnt1P = p2 ; *pntTypeP = 1 ; goto cleanup ; }    
                if( x == pointAddrP(dtmP,p3)->x && y == pointAddrP(dtmP,p3)->y ) { *pnt1P = p3 ; *pntTypeP = 1 ; goto cleanup ; }    
                *pnt1P = p1 ; 
                *pnt2P = p2 ; 
                *pnt3P = p3 ; 
                *pntTypeP = 2 ; 
                scan = 0 ;
                goto cleanup ; 
               }
/*
**           Test For Point Going External From Hull Line pnt3P-pnt2P
*/
             if( nodeAddrP(dtmP,p3)->hPtr == p2 )
               {
                if( bcdtmFind_hullIntersectionDtmObject(dtmP,&hullIntFnd,p1,x,y,&p0,&p2,&p3) ) goto errexit ;
                if( ! hullIntFnd ) goto cleanup ;
               }
/*
**           Get Next Point To Scan
*/
             else if(( p0 = bcdtmList_nextClkDtmObject(dtmP,p3,p2)) < 0 ) goto errexit ;
/*
**           Stop Scan Around Current Point
*/
             clc = dtmP->nullPtr ;
            }
         }
       p2 = p3 ;
       sdof1 = sdof2 ; 
      }
/*
**  Test For Line Goining External From Hull Point pnt1P
*/
    if( p1 == p0 ) 
      {
       if( bcdtmFind_hullIntersectionDtmObject(dtmP,&hullIntFnd,p1,x,y,&p0,&p2,&p3) ) goto errexit ;
       if( ! hullIntFnd ) goto cleanup ;
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
BENTLEYDTM_Private int bcdtmFind_hullIntersectionDtmObject(BC_DTM_OBJ *dtmP,long *intFoundP,long point,double x,double y,long *pnt1P,long *pnt2P,long *pnt3P )
{
/*
** Do Not Fucking Change This Function
*/
 int    ret=DTM_SUCCESS,sd1,sd2 ;
 long   fp,p1,p2,isw=1 ;
 double xInt,yInt,dist,dd  ;
 double xln,xlm,yln,ylm,xhn,xhm,yhn,yhm  ;
/*
** Initialise Variables
*/  
 *intFoundP = 0 ;  
 p1 = fp = dtmP->hullPoint ;
 p2 = nodeAddrP(dtmP,p1)->hPtr ;
 dist = bcdtmMath_distance(pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,x,y) ;
 if( pointAddrP(dtmP,point)->x <= x ) { xln = pointAddrP(dtmP,point)->x ; xlm = x ; }
 else                                 { xlm = pointAddrP(dtmP,point)->x ; xln = x ; }
 if( pointAddrP(dtmP,point)->y <= y ) { yln = pointAddrP(dtmP,point)->y ; ylm = y ; }
 else                                 { ylm = pointAddrP(dtmP,point)->y ; yln = y ; }  
/*
** Scan Convex Hull
*/
 do
   {
    if( p1 != point && p2 != point )
      {
       if( pointAddrP(dtmP,p1)->x <= pointAddrP(dtmP,p2)->x ) { xhn = pointAddrP(dtmP,p1)->x ; xhm = pointAddrP(dtmP,p2)->x ; }
       else                                                   { xhm = pointAddrP(dtmP,p1)->x ; xhn = pointAddrP(dtmP,p2)->x ; }
       if( pointAddrP(dtmP,p1)->y <= pointAddrP(dtmP,p2)->y ) { yhn = pointAddrP(dtmP,p1)->y ; yhm = pointAddrP(dtmP,p2)->y ; }
       else                                                   { yhm = pointAddrP(dtmP,p1)->y ; yhn = pointAddrP(dtmP,p2)->y ; }
       xhn = xhn - 0.0001 ; yhn = yhn - 0.0001 ;
       xhm = xhm + 0.0001 ; yhm = yhm + 0.0001 ; 
       if( xln <= xhm && xlm >= xhn && yln <= yhm  && ylm >= yhn  )
         { 
          sd1 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y) ;
          sd2 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,x,y) ;
          if( sd1 != sd2 )
            {
             sd1 = bcdtmMath_sideOf(x,y,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
             sd2 = bcdtmMath_sideOf(x,y,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
             if( sd1 != sd2 )
               {
                bcdtmMath_normalIntersectCordLines(x,y,pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,&xInt,&yInt) ;
                dd = bcdtmMath_distance(xInt,yInt,x,y) ;
                if( isw || dd < dist )
                  {
                   *pnt1P = p1 ;
                   *pnt2P = p2 ;
                   if(( *pnt3P = bcdtmList_nextAntDtmObject(dtmP,p1,p2) ) < 0 ) goto errexit ;
                   *intFoundP = 1 ;
                  }
               }
            }
         }
      }
    p1 = p2 ; p2 = nodeAddrP(dtmP,p1)->hPtr ;
   } while ( p1 != fp ) ;
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
BENTLEYDTM_EXPORT int bcdtmFind_findClosestHullLineDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double *z,long *fndTypeP,long *hullPnt1P,long *hullPnt2P)
/*
** This Routine Find the Closeset Hull Line to x,y
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   closePnt,pnt1,pnt2,onLine ;
 double dist,tolerance,xi,yi,xmin,ymin,xmax,ymax ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Hull Line") ;
/*
** Initialiase
*/
 *fndTypeP  = 0   ;
 *hullPnt1P = *hullPnt2P = dtmP->nullPnt ;
/*
** Find Closest Point To x,y
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Point") ;
 bcdtmFind_closestPointDtmObject(dtmP,x,y,&closePnt) ;
/*
** Test If Closest Point is a Hull Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking If Closest Point Is A Hull Point") ;
 if( nodeAddrP(dtmP,closePnt)->hPtr != dtmP->nullPnt ) 
    { 
     *hullPnt1P = closePnt ; 
     *fndTypeP = 1 ; 
     tolerance = bcdtmMath_distance(x,y,pointAddrP(dtmP,closePnt)->x,pointAddrP(dtmP,closePnt)->y) ;
    }
 else  tolerance = 1e60;
/*
** Find Closest Hull Point
*/
 if( *hullPnt1P == dtmP->nullPnt )
   {
   if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Hull Point") ;
    pnt1 = dtmP->hullPoint ;
    do
      {
       if( pnt1 < 0 || pnt1 >= dtmP->numPoints ) 
         { 
          bcdtmWrite_message(1,0,0,"First Pointer Error In Tin Hull") ;
          goto errexit ;
         }
       if( fabs(pointAddrP(dtmP,pnt1)->x-pointAddrP(dtmP,closePnt)->x) <= tolerance && fabs(pointAddrP(dtmP,pnt1)->y-pointAddrP(dtmP,closePnt)->y ) <= tolerance )
         {
          dist = bcdtmMath_distance(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,x,y) ;
          if( dist < tolerance ) { closePnt = *hullPnt1P = pnt1 ; *fndTypeP = 1 ; tolerance = dist ; }
         }
       pnt1 = nodeAddrP(dtmP,pnt1)->hPtr ;
      } while ( pnt1 != dtmP->hullPoint ) ;
   }
/*
** Scan Hull Looking For Closest Line
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Hull For Closest Hull Line") ;
 pnt1 = dtmP->hullPoint ; 
 do
   {
    pnt2 = nodeAddrP(dtmP,pnt1)->hPtr ;
    if( pointAddrP(dtmP,pnt1)->x <= pointAddrP(dtmP,pnt2)->x ) { xmin = pointAddrP(dtmP,pnt1)->x ; xmax = pointAddrP(dtmP,pnt2)->x ; }
    else                                                       { xmin = pointAddrP(dtmP,pnt2)->x ; xmax = pointAddrP(dtmP,pnt1)->x ; }
    if( pointAddrP(dtmP,pnt1)->y <= pointAddrP(dtmP,pnt2)->y ) { ymin = pointAddrP(dtmP,pnt1)->y ; ymax = pointAddrP(dtmP,pnt2)->y ; }
    else                                                       { ymin = pointAddrP(dtmP,pnt2)->y ; ymax = pointAddrP(dtmP,pnt1)->y ; }
    if( x >= (xmin-tolerance) && x <= (xmax+tolerance)  && y >= (ymin-tolerance) && y <= (ymax+tolerance)    )
      {
       dist = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,x,y,&xi,&yi) ;
       if( onLine && ( dist < tolerance ) )
         {
          tolerance  = dist ;
          *hullPnt1P = pnt1 ;
          *hullPnt2P = pnt2 ;
          *fndTypeP  = 2 ;
         }
      }
    pnt1 = pnt2 ;
   } while ( pnt1 != dtmP->hullPoint ) ;
/*
** Set z Value
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Hull Line z Value") ;
 if( *fndTypeP == 1 ) *z = pointAddrP(dtmP,*hullPnt1P)->z ;
 else                 bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,z,*hullPnt1P,*hullPnt2P) ; 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Hull Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Hull Line Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
