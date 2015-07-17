/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmReport.cpp $
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
BENTLEYDTM_EXPORT int bcdtmReport_duplicatePointErrorsDtmObject
(
 BC_DTM_OBJ *dtmP,            /* ==> Pointer To DTM Object              */
 DTMDuplicatePointsCallback browseFunctionP,    /* ==> Pointer To Browse Function         */
 void* userP
)
/*
** This Function Reports Duplicate Point Errors In An Untriangulated DTM Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  pnt,pnt1,pnt2,numPts,*ofsP,*featureOfsP=NULL ;
 long  dtmFeature,dupPointError,numDupPoints ; 
 char  dtmFeatureTypeName[50] ;
 unsigned char  *pointMarkP=NULL,*processMarkP=NULL ;
 DTM_TIN_POINT *pnt1P,*pnt2P ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_DUPLICATE_POINT_ERROR *dupPtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Reporting Duplicate Point Errors") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"browseFunctionP = %p",browseFunctionP) ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld",dtmP->numPoints) ;
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
** Validate Arguements
*/
 if( browseFunctionP == NULL )
   {
    bcdtmWrite_message(2,0,0,"Null Browse Function") ;
    goto errexit ;
   } 
/*
** Allocate Memory For Marking Duplicate Points
*/
 numPts = dtmP->numPoints / 8 + 1  ;
 pointMarkP = ( unsigned char * ) calloc( numPts,sizeof(char)) ;
 if( pointMarkP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Allocate Memory For Marking Processed Duplicate Points
*/
 processMarkP = ( unsigned char * ) calloc( numPts , sizeof(char)) ;
 if( processMarkP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Sort DTM Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting DTM Object") ;
 if( bcdtmObject_changeStateDtmFeaturesToOffsetsArrayDtmObject(dtmP)) goto errexit ;
 if( bcdtmObject_sortDtmObject(dtmP) ) goto errexit ;
/*
** Check DTM Points Are Sorted
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking DTM Sort Order") ;
    pnt1P = pointAddrP(dtmP,0) ;
    for( pnt = 1 ; pnt < dtmP->numPoints ; ++pnt )
      {
       pnt2P = pointAddrP(dtmP,pnt) ;
       if( pnt1P->x > pnt2P->x || ( pnt1P->x == pnt2P->x && pnt1P->y > pnt2P->y ))
         {
          bcdtmWrite_message(0,0,0,"Unsorted DTM Points") ;
          bcdtmWrite_message(0,0,0,"pnt1 = %8ld ** x = %12.5lf y = %12.5lf z = %10.4lf",pnt-1,pnt1P->x,pnt1P->y,pnt1P->z) ;
          bcdtmWrite_message(0,0,0,"pnt2 = %8ld ** x = %12.5lf y = %12.5lf z = %10.4lf",pnt,pnt2P->x,pnt2P->y,pnt2P->z) ;
          goto errexit ;
         }
       pnt1P = pnt2P ;
      }
   }
/*
** Create Pointer To Feature Entries
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating And Setting Pointers To Feature Points") ;
 featureOfsP = ( long * ) malloc ( dtmP->numPoints * sizeof(long)) ;
 if( featureOfsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   } 
 for( ofsP = featureOfsP ; ofsP < featureOfsP + dtmP->numPoints ; ++ofsP ) *ofsP = -1 ;
/*
** Set Feature Offsets
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
      {
      long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       for( ofsP = offsetP ; ofsP < offsetP + dtmFeatureP->numDtmFeaturePts ; ++ofsP )
       *(featureOfsP+*ofsP) = dtmFeature ;
      }
   }
/*
** Scan For Duplicate Point Errors
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Duplicate Points") ;
 for( pnt1 = 0 ; pnt1 < dtmP->numPoints ; ++pnt1 )
   {
/*
**  Check Point Has Not Been Prior Processed
*/
    if( ! bcdtmFlag_testFlag(processMarkP,pnt1) )
      { 
       pnt2 = pnt1 ;
       pnt2P = pnt1P = pointAddrP(dtmP,pnt1) ;
       while ( pnt2 <  dtmP->numPoints && pnt2P->x - pnt1P->x < dtmP->ppTol )
         {
          ++pnt2 ;
          pnt2P = pointAddrP(dtmP,pnt2) ;
         }
       --pnt2 ;
/*
**     Scan For Duplicate Points For Errors ( Different Elevations )
*/
       if( pnt2 > pnt1 )
         {
          dupPointError = FALSE ; 
          numDupPoints = 0 ; 
          for( pnt = pnt1 + 1 ; pnt <= pnt2 ; ++pnt )
            {
             if( ! bcdtmFlag_testFlag(processMarkP,pnt))
               {
                if( bcdtmMath_pointDistanceDtmObject(dtmP,pnt1,pnt) < dtmP->ppTol && pointAddrP(dtmP,pnt)->z != pnt1P->z ) 
                  {
                   ++numDupPoints ;
                   dupPointError = TRUE ;
                   bcdtmFlag_setFlag(pointMarkP,pnt) ;
                  }
               }
            }
/*
**        If Duplicate Point Errors Then Return Them To The Caller
*/
          if( dupPointError == TRUE ) 
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Duplicate Point Errors Detected") ;
             bcdtmFlag_setFlag(pointMarkP,pnt1) ;
             ++numDupPoints ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicate Points = %8ld",numDupPoints) ;
/*
**           Allocate Memory For Duplicate Point Errors
*/
             dupPtsP = ( DTM_DUPLICATE_POINT_ERROR * ) malloc ( numDupPoints * sizeof(DTM_DUPLICATE_POINT_ERROR)) ;
             if( dupPtsP == NULL )
               {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               } 
/*
**           Store Duplicate Point Errors
*/
             numPts = 0 ;
             for( pnt = pnt1 ; pnt <= pnt2 ; ++pnt )
               {
                if( bcdtmFlag_testFlag(pointMarkP,pnt) )
                  {
/*
**                 Check Duplicate Point
*/
                   if( cdbg )
                     {
                      if( bcdtmMath_pointDistanceDtmObject(dtmP,pnt1,pnt) >= dtmP->ppTol )
                        {
                         bcdtmWrite_message(0,0,0,"Not A Duplicate Point ** dtmP->ppTol = %12.8lf point distance = %12.8lf",dtmP->ppTol,bcdtmMath_pointDistanceDtmObject(dtmP,pnt1,pnt)) ;
                         goto errexit ; 
                        } 
                     } 
/*
**                 Set Duplicate Point Table Values
*/
                   (dupPtsP+numPts)->x = pointAddrP(dtmP,pnt)->x ;
                   (dupPtsP+numPts)->y = pointAddrP(dtmP,pnt)->y ;
                   (dupPtsP+numPts)->z = pointAddrP(dtmP,pnt)->z ;
                   if( *(featureOfsP+pnt) < 0 )
                     {
                      (dupPtsP+numPts)->dtmFeatureType = DTMFeatureType::RandomSpots ;
                      (dupPtsP+numPts)->dtmFeatureId   = dtmP->nullFeatureId ;
                      (dupPtsP+numPts)->dtmUserTag     = dtmP->nullUserTag ;
                     }
                   else
                     { 
                      dtmFeatureP = ftableAddrP(dtmP,*(featureOfsP+pnt)) ;
                      (dupPtsP+numPts)->dtmFeatureType = (DTMFeatureType)dtmFeatureP->dtmFeatureType ;
                      (dupPtsP+numPts)->dtmFeatureId   = dtmFeatureP->dtmFeatureId ;
                      (dupPtsP+numPts)->dtmUserTag     = dtmFeatureP->dtmUserTag ;
                     } 
                   if( dbg == 2 )
                     {
                      bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType((dupPtsP+numPts)->dtmFeatureType,dtmFeatureTypeName) ;
                      bcdtmWrite_message(0,0,0,"Error[%4ld] **  z = %12.5lf ** %30s ** id = %12I64d ** userTag = %12I64d",
                      pnt-pnt1+1,(dupPtsP+numPts)->z,dtmFeatureTypeName,(dupPtsP+numPts)->dtmFeatureId,(dupPtsP+numPts)->dtmUserTag) ;
                     }
/*
**                 Mark Duplicate Point As Processed
*/
                   bcdtmFlag_setFlag(processMarkP,pnt) ;
                   bcdtmFlag_clearFlag(pointMarkP,pnt) ;
                   ++numPts ;
                  }
               }  
/*
**           Call User Browser
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Calling Browse Function ** numPts = %8ld",numPts) ;
             if( browseFunctionP(pnt1P->x,pnt1P->y,dupPtsP,numPts,userP))
               {
                bcdtmWrite_message(1,0,0,"Terminated By Browser Function") ;
                goto errexit ;
               }
             free( dupPtsP ) ;
             dupPtsP = NULL ;
            }
         }
      }
   }
/*
** Change DTM Sate Back To Points Unsorted
*/
 if( dtmP->dtmState == DTMState::PointsSorted )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Changing DTM State To Points Unsorted") ;
    if( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
   }
/*
** Cleanup
*/
 cleanup :
 if( dupPtsP      != NULL ) free(dupPtsP) ; 
 if( featureOfsP  != NULL ) free(featureOfsP) ;
 if( pointMarkP   != NULL ) free(pointMarkP) ;
 if( processMarkP != NULL ) free(processMarkP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reporting Duplicate Point Errors Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reporting Duplicate Point Errors Error") ;
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
BENTLEYDTM_EXPORT int bcdtmReport_crossingFeaturesDtmObject
(
 BC_DTM_OBJ *dtmP,           /* ==> Pointer To DTM Object                          */
 DTMFeatureType  *featureListP,        /* ==> Features To Be Included For Crossing detection */
 long  numFeatureList,       /* ==> Number Of Features In List                     */
 DTMCrossingFeaturesCallback browseFunctionP,   /* ==> Pointer To Browse Function                     */
 void *userP                 /* ==> User Call Back Pointer                         */
)
/*
** This Function Reports Crossing Features In An Untriangulated DTM Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  n,n1,dtmFeature,totalFeatureCnts=0 ;
 long  *featureCntsP=NULL ;
 char  dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Reporting Crossing Feature Errors") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"featureListP    = %p",featureListP) ;
    bcdtmWrite_message(0,0,0,"numFeatureList  = %8ld",numFeatureList) ;
    bcdtmWrite_message(0,0,0,"browseFunctionP = %p",browseFunctionP) ;
    if( dbg == 2 )
      {
       for( n = 0 ; n < numFeatureList ; ++n )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(*(featureListP+n),dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"Feature[%2ld] ** Type = %4ld Name = %30s",n+1,*(featureListP+n),dtmFeatureTypeName) ;
         }
      } 
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
    if( dbg ) bcdtmWrite_message(0,0,0,"Changing DTM State From Tin To Data") ;
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
** Validate Arguements
*/
 if( browseFunctionP == NULL )
   {
    bcdtmWrite_message(2,0,0,"Null Browse Function") ;
    goto errexit ;
   } 
 if( featureListP == NULL || numFeatureList <= 0 )
   {
    bcdtmWrite_message(2,0,0,"Null Feature List For Crossing Features") ;
    goto errexit ;
   }
/*
** Validate Dtm Feature Types In Feature List
*/
 for( n = 0 ; n < numFeatureList ; ++n )
   {
    if( bcdtmData_testForValidDtmObjectImportFeatureType(*(featureListP+n)) == DTM_ERROR )
      {
       bcdtmWrite_message(2,0,0,"Invalid Dtm Feature Type In Feature List For Crossing Features") ;
       goto errexit ;
      }
   }
/*
** Remove Random And Group Spots From Feature List
*/
 for( n = n1 = 0 ; n1 < numFeatureList ; ++n1 )
   {
    if( *(featureListP+n1) != DTMFeatureType::RandomSpots && *(featureListP+n1) != DTMFeatureType::GroupSpots )
      {
       if( n1 != n ) *(featureListP+n) = *(featureListP+n1) ;
       ++n ;
      } 
   }
 numFeatureList = n ;
/*
** Check For Features In Feature List
*/
 if( numFeatureList <= 0 )
   {
    bcdtmWrite_message(2,0,0,"No Valid Dtm Feature Type In Feature List For Crossing Features") ;
    goto errexit ;
   }
/*
** Allocate Memory For Feature Counts
*/
 featureCntsP = ( long *) malloc( numFeatureList * sizeof(long)) ;
 if( featureCntsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( n = 0 ; n < numFeatureList ; ++n ) *(featureCntsP+n) = 0 ;
/*
** Count Number Of DTM Feature Types In DTM Object
*/
 totalFeatureCnts = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if(dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted)
        {
        for( n = 0 ; n < numFeatureList ; ++n )
          {
           if( *(featureListP+n) == dtmFeatureP->dtmFeatureType )
             {
              ++*(featureCntsP+n) ;
              ++totalFeatureCnts ;
             }
          } 
        }
   } 
/*
** Write Out Feature Counts
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Feature Types = %2ld ** Number Of Feature Occurrences = %6ld",numFeatureList,totalFeatureCnts) ; 
    for( n = 0 ; n < numFeatureList ; ++n )
      {
       bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(*(featureListP+n),dtmFeatureTypeName) ;
       bcdtmWrite_message(0,0,0,"%30s ** Feature Occurrences = %6ld",dtmFeatureTypeName,*(featureCntsP+n)) ;
      }
   }
/*
** Intersect Crossing Features
*/
 if( totalFeatureCnts > 0 )
   {
    if( bcdtmReport_intersectCrossingFeaturesDtmObject(dtmP,featureListP,numFeatureList,browseFunctionP, userP) ) goto errexit ;
   }
/*
** Cleanup
*/
 cleanup :
 if( featureCntsP != NULL ) 
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reporting Crossing Feature Errors Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reporting Crossing Feature Errors Error") ;
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
BENTLEYDTM_Private int bcdtmReport_intersectCrossingFeaturesDtmObject
(
 BC_DTM_OBJ *dtmP,           /* ==> Pointer To DTM Object                          */
 DTMFeatureType  *featureListP,        /* ==> Features To Be Included For Crossing detection */
 long  numFeatureList,       /* ==> Number Of Features In List                     */
 DTMCrossingFeaturesCallback browseFunctionP,   /* ==> Pointer To Browse Function                     */
 void* userP
)
/*
** This Function Intersects Crossing Features In An Untriangulated DTM Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  n,intersectType=1,numIntTable=0,numIntPoints=0,memIntPoints=0,memIntPointsInc=10000 ;
 char  dtmFeatureTypeName[50] ;
 DTM_STRING_INTERSECT_TABLE  *intTableP=NULL ;
 DTM_INTERSECT_POINT  *intP,*intPointsP=NULL ;
 DTM_CROSSING_FEATURE_ERROR crossError ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 DTM_TIN_POINT   *pntP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Intersecting Crossing Features") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"featureListP    = %p",featureListP) ;
    bcdtmWrite_message(0,0,0,"numFeatureList  = %8ld",numFeatureList) ;
    bcdtmWrite_message(0,0,0,"browseFunctionP = %p",browseFunctionP) ;
    if( dbg == 2 )
      {
       for( n = 0 ; n < numFeatureList ; ++n )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(*(featureListP+n),dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"Feature[%2ld] ** Type = %4ld Name = %30s",n+1,*(featureListP+n),dtmFeatureTypeName) ;
         }
      } 
   }
/*
** Create Intersection Tables
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Feature Intersection Tables") ; 
 if( bcdtmReport_buildFeatureIntersectionTableDtmObject(dtmP,featureListP,numFeatureList,&intTableP,&numIntTable) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numIntTable = %6ld",numIntTable) ;
/*
** Scan For Line String Intersections
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Intersections") ; 
 if( bcdtmString_scanForStringLineIntersections2D(intersectType,intTableP,numIntTable,&intPointsP,&numIntPoints,&memIntPoints,memIntPointsInc)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numIntPoints = %6ld",numIntPoints) ; 
/*
** Reallocate Memory For Intersections Points
*/
 if( numIntPoints > 0 && numIntPoints != memIntPoints ) intPointsP = ( DTM_INTERSECT_POINT * ) realloc( intPointsP , numIntPoints * sizeof( DTM_INTERSECT_POINT )) ;
/*
** Pass Intersection Points To Browser
*/
 if( numIntPoints > 0 )
   {
    for( intP = intPointsP ; intP < intPointsP + numIntPoints ; intP+=2 )
      {
       crossError.intersectionX = intP->x ;
       crossError.intersectionY = intP->y ;
       dtmFeatureP = ftableAddrP(dtmP,intP->string1Offset) ;
       crossError.dtmFeatureId1 = dtmFeatureP->dtmFeatureId ;
       crossError.dtmFeatureType1 = (DTMFeatureType)dtmFeatureP->dtmFeatureType ;
       dtmFeatureP = ftableAddrP(dtmP,intP->string2Offset) ;
       crossError.dtmFeatureId2 = dtmFeatureP->dtmFeatureId ;
       crossError.dtmFeatureType2 = (DTMFeatureType)dtmFeatureP->dtmFeatureType ;
       crossError.segmentOfset1 = intP->segment1Offset ;
       crossError.segmentOfset2 = intP->segment2Offset ;
       crossError.elevation1    = intP->zSegment1 ;
       crossError.elevation2    = intP->zSegment2 ;
       crossError.distance1     = intP->distance  ;
       pntP = pointAddrP(dtmP,dtmFeatureP->dtmFeaturePts.firstPoint+intP->segment2Offset) ;
       crossError.distance2     = bcdtmMath_distance(pntP->x,pntP->y,intP->x,intP->y)  ;
/*
**     Call User Browser
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Calling Browse Function") ;
       if( browseFunctionP (crossError, userP))
         {
          bcdtmWrite_message(1,0,0,"Terminated By Browser Function") ;
          goto errexit ;
         }
      } 
   }
/*
** Write Out Intersection Points
*/
 if( dbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersection Points = %6ld",numIntPoints) ; 
    for( intP = intPointsP ; intP < intPointsP + numIntPoints ; ++intP )
      {
      }
   }
/*
** Sort Intersection Points On String Number
*/
 if( numIntPoints > 0  )
   {
    qsortCPP(intPointsP,numIntPoints,sizeof(DTM_INTERSECT_POINT),bcdtmString_intersectPointsCompareFunction) ;
   }
/*
** Cleanup
*/
 cleanup :
 if( intTableP  != NULL ) free(intTableP) ;
 if( intPointsP != NULL ) free(intPointsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Crossing Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Crossing Features Error") ;
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
BENTLEYDTM_Private int  bcdtmReport_buildFeatureIntersectionTableDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureType  *featureListP,
 long  numFeatureList,
 DTM_STRING_INTERSECT_TABLE **intTablePP,
 long *numIntTableP
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   point, dtmFeature;
 DTMFeatureType *featureP;
 long   memIntTable=0,memIntTableInc=10000  ;
 double cord ; 
 DTM_TIN_POINT *pnt1P ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_STRING_INTERSECT_TABLE *intP  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Feature Intersection Table") ;
/*
** Initialise
*/
 *numIntTableP = memIntTable = 0 ;
 if( *intTablePP != NULL ) { free(*intTablePP) ; *intTablePP = NULL ; }
/*
** Scan DTM Features 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For DTM Features") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if(dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
       {
/*
**      Scan Feature List For Features Equal To This Feature
*/
        for( featureP = featureListP ; featureP < featureListP + numFeatureList ; ++featureP )
          {
           if( *featureP == dtmFeatureP->dtmFeatureType )
             { 
              pnt1P = pointAddrP(dtmP,dtmFeatureP->dtmFeaturePts.firstPoint) ;
              for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ; ++point )
                {
/*
**               Check For Memory Allocation
*/
                 if( *numIntTableP == memIntTable )
                   {
                    memIntTable = memIntTable + memIntTableInc ;
                    if( *intTablePP == NULL ) *intTablePP = ( DTM_STRING_INTERSECT_TABLE * ) malloc ( memIntTable * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
                    else                      *intTablePP = ( DTM_STRING_INTERSECT_TABLE * ) realloc ( *intTablePP,memIntTable * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
                    if( *intTablePP == NULL ) goto errexit ; 
                   }
/*
**               Store Dtm Feature
*/
                 (*intTablePP+*numIntTableP)->string      = dtmFeature ; 
                 (*intTablePP+*numIntTableP)->segment     = point - dtmFeatureP->dtmFeaturePts.firstPoint ;
                 (*intTablePP+*numIntTableP)->numSegments = dtmFeatureP->numDtmFeaturePts - 1 ;
                 (*intTablePP+*numIntTableP)->isReversed  = 0 ;
                 (*intTablePP+*numIntTableP)->X1 = pnt1P->x ;
                 (*intTablePP+*numIntTableP)->Y1 = pnt1P->y ;
                 (*intTablePP+*numIntTableP)->Z1 = pnt1P->z ;
                 pnt1P = pointAddrP(dtmP,point+1) ;
                 (*intTablePP+*numIntTableP)->X2 = pnt1P->x ;
                 (*intTablePP+*numIntTableP)->Y2 = pnt1P->y ;
                 (*intTablePP+*numIntTableP)->Z2 = pnt1P->z ;
                 ++*numIntTableP ;
                } 
             }
          }
        }
   } 
/*
** Reallocate Intersection Table Memory
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reallocating Intersection Table ** numIntTable = %8ld",*numIntTableP) ;
 if( *numIntTableP != memIntTable ) *intTablePP = ( DTM_STRING_INTERSECT_TABLE * ) realloc ( *intTablePP, *numIntTableP * sizeof(DTM_STRING_INTERSECT_TABLE)) ;
/*
** Order Line Coordinates In Increasing x and y Coordinate Values
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Ordering Line Coordinates") ;
 for( intP = *intTablePP ; intP < *intTablePP + *numIntTableP ; ++intP )
   {
    if( intP->X1 > intP->X2 || ( intP->X1 == intP->X2 && intP->Y1 > intP->Y2 ) )
      {
       intP->isReversed = 1 ;
       cord = intP->X1 ; intP->X1 = intP->X2 ; intP->X2 = cord ;       
       cord = intP->Y1 ; intP->Y1 = intP->Y2 ; intP->Y2 = cord ;       
       cord = intP->Z1 ; intP->Z1 = intP->Z2 ; intP->Z2 = cord ;       
      }
   }
/*
** Sort Intersection Table
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Intersection Table") ;
 qsortCPP(*intTablePP,*numIntTableP,sizeof(DTM_STRING_INTERSECT_TABLE),bcdtmReport_intersectionTableCompareFunction) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Feature Intersection Table Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Feature Intersection Table Error") ;
 return(ret) ;
/*
** Error Exit 
*/
 errexit :
 *numIntTableP = 0 ;
 if( *intTablePP != NULL ) { free(*intTablePP) ; *intTablePP = NULL ; }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmReport_intersectionTableCompareFunction(const DTM_STRING_INTERSECT_TABLE *int1P,const DTM_STRING_INTERSECT_TABLE *int2P)
/*
** Compare Function For Qsort Of String Line Intersection Table Entries
*/
{
 if     (  int1P->X1  < int2P->X1 ) return(-1) ;
 else if(  int1P->X1  > int2P->X1 ) return( 1) ;
 else if(  int1P->Y1  < int2P->Y2 ) return(-1) ;
 else if(  int1P->Y1  > int2P->Y2 ) return( 1) ;
 return(0) ;
}
