/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmPolygon.cpp $
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
BENTLEYDTM_Public int bcdtmPolygon_storeTptrPolygonInPolygonObjectDtmObject(BC_DTM_OBJ *dtmP,DTM_POLYGON_OBJ *polyP,long startPnt,long userTag)
/*
** This Function Stores a Tptr Polygon In A Polygon Object
*/
{
 int    ret=DTM_SUCCESS ;
 long   sp,numPolyPts,addPolyPts;
 DTMDirection direction;
 double area ;
/*
** Initialise
*/
 addPolyPts = 0 ;
 numPolyPts = polyP->numPolyPts ;
/*
** Calculate Area And Direction
*/
 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ) goto errexit ;
/*
** Set Polygon AntiClockwise
*/
 if (direction == DTMDirection::Clockwise) bcdtmList_reverseTptrPolygonDtmObject (dtmP, startPnt);
/*
** Store Points
*/
 sp = startPnt ; 
 do
   {
    if( polyP->numPolyPts == polyP->memPolyPts ) 
      { 
       if( bcdtmPolygon_allocateMemoryPolygonObject(polyP)) goto errexit ;
      }
    (polyP->polyPtsP+polyP->numPolyPts)->x = pointAddrP(dtmP,sp)->x ;
    (polyP->polyPtsP+polyP->numPolyPts)->y = pointAddrP(dtmP,sp)->y ;
    (polyP->polyPtsP+polyP->numPolyPts)->z = pointAddrP(dtmP,sp)->z ;
    ++polyP->numPolyPts ;
    ++addPolyPts ;
    sp = nodeAddrP(dtmP,+sp)->tPtr ;
   } while ( sp != startPnt ) ;
 if( polyP->numPolyPts == polyP->memPolyPts ) 
   { 
    if( bcdtmPolygon_allocateMemoryPolygonObject(polyP)) goto errexit ;
   }
 (polyP->polyPtsP + polyP->numPolyPts)->x = pointAddrP(dtmP,sp)->x ;
 (polyP->polyPtsP + polyP->numPolyPts)->y = pointAddrP(dtmP,sp)->y ;
 (polyP->polyPtsP + polyP->numPolyPts)->z = pointAddrP(dtmP,sp)->z ;
 ++polyP->numPolyPts ;
 ++addPolyPts ;
/*
** Set Polygon Header Values
*/
 if( polyP->numPolygons == polyP->memPolygons ) 
  { 
   if( bcdtmPolygon_allocateMemoryPolygonObject(polyP)) goto errexit ;
   }
 (polyP->polyListP + polyP->numPolygons)->area     = area ;
 (polyP->polyListP + polyP->numPolygons)->d1       = 0.0  ;
 (polyP->polyListP + polyP->numPolygons)->firstPnt = numPolyPts ;
 (polyP->polyListP + polyP->numPolygons)->lastPnt  = numPolyPts + addPolyPts - 1 ;
 (polyP->polyListP + polyP->numPolygons)->userTag  = userTag ;
 (polyP->polyListP + polyP->numPolygons)->s1       = addPolyPts ;
 ++polyP->numPolygons ;
/*
** Clean Up
*/
 cleanup :
/*
** Job completed
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
BENTLEYDTM_Public int bcdtmPolygon_intersectPolygonAndTinHullDtmObject
(
 BC_DTM_OBJ      *dtmP,
 DPoint3d             *userPolyPtsP,
 long            numUserPolyPts,
 DTM_POLYGON_OBJ **polyPP,
 long            *intersectFlagP
)
/*
**
** This Function Intesections A User Polygon And Tin Hull
** The Intersected Polygons Are Written to Polygon Object "polyPP"
**
** Return Values for intersectFlagP ==  0  No Intersection
**                                  ==  1  Tin Hull Totally Within Or Coincident With Clip Polygon
**                                  ==  2  Intersection Polygon(s) Found 
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   numPolyPts=0,numHullPts=0  ;
 double ppTol,plTol,xMin,yMin,zMin,xMax,yMax,zMax ;
 DPoint3d    *p3d1P,*p3d2P,*polyPtsP=NULL,*hullPtsP=NULL ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Polygon With Tin Hull") ;
/*
** Initialise 
*/
 *intersectFlagP = 0 ;
 ppTol = plTol = dtmP->mppTol * 1000.0 ;
/*
** Create Polygon Object If Necessary
*/
 if( *polyPP == NULL ) 
   { 
    if( bcdtmPolygon_createPolygonObject(polyPP)) goto errexit ;
   }
/*
** Validate Slope Area Polgon
*/
 numPolyPts = numUserPolyPts ;
 if( numPolyPts > 0 )
   {
/*
**  Make A Local Copy Of The Polygon
*/
    polyPtsP = ( DPoint3d * ) malloc( numPolyPts * sizeof(DPoint3d)) ;
    if( polyPtsP == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( p3d1P = polyPtsP , p3d2P = userPolyPtsP ; p3d1P < polyPtsP + numPolyPts ; ++p3d1P , ++p3d2P ) *p3d1P = *p3d2P ;
/*
**  Validate Pad Polygon
*/
    if( bcdtmMath_validatePointArrayPolygon(&polyPtsP,&numPolyPts,ppTol) ) goto errexit ; 
   }
/*
**  Get Point Array Extents
*/
 if( numPolyPts > 0 )
   {
    if( bcdtmMath_getPointArrayExtents(polyPtsP,numPolyPts,&xMin,&yMin,&zMin,&xMax,&yMax,&zMax)) goto errexit ; ;
/*
**  Extract Tin Hull
*/
    if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
/*
**  Check For Overlap Of Bounding Rectangles
*/
    if( xMin < dtmP->xMax && xMax > dtmP->xMin && yMin < dtmP->yMax && yMax > dtmP->yMin )
      {
/*
**     Intersect Polygon And Tin Hull
*/ 
       if( bcdtmPolygon_intersectPolygons(hullPtsP,numHullPts,polyPtsP,numPolyPts,intersectFlagP,polyPP,ppTol,plTol)) goto errexit ;
       if( *intersectFlagP == 0 ) 
         { 
          bcdtmWrite_message(1,0,0,"Polygon Does Not Intersect Tin Hull") ;
          goto errexit ; 
         }
       else if( *intersectFlagP == 2 || *intersectFlagP == 4 ) *intersectFlagP = 1 ;
       else                                                    *intersectFlagP = 2 ; 
      } 
   }
 else 
   {
    if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
    if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*polyPP,hullPtsP,numHullPts,1)) goto errexit ;
    *intersectFlagP = 1 ;
   }
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != NULL ) free(hullPtsP) ; 
 if( polyPtsP != NULL ) free(polyPtsP) ; 
/*
** Job completed
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
BENTLEYDTM_Public int bcdtmPolygon_intersectPolygonAndTinHullsDtmObjects
(
 BC_DTM_OBJ      *dtm1P,
 BC_DTM_OBJ      *dtm2P,
 DPoint3d             *userPolyPtsP,
 long            numUserPolyPts,
 DTM_POLYGON_OBJ **polyPP,
 long            *intersectFlagP 
)
/*
**
** This Function Gets The Intesection Of The User Polygon With The Tin Hulls
**
** The Intersected Polygons Are Written to Polygon Object "polyPP"
**
** Return Values for intersectFlagP ==  0  No Intersection
**                                  ==  1  Tin Hull Totally Within Or Coincident With Clip Polygon
**                                  ==  2  Intersection Polygon(s) Found 
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    intFlag,numPolyPts=0,numHullPts1=0,numHullPts2=0  ;
 DPoint3d     *p3d1P,*p3d2P,*polyPtsP=NULL,*hullPts1P=NULL,*hullPts2P=NULL ;
 double  ppTol,plTol ;
 DTM_POLYGON_OBJ *tempPolyP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Intersecting Polygon With Tin Hulls") ;
    bcdtmWrite_message(0,0,0,"dtm1P          = %p",dtm1P) ;
    bcdtmWrite_message(0,0,0,"dtm2P          = %p",dtm2P) ;
    bcdtmWrite_message(0,0,0,"userPolyPtsP   = %p",userPolyPtsP) ;
    bcdtmWrite_message(0,0,0,"numUserPolyPts = %8ld",numUserPolyPts) ;
   }
/*
** Initialise 
*/
 *intersectFlagP = 0 ;
 if( dtm1P->mppTol <= dtm2P->mppTol ) ppTol = dtm1P->mppTol ;
 else                                 ppTol = dtm2P->mppTol ;
 ppTol = plTol = ppTol * 1000.0 ;
/*
** Create Polygon Object If Necessary
*/
 if( *polyPP == NULL ) 
   {
    if( bcdtmPolygon_createPolygonObject(polyPP)) goto errexit ;
   }
/*
** Validate Volume Area Polgon
*/
 numPolyPts = numUserPolyPts ;
 if( numPolyPts > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Polygon") ; 
/*
**  Make  A Local Copy Of The Volume Area Polygon
*/
    polyPtsP = ( DPoint3d * ) malloc( numPolyPts * sizeof(DPoint3d)) ;
    if( polyPtsP == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( p3d1P = polyPtsP , p3d2P = userPolyPtsP ; p3d1P < polyPtsP + numPolyPts ; ++p3d1P , ++p3d2P ) *p3d1P = *p3d2P ;
/*
** Validate Volume Polygon
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Polygon") ; 
    if( bcdtmMath_validatePointArrayPolygon(&polyPtsP,&numPolyPts,ppTol) ) goto errexit ;
   }
/*
**  Extract Tin Hulls
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Tin Hulls") ; 
 if( bcdtmList_extractHullDtmObject(dtm1P,&hullPts1P,&numHullPts1)) goto errexit ;
 if( bcdtmList_extractHullDtmObject(dtm2P,&hullPts2P,&numHullPts2)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numHullPts1 = %8ld ** numHullPts2 =%8ld",numHullPts1,numHullPts2) ;
/*
** Intersect Tin Hulls
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Tin Hulls") ; 
 if( bcdtmPolygon_intersectPolygons(hullPts1P,numHullPts1,hullPts2P,numHullPts2,intersectFlagP,polyPP,ppTol,plTol)) goto errexit ;
// if( bcdtmPolygon_intersectPointArrayPolygons(hullPts1P,numHullPts1,hullPts2P,numHullPts2,intersectFlagP,polyPP,ppTol,plTol)) goto errexit ;
 if( *intersectFlagP == 0 )
   {
    bcdtmWrite_message(1,0,0,"Tin Hulls Do Not Intersect") ;
    goto errexit ;
   }   
 else if( *intersectFlagP == 2 || *intersectFlagP == 4 ) *intersectFlagP = 1 ;
 else                                                    *intersectFlagP = 2 ;  
/*
** Intersect Volume Area Polygon And Intersected Tin Hull Polygons
*/
 if( numPolyPts > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Volume Polygon With Intersection Of Tin Hulls") ; 
    bcdtmPolygon_intersectPolygonWithPolgyonObject(polyPtsP,numPolyPts,*polyPP,ppTol,plTol,&intFlag,&tempPolyP) ;
    if( intFlag == 0 ) 
      { 
       bcdtmWrite_message(1,0,0,"Polygon And Tin Hulls Do Not Intersect") ;
       goto errexit ;
      }
    else if( intFlag == 2 || intFlag == 4 ) *intersectFlagP = 1 ;
    else                                    *intersectFlagP = 2 ; 
    bcdtmPolygon_deletePolygonObject(polyPP) ; 
    *polyPP   = tempPolyP ;
    tempPolyP = NULL ;
   }
/*
** Clean Up
*/
 cleanup :
 if( hullPts1P != NULL ) free(hullPts1P) ; 
 if( hullPts2P != NULL ) free(hullPts2P) ; 
 if( polyPtsP  != NULL ) free(polyPtsP) ; 
 if( tempPolyP != NULL ) bcdtmPolygon_deletePolygonObject(&tempPolyP) ; 
/*
** Job completed
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
BENTLEYDTM_Public int bcdtmPolygon_intersectPolygons
(
 DPoint3d             *poly1PtsP,
 long            numPoly1Pts,
 DPoint3d             *poly2PtsP,
 long            numPoly2Pts,
 long            *intersectResult,
 DTM_POLYGON_OBJ **polyPP,
 double          ppTol,
 double          plTol
)
/*
**
** This Function Gets The Intesection Polygons of Two Polygons
** And Store The Intersection Polygons In A Polygon Object
**
** Return Values for intersectResult == 0 No Intersection
**                                   == 1 Intersection
**                                   == 2 Polygon1 Is Inside Polygon2
**                                   == 3 Polygon2 Is Inside Polygon1
**                                   == 4 Coincident Polygons
** Notes :-
**
**  1. Assumes poly1PtsP And poly2PtsP Have Been Validated And Set AntiClockwise
**  2. Direction Of Intersected Polygons are set anticlockwise
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long    pnt,listPtr,numHullPts,numPoly1HullPts,numPoly2HullPts,startTime ;
 long    hullPnt1,hullPnt2,numPolyPts,polyOffset,dtmFeature ;
 DTMDirection direction;
 double  area ;
 DPoint3d     *p3dP,*polyPtsP=NULL ; 
 DTM_DAT_OBJ *dataP=NULL ;
 BC_DTM_OBJ  *dtmP=NULL   ;
 DTM_POLYGON_LIST  *polyListP ;
 wchar_t    dataFile[128] ;
 static  long dbgSequence=0 ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Intersecting Polygons") ;
    bcdtmWrite_message(0,0,0,"poly1PtsP    = %p",poly1PtsP) ;
    bcdtmWrite_message(0,0,0,"numPoly1Pts  = %8ld",numPoly1Pts) ;
    bcdtmWrite_message(0,0,0,"poly2PtsP    = %p",poly2PtsP) ;
    bcdtmWrite_message(0,0,0,"numPoly2Pts  = %8ld",numPoly2Pts) ;
    bcdtmWrite_message(0,0,0,"polyPP       = %p",*polyPP) ;
    bcdtmWrite_message(0,0,0,"ppTol        = %15.10lf",ppTol) ;
    bcdtmWrite_message(0,0,0,"plTol        = %15.10lf",plTol) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Poly 1 Points = %8ld",numPoly1Pts) ;
       for( p3dP = poly1PtsP ; p3dP < poly1PtsP + numPoly1Pts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Poly 1 Point[%6ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-poly1PtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
       bcdtmWrite_message(0,0,0,"Number Of Poly 2 Points = %8ld",numPoly2Pts) ;
       for( p3dP = poly2PtsP ; p3dP < poly2PtsP + numPoly2Pts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Poly 2 Point[%6ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-poly2PtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Initialise
*/
 *intersectResult = 0 ;
/*
** Calculate Polygon Areas
*/
 if( dbg )
   {
    bcdtmMath_getPolygonDirectionP3D(poly1PtsP,numPoly1Pts,&direction,&area) ;
    bcdtmWrite_message(0,0,0,"Polygon 1 ** Area = %12.3lf Direction = %2ld",area,direction) ;
    bcdtmMath_getPolygonDirectionP3D(poly2PtsP,numPoly2Pts,&direction,&area) ;
    bcdtmWrite_message(0,0,0,"Polygon 2 ** Area = %12.3lf Direction = %2ld",area,direction) ;
   }
/*
** Create Polygon Object If Necessary
*/
 if( *polyPP == NULL ) { if( bcdtmPolygon_createPolygonObject(polyPP)) goto errexit ; }
/*
** Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
/*
** Set Memory Allocation Parameters For dtmP Object
*/
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,(numPoly1Pts+numPoly2Pts)*2,100)) goto errexit ;
/*
**  Store Polygons In Dtm Object As Break Lines
*/
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::GraphicBreak,1,1,&dtmP->nullFeatureId,poly1PtsP,numPoly1Pts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::GraphicBreak,2,1,&dtmP->nullFeatureId,poly2PtsP,numPoly2Pts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,1,1,&dtmP->nullFeatureId,poly1PtsP,numPoly1Pts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,2,1,&dtmP->nullFeatureId,poly2PtsP,numPoly2Pts)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"problemUntriangulated.bcdtm") ;
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Dtm Object") ;
 startTime = bcdtmClock() ;
 DTM_NORMALISE_OPTION = FALSE ;                       // To Inhibit Normalisation Of Coordinates - function 
 dtmP->ppTol = ppTol / 100.0 ;
 dtmP->plTol = plTol / 100.0 ;  
 if( bcdtmObject_createTinDtmObject(dtmP,1,0.0)) goto errexit ;
 DTM_NORMALISE_OPTION = TRUE ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check If PRGN Is Set To Zero
*/
 for( pnt = 0 ; pnt < dtmP->numPoints ; ++pnt )
   {
    if( nodeAddrP(dtmP,pnt)->PRGN != 0 ) nodeAddrP(dtmP,pnt)->PRGN = 0 ;
   }
/*
** Remove None Feature Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing None Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ; 
/*
** Write Dtm Features
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Dtm Features = %8ld",dtmP->numFeatures) ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       bcdtmWrite_message(0,0,0,"dtmFeature[%4ld] ** userTag = %10I64d",dtmFeature,ftableAddrP(dtmP,dtmFeature)->dtmUserTag) ;
      }
   }
/*
** Get Intersect Polygons
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Intersect Polygons") ;
 startTime = bcdtmClock() ;
 if( bcdtmPolygon_getIntersectPolygonsDtmObject(dtmP,*polyPP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersect Polygons = %2ld",(*polyPP)->numPolygons) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"**** Intersect Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Determine Intersection Result
*/
 startTime = bcdtmClock() ;
 if( (*polyPP)->numPolygons > 0 ) *intersectResult = 1 ;
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Counting Hull Points") ;
/*
** Count Hull Points
*/
    numHullPts = numPoly1HullPts = numPoly2HullPts = 0 ;
    pnt = dtmP->hullPoint ;
    do
      {
       ++numHullPts ;
       hullPnt1 = hullPnt2 = FALSE ; 
       listPtr = nodeAddrP(dtmP,pnt)->fPtr ;
       while( listPtr != dtmP->nullPtr )
         {
          if(ftableAddrP(dtmP,flistAddrP(dtmP,listPtr)->dtmFeature)->dtmUserTag == 1 ) hullPnt1 = TRUE ;
          if(ftableAddrP(dtmP,flistAddrP(dtmP,listPtr)->dtmFeature)->dtmUserTag == 2 ) hullPnt2 = TRUE ;
          listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
         } 
       if( hullPnt1 == TRUE ) ++numPoly1HullPts ;
       if( hullPnt2 == TRUE ) ++numPoly2HullPts ;
       pnt = nodeAddrP(dtmP,pnt)->hPtr ;
      } while ( pnt != dtmP->hullPoint ) ;
/*
** Check For Polygon One Totally Within Polygon Two
*/
    if( numPoly2HullPts == numHullPts && numPoly1HullPts != numHullPts )
      {
       *intersectResult = 2 ; 
       if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*polyPP,poly1PtsP,numPoly1Pts,1)) goto errexit ;
      } 
/*
** Check For Polygon Two Totally Within Polygon One
*/
    if( numPoly1HullPts == numHullPts && numPoly2HullPts != numHullPts )
      {
       *intersectResult = 3 ; 
       if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*polyPP,poly2PtsP,numPoly2Pts,2) ) goto errexit ;
      } 
/*
** Check For Coincident Polygons
*/
    if( numPoly1HullPts == numHullPts && numPoly2HullPts == numHullPts )
      {
       *intersectResult = 4 ; 
       if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*polyPP,poly2PtsP,numPoly2Pts,2) ) goto errexit ;
      } 
   }
 if( tdbg ) bcdtmWrite_message(0,0,0,"**** Polygon Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Write Out Polygons For Checking Purposes
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Intersect Result = %2ld",*intersectResult) ;
    if( ! intersectResult ) bcdtmWrite_message(0,0,0,"Polygons Do Not Intersect" ) ;
    else
      {
       bcdtmWrite_message(0,0,0,"Number Of Intersect Polygons = %6ld",(*polyPP)->numPolygons ) ;
       if( bcdtmObject_createDataObject(&dataP)) goto errexit ;
       for( polyListP = (*polyPP)->polyListP ; polyListP < (*polyPP)->polyListP + (*polyPP)->numPolygons ; ++polyListP )
         { 
          bcdtmWrite_message(0,0,0,"Polygon[%4ld] ** Area = %12.3lf",(long)(polyListP-(*polyPP)->polyListP),polyListP->area) ;
          polyOffset = (long)(polyListP-(*polyPP)->polyListP) ;
          swprintf(dataFile,128,L"intersectPolygon%d%d.dat",dbgSequence,polyOffset) ;
          if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(*polyPP,polyOffset,&polyPtsP,&numPolyPts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDataObject(dataP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,nullGuid,polyPtsP,numPolyPts)) goto errexit ;
          if( bcdtmWrite_dataFileFromDataObject(dataP,dataFile)) goto errexit ;
          dataP->numPts = dataP->numFeatPts = 0 ;
          free(polyPtsP) ;
          polyPtsP = NULL ;
          ++dbgSequence ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 DTM_NORMALISE_OPTION = TRUE ;
 if( dtmP     != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ; 
 if( polyPtsP != NULL ) { free(polyPtsP) ; polyPtsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Polygons Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Polygons Error") ;
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
BENTLEYDTM_Public int bcdtmPolygon_getIntersectPolygonsDtmObject(BC_DTM_OBJ *dtmP,DTM_POLYGON_OBJ *polyP) 
/*
** This Function Gets The Internal Intersect Polygons And Copies Them To
** The Polygon Object
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   priorPnt,listPnt,nextPnt,nextPnt1,nextPnt2,scanPnt,listPtr;
 long   dtmFeature,numDtmFeatures;
 DTMDirection direction;
 DTMUserTag  userTag,userTag1,userTag2 ;
 double area ;
 static long dbgSequence=0 ;
 wchar_t   dtmFileName[256] ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Intersect Polygons") ;
/*
** Write Dtm To File
*/
 if( dbg )
   {
    swprintf(dtmFileName,256,L"poly%d.tin",dbgSequence) ;
    if( bcdtmWrite_toFileDtmObject(dtmP,dtmFileName) ) goto errexit ;
    ++dbgSequence ;  
   }
/*
** Initialise
*/
 userTag  = dtmP->nullUserTag ; 
 userTag1 = dtmP->nullUserTag ; 
 userTag2 = dtmP->nullUserTag ; 
/*
** Scan Points For More Than One Feature
*/
 for( scanPnt = 0 ; scanPnt < dtmP->numPoints ; ++scanPnt )
   {
    if( nodeAddrP(dtmP,scanPnt)->cPtr != dtmP->nullPtr && ! nodeAddrP(dtmP,scanPnt)->PRGN ) 
      {
/*
**     Determine Number Of Features On Point
*/
       bcdtmData_countNumberOfContinuingDtmFeaturesForPointDtmObject(dtmP,scanPnt,&numDtmFeatures) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Continuing Dtm Features For Point %6ld ** %12.4lf %12.4lf %10.4lf = %6ld",scanPnt,pointAddrP(dtmP,scanPnt)->x,pointAddrP(dtmP,scanPnt)->y,pointAddrP(dtmP,scanPnt)->z,numDtmFeatures) ;
       if( numDtmFeatures > 1 ) 
         {
          if( dbg )
            {
             if( nodeAddrP(dtmP,scanPnt)->hPtr == dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"Internal Point = %6ld Number Features = %6ld",scanPnt,numDtmFeatures) ;
             else                                                 bcdtmWrite_message(0,0,0,"External Point = %6ld Number Features = %6ld",scanPnt,numDtmFeatures) ;
             bcdtmWrite_message(0,0,0,"P = %6ld ** %10.4lf %10.4lf %8.4lf",scanPnt,pointAddrP(dtmP,scanPnt)->x,pointAddrP(dtmP,scanPnt)->y,pointAddrP(dtmP,scanPnt)->z) ;
            }
/*
**        Get Next Point For Features
*/
          listPtr  = nodeAddrP(dtmP,scanPnt)->fPtr ;
          nextPnt1 = nextPnt2 = dtmP->nullPnt ;
          while( listPtr != dtmP->nullPtr && nextPnt2 == dtmP->nullPnt )
            {
             nextPnt = flistAddrP(dtmP,listPtr)->nextPnt ;
             if( nextPnt != dtmP->nullPnt )
               {
                dtmFeature = flistAddrP(dtmP,listPtr)->dtmFeature ;
                userTag    =  ftableAddrP(dtmP,dtmFeature)->dtmUserTag ; 
                if( nextPnt1 == dtmP->nullPnt ) { nextPnt1 = nextPnt ; userTag1 = userTag ; }
                else                            { nextPnt2 = nextPnt ; userTag2 = userTag ; }
               }
             listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
            }
/*
**        Write Next Points And User Tags
*/
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"nextPnt1 = %6ld ** %10.4lf %10.4lf %8.4lf",nextPnt1,pointAddrP(dtmP,nextPnt1)->x,pointAddrP(dtmP,nextPnt1)->y,pointAddrP(dtmP,nextPnt1)->z) ;
             bcdtmWrite_message(0,0,0,"nextPnt2 = %6ld ** %10.4lf %10.4lf %8.4lf",nextPnt2,pointAddrP(dtmP,nextPnt2)->x,pointAddrP(dtmP,nextPnt2)->y,pointAddrP(dtmP,nextPnt2)->z) ;
             bcdtmWrite_message(0,0,0,"UserTag 1 = %2I64d UserTag2 = %2I64d",userTag1,userTag2) ;
            }
/*
**        Select Next Point
*/
          nextPnt = dtmP->nullPnt ;
/*
**        External Point
*/
          if( nodeAddrP(dtmP,scanPnt)->hPtr != dtmP->nullPnt )
            {
             if( nextPnt1 != nextPnt2 ) 
               {  
                if( nodeAddrP(dtmP,scanPnt)->hPtr == nextPnt2 ) { nextPnt = nextPnt1 ; userTag = userTag2 ; }
                else                                            { nextPnt = nextPnt2 ; userTag = userTag1 ; } 
                bcdtmPolygon_getNextPointForUserTagDtmObject(dtmP,userTag,nextPnt,&nextPnt1) ;
                if( nextPnt1 == scanPnt )  nextPnt = dtmP->nullPnt ;
                else
                  {
                   priorPnt = nodeAddrP(dtmP,scanPnt)->hPtr ;
                   do
                     {
                      if( ( priorPnt = bcdtmList_nextAntDtmObject(dtmP,scanPnt,priorPnt)) < 0 ) goto errexit ;
                      bcdtmPolygon_getNextPointForUserTagDtmObject(dtmP,userTag,priorPnt,&listPnt) ;
                     } while ( listPnt != scanPnt ) ;
                   if( bcdtmMath_pointSideOfDtmObject(dtmP,scanPnt,priorPnt,nextPnt) > 0 ) nextPnt = dtmP->nullPnt ;
                  }  
               }
            } 
/*
**        Internal Point
*/
          else
            {
             bcdtmPolygon_getNextPointForUserTagDtmObject(dtmP,userTag2,nextPnt1,&listPnt) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"00 ** listPnt = %8ld",listPnt) ;
             if( listPnt != scanPnt )
               {
                nextPnt = nextPnt1 ;
                do
                  {
                   if( ( nextPnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,nextPnt)) < 0 ) goto errexit ;
                   bcdtmPolygon_getNextPointForUserTagDtmObject(dtmP,userTag2,nextPnt,&listPnt) ;
                  } while ( listPnt != scanPnt ) ;
                if( bcdtmMath_pointSideOfDtmObject(dtmP,scanPnt,nextPnt1,nextPnt) < 0 ) nextPnt = dtmP->nullPnt ;
                if( nextPnt != dtmP->nullPnt ) nextPnt = nextPnt1 ; 
               }
             else
               {
                bcdtmPolygon_getNextPointForUserTagDtmObject(dtmP,userTag1,nextPnt2,&listPnt) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"01 ** listPnt = %8ld",listPnt) ;
                if( listPnt != scanPnt ) 
                  {
                   nextPnt = nextPnt2 ;
                   do
                     {
                      if( ( nextPnt = bcdtmList_nextClkDtmObject(dtmP,scanPnt,nextPnt)) < 0 ) goto errexit ;
                      bcdtmPolygon_getNextPointForUserTagDtmObject(dtmP,userTag1,nextPnt,&listPnt) ;
                     } while ( listPnt != scanPnt ) ;
                   if( bcdtmMath_pointSideOfDtmObject(dtmP,scanPnt,nextPnt2,nextPnt) < 0 ) nextPnt = dtmP->nullPnt ;
                   if( nextPnt != dtmP->nullPnt ) nextPnt = nextPnt2 ; 
                  }
               } 
            }
/*
**        Scan Back To Start Point
*/
          if( nextPnt != dtmP->nullPnt )
            { 
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"Next Point = %6ld ** %10.4lf %10.4lf %8.4lf",nextPnt,pointAddrP(dtmP,nextPnt)->x,pointAddrP(dtmP,nextPnt)->y,pointAddrP(dtmP,nextPnt)->z) ;
                bcdtmWrite_message(0,0,0,"UserTag    = %2I64d ",userTag) ;
               } 
             listPnt = scanPnt ;
             do
               { 
                nodeAddrP(dtmP,listPnt)->tPtr = nextPnt ;
                if( dbg ) bcdtmWrite_message(0,0,0,"listPnt = %6ld ** %10.4lf %10.4lf %10.4lf",listPnt,pointAddrP(dtmP,listPnt)->x,pointAddrP(dtmP,listPnt)->y,pointAddrP(dtmP,listPnt)->z) ;
                priorPnt = listPnt ; listPnt = nextPnt ; nextPnt = priorPnt ;
                if( (nextPnt = bcdtmList_nextClkDtmObject(dtmP,listPnt,nextPnt)) < 0 ) goto errexit ;
                while( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,listPnt,nextPnt))
                  { if( (nextPnt = bcdtmList_nextClkDtmObject(dtmP,listPnt,nextPnt)) < 0 ) goto errexit ; }
               } while ( listPnt != scanPnt ) ;
/*
**           Get Polygon Direction And If Anti Clockwise Store Polygon In Polygon Object
*/
             if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,scanPnt,&area,&direction) ) goto errexit ;
             if (direction == DTMDirection::AntiClockwise)
               {
/*
**              Copy Internal Tptr Polygon To Polygon Object
*/
                if( bcdtmPolygon_storeDtmObjectTptrPolygonInPolygonObject(dtmP,polyP,scanPnt,3)) goto errexit ;
/*
**              Mark Tptr Polygon To Polygon Object
*/
                listPnt = scanPnt ;
                do
                  {
                   nodeAddrP(dtmP,listPnt)->PRGN = 1 ;
                   listPnt = nodeAddrP(dtmP,listPnt)->tPtr ;
                  } while ( listPnt != scanPnt ) ;
/*
**              Null Out Tptr Polygon
*/         
                bcdtmList_nullTptrListDtmObject(dtmP,scanPnt) ;
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
** Job completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Intersect Polygons Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Intersect Polygons Error") ;
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
BENTLEYDTM_Private int bcdtmPolygon_getNextPointForUserTagDtmObject(BC_DTM_OBJ *dtmP,DTMUserTag userTag,long point,long *nextPointP)
/*
** This Function Gets The Next Point For A User Tag
** This Is A Specific Function For Polygon Intersection Applications Only
*/
{
 long listPtr ;
/*
** Initialise
*/
 *nextPointP = dtmP->nullPnt ;
/*
** Scan Features For Point
*/
 listPtr = nodeAddrP(dtmP,point)->fPtr ; 
 while ( listPtr != dtmP->nullPtr && *nextPointP == dtmP->nullPnt )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,listPtr)->dtmFeature)->dtmUserTag == userTag ) 
      { 
       *nextPointP = flistAddrP(dtmP,listPtr)->nextPnt ; 
      }
    listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
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
BENTLEYDTM_Private int bcdtmPolygon_intersectPolygonWithPolgyonObject
(
 DPoint3d             *polyPtsP,
 long            numPolyPts,
 DTM_POLYGON_OBJ *poly1P,
 double          ppTol,
 double          plistPTol,
 long            *intersectResultP,
 DTM_POLYGON_OBJ **poly2PP
)
/*
**
** This Function Gets The Intesection Polygons Of A 3D Polygon "polyPtsP"
** With All The Polygons In A Polygon Object "poly1P". 
**
** The Intersected polyPtsPgons Are Written to Polygon Object "poly2PP"
**
** Return Values for intersectResultP ==  Number Of Intersected Polygon
**
** Notes :-  Assumes polyPtsP Has Been Validated And Set AntiClockwise
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   numTempPts=0 ;
 DPoint3d    *p3d1P,*p3d2P,*tempPtsP=NULL ;
 DTM_POLYGON_LIST *plistP  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Polygon With With Polygon Object") ;
/*
** Create Polygon Object To Store Intersected Polygons
*/
 if( *poly2PP == NULL ) { if( bcdtmPolygon_createPolygonObject(poly2PP)) goto errexit ; }
/*
** Intersect polyPtsPgons
*/
 for( plistP = poly1P->polyListP ; plistP <  poly1P->polyListP + poly1P->numPolygons ; ++plistP )
   {
/*
**  Allocate DPoint3d Memory For Polygon Object Polygon
*/
    numTempPts = plistP->lastPnt - plistP->firstPnt + 1 ;
    tempPtsP = ( DPoint3d * ) malloc ( numTempPts * sizeof(DPoint3d)) ;
    if( tempPtsP == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Copy Polygon Object Polygon to DPoint3d Polygon
*/
    for( p3d2P = tempPtsP , p3d1P = poly1P->polyPtsP + plistP->firstPnt ;  p3d1P <= poly1P->polyPtsP + plistP->lastPnt ; ++p3d1P , ++p3d2P ) *p3d2P = *p3d1P ;
/*
**  Intersect polyPtsPgons
*/
    if( bcdtmPolygon_intersectPolygons(tempPtsP,numTempPts,polyPtsP,numPolyPts,intersectResultP,poly2PP,ppTol,plistPTol))  goto errexit ; 
/*
**  Free memory
*/
    free(tempPtsP) ;
    tempPtsP = NULL ;
   }
/*
** Set Intersect Flag to Number Of Intersected polyPtsPgons
*/
 *intersectResultP = (*poly2PP)->numPolygons ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( tempPtsP != NULL ) { free(tempPtsP) ; tempPtsP = NULL ; }
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
BENTLEYDTM_Public int bcdtmPolygon_storeDtmObjectTptrPolygonInPolygonObject
(
 BC_DTM_OBJ      *dtmP,
 DTM_POLYGON_OBJ *polyP,
 long            startPnt,
 long            userTag
)
/*
** This Function Stores a Tptr Polygon In A Polygon Object
*/
{
 int    ret=DTM_SUCCESS ;
 long   sp,numPolyPts,numPts;
 DTMDirection direction;
 double area ;
/*
** Initialise
*/
 numPts     = 0 ;
 numPolyPts = polyP->numPolyPts ;
/*
** Calculate area And direction
*/
 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ) goto errexit ;
/*
** Set Polygon AntiClockwise
*/
 if (direction == DTMDirection::Clockwise) bcdtmList_reverseTptrPolygonDtmObject (dtmP, startPnt);
/*
** Store Points
*/
 sp = startPnt ; 
 do
   {
    if( polyP->numPolyPts == polyP->memPolyPts ) { if( bcdtmPolygon_allocateMemoryPolygonObject(polyP)) goto errexit ; }
    (polyP->polyPtsP + polyP->numPolyPts)->x = pointAddrP(dtmP,sp)->x ;
    (polyP->polyPtsP + polyP->numPolyPts)->y = pointAddrP(dtmP,sp)->y ;
    (polyP->polyPtsP + polyP->numPolyPts)->z = pointAddrP(dtmP,sp)->z ;
    ++polyP->numPolyPts ;
    ++numPts ;
    sp = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPnt ) ;
 if( polyP->numPolyPts == polyP->memPolyPts ) { if( bcdtmPolygon_allocateMemoryPolygonObject(polyP)) goto errexit ; }
 (polyP->polyPtsP + polyP->numPolyPts)->x = pointAddrP(dtmP,sp)->x ;
 (polyP->polyPtsP + polyP->numPolyPts)->y = pointAddrP(dtmP,sp)->y ;
 (polyP->polyPtsP + polyP->numPolyPts)->z = pointAddrP(dtmP,sp)->z ;
 ++polyP->numPolyPts ;
 ++numPts ;
/*
** Set Polygon Header Values
*/
 if( polyP->numPolygons == polyP->memPolygons ) { if( bcdtmPolygon_allocateMemoryPolygonObject(polyP)) goto errexit ; }
 (polyP->polyListP + polyP->numPolygons)->area     = area ;
 (polyP->polyListP + polyP->numPolygons)->d1       = 0.0  ;
 (polyP->polyListP + polyP->numPolygons)->firstPnt = numPolyPts ;
 (polyP->polyListP + polyP->numPolygons)->lastPnt  = numPolyPts + numPts - 1 ;
 (polyP->polyListP + polyP->numPolygons)->userTag  = userTag ;
 (polyP->polyListP + polyP->numPolygons)->s1       = numPts ;
 ++polyP->numPolygons ;
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
BENTLEYDTM_Public int bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon
(
 DTM_POLYGON_OBJ *polygonP,
 long  offset,
 DPoint3d  **polygonPtsP,
 long *numPolygonPtsP
 )
/*
** This Function Copies A Polygon From A Polygon Object To A DPoint3d Polygon
*/
{
 int ret=DTM_SUCCESS ;
 DPoint3d *p3d1P,*p3d2P ;
/*
** Initialise
*/
 *numPolygonPtsP = 0 ;
 if( *polygonPtsP != NULL ) { free(*polygonPtsP) ; *polygonPtsP = NULL ; }
/*
** Allocate Memory
*/
 *numPolygonPtsP = (polygonP->polyListP+offset)->lastPnt - (polygonP->polyListP+offset)->firstPnt + 1 ;
 if( *numPolygonPtsP > 0 )
   {
    *polygonPtsP    = ( DPoint3d * ) malloc (*numPolygonPtsP * sizeof(DPoint3d)) ;
    if( *polygonPtsP == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
/*
**  Copy Points
*/
    for( p3d1P = *polygonPtsP, p3d2P = polygonP->polyPtsP + (polygonP->polyListP+offset)->firstPnt ;  p3d2P <= polygonP->polyPtsP + (polygonP->polyListP+offset)->lastPnt ; ++p3d1P , ++p3d2P ) *p3d1P = *p3d2P ;
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
BENTLEYDTM_Public int bcdtmPolygon_initialisePolygonObject(DTM_POLYGON_OBJ *Poly)
/*
** This Function Initialises A Polygon Object
*/
{
/*
** Test For Valid Polygon Object
*/
// if( bcdtmPolygon_testForValidPolygonObject( Poly ) ) return(1) ;
/*
** Initialise Polygon Object
*/
 Poly->numPolygons  = Poly->memPolygons = 0 ;
 Poly->numPolyPts   = Poly->memPolyPts  = 0 ;
 Poly->iniMemPolygons = Poly->incMemPolygons = 10  ;
 Poly->iniMemPolyPts = Poly->incMemPolyPts = 1000 ;
 Poly->polyListP   = NULL  ;
 Poly->polyPtsP   = NULL  ;
/*
**  Job Completed
*/
 return(0) ;
}/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmPolygon_createPolygonObject(DTM_POLYGON_OBJ **polyPP )
/*
** This Function Creates a Polygon Object and
** Returns a Pointer To The Object
*/
{

    //  Check Polygon Object Is NULL

    if( *polyPP != NULL )
    {
        bcdtmWrite_message(1,0,0,"Cannot Create Polygon Object") ;
        return(DTM_ERROR) ;
    }

    // Create Polygon Object

    *polyPP = ( DTM_POLYGON_OBJ * ) malloc ( sizeof(DTM_POLYGON_OBJ)) ;
    if( *polyPP == NULL )
        {
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         return(DTM_ERROR) ;
        } 

    // Initialise Polygon Object

    (*polyPP)->polyListP = NULL ;
    (*polyPP)->polyPtsP = NULL ;
    bcdtmPolygon_initialisePolygonObject(*polyPP) ;
    return(DTM_SUCCESS) ;

     
   
/*
** Initialise Variables
*/
// *polyObjP = NULL ;
/*
** Scan Polygon Object Pointer List For Null Entry
*/
/*
 for ( i = 0 ; i < DTM_MAX_POLYGON_OBJS ; ++i )
   {
    if ( polyObjPtrs[i] == NULL )
      {
       polyObjPtrs[i] = ( DTM_POLYGON_OBJ * ) malloc ( sizeof(DTM_POLYGON_OBJ)) ;
       if( polyObjPtrs == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return(1) ; }
       *polyObjP = polyObjPtrs[i] ;
       (*polyObjP)->polyListP = NULL ;
       (*polyObjP)->polyPtsP = NULL ;
       bcdtmPolygon_initialisePolygonObject(*polyObjP) ;
       return(0) ;
      }
   }
*/   
/*
** No Entries
*/
// bcdtmWrite_message(1,0,0,"Maximum Number Of Polygon Objects Exceeded") ;
// return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmPolygon_deletePolygonObject(DTM_POLYGON_OBJ **polyPP)
/*
** This Function Deletes a Polygon Object
*/
{
/*
** Check For Null Data Object
*/
 if( *polyPP == NULL ) return(0) ;
 if(bcdtmPolygon_freeMemoryPolygonObject(*polyPP)) 
     return(1) ;
 else
    {
     *polyPP = NULL ;
     return(0) ;
    }
/*
** Scan Data Object Pointer List For Data Entry
*/
/*
 for ( i = 0 ; i < DTM_MAX_POLYGON_OBJS ; ++i )
   {
    if ( polyObjPtrs[i] == *Poly )
      {
       if(bcdtmPolygon_freeMemoryPolygonObject(*Poly)) return(1) ;
       free(polyObjPtrs[i]) ;
       polyObjPtrs[i] = *Poly = NULL ;
       return(0) ;
      }
   }
*/   
/*
** No Entries
*/
// bcdtmWrite_message(1,0,0,"Entry For Polygon Object Not Found") ;
// return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmPolygon_deleteAllPolygonObjects(void)
/*
** This Function Deletes a Polygon Object
*/
{
/*
** Scan Data Object Pointer List For Data Object Entries
*/
/*
 for ( i = 0 ; i < DTM_MAX_POLYGON_OBJS ; ++i )
   {
    if ( polyObjPtrs[i] != NULL )
      {
       if( bcdtmPolygon_freeMemoryPolygonObject(polyObjPtrs[i])) return(1) ;
       free(polyObjPtrs[i]) ; polyObjPtrs[i] = NULL ;
      }
   }
*/   
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
BENTLEYDTM_Public int bcdtmPolygon_testForValidPolygonObject( DTM_POLYGON_OBJ *Poly )
/*
** This Function Test For A Valid Data Object
*/
{
return 0;
///*
//** Scan Data Object Pointer List For Data Entry
//*/
// if( Poly != NULL )
//   {
////    for ( i = 0 ; i < DTM_MAX_POLYGON_OBJS ; ++i )
////      if ( polyObjPtrs[i] == Poly ) return(0) ;
//   }
///*
//** No Entry Found
//*/
// bcdtmWrite_message(2,0,0,"Invalid Polygon Object %p",Poly) ;
// return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   bcdtmPolygon_storePointArrayPolygonInPolygonObject               |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmPolygon_storePointArrayPolygonInPolygonObject(DTM_POLYGON_OBJ *Poly,DPoint3d *Poly3D,long NumPolyPts,long UserTag)
/*
** This Function Stores a 3D Polygon In A Polygon Object
*/
{
 DTMDirection Direction;
 double Area ;
 DPoint3d    *p3d ;
/*
** Calculate Area And Direction
*/
 bcdtmMath_getPolygonDirectionP3D(Poly3D,NumPolyPts,&Direction,&Area) ;
/*
** Set Polygon AntiClockwise
*/
 if (Direction == DTMDirection::Clockwise) bcdtmMath_reversePolygonDirectionP3D (Poly3D, NumPolyPts);
/*
** Store Header
*/
 if( Poly->numPolygons == Poly->memPolygons ) { if( bcdtmPolygon_allocateMemoryPolygonObject(Poly)) return(1) ; }
 (Poly->polyListP + Poly->numPolygons)->area = Area ;
 (Poly->polyListP + Poly->numPolygons)->d1   = 0.0  ;
 (Poly->polyListP + Poly->numPolygons)->firstPnt = Poly->numPolyPts ;
 (Poly->polyListP + Poly->numPolygons)->lastPnt  = Poly->numPolyPts + NumPolyPts - 1 ;
 (Poly->polyListP + Poly->numPolygons)->userTag  = UserTag ;
 (Poly->polyListP + Poly->numPolygons)->s1   = NumPolyPts ;
 ++Poly->numPolygons ;
/*
** Store Points
*/
 for( p3d = Poly3D ; p3d < Poly3D + NumPolyPts ; ++p3d )
   {
    if( Poly->numPolyPts == Poly->memPolyPts ) { if( bcdtmPolygon_allocateMemoryPolygonObject(Poly)) return(1) ; }
    (Poly->polyPtsP + Poly->numPolyPts)->x = p3d->x ;
    (Poly->polyPtsP + Poly->numPolyPts)->y = p3d->y ;
    (Poly->polyPtsP + Poly->numPolyPts)->z = p3d->z ;
    ++Poly->numPolyPts ;
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
BENTLEYDTM_Public int bcdtmPolygon_intersectPointArrayPolygons(DPoint3d *poly1PtsP,long numPoly1Pts,DPoint3d *poly2PtsP,long numPoly2Pts,long *intersectResult,DTM_POLYGON_OBJ **polyPP,double ppTol,double p2lTol)
/*
**
** This Function Gets The Intesection Polygons of Two Polygons
** And Store The Intersection Polygons In A Polygon Object
**
** Return Values for intersectResult == 0 No Intersection
**                                   == 1 Intersection
**                                   == 2 Polygon1 Is Inside Polygon2
**                                   == 3 Polygon2 Is Inside Polygon1
**                                   == 4 Coincident Polygons
** Notes :-
**
**  1. Assumes poly1PtsP And poly2PtsP Have Been Validated And Set AntiClockwise
**  2. Direction Of Intersected Polygons are set anticlockwise
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    pnt,listPtr,numHullPts,numPoly1HullPts,numPoly2HullPts ;
 long    hullPnt1,hullPnt2 ;
 DPoint3d     *p3d1P,*p3d2P,*polyPtsP=NULL,breakPts[2] ; 
 BC_DTM_OBJ  *dtmP=NULL   ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting 3D Polygons") ;
/*
** Initialise
*/
 *intersectResult = 0 ;
/*
** Create Polygon Object If Necessary
*/
 if( *polyPP == NULL ) { if( bcdtmPolygon_createPolygonObject(polyPP)) goto errexit ; }
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,(numPoly1Pts+numPoly2Pts)*2,100) ;
/*
** Write Polygons To DTM Object As Break Lines
*/
 for( p3d1P = poly1PtsP , p3d2P = poly1PtsP + 1 ; p3d2P < poly1PtsP + numPoly1Pts ; ++p3d1P , ++p3d2P )
   {
    breakPts[0].x = p3d1P->x ;
    breakPts[0].y = p3d1P->y ;
    breakPts[0].z = p3d1P->z ;
    breakPts[1].x = p3d2P->x ;
    breakPts[1].y = p3d2P->y ;
    breakPts[1].z = p3d2P->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,1,1,&nullFeatureId,breakPts,2)) goto errexit ;
   }
 for( p3d1P = poly2PtsP , p3d2P = poly2PtsP + 1 ; p3d2P < poly2PtsP + numPoly2Pts ; ++p3d1P , ++p3d2P )
   {
    breakPts[0].x = p3d1P->x ;
    breakPts[0].y = p3d1P->y ;
    breakPts[0].z = p3d1P->z ;
    breakPts[1].x = p3d2P->x ;
    breakPts[1].y = p3d2P->y ;
    breakPts[1].z = p3d2P->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,2,1,&nullFeatureId,breakPts,2)) goto errexit ;
   }
/*
** Triangulate DTM Object
*/ 
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Remove None Feature Hull Lines
*/
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ; 
/*
** Get Intersect Polygons
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Intersect Polygons") ;
 if( bcdtmPolygon_getIntersectPolygonsDtmObject(dtmP,*polyPP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersect Polygons = %2ld",(*polyPP)->numPolygons) ;
/*
** Determine Intersection Result
*/
 if( (*polyPP)->numPolygons > 0 ) *intersectResult = 1 ;
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Counting Hull Points") ;
/*
** Count Hull Points
*/
    numHullPts = numPoly1HullPts = numPoly2HullPts = 0 ;
    pnt = dtmP->hullPoint ;
    do
      {
       ++numHullPts ;
       hullPnt1 = hullPnt2 = FALSE ; 
       listPtr = nodeAddrP(dtmP,pnt)->fPtr ;
       while( listPtr != dtmP->nullPtr )
         {
          if( ftableAddrP(dtmP,flistAddrP(dtmP,listPtr)->dtmFeature)->dtmUserTag == 1 ) hullPnt1 = TRUE ;
          if( ftableAddrP(dtmP,flistAddrP(dtmP,listPtr)->dtmFeature)->dtmUserTag == 2 ) hullPnt2 = TRUE ;
          listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
         } 
       if( hullPnt1 == TRUE ) ++numPoly1HullPts ;
       if( hullPnt2 == TRUE ) ++numPoly2HullPts ;
       pnt = nodeAddrP(dtmP,pnt)->hPtr ;
      } while ( pnt != dtmP->hullPoint ) ;
/*
** Check For Polygon One Totally Within Polygon Two
*/
    if( numPoly2HullPts == numHullPts && numPoly1HullPts != numHullPts )
      {
       *intersectResult = 2 ; 
       if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*polyPP,poly1PtsP,numPoly1Pts,1)) goto errexit ;
      } 
/*
** Check For Polygon Two Totally Within Polygon One
*/
    if( numPoly1HullPts == numHullPts && numPoly2HullPts != numHullPts )
      {
       *intersectResult = 3 ; 
       if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*polyPP,poly2PtsP,numPoly2Pts,2) ) goto errexit ;
      } 
/*
** Check For Coincident Polygons
*/
    if( numPoly1HullPts == numHullPts && numPoly2HullPts == numHullPts )
      {
       *intersectResult = 4 ; 
       if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*polyPP,poly2PtsP,numPoly2Pts,2) ) goto errexit ;
      } 
   }
/*
** Clean Up
*/
 cleanup :
 if( dtmP     != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ; 
 if( polyPtsP != NULL ) { free(polyPtsP) ; polyPtsP = NULL ; }
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
BENTLEYDTM_Public int bcdtmPolygon_intersectPointArrayPolygonWithPolgyonObject
(
 DPoint3d             *polygonPtsP,
 long            numPolygonPts,
 DTM_POLYGON_OBJ *polygon1P,
 double          ppTol,
 double          plTol,
 long            *intersectFlagP,
 DTM_POLYGON_OBJ **polygon2PP
 )
/*
**
** This Function Gets The Intesection polygonPtsPgons of a 3D polygonPtsPgon "polygonPtsP"
** With A polygonPtsPgon Object "*polygon1P". 
** The Intersected polygonPtsPgons Are Written to polygonPtsPgon Object "polygon2PP"
**
** Return Values for intersectFlagP ==  Number Of Intersected polygonPtsPgons
**
** Notes :-  Assumes polygonPtsP Has Been Validated And Set AntiClockwise
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   numNewPolyPts=0 ;
 DPoint3d    *p3dP,*np3dP,*newPolyPtsP=NULL ;
 DTM_POLYGON_LIST *pl  ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting A 3D Polygon With A Polygon Object") ;
/*
** Create Polygon Object If Necessary
*/
 if( *polygon2PP == NULL ) 
   { 
    if( bcdtmPolygon_createPolygonObject(polygon2PP)) goto errexit ; 
   }
/*
** Intersect Polygons
*/
 for( pl = polygon1P->polyListP ; pl <  polygon1P->polyListP + polygon1P->numPolygons ; ++pl )
   {
/*
** Allocate DPoint3d Memory For polygonPtsPgon Object polygonPtsPgon
*/
    numNewPolyPts = pl->lastPnt - pl->firstPnt + 1 ;
    newPolyPtsP = ( DPoint3d * ) malloc ( numNewPolyPts * sizeof(DPoint3d)) ;
    if( newPolyPtsP == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Copy Points
*/
    for( np3dP = newPolyPtsP , p3dP = polygon1P->polyPtsP + pl->firstPnt ;  p3dP <= polygon1P->polyPtsP + pl->lastPnt ; ++p3dP , ++np3dP ) 
      {
       *np3dP = *p3dP ;
      } 
/*
**  Intersect polygonPtsPgons
*/
    if( bcdtmPolygon_intersectPointArrayPolygons(polygonPtsP,numPolygonPts,newPolyPtsP,numNewPolyPts,intersectFlagP,polygon2PP,ppTol,plTol)) 
      { 
       free( newPolyPtsP) ; 
       goto errexit ; 
      }
/*
**   Free memory
*/
    free(newPolyPtsP) ; 
    newPolyPtsP = NULL ;
   }
/*
** Set Intersect Flag to Number Of Intersected Polygons
*/
 *intersectFlagP = (*polygon2PP)->numPolygons ;
/*
** Clean Up
*/
 cleanup : 
 if( newPolyPtsP != NULL ) free(newPolyPtsP) ; 
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
BENTLEYDTM_Public int bcdtmPolygon_getTagListFromTagObject(TAGOBJ *Tag,long TagOfs,long **TagList,long *NumTagValues,long *Utag1,long *Utag2,long *Utag3,long *Utag4)
/*
** This Function Geta a TagList From A Tag Object
*/
{
 long  *pt1,*pt2,*ptl,*ptv ;
/*
** Initialise
*/
 *NumTagValues = 0 ;
 *Utag1 = *Utag2 = *Utag3 = *Utag4 = DTM_NULL_PNT ;
/*
** Test For Valid Tag Object
*/
 if( bcdtmPolygon_testForValidTagObject(Tag)) return(1) ;
/*
** Test For Legal Tag Offset
*/
 if( TagOfs < 0 || TagOfs >= Tag->NTAG ) { bcdtmWrite_message(2,0,0,"Invalid Tag Offset") ; return(1) ; }
/*
** Allocate memory For Tag List
*/
 if( *TagList != NULL ) free(*TagList) ;
 *NumTagValues = (Tag->PTAG+TagOfs)->LTAG - (Tag->PTAG+TagOfs)->FTAG + 1 ;
 *TagList = ( long * ) malloc( *NumTagValues * sizeof(long)) ;
 if( *TagList == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return(1) ; }
/*
** Copy Tag Values
*/
 pt1 = Tag->PVAL+(Tag->PTAG+TagOfs)->FTAG ;
 pt2 = Tag->PVAL+(Tag->PTAG+TagOfs)->LTAG ;
 ptl = *TagList ;
 for( ptv = pt1 ; ptv <= pt2 ; ++ptv ) { *ptl = *ptv ; ++ptl ; }
/*
** Copy User Tags
*/
 *Utag1 = (Tag->PTAG + TagOfs)->UTAG[0] ;
 *Utag2 = (Tag->PTAG + TagOfs)->UTAG[1] ;
 *Utag3 = (Tag->PTAG + TagOfs)->UTAG[2] ;
 *Utag4 = (Tag->PTAG + TagOfs)->UTAG[3] ;
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
BENTLEYDTM_Public int bcdtmPolygon_copyPolygonObjectToPolygonObject(DTM_POLYGON_OBJ *Poly1,DTM_POLYGON_OBJ *Poly2)
/*
** This Function Copies A Polygon Object To A Polygon Object
*/
{
 DTM_POLYGON_LIST *pl1,*pl2 ;
 DPoint3d    *pd1,*pd2 ;
/*
** Test For Valid Polygon Objects
*/
 if( bcdtmPolygon_testForValidPolygonObject(Poly1)) return(1) ;
 if( bcdtmPolygon_testForValidPolygonObject(Poly2)) return(1) ;
/*
** Free Memory For Poly2 Object
*/
 bcdtmPolygon_freeMemoryPolygonObject(Poly2) ;
/*
** Copy Polygon Object Variables
*/
 *Poly2 = *Poly1 ;
 Poly2->polyListP = NULL ;
 Poly2->polyPtsP = NULL ;
/*
** Copy Polygon Headers
*/
 Poly2->polyListP = ( DTM_POLYGON_LIST * ) malloc(Poly2->memPolygons * sizeof(DTM_POLYGON_LIST)) ;
 if( Poly2->polyListP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return(1) ; } 
 for( pl2 = Poly2->polyListP , pl1 = Poly1->polyListP ; pl1 < Poly1->polyListP + Poly1->memPolygons ; ++pl2 , ++pl1 ) *pl2 = *pl1 ;
/*
** Copy Polygon Points
*/
 Poly2->polyPtsP = ( DPoint3d * ) malloc(Poly2->memPolyPts * sizeof(DPoint3d)) ;
 if( Poly2->polyListP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; free(Poly2->polyListP) ; return(1) ; } 
 for( pd2 = Poly2->polyPtsP , pd1 = Poly1->polyPtsP ; pd1 < Poly1->polyPtsP + Poly1->memPolyPts ; ++pd2 , ++pd1 ) *pd2 = *pd1 ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   int bcdtmPolygon_allocateMemoryPolygonObject(DTM_POLYGON_OBJ *Polygon)     |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmPolygon_allocateMemoryPolygonObject(DTM_POLYGON_OBJ *Poly)
/*
** This Routine Gets the initial memory for the input data
*/
{
 DTM_POLYGON_LIST  *pl ;
 DPoint3d     *pd ;
/*
** Allocate Memory
*/
 if( Poly->memPolyPts != 0 ) { if( bcdtmPolygon_incrementMemoryPolygonObject(Poly)) return(1) ; }
 else
   {
    Poly->memPolygons = Poly->iniMemPolygons ;
    Poly->memPolyPts  = Poly->iniMemPolyPts ;
    Poly->polyListP  = ( DTM_POLYGON_LIST * ) malloc ( Poly->memPolygons * sizeof(DTM_POLYGON_LIST)) ;
    Poly->polyPtsP  = ( DPoint3d    * ) malloc ( Poly->memPolyPts  * sizeof(DPoint3d)) ;
    if( Poly->polyListP == NULL || Poly->polyPtsP == NULL )
      {
       if( Poly->polyListP != NULL ) { free(Poly->polyListP) ; Poly->polyListP = NULL ; }
       if( Poly->polyPtsP != NULL ) { free(Poly->polyPtsP) ; Poly->polyPtsP = NULL ; }
       Poly->memPolygons = Poly->memPolyPts = 0 ; 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       return(1) ;
      }
/*
** Initialise Data Values
*/
    for( pl = Poly->polyListP ; pl < Poly->polyListP + Poly->memPolygons ; ++pl ) { pl->area = pl->perimeter = pl->d1 = 0.0 ; pl->firstPnt = pl->lastPnt = pl->userTag = pl->s1 = DTM_NULL_PNT ; }
    for( pd = Poly->polyPtsP ; pd < Poly->polyPtsP + Poly->memPolyPts  ; ++pd ) { pd->x = pd->y = pd->z = 0.0 ; }
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
BENTLEYDTM_Public int bcdtmPolygon_incrementMemoryPolygonObject(DTM_POLYGON_OBJ *Poly)
/*
** This Routine Increments the Memory for A Polygon Object
*/
{
 DTM_POLYGON_LIST  *pl ;
 DPoint3d     *pd ;
/*
** Allocate Memory For Polygon Header List
*/
 if( Poly->numPolygons == Poly->memPolygons )
   {
    Poly->memPolygons = Poly->memPolygons + Poly->incMemPolygons ;
    Poly->polyListP = ( DTM_POLYGON_LIST * ) realloc( Poly->polyListP,Poly->memPolygons * sizeof(DTM_POLYGON_LIST)) ;
    if( Poly->polyListP == NULL )
      {
       if( Poly->polyPtsP != NULL ) { free(Poly->polyPtsP) ; Poly->polyPtsP = NULL ; }
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       return(1) ;
      } 
    for( pl = Poly->polyListP + Poly->numPolygons ; pl < Poly->polyListP + Poly->memPolygons ; ++pl ) { pl->area = pl->perimeter = pl->d1 = 0.0 ; pl->firstPnt = pl->lastPnt = pl->userTag = pl->s1 = DTM_NULL_PNT ; }
   }
/*
** Allocate Memory For Polygon Points
*/
 if( Poly->numPolyPts == Poly->memPolyPts )
   {
    Poly->memPolyPts = Poly->memPolyPts + Poly->incMemPolyPts ;
    Poly->polyPtsP = ( DPoint3d * ) realloc( Poly->polyPtsP,Poly->memPolyPts * sizeof(DPoint3d)) ;
    if( Poly->polyPtsP == NULL )
      {
       if( Poly->polyListP != NULL ) { free(Poly->polyListP) ; Poly->polyListP = NULL ; }
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       return(1) ;
      } 
    for( pd = Poly->polyPtsP + Poly->numPolyPts ; pd < Poly->polyPtsP + Poly->memPolyPts  ; ++pd ) { pd->x = pd->y = pd->z = 0.0 ; }
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
BENTLEYDTM_Public int bcdtmPolygon_freeMemoryPolygonObject(DTM_POLYGON_OBJ *Poly)
/*
** This Function Frees The Memory For A Polygon Object
*/
{
/*
** Free memory
*/
 if( Poly->polyListP != NULL ) { free(Poly->polyListP) ; Poly->polyListP = NULL ; } 
 if( Poly->polyPtsP != NULL ) { free(Poly->polyPtsP) ; Poly->polyPtsP = NULL ; } 
/*
** Reset Polygon Object Variables
*/
 Poly->numPolygons = Poly->memPolygons = 0 ;
 Poly->numPolyPts  = Poly->memPolyPts  = 0 ;
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
BENTLEYDTM_Public int bcdtmPolygon_testForValidTagObject(TAGOBJ *Tag )
/*
** This Function Test For A Valid Data Object
*/
{
/*
** Scan Data Object Pointer List For Data Entry
*/
 if( Tag != NULL )
   {
//    for ( i = 0 ; i < MAXTAOBJ ; ++i )
//      if ( TAOBJPTR[i] == Tag ) return(0) ;
   }
/*
** No Entry Found
*/
 bcdtmWrite_message(1,0,0,"Not A Valid Tag Object") ;
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmPolygon_intersectPolygonObjectPolygons
(
 DTM_POLYGON_OBJ *polygonP,
 DTM_POLYGON_OBJ **intPolygonPP,
 TAGOBJ          **intTagObjPP
 ) 
/*
** This Function Intersects The Polygons In Polygon Object Poly And Stores Them
** In Polygon Object IntPoly
*/
{
 int         ret=DTM_SUCCESS ;
 long        polygon,numPolyPts ;
 DPoint3d         *polyPtsP=NULL ;
 BC_DTM_OBJ  *dtmP=NULL  ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Check For Polygons In Polygon Object
*/
 if( polygonP->numPolygons > 0 ) 
   {
/*
**  Create DTM Object
*/
    if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
/*
**  Copy Polygon Object Polygons To Data Object
*/
    for( polygon = 0 ; polygon < polygonP->numPolygons ; ++polygon )
      {
       if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polygonP,polygon,&polyPtsP,&numPolyPts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,polyPtsP,numPolyPts)) goto errexit ;
      }
/*
**  Triangulate DTM Object
*/ 
    if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
**  Extract Polygons From DTM Object
*/
   if( bcdtmPolygon_extractPolygonsDtmObject(dtmP,intPolygonPP,intTagObjPP)) goto errexit ;
  } 
/*
** Clean Up
*/
 cleanup :
 if( dtmP     != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( polyPtsP != NULL ) free(polyPtsP) ;
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
BENTLEYDTM_Public int bcdtmPolygon_extractPolygonsDtmObject
(
 BC_DTM_OBJ      *dtmP,
 DTM_POLYGON_OBJ **polygonPP,
 TAGOBJ          **tagPP
)  
/*
** This Function Extracts Polygons From A  
** DTM Object And Stores Them In A Polygon Object
**
*/
{
 int    ret=DTM_SUCCESS ;
 long   pp,np,cp,spnt,clc,cln,clt,ofs,tag,numFeatures,polyNumber=0,numMarked ;
 DTMDirection direction;
 long   node,dtmFeature,*tagListP=NULL,tagNum=0,tagMem=0,tagMemInc=100 ;
 double area ;
 unsigned char   *lineMarkP=NULL,*pc ;
 DTM_TIN_NODE    *nodeP ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 DTM_POLYGON_LIST *ppl ;

// Allocate Memory

 lineMarkP = ( unsigned char * ) malloc ((dtmP->cListPtr/8+1)* sizeof(char)) ;
 if( lineMarkP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
    goto errexit ; 
   }
 for( pc = lineMarkP ; pc < lineMarkP + dtmP->cListPtr/8+1 ; ++pc ) *pc = 0 ;

// Scan For Points With More Than One Feature I.E. Intersecting Polygons

 for( spnt = 0 ; spnt < dtmP->numPoints ; ++spnt )
   {
    if( nodeAddrP(dtmP,spnt)->cPtr != dtmP->nullPtr ) 
      {

       // Determine Number Of Features On Point

       bcdtmPolygon_countNumberOfNonDtmPolygonFeaturesForPointDtmObject(dtmP,spnt,&numFeatures) ;
       if( numFeatures > 1 ) 
         {

          // Scan Point And Get Prior Points

          clc = nodeAddrP(dtmP,spnt)->cPtr ;
          while ( clc != dtmP->nullPtr )
            {
             pp  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;   

             // Check If Prior Point Points To Current Point If So Extract Polygon

             cln = nodeAddrP(dtmP,pp)->fPtr ;
             while( cln != dtmP->nullPtr )
               {
                bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,cln)->dtmFeature,pp,&np) ;
                cln = flistAddrP(dtmP,cln)->nextPtr ;

                // Extract Polygon

                if( np == spnt )
                  {
                   tagNum = 0 ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs,pp,spnt) ) goto errexit ;
                   if(! bcdtmFlag_testFlag(lineMarkP,ofs) )
                     {
                      np = pp ; 
                      cp = spnt ;
                      do
                        {

                         // Scan Clockwise To Next Feature lineMarkP

                         if(( np = bcdtmList_nextClkDtmObject(dtmP,cp,np)) < 0 ) goto errexit ;
                         while ( ! bcdtmList_testForLineOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,np,cp) )
                           {
                            if(( np = bcdtmList_nextClkDtmObject(dtmP,cp,np)) < 0 ) goto errexit ;
                           }

                         // Get Tag Value For lineMarkP Cp-Np

                         clt = nodeAddrP(dtmP,cp)->fPtr ;  
                         while( clt != dtmP->nullPtr )
                           {
                            if( flistAddrP(dtmP,clt)->nextPnt == np )
                              {
                               tag = (long)(ftableAddrP(dtmP,flistAddrP(dtmP,clt)->dtmFeature)->dtmUserTag) ;
                               if( bcdtmPolygon_storePolygonTag(&tagListP,1,tag,&tagNum,&tagMem,tagMemInc) )
                                 { 
                                  if( tagListP != NULL ) free(tagListP) ;
                                  goto errexit ; 
                                 } 
                              }
                            clt = flistAddrP(dtmP,clt)->nextPtr ;
                           }                        

                         //  Set Tptr List

                         nodeAddrP(dtmP,cp)->tPtr = np ;

                         //  Initialise For Next Point

                         pp = np ; np = cp ; cp = pp ;
                        } while ( cp != spnt ) ;

                      //  Get Polygon direction And If Anti Clockwise Store Polygon In Polygon Object

                      if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,spnt,&area,&direction) ) { free(lineMarkP) ;  goto errexit ; }
                      if (direction == DTMDirection::AntiClockwise)
                        {

                         //  If Necessary Create Polygon Object

                         if( *polygonPP == NULL ) 
                           {
                            if( bcdtmPolygon_createPolygonObject(polygonPP)) goto errexit ;
                           } 

                         //  Copy Internal Tptr Polygon To Polygon Object

                         if( bcdtmPolygon_storeTptrPolygonInPolygonObjectDtmObject(dtmP,*polygonPP,spnt,polyNumber)) goto errexit ;

                         //  Store Tptr Polygon In Tin Object As DTMFeatureType::Polygon

                         if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Polygon,polyNumber,dtmP->nullFeatureId,spnt,0)) goto errexit ; 

                         //  If Necessary Create Tag Object

                         if( *tagPP == NULL ) 
                           {
                            if( bcdtmPolygon_createTagObject(tagPP)) goto errexit ;
                           } 

                         //  Store Tag List In Tag Object 

                         if( bcdtmPolygon_storeTagListInTagObject(*tagPP,tagListP,tagNum,polyNumber,DTM_NULL_PNT,DTM_NULL_PNT,DTM_NULL_PNT)) goto errexit ;

                         //  Write Polygon Tags For Development Purposes

/*
                         bcdtmPolygon_writePolygonTag(TagList,tagNum) ;
*/

                         //  Increment Polygon Number

                         ++polyNumber ;

                         //  Mark Tptr Polygon 

                         cp = spnt ;
                         do
                           {
                            nodeAddrP(dtmP,cp)->PRGN = 1 ;
                            if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs,cp,nodeAddrP(dtmP,cp)->tPtr) ) goto errexit ;
                            bcdtmFlag_setFlag(lineMarkP,ofs) ;
                            cp = nodeAddrP(dtmP,cp)->tPtr ;
                           } while ( cp != spnt ) ;
                        }

                      //  Null Out Tptr Polygon
         
                      bcdtmList_nullTptrListDtmObject(dtmP,spnt) ;
                     }
                  }
               }
            }
         }
      }
   } 
/*
** Free memory
*/
 free(lineMarkP) ;
 lineMarkP = NULL ;
 if( tagListP != NULL ) free(tagListP) ;

// Scan Points For Non Intersecting Polygons

 for( spnt = 0 ; spnt < dtmP->numPoints ; ++spnt )
   {
    if( nodeAddrP(dtmP,spnt)->cPtr != dtmP->nullPtr && ! nodeAddrP(dtmP,spnt)->PRGN ) 
      {

       // Get Prior Point

       clc = nodeAddrP(dtmP,spnt)->cPtr ;
       while ( clc != dtmP->nullPtr )
         {
          pp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;   

          // Check If Prior Point Points To Current Point If So Extract Polygon

          cln = nodeAddrP(dtmP,pp)->fPtr ;
          while( cln != dtmP->nullPtr )
            {
             bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,cln)->dtmFeature,pp,&np) ;
             cln = flistAddrP(dtmP,cln)->nextPtr ;

             // Extract Polygon

             if( np == spnt )
               {

                // Initialise For Scan

                np = pp ; 
                cp = spnt ;
                do
                  {

                   // Scan Clockwise To Next Feature lineMarkP

                   if(( np = bcdtmList_nextClkDtmObject(dtmP,cp,np)) < 0 ) goto errexit ;
                   while ( ! bcdtmList_testForLineOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,np,cp) )
                     {
                      if(( np = bcdtmList_nextClkDtmObject(dtmP,cp,np)) < 0 ) goto errexit ;
                     }

                   //  Set Tptr List

                   nodeAddrP(dtmP,cp)->tPtr = np ;

                   //  Initialise For Next Point

                   pp = np ; np = cp ; cp = pp ;
                  } while ( cp != spnt ) ;

                //  Get Polygon direction And If Anti Clockwise Store Polygon In Polygon Object

                if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,spnt,&area,&direction) ) { free(lineMarkP) ;  goto errexit ; }
                if (direction == DTMDirection::AntiClockwise)
                  {

                   //  If Necessary Create Polygon Object

                   if( *polygonPP == NULL ) if( bcdtmPolygon_createPolygonObject(polygonPP)) goto errexit ;

                   //  Get Tag Value

                   tag = (long)(ftableAddrP(dtmP,flistAddrP(dtmP,nodeAddrP(dtmP,spnt)->fPtr)->dtmFeature)->dtmUserTag) ;

                   //  Copy Internal Tptr Polygon To Polygon Object

                   if( bcdtmPolygon_storeTptrPolygonInPolygonObjectDtmObject(dtmP,*polygonPP,spnt,polyNumber)) goto errexit ;
                   //  Store Tptr Polygon In Tin Object As DTMFeatureType::Polygon

                   if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Polygon,polyNumber,dtmP->nullFeatureId,spnt,0)) goto errexit ; 

                   //  If Necessary Create Tag Object

                   if( *tagPP == NULL ) if( bcdtmPolygon_createTagObject(tagPP)) goto errexit ;

                   //  Store Tag List In Tag Object 

                   if( bcdtmPolygon_storeTagListInTagObject(*tagPP,&tag,1,polyNumber,DTM_NULL_PNT,DTM_NULL_PNT,DTM_NULL_PNT)) goto errexit ;

                   //  Increment Polygon Number

                   ++polyNumber ;

                   //  Mark Tptr Polygon 

                   cp = spnt ;
                   do
                     {
                      nodeAddrP(dtmP,cp)->PRGN = 1 ;
                      cp = nodeAddrP(dtmP,cp)->tPtr ;
                     } while ( cp != spnt ) ;
                  }

                //  Null Out Tptr Polygon
         
                bcdtmList_nullTptrListDtmObject(dtmP,spnt) ;
               }
            } 
         }
      }
   } 

// Remove All Break Line Features

 if( bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline) ) goto errexit ;

// Mark All Points Internal To Tin Polygonal Features

 for( node = 0 ; node < dtmP->numPoints ; ++node )
   {
    nodeP = nodeAddrP(dtmP,node) ;
    nodeP->tPtr = dtmP->nullPnt ;
    nodeP->PRGN = 0 ;
   }
 for( node = 0 ; node < dtmP->numPoints ; ++node )
   {
    nodeP = nodeAddrP(dtmP,node) ;
    if( nodeP->cPtr != dtmP->nullPtr && nodeP->PRGN == 0 )
      {
       if( bcdtmList_countNumberOfDtmFeaturesForPointDtmObject(dtmP,node,&numFeatures)) goto errexit ;
       if( numFeatures > 0 ) 
         {
          cln = nodeP->fPtr ;
          while ( cln != dtmP->nullPtr )
            {
             dtmFeature  = flistAddrP(dtmP,cln)->dtmFeature ;
             dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
             if( bcdtmMark_internalPolygonPointsDtmObject(dtmP,dtmFeature,(long)dtmFeatureP->dtmUserTag,&numMarked )) goto errexit ; 
             if( bcdtmMark_prgnControlWordForFeatureDtmObject(dtmP,dtmFeature,1,&numMarked )) goto errexit ; 
             cln = flistAddrP(dtmP,cln)->nextPtr ;
            } 
         }
      }
   } 

// Determine Polygons Internal To Other Polygons

 for( ppl = (*polygonPP)->polyListP ; ppl < (*polygonPP)->polyListP + (*polygonPP)->numPolygons ; ++ppl) ppl->s1 = dtmP->nullPnt ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      { 
       if( nodeAddrP(dtmP,dtmFeatureP->dtmFeaturePts.firstPoint)->tPtr != dtmP->nullPnt )
         {
          ((*polygonPP)->polyListP+dtmFeatureP->dtmUserTag)->s1 = nodeAddrP(dtmP,dtmFeatureP->dtmFeaturePts.firstPoint)->tPtr ;
         }
      } 
   } 

// Clean Up

 cleanup :
 if( lineMarkP != NULL ) free(lineMarkP) ;
 if( tagListP  != NULL ) free(tagListP) ;

// Job Completed

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
BENTLEYDTM_Public int bcdtmPolygon_countNumberOfNonDtmPolygonFeaturesForPointDtmObject
(
 BC_DTM_OBJ  *dtmP,
 long        point,
 long        *numFeaturesP
)
/*
** This Function Counts The Number Of Dtm Features For A Point
*/
{
 long clc ;
/*
** Count Features
*/
 *numFeaturesP = 0 ;
 clc = nodeAddrP(dtmP,point)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType != DTMFeatureType::Polygon) ++(*numFeaturesP) ;
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
BENTLEYDTM_Public int bcdtmPolygon_createTagObject(TAGOBJ **tagPP)
/*
** This Function Creates a Tag Object and
** Returns a Pointer To The Object
*/
{
 int ret=DTM_SUCCESS ;

// Delete Tag Object If It Exists

 if( *tagPP != NULL )
   {
    if( bcdtmPolygon_deleteTagObject(tagPP)) goto errexit ;
   }

// Create Tag Object

 *tagPP = ( TAGOBJ * ) malloc ( sizeof(TAGOBJ)) ;
 if( *tagPP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
   
// Clean Up

 cleanup :

// Return
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
BENTLEYDTM_Public int bcdtmPolygon_deleteTagObject(TAGOBJ **tagPP)
/*
** This Function Deletes a Tag Object
*/
{
 int ret=DTM_SUCCESS ;

// Check For None Null Tag Object

 if( *tagPP != NULL ) 
   {
    if( bcdtmPolygon_freeMemoryTagObject(*tagPP)) goto errexit  ;
    free(*tagPP) ;
    *tagPP = NULL ;
   } 
   
// Clean Up

 cleanup :

// Return
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
BENTLEYDTM_Public int bcdtmPolygon_freeMemoryTagObject(TAGOBJ *tagP)
/*
** This Function Frees The Memory For A Taggon Object
*/
{
/*
** Free memory
*/
 if( tagP->PTAG != NULL ) { free(tagP->PTAG) ; tagP->PTAG = NULL ; } 
 if( tagP->PVAL != NULL ) { free(tagP->PVAL) ; tagP->PVAL = NULL ; } 
/*
** Reset Taggon Object Variables
*/
 tagP->NTAG = tagP->MTAG = 0 ;
 tagP->NVAL = tagP->MVAL = 0 ;
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
BENTLEYDTM_Public int bcdtmPolygon_storeTagListInTagObject
(
 TAGOBJ *tagP,
 long   *tagListP,
 long   numTagValues,
 long   utag1,
 long   utag2,
 long   utag3,
 long   utag4
)
/*
** This Function Stores a TagList In A Tag Object
*/
{
 int   ret=DTM_SUCCESS ;
 long  *pt ;
/*
** Store Header
*/
 if( tagP->NTAG == tagP->MTAG ) 
   { 
    if( bcdtmPolygon_allocateMemoryTagObject(tagP)) goto errexit ; 
   }
 (tagP->PTAG + tagP->NTAG)->FTAG = tagP->NVAL ;
 (tagP->PTAG + tagP->NTAG)->LTAG = tagP->NVAL + numTagValues - 1 ;
 (tagP->PTAG + tagP->NTAG)->UTAG[0] = utag1  ;
 (tagP->PTAG + tagP->NTAG)->UTAG[1] = utag2  ;
 (tagP->PTAG + tagP->NTAG)->UTAG[2] = utag3  ;
 (tagP->PTAG + tagP->NTAG)->UTAG[3] = utag4  ;
 ++(tagP->NTAG) ;
/*
** Store Tag Values
*/
 for( pt = tagListP ; pt < tagListP + numTagValues ; ++pt )
   {
    if( tagP->NVAL == tagP->MVAL ) 
      { 
       if( bcdtmPolygon_allocateMemoryTagObject(tagP)) goto errexit ; 
      }
    *(tagP->PVAL + tagP->NVAL) = *pt ;
    ++(tagP->NVAL) ;
   }
   
// Clean Up

 cleanup :

// Return
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
BENTLEYDTM_Public int bcdtmPolygon_allocateMemoryTagObject(TAGOBJ *Tag)
/*
** This Routine Gets the initial memory for the input data
*/
{
 int ret=DTM_SUCCESS ;
 TAGLIST  *pl ;
 long     *pd ;
/*
** Allocate Memory
*/
 if( Tag->MVAL != 0 ) { if( bcdtmPolygon_incrementMemoryTagObject(Tag)) goto errexit ; }
 else
   {
    Tag->MTAG = Tag->SMTAG ;
    Tag->MVAL = Tag->SMVAL ;
    Tag->PTAG  = ( TAGLIST * ) malloc ( Tag->MTAG * sizeof(TAGLIST)) ;
    Tag->PVAL  = ( long    * ) malloc ( Tag->MVAL * sizeof(long)) ;
    if( Tag->PTAG == NULL || Tag->PVAL == NULL )
      {
       if( Tag->PTAG != NULL ) { free(Tag->PTAG) ; Tag->PTAG = NULL ; }
       if( Tag->PVAL != NULL ) { free(Tag->PVAL) ; Tag->PVAL = NULL ; }
       Tag->MTAG = Tag->MVAL = 0 ; 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
** Initialise Data Values
*/
    for( pl = Tag->PTAG ; pl < Tag->PTAG + Tag->MTAG ; ++pl ) 
      { 
       pl->FTAG = pl->LTAG = DTM_NULL_PNT ;
       pl->UTAG[0] = pl->UTAG[1] = pl->UTAG[2] = pl->UTAG[3] = DTM_NULL_PNT ; 
      }
    for( pd = Tag->PVAL ; pd < Tag->PVAL + Tag->MVAL ; ++pd ) *pd = 0 ;
   }
   
// Clean Up

 cleanup :

// Return
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
BENTLEYDTM_Public int bcdtmPolygon_incrementMemoryTagObject(TAGOBJ *Tag)
/*
** This Routine Increments the Memory for A Taggon Object
*/
{
 int ret=DTM_SUCCESS ;
 TAGLIST  *pl ;
 long     *pd ;
/*
** Allocate Memory For Tag Header List
*/
 if( Tag->NTAG == Tag->MTAG )
   {
    Tag->MTAG = Tag->MTAG + Tag->IMTAG ;
    Tag->PTAG = ( TAGLIST * ) realloc(Tag->PTAG,Tag->MTAG * sizeof(TAGLIST)) ;
    if( Tag->PTAG == NULL )
      {
       if( Tag->PVAL != NULL ) { free(Tag->PVAL) ; Tag->PVAL = NULL ; }
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      } 
    for( pl = Tag->PTAG + Tag->NTAG ; pl < Tag->PTAG + Tag->MTAG ; ++pl ) 
      { 
       pl->FTAG = pl->LTAG = DTM_NULL_PNT ;
       pl->UTAG[0] = pl->UTAG[1] = pl->UTAG[2] = pl->UTAG[3] = DTM_NULL_PNT ; 
      }
   }
/*
** Allocate Memory For Tag Values
*/
 if( Tag->NVAL == Tag->MVAL )
   {
    Tag->MVAL = Tag->MVAL + Tag->IMVAL ;
    Tag->PVAL = (long * ) realloc( Tag->PVAL,Tag->MVAL * sizeof(long)) ;
    if( Tag->PVAL == NULL )
      {
       if( Tag->PTAG != NULL ) { free(Tag->PTAG) ; Tag->PTAG = NULL ; }
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      } 
    for( pd = Tag->PVAL + Tag->NVAL ; pd < Tag->PVAL + Tag->MVAL  ; ++pd ) *pd = 0 ;
   }

// Clean Up

 cleanup :

// Return
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
BENTLEYDTM_Public int bcdtmPolygon_storePolygonTag(long **TagList,long TagFlag,long TagValue,long *Tagne,long *Tagme,long Tagminc) 
/*
** This Function Stores A Polygon Tag In A Tag List
*/
{
 long *pl ;
/*
** Test For Memory Allocation
*/
 if( *TagList == NULL || *Tagne == *Tagme )
   {
    if( *TagList == NULL ) *Tagne = *Tagme = 0 ;
    *Tagme = *Tagme + Tagminc ;
    if( *TagList == NULL ) *TagList = (long *) malloc(*Tagme * sizeof(long)) ;
    else                   *TagList = (long *) realloc(*TagList,*Tagme * sizeof(long)) ;
    if( *TagList == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return(1) ; }
   }
/*
** If TagFlag == 1 Scan for Previously Stored Tag Values
*/
 if( TagFlag == 1 )
   {
    for( pl = *TagList ; pl < *TagList + *Tagne ; ++pl ) if( *pl == TagValue ) return(0) ; 
   }
/*
** Store Tag value
*/
 *(*TagList + *Tagne) = TagValue ;
 ++(*Tagne) ;
/*
** Job Completed
*/
 return(0) ;
}
