/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmSlope.cpp $
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
BENTLEYDTM_EXPORT int  bcdtmSlope_calculateSlopeAreaDtmFile
(
 WCharCP dtmFileP,
 DPoint3d *slopePtsP,
 long numSlopePts,
 DTMFeatureCallback callBackFunctionP,          
 double *flatAreaP,
 double *slopeAreaP,
 DTM_POINT_ARRAY ***slopePolygonsPPP,
 long *numSlopePolygonsP 
)
/*
** This Function Calculates The Slope Area From A Dtm File
*/
{
 int ret=DTM_SUCCESS ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Initialise
*/
 *flatAreaP = *slopeAreaP = 0.0 ;
/*
** Read Dtm File
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) 
   { 
    if( dtmP != NULL )bcdtmObject_destroyDtmObject(&dtmP) ;
    goto errexit ;
   }
/*
** Calculate Slope Area
*/
 if( bcdtmSlope_calculateSlopeAreaDtmObject(dtmP,slopePtsP,numSlopePts,callBackFunctionP,flatAreaP,slopeAreaP,slopePolygonsPPP,numSlopePolygonsP)) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmSlope_calculateSlopeAreaDtmObject
(
 BC_DTM_OBJ *dtm1P,                   /* ==> Pointer To Tin Object                                */
 DPoint3d *slopePtsP,                      /* ==> Pointer To Polygon Designating Extent Of Calculation */
 long numSlopePts,                    /* ==> Number Of Polygon Points                             */
 DTMFeatureCallback callBackFunctionP,          /* ==> Call back Function Pointer For Polygon Areas         */
 double *flatAreaP ,                  /* <== Flat Area Of Calculation                             */ 
 double *slopeAreaP,                  /* <== Slope Area Of Calculation                            */
 DTM_POINT_ARRAY ***slopePolygonsPPP, /* <== Pointer To Pointer Array Of Slope Polygons           */  
 long             *numSlopePolygonsP  /* <== Pointer To Number Of Slope Polygons                  */ 
 )
/*
**  This Function Calculates Tin Slope Areas  
*/
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long      p1,p2,p3,clc,voidFlag,numTempSlopePts,numHullPts,intersectFlag ;
 DTMDirection direction;
 long      numSlopeArray=0,memSlopeArray=0,memSlopeArrayInc=0 ;
 double    x1,y1,z1,x2,y2,z2,x3,y3,z3,tx,ty,tz ;
 double    x11,y11,z11,x21,y21,z21,x31,y31,z31 ;
 double    c,d,ca,cb,cc,cd,xa,ya,area ;
 DPoint3d       *p3d1P,*p3d2P,*tempSlopePtsP=NULL,*hullPtsP=NULL ;
 BC_DTM_OBJ       *dtm2P=NULL ;
 DTM_POLYGON_OBJ  *polyObjP=NULL ;
 DTM_POLYGON_LIST *pListP ;
/*
** Write Entry Message
*/
 if( dbg == 1 ) 
   {
    bcdtmWrite_message(0,0,0,"Calculating Tin Slope Area") ;
    bcdtmWrite_message(0,0,0,"dtm1P        = %p",dtm1P) ;
    bcdtmWrite_message(0,0,0,"slopePtsP    = %p",slopePtsP) ;
    bcdtmWrite_message(0,0,0,"numSlopePts  = %8ld",numSlopePts) ;
    bcdtmWrite_message(0,0,0,"flatAreaP    = %8.2lf",*flatAreaP) ;
    bcdtmWrite_message(0,0,0,"slopeAreaP   = %8.2lf",*slopeAreaP) ;
   }
/*
** Initialise
*/
 *flatAreaP  = 0.0 ;
 *slopeAreaP = 0.0 ;
 *numSlopePolygonsP = 0 ;
 if( *slopePolygonsPPP != NULL ) { free(*slopePolygonsPPP) ; *slopePolygonsPPP = NULL ; }
/*
** Test For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit  ;
/*
** Check DTM Is Triangulated
*/
 if( dtm1P->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check Slope Area Polygon
*/
 if( numSlopePts <  0 ) numSlopePts = 0 ;
 if( numSlopePts != 0 && slopePtsP == NULL ) numSlopePts = 0 ;
/*
** Validate Slope Area Polgon
*/
 numTempSlopePts = numSlopePts ;
 if( numTempSlopePts > 0 )
   {
/*
**  Make A Local Copy Of The Slope Points
*/
    tempSlopePtsP = ( DPoint3d * ) malloc( numTempSlopePts * sizeof(DPoint3d)) ;
    if( tempSlopePtsP == NULL ) 
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit  ;
      }
    for( p3d1P = tempSlopePtsP , p3d2P = slopePtsP ; p3d1P < tempSlopePtsP + numTempSlopePts ; ++p3d1P , ++p3d2P ) *p3d1P = *p3d2P ;
/*
**  Validate Slope Polygon
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Slope area Polygon")  ;
    if( bcdtmMath_validatePointArrayPolygon(&tempSlopePtsP,&numTempSlopePts,dtm1P->ppTol) ) goto errexit ;
/*
** Calculate Polygon direction And area
*/
    if( cdbg )
      {
       bcdtmMath_getPolygonDirectionP3D(tempSlopePtsP,numTempSlopePts,&direction,&area) ;
       bcdtmWrite_message(0,0,0,"Area of Slope Area Polygon = %10.4lf",area) ;
      } 
   }
/*
**  Extract Tin Hull
*/
 if( bcdtmList_extractHullDtmObject(dtm1P,&hullPtsP,&numHullPts)) goto errexit  ;
/*
** Store Intersection Polygons Of Slope Area Polygon And Tin Hull In A Polygon Object
*/
 if( numTempSlopePts > 0 )
   {
    if( bcdtmPolygon_intersectPointArrayPolygons(hullPtsP,numHullPts,tempSlopePtsP,numTempSlopePts,&intersectFlag,&polyObjP,dtm1P->ppTol,dtm1P->plTol)) goto errexit  ;
    if( intersectFlag == 0 ) { bcdtmWrite_message(1,0,0,"Slope area Polygon And Tin Hull Do Not Intersect") ; goto errexit  ; }
    free( tempSlopePtsP ) ;
    tempSlopePtsP = NULL  ;
   }
/*
** Store Tin Hull Polygon In A Polygon Object
*/
 else 
   {
    if( bcdtmPolygon_createPolygonObject(&polyObjP)) goto errexit  ; 
    if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(polyObjP,hullPtsP,numHullPts,1)) goto errexit  ;
    intersectFlag = 2 ;
   }
/*
** Allocate Memory For Pointers To Slope Area Polygons
*/
 if( polyObjP->numPolygons > 0 )
   {
    memSlopeArrayInc = polyObjP->numPolygons ;
    if( bcdtmMem_allocatePointerArrayToPointArrayMemory(slopePolygonsPPP,numSlopeArray,&memSlopeArray,memSlopeArrayInc)) goto errexit ;
   } 
/*
** Scan Intersect Polygons And Accumulate Areas
*/
  for( pListP = polyObjP->polyListP ; pListP < polyObjP->polyListP + polyObjP->numPolygons ; ++pListP )
    {
     if( dbg ) bcdtmWrite_message(0,0,0,"Processing Slope Area Polygon %3ld of %3ld",(long)(pListP-polyObjP->polyListP)+1,polyObjP->numPolygons) ;
/*
**   Clip Tin Object
*/ 
     if( intersectFlag == 2 || intersectFlag == 4 ) 
       {
        dtm2P = dtm1P ;
        bcdtmList_extractHullDtmObject(dtm2P,&tempSlopePtsP,&numTempSlopePts) ;
/*
**      Store Slope Area Polygon
*/
        if( numSlopeArray == memSlopeArray ) 
          {
           if( bcdtmMem_allocatePointerArrayToPointArrayMemory(slopePolygonsPPP,numSlopeArray,&memSlopeArray,memSlopeArrayInc)) goto errexit ;
          }
        if( bcdtmMem_storePointsInExistingPointerArrayToPointArray(*slopePolygonsPPP,numSlopeArray,tempSlopePtsP,numTempSlopePts)) goto errexit ;
        ++numSlopeArray ;
/*
**      Calculate Polygon direction And area
*/
        if( cdbg )
          { 
           bcdtmMath_getPolygonDirectionP3D(tempSlopePtsP,numTempSlopePts,&direction,&area) ;
           bcdtmWrite_message(0,0,0,"Tin Hull Polygon area = %10.4lf",area) ;
          } 
       } 
     else 
       {
        if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyObjP,(long)(pListP-polyObjP->polyListP),&tempSlopePtsP,&numTempSlopePts)) goto errexit ;
/*
**      Load Slope Area Polygon
*/
        if( callBackFunctionP != NULL )
          {
           for( p3d1P = tempSlopePtsP ; p3d1P < tempSlopePtsP + numTempSlopePts ; ++p3d1P )
             {
              if( bcdtmLoad_storePointInCache(p3d1P->x,p3d1P->y,p3d1P->z) ) goto errexit ;
             }
           if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(callBackFunctionP,DTMFeatureType::Polygon,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,NULL) ) goto errexit ; 
          }
/*
**      Store Slope Area Polygon
*/
        if( numSlopeArray == memSlopeArray ) 
          {
           if( bcdtmMem_allocatePointerArrayToPointArrayMemory(slopePolygonsPPP,numSlopeArray,&memSlopeArray,memSlopeArrayInc)) goto errexit ;
          }
        if( bcdtmMem_storePointsInExistingPointerArrayToPointArray(*slopePolygonsPPP,numSlopeArray,tempSlopePtsP,numTempSlopePts)) goto errexit ;
        ++numSlopeArray ;
/*
**      Write Area Of Slope Area Polygon
*/
        if( cdbg )
          { 
           bcdtmMath_getPolygonDirectionP3D(tempSlopePtsP,numTempSlopePts,&direction,&area) ;
           bcdtmWrite_message(0,0,0,"Slope area Intersected Polygon area = %10.4lf",area) ;
          } 
/*
**      Clip Tin Object To Polygon
*/
        if (bcdtmClip_cloneAndClipToPolygonDtmObject (dtm1P, &dtm2P, tempSlopePtsP, numTempSlopePts, DTMClipOption::External)) goto errexit;
       }
/*
**  Scan All Points And Calculate Slope Areas
*/
    for( p1 = 0 ; p1 < dtm2P->numPoints ; ++p1 )
      {
       if( ( clc = nodeAddrP(dtm2P,p1)->cPtr ) != dtm2P->nullPtr )
         {
          if( ( p2 = bcdtmList_nextAntDtmObject(dtm2P,p1,clistAddrP(dtm2P,clc)->pntNum)) < 0 ) goto errexit  ;
          while ( clc != dtm2P->nullPtr )
            {
             p3  = clistAddrP(dtm2P,clc)->pntNum ;
             clc = clistAddrP(dtm2P,clc)->nextPtr ;
             if( p2 > p1 && p3 > p1 && nodeAddrP(dtm2P,p1)->hPtr != p2 )
               {
                bcdtmList_testForVoidTriangleDtmObject(dtm2P,p1,p2,p3,&voidFlag) ;
                if( ! voidFlag )          
                  {
                   x1 = pointAddrP(dtm2P,p1)->x ; y1 = pointAddrP(dtm2P,p1)->y ; z1 = pointAddrP(dtm2P,p1)->z ;
                   x2 = pointAddrP(dtm2P,p2)->x ; y2 = pointAddrP(dtm2P,p2)->y ; z2 = pointAddrP(dtm2P,p2)->z ;
                   x3 = pointAddrP(dtm2P,p3)->x ; y3 = pointAddrP(dtm2P,p3)->y ; z3 = pointAddrP(dtm2P,p3)->z ;
/*
**                 Get Translation Of Triangle Centre To Origon
*/
                   tx = 0.0 - ( x1 + x2 + x3 ) / 3.0 ;
                   ty = 0.0 - ( y1 + y2 + y3 ) / 3.0 ;
                   tz = 0.0 - ( z1 + z2 + z3 ) / 3.0 ;
/*
**                 Translate Triangle Centre To Origon
*/
                   x1 = x1 + tx ; x2 = x2 + tx ; x3 = x3 + tx ;
                   y1 = y1 + ty ; y2 = y2 + ty ; y3 = y3 + ty ;
                   z1 = z1 + tz ; z2 = z2 + tz ; z3 = z3 + tz ;
/*
**                 Calculate Flat area
*/
                   *flatAreaP = *flatAreaP + bcdtmMath_coordinateTriangleArea(x1,y1,x2,y2,x3,y3) ;
/*
**                 Calculate Plane Coefficients
*/  
                   if( fabs(z1-z2) > 0.000001 || fabs(z1-z3) > 0.000001 )
                     {
                      bcdtmMath_calculatePlaneCoefficients(x1,y1,z1,x2,y2,z2,x3,y3,z3,&ca,&cb,&cc,&cd) ;
                      if( cd == 0.0 ) cd = 0.0000000001 ;
                      ca = ca / cd ; cb = cb / cd ; cc = cc / cd ;
                      d  = 1.0 / sqrt( ca*ca + cb*cb + cc*cc ) ;
                      c  = cc * d ;
/*
**                    Calculate Triangle Slope
*/
                      ya = -acos(fabs(c))   ;
/*
**                    Calculate Angle Of Normal Projected On XY Plane
*/
                      xa = atan2(cb,ca) ;
                      if( xa <  0.0 ) xa = xa + DTM_2PYE ;
                      if( c  >= 0.0 ) xa = xa + DTM_PYE  ;
/*
**                    Rotate About z Axis
*/
                      x11 = ( cos(xa) * x1 ) + ( sin(xa) * y1 )  ;
                      y11 = ( cos(xa) * y1 ) - ( sin(xa) * x1 )  ;
                      z11 = z1 ;
 
                      x21 = ( cos(xa) * x2 ) + ( sin(xa) * y2 )  ;
                      y21 = ( cos(xa) * y2 ) - ( sin(xa) * x2 )  ;
                      z21 = z2 ;
 
                      x31 = ( cos(xa) * x3 ) + ( sin(xa) * y3 )  ;
                      y31 = ( cos(xa) * y3 ) - ( sin(xa) * x3 )  ;
                      z31 = z3 ;
 
                      x1 = x11 ; y1 = y11 ; z1 = z11 ;
                      x2 = x21 ; y2 = y21 ; z2 = z21 ;
                      x3 = x31 ; y3 = y31 ; z3 = z31 ;
/*
**                    Rotate About y Axis
*/
                      x11 = ( cos(ya) * x1 ) - ( sin(ya) * z1 ) ;
                      y11 = y1 ;
                      z11 = ( sin(ya) * x1 ) + ( cos(ya) * z1 ) ;
            
                      x21 = ( cos(ya) * x2 ) - ( sin(ya) * z2 ) ;
                      y21 = y2 ;
                      z21 = ( sin(ya) * x2 ) + ( cos(ya) * z2 ) ;
            
                      x31 = ( cos(ya) * x3 ) - ( sin(ya) * z3 ) ;
                      y31 = y3 ;
                      z31 = ( sin(ya) * x3 ) + ( cos(ya) * z3 ) ;

                      x1 = x11 ; y1 = y11 ; z1 = z11 ;
                      x2 = x21 ; y2 = y21 ; z2 = z21 ;
                      x3 = x31 ; y3 = y31 ; z3 = z31 ;
                     }
/*
**                 Calculate  Slope area
*/
                   *slopeAreaP = *slopeAreaP + bcdtmMath_coordinateTriangleArea(x1,y1,x2,y2,x3,y3) ;
                  } 
               }
             p2 = p3 ; 
            }
         } 
      }
/*
** Free Memory
*/
    if( dtm2P != NULL && dtm2P != dtm1P ) bcdtmObject_destroyDtmObject(&dtm2P) ;
    if( tempSlopePtsP != NULL ) { free(tempSlopePtsP) ; tempSlopePtsP = NULL ; }
   }
/*
**  Report Areas
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"flatArea = %15.4lf slopeArea = %15.4lf",*flatAreaP,*slopeAreaP) ;
/*
** Clean Up
*/
 cleanup :
 if( dbg ) bcdtmWrite_message(10,0,0,"") ;
 if( hullPtsP != NULL ) free(hullPtsP) ;
 if( polyObjP != NULL ) bcdtmPolygon_deletePolygonObject(&polyObjP) ;
 if( tempSlopePtsP != NULL ) free(tempSlopePtsP) ;
 if( dtm2P != NULL && dtm2P != dtm1P ) bcdtmObject_destroyDtmObject(&dtm2P) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Tin Slope Area Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Tin Slope Area Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ; 
}
