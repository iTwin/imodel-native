/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmClean.cpp $
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
BENTLEYDTM_Public int bcdtmClean_setDtmPolygonalFeatureAntiClockwiseDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       dtmFeature
) 
/*
** This Function Sets The Polygonal DTM Feature Anti Clockwise
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    numPolyPts,firstPoint ;
 DTMDirection direction;
 double  area ;
 DPoint3d     *p3dP,*polyPtsP=NULL ;
 DPoint3d  *pointP ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Setting DTM Polygonal Feature AntiClockwise") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeature = %8ld",dtmFeature) ;
   }
/*
** Check For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Untriangulated DTM
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   }
/*
** Get Feature Points 
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ; 
 if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && bcdtmData_testForValidPolygonalDtmFeatureType( dtmFeatureP->dtmFeatureType) )
   {
    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&polyPtsP,&numPolyPts)) goto errexit ;
/*
**  Check Polygon Closes
*/
    if( polyPtsP->x == (polyPtsP+numPolyPts-1)->x && polyPtsP->y == (polyPtsP+numPolyPts-1)->y )
      {
/*
**     Check Polygon Direction
*/
       if( bcdtmMath_getPolygonDirectionP3D(polyPtsP,numPolyPts,&direction,&area)) goto errexit ; 
       if (dbg && direction == DTMDirection::Clockwise) bcdtmWrite_message (0, 0, 0, "Polygon direction Clockwise");
       if (dbg && direction == DTMDirection::AntiClockwise) bcdtmWrite_message (0, 0, 0, "Polygon direction Anti Clockwise");
/*
**     If direction Clockwise - Reverse direction
*/
       if (direction == DTMDirection::Clockwise)
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Setting Polygon direction Anti Clockwise") ;
          firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
          for( p3dP = polyPtsP + numPolyPts - 1 ; p3dP > polyPtsP ; --p3dP )
            {
             pointP = pointAddrP(dtmP,firstPoint) ;
             pointP->x = p3dP->x ;
             pointP->y = p3dP->y ;
             pointP->z = p3dP->z ;
             ++firstPoint ;
            }
         }
      }
/*
**  Free memory
*/
    if( polyPtsP != NULL ) { free(polyPtsP) ;  polyPtsP = NULL ; }
   } 
/*
** Clean Up
*/
 cleanup :
 if( polyPtsP != NULL ) free(polyPtsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Setting DTM Polygonal Feature AntiClockwise Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Setting DTM Polygonal Feature AntiClockwise Error") ;
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
BENTLEYDTM_Public int bcdtmClean_setDtmPolygonalFeatureTypeAntiClockwiseDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureType dtmFeatureType
) 
/*
** This Function Sets All Polygon DTM Features Types Anti Clockwise
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    dtmFeature,numPolygons,numPolyPts,firstPoint ;
 DTMDirection direction;
 double  area ;
 DPoint3d     *p3dP,*polyPtsP=NULL ;
 DPoint3d  *pointP ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Check For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Untriangulated DTM
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   }
/*
** Check For A DTM Polygonal Feature Type
*/
 if( ! bcdtmData_testForValidPolygonalDtmFeatureType(dtmFeatureType) )
   {
    bcdtmWrite_message(0,0,0,"Not A Polygonal Dtm Feature Type") ;
    goto errexit ; 
   } 
/*
** Scan DTM For Polygonal Dtm Features 
*/
 numPolygons = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ; 
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == dtmFeatureType )
      {
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&polyPtsP,&numPolyPts)) goto errexit ;
/*
**     Check Polygon Closes
*/
       if( polyPtsP->x == (polyPtsP+numPolyPts-1)->x && polyPtsP->y == (polyPtsP+numPolyPts-1)->y )
         {
          ++numPolygons ;
/*
**        Check Polygon Direction
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Checking direction Of Polygonal Feature %4ld",numPolygons) ;   
          if( bcdtmMath_getPolygonDirectionP3D(polyPtsP,numPolyPts,&direction,&area)) goto errexit ; 
          if (dbg && direction == DTMDirection::Clockwise) bcdtmWrite_message (0, 0, 0, "Polygon direction Clockwise");
          if (dbg && direction == DTMDirection::AntiClockwise) bcdtmWrite_message (0, 0, 0, "Polygon direction Anti Clockwise");
/*
**        If direction Clockwise - Reverse direction
*/
          if (direction == DTMDirection::Clockwise)
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Setting Polygon direction Anti Clockwise") ;
             firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
             for( p3dP = polyPtsP + numPolyPts - 1 ; p3dP > polyPtsP ; --p3dP )
               {
                pointP = pointAddrP(dtmP,firstPoint) ;
                pointP->x = p3dP->x ;
                pointP->y = p3dP->y ;
                pointP->z = p3dP->z ;
                ++firstPoint ;
               }
            }
         }
/*
**     Free memory
*/
       if( polyPtsP != NULL ) { free(polyPtsP) ;  polyPtsP = NULL ; }
      }
   } 
/*
** Write Number Of Polygons ** Developement Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Polygons Detected = %4ld",numPolygons) ;
/*
** Clean Up
*/
 cleanup :
 if( polyPtsP != NULL ) free(polyPtsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Setting Polygons AntiClockwise Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Setting Polygons AntiClockwise Error") ;
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
BENTLEYDTM_Public int bcdtmClean_polygoniseTinLinesDtmObject(BC_DTM_OBJ *dtmP,long *tinLinesP)
/*
** Assumes All Tin Lines Have Been Directionally Marked In A Clocwise Direction
** For Each Triangle The Line Forms Part Of
** 
** This Function Polygonises The Tin Lines In The Tin Object Form Their Assigned Values 
** And Store The Polygons In The Tin As DTMFeatureType::Polygon with a Usertag Value For The Line Value
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,sp,np,fp,lp,clc,cv,nv=0,ofs1,ofs2;
 DTMDirection direction ;
 double area ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Tin Lines") ;
/*
** Write DTM To File
*/
 if( dbg == 2  ) bcdtmWrite_toFileDtmObject(dtmP,L"tinLines.dtm") ;
/*
** Scan Tin Lines And Extract Polygons 
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    clc = nodeAddrP(dtmP,p1)->cPtr ;
    while ( clc != dtmP->nullPtr )
      {
       p2  = clistAddrP(dtmP,clc)->pntNum ; 
       clc = clistAddrP(dtmP,clc)->nextPtr ; 
       if( p2 > p1 )
         {
/*
**        Test For Polygon Edge Line
*/
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p1,p2)) goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p2,p1)) goto errexit ;
          if( *(tinLinesP+ofs1) != nv && *(tinLinesP+ofs1) != *(tinLinesP+ofs2) )
            {
             cv = *(tinLinesP+ofs1) ;
             *(tinLinesP+ofs1) = nv ;
             sp = p2 ;
             np = p1 ;
             fp = sp ;
/*
**           Set Tptr Polygon Around Feature
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Setting Tptr Polygon Around Cv = %4ld",cv) ;
             nodeAddrP(dtmP,sp)->tPtr = np ;
             nodeAddrP(dtmP,np)->tPtr = dtmP->nullPnt ;
             do
               {
                if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,fp) ;
                lp = sp ;
                if( ( lp = bcdtmList_nextClkDtmObject(dtmP,np,lp)) < 0 ) goto errexit ; 
                if( dbg == 2 )
                  {
                   bcdtmWrite_message(0,0,0,"sp = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                   bcdtmWrite_message(0,0,0,"np = %6ld ** %10.4lf %10.4lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
                   bcdtmWrite_message(0,0,0,"lp = %6ld  hPtr = %9ld  ** %10.4lf %10.4lf %10.4lf",lp,nodeAddrP(dtmP,lp)->hPtr,pointAddrP(dtmP,lp)->x,pointAddrP(dtmP,lp)->y,pointAddrP(dtmP,lp)->z) ;
                  }
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,np,lp)) goto errexit ; 
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,lp,np)) goto errexit ; 
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"ofs1 = %6ld ofs2 = %6ld",*(tinLinesP+ofs1),*(tinLinesP+ofs2)) ; 
                while( *(tinLinesP+ofs1) == cv && *(tinLinesP+ofs2) == cv )
                  {
                   if( ( lp = bcdtmList_nextClkDtmObject(dtmP,np,lp)) < 0 ) goto errexit ; 
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,np,lp)) goto errexit ; 
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,lp,np)) goto errexit ; 
                   if( dbg == 2 )
                     {
                      bcdtmWrite_message(0,0,0,"lp = %6ld hPtr = %9ld ** %10.4lf %10.4lf %10.4lf",lp,nodeAddrP(dtmP,lp)->hPtr,pointAddrP(dtmP,lp)->x,pointAddrP(dtmP,lp)->y,pointAddrP(dtmP,lp)->z) ;
                      bcdtmWrite_message(0,0,0,"ofs1 = %6ld ofs2 = %6ld",*(tinLinesP+ofs1),*(tinLinesP+ofs2)) ; 
                     }
                  }
                *(tinLinesP+ofs2) = nv ;
                sp = np ;
                np = lp ;
                nodeAddrP(dtmP,sp)->tPtr = np ;
               } while ( sp != fp ) ;
/*
**           Write Out Tptr List
*/
             if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,fp) ;
/*
**           Check Connectivity Of Tptr List
*/
             if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,fp,0)) 
               {
                bcdtmList_writeTptrListDtmObject(dtmP,fp) ;
                goto errexit ;
               } 
/*
**           Check Polygon Direction
*/
             if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,fp,&area,&direction)) goto errexit ;
/*
**           Store Feature In Tin Object Only If Direction Is Anti Clockwise
*/
             if (direction == DTMDirection::AntiClockwise)
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Polygon IN Tin ** cv = %4ld",cv) ;
                if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Polygon,cv,dtmP->nullFeatureId,fp,1)) goto errexit ;
               }
/*
**           Null Out Tptr List
*/           
             if( nodeAddrP(dtmP,fp)->tPtr != dtmP->nullPnt ) if( bcdtmList_nullTptrListDtmObject(dtmP,fp)) goto errexit ; 
            }  
         }
      }
   }
/*
** Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Polygonising Tin Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Polygonising Tin Lines Error") ;
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
BENTLEYDTM_EXPORT int bcdtmClean_validateDtmFeatureTypeDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureType dtmFeatureType,
 double     ppTol,
 long       *numErrorsP
)
/*
** This Function Validates Dtm Features In A Dtm Object
** Features With Errors Are Converted To Graphic Breaks
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs1,ofs2,ofs3,point,dtmFeature,numFeaturePts=0,closeFlag; 
 long   numFeatures=0,validateResult,polygonalFeature=FALSE ;
 DPoint3d    *p3dP,*featurePtsP=NULL ; 
 char   dtmFeatureName[256] ;
 DPoint3d  *pointP,*point1P,*point2P ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Validating Dtm Feature Type") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureName) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType = %s",dtmFeatureName) ;
   }
/*
** Initialise
*/
 *numErrorsP = 0 ;
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ; 
/*
** Check DTM Is In Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulted DTM") ;
    goto errexit ;
   }
/*
** Scan Features For Dtm Feature Type
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**  Only Process Data Features
*/
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == dtmFeatureType )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Validating DTM Feature[%8ld] ** Number Of Feature Points = %6ld",dtmFeature,dtmFeatureP->numDtmFeaturePts) ;
       ++numFeatures ;
/*
**     Check For Polygonal Feature
*/
       polygonalFeature = FALSE ;
       if     ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void       ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole       ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island     ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull       ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Polygon    ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Region     ) polygonalFeature = TRUE ;
/*
**     Allocate Memory For Feature Points
*/
       numFeaturePts = dtmFeatureP->numDtmFeaturePts ;
       featurePtsP = ( DPoint3d * ) malloc ( numFeaturePts * sizeof(DPoint3d)) ;
       if( featurePtsP == NULL ) 
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
/*
**     Copy Feature Points To Point Array
*/
       ofs1 = dtmFeatureP->dtmFeaturePts.firstPoint ;
       ofs2 = ofs1 + dtmFeatureP->numDtmFeaturePts - 1 ; 
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Feature Start Offset = %6ld Feature End Offset = %6ld",ofs1,ofs2) ;
       for( point = ofs1 , p3dP = featurePtsP ; point <= ofs2 ; ++point , ++p3dP ) 
         { 
          pointP = pointAddrP(dtmP,point) ;
          p3dP->x = pointP->x ; 
          p3dP->y = pointP->y ; 
          p3dP->z = pointP->z ; 
         }
/*
**     Check For Closure
*/
       closeFlag = 0 ;
       if( featurePtsP->x == (featurePtsP+numFeaturePts-1)->x && featurePtsP->y == (featurePtsP+numFeaturePts-1)->y ) closeFlag = 1 ;
       if( dbg == 2 && ! closeFlag ) bcdtmWrite_message(0,0,0,"Open Feature") ;
       if( dbg == 2 &&   closeFlag ) bcdtmWrite_message(0,0,0,"Closed Feature") ;
/*
**     Check For Closure Within Point To Point Tolerance
*/
       if( bcdtmMath_distance(featurePtsP->x,featurePtsP->y,(featurePtsP+numFeaturePts-1)->x,(featurePtsP+numFeaturePts-1)->y) <= ppTol )
         {
          featurePtsP->x = (featurePtsP+numFeaturePts-1)->x ;
          featurePtsP->y = (featurePtsP+numFeaturePts-1)->y ;
          featurePtsP->z = (featurePtsP+numFeaturePts-1)->z ;
          closeFlag = 1 ;
         } 
/*
**     Close Polygonal Features
*/
       if( polygonalFeature == TRUE && ! closeFlag )
         {
          featurePtsP->x = (featurePtsP+numFeaturePts-1)->x ;
          featurePtsP->y = (featurePtsP+numFeaturePts-1)->y ;
          featurePtsP->z = (featurePtsP+numFeaturePts-1)->z ;
          closeFlag = 1 ;
         }
/*
**     Validate Feature Points
*/ 
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Of Points Before Validate = %6ld",numFeaturePts) ;
       if( ! closeFlag ) validateResult = bcdtmClean_validateStringP3D(&featurePtsP,&numFeaturePts,ppTol) ;
       else              validateResult = bcdtmClean_validatePointArrayPolygon(&featurePtsP,&numFeaturePts,0,ppTol) ;
       if( validateResult )
         {
          ++*numErrorsP ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Validation Errors In Feature") ; 
         }
/*
**     If No Validation Errors Store Validated Points 
*/
       else
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Of Points  After Validate = %6ld",numFeaturePts) ;
          for( point = ofs1 , p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++point , ++p3dP ) 
            { 
             pointP = pointAddrP(dtmP,point) ;
             pointP->x = p3dP->x ; 
             pointP->y = p3dP->y ; 
             pointP->z = p3dP->z ; 
            }
/*
**        Copy Over Deleted Points
*/
          if( numFeaturePts < dtmFeatureP->numDtmFeaturePts )
            {
             ofs3 = dtmFeatureP->numDtmFeaturePts - numFeaturePts ;
             dtmFeatureP->numDtmFeaturePts = numFeaturePts ;
             ofs1 = ofs1 + numFeaturePts ;
             ++ofs2 ;
             while( ofs2 < dtmP->numPoints ) 
               { 
                point1P = pointAddrP(dtmP,ofs1) ;
                point2P = pointAddrP(dtmP,ofs2) ;
                *point1P = *point2P ;
                ++ofs1 ;
                ++ofs2 ; 
               } 
             dtmP->numPoints = ofs1 ;
/*
**           Adjust First Point Offset For Remaing Features
*/
             for( ofs1 = dtmFeature + 1 ; ofs1 < dtmP->numFeatures ; ++ofs1 )
               {
                dtmFeatureP = ftableAddrP(dtmP,ofs1) ;
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data ) dtmFeatureP->dtmFeaturePts.firstPoint -= ofs3 ;
               }
            }
/*
**        Set Polygonal Feature Anti Clockwise
*/
          if( polygonalFeature == TRUE )
            {
             if( bcdtmClean_setDtmPolygonalFeatureAntiClockwiseDtmObject(dtmP,dtmFeature)) goto errexit ;
            }
         } 
/*
**     Free Feature Points Memory
*/
       free(featurePtsP) ; featurePtsP = NULL ;
      }
   }
/*
** Log Number Of Errors
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number With Errors = %8ld of %8ld",*numErrorsP,numFeatures) ;
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP  != NULL ) { free(featurePtsP)  ; featurePtsP = NULL  ; }
/*
**  Job Completed
*/
 if( dbg && ! ret) bcdtmWrite_message(0,0,0,"Validating DTM Feature Type Completed") ;
 if( dbg &&   ret) bcdtmWrite_message(0,0,0,"Validating DTM Feature Type Error") ;
 return(ret) ;
/*
** Error Return
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
BENTLEYDTM_Public int bcdtmClean_retriangualteBreakLinesDtmObject(BC_DTM_OBJ *dtmP)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  dtmFeature,startPnt,numSwapped,totNumSwapped=0 ;
 BC_DTM_FEATURE *featP ;
/*
** Scan Feature List For Break Lines
*/
 for( dtmFeature = 0  ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    featP = ftableAddrP(dtmP,dtmFeature) ;
    if( featP->dtmFeatureState == DTMFeatureState::Tin && featP->dtmFeatureType == DTMFeatureType::Breakline )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Retriangulating Break Line %6ld",dtmFeature) ;
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&startPnt) ) goto errexit  ;
       if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,startPnt) ; 
       if( bcdtmClean_retriangualteAlongBreakLineDtmObject(dtmP,startPnt,&numSwapped)) goto errexit  ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Triangles Swapped = %6ld",numSwapped) ;
       totNumSwapped = totNumSwapped + numSwapped ;
       if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit  ;
      } 
   }
/*
** Write Total Triangles Swapped
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Total Number Of Triangles Swapped = %6ld",totNumSwapped) ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmClean_retriangualteAlongBreakLineDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long *numSwappedP)
/*
** This Function Retriangulates Along The Tptr List
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sp,ap,cp,ncp,clPtr,loop,onLine,numSwapped ;
 double aD,cD,nD,nX,nY ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Along Break Line ** startPnt = %10ld",startPnt) ;
/*
** Initialise
*/
 *numSwappedP = 0 ;
/*
** Scan Tptr List And Check MAX_MIN Criteria
*/
 if( startPnt >= 0 && startPnt < dtmP->numPoints ) 
   {
    if( nodeAddrP(dtmP,startPnt)->tPtr != dtmP->nullPnt ) 
      {
/*
**     Loop For A Maximum Of 5 Times
*/
       loop = 1 ;
       numSwapped = 1 ;
       while ( numSwapped && loop )
         {
          --loop ;
          numSwapped = 0 ;
          sp = startPnt ;
/*
**        Scan Along Tptr List
*/
          do
            {
/*
**           Scan Triangles Around Sp
*/
             clPtr = nodeAddrP(dtmP,sp)->cPtr ;
             if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
             while( clPtr != dtmP->nullPtr )
               {
                cp    = clistAddrP(dtmP,clPtr)->pntNum ; 
                clPtr = clistAddrP(dtmP,clPtr)->nextPtr ; 
                if( dbg )
                  {
                   bcdtmWrite_message(0,0,0,"sp = %6ld  ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z ) ;
                   bcdtmWrite_message(0,0,0,"ap = %6ld  ** %10.4lf %10.4lf %10.4lf",ap,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,ap)->z ) ;
                   bcdtmWrite_message(0,0,0,"cp = %6ld  ** %10.4lf %10.4lf %10.4lf",cp,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y,pointAddrP(dtmP,cp)->z ) ;
                  }
/*
**              Check For Swap Triangle
*/
                if( nodeAddrP(dtmP,sp)->hPtr != ap && nodeAddrP(dtmP,cp)->hPtr != ap )
                  {
                   if(   bcdtmList_testForBreakLineDtmObject(dtmP,sp,ap) &&
                         bcdtmList_testForBreakLineDtmObject(dtmP,sp,cp) &&
                       ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,cp,ap)     )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"Swap Triangle Found") ;
                      if( ( ncp = bcdtmList_nextClkDtmObject(dtmP,cp,ap)) < 0 ) goto errexit ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"ncp = %6ld  ** %10.4lf %10.4lf %10.4lf",ncp,pointAddrP(dtmP,ncp)->x,pointAddrP(dtmP,ncp)->y,pointAddrP(dtmP,ncp)->z ) ;
/*
**                    Check If ap-cp Can be swapped with sp-ncp
*/
                      if( bcdtmMath_pointSideOfDtmObject(dtmP,sp,ncp,ap) > 0  &&
                          bcdtmMath_pointSideOfDtmObject(dtmP,sp,ncp,cp) < 0      )
                        { 
                         if( dbg ) bcdtmWrite_message(0,0,0,"Swap Triangle Can Be Swapped") ;
/*
**                       Check For Swap On Basis Of Max Min Criteria
*/
                         if( bcdtmTin_maxMinTestDtmObject(dtmP,sp,ncp,ap,cp) )
                           {
                            if( dbg ) bcdtmWrite_message(0,0,0,"Triangle Swapped On MaxMin Test") ;
                            if( bcdtmList_deleteLineDtmObject(dtmP,ap,cp)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,sp,ncp,ap)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ncp,sp,cp)) goto errexit ;
                            ++numSwapped ;
                           }
/*
**                       Check Point To Line 
*/
                         else
                           {
                            nD = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,&nX,&nY) ;
                            if( dbg ) bcdtmWrite_message(0,0,0,"nD = %20.15lf",nD) ;
                            if( nD < dtmP->plTol && onLine)
                              {
                               if( dbg ) bcdtmWrite_message(0,0,0,"Triangle Swapped On P2L Test") ;
                               if( bcdtmList_deleteLineDtmObject(dtmP,ap,cp)) goto errexit ;
                               if( bcdtmList_insertLineAfterPointDtmObject(dtmP,sp,ncp,ap)) goto errexit ;
                               if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ncp,sp,cp)) goto errexit ;
                               ++numSwapped ;
                              }
                            else
                              {
                               aD = bcdtmMath_distance(nX,nY,pointAddrP(dtmP,ap)->x,pointAddrP(dtmP,ap)->y) ;
                               cD = bcdtmMath_distance(nX,nY,pointAddrP(dtmP,cp)->x,pointAddrP(dtmP,cp)->y) ;
                               if( dbg ) 
                                 {
                                  bcdtmWrite_message(0,0,0,"aD = %20.15lf",aD) ;
                                  bcdtmWrite_message(0,0,0,"cD = %20.15lf",cD) ;
                                 }
                               if( nD < aD && nD < cD )
                                 {  
                                 if( dbg ) bcdtmWrite_message(0,0,0,"Triangle Swapped On Shortest Distance Test") ;
                                 if( bcdtmList_deleteLineDtmObject(dtmP,ap,cp)) goto errexit ;
                                 if( bcdtmList_insertLineAfterPointDtmObject(dtmP,sp,ncp,ap)) goto errexit ;
                                 if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ncp,sp,cp)) goto errexit ;
                                 ++numSwapped ;
                                }
                              }
                           }  
                        }
                     } 
                  }
/*
**              Set Up For Next Triangle
*/
                ap = cp ; 
               }
/*
**           Set Up For Next Break Line Segment
*/
             sp = nodeAddrP(dtmP,sp)->tPtr ;
            } while ( sp != startPnt && sp != dtmP->nullPnt ) ;
  
/*
**        Accumulate Number Of Swappings
*/
          *numSwappedP = *numSwappedP + numSwapped ; 
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
BENTLEYDTM_EXPORT int bcdtmClean_breakTopologyDtmObject(BC_DTM_OBJ *dtmP,double snapTolerance)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   numRemoved ; 
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Break Topology") ;
/*
** Remove Dangling Breaks
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dangling Breaks") ;
 if( bcdtmClean_removeDanglingBreaksDtmObject(dtmP,snapTolerance,&numRemoved)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Dangles Removed = %6ld",numRemoved) ; 
/*
** Remove Sliver Breaks
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Sliver Breaks") ;
 if( bcdtmClean_removeSliverBreaksDtmObject(dtmP,snapTolerance,&numRemoved)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Slivers Removed = %6ld",numRemoved) ; 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Break Topology Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Break Topology Error") ;
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
BENTLEYDTM_Private int bcdtmClean_removeDanglingBreaksDtmObject(BC_DTM_OBJ *dtmP,double snapTolerance,long *numRemovedP)                 
/*
**
** This Function Removes Internal Dangling Break Lines
** A Dangling Break Line Is One That Dosnt Have A Connecting
** Break Line At Either End
**
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long p1,p2,clPtr,numBreaks ;
/*
** Initialise
*/
 *numRemovedP = 0 ;
/*
** Scan Tin Points For Dangling Break Points 
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    bcdtmClean_countNumberOfDtmFeatureTypeForTinPointDtmObject(dtmP,p1,DTMFeatureType::Breakline,&numBreaks) ;
/*
**  If Point On Only One Break Line Check For Dangling Break Line
*/ 
    if( numBreaks == 1 )
      {
/*
**     Scan Point For Connecting Break Line Point
*/          
       p2 = dtmP->nullPnt ;
       clPtr = nodeAddrP(dtmP,p1)->cPtr ;
       while( clPtr != dtmP->nullPtr && p2 == dtmP->nullPnt )
         {
          if( bcdtmList_testForBreakLineDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) p2 = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
         } 
/*
**     Check If Break Line Is A Dangle
*/
       if( p2 != dtmP->nullPnt )
         {
          if( bcdtmMath_pointDistanceDtmObject(dtmP,p1,p2) <= snapTolerance )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dangling Break At Point %6ld ** %10.4lf %10.4lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
             if( bcdtmClean_removeBreakLineSegmentAtTinPointDtmObject(dtmP,p1)) goto errexit ;
             ++*numRemovedP ;
            }
         }
      } 
   }
/*
** Write Stats On Number Of Dangles Removed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Dangles Removed = %6ld",*numRemovedP) ;
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
BENTLEYDTM_Private int bcdtmClean_removeSliverBreaksDtmObject(BC_DTM_OBJ *dtmP,double snapTolerance,long *numRemovedP)                 
/*
**
** This Function Removes Internal Sliver Break Lines
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,sp1=0,sp2=0,sp3=0,sp4=0,clPtr ;
 long   onLine2,onLine3,numDetected,removeSliver ;
 double d2,d3,X2,Y2,X3,Y3 ;
/*
** Initialise
*/
 *numRemovedP = 0 ;
/*
** Initialise Sptr List
*/
 for( p1 = 0 ; p1 < dtmP->numPoints; ++p1 ) nodeAddrP(dtmP,p1)->sPtr = 0 ;
/*
** Scan Tin Points For Slivers 
*/
 for( p1 = 0 ; p1 < dtmP->numPoints; ++p1 )
   {
/*
**  Scan Tin Points For Consecutive Break Lines
*/    
    if( ( clPtr = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr )
      {
       if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while( clPtr != dtmP->nullPtr )
         {
          p3    = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
/*
**        Test For Consecutive Break Lines
*/
          if( bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2) &&
              bcdtmList_testForBreakLineDtmObject(dtmP,p1,p3)    )
            {
             d2 = bcdtmMath_distanceOfPointFromLine(&onLine2,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,&X2,&Y2) ;
             d3 = bcdtmMath_distanceOfPointFromLine(&onLine3,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,&X3,&Y3) ;
/*
**           Determine If Sliver Can Be Removed
*/
             removeSliver = 0 ;
             if(   onLine2 && ! onLine3 && d2 <= snapTolerance )
               {
                ++nodeAddrP(dtmP,p3)->sPtr ; 
                sp1 = p1 ;
                sp2 = p2 ;   
                sp3 = p3 ;
                if(( sp4 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ; 
                removeSliver = 1 ;
               }              
             if( ! onLine2 &&   onLine3 && d3 <= snapTolerance )
               {
                ++nodeAddrP(dtmP,p2)->sPtr ;    
                sp1 = p1 ;
                sp2 = p3 ;   
                sp3 = p2 ;
                if(( sp4 = bcdtmList_nextClkDtmObject(dtmP,p1,p3)) < 0 ) goto errexit ; 
                removeSliver = 2 ;
               }              
             if(  onLine2 &&   onLine3 && d2 <= snapTolerance && d3 <= snapTolerance )
               {
                bcdtmWrite_message(0,0,0,"Multiple Snap Detected") ;
               } 
/*
**          Remove Type1 Sliver
*/
             if( removeSliver == 1 )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Removing Type 1 Sliver") ;
                if( nodeAddrP(dtmP,sp3)->hPtr == dtmP->nullPnt )
                  {
                   if( bcdtmMath_pointSideOfDtmObject(dtmP,sp3,sp4,sp1) > 0 &&
                       bcdtmMath_pointSideOfDtmObject(dtmP,sp3,sp4,sp2) < 0     )
                     {
                      if( clPtr != dtmP->nullPtr ) clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                      if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,sp1,sp2,sp3)) goto errexit ; 
                      if( bcdtmList_deleteLineDtmObject(dtmP,sp1,sp2)) goto errexit ; 
                      if( bcdtmList_insertLineAfterPointDtmObject(dtmP,sp3,sp4,sp1)) goto errexit ; 
                      if( bcdtmList_insertLineAfterPointDtmObject(dtmP,sp4,sp3,sp2)) goto errexit ; 
                      ++*numRemovedP ;
                     }
                  } 
               }   
/*
**           Remove Type 2 Slivers
*/          
             if( removeSliver == 2 )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Removing Type 2 Sliver") ;
/*
**              Internal Point
*/
                if( nodeAddrP(dtmP,sp3)->hPtr == dtmP->nullPnt )
                  {
                   if( bcdtmMath_pointSideOfDtmObject(dtmP,sp3,sp4,sp1) < 0 &&
                       bcdtmMath_pointSideOfDtmObject(dtmP,sp3,sp4,sp2) > 0     )
                     {
                      if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,sp1,sp2,sp3)) goto errexit ; 
                      if( bcdtmList_deleteLineDtmObject(dtmP,sp1,sp2)) goto errexit ; 
                      if( bcdtmList_insertLineAfterPointDtmObject(dtmP,sp3,sp4,sp2)) goto errexit ; 
                      if( bcdtmList_insertLineAfterPointDtmObject(dtmP,sp4,sp3,sp1)) goto errexit ; 
                      p3 = p2 ;
                      ++*numRemovedP ;
                     }
                  } 
/*
**              Hull Point
*/
               }             
            }
/*
**        Set For Next Iteration
*/
          p2 = p3 ;
         }
      }
   } 
/*
** Write Sliver Points
*/
 if( dbg )
   {
    numDetected = 0 ;
    for( p1 = 0 ; p1 < dtmP->numPoints; ++p1 )
      {
       if( nodeAddrP(dtmP,p1)->sPtr )
         {
          bcdtmWrite_message(0,0,0,"Break Sliver Detected At Tin Point[%6ld] ** %12.5lf %12.5lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ; 
          ++numDetected ;
         }
      } 
    bcdtmWrite_message(0,0,0,"Number Of Break Slivers Detected = %6ld",numDetected) ;
   }
/*
** Write Stats On Number Of Slivers Removed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Break Slivers Removed  = %6ld",*numRemovedP) ;
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
BENTLEYDTM_Private int bcdtmClean_countNumberOfDtmFeatureTypeForTinPointDtmObject(BC_DTM_OBJ *dtmP,long dtmPnt,DTMFeatureType dtmFeatureType,long *numFeaturesP)
/*
** This Function Counts The Number Of The DTM Features Types For A Tin Point
*/
{
 long cln ;
/*
** Initialise
*/
 *numFeaturesP = 0 ; 
/*
** Count Number Of Features At Point
*/
 if( dtmFeatureType == DTMFeatureType::Hull && nodeAddrP(dtmP,dtmPnt)->hPtr != dtmP->nullPnt ) ++*numFeaturesP ;
/*
** Scan Feature List For Point
*/
 cln = nodeAddrP(dtmP,dtmPnt)->fPtr ;
 while( cln != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,cln)->dtmFeature)->dtmFeatureType == dtmFeatureType ) ++*numFeaturesP ;
    cln = flistAddrP(dtmP,cln)->nextPtr ;
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
BENTLEYDTM_Private int bcdtmClean_removeBreakLineSegmentAtTinPointDtmObject(BC_DTM_OBJ *dtmP,long dtmPnt)             
/*
** This Function Removes The Break Line Segment At Tin Point 
** Assumes P Is On Only One Break line
*/
{
 int  ret=DTM_SUCCESS ;
 long clc,cln,pnt,dtmFeature,lastPnt,numPts,closeFlag ;
/*
** Get Break Line Feature For Point
*/
 lastPnt = 0 ;
 dtmFeature = dtmP->nullPnt ;
 if(( cln = nodeAddrP(dtmP,dtmPnt)->fPtr ) != dtmP->nullPtr ) dtmFeature = flistAddrP(dtmP,cln)->dtmFeature ;
 else
   {
    lastPnt = 1 ;
/*
** Scan Tin Point For Which Tin Point Is Last Point In Break Line Feature
*/
    clc = nodeAddrP(dtmP,dtmPnt)->cPtr ;
    while( clc != dtmP->nullPtr && dtmFeature == dtmP->nullPnt )
      {
       pnt = clistAddrP(dtmP,clc)->pntNum;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
/*
**     Check If Cyclic List Point Has a Dtm Feature Whose Next Point Is The Tin Point
*/
       cln = nodeAddrP(dtmP,pnt)->fPtr ;
       while( cln != dtmP->nullPtr && dtmFeature == dtmP->nullPnt )
         {
          if(ftableAddrP(dtmP,flistAddrP(dtmP,cln)->dtmFeature)->dtmFeatureType == DTMFeatureType::Breakline && flistAddrP(dtmP,cln)->nextPnt == dtmPnt )
            {
             dtmFeature = flistAddrP(dtmP,cln)->dtmFeature ;
            }
          cln = flistAddrP(dtmP,cln)->nextPtr ;
         }
      }
   }
/*
** Check Feature Found
*/
 if( dtmFeature == dtmP->nullPnt ) 
   { 
    bcdtmWrite_message(2,0,0,"Break Line Feature Not Found") ;
    goto errexit ;
   }
/*
** Count Number Of Points In Feature
*/
 if( bcdtmList_countNumberOfDtmFeaturePointsDtmObject(dtmP,dtmFeature,&numPts,&closeFlag)) goto errexit ;
/*
** If Number Of Points Is Equal To Two Delete Feature From DTMFeatureState::Tin
*/
 if      ( numPts == 2 ) { if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ; }
 else if ( numPts >  2 ) { if( bcdtmInsert_removePointFromDtmFeatureDtmObject(dtmP,dtmPnt,dtmFeature)) goto errexit ; }
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
BENTLEYDTM_Public int bcdtmClean_validateStringP3D(DPoint3d **String,long *NumStringPts,double Pptol)
{
 int    ret=0 ;
 long   dbg=DTM_TRACE_VALUE(0) ;
 DPoint3d    *p3d1,*p3d2 ; 
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating String DPoint3d") ;
/*
**  Check For More Than Three Points
*/
 if( *NumStringPts < 2 )
   {
    bcdtmWrite_message(1,0,0,"Less Than 2 Points In String" ) ;
    goto errexit ; 
   }
/*
**  Eliminate  Points Within The Point To Point Tolerance
*/
 for( p3d1 = *String , p3d2 = *String + 1 ; p3d2 < *String + *NumStringPts  ; ++p3d2 )
   {
    if( bcdtmMath_distance(p3d1->x,p3d1->y,p3d2->x,p3d2->y) > Pptol )
      {
       ++p3d1 ;
       *p3d1 = *p3d2 ;
      }   
   }
/*
**    Set Number of String Points
*/
 *NumStringPts = (long)(p3d1-*String+1) ;
/*
**    Check For More Than Three Points
*/
 if( *NumStringPts < 2 )
   {
    bcdtmWrite_message(1,0,0,"Less Than 2 Points In String" ) ;
    goto errexit ; 
   }
/*
** Clean String Internally
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning String") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points Before Clean = %6ld",*NumStringPts) ;
 if( bcdtmClean_stringP3D(String,NumStringPts,Pptol) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points After  Clean = %6ld",*NumStringPts) ;
/*
** Check For More Than Three Points
*/
 if( *NumStringPts < 2 )
   {
    bcdtmWrite_message(1,0,0,"Less Than 2 Points In String" ) ;
    goto errexit ; 
   }
/*
**  Job Completed
*/
 cleanup :
 if( dbg && ! ret) bcdtmWrite_message(0,0,0,"Validating String DPoint3d Completed") ;
 if( dbg &&   ret) bcdtmWrite_message(0,0,0,"Validating String DPoint3d Error") ;
 return(ret) ;
/*
** Error Return
*/
 errexit :
 ret = 1 ;
 goto cleanup ; 
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClean_validatePointArrayPolygon
(
 DPoint3d    **polyPtsPP,
 long   *numPolyPtsP,
 long   forceClose,
 double ppTol
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTMDirection direction;
 double area ;
 DPoint3d    *p3d1P,*p3d2P ; 
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Validating Polygon DPoint3d") ;
    bcdtmWrite_message(0,0,0,"polyPtsPP     = %p",*polyPtsPP) ;
    bcdtmWrite_message(0,0,0,"numPolyPtsP   = %8ld",*numPolyPtsP) ;
    bcdtmWrite_message(0,0,0,"forceClose    = %8ld",forceClose) ;
    bcdtmWrite_message(0,0,0,"ppTol         = %20.15lf",ppTol) ;
   }
/*
**  Check For More Than Three Points
*/
 if( *numPolyPtsP <= 3 )
   {
    bcdtmWrite_message(1,0,0,"Polygon Has Less Than Three Points" ) ;
    goto errexit ; 
   }
/*
** Force Polygon To Close
*/
 if( forceClose ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Forcing Polygon To Close") ;
    (*polyPtsPP+*numPolyPtsP-1)->x = (*polyPtsPP)->x ; 
    (*polyPtsPP+*numPolyPtsP-1)->y = (*polyPtsPP)->y ; 
    (*polyPtsPP+*numPolyPtsP-1)->z = (*polyPtsPP)->z ; 
   }
/*
**    Check For Closure
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Closure") ;
 if( (*polyPtsPP+*numPolyPtsP-1)->x != (*polyPtsPP)->x ||  (*polyPtsPP+*numPolyPtsP-1)->y != (*polyPtsPP)->y )
   { 
    bcdtmWrite_message(1,0,0,"Feature Does Not Close") ;
    goto errexit ; 
   }
/*
**  Eliminate  Points Within The Point To Point Tolerance
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Eliminating Duplicate Points") ;
 for( p3d1P = *polyPtsPP , p3d2P = *polyPtsPP + 1 ; p3d2P < *polyPtsPP + *numPolyPtsP  ; ++p3d2P )
   {
    if( bcdtmMath_distance(p3d1P->x,p3d1P->y,p3d2P->x,p3d2P->y) > ppTol )
      {
       ++p3d1P ;
       *p3d1P = *p3d2P ;
      }   
   }
/*
** Set Number of Polygon Points
*/
 *numPolyPtsP = (long)(p3d1P-*polyPtsPP+1) ;
/*
** Force Polygon Close
*/
 if( forceClose ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Forcing Polygon To Close") ;
    (*polyPtsPP+*numPolyPtsP-1)->x = (*polyPtsPP)->x ; 
    (*polyPtsPP+*numPolyPtsP-1)->y = (*polyPtsPP)->y ; 
    (*polyPtsPP+*numPolyPtsP-1)->z = (*polyPtsPP)->z ; 
   }
/*
**    Check For More Than Three Points
*/
 if( *numPolyPtsP <= 3 )
   {
    bcdtmWrite_message(1,0,0,"Polygon Has Less Than Three Points" ) ;
    goto errexit ; 
   }
/*
** Clean Polygon Internally
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Polygon Internally ** Number Of Polygon Points = %6ld",*numPolyPtsP) ;
 if( bcdtmClean_internalPointArrayPolygon(polyPtsPP,numPolyPtsP,ppTol) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of  Points After Internal Clean = %6ld",*numPolyPtsP) ;
/*
** Check For More Than Three Points
*/
 if( *numPolyPtsP <= 3 )
   {
    bcdtmWrite_message(1,0,0,"Polygon Has Less Than Three Points" ) ;
    goto errexit ; 
   }
/*
** Clean Polygon Externally
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Polygon Externally ** Number Of Polygon Points = %6ld",*numPolyPtsP) ;
 if( bcdtmClean_externalPointArrayPolygon(polyPtsPP,numPolyPtsP,ppTol) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Polygon Points After External Clean = %6ld",*numPolyPtsP) ;
/*
**  Check For More Than Three Points
*/
 if( *numPolyPtsP <= 3 )
   {
    bcdtmWrite_message(1,0,0,"Polygon Has Less Than Three Points" ) ;
    goto errexit ; 
   }
/*
**  Get Polygon Direction
*/
 bcdtmMath_getPolygonDirectionP3D(*polyPtsPP,*numPolyPtsP,&direction,&area) ;
 if (direction == DTMDirection::Unknown)
   {
    bcdtmWrite_message(1,0,0,"Polygon Lines Are Colinear") ;
    goto errexit ; 
   }
 if (direction == DTMDirection::Clockwise) bcdtmMath_reversePolygonDirectionP3D (*polyPtsPP, *numPolyPtsP);
/*
**  Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Polygon DPoint3d Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Polygon DPoint3d Error") ;
 return(ret) ;
/*
** Error Return
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
BENTLEYDTM_Public int bcdtmClean_stringP3D
(
 DPoint3d    **stringPtsP,
 long   *numStringPtsP,
 double ppTol
)
/*
** This Function Cleans A DPoint3d String
*/
{
 int        ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long       sp,tp,spnt,dtmFeature ;
 double     sx,sy,dx,dy,dl,xMin,xMax,yMin,yMax,zMin ; 
 DPoint3d        *p3dP,breakLine[2] ;
 BC_DTM_OBJ *dtmP=NULL ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Cleaning DPoint3d String") ;
    bcdtmWrite_message(0,0,0,"stringPtsP   = %p",*stringPtsP) ;
    bcdtmWrite_message(0,0,0,"*numStringPtsP = %8ld",*numStringPtsP) ;
    bcdtmWrite_message(0,0,0,"ppTol        = %8.5lf",ppTol) ;
   } 
/*
** Check For Existence Of DPoint3d Points
*/
 if( *stringPtsP != NULL && *numStringPtsP > 2 )
   {
/*
**  Save Start Point
*/
    sx = (*stringPtsP)->x ;   
    sy = (*stringPtsP)->y ;   
/*
**  Create data Object
*/
    if( bcdtmObject_createDtmObject(&dtmP))  goto errexit ; 
/*
**  Set Memory Allocation Parameters For DTM Object
*/
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,*numStringPtsP*3,*numStringPtsP) ;
/*
**  Copy DPoint3d String Segments To DTM Object As Break Lines
*/
    for( p3dP = *stringPtsP ; p3dP < *stringPtsP + *numStringPtsP - 1 ; ++p3dP )
      {
       breakLine[0].x = p3dP->x ;
       breakLine[0].y = p3dP->y ;
       breakLine[0].z = p3dP->z ;
       breakLine[1].x = (p3dP+1)->x ;
       breakLine[1].y = (p3dP+1)->y ;
       breakLine[1].z = (p3dP+1)->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,breakLine,2)) goto errexit ;
      }
/*
**  Create Bounding Rectangle For String
*/
    xMin = dtmP->xMin ;
    xMax = dtmP->xMax ;
    yMin = dtmP->yMin ;
    yMax = dtmP->yMax ;
    zMin  = dtmP->zMin ;
    dx = xMax - xMin ;
    dy = yMax - yMin ;
    if( dx >= dy ) dl = dx ;
    else           dl = dy ;
    dl = dl / 2.0 ;
    xMin = xMin - dl ;
    xMax = xMax + dl ;
    yMin = yMin - dl ;
    yMax = yMax + dl ;
/*
** Store Bounding Cube Coordinates In Data Object
*/
    breakLine[0].x = xMin ;
    breakLine[0].y = yMin ;
    breakLine[0].z = zMin ;
    breakLine[1].x = xMax ;
    breakLine[1].y = yMin ;
    breakLine[1].z = zMin ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,breakLine,2)) goto errexit ;
    breakLine[0].x = xMin ;
    breakLine[0].y = yMax ;
    breakLine[0].z = zMin ;
    breakLine[1].x = xMax ;
    breakLine[1].y = yMax ;
    breakLine[1].z = zMin ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,breakLine,2)) goto errexit ;
/*
** Free String Memory
*/
   free(*stringPtsP)  ;
   *stringPtsP = NULL ;
   *numStringPtsP = 0 ;
/*
** Triangulate DTM Object
*/
   dtmP->ppTol = dtmP->plTol = ppTol ;
   if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Copy Features To Tptr Array
*/
   for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
     {
      if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&spnt)) goto errexit ;
     } 
/*
** Find String Start Point In DTMFeatureState::Tin
*/
   bcdtmFind_closestPointDtmObject(dtmP,sx,sy,&spnt) ;
   if( dbg ) bcdtmWrite_message(0,0,0,"Start Point = %6ld Tptr = %9ld",spnt,nodeAddrP(dtmP,spnt)->tPtr) ;
/*
** Count Points In String
*/
    if( nodeAddrP(dtmP,spnt)->tPtr != dtmP->nullPnt )
      { 
       sp = spnt ;
       while(nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt && nodeAddrP(dtmP,sp)->tPtr >= 0 ) 
         { 
          ++(*numStringPtsP) ; 
          tp = sp ; 
          sp = nodeAddrP(dtmP,sp)->tPtr ; 
          nodeAddrP(dtmP,tp)->tPtr = -(nodeAddrP(dtmP,tp)->tPtr+1) ;  
         }
       if( *numStringPtsP > 0 ) ++(*numStringPtsP) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points In String = %6ld",*numStringPtsP) ;
/*
**     Copy Cleaned Points To DPoint3d Array
*/
       if( *numStringPtsP > 0 )
         {
          p3dP = *stringPtsP = (DPoint3d *) malloc( *numStringPtsP * sizeof(DPoint3d) ) ;
          if( *stringPtsP == NULL ) 
            { 
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
          sp = spnt ;
          do
            { 
             p3dP->x = pointAddrP(dtmP,sp)->x ;
             p3dP->y = pointAddrP(dtmP,sp)->y ;
             p3dP->z = pointAddrP(dtmP,sp)->z ;
             tp = sp ;
             if(nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) sp = -(nodeAddrP(dtmP,sp)->tPtr+1)  ;
             nodeAddrP(dtmP,tp)->tPtr = dtmP->nullPnt ;
             ++p3dP ;
            } while(nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
          p3dP->x = pointAddrP(dtmP,sp)->x ;
          p3dP->y = pointAddrP(dtmP,sp)->y ;
          p3dP->z = pointAddrP(dtmP,sp)->z ;
         }
      }
   }   
/*
** Clean Up
*/
 cleanup :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ; 
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Cleaning String DPoint3d Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Cleaning String DPoint3d Error") ;
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
BENTLEYDTM_Public int bcdtmClean_externalPointArrayPolygon(DPoint3d **polyPtsPP,long *numPolyPtsP,double ppTol)
/*
** This Function Externally Cleans A DPoint3d Array Of Points
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     sp ;
 DPoint3d      *p3dP ;
 BC_DTM_OBJ *dtmP=NULL ;
 DPoint3d *pntP ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
 /*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Cleaning Externally DPoint3d Polygon") ;
    bcdtmWrite_message(0,0,0,"polyPtsPP    = %p",*polyPtsPP) ;
    bcdtmWrite_message(0,0,0,"numPolyPtsP  = %8ld",*numPolyPtsP) ;
    bcdtmWrite_message(0,0,0,"ppTol        = %8.6lf",ppTol) ;
   }
/*
** Check For Existence Of DPoint3d Points
*/
 if( *polyPtsPP == NULL || *numPolyPtsP < 3 ) 
   { 
    bcdtmWrite_message(0,0,0,"Less Than 3 Polygon Points") ;
    goto errexit ;
   }
 if( (*polyPtsPP)->x != (*polyPtsPP+*numPolyPtsP-1)->x || (*polyPtsPP)->y != (*polyPtsPP+*numPolyPtsP-1)->y ) 
   { 
    bcdtmWrite_message(1,0,0,"DPoint3d Polygon Does Not Close") ;
    goto errexit ;
   }
/*
** Create data Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
/*
** Set Point Memory Allocation Parameters For Dtm Object
*/
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,*numPolyPtsP*2,*numPolyPtsP*2) ) goto errexit ;
/*
** Store DTM Feature
*/
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,*polyPtsPP,*numPolyPtsP)) goto errexit ; 
/*
** Free Memory
*/
 free(*polyPtsPP) ;
 *polyPtsPP = NULL ;
/*
** Triangulate DTM Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 dtmP->ppTol = dtmP->plTol = ppTol ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Remove None Feature Hull Lines
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
/*
**  Count Number Of Tin Hull Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Counting Number Of Tin Hull Points") ;
 *numPolyPtsP = 0 ;
 sp = dtmP->hullPoint ;
 do
   {
    ++*numPolyPtsP  ;
    sp = nodeAddrP(dtmP,sp)->hPtr ;
   } while ( sp != dtmP->hullPoint ) ;
 ++*numPolyPtsP  ;
/*
** Allocate Memory For Cleaned Points
*/
 *polyPtsPP = ( DPoint3d * ) malloc ( *numPolyPtsP * sizeof(DPoint3d)) ;
 if( *polyPtsPP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Copy Hull Points To Clean Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Hull Points To Clean Points") ;
 p3dP = *polyPtsPP ;
 sp = dtmP->hullPoint ;
 do
   {
    pntP = pointAddrP(dtmP,sp) ;
    p3dP->x = pntP->x ;
    p3dP->y = pntP->y ;
    p3dP->z = pntP->z ;
    ++p3dP ; 
    sp = nodeAddrP(dtmP,sp)->hPtr ; ;
   } while ( sp != dtmP->hullPoint ) ;
 pntP = pointAddrP(dtmP,sp) ;
 p3dP->x = pntP->x ;
 p3dP->y = pntP->y ;
 p3dP->z = pntP->z ;
/*
** Free Memory
*/
 cleanup :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ; 
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Cleaning Externally DPoint3d Polygon Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Cleaning Externally DPoint3d Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit : 
 *numPolyPtsP = 0 ;
 if( *polyPtsPP != NULL )  { free(*polyPtsPP) ; *polyPtsPP = NULL ; }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ; 
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClean_internalPointArrayPolygon
(
 DPoint3d    **p3dPtsPP,
 long   *numP3dPtsP,
 double ppTol
)

// This Function Internally Cleans A DPoint3d Array Of Points

{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     sp,np,lp=0,cln,snp=0,spnt,nfeat;
 DTMDirection direction;
 long     process,startPnt,startNextPoint=0,startDirection=0 ;
 long     scanDirection,polyFound ;
 double   area,featureArea=0.0 ;
 DPoint3d      *p3dP,dtmPoint[2] ;
 BC_DTM_OBJ    *dtmP=NULL ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;


// Write Status Message

 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Cleaning Internally DPoint3d Polygon") ;
    bcdtmWrite_message(0,0,0,"p3dPtsPP       = %p",*p3dPtsPP) ;
    bcdtmWrite_message(0,0,0,"numP3dPtsP     = %4ld",*numP3dPtsP) ;
    bcdtmWrite_message(0,0,0,"ppTol          = %12.10lf",ppTol) ;
   }

// Check For Existence Of DPoint3d Points

 if( *p3dPtsPP == NULL || *numP3dPtsP < 3 ) 
   { 
    bcdtmWrite_message(0,0,0,"Less Than 3 Polygon Points") ;
    goto errexit ; 
   }
 if( (*p3dPtsPP)->x != (*p3dPtsPP+*numP3dPtsP-1)->x || (*p3dPtsPP)->y != (*p3dPtsPP+*numP3dPtsP-1)->y ) 
   { 
    bcdtmWrite_message(1,0,0,"DPoint3d Polygon Does Not Close") ;
    goto errexit ; 
   }

// Get direction Of Polygon

 bcdtmMath_getPolygonDirectionP3D(*p3dPtsPP,*numP3dPtsP,&direction,&area) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygon direction = %2ld area = %10.4lf",direction,area) ;

// Set direction Of Polygon Anti Clockwise

 if (direction == DTMDirection::Clockwise)  bcdtmMath_reversePolygonDirectionP3D (*p3dPtsPP, *numP3dPtsP);

// Create DTM Object

 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;

// Set Memory Allocation Parameters For DTM Object

 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,*numP3dPtsP*2,*numP3dPtsP) ) goto errexit ;

//  Copy DPoint3d Line Segnments To DTM Object As Break Lines

 for( p3dP = *p3dPtsPP ; p3dP < *p3dPtsPP + *numP3dPtsP - 1 ; ++p3dP )
   {
    dtmPoint[0].x = p3dP->x     ;  dtmPoint[0].y = p3dP->y     ;  dtmPoint[0].z = p3dP->z ; 
    dtmPoint[1].x = (p3dP+1)->x ;  dtmPoint[1].y = (p3dP+1)->y ;  dtmPoint[1].z = (p3dP+1)->z ; 
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,dtmPoint,2)) goto errexit ;
   }

// Triangulate DTM Object

 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ; 
 dtmP->ppTol = ppTol ;
 dtmP->plTol = ppTol ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;

// Find A Point That Has Only One Feature

 startPnt = dtmP->nullPnt ;
 if(dbg) bcdtmWrite_message(0,0,0,"Finding Points That Have Only Feature") ;
 for( spnt = 0 ; spnt < dtmP->numPoints ; ++spnt ) nodeAddrP(dtmP,spnt)->PRGN = 1 ;
 for( spnt = 0 ; spnt < dtmP->numPoints ; ++spnt )
   {
    if( nodeAddrP(dtmP,spnt)->PRGN )
      {

//     Count Number Of Features At Point

       nfeat = 0 ;
       cln = nodeAddrP(dtmP,spnt)->fPtr  ;
       while ( cln != dtmP->nullPtr )
         {
          ++nfeat ;
          snp = flistAddrP(dtmP,cln)->nextPnt ;
          cln = flistAddrP(dtmP,cln)->nextPtr ;
         } 

//     Get Next Point

       if( nfeat == 1 )
         {

//        Scan Internal To Feature

          process = 1 ;   
          polyFound = 0 ;  
          scanDirection = 1 ;
          while ( process )
            {
             sp = spnt ;  
             np = snp  ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Scan direction = %2ld Start Point = %6ld Next Point = %6ld Number Of Features = %2ld",scanDirection,spnt,np,nfeat) ;
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

//           Check Polygon Closes Correctly

             if( sp != spnt )
               {
                if( dbg ) 
                  {
                   bcdtmWrite_message(0,0,0,"Polygon Does Not Close Correctly // scanDirection = %2ld",scanDirection) ;
                   bcdtmWrite_message(0,0,0,"Polygon Start Point = %7ld // %10.4lf %10.4lf %10.4lf",spnt,pointAddrP(dtmP,spnt)->x,pointAddrP(dtmP,spnt)->y,pointAddrP(dtmP,spnt)->z) ;
                   bcdtmWrite_message(0,0,0,"Polygon End   Point = %7ld // %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                  } 
                bcdtmList_nullTptrValuesDtmObject(dtmP) ;  
                if     ( scanDirection == 1 ) scanDirection = 2 ;
                else if( scanDirection == 2 ) process = 0 ;
               }
             else { polyFound = 1 ; process = 0 ; }
            } 

//        Determine direction Of Tptr Polgon

          if( polyFound )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Calculating area And direction Tptr Polygon") ;
             bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,spnt,&area,&direction) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Clean Polygon direction = %2ld area = %10.4lf",direction,area) ;

//           Check For Feature Start Point

             if( startPnt == dtmP->nullPnt || area > featureArea )
               {
                startPnt     = spnt ;
                startNextPoint = snp ;
                startDirection = scanDirection ;
                featureArea    = area ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Start Point = %6ld Next Point = %6ld",startPnt,startNextPoint,featureArea) ;
               }

//           Mark Points And Null Out Tptr List

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

// Write Status

 if( dbg && startPnt == dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"Polygon Not Cleaned") ;

//  If Start Point Found Copy Feature Polygon

 if( startPnt != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Polygon Cleaned Start Point = %6ld",startPnt) ;

//  Free Memory

    free(*p3dPtsPP)  ; 
    *p3dPtsPP = NULL ;
    *numP3dPtsP = 0   ;
    spnt = startPnt   ;
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

//  Copy Tptr List To Cleaned Feature Points

    if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,spnt,p3dPtsPP,numP3dPtsP)) goto errexit ;
   }

// Free Memory

 cleanup :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ; 

// Job Completed

 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Internally DPoint3d Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Internally DPoint3d Polygon Error") ;
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
BENTLEYDTM_Public int bcdtmClean_stringLineIntersectionTableCompareFunction(const DTM_STR_INT_TAB *Tp1,const DTM_STR_INT_TAB  *Tp2)
/*
** Compare Function For Qsort Of String Line Intersection Table Entries
*/
{
 if     (  Tp1->X1  < Tp2->X1 ) return(-1) ;
 else if(  Tp1->X1  > Tp2->X1 ) return( 1) ;
 else if(  Tp1->X1  == Tp2->X2 && Tp1->Y1 < Tp2->Y1 ) return(-1) ;
 else if(  Tp1->X1  == Tp2->X2 && Tp1->Y1 > Tp2->Y1 ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClean_scanForStringLineIntersections(DTM_STR_INT_TAB *IntTable,long IntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc)
/*
** This Function Scans for Radial Base Line Intersections
*/
{
 int     ret=0 ;
 long    ActIntTableNe=0,ActIntTableMe=0 ;
 DTM_STR_INT_TAB *pint,*ActIntTable=NULL ;
/*
** Scan Sorted Point Table and Look For Intersections
*/
 for( pint = IntTable ; pint < IntTable + IntTableNe  ; ++pint)
   {
    if( bcdtmClean_deleteActiveStringLines(ActIntTable,&ActIntTableNe,pint)) goto errexit ;
    if( bcdtmClean_addActiveStringLine(&ActIntTable,&ActIntTableNe,&ActIntTableMe,pint))  goto errexit ;
    if( bcdtmClean_determineActiveStringLineIntersections(ActIntTable,ActIntTableNe,IntPts,IntPtsNe,IntPtsMe,IntPtsMinc)) goto errexit ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( ActIntTable != NULL ) free(ActIntTable) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClean_deleteActiveStringLines(DTM_STR_INT_TAB *ActIntTable,long *ActIntTableNe,DTM_STR_INT_TAB *Pint)
/*
** This Functions Deletes Entries From The Active Radial Base Line Intersection List
*/
{
 long            cnt=0 ;
 DTM_STR_INT_TAB *pal1,*pal2 ;
/*
** Scan Active List List And Mark Entries For Deletion
*/
 for ( pal1 = ActIntTable ; pal1 < ActIntTable + *ActIntTableNe ; ++pal1 )
   {
    if( pal1->X2 < Pint->X1 ) { pal1->String = DTM_NULL_PNT ; ++cnt ; }
   }
 if( cnt == 0 ) return(0) ; 
/*
** Delete Marked Entries
*/
 if( cnt > 0 )
   {
    for( pal1 = pal2 = ActIntTable ; pal2 < ActIntTable + *ActIntTableNe ; ++pal2 )
      {
       if( pal2->String != DTM_NULL_PNT )
         {
          if( pal1 != pal2 ) *pal1 = *pal2 ;
          ++pal1 ;
         }
      }
   }
/*
** Reset Number Of Active Entries
*/
 *ActIntTableNe = *ActIntTableNe - cnt ;
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
BENTLEYDTM_Public int bcdtmClean_addActiveStringLine(DTM_STR_INT_TAB **ActIntTable,long *ActIntTableNe,long *ActIntTableMe,DTM_STR_INT_TAB *Pint)
/*
** This Functions Adds An Entry From The Active Line List
*/
{
 long MemInc=100 ;
/*
** Test For Memory
*/
 if( *ActIntTableNe == *ActIntTableMe )
   {
    *ActIntTableMe = *ActIntTableMe + MemInc ;
    if( *ActIntTable == NULL ) *ActIntTable = (DTM_STR_INT_TAB*)malloc ( *ActIntTableMe * sizeof(DTM_STR_INT_TAB)) ;
    else                       *ActIntTable = (DTM_STR_INT_TAB*)realloc( *ActIntTable, *ActIntTableMe * sizeof(DTM_STR_INT_TAB)) ;
    if( *ActIntTable == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return(1) ; }
   }
/*
** Store Entry
*/
 *(*ActIntTable+*ActIntTableNe) = *Pint ;
 ++*ActIntTableNe ;
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
BENTLEYDTM_Public int bcdtmClean_determineActiveStringLineIntersections(DTM_STR_INT_TAB *ActIntTable,long ActIntTableNe,DTM_STR_INT_PTS **IntPts,long *IntPtsNe,long *IntPtsMe,long IntPtsMinc ) 
/*
** Determine Line Intersections
*/
{
 double           di,dl,dz,Xs=0.0,Ys=0.0,Zs=0.0,Xe=0.0,Ye=0.0,Ze=0.0,x,y ;
 DTM_STR_INT_TAB  *alp,*slp ;
/*
** Initialise
*/
 alp = ActIntTable + ActIntTableNe - 1 ;
/*
** Scan Active Line List
*/
 for( slp = ActIntTable ; slp < ActIntTable + ActIntTableNe - 1 ; ++slp )
   {
/*
** Check Lines Are Not Consecutive Segments
*/
    if( alp->String != slp->String || ( alp->String == slp->String && labs(alp->Segment-slp->Segment) > 1 ) )
      {
/*
**  Check Lines Do Not Close String
*/
       if( alp->String == slp->String && ( alp->Type != 2 || slp->Type != 2 ) )
         {
/*
** Check Lines Intersect
*/
          if( bcdtmMath_checkIfLinesIntersect(slp->X1,slp->Y1,slp->X2,slp->Y2,alp->X1,alp->Y1,alp->X2,alp->Y2))
            {
/*
** Intersect Lines
*/          
             bcdtmMath_normalIntersectCordLines(slp->X1,slp->Y1,slp->X2,slp->Y2,alp->X1,alp->Y1,alp->X2,alp->Y2,&x,&y) ;
/*
** Check Memory
*/
             if( *IntPtsNe + 1 >= *IntPtsMe )
               {
                *IntPtsMe = *IntPtsMe + IntPtsMinc ;
                if( *IntPts == NULL ) *IntPts = ( DTM_STR_INT_PTS * ) malloc ( *IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
                else                  *IntPts = ( DTM_STR_INT_PTS * ) realloc( *IntPts,*IntPtsMe * sizeof(DTM_STR_INT_PTS)) ;
                if( *IntPts == NULL ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
               }
/*
** Calculate Distances For Alp
*/
             if( alp->Direction == 1 ) { Xs = alp->X1 ; Ys = alp->Y1 ; Zs = alp->Z1 ; Xe = alp->X2 ; Ye = alp->Y2 ; Ze = alp->Z2 ; }
             if( alp->Direction == 2 ) { Xs = alp->X2 ; Ys = alp->Y2 ; Zs = alp->Z2 ; Xe = alp->X1 ; Ye = alp->Y1 ; Ze = alp->Z1 ; }
             dz = Ze - Zs ;
             di = bcdtmMath_distance(Xs,Ys,x,y) ;
             dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
** Store Intersection Point Alp
*/
             (*IntPts+*IntPtsNe)->String1  = alp->String  ; 
             (*IntPts+*IntPtsNe)->Segment1 = alp->Segment ; 
             (*IntPts+*IntPtsNe)->String2  = slp->String  ; 
             (*IntPts+*IntPtsNe)->Segment2 = slp->Segment ; 
             (*IntPts+*IntPtsNe)->Distance = di ; 
             (*IntPts+*IntPtsNe)->x = x ; 
             (*IntPts+*IntPtsNe)->y = y ; 
             (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ; 
             ++*IntPtsNe ; 
/*
** Calculate Distances For Slp
*/
             if( slp->Direction == 1 ) { Xs = slp->X1 ; Ys = slp->Y1 ; Zs = slp->Z1 ; Xe = slp->X2 ; Ye = slp->Y2 ; Ze = slp->Z2 ; }
             if( slp->Direction == 2 ) { Xs = slp->X2 ; Ys = slp->Y2 ; Zs = slp->Z2 ; Xe = slp->X1 ; Ye = slp->Y1 ; Ze = slp->Z1 ; }
             dz = Ze - Zs ;
             di = bcdtmMath_distance(Xs,Ys,x,y) ;
             dl = bcdtmMath_distance(Xs,Ys,Xe,Ye) ;
/*
**           Store Intersection Point For Slp
*/
             (*IntPts+*IntPtsNe)->String1  = slp->String  ; 
             (*IntPts+*IntPtsNe)->Segment1 = slp->Segment ; 
             (*IntPts+*IntPtsNe)->String2  = alp->String  ; 
             (*IntPts+*IntPtsNe)->Segment2 = alp->Segment ; 
             (*IntPts+*IntPtsNe)->Distance = di ; 
             (*IntPts+*IntPtsNe)->x = x ; 
             (*IntPts+*IntPtsNe)->y = y ; 
             (*IntPts+*IntPtsNe)->z = Zs + dz * di / dl ; 
             ++*IntPtsNe ; 
/*
**           Store Z2 Values
*/
             (*IntPts+*IntPtsNe-2)->Z2 = (*IntPts+*IntPtsNe-1)->z ; 
             (*IntPts+*IntPtsNe-1)->Z2 = (*IntPts+*IntPtsNe-2)->z ; 
            }
         }
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
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClean_checkP3DStringForKnots(DPoint3d *StringPts,long StringPtsNe,DTM_STR_INT_PTS **KnotPts,long *KnotPtsNe)
/*
**
** This Function Checks A DPoint3d String For Knots
** Assumes Prior Removal Of Duplicate String Points
**
*/
{
 int    ret=0 ;
 long   dbg=DTM_TRACE_VALUE(0),IntTableNe,KnotPtsMe=0,KnotPtsMinc=100 ;
 DTM_STR_INT_TAB *pint,*IntTable=NULL ;
 DTM_STR_INT_PTS *pinp ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking DPoint3d String DPoint3d For Knots") ;
/*
** Initialise 
*/
 *KnotPtsNe = 0 ;
 if( *KnotPts != NULL ) { free(*KnotPts) ; *KnotPts = NULL ; }
/*
**  Check For More Than Three Points In String
*/
 if( StringPts == NULL || StringPtsNe <= 2 ) goto cleanup ;
/*
** Build String Line Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building String Line Intersection Table") ; 
 if( bcdtmClean_buildStringLineIntersectionTable(StringPts,StringPtsNe,&IntTable,&IntTableNe) )  goto errexit ;
/*
** Write Intersection Table
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of String Line Intersection Table Entries = %6ld",IntTableNe ) ;
    for( pint = IntTable ; pint < IntTable + IntTableNe ; ++pint )   
      {
       bcdtmWrite_message(0,0,0,"Entry[%4ld] ** String = %4ld Segment = %4ld Type = %1ld Direction = %1ld ** %10.4lf %10.4lf %10.4lf ** %10.4lf %10.4lf %10.4lf",(long)(pint-IntTable),pint->String,pint->Segment,pint->Type,pint->Direction,pint->X1,pint->Y1,pint->Z1,pint->X2,pint->Y2,pint->Z2) ;
      }
   }
/*
** Scan For Intersections
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Intersections") ; 
 if( bcdtmClean_scanForStringLineIntersections(IntTable,IntTableNe,KnotPts,KnotPtsNe,&KnotPtsMe,KnotPtsMinc) ) goto errexit ; 
/*
** Sort Intersection Points
*/
 if( *KnotPtsNe > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ; 
    qsortCPP(*KnotPts,*KnotPtsNe,sizeof(DTM_STR_INT_PTS),bcdtmClean_stringLineIntersectionPointsCompareFunction) ;
   }
/*
** Write Intersection Points
*/ 
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Number Of Intersections = %6ld",*KnotPtsNe) ; 
    for( pinp = *KnotPts ; pinp < *KnotPts + *KnotPtsNe ; ++pinp )
      {
       bcdtmWrite_message(0,0,0,"Int Point[%4ld] ** Str1 = %4ld Seg1 = %5ld Str2 = %4ld Seg2 = %5ld Dist = %8.4lf x = %10.4lf y = %10.4lf z = %10.4lf",(long)(pinp-*KnotPts),pinp->String1,pinp->Segment1,pinp->String2,pinp->Segment2,pinp->Distance,pinp->x,pinp->y,pinp->z) ;
      }
   } 
/*
**  Job Completed
*/
 cleanup :
 if( IntTable != NULL ) free(IntTable) ;
 if( dbg && ! ret) bcdtmWrite_message(0,0,0,"Checking DPoint3d String DPoint3d For Knots Completed") ;
 if( dbg &&   ret) bcdtmWrite_message(0,0,0,"Checking DPoint3d String DPoint3d For Knots Error") ;
 return(ret) ;
/*
** Error Return
*/
 errexit :
 ret = 1 ;
 *KnotPtsNe = 0 ;
 if( *KnotPts != NULL ) { free(*KnotPts) ; *KnotPts = NULL ; }
 goto cleanup ; 
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClean_stringLineIntersectionPointsCompareFunction(const DTM_STR_INT_PTS *Tp1,const DTM_STR_INT_PTS  *Tp2)
/*
** Compare Function For Qsort Of String Line Intersection Table Entries
*/
{
 if     (  Tp1->String1 < Tp2->String1  ) return(-1) ;
 else if(  Tp1->String1 > Tp2->String1  ) return( 1) ;
 else if( Tp1->Segment1 < Tp2->Segment1 ) return(-1) ;
 else if( Tp1->Segment1 > Tp2->Segment1 ) return( 1) ;
 else if( Tp1->Distance < Tp2->Distance ) return(-1) ;
 else if( Tp1->Distance > Tp2->Distance ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|    bcdtmClean_buildStringLineIntersectionTable                       |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClean_buildStringLineIntersectionTable(DPoint3d *StringPts,long StringPtsNe,DTM_STR_INT_TAB **IntTable,long *IntTableNe) 
{
 int    ret=0 ;
 long   dbg=DTM_TRACE_VALUE(0),IntTableMe,IntTableMinc,CloseFlag  ;
 double cord ; 
 DPoint3d    *p3d ;
 DTM_STR_INT_TAB *pint ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building String Line Intersection Table") ;
/*
** Initialise
*/
 *IntTableNe = IntTableMe = 0 ;
 if( *IntTable != NULL ) { free(*IntTable) ; *IntTable = NULL ; }
 IntTableMinc = StringPtsNe - 1  ;
/*
** Check For Closure
*/
 CloseFlag = 0 ;
 if( StringPts->x == (StringPts+StringPtsNe-1)->x && StringPts->y == (StringPts+StringPtsNe-1)->y ) CloseFlag = 1 ;
/*
** Store String Segments In Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Radials In Intersection Table") ;
 for( p3d = StringPts ; p3d < StringPts + StringPtsNe - 1 ; ++p3d )
   {
/*
**  Check For Memory Allocation
*/
    if( *IntTableNe == IntTableMe )
      {
       IntTableMe = IntTableMe + IntTableMinc ;
       if( *IntTable == NULL ) *IntTable = ( DTM_STR_INT_TAB * ) malloc ( IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
       else                    *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable,IntTableMe * sizeof(DTM_STR_INT_TAB)) ;
       if( *IntTable == NULL ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
/*
**  Store String Line
*/
    (*IntTable+*IntTableNe)->String  = 0 ;
    (*IntTable+*IntTableNe)->Segment = (long)(p3d-StringPts) ;
    if( ( *IntTableNe == 0 && CloseFlag ) || ( *IntTableNe == StringPtsNe - 2 && CloseFlag ) ) (*IntTable+*IntTableNe)->Type = 2   ;
    else                                                                                       (*IntTable+*IntTableNe)->Type = 1   ;
    (*IntTable+*IntTableNe)->Direction = 1 ;
    (*IntTable+*IntTableNe)->X1 = p3d->x ;
    (*IntTable+*IntTableNe)->Y1 = p3d->y ;
    (*IntTable+*IntTableNe)->Z1 = p3d->z ;
    (*IntTable+*IntTableNe)->X2 = (p3d+1)->x ;
    (*IntTable+*IntTableNe)->Y2 = (p3d+1)->y ;
    (*IntTable+*IntTableNe)->Z2 = (p3d+1)->z ;
    ++*IntTableNe ;
   }
/*
** Reallocate Intersection Table Memory
*/
 if( *IntTableNe != IntTableMe ) *IntTable = ( DTM_STR_INT_TAB * ) realloc ( *IntTable, *IntTableNe * sizeof(DTM_STR_INT_TAB)) ;
/*
** Order Line Coordinates In Increasing x and y Coordiante Values
*/
 for( pint = *IntTable ; pint < *IntTable + *IntTableNe ; ++pint )
   {
    if( pint->X1 > pint->X2 || ( pint->X1 == pint->X2 && pint->Y1 > pint->Y2 ) )
      {
       pint->Direction = 2 ;
       cord = pint->X1 ; pint->X1 = pint->X2 ; pint->X2 = cord ;       
       cord = pint->Y1 ; pint->Y1 = pint->Y2 ; pint->Y2 = cord ;       
       cord = pint->Z1 ; pint->Z1 = pint->Z2 ; pint->Z2 = cord ;       
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ; 
 qsortCPP(*IntTable,*IntTableNe,sizeof(DTM_STR_INT_TAB),bcdtmClean_stringLineIntersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Building Radial Base Line Intersection Table Error") ;
 return(ret) ;
/*
** Error Exit 
*/
 errexit :
 *IntTableNe = 0 ;
 if( *IntTable != NULL ) { free(*IntTable) ; *IntTable = NULL ; }
 ret = 1 ;
 goto cleanup ;
}
