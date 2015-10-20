/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmTrg.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TerrainModel\Core\bcDTMBaseDef.h>
#include <TerrainModel\Core\dtmevars.h>
#include <TerrainModel\Core\bcdtminlines.h>
#include <TerrainModel\Core\partitionarray.h>
#include <thread>
#include <Bentley\BeTimeUtilities.h>

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public void bcdtmObject_updateLastModifiedTime
(
 BC_DTM_OBJ *dtmP
)
{
  _time32(&dtmP->modifiedTime);
  UInt64 now = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();
  BeTimeUtilities::ConvertUnixMillisToFiletime (*(_FILETIME*)&dtmP->lastModifiedTime, now);
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_getApiCleanUpDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMCleanupFlags* type
)
{
 int ret=DTM_SUCCESS ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Set Roll Back
*/
 *type = dtmP->dtmCleanUp  ;
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
BENTLEYDTM_EXPORT int bcdtmObject_setApiCleanUpDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMCleanupFlags type
)
{
 int ret=DTM_SUCCESS ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

 if (type == dtmP->dtmCleanUp) goto cleanup ;

 if (type == DTMCleanupFlags::None && dtmP->dtmCleanUp != DTMCleanupFlags::None)
    bcdtmObject_clearCleanUpDtmObject(dtmP) ;
/*
** Clear Current Roll Back Settings
*/
 dtmP->dtmCleanUp = (DTMCleanupFlags)type ; /*ToDo Vancouver*/
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
BENTLEYDTM_EXPORT int bcdtmObject_testApiCleanUpDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMCleanupFlags cleanUpOption
)
{
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) return(0) ;
/*
** Set Roll Type
*/
 return (dtmP->dtmCleanUp & cleanUpOption) != DTMCleanupFlags::None ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_clearCleanUpDtmObject
(
 BC_DTM_OBJ *dtmP
)
{
 int ret=DTM_SUCCESS ;
 int dtmFeature;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Set Roll Back
*/
 dtmP->dtmCleanUp = DTMCleanupFlags::None;

/*
**  Remove Tin Error And Roll Back Features
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( bcdtmData_deleteAllTinErrorFeaturesDtmObject(dtmP)) goto errexit ;
    if( bcdtmData_deleteAllRollBackFeaturesDtmObject(dtmP)) goto errexit ;
    if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
   }
   for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
    {
    dtmFeatureP = ftableAddrP(dtmP, dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback )
      {
      bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
      dtmFeatureP->dtmFeaturePts.pointsPI = 0;
      dtmFeatureP->dtmFeatureState          = DTMFeatureState::Deleted ;
      dtmFeatureP->numDtmFeaturePts         = 0 ;
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

int bcdtmClean_appendRollbackDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P)
/*
** This Function Appends DTM2 (dtmP2) to DTM1 (dtm1P)
** It Does This By Copying The DTM Features From DTM2 To DTM1
** A Hull Feature Is Only Appended If One Does Not Exist In DTM1
*/
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long sPnt, nPnt, fPnt, lPnt, dtmFeature, numFeaturePts;
bool hullPresent = false;
 unsigned char *pointMarkP=nullptr ;
 DPoint3d* featurePtsP=nullptr ;
 DTM_TIN_POINT   *pointP ;
 BC_DTM_FEATURE  *dtmFeatureP = nullptr;
 DTMFeatureId  featureId ;
 const DTMUserTag nullUserTag = dtm1P->nullUserTag;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Appending DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtm1P   = %p",dtm1P) ;
    bcdtmWrite_message(0,0,0,"dtm2P   = %p",dtm2P) ;
    bcdtmWrite_message(0,0,0,"dtm1P->dtmState  = %8ld",dtm1P->dtmState) ;
    bcdtmWrite_message(0,0,0,"dtm1P->numPoints = %8ld",dtm1P->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtm2P->dtmState  = %8ld",dtm2P->dtmState) ;
    bcdtmWrite_message(0,0,0,"dtm2P->numPoints = %8ld",dtm2P->numPoints) ;
   }
/*
** Check For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(dtm2P)) goto errexit ;
/*
** Allocate Mark Array For Random Points
*/
 pointMarkP = ( unsigned char *) calloc((dtm2P->numPoints/8+1),sizeof(char)) ;
 if( pointMarkP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Scan DTM 2 Features And Store In DTM1
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Appending DTM2 Features To DTM1") ;
 for( dtmFeature = 0 ; dtmFeature < dtm2P->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtm2P,dtmFeature) ;
    if (dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError)
      {
/*
**     Check For Hull Feature
*/
      if (dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull || (dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull && hullPresent == FALSE))
         {
/*
**        Store DTM2 Feature In DTM1
*/
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtm2P,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
/*
**        Mark Feature Points
*/
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
            {
             fPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
             lPnt = fPnt + dtmFeatureP->numDtmFeaturePts - 1 ;
             for( sPnt = fPnt ; sPnt <= lPnt ; ++sPnt )
               {
                bcdtmFlag_setFlag(pointMarkP,sPnt) ;
               }
            }
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
             sPnt = fPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
             do
               {
                bcdtmFlag_setFlag(pointMarkP,sPnt) ;
                if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtm2P,dtmFeature,sPnt,&nPnt)) goto errexit ;
                sPnt = nPnt ;
               } while ( sPnt != fPnt && sPnt != dtm2P->nullPnt ) ;
            }
         }
      }
   }
/*
** Append Random Spots
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Appending Random Spots") ;
 numFeaturePts = 0 ;
 if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
 for( sPnt = 0 ; sPnt < dtm2P->numPoints ; ++sPnt )
   {
    if( ! bcdtmFlag_testFlag(pointMarkP,sPnt))
      {
/*
**     Check Memory
*/
       if( featurePtsP == nullptr )
         {
          featurePtsP = ( DPoint3d * ) malloc( 1000 * sizeof(DPoint3d)) ;
          if( featurePtsP == nullptr )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Store Point In Cache
*/
       pointP = pointAddrP(dtm2P,sPnt) ;
       (featurePtsP+numFeaturePts)->x = pointP->x ;
       (featurePtsP+numFeaturePts)->y = pointP->y ;
       (featurePtsP+numFeaturePts)->z = pointP->z ;
       ++numFeaturePts ;
/*
**     Store Random Spots In DTM1
*/
       if( numFeaturePts == 1000 )
         {
         DTMUserTag userTag = nullUserTag;
         if (dtmFeatureP != nullptr) userTag = dtmFeatureP->dtmUserTag;
         if (bcdtmObject_storeDtmFeatureInDtmObject (dtm1P, DTMFeatureType::RandomSpots, userTag, 1, &featureId, featurePtsP, numFeaturePts)) goto errexit;
          numFeaturePts = 0 ;
         }
      }
   }
/*
** Store Remaining Random Spots In DTM1
*/
 if( numFeaturePts > 0 )
   {
    DTMUserTag userTag = nullUserTag;
    if (dtmFeatureP != nullptr) userTag = dtmFeatureP->dtmUserTag;
    if (bcdtmObject_storeDtmFeatureInDtmObject (dtm1P, DTMFeatureType::RandomSpots, userTag, 3, &featureId, featurePtsP, numFeaturePts)) goto errexit;
    numFeaturePts = 0 ;
   }
/*
**  Update Modified Time
 */
 bcdtmObject_updateLastModifiedTime (dtm1P) ;
/*
** Clean Up
*/
 cleanup :
 if( pointMarkP  != nullptr ) { free(pointMarkP)  ; pointMarkP = nullptr  ; }
 if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Appending DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Appending DTM Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

static int (*bcdtmCleanUp_rollBackOverrideP) (BC_DTM_OBJ *dtmP, long rollBackOption) = nullptr;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCleanUp_overrideRollBackDtmObject (int (*overrideP) (BC_DTM_OBJ *dtmP, long rollBackOption))
    {
    bcdtmCleanUp_rollBackOverrideP = overrideP;
    return DTM_SUCCESS;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_rollBackDtmObject
(
 BC_DTM_OBJ *dtmP,                     //  ==> DTM To Be Rolled Back From A Tin State To A Data State
 long rollBackOption                   //  ==> Rool Back Option < 1 = Features Only , 2 = Features Plus Triangulation >
)
/*
**
** This Function Rolls A DTM Back From A Tin State To A Data State
** If The DTM Was Triangulated With The Roll Back Option Set Then It Rolls Back To
** The Initial Feature State Otherwise It Converts Directly From A Tin State To A Data State
**
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,node,point,firstPoint,nextPoint,dtmFeature,dtmFeature1,pnt1,pnt2,drapeFeatures=FALSE;
 long flPtr,numFeaturePts,hullFeatureFound,numPointsMarked=0,*pmarkP,*pointMarkP=nullptr ;
 char dtmFeatureTypeName[100] ;
 DPoint3d  *p3dP,randomPoint,*featurePtsP=nullptr ;
 BC_DTM_OBJ *tempDtmP=nullptr,*trianglesDtmP=nullptr ;
 BC_DTM_FEATURE *dtmFeatureP,*dtmFeature1P;
 DTMFeatureId dtmFeatureId=dtmP->nullFeatureId ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ; 
 DTM_TIN_POINT *pointP;
 long numCleanUpFeatures,numTinFeaturesRolledBack,numTinFeatureErrorsRolledBack ;
 long numNewFeaturesRolledBack,numRandomPointsRolledBack,totalCleanUpFeatures, markSize ;
 unsigned char *markFlagP=nullptr ;


 if (bcdtmCleanUp_rollBackOverrideP)
    return bcdtmCleanUp_rollBackOverrideP (dtmP, rollBackOption);
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Rolling Back DTM To Initial Feature State") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"rollBackOption  = %8ld",rollBackOption) ;
   }
/*
** Report Zero Length Tin Features
*/
 if( dbg == 2 )
   {
    long closeFlag ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
         {
          bcdtmList_countNumberOfDtmFeaturePointsDtmObject(dtmP,dtmFeature,&numFeaturePts,&closeFlag) ;
          if( numFeaturePts == 0 )
            {
             bcdtmWrite_message(0,0,0,"ERROR ** No Feature Points ** dtmFeature[%5ld] ** Type = %8ld Id = %10I64d ** NumFeaturePts = %8ld",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeatureId,numFeaturePts) ;
            }
         }
      }
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test For Tin State
*/
 if( dtmP->dtmState == DTMState::Tin)
   {
/*
**  Convert From Tin State To Data State
*/
    if( ! bcdtmObject_testApiCleanUpDtmObject(dtmP, DTMCleanupFlags::All) )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Rolling Back To Data State") ;
       if( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
      }
/*
**  Roll Back To Initial Feature State
*/
    else
     {
       if( dbg ) bcdtmWrite_message(0,0,0,"Rolling Back To Initial Feature State") ;
/*
**     Create Temporary DTM Object For Restructuring Points Array
*/
       if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
       tempDtmP->iniPoints = dtmP->numPoints ;
       tempDtmP->incPoints = dtmP->incPoints ;
/*
**     Mark Points Inserted Between Feature Points
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Inserted Feature Points") ;
       numPointsMarked = 0 ;
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Scanning Tin Feature %8ld Type = %4ld",dtmFeature,dtmFeatureP->dtmFeatureType) ;
             point = firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
             do
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Point = %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z) ;
                flPtr = nodeAddrP(dtmP,point)->fPtr ;
                while( flPtr != dtmP->nullPtr )
                  {
                  if( flistAddrP(dtmP,flPtr)->dtmFeature == dtmFeature )
                     {
                      if( dbg == 2 ) bcdtmWrite_message(0,0,0,"*** Point Type = %2ld",flistAddrP(dtmP,flPtr)->pntType) ;
                      if( flistAddrP(dtmP,flPtr)->pntType == 2 )  // None Feature Point
                        {
                         ++numPointsMarked ;
                         nodeAddrP(dtmP,point)->sPtr = 1 ;
                        }
                     }
                   flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
                  }
                if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,point,&nextPoint)) goto errexit ;
                point = nextPoint ;
               } while( point != dtmP->nullPnt && point != firstPoint ) ;
            }
         }
       if( dbg )bcdtmWrite_message(0,0,0,"Number Of Points Marked = %8ld",numPointsMarked) ;
/*
**     Release Clist Array As It Is Not Required
*/
       if( dtmP->cListPP != nullptr )
         {
          for( n = 0 ; n < dtmP->numClistPartitions ; ++n )
            {
             bcdtmMemory_freePartition(dtmP, DTMPartition::CList, n, dtmP->cListPP[n]) ;
            }
          free(dtmP->cListPP) ;
          dtmP->cListPP = nullptr ;
          dtmP->numClistPartitions = 0 ;
          dtmP->numClist           = 0 ;
          dtmP->memClist           = 0 ;
          dtmP->cListPtr           = 0 ;
          dtmP->cListDelPtr        = dtmP->nullPtr ;
         }
/*
**     Check For And Remove Deleted Feature Points If No Features Present
*/
       if( dtmP->numFeatures == 0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Removing Deleted Feature Points") ;
          for( point = node = 0 ; node < dtmP->numPoints ; ++node )
            {
             if( !bcdtmFlag_testInsertPoint(dtmP,node) )
               {
                *(pointAddrP(dtmP,point)) = *(pointAddrP(dtmP,node)) ;
                ++point ;
               }
            }
          dtmP->numPoints = point ;
         }
/*
**     No Features In DTM So Release Nodes Memory
*/
       if( dtmP->numFeatures == 0 )
         {
          if( dtmP->nodesPP != nullptr )
            {
             for( n = 0 ; n < dtmP->numNodePartitions ; ++n ) bcdtmMemory_freePartition(dtmP, DTMPartition::Node, n, dtmP->nodesPP[n]) ;
             free(dtmP->nodesPP) ;
             dtmP->nodesPP = nullptr ;
             dtmP->numNodePartitions = 0 ;
             dtmP->numNodes = 0 ;
             dtmP->memNodes = 0 ;
            }
         }
/*
**     Features In Dtm Object
*/
       hullFeatureFound = 0 ;
       if( dtmP->numFeatures > 0 )
         {
/*
**        Write Out Dtm Features
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Number Of DTM Feature = %6ld",dtmP->numFeatures) ;
          if( dbg == 2 )
            {
             for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
               {
                dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
                bcdtmWrite_message(0,0,0,"Dtm Feature[%6ld] ** Type = %20s ** userTag = %11I64d featureId = %11I64d ** state = %2ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmFeatureState) ;
               }
            }
/*
**        Mark All Tin Features That Are Also A Roll Back Feature
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Marking Rolled Back DTM Tin Features") ;
          if( bcdtmObject_markTinFeaturesThatAreRollBackFeaturesDtmObject(dtmP)) goto errexit ;
/*
**        Copy Feature Points To Temporary Dtm Object Points Array
*/
          numCleanUpFeatures = 0 ;
          numTinFeaturesRolledBack = 0 ;
          numTinFeatureErrorsRolledBack = 0 ;
          numNewFeaturesRolledBack = 0 ;
          for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Rolling Back Feature = %8ld of %8ld",dtmFeature,dtmP->numFeatures) ;
             dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
             if( dbg == 2 ) if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull )bcdtmWrite_message(0,0,0,"Type = DTMFeatureType::Hull ** Feature State = %2ld",dtmFeatureP->dtmFeatureState) ;
/*
**           Existing Tin Feature - One Incorporated In The Triangulation
*/
             if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Existing Tin Feature") ;
                if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull )  // For Roll Back Purpose Hulls Are Stored As Roll Back Features
                  {
                   dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
                  }
                else
                  {
                   ++numTinFeaturesRolledBack ;
                   if( bcdtmData_copyInitialDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                   if( numFeaturePts == 0 || ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void && numFeaturePts < 4) )   // Initial Stuff Up With Drape Voids Not Being Rolled Back
                     {
                      if( dbg == 1 )
                        {
                         bcdtmWrite_message(0,0,0,"ERROR ** numFeaturePts = 0 ** dtmFeature[%5ld] ** Type = %8ld Id = %10I64d",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeatureId) ;
                         long listPtr,nextPnt,firstPnt ;
                         firstPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
                         nextPnt = firstPnt ;
                         listPtr  = nodeAddrP(dtmP,nextPnt)->fPtr ;
                         while ( listPtr != dtmP->nullPtr )
                           {
                            while ( listPtr != dtmP->nullPtr && flistAddrP(dtmP,listPtr)->dtmFeature != dtmFeature ) listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
                            if( listPtr != dtmP->nullPtr )
                              {
                               pointP = pointAddrP(dtmP,nextPnt) ;
                               if( flistAddrP(dtmP,listPtr)->pntType != 2 )
                                 {
                                  bcdtmWrite_message(0,0,0,"Actual   Feature Point = %12.5lf %12.5lf %10.4lf",pointP->x,pointP->y,pointP->z) ;
                                 }
                               else
                                 {
                                  bcdtmWrite_message(0,0,0,"Inserted Feature Point = %12.5lf %12.5lf %10.4lf",pointP->x,pointP->y,pointP->z) ;
                                 }
                              }
                            nextPnt = flistAddrP(dtmP,listPtr)->nextPnt ;
                            if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) listPtr = dtmP->nullPtr ;
                            else                                                  listPtr  = nodeAddrP(dtmP,nextPnt)->fPtr ;
                           }
                        }
                      if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                     }
                   if( dbg == 2 )
                     {
                      if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
                      bcdtmWrite_message(0,0,0,"Tin Dtm Feature ** dtmFeature = %6ld ** Type = %20s ** userTag = %11I64ld featureId = %10I64ld ** numFeaturePts = %6ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,numFeaturePts) ;
                     }
                   dtmFeatureP->dtmFeatureState          = DTMFeatureState::Data ;
                   dtmFeatureP->dtmFeaturePts.firstPoint = tempDtmP->numPoints ;
                   dtmFeatureP->numDtmFeaturePts         = numFeaturePts ;
                   if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                   free(featurePtsP) ;
                   featurePtsP = nullptr ;
                  }
               }
/*
**           New Dtm Feature - One Added After Triangulation
*/
             if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray && dtmFeatureP->dtmFeatureType != DTMFeatureType::DrapeVoid && dtmFeatureP->dtmFeatureType != DTMFeatureType::DrapeHull )
               {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                if( dbg )
                  {
                   if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
                   bcdtmWrite_message(0,0,0,"New Dtm Feature ** dtmFeature = %6ld ** Type = %20s ** userTag = %11I64ld featureId = %10I64ld ** numFeaturePts = %6ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,dtmFeatureP->numDtmFeaturePts) ;
                  }
/*
**              Add New Feature
*/
                ++numNewFeaturesRolledBack ;
                firstPoint = tempDtmP->numPoints ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts)) goto errexit ;
                bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
                dtmFeatureP->dtmFeaturePts.pointsPI = 0;
                if( dtmFeatureP->dtmFeatureType == DTMFeatureType::RandomSpots )
                  {
                   dtmFeatureP->dtmFeatureState          = DTMFeatureState::Deleted ;
                   dtmFeatureP->numDtmFeaturePts         = 0 ;
                  }
                else
                  {
                   dtmFeatureP->dtmFeatureState          = DTMFeatureState::Data ;
                   dtmFeatureP->dtmFeaturePts.firstPoint = firstPoint ;
                  }
               }
/*
**           Roll Back Dtm Feature  -  Feature Added During Triangulation To Enable CleanUp
*/
             if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback )
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Roll Back Feature") ;
                ++numCleanUpFeatures ;
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                firstPoint = tempDtmP->numPoints ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts)) goto errexit ;
//              Mark Random Points In Tin That Are Part Of Error Feature
                for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
                  {
                   bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&point) ;
                   nodeAddrP(dtmP,point)->sPtr = 1 ;
                  }
                bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
                dtmFeatureP->dtmFeaturePts.pointsPI = 0;
                if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots && dtmFeatureP->dtmUserTag == -dtmP->nullUserTag )
                  {
                   dtmFeatureP->dtmFeatureState          = DTMFeatureState::Deleted ;
                   dtmFeatureP->numDtmFeaturePts         = 0 ;
                  }
                else
                  {
                   dtmFeatureP->dtmFeatureState          = DTMFeatureState::Data ;
                   dtmFeatureP->dtmFeaturePts.firstPoint = firstPoint ;
                  }
               }
/*
**           Tin Error Dtm Feature
*/
             if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError )
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Tin Error Feature") ;
                ++numTinFeatureErrorsRolledBack ;
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;

//              Mark Random Points In Tin That Are Part Of Error Feature

                for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
                  {
                   bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&point) ;
                   nodeAddrP(dtmP,point)->sPtr = 1 ;
                  }

//              Copy To Points To Temporary DTM

                firstPoint = tempDtmP->numPoints ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts)) goto errexit ;
                bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
                dtmFeatureP->dtmFeatureState          = DTMFeatureState::Data ;
                dtmFeatureP->dtmFeaturePts.firstPoint = firstPoint ;
               }
            }
/*
**        Copy Random Points ( None Feature Points )
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Copying Random Points To New Point Array") ;
          numRandomPointsRolledBack = 0 ;
          for( node = 0 ; node < dtmP->numPoints ; ++node )
            {
             if( ! bcdtmFlag_testDeletePointBitPCWD(&nodeAddrP(dtmP,node)->PCWD))
               {
                if( ! bcdtmFlag_testInsertPoint(dtmP,node) )
                  {
                   if( nodeAddrP(dtmP,node)->hPtr == dtmP->nullPnt || ( nodeAddrP(dtmP,node)->hPtr != dtmP->nullPnt ))
                     {
                      if( nodeAddrP(dtmP,node)->fPtr == dtmP->nullPtr )
                        {
                         if( nodeAddrP(dtmP,node)->sPtr != 1 )
                           {
                            ++numRandomPointsRolledBack ;
                            pointP = pointAddrP(dtmP,node) ;
                            randomPoint.x = pointP->x ;
                            randomPoint.y = pointP->y ;
                            randomPoint.z = pointP->z ;
                            if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Copying Random Point %12.5lf %12.5lf %10.4lf",randomPoint.x,randomPoint.y,randomPoint.z);
                            if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&randomPoint,1)) goto errexit ;
                           }
                        }
                     }
                  }
               }
            }
/*
**        Free points Array - Copy Temp Points To DTM
*/
          if( dtmP->pointsPP != nullptr )
            {
//             bcdtmObject_moveOrCopyPointsArray(dtmP,tempDtmP) ;
             dtmP->numPoints = 0 ;
             dtmP->dtmState  = DTMState::Data ;
             for( point = 0 ; point < tempDtmP->numPoints ; ++point )
               {
                if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(tempDtmP,point),1)) goto errexit ;
               }
             dtmP->numSortedPoints = 0 ;
            }
/*
**        Free Nodes Array
*/
          if( dtmP->nodesPP != nullptr )
            {
             for( n = 0 ; n < dtmP->numNodePartitions ; ++n )  bcdtmMemory_freePartition(dtmP, DTMPartition::Node, n, dtmP->nodesPP[n]) ;
             free(dtmP->nodesPP) ;
             dtmP->nodesPP = nullptr ;
             dtmP->numNodePartitions = 0 ;
             dtmP->numNodes = 0 ;
             dtmP->memNodes = 0 ;
            }
/*
**        Free Flist Array
*/
          if( dtmP->fListPP != nullptr )
            {
             for( n = 0 ; n < dtmP->numFlistPartitions ; ++n ) bcdtmMemory_freePartition(dtmP, DTMPartition::FList, n, dtmP->fListPP[n]) ;
             free(dtmP->fListPP) ;
             dtmP->fListPP            = nullptr ;
             dtmP->numFlistPartitions = 0 ;
             dtmP->numFlist           = 0 ;
             dtmP->memFlist           = 0 ;
             dtmP->fListDelPtr        = dtmP->nullPtr ;
            }
         }
/*
**    Write Out All DTM Features
*/
       if( dbg == 2 )
         {
          int featureCount = 0 ;
          int hullCount = 0 ;
          for( dtmFeature1 = 0 ; dtmFeature1 < dtmP->numFeatures ; ++dtmFeature1 )
            {
             dtmFeature1P = ftableAddrP(dtmP,dtmFeature1) ;
             if( dtmFeature1P->dtmFeatureState == DTMFeatureState::Data ) ++featureCount ;
             if( dtmFeature1P->dtmFeatureState == DTMFeatureState::Data && dtmFeature1P->dtmFeatureType == DTMFeatureType::Hull) ++hullCount ;
            }
          bcdtmWrite_message(0,0,0,"featureCount = %8ld",featureCount) ;
          bcdtmWrite_message(0,0,0,"hullCount    = %8ld",hullCount) ;
         }
/*
**     Remove All None DTMFeatureState::Data Features
*/
       for( dtmFeature = dtmFeature1 = 0 ; dtmFeature1 < dtmP->numFeatures ; ++dtmFeature1 )
         {
          dtmFeature1P = ftableAddrP(dtmP,dtmFeature1) ;
          if( dtmFeature1P->dtmFeatureState == DTMFeatureState::Data || ( dtmFeature1P->dtmFeatureState == DTMFeatureState::PointsArray && ( dtmFeature1P->dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeature1P->dtmFeatureType == DTMFeatureType::DrapeHull ) ) )
            {
             if( dtmFeature != dtmFeature1 )
               {
                dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                *dtmFeatureP = *dtmFeature1P ;
               }
             ++dtmFeature ;
            }
         }

       dtmP->numFeatures = dtmFeature  ;
/*
**     Reset DTM Header Variables
*/
       dtmP->dtmState           = DTMState::Data ;
       dtmP->hullPoint          = dtmP->nullPnt ;
       dtmP->nextHullPoint      = dtmP->nullPnt ;
       dtmP->numLines           = 0 ;
       dtmP->numTriangles       = 0 ;
       dtmP->numSortedPoints    = 0 ;
       bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
       if( bcdtmObject_resizeMemoryDtmObject(dtmP)) goto errexit ;
      }
/*
**  Report Roll Back Stats
*/
    if( dbg == 2  )
      {
       totalCleanUpFeatures = numCleanUpFeatures + numTinFeaturesRolledBack + numTinFeatureErrorsRolledBack + numNewFeaturesRolledBack  ;
       bcdtmWrite_message(0,0,0,"numDtmFeatures                = %8ld",dtmP->numFeatures) ;
       bcdtmWrite_message(0,0,0,"totalCleanUpFeatures         = %8ld",totalCleanUpFeatures) ;
       bcdtmWrite_message(0,0,0,"numCleanUpFeatures           = %8ld",numCleanUpFeatures) ;
       bcdtmWrite_message(0,0,0,"numTinFeaturesRolledBack      = %8ld",numTinFeaturesRolledBack) ;
       bcdtmWrite_message(0,0,0,"numTinFeatureErrorsRolledBack = %8ld",numTinFeatureErrorsRolledBack) ;
       bcdtmWrite_message(0,0,0,"numNewFeaturesRolledBack      = %8ld",numNewFeaturesRolledBack) ;
       bcdtmWrite_message(0,0,0,"numRandomPointsRolledBack     = %8ld",numRandomPointsRolledBack) ;
       if( dtmP->numFeatures != totalCleanUpFeatures )
         {
          bcdtmWrite_message(1,0,0,"DTM Roll Back Invalid") ;
          goto errexit ;
         }
      }
   }
 else if( (dtmP->dtmState == DTMState::PointsSorted || dtmP->dtmState == DTMState::DuplicatesRemoved || dtmP->dtmState == DTMState::TinError) )
     {



    if( dbg && dtmP->dtmState == DTMState::PointsSorted) bcdtmWrite_message(0,0,0,"Changing Dtm State From DTMState::PointsSorted to DTMState::Data" ) ;
    if( dbg && dtmP->dtmState == DTMState::DuplicatesRemoved) bcdtmWrite_message(0,0,0,"Changing Dtm State From DTMState::DuplicatesRemoved to DTMState::Data" ) ;

    // Probably need to scan to see if they are any DTMFeatureState::Tin FEATURES in the DTM.
/*
**  If There Are No features Just Reset State To DTMState::Data
*/
    if( dtmP->numFeatures == 0 ) dtmP->dtmState = DTMState::Data ;
/*
**  Else Place Feature Points In First Point Order
*/
    else
      {
/*
**     Allocate An Array For Marking Random Points
*/
       markSize = dtmP->numPoints / 8 + 1 ;
       markFlagP = (unsigned char *) malloc( markSize * sizeof(char));
       if( markFlagP == nullptr )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       for( n = 0 ; n < markSize ; ++n ) *(markFlagP+n) = (char) 0 ;

/*
**     Copy Feature Points To Temporary Dtm Object Points Array
*/
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;

          if(dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
              {
/*
**        Mark Feature Points Copied
*/
              if (dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray)
                  {
                  long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
                  for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
                    {
                     bcdtmFlag_setFlag(markFlagP,offsetP[n]) ;
                    }
                  }
              else if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
                {
                 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Scanning Tin Feature %8ld Type = %4ld",dtmFeature,dtmFeatureP->dtmFeatureType) ;
                 point = firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
                 do
                   {
                    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Point = %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z) ;
                    flPtr = nodeAddrP(dtmP,point)->fPtr ;
                    while( flPtr != dtmP->nullPtr )
                      {
                      if( flistAddrP(dtmP,flPtr)->dtmFeature == dtmFeature )
                         {
                          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"*** Point Type = %2ld",flistAddrP(dtmP,flPtr)->pntType) ;
//                          if( flistAddrP(dtmP,flPtr)->pntType == 2 )  // None Feature Point
                            bcdtmFlag_setFlag(markFlagP,point) ;
                         }
                       flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
                      }
                    if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,point,&nextPoint)) goto errexit ;
                    point = nextPoint ;
                   } while( point != dtmP->nullPnt && point != firstPoint ) ;
                  }
              }
         }
        /*
        **        Mark All Tin Features That Are Also A Roll Back Feature
        */
       if (dtmP->extended && dtmP->extended->rollBackInfoP)
            {
            if( dbg ) bcdtmWrite_message(0,0,0,"Marking Rolled Back DTM Tin Features") ;
            if( bcdtmObject_markTinFeaturesThatAreRollBackFeaturesPreMergeDtmObject(dtmP, dtmP->extended->rollBackInfoP->rollBackDtmP)) goto errexit ;
            }

/*
**     Create Temporary DTM Object For Restructuring Points Array
*/
       if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
       tempDtmP->iniPoints = dtmP->numPoints ;
       tempDtmP->incPoints = dtmP->incPoints ;
/*
**     Copy Feature Points To Temporary Dtm Object Points Array
*/
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;

          if(dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError)
              {
              if( dtmFeatureP->dtmFeatureState != DTMFeatureState::OffsetsArray && dtmFeatureP->dtmFeatureState != DTMFeatureState::Tin)
                   continue;

/*
**        Store Feature Points In Temporary DTM Object
*/
              if( bcdtmData_copyInitialDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
              if( dbg )
                {
                 if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
                 bcdtmWrite_message(0,0,0,"Dtm Feature ** dtmFeature = %6ld ** Type = %20s ** userTag = %11I64d featureId = %11I64d ** numFeaturePts = %6ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,numFeaturePts) ;
                }
              dtmFeatureP->dtmFeatureState          = DTMFeatureState::Data ;
              dtmFeatureP->dtmFeaturePts.firstPoint = tempDtmP->numPoints ;
              dtmFeatureP->numDtmFeaturePts         = numFeaturePts ;
              if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
              free(featurePtsP) ;
              featurePtsP = nullptr ;
              }
         }
/*
**     Copy Random Points
*/
       for( n = 0 ; n < dtmP->numPoints ; ++n )
         {
          if( ! bcdtmFlag_testFlag(markFlagP,n ))
            {
             pointP = pointAddrP(dtmP,n) ;
             randomPoint.x = pointP->x ;
             randomPoint.y = pointP->y ;
             randomPoint.z = pointP->z ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,&randomPoint,1)) goto errexit ;
            }
         }
    if (dtmP->dtmState == DTMState::TinError)
        {
        if( dtmP->cListPP != nullptr )
          {
           for( n = 0 ; n < dtmP->numClistPartitions ; ++n ) bcdtmMemory_freePartition(dtmP, DTMPartition::CList, n, dtmP->cListPP[n]) ;
           free(dtmP->cListPP) ;
           dtmP->cListPP = nullptr ;
           dtmP->numClistPartitions = 0 ;
           dtmP->numClist           = 0 ;
           dtmP->memClist           = 0 ;
           dtmP->cListPtr           = 0 ;
           dtmP->cListDelPtr        = dtmP->nullPtr ;
          }
/*
**     Free Nodes Array
*/
       if( dtmP->nodesPP != nullptr )
         {
          for( n = 0 ; n < dtmP->numNodePartitions ; ++n )  bcdtmMemory_freePartition(dtmP, DTMPartition::Node, n, dtmP->nodesPP[n]) ;
          free(dtmP->nodesPP) ;
          dtmP->nodesPP = nullptr ;
          dtmP->numNodePartitions = 0 ;
          dtmP->numNodes = 0 ;
          dtmP->memNodes = 0 ;
         }
/*
**     Free Flist Array
*/
       if( dtmP->fListPP != nullptr )
         {
          for( n = 0 ; n < dtmP->numFlistPartitions ; ++n ) bcdtmMemory_freePartition(dtmP, DTMPartition::FList, n, dtmP->fListPP[n]) ;
          free(dtmP->fListPP) ;
          dtmP->fListPP            = nullptr ;
          dtmP->numFlistPartitions = 0 ;
          dtmP->numFlist           = 0 ;
          dtmP->memFlist           = 0 ;
          dtmP->fListDelPtr        = dtmP->nullPtr ;
         }
/*
**  Reset DTM Header Variables
*/
        dtmP->hullPoint          = dtmP->nullPnt ;
        dtmP->nextHullPoint      = dtmP->nullPnt ;
        dtmP->numLines           = 0 ;
        dtmP->numTriangles       = 0 ;
        }

/*
**     Free DTM Points Array
*/
    if (dtmP->extended && dtmP->extended->rollBackInfoP)
        {
        // Need to sort out the random points which are stored as
        if( dbg ) bcdtmWrite_message(0,0,0,"Merge back the features") ;

       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**           Roll Back Dtm Feature  -  Feature Added During Triangulation To Enable CleanUp
*/
              if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots && dtmFeatureP->dtmUserTag == -dtmP->nullUserTag )
               {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Roll Back Feature") ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts)) goto errexit ;
                dtmFeatureP->dtmFeatureState          = DTMFeatureState::Deleted ;
                dtmFeatureP->numDtmFeaturePts         = 0 ;
               }
           }
        }
//     bcdtmObject_moveOrCopyPointsArray(dtmP, tempDtmP);
       dtmP->numPoints = 0 ;
       dtmP->dtmState  = DTMState::Data ;
       for( point = 0 ; point < tempDtmP->numPoints ; ++point )
         {
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(tempDtmP,point),1)) goto errexit ;
         }
      dtmP->numSortedPoints = 0 ;
/*
**     Reset DTM Header Values
*/
       dtmP->dtmState           = DTMState::Data ;
//       dtmP->pointsPP            = tempDtmP->pointsPP ;
//       dtmP->numPoints          = tempDtmP->numPoints ;
//       dtmP->memPoints          = tempDtmP->memPoints ;
//       dtmP->numPointPartitions = tempDtmP->numPointPartitions ;
//       dtmP->pointPartitionSize = tempDtmP->pointPartitionSize ;
       dtmP->numSortedPoints    = 0 ;
//       tempDtmP->pointsPP       = nullptr ;

      }

    if (dtmP->extended && dtmP->extended->rollBackInfoP)
        {
        // Need to sort out the random points which are stored as
        if( dbg ) bcdtmWrite_message(0,0,0,"Merge back the features") ;

        if (bcdtmClean_appendRollbackDtmObject(dtmP, dtmP->extended->rollBackInfoP->rollBackDtmP)) goto errexit ;

        }

     }

/*
** Check For Drape Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Drape Features") ;
 drapeFeatures = FALSE ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && drapeFeatures == FALSE ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( ( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid && dtmFeatureP->dtmFeatureState != DTMFeatureState::PointsArray )||
        ( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeHull && dtmFeatureP->dtmFeatureState != DTMFeatureState::PointsArray )   )
      {
       drapeFeatures = TRUE ;
      }
   }
/*
**  Change State Of Drape Features To Points Array
*/
 if( drapeFeatures )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Changing State Of Drape Features To Points Array") ;
    pointMarkP = ( long *) malloc(dtmP->numPoints*sizeof(long)) ;
    if( pointMarkP == nullptr )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( pmarkP = pointMarkP ; pmarkP < pointMarkP + dtmP->numPoints ; ++pmarkP ) *pmarkP = 0 ;

//  Mark Drape Features

    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Deleted)
           continue;
       if( ( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid && dtmFeatureP->dtmFeatureState != DTMFeatureState::PointsArray )||
           ( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeHull && dtmFeatureP->dtmFeatureState != DTMFeatureState::PointsArray )   )
         {
          firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmFeature[%5ld] ** dtmFeatureP->dtmFeatureType = %4ld ** firstPoint = %8ld",dtmFeature,dtmFeatureP->dtmFeatureType,firstPoint) ;
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          dtmFeatureP->dtmFeaturePts.pointsPI =  bcdtmMemory_allocate(dtmP, numFeaturePts * sizeof(DPoint3d)) ; 
          if( dtmFeatureP->dtmFeaturePts.pointsPI == 0 )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
          else
            {
             memcpy(bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),featurePtsP,numFeaturePts*sizeof(DPoint3d)) ; 
            }
          dtmFeatureP->dtmFeatureState = DTMFeatureState::PointsArray ;
          for( pmarkP = pointMarkP + firstPoint ; pmarkP < pointMarkP + firstPoint + dtmFeatureP->numDtmFeaturePts ; ++pmarkP ) *pmarkP = 1 ;
          if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
         }
      }

//   Remove Marked Points

     if( dbg ) bcdtmWrite_message(0,0,0,"Removing Marked Points") ;
     for( pnt1 = pnt2 = 0 ; pnt2 < dtmP->numPoints ; ++pnt2 )
       {
        if( ! *(pointMarkP+pnt2) )
          {
           if( pnt1 != pnt2 ) *pointAddrP(dtmP,pnt1) = *pointAddrP(dtmP,pnt2) ;
           ++pnt1 ;
          }
       }
     pnt2 = dtmP->numPoints ;
     dtmP->numPoints = pnt1 ;

//   Determine Offsets For None Drape Features

     for( pmarkP = pointMarkP + 1 ; pmarkP < pointMarkP + pnt2 ; ++pmarkP ) *pmarkP = *pmarkP + *(pmarkP-1) ;

//   Reset First Point For None Drape Features

     for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
       {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureType != DTMFeatureType::DrapeVoid && dtmFeatureP->dtmFeatureType != DTMFeatureType::DrapeHull )
          {
           firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
           dtmFeatureP->dtmFeaturePts.firstPoint = firstPoint - *(pointMarkP+firstPoint) ;
          }
       }
     if( pointMarkP != nullptr )  { free(pointMarkP)  ; pointMarkP  = nullptr ; }
    }
/*
** Append Roll Back Triangles
*/
 if( trianglesDtmP )
   {
    if( bcdtmObject_appendDtmObject(dtmP,trianglesDtmP)) goto errexit ;
   }
/*
** Set Bounding Cube
*/
 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->xRange = %12.4lf dtmP->yRange = %12.4lf dtmP->zRange = %12.4lf",dtmP->xRange,dtmP->yRange,dtmP->zRange) ;
/*
** Check For Multiple Hull Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dbg == 2 ) if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull ) bcdtmWrite_message(0,0,0,"**** Type = DTMFeatureType::Hull ** Feature State = %2ld",dtmFeatureP->dtmFeatureState) ;
   }
/*
** Log Features
*/
 if( dbg == 2 )
   {
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       bcdtmWrite_message(0,0,0,"dtmFeature[%5ld] ** State = %4ld Type = %4ld FirstPoint = %8ld NumPoints = %8ld",dtmFeature,dtmFeatureP->dtmFeatureState,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeaturePts.firstPoint,dtmFeatureP->numDtmFeaturePts) ;
      }
   }

/*
** Log Roll Back Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"DTM After Roll Back To Initial Feature State") ;
    bcdtmObject_reportStatisticsDtmObject(dtmP) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( markFlagP   != nullptr ) free(markFlagP) ;
 if( tempDtmP      != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 if( trianglesDtmP != nullptr ) bcdtmObject_destroyDtmObject(&trianglesDtmP) ;
 if( featurePtsP   != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
 if( pointMarkP    != nullptr ) { free(pointMarkP)  ; pointMarkP  = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Rolling Back DTM To Initial Feature State Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Rolling Back DTM To Initial Feature State Error") ;
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
BENTLEYDTM_EXPORT int bcdtmObject_setTriangulationParametersDtmObject
(
 BC_DTM_OBJ *dtmP,
 double ppTol,                // Point To Point Tolerance. Must Be Greater Than Or Equal To Zero
 double plTol,                // Point To Line Tolerance. Must Be Greater Than Or Equal To Zero
 long edgeOption,             // Three values 1 - Do Not Remove Any Tin Hull Triangles
                              //              2 - Remove Sliver Triangles On Tin Hull
                              //              3 - Remove Triangles On Tin Hull Whose Length > maxSide
 double maxSide               // MaxSide Value For Edge Option 3
)
/*
** Default Values For Triangulation Parameters When The DTM Object Is Created
**
**  ppTol = 0.0001
**  edgeOption = 2
**  maxSide = 1000.0
**
*/
{
 int ret=DTM_SUCCESS ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Validate Triangulation Parameters
*/
 if( ppTol < 0.0 ) ppTol = 0.0001 ;
 if( plTol < 0.0 ) plTol = 0.0001 ;
 if( edgeOption < 1 || edgeOption > 3 ) edgeOption = 2 ;
 if( edgeOption == 3 && maxSide <= 0.0 ) { maxSide = 1000.0 ; edgeOption = 2 ; }
/*
** Set Triangulation Parameters
*/
 if (dtmP->ppTol != ppTol || dtmP->plTol != plTol || dtmP->edgeOption != edgeOption || dtmP->maxSide != maxSide)
     {
    /*
    ** If DTM In Triangulated State Change It To Untriangulated State
    */
     if( dtmP->dtmState == DTMState::Tin )
       {
       if (bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
       }
    /*
    ** Check DTM Is In Untriangulated State
    */
      if( dtmP->dtmState != DTMState::Data )
        {
         bcdtmWrite_message(2,0,0,"Method Requires Untriangulated DTM") ;
         goto errexit ;
        }
         dtmP->ppTol = ppTol;
         dtmP->plTol = plTol;
         dtmP->edgeOption = edgeOption;
         dtmP->maxSide = maxSide;
         /*
         **  Update Modified Time
         */
         bcdtmObject_updateLastModifiedTime (dtmP);
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
BENTLEYDTM_EXPORT int bcdtmObject_getTriangulationParametersDtmObject
(
 BC_DTM_OBJ *dtmP,
 double* ppTol,                // Point To Point Tolerance. Must Be Greater Than Or Equal To Zero
 double* plTol,                // Point To Line Tolerance. Must Be Greater Than Or Equal To Zero
 long* edgeOption,             // Three values 1 - Do Not Remove Any Tin Hull Triangles
                               //              2 - Remove Sliver Triangles On Tin Hull
                               //              3 - Remove Triangles On Tin Hull Whose Length > maxSide
 double* maxSide               // MaxSide Value For Edge Option 3
)
{
 int ret=DTM_SUCCESS ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Set Triangulation Parameters
*/
 if (ppTol) *ppTol = dtmP->ppTol;
 if (plTol) *plTol = dtmP->plTol ;
 if (edgeOption) *edgeOption = dtmP->edgeOption;
 if (maxSide) *maxSide = dtmP->maxSide ;
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
BENTLEYDTM_Private int bcdtmObject_featureIdCompareFunction
(
 const DTMFeatureId *id1P ,
 const DTMFeatureId *id2P 
)
{
 if( *id1P <  *id2P ) return(-1) ;
 if( *id1P >  *id2P ) return( 1) ;
 return(0) ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmObject_markTinFeaturesThatAreRollBackFeaturesDtmObject
(
 BC_DTM_OBJ *dtmP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFeature,numFeatureIds=0,numCleanUpFeatures,numCleanUpTinFeatures=0 ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTMFeatureId *feat1P,*feat2P,*feat3P,*featureIdsP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Tin Features That Are Roll Back Features") ;
/*
** Only Process If Features Present
*/
 if( dtmP->numFeatures > 1 )
   {
/*
**  Count Number Of Roll Back Features
*/
    numCleanUpFeatures = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots ) ++numCleanUpFeatures ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"numCleanUpFeatures = %8ld",numCleanUpFeatures) ;
/*
**  Only Process If Roll Back Features Present
*/
    if( numCleanUpFeatures > 0 )
      {
/*
**     Allocate Memory For Feature Ids
*/
       numFeatureIds = numCleanUpFeatures ;
       featureIdsP = ( DTMFeatureId * ) malloc( numFeatureIds * sizeof(DTMFeatureId)) ;
       if( featureIdsP == nullptr )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Roll Back Features = %8ld",numCleanUpFeatures) ;
/*
**     Populate Feature Ids
*/
       numFeatureIds = 0 ;
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots )
            {
             *(featureIdsP+numFeatureIds) = dtmFeatureP->dtmFeatureId ;
             ++numFeatureIds ;
            }
         }
/*
**     Sort Feature Ids
*/
       if( numCleanUpFeatures > 1 ) qsort(featureIdsP,numFeatureIds,sizeof(DTMFeatureId),(int(*)(const void*, const void*))bcdtmObject_featureIdCompareFunction) ;
/*
**     Mark Tin Features That Have The Same Feature Id As A Roll Back Feature
*/
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( ( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin || dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError )&& dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots )
            {
             if( numCleanUpFeatures == 1 )
               {
                if( dtmFeatureP->dtmFeatureId == *featureIdsP )
                  {
                   dtmFeatureP->dtmFeatureState   = DTMFeatureState::Deleted ;
                   dtmFeatureP->numDtmFeaturePts  = 0 ;
                   ++numCleanUpTinFeatures ;
                  }
               }
             else
               {
                feat1P = featureIdsP ;
                feat2P = featureIdsP + numCleanUpFeatures-1 ;
                if( dtmFeatureP->dtmFeatureId == *feat1P || dtmFeatureP->dtmFeatureId == *feat2P )
                  {
                   dtmFeatureP->dtmFeatureState  = DTMFeatureState::Deleted ;
                   dtmFeatureP->numDtmFeaturePts = 0 ;
                   ++numCleanUpTinFeatures ;
                  }
                else
                  {
                   feat3P = featureIdsP +  ((long)(feat1P-featureIdsP) + (long)(feat2P-featureIdsP)) / 2 ;
                   while( feat3P != feat1P && feat3P != feat2P )
                     {
                      if( dtmFeatureP->dtmFeatureId == *feat3P )
                        {
                         dtmFeatureP->dtmFeatureState  = DTMFeatureState::Deleted ;
                         dtmFeatureP->numDtmFeaturePts = 0 ;
                         ++numCleanUpTinFeatures ;
                         feat1P = feat2P = feat3P ;
                        }
                      else
                        {
                         if      ( dtmFeatureP->dtmFeatureId < *feat3P ) feat2P = feat3P ;
                         else if ( dtmFeatureP->dtmFeatureId > *feat3P ) feat1P = feat3P ;
                         feat3P = featureIdsP +  ((long)(feat1P-featureIdsP) + (long)(feat2P-featureIdsP)) / 2 ;
                        }
                     }
                  }
               }
            }
         }
      }
   }
/*
** Log Number Of Tin Features That Are Roll Back Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Tin Roll Back Features = %8ld",numCleanUpTinFeatures ) ;
/*
** Clean Up
*/
 cleanup :
 if( featureIdsP != nullptr ) { free(featureIdsP) ; featureIdsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Tin Features That Are Roll Back Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Tin Features That Are Roll Back Features Error") ;
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
BENTLEYDTM_Private int bcdtmObject_markTinFeaturesThatAreRollBackFeaturesPreMergeDtmObject
(
 BC_DTM_OBJ *dtmP,
 BC_DTM_OBJ* rollbackDtmP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFeature,numFeatureIds=0,numCleanUpFeatures,numCleanUpTinFeatures=0 ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTMFeatureId *feat1P,*feat2P,*feat3P,*featureIdsP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Tin Features That Are Roll Back Features") ;
/*
** Only Process If Features Present
*/
 if( rollbackDtmP->numFeatures >= 1 )
   {
/*
**  Count Number Of Roll Back Features
*/
    numCleanUpFeatures = 0 ;
    for( dtmFeature = 0 ; dtmFeature < rollbackDtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(rollbackDtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots ) ++numCleanUpFeatures ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"numCleanUpFeatures = %8ld",numCleanUpFeatures) ;
/*
**  Only Process If Roll Back Features Present
*/
    if( numCleanUpFeatures > 0 )
      {
/*
**     Allocate Memory For Feature Ids
*/
       numFeatureIds = numCleanUpFeatures ;
       featureIdsP = ( DTMFeatureId * ) malloc( numFeatureIds * sizeof(DTMFeatureId)) ;
       if( featureIdsP == nullptr )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Roll Back Features = %8ld",numCleanUpFeatures) ;
/*
**     Populate Feature Ids
*/
       numFeatureIds = 0 ;
       for( dtmFeature = 0 ; dtmFeature < rollbackDtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(rollbackDtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots )
            {
             *(featureIdsP+numFeatureIds) = dtmFeatureP->dtmFeatureId ;
             ++numFeatureIds ;
            }
         }
/*
**     Sort Feature Ids
*/
       if( numCleanUpFeatures > 1 ) qsort(featureIdsP,numFeatureIds,sizeof(DTMFeatureId),(int(*)(const void*, const void*))bcdtmObject_featureIdCompareFunction) ;
/*
**     Mark Tin Features That Have The Same Feature Id As A Roll Back Feature
*/
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( ( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin || dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError )&& dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots )
            {
             if( numCleanUpFeatures == 1 )
               {
                if( dtmFeatureP->dtmFeatureId == *featureIdsP )
                  {
                   dtmFeatureP->dtmFeatureState   = DTMFeatureState::Deleted ;
                   dtmFeatureP->numDtmFeaturePts  = 0 ;
                   ++numCleanUpTinFeatures ;
                  }
               }
             else
               {
                feat1P = featureIdsP ;
                feat2P = featureIdsP + numCleanUpFeatures-1 ;
                if( dtmFeatureP->dtmFeatureId == *feat1P || dtmFeatureP->dtmFeatureId == *feat2P )
                  {
                   dtmFeatureP->dtmFeatureState  = DTMFeatureState::Deleted ;
                   dtmFeatureP->numDtmFeaturePts = 0 ;
                   ++numCleanUpTinFeatures ;
                  }
                else
                  {
                   feat3P = featureIdsP +  ((long)(feat1P-featureIdsP) + (long)(feat2P-featureIdsP)) / 2 ;
                   while( feat3P != feat1P && feat3P != feat2P )
                     {
                      if( dtmFeatureP->dtmFeatureId == *feat3P )
                        {
                         dtmFeatureP->dtmFeatureState  = DTMFeatureState::Deleted ;
                         dtmFeatureP->numDtmFeaturePts = 0 ;
                         ++numCleanUpTinFeatures ;
                         feat1P = feat2P = feat3P ;
                        }
                      else
                        {
                         if      ( dtmFeatureP->dtmFeatureId < *feat3P ) feat2P = feat3P ;
                         else if ( dtmFeatureP->dtmFeatureId > *feat3P ) feat1P = feat3P ;
                         feat3P = featureIdsP +  ((long)(feat1P-featureIdsP) + (long)(feat2P-featureIdsP)) / 2 ;
                        }
                     }
                  }
               }
            }
         }
      }
   }
/*
** Log Number Of Tin Features That Are Roll Back Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Tin Roll Back Features = %8ld",numCleanUpTinFeatures ) ;
/*
** Clean Up
*/
 cleanup :
 if( featureIdsP != nullptr ) { free(featureIdsP) ; featureIdsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Tin Features That Are Roll Back Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Tin Features That Are Roll Back Features Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}




static int (*bcdtmObject_overrideTriangulateP) (BC_DTM_OBJ *dtmP, bool normaliseOption , bool duplicateOption ) = nullptr;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_overrideTriangulateDtmObject (int (*overrideP) (BC_DTM_OBJ *dtmP, bool normaliseOption , bool duplicateOption ))
    {
    bcdtmObject_overrideTriangulateP = overrideP;
    return DTM_SUCCESS;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_triangulateDtmObject (BC_DTM_OBJ *dtmP, bool normaliseOption, bool duplicateOption)
/*
** This Function Triangulates A Dtm Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long numDeletePts,edgeOption,dtmFeature,numFeaturePts;
 double maxSide ;
 DPoint3d  *featurePtsP=nullptr ;
 DTM_ROLLBACK_DATA* rollBackInfo=nullptr ;
 BC_DTM_FEATURE *dtmFeatureP,*dtmCleanUpFeatureP ;
/*
** Write Entry Message
*/
 if (bcdtmObject_overrideTriangulateP)
     return bcdtmObject_overrideTriangulateP (dtmP, normaliseOption, duplicateOption);
// if( bcdtmObject_testCleanUpDtmObject(dtmP)) dbg=DTM_TRACE_VALUE(1) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Triangulating DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP->dtmCleanUp = %8ld",dtmP->dtmCleanUp) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints   = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
 if( dbg == 2 ) bcdtmWrite_toFileDtmObject(dtmP,L"createTin.bcdtm") ;
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
**  Log Roll Back Features
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Roll Back Features Pre Triangulation") ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback )
         {
          bcdtmWrite_message(0,0,0,"** CleanUp Feature[%5ld] ** Type = %2ld ID = %10I64d ** numPoints = %8ld",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeatureId,dtmFeatureP->numDtmFeaturePts) ;
          DPoint3d *p3dP,*pointsP = bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
          for( p3dP = pointsP ; p3dP < pointsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
            {
             bcdtmWrite_message(0,0,0,"**** Feature Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-pointsP),p3dP->x,p3dP->y,p3dP->z ) ;
            }
         }
      }
   }
/*
** Log DTM Object
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"DTM Stats Prior Triangulation") ;
    bcdtmObject_reportStatisticsDtmObject(dtmP) ;
   }
/*
** Set Triangulation Parameters
*/
 edgeOption = dtmP->edgeOption ;
 maxSide    = dtmP->maxSide ;
/*
** Create Temporary DTM For Roll Back Purposes
*/
 if( bcdtmObject_testApiCleanUpDtmObject(dtmP, DTMCleanupFlags::All) )
   {
/*
**  Create Roll Back DTM
*/
    if (!dtmP->extended && bcdtmObject_createDTMExtended (&dtmP->extended)) goto errexit;

    dtmP->extended->rollBackInfoP = rollBackInfo = new DTM_ROLLBACK_DATA ();
    if( bcdtmObject_createDtmObject(&rollBackInfo->rollBackDtmP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Created DTM %p For Roll Back Purposes",rollBackInfo->rollBackDtmP) ;
   }
/*
** Create Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Tin") ;
 if (bcdtmObject_createTinDtmObject (dtmP, edgeOption, maxSide, normaliseOption , duplicateOption)) goto errexit;
/*
** Append Roll Back Features To DTM
*/
 if( rollBackInfo && rollBackInfo->rollBackDtmP != nullptr )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Appending %8ld Roll Back Features To DTM",rollBackInfo->rollBackDtmP->numFeatures) ;
    for( dtmFeature = 0 ; dtmFeature < rollBackInfo->rollBackDtmP->numFeatures ; ++dtmFeature )
      {
       dtmCleanUpFeatureP = ftableAddrP(rollBackInfo->rollBackDtmP,dtmFeature) ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Roll Back Feature[%8ld] ** Type = %4ld Id = %10I64d UserTag = %10I64d",dtmFeature,dtmCleanUpFeatureP->dtmFeatureType,dtmCleanUpFeatureP->dtmFeatureId,dtmCleanUpFeatureP->dtmUserTag) ;
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(rollBackInfo->rollBackDtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
       if( dtmP->numFeatures == dtmP->memFeatures )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Allocating Memory For Features Array") ;
          if( bcdtmObject_allocateFeaturesMemoryDtmObject(dtmP)) goto errexit  ;
         }
       dtmFeatureP  = ftableAddrP(dtmP,dtmP->numFeatures)  ;
       dtmFeatureP->dtmFeatureState        =  DTMFeatureState::Rollback ;
       dtmFeatureP->dtmFeatureType         =  dtmCleanUpFeatureP->dtmFeatureType   ;
       dtmFeatureP->dtmUserTag             =  dtmCleanUpFeatureP->dtmUserTag          ;
       dtmFeatureP->dtmFeatureId           =  dtmCleanUpFeatureP->dtmFeatureId     ;
       dtmFeatureP->numDtmFeaturePts       =  dtmCleanUpFeatureP->numDtmFeaturePts  ;
       dtmFeatureP->dtmFeaturePts.pointsPI =  bcdtmMemory_allocate(dtmP,dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d)) ; 
       if( dtmFeatureP->dtmFeaturePts.pointsPI == 0 )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       else
         {
          memcpy(bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI),featurePtsP,numFeaturePts*sizeof(DPoint3d)) ; 
         }
       ++dtmP->numFeatures ;
       if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
      }
/*
**  Compact Feature Table
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Features Array") ;
    if( bcdtmObject_resizeMemoryDtmObject(dtmP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numFeatures = %8ld dtmP->memFeatures = %8ld",dtmP->numFeatures,dtmP->memFeatures) ;
/*
**  Log Roll Back Features
*/
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Roll Back Features Post Triangulation") ;
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback )
            {
             bcdtmWrite_message(0,0,0,"CleanUp Feature[%5ld] ** Type = %2ld ** numPoints = %8ld",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->numDtmFeaturePts) ;
             DPoint3d *p3dP,*pointsP = bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
             bcdtmWrite_message(0,0,0,"dtmFeatureP->dtmFeaturePts.pointsPI = %p",(DPoint3d *)dtmFeatureP->dtmFeaturePts.pointsPI) ;
             bcdtmWrite_message(0,0,0,"bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) = %p",bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI)) ;
             for( p3dP = pointsP ; p3dP < pointsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
               {
                bcdtmWrite_message(0,0,0,"Feature Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-pointsP),p3dP->x,p3dP->y,p3dP->z ) ;
               }
            }
         }
      }
   }


 // Log DTM Object Stats

 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"DTM Stats Post Triangulation") ;
    bcdtmObject_reportStatisticsDtmObject(dtmP) ;
   }

// Log Points Marked For Delete

 if( dbg == 1 )
   {
    bcdtmList_reportNumberOfPointsMarkedForDeleteDtmObject(dtmP,1,&numDeletePts) ;
   }

//  Log Created DTM To File

 if( dbg == 2 )
   {
    bcdtmWrite_toFileDtmObject(dtmP,L"createdTin.bcdtm") ;
   }

/*
** Clean Up
*/
 cleanup :
 if (dtmP->extended) dtmP->extended->rollBackInfoP = nullptr;
 if( rollBackInfo && rollBackInfo->rollBackDtmP  != nullptr  )  bcdtmObject_destroyDtmObject(&rollBackInfo->rollBackDtmP) ;
 if (rollBackInfo) { delete rollBackInfo; rollBackInfo = nullptr; }
 if( featurePtsP  != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Triangulating DTM Object Error") ;
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
BENTLEYDTM_Public int bcdtmObject_createTinDtmObject
(
 BC_DTM_OBJ *dtmP,
 long        edgeOption,
 double      maxSide,
 bool normaliseOption,
 bool duplicateOption
)
/*
** This Function Triangulates A Dtm Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 char dtmFeatureTypeName[50] ;
 long trgTime,createTime,dtmFeature,numTinFeatures,numErrorTinFeatures,numDeletedFeatures ;
 long numGraphicBreaks,numContourLines,numSoftBreaks,numHardBreaks,numVoids,numIslands,numHoles ;
 long numBreakVoids,numDrapeVoids,numGroupSpots,numRegions,numHulls,numDrapeHulls,numHullLines ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT  *pntP ;
/*
** Write Entry Message
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Creating Tin For Dtm Object %p ** dtmState = %2ld dtmP->numPoints = %8ld",dtmP,dtmP->dtmState,dtmP->numPoints) ;
   }
  createTime = bcdtmClock() ;
/*
** Procees DTM Object For Triangulation
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Processing DTM For Tin Creation") ;
 if( bcdtmObject_processForTriangulationDtmObject(dtmP, normaliseOption, duplicateOption)) goto errexit ;

/*
** Log DTM Object
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"DTM Stats After Pre Triangulation Processing") ;
    bcdtmObject_reportStatisticsDtmObject(dtmP) ;
    long numFeaturePts=0 ;
    DPoint3d *p3dP,*featurePtsP=nullptr ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       bcdtmWrite_message(0,0,0,"DtmFeatureType = %4ld **Number Of Feature Points = %8ld",dtmFeatureP->dtmFeatureType,dtmFeatureP->numDtmFeaturePts) ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
         {
          bcdtmWrite_message(0,0,0,"Number Of Feature Points = %8ld",dtmFeatureP->numDtmFeaturePts) ;
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          bcdtmWrite_message(0,0,0,"== Number Of Feature Points = %8ld",numFeaturePts) ;
          for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
            {
             bcdtmWrite_message(0,0,0,"Feature Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
            }
         }
      }
    if( featurePtsP != nullptr ) free(featurePtsP) ;
   }

/*
** Check If DTM Is Triangulated
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"DTM Is Triangulated") ;
    goto cleanup ;
   }
/*
** Count Number Of Dtm Triangulation Features
*/
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Counting Number Of Dtm Triangulation Features") ;
 bcdtmObject_countDtmTriangulationFeaturesDtmObject(dtmP,&numGraphicBreaks,&numContourLines,&numSoftBreaks,&numHardBreaks,&numVoids,&numIslands,&numHoles,&numBreakVoids,&numDrapeVoids,&numGroupSpots,&numRegions,&numHulls,&numDrapeHulls,&numHullLines) ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points           = %6ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Graphic Breaks   = %6ld",numGraphicBreaks) ;
    bcdtmWrite_message(0,0,0,"Number Of Contour Lines    = %6ld",numContourLines) ;
    bcdtmWrite_message(0,0,0,"Number Of Hard Breaks      = %6ld",numHardBreaks) ;
    bcdtmWrite_message(0,0,0,"Number Of Voids            = %6ld",numVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Island           = %6ld",numIslands) ;
    bcdtmWrite_message(0,0,0,"Number Of Holes            = %6ld",numHoles) ;
    bcdtmWrite_message(0,0,0,"Number Of Break Voids      = %6ld",numBreakVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Drape Voids      = %6ld",numDrapeVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Group Spots      = %6ld",numGroupSpots) ;
    bcdtmWrite_message(0,0,0,"Number Of Regions          = %6ld",numRegions) ;
    bcdtmWrite_message(0,0,0,"Number Of Hulls            = %6ld",numHulls) ;
    bcdtmWrite_message(0,0,0,"Number Of Drape Hulls      = %6ld",numDrapeHulls) ;
    bcdtmWrite_message(0,0,0,"Number Of Hull Lines       = %6ld",numHullLines) ;
   }
/*
** Check There Is Only One Hull Feature
*/
 if( numHulls + numDrapeHulls > 1 )
   {
    bcdtmWrite_message(1,0,0,"More Than One Hull Feature") ;
    goto errexit ;
   }
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Triangulate DTM Object
*/
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Triangulating Dtm Object") ;
 trgTime = bcdtmClock() ;
 if( bcdtmTin_createTinDtmObject(dtmP,edgeOption,maxSide,numGraphicBreaks,numContourLines,numSoftBreaks,numHardBreaks,numVoids,numIslands,numHoles,numBreakVoids,numDrapeVoids,numGroupSpots,numRegions,numHulls,numDrapeHulls,numHullLines) ) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Triangulate Dtm Object  = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),trgTime)) ;
/*
** Write Tin Stats
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Tin Points               = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"Number Tin Lines                = %8ld",dtmP->numLines) ;
    bcdtmWrite_message(0,0,0,"Number Tin Triangles            = %8ld",dtmP->numTriangles) ;
    bcdtmWrite_message(0,0,0,"Number Dtm Features             = %8ld",dtmP->numFeatures) ;
    numTinFeatures = 0 ;
    numErrorTinFeatures = 0 ;
    numDeletedFeatures = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if     ( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin ) ++numTinFeatures ;
       else if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError ) ++numErrorTinFeatures ;
       else if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Deleted   ) ++numDeletedFeatures ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          pntP = pointAddrP(dtmP,(long)dtmFeatureP->dtmFeaturePts.firstPoint) ;
          bcdtmWrite_message(0,0,0,"Tin Feature %30s ** featureId = %10I64d ** firstPoint = %12.5lf %12.5lf %10.4lf",dtmFeatureTypeName,dtmFeatureP->dtmFeatureId,pntP->x,pntP->y,pntP->z) ;
         }
      }
    bcdtmWrite_message(0,0,0,"Number Tin Features             = %8ld",numTinFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Tin Features With Errors = %8ld",numErrorTinFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Deleted Features         = %8ld",numDeletedFeatures) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Create Tin              = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),createTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Tin For Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Tin For Dtm Object %p Error",dtmP) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dtmP->dtmState == DTMState::DuplicatesRemoved || dtmP->dtmState == DTMState::PointsSorted || dtmP->dtmState == DTMState::TinError)
     bcdtmObject_changeStateDtmObject (dtmP, DTMState::Data);
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_processForTriangulationDtmObject
(
 BC_DTM_OBJ *dtmP,
 bool       normaliseOption,
 bool       duplicateOption)
/*
** This Function Prepares A Dtm Object For Triangulation
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long n,point,dtmFeature,startTime,firstPoint,numDuplicates ;
 DPoint3d  *hullPtsP=nullptr ;
 BC_DTM_FEATURE *dtmFeatureP=nullptr ;
 DTM_ROLLBACK_DATA* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : nullptr;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Processing Dtm Object %p For Triangulation",dtmP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Valid Prior Triangulation States
*/
 if( dtmP->dtmState != DTMState::Data && dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(0,0,0,"DTM Not In A Data or Tin State") ;
    goto errexit ;
   }
/*
** Check If Triangulation Already Exists
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Previously Triangulated") ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray || dtmFeatureP->dtmFeatureState == DTMFeatureState::Deleted)
         {
          startTime  = bcdtmClock() ;
          if ( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
          if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Change Dtm State        = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
          dtmFeature = dtmP->numFeatures ;
         }
      }
/*
**   Check For Inserted Points From Removed Features
*/
    if( dtmP->dtmState == DTMState::Tin )
      {
       for( point = 0 ; point < dtmP->numPoints ; ++point )
         {
          if( bcdtmFlag_testInsertPoint(dtmP,point))
            {
             if ( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
             point = dtmP->numPoints ;
            }
         }
      }
/*
**  If DTM Not In Data State No Need To Retriangulate
*/
    if( dtmP->dtmState == DTMState::Tin )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"No Need To Retriangulate") ;
       goto cleanup ;
      }
   }
/*
** If Roll Back Option Set Copy Selected DTM Features To Temporary Roll Back DTM
*/
 if(  rollBackInfo && rollBackInfo->rollBackDtmP != nullptr )
   {
    if ( bcdtmObject_testApiCleanUpDtmObject (dtmP, DTMCleanupFlags::Changes))
        {
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
          {
           dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
           if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
             {
              bool rollBack=false ;
              if     ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull          ) rollBack = true ;
              else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeHull    ) rollBack = true ;
              else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine     ) rollBack = true ;
              else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid    ) rollBack = true ;
              else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid    ) rollBack = true ;
              else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::VoidLine     )  rollBack = true ;
              if( rollBack )
                {
                 if (bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId)) goto errexit;
                }
             }
          }
        }
    if ( bcdtmCleanUp_cleanDtmObject (dtmP)) goto errexit;
   }
/*
** Remove Deleted Features
*/
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Removing Deleted Features") ;
 if( bcdtmData_compactUntriangulatedFeatureTableDtmObject(dtmP)) goto errexit ;
 if (rollBackInfo) rollBackInfo->rollBackMapInitialized = false;
/*
** Check For Less Than Three Points
*/
 if( dtmP->numPoints < 3 )
   {
    bcdtmWrite_message(1,0,0,"Less Than Three Points In DTM") ;
    goto errexit ;
   }
/*
** Check Dtm State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(2,0,0,"DTM In Wrong State For Prior Triangulation Processing") ;
    goto errexit ;
   }
/*
** Normalise Dtm Points
*/
 if (normaliseOption)
   {
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Normalising Dtm Points") ;
    if( bcdtmMath_normaliseCoordinatesDtmObject(dtmP)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Normalise Dtm Points    = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Calculate Machine Precision
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Machine Precision") ;
 bcdtmMath_calculateMachinePrecisionForDtmObject(dtmP) ;
/*
** Check Tolerances
*/
 if( dtmP->ppTol < dtmP->mppTol * 10000.0 )
   {
    dtmP->ppTol = dtmP->mppTol * 10000.0 ;
    dtmP->ppTol = dtmP->ppTol + dtmP->ppTol / 10.0 ;
    dtmP->plTol = dtmP->ppTol ;
   }
/*
** Set Bounding Cube For Dtm Object
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Bounding Cube For Dtm Object") ;
 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Set Bounding Cube       = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Add Point Offsets To Feature Array Entries
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Point Offsets To Feature Array Entries") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
      {
/*
**     Allocate Memory For Point Offsets
*/
       if( dbg == 2  ) bcdtmWrite_message(0,0,0,"dtmFeature[%5ld] ** type = %4ld firstPoint = %8ld numPoints = %8ld",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeaturePts.firstPoint,dtmFeatureP->numDtmFeaturePts) ;
       firstPoint = (long) dtmFeatureP->dtmFeaturePts.firstPoint ;
       dtmFeatureP->dtmFeaturePts.offsetPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts * sizeof(long)) ;
       if( dtmFeatureP->dtmFeaturePts.offsetPI == 0 )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
/*
**     Set Dtm Feature State
*/
       dtmFeatureP->dtmFeatureState = DTMFeatureState::OffsetsArray ;
/*
**     Populate Point Offsets
*/
       long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          offsetP[n] = firstPoint ;
          ++firstPoint ;
         }
/*
**     Write Feature Stats
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmFeature = %6ld type = %4ld offsetP = %p numDtmFeaturePts = %6ld",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeaturePts.offsetPI,dtmFeatureP->numDtmFeaturePts) ;
      }
   }
/*
** Check Point Offsets Before Sorting
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Point Offsets For Range Errors Before Sorting") ;
    if( bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Checking For Duplicate Index Offset Memory Points") ;
    if( bcdtmCheck_forDuplicatePointOffsetPointersDtmObject(dtmP)) goto errexit ;
   }
/*
** Sort Dtm Object
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting DTM Object") ;
 if( bcdtmObject_sortDtmObject(dtmP)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Sort Dtm Points         = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Check Point Offsets After Sorting
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Check Point Offsets For Range Errors After Sorting") ;
    if( bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Checking For Duplicate Index Offset Memory Points") ;
    if( bcdtmCheck_forDuplicatePointOffsetPointersDtmObject(dtmP)) goto errexit ;
   }
/*
** Check Sort Order
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Sort Order After Sorting Points") ;
    if( bcdtmCheck_sortOrderDtmObject(dtmP,0) != DTM_SUCCESS )
      {
       bcdtmWrite_message(1,0,0,"Dtm Sort Order Error") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Sort Order Valid After Sorting Points") ;
   }
/*
** Remove Duplicates
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicates DTM Object") ;
 if( bcdtmObject_removeDuplicatesDtmObject(dtmP,&numDuplicates, duplicateOption)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates Removed = %8ld",numDuplicates) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Remove Duplicate Points = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check Point Offsets After Duplicate Removal
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Check Point Offsets For Range Errors After After Duplicate Removal") ;
    if( bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Checking For Duplicate Index Offset Memory Points") ;
    if( bcdtmCheck_forDuplicatePointOffsetPointersDtmObject(dtmP)) goto errexit ;
   }
/*
** Remove Duplicates Point Offsets
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicates Point Offsets DTM Object") ;
 if( bcdtmObject_removeDuplicatePointOffsetsDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints = %8ld",dtmP->numSortedPoints) ;
/*
** Check Point Offsets After Duplicate Points Offsets Removal
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Check Point Offsets For Range Errors After After Duplicate Point Offsets Removal") ;
    if( bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Checking For Duplicate Index Offset Memory Points") ;
    if( bcdtmCheck_forDuplicatePointOffsetPointersDtmObject(dtmP)) goto errexit ;
   }
/*
** Check For Less Than Three Points
*/
 if( dtmP->numPoints < 3 )
   {
    bcdtmWrite_message(1,0,0,"Less Than Three Points In DTM") ;
    goto errexit ;
   }
/*
** Check Sort Order
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Sort Order After Removing Duplicates dtmP->numPoints = %8ld",dtmP->numPoints) ;
    if( bcdtmCheck_sortOrderDtmObject(dtmP,1) != DTM_SUCCESS )
      {
       bcdtmWrite_message(1,0,0,"Dtm Sort Order Error") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Sort Order Valid After Removing Duplicates") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Processing Dtm Object %p For Triangulation Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Processing Dtm Object %p For Triangulation Error",dtmP) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_changeStateDtmFeaturesToOffsetsArrayDtmObject(BC_DTM_OBJ *dtmP)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,dtmFeature,firstPoint ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Changing State DTM Features Completed") ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Valid DTM State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   }
/*
** Scan Features And Change state
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
      {
/*
**     Allocate Memory For Point Offsets
*/
       firstPoint = (long ) dtmFeatureP->dtmFeaturePts.firstPoint ;
       dtmFeatureP->dtmFeaturePts.offsetPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts * sizeof(long)) ;
       if( dtmFeatureP->dtmFeaturePts.offsetPI == 0 )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
/*
**     Set Dtm Feature State
*/
       dtmFeatureP->dtmFeatureState = DTMFeatureState::OffsetsArray ;
/*
**     Populate Point Offsets
*/
       long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          offsetP[n] = firstPoint ;
          ++firstPoint ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Changing State DTM Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Changing State DTM Features Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_moveOrCopyPointsArray(BC_DTM_OBJ* dtmP, BC_DTM_OBJ* tempDtmP)
{
 int n,ret = DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 if(dtmP->dtmObjType != BC_DTM_ELM_TYPE && tempDtmP->dtmObjType != BC_DTM_ELM_TYPE)
   {
    for( n = 0 ; n < dtmP->numPointPartitions ; ++n ) bcdtmMemory_freePartition(dtmP, DTMPartition::Point, n, dtmP->pointsPP[n]) ;
    free(dtmP->pointsPP) ;
    dtmP->pointsPP           = tempDtmP->pointsPP ;
    dtmP->numPoints          = tempDtmP->numPoints ;
    dtmP->memPoints          = tempDtmP->memPoints ;
    dtmP->numPointPartitions = tempDtmP->numPointPartitions ;
    dtmP->pointPartitionSize = tempDtmP->pointPartitionSize ;
    dtmP->numSortedPoints    = 0 ;
/*
**  Set Temporary DTM Objects Points To nullptr
*/
    tempDtmP->pointsPP       = nullptr ;
   }
 else
   {
    dtmP->numPoints = tempDtmP->numPoints ;
/*
**  Allocate Memory For Feature Table
*/
    if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit ;
/*
**  Copy Points
*/
    if( tempDtmP->memPoints > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Copying Points") ;
/*
**     For DTMElement we can't just move the pointers over,
**     so we need to reallocate and copy the data over.
*/
       for( int num = 0 ; num < tempDtmP->numPoints ; ++num )
         {
          *pointAddrP(dtmP,num) = *pointAddrP(tempDtmP,num) ;
         }
      }
    dtmP->numSortedPoints = 0 ;
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
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_changeStateDtmObject(BC_DTM_OBJ *dtmP,DTMState toState )
/*
** This Function Changes The State Of A DTM Object
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,node,point,dtmFeature,numFeaturePts,firstPoint,hullFeature=FALSE,markSize ;
 DPoint3d  randomPoint,*featurePtsP=nullptr ;
 char dtmFeatureTypeName[50];
 unsigned char *markFlagP=nullptr ;
 BC_DTM_FEATURE *dtmFeatureP=nullptr ;
 BC_DTM_OBJ     *tempDtmP=nullptr ;
 DTM_TIN_POINT  *pointP ;
 DTMFeatureId dtmFeatureId ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ; 
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Changing State Of DTM Object %p From %2ld To %2ld",dtmP,dtmP->dtmState,toState) ;
/*
** Initialise
*/
 dtmFeatureId = dtmP->nullFeatureId ;
/*
** Change From DTMState::Tin to DTMState::Data
*/
 if( bcdtmObject_testApiCleanUpDtmObject(dtmP, DTMCleanupFlags::All) && toState == DTMState::Data )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Rolling Back Dtm From DTMState::Tin to DTMState::Data" ) ;
    if( bcdtmObject_rollBackDtmObject(dtmP,1)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Rolling Back Dtm From DTMState::Tin to DTMState::Data Completed" ) ;
   }
 else if( dtmP->dtmState == DTMState::Tin && toState == DTMState::Data )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Changing Dtm State From DTMState::Tin to DTMState::Data" ) ;
/*
**  Create Temporary DTM Object For Restructuring Points Array
*/
    if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
    tempDtmP->iniPoints = dtmP->numPoints ;
    tempDtmP->incPoints = dtmP->incPoints ;
/*
**  Release Clist Array As It Is Not Required
*/
    if( dtmP->cListPP != nullptr )
      {
       for( n = 0 ; n < dtmP->numClistPartitions ; ++n ) bcdtmMemory_freePartition(dtmP, DTMPartition::CList, n, dtmP->cListPP[n]) ;
       free(dtmP->cListPP) ;
       dtmP->cListPP = nullptr ;
       dtmP->numClistPartitions = 0 ;
       dtmP->numClist           = 0 ;
       dtmP->memClist           = 0 ;
       dtmP->cListPtr           = 0 ;
       dtmP->cListDelPtr        = dtmP->nullPtr ;
      }
/*
**  No Features In DTM
*/
    if( dtmP->numFeatures == 0 )
      {
       if( dtmP->nodesPP != nullptr )
         {
          for( n = 0 ; n < dtmP->numNodePartitions ; ++n ) bcdtmMemory_freePartition(dtmP, DTMPartition::Node, n, dtmP->nodesPP[n]) ;
          free(dtmP->nodesPP) ;
          dtmP->nodesPP = nullptr ;
          dtmP->numNodePartitions = 0 ;
          dtmP->numNodes = 0 ;
          dtmP->memNodes = 0 ;
         }
      }

/*
**  Features In Dtm Object
*/
    if( dtmP->numFeatures > 0 )
      {
/*
**     Write Out Dtm Features
*/
       if( dbg == 1 )
         {
          bcdtmWrite_message(0,0,0,"Number Of DTM Feature = %6ld",dtmP->numFeatures) ;
          for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
             dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
             if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
             bcdtmWrite_message(0,0,0,"Dtm Feature[%6ld] ** Type = %20s ** userTag = %11I64d featureId = %11I64d ** state = %2ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmFeatureState) ;
            }
         }
/*
**     Store Hull Points
*/
       if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
/*
**     Copy Feature Points To Temporary Dtm Object Points Array
*/
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**        Existing Tin Feature
*/
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
             if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull )
               {
                if( bcdtmList_extractHullDtmObject(dtmP,&featurePtsP,&numFeaturePts)) goto errexit ;
                hullFeature = TRUE ;
               }
             else
               {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
               }
             if( dbg )
               {
                if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
                bcdtmWrite_message(0,0,0,"Tin Dtm Feature ** dtmFeature = %6ld ** Type = %20s ** userTag = %11I64ld featureId = %10I64ld ** numFeaturePts = %6ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,numFeaturePts) ;
               }
             dtmFeatureP->dtmFeatureState          = DTMFeatureState::Data ;
             dtmFeatureP->dtmFeaturePts.firstPoint = tempDtmP->numPoints ;
             dtmFeatureP->numDtmFeaturePts         = numFeaturePts ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
             free(featurePtsP) ;
             featurePtsP = nullptr ;
            }
/*
**        New Dtm Feature
*/
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray && dtmFeatureP->dtmFeatureType != DTMFeatureType::DrapeVoid && dtmFeatureP->dtmFeatureType != DTMFeatureType::DrapeHull )
            {
             if( dbg )
               {
                if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
                bcdtmWrite_message(0,0,0,"New Dtm Feature ** dtmFeature = %6ld ** Type = %20s ** userTag = %11I64ld featureId = %10I64ld ** numFeaturePts = %6ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,dtmFeatureP->numDtmFeaturePts) ;
               }
/*
**           Add New Feature
*/
             firstPoint = tempDtmP->numPoints ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts)) goto errexit ;
             bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
             dtmFeatureP->dtmFeaturePts.pointsPI = 0;
             if( dtmFeatureP->dtmFeatureType == DTMFeatureType::RandomSpots )
               {
                dtmFeatureP->dtmFeatureState          = DTMFeatureState::Deleted ;
                dtmFeatureP->numDtmFeaturePts         = 0 ;
               }
             else
               {
                dtmFeatureP->dtmFeatureState          = DTMFeatureState::Data ;
                dtmFeatureP->dtmFeaturePts.firstPoint = firstPoint ;
               }
            }
         }
/*
**     Copy Random Points ( None Feature Points )
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Copying Random Points To New Point Array") ;
       for( node = 0 ; node < dtmP->numPoints ; ++node )
         {
          if( ! bcdtmFlag_testDeletePointBitPCWD(&nodeAddrP(dtmP,node)->PCWD))
            {
             if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,node)->PCWD))
               {
                if( nodeAddrP(dtmP,node)->hPtr == dtmP->nullPnt || ( nodeAddrP(dtmP,node)->hPtr != dtmP->nullPnt && hullFeature == FALSE ))
                  {
                   if( nodeAddrP(dtmP,node)->fPtr == dtmP->nullPtr )
                     {
                      pointP = pointAddrP(dtmP,node) ;
                      randomPoint.x = pointP->x ;
                      randomPoint.y = pointP->y ;
                      randomPoint.z = pointP->z ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"Copying Random Point %12.5lf %12.5lf %10.4lf",randomPoint.x,randomPoint.y,randomPoint.z);
                      if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&randomPoint,1)) goto errexit ;
                     }
                  }
               }
            }
         }
/*
**     Free points Array
*/
       if( dtmP->pointsPP != nullptr )
         {
 //         bcdtmObject_moveOrCopyPointsArray(dtmP, tempDtmP);
          dtmP->numPoints = 0 ;
          dtmP->dtmState  = DTMState::Data ;
          for( point = 0 ; point < tempDtmP->numPoints ; ++point )
            {
             if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(tempDtmP,point),1)) goto errexit ;
            }
          dtmP->numSortedPoints = 0 ;
         }
/*
**     Free Nodes Array
*/
       if( dtmP->nodesPP != nullptr )
         {
          for( n = 0 ; n < dtmP->numNodePartitions ; ++n )  bcdtmMemory_freePartition(dtmP, DTMPartition::Node, n, dtmP->nodesPP[n]) ;
          free(dtmP->nodesPP) ;
          dtmP->nodesPP = nullptr ;
          dtmP->numNodePartitions = 0 ;
          dtmP->numNodes = 0 ;
          dtmP->memNodes = 0 ;
         }
/*
**     Free Flist Array
*/
       if( dtmP->fListPP != nullptr )
         {
          for( n = 0 ; n < dtmP->numFlistPartitions ; ++n ) bcdtmMemory_freePartition(dtmP, DTMPartition::FList, n, dtmP->fListPP[n]) ;
          free(dtmP->fListPP) ;
          dtmP->fListPP            = nullptr ;
          dtmP->numFlistPartitions = 0 ;
          dtmP->numFlist           = 0 ;
          dtmP->memFlist           = 0 ;
          dtmP->fListDelPtr        = dtmP->nullPtr ;
         }
      }
/*
**  Reset DTM Header Variables
*/
    dtmP->dtmState           = DTMState::Data ;
    dtmP->hullPoint          = dtmP->nullPnt ;
    dtmP->nextHullPoint      = dtmP->nullPnt ;
    dtmP->numLines           = 0 ;
    dtmP->numTriangles       = 0 ;
   }
/*
** Change From DTMState::PointsSorted to DTMState::Data
*/
 if( (dtmP->dtmState == DTMState::PointsSorted || dtmP->dtmState == DTMState::DuplicatesRemoved || dtmP->dtmState == DTMState::TinError) && toState == DTMState::Data )
   {
    if( dbg && dtmP->dtmState == DTMState::PointsSorted) bcdtmWrite_message(0,0,0,"Changing Dtm State From DTMState::PointsSorted to DTMState::Data" ) ;
    if( dbg && dtmP->dtmState == DTMState::DuplicatesRemoved) bcdtmWrite_message(0,0,0,"Changing Dtm State From DTMState::DuplicatesRemoved to DTMState::Data" ) ;

    // Probably need to scan to see if they are any DTMFeatureState::Tin FEATURES in the DTM.
    if (dtmP->dtmState == DTMState::TinError)
        {
        if( dtmP->cListPP != nullptr )
          {
           for( n = 0 ; n < dtmP->numClistPartitions ; ++n ) bcdtmMemory_freePartition(dtmP, DTMPartition::CList, n, dtmP->cListPP[n]) ;
           free(dtmP->cListPP) ;
           dtmP->cListPP = nullptr ;
           dtmP->numClistPartitions = 0 ;
           dtmP->numClist           = 0 ;
           dtmP->memClist           = 0 ;
           dtmP->cListPtr           = 0 ;
           dtmP->cListDelPtr        = dtmP->nullPtr ;
          }
/*
**     Free Nodes Array
*/
       if( dtmP->nodesPP != nullptr )
         {
          for( n = 0 ; n < dtmP->numNodePartitions ; ++n )  bcdtmMemory_freePartition(dtmP, DTMPartition::Node, n, dtmP->nodesPP[n]) ;
          free(dtmP->nodesPP) ;
          dtmP->nodesPP = nullptr ;
          dtmP->numNodePartitions = 0 ;
          dtmP->numNodes = 0 ;
          dtmP->memNodes = 0 ;
         }
/*
**     Free Flist Array
*/
       if( dtmP->fListPP != nullptr )
         {
          for( n = 0 ; n < dtmP->numFlistPartitions ; ++n ) bcdtmMemory_freePartition(dtmP, DTMPartition::FList, n, dtmP->fListPP[n]) ;
          free(dtmP->fListPP) ;
          dtmP->fListPP            = nullptr ;
          dtmP->numFlistPartitions = 0 ;
          dtmP->numFlist           = 0 ;
          dtmP->memFlist           = 0 ;
          dtmP->fListDelPtr        = dtmP->nullPtr ;
         }
/*
**  Reset DTM Header Variables
*/
        dtmP->hullPoint          = dtmP->nullPnt ;
        dtmP->nextHullPoint      = dtmP->nullPnt ;
        dtmP->numLines           = 0 ;
        dtmP->numTriangles       = 0 ;
        }

/*
**  If There Are No features Just Reset State To DTMState::Data
*/
    if( dtmP->numFeatures == 0 ) dtmP->dtmState = DTMState::Data ;
/*
**  Else Place Feature Points In First Point Order
*/
    else
      {
/*
**     Allocate An Array For Marking Random Points
*/
       markSize = dtmP->numPoints / 8 + 1 ;
       markFlagP = (unsigned char *) malloc( markSize * sizeof(char));
       if( markFlagP == nullptr )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       for( n = 0 ; n < markSize ; ++n ) *(markFlagP+n) = (char) 0 ;
/*
**     Create Temporary DTM Object For Restructuring Points Array
*/
       if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
       tempDtmP->iniPoints = dtmP->numPoints ;
       tempDtmP->incPoints = dtmP->incPoints ;
/*
**     Copy Feature Points To Temporary Dtm Object Points Array
*/
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;

          if(dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
              {

              if( dtmFeatureP->dtmFeatureState != DTMFeatureState::OffsetsArray)
                {
                 bcdtmWrite_message(2,0,0,"Invalid DTM Feature State For Method") ;
                 goto errexit ;
                }

/*
**        Mark Feature Points Copied
*/
              long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
              for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
                {
                 bcdtmFlag_setFlag(markFlagP,offsetP[n]) ;
                }
/*
**        Store Feature Points In Temporary DTM Object
*/
              if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
              if( dbg )
                {
                 if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
                 bcdtmWrite_message(0,0,0,"Dtm Feature ** dtmFeature = %6ld ** Type = %20s ** userTag = %11I64d featureId = %11I64d ** numFeaturePts = %6ld",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,numFeaturePts) ;
                }
              dtmFeatureP->dtmFeatureState          = DTMFeatureState::Data ;
              dtmFeatureP->dtmFeaturePts.firstPoint = tempDtmP->numPoints ;
              dtmFeatureP->numDtmFeaturePts         = numFeaturePts ;
              if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
              free(featurePtsP) ;
              featurePtsP = nullptr ;
              }
         }
/*
**     Copy Random Points
*/
       for( n = 0 ; n < dtmP->numPoints ; ++n )
         {
          if( ! bcdtmFlag_testFlag(markFlagP,n ))
            {
             pointP = pointAddrP(dtmP,n) ;
             randomPoint.x = pointP->x ;
             randomPoint.y = pointP->y ;
             randomPoint.z = pointP->z ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmFeatureId,&randomPoint,1)) goto errexit ;
            }
         }
/*
**     Free DTM Points Array
*/
//     bcdtmObject_moveOrCopyPointsArray(dtmP, tempDtmP);
       dtmP->numPoints = 0 ;
       dtmP->dtmState  = DTMState::Data ;
       for( point = 0 ; point < tempDtmP->numPoints ; ++point )
         {
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointAddrP(tempDtmP,point),1)) goto errexit ;
         }
      dtmP->numSortedPoints = 0 ;
/*
**     Reset DTM Header Values
*/
       dtmP->dtmState           = DTMState::Data ;
//       dtmP->pointsPP            = tempDtmP->pointsPP ;
//       dtmP->numPoints          = tempDtmP->numPoints ;
//       dtmP->memPoints          = tempDtmP->memPoints ;
//       dtmP->numPointPartitions = tempDtmP->numPointPartitions ;
//       dtmP->pointPartitionSize = tempDtmP->pointPartitionSize ;
       dtmP->numSortedPoints    = 0 ;
//       tempDtmP->pointsPP       = nullptr ;

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
 if( markFlagP   != nullptr ) free(markFlagP) ;
 if( featurePtsP != nullptr ) free(featurePtsP) ;
 if( tempDtmP    != nullptr ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Changing DTM %p State To %2ld Completed",dtmP,toState) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Changing DTM %p State To %2ld Error",dtmP,toState) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
typedef PartitionArray<DTM_TIN_POINT, DTM_PARTITION_SHIFT_POINT, MAllocAllocator> DtmTinPointArray;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_sortDtmObjectDaryl(BC_DTM_OBJ *dtmP)
/*
** This Function Sorts A Dtm Object
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   n,feature ;
// DTM_TIN_POINT   dtmPoint,*p1P,*p2P  ;
 BC_DTM_FEATURE *dtmFeatureP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Dtm Object %p",dtmP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Sort If Dtm Object Not Prior Sorted
*/
 if( dtmP->dtmState == DTMState::Data )
   {
    PartitionArray<DTM_TIN_POINT, DTM_PARTITION_SHIFT_POINT, MAllocAllocator> pointsArray(dtmP->pointsPP, dtmP->numPoints, dtmP->numPointPartitions, dtmP->pointPartitionSize);
/*
**  Only Sort If More Than One Dtm Point
*/
    if( dtmP->numPoints > 1 )
      {
/*
**     Sort the Dtm Points
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Sorting") ;
       XYPointArraySort<DTM_TIN_POINT,DtmTinPointArray>  sorter;
       sorter.DoSort(pointsArray);
       sorter.GetSortP();
       LongArray& tempP = sorter.GetTempP();
/*
**     Place Dtm Feature Point Offsets In Sort Order
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Placing Feature Points In Sort Order") ;
       for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
         {
          dtmFeatureP = ftableAddrP(dtmP,feature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
            {
/*
**           Reset Point Offsets
*/
            long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
             for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
               {
                offsetP[n] = *(tempP+offsetP[n]) ;
               }
            }
         }
      }
   }
/*
**  Set Dtm State To Sorted
*/
 dtmP->dtmState = DTMState::PointsSorted ;
/*
** Set Number Of Sorted Points
*/
 dtmP->numSortedPoints = dtmP->numPoints ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Sorting Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Sorting Dtm Object %p Error",dtmP) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_sortDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Sorts A Dtm Object
**
** Author : Rob Cormack  January 2007  Rob.Cormack@Bentley.com
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   ofs,useMultiThread=TRUE ;
 DTM_TIN_POINT *p1P,*p2P  ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Sorting Dtm Object   = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"DTM_NUM_PROCESSORS   = %8ld",DTM_NUM_PROCESSORS) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Multi Thread If More Than 100 Points
*/
 if( dtmP->numPoints < 100 ) useMultiThread = FALSE ;
/*
** Only Sort If Dtm In Data State
*/
  if( dtmP->dtmState == DTMState::Data )
   {
/*
**  Only Sort If More Than One Dtm Point
*/
    if( dtmP->numPoints > 1 )
      {
       if( DTM_NUM_PROCESSORS == 1 || useMultiThread == FALSE )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Single Thread Sorting") ;
          if( bcdtmObject_singleThreadSortDtmObject(dtmP)) goto errexit ;
         }
       else
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Multi Thread Sorting") ;
          if( bcdtmObject_multiThreadSortDtmObject(dtmP)) goto errexit ;
         }
/*
**     Check Points Are Sorted
*/
       if( cdbg )
         {
          bcdtmWrite_message(0,0,0,"Checking Point Sort Order") ;
          p1P = pointAddrP(dtmP,0) ;
          for( ofs = 1 ; ofs < dtmP->numPoints ; ++ofs )
            {
             p2P = pointAddrP(dtmP,ofs) ;
             if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
               {
                bcdtmWrite_message(1,0,0,"Point Sort Order Error") ;
                bcdtmWrite_message(0,0,0,"p1 = %8ld ** %12.5lf %12.5lf %10.4lf",ofs-1,p1P->x,p1P->y,p1P->z) ;
                bcdtmWrite_message(0,0,0,"p2 = %8ld ** %12.5lf %12.5lf %10.4lf",ofs,p2P->x,p2P->y,p2P->z) ;
                goto errexit ;
               }
             p1P = p2P ;
            }
          bcdtmWrite_message(0,0,0,"Point Sort Order Valid") ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Sorting Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Sorting Dtm Object %p Error",dtmP) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmObject_singleThreadSortDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Single Thread Sorts A Dtm Object
**
** Author : Rob Cormack  November 2008  Rob.Cormack@Bentley.com
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   n,ofs,*sP,*offsetP,*sortP=nullptr,*tempP=nullptr,feature ;
 long   startTime ;
 DTM_TIN_POINT   dtmPoint,*p1P,*p2P  ;
 BC_DTM_FEATURE  *dtmFeatureP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Single Thread Sorting Dtm Object %p",dtmP) ;
 startTime = bcdtmClock() ;
/*
** Allocate Memory For Sort Offset Pointers
*/
 sortP = ( long *  ) malloc(dtmP->numPoints*sizeof(long)) ;
 if( sortP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
 tempP = ( long *  ) malloc(dtmP->numPoints*sizeof(long)) ;
 if( tempP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
/*
** Initialise Sort Offset Pointer
*/
 for( sP = sortP, ofs = 0 ; sP < sortP + dtmP->numPoints ; ++sP, ++ofs) *sP = ofs ;
/*
** Sort the Dtm Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting") ;
 bcdtmObject_divConqMergeSortDtmObject(dtmP,0,dtmP->numPoints,sortP,tempP) ;
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Calculate Dtm Point Sort Position
*/
 for( sP = sortP ; sP < sortP + dtmP->numPoints ; ++sP  ) *(tempP+*sP) = (long)(sP-sortP) ;
/*
** Place Dtm Feature Point Offsets In Sort Order
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Placing Feature Points In Sort Order") ;
 for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
   {
    dtmFeatureP = ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
      {
/*
**     Reset Point Offsets
*/
       offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          offsetP[n] = *(tempP+offsetP[n]) ;
         }
      }
   }
/*
** Place Dtm Points In Sort Order
*/
 for( sP = sortP , ofs = 0 ; sP < sortP + dtmP->numPoints ; ++sP , ++ofs )
   {
    p1P = pointAddrP(dtmP,ofs) ;
    p2P = pointAddrP(dtmP,*sP) ;
    dtmPoint = *p1P ;
    *p1P = *p2P ;
    *p2P = dtmPoint ;
    *(sortP+*(tempP+ofs)) = *sP ;
    *(tempP+*sP) = *(tempP+ofs) ;
   }
/*
**  Set Dtm State To Sorted
*/
 dtmP->dtmState = DTMState::PointsSorted ;
/*
** Clean Up
*/
 cleanup :
 if( sortP != nullptr ) free(sortP) ;
 if( tempP != nullptr ) free(tempP) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time To Single Thread Sort = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Single Thread Sorting Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Single Thread Sorting Dtm Object %p Error",dtmP) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
void bcdtmTin_multiThreadSortWorkerDtmObject (DTM_MULTI_THREAD *srtParametersP)
/*
** This function uses a divide and conquer strategy to sort DTM Points
**
** Author : Rob Cormack  November 2008  Rob.Cormack@Bentley.com
**
*/
    {
    int    ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0), tdbg = DTM_TIME_VALUE (0);
    __time32_t startTime;

    /*
    ** Initialise variables
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** startPoint = %8ld numPoints = %8ld", srtParametersP->thread, srtParametersP->startPoint, srtParametersP->numPoints);
    /*
    ** Sort Points Into Tiles
    */
    startTime = bcdtmClock ();
    if (dbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** Sorting %9ld Dtm Points", srtParametersP->thread, srtParametersP->numPoints);
    if (bcdtmObject_divConqMergeSortDtmObject (srtParametersP->dtmP, srtParametersP->startPoint, srtParametersP->numPoints, srtParametersP->sortOfsP, srtParametersP->tempOfsP)) goto errexit;
    if (tdbg) bcdtmWrite_message (0, 0, 0, "Thread[%2ld] ** Sorting Time = %8.3lf Seconds", srtParametersP->thread, bcdtmClock_elapsedTime (bcdtmClock (), startTime));
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Job Completed
    */
    return;
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
BENTLEYDTM_Private int bcdtmObject_multiThreadSortDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Multi Thread Sorts A Dtm Object
**
** Author : Rob Cormack  November 2008  Rob.Cormack@Bentley.com
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   i,n,ofs,*sP,*offsetP,*sortP=nullptr,*tempP=nullptr,feature ;
 long   startTime ;
 long   numPts,numPts1,numPts2,startPnt1,startPnt2,endPnt1,endPnt2 ;
 DTM_TIN_POINT   dtmPoint,*p1P,*p2P  ;
 BC_DTM_FEATURE  *dtmFeatureP=nullptr ;
 long startPoint, numThreadPoints = 0;
 bvector<long> numThreadArrayPoints;
 std::vector<std::thread> thread;
 bvector<DTM_MULTI_THREAD> multiThread;
 /*
 ** Resize arrays
 */
 numThreadArrayPoints.resize (DTM_NUM_PROCESSORS);
 thread.resize (DTM_NUM_PROCESSORS);;
 multiThread.resize (DTM_NUM_PROCESSORS);
 /*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Multi Thread Sorting Dtm Object %p",dtmP) ;
 startTime = bcdtmClock() ;
/*
** Allocate Memory For Sort Offset Pointers
*/
 sortP = ( long *  ) malloc(dtmP->numPoints*sizeof(long)) ;
 if( sortP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
 tempP = ( long *  ) malloc(dtmP->numPoints*sizeof(long)) ;
 if( tempP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
/*
** Initialise Sort Offset Pointer
*/
 for( sP = sortP, ofs = 0 ; sP < sortP + dtmP->numPoints ; ++sP, ++ofs) *sP = ofs ;
/*
** Determine Number Of Points Per Thread
*/
 numThreadPoints = dtmP->numPoints / DTM_NUM_PROCESSORS + 1 ;
 if( dtmP->numPoints % DTM_NUM_PROCESSORS == 0 ) --numThreadPoints ;
 for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n ) numThreadArrayPoints[n] =  numThreadPoints ;
 if( dtmP->numPoints % DTM_NUM_PROCESSORS != 0 ) numThreadArrayPoints[DTM_NUM_PROCESSORS-1] = dtmP->numPoints - (DTM_NUM_PROCESSORS-1) * numThreadPoints ;
 if( dbg ) for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n  ) bcdtmWrite_message(0,0,0,"Thread[%2ld] ** numPoints = %6ld",n,numThreadArrayPoints[n]) ;
/*
** Initialise Multi Thread Sort Array
*/
 startPoint = 0 ;
 for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
   {
    multiThread[n].thread          = n ;
    multiThread[n].dtmP            = dtmP ;
    multiThread[n].startPoint      = startPoint ;
    multiThread[n].numPoints       = numThreadArrayPoints[n] ;
    multiThread[n].sortOfsP        = sortP ;
    multiThread[n].tempOfsP        = tempP ;
    startPoint = startPoint + numThreadArrayPoints[n] ;
   }
/*
** Create Sorting Threads To Triangulate Dtm Object
*/
 for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
   {
   thread[n] = std::thread (bcdtmTin_multiThreadSortWorkerDtmObject, &multiThread[n]);
        //CreateThread(nullptr,0,(LPTHREAD_START_ROUTINE) bcdtmTin_multiThreadSortWorkerDtmObject,&multiThread[n],0,&threadId[n]) ;
   }
/*
** Wait For All Threads To Complete
*/
 for (n = 0; n < DTM_NUM_PROCESSORS; ++n)
     thread[n].join ();
 //WaitForMultipleObjects(DTM_NUM_PROCESSORS,thread,true,INFINITE) ;
 //for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n ) CloseHandle(thread[n]) ;
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Check Threads Are Sorted
*/
 if( cdbg )
   {
    for( n = 0 ; n < DTM_NUM_PROCESSORS ; ++n )
      {
       bcdtmWrite_message(0,0,0,"Thread[%2ld] ** Checking Point Sort Order",n) ;
       numPts1   = multiThread[n].numPoints  ;
       startPnt1 = multiThread[n].startPoint ;
       p1P = pointAddrP(dtmP,*(sortP+startPnt1)) ;
       for( ofs = startPnt1 + 1 ; ofs < startPnt1 + numPts1 ; ++ofs )
         {
          p2P = pointAddrP(dtmP,*(sortP+ofs)) ;
          if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
            {
             bcdtmWrite_message(1,0,0,"Thread[%2ld] ** Point Sort Order Error",n) ;
             bcdtmWrite_message(0,0,0,"p1 = %8ld ** %12.5lf %12.5lf %10.4lf",ofs-1,p1P->x,p1P->y,p1P->z) ;
             bcdtmWrite_message(0,0,0,"p2 = %8ld ** %12.5lf %12.5lf %10.4lf",ofs,p2P->x,p2P->y,p2P->z) ;
             goto errexit ;
            }
          p1P = p2P ;
         }
       bcdtmWrite_message(0,0,0,"Thread[%2ld] ** Point Sort Order Valid",n) ;
      }
   }
/*
** Merge Threaded Sorts
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Merging Threaded Sorts") ;
 for( n = 1 ; n < DTM_NUM_PROCESSORS ; ++n )
   {
    numPts1    = multiThread[0].numPoints  ;
    numPts2    = multiThread[n].numPoints  ;
    startPnt1  = multiThread[0].startPoint ;
    startPnt2  = multiThread[n].startPoint ;
    numPts     = numPts1 + numPts2         ;
    endPnt1    = startPnt1 + numPts1 ;
    endPnt2    = startPnt2 + numPts2 ;
    for( i = 0 ; i <  numPts  ; ++i )
      {
       if     ( startPnt1 >= endPnt1 ) {*(tempP+i)=*(sortP+startPnt2);++startPnt2;}
       else if( startPnt2 >= endPnt2 ) {*(tempP+i)=*(sortP+startPnt1);++startPnt1;}
       else
         {
          p1P = pointAddrP(dtmP,*(sortP+startPnt1)) ;
          p2P = pointAddrP(dtmP,*(sortP+startPnt2)) ;
          if( p1P->x < p2P->x || ( p1P->x == p2P->x && p1P->y <= p2P->y))
            {
             *(tempP+i) = *(sortP+startPnt1) ;
             ++startPnt1 ;
            }
          else
            {
             *(tempP+i) = *(sortP+startPnt2) ;
             ++startPnt2 ;
            }
         }
      }
/*
**  Restore Sort Pointers
*/
    multiThread[0].numPoints = numPts ;
    for( i = 0 ; i <  numPts ; ++i ) *(sortP+i) = *(tempP+i) ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Merging Threaded Sorts Completed") ;
/*
** Calculate Dtm Point Sort Position
*/
 for( sP = sortP ; sP < sortP + dtmP->numPoints ; ++sP  ) *(tempP+*sP) = (long)(sP-sortP) ;
/*
** Place Dtm Feature Point Offsets In Sort Order
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Placing Feature Points In Sort Order") ;
 for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
   {
    dtmFeatureP = ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
      {
/*
**     Reset Point Offsets
*/
       offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          offsetP[n] = *(tempP+offsetP[n]) ;
         }
      }
   }
/*
** Place Dtm Points In Sort Order
*/
 for( sP = sortP , ofs = 0 ; sP < sortP + dtmP->numPoints ; ++sP , ++ofs )
   {
    p1P = pointAddrP(dtmP,ofs) ;
    p2P = pointAddrP(dtmP,*sP) ;
    dtmPoint = *p1P ;
    *p1P = *p2P ;
    *p2P = dtmPoint ;
    *(sortP+*(tempP+ofs)) = *sP ;
    *(tempP+*sP) = *(tempP+ofs) ;
   }
/*
**  Set Dtm State To Sorted
*/
 dtmP->dtmState = DTMState::PointsSorted ;
/*
** Clean Up
*/
 cleanup :
 if( sortP != nullptr ) free(sortP) ;
 if( tempP != nullptr ) free(tempP) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time To Multi Thread Sort = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Multi Thread Sorting Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Multi Thread Sorting Dtm Object %p Error",dtmP) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_divConqMergeSortDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long numPts,long *sortP, long *tempP )
/*
** This Routine Sorts The Dtm Object By A Combined Divide And Conquer Merging Method
*/
{
 long  i,temp,numPts1,numPts2,startPnt1,startPnt2 ;
 DTM_TIN_POINT *p1P , *p2P ;
/*
** Two data points
*/
 if( numPts == 2 )
   {
    p1P = pointAddrP(dtmP,*(sortP+startPnt)) ;
    p2P = pointAddrP(dtmP,*(sortP+startPnt+1)) ;
    if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
      {
       temp = *(sortP+startPnt) ;
       *(sortP+startPnt)   = *(sortP+startPnt+1) ;
       *(sortP+startPnt+1) = temp ;
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
    bcdtmObject_divConqMergeSortDtmObject(dtmP,startPnt1,numPts1,sortP,tempP) ;
    bcdtmObject_divConqMergeSortDtmObject(dtmP,startPnt2,numPts2,sortP,tempP) ;
/*
**  Merge data sets
*/
    p1P = pointAddrP(dtmP,*(sortP+startPnt2-1)) ;
    p2P = pointAddrP(dtmP,*(sortP+startPnt2)) ;
    if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
      {
      long* tempPi = tempP + startPnt;
      int endPnt = startPnt + numPts;
      long* sortP1 = sortP + startPnt1;
      long* sortP2 = sortP + startPnt2;
      long* sortendP1 = sortP + startPnt + numPts1;
      long* sortendP2 = sortP + startPnt + numPts;
       for( i = startPnt ; i < endPnt ; ++i, ++tempPi )
         {
          if     ( sortP1 >= sortendP1 ) {*tempPi=*sortP2;++sortP2;}
          else if( sortP2 >= sortendP2 ) {*tempPi=*sortP1;++sortP1;}
          else
            {
             p1P = pointAddrP(dtmP,*sortP1) ;
             p2P = pointAddrP(dtmP,*sortP2) ;
             if( p1P->x < p2P->x || ( p1P->x == p2P->x && p1P->y <= p2P->y))
               {
                            *tempPi = *sortP1 ;
                                ++sortP1 ;
               }
             else
               {
                            *tempPi = *sortP2 ;
                             ++sortP2 ;
               }
            }
         }
    //   for( i = startPnt ; i < startPnt + numPts ; ++i )
    //     {
    //      if     ( startPnt1 >= startPnt + numPts1 ) {*(tempP+i)=*(sortP+startPnt2);++startPnt2;}
    //      else if( startPnt2 >= startPnt + numPts  ) {*(tempP+i)=*(sortP+startPnt1);++startPnt1;}
    //      else
    //        {
    //         p1P = pointAddrP(dtmP,*(sortP+startPnt1)) ;
    //         p2P = pointAddrP(dtmP,*(sortP+startPnt2)) ;
    //         if( p1P->X < p2P->X || ( p1P->X == p2P->X && p1P->Y <= p2P->Y))
    //           {
                         //   *(tempP+i) = *(sortP+startPnt1) ;
                                //++startPnt1 ;
                         //  }
    //         else
                         //  {
                         //   *(tempP+i) = *(sortP+startPnt2) ;
                         //    ++startPnt2 ;
    //           }
    //        }
    //     }
/*
**     Restore Sort Pointers
*/
       tempPi = tempP + startPnt;
       long* sortPi = sortP + startPnt;
       long* endsortP = sortPi + numPts;
       for(  ; sortPi < endsortP ; ++sortPi, ++tempPi ) *sortPi = *tempPi ;
       //for( i = startPnt ; i < startPnt + numPts ; ++i ) *(sortP+i) = *(tempP+i) ;
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
BENTLEYDTM_Public int bcdtmObject_removeDuplicatesDtmObject(BC_DTM_OBJ *dtmP,long *numDuplicatesP, bool duplicateOption)
/*
** This Function Removes Duplicate Points From The Dtm Object
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   n,ofs1,ofs2,ofs3,process,feature,removeFlag,numMarked=0 ;
 long   rollBackDuplicatePoints=FALSE,numDuplicatesRolledBack=0 ;
 long   nowTime=bcdtmClock(),startTime=bcdtmClock() ;
 double dp  ;
 DPoint3d    *dupPtsP=nullptr ;
 DTM_TIN_POINT  *p1P,*p2P  ;
 BC_DTM_FEATURE *dtmFeatureP=nullptr ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ; 
 LongArray temP;
 LongArray::iterator sP;
 LongArray::iterator sP2;
 LongArray::iterator temPEnd;
 DTM_ROLLBACK_DATA* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : nullptr;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicates Dtm Object %p",dtmP) ;
/*
** Initialise
*/
 *numDuplicatesP = 0 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test If Dtm Object Already Processed
*/
 if( dtmP->dtmState != DTMState::PointsSorted ) { bcdtmWrite_message(2,0,0,"Dtm Object Not In Sorted State")      ; goto errexit ; }
/*
** Allocate Memory For Temp Array To Mark Duplicate Points
*/
 if( temP.resize(dtmP->numPoints) != DTM_SUCCESS) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Null Out Temp Array To Mark Duplicate Points
*/
 temPEnd = temP.end();
 for( sP = temP.start() ; sP != temPEnd ; ++sP ) *sP = dtmP->nullPnt ;
/*
** Mark Points Within The Point To Point Tolerance
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Point To Point Tolerance") ;
 for( ofs1 = 0, sP = temP.start(); ofs1 < dtmP->numPoints - 1 ; ++ofs1, ++sP  )
   {
/*
**  Check For Check Stop Termination
*/
    if( (ofs1 &  0x3ff) == 0x3ff )
      {
       nowTime = bcdtmClock() ;
       if( ( double )( nowTime-startTime) / CLOCKS_PER_SEC > 0.025 )
         {
          if( bcdtmTin_checkForTriangulationTermination (dtmP)) goto errexit  ;
         }
       startTime = nowTime ;
      }
/*
**  Mark Points
*/
    if( *sP == dtmP->nullPnt )
      {
       p1P = pointAddrP(dtmP,ofs1) ;
       sP2 = sP;
       ofs2 = ofs1 + 1 ;
       process = 1 ;
       while ( ofs2 < dtmP->numPoints && process )
         {
          ++sP2;
          p2P = pointAddrP(dtmP,ofs2) ;
//          if( p2P->X - p1P->X > dtmP->ppTol ) process = 0 ; 
/*
**        RobC Modification To Handle the long columns with raster DTM's
**        Robc Modification Start 2-July-2009
*/
          if( p2P->x - p1P->x > dtmP->ppTol )  
            {
             process = 0 ;
             if( ofs2 - ofs1 > 2 && pointAddrP(dtmP,ofs2-1)->x == p1P->x )
               {
                process = 1 ;
                ofs3 = ofs1 ;
                while( ofs3 < ofs2 - 1 && process )
                  {
                   if( pointAddrP(dtmP,ofs3+1)->y - pointAddrP(dtmP,ofs3)->y < dtmP->ppTol ) process = 0 ;
                   else                                                                      ++ofs3  ;
                  }
                process = 0 ;
                if( ofs3 > ofs1 )
                  {
                   for( n = ofs1 + 1 ; n < ofs3 ; ++n ) ++sP ;
                   ofs1 = ofs3 - 1 ;
                  }
               }
            }
/*
**        Robc Modification End 2-July-2009
*/
          else
            {
             if( *sP2 == dtmP->nullPnt )
               {
                if( fabs(p1P->y-p2P->y) < dtmP->ppTol )
                  {
                   removeFlag = FALSE ;
                   dp = bcdtmMath_distance(p1P->x,p1P->y,p2P->x,p2P->y) ;
                   if (!duplicateOption && dp == 0.0) removeFlag = TRUE;
                   else if (duplicateOption  && dp <  dtmP->ppTol) removeFlag = TRUE;
                   if( removeFlag == TRUE )
                     {
                      *sP2 = ofs1;
                      ++*numDuplicatesP ;
                     }
                  }
               }
            }
          ++ofs2 ;
         }
      }
   }
if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",*numDuplicatesP) ;
/*
** Report Duplicate Points Being Removed
*/
 if( cdbg )
   {
    long dupCount=0 ;
    for( ofs1 = 0, sP = temP.start(); ofs1 < dtmP->numPoints  ; ++ofs1, ++sP  )
      {
       if( *sP != dtmP->nullPnt )
         {
          ++dupCount ;
          p1P = pointAddrP(dtmP,ofs1) ;
          bcdtmWrite_message(0,0,0,"Duplicate Point[%8ld] ** [%8ld] [%8ld] = %12.5lf %12.5lf %10.4lf",dupCount,ofs1,*sP,p1P->x,p1P->y,p1P->z) ;
         }
      }
    bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",dupCount) ;
   }
/*
** Reset Dtm Feature Point Offsets Of Merged Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reseting Point Offsets Of Merged Points ") ;
 for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
   {
    dtmFeatureP = ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
      {
       long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       bool rollBacked=FALSE ;
      for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          if( offsetP[n] < 0 || offsetP[n] > dtmP->numPoints )
            {
             bcdtmWrite_message(2,0,0,"Feature Point Index Range Error") ;
             goto errexit ;
            }
          if( *(temP+offsetP[n]) != dtmP->nullPnt )
            {
/*
**           Check For Duplicate Point Error
*/
             if( bcdtmMath_pointDistance3DDtmObject(dtmP,*(temP+offsetP[n]),offsetP[n]) > 0.0 )
               {
                if( rollBackInfo && rollBacked == false && rollBackInfo->rollBackDtmP != nullptr )
                  {
                   dupPtsP = ( DPoint3d * ) malloc ( dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d)) ;
                   if( dupPtsP == nullptr )
                     {
                      bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                      goto errexit ;
                     }
                   for( int m = 0 ; m < dtmFeatureP->numDtmFeaturePts ; ++m )
                     {
                      int offset=offsetP[m] ;
                      if( offset < 0 ) offset = -(offset+1) ;
                      *(dupPtsP+m) = *(DPoint3d *) pointAddrP(dtmP,offset) ;
                     }
                   if( dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull       && dtmFeatureP->dtmFeatureType != DTMFeatureType::HullLine  &&
                       dtmFeatureP->dtmFeatureType != DTMFeatureType::DrapeHull && dtmFeatureP->dtmFeatureType != DTMFeatureType::DrapeVoid &&
                       dtmFeatureP->dtmFeatureType != DTMFeatureType::GraphicBreak                                                      )
                     {
                     if (bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId)) goto errexit;
                     }
                   if( dupPtsP != nullptr ) { free(dupPtsP) ; dupPtsP = nullptr ; }
                   rollBacked = true ;
                  }
                int offset=offsetP[n] ;
                if( offset < 0 ) offset = -(offset+1) ;
                offsetP[n] = *(temP+offset) ;
                if( *(temP+offsetP[n]) != dtmP->nullPnt && *(temP+offsetP[n])  >= 0 ) *(temP+offsetP[n]) = -(*(temP+offsetP[n])+1) ;
               }
            }
         }
      }
   }
/*
** Copy Duplicate Points To Restore DTM For Roll Back Requirements
*/
 rollBackDuplicatePoints = FALSE ;
 if( rollBackInfo && rollBackInfo->rollBackDtmP != nullptr && rollBackDuplicatePoints == TRUE )
   {
/*
**  Backup Duplicate Points As Point Features With A Maximum  Of 1000 Points Per Point Feature
*/
    dupPtsP = ( DPoint3d *) malloc(1000*sizeof(DPoint3d)) ;
    if( dupPtsP == nullptr )
      {
       bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    numMarked = 0 ;
    for( sP = temP.start() ; sP != temP.end() ; ++sP ) 
      {
       if( *sP != dtmP->nullPnt )
         {
          if( *sP >= 0 )
            {
             if( bcdtmMath_pointDistance3DDtmObject(dtmP,*sP,(long)(sP-temP.start())) > 0.0 )
               {
                *(dupPtsP+numMarked) = *( DPoint3d *)pointAddrP(dtmP,(long)(sP-temP.start())) ;
                ++numMarked ;
                if( numMarked == 1000 )
                  {
                   numDuplicatesRolledBack = numDuplicatesRolledBack + numMarked ;
                   if( bcdtmObject_storeDtmFeatureInDtmObject(rollBackInfo->rollBackDtmP,DTMFeatureType::GroupSpots,-dtmP->nullUserTag,1,&nullFeatureId,dupPtsP,numMarked)) goto errexit ;
                   numMarked = 0 ;
                  }
               }
            }
         }
      }
    if( numMarked > 0 )
      {
       numDuplicatesRolledBack = numDuplicatesRolledBack + numMarked ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(rollBackInfo->rollBackDtmP,DTMFeatureType::GroupSpots,-dtmP->nullUserTag,1,&nullFeatureId,dupPtsP,numMarked)) goto errexit ;
       numMarked = 0 ;
      }
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numDuplicatesRolledBack = %8ld",numDuplicatesRolledBack) ;
   }
/*
**  Mark Merged Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Merged Points") ;
 for( sP = temP.start() ; sP != temPEnd; ++sP ) 
   {
    if( *sP == dtmP->nullPnt ) *sP = 0 ;
    else                       *sP = 1 ;
   }
/*
**  Remove Merged Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Merged Points") ;
 for( sP = temP.start(), ofs1 = ofs2 = 0 ; ofs2 < dtmP->numPoints ; ++ofs2, ++sP )
   {
    if( ! *sP )
      {
       if( ofs1 != ofs2 )
         {
          p1P = pointAddrP(dtmP,ofs1) ;
          p2P = pointAddrP(dtmP,ofs2) ;
          *p1P = *p2P ;
         }
       ++ofs1 ;
      }
   }
/*
**  Accumulate Delete Counts
*/
 for( sP = temP.start()+1  ; sP != temPEnd; ++sP ) *sP += *(sP-1) ;
/*
** Reset Dtm Feature Point Offsets
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resetting DTM Feature Point Offsets") ;
 for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
   {
    dtmFeatureP = ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
      {
      long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          offsetP[n] = offsetP[n] - *(temP+offsetP[n]) ;
         }
      }
   }
/*
**  Reset Dtm Object Variables
*/
 dtmP->dtmState  = DTMState::DuplicatesRemoved ;
 dtmP->numPoints = dtmP->numSortedPoints = dtmP->numPoints - *numDuplicatesP ;
/*
**  Resize DTM Object Arrays
*/
 if( bcdtmObject_resizeMemoryDtmObject(dtmP)) goto errexit ;
/*
** Write Number Of Duplicate Points
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Number of Duplicate Points  = %9ld",*numDuplicatesP) ;
    bcdtmWrite_message(0,0,0,"**** Number of Dtm Points        = %9ld",dtmP->numPoints) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Duplicates Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Duplicates Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_Private int bcdtmObject_removeDuplicatePointOffsetsDtmObject(BC_DTM_OBJ *dtmP)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,ofs1,ofs2,feature,partNum,partOfs,duplicatesFound=FALSE ;
 DPoint3d featurePnt ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_ROLLBACK_DATA* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : nullptr;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Point Offsets Dtm Object %p",dtmP) ;
/*
** Only Count If Features Are Present
*/
 if( dtmP->numFeatures > 0 )
   {
/*
** Scan Features Array
*/
    partNum = 0 ;
    partOfs = 0 ;
    dtmFeatureP = dtmP->fTablePP[partNum] ;
    for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
      {
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
         {
/*
**        Mark Duplicates In Feature Point Array
*/
          duplicatesFound = FALSE ;

          long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
          for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts - 1 ; ++n )
            {
             if( offsetP[n] >= 0 )
               {
                if( offsetP[n] == offsetP[n+1] )
                  {
                   offsetP[n+1] = - ( offsetP[n+1] +1 ) ;
                   duplicatesFound = TRUE ;
                  }
               }
            }
/*
**        Remove Duplicates In Feature Point Array
*/
          if( duplicatesFound == TRUE )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Duplicate Point Offsets Found In Feature %6ld",feature) ;
             for( ofs1 = ofs2 = 0 ; ofs2 < dtmFeatureP->numDtmFeaturePts ; ++ofs2 )
               {
                if( offsetP[ofs2] >= 0 )
                  {
                   if( ofs1 != ofs2 ) offsetP[ofs1] = offsetP[ofs2] ;
                   ++ofs1 ;
                  }
               }
/*
**           Reset Number Of Feature Point
*/
             dtmFeatureP->numDtmFeaturePts = ofs1  ;
             if( dtmFeatureP->numDtmFeaturePts <= 1 )
               {
                if( rollBackInfo && rollBackInfo->rollBackDtmP != nullptr )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeatureP->numDtmFeaturePts = %2ld",dtmFeatureP->numDtmFeaturePts) ;
                   if( dtmFeatureP->numDtmFeaturePts == 1 )
                     {
                      long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
                      featurePnt.x = pointAddrP(dtmP,*offsetP)->x ;
                      featurePnt.y = pointAddrP(dtmP,*offsetP)->y ;
                      featurePnt.z = pointAddrP(dtmP,*offsetP)->z ;
                     }
                   if (bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId)) goto errexit;
                  }
                bcdtmMemory_free(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ;
                dtmFeatureP->numDtmFeaturePts      = 0  ;
                dtmFeatureP->dtmFeaturePts.pointsPI = 0;
                dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;
               }
             else
               dtmFeatureP->dtmFeaturePts.offsetPI = bcdtmMemory_reallocate(dtmP, dtmFeatureP->dtmFeaturePts.offsetPI, dtmFeatureP->numDtmFeaturePts * sizeof(long)) ;
            }
         }
/*
**     Increment For Next Feature
*/
       ++partOfs ;
       if( partOfs == dtmP->featurePartitionSize )
         {
          partOfs = 0 ;
          ++partNum   ;
          dtmFeatureP = dtmP->fTablePP[partNum] ;
         }
       else  ++dtmFeatureP ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Duplicate Point Offsets Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Duplicate Point Offsets Error") ;
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
BENTLEYDTM_Private int bcdtmObject_countDtmTriangulationFeaturesDtmObject
(
BC_DTM_OBJ *dtmP,
long *numGraphicBreaksP,
long *numContourLinesP,
long *numSoftBreaksP,
long *numHardBreaksP,
long *numVoidsP,
long *numIslandsP,
long *numHolesP,
long *numBreakVoidsP,
long *numDrapeVoidsP,
long *numGroupSpotsP,
long *numRegionsP,
long *numHullsP,
long *numDrapeHullsP,
long *numHullLinesP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long feature ;
 char dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT *pntP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Counting Dtm Triangulation Features DTM Object %p",dtmP) ;
/*
** Initialise
*/
 *numGraphicBreaksP = 0 ;
 *numContourLinesP  = 0 ;
 *numSoftBreaksP    = 0 ;
 *numHardBreaksP    = 0 ;
 *numVoidsP         = 0 ;
 *numIslandsP       = 0 ;
 *numHolesP         = 0 ;
 *numDrapeVoidsP    = 0 ;
 *numBreakVoidsP    = 0 ;
 *numGroupSpotsP    = 0 ;
 *numRegionsP      = 0 ;
 *numHullsP         = 0 ;
 *numDrapeHullsP    = 0 ;
 *numHullLinesP     = 0 ;
/*
** Scan Dtm Features Array
*/
 for( feature = 0 ; feature < dtmP->numFeatures  ; ++feature )
   {
    dtmFeatureP =  ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError &&  dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted  && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
      {
       if( dbg == 2 )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          pntP = pointAddrP(dtmP,bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[0]) ;
          bcdtmWrite_message(0,0,0,"Feature Type %30s featureId = %10I64d ** %12.5lf %12.5lf %10.4lf",dtmFeatureTypeName,dtmFeatureP->dtmFeatureId,pntP->x,pntP->y,pntP->z) ;
         }
       if     ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline    ) ++*numHardBreaksP    ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::SoftBreakline    ) ++*numSoftBreaksP    ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::ContourLine  ) ++*numContourLinesP  ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void          ) ++*numVoidsP         ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island        ) ++*numIslandsP       ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GraphicBreak ) ++*numGraphicBreaksP ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull          ) ++*numHullsP         ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeHull    ) ++*numDrapeHullsP    ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine     ) ++*numHullLinesP     ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole          ) ++*numHolesP         ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid    ) ++*numDrapeVoidsP    ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid    ) ++*numBreakVoidsP    ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots    ) ++*numGroupSpotsP    ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Region        ) ++*numRegionsP      ;
      }
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Counting Dtm Triangulation Features For DTM Object %p Completed",dtmP) ;
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_triangulateLatticeObject(DTM_LAT_OBJ *latticeP, BC_DTM_OBJ **dtmPP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long i,j ;
 DPoint3d rasterPoint ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ; 
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Triangulating Lattice Object") ;
    bcdtmWrite_message(0,0,0,"latticeP  = %p",latticeP) ;
    bcdtmWrite_message(0,0,0,"dtmPP     = %p",*dtmPP) ;
   }
/*
**  Check For Valid Lattice Object
*/
 if( bcdtmObject_testForValidLatticeObject(latticeP)) goto errexit ;
/*
** Destroy Dtm Object If It Exists Dtm Object
*/
 if( *dtmPP != nullptr ) bcdtmObject_destroyDtmObject(dtmPP) ;
 *dtmPP = nullptr ;
/*
** Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(dtmPP)) goto errexit ;
/*
** Set Point Memory Allocation Parameters
*/
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(*dtmPP,latticeP->NOLATPTS,10000)) goto errexit ;
/*
** Store Lattice In Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Lattice Points In Dtm Object") ;
 for( j = 0 ; j < latticeP->NYL ; ++j )
   {
    rasterPoint.x = latticeP->LXMIN + latticeP->DX * j ;
    for( i = 0 ; i < latticeP->NXL ; ++i )
      {
       rasterPoint.y = latticeP->LYMIN + latticeP->DY * i ;
       rasterPoint.z = *(latticeP->LAT + i*latticeP->NYL + j ) ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(*dtmPP,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,&rasterPoint,1)) goto errexit ;
      }
   }
/*
** Triangulate DEM In DTM Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating DEM In DTM Object") ;
 if( bcdtmObject_triangulateDemDtmObject(*dtmPP,latticeP->NXL,latticeP->NYL,latticeP->NULLVAL )) goto errexit ;
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
 if( *dtmPP != nullptr ) bcdtmObject_destroyDtmObject(dtmPP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_triangulateDemDtmObject(BC_DTM_OBJ *dtmP,long numRows,long numColumns,double nullValue )
/*
** This Is The Controlling Routine For Triangulating A DEM Tin Object
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long point ;
 double dd,dx,dy ;
 DTM_TIN_POINT *p1P,*p2P ;
/*
** Calculate Machine Precision
*/
 bcdtmMath_calculateMachinePrecisionForDtmObject(dtmP) ;
/*
** Caculate Point To Point Tolerance
*/
 point = 1 ;
 p1P = pointAddrP(dtmP,0) ;
 p2P = pointAddrP(dtmP,point) ;
 dy  = p2P->y - p1P->y ;
 while( p2P->x == p1P->x && point < dtmP->numPoints )
   {
    p1P = p2P ;
    ++point  ;
    p2P = pointAddrP(dtmP,point) ;
   }
 dx = p2P->x - p1P->x ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dx = %15.12lf ** dy = %12.15lf",dx,dy) ;
 if( dx <= dy ) dd = dx ;
 else           dd = dy ;
 dtmP->ppTol = dtmP->plTol = dd / 1000.0 ;
 if( dtmP->ppTol > 0.0001 ) dtmP->ppTol = dtmP->plTol = 0.0001 ;
 if( dtmP->ppTol / 1000.0 < dtmP->mppTol ) dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 1000.0 ;
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"mppTol = %20.15lf ppTol = %20.15lf plTol = %20.15lf",dtmP->mppTol,dtmP->ppTol,dtmP->plTol) ;
/*
** Check Memory
*/
 if( dtmP->nodesPP == nullptr ) if( bcdtmObject_allocateNodesMemoryDtmObject(dtmP)) goto errexit ;
 if( dtmP->cListPP == nullptr ) if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit ;
/*
** Set Tin State
*/
 dtmP->dtmState = DTMState::Tin ;
/*
** Triangulate Data Set
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating %8ld Point DEM",numRows*numColumns) ;
 if( bcdtmObject_triangulateGridDtmObject(dtmP,numRows,numColumns,nullValue) ) goto errexit ;
/*
** Compact Tin Structure
*/
 if( bcdtmTin_compactCircularListDtmObject(dtmP)) goto errexit ;
 if( bcdtmTin_compactFeatureTableDtmObject(dtmP)) goto errexit ;
 if( bcdtmTin_compactFeatureListDtmObject(dtmP)) goto errexit ;
 if( bcdtmTin_compactPointAndNodeTablesDtmObject(dtmP)) goto errexit ;
/*
** Resize Tin Memory
*/
 if( bcdtmObject_resizeMemoryDtmObject(dtmP) ) goto errexit ;
/*
** Set Number Of Tin Points For Binary Searching
*/
 dtmP->numSortedPoints = dtmP->numPoints ;
/*
** Set Number Of Triangles and Lines
*/
 dtmP->numTriangles = ( numRows - 1 ) * ( numColumns - 1 ) * 2 ;
 dtmP->numLines     = ( numColumns - 1 ) * numRows + ( numRows - 1 ) * numColumns + ( numRows - 1 ) * ( numColumns - 1 ) ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Triangulation Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
   }
/*
** Write Tin To File
*/
 if( dbg == 2 ) if( bcdtmWrite_toFileDtmObject(dtmP,L"rasterDem.tin")) goto errexit ;
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
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmObject_triangulateGridDtmObject(BC_DTM_OBJ *dtmP,long numRows,long numColumns,double nullValue )
/*
** This Function Triangulates A DEM Dtm Object
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
 long i,j,p1,p2,p3,p4 ;
 long nullFlag,point ;
 DTM_TIN_POINT  *pointP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Grid ** numRows = %6ld numCols = %6ld nullValue = %10.4lf",numRows,numColumns,nullValue) ;
/*
** Join Rows
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Joining Rows") ;
 for( i = 0 ; i < numRows ; ++i )
   {
    p1 = i ;
    for ( j = 0 ; j < numColumns - 1 ; ++j )
      {
       p2 = p1 + numRows ;
       p3 = p1 - numRows ;
       if( j == 0 ) p3 = dtmP->nullPnt ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,p2,p3)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p1,dtmP->nullPnt)) goto errexit ;
       p1 = p2 ;
      }
   }
/*
** Join Columns
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Joining Columns") ;
 for( j = 0 ; j < numColumns - 1 ; ++j )
   {
    p1 = j * numRows ;
    for ( i = 0 ; i < numRows - 1 ; ++i )
      {
       p2 = p1 + 1 ;
       p3 = p2 + numRows ;
       p4 = p1 + numRows ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p1,p2,p4)) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p1,p3))  goto errexit ;
       if( j == numColumns - 2 )
         {
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p3,p1))  goto errexit ;
          if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p3,p4,p2)) goto errexit ;
         }
       p1 = p2 ;
      }
   }
/*
** Join Diagonals
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Joining Diagonals") ;
 for( j = 0 ; j < numColumns - 1 ; ++j )
   {
    p1 = j * numRows ;
    for ( i = 0 ; i < numRows - 1 ; ++i )
      {
       p2 = p1 + 1 ;
       p3 = p2 + numRows ;
       p4 = p1 + numRows ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p4,p3))  goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p2,p1))  goto errexit ;
       p1 = p2 ;
      }
   }
/*
** Set Convex Hull
*/
 dtmP->hullPoint = 0 ;
 dtmP->nextHullPoint = numRows ;
 bcdtmTin_setConvexHullDtmObject(dtmP,dtmP->hullPoint,dtmP->nextHullPoint) ;
/*
** Resort Tin Points
*/
// if( bcdtmTin_resortTinStructureDtmObject(dtmP) ) goto errexit ;
/*
** Set Number Of Tin Points For Binary Searching
*/
 dtmP->numSortedPoints = dtmP->numPoints ;
/*
** Set Number Of Triangles and Lines
*/
 dtmP->numTriangles = ( numRows - 1 ) * ( numColumns - 1 ) * 2 ;
 dtmP->numLines     = ( numColumns - 1 ) * numRows + ( numRows - 1 ) * numColumns + ( numRows - 1 ) * ( numColumns - 1 ) ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Triangulation Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
   }
/*
** Search For Null Values
*/
 nullFlag = FALSE ;
 for( point = 0 ; point < dtmP->numPoints && nullFlag == FALSE ; ++point )
   {
    pointP = pointAddrP(dtmP,point) ;
    if( pointP->z == nullValue ) nullFlag = TRUE ;
   }
/*
** Create Voids Around Null Values
*/
 if( nullFlag == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Placing Voids Around Null Values") ;
    if( bcdtmObject_placeVoidsAroundNullValuesDtmObject(dtmP,nullValue)) goto errexit ;
   }
/*
** Re Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Triangulation Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
   }
/*
** Write Tin To File
*/
 if( cdbg ) if( bcdtmWrite_toFileDtmObject(dtmP,L"raster.tin")) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmObject_placeVoidsAroundNullValuesDtmObject(BC_DTM_OBJ *dtmP,double nullValue)
/*
** This Function Places Voids Around Null Values
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
 long p1, p2, p3, clPtr, offset, sizeVoidLine = 0;
 DTMFeatureType dtmFeatureType;
 long offset1,offset2,voidHull,numNullPoints ;
 DTMDirection direction;
 long startPnt,nextPnt,endPnt,scanPnt,numVoids=0,numIslands=0 ;
 unsigned char *charP,*voidLineP=nullptr ;
 double area ;
 DTM_TIN_POINT *pointP ;
 DTM_CIR_LIST  *clistP ;
 DTM_TIN_NODE  *nodeP  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Placing Voids Around Null Values") ;
/*
** Count Number Of Null Points
*/
 if( dbg )
   {
    numNullPoints = 0 ;
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       pointP = pointAddrP(dtmP,p1) ;
       if( pointP->z == nullValue ) ++numNullPoints ;
      }
    bcdtmWrite_message(0,0,0,"numNullPoints = %8ld",numNullPoints) ;
   }
/*
** Allocate Memory
*/
 sizeVoidLine = dtmP->cListPtr / 8 + 1 ;
 voidLineP = ( unsigned char * ) malloc ( sizeVoidLine * sizeof(char)) ;
 if( voidLineP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( charP = voidLineP ; charP < voidLineP + sizeVoidLine ; ++charP ) *charP = 0 ;
/*
** Mark All Void Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Lines") ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    pointP = pointAddrP(dtmP,p1) ;
    if( pointP->z == nullValue )
      {
       clPtr = nodeAddrP(dtmP,p1)->cPtr ;
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while( clPtr != dtmP->nullPtr )
         {
          clistP = clistAddrP(dtmP,clPtr) ;
          p3     = clistP->pntNum ;
          clPtr  = clistP->nextPtr ;
          if( nodeAddrP(dtmP,p3)->hPtr != p1 )
            {
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p1,p2) ) goto errexit ;
             bcdtmFlag_setFlag(voidLineP,offset) ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p2,p3) ) goto errexit ;
             bcdtmFlag_setFlag(voidLineP,offset) ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p3,p1) ) goto errexit ;
             bcdtmFlag_setFlag(voidLineP,offset) ;
             nodeAddrP(dtmP,p1)->sPtr = dtmP->nullPnt ;
             nodeAddrP(dtmP,p2)->sPtr = 1 ;
             nodeAddrP(dtmP,p3)->sPtr = 1 ;
            }
          p2 = p3 ;
         }
      }
   }
/*
** Polygonise Voids On the Tin Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Voids On Tin Hull") ;
 startPnt = dtmP->hullPoint ;
 do
   {
    if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt )
      {
       nextPnt = nodeAddrP(dtmP,startPnt)->hPtr ;
       bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset1,nextPnt,startPnt) ;
       bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset2,startPnt,nextPnt) ;
       if(  bcdtmFlag_testFlag(voidLineP,offset1) != bcdtmFlag_testFlag(voidLineP,offset2) )
         {
          endPnt = startPnt ;
/*
**        Scan Until Back To First Point
*/
          do
            {
             if( ( scanPnt = bcdtmList_nextClkDtmObject(dtmP,nextPnt,startPnt)) < 0 ) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset1,scanPnt,nextPnt) )  goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset2,nextPnt,scanPnt) )  goto errexit ;
             while( bcdtmFlag_testFlag(voidLineP,offset1) == bcdtmFlag_testFlag(voidLineP,offset2) )
               {
                if( ( scanPnt = bcdtmList_nextClkDtmObject(dtmP,nextPnt,scanPnt)) < 0 ) goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,scanPnt,nextPnt) )  goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset1,scanPnt,nextPnt) )  goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset2,nextPnt,scanPnt) )  goto errexit ;
               }
             nodeAddrP(dtmP,startPnt)->tPtr = nextPnt ;
             startPnt = nextPnt ;
             nextPnt  = scanPnt ;
            } while( startPnt != endPnt ) ;
/*
**        Check Connectivity Of Tptr Polygon
*/
          if( cdbg )
            {
             if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0))
               {
                bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
                goto errexit ;
               }
            }
/*
**        Calculate Area Of Void
*/
          if( cdbg )
            {
             if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction)) goto errexit ;
             bcdtmWrite_message(0,0,0,"Area = %12.5lf Direction = %2ld",area,direction) ;
           }
/*
**        Add Void To Dtm
*/
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,nullptr,0,DTMFeatureType::Void,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,startPnt,0)) goto errexit ;
          ++numVoids ;
         }
      }
    startPnt = nodeAddrP(dtmP,startPnt)->hPtr  ;
   } while( startPnt != dtmP->hullPoint ) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Voids On Tin Hull Completed") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Voids = %8ld",numVoids) ;
/*
** Polygonise Internal Voids
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Internal Voids") ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    pointP = pointAddrP(dtmP,p1) ;
    nodeP  = nodeAddrP(dtmP,p1)  ;
    if( pointP->z != nullValue && nodeP->tPtr == dtmP->nullPnt && nodeP->sPtr == 1 )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Polygonising From Point = %8ld",p1) ;
       clPtr = nodeP->cPtr ;
       while ( clPtr != dtmP->nullPtr )
         {
          clistP = clistAddrP(dtmP,clPtr) ;
          p2     = clistP->pntNum ;
          clPtr  = clistP->nextPtr ;
/*
**        Check For Void Hull
*/
          if( nodeAddrP(dtmP,p2)->tPtr == dtmP->nullPnt )
            {
             voidHull = 0 ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset1,p1,p2) ) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset2,p2,p1) ) goto errexit ;
             if( bcdtmFlag_testFlag(voidLineP,offset1) != bcdtmFlag_testFlag(voidLineP,offset2)) voidHull = 1 ;
/*
**           Void Hull Found
*/
             if( voidHull )
               {
                if( bcdtmFlag_testFlag(voidLineP,offset1)) { startPnt = p2 ; nextPnt = p1 ; }
                if( bcdtmFlag_testFlag(voidLineP,offset2)) { startPnt = p1 ; nextPnt = p2 ; }
/*
**              Trace Around Void Area
*/
                endPnt = startPnt ;
                do
                  {
                   if( ( scanPnt = bcdtmList_nextClkDtmObject(dtmP,nextPnt,startPnt)) < 0 ) goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset1,scanPnt,nextPnt) )   goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset2,nextPnt,scanPnt) )   goto errexit ;
                   while( bcdtmFlag_testFlag(voidLineP,offset1) == bcdtmFlag_testFlag(voidLineP,offset2) )
                     {
                      if( ( scanPnt = bcdtmList_nextClkDtmObject(dtmP,nextPnt,scanPnt)) < 0 ) goto errexit ;
                      if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset1,scanPnt,nextPnt) )  goto errexit ;
                      if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset2,nextPnt,scanPnt) )  goto errexit ;
                     }
                   nodeAddrP(dtmP,startPnt)->tPtr = nextPnt ;
                   startPnt = nextPnt ;
                   nextPnt  = scanPnt ;
                  } while( startPnt != endPnt ) ;
/*
**              Check Connectivity Of Tptr Polygon
*/
                if( cdbg )
                  {
                   if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0))
                     {
                      bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
                      goto errexit ;
                     }
                  }
/*
**              Calculate Area Of Void
*/
                if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction)) goto errexit ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Area = %12.5lf Direction = %2ld",area,direction) ;
                dtmFeatureType = DTMFeatureType::Void ;
                if (direction == DTMDirection::Clockwise)
                  {
                   dtmFeatureType = DTMFeatureType::Island ;
                   if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
                  }
/*
**              Add Void/Island To Dtm
*/
                if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,nullptr,0,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,startPnt,0)) goto errexit ;
                if( dtmFeatureType == DTMFeatureType::Void ) ++numVoids ;
                if( dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
               }
            }
         }
      }
   }
/*
** Set Void Bits And z Range
*/
 if( numVoids > 0 )
   {
    p2 = 1 ;
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       pointP = pointAddrP(dtmP,p1) ;
       if( pointP->z == nullValue && nodeAddrP(dtmP,p1)->hPtr == dtmP->nullPnt ) bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD) ; 
       else if( pointP->z != nullValue )
         {
          if( p2 ) { dtmP->zMin = dtmP->zMax = pointP->z ; p2 = 0 ; }
          else
            {
             if( pointP->z < dtmP->zMin ) dtmP->zMin = pointP->z ;
             if( pointP->z > dtmP->zMax ) dtmP->zMax = pointP->z ;
            }
         }
      }
    dtmP->zRange = dtmP->zMax - dtmP->zMin ;
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       pointP = pointAddrP(dtmP,p1) ;
       if( pointP->z == nullValue ) pointP->z = dtmP->zMin ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Voids = %8ld ** Number Of Islands = %8ld",numVoids,numIslands) ;
/*
** Clean Up
*/
 cleanup :
 if( voidLineP != nullptr ) { free(voidLineP) ; voidLineP = nullptr ; }
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Placing Voids Around Null Values Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Placing Voids Around Null Values Error") ;
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
BENTLEYDTM_EXPORT int bcdtmObject_slopeModifyDemTriangulationDtmObject(BC_DTM_OBJ *dtmP,long slopeModifyOption)
/*
**
** This Function Modifies A DEM Triangulation For Minimum Or Maximum Slope
** slopeModifyOption = 1 minimise slope
**                   = 2 maximise slope
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
 long   p1,p2,p3,p4,clPtr ;
 long   swap, demDiagonal;
 bool voidsInDtm, voidLine;
 double descentAngle,ascentAngle,slope1,slope2,slope3,slope4 ;
 double angle,angleDelta,angle45,angle135,angle225,angle315 ;
 DTM_CIR_LIST *clistP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Slope Modifying DEM DTM") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"slopeModifyOption = %8ld",slopeModifyOption) ;
   }
/*
** Set Diagonal Angles
*/
 angle45  = DTM_PYE / 4.0 ;
 angle135 = angle45 + DTM_PYE / 2.0 ;
 angle225 = angle135 + DTM_PYE / 2.0 ;
 angle315 = angle225 + DTM_PYE / 2.0 ;
 angleDelta = 5.0 / 360.0 * DTM_2PYE ;
/*
** Check For Voids In DTM
*/
 if( bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm)) goto errexit ;
/*
**  Scan Tin Lines And Flip For Slope Criteria
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1)
   {
    if( ( clPtr = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr )
      {
       while( clPtr != dtmP->nullPtr )
         {
          clistP = clistAddrP(dtmP,clPtr) ;
          p2 = clistP->pntNum ;
          clPtr = clistP->nextPtr ;
/*
**        Check For Hull Line
*/
          if( nodeAddrP(dtmP,p1)->hPtr != p2 && nodeAddrP(dtmP,p2)->hPtr != p1 )
            {
/*
**           Check For None Feature Line
*/
             if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p1,p2) )
               {
/*
**              Check For Void Line
*/
                voidLine = FALSE ;
                if( voidsInDtm )if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,voidLine)) goto errexit ;
                if( ! voidLine )
                  {
/*
**                 Check For Dem Diagonal
*/
                   demDiagonal = FALSE ;
                   angle = bcdtmMath_getPointAngleDtmObject(dtmP,p1,p2) ;
                   if     ( fabs(angle-angle45)  <= angleDelta ) demDiagonal = TRUE ;
                   else if( fabs(angle-angle135) <= angleDelta ) demDiagonal = TRUE ;
                   else if( fabs(angle-angle225) <= angleDelta ) demDiagonal = TRUE ;
                   else if( fabs(angle-angle315) <= angleDelta ) demDiagonal = TRUE ;
                   if( dbg == 2 && demDiagonal == TRUE ) bcdtmWrite_message(0,0,0,"angle = %8.4lf ** p1 = %8ld p2 = %8ld",angle*360.0/DTM_2PYE,p1,p2) ;
/*
**                 DEM Diagonal Found
*/
                   if( demDiagonal )
                     {
                      if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                      if( ( p4 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
                      if( ! bcdtmList_testLineDtmObject(dtmP,p2,p3)) p3 = dtmP->nullPnt ;
                      if( ! bcdtmList_testLineDtmObject(dtmP,p2,p4)) p4 = dtmP->nullPnt ;
                      if( p3 != dtmP->nullPnt && p4 != dtmP->nullPnt )
                        {
                         if( bcdtmMath_getTriangleDescentAndAscentAnglesDtmObject(dtmP,p1,p3,p2,&descentAngle,&ascentAngle,&slope1)) goto errexit ;
                         if( bcdtmMath_getTriangleDescentAndAscentAnglesDtmObject(dtmP,p1,p2,p4,&descentAngle,&ascentAngle,&slope2)) goto errexit ;
                         if( bcdtmMath_getTriangleDescentAndAscentAnglesDtmObject(dtmP,p1,p3,p4,&descentAngle,&ascentAngle,&slope3)) goto errexit ;
                         if( bcdtmMath_getTriangleDescentAndAscentAnglesDtmObject(dtmP,p2,p4,p3,&descentAngle,&ascentAngle,&slope4)) goto errexit ;
                         swap = FALSE ;
                         if( slopeModifyOption == 1 && fabs(slope1-slope2) > fabs(slope3-slope4)) swap = TRUE ;
                         if( slopeModifyOption == 2 && fabs(slope1-slope2) < fabs(slope3-slope4)) swap = TRUE ;
                         if( swap == TRUE )
                           {
                            clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                            if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p3,p4,p2)) goto errexit ;
                            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p3,p1)) goto errexit ;
                            if( cdbg == 2 )
                              {
                               if( bcdtmCheck_tinComponentDtmObject(dtmP))
                                 {
                                  bcdtmWrite_message(1,0,0,"Tin Corrupted After Modification") ;
                                  goto errexit ;
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(1,0,0,"Tin Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(1,0,0,"Tin Valid") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Slope Modifying DEM DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Slope Modifying DEM DTM Error") ;
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
BENTLEYDTM_Public int bcdtmObject_createTinDtmObjectOverload
(
 BC_DTM_OBJ  *dtmP,
 long        edgeOption,
 double      maxSide,
 bool        normaliseOption,
 bool        mergeOption
)
/*
** This A Special Purpose Triangulation Function For Core DTM Purposes Only
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 char dtmFeatureTypeName[50] ;
 long trgTime,createTime,dtmFeature,numTinFeatures,numErrorTinFeatures,numDeletedFeatures ;
 long numGraphicBreaks,numContourLines,numSoftBreaks,numHardBreaks,numVoids,numIslands,numHoles ;
 long numBreakVoids,numDrapeVoids,numGroupSpots,numRegions,numHulls,numDrapeHulls,numHullLines ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT  *pntP ;
/*
** Write Entry Message
*/
 if( dbg == 1 ) 
   {
    bcdtmWrite_message(0,0,0,"Creating Tin For Dtm Object %p ** dtmState = %2ld dtmP->numPoints = %8ld",dtmP,dtmP->dtmState,dtmP->numPoints) ;
   }
  createTime = bcdtmClock() ; 
/*
** Procees DTM Object For Triangulation
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Processing DTM For Tin Creation") ;
 if( bcdtmObject_processForTriangulationDtmObject(dtmP)) goto errexit ;

/*
** Log DTM Object
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"DTM Stats After Pre Triangulation Processing") ;
    bcdtmObject_reportStatisticsDtmObject(dtmP) ;
    long numFeaturePts=0 ;
    DPoint3d *p3dP,*featurePtsP=nullptr ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       bcdtmWrite_message(0,0,0,"DtmFeatureType = %4ld **Number Of Feature Points = %8ld",dtmFeatureP->dtmFeatureType,dtmFeatureP->numDtmFeaturePts) ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
         {
          bcdtmWrite_message(0,0,0,"Number Of Feature Points = %8ld",dtmFeatureP->numDtmFeaturePts) ;
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          bcdtmWrite_message(0,0,0,"== Number Of Feature Points = %8ld",numFeaturePts) ;
          for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
            {
             bcdtmWrite_message(0,0,0,"Feature Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
            }
         }
      }
    if( featurePtsP != nullptr ) free(featurePtsP) ;  
   }
/*
** Check If DTM Is Triangulated
*/
 if( dtmP->dtmState == DTMState::Tin ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"DTM Is Triangulated") ;
    goto cleanup ;
   }
/*
** Count Number Of Dtm Triangulation Features
*/
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Counting Number Of Dtm Triangulation Features") ;
 bcdtmObject_countDtmTriangulationFeaturesDtmObject(dtmP,&numGraphicBreaks,&numContourLines,&numSoftBreaks,&numHardBreaks,&numVoids,&numIslands,&numHoles,&numBreakVoids,&numDrapeVoids,&numGroupSpots,&numRegions,&numHulls,&numDrapeHulls,&numHullLines) ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points           = %6ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Graphic Breaks   = %6ld",numGraphicBreaks) ;
    bcdtmWrite_message(0,0,0,"Number Of Contour Lines    = %6ld",numContourLines) ;
    bcdtmWrite_message(0,0,0,"Number Of Hard Breaks      = %6ld",numHardBreaks) ;
    bcdtmWrite_message(0,0,0,"Number Of Voids            = %6ld",numVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Island           = %6ld",numIslands) ;
    bcdtmWrite_message(0,0,0,"Number Of Holes            = %6ld",numHoles) ;
    bcdtmWrite_message(0,0,0,"Number Of Break Voids      = %6ld",numBreakVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Drape Voids      = %6ld",numDrapeVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Group Spots      = %6ld",numGroupSpots) ;
    bcdtmWrite_message(0,0,0,"Number Of Regions          = %6ld",numRegions) ;
    bcdtmWrite_message(0,0,0,"Number Of Hulls            = %6ld",numHulls) ;
    bcdtmWrite_message(0,0,0,"Number Of Drape Hulls      = %6ld",numDrapeHulls) ;
    bcdtmWrite_message(0,0,0,"Number Of Hull Lines       = %6ld",numHullLines) ;
   }
/*
** Check There Is Only One Hull Feature
*/
 if( numHulls + numDrapeHulls > 1 )
   {
    bcdtmWrite_message(1,0,0,"More Than One Hull Feature") ;
    goto errexit ;
   }
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Triangulate DTM Object
*/
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Triangulating Dtm Object") ;
 trgTime = bcdtmClock() ;
 if( bcdtmTin_createTinDtmObject(dtmP,edgeOption,maxSide,numGraphicBreaks,numContourLines,numSoftBreaks,numHardBreaks,numVoids,numIslands,numHoles,numBreakVoids,numDrapeVoids,numGroupSpots,numRegions,numHulls,numDrapeHulls,numHullLines) ) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Triangulate Dtm Object  = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),trgTime)) ;
/*
** Write Tin Stats
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Tin Points               = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"Number Tin Lines                = %8ld",dtmP->numLines) ;
    bcdtmWrite_message(0,0,0,"Number Tin Triangles            = %8ld",dtmP->numTriangles) ;
    bcdtmWrite_message(0,0,0,"Number Dtm Features             = %8ld",dtmP->numFeatures) ;
    numTinFeatures = 0 ;
    numErrorTinFeatures = 0 ;
    numDeletedFeatures = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if     ( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin ) ++numTinFeatures ;
       else if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError ) ++numErrorTinFeatures ;
       else if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Deleted   ) ++numDeletedFeatures ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          pntP = pointAddrP(dtmP,(long)dtmFeatureP->dtmFeaturePts.firstPoint) ;
          bcdtmWrite_message(0,0,0,"Tin Feature %30s ** featureId = %10I64d ** firstPoint = %12.5lf %12.5lf %10.4lf",dtmFeatureTypeName,dtmFeatureP->dtmFeatureId,pntP->x,pntP->y,pntP->z) ;
         }
      } 
    bcdtmWrite_message(0,0,0,"Number Tin Features             = %8ld",numTinFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Tin Features With Errors = %8ld",numErrorTinFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Deleted Features         = %8ld",numDeletedFeatures) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Create Tin              = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),createTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Tin For Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Tin For Dtm Object %p Error",dtmP) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dtmP->dtmState == DTMState::DuplicatesRemoved || dtmP->dtmState == DTMState::PointsSorted || dtmP->dtmState == DTMState::TinError) 
     bcdtmObject_changeStateDtmObject (dtmP, DTMState::Data);
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_processForTriangulationDtmObjectOverload
(
 BC_DTM_OBJ *dtmP,
 bool       normaliseOption,
 bool       duplicateOption
)
/*
** This Function Prepares A Dtm Object For Triangulation
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long n,point,dtmFeature,startTime,firstPoint,numDuplicates,numHullpts=0;
 DPoint3d  *hullPtsP=nullptr ;
 BC_DTM_FEATURE *dtmFeatureP=nullptr ;
 DTM_ROLLBACK_DATA* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : nullptr;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Processing Dtm Object %p For Triangulation",dtmP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Valid Prior Triangulation States
*/
 if( dtmP->dtmState != DTMState::Data && dtmP->dtmState != DTMState::Tin ) 
   {
    bcdtmWrite_message(0,0,0,"DTM Not In A Data or Tin State") ;
    goto errexit ;
   }
/*
** Check If Triangulation Already Exists
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Previously Triangulated") ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray || dtmFeatureP->dtmFeatureState == DTMFeatureState::Deleted)
         {
          startTime  = bcdtmClock() ;
          if ( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
          if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Change Dtm State        = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
          dtmFeature = dtmP->numFeatures ;   
         }
      }
/*
**   Check For Inserted Points From Removed Features
*/
    if( dtmP->dtmState == DTMState::Tin )
      {
       for( point = 0 ; point < dtmP->numPoints ; ++point )
         {
          if( bcdtmFlag_testInsertPoint(dtmP,point))
            {
             if ( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
             point = dtmP->numPoints ;
            } 
         }      
      }  
/*
**  If DTM Not In Data State No Need To Retriangulate
    if( dtmP->dtmState == DTMState::Tin ) 
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"No Need To Retriangulate") ;
       goto cleanup ;
      } 
*/
   }
/*
** If Roll Back Option Set Copy Selected DTM Features To Temporary Roll Back DTM
*/
 if(  rollBackInfo && rollBackInfo->rollBackDtmP != nullptr )
   {
    if ( bcdtmObject_testApiCleanUpDtmObject (dtmP, DTMCleanupFlags::Changes))
        {
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
          {
           dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
           if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
             {
              bool rollBack=false ; 
              if     ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull          ) rollBack = true ;
              else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeHull    ) rollBack = true ;
              else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine     ) rollBack = true ;
              else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid    ) rollBack = true ;
              else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid    ) rollBack = true ;
              else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::VoidLine     )  rollBack = true ;
              if( rollBack )
                {
                 if (bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId)) goto errexit;
                } 
             }  
          } 
        }
    if ( bcdtmCleanUp_cleanDtmObject (dtmP)) goto errexit;
   }
/*
** Remove Deleted Features
*/
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Removing Deleted Features") ;
 if( bcdtmData_compactUntriangulatedFeatureTableDtmObject(dtmP)) goto errexit ;
 if (rollBackInfo) rollBackInfo->rollBackMapInitialized = false;
/*
** Check For Less Than Three Points
 if( dtmP->numPoints < 3 )
   {
    bcdtmWrite_message(1,0,0,"Less Than Three Points In DTM") ;
    goto errexit ;
   }
/*
** Check Dtm State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(2,0,0,"DTM In Wrong State For Prior Triangulation Processing") ;
    goto errexit ;
   }
/*
** Normalise Dtm Points
*/
 if( normaliseOption == true )                             
   {
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Normalising Dtm Points") ;
    if( bcdtmMath_normaliseCoordinatesDtmObject(dtmP)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Normalise Dtm Points    = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
/*
** Calculate Machine Precision
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Machine Precision") ;
 bcdtmMath_calculateMachinePrecisionForDtmObject(dtmP) ;
/*
** Check Tolerances
*/
 if( dtmP->ppTol < dtmP->mppTol * 10000.0 ) 
   {
    dtmP->ppTol = dtmP->mppTol * 10000.0 ;
    dtmP->ppTol = dtmP->ppTol + dtmP->ppTol / 10.0 ;
    dtmP->plTol = dtmP->ppTol ;
   } 
/*
** Set Bounding Cube For Dtm Object
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Bounding Cube For Dtm Object") ;
 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Set Bounding Cube       = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Add Point Offsets To Feature Array Entries
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Point Offsets To Feature Array Entries") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
      {
/*
**     Allocate Memory For Point Offsets
       if( dbg == 2  ) bcdtmWrite_message(0,0,0,"dtmFeature[%5ld] ** type = %4ld firstPoint = %8ld numPoints = %8ld",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeaturePts.firstPoint,dtmFeatureP->numDtmFeaturePts) ;
       firstPoint = (long) dtmFeatureP->dtmFeaturePts.firstPoint ;      
       dtmFeatureP->dtmFeaturePts.offsetPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts * sizeof(long)) ;
       if( dtmFeatureP->dtmFeaturePts.offsetPI == 0 )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
/*
**     Set Dtm Feature State
*/
       dtmFeatureP->dtmFeatureState = DTMFeatureState::OffsetsArray ; 
/*
**     Populate Point Offsets
*/
       long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       firstPoint = (long) dtmFeatureP->dtmFeaturePts.firstPoint ;

       for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          offsetP[n] = firstPoint ;
          ++firstPoint ;
         }
/*
**     Write Feature Stats
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmFeature = %6ld type = %4ld offsetP = %p numDtmFeaturePts = %6ld",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeaturePts.offsetPI,dtmFeatureP->numDtmFeaturePts) ;
      }
   }
/*
** Check Point Offsets Before Sorting
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Point Offsets For Range Errors Before Sorting") ; 
    if( bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Checking For Duplicate Index Offset Memory Points") ; 
    if( bcdtmCheck_forDuplicatePointOffsetPointersDtmObject(dtmP)) goto errexit ;
   }
/*
** Sort Dtm Object
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting DTM Object") ;
 if( bcdtmObject_sortDtmObject(dtmP)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Sort Dtm Points         = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check For Check Stop Termination
*/
 if( bcdtmTin_checkForTriangulationTermination(dtmP)) goto errexit  ;
/*
** Check Point Offsets After Sorting
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Check Point Offsets For Range Errors After Sorting") ; 
    if( bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Checking For Duplicate Index Offset Memory Points") ; 
    if( bcdtmCheck_forDuplicatePointOffsetPointersDtmObject(dtmP)) goto errexit ;
   }
/*
** Check Sort Order
*/
 if( cdbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking Sort Order After Sorting Points") ;
    if( bcdtmCheck_sortOrderDtmObject(dtmP,0) != DTM_SUCCESS )
      {
       bcdtmWrite_message(1,0,0,"Dtm Sort Order Error") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Sort Order Valid After Sorting Points") ;
   } 
/*
** Remove Duplicates
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicates DTM Object") ;
 if( bcdtmObject_removeDuplicatesDtmObjectOverload(dtmP,&numDuplicates,duplicateOption)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates Removed = %8ld",numDuplicates) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Remove Duplicate Points = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check Point Offsets After Duplicate Removal
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Check Point Offsets For Range Errors After After Duplicate Removal") ; 
    if( bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Checking For Duplicate Index Offset Memory Points") ; 
    if( bcdtmCheck_forDuplicatePointOffsetPointersDtmObject(dtmP)) goto errexit ;
   }
/*
** Remove Duplicates Point Offsets
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicates Point Offsets DTM Object") ;
 if( bcdtmObject_removeDuplicatePointOffsetsDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints = %8ld",dtmP->numSortedPoints) ;
/*
** Check Point Offsets After Duplicate Points Offsets Removal
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Check Point Offsets For Range Errors After After Duplicate Point Offsets Removal") ; 
    if( bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(dtmP)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Checking For Duplicate Index Offset Memory Points") ; 
    if( bcdtmCheck_forDuplicatePointOffsetPointersDtmObject(dtmP)) goto errexit ;
   }
/*
** Check For Less Than Three Points
 if( dtmP->numPoints < 3 )
   {
    bcdtmWrite_message(1,0,0,"Less Than Three Points In DTM") ;
    goto errexit ;
   }
/*
** Check Sort Order
*/
 if( cdbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking Sort Order After Removing Duplicates dtmP->numPoints = %8ld",dtmP->numPoints) ;
    if( bcdtmCheck_sortOrderDtmObject(dtmP,1) != DTM_SUCCESS )
      {
       bcdtmWrite_message(1,0,0,"Dtm Sort Order Error") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Sort Order Valid After Removing Duplicates") ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Processing Dtm Object %p For Triangulation Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Processing Dtm Object %p For Triangulation Error",dtmP) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_removeDuplicatesDtmObjectOverload
( 
 BC_DTM_OBJ *dtmP,
 long       *numDuplicatesP,
 bool       duplicateOption 
)
/*
** This Function Removes Duplicate Points From The Dtm Object
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   n,ofs1,ofs2,ofs3,process,feature,removeFlag,numMarked=0 ;
 long   rollBackDuplicatePoints=FALSE,numDuplicatesRolledBack=0 ;
 long   nowTime=bcdtmClock(),startTime=bcdtmClock() ; 
 double dp  ;
 DPoint3d    *dupPtsP=nullptr ;
 DTM_TIN_POINT  *p1P,*p2P  ;
 BC_DTM_FEATURE *dtmFeatureP=nullptr ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ; 
 LongArray temP;
 LongArray::iterator sP;
 LongArray::iterator sP2;
 LongArray::iterator temPEnd;
 DTM_ROLLBACK_DATA* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : nullptr;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicates Dtm Object %p",dtmP) ;
/*
** Initialise
*/
 *numDuplicatesP = 0 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test If Dtm Object Already Processed
*/
 if( dtmP->dtmState != DTMState::PointsSorted ) { bcdtmWrite_message(2,0,0,"Dtm Object Not In Sorted State")      ; goto errexit ; }
/*
** Allocate Memory For Temp Array To Mark Duplicate Points
*/
 if( temP.resize(dtmP->numPoints) != DTM_SUCCESS) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Null Out Temp Array To Mark Duplicate Points
 temPEnd = temP.end();
 for( sP = temP.start() ; sP != temPEnd ; ++sP ) *sP = dtmP->nullPnt ;
/*
** Mark Points Within The Point To Point Tolerance
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Point To Point Tolerance") ;
 for( ofs1 = 0, sP = temP.start(); ofs1 < dtmP->numPoints - 1 ; ++ofs1, ++sP  )
   {
/*
**  Check For Check Stop Termination
*/
    if( (ofs1 &  0x3ff) == 0x3ff )
      {
       nowTime = bcdtmClock() ;
       if( ( double )( nowTime-startTime) / CLOCKS_PER_SEC > 0.025 )
         {  
          if( bcdtmTin_checkForTriangulationTermination (dtmP)) goto errexit  ;
         }
       startTime = nowTime ;
      }
/*
**  Mark Points
*/
    if( *sP == dtmP->nullPnt )
      {
       p1P = pointAddrP(dtmP,ofs1) ;
       sP2 = sP;
       ofs2 = ofs1 + 1 ;
       process = 1 ;
       while ( ofs2 < dtmP->numPoints && process )
         {
          ++sP2;
          p2P = pointAddrP(dtmP,ofs2) ;
/*
**        RobC Modification To Handle the long columns with raster DTM's
*/
          if( p2P->x - p1P->x > dtmP->ppTol )  
            {
             process = 0 ; 
             if( ofs2 - ofs1 > 2 && pointAddrP(dtmP,ofs2-1)->x == p1P->x )
               {
                process = 1 ;
                ofs3 = ofs1 ;
                while( ofs3 < ofs2 - 1 && process )
                  {
                   if( pointAddrP(dtmP,ofs3+1)->y - pointAddrP(dtmP,ofs3)->y < dtmP->ppTol ) process = 0 ;
                   else                                                                      ++ofs3  ;
                  }
                process = 0 ;
                if( ofs3 > ofs1 )
                  {
                   for( n = ofs1 + 1 ; n < ofs3 ; ++n ) ++sP ;
                   ofs1 = ofs3 - 1 ;
                  } 
               }
            }
/*
**        Robc Modification End 2-July-2009
*/
          else
            {
             if( *sP2 == dtmP->nullPnt )
               {
                if( fabs(p1P->y-p2P->y) < dtmP->ppTol )
                  {
                   removeFlag = FALSE ;
                   dp = bcdtmMath_distance(p1P->x,p1P->y,p2P->x,p2P->y) ;
                   if      ( duplicateOption == false && dp <  dtmP->mppTol ) removeFlag = TRUE ;
                   else if ( duplicateOption == true  && dp <  dtmP->ppTol  ) removeFlag = TRUE ;
                   if( removeFlag == TRUE ) 
                     {
                      *sP2 = ofs1;
                      ++*numDuplicatesP ;
                     }
                  }
               }
            }
          ++ofs2 ;
         }
      }
   }
if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",*numDuplicatesP) ;
/*
** Report Duplicate Points Being Removed
*/
 if( cdbg )
   {
    long dupCount=0 ;
    for( ofs1 = 0, sP = temP.start(); ofs1 < dtmP->numPoints  ; ++ofs1, ++sP  )
      {
       if( *sP != dtmP->nullPnt )
         {
          ++dupCount ;
          p1P = pointAddrP(dtmP,ofs1) ;
          bcdtmWrite_message(0,0,0,"Duplicate Point[%8ld] ** [%8ld] [%8ld] = %12.5lf %12.5lf %10.4lf",dupCount,ofs1,*sP,p1P->x,p1P->y,p1P->z) ;
         }
      }
    bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",dupCount) ;
   }
/*
** Reset Dtm Feature Point Offsets Of Merged Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reseting Point Offsets Of Merged Points ") ;
 for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
   {
    dtmFeatureP = ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
      {
       long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       bool rollBacked=FALSE ;
      for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          if( offsetP[n] < 0 || offsetP[n] > dtmP->numPoints )
            {
             bcdtmWrite_message(2,0,0,"Feature Point Index Range Error") ;
             goto errexit ;
            }  
          if( *(temP+offsetP[n]) != dtmP->nullPnt )
            {
/*
**           Check For Duplicate Point Error
*/
             if( bcdtmMath_pointDistance3DDtmObject(dtmP,*(temP+offsetP[n]),offsetP[n]) > 0.0 ) 
               {  
                if( rollBackInfo && rollBacked == false && rollBackInfo->rollBackDtmP != nullptr )
                  {
                   dupPtsP = ( DPoint3d * ) malloc ( dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d)) ;
                   if( dupPtsP == nullptr )
                     {
                      bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                      goto errexit ;
                     } 
                   for( int m = 0 ; m < dtmFeatureP->numDtmFeaturePts ; ++m )
                     {
                      int offset=offsetP[m] ;
                      if( offset < 0 ) offset = -(offset+1) ;
                      *(dupPtsP+m) = *(DPoint3d *) pointAddrP(dtmP,offset) ;
                     }  
                   if( dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull       && dtmFeatureP->dtmFeatureType != DTMFeatureType::HullLine  && 
                       dtmFeatureP->dtmFeatureType != DTMFeatureType::DrapeHull && dtmFeatureP->dtmFeatureType != DTMFeatureType::DrapeVoid &&  
                       dtmFeatureP->dtmFeatureType != DTMFeatureType::GraphicBreak                                                      )
                     {
                     if (bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId)) goto errexit;
                     }
                   if( dupPtsP != nullptr ) { free(dupPtsP) ; dupPtsP = nullptr ; }
                   rollBacked = true ;
                  }
                int offset=offsetP[n] ;
                if( offset < 0 ) offset = -(offset+1) ;
                offsetP[n] = *(temP+offset) ;
                if( *(temP+offsetP[n]) != dtmP->nullPnt && *(temP+offsetP[n])  >= 0 ) *(temP+offsetP[n]) = -(*(temP+offsetP[n])+1) ;
               }
            }
         } 
      }
   }
/*
** Copy Duplicate Points To Restore DTM For Roll Back Requirements
*/
 rollBackDuplicatePoints = FALSE ;
 if( rollBackInfo && rollBackInfo->rollBackDtmP != nullptr && rollBackDuplicatePoints == TRUE )
   {
/*
**  Backup Duplicate Points As Point Features With A Maximum  Of 1000 Points Per Point Feature
*/
    dupPtsP = ( DPoint3d *) malloc(1000*sizeof(DPoint3d)) ;
    if( dupPtsP == nullptr )
      {
       bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
       goto errexit ;  
      }
    numMarked = 0 ;
    for( sP = temP.start() ; sP != temP.end() ; ++sP ) 
      {
       if( *sP != dtmP->nullPnt ) 
         {
          if( *sP >= 0 )
            {
             if( bcdtmMath_pointDistance3DDtmObject(dtmP,*sP,(long)(sP-temP.start())) > 0.0 ) 
               {  
                *(dupPtsP+numMarked) = *( DPoint3d *)pointAddrP(dtmP,(long)(sP-temP.start())) ;
                ++numMarked ;
                if( numMarked == 1000 )
                  {
                   numDuplicatesRolledBack = numDuplicatesRolledBack + numMarked ;
                   if( bcdtmObject_storeDtmFeatureInDtmObject(rollBackInfo->rollBackDtmP,DTMFeatureType::GroupSpots,-dtmP->nullUserTag,1,&nullFeatureId,dupPtsP,numMarked)) goto errexit ;
                   numMarked = 0 ;
                  }
               }
            }
         }
      }
    if( numMarked > 0 )
      {
       numDuplicatesRolledBack = numDuplicatesRolledBack + numMarked ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(rollBackInfo->rollBackDtmP,DTMFeatureType::GroupSpots,-dtmP->nullUserTag,1,&nullFeatureId,dupPtsP,numMarked)) goto errexit ;
       numMarked = 0 ;
      } 
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numDuplicatesRolledBack = %8ld",numDuplicatesRolledBack) ;
   } 
/*
**  Mark Merged Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Merged Points") ;
 for( sP = temP.start() ; sP != temPEnd; ++sP ) 
   {
    if( *sP == dtmP->nullPnt ) *sP = 0 ;
    else                       *sP = 1 ; 
   }
/*
**  Remove Merged Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Merged Points") ;
 for( sP = temP.start(), ofs1 = ofs2 = 0 ; ofs2 < dtmP->numPoints ; ++ofs2, ++sP )
   {
    if( ! *sP )
      {
       if( ofs1 != ofs2 )
         {
          p1P = pointAddrP(dtmP,ofs1) ;
          p2P = pointAddrP(dtmP,ofs2) ;
          *p1P = *p2P ;
         }
       ++ofs1 ;
      }
   }
/*
**  Accumulate Delete Counts
*/
 for( sP = temP.start()+1  ; sP != temPEnd; ++sP ) *sP += *(sP-1) ;
/*
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resetting DTM Feature Point Offsets") ;
 for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
   {
    dtmFeatureP = ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
      {
      long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
         {
          offsetP[n] = offsetP[n] - *(temP+offsetP[n]) ;
         } 
      }
   }
/*
**  Reset Dtm Object Variables
*/
 dtmP->dtmState  = DTMState::DuplicatesRemoved ;
 dtmP->numPoints = dtmP->numSortedPoints = dtmP->numPoints - *numDuplicatesP ;
/*
**  Resize DTM Object Arrays
*/
 if( bcdtmObject_resizeMemoryDtmObject(dtmP)) goto errexit ;
/*
** Write Number Of Duplicate Points
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Number of Duplicate Points  = %9ld",*numDuplicatesP) ;
    bcdtmWrite_message(0,0,0,"**** Number of Dtm Points        = %9ld",dtmP->numPoints) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Duplicates Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Duplicates Dtm Object %p Error",dtmP) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

#ifdef OLDFUNCTIONS
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_triangulateStmTrianglesDtmObjectOld
(
 BC_DTM_OBJ *dtmP //  Pointer To DTM Object
 )
    {
    int ret = DTM_SUCCESS, dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0), tdbg = DTM_TIME_VALUE(0);
    long n, dtmFeature, numTrgPts, stmPoints[4], startTime = bcdtmClock ();
    long index, mid, bottom, top;
    DPoint3d*p3dP,*trgPtsP=nullptr  ;
    BC_DTM_FEATURE *dtmFeatureP;
    BC_DTM_OBJ *tempDtmP = nullptr;

    // Log Arguments
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Triangulating STM Triangles");
        bcdtmWrite_message (0, 0, 0, "dtmP  = %p", dtmP);
        }

    // Check For Valid DTM Object
    if (bcdtmObject_testForValidDtmObject (dtmP)) goto errexit;

    // Check For DTM Data State
    if (dtmP->dtmState != DTMState::Data)
        {
        bcdtmWrite_message (1, 0, 0, "Method Requires Untriangulated DTM");
        goto errexit;
        }

    // Check For Presence Of Features
    if (dtmP->numFeatures == 0)
        {
        bcdtmWrite_message (1, 0, 0, "No Triangles Present In DTM");
        goto errexit;
        }

    // Log Number Of Triangles
    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of STM Triangles = %8ld", dtmP->numFeatures);

    // Create Temporary DTM
    if (bcdtmObject_createDtmObject (&tempDtmP)) goto errexit;

    // Check For Presence Of And Validity Of STM Triangles
    for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
        {
        dtmFeatureP = ftableAddrP (dtmP, dtmFeature);

        // Check For None Graphic Break Feature
        if (dtmFeatureP->dtmFeatureType != DTMFeatureType::GraphicBreak)
            {
            bcdtmWrite_message (1, 0, 0, "None STM Triangle Features Present In DTM");
            goto errexit;
            }

        // Check For Correct Number Of Points
        if (dtmFeatureP->numDtmFeaturePts != 4)
            {
            bcdtmWrite_message (1, 0, 0, "Incorrect Number Of Points For STM Triangle");
            goto errexit;
            }

        // Get Triangle Points
        if (bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject (dtmP, dtmFeature, &trgPtsP, &numTrgPts)) goto errexit;

        // Check Triangle Closes
    if( trgPtsP->x != (trgPtsP+3)->x  || trgPtsP->y != (trgPtsP+3)->y )
            {
            bcdtmWrite_message (1, 0, 0, "STM Triangle Does Not Close");
            goto errexit;
            }

        // Check Elevations
    if( trgPtsP->z != (trgPtsP+3)->z )
            {
            bcdtmWrite_message (1, 0, 0, "STM Triangle Start And End Point Elevations Are Not The Same");
            goto errexit;
            }

        // Check For Colinear Triangle
    if( bcdtmMath_sideOf(trgPtsP->x,trgPtsP->y,(trgPtsP+2)->x,(trgPtsP+2)->y,(trgPtsP+1)->x,(trgPtsP+1)->y) == 0 )
            {
            bcdtmWrite_message (1, 0, 0, "Colinear STM Triangle");
            goto errexit;
            }

        // Store Triangle Points In Temp DTM
        if (bcdtmObject_storeDtmFeatureInDtmObject (tempDtmP, DTMFeatureType::RandomSpots, tempDtmP->nullUserTag, 1, &tempDtmP->nullFeatureId, trgPtsP, numTrgPts)) goto errexit;
        }

    // Log Input STM Triangles
    if (dbg == 2)
        {
        for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
            {
            dtmFeatureP = ftableAddrP (dtmP, dtmFeature);
            dtmFeatureP->dtmFeatureType = DTMFeatureType::Breakline;
            }
        bcdtmWrite_toFileDtmObject (dtmP, L"rawStmTriangles.tin");
        for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
            {
            dtmFeatureP = ftableAddrP (dtmP, dtmFeature);
            dtmFeatureP->dtmFeatureType = DTMFeatureType::GraphicBreak;
            }
        }

    // Process For Triangulation
    if (dbg) bcdtmWrite_message (0, 0, 0, "Processing For Triangulation ** Number Of DTM Points = %8ld", tempDtmP->numPoints);
    DTM_NORMALISE_OPTION = FALSE;
    DTM_DUPLICATE_OPTION = FALSE;
    tempDtmP->ppTol = tempDtmP->plTol = 0.0;
    if (bcdtmObject_processForTriangulationDtmObject (tempDtmP))goto errexit;
    if (dbg) bcdtmWrite_message (0, 0, 0, "Processing For Triangulation Completed ** Number Of DTM Points = %8ld", tempDtmP->numPoints);

    // Allocate Memory For Nodes Array
    if (bcdtmObject_allocateNodesMemoryDtmObject (tempDtmP)) goto errexit;

    // Initialise Circular List Parameters
    tempDtmP->cListPtr = 0;
    tempDtmP->cListDelPtr = tempDtmP->nullPtr;
    tempDtmP->numSortedPoints = tempDtmP->numPoints;

    //  Allocate Circular List Memory For Dtm Object
    if (dbg) bcdtmWrite_message (0, 0, 0, "Allocating Circular List Memory For Dtm Object");
    if (bcdtmObject_allocateCircularListMemoryDtmObject (tempDtmP)) goto errexit;

    // Create Circular List From STM Triangles
    if (dbg) bcdtmWrite_message (0, 0, 0, "Creating Circular List From STM Triangles");
    for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
        {
        if (dbg && dtmFeature % 1000 == 0) bcdtmWrite_message (0, 0, 0, "Inserting STM Triangle %8ld of %8ld", dtmFeature, dtmP->numFeatures);
        dtmFeatureP = ftableAddrP (dtmP, dtmFeature);
        if (bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject (dtmP, dtmFeature, &trgPtsP, &numTrgPts)) goto errexit;

        //  Get Point Numbers For STM Triangle
        for (n = 0, p3dP = trgPtsP; p3dP < trgPtsP + numTrgPts; ++n, ++p3dP)
            {
            // Binary Search To Find Related DTM Point Number
            index = -1;
            bottom = 0;
            top = tempDtmP->numPoints - 1;
       if      ( p3dP->x == pointAddrP(tempDtmP,bottom)->x && p3dP->y == pointAddrP(tempDtmP,bottom)->y ) index = bottom ;
       else if ( p3dP->x == pointAddrP(tempDtmP,top)->x    && p3dP->y == pointAddrP(tempDtmP,top)->y    ) index = top ;
            else
                {
                while (top - bottom > 1)
                    {
                    mid = (top + bottom) / 2;
             if      ( p3dP->x == pointAddrP(tempDtmP,mid)->x && p3dP->y == pointAddrP(tempDtmP,mid)->y ) top = bottom = index = mid ;
             else if ( p3dP->x >  pointAddrP(tempDtmP,mid)->x || ( p3dP->x == pointAddrP(tempDtmP,mid)->x && p3dP->y > pointAddrP(tempDtmP,mid)->y )) bottom = mid ;
                    else top = mid;
                    }
                }

            // Check Point Was Found
            if (index == -1)
                {
                bcdtmWrite_message (1, 0, 0, "Cannot Find DTM Point Number For STM Triangle Vertex");
                goto errexit;
                }
            stmPoints[n] = index;

            //  Check Point Found Has The Correct Coordinates
            if (cdbg)
                {
          if( p3dP->x != pointAddrP(tempDtmP,stmPoints[n])->x || p3dP->y != pointAddrP(tempDtmP,stmPoints[n])->y )
                    {
                    bcdtmWrite_message (2, 0, 0, "Incorrect Point Association");
                    goto errexit;
                    }
                }
            }

        // Insert Lines Into Circular List

        for (n = 0; n < 3; ++n)
            {
            if (!bcdtmList_testLineDtmObject (tempDtmP, stmPoints[n], stmPoints[n + 1]))
                {
                if (bcdtmList_insertLineDtmObject (tempDtmP, stmPoints[n], stmPoints[n + 1])) goto errexit;
                }
            }
        }

    // Check And If Necessary Fix Tin Topology
    if (bcdtmObject_checkAndFixTopologyStmTrianglesDtmObject (tempDtmP, dtmP)) goto errexit;

    // Overwrite STM Triangles DTM
    bcdtmObject_initialiseDtmObject (dtmP);
    *dtmP = *tempDtmP;
    tempDtmP = nullptr;

    // Check Triangulated STM Triangles
    if (cdbg)
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Checking DTM Validity");
        if (bcdtmCheck_trianglesDtmObject (dtmP))
            {
            bcdtmWrite_message (1, 0, 0, "DTM Invalid");
            goto errexit;
            }
        if (dbg) bcdtmWrite_message (0, 0, 0, "Checking DTM Valid");
        }

    // Log Triangulation Statistics
    if (dbg == 1) bcdtmObject_reportStatisticsDtmObject (dtmP);

    // Log Triangualtion Times
    if (tdbg) bcdtmWrite_message (0, 0, 0, "Time To Triangulate %8ld STM Triangles = %8.3lf Seconds", dtmP->numFeatures, bcdtmClock_elapsedTime (bcdtmClock (), startTime));

    // Clean Up
cleanup:
    DTM_NORMALISE_OPTION = TRUE;
    DTM_DUPLICATE_OPTION = TRUE;
    if (trgPtsP != nullptr) free (trgPtsP);
    if (tempDtmP != nullptr) bcdtmObject_destroyDtmObject (&tempDtmP);

    // Return

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Triangulating STM Triangles Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Triangulating STM Triangles Error");
    return (ret);

    // Error Exit

errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }
#endif
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmObject_checkAndFixTopologyStmTrianglesDtmObject
(
 BC_DTM_OBJ *dtmP, //  Pointer To Circular List DTM Object
 BC_DTM_OBJ *trgDtmP //  Pointer To Untriangulated DTM Object
 )
    {
    int ret = DTM_SUCCESS, dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0), tdbg = DTM_TIME_VALUE(0);
    long n, cPtr, pnt, npnt, spnt, antPnt, clkPnt, numUnconnectedTriangles = 0;
    long numTriangles, hullStartPnt, hullNextPnt, numTinHulls, numConnected, numUnconnected;
    long cNpnt, cSpnt, lcNpnt, mark = -9999, numMarked = 0, totalNumMarked = 0;
    long numHullPts, numVoidHullStartPoints = 0, hullStartPoints[100], voidHullStartPoints[100];
 DPoint3d trgPts[4],*hullPtsP=nullptr ;
    DTM_TIN_POINT *pntP;
    BC_DTM_OBJ *tempDtmP = nullptr, *voidP = nullptr;
    bool missingPoints = false;

    // Log Arguments

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Checking And Fixing Topology STM Triangles");
        bcdtmWrite_message (0, 0, 0, "dtmP    = %p", dtmP);
        bcdtmWrite_message (0, 0, 0, "trgDtmP = %p", trgDtmP);
        }

    // Create Temporary DTM Object

    if (dbg)
        {
        if (bcdtmObject_createDtmObject (&tempDtmP)) goto errexit;
        }

    // Count Number Of Triangles

    if (cdbg == 1)
        {
        bcdtmWrite_message (0, 0, 0, "Counting Number Of Circular List Triangles");
        numTriangles = 0;
        for (pnt = 0; pnt < dtmP->numPoints; ++pnt)
            {
            if ((cPtr = nodeAddrP (dtmP, pnt)->cPtr) != dtmP->nullPtr)
                {
                clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, pnt, clkPnt)) < 0) goto errexit;
                if (antPnt != clkPnt)
                    {
                    while (cPtr != dtmP->nullPtr)
                        {
                        clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                        cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                        if (antPnt > pnt && clkPnt > pnt && bcdtmList_testLineDtmObject (dtmP, antPnt, clkPnt))
                            {
                            if (bcdtmMath_pointSideOfDtmObject (dtmP, clkPnt, antPnt, pnt) > 0)
                                {
                                ++numTriangles;
                                if (dbg)
                                    {
                                    pntP = pointAddrP (dtmP, pnt);
                         trgPts[0].x = pntP->x ; trgPts[0].y = pntP->y ; trgPts[0].z = pntP->z ;
                                    trgPts[0].y = pntP->y;
                                    trgPts[0].z = pntP->z;
                                    pntP = pointAddrP (dtmP, antPnt);
                         trgPts[1].x = pntP->x ; trgPts[1].y = pntP->y ; trgPts[1].z = pntP->z ;
                                    trgPts[1].y = pntP->y;
                                    trgPts[1].z = pntP->z;
                                    pntP = pointAddrP (dtmP, clkPnt);
                         trgPts[2].x = pntP->x ; trgPts[2].y = pntP->y ; trgPts[2].z = pntP->z ;
                                    trgPts[2].y = pntP->y;
                                    trgPts[2].z = pntP->z;
                                    pntP = pointAddrP (dtmP, pnt);
                         trgPts[3].x = pntP->x ; trgPts[3].y = pntP->y ; trgPts[3].z = pntP->z ;
                                    trgPts[3].y = pntP->y;
                                    trgPts[3].z = pntP->z;
                                    if (bcdtmObject_storeDtmFeatureInDtmObject (tempDtmP, DTMFeatureType::Breakline, tempDtmP->nullUserTag, 1, &tempDtmP->nullFeatureId, trgPts, 4)) goto errexit;
                                    }
                                }
                            }
                        antPnt = clkPnt;
                        }
                    }
                }
            }

        // Save Circular List Triangles To File

        if (dbg == 1)
            {
            bcdtmWrite_geopakDatFileFromDtmObject (tempDtmP, L"circularListTriangles.dat");
            }

        // Log Triangle Counts

        if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of Circular List Triangles = %8ld", numTriangles);
        if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of STM           Triangles = %8ld", trgDtmP->numFeatures);

        // Check Triangle Counts

        if (numTriangles != trgDtmP->numFeatures)
            {
            bcdtmWrite_message (1, 0, 0, "Incorrect Number Of Triangles");
            //       goto errexit ;

            // Log Problem STM Triangles - Development Only

            //      if( bcdtmObject_logProblemStmTrianglesDtmObject(dtmP,trgDtmP)) goto errexit ;

            }
        }

    // Scan Points Looking For Missing External And Internal Triangles Triangles

    if (bcdtmObject_findAndFixMissingStmTrianglesDtmObject (dtmP)) goto errexit;
    if (true) goto resume;

    // RobC 7Jun2013 - The following Code to label resume is now redundant
    // I have left The code in Place As There May Be need To Come Back to it.

    // Scan Points Looking For Hull Start Points

    numTinHulls = 0;
    for (pnt = 0; pnt < dtmP->numPoints; ++pnt)
        {
        pntP = pointAddrP (dtmP, pnt);
        if (nodeAddrP (dtmP, pnt)->sPtr == dtmP->nullPnt && nodeAddrP (dtmP, pnt)->tPtr == dtmP->nullPnt)
            {

       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Processing Point %8ld ** %12.5lf %12.5lf %10.4lf",pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z ) ;

            // Check Point Has Connected Points

            if ((cPtr = nodeAddrP (dtmP, pnt)->cPtr) == dtmP->nullPtr)
                {
                bcdtmWrite_message (0, 0, 0, "Pnt %8ld Has No Connected Points", pnt);
                }
            else
                {
                clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, pnt, clkPnt)) < 0) goto errexit;
                if (antPnt == clkPnt)
                    {
                    bcdtmWrite_message (0, 0, 0, "Pnt %8ld Has Only One Connected Point", pnt);
                    }
                else
                    {

                    // Check For Hull Start Point

                    hullStartPnt = dtmP->nullPnt;
                    hullNextPnt = dtmP->nullPnt;

                    // Check For Knot Point As You Cannot Start On A Knot Point

                    numConnected = 0;
                    numUnconnected = 0;
                    do
                        {
                        clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                        if (!bcdtmList_testLineDtmObject (dtmP, antPnt, clkPnt))
                            {
                            ++numUnconnected;
                            }
                        ++numConnected;
                        antPnt = clkPnt;
                        cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                        }
                    while (cPtr != dtmP->nullPtr);

                    //

                    if (dbg) bcdtmWrite_message (0, 0, 0, "numConnected = %8ld", numConnected);

                    // Check For Hull Start Point

                    if (numConnected == 2)
                        {
                        cPtr = nodeAddrP (dtmP, pnt)->cPtr;
                        clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                        if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, pnt, clkPnt)) < 0) goto errexit;
                        if (bcdtmMath_pointSideOfDtmObject (dtmP, clkPnt, antPnt, pnt) == 1)
                            {
                            hullStartPnt = pnt;
                            hullNextPnt = clkPnt;
                            }
                        else
                            {
                            hullStartPnt = pnt;
                            hullNextPnt = antPnt;
                            }
                        }

                    //  Look For Isolated Triangles

                    if (numUnconnected == 1)
                        {
                        cPtr = nodeAddrP (dtmP, pnt)->cPtr;
                        clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                        if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, pnt, clkPnt)) < 0) goto errexit;
                        do
                            {
                            clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                            if (!bcdtmList_testLineDtmObject (dtmP, antPnt, clkPnt))
                                {
                                hullStartPnt = pnt;
                                hullNextPnt = antPnt;
                                if (dbg) bcdtmWrite_message (0, 0, 0, "Hull Start Found ** pnt = %8ld antPnt = %8ld clkPnt = %8ld", pnt, antPnt, clkPnt);
                                }
                            antPnt = clkPnt;
                            cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                            }
                        while (cPtr != dtmP->nullPtr);
                        }

                    // If Hull Start Point Found Insert Hull

                    if (hullStartPnt != dtmP->nullPnt)
                        {

                        //  Insert Tin Hull

                        if (dbg)
                            {
                   bcdtmWrite_message(0,0,0,"hullStartPnt = %8ld ** %12.5lf %12.5lf %10.4lf",hullStartPnt,pointAddrP(dtmP,hullStartPnt)->x,pointAddrP(dtmP,hullStartPnt)->y,pointAddrP(dtmP,hullStartPnt)->z) ;
                   bcdtmWrite_message(0,0,0,"hullNextPnt  = %8ld ** %12.5lf %12.5lf %10.4lf",hullNextPnt,pointAddrP(dtmP,hullNextPnt)->x,pointAddrP(dtmP,hullNextPnt)->y,pointAddrP(dtmP,hullNextPnt)->z) ;
                            }
                        spnt = hullStartPnt;
                        npnt = hullNextPnt;
                        do
                            {

                            // Log Hull Line

                            if (dbg == 2)
                                {
                                bcdtmWrite_message (0, 0, 0, "**** Setting Hull Line ** spnt = %8ld npnt = %8ld", spnt, npnt);
                                pntP = pointAddrP (dtmP, spnt);
                      bcdtmWrite_message(0,0,0,"spnt = %8ld ** %12.5lf %12.5lf %10.4lf",spnt,pntP->x,pntP->y,pntP->z) ;
                                pntP = pointAddrP (dtmP, npnt);
                      bcdtmWrite_message(0,0,0,"npnt = %8ld ** %12.5lf %12.5lf %10.4lf",npnt,pntP->x,pntP->y,pntP->z) ;
                                }

                            // Check For Hull Knot

                            if (npnt != hullStartPnt && nodeAddrP (dtmP, npnt)->hPtr != dtmP->nullPnt)
                                {
                                if (dbg)
                                    {
                                    bcdtmWrite_message (0, 0, 0, "Point %8ld Already Marked As Hull Point", npnt);
                        bcdtmWrite_message(0,0,0,"npnt %8ld ** %12.5lf %12.5lf %10.4lf",npnt,pointAddrP(dtmP,npnt)->x,pointAddrP(dtmP,npnt)->y,pointAddrP(dtmP,npnt)->z) ;
                        bcdtmWrite_message(0,0,0,"spnt %8ld ** %12.5lf %12.5lf %10.4lf",spnt,pointAddrP(dtmP,spnt)->x,pointAddrP(dtmP,spnt)->y,pointAddrP(dtmP,spnt)->z) ;
                                    }

                                // Fix Hull At Knot Point

                                if (bcdtmObject_fixStmHullKnotPointDtmObject (dtmP, npnt, spnt)) goto errexit;

                                // Reset Next Point On Tin Hull

                                if (nodeAddrP (dtmP, npnt)->hPtr != dtmP->nullPnt)
                                    {
                                    if ((npnt = bcdtmList_nextClkDtmObject (dtmP, spnt, npnt)) < 0) goto errexit;
                                    if (dbg) bcdtmWrite_message (0, 0, 0, "npnt = %8ld", npnt);
                                    }

                                }
                            nodeAddrP (dtmP, spnt)->hPtr = npnt;
                            if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, npnt, spnt)) < 0) goto errexit;
                            spnt = npnt;
                            npnt = antPnt;
                            }
                        while (spnt != hullStartPnt);

                        // Mark All Points Internal To Tin Hull

                        hullStartPoints[numTinHulls] = hullStartPnt;
                        ++numTinHulls;
                        if (numTinHulls >= 100)
                            {
                            bcdtmWrite_message (1, 0, 0, "Too Many Tin Hulls");
                            goto errexit;
                            }
                        if (bcdtmList_copyHptrListToTptrListDtmObject (dtmP, hullStartPnt)) goto errexit;
                        if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Internal STM Triangle Points");
                        if (bcdtmMark_internalStmTrianglePointsDtmObject (dtmP, hullStartPnt, mark, &numMarked)) goto errexit;
                        if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of Internal STM Triangle Points Marked = %8ld", numMarked);
                        }
                    }
                }
            }
        }

    // Check For Missing Points Not Included With The Tin Hull

    missingPoints = false;
    for (pnt = 0; pnt < dtmP->numPoints; ++pnt)
        {
        if (nodeAddrP (dtmP, pnt)->sPtr == dtmP->nullPnt && nodeAddrP (dtmP, pnt)->tPtr == dtmP->nullPnt)
            {
       bcdtmWrite_message(0,0,0,"Mising Point = %8ld %12.5lf %12.5lf %10.4lf",pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z ) ;
            missingPoints = true;
            }
        }
    if (missingPoints)
        {
        bcdtmWrite_message (1, 0, 0, "Missing Points In Triangulation");
        goto errexit;
        }

    // Add Voids If More Than One Tin Hull

    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of Tin Hulls = %2d", numTinHulls);
    if (numTinHulls > 1)
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Resolving Multiple Tin Hulls");
        if (bcdtmObject_createDtmObject (&voidP)) goto errexit;
        for (n = 0; n < numTinHulls; ++n)
            {
            if (bcdtmList_copyTptrListToPointArrayDtmObject (dtmP, hullStartPoints[n], &hullPtsP, &numHullPts)) goto errexit;
            if (bcdtmObject_storeDtmFeatureInDtmObject (voidP, DTMFeatureType::Breakline, voidP->nullUserTag, 1, &voidP->nullFeatureId, hullPtsP, numHullPts)) goto errexit;
            }
        voidP->ppTol = voidP->plTol = 0.0;
        bcdtmObject_triangulateDtmObject (voidP, false, false);
        if (bcdtmList_removeNoneFeatureHullLinesDtmObject (voidP)) goto errexit;
        if (dbg == 1) bcdtmWrite_toFileDtmObject (voidP, L"void.tin");

        // Insert Lines Between Different Tin Hulls

        spnt = voidP->hullPoint;
        if (dbg) bcdtmWrite_message (0, 0, 0, "Inserting Lines Between Hulls");
        dtmP->numSortedPoints = dtmP->numPoints;
        dtmP->dtmState = DTMState::Tin;
        do
            {
            npnt = nodeAddrP (voidP, spnt)->hPtr;
       bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,spnt)->x,pointAddrP(voidP,spnt)->y,&cSpnt) ;
       bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,npnt)->x,pointAddrP(voidP,npnt)->y,&cNpnt) ;
            if (!bcdtmList_testLineDtmObject (dtmP, cSpnt, cNpnt))
                {
                voidHullStartPoints[numVoidHullStartPoints] = spnt;
                ++numVoidHullStartPoints;
                if (dbg == 2)
                    {
                    bcdtmWrite_message (0, 0, 0, "Inserting Hull Line %8ld %8ld", cSpnt, cNpnt);
             bcdtmWrite_message(0,0,0,"cSpnt = %8ld ** %12.5lf %12.5lf %10.4lf",cSpnt,pointAddrP(dtmP,cSpnt)->x,pointAddrP(dtmP,cSpnt)->y,pointAddrP(dtmP,cSpnt)->z ) ;
             bcdtmWrite_message(0,0,0,"cNpnt = %8ld ** %12.5lf %12.5lf %10.4lf",cNpnt,pointAddrP(dtmP,cNpnt)->x,pointAddrP(dtmP,cNpnt)->y,pointAddrP(dtmP,cNpnt)->z ) ;
                    }
                if (bcdtmList_insertLineDtmObject (dtmP, cSpnt, cNpnt)) goto errexit;

                // Insert Fill In Lines

                lcNpnt = cNpnt;
                if ((antPnt = bcdtmList_nextAntDtmObject (voidP, spnt, npnt)) < 0) goto errexit;
                while (!bcdtmList_testForBreakLineDtmObject (voidP, spnt, antPnt))
                    {
             bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,antPnt)->x,pointAddrP(voidP,antPnt)->y,&cNpnt) ;
                    if (!bcdtmList_testLineDtmObject (dtmP, cSpnt, cNpnt))
                        {
                        if (dbg == 2)
                            {
                            bcdtmWrite_message (0, 0, 0, "Inserting Fill In Line %8ld %8ld", cSpnt, cNpnt);
                   bcdtmWrite_message(0,0,0,"cSpnt = %8ld ** %12.5lf %12.5lf %10.4lf",cSpnt,pointAddrP(dtmP,cSpnt)->x,pointAddrP(dtmP,cSpnt)->y,pointAddrP(dtmP,cSpnt)->z ) ;
                   bcdtmWrite_message(0,0,0,"cNpnt = %8ld ** %12.5lf %12.5lf %10.4lf",cNpnt,pointAddrP(dtmP,cNpnt)->x,pointAddrP(dtmP,cNpnt)->y,pointAddrP(dtmP,cNpnt)->z ) ;
                            }
                        if (bcdtmList_insertLineDtmObject (dtmP, cSpnt, cNpnt)) goto errexit;
                        if (!bcdtmList_testLineDtmObject (dtmP, cNpnt, lcNpnt))
                            {
                            if (dbg == 2)
                                {
                                bcdtmWrite_message (0, 0, 0, "Inserting Backwards Fill In Line %8ld %8ld", cNpnt, lcNpnt);
                      bcdtmWrite_message(0,0,0,"cNpnt  = %8ld ** %12.5lf %12.5lf %10.4lf",cNpnt,pointAddrP(dtmP,cNpnt)->x,pointAddrP(dtmP,cNpnt)->y,pointAddrP(dtmP,cNpnt)->z ) ;
                      bcdtmWrite_message(0,0,0,"lcNpnt = %8ld ** %12.5lf %12.5lf %10.4lf",lcNpnt,pointAddrP(dtmP,lcNpnt)->x,pointAddrP(dtmP,lcNpnt)->y,pointAddrP(dtmP,lcNpnt)->z ) ;
                                }
                            if (bcdtmList_insertLineDtmObject (dtmP, cNpnt, lcNpnt)) goto errexit;
                            }
                        lcNpnt = cNpnt;
                        }
                    if ((antPnt = bcdtmList_nextAntDtmObject (voidP, spnt, antPnt)) < 0) goto errexit;
                    }
                }
            spnt = npnt;
            }
        while (spnt != voidP->hullPoint);

        //  Insert Voids On Hull

        if (dbg) bcdtmWrite_message (0, 0, 0, "numVoidHullStartPoints = %8ld", numVoidHullStartPoints);
        for (n = 0; n < numVoidHullStartPoints; ++n)
            {
            if (dbg == 1) bcdtmWrite_message (0, 0, 0, "Inserting Hull Void %4d ** startPnt = %8ld", n, voidHullStartPoints[n]);
            spnt = voidHullStartPoints[n];
            if (nodeAddrP (voidP, spnt)->sPtr == dtmP->nullPnt)
                {
                npnt = nodeAddrP (voidP, spnt)->hPtr;
            bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,spnt)->x,pointAddrP(voidP,spnt)->y,&cSpnt) ;
            bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,npnt)->x,pointAddrP(voidP,npnt)->y,&cNpnt) ;
                hullStartPnt = cSpnt;
                nodeAddrP (dtmP, cSpnt)->tPtr = cNpnt;
                do
                    {
                    if (dbg == 2)
                        {
                        bcdtmWrite_message (0, 0, 0, "Inserting Void Hull Line");
                  bcdtmWrite_message(0,0,0,"**** spnt = %8ld ** %12.5lf %12.5lf %10.4lf",spnt,pointAddrP(voidP,spnt)->x,pointAddrP(voidP,spnt)->y,pointAddrP(voidP,spnt)->z ) ;
                  bcdtmWrite_message(0,0,0,"**** npnt = %8ld ** %12.5lf %12.5lf %10.4lf",npnt,pointAddrP(voidP,npnt)->x,pointAddrP(voidP,npnt)->y,pointAddrP(voidP,npnt)->z ) ;
                        }
                    clkPnt = spnt;
                    if ((clkPnt = bcdtmList_nextClkDtmObject (voidP, npnt, clkPnt)) < 0) goto errexit;
                    while (!bcdtmList_testForBreakLineDtmObject (voidP, npnt, clkPnt) && nodeAddrP (voidP, npnt)->hPtr != clkPnt)
                        {
                        if ((clkPnt = bcdtmList_nextClkDtmObject (voidP, npnt, clkPnt)) < 0) goto errexit;
                        }
                    spnt = npnt;
                    npnt = clkPnt;
                    nodeAddrP (voidP, spnt)->sPtr = spnt;
                    nodeAddrP (voidP, npnt)->sPtr = npnt;
               bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,spnt)->x,pointAddrP(voidP,spnt)->y,&cSpnt) ;
               bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,npnt)->x,pointAddrP(voidP,npnt)->y,&cNpnt) ;
                    nodeAddrP (dtmP, cSpnt)->tPtr = cNpnt;
                    }
                while (spnt != voidHullStartPoints[n]);
                if (bcdtmInsert_addDtmFeatureToDtmObject (dtmP, nullptr, 0, DTMFeatureType::Void, dtmP->nullUserTag, dtmP->nullFeatureId, hullStartPnt, 1)) goto errexit;
                }
            }
        }

    // Set Required Tin State Variables

resume:
    dtmP->dtmState = DTMState::Tin;
    dtmP->ppTol = trgDtmP->ppTol;
    dtmP->plTol = trgDtmP->plTol;
    // dtmP->hullPoint = hullStartPnt ;
    // dtmP->nextHullPoint = hullNextPnt ;
    dtmP->numSortedPoints = dtmP->numPoints;
    if (bcdtmList_cleanDtmObject (dtmP)) goto errexit;
    if (dbg == 1) bcdtmWrite_toFileDtmObject (dtmP, L"cleanStmTriangles.tin");

    // Clean Up

cleanup:
    if (voidP != nullptr) bcdtmObject_destroyDtmObject (&voidP);
    if (tempDtmP != nullptr) bcdtmObject_destroyDtmObject (&tempDtmP);

    // Return

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Checking And Fixing Topology Stm Triangles Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Checking And Fixing Topology Stm Triangles Error");
    return (ret);

    // Error Exit

errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmObject_fixStmHullKnotPointDtmObject
(
 BC_DTM_OBJ *dtmP, //  Pointer To Circular List DTM Object
 long hullPoint, //  Point At Which Knot Occurs On Hull
 long priorHullPoint //  Prior Point On Tin Hull Before Knot Point
 )
    {
    int ret = DTM_SUCCESS, dbg = 0, cdbg = 0;
    int sideOf1, sideOf2;
    long ppnt1, npnt1, ppnt2, npnt2;
    double radius1, radius2;
    bool fixClk = false, fixAnt = false;

    // Log Arguments

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Fixing STM Hull Knot Point");
        bcdtmWrite_message (0, 0, 0, "dtmP           = %p", dtmP);
        bcdtmWrite_message (0, 0, 0, "hullPoint      = %8ld", hullPoint);
        bcdtmWrite_message (0, 0, 0, "priorHullPoint = %8ld", priorHullPoint);
        }

    // Get Prior Point And Next Points At Hull Knot Point

    ppnt1 = priorHullPoint;
    if ((npnt1 = bcdtmList_nextAntDtmObject (dtmP, hullPoint, ppnt1)) < 0) goto errexit;
    if ((ppnt2 = bcdtmList_nextClkDtmObject (dtmP, hullPoint, ppnt1)) < 0) goto errexit;
    npnt2 = ppnt1;
    while (nodeAddrP (dtmP, ppnt2)->hPtr != hullPoint)
        {
        npnt2 = ppnt2;
        if ((ppnt2 = bcdtmList_nextClkDtmObject (dtmP, hullPoint, ppnt2)) < 0) goto errexit;
        }

    // Log Prior And Next Points

    if (dbg == 1)
        {
     bcdtmWrite_message(0,0,0,"npnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",npnt1,pointAddrP(dtmP,npnt1)->x,pointAddrP(dtmP,npnt1)->y,pointAddrP(dtmP,npnt1)->z) ;
     bcdtmWrite_message(0,0,0,"ppnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",ppnt1,pointAddrP(dtmP,ppnt1)->x,pointAddrP(dtmP,ppnt1)->y,pointAddrP(dtmP,ppnt1)->z) ;
     bcdtmWrite_message(0,0,0,"npnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",npnt2,pointAddrP(dtmP,npnt2)->x,pointAddrP(dtmP,npnt2)->y,pointAddrP(dtmP,npnt2)->z) ;
     bcdtmWrite_message(0,0,0,"ppnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",ppnt2,pointAddrP(dtmP,ppnt2)->x,pointAddrP(dtmP,ppnt2)->y,pointAddrP(dtmP,ppnt2)->z) ;
        }

    // Calculate Side Ofs

    sideOf1 = bcdtmMath_pointSideOfDtmObject (dtmP, ppnt1, npnt1, hullPoint);
    sideOf2 = bcdtmMath_pointSideOfDtmObject (dtmP, ppnt2, npnt2, hullPoint);
    if (dbg == 1) bcdtmWrite_message (0, 0, 0, "sideOf1 = %2d sideOf2 = %2d", sideOf1, sideOf2);

    // Fix Knot Point

    fixClk = fixAnt = false;
    if (sideOf1 <= 0 && sideOf2 == 1)
        fixClk = true;
    else if (sideOf1 == 1 && sideOf2 <= 0)
        fixAnt = true;
    else if (sideOf1 == 1 && sideOf2 == 1)
        {

        // Choose Between Sides On The Basis Of The Delaunay Criteria

        if (dbg) bcdtmWrite_message (0, 0, 0, "Fixing According To The Delaunay Criteria");
        radius1 = bcdtmMath_calculateRadiusOfCircumscribedCircleDtmObject (dtmP, ppnt1, hullPoint, npnt1);
        radius2 = bcdtmMath_calculateRadiusOfCircumscribedCircleDtmObject (dtmP, ppnt2, hullPoint, npnt2);
        if (radius1 <= radius2)
            fixAnt = true;
        else
            fixClk = true;
        }
    else
        {
        bcdtmWrite_message (1, 0, 0, "Cannot Fix STM HULL Knot");
        goto errexit;
        }

    // Insert Hull Line Fix

    if (fixAnt)
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Fixing Anti Clockwise");
        if (dbg) bcdtmWrite_message (0, 0, 0, "Inserting Line ppnt1 = %8ld npnt1 = %8ld", ppnt1, npnt1);

        // Need to test if the line doesn't overlap the existing triangle.

        long lineConnected = 0;
        if (bcdtmTin_swapTinLinesThatIntersectConnectLineDtmObject (dtmP, ppnt1, npnt1, &lineConnected)) goto errexit;
        if (!bcdtmList_testLineDtmObject (dtmP, ppnt1, npnt1))
            if (bcdtmList_insertLineDtmObject (dtmP, ppnt1, npnt1)) goto errexit;
        if (nodeAddrP (dtmP, ppnt1)->hPtr != dtmP->nullPnt)
            {
            if (dbg) bcdtmWrite_message (0, 0, 0, "Setting ppnt1[%8ld]->hPtr = npnt1[%8ld]", ppnt1, npnt1);
            nodeAddrP (dtmP, ppnt1)->hPtr = npnt1;
            nodeAddrP (dtmP, hullPoint)->hPtr = dtmP->nullPnt;
            if (dbg) bcdtmWrite_message (0, 0, 0, "hullPoint[%8ld]->hPtr = %10ld", hullPoint, nodeAddrP (dtmP, hullPoint)->hPtr);
            }

        // Add Void Representing The Added Triangle To Tin

        nodeAddrP (dtmP, hullPoint)->tPtr = ppnt1;
        nodeAddrP (dtmP, ppnt1)->tPtr = npnt1;
        nodeAddrP (dtmP, npnt1)->tPtr = hullPoint;
        if (bcdtmInsert_addDtmFeatureToDtmObject (dtmP, nullptr, 0, DTMFeatureType::Void, dtmP->nullUserTag, dtmP->nullFeatureId, hullPoint, 1)) goto errexit;

        }

    if (fixClk)
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Fixing Clockwise");
        if (dbg) bcdtmWrite_message (0, 0, 0, "Inserting Line ppnt2 = %8ld npnt2 = %8ld", ppnt2, npnt2);
        if (bcdtmList_insertLineDtmObject (dtmP, ppnt2, npnt2)) goto errexit;
        if (nodeAddrP (dtmP, ppnt2)->hPtr != dtmP->nullPnt)
            {
            if (dbg) bcdtmWrite_message (0, 0, 0, "Setting ppnt2[%8ld]->hPtr = npnt2[%8ld]", ppnt2, npnt2);
            nodeAddrP (dtmP, ppnt2)->hPtr = npnt2;
            nodeAddrP (dtmP, hullPoint)->hPtr = dtmP->nullPnt;
            if (dbg) bcdtmWrite_message (0, 0, 0, "hullPoint[%8ld]->hPtr = %10ld", hullPoint, nodeAddrP (dtmP, hullPoint)->hPtr);
            }

        // Add Void Representing The Added Triangle To Tin

        nodeAddrP (dtmP, hullPoint)->tPtr = ppnt2;
        nodeAddrP (dtmP, ppnt2)->tPtr = npnt2;
        nodeAddrP (dtmP, npnt2)->tPtr = hullPoint;
        if (bcdtmInsert_addDtmFeatureToDtmObject (dtmP, nullptr, 0, DTMFeatureType::Void, dtmP->nullUserTag, dtmP->nullFeatureId, hullPoint, 1)) goto errexit;
        }

    // Clean Up

cleanup:

    // Return

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Fixing STM Hull Knot Point Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Fixing STM Hull Knot Point Error");
    return (ret);

    // Error Exit

errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_internalStmTrianglePointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 long startPnt,
 long mark,
 long *numMarkedP
 )
//
// This Function Marks All Points Internal To A tPtr Polygon.
//
// The tPtr Polygon Must Be Set AntiClockwise
//
    {
    long ret = DTM_SUCCESS, dbg = 0;
    long priorPnt, scanPnt, nextPnt, antPnt, clPnt, clPtr, firstPnt, lastPnt;

    // Log Arguments

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Mark Internal STM Triangle Points");
        bcdtmWrite_message (0, 0, 0, "dtmP           = %p", dtmP);
        bcdtmWrite_message (0, 0, 0, "startPnt       = %8ld", startPnt);
        bcdtmWrite_message (0, 0, 0, "mark           = %8ld", mark);
        bcdtmWrite_message (0, 0, 0, "*numMarkedP    = %8ld", *numMarkedP);
        }

    // Log Tptr List

    if (dbg == 2) bcdtmList_writeTptrListDtmObject (dtmP, startPnt);

    // Initialise

    *numMarkedP = 0;
    firstPnt = lastPnt = dtmP->nullPnt;

    // Check Start Point tPtr List Is Not Null

    if (nodeAddrP (dtmP, startPnt)->tPtr == dtmP->nullPnt)
        {
        bcdtmWrite_message (2, 0, 0, "Tptr List Start Point Is Null");
        goto errexit;
        }

    // Log Coordinates Of Starting Point

    if (dbg == 1)
        {
    bcdtmWrite_message(0,0,0,"Start Point = %8ld ** %12.5lf %12.5lf %10.4lf",startPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,startPnt)->z ) ;
        }

    // Scan Around Tptr Polygon And Mark Internal Points And Create Internal Tptr List

    if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Internal To Tptr Polygon");
    priorPnt = startPnt;
    scanPnt = nodeAddrP (dtmP, startPnt)->tPtr;
    do
        {
        antPnt = nextPnt = nodeAddrP (dtmP, scanPnt)->tPtr;
        if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, scanPnt, antPnt)) < 0) goto errexit;
        while (antPnt != priorPnt)
            {
            if (dbg == 2) bcdtmWrite_message (0, 0, 0, "priorPnt = %8ld scanPnt = %8ld nextPnt = %8ld", priorPnt, scanPnt, nextPnt);
            if (nodeAddrP (dtmP, antPnt)->sPtr == dtmP->nullPnt)
                {
                nodeAddrP (dtmP, antPnt)->sPtr = mark;
                ++(*numMarkedP);
                clPtr = nodeAddrP (dtmP, antPnt)->cPtr;
                while (clPtr != dtmP->nullPtr)
                    {
                    clPnt = clistAddrP (dtmP, clPtr)->pntNum;
                    clPtr = clistAddrP (dtmP, clPtr)->nextPtr;
                    if (nodeAddrP (dtmP, clPnt)->sPtr == dtmP->nullPnt)
                        {
                        if (lastPnt == dtmP->nullPnt)
                            {
                            firstPnt = lastPnt = clPnt;
                            }
                        else
                            {
                            nodeAddrP (dtmP, lastPnt)->sPtr = clPnt;
                            lastPnt = clPnt;
                            }
                        nodeAddrP (dtmP, clPnt)->sPtr = clPnt;
                        ++(*numMarkedP);
                        }
                    }
                }
            if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, scanPnt, antPnt)) < 0) goto errexit;
            }
        priorPnt = scanPnt;
        scanPnt = nextPnt;
        }
    while (priorPnt != startPnt);

    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Marked Immediately Internal = %8ld", *numMarkedP);

    // Scan Tptr List And Mark Connected Points

    if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Sptr List Points");
    while (firstPnt != lastPnt)
        {
        nextPnt = nodeAddrP (dtmP, firstPnt)->sPtr;
        nodeAddrP (dtmP, firstPnt)->sPtr = mark;
        ++(*numMarkedP);
        clPtr = nodeAddrP (dtmP, firstPnt)->cPtr;
        while (clPtr != dtmP->nullPtr)
            {
            clPnt = clistAddrP (dtmP, clPtr)->pntNum;
            clPtr = clistAddrP (dtmP, clPtr)->nextPtr;
            if (nodeAddrP (dtmP, clPnt)->sPtr == dtmP->nullPnt && nodeAddrP (dtmP, clPnt)->tPtr == dtmP->nullPnt)
                {
                  nodeAddrP(dtmP,lastPnt)->sPtr = clPnt ;
                  lastPnt = clPnt ;
                  nodeAddrP(dtmP,clPnt)->sPtr = clPnt ;
             }
            }
        firstPnt = nextPnt;
        }
    /*
     ** Mark Last Point
     */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Last Point");
    if (lastPnt != dtmP->nullPnt)
        {
        nodeAddrP (dtmP, lastPnt)->sPtr = mark;
        ++(*numMarkedP);
        }
    /*
     ** Cleanup
     */
cleanup:
    /*
     ** Job Completed
     */
    return (ret);
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
BENTLEYDTM_Public int bcdtmObject_findAndFixMissingStmTrianglesDtmObject
(
 BC_DTM_OBJ *dtmP //  Pointer To Circular List DTM Object
 )
    {
    int ret = DTM_SUCCESS, dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
    long pnt, cPtr, cCpnt, cSpnt, cNpnt, antPnt, clkPnt, cStartPnt, polyStartPnt, polyNextPnt;
    long   spnt,npnt,numInternal=0,numExternal=0,numPolyPts,dtmFeature ;
    DTMDirection direction;
    long numConnected, numUnConnected, numPolyHulls, polyStartPoints[1000], startPnt;
    long voidStartPnt, sspnt, snpnt;
    DPoint3d *polyPtsP=nullptr ;
    double area;
    BC_DTM_OBJ *voidP = nullptr;
    DTM_TIN_POINT *pntP;
    BC_DTM_FEATURE *dtmFeatureP;

    // Log Arguments

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Finding And Fixing Missing STM Triangles");
        bcdtmWrite_message (0, 0, 0, "dtmP           = %p", dtmP);
        }

    // Scan Points Looking For Polygon Start Points

    numPolyHulls = 0;
    for (pnt = 0; pnt < dtmP->numPoints; ++pnt)
        {
        pntP = pointAddrP (dtmP, pnt);
        if (nodeAddrP (dtmP, pnt)->sPtr == dtmP->nullPnt)
            {

       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Processing Point %8ld ** %12.5lf %12.5lf %10.4lf",pnt,pointAddrP(dtmP,pnt)->x,pointAddrP(dtmP,pnt)->y,pointAddrP(dtmP,pnt)->z ) ;

            // Check Point Has Connected Points

            if ((cPtr = nodeAddrP (dtmP, pnt)->cPtr) == dtmP->nullPtr)
                {
                bcdtmWrite_message (0, 0, 0, "Pnt %8ld Has No Connected Points", pnt);
                }
            else
                {
                clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, pnt, clkPnt)) < 0) goto errexit;
                if (antPnt == clkPnt)
                    {
                    bcdtmWrite_message (0, 0, 0, "Pnt %8ld Has Only One Connected Point", pnt);
                    }
                else
                    {

                    // Check For Hull Start Point

                    polyStartPnt = dtmP->nullPnt;
                    polyNextPnt = dtmP->nullPnt;

                    // Check For Knot Point As You Cannot Start On A Knot Point

                    numConnected = 0;
                    numUnConnected = 0;
                    do
                        {
                        clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                        if (!bcdtmList_testLineDtmObject (dtmP, antPnt, clkPnt))
                            {
                            ++numUnConnected;
                            }
                        ++numConnected;
                        antPnt = clkPnt;
                        cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                        }
                    while (cPtr != dtmP->nullPtr);

                    //

                    if (dbg == 2) bcdtmWrite_message (0, 0, 0, "numConnected = %8ld", numConnected);

                    // Check For Hull Start Point

                    if (numConnected == 2)
                        {
                        cPtr = nodeAddrP (dtmP, pnt)->cPtr;
                        clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                        if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, pnt, clkPnt)) < 0) goto errexit;
                        if (bcdtmMath_pointSideOfDtmObject (dtmP, clkPnt, antPnt, pnt) == 1)
                            {
                            polyStartPnt = pnt;
                            polyNextPnt = clkPnt;
                            }
                        else
                            {
                            polyStartPnt = pnt;
                            polyNextPnt = antPnt;
                            }
                        }

                    //  Look For Isolated Triangles

                    if (numUnConnected == 1)
                        {
                        cPtr = nodeAddrP (dtmP, pnt)->cPtr;
                        clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                        if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, pnt, clkPnt)) < 0) goto errexit;
                        do
                            {
                            clkPnt = clistAddrP (dtmP, cPtr)->pntNum;
                            if (!bcdtmList_testLineDtmObject (dtmP, antPnt, clkPnt))
                                {
                                polyStartPnt = pnt;
                                polyNextPnt = antPnt;
                                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Poly Start Found ** pnt = %8ld antPnt = %8ld clkPnt = %8ld", pnt, antPnt, clkPnt);
                                }
                            antPnt = clkPnt;
                            cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                            }
                        while (cPtr != dtmP->nullPtr);
                        }

                    // If Poly Start Point Found Insert Hull

                    if (polyStartPnt != dtmP->nullPnt)
                        {
                        if (nodeAddrP (dtmP, polyStartPnt)->hPtr != dtmP->nullPnt)
                            {
                            polyStartPnt = dtmP->nullPnt;
                            }
                        }
                    if (polyStartPnt != dtmP->nullPnt)
                        {
                        //  Insert Tin Hull

                        if (dbg == 1)
                            {
                   bcdtmWrite_message(0,0,0,"polyStartPnt = %8ld ** %12.5lf %12.5lf %10.4lf",polyStartPnt,pointAddrP(dtmP,polyStartPnt)->x,pointAddrP(dtmP,polyStartPnt)->y,pointAddrP(dtmP,polyStartPnt)->z) ;
                   bcdtmWrite_message(0,0,0,"polyNextPnt  = %8ld ** %12.5lf %12.5lf %10.4lf",polyNextPnt,pointAddrP(dtmP,polyNextPnt)->x,pointAddrP(dtmP,polyNextPnt)->y,pointAddrP(dtmP,polyNextPnt)->z) ;
                            }
                        spnt = polyStartPnt;
                        npnt = polyNextPnt;
                        do
                            {

                            // Log Hull Line

                            if (dbg == 2)
                                {
                                bcdtmWrite_message (0, 0, 0, "**** Setting Hull Line ** spnt = %8ld npnt = %8ld", spnt, npnt);
                                pntP = pointAddrP (dtmP, spnt);
                      bcdtmWrite_message(0,0,0,"spnt = %8ld ** %12.5lf %12.5lf %10.4lf",spnt,pntP->x,pntP->y,pntP->z) ;
                                pntP = pointAddrP (dtmP, npnt);
                      bcdtmWrite_message(0,0,0,"npnt = %8ld ** %12.5lf %12.5lf %10.4lf",npnt,pntP->x,pntP->y,pntP->z) ;
                                }

                            // Check For Hull Knot

                            //                  if( npnt != polyStartPnt && nodeAddrP(dtmP,npnt)->hPtr != dtmP->nullPnt )
                            while (npnt != polyStartPnt && nodeAddrP (dtmP, npnt)->hPtr != dtmP->nullPnt)
                                {

                                if (dbg == 2)
                                    {
                                    bcdtmWrite_message (0, 0, 0, "Point %8ld Already Marked As Hull Point", npnt);
                        bcdtmWrite_message(0,0,0,"npnt %8ld ** %12.5lf %12.5lf %10.4lf",npnt,pointAddrP(dtmP,npnt)->x,pointAddrP(dtmP,npnt)->y,pointAddrP(dtmP,npnt)->z) ;
                        bcdtmWrite_message(0,0,0,"spnt %8ld ** %12.5lf %12.5lf %10.4lf",spnt,pointAddrP(dtmP,spnt)->x,pointAddrP(dtmP,spnt)->y,pointAddrP(dtmP,spnt)->z) ;
                                    }

                                // Fix Hull At Knot Point

                                if (bcdtmObject_fixStmHullKnotPointDtmObject (dtmP, npnt, spnt)) goto errexit;

                                // Reset Next Point On Tin Hull

                                if (nodeAddrP (dtmP, npnt)->hPtr != dtmP->nullPnt)
                                    {
                                    if ((npnt = bcdtmList_nextClkDtmObject (dtmP, spnt, npnt)) < 0) goto errexit;
                                    if (dbg == 2) bcdtmWrite_message (0, 0, 0, "npnt = %8ld", npnt);
                                    }

                                }
                            nodeAddrP (dtmP, spnt)->hPtr = npnt;
                            if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, npnt, spnt)) < 0) goto errexit;
                            spnt = npnt;
                            npnt = antPnt;
                            }
                        while (spnt != polyStartPnt);

                        // Mark All Points Internal To Tin Hull

                        polyStartPoints[numPolyHulls] = polyStartPnt;
                        ++numPolyHulls;
                        if (numPolyHulls >= 1000)
                            {
                            bcdtmWrite_message (1, 0, 0, "Too Many Tin Hulls");
                            goto errexit;
                            }

                        // Copy Hptr List To Tptr List

                        if (bcdtmList_copyHptrListToTptrListDtmObject (dtmP, polyStartPnt)) goto errexit;
                        if (bcdtmList_copyTptrListToSptrListDtmObject (dtmP, polyStartPnt)) goto errexit;

                        // Get Polygon Direction

                        if (bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject (dtmP, polyStartPnt, &area, &direction)) goto errexit;
                        if (direction == DTMDirection::Clockwise) ++numInternal;
                        if (direction == DTMDirection::AntiClockwise) ++numExternal;
                        if (direction == DTMDirection::Clockwise) bcdtmList_reverseTptrPolygonDtmObject (dtmP, polyStartPnt);

                        // Store Polygon Boundaries In DTM Object

                        if (voidP == nullptr)
                            {
                            if (bcdtmObject_createDtmObject (&voidP)) goto errexit;
                            }
                        if (bcdtmList_copyTptrListToPointArrayDtmObject (dtmP, polyStartPnt, &polyPtsP, &numPolyPts)) goto errexit;
                        if (bcdtmObject_storeDtmFeatureInDtmObject (voidP, DTMFeatureType::Breakline, (DTMUserTag)direction, 1, &voidP->nullFeatureId, polyPtsP, numPolyPts)) goto errexit;

                        // Null Tptr And Hptr Lists

                        if (bcdtmList_nullTptrListDtmObject (dtmP, polyStartPnt)) goto errexit;
                        spnt = polyStartPnt;
                        do
                            {
                            npnt = nodeAddrP (dtmP, spnt)->hPtr;
                            nodeAddrP (dtmP, spnt)->hPtr = dtmP->nullPnt;
                            spnt = npnt;
                            }
                        while (spnt != polyStartPnt);
                        }
                    }
                }
            }
        }

    // Log Number Of Polygons

    if (dbg == 1)
        {
        bcdtmWrite_message (0, 0, 0, "Number Of Polygons = %8ld", numPolyHulls);
        bcdtmWrite_message (0, 0, 0, "Number Internal    = %8ld", numInternal);
        bcdtmWrite_message (0, 0, 0, "Number External    = %8ld", numExternal);
        }
    if (dbg == 2) bcdtmWrite_geopakDatFileFromDtmObject (voidP, L"stmFixPolygons.dat");

    // Check Hulls Were Detected

    if (numPolyHulls <= 0)
        {
        bcdtmWrite_message (1, 0, 0, "Cannot Fix STM Triangles");
        goto errexit;
        }


    // Set DTM Object Variables Required For Searching Purposes

    dtmP->numSortedPoints = dtmP->numPoints;
    dtmP->dtmState = DTMState::Tin;

    // No Fix Required Just Set Tin Hull

    if (numPolyHulls == 1)
        {
        if (bcdtmList_setConvexHullDtmObject (dtmP)) goto errexit;
        }
    else if (numPolyHulls > 1)
        {

        // Triangulate Poly Hulls

        voidP->ppTol = voidP->plTol = 0.0;
        if (bcdtmObject_triangulateDtmObject (voidP), false, false) goto errexit;
        if (bcdtmList_removeNoneFeatureHullLinesDtmObject (voidP)) goto errexit;
        if (dbg == 2) bcdtmWrite_toFileDtmObject (voidP, L"void.tin");

        // Insert Missing Internal Triangles Into STM Triangles As Void Triangles

        if (numInternal > 0)
            {
            numInternal = 0;
            for (dtmFeature = 0; dtmFeature < voidP->numFeatures; ++dtmFeature)
                {
                dtmFeatureP = ftableAddrP (voidP, dtmFeature);
                if (dtmFeatureP->dtmUserTag == 1) // Internal Feature
                    {
                    if (bcdtmList_copyDtmFeatureToTptrListDtmObject (voidP, dtmFeature, &startPnt)) goto errexit;

                    // Scan Around Tptr And Insert Triangles

                    spnt = startPnt;
                    do
                        {
                        npnt = nodeAddrP (voidP, spnt)->tPtr;
                bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,spnt)->x,pointAddrP(voidP,spnt)->y,&cSpnt) ;
                bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,npnt)->x,pointAddrP(voidP,npnt)->y,&cNpnt) ;
                        cStartPnt = cSpnt;
                        if (!bcdtmList_testLineDtmObject (dtmP, cSpnt, cNpnt))
                            {
                            bcdtmWrite_message (1, 0, 0, "Line Missing From STM Triangles");
                            goto errexit;
                            }
                        nodeAddrP (dtmP, cSpnt)->tPtr = cNpnt;

                        // Insert Fixing Triangles Into Stm Triangles

                        if ((clkPnt = bcdtmList_nextClkDtmObject (voidP, npnt, spnt)) < 0) goto errexit;
                        while (nodeAddrP (voidP, npnt)->tPtr != clkPnt)
                            {
                   bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,clkPnt)->x,pointAddrP(voidP,clkPnt)->y,&cCpnt) ;
                            if (!bcdtmList_testLineDtmObject (dtmP, cNpnt, cCpnt))
                                {
                                if (bcdtmList_insertLineDtmObject (dtmP, cNpnt, cCpnt)) goto errexit;
                                }
                            if ((clkPnt = bcdtmList_nextClkDtmObject (voidP, npnt, clkPnt)) < 0) goto errexit;
                            }

                        // Set Next Point

                        spnt = npnt;

                        }
                    while (spnt != startPnt);

                    // Insert Void Feature Into STM Triangles

               if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,nullptr,0,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->nullFeatureId,cStartPnt,1)) goto errexit ;
                    ++numInternal;

                    }
                }

            // Log

            if (dbg == 1) bcdtmWrite_message (0, 0, 0, "Number Of Internal Voids Inserted = %8ld", numInternal);
            if (dbg == 2) bcdtmWrite_toFileDtmObject (dtmP, L"stmAfterInternal.tin");
            }

        // Insert Missing External Triangles Into STM Triangles As Void Triangles

        if (numExternal > 0)
            {
            numExternal = 0;
            for (dtmFeature = 0; dtmFeature < voidP->numFeatures; ++dtmFeature)
                {
                dtmFeatureP = ftableAddrP (voidP, dtmFeature);
                if (dtmFeatureP->dtmUserTag == 2) // Internal Feature
                    {
                    if (bcdtmList_copyDtmFeatureToTptrListDtmObject (voidP, dtmFeature, &startPnt)) goto errexit;

                    // Scan Around Tptr And Insert Triangles

                    spnt = startPnt;
                    do
                        {
                        npnt = nodeAddrP (voidP, spnt)->tPtr;
                bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,spnt)->x,pointAddrP(voidP,spnt)->y,&cSpnt) ;
                bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,npnt)->x,pointAddrP(voidP,npnt)->y,&cNpnt) ;
                        cStartPnt = cSpnt;
                        if (!bcdtmList_testLineDtmObject (dtmP, cSpnt, cNpnt))
                            {
                            bcdtmWrite_message (1, 0, 0, "Line Missing From STM Triangles");
                            goto errexit;
                            }
                        nodeAddrP (dtmP, cSpnt)->tPtr = cNpnt;

                        // Insert Fixing Triangles Into Stm Triangles

                        if ((clkPnt = bcdtmList_nextClkDtmObject (voidP, spnt, npnt)) < 0) goto errexit;
                        while (nodeAddrP (voidP, clkPnt)->tPtr != spnt)
                            {
                   bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,clkPnt)->x,pointAddrP(voidP,clkPnt)->y,&cCpnt) ;
                            if (!bcdtmList_testLineDtmObject (dtmP, cSpnt, cCpnt))
                                {
                                if (bcdtmList_insertLineDtmObject (dtmP, cSpnt, cCpnt)) goto errexit;
                                }
                            if ((clkPnt = bcdtmList_nextClkDtmObject (voidP, spnt, clkPnt)) < 0) goto errexit;
                            }

                        // Set Next Point

                        spnt = npnt;

                        }
                    while (spnt != startPnt);

                    ++numExternal;
                    }
                }
            if (dbg == 1) bcdtmWrite_message (0, 0, 0, "Number Of External Triangulations Processed = %8ld", numExternal);

            //  Insert Voids Around Tin Hull

            if (dbg == 1) bcdtmWrite_message (0, 0, 0, "Inserting Voids On Tin Hull");
            numExternal = 0;
            bcdtmList_nullSptrValuesDtmObject (voidP);
            bcdtmList_nullTptrValuesDtmObject (dtmP);
            spnt = voidP->hullPoint;
            do
                {
                npnt = nodeAddrP (voidP, spnt)->hPtr;
                if (nodeAddrP (voidP, spnt)->sPtr == dtmP->nullPnt && !bcdtmList_testForBreakLineDtmObject (voidP, spnt, npnt))
                    {
                    voidStartPnt = spnt;

                    // Scan Clockwise To Get Void Boundary

                    sspnt = spnt;
                    snpnt = npnt;
                    nodeAddrP (voidP, sspnt)->sPtr = snpnt;
                    do
                        {
                        if ((clkPnt = bcdtmList_nextClkDtmObject (voidP, snpnt, sspnt)) < 0) goto errexit;
                        while (!bcdtmList_testForBreakLineDtmObject (voidP, snpnt, clkPnt) && nodeAddrP (voidP, snpnt)->hPtr != clkPnt)
                            {
                            if ((clkPnt = bcdtmList_nextClkDtmObject (voidP, snpnt, clkPnt)) < 0) goto errexit;
                            }
                        sspnt = snpnt;
                        snpnt = clkPnt;
                        nodeAddrP (voidP, sspnt)->sPtr = snpnt;
                        }
                    while (sspnt != voidStartPnt);

                    // Check Connectivity Of Inserted Sptr Polygon

                    if (cdbg == 1)
                        {
                        if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Checking Connectivity Of Sptr Polygon");
                        if (bcdtmList_checkConnectivitySptrPolygonDtmObject (voidP, voidStartPnt, 1))
                            {
                            bcdtmWrite_message (2, 0, 0, "Sptr Polygon Connectivity Error");
                            goto errexit;
                            }
                        }

                    // Insert Void Boundary Into STM Triangles

                    startPnt = dtmP->nullPnt;
                    sspnt = voidStartPnt;
                    do
                        {
                        snpnt = nodeAddrP (voidP, sspnt)->sPtr;
                bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,sspnt)->x,pointAddrP(voidP,sspnt)->y,&cSpnt) ;
                bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,snpnt)->x,pointAddrP(voidP,snpnt)->y,&cNpnt) ;
                        if (startPnt == dtmP->nullPnt) startPnt = cSpnt;
                        if (cdbg == 1)
                            {
                            if (!bcdtmList_testLineDtmObject (dtmP, cSpnt, cNpnt))
                                {
                                bcdtmWrite_message (0, 0, 0, "Void Line %8ld %8ld Is Not Connected", cSpnt, cNpnt);
                                bcdtmWrite_message (2, 0, 0, "Void Line Is Not Connected", cSpnt, cNpnt);
                                goto errexit;
                                }
                            }
                        nodeAddrP (dtmP, cSpnt)->tPtr = cNpnt;
                        sspnt = snpnt;
                        }
                    while (sspnt != voidStartPnt);

                    // Check Connectivity Of Inserted Tptr Polygon

                    if (cdbg == 1)
                        {
                        if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Checking Connectivity Of Tptr Polygon");
                        if (bcdtmList_checkConnectivityTptrPolygonDtmObject (dtmP, startPnt, 1))
                            {
                            if (dbg == 2) bcdtmList_writeTptrListDtmObject (dtmP, startPnt);
                            bcdtmWrite_message (2, 0, 0, "Tptr Polygon Connectivity Error");
                            goto errexit;
                            }
                        }

                    // Add Void To STM Triangles

                    if (bcdtmInsert_addDtmFeatureToDtmObject (dtmP, nullptr, 0, DTMFeatureType::Void, dtmP->nullUserTag, dtmP->nullFeatureId, startPnt, 1)) goto errexit;
                    ++numExternal;

                    }

                // Set Next Hull Line

                spnt = npnt;
                }
            while (spnt != voidP->hullPoint);

            if (dbg == 1) bcdtmWrite_message (0, 0, 0, "Number Of External Voids Processed = %8ld", numExternal);
            if (dbg == 2) bcdtmWrite_toFileDtmObject (dtmP, L"stmAfterExternal.tin");

            }

        // Set Tin Hull

        if (dbg == 1) bcdtmWrite_message (0, 0, 0, "Setting Tin Hull");
        spnt = voidP->hullPoint;
    bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,spnt)->x,pointAddrP(voidP,spnt)->y,&cSpnt) ;
        dtmP->hullPoint = cSpnt;
        do
            {
            npnt = nodeAddrP (voidP, spnt)->hPtr;
       bcdtmFind_closestPointDtmObject(dtmP,pointAddrP(voidP,npnt)->x,pointAddrP(voidP,npnt)->y,&cNpnt) ;
            if (!bcdtmList_testLineDtmObject (dtmP, cSpnt, cNpnt))
                {
                bcdtmWrite_message (0, 0, 0, "Hull Line %8ld %8ld Is Not Connected", cSpnt, cNpnt);
                }
            nodeAddrP (dtmP, cSpnt)->hPtr = cNpnt;
            cSpnt = cNpnt;
            spnt = npnt;
            }
        while (spnt != voidP->hullPoint);

        // Set Tin State Varaibles

        dtmP->nextHullPoint = nodeAddrP (dtmP, dtmP->hullPoint)->hPtr;

        // Check Triangulated STM Triangles

        if (cdbg == 2)
            {
            if (dbg) bcdtmWrite_message (0, 0, 0, "Checking DTM Validity");
            if (bcdtmCheck_trianglesDtmObject (dtmP))
                {
                bcdtmWrite_message (1, 0, 0, "DTM Invalid");
                bcdtmList_writeCircularListForPointDtmObject (dtmP, 399);
                bcdtmList_writeCircularListForPointDtmObject (dtmP, 398);
                bcdtmList_writeCircularListForPointDtmObject (dtmP, 573);

                goto errexit;
                }
            if (dbg) bcdtmWrite_message (0, 0, 0, "Checking DTM Valid");
            }
        }

    // Clean Up

cleanup:
    if (polyPtsP != nullptr) free (polyPtsP);
    if (voidP != nullptr) bcdtmObject_destroyDtmObject (&voidP);

    // Return

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Finding And Fixing Missing STM Triangles Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Finding And Fixing Missing STM Triangles Error");
    return (ret);

    // Error Exit

errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmObject_logProblemStmTrianglesDtmObject
(
 BC_DTM_OBJ *dtmP, //  Pointer To Circular List DTM Object
 BC_DTM_OBJ *stmTrgDtmP //  Pointer To Untriangulated STM Triangles Object
 )
    {
    int ret = DTM_SUCCESS, dbg = 0, cdbg = 0;
    long n, dtmFeature, numTrgPts, stmPoints[4], startTime = bcdtmClock ();
    long index, mid, bottom, top, clkPnt, numLines, offset;
 unsigned char *linesP=nullptr ;
 DPoint3d *p3dP,*trgPtsP=nullptr  ;
    BC_DTM_FEATURE *dtmFeatureP;


    // Log Arguments

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Logging Problem STM Triangles");
        }

    // Allocate memory to Mark the circular List

    numLines = dtmP->cListPtr / 8 + 1;
 linesP = ( unsigned char * ) malloc( numLines * sizeof(unsigned char)) ;
 for( n = 0 ; n < numLines ; ++n ) *(linesP+n) = (unsigned char) 0 ;


    // Match STM Triangles To Circular List

    if (dbg) bcdtmWrite_message (0, 0, 0, "Looking For STM Triangles In Circular List");
    for (dtmFeature = 0; dtmFeature < stmTrgDtmP->numFeatures; ++dtmFeature)
        {
        dtmFeatureP = ftableAddrP (stmTrgDtmP, dtmFeature);
        if (bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject (stmTrgDtmP, dtmFeature, &trgPtsP, &numTrgPts)) goto errexit;

        //  Get Point Numbers For STM Triangle

        for (n = 0, p3dP = trgPtsP; p3dP < trgPtsP + numTrgPts; ++n, ++p3dP)
            {

            // Binary Search To Find Related DTM Point Number

            index = -1;
            bottom = 0;
            top = dtmP->numPoints - 1;
       if      ( p3dP->x == pointAddrP(dtmP,bottom)->x && p3dP->y == pointAddrP(dtmP,bottom)->y ) index = bottom ;
       else if ( p3dP->x == pointAddrP(dtmP,top)->x    && p3dP->y == pointAddrP(dtmP,top)->y    ) index = top ;
            else
                {
                while (top - bottom > 1)
                    {
                    mid = (top + bottom) / 2;
             if      ( p3dP->x == pointAddrP(dtmP,mid)->x && p3dP->y == pointAddrP(dtmP,mid)->y ) top = bottom = index = mid ;
             else if ( p3dP->x >  pointAddrP(dtmP,mid)->x || ( p3dP->x == pointAddrP(dtmP,mid)->x && p3dP->y > pointAddrP(dtmP,mid)->y )) bottom = mid ;
                    else top = mid;
                    }
                }

            // Check Point Was Found

            if (index == -1)
                {
                bcdtmWrite_message (1, 0, 0, "Cannot Find DTM Point Number For STM Triangle Vertex");
                goto errexit;
                }
            stmPoints[n] = index;

            //  Check Point Found Has The Correct Coordinates

            if (cdbg)
                {
          if( p3dP->x != pointAddrP(dtmP,stmPoints[n])->x || p3dP->y != pointAddrP(dtmP,stmPoints[n])->y )
                    {
                    bcdtmWrite_message (2, 0, 0, "Incorrect Point Association");
                    goto errexit;
                    }
                }
            }

        // Check STM Triangle Points Are Connected

        if (!bcdtmList_testLineDtmObject (dtmP, stmPoints[0], stmPoints[1]))
            {
            bcdtmWrite_message (0, 0, 0, "STM Triangle Edge %8ld %8ld Is Not Connected", stmPoints[0], stmPoints[1]);
            }
        if (bcdtmTheme_getLineOffsetDtmObject (dtmP, &offset, stmPoints[0], stmPoints[1])) goto errexit;
        bcdtmFlag_setFlag (linesP, offset);
        if (bcdtmTheme_getLineOffsetDtmObject (dtmP, &offset, stmPoints[1], stmPoints[0])) goto errexit;
        bcdtmFlag_setFlag (linesP, offset);

        if (!bcdtmList_testLineDtmObject (dtmP, stmPoints[1], stmPoints[2]))
            {
            bcdtmWrite_message (0, 0, 0, "STM Triangle Edge %8ld %8ld Is Not Connected", stmPoints[1], stmPoints[2]);
            }
        if (bcdtmTheme_getLineOffsetDtmObject (dtmP, &offset, stmPoints[1], stmPoints[2])) goto errexit;
        bcdtmFlag_setFlag (linesP, offset);
        if (bcdtmTheme_getLineOffsetDtmObject (dtmP, &offset, stmPoints[2], stmPoints[1])) goto errexit;
        bcdtmFlag_setFlag (linesP, offset);

        if (!bcdtmList_testLineDtmObject (dtmP, stmPoints[2], stmPoints[3]))
            {
            bcdtmWrite_message (0, 0, 0, "STM Triangle Edge %8ld %8ld Is Not Connected", stmPoints[2], stmPoints[3]);
            }
        if (bcdtmTheme_getLineOffsetDtmObject (dtmP, &offset, stmPoints[2], stmPoints[3])) goto errexit;
        bcdtmFlag_setFlag (linesP, offset);
        if (bcdtmTheme_getLineOffsetDtmObject (dtmP, &offset, stmPoints[3], stmPoints[2])) goto errexit;
        bcdtmFlag_setFlag (linesP, offset);


        // Check Topology

        if ((clkPnt = bcdtmList_nextClkDtmObject (dtmP, stmPoints[0], stmPoints[1])) < 0) goto errexit;
        if (clkPnt != stmPoints[2])
            {
            bcdtmWrite_message (0, 0, 0, "STM Triangle Edge Topology Error");
            }
        if ((clkPnt = bcdtmList_nextClkDtmObject (dtmP, stmPoints[1], stmPoints[2])) < 0) goto errexit;
        if (clkPnt != stmPoints[0])
            {
            bcdtmWrite_message (0, 0, 0, "STM Triangle Edge Topology Error");
            }
        if ((clkPnt = bcdtmList_nextClkDtmObject (dtmP, stmPoints[2], stmPoints[3])) < 0) goto errexit;
        if (clkPnt != stmPoints[1])
            {
            bcdtmWrite_message (0, 0, 0, "STM Triangle Edge Topology Error");
            }

        }

    //  Report Unconnected Lines

    for (n = 0; n < dtmP->cListPtr; ++n)
        {
        if (!bcdtmFlag_testFlag (linesP, n))
            {
            bcdtmWrite_message (0, 0, 0, "Line %8ld Not Marked", n);
            }
        }

    // Clean Up

cleanup:
    if (trgPtsP != nullptr) free (trgPtsP);
    if (linesP != nullptr) free (linesP);

    // Return

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Logging Problem STM Triangles Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Logging Problem STM Triangles Error");
    return (ret);

    // Error Exit

errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

inline int HasFeatureLink (BC_DTM_OBJ* dtmP, long p1, long p2)
    {
    long count1 = 0;
    long count2 = 0;
    bcdtmList_countNumberOfDtmFeaturesForLineDtmObject (dtmP, p2, p1, &count1);
    bcdtmList_countNumberOfDtmFeaturesForLineDtmObject (dtmP, p1, p2, &count2);
    return count1 + count2;
    }

inline int HasFeatureLinkSTM (BC_DTM_OBJ* dtmP, long p1, long p2)
    {
    long count1 = 0;
    bcdtmList_countNumberOfDtmFeaturesForLineDtmObject (dtmP, p2, p1, &count1);
    return count1;
    }

BENTLEYDTM_EXPORT int bcdtmObject_addVoidsToTinHullDtmObject (BC_DTM_OBJ* dtmP)
    {
    int ret = DTM_SUCCESS;
    int dbg = DTM_TRACE_VALUE(0);
    long startPnt = dtmP->hullPoint;

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Adding voids to areas external of features but internal of the hull");
        bcdtmWrite_message (0, 0, 0, "dtmP  = %p", dtmP);

        bcdtmWrite_message (0, 0, 0, "Hull Point = %d", startPnt);
        }

    long pnt = GetNextHullPtr (dtmP, startPnt);
    long prevPnt = startPnt;
    while (pnt != startPnt)
        {
        // If there isnt a feature on this point then we need to find all the links on the edge.
        if (HasFeatureLinkSTM (dtmP, pnt, GetNextHullPtr (dtmP, pnt)))
            {
            prevPnt = pnt;
            pnt = GetNextHullPtr (dtmP, pnt);
            continue;
            }

        // Found hull link with out a feature on it.
        if (dbg)
            bcdtmWrite_message (0, 0, 0, "Found feature with out Hull Point = %d", startPnt);

        bvector<long> pntList;

        // Loop round hull points till we find one that has a feature.
        long endPnt = pnt;
        do
            {
            if (dbg)
                bcdtmWrite_message (0, 0, 0, "Adding Hull Point = %d", pnt);

            pntList.push_back (pnt);
            prevPnt = pnt;
            pnt = GetNextHullPtr (dtmP, pnt);
            }
        while (GetPointFeatureListPtr (dtmP, pnt) == dtmP->nullPtr);

        if (dbg)
            bcdtmWrite_message (0, 0, 0, "Adding Hull Point = %d", pnt);

        pntList.push_back (pnt);
        long pnt1 = pnt;
        long pnt2 = prevPnt;

        // now follow the links back till we get to the hull point.
        while (pnt1 != endPnt)
            {
            long pnt3 = bcdtmList_nextClkDtmObject (dtmP, pnt1, pnt2);

            if (GetNextHullPtr (dtmP, pnt1) != pnt3 && !HasFeatureLinkSTM (dtmP, pnt3, pnt1))
                {
                // Not this link.
                pnt2 = pnt3;
                continue;
                }
            if (dbg)
                bcdtmWrite_message (0, 0, 0, "Adding Link = %d", pnt3);
            pntList.push_back (pnt3);
            pnt2 = pnt1;
            pnt1 = pnt3;
            }

        // Add the void.
        for (size_t i = 0; i < pntList.size() - 1; i++)
            nodeAddrP (dtmP, pntList[i])->tPtr = pntList [i + 1];
        if (bcdtmInsert_addDtmFeatureToDtmObject (dtmP, nullptr, 0, DTMFeatureType::Void, dtmP->nullUserTag, dtmP->nullFeatureId, endPnt, 1)) goto errexit;
        }
cleanup:
    return ret;
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

static int bcdtmObject_addVoidsToInternalDtmObject_AddFeature (BC_DTM_OBJ* dtmP, long startPnt)
    {
    DTMDirection direction = DTMDirection::Unknown;
    double area = 0;

    long numPoints = 0;
    long numHullPoints = 0;
    long p1;
    /*
    ** Get Prior Pointer To firstPnt
    */
    p1 = startPnt;
    do
        {
        numPoints++;
        if (nodeAddrP (dtmP, p1)->hPtr != dtmP->nullPnt)
            numHullPoints++;
        p1 = nodeAddrP (dtmP, p1)->tPtr;
        } while (p1 != startPnt);

    if (numPoints < 3)
        {
        bcdtmList_nullTptrListDtmObject (dtmP, startPnt);
        return DTM_SUCCESS;
        }
    if (bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject (dtmP, startPnt, &area, &direction))
        return DTM_ERROR;

    if (area < 0.0000001)
        {
        bcdtmList_nullTptrListDtmObject (dtmP, startPnt);
        return DTM_SUCCESS;
        }

    if (direction == DTMDirection::AntiClockwise)
        {
        if (bcdtmInsert_addDtmFeatureToDtmObject (dtmP, nullptr, 0, DTMFeatureType::Void, dtmP->nullUserTag, dtmP->nullFeatureId, startPnt, 1))
            return DTM_ERROR;
        }
    else
        {
        if (bcdtmList_reverseTptrPolygonDtmObject (dtmP, startPnt))
            return DTM_ERROR;

        if (numHullPoints * 2 < numPoints)
            {
            if (bcdtmInsert_addDtmFeatureToDtmObject (dtmP, nullptr, 0, DTMFeatureType::Island, dtmP->nullUserTag, dtmP->nullFeatureId, startPnt, 1))
                return DTM_ERROR;
            }
        else
            {
            bcdtmList_nullTptrListDtmObject (dtmP, startPnt);
            }
        }
    return DTM_SUCCESS;
    }

BENTLEYDTM_EXPORT int bcdtmObject_addVoidsToInternalDtmObject (BC_DTM_OBJ* dtmP)
    {
    int ret = DTM_SUCCESS;
    int dbg = DTM_TRACE_VALUE(0);

    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Adding voids to internal STM Triangles");
        bcdtmWrite_message (0, 0, 0, "dtmP  = %p", dtmP);
        }

    // Go through every pnt.
    for (long pnt = 0; pnt < dtmP->numPoints; pnt++)
        {
        // Go through all links.
        long cPtr = nodeAddrP (dtmP, pnt)->cPtr;
        long startPnt = clistAddrP (dtmP, cPtr)->pntNum;

        long linePnt = bcdtmList_nextAntDtmObject (dtmP, pnt, startPnt);
        startPnt = linePnt;
        do
            {
            // Check if there is a valid link here
            if (nodeAddrP (dtmP, pnt)->hPtr == linePnt)
                {
                // Not this link.
                }
            else if (HasFeatureLinkSTM (dtmP, pnt, linePnt) == 1 && HasFeatureLink (dtmP, linePnt, pnt) == 1)
                {
                // if not then we need to create a void.
                long endPnt = linePnt;

                long pnt1 = pnt;
                long pnt2 = linePnt;

                nodeAddrP (dtmP, pnt2)->tPtr = pnt1;
                long pntChk = pnt2;
                // now follow the links back till we get to the start/end point.
                while (pnt1 != endPnt)
                    {
                    long pnt3 = bcdtmList_nextClkDtmObject (dtmP, pnt1, pnt2);

                    if (HasFeatureLink (dtmP, pnt3, pnt1) == 0 && nodeAddrP (dtmP, pnt1)->hPtr != pnt3)
                        {
                        // Not this link.
                        pnt2 = pnt3;
                        if (pntChk == pnt2)
                            break;
                        continue;
                        }
                    nodeAddrP (dtmP, pnt1)->tPtr = pnt3;

                    // If this is now closed
                    if (pnt3 != endPnt && nodeAddrP (dtmP, pnt3)->tPtr != dtmP->nullPnt)
                        {
                        if (bcdtmObject_addVoidsToInternalDtmObject_AddFeature (dtmP, pnt3))
                            goto errexit;
                        pnt2 = pnt3;
                        }

                    pnt2 = pnt1;
                    pnt1 = pnt3;
                    pntChk = pnt2;
                    }
                if (bcdtmObject_addVoidsToInternalDtmObject_AddFeature (dtmP, endPnt))
                    goto errexit;
                }
            linePnt = bcdtmList_nextAntDtmObject (dtmP, pnt, linePnt);
            } while (linePnt != startPnt);
        }
cleanup:
    return ret;
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

BENTLEYDTM_EXPORT int bcdtmObject_triangulateStmTrianglesDtmObject
(
 BC_DTM_OBJ *dtmP //  Pointer To DTM Object
 )
    {
    int ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE(0), cdbg = DTM_CHECK_VALUE(0), tdbg = DTM_TIME_VALUE(0);
    long dtmFeature;
    DPoint3dP trgPtsP = nullptr;
    BC_DTM_FEATURE *dtmFeatureP;
    long numTrgPts;

    // Log Arguments
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Triangulating STM Triangles");
        bcdtmWrite_message (0, 0, 0, "dtmP  = %p", dtmP);
        }

    // Check For Valid DTM Object
    if (bcdtmObject_testForValidDtmObject (dtmP)) goto errexit;

    // Check For DTM Data State
    if (dtmP->dtmState != DTMState::Data)
        {
        bcdtmWrite_message (1, 0, 0, "Method Requires Untriangulated DTM");
        goto errexit;
        }

    // Check For Presence Of Features
    if (dtmP->numFeatures == 0)
        {
        bcdtmWrite_message (1, 0, 0, "No Triangles Present In DTM");
        goto errexit;
        }

    // Log Number Of Triangles
    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of STM Triangles = %8ld", dtmP->numFeatures);

    // Validate the features are valid triangles.
    for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
        {
        dtmFeatureP = ftableAddrP (dtmP, dtmFeature);

        // Check For None Graphic Break Feature
        if (dtmFeatureP->dtmFeatureType != DTMFeatureType::GraphicBreak)
            {
            bcdtmWrite_message (1, 0, 0, "None STM Triangle Features Present In DTM");
            goto errexit;
            }

        // Check For Correct Number Of Points
        if (dtmFeatureP->numDtmFeaturePts != 4)
            {
            bcdtmWrite_message (1, 0, 0, "Incorrect Number Of Points For STM Triangle");
            goto errexit;
            }

        // Get Triangle Points
        if (bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject (dtmP, dtmFeature, &trgPtsP, &numTrgPts)) goto errexit;

        // Check Triangle Closes
        if (trgPtsP->x != (trgPtsP + 3)->x || trgPtsP->y != (trgPtsP + 3)->y)
            {
            bcdtmWrite_message (1, 0, 0, "STM Triangle Does Not Close");
            goto errexit;
            }

        // Check Elevations
        if (trgPtsP->z != (trgPtsP + 3)->z)
            {
            bcdtmWrite_message (1, 0, 0, "STM Triangle Start And End Point Elevations Are Not The Same");
            goto errexit;
            }

        if (bcdtmMath_sideOf (trgPtsP->x, trgPtsP->y, (trgPtsP + 2)->x, (trgPtsP + 2)->y, (trgPtsP + 1)->x, (trgPtsP + 1)->y) == 0)
            {
            bcdtmWrite_message (1, 0, 0, "Colinear STM Triangle");
            goto errexit;
            }
        }

    // Change all Graphic Breaks to hard breaks
    // ToDo bcdtmData_changeAllOccurrencesOfDtmFeatureTypeDataObject
    for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; dtmFeature++)
        {
        dtmFeatureP = ftableAddrP (dtmP, dtmFeature);

        // Check For None Graphic Break Feature
        dtmFeatureP->dtmFeatureType = DTMFeatureType::Breakline;
        }

    // Triangulate DTM
    dtmP->edgeOption = 3;
    double prevMaxSide = dtmP->maxSide;
    dtmP->maxSide = dtmP->ppTol;
    dtmP->ppTol = 0;
    dtmP->plTol = 0;
    bcdtmObject_triangulateDtmObject (dtmP, false, false);
    dtmP->maxSide = prevMaxSide;

    if (dbg)
        {
        for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; dtmFeature++)
            {
            // Check For None Graphic Break Feature
            long numPts, closed;
            bcdtmList_countNumberOfDtmFeaturePointsDtmObject (dtmP, dtmFeature, &numPts, &closed);

            if (!closed && numPts != 4)
                bcdtmWrite_message (0, 0, 0, "Triangles must have crossed %i", dtmFeature);
            }
        }

    // Remove as much tin edges as we can.
    bcdtmList_removeNoneFeatureHullLinesDtmObject (dtmP);

    // Add voids to hull.
    //bcdtmObject_addVoidsToTinHullDtmObject (dtmP);

    // Add internal voids.
    bcdtmObject_addVoidsToInternalDtmObject (dtmP);

    // Delete all breaklines.
    bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject (dtmP, DTMFeatureType::Breakline);

    // Remove the deleted flag from the nodes.
    for (int i = 0; i < dtmP->numNodes; i++)
        {
        nodeAddrP (dtmP, i)->PCWD = 0;
        nodeAddrP (dtmP, i)->PRGN = 0;
        }

    if( dbg )bcdtmWrite_message(0,0,0,"Compacting Circular List") ;
    if( bcdtmTin_compactCircularListDtmObject(dtmP)  ) goto errexit ;
    if( dbg )bcdtmWrite_message(0,0,0,"Compacting Feature Table") ;
    if( bcdtmTin_compactFeatureTableDtmObject(dtmP)) goto errexit ;
    if( dbg )bcdtmWrite_message(0,0,0,"Compacting Feature List") ;
    if( bcdtmTin_compactFeatureListDtmObject(dtmP) ) goto errexit ;

    if( dbg )bcdtmWrite_message(0,0,0,"Resizing Tin Memory") ;
    if( bcdtmObject_resizeMemoryDtmObject(dtmP) ) goto errexit ;

    cleanup:
    if (trgPtsP != nullptr) free (trgPtsP);

    // Return

    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Triangulating STM Triangles Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Triangulating STM Triangles Error");
    return (ret);

    // Error Exit

errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }
