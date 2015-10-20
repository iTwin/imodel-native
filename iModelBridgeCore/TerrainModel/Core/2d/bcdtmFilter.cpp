/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmFilter.cpp $
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
BENTLEYDTM_EXPORT int bcdtmFilter_dtmFeatureTypeDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureType dtmFeatureType,
 double xyTolerance,
 double zTolerance,
 long   *numFeaturesFilteredP,
 long   *numVerticesBeforeFilterP,
 long   *numVerticesAfterFilterP
)
/*
** This Function Filters All Occurrences Of A DTM Feature Type In A DTM Object
*/
{
 int            ret=DTM_SUCCESS;
 long           point,feature,dtmFeature,numFeaturePts,numBeforeFilter,numAfterFilter ;
 DPoint3d            *p3dP,*featurePtsP=NULL ; 
 BC_DTM_FEATURE *dtmFeatureP ;
 DPoint3d   *point1P,*point2P ;
/*
** Initialise
*/
 *numFeaturesFilteredP     = 0 ;
 *numVerticesBeforeFilterP = 0 ;
 *numVerticesAfterFilterP  = 0 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** DTM Must Be In Points Unsorted State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(2,0,0,"DTM State Not Valid For Feature Filtering") ;
    goto errexit ;
   }
/*
** Scan Dtm Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == dtmFeatureType )
      {
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
         {
/*
**        Get Feature Points
*/
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
/*
**        Filter Feature
*/
          numBeforeFilter = numFeaturePts ;
          if( bcdtmFilter_pointArray(&featurePtsP,&numFeaturePts,xyTolerance,zTolerance)) goto errexit ;
          numAfterFilter  = numFeaturePts ;
          ++*numFeaturesFilteredP ;
          *numVerticesBeforeFilterP = *numVerticesBeforeFilterP + numBeforeFilter  ;
          *numVerticesAfterFilterP  = *numVerticesAfterFilterP  + numAfterFilter   ;
/*
**        Copy Over Points
*/
          if( numAfterFilter < numBeforeFilter )
            {
/*
**           Copy Filtered Points To DTM Points Array
*/
             point = dtmFeatureP->dtmFeaturePts.firstPoint ;
             for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
               {
                point1P = pointAddrP(dtmP,point) ;
                point1P->x = p3dP->x ;
                point1P->y = p3dP->y ;
                point1P->z = p3dP->z ;
                ++point ;
               }
/*
**           Copy Points In Points Array Up Over Deleted Points
*/
             point1P = pointAddrP(dtmP,point) ;
             ++point ;
             while( point < dtmP->numPoints )
               {
                point2P  = pointAddrP(dtmP,point) ;
                *point1P = *point2P ;
                point1P  = point2P ;
                ++point ;
               }
/*
**           Reset number Of Points In Points Array
*/
             dtmP->numPoints = dtmP->numPoints - ( numBeforeFilter - numAfterFilter) ;  
/*
**           Reset Lower Feature First Points
*/
             for( feature = dtmFeature + 1 ; feature < dtmP->numFeatures ; ++feature )
               {
                dtmFeatureP = ftableAddrP(dtmP,feature) ;
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
                  {
                   dtmFeatureP->dtmFeaturePts.firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint - ( numBeforeFilter - numAfterFilter) ;
                  }
               }
            }
/*
**        Free Feature Points Memory
*/
          if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
         }  
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
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
BENTLEYDTM_Private int bcdtmFilter_pointArray(DPoint3d **ptsPP,long *numPtsP,double xyTolerance,double zTolerance)
/*
** This Routine Filters The Point Array Vertices
*/
{
 int     ret=DTM_SUCCESS ;
 long    is1,is2,is3,numMark ;
 unsigned char    *markP,*markArrayP=NULL ;
 double  deltaxy,deltaz,d,dx,dy,dz,x,y,z,x1,y1,z1,x2,y2,z2,zi,a1,a2,a3,r;
 DPoint3d     *p3d1P,*p3d2P ;
/*
** Only Filter If Three Or More Points
*/
 if( *numPtsP >= 3 )
   {
/*
**  Create Array To Mark Deleted Vertices
*/
    numMark = *numPtsP / 8 + 1 ;
    markArrayP = ( unsigned char *) malloc ( numMark * sizeof(char)) ;
    if( markArrayP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      } 
    for( markP = markArrayP ; markP < markArrayP + numMark ; ++markP ) *markP = 255 ;
/*
**  Initialise Variables
*/
    deltaxy = fabs(xyTolerance) ;
    deltaz  = fabs(zTolerance)  ;
    is1 = 0 ; 
    is2 = 2 ; 
/*
**  Filter linear feature
*/
    while ( is2 < *numPtsP )
      {
       x1 = (*ptsPP+is1)->x ; y1 = (*ptsPP+is1)->y ; z1 = (*ptsPP+is1)->z ;
       x2 = (*ptsPP+is2)->x ; y2 = (*ptsPP+is2)->y ; z2 = (*ptsPP+is2)->z ;
       dx = x2 - x1 ; dy = y2 - y1 ; dz = z2 - z1 ;
       r = sqrt( dx*dx + dy*dy)    ;
       if( r > 0.0 )
         {
          a1 = dy / r ; a2 = -dx / r ; a3 = -a1 * x1 - a2 * y1 ;
          for( is3 = is1 + 1 ; is3 < is2 ; ++is3 )
            {
             x = (*ptsPP+is3)->x ; y = (*ptsPP+is3)->y ; z = (*ptsPP+is3)->z ;
             d = a1 * x + a2 * y + a3   ;
             if( dx != 0.0 ) zi = z1 + ( x - x1 ) / dx * dz ;
             else            zi = z1 + ( y - y1 ) / dy * dz ;
             if( fabs(d) > deltaxy || fabs(z-zi) > deltaz )
               {
                for( is3 = is1 + 1 ; is3 < is2 - 1  ; ++is3 ) bcdtmFlag_clearFlag(markArrayP,is3) ;
                is1 = is2 - 1 ;
                is3 = is2 ;
               }
            }
         }
       else  bcdtmFlag_clearFlag(markArrayP,is2) ;
       ++is2 ;
      }
/*
**  Copy Over Filtered Vertices
*/
    for( is1 = 0 , p3d1P = p3d2P = *ptsPP ; p3d2P < *ptsPP + *numPtsP ; ++is1 , ++p3d2P )
      {
       if( bcdtmFlag_testFlag(markArrayP,is1))
         {
          if( p3d1P != p3d2P ) *p3d1P = *p3d2P ;
          ++p3d1P ; 
         }  
      } 
/*
**  Reset Number Of Points
*/
    *numPtsP = (long ) (p3d1P - *ptsPP)  ;  
    *ptsPP = ( DPoint3d * ) realloc(*ptsPP , *numPtsP * sizeof(DPoint3d)) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( markArrayP != NULL ) free(markArrayP) ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_tinFilterRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object                         */
 long   filterOption,                  /* ==> Filter Option <1= Fine,2=Coarse>              */
 long   reinsertOption,                /* ==> Reinsert Points Out Of Tolerance <TRUE,FALSE> */
 double zTolerance,                    /* ==> Filter z Tolerance > 0.0                      */ 
 long   *numSpotsBeforeFilterP,        /* <== Number Of Spots Before Filter                 */
 long   *numSpotsAfterFilterP,         /* <== Number Of Spots After Filter                  */ 
 double *filterReductionP              /* <== Filter Reduction                              */ 
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long n,firstPoint,lastPoint,numTiles,saveLastPoint,maxPts=500000 ;
 long *tileOffsetP=NULL,*numTilePtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin Filtering Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterOption   = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"reinsertOption = %8ld",reinsertOption) ;
    bcdtmWrite_message(0,0,0,"zTolerance     = %8.3lf",zTolerance) ;
   }
/*
** Initialise
*/
 *filterReductionP = 0.0 ;
 *numSpotsBeforeFilterP = 0 ;
 *numSpotsAfterFilterP  = 0 ;
/*
** Validate Parameters
*/
 if( filterOption < 1 || filterOption > 2 ) filterOption = 1 ;
 if( zTolerance <= 0.0 )
   {
    bcdtmWrite_message(1,0,0,"Filter z Tolerance Must Be Greater Than Zero") ;
    goto errexit ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** If DTM In Triangulated State Change It To Untriangulated State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
   }
/*
** Check DTM Is In Untriangulated State
*/
  if( dtmP->dtmState != DTMState::Data )
    {
     bcdtmWrite_message(2,0,0,"Method Requires Untriangulated DTM") ;
     goto errexit ;
    }
/*
** Place Random Spots In One Contiguous DTM Block
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Blocking Random Spots") ;
 if( bcdtmFilter_blockRandomSpotsDtmObject(dtmP,&firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %8ld lastPoint = %8ld",firstPoint,lastPoint) ;
 *numSpotsBeforeFilterP = *numSpotsAfterFilterP = lastPoint - firstPoint + 1  ;
/*
** Sort Points On x Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Random Spots") ;
 if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
/*
** Tile Random Points
*/
 if( lastPoint - firstPoint > 10 )
   {
/*
**  Create maxPts Tiles
*/
    if( lastPoint - firstPoint > maxPts )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Tiling Random Points") ;
       if( bcdtmSort_pointsIntoTilesDtmObject(dtmP,firstPoint,lastPoint-firstPoint+1,maxPts,&tileOffsetP,&numTilePtsP,&numTiles)) goto errexit ;
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Number Of Tiles = %4ld",numTiles) ; 
          for( n = 0 ; n < numTiles ; ++n )
            {
             bcdtmWrite_message(0,0,0,"Tile[%4ld] ** offset = %8ld ** numPts = %8ld",n+1,*(tileOffsetP+n),*(numTilePtsP+n)) ;
            }
         }
      }
/*
**  Create One Tile
*/
    else
      {
       numTiles = 1 ; 
       tileOffsetP = ( long * ) malloc( numTiles * sizeof(long)) ;
       numTilePtsP = ( long * ) malloc( numTiles * sizeof(long)) ;
       if( tileOffsetP == NULL || numTilePtsP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         } 
       *tileOffsetP = 0 ;
       *numTilePtsP = lastPoint - firstPoint + 1 ; 
      } 
/*
**  Filtering Random Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Random Spot Tiles") ;
    if( bcdtmFilter_filterRandomSpotTilesDtmObject(dtmP,filterOption,reinsertOption,zTolerance,tileOffsetP,numTilePtsP,numTiles,numSpotsAfterFilterP)) goto errexit ;
   }
/*
** Set Feature States Back To First Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resetting Feature States To First Point") ;
 if( bcdtmFilter_setFeatureStatesToFirstPointDtmObject(dtmP)) goto errexit ;
/*
** Resize DTM Object
*/
 if( bcdtmObject_resizeMemoryDtmObject(dtmP)) goto errexit ;
/*
** Set Return Values
*/ 
 if( *numSpotsAfterFilterP == *numSpotsBeforeFilterP ) *filterReductionP = 0.0 ;
 else                                                  *filterReductionP = ( 1.0 - ( double ) *numSpotsAfterFilterP / ( double ) *numSpotsBeforeFilterP ) * 100.0 ;
/*
** Write Out Filter Results
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"spotsBefore = %8ld spotsAfter = %8ld reduction = %8.4lf",*numSpotsBeforeFilterP,*numSpotsAfterFilterP,*filterReductionP) ;
   } 
/*
**  Update Modified Time
 */
 bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
 if( tileOffsetP != NULL ) free(tileOffsetP) ;
 if( numTilePtsP != NULL ) free(numTilePtsP) ;  
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Filtering Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Filtering Random Spots Error") ;
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
BENTLEYDTM_Private int bcdtmFilter_blockRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object                         */
 long   *firstPointP,                  /* ==> Index To First Random Spot In DTM Object      */
 long   *lastPointP                    /* ==> Index To Last Random Spot In DTM Object       */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long n,pnt1,pnt2,markSize,dtmFeature,numFeaturePts,numFeatures=0,numRandomSpots,numCleared=0 ;
 unsigned char *cP,*markFlagP=NULL ;
 char dtmFeatureTypeName[50] ;
 DPoint3d  *featurePtsP=NULL ; 
 
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Blocking Random Spots In DTM Object") ;
/*
** Delete All Tin Feature Errors And Compact DTM
*/
 if( bcdtmData_deleteAllTinErrorFeaturesDtmObject(dtmP)) goto errexit ;
 if( bcdtmData_compactUntriangulatedFeatureTableDtmObject(dtmP )) goto errexit ;
/*
** Initialise
*/
 *firstPointP = 0 ;
 *lastPointP  = dtmP->numPoints - 1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Only Block If Features Are Present
*/
 if( dtmP->numFeatures )
   {
/*
**  Allocate An Array For Marking Random Points
*/
    markSize = dtmP->numPoints / 8 + 1 ;
    markFlagP = (unsigned char *) malloc( markSize * sizeof(char));
    if( markFlagP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }         
    for( cP = markFlagP ; cP < markFlagP + markSize ; ++cP ) *cP = 0xff ;
/*
**  Mark None Random Spots
*/
    numFeatures = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dbg == 1 ) 
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"dtmFeature = %6ld dtmFeatureType = %30s dtmFeatureState = %2ld numDtmFeaturePts = %8ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmFeatureState,dtmFeatureP->numDtmFeaturePts) ;
         } 
       if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Data )
         {
          bcdtmWrite_message(2,0,0,"Invalid DTM Feature State For Method") ;
          goto errexit ;
         }
/*
**     Copy Feature Points To Point Array
*/
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
/*
**     Mark Feature Points Copied
*/
       for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          ++numCleared ;
          bcdtmFlag_clearFlag(markFlagP,dtmFeatureP->dtmFeaturePts.firstPoint+n) ;
         }
/*
**     Store Points As Point Array
*/
       dtmFeatureP->dtmFeatureState = DTMFeatureState::PointsArray ;
       dtmFeatureP->dtmFeaturePts.pointsPI = bcdtmMemory_allocate(dtmP, numFeaturePts * sizeof(DPoint3d));
       memcpy(bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI), featurePtsP, numFeaturePts * sizeof(DPoint3d));
       free(featurePtsP);
       featurePtsP = NULL ;
       ++numFeatures ; 
      }
/*
**  Write Number Of Features
*/
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Number Of Features = %8ld",numFeatures) ; 
       bcdtmWrite_message(0,0,0,"Number Of Feature Points Cleared = %8ld",numCleared) ; 
      }
/*
**  Count Number Of Random Points
*/
    if( dbg == 1 )
      {
       numRandomSpots = 0 ;
       for( pnt1 = 0 ; pnt1 < dtmP->numPoints ; ++pnt1 )
         {
          if( bcdtmFlag_testFlag(markFlagP,pnt1)) ++numRandomSpots ;
         }
       bcdtmWrite_message(0,0,0,"Number Of Random Spots = %8ld",numRandomSpots) ;
      }
/*
**  Copy Over Marked Points
*/
    for( pnt1 = pnt2 = 0 ; pnt2 < dtmP->numPoints ; ++pnt2 )
      {
       if( bcdtmFlag_testFlag(markFlagP,pnt2) )
         {
          if( pnt2 != pnt1 ) *pointAddrP(dtmP,pnt1) = *pointAddrP(dtmP,pnt2) ;
          ++pnt1 ;
         } 
      } 
    dtmP->numPoints = pnt1 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
**  Set Return Values
*/
    *firstPointP = 0 ;
    *lastPointP  = dtmP->numPoints - 1 ;
   }
/*
** Clean Up
*/
 cleanup :
 if( markFlagP != NULL ) free(markFlagP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Blocking Random Spots In DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Blocking Random Spots In DTM Object Error") ;
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
BENTLEYDTM_Private int bcdtmFilter_setFeatureStatesToFirstPointDtmObject(BC_DTM_OBJ *dtmP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   dtmFeature,numFeaturePts ; 
 DPoint3d    *p3dP,*featurePtsP=NULL ;
 char   dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE *dtmFeatureP  ;
 DPoint3d *pointP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Feature States To First Point ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
**  Reset Features Back To First Point
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
      { 
       if( dbg ) 
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"dtmFeature = %6ld dtmFeatureType = %30s dtmFeatureState = %2ld numDtmFeaturePts = %8ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmFeatureState,dtmFeatureP->numDtmFeaturePts) ;
         }
/*
**     Copy Feature Points To Point Array
*/
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
/*
**     Free Feature Point Array Memory
*/
       bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
       dtmFeatureP->dtmFeaturePts.pointsPI = 0 ;
/*
**     Store Feature Points As First Points
*/
       dtmFeatureP->dtmFeatureState = DTMFeatureState::Data ;
       dtmFeatureP->dtmFeaturePts.firstPoint = dtmP->numPoints ;
/*
**     Store Points In Points Array
*/
       for( p3dP = featurePtsP ;  p3dP < featurePtsP + numFeaturePts ; ++p3dP )
         {
          pointP = pointAddrP(dtmP,dtmP->numPoints) ;
          pointP->x = p3dP->x ;
          pointP->y = p3dP->y ;
          pointP->z = p3dP->z ;
          ++dtmP->numPoints  ;
         }
/*
**     Free Memory
*/
       if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Setting Feature States To First Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Setting Feature States To First Point Error") ;
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
BENTLEYDTM_Private int bcdtmFilter_filterRandomSpotTilesDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object          */
 long   filterOption,                  /* ==> Filter Option                  */
 long   reinsertOption,                /* ==> Reinsert Option                */
 double zTolerance,                    /* ==> z Filter Tolerance             */
 long   *tileOffsetP,                  /* ==> Index  To First In Tile        */
 long   *numTilePtsP,                  /* ==> Number Of Spots In Tile        */
 long   numTiles,                      /* ==> Number Of Tiles                */
 long   *numFilteredSpotsP             /* <== Number Of Spots After Filter   */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,m,loop,point,node1,node2,numPtsFiltered ; 
 long fndType,tinPnt1,tinPnt2,tinPnt3,numReinserted ;
 DPoint3d  randomSpot ;
 double z ;
 BC_DTM_OBJ *dtm1P=NULL,*dtm2P=NULL ;
 DPoint3d *pointP ;
 DTMFeatureId featureId ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ; 
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Single Point Group Spot Tiles") ;
/*
** Initialise
*/
 *numFilteredSpotsP = 0 ;
/*
** Scan And Filter Each Tile
*/
 for( n = 0 ; n < numTiles ; ++n )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Filter Tile %4ld of %4ld ** numTilePts = %8ld",n+1,numTiles,*(numTilePtsP+n)) ;
/*
**  Create DTM Object For Tile Points
*/
    if( dtm1P != NULL ) if( bcdtmObject_destroyDtmObject(&dtm1P)) goto errexit ;
    if( bcdtmObject_createDtmObject(&dtm1P)) goto errexit ;
/*
**  Polulate DTM Object With Tile Spots
*/
    for( point = *(tileOffsetP+n) ; point < *(tileOffsetP+n) + *(numTilePtsP+n) ; ++point )
      {
       pointP = pointAddrP(dtmP,point) ;
       randomSpot.x = pointP->x ;
       randomSpot.y = pointP->y ;
       randomSpot.z = pointP->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,&randomSpot,1)) goto errexit ;
      }
/*
**  Iteratively Triangulate And Filter Dtm Object
*/
    loop = 5 ;
    while ( loop )
      {
       --loop ;
       if( dbg ) bcdtmWrite_message(0,0,0,"**** Triangulating And Filtering Tile ** numTilePts = %8ld",dtm1P->numPoints) ;
       dtm1P->edgeOption = 1 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Before Triangulation ** dtm1P->numPoints = %8ld",dtm1P->numPoints) ;
       if( bcdtmObject_triangulateDtmObject(dtm1P)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"After  Triangulation ** dtm1P->numPoints = %8ld",dtm1P->numPoints) ;
       if( filterOption == 1 ) if( bcdtmLidarFilter_coarseFilterTinPointsDtmObject(dtm1P,zTolerance,&numPtsFiltered)) goto errexit ;
       if( filterOption == 2 ) if( bcdtmLidarFilter_fineFilterTinPointsDtmObject(dtm1P,zTolerance,&numPtsFiltered)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points Filtered = %8ld",numPtsFiltered) ;
       if( numPtsFiltered == 0 ) loop = 0 ;
/*
**     Remove Deleted Spots
*/
       for( node1 = node2 = 0 ; node2 < dtm1P-> numPoints ; ++node2 )
         {
          if( nodeAddrP(dtm1P,node2)->tPtr == dtm1P->nullPnt )
            {
             if( node2 != node1 )
               {
                *pointAddrP(dtm1P,node1) = *pointAddrP(dtm1P,node2) ;
               }
             ++node1 ;
            }
         } 
       dtm1P->numPoints = node1 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"After Removing Deleted ** dtm1P->numPoints = %8ld",dtm1P->numPoints) ;
/*
**     Free Nodes memory
*/
       if( dtm1P->nodesPP != NULL ) 
         { 
          for( m = 0 ; m < dtm1P->numPointPartitions ; ++m ) free(dtm1P->nodesPP[m]) ;
          free( dtm1P->nodesPP) ;
          dtm1P->nodesPP = NULL  ; 
          dtm1P->numNodePartitions = 0 ;
          dtm1P->numNodes = 0 ;
          dtm1P->memNodes = 0 ;
         }
/*
**     Free Circular List Memory
*/
       if( dtm1P->cListPP != NULL ) 
         { 
          for( m = 0 ; m < dtm1P->numClistPartitions ; ++m ) free(dtm1P->cListPP[m]) ;
          free( dtm1P->cListPP) ;
          dtm1P->cListPP = NULL  ; 
          dtm1P->numClistPartitions = 0 ;
          dtm1P->numClist = 0 ;
          dtm1P->memClist = 0 ;
          dtm1P->cListPtr = 0 ;
          dtm1P->cListDelPtr = dtm1P->nullPtr ;
         }
/*
**     Reset DTM Header Values
*/
       dtm1P->dtmState        = DTMState::Data ;
       dtm1P->hullPoint       = dtm1P->nullPnt ;
       dtm1P->nextHullPoint   = dtm1P->nullPnt ;
       dtm1P->numSortedPoints = 0 ;
       dtm1P->numLines        = 0 ;
       dtm1P->numTriangles    = 0 ;
      }
/*
**  Store Filtered Points In DTM Object
*/
    if( dtm2P == NULL ) if( bcdtmObject_createDtmObject(&dtm2P)) goto errexit ;
    for( point = 0 ; point <  dtm1P->numPoints ; ++point )
      {
       pointP = pointAddrP(dtm1P,point) ;
       randomSpot.x = pointP->x ;
       randomSpot.y = pointP->y ;
       randomSpot.z = pointP->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtm2P,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,&randomSpot,1)) goto errexit ;
      }
/*
**  Destroy DTM Object
*/
    if( dtm1P != NULL ) bcdtmObject_destroyDtmObject(&dtm1P) ;
   }
/*
** Write Out Number Of Unfiltered Points
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"Before Reinsert ** Number Of Unfiltered Points = %8ld",dtm2P->numPoints) ;
/*
**  Reinsert Points Out Of z Tolerance Range
*/
 if( reinsertOption == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reinserting Points Outside Of z Tolerance") ;
    dtm2P->edgeOption = 1 ;
    if( bcdtmObject_triangulateDtmObject(dtm2P)) goto errexit ;
    if( bcdtmObject_createDtmObject(&dtm1P)) goto errexit ;
/*
**  Scan Tile Points And Drape On Tin
*/
    numReinserted = 0 ;
    for( n = 0 ; n < numTiles ; ++n )
      {
       for( point = *(tileOffsetP+n) ; point < *(tileOffsetP+n) + *(numTilePtsP+n) ; ++point )
         {
          pointP = pointAddrP(dtmP,point) ;
          if( bcdtmFind_triangleForPointDtmObject(dtm2P,pointP->x,pointP->y,&z,&fndType,&tinPnt1,&tinPnt2,&tinPnt3 )) goto errexit ;
          if( ! fndType || fabs(z-pointP->z) > zTolerance )
            {
             ++numReinserted ;
             randomSpot.x = pointP->x ;
             randomSpot.y = pointP->y ;
             randomSpot.z = pointP->z ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,&randomSpot,1)) goto errexit ;
            }
         }
      }
/*
**  Change State DTM Object 2
*/
    if( bcdtmObject_changeStateDtmObject(dtm2P,DTMState::Data)) goto errexit ; 
/*
**  Store Reinserted Point In DTM Object 2
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Reinserted Points") ;
    for( point = 0 ; point <  dtm1P->numPoints ; ++point )
      {
       pointP = pointAddrP(dtm1P,point) ;
       randomSpot.x = pointP->x ;
       randomSpot.y = pointP->y ;
       randomSpot.z = pointP->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtm2P,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,&randomSpot,1)) goto errexit ;
      }
/*
**  Destroy DTM Object
*/
    if( bcdtmObject_destroyDtmObject(&dtm1P)) goto errexit ; 
/*
**  Write number Of Points Reinserted
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points Reinserted = %8ld",numReinserted) ;
   }
/*
** Write Out Number Of Unfiltered Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"After   Reinsert ** Number Of Unfiltered Points = %8ld",dtm2P->numPoints) ;
/*
**  Set Number Of Spots
*/
 *numFilteredSpotsP = dtm2P->numPoints ;  
/*
** Set Number Of Points In Start Object
*/
 dtmP->numPoints = 0 ;
/*
** Store Group Spots
*/
 for( point = 0 ; point <  dtm2P->numPoints ; ++point )
   {
    pointP = pointAddrP(dtm2P,point) ;
    randomSpot.x = pointP->x ;
    randomSpot.y = pointP->y ;
    randomSpot.z = pointP->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&featureId,&randomSpot,1)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dtm1P != NULL ) bcdtmObject_destroyDtmObject(&dtm1P) ;
 if( dtm2P != NULL ) bcdtmObject_destroyDtmObject(&dtm2P) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering Single Point Group Spot Tiles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering Single Point Group Spot Tiles Error") ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_tileFilterRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object                         */
 long   minTilePts,                    /* ==> Minimum Tile Points                           */
 long   maxTileDivide,                 /* ==> Max Tile Divide                               */
 double tileLength,                    /* ==> Length Of Tile Side                           */
 double zTolerance,                    /* ==> z Filter Tolerance                            */
 long   *numSpotsBeforeFilterP,        /* <== Number Of Spots Before Filter                 */
 long   *numSpotsAfterFilterP,         /* <== Number Of Spots After Filter                  */ 
 double *filterReductionP              /* <== Filter Reduction                              */ 
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long point,firstPoint,lastPoint ;
 DPoint3d  randomSpot;
 DPoint3d *pointP ;
 BC_DTM_OBJ *filterDtmP=NULL ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tile Filtering Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"minTilePts     = %8ld",minTilePts) ;
    bcdtmWrite_message(0,0,0,"maxTileDivide  = %8ld",maxTileDivide) ;
    bcdtmWrite_message(0,0,0,"tileLength     = %8.2lf",tileLength) ;
    bcdtmWrite_message(0,0,0,"zTolerance     = %8.3lf",zTolerance) ;
   }
/*
** Initialise
*/
 *filterReductionP = 0.0 ;
 *numSpotsBeforeFilterP = 0 ;
 *numSpotsAfterFilterP  = 0 ;
/*
** Validate Parameters
*/
 if( zTolerance <= 0.0 )
   {
    bcdtmWrite_message(1,0,0,"Filter z Tolerance Must Be Greater Than Zero") ;
    goto errexit ;
   }
 if( minTilePts <  4  ) minTilePts =  4 ;
 if( minTilePts > 10  ) minTilePts = 10 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Log DTM Stats
*/
 if( dbg ) bcdtmObject_reportStatisticsDtmObject(dtmP) ;
/*
** If DTM In Triangulated State Change It To Untriangulated State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
   }
/*
** Check DTM Is In Untriangulated State
*/
  if( dtmP->dtmState != DTMState::Data )
    {
     bcdtmWrite_message(2,0,0,"Method Requires Untriangulated DTM") ;
     goto errexit ;
    }
/*
** Log DTM Stats
*/
 if( dbg ) bcdtmObject_reportStatisticsDtmObject(dtmP) ;
/*
** Place Random Spots In One Contiguous DTM Block
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Blocking Random Spots") ;
 if( bcdtmFilter_blockRandomSpotsDtmObject(dtmP,&firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %8ld lastPoint = %8ld",firstPoint,lastPoint) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld",dtmP->numPoints) ;
 *numSpotsBeforeFilterP = *numSpotsAfterFilterP = lastPoint - firstPoint + 1  ;
/*
** Log DTM Stats
*/
 if( dbg ) bcdtmObject_reportStatisticsDtmObject(dtmP) ;
/*
** Tile Filter Random Spots
*/
 if( lastPoint - firstPoint >  10 )
   {
    if( bcdtmFilter_tileFilterBlockRandomSpotsDtmObject(dtmP,&filterDtmP,firstPoint,lastPoint,zTolerance,minTilePts,maxTileDivide,tileLength)) goto errexit ;
    *numSpotsAfterFilterP = filterDtmP->numPoints ;
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"dtmP->numPoints       = %8ld",dtmP->numPoints) ;
       bcdtmWrite_message(0,0,0,"filterDtmP->numPoints = %8ld",filterDtmP->numPoints) ;
      }
/*
**  Replace Random Spots In Dtm Object With Filtered Spots
*/
    dtmP->numPoints = firstPoint ;
    for( point = 0 ; point <  filterDtmP->numPoints ; ++point )
      {
       pointP = pointAddrP(filterDtmP,point) ;
       randomSpot.x = pointP->x ;
       randomSpot.y = pointP->y ;
       randomSpot.z = pointP->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,&randomSpot,1)) goto errexit ;
      }
/*
**  Destroy Filter Dtm Object
*/
    if( filterDtmP != NULL ) bcdtmObject_destroyDtmObject(&filterDtmP) ;
/*
**  Convert Features Back From Point Array State To Data State
*/
    if( bcdtmFilter_setFeatureStatesToFirstPointDtmObject(dtmP)) goto errexit ;
   }
/*
** Log DTM Stats
*/
 if( dbg ) bcdtmObject_reportStatisticsDtmObject(dtmP) ;
/*
** Set Return Values
*/ 
 if( *numSpotsAfterFilterP == *numSpotsBeforeFilterP ) *filterReductionP = 0.0 ;
 else                                                  *filterReductionP = ( 1.0 - ( double ) *numSpotsAfterFilterP / ( double ) *numSpotsBeforeFilterP ) * 100.0 ;
/*
** Write Out Results
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"spotsBefore = %8ld spotsAfter = %8ld reduction = %8.4lf",*numSpotsBeforeFilterP,*numSpotsAfterFilterP,*filterReductionP) ;
//    bcdtmObject_triangulateDtmObject(dtmP) ;
    bcdtmWrite_toFileDtmObject(dtmP,L"tileFiltered.bcdtm") ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( filterDtmP != NULL ) bcdtmObject_destroyDtmObject(&filterDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Filtering Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Filtering Random Spots Error") ;
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
BENTLEYDTM_Private int bcdtmFilter_tileFilterBlockRandomSpotsDtmObject
(
 BC_DTM_OBJ   *dtmP,               /* ==> Pointer To Dtm Object With Spots To Be Filtered */
 BC_DTM_OBJ   **filterDtmPP,       /* <== Pointer To Dtm Object To Store Filtered Spots   */
 long         firstPoint,          /* ==> Index Of First Spot In Dtm Object               */
 long         lastPoint,           /* ==> Index Of Last Spot In Dtm Object                */ 
 double       zTolerance,          /* ==> z Filter Tolerance                              */
 int          minTilePts,          /* ==> Minimum Points Per Tile                         */
 int          maxTileDivisions,    /* ==> Maximum Number Of Tile Subdivisisons            */ 
 double       tileLength           /* ==> Length Of Tile Side                             */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long slabNum=0,saveLastPoint ;
 int nTemp = 0;
 long iXL = 0, iXR = 0;
 long iYL = 0, iYR = 0;
 double dXL   = 0.0, dXM   = 0.0, dXR = 0.0 ;
 double dYL   = 0.0, dYM   = 0.0, dYR = 0.0 ;
 double dXMin = 0.0, dXMax = 0.0;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tile Filtering Random Spots Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterDtmP       = %p",*filterDtmPP) ;
    bcdtmWrite_message(0,0,0,"firstPoint       = %8ld",firstPoint) ;
    bcdtmWrite_message(0,0,0,"lastPoint        = %8ld",lastPoint) ;
    bcdtmWrite_message(0,0,0,"zTolerance       = %8.2lf",zTolerance) ;
    bcdtmWrite_message(0,0,0,"minTilePts       = %8ld",minTilePts) ;
    bcdtmWrite_message(0,0,0,"maxTileDivisions = %8ld",maxTileDivisions) ;
    bcdtmWrite_message(0,0,0,"tileLength       = %8.2lf",tileLength) ;
   }
/*
** Sort Points On x Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Random Spots") ;
 if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
/*
** Create DTM Object To Store Filtered Points
*/
 if( *filterDtmPP != NULL ) if( bcdtmObject_destroyDtmObject(filterDtmPP)) goto errexit ;
 if( bcdtmObject_createDtmObject(filterDtmPP)) goto errexit ;
/*
** Initialize Filtering Parameters
*/
 iXL   = firstPoint ;
 dXMin = pointAddrP(dtmP,firstPoint)->x;
 nTemp = ( long ) ( ( pointAddrP(dtmP,lastPoint)->x - dXMin ) / tileLength );
 dXMax = dXMin + tileLength * ( nTemp + 1 );
 dXL   = dXMin;
/*
** Core Filtering Algorithm
*/
 while( dXL < dXMax )
   {
//  the next XSlab.
    ++slabNum ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing XSlab %8ld of %8ld ** Points = %10ld ",slabNum,nTemp+1,(*filterDtmPP)->numPoints) ;

    dXR = dXL + tileLength;
    dXM = dXL + 0.5 * ( dXR - dXL );

//  count all points in current XSlab [dXL,dXR].

    iXR = 0;

    while( dXL <= pointAddrP(dtmP,iXL+iXR)->x && pointAddrP(dtmP,iXL+iXR)->x < dXR && (iXL+iXR) < lastPoint ) iXR++ ;

    if( dbg ) bcdtmWrite_message(0,0,0,"Processing XSlab %8ld of %8ld ** numTilePts = %8ld",slabNum,nTemp+1,iXR) ;

    if( iXR >= minTilePts )
      {
//     a minimum number of slab points was found - the XSlab can be processed.

//     sort XSlab points w/r to y. The points are not needed after the slab has been processed.
//     we now have access to (iXR) points in the index range [iXL,iXR-1].

       if( bcdtmFilter_sortPointRangeDtmObject(dtmP,iXL,iXL+iXR-1,DTM_Y_AXIS)) goto errexit ;

//     subdivide 1D XSlab into 2D sub-slab(s) : [dXL,dXR]x[dYL,dYR] with [dXM,dYM] being center.
//     the entire 1D XSlab is processed in tiles moving in the y-direction.

       dYL = pointAddrP(dtmP,iXL)->y;
       dYR = dYL + tileLength;
       dYM = dYL + 0.5 * ( dYR - dYL );

//     iXL is now the index of the point with the least most y-value.
//     start accessing the y-sorted points at this index.

       iYL = iXL;

       while( iYL < (iXL + iXR) )
         {
//        count all points in current sub-slab ("tile") [dXL,dXR]x[dYL,dYR].

          iYR = 0;

          while( dYL <= pointAddrP(dtmP,iYL+iYR)->y && pointAddrP(dtmP,iYL+iYR)->y < dYR && iYR < iXR && (iYL+iYR) < lastPoint ) iYR++;

//        process the tile.

          bcdtmFilter_processTileDtmObject(dtmP,*filterDtmPP,iYL,iYR,dXL,dXM,dXR,dYL,dYM,dYR,zTolerance,0,maxTileDivisions, minTilePts );

//        prepare next tile.

          iYL += iYR;
          dYL  = dYR;
          dYR += tileLength;
          dYM  = dYL + 0.5 * ( dYR - dYL );
         }
      }

 //   prepare the next XSlab.

    iXL += iXR;
    dXL  = dXR;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Filtering Random Spots Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Filtering Random Spots Dtm Object Error") ;
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
BENTLEYDTM_Private void  bcdtmFilter_processTileDtmObject            
(
 BC_DTM_OBJ* dtmP,               // all (sorted) points in current SlabX.
 BC_DTM_OBJ* filterDtmP,         // the thinned points.
 long iYL,                       // left most index into dtmP.
 long iYR,                       // number of indices AFTER iYL (not a true index - more of a count.)
 double dXL,                     // the tile perimiter is [dXL,dXR]x[dYL,dYR] with midpoint (dXM,dYM)
 double dXM,                     // the tile perimiter is [dXL,dXR]x[dYL,dYR] with midpoint (dXM,dYM)
 double dXR,                     // the tile perimiter is [dXL,dXR]x[dYL,dYR] with midpoint (dXM,dYM)
 double dYL,                     // the tile perimiter is [dXL,dXR]x[dYL,dYR] with midpoint (dXM,dYM)
 double dYM,                     // the tile perimiter is [dXL,dXR]x[dYL,dYR] with midpoint (dXM,dYM)
 double dYR,                     // the tile perimiter is [dXL,dXR]x[dYL,dYR] with midpoint (dXM,dYM)
 double zTolerance,              // z-tolerance to be satisfied in a tile.
 int iDivision,                  // current tile sub-division.
 int maxTileDivisions,           // maximum number of tile sub-divisions.
 int minTilePts                  // minimum number of points in a tile.
)
{
 int dbg=DTM_TRACE_VALUE(0) ;
 int iY=0;
 double dZ = 0.0;
 double dZMin = +DTM_INFINITE;
 double dZMax = -DTM_INFINITE;

 long iYMin   = 0;
 double dDMin = +DTM_INFINITE;
 double dNorm = 0.0;

 long iXR1 = 0;              // number of sub-tile points found.
 double dXL1, dXR1, dXM1;    // sub-tile [dXL1,dXR1]x[dYL1,dYR1] with midpoint (dXM1,dYM1).
 double dYL1, dYR1, dYM1;    // sub-tile [dXL1,dXR1]x[dYL1,dYR1] with midpoint (dXM1,dYM1).

 long iniPts = 0 ;
 long point ;
 DPoint3d *pointP ;
 BC_DTM_OBJ  *tempFilterP=NULL ;  // all (sorted) points in current SlabX.
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Filter Processing Tile") ;
    bcdtmWrite_message(0,0,0,"dtmP                  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterDtmP            = %p",filterDtmP) ;
    bcdtmWrite_message(0,0,0,"filterDtmP->numPoints = %8ld",filterDtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"iYL                   = %8ld",iYL) ;
    bcdtmWrite_message(0,0,0,"iYR                   = %8ld",iYR) ;
    bcdtmWrite_message(0,0,0,"iDivision             = %8ld",iDivision) ;
    bcdtmWrite_message(0,0,0,"zTolerance            = %8.4lf",zTolerance) ;
   }
/*
** Check Number Of Tile Points Is Greater Than Or Equal To The Minimum Number Of Tile Points
*/
 if( iYR >= minTilePts )
   {
    int sts = DTM_SUCCESS;
    DTM_PLANE plane; memset( &plane, 0, sizeof( DTM_PLANE ) );
/*
**  Calculate Least Squares Plane Through The Tile Points
*/
    sts = bcdtmFilter_findPlaneDtmObject(dtmP,iYL,iYR,&plane );
    for( iY = iYL; iY < iYL + iYR; iY++ )
      {
       if( sts == DTM_SUCCESS )
         {
          dZ = pointAddrP(dtmP,iY)->z - ( plane.A * pointAddrP(dtmP,iY)->x + plane.B * pointAddrP(dtmP,iY)->y + plane.C );
          dZMin = ( dZ < dZMin ) ? dZ : dZMin;
          dZMax = ( dZ > dZMax ) ? dZ : dZMax;
         }
       else
         {
          dZMin = ( pointAddrP(dtmP,iY)->z < dZMin ) ? pointAddrP(dtmP,iY)->z : dZMin;
          dZMax = ( pointAddrP(dtmP,iY)->z > dZMax ) ? pointAddrP(dtmP,iY)->z : dZMax;
         }
      } 
/*
**  All Tile Points Are Within The z Tolerance
*/
    if( dZMax - dZMin <= zTolerance )
      {
/*
**     A single point will now represent all the points in the tile [dXL,dXR]x[dYL,dYR].
**     Choose the point closet to the tile midpoint (dXM,dYM).
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"All Tile Points Within z Tolerance") ;
       if( dbg == 1 )
         {
          for( point = 0 ; point < iYR ; ++point )
            {
             pointP = pointAddrP(dtmP,point) ;
             bcdtmWrite_message(0,0,0,"point[%8ld] = %12.5lf %12.5lf %12.5lf",point,pointP->x,pointP->y,pointP->z) ;
            }
         } 
       for( iY = iYL; iY < iYL + iYR; iY++ )
         {
          dNorm = ( pointAddrP(dtmP,iY)->x - dXM ) * ( pointAddrP(dtmP,iY)->x - dXM ) + ( pointAddrP(dtmP,iY)->y - dYM ) * ( pointAddrP(dtmP,iY)->y - dYM );
          if( dNorm < dDMin )
            {
             iYMin = iY;
             dDMin = dNorm;
            }
         }
       bcdtmObject_storeDtmFeatureInDtmObject(filterDtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(dtmP,iYMin),1) ;
      }
/*
**  Subdivide Tile If Max Tile Divisions Has Not Been Exceede
*/
    else if( iDivision < maxTileDivisions )
      {
       bcdtmObject_createDtmObject(&tempFilterP) ;
       if( tempFilterP )
         {
          iniPts = iYR / 2 ;
          if( iniPts < 1000 ) iniPts = 1000 ; 
          bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempFilterP,iniPts,iniPts) ; 
/*
**        Bottom Left Sub Tile.
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"**** Bottom Left Sub Tile") ;
          dXL1 = dXL ; dXR1 = dXM ; dXM1 = dXL1 + 0.5 * ( dXR1 - dXL1 );
          dYL1 = dYL ; dYR1 = dYM ; dYM1 = dYL1 + 0.5 * ( dYR1 - dYL1 );
          iXR1 = 0;
          tempFilterP->numPoints = 0 ;
          for( iY = iYL; iY < iYL + iYR; iY++ )
            {
             if( dXL1 <= pointAddrP(dtmP,iY)->x && pointAddrP(dtmP,iY)->x < dXR1 && dYL1 <= pointAddrP(dtmP,iY)->y && pointAddrP(dtmP,iY)->y < dYR1 )
               { 
                ++iXR1 ;
                bcdtmObject_storeDtmFeatureInDtmObject(tempFilterP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(dtmP,iY),1) ;
               }
            }
          bcdtmFilter_processTileDtmObject(tempFilterP, filterDtmP, 0, iXR1, dXL1, dXM1, dXR1, dYL1, dYM1, dYR1, zTolerance, iDivision + 1, maxTileDivisions, minTilePts );
/*
**        Bottom Right Sub Tile.
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"**** Bottom Right Sub Tile") ;
          dXL1 = dXM; dXR1 = dXR; dXM1 = dXL1 + 0.5 * ( dXR1 - dXL1 );
          dYL1 = dYL; dYR1 = dYM; dYM1 = dYL1 + 0.5 * ( dYR1 - dYL1 );
          iXR1 = 0;
          tempFilterP->numPoints = 0 ;
          for( iY = iYL; iY < iYL + iYR; iY++ )
            {
             if( dXL1 <= pointAddrP(dtmP,iY)->x && pointAddrP(dtmP,iY)->x < dXR1 && dYL1 <= pointAddrP(dtmP,iY)->y && pointAddrP(dtmP,iY)->y < dYR1 )
               { 
                ++iXR1 ;
                bcdtmObject_storeDtmFeatureInDtmObject(tempFilterP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(dtmP,iY),1) ;
               }
            }
          bcdtmFilter_processTileDtmObject(tempFilterP, filterDtmP, 0, iXR1, dXL1, dXM1, dXR1, dYL1, dYM1, dYR1, zTolerance, iDivision + 1, maxTileDivisions, minTilePts );
/*
**        Top Left Sub Tile.
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"**** Top Left Sub Tile") ;
          dXL1 = dXL; dXR1 = dXM; dXM1 = dXL1 + 0.5 * ( dXR1 - dXL1 );
          dYL1 = dYM; dYR1 = dYR; dYM1 = dYL1 + 0.5 * ( dYR1 - dYL1 );
          iXR1 = 0;
          tempFilterP->numPoints = 0 ;
          for( iY = iYL; iY < iYL + iYR; iY++ )
            {
             if( dXL1 <= pointAddrP(dtmP,iY)->x && pointAddrP(dtmP,iY)->x < dXR1 && dYL1 <= pointAddrP(dtmP,iY)->y && pointAddrP(dtmP,iY)->y < dYR1 )
               { 
                ++iXR1 ;
                bcdtmObject_storeDtmFeatureInDtmObject(tempFilterP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(dtmP,iY),1) ;
               }
            } 
          bcdtmFilter_processTileDtmObject(tempFilterP, filterDtmP, 0, iXR1, dXL1, dXM1, dXR1, dYL1, dYM1, dYR1, zTolerance, iDivision + 1, maxTileDivisions, minTilePts );
/*
**        Top Right Sub Tile.
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"**** Top Right Sub Tile") ;
          dXL1 = dXM ; dXR1 = dXR ; dXM1 = dXL1 + 0.5 * ( dXR1 - dXL1 );
          dYL1 = dYM ; dYR1 = dYR ; dYM1 = dYL1 + 0.5 * ( dYR1 - dYL1 );
          iXR1 = 0;
          tempFilterP->numPoints = 0 ;
          for( iY = iYL; iY < iYL + iYR; iY++ )
            {
             if( dXL1 <= pointAddrP(dtmP,iY)->x && pointAddrP(dtmP,iY)->x < dXR1 && dYL1 <= pointAddrP(dtmP,iY)->y && pointAddrP(dtmP,iY)->y < dYR1 )
               {
                ++iXR1 ;
                bcdtmObject_storeDtmFeatureInDtmObject(tempFilterP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(dtmP,iY),1) ;
               }
            } 
          bcdtmFilter_processTileDtmObject(tempFilterP, filterDtmP, 0, iXR1, dXL1, dXM1, dXR1, dYL1, dYM1, dYR1, zTolerance, iDivision + 1, maxTileDivisions, minTilePts );
          bcdtmObject_destroyDtmObject(&tempFilterP) ;
         }
      }
/*
**  Maximum Sub Divisions Exceed
*/
    else
      {
       for( iY = iYL; iY < iYL + iYR; iY++ )
         {
          if( bcdtmObject_storeDtmFeatureInDtmObject(filterDtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(dtmP,iY),1) )
            {
             bcdtmWrite_message(0,0,0,"Store Error 1 With filterDtmP = %p",filterDtmP) ;
            }
         }
      }
   }
/*
**  The Tile Is Empty Or There Is not Enough Points To Further Tile
*/
 else
   {
    if( iYR > 0 )
      {
       for( iY = iYL; iY < iYL + iYR; iY++ )
         {
          bcdtmObject_storeDtmFeatureInDtmObject(filterDtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(dtmP,iY),1) ;
         }
      }
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFilter_findPlaneDtmObject
(
 BC_DTM_OBJ *dtmP, 
 long pnt,
 long nPnts,
 DTM_PLANE* pPlane
)
{
 int sts = DTM_SUCCESS;

 long i = 0;
 double A[9] = {0.0};
 double b[3] = {0.0};
 DPoint3d pntC;
 DPoint3d *pntP ;

 pPlane->A = 0.0 ;
 pPlane->B = 0.0 ;
 pPlane->C = 0.0 ;

 memset( &pntC, 0, sizeof( DPoint3d ) );

 for( i = 0; i < nPnts; i++ )
   {
    pntP = pointAddrP(dtmP,pnt+i) ;
    pntC.x += pntP->x ;
    pntC.y += pntP->y ;
    pntC.z += pntP->z ;
   }

 pntC.x /= nPnts;
 pntC.y /= nPnts;
 pntC.z /= nPnts;

 for( i = 0; i < nPnts; i++ )
   {
//  Translated plane.
    pntP = pointAddrP(dtmP,pnt+i) ;
    A[0] += ( pntP->x - pntC.x ) * ( pntP->x - pntC.x );
    A[1] += ( pntP->x - pntC.x ) * ( pntP->y - pntC.y );
    A[2] += ( pntP->x - pntC.x );

    A[4] += ( pntP->y - pntC.y ) * ( pntP->y - pntC.y );
    A[5] += ( pntP->y - pntC.y );

    b[0] += ( pntP->x - pntC.x ) * ( pntP->z - pntC.z );
    b[1] += ( pntP->y - pntC.y ) * ( pntP->z - pntC.z );
    b[2] += ( pntP->z - pntC.z );
   }

 A[3] = A[1];
 A[6] = A[2];
 A[7] = A[5];
 A[8] = nPnts;

 if( ( sts = bcdtmFilter_gaussJordanPartialPivot( 3, A ) ) == DTM_SUCCESS )
    {
     pPlane->A = A[0] * b[0] + A[1] * b[1] + A[2] * b[2];
     pPlane->B = A[3] * b[0] + A[4] * b[1] + A[5] * b[2];
     pPlane->C = A[6] * b[0] + A[7] * b[1] + A[8] * b[2];

//   Translated plane.
        pPlane->C = pPlane->C + pntC.z - pPlane->A * pntC.x - pPlane->B * pntC.y;
    }

  return sts;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmFilter_gaussJordanPartialPivot
(
    int n,
    double *A
)
{
    int sts = DTM_SUCCESS, i, j, k, r, hi;
    int *p = NULL;
    double *hv = NULL, hr, maxpivot;

    if( ( hv = ( double * )calloc( n, sizeof( double ) ) ) != NULL )
    {
        if( ( p = ( int * )calloc( n, sizeof( int ) ) ) != NULL )
        {
            for( j = 0; j <= n-1; j++ )
                p[j] = j;

            for( j = 0; j <= n-1; j++ )
            {
                maxpivot = fabs( A[j*n+j] );
                r        = j;

                for( i = j+1; i <= n-1; i++ )
                    if( fabs( A[i*n+j] ) > maxpivot )
                    {
                        maxpivot = fabs( A[i*n+j] );
                        r        = i;
                    }

                if( fabs( maxpivot ) <= DTM_TINY )
                    return( 1 );

                if( r > j )
                {
                    for( k = 0; k <= n-1; k++ )
                    {
                        hr       = A[j*n+k];
                        A[j*n+k] = A[r*n+k];
                        A[r*n+k] = hr;
                    }

                    hi   = p[j];
                    p[j] = p[r];
                    p[r] = hi;
                }

                hr = 1.0 / A[j*n+j];

                for( i = 0; i <= n-1; i++ )
                    A[i*n+j] = hr * A[i*n+j];

                A[j*n+j] = hr;

                for( k = 0; k <= n-1; k++ )
                    if( k != j )
                    {
                        for( i = 0; i <= n-1; i++ )
                            if( i != j )
                                A[i*n+k] = A[i*n+k] - A[i*n+j] * A[j*n+k];

                        A[j*n+k] = -hr * A[j*n+k];
                    }
            }
        }
        else
        {
            sts = 1;
        }
    }
    else
    {
        sts = 1;
    }
/*
** Apply permutation matrix to complete the inverse.
*/
    if( sts == DTM_SUCCESS )
        for( i= 0; i <= n-1; i++ )
        {
            for( k = 0; k <= n-1; k++ )
                hv[p[k]] = A[i*n+k];

            for( k = 0; k <= n-1; k++ )
                A[i*n+k] = hv[k];
        }

    if( hv )
    {
        free( hv );
        hv = NULL;
    }

    if( p )
    {
        free( p );
        p = NULL;
    }

    return sts;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFilter_sortPointRangeDtmObject
(
 BC_DTM_OBJ *dtmP,        /* ==> Pointer To Dtm Object             */
 long firstPoint,         /* ==> First Point Of Point Range        */
 long lastPoint,          /* ==> Last Point Of Point Range         */
 long axis                /* ==> Sort Axis <DTM_X_AXIS,DTM_Y_AXIS> */
)
/*
** This Function Sorts A Range Of Points In A DTM Object
** This is special application function and is not be used for other purposes
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   ofs,pnt,numPoints=0 ; 
 long   *sP,*sortP=NULL,*tempP=NULL ;
 double dSave ;
 DPoint3d *pnt1P,*pnt2P,dtmPoint  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Range Sorting Points Dtm Object %p",dtmP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Sort If Dtm Object Is In Dtm State DTMState::Data
*/
 if( dtmP->dtmState != DTMState::Data ) 
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated Dtm") ;
    goto errexit ;
   }
/*
** Check Point Range
*/
 if( firstPoint < 0 || firstPoint >= dtmP->numPoints || lastPoint < 0 || lastPoint >= dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
** Check Point Range
*/
 if( lastPoint <= firstPoint )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   }  
/*
** Initialise
*/
 numPoints = lastPoint - firstPoint + 1 ;
/*
**  Only Sort If More Than One Dtm Point
*/
 if( numPoints > 1 )   
   {
/*
**  Exchange x & y For y Axis
*/
    if( axis == DTM_Y_AXIS )
      {
       for( pnt = firstPoint ; pnt <= lastPoint ; ++pnt )
         {
          pnt1P = pointAddrP(dtmP,pnt) ;  
          dSave = pnt1P->x ;
          pnt1P->x = pnt1P->y ;
          pnt1P->y = dSave ;
         }
      }
/*
**  Allocate Memory For Sort Offset Pointers
*/
   sortP = ( long *  ) malloc(dtmP->numPoints*sizeof(long)) ;
   if( sortP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
   tempP = ( long *  ) malloc(dtmP->numPoints*sizeof(long)) ;
   if( tempP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
/*
** Initialise Sort Offset Pointer
*/
   for( sP = sortP, ofs = 0 ; sP < sortP + dtmP->numPoints ; ++sP, ++ofs) *sP = ofs ;
/*
**  Sort the Dtm Points
*/
   if( dbg ) bcdtmWrite_message(0,0,0,"Sorting") ;
   bcdtmObject_divConqMergeSortDtmObject(dtmP,firstPoint,lastPoint-firstPoint+1,sortP,tempP) ;
/*
** Calculate Dtm Point Sort Position
*/
   for( sP = sortP + firstPoint ; sP <= sortP + lastPoint ; ++sP  ) *(tempP+*sP) = (long)(sP-sortP) ;
/*
** Place Dtm Points In Sort Order
*/
   for( sP = sortP + firstPoint , ofs = firstPoint ; sP <= sortP + lastPoint ; ++sP , ++ofs )
     {
      pnt1P = pointAddrP(dtmP,ofs) ; 
      pnt2P = pointAddrP(dtmP,*sP) ; 
      dtmPoint = *pnt1P ;
      *pnt1P = *pnt2P ;
      *pnt2P = dtmPoint ;
      *(sortP+*(tempP+ofs)) = *sP ;
      *(tempP+*sP) = *(tempP+ofs) ;
     }
/*
**  Exchange x & y For y Axis
*/
    if( axis == DTM_Y_AXIS )
      {
       for( pnt = firstPoint ; pnt <= lastPoint ; ++pnt )
         {
          pnt1P = pointAddrP(dtmP,pnt) ;  
          dSave = pnt1P->x ;
          pnt1P->x = pnt1P->y ;
          pnt1P->y = dSave ;
         }
      }
/*
**  Check Sort Order
*/
    if( cdbg )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Sort Order") ;
       if( axis == DTM_X_AXIS )
         {
          pnt1P = pointAddrP(dtmP,firstPoint) ;
          for( pnt = firstPoint ; pnt <= lastPoint ; ++pnt )
            {
             pnt2P = pointAddrP(dtmP,pnt) ;
             if( pnt2P->x < pnt1P->x || ( pnt2P->x == pnt1P->x && pnt2P->y < pnt1P->y ))
               {
                bcdtmWrite_message(1,0,0,"Dtm Point x Axis Sort Order Error") ;
                goto errexit ;
               }
             pnt1P = pnt2P ;
            }
         }  
       if( axis == DTM_Y_AXIS )
         {
          pnt1P = pointAddrP(dtmP,firstPoint) ;
          for( pnt = firstPoint ; pnt <= lastPoint ; ++pnt )
            {
             pnt2P = pointAddrP(dtmP,pnt) ;
             if( pnt2P->y < pnt1P->y || ( pnt2P->y == pnt1P->y && pnt2P->x < pnt1P->x ))
               {
                bcdtmWrite_message(1,0,0,"Dtm Point y Axis Sort Order Error") ;
                goto errexit ;
               }
             pnt1P = pnt2P ;
            }
         }  
       if( dbg ) bcdtmWrite_message(0,0,0,"Sort Order OK") ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( sortP != NULL ) free(sortP) ;
 if( tempP != NULL ) free(tempP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Range Sorting Points Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Range Sorting Points Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_sortTaggedPointRangeDtmObject
(
 BC_DTM_OBJ *dtmP,        /* ==> Pointer To Dtm Object             */
 long *tagP,              /* ==> Pointer to Tag Array              */
 long firstPoint,         /* ==> First Point Of Point Range        */
 long lastPoint,          /* ==> Last Point Of Point Range         */
 long axis                /* ==> Sort Axis <DTM_X_AXIS,DTM_Y_AXIS> */
)
/*
** This Function Sorts A Range Of Points In A DTM Object
** This is special application function and is not be used for other purposes
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   ofs,pnt,numPoints=0,userTag ; 
 long   *sP,*sortP=NULL,*tempP=NULL ;
 double dSave ;
 DPoint3d *pnt1P,*pnt2P,dtmPoint  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Range Sorting Points Dtm Object %p",dtmP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Sort If Dtm Object Is In Dtm State DTMState::Data
*/
 if( dtmP->dtmState != DTMState::Data ) 
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated Dtm") ;
    goto errexit ;
   }
/*
** Check Point Range
*/
 if( firstPoint < 0 || firstPoint >= dtmP->numPoints || lastPoint < 0 || lastPoint >= dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
** Check Point Range
*/
 if( lastPoint <= firstPoint )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   } 
/*
** Check Tags Are Set
*/
 if( tagP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Null Tag Array") ;
    goto errexit ;
   }    
/*
** Initialise
*/
 numPoints = lastPoint - firstPoint + 1 ;
/*
**  Only Sort If More Than One Dtm Point
*/
 if( numPoints > 1 )   
   {
/*
**  Exchange x & y For y Axis
*/
    if( axis == DTM_Y_AXIS )
      {
       for( pnt = firstPoint ; pnt <= lastPoint ; ++pnt )
         {
          pnt1P = pointAddrP(dtmP,pnt) ;  
          dSave = pnt1P->x ;
          pnt1P->x = pnt1P->y ;
          pnt1P->y = dSave ;
         }
      }
/*
**  Allocate Memory For Sort Offset Pointers
*/
   sortP = ( long *  ) malloc(dtmP->numPoints*sizeof(long)) ;
   if( sortP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
   tempP = ( long *  ) malloc(dtmP->numPoints*sizeof(long)) ;
   if( tempP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
/*
** Initialise Sort Offset Pointer
*/
   for( sP = sortP, ofs = 0 ; sP < sortP + dtmP->numPoints ; ++sP, ++ofs) *sP = ofs ;
/*
**  Sort the Dtm Points
*/
   if( dbg ) bcdtmWrite_message(0,0,0,"Sorting") ;
   bcdtmObject_divConqMergeSortDtmObject(dtmP,firstPoint,lastPoint-firstPoint+1,sortP,tempP) ;
/*
** Calculate Dtm Point Sort Position
*/
   for( sP = sortP + firstPoint ; sP <= sortP + lastPoint ; ++sP  ) *(tempP+*sP) = (long)(sP-sortP) ;
/*
** Place Dtm Points In Sort Order
*/
   for( sP = sortP + firstPoint , ofs = firstPoint ; sP <= sortP + lastPoint ; ++sP , ++ofs )
     {
      pnt1P = pointAddrP(dtmP,ofs) ; 
      pnt2P = pointAddrP(dtmP,*sP) ; 
      dtmPoint = *pnt1P ;
      *pnt1P = *pnt2P ;
      *pnt2P = dtmPoint ;
      userTag = *(tagP+ofs) ;
      *(tagP+ofs) = *(tagP+*sP) ;
      *(tagP+*sP) = userTag ;
      *(sortP+*(tempP+ofs)) = *sP ;
      *(tempP+*sP) = *(tempP+ofs) ;
     }
/*
**  Exchange x & y For y Axis
*/
    if( axis == DTM_Y_AXIS )
      {
       for( pnt = firstPoint ; pnt <= lastPoint ; ++pnt )
         {
          pnt1P = pointAddrP(dtmP,pnt) ;  
          dSave = pnt1P->x ;
          pnt1P->x = pnt1P->y ;
          pnt1P->y = dSave ;
         }
      }
/*
**  Check Sort Order
*/
    if( cdbg )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Sort Order") ;
       if( axis == DTM_X_AXIS )
         {
          pnt1P = pointAddrP(dtmP,firstPoint) ;
          for( pnt = firstPoint ; pnt <= lastPoint ; ++pnt )
            {
             pnt2P = pointAddrP(dtmP,pnt) ;
             if( pnt2P->x < pnt1P->x || ( pnt2P->x == pnt1P->x && pnt2P->y < pnt1P->y ))
               {
                bcdtmWrite_message(1,0,0,"Dtm Point x Axis Sort Order Error") ;
                goto errexit ;
               }
             pnt1P = pnt2P ;
            }
         }  
       if( axis == DTM_Y_AXIS )
         {
          pnt1P = pointAddrP(dtmP,firstPoint) ;
          for( pnt = firstPoint ; pnt <= lastPoint ; ++pnt )
            {
             pnt2P = pointAddrP(dtmP,pnt) ;
             if( pnt2P->y < pnt1P->y || ( pnt2P->y == pnt1P->y && pnt2P->x < pnt1P->x ))
               {
                bcdtmWrite_message(1,0,0,"Dtm Point y Axis Sort Order Error") ;
                goto errexit ;
               }
             pnt1P = pnt2P ;
            }
         }  
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Sort Order Completed") ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( sortP != NULL ) free(sortP) ;
 if( tempP != NULL ) free(tempP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Range Sorting Points Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Range Sorting Points Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_Private int bcdtmFilter_divConqMergeSortDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long numPts )
/*
** This Routine Sorts The Dtm Object By A Combined Divide And Conquer Merging Method
*/
{
 long  i,numPts1,numPts2,startPnt1,startPnt2 ;
 DPoint3d *p1P,*p2P,point ;
/*
** Two data points
*/
 if( numPts == 2 )
   {
    p1P = pointAddrP(dtmP,startPnt) ;
    p2P = pointAddrP(dtmP,startPnt+1) ;
    if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
      {
       point = *p1P ;
       *p1P  = *p2P ;
       *p2P  = point ;
      }
   }
/*
**  More than two data Points
*/
 if( numPts > 2 )
   {
    numPts1 = numPts / 2  ; 
    if( numPts % 2 != 0 ) numPts1 = numPts1 + 1 ; 
    numPts2 = numPts - numPts1 ;
    startPnt1 = startPnt  ; 
    startPnt2 = startPnt + numPts1 ;
    bcdtmFilter_divConqMergeSortDtmObject(dtmP,startPnt1,numPts1) ;
    bcdtmFilter_divConqMergeSortDtmObject(dtmP,startPnt2,numPts2) ;
/*
**  Merge data sets
*/
    p1P = pointAddrP(dtmP,startPnt2-1) ;
    p2P = pointAddrP(dtmP,startPnt2) ;
    if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
      {
       for( i = startPnt ; i < startPnt + numPts ; ++i )
         {
          p1P = pointAddrP(dtmP,startPnt1) ;
          p2P = pointAddrP(dtmP,startPnt2) ;
          if( p1P->x < p2P->x || ( p1P->x == p2P->x && p1P->y <= p2P->y)) ++startPnt1 ;
          else
            {
             point = *p1P ;
             *p1P  = *p2P ;
             *p2P  = point ;
             ++startPnt2 ;
            }
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
BENTLEYDTM_EXPORT int bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(BC_DTM_OBJ *dtmP,long firstPoint,long *lastPointP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   pnt1,pnt2 ; 
 DPoint3d *pnt1P,*pnt2P  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points From Range Dtm Object %p",dtmP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Sort If Dtm Object Is In Dtm State DTMState::Data
*/
 if( dtmP->dtmState != DTMState::Data ) 
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated Dtm") ;
    goto errexit ;
   }
/*
** Check Point Range
*/
 if( firstPoint < 0 || firstPoint >= dtmP->numPoints || *lastPointP < 0 || *lastPointP >= dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
** Check Point Range
*/
 if( *lastPointP <= firstPoint )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   }  
/*
** Remove Duplicates
*/
 pnt1P = pointAddrP(dtmP,firstPoint) ;
 for( pnt1 = pnt2 = firstPoint ; pnt2 <= *lastPointP ; ++pnt2 )
   {
    pnt2P = pointAddrP(dtmP,pnt2) ;
    if( pnt2P->x != pnt1P->x ||  pnt2P->y != pnt1P->y )
      {
       ++pnt1 ;
       pnt1P = pointAddrP(dtmP,pnt1) ;
       if( pnt1P != pnt2P ) *pnt1P = *pnt2P ;
      }
    else if( dbg && pnt1 != pnt2 )
      {
       bcdtmWrite_message(0,0,0,"Duplicate Points") ;
       bcdtmWrite_message(0,0,0,"pnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",pnt1,pnt1P->x,pnt1P->y,pnt1P->z) ;
       bcdtmWrite_message(0,0,0,"pnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",pnt2,pnt2P->x,pnt2P->y,pnt2P->z) ;
      }
   }
/*
** Reset Last Point
*/
 *lastPointP = pnt1 ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points From Range Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points From Range Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_removeTaggedDuplicatePointsFromRangeDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long firstPoint,long *lastPointP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   pnt1,pnt2 ; 
 DPoint3d *pnt1P,*pnt2P  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points From Range Dtm Object %p",dtmP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Sort If Dtm Object Is In Dtm State DTMState::Data
*/
 if( dtmP->dtmState != DTMState::Data ) 
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated Dtm") ;
    goto errexit ;
   }
/*
** Check Point Range
*/
 if( firstPoint < 0 || firstPoint >= dtmP->numPoints || *lastPointP < 0 || *lastPointP >= dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
** Check Point Range
*/
 if( *lastPointP <= firstPoint )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   }  
/*
** Remove Duplicates
*/
 pnt1P = pointAddrP(dtmP,firstPoint) ;
 for( pnt1 = pnt2 = firstPoint ; pnt2 <= *lastPointP ; ++pnt2 )
   {
    pnt2P = pointAddrP(dtmP,pnt2) ;
    if( pnt2P->x != pnt1P->x || pnt2P->y != pnt1P->y )
      {
       ++pnt1 ;
       *(tagP+pnt1) = *(tagP+pnt2) ;
       pnt1P = pointAddrP(dtmP,pnt1) ;
       if( pnt1P != pnt2P ) *pnt1P = *pnt2P ;
      }
    else if( dbg && pnt1 != pnt2 )
      {
       bcdtmWrite_message(0,0,0,"Duplicate Points") ;
       bcdtmWrite_message(0,0,0,"pnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",pnt1,pnt1P->x,pnt1P->y,pnt1P->z) ;
       bcdtmWrite_message(0,0,0,"pnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",pnt2,pnt2P->x,pnt2P->y,pnt2P->z) ;
      }
   }
/*
** Reset Last Point
*/
 *lastPointP = pnt1 ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points From Range Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points From Range Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_tileFilterSinglePointGroupSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object                         */
 long   minTilePts,                    /* ==> Minimum Tile Points                           */
 long   maxTileDivide,                 /* ==> Max Tile Divide                               */
 double tileLength,                    /* ==> Length Of Tile Side                           */
 double zTolerance,                    /* ==> z Filter Tolerance                            */
 long   *numSpotsBeforeFilterP,        /* <== Number Of Spots Before Filter                 */
 long   *numSpotsAfterFilterP,         /* <== Number Of Spots After Filter                  */ 
 double *filterReductionP              /* <== Filter Reduction                              */ 
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long point,firstPoint,lastPoint,dtmFeature ;
 DPoint3d  randomSpot ;
 DPoint3d *pointP ;
 BC_DTM_OBJ *filterDtmP=NULL ;
 DTMFeatureId featureId ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tile Filtering Single Point Group Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"minTilePts     = %8ld",minTilePts) ;
    bcdtmWrite_message(0,0,0,"maxTileDivide  = %p",maxTileDivide) ;
    bcdtmWrite_message(0,0,0,"tileLength     = %8.2lf",tileLength) ;
    bcdtmWrite_message(0,0,0,"zTolerance     = %8.3lf",zTolerance) ;
   }
/*
** Initialise
*/
 *filterReductionP = 0.0 ;
 *numSpotsBeforeFilterP = 0 ;
 *numSpotsAfterFilterP  = 0 ;
/*
** Validate Parameters
*/
 if( zTolerance <= 0.0 )
   {
    bcdtmWrite_message(1,0,0,"Filter z Tolerance Must Be Greater Than Zero") ;
    goto errexit ;
   }
 if( minTilePts <  4  ) minTilePts =  4 ;
 if( minTilePts > 10  ) minTilePts = 10 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** If DTM In Triangulated State Change It To Untriangulated State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Changing DTM State To Points Unsorted ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
    if( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Changing DTM State To Points Unsorted Completed ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
   }
/*
** Check DTM Is In Untriangulated State
*/
  if( dtmP->dtmState != DTMState::Data )
    {
     bcdtmWrite_message(2,0,0,"Method Requires Untriangulated DTM") ;
     goto errexit ;
    }
/*
** Change Single Point Group Spots To Random Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Changing Single Point Group Spots To Random Spots") ;
 if( bcdtmFilter_changeSinglePointGroupSpotsToRandomSpotsObject(dtmP)) goto errexit ;
/*
** Place Random Spots In One Contiguous DTM Block
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Blocking Random Spots") ;
 if( bcdtmFilter_blockRandomSpotsDtmObject(dtmP,&firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %8ld lastPoint = %8ld",firstPoint,lastPoint) ;
 *numSpotsBeforeFilterP = *numSpotsAfterFilterP = lastPoint - firstPoint + 1  ;
/*
** Tile Filter Random Spots
*/
 if( lastPoint - firstPoint >  10 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Tile Filtering Random Spots") ;
    if( bcdtmFilter_tileFilterBlockRandomSpotsDtmObject(dtmP,&filterDtmP,firstPoint,lastPoint,zTolerance,minTilePts,maxTileDivide,tileLength)) goto errexit ;
    *numSpotsAfterFilterP = filterDtmP->numPoints ;
/*
**  Replace Random Spots In Dtm Object With Filtered Spots
*/
    dtmP->numPoints = firstPoint ;
    for( point = 0 ; point <  filterDtmP->numPoints ; ++point )
      {
       pointP = pointAddrP(filterDtmP,point) ;
       randomSpot.x = pointP->x ;
       randomSpot.y = pointP->y ;
       randomSpot.z = pointP->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::GroupSpots,DTM_NULL_USER_TAG,3,&featureId,&randomSpot,1)) goto errexit ;
      }
/*
**  Destroy Filter Dtm Object
*/
    if( filterDtmP != NULL ) bcdtmObject_destroyDtmObject(&filterDtmP) ;
   }
/*
** Set Return Values
*/ 
 if( *numSpotsAfterFilterP == *numSpotsBeforeFilterP ) *filterReductionP = 0.0 ;
 else                                                  *filterReductionP = ( 1.0 - ( double ) *numSpotsAfterFilterP / ( double ) *numSpotsBeforeFilterP ) * 100.0 ;
/*
** Set Feature States Back To First Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resetting Feature States To First Point") ;
 if( bcdtmFilter_setFeatureStatesToFirstPointDtmObject(dtmP)) goto errexit ;
/*
** Write Out Filter Results
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"spotsBefore = %8ld spotsAfter = %8ld reduction = %8.4lf",*numSpotsBeforeFilterP,*numSpotsAfterFilterP,*filterReductionP) ;
    if( dbg == 1 )
      {
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots )
            {
             bcdtmWrite_message(0,0,0,"dtmfeatureType = DTMFeatureType::GroupSpots ** dtmFeatureId = %10I64d ** numDtmFeaturePts = %8ld",dtmFeatureP->dtmFeatureId,dtmFeatureP->numDtmFeaturePts) ;
             if( dtmFeatureP->dtmFeatureId == dtmP->nullFeatureId )
               {
                bcdtmWrite_message(0,0,0,"**** Error - Null Feature Id") ;
               }
            }
         }
      } 
   } 
/*
**  Update Modified Time
 */
 bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
 if( filterDtmP != NULL ) bcdtmObject_destroyDtmObject(&filterDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Filtering Single Point Group Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Filtering Single Point Group Spots Error") ;
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
BENTLEYDTM_Private int bcdtmFilter_changeSinglePointGroupSpotsToRandomSpotsObject(BC_DTM_OBJ *dtmP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   dtmFeature,numDeleted ; 
 BC_DTM_FEATURE *dtmFeatureP  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Changing Single Point Group Spots To Random Spots") ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Change If Dtm Object Is In Dtm State DTMState::Data
*/
 if( dtmP->dtmState != DTMState::Data ) 
   {
    bcdtmWrite_message(2,0,0,"Method Requires Untriangulated Dtm") ;
    goto errexit ;
   }
/*
** Write Number Of Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Scan For Single Point Group Spots
*/
 numDeleted = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots && dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->numDtmFeaturePts == 1 )
      {
       dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
       ++numDeleted ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Single Point Group Spots = %8ld",numDeleted) ;
/*
** Compact Feature Table
*/
 if( numDeleted > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature Table") ;
    if( bcdtmData_compactUntriangulatedFeatureTableDtmObject(dtmP)) goto errexit ;
   } 
/*
** Write Number Of Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Changing Single Point Group Spots To Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Changing Single Point Group Spots To Random Spots Error") ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_tinFilterSinglePointGroupSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object                         */
 long   filterOption,                  /* ==> Filter Option <1= Fine,2=Coarse>              */
 long   reinsertOption,                /* ==> Reinsert Points Out Of Tolerance <TRUE,FALSE> */
 double zTolerance,                    /* ==> Filter z Tolerance > 0.0                      */ 
 long   *numSpotsBeforeFilterP,        /* <== Number Of Spots Before Filter                 */
 long   *numSpotsAfterFilterP,         /* <== Number Of Spots After Filter                  */ 
 double *filterReductionP              /* <== Filter Reduction                              */ 
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ; 
 long n,firstPoint,lastPoint,numTiles,saveLastPoint,maxPts=500000,dtmFeature ;
 long *tileOffsetP=NULL,*numTilePtsP=NULL ;
 char dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin Filtering Single Point Group Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterOption   = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"reinsertOption = %8ld",reinsertOption) ;
    bcdtmWrite_message(0,0,0,"zTolerance     = %8.3lf",zTolerance) ;
   }
/*
** Initialise
*/
 *filterReductionP = 0.0 ;
 *numSpotsBeforeFilterP = 0 ;
 *numSpotsAfterFilterP  = 0 ;
/*
** Validate Parameters
*/
 if( filterOption < 1 || filterOption > 2 ) filterOption = 1 ;
 if( zTolerance <= 0.0 )
   {
    bcdtmWrite_message(1,0,0,"Filter z Tolerance Must Be Greater Than Zero") ;
    goto errexit ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** If DTM In Triangulated State Change It To Untriangulated State
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
   }
/*
** Check DTM Is In Untriangulated State
*/
  if( dtmP->dtmState != DTMState::Data )
    {
     bcdtmWrite_message(2,0,0,"Method Requires Untriangulated DTM") ;
     goto errexit ;
    }
/*
** Change Single Point Group Spots To Random Points
*/
 if( bcdtmFilter_changeSinglePointGroupSpotsToRandomSpotsObject(dtmP)) goto errexit ;
/*
** Place Random Spots In One Contiguous DTM Block
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Blocking Random Spots") ;
 if( bcdtmFilter_blockRandomSpotsDtmObject(dtmP,&firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %8ld lastPoint = %8ld",firstPoint,lastPoint) ;
 *numSpotsBeforeFilterP = *numSpotsAfterFilterP = lastPoint - firstPoint + 1  ;
/*
** Sort Points On x Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Random Spots") ;
 if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
/*
** Tile Random Points
*/
 if( lastPoint - firstPoint > 10 )
   {
/*
**  Create maxPts Tiles
*/
    if( lastPoint - firstPoint > maxPts )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Tiling Random Points") ;
       if( bcdtmSort_pointsIntoTilesDtmObject(dtmP,firstPoint,lastPoint-firstPoint+1,maxPts,&tileOffsetP,&numTilePtsP,&numTiles)) goto errexit ;
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Number Of Tiles = %4ld",numTiles) ; 
          for( n = 0 ; n < numTiles ; ++n )
            {
             bcdtmWrite_message(0,0,0,"Tile[%4ld] ** offset = %8ld ** numPts = %8ld",n+1,*(tileOffsetP+n),*(numTilePtsP+n)) ;
            }
         }
      }
/*
**  Create One Tile
*/
    else
      {
       numTiles = 1 ; 
       tileOffsetP = ( long * ) malloc( numTiles * sizeof(long)) ;
       numTilePtsP = ( long * ) malloc( numTiles * sizeof(long)) ;
       if( tileOffsetP == NULL || numTilePtsP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         } 
       *tileOffsetP = 0 ;
       *numTilePtsP = lastPoint - firstPoint + 1 ; 
      } 
/*
**  Filtering Random Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Random Spot Tiles") ;
    if( bcdtmFilter_filterSinglePointGroupSpotTilesDtmObject(dtmP,filterOption,reinsertOption,zTolerance,tileOffsetP,numTilePtsP,numTiles,numSpotsAfterFilterP)) goto errexit ;
   }
/*
** Set Feature States Back To First Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resetting Feature States To First Point") ;
 if( bcdtmFilter_setFeatureStatesToFirstPointDtmObject(dtmP)) goto errexit ;
/*
** Resize DTM Object
*/
 if( bcdtmObject_resizeMemoryDtmObject(dtmP)) goto errexit ;
/*
** Set Return Values
*/ 
 if( *numSpotsAfterFilterP == *numSpotsBeforeFilterP ) *filterReductionP = 0.0 ;
 else                                                  *filterReductionP = ( 1.0 - ( double ) *numSpotsAfterFilterP / ( double ) *numSpotsBeforeFilterP ) * 100.0 ;
/*
** Write Out Filter Results
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"spotsBefore = %8ld spotsAfter = %8ld reduction = %8.4lf",*numSpotsBeforeFilterP,*numSpotsAfterFilterP,*filterReductionP) ;
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld",dtmP->numPoints) ;
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"dtmFeature = %6ld dtmFeatureType = %30s dtmFeatureState = %2ld numDtmFeaturePts = %8ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmFeatureState,dtmFeatureP->numDtmFeaturePts) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots )
            {
             if( dtmFeatureP->dtmFeatureId == dtmP->nullFeatureId )
               {
                bcdtmWrite_message(0,0,0,"**** Error - Null Feature Id") ;
               }
            }
          if( dtmFeatureP->dtmFeaturePts.firstPoint < 0 || dtmFeatureP->dtmFeaturePts.firstPoint >= dtmP->numPoints ||
              dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts - 1 < 0 || dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts - 1  >= dtmP->numPoints )
             {
              bcdtmWrite_message(1,0,0,"Feature First Point Range Index Error") ;
              goto errexit ;
             }
         }
       for( n = 0 ; n < dtmP->numPoints ; ++n )
         {
          bcdtmWrite_message(0,0,0,"Point[%4ld] = %12.5lf %12.5lf %10.4lf",n,pointAddrP(dtmP,n)->x,pointAddrP(dtmP,n)->y,pointAddrP(dtmP,n)->z) ;
         }
      } 
   } 
/*
**  Update Modified Time
 */
 bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
 if( tileOffsetP != NULL ) free(tileOffsetP) ;
 if( numTilePtsP != NULL ) free(numTilePtsP) ;  
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Filtering Single Point Group Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Filtering Single Point Group Spots Error") ;
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
BENTLEYDTM_Private int bcdtmFilter_filterSinglePointGroupSpotTilesDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object          */
 long   filterOption,                  /* ==> Filter Option                  */
 long   reinsertOption,                /* ==> Reinsert Option                */
 double zTolerance,                    /* ==> z Filter Tolerance             */
 long   *tileOffsetP,                  /* ==> Index  To First In Tile        */
 long   *numTilePtsP,                  /* ==> Number Of Spots In Tile        */
 long   numTiles,                      /* ==> Number Of Tiles                */
 long   *numFilteredSpotsP             /* <== Number Of Spots After Filter   */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,m,loop,point,node1,node2,numPtsFiltered ; 
 long fndType,tinPnt1,tinPnt2,tinPnt3,numReinserted ;
 DPoint3d  randomSpot ;
 double z ;
 BC_DTM_OBJ *dtm1P=NULL,*dtm2P=NULL ;
 DPoint3d *pointP ;
 DTMFeatureId featureId ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Single Point Group Spot Tiles") ;
/*
** Initialise
*/
 *numFilteredSpotsP = 0 ;
/*
** Scan And Filter Each Tile
*/
 for( n = 0 ; n < numTiles ; ++n )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Filter Tile %4ld of %4ld ** numTilePts = %8ld",n+1,numTiles,*(numTilePtsP+n)) ;
/*
**  Create DTM Object For Tile Points
*/
    if( dtm1P != NULL ) if( bcdtmObject_destroyDtmObject(&dtm1P)) goto errexit ;
    if( bcdtmObject_createDtmObject(&dtm1P)) goto errexit ;
/*
**  Polulate DTM Object With Tile Spots
*/
    for( point = *(tileOffsetP+n) ; point < *(tileOffsetP+n) + *(numTilePtsP+n) ; ++point )
      {
       pointP = pointAddrP(dtmP,point) ;
       randomSpot.x = pointP->x ;
       randomSpot.y = pointP->y ;
       randomSpot.z = pointP->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,&randomSpot,1)) goto errexit ;
      }
/*
**  Iteratively Triangulate And Filter Dtm Object
*/
    loop = 5 ;
    while ( loop )
      {
       --loop ;
       if( dbg ) bcdtmWrite_message(0,0,0,"**** Triangulating And Filtering Tile ** numTilePts = %8ld",dtm1P->numPoints) ;
       dtm1P->edgeOption = 1 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Before Triangulation ** dtm1P->numPoints = %8ld",dtm1P->numPoints) ;
       if( bcdtmObject_triangulateDtmObject(dtm1P)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"After  Triangulation ** dtm1P->numPoints = %8ld",dtm1P->numPoints) ;
       if( filterOption == 1 ) if( bcdtmLidarFilter_coarseFilterTinPointsDtmObject(dtm1P,zTolerance,&numPtsFiltered)) goto errexit ;
       if( filterOption == 2 ) if( bcdtmLidarFilter_fineFilterTinPointsDtmObject(dtm1P,zTolerance,&numPtsFiltered)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points Filtered = %8ld",numPtsFiltered) ;
       if( numPtsFiltered == 0 ) loop = 0 ;
/*
**     Remove Deleted Spots
*/
       for( node1 = node2 = 0 ; node2 < dtm1P-> numPoints ; ++node2 )
         {
          if( nodeAddrP(dtm1P,node2)->tPtr == dtm1P->nullPnt )
            {
             if( node2 != node1 )
               {
                *pointAddrP(dtm1P,node1) = *pointAddrP(dtm1P,node2) ;
               }
             ++node1 ;
            }
         } 
       dtm1P->numPoints = node1 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"After Removing Deleted ** dtm1P->numPoints = %8ld",dtm1P->numPoints) ;
/*
**     Free Nodes memory
*/
       if( dtm1P->nodesPP != NULL ) 
         { 
          for( m = 0 ; m < dtm1P->numPointPartitions ; ++m ) free(dtm1P->nodesPP[m]) ;
          free( dtm1P->nodesPP) ;
          dtm1P->nodesPP = NULL  ; 
          dtm1P->numNodePartitions = 0 ;
          dtm1P->numNodes = 0 ;
          dtm1P->memNodes = 0 ;
         }
/*
**     Free Circular List Memory
*/
       if( dtm1P->cListPP != NULL ) 
         { 
          for( m = 0 ; m < dtm1P->numClistPartitions ; ++m ) free(dtm1P->cListPP[m]) ;
          free( dtm1P->cListPP) ;
          dtm1P->cListPP = NULL  ; 
          dtm1P->numClistPartitions = 0 ;
          dtm1P->numClist = 0 ;
          dtm1P->memClist = 0 ;
          dtm1P->cListPtr = 0 ;
          dtm1P->cListDelPtr = dtm1P->nullPtr ;
         }
/*
**     Reset DTM Header Values
*/
       dtm1P->dtmState        = DTMState::Data ;
       dtm1P->hullPoint       = dtm1P->nullPnt ;
       dtm1P->nextHullPoint   = dtm1P->nullPnt ;
       dtm1P->numSortedPoints = 0 ;
       dtm1P->numLines        = 0 ;
       dtm1P->numTriangles    = 0 ;
      }
/*
**  Store Filtered Points In DTM Object
*/
    if( dtm2P == NULL ) if( bcdtmObject_createDtmObject(&dtm2P)) goto errexit ;
    for( point = 0 ; point <  dtm1P->numPoints ; ++point )
      {
       pointP = pointAddrP(dtm1P,point) ;
       randomSpot.x = pointP->x ;
       randomSpot.y = pointP->y ;
       randomSpot.z = pointP->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtm2P,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,&randomSpot,1)) goto errexit ;
      }
/*
**  Destroy DTM Object
*/
    if( dtm1P != NULL ) bcdtmObject_destroyDtmObject(&dtm1P) ;
   }
/*
** Write Out Number Of Unfiltered Points
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"Before Reinsert ** Number Of Unfiltered Points = %8ld",dtm2P->numPoints) ;
/*
**  Reinsert Points Out Of z Tolerance Range
*/
 if( reinsertOption == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reinserting Points Outside Of z Tolerance") ;
    dtm2P->edgeOption = 1 ;
    if( bcdtmObject_triangulateDtmObject(dtm2P)) goto errexit ;
    if( bcdtmObject_createDtmObject(&dtm1P)) goto errexit ;
/*
**  Scan Tile Points And Drape On Tin
*/
    numReinserted = 0 ;
    for( n = 0 ; n < numTiles ; ++n )
      {
       for( point = *(tileOffsetP+n) ; point < *(tileOffsetP+n) + *(numTilePtsP+n) ; ++point )
         {
          pointP = pointAddrP(dtmP,point) ;
          if( bcdtmFind_triangleForPointDtmObject(dtm2P,pointP->x,pointP->y,&z,&fndType,&tinPnt1,&tinPnt2,&tinPnt3 )) goto errexit ;
          if( ! fndType || fabs(z-pointP->z) > zTolerance )
            {
             ++numReinserted ;
             randomSpot.x = pointP->x ;
             randomSpot.y = pointP->y ;
             randomSpot.z = pointP->z ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,&randomSpot,1)) goto errexit ;
            }
         }
      }
/*
**  Change State DTM Object 2
*/
    if( bcdtmObject_changeStateDtmObject(dtm2P,DTMState::Data)) goto errexit ; 
/*
**  Store Reinserted Point In DTM Object 2
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Reinserted Points") ;
    for( point = 0 ; point <  dtm1P->numPoints ; ++point )
      {
       pointP = pointAddrP(dtm1P,point) ;
       randomSpot.x = pointP->x ;
       randomSpot.y = pointP->y ;
       randomSpot.z = pointP->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtm2P,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,&randomSpot,1)) goto errexit ;
      }
/*
**  Destroy DTM Object
*/
    if( bcdtmObject_destroyDtmObject(&dtm1P)) goto errexit ; 
/*
**  Write number Of Points Reinserted
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points Reinserted = %8ld",numReinserted) ;
   }
/*
** Write Out Number Of Unfiltered Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"After   Reinsert ** Number Of Unfiltered Points = %8ld",dtm2P->numPoints) ;
/*
**  Set Number Of Spots
*/
 *numFilteredSpotsP = dtm2P->numPoints ;  
/*
** Set Number Of Points In Start Object
*/
 dtmP->numPoints = 0 ;
/*
** Store Group Spots
*/
 for( point = 0 ; point <  dtm2P->numPoints ; ++point )
   {
    pointP = pointAddrP(dtm2P,point) ;
    randomSpot.x = pointP->x ;
    randomSpot.y = pointP->y ;
    randomSpot.z = pointP->z ;
//    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::GroupSpots,DTM_NULL_USER_TAG,3,&featureId,&randomSpot,1)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,3,&featureId,&randomSpot,1)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dtm1P != NULL ) bcdtmObject_destroyDtmObject(&dtm1P) ;
 if( dtm2P != NULL ) bcdtmObject_destroyDtmObject(&dtm2P) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering Single Point Group Spot Tiles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering Single Point Group Spot Tiles Error") ;
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
BENTLEYDTM_Private int bcdtmFilter_elevationDifferenceCompareFunction(const void *p1P,const void *p2P)
/*
** Compare Function For Qsort Of Elevation Differences
*/
{
 struct ElevDifference { double elevation ; long point ; } *ed1P,*ed2P ;
 ed1P = ( struct ElevDifference * ) p1P ;
 ed2P = ( struct ElevDifference * ) p2P ;
 if     (  ed1P->elevation   <  ed2P->elevation  ) return(-1) ;
 else if(  ed1P->elevation   >  ed2P->elevation  ) return( 1)  ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmFilter_elevationDifferenceKeepCompareFunction(const void *p1P,const void *p2P)
/*
** Compare Function For Qsort Of Elevation Differences
*/
{
 struct ElevDifference { double elevation ; long point ; long tile ; long keep ; } *ed1P,*ed2P ;
 ed1P = ( struct ElevDifference * ) p1P ;
 ed2P = ( struct ElevDifference * ) p2P ;
 if     (  ed1P->elevation < ed2P->elevation  ) return(-1) ;
 else if(  ed1P->elevation > ed2P->elevation  ) return( 1)  ;
 else if(  ed1P->tile < ed2P->tile  ) return(-1) ;
 else if(  ed1P->tile > ed2P->tile  ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmFilter_elevationDifferenceTileKeepCompareFunction(const void *p1P,const void *p2P)
/*
** Compare Function For Qsort Of Void Points
*/
{
 struct ElevDifference { double elevation ; long point ; long tile ; long keep ; } *ed1P,*ed2P ;
 ed1P = ( struct ElevDifference * ) p1P ;
 ed2P = ( struct ElevDifference * ) p2P ;
 if     (  ed1P->tile   <  ed2P->tile  ) return(-1) ;
 else if(  ed1P->tile   >  ed2P->tile  ) return( 1) ;
 else if(  ed1P->keep   <  ed2P->keep  ) return(-1) ;
 else if(  ed1P->keep   >  ed2P->keep  ) return( 1) ;
 return(0) ;
}


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFilter_tinElevDiffRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object Containing points To Be Filtered       */
 long   filterOption,                  /* ==> < 1-Least Squares Plane , 2 - Average >                      */
 long   boundaryOption,                /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */ 
 double *elevationDifferencesP         /* <== Filter Tolerance Used To Decimate                            */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long point,point1,clPtr,numElevationPoints,numPointDiffs ; 
 long firstPoint,lastPoint,saveLastPoint,usePlane=TRUE,excludeBoundary=TRUE ;
 double elevation ; 
 DTM_TIN_NODE  *nodeP ;
 DPoint3d *pointP ;
 DTM_CIR_LIST  *clistP ;
 BC_DTM_OBJ    *planePtsP=NULL ;
 DTM_PLANE plane ;
 struct ElevDifference { double elevation ; long point ; } *elevDiffP = NULL;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin Decimating Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterOption      = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"boundaryOption    = %8ld",boundaryOption) ;        
   }

/*
** Check Parameters
*/
 if( filterOption < 1 || filterOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Filter Option") ;
    goto errexit ;
   }
 if( boundaryOption < 1 || boundaryOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Boundary Option") ;
    goto errexit ;
   }
/*
** Set Program Options
*/
 if( filterOption == 2 ) usePlane = FALSE ;
 if( boundaryOption == 2 ) excludeBoundary = FALSE ;
/*
** Check For Valid DTM
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ; 
/*
** Check For Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   } 

/*
** Sort Points On x Axis
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Random Spots") ;
    firstPoint = 0 ;
    saveLastPoint = lastPoint  = dtmP->numPoints - 1 ;
    if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
**  Remove Duplicate Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
    saveLastPoint = lastPoint ;
    if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
    if( saveLastPoint-lastPoint > 0 )
      {
       bcdtmWrite_message(1,0,0,"Duplicate points Found While Filtering") ;
       goto errexit ; 
      }
   }
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 dtmP->edgeOption = 2 ;
 dtmP->ppTol = dtmP->plTol = 0.0  ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Create DTM To Store Plane Points
*/
 if( usePlane == TRUE )
   {
    if( bcdtmObject_createDtmObject(&planePtsP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(planePtsP,100,100) ;
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Scan All Internal Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 numPointDiffs = 0 ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
/*
**  Least Squares Plane Filter Point
*/
    if( usePlane == TRUE )
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          planePtsP->numPoints = 0 ;
          clPtr = nodeP->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
/*
**           Store Point In DTM Object
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Point") ;
             pointP = pointAddrP(dtmP,point1) ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(planePtsP,DTMFeatureType::RandomSpots,planePtsP->nullUserTag,1,&planePtsP->nullFeatureId,(DPoint3d *)pointP,1)) goto errexit ;
            }
/*
**        Calculate Least Squares Plane Through Points
*/
          if( planePtsP->numPoints > 3 )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calculating Plane") ;
             if( bcdtmFilter_findPlaneDtmObject(planePtsP,0,planePtsP->numPoints,&plane) ) goto errexit ;
             pointP = pointAddrP(dtmP,point) ;
             elevation = pointP->z - ( plane.A * pointP->x + plane.B * pointP->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(elevation) ;
             (elevDiffP+numPointDiffs)->point     = point ;                         
             elevationDifferencesP[point] = (elevDiffP+numPointDiffs)->elevation ;
             ++numPointDiffs ;
            }
          else
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting To Max Elevation Difference") ;
             (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
             (elevDiffP+numPointDiffs)->point     = point ;
             elevationDifferencesP[point] = (elevDiffP+numPointDiffs)->elevation ;
             ++numPointDiffs ;
            } 
         }
       else
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting To Max Elevation Difference") ;
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          elevationDifferencesP[point] = (elevDiffP+numPointDiffs)->elevation ;          
          ++numPointDiffs ;
         } 
      }
/*
**  Average Filter Point
*/
    else
      {
       
          clPtr = nodeP->cPtr ;
          elevation = 0.0 ;
          numElevationPoints = 0 ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             elevation = elevation + pointAddrP(dtmP,point1)->z ;
             ++numElevationPoints ;
            } 
          elevation = elevation / ( double ) numElevationPoints ;
          elevation = fabs(elevation - pointAddrP(dtmP,point)->z ) ;
          
          if(( excludeBoundary == TRUE ) && ( nodeP->hPtr != dtmP->nullPnt ))
            {
            elevation = DBL_MAX;               
            }

          (elevDiffP+numPointDiffs)->elevation = elevation ;
          (elevDiffP+numPointDiffs)->point     = point ;
          elevationDifferencesP[point] = (elevDiffP+numPointDiffs)->elevation ;


          ++numPointDiffs ;
         

    /*
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin * 2;
          (elevDiffP+numPointDiffs)->point     = point ;
          elevationDifferencesP[point] = (elevDiffP+numPointDiffs)->elevation ;
          ++numPointDiffs ;
         } */
      }
   }
/*
** Check Consistency Of points Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointDiffs = %8ld",numPointDiffs) ;
 if( numPointDiffs != dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Point Differences") ;
    goto errexit ;
   }

 /*
** Clean Up
*/
 cleanup :
 if( planePtsP != NULL ) bcdtmObject_destroyDtmObject(&planePtsP) ;
 if( elevDiffP != NULL ) free(elevDiffP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Decimating Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Decimating Random Spots Error") ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_tinDecimateRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object Containing points To Be Filtered       */
 long   filterOption,                  /* ==> < 1-Least Squares Plane , 2 - Average >                      */
 long   boundaryOption,                /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */
 long   numPointsRemove,               /* ==> Number Of Points To Remove                                   */ 
 long   *numFilteredPtsP,              /* <== Number Of Points After Filter                                */
 BC_DTM_OBJ *filteredPtsP,             /* <== Pointer To DTM Containing The Filtered Points                */
 double  *filterToleranceUsedP         /* <== Filter Tolerance Used To Decimate                            */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long n,m,node1,node2,point,point1,clPtr,numElevationPoints,numPointDiffs ; 
 long firstPoint,lastPoint,saveLastPoint,usePlane=TRUE,excludeBoundary=TRUE ;
 double elevation ;
 DPoint3d dtmPoint ;
 DTM_TIN_NODE  *nodeP ;
 DPoint3d *pointP ;
 DTM_CIR_LIST  *clistP ;
 BC_DTM_OBJ    *planePtsP=NULL ;
 DTM_PLANE plane ;
 struct ElevDifference { double elevation ; long point ; } *elevDiffP = NULL;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin Decimating Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterOption      = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"boundaryOption    = %8ld",boundaryOption) ;
    bcdtmWrite_message(0,0,0,"numFilteredPtsP   = %8ld",*numFilteredPtsP) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP      = %p",*filteredPtsP) ;
   }
/*
** Initialise
*/
 *numFilteredPtsP = 0 ;
 *filterToleranceUsedP = 0.0 ;
/*
** Check Parameters
*/
 if( filterOption < 1 || filterOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Filter Option") ;
    goto errexit ;
   }
 if( boundaryOption < 1 || boundaryOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Boundary Option") ;
    goto errexit ;
   }
/*
** Set Program Options
*/
 if( filterOption == 2 ) usePlane = FALSE ;
 if( boundaryOption == 2 ) excludeBoundary = FALSE ;
/*
** Check For Valid DTM
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(filteredPtsP)) goto errexit ;
/*
** Check For Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   } 
/*
** Initialise
*/
 if( bcdtmObject_initialiseDtmObject(filteredPtsP)) goto errexit ; 
/*
** Sort Points On x Axis
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Random Spots") ;
    firstPoint = 0 ;
    saveLastPoint = lastPoint  = dtmP->numPoints - 1 ;
    if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
**  Remove Duplicate Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
    saveLastPoint = lastPoint ;
    if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
    if( saveLastPoint-lastPoint > 0 )
      {
       bcdtmWrite_message(1,0,0,"Duplicate points Found While Filtering") ;
       goto errexit ; 
      }
   }
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 dtmP->edgeOption = 2 ;
 dtmP->ppTol = dtmP->plTol = 0.0  ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Create DTM To Store Plane Points
*/
 if( usePlane == TRUE )
   {
    if( bcdtmObject_createDtmObject(&planePtsP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(planePtsP,100,100) ;
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Scan All Internal Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 numPointDiffs = 0 ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
/*
**  Least Squares Plane Filter Point
*/
    if( usePlane == TRUE )
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          planePtsP->numPoints = 0 ;
          clPtr = nodeP->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
/*
**           Store Point In DTM Object
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Point") ;
             pointP = pointAddrP(dtmP,point1) ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(planePtsP,DTMFeatureType::RandomSpots,planePtsP->nullUserTag,1,&planePtsP->nullFeatureId,(DPoint3d *)pointP,1)) goto errexit ;
            }
/*
**        Calculate Least Squares Plane Through Points
*/
          if( planePtsP->numPoints > 3 )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calculating Plane") ;
             if( bcdtmFilter_findPlaneDtmObject(planePtsP,0,planePtsP->numPoints,&plane) ) goto errexit ;
             pointP = pointAddrP(dtmP,point) ;
             elevation = pointP->z - ( plane.A * pointP->x + plane.B * pointP->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(elevation) ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            }
          else
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting To Max Elevation Difference") ;
             (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            } 
         }
       else
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting To Max Elevation Difference") ;
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
/*
**  Average Filter Point
*/
    else
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          clPtr = nodeP->cPtr ;
          elevation = 0.0 ;
          numElevationPoints = 0 ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             elevation = elevation + pointAddrP(dtmP,point1)->z ;
             ++numElevationPoints ;
            } 
          elevation = elevation / ( double ) numElevationPoints ;
          elevation = fabs(elevation - pointAddrP(dtmP,point)->z ) ;
          (elevDiffP+numPointDiffs)->elevation = elevation ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
   }
/*
** Check Consistency Of points Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointDiffs = %8ld",numPointDiffs) ;
 if( numPointDiffs != dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Point Differences") ;
    goto errexit ;
   }
/*
**  Quick Sort Elevation Difference Structure
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
 qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmFilter_elevationDifferenceCompareFunction) ;
/*
** Mark All Points That Can Be Removed And Copy All Filtered Points To Filtered Points DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 for( n = 0 ; n < numPointsRemove && n < numPointDiffs ; ++n )
   {
    nodeAddrP(dtmP,(elevDiffP+n)->point)->tPtr = 1 ;
    pointP = pointAddrP(dtmP,(elevDiffP+n)->point) ;
    dtmPoint.x = pointP->x ;
    dtmPoint.y = pointP->y ;
    dtmPoint.z = pointP->z ;
    if( (elevDiffP+n)->elevation > *filterToleranceUsedP ) *filterToleranceUsedP = (elevDiffP+n)->elevation ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(filteredPtsP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ;
   }
/*
**  Remove Deleted Points From DTM
*/
 for( node1 = node2 = 0 ; node2 < dtmP->numPoints ; ++node2 )
   {
    if( nodeAddrP(dtmP,node2)->tPtr == dtmP->nullPnt )
      {
       if( node2 != node1 )
         {
          *pointAddrP(dtmP,node1) = *pointAddrP(dtmP,node2) ;
         }
        ++node1 ;
       }
    } 
  dtmP->numPoints = node1 ;
  if( dbg ) bcdtmWrite_message(0,0,0,"After Removing Filtered Points ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Free Nodes memory
*/
 if( dtmP->nodesPP != NULL ) 
   { 
    for( m = 0 ; m < dtmP->numPointPartitions ; ++m ) free(dtmP->nodesPP[m]) ;
    free( dtmP->nodesPP) ;
    dtmP->nodesPP = NULL  ; 
    dtmP->numNodePartitions = 0 ;
    dtmP->numNodes = 0 ;
    dtmP->memNodes = 0 ;
   }
/*
** Free Circular List Memory
*/
 if( dtmP->cListPP != NULL ) 
   { 
    for( m = 0 ; m < dtmP->numClistPartitions ; ++m ) free(dtmP->cListPP[m]) ;
    free( dtmP->cListPP) ;
    dtmP->cListPP = NULL  ; 
    dtmP->numClistPartitions = 0 ;
    dtmP->numClist = 0 ;
    dtmP->memClist = 0 ;
    dtmP->cListPtr = 0 ;
    dtmP->cListDelPtr = dtmP->nullPtr ;
   }
/*
**  Reset DTM Header Values
*/
 dtmP->dtmState        = DTMState::Data ;
 dtmP->hullPoint       = dtmP->nullPnt ;
 dtmP->nextHullPoint   = dtmP->nullPnt ;
 dtmP->numSortedPoints = 0 ;
 dtmP->numLines        = 0 ;
 dtmP->numTriangles    = 0 ;
/*
**  Set Number Of Spots
*/
 *numFilteredPtsP = filteredPtsP->numPoints ;  
/*
** Clean Up
*/
 cleanup :
 if( planePtsP != NULL ) bcdtmObject_destroyDtmObject(&planePtsP) ;
 if( elevDiffP != NULL ) free(elevDiffP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Decimating Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Decimating Random Spots Error") ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_tileElevDiffRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object Containing points To Be Filtered       */ 
 double *elevationDifferencesP         /* <== Filter Tolerance Used To Decimate                            */
) 
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long saveLastPoint,numPointDiffs ; 
 long pnt,firstPoint,lastPoint,maxTilePts,minTilePts,startTime ;
 long tile,numTiles;
 char *pointMarkP=NULL ;
 double dZ,xMin,yMin,zMin,xMax,yMax,zMax ;
 struct ElevDifference { double elevation ; long point ; } *elevDiffP=NULL ;
 DPoint3d *pntP ;
 DTM_PLANE plane ;
 DTM_POINT_TILE *pointTilesP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tile Decimating Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;        
   }
/*
** Initialise
*/ 
 firstPoint = 0 ;
 lastPoint  = dtmP->numPoints - 1 ;
/*
** Sort Points On x Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Random Spots") ;
 if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %8ld ** lastPoint = %8ld",firstPoint,lastPoint) ;
 if( saveLastPoint-lastPoint > 0 )
   {
    bcdtmWrite_message(1,0,0,"Duplicate points Found While Filtering") ;
//    goto errexit ; 
   }
/*
**  Tile Dtm Points
*/
 startTime = bcdtmClock() ;
 maxTilePts = 10 ;
 minTilePts = 5 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tiling XYZ Points Into Tiles With A Maximum Of %8ld Points Per Tile",maxTilePts) ;
 if( bcdtmMedianTile_pointsDtmObject(dtmP,NULL,0,dtmP->numPoints,minTilePts,&pointTilesP,&numTiles)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Tile %8ld Points Into %8ld Tiles = %8.3lf Secs",dtmP->numPoints,numTiles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Write Tiles
*/
 if( dbg == 2 )
   {
    for( tile = 0 ; tile  < numTiles ; ++tile )
      {
       pntP = pointAddrP(dtmP,(pointTilesP+tile)->tileOffset) ;
       xMin = xMax = pntP->x ;
       yMin = yMax = pntP->y ;
       zMin = zMax = pntP->z ;
       for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
         {
          pntP = pointAddrP(dtmP,pnt) ;
          if( pntP->x < xMin ) xMin = pntP->x ;
          if( pntP->x > xMax ) xMax = pntP->x ;
          if( pntP->y < yMin ) yMin = pntP->y ;
          if( pntP->y > yMax ) yMax = pntP->y ;
          if( pntP->z < zMin ) zMin = pntP->z ;
          if( pntP->z > zMax ) zMax = pntP->z ;
         }
       bcdtmWrite_message(0,0,0,"tile[%4ld] ** startPoint = %8ld numPoints = %8ld ** xMin = %12.5lf xMax = %12.5lf ** yMin = %12.5lf yMax = %12.5lf ** zMin = %10.4lf zMax = %10.4lf",tile,(pointTilesP+tile)->tileOffset,(pointTilesP+tile)->numTilePts,xMin,xMax,yMin,yMax,zMin,zMax) ;
      }
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Filter Tiles
*/
 numPointDiffs = 0 ;
 for( tile = 0 ; tile  < numTiles ; ++tile )
   {
/*
**  Calculate Least Squares Plane Through The Tile Points
*/
    if( (pointTilesP+tile)->numTilePts > 3 )
      {
       plane.A = 0.0 ;
       plane.B = 0.0 ;
       plane.C = 0.0 ;  
       if( bcdtmFilter_findPlaneDtmObject(dtmP,(pointTilesP+tile)->tileOffset,(pointTilesP+tile)->numTilePts,&plane) == DTM_SUCCESS )
         {
          for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
            {
             dZ = pointAddrP(dtmP,pnt)->z - ( plane.A * pointAddrP(dtmP,pnt)->x + plane.B * pointAddrP(dtmP,pnt)->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(dZ) ;
             (elevDiffP+numPointDiffs)->point     = pnt ;
             elevationDifferencesP[numPointDiffs] = (elevDiffP+numPointDiffs)->elevation ;
             ++numPointDiffs ;
            }
         }
      }
    else
      {
       for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = pnt ;
          elevationDifferencesP[numPointDiffs] = (elevDiffP+numPointDiffs)->elevation ;
           ++numPointDiffs ;
         }
      }  
   }

/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( pointTilesP != NULL ) free(pointTilesP) ;
 if( pointMarkP  != NULL ) free(pointMarkP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Decimating Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Decimating Random Spots Error") ;
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
int bcdtmFilter_tileDecimateRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object                            */
 long   numPointsRemove,               /* ==> Number Of Points To Remove                       */ 
 long   *numFilteredSpotsP,            /* <== Number Of Spots After Filter                     */
 BC_DTM_OBJ *filteredDtmP,             /* <== Pointer To DTM Object With The Filtered Points   */
 double  *filterToleranceUsedP         /* <== Filter Tolerance Used To Decimate                */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long n,node1,node2,numMarks,saveLastPoint,numPointDiffs ; 
 long pnt,firstPoint,lastPoint,maxTilePts,minTilePts,startTime ;
 long tile,numTiles;
 unsigned char *cP,*pointMarkP=NULL ;
 double dZ,xMin,yMin,zMin,xMax,yMax,zMax ;
 struct ElevDifference { double elevation ; long point ; } *eldP,*elevDiffP=NULL ;
 DPoint3d *pntP ;
 DTM_PLANE plane ;
 DTM_POINT_TILE *pointTilesP=NULL ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tile Decimating Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"numPointsRemove   = %8ld",numPointsRemove) ;
    bcdtmWrite_message(0,0,0,"numFilteredSpotsP = %8ld",*numFilteredSpotsP) ;
   }
/*
** Initialise
*/
 *numFilteredSpotsP = 0 ;
 *filterToleranceUsedP = 0.0 ;
 firstPoint = 0 ;
 lastPoint  = dtmP->numPoints - 1 ;
/*
** Sort Points On x Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Random Spots") ;
 if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %8ld ** lastPoint = %8ld",firstPoint,lastPoint) ;
 if( saveLastPoint-lastPoint > 0 )
   {
    bcdtmWrite_message(1,0,0,"Duplicate points Found While Filtering") ;
//    goto errexit ; 
   }
/*
**  Tile Dtm Points
*/
 startTime = bcdtmClock() ;
 maxTilePts = 10 ;
 minTilePts = 5 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tiling XYZ Points Into Tiles With A Maximum Of %8ld Points Per Tile",maxTilePts) ;
 if( bcdtmMedianTile_pointsDtmObject(dtmP,NULL,0,dtmP->numPoints,minTilePts,&pointTilesP,&numTiles)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Tile %8ld Points Into %8ld Tiles = %8.3lf Secs",dtmP->numPoints,numTiles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Write Tiles
*/
 if( dbg == 2 )
   {
    for( tile = 0 ; tile  < numTiles ; ++tile )
      {
       pntP = pointAddrP(dtmP,(pointTilesP+tile)->tileOffset) ;
       xMin = xMax = pntP->x ;
       yMin = yMax = pntP->y ;
       zMin = zMax = pntP->z ;
       for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
         {
          pntP = pointAddrP(dtmP,pnt) ;
          if( pntP->x < xMin ) xMin = pntP->x ;
          if( pntP->x > xMax ) xMax = pntP->x ;
          if( pntP->y < yMin ) yMin = pntP->y ;
          if( pntP->y > yMax ) yMax = pntP->y ;
          if( pntP->z < zMin ) zMin = pntP->z ;
          if( pntP->z > zMax ) zMax = pntP->z ;
         }
       bcdtmWrite_message(0,0,0,"tile[%4ld] ** startPoint = %8ld numPoints = %8ld ** xMin = %12.5lf xMax = %12.5lf ** yMin = %12.5lf yMax = %12.5lf ** zMin = %10.4lf zMax = %10.4lf",tile,(pointTilesP+tile)->tileOffset,(pointTilesP+tile)->numTilePts,xMin,xMax,yMin,yMax,zMin,zMax) ;
      }
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Filter Tiles
*/
 numPointDiffs = 0 ;
 for( tile = 0 ; tile  < numTiles ; ++tile )
   {
/*
**  Calculate Least Squares Plane Through The Tile Points
*/
    if( (pointTilesP+tile)->numTilePts > 3 )
      {
       plane.A = 0.0 ;
       plane.B = 0.0 ;
       plane.C = 0.0 ;  
       if( bcdtmFilter_findPlaneDtmObject(dtmP,(pointTilesP+tile)->tileOffset,(pointTilesP+tile)->numTilePts,&plane) == DTM_SUCCESS )
         {
          for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
            {
             dZ = pointAddrP(dtmP,pnt)->z - ( plane.A * pointAddrP(dtmP,pnt)->x + plane.B * pointAddrP(dtmP,pnt)->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(dZ) ;
             (elevDiffP+numPointDiffs)->point     = pnt ;
             ++numPointDiffs ;
            }
         }
      }
    else
      {
       for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = pnt ;
           ++numPointDiffs ;
         }
      }  
   }
/*
**  Quick Sort Elevation Difference Structure
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
 qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmFilter_elevationDifferenceCompareFunction) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
    for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
      {
       bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** point = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->point) ;
      } 
   }
 if( dbg )  bcdtmWrite_message(0,0,0,"Median Elevation Point Difference = %10.4lf",(elevDiffP+numPointsRemove-1)->elevation) ;
/*
** Allocate Memory For Marking Points For Removal
*/
 numMarks = dtmP->numPoints / 8 + 1 ;
 pointMarkP = ( unsigned char *) malloc(numMarks*sizeof(char)) ;
 if( pointMarkP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( cP = pointMarkP ; cP < pointMarkP + numMarks ; ++cP ) *cP = 255 ;
/*
** Mark All Points That Can Be Removed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
 for( n = 0 ; n < numPointsRemove && n < numPointDiffs ; ++n )
   {
    bcdtmFlag_clearFlag(pointMarkP,(elevDiffP+n)->point) ;
    if( (elevDiffP+n)->elevation > *filterToleranceUsedP ) *filterToleranceUsedP = (elevDiffP+n)->elevation ;
   }
/*
** Remove Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Marked Points") ;
 for( node1 = node2 = 0 ; node2 < dtmP->numPoints ; ++node2 )
   {
    if( bcdtmFlag_testFlag(pointMarkP,node2) )
      {
       if( node2 != node1 )
         {
          *pointAddrP(dtmP,node1) = *pointAddrP(dtmP,node2) ;
         }
        ++node1 ;
       }
     else
       {
        if( bcdtmObject_storeDtmFeatureInDtmObject(filteredDtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(dtmP,node2),1)) goto errexit ;
       } 
    } 
  dtmP->numPoints = node1 ;
  if( dbg ) bcdtmWrite_message(0,0,0,"After Removing Filtered Points ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( pointTilesP != NULL ) free(pointTilesP) ;
 if( pointMarkP  != NULL ) free(pointMarkP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Decimating Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Decimating Random Spots Error") ;
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
int bcdtmFilter_tileDecimateRandomSpotsDtmObject (
    BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object                            */
    long numPointsToRemove,
    double percentageToRemove,
    long& pointsBefore,
    long& pointsAfter)
    {
    int  ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long firstPoint, lastPoint;
    long *tileOffsetP = NULL, *numTilePtsP = NULL;
    BC_DTM_OBJ* filteredDtmP = NULL;
    double filterToleranceUsedP;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "tile Decimate Random Spots");
        bcdtmWrite_message (0, 0, 0, "dtmP           = %p", dtmP);
        //bcdtmWrite_message(0,0,0,"filterOption   = %8ld",filterOption) ;
        //bcdtmWrite_message(0,0,0,"reinsertOption = %8ld",reinsertOption) ;
        //bcdtmWrite_message(0,0,0,"zTolerance     = %8.3lf",zTolerance) ;
        }
    /*
    ** Test For Valid Dtm Object
    */
    if (bcdtmObject_testForValidDtmObject (dtmP)) goto errexit;
    /*
    ** If DTM In Triangulated State Change It To Untriangulated State
    */
    if (dtmP->dtmState == DTMState::Tin)
        {
        if (bcdtmObject_changeStateDtmObject (dtmP, DTMState::Data)) goto errexit;
        }
    /*
    ** Check DTM Is In Untriangulated State
    */
    if (dtmP->dtmState != DTMState::Data)
        {
        bcdtmWrite_message (2, 0, 0, "Method Requires Untriangulated DTM");
        goto errexit;
        }
    /*
    ** Place Random Spots In One Contiguous DTM Block
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Blocking Random Spots");
    if (bcdtmFilter_blockRandomSpotsDtmObject (dtmP, &firstPoint, &lastPoint)) goto errexit;
    if (dbg) bcdtmWrite_message (0, 0, 0, "firstPoint = %8ld lastPoint = %8ld", firstPoint, lastPoint);
    pointsAfter = pointsBefore = lastPoint - firstPoint + 1;

    if (pointsBefore != 0)
        {
        /*
        ** Create DTM Object
        */
        if (bcdtmObject_createDtmObject (&filteredDtmP))
            goto errexit;

        if (numPointsToRemove == 0)
            numPointsToRemove = (int)(percentageToRemove * pointsBefore);
        /*
        ** Fitler out the points from this DTM to the filtered DTM
        */
        if (bcdtmFilter_tileDecimateRandomSpotsDtmObject (dtmP, numPointsToRemove, &pointsAfter, filteredDtmP, &filterToleranceUsedP))
            goto errexit;

        /*
        ** Copy the filtered points back into the original DTM.
        */
        pointsAfter = dtmP->numPoints = filteredDtmP->numPoints;

        for (int i = 0; i < dtmP->numPoints; i++)
            {
            *pointAddrP (dtmP, i) = *pointAddrP (filteredDtmP, i);
            }
        }

    /*
    ** Set Feature States Back To First Point
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Resetting Feature States To First Point");
    if (bcdtmFilter_setFeatureStatesToFirstPointDtmObject (dtmP)) goto errexit;
    /*
    ** Resize DTM Object
    */
    if (bcdtmObject_resizeMemoryDtmObject (dtmP)) goto errexit;
    /*
    ** Write Out Filter Results
    */
    if (dbg)
        {
        //    bcdtmWrite_message(0,0,0,"spotsBefore = %8ld spotsAfter = %8ld reduction = %8.4lf",*numSpotsBeforeFilterP,*numSpotsAfterFilterP,*filterReductionP) ;
        }
    /*
    **  Update Modified Time
    */
    bcdtmObject_updateLastModifiedTime (dtmP);
    /*
    ** Clean Up
    */
cleanup:
    if (filteredDtmP != NULL) bcdtmObject_destroyDtmObject (&filteredDtmP);
    if (tileOffsetP != NULL) free (tileOffsetP);
    if (numTilePtsP != NULL) free (numTilePtsP);
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Tin Filtering Random Spots Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Tin Filtering Random Spots Error");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFilter_tileZToleranceFilterRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object          */
 double filterTolerance,               /* ==> Number Of Points To Remove     */ 
 long   *numFilteredSpotsP,            /* <== Number Of Spots After Filter   */
 BC_DTM_OBJ *filteredDtmP              /* <== Pointer To DTM Object With The Filtered Points   */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long n,node1,node2,numMarks,saveLastPoint,numPointDiffs ; 
 long pnt,firstPoint,lastPoint,maxTilePts,minTilePts,startTime ;
 long tile,numTiles;
 unsigned char *cP,*pointMarkP=NULL ;
 double dZ,xMin,yMin,zMin,xMax,yMax,zMax ;
 struct ElevDifference { double elevation ; long point ; } *eldP,*elevDiffP=NULL ;
 DPoint3d *pntP ;
 DTM_PLANE plane ;
 DTM_POINT_TILE *pointTilesP=NULL ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tile z Tolerance Filter Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterTolerance   = %8ld",filterTolerance) ;
    bcdtmWrite_message(0,0,0,"numFilteredSpotsP = %8ld",*numFilteredSpotsP) ;
   }
/*
** Initialise
*/
 *numFilteredSpotsP = 0 ;
 firstPoint = 0 ;
 lastPoint  = dtmP->numPoints - 1 ;
/*
** Sort Points On x Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Random Spots") ;
 if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %8ld ** lastPoint = %8ld",firstPoint,lastPoint) ;
 if( saveLastPoint-lastPoint > 0 )
   {
    bcdtmWrite_message(1,0,0,"Duplicate points Found While Filtering") ;
//    goto errexit ; 
   }
/*
**  Tile Dtm Points
*/
 startTime = bcdtmClock() ;
 maxTilePts = 10 ;
 minTilePts = 5 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tiling XYZ Points Into Tiles With A Maximum Of %8ld Points Per Tile",maxTilePts) ;
 if( bcdtmMedianTile_pointsDtmObject(dtmP,NULL,0,dtmP->numPoints,minTilePts,&pointTilesP,&numTiles)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Tile %8ld Points Into %8ld Tiles = %8.3lf Secs",dtmP->numPoints,numTiles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Write Tiles
*/
 if( dbg == 2 )
   {
    for( tile = 0 ; tile  < numTiles ; ++tile )
      {
       pntP = pointAddrP(dtmP,(pointTilesP+tile)->tileOffset) ;
       xMin = xMax = pntP->x ;
       yMin = yMax = pntP->y ;
       zMin = zMax = pntP->z ;
       for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
         {
          pntP = pointAddrP(dtmP,pnt) ;
          if( pntP->x < xMin ) xMin = pntP->x ;
          if( pntP->x > xMax ) xMax = pntP->x ;
          if( pntP->y < yMin ) yMin = pntP->y ;
          if( pntP->y > yMax ) yMax = pntP->y ;
          if( pntP->z < zMin ) zMin = pntP->z ;
          if( pntP->z > zMax ) zMax = pntP->z ;
         }
       bcdtmWrite_message(0,0,0,"tile[%4ld] ** startPoint = %8ld numPoints = %8ld ** xMin = %12.5lf xMax = %12.5lf ** yMin = %12.5lf yMax = %12.5lf ** zMin = %10.4lf zMax = %10.4lf",tile,(pointTilesP+tile)->tileOffset,(pointTilesP+tile)->numTilePts,xMin,xMax,yMin,yMax,zMin,zMax) ;
      }
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Filter Tiles
*/
 numPointDiffs = 0 ;
 for( tile = 0 ; tile  < numTiles ; ++tile )
   {
/*
**  Calculate Least Squares Plane Through The Tile Points
*/
    if( (pointTilesP+tile)->numTilePts > 3 )
      {
       plane.A = 0.0 ;
       plane.B = 0.0 ;
       plane.C = 0.0 ;  
       if( bcdtmFilter_findPlaneDtmObject(dtmP,(pointTilesP+tile)->tileOffset,(pointTilesP+tile)->numTilePts,&plane) == DTM_SUCCESS )
         {
          for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
            {
             dZ = pointAddrP(dtmP,pnt)->z - ( plane.A * pointAddrP(dtmP,pnt)->x + plane.B * pointAddrP(dtmP,pnt)->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(dZ) ;
             (elevDiffP+numPointDiffs)->point     = pnt ;
             ++numPointDiffs ;
            }
         }
      }
    else
      {
       for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = pnt ;
           ++numPointDiffs ;
         }
      }  
   }
/*
**  Quick Sort Elevation Difference Structure
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
 qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmFilter_elevationDifferenceCompareFunction) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
    for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
      {
       bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** point = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->point) ;
      } 
   }
/*
** Allocate Memory For Marking Points For Removal
*/
 numMarks = dtmP->numPoints / 8 + 1 ;
 pointMarkP = ( unsigned char *) malloc(numMarks*sizeof(char)) ;
 if( pointMarkP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( cP = pointMarkP ; cP < pointMarkP + numMarks ; ++cP ) *cP = 255 ;
/*
** Mark All Points That Can Be Removed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
 for( n = 0 ; n < numPointDiffs ; ++n )
   {
    if( (elevDiffP+n)->elevation <= filterTolerance )
      {
       bcdtmFlag_clearFlag(pointMarkP,(elevDiffP+n)->point) ;
      }  
   }
/*
** Remove Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Marked Points") ;
 for( node1 = node2 = 0 ; node2 < dtmP->numPoints ; ++node2 )
   {
    if( bcdtmFlag_testFlag(pointMarkP,node2) )
      {
       if( node2 != node1 )
         {
          *pointAddrP(dtmP,node1) = *pointAddrP(dtmP,node2) ;
         }
        ++node1 ;
       }
     else
       {
        if( bcdtmObject_storeDtmFeatureInDtmObject(filteredDtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(dtmP,node2),1)) goto errexit ;
       } 
    } 
  dtmP->numPoints = node1 ;
  if( dbg ) bcdtmWrite_message(0,0,0,"After Removing Filtered Points ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( pointTilesP != NULL ) free(pointTilesP) ;
 if( pointMarkP  != NULL ) free(pointMarkP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile z Tolerance Filter Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile z Tolerance Filter Random Spots Error") ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_tinZToleranceFilterRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object Containing points To Be Filtered       */
 long   filterOption,                  /* ==> < 1-Least Squares Plane , 2 - Average >                      */
 long   boundaryOption,                /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */
 double filterTolerance,               /* ==> Filter Tolerance                                             */ 
 long   *numFilteredPtsP,              /* <== Number Of Points After Filter                                */
 BC_DTM_OBJ *filteredPtsP              /* <== Pointer To DTM Containing The Filtered Points                */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long n,m,node1,node2,point,point1,clPtr,numElevationPoints,numPointDiffs ; 
 long firstPoint,lastPoint,saveLastPoint,usePlane=TRUE,excludeBoundary=TRUE ;
 double elevation ;
 DPoint3d dtmPoint ;
 DTM_TIN_NODE  *nodeP ;
 DPoint3d *pointP ;
 DTM_CIR_LIST  *clistP ;
 BC_DTM_OBJ    *planePtsP=NULL ;
 DTM_PLANE plane ;
 struct ElevDifference { double elevation ; long point ; } *elevDiffP = NULL;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin z Tolerance Filter Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterOption      = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"boundaryOption    = %8ld",boundaryOption) ;
    bcdtmWrite_message(0,0,0,"filterTolerance   = %8.3lf",filterTolerance) ;
    bcdtmWrite_message(0,0,0,"numFilteredPtsP   = %8ld",*numFilteredPtsP) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP      = %p",*filteredPtsP) ;
   }
/*
** Initialise
*/
 *numFilteredPtsP = 0 ;
 if( filterTolerance < 0.0 ) filterTolerance = 0.0 ;
/*
** Check Parameters
*/
 if( filterOption < 1 || filterOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Filter Option") ;
    goto errexit ;
   }
 if( boundaryOption < 1 || boundaryOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Boundary Option") ;
    goto errexit ;
   }
/*
** Set Program Options
*/
 if( filterOption == 2 ) usePlane = FALSE ;
 if( boundaryOption == 2 ) excludeBoundary = FALSE ;
/*
** Check For Valid DTM
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(filteredPtsP)) goto errexit ;
/*
** Check For Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   } 
/*
** Initialise
*/
 if( bcdtmObject_initialiseDtmObject(filteredPtsP)) goto errexit ; 
/*
** Sort Points On x Axis
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Random Spots") ;
    firstPoint = 0 ;
    saveLastPoint = lastPoint  = dtmP->numPoints - 1 ;
    if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
**  Remove Duplicate Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
    saveLastPoint = lastPoint ;
    if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
    if( saveLastPoint-lastPoint > 0 )
      {
       bcdtmWrite_message(1,0,0,"Duplicate points Found While Filtering") ;
       goto errexit ; 
      }
   }
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 dtmP->edgeOption = 2 ;
 dtmP->ppTol = dtmP->plTol = 0.0  ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Create DTM To Store Plane Points
*/
 if( usePlane == TRUE )
   {
    if( bcdtmObject_createDtmObject(&planePtsP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(planePtsP,100,100) ;
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Scan All Internal Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 numPointDiffs = 0 ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
/*
**  Least Squares Plane Filter Point
*/
    if( usePlane == TRUE )
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          planePtsP->numPoints = 0 ;
          clPtr = nodeP->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
/*
**           Store Point In DTM Object
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Point") ;
             pointP = pointAddrP(dtmP,point1) ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(planePtsP,DTMFeatureType::RandomSpots,planePtsP->nullUserTag,1,&planePtsP->nullFeatureId,(DPoint3d *)pointP,1)) goto errexit ;
            }
/*
**        Calculate Least Squares Plane Through Points
*/
          if( planePtsP->numPoints > 3 )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calculating Plane") ;
             if( bcdtmFilter_findPlaneDtmObject(planePtsP,0,planePtsP->numPoints,&plane) ) goto errexit ;
             pointP = pointAddrP(dtmP,point) ;
             elevation = pointP->z - ( plane.A * pointP->x + plane.B * pointP->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(elevation) ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            }
          else
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting To Max Elevation Difference") ;
             (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            } 
         }
       else
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting To Max Elevation Difference") ;
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
/*
**  Average Filter Point
*/
    else
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          clPtr = nodeP->cPtr ;
          elevation = 0.0 ;
          numElevationPoints = 0 ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             elevation = elevation + pointAddrP(dtmP,point1)->z ;
             ++numElevationPoints ;
            } 
          elevation = elevation / ( double ) numElevationPoints ;
          elevation = fabs(elevation - pointAddrP(dtmP,point)->z ) ;
          (elevDiffP+numPointDiffs)->elevation = elevation ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
   }
/*
** Check Consistency Of points Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointDiffs = %8ld",numPointDiffs) ;
 if( numPointDiffs != dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Point Differences") ;
    goto errexit ;
   }
/*
**  Quick Sort Elevation Difference Structure
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
 qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmFilter_elevationDifferenceCompareFunction) ;
/*
** Mark All Points That Can Be Removed And Copy All Filtered Points To Filtered Points DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 for( n = 0 ; n < numPointDiffs ; ++n )
   {
    if( (elevDiffP+n)->elevation <= filterTolerance )
      {
       nodeAddrP(dtmP,(elevDiffP+n)->point)->tPtr = 1 ;
       pointP = pointAddrP(dtmP,(elevDiffP+n)->point) ;
       dtmPoint.x = pointP->x ;
       dtmPoint.y = pointP->y ;
       dtmPoint.z = pointP->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(filteredPtsP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ;
      } 
   }
/*
**  Remove Deleted Points From DTM
*/
 for( node1 = node2 = 0 ; node2 < dtmP->numPoints ; ++node2 )
   {
    if( nodeAddrP(dtmP,node2)->tPtr == dtmP->nullPnt )
      {
       if( node2 != node1 )
         {
          *pointAddrP(dtmP,node1) = *pointAddrP(dtmP,node2) ;
         }
        ++node1 ;
       }
    } 
  dtmP->numPoints = node1 ;
  if( dbg ) bcdtmWrite_message(0,0,0,"After Removing Filtered Points ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Free Nodes memory
*/
 if( dtmP->nodesPP != NULL ) 
   { 
    for( m = 0 ; m < dtmP->numPointPartitions ; ++m ) free(dtmP->nodesPP[m]) ;
    free( dtmP->nodesPP) ;
    dtmP->nodesPP = NULL  ; 
    dtmP->numNodePartitions = 0 ;
    dtmP->numNodes = 0 ;
    dtmP->memNodes = 0 ;
   }
/*
** Free Circular List Memory
*/
 if( dtmP->cListPP != NULL ) 
   { 
    for( m = 0 ; m < dtmP->numClistPartitions ; ++m ) free(dtmP->cListPP[m]) ;
    free( dtmP->cListPP) ;
    dtmP->cListPP = NULL  ; 
    dtmP->numClistPartitions = 0 ;
    dtmP->numClist = 0 ;
    dtmP->memClist = 0 ;
    dtmP->cListPtr = 0 ;
    dtmP->cListDelPtr = dtmP->nullPtr ;
   }
/*
**  Reset DTM Header Values
*/
 dtmP->dtmState        = DTMState::Data ;
 dtmP->hullPoint       = dtmP->nullPnt ;
 dtmP->nextHullPoint   = dtmP->nullPnt ;
 dtmP->numSortedPoints = 0 ;
 dtmP->numLines        = 0 ;
 dtmP->numTriangles    = 0 ;
/*
**  Set Number Of Spots
*/
 *numFilteredPtsP = filteredPtsP->numPoints ;  
/*
** Clean Up
*/
 cleanup :
 if( planePtsP != NULL ) bcdtmObject_destroyDtmObject(&planePtsP) ;
 if( elevDiffP != NULL ) free(elevDiffP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin z Tolerance Filter Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin z Tolerance Filter Random Spots Error") ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_tinZToleranceFilterGroupSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object           */
 long   filterOption,                  /* ==> < 1-Least Squares Plane , 2 - Average >                      */
 long   boundaryOption,                /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */
 double filterTolerance,               /* ==> Filter Tolerance                */ 
 long   *numFilteredPtsP,              /* <== Number Of Points After Filter   */
 BC_DTM_OBJ *filteredPtsP              /* <== Pointer To DTM Object With The Filtered Points   */
)
/*
** RobC - Note This Is A Special Purpose Multi Resolution Filtering Function And Should Not
** Be Used Out Of Its Current Context
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long point,point1,tile,numPointDiffs ;
 long numPoints,memPoints=0,numFilteredPoints,numUnFilteredPoints ; 
 long firstPoint,dtmFeature,saveNumPoints,usePlane=TRUE,excludeBoundary=TRUE ; 
 long clPtr,numElevationPoints ;
 double elevation ;
 DPoint3d    *p3dP,*pointsP=NULL ;
 struct ElevDifference { double elevation ; long point ; } *eldP,*elevDiffP=NULL ;
 DTM_PLANE plane ;
 DPoint3d *pointP ;
 DTM_TIN_NODE  *nodeP ;
 DTM_CIR_LIST  *clistP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ *tempDtmP=NULL,*planePtsP=NULL ;
 DTMFeatureId dtmFeatureId ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin z Tolerance Filter Group Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;    
    bcdtmWrite_message(0,0,0,"dtmP->numPoints   = %8ld",dtmP->numPoints) ;    
    bcdtmWrite_message(0,0,0,"filterOption      = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"boundaryOption    = %8ld",boundaryOption) ;
    bcdtmWrite_message(0,0,0,"filterTolerance   = %8.3lf",filterTolerance) ;
    bcdtmWrite_message(0,0,0,"numFilteredPtsP   = %8ld",*numFilteredPtsP) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP      = %p",filteredPtsP) ;    
   }
/*
** Check Parameters
*/
 if( filterTolerance < 0.0 ) filterTolerance = 0.0 ;
 if( filterOption < 1 || filterOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Filter Option") ;
    goto errexit ;
   }
 if( boundaryOption < 1 || boundaryOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Boundary Option") ;
    goto errexit ;
   }
/*
** Initialise
*/
 *numFilteredPtsP = 0 ;
 if( bcdtmObject_initialiseDtmObject(filteredPtsP)) goto errexit ;
 if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
 saveNumPoints = dtmP->numPoints ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numfeatures = %8ld",dtmP->numFeatures) ;
/*
** Check For Valid DTM
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   }  
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 dtmP->edgeOption = 2 ;
 dtmP->ppTol = dtmP->plTol = 0.0  ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Check Triangulation Has Not Removed Duplicate Points
*/
 if( dtmP->numPoints != saveNumPoints )
   {
    bcdtmWrite_message(1,0,0,"Duplicate Points In Filter Data Set") ;
    goto errexit ;
   }
/*
** Create DTM To Store Plane Points
*/
 if( usePlane == TRUE )
   {
    if( bcdtmObject_createDtmObject(&planePtsP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(planePtsP,100,100) ;
   }
/*
** Allocate memory For Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Scan All Internal Tin Points And Determine Elevation Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 numPointDiffs = 0 ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
/*
**  Least Squares Plane Filter Points
*/
    if( usePlane == TRUE )
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          planePtsP->numPoints = 0 ;
          clPtr = nodeP->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
/*
**           Store Point In DTM Object
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Point") ;
             pointP = pointAddrP(dtmP,point1) ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(planePtsP,DTMFeatureType::RandomSpots,planePtsP->nullUserTag,1,&planePtsP->nullFeatureId,(DPoint3d *)pointP,1)) goto errexit ;
            }
/*
**        Calculate Least Squares Plane Through Points
*/
          if( planePtsP->numPoints > 3 )
            {
             if( bcdtmFilter_findPlaneDtmObject(planePtsP,0,planePtsP->numPoints,&plane) ) goto errexit ;
             pointP = pointAddrP(dtmP,point) ;
             elevation = pointP->z - ( plane.A * pointP->x + plane.B * pointP->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(elevation) ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            }
          else
            {
             (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            } 
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
/*
**  Average Filter points
*/
    else
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          clPtr = nodeP->cPtr ;
          elevation = 0.0 ;
          numElevationPoints = 0 ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             elevation = elevation + pointAddrP(dtmP,point1)->z ;
             ++numElevationPoints ;
            } 
          elevation = elevation / ( double ) numElevationPoints ;
          elevation = fabs(elevation - pointAddrP(dtmP,point)->z ) ;
          (elevDiffP+numPointDiffs)->elevation = elevation ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
   }
/*
** Check Consistency Of points Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointDiffs = %8ld",numPointDiffs) ;
 if( numPointDiffs != dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Point Differences") ;
//    goto errexit ;
   }
/*
** QSort Elevation Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Qsorting Elevation Differences") ;
 qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmFilter_elevationDifferenceCompareFunction) ;
/*
**  Mark All Points That Can Be Removed 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
 for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
   {
    if( eldP->elevation <= filterTolerance ) nodeAddrP(dtmP,eldP->point)->tPtr = eldP->point ;
   }
/*
** Count Number Of Filtered And Unfiltered Points
*/
 if( cdbg == 2 )
   {
    numPoints = numFilteredPoints = numUnFilteredPoints = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
         {
          firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do 
            {
             ++numPoints ;
             if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) ++numUnFilteredPoints ;
             else                                                    ++numFilteredPoints ;  
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
         }
       else
         {
          bcdtmWrite_message(1,0,0,"ERROR - Feature Not In Tin State") ;
         }
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPoints = %6ld ** numFilteredPoints = %8ld numUnfilteredPoints = %8ld",numPoints,numFilteredPoints,numUnFilteredPoints) ; 
   }
/*
** Copy Filtered And Unfiltered Points To Different DTMs
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Filtered And Unfiltered Points") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
/*
**     Count Number Of Filtered And Un Filtered Points 
*/      
       numPoints = numFilteredPoints = numUnFilteredPoints = 0 ;
       firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
       tile        = (long)dtmFeatureP->dtmUserTag ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Processing Tile %8ld",tile) ;  
       do
         {
          ++numPoints ;
          if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) ++numUnFilteredPoints ;
          else                                                    ++numFilteredPoints ;  
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
         } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
/*
**     Check memory
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Checking memory") ;
       if( numPoints >= memPoints )
         {
          memPoints = numPoints * 2 ;
          if( pointsP == NULL ) pointsP = ( DPoint3d *) malloc( memPoints * sizeof(DPoint3d)) ;
          else                  pointsP = ( DPoint3d *) realloc(pointsP,memPoints * sizeof(DPoint3d)) ;
          if( pointsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            } 
         }   
/*
**     Store Filtered Points In Filtered Points In FilteredPts DTM
*/
       if( numFilteredPoints > 0 )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Filtered Points") ;
          p3dP = pointsP ;         
          firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do
            {
             if( nodeAddrP(dtmP,firstPoint)->tPtr != dtmP->nullPnt ) 
               {
                pointP = pointAddrP(dtmP,firstPoint) ;
                p3dP->x = pointP->x ;
                p3dP->y = pointP->y ;
                p3dP->z = pointP->z ;
                ++p3dP ;
               }
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(filteredPtsP,DTMFeatureType::GroupSpots,tile,3,&dtmFeatureId,pointsP,numFilteredPoints)) goto errexit ;
         }
/*
**     Store Un Filtered Points In Temp DTM
*/
       if( numUnFilteredPoints > 0 )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Un Filtered Points") ;
          p3dP = pointsP ;         
          firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
          tile        = (long)dtmFeatureP->dtmUserTag ;
          do
            {
             if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) 
               {
                pointP = pointAddrP(dtmP,firstPoint) ;
                p3dP->x = pointP->x ;
                p3dP->y = pointP->y ;
                p3dP->z = pointP->z ;
                ++p3dP ;
               }
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::GroupSpots,tile,3,&dtmFeatureId,pointsP,numUnFilteredPoints)) goto errexit ;
         }
      } 
   }
/*
**  Copy Temp DTM To DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Temp DTM") ;
 if( bcdtmObject_initialiseDtmObject(dtmP)) goto errexit ;
 if( tempDtmP->numPoints > 0 )
   {
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,tempDtmP->numPoints,tempDtmP->numPoints)) goto errexit ; 
    if( bcdtmObject_appendDtmObject(dtmP,tempDtmP)) goto errexit ;
   } 
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points Before Filter = %8ld",saveNumPoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Points After  Filter = %8ld",dtmP->numPoints+filteredPtsP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints         = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP->numPoints = %8ld",filteredPtsP->numPoints) ;
    bcdtmWrite_toFileDtmObject(filteredPtsP,L"filteredPoints.dtm") ;
    bcdtmWrite_toFileDtmObject(dtmP,L"unFilteredPoints.dtm") ;
   }
/*
** Check That The Total Number Of Points Before And After Filter Are The Same
*/
 if( saveNumPoints != dtmP->numPoints+filteredPtsP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Warning ** Inconsistent Number Of Filter Points") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( pointsP     != NULL ) free(pointsP) ;
 if( tempDtmP    != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 if( planePtsP   != NULL ) bcdtmObject_destroyDtmObject(&planePtsP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin z Tolerance Filter Group Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin z Tolerance Filter Group Spots Error") ;
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
BENTLEYDTM_Private int bcdtmFilter_elevationDifferenceTileCompareFunction(const void *p1P,const void *p2P)
/*
** Compare Function For Qsort Of Void Points
*/
{
 struct ElevDifference { double elevation ; long point ; long tile ; long keep ; } *ed1P,*ed2P ;
 ed1P = ( struct ElevDifference * ) p1P ;
 ed2P = ( struct ElevDifference * ) p2P ;
 if     (  ed1P->tile < ed2P->tile  ) return(-1) ;
 else if(  ed1P->tile > ed2P->tile  ) return( 1) ;
 else if(  ed1P->elevation < ed2P->elevation  ) return(-1) ;
 else if(  ed1P->elevation > ed2P->elevation  ) return( 1)  ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFilter_tinDecimateGroupSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object           */
 long   filterOption,                  /* ==> < 1-Least Squares Plane , 2 - Average >                      */
 long   boundaryOption,                /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */
 long   numPointsRemove,               /* ==> Number Of Points To Remove      */ 
 long   *numFilteredPtsP,              /* <== Number Of Points After Filter   */
 BC_DTM_OBJ *filteredPtsP,             /* <== Pointer To DTM Object With The Filtered Points   */
 double  *filterToleranceUsedP         /* <== Filter Tolerance Used To Decimate                */
)
/*
** RobC - Note This Is A Special Purpose Multi Resolution Filtering Function And Should Not
** Be Used Out Of Its Current Context
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long point,point1,tile,numPointDiffs ;
 long numPoints,memPoints=0,numFilteredPoints,numUnFilteredPoints ; 
 long firstPoint,dtmFeature,saveNumPoints,usePlane=TRUE,excludeBoundary=TRUE ; 
 long clPtr,numElevationPoints ;
 double elevation ;
 DPoint3d    *p3dP,*pointsP=NULL ;
 struct ElevDifference { double elevation ; long point ; } *eldP,*elevDiffP=NULL ;
 DTM_PLANE plane ;
 DPoint3d *pointP ;
 DTM_TIN_NODE  *nodeP ;
 DTM_CIR_LIST  *clistP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ *tempDtmP=NULL,*planePtsP=NULL ;
 DTMFeatureId dtmFeatureId ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin Decimating Group Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;    
    bcdtmWrite_message(0,0,0,"dtmP->numPoints   = %8ld",dtmP->numPoints) ;    
    bcdtmWrite_message(0,0,0,"filterOption      = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"boundaryOption    = %8ld",boundaryOption) ;
    bcdtmWrite_message(0,0,0,"numPointsRemove   = %8ld",numPointsRemove) ;
    bcdtmWrite_message(0,0,0,"numFilteredPtsP   = %8ld",*numFilteredPtsP) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP      = %p",filteredPtsP) ;    
   }
/*
** Check Parameters
*/
 if( filterOption < 1 || filterOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Filter Option") ;
    goto errexit ;
   }
 if( boundaryOption < 1 || boundaryOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Boundary Option") ;
    goto errexit ;
   }
/*
** Initialise
*/
 *numFilteredPtsP = 0 ;
 *filterToleranceUsedP = 0.0 ;
 if( bcdtmObject_initialiseDtmObject(filteredPtsP)) goto errexit ;
 if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
 saveNumPoints = dtmP->numPoints ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numfeatures = %8ld",dtmP->numFeatures) ;
/*
** Check For Valid DTM
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   }  
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 dtmP->edgeOption = 2 ;
 dtmP->ppTol = dtmP->plTol = 0.0  ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Check Triangulation Has Not Removed Duplicate Points
*/
 if( dtmP->numPoints != saveNumPoints )
   {
    bcdtmWrite_message(1,0,0,"Duplicate Points In Filter Data Set") ;
    goto errexit ;
   }
/*
** Create DTM To Store Plane Points
*/
 if( usePlane == TRUE )
   {
    if( bcdtmObject_createDtmObject(&planePtsP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(planePtsP,100,100) ;
   }
/*
** Allocate memory For Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Scan All Internal Tin Points And Determine Elevation Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 numPointDiffs = 0 ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
/*
**  Least Squares Plane Filter Points
*/
    if( usePlane == TRUE )
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          planePtsP->numPoints = 0 ;
          clPtr = nodeP->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
/*
**           Store Point In DTM Object
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Point") ;
             pointP = pointAddrP(dtmP,point1) ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(planePtsP,DTMFeatureType::RandomSpots,planePtsP->nullUserTag,1,&planePtsP->nullFeatureId,(DPoint3d *)pointP,1)) goto errexit ;
            }
/*
**        Calculate Least Squares Plane Through Points
*/
          if( planePtsP->numPoints > 3 )
            {
             if( bcdtmFilter_findPlaneDtmObject(planePtsP,0,planePtsP->numPoints,&plane) ) goto errexit ;
             pointP = pointAddrP(dtmP,point) ;
             elevation = pointP->z - ( plane.A * pointP->x + plane.B * pointP->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(elevation) ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            }
          else
            {
             (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            } 
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
/*
**  Average Filter points
*/
    else
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          clPtr = nodeP->cPtr ;
          elevation = 0.0 ;
          numElevationPoints = 0 ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             elevation = elevation + pointAddrP(dtmP,point1)->z ;
             ++numElevationPoints ;
            } 
          elevation = elevation / ( double ) numElevationPoints ;
          elevation = fabs(elevation - pointAddrP(dtmP,point)->z ) ;
          (elevDiffP+numPointDiffs)->elevation = elevation ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
   }
/*
** Check Consistency Of points Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointDiffs = %8ld",numPointDiffs) ;
 if( numPointDiffs != dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Point Differences") ;
//    goto errexit ;
   }
/*
** Adjust Number Of Points To Remove
*/
 if( numPointsRemove > numPointDiffs ) numPointsRemove = numPointDiffs ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointsRemove = %8ld",numPointsRemove) ;
/*
** QSort Elevation Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Qsorting Elevation Differences") ;
 qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmFilter_elevationDifferenceCompareFunction) ;
/*
**  Mark All Points That Can Be Removed 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
 for( eldP = elevDiffP ; eldP < elevDiffP + numPointsRemove ; ++eldP )
   {
    nodeAddrP(dtmP,eldP->point)->tPtr = eldP->point ;
    if( eldP->elevation > *filterToleranceUsedP ) *filterToleranceUsedP = eldP->elevation ;
   }
/*
** Count Number Of Filtered And Unfiltered Points
*/
 if( cdbg == 2 )
   {
    numPoints = numFilteredPoints = numUnFilteredPoints = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
         {
          firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do 
            {
             ++numPoints ;
             if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) ++numUnFilteredPoints ;
             else                                                    ++numFilteredPoints ;  
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
         }
       else
         {
          bcdtmWrite_message(1,0,0,"ERROR - Feature Not In Tin State") ;
         }
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPoints = %6ld ** numFilteredPoints = %8ld numUnfilteredPoints = %8ld",numPoints,numFilteredPoints,numUnFilteredPoints) ; 
   }
/*
** Copy Filtered And Unfiltered Points To Different DTMs
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Filtered And Unfiltered Points") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
/*
**     Count Number Of Filtered And Un Filtered Points 
*/      
       numPoints = numFilteredPoints = numUnFilteredPoints = 0 ;
       firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
       tile        = (long)dtmFeatureP->dtmUserTag ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Processing Tile %8ld",tile) ;  
       do
         {
          ++numPoints ;
          if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) ++numUnFilteredPoints ;
          else                                                    ++numFilteredPoints ;  
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
         } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
/*
**     Check memory
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Checking memory") ;
       if( numPoints >= memPoints )
         {
          memPoints = numPoints * 2 ;
          if( pointsP == NULL ) pointsP = ( DPoint3d *) malloc( memPoints * sizeof(DPoint3d)) ;
          else                  pointsP = ( DPoint3d *) realloc(pointsP,memPoints * sizeof(DPoint3d)) ;
          if( pointsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            } 
         }   
/*
**     Store Filtered Points In Filtered Points In FilteredPts DTM
*/
       if( numFilteredPoints > 0 )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Filtered Points") ;
          p3dP = pointsP ;         
          firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do
            {
             if( nodeAddrP(dtmP,firstPoint)->tPtr != dtmP->nullPnt ) 
               {
                pointP = pointAddrP(dtmP,firstPoint) ;
                p3dP->x = pointP->x ;
                p3dP->y = pointP->y ;
                p3dP->z = pointP->z ;
                ++p3dP ;
               }
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(filteredPtsP,DTMFeatureType::GroupSpots,tile,3,&dtmFeatureId,pointsP,numFilteredPoints)) goto errexit ;
         }
/*
**     Store Un Filtered Points In Temp DTM
*/
       if( numUnFilteredPoints > 0 )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Un Filtered Points") ;
          p3dP = pointsP ;         
          firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
          tile        = (long)dtmFeatureP->dtmUserTag ;
          do
            {
             if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) 
               {
                pointP = pointAddrP(dtmP,firstPoint) ;
                p3dP->x = pointP->x ;
                p3dP->y = pointP->y ;
                p3dP->z = pointP->z ;
                ++p3dP ;
               }
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::GroupSpots,tile,3,&dtmFeatureId,pointsP,numUnFilteredPoints)) goto errexit ;
         }
      } 
   }
/*
**  Copy Temp DTM To DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Temp DTM") ;
 if( bcdtmObject_initialiseDtmObject(dtmP)) goto errexit ;
 if( tempDtmP->numPoints > 0 )
   {
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,tempDtmP->numPoints,tempDtmP->numPoints)) goto errexit ; 
    if( bcdtmObject_appendDtmObject(dtmP,tempDtmP)) goto errexit ;
   } 
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points Before Filter = %8ld",saveNumPoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Points After  Filter = %8ld",dtmP->numPoints+filteredPtsP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints         = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP->numPoints = %8ld",filteredPtsP->numPoints) ;
    bcdtmWrite_toFileDtmObject(filteredPtsP,L"filteredPoints.dtm") ;
    bcdtmWrite_toFileDtmObject(dtmP,L"unFilteredPoints.dtm") ;
   }
/*
** Check That The Total Number Of Points Before And After Filter Are The Same
*/
 if( saveNumPoints != dtmP->numPoints+filteredPtsP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Warning ** Inconsistent Number Of Filter Points") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( pointsP     != NULL ) free(pointsP) ;
 if( tempDtmP    != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 if( planePtsP   != NULL ) bcdtmObject_destroyDtmObject(&planePtsP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Decimating Group Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Decimating Group Spots Error") ;
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
BENTLEYDTM_Public int bcdtmFilter_tileDecimateGroupSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object                            */
 long   numPointsRemove,               /* ==> Number Of Points To Remove                       */ 
 long   *numFilteredSpotsP,            /* <== Number Of Spots After Filter                     */
 BC_DTM_OBJ *filteredDtmP,             /* <== Pointer To DTM Object With The Filtered Points   */
 double  *filterToleranceUsedP         /* <== Filter Tolerance Used To Decimate                */
)
/*
** RobC - Note This Is A Special Multi Resolution Filtering Function And Should Not
** Be Used Out Of Its Current Context
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long point,saveLastPoint,numPointDiffs,numPoints=0,memPoints=0,numRemovePoints ; 
 long pnt,firstPoint,lastPoint,maxTilePts,dtmFeature,startTime,lastTile,lastKeep ;
 long tile,numTiles,*tnumP,*tileNumberP=NULL,removeMethod=2 ;
 double dZ,xMin,yMin,zMin,xMax,yMax,zMax ;
 DPoint3d    *p3dP,*pointsP=NULL ;
 struct ElevDifferenceTile { double elevation ; long point ; long tile ; long keep ; } *eldP,*eld1P,*elevDiffP=NULL ;
 DPoint3d *pntP ;
 DTM_PLANE plane ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ *tempDtmP=NULL ;
 DTMFeatureId dtmFeatureId ;
 DTM_POINT_TILE *tilesP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tile Decimate Group Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;    
    bcdtmWrite_message(0,0,0,"numPointsRemove   = %8ld",numPointsRemove) ;
    bcdtmWrite_message(0,0,0,"numFilteredSpotsP = %8ld",*numFilteredSpotsP) ;
   }
/*
** Initialise
*/
 *numFilteredSpotsP = 0 ;
 *filterToleranceUsedP = 0.0 ;
/*
** Allocate Memory To Store Point Tile Numbers
*/
 tileNumberP = ( long * ) malloc ( dtmP->numPoints * sizeof(long)) ;
 if( tileNumberP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
 if( cdbg ) for( tnumP = tileNumberP ; tnumP < tileNumberP + dtmP->numPoints ; ++tnumP ) *tnumP = -9999 ;
/*
** Populate Tile Number Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Tile Number Array") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
    lastPoint   = firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ;
    for( tnumP = tileNumberP + firstPoint ; tnumP <= tileNumberP + lastPoint ; ++tnumP )
      {
       *tnumP = (long ) dtmFeatureP->dtmUserTag ;
      } 
   }
/*
** Check All Points Have Been Allocated A Tile Number
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking All Points Have A Tile Number") ;
    for( tnumP = tileNumberP ; tnumP < tileNumberP + dtmP->numPoints ; ++tnumP )
      {
       if( *tnumP == -9999 ) 
         {
          bcdtmWrite_message(1,0,0,"Point %8ld Has Not Been Allocated A Tile Number",(long)(tnumP-tileNumberP)) ;
          goto errexit ;
         } 
      } 
   }
/*
** Sort Points On x Axis
*/
 firstPoint = 0 ;
 lastPoint  = dtmP->numPoints - 1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Points") ;
 if( bcdtmFilter_sortTaggedPointRangeDtmObject(dtmP,tileNumberP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeTaggedDuplicatePointsFromRangeDtmObject(dtmP,tileNumberP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
 if( saveLastPoint-lastPoint > 0 ) bcdtmWrite_message(0,0,0,"Warning **** Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ; 
/*
**  Tile Dtm Points
*/
 startTime = bcdtmClock() ;
 maxTilePts = 5 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tiling XYZ Points Into Tiles With A Maximum Of %8ld Points Per Tile",maxTilePts) ;
// if( bcdtmTile_pointsDtmObject(dtmP,tileNumberP,firstPoint,dtmP->numPoints,maxTilePts,&tilesP,&numTiles)) goto errexit ;
 if( bcdtmMedianTile_pointsDtmObject(dtmP,tileNumberP,firstPoint,dtmP->numPoints,maxTilePts,&tilesP,&numTiles)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Tile %8ld Points Into %8ld Tiles = %8.3lf Secs",dtmP->numPoints,numTiles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Write Tiles
*/
 if( dbg == 2 )
   {
    for( tile = 0 ; tile  < numTiles ; ++tile )
      {
       pntP = pointAddrP(dtmP,(tilesP+tile)->tileOffset) ;
       xMin = xMax = pntP->x ;
       yMin = yMax = pntP->y ;
       zMin = zMax = pntP->z ;
       for( pnt = (tilesP+tile)->tileOffset ; pnt < (tilesP+tile)->tileOffset + (tilesP+tile)->numTilePts ; ++pnt )
         {
          pntP = pointAddrP(dtmP,pnt) ;
          if( pntP->x < xMin ) xMin = pntP->x ;
          if( pntP->x > xMax ) xMax = pntP->x ;
          if( pntP->y < yMin ) yMin = pntP->y ;
          if( pntP->y > yMax ) yMax = pntP->y ;
          if( pntP->z < zMin ) zMin = pntP->z ;
          if( pntP->z > zMax ) zMax = pntP->z ;
         }
       bcdtmWrite_message(0,0,0,"tile[%4ld] ** startPoint = %8ld numPoints = %8ld ** xMin = %12.5lf xMax = %12.5lf ** yMin = %12.5lf yMax = %12.5lf ** zMin = %10.4lf zMax = %10.4lf",tile,(tilesP+tile)->tileOffset,(tilesP+tile)->numTilePts,xMin,xMax,yMin,yMax,zMin,zMax) ;
      }
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifferenceTile * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifferenceTile)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Filter Tiles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Least Square Filtering Tiles") ;
 numPointDiffs = 0 ;
 for( tile = 0 ; tile  < numTiles ; ++tile )
   {
/*
**  Calculate Least Squares Plane Through The Tile Points
*/
    if( (tilesP+tile)->numTilePts > 3 )
      {
       plane.A = 0.0 ;
       plane.B = 0.0 ;
       plane.C = 0.0 ;  
       if( bcdtmFilter_findPlaneDtmObject(dtmP,(tilesP+tile)->tileOffset,(tilesP+tile)->numTilePts,&plane) == DTM_SUCCESS )
         {
          for( pnt = (tilesP+tile)->tileOffset ; pnt < (tilesP+tile)->tileOffset + (tilesP+tile)->numTilePts ; ++pnt )
            {
             dZ = pointAddrP(dtmP,pnt)->z - ( plane.A * pointAddrP(dtmP,pnt)->x + plane.B * pointAddrP(dtmP,pnt)->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(dZ) ;
             (elevDiffP+numPointDiffs)->point     = pnt ;
             (elevDiffP+numPointDiffs)->tile      = *(tileNumberP+pnt) ;
             (elevDiffP+numPointDiffs)->keep      = 1 ;
             ++numPointDiffs ;
            }
         }
      }
    else
      {
       for( pnt = (tilesP+tile)->tileOffset ; pnt < (tilesP+tile)->tileOffset + (tilesP+tile)->numTilePts ; ++pnt )
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = pnt ;
          (elevDiffP+numPointDiffs)->tile      = *(tileNumberP+pnt) ;
          (elevDiffP+numPointDiffs)->keep      = 1 ;
           ++numPointDiffs ;
         }
      }  
   }
/*
** Adjust Number Of Points To Remove
*/
 if( numPointsRemove > numPointDiffs ) numPointsRemove = numPointDiffs ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointsRemove = %8ld",numPointsRemove) ;
/*
** Remove Method 1
*/
 if( removeMethod == 1 )
   {
/*
**  Quick Sort Elevation Difference Structure
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
    qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifferenceTile),bcdtmFilter_elevationDifferenceTileCompareFunction) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
       for( eldP = elevDiffP ; eldP < elevDiffP + 1000 ; ++eldP )
         {
          bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** point = %8ld tile = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->point,eldP->tile) ;
         } 
      }
    if( dbg )  bcdtmWrite_message(0,0,0,"Elevation Point Difference Of Last Remove Point = %10.4lf",(elevDiffP+numPointsRemove-1)->elevation) ;
/*
**  Mark All Points That Can Be Removed By Removing Equal Numbers From All Tiles
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
    eldP = elevDiffP ;
    lastTile = eldP->tile ;
    lastPoint = firstPoint = 0 ;
    while( eldP < elevDiffP + numPointDiffs )
      {
       while( eldP < elevDiffP + numPointDiffs && eldP->tile == lastTile ) ++eldP ;
       --eldP ;
       lastPoint = (long)(eldP-elevDiffP) ;
       numPoints = lastPoint - firstPoint + 1 ;
       numRemovePoints = firstPoint + numPoints / 2 ;
       for( eld1P = elevDiffP + firstPoint ; eld1P < elevDiffP + numRemovePoints ; ++eld1P ) 
         {
          eld1P->keep = 0 ;
          if( eld1P->elevation > *filterToleranceUsedP ) *filterToleranceUsedP = eld1P->elevation ;
         }
       for( eld1P = elevDiffP + numRemovePoints + 1 ; eld1P < elevDiffP + numPoints ; ++eld1P ) eld1P->keep = 1 ;
/*
**     Reset For Next Tile
*/
       ++eldP ;
       if( eldP < elevDiffP + numPointDiffs )
         {
          lastTile = eldP->tile ;
          lastPoint = firstPoint = (long)(eldP-elevDiffP) ;
         }
      }
   }
/*
**  Remove Method Two
*/
 if( removeMethod == 2 )
   {
/*
**  Quick Sort Elevation Difference Structure
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
    qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifferenceTile),bcdtmFilter_elevationDifferenceKeepCompareFunction) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
       for( eldP = elevDiffP ; eldP < elevDiffP + 1000 ; ++eldP )
         {
          bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** point = %8ld tile = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->point,eldP->tile) ;
         } 
      }
    if( dbg )  bcdtmWrite_message(0,0,0,"Elevation Point Difference Of Last Remove Point = %10.4lf",(elevDiffP+numPointsRemove-1)->elevation) ;
/*
**  Mark All Points That Can Be Removed By Removing The Points With The Smallest Difference
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
    for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
      {
       if( (long)(eldP-elevDiffP) < numPointsRemove ) 
         {
          eldP->keep = 0 ;
          if( eldP->elevation > *filterToleranceUsedP ) *filterToleranceUsedP = eldP->elevation ;
         } 
       else eldP->keep = 1 ;
      }
   }
/*
** Sort Points Into Tiles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Points Tiles") ;
 qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifferenceTile),bcdtmFilter_elevationDifferenceTileKeepCompareFunction) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
//    for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
    for( eldP = elevDiffP ; eldP < elevDiffP + 1000 ; ++eldP )
      {
       bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** keep = %2ld point = %8ld tile = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->keep,eldP->point,eldP->tile) ;
      } 
   }
/*
** Copy Filtered Points To Filtered DTM
** Copy Unfiltered Points To Temp DTM
*/
 if( dbg )  bcdtmWrite_message(0,0,0,"Copying Filtered And Unfiltered Points") ;
 if( bcdtmObject_initialiseDtmObject(filteredDtmP)) goto errexit ;
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(filteredDtmP,numPointsRemove,numPointsRemove)) goto errexit ;
 if( numPointsRemove < numPointDiffs )
   { 
    if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,numPointDiffs-numPointsRemove,1000)) goto errexit ;
   }
 eldP = elevDiffP ;
 lastTile = eldP->tile ;
 lastKeep = eldP->keep ;
 lastPoint = firstPoint = 0 ;
 while( eldP < elevDiffP + numPointDiffs )
   {
    while( eldP < elevDiffP + numPointDiffs && eldP->keep == lastKeep && eldP->tile == lastTile ) ++eldP ;
    --eldP ;
    lastPoint = (long)(eldP-elevDiffP) ;
    numPoints = lastPoint - firstPoint + 1 ;
    if( numPoints > memPoints )
      {
       memPoints = numPoints * 2 ;
       if( pointsP == NULL ) pointsP = (DPoint3d *) malloc( memPoints * sizeof(DPoint3d)) ;
       else                  pointsP = (DPoint3d *) realloc( pointsP , memPoints * sizeof(DPoint3d)) ;
       if( pointsP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         } 
      }
/*
**  Accumulate Points In Point Array
*/
    p3dP = pointsP ;
    for( point = firstPoint ; point <= lastPoint ; ++point )
      {
       pntP = pointAddrP(dtmP,point) ;
       p3dP->x = pntP->x ;
       p3dP->y = pntP->y ;
       p3dP->z = pntP->z ;
       ++p3dP ;
      } 
/*
**  Store Points As Point Feature 
*/
    if( lastKeep == 0 )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"lastKeep = %2ld lastTile = %8ld ** numPoints = %8ld",lastKeep,lastTile,numPoints) ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(filteredDtmP,DTMFeatureType::GroupSpots,lastTile,3,&dtmFeatureId,pointsP,numPoints)) goto errexit ;
      }
    else
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"lastKeep = %2ld lastTile = %8ld ** numPoints = %8ld",lastKeep,lastTile,numPoints) ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::GroupSpots,lastTile,3,&dtmFeatureId,pointsP,numPoints)) goto errexit ;
      }
/*
**  Reset For Next Tile
*/
    ++eldP ;
    if( eldP < elevDiffP + numPointDiffs )
      {
       lastTile = eldP->tile ;
       lastKeep = eldP->keep ;
       lastPoint = firstPoint = (long)(eldP-elevDiffP) ;
      }
   }
/*
**  Copy Temp DTM To DTM
*/
 if( bcdtmObject_initialiseDtmObject(dtmP)) goto errexit ;
 if( tempDtmP->numPoints > 0 )
   {
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,tempDtmP->numPoints,tempDtmP->numPoints)) goto errexit ; 
    if( bcdtmObject_appendDtmObject(dtmP,tempDtmP)) goto errexit ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( tileNumberP != NULL ) free(tileNumberP) ;
 if( pointsP     != NULL ) free(pointsP) ;
 if( tempDtmP    != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Decimate Group Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Decimate Group Spots Error") ;
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
BENTLEYDTM_Public int bcdtmFilter_tileZToleranceFilterGroupSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object          */
 double filterTolerance,               /* ==> Number Of Points To Remove     */ 
 long   *numFilteredSpotsP,            /* <== Number Of Spots After Filter   */
 BC_DTM_OBJ *filteredDtmP              /* <== Pointer To DTM Object With The Filtered Points   */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long point,saveLastPoint,numPointDiffs,numPoints=0,memPoints=0,numRemovePoints,numPointsRemove ; 
 long pnt,firstPoint,lastPoint,maxTilePts,dtmFeature,startTime,lastTile,lastKeep ;
 long tile,numTiles,*tnumP,*tileNumberP=NULL,removeMethod=2 ;
 double dZ,xMin,yMin,zMin,xMax,yMax,zMax ;
 DPoint3d    *p3dP,*pointsP=NULL ;
 struct ElevDifferenceTile { double elevation ; long point ; long tile ; long keep ; } *eldP,*eld1P,*elevDiffP=NULL ;
 DPoint3d *pntP ;
 DTM_PLANE plane ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ *tempDtmP=NULL ;
 DTMFeatureId dtmFeatureId ;
 DTM_POINT_TILE *tilesP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tile z Tolerance Filter Group Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;    
    bcdtmWrite_message(0,0,0,"filterTolerance   = %8.3lf",filterTolerance) ;
    bcdtmWrite_message(0,0,0,"numFilteredSpotsP = %8ld",*numFilteredSpotsP) ;
   }
/*
** Initialise
*/
 *numFilteredSpotsP = 0 ;
/*
** Allocate Memory To Store Point Tile Numbers
*/
 tileNumberP = ( long * ) malloc ( dtmP->numPoints * sizeof(long)) ;
 if( tileNumberP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
 if( cdbg ) for( tnumP = tileNumberP ; tnumP < tileNumberP + dtmP->numPoints ; ++tnumP ) *tnumP = -9999 ;
/*
** Populate Tile Number Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Tile Number Array") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
    lastPoint   = firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ;
    for( tnumP = tileNumberP + firstPoint ; tnumP <= tileNumberP + lastPoint ; ++tnumP )
      {
       *tnumP = (long ) dtmFeatureP->dtmUserTag ;
      } 
   }
/*
** Check All Points Have Been Allocated A Tile Number
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking All Points Have A Tile Number") ;
    for( tnumP = tileNumberP ; tnumP < tileNumberP + dtmP->numPoints ; ++tnumP )
      {
       if( *tnumP == -9999 ) 
         {
          bcdtmWrite_message(1,0,0,"Point %8ld Has Not Been Allocated A Tile Number",(long)(tnumP-tileNumberP)) ;
          goto errexit ;
         } 
      } 
   }
/*
** Sort Points On x Axis
*/
 firstPoint = 0 ;
 lastPoint  = dtmP->numPoints - 1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"x Axis Sorting Points") ;
 if( bcdtmFilter_sortTaggedPointRangeDtmObject(dtmP,tileNumberP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeTaggedDuplicatePointsFromRangeDtmObject(dtmP,tileNumberP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
 if( saveLastPoint-lastPoint > 0 ) bcdtmWrite_message(0,0,0,"Warning **** Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ; 
/*
**  Tile Dtm Points
*/
 startTime = bcdtmClock() ;
 maxTilePts = 5 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tiling XYZ Points Into Tiles With A Maximum Of %8ld Points Per Tile",maxTilePts) ;
// if( bcdtmTile_pointsDtmObject(dtmP,tileNumberP,firstPoint,dtmP->numPoints,maxTilePts,&tilesP,&numTiles)) goto errexit ;
 if( bcdtmMedianTile_pointsDtmObject(dtmP,tileNumberP,firstPoint,dtmP->numPoints,maxTilePts,&tilesP,&numTiles)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Tile %8ld Points Into %8ld Tiles = %8.3lf Secs",dtmP->numPoints,numTiles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Write Tiles
*/
 if( dbg == 2 )
   {
    for( tile = 0 ; tile  < numTiles ; ++tile )
      {
       pntP = pointAddrP(dtmP,(tilesP+tile)->tileOffset) ;
       xMin = xMax = pntP->x ;
       yMin = yMax = pntP->y ;
       zMin = zMax = pntP->z ;
       for( pnt = (tilesP+tile)->tileOffset ; pnt < (tilesP+tile)->tileOffset + (tilesP+tile)->numTilePts ; ++pnt )
         {
          pntP = pointAddrP(dtmP,pnt) ;
          if( pntP->x < xMin ) xMin = pntP->x ;
          if( pntP->x > xMax ) xMax = pntP->x ;
          if( pntP->y < yMin ) yMin = pntP->y ;
          if( pntP->y > yMax ) yMax = pntP->y ;
          if( pntP->z < zMin ) zMin = pntP->z ;
          if( pntP->z > zMax ) zMax = pntP->z ;
         }
       bcdtmWrite_message(0,0,0,"tile[%4ld] ** startPoint = %8ld numPoints = %8ld ** xMin = %12.5lf xMax = %12.5lf ** yMin = %12.5lf yMax = %12.5lf ** zMin = %10.4lf zMax = %10.4lf",tile,(tilesP+tile)->tileOffset,(tilesP+tile)->numTilePts,xMin,xMax,yMin,yMax,zMin,zMax) ;
      }
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifferenceTile * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifferenceTile)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Filter Tiles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Least Square Filtering Tiles") ;
 numPointDiffs = 0 ;
 for( tile = 0 ; tile  < numTiles ; ++tile )
   {
/*
**  Calculate Least Squares Plane Through The Tile Points
*/
    if( (tilesP+tile)->numTilePts > 3 )
      {
       plane.A = 0.0 ;
       plane.B = 0.0 ;
       plane.C = 0.0 ;  
       if( bcdtmFilter_findPlaneDtmObject(dtmP,(tilesP+tile)->tileOffset,(tilesP+tile)->numTilePts,&plane) == DTM_SUCCESS )
         {
          for( pnt = (tilesP+tile)->tileOffset ; pnt < (tilesP+tile)->tileOffset + (tilesP+tile)->numTilePts ; ++pnt )
            {
             dZ = pointAddrP(dtmP,pnt)->z - ( plane.A * pointAddrP(dtmP,pnt)->x + plane.B * pointAddrP(dtmP,pnt)->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(dZ) ;
             (elevDiffP+numPointDiffs)->point     = pnt ;
             (elevDiffP+numPointDiffs)->tile      = *(tileNumberP+pnt) ;
             (elevDiffP+numPointDiffs)->keep      = 1 ;
             ++numPointDiffs ;
            }
         }
      }
    else
      {
       for( pnt = (tilesP+tile)->tileOffset ; pnt < (tilesP+tile)->tileOffset + (tilesP+tile)->numTilePts ; ++pnt )
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = pnt ;
          (elevDiffP+numPointDiffs)->tile      = *(tileNumberP+pnt) ;
          (elevDiffP+numPointDiffs)->keep      = 1 ;
           ++numPointDiffs ;
         }
      }  
   }
/*
** Remove Method 1
*/
 if( removeMethod == 1 )
   {
/*
**  Quick Sort Elevation Difference Structure
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
    qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifferenceTile),bcdtmFilter_elevationDifferenceTileCompareFunction) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
       for( eldP = elevDiffP ; eldP < elevDiffP + 1000 ; ++eldP )
         {
          bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** point = %8ld tile = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->point,eldP->tile) ;
         } 
      }
/*
**  Mark All Points That Can Be Removed By Removing Equal Numbers From All Tiles
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
    eldP = elevDiffP ;
    lastTile = eldP->tile ;
    lastPoint = firstPoint = 0 ;
    while( eldP < elevDiffP + numPointDiffs )
      {
       while( eldP < elevDiffP + numPointDiffs && eldP->tile == lastTile ) ++eldP ;
       --eldP ;
       lastPoint = (long)(eldP-elevDiffP) ;
       numPoints = lastPoint - firstPoint + 1 ;
       numRemovePoints = firstPoint + numPoints / 2 ;
       for( eld1P = elevDiffP + firstPoint ; eld1P < elevDiffP + numRemovePoints ; ++eld1P ) eld1P->keep = 0 ;
       for( eld1P = elevDiffP + numRemovePoints + 1 ; eld1P < elevDiffP + numPoints ; ++eld1P ) eld1P->keep = 1 ;
/*
**     Reset For Next Tile
*/
       ++eldP ;
       if( eldP < elevDiffP + numPointDiffs )
         {
          lastTile = eldP->tile ;
          lastPoint = firstPoint = (long)(eldP-elevDiffP) ;
         }
      }
   }
/*
**  Remove Method Two
*/
 if( removeMethod == 2 )
   {
/*
**  Quick Sort Elevation Difference Structure
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
    qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifferenceTile),bcdtmFilter_elevationDifferenceKeepCompareFunction) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
       for( eldP = elevDiffP ; eldP < elevDiffP + 1000 ; ++eldP )
         {
          bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** point = %8ld tile = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->point,eldP->tile) ;
         } 
      }
/*
**  Mark All Points That Can Be Removed By Removing The Points With The Smallest Difference
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
    numPointsRemove = 0 ;
    for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
      {
       if( eldP->elevation <= filterTolerance ) { eldP->keep = 0 ; ++numPointsRemove ; }
       else                                     eldP->keep = 1 ;
      }
   }
/*
** Sort Points Into Tiles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Points Tiles") ;
 qsortCPP(elevDiffP,numPointDiffs,sizeof(struct ElevDifferenceTile),bcdtmFilter_elevationDifferenceTileKeepCompareFunction) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
//    for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
    for( eldP = elevDiffP ; eldP < elevDiffP + 1000 ; ++eldP )
      {
       bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** keep = %2ld point = %8ld tile = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->keep,eldP->point,eldP->tile) ;
      } 
   }
/*
** Copy Filtered Points To Filtered DTM
** Copy Unfiltered Points To Temp DTM
*/
 if( dbg )  bcdtmWrite_message(0,0,0,"Copying Filtered And Unfiltered Points") ;
 if( bcdtmObject_initialiseDtmObject(filteredDtmP)) goto errexit ;
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(filteredDtmP,numPointsRemove,numPointsRemove)) goto errexit ;
 if( numPointsRemove < numPointDiffs )
   { 
    if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,numPointDiffs-numPointsRemove,1000)) goto errexit ;
   }
 eldP = elevDiffP ;
 lastTile = eldP->tile ;
 lastKeep = eldP->keep ;
 lastPoint = firstPoint = 0 ;
 while( eldP < elevDiffP + numPointDiffs )
   {
    while( eldP < elevDiffP + numPointDiffs && eldP->keep == lastKeep && eldP->tile == lastTile ) ++eldP ;
    --eldP ;
    lastPoint = (long)(eldP-elevDiffP) ;
    numPoints = lastPoint - firstPoint + 1 ;
    if( numPoints > memPoints )
      {
       memPoints = numPoints * 2 ;
       if( pointsP == NULL ) pointsP = (DPoint3d *) malloc( memPoints * sizeof(DPoint3d)) ;
       else                  pointsP = (DPoint3d *) realloc( pointsP , memPoints * sizeof(DPoint3d)) ;
       if( pointsP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         } 
      }
/*
**  Accumulate Points In Point Array
*/
    p3dP = pointsP ;
    for( point = firstPoint ; point <= lastPoint ; ++point )
      {
       pntP = pointAddrP(dtmP,point) ;
       p3dP->x = pntP->x ;
       p3dP->y = pntP->y ;
       p3dP->z = pntP->z ;
       ++p3dP ;
      } 
/*
**  Store Points As Point Feature 
*/
    if( lastKeep == 0 )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"lastKeep = %2ld lastTile = %8ld ** numPoints = %8ld",lastKeep,lastTile,numPoints) ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(filteredDtmP,DTMFeatureType::GroupSpots,lastTile,3,&dtmFeatureId,pointsP,numPoints)) goto errexit ;
      }
    else
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"lastKeep = %2ld lastTile = %8ld ** numPoints = %8ld",lastKeep,lastTile,numPoints) ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::GroupSpots,lastTile,3,&dtmFeatureId,pointsP,numPoints)) goto errexit ;
      }
/*
**  Reset For Next Tile
*/
    ++eldP ;
    if( eldP < elevDiffP + numPointDiffs )
      {
       lastTile = eldP->tile ;
       lastKeep = eldP->keep ;
       lastPoint = firstPoint = (long)(eldP-elevDiffP) ;
      }
   }
/*
**  Copy Temp DTM To DTM
*/
 if( bcdtmObject_initialiseDtmObject(dtmP)) goto errexit ;
 if( tempDtmP->numPoints > 0 )
   {
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,tempDtmP->numPoints,tempDtmP->numPoints)) goto errexit ; 
    if( bcdtmObject_appendDtmObject(dtmP,tempDtmP)) goto errexit ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( tileNumberP != NULL ) free(tileNumberP) ;
 if( pointsP     != NULL ) free(pointsP) ;
 if( tempDtmP    != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile z Tolerance Filter Group Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile z Tolerance Filter Group Spots Error") ;
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
BENTLEYDTM_EXPORT int bcdtmFilter_quadTree
(
 DTM_QUAD_TREE_TILE *quadTreeP,            // ==> Pointer To Quad Tree
 long numQuadTreeTiles,                    // ==> Number Of quad Tree Tiles
 long numQuadTreeLevels,                   // ==> Number Of Quad Tree Levels
 long decimationFactor,                    // ==> Decimation factor
 long decouplePoints,                      // ==> Number Of Index Level Points To Decouple The Filtering From The Tiles
 long coplanarFilterOption,                // ==> <0> Do Not Filter Coplanar Points <1> Filter Coplanar Points
 double coplanarFilterTolerance            // ==> z Filter Tolerance For Removing Coplanar Points
)
{
/*
**  This Function Filters Points Represented by a Depth Balanced Quadtree Index.
**  The Filtering Proceess Is A Two Step Process 
**  1. Filter Nodes Individually Until There Is A Set Number Of Points At The Quadtree Index Level
**  2. Once This Number Of Points Is Reached Combine All Nodes ( Decouple ) At The Index Level And Filter As One
**  
**  The Base ( leaf nodes ) Of The Index are not filtered. This could be done.  
*/   
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  n,level,point,dtmFeature,firstPoint,lastPoint,parentTile,numChildPoints,memChildPoints ; 
 long  numLevelPoints,numFilteredPoints,numPointsRemove,lastParentTile,internalTile ; 
 double filterToleranceUsed=0.0 ;
 DPoint3d   *p3dP,*childPointsP=NULL ;
 DTM_QUAD_TREE_TILE *tileP ;
 BC_DTM_OBJ *unFilteredPtsP=NULL,*filteredPtsP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ; 
 DPoint3d *pointP ;
 wchar_t fileName[32] ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Filtering Depth Balanced QuadTree Points") ;
    bcdtmWrite_message(0,0,0,"quadTreeP               = %p",quadTreeP) ;
    bcdtmWrite_message(0,0,0,"numQuadTreeTiles        = %8ld",numQuadTreeTiles) ;
    bcdtmWrite_message(0,0,0,"numQuadTreeLevels       = %8ld",numQuadTreeLevels) ;
    bcdtmWrite_message(0,0,0,"decimationFactor        = %8ld",decimationFactor) ;
    bcdtmWrite_message(0,0,0,"decouplePoints          = %8ld",decouplePoints) ;
    bcdtmWrite_message(0,0,0,"coplanarFilterOption    = %8ld",coplanarFilterOption) ;
    bcdtmWrite_message(0,0,0,"coplanarFilterTolerance = %8.3lf",coplanarFilterTolerance) ;
   }
/*
** Create Dtm Objects For Filtering Function
*/
 if( bcdtmObject_createDtmObject(&unFilteredPtsP)) goto errexit ;
 if( bcdtmObject_createDtmObject(&filteredPtsP)) goto errexit ;
/*
** Check For One Tile
*/
 if( numQuadTreeTiles == 1 )
   {
    if( coplanarFilterOption )
      {
       if( bcdtmFilter_tinZToleranceFilterRandomSpotsDtmObject(quadTreeP->dtmP,1,1,coplanarFilterTolerance,&numFilteredPoints,filteredPtsP)) goto errexit ;
       if( bcdtmObject_resizeMemoryDtmObject(quadTreeP->dtmP)) goto errexit ; 
       quadTreeP->numTilePts = quadTreeP->dtmP->numPoints ;
       if( dbg ) bcdtmWrite_toFileDtmObject(quadTreeP->dtmP,L"quadtreeFilter.dtm") ;
      }
   }
/*
** Filter From The Leaf Nodes Up
*/
 else
   {
    for( level = numQuadTreeLevels - 2 ; level >= 0 ; --level )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Index Level %2ld",level) ;
/*
**     Get Number Of Points At Lower Index Level
*/
       numLevelPoints = 0 ;
       for( tileP = quadTreeP ; tileP < quadTreeP + numQuadTreeTiles ; ++tileP )
         {
          if( tileP->quadTreeLevel == level + 1 ) numLevelPoints =  numLevelPoints + tileP->numTilePts ;
         } 
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points At Index Level %2ld = %8ld",level+1,numLevelPoints) ;
/*
**     Initialise DTM Objects - That Is Remove All Points
*/
       if( bcdtmObject_initialiseDtmObject(unFilteredPtsP)) goto errexit ;
       if( bcdtmObject_initialiseDtmObject(filteredPtsP)) goto errexit ;
/*
**     Filter The Tiles
*/
       if( numLevelPoints > decouplePoints )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Filtering Tiles") ;
          for( tileP = quadTreeP ; tileP < quadTreeP + numQuadTreeTiles ; ++tileP )
            {
             if( tileP->quadTreeLevel == level ) 
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Parent Node = %8ld ** Child Nodes = %8ld %8ld %8ld %8ld",tileP->tileNumber,tileP->childNodes[0],tileP->childNodes[1],tileP->childNodes[2],tileP->childNodes[3]) ;
/*
**              Get Points From Child Nodes And Populate Unfiltered DTM Object
*/
                unFilteredPtsP->numPoints = 0 ;
                for( n = 0 ; n < 4 ; ++n )
                  {
                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Child Node = %8ld ** numPoints = %8ld",(quadTreeP+tileP->childNodes[n])->tileNumber,(quadTreeP+tileP->childNodes[n])->dtmP->numPoints) ;
                   if( bcdtmObject_appendDtmObject(unFilteredPtsP,(quadTreeP+tileP->childNodes[n])->dtmP)) goto errexit ;
                  }
/*
**              Tile Decimate Points
*/
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Of Parent Points = %8ld",unFilteredPtsP->numPoints) ;
                numPointsRemove = unFilteredPtsP->numPoints -  unFilteredPtsP->numPoints / decimationFactor ;
                if( unFilteredPtsP->numPoints > 5 ) 
                  { 
                   internalTile = TRUE ;     // Still To Do To Work Out If Tile Is Internal
                   if( internalTile == TRUE ) 
                     {
                      if( bcdtmFilter_tileDecimateRandomSpotsDtmObject(unFilteredPtsP,numPointsRemove,&numFilteredPoints,filteredPtsP,&filterToleranceUsed)) goto errexit ;
                     }
                   else   // Maintain External Extent Of Data Set
                     {
                      if( bcdtmFilter_tinDecimateRandomSpotsDtmObject(unFilteredPtsP,2,1,numPointsRemove,&numFilteredPoints,filteredPtsP,&filterToleranceUsed)) goto errexit ;
                     } 
                  }
                if( dbg == 2 )
                  {
                   bcdtmWrite_message(0,0,0,"Number Of Unfilterd Points = %8ld",unFilteredPtsP->numPoints) ;
                   bcdtmWrite_message(0,0,0,"Number Of Filterd   Points = %8ld",filteredPtsP->numPoints) ;
                  }
/*
**              Optionally Filter Coplanar Points
*/
                if( coplanarFilterOption )
                  {
                   if( internalTile == FALSE )
                     {
                      if( bcdtmFilter_tileZToleranceFilterRandomSpotsDtmObject(unFilteredPtsP,coplanarFilterTolerance,&numFilteredPoints,filteredPtsP)) goto errexit ;
                     }
                   else
                     {
                      if( bcdtmFilter_tinZToleranceFilterRandomSpotsDtmObject(unFilteredPtsP,2,1,coplanarFilterTolerance,&numFilteredPoints,filteredPtsP)) goto errexit ;
                     }
                  }
/*
**              Release Unused memory
*/
                if( bcdtmObject_resizeMemoryDtmObject(unFilteredPtsP)) goto errexit ; 
/*
**              Copy Filtered Points To Tile
*/
                if( unFilteredPtsP->numPoints > 0 )
                  { 
                   if( bcdtmObject_cloneDtmObject(unFilteredPtsP,&tileP->dtmP)) goto errexit ;
                   tileP->numTilePts = unFilteredPtsP->numPoints ; 
                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"tileP->numTilePts = %8ld",tileP->numTilePts) ;
                  }
               }
            } 
         }
/*
**     Decouple The Filtering From The Tiles
*/
       else 
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Decoupling The Filtering From The Tiles") ;
/*
**        Allocate Memory For Child Points
*/
          memChildPoints = 100000 ;
          if( childPointsP == NULL ) childPointsP = (DPoint3d *) malloc( memChildPoints * sizeof(DPoint3d)) ;
          if( childPointsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }  
/*
**        Get Points From All Child Nodes For This Index Level
*/
          for( tileP = quadTreeP ; tileP < quadTreeP + numQuadTreeTiles ; ++tileP )
            {
             if( tileP->quadTreeLevel == level ) 
               {
                for( n = 0 ; n < 4 ; ++n )
                  {
/*
**                 Check Child Points Memory
*/ 
                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Parent Node = %8ld ** Child Nodes = %8ld %8ld %8ld %8ld",tileP->tileNumber,tileP->childNodes[0],tileP->childNodes[1],tileP->childNodes[2],tileP->childNodes[3]) ;
                   if( (quadTreeP+tileP->childNodes[n])->dtmP != NULL )
                     {   
                      if( (quadTreeP+tileP->childNodes[n])->dtmP->numPoints > memChildPoints )
                        {
                         memChildPoints = (quadTreeP+tileP->childNodes[n])->dtmP->numPoints ;
                         childPointsP = (DPoint3d *) realloc( childPointsP, memChildPoints * sizeof(DPoint3d)) ;
                         if( childPointsP == NULL )
                           {
                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                            goto errexit ;
                           }  
                        }
/*
**                    Copy All The Points For Each Tile As A Point Feature With The A Usertag Set To The Parent TileNumber
*/ 
                      numChildPoints = 0 ;
                      for( p3dP = childPointsP , point = 0 ; point < (quadTreeP+tileP->childNodes[n])->dtmP->numPoints ; ++p3dP , ++point )
                        {
                         pointP = pointAddrP((quadTreeP+tileP->childNodes[n])->dtmP,point) ;
                         p3dP->x = pointP->x ; 
                         p3dP->y = pointP->y ; 
                         p3dP->z = pointP->z ; 
                         ++numChildPoints ;
                        } 
                      if( bcdtmObject_storeDtmFeatureInDtmObject(unFilteredPtsP,DTMFeatureType::GroupSpots,tileP->tileNumber,1,&nullFeatureId,childPointsP,numChildPoints)) goto errexit ;
                     }
                  }
               }
            } 
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Number Of Points To Be Filtered = %8ld",unFilteredPtsP->numPoints) ;
/*
**        Tin Decimate Points Maintaining Data Extent Shape
*/
          numPointsRemove = unFilteredPtsP->numPoints - unFilteredPtsP->numPoints / decimationFactor ;
          if( unFilteredPtsP->numPoints > 5 ) if( bcdtmFilter_tinDecimateGroupSpotsDtmObject(unFilteredPtsP,1,1,numPointsRemove,&numFilteredPoints,filteredPtsP,&filterToleranceUsed)) goto errexit ;
          if( dbg == 1 )
            {
             bcdtmWrite_message(0,0,0,"Number Of Unfilterd Points = %8ld",unFilteredPtsP->numPoints) ;
             bcdtmWrite_message(0,0,0,"Number Of Filterd   Points = %8ld",filteredPtsP->numPoints) ;
            }
/*
**        Optionally Coplanar Filter
*/
          if( coplanarFilterOption && unFilteredPtsP->numPoints > 5 )
            {
             if( bcdtmFilter_tinZToleranceFilterGroupSpotsDtmObject(unFilteredPtsP,2,1,coplanarFilterTolerance,&numFilteredPoints,filteredPtsP)) goto errexit ;
            }
/*
**        Copy The Unfiltered Points To Their Parent Tiles
*/
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Copying Unfiltered Points To Parent Tile") ;
          lastParentTile = - 1 ;
          for( dtmFeature = 0 ; dtmFeature < unFilteredPtsP->numFeatures ; ++dtmFeature )
            {
             dtmFeatureP = ftableAddrP(unFilteredPtsP,dtmFeature) ;
             firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
             lastPoint  = firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ;
             parentTile = (long) dtmFeatureP->dtmUserTag ;
             if( lastParentTile != parentTile )
               {
                if( lastParentTile != -1 )
                  {
                   if((quadTreeP+lastParentTile)->dtmP != NULL ) bcdtmObject_resizeMemoryDtmObject((quadTreeP+lastParentTile)->dtmP) ;
                  } 
                lastParentTile = parentTile ;
               }
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Copying DTM Feature %2ld ** parentTile = %8ld numPoints = %8ld",dtmFeature,parentTile,dtmFeatureP->numDtmFeaturePts) ;
             if( (quadTreeP+parentTile)->dtmP == NULL )
               {
                if( bcdtmObject_createDtmObject(&(quadTreeP+parentTile)->dtmP)) goto errexit ;
                (quadTreeP+parentTile)->numTilePts = 0 ;
               }
             (quadTreeP+parentTile)->numTilePts = (quadTreeP+parentTile)->numTilePts + dtmFeatureP->numDtmFeaturePts ;
             for( point = firstPoint ; point <= lastPoint ; ++point )
               {
                pointP = pointAddrP(unFilteredPtsP,point) ;
                if( bcdtmObject_storeDtmFeatureInDtmObject((quadTreeP+parentTile)->dtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *) pointP,1)) goto errexit ;
               }
            }
/*
**        Resize  Parent Tiles
*/
          for( tileP = quadTreeP ; tileP < quadTreeP + numQuadTreeTiles ; ++tileP )
            {
             if( tileP->quadTreeLevel == level && tileP->dtmP != NULL ) 
               {
                 if( bcdtmObject_resizeMemoryDtmObject(tileP->dtmP)) goto errexit ; 
               }  
            } 
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"ParentTile = %8ld Number Of Points = %8ld",parentTile,(quadTreeP+parentTile)->numTilePts ) ;
         } 
      }
   }
/*
** Write DTMs For Filtered Quadtree Levels
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points For Index Level 0 = %8ld",quadTreeP->numTilePts) ;
    for( level = 0 ; level < numQuadTreeLevels  ; ++level )
      {
       if( filteredPtsP != NULL ) bcdtmObject_destroyDtmObject(&filteredPtsP) ;
       if( bcdtmObject_createDtmObject(&filteredPtsP)) goto errexit ;
       for( tileP = quadTreeP ; tileP < quadTreeP + numQuadTreeTiles ; ++tileP )
         {
          if( tileP->quadTreeLevel == level ) 
            {
             if( tileP->dtmP != NULL ) if( bcdtmObject_appendDtmObject(filteredPtsP,tileP->dtmP)) goto errexit ;
            }
         }
       if( bcdtmObject_triangulateDtmObject(filteredPtsP) ) goto errexit ;
       swprintf(fileName,32,L"quadTreeLevel%d.dtm",level) ;
       bcdtmWrite_toFileDtmObject(filteredPtsP,fileName) ;
      } 
    if( dbg ) bcdtmWrite_toFileDtmObject(quadTreeP->dtmP,L"quadtreeFilter.dtm") ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( childPointsP   != NULL ) free(childPointsP) ;   
 if( unFilteredPtsP != NULL ) bcdtmObject_destroyDtmObject(&unFilteredPtsP) ;
 if( filteredPtsP   != NULL ) bcdtmObject_destroyDtmObject(&filteredPtsP) ;
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


