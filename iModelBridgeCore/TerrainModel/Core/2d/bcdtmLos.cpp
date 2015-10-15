/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmLos.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
#include <stdlib.h>
//#pragma optimize( "p", on )

/*
** Static Global Variables For Accumulating Los Vertices
*/
thread_local static DPoint3d  *losPtsP = nullptr;
thread_local static long *losLinesP = nullptr;
thread_local static long numLosPts = 0;
thread_local static long memLosPts = 0;
thread_local static long memLosPtsInc = 1000;

BENTLEYDTM_Private int bcdtmVisibility_determineIfHorizonLineIsTotallyCovered
(
 double Xe,
 double Ye,
 DTM_HORIZON_LINE *hLine1P,
 DTM_HORIZON_LINE *hLine2P
) ;

/*
** Static Global Variables 
*/
static long   numHorLines=0,memHorLines=0,trgNumber,numHorLinesIndex=0,*hozIndexListP=nullptr,numHorIndexList=0,memHorIndexList=0 ;
static double eyeX,eyeY,eyeZ,tinRadius=0.0 ;
static BC_DTM_OBJ              *losDtmP=nullptr ;
static DTM_HORIZON_LINE        *horLinesP=nullptr ;
static DTM_HORIZON_LINE_INDEX  *horLinesIndexP=nullptr ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmVisibility_freeMemory(void)
/*
** This Function Frees Los Memory
*/
{
 bcdtmVisibility_freeLosVertices() ;
 bcdtmVisibility_freeHorizonLineArrays() ;
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
BENTLEYDTM_Private int bcdtmVisibility_freeLosVertices(void)
/*
** This Function Frees Los Memory
*/
{
 numLosPts = 0 ;
 memLosPts = 0 ;
 if( losPtsP   != nullptr ) { free(losPtsP)   ; losPtsP   = nullptr ; }
 if( losLinesP != nullptr ) { free(losLinesP) ; losLinesP = nullptr ; }
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
BENTLEYDTM_Private int bcdtmVisibility_freeHorizonLineArrays(void)
/*
** This Function Frees The Horizon Arrays
*/
{
 numHorLines      = 0 ;
 memHorLines      = 0 ;
 numHorLinesIndex = 0 ;
 numHorIndexList  = 0 ;
 memHorIndexList  = 0 ;
 if( horLinesP      != nullptr ) { free(horLinesP)      ; horLinesP      = nullptr ; }
 if( horLinesIndexP != nullptr ) { free(horLinesIndexP) ; horLinesIndexP = nullptr ; }
 if( hozIndexListP  != nullptr ) { free(hozIndexListP)  ; hozIndexListP  = nullptr ; }
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
BENTLEYDTM_Private int bcdtmVisibility_fudgeLineVisibility(long *Vline)
{
 double Vl=0.0,Il=0.0,dx,dy ;
 long *pl ;
 DPoint3d  *p3d ;
/*
** Initialise
*/
 *Vline = 1 ;
 for( pl = losLinesP , p3d = losPtsP ; pl < losLinesP + numLosPts ; pl=pl+2 ,p3d=p3d+2 )
   {
    dx = (p3d+1)->x - p3d->x ;
    dy = (p3d+1)->y - p3d->y ;
    if( *pl == 1 ) Vl = Vl + sqrt(dx*dx + dy*dy) ;
    else           Il = Il + sqrt(dx*dx + dy*dy) ;
   }
/*
** Set Visibility
*/
 if( Il > Vl ) *Vline = -1 ;
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
BENTLEYDTM_Private int bcdtmVisibility_writeHorizonLines(DTM_HORIZON_LINE *hozPtsP,long numHorPts)
/*
** This Function Writes The Horizon Lines
*/
{
 DTM_HORIZON_LINE *hptsP ;
/*
** Write Structure Data
*/
 for( hptsP = hozPtsP ; hptsP < hozPtsP + numHorPts ; ++hptsP )
   {
    bcdtmWrite_message(0,0,0,"hptsP[%6ld] == %10.8lf %10.8lf ** %9.3lf %9.3lf ** %9.3lf %9.3lf %7.3lf ** %9.3lf %9.3lf %7.3lf",(long)(hptsP-hozPtsP),hptsP->Ang1,hptsP->Ang2,hptsP->D1,hptsP->D2,hptsP->X1,hptsP->Y1,hptsP->Z1,hptsP->X2,hptsP->Y2,hptsP->Z2) ;
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
BENTLEYDTM_Private int bcdtmVisibility_writeVisibilityPoints(long *visFlagP,DPoint3d *visPtsP,long numVisPts)
/*
** This Function Writes The Visibility Points
*/
{
 long *pl ;
 DPoint3d  *p3d ;
/*
** Write The Points
*/
 for( pl = visFlagP , p3d = visPtsP ; pl < visFlagP + numVisPts ; ++pl , ++p3d )
   {
    bcdtmWrite_message(0,0,0,"%2ld ** %12.6lf %12.6lf %10.6lf",*pl,p3d->x,p3d->y,p3d->z ) ;
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
BENTLEYDTM_Private int bcdtmVisibility_determineVisibilityDirectionOfTriangleDtmObject(BC_DTM_OBJ *dtmP,double Xe,double Ye,double Ze,long P1,long P2,long P3,long *visibilityDirectionP)
/*
** This Function Determines If The Eye Is On The Visible Side Of The Triangle Face
** P1,P2,P3 Are The Tin Point Numbers For The Traingle And Must Be In A Clockwise Direction
**
** Visibility Direction == 1  If Eye On Visible Side Of Triangle
**                      == 2  If Eye Is On Invisible Side Of Triangle
*/
{
 int    ret=DTM_SUCCESS ;
 double dp,Ca,Cb,Cc,Cd,X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,Xmin,Ymin,Zmin ;
 DTM_TIN_POINT *pntP ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
  if( dtmP->dtmState != DTMState::Tin )
    {
     bcdtmWrite_message(2,0,0,"DTM Object Not Triangulated") ;
     goto errexit ;
    }
/*
** Get Triangle Coordinates
*/
 pntP = pointAddrP(dtmP,P1) ;
 X1 = pntP->x ; Y1 = pntP->y ; Z1 = pntP->z ;
 pntP = pointAddrP(dtmP,P2) ;
 X2 = pntP->x ; Y2 = pntP->y ; Z2 = pntP->z ;
 pntP = pointAddrP(dtmP,P3) ;
 X3 = pntP->x ; Y3 = pntP->y ; Z3 = pntP->z ;
/*
** Normalise Triangle Cordinates
*/
 Xmin = X1 ; if( X2 < Xmin ) Xmin = X2 ; if( X3 < Xmin ) Xmin = X3 ;
 Ymin = Y1 ; if( Y2 < Ymin ) Ymin = Y2 ; if( Y3 < Ymin ) Ymin = Y3 ;
 Zmin = Z1 ; if( Z2 < Zmin ) Zmin = Z2 ; if( Z3 < Zmin ) Zmin = Z3 ;
 X1 -= Xmin ; X2 -= Xmin ; X3 -= Xmin ;
 Y1 -= Ymin ; Y2 -= Ymin ; Y3 -= Ymin ;
 Z1 -= Zmin ; Z2 -= Zmin ; Z3 -= Zmin ;
/*
** Normalise Eye Cordinates
*/
 Xe -= Xmin ; Ye -= Ymin ; Ze -=Zmin ;
/*
** Calculate Plane Coefficients
*/
 bcdtmMath_calculatePlaneCoefficients(X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,&Ca,&Cb,&Cc,&Cd) ;
/*
** Calculate Distance Of Eye To Triangle Plane
*/
 dp = ( Xe*Ca + Ye*Cb + Ze*Cc + Cd ) / sqrt(Ca*Ca + Cb*Cb + Cc*Cc) ;
 if( dp >= 0.0 ) *visibilityDirectionP = 1 ;
 else            *visibilityDirectionP = 0 ;
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
BENTLEYDTM_Private int bcdtmVisibility_determineVisibilityOfPoint
(
 double Xe,
 double Ye,
 double Ze,
 double X1,
 double Y1,
 double Z1,
 double X2,
 double Y2,
 double Z2,
 double x,
 double y,
 double z,
 long   *visibilityP 
)
/*
** This Function Determines If The Eye Is On Visible Side Of The Triangle Face
** Formed By XeYe-X1Y1-X2Y2 With A ClockWise Direction Of XeYe-X1Y1-X2Y2
** Visibility Direction == 1  If Point Is Visible   To Eye
**                      == 0  If Point Is Invisible To Eye
*/
{
 int    dbg=DTM_TRACE_VALUE(0) ;
 long   Vp ;
 double dp,dl,Ca,Cb,Cc,Cd,Xmin,Ymin,Zmin ;
/*
** Write Entry Parameters
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determining Visibility Of Point") ;
    bcdtmWrite_message(0,0,0,"Xe    = %12.5lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Ye    = %12.5lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Ze    = %12.5lf",Ze) ;
    bcdtmWrite_message(0,0,0,"X1    = %12.5lf",X1) ;
    bcdtmWrite_message(0,0,0,"Y1    = %12.5lf",Y1) ;
    bcdtmWrite_message(0,0,0,"Z1    = %12.5lf",Z1) ;
    bcdtmWrite_message(0,0,0,"X2    = %12.5lf",X2) ;
    bcdtmWrite_message(0,0,0,"Y2    = %12.5lf",Y2) ;
    bcdtmWrite_message(0,0,0,"Z2    = %12.5lf",Z2) ;
    bcdtmWrite_message(0,0,0,"x     = %12.5lf",x) ;
    bcdtmWrite_message(0,0,0,"y     = %12.5lf",y) ;
    bcdtmWrite_message(0,0,0,"z     = %12.5lf",z) ;
   } 
/*
** Initialise
*/
 *visibilityP = 0 ;
/*
** Normalise Triangle Cordinates
*/
 Xmin = Xe ; if( X1 < Xmin ) Xmin = X1 ; if( X2 < Xmin ) Xmin = X2 ;
 Ymin = Ye ; if( Y1 < Ymin ) Ymin = Y1 ; if( Y2 < Ymin ) Ymin = Y2 ;
 Zmin = Ze ; if( Z1 < Zmin ) Zmin = Z1 ; if( Z2 < Zmin ) Zmin = Z2 ;
/*
** Normalise Eye Cordinates
*/
 Xe -= Xmin ; Ye -= Ymin ; Ze -= Zmin ;
 X1 -= Xmin ; Y1 -= Ymin ; Z1 -= Zmin ;
 X2 -= Xmin ; Y2 -= Ymin ; Z2 -= Zmin ;
 x  -= Xmin ; y  -= Ymin ; z  -= Zmin ;
/*
** Calculate Plane Coefficients
*/
 bcdtmMath_calculatePlaneCoefficients(Xe,Ye,Ze,X1,Y1,Z1,X2,Y2,Z2,&Ca,&Cb,&Cc,&Cd) ;
 dl = sqrt(Ca*Ca + Cb*Cb + Cc*Cc) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dl = %15.12lf",dl) ;
 if( dl == 0.0 ) { *visibilityP = 1 ; return(DTM_SUCCESS) ; }
/*
** Calculate Distance Of Point To Eye Plane
*/
 dp = ( x*Ca + y*Cb + z*Cc + Cd ) / dl ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dp = %15.12lf",dp) ;
 if( dp >= 0.0 ) Vp = 0 ;
 else            Vp = 1 ;
 if( Vp == 0 && fabs(dp) < 0.0000001 ) Vp = 1 ;
/*
** Set Visibility
*/
 *visibilityP = Vp ;
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
BENTLEYDTM_Private int bcdtmVisibility_determineVisibilityOfEdge(double Xe,double Ye,double Ze,double X1,double Y1,double Z1,double X2,double Y2,double Z2,double X3,double Y3,double Z3,double X4,double Y4,double Z4,long *Visibility,DPoint3d Point[] )

/*
** This Function Determines If The Eye Is On Visible Side Of The Triangle Face
** P1,P2,P3 Are The Tin Point Numbers For The Traingle And Must Be In A Clockwise
** Direction
** Visibility Direction == 1  If Eye On Visible Side Of Triangle
**                      == 2  If Eye Is On Invisible Side Of Triangle
*/
{
 long   Vp1,Vp2 ;
 double dp1,dp2,dl,Ca,Cb,Cc,Cd,x,y,z,Xmin,Ymin,Zmin ;
/*
** Initialise
*/
 *Visibility = 0 ;
/*
** Normalise Triangle Cordinates
*/
 Xmin = Xe ; if( X1 < Xmin ) Xmin = X1 ; if( X2 < Xmin ) Xmin = X2 ;
 Ymin = Ye ; if( Y1 < Ymin ) Ymin = Y1 ; if( Y2 < Ymin ) Ymin = Y2 ;
 Zmin = Ze ; if( Z1 < Zmin ) Zmin = Z1 ; if( Z2 < Zmin ) Zmin = Z2 ;
/*
** Normalise Eye Cordinates
*/
 Xe -= Xmin ; Ye -= Ymin ; Ze -= Zmin ;
 X1 -= Xmin ; Y1 -= Ymin ; Z1 -= Zmin ;
 X2 -= Xmin ; Y2 -= Ymin ; Z2 -= Zmin ;
 X3 -= Xmin ; Y3 -= Ymin ; Z3 -= Zmin ;
 X4 -= Xmin ; Y4 -= Ymin ; Z4 -= Zmin ;
/*
** Calculate Plane Coefficients
*/
 bcdtmMath_calculatePlaneCoefficients(Xe,Ye,Ze,X1,Y1,Z1,X2,Y2,Z2,&Ca,&Cb,&Cc,&Cd) ;
 dl = sqrt(Ca*Ca + Cb*Cb + Cc*Cc) ;
 if( dl == 0.0 ) { *Visibility = 1 ; return(1) ; }
/*
** Calculate Distance Of Edge To Eye Plane
*/
 dp1 = ( X3*Ca + Y3*Cb + Z3*Cc + Cd ) / dl ;
 if( dp1 >= 0.0 ) Vp1 = 0 ;
 else             Vp1 = 1 ;
 dp2 = ( X4*Ca + Y4*Cb + Z4*Cc + Cd ) / dl ;
 if( dp2 >= 0.0 ) Vp2 = 0 ;
 else             Vp2 = 1 ;
/*
** Set Visibility
*/
 if     ( Vp1 == 0 && Vp2 == 0 ) *Visibility = -1 ;
 else if( Vp1 == 1 && Vp2 == 1 ) *Visibility =  1 ; 
/*
** Set Visible Part Of Line
*/
 if( ! *Visibility )
   {
    if( fabs(dp1) < 0.0000001 )
      {
       if( dp2 < 0.0 ) { *Visibility =  1 ; return(0) ; }
       if( dp2 > 0.0 ) { *Visibility = -1 ; return(0) ; }
      }   
    if( fabs(dp2) < 0.0000001 )
      {
       if( dp1 < 0.0 ) { *Visibility =  1 ; return(0) ; }
       if( dp1 > 0.0 ) { *Visibility = -1 ; return(0) ; }
      } 
    if( dp1 < 0.0 ) dp1 = -dp1 ;
    if( dp2 < 0.0 ) dp2 = -dp2 ;
    x = X3 + (X4-X3) * dp1/(dp1+dp2) ;   
    y = Y3 + (Y4-Y3) * dp1/(dp1+dp2) ;   
    z = Z3 + (Z4-Z3) * dp1/(dp1+dp2) ; 
    if( Vp1 ) 
      {
       Point[0].x = X3 ; Point[0].y = Y3 ; Point[0].z = Z3 ;  
       Point[1].x = x  ; Point[1].y = y  ; Point[1].z = z  ;  
      }
    else
      {
       Point[0].x = x  ; Point[0].y = y  ; Point[0].z = z  ;  
       Point[1].x = X4 ; Point[1].y = Y4 ; Point[1].z = Z4 ;  
      }
    Point[0].x += Xmin ; Point[0].y += Ymin ; Point[0].z += Zmin ;
    Point[1].x += Xmin ; Point[1].y += Ymin ; Point[1].z += Zmin ;
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
BENTLEYDTM_Private double bcdtmVisibility_interpolateZOnLineOfSight(double Xe,double Ye,double Ze,double Xp,double Yp,double Zp,double x, double y)
/*
** This Function Calculates The z Value At XY On The Line Of Sight XeYeZe-XpYpZp
*/
{
 double dx,dy,dz,z ;
 dx = Xp - Xe ;
 dy = Yp - Ye ;
 dz = Zp - Ze ;
 if( fabs(dx) >= fabs(dy) ) z = Ze + dz * (x-Xe) / dx ;
 else                       z = Ze + dz * (y-Ye) / dy ;
/*
** Job Completed
*/
 return(z) ;   
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmVisibility_determinePointVisibilityDtmFile(WCharCP dtmFileP,double Xe,double Ye,double Ze,double Xp,double Yp,double Zp,long *IsVisible)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determining Point Visibility From Dtm File") ;
/*
** Write Arguements For Development Purposes
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Dtm File = %s",dtmFileP) ;
    bcdtmWrite_message(0,0,0,"Eye x    = %10.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Eye y    = %10.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Eye z    = %10.4lf",Ze) ;
    bcdtmWrite_message(0,0,0,"Point x  = %10.4lf",Xp) ;
    bcdtmWrite_message(0,0,0,"Point y  = %10.4lf",Yp) ;
    bcdtmWrite_message(0,0,0,"Point z  = %10.4lf",Zp) ;
   }
/*
** Test If Requested Dtm Is Current Dtm
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Process Point Visibility On Dtm Object
*/
 if( bcdtmVisibility_determinePointVisibilityDtmObject(dtmP,Xe,Ye,Ze,Xp,Yp,Zp,IsVisible)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 return(ret) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Point Visibility From Dtm File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Point Visibility From Dtm File Error") ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determinePointVisibilityDtmObject(BC_DTM_OBJ *dtmP,double Xe,double Ye,double Ze,double Xp,double Yp,double Zp,long *isVisibleP)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    drapeFlag,numDrapePts ;
 DPoint3d     stringPts[2] ;
 double  Zs ;
 DTM_DRAPE_POINT *drapeP,*drapePtsP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determining Point Visibility Dtm Object") ;
    bcdtmWrite_message(0,0,0,"DTM Object = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Eye x      = %10.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Eye y      = %10.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Eye z      = %10.4lf",Ze) ;
    bcdtmWrite_message(0,0,0,"Point x    = %10.4lf",Xp) ;
    bcdtmWrite_message(0,0,0,"Point y    = %10.4lf",Yp) ;
    bcdtmWrite_message(0,0,0,"Point z    = %10.4lf",Zp) ;
   }
/*
** Initialise
*/
 *isVisibleP = 1 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
  if( dtmP->dtmState != DTMState::Tin )
    {
     bcdtmWrite_message(2,0,0,"DTM Object Not Triangulated") ;
     goto errexit ;
    }
/*
** Determine If Eye Is Within Tin Hull
*/
 if( bcdtmDrape_pointDtmObject(dtmP,Xe,Ye,&Zs,&drapeFlag)) goto errexit ;
 if( ! drapeFlag ) 
   { 
    bcdtmWrite_message(1,0,0,"Eye External To Tin or Internal To Void") ;
    *isVisibleP = 0 ; 
    goto errexit ;
   }
/*
** Check Eye Is Above Tin Surface
*/
 if( drapeFlag && Zs > Ze ) 
   {
    bcdtmWrite_message(1,0,0,"Eye Below Tin Surface") ;
    *isVisibleP = 0 ; 
    goto errexit ;
   }
/*
** Determine If Target Point Is Within Tin Hull
*/
 if( bcdtmDrape_pointDtmObject(dtmP,Xp,Yp,&Zs,&drapeFlag)) goto errexit ;
 if( ! drapeFlag ) 
   { 
    bcdtmWrite_message(1,0,0,"Target Point External To Tin or Internal To Void") ;
    *isVisibleP = 0 ;
    goto errexit ;
   }
/*
**  Check Target Point Is Above Surface
*/
 if( drapeFlag && Zs > Zp ) 
   {
    bcdtmWrite_message(1,0,0,"Target Point Below Tin Surface") ;
    *isVisibleP = 0 ; 
    goto errexit ;
   }
/*
**  Drape Line From Eye To Point On Tin
*/
 stringPts[0].x = Xe ; stringPts[0].y = Ye ; stringPts[0].z = Ze ;
 stringPts[1].x = Xp ; stringPts[1].y = Yp ; stringPts[1].z = Zp ;
 if( bcdtmDrape_stringDtmObject(dtmP,stringPts,2,FALSE,&drapePtsP,&numDrapePts)) goto errexit ;
/*
** Log Drape Angle
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Angle From Eye To Point = %12.10lf",bcdtmMath_getAngle(Xe,Ye,Xp,Yp)) ;
   } 
/*
**  Scan Drape Points
*/
 for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts && *isVisibleP == 1 ; ++drapeP )
   {
    if( drapeP->drapeZ > bcdtmVisibility_interpolateZOnLineOfSight(Xe,Ye,Ze,Xp,Yp,Zp,drapeP->drapeX,drapeP->drapeY)) *isVisibleP = 0 ; 
   }
/*
** Clean Up
*/
 cleanup :
 if( drapePtsP != nullptr ) 
   {
    bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
   } 
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Point Visibility Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Point Visibility Dtm Object Error") ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineLineVisibiltyDtmFile
(
 WCharCP dtmFileP,
 double Xe,
 double Ye,
 double Ze,
 double X1,
 double Y1,
 double Z1,
 double X2,
 double Y2,
 double Z2,
 long *isVisibleP,
 DTMFeatureCallback loadFunctionP,
 void *userP
)
/*
** This Function Determines The Visibility Of A Line XlY1Z1-X2Y2Z2 From The Eye XeYeZe
**
** isVisibleP = -1  Line Not Visible
**            =  0  Line Partially Visible
**            =  1  Line Visible 
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determining Line Visibility Dtm File") ;
/*
** Write Arguements For Development Purposes
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Dtm File      = %s",dtmFileP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"Eye x         = %10.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Eye y         = %10.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Eye z         = %10.4lf",Ze) ;
    bcdtmWrite_message(0,0,0,"X1            = %10.4lf",X1) ;
    bcdtmWrite_message(0,0,0,"Y1            = %10.4lf",Y1) ;
    bcdtmWrite_message(0,0,0,"Z1            = %10.4lf",Z1) ;
    bcdtmWrite_message(0,0,0,"X2            = %10.4lf",X2) ;
    bcdtmWrite_message(0,0,0,"Y2            = %10.4lf",Y2) ;
    bcdtmWrite_message(0,0,0,"Z2            = %10.4lf",Z2) ;
    bcdtmWrite_message(0,0,0,"userP         = %p",userP) ;
   }
/*
** Test If Requested Dtm Is Current Dtm
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Line Visibility On Dtm Object
*/
 if( bcdtmVisibility_determineLineVisibiltyDtmObject(dtmP,Xe,Ye,Ze,X1,Y1,Z1,X2,Y2,Z2,isVisibleP,loadFunctionP,userP)) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineLineVisibiltyDtmObject
(
 BC_DTM_OBJ *dtmP,
 double Xe,
 double Ye,
 double Ze,
 double X1,
 double Y1,
 double Z1,
 double X2,
 double Y2,
 double Z2,
 long *isVisibleP,
 DTMFeatureCallback loadFunctionP,
 void *userP
)
/*
** This Function Determines The Visibility Of A Line XlY1Z1-X2Y2Z2 From The Eye XeYeZe
**
** isVisibleP = -1  Line Not Visible
**            =  0  Line Partially Visible
**            =  1  Line Visible 
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   isPointVisible,c0,c1,*pl,lastVisibility ;  
 double Xp,Yp,Zp,dx,dy,dz,llen,linc,ldist,stroke=100 ;
 DPoint3d    *p3dP,loadPts[2] ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determining Line Visibility DTM Object") ;
    bcdtmWrite_message(0,0,0,"Dtm Object    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Eye x         = %10.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Eye y         = %10.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Eye z         = %10.4lf",Ze) ;
    bcdtmWrite_message(0,0,0,"X1            = %10.4lf",X1) ;
    bcdtmWrite_message(0,0,0,"Y1            = %10.4lf",Y1) ;
    bcdtmWrite_message(0,0,0,"Z1            = %10.4lf",Z1) ;
    bcdtmWrite_message(0,0,0,"X2            = %10.4lf",X2) ;
    bcdtmWrite_message(0,0,0,"Y2            = %10.4lf",Y2) ;
    bcdtmWrite_message(0,0,0,"Z2            = %10.4lf",Z2) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"userP         = %p",userP) ;
   }
/*
** Initialise
*/
 ldist = 0.0 ;
 *isVisibleP = 0 ;
 /*
** Calculate 3D Line Length And Line Increments
*/
 dx = X2 - X1 ; 
 dy = Y2 - Y1 ; 
 dz = Z2 - Z1 ;
 llen = sqrt(dx*dx+dy*dy+dz*dz) ;
 linc = llen / stroke ;
 dx = dx / stroke ;
 dy = dy / stroke ;
 dz = dz / stroke ;
/*
** Clear Memory
*/
 if( bcdtmVisibility_storeVertice(100,0,0.0,0.0,0.0) ) goto errexit ; 
/*
** Scan Line And Determine Visibility Of Points On Line
*/
 Xp = X1 ;
 Yp = Y1 ;
 Zp = Z1 ; 
 if(bcdtmVisibility_determinePointVisibilityDtmObject(dtmP,Xe,Ye,Ze,Xp,Yp,Zp,&isPointVisible)) goto errexit ;
 if( bcdtmVisibility_storeVertice(1,isPointVisible,Xp,Yp,Zp) ) goto errexit ; 
 lastVisibility = isPointVisible ;
 while( ldist < llen )
   {
    ldist = ldist + linc ;
    if( ldist >= llen || (llen-ldist) < linc ) { Xp = X2 ; Yp = Y2 ; Zp = Z2 ; }
    else                                       { Xp = Xp + dx ; Yp = Yp + dy ; Zp = Zp + dz ; }
    if(bcdtmVisibility_determinePointVisibilityDtmObject(dtmP,Xe,Ye,Ze,Xp,Yp,Zp,&isPointVisible)) goto errexit ;
    if( isPointVisible != lastVisibility )
      {
       if( bcdtmVisibility_storeVertice(1,lastVisibility,Xp,Yp,Zp) ) goto errexit ; 
       if( bcdtmVisibility_storeVertice(1,isPointVisible,Xp,Yp,Zp) ) goto errexit ; 
       lastVisibility = isPointVisible ;
      }
   } 
 if( bcdtmVisibility_storeVertice(1,isPointVisible,X2,Y2,Z2) ) goto errexit ; 
/*
** Determine Line Visibility
*/
 c0 = c1 = 0 ;
 for( pl = losLinesP ; pl < losLinesP + numLosPts ; ++pl )
   {
    if( *pl ) ++c1 ; 
    else      ++c0 ;
   }
 if     ( c1 == numLosPts ) *isVisibleP =  1 ;
 else if( c0 == numLosPts ) *isVisibleP = -1 ;
 else                       *isVisibleP =  0 ; 
/*
** Load Visibility Lines
*/
 if( loadFunctionP != nullptr  )
   {
    for( p3dP = losPtsP , pl = losLinesP ; p3dP < losPtsP + numLosPts ; p3dP = p3dP + 2 , pl = pl + 2  )
      {
       loadPts[0].x = p3dP->x     ; loadPts[0].y = p3dP->y     ; loadPts[0].z = p3dP->z ; 
       loadPts[1].x = (p3dP+1)->x ; loadPts[1].y = (p3dP+1)->y ; loadPts[1].z = (p3dP+1)->z ; 
       if( *pl ) { if( loadFunctionP(DTMFeatureType::VisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,loadPts,2,userP)) goto errexit ; }
       else      { if( loadFunctionP(DTMFeatureType::InvisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,loadPts,2,userP)) goto errexit ; }
      }
   } 
/*
** Clean Up
*/
 cleanup :
 bcdtmVisibility_freeLosVertices() ;
/*
** Job Completed
*/
 return(ret) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Line Visibility DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Line Visibility DTM Object Error") ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineVisibilityTinPointsDtmFile
(
 WCharCP dtmFileP,
 double Xe,
 double Ye,
 double Ze,
 DTMFeatureCallback loadFunctionP,
 void   *userP
)
/*
** This Function Determines The Visibility Of All Dtm Points
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determining Visibility Tin Points From Dtm File") ;
/*
** Test If Requested Dtm Is Current Dtm
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
**  Determine Visibility Of All Dtm Points
*/
 if( bcdtmVisibility_determineVisibilityTinPointsDtmObject(dtmP,Xe,Ye,Ze,loadFunctionP,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job ComlosLinePeted
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Visibility Dtm Points From Dtm File ComlosLinePeted") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Visibility Dtm Points From Dtm File Error") ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineVisibilityTinPointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 double Xe,
 double Ye,
 double Ze,
 DTMFeatureCallback loadFunctionP,
 void  *userP
)
/*
** This Function Determines The Visibility Of All Dtm Points
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   point,isVisible,drapeFlag,startTime ;
 long   numVisiblePoints=0,numInvisiblePoints=0 ;
 double x,y,z,Zs  ;
 DPoint3d    dtmPoint[2]  ;
 DTM_TIN_NODE  *nodeP ;
 DTM_TIN_POINT  *pointP ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determining Visibility Of Tin Points") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Eye x         = %10.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Eye y         = %10.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Eye z         = %10.4lf",Ze) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"userP         = %p",userP) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
  if( dtmP->dtmState != DTMState::Tin )
    {
     bcdtmWrite_message(2,0,0,"DTM Object Not Triangulated") ;
     goto errexit ;
    }
/*
** Determine If Eye Is Inside Tin Hull
*/
 if( bcdtmDrape_pointDtmObject(dtmP,Xe,Ye,&Zs,&drapeFlag)) goto errexit ;
 if( drapeFlag == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Eye External To Tin Or Internal To Void") ;
    goto errexit ;
   }
 if( drapeFlag  && Zs > Ze )  
   { 
    bcdtmWrite_message(1,0,0,"Eye Below Tin Surface") ; 
    goto errexit ;
   }
/*
** Check Load Function Is Set
*/
 if( loadFunctionP == nullptr )
   {
    bcdtmWrite_message(2,0,0,"No Load Function Set") ;
    goto errexit ;
   }
/*
** Build Visibility Tables For Dtm 
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Visibility Tables") ;
 if( bcdtmVisibility_buildVisibilityTablesForDtmObject(dtmP,Xe,Ye,Ze)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Build Visibility Tables = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Process All Dtm Points
*/
 startTime = bcdtmClock() ;
 for( point =0  ; point < dtmP->numPoints ; ++point)
   {
    nodeP = nodeAddrP(dtmP,point) ;
    pointP = pointAddrP(dtmP,point) ; 
    if( dbg && ( point % 10000 == 0 || point + 1 == dtmP->numPoints) ) bcdtmWrite_message(0,0,0,"Processing Point %8ld of %8ld",point,dtmP->numPoints) ;
/*
**  Determine Dtm Point Visibility
*/
    if( ! bcdtmFlag_testVoidBitPCWD(&nodeP->PCWD) )
      {
       x = pointP->x ;
       y = pointP->y ; 
       z = pointP->z + 0.000001 ;
       if( bcdtmVisibility_determinePointVisibilityUsingVisibilityTables(horLinesP,numHorLines,horLinesIndexP,numHorLinesIndex,hozIndexListP,Xe,Ye,Ze,x,y,z,&isVisible)) goto errexit ;
/*
**    Load Visibility Points
*/
      dtmPoint[0].x = x ; 
      dtmPoint[0].y = y ; 
      dtmPoint[0].z = pointP->z ; 
      dtmPoint[1].x = x + dtmP->ppTol ; 
      dtmPoint[1].y = z + dtmP->ppTol ; 
      dtmPoint[1].z = pointP->z ; 
      if( isVisible ) { if( loadFunctionP(DTMFeatureType::VisiblePoint,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,dtmPoint,1,userP)) goto errexit ; }
      else            { if( loadFunctionP(DTMFeatureType::InvisiblePoint,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,dtmPoint,1,userP)) goto errexit ; }
      if( isVisible ) ++numVisiblePoints ;
      else            ++numInvisiblePoints ;
     } 
  }
/*
** Write Timing Information
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Number Of Visible Points   = %8ld",numVisiblePoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Invisible Points = %8ld",numInvisiblePoints) ;
    bcdtmWrite_message(0,0,0,"Time To Determine Visibility All Dtm Points = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job ComlosLinePeted
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Visibility Of Tin Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Visibility Of Tin Points Error") ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineVisibilityTinLinesDtmFile
(
 WCharCP dtmFileP,
 double Xe,
 double Ye,
 double Ze,
 DTMFeatureCallback loadFunctionP,
 void   *userP
)
/*
** This Function Determines The Visibility Of All Dtm Points
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determining Visibility Tin Lines From Dtm File") ;
/*
** Test If Requested Dtm Is Current Dtm
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
**  Determine Visibility Of All Dtm Points
*/
 if( bcdtmVisibility_determineVisibilityTinLinesDtmObject(dtmP,Xe,Ye,Ze,loadFunctionP,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job ComlosLinePeted
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Visibility Dtm Points From Dtm File ComlosLinePeted") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Visibility Dtm Points From Dtm File Error") ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineVisibilityTinLinesDtmObject
(
 BC_DTM_OBJ *dtmP,
 double Xe,
 double Ye,
 double Ze,
 DTMFeatureCallback loadFunctionP,
 void  *userP
)
/*
** This Function Determines The Visibility Of All Dtm Lines
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,clc,*losLineP,isVisible,voidLine,drapeFlag,startTime,lineNum=0 ;
 double X1,Y1,Z1,X2,Y2,Z2,Zs ;
 DPoint3d    *p3dP,linePts[2]  ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determining Visibility Of Tin Lines") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Eye x         = %10.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Eye y         = %10.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Eye z         = %10.4lf",Ze) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"userP         = %p",userP) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
  if( dtmP->dtmState != DTMState::Tin )
    {
     bcdtmWrite_message(2,0,0,"DTM Object Not Triangulated") ;
     goto errexit ;
    }
/*
** Determine If Eye Is Inside Tin Hull
*/
 if( bcdtmDrape_pointDtmObject(dtmP,Xe,Ye,&Zs,&drapeFlag)) goto errexit ;
 if( drapeFlag == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Eye External To Tin Or Internal To Void") ;
    goto errexit ;
   }
 if( drapeFlag  && Zs > Ze )  
   { 
    bcdtmWrite_message(1,0,0,"Eye Below Tin Surface") ; 
    goto errexit ;
   }
/*
** Check Load Function Is Set
*/
 if( loadFunctionP == nullptr )
   {
    bcdtmWrite_message(2,0,0,"No Load Function Set") ;
    goto errexit ;
   }
/*
** Build Visibility Tables For Dtm 
*/
 if( bcdtmVisibility_buildVisibilityTablesForDtmObject(dtmP,Xe,Ye,Ze)) goto errexit ; 
/*
** Scan Tin Lines And Determine Their Visibility
*/
 startTime = bcdtmClock() ;
 lineNum = 0 ;
 for( p1 = 0 ; p1 <  dtmP->numPoints  ; ++p1 )
   {
    if( ( clc = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr )
      {
       X1 = pointAddrP(dtmP,p1)->x ;
       Y1 = pointAddrP(dtmP,p1)->y ;
       Z1 = pointAddrP(dtmP,p1)->z + 0.00001 ;
       while ( clc != dtmP->nullPtr )
         {
          p2  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( p2 > p1 )
            { 
             if( dbg && ( lineNum % 10000 == 0 || lineNum + 1 == dtmP->numLines) ) bcdtmWrite_message(1,0,0,"Processing Line %8ld of %8ld",lineNum+1,dtmP->numLines) ;
             ++lineNum ;  
             if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidLine)) goto errexit ;
             if( ! voidLine ) 
               { 
                X2 = pointAddrP(dtmP,p2)->x ;
                Y2 = pointAddrP(dtmP,p2)->y ;
                Z2 = pointAddrP(dtmP,p2)->z + 0.00001 ;
                if( bcdtmVisibility_determineLineVisibilityUsingVisibilityTables(horLinesP,numHorLines,horLinesIndexP,numHorLinesIndex,hozIndexListP,Xe,Ye,Ze,X1,Y1,Z1,X2,Y2,Z2,&isVisible)) goto errexit ;
/*
**              Load Tin Line Visibility
*/
                for( p3dP = losPtsP , losLineP = losLinesP ; p3dP < losPtsP + numLosPts ; p3dP = p3dP + 2 , losLineP = losLineP +2  )
                  {
                   linePts[0].x = p3dP->x     ; linePts[0].y = p3dP->y     ; linePts[0].z = p3dP->z ;
                   linePts[1].x = (p3dP+1)->x ; linePts[1].y = (p3dP+1)->y ; linePts[1].z = (p3dP+1)->z ;
                   if( *losLineP ) { if( loadFunctionP(DTMFeatureType::VisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,linePts,2,userP)) goto errexit ; }
                   else            { if( loadFunctionP(DTMFeatureType::InvisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,linePts,2,userP)) goto errexit ; }
                  }
               }
            }
         }
      } 
   }
/*
** Write Elapsed Time ** Developement Purposes Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Determine Visibility All Tin Lines = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job ComlosLinePeted
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Visibility Tin Lines From Dtm File ComlosLinePeted") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Visibility Tin Lines From Dtm File Error") ;
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
BENTLEYDTM_Private int bcdtmVisibility_storeVertice(long actionFlag,long visibility,double x,double y,double z) 
/*
** This Function Stores A Visibility Vertice
*/
{
 int ret=DTM_SUCCESS ;
/*
** Check For CleanUp Call
*/
 if( actionFlag == 100 ) { numLosPts = 0 ; goto cleanup ; }
//bcdtmWrite_message(0,0,0,"actionFlag = %3ld ** Storing x = %12.5lf y = %12.5lf z = %10.4lf",actionFlag,x,y,z) ;
/*
** Check For Sufficient Heap Space
*/
 if( numLosPts == memLosPts )
   {
    memLosPts = memLosPts + memLosPtsInc ;
    if( losPtsP == nullptr )  
      {
       losPtsP   = (  DPoint3d  * )  malloc  ( memLosPts * sizeof(DPoint3d)) ;
       losLinesP = ( long  * )  malloc  ( memLosPts * sizeof(long)) ;
      }
    else
      {
       losPtsP   = (  DPoint3d  * )  realloc  ( losPtsP, memLosPts * sizeof(DPoint3d)) ;
       losLinesP = ( long  * )  realloc  ( losLinesP,memLosPts * sizeof(long)) ;
      }    
    if( losPtsP == nullptr || losLinesP == nullptr )
      {
       if( losPtsP   != nullptr ) { free(losPtsP)   ; losPtsP = nullptr ; }
       if( losLinesP != nullptr ) { free(losLinesP) ; losPtsP = nullptr ; }
       numLosPts = memLosPts = 0 ;
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
       goto errexit ; 
      }
   }
/*
** Check If Visibility Has Changed
*/
 if( actionFlag == 0 && numLosPts > 0 )
   {
    if( visibility == *(losLinesP+numLosPts-1) ) goto cleanup ; 
   }
 if( actionFlag == 2 )
   {
    if( x == (losPtsP+numLosPts-1)->x && y == (losPtsP+numLosPts-1)->y && z == (losPtsP+numLosPts-1)->z ) goto cleanup ;
   }
/*
** Store Point
*/
 *(losLinesP+numLosPts) = visibility ;
 (losPtsP+numLosPts)->x = x ;
 (losPtsP+numLosPts)->y = y ;
 (losPtsP+numLosPts)->z = z ;
 ++numLosPts ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineRadialViewShedsDtmFile
(
 WCharCP dtmFileP,
 double Xe,
 double Ye,
 double Ze,
 long   viewShedOption,
 long   numberRadials,
 double radialIncrement,
 DTMFeatureCallback loadFunctionP,
 void   *userP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0)  ;
 BC_DTM_OBJ *dtmP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determining Radial View Sheds From Dtm File") ;
/*
** Test If Requested Dtm Is Current Dtm
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Process Point Visibility On Dtm Object
*/
 if( bcdtmVisibility_determineRadialViewShedsDtmObject(dtmP,Xe,Ye,Ze,viewShedOption,numberRadials,radialIncrement,loadFunctionP,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Radial View Sheds From Dtm File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Radial View Sheds From Dtm File Error") ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineRadialViewShedsDtmObject
(
 BC_DTM_OBJ *dtmP,
 double Xe,
 double Ye,
 double Ze,
 long   viewShedOption,
 long   numberRadials,
 double radialIncrement,
 DTMFeatureCallback loadFunctionP,
 void   *userP
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   *pl,scan,process,drapeFlag,numDrapePts ;
 long   numLoadPts=0,memLoadPts=0,memLoadPtsInc=1000 ;
 double dd,dx,dy,dz,Zs,x,y,z,angle,anginc,radius,maxAngle,eyeAngle=0.0,lastEyeAngle=0.0 ;
 DPoint3d    *p3d,radial[2],*loadPtsP=nullptr ;
 long   start,finish ;
 DTM_DRAPE_POINT *drape1P,*drape2P,*drapePtsP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determining Radial View Sheds DTM Object") ;
    bcdtmWrite_message(0,0,0,"Dtm Object       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Eye x            = %10.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Eye y            = %10.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Eye z            = %10.4lf",Ze) ;
    bcdtmWrite_message(0,0,0,"viewShedOption   = %8ld",viewShedOption) ;
    bcdtmWrite_message(0,0,0,"numberRadials    = %8ld",numberRadials) ;
    bcdtmWrite_message(0,0,0,"radialIncrement  = %10.4lf",radialIncrement) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP    = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"userP            = %p",userP) ;
   }
/*
** Initialise
*/
 start = bcdtmClock() ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
  if( dtmP->dtmState != DTMState::Tin )
    {
     bcdtmWrite_message(2,0,0,"DTM Object Not Triangulated") ;
     goto errexit ;
    }
/*
** Determine If Eye Is Inside Tin Hull
*/
 if( bcdtmDrape_pointDtmObject(dtmP,Xe,Ye,&Zs,&drapeFlag)) goto errexit ;
 if( drapeFlag == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Eye External To Tin Or Internal To Void") ;
    goto errexit ;
   }
 if( drapeFlag  && Zs > Ze )  
   { 
    bcdtmWrite_message(1,0,0,"Eye Below Tin Surface") ; 
    goto errexit ;
   }
/*
** Check Load Function Is Set
*/
 if( loadFunctionP == nullptr )
   {
    bcdtmWrite_message(2,0,0,"No Load Function Set") ;
    goto errexit ;
   }
/*
** Initialise Radial Scan Parameters
*/
 if( numberRadials   < 0   || numberRadials > 100000  ) numberRadials = 1000 ;
 if( radialIncrement < 0.0 || radialIncrement > 360.0 ) radialIncrement = 360.0 / 1000.0 ;
 angle  = 0.0 ;
 if( viewShedOption == 1 ) anginc = DTM_2PYE / numberRadials ;
 else                      anginc = DTM_2PYE * radialIncrement / 360.0 ;
 radius = sqrt((dtmP->xMax-dtmP->xMin)*(dtmP->xMax-dtmP->xMin)+(dtmP->yMax-dtmP->yMin)*(dtmP->yMax-dtmP->yMin)) ;
/*
** Scan Radially About Eye
*/ 
 while( angle <= DTM_2PYE )
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Angle = %10.8lf",angle) ;
/*
**  Set Radial Coordinates
*/
    radial[0].x = Xe ;
    radial[0].y = Ye ;
    radial[0].z = 0.0 ;
    radial[1].x = Xe + radius * cos(angle) ; 
    radial[1].y = Ye + radius * sin(angle) ;
    radial[1].z = 0.0 ;
/*
**  Drape Radial On Tin Surface
*/
    if( bcdtmDrape_stringDtmObject(dtmP,radial,2,FALSE,&drapePtsP,&numDrapePts)) goto errexit ;
/*
**  Remove Drape End Points Not On Tin
*/
    drape1P  = drapePtsP + numDrapePts - 1 ;
    while (drape1P->drapeType == DTMDrapedLineCode::External) { --drape1P; --numDrapePts; }
/*
**  Remove Duplicate Drape Points
*/
    for( drape1P = drape2P = drapePtsP ; drape2P < drapePtsP + numDrapePts ; ++drape2P )
      {
       if( drape2P->drapeX != drape1P->drapeX || drape2P->drapeY != drape1P->drapeY )
         {
          ++drape1P ;
          *drape1P = *drape2P ;
         }
      }
    numDrapePts = (long)(drape1P-drapePtsP)  ;
/*
**  Determine Visibility Of Draped Radial Points
*/
    if( bcdtmVisibility_storeVertice(100,0,0.0,0.0,0.0) ) goto errexit ;
    if( bcdtmVisibility_storeVertice(1,1,drapePtsP->drapeX,drapePtsP->drapeY,drapePtsP->drapeZ) ) goto errexit ;
    dx = (drapePtsP+1)->drapeX - Xe ;
    dy = (drapePtsP+1)->drapeY - Ye ;
    dz = (drapePtsP+1)->drapeZ - Ze ;
    dd = sqrt(dx*dx+dy*dy) ;
    maxAngle = atan2(dz,dd) ;
    drape1P = drapePtsP + 2 ;
    drape2P = drapePtsP + numDrapePts - 1 ;
    scan = 1 ;
    while ( scan )
      { 
/*
**     Scan Visible Drape Points
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Scanning Visible Draped Points") ; 
       if( drape1P <= drape2P ) 
         {
          process = 1 ; 
          while ( drape1P <= drape2P && process )
            {
             dx = drape1P->drapeX - Xe ;
             dy = drape1P->drapeY - Ye ;
             dz = drape1P->drapeZ - Ze ;
             dd = sqrt(dx*dx+dy*dy) ;
             eyeAngle = atan2(dz,dd) ;
             if( eyeAngle >= maxAngle ) { maxAngle = eyeAngle ; ++drape1P ; }
             else                        process = 0 ; 
            }  
          --drape1P ;
          if( bcdtmVisibility_storeVertice(1,1,drape1P->drapeX,drape1P->drapeY,drape1P->drapeZ) ) goto errexit ;
          if( drape1P < drape2P ) if( bcdtmVisibility_storeVertice(1,0,drape1P->drapeX,drape1P->drapeY,drape1P->drapeZ) ) goto errexit ;
          ++drape1P ; 
         } 
/*
**     Scan Invisible Drape Points
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Scanning Invisible Draped Points") ; 
       if( drape1P <= drape2P )  
         {
          process = 1 ; 
          while ( drape1P <= drape2P && process )
            {
             dx = drape1P->drapeX - Xe ;
             dy = drape1P->drapeY - Ye ;
             dz = drape1P->drapeZ - Ze ;
             dd = sqrt(dx*dx+dy*dy) ;
             eyeAngle = atan2(dz,dd) ;
             if( eyeAngle < maxAngle ) { lastEyeAngle = eyeAngle ; ++drape1P ; }
             else                        process = 0 ; 
            }
/*
**        Calculate Max Angle Intercept On Radial
*/
          if( process )
            {
             --drape1P ;
             if( bcdtmVisibility_storeVertice(1,0,drape1P->drapeX,drape1P->drapeY,drape1P->drapeZ) ) goto errexit ;
             ++drape1P ;
            }
          else
            { 
             dx = maxAngle - lastEyeAngle ;
             dd = eyeAngle - lastEyeAngle ;
             x =  (drape1P-1)->drapeX + (drape1P->drapeX - (drape1P-1)->drapeX) * dx / dd ;
             y =  (drape1P-1)->drapeY + (drape1P->drapeY - (drape1P-1)->drapeY) * dx / dd ;
             z =  (drape1P-1)->drapeZ + (drape1P->drapeZ - (drape1P-1)->drapeZ) * dx / dd ;
             if( bcdtmVisibility_storeVertice(1,0,x,y,z) ) goto errexit ;
             if( bcdtmVisibility_storeVertice(1,1,x,y,z) ) goto errexit ;
            } 
         }
/*
**     Test For End Of Scan
*/
       if( drape1P > drape2P ) scan = 0 ;
      } 
/*
** Load Visibility Lines
*/
 for( p3d = losPtsP , pl = losLinesP ; p3d < losPtsP + numLosPts ; p3d = p3d + 2 , pl = pl +2  )
   {
    if( bcdtmLoad_storeFeaturePoint(p3d->x,p3d->y,p3d->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
    if( bcdtmLoad_storeFeaturePoint((p3d+1)->x,(p3d+1)->y,(p3d+1)->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
    if( *pl ) { if( loadFunctionP(DTMFeatureType::VisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,loadPtsP,numLoadPts,userP)) goto errexit ; }
    else      { if( loadFunctionP(DTMFeatureType::InvisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,loadPtsP,numLoadPts,userP)) goto errexit ; }
    numLoadPts = 0 ;
   } 
/*
**  Increment Angle For Next Radial
*/
    angle = angle + anginc ;
/*
**  Free Drape Points
*/
    if( drapePtsP != nullptr ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
   } 
/*
** Get Elapsed Time ** Developement Purposes Only
*/
 finish = bcdtmClock() ;
 if( tdbg )bcdtmWrite_message(0,0,0,"Time To Radial Sweep View Shed = %7.3lf seconds",bcdtmClock_elapsedTime(finish,start)) ;
/*
** Clean Up
*/
 cleanup :
 if( loadPtsP != nullptr ) { free(loadPtsP) ; loadPtsP = nullptr ; }
 if( drapePtsP != nullptr ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Radial View Sheds From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Radial View Sheds From Dtm Object Error") ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineRegionViewShedsDtmFile
(
 WCharCP dtmFileP,
 DTMFeatureCallback loadFunctionP,
 double Xe,
 double Ye,
 double Ze,
 void   *userP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=nullptr ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Determining Region View Sheds From DTM File") ;
    bcdtmWrite_message(0,0,0,"Dtm File      = %s",dtmFileP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"Eye x         = %10.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Eye y         = %10.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Eye z         = %10.4lf",Ze) ;
   }
/*
** Test If Requested Dtm Is Current Dtm
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Process Point Visibility On Dtm Object
*/
 if( bcdtmVisibility_determineRegionViewShedsDtmObject(dtmP,Xe,Ye,Ze,loadFunctionP,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Radial View Sheds From Dtm File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Radial View Sheds From Dtm File Error") ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineRegionViewShedsDtmObject
(
 BC_DTM_OBJ *dtmP,
 double Xe,
 double Ye,
 double Ze,
 DTMFeatureCallback loadFunctionP,
 void   *userP
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   drapeFlag,startTime=0,processTime=bcdtmClock() ;
 double Zs ;
 BC_DTM_OBJ *tempDtmP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Determining Region View Sheds From DTM Object") ;
    bcdtmWrite_message(0,0,0,"Dtm Object       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP    = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"Eye x            = %10.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Eye y            = %10.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Eye z            = %10.4lf",Ze) ;
    bcdtmWrite_message(0,0,0,"userP            = %p",userP) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
  if( dtmP->dtmState != DTMState::Tin )
    {
     bcdtmWrite_message(2,0,0,"DTM Object Not Triangulated") ;
     goto errexit ;
    }
/*
** Determine If Eye Is Inside Tin Hull
*/
 if( bcdtmDrape_pointDtmObject(dtmP,Xe,Ye,&Zs,&drapeFlag)) goto errexit ;
 if( drapeFlag == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Eye External To Tin Or Internal To Void") ;
    goto errexit ;
   }
 if( drapeFlag && Zs > Ze )  
   { 
    bcdtmWrite_message(1,0,0,"Eye Below Tin Surface") ; 
    goto errexit ;
   }
/*
** Check Load Function Is Set
*/
 if( loadFunctionP == nullptr )
   {
    bcdtmWrite_message(2,0,0,"Load Function Not set") ;
    goto errexit ;
   }
/*
** Clone DTM Object
*/
 if( bcdtmObject_cloneDtmObject(dtmP,&tempDtmP)) goto errexit ;
/*
** Calculate Visibility Tables For Tin
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Visibility Tables For DTM Object") ;
 if( bcdtmVisibility_buildVisibilityTablesForDtmObject(tempDtmP,Xe,Ye,Ze)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Build Visibility Tables  = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Refine Tin For View Sheds
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Refining Tin For Region Visibility") ;
 if( bcdtmVisibility_refineTinForRegionVisibilityDtmObject(tempDtmP,Xe,Ye,Ze,horLinesIndexP,numHorLinesIndex,hozIndexListP) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Refine Tin For Region Visibility = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Polygonise And Load Region Visibility
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Tin Region Visibility") ;
 if( bcdtmVisibility_polygoniseAndLoadRegionVisibilityFromDtmObject(tempDtmP,Xe,Ye,Ze,horLinesIndexP,numHorLinesIndex,hozIndexListP,loadFunctionP,userP)) goto errexit ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Time To Polygonise Regional Visibility   = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
    bcdtmWrite_message(0,0,0,"Time To Process    Regional Visibility   = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),processTime) ) ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( tempDtmP != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Region View Sheds From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Region View Sheds From Dtm Object Error") ;
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
BENTLEYDTM_Private int bcdtmVisibility_buildVisibilityTablesForDtmObject
(
 BC_DTM_OBJ *dtmP,
 double Xe,
 double Ye,
 double Ze
)
/*
** This Function Calculates The Visibility Tables For A View Point
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  p1,p2,ap,cp,clc,visibiltyAnt=0,visibiltyClk=0,visibiltyRidge ;
 long  dtmFeature,voidsInDtm,voidLine,startTime=bcdtmClock() ;
 DPoint3d   breakPts[2] ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ *dataDtmP=nullptr ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Log Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Building Visibility Tables") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Xe         = %12.5lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Ye         = %12.5lf",Ye) ;
   }  
/*
** Only Build Visibility Tables For Tin If Necessary
*/
 if( Xe == eyeX && Ye == eyeY && Ze == eyeZ && losDtmP == dtmP && horLinesP != nullptr && horLinesIndexP != nullptr && hozIndexListP != nullptr ) goto cleanup ;
/*
** Check For Voids In DTM
*/
 voidsInDtm = FALSE ; 
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && voidsInDtm == FALSE ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ))
      {
       voidsInDtm = TRUE ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;  
/*
** Initialise Visibility Data Structures
*/
 bcdtmVisibility_freeMemory() ;
 eyeX = Xe ; 
 eyeY = Ye ; 
 eyeZ = Ze ; 
 losDtmP = dtmP ;
 tinRadius = sqrt((dtmP->xMax-dtmP->xMin)*(dtmP->xMax-dtmP->xMin) + (dtmP->yMax-dtmP->yMin)*(dtmP->yMax-dtmP->yMin) ) ;
/*
** Create Data Object
*/
 if( bcdtmObject_createDtmObject(&dataDtmP)) goto errexit ; 
/*
** Write Visibility Ridges To Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determining Horizon Lines") ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    if( (clc = nodeAddrP(dtmP,p1)->cPtr) != dtmP->nullPtr )
      {
       while ( clc != dtmP->nullPtr )
         {
          p2  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if(  p1 < p2  )
            {
             voidLine = FALSE ;
             if( voidsInDtm == TRUE )
               {
                if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidLine)) goto errexit ;
               } 
             if( voidLine == FALSE  )
               {  
             if( ( ap = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
             if( ( cp = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
             if( ! bcdtmList_testLineDtmObject(dtmP,p2,ap)) ap = dtmP->nullPnt ; 
             if( ! bcdtmList_testLineDtmObject(dtmP,p2,cp)) cp = dtmP->nullPnt ; 
             if( ap != dtmP->nullPnt ) bcdtmVisibility_determineVisibilityDirectionOfTriangleDtmObject(dtmP,Xe,Ye,Ze,p1,p2,ap,&visibiltyAnt) ;
             if( cp != dtmP->nullPnt ) bcdtmVisibility_determineVisibilityDirectionOfTriangleDtmObject(dtmP,Xe,Ye,Ze,p1,cp,p2,&visibiltyClk) ;
             visibiltyRidge = 0 ;
             if     ( ap == dtmP->nullPnt && cp != dtmP->nullPnt && visibiltyClk ) visibiltyRidge = 1 ;
             else if( ap != dtmP->nullPnt && cp == dtmP->nullPnt && visibiltyAnt ) visibiltyRidge = 1 ;
             else if( ap != dtmP->nullPnt && cp != dtmP->nullPnt && visibiltyAnt != visibiltyClk ) visibiltyRidge = 1 ;
             if( visibiltyRidge )
               {
                breakPts[0].x = pointAddrP(dtmP,p1)->x ; breakPts[0].y = pointAddrP(dtmP,p1)->y ; breakPts[0].z = pointAddrP(dtmP,p1)->z ;
                breakPts[1].x = pointAddrP(dtmP,p2)->x ; breakPts[1].y = pointAddrP(dtmP,p2)->y ; breakPts[1].z = pointAddrP(dtmP,p2)->z ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(dataDtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,breakPts,2)) goto errexit ;
               }  
            }  
         }
      }
   } 
   } 
/*
** Log Number Of Visibility Edges
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Visibility Ridges = %8ld",dataDtmP->numFeatures) ;
    if( bcdtmWrite_geopakDatFileFromDtmObject(dataDtmP,L"visibilityRidges.dat")) goto errexit ;
   } 
/*
** Build Horizon Table
*/
 if( dbg) bcdtmWrite_message(0,0,0,"Building Horizon Table") ;
 if( bcdtmVisibility_buildHorizonTableFromDtmObject(dataDtmP,Xe,Ye,&horLinesP,&numHorLines,&memHorLines) ) goto errexit ;
 if( dbg) bcdtmWrite_message(0,0,0,"Building Horizon Table Completed") ;
/*
**  Remove Invisible Horizon Lines
*/
 if( dbg) bcdtmWrite_message(0,0,0,"Removing Invisible Horizon Lines") ;
 if( bcdtmVisibility_removeInvisibleHorizonLines(&horLinesP,&numHorLines,&horLinesIndexP,&numHorLinesIndex,&hozIndexListP,&numHorIndexList,&memHorIndexList,Xe,Ye,Ze)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Invisible Horizon Lines Completed") ;
/*
** Write Statistics Debugging Purposes Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Final Number Of Horizon Lines = %6ld",numHorLines) ;
    bcdtmWrite_message(0,0,0,"Time To Build Visibility Tables = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dataDtmP != nullptr ) bcdtmObject_destroyDtmObject(&dataDtmP);
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmVisibility_freeMemory() ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmVisibility_buildHorizonTableFromDtmObject
(
 BC_DTM_OBJ *dataDtmP,
 double Xe,
 double Ye,
 DTM_HORIZON_LINE **horLinesPP,
 long *numHorLinesP,
 long *memHorLinesP
) 
/*
** This Function Builds The Horizon Table From Horizon Lines Stored In A Dtm Object
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2 ; 
 double D1,D2,X1,Y1,Z1,X2,Y2,Z2,Ang1,Ang2,ratio ; 
 DTM_HORIZON_LINE  *hLineP ;
/*
** Initialise
*/
 *numHorLinesP = 0 ;
 *numHorLinesP = 0 ;  
 if( *horLinesPP != nullptr ) { free(*horLinesPP) ; *horLinesPP = nullptr ; }
/*
** Copy Horizon Line Coordinates To Horizon Table Data Structure
*/
 for( p1 = 0 ; p1 < dataDtmP->numPoints ; p1 = p1 + 2 )
   {
    p2 = p1 + 1 ; 
    if( bcdtmMath_sideOf(pointAddrP(dataDtmP,p1)->x,pointAddrP(dataDtmP,p1)->y,pointAddrP(dataDtmP,p2)->x,pointAddrP(dataDtmP,p2)->y,Xe,Ye) != 0 )
      { 
       Ang1 = bcdtmMath_getAngle(Xe,Ye,pointAddrP(dataDtmP,p1)->x,pointAddrP(dataDtmP,p1)->y) ;   
       Ang2 = bcdtmMath_getAngle(Xe,Ye,pointAddrP(dataDtmP,p2)->x,pointAddrP(dataDtmP,p2)->y) ;
       X1   = pointAddrP(dataDtmP,p1)->x  ;
       Y1   = pointAddrP(dataDtmP,p1)->y  ;
       Z1   = pointAddrP(dataDtmP,p1)->z  ;
       D1   = bcdtmMath_distance(pointAddrP(dataDtmP,p1)->x,pointAddrP(dataDtmP,p1)->y,Xe,Ye) ; 
       X2   = pointAddrP(dataDtmP,p2)->x  ;
       Y2   = pointAddrP(dataDtmP,p2)->y  ;
       Z2   = pointAddrP(dataDtmP,p2)->z  ;
       D2   = bcdtmMath_distance(pointAddrP(dataDtmP,p2)->x,pointAddrP(dataDtmP,p2)->y,Xe,Ye) ; 
/*
**     Test And Set Eye-P1-P2 In An Anticlockwise Direction
*/
       if( bcdtmMath_sideOf(pointAddrP(dataDtmP,p1)->x,pointAddrP(dataDtmP,p1)->y,pointAddrP(dataDtmP,p2)->x,pointAddrP(dataDtmP,p2)->y,Xe,Ye) > 0 )
         {
          if( bcdtmVisibility_storeLineInHorizonTable(horLinesPP,numHorLinesP,memHorLinesP,Ang1,Ang2,D1,D2,X1,Y1,Z1,X2,Y2,Z2)) goto errexit ;
         }
       else
         {
          if( bcdtmVisibility_storeLineInHorizonTable(horLinesPP,numHorLinesP,memHorLinesP,Ang2,Ang1,D2,D1,X2,Y2,Z2,X1,Y1,Z1)) goto errexit ;
         }
/*
**     Test If Last Angle Equal To Zero
*/
       hLineP = *horLinesPP + *numHorLinesP - 1 ;
       if( hLineP->Ang2 < hLineP->Ang1 && hLineP->Ang2 == 0.0 ) hLineP->Ang2 = DTM_2PYE ;  
/*
**     Normalise Edge Lines Crossing 0 Degress
*/
       if( hLineP->Ang2 < hLineP->Ang1 )
         {
          ratio = (Ye - hLineP->Y1)/( hLineP->Y2 - hLineP->Y1) ;
          Ang2 = hLineP->Ang2 ;
          D2   = hLineP->D2 ;
          X2   = hLineP->X2 ;
          Y2   = hLineP->Y2 ;
          Z2   = hLineP->Z2 ; 
          hLineP->Ang2 = DTM_2PYE ;
          hLineP->X2   = hLineP->X1 + ratio * (X2-hLineP->X1) ;
          hLineP->Y2   = Ye  ;
          hLineP->Z2   = hLineP->Z1 + ratio * (Z2-hLineP->Z1) ;
          hLineP->D1   = bcdtmMath_distance(hLineP->X1,hLineP->Y1,Xe,Ye) ; 
          hLineP->D2   = bcdtmMath_distance(hLineP->X2,hLineP->Y2,Xe,Ye) ; 
          if( bcdtmMath_distance(hLineP->X1,hLineP->Y1,hLineP->X2,hLineP->Y2) == 0.0 ) --(*numHorLinesP) ;
          if( bcdtmVisibility_storeLineInHorizonTable(horLinesPP,numHorLinesP,memHorLinesP,0.0,Ang2,hLineP->D2,D2,hLineP->X2,hLineP->Y2,hLineP->Z2,X2,Y2,Z2)) goto errexit ;
         }
      }
   }
/*
** Report Normalised Horizon Lines   
*/
 if( dbg == 2 )
   {
    for( hLineP = *horLinesPP ; hLineP < *horLinesPP + *numHorLinesP ; ++hLineP )
      {
       if( hLineP->Ang1 == 0 || hLineP->Ang2 == DTM_2PYE )
         {
          bcdtmWrite_message(0,0,0,"Hline[%8ld] ** Ang1 = %12.10lf Ang2 = %12.10lf",(long)(hLineP-*horLinesPP),hLineP->Ang1,hLineP->Ang2) ;
         }
      }
   }
/*
** Write Statistics Debugging Purposes Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Initial Number Of Horizon Lines = %6ld",*numHorLinesP) ;
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
BENTLEYDTM_Private int bcdtmVisibility_storeLineInHorizonTable
(
 DTM_HORIZON_LINE **horLinesPP,
 long *numHorLinesP,
 long *memHorLinesP,
 double Ang1,
 double Ang2,
 double D1,
 double D2,
 double X1,
 double Y1,
 double Z1,
 double X2,
 double Y2,
 double Z2
) 
/*
** This Function Store A Line In The Horizon Table
*/
{
 int   ret=DTM_SUCCESS ;
 long  minc=5000 ;
 DTM_HORIZON_LINE *hLineP ;
/*
** Check For Sufficient Memory
*/
 if( *numHorLinesP == *memHorLinesP )
   {
    *memHorLinesP = *memHorLinesP + minc ;
    if( *horLinesPP == nullptr )  *horLinesPP = (DTM_HORIZON_LINE *) malloc  ( *memHorLinesP * sizeof(DTM_HORIZON_LINE)) ;
    else                       *horLinesPP = (DTM_HORIZON_LINE *) realloc ( *horLinesPP, *memHorLinesP * sizeof(DTM_HORIZON_LINE)) ;  
    if( *horLinesPP == nullptr ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
   }
/*
** Store Entry In Table
*/
 hLineP = *horLinesPP + *numHorLinesP ;
 hLineP->ActiveFlag = 1 ;
 hLineP->Ang1 = Ang1 ;
 hLineP->Ang2 = Ang2 ;
 hLineP->D1   = D1   ;
 hLineP->D2   = D2   ;
 hLineP->X1   = X1   ;
 hLineP->Y1   = Y1   ;
 hLineP->Z1   = Z1   ;
 hLineP->X2   = X2   ;
 hLineP->Y2   = Y2   ;
 hLineP->Z2   = Z2   ;
 ++(*numHorLinesP)   ;
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
BENTLEYDTM_Private int bcdtmVisibility_removeInvisibleHorizonLines
(
 DTM_HORIZON_LINE **horLinesPP,
 long *numHorLinesP,
 DTM_HORIZON_LINE_INDEX **horLinesIndexPP,
 long *numHorLinesIndexP,
 long **hozIndexListPP,
 long *numHorIndexListP,
 long *memHorIndexListP,
 double Xe,
 double Ye,
 double Ze
) 
/*
** This Function Removes All Invisible Horizon Lines Whose End Points
** Are Totally Within The Vision Angle
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   loop,process,firstSection,lastSection,hLineOffset,numLines,memLines,visibility ;
 long   numSaveHorLines=0,numTempHorLines=0,memTempHorLines=0,numHorDist=0 ;
 double radius,X1,Y1,Z1,X2,Y2,Z2,D1,D2,Ang1,Ang2 ;
 DPoint3d    visiblePts[2] ;
 DTM_HORIZON_LINE      *hLine1P,*hLine2P,*hozLineOfs1P,*hozLineOfs2P,*saveHorLinesP ;
 DTM_HORIZON_LINE      *hozLineLowP,*hozLineHighP,*tempHorLinesP=nullptr ;
 DTM_HORIZON_DISTANCE  *hDistP,*hozDistP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Invisible Horizon Lines") ;
/*
** Initialise
*/
 radius = tinRadius ;
/*
** Sort Horizon Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Horizon Table") ;
 qsortCPP(*horLinesPP,*numHorLinesP,sizeof(DTM_HORIZON_LINE),bcdtmLos_horizonPointsAngleCompareFunction) ;
/*
** Report Sorted Horizon Tables
*/
 if( dbg == 2 )
   {
    for( hLine1P = *horLinesPP ; hLine1P < *horLinesPP + *numHorLinesP ; ++hLine1P )
    bcdtmWrite_message(0,0,0,"HorizonLine[%8ld] ** Ang1 = %12.10lf Ang2 = %12.10lf ** D1 = %12.4lf D2 = %12.4lf",(long)(hLine1P-*horLinesPP),hLine1P->Ang1,hLine1P->Ang2,hLine1P->D1,hLine1P->D2) ;
   } 
/*
** Remove Totally Invisible Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Totally Invisible Horizon Lines") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Horizon Lines ** Before Removing Invisible Horiozon Lines = %8ld",*numHorLinesP) ;
 if( bcdtmVisibility_removeTotallyInvisibleHorizonLines(horLinesPP,numHorLinesP,Xe,Ye,Ze)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Horizon Lines ** After  Removing Invisible Horiozon Lines = %8ld",*numHorLinesP) ;
/*
** Build Horizon Line Indexes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Horizon Line Index Table") ;
 if( bcdtmVisibility_buildHorizonLineIndexFromHorizonLineTable(*horLinesPP,*numHorLinesP,horLinesIndexPP,numHorLinesIndexP,hozIndexListPP,numHorIndexListP,memHorIndexListP)) goto errexit ;
/*
** Build Distance Index
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Distance Index Table") ;
 if( bcdtmVisibility_buildDistanceIndexFromHorizonTable(*horLinesPP,*numHorLinesP,&hozDistP,&numHorDist)) goto errexit ;
 goto cleanup ;
/*
** Initilaise
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Invisible Horizon Line Sections") ;
 loop = 0 ;
 process = 1 ;
 while ( process )
   {
    if( dbg )
      {
       ++loop ; if( loop > 10 ) goto cleanup ;
       bcdtmWrite_message(0,0,0,"Loop = %6ld",loop) ;
      }
    process = 0 ;
    numSaveHorLines = *numHorLinesP ;
    saveHorLinesP   = *horLinesPP ;
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Number Of Horizon Lines Start = %6ld",*numHorLinesP) ;
/*
** Scan Horizon Points Until No More Invisible Lines
*/
    for( hDistP = hozDistP ; hDistP < hozDistP + numHorDist ; ++hDistP )
      {
       hLine1P = saveHorLinesP + hDistP->HlineOfs ;
       if( hLine1P->ActiveFlag == 1 )
         {
/*
**        Find Horizon Line Entry List Point For Angle1
*/
          if( bcdtmLos_findHorizonLineEntryListUsingHorizonLineIndex(*horLinesIndexPP,*numHorLinesIndexP,*hozIndexListPP,hLine1P->Ang1,&hLineOffset)) goto errexit ;
          if( hLineOffset != DTM_NULL_PNT ) hozLineLowP = saveHorLinesP + hLineOffset ;
          else                     hozLineLowP = saveHorLinesP ;
          hozLineLowP  = saveHorLinesP + hLineOffset ; 
          hozLineHighP = saveHorLinesP + numSaveHorLines - 1 ; 
/*
**        Process All Lines Whose End Points Are Within The Vision Angles
*/
          for( hLine2P = hozLineLowP ; hLine2P <= hozLineHighP && hLine2P->Ang1 < hLine1P->Ang2 ; ++hLine2P )
            { 
             if( hLine2P != hLine1P && hLine2P->ActiveFlag == 1 )
               {
/*
**              Edges Totally Within Current Edge Angles
*/
                if( hLine2P->Ang1 < hLine1P->Ang2 && hLine2P->Ang2 > hLine1P->Ang1 )              
                  {
                   if( bcdtmLos_determineIfHorizonLineIsCovered(Xe,Ye,hLine1P,hLine2P) )
                     {
                      if( hLine2P->Ang1 >= hLine1P->Ang1 ) { firstSection = 0 ; X1 = hLine2P->X1 ; Y1 = hLine2P->Y1 ; Z1 = hLine2P->Z1 ; }
                      else 
                        {
                         firstSection = 1 ;
                         X1 = Xe + radius * cos(hLine1P->Ang1) ;
                         Y1 = Ye + radius * sin(hLine1P->Ang1) ;
                         bcdtmMath_normalIntersectCordLines(Xe,Ye,X1,Y1,hLine2P->X1,hLine2P->Y1,hLine2P->X2,hLine2P->Y2,&X1,&Y1) ;
                         bcdtmMath_interpolatePointOnLine(hLine2P->X1,hLine2P->Y1,hLine2P->Z1,hLine2P->X2,hLine2P->Y2,hLine2P->Z2,X1,Y1,&Z1) ;
                        }  
                      if( hLine2P->Ang2 <= hLine1P->Ang2 ) { lastSection = 0 ; X2 = hLine2P->X2 ; Y2 = hLine2P->Y2 ; Z2 = hLine2P->Z2 ; }
                      else
                        {
                         lastSection = 1 ;
                         X2 = Xe + radius * cos(hLine1P->Ang2) ;
                         Y2 = Ye + radius * sin(hLine1P->Ang2) ;
                         bcdtmMath_normalIntersectCordLines(Xe,Ye,X2,Y2,hLine2P->X1,hLine2P->Y1,hLine2P->X2,hLine2P->Y2,&X2,&Y2) ;
                         bcdtmMath_interpolatePointOnLine(hLine2P->X1,hLine2P->Y1,hLine2P->Z1,hLine2P->X2,hLine2P->Y2,hLine2P->Z2,X2,Y2,&Z2) ;
                        }  
/*
**                    Determine Visibility
*/
                      if( bcdtmLos_determineVisibilityOfEdge(Xe,Ye,Ze,hLine1P->X2,hLine1P->Y2,hLine1P->Z2,hLine1P->X1,hLine1P->Y1,hLine1P->Z1,X1,Y1,Z1,X2,Y2,Z2,&visibility,visiblePts)) goto errexit ;
                      if( visibility != 1 ) 
                        {
                         if( visibility == -1 ) 
                           {
                            if( firstSection )
                              {
                               D2 = bcdtmMath_distance(Xe,Ye,X1,Y1) ;
                               if( bcdtmLos_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,hLine2P->Ang1,hLine1P->Ang1,hLine2P->D1,D2,hLine2P->X1,hLine2P->Y1,hLine2P->Z1,X1,Y1,Z1)) goto errexit ; 
                              }
                            if( lastSection )
                              {
                               D1 = bcdtmMath_distance(Xe,Ye,X2,Y2) ;
                               if( bcdtmLos_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,hLine1P->Ang2,hLine2P->Ang2,D1,hLine2P->D2,X2,Y2,Z2,hLine2P->X2,hLine2P->Y2,hLine2P->Z2)) goto errexit ; 
                              } 
                           } 
                         if( visibility ==  0 ) 
                           {
                            if( firstSection || lastSection )
                              {
                               if( firstSection )
                                 {
                                  if( visiblePts[0].x == X1 && visiblePts[0].y == Y1 )
                                    {
                                     D2   = bcdtmMath_distance(Xe,Ye,visiblePts[1].x,visiblePts[1].y) ;
                                     Ang2 = bcdtmMath_getAngle(Xe,Ye,visiblePts[1].x,visiblePts[1].y) ;
                                     if( Ang2 < hLine2P->Ang1 ) Ang2 = DTM_2PYE ;
                                     if( bcdtmLos_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,hLine2P->Ang1,Ang2,hLine2P->D1,D2,hLine2P->X1,hLine2P->Y1,hLine2P->Z1,visiblePts[1].x,visiblePts[1].y,visiblePts[1].z)) goto errexit ; 
                                    }
                                  else
                                    {
                                     D2   = bcdtmMath_distance(Xe,Ye,X1,Y1) ;
                                     if( bcdtmLos_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,hLine2P->Ang1,hLine1P->Ang1,hLine2P->D1,D2,hLine2P->X1,hLine2P->Y1,hLine2P->Z1,X1,Y1,Z1)) goto errexit ; 
                                    } 
                                 }
                               if( lastSection )
                                 {
                                  if( visiblePts[1].x == X2 && visiblePts[1].y == Y2 )
                                    {
                                     D1   = bcdtmMath_distance(Xe,Ye,visiblePts[0].x,visiblePts[0].y) ;
                                     Ang1 = bcdtmMath_getAngle(Xe,Ye,visiblePts[0].x,visiblePts[0].y) ;
                                     if( bcdtmLos_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,Ang1,hLine2P->Ang2,D1,hLine2P->D2,visiblePts[0].x,visiblePts[0].y,visiblePts[0].z,hLine2P->X2,hLine2P->Y2,hLine2P->Z2)) goto errexit ; 
                                    }
                                  else
                                    {
                                     D1   = bcdtmMath_distance(Xe,Ye,X2,Y2) ;
                                     if( bcdtmLos_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,hLine1P->Ang2,hLine2P->Ang2,D1,hLine2P->D2,X2,Y2,Z2,hLine2P->X2,hLine2P->Y2,hLine2P->Z2)) goto errexit ; 
                                     Ang1 = bcdtmMath_getAngle(Xe,Ye,visiblePts[0].x,visiblePts[0].y) ;
                                     Ang2 = bcdtmMath_getAngle(Xe,Ye,visiblePts[1].x,visiblePts[1].y) ;
                                     if( Ang2 < Ang1 ) Ang2 = DTM_2PYE ;
                                     D1   = bcdtmMath_distance(Xe,Ye,visiblePts[0].x,visiblePts[0].y) ;
                                     D2   = bcdtmMath_distance(Xe,Ye,visiblePts[1].x,visiblePts[1].y) ;
                                     if( bcdtmLos_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,Ang1,Ang2,D1,D2,visiblePts[0].x,visiblePts[0].y,visiblePts[0].z,visiblePts[1].x,visiblePts[1].y,visiblePts[1].z)) goto errexit ; 
                                    } 
                                 }
                              }
                            else
                              {
                               Ang1 = bcdtmMath_getAngle(Xe,Ye,visiblePts[0].x,visiblePts[0].y) ;
                               Ang2 = bcdtmMath_getAngle(Xe,Ye,visiblePts[1].x,visiblePts[1].y) ;
                               if( Ang2 < Ang1 ) Ang2 = DTM_2PYE ;
                               D1   = bcdtmMath_distance(Xe,Ye,visiblePts[0].x,visiblePts[0].y) ;
                               D2   = bcdtmMath_distance(Xe,Ye,visiblePts[1].x,visiblePts[1].y) ;
                               if( bcdtmLos_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,Ang1,Ang2,D1,D2,visiblePts[0].x,visiblePts[0].y,visiblePts[0].z,visiblePts[1].x,visiblePts[1].y,visiblePts[1].z)) goto errexit ; 
                              }  
                           }
                         process = 1 ;
                         hLine2P->ActiveFlag = 0 ; 
                        }
                     }
                  }
               }
            }
         }
      }
/*
**  Update Horizon Table
*/
    if( process )
      {
/*
**     Remove Invisible Lines From Horizon Lines Table
*/
       memLines = numSaveHorLines ;
       hozLineOfs1P = saveHorLinesP ;
       for( hozLineOfs2P = saveHorLinesP ; hozLineOfs2P < saveHorLinesP + numSaveHorLines ; ++hozLineOfs2P )
          {
           if( hozLineOfs2P->ActiveFlag )
             {
              if( hozLineOfs1P != hozLineOfs2P ) *hozLineOfs1P = *hozLineOfs2P ;
              ++hozLineOfs1P ;
             }
          }
        numLines = *numHorLinesP = (long)(hozLineOfs1P-saveHorLinesP) ;
/*
**      Copy Temporay To Horizon
*/
       for( hLine2P = tempHorLinesP ; hLine2P < tempHorLinesP + numTempHorLines ; ++hLine2P )
         {
          if( bcdtmLos_storePointInHorizonTable(&saveHorLinesP,&numLines,&memLines,hLine2P->Ang1,hLine2P->Ang2,hLine2P->D1,hLine2P->D2,hLine2P->X1,hLine2P->Y1,hLine2P->Z1,hLine2P->X2,hLine2P->Y2,hLine2P->Z2)) goto errexit ; 
         }
/*
**     Reallocate Horizon Lines
*/
       *numHorLinesP = numLines ;
       *horLinesPP = saveHorLinesP = ( DTM_HORIZON_LINE *) realloc(saveHorLinesP,*numHorLinesP*sizeof(DTM_HORIZON_LINE)) ;
/*
**     Free Temporary Table
*/
       if( tempHorLinesP != nullptr ) { free(tempHorLinesP) ; tempHorLinesP = nullptr ; }
       numTempHorLines = memTempHorLines = 0 ;
/*
**     Sort Horizon Table
*/
       qsortCPP(*horLinesPP,*numHorLinesP,sizeof(DTM_HORIZON_LINE),bcdtmLos_horizonPointsAngleCompareFunction) ;
/*
**     Build Horizon Line Indexes
*/
       if( bcdtmVisibility_buildHorizonLineIndexFromHorizonLineTable(*horLinesPP,*numHorLinesP,horLinesIndexPP,numHorLinesIndexP,hozIndexListPP,numHorIndexListP,memHorIndexListP)) goto errexit ;
/*
**     Build Distance Index
*/
       if( bcdtmVisibility_buildDistanceIndexFromHorizonTable(*horLinesPP,*numHorLinesP,&hozDistP,&numHorDist)) goto errexit ;
      }
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Number Of Horizon Lines End   = %6ld",*numHorLinesP) ;
   }
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Number Loops = %6ld",loop) ;
/*
** Clean Up
*/
 cleanup :
 if( hozDistP      != nullptr ) { free(hozDistP)      ; hozDistP = nullptr      ; }
 if( tempHorLinesP != nullptr ) { free(tempHorLinesP) ; tempHorLinesP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Invisible Horizon Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Invisible Horizon Lines Error") ;
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
BENTLEYDTM_Private int bcdtmVisibility_getLastVisibleSectionOfSurfaceLineBetweenEyeAndHorizonLineDtmObject
(
 BC_DTM_OBJ *dtmP,
 double Xe,
 double Ye,
 double Ze,
 double Xh,
 double Yh,
 double Zh,
 double *Xp,
 double *Yp,
 double *Zp
) 
/*
** This Function Gets The Visible Section Of A Line Draped On The Tin Surface
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   scan,process,numDrapePts=0 ;
 double dd,dx,dy,dz,x,y,z,maxAngle,eyeAngle=0.0,lastEyeAngle=0.0 ;
 DPoint3d    radial[2] ;
 DTM_DRAPE_POINT *drape1P,*drape2P,*drapePtsP=nullptr ;
/*
** Initialise
*/ 
 *Xp = Xh ; *Yp = Yh ; *Zp = Zh ;
/*
**  Drape Line On Surface
*/
 radial[0].x = Xe ; radial[0].y = Ye ; radial[0].z = Ze ;
 radial[1].x = Xh ; radial[1].y = Yh ; radial[1].z = Zh ;
/*
**  Drape Radial On Tin Surface
*/
 if( bcdtmDrape_stringDtmObject(dtmP,radial,2,FALSE,&drapePtsP,&numDrapePts)) goto errexit ;
/*
**  Remove Drape End Points Not On Tin
*/
 drape1P  = drapePtsP + numDrapePts - 1 ;
 while (drape1P->drapeType == DTMDrapedLineCode::External) { --drape1P; --numDrapePts; }
/*
**  Remove Duplicate Drape Points
*/
 for( drape1P = drape2P = drapePtsP ; drape2P < drapePtsP + numDrapePts ; ++drape2P )
   {
    if( drape2P->drapeX != drape1P->drapeX || drape2P->drapeY != drape1P->drapeY )
      {
       *drape1P = *drape2P ;
       ++drape1P ;
      }
   }
 numDrapePts = (long)(drape1P-drapePtsP)  ;
/*
**  Determine Visibility Of Draped Radial Points
*/
 if( bcdtmVisibility_storeVertice(100,0,0.0,0.0,0.0) ) goto errexit  ;
 if( bcdtmVisibility_storeVertice(1,1,drapePtsP->drapeX,drapePtsP->drapeY,drapePtsP->drapeZ) ) goto errexit  ;
 dx = (drapePtsP+1)->drapeX - Xe ;
 dy = (drapePtsP+1)->drapeY - Ye ;
 dz = (drapePtsP+1)->drapeZ - Ze ;
 dd = sqrt(dx*dx+dy*dy) ;
 maxAngle = atan2(dz,dd) ;
 drape1P = drapePtsP + 2 ;
 drape2P = drapePtsP + numDrapePts - 1 ;
 scan = 1 ;
 while ( scan )
   { 
/*
**  Scan Visible Drape Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Visible Draped Points") ; 
    if( drape1P <= drape2P ) 
      {
       process = 1 ; 
       while ( drape1P <= drape2P && process )
         {
          dx = drape1P->drapeX - Xe ;
          dy = drape1P->drapeY - Ye ;
          dz = drape1P->drapeZ - Ze ;
          dd = sqrt(dx*dx+dy*dy) ;
          eyeAngle = atan2(dz,dd) ;
          if( eyeAngle >= maxAngle ) { maxAngle = eyeAngle ; ++drape1P ; }
          else                        process = 0 ; 
         }  
       --drape1P ;
       if( bcdtmVisibility_storeVertice(1,1,drape1P->drapeX,drape1P->drapeY,drape1P->drapeZ) ) goto errexit ;
       if( drape1P < drape2P ) if( bcdtmVisibility_storeVertice(1,0,drape1P->drapeX,drape1P->drapeY,drape1P->drapeZ) ) goto errexit ;
       ++drape1P ; 
      } 
/*
**  Scan Invisible Drape Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Invisible Draped Points") ; 
    if( drape1P <= drape2P )  
      {
       process = 1 ; 
       while ( drape1P <= drape2P && process )
         {
          dx = drape1P->drapeX - Xe ;
          dy = drape1P->drapeY - Ye ;
          dz = drape1P->drapeZ - Ze ;
          dd = sqrt(dx*dx+dy*dy) ;
          eyeAngle = atan2(dz,dd) ;
          if( eyeAngle < maxAngle ) { lastEyeAngle = eyeAngle ; ++drape1P ; }
          else                        process = 0 ; 
         }
/*
**   Calculate Max Angle Intercept On Radial
*/
       if( process )
         {
          --drape1P ;
          if( bcdtmVisibility_storeVertice(1,0,drape1P->drapeX,drape1P->drapeY,drape1P->drapeZ) ) goto errexit ;
          ++drape1P ;
         }
       else
         { 
          dx = maxAngle - lastEyeAngle ;
          dd = eyeAngle - lastEyeAngle ;
          x =  (drape1P-1)->drapeX + (drape1P->drapeX - (drape1P-1)->drapeX) * dx / dd ;
          y =  (drape1P-1)->drapeY + (drape1P->drapeY - (drape1P-1)->drapeY) * dx / dd ;
          z =  (drape1P-1)->drapeZ + (drape1P->drapeZ - (drape1P-1)->drapeZ) * dx / dd ;
          if( bcdtmVisibility_storeVertice(1,0,x,y,z) ) goto errexit ;
          if( bcdtmVisibility_storeVertice(1,1,x,y,z) ) goto errexit ;
         } 
      }
/*
**  Test For End Of Scan
*/
    if( drape1P > drape2P ) scan = 0 ;
   } 
/*
** Set Coordinates Back Into Radial
*/
 if( numLosPts < 2 ) return(0) ;
 *Xp = (losPtsP + numLosPts - 2)->x ; 
 *Yp = (losPtsP + numLosPts - 2)->y ; 
 *Zp = (losPtsP + numLosPts - 2)->z ; 
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
BENTLEYDTM_Private int bcdtmVisibility_determineIfHorizonLineIsCovered
(
 double Xe,
 double Ye,
 DTM_HORIZON_LINE *hLine1P,
 DTM_HORIZON_LINE *hLine2P
) 
/*
** This Function Determines If The hLine1P is between the Eye Position And hLine2P
*/
{
 int    ret=DTM_SUCCESS ;
 double minAngle,maxAngle,angle,x,y,X1,Y1,X2,Y2,radius=1000000.0 ;
/*
** Get Angle Coverage
*/
 if( hLine1P->Ang1 >= hLine2P->Ang1 ) minAngle = hLine1P->Ang1 ;
 else                                 minAngle = hLine2P->Ang1 ;
 if( hLine1P->Ang2 <= hLine2P->Ang2 ) maxAngle = hLine1P->Ang2 ;
 else                                 maxAngle = hLine2P->Ang2 ;
 angle = ( minAngle + maxAngle ) / 2.0 ;
/*
** Calculate Radial From Eye
*/
 x = Xe + radius * cos(angle) ;
 y = Ye + radius * sin(angle) ;
/*
** Intersect Horizon Lines
*/
 bcdtmMath_normalIntersectCordLines(Xe,Ye,x,y,hLine1P->X1,hLine1P->Y1,hLine1P->X2,hLine1P->Y2,&X1,&Y1) ;
 bcdtmMath_normalIntersectCordLines(Xe,Ye,x,y,hLine2P->X1,hLine2P->Y1,hLine2P->X2,hLine2P->Y2,&X2,&Y2) ;
 if( bcdtmMath_distance(Xe,Ye,X1,Y1) < bcdtmMath_distance(Xe,Ye,X2,Y2) ) goto errexit  ;
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
BENTLEYDTM_Private int bcdtmVisibility_determineIfHorizonLineIsTotallyCovered
(
 double Xe,
 double Ye,
 DTM_HORIZON_LINE *hLine1P,
 DTM_HORIZON_LINE *hLine2P
) 
/*
** This Function Determines If The hLine1P is between the 
** Eye Position And hline2P and Totally Covers hLine2P
*/
{
 int    ret=DTM_SUCCESS ;
 double angle,x,y,X1,Y1,X2,Y2,radius=10000000.0 ;
/*
** Get Angle Coverage
*/
 if( hLine2P->Ang1 >= hLine1P->Ang1 && hLine2P->Ang2 <= hLine1P->Ang2 )
   {
    angle = ( hLine2P->Ang1 + hLine2P->Ang2 ) / 2.0 ;
/*
**  Calculate Radial From Eye
*/
    x = Xe + radius * cos(angle) ;
    y = Ye + radius * sin(angle) ;
/*
** Intersect Horizon Lines
*/
   bcdtmMath_normalIntersectCordLines(Xe,Ye,x,y,hLine1P->X1,hLine1P->Y1,hLine1P->X2,hLine1P->Y2,&X1,&Y1) ;
   bcdtmMath_normalIntersectCordLines(Xe,Ye,x,y,hLine2P->X1,hLine2P->Y1,hLine2P->X2,hLine2P->Y2,&X2,&Y2) ;
   if( bcdtmMath_distance(Xe,Ye,X1,Y1) < bcdtmMath_distance(Xe,Ye,X2,Y2) ) goto errexit  ;
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
BENTLEYDTM_Private int bcdtmVisibility_findHorizonLine(DTM_HORIZON_LINE *horLinesP,long numHorLines,double x,double y,long *hozLineOffsetP) 
{
 DTM_HORIZON_LINE *hLineP ;
/*
** Initialise
*/
 *hozLineOffsetP = DTM_NULL_PNT ;
/*
** Scan Horizon Line Structure
*/
 for( hLineP = horLinesP ; hLineP < horLinesP + numHorLines ; ++hLineP )
   {
    if( bcdtmMath_distance(x,y,hLineP->X1,hLineP->Y1) < 0.01 ) 
      {
       *hozLineOffsetP = (long)(hLineP - horLinesP) ;
       return(DTM_SUCCESS) ; 
      } 
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
BENTLEYDTM_Private int bcdtmVisibility_horizonPointsAngleCompareFunction(const void *Cp1,const void *Cp2)
/*
** Compare Function For Qsort Of Horizon Lines
*/
{
 DTM_HORIZON_LINE *hLine1P,*hLine2P ;
 hLine1P = ( DTM_HORIZON_LINE * ) Cp1 ;
 hLine2P = ( DTM_HORIZON_LINE * ) Cp2 ;
 if     ( hLine1P->Ang1 < hLine2P->Ang1 ) return(-1) ;
 else if( hLine1P->Ang1 > hLine2P->Ang1 ) return( 1) ;
 else if( hLine1P->Ang2 > hLine2P->Ang2 ) return(-1) ;
 else if( hLine1P->Ang2 < hLine2P->Ang2 ) return( 1) ;
 else if( hLine1P->D1   < hLine2P->D1   ) return(-1) ;
 else if( hLine1P->D1   > hLine2P->D1   ) return( 1) ;
 else if( hLine1P->D2   < hLine2P->D2   ) return(-1) ;
 else if( hLine1P->D2   > hLine2P->D2   ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmVisibility_checkAngleSequenceVisibilityLine(DTM_HORIZON_LINE *horLinesP,long numHorLines,long normalFlag, double angle )
{
 int ret=DTM_SUCCESS ;
 DTM_HORIZON_LINE *hLineP ;
 for( hLineP = horLinesP ; hLineP < horLinesP + numHorLines - 1 ; ++hLineP ) 
   {
    if( hLineP->Ang2 != (hLineP+1)->Ang1 )
      { 
       if( normalFlag &&  hLineP->Ang2 != angle )
         { 
          bcdtmWrite_message(0,0,0,"Angle Sequence Error ** horLinesP = %6ld Ang2 = %12.10lf ** horLinesP = %6ld Ang1 = %12.10lf",(long)(hLineP-horLinesP),hLineP->Ang2,(long)(hLineP-horLinesP+1),(hLineP+1)->Ang1) ;
          ret = DTM_ERROR ;
         }
      }  
   }
/*
** Job Completed
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmVisibility_calculateHorizonLineIntercept(DTM_HORIZON_LINE *hLineP,double Xe,double Ye,double Angle,double Radius,double *x,double *y,double *z) 
{
 double  Xr,Yr ;
/*
** Initialise
*/
 *x = *y = *z = 0.0 ;
/*
** Calculate Radial Out From Eye
*/
 Xr = Xe + Radius * cos(Angle) ;
 Yr = Ye + Radius * sin(Angle) ;
/*
** Intersect Horizon Line With Visibility Line
*/
 bcdtmMath_normalIntersectCordLines(hLineP->X1,hLineP->Y1,hLineP->X2,hLineP->Y2,Xe,Ye,Xr,Yr,x,y) ;
/*
** Interpolate z On Line
*/
 bcdtmMath_interpolatePointOnLine(hLineP->X1,hLineP->Y1,hLineP->Z1,hLineP->X2,hLineP->Y2,hLineP->Z2,*x,*y,z) ;
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
BENTLEYDTM_Private int bcdtmVisibility_checkLengthVisibilityLine(DTM_HORIZON_LINE *horLinesP,long numHorLines,double length )
{
 int    ret=DTM_SUCCESS   ;
 double len=0.0 ;
 DTM_HORIZON_LINE  *hLineP  ;
 for( hLineP = horLinesP ; hLineP < horLinesP + numHorLines ; ++hLineP ) len +=  bcdtmMath_distance(hLineP->X1,hLineP->Y1,hLineP->X2,hLineP->Y2) ;
 if( fabs(len-length) > 0.0000001 )
   {
    bcdtmWrite_message(0,0,0,"Length Error ** Line Length = %15.8lf Summed Length = %15.8lf ** Diff = %15.8lf",length,len,length-len) ;
    ret = DTM_ERROR ;
   }
/*
** Job Completed
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmVisibility_refineTinForRegionVisibilityDtmObject
(
 BC_DTM_OBJ *dtmP,
 double Xe,
 double Ye,
 double Ze,
 DTM_HORIZON_LINE_INDEX *hozLineIndexP,
 long numHorLineIndex,
 long *hozIndexListP
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long  p1,p2,p3,clPtr,voidFlag,startPnt,refineFlag,numStringPts,numPoints  ;
 long  trgNumber,dtmFeature,numFeatures,numBefore,numEnd,numJoinUserTags ;
 DPoint3d   *p3dP,*stringPtsP=nullptr,insertLine[2] ;
 DTM_TIN_NODE   *nodeP ;
 BC_DTM_OBJ     *dtmDataP=nullptr ;
 BC_DTM_FEATURE *dtmFeatureP  ;
 DTM_JOIN_USER_TAGS *joinUserTagsP=nullptr ;
/*
** Log Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Refining Tin For Region Visibility") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Xe              = %12.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Ye              = %12.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Ze              = %12.4lf",Ze) ;
    bcdtmWrite_message(0,0,0,"hozLineIndexP   = %p",hozLineIndexP) ;
    bcdtmWrite_message(0,0,0,"numHorLineIndex = %8ld",numHorLineIndex) ;
    bcdtmWrite_message(0,0,0,"hozIndexListP   = %p",hozIndexListP) ;
   } 
/*
** Set Global Eye Values
*/
 eyeX = Xe ; 
 eyeY = Ye ; 
 eyeZ = Ze ;
/*
** Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(&dtmDataP)) goto errexit  ;
/*
** Scan All Triangle And Set Visibility Attributes 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Region Visibility Break Lines") ;
 refineFlag = 0 ;
 trgNumber  = 0 ;
 for( p1 = 0 ; p1 < dtmP->numPoints  ; ++p1 )
   {
    nodeP = nodeAddrP(dtmP,p1) ;
    clPtr = nodeP->cPtr ;
    if( clPtr != dtmP->nullPtr )
      {
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit  ;
       while( clPtr != dtmP->nullPtr )
         {
          p3    = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          if( p2 > p1 && p3 > p1  )
            {
             if( nodeP->hPtr != p2 ) 
               {
                ++trgNumber ;
                if( trgNumber == 1 || trgNumber % 100000 == 0 || trgNumber == dtmP->numTriangles ) bcdtmWrite_message(1,0,0,"Processing Triangle %6ld of %6ld",trgNumber,dtmP->numTriangles) ;
                if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidFlag)) goto errexit  ;
                if( ! voidFlag ) 
                  {
                   if( bcdtmVisibility_calculateRegionVisibilityBreakLinesDtmObject(dtmP,dtmDataP,p1,p2,p3,numHorLines,hozLineIndexP,numHorLineIndex,hozIndexListP)) goto errexit  ;
                  }
               }
            } 
          p2 = p3 ;
         }
      }
   }
/*
**  Write Number Of Visibility Break Lines
*/
 if( dbg == 1 ) 
   {
//    bcdtmObject_reportStatisticsDtmObject(dtmP) ;
    bcdtmWrite_message(0,0,0,"Number Of Visibility Break Lines  = %8ld",dtmDataP->numFeatures) ;   
    bcdtmWrite_message(0,0,0,"Number Of Visibility Break Points = %8ld",dtmDataP->numPoints) ;   
    if( bcdtmWrite_geopakDatFileFromDtmObject(dtmDataP,L"visibilityBreakLines.dat")) goto errexit ;
    if( bcdtmObject_appendDtmObject(dtmP,dtmDataP)) goto errexit ;
    if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"After Retriangulation Number Of Points = %8ld",dtmP->numPoints) ;   
//    bcdtmObject_reportStatisticsDtmObject(dtmP) ;
    goto cleanup ;
   } 
/*
**  Join And Filter Horizon Lines
*/
 if( dtmDataP->numFeatures > 0  )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Joining Horizon Lines") ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Horizon Lines Coordinates Start = %6ld",dtmDataP->numPoints) ;
    if( bcdtmJoin_dtmFeatureTypeDtmObject(dtmDataP,dtmP->ppTol,DTMFeatureType::Breakline,DTMFeatureType::Breakline,&numBefore,&numEnd,&joinUserTagsP,&numJoinUserTags)) goto errexit ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Horizon Lines Start = %6ld",numBefore) ;
       bcdtmWrite_message(0,0,0,"Number Horizon Lines End   = %6ld",numEnd) ;
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Horizon Lines") ;
    if( bcdtmFilter_dtmFeatureTypeDtmObject(dtmDataP,DTMFeatureType::Breakline,dtmP->ppTol*100.0,dtmP->ppTol*100.0,&numFeatures,&numBefore,&numEnd)) goto errexit ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Horizon Lines Filtered          = %6ld",numFeatures) ;
       bcdtmWrite_message(0,0,0,"Number Horizon Lines Coordinates Start = %6ld",numBefore) ;
       bcdtmWrite_message(0,0,0,"Number Horizon Lines Coordinates End   = %6ld",numEnd) ;
      } 
   } 
/*
** Refine Tin Object By Inserting Visibility Break Lines
*/ 
 if( dtmDataP->numFeatures > 0  )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Before Inserting Visibility Break Lines Into Tin ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
    numPoints = dtmP->numPoints ;
    for( dtmFeature = 0 ; dtmFeature < dtmDataP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmDataP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline )
         {
          if( bcdtmObject_getPointsForDtmFeatureDtmObject(dtmDataP,dtmFeature,(DTM_TIN_POINT **) &stringPtsP ,&numStringPts)) goto errexit ;
          if( numStringPts > 1 ) 
            {         
             for( p3dP = stringPtsP ; p3dP < stringPtsP + numStringPts - 1 ; ++p3dP)
               {
                insertLine[0].x = p3dP->x     ; insertLine[0].y = p3dP->y     ; insertLine[0].z = p3dP->z ;
                insertLine[1].x = (p3dP+1)->x ; insertLine[1].y = (p3dP+1)->y ; insertLine[1].z = (p3dP+1)->z ;
                if( bcdtmInsert_internalStringIntoDtmObject(dtmP,1,2,insertLine,2,&startPnt)) goto errexit ;
                if( startPnt != dtmP->nullPnt ) bcdtmList_nullTptrListDtmObject(dtmP,startPnt) ;
               }
            }    
          if( stringPtsP != nullptr ) { free(stringPtsP) ; stringPtsP = nullptr ; }
         }
/*
**     Clean DTM Object If More Than 2500 Tin Points Have Been Inserted
*/
       if( dtmP->numPoints - numPoints > 2500 ) 
         {
          if( bcdtmList_cleanDtmObject(dtmP) ) goto errexit ;
          numPoints = dtmP->numPoints ;
         } 
      }
    if( bcdtmList_cleanDtmObject(dtmP) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"After  Inserting Visibility Break Lines Into Tin ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
   }   
/*
** Clean Up
*/
 cleanup :
 if( dtmDataP      != nullptr ) bcdtmObject_destroyDtmObject(&dtmDataP) ;
 if( stringPtsP    != nullptr ) free(stringPtsP) ;
 if( joinUserTagsP != nullptr ) free(joinUserTagsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Refining Tin For Region Visibility Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Refining Tin For Region Visibility Error") ;
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
BENTLEYDTM_Private int bcdtmVisibility_polygoniseAndLoadRegionVisibilityFromDtmObject
(
 BC_DTM_OBJ *dtmP,
 double Xe,
 double Ye,
 double Ze,
 DTM_HORIZON_LINE_INDEX *horLinesIndexP,
 long numHorLinesIndex,
 long *hozIndexListP,
 DTMFeatureCallback loadFunctionP,
 void   *userP
)
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long      p1,p2,p3,lp,clc,offset,ofs1,ofs2,voidFlag,visValue,visibility ;
 long      numLoadPts=0,memLoadPts=0,memLoadPtsInc=1000 ;
 double    x,y,sx,sy,area,X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3  ;
 char      cv,nv=(char)-1/*255*/,*cP,*linesP=nullptr ;
 DPoint3d       *loadPtsP=nullptr ;
 DTM_TIN_NODE *nodeP ;
/*
** Allocate Memory
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory") ;
 linesP = ( char * ) malloc ( dtmP->cListPtr * sizeof(char)) ;
 if( linesP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
 for( cP = linesP ; cP < linesP + dtmP->cListPtr ; ++cP ) *cP = nv ;
/*
** Scan All Triangle And Set Visibility Attributes 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonizing Tin Region Visibility") ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    nodeP = nodeAddrP(dtmP,p1) ;
    clc = nodeP->cPtr ;
    if( clc != dtmP->nullPtr )
      {
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit  ;
       while( clc != dtmP->nullPtr )
         {
          p3  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( p2 > p1 && p3 > p1 ) 
            {
             if( nodeP->hPtr != p2 ) 
               {
                if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidFlag)) goto errexit  ;
                if( ! voidFlag ) 
                  {
                   X1 = pointAddrP(dtmP,p1)->x ; Y1 = pointAddrP(dtmP,p1)->y ; Z1 = pointAddrP(dtmP,p1)->z + 0.00001 ;
                   X2 = pointAddrP(dtmP,p2)->x ; Y2 = pointAddrP(dtmP,p2)->y ; Z2 = pointAddrP(dtmP,p2)->z + 0.00001 ;
                   X3 = pointAddrP(dtmP,p3)->x ; Y3 = pointAddrP(dtmP,p3)->y ; Z3 = pointAddrP(dtmP,p3)->z + 0.00001 ;
                   if( bcdtmVisibility_determinePointVisibilityUsingVisibilityTables(horLinesP,numHorLines,horLinesIndexP,numHorLinesIndex,hozIndexListP,Xe,Ye,Ze,(X1+X2+X3)/3.0,(Y1+Y2+Y3)/3.0,(Z1+Z2+Z3)/3.0,&visibility)) goto errexit ;
                   if( visibility ) visValue = 1 ;
                   else             visValue = 2 ; 
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p1,p2) ) goto errexit ;
                   *(linesP+offset) = ( char ) visValue ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p2,p3) ) goto errexit ;
                   *(linesP+offset) = ( char ) visValue  ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p3,p1) ) goto errexit ;
                   *(linesP+offset) = ( char ) visValue  ;
                  }
               }
            } 
          p2 = p3 ;
         }
      }
   }
/*
** Extract And Load Visibility Polygons
*/             
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Visibility Polygons") ;	  
/*
** Extract Polygons On the Tin Hull
*/
 p1 = dtmP->hullPoint ;
 do 
   {
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
    bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p1) ;
    if( ( cv = *(linesP+ofs1)) != nv )
      {
       p3 = p2 ; p2 = p1 ;
       if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
       if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
/*
**     Scan Until Back To First Point
*/
       do
         {
          if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0  )       goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) )       goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) )       goto errexit ;
          while( *(linesP+ofs1) == cv && *(linesP+ofs2) == cv )
            {
             if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0  )    goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) )    goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) )    goto errexit ;
            }
          *(linesP+ofs1) = nv ;
          if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
          lp = p2 ; p2 = p3 ; p3 = lp ;
         } while ( p2 != p1 ) ;
       if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
       if( cv == 1 ) { if( loadFunctionP(DTMFeatureType::VisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,loadPtsP,numLoadPts,userP)) goto errexit  ; }
       else          { if( loadFunctionP(DTMFeatureType::InvisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,loadPtsP,numLoadPts,userP)) goto errexit  ; }
       numLoadPts = 0 ;
      }
    p1 = nodeAddrP(dtmP,p1)->hPtr  ;
   } while( p1 != dtmP->hullPoint ) ;
/*
** Extract Internal Polygons
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Internal Polygons") ;
 for( p1=0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    clc = nodeAddrP(dtmP,p1)->cPtr ;
    while ( clc != dtmP->nullPtr )
      {
       p2 = clistAddrP(dtmP,clc)->pntNum ;
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p1,p2) ) goto errexit ;
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p2,p1) ) goto errexit ;
       if( *(linesP+ofs1) != *(linesP+ofs2)  && *(linesP+ofs1) != nv  && bcdtmList_testLineDtmObject(dtmP,p1,clistAddrP(dtmP,clc)->pntNum) && ( nodeAddrP(dtmP,p1)->hPtr != p2 || nodeAddrP(dtmP,p2)->hPtr != p1 ) )
         {         
          cv = *(linesP+ofs1) ;
          p3 = p1 ;
/*
**        Get Polygon Direction
*/
          area = 0.0 ; sx = pointAddrP(dtmP,p1)->x ; sy = pointAddrP(dtmP,p1)->y ;
          x = pointAddrP(dtmP,p2)->x - sx ; y = pointAddrP(dtmP,p2)->y - sy  ;
          area = area + ( x * y ) / 2.0 + x * sy ;
          sx = pointAddrP(dtmP,p2)->x ; sy = pointAddrP(dtmP,p2)->y ;
          do
            {
             if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0 )  goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) )       goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) )       goto errexit ;
             while( *(linesP+ofs1) == cv && *(linesP+ofs2) == cv )
               {
                if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0 ) goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) )      goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) )      goto errexit ;
               }
             x = pointAddrP(dtmP,p3)->x - sx ; y = pointAddrP(dtmP,p3)->y - sy  ;
             area = area + ( x * y ) / 2.0 + x * sy ;
             sx = pointAddrP(dtmP,p3)->x ; sy = pointAddrP(dtmP,p3)->y ;
             lp = p2 ; p2 = p3 ; p3 = lp ;
            } while ( p2 != p1 ) ;
/*
**        If Polygon Is Clockwise Write Polygon
*/
          if( area > 0.0 ) 
            {
             p3 = p1 ;
             p2 = clistAddrP(dtmP,clc)->pntNum ;
             if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
             if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
/*
**           Scan Until Back To First Point
*/
             do
               {
                if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0 )  goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) )     goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) )     goto errexit ;
                while( *(linesP+ofs1) == cv && *(linesP+ofs2) == cv )
                  {
                   if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0 ) goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) )    goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) )    goto errexit ;
                  }
                *(linesP+ofs1)  = nv ;
                if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
                lp = p2 ; p2 = p3 ; p3 = lp ;
               } while ( p2 != p1 ) ;
             if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
             if( cv == 1 ) { if( loadFunctionP(DTMFeatureType::VisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,loadPtsP,numLoadPts,userP)) goto errexit  ; }
             else          { if( loadFunctionP(DTMFeatureType::InvisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,loadPtsP,numLoadPts,userP)) goto errexit  ; }
             numLoadPts = 0 ;
            }
         }
       clc = clistAddrP(dtmP,clc)->nextPtr ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( linesP   != nullptr ) free(linesP) ;
 if( loadPtsP != nullptr ) free(loadPtsP) ;
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
BENTLEYDTM_Private int bcdtmVisibility_calculateRegionVisibilityBreakLinesDtmObject
(
 BC_DTM_OBJ *dtmP,
 BC_DTM_OBJ *dtmDataP,
 long P1,
 long P2,
 long P3,
 long numHorLines,
 DTM_HORIZON_LINE_INDEX *hozLineIndexP,
 long numHorLineIndex,
 long *hozLineIndexListP
 )
/*
** This Function Calculates The Break Lines To Refine The Tin For Region Visibility
*/ 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p1,p2,p3,fndType,h1Inside,h2Inside,hozLineIntersect,piVisibility,p2Visibility,p3Visibility,coverFlag,hLineOffset ;  
 double angle1,angle2,angle3,maxAngle,minAngle ;
 double X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,radius,zMin,zMax,xMid,hzMin,hzMax ;
 double IX1,IY1,IZ1,IX2,IY2,IZ2 ;
 DPoint3d    breakPts[2] ;
 DTM_HORIZON_LINE  *hLineP,*lhLineP,*hhLineP ;
 double A0,B0,C0,D0,A1,B1,C1,D1,HX1,HY1,HZ1,HX2,HY2,HZ2,ZP1,ZP2 ;
 long   fndType1,fndType2 ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;

/*
** Initialise Coordinates
*/
 X1 = pointAddrP(dtmP,P1)->x ; Y1 = pointAddrP(dtmP,P1)->y ; Z1 = pointAddrP(dtmP,P1)->z  ;
 X2 = pointAddrP(dtmP,P2)->x ; Y2 = pointAddrP(dtmP,P2)->y ; Z2 = pointAddrP(dtmP,P2)->z  ;
 X3 = pointAddrP(dtmP,P3)->x ; Y3 = pointAddrP(dtmP,P3)->y ; Z3 = pointAddrP(dtmP,P3)->z  ;
 radius = tinRadius ;
 zMin = zMax = Z1 ;
 if( Z2 < zMin ) zMin = Z2 ;
 if( Z3 < zMin ) zMin = Z3 ;
 if( Z2 > zMax ) zMax = Z2 ;
 if( Z3 > zMax ) zMax = Z3 ;
 xMid = (dtmP->xMin + dtmP->xMax) / 2.0 ;
/*
** Calulate Angles To Eye And Max Min Angles For Triangle
*/
 angle1 = bcdtmMath_getAngle(eyeX,eyeY,X1,Y1) ;
 angle2 = bcdtmMath_getAngle(eyeX,eyeY,X2,Y2) ;
 angle3 = bcdtmMath_getAngle(eyeX,eyeY,X3,Y3) ;
 minAngle = maxAngle = angle1 ;
 if( angle2 < minAngle ) minAngle = angle2 ;
 if( angle3 < minAngle ) minAngle = angle3 ;
 if( angle2 > maxAngle ) maxAngle = angle2 ;
 if( angle3 > maxAngle ) maxAngle = angle3 ;
/*
** Write Statistics For Development Purposes
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"trgNumber = %6ld",trgNumber) ;
    bcdtmWrite_message(0,0,0,"Triangle Coordinates") ;
    bcdtmWrite_message(0,0,0,"%6ld ** %10.4lf %10.4lf %10.4lf",P1,X1,Y1,Z1) ;
    bcdtmWrite_message(0,0,0,"%6ld ** %10.4lf %10.4lf %10.4lf",P2,X2,Y2,Z2) ;
    bcdtmWrite_message(0,0,0,"%6ld ** %10.4lf %10.4lf %10.4lf",P3,X3,Y3,Z3) ;
    if( minAngle <= maxAngle ) bcdtmWrite_message(0,0,0,"Angles = %12.8lf %12.8lf %12.8lf ** %12.8lf %12.8lf",angle1,angle2,angle3,minAngle,maxAngle) ;
    else                       bcdtmWrite_message(0,0,0,"Bngles = %12.8lf %12.8lf %12.8lf ** %12.8lf %12.8lf",angle1,angle2,angle3,minAngle,maxAngle) ;      
    bcdtmWrite_message(0,0,0,"Eye-P1 = %10.4lf",bcdtmMath_distance(eyeX,eyeY,X1,Y1)) ;
    bcdtmWrite_message(0,0,0,"Eye-P2 = %10.4lf",bcdtmMath_distance(eyeX,eyeY,X2,Y2)) ;
    bcdtmWrite_message(0,0,0,"Eye-P3 = %10.4lf",bcdtmMath_distance(eyeX,eyeY,X3,Y3)) ;
   } 
/*
**  Calculate Plane Coefficients For Triangle
*/
 bcdtmMath_calculatePlaneCoefficients(X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,&A0,&B0,&C0,&D0) ;
/*
** Scan Horizon Lines And Determine Visibility BreakLines 
*/
    bcdtmVisibility_findHorizonLineEntryListUsingHorizonLineIndex(hozLineIndexP,numHorLineIndex,hozLineIndexListP,minAngle,&hLineOffset) ;
    if( hLineOffset != DTM_NULL_PNT ) lhLineP = horLinesP + hLineOffset ;
    else                         lhLineP = horLinesP ;
    hhLineP = horLinesP + numHorLines - 1 ;
    for( hLineP = lhLineP ; hLineP <= hhLineP && hLineP->Ang1 < maxAngle  ; ++hLineP )
      {
/*
**     Test For Covering Horizon Line
*/
       if( hLineP->Ang1 < maxAngle && hLineP->Ang2 > minAngle )
         { 
/*
**        Determine If Horizon Line Is Between Eye And Triangle
*/
          bcdtmVisibility_testForCoveringHorizonLineOfTriangle(hLineP,eyeX,eyeY,minAngle,maxAngle,X1,Y1,X2,Y2,X3,Y3,&coverFlag) ;
/*
**        Calculate Visibility Of Points
*/
          if( coverFlag )
            { 
             bcdtmVisibility_determineVisibilityOfPoint(eyeX,eyeY,eyeZ,hLineP->X2,hLineP->Y2,hLineP->Z2,hLineP->X1,hLineP->Y1,hLineP->Z1,X1,Y1,Z1,&piVisibility) ;
             bcdtmVisibility_determineVisibilityOfPoint(eyeX,eyeY,eyeZ,hLineP->X2,hLineP->Y2,hLineP->Z2,hLineP->X1,hLineP->Y1,hLineP->Z1,X2,Y2,Z2,&p2Visibility) ;
             bcdtmVisibility_determineVisibilityOfPoint(eyeX,eyeY,eyeZ,hLineP->X2,hLineP->Y2,hLineP->Z2,hLineP->X1,hLineP->Y1,hLineP->Z1,X3,Y3,Z3,&p3Visibility) ;
/*
**           If Triangle Is Partially Visible From Horizon Line Determine Break Line
*/
             if( ( ! piVisibility || !p2Visibility || !p3Visibility ) && (  piVisibility || p2Visibility || p3Visibility ) )
               {

/*
**              Calculate Plane Coefficients For Horizon Line
*/
                bcdtmMath_calculatePlaneCoefficients(eyeX,eyeY,eyeZ,hLineP->X1,hLineP->Y1,hLineP->Z1,hLineP->X2,hLineP->Y2,hLineP->Z2,&A1,&B1,&C1,&D1) ;
/*
**              Get Line Of Intersection Between The Triangle Plane And Horizon Plane
*/
                hzMin = zMin ; hzMax = zMax ;
                if( eyeZ == hLineP->Z1 && eyeZ == hLineP->Z2 ) { hzMin = hzMax = eyeZ ; }
                bcdtmVisibility_calculateIntersectionLineBetweenPlanes(xMid,hzMin,hzMax,A0,B0,C0,D0,A1,B1,C1,D1,&IX1,&IY1,&IZ1,&IX2,&IY2,&IZ2) ;
/*
**              Truncate Intersection Line To Horizon Line Edges
*/
                bcdtmMath_normalIntersectCordLines(eyeX,eyeY,eyeX+radius*cos(hLineP->Ang1),eyeY+radius*sin(hLineP->Ang1),IX1,IY1,IX2,IY2,&HX1,&HY1) ;
                bcdtmMath_interpolatePointOnLine(IX1,IY1,IZ1,IX2,IY2,IZ2,HX1,HY1,&HZ1) ;
                bcdtmMath_normalIntersectCordLines(eyeX,eyeY,eyeX+radius*cos(hLineP->Ang2),eyeY+radius*sin(hLineP->Ang2),IX1,IY1,IX2,IY2,&HX2,&HY2) ;
                bcdtmMath_interpolatePointOnLine(IX1,IY1,IZ1,IX2,IY2,IZ2,HX2,HY2,&HZ2) ;
/*
**              Check End Points Have Same z Values On Both Planes - Development Only
*/
                if( cdbg )
                  { 
                   if( dbg ) bcdtmWrite_message(0,0,0,"Checking End Points") ;
                   bcdtmMath_interpolatePointOnPlane(HX1,HY1,&ZP1,A0,B0,C0,D0) ;
                   bcdtmMath_interpolatePointOnPlane(HX1,HY1,&ZP2,A1,B1,C1,D1) ;
                   if( fabs(ZP1-ZP2) > 0.00001 ) bcdtmWrite_message(0,0,0,"DTM_ERROR ** Z1 = %10.4lf Z2 = %10.4lf Diff = %12.10lf",ZP1,ZP2,fabs(ZP1-ZP2)) ;
                   bcdtmMath_interpolatePointOnPlane(HX2,HY2,&ZP1,A0,B0,C0,D0) ;
                   bcdtmMath_interpolatePointOnPlane(HX2,HY2,&ZP2,A1,B1,C1,D1) ;
                   if( fabs(ZP1-ZP2) > 0.00001 ) bcdtmWrite_message(0,0,0,"DTM_ERROR ## Z1 = %10.4lf Z2 = %10.4lf Diff = %12.10lf",ZP1,ZP2,fabs(ZP1-ZP2)) ;
                  }
/*
**               Check Points Are On Tin - Development Only
*/
                 if( cdbg )
                   {
                    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Points Are On Tin ** HX1 = %12.5lf HY1 = %12.5lf ** HX2 = %12.5lf HY2 = %12.5lf",HX1,HY1,HX2,HY2) ;
                    if( bcdtmFind_triangleDtmObject(dtmP,HX1,HY1,&fndType1,&p1,&p2,&p3) ) goto errexit ;
                    if( ! fndType1 )bcdtmWrite_message(0,0,0,"** Point Hx1 = %10.4lf Hy1 = %10.4lf Not On Tin",HX1,HY1) ;
                    if( bcdtmFind_triangleDtmObject(dtmP,HX2,HY2,&fndType2,&p1,&p2,&p3) ) goto errexit ;
                    if( ! fndType2 )bcdtmWrite_message(0,0,0,"** Point Hx2 = %10.4lf Hy2 = %10.4lf Not On Tin",HX2,HY2) ;
                    if( dbg && fndType1 && fndType2 ) bcdtmWrite_message(0,0,0,"Points Are On Tin") ;
                   }
/*
**               Intersect Triangle Edges With Line Of Intersection
*/
                 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Triange Edges With Line Of Intersection") ;
                 hozLineIntersect = 1 ;
                 h1Inside = bcdtmMath_pointInTriangleDtmObject(dtmP,P1,P2,P3,HX1,HY1) ;
                 h2Inside = bcdtmMath_pointInTriangleDtmObject(dtmP,P1,P2,P3,HX2,HY2) ;
                 if( ! h1Inside && ! h2Inside ) hozLineIntersect =  bcdtmVisibility_checkHorizonLineIntersectsTriangle(dtmP,P1,P2,P3,HX1,HY1,HX2,HY2) ;
                 if( hozLineIntersect )
                   {
                    if( ! h1Inside )
                      {
                       bcdtmVisibility_calculateHorizonLineTriangleEdgeIntersection(dtmP,DTMDirection::Clockwise,P1,P2,P3,HX1,HY1,HX2,HY2,&HX1,&HY1) ;
                       bcdtmMath_interpolatePointOnPlane(HX1,HY1,&HZ1,A0,B0,C0,D0) ;
                      }
                    if( ! h2Inside )
                      {
                       bcdtmVisibility_calculateHorizonLineTriangleEdgeIntersection(dtmP,DTMDirection::AntiClockwise,P1,P2,P3,HX1,HY1,HX2,HY2,&HX2,&HY2) ;
                       bcdtmMath_interpolatePointOnPlane(HX2,HY2,&HZ2,A0,B0,C0,D0) ;
                      }
/*
**                  Check Points Are On Tin - Development Only
*/
                    if( cdbg )
                      {
                       if( bcdtmFind_triangleDtmObject(dtmP,HX1,HY1,&fndType,&p1,&p2,&p3) ) goto errexit ;
                       if( ! fndType ) bcdtmWrite_message(0,0,0,"## Point Hx1 = %10.4lf Hy1 = %10.4lf Not On Tin",HX1,HY1) ;
                       if( bcdtmFind_triangleDtmObject(dtmP,HX2,HY2,&fndType,&p1,&p2,&p3) ) goto errexit ;
                       if( ! fndType ) bcdtmWrite_message(0,0,0,"## Point Hx2 = %10.4lf Hy2 = %10.4lf Not On Tin",HX2,HY2) ;
                      }
/*
**                 Store Intersection Line As Break Line
*/
                   if( bcdtmFind_triangleDtmObject(dtmP,HX1,HY1,&fndType1,&p1,&p2,&p3) ) goto errexit ;
                   if( bcdtmFind_triangleDtmObject(dtmP,HX1,HY1,&fndType2,&p1,&p2,&p3) ) goto errexit ;
                   if( dbg && ! fndType1 || ! fndType2  )
                     {
                      bcdtmWrite_message(0,0,0,"Break Line ** %12.5lf %12.5lf ** %12.5lf %12.5lf",HX1,HY1,HX2,HY2) ;
                      bcdtmWrite_message(0,0,0,"hLineP = %8ld ** lhLineP = %8ld ** hhLineP = %8ld",(long)(hLineP-horLinesP),(long)(lhLineP-horLinesP),(long)(hhLineP-horLinesP)) ;
                     }                  
                   if( fndType1 && fndType2 ) 
                     { 
                      breakPts[0].x = HX1 ; breakPts[0].y = HY1 ; breakPts[0].z = HZ1 ;
                      breakPts[1].x = HX2 ; breakPts[1].y = HY2 ; breakPts[1].z = HZ2 ;
                   if( bcdtmObject_storeDtmFeatureInDtmObject(dtmDataP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,breakPts,2)) goto errexit ;
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
 bcdtmWrite_message(0,0,0,"**Error** hLineP = %8ld ** lhLineP = %8ld ** hhLineP = %8ld",(long)(hLineP-horLinesP),(long)(lhLineP-horLinesP),(long)(hhLineP-horLinesP)) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmVisibility_calculateIntersectionLineBetweenPlanes(double Xval,double Zmin,double Zmax,double A0,double B0,double C0,double D0,double A1,double B1,double C1,double D1,double *X1,double *Y1,double *Z1,double *X2,double *Y2,double *Z2)
/*
** This Function Calculates The Line Of Intersection Between Two Planes
*/
{
 double x,y,z,A,B,C,Xm,Ym,Zm,x1,y1,z1,x2,y2,z2,ang ;
/*
** Find Arbitary Solution Parallel To Line Of Intersection
*/
 if( B0 == 0.0 ) B0 = 0.0000000001 ;
 x = Xval ;
 z = (B1*D0/B0 + B1*A0*x/B0 - D1 - A1*x) / (C1-B1*C0/B0) ; 
 y = -(A0*x+C0*z+D0)/B0 ; 
/*
** Calculate Coefficients For Line Of Intersection Vector ( Cross Product Normal Vectors )
*/
 A = B0*C1 - B1*C0 ;
 B = C0*A1 - C1*A0 ;
 C = A0*B1 - A1*B0 ; 
/*
** Calculate Coordinates For Intersection Line
*/
 if( C == 0.0 ) C = 0.0000000001 ;
 *Z1 = Zmin ;
 *X1 = x + A * (*Z1 - z) / C ;
 *Y1 = y + B * (*Z1 - z) / C ;
 *Z2 = Zmax ;
 *X2 = x + A * (*Z2 - z) / C ;
 *Y2 = y + B * (*Z2 - z) / C ;
/*
** Extend Line To Intersect Horizon Line Edges
*/
 Xm  = (*X1+*X2) / 2.0 ;
 Ym  = (*Y1+*Y2) / 2.0 ;
 Zm  = (*Z1+*Z2) / 2.0 ;
 if( Zmin != Zmax )
   {
    ang = bcdtmMath_getAngle(Xm,Ym,*X1,*Y1) ;
    x1  = Xm + tinRadius * cos(ang) ;
    y1  = Ym + tinRadius * sin(ang) ;
    ang = bcdtmMath_getAngle(Xm,Ym,*X2,*Y2) ;
    x2  = Xm + tinRadius * cos(ang) ;
    y2  = Ym + tinRadius * sin(ang) ;
    bcdtmMath_interpolatePointOnLine(*X1,*Y1,*Z1,*X2,*Y2,*Z2,x1,y1,&z1) ;
    bcdtmMath_interpolatePointOnLine(*X1,*Y1,*Z1,*X2,*Y2,*Z2,x2,y2,&z2) ;
   }
 else 
   {
    ang = atan2(B,A) ;
    if( ang < 0.0 ) ang += DTM_2PYE ;
    x1  = Xm + tinRadius * cos(ang) ;
    y1  = Ym + tinRadius * sin(ang) ;
    z1  = Zmin ;
    ang += DTM_PYE ;
    if( ang > DTM_2PYE) ang -= DTM_2PYE ;
    x2  = Xm + tinRadius * cos(ang) ;
    y2  = Ym + tinRadius * sin(ang) ;
    z2  = Zmin ;
    if( bcdtmMath_sideOf(x1,y1,x2,y2,eyeX,eyeY) < 0 ) 
      {
       x  = x1 ; y  = y1 ; z  = z1 ;
       x1 = x2 ; y1 = y2 ; z1 = z2 ;
       x2 = x  ; y2 = y  ; z2 = z  ;
      }     
   }
/*
** Set Return Valures
*/
 *X1 = x1 ; *Y1 = y1 ; *Z1 = z1 ;
 *X2 = x2 ; *Y2 = y2 ; *Z2 = z2 ;
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
BENTLEYDTM_Private int  bcdtmVisibility_checkHorizonLineIntersectsTriangle
(
 BC_DTM_OBJ *dtmP,
 long P1,
 long P2,
 long P3,
 double X1,
 double Y1,
 double X2,
 double Y2
) 
{
 int  ret=DTM_SUCCESS ;
 long s1,s2 ;
/*
** Check For Intersections With P1-P2
*/
 s1 = bcdtmMath_sideOf(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,X1,Y1) ;
 s2 = bcdtmMath_sideOf(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,X2,Y2) ;
 if( s1 != s2 )
   {
    s1 = bcdtmMath_sideOf(X1,Y1,X2,Y2,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) ;
    s2 = bcdtmMath_sideOf(X1,Y1,X2,Y2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) ;
    if( s1 != s2 ) goto errexit  ;
   }
/*
** Check For Intersections With P2-P3
*/
 s1 = bcdtmMath_sideOf(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,X1,Y1) ;
 s2 = bcdtmMath_sideOf(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,X2,Y2) ;
 if( s1 != s2 )
   {
    s1 = bcdtmMath_sideOf(X1,Y1,X2,Y2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) ;
    s2 = bcdtmMath_sideOf(X1,Y1,X2,Y2,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y) ;
    if( s1 != s2 ) goto errexit  ;
   }
/*
** Check For Intersections With P3-P1
*/
 s1 = bcdtmMath_sideOf(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,X1,Y1) ;
 s2 = bcdtmMath_sideOf(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,X2,Y2) ;
 if( s1 != s2 )
   {
    s1 = bcdtmMath_sideOf(X1,Y1,X2,Y2,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y) ;
    s2 = bcdtmMath_sideOf(X1,Y1,X2,Y2,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) ;
    if( s1 != s2 ) goto errexit  ;
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
BENTLEYDTM_Private int   bcdtmVisibility_calculateHorizonLineTriangleEdgeIntersection
(
 BC_DTM_OBJ *dtmP,
 DTMDirection direction,
 long P1,
 long P2,
 long P3,
 double X1,
 double Y1,
 double X2,
 double Y2,
 double *X3,
 double *Y3
) 
/*
** This Function Calculates The Horizon Line Intersection With The Triangle Edge
*/
{
 int ret=DTM_SUCCESS ;
 double Xi,Yi ;
/*
** Calculate Intersection
*/
 if( direction == DTMDirection::Clockwise )
   {
    if( bcdtmMath_sideOf(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,X1,Y1) > 0 )
      {  
       if( bcdtmMath_intersectCordLines(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,X1,Y1,X2,Y2,&Xi,&Yi) )
         { *X3 = Xi ; *Y3 = Yi ; goto errexit  ; }
      } 
    
    if( bcdtmMath_sideOf(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,X1,Y1) > 0 )
      {  
       if( bcdtmMath_intersectCordLines(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,X1,Y1,X2,Y2,&Xi,&Yi) )
         { *X3 = Xi ; *Y3 = Yi ; goto errexit  ; }
      } 

    if( bcdtmMath_sideOf(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,X1,Y1) > 0 )
      {  
       if( bcdtmMath_intersectCordLines(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,X1,Y1,X2,Y2,&Xi,&Yi) )
         { *X3 = Xi ; *Y3 = Yi ; goto errexit  ; }
      } 
    *X3 = X1 ; *Y3 = Y1 ;
   }

 if( direction == DTMDirection::AntiClockwise )
   {
    if( bcdtmMath_sideOf(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,X2,Y2) > 0 )
      {  
       if( bcdtmMath_intersectCordLines(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,X1,Y1,X2,Y2,&Xi,&Yi) )
         { *X3 = Xi ; *Y3 = Yi ; goto errexit  ; }
      } 
    
    if( bcdtmMath_sideOf(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,X2,Y2) > 0 )
      {  
       if( bcdtmMath_intersectCordLines(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,X1,Y1,X2,Y2,&Xi,&Yi) )
         { *X3 = Xi ; *Y3 = Yi ; goto errexit  ; }
      } 

    if( bcdtmMath_sideOf(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,X2,Y2) > 0 )
      {  
       if( bcdtmMath_intersectCordLines(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,X1,Y1,X2,Y2,&Xi,&Yi) )
         { *X3 = Xi ; *Y3 = Yi ; goto errexit  ; }
      } 
    *X3 = X2 ; *Y3 = Y2 ;
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
BENTLEYDTM_Private int bcdtmVisibility_testForCoveringHorizonLineOfTriangle
(
 DTM_HORIZON_LINE *hLineP,
 double Xe,
 double Ye,
 double minAngle,
 double maxAngle,
 double X1,
 double Y1,
 double X2,
 double Y2,
 double X3,
 double Y3,
 long   *coverFlagP
)
/*
** This Function Determines If The Horizon Line Is Between The Eye And Triangle
*/
{
 double dr,Xr,Yr,Xi,Yi,Xh,Yh,angle,radius ;
/*
** Initialise
*/
 *coverFlagP = 0 ;
 if( minAngle < hLineP->Ang1 ) minAngle = hLineP->Ang1 ;
 if( maxAngle > hLineP->Ang2 ) maxAngle = hLineP->Ang2 ;
 angle  = ( minAngle + maxAngle ) / 2.0 ; 
 radius = bcdtmMath_distance(Xe,Ye,X1,Y1) ;
 if( ( dr = bcdtmMath_distance(Xe,Ye,X2,Y2)) > radius ) radius = dr ;
 if( ( dr = bcdtmMath_distance(Xe,Ye,X3,Y3)) > radius ) radius = dr ;
 radius = radius * 2.0 ;
 Xr = Xe + radius * cos(angle) ;
 Yr = Ye + radius * sin(angle) ;
/*
** Test Side 1
*/
 if( bcdtmMath_intersectCordLines(Xe,Ye,Xr,Yr,X1,Y1,X2,Y2,&Xi,&Yi) )
   {
    if( bcdtmMath_intersectCordLines(Xe,Ye,Xr,Yr,hLineP->X1,hLineP->Y1,hLineP->X2,hLineP->Y2,&Xh,&Yh))
      {
       if( bcdtmMath_distance(Xe,Ye,Xh,Yh) < bcdtmMath_distance(Xe,Ye,Xi,Yi) ) *coverFlagP = 1 ;
      }
   }
/*
** Test Side 2
*/
 if( bcdtmMath_intersectCordLines(Xe,Ye,Xr,Yr,X2,Y2,X3,Y3,&Xi,&Yi) )
   {
    if( bcdtmMath_intersectCordLines(Xe,Ye,Xr,Yr,hLineP->X1,hLineP->Y1,hLineP->X2,hLineP->Y2,&Xh,&Yh))
      {
       if( bcdtmMath_distance(Xe,Ye,Xh,Yh) < bcdtmMath_distance(Xe,Ye,Xi,Yi) ) *coverFlagP = 1 ;
      }
   }
/*
** Test Side 3
*/
 if( bcdtmMath_intersectCordLines(Xe,Ye,Xr,Yr,X3,Y3,X1,Y1,&Xi,&Yi) )
   {
    if( bcdtmMath_intersectCordLines(Xe,Ye,Xr,Yr,hLineP->X1,hLineP->Y1,hLineP->X2,hLineP->Y2,&Xh,&Yh))
      {
       if( bcdtmMath_distance(Xe,Ye,Xh,Yh) < bcdtmMath_distance(Xe,Ye,Xi,Yi) ) *coverFlagP = 1 ; 
      }
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
BENTLEYDTM_Private int bcdtmVisibility_findHorizonLineOffsetUsingAngleIndex
(
 void *angleIndexP,
 long numAngleIndex,
 double angleLow,
 double angleHigh,
 long *hLineOffsetP
 )
/*
** This Function Binary Searches The Angle Index
*/
{
 int ret=DTM_SUCCESS ;
 struct  ANGLE_INDEX { double Angle ;long EdgeOfs,AngOfs ; }*baseAngIdxP,*angIdxBotP,*angIdxTopP,*angIdxP ; 
/*
** Initialise
*/
 *hLineOffsetP = 0 ;
 baseAngIdxP = angIdxBotP = ( struct ANGLE_INDEX * ) angleIndexP ;
 angIdxTopP  = angIdxBotP + numAngleIndex - 1 ;
/*
** Test For End Points
*/
 if( angleLow < angIdxBotP->Angle ) return(ret) ; 
 if( angleLow > angIdxTopP->Angle ) *hLineOffsetP = angIdxTopP->EdgeOfs ;
/*
** Binary Scan Angle Index For Closest Entry
*/
 angIdxP = baseAngIdxP + ( (long)(angIdxBotP-baseAngIdxP) + (long)(angIdxTopP-baseAngIdxP)) / 2  ;
 while( angIdxTopP - angIdxBotP > 1 )
   {
    if( angIdxP->Angle == angleLow )  angIdxBotP = angIdxTopP = angIdxP ;
    if( angIdxP->Angle  < angleLow )  angIdxBotP = angIdxP ; else angIdxTopP = angIdxP ;
    angIdxP = baseAngIdxP + ( (long)(angIdxBotP-baseAngIdxP) + (long)(angIdxTopP-baseAngIdxP)) / 2 ;
   }
/*
** Reset Search Limits
*/
 angIdxBotP = ( struct ANGLE_INDEX * ) angleIndexP ;
 angIdxTopP = angIdxBotP + numAngleIndex - 1 ;
/*
**  Go Low Until Index Angle Is Less Than Angle Low
*/
 while ( angIdxP > angIdxBotP && angIdxP->Angle >= angleLow ) --angIdxP ;
 if( angIdxP != angIdxBotP ) ++angIdxP ;
 *hLineOffsetP = angIdxP->EdgeOfs ;
/*
**  Go High Until Index Angle Is Greater Than Angle High
*/
 while ( angIdxP < angIdxTopP && angIdxP->Angle <= angleHigh ) 
   {
    ++angIdxP ;
    if( angIdxP->EdgeOfs < *hLineOffsetP ) *hLineOffsetP = angIdxP->EdgeOfs ;
   } 
/*
** Job Completed
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmVisibility_distanceIndexCompareFunction(const void *Cp1,const void *Cp2)
/*
** Compare Function For Qsort Of Horizon Point Angles
*/
{
 DTM_HORIZON_DISTANCE *hd1P,*hd2P ; 
 hd1P = ( DTM_HORIZON_DISTANCE * ) Cp1 ;
 hd2P = ( DTM_HORIZON_DISTANCE * ) Cp2 ;
 if ( hd1P->Dist < hd2P->Dist  ) return(-1) ;
 if ( hd1P->Dist > hd2P->Dist  ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmVisibility_buildDistanceIndexFromHorizonTable
(
 DTM_HORIZON_LINE *horLinesP,
 long numHorLines,
 DTM_HORIZON_DISTANCE **hozDistPP,
 long *numHorDistP
) 
/*
**  This Function Builds An Angle Index From The Horizon Table
*/
{
 int ret=DTM_SUCCESS ;
 DTM_HORIZON_LINE *hLineP ;
 DTM_HORIZON_DISTANCE *hDistP ;
/*
** Initialise
*/
 *numHorDistP = 0 ;
 if( *hozDistPP != nullptr ) { free(*hozDistPP) ; *hozDistPP = nullptr ; }
/*
** Allocate memory For Horizon Points
*/
 *numHorDistP = numHorLines ;
 *hozDistPP = (DTM_HORIZON_DISTANCE * ) malloc( *numHorDistP * sizeof( DTM_HORIZON_DISTANCE) ) ;
 if( *hozDistPP == nullptr ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Create Distance Index Table
*/
 for( hLineP = horLinesP , hDistP = *hozDistPP ; hLineP < horLinesP + numHorLines ; ++hLineP , ++hDistP )
  {
   hDistP->Dist = bcdtmMath_distance(eyeX,eyeY,(hLineP->X1+hLineP->X2)/2.0,(hLineP->Y1+hLineP->Y2)/2.0 ) ;
   hDistP->HlineOfs = (long)(hLineP-horLinesP) ;
  }
/*
** Sort Distance Index Tables
*/
 qsortCPP(*hozDistPP,*numHorDistP,sizeof(DTM_HORIZON_DISTANCE),bcdtmVisibility_distanceIndexCompareFunction) ;
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
BENTLEYDTM_Private int bcdtmVisibility_horizonLineIndexCompareFunction(const void *Cp1,const void *Cp2)
/*
** Compare Function For Qsort Of Horizon Point Angles
*/
{
 DTM_HORIZON_LINE_INDEX *hIndex1P,*hIndex2P ; 
 hIndex1P = ( DTM_HORIZON_LINE_INDEX * ) Cp1 ;
 hIndex2P = ( DTM_HORIZON_LINE_INDEX * ) Cp2 ;
 if ( hIndex1P->Angle < hIndex2P->Angle ) return(-1) ;
 if ( hIndex1P->Angle > hIndex2P->Angle ) return( 1) ;
 if ( hIndex1P->Htype < hIndex2P->Htype ) return(-1) ;
 if ( hIndex1P->Htype > hIndex2P->Htype ) return( 1) ;
 if ( hIndex1P->Hofs  < hIndex2P->Hofs  ) return(-1) ;
 if ( hIndex1P->Hofs  > hIndex2P->Hofs  ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmVisibility_buildHorizonLineIndexFromHorizonLineTable
(
 DTM_HORIZON_LINE *horLinesP,
 long numHorLines,
 DTM_HORIZON_LINE_INDEX **horLinesIndexPP,
 long *numHorLinesIndexP,
 long                   **hozLinesIndexListPP,
 long *numHorLinesIndexListP,
 long *memHorLinesIndexListP
) 
/*
**  This Function Builds An Angle Index From The Horizon Table
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long *activeHorLineListP=nullptr,numActiveHorLines=0,memActiveHorLines=0 ;
 DTM_HORIZON_LINE *hLineP ;
 DTM_HORIZON_LINE_INDEX *hIndexP ;

 long  MaxActiveEntries=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Building Horizon Line Index Table") ;
    bcdtmWrite_message(0,0,0,"horLinesP             = %p",horLinesP) ;
    bcdtmWrite_message(0,0,0,"numHorLines           = %8ld",numHorLines) ;
    bcdtmWrite_message(0,0,0,"*horLinesIndexPP      = %p",*horLinesIndexPP) ;
    bcdtmWrite_message(0,0,0,"numHorLinesIndexP     = %8ld",*numHorLinesIndexP) ;
    bcdtmWrite_message(0,0,0,"*hozLinesIndexListPP  = %p",*hozLinesIndexListPP) ;
    bcdtmWrite_message(0,0,0,"numHorLinesIndexListP = %8ld",numHorLinesIndexListP) ;
    bcdtmWrite_message(0,0,0,"memHorLinesIndexListP = %8ld",memHorLinesIndexListP) ;
   } 
/*
** Clear Horizon Line Index List
*/
 *numHorLinesIndexListP = *memHorLinesIndexListP = 0 ;
 if( *hozLinesIndexListPP != nullptr ) { free(*hozLinesIndexListPP) ; *hozLinesIndexListPP = nullptr ; }
/*
** Allocate memory For Horizon Points
*/
 if( *horLinesIndexPP != nullptr ) { free(*horLinesIndexPP) ; *horLinesIndexPP = nullptr ; }
 *numHorLinesIndexP = numHorLines * 2 ;
 *horLinesIndexPP = (DTM_HORIZON_LINE_INDEX * ) malloc( *numHorLinesIndexP * sizeof( DTM_HORIZON_LINE_INDEX) ) ;
 if( *horLinesIndexPP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
/*
** Creating Horizon Line Index Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Horizon Line Index Table") ;
 hIndexP = *horLinesIndexPP ;
 for( hLineP = horLinesP ; hLineP < horLinesP + numHorLines ; ++hLineP )
  {
   hIndexP->Angle  = hLineP->Ang1 ;
   hIndexP->Htype  = 1 ;
   hIndexP->Hofs   = (long)(hLineP-horLinesP) ;
   hIndexP->Hlist  = DTM_NULL_PNT ;
   hIndexP->Nhlist = 0 ;
   ++hIndexP ;
   hIndexP->Angle  = hLineP->Ang2 ;
   hIndexP->Htype  = 2 ;
   hIndexP->Hofs   = (long)(hLineP-horLinesP) ;
   hIndexP->Hlist  = DTM_NULL_PNT ;
   hIndexP->Nhlist = 0 ;
   ++hIndexP ;
  }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Horizon Line Index Table Entries = %6ld",*numHorLinesIndexP) ;
/*
** Sort Horizon Index Tables
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Horizon Line Index Table") ;
 qsortCPP(*horLinesIndexPP,*numHorLinesIndexP,sizeof(DTM_HORIZON_LINE_INDEX),bcdtmVisibility_horizonLineIndexCompareFunction) ;
/*
** Scan Horizon Line Index And Determine Active Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Horizon Line Active Lists") ;
 for( hIndexP = *horLinesIndexPP ; hIndexP < *horLinesIndexPP + *numHorLinesIndexP ; ++hIndexP )
   {
    if( dbg && (long)(hIndexP-*horLinesIndexPP) % 10000 == 0 ) bcdtmWrite_message(0,0,0,"Number Of Horizon Lines Processed = %8ld of %8ld",(long)(hIndexP-*horLinesIndexPP),*numHorLinesIndexP) ;
    if( hIndexP->Htype == 1 ) if( bcdtmVisibility_addHorizonLineToActiveList(&activeHorLineListP,&numActiveHorLines,&memActiveHorLines,hIndexP->Hofs)) goto errexit ;
    if( hIndexP->Htype == 2 ) if( bcdtmVisibility_removeHorizonLineFromActiveList(activeHorLineListP,&numActiveHorLines,hIndexP->Hofs)) goto errexit ;
    hIndexP->Hlist  = *numHorLinesIndexListP  ;
    hIndexP->Nhlist = numActiveHorLines ; 
    if( numActiveHorLines > 0 ) 
      {
       if( bcdtmVisibility_addHorizonListEntiesToHorizonIndexList(hozLinesIndexListPP,numHorLinesIndexListP,memHorLinesIndexListP,activeHorLineListP,numActiveHorLines)) 
         { 
          free(*hozLinesIndexListPP) ; 
          hozLinesIndexListPP = nullptr ;
          goto errexit ; 
         }
      } 
    if( numActiveHorLines > MaxActiveEntries ) MaxActiveEntries = numActiveHorLines ;
   }
/*
** Log Stats On Table Sizes
*/   
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Maximum Size Of Horizon Line Active List = %6ld",MaxActiveEntries) ;
    bcdtmWrite_message(0,0,0,"Maximum Size Of Horizon Line Index List  = %6ld",*numHorLinesIndexListP) ;
   } 
/*
** Realloc Memory For Horizon Line Index List
*/
 *hozLinesIndexListPP = (long *) realloc(*hozLinesIndexListPP,*numHorLinesIndexListP*sizeof(long)) ;
/*
** Clean Up
*/
 cleanup :
 if( activeHorLineListP != nullptr ) free(activeHorLineListP);
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Horizon Line Index Table Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Horizon Line Index Table Error") ;
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
BENTLEYDTM_Private int bcdtmVisibility_addHorizonLineToActiveList
(
 long **activeHorLinesPP,
 long *numActiveHorLinesP,
 long *memActiveHorLinesP,
 long hozLineOffset
 )
/*
** This Functions Adds An Entry To The Active Horizon Line List
*/
{
 int  ret=DTM_SUCCESS ;
 long memInc=1000 ;
/*
** Test For Memory
*/
 if( *numActiveHorLinesP == *memActiveHorLinesP )
   {
    *memActiveHorLinesP = *memActiveHorLinesP + memInc ;
    if( *activeHorLinesPP == nullptr ) *activeHorLinesPP = (long*)malloc ( *memActiveHorLinesP * sizeof(long)) ;
    else                            *activeHorLinesPP = (long*)realloc( *activeHorLinesPP, *memActiveHorLinesP * sizeof(long)) ;
    if( *activeHorLinesPP == nullptr ) 
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
       goto errexit  ;
      }
   }
/*
** Store Entry
*/
 *(*activeHorLinesPP+*numActiveHorLinesP) = hozLineOffset ;
 ++*numActiveHorLinesP ;
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
BENTLEYDTM_Private int bcdtmVisibility_removeHorizonLineFromActiveList
(
 long *activeHorLinesP,
 long *numActiveHorLinesP,
 long hozLineOffset
 )
/*
** This Functions Removes An Entry From The Active Line List
*/
{
 int  ret=DTM_SUCCESS ;
 long *hLineP,*hLineEntryP=nullptr ;
/*
** Check For Entries
*/
 if( *numActiveHorLinesP <= 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"No Horizon Line Entries In Active List") ; 
    goto errexit  ; 
   }
/*
** Find Entry
*/
 for( hLineP = activeHorLinesP ; hLineP < activeHorLinesP + *numActiveHorLinesP && hLineEntryP == nullptr ; ++hLineP )
   {
    if( *hLineP == hozLineOffset ) hLineEntryP = hLineP ; 
   }
 if( hLineEntryP == nullptr ){ bcdtmWrite_message(1,0,0,"Horizon Line Entry %6ld Not In Active List",hozLineOffset) ; goto errexit  ; } 
/*
** Copy Over Removed Entry
*/
 while ( hLineEntryP < activeHorLinesP + *numActiveHorLinesP - 1 ) { *hLineEntryP = *(hLineEntryP+1) ; ++hLineEntryP ; }
 --*numActiveHorLinesP ;
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
BENTLEYDTM_Private int bcdtmVisibility_addHorizonListEntiesToHorizonIndexList
(
 long **hozIndexPP,
 long *numHorIndexP,
 long *memHorIndexP,
 long *hozListP,
 long numHorList
)
/*
** This Functions Adds An Entry From The Active Line hozListP
*/
{
 int  ret=DTM_SUCCESS ;
 long *idxP,*listP,memInc=1000000 ;
/*
** Test For No Entries
*/
 if( numHorList <= 0 ) 
   {
    bcdtmWrite_message(2,0,0,"No Horizon List Entries") ;
    goto errexit ;
   }
/*
** Test For Memory
*/
 if( *numHorIndexP + numHorList > *memHorIndexP )
   {
    while( *numHorIndexP + numHorList > *memHorIndexP ) *memHorIndexP = *memHorIndexP + memInc ;
    if( *hozIndexPP == nullptr ) *hozIndexPP = (long*)malloc ( *memHorIndexP * sizeof(long)) ;
    else              *hozIndexPP = (long*)realloc( *hozIndexPP, *memHorIndexP * sizeof(long)) ;
    if( *hozIndexPP == nullptr ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit  ; 
      }
   }
/*
** Store Entry
*/
 for(  idxP = *hozIndexPP + *numHorIndexP , listP = hozListP ; listP < hozListP + numHorList ; ++idxP , ++listP ) *idxP = *listP ; 
 *numHorIndexP = *numHorIndexP + numHorList ;
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
BENTLEYDTM_Private int bcdtmVisibility_findHorizonLineEntryListUsingHorizonLineIndex
(
 DTM_HORIZON_LINE_INDEX *hozLineIndexP,
 long numHorLineIndex,
 long *hozLineIndexListP,
 double angle,
 long *hozLineOffsetP
)
/*
** This Function Binary Searches The angle Index
*/
{
 int   dbg=DTM_TRACE_VALUE(0) ;
 long  *hListP ;
 DTM_HORIZON_LINE_INDEX *hIdxP,*hIdxBotP,*hIdxTopP,*sIdxP ;
/*
** Check For Null Horizon Index
*/
 if( hozLineIndexP == nullptr )
   {
    bcdtmWrite_message(0,0,0,"Null Horizon Line Index") ;
    return(1) ;
   }  
 if( hozLineIndexListP == nullptr )
   {
    bcdtmWrite_message(0,0,0,"Null Horizon Line Index List") ;
    return(1) ;
   }  
/*
** Initialise
*/
 *hozLineOffsetP = DTM_NULL_PNT ;
 hIdxBotP = hozLineIndexP ;
 hIdxTopP = hozLineIndexP + numHorLineIndex - 1 ;
/*
** Test For End Points
*/
 if( angle < hIdxBotP->Angle || angle > hIdxTopP->Angle ) return(0) ; 
/*
** Binary Scan Horizon Line Index For Closest Entry
*/
 hIdxP =  hozLineIndexP + ((long)(hIdxBotP-hozLineIndexP)+(long)(hIdxTopP-hozLineIndexP)) / 2  ;
 while( hIdxTopP - hIdxBotP > 1 )
   {
    if( hIdxP->Angle == angle )  hIdxBotP = hIdxTopP = hIdxP ;
    if( hIdxP->Angle  < angle )  hIdxBotP = hIdxP ; else hIdxTopP = hIdxP ;
    hIdxP =  hozLineIndexP + ((long)(hIdxBotP-hozLineIndexP)+(long)(hIdxTopP-hozLineIndexP)) / 2  ;
   }
/*
** 
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"hIdxP offset = %6ld",(long)(hIdxP-hozLineIndexP)) ;
    if( angle == hIdxP->Angle ) bcdtmWrite_message(0,0,0,"Equal   angles %12.10lf %12.10lf",angle,hIdxP->Angle) ;
    if( angle != hIdxP->Angle ) bcdtmWrite_message(0,0,0,"Unequal angles %12.10lf %12.10lf",angle,hIdxP->Angle) ;
   }
/*
** Get Offset For Unequal angles 
*/
 if( angle != hIdxP->Angle ) 
   {
    if( hIdxP > hozLineIndexP ) --hIdxP ;
    if( hIdxP->Nhlist > 0 )
      {
       hListP = hozLineIndexListP + hIdxP->Hlist ;
       *hozLineOffsetP = *hListP ;
       while ( hListP < hozLineIndexListP + hIdxP->Hlist + hIdxP->Nhlist )
         {
          if( *hListP < *hozLineOffsetP ) *hozLineOffsetP = *hListP ;
          ++hListP ;
         }
      }
   } 

/*
** Get Offset For Equal angles 
*/
 else 
   {
    sIdxP = hIdxP ;
    if( hIdxP > hozLineIndexP && hIdxP->Angle == angle ) --hIdxP ;
    if( hIdxP->Nhlist > 0 )
      {
       hListP = hozLineIndexListP + hIdxP->Hlist ;
       *hozLineOffsetP = *hListP ;
       while ( hListP < hozLineIndexListP + hIdxP->Hlist + hIdxP->Nhlist )
         {
          if( *hListP < *hozLineOffsetP ) *hozLineOffsetP = *hListP ;
          ++hListP ;
         }
      }
    hIdxP = sIdxP ;
    if( hIdxP < hozLineIndexP + numHorLineIndex && hIdxP->Angle == angle ) ++hIdxP ;
    --hIdxP ; 
    if( hIdxP->Nhlist > 0 )
      {
       hListP = hozLineIndexListP + hIdxP->Hofs ;
       if( *hozLineOffsetP == DTM_NULL_PNT ) *hozLineOffsetP = *hListP ;
       while ( hListP < hozLineIndexListP + hIdxP->Hofs + hIdxP->Nhlist )
         {
          if( *hListP < *hozLineOffsetP ) *hozLineOffsetP = *hListP ;
          ++hListP ;
         }
      }
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
BENTLEYDTM_Private int bcdtmVisibility_determinePointVisibilityUsingVisibilityTables
(
 DTM_HORIZON_LINE *horLinesP,
 long numHorLines,
 DTM_HORIZON_LINE_INDEX *horLinesIndexP,
 long numHorLinesIndex,
 long *hozLinesIndexListP,
 double Xe,
 double Ye,
 double Ze,
 double x,
 double y,
 double z,
 long *visibilityP
 )
/*
** This Function Determines The Visibility Of Point By Using Visibility Tables
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sideof,hozLineOffset ;
 double eyeAngle ;
 DTM_HORIZON_LINE  *hLineP,*hLineLowP,*hLineHighP ;
/*
** Log Function Parameters
*/ 
if( dbg )
  {
   bcdtmWrite_message(0,0,0,"Determining Point Visibility") ;
   bcdtmWrite_message(0,0,0,"horLinesP          = %p",horLinesP) ;
   bcdtmWrite_message(0,0,0,"numHorLines        = %8ld",numHorLines) ;
   bcdtmWrite_message(0,0,0,"horLinesIndexP     = %p",horLinesIndexP) ;
   bcdtmWrite_message(0,0,0,"numHorLinesIndex   = %8ld",numHorLinesIndex) ;
   bcdtmWrite_message(0,0,0,"hozLinesIndexListP = %p",hozLinesIndexListP) ;
   bcdtmWrite_message(0,0,0,"Xe                 = %15.5lf",Xe) ;
   bcdtmWrite_message(0,0,0,"Ye                 = %15.5lf",Ye) ;
   bcdtmWrite_message(0,0,0,"Ze                 = %15.5lf",Ze) ;
   bcdtmWrite_message(0,0,0,"x                  = %15.5lf",x) ;
   bcdtmWrite_message(0,0,0,"y                  = %15.5lf",y) ;
   bcdtmWrite_message(0,0,0,"z                  = %15.5lf",z) ;
  }            

/*
** Initialise
*/
 *visibilityP = 1 ;
/*
** Calculate Angle From Eye To Point
*/
 eyeAngle = bcdtmMath_getAngle(Xe,Ye,x,y) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"EyeAngle = %16.14lf",eyeAngle) ;
/*
** Find Horizon Line Entry List Covering Point
*/
 if( bcdtmVisibility_findHorizonLineEntryListUsingHorizonLineIndex(horLinesIndexP,numHorLinesIndex,hozLinesIndexListP,eyeAngle,&hozLineOffset)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"hozLineOffset = %6ld",hozLineOffset) ;
 if( hozLineOffset != DTM_NULL_PNT ) hLineLowP = horLinesP + hozLineOffset ;
 else                           hLineLowP = horLinesP ;
 hLineHighP = horLinesP + numHorLines - 1 ;
 for( hLineP = hLineLowP ; hLineP <= hLineHighP && hLineP->Ang1 < eyeAngle ; ++hLineP  )
   {
    if( dbg) bcdtmWrite_message(0,0,0,"Ang1 = %12.10lf Ang2 = %12.10lf",hLineP->Ang1,hLineP->Ang2) ;
    if( eyeAngle >= hLineP->Ang1 && eyeAngle <= hLineP->Ang2 )
      {
       sideof = bcdtmMath_sideOf(hLineP->X1,hLineP->Y1,hLineP->X2,hLineP->Y2,x,y) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Eye Covered sideOf = %2d",sideof) ;
       if( sideof <= 0 ) 
         {
          bcdtmVisibility_determineVisibilityOfPoint(Xe,Ye,Ze,hLineP->X2,hLineP->Y2,hLineP->Z2,hLineP->X1,hLineP->Y1,hLineP->Z1,x,y,z,visibilityP) ;
          if( *visibilityP == 0 ) { if( dbg) bcdtmWrite_message(0,0,0,"Point Invisible") ; return(0) ; }
         }
      } 
   }
 if( dbg) bcdtmWrite_message(0,0,0,"Point Visible") ;
/*
** Scan All Horizon Lines
*/
 if( dbg )
   {
    for( hLineP = horLinesP ; hLineP < horLinesP +numHorLines ; ++hLineP  )
      {
       if( eyeAngle >= hLineP->Ang1 && eyeAngle <= hLineP->Ang2 )
         {
          bcdtmWrite_message(0,0,0,"Eye Covered ** ang1 = %12.10lf eye = %12.10lf ang2 = %12.10lf",hLineP->Ang1,eyeAngle,hLineP->Ang2) ;
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
BENTLEYDTM_Private int bcdtmVisibility_determineLineVisibilityUsingVisibilityTables
(
 DTM_HORIZON_LINE *horLinesP,
 long numHorLines,
 DTM_HORIZON_LINE_INDEX *horLinesIndexP,
 long numHorLinesIndex,
 long *horIndexListP,
 double Xe,
 double Ye,
 double Ze,
 double X1,
 double Y1,
 double Z1,
 double X2,
 double Y2,
 double Z2,
 long *lineVisibilityP
 )
/*
** This Function Determines The Visibility Of A Line By Using The Visibility Tables
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   isw,process,visibility,sideof,tLineOffset,hLineOffset=0 ;
 long   reverseFlag,normalFlag,numVisible,numInvisible ;
 long   numDistLines,numDistHorLines=0,memDistHorLines=0,numTempLines,numTempHorLines=0,memTempHorLines=0 ;
 double s,radius,ratio,Ang1,Ang2,Ang,AngL,D1,D2,D3,x,y,z,L1 ;
 DPoint3d    visibilityPts[2] ; 
 DTM_HORIZON_LINE  *dLineP,*hLineP,*tLineP,*hLineLowP,*hLineHighP ;
 DTM_HORIZON_LINE  *tLineOffset1P,*tLineOffset2P,saveHorLine ;
 DTM_HORIZON_LINE  *distHorLinesP=nullptr,*tempHorLinesP=nullptr  ;
 DTM_HORIZON_LINE_INDEX *hIndexP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Line Visibility Using Visibility Tables") ;
/*
** Initialise
*/
 *lineVisibilityP = 0 ;
 radius = tinRadius ;
 L1 = bcdtmMath_distance(X1,Y1,X2,Y2) ; 
 if( bcdtmVisibility_storeVertice(100,0,0.0,0.0,0.0) ) goto errexit  ; 
/*
** Set Line Anti Clockwise About Eye
*/
 reverseFlag = 0 ;
 sideof = bcdtmMath_sideOf(X1,Y1,X2,Y2,Xe,Ye) ;
 if( sideof == 0 ) return(0) ;
 if( sideof  < 0 ) 
   { 
    s = X1 ; X1 = X2 ; X2 = s ; 
    s = Y1 ; Y1 = Y2 ; Y2 = s ; 
    s = Z1 ; Z1 = Z2 ; Z2 = s ; 
    reverseFlag = 1 ;
   }
/*
** Get Start And End Angles Of Line
*/
 D1 = bcdtmMath_distance(Xe,Ye,X1,Y1) ; 
 D2 = bcdtmMath_distance(Xe,Ye,X2,Y2) ; 
 Ang1 = bcdtmMath_getAngle(Xe,Ye,X1,Y1) ;
 Ang2 = bcdtmMath_getAngle(Xe,Ye,X2,Y2) ;
 AngL = Ang2 ;
/*
** Allocate Memory For distHorLinesP Horizon Structure
*/
 memDistHorLines  = 2 ;
 distHorLinesP = ( DTM_HORIZON_LINE *) malloc(memDistHorLines*sizeof(DTM_HORIZON_LINE)) ;
/*
** Initialise Horizon Line Structure
*/
 distHorLinesP->Ang1 = Ang1 ;
 distHorLinesP->Ang2 = Ang2 ;
 distHorLinesP->D1 = D1 ;
 distHorLinesP->D2 = D2 ;
 distHorLinesP->X1 = X1 ; distHorLinesP->Y1 = Y1 ; distHorLinesP->Z1 = Z1 ; 
 distHorLinesP->X2 = X2 ; distHorLinesP->Y2 = Y2 ; distHorLinesP->Z2 = Z2 ; 
 distHorLinesP->ActiveFlag = 1 ; 
 numDistHorLines = numDistLines = 1 ;
/*
** Normalise Line Across Zero Degrees
*/
 normalFlag = 0 ;
 if( Ang1 > Ang2 )
   {
    ratio = (Ye-Y1)/(Y2-Y1) ;
    (distHorLinesP+1)->Ang1 = 0.0 ;
    (distHorLinesP+1)->Ang2 = Ang2 ;
    (distHorLinesP+1)->D2 = D2 ;
    distHorLinesP->Ang2 = DTM_2PYE ;
    distHorLinesP->D2 = (distHorLinesP+1)->D1 = bcdtmMath_distance(Xe,Ye,distHorLinesP->X2,distHorLinesP->Y2) ;
    (distHorLinesP+1)->X2 = X2 ; (distHorLinesP+1)->Y2 = Y2 ; (distHorLinesP+1)->Z2 = Z2 ; 
    distHorLinesP->X2 = (distHorLinesP+1)->X1 = X1 + ratio * (X2-X1) ;
    distHorLinesP->Z2 = (distHorLinesP+1)->Z1 = Z1 + ratio * (Z2-Z1) ;
    distHorLinesP->Y2 = (distHorLinesP+1)->Y1 = Ye  ;
    (distHorLinesP+1)->ActiveFlag = 1 ; 
    numDistHorLines = numDistLines = 2 ;
    normalFlag = 1 ;
   }
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"reverseFlag = %2ld normalFlag = %2ld NdLineP = %2ld",reverseFlag,normalFlag,numDistLines) ;
    bcdtmWrite_message(0,0,0,"Ang1 = %12.10lf X1 = %10.4lf Y1 = %10.4lf",Ang1,X1,Y1) ;
    bcdtmWrite_message(0,0,0,"Ang2 = %12.10lf X2 = %10.4lf Y2 = %10.4lf",Ang2,X2,Y2) ;
    for( dLineP = distHorLinesP ; dLineP < distHorLinesP + numDistLines ; ++dLineP )
      {
       bcdtmWrite_message(0,0,0,"distHorLinesP = %6ld ** %12.8lf %12.8lf ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(dLineP-distHorLinesP),dLineP->Ang1,dLineP->Ang2,dLineP->X1,dLineP->Y1,dLineP->Z2,dLineP->X2,dLineP->Y2,dLineP->Z2) ;
      }
   }
/*
** Intersect Data Line With Horizon Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Data Line With Horizon Lines") ;
 for( dLineP = distHorLinesP ; dLineP < distHorLinesP + numDistLines ; ++dLineP )
   {
    isw = 0 ;
    for( hIndexP = horLinesIndexP ; hIndexP <= horLinesIndexP + numHorLinesIndex && hIndexP->Angle < dLineP->Ang2 ; ++hIndexP )
      {
       if( hIndexP->Angle > dLineP->Ang1 && hIndexP->Angle < dLineP->Ang2 )
         {
          if( bcdtmVisibility_determineIfHorizonLineIsCovered(Xe,Ye,(horLinesP+hIndexP->Hofs),dLineP) )
            {
             if( dLineP->D1 >= dLineP->D2 ) radius = dLineP->D1 ;
             else                         radius = dLineP->D2 ; 
             bcdtmVisibility_calculateHorizonLineIntercept(dLineP,Xe,Ye,hIndexP->Angle,radius*10.0,&x,&y,&z) ;
             D1 = bcdtmMath_distance(dLineP->X1,dLineP->Y1,x,y) ;
             D2 = bcdtmMath_distance(dLineP->X2,dLineP->Y2,x,y) ;
             if( D1 > 0.0 && D2 > 0.0 )
               {
                D3 = bcdtmMath_distance(Xe,Ye,x,y) ;
                if( bcdtmVisibility_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,dLineP->Ang1,hIndexP->Angle,dLineP->D1,D3,dLineP->X1,dLineP->Y1,dLineP->Z1,x,y,z)) goto errexit  ; 
                dLineP->Ang1 = hIndexP->Angle ; dLineP->D1 = D3 ; dLineP->X1 = x ; dLineP->Y1 = y ; dLineP->Z1 = z ;
               }
            }
         } 
      }     
    if( bcdtmMath_distance(dLineP->X1,dLineP->Y1,dLineP->X2,dLineP->Y2) > 0 ) 
      {  
       if( bcdtmVisibility_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,dLineP->Ang1,dLineP->Ang2,dLineP->D1,dLineP->D2,dLineP->X1,dLineP->Y1,dLineP->Z1,dLineP->X2,dLineP->Y2,dLineP->Z2)) goto errexit  ; 
      }
   }  
/*
** Sort tempHorLinesP Structure
*/
 if( numTempHorLines > 1 ) qsort(tempHorLinesP,numTempHorLines,sizeof(DTM_HORIZON_LINE),bcdtmVisibility_horizonPointsAngleCompareFunction) ;
/*
** Check Calculations ** Development Only
*/
 if( cdbg )
   {
    if( bcdtmVisibility_checkLengthVisibilityLine(tempHorLinesP,numTempHorLines,L1) ||
        bcdtmVisibility_checkAngleSequenceVisibilityLine(tempHorLinesP,numTempHorLines,normalFlag,AngL) )
      {
       for( tLineP = tempHorLinesP ; tLineP < tempHorLinesP + numTempHorLines ; ++tLineP )
         {
          bcdtmWrite_message(0,0,0,"tempHorLinesP = %6ld %2ld ** %12.10lf %12.10lf ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(tLineP-tempHorLinesP),tLineP->ActiveFlag,tLineP->Ang1,tLineP->Ang2,tLineP->X1,tLineP->Y1,tLineP->Z2,tLineP->X2,tLineP->Y2,tLineP->Z2) ;
         }
       goto errexit  ;
      }
   }  
/*
** Determine Line Visibility
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Data Line Visibility") ;
 process = 1 ;
 while ( process )
   {
    process = 0 ;
    numTempLines = numTempHorLines ;
    for( tLineP = tempHorLinesP ; tLineP < tempHorLinesP + numTempLines ; ++tLineP )
      {
       if( tLineP->ActiveFlag > 0 )
         {
          bcdtmVisibility_findHorizonLineEntryListUsingHorizonLineIndex(horLinesIndexP,numHorLinesIndex,horIndexListP,tLineP->Ang1,&hLineOffset) ;
          if( hLineOffset != DTM_NULL_PNT ) hLineLowP = horLinesP + hLineOffset ;
          else                         hLineLowP = horLinesP ;
          hLineHighP = horLinesP + numHorLines - 1 ; 
          for( hLineP = hLineLowP ; hLineP <= hLineHighP && hLineP->Ang1 < tLineP->Ang2 && tLineP->ActiveFlag > 0 ; ++hLineP )
            { 
             if( tLineP->Ang1 >= hLineP->Ang1 && tLineP->Ang2 <= hLineP->Ang2 )              
               {
                if( bcdtmVisibility_determineIfHorizonLineIsCovered(Xe,Ye,hLineP,tLineP) )
                  {
                   process = 0 ;
                   bcdtmVisibility_determineVisibilityOfEdge(Xe,Ye,Ze,hLineP->X2,hLineP->Y2,hLineP->Z2,hLineP->X1,hLineP->Y1,hLineP->Z1,tLineP->X1,tLineP->Y1,tLineP->Z1,tLineP->X2,tLineP->Y2,tLineP->Z2,&visibility,visibilityPts) ;
                   if( visibility == -1 ) { process = 1 ; tLineP->ActiveFlag = 0 ; }
                   if( visibility ==  0 ) 
                     {
                      process = 1 ;
                      tLineP->ActiveFlag = -1 ;
                      tLineOffset = (long)(tLineP-tempHorLinesP) ;
                      if( visibilityPts[0].x == tLineP->X1 && visibilityPts[0].y == tLineP->Y1 )
                        {
                         Ang = bcdtmMath_getAngle(Xe,Ye,visibilityPts[1].x,visibilityPts[1].y) ;
                         D2  = bcdtmMath_distance(Xe,Ye,visibilityPts[1].x,visibilityPts[1].y) ; 
                         if( bcdtmVisibility_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,tLineP->Ang1,Ang,tLineP->D1,D2,tLineP->X1,tLineP->Y1,tLineP->Z1,visibilityPts[1].x,visibilityPts[1].y,visibilityPts[1].z)) goto errexit  ; 
                         if( bcdtmVisibility_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,Ang,tLineP->Ang2,D2,tLineP->D2,visibilityPts[1].x,visibilityPts[1].y,visibilityPts[1].z,tLineP->X2,tLineP->Y2,tLineP->Z2)) goto errexit  ; 
                         (tempHorLinesP+numTempHorLines-1)->ActiveFlag = 0 ;
                        } 
                      else
                        {
                         Ang = bcdtmMath_getAngle(Xe,Ye,visibilityPts[0].x,visibilityPts[0].y) ;
                         if( Ang > tLineP->Ang1 && Ang < tLineP->Ang2 )
                           {
                            D2  = bcdtmMath_distance(Xe,Ye,visibilityPts[0].x,visibilityPts[0].y) ; 
                            if( bcdtmVisibility_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,tLineP->Ang1,Ang,tLineP->D1,D2,tLineP->X1,tLineP->Y1,tLineP->Z1,visibilityPts[0].x,visibilityPts[0].y,visibilityPts[0].z)) goto errexit  ; 
                            (tempHorLinesP+numTempHorLines-1)->ActiveFlag = 0 ;
                            if( bcdtmVisibility_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,Ang,tLineP->Ang2,D2,tLineP->D2,visibilityPts[0].x,visibilityPts[0].y,visibilityPts[0].z,tLineP->X2,tLineP->Y2,tLineP->Z2)) goto errexit  ; 
                           }
                         else tLineP->ActiveFlag = 0 ;
                        }
                      tLineP = tempHorLinesP + tLineOffset ;
                     }
                  }
               }
            }
         }   
      }
   }
/*
** Remove Deleted Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Deleted Lines") ;
 tLineOffset1P = tempHorLinesP ;
 for( tLineOffset2P = tempHorLinesP ; tLineOffset2P < tempHorLinesP + numTempHorLines ; ++tLineOffset2P )
   {
    if( tLineOffset2P->ActiveFlag != -1 )
      {
       if( tLineOffset1P != tLineOffset2P ) *tLineOffset1P = *tLineOffset2P ;
       ++tLineOffset1P ;
      }
   }
 numTempHorLines = (long)(tLineOffset1P-tempHorLinesP) ;
/*
** Sort tempHorLinesP Structure
*/
 if( numTempHorLines > 1 ) qsortCPP(tempHorLinesP,numTempHorLines,sizeof(DTM_HORIZON_LINE),bcdtmVisibility_horizonPointsAngleCompareFunction) ;
/*
** Check Calculations ** Development Only
*/
 if( cdbg )
   {
    if( bcdtmVisibility_checkLengthVisibilityLine(tempHorLinesP,numTempHorLines,L1) ||
        bcdtmVisibility_checkAngleSequenceVisibilityLine(tempHorLinesP,numTempHorLines,normalFlag,AngL) )
      {
       for( tLineP = tempHorLinesP ; tLineP < tempHorLinesP + numTempHorLines ; ++tLineP )
         {
          bcdtmWrite_message(0,0,0,"tempHorLinesP = %6ld %2ld ** %12.10lf %12.10lf ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(tLineP-tempHorLinesP),tLineP->ActiveFlag,tLineP->Ang1,tLineP->Ang2,tLineP->X1,tLineP->Y1,tLineP->Z2,tLineP->X2,tLineP->Y2,tLineP->Z2) ;
         }
       goto errexit  ;
      }
   }  
/*
** De Normalise Line
*/
 if( normalFlag )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Denormalising Lines") ;
    for( tLineP = tempHorLinesP ; tLineP->Ang2 <= AngL ; ++tLineP )
      {
       tLineOffset = (long)(tLineP-tempHorLinesP) ;
       if( bcdtmVisibility_storePointInHorizonTable(&tempHorLinesP,&numTempHorLines,&memTempHorLines,tLineP->Ang1,tLineP->Ang2,tLineP->D1,tLineP->D2,tLineP->X1,tLineP->Y1,tLineP->Z1,tLineP->X2,tLineP->Y2,tLineP->Z2)) goto errexit  ; 
       tLineP = tempHorLinesP + tLineOffset ;
       (tempHorLinesP+numTempHorLines-1)->ActiveFlag = tLineP->ActiveFlag ;
       tLineP->ActiveFlag = -1 ;
      }
    tLineOffset1P = tempHorLinesP ;
    for( tLineOffset2P = tempHorLinesP ; tLineOffset2P < tempHorLinesP + numTempHorLines ; ++tLineOffset2P )
      {
       if( tLineOffset2P->ActiveFlag != -1 )
         {
          if( tLineOffset1P != tLineOffset2P ) *tLineOffset1P = *tLineOffset2P ;
          ++tLineOffset1P ;
         }
      }
    numTempHorLines = (long)(tLineOffset1P-tempHorLinesP) ;
   }  
/*
** Check Calculations ** Development Only
*/
 if( cdbg )
   {
    if( bcdtmVisibility_checkLengthVisibilityLine(tempHorLinesP,numTempHorLines,L1) ||
        bcdtmVisibility_checkAngleSequenceVisibilityLine(tempHorLinesP,numTempHorLines,normalFlag,DTM_2PYE) )
      {
       for( tLineP = tempHorLinesP ; tLineP < tempHorLinesP + numTempHorLines ; ++tLineP )
         {
          bcdtmWrite_message(0,0,0,"tempHorLinesP = %6ld %2ld ** %12.10lf %12.10lf ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(tLineP-tempHorLinesP),tLineP->ActiveFlag,tLineP->Ang1,tLineP->Ang2,tLineP->X1,tLineP->Y1,tLineP->Z2,tLineP->X2,tLineP->Y2,tLineP->Z2) ;
         }
       goto errexit  ;
      }
   }  
/*
** Reverse Line Direction
*/
 if( reverseFlag )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reversing Line Direction") ;
    hLineLowP = tempHorLinesP ;
    hLineHighP = tempHorLinesP + numTempHorLines - 1 ; 
    while ( hLineLowP <= hLineHighP )
      {
       *(&saveHorLine) = *hLineLowP ;
       if( hLineLowP != hLineHighP )
         {
          hLineLowP->ActiveFlag = hLineHighP->ActiveFlag ;  
          hLineLowP->Ang1 = hLineHighP->Ang2 ;
          hLineLowP->Ang2 = hLineHighP->Ang1 ;
          hLineLowP->D1   = hLineHighP->D2 ;
          hLineLowP->D2   = hLineHighP->D1 ;
          hLineLowP->X1   = hLineHighP->X2 ;
          hLineLowP->Y1   = hLineHighP->Y2 ;
          hLineLowP->Z1   = hLineHighP->Z2 ;
          hLineLowP->X2   = hLineHighP->X1 ;
          hLineLowP->Y2   = hLineHighP->Y1 ;
          hLineLowP->Z2   = hLineHighP->Z1 ;
         }
       hLineHighP->ActiveFlag = saveHorLine.ActiveFlag ;  
       hLineHighP->Ang1 = saveHorLine.Ang2 ;
       hLineHighP->Ang2 = saveHorLine.Ang1 ;
       hLineHighP->D1   = saveHorLine.D2 ;
       hLineHighP->D2   = saveHorLine.D1 ;
       hLineHighP->X1   = saveHorLine.X2 ;
       hLineHighP->Y1   = saveHorLine.Y2 ;
       hLineHighP->Z1   = saveHorLine.Z2 ;
       hLineHighP->X2   = saveHorLine.X1 ;
       hLineHighP->Y2   = saveHorLine.Y1 ;
       hLineHighP->Z2   = saveHorLine.Z1 ;
       ++hLineLowP ; --hLineHighP ;  
      }
   }
/*
** Check Calculations ** Development Only
*/
 if( cdbg )
   {
    if( bcdtmVisibility_checkLengthVisibilityLine(tempHorLinesP,numTempHorLines,L1) ||
        bcdtmVisibility_checkAngleSequenceVisibilityLine(tempHorLinesP,numTempHorLines,normalFlag,0.0) )
      {
       for( tLineP = tempHorLinesP ; tLineP < tempHorLinesP + numTempHorLines ; ++tLineP )
         {
          bcdtmWrite_message(0,0,0,"tempHorLinesP = %6ld %2ld ** %12.10lf %12.10lf ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(tLineP-tempHorLinesP),tLineP->ActiveFlag,tLineP->Ang1,tLineP->Ang2,tLineP->X1,tLineP->Y1,tLineP->Z2,tLineP->X2,tLineP->Y2,tLineP->Z2) ;
         }
       goto errexit  ;
      }
   }  
/*
** Remove Contiguos Visible Or Invisible Sections
*/
 if( numTempHorLines > 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Contiguous Sections") ;
    tLineOffset1P = tLineOffset2P = tempHorLinesP ;
    process = 1 ;
    while( tLineOffset2P < tempHorLinesP + numTempHorLines )
      { 
       while ( tLineOffset2P < tempHorLinesP + numTempHorLines && tLineOffset2P->ActiveFlag == tLineOffset1P->ActiveFlag ) ++tLineOffset2P ;
       --tLineOffset2P ;
       if( tLineOffset1P != tLineOffset2P )
         {
          tLineOffset1P->Ang2 = tLineOffset2P->Ang2 ; 
          tLineOffset1P->D2   = tLineOffset2P->D2   ; 
          tLineOffset1P->X2   = tLineOffset2P->X2   ; 
          tLineOffset1P->Y2   = tLineOffset2P->Y2   ; 
          tLineOffset1P->Z2   = tLineOffset2P->Z2   ; 
          for( tLineP = tLineOffset1P + 1 ; tLineP <= tLineOffset2P ; ++tLineP ) tLineP->ActiveFlag = -1 ;
         }
       ++tLineOffset2P ;
       tLineOffset1P = tLineOffset2P ;
      }  
    tLineOffset1P = tempHorLinesP ;
    for( tLineOffset2P = tempHorLinesP ; tLineOffset2P < tempHorLinesP + numTempHorLines ; ++tLineOffset2P )
      {
       if( tLineOffset2P->ActiveFlag != -1 )
         {
          if( tLineOffset1P != tLineOffset2P ) *tLineOffset1P = *tLineOffset2P ;
          ++tLineOffset1P ;
         }
      }
    numTempHorLines = (long)(tLineOffset1P-tempHorLinesP) ;
   }
/*
** Set Visibility Of Line
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Line Visibility") ;
 numVisible = numInvisible = 0 ;
 for( tLineP = tempHorLinesP ; tLineP < tempHorLinesP + numTempHorLines ; ++tLineP )
   {
    if( tLineP->ActiveFlag ) ++numVisible ;
    else                     ++numInvisible ; 
   } 
 if     ( ! numVisible &&   numInvisible ) *lineVisibilityP = -1 ;
 else if(   numVisible && ! numInvisible ) *lineVisibilityP =  1 ;
 else                                      *lineVisibilityP =  0 ; 
/*
** Check Length Of Line - Development Only
*/
 if( cdbg  )
   {
    if( bcdtmVisibility_checkLengthVisibilityLine(tempHorLinesP,numTempHorLines,L1) ||
        bcdtmVisibility_checkAngleSequenceVisibilityLine(tempHorLinesP,numTempHorLines,normalFlag,AngL) )
      {
       for( tLineP = tempHorLinesP ; tLineP < tempHorLinesP + numTempHorLines ; ++tLineP )
         {
          bcdtmWrite_message(0,0,0,"tempHorLinesP = %6ld %2ld ** %12.10lf %12.10lf ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(tLineP-tempHorLinesP),tLineP->ActiveFlag,tLineP->Ang1,tLineP->Ang2,tLineP->X1,tLineP->Y1,tLineP->Z2,tLineP->X2,tLineP->Y2,tLineP->Z2) ;
         }
       goto errexit  ;
      }
   }  
/*
** Copy Visibility To Los Data Structures
*/
 if( bcdtmVisibility_storeVertice(100,0,0.0,0.0,0.0) ) goto errexit  ; 
 for( tLineP = tempHorLinesP ; tLineP < tempHorLinesP + numTempHorLines ; ++tLineP )
   {
    if( bcdtmVisibility_storeVertice(1,tLineP->ActiveFlag,tLineP->X1,tLineP->Y1,tLineP->Z1) ) goto errexit  ;
    if( bcdtmVisibility_storeVertice(1,tLineP->ActiveFlag,tLineP->X2,tLineP->Y2,tLineP->Z2) ) goto errexit  ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( distHorLinesP != nullptr ) { free(distHorLinesP) ; distHorLinesP = nullptr ; }
 if( tempHorLinesP != nullptr ) { free(tempHorLinesP) ; tempHorLinesP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determing Line Visibility Using Visibility Tables Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determing Line Visibility Using Visibility Tables Error") ;
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
BENTLEYDTM_Private int bcdtmVisibility_storePointInHorizonTable(DTM_HORIZON_LINE **horLinesPP,long *numHorLinesP,long *memHorLinesP,double Ang1,double Ang2,double D1,double D2,double X1,double Y1,double Z1,double X2,double Y2,double Z2) 
/*
** This Function Store A Point In The Horizon Table
*/
{
 int   ret=DTM_SUCCESS ;
 long  minc=5000 ;
 DTM_HORIZON_LINE *hLineP ;
/*
** Check For Sufficient Memory
*/
 if( *numHorLinesP == *memHorLinesP )
   {
    *memHorLinesP = *memHorLinesP + minc ;
    if( *horLinesPP == nullptr )  *horLinesPP = (DTM_HORIZON_LINE *) malloc  ( *memHorLinesP * sizeof(DTM_HORIZON_LINE)) ;
    else                       *horLinesPP = (DTM_HORIZON_LINE *) realloc ( *horLinesPP, *memHorLinesP * sizeof(DTM_HORIZON_LINE)) ;  
    if( *horLinesPP == nullptr ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
   }
/*
** Store Entry In Table
*/
 hLineP = *horLinesPP + *numHorLinesP ;
 hLineP->ActiveFlag = 1 ;
 hLineP->Ang1 = Ang1 ;
 hLineP->Ang2 = Ang2 ;
 hLineP->D1   = D1   ;
 hLineP->D2   = D2   ;
 hLineP->X1   = X1   ;
 hLineP->Y1   = Y1   ;
 hLineP->Z1   = Z1   ;
 hLineP->X2   = X2   ;
 hLineP->Y2   = Y2   ;
 hLineP->Z2   = Z2   ;
 ++(*numHorLinesP) ;
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
BENTLEYDTM_Private int bcdtmVisibility_removeTotallyInvisibleHorizonLines
(
 DTM_HORIZON_LINE **horLinesPP,
 long             *numHorLinesP,
 double           Xe,
 double           Ye,
 double           Ze 
 )    
/*
** This Function Removes Totally Invisible Horizon Lines
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   removeFlag=0 ;
 double Z1,Z2,Ca,Cb,Cc,Cd ;
 DTM_HORIZON_LINE  *hLineP,*hLineLowP,*hLineHighP,*hzLineP ;
/*
** Initialise
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Horizon Lines Before Removal Of Invisible Lines = %8ld",*numHorLinesP) ;
/*
** Scan Horizon Line Structure
*/
 hLineHighP = *horLinesPP + *numHorLinesP - 1 ;
 for( hLineP = *horLinesPP ; hLineP <= hLineHighP ; ++hLineP )
   { 
    if( hLineP->ActiveFlag )
      {
       hLineLowP = hLineP ;
       while ( hLineLowP > *horLinesPP && hLineLowP->Ang1 == hLineP->Ang1 ) --hLineLowP ;
/*
**     Scan Horizon Line Structure For Covered Horizon Line
*/
       for( hzLineP = hLineLowP ; hzLineP <= hLineHighP && hzLineP->Ang1 < hLineP->Ang2 ; ++hzLineP )
         {   
          if( hzLineP->ActiveFlag )
            {
             if( hzLineP->Ang1 >= hLineP->Ang1 && hzLineP->Ang2 <= hLineP->Ang2 )              
               {
//                if( bcdtmVisibility_determineIfHorizonLineIsCovered(Xe,Ye,hLineP,hzLineP) )
                if( bcdtmVisibility_determineIfHorizonLineIsTotallyCovered(Xe,Ye,hLineP,hzLineP) )
                  {
                   if( Ze == hLineP->Z1 && Ze == hLineP->Z2 && Ze == hzLineP->Z1 && Ze == hzLineP->Z2) Z1 = Z2 = Ze ;
                   else
                     {
                      if( bcdtmMath_calculatePlaneCoefficients(Xe,Ye,Ze,hLineP->X1,hLineP->Y1,hLineP->Z1,hLineP->X2,hLineP->Y2,hLineP->Z2,&Ca,&Cb,&Cc,&Cd) ) goto errexit ;
                      if( Cc == 0.0 ) Cc = 0.000000001  ;  
                      Z1 = - ( Ca * hzLineP->X1 + Cb * hzLineP->Y1 + Cd ) / Cc ;
                      Z2 = - ( Ca * hzLineP->X2 + Cb * hzLineP->Y2 + Cd ) / Cc ;
                     }
                   if( Z1 > hzLineP->Z1             && Z2 > hzLineP->Z2 ||
                       fabs(Z1-hzLineP->Z1) < 0.001 && Z2 > hzLineP->Z2 ||  
                       fabs(Z2-hzLineP->Z2) < 0.001 && Z1 > hzLineP->Z1    )  
                     { 
                      hzLineP->ActiveFlag = 0 ;
                      removeFlag = 1 ;
                     }
                  }
               }
            }
         }
      }
   }   
/*
** Remove Invisible Lines
*/
 if( removeFlag )
   {
    hLineLowP = *horLinesPP ;
    for( hLineHighP = *horLinesPP ; hLineHighP < *horLinesPP + *numHorLinesP ; ++hLineHighP )
      {
       if( hLineHighP->ActiveFlag )
         {
          if( hLineLowP != hLineHighP ) *hLineLowP = *hLineHighP ;
          ++hLineLowP ;
         }
      }
    *numHorLinesP = (long)(hLineLowP-*horLinesPP) ;
    *horLinesPP = ( DTM_HORIZON_LINE * ) realloc( *horLinesPP,*numHorLinesP*sizeof(DTM_HORIZON_LINE)) ;
   }
/*
** Write Stats
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Horizon Lines After  Removal Of Invisible Lines = %8ld",*numHorLinesP) ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_determineRadialVisibilityDtmObject
(
 BC_DTM_OBJ *dtmP,double Xe,double Ye,double Ze,double Xp,double Yp,double Zp, DTMFeatureCallback loadFunctionP, void *userP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   *lP,scan,process,drapeFlag,numDrapePts=0 ;
 long   numLoadPts=0,memLoadPts=0,memLoadPtsInc=1000 ;
 double x,y,z,dx,dy,dz,dd,Zs,maxangle,eyeangle = 0.0,lasteyeangle = 0.0;
 DPoint3d    *p3dP,radialPts[2],*loadPtsP=nullptr ;
 DTM_DRAPE_POINT *drapeP,*drape1P,*drape2P,*drapePtsP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Determining Radial Visibility") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"Xe             = %12.5lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Ye             = %12.5lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Ze             = %12.5lf",Ze) ;
    bcdtmWrite_message(0,0,0,"Xp             = %12.5lf",Xp) ;
    bcdtmWrite_message(0,0,0,"Yp             = %12.5lf",Yp) ;
    bcdtmWrite_message(0,0,0,"Zp             = %12.5lf",Zp) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP  = %12.5lf",Zp) ;
    bcdtmWrite_message(0,0,0,"userP          = %12.5lf",Zp) ;
   } 
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check For Triangulated DTM Object
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated Dtm") ;
    goto errexit ;
   }
/*
** Determine If Eye Is Within Tin Hull
*/
 if( bcdtmDrape_pointDtmObject(dtmP,Xe,Ye,&Zs,&drapeFlag)) goto errexit  ;
 if( ! drapeFlag ) 
   { 
    bcdtmWrite_message(1,0,0,"Eye External To Tin or Internal To Void") ;
    goto errexit  ;
   }
/*
** Check If Eye Is Below Surface
*/
 if( drapeFlag && Zs > Ze ) 
   {  
    bcdtmWrite_message(1,0,0,"Eye Below Tin Surface") ;
    goto errexit  ;
   }
/*
** Determine If Target Point Is Within Tin Hull
*/
 if( bcdtmDrape_pointDtmObject(dtmP,Xp,Yp,&Zs,&drapeFlag)) goto errexit  ;
 if( ! drapeFlag ) 
   { 
    bcdtmWrite_message(1,0,0,"Target Point External To Tin or Internal To Void") ;
    goto errexit  ;
   }
/*
** Check If Target Point Is Above Below Surface
*/
 if( drapeFlag && Zs > Zp ) 
   {
    bcdtmWrite_message(1,0,0,"Target Point Below Tin Surface") ;
    goto errexit  ;
   }
/*
** Set radialPts Coordinates
*/
 radialPts[0].x = Xe ; radialPts[0].y = Ye ; radialPts[0].z = 0.0 ;
 radialPts[1].x = Xp ; radialPts[1].y = Yp ; radialPts[1].z = 0.0 ;
/*
**  Drape Radial On Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Draping Radial On Tin") ;
 if( bcdtmDrape_stringDtmObject(dtmP,radialPts,2,FALSE,&drapePtsP,&numDrapePts)) goto errexit  ;
/*
**  Remove Drape End Points Not On Tin
*/
 drapeP  = drapePtsP + numDrapePts - 1 ;
 while (drapeP->drapeType == DTMDrapedLineCode::External) { --drapeP; --numDrapePts; }
/*
**  Remove Duplicate Drape Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Drape Points") ;
 for( drape1P = drape2P = drapePtsP ; drape2P < drapePtsP + numDrapePts ; ++drape2P )
   {
    if( drape2P->drapeX != drape1P->drapeX || drape2P->drapeY != drape1P->drapeY )
      {
       if( drape1P != drape2P ) *drape1P = *drape2P ;
       ++drape1P ;
      }
   } 
 numDrapePts = (long)(drape1P-drapePtsP) ;
/*
**  Determine Visibility Of Draped Radial Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determine Visibility Of Draped Radial Points ** numDrapePts = %8ld", numDrapePts) ;
 if( bcdtmVisibility_storeVertice(100,0,0.0,0.0,0.0) ) goto errexit  ;
 if( bcdtmVisibility_storeVertice(1,1,drapePtsP->drapeX,drapePtsP->drapeY,drapePtsP->drapeZ) ) goto errexit  ;
 dx = (drapePtsP+1)->drapeX - Xe ;
 dy = (drapePtsP+1)->drapeY - Ye ;
 dz = (drapePtsP+1)->drapeZ - Ze ;
 dd = sqrt(dx*dx+dy*dy) ;
 maxangle = atan2(dz,dd) ;
 drape1P = drapePtsP + 2 ;
 drape2P = drapePtsP + numDrapePts - 1 ;
 scan = 1 ;
 while ( scan )
   { 
/*
**  Scan Visible Drape Points
*/
    if( drape1P <= drape2P ) 
      {
       process = 1 ; 
       while ( drape1P <= drape2P && process )
         {
          dx = drape1P->drapeX - Xe ;
          dy = drape1P->drapeY - Ye ;
          dz = drape1P->drapeZ - Ze ;
          dd = sqrt(dx*dx+dy*dy) ;
          eyeangle = atan2(dz,dd) ;
          if( eyeangle >= maxangle ) { maxangle = eyeangle ; ++drape1P ; }
          else                        process = 0 ; 
         }  
       --drape1P ;
       if( bcdtmVisibility_storeVertice(1,1,drape1P->drapeX,drape1P->drapeY,drape1P->drapeZ) ) goto errexit  ;
       if( drape1P < drape2P ) if( bcdtmVisibility_storeVertice(1,0,drape1P->drapeX,drape1P->drapeY,drape1P->drapeZ) ) goto errexit  ;
       ++drape1P ; 
      } 
/*
**  Scan Invisible Drape Points
*/
    if( drape1P <= drape2P )  
      {
       process = 1 ; 
       while ( drape1P <= drape2P && process )
         {
          dx = drape1P->drapeX - Xe ;
          dy = drape1P->drapeY - Ye ;
          dz = drape1P->drapeZ - Ze ;
          dd = sqrt(dx*dx+dy*dy) ;
          eyeangle = atan2(dz,dd) ;
          if( eyeangle < maxangle ) { lasteyeangle = eyeangle ; ++drape1P ; }
          else                        process = 0 ; 
         }
/*
**     Calculate Max Angle Intercept On radialPts
*/
       if( process )
         {
          --drape1P ;
          if( bcdtmVisibility_storeVertice(1,0,drape1P->drapeX,drape1P->drapeY,drape1P->drapeZ) ) goto errexit  ;
          ++drape1P ;
         }
       else
         { 
          dx = maxangle - lasteyeangle ;
          dd = eyeangle - lasteyeangle ;
          x =  (drape1P-1)->drapeX + (drape1P->drapeX - (drape1P-1)->drapeX) * dx / dd ;
          y =  (drape1P-1)->drapeY + (drape1P->drapeY - (drape1P-1)->drapeY) * dx / dd ;
          z =  (drape1P-1)->drapeZ + (drape1P->drapeZ - (drape1P-1)->drapeZ) * dx / dd ;
          if( bcdtmVisibility_storeVertice(1,0,x,y,z) ) goto errexit  ;
          if( bcdtmVisibility_storeVertice(1,1,x,y,z) ) goto errexit  ;
         } 
      }
/*
**  Test For End Of Scan
*/
    if( drape1P > drape2P ) scan = 0 ;
   } 
/*
** Load Visibility Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Visibility Lines") ;
 for( p3dP = losPtsP , lP = losLinesP ; p3dP < losPtsP + numLosPts ; p3dP = p3dP + 2 , lP = lP + 2  )
   {
    if( bcdtmLoad_storeFeaturePoint(p3dP->x,p3dP->y,p3dP->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
    if( bcdtmLoad_storeFeaturePoint((p3dP+1)->x,(p3dP+1)->y,(p3dP+1)->z,&loadPtsP,&numLoadPts,&memLoadPts,memLoadPtsInc)) goto errexit ;
    if( *lP ) { if( loadFunctionP(DTMFeatureType::VisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,loadPtsP,numLoadPts,userP)) goto errexit ; }
    else      { if( loadFunctionP(DTMFeatureType::InvisibleLine,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,loadPtsP,numLoadPts,userP)) goto errexit ; }
    numLoadPts = 0 ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( loadPtsP != nullptr ) { free(loadPtsP) ; loadPtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Radial Visibility Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Radial Visibility Error") ;
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
BENTLEYDTM_EXPORT int bcdtmVisibility_createVisibilityLatticeDtmObject
(
 BC_DTM_OBJ *dtmP,                     // ==> Pointer To DTM Object
 DTM_LAT_OBJ **latticePP,              // <== Created View Shed Lattice
 long   numLatticePts,                 // ==> Number Of Lattice Points
 double Xe,                            // ==> x Eye Coordinate 
 double Ye,                            // ==> y Eye Coordinate
 double Ze,                            // ==> z Eye Coordinate
 double zOffset                        // ==> z offset Value To Apply To Lattice Elevation Values
)
/*
** This Function Creates A Visibility Lattice 
*/
{
 int         ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long        i,j,isVisible,drapeFlag,index,startTime,totalTime=bcdtmClock() ;
 long        numVisiblePoints=0,numInvisiblePoints=0,numNullPoints=0 ;
 double      x,y,z,Zs  ;
 DTM_LAT_OBJ *latticeP ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Visibility Lattice") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"*latticePP    = %p",*latticePP) ;
    bcdtmWrite_message(0,0,0,"numLatticePts = %8ld",numLatticePts) ;
    bcdtmWrite_message(0,0,0,"Eye x         = %10.4lf",Xe) ;
    bcdtmWrite_message(0,0,0,"Eye y         = %10.4lf",Ye) ;
    bcdtmWrite_message(0,0,0,"Eye z         = %10.4lf",Ze) ;
    bcdtmWrite_message(0,0,0,"z Offset      = %10.4lf",zOffset) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
  if( dtmP->dtmState != DTMState::Tin )
    {
     bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
     goto errexit ;
    }
/*
** Check Lattice Pointer Id Null
*/
 if( *latticePP != nullptr )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Null Lattice Pointer") ;
    goto errexit ;
   }    
/*
** Determine If Eye Is Inside Tin Hull
*/
 if( bcdtmDrape_pointDtmObject(dtmP,Xe,Ye,&Zs,&drapeFlag)) goto errexit ;
 if( drapeFlag == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Eye External To Tin Or Internal To Void") ;
    goto errexit ;
   }
 if( drapeFlag  && Zs > Ze )  
   { 
    bcdtmWrite_message(1,0,0,"Eye Below Tin Surface") ; 
    goto errexit ;
   }
/*
** Create Lattice
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating %8ld Point Lattice",numLatticePts) ;
 if( bcdtmLattice_createLatticeFromDtmObject(dtmP,latticePP,0,1,numLatticePts,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Create %8ld Point Lattice = %7.3lf seconds",numLatticePts,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Log Number Of Null Lattice Points
*/
 if( dbg )
   {
    numNullPoints = 0 ;
    latticeP = *latticePP ;
    for( i = 0 ; i < latticeP->NXL ; ++i )
      {
       for( j = 0 ; j < latticeP->NYL ; ++j )
         {
          index = i * latticeP->NYL + j ;
          if( ( z = *(latticeP->LAT+index) ) == latticeP->NULLVAL ) ++numNullPoints ;
         }
      } 
    bcdtmWrite_message(0,0,0,"Number Of Null Lattice Points = %8ld",numNullPoints) ;     
    numNullPoints = 0 ;
   } 
/*
** Build Visibility Tables For Dtm 
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Visibility Tables") ;
 if( bcdtmVisibility_buildVisibilityTablesForDtmObject(dtmP,Xe,Ye,Ze)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Build Visibility Tables = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Scan Lattice Points And Determine Visibility
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Visibility Of Lattice Points") ;
 latticeP = *latticePP ;
 for( i = 0 ; i < latticeP->NXL ; ++i )
   {
    for( j = 0 ; j < latticeP->NYL ; ++j )
      {
       index = i * latticeP->NYL + j ;
       if( ( z = *(latticeP->LAT+index) ) == latticeP->NULLVAL ) ++numNullPoints ;
       else
         {
          x = latticeP->LXMIN + j * latticeP->DX ;
          y = latticeP->LYMIN + i * latticeP->DY ;
          z = z + zOffset ;
          if( bcdtmVisibility_determinePointVisibilityUsingVisibilityTables(horLinesP,numHorLines,horLinesIndexP,numHorLinesIndex,hozIndexListP,Xe,Ye,Ze,x,y,z,&isVisible)) goto errexit ;
          if( isVisible ) 
            {
             ++numVisiblePoints ;
             *(latticeP->LAT+index) = 1.0 ;
            } 
          else
            {
             ++numInvisiblePoints ;
             *(latticeP->LAT+index) = 0.0 ; 
            } 
         }
      }
   }
/*
** Set Lattice Min and Max z Values
*/
 latticeP->LZMIN = 0.0 ;   
 latticeP->LZMAX = 1.0 ;   
/*
** Write Timing Information
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Time To Determine Visibility Of Lattice Points = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    bcdtmWrite_message(0,0,0,"Number Of Visible Points   = %8ld",numVisiblePoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Invisible Points = %8ld",numInvisiblePoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Null Points      = %8ld",numNullPoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Lattice Points   = %8ld",latticeP->NXL*latticeP->NYL) ;
    bcdtmWrite_message(0,0,0,"Total Time To Create Visibility Lattice = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),totalTime)) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job ComlosLinePeted
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Visibility Lattice Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Visibility Lattice Error") ;
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
BENTLEYDTM_Public int bcdtmLos_horizonPointsAngleCompareFunction(const void *Cp1,const void *Cp2)
/*
** Compare Function For Qsort Of Horizon Points
*/
{
 DTM_HORIZON_LINE *Hpts1,*Hpts2 ;
 Hpts1 = ( DTM_HORIZON_LINE * ) Cp1 ;
 Hpts2 = ( DTM_HORIZON_LINE * ) Cp2 ;
 if     ( Hpts1->Ang1 < Hpts2->Ang1 ) return(-1) ;
 else if( Hpts1->Ang1 > Hpts2->Ang1 ) return( 1) ;
 else if( Hpts1->Ang2 > Hpts2->Ang2 ) return(-1) ;
 else if( Hpts1->Ang2 < Hpts2->Ang2 ) return( 1) ;
 else if( Hpts1->D1   < Hpts2->D1   ) return(-1) ;
 else if( Hpts1->D1   > Hpts2->D1   ) return( 1) ;
 else if( Hpts1->D2   < Hpts2->D2   ) return(-1) ;
 else if( Hpts1->D2   > Hpts2->D2   ) return( 1) ;
 return(0) ;
}
