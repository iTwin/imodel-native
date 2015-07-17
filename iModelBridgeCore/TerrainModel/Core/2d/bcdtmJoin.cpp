/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmJoin.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 

/*==============================================================================*//**
* @memo   Join DTM Feature Type Occurrences In A Data File
* @doc    Join DTM Feature Type Occurrences In A Data File 
* @notes  This is a total rewrite of the feature joining functions
* @notes  initially written in 1993 and enhanced since then.
* @author Rob Cormack 18 June 2004 rob.cormack@bentley,com
* @param  dataFileName       ==> Input Data File Name  
* @param  joinFileName       ==> Output Join File Name 
* @param  dtmFeatureType     ==> DTM feature type to be joined 
* @param  tolerance          ==> Join tolerance 
* @param  numBeforeJoinP     <== Number of DTM feature type occurrences before the join 
* @param  numAfterJoinP      <== Number of DTM feature type occurrences after  the join 
* @return DTM_SUCCESS or DTM_ERROR
* @version 
* @see None
*===============================================================================*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmJoin_dtmFeatureTypeGeopakDatFile
(
 WCharCP dataFileName,      /* ==> Input Data File Name                                */
 WCharCP joinFileName,      /* ==> Output Join File Name                               */
 DTMFeatureType dtmFeatureType,        /* ==> DTM feature type to be joined                       */
 DTMFeatureType joinFeatureType,       /* ==> DTM Joinfeature type                                */
 double tolerance,           /* ==> Join tolerance                                      */ 
 long *numBeforeJoin,        /* <== Number of DTM feature type occurrences before join  */
 long *numAfterJoin          /* <== Number of DTM feature type occurrences after join   */
)
/*
** This the controlling function for joining DTM feature type occurrences
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    joinDataFileType=0,joinDataFileNumDecPts=0,numJoinUserTags=0;
 BC_DTM_OBJ *dataP=NULL ;
 DTM_JOIN_USER_TAGS *joinUserTagsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Joining Linear Features") ; 
/*
** Read Data File into Data Object
*/
 if( bcdtmRead_geopakDatFileToDtmObject(&dataP,dataFileName) != DTM_SUCCESS ) goto errexit ;
/*
** Set Data File Type 
*/
 joinDataFileType      =  DTM_LAST_DAT_FILE_TYPE ;
 joinDataFileNumDecPts =  DTM_LAST_DAT_FILE_NUM_DEC_PTS ; 
/*
** Join Features
*/
 if( bcdtmJoin_dtmFeatureTypeDtmObject(dataP,tolerance,dtmFeatureType,joinFeatureType,numBeforeJoin,numAfterJoin,&joinUserTagsP,&numJoinUserTags) != DTM_SUCCESS ) goto errexit ;
/*
** Write Joined Features To File
*/
 if( joinDataFileType == 1 ) 
   {
    if( bcdtmWrite_asciiGeopakDatFileFromDtmObject(dataP,joinFileName,joinDataFileNumDecPts) != DTM_SUCCESS ) goto errexit ;
   } 
 else if( bcdtmWrite_geopakDatFileFromDtmObject(dataP,joinFileName) != DTM_SUCCESS ) goto errexit ;
/*
** Set Current Data Object To The Joined Data Object
*/
// if( DTM_CDTM != NULL ) bcdtmObject_destroyDtmObject(&DTM_CDTM) ;
// DTM_CDTM = dataP ; 
// wcscpy(DTM_CDTM_FILE,joinFileName)  ;
/*
** Clean Up 
*/
 cleanup :
 if( joinUserTagsP != NULL ) free(joinUserTagsP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Joining Linear Features Complete") ; 
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Joining Linear Features Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dataP != NULL ) bcdtmObject_destroyDtmObject(&dataP) ;
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Join DTM Feature Type Occurrences In A Data Object
* @doc    Join DTM Feature Type Occurrences In A Data Object 
* @notes  1. This is a total rewrite of the feature joining functions
* @notes  initially written in 1993 and enhanced since then.
* @notes  2. Only DTM feature occurrences with the same user tag will be joined.
* @notes  3. The function will optionally change the DTM feature type of joined 
* @notes  features if the "joinedFeatureType" is set differently to the "dtmFeatureType".
* @notes  Normally the "joinedFeatureType" should be set to same value as the "dtmFeatureType".
* @author Rob Cormack 18 June 2004 rob.cormack@bentley,com
* @param  *dataP                ==>  Pointer to a DTM Data Object                 
* @param  tolerance             ==>  Joining tolerance                         
* @param  dtmFeatureType        ==>  DTM feature type to be joined             
* @param  joinedFeatureType     ==>  DTM feature type of joined features            
* @param  numBeforeJoinP        <==  Number of DTM feature type occurrences before the join 
* @param  numAfterJoinP         <==  Number of DTM feature type occurrences after  the join 
* @return DTM_SUCCESS or DTM_ERROR
* @version 
* @see None
*===============================================================================*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmJoin_dtmFeatureTypeWithRollbackDtmObject
(
 BC_DTM_OBJ *dataP,                   /* ==>  Pointer to a DTM Object                                */
 double tolerance,                    /* ==>  Joining tolerance                                      */ 
 DTMFeatureType dtmFeatureType,                 /* ==>  DTM feature type to be joined                          */
 DTMFeatureType joinFeatureType,                /* ==>  DTM feature type of joined features                    */
 long *numBeforeJoinP,                /* <==  Number of DTM feature type occurrences before the join */
 long *numAfterJoinP,                 /* <==  Number of DTM feature type occurrences after  the join */
 DTM_JOIN_USER_TAGS **joinUserTagsPP, /* <==  User Tag sequence For Joined Lines                     */
 long *numJoinUserTagsP,              /* <==  Size Of User Tag sequence                              */      
 int useRollBack                      /* <==  UseRollBack or not                                     */
)
/*
** This the controlling function for joining DTM features in a Dtm object.
*/
{
 int          ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long         dtmFeature,numFeatures,numFeaturePts,numFeatureTable,numNodeTable,numLineTable ;
 long         joinContours ;
 DPoint3d          *featurePtsP=NULL ;
 BC_DTM_OBJ             *joinP=NULL;
 BC_DTM_FEATURE         *dtmFeatureP ;
 DTM_JOIN_FEATURE_TABLE *ftP,*featureTableP=NULL ;
 DTM_JOIN_NODE_TABLE    *nodeTableP=NULL ;
 DTM_JOIN_LINE_TABLE    *lineTableP=NULL ;
 struct DTM_ROLLBACK_DATA* rollBackInfo = NULL;
 /*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Joining DTM Features with RollBack") ;
    bcdtmWrite_message(0,0,0,"dataP           = %p",dataP) ;
    bcdtmWrite_message(0,0,0,"tolerance       = %8.4lf",tolerance) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType  = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"joinFeatureType = %8ld",joinFeatureType) ;
    bcdtmWrite_message(0,0,0,"useRollBack     = %d",useRollBack) ;
   }
/*
** Initialise varaiables
*/
 *numBeforeJoinP   = 0 ;
 *numAfterJoinP    = 0 ;
 *numJoinUserTagsP = 0 ;
 if( *joinUserTagsPP != NULL ) { free(*joinUserTagsPP) ; *joinUserTagsPP = NULL ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dataP)) goto errexit ;
/*
** Check DTM Is In Data State
*/
 if( dataP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(2,0,0,"DTM Not In Data State") ;
    goto errexit ;
   }
/*
** Check For Valid Dtm Feature Type
*/
 if( bcdtmData_testForValidDtmObjectImportFeatureType(dtmFeatureType))
   {
    bcdtmWrite_message(2,0,0,"Invalid Dtm Feature Type") ;
    goto errexit ;
   }
/*
** Check Tolerance
*/
 if( tolerance < 0.0 )
   {
    bcdtmWrite_message(1,0,0,"Invalid Join Tolerance Value") ;
    goto errexit ;
   }

 if (useRollBack)
     {
     rollBackInfo = dataP->extended ? dataP->extended->rollBackInfoP : NULL;

     if (!rollBackInfo)
         {
         bcdtmWrite_message(1,0,0,"Rollback asked for in Join but no RollBack information.") ;
         goto errexit ;
         }
     }
/*
** Count Number Of Dtm Features
*/
 numFeatures   = 0 ;
 numFeaturePts = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dataP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dataP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == dtmFeatureType )
      {
       ++numFeatures ;
       numFeaturePts = numFeaturePts + dtmFeatureP->numDtmFeaturePts ;
      }
   }
 *numBeforeJoinP = numFeatures ;
 *numAfterJoinP  = numFeatures ;
/*
** Write Feature Statistics
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Number Of Features       = %10ld",numFeatures) ; 
    bcdtmWrite_message(0,0,0,"Number Of Feature Points = %10ld",numFeaturePts) ; 
   }
/*
** Join DTM Features
*/
 if( numFeatures > 1 ) 
   {
/*
**  Create Data Object To Store Features To Be Joined
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Join Data Object") ;
    if( bcdtmObject_createDtmObject(&joinP)  != DTM_SUCCESS ) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(joinP,numFeaturePts,10) ;
/*
**  Copy Dtm Features To Join
*/
    for( dtmFeature = 0 ; dtmFeature < dataP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dataP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureType == dtmFeatureType )
         {
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dataP,dtmFeature,&featurePtsP,&numFeaturePts) ) goto errexit ;
//          if( bcdtmObject_storeDtmFeatureInDtmObject(joinP,dtmFeatureType,DTM_NULL_USER_TAG,1,&nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(joinP,dtmFeatureType,dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
          if (rollBackInfo != NULL && bcdtmInsert_rollBackDtmFeatureDtmObject (dataP, dtmFeatureP->dtmFeatureId)) goto errexit;
          if( bcdtmInsert_removeDtmFeatureFromDtmObject(dataP,dtmFeature)) goto errexit ;
         }
      }
/*
**   Remove Deleted Features
*/
    if( bcdtmTin_compactFeatureTableDtmObject(dataP)) goto errexit ;
/*
**   Build Join Feature Table
*/ 
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Join Feature Table") ;
    if( bcdtmJoin_buildJoinFeatureTableFromDtmObject(joinP,dtmFeatureType,&featureTableP,&numFeatureTable) != DTM_SUCCESS ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Features = %10ld",numFeatureTable) ;
/*
**  Build Join Node Table
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Join Node Table") ;
    if( bcdtmJoin_buildJoinNodeTable(featureTableP,numFeatureTable,&nodeTableP,&numNodeTable)  != DTM_SUCCESS ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Nodes    = %10ld",numNodeTable) ;
/*
**  Join Nodes In Node Table
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Joining Nodes In Node Table") ;
    if( dtmFeatureType == DTMFeatureType::ContourLine ) joinContours = TRUE ;  
    else                                      joinContours = FALSE ;
    if( bcdtmJoin_nodes(nodeTableP,numNodeTable,joinContours,tolerance) != DTM_SUCCESS ) goto errexit ;
/*
**  Build Line Table From Node Table
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Join Line Table") ;
    if( bcdtmJoin_buildJoinLineTable(nodeTableP,numNodeTable,&lineTableP,&numLineTable,numAfterJoinP) != DTM_SUCCESS ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Lines    = %10ld",numLineTable) ;
/*
**  Increment Number Of Strings After Join By the Number Of Closed Strings
*/
    for( ftP = featureTableP ; ftP < featureTableP + numFeatureTable ; ++ftP )
      {
       if( ftP->closeFlag ) ++*numAfterJoinP ;
      }
/*
**  Save Number Of Points In Data Object For Latter Change Of DTM Feature Type
*/
    numFeatures = dataP->numFeatures ;
/*
**  Append Joined Features To Data Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Appending Joined Line Strings To Data Object") ;
    if( bcdtmJoin_appendJoinedLineStringsToDtmObject(dataP,joinP,dtmFeatureType,tolerance,featureTableP,numFeatureTable,lineTableP,numLineTable,joinUserTagsPP,numJoinUserTagsP) != DTM_SUCCESS ) goto errexit ;
   }
/*
** Change DTM Feature Types
*/
 if( joinFeatureType != dtmFeatureType )
   {
    for( dtmFeature = numFeatures ; dtmFeature < dataP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dataP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureType == dtmFeatureType ) dtmFeatureP->dtmFeatureType = (DTMFeatureType)joinFeatureType ;
      }
   }
/*
**  Update Modified Time
 */
 if (*numBeforeJoinP != *numAfterJoinP)
   _time32(&dataP->modifiedTime) ;
/*
** Clean Up
*/
 cleanup :
 if( joinP         != NULL ) bcdtmObject_destroyDtmObject(&joinP) ;
 if( nodeTableP    != NULL ) { free(nodeTableP)    ; nodeTableP    = NULL ; }
 if( lineTableP    != NULL ) { free(lineTableP)    ; lineTableP    = NULL ; }
 if( featureTableP != NULL ) { free(featureTableP) ; featureTableP = NULL ; }
 if( featurePtsP   != NULL ) { free(featurePtsP)   ; featurePtsP   = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Joining DTM Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Joining DTM Features Error") ;
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
BENTLEYDTM_EXPORT int bcdtmJoin_dtmFeatureTypeDtmObject
(
 BC_DTM_OBJ *dataP,                   /* ==>  Pointer to a DTM Object                                */
 double tolerance,                    /* ==>  Joining tolerance                                      */ 
 DTMFeatureType dtmFeatureType,                 /* ==>  DTM feature type to be joined                          */
 DTMFeatureType joinFeatureType,                /* ==>  DTM feature type of joined features                    */
 long *numBeforeJoinP,                /* <==  Number of DTM feature type occurrences before the join */
 long *numAfterJoinP,                 /* <==  Number of DTM feature type occurrences after  the join */
 DTM_JOIN_USER_TAGS **joinUserTagsPP, /* <==  User Tag sequence For Joined Lines                     */
 long *numJoinUserTagsP               /* <==  Size Of User Tag sequence                              */      
 )
    {
    return bcdtmJoin_dtmFeatureTypeWithRollbackDtmObject (dataP, tolerance, dtmFeatureType, joinFeatureType, numBeforeJoinP, numAfterJoinP, joinUserTagsPP, numJoinUserTagsP, 0);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmJoin_buildJoinFeatureTableFromDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,DTM_JOIN_FEATURE_TABLE **featureTablePP,long *numFeatureTableP)
/*
** This Function Builds The Join Feature Table
*/
{
 int     ret=DTM_SUCCESS ;
 long    dtmFeature,numFeaturePts ;
 DPoint3d     *featurePtsP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_JOIN_FEATURE_TABLE    *ftP ;
/*
** Count Number Of Features 
*/
 *numFeatureTableP = dtmP->numFeatures * 2 ;
/*
** Allocate Memory For Feature Table
*/
 *featureTablePP = ( DTM_JOIN_FEATURE_TABLE *) malloc(*numFeatureTableP * sizeof(DTM_JOIN_FEATURE_TABLE)) ;
 if( *featureTablePP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Store Values in Feature Table
*/
 ftP = *featureTablePP - 1 ;
 *numFeatureTableP = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == dtmFeatureType )
      {
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts) ) goto errexit ;
/*
**     Populate Table
*/
       ++ftP  ;
       ftP->featureOfs  = *numFeatureTableP ;
       ftP->startOfs    = dtmFeature ;
       ftP->closeFlag   = FALSE ;
       ftP->userTag     = dtmFeatureP->dtmUserTag ;
       ftP->Xs          = featurePtsP->x ;
       ftP->Ys          = featurePtsP->y ;
       ftP->Zs          = featurePtsP->z ;
       ftP->endOfs      = numFeaturePts  ;
       ftP->Xe          = (featurePtsP+numFeaturePts-1)->x ;
       ftP->Ye          = (featurePtsP+numFeaturePts-1)->y ;
       ftP->Ze          = (featurePtsP+numFeaturePts-1)->z ;
       ++*numFeatureTableP ;
/*
**     Free Feature Points
*/
       if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
      }
   }
/*
** Mark Closing Features
*/
 for ( ftP = *featureTablePP ; ftP < *featureTablePP + *numFeatureTableP ; ++ftP )
   {
    if( ftP->Xs == ftP->Xe && ftP->Ys == ftP->Ye ) ftP->closeFlag = TRUE ;
   } 
/*
** Clean up
*/
 cleanup: 
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
BENTLEYDTM_Private int bcdtmJoin_buildJoinNodeTable(DTM_JOIN_FEATURE_TABLE *featureTableP,long numFeatureTable,DTM_JOIN_NODE_TABLE **nodeTablePP,long *numNodeTableP)
/*
** This function builds the node table for feature joining
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  ofs ;
 DTM_JOIN_FEATURE_TABLE *ftP ;
 DTM_JOIN_NODE_TABLE    *ntP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Join Node Table") ;
/*
** Set Number Of Node Entries 
*/
 *numNodeTableP = numFeatureTable * 2  ;
/*
** Allocate Memory For Node Table
*/
 *nodeTablePP = ( DTM_JOIN_NODE_TABLE *) malloc(*numNodeTableP * sizeof(DTM_JOIN_NODE_TABLE)) ;
 if( *nodeTablePP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Populate Node Table
** typedef struct { long  node,featureOfs,sortOfs,direction,joinNode ; DTMUserTag userTag ; double x,y,z ; } DTM_JOIN_NODE_TABLE ; 
*/
 ntP = *nodeTablePP ;
 for( ftP = featureTableP ; ftP < featureTableP + numFeatureTable ; ++ftP )
   {
    if( ftP->closeFlag == FALSE )
      {
       ntP->node       = (long)(ntP-*nodeTablePP)  ; 
       ntP->featureOfs = (long)(ftP-featureTableP) ;
       ntP->sortOfs    = 0 ;
       ntP->direction  = 1 ;
       ntP->joinNode   = DTM_NULL_PNT ;
       ntP->userTag    = ftP->userTag ;
       ntP->x          = ftP->Xs ;
       ntP->y          = ftP->Ys ;
       ntP->z          = ftP->Zs ;
       ++ntP ;
       ntP->node       = (long)(ntP-*nodeTablePP)  ; 
       ntP->featureOfs = (long)(ftP-featureTableP) ;
       ntP->sortOfs    = 0 ;
       ntP->direction  = 2 ;
       ntP->joinNode   = DTM_NULL_PNT ;
       ntP->userTag    = ftP->userTag ;
       ntP->x          = ftP->Xe ;
       ntP->y          = ftP->Ye ;
       ntP->z          = ftP->Ze ;
       ++ntP ;
      }
   }
/*
** Reallocate Memory
*/
 if((long)(ntP-*nodeTablePP) < *numNodeTableP )
   {
    *numNodeTableP = (long)(ntP-*nodeTablePP) ;
        *nodeTablePP = ( DTM_JOIN_NODE_TABLE *) realloc(*nodeTablePP,*numNodeTableP * sizeof(DTM_JOIN_NODE_TABLE)) ;
   }
/*
** QSort Node Table
*/
 qsortCPP(*nodeTablePP,*numNodeTableP,sizeof(DTM_JOIN_NODE_TABLE),bcdtmJoin_nodeTableCompareFunction) ;
/*
** Set Sort Offsets
*/
 for( ofs = 0 , ntP = *nodeTablePP ; ofs < *numNodeTableP ; ++ofs , ++ntP )
   {
    (*nodeTablePP+ntP->node)->sortOfs = ofs ;
   }
/*
** Clean up
*/
 cleanup: 
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Join Node Table Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Join Node Table Error") ;
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
BENTLEYDTM_Private int bcdtmJoin_nodeTableCompareFunction(const DTM_JOIN_NODE_TABLE *n1P,const DTM_JOIN_NODE_TABLE *n2P)
/*
** Compare function for qsort of join node table
*/
{
 if     (  n1P->x < n2P->x ) return(-1) ;
 else if(  n1P->x > n2P->x ) return( 1) ;
 else if(  n1P->y < n2P->y ) return(-1) ;
 else if(  n1P->y > n2P->y ) return( 1) ;
 else                        return( 0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmJoin_nodes(DTM_JOIN_NODE_TABLE *nodeTableP,long numNodeTable,long joinContours,double tolerance)
/*
** This function joins nodes in the node table
*/
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long      process=1,numJoinedNodes=0 ; 
 DTM_JOIN_NODE_TABLE *nodeP,*cnodeP,*ccnodeP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Joining Nodes") ;
/*
**  Get closest node within tolerance to current node
*/
 while ( process )
   {
    process = 0 ;
    for( nodeP = nodeTableP ; nodeP < nodeTableP + numNodeTable ; ++nodeP )
      {
       if( nodeP->joinNode == DTM_NULL_PNT )
         {
          bcdtmJoin_findClosestNode(nodeTableP,numNodeTable,nodeP,joinContours,tolerance,&cnodeP) ;
          if( cnodeP != NULL )
            {
             bcdtmJoin_findClosestNode(nodeTableP,numNodeTable,cnodeP,joinContours,tolerance,&ccnodeP) ;
             if( ccnodeP == nodeP )
               {
                process = 1 ;
                nodeP->joinNode  = (long)(cnodeP-nodeTableP) ;
                cnodeP->joinNode = (long)(nodeP-nodeTableP) ;
                ++numJoinedNodes ;
               }
            } 
	     }
      }
   }
/*
** Write Number Of Nodes Joined
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Joined Nodes = %10ld",numJoinedNodes) ;
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Joining Nodes Completed") ;
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmJoin_findClosestNode(DTM_JOIN_NODE_TABLE *nodeTableP,long numNodeTable,DTM_JOIN_NODE_TABLE *nodeP,long joinContours,double tolerance,DTM_JOIN_NODE_TABLE **closestNodePP)
/*
** This routine finds the closest node to the current node
*/
{
 long    firstCompare    ;
 double  dn=0.0,dd    ;
 DTM_JOIN_NODE_TABLE *snodeP ;
/*
** Initialise Variables
*/
 *closestNodePP = NULL ;
/*
** Not Joining Contours
*/
 if( joinContours == FALSE )
   { 
/*
** Scan Back Until x - x point value  > tolerance
*/
    firstCompare = 1 ; 
    for( snodeP = nodeP ; snodeP >= nodeTableP && nodeP->x - snodeP->x <= tolerance ; --snodeP )
      {
       if( snodeP->userTag == nodeP->userTag && snodeP->joinNode == DTM_NULL_PNT && snodeP->featureOfs != nodeP->featureOfs )
         {
          if( ( dd = bcdtmMath_distance(snodeP->x,snodeP->y,nodeP->x,nodeP->y) ) <= tolerance )
	        {
	         if( firstCompare || dd < dn ) 
               { 
                firstCompare = 0 ; 
                dn = dd ; 
                *closestNodePP = snodeP ; 
               }
	        }
         }
      }
/*
** Scan Fowards Until x - x  point value > tolerance
*/
    for( snodeP = nodeP ; snodeP < nodeTableP + numNodeTable && snodeP->x - nodeP->x <= tolerance ; ++snodeP )
      {
       if( snodeP->userTag == nodeP->userTag && snodeP->joinNode == DTM_NULL_PNT && snodeP->featureOfs != nodeP->featureOfs )
         {
          if( ( dd = bcdtmMath_distance(snodeP->x,snodeP->y,nodeP->x,nodeP->y) ) <= tolerance )
	        {
	         if( firstCompare || dd < dn ) 
               { 
                firstCompare = 0 ; 
                dn = dd ; 
                *closestNodePP = snodeP ; 
               }
	        }
         }
      }
   }
/*
**  Joining Contours
*/
 if( joinContours == TRUE )
   { 
/*
** Scan Back Until x - x point value  > tolerance
*/
    firstCompare = 1 ; 
    for( snodeP = nodeP ; snodeP >= nodeTableP && nodeP->x - snodeP->x <= tolerance ; --snodeP )
      {
       if( snodeP->z == nodeP->z && snodeP->userTag == nodeP->userTag && snodeP->joinNode == DTM_NULL_PNT && snodeP->featureOfs != nodeP->featureOfs )
         {
          if( ( dd = bcdtmMath_distance(snodeP->x,snodeP->y,nodeP->x,nodeP->y) ) <= tolerance )
	        {
	         if( firstCompare || dd < dn ) 
               { 
                firstCompare = 0 ; 
                dn = dd ; 
                *closestNodePP = snodeP ; 
               }
	        }
         }
      }
/*
** Scan Fowards Until x - x  point value > tolerance
*/
    for( snodeP = nodeP ; snodeP < nodeTableP + numNodeTable && snodeP->x - nodeP->x <= tolerance ; ++snodeP )
      {
       if( snodeP->z == nodeP->z && snodeP->userTag == nodeP->userTag && snodeP->joinNode == DTM_NULL_PNT && snodeP->featureOfs != nodeP->featureOfs )
         {
          if( ( dd = bcdtmMath_distance(snodeP->x,snodeP->y,nodeP->x,nodeP->y) ) <= tolerance )
	        {
	         if( firstCompare || dd < dn ) 
               { 
                firstCompare = 0 ; 
                dn = dd ; 
                *closestNodePP = snodeP ; 
               }
	        }
         }
      }
   }
/*
**  Job Completed
*/
 return(0)    ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmJoin_buildJoinLineTable(DTM_JOIN_NODE_TABLE *nodeTableP,long numNodeTable,DTM_JOIN_LINE_TABLE **lineTablePP,long *numLineTableP,long *numJoinedLinesP)
/*
** This function builds the line table from the node table
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long node1,node2,lineNum,memLines,memLinesInc=1000 ;
 long node,snode,startNode,lastNode ;
 DTM_JOIN_NODE_TABLE *nodeP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Building Join Line Table") ;
/*
** Initialise
*/
 lineNum = 0 ;
 *numLineTableP = 0 ;
 *numJoinedLinesP = 0 ;
 memLines = numNodeTable / 2 ;
/*
** Allocate Initial Memory For Lines Table
*/
 *lineTablePP = ( DTM_JOIN_LINE_TABLE * ) malloc ( memLines * sizeof(DTM_JOIN_LINE_TABLE)) ;
 if( *lineTablePP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Get All Non Joining Lines
*/
 for( nodeP = nodeTableP ; nodeP < nodeTableP + numNodeTable ; nodeP = nodeP + 2 )
   {
    node1 = nodeP->sortOfs ;
    node2 = ( nodeP+1)->sortOfs ;
/*
**  Check Both Nodes Point To Same Line
*/
    if( cdbg )
      { 
       if( (nodeTableP+node1)->featureOfs != (nodeTableP+node2)->featureOfs )
         {
          bcdtmWrite_message(0,0,0,"Node Feature Offsets Not The Same") ;
          bcdtmWrite_message(0,0,0,"Node1[%10ld] ** Feature Offset = %10ld",node1,(nodeTableP+node1)->featureOfs) ;
          bcdtmWrite_message(0,0,0,"Node2[%10ld] ** Feature Offset = %10ld",node2,(nodeTableP+node2)->featureOfs) ;
         }
      }
/*
**  Check For Non Joining Line
*/
    if( (nodeTableP+node1)->joinNode == DTM_NULL_PNT && (nodeTableP+node2)->joinNode == DTM_NULL_PNT )
      {
/*
**  Check For Memory Reallocation
*/
       if( *numLineTableP == memLines )
         {
          memLines = memLines + memLinesInc ;
          *lineTablePP = ( DTM_JOIN_LINE_TABLE * ) realloc ( *lineTablePP,memLines * sizeof(DTM_JOIN_LINE_TABLE)) ;
          if( *lineTablePP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
         }
/*
**  Store Line In Line Table
*/
       (*lineTablePP+*numLineTableP)->lineNum    = lineNum ; 
       (*lineTablePP+*numLineTableP)->featureOfs = (nodeTableP+node1)->featureOfs ; 
       (*lineTablePP+*numLineTableP)->direction  = 1 ;
       ++*numLineTableP ;
       ++lineNum ;
/*
**  Mark Non Joining Nodes As Being Processed
*/
       (nodeTableP+node1)->joinNode = -DTM_NULL_PNT ;
       (nodeTableP+node2)->joinNode = -DTM_NULL_PNT ;
      } 
   }
/*
** Get All Joining Lines
*/
 for( nodeP = nodeTableP ; nodeP < nodeTableP + numNodeTable ; nodeP = nodeP + 2 )
   {
    node1 = nodeP->sortOfs ;
    node2 = (nodeP+1)->sortOfs ;
//    bcdtmWrite_message(0,0,0,"node1 = %10ld node2 = %10ld",node1,node2) ;         
/*
**  Check For Joining Line
*/
    if( (nodeTableP+node1)->joinNode >= 0 && (nodeTableP+node2)->joinNode >= 0 )
      {
/*
**  Scan To Last Node Of Joined Line String
*/
       startNode = lastNode = node2 ;
       snode = (nodeTableP+node2)->joinNode  ;
       while( snode != DTM_NULL_PNT && snode != startNode )
         {
/*
**       Get Node At Other End Line
*/
          node = (nodeTableP+snode)->node ; 
          if( (nodeTableP+snode)->direction == 1 ) node = node + 1 ;
          else                                     node = node - 1 ;
          node = (nodeTableP+node)->sortOfs ;
          lastNode = node ;
/*
**        Check Both Nodes Point To Same Line
*/
          if( cdbg )
            { 
             if( (nodeTableP+node)->featureOfs != (nodeTableP+snode)->featureOfs )
               {
                bcdtmWrite_message(0,0,0,"Node Feature Offsets Not The Same") ;
                bcdtmWrite_message(0,0,0,"Node [%10ld] ** Feature Offset = %10ld",node,(nodeTableP+node)->featureOfs) ;
                bcdtmWrite_message(0,0,0,"sNode[%10ld] ** Feature Offset = %10ld",snode,(nodeTableP+snode)->featureOfs) ;
                goto errexit ;
               }
            }
/*
**        Get Next Join Node
*/
          if( lastNode != startNode )  snode = (nodeTableP+node)->joinNode  ;
          else                         snode = DTM_NULL_PNT ;
         }
/*
**     Scan To Other End Of Joined Line String 
*/
       snode = lastNode ;
       do
         {
/*
**        Check For Memory Reallocation
*/
          if( *numLineTableP == memLines )
            {
             memLines = memLines + memLinesInc ;
             *lineTablePP = ( DTM_JOIN_LINE_TABLE * ) realloc ( *lineTablePP,memLines * sizeof(DTM_JOIN_LINE_TABLE)) ;
             if( *lineTablePP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
/*
**        Store Line In Line Table
*/
          (*lineTablePP+*numLineTableP)->lineNum    = lineNum ; 
          (*lineTablePP+*numLineTableP)->featureOfs = (nodeTableP+snode)->featureOfs ; 
          (*lineTablePP+*numLineTableP)->direction  = (nodeTableP+snode)->direction  ;
          ++*numLineTableP ;
/*
**        Mark Node As Being Processed
*/      
          (nodeTableP+snode)->joinNode = -DTM_NULL_PNT  ;
/*
**        Get Node At Other End Of Line
*/
          node = (nodeTableP+snode)->node ; 
          if( (nodeTableP+snode)->direction == 1 ) node = node + 1 ;
          else                                     node = node - 1 ;
          node = (nodeTableP+node)->sortOfs ;
/*
**        Get Join Node
*/
          snode = (nodeTableP+node)->joinNode  ;
/*
**        Mark Node As Being Processed
*/      
          (nodeTableP+node)->joinNode = -DTM_NULL_PNT  ;
/*
**        Continue Scan
*/
         } while ( snode != lastNode && snode != DTM_NULL_PNT ) ;
/*
**     Increment Line Number
*/
       ++lineNum ;
      }
   } 
/*
** Resize Memory
*/
 if( *numLineTableP < memLines ) *lineTablePP = ( DTM_JOIN_LINE_TABLE * ) realloc ( *lineTablePP,*numLineTableP * sizeof(DTM_JOIN_LINE_TABLE)) ;
/*
** Write Number Of Joined Lines
*/
 *numJoinedLinesP = lineNum ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Joined Lines = %10ld",lineNum) ;
/*
** Clean up
*/
 cleanup: 
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Join Line Table Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Join Line Table Error") ;
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
BENTLEYDTM_Private int bcdtmJoin_appendJoinedLineStringsToDtmObject(BC_DTM_OBJ *dataP,BC_DTM_OBJ *joinP,DTMFeatureType dtmFeatureType,double tolerance,DTM_JOIN_FEATURE_TABLE *featureTableP,long numFeatureTable,DTM_JOIN_LINE_TABLE *lineTableP,long numLineTable,DTM_JOIN_USER_TAGS **joinUserTagsPP,long *numJoinUserTagsP) 
/*
** This function appends the joined line strings to the data object
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFeature,numFeaturePts,numPts,numLinePts,memUserTags=0,memUserTagsInc=1000 ;
 DPoint3d  *p3d1P,*p3d2P,*featurePtsP=NULL,*linePtsP=NULL ;
 BC_DTM_FEATURE         *dtmFeatureP=NULL ;
 DTM_JOIN_LINE_TABLE    *ltP,*lt1P,*lt2P  ;
/*
** Initialise
*/
 *numJoinUserTagsP = 0 ;
 if( *joinUserTagsPP != NULL ) { free(*joinUserTagsPP) ; *joinUserTagsPP = NULL ; }
/*
** Allocate Memory For User Tag Sequence
*/
 memUserTags = memUserTagsInc ;
 *joinUserTagsPP =  ( DTM_JOIN_USER_TAGS * ) malloc( memUserTags * sizeof(DTM_JOIN_USER_TAGS)) ;
 if( *joinUserTagsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Scan Line Table And Copy Joined Strings
*/
 lt1P = lt2P = lineTableP ;
 while ( lt1P < lineTableP + numLineTable )
   {
/*
**  Scan To End Of Joined Line String
*/
     while ( lt2P < lineTableP + numLineTable && lt2P->lineNum == lt1P->lineNum ) ++lt2P ;
     --lt2P ;
/*
**  Copy Joined Lines From Join To Data
*/
    numLinePts = 0 ;
    for( ltP = lt1P ; ltP <= lt2P ; ++ltP )
      {
       dtmFeature = ltP->featureOfs ;
       dtmFeatureP = ftableAddrP(joinP,dtmFeature) ; 
       if( bcdtmObject_getPointsForDtmFeatureDtmObject(joinP,dtmFeature,(DTM_TIN_POINT **)&featurePtsP,&numFeaturePts)) goto errexit ; 
       if( ltP->direction == 2 ) bcdtmMath_reversePolygonDirectionP3D(featurePtsP,numFeaturePts) ;
/*
**     Copy Feature Points To line Points
*/
       if( numFeaturePts > 0 )
         { 
          numPts = numLinePts ;
          numLinePts = numLinePts + numFeaturePts ;
          if( linePtsP == NULL ) linePtsP = ( DPoint3d * ) malloc(numLinePts*sizeof(DPoint3d)) ;
          else                   linePtsP = ( DPoint3d * ) realloc(linePtsP,numLinePts*sizeof(DPoint3d)) ;        
          if( linePtsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
          for( p3d1P = linePtsP + numPts , p3d2P = featurePtsP ; p3d2P < featurePtsP + numFeaturePts ; ++p3d1P , ++p3d2P )
            {
             *p3d1P = *p3d2P ;
            }
        }
       if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/*
**     Check User Tag memory
*/
       if( *numJoinUserTagsP == memUserTags )
         {
          memUserTags = memUserTags + memUserTagsInc ;
          *joinUserTagsPP =  ( DTM_JOIN_USER_TAGS * ) realloc(*joinUserTagsPP,memUserTags * sizeof(DTM_JOIN_USER_TAGS)) ;
          if( *joinUserTagsPP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Store User Tag Sequence
*/
       (*joinUserTagsPP+*numJoinUserTagsP)->lineNumber = ltP->lineNum  ;
       (*joinUserTagsPP+*numJoinUserTagsP)->direction  = ltP->direction ;
       (*joinUserTagsPP+*numJoinUserTagsP)->userTag    = dtmFeatureP->dtmUserTag ;
       ++*numJoinUserTagsP ;
      }   
/*
**  Store Joined Line In DTM Object 
*/
    if( numLinePts > 0 ) if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,dtmFeatureP->dtmFeatureType,DTM_NULL_USER_TAG,2,&dtmFeatureP->dtmFeatureId,linePtsP,numLinePts)) goto errexit ; 
    if( linePtsP != NULL ) { free(linePtsP) ; linePtsP = NULL ; }
    
/*
**  Reset For Next Join Line
*/
    lt1P = lt2P = lt2P + 1 ;
   } 
/*
** Clean up
*/
 cleanup: 
 if( linePtsP    != NULL ) { free(linePtsP)    ; linePtsP = NULL    ; }
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Appending Joined Line Strings To Data Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Appending Joined Line Strings To Data Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
