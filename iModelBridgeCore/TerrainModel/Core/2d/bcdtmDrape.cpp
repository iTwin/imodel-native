/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmDrape.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
/* 
** macros, globals & functions used by bcdtmDrape_getCoordiantesFromLength 
*/
#define SIGN_OF(_x)  (_x >= 0 ? 1 : -1)
static const double fc_zero      = 0.0    ;
static const double fc_epsilon   = 0.00001;
static const double fc_1         = 1.0    ;
static const double fc_2         = 2.0    ;
static const double fc_90        = 90.0   ;
static const double fc_100       = 100.0  ;
static const double fc_180       = 180.0  ;
static const double fc_270       = 270.0  ;
static const double fc_360       = 360.0  ;
static const double fc_pi        = 3.14159265359    ;
static const double fc_180overpi = 57.295779513082  ;
static const double fc_piover180 = 0.017453292520 ;
static const double SMALL        = 0.0000001 ; 
static const int    FAIL         = 1  ;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmDrape_pointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double *z,long *drapeFlagP)
/*
**
**  drapeFlagP
**
**    <==  0  Point Outside Tin Object
**    <==  1  Point Inside  Tin Object
**    <==  2  Point In Void  
**     
**  Return Values
**
**     ==> 0  Success
**     ==> 1  Error 
**     
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long pnt1, pnt2, pnt3, fndType;
 bool voidFlag;
 DPoint3d *pnt1P,*pnt2P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Draping Point On Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x    = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y    = %12.5lf",y) ;
   } 
/*
** Initialise 
*/
 *z = 0.0  ;
 *drapeFlagP = 0 ;
/*
**  Find Triangle
**
** Note :- fndType ( Point Find Type ) Return Values
**
**  == 0   Point External To Dtm
**  == 1   Point Coincident with Point pnt1P
**  == 2   Point On Line pnt1-Ppnt2P
**  == 3   Point On Hull Line pnt1P-pnt2P
**  == 4   Point In Triangle pnt1P-pnt2P-pnt3P
**
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,z,&fndType,&pnt1,&pnt2,&pnt3 ) ) goto errexit ;
/*
** If Point External Test If Point Within ppTol Of Hull Line Or Hull Point
*/
 if( ! fndType )
   {
    bcdtmDrape_findClosestHullLineDtmObject(dtmP,x,y,&pnt1,&pnt2) ;
    if( pnt1 != dtmP->nullPnt && pnt2 != dtmP->nullPnt ) 
      {
       pnt1P = pointAddrP(dtmP,pnt1) ;
       pnt2P = pointAddrP(dtmP,pnt2) ;
       if( bcdtmMath_distance(x,y,pnt1P->x,pnt1P->y) <= dtmP->ppTol ) { fndType = 1 ; ; pnt2 = dtmP->nullPnt ; *z = pnt1P->z ; }
       else if( bcdtmMath_distance(x,y,pnt2P->x,pnt2P->y) <= dtmP->ppTol ) { fndType = 1 ; pnt1 = pnt2 ; pnt2 = dtmP->nullPnt ; *z = pnt2P->z ; }
       else if( bcdtmMath_normalDistanceToLineDtmObject(dtmP,pnt1,pnt2,x,y) <= dtmP->ppTol )
         {
          fndType = 3 ; 
          bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,z,pnt1,pnt2) ;
         }
      }   
   }
/*
** Test For Point In Void
*/
 if( fndType )
   {
    *drapeFlagP = 1 ;
    if( fndType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,pnt1)->PCWD) ) *drapeFlagP = 2 ;
    if( fndType == 2 || fndType == 3 ) 
      {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt1,pnt2,voidFlag)) goto errexit ;
       if( voidFlag ) *drapeFlagP = 2 ;
      }
    if( fndType == 4 ) 
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidFlag)) goto errexit ;
       if( voidFlag ) *drapeFlagP = 2 ;
      }
   }  
/*
** Set z To Zero For Point In Void
*/
 if( *drapeFlagP == 2 ) *z = 0.0 ;
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
BENTLEYDTM_Private int bcdtmDrape_findClosestHullLineDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *pnt1P,long *pnt2P)
/*
** This Routine Find the Closeset Hull Line to x,y
*/
{
 long   p1,p2,isw,lf ;
 double d1,d2,d3,d4,dn=0.0,Xn,Yn  ;
 DPoint3d *p1P,*p2P ;
/*
** Initialiase
*/
 *pnt1P = *pnt2P = dtmP->nullPnt ;
/*
** Find Closest Hull Line
*/
 isw = 1 ;
 p1  = dtmP->hullPoint ;
 p1P = pointAddrP(dtmP,p1) ;
 do
   {
    p2  = nodeAddrP(dtmP,p1)->hPtr ;
    p2P = pointAddrP(dtmP,p2) ;
    if( bcdtmMath_sideOf(p1P->x,p1P->y,p2P->x,p2P->y,x,y) < 0 )
      {
       d1 = bcdtmMath_distance(p1P->x,p1P->y,x,y) ;
       d2 = bcdtmMath_distance(p2P->x,p2P->y,x,y) ;
       d3 = bcdtmMath_distance((p1P->x+p2P->x) / 2.0,(p1P->y+p2P->y)/2.0,x,y) ;
       d4 = bcdtmMath_distanceOfPointFromLine(&lf,p1P->x,p1P->y,p2P->x,p2P->y,x,y,&Xn,&Yn) ;
       if( isw )
         { 
          *pnt1P = p1 ;
          *pnt2P = p2 ; 
          dn = d1 ;
          if( d2 < dn ) dn = d2 ;
          if( d3 < dn ) dn = d3 ;
          if( lf && d4 < dn ) dn = d4 ;  
          isw = 0 ;
         }
       else
         {
          if( d1 < dn || d2 < dn || d3 < dn || ( lf && d4 < dn ) ) 
            {
             *pnt1P = p1 ;
             *pnt2P = p2 ; 
             if( d1 < dn ) dn = d1 ;
             if( d2 < dn ) dn = d2 ;
             if( d3 < dn ) dn = d3 ;
             if( lf && d4 < dn ) dn = d4 ;  
            }
         }
      } 
    p1  = p2 ;
    p1P = p2P ;
   } while ( p1 != dtmP->hullPoint ) ;
/*
** Job Completed
*/
 return(0) ;
}
/*==============================================================================*//**
* @memo   Drapes A User String On The Tin And Optionally Returns The Dtm Features At The Drape Points
* @doc    Drapes A User String On The Tin And Optionally Returns The Dtm Features At The Drape Points
* @author Rob Cormack 14 December 2005 rob.cormack@bentley.com
* @param  dtmP                  ==> Pointer To Dtm Object                   
* @param  stringPtsP            ==> DPoint3d User Drape String     
* @param  numStringPts          ==> Number Of String Points               
* @param  dtmFeatureOption      ==> Optionally Return Dtm Features For Drape Points <TRUE,FALSE>            
* @param  drapePtsPP            <== Pointer To Drape Points               
* @param  numDrapePtsP          <== Number Of Drape Points 
* @param                  
* @return DTM_SUCCESS or DTM_ERROR
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmDrape_stringDtmObject
(
 BC_DTM_OBJ      *dtmP,              /* ==> Pointer To DTM Object                                        */
 DPoint3d             *stringPtsP,        /* ==> Pointer To String Points                                     */ 
 long            numStringPts,       /* ==> Number Of String Points                                      */
 long            dtmFeatureOption,   /* ==> Optionally Return Dtm Features For Drape Points <TRUE,FALSE> */ 
 bvector<DTMDrapePoint>& drapePts       /* <== Pointer To Drape Points                                      */
)
/*
**
** drapeType Values   ==  0  Drape Point External To Tin 
**                    ==  1  Drape Point In Triangle
**                    ==  2  Drape Point On Break Line  
**                    ==  3  Drape Point On Break Triangle Edge
**                    ==  4  Drape Point In Void     
**                    ==  5  Drape Point On Triangle Point
**                    ==  6  Drape Point On Triangle Edge
**
*/

{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    p1,p2,p3,np1,np2,np3,fndType,processDrape ;
 long    drapeType,lineNum,onLine ;
 double  nd,dz,xi,yi,zi,xls,yls,zls,xle,yle  ;
 DPoint3d     *p3dP ;
 DTMDrapePoint *dpP=NULL ;
 DTM_DAT_OBJ     *dataP=NULL ;
 DPoint3d   *pnt1P,*pnt2P ;
// long    removeOn ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Draping String On DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ; 
    bcdtmWrite_message(0,0,0,"stringPtsP       = %p",stringPtsP) ;
    bcdtmWrite_message(0,0,0,"numStringPts     = %8ld",numStringPts) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureOption = %8ld",dtmFeatureOption) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"**** String Points") ;
       for( p3dP = stringPtsP ; p3dP < stringPtsP + numStringPts ; ++p3dP)
         {
          bcdtmWrite_message(0,0,0,"Point[%4ld] ** x = %12.6lf y = %12.6lf",(long)(p3dP-stringPtsP),p3dP->x,p3dP->y) ;
         }
      }
   }
/*
** Validate Arguments
*/
 if( stringPtsP == NULL || numStringPts < 2  ) 
   {
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"stringPtsP   =  %p",stringPtsP) ;
       bcdtmWrite_message(0,0,0,"numStringPts =  %8ld",numStringPts) ;
      } 
    bcdtmWrite_message(1,0,0,"Invalid Drape String Arguments") ;
    goto errexit ;
   }
/*
** Test For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Initialise
*/
 drapePts.clear() ;
/*
** Process Each String Section
*/
 lineNum = 0 ;
 for( p3dP = stringPtsP + 1 ; p3dP < stringPtsP + numStringPts ; ++p3dP )
   {
    ++lineNum ;
    xls = (p3dP-1)->x ; yls = (p3dP-1)->y ;
    xle =  p3dP->x    ; yle = p3dP->y     ;
/*
**  Write Out Line To Be Drapped 
*/
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Drape Line [%4ld] = %10.4lf %10.4lf ** %10.4lf %10.4lf",lineNum-1,xls,yls,xle,yle) ;
/*
**  Find Triangle Containing Drape Start Point
*/
    processDrape = 1 ;
    if( bcdtmFind_triangleDtmObject(dtmP,xls,yls,&fndType,&p1,&p2,&p3)) goto errexit ;
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"fndType = %2ld",fndType) ;
/*
**  Triangle Not Found
*/
    if( fndType == 0 )
      {
/*
**     Check If Point Is In Point To Point Tolerance Of Tin Hull
*/
       if( bcdtmFind_findClosestHullLineDtmObject(dtmP,xls,yls,&zls,&fndType,&p1,&p2)) goto errexit ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Hull Line ** FndType = %2ld ** p1 = %8ld p2 = %8ld",fndType,p1,p2) ;
       if( fndType )
         {
          if( fndType == 1 )     // Hull Point
            {
             pnt1P = pointAddrP(dtmP,p1) ;
             if( bcdtmMath_distance(xls,yls,pnt1P->x,pnt1P->y) > dtmP->ppTol ) fndType = 0 ;
            }
          if( fndType == 2 )     // Hull Line
            {
             fndType = 0 ;
             pnt1P = pointAddrP(dtmP,p1) ;
             pnt2P = pointAddrP(dtmP,p2) ;
             nd = bcdtmMath_distanceOfPointFromLine(&onLine,pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y,xls,yls,&xi,&yi) ;
             if( onLine && nd <= dtmP->ppTol )
               {
                fndType = 2 ;
                xls = xi ;
                yls = yi ;
               } 
            }           
         }
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Hull Line ** FndType = %2ld ** p1 = %8ld p2 = %8ld",fndType,p1,p2) ;
/*
**     Find Closest Intersection With Tin Hull
*/
       if( fndType == 0 )
         { 
         if (bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject(dtmP, lineNum, 0, xls, yls, dtmP->zMin, dtmP->nullPnt, dtmP->nullPnt, dtmP->nullPnt, dtmFeatureOption, drapePts)) goto errexit;
          p1 = p2 = dtmP->nullPnt ;
          if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject(dtmP,xls,yls,xle,yle,&fndType,&p1,&p2,&xls,&yls,&zls) ) goto errexit ;
          if( fndType == 3 ) fndType = 1 ;
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Hull Intercept ** fndType = %2ld ** p1 = %8ld p2 = %8ld p3 = %8ld",fndType,p1,p2,p3) ;
         } 
       if( fndType == 0 ) 
         { 
          processDrape = 0 ; 
          if( bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject(dtmP,lineNum,0,xle,yle,dtmP->zMin,dtmP->nullPnt,dtmP->nullPnt,dtmP->nullPnt,dtmFeatureOption,drapePts) ) goto errexit ;
         }
       if( fndType == 2 )   
         { 
          p3 = p1 ; 
          p1 = p2 ; 
          p2 = p3 ;
          if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
         }
      }
/*
**  Point In Triangle - Test Snap To Triangle Verices Or Triangle Edges
*/
    else if( fndType == 2 ) 
      {
       fndType = 3 ;
       if     ( bcdtmMath_distance(xls,yls,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) < dtmP->ppTol ) { fndType = 1 ; p2 = p3 = dtmP->nullPnt ; }
       else if( bcdtmMath_distance(xls,yls,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) < dtmP->ppTol ) { fndType = 1 ; p1 = p2 ; p2 = p3 = dtmP->nullPnt ; }
       else if( bcdtmMath_distance(xls,yls,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y) < dtmP->ppTol ) { fndType = 1 ; p1 = p3 ; p2 = p3 = dtmP->nullPnt ; }
       else if( bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,xls,yls) < dtmP->plTol ) { fndType = 2 ; p3 = dtmP->nullPnt ; }
       else if( bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,xls,yls) < dtmP->plTol ) { fndType = 2 ; p1 = p2 ; p2 = p3 ; p3 = dtmP->nullPnt ; }
       else if( bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,xls,yls) < dtmP->plTol ) { fndType = 2 ; p2 = p1 ; p1 = p3 ; p3 = dtmP->nullPnt ; }
       if( fndType == 2 )   
         { 
          if( bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,xle,yle) > 0 )
            {
             p3 = p1 ; 
             p1 = p2 ; 
             p2 = p3 ;
            }
          if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
         } 
      } 
/*
**  Plot Start of Profile Line on Surface
*/
    if( processDrape )
      {
       drapeType = fndType ;
       if( drapeType == 1 ) 
         { 
          p2 = p3 = dtmP->nullPnt ;
          zls = pointAddrP(dtmP,p1)->z ;
         }
       else bcdtmMath_interpolatePointOnTriangleDtmObject(dtmP,xls,yls,&zls,p1,p2,p3) ;
       if( bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject(dtmP,lineNum,drapeType,xls,yls,zls,p1,p2,p3,dtmFeatureOption,drapePts) ) goto errexit ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Drape Start Point ** drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld ** %10.4lf %10.4lf %10.4lf",drapeType,p1,p2,p3,xls,yls,zls) ;
      }
/*
**  Scan To Drape Line End
*/
    while ( processDrape )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld",drapeType,p1,p2,p3) ;
       fndType = bcdtmDrape_getNextPointForDrapeDtmObject(dtmP,xls,yls,xle,yle,&drapeType,p1,p2,p3,&np1,&np2,&np3,&xi,&yi,&zi)  ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"fndType = %2ld drapeType = %2ld np1 = %8ld np2 = %8ld np3 = %8ld ** %10.4lf %10.4lf %10.4lf",fndType,drapeType,np1,np2,np3,xi,yi,zi) ;
/*
**     Error Detected
*/
       if( fndType == 4 ) goto errexit ;
/*
**     Next Drape Point Not Found
*/
       if( fndType == 3 ) processDrape = 0 ;
/*
**     Next Drape Point Found
*/
       if( fndType == 0 || fndType == 1 )
          {
           xls = xi ; yls = yi ; zls = zi ;
           p1 = np1 ; p2 = np2 ; p3 = np3 ;
/*
**         If Drape Terminates In Triangle Check Snap Tolerances
*/
           if( drapeType == 3 )
             {
              if     ( bcdtmMath_distance(xls,yls,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) < dtmP->ppTol ) { drapeType = 1 ; p2 = p3 = dtmP->nullPnt ; }
              else if( bcdtmMath_distance(xls,yls,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) < dtmP->ppTol ) { drapeType = 1 ; p1 = p2 ; p2 = p3 = dtmP->nullPnt ; }
              else if( bcdtmMath_distance(xls,yls,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y) < dtmP->ppTol ) { drapeType = 1 ; p1 = p3 ; p2 = p3 = dtmP->nullPnt ; }
              else if( bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,xls,yls) < dtmP->plTol ) { drapeType = 2 ; p3 = dtmP->nullPnt ; }
              else if( bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,xls,yls) < dtmP->plTol ) { drapeType = 2 ; p1 = p2 ; p2 = p3 ; p3 = dtmP->nullPnt ; }
              else if( bcdtmMath_normalDistanceToCordLine(pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,xls,yls) < dtmP->plTol ) { drapeType = 2 ; p2 = p1 ; p1 = p3 ; p3 = dtmP->nullPnt ; }
             }
          if( bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject(dtmP,lineNum,drapeType,xls,yls,zls,p1,p2,p3,dtmFeatureOption,drapePts) ) goto errexit ;
          if( fndType == 1 ) processDrape = 0 ;
         }
/*
**     Drape Line Goes External To Tin Hull
*/
       if( fndType == 2 )
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Line Goes External To Tin Hull") ;
/*
**        Check If End Point Is In Point To Point Tolerance Of Tin Hull
*/
          if( bcdtmFind_findClosestHullLineDtmObject(dtmP,xle,yle,&zls,&fndType,&np1,&np2)) goto errexit ;
          if( dbg == 1 )
            {
             bcdtmWrite_message(0,0,0,"p1 = %10ld p2 = %10ld p3 = %10ld",p1,p2,p3) ;
             bcdtmWrite_message(0,0,0,"Hull Line ** FndType = %2ld ** np1 = %8ld np2 = %8ld ** zls = %10.4lf",fndType,np1,np2,zls) ;
            }
          if( fndType )
            {
             if( fndType == 1 )     // Hull Point
               {
                pnt1P = pointAddrP(dtmP,np1) ;
                if( bcdtmMath_distance(xle,yle,pnt1P->x,pnt1P->y) > dtmP->ppTol ) fndType = 0 ;
                if( fndType && p2 == dtmP->nullPnt && np1 != p1 && ! bcdtmList_testLineDtmObject(dtmP,p1,np1)) fndType = 0 ;
                if( fndType && p2 != dtmP->nullPnt && np1 != p1 && np2 != p1 ) fndType = 0 ;
                if( fndType )
                  {
                   drapeType = 1 ;
                   xls = pnt1P->x ; 
                   yls = pnt1P->y ; 
                   p1 = np1 ;
                   p2 = dtmP->nullPnt ; p3 = dtmP->nullPnt ;
                   if( bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject(dtmP,lineNum,drapeType,xls,yls,zls,np1,p2,p3,dtmFeatureOption,drapePts) ) goto errexit ;
                   processDrape = 0 ;
                  } 
               }
             else if( fndType == 2 )     // Hull Line
               {
                fndType = 0 ; 
                if( p2 == dtmP->nullPnt && ( np1 == p1 || np2 == p1 ) ) fndType = 2 ;
                if( p2 != dtmP->nullPnt && ( np1 != p1 || np2 != p2 ) && ( np1 != p2 || np2 != p1 ) ) fndType = 0 ;
                if( fndType )
                  {
                   pnt1P = pointAddrP(dtmP,np1) ;
                   pnt2P = pointAddrP(dtmP,np2) ;
                   nd = bcdtmMath_distanceOfPointFromLine(&onLine,pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y,xle,yle,&xi,&yi) ;
                   if( ! onLine || nd > dtmP->ppTol ) fndType = 0 ;
                   if( fndType ) 
                     {
                      drapeType = 2 ;
                      xls = xi ;
                      yls = yi ;
                      p1 = np1 ;
                      p2 = np2 ; 
                      p3 = dtmP->nullPnt ;
                      if( bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject(dtmP,lineNum,drapeType,xls,yls,zls,p1,p2,p3,dtmFeatureOption,drapePts) ) goto errexit ;
                      processDrape = 0 ;
                     } 
                  }
               }           
            }
/*
**        Look For Intercept With Tin Hull
*/
          if( fndType == 0 )
            {
             if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject(dtmP,xls,yls,xle,yle,&drapeType,&p1,&p2,&xi,&yi,&zls) ) goto errexit ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"drapeType = %2ld ** p1 = %9ld p2 = %9ld",drapeType,p1,p2) ;
/*
**           No Further Intersections Of Drape Line With Tin Hull
*/
             if( drapeType == 0 ) 
               { 
/*
**              Check If Last Point Is Within Point To Point Tolerance Of Tin Hull
*/
                if( bcdtmMath_distance(xls,yls,xle,yle) > dtmP->ppTol )
                  {
                   if( bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject(dtmP,lineNum,0,xle,yle,dtmP->zMin,dtmP->nullPnt,dtmP->nullPnt,dtmP->nullPnt,dtmFeatureOption,drapePts) ) goto errexit ; 
                  }
                processDrape = 0 ;
               }
/*
**           Drape Line Coincident With Tin Hull
*/ 
             if( drapeType == 3 )
               {
                drapeType = 1 ;
                xls = xi ; yls = yi ; 
                p2 = dtmP->nullPnt ; p3 = dtmP->nullPnt ;
                if( bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject(dtmP,lineNum,drapeType,xls,yls,zls,p1,p2,p3,dtmFeatureOption,drapePts) ) goto errexit ;
               }
/*
**           Drape Line Crosses Gulf In Tin Hull
*/
             else if( drapeType != 0 )
               {
/*
**              Store Dummy Drape Point At Mid Point In Gulf
*/
                xls = ( xls + xi ) / 2.0 ; 
                yls = ( yls + yi ) / 2.0 ;
                if( bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject(dtmP,lineNum,0,xls,yls,dtmP->zMin,dtmP->nullPnt,dtmP->nullPnt,dtmP->nullPnt,dtmFeatureOption,drapePts) ) goto errexit ; 
/*
**              Store Drape Point At Hull Intersection
*/
                xls =  xi ; yls = yi  ;
                if( drapeType == 2 )
                  { 
                   p3 = p1 ; p1 = p2 ; p2 = p3 ;
                   if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                  } 
                if( bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject(dtmP,lineNum,drapeType,xls,yls,zls,p1,p2,p3,dtmFeatureOption,drapePts) ) goto errexit ;
               }
            }
         }
      }
   }
/*
** Reset Drape Type 3 - Fix For Wrapper Interpretation Of Type 3 As Between Break Lines
*/
/*
 removeOn = 1 ;
 for( dpP = *drapePtsPP ; dpP < *drapePtsPP + *numDrapePtsP ; ++dpP )
   {
    if( dpP->drapeType == 3 && removeOn ) dpP->drapeType = 1 ;
    if( dpP->drapeType == 1 ) removeOn = 1 ; 
    if( dpP->drapeType == 2 ) removeOn = 0 ; 
   }
 removeOn = 1 ;
 for( dpP = *drapePtsPP + *numDrapePtsP - 1 ; dpP >= *drapePtsPP ; --dpP )
   {
    if( dpP->drapeType == 3 && removeOn ) dpP->drapeType = 1 ;
    if( dpP->drapeType == 1 ) removeOn = 1 ; 
    if( dpP->drapeType == 2 ) removeOn = 0 ; 
   }
*/
/*
** Write Out Drape Points
*/
 if( dbg == 1 )
   {
    bcdtmObject_createDataObject(&dataP) ;
    bcdtmObject_setMemoryAllocationParametersDataObject(dataP,1000,1000) ;
    bcdtmWrite_message(0, 0, 0, "Number Of Drape Points = %6ld", drapePts.size());
    for (dpP = drapePts.data(); dpP < drapePts.data() + drapePts.size(); ++dpP)
      {
      bcdtmDrape_pointDtmObject(dtmP, dpP->drapePt.x, dpP->drapePt.y, &zls, &drapeType);
       dz = fabs(zls-dpP->drapePt.z) ;
       if (dz < 0.00001) bcdtmWrite_message(0, 0, 0, "[%4ld] %4ld %4ld %12.6lf %12.6lf %12.6lf ** Type = %2ld  %12.6lf", (long)(dpP - drapePts.data()), dpP->drapeLine, dpP->drapeType, dpP->drapePt.x, dpP->drapePt.y, dpP->drapePt.z, drapeType, zls);
       else               bcdtmWrite_message(0,0,0,"[%4ld] %4ld %4ld %12.6lf %12.6lf %12.6lf ** Type = %2ld  %12.6lf ** DTM_ERROR **",(long)(dpP-drapePts.data()),dpP->drapeLine,dpP->drapeType,dpP->drapePt.x,dpP->drapePt.y,dpP->drapePt.z,drapeType,zls) ;
       if( dpP == drapePts.data() ) bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullGuid,dpP->drapePt.x,dpP->drapePt.y,dpP->drapePt.z) ;
       else                     bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullGuid,dpP->drapePt.x,dpP->drapePt.y,dpP->drapePt.z) ;
      } 
    bcdtmWrite_dataFileFromDataObject(dataP,L"drapePts.dat") ;
    bcdtmObject_deleteDataObject(&dataP) ;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Draping String On DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Draping String On DTM Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmDrape_getNextPointForDrapeDtmObject(BC_DTM_OBJ *dtmP,double x1,double y1,double x2,double y2,long *drapeTypeP, long pnt1, long pnt2, long pnt3,long *nxtPnt1P,long *nxtPnt2P,long *nxtPnt3P,double *xdP,double *ydP,double *zdP )
/*
** Return Values
**
**   == 0   Next Point Found
**   == 1   End Point In Triangle
**   == 2   Drape Goes Outside Hull
**   == 3   Error No Intercept Found
**   == 4   System Error Detected - Terminate Processing
*/
{
/*
** Initialise Variables
*/
 *xdP  = 0.0 ;
 *ydP  = 0.0 ;
 *zdP  = 0.0 ;
 *nxtPnt1P = dtmP->nullPnt ;
 *nxtPnt2P = dtmP->nullPnt ;
 *nxtPnt3P = dtmP->nullPnt ;
/*
** Drape From Last Point
*/
 if     ( *drapeTypeP == 1 ) return( bcdtmDrape_getNextDrapePointFromPointDtmObject   (dtmP,x1,y1,x2,y2,drapeTypeP,pnt1,nxtPnt1P,nxtPnt2P,nxtPnt3P,xdP,ydP,zdP)) ;
 else if( *drapeTypeP == 2 ) return( bcdtmDrape_getNextDrapePointFromLineDtmObject    (dtmP,x1,y1,x2,y2,drapeTypeP,pnt1,pnt2,pnt3,nxtPnt1P,nxtPnt2P,nxtPnt3P,xdP,ydP,zdP)) ;
 else if( *drapeTypeP == 3 ) return( bcdtmDrape_getNextDrapePointFromTriangleDtmObject(dtmP,x1,y1,x2,y2,drapeTypeP,pnt1,pnt2,pnt3,nxtPnt1P,nxtPnt2P,nxtPnt3P,xdP,ydP,zdP)) ;
 else  return(4) ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmDrape_getNextDrapePointFromTriangleDtmObject(BC_DTM_OBJ *dtmP,double x1,double y1,double x2,double y2,long *drapeTypeP,long pnt1, long pnt2, long pnt3,long *nxtPnt1P,long *nxtPnt2P,long *nxtPnt3P,double *xdP,double *ydP,double *zdP )
{
 int    sd1,sd2,sd3 ;
 long   ps ;
 double d1,d2,xmin,xmax,ymin,ymax ;
/*
** Test For Last Point Equal To One Of The Triangle Points
*/
 if( x2 == pointAddrP(dtmP,pnt1)->x && y2 == pointAddrP(dtmP,pnt1)->y ) 
   { 
    *drapeTypeP = 1 ; 
    *nxtPnt1P = pnt1 ;  
    *xdP = pointAddrP(dtmP,pnt1)->x ; 
    *ydP = pointAddrP(dtmP,pnt1)->y ; 
    *zdP = pointAddrP(dtmP,pnt1)->z ; 
    return(1) ;
   }
 if( x2 == pointAddrP(dtmP,pnt2)->x && y2 == pointAddrP(dtmP,pnt2)->y ) 
   { 
    *drapeTypeP = 1 ; 
    *nxtPnt1P = pnt2 ; 
    *xdP = pointAddrP(dtmP,pnt2)->x ; 
    *ydP = pointAddrP(dtmP,pnt2)->y ; 
    *zdP = pointAddrP(dtmP,pnt2)->z ; 
    return(1) ;
   }
 if( x2 == pointAddrP(dtmP,pnt3)->x && y2 == pointAddrP(dtmP,pnt3)->y ) 
   { 
    *drapeTypeP = 1 ; 
    *nxtPnt1P = pnt3 ; 
    *xdP = pointAddrP(dtmP,pnt3)->x ; 
    *ydP = pointAddrP(dtmP,pnt3)->y ; 
    *zdP = pointAddrP(dtmP,pnt3)->z ; 
    return(1) ;
   }
/*
** Test For Last Point In Triangle
*/
 if( bcdtmMath_pointInTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,x2,y2) )
   {
    *drapeTypeP = 3 ;
    *nxtPnt1P = pnt1 ; 
    *nxtPnt2P = pnt2 ; 
    *nxtPnt3P = pnt3 ;
    *xdP = x2  ; 
    *ydP = y2  ;
    bcdtmMath_interpolatePointOnTriangleDtmObject(dtmP,x2,y2,zdP,*nxtPnt1P,*nxtPnt2P,*nxtPnt3P) ;
    return(1) ;
   }
/*
** Set Triangle Points Anti Clockwise
*/
 if( bcdtmMath_pointSideOfDtmObject(dtmP,pnt1,pnt2,pnt3) < 0 ) { ps = pnt3 ; pnt3 = pnt2 ; pnt2 = ps ; }
/*
** Initialise Variables
*/
 if( x1 <= x2 ) { xmin = x1 ; xmax = x2 ; }
 else           { xmin = x2 ; xmax = x1 ; }
 if( y1 <= y2 ) { ymin = y1 ; ymax = y2 ; }
 else           { ymin = y2 ; ymax = y1 ; }
/*
** Test For Intersection With Tin Point pnt1
*/
 if( bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) == 0 )
   {
    if( pointAddrP(dtmP,pnt1)->x >= xmin && pointAddrP(dtmP,pnt1)->x <= xmax &&
        pointAddrP(dtmP,pnt1)->y >= ymin && pointAddrP(dtmP,pnt1)->y <= ymax    )
      { 
       *xdP = pointAddrP(dtmP,pnt1)->x ; 
       *ydP = pointAddrP(dtmP,pnt1)->y ; 
       *zdP = pointAddrP(dtmP,pnt1)->z ; 
       *nxtPnt1P = pnt1 ;
       *drapeTypeP = 1  ; 
       return(0) ; 
      }
   }
/*
** Test For Intersection With Tin Point pnt2
*/
 if( bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) == 0 )
   {
    if( pointAddrP(dtmP,pnt2)->x >= xmin && pointAddrP(dtmP,pnt2)->x <= xmax &&
        pointAddrP(dtmP,pnt2)->y >= ymin && pointAddrP(dtmP,pnt2)->y <= ymax    )
      { 
       *xdP = pointAddrP(dtmP,pnt2)->x ;
       *ydP = pointAddrP(dtmP,pnt2)->y ; 
       *zdP = pointAddrP(dtmP,pnt2)->z ; 
       *nxtPnt1P = pnt2 ; 
       *drapeTypeP = 1  ;
       return(0) ;
      }
   }
/*
** Test For Intersection With Tin Point pnt3
*/
 if( bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) == 0 )
   {
    if( pointAddrP(dtmP,pnt3)->x >= xmin && pointAddrP(dtmP,pnt3)->x <= xmax &&
        pointAddrP(dtmP,pnt3)->y >= ymin && pointAddrP(dtmP,pnt3)->y <= ymax    )
      { 
       *xdP = pointAddrP(dtmP,pnt3)->x ; 
       *ydP = pointAddrP(dtmP,pnt3)->y ; 
       *zdP = pointAddrP(dtmP,pnt3)->z ; 
       *nxtPnt1P = pnt3 ;
       *drapeTypeP = 1 ;
       return(0) ; 
      }
   }
/*
** Test For Intersection With Tin Line pnt1-pnt2
*/
 sd1 = bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) ;
 sd2 = bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
 sd3 = bcdtmMath_sideOf(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,x2,y2) ;
 if( sd1 < 0 && sd2 > 0 && sd3 <= 0 )
   {
    bcdtmMath_normalIntersectCordLines(x1,y1,x2,y2,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,xdP,ydP) ;
    d1 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) ;
    d2 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
    if( d1 <= d2 && d1 < dtmP->ppTol / 10.0 ) { *zdP = pointAddrP(dtmP,pnt1)->z ; *nxtPnt1P = pnt1 ; *drapeTypeP = 1 ; return(0) ; }
    if( d2 <  d1 && d2 < dtmP->ppTol / 10.0 ) { *zdP = pointAddrP(dtmP,pnt2)->z ; *nxtPnt1P = pnt2 ; *drapeTypeP = 1 ; return(0) ; }
    *nxtPnt1P = pnt1 ; *nxtPnt2P = pnt2 ; *drapeTypeP = 2 ;
    bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*xdP,*ydP,zdP,*nxtPnt1P,*nxtPnt2P) ;
    if(( *nxtPnt3P = bcdtmList_nextClkDtmObject(dtmP,*nxtPnt1P,*nxtPnt2P)) < 0 ) return(4) ;
    return(0) ;
   }
/*
** Test For Intersection With Tin Line pnt2-pnt3
*/
 sd1 = bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
 sd2 = bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) ;
 sd3 = bcdtmMath_sideOf(pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,x2,y2) ;
 if( sd1 < 0 && sd2 > 0 && sd3 <= 0 )
   {
    bcdtmMath_normalIntersectCordLines(x1,y1,x2,y2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,xdP,ydP) ;
    d1 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
    d2 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) ;
    if( d1 <= d2 && d1 < dtmP->ppTol / 10.0 ) { *zdP = pointAddrP(dtmP,pnt2)->z ; *nxtPnt1P = pnt2 ; *drapeTypeP = 1 ; return(0) ; }
    if( d2 <  d1 && d2 < dtmP->ppTol / 10.0 ) { *zdP = pointAddrP(dtmP,pnt3)->z ; *nxtPnt1P = pnt3 ; *drapeTypeP = 1 ; return(0) ; }
    *nxtPnt1P = pnt2 ; *nxtPnt2P = pnt3 ; *drapeTypeP = 2 ;
    bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*xdP,*ydP,zdP,*nxtPnt1P,*nxtPnt2P) ;
    if(( *nxtPnt3P = bcdtmList_nextClkDtmObject(dtmP,*nxtPnt1P,*nxtPnt2P)) < 0 ) return(4) ;
    return(0) ;
   }
/*
** Test For Intersection With  Tin Line pnt3-pnt1
*/
 sd1 = bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) ;
 sd2 = bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) ;
 sd3 = bcdtmMath_sideOf(pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,x2,y2) ;
 if( sd1 < 0 && sd2 > 0 && sd3 <= 0 )
   {
    bcdtmMath_normalIntersectCordLines(x1,y1,x2,y2,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,xdP,ydP) ;
    d1 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) ;
    d2 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) ;
    if( d1 <= d2 && d1 < dtmP->ppTol / 10.0 ) { *zdP = pointAddrP(dtmP,pnt1)->z ; *nxtPnt1P = pnt1 ; *drapeTypeP = 1 ; return(0) ; }
    if( d2 <  d1 && d2 < dtmP->ppTol / 10.0 ) { *zdP = pointAddrP(dtmP,pnt3)->z ; *nxtPnt1P = pnt3 ; *drapeTypeP = 1 ; return(0) ; }
    *nxtPnt1P = pnt3 ; *nxtPnt2P = pnt1 ; *drapeTypeP = 2 ;
    bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*xdP,*ydP,zdP,*nxtPnt1P,*nxtPnt2P) ;
    if(( *nxtPnt3P = bcdtmList_nextClkDtmObject(dtmP,*nxtPnt1P,*nxtPnt2P)) < 0 ) return(4) ;
    return(0) ;
   }
/*
** Job Completed
*/
 return(3) ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmDrape_getNextDrapePointFromLineDtmObject(BC_DTM_OBJ *dtmP,double x1,double y1,double x2,double y2,long *Type,long pnt1, long pnt2, long pnt3,long *nxtPnt1P,long *nxtPnt2P,long *nxtPnt3P,double *xdP,double *ydP,double *zdP)
{
 int    sd1,sd2,dbg=DTM_TRACE_VALUE(0);
 long   closestPoint,onLine ;
 double d1,d2,closestDist,xn,yn  ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Next Drape Point From Line") ;
    bcdtmWrite_message(0,0,0,"p1 = %8ld p2 = %8ld p3 = %8ld",pnt1,pnt2,pnt3) ;
    bcdtmWrite_message(0,0,0,"x1 = %12.5lf y1 = %12.5lf",x1,y1) ;
    bcdtmWrite_message(0,0,0,"x2 = %12.5lf y2 = %12.5lf",x2,y2) ;
   }
/*
** Test For Hull Edge
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Hull Line") ;
 if( ! bcdtmList_testLineDtmObject(dtmP,pnt2,pnt3) )            return(2) ;
 if( bcdtmMath_pointSideOfDtmObject(dtmP,pnt1,pnt2,pnt3) >= 0 ) return(2) ;
/*
** Test For Last Point Equal To One Of The Triangle Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Last Point Equal To Triangle Point") ;
 *nxtPnt1P = *nxtPnt2P = *nxtPnt3P = dtmP->nullPnt ;
 if( x2 == pointAddrP(dtmP,pnt1)->x && y2 == pointAddrP(dtmP,pnt1)->y ) 
   { *Type = 1 ; *nxtPnt1P = pnt1 ; *xdP = pointAddrP(dtmP,pnt1)->x ; *ydP = pointAddrP(dtmP,pnt1)->y ; *zdP = pointAddrP(dtmP,pnt1)->z ; return(1) ; }
 if( x2 == pointAddrP(dtmP,pnt2)->x && y2 == pointAddrP(dtmP,pnt2)->y ) 
   { *Type = 1 ; *nxtPnt1P = pnt2 ; *xdP = pointAddrP(dtmP,pnt2)->x ; *ydP = pointAddrP(dtmP,pnt2)->y ; *zdP = pointAddrP(dtmP,pnt2)->z ; return(1) ; }
 if( x2 == pointAddrP(dtmP,pnt3)->x && y2 == pointAddrP(dtmP,pnt3)->y ) 
   { *Type = 1 ; *nxtPnt1P = pnt3 ; *xdP = pointAddrP(dtmP,pnt3)->x ; *ydP = pointAddrP(dtmP,pnt3)->y ; *zdP = pointAddrP(dtmP,pnt3)->z ; return(1) ; }
/*
** Test For Last Point In Triangle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Last Point In Triangle") ;
 if( bcdtmMath_pointInTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,x2,y2) )
   {
    *Type = 3 ;
    *nxtPnt1P = pnt1 ; 
    *nxtPnt2P = pnt2 ; 
    *nxtPnt3P = pnt3 ;
    *xdP = x2  ; 
    *ydP = y2  ;
    bcdtmMath_interpolatePointOnTriangleDtmObject(dtmP,x2,y2,zdP,*nxtPnt1P,*nxtPnt2P,*nxtPnt3P) ;
    return(1) ;
   }
/*
** Test For Intersection With pnt3
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Intersection With Pnt3") ;
 if( bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) == 0 )
   { *xdP = pointAddrP(dtmP,pnt3)->x ; *ydP = pointAddrP(dtmP,pnt3)->y ; *zdP = pointAddrP(dtmP,pnt3)->z ; *nxtPnt1P = pnt3 ; *Type = 1 ; return(0) ; }
/*
 if( bcdtmMath_normalDistanceToCordLine(x1,y1,x2,y2,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) < dtmP->PLTOL / 10.0 ) 
   { *xdP = pointAddrP(dtmP,pnt3)->x ; *ydP = pointAddrP(dtmP,pnt3)->y ; *zdP = pointAddrP(dtmP,pnt3)->z ; *nxtPnt1P = pnt3 ; *Type = 1 ; return(0) ; }
*/
/*
** Test For Intersection With pnt1pnt3
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Intersection With Pnt1-Pnt3") ;
 sd1 = bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) ;
 sd2 = bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) ;
 if( sd1 < 0 && sd2 > 0  )
   {
    bcdtmMath_normalIntersectCordLines(x1,y1,x2,y2,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,xdP,ydP) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"xdP = %12.5lf ydP = %12.5lf",*xdP,*ydP) ;
    d1 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y) ;
    d2 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) ;
    if( d1 <= d2 && d1 < dtmP->ppTol / 10.0 ) { *zdP = pointAddrP(dtmP,pnt1)->z ; *nxtPnt1P = pnt1 ; *Type = 1 ; return(0) ; }
    if( d2 <  d1 && d2 < dtmP->ppTol / 10.0 ) { *zdP = pointAddrP(dtmP,pnt3)->z ; *nxtPnt1P = pnt3 ; *Type = 1 ; return(0) ; }
    *nxtPnt1P = pnt1 ; *nxtPnt2P = pnt3 ; *Type = 2 ;
    bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*xdP,*ydP,zdP,*nxtPnt1P,*nxtPnt2P) ;
    if(( *nxtPnt3P = bcdtmList_nextClkDtmObject(dtmP,*nxtPnt1P,*nxtPnt2P)) < 0 ) return(4) ;
    return(0) ;
   }
/*
** Test For Intersection With pnt3pnt2
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Intersection With Pnt2-Pnt3") ;
 sd1 = bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) ;
 sd2 = bcdtmMath_sideOf(x1,y1,x2,y2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"sd1 = %2d sd2 = %2d",sd1,sd2) ;
 if( sd1 < 0 && sd2 > 0 )
   {
    bcdtmMath_normalIntersectCordLines(x1,y1,x2,y2,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,xdP,ydP) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"xdP = %12.5lf ydP = %12.5lf",*xdP,*ydP) ;
    d1 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y) ;
    d2 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y) ;
    if( d1 <= d2 && d1 < dtmP->ppTol / 10.0 ) { *zdP = pointAddrP(dtmP,pnt2)->z ; *nxtPnt1P = pnt2 ; *Type = 1 ; return(0) ; }
    if( d2 <  d1 && d2 < dtmP->ppTol / 10.0 ) { *zdP = pointAddrP(dtmP,pnt3)->z ; *nxtPnt1P = pnt3 ; *Type = 1 ; return(0) ; }
    *nxtPnt1P = pnt3 ; *nxtPnt2P = pnt2 ; *Type = 2 ;
    bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*xdP,*ydP,zdP,*nxtPnt1P,*nxtPnt2P) ;
    if(( *nxtPnt3P = bcdtmList_nextClkDtmObject(dtmP,*nxtPnt1P,*nxtPnt2P)) < 0 ) return(4) ;
    return(0) ;
   }
/*
** Precison Problem So Do A Fiddle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Precision Problem") ;
 closestDist  = 0.0 ;
 closestPoint = dtmP->nullPnt ;
 d1 = bcdtmMath_distanceOfPointFromLine(&onLine,x1,y1,x2,y2,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,&xn,&yn) ;
 if( onLine ) { closestPoint = pnt1 ; closestDist = d1 ;  }
 d1 = bcdtmMath_distanceOfPointFromLine(&onLine,x1,y1,x2,y2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,&xn,&yn) ;
 if( onLine && ( closestPoint == dtmP->nullPnt || d1 < closestDist ) )  { closestPoint = pnt2 ; closestDist = d1 ; }
 d1 = bcdtmMath_distanceOfPointFromLine(&onLine,x1,y1,x2,y2,pointAddrP(dtmP,pnt3)->x,pointAddrP(dtmP,pnt3)->y,&xn,&yn) ;
 if( onLine && ( closestPoint == dtmP->nullPnt || d1 < closestDist ) )  { closestPoint = pnt3 ; closestDist = d1 ; }
 if( closestPoint != dtmP->nullPnt )
   {
    *xdP = pointAddrP(dtmP,closestPoint)->x ;
    *ydP = pointAddrP(dtmP,closestPoint)->y ;
    *zdP = pointAddrP(dtmP,closestPoint)->z ;
    *nxtPnt1P = closestPoint ;
    *Type = 1 ; 
    return(0) ;     
   }
/*
** Job Completed
*/
 return(3) ;
}
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmDrape_getNextDrapePointFromPointDtmObject
(
 BC_DTM_OBJ *dtmP,
 double x1,double y1,double x2,double y2,
 long   *drapeTypeP,
 long   pnt1,
 long   *nxtPnt1P,long *nxtPnt2P,long *nxtPnt3P,
 double *xdP,double *ydP,double *zdP 
)
{
 long   p1,p2,clc ;
 double a1,a2,a3,d1,d2,z ;
 double drapeAng,pnt1Ang,pnt2Ang ;
/*
** Initialise
*/
 *xdP = *ydP = *zdP = 0.0 ;
/*
** Test For End Of Drape Line
*/
 if( x1 == x2 && y1 == y2 ) 
   { 
    *drapeTypeP = 1 ;
    *nxtPnt1P = pnt1 ;
    *xdP = pointAddrP(dtmP,pnt1)->x ; 
    *ydP = pointAddrP(dtmP,pnt1)->y ; 
    *zdP = pointAddrP(dtmP,pnt1)->z ; 
    return(1) ; 
   }
/*
** Calculate Drape Parameters
*/
 d1 = bcdtmMath_distance(x1,y1,x2,y2) ; 
 drapeAng = bcdtmMath_getAngle(x1,y1,x2,y2) ;
/*
**  Find Points connected to pnt1 that are either side of x1y1 x2y2
*/
 clc = nodeAddrP(dtmP,pnt1)->cPtr  ;
 p2  = clistAddrP(dtmP,clc)->pntNum ;
 if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,pnt1,p2)) < 0 ) return(0) ;
 if( x2 == pointAddrP(dtmP,p1)->x && y2 == pointAddrP(dtmP,p1)->y ) { *drapeTypeP = 1 ; *nxtPnt1P = p1 ; *nxtPnt2P = *nxtPnt3P = dtmP->nullPnt ; *xdP = pointAddrP(dtmP,p1)->x ; *ydP = pointAddrP(dtmP,p1)->y ; *zdP = pointAddrP(dtmP,p1)->z ; return(1) ; }
 pnt1Ang = bcdtmMath_getAngle(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
 while ( clc  != dtmP->nullPtr )
   {
    p2  = clistAddrP(dtmP,clc)->pntNum ;
    if( x2 == pointAddrP(dtmP,p2)->x && y2 == pointAddrP(dtmP,p2)->y ) { *drapeTypeP = 1 ; *nxtPnt1P = p2 ; *nxtPnt2P = *nxtPnt3P = dtmP->nullPnt ; *xdP = pointAddrP(dtmP,p2)->x ; *ydP = pointAddrP(dtmP,p2)->y ; *zdP = pointAddrP(dtmP,p2)->z ; return(1) ; }
    clc = clistAddrP(dtmP,clc)->nextPtr ;
    pnt2Ang = bcdtmMath_getAngle(pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
    if( nodeAddrP(dtmP,pnt1)->hPtr != p1 )
      {
       if( pnt1Ang == drapeAng ) 
         { 
          d2 = bcdtmMath_distance(x1,y1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
          if( d1 <= d2 ) { *drapeTypeP = 2 ; *nxtPnt1P = pnt1 ; *nxtPnt2P = p1 ;*xdP = x2 ; *ydP = y2 ; bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*xdP,*ydP,zdP,pnt1,p1) ; return(1) ; }
          else           { *drapeTypeP = 1 ; *nxtPnt1P = p1 ; *xdP = pointAddrP(dtmP,p1)->x ; *ydP = pointAddrP(dtmP,p1)->y ; *zdP = pointAddrP(dtmP,p1)->z ; return(0) ; }
         }
       else if( pnt2Ang == drapeAng ) 
         { 
          d2 = bcdtmMath_distance(x1,y1,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
          if( d1 <= d2 ) { *drapeTypeP = 2 ; *nxtPnt1P = pnt1 ; *nxtPnt2P = p2 ; *xdP = x2 ; *ydP = y2 ; bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*xdP,*ydP,zdP,pnt1,p2) ; return(1) ; }
          else           { *drapeTypeP = 1 ; *nxtPnt1P = p2 ; *xdP = pointAddrP(dtmP,p2)->x ; *ydP = pointAddrP(dtmP,p2)->y ; *zdP = pointAddrP(dtmP,p2)->z ; return(0) ; }
         }  

       else 
         {
          a1 = pnt1Ang  ; 
          a2 = drapeAng ; 
          a3 = pnt2Ang  ;
          if( a3 > a1 ) a1 = a1 + DTM_2PYE ; 
          if( a3 > a2 ) a2 = a2 + DTM_2PYE ; 
          if( a2 <= a1 && a2 >= a3 )
            {
/*
**           Test For Last Point In Triangle
*/
             if( bcdtmMath_linePointSideOfDtmObject(dtmP,p1,p2,x2,y2) <= 0 )
               {
                *drapeTypeP = 3 ;
                *nxtPnt1P = pnt1 ; *nxtPnt2P = p1 ; *nxtPnt3P = p2 ;
                *xdP = x2  ; *ydP = y2  ;
                bcdtmMath_interpolatePointOnTriangleDtmObject(dtmP,x2,y2,&z,*nxtPnt1P,*nxtPnt2P,*nxtPnt3P) ;
                *zdP = z ;
                return(1) ;
               }
            
/*
**           Intersect x1y1-x2y2 with p1,p2
*/
             bcdtmMath_normalIntersectCordLines(x1,y1,x2,y2,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,xdP,ydP) ;
             d1 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
             d2 = bcdtmMath_distance(*xdP,*ydP,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
             if( d1 <= d2 && d1 < dtmP->ppTol / 10.0 ) { *nxtPnt1P = p1 ; *drapeTypeP = 1 ; *zdP = pointAddrP(dtmP,p1)->z ; return(0) ;}
             if( d2 <  d1 && d2 < dtmP->ppTol / 10.0 ) { *nxtPnt1P = p2 ; *drapeTypeP = 1 ; *zdP = pointAddrP(dtmP,p2)->z ; return(0) ; }
             *nxtPnt1P = p2 ; *nxtPnt2P = p1 ; *drapeTypeP = 2 ;
             bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*xdP,*ydP,zdP,*nxtPnt1P,*nxtPnt2P) ;
             if( (*nxtPnt3P = bcdtmList_nextClkDtmObject(dtmP,*nxtPnt1P,*nxtPnt2P)) < 0 ) return(4) ;
             return(0) ;
            }
         }
      }
   p1 = p2 ;
   pnt1Ang = pnt2Ang ;
  }
/*
** Line Goes External To Tin Hull 
*/
 return(2) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmDrape_storeDrapePointWithDtmFeaturesDtmObject
(
 BC_DTM_OBJ *dtmP,
 long   lineNum,
 long   drapeType,
 double x,
 double y,
 double z,
 long   pnt1,
 long   pnt2,
 long   pnt3,
 long   dtmFeatureOption,
 bvector<DTMDrapePoint>& drapePts
) 
/*
** This Function Stores A Drape Point Into the DTMDrapePoint structure
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 bool voidFlag = false;
 long dtmFeature, breakPoint = 0;
 long numDrapeFeatures = 0;
 DTMDrapedLineCode newDrapeType;
 bvector<DTMTinPointFeatures> drapeFeatures;
 DTMTinPointFeatures *dfP;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Storing Drape Point") ;
    bcdtmWrite_message(0,0,0,"drapeType = %2ld ** pnt1 = %9ld pnt2 = %9ld pnt3 = %9ld",drapeType,pnt1,pnt2,pnt3) ;
   }
/*
**  Input Drape Type Values
**
**  drapeType == 0  Drape Point External To Tin 
**  drapeType == 1  Drape Point On Tin Point
**  drapeType == 2  Drape Point On Tin Line
**  drapeType == 3  Drape Point In Triangle
**
** 
** DTMDrapePoint drapeType Values   ==  0  Drape Point External To Tin 
**                                    ==  1  Drape Point In Triangle
**                                    ==  2  Drape Point On Break Line  
**                                    ==  3  Drape Point On Break Triangle Edge
**                                    ==  4  Drape Point In Void     
**                                    ==  5  Drape Point On Triangle Point
**                                    ==  6  Drape Point On Triangle Edge
*/

/*
** Initialise
*/
 if (drapeType == 0) newDrapeType = DTMDrapedLineCode::External;
 else if (drapeType == 1) newDrapeType = DTMDrapedLineCode::OnPoint;
 else if (drapeType == 2) newDrapeType = DTMDrapedLineCode::Edge;
 else if (drapeType == 3) newDrapeType = DTMDrapedLineCode::Tin;
/*
** Check For Drape Point In Void
*/
 if (newDrapeType != DTMDrapedLineCode::External)
   {
   if (drapeType == 1 && bcdtmFlag_testVoidBitPCWD (&nodeAddrP (dtmP, pnt1)->PCWD)) newDrapeType = DTMDrapedLineCode::Void;
    if( drapeType == 2 ) 
      {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt1,pnt2,voidFlag)) goto errexit ;
       if (voidFlag) newDrapeType = DTMDrapedLineCode::Void;
      } 
    if( drapeType == 3 )
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidFlag)) goto errexit ;
       if (voidFlag) newDrapeType = DTMDrapedLineCode::Void;
      }
   } 
/*
** If Drape Point Not External To Tin Or In Void Then Get Features And Point Type
*/
 if (newDrapeType != DTMDrapedLineCode::External && newDrapeType != DTMDrapedLineCode::Void)
   {
/*
** Get Dtm Features For Drape Point
*/
    if( dtmFeatureOption == TRUE )
      {
       if( drapeType == 1 )
         {
          if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,pnt1,drapeFeatures)) goto errexit ; 
         }
       if( drapeType == 2 )
         {
          if( bcdtmList_getDtmFeaturesForLineDtmObject(dtmP,pnt1,pnt2,drapeFeatures)) goto errexit ; 
         } 
       numDrapeFeatures = (long)drapeFeatures.size();
      }
/*
**  Check If Drape Point Is A Break Point
*/
    breakPoint = 0 ;
    if( dtmFeatureOption == TRUE )
      {
      for (dfP = drapeFeatures.data(); dfP < drapeFeatures.data() + numDrapeFeatures && !breakPoint; ++dfP)
         {
          if( dfP->dtmFeatureType == DTMFeatureType::Breakline ) breakPoint = 1 ;
         }
      }
    if( dtmFeatureOption == FALSE )
      {
       if( drapeType == 1 ) breakPoint = bcdtmList_testForBreakPointDtmObject(dtmP,pnt1) ;
       if( drapeType == 2 ) breakPoint = bcdtmList_testForBreakLineDtmObject(dtmP,pnt1,pnt2) ;
      }
/*
**  Set New Drape Type
*/
    if (breakPoint) newDrapeType = DTMDrapedLineCode::Breakline;
/*
**  Test For Point On Break Triangle
*/
    if (newDrapeType != DTMDrapedLineCode::Breakline && newDrapeType != DTMDrapedLineCode::Tin)
      {
       if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,pnt1,&dtmFeature) &&
           bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,pnt2,&dtmFeature)     )
         { 
          if(( pnt3 = bcdtmList_nextClkDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
          if( bcdtmList_testLineDtmObject(dtmP,pnt3,pnt2) )
            {
            if (bcdtmList_testForPointOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Breakline, pnt3, &dtmFeature)) newDrapeType = DTMDrapedLineCode::BetweenBreaklines;
            }
          if (newDrapeType != DTMDrapedLineCode::BetweenBreaklines)
            {
             if(( pnt3 = bcdtmList_nextAntDtmObject(dtmP,pnt1,pnt2)) < 0 ) goto errexit ;
             if( bcdtmList_testLineDtmObject(dtmP,pnt3,pnt2) )
               {
               if (bcdtmList_testForPointOnDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Breakline, pnt3, &dtmFeature)) newDrapeType = DTMDrapedLineCode::BetweenBreaklines;
               }
            }
         } 
      }
   }
/*
** Store Drape Point
*/
 size_t numDrapePts = drapePts.size();
 drapePts.resize(numDrapePts + 1);
 auto& drapePt = drapePts[numDrapePts];
 drapePt.drapeLine = lineNum ;
 drapePt.drapeType = newDrapeType ;
 drapePt.drapePt.x = x   ;
 drapePt.drapePt.y = y   ;
 drapePt.drapePt.z = z   ;
 drapePt.drapeFeatures.swap(drapeFeatures) ;
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
BENTLEYDTM_Public int bcdtmDrape_findClosestLineInterceptWithHullDtmObject
(
 BC_DTM_OBJ *dtmP,
 double startX,
 double startY,
 double endX,
 double endY,
 long   *fndTypeP,
 long   *pnt1P,
 long   *pnt2P,
 double *xIntP,
 double *yIntP,
 double *zIntP
)
/*
** This Routine Find the Closeset Hull Line Intercept With Line [startX,startY]-[endX,endY]
**
** Return Values == 0  Success
**               == 1  Error Terminate
**
** fndTypeP == 0  No Intercept
**          == 1  Intesects With Hull Point pnt1P
**          == 2  Intesects With Hull Line  pnt1P-pnt2P 
**          == 3  Pulled Onto Next Hull Point 
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,sp1,sp2,sdof1,sdof2,onLine1,onLine2,process,firstFound=1,scanHull=FALSE ;
 double n1,n2,xc,yc,dist,tolerance=0,xmin,ymin,xmax,ymax,xlmin,ylmin,xlmax,ylmax ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Finding Closest Line Intercept With Tin Hull") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %P",dtmP) ; 
    bcdtmWrite_message(0,0,0,"startX     = %14.6lf startY = %14.6lf",startX,startY) ;
    bcdtmWrite_message(0,0,0,"endX       = %14.6lf endY   = %14.6lf",endX,endY) ;
    bcdtmWrite_message(0,0,0,"*fndTypeP  = %8ld",*fndTypeP) ; 
    bcdtmWrite_message(0,0,0,"*pnt1P     = %8ld",*pnt1P) ; 
    bcdtmWrite_message(0,0,0,"*pnt2P     = %8ld",*pnt2P) ; 
    if( *pnt1P != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"pnt1P = %9ld  fTableP = %8ld ** %10.4lf %10.4lf %10.4lf",*pnt1P,nodeAddrP(dtmP,*pnt1P)->hPtr,pointAddrP(dtmP,*pnt1P)->x,pointAddrP(dtmP,*pnt1P)->y,pointAddrP(dtmP,*pnt1P)->z) ; 
    if( *pnt2P != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"pnt2P = %9ld  fTableP = %8ld ** %10.4lf %10.4lf %10.4lf",*pnt2P,nodeAddrP(dtmP,*pnt2P)->hPtr,pointAddrP(dtmP,*pnt2P)->x,pointAddrP(dtmP,*pnt2P)->y,pointAddrP(dtmP,*pnt2P)->z) ; 
   }
/*
** Initialiase
*/
 *fndTypeP = 0   ;
 sp1 = *pnt1P ; sp2 = *pnt2P   ;
 *pnt1P = *pnt2P = dtmP->nullPnt ;
 *xIntP = *yIntP = *zIntP = 0.0 ;
/*
** Test For Hull Intercept From A Single Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Hull Intercept From A Single Point") ;
 if( sp1 != dtmP->nullPnt && nodeAddrP(dtmP,sp1)->hPtr != dtmP->nullPnt )
   {
    if( sp2 == dtmP->nullPnt )
      {
       p2 = nodeAddrP(dtmP,sp1)->hPtr ;
       if( ( p1 = bcdtmList_nextClkDtmObject(dtmP,sp1,p2)) < 0 ) goto errexit ;
      }
    else { p1 = sp1 ; p2 = sp2 ; }
    n1 = bcdtmMath_distanceOfPointFromLine(&onLine1,startX,startY,endX,endY,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,&xc,&yc) ;
    n2 = bcdtmMath_distanceOfPointFromLine(&onLine2,startX,startY,endX,endY,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,&xc,&yc) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"onLine1 = %2ld n1 = %20.15lf",onLine1,n1) ;    
    if( dbg ) bcdtmWrite_message(0,0,0,"onLine2 = %2ld n2 = %20.15lf",onLine2,n2) ;    
    if( onLine1 && n1 <= dtmP->plTol ) { *fndTypeP = 3 ; *pnt1P = p1 ; *xIntP = pointAddrP(dtmP,p1)->x ; *yIntP = pointAddrP(dtmP,p1)->y ;*zIntP = pointAddrP(dtmP,p1)->z ; }
    if( onLine2 && n2 <= dtmP->plTol ) { *fndTypeP = 3 ; *pnt1P = p2 ; *xIntP = pointAddrP(dtmP,p2)->x ; *yIntP = pointAddrP(dtmP,p2)->y ;*zIntP = pointAddrP(dtmP,p2)->z ; }
    if( *fndTypeP != 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"fndTypeP = %2ld ** pnt1P = %6ld",*fndTypeP,*pnt1P) ;
       n1 = bcdtmMath_distance(startX,startY,endX,endY) ;
       n2 = bcdtmMath_distance(pointAddrP(dtmP,*pnt1P)->x,pointAddrP(dtmP,*pnt1P)->y,endX,endY) ;
       if( dbg ) 
         {
          bcdtmWrite_message(0,0,0,"n1 = %20.12lf",n1) ;    
          bcdtmWrite_message(0,0,0,"n2 = %20.12lf",n2) ; 
          bcdtmWrite_message(0,0,0,"Included Angle = %15.12lf",bcdtmMath_calculateIncludedAngle(startX,startY,pointAddrP(dtmP,*pnt1P)->x,pointAddrP(dtmP,*pnt1P)->y,endX,endY)) ;  
         }
       if( n1 <= n2 || bcdtmMath_calculateIncludedAngle(startX,startY,pointAddrP(dtmP,*pnt1P)->x,pointAddrP(dtmP,*pnt1P)->y,endX,endY) < DTM_PYE / 2.0 ) *fndTypeP = 0 ;
       if( *fndTypeP != 0 ) return(0) ;
      } 
   }
/*
** Set Scan Parameters
*/
 if( startX <= endX ) { xlmin = startX ; xlmax = endX   ; }
 else                 { xlmin = endX   ; xlmax = startX ; }
 if( startY <= endY ) { ylmin = startY ; ylmax = endY   ; }
 else                 { ylmin = endY   ; ylmax = startY ; }
/*
** Write Out Bounding Rectangle For Drape Line
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Drape Line ** xmin = %12.5lf ymin = %12.5lf ** xmax = %12.5lf ymax = %12.5lf",xlmin,ylmin,xlmax,ylmax) ;
    bcdtmWrite_message(0,0,0,"DTM        ** xmin = %12.5lf ymin = %12.5lf ** xmax = %12.5lf ymax = %12.5lf",dtmP->xMin,dtmP->yMin,dtmP->xMax,dtmP->yMax) ;
    bcdtmWrite_message(0,0,0,"startX = %12.5lf startY = %12.5lf",startX,startY) ;
    bcdtmWrite_message(0,0,0,"endX   = %12.5lf endY   = %12.5lf",endX,endY) ;
   }
/*
** Scan Hull Looking For Closest Line Intercept
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Hull Intercept For Closest Intercept") ;
 p1 = dtmP->hullPoint ; 
 do
   {
    process = 0 ;
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
    if     ( sp1 == dtmP->nullPnt && sp2 == dtmP->nullPnt )                           process = 1 ;
    else if( sp1 != dtmP->nullPnt && sp2 == dtmP->nullPnt && p1 != sp1 && p2 != sp1 ) process = 1 ;
    else if( sp1 != dtmP->nullPnt && sp2 != dtmP->nullPnt && p1 != sp1              ) process = 1 ;
    if( process )
      {
       if( pointAddrP(dtmP,p1)->x <= pointAddrP(dtmP,p2)->x ) { xmin = pointAddrP(dtmP,p1)->x ; xmax = pointAddrP(dtmP,p2)->x ; }
       else                                                   { xmin = pointAddrP(dtmP,p2)->x ; xmax = pointAddrP(dtmP,p1)->x ; }
       if( pointAddrP(dtmP,p1)->y <= pointAddrP(dtmP,p2)->y ) { ymin = pointAddrP(dtmP,p1)->y ; ymax = pointAddrP(dtmP,p2)->y ; }
       else                                                   { ymin = pointAddrP(dtmP,p2)->y ; ymax = pointAddrP(dtmP,p1)->y ; }
       if( xmin <= xlmax && xmax >= xlmin && ymin <= ylmax && ymax >= ylmin ) 
         {
          sdof1 = bcdtmMath_sideOf(startX,startY,endX,endY,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
          sdof2 = bcdtmMath_sideOf(startX,startY,endX,endY,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
          if( sdof1 != sdof2 ) 
            {
             sdof1 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,startX,startY) ;
             sdof2 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,endX,endY) ;
             if( sdof1 != sdof2 )
               {
                bcdtmMath_normalIntersectCordLines(startX,startY,endX,endY,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,&xc,&yc) ;
                dist = bcdtmMath_distance(startX,startY,xc,yc) ;
                if( firstFound || dist < tolerance  )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"p1 = %8ld p2 = %8ld ** xc = %12.5lf yc = %12.5lf  ** dist %12.5lf",p1,p2,xc,yc,dist) ;
                   firstFound = 0 ;
                   tolerance = dist ;
                   *xIntP = xc ; *yIntP = yc ;
                   *pnt1P = p1 ; *pnt2P = p2 ; *fndTypeP = 2 ;
                   if( *xIntP == pointAddrP(dtmP,p1)->x && *yIntP == pointAddrP(dtmP,p1)->y ) { *fndTypeP = 1 ; *pnt2P = dtmP->nullPnt ; }
                   if( *xIntP == pointAddrP(dtmP,p2)->x && *yIntP == pointAddrP(dtmP,p2)->y ) { *fndTypeP = 1 ; *pnt1P = *pnt2P ; *pnt2P = dtmP->nullPnt ; }
                  }
               }
            }
         }
      }
    p1 = p2 ;
   } while ( p1 != dtmP->hullPoint ) ;
/*
** If No Intercept Found , Check If A Hull Point Is Within Point To Point Tolerance Of Drape Line
** This Code Added 14/6/2002 At Jay's Request
*/
 if( ! *fndTypeP && scanHull== TRUE )
   {
/*
**  Set Search Bounding Rectangle Around Drape Line
*/
    xlmin = xlmin - dtmP->ppTol * 2.0 ;
    xlmax = xlmax + dtmP->ppTol * 2.0 ;
    ylmin = ylmin - dtmP->ppTol * 2.0 ;
    ylmax = ylmax + dtmP->ppTol * 2.0 ;
/*
**  Scan Hull Points
*/
    p1 = dtmP->hullPoint ; 
    do
      {
/*
**     Check For Point Within Bounding Rectangle
*/
       if( pointAddrP(dtmP,p1)->x >= xlmin && pointAddrP(dtmP,p1)->x <= xlmax && pointAddrP(dtmP,p1)->y >= ylmin && pointAddrP(dtmP,p1)->y <= ylmax )
         {
          n1 = bcdtmMath_distanceOfPointFromLine(&onLine1,startX,startY,endX,endY,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,&xc,&yc) ;
          if( n1 <= dtmP->ppTol )
            {  
             dist = bcdtmMath_distance(startX,startY,xc,yc) ;
             if( firstFound || dist < tolerance  )
               {
                firstFound = 0 ;
                tolerance = dist ;
                *xIntP = pointAddrP(dtmP,p1)->x ; 
                *yIntP = pointAddrP(dtmP,p1)->y ;
                *pnt1P = p1 ; *pnt2P = dtmP->nullPnt ; 
                *fndTypeP = 1 ;
               }
            }
         }
       p1 = nodeAddrP(dtmP,p1)->hPtr ;
      } while ( p1 != dtmP->hullPoint ) ;
   }
/*
** Set z Value
*/
 if( *fndTypeP )
   {
    if( *fndTypeP == 1 ) *zIntP = pointAddrP(dtmP,*pnt1P)->z ;
    else                 bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*xIntP,*yIntP,zIntP,*pnt1P,*pnt2P) ; 
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
BENTLEYDTM_EXPORT int  bcdtmDrape_pointReturnAttributesDtmFile
(
 WCharCP dtmFileP,
 double x,
 double y,
 double *z,
 long *drapeFlagP,
 DPoint3d dtmPoints[],
 double *slopeDegreesP,
 double *slopePercentP,
 double *aspectP,
 double *heightP
)
/*
**
**  drapeFlagP
**
**    <==  0  Point Outside Tin Object
**    <==  1  Point Inside  Tin Object
**    <==  2  Point In Void  
**    <==  3  Point On Tin Line Or Point
**     
**  Return Values
**
**     ==> 0  Success
**     ==> 1  Error 
**     
*/
{
 int ret=DTM_SUCCESS ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Test If Requested Dtm File Is Current Dtm Object
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) 
   { 
    if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
    goto errexit ;
   }
/*
**  Drape Point
*/
 if( bcdtmDrape_pointReturnAttributesDtmObject(dtmP,x,y,z,drapeFlagP,dtmPoints,slopeDegreesP,slopePercentP,aspectP,heightP) ) goto errexit ;
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
BENTLEYDTM_EXPORT int  bcdtmDrape_pointReturnAttributesDtmObject
(
 BC_DTM_OBJ *dtmP,
 double x,
 double y,
 double *z,
 long *drapeFlagP,
 DPoint3d dtmPoints[],
 double *slopeDegreesP,
 double *slopePercentP,
 double *aspectP,
 double *heightP
)
/*
**  Note That Triangle Attribute Are Only Set For 
**
**  drapeFlagP
**
**    <==  0  Point Outside Tin Object
**    <==  1  Point Inside  Tin Object - In Triangle
**    <==  2  Point In Void  
**    <==  3  Point On Tin Line Or Point
** 
**  Note That Triangle Attribute Are Only Set For drapeFlagP = 1 
**    
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long P1,P2,P3,fndType,startTime=0 ;
 bool voidFlag = false;
 DPoint3d *pnt1P,*pnt2P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Draping Point Returning Attributes") ;
    bcdtmWrite_message(0,0,0,"Dtm Object      = %p",dtmP) ; 
    bcdtmWrite_message(0,0,0,"x               = %12.5lf",x) ; 
    bcdtmWrite_message(0,0,0,"y               = %12.5lf",y) ; 
    bcdtmWrite_message(0,0,0,"dtmP->dtmState  = %12ld",dtmP->dtmState) ;
    startTime = bcdtmClock() ;
   }
/*
** Initialise 
*/
 *z = 0.0  ;
 *drapeFlagP = 0 ;
 *slopeDegreesP = *slopePercentP = *aspectP = *heightP = 0.0 ;
 dtmPoints[0].x = dtmPoints[0].y = dtmPoints[0].z = 0.0 ;
 dtmPoints[1].x = dtmPoints[1].y = dtmPoints[1].z = 0.0 ;
 dtmPoints[2].x = dtmPoints[2].y = dtmPoints[2].z = 0.0 ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
**  Find Triangle
**
** Note :- fndType ( Point Find Type ) Return Values
**
**  == 0   Point External To Dtm
**  == 1   Point Coincident with Point pnt1P
**  == 2   Point On Line pnt1-Ppnt2P
**  == 3   Point On Hull Line pnt1P-pnt2P
**  == 4   Point In Triangle pnt1P-pnt2P-pnt3P
**
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,z,&fndType,&P1,&P2,&P3 ) ) goto errexit  ;
 if( dbg ) bcdtmWrite_message(0,0,0,"fndType = %2ld",fndType) ;
/*
** If Point External Test If Point Within ppTol Of Hull Line Or PoinT
*/
 if( fndType == 0 )
   {
    bcdtmDrape_findClosestHullLineDtmObject(dtmP,x,y,&P1,&P2) ;
    if( P1 != dtmP->nullPnt && P2 != dtmP->nullPnt ) 
      {
       pnt1P = pointAddrP(dtmP,P1) ;
       pnt2P = pointAddrP(dtmP,P2) ;
       if( bcdtmMath_distance(x,y,pnt1P->x,pnt1P->y) <= dtmP->ppTol ) { fndType = 1 ; P2 = dtmP->nullPnt ; *z = pnt1P->z ; }
       else if( bcdtmMath_distance(x,y,pnt2P->x,pnt2P->y) <= dtmP->ppTol ) { fndType = 1 ; P1 = P2 ; P2 = dtmP->nullPnt ; *z = pnt2P->z ; }
       else if( bcdtmMath_normalDistanceToLineDtmObject(dtmP,P1,P2,x,y) <= dtmP->ppTol )
         {
          fndType = 3 ; 
          bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,z,P1,P2) ;
         }
      }   
   }
/*
** If Point Internal To Tin Hull Set Attributes
*/
 if( fndType ) 
   {
/*
**  Test For Point In Void
*/
    *drapeFlagP = 1 ;
    if( fndType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P1)->PCWD) ) *drapeFlagP = 2 ;
    if( fndType == 2 || fndType == 3 ) 
      {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,P1,P2,voidFlag)) goto errexit  ;
       if( voidFlag ) *drapeFlagP = 2 ;
      }
    if( fndType == 4 ) 
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,P2,P3,voidFlag)) goto errexit  ;
       if( voidFlag ) *drapeFlagP = 2 ;
      } 
/*
**  Set Return Values In Drape Point Not In A Void
*/
    if( ! voidFlag ) 
      {
/*
**     Drape Point On Tin Line Or Tin Point
*/
       if( fndType >= 1 &&  fndType <= 3 ) 
         {
          dtmPoints[0].x = pointAddrP(dtmP,P1)->x ;
          dtmPoints[0].y = pointAddrP(dtmP,P1)->y ;
          dtmPoints[0].z = pointAddrP(dtmP,P1)->z ;
          if( P2 != dtmP->nullPnt ) 
            { 
             dtmPoints[1].x = pointAddrP(dtmP,P2)->x ;
             dtmPoints[1].y = pointAddrP(dtmP,P2)->y ;
             dtmPoints[1].z = pointAddrP(dtmP,P2)->z ; 
            }
          *drapeFlagP = 3 ;
         }
/*
**     Set Triangle Attributes
*/
       if( fndType == 4 )
         {
          if( bcdtmMath_getTriangleAttributesDtmObject(dtmP,P1,P2,P3,slopeDegreesP,slopePercentP,aspectP,heightP)) goto errexit  ;
          dtmPoints[0].x = pointAddrP(dtmP,P1)->x ; dtmPoints[0].y =pointAddrP(dtmP,P1)->y ; dtmPoints[0].z = pointAddrP(dtmP,P1)->z ;
          dtmPoints[1].x = pointAddrP(dtmP,P2)->x ; dtmPoints[1].y =pointAddrP(dtmP,P2)->y ; dtmPoints[1].z = pointAddrP(dtmP,P2)->z ;
          dtmPoints[2].x = pointAddrP(dtmP,P3)->x ; dtmPoints[2].y =pointAddrP(dtmP,P3)->y ; dtmPoints[2].z = pointAddrP(dtmP,P3)->z ;
          *drapeFlagP = 1 ;
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
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Drape Point = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Draping Point Returning Attributes Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Draping Point Returning Attributes Error") ;
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
BENTLEYDTM_Public int bcdtmDrape_findClosestLineInterceptWithVoidHullDtmObject
(
 BC_DTM_OBJ *dtmP,
 double      startX,
 double      startY,
 double      endX,
 double      endY,
 long        *findTypeP,
 long        *pnt1P,
 long        *pnt2P,
 double      *intXP,
 double      *intYP,
 double      *intZP
)
/*
** This Routine Finds the Closeset Void Hull Intercept With The Line
** Joining  <startX,startY> To <endX,endY>
**
** Return Values == 0  Success
**               == 1  Error 
**
** findTypeP == 0  No Intercept
**           == 1  Intesects With Void Hull Point pnt1P
**           == 2  Intesects With Void Hull Line  pnt1P-pnt2P 
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sp,np,sd1,sd2,dtmFeature,firstIntersect,intersectFnd ;
 double x=0.0,y=0.0,dd=0.0,ds=0.0,dn=0.0,closestDistance=0 ;
 double xmin,ymin,xmax,ymax,xlmin,ylmin,xlmax,ylmax ;
 BC_DTM_FEATURE  *fP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Finding Closest Line Intercept With A Void Hull") ;
    bcdtmWrite_message(0,0,0,"startX = %14.6lf startY = %14.6lf",startX,startY) ;
    bcdtmWrite_message(0,0,0,"endX   = %14.6lf endY   = %14.6lf",endX,endY) ;
   }
/*
** Initialise
*/
 *findTypeP = 0 ;
 *pnt1P = *pnt2P = dtmP->nullPnt ;
 *intXP = *intYP = *intZP = 0.0  ;
 firstIntersect = 1 ;
/*
** Set Boundary Rectangle For Intersect Line
*/
 if( startX <= endX ) { xlmin = startX ; xlmax = endX   ; }
 else                 { xlmin = endX   ; xlmax = startX ; }
 if( startY <= endY ) { ylmin = startY ; ylmax = endY   ; }
 else                 { ylmin = endY   ; ylmax = startY ; }
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Intersect Line Bounding Rectangle Min ** x = %12.5lf y = %12.5lf",xlmin,ylmin) ;
    bcdtmWrite_message(0,0,0,"Intersect Line Bounding Rectangle Max ** x = %12.5lf y = %12.5lf",xlmax,ylmax) ;
   }
/*
** Scan Void Features Looking For Closest Line Intercept
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Void Hulls For Closest Intercept") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ; 
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Void )
      {
       sp = fP->dtmFeaturePts.firstPoint ;
/*
**     Scan Void Hull Looking For Intersection
*/
       do
         { 
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np)) goto errexit ;
/*
**        Set Bounding Rectangle For Void Hull Line
*/ 
          if( dbg == 2 )
            {
             bcdtmWrite_message(0,0,0,"spX = %12.5lf spY = %12.5lf",pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y) ;
             bcdtmWrite_message(0,0,0,"npX = %12.5lf npY = %12.5lf",pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y) ;
            }
          if( pointAddrP(dtmP,sp)->x <= pointAddrP(dtmP,np)->x ) { xmin = pointAddrP(dtmP,sp)->x ; xmax = pointAddrP(dtmP,np)->x ; }
          else                                                   { xmin = pointAddrP(dtmP,np)->x ; xmax = pointAddrP(dtmP,sp)->x ; }
          if( pointAddrP(dtmP,sp)->y <= pointAddrP(dtmP,np)->y ) { ymin = pointAddrP(dtmP,sp)->y ; ymax = pointAddrP(dtmP,np)->y ; }
          else                                                   { ymin = pointAddrP(dtmP,np)->y ; ymax = pointAddrP(dtmP,sp)->y ; }
          if( dbg == 2 )
            {
             bcdtmWrite_message(0,0,0,"Void Hull Line Bounding Rectangle Min ** x = %12.5lf y = %12.5lf",xmin,ymin) ;
             bcdtmWrite_message(0,0,0,"Void Hull Line Bounding Rectangle Max ** x = %12.5lf y = %12.5lf",xmax,ymax) ;
            }
/*
**        Check If Bounding Rectangles For Intersect And Void Hull Lines Overlap
*/
          if( xmin <= xlmax && xmax >= xlmin && ymin <= ylmax && ymax >= ylmin ) 
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Bounding Rectangles Overlap") ;
/*
**           Check If The Intersect And Void Hull Lines Intersect
*/
             intersectFnd = 0 ;
             sd1 = bcdtmMath_sideOf(startX,startY,endX,endY,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y) ;
             sd2 = bcdtmMath_sideOf(startX,startY,endX,endY,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y) ;
/*
**           Colinear Lines - Check For Overlap
*/ 
             if( sd1 == 0 && sd2 == 0 )
               {
                dd = bcdtmMath_distance(startX,startY,endX,endY) ;
                ds = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y) ;
                dn = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y) ;
                if( ds <= dn && ds <= dd )
                  {
                   dd = ds ;
                   x  = pointAddrP(dtmP,sp)->x ;
                   y  = pointAddrP(dtmP,sp)->y ;
                   intersectFnd = 1 ;
                  }
                if( dn <  ds && dn <= dd )
                  {
                   dd = dn ;
                   x  = pointAddrP(dtmP,np)->x ;
                   y  = pointAddrP(dtmP,np)->y ;
                   intersectFnd = 1 ;
                  }
               }
/*
**           Lines May Intersect
*/
             else if( sd1 != sd2 ) 
               {
                sd1 = bcdtmMath_sideOf(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,startX,startY) ;
                sd2 = bcdtmMath_sideOf(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,endX,endY) ;
/*
**              Colinear Lines - Check For Overlap
*/ 
                if( sd1 == 0 && sd2 == 0 )
                  {
                   dd = bcdtmMath_distance(startX,startY,endX,endY) ;
                   ds = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y) ;
                   dn = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y) ;
                   if( ds <= dn && ds < dd )
                     {
                      dd = ds ;
                      x  = pointAddrP(dtmP,sp)->x ;
                      y  = pointAddrP(dtmP,sp)->y ;
                      intersectFnd = 1 ;
                     }
                   if( dn <  ds && dn < dd )
                     {
                      dd = dn ;
                      x  = pointAddrP(dtmP,np)->x ;
                      y  = pointAddrP(dtmP,np)->y ;
                      intersectFnd = 1 ;
                     }
                  } 
/*
**              Lines Intersect
*/
                else if( sd1 != sd2 ) 
                  {
                   bcdtmMath_normalIntersectCordLines(startX,startY,endX,endY,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,&x,&y) ;
                   dd = bcdtmMath_distance(startX,startY,x,y) ;
                   intersectFnd = 1 ;
                  }
               }
/*
**           Intersection Found Update Closest Point
*/
             if( intersectFnd )
               {
                if( firstIntersect || dd < closestDistance )
                  {
                   firstIntersect  = 0  ;
                   closestDistance = dd ;
                   *intXP = x ; 
                   *intYP = y ;
                   *pnt1P = sp ; 
                   *pnt2P = np ; 
                   *findTypeP = 2 ;
                   if( *intXP == pointAddrP(dtmP,sp)->x && *intYP == pointAddrP(dtmP,sp)->y ) 
                     { 
                      *findTypeP = 1 ; 
                      *pnt2P = dtmP->nullPnt ; 
                     }
                   if( *intXP == pointAddrP(dtmP,np)->x && *intYP == pointAddrP(dtmP,np)->y ) 
                     { 
                      *findTypeP = 1 ; 
                      *pnt1P = *pnt2P ;
                      *pnt2P = dtmP->nullPnt ;
                     }
                  }
               }
            }
          sp = np ;
         } while ( sp != fP->dtmFeaturePts.firstPoint ) ;
      }
   }
/*
** Set z Value
*/
 if( *findTypeP )
   {
    if( *findTypeP == 1 ) *intZP = pointAddrP(dtmP,*pnt1P)->z ;
    else                 bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*intXP,*intYP,intZP,*pnt1P,*pnt2P) ; 
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
BENTLEYDTM_EXPORT int bcdtmDrape_pointWithOffHullToleranceDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double offHullTolerance,double *z,long *drapeFlagP)
/*
**
**  drapeFlagP
**
**    <==  0  Point Outside Tin Object
**    <==  1  Point Inside  Tin Object
**    <==  2  Point In Void  
**     
**  Return Values
**
**     ==> 0  Success
**     ==> 1  Error 
**     
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long pnt1, pnt2, pnt3, fndType;
 bool voidFlag;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Draping Point On Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x    = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y    = %12.5lf",y) ;
   } 
/*
** Initialise 
*/
 *z = 0.0  ;
 *drapeFlagP = 0 ;
/*
**  Find Triangle
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,z,&fndType,&pnt1,&pnt2,&pnt3 ) ) goto errexit ;
/*
** If Point External Test If Point Within ppTol Of Hull Line 
*/
 if( ! fndType )
   {
    bcdtmDrape_findClosestHullLineDtmObject(dtmP,x,y,&pnt1,&pnt2) ;
    if( pnt1 != dtmP->nullPnt && pnt2 != dtmP->nullPnt ) 
      {
       if( bcdtmMath_normalDistanceToLineDtmObject(dtmP,pnt1,pnt2,x,y) <= offHullTolerance )
         {
          fndType = 1 ; *drapeFlagP = 1 ;
          bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,z,pnt1,pnt2) ;
         }
      }   
   }
/*
** Test For Point In Void
*/
 else
   {
    *drapeFlagP = 1 ;
    if( fndType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,pnt1)->PCWD) ) *drapeFlagP = 2 ;
    if( fndType == 2 || fndType == 3 ) 
      {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt1,pnt2,voidFlag)) goto errexit ;
       if( voidFlag ) *drapeFlagP = 2 ;
      }
    if( fndType == 4 ) 
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidFlag)) goto errexit ;
       if( voidFlag ) *drapeFlagP = 2 ;
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
BENTLEYDTM_EXPORT int bcdtmDrape_pointCopyParallelDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double *z,long *drapeFlagP)
/*
**
**  Special Drape Function For Site Modeler Copy Parallel Operation
**  If The Point is External To Tin Will Snap To Closest Tin Hull Line 
**
**  DrapeFlag
**
**    <==  0  Point Outside Tin Object
**    <==  1  Point Inside  Tin Object
**    <==  2  Point In Void  
**     
**  Return Values
**
**     ==> 0  Success
**     ==> 1  Error 
**     
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long pnt1, pnt2, pnt3, fndType;
 bool voidFlag;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Draping Point On Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x    = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y    = %12.5lf",y) ;
   } 
/*
** Initialise 
*/
 *z = 0.0  ;
 *drapeFlagP = 0 ;
/*
**  Find Triangle
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,z,&fndType,&pnt1,&pnt2,&pnt3 ) ) goto errexit ;
/*
** If Point External Test If Point Within ppTol Of Hull Line 
*/
 if( ! fndType )
   {
    bcdtmEdit_findClosestHullLineDtmObject(dtmP,x,y,&pnt1,&pnt2) ;
    if( pnt1 != dtmP->nullPnt && pnt2 != dtmP->nullPnt ) 
      {
       fndType = 1 ; *drapeFlagP = 1 ;
       bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,z,pnt1,pnt2) ;
      }
    goto cleanup ;
   }
/*
** Test For Point In Void
*/
 else
   {
    *drapeFlagP = 1 ;
    if( fndType == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,pnt1)->PCWD) ) *drapeFlagP = 2 ;
    if( fndType == 2 || fndType == 3 ) 
      {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt1,pnt2,voidFlag)) goto errexit ;
       if( voidFlag ) *drapeFlagP = 2 ;
      }
    if( fndType == 4 ) 
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidFlag)) goto errexit ;
       if( voidFlag ) *drapeFlagP = 2 ;
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
BENTLEYDTM_EXPORT int bcdtmDrape_spiralOnDtmFile (WCharCP dtmFileP, double R1, double R2, double SpiralLength, double Offset, double Xs, double Ys, double Xe, double Ye, double Xi, double Yi, DTMDrapedLineCode **SpiralPointType, DPoint3d **SpiralPoints, long *NumSpiralPoints)
/*
** This Function Drapes A Spiral Onto A dtmP File
*/
{
 int ret=DTM_SUCCESS ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Test If Requested dtmP File Is Current dtmP Object
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ; 
/*
** Call Object Module
*/
 if( bcdtmDrape_spiralOnDtmObject(dtmP,R1,R2,SpiralLength,Offset,Xs,Ys,Xe,Ye,Xi,Yi,SpiralPointType,SpiralPoints,NumSpiralPoints)) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmDrape_spiralOnDtmObject (BC_DTM_OBJ *dtmP, double R1, double R2, double spiralLength, double Offset, double Xs, double Ys, double Xe, double Ye, double Xi, double Yi, DTMDrapedLineCode **spiralPointTypePP, DPoint3d **spiralPointsPP, long *numSpiralPointsP)
/*
** This Function Drapes A Spiral On A Dtm Object
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs,endFlag ;
 DTMDrapedLineCode* pp1;
 long   numSpiralPts=0,memSpiralPts=0,memSpiralPtsInc=1000,numDrapePts ;
 double Sx,Sy,Nx,Ny,spiralInc,spiralLen,spiralStrokeTolerance=1000.0 ;
 DPoint3d    *p3dP,*spiralPtsP=NULL ;
 DTMDrapePoint *drapeP;
 bvector<DTMDrapePoint> drapePtsP;
/*
** Write Status Message
*/ 
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Draping Spiral On Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP Object      = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Spiral length    = %12.4lf",spiralLength) ;
    bcdtmWrite_message(0,0,0,"Xs               = %12.4lf",Xs) ;
    bcdtmWrite_message(0,0,0,"Ys               = %12.4lf",Ys) ;
    bcdtmWrite_message(0,0,0,"Xe               = %12.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Ye               = %12.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Xi               = %12.4lf",Xi) ;
    bcdtmWrite_message(0,0,0,"Yi               = %12.4lf",Yi) ;
    bcdtmWrite_message(0,0,0,"Offset           = %12.4lf",Offset) ;
    bcdtmWrite_message(0,0,0,"spiralPointTypePP  = %p",*spiralPointTypePP) ;
    bcdtmWrite_message(0,0,0,"spiralPointsPP     = %p",*spiralPointsPP) ;
    bcdtmWrite_message(0,0,0,"numSpiralPointsP  = %4ld",*numSpiralPointsP) ;
   }
/*
** Initialise Return values
*/
 *numSpiralPointsP  = 0    ;
 if( *spiralPointsPP    != NULL ) { free(*spiralPointsPP)    ; *spiralPointsPP    = NULL ; }
 if( *spiralPointTypePP != NULL ) { free(*spiralPointTypePP) ; *spiralPointTypePP = NULL ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(0,0,0,"Method Requires Triangulated Dtm") ;
    goto errexit ;
   }
/*
** Initialise Function Variables
*/
 spiralInc = spiralLength / spiralStrokeTolerance ;
 if( spiralInc < dtmP->ppTol * 100.0 ) spiralInc = dtmP->ppTol * 100.0 ;
 if( Offset == 0.0 ) { Sx = Xs ; Sy = Ys ; }
 else                bcdtmDrape_getCoordiantesFromLength(&Sx,&Sy,R1,R2,spiralLength,0.0001,Offset,Xs,Ys,Xi,Yi,Xe,Ye) ;
/*
** Allocate Memory For Stroked Spiral Points
*/
 memSpiralPts = memSpiralPtsInc ;
 spiralPtsP = ( DPoint3d * ) malloc(memSpiralPts*sizeof(DPoint3d)) ;
 if( spiralPtsP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Store First Spiral Point
*/
 numSpiralPts = 0 ;
 (spiralPtsP+numSpiralPts)->x = Sx  ;
 (spiralPtsP+numSpiralPts)->y = Sy  ;
 (spiralPtsP+numSpiralPts)->z = 0.0 ;
 ++numSpiralPts ;
/*
** Get Stroked Spiral Points
*/
 endFlag = 0 ;
 spiralLen = 0.0 ;
 while ( spiralLen < spiralLength )
   {
/*
**  Get Coordinates Of Next Spiral Stoke Point
*/
    spiralLen = spiralLen + spiralInc ;
    if( spiralLen >= spiralLength ) { spiralLen = spiralLength ; endFlag = 1 ; }
/*
**  Bug In Sum's Code When spiralLen == spiralLength So Offset spiralLen marginally From End
*/
    if( endFlag ) spiralLen = spiralLen - 0.0001 ;
    bcdtmDrape_getCoordiantesFromLength(&Nx,&Ny,R1,R2,spiralLength,spiralLen,Offset,Xs,Ys,Xi,Yi,Xe,Ye) ;
/*
**  Check Memory
*/
    if( numSpiralPts == memSpiralPts )
      {
       memSpiralPts = memSpiralPts + memSpiralPtsInc ;
       spiralPtsP = ( DPoint3d * ) realloc(spiralPtsP,memSpiralPts*sizeof(DPoint3d)) ;
       if( spiralPtsP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Store Stoke Point
*/
    (spiralPtsP+numSpiralPts)->x = Nx  ;
    (spiralPtsP+numSpiralPts)->y = Ny  ;
    (spiralPtsP+numSpiralPts)->z = 0.0 ;
    ++numSpiralPts ;

    if( endFlag ) break;
   }
/*
**  Drape Stroked Spiral Points On Tin
*/
 if( bcdtmDrape_stringDtmObject(dtmP,spiralPtsP,numSpiralPts,FALSE,drapePtsP)) goto errexit ;
 numDrapePts = (long)drapePtsP.size();
/*
** Allocate Memory For Return Arrays
*/
 *numSpiralPointsP   = numDrapePts ;
 *spiralPointsPP     = (DPoint3d *) malloc( *numSpiralPointsP * sizeof(DPoint3d)) ;
 *spiralPointTypePP = (DTMDrapedLineCode *)malloc (*numSpiralPointsP * sizeof(DTMDrapedLineCode));
 if( *spiralPointsPP == NULL || spiralPointTypePP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Copy Drape Points To Return Arrays
*/
 for( ofs = 0 , drapeP = drapePtsP.data() ; drapeP < drapePtsP.data() + numDrapePts ; ++ofs , ++drapeP )
   {
    (*spiralPointsPP+ofs)->x  = drapeP->drapePt.x ;
    (*spiralPointsPP+ofs)->y  = drapeP->drapePt.y ;
    (*spiralPointsPP+ofs)->z  = drapeP->drapePt.z ;
    *(*spiralPointTypePP+ofs) = drapeP->drapeType ;
   }
/*
** Write Out Drape Points For Development Purpose
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Spirallength = %10.4lf Interpolated Length = %10.4lf",spiralLength,spiralLen) ; 
    spiralLen = 0.0 ;
    bcdtmWrite_message(0,0,0,"Number Of Spiral Drape Points = %4ld",*numSpiralPointsP) ;
    for( pp1 = *spiralPointTypePP , p3dP = *spiralPointsPP ; pp1 < *spiralPointTypePP + *numSpiralPointsP ; ++pp1 , ++p3dP )
      {
       if( p3dP < *spiralPointsPP + *numSpiralPointsP - 1 ) spiralLen = spiralLen + bcdtmMath_distance(p3dP->x,p3dP->y,(p3dP+1)->x,(p3dP+1)->y) ; 
       bcdtmWrite_message(0,0,0,"Type = %2ld  x = %12.4lf y = %12.4lf  z = %10.4lf",*pp1,p3dP->x,p3dP->y,p3dP->z) ;
      } 
    bcdtmWrite_message(0,0,0,"Spiral Length = %10.4lf",spiralLen) ; 
   }
/*
** Clean Up
*/
 cleanup :
 if( spiralPtsP != NULL ) free(spiralPtsP) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *spiralPointsPP    != NULL ) { free(*spiralPointsPP)    ; *spiralPointsPP    = NULL ; }
 if( *spiralPointTypePP != NULL ) { free(*spiralPointTypePP) ; *spiralPointTypePP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmDrape_arcOnDtmFile (WCharCP DtmFile, double Sx, double Sy, double Ex, double Ey, double Cx, double Cy, double A1, double A2, double Ap, double As, double Ra, DTMDrapedLineCode **ArcPointType, DPoint3d **ArcPoints, long *NumArcPoints)
/*
** This Function Drapes An Arc Onto A dtmP File
*/
{
 int ret=DTM_SUCCESS ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Initialise Variables
*/
 *ArcPointType = NULL ;
 *ArcPoints    = NULL ;
 *NumArcPoints = 0    ;
/*
** Test If Requested dtmP File Is Current dtmP Object
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,DtmFile)) goto errexit ;
/*
** Call Object Module
*/
 if( bcdtmDrape_arcOnDtmObject(dtmP,Sx,Sy,Ex,Ey,Cx,Cy,A1,A2,Ap,As,Ra,ArcPointType,ArcPoints,NumArcPoints)) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmDrape_arcOnDtmObject (BC_DTM_OBJ *dtmP, double Sx, double Sy, double Ex, double Ey, double Cx, double Cy, double A1, double A2, double Ap, double As, double Ra, DTMDrapedLineCode **arcPointTypePP, DPoint3d **arcPointsPP, long *numArcPointsP)
/*
** This Function Drapes An Arc On A Dtm Object
**
**   dtmP     = dtmP Object
**   Sx,Sy   = Start Cordinates Of Arc
**   Ex,Ey   = End Coordinates Of Arc
**   Cx,Cy   = Centre Coordinates Of Arc
**   A1,A2   = Start Angle And Sweep Angle
**   Ap,As   = Primary And Secondary Axis Of Arc
**   Ra      = Rotation Angle 
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs,ArcDirection,numDrapePts ;
 DTMDrapedLineCode* pp1;
 double  A3,Aa,Rs,Px,Py,Tx,Ty,Nx,Ny  ;
 long    numArcPts=0,memArcPts=0,memArcPtsInc=1000 ;
 DPoint3d    *p3dP,*arcPtsP=NULL ;
 bvector<DTMDrapePoint> drapePtsP;
/*
 double d1,dn,ang,angn,Ix,Iy;
 long isw ;
*/
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Draping Arc On dtmP Object") ;
    bcdtmWrite_message(0,0,0,"dtmP Object      = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Sx              = %12.4lf",Sx) ;
    bcdtmWrite_message(0,0,0,"Sy              = %12.4lf",Sy) ;
    bcdtmWrite_message(0,0,0,"Ex              = %12.4lf",Ex) ;
    bcdtmWrite_message(0,0,0,"Ey              = %12.4lf",Ey) ;
    bcdtmWrite_message(0,0,0,"Cx              = %12.4lf",Cx) ;
    bcdtmWrite_message(0,0,0,"Cy              = %12.4lf",Cy) ;
    bcdtmWrite_message(0,0,0,"A1              = %12.4lf",A1) ;
    bcdtmWrite_message(0,0,0,"A2              = %12.4lf",A2) ;
    bcdtmWrite_message(0,0,0,"Ap              = %12.4lf",Ap) ;
    bcdtmWrite_message(0,0,0,"As              = %12.4lf",As) ;
    bcdtmWrite_message(0,0,0,"Ra              = %12.4lf",Ra) ;
    bcdtmWrite_message(0,0,0,"arcPointTypePP    = %p",*arcPointTypePP) ;
    bcdtmWrite_message(0,0,0,"arcPointsPP       = %p",*arcPointsPP) ;
    bcdtmWrite_message(0,0,0,"numArcPointsP    = %4ld",*numArcPointsP) ;
   }
/*
** Initialise Return Variables
*/
 *numArcPointsP = 0    ;
 if( *arcPointsPP    != NULL ) { free(*arcPointsPP)    ; *arcPointsPP    = NULL ; }
 if( *arcPointTypePP != NULL ) { free(*arcPointTypePP) ; *arcPointTypePP = NULL ; }
/*
** Check For Valid Dtm Object
*/
  if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
  if( dtmP->dtmState != DTMState::Tin )
    {
     bcdtmWrite_message(0,0,0,"Method Requires Triangulated Dtm") ;
     goto errexit ;
    }
/*
** Initialise Arc  Variables
*/
 Ap = Ap * 2.0   ;
 As = As * 2.0   ;
 A1 = A1 / 360.0 * DTM_2PYE ;
 A2 = A2 / 360.0 * DTM_2PYE ;
 Ra = Ra / 360.0 * DTM_2PYE ; 
/*
** Set Arc Stroking Tolerance
*/
 if( A2 >= 0.0 ) Rs =  0.001 ;
 else            Rs = -0.001 ; 
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
 if( dbg )
   {
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
   }
/*
** Negate Arc Parameters If Necessary
*/
 if( ArcDirection )  { Ra = -Ra ; Rs = - Rs ; A2 = -A2 ; } 
/*
** Allocate Memory For Stroked Arc Points
*/
 memArcPts = memArcPtsInc ;
 arcPtsP = ( DPoint3d * ) malloc(memArcPts*sizeof(DPoint3d)) ;
 if( arcPtsP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Store First Arc Point
*/
 numArcPts = 0 ;
 (arcPtsP+numArcPts)->x = Sx  ;
 (arcPtsP+numArcPts)->y = Sy  ;
 (arcPtsP+numArcPts)->z = 0.0 ;
 ++numArcPts ;
/*
** Get Stroked Arc Points
*/
 Aa = A1 ; 
 A3 = A1 +  A2  ;
 while ( Aa != A3  )
   {
/*
**  Get Coordinates Of Next Arc Stoke Point
*/
    if( fabs(A3 - (Aa + Rs)) > fabs(Rs) ) Aa = Aa + Rs ;
    else                                  Aa = A3 ; 
    bcdtmInterpolate_getPointOnArc(&Px,&Py,Ap,As,Aa) ;
    bcdtmInterpolate_rotateAndTranslatePoint(Px,Py,Tx,Ty,Ra,&Nx,&Ny) ;
/*
**  Check Memory
*/
    if( numArcPts == memArcPts )
      {
       memArcPts = memArcPts + memArcPtsInc ;
       arcPtsP = ( DPoint3d * ) realloc(arcPtsP,memArcPts*sizeof(DPoint3d)) ;
       if( arcPtsP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Store Stoke Point
*/
    (arcPtsP+numArcPts)->x = Nx  ;
    (arcPtsP+numArcPts)->y = Ny  ;
    (arcPtsP+numArcPts)->z = 0.0 ;
    ++numArcPts ;
   }
/*
**  Drape Stroked Arc Points On Tin
*/
 if( bcdtmDrape_stringDtmObject(dtmP,arcPtsP,numArcPts,FALSE,drapePtsP)) goto errexit ;
 numDrapePts = (long)drapePtsP.size();
/*
** Allocate Memory For Return Arrays
*/
 *numArcPointsP   = numDrapePts ;
 *arcPointsPP     = (DPoint3d *) malloc( *numArcPointsP * sizeof(DPoint3d)) ;
 *arcPointTypePP = (DTMDrapedLineCode *)malloc (*numArcPointsP * sizeof(DTMDrapedLineCode));
 if( *arcPointsPP == NULL || arcPointTypePP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Copy Drape Points To Return Arrays
*/
 ofs = 0;
 for( auto& drapeP : drapePtsP)
   {
    (*arcPointsPP+ofs)->x = drapeP.drapePt.x ;
    (*arcPointsPP+ofs)->y = drapeP.drapePt.y ;
    (*arcPointsPP+ofs)->z = drapeP.drapePt.z ;
    *(*arcPointTypePP+ofs) = drapeP.drapeType ;
    ofs++;
   }
/*
** Write Out Drape Points For Development Purpose
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Arc Drape Points = %4ld",*numArcPointsP) ;
    for( pp1 = *arcPointTypePP , p3dP = *arcPointsPP ; pp1 < *arcPointTypePP + *numArcPointsP ; ++pp1 , ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Type = %2ld  x = %12.4lf y = %12.4lf  z = %10.4lf",*pp1,p3dP->x,p3dP->y,p3dP->z) ;
      } 
   }
/*
** Clean Up
*/
 cleanup :
 if( arcPtsP   != NULL ) free(arcPtsP) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( *arcPointsPP    != NULL ) { free(*arcPointsPP)    ; *arcPointsPP    = NULL ; }
 if( *arcPointTypePP != NULL ) { free(*arcPointTypePP) ; *arcPointTypePP = NULL ; }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmDrape_testForPointInBoundingCubeDtmObject
(
 BC_DTM_OBJ *dtmP,              // ==> Pointer To DTM Object
 DPoint3d        *pointP             // ==> Point 
)
{
 if( pointP->x < dtmP->xMin || pointP->x > dtmP->xMax ) return(0) ;
 if( pointP->y < dtmP->yMin || pointP->y > dtmP->yMax ) return(0) ;
 if( pointP->z < dtmP->zMin || pointP->z > dtmP->zMax ) return(0) ;
 return(1) ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmDrape_intersectSurfaceDtmObject
(
 BC_DTM_OBJ *dtmP,                   // ==> Pointer To DTM Object
 DPoint3d    *startPointP,                // ==> Vector Start Point 
 DPoint3d    *endPointP,                  // ==> Vector End Point
 long   *intersectTypeP,             // <== Intersect Type ** 0 = No Intersection, 1=Tin Point, 2= Tin Line , 3 = Triangle
 DPoint3d    *surfacePointP,              // <== Surface Point 
 long   *trgPnt1P,                   // <== Tin Point Index
 long   *trgPnt2P,                   // <== Tin Point Index
 long   *trgPnt3P,                   // <== Tin Point Index
 bool&   voidFlag                    // <== Set To One If Surface Point In Void Otherwise Zero
) 
/*
** This Function Intersects A Vector Defind By A Start Point And End Point With The Tin Surface
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long pnt1, pnt2, pnt3, process, fndType;
 double nx,ny,xls,yls,zls,xle,yle,xMin,yMin,xMax,yMax ;
 double dx,dy,dz,nzs,nzl,pzs,pzl,zdn,zdp ;
 DPoint3d vectorPoint ;
/*
** Write Entry Message
*/
 if( dbg == 1 ) 
   {
    bcdtmWrite_message(0,0,0,"Intersecting Surface With Vector") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPointP->x  = %12.5lf",startPointP->x) ;
    bcdtmWrite_message(0,0,0,"startPointP->y  = %12.5lf",startPointP->y) ;
    bcdtmWrite_message(0,0,0,"startPointP->z  = %12.5lf",startPointP->z) ;
    bcdtmWrite_message(0,0,0,"endPointP->x    = %12.5lf",endPointP->x) ;
    bcdtmWrite_message(0,0,0,"endPointP->y    = %12.5lf",endPointP->y) ;
    bcdtmWrite_message(0,0,0,"endPointP->z    = %12.5lf",endPointP->z) ;
   } 
/*
** Initialise
*/
 *intersectTypeP = 0 ;
 surfacePointP->x = 0.0 ;
 surfacePointP->y = 0.0 ;
 surfacePointP->z = 0.0 ;
 *trgPnt1P = DTM_NULL_PNT ;
 *trgPnt2P = DTM_NULL_PNT ;
 *trgPnt3P = DTM_NULL_PNT ;
 voidFlag = false ;
 zdn = 1.0 ;
 zdp = 1.0 ;
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    
/*
** Check DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
 /*
  * If this is a zero Length vector just extend the z range.
  */
 if ( startPointP->x == endPointP->x && startPointP->y == endPointP->y && startPointP->z == endPointP->z)
     {
     if (!bcdtmDrape_testForPointInBoundingCubeDtmObject(dtmP,startPointP))
         goto cleanup;
     }
 else
     {
/*
** Extend Start Point Out If It Is In Bounding Cube
*/
     while( bcdtmDrape_testForPointInBoundingCubeDtmObject(dtmP,startPointP))
       {
        if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Extending Out Start Point") ;
        vectorPoint.x = ( startPointP->x + endPointP->x ) / 2.0 ;
        vectorPoint.y = ( startPointP->y + endPointP->y ) / 2.0 ;
        vectorPoint.z = ( startPointP->z + endPointP->z ) / 2.0 ;
        dx = startPointP->x - vectorPoint.x ;
        dy = startPointP->y - vectorPoint.y ;
        dz = startPointP->z - vectorPoint.z ;
        startPointP->x = vectorPoint.x + dx * 10.0 ;
        startPointP->y = vectorPoint.y + dy * 10.0 ;
        startPointP->z = vectorPoint.z + dz * 10.0 ;
       }
/*
** Extend End Point Out If It Is In Bounding Cube
*/
     while( bcdtmDrape_testForPointInBoundingCubeDtmObject(dtmP,endPointP))
       {
        if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Extending Out End Point") ;
        vectorPoint.x = ( startPointP->x + endPointP->x ) / 2.0 ;
        vectorPoint.y = ( startPointP->y + endPointP->y ) / 2.0 ;
        vectorPoint.z = ( startPointP->z + endPointP->z ) / 2.0 ;
        dx = endPointP->x - vectorPoint.x ;
        dy = endPointP->y - vectorPoint.y ;
        dz = endPointP->z - vectorPoint.z ;
        endPointP->x = vectorPoint.x + dx * 10.0 ;
        endPointP->y = vectorPoint.y + dy * 10.0 ;
        endPointP->z = vectorPoint.z + dz * 10.0 ;
       }
     }
/*
**  Write Modified Start Points
*/
 if( dbg == 1 )
   {  
    bcdtmWrite_message(0,0,0,"Extended Vector End Points") ;
    bcdtmWrite_message(0,0,0,"startPointP->x  = %12.5lf",startPointP->x) ;
    bcdtmWrite_message(0,0,0,"startPointP->y  = %12.5lf",startPointP->y) ;
    bcdtmWrite_message(0,0,0,"startPointP->z  = %12.5lf",startPointP->z) ;
    bcdtmWrite_message(0,0,0,"endPointP->x    = %12.5lf",endPointP->x) ;
    bcdtmWrite_message(0,0,0,"endPointP->y    = %12.5lf",endPointP->y) ;
    bcdtmWrite_message(0,0,0,"endPointP->z    = %12.5lf",endPointP->z) ;
   }
/*
** Check For Overlap Of Line
*/
 xMin = xMax = startPointP->x ;
 yMin = yMax = startPointP->y ;
 if( endPointP->x < xMin ) xMin = endPointP->x ; 
 if( endPointP->x > xMax ) xMax = endPointP->x ; 
 if( endPointP->y < yMin ) yMin = endPointP->y ; 
 if( endPointP->y > yMax ) yMax = endPointP->y ; 
/*
** Only Calculate If Line Is Internal Or Overlaps Bounding Rectangle
*/
 if( xMin <= dtmP->xMax && xMax >= dtmP->xMin && yMin <= dtmP->yMax && yMax >= dtmP->yMin )
   {
    xls = startPointP->x ; yls = startPointP->y ; zls = startPointP->z ;
    xle = endPointP->x   ; yle = endPointP->y ;
    pzs = startPointP->z ;
    nzl = endPointP->z ;
/*
**  Check If Start Point Is Within Tin Hull
*/
    if( bcdtmFind_triangleForPointDtmObject(dtmP,xls,yls,&zls,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"00 fndType = %2ld",fndType) ;
    if( fndType >= 3 ) fndType = fndType - 1 ;
/*
**  If Start Point Not Inside Tin Hull Find Intersect Of Line With Tin Hull
*/
    if( ! fndType  ) 
      {
       if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject(dtmP,xls,yls,xle,yle,&fndType,&pnt1,&pnt2,&xls,&yls,&zls) ) goto errexit ;
       if( fndType == 3 ) fndType = 1 ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"01 fndType = %2ld ** xls = %12.5lf yls = %12.5lf zls = %12.5lf",fndType,xls,yls,zls) ;
     }
/*
**  Intersect Surface With Vector
*/
    if( fndType )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"02 fndType = %2ld",fndType) ;
       pzs = zls ;
       bcdtmMath_interpolatePointOnLine(startPointP->x,startPointP->y,startPointP->z,endPointP->x,endPointP->y,endPointP->z,xls,yls,&pzl) ;
       process = 1 ;
       while ( process )
        {
         if( bcdtmDrape_getNextSurfaceInterceptDtmObject(dtmP,xls,yls,xle,yle,&fndType,&pnt1,&pnt2,&pnt3,&nx,&ny,&nzs)) goto errexit ;
         if( dbg == 1 ) bcdtmWrite_message(0,0,0,"fndType = %2ld ** nx = %12.5lf ny = %12.5lf nzs = %12.5lf",fndType,nx,ny,nzs) ;
/*
**       Shot Vector Goes External To Tin Hull
*/
         if( fndType == 5 ) 
           {
            if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Shot Vector Goes External To Tin Hull") ;
            if( nodeAddrP(dtmP,pnt1)->hPtr != pnt2 )
              {
               pnt3 = pnt1 ;
               pnt1 = pnt2 ;
               pnt2 = pnt3 ;
               pnt3 = dtmP->nullPnt ;
              }
            if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject(dtmP,xls,yls,xle,yle,&fndType,&pnt1,&pnt2,&xls,&yls,&pzs) ) goto errexit ;
            if( fndType == 3 ) fndType = 1 ;
            if( dbg == 2 ) bcdtmWrite_message(0,0,0,"03 fndType = %2ld ** xls = %12.5lf yls = %12.5lf zls = %12.5lf",fndType,xls,yls,zls) ;
            process = 0 ;
            if( fndType )
              {
               process = 1 ;
               bcdtmMath_interpolatePointOnLine(startPointP->x,startPointP->y,startPointP->z,endPointP->x,endPointP->y,endPointP->z,xls,yls,&pzl) ;
               zdp = zdn = 1.0 ;
              } 
            else fndType = 5 ;
           }
/*
**       Shot Vector End Point In Triangle
*/
         else if( fndType == 4 ) 
           {
            zdp = pzl - pzs ;
            zdn = nzl - nzs ;
            process = 0 ;
           } 
/*
**       Shot Vector Intersects Triangle Point Or Edge 
*/
         else
           {
            bcdtmMath_interpolatePointOnLine(startPointP->x,startPointP->y,startPointP->z,endPointP->x,endPointP->y,endPointP->z,nx,ny,&nzl) ;
            zdp = pzl - pzs ;
            zdn = nzl - nzs ;
            if( zdp == 0.0 || zdn == 0.0 || ( zdp > 0.0 && zdn < 0.0 ) || ( zdp < 0.0 && zdn > 0.0 )) process = 0 ;
            else 
              { 
               xls = nx ; 
               yls = ny ; 
               zls = nzs ;
               pzs = nzs ;
               pzl = nzl ; 
              } 
           }
        }
/*
**    Calculate Surface Intersection Point
*/
      if( fndType != 5 )    //  Terminates On Tin Hull
        { 
         if( dbg == 2  ) bcdtmWrite_message(0,0,0,"zdp = %12.5lf zdn = %12.5lf",zdp,zdn) ;
         if( zdp == 0.0 || zdn == 0.0 || ( zdp > 0.0 && zdn < 0.0 ) || ( zdp < 0.0 && zdn > 0.0 )) 
           { 
            if( zdp == 0.0 ) 
              {
               surfacePointP->x = xls ; 
               surfacePointP->y = yls ; 
               surfacePointP->z = zls ; 
              }
            else if( zdn == 0.0 ) 
              { 
               surfacePointP->x = nx ;
               surfacePointP->y = ny ; 
               surfacePointP->z = nzl ;
              }
            else
              { 
               if( zdn < 0.0 ) zdn = -zdn ;
               if( zdp < 0.0 ) zdp = -zdp ;
               surfacePointP->x = xls + ( nx  - xls  ) * zdp / (zdp+zdn) ;
               surfacePointP->y = yls + ( ny  - yls  ) * zdp / (zdp+zdn) ;
               surfacePointP->z = zls + ( nzl - zls  ) * zdp / (zdp+zdn) ;
              } 
/*
**          Get Triangle For Surface Point
*/
            if( bcdtmFind_triangleForPointDtmObject(dtmP,surfacePointP->x,surfacePointP->y,&nzs,intersectTypeP,trgPnt1P,trgPnt2P,trgPnt3P)) goto errexit ; 
            if( *intersectTypeP >= 3 ) *intersectTypeP = *intersectTypeP - 1 ;
/*
**          Set Void Flag For Surface Point
*/
            if( *intersectTypeP == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,*trgPnt1P)->PCWD) ) voidFlag = true ;
            if( *intersectTypeP == 2 ) 
              {
               if( bcdtmList_testForVoidLineDtmObject(dtmP,*trgPnt1P,*trgPnt2P,voidFlag)) goto errexit ;
              }
            if( *intersectTypeP == 3 ) 
              {
               if( bcdtmList_testForVoidTriangleDtmObject(dtmP,*trgPnt1P,*trgPnt2P,*trgPnt3P,voidFlag)) goto errexit ;
              }
           }
        }
     }
  }
/*
** Report Intersect Type
*/
 if( dbg )
   {
    if( ! *intersectTypeP ) bcdtmWrite_message(0,0,0,"Vector Does Not Intersect Surface") ;
    else                    bcdtmWrite_message(0,0,0,"Vector Intersects Surface") ;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Surface With Vector Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Surface With Vector Error") ;
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
BENTLEYDTM_EXPORT int bcdtmDrape_intersectTriangleDtmObject
(
 BC_DTM_OBJ *dtmP,                   // ==> Pointer To DTM Object
 DPoint3d    *startPoint,                 // ==> Vector Start Point 
 DPoint3d    *endPoint,                   // ==> Vector End Point
 long   *intersectTypeP,             // <== Intersect Type ** 0 = No Intersection, 1=Tin Point, 2= Tin Line , 3 = Triangle
 DPoint3d    *surfacePointP,              // <== Surface Point 
 DPoint3d    tinPoints[],                 // <== Triangle Points
 bool& voidFlag                   // <== Set To One If Surface Point In Void Otherwise Zero
) 
{
 int  ret=DTM_SUCCESS ;
 long trgPnt1;
 long trgPnt2;
 long trgPnt3;
/*
** Intersect Surface
*/
 if( bcdtmDrape_intersectSurfaceDtmObject(dtmP, startPoint, endPoint, intersectTypeP, surfacePointP, &trgPnt1, &trgPnt2, &trgPnt3, voidFlag) != DTM_SUCCESS ) goto errexit ;
/*
** Set Triangle Coordinates
*/
 if( trgPnt1 != dtmP->nullPnt )  tinPoints[0] = *(DPoint3d*)pointAddrP(dtmP, trgPnt1);
 if( trgPnt2 != dtmP->nullPnt )  tinPoints[1] = *(DPoint3d*)pointAddrP(dtmP, trgPnt2);
 if( trgPnt3 != dtmP->nullPnt )  tinPoints[2] = *(DPoint3d*)pointAddrP(dtmP, trgPnt3);
/*
** Cleanup
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
BENTLEYDTM_Private int bcdtmDrape_getNextSurfaceInterceptDtmObject
(
 BC_DTM_OBJ *dtmP,
 double startX,
 double startY,
 double endX,
 double endY,
 long   *intersectTypeP,
 long   *trgPnt1P,
 long   *trgPnt2P,
 long   *trgPnt3P,
 double *nextXP,
 double *nextYP, 
 double *nextZP 
)
/*
** This Function Gets The Next Triangle Intercept with Line startX-startY  endX-endY
** Return Values For Intersect Type
** 
**      1  -  Intersects Tin Point
**      2  -  Intersects Tin Line
**      3  -  Intersects Tin Triangle
**      4  -  Terminates In Triangle
**      5  -  Terminates On Tin Hull
**  
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long pnt,antPnt,clkPnt,sp1,sp2,sp3,clPtr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Next Surface Intercept Point") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startX         = %12.5lf",startX) ;
    bcdtmWrite_message(0,0,0,"startY         = %12.5lf",startY) ;
    bcdtmWrite_message(0,0,0,"endX           = %12.5lf",endX) ;
    bcdtmWrite_message(0,0,0,"endY           = %12.5lf",endY) ;
    bcdtmWrite_message(0,0,0,"intersectTypeP = %8ld",*intersectTypeP) ;
   } 
/*
** Initialise Varaibles
*/
 *nextXP = *nextYP = *nextZP = 0.0 ;
/*
** Get Next Intercept on Basis Of Last Intercept
*/
 switch( *intersectTypeP )
   {
    case 1 :  /* Last Intercept Was Point trgPnt1P */
/*
**  Find Points connected to trgPnt1P that are either side of  Line
*/
      clPtr  = nodeAddrP(dtmP,*trgPnt1P)->cPtr  ;
      clkPnt = clistAddrP(dtmP,clPtr)->pntNum ;
      if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,*trgPnt1P,clkPnt)) < 0 ) goto errexit ;
      while ( clPtr  != dtmP->nullPtr )
        {
         clkPnt = clistAddrP(dtmP,clPtr)->pntNum ;
         clPtr  = clistAddrP(dtmP,clPtr)->nextPtr ;
/*
**       Check For End Point In Triangle
*/
         if( bcdtmList_testLineDtmObject(dtmP,clkPnt,antPnt) )
           {
            if( bcdtmMath_pointInTriangleDtmObject(dtmP,*trgPnt1P,antPnt,clkPnt,endX,endY))
              {
               *nextXP = endX ;
               *nextYP = endY ;
               *trgPnt2P = antPnt ;
               *trgPnt3P = clkPnt ;
               bcdtmMath_interpolatePointOnTriangleDtmObject(dtmP,*nextXP,*nextYP,nextZP,*trgPnt1P,*trgPnt2P,*trgPnt3P) ;
               *intersectTypeP =  4 ;
               goto cleanup ;  
              }  
           }
/*
**       Get Intersection On Triangle Edge
*/
         if( bcdtmList_testLineDtmObject(dtmP,clkPnt,antPnt) && bcdtmMath_pointSideOfDtmObject(dtmP,clkPnt,antPnt,*trgPnt1P) > 0 )
           {
            sp1 = bcdtmMath_sideOf(pointAddrP(dtmP,*trgPnt1P)->x,pointAddrP(dtmP,*trgPnt1P)->y,endX,endY,pointAddrP(dtmP,clkPnt)->x,pointAddrP(dtmP,clkPnt)->y) ;
            sp2 = bcdtmMath_sideOf(pointAddrP(dtmP,*trgPnt1P)->x,pointAddrP(dtmP,*trgPnt1P)->y,endX,endY,pointAddrP(dtmP,antPnt)->x,pointAddrP(dtmP,antPnt)->y) ;
            sp3 = bcdtmMath_sideOf(pointAddrP(dtmP,clkPnt)->x,pointAddrP(dtmP,clkPnt)->y,pointAddrP(dtmP,antPnt)->x,pointAddrP(dtmP,antPnt)->y,endX,endY) ;
            if( sp1 <= 0 && sp2 >= 0 && sp3 <= 0 )
              {
               if( sp1 == 0 ){ *intersectTypeP = 1 ; *trgPnt1P = clkPnt ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,clkPnt)->x ; *nextYP = pointAddrP(dtmP,clkPnt)->y ; *nextZP = pointAddrP(dtmP,clkPnt)->z ; goto cleanup  ; }
               if( sp2 == 0 ){ *intersectTypeP = 1 ; *trgPnt1P = antPnt ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,antPnt)->x ; *nextYP = pointAddrP(dtmP,antPnt)->y ; *nextZP = pointAddrP(dtmP,antPnt)->z ; goto cleanup  ; }
               bcdtmMath_normalIntersectCordLines(pointAddrP(dtmP,clkPnt)->x,pointAddrP(dtmP,clkPnt)->y,pointAddrP(dtmP,antPnt)->x,pointAddrP(dtmP,antPnt)->y,startX,startY,endX,endY,nextXP,nextYP) ;
               if( pointAddrP(dtmP,clkPnt)->x == *nextXP && pointAddrP(dtmP,clkPnt)->y == *nextYP )
                 { *intersectTypeP = 1 ; *trgPnt1P = clkPnt ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,clkPnt)->x ; *nextYP = pointAddrP(dtmP,clkPnt)->y ; *nextZP = pointAddrP(dtmP,clkPnt)->z ; goto cleanup  ; }
               if( pointAddrP(dtmP,antPnt)->x == *nextXP && pointAddrP(dtmP,antPnt)->y == *nextYP )
                 { *intersectTypeP = 1 ; *trgPnt1P = antPnt ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,antPnt)->x ; *nextYP = pointAddrP(dtmP,antPnt)->y ; *nextZP = pointAddrP(dtmP,antPnt)->z ; goto cleanup  ; }
               bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*nextXP,*nextYP,nextZP,antPnt,clkPnt) ;
               *intersectTypeP = 2 ; *trgPnt1P = antPnt ; *trgPnt2P = clkPnt ; *trgPnt3P = dtmP->nullPnt ;
               goto cleanup  ;
              }
           }
         antPnt = clkPnt ;
        }
      *intersectTypeP = 5 ; 
      goto cleanup   ;
    break  ;

    case 2 :  /* Last Intercept Was Line trgPnt1P-trgPnt2P */
/*
**    Check For Going External
*/
      sp1 = *trgPnt1P ;
      sp2 = *trgPnt2P ;
      if( nodeAddrP(dtmP,sp1)->hPtr == sp2 )
        {
         sp3 = bcdtmMath_sideOf(pointAddrP(dtmP,sp1)->x,pointAddrP(dtmP,sp1)->y,pointAddrP(dtmP,sp2)->x,pointAddrP(dtmP,sp2)->y,endX,endY) ;
         if( sp3 <  0 ) 
           {
            *intersectTypeP = 5 ; 
            goto cleanup   ;
           }
        } 
      if( nodeAddrP(dtmP,sp2)->hPtr == sp1 )
        {
         sp3 = bcdtmMath_sideOf(pointAddrP(dtmP,sp2)->x,pointAddrP(dtmP,sp2)->y,pointAddrP(dtmP,sp1)->x,pointAddrP(dtmP,sp1)->y,endX,endY) ;
         if( sp3 <  0 ) 
           {
            *intersectTypeP = 5 ; 
            goto cleanup   ;
           }
        } 
/*
**    Calculate Intersection On Opposite Side Of Triangle
*/
      antPnt = clkPnt = pnt = dtmP->nullPnt ;
      if( ( antPnt = bcdtmList_nextAntDtmObject(dtmP,*trgPnt1P,*trgPnt2P)) < 0 ) goto errexit ;
      if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,*trgPnt1P,*trgPnt2P)) < 0 ) goto errexit ;
      if( ! bcdtmList_testLineDtmObject(dtmP,antPnt,*trgPnt2P) ) antPnt = dtmP->nullPnt ;
      if( ! bcdtmList_testLineDtmObject(dtmP,clkPnt,*trgPnt2P) ) clkPnt = dtmP->nullPnt ;
      sp1 = bcdtmMath_sideOf(pointAddrP(dtmP,*trgPnt1P)->x,pointAddrP(dtmP,*trgPnt1P)->y,pointAddrP(dtmP,*trgPnt2P)->x,pointAddrP(dtmP,*trgPnt2P)->y,endX,endY) ;
      if( sp1 == 0 ) 
        {
         if( bcdtmMath_distance(endX,endY,pointAddrP(dtmP,*trgPnt1P)->x,pointAddrP(dtmP,*trgPnt1P)->y) <= bcdtmMath_distance(endX,endY,pointAddrP(dtmP,*trgPnt2P)->x,pointAddrP(dtmP,*trgPnt2P)->y) )
           { 
            *intersectTypeP = 1 ; 
            *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; 
            *nextXP = pointAddrP(dtmP,*trgPnt1P)->x ; 
            *nextYP = pointAddrP(dtmP,*trgPnt1P)->y ; 
            *nextZP = pointAddrP(dtmP,*trgPnt1P)->z ;
            goto cleanup  ; 
           }  
         else 
           { 
            *intersectTypeP = 1 ; 
            *trgPnt1P = *trgPnt2P ; 
            *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; 
            *nextXP = pointAddrP(dtmP,*trgPnt1P)->x ; 
            *nextYP = pointAddrP(dtmP,*trgPnt1P)->y ;
            *nextZP = pointAddrP(dtmP,*trgPnt1P)->z ;
            goto cleanup  ;
           }
        }
      if( sp1 >  0 ) pnt = antPnt ;
      if( sp1 <  0 ) pnt = clkPnt ;
      if( pnt == dtmP->nullPnt ) { *intersectTypeP = 5 ; goto cleanup  ; } /* On Tin Hull */
//      if( bcdtmList_testForVoidHullLineDtmObject(dtmP,*trgPnt1P,*trgPnt2P)){ *intersectTypeP = 5 ; goto cleanup  ; } ;
      *trgPnt3P = pnt ;
/*
**    Check If End Point Is In Triangle
*/
      if( bcdtmMath_pointInTriangleDtmObject(dtmP,*trgPnt1P,*trgPnt2P,*trgPnt3P,endX,endY))
        {
         *nextXP = endX ;
         *nextYP = endY ;
         bcdtmMath_interpolatePointOnTriangleDtmObject(dtmP,*nextXP,*nextYP,nextZP,*trgPnt1P,*trgPnt2P,*trgPnt3P) ;
         *intersectTypeP =  4 ;
         goto cleanup ;  
        }  
/*
**    Determine Intersection On Triangle Edge
*/      
      sp3 = bcdtmMath_sideOf(startX,startY,endX,endY,pointAddrP(dtmP,*trgPnt3P)->x,pointAddrP(dtmP,*trgPnt3P)->y) ;
      if( sp3 == 0 ) { *intersectTypeP = 1 ; *trgPnt1P = *trgPnt3P ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,*trgPnt1P)->x ; *nextYP = pointAddrP(dtmP,*trgPnt1P)->y ; *nextZP = pointAddrP(dtmP,*trgPnt1P)->z ; goto cleanup  ; }
      sp1 = bcdtmMath_sideOf(startX,startY,endX,endY,pointAddrP(dtmP,*trgPnt1P)->x,pointAddrP(dtmP,*trgPnt1P)->y) ;
      sp2 = bcdtmMath_sideOf(startX,startY,endX,endY,pointAddrP(dtmP,*trgPnt2P)->x,pointAddrP(dtmP,*trgPnt2P)->y) ;
      if( sp1 != sp3 )
        {
         *intersectTypeP = 2 ;
         bcdtmMath_normalIntersectCordLines(pointAddrP(dtmP,*trgPnt1P)->x,pointAddrP(dtmP,*trgPnt1P)->y,pointAddrP(dtmP,*trgPnt3P)->x,pointAddrP(dtmP,*trgPnt3P)->y,startX,startY,endX,endY,nextXP,nextYP) ;
         if( pointAddrP(dtmP,*trgPnt1P)->x == *nextXP && pointAddrP(dtmP,*trgPnt1P)->y == *nextYP )
           { *intersectTypeP = 1 ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,*trgPnt1P)->x ; *nextYP = pointAddrP(dtmP,*trgPnt1P)->y ; *nextZP = pointAddrP(dtmP,*trgPnt1P)->z ; goto cleanup  ; }
         if( pointAddrP(dtmP,*trgPnt3P)->x == *nextXP && pointAddrP(dtmP,*trgPnt3P)->y == *nextYP )
           { *intersectTypeP = 1 ; *trgPnt1P = *trgPnt3P ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,*trgPnt1P)->x ; *nextYP = pointAddrP(dtmP,*trgPnt1P)->y ; *nextZP = pointAddrP(dtmP,*trgPnt1P)->z ; goto cleanup  ; }
         bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*nextXP,*nextYP,nextZP,*trgPnt1P,*trgPnt3P) ;
         *trgPnt1P = *trgPnt1P ; *trgPnt2P = *trgPnt3P ; *trgPnt3P = dtmP->nullPnt ;
          goto cleanup  ;
        }
      if( sp2 != sp3 )
        {
         *intersectTypeP = 2 ;
         bcdtmMath_normalIntersectCordLines(pointAddrP(dtmP,*trgPnt2P)->x,pointAddrP(dtmP,*trgPnt2P)->y,pointAddrP(dtmP,*trgPnt3P)->x,pointAddrP(dtmP,*trgPnt3P)->y,startX,startY,endX,endY,nextXP,nextYP) ;
         if( pointAddrP(dtmP,*trgPnt2P)->x == *nextXP && pointAddrP(dtmP,*trgPnt2P)->y == *nextYP )
           { *intersectTypeP = 1 ; *trgPnt1P = *trgPnt2P ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,*trgPnt1P)->x ; *nextYP = pointAddrP(dtmP,*trgPnt1P)->y ; *nextZP = pointAddrP(dtmP,*trgPnt1P)->z ; goto cleanup  ; }
         if( pointAddrP(dtmP,*trgPnt3P)->x == *nextXP && pointAddrP(dtmP,*trgPnt3P)->y == *nextYP )
           { *intersectTypeP = 1 ; *trgPnt1P = *trgPnt3P ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,*trgPnt1P)->x ; *nextYP = pointAddrP(dtmP,*trgPnt1P)->y ; *nextZP = pointAddrP(dtmP,*trgPnt1P)->z ; goto cleanup  ; }
         bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*nextXP,*nextYP,nextZP,*trgPnt2P,*trgPnt3P) ;
         *trgPnt1P = *trgPnt2P ; *trgPnt2P = *trgPnt3P ; *trgPnt3P = dtmP->nullPnt ;
         goto cleanup  ;
        }
      bcdtmWrite_message(1,0,0,"Slope Intercept Error 2 ") ;
      goto errexit ;
    break  ;

    case 3 :  /* Last Intercept Was In Triangle trgPnt1P-trgPnt2P-trgPnt3P */

/*
**    Check If Last Point Is In Triangle
*/
      if( bcdtmMath_pointInTriangleDtmObject(dtmP,*trgPnt1P,*trgPnt2P,*trgPnt3P,endX,endY))
        {
         *nextXP = endX ;
         *nextYP = endY ;
         bcdtmMath_interpolatePointOnTriangleDtmObject(dtmP,*nextXP,*nextYP,nextZP,*trgPnt1P,*trgPnt2P,*trgPnt3P) ;
         *intersectTypeP =  4 ;
         goto cleanup ;  
        }
/*
**    Calculate Intesect With Triangle
*/      
      if( bcdtmMath_pointSideOfDtmObject(dtmP,*trgPnt1P,*trgPnt2P,*trgPnt3P) > 0 ) { pnt = *trgPnt2P ; *trgPnt2P = *trgPnt3P ; *trgPnt3P = pnt ; }
      sp1 = bcdtmMath_sideOf(startX,startY,endX,endY,pointAddrP(dtmP,*trgPnt1P)->x,pointAddrP(dtmP,*trgPnt1P)->y) ;
      if( sp1 == 0 ) { *intersectTypeP = 1 ; *trgPnt1P = *trgPnt1P ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,*trgPnt1P)->x ; *nextYP = pointAddrP(dtmP,*trgPnt1P)->y ; *nextZP = pointAddrP(dtmP,*trgPnt1P)->z ; goto cleanup  ; }
      sp2 = bcdtmMath_sideOf(startX,startY,endX,endY,pointAddrP(dtmP,*trgPnt2P)->x,pointAddrP(dtmP,*trgPnt2P)->y) ;
      if( sp2 == 0 ) { *intersectTypeP = 1 ; *trgPnt1P = *trgPnt2P ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,*trgPnt1P)->x ; *nextYP = pointAddrP(dtmP,*trgPnt1P)->y ; *nextZP = pointAddrP(dtmP,*trgPnt1P)->z ; goto cleanup  ; }
      sp3 = bcdtmMath_sideOf(startX,startY,endX,endY,pointAddrP(dtmP,*trgPnt3P)->x,pointAddrP(dtmP,*trgPnt3P)->y) ;
      if( sp3 == 0 ) { *intersectTypeP = 1 ; *trgPnt1P = *trgPnt3P ; *trgPnt2P = *trgPnt3P = dtmP->nullPnt ; *nextXP = pointAddrP(dtmP,*trgPnt1P)->x ; *nextYP = pointAddrP(dtmP,*trgPnt1P)->y ; *nextZP = pointAddrP(dtmP,*trgPnt1P)->z ; goto cleanup  ; }
      if( sp1 > 0 && sp2 < 0 )
        {
         *intersectTypeP = 2 ; *trgPnt3P = dtmP->nullPnt ;
         bcdtmMath_normalIntersectCordLines(pointAddrP(dtmP,*trgPnt1P)->x,pointAddrP(dtmP,*trgPnt1P)->y,pointAddrP(dtmP,*trgPnt2P)->x,pointAddrP(dtmP,*trgPnt2P)->y,startX,startY,endX,endY,nextXP,nextYP) ;
         bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*nextXP,*nextYP,nextZP,*trgPnt1P,*trgPnt2P) ;
         goto cleanup  ;
        }
      if( sp2 > 0 && sp3 < 0 )
        {
         *intersectTypeP = 2 ;
         bcdtmMath_normalIntersectCordLines(pointAddrP(dtmP,*trgPnt2P)->x,pointAddrP(dtmP,*trgPnt2P)->y,pointAddrP(dtmP,*trgPnt3P)->x,pointAddrP(dtmP,*trgPnt3P)->y,startX,startY,endX,endY,nextXP,nextYP) ;
         bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*nextXP,*nextYP,nextZP,*trgPnt2P,*trgPnt3P) ;
         *trgPnt1P = *trgPnt2P ; *trgPnt2P = *trgPnt3P ; *trgPnt3P = dtmP->nullPnt ;
         goto cleanup  ;
        }
      if( sp3 > 0 && sp1 < 0 )
        {
         *intersectTypeP = 2 ;
         bcdtmMath_normalIntersectCordLines(pointAddrP(dtmP,*trgPnt3P)->x,pointAddrP(dtmP,*trgPnt3P)->y,pointAddrP(dtmP,*trgPnt1P)->x,pointAddrP(dtmP,*trgPnt1P)->y,startX,startY,endX,endY,nextXP,nextYP) ;
         bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*nextXP,*nextYP,nextZP,*trgPnt3P,*trgPnt1P) ;
         *trgPnt2P = *trgPnt1P ; *trgPnt1P = *trgPnt3P ; *trgPnt3P = dtmP->nullPnt ;
         goto cleanup  ;
        }
      bcdtmWrite_message(1,0,0,"Slope Intercept Error 3 ") ;
      goto errexit ;
    break  ;

    default :
      bcdtmWrite_message(1,0,0,"Slope Intercept Error 4 ") ;
      goto errexit ;
    break ;
   } ;
/*
** Cleanup
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Surface With Vector Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Surface With Vector Error") ;
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
BENTLEYDTM_Private int  bcdtmDrape_intersectContourCallBackFunction
(
 DTMFeatureType dtmFeatureType,
 DTMUserTag   userTag,
 DTMFeatureId featureId,
 DPoint3d       *featurePtsP,
 size_t         numFeaturePts,
 void           *userP
)
/*
** Call Back Function For Intersect Contour
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 char  dtmFeatureTypeName[100] ;
 DTM_POINT_ARRAY *contourPointsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg == 1 ) 
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"DTM Feature = %s userTag = %10I64d featureId = %10I64d featurePtsP = %p numFeaturePts = %6ld userP = %p",dtmFeatureTypeName,userTag,featureId,featurePtsP,numFeaturePts,userP) ;
   } 
/*
** Copy Contour Points To Point Array
*/
 if( userP != NULL )
   {
    contourPointsP = ( DTM_POINT_ARRAY *) userP ;
    contourPointsP->numPoints = (long)numFeaturePts ;
    contourPointsP->pointsP   = ( DPoint3d *) malloc(contourPointsP->numPoints * sizeof(DPoint3d)) ;
    if( contourPointsP->pointsP == NULL )
      {
       bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    memcpy(contourPointsP->pointsP,featurePtsP,numFeaturePts*sizeof(DPoint3d)) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersect Contour Call Back Function Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersect Contour Call Back Function Error") ;
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
BENTLEYDTM_EXPORT int bcdtmDrape_intersectContourDtmObject
(
 BC_DTM_OBJ *dtmP,                   // ==> Pointer To DTM Object
 DPoint3d    *startPointP,                // ==> Vector Start Point 
 DPoint3d    *endPointP,                  // ==> Vector End Point
 double contourInterval,             // ==> Contour Interval
 double contourRegistration,         // ==> Contour Registration
 DTMContourSmoothing smoothOption,                // ==> Contour Smoothing Option<NONE(0),VERTEX(1),SPLINE(2),SPLINE(3)> 
 double smoothFactor,                // ==> Contour Smoothing Factor                               
 long   smoothDensity,               // ==> Point Densification For Spline Smoothing          
 double snapTolerance,               // ==> Snap Tolerance
 long   *contourFoundP,              // <== Contour Found < 1 = TRUE > < 0 = FALSE >
 DPoint3d    **conPtsPP,                  // <== Contour Points
 long   *numConPtsP                  // <== Number Of Contour Points
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   pnt1, pnt2, pnt3, fndType, conType, surfaceType, offset, numDrapePts;
 bool voidFlag, contourPointFound = false;
 double dx,dy,dz,dl,zMin,zMax,minContourPointDistance  ;
 double contourLow,contourHigh,startAngle,drapeAngle=0.0,drapeAngleInc=0.0 ;
 DPoint3d    surfacePoint,contourPoint,closestContourPoint,drapeLine[2] ;
 DTMDrapePoint *drapeP, *drapePtsP;
 bvector<DTMDrapePoint> drapePts;
 DTM_POINT_ARRAY contourPoints ;
/*
** Write Entry Message
*/
 if( dbg == 1 ) 
   {
    bcdtmWrite_message(0,0,0,"Intersecting Contour With Vector") ;
    bcdtmWrite_message(0,0,0,"dtmP                = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPointP->x      = %12.5lf",startPointP->x) ;
    bcdtmWrite_message(0,0,0,"startPointP->y      = %12.5lf",startPointP->y) ;
    bcdtmWrite_message(0,0,0,"startPointP->z      = %12.5lf",startPointP->z) ;
    bcdtmWrite_message(0,0,0,"endPointP->x        = %12.5lf",endPointP->x) ;
    bcdtmWrite_message(0,0,0,"endPointP->y        = %12.5lf",endPointP->y) ;
    bcdtmWrite_message(0,0,0,"endPointP->z        = %12.5lf",endPointP->z) ;
    bcdtmWrite_message(0,0,0,"contourInterval     = %12.5lf",contourInterval) ;
    bcdtmWrite_message(0,0,0,"contourRegistration = %12.5lf",contourRegistration) ;
    bcdtmWrite_message(0,0,0,"Smooth Option       = %8ld",smoothOption) ;
    bcdtmWrite_message(0,0,0,"Smooth Factor       = %8.2lf",smoothFactor) ;
    bcdtmWrite_message(0,0,0,"Smooth Density      = %8ld",smoothDensity) ;
    bcdtmWrite_message(0,0,0,"snapTolerance       = %12.5lf",snapTolerance) ;
   } 
/*
** Initialise
*/ 
 *contourFoundP = FALSE ;
 *numConPtsP    = 0 ;
 if( *conPtsPP != NULL ) { free(*conPtsPP) ; *conPtsPP = NULL ; }  
 contourPoints.pointsP = NULL ;
 contourPoints.numPoints = 0 ;
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Find Triangle For Vector
*/
 voidFlag = false;
 surfaceType = 0 ;
 if( startPointP->x == endPointP->x )    // Top View
   {
    surfacePoint.x = startPointP->x ;
    surfacePoint.y = startPointP->y ;
    surfacePoint.z = startPointP->z ;
    if( bcdtmFind_triangleForPointDtmObject(dtmP,surfacePoint.x,surfacePoint.y,&surfacePoint.z,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
/*
**  Test For Surface Point In Void
*/
    if( fndType )
      {
       if( fndType == 1 )
         {
          surfaceType = 1 ;
          if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,pnt1)->PCWD) ) voidFlag = 1 ;
         } 
       if( fndType == 2 || fndType == 3 ) 
         {
          surfaceType = 2 ;
          if( bcdtmList_testForVoidLineDtmObject(dtmP,pnt1,pnt2,voidFlag)) goto errexit ;
         }
       if( fndType == 4 ) 
         {
          surfaceType = 2 ;
          if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidFlag)) goto errexit ;
         }
      }
   }
 else                                    // Not A Top View
   {
    if( bcdtmDrape_intersectSurfaceDtmObject(dtmP,startPointP,endPointP,&surfaceType,&surfacePoint,&pnt1,&pnt2,&pnt3,voidFlag)) goto errexit ;
   }
/*
** Look For Contour In Snapping Distance
*/
 if( surfaceType && ! voidFlag )
   {
/*
**  Calculate Contour Values On Either Side Of Surface Point
*/
    offset = (long) ((surfacePoint.z - contourRegistration ) / contourInterval ) ;
    contourLow  = contourRegistration + ((double)offset)     * contourInterval ;
    contourHigh = contourRegistration + ((double)(offset+1)) * contourInterval ;
    if( dbg ) bcdtmWrite_message(0,0,0,"surfacePoint.z = %12.5lf contourLow = %12.5lf contourHigh = %12.5lf",surfacePoint.z,contourLow,contourHigh) ;
/*
**  Scan Around Point Looking For A Contour Intersection
*/
    fndType = 0 ;
    drapeAngle = startAngle = 0.0 ;
    drapeAngleInc  = DTM_2PYE / 100.0 ;
    drapeLine[0].x = surfacePoint.x ;
    drapeLine[0].y = surfacePoint.y ;
    drapeLine[0].z = surfacePoint.z ;
    contourPointFound = FALSE ;
    while( drapeAngle - startAngle <= DTM_2PYE /*&& ! contourPointFound */)
      {
       drapeLine[1].x  = surfacePoint.x + snapTolerance * cos(drapeAngle) ;
       drapeLine[1].y  = surfacePoint.y + snapTolerance * sin(drapeAngle) ;
       if( bcdtmDrape_stringDtmObject(dtmP,drapeLine,2,FALSE,drapePts)) goto errexit ;
       numDrapePts = (long)drapePts.size();
       drapePtsP = drapePts.data();
/*
**     Scan Drape Points For Contour Start Point
*/
       for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts - 1 && ! contourPointFound  ; ++drapeP )
         {
         if (drapeP->drapeType != DTMDrapedLineCode::External && (drapeP + 1)->drapeType != DTMDrapedLineCode::External && drapeP->drapePt.z != (drapeP + 1)->drapePt.z)
            {
             zMin = zMax = drapeP->drapePt.z ;
             if( (drapeP+1)->drapePt.z < zMin ) zMin = (drapeP+1)->drapePt.z ;
             if( (drapeP+1)->drapePt.z > zMax ) zMax = (drapeP+1)->drapePt.z ;
             if( contourLow >= zMin && contourLow <= zMax )
               {
                dx = (drapeP+1)->drapePt.x - drapeP->drapePt.x ;
                dy = (drapeP+1)->drapePt.y - drapeP->drapePt.y ;
                dz = (drapeP+1)->drapePt.z - drapeP->drapePt.z ;
                dl = contourLow - drapeP->drapePt.z ;
                contourPoint.x = drapeP->drapePt.x + dx * dl / dz ;
                contourPoint.y = drapeP->drapePt.y + dy * dl / dz ;
                contourPoint.z = contourLow ;
                dl = bcdtmMath_distance(surfacePoint.x,surfacePoint.y,contourPoint.x,contourPoint.y) ;
                if( contourPointFound == FALSE || dl < minContourPointDistance )
                  { 
                   contourPointFound = TRUE ;
                   minContourPointDistance = dl ; 
                   closestContourPoint.x = contourPoint.x ;
                   closestContourPoint.y = contourPoint.y ;
                   closestContourPoint.z = contourPoint.z ;
                  }
               }  
             if( contourHigh >= zMin && contourHigh <= zMax )
               {
                dx = (drapeP+1)->drapePt.x - drapeP->drapePt.x ;
                dy = (drapeP+1)->drapePt.y - drapeP->drapePt.y ;
                dz = (drapeP+1)->drapePt.z - drapeP->drapePt.z ;
                dl = contourHigh  - drapeP->drapePt.z ;
                contourPoint.x = drapeP->drapePt.x + dx * dl / dz ;
                contourPoint.y = drapeP->drapePt.y + dy * dl / dz ;
                contourPoint.z = contourHigh ;
                if( contourPointFound == FALSE || dl < minContourPointDistance )
                  { 
                   contourPointFound = TRUE ;
                   minContourPointDistance = dl ; 
                   closestContourPoint.x = contourPoint.x ;
                   closestContourPoint.y = contourPoint.y ;
                   closestContourPoint.z = contourPoint.z ;
                  }
               }  
            }
         }
/*
**     Increment Drape Angle
*/
       drapeAngle = drapeAngle + drapeAngleInc ;
      }
   } 
/*
** Write Contour Point
*/
 if( dbg == 1 ) 
   {
    if( ! contourPointFound ) bcdtmWrite_message(0,0,0,"Contour Point Not Found") ;
    else 
      {
       bcdtmWrite_message(0,0,0,"Contour Point Found ** %12.5lf %12.5lf %12.5lf",closestContourPoint.x,closestContourPoint.y,closestContourPoint.z) ;
       long drapeFlag;
       bcdtmDrape_pointDtmObject(dtmP, closestContourPoint.x, closestContourPoint.y, &closestContourPoint.z, &drapeFlag);
       bcdtmWrite_message(0,0,0,"Contour Point Drape ** %12.5lf %12.5lf %12.5lf",closestContourPoint.x,closestContourPoint.y,closestContourPoint.z) ;
      }  
   } 
/*
**  Trace Contour At Contour Point
*/
 if( contourPointFound )
   {
/*
**  Trace Contour
*/
    if( bcdtmLoad_contourForPointDtmObject(dtmP,closestContourPoint.x,closestContourPoint.y,contourInterval,DTMContourSmoothing::None,0.0,5,bcdtmDrape_intersectContourCallBackFunction,0,DTMFenceOption::Inside,DTMFenceType::Block,NULL,0,(void *)&contourPoints)) goto errexit ;
    *conPtsPP   = contourPoints.pointsP ;
    *numConPtsP = contourPoints.numPoints ;
    contourPoints.pointsP = NULL ;
    if( *conPtsPP != NULL ) *contourFoundP = 1 ;
/*
**  Check For Depression Contour
*/
    DTMPondAppData* pondAppData = dynamic_cast<DTMPondAppData*>(dtmP->FindAppData (DTMPondAppData::AppDataID));

    if ( !pondAppData)
        {
        pondAppData = DTMPondAppData::Create ();
        dtmP->AddAppData (DTMPondAppData::AppDataID, pondAppData);
        }
    if( pondAppData != nullptr)
      {
       if( pondAppData->hasPonds && pondAppData->pondDtmP != NULL )
         {
          if( bcdtmLoad_checkForDepressionContourDtmObject(pondAppData->pondDtmP, *conPtsPP, *numConPtsP, 1.0, &conType)) goto errexit ;
          if( conType ) *contourFoundP = 2 ;
         }
      }
   }  
/*
** Cleanup
*/
 cleanup :
 if( contourPoints.pointsP  != NULL ) free(contourPoints.pointsP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Contour With Vector Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Contour With Vector Error") ;
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
BENTLEYDTM_Public void bcdtmDrape_locatePoint(	double xa,double ya,double distance,double direction,double *xb,double *yb)
/*
** xa,*ya     => beginning point A            
** distance   => distance                     
** direction  => direction in azimuth degrees 
** xb,*yb     <=  ending point B               
*/
{
 double x;
 double y;
 bcdtmDrape_traverseIgds(ya, xa, distance, bcdtmDrape_azimuthToAngle(direction), &y, &x);
 *xb = x;
 *yb = y;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmDrape_getCoordiantesFromLength(double *xs,double *ys,double r1,double r2,double spiralLength,double dl,double offset,double x1,double y1,double xi,double yi,double x2,double y2)
/*
**
** *xs,*ys      <= spiral coordinates 
** r1           => beginning radius 
** r2           => ending radius 
** spiralLength => spiral length 
** dl           => length along spiral curve (0 < dl < spiralLength) 
** offset       => signed offset (+ to the right, - to the left) 
** x1,y1        => beginning point of spiral 
** xi,yi        => PIS point of spiral 
** x2,y2        => ending point of spiral 
**
*/
{
    int entrance=0;
    double l0=0.0 ;
    double tangentDirection;
    double backDirection, offsetDirection;
    double dir, dis;
    double r;
    double da;
    
    double dx,dy,xn,yn;
    double xp, yp;
    double departure;
    
    bcdtmDrape_inverseIgds(y1,x1,yi,xi,&dis,&dir);
    backDirection   = bcdtmDrape_angleToAzimuth(dir);
    departure = bcdtmDrape_offset(x1, y1, xi, yi, x2, y2);
    offsetDirection = backDirection + 90.00 * SIGN_OF(departure);
    offsetDirection = bcdtmDrape_normalizeAngle(offsetDirection);
    
    if      (r1 < fc_epsilon)
    {
        entrance = 1;
        l0 = fc_zero;
    }
    else if (r2 < fc_epsilon)
    {
        entrance = 0;
        l0 = fc_zero;
    } 
    else if (r1 > r2)
    {
        entrance = 1;
        l0 = spiralLength * r2 / (r1 - r2);
    }
    else if (r1 < r2)
    {
        entrance = 0;
        l0 = spiralLength * r1 / (r2 - r1);
    }
    
    /* to compute coordinates of a spiral given length dl */
    if (entrance)
    {
        r = (l0 + spiralLength) * r2 / (l0 + dl);
    }
    else
    {
        r = (l0 + spiralLength) * r1 / (l0 + spiralLength - dl);
    }


    bcdtmDrape_coord(r1, r, dl, &dx, &dy);
    bcdtmDrape_locatePoint(x1, y1, dx, backDirection, &xn, &yn);
    bcdtmDrape_locatePoint(xn, yn, dy, offsetDirection, &xp, &yp);
    
    if (fabs(offset) > fc_epsilon)
    {
        if (entrance)
        {
            if (r1 < fc_epsilon)
                da = fabs((l0 + dl) / (2.0 * r));
            else
                da = fabs((l0 / (2.0 * r1)) - (l0 + dl) / (2.0 * r));
        }
        else
            da = 0.5 * ((spiralLength / r1) - ((l0 + spiralLength - dl) / r));

        da = da * 180.00 / fc_pi;

        tangentDirection = backDirection + SIGN_OF(departure) * da;
        offsetDirection = tangentDirection + SIGN_OF(offset) * 90.00;
        offsetDirection = bcdtmDrape_normalizeAngle(offsetDirection);
        bcdtmDrape_locatePoint(xp, yp, fabs(offset), offsetDirection, xs, ys);
    }
    else
    {
        *xs = xp;
        *ys = yp;
    }
    
    /*
    ** Job Completed
    */
    return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  Function bcdtmDrape_traverseIgds()                                  |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmDrape_traverseIgds (double p1y,double  p1x,double  dist,double  ang,double *ptr_p2y,double  *ptr_p2x)
{
    if (ang == fc_zero)
    {
        *ptr_p2y = p1y;
        *ptr_p2x = p1x + dist;
    }
    else if (ang == fc_90)
    {
        *ptr_p2x = p1x;
        *ptr_p2y = p1y + dist;
    }
    else if (ang == fc_180)
    {
        *ptr_p2y = p1y;
        *ptr_p2x = p1x - dist;
    }
    else if (ang == fc_270)
    {
        *ptr_p2x = p1x;
        *ptr_p2y = p1y - dist;
    }
    else
    {
        *ptr_p2x = p1x + dist * cos (ang * fc_piover180);       /* ang IN RADIANS */
        *ptr_p2y = p1y + dist * sin (ang * fc_piover180);
    }
    return (0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  Function bcdtmDrape_azimuthToAngle()                               |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmDrape_azimuthToAngle(double angle)
/* 
 angle  => azimuth angle measured CW from north   
 Description     This function converts a north azimuth CW angle 
 to a mathematical angle measured CCW from the x-axis
*/
{
 return (bcdtmDrape_normalizeAngle(fc_90 - angle));
}
/*-------------------------------------------------------------------+
|                                                                    |
|  Function bcdtmDrape_coord()                                         |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmDrape_coord( double r1,double r2,double l,double *xs,double *ys)
/*
**
 double r1,        => beginning radius of spiral (0 if tangent) 
 double r2,        => ending radius of spiral (0 if tangent)    
 double l,         => spiral length from r1 to r2               
 double *xs,       <=  distance along  long x-axis from 1 to 2   
 double *ys        <=  distance offset from x-axis from 1 to 2   

 Description     
 This function returns the local coordinates (xs, ys)
 where xs and ys are distances along and offset from
 the local x-axis resectively.  The local x-axis
 origin is at the spiral beginning point (at r = r1)
 in the direction equal to the spiral back tangent.
 
  The computation of xs and ys is by Taylor series
  expansion up to j=4:
  
               j=3  (-1^j) * (u^(2j)) * F(2j) 
    Xs = Ls * Sigma -------------------------
               j=0            (2j)!
   
               j=3  (-i^j) * (u^(2j+1)) * F(2j+1)
    Ys = Ls * Sigma -----------------------------
               j=0            (2j+1)!
    
    where F(n) is the following finite sum:
     
            i=n    n! (m^(n-i))
    F(n) = Sigma ----------------- => function bcdtmDrape_fn(n,m)
            i=0  (n+1+1) i! (n-i)! 
      
**
*/
       
{
    int i;
    int j;
    int k;
    double d1, d2;
    double m, u;
    
    *xs = fc_zero;
    *ys = fc_zero;
    
    /* compute degrees of curvature */
    d1 = (r1 < fc_epsilon) ? fc_zero : fc_100 / r1;
    d2 = (r2 < fc_epsilon) ? fc_zero : fc_100 / r2;
    
    if (fabs(d1 - d2) < fc_epsilon) return FAIL;
    
    m = 2.0 * d1 / (d2 - d1);
    u = l * (d2 - d1) / 200.0;
    
    /*
    the highest power term for x is u^2i, and for y is u^(2i+1)
    */
    for (i = 0; i <= 4; i++)
    {
        j = 2 * i;
        k = (int)pow(-1.0, i);
        *xs = (*xs) + (k * pow(u, j) * bcdtmDrape_fn(j, m) / bcdtmDrape_dfac(j));
        j = 2 * i + 1;
        *ys = (*ys) + (k * pow(u, j) * bcdtmDrape_fn(j, m) / bcdtmDrape_dfac(j));
    }
    *xs = fabs(*xs) * l;
    *ys = fabs(*ys) * l;
    return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  Function bcdtmDrape_normalizeAngle()                                |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmDrape_normalizeAngle( double angle )
/*
  angle => angle to be normalized                    

 Description     This function normalize an angle to be bounded between
 0 and 2pi

*/
{
 while (angle >= fc_360) angle -= fc_360;
 while (angle < fc_zero) angle += fc_360;
 return (angle);
}

/*-------------------------------------------------------------------+
|                                                                    |
|  Function bcdtmDrape_dfac()                                          |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmDrape_dfac(int n)
/*
Description     This function returns n! as type double
*/
{
    int i;
    unsigned long f;
    
    f = 1uL;
    if (n > 1) for (i = n; i > 1; i--) f = f * i;
    return ((double)f);
}
/*-------------------------------------------------------------------+
|                                                                    |
|  Function bcdtmDrape_fn()                                            |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmDrape_fn (int n, double m )
/*
 Description     
 This function computes the finite sum f expressed as:
 
          i=n    n! (m^(n-i))
  F(n) = Sigma -----------------   Fresnel Integral
          i=0  (n+1+1) i! (n-i)! 
  
  called by bcdtmDrape_coord 
   
*/
{
    int i;
    double f;
    
    f = fc_zero;
    for (i = 0; i <= n; i++)
    {
        /* check m^(n-i) to trap 0^0 */
        if (m < fc_epsilon && (n - i) == 0)
            f = f + bcdtmDrape_facdiv(n, n - i) / ((n + i + 1) * bcdtmDrape_dfac(i)) ;
        else
            f = f + bcdtmDrape_facdiv(n, n - i) * pow(m, n - i) / ((n + i + 1) * bcdtmDrape_dfac(i)) ;
    }
/*
** Job Completed
*/
 return (f);
}
/*-------------------------------------------------------------------+
|                                                                    |
|  Function bcdtmDrape_angleToAzimuth()                               |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmDrape_angleToAzimuth(double angle)

/* 
 angle => horizontal angle measured CCW from x-axis 

 Description     This function converts a mathematical angle measured
 CCW from the x-axis to a north azimuth CW angle
 */
{
 return (bcdtmDrape_normalizeAngle(fc_90 - angle));
}
/*-------------------------------------------------------------------+
|                                                                    |
|  Function bcdtmDrape_offset()                                        |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmDrape_offset(double xa,double ya,double xb,double yb,double xc,double yc )
/*
** Description     This function computes the signed offset of (xc, yc)
**                 with respect to the line (xa, ya) to (xb, yb).
** Return Value    Offset distance (- for left, + for right). 
*/
{
 double d;
 double s;

 d = sqrt((xb - xa) * (xb - xa) + (yb - ya) * (yb - ya));
 s = ((ya - yc) * (xb - xa) - (xa - xc) * (yb - ya)) / d;
 return (s);
}
/*-------------------------------------------------------------------+
|                                                                    |
|  Function bcdtmDrape_facdiv()                                        |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmDrape_facdiv( int a, int b )
/*
** Description --  This function returns (a! / b!) as type double
*/
{
    int i;
    long f;
    
    f = 1;
    if (a > b)
    {
        for (i = a; i > b; i--) f = f * i;
        return ((double)f);
    }
    else if (b > a)
    {
        for (i = b; i > a; i--) f = f * i;
        return ((double)(fc_1 / f)); 
    }
/*
** Job Completed
*/
 return (fc_1);
}
/*-------------------------------------------------------------------+
|                                                                    |
|  Function bcdtmDrape_inverseIgds()                                   |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmDrape_inverseIgds(double n1,double e1,double n2,double e2,double *ptr_dist,double *ptr_ang)
{
 double ee = e2-e1;
 double nn = n2-n1;

if (fabs(nn)<.000001)
    {
    *ptr_dist = fabs(ee);
    if (ee < fc_zero) *ptr_ang = fc_180;
    else *ptr_ang = fc_zero;
    return 0;
    }
else if (fabs(ee)<.000001)
    {
    *ptr_dist = fabs(nn);
    if (nn < fc_zero) *ptr_ang = fc_270;
    else *ptr_ang = fc_90;
    return 0;
    }

/* COMPUTE DISTANCE BETWEEN THE TWO POINTS */
*ptr_dist = sqrt(pow(fabs(ee),fc_2)+pow(fabs(nn),fc_2));

/* TEST TO SEE IF THE TWO POINTS COINCIDE */
if(*ptr_dist == fc_zero)
 {
 *ptr_ang = fc_zero;
 return 0;
 }

/* COMPUTE THE DIRECTION FROM FIRST TO SECOND POINT -- */
*ptr_ang = asin (fabs( nn / (*ptr_dist) ) );   /* IN RADIAN */

/* N-E QUADRANT */
if (((n1 == n2) && (e2 > e1)) || ((n2 > n1) && (e2 >= e1)))*ptr_ang = *ptr_ang;

/* S-E QUADRANT */
else if(( n2 < n1) && ( e2 >= e1))*ptr_ang = fc_2 * fc_pi - *ptr_ang;

/* N-W QUADRANT */
else if(( n2 >= n1) && ( e2 < e1) )*ptr_ang = fc_pi - *ptr_ang;

/* S-W QUADRANT */
else *ptr_ang += fc_pi;

/* AZIMUTH ALWAYS POSITIVE */
if (*ptr_ang < 0.) *ptr_ang += fc_2 * fc_pi;
/*-----------------------------*/
/*  CONVERT AZIMUTH TO DEGREES */
/*-----------------------------*/
*ptr_ang *= fc_180overpi;
/*
** Job Completed
*/
 return(0) ;
}
