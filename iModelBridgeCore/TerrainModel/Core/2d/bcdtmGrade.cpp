/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmGrade.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
BENTLEYDTM_EXPORT int bcdtmGrade_getGradeSlopeStartDirectionsDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double Slope,double *Zs,long *Tp1,long *Tp2,long *Tp3,double *TriangleSlope,double StartDirections[],long *NumStartDirections)
/*
** This Function Gets The Starting Directions For A Graded Slope Trace On The Tin Surface
**
** Tin                      ==> Dtm Object
** x                        ==> Start Point x Coordinate
** y                        ==> Start Point y Coordinate
** Slope                    ==> Slope ( Vertical / Horizontal ) For Determing Grade Slope Start Directions
** Zs                      <==  Surface Elevation At Point x,y
** Tp1                     <==  Tin Point Number For Triangle Vertex 
** Tp2                     <==  Tin Point Number For Triangle Vertex 
** Tp3                     <==  Tin Point Number For Triangle Vertex 
** TriangleSlope           <==  Slope Of Triangle  Tp1Tp2Tp3
** StartDirection[2]       <==  The Two Angle Directions That The Grade Slope Can Trace From The Point XY 
** NumberOfStartDirections <==  Number Of Values Stored In StartDirection[2]
**                         <==  Will Be Either 0 or 2. If Zero ,Then A Grade Slope Cannot Be Started 
**                              From The Triangle As The fabs(Slope) > TriangleSlope  
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ns,P1,P2,P3,Ptype,savePnt,VoidFlag ;
 double z ;
/*
** Write Arguements For Development Purposes
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Calculating Grade Slope Start Directions") ;
    bcdtmWrite_message(0,0,0,"Dtm Object      = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"x               = %15.10lf",x)  ;
    bcdtmWrite_message(0,0,0,"y               = %15.10lf",y)  ;
    bcdtmWrite_message(0,0,0,"Slope           = %10.4lf",Slope) ;
   }
/*
** Initialise  
*/
 *Zs = 0 ;
 *NumStartDirections = 0 ;
 *TriangleSlope      = 0.0 ;
 P1 = P2 = P3 = dtmP->nullPnt ;
/*
** Test For Valid DTMFeatureState::Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ; ;
/*
** Check DTM Is Triangulated
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Find Triangle Containing Start Point On DTMFeatureState::Tin
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&z,&Ptype,&P1,&P2,&P3)) goto errexit ; ;
 if( Ptype == 0 ) { bcdtmWrite_message(1,0,0,"Start Point Not On Tin") ; goto errexit ; ; }
/*
** Check Point To Point Tolerance
*/
 if ( bcdtmMath_distance(x,y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) <= dtmP->ppTol ) { Ptype = 1 ; P2 = P3 = dtmP->nullPnt ; }
 if( P2 != dtmP->nullPnt ) if( bcdtmMath_distance(x,y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) <= dtmP->ppTol ) { Ptype = 1 ; P1 = P2 ; P2 = P3 = dtmP->nullPnt ; }
 if( P3 != dtmP->nullPnt ) if( bcdtmMath_distance(x,y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y) <= dtmP->ppTol ) { Ptype = 1 ; P1 = P3 ; P2 = P3 = dtmP->nullPnt ; }
/*
** Test For Point In Void
*/
 VoidFlag = 0 ;
 if( Ptype == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P1)->PCWD) ) VoidFlag = 1 ;
 if( Ptype == 2 || Ptype == 3 )  if( bcdtmList_testForVoidLineDtmObject(dtmP,P1,P2,&VoidFlag)) goto errexit ; ;
 if( Ptype == 4 ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,P2,P3,&VoidFlag)) goto errexit ; ;
 if( VoidFlag ) { bcdtmWrite_message(1,0,0,"Start Point In Void") ; goto cleanup ; ; }
/*
** Set Triangle Anti Clockwise
*/
 if      ( P2 != dtmP->nullPnt && P3 != dtmP->nullPnt )
   {
    if( bcdtmMath_pointSideOfDtmObject(dtmP,P1,P2,P3) < 0 ) { savePnt = P2 ; P2 = P3 ; P3 = savePnt ; }
   }
 else if ( P2 != dtmP->nullPnt && P3 == dtmP->nullPnt )
   {
    if(( P3 = bcdtmList_nextAntDtmObject(dtmP,P1,P2)) < 0 ) goto errexit ; ;
    if( ! bcdtmList_testLineDtmObject(dtmP,P3,P2))
      {
       if(( P3 = bcdtmList_nextClkDtmObject(dtmP,P1,P2)) < 0 ) goto errexit ; ;
       if( ! bcdtmList_testLineDtmObject(dtmP,Ptype,P2) ) goto errexit ; ;
       savePnt = P1 ; P1 = P2 ; P2 = savePnt ; 
      }
   } 
/*
** Get Start Dirtection For Triangle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Start Directions") ;
 if( bcdtmGrade_calculateGradeSlopeStartDirectionsForTriangleDtmObject(dtmP,x,y,z,Slope,Ptype,P1,P2,P3,TriangleSlope,StartDirections,NumStartDirections) ) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Start Directions = %2ld",*NumStartDirections) ;
    for( ns = 0 ; ns < *NumStartDirections ; ++ns )
      {
       bcdtmWrite_message(0,0,0,"Start Direction = %10.8lf radians",StartDirections[ns]) ;
      } 
   } 
/*
** Set Surface Elevation And Triangle Point Numbers
*/
 *Zs  = z  ;
 *Tp1 = P1 ;
 *Tp2 = P2 ;
 *Tp3 = P3 ; 
/*
** Clean Up
*/
 cleanup :
 if( ! ret && dbg ) bcdtmWrite_message(0,0,0,"Calculating Grade Slope Start Directions Completed") ;
 if(   ret && dbg ) bcdtmWrite_message(0,0,0,"Calculating Grade Slope Start Directions Error") ;
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
BENTLEYDTM_Public int bcdtmGrade_calculateGradeSlopeStartDirectionsForTriangleDtmObject
(
 BC_DTM_OBJ *dtmP,
 double     Sx,
 double     Sy,
 double     Sz,
 double     Slope,
 long       Ptype,
 long       P1,
 long       P2,
 long       P3,
 double     *TriangleSlopeP,
 double     StartDirections[],
 long       *NumStartDirectionsP
) 
/*
** Calculates The Start Directions For A Grade Line Within A Triangle
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,clc,loop,process,gradeFound,gradeSlopeAngleFound ;
 double dz,Xr,Yr,Zr,Zs,Zg;
 double A,B,C,D ;
 double radius,angle,anginc,gradeAngle=0.0,gradeSlope,gradeSlopeAngleOne,gradeSlopeAngleTwo ;
 double anglemin=0.0,anglemax=0.0 ;
 double descentAngle,ascentAngle ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Calculating Grade Slope Start Directions For Triangle") ;
    bcdtmWrite_message(0,0,0,"dtmP     = %p",dtmP) ; 
    bcdtmWrite_message(0,0,0,"Sx       = %12.5lf",Sx) ; 
    bcdtmWrite_message(0,0,0,"Sy       = %12.5lf",Sy) ; 
    bcdtmWrite_message(0,0,0,"Sz       = %12.5lf",Sz) ; 
    bcdtmWrite_message(0,0,0,"Slope    = %8.4lf",Slope) ; 
    bcdtmWrite_message(0,0,0,"Ptype    = %1ld",Ptype) ;  
    bcdtmWrite_message(0,0,0,"P1       = %12ld",P1) ; 
    bcdtmWrite_message(0,0,0,"P2       = %12ld",P2) ; 
    bcdtmWrite_message(0,0,0,"P3       = %12ld",P3) ; 
   }
/*
** Initialise
*/
 *TriangleSlopeP = 0.0 ;
 *NumStartDirectionsP = 0 ;
/*
** Check For Solution From Start Point In Triangle
*/
 if( Ptype == 4 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Start Point In Triangle") ;
    if( bcdtmMath_getTriangleDescentAndAscentAnglesDtmObject(dtmP,P1,P2,P3,&descentAngle,&ascentAngle,TriangleSlopeP)) goto errexit ;
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Triangle Slope = %12.6lf",*TriangleSlopeP) ;
       bcdtmWrite_message(0,0,0,"Descent  Angle = %12.10lf",descentAngle) ;
       bcdtmWrite_message(0,0,0,"Ascent   Angle = %12.10lf",ascentAngle) ;
      }
    if( fabs(Slope) > *TriangleSlopeP ) 
      { 
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"No Solution For Grade Slope Start Point") ; 
          bcdtmWrite_message(0,0,0,"Slope %10.4lf > Triangle Slope %10.4lf",fabs(Slope),*TriangleSlopeP) ; 
         }
       goto cleanup  ; 
      }
/*
** Calculate Plane Coefficients For Triangle
*/
    bcdtmMath_calculatePlaneCoefficients(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z,&A,&B,&C,&D) ;
/*
** Scan About Point Looking For Direction Of Slope Grade
*/
    process = 2 ;
    while ( process )
      {
       if( process == 2 ) { anglemin = descentAngle ; anglemax = ascentAngle  ; }
       if( process == 1 ) { anglemin = ascentAngle  ; anglemax = descentAngle ; }
       --process ;
       if( anglemax < anglemin ) anglemax += DTM_2PYE ;
       dz     = 1000.0  ;
       radius = 1.0     ;
       loop   = 0       ; 
       anginc = ( anglemax - anglemin ) / 5.0  ;
       Zg = Zs = Sz + radius * Slope ;
       while ( dz > 0.00001 )
         {
          angle = anglemin ;
          while ( angle <= anglemax )
            {
             ++loop ;
             Xr = Sx + radius * cos(angle) ;  
             Yr = Sy + radius * sin(angle) ;  
             bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A,B,C,D) ;
             if( fabs(Zs-Zr) < dz || angle == anglemin )
               {
                Zg = Zr ;
                dz = fabs(Zs-Zr) ;
                gradeAngle = angle ; 
               }
             angle = angle + anginc ;
            }
          anglemin = gradeAngle - anginc ;
          anglemax = gradeAngle + anginc ;
          anginc = ( anglemax - anglemin ) / 5.0  ;
         } 
       while ( gradeAngle > DTM_2PYE ) gradeAngle = gradeAngle - DTM_2PYE ;
       while ( gradeAngle < 0.0  ) gradeAngle = gradeAngle + DTM_2PYE ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Grade Angle = %12.10lf Grade Slope = %10.4lf",gradeAngle,(Zg-Sz)/radius) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Interations = %4ld",loop) ;
       StartDirections[process] = gradeAngle ;
       ++*NumStartDirectionsP ;
      }
   }
/*
** Check For Solution From Start At Tin Point
*/
 if( Ptype == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Start Point From Tin Point") ;
    gradeSlope = Slope ;
/*
** Scan Triangle Point For Grade Slope
*/
    p1 = P1 ;
    if( ( clc = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr ) 
      {
       if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
       while ( clc != dtmP->nullPtr  /* && ! gradeFound */ )
         {
          p3  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
/*
**        Calculate Grade Slope Angles For Triangle
*/
          if( bcdtmGrade_calculateGradeSlopeAnglesForTriangleDtmObject(dtmP,p1,p2,p3,gradeSlope,&gradeSlopeAngleFound,&gradeSlopeAngleOne,&gradeSlopeAngleTwo) ) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"gradeSlopeAngleFound = %2ld gradeSlopeAngleOne = %12.10lf gradeSlopeAngleTwo = %12.10lf",gradeSlopeAngleFound,gradeSlopeAngleOne,gradeSlopeAngleTwo) ;
/*
**        If Grade Slope Angles Found, Test If Grade Slope Angles Are Internal To Triangle
*/
          if( gradeSlopeAngleFound )
            {
             gradeFound = 0 ;
             anglemin = bcdtmMath_getPointAngleDtmObject(dtmP,p1,p3) ;   
             anglemax = bcdtmMath_getPointAngleDtmObject(dtmP,p1,p2) ;
             if( anglemax < anglemin ) anglemax += DTM_2PYE ;
             if( dbg ) bcdtmWrite_message(0,0,0,"anglemin = %12.8lf anglemax = %12.8lf",anglemin,anglemax) ;
             if( gradeSlopeAngleOne < anglemin ) gradeSlopeAngleOne += DTM_2PYE ;
             if( gradeSlopeAngleTwo < anglemin ) gradeSlopeAngleTwo += DTM_2PYE ;
             if      ( gradeSlopeAngleOne <= anglemax ) { gradeFound = 1 ; gradeAngle = gradeSlopeAngleOne ; }
             else if (  gradeSlopeAngleFound == 2 && gradeSlopeAngleTwo <= anglemax ) { gradeFound = 1 ; gradeAngle = gradeSlopeAngleTwo ; }
             if( dbg && gradeFound ) bcdtmWrite_message(0,0,0,"Grade Slope Found") ;
             if( gradeFound &&  *NumStartDirectionsP < 4 )
               {
                StartDirections[*NumStartDirectionsP] = gradeAngle ;
                ++*NumStartDirectionsP ;
               }
            }   
/*
**        Reset For Next Triangle
*/
          p2 = p3 ;
         }
      }
   }
/*
** Cleanup
*/
 cleanup   :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Grade Slope Start Directions For Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Grade Slope Start Directions For Triangle Error") ;
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
BENTLEYDTM_EXPORT int bcdtmGrade_traceGradeDtmObject
(
 BC_DTM_OBJ *dtmP,
 double     Sx,
 double     Sy,
 double     Sz,
 DTMFeatureCallback callBackFunctionP,          /* ==> Call Back Function For Tracing Grade Slope  */
 long       P1,
 long       P2,
 long       P3,
 double     GradeSlope,
 double     GradeAngle,
 double     Distance,
 void       *userP
)
/*
** This Function Traces A Constant Grade Slope Across The DTMFeatureState::Tin Surface
** Starting From Point XY In Traingle P1,P2,P3 ;
**
** dtmP               ==>  Dtm Object
** Sx                 ==>  Start Point x Coordinate
** Sy                 ==>  Start Point y Coordinate
** Sx                 ==>  Start Point z Coordinate
** callBackFunctionP  ==>  Draw The Grade Slope Into MicroStation
** P1                 ==>  Tin Point Number For Triangle Vertex 
** P2                 ==>  Tin Point Number For Triangle Vertex 
** P3                 ==>  Tin Point Number For Triangle Vertex 
** GradeSlope         ==>  Grade Slope Value ( Vertical / Horizontal )
** GradeAngle         ==>  Initial Direction Of Grade Slope
** Distance           ==>  Distance To Trace Along The Grade Slope
**                    ==>  If Distance <= 0.0 Then The Trace Will Continue Until It Stops
** userP              ==>  Pointer Passed Back To User
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   Np1,Np2,Np3,TracePointFound ;
 double Nx,Ny,Nz,Lx,Ly,Lz,PointDistance,TraceDistance,LastAngle ;
/*
** Write Status Information ** Developement Only
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tracing Grade") ;
    bcdtmWrite_message(0,0,0,"Sx = %12.4lf",Sx) ;
    bcdtmWrite_message(0,0,0,"Sy = %12.4lf",Sy) ;
    bcdtmWrite_message(0,0,0,"Sz = %12.4lf",Sz) ;
    if( P1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
    else                    bcdtmWrite_message(0,0,0,"P1 = %9ld",P1) ; 
    if( P2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
    else                    bcdtmWrite_message(0,0,0,"P2 = %9ld",P2) ; 
    if( P3 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"P3 = %6ld ** %10.4lf %10.4lf %10.4lf",P3,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z) ;
    else                    bcdtmWrite_message(0,0,0,"P3 = %9ld",P3) ; 
    bcdtmWrite_message(0,0,0,"Grade Slope = %12.10lf",GradeSlope) ;
    bcdtmWrite_message(0,0,0,"Grade Angle = %12.10lf",GradeAngle) ;
    bcdtmWrite_message(0,0,0,"Distance    = %12.4lf",Distance) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ; ;
/*
** Check DTM Is Triangulated
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Initialise
*/
 PointDistance = TraceDistance = 0.0 ;
 Lx = Sx ; Ly = Sy ; Lz = Sz ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Start Point = %10.4lf %10.4lf %10.4lf",Sx,Sy,Sz) ;
 if( callBackFunctionP != NULL ) { if( bcdtmLoad_storePointInCache(Sx,Sy,Sz)) goto errexit ; }
/*
** Trace To First Triangle Edge From Inside Triangle
*/
 if( bcdtmGrade_getFirstGradeTracePointDtmObject(dtmP,Sx,Sy,Sz,P1,P2,P3,GradeAngle,&TracePointFound,&Np1,&Np2,&Np3,&Nx,&Ny,&Nz) ) goto errexit ;
 if( dbg )
   {
    if( TracePointFound ) bcdtmWrite_message(0,0,0,"First Trace Point Found") ;
    else                  bcdtmWrite_message(0,0,0,"First Trace Point Not Found") ;
   }
/*
** Only Process Trace If First Trace Point Found
*/
 if(  TracePointFound ) 
   {
    PointDistance = bcdtmMath_distance(Lx,Ly,Nx,Ny) ;
    if( Distance <= 0.0 || TraceDistance + PointDistance < Distance  )
      {
       Sx = Nx  ; Sy = Ny  ; Sz = Nz  ;
       P1 = Np1 ; P2 = Np2 ; P3 = Np3 ;
       TraceDistance = TraceDistance + PointDistance ;
      }
    else
      {
       Sx = Lx + (Nx-Lx) * (Distance-TraceDistance) / PointDistance ;
       Sy = Ly + (Ny-Ly) * (Distance-TraceDistance) / PointDistance ;
       Sz = Lz + (Nz-Lz) * (Distance-TraceDistance) / PointDistance ;
       TracePointFound = 0 ;
       if( dbg ) TraceDistance = TraceDistance + bcdtmMath_distance(Lx,Ly,Sx,Sy) ;
      }    
    if( callBackFunctionP != NULL ) { if( bcdtmLoad_storePointInCache(Sx,Sy,Sz)) goto errexit ; }
/*
**  Trace Across Succesive Triangles
*/
    while ( TracePointFound )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Next  Point = %10.4lf %10.4lf %10.4lf P1 = %6ld P2 = %9ld P3 = %9ld ** Distance = %10.4lf Angle = %10.8lf TraceDistance = %10.4lf Slope = %10.4lf",Sx,Sy,Sz,P1,P2,P3,PointDistance,bcdtmMath_getAngle(Lx,Ly,Sx,Sy),TraceDistance,(Sz-Lz)/PointDistance)  ;
       if( Lx != Sx || Ly != Sy )  LastAngle = bcdtmMath_getAngle(Lx,Ly,Sx,Sy) ;
       else                        LastAngle = GradeAngle ;
       Lx = Sx ; Ly = Sy ; Lz = Sz ;
       if( P2 == dtmP->nullPnt ) { if( bcdtmGrade_traceGradedSlopeFromTrianglePointDtmObject(dtmP,P1,Sx,Sy,GradeSlope,&TracePointFound,&Np1,&Np2,&Np3,&Nx,&Ny,&Nz)) goto  errexit ; }
       else                      { if( bcdtmGrade_traceGradedSlopeFromTriangleEdgeDtmObject(dtmP,P1,P2,P3,Sx,Sy,Sz,GradeSlope,LastAngle,&TracePointFound,&Np1,&Np2,&Np3,&Nx,&Ny,&Nz)) goto errexit ; }
       if( TracePointFound )
         {
          PointDistance = bcdtmMath_distance(Lx,Ly,Nx,Ny) ;
          if( Distance <= 0.0 || TraceDistance + PointDistance < Distance  )
            {
             Sx = Nx  ; Sy = Ny  ; Sz = Nz  ;
             P1 = Np1 ; P2 = Np2 ; P3 = Np3 ;
             TraceDistance = TraceDistance + PointDistance ;
            }
          else
            {
             Sx = Lx + (Nx-Lx) * (Distance-TraceDistance) / PointDistance ;
             Sy = Ly + (Ny-Ly) * (Distance-TraceDistance) / PointDistance ;
             Sz = Lz + (Nz-Lz) * (Distance-TraceDistance) / PointDistance ;
             TracePointFound = 0 ;
             if( dbg ) TraceDistance = TraceDistance + bcdtmMath_distance(Lx,Ly,Sx,Sy) ;
            }    
          if( callBackFunctionP != NULL ) { if( bcdtmLoad_storePointInCache(Sx,Sy,Sz)) goto errexit ; }
         }  
      }
/*
**  Draw Remaining Grade Slope Trace
*/
    if( callBackFunctionP != NULL ) 
      {
       if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(callBackFunctionP,DTMFeatureType::GradeSlope,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,userP) ) goto errexit ; 
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Trace Distance = %10.4lf",TraceDistance) ; 
   }
/*
** Clean Up
*/
 cleanup :
 bcdtmLoad_clearCache()  ; 
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Tracing Grade Completed") ;
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
BENTLEYDTM_Public int bcdtmGrade_getFirstGradeTracePointDtmObject(BC_DTM_OBJ *dtmP,double Sx,double Sy,double Sz,long P1,long P2,long P3,double GradeAngle,long *TracePointFound,long *Np1,long *Np2,long *Np3,double *Nx,double *Ny,double *Nz) 
/*
** Calculates The Intersection Of A Grade Line with A Triangle Edge
*/
{
 int ret = DTM_SUCCESS;
 int    sd1,sd2 ;
 long   dbg=DTM_TRACE_VALUE(0),mp1,mp2,clc ;
 double dx,dy,dz,Xr,Yr;
 double radius ;
/*
** Write Debug Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting First Grade Trace Point") ;
    bcdtmWrite_message(0,0,0,"Start Point = %10.4lf %10.4lf %10.4lf",Sx,Sy,Sz) ; 
    bcdtmWrite_message(0,0,0,"P1 = %6ld P2 = %6ld P3 = %6ld",P1,P2,P3) ; 
    bcdtmWrite_message(0,0,0,"Grade Angle = %12.10lf",GradeAngle) ;
   }
/*
** Initialise
*/
 *TracePointFound = 0 ;
 *Np1 = *Np2 = *Np3 = dtmP->nullPnt ;
 *Nx = *Ny = *Nz = 0.0 ;
/*
** Calculate Radial At Grade Slope Angle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating EndPoints") ; 
 dx = dtmP->xMax - dtmP->xMin ;
 dy = dtmP->yMax - dtmP->yMin ;
 radius = sqrt(dx*dx + dy*dy) ;
 Xr = Sx + radius * cos(GradeAngle) ;
 Yr = Sy + radius * sin(GradeAngle) ;
/*
** Grade Slope Start From Tin Point
*/
 if( P2 == dtmP->nullPnt && P3 == dtmP->nullPnt )
   {
    clc = nodeAddrP(dtmP,P1)->cPtr ;
    mp1  = clistAddrP(dtmP,clc)->pntNum ;
    if( ( mp2 = bcdtmList_nextAntDtmObject(dtmP,P1,mp1)) < 0 ) goto errexit ;
    while ( clc != dtmP->nullPtr )
      {
       mp1 = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( nodeAddrP(dtmP,P1)->hPtr != mp1 )   
         {
          sd1 = bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,mp1)->x,pointAddrP(dtmP,mp1)->y) ;
          sd2 = bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,mp2)->x,pointAddrP(dtmP,mp2)->y) ;
          if( sd1 != sd2 )
            {
             sd1 = bcdtmMath_sideOf(pointAddrP(dtmP,mp1)->x,pointAddrP(dtmP,mp1)->y,pointAddrP(dtmP,mp2)->x,pointAddrP(dtmP,mp2)->y,Sx,Sy) ;
             sd2 = bcdtmMath_sideOf(pointAddrP(dtmP,mp1)->x,pointAddrP(dtmP,mp1)->y,pointAddrP(dtmP,mp2)->x,pointAddrP(dtmP,mp2)->y,Xr,Yr) ;
             if( sd1 != sd2 )
               {
                clc = dtmP->nullPtr ;
                *TracePointFound = 1 ;
               }
            } 
         }
       if( clc != dtmP->nullPtr ) mp2 = mp1 ;
      }
    if( ! *TracePointFound ) return(0) ;
   }
/*
** Grade Slope Start From Triangle
*/
 else
   {
/*
** Determine If Radial Intersect Triangle Vertex
*/
    mp1 = mp2 = dtmP->nullPnt ;
    if( bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) == 0 ) mp1 = P1 ;
    if( bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) == 0 ) mp1 = P2 ;
    if( bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y) == 0 ) mp1 = P3 ;
    if( mp1 != dtmP->nullPnt )
      {
       *Nx = pointAddrP(dtmP,mp1)->x ; 
       *Ny = pointAddrP(dtmP,mp1)->y ; 
       *Nz = pointAddrP(dtmP,mp1)->z ;
       *Np1 = mp1 ;
       return(0) ;
      }
/*
** Determine Triangle Edge Intersected With Radial
*/
    if( bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) < 0 &&
        bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) > 0 &&
        bcdtmMath_sideOf(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,Xr,Yr) < 0 ) { mp1 = P1 ; mp2 = P2 ; }	 
    if( bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) < 0 &&
        bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y) > 0 &&   
        bcdtmMath_sideOf(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,Xr,Yr) < 0 ) { mp1 = P2 ; mp2 = P3 ; }	 
    if( bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y) < 0 &&
        bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) > 0 && 
        bcdtmMath_sideOf(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,Xr,Yr) < 0 ) { mp1 = P3 ; mp2 = P1 ; }	 
   }
/*
** Calculate Intercept Of Radial On Mp1,Mp2 
*/
 bcdtmMath_intersectCordLines(Sx,Sy,Xr,Yr,pointAddrP(dtmP,mp1)->x,pointAddrP(dtmP,mp1)->y,pointAddrP(dtmP,mp2)->x,pointAddrP(dtmP,mp2)->y,Nx,Ny) ;
 dx = pointAddrP(dtmP,mp2)->x - pointAddrP(dtmP,mp1)->x ;
 dy = pointAddrP(dtmP,mp2)->y - pointAddrP(dtmP,mp1)->y ;
 dz = pointAddrP(dtmP,mp2)->z - pointAddrP(dtmP,mp1)->z ;
 if( fabs(dx) >= fabs(dy) ) *Nz = pointAddrP(dtmP,mp1)->z +  dz * (*Nx - pointAddrP(dtmP,mp1)->x) / dx ;
 else                       *Nz = pointAddrP(dtmP,mp1)->z +  dz * (*Ny - pointAddrP(dtmP,mp1)->y) / dy ;
/*
** Set Point For Next Trace
*/
 *Np1 = mp2 ; *Np2 = mp1 ;
 if( ( *Np3 = bcdtmList_nextAntDtmObject(dtmP,mp2,mp1)) < 0 ) goto errexit ;
 *TracePointFound = 1 ;
/*
** Job Completed
*/
/*
** Clean Up
*/
 cleanup :
 if( ! ret && dbg ) bcdtmWrite_message(0,0,0,"Calculating Grade Slope Start Directions Completed") ;
 if(   ret && dbg ) bcdtmWrite_message(0,0,0,"Calculating Grade Slope Start Directions Error") ;
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
BENTLEYDTM_Public int bcdtmGrade_traceGradedSlopeFromTrianglePointDtmObject(BC_DTM_OBJ *dtmP,long P1,double Sx,double Sy,double GradeSlope,long *TracePointFound,long *Np1,long *Np2,long *Np3,double *Nx,double *Ny,double *Nz) 
/*
** This Function Determines A Grade Slope From A Triangle Point
*/
{
 long   dbg=DTM_TRACE_VALUE(0),p1,p2,p3,clc,GradeFound,GradeSlopeAngleFound ;
 double dx,dy,dz,Xr,Yr,radius,anglemin=0.0,anglemax=0.0,GradeAngle=0.0,GradeSlopeAngleOne,GradeSlopeAngleTwo ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Graded Slope of %10.4lf From Triangle Point P1 = %6ld",GradeSlope,P1) ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
   }
/*
** Initialise
*/
 *TracePointFound = 0 ;
 *Np1 = *Np2 = *Np3 = dtmP->nullPnt ;
 *Nx  = *Ny  = *Nz  = 0.0    ;
 dx   = dtmP->xMax - dtmP->xMin ;
 dy   = dtmP->yMax - dtmP->yMin ;
/*
** Scan Triangle Point For Grade Slope
*/
 p1 = P1 ;
 GradeFound = 0 ;
 if( ( clc = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr ) 
   {
    if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while ( clc != dtmP->nullPtr  && ! GradeFound )
      {
       p3  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
/*
**  Calculate Grade Slope Angles For Triangle
*/
       if( bcdtmGrade_calculateGradeSlopeAnglesForTriangleDtmObject(dtmP,p1,p2,p3,GradeSlope,&GradeSlopeAngleFound,&GradeSlopeAngleOne,&GradeSlopeAngleTwo) ) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"GradeSlopeAngleFound = %2ld GradeSlopeAngleOne = %12.10lf GradeSlopeAngleTwo = %12.10lf",GradeSlopeAngleFound,GradeSlopeAngleOne,GradeSlopeAngleTwo) ;
/*
**  If Grade Slope Angles Found, Test If Grade Slope Angles Are Internal To Triangle
*/
       if( GradeSlopeAngleFound )
         {
          anglemin = bcdtmMath_getPointAngleDtmObject(dtmP,p1,p3) ;   
          anglemax = bcdtmMath_getPointAngleDtmObject(dtmP,p1,p2) ;
          if( anglemax < anglemin ) anglemax += DTM_2PYE ;
          if( dbg ) bcdtmWrite_message(0,0,0,"anglemin = %12.8lf anglemax = %12.8lf",anglemin,anglemax) ;
          if( GradeSlopeAngleOne < anglemin ) GradeSlopeAngleOne += DTM_2PYE ;
          if( GradeSlopeAngleTwo < anglemin ) GradeSlopeAngleTwo += DTM_2PYE ;
          if( GradeSlopeAngleOne <= anglemax ) { GradeFound = 1 ; GradeAngle = GradeSlopeAngleOne ; }
          else if (  GradeSlopeAngleFound == 2 && GradeSlopeAngleTwo <= anglemax ) { GradeFound = 1 ; GradeAngle = GradeSlopeAngleTwo ; }
          if( GradeFound ) { *Np1 = p2 ; *Np2 = p3 ; }
          if( dbg && GradeFound ) bcdtmWrite_message(0,0,0,"Grade Slope Found ** Np1 = %6ld Np2 = %6ld",*Np1,*Np2) ;
         }   
/*
**  Reset For Next Triangle
*/
       p2 = p3 ;
      }
   }
/*
** If Grade Found Calculate Next Grade Slope Point
*/
 if( GradeFound )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Next Grade Slope Point") ;
    *TracePointFound = 1 ;
    if     ( GradeAngle == anglemin ) { *Np1 = *Np2 ; *Np2 = *Np3 = dtmP->nullPnt ; }
    else if( GradeAngle == anglemax ) { *Np2 = *Np3 = dtmP->nullPnt ; *Nx = pointAddrP(dtmP,*Np1)->x ;  *Ny = pointAddrP(dtmP,*Np1)->x ; *Nz = pointAddrP(dtmP,*Np1)->z ; }
/*
** Caculate Intercept On P2-P3
*/
    else
      {
       radius = sqrt(dx*dx+dy*dy) ;
       Xr = Sx + radius * cos(GradeAngle) ;
       Yr = Sy + radius * sin(GradeAngle) ;
       bcdtmMath_intersectCordLines(Sx,Sy,Xr,Yr,pointAddrP(dtmP,*Np1)->x,pointAddrP(dtmP,*Np1)->y,pointAddrP(dtmP,*Np2)->x,pointAddrP(dtmP,*Np2)->y,Nx,Ny) ;
       if     ( bcdtmMath_distance(pointAddrP(dtmP,*Np1)->x,pointAddrP(dtmP,*Np1)->y,*Nx,*Ny) <= dtmP->ppTol ) { *Np2 = *Np3 = dtmP->nullPnt ; }
       else if( bcdtmMath_distance(pointAddrP(dtmP,*Np2)->x,pointAddrP(dtmP,*Np2)->y,*Nx,*Ny) <= dtmP->ppTol ) { *Np1 = *Np2 ; *Np2 = *Np3 = dtmP->nullPnt ; }
       else
         {
          dx = pointAddrP(dtmP,*Np2)->x - pointAddrP(dtmP,*Np1)->x ;
          dy = pointAddrP(dtmP,*Np2)->y - pointAddrP(dtmP,*Np1)->y ;
          dz = pointAddrP(dtmP,*Np2)->z - pointAddrP(dtmP,*Np1)->z ;
          if( fabs(dx) >= fabs(dy) ) *Nz = pointAddrP(dtmP,*Np1)->z +  dz * (*Nx - pointAddrP(dtmP,*Np1)->x) / dx ;
          else                       *Nz = pointAddrP(dtmP,*Np1)->z +  dz * (*Ny - pointAddrP(dtmP,*Np1)->y) / dy ;
          if(( *Np3 = bcdtmList_nextAntDtmObject(dtmP,*Np1,*Np2)) < 0 ) goto errexit ;
         }
      }
/*
** Check For Grade Slope Through Point
*/
    if( *Np2 == dtmP->nullPnt ) { *Nx = pointAddrP(dtmP,*Np1)->x ;  *Ny = pointAddrP(dtmP,*Np1)->x ; *Nz = pointAddrP(dtmP,*Np1)->z ; }
   }
/*
** Write Termininating Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Trace Point Found = %2ld",*TracePointFound) ;
    bcdtmWrite_message(0,0,0,"Np1 = %10ld Np2 = %10ld Np3 = %10ld",*Np1,*Np2,*Np3) ;
    bcdtmWrite_message(0,0,0,"Nx  = %10.4lf Ny  = %10.4lf Nz = %10.4lf",*Nx,*Ny,*Nz)  ;
    bcdtmWrite_message(0,0,0,"Tracing Graded Slope of %10.4lf From Triangle Point P1 = %6ld Completed",GradeSlope,P1) ;
   }
/*
** Normal Exit
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 goto errexit ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmGrade_traceGradedSlopeFromTriangleEdgeDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,double Sx,double Sy,double Sz,double GradeSlope,double LastAngle,long *TracePointFound,long *Np1,long *Np2,long *Np3,double *Nx,double *Ny,double *Nz)
{
 int    sdof ;
 long   dbg=DTM_TRACE_VALUE(0),HullLine ;
 double dx,dy,dz,Xr,Yr,Radius,GradeSlopeAngle ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tracing Graded Slope of %10.4lf From Triangle Edge P1 = %6ld P2 = %6ld P3 = %6ld",GradeSlope,P1,P2,P3) ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
    bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
    bcdtmWrite_message(0,0,0,"P3 = %6ld ** %10.4lf %10.4lf %10.4lf",P3,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z) ;
   }
/*
** Initialise Variables
*/
 *TracePointFound = 0 ;
 *Np1 = *Np2 = *Np3 = dtmP->nullPnt ;
 *Nx  = *Ny  = *Nz  = 0.0    ;
 dx   = dtmP->xMax - dtmP->xMin ;
 dy   = dtmP->yMax - dtmP->yMin ;
/*
** Check For Termination On DTMFeatureState::Tin Edge
*/
 if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,P1,P2,&HullLine)) goto errexit ;
 if( HullLine ) return(0) ;
/*
** Calculate Angle Of Grade Slope Across Triangle Surface
*/
 if( bcdtmGrade_calculateGradeSlopeAngleAcrossTriangleFaceDtmObject(dtmP,P1,P2,P3,Sx,Sy,Sz,GradeSlope,LastAngle,TracePointFound,&GradeSlopeAngle) ) goto errexit ;
 if( ! *TracePointFound ) return(0) ;
/*
** Calculate Radial Out From Sx,Sy,Sz
*/
 Radius = dx * dx + dy * dy ;
 Xr = Sx + Radius * cos(GradeSlopeAngle) ;
 Yr = Sy + Radius * sin(GradeSlopeAngle) ;
/*
** Calculate Intercept Of Radial On P1-P3 Or P2-P3
*/
 sdof = bcdtmMath_sideOf(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y) ;
/*
** Radial Passes Through P3
*/
 if( sdof == 0 ) { *Np1 = P3 ; *Nx = pointAddrP(dtmP,P3)->x ; *Ny = pointAddrP(dtmP,P3)->y ; *Nz = pointAddrP(dtmP,P3)->z ; return(0) ; }
/*
** Radial Intersects P3-P2
*/
 if( sdof >  0 ) 
   { 
    bcdtmMath_intersectCordLines(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,Nx,Ny) ;
    if     ( bcdtmMath_distance(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,*Nx,*Ny) <= dtmP->ppTol ) { *Np1 = P3 ; *Np2 = *Np3 = dtmP->nullPnt ; }
    else if( bcdtmMath_distance(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,*Nx,*Ny) <= dtmP->ppTol ) { *Np1 = P2 ; *Np2 = *Np3 = dtmP->nullPnt ; }
    else
      {
       dx = pointAddrP(dtmP,P3)->x - pointAddrP(dtmP,P2)->x ;
       dy = pointAddrP(dtmP,P3)->y - pointAddrP(dtmP,P2)->y ;
       dz = pointAddrP(dtmP,P3)->z - pointAddrP(dtmP,P2)->z ;
       if( fabs(dx) >= fabs(dy) ) *Nz = pointAddrP(dtmP,P2)->z +  dz * (*Nx - pointAddrP(dtmP,P2)->x) / dx ;
       else                       *Nz = pointAddrP(dtmP,P2)->z +  dz * (*Ny - pointAddrP(dtmP,P2)->y) / dy ;
       *Np1 = P3 ; *Np2 = P2 ; 
       if(( *Np3 = bcdtmList_nextAntDtmObject(dtmP,*Np1,*Np2)) < 0 ) goto errexit ;
      }
    if( *Np2 == dtmP->nullPnt ) { *Nx = pointAddrP(dtmP,*Np1)->x ; *Ny = pointAddrP(dtmP,*Np1)->y ; *Nz = pointAddrP(dtmP,*Np1)->z ; }
    *TracePointFound = 1 ;
   }
/*
** Radial Intersects P3-P1
*/
 if( sdof <  0 ) 
   { 
    bcdtmMath_intersectCordLines(Sx,Sy,Xr,Yr,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,Nx,Ny) ;
    if     ( bcdtmMath_distance(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,*Nx,*Ny) <= dtmP->ppTol ) { *Np1 = P3 ; *Np2 = *Np3 = dtmP->nullPnt ; }
    else if( bcdtmMath_distance(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,*Nx,*Ny) <= dtmP->ppTol ) { *Np1 = P1 ; *Np2 = *Np3 = dtmP->nullPnt ; }
    else
      { 
       dx = pointAddrP(dtmP,P3)->x - pointAddrP(dtmP,P1)->x ;
       dy = pointAddrP(dtmP,P3)->y - pointAddrP(dtmP,P1)->y ;
       dz = pointAddrP(dtmP,P3)->z - pointAddrP(dtmP,P1)->z ;
       if( fabs(dx) >= fabs(dy) ) *Nz = pointAddrP(dtmP,P1)->z +  dz * (*Nx - pointAddrP(dtmP,P1)->x) / dx ;
       else                       *Nz = pointAddrP(dtmP,P1)->z +  dz * (*Ny - pointAddrP(dtmP,P1)->y) / dy ;
       *Np1 = P1 ; *Np2 = P3 ; 
       if(( *Np3 = bcdtmList_nextAntDtmObject(dtmP,*Np1,*Np2)) < 0 ) goto errexit ;
      }
    if( *Np2 == dtmP->nullPnt ) { *Nx = pointAddrP(dtmP,*Np1)->x ; *Ny = pointAddrP(dtmP,*Np1)->y ; *Nz = pointAddrP(dtmP,*Np1)->z ; }
   }
/*
** If Trace Point Found Write Angles And Slopes ** Development Only
*/
 if( dbg && TracePointFound )
   {
    bcdtmWrite_message(0,0,0,"*****************************************************") ;
    bcdtmWrite_message(0,0,0,"** Slope Trace Statistics From Triangle Edge %6ld %6ld Across Triangle Face",P1,P2) ;
    bcdtmWrite_message(0,0,0,"** Sx = %10.4lf Sy = %10.4lf Sz = %10.4lf",Sx,Sy,Sz) ;
    bcdtmWrite_message(0,0,0,"** Nx = %10.4lf Ny = %10.4lf Nz = %10.4lf",*Nx,*Ny,*Nz) ;
    bcdtmWrite_message(0,0,0,"** Angle = %10.8lf Distance = %10.4lf Slope = %10.4lf",bcdtmMath_getAngle(Sx,Sy,*Nx,*Ny),bcdtmMath_distance(Sx,Sy,*Nx,*Ny),(*Nz-Sz)/bcdtmMath_distance(Sx,Sy,*Nx,*Ny)) ;
    bcdtmWrite_message(0,0,0,"*****************************************************") ;
   }
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit : 
 *TracePointFound = 0 ;
 goto errexit ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmGrade_calculateGradeSlopeAngleAcrossTriangleFaceDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,double Sx,double Sy,double Sz,double GradeSlope,double LastAngle,long *GradePointFound,double *GradeSlopeAngle)
{
 int    dbg=DTM_TRACE_VALUE(0) ;
 long   GradeSlopeAngleFound ;
 double a1,a2,anglemin,anglemax,Xr,Yr,Zr ;
 double A,B,C,D,GradeSlopeAngleOne,GradeSlopeAngleTwo ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Grade Slope Angle From Edge %6ld %6ld [%6ld] Across Triangle Face",P1,P2,P3) ;
/*
** Initialise
*/
 *GradePointFound = 0 ;
 *GradeSlopeAngle = 0.0 ;
/*
** Get Min & Max Angles For Triangle Edge In An AntiClockwise Direction
*/
 anglemin = bcdtmMath_getPointAngleDtmObject(dtmP,P1,P2) ;   
 anglemax = bcdtmMath_getPointAngleDtmObject(dtmP,P2,P1) ;
 if( anglemax < anglemin ) anglemax += DTM_2PYE ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Triangle Edge Angle In An Anticlockwise Direction") ;
    bcdtmWrite_message(0,0,0,"Min Angle = %12.10lf Max Angle = %12.10lf",anglemin,anglemax) ;
   }
/*
** Calculate Grade Slope Angles For Triangle
*/
 if( bcdtmGrade_calculateGradeSlopeAnglesForTriangleDtmObject(dtmP,P1,P2,P3,GradeSlope,&GradeSlopeAngleFound,&GradeSlopeAngleOne,&GradeSlopeAngleTwo) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"GradeSlopeAngleFound = %2ld GradeSlopeAngleOne = %12.10lf GradeSlopeAngleTwo = %12.10lf",GradeSlopeAngleFound,GradeSlopeAngleOne,GradeSlopeAngleTwo) ;
/*
** If Two Grade Slope Angles Found Take Angle Closest To Last Angle
*/
 if( GradeSlopeAngleFound == 2 )
   {
    if( GradeSlopeAngleOne < anglemin ) GradeSlopeAngleOne += DTM_2PYE ;
    if( GradeSlopeAngleTwo < anglemin ) GradeSlopeAngleTwo += DTM_2PYE ;
    if( anglemin <= GradeSlopeAngleOne && GradeSlopeAngleOne <= anglemax && 
        anglemin <= GradeSlopeAngleTwo && GradeSlopeAngleTwo <= anglemax    )
      {
       while ( GradeSlopeAngleOne >= DTM_2PYE ) GradeSlopeAngleOne -= DTM_2PYE ;
       while ( GradeSlopeAngleTwo >= DTM_2PYE ) GradeSlopeAngleTwo -= DTM_2PYE ;
       if( dbg ) 
         {
          bcdtmWrite_message(0,0,0,"Two Grade Slope Angle Solutions Found") ;
          bcdtmWrite_message(0,0,0,"Grade Slope One = %12.10lf",GradeSlopeAngleOne) ;
          bcdtmWrite_message(0,0,0,"Grade Slope Two = %12.10lf",GradeSlopeAngleTwo) ;
          bcdtmWrite_message(0,0,0,"LastAngle       = %12.10lf",LastAngle) ;
         } 
       if( GradeSlopeAngleOne < LastAngle ) GradeSlopeAngleOne += DTM_2PYE ;
       if( GradeSlopeAngleTwo < LastAngle ) GradeSlopeAngleTwo += DTM_2PYE ;
       a1 = GradeSlopeAngleOne - LastAngle ;
       if( a1 > DTM_2PYE - a1 ) a1 = DTM_2PYE - a1 ;
       a2 = GradeSlopeAngleTwo - LastAngle ;
       if( a2 > DTM_2PYE - a2 ) a2 = DTM_2PYE - a2 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"a1 = %10.8lf a2 = %10.8lf",a1,a2) ; 
       if( a1 <= a2 ) *GradeSlopeAngle = GradeSlopeAngleOne ;
       else           *GradeSlopeAngle = GradeSlopeAngleTwo ;
       while ( *GradeSlopeAngle >= DTM_2PYE ) *GradeSlopeAngle -= DTM_2PYE ;
       *GradePointFound = 1 ;
       GradeSlopeAngleFound = 0 ;
      }   
   }

/*
** Determine Solution To The Left Of Triangle Edge P1P2
*/
 if( GradeSlopeAngleFound )
   {
/*
**  Check If Grade Slope Angle One Is Between AngleMin And AngleMax In An Anticlockwise Direction
*/
    if( GradeSlopeAngleOne < anglemin ) GradeSlopeAngleOne += DTM_2PYE ;
    if( anglemin <= GradeSlopeAngleOne && GradeSlopeAngleOne <= anglemax )
      {
       *GradePointFound = 1 ;
       *GradeSlopeAngle = GradeSlopeAngleOne ;
       while ( *GradeSlopeAngle >= DTM_2PYE ) *GradeSlopeAngle -= DTM_2PYE ;
      } 
/*
**  Check If Grade Slope Angle Two Is Between AngleMin And AngleMax In An Anticlockwise Direction
*/
    else if ( GradeSlopeAngleFound == 2 )
      {
       if( GradeSlopeAngleTwo < anglemin ) GradeSlopeAngleTwo += DTM_2PYE ;
       if( anglemin <= GradeSlopeAngleTwo && GradeSlopeAngleTwo <= anglemax )
         {
          *GradePointFound = 1 ;
          *GradeSlopeAngle = GradeSlopeAngleTwo ;
          while ( *GradeSlopeAngle >= DTM_2PYE ) *GradeSlopeAngle -= DTM_2PYE ;
         } 
      } 
   }
/*
** Write Status Message And Check Slope
*/
 if( dbg ) 
   {
    if( *GradePointFound )
      {
       bcdtmMath_calculatePlaneCoefficients(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z,&A,&B,&C,&D) ;
       Xr = Sx + cos(*GradeSlopeAngle) ;  
       Yr = Sy + sin(*GradeSlopeAngle) ;  
       bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A,B,C,D) ;
       bcdtmWrite_message(0,0,0,"Edge Grade Slope Angle = %12.10lf GradeSlope = %10.4lf",*GradeSlopeAngle,(Zr-Sz)) ;
      }
    else bcdtmWrite_message(0,0,0,"Grade Point Not Found") ;
   }
/*
** Normal Return
*/
 return(0) ; 
/*
** Error Exit
*/
 errexit :
 goto errexit ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmGrade_calculateGradeSlopeAnglesForTriangleDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long P3,double GradeSlope,long *GradeSlopeAngleFound,double *GradeSlopeAngleOne,double *GradeSlopeAngleTwo)
/*
** This Function Calculates The Grade Slope Angles For A Triangle
** Arguements
**
** <== GradeSlopeAngleFound == 0   No Solution  ( Grade Slope >  Max Triangle Slope )  
**                          == 1   One Solution ( Grade Slope == Max Triangle Slope )
**                          == 2   Two Solution ( Grade Slope <  Max Triangle Slope )   
*/
{
 long   dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 double A,B,C,D,DescentAngle,AscentAngle,MinTriangleSlope,MaxTriangleSlope ;
 double angle,anglemin,anglemax,Xr,Yr,Zr,Zs,x,y,z ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Grade Slope Angles For Triangle") ;
/*
** Initialise
*/
 *GradeSlopeAngleFound = 0 ;
 *GradeSlopeAngleOne = *GradeSlopeAngleTwo = 0.0 ;
/*
** Calculate Triangle Parameters
*/
 bcdtmMath_calculatePlaneCoefficients(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z,&A,&B,&C,&D) ;
 x  = (pointAddrP(dtmP,P1)->x + pointAddrP(dtmP,P2)->x + pointAddrP(dtmP,P3)->x ) / 3.0 ;
 y  = (pointAddrP(dtmP,P1)->y + pointAddrP(dtmP,P2)->y + pointAddrP(dtmP,P3)->y ) / 3.0 ;
 z  = (pointAddrP(dtmP,P1)->z + pointAddrP(dtmP,P2)->z + pointAddrP(dtmP,P3)->z ) / 3.0 ;
 Zs = z + GradeSlope ;
/*
** Get Maximum Descent And Ascent Angles For Triangle
*/
 bcdtmMath_getTriangleDescentAndAscentAnglesDtmObject(dtmP,P1,P2,P3,&DescentAngle,&AscentAngle,&MaxTriangleSlope) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Descent Angle = %12.8lf Ascent Angle = %12.8lf Max Triangle Slope = %10.4lf",DescentAngle,AscentAngle,MaxTriangleSlope) ;
 MinTriangleSlope = -MaxTriangleSlope ;
/*
** If Solution ** Calculate Grade Slope Angles
*/ 
 if( fabs(GradeSlope) <= MaxTriangleSlope ) 
   { 
/*
** Check For Grade Slope Equal To Maximum Triangle Slope
*/
    if( fabs(GradeSlope) == MaxTriangleSlope ) 
      {
       *GradeSlopeAngleFound = 1 ;
       if( GradeSlope <= 0.0 ) *GradeSlopeAngleOne = DescentAngle ;
       else                    *GradeSlopeAngleOne = AscentAngle  ;
      }
/*
** Check For Grade Slope Equal To Zero
*/
    else if( GradeSlope == 0.0 )
      {
       *GradeSlopeAngleFound = 2 ;
       *GradeSlopeAngleOne = DescentAngle + DTM_PYE / 2.0 ;
       while( *GradeSlopeAngleOne >= DTM_2PYE ) *GradeSlopeAngleOne -= DTM_2PYE ;
       *GradeSlopeAngleTwo = DescentAngle - DTM_PYE / 2.0 ;
       while( *GradeSlopeAngleTwo <  0.0  ) *GradeSlopeAngleTwo += DTM_2PYE ;
      }
/*
** Calculate Two Grade Slopes 
*/
    else 
      {
       *GradeSlopeAngleFound = 2 ;
/*
** Calculate Angles For Grade Slope Less Than Zero
*/
       if( GradeSlope < 0 )
         {
/*
**        Calculate Angle To Right Of Descent Angle
*/
          anglemin = DescentAngle ; 
          anglemax = DescentAngle + DTM_PYE / 2.0 ;
          angle = ( anglemax + anglemin ) / 2.0 ;
          while( anglemax-anglemin > 0.000001 )
            {
             Xr = x + cos(angle) ;
             Yr = y + sin(angle) ;
             bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A,B,C,D) ;
             if      ( Zr == Zs ) anglemax = anglemin = angle ;
             else if ( Zr >  Zs ) anglemax = angle ;
             else                 anglemin = angle ;
             angle = ( anglemax + anglemin ) / 2.0 ;
            } 
          *GradeSlopeAngleOne = angle ;
/*
**        Calculate Angle To Left Of Descent Angle
*/
          anglemin = DescentAngle - DTM_PYE / 2.0 ;
          anglemax = DescentAngle ; 
          while( anglemin < 0.0 ) anglemin += DTM_2PYE ;
          if( anglemax < anglemin ) anglemax += DTM_2PYE ;
          angle = ( anglemax + anglemin ) / 2.0 ;
          while( anglemax-anglemin > 0.00000001 )
            {
             Xr = x + cos(angle) ;
             Yr = y + sin(angle) ;
             bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A,B,C,D) ;
             if      ( Zr == Zs ) anglemax = anglemin = angle ;
             else if ( Zr >  Zs ) anglemin = angle ;
             else                 anglemax = angle ;
             angle = ( anglemax + anglemin ) / 2.0 ;
            } 
          *GradeSlopeAngleTwo = angle ;
         }
/*
**     Calculate Angles For Grade Slope Greater Than Zero
*/
       if( GradeSlope > 0 )
         {
/*
**        Calculate Angle To Right Of Ascent Angle
*/
          anglemin = AscentAngle - DTM_PYE / 2.0 ; 
          anglemax = AscentAngle  ;
          if( anglemin < 0.0 ) anglemin += DTM_2PYE ;
          if( anglemax < anglemin ) anglemax += DTM_2PYE ; 
          angle = ( anglemax + anglemin ) / 2.0 ;
          while( anglemax-anglemin > 0.00000001 )
            {
             Xr = x + cos(angle) ;
             Yr = y + sin(angle) ;
             bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A,B,C,D) ;
             if      ( Zr == Zs ) anglemax = anglemin = angle ;
             else if ( Zr >  Zs ) anglemax = angle ;
             else                 anglemin = angle ;
             angle = ( anglemax + anglemin ) / 2.0 ;
            } 
          *GradeSlopeAngleOne = angle ;
/*
**        Calculate Angle To Right Of Ascent Angle
*/
          anglemin = AscentAngle  ;
          anglemax = AscentAngle + DTM_PYE/2.0  ; 
          angle = ( anglemax + anglemin ) / 2.0 ;
          while( anglemax-anglemin > 0.00000001 )
            {
             Xr = x + cos(angle) ;
             Yr = y + sin(angle) ;
             bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A,B,C,D) ;
             if      ( Zr == Zs ) anglemax = anglemin = angle ;
             else if ( Zr >  Zs ) anglemin = angle ;
             else                 anglemax = angle ;
             angle = ( anglemax + anglemin ) / 2.0 ;
            } 
          *GradeSlopeAngleTwo = angle ;
         }
      }
   } 
/*
** Normalise Angles
*/
 while( *GradeSlopeAngleOne <   0.0 ) *GradeSlopeAngleOne += DTM_2PYE ;
 while( *GradeSlopeAngleOne >= DTM_2PYE ) *GradeSlopeAngleOne -= DTM_2PYE ;
 while( *GradeSlopeAngleTwo <   0.0 ) *GradeSlopeAngleTwo += DTM_2PYE ;
 while( *GradeSlopeAngleTwo >= DTM_2PYE ) *GradeSlopeAngleTwo -= DTM_2PYE ;
/*
** Write Status Message And Slope 
*/
 if( dbg )
   {
    if( ! *GradeSlopeAngleFound ) bcdtmWrite_message(0,0,0,"No Triangle Grade Slope Solution") ; 
    else
      {
       Xr = x + cos(*GradeSlopeAngleOne) ;
       Yr = y + sin(*GradeSlopeAngleOne) ;
       bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A,B,C,D) ;
       bcdtmWrite_message(0,0,0,"Grade Slope Angle One = %12.10lf Slope = %10.4lf",*GradeSlopeAngleOne,Zr-z) ; 
       if( *GradeSlopeAngleFound == 2 ) 
         {
          Xr = x + cos(*GradeSlopeAngleTwo) ;
          Yr = y + sin(*GradeSlopeAngleTwo) ;
          bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A,B,C,D) ;
          bcdtmWrite_message(0,0,0,"Grade Slope Angle Two = %12.10lf Slope = %10.4lf",*GradeSlopeAngleTwo,Zr-z) ; 
         }
      }  
    bcdtmWrite_message(0,0,0,"Calculating Grade Slope Angles For Triangle Completed") ;
   }
/*
** Check Calculated Slopes
*/
 if( cdbg && *GradeSlopeAngleFound )
   {
    Xr = x + cos(*GradeSlopeAngleOne) ;
    Yr = y + sin(*GradeSlopeAngleOne) ;
    bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A,B,C,D) ;
    if( fabs((Zr-z)-GradeSlope) > 0.0001 ) { bcdtmWrite_message(2,0,0,"Angle One Calculated Grade Slope %10.5lf Different To Requested Grade Slope %10.5lf",(Zr-z),GradeSlope) ; goto errexit ; } 
    if( *GradeSlopeAngleFound == 2 ) 
      {
       Xr = x + cos(*GradeSlopeAngleTwo) ;
       Yr = y + sin(*GradeSlopeAngleTwo) ;
       bcdtmMath_interpolatePointOnPlane(Xr,Yr,&Zr,A,B,C,D) ;
       if( fabs((Zr-z)-GradeSlope) > 0.0001 ) { bcdtmWrite_message(2,0,0,"Angle Two Calculated Grade Slope %10.5lf Different To Requested Grade Slope %10.5lf",(Zr-z),GradeSlope) ; goto errexit ; }
      } 
   }
/*
** Normal Return 
*/
 return(0) ;
/*
** Error Return
*/
 errexit :
 goto errexit ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmGrade_getMacaoGradeSlopeStartDirectionsDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double Slope,double Distance,double **StartDirections,long *NumStartDirections)
/*
** This Function Gets The Starting Directions For A Macao Graded Slope Trace On The Tin Surface
**
** Tin                      ==> Dtm Object
** x                        ==> Start Point x Coordinate
** y                        ==> Start Point y Coordinate
** Slope                    ==> Slope ( Vertical / Horizontal ) For Determing Grade Slope Start Directions
** Distance                <==  Radial Distance Out To Search For A Surface Point
** StartDirections         <==  Angle Directions In Radians That The Grade Slope Can Trace From The Point XY 
** NumberOfStartDirections <==  Number Of Values Stored In StartDirection
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   P1,P2,P3,Ptype,VoidFlag ;
 double z ;
/*
** Write Arguements For Development Purposes
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Calculating Macao Grade Slope Start Directions") ;
    bcdtmWrite_message(0,0,0,"Dtm Object      = %p",dtmP)  ;
    bcdtmWrite_message(0,0,0,"x               = %15.10lf",x)  ;
    bcdtmWrite_message(0,0,0,"y               = %15.10lf",y)  ;
    bcdtmWrite_message(0,0,0,"Slope           = %10.4lf",Slope) ;
   }
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ; ;
/*
** Check DTM Is Triangulated
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Initialise  
*/
 *NumStartDirections = 0 ;
 if( *StartDirections != NULL ) { free(*StartDirections) ; *StartDirections = NULL ; }
 P1 = P2 = P3 = dtmP->nullPnt ;
/*
** Test For Valid DTMFeatureState::Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ; ;
/*
** Find Triangle Containing Start Point On DTMFeatureState::Tin
*/
 if( bcdtmFind_triangleForPointDtmObject(dtmP,x,y,&z,&Ptype,&P1,&P2,&P3)) goto errexit ; ;
 if( Ptype == 0 ) { bcdtmWrite_message(1,0,0,"Start Point Not On Tin") ; goto errexit ; ; }
/*
** Check Point To Point Tolerance
*/
 if ( bcdtmMath_distance(x,y,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) <= dtmP->ppTol ) { Ptype = 1 ; P2 = P3 = dtmP->nullPnt ; }
 if( P2 != dtmP->nullPnt ) if( bcdtmMath_distance(x,y,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) <= dtmP->ppTol ) { Ptype = 1 ; P1 = P2 ; P2 = P3 = dtmP->nullPnt ; }
 if( P3 != dtmP->nullPnt ) if( bcdtmMath_distance(x,y,pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y) <= dtmP->ppTol ) { Ptype = 1 ; P1 = P3 ; P2 = P3 = dtmP->nullPnt ; }
/*
** Test For Point In Void
*/
 VoidFlag = 0 ;
 if( Ptype == 1 && bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,P1)->PCWD) ) VoidFlag = 1 ;
 if( Ptype == 2 || Ptype == 3 )  if( bcdtmList_testForVoidLineDtmObject(dtmP,P1,P2,&VoidFlag)) goto errexit ; ;
 if( Ptype == 4 ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,P2,P3,&VoidFlag)) goto errexit ; ;
 if( VoidFlag ) { bcdtmWrite_message(1,0,0,"Start Point In Void") ; goto cleanup ; ; }
/*
** Get Start Dirtection For Triangle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Start Directions") ;
 if( bcdtmGrade_calculateMacaoGradeSlopeStartDirectionsForPointDtmObject(dtmP,x,y,z,Slope,Distance,StartDirections,NumStartDirections) ) goto errexit ;
/*
** Write Out Start Directions
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Start Directions = %4ld",*NumStartDirections) ;
    for( P1 = 0 ; P1 < *NumStartDirections ; ++P1 )
      {
       bcdtmWrite_message(0,0,0,"StartDirection[%4ld] = %12.5lf Radians",P1,*(StartDirections+P1)) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Macao Grade Slope Start Directions Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Macao Grade Slope Start Directions Error") ;
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
BENTLEYDTM_Public int bcdtmGrade_calculateMacaoGradeSlopeStartDirectionsForPointDtmObject(BC_DTM_OBJ *dtmP,double Sx,double Sy,double Sz,double Slope,double Distance,double **startDirectionsPP,long *numStartDirectionsP) 
/*
** Calculates The Macao Start Directions For A Grade Line From A Point
*/
{
 long   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   numDrapePts,surfaceFlag,numMem=0,incMem=10 ;
 double X1,Y1,X2,Y2,Xs=0.0,Ys=0.0,Zs=0.0,angle,anginc ;
 DPoint3d    drapeString[2] ;
 DTM_DRAPE_POINT *drapeP,*drapePtsP=NULL ;
/*
** Write Debug Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Calculating Macao Grade Slope Start Directions For Point") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ; 
    bcdtmWrite_message(0,0,0,"Sx          = %12.4lf",Sx) ; 
    bcdtmWrite_message(0,0,0,"Sy          = %12.4lf",Sy) ; 
    bcdtmWrite_message(0,0,0,"Sz          = %12.4lf",Sz) ; 
    bcdtmWrite_message(0,0,0,"Slope       = %12.8lf",Slope) ; 
    bcdtmWrite_message(0,0,0,"Distance    = %12.4lf",Distance) ; 
   }
/*
** Initialise
*/
 if( *startDirectionsPP != NULL ) { free(*startDirectionsPP) ; *startDirectionsPP = NULL ; }
 *numStartDirectionsP = 0 ;
/*
** Calcualte z Value At Slope And Distance
*/
 Zs = Sz + Slope * Distance ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Surface z At Slope And Distance = %10.4lf",Zs) ;
/*
** Scan Around Point At Diameter Distance Looking For Surface Point At Slope
*/
 angle  = 0.0 ;
 X1 = Sx + Distance * cos(angle) ;
 Y1 = Sy + Distance * sin(angle) ;
 anginc = DTM_2PYE / 500.0 ;
 while ( angle < DTM_2PYE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"angle = %12.10lf",angle) ;
    angle = angle + anginc ;
    X2 = Sx + Distance * cos(angle) ;
    Y2 = Sy + Distance * sin(angle) ;
/*
**  Drape Feature Points On Tin
*/
    drapeString[0].x = X1 ; drapeString[0].y = Y1 ; drapeString[0].z = 0.0 ; 
    drapeString[1].x = X2 ; drapeString[1].y = Y2 ; drapeString[1].z = 0.0 ; 
    if( bcdtmDrape_stringDtmObject(dtmP,drapeString,2,FALSE,&drapePtsP,&numDrapePts)) goto errexit ;
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Drape Points = %6ld",numDrapePts) ;
       for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP )  
         {
          bcdtmWrite_message(0,0,0,"Drape Point[%6ld]  L = %4ld T = %2ld ** %10.4lf %10.4lf",(long)(drapeP-drapePtsP),drapeP->drapeLine,drapeP->drapeType,drapeP->drapeX,drapeP->drapeY ) ;
         }
      } 
/*
** Look For Surface z
*/
   for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts - 1 ; ++drapeP )
     {
      if( dbg ) bcdtmWrite_message(0,0,0,"drapePoint[%8ld] ** Type1 = %2ld Type2 = %2ld",(long)(drapeP-drapePtsP),drapeP->drapeType,(drapeP+1)->drapeType) ;
      if (drapeP->drapeType != DTMDrapedLineCode::External && (drapeP + 1)->drapeType != DTMDrapedLineCode::External)
        {
         surfaceFlag = 0 ;
         if     (  drapeP->drapeZ    == Zs ) { surfaceFlag = 1 ; Xs = (drapeP+1)->drapeX ; Ys = drapeP->drapeY ; }
         else if( (drapeP+1)->drapeZ == Zs && drapeP+1 == drapePtsP + numDrapePts - 1 ) { surfaceFlag = 1 ; Xs = (drapeP+1)->drapeX ; Ys = drapeP->drapeY ; }
         else if( ( drapeP->drapeZ > Zs && Zs > (drapeP+1)->drapeZ )  || ( drapeP->drapeZ < Zs && Zs < (drapeP+1)->drapeZ ) )
           {
            surfaceFlag = 1 ;
            Xs = drapeP->drapeX + ((drapeP+1)->drapeX - drapeP->drapeX) * ( Zs - drapeP->drapeZ ) / ( (drapeP+1)->drapeZ - drapeP->drapeZ ) ;
            Ys = drapeP->drapeY + ((drapeP+1)->drapeY - drapeP->drapeY) * ( Zs - drapeP->drapeZ ) / ( (drapeP+1)->drapeZ - drapeP->drapeZ ) ;
           }
         if( dbg ) bcdtmWrite_message(0,0,0,"surfaceFlag = %2ld",surfaceFlag) ;
         if( surfaceFlag )
           {
            if( *numStartDirectionsP == numMem )
              {
               numMem = numMem + incMem ;
               if( *startDirectionsPP == NULL ) *startDirectionsPP = ( double *) malloc( numMem * sizeof(double)) ;
               else                             *startDirectionsPP = ( double *) realloc( *startDirectionsPP,numMem * sizeof(double)) ; 
               if( *startDirectionsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
              } 
            *(*startDirectionsPP+*numStartDirectionsP) = bcdtmMath_getAngle(Sx,Sy,Xs,Ys) ;
            if( dbg ) bcdtmWrite_message(0,0,0,"Start Direction Determined = %12.10lf Distance = %10.4lf Slope = %10.4lf",*(*startDirectionsPP+*numStartDirectionsP),bcdtmMath_distance(Sx,Sy,Xs,Ys),(Zs-Sz)/bcdtmMath_distance(Sx,Sy,Xs,Ys) ) ;
            ++*numStartDirectionsP ; 
           }
        }
     }
/*
**  Reset For Next Increment
*/
    X1 = X2 ;
    Y1 = Y2 ;
    if( drapePtsP != NULL ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( drapePtsP != NULL ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup  ;
}
 