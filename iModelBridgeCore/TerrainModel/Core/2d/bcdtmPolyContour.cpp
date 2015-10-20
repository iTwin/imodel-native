/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmPolyContour.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
//#pragma optimize("2x",on) 
/*
**  dtmPolyContour TypeDefs
**
**  Structure DTM_SMOOTH_CONTOUR_INDEX ( Smooth Contour Line Index Table ) Variables
**
**   P1       Lowest Valued Point Number Of Line
**   P2       Highest valued Point Number Of Line
**   Type     Line Type 
**            ==  0  Void Line
**            ==  1  Internal Tin Line
**            == -2  Tin Hull Line - Start Triangle Third Point Anti Clokwise
**            ==  2  Tin Hull Line - Start Triangle Third Point Clokwise       
**            == -3  Void Hull Line - Start Triangle Third Point Anti Clokwise
**            ==  3  Void Hull Line - Start Triangle Third Point Clokwise       
**            == -4  Island Hull Line - Start Triangle Third Point Anti Clokwise
**            ==  4  Island Hull Line - Start Triangle Third Point Clokwise       
**  NumZero    Number Of Contour Value Zeros On Line. Min 0 Max 5 
**  Idx       Pointer To Location Of First Entry For P1 In Line Index Table
**  Next      Linked List Pointer To Next Line In Structure For Contour Value
**  Zmin      Minimum Polynomial z Value For Line
**  Zmax      Maximum Polynomial z Value For Line
**  Zeros[5]  Contour Zero Values For Line expressed as Ratio of Distance Of Zero
**            From Point P1 Divided By Length Of Line
**
** Structure DTM_SMOOTH_CONTOUR_ZERO_POINT ( Smooth Contour Zero Point Table ) Variables
**
**  Pofs      Offset In Table To First Entry For Point
**  P0        Zero Tin Point Number
**  P1        Triangle Point Number
**  P2        Triangle Point Number
**  Angle     Angle To Start Contour Trace Across Triangle P0P1P2
**
*/
struct DTM_SMOOTH_CONTOUR_INDEX { long P1,P2,Type,NumZero ; DTM_SMOOTH_CONTOUR_INDEX *Idx,*Next ; double Zmin,Zmax,Zeros[5] ; } ;
struct DTM_SMOOTH_CONTOUR_ZERO_POINT { long P0,P1,P2; double Angle ; } ;
/*
**     Scanning And Tracing Functions
*/
BENTLEYDTM_EXPORT  int bcdtmPolyContour_plotContoursFromDtmObject(BC_DTM_OBJ *dtmP,long plotRangeContours,long plotSingleContours,double contourInterval,double contourRegistration,double contourMinimum,double contourMaximum,double contourValue[],long numContourValues,DPoint3d *fencePtsP,long numFencePts,DTMFeatureCallback callBackFunctionP, void *userP ) ;
BENTLEYDTM_Private int    bcdtmPolyContour_scanForContourDtmObject(BC_DTM_OBJ *dtmP,double contourValue,double Eps,double Fstep,double Rmax,double XYTolerance,double ZTolerance,DTM_SMOOTH_CONTOUR_INDEX *contourIndexP,double *partDerivP,DTMFeatureCallback callBackFunctionP, void *userP ) ;
BENTLEYDTM_Private int    bcdtmPolyContour_traceContourDtmObject(BC_DTM_OBJ *dtmP, double contourValue,long P0,long P1,long P2,long StartType,double StartAngle,double XYTolerance,double ZTolerance,DTM_SMOOTH_CONTOUR_INDEX *Cline,DTM_SMOOTH_CONTOUR_INDEX *contourIndexP,long numLines,DTM_SMOOTH_CONTOUR_ZERO_POINT *zeroPtsP,long numZeroPts,double *partDerivP,DTMFeatureCallback callBackFunctionP,void *userP) ;
BENTLEYDTM_Private int    bcdtmPolyContour_getNextTraceTriangleForContourDtmObject(BC_DTM_OBJ *dtmP,double contourValue,long ExitSide,long CloseFlag,double Sx,double Sy,DTM_SMOOTH_CONTOUR_INDEX *contourIndexP,long numLines,DTM_SMOOTH_CONTOUR_ZERO_POINT *zeroPtsP,long numZeroPts,double Eps,double Fstep,double Rmax,double XYTolerance,double ZTolerance,double *partDerivP,long *P0,long *P1,long *P2,double *Zx,double *Zy,double *Zz,long *StartType,double *StartAngle,long *TinTrace) ;
BENTLEYDTM_Private int    bcdtmPolyContour_getContourStartDirectionDtmObject(BC_DTM_OBJ *dtmP,long P0,long P1,long P2,double Zx,double Zy,double Zz,double Eps,double *StartAngle) ;
BENTLEYDTM_Private int    bcdtmPolyContour_getStartScanTriangleAtTinPointDtmObject(BC_DTM_OBJ *dtmP,long P0,double Eps,double *partDerivP,long *Np0,long *Np1,long *Np2,double *Angle) ;
BENTLEYDTM_Private int    bcdtmPolyContour_getNextScanTriangleAtTinPointDtmObject(BC_DTM_OBJ *dtmP,long P0,long P1,long P2,double Eps,double *partDerivP,long *Np0,long *Np1,long *Np2,double *Angle) ;
BENTLEYDTM_Private int    bcdtmPolyContour_removeAllZerosAtPointDtmObject(BC_DTM_OBJ *dtmP,long P,DTM_SMOOTH_CONTOUR_INDEX *contourIndexP,long numLines) ;
BENTLEYDTM_Private int bcdtmPolyContour_traceContourOverTriangleDtmObject
(
 BC_DTM_OBJ *dtmP,         // ==> Pointer To DTM 
 double contourValue,      // ==> Value Of Contour being Traced
 long   P0,                // ==> DTM Point Number Of Triangle Vertex
 long   P1,                // ==> DTM Point Number Of Triangle Vertex 
 long   P2,                // ==> DTM Point Number Of Triangle Vertex
 double zx,                // ==> Start Zero z Coordinate
 double zy,                // ==> Start Zero y Coordinate
 double zz,                // ==> Start Zero z Coordinate
 double startAngle,        // ==> Contour Start Angle
 double radiusMax,         // ==> Distance Normal To Curve Direction Within which a Zero must be found
 double radiusMin,         // ==> If Zero Is Found Within A Distance smaller Than radiusMin, Then step size is multiplied by 2.0
 double radiusMax2,        // ==> Distance Normal To Curve Direction. A Zero Must Be Found Within This Distance When Crossing A Triangle Boundary.
 double posError,          // ==> Permitted Positional Error
 double maxStep,           // ==> Maximum Step Size
 double firstStep,         // ==> Starting Step Size
 double startDifference,   // ==> Difference For Estimating Starting Direction
 double xyTolerance,       // ==> Filtering Tolerance
 double *lzxP,             // <== Last Zero x Coordinate
 double *lzyP,             // <== Last Zero y Coordinate
 double *lzzP,             // <== Last Zero z Coordinate
 long   *exitSideP         // <== Contour Exit Side Of Triangle
) ;
/*
**     Trace Utility Functions
*/
BENTLEYDTM_Private int    bcdtmPolyContour_checkIfPointIsInsideTriangle(double Xp,double Yp,long *Side) ;
BENTLEYDTM_Private int    bcdtmPolyContour_testForVoidHullLineDtmObject(BC_DTM_OBJ *dtmP,long point1,long point2,long *directionP) ;
BENTLEYDTM_Private int    bcdtmPolyContour_testForIslandHullLineDtmObject(BC_DTM_OBJ *dtmP,long point1,long point2,long *directionP) ;
BENTLEYDTM_Private int    bcdtmPolyContour_testForVoidHullPointDtmObject(BC_DTM_OBJ *dtmP,long Point) ;
BENTLEYDTM_Private int    bcdtmPolyContour_testForHullLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2) ;
/*
**     Line Index Functions
*/
BENTLEYDTM_Private int    bcdtmPolyContour_buildLineIndexTable(BC_DTM_OBJ *dtmP,DTM_SMOOTH_CONTOUR_INDEX **contourIndexP) ;
BENTLEYDTM_Private int    bcdtmPolyContour_getLineOffsetInLineIndexTable(DTM_SMOOTH_CONTOUR_INDEX *contourIndexP,long numLines,long P1,long P2,long *Offset) ;
/*
**     Zero Tin Point Table Functions
*/
BENTLEYDTM_Private int    bcdtmPolyContour_buildZeroTinPointTableDtmObject(BC_DTM_OBJ *dtmP,double contourValue,double Eps,double *partDerivP,DTM_SMOOTH_CONTOUR_ZERO_POINT **zeroPtsP,long *numZeroPts) ;
BENTLEYDTM_Private int    bcdtmPolyContour_getScanTrianglesAtZeroTinPointDtmObject(BC_DTM_OBJ *dtmP,long P0,double Eps,double *partDerivP,DTM_SMOOTH_CONTOUR_ZERO_POINT **zeroPtsP,long *numZeroPts,long *MemZeroPTab,long MemZeroPtabInc) ;
BENTLEYDTM_Private int    bcdtmPolyContour_findEntryInZeroTinPointTable(DTM_SMOOTH_CONTOUR_ZERO_POINT *zeroPtsP,long numZeroPts,long P0,long P1,long P2,long *Np1,long *Np2,double *Angle) ;
/*
**     Polynomial Interval And Zero Coordinate Calculation Functions
*/
BENTLEYDTM_Private int    bcdtmPolyContour_calculatePolynomialMinimaAndMaximaForTriangleSidesDtmObject(BC_DTM_OBJ *dtmP,double *partDerivP,double *polyZMin,double *polyZMax,DTM_SMOOTH_CONTOUR_INDEX *contourIndexP) ;
BENTLEYDTM_Private int    bcdtmPolyContour_calculateSideEndPointsAndMinimaMaxima(long Side,double Ts1[6][3],double Z1[6][3],long NumberNintervals[3],double Zmin[3],double Zmax[3]) ;
BENTLEYDTM_Private int    bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinates(long Side,long Zero,double contourValue,double Tzr[5][3],double *Zx,double *Zy,double *Zz) ;
BENTLEYDTM_Private int    bcdtmPolyContour_calculateContourZeroCoordinates(long Side,long Zero,double contourValue,double Tzr[5],double *Zx,double *Zy,double *Zz) ;
BENTLEYDTM_Private int    bcdtmPolyContour_calculateContourZerosForTriangleSide(long Side,double contourValue,long *NumZero,double ZeroValues[]) ;
BENTLEYDTM_Private int    bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinatesAlongTinLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,double contourValue,double Zero,double *Zx,double *Zy,double *Zz) ;
/*
**     Polynomial Math Functions
*/
BENTLEYDTM_Private int    bcdtmPolyContour_calculatePartialDerivativesDtmObject(BC_DTM_OBJ *dtmP,double **partDerivP ) ;
BENTLEYDTM_Private int    bcdtmPolyContour_orderPointsAboutPointDtmObject(BC_DTM_OBJ *dtmP,long P,long Pts[],long *NumPts) ;
BENTLEYDTM_Private int    bcdtmPolyContour_calculatePolynomialCoefficientsForPointTriangleDtmObject(BC_DTM_OBJ *dtmP,long P0,long P1,long P2,double *partDerivP) ;
BENTLEYDTM_Private int    bcdtmPolyContour_calculatePolynomialCoefficientsForTriangle(double *PDV) ;
BENTLEYDTM_Private int    bcdtmPolyContour_interpolatePolynomialTriangle(long NewTrg,double xp,double yp,double *zp,double x[],double y[],double z[],double pd[] ) ;
BENTLEYDTM_Public double  bcdtmPolyContour_evaluatePolynomialFunction(long EvalFunction,double T,long Side) ;
BENTLEYDTM_Public double  bcdtmPolyContour_calculatePolynomialFunctionZero(long EvalFunction,long Side,double TA,double TB,double F1,double F2) ;
BENTLEYDTM_Public double  bcdtmPolyContour_evaluateBivariateFunction(double Xp,double Yp,double T,double Cos,double Sin) ;
BENTLEYDTM_Private int    bcdtmPolyContour_calculateBivariateFunctionZero(double Xp,double Yp,double Cos,double Sin,double TA,double TB,double F1,double F2,double *Zx,double *Zy,double *Zz) ;
BENTLEYDTM_Private int    bcdtmPolyContour_calculateBivariateFunctionZeroBetweenPoints(double X1,double Y1,double X2,double Y2,double *Zx,double *Zy,double *Zz) ;
/*
**     Plotting And Filtering Functions
*/ 
BENTLEYDTM_Private int    bcdtmPolyContour_storeContourPointInCache(double x, double y, double z) ;
BENTLEYDTM_Private int    bcdtmPolyContour_writeContourPoints(FILE **fpDATA,DPoint3d *ContourPts,long NumConPts) ;
BENTLEYDTM_Private int    bcdtmPolyContour_xyzFilterContourPoints(DPoint3d *Points,long *NumPts,double XYTolerance,double ZTolerance) ;
BENTLEYDTM_Private int    bcdtmPolyContour_xyFilterContourPoints(DPoint3d *Points,long *NumPts,double Tolerance) ;
/*
**
** Global Constants For Polynomial Functions
** 
**  x,y,z,       Coordinates Of The Triangle Vertices 
**  DX,DY        Differences Of x and y 
**  DX2,DY2      Differences Of x and y squared 
**  SL           Side Lengths 
**  SL2          Side Lengths Squared 
**  ZT(IV,K)     First Partial Derivative With Respect To Variable IV (U,V,W FOR IV=1,2,3) At Point  K 
**  ZTT          Second Partial Derivative 
**  ZUV          Mixed Partial Derivative 
**  AP,BP,CP,DP  Transformation Constants Derived From DX,DY 
**  P0...P5      Coefficients Of Polynomials Along The Sides
**  Q0...Q4      Coefficients Of 1st Derivative Along The Sides
**  R0...R3      Coefficients Of 2nd Derivative Along The Sides
**  S0...S2      Coefficients Of 3rd Derivative Along The Sides
**  T0...T1      Coefficients Of 4th Derivative Along The Sides
**  P11...P41    Coefficients For Bivariate Polynomial Inside Triangle
**  CL           Value Of Contour Being Plotted 
**
*/
thread_local static double  X[3],Y[3],Z[3],DX[3],DY[3],DX2[3],DY2[3],SL[3],SL2[3],ZT[3][3],ZTT[3][3],ZUV[3],AP,BP,CP,DP ;
thread_local static double  P0[3],P1[3],P2[3],P3[3],P4[3],P5[3] ;
thread_local static double  Q0[3],Q1[3],Q2[3],Q3[3],Q4[3] ;
thread_local static double  R0[3],R1[3],R2[3],R3[3] ;
thread_local static double  S0[3],S1[3],S2[3] ;
thread_local static double  T0[3],T1[3] ;
thread_local static double  P11,P12,P13,P14,P21,P22,P23,P31,P32,P41 ;
thread_local static double  CL ;
thread_local static long    TotalContourPoints=0,TotalFilteredPoints=0 ;
/*
** Contour Points Cache
*/
thread_local static long  NumContourCachePts=0,MemContourCachePts=0,MemContourCachePtsInc=50000 ;
thread_local static DPoint3d   *ContourPtsCacheP=nullptr ; 
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmPolyContour_plotContoursFromDtmFile(WCharCP dtmFileP,long plotRangeContours,long plotSingleContours,double contourInterval,double contourRegistration,double contourMinimum,double contourMaximum,double contourValue[],long numContourValues,DPoint3d *fencePtsP,long numFencePts,DTMFeatureCallback callBackFunctionP,void *userP) 
/*
** See bcdtmPolyContour_plotContoursFromDtmObject For Function Description
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Plotting Smooth Contours From DTM File")  ;
/*
** Test If Requested Tin File Is Current Tin Object
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Plot Smooth Contours
*/
 if( bcdtmPolyContour_plotContoursFromDtmObject(dtmP,plotRangeContours,plotSingleContours,contourInterval,contourRegistration,contourMinimum,contourMaximum,contourValue,numContourValues,fencePtsP,numFencePts,callBackFunctionP,userP) != DTM_SUCCESS ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Plotting Smooth Contours From DTM File Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Plotting Smooth Contours From DTM File Error")  ;
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
BENTLEYDTM_EXPORT int bcdtmPolyContour_plotContoursFromDtmObject
(
 BC_DTM_OBJ *dtmP,
 long   plotRangeContours,
 long   plotSingleContours,
 double contourInterval,
 double contourRegistration,
 double contourMinimum,
 double contourMaximum,
 double contourValues[],
 long   numContourValues,
 DPoint3d    *fencePtsP,
 long   numFencePts,
 DTMFeatureCallback callBackFunctionP, 
 void   *userP        
) 
/*
** This Is The Entry Point Function For Plotting Polynomial Contours From A Tin
**
** Arguements
**
**  ==> dtmP                  = Pointer To DTM Object 
**  ==> plotRangeContours     = Plot A Range Of Contours Between contourMinimum And contourMaximum
**                            = 0 Do Not Plot Contour Range
**                            = 1 Plot Contour Range
**  ==> plotSingleContours    = Plot Single Value Contours Specified In contourValue
**                            = 0 Do Not Plot Single Contours
**                            = 1 Plot Single Contours 
**  ==> contourInterval       = Range Contour Interval Must be geater than zero.
**  ==> contourRegistration   = Range Contour Registration. All plotted Contours are a multiple of contourInterval from contourRegistration.
**  ==> contourMinimum        = Range Minimum Contour Value
**  ==> contourMaximum        = Range Maximum Contour Value
**  ==> contourValue         = An array of numContourValues Single Contour Values To be plotted
**  ==> numContourValues      = Number of Single Contour Values to be plotted
**  ==> fencePtsP             = DPoint3d Array of the points for Clipping Tin Prior To Contouring
**  ==> numFencePts           = Number Of Fence Points
**                            = 0  Do Not Clip Tin
**                            > 0  Clip Tin
**  ==> callBackFunctionP     =  Pointer To Call Back Function
**  ==> userP                 =  User pointer Passed Back To The call Back Function
**
** Return Value = DTM_SUCCESS = Success , DTM_ERROR = Error
** 
** Parameter Validation - Complete
** Visibility           - dtmPublic
** Author               - Rob Cormack
** Date                 - 9th August 2001
**
*/
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long      i,lr,level,voidsInDTM,contoursInDTM,dtmFeature ;
 double    *partDerivP=nullptr,bottomContour,topContour,cv,contourValue,cf,cr,polyZMin,polyZMax ;
 double    Fstep,Eps,Rmax,XYTolerance,ZTolerance ;
 long      startTime,contourStartTime ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ    *tempDtmP=nullptr ;
 DTM_SMOOTH_CONTOUR_INDEX *contourIndexP=nullptr ;
/*
** Write Status Message
*/
 contourStartTime = bcdtmClock() ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Plotting Polynomial Contours From DTM Object")  ;
    bcdtmWrite_message(0,0,0,"dtmP                  =  %p",dtmP) ; 
    bcdtmWrite_message(0,0,0,"plotRangeContours     =  %1ld",plotRangeContours) ;
    bcdtmWrite_message(0,0,0,"plotSingleContours    =  %1ld",plotSingleContours) ;
    bcdtmWrite_message(0,0,0,"Contour Interval      =  %10.4lf",contourInterval) ; 
    bcdtmWrite_message(0,0,0,"Contour Registration  =  %10.4lf",contourRegistration) ; 
    bcdtmWrite_message(0,0,0,"Contour Minimum       =  %10.4lf",contourMinimum) ; 
    bcdtmWrite_message(0,0,0,"Contour Maximum       =  %10.4lf",contourMaximum) ; 
    bcdtmWrite_message(0,0,0,"callBackFunctionP     =  %p",callBackFunctionP) ; 
    bcdtmWrite_message(0,0,0,"userP                 =  %p",userP) ; 
    bcdtmWrite_message(0,0,0,"Number Contour Values =  %5ld",numContourValues) ; 
    if( numContourValues > 0 ) { for( i = 0 ; i < numContourValues ; ++i ) bcdtmWrite_message(0,0,0,"Contour Value = %10.4lf",contourValues[i]) ; }
    bcdtmWrite_message(0,0,0,"Number Of Fence Points    =  %5ld",numFencePts) ; 
    if( numFencePts > 0 ) { for( i = 0 ; i < numFencePts ; ++i ) bcdtmWrite_message(0,0,0,"Fence Point[%4ld] = %10.4lf %10.4lf",i,(fencePtsP+i)->x,(fencePtsP+i)->y) ; }
   } 
   
/*
** Set Floating Point Precision
*/
/*
 bcdtmWrite_message(0,0,0,"I1 - FCW = 0x%.4x ",_control87(0,0)) ;
 _control87(_PC_64,MCW_PC) ;  
 bcdtmWrite_message(0,0,0,"I2 - FCW = 0x%.4x ",_control87(0,0)) ;
*/
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
** Validate  Parameters
*/
 if( plotRangeContours  < 0 || plotRangeContours  > 1 ) { bcdtmWrite_message(1,0,0,"Invalid Value For Plot Range Contours")  ; goto errexit ; }
 if( plotSingleContours < 0 || plotSingleContours > 1 ) { bcdtmWrite_message(1,0,0,"Invalid Value For Plot Single Contours") ; goto errexit ; }
 if( numFencePts        < 0 ) { bcdtmWrite_message(0,0,0,"Invalid Value For Number Of Clip Points") ; goto errexit ; }
 if( plotRangeContours )
   {
    if( contourInterval <= 0.0  ) { bcdtmWrite_message(0,0,0,"Invalid Value For Contour Interval") ; goto errexit ; }
    if( contourMinimum  >= contourMaximum ) { bcdtmWrite_message(0,0,0,"Invalid Value For Contour Minimum And/Or Contour Maximum") ; goto errexit ; }
   }
 if( plotSingleContours  && numContourValues <= 0  ) { bcdtmWrite_message(1,0,0,"Invalid Value For Number Of Single Contours") ; goto errexit ; }
 if( numContourValues    && contourValues == nullptr  ) { bcdtmWrite_message(1,0,0,"Single Contour Value Array nullptr") ; goto errexit ; }
 if( numFencePts         && fencePtsP == nullptr      ) { bcdtmWrite_message(1,0,0,"Fence Point Array nullptr") ; goto errexit ; }
 if( ! plotRangeContours && ! plotSingleContours   ) { bcdtmWrite_message(1,0,0,"No Contour Values Specified For Plotting") ; goto errexit ; }
/*
**  Test For And Clip Dtm Prior To Contouring
*/
 if( numFencePts > 0 && fencePtsP != nullptr )
   {
    if( dbg  )  bcdtmWrite_message(0,0,0,"Clipping DTM") ; 
    startTime = bcdtmClock() ; 
    if( bcdtmClip_cloneAndClipToPolygonDtmObject(dtmP,&tempDtmP,fencePtsP,numFencePts,DTMClipOption::External)) goto errexit ;
    if( dbg  )  bcdtmWrite_message(0,0,0,"Time To Clip Dtm = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
    if( cdbg ) bcdtmWrite_toFileDtmObject(tempDtmP,L"smoothContour.dtm") ;
   }
 else tempDtmP = dtmP ;
/*
**  Test For Voids, Islands, Holes And Contours In The DTMFeatureState::Tin
*/
 voidsInDTM = contoursInDTM = 0 ;
 for( dtmFeature = 0 ; dtmFeature < tempDtmP->numFeatures && ! voidsInDTM  && ! contoursInDTM ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole ) voidsInDTM   = 1 ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::ContourLine ) contoursInDTM = 1 ;
      }
   }
/*
** Calculate Partial Derivatives
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Partial Derivatives") ; 
 startTime = bcdtmClock() ;
 if( bcdtmPolyContour_calculatePartialDerivativesDtmObject(tempDtmP,&partDerivP)) goto cleanup ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Calculate Partial Derivatives = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ; 
/*
** Build Line Index Table For Contour Tracing Purposes
*/
 if( dbg )  bcdtmWrite_message(0,0,0,"Building Line Index Table") ; 
 startTime = bcdtmClock() ;
 if( bcdtmPolyContour_buildLineIndexTable(tempDtmP,&contourIndexP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Time To Build Line Index Table = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Calculate Polynomial Minina And Maxima z Values For All Triangle Sides
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Polynomial Minima And Maxima For All Triangles") ; 
 startTime = bcdtmClock() ;
 if( bcdtmPolyContour_calculatePolynomialMinimaAndMaximaForTriangleSidesDtmObject(tempDtmP,partDerivP,&polyZMin,&polyZMax,contourIndexP)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tin  Zmin = %10.4lf Zmax = %10.4lf",tempDtmP->zMin,tempDtmP->zMax) ;
    bcdtmWrite_message(0,0,0,"Poly Zmin = %10.4lf Zmax = %10.4lf",polyZMin,polyZMax) ;
    bcdtmWrite_message(0,0,0,"Time To Calculate Polynomial z Minima And Maxima = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ; 
   }
/*
** Set Contour Ranges To Polynomial z Minima And Maxima
*/
 if( contourMinimum < polyZMin )  contourMinimum = polyZMin ;
 if( contourMaximum > polyZMax )  contourMaximum = polyZMax ;
/*
** Set Contour Tracing Parameters
*/
 Fstep = 0.5   ;
 Rmax  = 0.01  ;
 Eps   = 0.001 ;
 XYTolerance = 0.001 ;
 ZTolerance  = 0.01 ;
/*
** Plot Range Of Contours From contourMinimum To contourMaximum
*/
 if( plotRangeContours )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Plotting Range Contours") ; 
    startTime = bcdtmClock() ;
    level = (long) ((contourMinimum - contourRegistration) / contourInterval ) ;
    bottomContour = contourRegistration + ((double) level) * contourInterval ;
    level = (long) ((contourMaximum - contourRegistration) / contourInterval ) + 1 ;
    topContour = contourRegistration + ((double) level) * contourInterval ;
    for ( cv = bottomContour ; cv < topContour ; cv = cv + contourInterval )
      {
/*
**     Round Contour Value
*/
       cf = floor(cv) ;
       cr = cv - cf   ;
       lr = (long)(cr * 1000000.0) ;
       lr = ( lr + 5 ) / 10 ;
       contourValue = cf + (double)(lr) / 100000.0 ; 
/*
**     Plot Contours
*/     
       if( contourValue >= polyZMin && contourValue <= polyZMax ) 
         {
          if( bcdtmPolyContour_scanForContourDtmObject(tempDtmP,contourValue,Eps,Fstep,Rmax,XYTolerance,ZTolerance,contourIndexP,partDerivP,callBackFunctionP,userP) ) goto errexit ;
         } 
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Plot Range Contours = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ; 
   }
/*
** Plot Single Valued Contours
*/
 if( plotSingleContours )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Plotting Single Value Contours") ; 
    startTime = bcdtmClock() ;
    for ( i = 0 ; i < numContourValues ; ++i ) 
      {
       if( contourValues[i] >= polyZMin && contourValues[i] <= polyZMax ) 
         {
          if( bcdtmPolyContour_scanForContourDtmObject(tempDtmP,contourValues[i],Eps,Fstep,Rmax,XYTolerance,ZTolerance,contourIndexP,partDerivP,callBackFunctionP,userP )) goto errexit ;
         }
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Time To Plot Single Value Contours = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ; 
   }
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Total Contour  Points = %8ld",TotalContourPoints) ;
    bcdtmWrite_message(0,0,0,"Total Filtered Points = %8ld",TotalFilteredPoints) ;
    bcdtmWrite_message(0,0,0,"Total Time To Plot Smooth Contours From DTM = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),contourStartTime)) ; 
   }
/*
** Clean Up
*/ 
 cleanup :
 NumContourCachePts = 0 ;
 MemContourCachePts = 0 ;
 if( ContourPtsCacheP != nullptr ) free(ContourPtsCacheP) ; 
 if( contourIndexP    != nullptr ) free(contourIndexP) ;
 if( partDerivP       != nullptr ) free(partDerivP) ; 
 if( tempDtmP         != nullptr && tempDtmP != dtmP ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Plotting Polynomial Contours From Tin Object Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Plotting Polynomial Contours From Tin Object Error")  ;
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
BENTLEYDTM_Private int bcdtmPolyContour_scanForContourDtmObject
(
 BC_DTM_OBJ *dtmP,
 double contourValue,
 double Eps,
 double Fstep,
 double Rmax,
 double XYTolerance,
 double ZTolerance,
 DTM_SMOOTH_CONTOUR_INDEX *contourIndexP,
 double *partDerivP,
 DTMFeatureCallback callBackFunctionP, 
 void *userP 
)
/*
** This Routine Scans The Tin Points And Tin Edges To Detect The Start Of A Contour Line
**
** Arguements
**  
** Tin            ==> Tin Object
** contourValue   ==> Contour Value To Be Plotted
** contourIndexP  ==> Smooth Contour Line Index Table 
**
** Return Value = 0 Success , 1 Error
**
** Validation - Nil
** Visibility - dtmPrivate
**
** Author     - Rob Cormack 
** Date       - 10 August 2001 
**
*/
{
 int        ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_TRACE_VALUE(0) ;
 long       p0,p1,p2,numLines,nz,numZeroPts,dtmFeature ;
 double     Zx,Zy,Zz,LineZ,DeveZ,ProdZ,Pd[15],StartAngle ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_SMOOTH_CONTOUR_INDEX      *indexP,*startLineP,*lastLineP,*scanLineP ; 
 DTM_SMOOTH_CONTOUR_ZERO_POINT *zptsP,*zeroPtsP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For %10.4lf Contour Start",contourValue) ;
/*
** Initialise
*/
 numLines = dtmP->numLines  ;
 startLineP = lastLineP = nullptr ;
/*
** Scan Tin Lines And Calculate Contour Zeros
*/
 for( indexP = contourIndexP ; indexP < contourIndexP + numLines ; ++indexP )
   {
    if( indexP->Type && contourValue >= indexP->Zmin && contourValue <= indexP->Zmax )
      {
/*
**     Set Linked List Into Structure For Subsequent Scanning 
*/
       if( startLineP == nullptr )  { startLineP = lastLineP = indexP ; lastLineP->Next = nullptr ;  }
       else                      { lastLineP->Next = indexP ; lastLineP = indexP ; lastLineP->Next = nullptr ;} 
/*
**    Get Triangle Points For Line
*/ 
       p0 = indexP->P1 ;
       p1 = indexP->P2 ;
       if( indexP->Type < 0  ) { if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p0,p1)) < 0 ) goto errexit ; }
       else                    { if(( p2 = bcdtmList_nextClkDtmObject(dtmP,p0,p1)) < 0 ) goto errexit ; }
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"p0 = %6ld p1 = %6ld p2 = %6ld Type = %1ld Zmin = %10.4lf Zmax = %10.4lf",p0,p1,p2,indexP->Type,indexP->Zmin,indexP->Zmax) ;
          bcdtmWrite_message(0,0,0,"p0 = %6ld ** %10.4lf %10.4lf %10.4lf",p0,pointAddrP(dtmP,p0)->x,pointAddrP(dtmP,p0)->y,pointAddrP(dtmP,p0)->z) ;
          bcdtmWrite_message(0,0,0,"p1 = %6ld ** %10.4lf %10.4lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
         }
/*
**     Calculate Polynomial Coefficients For Triangle So That Triangle Side 0 Is P0,P1
*/
       if( bcdtmPolyContour_calculatePolynomialCoefficientsForPointTriangleDtmObject(dtmP,p2,p1,p0,partDerivP)) goto errexit ;
/*
**     Calculate Contour Zero For Triangle Side 0
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Contour %10.4lf Zeros For Side %6ld %6ld",contourValue,p0,p1) ;
       if( bcdtmPolyContour_calculateContourZerosForTriangleSide(0,contourValue,&indexP->NumZero,indexP->Zeros)) goto errexit ;
/*
**     Check Zero Calculations Against Polynomial Functions  ** Developemnt Only
*/
       if( cdbg )
         {
          for( nz = 0 ; nz < indexP->NumZero ; ++nz )
            { 
             bcdtmPolyContour_calculateContourZeroCoordinates(0,nz,contourValue,indexP->Zeros,&Zx,&Zy,&Zz) ;
             LineZ = bcdtmPolyContour_evaluatePolynomialFunction(5,indexP->Zeros[nz],0) ;
             DeveZ = bcdtmPolyContour_evaluateBivariateFunction(Zx,Zy,0.0,0.0,0.0) ; 
             bcdtmLattice_getPartialDerivatives(p2,p1,p0,partDerivP,Pd) ;
             bcdtmPolyContour_interpolatePolynomialTriangle(1,Zx,Zy,&ProdZ,X,Y,Z,Pd) ;
             if( fabs(contourValue-LineZ) > 0.00001 || fabs(contourValue-DeveZ) > 0.00001 || fabs(contourValue-ProdZ) > 0.00001) 
               {
                bcdtmWrite_message(1,0,0,"Error With Contour Zero Coordinates") ;
                bcdtmWrite_message(0,0,0,"Zero  Coordinates = %10.4lf %10.4lf %10.4lf",Zx,Zy,Zz) ;
                bcdtmWrite_message(0,0,0,"Line  Coordinates = %10.4lf %10.4lf %10.4lf",Zx,Zy,LineZ) ;
                bcdtmWrite_message(0,0,0,"Deve  Coordinates = %10.4lf %10.4lf %10.4lf",Zx,Zy,DeveZ) ;
                bcdtmWrite_message(0,0,0,"Prod  Coordinates = %10.4lf %10.4lf %10.4lf",Zx,Zy,ProdZ) ;
                goto errexit ;
               }
            } 
         }
      }
   }
/*
** Build Zero Tin Point Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Zero Point Table") ;
 if( bcdtmPolyContour_buildZeroTinPointTableDtmObject(dtmP,contourValue,Eps,partDerivP,&zeroPtsP,&numZeroPts)) goto errexit ;
/*
** Remove Redundant Tin Point Zeros From Line Index Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Redundant Tin Point Zeros") ;
 if( numZeroPts > 0 )
   {
    scanLineP = startLineP ;
    while ( scanLineP != nullptr )
      {
       if( scanLineP->NumZero > 0 && scanLineP->Zeros[0] == 0.0 )
         {
          for( nz = 0 ; nz < scanLineP->NumZero - 1 ; ++nz ) scanLineP->Zeros[nz] = scanLineP->Zeros[nz+1] ;
          --scanLineP->NumZero ;
         }
       if( scanLineP->NumZero > 0 && scanLineP->Zeros[scanLineP->NumZero-1] == 1.0 ) --scanLineP->NumZero ;
       scanLineP = scanLineP->Next ;
      }
   }
/*
** Plot Contours Starting At Zero Tin Points On Tin Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Plotting Contours Starting At Hull Zero Tin Points") ;
 p0 = dtmP->hullPoint ;
 do
   {
    if( pointAddrP(dtmP,p0)->z == contourValue && ! bcdtmPolyContour_testForVoidHullPointDtmObject(dtmP,p0) )
      {
       if( bcdtmPolyContour_findEntryInZeroTinPointTable(zeroPtsP,numZeroPts,p0,dtmP->nullPnt,dtmP->nullPnt,&p1,&p2,&StartAngle)) goto errexit ;
       while ( p1 != dtmP->nullPnt )
         {
          if( bcdtmPolyContour_traceContourDtmObject(dtmP,contourValue,p0,p1,p2,1,StartAngle,XYTolerance,ZTolerance,nullptr,contourIndexP,numLines,zeroPtsP,numZeroPts,partDerivP,callBackFunctionP,userP) ) goto errexit  ;
          if( bcdtmPolyContour_findEntryInZeroTinPointTable(zeroPtsP,numZeroPts,p0,dtmP->nullPnt,dtmP->nullPnt,&p1,&p2,&StartAngle)) goto errexit ;
         }
      }
    p0 = nodeAddrP(dtmP,p0)->hPtr ;  
   } while ( p0 != dtmP->hullPoint ) ;
/*
** Plot Contours Starting At Zero Tin Points On Feature Hulls
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Plotting Contours Starting On A Hull Feature ") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void ||  dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole ||  dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
         {
          p0 = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do
            {  
             if( pointAddrP(dtmP,p0)->z == contourValue )
               {
                if( bcdtmPolyContour_findEntryInZeroTinPointTable(zeroPtsP,numZeroPts,p0,dtmP->nullPnt,dtmP->nullPnt,&p1,&p2,&StartAngle)) goto errexit ;
                while ( p1 != dtmP->nullPnt )
                  {
                   if( bcdtmPolyContour_traceContourDtmObject(dtmP,contourValue,p0,p1,p2,1,StartAngle,XYTolerance,ZTolerance,nullptr,contourIndexP,numLines,zeroPtsP,numZeroPts,partDerivP,callBackFunctionP,userP) ) goto errexit  ;
                   if( bcdtmPolyContour_findEntryInZeroTinPointTable(zeroPtsP,numZeroPts,p0,dtmP->nullPnt,dtmP->nullPnt,&p1,&p2,&StartAngle)) goto errexit ;
                  }
               }
             p1 = p0 ; 
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,p1,&p0) ) goto errexit ;
            } while ( p0 != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
         }
      }
   } 
/*
** Scan Tin Lines And Trace Contours Starting On The Tin Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Plotting Contours Starting On Tin Hull Lines") ;
 scanLineP = startLineP ;
 while ( scanLineP != nullptr )
   {
/*
** Test For Contour Start On Tin Hull
*/
    if( labs(scanLineP->Type) == 2 )  
      {   
       p0 = scanLineP->P1 ;
       p1 = scanLineP->P2 ;
       if( scanLineP->Type < 0  ) { if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p0,p1)) < 0 ) goto errexit ; }
       else                       { if(( p2 = bcdtmList_nextClkDtmObject(dtmP,p0,p1)) < 0 ) goto errexit ; }
       while ( scanLineP->NumZero > 0 )
         {
          if( bcdtmPolyContour_traceContourDtmObject(dtmP,contourValue,p0,p1,p2,2,0.0,XYTolerance,ZTolerance,scanLineP,contourIndexP,numLines,zeroPtsP,numZeroPts,partDerivP,callBackFunctionP,userP) ) goto errexit  ;
         }
      } 
    scanLineP = scanLineP->Next ;
   }
/*
** Scan Tin Lines And Trace Contours Starting Internal To Tin Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Plotting Contours Starting On Internal Tin Lines") ;
 scanLineP = startLineP ;
 while ( scanLineP != nullptr )
   {
/*
** Test For Contour Start On An Internal Tin Line
*/
    if( labs(scanLineP->Type) == 1 )  
      {   
       p0 = scanLineP->P1 ;
       p1 = scanLineP->P2 ;
       if(( p2 = bcdtmList_nextClkDtmObject(dtmP,p0,p1)) < 0 ) goto errexit ;        while ( scanLineP->NumZero > 0 )
       while ( scanLineP->NumZero > 0 )
         {
          if( bcdtmPolyContour_traceContourDtmObject(dtmP,contourValue,p0,p1,p2,2,0.0,XYTolerance,ZTolerance,scanLineP,contourIndexP,numLines,zeroPtsP,numZeroPts,partDerivP,callBackFunctionP,userP) ) goto errexit  ;
         }
      } 
    scanLineP = scanLineP->Next ;
   }
/*
** Scan And Null Out Tin Line Linked List Pointers
*/
 while ( startLineP != nullptr )
   {
    scanLineP = startLineP->Next ;
    startLineP->Next = nullptr  ;
    startLineP = scanLineP ;
   }
/*
** Plot Contours Starting At Internal Zero Tin Points 
*/
 for( zptsP = zeroPtsP ; zptsP < zeroPtsP + numZeroPts ; ++zptsP )
   {
    if( zptsP->P1 != dtmP->nullPnt && ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,zptsP->P1)->PCWD) )
      {
       p0 = zptsP->P0 ; p1 = zptsP->P1 ; p2 = zptsP->P2 ;
       zptsP->P1 = zptsP->P2 = dtmP->nullPnt ; 
       if( bcdtmPolyContour_traceContourDtmObject(dtmP,contourValue,p0,p1,p2,-1,StartAngle,XYTolerance,ZTolerance,scanLineP,contourIndexP,numLines,zeroPtsP,numZeroPts,partDerivP,callBackFunctionP,userP) ) goto errexit  ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( zeroPtsP != nullptr ) { free(zeroPtsP) ; zeroPtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning For %10.4lf Contour Start Completed",contourValue) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning For %10.4lf Contour Start Error",contourValue) ;
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
BENTLEYDTM_Private int bcdtmPolyContour_calculateContourZerosForTriangleSide(long Side,double contourValue,long *NumZero,double ZeroValues[])
/*
** This Function Calculates The Contour Zero's ( if any ) For a Triangle Side
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_TRACE_VALUE(0) ;
 long   i,j,NumInterval[3] ;
 double F1,F2,F1F2 ;
 double Tzr[6][3],Vzr[6][3],Zmax[3],Zmin[3] ; 
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Contour Zeros For Side %1ld",Side) ;
/*
** Initialise
*/
 *NumZero = 0 ;
 for( j = 0 ; j < 5 ; ++j ) ZeroValues[j] = 0.0 ;
/*
** Calculate Intervals And z Minima And Maxima For Side
*/
 bcdtmPolyContour_calculateSideEndPointsAndMinimaMaxima(Side,Tzr,Vzr,NumInterval,Zmin,Zmax) ;
/*
** Write Calcs ** Developement Purposes Only
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Side = %1ld Number Of Intervals = %1ld Zmin = %10.4lf Zmax = %10.4lf",Side,NumInterval[Side],Zmin[Side],Zmax[Side]) ;
    for( j = 0 ; j <= NumInterval[Side] ; ++j ) bcdtmWrite_message(0,0,0,"Tzr = %10.4lf Vzr = %10.4lf",Tzr[j][Side],Vzr[j][Side]) ;
   }
/*
** Check For Contour Value Between Side Minima and Maxima ** Developement Only
*/
 if( cdbg )
   {
    if( contourValue < Zmin[Side] ) { bcdtmWrite_message(0,0,0,"Contour %10.4lf Less    Than Side Min %10.4lf",contourValue,Zmin[Side]) ; goto errexit ; }  
    if( contourValue > Zmax[Side] ) { bcdtmWrite_message(0,0,0,"Contour %10.4lf Greater Than Side Max %10.4lf",contourValue,Zmax[Side]) ; goto errexit ; }  
   }
/*
**  Check Intervals For Contour Zero's 
*/
 CL = contourValue ;
 for( j = 0 ; j < NumInterval[Side] ; ++j )
   {
    F1   = Vzr[j][Side]   - contourValue ;
    F2   = Vzr[j+1][Side] - contourValue ;
    F1F2 = F1*F2 ;
/*
**  Calculate Contour Zero For Side
*/
    if( F1F2 <  0.0 ) 
      {
       ZeroValues[*NumZero] = bcdtmPolyContour_calculatePolynomialFunctionZero(5,Side,Tzr[j][Side],Tzr[j+1][Side],F1,F2) ;
       if( ZeroValues[*NumZero]       < 0.0000001 ) ZeroValues[*NumZero] = 0.0 ;
       if( 1.0 - ZeroValues[*NumZero] < 0.0000001 ) ZeroValues[*NumZero] = 1.0 ;
       ++*NumZero ;
      }
/*
**  Special Conditions
*/
    else if( F1F2 == 0.0  )
      {
       if( dbg ) 
         {
          bcdtmWrite_message(0,0,0,"Ni = %ld Special Condition F1 = %8.5lf F2 = %8.5lf",NumInterval[Side],F1,F2) ;
          for( i = 0 ; i < NumInterval[Side] + 1 ; ++i )  bcdtmWrite_message(0,0,0,"Interval[%1ld] = %8.5lf",i,Tzr[i][Side]) ;
         }
/*
**   Check For Contour Coincident With Triangle Side Or Triangle Point
*/  
       if( NumInterval[Side] == 1 )
         {
          if( F1 == 0.0 ) { ZeroValues[*NumZero] = 0.0 ; ++*NumZero ; }
          if( F2 == 0.0 ) { ZeroValues[*NumZero] = 1.0 ; ++*NumZero ; }
         }
       else 
         {
          if( j == 0 && F1 == 0.0 ) { ZeroValues[*NumZero] = Tzr[j][Side]   ; ++*NumZero ; }  
          if( j  > 0 && F2 == 0.0 ) { ZeroValues[*NumZero] = Tzr[j+1][Side] ; ++*NumZero ; }
         } 
      }
   }
/*
** Write Out Zeros Development Only
*/ 
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Number Zeros Side %1ld = %2ld",Side,*NumZero) ;
    for( i = 0 ; i < *NumZero ; ++i) bcdtmWrite_message(0,0,0,"Zero[%1ld] = %12.10lf",i,ZeroValues[i]) ; 
   }
/*
** Clean Up
*/
 cleanup :
 CL = 0.0 ;
/*
** job Completed
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
BENTLEYDTM_Private int bcdtmPolyContour_traceContourDtmObject
(
 BC_DTM_OBJ *dtmP, 
 double     contourValue,
 long       P0,
 long       P1,
 long       P2,
 long       contourStartType,
 double     contourDirection,
 double     XYTolerance,
 double     ZTolerance,
 DTM_SMOOTH_CONTOUR_INDEX *contourLineP,
 DTM_SMOOTH_CONTOUR_INDEX *contourIndexP,
 long       numLines,
 DTM_SMOOTH_CONTOUR_ZERO_POINT *zeroPtsP,
 long       numcontourZeroPts,
 double     *partDerivP,
 DTMFeatureCallback callBackFunctionP,
 void       *userP
)
/*
** This Function Traces A Contour Starting On Triangle Point P0
** Or From A Triangle Edge P0P1 Of Triangle P0P1P2
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_TRACE_VALUE(0) ;
 long   i,startFlag=1,tinTrace,exitSide,closeFlag,direction ;
 double Sx,Sy,Sz,Zx,Zy,Zz,chkZ,contourZero,contourStartDirection ;
 double d0,d1,d2,hMin,slMax ;
 double eps,rMin,rMax,rMax2,posError,maxStep,firstStep ;
/*
** Write Status Message
*/
 if( dbg && contourStartType == 1 ) bcdtmWrite_message(0,0,0,"Tracing Contour = %10.4lf Start Type = %1ld Start Angle = %12.10lf",contourValue,contourStartType,contourDirection) ;
 if( dbg && contourStartType == 2 ) bcdtmWrite_message(0,0,0,"Tracing Contour = %10.4lf Start Type = %1ld Start Line  = %6ld",contourValue,contourStartType,(long)(contourLineP-contourIndexP)) ;
 if( contourStartType == 1 ) bcdtmWrite_message(0,0,0,"Tracing Contour = %10.4lf Start Type = %1ld Start Angle = %12.10lf",contourValue,contourStartType,contourDirection) ;
 if( contourStartType == 2 ) bcdtmWrite_message(0,0,0,"Tracing Contour = %10.4lf Start Type = %1ld Start Line  = %6ld",contourValue,contourStartType,(long)(contourLineP-contourIndexP)) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Contour") ;
    bcdtmWrite_message(0,0,0,"Trace Start Triangle %6ld %6ld %6ld",P0,P1,P2) ;
    bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",P0,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,pointAddrP(dtmP,P0)->z) ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
    bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
   } 
/*
** Get Contour Start Coordinates For Contour Start At Point P0
*/
 if( contourStartType == 1 || contourStartType == -1 )
   {
    if( contourStartType == -1 ) closeFlag = 1 ;
    contourStartType = 1 ; 
    Zx = pointAddrP(dtmP,P0)->x ;
    Zy = pointAddrP(dtmP,P0)->y ;
    Zz = pointAddrP(dtmP,P0)->z ;
    contourStartDirection = contourDirection ;
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Contour Start At Point %6ld",P0) ;
       bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",P0,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,pointAddrP(dtmP,P0)->z) ;   
       bcdtmWrite_message(0,0,0,"contourZero Coordinates = %10.4lf %10.4lf %10.4lf",Zx,Zy,Zz) ;
      }  
   }
/*
** Get Zero For Start On Tin Line
*/
 if( contourStartType == 2 )
   {
    direction = contourLineP->Type ;
/*
**  Set Close Flag For Contour Starting From An Internal Tin Line
*/
    closeFlag = 0 ;
    if( contourLineP->Type == 1 ) closeFlag = 1 ;
/*
**  Get Contour Start Zero For P0P1
*/ 
    if( contourLineP->NumZero <= 0 ) 
      { 
       bcdtmWrite_message(1,0,0,"No Contour Zeros For Start Line") ;
       goto errexit ; 
      }
    contourZero = contourLineP->Zeros[0] ;
    if( dbg ) bcdtmWrite_message(0,0,0,"contourZero = %10.8lf",contourZero) ;
/*
**  Remove Contour Zero From Contour Zero List For Line P0P1
*/
    for( i = 0 ; i < contourLineP->NumZero - 1 ; ++i ) contourLineP->Zeros[i] = contourLineP->Zeros[i+1] ;
    --contourLineP->NumZero ;
/*
** Get Start Coordinates For Contour Start On Line
*/
    bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinatesAlongTinLineDtmObject(dtmP,P0,P1,contourValue,contourZero,&Zx,&Zy,&Zz) ;
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Contour Start On Line %6ld %6ld",P0,P1) ;
       bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",P0,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,pointAddrP(dtmP,P0)->z) ;   
       bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;   
       bcdtmWrite_message(0,0,0,"contourZero Coordinates = %10.4lf %10.4lf %10.4lf",Zx,Zy,Zz) ;
      }  
   } 
/*
** Trace Contour Over Tin Until Contour Ends
*/
 tinTrace = 1 ;
 if( ! contourStartType ) tinTrace = 0 ;
 while( tinTrace )
   {
/*
**  Write Triangle Coordinates ** Development Only
*/
    if( dbg )
     {
      bcdtmWrite_message(0,0,0,"Tracing Over Triangle %6ld %6ld %6ld",P0,P1,P2) ;
      bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",P0,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,pointAddrP(dtmP,P0)->z) ;
      bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
      bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
      bcdtmWrite_message(0,0,0,"Start Type = %1ld",contourStartType) ;
     } 
/*
**  Calculate Distance Of Each Triangle Vertex To Its Opposite Side
*/
   d0 = bcdtmMath_normalDistanceToLineDtmObject(dtmP,P0,P1,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y) ;
   d1 = bcdtmMath_normalDistanceToLineDtmObject(dtmP,P1,P2,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y) ;
   d2 = bcdtmMath_normalDistanceToLineDtmObject(dtmP,P2,P0,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y) ;
   hMin = d0 ;
   if( d1 < hMin ) hMin = d1 ;
   if( d2 < hMin ) hMin = d2 ;
   if( dbg ) bcdtmWrite_message(0,0,0,"hMin  = %12.5lf ** d1 = %12.5lf d2 = %12.5lf d3 = %12.5lf",hMin,d0,d1,d2) ;
/*
** Calculate Maximum Side Length
*/ 
   d0 = bcdtmMath_pointDistanceDtmObject(dtmP,P0,P1) ;
   d1 = bcdtmMath_pointDistanceDtmObject(dtmP,P1,P2) ;
   d2 = bcdtmMath_pointDistanceDtmObject(dtmP,P2,P0) ;
   slMax = d0 ;
   if( d1 > slMax ) slMax = d1 ;
   if( d2 > slMax ) slMax = d2 ;
   if( dbg ) bcdtmWrite_message(0,0,0,"slMax = %12.5lf ** d1 = %12.5lf d2 = %12.5lf d3 = %12.5lf",slMax,d0,d1,d2) ;
/*
** Set Contour Tracing Parameters
*/
   rMax         = hMin * 0.01 ;               // Distance Normal To Curve Direction Within which a Zero must be found
   rMin         = rMax * 0.2  ;               // If Zero Is Found Within A Distance smaller Than rMin, Then step size is multiplied by 2.0
   posError     = 0.001 * hMin*hMin / slMax ; // Permitted Positional Error
   maxStep      = hMin * 0.2  ;               // Maximum Step Size
   firstStep    = rMax * 10.0 ;               // Starting Step Size
   rMax2        = rMax * 2.0  ;               // Distance Normal To Curve Direction. A Zero Must Be Found Within This Distance When Crossing A Triangle Boundary.
   eps          = hMin * 0.01 ;               // Difference For Estimating Starting Direction
   if( dbg )
     {
      bcdtmWrite_message(0,0,0,"rmax = %10.5lf rMin = %10.5lf rMax2 = %10.5lf",rMax,rMin,rMax2) ;
      bcdtmWrite_message(0,0,0,"eps  = %10.5lf posError = %10.5lf",eps,posError) ;
      bcdtmWrite_message(0,0,0,"maxStep  = %10.5lf firstStep = %10.5lf",maxStep,firstStep) ;
     }
/*
**  Calculate Polynomial Coefficients For Triangle So That Triangle exitSide 0 Is P0,P1
*/
    if( bcdtmPolyContour_calculatePolynomialCoefficientsForPointTriangleDtmObject(dtmP,P2,P1,P0,partDerivP)) goto errexit ;
/*
**  Check Contour Zero Coordiantes
*/
    if( cdbg )
      { 
       chkZ = bcdtmPolyContour_evaluateBivariateFunction(Zx,Zy,0.0,0.0,0.0)  ; 
       if( fabs(chkZ-Zz) > 0.00001 )
         {
          bcdtmWrite_message(1,0,0,"Error With Start Contour contourZero Coordinates") ;
          bcdtmWrite_message(0,0,0,"contourZero = %10.4lf %10.4lf %10.4lf",Zx,Zy,Zz) ;
          bcdtmWrite_message(0,0,0,"Chkz        = %10.4lf %10.4lf %10.4lf",Zx,Zy,chkZ) ;
          goto errexit ;
         }
      }
/*
**  Plot Start Of Contour
*/
    if( bcdtmPolyContour_storeContourPointInCache(Zx,Zy,Zz) ) goto errexit ;   
    if( startFlag ) 
      {  
       startFlag = 0 ;
       Sx = Zx ;
       Sy = Zy ;
       Sz = Zz ; 
      }
/*
**  Calculate Start Angle Direction Of Contour
*/
    if( contourStartType == 2 ) if( bcdtmPolyContour_getContourStartDirectionDtmObject(dtmP,P0,P1,P2,Zx,Zy,Zz,eps,&contourStartDirection) ) goto errexit ;
    if(  dbg ) bcdtmWrite_message(0,0,0,"Start Angle = %12.10lf",contourStartDirection) ;
/*
**  Trace Contour Over Triangle  
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Tracing Over Triangle") ;
    if( bcdtmPolyContour_traceContourOverTriangleDtmObject(dtmP,contourValue,P0,P1,P2,Zx,Zy,Zz,contourStartDirection,rMax,rMin,rMax2,posError,maxStep,firstStep,eps,XYTolerance,&Zx,&Zy,&Zz,&exitSide)) goto errexit ;
    if( exitSide < 0 ) tinTrace = 0 ;
/*
**  Get Next Triangle To Trace Contour Over
*/
    else
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Getting Next Triangle To Trace Over") ;
       if( bcdtmPolyContour_getNextTraceTriangleForContourDtmObject(dtmP,contourValue,exitSide,closeFlag,Sx,Sy,contourIndexP,numLines,zeroPtsP,numcontourZeroPts,eps,firstStep,rMax,XYTolerance,ZTolerance,partDerivP,&P0,&P1,&P2,&Zx,&Zy,&Zz,&contourStartType,&contourStartDirection,&tinTrace) ) goto errexit ; 
      } 
   }
/*
** Plot Contour
*/
 if( callBackFunctionP != nullptr )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Contour Points Before Filter = %8ld",NumContourCachePts) ;
    if( bcdtmPolyContour_xyFilterContourPoints(ContourPtsCacheP,&NumContourCachePts,XYTolerance)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Contour Points After  Filter = %8ld",NumContourCachePts) ;
    if( NumContourCachePts > 1 && ContourPtsCacheP != nullptr )
      {
       if( callBackFunctionP(DTMFeatureType::Contour,dtmP->nullUserTag,dtmP->nullFeatureId,ContourPtsCacheP,NumContourCachePts,userP)) goto errexit ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 NumContourCachePts = 0 ;
 CL = 0.0 ;
/*
** Return
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit  :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmPolyContour_traceContourOverTriangleDtmObject
(
 BC_DTM_OBJ *dtmP,         // ==> Pointer To DTM 
 double contourValue,      // ==> Value Of Contour being Traced
 long   P0,                // ==> DTM Point Number Of Triangle Vertex
 long   P1,                // ==> DTM Point Number Of Triangle Vertex 
 long   P2,                // ==> DTM Point Number Of Triangle Vertex
 double zx,                // ==> Start Zero z Coordinate
 double zy,                // ==> Start Zero y Coordinate
 double zz,                // ==> Start Zero z Coordinate
 double startAngle,        // ==> Contour Start Angle
 double radiusMax,         // ==> Distance Normal To Curve Direction Within which a Zero must be found
 double radiusMin,         // ==> If Zero Is Found Within A Distance smaller Than radiusMin, Then step size is multiplied by 2.0
 double radiusMax2,        // ==> Distance Normal To Curve Direction. A Zero Must Be Found Within This Distance When Crossing A Triangle Boundary.
 double posError,          // ==> Permitted Positional Error
 double maxStep,           // ==> Maximum Step Size
 double firstStep,         // ==> Starting Step Size
 double startDifference,   // ==> Difference For Estimating Starting Direction
 double xyTolerance,       // ==> Filtering Tolerance
 double *lzxP,             // <== Last Zero x Coordinate
 double *lzyP,             // <== Last Zero y Coordinate
 double *lzzP,             // <== Last Zero z Coordinate
 long   *exitSideP         // <== Contour Exit Side Of Triangle
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   n,side,numPointsInQue,triangleTrace,loop,process,totalTrgPoints,zeroFound,zeroNotFound,retryCount ;
 double scos,ssin,radMax,contourStep,contourDirection,F1,F2,F3,F12,F13 ;
 double dx12,dy12,qx1,qy1,qx2,qy2,qx3,qy3 ;
 double contourStep12,contourStep23,contourStepDx,contourStepDy ;
 double contourStep13,Pl0,Pl1,Pl2,stepLength ;
 double x,y,lx,ly,angle,angleInc ;
/*
** Write Entry Message
*/
 bcdtmWrite_message(0,0,0,"Tracing Contour Over Triangle ** P0 = %8ld P1 = %8ld P3 = %8ld",P0,P1,P2) ;

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Contour Over Triangle") ;
    bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",P0,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,pointAddrP(dtmP,P0)->z) ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
    bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
    bcdtmWrite_message(0,0,0,"Contour Start Zero = %10.4lf %10.4lf %10.4lf",zx,zy,zz) ;
    bcdtmWrite_message(0,0,0,"Start Angle        = %15.12lf",startAngle) ;
    bcdtmWrite_message(0,0,0,"startDifference    = %15.12lf",startDifference) ;
    bcdtmWrite_message(0,0,0,"radiusMin          = %15.12lf",radiusMin) ;
    bcdtmWrite_message(0,0,0,"radiusMax          = %15.12lf",radiusMax) ;
    bcdtmWrite_message(0,0,0,"radiusMax2         = %15.12lf",radiusMax2) ;
    bcdtmWrite_message(0,0,0,"maxStep            = %15.12lf",maxStep) ;
    bcdtmWrite_message(0,0,0,"firstStep          = %15.12lf",firstStep) ;
    bcdtmWrite_message(0,0,0,"posError           = %15.12lf",posError) ;
   } 
/*
**  Initialise Parameters For Tracing Contour Triangle
*/
 numPointsInQue   = 0 ;
 totalTrgPoints   = 0 ;
 CL = contourValue ;
 contourStep = maxStep ;
 radMax = radiusMax  ;
 contourDirection = startAngle ;
 dx12 = contourStep * cos(contourDirection) ;
 dy12 = contourStep * sin(contourDirection) ; 
 qx3  = zx ;
 qy3  = zy ;
 *exitSideP = - 1 ;
/*
**  Ensure First Point qx3,qy3 Is Inside Triangle
*/
 while ( ! bcdtmPolyContour_checkIfPointIsInsideTriangle(zx,zy,&side)) 
   {
    if( contourStep > startDifference && contourStep > dtmP->ppTol )
      { 
       contourStep = contourStep * 0.5 ; 
       dx12 = contourStep * cos(contourDirection) ;
       dy12 = contourStep * sin(contourDirection) ; 
       zx = qx3 + dx12 ;
       zy = qy3 + dy12 ;
      }
    else
      {
       *exitSideP = side ;
       goto cleanup ; 
      } 
   } 
/*
** Initailise Queue Points For Curve Calculations
*/
 qx2  = qx3 - dx12 ;
 qy2  = qy3 - dy12 ;
 qx1  = qx2 - dx12 ;
 qy1  = qy2 - dy12 ; 
 contourStep12 = contourStep ;
 contourStep23 = contourStep ;
/*
** Calculate Succesive Points On Contour Over Triangle
*/
 triangleTrace = 1 ;
 while ( triangleTrace )
   {
/*
**  Calculate Curve Direction
*/
    contourStep13 =  contourStep23 + contourStep12 ;
    Pl0  =  contourStep23/(contourStep12*contourStep13) ;
    Pl1  = -contourStep13/(contourStep12*contourStep23) ;
    Pl2  =  (contourStep13+contourStep23)/(contourStep13*contourStep23) ;
    contourStepDx =  Pl0*qx1 + Pl1*qx2 + Pl2*qx3 ;
    contourStepDy =  Pl0*qy1 + Pl1*qy2 + Pl2*qy3 ;
    stepLength    =  sqrt(contourStepDx*contourStepDx+contourStepDy*contourStepDy) ;
    contourStepDx =  contourStepDx / stepLength ;
    contourStepDy =  contourStepDy / stepLength ;
    contourDirection = atan2(contourStepDy,contourStepDx) ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"contourDirection = %14.10lf",contourDirection) ;
/*
**  Get Next Contour Zero
*/
    retryCount = 0 ;
    zeroNotFound = 1 ;
    while ( zeroNotFound )
      {
if( dbg ) bcdtmWrite_message(0,0,0,"00 contourStep = %12.10lf",contourStep) ;

/*
**     Can Not Find Zero
*/ 
       ++retryCount ;
       if( retryCount > 10 ) 
         { 
             bcdtmWrite_message(0,0,0,"Cannot Find Zero ** Terminating Trace") ; 
             bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf zz = %10.4lf",zx,zy) ; 
          if( dbg == 1 ) 
            {
             bcdtmWrite_message(0,0,0,"Cannot Find Zero ** Terminating Trace") ; 
             bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf zz = %10.4lf",zx,zy) ; 
            }
/*
**        Clear Queue
*/
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Clearing Cache **** numPointsInQue = %8ld",numPointsInQue) ;
          if (numPointsInQue > 3 ) if( bcdtmPolyContour_storeContourPointInCache(qx1,qy1,contourValue)) goto errexit ;
          if (numPointsInQue > 2 ) if( bcdtmPolyContour_storeContourPointInCache(qx2,qy2,contourValue)) goto errexit ;
          if (numPointsInQue > 1 ) if( bcdtmPolyContour_storeContourPointInCache(qx3,qy3,contourValue)) goto errexit ;
          numPointsInQue = 0 ;
          goto cleanup ; 
         }
/*
**     Look For Zero 
*/
       zeroNotFound = 0 ;
       contourStepDx = cos(contourDirection) ;
       contourStepDy = sin(contourDirection) ;
       scos = cos(contourDirection+DTM_PYE/2.0) ;
       ssin = sin(contourDirection+DTM_PYE/2.0) ;
/*
**     Find Two Points On Either Side Of Contour
*/
       loop = 0 ;
       process = 1 ;
       zeroFound = 0 ;
       while ( process )
         {
          ++loop ;
          zx = qx3 + contourStep * contourStepDx ;
          zy = qy3 + contourStep * contourStepDy ;
          if( contourStep > startDifference && ! bcdtmPolyContour_checkIfPointIsInsideTriangle(zx,zy,&side))
            {
             contourStep = contourStep * 0.5 ;
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"loop = %2ld zx zy Outside Triangle",loop) ;
            }
          else
            {
/*
**           Evalaute Polynomial At Trace Points
*/  
             F1 = bcdtmPolyContour_evaluateBivariateFunction(zx,zy,0.0,scos,ssin)  ; 
             F2 = bcdtmPolyContour_evaluateBivariateFunction(zx,zy,radMax,scos,ssin)  ; 
             F3 = bcdtmPolyContour_evaluateBivariateFunction(zx,zy,-radMax,scos,ssin)  ; 
             F12 = F1 * F2 ;
             F13 = F1 * F3 ;
/*
**           Check If Zero Found
*/
             if     ( F12 <  0.0 && F13  > 0.0 ) { zeroFound = 1 ; process = 0 ; } // Zero Between F2 And F1
             else if( F12 >  0.0 && F13  < 0.0 ) { zeroFound = 2 ; process = 0 ; } // Zero Between F1 And F3
             else if( fabs(F1) < 0.000000001 )  { zeroFound = 3 ; process = 0 ; } // Zero At F1
             else if( fabs(F2) < 0.000000001 )  { zeroFound = 4 ; process = 0 ; } // Zero At F2
             else if( fabs(F3) < 0.000000001 )  { zeroFound = 5 ; process = 0 ; } // Zero At F3 
/*
**           Reset Trace Parameters
*/ 
             else if( F12 == 0.0 || F13 == 0.0 ) process = 0 ;
             else if( F12 < 0.0 && F13 < 0.0 && radMax > 0.00001 ) radMax = radMax * 0.5 ; 
             else if( F12 > 0.0 && F13 > 0.0 && radMax > 0.00001 ) radMax = radMax * 0.5 ; 
             else if( fabs(F1) < fabs(F2) && fabs(F1) < fabs(F3) && radMax > 0.00001 ) radMax = radMax * 0.5 ;
             else if( contourStep > 0.00001 ) contourStep = contourStep * 0.5 ;
             else process = 0 ; 
            }
         }
if( zeroFound >=3 && zeroFound <= 5 ) bcdtmWrite_message(0,0,0,"ZERO FOUND = %2ld",zeroFound) ;
/*
**     Zero Not Found
*/
       if( ! zeroFound  )
         {
          if( dbg == 0 )
            { 
             bcdtmWrite_message(0,0,0,"Zero Not Found") ;
             bcdtmWrite_message(0,0,0,"loop = %2ld ** contourStep = %15.12lf radMax = %15.12lf ** F3  = %13.10lf F1  = %13.10lf F2 = %13.10lf",loop,contourStep,radMax,F3,F1,F2) ;   
             bcdtmWrite_message(0,0,0,"F13 = %15.12lf F12 = %15.12lf",F13,F12) ; 
             bcdtmWrite_message(0,0,0,"zx  = %15.12lf zy  = %15.12lf",zx,zy) ; 
            }
/*
**        Scan Around Last Zero looking For Next Zero Point
*/
          x = zx ;
          y = zy ;
          F1 = bcdtmPolyContour_evaluateBivariateFunction(x,y,0.0,0.0,0.0)  ; 
          F3 = bcdtmPolyContour_evaluateBivariateFunction(qx3,qy3,0.0,0.0,0.0)  ; 
          bcdtmWrite_message(0,0,0,"fabs(F3) = %12.5lf",fabs(F3)) ;
          radMax = 0.1 ;
          angleInc = DTM_2PYE / 10000.0 ;
          angle  = bcdtmMath_getAngle(qx3,qy3,x,y) ;
          bcdtmWrite_message(0,0,0,"Start Angle = %12.10lf",angle) ;
          angle = angle + angleInc ;
          lx = x = qx3 + radMax * cos(angle) ;
          ly = y = qy3 + radMax * sin(angle) ;
         F1 = bcdtmPolyContour_evaluateBivariateFunction(x,y,0.0,0.0,0.0)  ; 
          for( n = 1 ; n < 9999 ; ++n )
            {
             angle = angle + angleInc ;
             angle = bcdtmMath_normaliseAngle(angle) ;
             x = qx3 + radMax * cos(angle) ;
             y = qy3 + radMax * sin(angle) ;
             F2 = bcdtmPolyContour_evaluateBivariateFunction(x,y,0.0,0.0,0.0)  ;
             if( F1 * F2 < 0 ) 
               {
                bcdtmWrite_message(0,0,0,"qx = %15.12lf qy = %15.12lf qz = %15.12lf",zx,zy,bcdtmPolyContour_evaluateBivariateFunction(qx3,qy3,0.0,0.0,0.0)) ; 
                bcdtmWrite_message(0,0,0,"x  = %15.12lf y  = %15.12lf",x,y) ;
                bcdtmWrite_message(0,0,0,"lx = %15.12lf ly = %15.12lf",lx,ly) ;
                bcdtmWrite_message(0,0,0,"Zero Found ** F1 = %15.10lf F2 = %15.10lf",F1,F2) ;
                if( bcdtmPolyContour_calculateBivariateFunctionZeroBetweenPoints(x,y,lx,ly,&zx,&zy,&zz)) goto errexit  ;
                bcdtmWrite_message(0,0,0,"zx = %15.12lf zy = %15.12lf zz = %15.12lf",zx,zy,zz) ;
               }
             lx = x ;
             ly = y ;
             F1 = F2 ;
            }     
          contourDirection = bcdtmMath_getAngle(qx3,qy3,zx,zy) ;
          contourStep = bcdtmMath_distance(qx3,qy3,zx,zy) ;
          zeroFound = 3 ;

 //         goto errexit ;
/*
**        Change Contour Direction
*/
/*
          dx12 = firstStep * cos(contourDirection) ;
          dy12 = firstStep * sin(contourDirection) ; 
          zx = qx3 + dx12 ;
          zy = qy3 + dy12 ;
          if( fabs(F2) < fabs(F3) ) { zx = zx + radiusMax * scos ; zy = zy + radiusMax * ssin ; } 
          else                      { zx = zx - radiusMax * scos ; zy = zy - radiusMax * ssin ; } 
if( dbg ) bcdtmWrite_message(0,0,0,"00 contourDirection = %12.10lf",contourDirection) ;
          contourDirection = bcdtmMath_getAngle(qx3,qy3,zx,zy) ;
if( dbg ) bcdtmWrite_message(0,0,0,"01 contourDirection = %12.10lf",contourDirection) ;
          contourDirection = zeroAngle ;
          contourStep = firstStep ;
          radMax = zeroRad ;
          zeroNotFound = 1 ;
*/
         } 
/*
**     Zero Found
*/
       if( zeroFound )
         { 
          if( zeroFound == 1 ) if( bcdtmPolyContour_calculateBivariateFunctionZero(zx,zy,scos,ssin,0.0, radMax,F1,F2,&zx,&zy,&zz) ) goto errexit ;
          if( zeroFound == 2 ) if( bcdtmPolyContour_calculateBivariateFunctionZero(zx,zy,scos,ssin,0.0,-radMax,F1,F3,&zx,&zy,&zz) ) goto errexit ;
          if( zeroFound == 3 ) zz = 0.0 ;
          if( zeroFound == 4 ) { zx = zx +  radMax * scos ; zy = zy +  radMax * ssin ; zz = 0.0 ; } 
          if( zeroFound == 5 ) { zx = zx + -radMax * scos ; zy = zy + -radMax * ssin ; zz = 0.0 ; } 
          if( dbg == 2 ) 
            {
             bcdtmWrite_message(0,0,0,"Zero Found") ;
             bcdtmWrite_message(0,0,0,"F3 = %13.10lf F1 = %13.10lf F2 = %13.10lf",F3,F1,F2) ;   
             bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf zz = %13.10lf",zx,zy,zz) ; 
            } 
/*
**        Check If Point Is Inside Triangle
*/
          if( bcdtmPolyContour_checkIfPointIsInsideTriangle(zx,zy,&side))
            {
             if( dbg == 2  ) bcdtmWrite_message(0,0,0,"Point Inside Triangle") ;
/*
**           Check MidPoint Of Line Joining Last Zero To Current Zero
*/
             F1 = bcdtmPolyContour_evaluateBivariateFunction((zx+qx3)/2.0,(zy+qy3)/2.0,0.0,0.0,0.0)  ; 
             if( dbg  == 2 ) bcdtmWrite_message(0,0,0,"MidPoint F1 = %12.10lf",F1) ; 
             if( fabs(F1) > 0.001  ) 
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Midpoint To High ** SubDividing ** contourStep = %12.10lf",contourStep) ;
                contourDirection = bcdtmMath_getAngle(qx3,qy3,(zx+qx3)/2.0,(zy+qy3)/2.0) ;
                contourStep = contourStep * 0.5 ;
 //               radMax = radiusMax ;
                if( dbg ) bcdtmWrite_message(0,0,0,"01 contourStep = %12.10lf",contourStep) ;
               } 
/*
**           If MidPoint Within Tolerance Plot Point   
*/ 
             else
               {
/*
**              Check Distance To Last Point
*/
                if( dbg == 3 )
                  {
                   if( ( F1 = bcdtmMath_distance(zx,zy,qx3,qy3)) < 0.0001 )
                     {
                      bcdtmWrite_message(0,0,0,"Last Point = %10.4lf %10.4lf %10.4lf",zx,zy,zz+CL) ;
                      bcdtmWrite_message(0,0,0,"Distance To Next point = %10.6lf",F1) ;
                     }
                  }
/*
**              Update Queue
*/
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Plotting Point %10.4lf %10.4lf %15.12lf",zx,zy,zz) ;
                ++numPointsInQue   ;
                ++totalTrgPoints   ;
                if( totalTrgPoints > 1000000 ) { bcdtmWrite_message(1,0,0,"Too Many Points On Contour") ; goto errexit ; }
/*
**              If Four Points In Queue Plot P1
*/ 
                if( numPointsInQue == 4 ) 
                  { 
                   --numPointsInQue ; 
                   if( bcdtmPolyContour_storeContourPointInCache(qx1,qy1,contourValue) ) goto errexit ;
                  }
/*
**              Reset Queue Points
*/
                contourStep12 = contourStep23 ;
                contourStep23 = bcdtmMath_distance(qx3,qy3,zx,zy) ;
                qx1  = qx2  ;
                qy1  = qy2  ;
                qx2  = qx3  ;
                qy2  = qy3  ;
                qx3  = zx   ;
                qy3  = zy   ;
/*
**              Reset Trace Parameters
*/
                contourStep = firstStep ;
                radMax = radiusMax ;
               }
            }
/*
**        Point Is Outside Triangle
*/
          else
            {
             if( dbg == 2 ) 
               {
                bcdtmWrite_message(0,0,0,"Point Outside Triangle ** Side = %1ld",side) ;
                bcdtmWrite_message(0,0,0,"startDifference = %10.8lf contourStep = %10.8lf Distance To Last Zero = %10.8lf",startDifference,contourStep,bcdtmMath_distance(zx,zy,qx3,qy3)) ;
               }
/*
**           Check Point Is Less Than Exit Distance
*/
//           if( bcdtmMath_distance(zx,zy,qx3,qy3) > startDifference && contourStep > startDifference )
             if( bcdtmMath_distance(zx,zy,qx3,qy3) > radiusMax2 )
               {
                contourDirection = bcdtmMath_getAngle(qx3,qy3,zx,zy) ;
                contourStep = contourStep * 0.5 ;
               }
/*
**           Terminate Trace Over Triangle
*/
             else
               {
/*
**              Clear Queue
*/
                if( dbg == 3 ) bcdtmWrite_message(0,0,0,"Clearing Cache **** numPointsInQue = %8ld",numPointsInQue) ;
                if( numPointsInQue > 3 ) if( bcdtmPolyContour_storeContourPointInCache(qx1,qy1,contourValue)) goto errexit ;
                if( numPointsInQue > 2 ) if( bcdtmPolyContour_storeContourPointInCache(qx2,qy2,contourValue)) goto errexit ;
                if( numPointsInQue > 1 ) if( bcdtmPolyContour_storeContourPointInCache(qx3,qy3,contourValue)) goto errexit ;
                numPointsInQue = 0 ;
/*
**              Filter Points Traced Over Triangle
*/
//              if( bcdtmPolyContour_storeContourPointInCache(4,0.0,0.0,0.0,xyTolerance,zTolerance) ) goto errexit ;
/*
**              Terminate Trace Over Triangle
*/
                triangleTrace = 0 ;
/*
**              Set Exit Side Of Triangle
*/
                *exitSideP = side ;
/*
**              Set Value Of Last Zero Plotted
*/
                *lzxP = qx3 ;
                *lzyP = qy3 ;
                *lzzP = contourValue ;
               }
            }
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
 CL = 0.0 ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Contour Over Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Contour Over Triangle Error") ;
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
BENTLEYDTM_Private int bcdtmPolyContour_traceContourOverTriangleDtmObjectXX00
(
 BC_DTM_OBJ *dtmP,         // ==> Pointer To DTM 
 double contourValue,      // ==> Value Of Contour being Traced
 long   P0,                // ==> DTM Point Number Of Triangle Vertex
 long   P1,                // ==> DTM Point Number Of Triangle Vertex 
 long   P2,                // ==> DTM Point Number Of Triangle Vertex
 double zx,                // ==> Start Zero x Coordinate
 double zy,                // ==> Start Zero y Coordinate
 double zz,                // ==> Start Zero z Coordinate
 double startAngle,        // ==> Contour Start Angle
 double radiusMax,         // ==> Distance Normal To Curve Direction Within which a Zero must be found
 double radiusMin,         // ==> If Zero Is Found Within A Distance smaller Than radiusMin, Then step size is multiplied by 2.0
 double radiusMax2,        // ==> Distance Normal To Curve Direction. A Zero Must Be Found Within This Distance When Crossing A Triangle Boundary.
 double posError,          // ==> Permitted Positional Error
 double maxStep,           // ==> Maximum Step Size
 double firstStep,         // ==> Starting Step Size
 double startDifference,   // ==> Difference For Estimating Starting Direction
 double xyTolerance,       // ==> Filtering Tolerance
 double *lzxP,             // <== Last Zero x Coordinate
 double *lzyP,             // <== Last Zero y Coordinate
 double *lzzP,             // <== Last Zero z Coordinate
 long   *exitSideP         // <== Contour Exit Side Of Triangle
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   side,numPointsInQue,triangleTrace,loop,process,totalTrgPoints,zeroNotFound,retryCount ;
 double scos,ssin,radMax,contourStep,contourDirection,F1,F2,F3,F12,F13 ;
 double dx12,dy12,qx1,qy1,qx2,qy2,qx3,qy3 ;
 double contourStep12,contourStep23,contourStepDx,contourStepDy ;
 double contourStep13,Pl0,Pl1,Pl2,stepLength ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Contour Over Triangle") ;
    bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",P0,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,pointAddrP(dtmP,P0)->z) ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
    bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
    bcdtmWrite_message(0,0,0,"Contour Start Zero = %10.4lf %10.4lf %10.4lf",zx,zy,zz) ;
    bcdtmWrite_message(0,0,0,"Start Angle        = %12.10lf",startAngle) ;
    bcdtmWrite_message(0,0,0,"startDifference    = %15.12lf",startDifference) ;
    bcdtmWrite_message(0,0,0,"radiusMin          = %15.12lf",radiusMin) ;
    bcdtmWrite_message(0,0,0,"radiusMax          = %15.12lf",radiusMax) ;
    bcdtmWrite_message(0,0,0,"radiusMax2         = %15.12lf",radiusMax2) ;
    bcdtmWrite_message(0,0,0,"maxStep            = %15.12lf",maxStep) ;
    bcdtmWrite_message(0,0,0,"firstStep          = %15.12lf",firstStep) ;
    bcdtmWrite_message(0,0,0,"posError           = %15.12lf",posError) ;
   } 
/*
**  Initialise Parameters For Tracing Contour Triangle
*/
 numPointsInQue   = 0 ;
 totalTrgPoints   = 0 ;
 CL = contourValue ;
 contourStep = maxStep ;
 radMax = radiusMax  ;
 contourDirection = startAngle ;
 dx12 = contourStep * cos(contourDirection) ;
 dy12 = contourStep * sin(contourDirection) ; 
 qx3  = zx ;
 qy3  = zy ;
 *exitSideP = - 1 ;
/*
**  Ensure First Point qx3,qy3 Is Inside Triangle
*/
 while ( ! bcdtmPolyContour_checkIfPointIsInsideTriangle(zx,zy,&side)) 
   {
//    if( contourStep > startDifference && contourStep > 0.001 )
    if( contourStep > startDifference && contourStep > dtmP->ppTol )
      { 
       contourStep = contourStep * 0.5 ; 
       dx12 = contourStep * cos(contourDirection) ;
       dy12 = contourStep * sin(contourDirection) ; 
       zx = qx3 + dx12 ;
       zy = qy3 + dy12 ;
      }
    else
      {
       *exitSideP = side ;
       goto cleanup ; 
      } 
   } 
/*
** Initailise Queue Points For Curve Calculations
*/
 qx2  = qx3 - dx12 ;
 qy2  = qy3 - dy12 ;
 qx1  = qx2 - dx12 ;
 qy1  = qy2 - dy12 ; 
 contourStep12 = contourStep ;
 contourStep23 = contourStep ;
/*
** Calculate Succesive Points On Contour Over Triangle
*/
 triangleTrace = 1 ;
 while ( triangleTrace )
   {
/*
**  Calculate Curve Direction
*/
    contourStep13 =  contourStep23 + contourStep12 ;
    Pl0  =  contourStep23/(contourStep12*contourStep13) ;
    Pl1  = -contourStep13/(contourStep12*contourStep23) ;
    Pl2  =  (contourStep13+contourStep23)/(contourStep13*contourStep23) ;
    contourStepDx =  Pl0*qx1 + Pl1*qx2 + Pl2*qx3 ;
    contourStepDy =  Pl0*qy1 + Pl1*qy2 + Pl2*qy3 ;
    stepLength    =  sqrt(contourStepDx*contourStepDx+contourStepDy*contourStepDy) ;
    contourStepDx =  contourStepDx / stepLength ;
    contourStepDy =  contourStepDy / stepLength ;
    contourDirection = atan2(contourStepDy,contourStepDx) ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"contourDirection = %14.10lf",contourDirection) ;
/*
**  Get Next Contour Zero
*/
    retryCount = 0 ;
    zeroNotFound = 1 ;
    while ( zeroNotFound )
      {
/*
**     Can Not Find Zero
*/ 
       ++retryCount ;
       if( retryCount > 10 ) 
         { 
          if( dbg == 1 ) 
            {
             bcdtmWrite_message(0,0,0,"Cannot Find Zero ** Terminating Trace") ; 
             bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf zz = %10.4lf",zx,zy) ; 
            }
/*
**        Clear Queue
*/
          if( dbg == 3 ) bcdtmWrite_message(0,0,0,"Clearing Cache **** numPointsInQue = %8ld",numPointsInQue) ;
          if (numPointsInQue > 3 ) if( bcdtmPolyContour_storeContourPointInCache(qx1,qy1,contourValue)) goto errexit ;
          if (numPointsInQue > 2 ) if( bcdtmPolyContour_storeContourPointInCache(qx2,qy2,contourValue)) goto errexit ;
          if (numPointsInQue > 1 ) if( bcdtmPolyContour_storeContourPointInCache(qx3,qy3,contourValue)) goto errexit ;
          numPointsInQue = 0 ;
          goto cleanup ; 
         }
/*
**     Look For Zero 
*/
       zeroNotFound  = 0 ;
       contourStepDx = cos(contourDirection) ;
       contourStepDy = sin(contourDirection) ;
       scos = cos(contourDirection+DTM_PYE/2.0) ;
       ssin = sin(contourDirection+DTM_PYE/2.0) ;
/*
**     Find Two Points On Either Side Of Contour
*/
       loop = 0 ;
       process = 1 ;
       while ( process )
         {
          ++loop ;
          zx = qx3 + contourStep * contourStepDx ;
          zy = qy3 + contourStep * contourStepDy ;
          if( contourStep > startDifference && ! bcdtmPolyContour_checkIfPointIsInsideTriangle(zx,zy,&side))
            {
             contourStep = contourStep * 0.5 ;
             if( dbg ) bcdtmWrite_message(0,0,0,"loop = %2ld zx zy Outside Triangle",loop) ;
            }
          else
            {  
             F1 = bcdtmPolyContour_evaluateBivariateFunction(zx,zy,0.0,scos,ssin)  ; 
             F2 = bcdtmPolyContour_evaluateBivariateFunction(zx,zy,radMax,scos,ssin)  ; 
             F3 = bcdtmPolyContour_evaluateBivariateFunction(zx,zy,-radMax,scos,ssin)  ; 
             F12 = F1 * F2 ;
             F13 = F1 * F3 ;
             if     ( F12 <  0.0 && F13  > 0.0 ) process = 0 ;
             else if( F12 >  0.0 && F13  < 0.0 ) process = 0 ;
             else if( F12 == 0.0 || F13 == 0.0 ) process = 0 ;
             else if( F12 < 0.0 && F13 < 0.0 && radMax > 0.00001 ) radMax = radMax * 0.5 ; 
             else if( fabs(F1) < fabs(F2) && fabs(F1) < fabs(F3) && radMax > 0.00001 ) radMax = radMax * 0.5 ;
             else if( contourStep > 0.00001 ) contourStep = contourStep * 0.5 ;
             else process = 0 ; 
            }
         }
/*
**     Test If Zero Found And If Found Calculate Zero
*/
       if     ( F12 < 0.0 && F13 > 0.0 ) process = 1 ;
       else if( F12 > 0.0 && F13 < 0.0 ) process = 2 ;
       else if( F2 == 0.0 )              process = 3 ;
       else if( F3 == 0.0 )              process = 4 ; 
       else if( fabs(F1) < 0.000000001 ) process = 5 ;
/*
**     Zero Not Found
*/
       if( process == 0 )
         {
          if( dbg == 1 )
            { 
             bcdtmWrite_message(0,0,0,"Zero Not Found") ;
             bcdtmWrite_message(0,0,0,"loop = %2ld contourStep = %10.8lf radMax = %11.8lf",loop,contourStep,radMax) ; 
             bcdtmWrite_message(0,0,0,"F13 = %13.10lf F12 = %13.10lf",F13,F12) ; 
             bcdtmWrite_message(0,0,0,"contourStep = %10.8lf radMax = %11.8lf ** F3  = %13.10lf F1  = %13.10lf F2 = %13.10lf",contourStep,radMax,F3,F1,F2) ;   
             bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf",zx,zy) ; 
            }
          if( fabs(F2) < fabs(F3) ) { zx = zx + radMax * scos ; zy = zy + radMax * ssin ; } 
          else                      { zx = zx - radMax * scos ; zy = zy - radMax * ssin ; } 
          contourDirection = bcdtmMath_getAngle(qx3,qy3,zx,zy) ;
          contourStep = firstStep ;
          radMax = radiusMax ;
          zeroNotFound = 1 ;
         } 
/*
**     Zero Found
*/
       if( process == 1 ) if( bcdtmPolyContour_calculateBivariateFunctionZero(zx,zy,scos,ssin,0.0, radMax,F1,F2,&zx,&zy,&zz) ) goto errexit ;
       if( process == 2 ) if( bcdtmPolyContour_calculateBivariateFunctionZero(zx,zy,scos,ssin,0.0,-radMax,F1,F3,&zx,&zy,&zz) ) goto errexit ;
       if( process == 3 ) { zx = zx +  radMax * scos ; zy = zy +  radMax * ssin ; zz = 0.0 ; } 
       if( process == 4 ) { zx = zx + -radMax * scos ; zy = zy + -radMax * ssin ; zz = 0.0 ; } 
       if( process == 5 ) zz = 0.0 ;
       if( dbg == 2 ) 
         {
          bcdtmWrite_message(0,0,0,"Zero Found") ;
          bcdtmWrite_message(0,0,0,"F3 = %13.10lf F1 = %13.10lf F2 = %13.10lf",F3,F1,F2) ;   
          bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf zz = %13.10lf",zx,zy,zz) ; 
          bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf zz = %13.10lf",zx,zy,bcdtmPolyContour_evaluateBivariateFunction(zx,zy,0.0,0.0,0.0)) ; 
         } 
/*
**     Check If Point Is Inside Triangle
*/
       if( bcdtmPolyContour_checkIfPointIsInsideTriangle(zx,zy,&side))
         {
          if( dbg == 2  ) bcdtmWrite_message(0,0,0,"Point Inside Triangle") ;
/*
**        Check MidPoint Of Line Joining Last Zero To Current Zero
*/
          F1 = bcdtmPolyContour_evaluateBivariateFunction((zx+qx3)/2.0,(zy+qy3)/2.0,0.0,0.0,0.0)  ; 
          if( dbg  == 2 ) bcdtmWrite_message(0,0,0,"MidPoint F1 = %12.10lf",F1) ; 
          if( fabs(F1) > 0.01  ) 
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Midpoint To High ** SubDividing") ;
             contourDirection = bcdtmMath_getAngle(qx3,qy3,zx,zy) ;
             contourStep = contourStep * 0.5 ;
            } 
/*
**        If MidPoint Within Tolerance Plot Point   
*/ 
          else
            {
/*
**           Check Distance To Last Point
*/
             if( dbg == 3 )
               {
                if( ( F1 = bcdtmMath_distance(zx,zy,qx3,qy3)) < 0.0001 )
                  {
                   bcdtmWrite_message(0,0,0,"Last Point = %10.4lf %10.4lf %10.4lf",zx,zy,zz+CL) ;
                   bcdtmWrite_message(0,0,0,"Distance To Next point = %10.6lf",F1) ;
                  }
               }
/*
**           Update Queue
*/
             if( dbg == 3 ) bcdtmWrite_message(0,0,0,"Plotting Point %10.4lf %10.4lf %15.12lf",zx,zy,zz) ;
             ++numPointsInQue   ;
             ++totalTrgPoints   ;
             if( totalTrgPoints > 1000000 ) { bcdtmWrite_message(1,0,0,"Too Many Points On Contour") ; goto errexit ; }
/*
**           If Four Points In Queue Plot P1
*/ 
             if( numPointsInQue == 4 ) 
               { 
                --numPointsInQue ; 
                if( bcdtmPolyContour_storeContourPointInCache(qx1,qy1,contourValue) ) goto errexit ;
               }
/*
**           Reset Queue Points
*/
             contourStep12 = contourStep23 ;
             contourStep23 = bcdtmMath_distance(qx3,qy3,zx,zy) ;
             qx1  = qx2  ;
             qy1  = qy2  ;
             qx2  = qx3  ;
             qy2  = qy3  ;
             qx3  = zx   ;
             qy3  = zy   ;
/*
**           Reset Trace Parameters
*/
             contourStep = firstStep ;
             radMax = radiusMax ;
            }
         }
/*
**     Point Is Outside Triangle
*/
       else
         {
          if( dbg == 2 ) 
            {
             bcdtmWrite_message(0,0,0,"Point Outside Triangle ** Side = %1ld",side) ;
             bcdtmWrite_message(0,0,0,"startDifference = %10.8lf contourStep = %10.8lf Distance To Last Zero = %10.8lf",startDifference,contourStep,bcdtmMath_distance(zx,zy,qx3,qy3)) ;
            }
/*
**        Check Point Is Less Than Exit Distance
*/
//          if( bcdtmMath_distance(zx,zy,qx3,qy3) > startDifference && contourStep > startDifference )
          if( bcdtmMath_distance(zx,zy,qx3,qy3) > radiusMax2 )
            {
             contourDirection = bcdtmMath_getAngle(qx3,qy3,zx,zy) ;
             contourStep = contourStep * 0.5 ;
            }
          else
            {
/*
**           Clear Queue
*/
             if( dbg == 3 ) bcdtmWrite_message(0,0,0,"Clearing Cache **** numPointsInQue = %8ld",numPointsInQue) ;
             if( numPointsInQue > 3 ) if( bcdtmPolyContour_storeContourPointInCache(qx1,qy1,contourValue)) goto errexit ;
             if( numPointsInQue > 2 ) if( bcdtmPolyContour_storeContourPointInCache(qx2,qy2,contourValue)) goto errexit ;
             if( numPointsInQue > 1 ) if( bcdtmPolyContour_storeContourPointInCache(qx3,qy3,contourValue)) goto errexit ;
             numPointsInQue = 0 ;
/*
**           Filter Points Traced Over Triangle
*/
//         if( bcdtmPolyContour_storeContourPointInCache(4,0.0,0.0,0.0,xyTolerance,zTolerance) ) goto errexit ;
/*
**           Terminate Trace Over Triangle
*/
             triangleTrace = 0 ;
/*
**           Set Exit Side Of Triangle
*/
             *exitSideP = side ;
/*
**           Set Value Of Last Zero Plotted
*/
             *lzxP = qx3 ;
             *lzyP = qy3 ;
             *lzzP = contourValue ;
            }
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
 CL = 0.0 ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Contour Over Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Contour Over Triangle Error") ;
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
BENTLEYDTM_Private int bcdtmPolyContour_traceContourOverTriangleDtmObjectOld
(
 BC_DTM_OBJ *dtmP,         // ==> Pointer To DTM 
 double contourValue,      // ==> Value Of Contour being Traced
 long   P0,                // ==> DTM Point Number Of Triangle Vertex
 long   P1,                // ==> DTM Point Number Of Triangle Vertex 
 long   P2,                // ==> DTM Point Number Of Triangle Vertex
 double zx,                // ==> Start Zero z Coordinate
 double zy,                // ==> Start Zero y Coordinate
 double zz,                // ==> Start Zero z Coordinate
 double startAngle,        // ==> Contour Start Angle
 double radiusMax,         // ==> Distance Normal To Curve Direction Within which a Zero must be found
 double radiusMin,         // ==> If Zero Is Found Within A Distance smaller Than radiusMin, Then step size is multiplied by 2.0
 double radiusMax2,        // ==> Distance Normal To Curve Direction. A Zero Must Be Found Within This Distance When Crossing A Triangle Boundary.
 double posError,          // ==> Permitted Positional Error
 double maxStep,           // ==> Maximum Step Size
 double firstStep,         // ==> Starting Step Size
 double startDifference,   // ==> Difference For Estimating Starting Direction
 double xyTolerance,       // ==> Filtering Tolerance
 double *lzxP,             // <== Last Zero x Coordinate
 double *lzyP,             // <== Last Zero y Coordinate
 double *lzzP,             // <== Last Zero z Coordinate
 long   *exitSideP         // <== Contour Exit Side Of Triangle
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   side,numPointsInQue,triangleTrace,loop,process,totalTrgPoints,zeroNotFound,retryCount ;
 double scos,ssin,radMax,contourStep,contourDirection,F1,F2,F3,F12,F13 ;
 double dx12,dy12,qx1,qy1,qx2,qy2,qx3,qy3 ;
 double contourStep12,contourStep23,contourStepDx,contourStepDy ;
 double contourStep13,Pl0,Pl1,Pl2,stepLength ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tracing Contour Over Triangle") ;
    bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",P0,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,pointAddrP(dtmP,P0)->z) ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
    bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
    bcdtmWrite_message(0,0,0,"Contour Start Zero = %10.4lf %10.4lf %10.4lf",zx,zy,zz) ;
    bcdtmWrite_message(0,0,0,"Start Angle        = %12.10lf",startAngle) ;
    bcdtmWrite_message(0,0,0,"startDifference    = %15.12lf",startDifference) ;
    bcdtmWrite_message(0,0,0,"radiusMin          = %15.12lf",radiusMin) ;
    bcdtmWrite_message(0,0,0,"radiusMax          = %15.12lf",radiusMax) ;
    bcdtmWrite_message(0,0,0,"radiusMax2         = %15.12lf",radiusMax2) ;
    bcdtmWrite_message(0,0,0,"maxStep            = %15.12lf",maxStep) ;
    bcdtmWrite_message(0,0,0,"firstStep          = %15.12lf",firstStep) ;
    bcdtmWrite_message(0,0,0,"posError           = %15.12lf",posError) ;
   } 
/*
**  Initialise Parameters For Tracing Contour Triangle
*/
 numPointsInQue   = 0 ;
 totalTrgPoints   = 0 ;
 CL = contourValue ;
 contourStep = maxStep ;
 radMax = radiusMax  ;
 contourDirection = startAngle ;
 dx12 = contourStep * cos(contourDirection) ;
 dy12 = contourStep * sin(contourDirection) ; 
 qx3  = zx ;
 qy3  = zy ;
 *exitSideP = - 1 ;
/*
**  Ensure First Point qx3,qy3 Is Inside Triangle
*/
 while ( ! bcdtmPolyContour_checkIfPointIsInsideTriangle(zx,zy,&side)) 
   {
//    if( contourStep > startDifference && contourStep > 0.001 )
    if( contourStep > startDifference && contourStep > dtmP->ppTol )
      { 
       contourStep = contourStep * 0.5 ; 
       dx12 = contourStep * cos(contourDirection) ;
       dy12 = contourStep * sin(contourDirection) ; 
       zx = qx3 + dx12 ;
       zy = qy3 + dy12 ;
      }
    else
      {
       *exitSideP = side ;
       goto cleanup ; 
      } 
   } 
/*
** Initailise Queue Points For Curve Calculations
*/
 qx2  = qx3 - dx12 ;
 qy2  = qy3 - dy12 ;
 qx1  = qx2 - dx12 ;
 qy1  = qy2 - dy12 ; 
 contourStep12 = contourStep ;
 contourStep23 = contourStep ;
/*
** Calculate Succesive Points On Contour Over Triangle
*/
 triangleTrace = 1 ;
 while ( triangleTrace )
   {
/*
**  Calculate Curve Direction
*/
    contourStep13 =  contourStep23 + contourStep12 ;
    Pl0  =  contourStep23/(contourStep12*contourStep13) ;
    Pl1  = -contourStep13/(contourStep12*contourStep23) ;
    Pl2  =  (contourStep13+contourStep23)/(contourStep13*contourStep23) ;
    contourStepDx =  Pl0*qx1 + Pl1*qx2 + Pl2*qx3 ;
    contourStepDy =  Pl0*qy1 + Pl1*qy2 + Pl2*qy3 ;
    stepLength    =  sqrt(contourStepDx*contourStepDx+contourStepDy*contourStepDy) ;
    contourStepDx =  contourStepDx / stepLength ;
    contourStepDy =  contourStepDy / stepLength ;
    contourDirection = atan2(contourStepDy,contourStepDx) ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"contourDirection = %14.10lf",contourDirection) ;
/*
**  Get Next Contour Zero
*/
    retryCount = 0 ;
    zeroNotFound = 1 ;
    while ( zeroNotFound )
      {
       ++retryCount ;
/*
**     Can Not Find Zero
*/ 
       if( retryCount > 100 ) 
         { 
          if( dbg == 1 ) 
            {
             bcdtmWrite_message(0,0,0,"Cannot Find Zero ** Terminating Trace") ; 
             bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf zz = %10.4lf",zx,zy) ; 
            }
/*
**        Clear Queue
*/
          if( dbg == 3 ) bcdtmWrite_message(0,0,0,"Clearing Cache **** numPointsInQue = %8ld",numPointsInQue) ;
          if (numPointsInQue > 3 ) if( bcdtmPolyContour_storeContourPointInCache(qx1,qy1,contourValue)) goto errexit ;
          if (numPointsInQue > 2 ) if( bcdtmPolyContour_storeContourPointInCache(qx2,qy2,contourValue)) goto errexit ;
          if (numPointsInQue > 1 ) if( bcdtmPolyContour_storeContourPointInCache(qx3,qy3,contourValue)) goto errexit ;
          numPointsInQue = 0 ;
          goto cleanup ; 
         }
/*
**     Look For Zero 
*/
       zeroNotFound  = 0 ;
       contourStepDx = cos(contourDirection) ;
       contourStepDy = sin(contourDirection) ;
       scos = cos(contourDirection+DTM_PYE/2.0) ;
       ssin = sin(contourDirection+DTM_PYE/2.0) ;
/*
**     Find Two Points On Either Side Of Contour
*/
       loop = 0 ;
       process = 1 ;
       while ( process )
         {
          ++loop ;
          zx = qx3 + contourStep * contourStepDx ;
          zy = qy3 + contourStep * contourStepDy ;
          if( contourStep > startDifference && ! bcdtmPolyContour_checkIfPointIsInsideTriangle(zx,zy,&side))
            {
             contourStep = contourStep * 0.5 ;
             if( dbg ) bcdtmWrite_message(0,0,0,"loop = %2ld zx zy Outside Triangle",loop) ;
            }
          else
            {  
             F1 = bcdtmPolyContour_evaluateBivariateFunction(zx,zy,0.0,scos,ssin)  ; 
             F2 = bcdtmPolyContour_evaluateBivariateFunction(zx,zy,radMax,scos,ssin)  ; 
             F3 = bcdtmPolyContour_evaluateBivariateFunction(zx,zy,-radMax,scos,ssin)  ; 
             F12 = F1 * F2 ;
             F13 = F1 * F3 ;
             if     ( F12 <  0.0 && F13  > 0.0 ) process = 0 ;
             else if( F12 >  0.0 && F13  < 0.0 ) process = 0 ;
             else if( F12 == 0.0 || F13 == 0.0 ) process = 0 ;
             else if( F12 < 0.0 && F13 < 0.0 && radMax > 0.00001 ) radMax = radMax * 0.5 ; 
             else if( fabs(F1) < fabs(F2) && fabs(F1) < fabs(F3) && radMax > 0.00001 ) radMax = radMax * 0.5 ;
             else if( contourStep > 0.00001 ) contourStep = contourStep * 0.5 ;
             else process = 0 ; 
            }
         }
/*
**     Test If Zero Found And If Found Calculate Zero
*/
       if     ( F12 < 0.0 && F13 > 0.0 ) process = 1 ;
       else if( F12 > 0.0 && F13 < 0.0 ) process = 2 ;
       else if( F2 == 0.0 )              process = 3 ;
       else if( F3 == 0.0 )              process = 4 ; 
       else if( fabs(F1) < 0.000000001 ) process = 5 ;
/*
**     Zero Not Found
*/
       if( process == 0 )
         {
          if( dbg == 1 )
            { 
             bcdtmWrite_message(0,0,0,"Zero Not Found") ;
             bcdtmWrite_message(0,0,0,"loop = %2ld contourStep = %10.8lf radMax = %11.8lf",loop,contourStep,radMax) ; 
             bcdtmWrite_message(0,0,0,"F13 = %13.10lf F12 = %13.10lf",F13,F12) ; 
             bcdtmWrite_message(0,0,0,"contourStep = %10.8lf radMax = %11.8lf ** F3  = %13.10lf F1  = %13.10lf F2 = %13.10lf",contourStep,radMax,F3,F1,F2) ;   
             bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf",zx,zy) ; 
            }
          if( fabs(F2) < fabs(F3) ) { zx = zx + radMax * scos ; zy = zy + radMax * ssin ; } 
          else                      { zx = zx - radMax * scos ; zy = zy - radMax * ssin ; } 
          contourDirection = bcdtmMath_getAngle(qx3,qy3,zx,zy) ;
//          contourStep = firstStep ;
          radMax = radiusMax ;
          zeroNotFound = 1 ;
         } 
/*
**     Zero Found
*/
       if( process == 1 ) if( bcdtmPolyContour_calculateBivariateFunctionZero(zx,zy,scos,ssin,0.0, radMax,F1,F2,&zx,&zy,&zz) ) goto errexit ;
       if( process == 2 ) if( bcdtmPolyContour_calculateBivariateFunctionZero(zx,zy,scos,ssin,0.0,-radMax,F1,F3,&zx,&zy,&zz) ) goto errexit ;
       if( process == 3 ) { zx = zx +  radMax * scos ; zy = zy +  radMax * ssin ; zz = 0.0 ; } 
       if( process == 4 ) { zx = zx + -radMax * scos ; zy = zy + -radMax * ssin ; zz = 0.0 ; } 
       if( process == 5 ) zz = 0.0 ;
       if( dbg == 2 ) 
         {
          bcdtmWrite_message(0,0,0,"Zero Found") ;
          bcdtmWrite_message(0,0,0,"F3 = %13.10lf F1 = %13.10lf F2 = %13.10lf",F3,F1,F2) ;   
          bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf zz = %13.10lf",zx,zy,zz) ; 
          bcdtmWrite_message(0,0,0,"zx = %10.4lf zy = %10.4lf zz = %13.10lf",zx,zy,bcdtmPolyContour_evaluateBivariateFunction(zx,zy,0.0,0.0,0.0)) ; 
         } 
/*
**     Check If Point Is Inside Triangle
*/
       if( bcdtmPolyContour_checkIfPointIsInsideTriangle(zx,zy,&side))
         {
          if( dbg == 2  ) bcdtmWrite_message(0,0,0,"Point Inside Triangle") ;
/*
**        Check MidPoint Of Line Joining Last Zero To Current Zero
*/
          F1 = bcdtmPolyContour_evaluateBivariateFunction((zx+qx3)/2.0,(zy+qy3)/2.0,0.0,0.0,0.0)  ; 
          if( dbg  == 2 ) bcdtmWrite_message(0,0,0,"MidPoint F1 = %12.10lf",F1) ; 
          if( fabs(F1) > 0.01  ) 
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Midpoint To High ** SubDividing") ;
             contourDirection = bcdtmMath_getAngle(qx3,qy3,zx,zy) ;
             contourStep = contourStep * 0.5 ;
            } 
/*
**        If MidPoint Within Tolerance Plot Point   
*/ 
          else
            {
/*
**           Check Distance To Last Point
*/
             if( dbg == 3 )
               {
                if( ( F1 = bcdtmMath_distance(zx,zy,qx3,qy3)) < 0.0001 )
                  {
                   bcdtmWrite_message(0,0,0,"Last Point = %10.4lf %10.4lf %10.4lf",zx,zy,zz+CL) ;
                   bcdtmWrite_message(0,0,0,"Distance To Next point = %10.6lf",F1) ;
                  }
               }
/*
**           Update Queue
*/
             if( dbg == 3 ) bcdtmWrite_message(0,0,0,"Plotting Point %10.4lf %10.4lf %15.12lf",zx,zy,zz) ;
             ++numPointsInQue   ;
             ++totalTrgPoints   ;
             if( totalTrgPoints > 1000000 ) { bcdtmWrite_message(1,0,0,"Too Many Points On Contour") ; goto errexit ; }
/*
**           If Four Points In Queue Plot P1
*/ 
             if( numPointsInQue == 4 ) 
               { 
                --numPointsInQue ; 
                if( bcdtmPolyContour_storeContourPointInCache(qx1,qy1,contourValue) ) goto errexit ;
               }
/*
**           Reset Queue Points
*/
             contourStep12 = contourStep23 ;
             contourStep23 = bcdtmMath_distance(qx3,qy3,zx,zy) ;
             qx1  = qx2  ;
             qy1  = qy2  ;
             qx2  = qx3  ;
             qy2  = qy3  ;
             qx3  = zx   ;
             qy3  = zy   ;
/*
**           Reset Trace Parameters
*/
             contourStep = firstStep ;
             radMax = radiusMax ;
            }
         }
/*
**     Point Is Outside Triangle
*/
       else
         {
          if( dbg == 2 ) 
            {
             bcdtmWrite_message(0,0,0,"Point Outside Triangle ** Side = %1ld",side) ;
             bcdtmWrite_message(0,0,0,"startDifference = %10.8lf contourStep = %10.8lf Distance To Last Zero = %10.8lf",startDifference,contourStep,bcdtmMath_distance(zx,zy,qx3,qy3)) ;
            }
/*
**        Check Point Is Less Than Exit Distance
*/
//          if( bcdtmMath_distance(zx,zy,qx3,qy3) > startDifference && contourStep > startDifference )
          if( bcdtmMath_distance(zx,zy,qx3,qy3) > radiusMax2 )
            {
             contourDirection = bcdtmMath_getAngle(qx3,qy3,zx,zy) ;
             contourStep = contourStep * 0.5 ;
            }
          else
            {
/*
**           Clear Queue
*/
             if( dbg == 3 ) bcdtmWrite_message(0,0,0,"Clearing Cache **** numPointsInQue = %8ld",numPointsInQue) ;
             if( numPointsInQue > 3 ) if( bcdtmPolyContour_storeContourPointInCache(qx1,qy1,contourValue)) goto errexit ;
             if( numPointsInQue > 2 ) if( bcdtmPolyContour_storeContourPointInCache(qx2,qy2,contourValue)) goto errexit ;
             if( numPointsInQue > 1 ) if( bcdtmPolyContour_storeContourPointInCache(qx3,qy3,contourValue)) goto errexit ;
             numPointsInQue = 0 ;
/*
**           Filter Points Traced Over Triangle
*/
//         if( bcdtmPolyContour_storeContourPointInCache(4,0.0,0.0,0.0,xyTolerance,zTolerance) ) goto errexit ;
/*
**           Terminate Trace Over Triangle
*/
             triangleTrace = 0 ;
/*
**           Set Exit Side Of Triangle
*/
             *exitSideP = side ;
/*
**           Set Value Of Last Zero Plotted
*/
             *lzxP = qx3 ;
             *lzyP = qy3 ;
             *lzzP = contourValue ;
            }
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
 CL = 0.0 ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Contour Over Triangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tracing Contour Over Triangle Error") ;
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
BENTLEYDTM_Private int bcdtmPolyContour_getNextTraceTriangleForContourDtmObject
(
 BC_DTM_OBJ *dtmP,
 double contourValue,
 long   ExitSide,
 long   CloseFlag,
 double Sx,
 double Sy,
 DTM_SMOOTH_CONTOUR_INDEX *contourIndexP,
 long numLines,
 DTM_SMOOTH_CONTOUR_ZERO_POINT *zeroPtsP,
 long   numZeroPts,
 double Eps,
 double Fstep,
 double Rmax,
 double XYTolerance,
 double ZTolerance,
 double *partDerivP,
 long   *P0,
 long   *P1,
 long   *P2,
 double *Zx,
 double *Zy,
 double *Zz,
 long   *StartType,
 double *StartAngle,
 long   *TinTrace
)
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_TRACE_VALUE(0) ;
 long      Na,Nc,Cz,Nz,Np0,Np1,Np2,Sp0,Sp1,Sp2,Direction,Offset ;
 double    Dp,Dz,X1,Y1,Z1,X2,Y2,Z2,chkZ ;
 DTM_SMOOTH_CONTOUR_INDEX *Cline,*Zline ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Next Trace Triangle For Contour") ;
/*
** Initialise
*/
 Sp0 = *P0 ; Sp1 = *P1 ; Sp2 = *P2 ;
/*
** Write Contour Zero Statistics For Exiting Triangle Lines ** Development Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Completed Scan Over Triangle %6ld %6ld %6ld",Sp0,Sp1,Sp2) ;
    bcdtmWrite_message(0,0,0,"Sp0->hPtr = %8ld",nodeAddrP(dtmP,Sp0)->hPtr ) ;
    bcdtmWrite_message(0,0,0,"Sp1->hPtr = %8ld",nodeAddrP(dtmP,Sp1)->hPtr ) ;
    bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",Sp0,pointAddrP(dtmP,Sp0)->x,pointAddrP(dtmP,Sp0)->y,pointAddrP(dtmP,Sp0)->z) ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",Sp1,pointAddrP(dtmP,Sp1)->x,pointAddrP(dtmP,Sp1)->y,pointAddrP(dtmP,Sp1)->z) ;
    bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",Sp2,pointAddrP(dtmP,Sp2)->x,pointAddrP(dtmP,Sp2)->y,pointAddrP(dtmP,Sp2)->z) ;
    bcdtmWrite_message(0,0,0,"Exit Side = %1ld",ExitSide) ;
    bcdtmWrite_message(0,0,0,"Last Zero = %10.4lf %10.4lf %10.4lf",*Zx,*Zy,*Zz) ;
    if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,Sp0,Sp1,&Offset)) goto errexit ; 
    Cline = contourIndexP + Offset ;
    bcdtmWrite_message(0,0,0,"Cline[%6ld] ** P1 = %6ld P2 = %6ld  NumZeros = %6ld Zmin = %10.4lf Zmax = %10.4lf",Offset,Cline->P1,Cline->P2,Cline->NumZero,Cline->Zmin,Cline->Zmax) ;
    if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,Sp1,Sp2,&Offset)) goto errexit ; 
    Cline = contourIndexP + Offset ;
    bcdtmWrite_message(0,0,0,"Cline[%6ld] ** P1 = %6ld P2 = %6ld  NumZeros = %6ld Zmin = %10.4lf Zmax = %10.4lf",Offset,Cline->P1,Cline->P2,Cline->NumZero,Cline->Zmin,Cline->Zmax) ;
    if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,Sp2,Sp0,&Offset)) goto errexit ; 
    Cline = contourIndexP + Offset ;
    bcdtmWrite_message(0,0,0,"Cline[%6ld] ** P1 = %6ld P2 = %6ld  NumZeros = %6ld Zmin = %10.4lf Zmax = %10.4lf",Offset,Cline->P1,Cline->P2,Cline->NumZero,Cline->Zmin,Cline->Zmax) ;
   }
/*
** Get Triangle Side Where Contour Exits
*/
 if( ExitSide == 0 ) { Np0 = Sp0 ; Np1 = Sp1  ; Np2 = Sp2 ; }
 if( ExitSide == 1 ) { Np0 = Sp2 ; Np1 = Sp0  ; Np2 = Sp1 ; }
 if( ExitSide == 2 ) { Np0 = Sp2 ; Np1 = Sp1  ; Np2 = Sp0 ; }
 if( Np0 > Np1 )     { Na  = Np0 ; Np0 = Np1  ; Np1 = Na  ; }
 if(( Na = bcdtmList_nextAntDtmObject(dtmP,Np0,Np1)) < 0 ) goto errexit ; 
 if(( Nc = bcdtmList_nextClkDtmObject(dtmP,Np0,Np1)) < 0 ) goto errexit ; 
 if( Na != Np2 ) { Np2 = Na ; Direction = -1 ; }
 else            { Np2 = Nc ; Direction =  1 ; }
/*
**  Get Zero For Triangle Side Where Contour Exists
*/
 if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,Np0,Np1,&Offset)) goto errexit ; 
 Cline = contourIndexP + Offset ;
/*
** Write Exit Side Zeros ** Development Only
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Exit Side Cline [%6ld] ** P1 = %6ld P2 = %6ld  NumZeros = %6ld Zmin = %10.4lf Zmax = %10.4lf",Offset,Cline->P1,Cline->P2,Cline->NumZero,Cline->Zmin,Cline->Zmax) ;
    for( Nz = 0 ; Nz < Cline->NumZero ; ++Nz ) bcdtmWrite_message(0,0,0,"Exit Zero = %12.10lf",Cline->Zeros[Nz]) ;
   }
/*
** If There Are Zeros Get Closest Zero To Continue Trace Over Side
*/ 
 *StartType = 1 ;
 if( Cline->NumZero > 0 )
   {
/*
** Set Start Type To Line
*/
    *StartType = 2 ;
/*
** Get Closest Zero
*/
    for( Nz = 0 ; Nz < Cline->NumZero ; ++Nz )
      { 
       bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinatesAlongTinLineDtmObject(dtmP,Np0,Np1,contourValue,Cline->Zeros[Nz],&X1,&Y1,&Z1) ;
       Dp = bcdtmMath_distance(*Zx,*Zy,X1,Y1) ;
       if( Nz == 0 || Dp < Dz ) { Dz = Dp ; Cz = Nz ; X2 = X1 ; Y2 = Y1 ; Z2 = Z1 ; }
      }
/*
**  Check For Zero Tin Points At Line Ends
*/
    if( pointAddrP(dtmP,Cline->P1)->z == contourValue && bcdtmMath_distance(*Zx,*Zy,pointAddrP(dtmP,Cline->P1)->x,pointAddrP(dtmP,Cline->P1)->y) < Dz ) *StartType = 1 ;
    if( pointAddrP(dtmP,Cline->P2)->z == contourValue && bcdtmMath_distance(*Zx,*Zy,pointAddrP(dtmP,Cline->P2)->x,pointAddrP(dtmP,Cline->P2)->y) < Dz ) *StartType = 1 ;
/*
**  Next Scan Starts From Line
*/
    if( *StartType == 2 )
      { 
/*
**      Set Zero Start Coordinates
*/
       *Zx = X2 ; *Zy = Y2 ; *Zz = Z2 ;
/*
**      Remove Zero From Zero List
*/
        for( Nz = Cz ; Nz < Cline->NumZero - 1 ; ++Nz ) Cline->Zeros[Nz] = Cline->Zeros[Nz+1] ;
        --Cline->NumZero ;
/*
**     Check Exit Zero Coordiantes  ** Development Only
*/
       if( cdbg )
         { 
          chkZ = bcdtmPolyContour_evaluateBivariateFunction(*Zx,*Zy,0.0,0.0,0.0) + CL ; 
          if( fabs(chkZ-*Zz) > 0.00001 )
            {
             bcdtmWrite_message(1,0,0,"Error With Exit Contour Zero Coordinates") ;
             bcdtmWrite_message(0,0,0,"Zero = %10.4lf %10.4lf %10.4lf",Zx,Zy,Zz) ;
             bcdtmWrite_message(0,0,0,"Chkz = %10.4lf %10.4lf %10.4lf",Zx,Zy,chkZ) ;
             goto errexit ;
            }
         }
/*
**     Set Triangle Points To Continue Trace Over Adjoining Triangle
*/ 
       *P0 = Np0 ; *P1 = Np1 ; *P2 = Np2 ;
/*
**     If Side P0-P1 Is A Hull Line Terminate Trace Over Tin And Plot Zero Coordinate
*/         
       if( bcdtmPolyContour_testForHullLineDtmObject(dtmP,*P0,*P1) ) 
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Terminating At Hull Line") ;
          *TinTrace = 0 ;
          if( bcdtmPolyContour_storeContourPointInCache(*Zx,*Zy,contourValue) ) goto errexit ;
         } 
      }
   }
/*
** No More Zeros On Triangle Side
*/
 if( *StartType == 1 )
   {
/*
** Check For Contour Passing Through Tin Point
*/
    Np0 = dtmP->nullPnt ;
    if( pointAddrP(dtmP,Sp0)->z == contourValue ) { Np0 = Sp0 ; Np1 = Sp1 ; Np2 = Sp2 ; Dz = bcdtmMath_distance(*Zx,*Zy,pointAddrP(dtmP,Sp0)->x,pointAddrP(dtmP,Sp0)->y) ; }
    if( pointAddrP(dtmP,Sp1)->z == contourValue ) { Dp = bcdtmMath_distance(*Zx,*Zy,pointAddrP(dtmP,Sp1)->x,pointAddrP(dtmP,Sp1)->y) ; if( Np0 == dtmP->nullPnt || Dp < Dz ) { Np0 = Sp1 ; Np1 = Sp2 ; Np2 = Sp0 ; Dz = Dp ; } }  
    if( pointAddrP(dtmP,Sp2)->z == contourValue ) { Dp = bcdtmMath_distance(*Zx,*Zy,pointAddrP(dtmP,Sp2)->x,pointAddrP(dtmP,Sp2)->y) ; if( Np0 == dtmP->nullPnt || Dp < Dz ) { Np0 = Sp2 ; Np1 = Sp0 ; Np2 = Sp1 ; Dz = Dp ; } }  
/*
** Remove Zero For Exiting Tin Point
*/
    if( Np0 != dtmP->nullPnt ) if( bcdtmPolyContour_findEntryInZeroTinPointTable(zeroPtsP,numZeroPts,Np0,Np1,Np2,P1,P2,StartAngle)) goto errexit ;
/*
** Test For Internal Contour Terminating At Start Point
*/
    if( CloseFlag && bcdtmMath_distance(Sx,Sy,*Zx,*Zy) <= Fstep )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Terminating At Start Point") ;
       *TinTrace = 0 ;
       if( bcdtmPolyContour_storeContourPointInCache(Sx,Sy,contourValue) ) goto errexit ;
      }
/*
** Contour Passes Through Point
*/
    else
      {
       if( Np0 != dtmP->nullPnt )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Zero At Triangle Point %6ld ** %10.4lf %10.4lf %10.4lf",Np0,pointAddrP(dtmP,Np0)->x,pointAddrP(dtmP,Np0)->y,pointAddrP(dtmP,Np0)->z ) ;
          *StartType = 1 ;
/*
**        Termnate Trace If Hull Point
*/
          if( bcdtmList_testForPointOnAnIslandOrVoidHullDtmObject(dtmP,Np0))
            {
             *TinTrace = 0 ;
             if( bcdtmPolyContour_storeContourPointInCache(pointAddrP(dtmP,Np0)->x,pointAddrP(dtmP,Np0)->y,contourValue) ) goto errexit ;
            }
/*
**        Else Get Adjoining Triangle To Continue Trace At Tin Point
*/
          else 
            {
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"Last Triangle") ;  
                bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",Sp0,pointAddrP(dtmP,Sp0)->x,pointAddrP(dtmP,Sp0)->y,pointAddrP(dtmP,Sp0)->z) ;
                bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",Sp1,pointAddrP(dtmP,Sp1)->x,pointAddrP(dtmP,Sp1)->y,pointAddrP(dtmP,Sp1)->z) ;
                bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",Sp2,pointAddrP(dtmP,Sp2)->x,pointAddrP(dtmP,Sp2)->y,pointAddrP(dtmP,Sp2)->z) ;
               }
/*
**           Find Entry In Zero Tin Point Table For Zero Tin Point
*/
             if( bcdtmPolyContour_getNextScanTriangleAtTinPointDtmObject(dtmP,Np0,Np1,Np2,Eps,partDerivP,P0,P1,P2,StartAngle)) goto errexit ;
             *P0 = Np0 ;
             if( bcdtmPolyContour_findEntryInZeroTinPointTable(zeroPtsP,numZeroPts,*P0,dtmP->nullPnt,dtmP->nullPnt,P1,P2,StartAngle)) goto errexit ;
             if( *P1 == dtmP->nullPnt ) *TinTrace = 0 ;
             else
               {
                if( dbg )
                  {
                   bcdtmWrite_message(0,0,0,"New Triangle From Tin Point") ;  
                   bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",*P0,pointAddrP(dtmP,*P0)->x,pointAddrP(dtmP,*P0)->y,pointAddrP(dtmP,*P0)->z) ;
                   bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",*P1,pointAddrP(dtmP,*P1)->x,pointAddrP(dtmP,*P1)->y,pointAddrP(dtmP,*P1)->z) ;
                   bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",*P2,pointAddrP(dtmP,*P2)->x,pointAddrP(dtmP,*P2)->y,pointAddrP(dtmP,*P2)->z) ;
                   bcdtmWrite_message(0,0,0,"Start Angle = %12.10lf",*StartAngle) ;
                  }
                *Zx = pointAddrP(dtmP,*P0)->x ;
                *Zy = pointAddrP(dtmP,*P0)->y ;
                *Zz = pointAddrP(dtmP,*P0)->z ;
               }
            }
         }
/*
**     Error Detecteted In Trace If Contour Does Not Pass Through A Point
*/
       else 
         { 
/*
**        Scan Triangle Edges  For Closest Zero Point ** Fudge
*/
          Zline = nullptr ;
          *P0 = *P1 = *P2 = dtmP->nullPnt ;
          if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,Sp0,Sp1,&Offset)) goto errexit ; 
          Cline = contourIndexP + Offset ;
          for( Nz = 0 ; Nz < Cline->NumZero ; ++Nz )
            { 
             bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinatesAlongTinLineDtmObject(dtmP,Cline->P1,Cline->P2,contourValue,Cline->Zeros[Nz],&X1,&Y1,&Z1) ;
             Dp = bcdtmMath_distance(*Zx,*Zy,X1,Y1) ;
             if( *P0 == dtmP->nullPnt || Dp < Dz ) { Dz = Dp ; Cz = Nz ; Zline = Cline ; *P0 = Sp0 ; *P1 = Sp1 ; X2 = X1 ; Y2 = Y1 ; Z2 = Z1 ; }
            }
          if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,Sp1,Sp2,&Offset)) goto errexit ; 
          Cline = contourIndexP + Offset ;
          for( Nz = 0 ; Nz < Cline->NumZero ; ++Nz )
            { 
             bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinatesAlongTinLineDtmObject(dtmP,Cline->P1,Cline->P2,contourValue,Cline->Zeros[Nz],&X1,&Y1,&Z1) ;
             Dp = bcdtmMath_distance(*Zx,*Zy,X1,Y1) ;
             if( *P0 == dtmP->nullPnt || Dp < Dz ) { Dz = Dp ; Cz = Nz ; Zline = Cline ; *P0 = Sp1 ; *P1 = Sp2 ; X2 = X1 ; Y2 = Y1 ; Z2 = Z1 ; }
            }

          if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,Sp2,Sp0,&Offset)) goto errexit ; 
          Cline = contourIndexP + Offset ;
          for( Nz = 0 ; Nz < Cline->NumZero ; ++Nz )
            { 
             bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinatesAlongTinLineDtmObject(dtmP,Cline->P1,Cline->P2,contourValue,Cline->Zeros[Nz],&X1,&Y1,&Z1) ;
             Dp = bcdtmMath_distance(*Zx,*Zy,X1,Y1) ;
             if( *P0 == dtmP->nullPnt || Dp < Dz ) { Dz = Dp ; Cz = Nz ; Zline = Cline ; *P0 = Sp2 ; *P1 = Sp0 ; X2 = X1 ; Y2 = Y1 ; Z2 = Z1 ; }
            }
          if( Zline != nullptr )
            {
             for( Nz = Cz ; Nz < Zline->NumZero - 1 ; ++Nz ) Zline->Zeros[Nz] = Zline->Zeros[Nz+1] ;
              --Zline->NumZero ;
             *Zx = X2 ; *Zy = Y2 ; *Zz = Z2 ;
             if( *P0 > *P1 ) { *P2 = *P0 ; *P0 = *P1 ; *P1 = *P2 ; }
             if(( *P2 = bcdtmList_nextClkDtmObject(dtmP,*P0,*P1)) < 0 ) goto errexit ;
             if( *P2 == Sp0 || *P2 == Sp1 || *P3 == Sp2 )
               {
                if(( *P2 = bcdtmList_nextAntDtmObject(dtmP,*P0,*P1)) < 0 ) goto errexit ;
               }
             *TinTrace = 1 ;  
            }
          else 
            {
             bcdtmWrite_message(0,0,0,"No Corresponding Zero On Exit Side Of Triangle - Terminating Trace") ; 
             bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",Sp0,pointAddrP(dtmP,Sp0)->x,pointAddrP(dtmP,Sp0)->y,pointAddrP(dtmP,Sp0)->z) ;
             bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",Sp1,pointAddrP(dtmP,Sp1)->x,pointAddrP(dtmP,Sp1)->y,pointAddrP(dtmP,Sp1)->z) ;
             bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",Sp2,pointAddrP(dtmP,Sp2)->x,pointAddrP(dtmP,Sp2)->y,pointAddrP(dtmP,Sp2)->z) ;
             *TinTrace = 0 ;
             goto cleanup ; 
            }
         }
      }
   } 
/*
** Write Next Triangle To Scan ** Development Only
*/
 if( dbg && *TinTrace )
   {
    bcdtmWrite_message(0,0,0,"Next Triangle To Scan") ;
    bcdtmWrite_message(0,0,0,"Np0 = %6ld ** %10.4lf %10.4lf %10.4lf",*P0,pointAddrP(dtmP,*P0)->x,pointAddrP(dtmP,*P0)->y,pointAddrP(dtmP,*P0)->z) ;
    bcdtmWrite_message(0,0,0,"Np1 = %6ld ** %10.4lf %10.4lf %10.4lf",*P1,pointAddrP(dtmP,*P1)->x,pointAddrP(dtmP,*P1)->y,pointAddrP(dtmP,*P1)->z) ;
    bcdtmWrite_message(0,0,0,"Np2 = %6ld ** %10.4lf %10.4lf %10.4lf",*P2,pointAddrP(dtmP,*P2)->x,pointAddrP(dtmP,*P2)->y,pointAddrP(dtmP,*P2)->z) ;
   } 
/*
** Clean Up
*/
 cleanup :
 CL = 0 ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Next Trace Triangle For Contour Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Next Trace Triangle For Contour Error") ;
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
BENTLEYDTM_Private int bcdtmPolyContour_getContourStartDirectionDtmObject
(
 BC_DTM_OBJ *dtmP,
 long P0,
 long P1,
 long P2,
 double Zx,
 double Zy,
 double Zz,
 double Eps,
 double *StartAngle
)
/*
** This Function Calculates The Contour Start Angle From Side P0P1 Of Triangle P0P1P2
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_TRACE_VALUE(0) ;
 long   direction,process ;
 double angside,angnorm,F1,F2,F3,F12,F23,X1,Y1,X2,Y2,X3,Y3,Z3 ;
/*
** Write Status Message ** Development Only
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Calculating Contour Start Angle") ;
    bcdtmWrite_message(0,0,0,"Zero = %10.4lf %10.4lf %15.12lf",Zx,Zy,Zz) ;
   } 
/*
** Determine Direction ** ClockWise = -1  AntiClockWise = 1 
*/
 direction = bcdtmMath_pointSideOfDtmObject(dtmP,P0,P1,P2) ;
/*
** Get Angle Of P0P1
*/
 angside = bcdtmMath_getPointAngleDtmObject(dtmP,P0,P1) ;
/*
** Get Angle Of Normal To Side P0P1
*/
 angnorm = angside  + (double) direction * DTM_PYE / 2.0 ;
 if( angnorm <  0.0 ) angnorm += DTM_2PYE ;
 if( angnorm > DTM_2PYE ) angnorm -= DTM_2PYE ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Direction = %2ld ** Angle Side = %12.10lf Angle Normal = %12.8lf",direction,angside,angnorm) ;
/*
** Calculate Bivariate Function Values
*/
 CL = Zz ;
 process = 1 ;
 while ( process )
   {
    F1 = bcdtmPolyContour_evaluateBivariateFunction(Zx,Zy, Eps,cos(angside),sin(angside))  ; 
    F2 = bcdtmPolyContour_evaluateBivariateFunction(Zx,Zy, Eps,cos(angnorm),sin(angnorm))  ; 
    F3 = bcdtmPolyContour_evaluateBivariateFunction(Zx,Zy,-Eps,cos(angside),sin(angside))  ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"Eps = %10.8lf ** F1 = %10.4lf F2 = %10.4lf F3 = %10.4lf",Eps,F1,F2,F3) ;
    F12 = F1 * F2 ;
    F23 = F2 * F3 ;
    if     ( fabs(F1) < 0.00000001  ) process = 0 ;
    else if( F12 < 0.0 && F23 > 0.0 ) process = 0 ;
    else if( F12 > 0.0 && F23 < 0.0 ) process = 0 ;
    else                              Eps = Eps / 2.0 ;
    if( Eps < 0.00000001 ) process = 0 ;
   }
/*
** Test If Solution Found
*/
 if     ( fabs(F1) < 0.00000001  ) process = 1 ;
 else if( F12 < 0.0 && F23 > 0.0 ) process = 2 ;
 else if( F12 > 0.0 && F23 < 0.0 ) process = 3 ;
/*
** Caculate Start Angle
*/
 switch ( process )
   {
    case 0 :
      bcdtmWrite_message(1,0,0,"Cannot Determine Starting Angle For Contour") ;
      goto errexit ;
    break  ;

    case 1 :   /* Starting Angle Normal To Side  */
      if( dbg ) bcdtmWrite_message(0,0,0,"Start Angle Normal To Side") ; 
      *StartAngle = angnorm ;
    break  ;

    case 2 :   /* Contour Zero Between F1 And F2 */
      if( dbg ) bcdtmWrite_message(0,0,0,"Start Angle Between F1 And F2") ; 
      X2 = Zx + Eps * cos(angnorm) ;
      Y2 = Zy + Eps * sin(angnorm) ;
      X1 = Zx + Eps * cos(angside) ;
      Y1 = Zy + Eps * sin(angside) ;
      if( bcdtmPolyContour_calculateBivariateFunctionZeroBetweenPoints(X1,Y1,X2,Y2,&X3,&Y3,&Z3)) goto errexit ;
      *StartAngle = bcdtmMath_getAngle(Zx,Zy,X3,Y3) ;
    break  ;
 
    case 3 :   /* Contour Zero Between F3 And F2 */
      if( dbg ) bcdtmWrite_message(0,0,0,"Start Angle Between F3 And F2") ; 
      X2 = Zx + Eps * cos(angnorm) ;
      Y2 = Zy + Eps * sin(angnorm) ;
      X1 = Zx - Eps * cos(angside) ;
      Y1 = Zy - Eps * sin(angside) ;
      if( bcdtmPolyContour_calculateBivariateFunctionZeroBetweenPoints(X1,Y1,X2,Y2,&X3,&Y3,&Z3)) goto errexit ;
      *StartAngle = bcdtmMath_getAngle(Zx,Zy,X3,Y3) ;
    break  ;
   } ;
/*
** Reset Contour level
*/
 CL = 0.0 ;
/*
 if( dbg ) bcdtmWrite_message(0,0,0,"Contour Zero        = %10.4lf %10.4lf %15.12lf",X2,Y2,Z2) ; 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Contour Zero        = %10.4lf %10.4lf %15.12lf",X3,Y3,Z3) ; 
 if( dbg ) bcdtmWrite_message(0,0,0,"Contour Start Angle = %12.10lf",*StartAngle) ;
/*
**  Check Calculated Start Angle Direction ** Development Only
*/
 if( cdbg )
   {
    if( direction > 0 )   { angnorm = angside + DTM_PYE ; }
    else                  { angnorm = angside ; angside = angside + DTM_PYE ; }
    if( angside > DTM_2PYE ) angside -= DTM_2PYE ;
    if( angnorm > DTM_2PYE ) angnorm -= DTM_2PYE ;
    if( angnorm     < angside ) angnorm     += DTM_2PYE ;        
    if( *StartAngle < angside ) *StartAngle += DTM_2PYE ;
    if( *StartAngle > angnorm ) 
      {
       bcdtmWrite_message(1,0,0,"Error With Contour Calculated Starting Angle") ;
       bcdtmWrite_message(0,0,0,"angside = %12.10lf  StartAngle = %12.10lf angnorm = %12.10lf",angside,*StartAngle,angnorm) ;
       goto errexit ;
      } 
    if( *StartAngle > DTM_2PYE ) *StartAngle -= DTM_2PYE ;
   }
/*
** Clean Up
*/
 cleanup :
 CL = 0.0 ;
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Contour Start Angle Completed") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Contour Start Angle Error") ;
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
BENTLEYDTM_Private int bcdtmPolyContour_getStartScanTriangleAtTinPointDtmObject(BC_DTM_OBJ *dtmP,long P0,double Eps,double *partDerivP,long *Np0,long *Np1,long *Np2,double *Angle)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   clc,P1,P2 ;
 double F1,F2,ang1,ang2,X1,Y1,X2,Y2,z ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Start Scan Triangle At Tin Point %6ld",P0) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",P0,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,pointAddrP(dtmP,P0)->z) ;
/*
** Initialise
*/
 *Angle = 0 ;
 *Np0 = *Np1 = *Np2 = dtmP->nullPnt ;
/*
** Scan About Point P0
*/
 if( ( clc = nodeAddrP(dtmP,P0)->cPtr ) != dtmP->nullPtr )
   {
    if(( P2 = bcdtmList_nextAntDtmObject(dtmP,P0,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    ang2 = bcdtmMath_getPointAngleDtmObject(dtmP,P0,P2) ; 
    X2   = pointAddrP(dtmP,P0)->x + Eps * cos(ang2) ;
    Y2   = pointAddrP(dtmP,P0)->y + Eps * sin(ang2) ;
    while ( clc != dtmP->nullPtr )
      {
       P1   = clistAddrP(dtmP,clc)->pntNum ; 
       clc  = clistAddrP(dtmP,clc)->nextPtr ;
       ang1 = bcdtmMath_getPointAngleDtmObject(dtmP,P0,P1) ; 
       X1   = pointAddrP(dtmP,P0)->x + Eps * cos(ang1) ;
       Y1   = pointAddrP(dtmP,P0)->y + Eps * sin(ang1) ;
       F1   =  F2 = 1.0 ;  
       if( bcdtmList_testLineDtmObject(dtmP,P1,P2) )
         {
/*
** Calculate Polynomial Coefficients
*/
          CL = 0.0 ;
          if( bcdtmPolyContour_calculatePolynomialCoefficientsForPointTriangleDtmObject(dtmP,P0,P1,P2,partDerivP)) goto errexit ;
          CL = pointAddrP(dtmP,P0)->z ; 
/*
** Calculate Function Values At A Distance Of Eps From P0
*/
          F2   = bcdtmPolyContour_evaluateBivariateFunction(X2,Y2,0.0,0.0,0.0)  ; 
          F1   = bcdtmPolyContour_evaluateBivariateFunction(X1,Y1,0.0,0.0,0.0)  ; 
         }
/*
** Reset For Next Triangle
*/
       if( F1*F2 > 0.0 )
         {        
          P2   = P1   ; 
          ang2 = ang1 ;
          X2   = X1   ;
          Y2   = Y1   ; 
         }
       else  clc = dtmP->nullPtr ;
      }
   }
/*
** If Zero Found
*/
 if( F1*F2 < 0.0 ) 
   { 
/*
** Set Next Triangle
*/
    *Np0 = P0 ; *Np1 = P1 ; *Np2 = P2 ;
/*
** Calculate Zero Value
*/
    ang1 = bcdtmMath_getAngle(X1,Y1,X2,Y2) ;
    if( bcdtmPolyContour_calculateBivariateFunctionZero(X1,Y1,cos(ang1),sin(ang1),0.0,bcdtmMath_distance(X1,Y1,X2,Y2),F1,F2,&X2,&Y2,&z) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Zero = %10.4lf %10.4lf %10.4lf",X2,Y2,z) ;
   }
/*
** Zero Not Found
*/
 else if( dbg ) bcdtmWrite_message(0,0,0,"Zero Not Found For Start Triangle At Tin Point") ;
/*
** Calculate Start Angle
*/
 *Angle = bcdtmMath_getAngle(pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,X2,Y2) ;
/*
** Clean Up
*/
 cleanup :
 CL = 0.0 ;
/*
**  Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Start Trace Triangle At Tin Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Start Trace Triangle At Tin Point Error") ;
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
BENTLEYDTM_Private int bcdtmPolyContour_getNextScanTriangleAtTinPointDtmObject(BC_DTM_OBJ *dtmP,long P0,long P1,long P2,double Eps,double *partDerivP,long *Np0,long *Np1,long *Np2,double *Angle)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   P3 ;
 double F1,F2,ang1,ang2,X1,Y1,X2,Y2,z ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Next Trace Triangle At Tin Point %6ld",P0) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",P0,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,pointAddrP(dtmP,P0)->z) ;
    bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
    bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
   }
/*
** Initialise
*/
 *Angle = 0 ;
 *Np0 = *Np1 = *Np2 = dtmP->nullPnt ;
/*
** Set Triangle Points ClockWise About P0
*/
 if( bcdtmMath_pointSideOfDtmObject(dtmP,P0,P1,P2) > 0 ) { P3 = P1 ; P1 = P2 ; P2 = P3 ; }
 P3 = P1 ;
/*
** Scan About Point P0
*/
 ang2 = bcdtmMath_getPointAngleDtmObject(dtmP,P0,P2) ; 
 X2   = pointAddrP(dtmP,P0)->x + Eps * cos(ang2) ;
 Y2   = pointAddrP(dtmP,P0)->y + Eps * sin(ang2) ;
 do
   {
    if(( P1 = bcdtmList_nextClkDtmObject(dtmP,P0,P2)) < 0 ) goto errexit ;
    ang1 = bcdtmMath_getPointAngleDtmObject(dtmP,P0,P1) ; 
    X1   = pointAddrP(dtmP,P0)->x + Eps * cos(ang1) ;
    Y1   = pointAddrP(dtmP,P0)->y + Eps * sin(ang1) ;
/*
** Calculate Polynomial Coefficients
*/
    CL = 0.0 ;
    if( bcdtmPolyContour_calculatePolynomialCoefficientsForPointTriangleDtmObject(dtmP,P0,P1,P2,partDerivP)) goto errexit ;
    CL = pointAddrP(dtmP,P0)->z ;
/*
** Calculate Function Values At A Distance Of Eps From P0
*/
    F2 = bcdtmPolyContour_evaluateBivariateFunction(X2,Y2,0.0,0.0,0.0)  ; 
    F1 = bcdtmPolyContour_evaluateBivariateFunction(X1,Y1,0.0,0.0,0.0)  ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"P1 = %6ld P2 = %6ld F1 = %10.6lf F2 = %10.6lf",P1,P2,F1,F2) ;
/*
** Test For Zero Between F1 And F2 I.E F1*F2 < 0.0 
*/
    if( F1*F2 >= 0.0 )
      { 
       P2   = P1 ; 
       ang2 = ang1 ;
       X2   = X1   ;
       Y2   = Y1   ; 
      }
   } while ( P2 != P3 && F1*F2 >= 0.0 ) ;
/*
** Check If Zero Found
*/
 if( F1*F2 >= 0.0 ) { bcdtmWrite_message(1,0,0,"Zero Not Found For Adjoining Triangle At Tin Point") ; goto errexit ; }
/*
** Set Next Triangle
*/
 *Np0 = P0 ; *Np1 = P1 ; *Np2 = P2 ;
/*
** Calculate Zero Value
*/
 ang1 = bcdtmMath_getAngle(X1,Y1,X2,Y2) ;
 if( bcdtmPolyContour_calculateBivariateFunctionZero(X1,Y1,cos(ang1),sin(ang1),0.0,bcdtmMath_distance(X1,Y1,X2,Y2),F1,F2,&X2,&Y2,&z) ) goto errexit ;
/*
** Calculate Start Angle
*/
 *Angle = bcdtmMath_getAngle(pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,X2,Y2) ;
/*
** Clean Up
*/
 cleanup :
 CL = 0.0 ;
/*
**  Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Next Trace Triangle At Tin Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Next Trace Triangle At Tin Point Error") ;
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
BENTLEYDTM_Private int bcdtmPolyContour_removeAllZerosAtPointDtmObject(BC_DTM_OBJ *dtmP,long P,DTM_SMOOTH_CONTOUR_INDEX *contourIndexP,long numLines )
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long      p1,clc,Offset,nz;
 DTM_SMOOTH_CONTOUR_INDEX *Cline ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing All Zeros At Point %6ld",P) ;
/*
** Scan Point And Get All Lines Connected To Point
*/
 clc = nodeAddrP(dtmP,P)->cPtr ;
 while( clc != dtmP->nullPtr )
   {
    p1 = clistAddrP(dtmP,clc)->pntNum ;
/*
** Get Offset In contourIndexP For Line Pp1
*/
    if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,P,p1,&Offset)) goto errexit ; 
    Cline = contourIndexP + Offset ;
    if( dbg ) bcdtmWrite_message(0,0,0,"00 Cline [%6ld] ** P1 = %6ld P2 = %6ld  NumZeros = %6ld Zero = %8.6lf Zmin = %10.4lf Zmax = %10.4lf",Offset,Cline->P1,Cline->P2,Cline->NumZero,Cline->Zeros[0],Cline->Zmin,Cline->Zmax) ;
    if( Cline->NumZero > 0 )
      {  
       if( Cline->P1 == P && Cline->Zeros[0] == 0.0 )
         {
          for( nz = 0 ; nz < Cline->NumZero - 1 ; ++nz ) Cline->Zeros[nz] = Cline->Zeros[nz+1] ;
          --Cline->NumZero ;
         }
       if( Cline->P2 == P && fabs(Cline->Zeros[Cline->NumZero-1] - 1.0 ) < 0.0000001 ) --Cline->NumZero ;
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"01 Cline [%6ld] ** P1 = %6ld P2 = %6ld  NumZeros = %6ld Zero = %8.6lf Zmin = %10.4lf Zmax = %10.4lf",Offset,Cline->P1,Cline->P2,Cline->NumZero,Cline->Zeros[0],Cline->Zmin,Cline->Zmax) ;
/*
** Get Next Line
*/
    clc = clistAddrP(dtmP,clc)->nextPtr ;
   }
/*
** Clean Up
*/
 cleanup :
 CL = 0 ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing All Zeros At Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing All Zeros At Point Error") ;
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
BENTLEYDTM_Private int bcdtmPolyContour_checkIfPointIsInsideTriangle(double Xp,double Yp,long *Side)
{
 double u,v ;
/*
** Transform Coordinates To UV System
*/
 Xp = Xp - X[2] ;
 Yp = Yp - Y[2] ;
 u  = Xp * AP + Yp * BP ;
 v  = Xp * CP + Yp * DP ;
/*
** Check If Point Is Outside Triangle
*/
 *Side = 0 ; if ( u < 0.0     ) return(0) ;
 *Side = 1 ; if ( v < 0.0     ) return(0) ;
 *Side = 2 ; if ( v > 1.0 - u ) return(0) ;
/*
** Point Is Inside Triangle
*/
 *Side = -1 ;
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmPolyContour_testForVoidHullLineDtmObject(BC_DTM_OBJ *dtmP,long point1,long point2,long *directionP)
/*
** This Function Tests If The Line point1-point2 is On A Void Or Hole Hull
*/
{
 long clc ;
/*
** Initialise
*/
 *directionP = 0 ;
/*
** Test If point1 -point2 Is A Void Or Hole Hull Line
*/
 clc = nodeAddrP(dtmP,point1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clc)->nextPnt == point2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ) { *directionP = 1 ; return(1) ; }
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole ) { *directionP = 1 ; return(1) ; }
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;  
   } 
/*
** Test If point2point1 Is A Void Or Hole Hull Line
*/
 clc = nodeAddrP(dtmP,point2)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,clc)->nextPnt == point1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ) { *directionP = -1 ; return(1) ; }
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole ) { *directionP = -1 ; return(1) ; }
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;  
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
BENTLEYDTM_Private int bcdtmPolyContour_testForIslandHullLineDtmObject(BC_DTM_OBJ *dtmP,long point1,long point2,long *directionP)
/*
** This Function Tests If The Line point1-point2 is On A Void Or Hole Hull
*/
{
 long clc ;
/*
** Initialise
*/
 *directionP = 0 ;
/*
** Test If point1 point2 Is An Island Hull Line
*/
 clc = nodeAddrP(dtmP,point1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,clc)->nextPnt == point2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) { *directionP = -1 ; return(1) ; }
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;  
   } 
/*
** Test If point2 point1 Is An Island Hull Line
*/
 clc = nodeAddrP(dtmP,+point2)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,clc)->nextPnt == point1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) { *directionP = 1 ; return(1) ; }
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;  
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
BENTLEYDTM_Private int bcdtmPolyContour_testForVoidHullPointDtmObject(BC_DTM_OBJ *dtmP,long Point)
/*
** This Function Tests If The Line P1-P2 is On A Tin Hull Line
*/
{
 long clc ;
/*
** Test For Point On A Void Or Hole Feature
*/
 clc = nodeAddrP(dtmP,Point)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ) return(1) ;
    if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole   ) return(1)  ;
    clc = flistAddrP(dtmP,clc)->nextPtr ;  
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
BENTLEYDTM_Private int bcdtmPolyContour_testForHullLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2)
/*
** This Function Tests If The Line P1-P2 is On A Tin Hull Line
*/
{
 long clc ;
/*
** Test If P1-P2 Is A Valid Tin Line
*/
 if(! bcdtmList_testLineDtmObject(dtmP,P1,P2)) return(0) ;
/*
** Test For Hull Line
*/
 if( nodeAddrP(dtmP,P1)->hPtr == P2 || nodeAddrP(dtmP,P2)->hPtr == P1 ) return(1)  ;
/*
** Test For P1 P2 Being On A DTM Feature
*/
 clc = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,clc)->nextPnt == P2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ) return(1)  ;
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole   ) return(1)  ;
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) return(1)  ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;  
   } 
/*
** Test For P2 P1 Being On A DTM Feature
*/
 clc = nodeAddrP(dtmP,P2)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( flistAddrP(dtmP,clc)->nextPnt == P1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ) return(1) ;
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole   ) return(1) ;
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) return(1) ;
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;  
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
BENTLEYDTM_Private int bcdtmPolyContour_buildLineIndexTable(BC_DTM_OBJ *dtmP,DTM_SMOOTH_CONTOUR_INDEX **contourIndexP)
/*
**
** This Function Builds A Tin Line Index Table For Contour Tracing Purposes 
**
** Arguements
**  
** Tin         ==>  Tin Object
** contourIndexP  <==   Index Table To Be Built
**
** Return Value = 0 Success , 1 Error
**
** Validation - Minimal
** Visibility - dtmPrivate
**
** Author     - Rob Cormack 
** Date       - 10 August 2001 
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    p1,p2,clc,Nl,Direction ;
 bool VoidLine;
 DTM_SMOOTH_CONTOUR_INDEX *pItab ;
/*
** Write Status Message - Development Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Line Index Table") ;
/*
** Validate Line Index Table
*/
 if( *contourIndexP != nullptr ) { bcdtmWrite_message(0,0,0,"Line Index Table Not Initialised To nullptr") ; goto errexit ; }
/*
** Allocate Memory For Table
*/
 *contourIndexP = ( DTM_SMOOTH_CONTOUR_INDEX * ) malloc( dtmP->numLines * sizeof(DTM_SMOOTH_CONTOUR_INDEX)) ;
 if( *contourIndexP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Initialise Table
*/
 for( pItab = *contourIndexP ; pItab < *contourIndexP + dtmP->numLines ; ++pItab )
   { 
    pItab->P1 = pItab->P2 =  dtmP->nullPnt ;
    pItab->NumZero = 0 ; 
    pItab->Idx = pItab->Next = nullptr ; 
    pItab->Zeros[0] = pItab->Zeros[1] = pItab->Zeros[2] = pItab->Zeros[3] = pItab->Zeros[4] = 0.0 ;
    pItab->Zmin = pItab->Zmax = 0.0 ;
   }
/*
**  Scan Tin Lines And Build Index Table
*/
 Nl = 0 ;
 pItab = *contourIndexP ;
 for( p1 = 0  ; p1 < dtmP->numPoints  ; ++p1 )
   {
    (*contourIndexP+p1)->Idx = *contourIndexP + Nl ;
    if( ( clc = nodeAddrP(dtmP,p1)->cPtr) != dtmP->nullPtr )
      {
       while ( clc != dtmP->nullPtr )
         {
          p2  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( p1 < p2 )
            {
             if( Nl >= dtmP->numLines ) { bcdtmWrite_message(1,0,0,"Error With Line Index Table") ; goto errexit ; } 
             if( dbg ) if( Nl % 1000 == 0.0 ) bcdtmWrite_message(0,0,0,"Lines Processed = %6ld of %6ld",Nl,dtmP->numLines) ;   
             pItab->P1 = p1 ;
             pItab->P2 = p2 ;
/*
**           Determine Line Type
*/
             if(  bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,VoidLine) ) goto errexit ;
             if( VoidLine ) pItab->Type = 0 ;
             else if( nodeAddrP(dtmP,p1)->hPtr == p2 ) pItab->Type = -2 ;
             else if( nodeAddrP(dtmP,p2)->hPtr == p1 ) pItab->Type =  2 ; 
             else if( bcdtmPolyContour_testForVoidHullLineDtmObject(dtmP,p1,p2,&Direction) )  pItab->Type = Direction * 3 ;
             else if( bcdtmPolyContour_testForIslandHullLineDtmObject(dtmP,p1,p2,&Direction)) pItab->Type = Direction * 4 ;
             else pItab->Type = 1 ;
/*
**           Increment Pointers
*/
             ++pItab ;
             ++Nl ;  
            }
         }
      }
   } 
 if( dbg ) bcdtmWrite_message(0,0,0,"Lines Processed = %6ld of %6ld",Nl,dtmP->numLines) ;  
/*
** Clean Up
*/
 cleanup : 
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Line Index Table Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Line Index Table Error") ;
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
BENTLEYDTM_Private int bcdtmPolyContour_getLineOffsetInLineIndexTable(DTM_SMOOTH_CONTOUR_INDEX *contourIndexP,long numLines,long P1,long P2,long *Offset)
/*
**
** This Function Gets The Offset For A Line In  The Tin Line Index Table 
**
** Arguements
**  
** contourIndexP  ==>   Index Table 
** P1         ==>   Point Number At One End Of Line
** P2         ==>   Point Number At Other End Of Line
** Offset     <==   Line Offset In Table
**
** Return Value = 0 Success , 1 Error
**
** Validation - Total
** Visibility - dtmPrivate
**
** Author     - Rob Cormack 
** Date       - 10 August 2001 
**
*/
{
 long      sp ;
 DTM_SMOOTH_CONTOUR_INDEX *bp,*tp ;
/*
** Initialise 
*/
 *Offset = -1 ;
 if( P1 > P2 ) { sp = P1 ; P1 = P2 ; P2 = sp ; }
 if( P1 < 0 || P1 >= numLines ) goto errexit ;
 if( P2 < 0 || P2 >= numLines ) goto errexit ;
/*
** Set Pointers To Scan Table
*/
 bp = (contourIndexP+P1)->Idx ;
 tp = contourIndexP + numLines ; 
/*
** Scan For Line Entry
*/
 while( bp < tp && *Offset == - 1 )
   {
    if( bp->P2 == P2 ) *Offset = (long)(bp-contourIndexP) ;
    ++bp ;
   } 
 if( *Offset == -1 ) goto errexit ;
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmPolyContour_buildZeroTinPointTableDtmObject(BC_DTM_OBJ *dtmP,double contourValue,double Eps,double *partDerivP,DTM_SMOOTH_CONTOUR_ZERO_POINT **zeroPtsP,long *numZeroPts)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   point,MemZeroPTab=0,MemZeroPTabInc=1000 ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For %10.4lf Tin Point Zeros",contourValue) ;
/*
** Initialise
*/
 *numZeroPts = 0 ; 
 if( *zeroPtsP != nullptr ) { free(*zeroPtsP) ; *zeroPtsP = nullptr ; }
/*
** Scan Tin Points
*/
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    if( pointAddrP(dtmP,point)->z == contourValue )
      {
       if(bcdtmPolyContour_getScanTrianglesAtZeroTinPointDtmObject(dtmP,point,Eps,partDerivP,zeroPtsP,numZeroPts,&MemZeroPTab,MemZeroPTabInc)) goto errexit ;
      } 
   }
/*
** Write Tin Point Zeros ** Development Purposes Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of %10.4lf Tin Point Zeros = %6ld",contourValue,*numZeroPts) ;
    for( point = 0 ; point < *numZeroPts ; ++point ) bcdtmWrite_message(0,0,0,"P0 = %6ld P1 = %6ld P2 = %6ld ** Angle = %12.10lf",(*zeroPtsP+point)->P0,(*zeroPtsP+point)->P1,(*zeroPtsP+point)->P2,(*zeroPtsP+point)->Angle) ;
   }
/*
**  Clean Up
*/
 cleanup :
/*
**  Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning For Tin Point Zeros Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Scanning For Tin Point Zeros Error") ;
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
BENTLEYDTM_Private int bcdtmPolyContour_getScanTrianglesAtZeroTinPointDtmObject(BC_DTM_OBJ *dtmP,long P0,double Eps,double *partDerivP,DTM_SMOOTH_CONTOUR_ZERO_POINT **zeroPtsP,long *numZeroPts,long *MemZeroPTab,long MemZeroPTabInc)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   clc, P1, P2;
 bool VoidTriangle;
 double F1,F2,ang,ang1,ang2,X1,Y1,X2,Y2,X11,Y11,X12,Y12,Zx,Zy,Zz ;
 double rad,radinc,radius ;
/*
** Write Status Message
*/
 Eps = Eps * 10.0 ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Getting Scan Triangles At Zero Tin Point %6ld",P0) ;
    bcdtmWrite_message(0,0,0,"P0 = %6ld ** %10.4lf %10.4lf %10.4lf",P0,pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,pointAddrP(dtmP,P0)->z) ;
   }
/*
** Scan About Point P0
*/
 if(( clc = nodeAddrP(dtmP,P0)->cPtr ) != dtmP->nullPtr )
   {
    if(( P2 = bcdtmList_nextAntDtmObject(dtmP,P0,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    ang2 = bcdtmMath_getPointAngleDtmObject(dtmP,P0,P2) ; 
    X2   = pointAddrP(dtmP,P0)->x + Eps * cos(ang2) ;
    Y2   = pointAddrP(dtmP,P0)->y + Eps * sin(ang2) ;
    while( clc != dtmP->nullPtr )
      {
       P1   = clistAddrP(dtmP,clc)->pntNum ;
       clc  = clistAddrP(dtmP,clc)->nextPtr ;
       ang1 = bcdtmMath_getPointAngleDtmObject(dtmP,P0,P1) ; 
       X1   = pointAddrP(dtmP,P0)->x + Eps * cos(ang1) ;
       Y1   = pointAddrP(dtmP,P0)->y + Eps * sin(ang1) ;
/*
**     Check For Valid Triangle
*/
       if( nodeAddrP(dtmP,P0)->hPtr != P2 )
         {
/*
**        Check For Non Void Triangle
*/
          if(  bcdtmList_testForVoidTriangleDtmObject(dtmP,P0,P2,P1,VoidTriangle)) goto errexit ;
          if( ! VoidTriangle )
            { 
/*
**           Calculate Polynomial Coefficients
*/
             CL = 0.0 ;
             if( bcdtmPolyContour_calculatePolynomialCoefficientsForPointTriangleDtmObject(dtmP,P0,P1,P2,partDerivP)) goto errexit ;
             CL = pointAddrP(dtmP,P0)->z ;
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %10.4lf",P1,pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z) ;
                bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %10.4lf",P2,pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z) ;
                F1 = bcdtmPolyContour_evaluateBivariateFunction(X1,Y1,0.0,0.0,0.0)  ; 
                F2 = bcdtmPolyContour_evaluateBivariateFunction(X2,Y2,0.0,0.0,0.0)  ;
                bcdtmWrite_message(0,0,0,"Eps X1Y1F1 = %10.4lf %10.4lf %12.8lf",X1,Y1,F1) ;
                bcdtmWrite_message(0,0,0,"Eps X2Y2F2 = %10.4lf %10.4lf %12.8lf",X2,Y2,F2) ;
               }
/*
**
**           Scan Across X1Y1-X2Y2 Looking For Turning Points
**           I Cannot Figure Out, How To do this Analytically
**
*/
             ang = bcdtmMath_getAngle(X1,Y1,X2,Y2) ;
             rad = bcdtmMath_distance(X1,Y1,X2,Y2) ;
             radinc = rad / 10.0 ;
             radius = radinc  ;
             X11 = X1 ;
             Y11 = Y1 ;
             F1 = bcdtmPolyContour_evaluateBivariateFunction(X11,Y11,0.0,0.0,0.0)  ; 
             if( dbg ) bcdtmWrite_message(0,0,0,"F1 = %12.8lf",F1) ;
             while ( radius <= rad )
               {
                X12 = X1 + radius * cos(ang) ;
                Y12 = Y1 + radius * sin(ang) ;
                F2 = bcdtmPolyContour_evaluateBivariateFunction(X12,Y12,0.0,0.0,0.0)  ;
                if( dbg ) bcdtmWrite_message(0,0,0,"F2 = %12.8lf",F2) ;
/*
**              Test For Zero
*/
/*
                if( F1*F2 < 0.0 ) 
                  {
                   if( bcdtmPolyContour_calculateBivariateFunctionZero(X11,Y11,cos(ang),sin(ang),0.0,radinc,F1,F2,&Zx,&Zy,&Zz) ) goto errexit ;
*/
                if( F1*F2 < 0.0 || F2 == 0.0  ) 
                  {
                   if( F2 == 0.0 ) { Zx = X12 ; Zy = Y12 ; Zz = 0.0 ; } 
                   else            if( bcdtmPolyContour_calculateBivariateFunctionZero(X11,Y11,cos(ang),sin(ang),0.0,radinc,F1,F2,&Zx,&Zy,&Zz) ) goto errexit ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"Zero Found = %10.4lf %10.4lf %12.8lf",Zx,Zy,Zz) ;
                   if( *numZeroPts == *MemZeroPTab )
                     {
                      *MemZeroPTab = *MemZeroPTab + MemZeroPTabInc ; 
                      if( *zeroPtsP == nullptr ) *zeroPtsP = ( DTM_SMOOTH_CONTOUR_ZERO_POINT *) malloc(*MemZeroPTab * sizeof(DTM_SMOOTH_CONTOUR_ZERO_POINT)) ;
                      else                      *zeroPtsP = ( DTM_SMOOTH_CONTOUR_ZERO_POINT *) realloc(zeroPtsP,*MemZeroPTab * sizeof(DTM_SMOOTH_CONTOUR_ZERO_POINT)) ;
                      if(  *zeroPtsP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
                     }
                   (*zeroPtsP+*numZeroPts)->P0 = P0 ;
                   (*zeroPtsP+*numZeroPts)->P1 = P1 ;
                   (*zeroPtsP+*numZeroPts)->P2 = P2 ;
                   (*zeroPtsP+*numZeroPts)->Angle = bcdtmMath_getAngle(pointAddrP(dtmP,P0)->x,pointAddrP(dtmP,P0)->y,Zx,Zy) ;
                   ++*numZeroPts ;
                  } 
                F1  = F2 ;
                X11 = X12 ;
                Y11 = Y12 ;
                radius = radius + radinc ; 
                if( fabs(rad-radius) < radinc / 2.0 ) radius = rad ;
               }
            } 
         }
/*
** Set For Next Triangle
*/ 
       P2   = P1 ; 
       ang2 = ang1 ;
       X2   = X1   ;
       Y2   = Y1   ; 
      }
   }
/*
** Clean Up
*/
 cleanup :
 CL = 0.0 ; 
/*
** Return
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Scan Triangles At Zero Tin Point Completed") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Scan Triangles At Zero Tin Point Error") ;
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
int bcdtmPolyContour_findEntryInZeroTinPointTable(DTM_SMOOTH_CONTOUR_ZERO_POINT *zeroPtsP,long numZeroPts,long P0,long P1,long P2,long *Np1,long *Np2,double *Angle)
{
 long       dbg=DTM_TRACE_VALUE(0),notfound ;
 DTM_SMOOTH_CONTOUR_ZERO_POINT *pt ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding %9ld %9ld %9ld Entry In Zero Point Table",P0,P1,P2) ;
/*
** Initialise
*/
 *Angle = 0.0 ;
 *Np1 = *Np2 = DTM_NULL_PNT ;
/*
** Check There Are Entries In Table
*/
 if( zeroPtsP != nullptr && numZeroPts > 0 ) 
   {
/*
** Write Zero Point Table Developement Only
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Zero Tin Point Table Entries = %6ld",numZeroPts) ;
       for( pt = zeroPtsP ; pt < zeroPtsP + numZeroPts ; ++pt ) bcdtmWrite_message(0,0,0,"Entry[%4ld] = %6ld %9ld %9ld ** %12.10lf",(long)(pt-zeroPtsP),pt->P0,pt->P1,pt->P2,pt->Angle) ;
      } 
/*
** Find First Entry For P0
*/
    pt = zeroPtsP ;
    while ( pt < zeroPtsP + numZeroPts && pt->P0 != P0 ) ++pt ;
/*
** Find Relevent Entry For P0
*/
    notfound = 1 ;
    while ( pt < zeroPtsP + numZeroPts && pt->P0 == P0 && notfound )
      {
       if( pt->P1 != DTM_NULL_PNT )
         {
          if( P1 ==  DTM_NULL_PNT || ( pt->P1 == P1 && pt->P2 == P2 ) || ( pt->P1 == P2 && pt->P2 == P1 ) )
            { 
             *Angle = pt->Angle ; 
             *Np1 = pt->P1 ;
             *Np2 = pt->P2 ;
             pt->P1 = pt->P2 = DTM_NULL_PNT ; 
             notfound = 0 ; 
             if( dbg ) bcdtmWrite_message(0,0,0,"Found Offset = %6ld",(long)(pt-zeroPtsP)) ;
            }
         }
       ++pt ;    
      }
    if( dbg )  bcdtmWrite_message(0,0,0,"NotFound = %1ld Np1 = %9ld Np2 = %9ld Angle = %12.10lf",notfound,*Np1,*Np2,*Angle) ;
   }
/*
**  Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding %6ld %9ld %9ld Entry In Zero Point Table Completed",P0,P1,P2) ;
 return(0) ;
}
/*--------------------------------------------------------------------------------+
|                                                                                 |
|                                                                                 |
|                                                                                 |
+--------------------------------------------------------------------------------*/
int bcdtmPolyContour_calculatePolynomialMinimaAndMaximaForTriangleSidesDtmObject(BC_DTM_OBJ *dtmP,double *partDerivP,double *polyZMin,double *polyZMax,DTM_SMOOTH_CONTOUR_INDEX *contourIndexP)
/*
**
** This Function Calculates The Polynomial Minima And Maxima z Values 
** For All Triangle Edges
**
** Arguements
**  
** dtmP          => Dtm Object
** partDerivP    => Partial Derivatives For Each Tin Point
** polyZMin      <= Minimum Polynomial z value
** polyZMax      <= Maximum Polynomial z value
** contourIndexP => Line Index Table For Updating Zmin And Zmax Of Line
**
** Return Value = 0 Success , 1 Error
**
** Validation - Nill
** Visibility - dtmPrivate
**
** Author     - Rob Cormack 
** Date       - 8 August 2001 
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_TRACE_VALUE(0) ;
 long   p0,p1,p2,j0,j1,j2,clc,Nt,In[3] ;
 long   Nzr,Offset,numLines ;
 double Pd[15],Tzr[6][3],Vzr[6][3],Zmax[3],Zmin[3] ; 
 double Xp,Yp,ProdZ,DeveZ,ZeroZ,LineZ ;
/*
** Write Status Message - Developement Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Polynomial Minima And Maxima") ;
/*
** Initialise
*/
 *polyZMin = dtmP->zMin ;
 *polyZMax = dtmP->zMax ;
 numLines = dtmP->numLines  ;
/*
**  Scan All Tin Triangles
*/
 Nt = 0 ;
 for( p0 = 0  ; p0 < dtmP->numPoints  ; ++p0 )
   {
    if( ( clc = nodeAddrP(dtmP,p0)->cPtr) != dtmP->nullPtr )
      {
       p1 = clistAddrP(dtmP,clc)->pntNum ;
       if( ( p1 = bcdtmList_nextAntDtmObject(dtmP,p0,p1)) < 0 ) goto errexit ;
       while ( clc != dtmP->nullPtr )
         {
          p2  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( p1 > p0 && p2 > p0  && nodeAddrP(dtmP,p0)->hPtr != p1 )
            {
             if( dbg ) if( Nt % 1000 == 0.0 ) bcdtmWrite_message(0,0,0,"Triangles Processed = %6ld of %6ld",Nt,dtmP->numTriangles) ;   
             ++Nt ;
/*
**          Set Coordinates Of Triangle Vertices For Polynomial Calculations
*/  
             X[0] = pointAddrP(dtmP,p0)->x ; Y[0] = pointAddrP(dtmP,p0)->y ; Z[0] = pointAddrP(dtmP,p0)->z ;
             X[1] = pointAddrP(dtmP,p1)->x ; Y[1] = pointAddrP(dtmP,p1)->y ; Z[1] = pointAddrP(dtmP,p1)->z ;
             X[2] = pointAddrP(dtmP,p2)->x ; Y[2] = pointAddrP(dtmP,p2)->y ; Z[2] = pointAddrP(dtmP,p2)->z ;
/*
**          Calculate Triangle Geometry For Polynomial Calculations
*/
             for( j0 = 0 ; j0 < 3 ; ++j0 ) 
               {
                if( j0 == 0 ) { j1 = 2 ; j2 = 1 ; }
                if( j0 == 1 ) { j1 = 2 ; j2 = 0 ; }
                if( j0 == 2 ) { j1 = 0 ; j2 = 1 ; } 
                DX[j0]  = X[j2] - X[j1] ;
                DY[j0]  = Y[j2] - Y[j1] ;
                DX2[j0] = DX[j0]*DX[j0] ;
                DY2[j0] = DY[j0]*DY[j0] ;
                SL2[j0] = DX2[j0] + DY2[j0] ;
                SL[j0]  = sqrt(SL2[j0]) ;
               }
/*
**           Calculate Polynomial Coefficients For Triangle
*/
             bcdtmLattice_getPartialDerivatives(p0,p1,p2,partDerivP,Pd) ;
             bcdtmPolyContour_calculatePolynomialCoefficientsForTriangle(Pd) ;
/*
**           Check Polynomial Functions - Developement Only
**           Check z value Of Triangle Centroid Against Production Polynomial Interpolation Function
*/
             if( cdbg )
               {
                Xp = ( X[0] + X[1] + X[2] ) / 3.0 ;               
                Yp = ( Y[0] + Y[1] + Y[2] ) / 3.0 ;               
                DeveZ = bcdtmPolyContour_evaluateBivariateFunction(Xp,Yp,0.0,0.0,0.0) ; 
                bcdtmPolyContour_interpolatePolynomialTriangle(1,Xp,Yp,&ProdZ,X,Y,Z,Pd) ;
/*
                if( dbg ) bcdtmWrite_message(0,0,0,"Polynomial Check P = %10.4lf %10.4lf ** DeveZ = %12.6lf ProdZ = %12.6lf Diff = %13.10lf",Xp,Yp,DeveZ,ProdZ,DeveZ-ProdZ) ;
*/
                if( fabs(DeveZ-ProdZ) > 0.00001 ) 
                  {
                   bcdtmWrite_message(1,0,0,"Error With Polynomial Coefficients") ; 
                   bcdtmWrite_message(0,0,0,"Deve Coordinates = %10.4lf %10.4lf %10.4lf",Xp,Yp,DeveZ) ;
                   bcdtmWrite_message(0,0,0,"Prod Coordinates = %10.4lf %10.4lf %10.4lf",Xp,Yp,ProdZ) ;
                   goto errexit ;
                  }
               } 
/*
**           Calculate Minima And Maxima For Each Triangle Side
*/
             for( j0 = 0 ; j0 < 3 ; ++j0 ) 
               {
                bcdtmPolyContour_calculateSideEndPointsAndMinimaMaxima(j0,Tzr,Vzr,In,Zmin,Zmax) ;
                if( Zmin[j0] < *polyZMin ) *polyZMin = Zmin[j0] ;
                if( Zmax[j0] > *polyZMax ) *polyZMax = Zmax[j0] ;
/*
**              Update Line Index Table With Zmin And Zmaxs For Line
*/            
                if( j0 == 0 ) { if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,p2,p1,&Offset)) goto errexit ; }
                if( j0 == 1 ) { if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,p2,p0,&Offset)) goto errexit ; }
                if( j0 == 2 ) { if( bcdtmPolyContour_getLineOffsetInLineIndexTable(contourIndexP,numLines,p0,p1,&Offset)) goto errexit ; }
                (contourIndexP+Offset)->Zmin = Zmin[j0] ;
                (contourIndexP+Offset)->Zmax = Zmax[j0] ;
/*
**              Write Out Zeros - Development Only
*/
/*
                if( dbg ) 
                  {
                   bcdtmWrite_message(0,0,0,"Side = %1ld Nzr = %1ld Zmin = %10.4lf Zmax = %10.4lf",j0,In[j0],Zmin[j0],Zmax[j0]) ;
                   for( Nzr = 0 ; Nzr < In[j0] ; ++Nzr )
                     {
                      bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinates(j0,Nzr,Vzr[Nzr][j0],Tzr,&Xp,&Yp,&ZeroZ) ;
                      bcdtmWrite_message(0,0,0,"Side = %1ld Zero = %1ld ** %10.4lf %10.4lf %10.4lf",j0,Nzr+1,Xp,Yp,ZeroZ) ;
                     }
                  }  
*/
/*
**             Check Calculations For Zero Points - Developement Only
*/
                if( cdbg )
                  {
                   for( Nzr = 0 ; Nzr < In[j0] ; ++Nzr )
                     {
                      bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinates(j0,Nzr,Vzr[Nzr][j0],Tzr,&Xp,&Yp,&ZeroZ) ;
                      LineZ = bcdtmPolyContour_evaluatePolynomialFunction(5,Tzr[Nzr][j0],j0) ;
                      DeveZ = bcdtmPolyContour_evaluateBivariateFunction(Xp,Yp,0.0,0.0,0.0) ; 
                      bcdtmPolyContour_interpolatePolynomialTriangle(1,Xp,Yp,&ProdZ,X,Y,Z,Pd) ;
                      if( fabs(ZeroZ-LineZ) > 0.00001 || fabs(ZeroZ-DeveZ) > 0.00001 || fabs(ZeroZ-ProdZ) > 0.00001) 
                        {
                         bcdtmWrite_message(1,0,0,"Error With Polynomial Zero Coordinates") ;
                         bcdtmWrite_message(0,0,0,"Zero  Coordinates = %10.4lf %10.4lf %10.4lf",Xp,Yp,ZeroZ) ;
                         bcdtmWrite_message(0,0,0,"Line  Coordinates = %10.4lf %10.4lf %10.4lf",Xp,Yp,LineZ) ;
                         bcdtmWrite_message(0,0,0,"Deve  Coordinates = %10.4lf %10.4lf %10.4lf",Xp,Yp,DeveZ) ;
                         bcdtmWrite_message(0,0,0,"Prod  Coordinates = %10.4lf %10.4lf %10.4lf",Xp,Yp,ProdZ) ;
                        } 
                     }
                  }
               }
            }
/*
**        Reset For Next Triangle
*/ 
          p1 = p2 ;
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
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Polynomial Minima And Maxima Completed") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Polynomial Minima And Maxima Error") ;
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
|  bcdtmPolyContour_calculateSideEndPointsAndMinimaMaxima                 |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_calculateSideEndPointsAndMinimaMaxima(long Side,double Tzr1[6][3],double Z1[6][3],long NumberNintervals[3],double Zmin[3],double Zmax[3])
/*
** This Function Calculates The Number Of Polynomial Intervals And 
** The  Polynomial Maxima And Minina For A Triangle Side
*/
{
 long   dbg=DTM_TRACE_VALUE(0),j,k,p1,p2,Ni,Nii ;
 double Ta,Tb,F1,F2,Tzr2[6]   ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Side %1ld End Points And Maxima Minima",Side) ;
/*
** Set End Points For Initial Polynomial Interval
*/
 Tzr1[0][Side] = 0.0 ;
 Tzr1[1][Side] = 1.0 ; 
 Ni= 1 ;
/*
** Scan Polynomial For Additional Intervals Starting At The 4th Derivative
*/
 for( k = 1 ; k <= 4 ; ++k )
   {
    Tzr2[0] = 0.0 ;
    Nii = 1 ;
    Tb = 0.0 ;
    F2 = bcdtmPolyContour_evaluatePolynomialFunction(k,Tb,Side) ;
    for( j = 1 ; j <= Ni ; ++j )
      {
       Ta = Tb ;
       F1 = F2 ;
       Tb = Tzr1[j][Side] ; 
       F2 = bcdtmPolyContour_evaluatePolynomialFunction(k,Tb,Side) ;
       if( F1 * F2 < 0.0  ) 
         {
          Tzr2[Nii] =  bcdtmPolyContour_calculatePolynomialFunctionZero(k,Side,Ta,Tb,F1,F2) ;
          Nii= Nii + 1 ;
         } 
      }
    if( 1.0-Tzr2[Nii-1] < 0.0000001 )  --Nii ;   
    Tzr2[Nii]= 1.0 ;
    for( j = 1 ; j <= Nii ; ++j )  Tzr1[j][Side]= Tzr2[j] ;
    Ni = Nii ;
   }
/*
**  Set Number Of Nintervals For Side 
*/
 NumberNintervals[Side]= Ni ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intervals Side %1ld = %2ld",Side,NumberNintervals[Side]) ;
/*
**  Set Maxima and Minima To Line End Points
*/
 if( Side == 0 ) { p1 = 2 ; p2 = 1 ; }
 if( Side == 1 ) { p1 = 2 ; p2 = 0 ; }
 if( Side == 2 ) { p1 = 0 ; p2 = 1 ; }
 if( Z[p1] <= Z[p2] ) { Zmin[Side] = Z[p1] ; Zmax[Side] = Z[p2] ; }
 else                 { Zmin[Side] = Z[p2] ; Zmax[Side] = Z[p1] ; }
 Z1[0][Side]    = Z[p1] ;
 Z1[Ni+1][Side] = Z[p2] ;
/*
** Get Maxima And Minima For Polynomial Zeros
*/
 if( Ni != 0) 
   { 
    for( j = 0 ; j <= Ni ; ++j  )
      {  
       Z1[j][Side] = bcdtmPolyContour_evaluatePolynomialFunction(5,Tzr1[j][Side],Side) ;
       if( Z1[j][Side] > Zmax[Side] ) Zmax[Side] = Z1[j][Side] ;
       if( Z1[j][Side] < Zmin[Side] ) Zmin[Side] = Z1[j][Side] ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Zero Point = %10.4lf  z = %10.4lf",Tzr1[j][Side],Z1[j][Side]) ;
      }
   }
 if( dbg) bcdtmWrite_message(0,0,0,"Side %2ld ** Minima = %10.4lf Maxima = %10.4lf",Side,Zmin[Side],Zmax[Side]) ;
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
int bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinates(long Side,long Zero,double contourValue,double Tzr[5][3],double *Zx,double *Zy,double *Zz)
/*
** This Function Calculates The XYZ Coordinates For A Zero
*/
{
/*
** Initialise
*/
 *Zx = *Zy = *Zz = 0.0 ;
/*
** Calculate Coordinates
*/
 *Zz = contourValue ;
 if( Side < 2 )
   {
    *Zx = DX[Side] * Tzr[Zero][Side] ; 
    *Zy = DY[Side] * Tzr[Zero][Side] ; 
   }
 else
   {
    *Zx = DX[1] + DX[Side] * Tzr[Zero][Side] ; 
    *Zy = DY[1] + DY[Side] * Tzr[Zero][Side] ; 
   } 
/*
** Add Offset
*/
 *Zx = *Zx + X[2] ;
 *Zy = *Zy + Y[2] ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmPolyContour_calculateContourZeroCoordinates                       |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_calculateContourZeroCoordinates(long Side,long Zero,double contourValue,double Tzr[5],double *Zx,double *Zy,double *Zz)
/*
** This Function Calculates The XYZ Coordinates For A Zero
*/
{
/*
** Initialise
*/
 *Zx = *Zy = *Zz = 0.0 ;
/*
** Calculate Coordinates
*/
 *Zz = contourValue ;
 if( Side < 2 )
   {
    *Zx = DX[Side] * Tzr[Zero] ; 
    *Zy = DY[Side] * Tzr[Zero] ; 
   }
 else
   {
    *Zx = DX[1] + DX[Side] * Tzr[Zero] ; 
    *Zy = DY[1] + DY[Side] * Tzr[Zero] ; 
   } 
/*
** Add Offset
*/
 *Zx = *Zx + X[2] ;
 *Zy = *Zy + Y[2] ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|    bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinatesAlongTinLineDtmObject       |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_calculatePolynomialFunctionZeroCoordinatesAlongTinLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,double contourValue,double Zero,double *Zx,double *Zy,double *Zz)
/*
** This Function Calculates The XYZ Coordinates For A Zero Along A Tin Line P1-P2
*/
{
 double dx,dy ;
/*
** Initialise
*/
 dx  = pointAddrP(dtmP,P2)->x - pointAddrP(dtmP,P1)->x ;
 dy  = pointAddrP(dtmP,P2)->y - pointAddrP(dtmP,P1)->y ;
/*
** Calculate Coordinates
*/
 *Zz = contourValue ;
 *Zx = pointAddrP(dtmP,P1)->x + dx * Zero ;
 *Zy = pointAddrP(dtmP,P1)->y + dy * Zero ;
/*
** Job Completed
*/
 return(0) ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmPolyContour_calculatePartialDerivativesDtmObject()                |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_calculatePartialDerivativesDtmObjectOld(BC_DTM_OBJ *dtmP,double **partDerivP )
/*
** This Routine Calculate the Partial Derivatives at
** the Vertices of the Triangles
*/
{
 int     ret=DTM_SUCCESS ;
 long    p,cl,cp,dtmFeature,nullOut ;
 double  x,y,z,zx,zy ;
 double  nmx,nmy,nmz,nmxx,nmxy,nmyx,nmyy ;
 double  dx1,dy1,dz1,dx2,dy2,dz2,dzx1,dzy1,dzx2,dzy2 ;
 double  dnmy,dnmx,dnmz ;
 double  dnmxx,dnmxy,dnmyx,dnmyy ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Allocate Storage for Partial Derivatives
*/
 *partDerivP = ( double * ) malloc( dtmP->numPoints * 5 * sizeof(double) ) ;
 if( *partDerivP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto cleanup ;
   }
/*
** Estimate zx and zy
*/
 for( p = 0 ; p < dtmP->numPoints ; ++ p )
   {
    x = pointAddrP(dtmP,p)->x ;
    y = pointAddrP(dtmP,p)->y ;
    z = pointAddrP(dtmP,p)->z ;
    nmx = nmy = nmz = 0.0 ;
    cl = nodeAddrP(dtmP,p)->cPtr ;
    while ( clistAddrP(dtmP,cl)->nextPtr != dtmP->nullPtr )
      {
       dx1 = pointAddrP(dtmP,clistAddrP(dtmP,cl)->pntNum)->x - x ;
       dy1 = pointAddrP(dtmP,clistAddrP(dtmP,cl)->pntNum)->y - y ;
       dz1 = pointAddrP(dtmP,clistAddrP(dtmP,cl)->pntNum)->z - z ;
       cp  = clistAddrP(dtmP,cl)->nextPtr    ;
       while ( cp != dtmP->nullPtr )
         {
          dx2 = pointAddrP(dtmP,clistAddrP(dtmP,cp)->pntNum)->x - x ;
          dy2 = pointAddrP(dtmP,clistAddrP(dtmP,cp)->pntNum)->y - y ;
          dnmz = dx1*dy2 - dy1*dx2 ;
          if( dnmz != 0.0 )
            {
             dz2 = pointAddrP(dtmP,clistAddrP(dtmP,cp)->pntNum)->z - z ;
             dnmx = dy1*dz2 - dz1*dy2 ;
             dnmy = dz1*dx2 - dx1*dz2 ;
             if( dnmz < 0.0 ) { dnmx = - dnmx ; dnmy = - dnmy ; dnmz = - dnmz ; }
             nmx = nmx + dnmx ; nmy = nmy + dnmy ; nmz = nmz + dnmz ;
            }
          cp = clistAddrP(dtmP,cp)->nextPtr ;
         }
       cl = clistAddrP(dtmP,cl)->nextPtr ;
      }
    *(*partDerivP + p*5 )     = -nmx/nmz ;
    *(*partDerivP + p*5 + 1 ) = -nmy/nmz ;
   }
/*
** Estimate ZXX ZXY ZYY
*/
 for( p = 0 ; p < dtmP->numPoints ; ++p )
   {
    x = pointAddrP(dtmP,p)->x ;
    y = pointAddrP(dtmP,p)->y ;
    zx = *(*partDerivP + p*5 ) ;
    zy = *(*partDerivP + p*5 + 1 ) ;
    nmxx = nmxy = nmyx = nmyy = nmz = 0.0 ;
    cl = nodeAddrP(dtmP,p)->cPtr ;
    while( clistAddrP(dtmP,cl)->nextPtr != dtmP->nullPtr )
      {
       dx1 = pointAddrP(dtmP,clistAddrP(dtmP,cl)->pntNum)->x - x ;
       dy1 = pointAddrP(dtmP,clistAddrP(dtmP,cl)->pntNum)->y - y ;
       dzx1 = *(*partDerivP+clistAddrP(dtmP,cl)->pntNum * 5 )     - zx ;
       dzy1 = *(*partDerivP+clistAddrP(dtmP,cl)->pntNum * 5 + 1 ) - zy ;
       cp  = clistAddrP(dtmP,cl)->nextPtr    ;
       while ( cp != dtmP->nullPtr )
         {
          dx2 = pointAddrP(dtmP,clistAddrP(dtmP,cp)->pntNum)->x - x ;
          dy2 = pointAddrP(dtmP,clistAddrP(dtmP,cp)->pntNum)->y - y ;
          dnmz = dx1*dy2 - dy1*dx2 ;
          if( dnmz != 0.0 )
            {
             dzx2 = *( *partDerivP +clistAddrP(dtmP,cp)->pntNum * 5 )     - zx ;
             dzy2 = *( *partDerivP +clistAddrP(dtmP,cp)->pntNum * 5 + 1 ) - zy ;
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
    *(*partDerivP + p * 5 + 2 ) = - ( nmxx / nmz ) ;
    *(*partDerivP + p * 5 + 3 ) = - ((nmxy + nmyx) / (2.0 * nmz)) ;
    *(*partDerivP + p * 5 + 4 ) = - ( nmyy / nmz ) ;
   }
/*
** Null Out Partial Derivatives For Points On Tin Hull
*/
 p = dtmP->hullPoint ;
 do
   {
    *(*partDerivP+p*5) = *(*partDerivP+p*5+1) = *(*partDerivP+p*5+2) = *(*partDerivP+p*5+3) = *(*partDerivP+p*5+4) = 0.0 ;
    p = nodeAddrP(dtmP,p)->hPtr ;
   } while ( p != dtmP->hullPoint ) ;  
/*
** Null Out Partial Derivatives For DTM Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       nullOut = FALSE ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void       || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole        || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) nullOut = TRUE ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline || dtmFeatureP->dtmFeatureType == DTMFeatureType::ContourLine ) nullOut = TRUE ;
       if( nullOut == TRUE )
         {
          p = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do
            {
             *(*partDerivP+p*5) = *(*partDerivP+p*5+1) = *(*partDerivP+p*5+2) = *(*partDerivP+p*5+3) = *(*partDerivP+p*5+4) = 0.0 ;
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,p,&p)) goto errexit ;
            } while ( p != dtmFeatureP->dtmFeaturePts.firstPoint && p != dtmP->nullPnt ) ;
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
/*----------------------------------------------------------------------+
|                                                                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
int bcdtmPolyContour_calculatePolynomialCoefficientsForPointTriangleDtmObject(BC_DTM_OBJ *dtmP,long P0,long P1,long P2,double *partDerivP)
{
 long   cdbg=DTM_TRACE_VALUE(0),j0,j1,j2 ;
 double Pd[15],Xp,Yp,DeveZ,ProdZ ;
/*
** Set Coordinates Of Triangle Vertices For Polynomial Calculations
*/  
 X[0] = pointAddrP(dtmP,P0)->x ; Y[0] = pointAddrP(dtmP,P0)->y ; Z[0] = pointAddrP(dtmP,P0)->z ;
 X[1] = pointAddrP(dtmP,P1)->x ; Y[1] = pointAddrP(dtmP,P1)->y ; Z[1] = pointAddrP(dtmP,P1)->z ;
 X[2] = pointAddrP(dtmP,P2)->x ; Y[2] = pointAddrP(dtmP,P2)->y ; Z[2] = pointAddrP(dtmP,P2)->z ;
/*
** Calculate Triangle Geometry For Polynomial Calculations
*/
 for( j0 = 0 ; j0 < 3 ; ++j0 ) 
   {
    if( j0 == 0 ) { j1 = 2 ; j2 = 1 ; }
    if( j0 == 1 ) { j1 = 2 ; j2 = 0 ; }
    if( j0 == 2 ) { j1 = 0 ; j2 = 1 ; } 
    DX[j0]  = X[j2] - X[j1] ;
    DY[j0]  = Y[j2] - Y[j1] ;
    DX2[j0] = DX[j0]*DX[j0] ;
    DY2[j0] = DY[j0]*DY[j0] ;
    SL2[j0] = DX2[j0] + DY2[j0] ;
    SL[j0]  = sqrt(SL2[j0]) ;
   }
/*
** Calculate Polynomial Coefficients For Triangle
*/
 bcdtmLattice_getPartialDerivatives(P0,P1,P2,partDerivP,Pd) ;
 bcdtmPolyContour_calculatePolynomialCoefficientsForTriangle(Pd) ;
/*
** Check Polynomial Functions - Developement Only
** Check z value Of Triangle Centroid Against 
** Production Polynomial Interpolation Function
*/
 if( cdbg )
   {
    Xp = ( X[0] + X[1] + X[2] ) / 3.0 ;               
    Yp = ( Y[0] + Y[1] + Y[2] ) / 3.0 ;               
    DeveZ = bcdtmPolyContour_evaluateBivariateFunction(Xp,Yp,0.0,0.0,0.0) ; 
    bcdtmPolyContour_interpolatePolynomialTriangle(1,Xp,Yp,&ProdZ,X,Y,Z,Pd) ;
    if( fabs(DeveZ-ProdZ) > 0.00001 ) 
      {
       bcdtmWrite_message(1,0,0,"Error With Polynomial Coefficients") ; 
       bcdtmWrite_message(0,0,0,"Deve Coordinates      = %10.4lf %10.4lf %10.4lf",Xp,Yp,DeveZ) ;
       bcdtmWrite_message(0,0,0,"Prod Coordinates      = %10.4lf %10.4lf %10.4lf",Xp,Yp,ProdZ) ;
       bcdtmWrite_message(0,0,0,"Polynomial Difference =  %13.10lf",ProdZ-DeveZ) ;
       goto errexit ;
      }
   } 
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmPolyContour_calculatePolynomialCoefficientsForTriangle            |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_calculatePolynomialCoefficientsForTriangle(double *PDV)
{
 long    dbg=DTM_TRACE_VALUE(0),i,j,k,p1,p2 ;
 double  ad,bc,dlt,ab,adbc,cd,dxdy1,dxdy2,dxdy3,h1,h2,h3,e1,g1,g2,g3 ;
 double  Pd[5][3] ;
/*
** Load The Partial Derivatives
*/
 k = 0 ; for( j = 0 ; j < 3 ; ++j ) for( i = 0 ; i < 5 ; ++i ) { Pd[i][j] = *(PDV+k) ; ++k ; }
/*
**   Determine The Coefficients For The XY Coordinate System 
**   Transformation To The UV System
*/ 
 ad  = DX[1]*DY[0] ;
 bc  = DX[0]*DY[1] ;
 dlt = ad-bc       ;
 AP  =  DY[0]/dlt  ;
 BP  = -DX[0]/dlt  ;
 CP  = -DY[1]/dlt  ;
 DP  =  DX[1]/dlt  ;
/*
**   Convert The Partial Derivative At The Vertices Of The Triangle
**   For The UV System
*/
 ab    = DX[1]*DX[0] ;
 adbc  = ad+bc ;
 cd    =  DY[0]*DY[1] ;
 dxdy1 = 2.0*DX[0]*DY[0] ;
 dxdy2 = 2.0*DX[1]*DY[1] ;
 dxdy3 = 2.0*DX[2]*DY[2] ;
 for( k = 0 ; k < 3 ; ++k )
   {
    ZT[0][k] =  DX[1]*Pd[0][k] + DY[1]*Pd[1][k] ;
    ZT[1][k] =  DX[0]*Pd[0][k] + DY[0]*Pd[1][k] ;
    ZTT[0][k]= DX2[1]*Pd[2][k] + dxdy2*Pd[3][k] + DY2[1]*Pd[4][k] ;
    ZUV[k]   =   ab  *Pd[2][k] +  adbc*Pd[3][k] +   cd  *Pd[4][k] ;
    ZTT[1][k]= DX2[0]*Pd[2][k] + dxdy1*Pd[3][k] + DY2[0]*Pd[4][k] ;
   } 
 for( k = 0 ; k < 2 ; ++k )
   {
    ZT[2][k] =  DX[2]*Pd[0][k] + DY[2]*Pd[1][k] ;
    ZTT[2][k]= DX2[2]*Pd[2][k] + dxdy3*Pd[3][k] + DY2[2]*Pd[4][k] ;
   }
/*
** Calculate The Coefficients Of The Polynomials Along
** The Three Sides Of The Triangle
*/
 for( k = 0 ; k < 3 ; ++k )  
   {
    if( k == 0 ) { p1 = 2 ; p2 = 1 ; i = 1 ; }
    if( k == 1 ) { p1 = 2 ; p2 = 0 ; i = 0 ; }
    if( k == 2 ) { p1 = 0 ; p2 = 1 ; i = 2 ; } 
    P0[k] = Z[p1] ;
    P1[k] = ZT[i][p1] ;
    P2[k] = 0.5*ZTT[i][p1] ;
    h1= Z[p2] - P0[k] - P1[k] - P2[k] ;
    h2= ZT[i][p2] - P1[k] - ZTT[i][p1] ;
    h3= ZTT[i][p2] - ZTT[i][p1] ;   
    P3[k]= 10.0*h1-4.0*h2+0.5*h3 ;
    P4[k]=-15.0*h1+7.0*h2    -h3 ;
    P5[k]=  6.0*h1-3.0*h2+0.5*h3 ;
/*
**  Calculate Coefficients For Derivatives Along Sides
*/
    Q0[k]=      P1[k] ;
    Q1[k]= 2.0 *P2[k] ;
    Q2[k]= 3.0 *P3[k] ;
    Q3[k]= 4.0 *P4[k] ;
    Q4[k]= 5.0 *P5[k] ;
    R0[k]=      Q1[k] ;
    R1[k]= 2.0 *Q2[k] ;
    R2[k]= 3.0 *Q3[k] ;
    R3[k]= 4.0 *Q4[k] ;
    S0[k]=      R1[k] ;
    S1[k]= 2.0 *R2[k] ;
    S2[k]= 3.0 *R3[k] ;
    T0[k]=      S1[k] ;
    T1[k]= 2.0 *S2[k] ;
   }
/*
** Calculate Polynomial Coefficients P11...P41 For
** Bivariate Polynomial Inside Rectangle
*/
 P14 = P5[0] * (2.5*(SL2[1]-SL2[2])/SL2[0] + 2.5) ;
 P41 = P5[1] * (2.5*(SL2[0]-SL2[2])/SL2[1] + 2.5) ;
 P11 = ZUV[2] ;
 h1  = ZT[1][0]-P1[0]-P11-P41 ;
 h2  = ZUV[0]-P11-4.0*P41 ;
 P21 =  3.0*h1-h2 ;
 P31 = -2.0*h1+h2 ;
 h1  = ZT[0][1]-P1[1]-P11-P14 ;
 h2  = ZUV[1]-P11-4.0*P14 ;
 P12 =  3.0*h1-h2 ;   
 P13 = -2.0*h1+h2 ;
 h1  = 0.5*ZTT[1][0]-P2[0]-P12 ;
 h2  = 0.5*ZTT[0][1]-P2[1]-P21 ;
 e1  = 2.5*(SL2[1]-SL2[0])/SL2[2] + 2.5 ;
 g1  =  3.0 - e1 ;
 g2  = -2.0 + e1 ;
 g3  = e1*(P5[0] - P5[1] + P41 -P14) + P14 - 4.*P41 + 5.*P5[1] ;
 P22 = g1*h1 + g2*h2 + g3 ;
 P32 = h1-P22 ;
 P23 = h2-P22 ;
/*
** Write Out Coefficients ** Developement Purposes Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Coefficients Of Polynomials Along The Sides") ;
    for( k = 0 ; k < 3 ; ++k ) bcdtmWrite_message(0,0,0,"Side %1ld ** %10.4lf %10.4lf %10.4lf %10.4lf %10.4lf %10.4lf",k,P0[k],P1[k],P2[k],P3[k],P4[k],P5[k]) ;
    bcdtmWrite_message(0,0,0,"Coefficients Of 1ST Derivative Along The Sides") ;
    for( k = 0 ; k < 3 ; ++k ) bcdtmWrite_message(0,0,0,"Side %1ld ** %10.4lf %10.4lf %10.4lf %10.4lf %10.4lf",k,Q0[k],Q1[k],Q2[k],Q3[k],Q4[k] ) ;
    bcdtmWrite_message(0,0,0,"Coefficients Of 2ND Derivative Along The Sides") ;
    for( k = 0 ; k < 3 ; ++k ) bcdtmWrite_message(0,0,0,"Side %1ld ** %10.4lf %10.4lf %10.4lf %10.4lf",k,R0[k],R1[k],R2[k],R3[k] ) ;
    bcdtmWrite_message(0,0,0,"Coefficients Of 3RD Derivative Along The Sides") ;
    for( k = 0 ; k < 3 ; ++k ) bcdtmWrite_message(0,0,0,"Side %1ld ** %10.4lf %10.4lf %10.4lf",k,S0[k],S1[k],S2[k] ) ;
    bcdtmWrite_message(0,0,0,"Coefficients Of 4TH Derivative Along The Sides") ;
    for( k = 0 ; k < 3 ; ++k )  bcdtmWrite_message(0,0,0,"Side %1ld ** %10.4lf %10.4lf",k,T0[k],T1[k]) ;
    bcdtmWrite_message(0,0,0,"Coefficients For Bivarate Polynomial Inside Triangle") ;
    bcdtmWrite_message(0,0,0,"AP = %10.4lf BP = %10.4lf",AP,BP) ; 
    bcdtmWrite_message(0,0,0,"CP = %10.4lf DP = %10.4lf",CP,DP) ; 
    bcdtmWrite_message(0,0,0,"P1X ** %10.4lf %10.4lf %10.4lf %10.4lf",P11,P12,P13,P14) ;
    bcdtmWrite_message(0,0,0,"P2X ** %10.4lf %10.4lf %10.4lf",P21,P22,P23) ;
    bcdtmWrite_message(0,0,0,"P3X ** %10.4lf %10.4lf",P31,P32) ;
    bcdtmWrite_message(0,0,0,"P4X ** %10.4lf",P41) ;
   } 
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmPolyContour_interpolatePolynomialTriangle()                                     |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_interpolatePolynomialTriangle(long NewTrg,double xp,double yp,double *zp,double x[],double y[],double z[],double pd[] )
/*
**  This routine interpolates the z value for a point
**  within a triangle from a bivarate polynominal
*/
{
 long   dbg=DTM_TRACE_VALUE(0),i,jpd ;
 double a,b,c,d,dlt,dx,dy,u,v ;
 double act2,adbc,bdt2,lu,lv,thxu,thuv ;
 double aa,bb,cc,dd,ab,cd,ac,ad,bc,g1,g2,h1,h2,h3  ;
 double zu[3],zuu[3],zv[3],zvv[3],zuv[3] ;
 double p0,p1,p2,p3,p4,p5 ;
 double csuv,thus,thsv ;
 static double x0,y0,ap,bp,cp,dp ;
 static double p00,p01,p02,p03,p04,p05 ;
 static double p10,p11,p12,p13,p14 ;
 static double p20,p21,p22,p23 ;
 static double p30,p31,p32 ;
 static double p40,p41 ;
 static double p50 ;
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
     h1 = -aa*aa*aa*(5.0*aa*bb*p50+(4.0*bc+ad)*p41) -  cc*cc*cc*(5.0*cc*dd*p05+(4.0*ad+bc)*p14) ;
     h2 = 0.5 * zvv[1] - p02 - p12 ;
     h3 = 0.5 * zuu[2] - p20 - p21 ;
     p22 = (g1*h2 + g2*h3 - h1) / ( g1 + g2 ) ;
     p32 = h2 - p22 ;
     p23 = h3 - p22 ;
/*
** Write Out Coefficients
*/
     if( dbg )
       {
        bcdtmWrite_message(0,0,0,"Coefficients For Bivarate Polynomial Inside Triangle") ;
        bcdtmWrite_message(0,0,0,"x0 = %10.4lf y0 = %10.4lf",x0,y0) ; 
        bcdtmWrite_message(0,0,0,"ap = %10.4lf bp = %10.4lf",ap,bp) ; 
        bcdtmWrite_message(0,0,0,"cp = %10.4lf dp = %10.4lf",cp,dp) ; 
        bcdtmWrite_message(0,0,0,"P0X ** %10.4lf %10.4lf %10.4lf %10.4lf %10.4lf %10.4lf",p00,p01,p02,p03,p04,p05) ;
        bcdtmWrite_message(0,0,0,"P1X ** %10.4lf %10.4lf %10.4lf %10.4lf %10.4lf",p10,p11,p12,p13,p14) ;
        bcdtmWrite_message(0,0,0,"P2X ** %10.4lf %10.4lf %10.4lf %10.4lf",p20,p21,p22,p23) ;
        bcdtmWrite_message(0,0,0,"P3X ** %10.4lf %10.4lf %10.4lf",p30,p31,p32) ;
        bcdtmWrite_message(0,0,0,"P4X ** %10.4lf %10.4lf",p40,p41) ;
        bcdtmWrite_message(0,0,0,"P5X ** %10.4lf",p50) ;
       }
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
|  bcdtmPolyContour_evaluatePolynomialFunction                                      |
|                                                                    |
+-------------------------------------------------------------------*/
double bcdtmPolyContour_evaluatePolynomialFunction(long EvalFunction,double T,long Side )
/*
**  This Function Evalautes The Various Polynomial Functions For A Traingle
**
**  Arguements
**
**  EvalFunction ==>    Number Of Function To Be Evaluated
**  EvalFunction = 1    4th Derivative Along Side
**  EvalFunction = 2    3rd Derivative Along Side
**  EvalFunction = 3    2nd Derivative Along Side
**  EvalFunction = 4    1st Derivative Along Side
**  EvalFunction = 5    Origonal Polynomial Along Side
**  T            ==>    Value To Be Used For 1 to 5 
**  Side         ==>    Side Number <0,1,2> For Eval Functions 1 to 5
**
**  Return Value  <==   Function Value 
**  Visibility  - dtmPrivate
**  Author      - Rob Cormack
**  Date        - 8th August 2001 
**  
*/
{
/*
** Calculate Requested Function
*/
 switch ( EvalFunction )
   { 
    case  1 :
      return(T0[Side] + T*T1[Side]) ;
    break     ;

    case  2 :
      return(S0[Side] + T*(S1[Side]+T*S2[Side])) ;
    break    ; 

    case  3 :
      return(R0[Side] + T*(R1[Side]+T*(R2[Side]+T*R3[Side]))) ;
    break    ;

    case  4 :
      return(Q0[Side]+T*(Q1[Side]+T*(Q2[Side]+T*(Q3[Side]+T*Q4[Side])))) ;
    break    ;

    case  5 :
      return(P0[Side]+T*(P1[Side]+T*(P2[Side]+T*(P3[Side]+T*(P4[Side]+T*P5[Side])))) - CL ) ;
    break    ;

    default :
      return(0.0) ;
    break   ; 
   } ;
/*
** Job Completed
*/
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
double bcdtmPolyContour_evaluateBivariateFunction(double Xp,double Yp,double T,double Cos,double Sin)
/*
**  This Function Evalautes The Biviate Polynomial Function Inside A Triangle
**
**  Arguements
**
**  x            ==>    x Coordinate 
**  y            ==>    y Coordinate 
**  T            ==>    Radius From Point x y
**  Cos          ==>    Cosine Of Angle From Point XY
**  Sin          ==>    Sine Of Angle From Point XY
**
**  Return Value  <==   Function Value 
**  Visibility  - dtmPrivate
**  Author      - Rob Cormack
**  Date        - 8th August 2001 
**  
*/
{
 double x,y,u,v,h0,h1,h2,h3,h4,h5 ;
/*
** Transform Point Coordinates To UV System
*/
 x = Xp - X[2] + T * Cos ;
 y = Yp - Y[2] + T * Sin ;
 u = AP*x + BP*y  ;
 v = CP*x + DP*y  ;
/*
** Evaluate Function
*/
 h0=P0[0]+v*(P1[0]+v*(P2[0]+v*(P3[0]+v*(P4[0]+v*P5[0])))) ;
 h1=P1[1]+v*(P11+v*(P12+v*(P13+v*P14))) ;
 h2=P2[1]+v*(P21+v*(P22+v*P23)) ;
 h3=P3[1]+v*(P31+v*P32) ;
 h4=P4[1]+v*P41 ;
 h5=P5[1] ; 
/*
** Return Interpolated z Value
*/
 return(h0+u*(h1+u*(h2+u*(h3+u*(h4+u*h5))))-CL) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   bcdtmPolyContour_calculatePolynomialFunctionZero                                        |
|                                                                    |
+-------------------------------------------------------------------*/
double bcdtmPolyContour_calculatePolynomialFunctionZero(long EvalFunction,long Side,double TA,double TB,double F1,double F2)
/*
** This Function Calculates A Zero Between TA and Tb For A Polynomial Triangle Function
**      
** Arguements
**
**  EvalFunction ==> Function To Be Used For Calculating Zero
**  Side         ==> Side Of Triangle To Be Used For Calculating Zero
**  TA           ==> Start Point Value To Be Used For Function
**  TB           ==> End   Point Value To Be Used For Function
**  F1           ==> Function Value At TA
**  F2           ==> Function Value At TB
** 
** Return Value  -  Function Value At Zero
**
** Parameter Validation - NIL
** Visibility           - dtmPrivate
**
** Notes
** 1. F1 and F2 must have opposite sign 
** 2. ER is set to the accuracy of locating a Zero  
**
** Author      - Rob Cormack
** Date        - 8th August 2001
*/
{
 double A,B,E,FA,FB,FC,C,S,FS,H,FY,G,y,FG,ER=0.00000001 ;
/*
** Initialise
*/
 A  = TA ;
 B  = TB ;
 FA = F1 ;
 FB = F2 ;
 C  = A  ;
 FC = FA ;
 S  = C  ;
 FS = FC ;
/*
** Loop 
*/
 L10 : 
 H = 0.5*(B+C) ;
 if(fabs(H-B) <= ER) goto L110 ;
 if(fabs(FB)  <= fabs(FC)) goto L15 ;
 y=B ;
 FY=FB ;
 G=B ;
 FG=FB ;
 S=C ;
 FS=FC ;
 goto L20 ;
 L15 :
 y=S ;
 FY=FS ;
 G=C ;
 FG=FC ;
 S=B ;
 FS=FB ;
 L20 :
 if (FY != FS) goto L21 ;
 B=H ;
 goto L29 ;
 L21 :
 E=(S*FY-y*FS)/(FY-FS) ;
 if( fabs(E-S) <= ER )
   {
    if( G-S > 0.0 ) E = S + fabs(ER) ;
    else            E = S - fabs(ER) ;
   } 
 if ((E-H)*(S-E) <  0.0) goto L28 ;
 B=E ;
 goto L29 ;
 L28 :
 B=H ;
 L29 :  
 FB = bcdtmPolyContour_evaluatePolynomialFunction(EvalFunction,B,Side) ;
 if (FG*FB < 0.0) goto L35 ;
 C=S ;
 FC=FS ;
 goto L10 ;
 L35 :
 C=G ;
 FC=FG ;
 goto L10 ;
 L110 :return(H) ; 
}
/*-------------------------------------------------------------------+
|                                                                    |
|   bcdtmPolyContour_calculateBivariateFunctionZero                       |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_calculateBivariateFunctionZero(double Xp,double Yp,double Cos,double Sin,double Ta,double Tb,double F1,double F2,double *Zx,double *Zy,double *Zz)
/*
** This Function Finds A Contour Zero For The Bivariate Polynomial Inside A Triangle
**      
** Arguements
**
**  Xp           ==> x Coordinate Of Scan Start
**  Yp           ==> y Coordiante Of Scan Start
**  Cos          ==> Cos Of Scan Angle
**  Sin          ==> Sin Of Scan Angle
**  TA           ==> Distance Along Line For Scan Start
**  TB           ==> Disatnce Along Line For Scan End
**  F1           ==> Function Value At Scan Start ;
**  F2           ==> Function Value At Scan End   ;
** 
** Return Value  -  Function Value At Zero
**
** Parameter Validation - NIL
** Visibility           - dtmPrivate
**
** Notes
**
** 1. F1 and F2 must have opposite sign 
** 2. ER is set to the accuracy of locating a Zero  
**
** Author      - Rob Cormack
** Date        - 17th August 2001
*/
{
 long   dbg=DTM_TRACE_VALUE(0),cdbg=DTM_TRACE_VALUE(0) ;
 double L,H,R,Fl,Fh,Fr,Tl ;

 double X1,Y1,X2,Y2 ;
 X1 = Xp + Ta * Cos ;
 Y1 = Yp + Ta * Sin ;
 X2 = Xp + Tb * Cos ;
 Y2 = Yp + Tb * Sin ;
 return(bcdtmPolyContour_calculateBivariateFunctionZeroBetweenPoints(X1,Y1,X2,Y2,Zx,Zy,Zz)) ;
/*
** Write Parameters ** Development Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Xp  = %14.8lf  Yp  = %14.8lf",Xp,Yp) ;
    bcdtmWrite_message(0,0,0,"Ta  = %14.8lf  Tb  = %14.8lf",Ta,Tb) ; 
    bcdtmWrite_message(0,0,0,"F1  = %14.8lf  F2  = %14.8lf",F1,F2) ; 
    bcdtmWrite_message(0,0,0,"Cos = %14.10lf Sin = %14.10lf",Cos,Sin) ;
   }
/*
** Check Parameter Values ** Development Only
*/
 if( cdbg )
   {
    Fl = bcdtmPolyContour_evaluateBivariateFunction(Xp,Yp,Ta,Cos,Sin) ; 
    Fh = bcdtmPolyContour_evaluateBivariateFunction(Xp,Yp,Tb,Cos,Sin) ; 
    if( F1*F2 >= 0.0 || fabs(Fl-F1) > 0.000000001 || fabs(Fh-F2) > 0.000000001 )
      {
       bcdtmWrite_message(1,0,0,"Error With Bivariate Function Zeros") ; 
       bcdtmWrite_message(0,0,0,"F1 = %15.12lf F2 = %15.12lf",F1,F2) ;
       bcdtmWrite_message(0,0,0,"Fl = %15.12lf Fh = %15.12lf",Fl,Fh) ;
       goto errexit ;
      }
   }  
/*
** Initialise
*/
 *Zx = *Zy = *Zz = 0.0 ;
/*
** Check For Correct Values Of F1 and F2 
*/
 if( F1*F2 >= 0.0 ) { bcdtmWrite_message(1,0,0,"Ilegal Function Values") ; goto errexit ; }
/*
** Binary Scan To Find Zero
*/
 Tl = sqrt((Tb-Ta)*(Tb-Ta)) ;
 L  = Ta/Tl ; Fl = F1 ;
 H  = Tb/Tl ; Fh = F2 ;
 while ( fabs(H-L) > 0.0000000001 )
   {
    R = (L+H) / 2.0 ;
    Fr = bcdtmPolyContour_evaluateBivariateFunction(Xp,Yp,Tl*R,Cos,Sin) ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"R = %10.4lf L = %10.4lf H = %10.4lf Fr = %10.4lf",R,L,H,Fr) ;
    if     ( fabs(Fr) < 0.0000001) L = H ;
    else if( Fr*Fl >= 0.0 ) L = R ;
    else if( Fr*Fh >= 0.0 ) H = R ; 
   }
/*
** Write Parameters
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"L = %8.6lf R = %8.6lf H = %8.6lf Tl = %8.6lf Fr = %8.6lf",L,R,H,Tl,Fr) ;
   }
/*
** Calculate Zero Coordinates
*/
 *Zx = Xp+Tl*R*Cos ;
 *Zy = Yp+Tl*R*Sin ;
 *Zz = Fr ;
/*
** Write Zero Coordinates  ** Development Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Zero Coordinates = %10.4lf %10.4lf %10.4lf ** %10.4lf",*Zx,*Zy,*Zz,*Zz+CL) ;
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit : 
 return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_calculateBivariateFunctionZeroBetweenPoints(double X1,double Y1,double X2,double Y2,double *Zx,double *Zy,double *Zz)
/*
** This Function Finds A Contour Zero Between Point X1,Y1 and X2,Y2
**      
** Arguements
**
**  X1           ==> x Coordinate Of Point 1
**  Y1           ==> y Coordiante Of Point 1
**  X2           ==> x Coordinate Of Point 2
**  Y2           ==> y Coordiante Of Point 2
**  Zx          <==  x Coordinate Of Zero Point
**  Zy          <==  y Coordinate Of Zero Point
**  Zz          <==  z Coordinate Of Zero Point ( == 0.0 )
** 
** Return Value  -  Function Value At Zero
**
** Parameter Validation - NIL
** Visibility           - dtmPrivate
**
** Author      - Rob Cormack
** Date        - 7th October 2001
*/
{
 long   dbg=DTM_TRACE_VALUE(0) ;
 double F1,F2,Fm,Xm,Ym,Lxm,Lym ;
/*
** Initialise
*/
 *Zx = *Zy = *Zz = 0.0 ;
/*
** Evaluate Function Values At X1Y1 And X2Y2
*/
 F1 = bcdtmPolyContour_evaluateBivariateFunction(X1,Y1,0.0,0.0,0.0) ; 
 F2 = bcdtmPolyContour_evaluateBivariateFunction(X2,Y2,0.0,0.0,0.0) ; 
/*
** Write Parameters ** Development Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"X1  = %14.8lf  Y1  = %14.8lf",X1,Y1) ;
    bcdtmWrite_message(0,0,0,"X2  = %14.8lf  Y2  = %14.8lf",X2,Y2) ; 
    bcdtmWrite_message(0,0,0,"F1  = %14.10lf  F2  = %14.10lf",F1,F2) ; 
   }
/*
** Check For Correct Values Of F1 and F2 
*/
 if( F1*F2 > 0.0 || ( F1 == 0.0 && F2 == 0.0 ))
   {
    bcdtmWrite_message(1,0,0,"Error With Bivariate Function Zeros") ; 
    bcdtmWrite_message(0,0,0,"F1 = %15.12lf F2 = %15.12lf",F1,F2) ;
    goto errexit ;
   }
/*
** Check For Zero At X1Y1 or X2Y2
*/
 if     ( F1 == 0.0 ) { *Zx = X1 ; *Zy = Y1 ; }
 else if( F2 == 0.0 ) { *Zx = X2 ; *Zy = Y2 ; }
/*
** Binary Scan To Find Zero
*/
 else
   {
    Lxm = X1 ;
    Lym = Y1 ;
    Xm = (X1+X2) / 2.0 ;
    Ym = (Y1+Y2) / 2.0 ; 
    Fm = bcdtmPolyContour_evaluateBivariateFunction(Xm,Ym,0.0,0.0,0.0) ;
    while ( Fm != 0.0 && Xm != Lxm && Ym != Lym )
      {
       if     ( Fm < 0.0 && F1 < 0.0 ) { F1 = Fm ; X1 = Xm ; Y1 = Ym ; }
       else if( Fm < 0.0 && F2 < 0.0 ) { F2 = Fm ; X2 = Xm ; Y2 = Ym ; }
       else if( Fm > 0.0 && F1 > 0.0 ) { F1 = Fm ; X1 = Xm ; Y1 = Ym ; }
       else if( Fm > 0.0 && F2 > 0.0 ) { F2 = Fm ; X2 = Xm ; Y2 = Ym ; }
       Lxm = Xm ;
       Lym = Ym ;
       Xm = (X1+X2) / 2.0 ;
       Ym = (Y1+Y2) / 2.0 ; 
       Fm = bcdtmPolyContour_evaluateBivariateFunction(Xm,Ym,0.0,0.0,0.0) ;
      } 
    *Zx = Xm ; *Zy = Ym ; *Zz = Fm ; 
   } 
/*
** Write Zero Coordinates  ** Development Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Zero Coordinates = %10.4lf %10.4lf %14.10lf ** %10.4lf",*Zx,*Zy,*Zz,*Zz+CL) ;
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit : 
 return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_storeContourPointInCache(double x, double y, double z)
{
 long ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Point ** x = %12.5lf y = %12.5lf z = %10.4ld",x,y,z) ;
/*
** Check Cache Memory
*/
 if( NumContourCachePts == MemContourCachePts )
   {
    MemContourCachePts = MemContourCachePts + MemContourCachePtsInc ;
    if( ContourPtsCacheP == nullptr ) ContourPtsCacheP = ( DPoint3d * ) malloc( MemContourCachePts * sizeof(DPoint3d)) ;
    else                           ContourPtsCacheP = ( DPoint3d * ) realloc( ContourPtsCacheP, MemContourCachePts * sizeof(DPoint3d)) ;
    if( ContourPtsCacheP == nullptr )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
   }
/*
** Store Contour Point
*/
 (ContourPtsCacheP+NumContourCachePts)->x = x ;
 (ContourPtsCacheP+NumContourCachePts)->y = y ;
 (ContourPtsCacheP+NumContourCachePts)->z = z ;
 ++NumContourCachePts ;
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
int bcdtmPolyContour_writeContourPoints(FILE **fpDATA,DPoint3d *ContourPts,long NumConPts )
{
/*
 long fc ;
 char Buffer[28] ;
 DPoint3d  *p3d ;
*/
/*
**  Write To File
*/
/*
 if( *fpDATA == nullptr )
   {
    *fpDATA = bcdtmFile_open("contour.dat",L"wb") ;
    if( *fpDATA == nullptr ) { bcdtmWrite_message(1,0,0,"Error Opening Contour Data file") ; goto errexit ; }
   } 
 fc = 5 ;
 for( p3d = ContourPts ; p3d < ContourPts + NumConPts ; ++p3d )
   { 
    memcpy(&Buffer[ 0],&fc,4) ;
    memcpy(&Buffer[ 4],&p3d->x,8) ;
    memcpy(&Buffer[12],&p3d->y,8) ;
    memcpy(&Buffer[20],&p3d->z,8) ;
    if( fwrite(Buffer,28,1,*fpDATA) != 1 ) 
      { 
       bcdtmWrite_message(1,0,0,"Error Writing Binary Data File") ;
       goto errexit ; 
      }
    fc = 6 ;
   } 
  fflush(*fpDATA) ;
*/
/*
** Write Feature
*/
// if( dtmUser_writeDtmFeature(DTMFeatureType::Contour,nullPnt,ContourPts,NumConPts)) goto errexit ;
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit 
*/
// errexit :
// return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_xyzFilterContourPoints(DPoint3d *Points,long *NumPts,double XYTolerance,double ZTolerance)
/*
** This Routine Filters The Contour Line Strings
*/
{
 int     err=0,s2 ;
 long    dbg=DTM_TRACE_VALUE(0),memamt ;
 unsigned char    *FilterFlag=nullptr ; 
 double  xydelta,zdelta,d,dx,dy,a1,a2,a3,r,angle,zx,zy,zz ;
 DPoint3d     *p3d1,*p3d2,*p3d3,*p3dt ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"z Filtering Contour Points ** Number Of Points = %6ld",*NumPts) ;
/*
** Validate
*/
 if( Points == nullptr ) { bcdtmWrite_message(1,0,0,"Points Array nullptr") ; goto errexit ; }
 if( *NumPts <= 0   )  goto cleanup ;
/*
** Allocate Memory For Filter Flag
*/
 memamt = *NumPts / 8  + 1 ;
 FilterFlag = (unsigned char *) malloc( memamt * sizeof(char)) ;
 if( FilterFlag == nullptr ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
 memset(FilterFlag,0,memamt) ;
/*
** Filter Points
*/
 TotalContourPoints = TotalContourPoints + *NumPts ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Points") ;
 xydelta = fabs(XYTolerance) ;
 zdelta  = fabs(ZTolerance) ;
 p3d1  = Points  ;
 p3d2  = Points + 2 ;
 p3dt  = Points + *NumPts - 1 ;
 while ( p3d2 <= p3dt )
   {
    dx = p3d2->x - p3d1->x ;
    dy = p3d2->y - p3d1->y ;
    r = sqrt( dx*dx + dy*dy)    ;
    if( r > 0.0 )
      {
       a1 = dy / r ; a2 = -dx / r ; a3 = -a1 * p3d1->x - a2 * p3d1->y ;
       for( p3d3 = p3d1 + 1 ; p3d3 < p3d2 ; ++p3d3 )
         { 
          d = fabs( a1 * p3d3->x + a2 * p3d3->y + a3 )  ;
          s2 = bcdtmMath_sideOf(p3d1->x,p3d1->y,p3d2->x,p3d2->y,p3d3->x,p3d3->y) ;
          angle = atan2(dy,dx) ;
          if( s2 > 0 ) angle += DTM_PYE / 2.0 ;
          if( s2 < 0 ) angle -= DTM_PYE / 2.0 ;
          zx = p3d3->x + d * cos(angle) ;
          zy = p3d3->y + d * sin(angle) ;
          zz = bcdtmPolyContour_evaluateBivariateFunction(zx,zy,0.0,0.0,0.0) ; 
          if(  fabs(zz) > zdelta ||   d > xydelta )
            {
             for( p3d3 = p3d1 + 1 ; p3d3 < p3d2 - 1 ; ++p3d3 )
               {
                bcdtmFlag_setFlag(FilterFlag,(long)(p3d3-Points)) ;
               } 
             p3d1 = p3d2 - 1 ;
             p3d3 = p3d2     ;
            }
         }  
      }   
    else bcdtmFlag_setFlag(FilterFlag,(long)(p3d2-Points)) ;
    ++p3d2 ;
   }
/*
** Remove Filtered Vertices
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Filtered Points") ;
 for( p3d1 = p3d2 = Points ; p3d2 < Points + *NumPts ; ++p3d2 )
   {  
    if( ! bcdtmFlag_testFlag(FilterFlag,(long)(p3d2-Points)) )
      {
       if( p3d1 != p3d2 ) *p3d1 = *p3d2 ;
       ++p3d1 ;
      }   
   } 
 *NumPts = (long)(p3d1-Points) ;
 TotalFilteredPoints = TotalFilteredPoints + *NumPts ;
/*
** Free Memory
*/
 cleanup :
 if( FilterFlag != nullptr ) free(FilterFlag) ;
/*
** Job Completed
*/
 if( dbg && ! err ) bcdtmWrite_message(0,0,0,"Filtering Contour Points Completed") ;
 if( dbg &&   err ) bcdtmWrite_message(0,0,0,"Filtering Contour Points Error") ;
 return(err) ;
/*
** Error Exit
*/
 errexit :
 err = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmPolyContour_xyFilterContourPoints(DPoint3d *contourPtsP,long *numContourPtsP,double xyTolerance)
/*
** This Routine Filters The Contour Line Strings
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    memamt ;
 unsigned char    *filterFlagP=nullptr ; 
 double  xydelta,d,dx,dy,a1,a2,a3,r;
 DPoint3d     *p3d1P,*p3d2P,*p3d3P,*p3dtP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Contour Points") ;
/*
** Validate
*/
 if( contourPtsP == nullptr || *numContourPtsP <= 0 ) 
   { 
    bcdtmWrite_message(2,0,0,"No Contour Points To Filter") ;
    goto errexit ; 
   }
/*
** Only Filter For More Than Two Contour Points
*/   
 if( *numContourPtsP > 2   )
   {
/*    
**  Allocate Memory For Filter Flag
*/
    memamt = *numContourPtsP / 8  + 1 ;
    filterFlagP = (unsigned char *) malloc( memamt * sizeof(char)) ;
    if( filterFlagP == nullptr ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    memset(filterFlagP,0,memamt) ;
/*
** Filter Contour Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Contour Points") ;
    xydelta = fabs(xyTolerance) ;
    p3d1P    = contourPtsP  ;
    p3d2P    = contourPtsP + 2 ;
    p3dtP    = contourPtsP + *numContourPtsP - 1 ;
    while ( p3d2P <= p3dtP )
      {
       dx = p3d2P->x - p3d1P->x ;
       dy = p3d2P->y - p3d1P->y ;
       r = sqrt( dx*dx + dy*dy)    ;
       if( r > 0.0 )
         {
          a1 = dy / r ; a2 = -dx / r ; a3 = -a1 * p3d1P->x - a2 * p3d1P->y ;
          for( p3d3P = p3d1P + 1 ; p3d3P < p3d2P ; ++p3d3P )
            { 
             d =  a1 * p3d3P->x + a2 * p3d3P->y + a3   ;
             if( fabs(d) > xydelta )
               {
                for( p3d3P = p3d1P + 1 ; p3d3P < p3d2P - 1 ; ++p3d3P ) bcdtmFlag_setFlag(filterFlagP,(long)(p3d3P-contourPtsP)) ;
                p3d1P = p3d2P - 1 ;
                p3d3P = p3d2P     ;
               }
            }  
         }   
       else bcdtmFlag_setFlag(filterFlagP,(long)(p3d2P-contourPtsP)) ;
       ++p3d2P ;
      }
/*
**  Remove Filtered Vertices
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Filtered contourPtsP") ;
    for( p3d1P = p3d2P = contourPtsP ; p3d2P < contourPtsP + *numContourPtsP ; ++p3d2P )
      {  
       if( ! bcdtmFlag_testFlag(filterFlagP,(long)(p3d2P-contourPtsP)) )
         {
          if( p3d1P != p3d2P ) *p3d1P = *p3d2P ;
          ++p3d1P ;
         }   
      } 
    *numContourPtsP = (long)(p3d1P-contourPtsP) ;
   }    
/*
** Free Memory
*/
 cleanup :
 if( filterFlagP != nullptr ) free(filterFlagP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering Contour Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering Contour Points Error") ;
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
int bcdtmPolyContour_calculatePartialDerivativesDtmObject(BC_DTM_OBJ *dtmP,double **partDerivP )
/*
** This Routine Calculate the Partial Derivatives at
** the Vertices of the Triangles
*/
{
 long    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    p,*np,*pp,Pts[100],NumPts,dtmFeature,nullOut ;
 double  x,y,z,zx,zy ;
 double  nmx,nmy,nmz,nmxx,nmxy,nmyx,nmyy ;
 double  dx1,dy1,dz1,dx2,dy2,dz2,dzx1,dzy1,dzx2,dzy2 ;
 double  dnmy,dnmx,dnmz ;
 double  dnmxx,dnmxy,dnmyx,dnmyy ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Partial Derivatives For Tin Points") ;
/*
** Allocate Storage for Partial Derivatives
*/
 *partDerivP = ( double * ) malloc( dtmP->numPoints * 5 * sizeof(double) ) ;
 if( *partDerivP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto cleanup ; }
/*
** Scan Points In DTMFeatureState::Tin To Extimate zx and zy
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Estimating ZX ZY") ;
 for( p = 0 ; p < dtmP->numPoints ; ++ p )
   {
/*
**  Dont Process Deleted Points
*/
    if( nodeAddrP(dtmP,p)->cPtr != dtmP->nullPtr )
      { 
/*
**      Order Points About P In Increasing Distance From P
*/
        if( bcdtmPolyContour_orderPointsAboutPointDtmObject(dtmP,p,Pts,&NumPts)) goto errexit ;
/*
**      Estimate zx and zy
*/
        x = pointAddrP(dtmP,p)->x ;
        y = pointAddrP(dtmP,p)->y ;
        z = pointAddrP(dtmP,p)->z ;
        nmx = nmy = nmz = 0.0 ;
        for ( pp = Pts ; pp < Pts + NumPts - 1 ; ++pp )
          {
           dx1 = pointAddrP(dtmP,*pp)->x - x ;
           dy1 = pointAddrP(dtmP,*pp)->y - y ;
           dz1 = pointAddrP(dtmP,*pp)->z - z ;
           np  = pp + 1 ;
           for( np = pp + 1 ; np < Pts + NumPts ; ++np )
             {
              dx2 = pointAddrP(dtmP,*np)->x - x ;
              dy2 = pointAddrP(dtmP,*np)->y - y ;
              dnmz = dx1*dy2 - dy1*dx2 ;
              if( dnmz != 0.0 )
                {
                 dz2 = pointAddrP(dtmP,*np)->z - z ;
                 dnmx = dy1*dz2 - dz1*dy2 ;
                 dnmy = dz1*dx2 - dx1*dz2 ;
                 if( dnmz < 0.0 ) 
                   { 
                    dnmx = - dnmx ; 
                    dnmy = - dnmy ; 
                    dnmz = - dnmz ; 
                   }
                 nmx = nmx + dnmx ; 
                 nmy = nmy + dnmy ; 
                 nmz = nmz + dnmz ;
                }
             }
         }  
       *(*partDerivP + p*5 )     = -nmx/nmz ;
       *(*partDerivP + p*5 + 1 ) = -nmy/nmz ;
      }
   }
/*
** Scan Tin Points To Estimate ZXX ZXY ZYY
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Estimating ZXX ZXY ZYY") ;
 for( p = 0 ; p < dtmP->numPoints ; ++p )
   {
/*
**  Dont Process Deleted Points
*/
    if( nodeAddrP(dtmP,p)->cPtr != dtmP->nullPtr )
      { 
/*
**  Order Points About P In Increasing Distance From P
*/
       if( bcdtmPolyContour_orderPointsAboutPointDtmObject(dtmP,p,Pts,&NumPts)) goto errexit ;
/*
**  Estimate ZXX,ZXY,ZYY
*/
       x  = pointAddrP(dtmP,p)->x ;
       y  = pointAddrP(dtmP,p)->y ;
       zx = *(*partDerivP + p*5 ) ;
       zy = *(*partDerivP + p*5 + 1 ) ;
       nmxx = nmxy = nmyx = nmyy = nmz = 0.0 ;
       for ( pp = Pts ; pp < Pts + NumPts - 1 ; ++pp )
         {
          dx1  = pointAddrP(dtmP,*pp)->x - x ;
          dy1  = pointAddrP(dtmP,*pp)->y - y ;
          dzx1 = *(*partDerivP+ *pp * 5 )     - zx ;
          dzy1 = *(*partDerivP+ *pp * 5 + 1 ) - zy ;
          for( np = pp + 1 ; np < Pts + NumPts ; ++np )
            {
             dx2 = pointAddrP(dtmP,*np)->x - x ;
             dy2 = pointAddrP(dtmP,*np)->y - y ;
             dnmz = dx1*dy2 - dy1*dx2 ;
             if( dnmz != 0.0 )
               {
                dzx2  = *( *partDerivP + *np * 5 )     - zx ;
                dzy2  = *( *partDerivP + *np * 5 + 1 ) - zy ;
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
            }
         }
       *(*partDerivP + p * 5 + 2 ) = - ( nmxx / nmz ) ;
       *(*partDerivP + p * 5 + 3 ) = - ((nmxy + nmyx) / (2.0 * nmz)) ;
       *(*partDerivP + p * 5 + 4 ) = - ( nmyy / nmz ) ;
      }
   }
 return(0) ;
/*
** Null Out Partial Derivatives For Points On Tin Hull
*/
 p = dtmP->hullPoint ;
 do
   {
    *(*partDerivP+p*5) = *(*partDerivP+p*5+1) = *(*partDerivP+p*5+2) = *(*partDerivP+p*5+3) = *(*partDerivP+p*5+4) = 0.0 ;
    p = nodeAddrP(dtmP,p)->hPtr ;
   } while ( p != dtmP->hullPoint ) ;  
/*
** Null Out Partial Derivatives For DTM Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       nullOut = FALSE ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void       || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole        || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) nullOut = TRUE ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline || dtmFeatureP->dtmFeatureType == DTMFeatureType::ContourLine ) nullOut = TRUE ;
       if( nullOut == TRUE )
         {
          p = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do
            {
             *(*partDerivP+p*5) = *(*partDerivP+p*5+1) = *(*partDerivP+p*5+2) = *(*partDerivP+p*5+3) = *(*partDerivP+p*5+4) = 0.0 ;
             bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,p,&p) ;
            } while ( p != dtmFeatureP->dtmFeaturePts.firstPoint && p != dtmP->nullPnt ) ;
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
int bcdtmPolyContour_orderPointsAboutPointDtmObject(BC_DTM_OBJ *dtmP,long P,long Pts[],long *NumPts)
{
 long   cl,p1,p2,isw ;
 double Distance[100],td ;
/*
** Initilaise
*/
 *NumPts = 0 ;
/*
** Get Points About P
*/
 if(( cl = nodeAddrP(dtmP,P)->cPtr ) == dtmP->nullPtr ) { bcdtmWrite_message(1,0,0,"Deleted Point") ; goto errexit ; }
 while ( cl != dtmP->nullPtr )
   {
    Pts[*NumPts] = clistAddrP(dtmP,cl)->pntNum ;
    ++*NumPts ;
    cl = clistAddrP(dtmP,cl)->nextPtr ;
   }
/*
** Calculate Distance From P
*/
 for( p1 = 0 ; p1 < *NumPts ; ++p1 )  Distance[p1] = bcdtmMath_pointDistanceDtmObject(dtmP,P,Pts[p1]) ;
/*
** Write Out UnSorted Points
*/
/*
 if( P == 500 )
   {
    bcdtmWrite_message(0,0,0,"Points Un Sorted On Distance") ;
    for( p1 = 0 ; p1 < *NumPts ; ++p1 ) bcdtmWrite_message(0,0,0,"Point = %6ld Distance = %10.4lf",Pts[p1],Distance[p1]) ;
   } 
*/
/*
** Buble Sort Points On Icreasing Distance
*/
 isw = 1 ;
 while ( isw )
   { 
    isw = 0 ;
    for( p1 = 0 ; p1 < *NumPts - 1 ; ++p1 )
      {
       p2 = p1 + 1 ;
       if( Distance[p1] > Distance[p2] )
         {
          isw = 1 ;
          td = Distance[p1] ;
          Distance[p1] = Distance[p2] ; 
          Distance[p2] = td ;
          cl = Pts[p1] ;
          Pts[p1] = Pts[p2] ;
          Pts[p2] = cl ;
         }
      }
   } 
/*
** Write Out Sorted Points
*/
/*
 if( P == 500 )
   {
    bcdtmWrite_message(0,0,0,"Points Sorted On Distance") ;
    for( p1 = 0 ; p1 < *NumPts ; ++p1 ) bcdtmWrite_message(0,0,0,"Point = %6ld Distance = %10.4lf",Pts[p1],Distance[p1]) ;
   } 
*/
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 return(DTM_ERROR) ;
}
 
