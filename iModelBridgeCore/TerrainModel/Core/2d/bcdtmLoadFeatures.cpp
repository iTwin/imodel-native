/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmLoadFeatures.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h"
/*
** Define Cache Variables
*/
thread_local static DPoint3d *cachePtsP=nullptr ;
thread_local static int numCachePts = 0;
thread_local static int memCachePts = 0;
thread_local static int memCachePtsInc = 10000;
thread_local static DTMFeatureCallback  dtmDllLoadFunction = nullptr;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLoad_setDtmDllLoadFunction
(
 DTMFeatureCallback dllFunctionP
)
/*
** This Function Sets The DTM DLL Load Function
*/
{
 int ret=DTM_SUCCESS ;
 dtmDllLoadFunction =  dllFunctionP ;
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
BENTLEYDTM_EXPORT int bcdtmLoad_freeCache(void)
{
 if( cachePtsP != nullptr ) free(cachePtsP) ;
 cachePtsP = nullptr ;
 numCachePts = 0  ;
 memCachePts = 0  ;
 return(DTM_SUCCESS)  ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_clearCache(void)
{
 numCachePts = 0  ;
 return(DTM_SUCCESS)  ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_getCachePoints(DPoint3d **p3dPtsPP,long *numP3dPtsP)
{
 int ret=DTM_SUCCESS ;
 DPoint3d *p3d1P,*p3d2P ;
/*
** Initialise
*/
 *numP3dPtsP = 0 ;
 if( *p3dPtsPP != nullptr ) { free(*p3dPtsPP) ; *p3dPtsPP = nullptr ; }
/*
** Copy Cache Points
*/
 if( numCachePts > 0 )
   {
    *numP3dPtsP = numCachePts ;
    *p3dPtsPP = ( DPoint3d * ) malloc ( numCachePts * sizeof(DPoint3d)) ;
    if( *p3dPtsPP == nullptr )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( p3d1P = *p3dPtsPP , p3d2P = cachePtsP ; p3d2P < cachePtsP + numCachePts ; ++p3d1P , ++p3d2P )
      {
       *p3d1P = *p3d2P ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 numCachePts = 0 ;
/*
** Job Completed
*/
 return(ret)  ;
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
BENTLEYDTM_Public int bcdtmLoad_storePointInCache(double x,double y,double z)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** This Function Stores A Feature Point. All points for a DTM feature are
** accumulated to the cache before the DTM feature is written to
** the user load function
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Load Feature Point %10.4lf %10.4lf %10.4lf",x,y,z) ;
/*
** Check For Sufficient Heap Space
*/
 if( numCachePts == memCachePts )
   {
    memCachePts = memCachePts + memCachePtsInc ;
    if( cachePtsP == nullptr ) cachePtsP = ( DPoint3d * )  malloc  ( memCachePts * sizeof(DPoint3d)) ;
    else                    cachePtsP = ( DPoint3d * )  realloc ( cachePtsP , memCachePts * sizeof(DPoint3d)) ;
    if( cachePtsP == nullptr )
      {
       numCachePts = memCachePts = 0 ;
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
   }
/*
** Store Point
*/
 (cachePtsP+numCachePts)->x = x ;
 (cachePtsP+numCachePts)->y = y ;
 (cachePtsP+numCachePts)->z = z ;
 ++numCachePts ;
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
BENTLEYDTM_Public int bcdtmLoad_checkIfPointInCache(double x,double y,double z)
{
 int stored=FALSE ;
 DPoint3d *p3dP ;
 if( cachePtsP != nullptr && numCachePts > 0 )
   {
    for( p3dP = cachePtsP ; p3dP < cachePtsP + numCachePts && stored == FALSE ; ++p3dP )
      {
       if( p3dP->x == x && p3dP->y == y && p3dP->z == z ) stored = TRUE ;
      }
   }
 return(stored) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_copyCachePointsToPointArray(DPoint3d **pointsPP,long *numPointsP)
/*
** This Function Copies The Load Points To A DPoint3d Point Array
** And sets the number of load points to zero ;
*/
{
 int ret=DTM_SUCCESS ;
 DPoint3d *p3d1P,*p3d2P ;
/*
** Free Points Array Memory If Necessary
*/
 *numPointsP = numCachePts ;
 if( *pointsPP != nullptr ) { free(*pointsPP) ; *pointsPP = nullptr ; }
/*
** Only Copy If Load Points Present
*/
 if( numCachePts > 0 )
   {
/*
** Allocate memeory
*/
    *pointsPP = (DPoint3d * ) malloc ( *numPointsP * sizeof(DPoint3d)) ;
    if( *pointsPP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Copy Load Points
*/
    for( p3d1P = cachePtsP , p3d2P = *pointsPP ; p3d1P < cachePtsP + numCachePts ; ++p3d1P , ++p3d2P ) *p3d2P = *p3d1P ;
   }
/*
** Clean Up
*/
 cleanup :
 numCachePts = 0 ;
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
BENTLEYDTM_Public int bcdtmLoad_callUserBrowseFunctionWithCachePoints
(
 DTMFeatureCallback loadFunctionP,
 DTMFeatureType dtmFeatureType,
 DTMUserTag   userTag,
 DTMFeatureId userFeatureId,
 void           *userP
)
/*
** This Function Loads A DTM Feature By Calling A User Function
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 DPoint3d *p3dP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing DTM Feature Type = %4ld User Tag = %10I64d User Feature Id = %10I64d NumLoadPts = %5ld userP = %p",dtmFeatureType,userTag,userFeatureId,numCachePts,userP) ;
/*
** Only write for one or more load points
*/
 if( numCachePts > 0  )
   {
/*
**  Write DTM Feature Points To Log File
*/
    if( dbg == 2 )
      {
       for( p3dP = cachePtsP ; p3dP < cachePtsP + numCachePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %10.4lf",(long)(p3dP-cachePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
/*
**  Call User Browse Function
*/
    if( loadFunctionP != nullptr )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calling Load Function = %p",loadFunctionP ) ;
       if( loadFunctionP((DTMFeatureType)dtmFeatureType,userTag,userFeatureId,cachePtsP,numCachePts,userP) != DTM_SUCCESS ) goto errexit ;
      }
/*
**  No Browse Function Set
*/
    else
      {
       bcdtmWrite_message(2,0,0,"No Browse Function Set") ;
       goto errexit ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 numCachePts = 0 ;
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
BENTLEYDTM_Public int bcdtmLoad_storeFeaturePoint(double x,double y,double z,DPoint3d **loadPtsPP,long *numLoadPtsP,long *memLoadPtsP,long memLoadPtsInc )
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** This Function Stores A Feature Point. All points for a DTM feature are
** accumulated in *loadPtsPP global array before the DTM feature is written to
** the user load function
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Load Feature Point %10.4lf %10.4lf %10.4lf",x,y,z) ;
/*
** Check For Sufficient Heap Space
*/
 if( *numLoadPtsP == *memLoadPtsP )
   {
    *memLoadPtsP = *memLoadPtsP + memLoadPtsInc ;
    if( *loadPtsPP == nullptr ) *loadPtsPP = ( DPoint3d * )  malloc  ( *memLoadPtsP * sizeof(DPoint3d)) ;
    else                     *loadPtsPP = ( DPoint3d * )  realloc ( *loadPtsPP , *memLoadPtsP * sizeof(DPoint3d)) ;
    if( *loadPtsPP == nullptr )
      {
       *numLoadPtsP = *memLoadPtsP = 0 ;
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
   }
/*
** Store Point
*/
 (*loadPtsPP+*numLoadPtsP)->x = x ;
 (*loadPtsPP+*numLoadPtsP)->y = y ;
 (*loadPtsPP+*numLoadPtsP)->z = z ;
 ++*numLoadPtsP ;
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
BENTLEYDTM_Public int bcdtmLoad_callUserLoadFunction
(
 DTMFeatureCallback loadFunctionP,
 DTMFeatureType dtmFeatureType,
 DTMUserTag       userTag,
 DTMFeatureId     userFeatureId,
 DPoint3d                *featurePtsP,
 long               numFeaturePts,
 void               *userP
)
/*
** This Function Loads A DTM Feature By Calling A User Function
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 DPoint3d *p3dP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing DTM Feature Type = %4ld User Tag = %10I64d User Feature Id = %10I64d NumLoadPts = %10ld  userP = %p",dtmFeatureType,userTag,userFeatureId,numFeaturePts,userP) ;
/*
** Only write for one or more load points
*/
 if( numFeaturePts > 0 || dtmFeatureType == DTMFeatureType::CheckStop)
   {
/*
**  Write DTM Feature To Log File
*/
    if( dbg == 2 )
      {
       for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
/*
**  Note:-
**  1. For bcLIB loadFunctionP should always be set
**  2. For Geopak the loadFunctionP is set through calls to the relevent functions
**
**  bcLib Call
*/
   if( loadFunctionP != nullptr )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calling Load Function = %p",loadFunctionP ) ;
       if( (ret = loadFunctionP((DTMFeatureType)dtmFeatureType,userTag,userFeatureId,featurePtsP,numFeaturePts,userP)) != DTM_SUCCESS ) goto errexit ;
      }
/*
**  DLL Function Call
*/
   else if( dtmDllLoadFunction != nullptr )
     {
      if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calling Load Function = %p",dtmDllLoadFunction ) ;
      if( dtmDllLoadFunction((DTMFeatureType)dtmFeatureType,userTag,userFeatureId,featurePtsP,numFeaturePts,userP) != DTM_SUCCESS ) goto errexit ;
     }
/*
**  MDL Function Call
*/
//    else if( dtmMdlLoadFunction != 0 )
//     {
//      if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calling MDL Function ** DescP = %p Function Offset = %10ld",dtmMdlDescriptorP,dtmMdlLoadFunction) ;
//	    if( dlmSystem_callMdlFunction(dtmMdlDescriptorP,dtmMdlLoadFunction,dtmFeatureType,userTag,userFeatureId,loadPtsP,numLoadPts,userP) != DTM_SUCCESS ) goto errexit ;
//     }
/*
**  No Load Function Set
*/
    else
      {
       bcdtmWrite_message(2,0,0,"No load function set") ;
       goto errexit ;
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
// bcdtmWrite_message(0,0,0,"Load Function Terminated By Calling Function") ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_callUserTriangleShadeMeshLoadFunction
(
 DTMTriangleShadeMeshCallback loadFunctionP,
 DTMFeatureType dtmFeatureType,
 long           numTriangles,
 long           numMeshPts,
 DPoint3d       *meshPtsP,
 DPoint3d       *meshVectorsP,
 long           numMeshFaces,
 long           *meshFacesP,
 void           *userP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Calling User Triangle Shade Mesh Load Function") ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType  = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"numTriangles    = %8ld",numTriangles) ;
    bcdtmWrite_message(0,0,0,"numMeshPts      = %8ld",numMeshPts) ;
    bcdtmWrite_message(0,0,0,"meshPtsP        = %p",meshPtsP) ;
    bcdtmWrite_message(0,0,0,"meshVectorsP    = %p",meshVectorsP) ;
    bcdtmWrite_message(0,0,0,"numMeshFaces    = %8ld",numMeshFaces) ;
    bcdtmWrite_message(0,0,0,"meshFacesP      = %p",meshFacesP) ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
   }
/*
** Call User Load Function
*/
 if( loadFunctionP(dtmFeatureType,numTriangles,numMeshPts,meshPtsP,meshVectorsP,numMeshFaces,meshFacesP,userP)) goto errexit ;
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
/*==============================================================================*//**
* @memo   Load All Occurrences Of A DTM Feature Type From A DTM File
* @doc    Load All Occurrences Of A DTM Feature Type From A DTM  File
* @notes  This function loads A DTM Feature Type From A DTM File
* @author Rob Cormack March 2007  rob.cormack@bentley.con
* @param  dtmFile,              ==> Dtm File Name
* @param  dtmFeatureType,       ==> Type Of DTM Feature To Be Loaded
* @param  maxSpots              ==> Maximum Number Of Points To Load Per Call For Random and Group Spots
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence,             ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts,             ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_dtmFeatureTypeFromDtmFile
(
 WCharCP dtmFileP,        /* ==> Dtm File                                       */
 DTMFeatureType dtmFeatureType,      /* ==> Dtm Feature Type To Be Loaded                  */
 long maxSpots,            /* ==> Maximum Number Of Spots To Load Per Call       */
 DTMFeatureCallback loadFunctionP,   /* ==> Pointer To Load Function                       */
 long useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>         */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>   */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>            */
 DPoint3d  *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                      */
 long numFencePts,         /* ==> Number Of Fence Points                         */
 void *userP               /* ==> User Pointer Passed Back To User               */
)
/*
** This Function Loads All Occurrences Of A DTM Feature Type From A Dtm File
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Loading Dtm Feature Type From Dtm File") ;
    bcdtmWrite_message(0,0,0,"dtmFile           = %s",dtmFileP) ;
    bcdtmWrite_message(0,0,0,"maxSpots          = %8ld",maxSpots) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType    = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
   }
/*
** Test If Requested Dtm Is Current Dtm
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Load Dtm Feature From Dtm Object
*/
 if( bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject(dtmP,dtmFeatureType,maxSpots,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP)) goto errexit ;
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
/*==============================================================================*//**
* @memo   Load Occurrences Of A DTM Feature Type From A DTM Dtm Object
* @doc    Load Occurrences Of A DTM Feature Type From A DTM Dtm Object
* @author Rob Cormack March 2007  rob.cormack@bentley.con
* @param  dtmP                  ==> Pointer To DTM object
* @param  dtmFeatureType        ==> Type Of DTM Feature To Be Loaded
* @param  maxSpots              ==> Maximum Number Of Points To Load Per Call For Random and Group Spots
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject
(
 BC_DTM_OBJ  *dtmP,           /* ==> Pointer To DTM Dtm object                    */
 DTMFeatureType dtmFeatureType,      /* ==> Type Of DTM Feature To Be Loaded             */
 long    maxSpots,            /* ==> Maximum Number Of Spots Points To Load Per Call       */
 DTMFeatureCallback loadFunctionP,   /* ==> Pointer To Load Function                     */
 long    useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape> */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>          */
 DPoint3d     *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                    */
 long    numFencePts,         /* ==> Number Of Fence Points                       */
 void    *userP               /* ==> User Pointer Passed Back To User             */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DPoint3d *p3dP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interrupt Loading Dtm Feature Type From Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType    = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"maxSpots          = %8ld",maxSpots) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
    if( fencePtsP != nullptr && numFencePts > 0 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          p3dP->z = 0.0 ;
          bcdtmWrite_message(0,0,0,"fencePts[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == nullptr || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Test For Valid Dtm Feature Type
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object Feature Type") ;
 if( bcdtmData_testForValidDtmObjectExportFeatureType(dtmFeatureType) == DTM_ERROR  )
   {
    bcdtmWrite_message(2,0,0,"Invalid Dtm Feature Type %4ld",dtmFeatureType) ;
    goto errexit ;
   }
/*
** Validate Max Spots
*/
 if( maxSpots < 0 ) maxSpots = 1 ;
 if( maxSpots > 50000 ) maxSpots = 50000 ;
 if( maxSpots > dtmP->numPoints ) maxSpots = dtmP->numPoints ;
/*
** Load DTM Feature Type
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Occurrences Of Dtm Feature Type") ;
 if( ret = bcdtmInterruptLoad_dtmFeatureTypeOccurrencesDtmObject(dtmP,dtmFeatureType,maxSpots,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Dtm Feature Type From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Dtm Feature Type From Dtm Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if (ret == DTM_SUCCESS) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmInterruptLoad_dtmFeatureTypeOccurrencesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,long maxSpots,DTMFeatureCallback loadFunctionP,long useFence,DTMFenceType fenceType,DTMFenceOption fenceOption,DPoint3d *fencePtsP,long numFencePts,void *userP )
/*
** This Function Loads All Occurrences of a DTM Feature Type From A Dtm Object
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0);
 long            n,p1,p2,p3,clPtr,voidFlag,numSpots=0,numLines,numTriangles,numClipArrays,clipResult;
 long            *ofsP,voidsInDtm=FALSE,startPnt,lastPnt,startTime,dtmFeatureNum,numFeaturePts ;
 long            pnt1,pnt2,pnt3,fndType,insideFence,fenceLoad,numMask,numMarked=0 ;
 long            findType,trgPnt1,trgPnt2,trgPnt3 ;
 unsigned char   *charP,*pointMaskP=nullptr ;
 DPoint3d             *p3dP,tinPoint,edgePts[10],trgPts[5],*featurePtsP=nullptr ;
 DTM_TIN_NODE    *nodeP,*node1P,*node2P,*node3P ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 BC_DTM_OBJ      *clipDtmP=nullptr  ;
 DTM_TIN_POINT   *p1P,*p2P,*p3P,*pntP  ;
 DTM_POINT_ARRAY **clipArraysPP=nullptr ;
 DTM_CIR_LIST    *clistP ;
 DTMUserTag    hullUserTag ;
 DTMFeatureId  hullFeatureId ;
 char            dtmFeatureTypeName[50] ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interrupt Feature Type Occurrences From Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType    = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"maxSpots          = %8ld",maxSpots) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
    if( fencePtsP != nullptr && numFencePts > 0 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          p3dP->z = 0.0 ;
          bcdtmWrite_message(0,0,0,"fencePts[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Set Hull Feature Type Depending On DTM State
*/
 if( dtmP->dtmState == DTMState::Tin && dtmFeatureType == DTMFeatureType::Hull ) dtmFeatureType = DTMFeatureType::TinHull ;
/*
** Write Entry Message
*/
 if( dbg == 20 )
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName);
    bcdtmWrite_message(0,0,0,"Interrupt Loading %s From DTM Object %p",dtmFeatureTypeName,dtmP) ;
    for( dtmFeatureNum = 0 ; dtmFeatureNum < dtmP->numFeatures ; ++dtmFeatureNum )
      {
       bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(ftableAddrP(dtmP,dtmFeatureNum)->dtmFeatureType,dtmFeatureTypeName);
       bcdtmWrite_message(0,0,0,"dtmFeature[%6ld] ** dtmFeatureType = %s",dtmFeatureNum,dtmFeatureTypeName) ;
      }
   }
/*
** Build Clipping Dtm For Fence Operations
*/
 if( useFence == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Clipping Tin") ;
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Clipping Tin Completed") ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
   }
/*
** Switch Depending On DTM Feature Type
*/
 switch( dtmFeatureType )
   {
    case  DTMFeatureType::Spots :    // All Dtm Points
      if( dbg ) bcdtmWrite_message(0,0,0,"Loading DTMFeatureType::Spots") ;
      numSpots = 0 ;
      featurePtsP = ( DPoint3d * ) malloc ( maxSpots * sizeof(DPoint3d)) ;
      if( featurePtsP == nullptr )
        {
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit ;
        }
      for( p1 = 0 ; p1 < dtmP->numPoints  ; ++p1 )
        {
         if( dtmP->dtmState != DTMState::Tin || ( dtmP->dtmState == DTMState::Tin && ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD) ) )
           {
            pntP = pointAddrP(dtmP,p1) ;
            tinPoint.x = pntP->x ;
            tinPoint.y = pntP->y ;
            tinPoint.z = pntP->z ;
            if( useFence == FALSE )
              {
               (featurePtsP+numSpots)->x = pntP->x ;
               (featurePtsP+numSpots)->y = pntP->y ;
               (featurePtsP+numSpots)->z = pntP->z ;
               ++numSpots ;
              }
            else
              {
               insideFence = FALSE ;
               if( fenceType == DTMFenceType::Block && tinPoint.x >= clipDtmP->xMin && tinPoint.x <= clipDtmP->xMax && tinPoint.y >= clipDtmP->yMin && tinPoint.y <= clipDtmP->yMax ) insideFence = TRUE ; ;
               if( fenceType == DTMFenceType::Shape && tinPoint.x >= clipDtmP->xMin && tinPoint.x <= clipDtmP->xMax && tinPoint.y >= clipDtmP->yMin && tinPoint.y <= clipDtmP->yMax )
                 {
                  if( bcdtmFind_triangleDtmObject(dtmP,tinPoint.x,tinPoint.y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
                  if( fndType ) insideFence = TRUE ;
                 }
               fenceLoad = FALSE ;
               if( ( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )  && insideFence == TRUE  ) fenceLoad = TRUE ;
               if(   fenceOption == DTMFenceOption::Outside && insideFence == FALSE ) fenceLoad = TRUE ;
               if( fenceLoad == TRUE )
                 {
                  (featurePtsP+numSpots)->x = pntP->x ;
                  (featurePtsP+numSpots)->y = pntP->y ;
                  (featurePtsP+numSpots)->z = pntP->z ;
                  ++numSpots ;
                 }
              }
           }
/*
**       Check For Max Spots
*/
         if( numSpots == maxSpots )
           {
            if( dbg ) bcdtmWrite_message(0,0,0,"Loading %8ld Points",numSpots) ;
            if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,featurePtsP,maxSpots,userP)) goto errexit ;
            numSpots = 0 ;
           }
        }
/*
**    Check For Remaining Spots
*/
      if( numSpots > 0 )
        {
         if( dbg ) bcdtmWrite_message(0,0,0,"Loading %8ld Points",numSpots) ;
         if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,featurePtsP,numSpots,userP)) goto errexit ;
         numSpots = 0 ;
        }

    break ;

    case  DTMFeatureType::RandomSpots  :    // Points That Are Not A DTM Feature Point
    case  DTMFeatureType::FeatureSpot :    // Points That Are A DTM Feature Point
/*
**    If The DTM Is Not In The DTMFeatureState::Tin State Mask All Features Points
*/
      numSpots = 0 ;
      featurePtsP = ( DPoint3d * ) malloc ( maxSpots * sizeof(DPoint3d)) ;
      if( featurePtsP == nullptr )
        {
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit ;
        }

      if( dtmP->dtmState == DTMState::Data )
        {
         numMask = dtmP->numPoints / 8 + 1 ;
         pointMaskP = ( unsigned char * ) malloc( numMask * sizeof(char)) ;
         if( pointMaskP == nullptr )
           {
            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
            goto errexit ;
           }
         for( charP = pointMaskP ; charP < pointMaskP + numMask ; ++charP ) *charP = ( char ) 0 ;
/*
**       Scan Features And Mask Feature Points
*/
         for( n = 0 ; n < dtmP->numFeatures ; ++n )
           {
            dtmFeatureP = ftableAddrP(dtmP,n) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
              {
               for( p1 = dtmFeatureP->dtmFeaturePts.firstPoint ; p1 < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++p1 )
                 {
                  bcdtmFlag_setFlag(pointMaskP,p1) ;
                 }
              }
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
              {
               for( ofsP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ; ofsP < bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) + dtmFeatureP->numDtmFeaturePts ; ++ofsP )
                 {
                  bcdtmFlag_setFlag(pointMaskP,*ofsP) ;
                 }
              }
           }
        }
/*
**    Scan DTM Points
*/
      for( p1 = 0 ; p1 < dtmP->numPoints  ; ++p1 )
        {
         pntP = nullptr ;
         if( dtmP->dtmState == DTMState::Data )
           {
            if( ( dtmFeatureType == DTMFeatureType::RandomSpots && ! bcdtmFlag_testFlag(pointMaskP,p1)) || ( dtmFeatureType == DTMFeatureType::FeatureSpot && bcdtmFlag_testFlag(pointMaskP,p1)) )
              {
               pntP = pointAddrP(dtmP,p1) ;
               tinPoint.x = pntP->x ;
               tinPoint.y = pntP->y ;
               tinPoint.z = pntP->z ;
              }
           }
         if( dtmP->dtmState == DTMState::Tin )
           {
            if( nodeAddrP(dtmP,p1)->cPtr != dtmP->nullPtr )
              {
               if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD))
                 {
                  if( ( dtmFeatureType == DTMFeatureType::RandomSpots && nodeAddrP(dtmP,p1)->fPtr == dtmP->nullPtr ) || ( dtmFeatureType == DTMFeatureType::FeatureSpot && nodeAddrP(dtmP,p1)->fPtr != dtmP->nullPtr ) )
                    {
                     pntP = pointAddrP(dtmP,p1) ;
                     tinPoint.x = pntP->x ;
                     tinPoint.y = pntP->y ;
                     tinPoint.z = pntP->z ;
                    }
                 }
              }
           }
         if( pntP != nullptr )
           {
            if( useFence == FALSE )
              {
               (featurePtsP+numSpots)->x = pntP->x ;
               (featurePtsP+numSpots)->y = pntP->y ;
               (featurePtsP+numSpots)->z = pntP->z ;
               ++numSpots ;
              }
            else
              {
               insideFence = FALSE ;
               if( fenceType == DTMFenceType::Block && tinPoint.x >= clipDtmP->xMin && tinPoint.x <= clipDtmP->xMax && tinPoint.y >= clipDtmP->yMin && tinPoint.y <= clipDtmP->yMax ) insideFence = TRUE ; ;
               if( fenceType == DTMFenceType::Shape && tinPoint.x >= clipDtmP->xMin && tinPoint.x <= clipDtmP->xMax && tinPoint.y >= clipDtmP->yMin && tinPoint.y <= clipDtmP->yMax )
                 {
                  if( bcdtmFind_triangleDtmObject(dtmP,tinPoint.x,tinPoint.y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
                  if( fndType ) insideFence = TRUE ;
                 }
               fenceLoad = FALSE ;
               if( ( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )  && insideFence == TRUE  ) fenceLoad = TRUE ;
               if(   fenceOption == DTMFenceOption::Outside && insideFence == FALSE ) fenceLoad = TRUE ;
               if( fenceLoad == TRUE )
                 {
                  (featurePtsP+numSpots)->x = pntP->x ;
                  (featurePtsP+numSpots)->y = pntP->y ;
                  (featurePtsP+numSpots)->z = pntP->z ;
                  ++numSpots ;
                 }
              }
           }
/*
**       Check For Max Spots
*/
         if( numSpots == maxSpots )
           {
            if( dbg ) bcdtmWrite_message(0,0,0,"Loading %8ld Random Points",numSpots) ;
            if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,featurePtsP,maxSpots,userP)) goto errexit ;
            numSpots = 0 ;
           }
        }
/*
**    Check For Remaining Spots
*/
      if( numSpots > 0 )
        {
         if( dbg ) bcdtmWrite_message(0,0,0,"Loading %8ld Random Points",numSpots) ;
        if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,featurePtsP,numSpots,userP)) goto errexit ;
         numSpots = 0 ;
        }
   break ;

    case  DTMFeatureType::TinPoint  :

      numSpots = 0 ;
      featurePtsP = ( DPoint3d * ) malloc ( maxSpots * sizeof(DPoint3d)) ;
      if( featurePtsP == nullptr )
        {
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit ;
        }

      if( dtmP->dtmState == DTMState::Tin )
        {
         for( p1 = 0 ; p1 < dtmP->numPoints  ; ++p1 )
           {
            pntP = pointAddrP(dtmP,p1) ;
            tinPoint.x = pntP->x ;
            tinPoint.y = pntP->y ;
            tinPoint.z = pntP->z ;
            if( useFence == FALSE )
              {
               (featurePtsP+numSpots)->x = pntP->x ;
               (featurePtsP+numSpots)->y = pntP->y ;
               (featurePtsP+numSpots)->z = pntP->z ;
               ++numSpots ;
              }
            else
              {
               insideFence = FALSE ;
               if( fenceType == DTMFenceType::Block && tinPoint.x >= clipDtmP->xMin && tinPoint.x <= clipDtmP->xMax && tinPoint.y >= clipDtmP->yMin && tinPoint.y <= clipDtmP->yMax ) insideFence = TRUE ; ;
               if( fenceType == DTMFenceType::Shape && tinPoint.x >= clipDtmP->xMin && tinPoint.x <= clipDtmP->xMax && tinPoint.y >= clipDtmP->yMin && tinPoint.y <= clipDtmP->yMax )
                 {
                  if( bcdtmFind_triangleDtmObject(dtmP,tinPoint.x,tinPoint.y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
                  if( fndType ) insideFence = TRUE ;
                 }
               fenceLoad = FALSE ;
               if( ( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )  && insideFence == TRUE  ) fenceLoad = TRUE ;
               if(   fenceOption == DTMFenceOption::Outside && insideFence == FALSE ) fenceLoad = TRUE ;
               if( fenceLoad == TRUE )
                 {
                  (featurePtsP+numSpots)->x = pntP->x ;
                  (featurePtsP+numSpots)->y = pntP->y ;
                  (featurePtsP+numSpots)->z = pntP->z ;
                  ++numSpots ;
                 }
              }
/*
**          Check For Max Spots
*/
            if( numSpots == maxSpots )
              {
               if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,featurePtsP,maxSpots,userP)) goto errexit ;
               numSpots = 0 ;
              }
           }
/*
**       Check For Remaining Spots
*/
         if( numSpots > 0 )
           {
            if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,featurePtsP,numSpots,userP)) goto errexit ;
            numSpots = 0 ;
           }
        }
    break ;

/*
    case  DTMFeatureType::TriangleIndex:
      if( dtmP->dtmState == DTMState::Tin )
        {
        for( p1 = 0 ; p1 < dtmP->numPoints  ; ++p1 )
           {
            node1P = nodeAddrP(dtmP,p1) ;
            if( ( clPtr = node1P->cPtr) != dtmP->nullPtr )
              {
               if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
               while ( clPtr != dtmP->nullPtr )
                 {
                  clistP = clistAddrP(dtmP,clPtr) ;
                  clPtr  = clistP->nextPtr ;
                  p3     = clistP->pntNum ;
                  if( p2 > p1 && p3 > p1 && node1P->hPtr != p2 )
                    {
                     tinPoint.x = ( double ) p1  ;
                     tinPoint.y = ( double ) p2  ;
                     tinPoint.z = ( double ) p3  ;
                     if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,&tinPoint,1,userP)) goto errexit ;
                    }
                  p2 = p3 ;
                 }
              }
           }
        }
    break ;
*/
/*
**   Triangle Info... This returns the 3 triangle points the vertex numbers, if this is a valid triangulation, the aspect and slope.
*/
/*
    case  DTMFeatureType::TriangleInfo  :
      if( dtmP->dtmState == DTMState::Tin )
        {
        DPoint3d tinInfoPts[5];
        double slopeDegrees;
        double slopePercent;
        double aspect;
        double height;

//     Check For Voids In Dtm

        bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
        if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;

        for( p1 = 0 ; p1 < dtmP->numPoints  ; ++p1 )
           {
            node1P = nodeAddrP(dtmP,p1) ;
            if( ( clPtr = node1P->cPtr) != dtmP->nullPtr )
              {
               if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
               while ( clPtr != dtmP->nullPtr )
                 {
                  clistP = clistAddrP(dtmP,clPtr) ;
                  clPtr  = clistP->nextPtr ;
                  p3     = clistP->pntNum ;
                  if( p2 > p1 && p3 > p1 && node1P->hPtr != p2 )
                    {
                       voidFlag = FALSE ;
                       if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidFlag)) goto errexit ; }

                       // Set up the triangle points.
                       pntP = pointAddrP(dtmP,p1) ;
                       tinInfoPts[0].x = pntP->x ;
                       tinInfoPts[0].y = pntP->y ;
                       tinInfoPts[0].z = pntP->z ;
                       pntP = pointAddrP(dtmP,p2) ;
                       tinInfoPts[1].x = pntP->x ;
                       tinInfoPts[1].y = pntP->y ;
                       tinInfoPts[1].z = pntP->z ;
                       pntP = pointAddrP(dtmP,p3) ;
                       tinInfoPts[2].x = pntP->x ;
                       tinInfoPts[2].y = pntP->y ;
                       tinInfoPts[2].z = pntP->z ;

                       // Set up the triangle vertex numbers.
                       tinInfoPts[3].x = ( double ) p1  ;
                       tinInfoPts[3].y = ( double ) p2  ;
                       tinInfoPts[3].z = ( double ) p3  ;

                       bcdtmMath_getTriangleAttributesDtmObject(dtmP, p1, p2, p3, &slopeDegrees, &slopePercent, &aspect, &height);

                       // Set the void flag, aspect and slope.
                       tinInfoPts[4].x = voidFlag != FALSE ? 1 : 0;
                       tinInfoPts[4].y = ( double ) aspect  ;
                       tinInfoPts[4].z = ( double ) slopePercent  ;
                       if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,tinInfoPts,5,userP)) goto errexit ;
                    }
                  p2 = p3 ;
                 }
              }
           }
        }
    break ;
*/


    case  DTMFeatureType::Breakline :
    case  DTMFeatureType::SoftBreakline :
    case  DTMFeatureType::ContourLine :
    case  DTMFeatureType::Void :
    case  DTMFeatureType::Island :
    case  DTMFeatureType::Hole :
    case  DTMFeatureType::GraphicBreak :
    case  DTMFeatureType::Hull :
    case  DTMFeatureType::Polygon:
    case  DTMFeatureType::SlopeToe :
    case  DTMFeatureType::Region :
/*
**     Scan Dtm Object And Extract All Occurrences For The DTM Feature Type
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Loading Dtm Feature Type = %2ld",dtmFeatureType) ;
       bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,dtmFeatureType,1,&dtmFeatureP,&dtmFeatureNum) ;
       while( dtmFeatureP != nullptr )
         {
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
             if( bcdtmObject_getPointsForDtmFeatureDtmObject(dtmP,dtmFeatureNum,(DTM_TIN_POINT **) &featurePtsP ,&numFeaturePts)) goto errexit ;
             if( numFeaturePts > 0 )
               {
                if( useFence == FALSE )
                  {
                   if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
                  }
                else
                  {
                   if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,featurePtsP,numFeaturePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                   if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
                   if( clipResult == 2 )
                     {
                      for( n = 0 ; n < numClipArrays ; ++n )
                        {
                         if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                        }
                      bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                     }
                  }
               }
            }
          if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
          bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,dtmFeatureType,0,&dtmFeatureP,&dtmFeatureNum) ;
         }
    break ;

    case  DTMFeatureType::GroupSpots :

      numSpots = 0 ;
      featurePtsP = ( DPoint3d * ) malloc ( maxSpots * sizeof(DPoint3d)) ;
      if( featurePtsP == nullptr )
        {
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit ;
        }
/*
**     Scan Dtm Object And Extract All Occurrences For The DTM Feature Type
*/
       bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,dtmFeatureType,1,&dtmFeatureP,&dtmFeatureNum) ;
       while( dtmFeatureP != nullptr )
         {
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
             p1 = dtmFeatureP->dtmFeaturePts.firstPoint ;
             do
               {
                pntP = pointAddrP(dtmP,p1) ;
                if( useFence == FALSE )
                  {
                   (featurePtsP+numSpots)->x = pntP->x ;
                   (featurePtsP+numSpots)->y = pntP->y ;
                   (featurePtsP+numSpots)->z = pntP->z ;
                   ++numSpots ;
                  }
                else
                  {
                   insideFence = FALSE ;
                   if( fenceType == DTMFenceType::Block && pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax ) insideFence = TRUE ; ;
                   if( fenceType == DTMFenceType::Shape && pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
                     {
                      if( bcdtmFind_triangleDtmObject(dtmP,pntP->x,pntP->y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
                      if( fndType ) insideFence = TRUE ;
                     }
                   fenceLoad = FALSE ;
                   if( ( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )  && insideFence == TRUE  ) fenceLoad = TRUE ;
                   if(   fenceOption == DTMFenceOption::Outside && insideFence == FALSE ) fenceLoad = TRUE ;
                   if( fenceLoad == TRUE )
                     {
                      (featurePtsP+numSpots)->x = pntP->x ;
                      (featurePtsP+numSpots)->y = pntP->y ;
                      (featurePtsP+numSpots)->z = pntP->z ;
                      ++numSpots ;
                     }
                  }
/*
**              Check For Max Spots
*/
                if( numSpots == maxSpots )
                  {
                   if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,maxSpots,userP)) goto errexit ;
                   numSpots = 0 ;
                  }
/*
**              Get Next Feature Point
*/
                p2 = p1 ;
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
                  {
                   if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeatureNum,p1,&p2)) goto errexit ;
                  }
                else
                  {
                   ++p2 ;
                   if( p2 >= dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ) p2 = dtmP->nullPnt ;
                  }
                p1 = p2 ;
               } while ( p1 != dtmP->nullPnt && p1 != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
/*
**           Check For Remaining Spots
*/
             if( numSpots > 0 )
               {
                if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numSpots,userP)) goto errexit ;
                numSpots = 0 ;
               }
            }
          bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,dtmFeatureType,0,&dtmFeatureP,&dtmFeatureNum) ;
         }
    break ;

    case  DTMFeatureType::Triangle :
    case  DTMFeatureType::TriangleInfo :
    case  DTMFeatureType::TriangleIndex :
    case  DTMFeatureType::FlowArrow :
/*
**    Write Start Message
*/
      if( dbg )
        {
         if( dtmFeatureType == DTMFeatureType::Triangle ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p",dtmP) ;
         if( dtmFeatureType == DTMFeatureType::TriangleInfo ) bcdtmWrite_message(0,0,0,"Loading Triangle Info From DTM Object %p",dtmP) ;
         if( dtmFeatureType == DTMFeatureType::TriangleIndex ) bcdtmWrite_message(0,0,0,"Loading Triangle Index From DTM Object %p",dtmP) ;
        }
/*
**    Check If DTM Is In Tin State
*/
      if( dtmP->dtmState != DTMState::Tin )
        {
         bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
         goto errexit ;
        }
/*
**    Check For Voids In Dtm
*/
      bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
      if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
**    Find First Point Before And Last Point After Fence
*/
      startPnt = 0 ;
      lastPnt  = dtmP->numPoints ;
      numTriangles = 0 ;
      if( useFence == TRUE )
        {
         startTime = bcdtmClock() ;
         if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Overlap Triangles") ;
         bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
         while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
         if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
         bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
         while( lastPnt < dtmP->numPoints - 1 && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
         if( tdbg )
           {
            bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
            bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
            bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
            bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
            bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
           }
        if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;

/*
**      Mark Points Within Fence Block
*/
        startTime = bcdtmClock() ;
//        bcdtmList_nullSptrValuesDtmObject(dtmP) ;
        numMarked = 0 ;
        if( fenceType == DTMFenceType::Block )
          {
           if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Block") ;
           for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
             {
              pntP = pointAddrP(dtmP,p1) ;
              if( pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
                {
                 nodeAddrP(dtmP,p1)->sPtr = 1 ;
                 ++numMarked ;
                }
             }
          }
        if( fenceType == DTMFenceType::Shape )
          {
           if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Shape") ;
           for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
             {
              pntP = pointAddrP(dtmP,p1) ;
              findType = 0 ;
              if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
                {
                 if( bcdtmFind_triangleDtmObject(clipDtmP,pntP->x,pntP->y,&findType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                }
              if( findType  )
                {
                 nodeAddrP(dtmP,p1)->sPtr = 1 ;
                 ++numMarked ;
                }
             }
          }
/*
**       If No Points Marked Drape Fence
*/
         if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points Within Fence = %8ld",numMarked) ;
         if( numMarked == 0 )
           {

            if( dbg ) bcdtmWrite_message(0,0,0,"Marking Triangles",numMarked) ;
            if( bcdtmFind_triangleDtmObject(dtmP,clipDtmP->xMin,clipDtmP->yMin,&p1,&p2,&p3,&fndType)) goto errexit ;
            if( p1 != dtmP->nullPnt )
              {
               if( p1 < startPnt ) startPnt = p1 ;
               if( p1 > lastPnt  ) lastPnt  = p1 ;
               nodeAddrP(dtmP,p1)->sPtr = 1 ;
              }
            if( p2 != dtmP->nullPnt )
              {
               if( p2 < startPnt ) startPnt = p2 ;
               if( p2 > lastPnt  ) lastPnt  = p2 ;
               nodeAddrP(dtmP,p2)->sPtr = 1 ;
              }
            if( p3 != dtmP->nullPnt )
              {
               if( p3 < startPnt ) startPnt = p3 ;
               if( p3 > lastPnt  ) lastPnt  = p3 ;
               nodeAddrP(dtmP,p3)->sPtr = 1 ;
              }
            if( bcdtmFind_triangleDtmObject(dtmP,clipDtmP->xMax,clipDtmP->yMin,&p1,&p2,&p3,&fndType)) goto errexit ;
            if( p1 != dtmP->nullPnt )
              {
               if( p1 < startPnt ) startPnt = p1 ;
               if( p1 > lastPnt  ) lastPnt  = p1 ;
               nodeAddrP(dtmP,p1)->sPtr = 1 ;
             }
            if( p2 != dtmP->nullPnt )
              {
               if( p2 < startPnt ) startPnt = p2 ;
               if( p2 > lastPnt  ) lastPnt  = p2 ;
               nodeAddrP(dtmP,p2)->sPtr = 1 ;
              }
            if( p3 != dtmP->nullPnt )
              {
               if( p3 < startPnt ) startPnt = p3 ;
               if( p3 > lastPnt  ) lastPnt  = p3 ;
               nodeAddrP(dtmP,p3)->sPtr = 1 ;
              }
            if( bcdtmFind_triangleDtmObject(dtmP,clipDtmP->xMax,clipDtmP->yMax,&p1,&p2,&p3,&fndType)) goto errexit ;
            if( p1 != dtmP->nullPnt )
              {
               if( p1 < startPnt ) startPnt = p1 ;
               if( p1 > lastPnt  ) lastPnt  = p1 ;
               nodeAddrP(dtmP,p1)->sPtr = 1 ;
              }
            if( p2 != dtmP->nullPnt )
              {
               if( p2 < startPnt ) startPnt = p2 ;
               if( p2 > lastPnt  ) lastPnt  = p2 ;
               nodeAddrP(dtmP,p2)->sPtr = 1 ;
              }
            if( p3 != dtmP->nullPnt )
              {
               if( p3 < startPnt ) startPnt = p3 ;
               if( p3 > lastPnt  ) lastPnt  = p3 ;
               nodeAddrP(dtmP,p3)->sPtr = 1 ;
              }
            if( bcdtmFind_triangleDtmObject(dtmP,clipDtmP->xMin,clipDtmP->yMax,&p1,&p2,&p3,&fndType)) goto errexit ;
            if( p1 != dtmP->nullPnt )
              {
               if( p1 < startPnt ) startPnt = p1 ;
               if( p1 > lastPnt  ) lastPnt  = p1 ;
               nodeAddrP(dtmP,p1)->sPtr = 1 ;
              }
            if( p2 != dtmP->nullPnt )
              {
               if( p2 < startPnt ) startPnt = p2 ;
               if( p2 > lastPnt  ) lastPnt  = p2 ;
               nodeAddrP(dtmP,p2)->sPtr = 1 ;
              }
            if( p3 != dtmP->nullPnt )
              {
               if( p3 < startPnt ) startPnt = p3 ;
               if( p3 > lastPnt  ) lastPnt  = p3 ;
               nodeAddrP(dtmP,p3)->sPtr = 1 ;
              }

            for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
              {
               if( nodeAddrP(dtmP,p1)->sPtr == dtmP->nullPnt )
                 {
                  p1P = pointAddrP(dtmP,p1) ;
                  clPtr = nodeAddrP(dtmP,p1)->cPtr ;
                  while( clPtr != dtmP->nullPtr )
                    {
                     clistP = clistAddrP(dtmP,clPtr) ;
                     p2 = clistP->pntNum ;
                     clPtr = clistP->nextPtr ;
                     if( nodeAddrP(dtmP,p2)->sPtr == dtmP->nullPnt )
                       {
                        double xMin,xMax,yMin,yMax ;
                        xMin = xMax = p1P->x ;
                        yMin = yMax = p1P->y ;
                        p2P = pointAddrP(dtmP,p2) ;
                        if( p2P->x < xMin ) xMin = p2P->x ;
                        if( p2P->x > xMax ) xMax = p2P->x ;
                        if( p2P->y < yMin ) yMin = p2P->y ;
                        if( p2P->y > yMax ) yMax = p2P->y ;
                        if( xMin <= clipDtmP->xMax && xMax >= clipDtmP->yMin &&
                            yMin <= clipDtmP->yMax && yMax >= clipDtmP->yMin )
                          {
                           nodeAddrP(dtmP,p1)->sPtr = 1 ;
                          }
                       }
                    }
                 }
              }
           }
         if( tdbg ) bcdtmWrite_message(0,0,0,"** Index Time 01 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
**      Scan And Load Triangles
*/
        if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Fence Triangles") ;
        startTime = bcdtmClock() ;
        for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
          {
           node1P = nodeAddrP(dtmP,p1) ;
           if( node1P->sPtr == 1 && ( clPtr = node1P->cPtr) != dtmP->nullPtr )
             {
              p1P = pointAddrP(dtmP,p1) ;
              trgPts[0].x = p1P->x ;
              trgPts[0].y = p1P->y ;
              trgPts[0].z = p1P->z ;
              trgPts[3].x = p1P->x ;
              trgPts[3].y = p1P->y ;
              trgPts[3].z = p1P->z ;
              if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
              node2P = nodeAddrP(dtmP,p2) ;
              while ( clPtr != dtmP->nullPtr )
                {
                 clistP = clistAddrP(dtmP,clPtr) ;
                 clPtr  = clistP->nextPtr ;
                 p3     = clistP->pntNum ;
                 node3P = nodeAddrP(dtmP,p3) ;
                 if( node1P->hPtr != p2 )
                   {
                    if( ( p2 > p1 && p3 > p1 && node2P->sPtr == 1 && node3P->sPtr == 1 ) || ( node2P->sPtr == dtmP->nullPnt || node3P->sPtr == dtmP->nullPnt )  )
                      {
                       voidFlag = FALSE ;
                       if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidFlag)) goto errexit ; }
                       if( voidFlag == FALSE )
                         {
/*
**                        Set Point Addresses
*/
                          p2P = pointAddrP(dtmP,p2) ;
                          p3P = pointAddrP(dtmP,p3) ;
/*
**                        Set Point Coordinates
*/
                          trgPts[1].x = p2P->x ;
                          trgPts[1].y = p2P->y ;
                          trgPts[1].z = p2P->z ;
                          trgPts[2].x = p3P->x ;
                          trgPts[2].y = p3P->y ;
                          trgPts[2].z = p3P->z ;
                          ++numTriangles ;
/*
**                        Get Correct Data For Feature Type And Call Load Function
*/
                          switch ( dtmFeatureType )
                            {
                             case DTMFeatureType::Triangle :
                                if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                             break ;

                             case DTMFeatureType::TriangleInfo :
                               {
                                double slopeDegrees;
                                double slopePercent;
                                double aspect;
                                double height;
                                bcdtmMath_getTriangleAttributesDtmObject(dtmP, p1, p2, p3, &slopeDegrees, &slopePercent, &aspect, &height);
                                trgPts[3].x = ( double ) p1  ;
                                trgPts[3].y = ( double ) p2  ;
                                trgPts[3].z = ( double ) p3  ;
                                trgPts[4].x = voidFlag != FALSE ? 1 : 0;
                                trgPts[4].y = ( double ) aspect  ;
                                trgPts[4].z = ( double ) slopePercent  ;
                                if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,5,userP)) goto errexit ;
                               }
                             break ;

                             case DTMFeatureType::TriangleIndex :
                               tinPoint.x = ( double ) p1  ;
                               tinPoint.y = ( double ) p2  ;
                               tinPoint.z = ( double ) p3  ;
                               if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,&tinPoint,1,userP)) goto errexit ;
                             break ;

                             case DTMFeatureType::FlowArrow :
                               {
                                double ascentAngle,descentAngle,trgSlope ;
                                if( bcdtmMath_getTriangleDescentAndAscentAnglesDtmObject(dtmP,p1,p2,p3,&descentAngle,&ascentAngle,&trgSlope) != DTM_SUCCESS ) goto errexit ;
                                trgPts[0].x = (p1P->x + p2P->x + p3P->x ) / 3.0 ;
                                trgPts[0].y = (p1P->y + p2P->y + p3P->y ) / 3.0 ;
                                trgPts[0].z = (p1P->z + p2P->z + p3P->z ) / 3.0 ;
                                trgPts[1].x = ascentAngle ;
                                trgPts[1].y = descentAngle ;
                                trgPts[1].z = trgSlope ;
                                if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,2,userP)) goto errexit ;
                               }
                             break ;
                            }
                         }
                      }
                   }
                 p2     = p3 ;
                 node2P = node3P ;
                }
             }
          }
        if( tdbg )
          {
           bcdtmWrite_message(0,0,0,"** Index Time 02 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
           bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded = %8ld",numTriangles) ;
          }
        if( dtmP->dtmState == DTMState::Tin )bcdtmList_nullSptrValuesDtmObject(dtmP) ;
       }
/*
**    Scan And Load All Triangles
*/
      else
        {
         if( dbg ) bcdtmWrite_message(0,0,0,"Scanning All Triangles") ;
         for( p1 = startPnt ; p1 < lastPnt ; ++p1 )
          {
           nodeP = nodeAddrP(dtmP,p1) ;
           if( ( clPtr = nodeP->cPtr) != dtmP->nullPtr )
            if( clPtr != dtmP->nullPtr )
              {
               p1P = pointAddrP(dtmP,p1) ;
               trgPts[0].x = p1P->x ;
               trgPts[0].y = p1P->y ;
               trgPts[0].z = p1P->z ;
               trgPts[3].x = p1P->x ;
               trgPts[3].y = p1P->y ;
               trgPts[3].z = p1P->z ;
               if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
               while( clPtr != dtmP->nullPtr )
                 {
                  p3 = clistAddrP(dtmP,clPtr)->pntNum ;
                  if( p2 > p1 && p3 > p1 )
                    {
                     if( nodeAddrP(dtmP,p1)->hPtr != p2 )
                       {
                        voidFlag = FALSE ;
                        if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidFlag)) goto errexit ; }
                        if( voidFlag == FALSE )
                          {
/*
**                         Set Point Addresses
*/
                           p2P = pointAddrP(dtmP,p2) ;
                           p3P = pointAddrP(dtmP,p3) ;
/*
**                         Set Point Coordinates
*/
                           trgPts[1].x = p2P->x ;
                           trgPts[1].y = p2P->y ;
                           trgPts[1].z = p2P->z ;
                           trgPts[2].x = p3P->x ;
                           trgPts[2].y = p3P->y ;
                           trgPts[2].z = p3P->z ;
/*
**                         Load Triangle
*/
                           if( useFence == FALSE )
                             {
                              ++numTriangles ;
/*
**                            Get Correct Data For Feature Type And Call Load Function
*/
                              switch ( dtmFeatureType )
                                {
                                 case DTMFeatureType::Triangle :
                                 if( ret = bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                                 break ;

                                 case DTMFeatureType::TriangleInfo :
                                   {
                                    double slopeDegrees;
                                    double slopePercent;
                                    double aspect;
                                    double height;
                                    bcdtmMath_getTriangleAttributesDtmObject(dtmP, p1, p2, p3, &slopeDegrees, &slopePercent, &aspect, &height);
                                    trgPts[3].x = ( double ) p1  ;
                                    trgPts[3].y = ( double ) p2  ;
                                    trgPts[3].z = ( double ) p3  ;
                                    trgPts[4].x = voidFlag != FALSE ? 1 : 0;
                                    trgPts[4].y = ( double ) aspect  ;
                                    trgPts[4].z = ( double ) slopePercent  ;
                                    if( ret = bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,5,userP)) goto errexit ;
                                   }
                                 break ;

                                 case DTMFeatureType::TriangleIndex :
                                   tinPoint.x = ( double ) p1  ;
                                   tinPoint.y = ( double ) p2  ;
                                   tinPoint.z = ( double ) p3  ;
                                   if( ret = bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,&tinPoint,1,userP)) goto errexit ;
                                 break ;

                                 case DTMFeatureType::FlowArrow :
                                   {
                                    double ascentAngle,descentAngle,trgSlope ;
                                    if( bcdtmMath_getTriangleDescentAndAscentAnglesDtmObject(dtmP,p1,p2,p3,&descentAngle,&ascentAngle,&trgSlope) != DTM_SUCCESS ) goto errexit ;
                                    trgPts[0].x = (p1P->x + p2P->x + p3P->x ) / 3.0 ;
                                    trgPts[0].y = (p1P->y + p2P->y + p3P->y ) / 3.0 ;
                                    trgPts[0].z = (p1P->z + p2P->z + p3P->z ) / 3.0 ;
                                    trgPts[1].x = ascentAngle ;
                                    trgPts[1].y = descentAngle ;
                                    trgPts[1].z = trgSlope ;
                                    if( ret = bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,2,userP)) goto errexit ;
                                   }
                                 break ;
                                }
                             }
/*
**                        Check If Triangle Lies In Fence
*/
                           else
                             {
                              if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,trgPts,4,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                              if( clipResult == 1 )
                                {
                                 ++numTriangles ;
                                 if( ret = bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP))
                                   {
                                    bcdtmWrite_message(1,0,0,"User Load Function Returned Error") ;
                                    goto errexit ;
                                   }
                                }
                              if( clipResult == 2 )
                                {
                                 ++numTriangles ;
                                 for( n = 0 ; n < numClipArrays ; ++n )
                                   {
                                    if( ret = bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP))
                                      {
                                       bcdtmWrite_message(1,0,0,"User Load Function Returned Error") ;
                                       goto errexit ;
                                      }
                                   }
                                 bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                                }
                             }
                          }
                       }
                    }
                  clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                  p2  = p3 ;
                 }
              }
           }
        }
      if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded = %10ld",numTriangles) ;
    break ;

    case  DTMFeatureType::TinLine :
/*
**    Write Start Message
*/
      if( dbg ) bcdtmWrite_message(0,0,0,"Loading Triangle Edges From DTM Object %p",dtmP) ;
/*
**    Check If DTM Is In Tin State
*/
      if( dtmP->dtmState != DTMState::Tin )
        {
         bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
         goto errexit ;
        }
/*
**    Check For Voids In Dtm
*/
      bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
      if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
**    Find First Point Before And Last Point After Fence
*/
      startPnt = 0 ;
      lastPnt  = dtmP->numPoints ;
      numLines = 0 ;
      if( useFence == TRUE && fenceOption == DTMFenceOption::Overlap )
        {
         startTime = bcdtmClock() ;
         if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Overlap Triangle Edges") ;
         bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
         while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
         if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
         bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
         while( lastPnt < dtmP->numPoints - 1 && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
         if( tdbg )
           {
            bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
            bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
            bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
            bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
            bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
           }
/*
**      Mark Points Within Fence Block
*/
        startTime = bcdtmClock() ;
        if( fenceType == DTMFenceType::Block )
          {
           for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
             {
              pntP = pointAddrP(dtmP,p1) ;
              if( pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax ) nodeAddrP(dtmP,p1)->sPtr = 1 ;
             }
          }
/*
**      Mark Points Within Fence Shape
*/
        if( fenceType == DTMFenceType::Shape )
          {
           for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
             {
              pntP = pointAddrP(dtmP,p1) ;
              findType = 0 ;
              if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
                {
                 if( bcdtmFind_triangleDtmObject(clipDtmP,pntP->x,pntP->y,&findType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                }
              if( findType  ) nodeAddrP(dtmP,p1)->sPtr = 1 ;
             }
          }
        if( tdbg ) bcdtmWrite_message(0,0,0,"** Index Time 01 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
**      Scan And Load Triangle Edges
*/
        startTime = bcdtmClock() ;
        for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
          {
           node1P = nodeAddrP(dtmP,p1) ;
           if( node1P->sPtr == 1 && ( clPtr = node1P->cPtr) != dtmP->nullPtr )
             {
              p1P = pointAddrP(dtmP,p1) ;
              edgePts[0].x = p1P->x ;
              edgePts[0].y = p1P->y ;
              edgePts[0].z = p1P->z ;
              while ( clPtr != dtmP->nullPtr )
                {
                 clistP = clistAddrP(dtmP,clPtr) ;
                 p2     = clistP->pntNum ;
                 clPtr  = clistP->nextPtr ;
                 node2P = nodeAddrP(dtmP,p2) ;
                 if( p2 > p1 && node2P->sPtr == 1 )
                   {
                    voidFlag = FALSE ;
                    if( voidsInDtm ) { if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidFlag)) goto errexit ; }
                    if( voidFlag == FALSE )
                      {
/*
**                     Set Point Addresses And Coordinates
*/
                       p2P = pointAddrP(dtmP,p2) ;
                       edgePts[1].x = p2P->x ;
                       edgePts[1].y = p2P->y ;
                       edgePts[1].z = p2P->z ;
                       ++numLines ;
                       if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,edgePts,2,userP)) goto errexit ;
                      }
                   }
                }
             }
          }
        if( tdbg )
          {
           bcdtmWrite_message(0,0,0,"** Index Time 02 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
           bcdtmWrite_message(0,0,0,"Number Of Triangle Edges Loaded = %8ld",numLines) ;
          }
        }
/*
**    Scan And Load Triangle Edges
*/
      else
        {
         if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Triangle Edges") ;
         for( p1 = startPnt ; p1 < lastPnt ; ++p1 )
           {
            nodeP = nodeAddrP(dtmP,p1) ;
            if( ( clPtr = nodeP->cPtr) != dtmP->nullPtr )
              {
               p1P = pointAddrP(dtmP,p1) ;
               edgePts[0].x = p1P->x ;
               edgePts[0].y = p1P->y ;
               edgePts[0].z = p1P->z ;
               while( clPtr != dtmP->nullPtr )
                 {
                  clistP = clistAddrP(dtmP,clPtr) ;
                  p2     = clistP->pntNum ;
                  clPtr  = clistP->nextPtr ;
                  if( p2 > p1 )
                    {
                     voidFlag = FALSE ;
                     if( voidsInDtm ) { if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidFlag)) goto errexit ; }
                     if( voidFlag == FALSE )
                       {
/*
**                      Set Point Addresse And Coordinates
*/
                        p2P = pointAddrP(dtmP,p2) ;
                        edgePts[1].x = p2P->x ;
                        edgePts[1].y = p2P->y ;
                        edgePts[1].z = p2P->z ;
/*
**                      Load Triangle Edge
*/
                        if( useFence == FALSE )
                          {
                           ++numLines ;
                           if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,edgePts,2,userP))
                             {
                              bcdtmWrite_message(1,0,0,"User Load Function Returned Error") ;
                              goto errexit ;
                             }
                          }
/*
**                      Check If Triangle Edge Lies In Fence
*/
                        else
                          {
                           if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,edgePts,2,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                           if( clipResult == 1 )
                             {
                              ++numLines ;
                              if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,edgePts,2,userP))
                                {
                                 bcdtmWrite_message(1,0,0,"User Load Function Returned Error") ;
                                 goto errexit ;
                                }
                             }
                           if( clipResult == 2 )
                             {
                              ++numLines ;
                              for( n = 0 ; n < numClipArrays ; ++n )
                                {
                                 if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP))
                                   {
                                    bcdtmWrite_message(1,0,0,"User Load Function Returned Error") ;
                                    goto errexit ;
                                   }
                                }
                              bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                             }
                          }
                       }
                    }
                 }
              }
           }
        }
      if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Triangle Edges Loaded = %10ld",numLines) ;
    break ;

    case  DTMFeatureType::TinHull :
/*
**  Get Hull User Tag And Feature Id For User Hull Feature
*/
    hullUserTag = dtmP->nullUserTag ;
    hullFeatureId = dtmP->nullFeatureId ;
    bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Hull,1,&dtmFeatureP,&dtmFeatureNum) ;
    if( dtmFeatureP != nullptr )
      {
       hullUserTag = dtmFeatureP->dtmUserTag ;
       hullFeatureId = dtmFeatureP->dtmFeatureId ;
      }

    case  DTMFeatureType::TriangleEdge :
/*
**    Check If DTM Is In Tin State
*/
      if( dtmP->dtmState != DTMState::Tin )
        {
         bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
         goto errexit ;
        }
/*
**    Count Number Of Points In Tin Hull
*/
      if( dbg ) bcdtmWrite_message(0,0,0,"Loading Tin Hull") ;
      numFeaturePts = 0 ;
      p1 = dtmP->hullPoint ;
      do
        {
         ++numFeaturePts ;
         p1 = nodeAddrP(dtmP,p1)->hPtr ;
        } while ( p1 != dtmP->hullPoint ) ;
      ++numFeaturePts ;
/*
**    Allocate Memory For Hull Points
*/
      featurePtsP = ( DPoint3d * ) malloc ( numFeaturePts * sizeof(DPoint3d)) ;
      if( featurePtsP == nullptr )
        {
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit ;
        }
/*
**    Copy Hull Points
*/
      p1 = dtmP->hullPoint ;
      p3dP = featurePtsP ;
      do
        {
         p1P = pointAddrP(dtmP,p1) ;
         p3dP->x = p1P->x ;
         p3dP->y = p1P->y ;
         p3dP->z = p1P->z ;
         ++p3dP ;
         p1 = nodeAddrP(dtmP,p1)->hPtr ;
        } while ( p1 != dtmP->hullPoint ) ;
      p1P = pointAddrP(dtmP,p1) ;
      p3dP->x = p1P->x ;
      p3dP->y = p1P->y ;
      p3dP->z = p1P->z ;
/*
**    Check For Fence
*/
      if( useFence == FALSE )
        {
         if( dtmFeatureType == DTMFeatureType::TinHull ) { if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,hullUserTag,hullFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ; }
         if( dtmFeatureType == DTMFeatureType::TriangleEdge )
           {
            for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts - 1 ; ++p3dP )
              {
               edgePts[0].x = p3dP->x     ; edgePts[0].y = p3dP->y     ; edgePts[0].z = p3dP->z ;
               edgePts[1].x = (p3dP+1)->x ; edgePts[1].y = (p3dP+1)->y ; edgePts[1].z = (p3dP+1)->z ;
               edgePts[2].x = (p3dP+1)->x ; edgePts[2].y = (p3dP+1)->y ; edgePts[2].z = dtmP->zMin ;
               edgePts[3].x = p3dP->x     ; edgePts[3].y = p3dP->y     ; edgePts[3].z = dtmP->zMin ;
               edgePts[4].x = p3dP->x     ; edgePts[4].y = p3dP->y     ; edgePts[4].z = p3dP->z ;
               if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,edgePts,5,userP)) goto errexit ;
              }
           }
        }
      else
        {
         if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,featurePtsP,numFeaturePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
         if( clipResult == 1 )
           {
            if( dtmFeatureType == DTMFeatureType::TinHull ) { if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ; }
            if( dtmFeatureType == DTMFeatureType::TriangleEdge )
              {
               for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts - 1 ; ++p3dP )
                 {
                  edgePts[0].x = p3dP->x     ; edgePts[0].y = p3dP->y     ; edgePts[0].z = p3dP->z ;
                  edgePts[1].x = (p3dP+1)->x ; edgePts[1].y = (p3dP+1)->y ; edgePts[1].z = (p3dP+1)->z ;
                  edgePts[2].x = (p3dP+1)->x ; edgePts[2].y = (p3dP+1)->y ; edgePts[2].z = dtmP->zMin ;
                  edgePts[3].x = p3dP->x     ; edgePts[3].y = p3dP->y     ; edgePts[3].z = dtmP->zMin ;
                  edgePts[4].x = p3dP->x     ; edgePts[4].y = p3dP->y     ; edgePts[4].z = p3dP->z ;
                  if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,edgePts,5,userP)) goto errexit ;
                 }
              }
           }
         if( clipResult == 2 )
           {
            for( n = 0 ; n < numClipArrays ; ++n )
              {
               if( dtmFeatureType == DTMFeatureType::TinHull ) { if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,hullUserTag,hullFeatureId,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ; }
               if( dtmFeatureType == DTMFeatureType::TriangleEdge )
                 {
                  for( p3dP = clipArraysPP[n]->pointsP ; p3dP < clipArraysPP[n]->pointsP + clipArraysPP[n]->numPoints -1 ; ++p3dP )
                    {
                     edgePts[0].x = p3dP->x     ; edgePts[0].y = p3dP->y     ; edgePts[0].z = p3dP->z ;
                     edgePts[1].x = (p3dP+1)->x ; edgePts[1].y = (p3dP+1)->y ; edgePts[1].z = (p3dP+1)->z ;
                     edgePts[2].x = (p3dP+1)->x ; edgePts[2].y = (p3dP+1)->y ; edgePts[2].z = dtmP->zMin ;
                     edgePts[3].x = p3dP->x     ; edgePts[3].y = p3dP->y     ; edgePts[3].z = dtmP->zMin ;
                     edgePts[4].x = p3dP->x     ; edgePts[4].y = p3dP->y     ; edgePts[4].z = p3dP->z ;
                     if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,edgePts,5,userP)) goto errexit ;
                    }
                 }
              }
            bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
           }
        }
    break ;

    default :
    break ;

   } ;
/*
** Clean Up
*/
 cleanup :
 if( clipDtmP    != nullptr ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 if( featurePtsP != nullptr ) { free( featurePtsP) ; featurePtsP = nullptr ; }
 if( pointMaskP  != nullptr ) { free(pointMaskP)   ; pointMaskP  = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Interrupt Feature Type Occurrences From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Interrupt Feature Type Occurrences From Dtm Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( dtmP->dtmState == DTMState::Tin ) bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_dtmFeaturesWithTinErrorsDtmObject
(
 BC_DTM_OBJ *dtmP,            /* ==> Pointer To Dtm Object                  */
 DTMFeatureCallback loadFunctionP ,     /* ==> Load Function                          */
 void    *userP               /* ==> User Pointer Passed Back To User       */
)
/*
** This Function Loads All Occurrences of DTM Features With Tin Errors From A Dtm Object
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    dtmFeature,numErrorFeatures=0 ;
 char    dtmFeatureTypeName[100] ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interrupt Loading Dtm Features With Tin Errors From Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
  }
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Scan Dtm Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError )
      {
       ++numErrorFeatures ;
       if( dbg )
         {
          if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ) goto errexit ;
          bcdtmWrite_message(0,0,0,"Tin Feature Error ** Feature = %8ld ** Type = %s",dtmFeature,dtmFeatureTypeName) ;
         }
       if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts,userP)) goto errexit ;
      }
   }
/*
** Write Number Of Features With Errors
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Error Features = %8ld",numErrorFeatures) ;
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
/*==============================================================================*//**
* @memo   Scan Load Next Occurrence Of A DTM Feature Type From A DTM Dtm Object
* @doc    Scan Load Next Occurrence Of A DTM Feature Type From A DTM Dtm Object
* @author Rob Cormack April 2007  rob.cormack@bentley.con
* @param  dtmP                  ==> Pointer To DTM object
* @param  dtmFeatureType        ==> Type Of DTM Feature To Be Loaded
* @param  firstCall             ==> First Call <TRUE,FALSE>
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  featureFoundP         <== Feature Found <TRUE,FALSE>
* @param  scanFeature           <== Feature Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmScanLoad_nextDtmFeatureTypeOccurrenceDtmObject
(
 BC_DTM_OBJ  *dtmP,                /* ==> Pointer To Dtm Object                        */
 DTMFeatureType dtmFeatureType,           /* ==> Type Of DTM Feature To Be Loaded             */
 long    firstCall,                /* ==> First Call <TRUE,FALSE>                      */
 long    useFence,                 /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DTMFenceType fenceType,                /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape> */
 DTMFenceOption fenceOption,              /* ==> Fence Option <INSIDE(1),OVERLAP(2)>          */
 DPoint3d     *fencePtsP,               /* ==> DPoint3d Array Of Fence Points                    */
 long    numFencePts,              /* ==> Number Of Fence Points                       */
 long    *featureFoundP,           /* <== Scan Feature Found<TRUE,FALSE>               */
 BC_DTM_USER_FEATURE *scanFeatureP /* <== Pointer To Scan Feature                      */
)
/*
** This Function Loads The Next Dtm Feature Occurrence Occurrence From A Dtm Object
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long            n,p1,p2,p3,clPtr,voidFlag,clipResult;
 long            *ofsP, startTime, dtmFeatureNum, numFeaturePts;
 DTMFenceOption trgExtent;
 long            pnt1,pnt2,pnt3,fndType,insideFence,fenceLoad,numMask,numPts=0,maxPoints=10000 ;
 unsigned char            *charP ;
 double          xMin,yMin,xMax,yMax ;
 DPoint3d             *p3dP,*featurePtsP=nullptr ;
 char            dtmFeatureTypeName[50] ;
 DTM_TIN_NODE    *nodeP ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 DTM_TIN_POINT   *p1P,*p2P,*p3P,*pntP  ;
 DTM_CIR_LIST    *clistP ;
/*
** Static Variables For Maintaining Scan Context
*/
 static long scanPnt1=0,scanPnt2=0,scanPnt3=0,scanClPtr=0  ;
 static long startScanPnt=0,lastScanPnt=0,voidsInDtm=FALSE ;
 static DTM_TIN_NODE  *scanNodeP=nullptr ;
 static unsigned char          *pointMaskP=nullptr ;
/*
** Static Variables For Maintaining Clipping Context
*/
 static long numClipNext=0,numClipArrays=0,numHullPts=0 ;
 static BC_DTM_OBJ       *clipDtmP=nullptr ;
 static DTM_POINT_ARRAY **clipArraysPP=nullptr ;
 static BC_DTM_FEATURE   *clipFeatureP ;
 static DPoint3d              trgPts[4],*hullPtsP=nullptr,*scanHullPntP ;
/*
** Write Entry Message
*/
 bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName);
 bcdtmWrite_message(0,0,0,"Scan Loading %s From DTM Object %p",dtmFeatureTypeName,dtmP) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scan Loading Dtm Feature Type From Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType    = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"firstCall         = %8ld",firstCall) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
   }
/*
** Initialise
*/
 *featureFoundP = FALSE ;
/*
** Do First Call Processing
*/
 if( firstCall == TRUE )
   {
/*
**  Check Scan Feature Points Pointer
*/
    if( scanFeatureP->dtmFeaturePtsP != nullptr )
      {
       bcdtmWrite_message(2,0,0,"Pointer To Scan Feature Points Is Not nullptr") ;
       goto errexit ;
      }
/*
**  Validate Fence
*/
    if( useFence == TRUE && ( fencePtsP == nullptr || numFencePts <= 2 ) ) useFence = FALSE ;
    if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
    if( useFence )
      {
       if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
       if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
      }
/*
**  Test For Valid Dtm Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
    if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
**  Test For Valid Dtm Feature Type
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object Feature Type") ;
    if( bcdtmData_testForValidDtmObjectExportFeatureType(dtmFeatureType) == DTM_ERROR  )
      {
       bcdtmWrite_message(2,0,0,"Invalid Dtm Feature Type %4ld",dtmFeatureType) ;
       goto errexit ;
      }
/*
**  Create Point Mask For DTMFeatureType::RandomSpots
*/
    if( dtmFeatureType == DTMFeatureType::RandomSpots && dtmP->dtmState != DTMState::Tin )
      {
       numMask = dtmP->numPoints / 8 + 1 ;
       pointMaskP = ( unsigned char * ) malloc( numMask * sizeof(char)) ;
       if( pointMaskP == nullptr )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       for( charP = pointMaskP ; charP < pointMaskP + numMask ; ++charP ) *charP = ( char ) 0 ;
/*
**    Scan Features And Mask Feature Points
*/
      for( n = 0 ; n < dtmP->numFeatures ; ++n )
        {
         dtmFeatureP = ftableAddrP(dtmP,n) ;
         if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
           {
            for( p1 = dtmFeatureP->dtmFeaturePts.firstPoint ; p1 < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++p1 )
              {
               bcdtmFlag_setFlag(pointMaskP,p1) ;
              }
           }
         if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
           {
            for( ofsP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ; ofsP < bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) + dtmFeatureP->numDtmFeaturePts ; ++ofsP )
              {
               bcdtmFlag_setFlag(pointMaskP,*ofsP) ;
              }
           }
        }
     }
/*
**  Check If DTM Is In Tin State
*/
    if( dtmFeatureType == DTMFeatureType::Triangle || dtmFeatureType == DTMFeatureType::TinLine )
      {
       if( dtmP->dtmState != DTMState::Tin )
         {
          bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
          goto errexit ;
         }
/*
**     Check For Voids In Dtm
*/
       bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
**     Allocate Memory For Triangle Points
*/
       scanFeatureP->dtmFeaturePtsP = ( DPoint3d * ) malloc( 4 * sizeof(DPoint3d)) ;
       if( scanFeatureP->dtmFeaturePtsP == nullptr )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Initialise Static Scan Variables
*/
    scanPnt1     = 0 ;
    scanPnt2     = 0 ;
    scanPnt3     = 0 ;
    scanClPtr    = dtmP->nullPtr ;
    scanNodeP    = nullptr ;
    startScanPnt = 0 ;
    lastScanPnt  = dtmP->numPoints ;
    numClipNext   = 0 ;
    numClipArrays = 0 ;
/*
**  Build Clipping Dtm For Fence Operations
*/
    if( useFence == TRUE )
      {
/*
**     Build Clipping Tin
*/
       if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
/*
**     Find First And Last Scan Points For Point Scan
*/
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startScanPnt) ;
       while( startScanPnt > 0 && pointAddrP(dtmP,startScanPnt)->x >= clipDtmP->xMin ) --startScanPnt ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastScanPnt) ;
       while( lastScanPnt < dtmP->numPoints  && pointAddrP(dtmP,lastScanPnt)->x <= clipDtmP->xMin ) ++lastScanPnt ;
       if( lastScanPnt > dtmP->numPoints ) lastScanPnt = dtmP->numPoints ;
       if( dbg ) bcdtmWrite_message(0,0,0,"startScanPnt = %8ld lastScanPnt = %8ld",startScanPnt,lastScanPnt) ;
/*
**     Set ScanPnt1 To Start Scan Point
*/
       scanPnt1 = startScanPnt ;
      }
   }
/*
** Free Scan Feature Points Memory
*/
  if( dtmFeatureType != DTMFeatureType::Triangle && dtmFeatureType != DTMFeatureType::TinLine )
    {
     if( scanFeatureP->dtmFeaturePtsP != nullptr ) { free(scanFeatureP->dtmFeaturePtsP) ; scanFeatureP->dtmFeaturePtsP = nullptr ; }
    }
  scanFeatureP->numDtmFeaturePts = 0 ;
/*
** Set Hull Feature Type Depending On DTM State
*/
 if( dtmP->dtmState == DTMState::Tin && dtmFeatureType == DTMFeatureType::Hull ) dtmFeatureType = DTMFeatureType::TinHull ;
/*
** Switch Depending On DTM Feature Type
*/
 switch( dtmFeatureType )
   {
    case  DTMFeatureType::Spots :    // All Dtm Points

      for( p1 = scanPnt1 ; p1 < lastScanPnt && numPts < maxPoints ; ++p1 )
        {
         if( dtmP->dtmState != DTMState::Tin || ( dtmP->dtmState == DTMState::Tin && ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD) ) )
           {
            pntP = pointAddrP(dtmP,p1) ;
/*
**          Apply Fence
*/
            fenceLoad = TRUE ;
            if( useFence == TRUE )
              {
               insideFence = FALSE ;
               if( fenceType == DTMFenceType::Block && pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax ) insideFence = TRUE ; ;
               if( fenceType == DTMFenceType::Shape && pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
                 {
                  if( bcdtmFind_triangleDtmObject(dtmP,pntP->x,pntP->y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
                  if( fndType ) insideFence = TRUE ;
                 }
               fenceLoad = FALSE ;
               if( ( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )  && insideFence == TRUE  ) fenceLoad = TRUE ;
               if(   fenceOption == DTMFenceOption::Outside && insideFence == FALSE ) fenceLoad = TRUE ;
              }
/*
**          Point Satisfies Fence
*/
            if( fenceLoad == TRUE )
              {
               if( numPts == 0 )
                 {
                  scanFeatureP->dtmFeaturePtsP = ( DPoint3d * ) malloc( maxPoints * sizeof(DPoint3d)) ;
                  if( scanFeatureP->dtmFeaturePtsP == nullptr )
                    {
                     bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                     goto errexit ;
                    }
                 }
               (scanFeatureP->dtmFeaturePtsP+numPts)->x = pntP->x ;
               (scanFeatureP->dtmFeaturePtsP+numPts)->y = pntP->y ;
               (scanFeatureP->dtmFeaturePtsP+numPts)->z = pntP->z ;
               ++numPts ;
              }
           }
        }
/*
**    Set Scan Feature Variables
*/
      if( numPts > 0 )
        {
         scanFeatureP->dtmFeatureType   = DTMFeatureType::Spots ;
         scanFeatureP->dtmFeatureId     = dtmP->nullFeatureId ;
         scanFeatureP->dtmUserTag       = dtmP->nullUserTag ;
         scanFeatureP->numDtmFeaturePts = numPts ;
         *featureFoundP = TRUE ;
         scanPnt1 = p1  ;
        }
    break ;

    case  DTMFeatureType::RandomSpots :    // Points That Are Not On Any Dtm Feature Type
/*
**    Scan DTM Points
*/
      for( p1 = scanPnt1 ; p1 < lastScanPnt && numPts < maxPoints ; ++p1 )
        {
/*
**       Set Point Adress For Random Spot
*/
         pntP = nullptr ;
         if( dtmP->dtmState != DTMState::Tin )
           {
            if( ! bcdtmFlag_testFlag(pointMaskP,p1))
              {
               pntP = pointAddrP(dtmP,p1) ;
              }
           }
         if( dtmP->dtmState == DTMState::Tin )
           {
            if( nodeAddrP(dtmP,p1)->cPtr != dtmP->nullPtr )
              {
               if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD))
                 {
                  if( nodeAddrP(dtmP,p1)->fPtr == dtmP->nullPtr )
                    {
                     pntP = pointAddrP(dtmP,p1) ;
                    }
                 }
              }
           }
/*
**       Random Spot Found
*/
         if( pntP != nullptr )
           {
/*
**          Apply Fence
*/
            fenceLoad = TRUE ;
            if( useFence == TRUE )
              {
               insideFence = FALSE ;
               if( fenceType == DTMFenceType::Block && pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax ) insideFence = TRUE ; ;
               if( fenceType == DTMFenceType::Shape && pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
                 {
                  if( bcdtmFind_triangleDtmObject(dtmP,pntP->x,pntP->y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
                  if( fndType ) insideFence = TRUE ;
                 }
               fenceLoad = FALSE ;
               if( ( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )  && insideFence == TRUE  ) fenceLoad = TRUE ;
               if(   fenceOption == DTMFenceOption::Outside && insideFence == FALSE ) fenceLoad = TRUE ;
              }
/*
**          Point Satisfies Fence
*/
            if( fenceLoad == TRUE )
              {
               if( numPts == 0 )
                 {
                  scanFeatureP->dtmFeaturePtsP = ( DPoint3d * ) malloc( maxPoints * sizeof(DPoint3d)) ;
                  if( scanFeatureP->dtmFeaturePtsP == nullptr )
                    {
                     bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                     goto errexit ;
                    }
                 }
               (scanFeatureP->dtmFeaturePtsP+numPts)->x = pntP->x ;
               (scanFeatureP->dtmFeaturePtsP+numPts)->y = pntP->y ;
               (scanFeatureP->dtmFeaturePtsP+numPts)->z = pntP->z ;
               ++numPts ;
              }
           }
        }
/*
**    Set Scan Feature Variables
*/
      if( numPts > 0 )
        {
         scanFeatureP->dtmFeatureType   = DTMFeatureType::RandomSpots  ;
         scanFeatureP->dtmFeatureId     = dtmP->nullFeatureId    ;
         scanFeatureP->dtmUserTag       = dtmP->nullUserTag ;
         scanFeatureP->numDtmFeaturePts = numPts ;
         *featureFoundP = TRUE ;
         scanPnt1 = p1  ;
        }
    break ;

    case  DTMFeatureType::Breakline :
    case  DTMFeatureType::ContourLine :
    case  DTMFeatureType::Void :
    case  DTMFeatureType::Island :
    case  DTMFeatureType::Hole :
    case  DTMFeatureType::GroupSpots :
    case  DTMFeatureType::Polygon :
    case  DTMFeatureType::Region :

       if( dbg ) bcdtmWrite_message(0,0,0,"Loading DTM Feature %2ld",dtmFeatureType) ;
/*
**     Check For Remaining Clipped Feature Sections To Be Loaded
*/
       if( numClipNext < numClipArrays )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Loading Remaining Clipped Sections") ;
          scanFeatureP->dtmFeatureType   = (DTMFeatureType)dtmFeatureType  ;
          scanFeatureP->dtmFeatureId          = clipFeatureP->dtmFeatureId  ;
          scanFeatureP->dtmUserTag       = clipFeatureP->dtmUserTag ;
          scanFeatureP->dtmFeaturePtsP   = clipArraysPP[numClipNext]->pointsP ;
          scanFeatureP->numDtmFeaturePts = clipArraysPP[numClipNext]->numPoints ;
          *featureFoundP = TRUE ;
          ++numClipNext ;
          if( numClipNext >= numClipArrays )
            {
             bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
             numClipNext = numClipArrays = 0 ;
            }
         }
/*
**     Scan Dtm Object And Extract All Occurrences For The DTM Feature Type
*/
       else
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Next Feature") ;
          bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,dtmFeatureType,firstCall,&dtmFeatureP,&dtmFeatureNum) ;
          if( dtmFeatureP != nullptr )
            {
             if( dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
               {
                if( bcdtmObject_getPointsForDtmFeatureDtmObject(dtmP,dtmFeatureNum,(DTM_TIN_POINT **) &featurePtsP ,&numFeaturePts)) goto errexit ;
                if( numFeaturePts > 0 )
                  {
                   fenceLoad  = TRUE ;
                   clipResult = 0 ;
                   if( useFence == TRUE )
                     {
                      fenceLoad  = FALSE ;
                      if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,featurePtsP,numFeaturePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                      if( clipResult == 1 || clipResult == 2 ) fenceLoad = TRUE ;
                     }
/*
**                 Set Scan Feature Variables
*/
                   if( fenceLoad == TRUE )
                     {
                      scanFeatureP->dtmFeatureType   = (DTMFeatureType)dtmFeatureType  ;
                      scanFeatureP->dtmFeatureId     = dtmFeatureP->dtmFeatureId  ;
                      scanFeatureP->dtmUserTag       = dtmFeatureP->dtmUserTag ;
                      if( ! clipResult )
                        {
                         scanFeatureP->dtmFeaturePtsP   = featurePtsP ;
                         scanFeatureP->numDtmFeaturePts = numFeaturePts ;
                        }
                      else
                        {
                         scanFeatureP->dtmFeaturePtsP   = clipArraysPP[0]->pointsP ;
                         scanFeatureP->numDtmFeaturePts = clipArraysPP[0]->numPoints ;
                         numClipNext  = 1 ;
                         clipFeatureP = dtmFeatureP ;
                         if( clipResult == 1 )
                           {
                            bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                            numClipNext = numClipArrays = 0 ;
                           }
                        }
                      *featureFoundP = TRUE ;
                      featurePtsP    = nullptr ;
                     }
                  }
               }
            }
         }
    break ;

    case  DTMFeatureType::Triangle :
/*
**    Write Start Message
*/
      if( dbg ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p",dtmP) ;
/*
**    Scan And Load Triangles
*/
      startTime = bcdtmClock() ;
      for( p1 = scanPnt1 ; p1 < lastScanPnt && *featureFoundP == FALSE  ; ++p1 )
        {
/*
**       Scan Start From Point
*/
         if( scanClPtr == dtmP->nullPtr )
           {
            nodeP = nodeAddrP(dtmP,p1) ;
            clPtr = nodeP->cPtr ;
            if( clPtr != dtmP->nullPtr )
              {
               p1P = pointAddrP(dtmP,p1) ;
               trgPts[0].x = p1P->x ;
               trgPts[0].y = p1P->y ;
               trgPts[0].z = p1P->z ;
               trgPts[3].x = p1P->x ;
               trgPts[3].y = p1P->y ;
               trgPts[3].z = p1P->z ;
               if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
              }
           }
/*
**       Scan Start From Previous Triangle
*/
         else
           {
            clPtr = scanClPtr ;
            nodeP = scanNodeP ;
            p2    = scanPnt2  ;
            scanNodeP = nullptr  ;
            scanClPtr = dtmP->nullPtr ;
           }
/*
**       Scan Cyclic List For Point
*/
         while ( clPtr != dtmP->nullPtr && *featureFoundP == FALSE )
           {
            clistP = clistAddrP(dtmP,clPtr) ;
            p3     = clistP->pntNum ;
            clPtr  = clistP->nextPtr ;
/*
**          Check For Valid Triangle
*/
            if( p2 > p1 && p3 > p1 && nodeP->hPtr != p2 )
              {
/*
**             Set Point Addresses
*/
               p2P = pointAddrP(dtmP,p2) ;
               p3P = pointAddrP(dtmP,p3) ;
/*
**             Set Point Coordinates
*/
               trgPts[1].x = p2P->x ;
               trgPts[1].y = p2P->y ;
               trgPts[1].z = p2P->z ;
               trgPts[2].x = p3P->x ;
               trgPts[2].y = p3P->y ;
               trgPts[2].z = p3P->z ;
/*
**             Apply Fence
*/
               fenceLoad = TRUE ;
               if( useFence == TRUE )
                 {
                  fenceLoad = FALSE ;
/*
**                Set Bounding Cube For Triangle
*/
                  xMin = xMax = trgPts[0].x ;
                  yMin = yMax = trgPts[0].y ;
                  if( trgPts[1].x < xMin ) xMin = trgPts[1].x ;
                  if( trgPts[1].x > xMax ) xMax = trgPts[1].x ;
                  if( trgPts[2].x < xMin ) xMin = trgPts[2].x ;
                  if( trgPts[2].x > xMax ) xMax = trgPts[2].x ;
                  if( trgPts[1].y < yMin ) yMin = trgPts[1].y ;
                  if( trgPts[1].y > yMax ) yMax = trgPts[1].y ;
                  if( trgPts[2].y < yMin ) yMin = trgPts[2].y ;
                  if( trgPts[2].y > yMax ) yMax = trgPts[2].y ;
/*
**                Check If Triangle Is Internal, Overlaps Or External To Fence
*/
                  trgExtent = DTMFenceOption::Outside ;
                  if( xMin <= clipDtmP->xMax && xMax >= clipDtmP->xMin && yMin <= clipDtmP->yMax && yMax >= clipDtmP->yMin )
                    {
                     if( fenceType == DTMFenceType::Block && xMin >= clipDtmP->xMin && xMax <= clipDtmP->xMax && yMin >= clipDtmP->yMin && yMax <= clipDtmP->yMax ) trgExtent = DTMFenceOption::Inside ;
                     else
                       {
                        if( bcdtmLoad_testForOverlapWithTinHullDtmObject(clipDtmP,trgPts,4,&trgExtent)) goto errexit ;
                       }
                    }
/*
**                Check If Triangle Is To Be Loaded
*/
                  if(  fenceOption == DTMFenceOption::Inside  && trgExtent == DTMFenceOption::Inside ) fenceLoad = TRUE ;
                  if(  fenceOption == DTMFenceOption::Overlap && ( trgExtent == DTMFenceOption::Inside || trgExtent == DTMFenceOption::Overlap ) ) fenceLoad = TRUE ;
                  if(  fenceOption == DTMFenceOption::Outside && trgExtent == DTMFenceOption::Outside ) fenceLoad = TRUE ;
                 }
/*
**             Check For Void Triangle
*/
               if( fenceLoad == TRUE && voidsInDtm )
                 {
                  if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidFlag)) goto errexit ;
                  if( voidFlag ) fenceLoad = FALSE ;
                 }
/*
**             Load Triangles
*/
               if( fenceLoad == TRUE )
                 {
                  if( scanFeatureP->dtmFeaturePtsP == nullptr )
                    {
                     scanFeatureP->dtmFeaturePtsP = ( DPoint3d * ) malloc( 4 * sizeof(DPoint3d)) ;
                     if( scanFeatureP->dtmFeaturePtsP == nullptr )
                       {
                        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                        goto errexit ;
                       }
                    }
                  memcpy(scanFeatureP->dtmFeaturePtsP,trgPts,4*sizeof(DPoint3d)) ;
                  scanFeatureP->numDtmFeaturePts = 4 ;
                  scanFeatureP->dtmFeatureType   = DTMFeatureType::Triangle  ;
                  scanFeatureP->dtmFeatureId          = dtmP->nullFeatureId    ;
                  scanFeatureP->dtmUserTag       = dtmP->nullUserTag ;
                  *featureFoundP = TRUE ;
/*
**                Set Context Scan Variables
*/
                  if( clPtr == dtmP->nullPtr ) scanPnt1 = p1 + 1 ;
                  else
                    {
                     scanPnt1  = p1 ;
                     scanPnt2  = p3 ;
                     scanClPtr = clPtr ;
                     scanNodeP = nodeP ;
                    }
                 }
              }
            p2  = p3 ;
           }
        }
  break ;


    case  DTMFeatureType::TinLine :
/*
**    Write Start Message
*/
      if( dbg ) bcdtmWrite_message(0,0,0,"Loading Tin Lines From DTM Object %p",dtmP) ;
/*
**    Scan And Load Triangles
*/
      startTime = bcdtmClock() ;
      for( p1 = scanPnt1 ; p1 < lastScanPnt && *featureFoundP == FALSE  ; ++p1 )
        {
/*
**       Scan Start From Point
*/
         if( scanClPtr == dtmP->nullPtr )
           {
            nodeP = nodeAddrP(dtmP,p1) ;
            clPtr = nodeP->cPtr ;
            if( clPtr != dtmP->nullPtr )
              {
               p1P = pointAddrP(dtmP,p1) ;
               trgPts[0].x = p1P->x ;
               trgPts[0].y = p1P->y ;
               trgPts[0].z = p1P->z ;
              }
           }
/*
**       Scan Start From Previous Line
*/
         else
           {
            clPtr = scanClPtr ;
            nodeP = scanNodeP ;
            scanNodeP = nullptr  ;
            scanClPtr = dtmP->nullPtr ;
           }
/*
**       Scan Cyclic List For Point
*/
         while ( clPtr != dtmP->nullPtr && *featureFoundP == FALSE )
           {
            clistP = clistAddrP(dtmP,clPtr) ;
            p2     = clistP->pntNum ;
            clPtr  = clistP->nextPtr ;
/*
**          Check For Valid Tin Line
*/
            if( p2 > p1 )
              {
/*
**             Set Point Addresses
*/
               p2P = pointAddrP(dtmP,p2) ;
/*
**             Set Point Coordinates
*/
               trgPts[1].x = p2P->x ;
               trgPts[1].y = p2P->y ;
               trgPts[1].z = p2P->z ;
/*
**             Apply Fence
*/
               fenceLoad = TRUE ;
               if( useFence == TRUE )
                 {
                  fenceLoad = FALSE ;
/*
**                Set Bounding Cube For Triangle
*/
                  xMin = xMax = trgPts[0].x ;
                  yMin = yMax = trgPts[0].y ;
                  if( trgPts[1].x < xMin ) xMin = trgPts[1].x ;
                  if( trgPts[1].x > xMax ) xMax = trgPts[1].x ;
                  if( trgPts[1].y < yMin ) yMin = trgPts[1].y ;
                  if( trgPts[1].y > yMax ) yMax = trgPts[1].y ;
/*
**                Check If Triangle Is Internal, Overlaps Or External To Fence
*/
                  trgExtent = DTMFenceOption::Outside ;
                  if( xMin <= clipDtmP->xMax && xMax >= clipDtmP->xMin && yMin <= clipDtmP->yMax && yMax >= clipDtmP->yMin )
                    {
                     if( fenceType == DTMFenceType::Block && xMin >= clipDtmP->xMin && xMax <= clipDtmP->xMax && yMin >= clipDtmP->yMin && yMax <= clipDtmP->yMax ) trgExtent = DTMFenceOption::Inside ;
                     else
                       {
                        if( bcdtmLoad_testForOverlapWithTinHullDtmObject(clipDtmP,trgPts,4,&trgExtent)) goto errexit ;
                       }
                    }
/*
**                Check If Triangle Is To Be Loaded
*/
                  if(  fenceOption == DTMFenceOption::Inside  && trgExtent == DTMFenceOption::Inside ) fenceLoad = TRUE ;
                  if(  fenceOption == DTMFenceOption::Overlap && ( trgExtent == DTMFenceOption::Inside || trgExtent == DTMFenceOption::Overlap ) ) fenceLoad = TRUE ;
                  if(  fenceOption == DTMFenceOption::Outside && trgExtent == DTMFenceOption::Outside ) fenceLoad = TRUE ;
                 }
/*
**             Check For Void Line
*/
               if( fenceLoad == TRUE && voidsInDtm )
                 {
                  if( bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidFlag)) goto errexit ;
                  if( voidFlag ) fenceLoad = FALSE ;
                 }
/*
**             Load Triangles
*/
               if( fenceLoad == TRUE )
                 {
                  if( scanFeatureP->dtmFeaturePtsP == nullptr )
                    {
                     scanFeatureP->dtmFeaturePtsP = ( DPoint3d * ) malloc( 2 * sizeof(DPoint3d)) ;
                     if( scanFeatureP->dtmFeaturePtsP == nullptr )
                       {
                        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                        goto errexit ;
                       }
                    }
                  memcpy(scanFeatureP->dtmFeaturePtsP,trgPts,2*sizeof(DPoint3d)) ;
                  scanFeatureP->numDtmFeaturePts = 2 ;
                  scanFeatureP->dtmFeatureType   = DTMFeatureType::TinLine  ;
                  scanFeatureP->dtmFeatureId          = dtmP->nullFeatureId    ;
                  scanFeatureP->dtmUserTag       = dtmP->nullUserTag ;
                  *featureFoundP = TRUE ;
/*
**                Set Context Scan Variables
*/
                  if( clPtr == dtmP->nullPtr ) scanPnt1 = p1 + 1 ;
                  else
                    {
                     scanPnt1  = p1 ;
                     scanClPtr = clPtr ;
                     scanNodeP = nodeP ;
                    }
                 }
              }
           }
        }
    break ;


    case  DTMFeatureType::TinHull :
    case  DTMFeatureType::TriangleEdge :
/*
**    Get Tin Hull Points
*/
      if( firstCall == TRUE )
        {
/*
**       Count Number Of Points In Tin Hull
*/
         if( dbg ) bcdtmWrite_message(0,0,0,"Loading Tin Hull") ;
         numHullPts = 0 ;
         p1 = dtmP->hullPoint ;
         do
           {
            ++numHullPts ;
            p1 = nodeAddrP(dtmP,p1)->hPtr ;
           } while ( p1 != dtmP->hullPoint ) ;
         ++numHullPts ;
/*
**       Allocate Memory For Hull Points
*/
         hullPtsP = ( DPoint3d * ) malloc ( numHullPts * sizeof(DPoint3d)) ;
         if( hullPtsP == nullptr )
           {
            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
            goto errexit ;
           }
/*
**       Copy Hull Points
*/
         p1 = dtmP->hullPoint ;
         p3dP = hullPtsP ;
         do
           {
            p1P = pointAddrP(dtmP,p1) ;
            p3dP->x = p1P->x ;
            p3dP->y = p1P->y ;
            p3dP->z = p1P->z ;
            ++p3dP ;
            p1 = nodeAddrP(dtmP,p1)->hPtr ;
           } while ( p1 != dtmP->hullPoint ) ;
         p1P = pointAddrP(dtmP,p1) ;
         p3dP->x = p1P->x ;
         p3dP->y = p1P->y ;
         p3dP->z = p1P->z ;
/*
**       Set Scan Hull Pointer For triangle Edges
*/
         if( dtmFeatureType == DTMFeatureType::TriangleEdge && useFence == FALSE ) scanHullPntP = hullPtsP ;
         else                                                            scanHullPntP = nullptr ;
/*
**       Clip Tin Hull To Fence
*/
         if( useFence )
           {
            if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,hullPtsP,numHullPts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
            numClipNext = 0 ;
           }
        }
/*
**    Check For Fence
*/
      if( useFence == FALSE )
        {
         if( dtmFeatureType == DTMFeatureType::TinHull && firstCall )
           {
            scanFeatureP->dtmFeaturePtsP   = hullPtsP ;
            scanFeatureP->numDtmFeaturePts = numHullPts ;
            scanFeatureP->dtmFeatureType   = DTMFeatureType::TinHull  ;
            scanFeatureP->dtmFeatureId          = dtmP->nullFeatureId    ;
            scanFeatureP->dtmUserTag       = dtmP->nullUserTag ;
            *featureFoundP = TRUE ;
            hullPtsP = nullptr ;
           }
         if( dtmFeatureType == DTMFeatureType::TriangleEdge )
           {
            for( p3dP = scanHullPntP ; p3dP < hullPtsP + numHullPts - 1 && *featureFoundP == FALSE ; ++p3dP )
              {
               scanFeatureP->dtmFeaturePtsP = ( DPoint3d * ) malloc( 5 * sizeof(DPoint3d)) ;
               if( scanFeatureP->dtmFeaturePtsP == nullptr )
                 {
                  bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                  goto errexit ;
                 }
               (scanFeatureP->dtmFeaturePtsP+0)->x = p3dP->x     ; (scanFeatureP->dtmFeaturePtsP+0)->y = p3dP->y     ; (scanFeatureP->dtmFeaturePtsP+0)->z = p3dP->z ;
               (scanFeatureP->dtmFeaturePtsP+1)->x = (p3dP+1)->x ; (scanFeatureP->dtmFeaturePtsP+1)->y = (p3dP+1)->y ; (scanFeatureP->dtmFeaturePtsP+1)->z = (p3dP+1)->z ;
               (scanFeatureP->dtmFeaturePtsP+2)->x = (p3dP+1)->x ; (scanFeatureP->dtmFeaturePtsP+2)->y = (p3dP+1)->y ; (scanFeatureP->dtmFeaturePtsP+2)->z = dtmP->zMin ;
               (scanFeatureP->dtmFeaturePtsP+3)->x = p3dP->x     ; (scanFeatureP->dtmFeaturePtsP+3)->y = p3dP->y     ; (scanFeatureP->dtmFeaturePtsP+3)->z = dtmP->zMin ;
               (scanFeatureP->dtmFeaturePtsP+4)->x = p3dP->x     ; (scanFeatureP->dtmFeaturePtsP+4)->y = p3dP->y     ; (scanFeatureP->dtmFeaturePtsP+4)->z = p3dP->z ;
               scanFeatureP->numDtmFeaturePts = 5 ;
               scanFeatureP->dtmFeatureType   = DTMFeatureType::TriangleEdge  ;
               scanFeatureP->dtmFeatureId     = dtmP->nullFeatureId    ;
               scanFeatureP->dtmUserTag       = dtmP->nullUserTag ;
               *featureFoundP = TRUE ;
               ++scanHullPntP ;
              }
           }
        }
/*
**    Load Clipped Sections
*/
      else
        {
         if( dtmFeatureType == DTMFeatureType::TinHull )
           {
            for( n = numClipNext ; n < numClipArrays && *featureFoundP == FALSE ; ++n )
              {
               scanFeatureP->dtmFeaturePtsP    = clipArraysPP[n]->pointsP ;
               scanFeatureP->numDtmFeaturePts  = clipArraysPP[n]->numPoints ;
               scanFeatureP->dtmFeatureType    = DTMFeatureType::TinHull  ;
               scanFeatureP->dtmFeatureId           = dtmP->nullFeatureId    ;
               scanFeatureP->dtmUserTag        = dtmP->nullUserTag ;
               *featureFoundP = TRUE ;
               clipArraysPP[n]->pointsP = nullptr ;
               clipArraysPP[n]->numPoints = 0  ;
               ++numClipNext ;
              }
           }
         if( dtmFeatureType == DTMFeatureType::TriangleEdge )
           {
            for( n = numClipNext ; n < numClipArrays && *featureFoundP == FALSE ; ++n )
              {
               if( scanHullPntP == nullptr ) scanHullPntP = clipArraysPP[n]->pointsP ;
               for( p3dP = scanHullPntP ; p3dP < clipArraysPP[n]->pointsP + clipArraysPP[n]->numPoints - 1 && *featureFoundP == FALSE ; ++p3dP )
                 {
                  scanFeatureP->dtmFeaturePtsP = ( DPoint3d * ) malloc( 5 * sizeof(DPoint3d)) ;
                  if( scanFeatureP->dtmFeaturePtsP == nullptr )
                    {
                     bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                     goto errexit ;
                    }
                  (scanFeatureP->dtmFeaturePtsP+0)->x = p3dP->x     ; (scanFeatureP->dtmFeaturePtsP+0)->y = p3dP->y     ; (scanFeatureP->dtmFeaturePtsP+0)->z = p3dP->z ;
                  (scanFeatureP->dtmFeaturePtsP+1)->x = (p3dP+1)->x ; (scanFeatureP->dtmFeaturePtsP+1)->y = (p3dP+1)->y ; (scanFeatureP->dtmFeaturePtsP+1)->z = (p3dP+1)->z ;
                  (scanFeatureP->dtmFeaturePtsP+2)->x = (p3dP+1)->x ; (scanFeatureP->dtmFeaturePtsP+2)->y = (p3dP+1)->y ; (scanFeatureP->dtmFeaturePtsP+2)->z = dtmP->zMin ;
                  (scanFeatureP->dtmFeaturePtsP+3)->x = p3dP->x     ; (scanFeatureP->dtmFeaturePtsP+3)->y = p3dP->y     ; (scanFeatureP->dtmFeaturePtsP+3)->z = dtmP->zMin ;
                  (scanFeatureP->dtmFeaturePtsP+4)->x = p3dP->x     ; (scanFeatureP->dtmFeaturePtsP+4)->y = p3dP->y     ; (scanFeatureP->dtmFeaturePtsP+4)->z = p3dP->z ;
                  scanFeatureP->numDtmFeaturePts = 5 ;
                  scanFeatureP->dtmFeatureType   = DTMFeatureType::TriangleEdge  ;
                  scanFeatureP->dtmFeatureId     = dtmP->nullFeatureId    ;
                  scanFeatureP->dtmUserTag       = dtmP->nullUserTag ;
                  *featureFoundP = TRUE ;
                  ++scanHullPntP ;
                  if( n > numClipNext ) numClipNext = n  ;
                 }
               if( *featureFoundP == FALSE ) scanHullPntP = nullptr ;
              }
           }
        }
  break ;

    default :
    break ;

   } ;
/*
** Clean Up
*/
 cleanup :
 if( *featureFoundP == FALSE )
   {
    if( clipDtmP     != nullptr ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
    if( featurePtsP  != nullptr ) { free( featurePtsP) ; featurePtsP = nullptr ; }
    if( pointMaskP   != nullptr ) { free(pointMaskP)   ; pointMaskP  = nullptr ; }
    if( hullPtsP     != nullptr ) { free(hullPtsP)     ; hullPtsP    = nullptr ; }
    if( clipArraysPP != nullptr ) bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
    if( scanFeatureP->dtmFeaturePtsP != nullptr ) { free(scanFeatureP->dtmFeaturePtsP) ; scanFeatureP->dtmFeaturePtsP = nullptr ; }
   }
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *featureFoundP = FALSE ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Load Occurrences Of A DTM Feature Type From A DTM Lattice File
* @doc    Load Occurrences Of A DTM Feature Type From A DTM Lattice File
* @notes  Requires the user load function to be prior defined by "bcdtmLoad_setDtmLoadFunction".
* @author Rob Cormack 1 November 2004  rob.cormack@bentley.con
* @param  *latticeFileP,        ==> Lattice File Name
* @param  dtmFeatureType,       ==> Type Of DTM Feature To Be Loaded
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence,             ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts,             ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmLoad_dtmFeatureTypeFromLatticeFile
(
 WCharCP latticeFileP,      /* ==> Lattice File Name                 */
 DTMFeatureType dtmFeatureType,     /* ==> Type Of DTM Feature To Be Loaded  */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function          */
 long    useFence,           /* ==> Load Feature Within Fence         */
 DTMFenceType fenceType,          /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>   */
 DTMFenceOption fenceOption,        /* ==> Fence Option <INSIDE(1),OVERLAP(2)>            */
 DPoint3d     *fencePtsP,         /* ==> DPoint3d Array Of Fence Points         */
 long    numFencePts,        /* ==> Number Of Fence Points            */
 void    *userP              /* ==> User Pointer Passed Back To User  */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTM_LAT_OBJ *latticeP=nullptr ;
/*
** Test If Requested Lattice File Is Current Lattice Object
*/
 if( bcdtmUtl_testForAndSetCurrentLatticeObject(&latticeP,latticeFileP)) goto errexit ;
/*
** Load Dtm Feature From Tin Object
*/
 if( bcdtmLoad_dtmFeatureTypeFromLatticeObject(latticeP,dtmFeatureType,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading DTM Feature Type From Lattice File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading DTM Feature Type From Lattice File Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Load Occurrences Of A DTM Feature Type From A DTM Lattice Object
* @doc    Load Occurrences Of A DTM Feature Type From A DTM Lattice Object
* @notes  Requires the user load function to be prior defined by "bcdtmLoad_setDtmLoadFunction".
* @author Rob Cormack 1 November 2004  rob.cormack@bentley.con
* @param  *latticeP,            ==> Pointer To Lattice Object
* @param  dtmFeatureType,       ==> DTM Feature Type To Be Loaded
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence,             ==> Load Feature Within Fence <TRUE,FALSE>
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts,             ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmLoad_dtmFeatureTypeFromLatticeObject
(
 DTM_LAT_OBJ  *latticeP,      /* ==> Pointer To Lattice Object         */
 DTMFeatureType dtmFeatureType,      /* ==> Type Of DTM Feature To Be Loaded  */
 DTMFeatureCallback loadFunctionP, /* ==> Pointer To Load Function          */
 long    useFence,            /* ==> Load Feature Within Fence         */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>   */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>            */
 DPoint3d     *fencePtsP,          /* ==> DPoint3d Array Of Fence Points         */
 long    numFencePts,         /* ==> Number Of Fence Points            */
 void    *userP               /* ==> User Pointer Passed Back To User  */
)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 char   dtmFeatureTypeName[128] ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Loading DTM Feature Type From Lattice Object %p",latticeP) ;
    bcdtmWrite_message(0,0,0,"latticeP         = %p",latticeP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType   = %8ld",dtmFeatureType) ;
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"*** Feature Name = %s",dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP    = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence         = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType        = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption      = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP        = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts      = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP            = %p",userP) ;
   }
/*
** Initialise
*/
 if( useFence == TRUE && ( fencePtsP == nullptr || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
/*
** Test For Valid Lattice Object
*/
// if( bcdtmObject_testForValidLatticeObject(latticeP) ) goto errexit ;
/*
** Test For Valid DTM Lattice Object Dtm Feature Type
*/
 if( bcdtmData_testForValidLatticeObjectDtmFeatureType(dtmFeatureType) == DTM_ERROR )
   {
    bcdtmWrite_message(2,0,0,"Invalid Dtm Feature Type %4ld",dtmFeatureType) ;
    goto errexit ;
   }
/*
** Load DTM Feature Type
*/
 if( bcdtmLoad_dtmFeatureTypeOccurrencesFromLatticeObject(latticeP,dtmFeatureType,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP) ) goto errexit ;
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
BENTLEYDTM_Private int bcdtmLoad_dtmFeatureTypeOccurrencesFromLatticeObject
(
 DTM_LAT_OBJ *latticeP,
 DTMFeatureType dtmFeatureType,
 DTMFeatureCallback loadFunctionP,
 long useFence,
 DTMFenceType fenceType,
 DTMFenceOption fenceOption,
 DPoint3d *fencePtsP,
 long numFencePts,
 void *userP
)
/*
** This Function Loads All Occurrences of Dtm Feature Type From A Lattice Object
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    i,j,n,pnt1,pnt2,pnt3,fenceLoad,insideFence ;
 long    fndType,numLinePts,numClipArrays,clipResult ;
 double  x1,y1,x2,y2,z1,z2,z3,z4 ;
 float   nullLatticeValue=(float)0.0 ;
 char    dtmFeatureTypeName[128] ;
 DPoint3d     latticePoint,latticePts[5],*linePtsP=nullptr ;
 BC_DTM_OBJ  *clipDtmP=nullptr ;
 DTM_POINT_ARRAY **clipArraysPP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Loading DTM Feature Type Occurrences From Lattice Object") ;
    bcdtmWrite_message(0,0,0,"Lattice Object   = %p",latticeP) ;
    bcdtmWrite_message(0,0,0,"Dtm Feature Type = %8ld",dtmFeatureType) ;
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"*** Feature Name = %s",dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"Load Function    = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"Use Fence        = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"Fence Type       = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"Fence Option     = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"Fence Points     = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"Num Fence Points = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"User Pointer     = %p",userP) ;
   }
/*
** Initialise
*/
 nullLatticeValue = latticeP->NULLVAL ;
/*
** Build Clipping Tin For Fence Operations
*/
 if( useFence == TRUE ) if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
/*
** Switch Depending On Data Feature Type
*/
 switch( dtmFeatureType )
   {
    case  DTMFeatureType::LatticePoint :
      if( dbg ) bcdtmWrite_message(0,0,0,"Loading Lattice Points") ;
      for( i = 0 ; i < latticeP->NYL  - 1 ; ++i )
        {
         for( j = 0 ; j < latticeP->NXL  - 1 ; ++j )
           {
            latticePoint.z = *(latticeP->LAT + j*latticeP->NYL  + i)  ;
            if( latticePoint.z != nullLatticeValue  )
              {
               latticePoint.x = latticeP->DX * i + latticeP->LXMIN ;
               latticePoint.y = latticeP->DY * j + latticeP->LYMIN ;
               if( useFence == FALSE )
                 {
                  if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,&latticePoint,1,userP)) goto errexit ;
                 }
               else
                 {
                  insideFence = FALSE ;
                  if( fenceType == DTMFenceType::Block && latticePoint.x >= clipDtmP->xMin && latticePoint.x <= clipDtmP->xMax && latticePoint.y >= clipDtmP->yMin && latticePoint.y <= clipDtmP->yMax ) insideFence = TRUE ; ;
                  if( fenceType == DTMFenceType::Shape && latticePoint.x >= clipDtmP->xMin && latticePoint.x <= clipDtmP->xMax && latticePoint.y >= clipDtmP->yMin && latticePoint.y <= clipDtmP->yMax )
                    {
                     if( bcdtmFind_triangleDtmObject(clipDtmP,latticePoint.x,latticePoint.y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
                     if( fndType ) insideFence = TRUE ;
                    }
                  fenceLoad = FALSE ;
                  if( ( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )  && insideFence == TRUE  ) fenceLoad = TRUE ;
                  if(   fenceOption == DTMFenceOption::Outside && insideFence == FALSE ) fenceLoad = TRUE ;
                  if( fenceLoad == TRUE )
                    {
                     if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,&latticePoint,1,userP)) goto errexit ;
                    }
                 }
              }
           }
        }
    break ;

    case  DTMFeatureType::Lattice :
      if( dbg ) bcdtmWrite_message(0,0,0,"Loading Lattice") ;
      for( i = 0 ; i < latticeP->NYL  - 1 ; ++i )
        {
         for( j = 0 ; j < latticeP->NXL  - 1 ; ++j )
           {
            z1 = *(latticeP->LAT + j*latticeP->NYL  + i)     ; z2 = *(latticeP->LAT+ j*latticeP->NYL  + i + 1) ;
            z3 = *(latticeP->LAT + (j+1)*latticeP->NYL  + i) ; z4 = *(latticeP->LAT + (j+1)*latticeP->NYL  + i + 1) ;
            if( z1 != nullLatticeValue && z2 != nullLatticeValue && z3 != nullLatticeValue && z4 != nullLatticeValue )
              {
               x1 = latticeP->DX * i  ; x2 = x1 + latticeP->DX ;
               y1 = latticeP->DY * j  ; y2 = y1 + latticeP->DY ;
               latticePts[0].x = x1 + latticeP->LXMIN ; latticePts[0].y = y1 +  latticeP->LYMIN  ; latticePts[0].z = z1 ;
               latticePts[1].x = x2 + latticeP->LXMIN ; latticePts[1].y = y1 +  latticeP->LYMIN  ; latticePts[1].z = z2 ;
               latticePts[2].x = x2 + latticeP->LXMIN ; latticePts[2].y = y2 +  latticeP->LYMIN  ; latticePts[2].z = z4 ;
               latticePts[3].x = x1 + latticeP->LXMIN ; latticePts[3].y = y2 +  latticeP->LYMIN  ; latticePts[3].z = z3 ;
               latticePts[4].x = x1 + latticeP->LXMIN ; latticePts[4].y = y1 +  latticeP->LYMIN  ; latticePts[4].z = z1  ;
/*
**             Load Feature
*/
               if( useFence == FALSE )
                 {
                  if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,latticePts,5,userP)) goto errexit ;
                 }
/*
**             Check Lattice Lies In Fence Before Loading
*/
               else
                 {
                  if( x1 <= clipDtmP->xMax && x2 >= clipDtmP->xMin && y1 <= clipDtmP->yMax && y2 >= clipDtmP->yMin )
                    {
                     if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,latticePts,5,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                     if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,latticePts,5,userP)) goto errexit ;
                     if( clipResult == 2 )
                       {
                        for( n = 0 ; n < numClipArrays ; ++n )
                          {
                           if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                          }
                        bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                       }
                    }
                 }
              }
           }
        }
    break ;

/*
** Write Edge Rectangles
*/
    case DTMFeatureType::LatticeEdge :
      if( dbg ) bcdtmWrite_message(0,0,0,"Loading Lattice Edges") ;
      for( i = 0 ; i < latticeP->NYL  - 1 ; ++i )
        {
         for( j = 0 ; j < latticeP->NXL  - 1 ; ++j )
           {
            z1 = *(latticeP->LAT + j*latticeP->NYL  + i) ; z2 = *(latticeP->LAT+ j*latticeP->NYL  + i + 1) ;
            z3 = *(latticeP->LAT + (j+1)*latticeP->NYL  + i) ; z4 = *(latticeP->LAT + (j+1)*latticeP->NYL  + i + 1) ;
            x1 = latticeP->DX * i  ; x2 = x1 + latticeP->DX ;
            y1 = latticeP->DY * j  ; y2 = y1 + latticeP->DY ;
            if( ( z1 == nullLatticeValue || z2 == nullLatticeValue || j == latticeP->NXL  - 2 ) && z3 != nullLatticeValue && z4 != nullLatticeValue )
              {
               latticePts[0].x = x1 + latticeP->LXMIN ; latticePts[0].y = y2 +  latticeP->LYMIN  ; latticePts[0].z = z3 ;
               latticePts[1].x = x2 + latticeP->LXMIN ; latticePts[1].y = y2 +  latticeP->LYMIN  ; latticePts[1].z = z4 ;
               latticePts[2].x = x2 + latticeP->LXMIN ; latticePts[2].y = y2 +  latticeP->LYMIN  ; latticePts[2].z = latticeP->LZMIN ;
               latticePts[3].x = x1 + latticeP->LXMIN ; latticePts[3].y = y2 +  latticeP->LYMIN  ; latticePts[3].z = latticeP->LZMIN ;
               latticePts[4].x = x1 + latticeP->LXMIN ; latticePts[4].y = y2 +  latticeP->LYMIN  ; latticePts[4].z = z3 ;
               if( useFence == FALSE ) { if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,latticePts,5,userP)) goto errexit ; }
               else
                 {
                  if( x1 <= clipDtmP->xMax && x2 >= clipDtmP->xMin && y1 <= clipDtmP->yMax && y2 >= clipDtmP->yMin )
                    {
                     if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,latticePts,5,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                     if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,latticePts,5,userP)) goto errexit ;
                     if( clipResult == 2 )
                       {
                        for( n = 0 ; n < numClipArrays ; ++n )
                          {
                           if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                          }
                        bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                       }
                    }
                 }
              }
            if( z1 != nullLatticeValue && z2 != nullLatticeValue && ( z3 == nullLatticeValue || z4 == nullLatticeValue || j == 0 ) )
              {
               latticePts[0].x = x1 + latticeP->LXMIN ; latticePts[0].y = y1 +  latticeP->LYMIN  ; latticePts[0].z = z1 ;
               latticePts[1].x = x2 + latticeP->LXMIN ; latticePts[1].y = y1 +  latticeP->LYMIN  ; latticePts[1].z = z2 ;
               latticePts[2].x = x2 + latticeP->LXMIN ; latticePts[2].y = y1 +  latticeP->LYMIN  ; latticePts[2].z = latticeP->LZMIN ;
               latticePts[3].x = x1 + latticeP->LXMIN ; latticePts[3].y = y1 +  latticeP->LYMIN  ; latticePts[3].z = latticeP->LZMIN ;
               latticePts[4].x = x1 + latticeP->LXMIN ; latticePts[4].y = y1 +  latticeP->LYMIN  ; latticePts[4].z = z1  ;
               if( useFence == FALSE ) { if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,latticePts,5,userP)) goto errexit ; }
               else
                 {
                  if( x1 <= clipDtmP->xMax && x2 >= clipDtmP->xMin && y1 <= clipDtmP->yMax && y2 >= clipDtmP->yMin )
                    {
                     if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,latticePts,5,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                     if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,latticePts,5,userP)) goto errexit ;
                     if( clipResult == 2 )
                       {
                        for( n = 0 ; n < numClipArrays ; ++n )
                          {
                           if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                          }
                        bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                       }
                    }
                 }
              }
           }
        }
      for( j = 0 ; j < latticeP->NXL  - 1 ; ++j )
        {
         for( i = 0 ; i < latticeP->NYL  - 1 ; ++i )
           {
            z1 = *(latticeP->LAT + j*latticeP->NYL  + i) ; z2 = *(latticeP->LAT+ j*latticeP->NYL  + i + 1) ;
            z3 = *(latticeP->LAT + (j+1)*latticeP->NYL  + i) ; z4 = *(latticeP->LAT + (j+1)*latticeP->NYL  + i + 1) ;
            x1 = latticeP->DX * i  ; x2 = x1 + latticeP->DX ;
            y1 = latticeP->DY * j  ; y2 = y1 + latticeP->DY ;
            if(( z1 == nullLatticeValue || z3 == nullLatticeValue || i == latticeP->NYL  - 2 ) && z2 != nullLatticeValue && z4 != nullLatticeValue )
              {
               latticePts[0].x = x2 + latticeP->LXMIN ; latticePts[0].y = y2 +  latticeP->LYMIN  ; latticePts[0].z = z2 ;
               latticePts[1].x = x2 + latticeP->LXMIN ; latticePts[1].y = y2 +  latticeP->LYMIN  ; latticePts[1].z = z4 ;
               latticePts[2].x = x2 + latticeP->LXMIN ; latticePts[2].y = y2 +  latticeP->LYMIN  ; latticePts[2].z = latticeP->LZMIN ;
               latticePts[3].x = x2 + latticeP->LXMIN ; latticePts[3].y = y1 +  latticeP->LYMIN  ; latticePts[3].z = latticeP->LZMIN ;
               latticePts[4].x = x2 + latticeP->LXMIN ; latticePts[4].y = y1 +  latticeP->LYMIN  ; latticePts[4].z = z2 ;
               if( useFence == FALSE ) { if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,latticePts,5,userP)) goto errexit ; }
               else
                 {
                  if( x1 <= clipDtmP->xMax && x2 >= clipDtmP->xMin && y1 <= clipDtmP->yMax && y2 >= clipDtmP->yMin )
                    {
                     if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,latticePts,5,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                     if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,latticePts,5,userP)) goto errexit ;
                     if( clipResult == 2 )
                       {
                        for( n = 0 ; n < numClipArrays ; ++n )
                          {
                           if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                          }
                        bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                       }
                    }
                 }
              }
            if( z1 != nullLatticeValue && z3 != nullLatticeValue && ( z2 == nullLatticeValue || z4 == nullLatticeValue || i == 0 ) )
              {
               latticePts[0].x = x1 + latticeP->LXMIN ; latticePts[0].y = y1 +  latticeP->LYMIN  ; latticePts[0].z = z1 ;
               latticePts[1].x = x1 + latticeP->LXMIN ; latticePts[1].y = y2 +  latticeP->LYMIN  ; latticePts[1].z = z3 ;
               latticePts[2].x = x1 + latticeP->LXMIN ; latticePts[2].y = y2 +  latticeP->LYMIN  ; latticePts[2].z = latticeP->LZMIN ;
               latticePts[3].x = x1 + latticeP->LXMIN ; latticePts[3].y = y1 +  latticeP->LYMIN  ; latticePts[3].z = latticeP->LZMIN ;
               latticePts[4].x = x1 + latticeP->LXMIN ; latticePts[4].y = y1 +  latticeP->LYMIN  ; latticePts[4].z = z1 ;
               if( useFence == FALSE ) { if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,latticePts,5,userP)) goto errexit ; }
               else
                 {
                  if( x1 <= clipDtmP->xMax && x2 >= clipDtmP->xMin && y1 <= clipDtmP->yMax && y2 >= clipDtmP->yMin )
                    {
                     if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,latticePts,5,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                     if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,latticePts,5,userP)) goto errexit ;
                     if( clipResult == 2 )
                       {
                        for( n = 0 ; n < numClipArrays ; ++n )
                          {
                           if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                          }
                        bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                       }
                    }
                 }
              }
           }
        }
    break ;

    case DTMFeatureType::LatticeXLine  :
      if( dbg ) bcdtmWrite_message(0,0,0,"Loading Lattice x Lines") ;
/*
**    Allocate Memory To Store Line Points
*/
      linePtsP = ( DPoint3d * ) malloc( latticeP->NYL * sizeof(DPoint3d)) ;
      if( linePtsP == nullptr )
        {
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit ;
        }
/*
**    Scan Lattice Rows
*/
      for( j = 0 ; j < latticeP->NXL ; ++j )
        {
         numLinePts = 0 ;
         y1 = latticeP->DY * j  + latticeP->LYMIN ;
         for( i = 0 ; i < latticeP->NYL ; ++i )
           {
            x1 = latticeP->DX * i  + latticeP->LXMIN ;
            z1 = *(latticeP->LAT + j*latticeP->NYL + i)  ;
            if( z1 == latticeP->NULLVAL )
              {
               if( numLinePts > 1 )
                 {
                  if( useFence == FALSE ){ if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,linePtsP,numLinePts,userP)) goto errexit ; }
                  else
                    {
                     if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,linePtsP,numLinePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                     if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,linePtsP,numLinePts,userP)) goto errexit ;
                     if( clipResult == 2 )
                       {
                        for( n = 0 ; n < numClipArrays ; ++n )
                          {
                           if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                          }
                        bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                       }
                    }
                 }
               numLinePts = 0 ;
              }
            else
              {
               (linePtsP+numLinePts)->x = x1 ;
               (linePtsP+numLinePts)->y = y1 ;
               (linePtsP+numLinePts)->z = z1 ;
               ++numLinePts ;
              }
           }
/*
**       Load Last Section
*/
         if( numLinePts > 1 )
           {
            if( useFence == FALSE ) { if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,linePtsP,numLinePts,userP)) goto errexit ; }
            else
              {
               if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,linePtsP,numLinePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
               if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,linePtsP,numLinePts,userP)) goto errexit ;
               if( clipResult == 2 )
                 {
                  for( n = 0 ; n < numClipArrays ; ++n )
                    {
                     if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                    }
                  bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                 }
              }
           }
        }
    break ;

    case DTMFeatureType::LatticeYLine  :
      if( dbg ) bcdtmWrite_message(0,0,0,"Loading Lattice y Lines") ;
/*
**    Allocate Memory To Store Line Points
*/
      linePtsP = ( DPoint3d * ) malloc( latticeP->NYL * sizeof(DPoint3d)) ;
      if( linePtsP == nullptr )
        {
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit ;
        }
/*
**    Scan Lattice
*/
      for( i = 0 ; i < latticeP->NYL  ; ++i )
        {
         numLinePts = 0 ;
         x1 = latticeP->DX * i  + latticeP->LXMIN ;
         for( j = 0 ; j < latticeP->NXL ; ++j )
           {
            y1 = latticeP->DY * j  + latticeP->LYMIN ;
            z1 = *(latticeP->LAT + j*latticeP->NYL + i)  ;
            if( z1 == latticeP->NULLVAL )
              {
               if( numLinePts > 1 )
                 {
                  if( useFence == FALSE ) { if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,linePtsP,numLinePts,userP)) goto errexit ; }
                  else
                    {
                     if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,linePtsP,numLinePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                     if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,linePtsP,numLinePts,userP)) goto errexit ;
                     if( clipResult == 2 )
                       {
                        for( n = 0 ; n < numClipArrays ; ++n )
                          {
                           if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                          }
                        bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                       }
                    }
                 }
               numLinePts = 0 ;
              }
            else
              {
               (linePtsP+numLinePts)->x = x1 ;
               (linePtsP+numLinePts)->y = y1 ;
               (linePtsP+numLinePts)->z = z1 ;
               ++numLinePts ;
              }
           }
/*
**       Load Last Section
*/
         if( numLinePts > 1 )
           {
            if( useFence == FALSE ) { if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,linePtsP,numLinePts,userP)) goto errexit ; }
            else
              {
               if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,linePtsP,numLinePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
               if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,linePtsP,numLinePts,userP)) goto errexit ;
               if( clipResult == 2 )
                 {
                  for( n = 0 ; n < numClipArrays ; ++n )
                    {
                     if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                    }
                  bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                 }
              }
           }
        }
    break ;

    default :
    break  ;
   }
/*
** Clean Up
*/
 cleanup :
 if( clipDtmP     != nullptr ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 if( clipArraysPP != nullptr ) bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading DTM Feature Type Occurrences From Lattice Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading DTM Feature Type Occurrences From Lattice Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Scan Load Next DTM Feature Occurence For A Feature Id From A DTM Object
* @doc    Scan Load Next DTM Feature Occurence For A Feature Id From A DTM Object
* @author Rob Cormack September 2007  rob.cormack@bentley.con
* @param  dtmP                  ==> Pointer To DTM object
* @param  userFeatureId         ==> Feature Id Value
* @param  firstCall             ==> First Call <TRUE,FALSE>
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  featureFoundP         <== Feature Found <TRUE,FALSE>
* @param  scanFeature           <== Feature Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmScanLoad_nextDtmFeatureOccurrenceForDtmFeatureIdFromDtmObject
(
 BC_DTM_OBJ  *dtmP,                 /* ==> Pointer To Dtm Object                        */
 DTMFeatureId userFeatureId,      /* ==> Dtm Feature Id Value                         */
 long     firstCall,                /* ==> First Call <TRUE,FALSE>                      */
 long     useFence,                 /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DTMFenceType  fenceType,                /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape> */
 DTMFenceOption fenceOption,              /* ==> Fence Option <INSIDE(1),OVERLAP(2)>          */
 DPoint3d      *fencePtsP,               /* ==> DPoint3d Array Of Fence Points                    */
 long     numFencePts,              /* ==> Number Of Fence Points                       */
 long     *featureFoundP,           /* <== Scan Feature Found<TRUE,FALSE>               */
 BC_DTM_USER_FEATURE *scanFeatureP  /* <== Pointer To Scan Feature                      */
)
/*
** This Function Loads The Next Dtm Feature Occurrence For A Feature Id
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long            clipResult,numFeaturePts,fenceLoad ;
 DPoint3d             *featurePtsP=nullptr ;
 char            dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE  *dtmFeatureP ;
/*
** Static Variables For Maintaining Scan Context
*/
 static long dtmFeature=0  ;
/*
** Static Variables For Maintaining Clipping Context
*/
 static long numClipNext=0,numClipArrays=0,numHullPts=0 ;
 static BC_DTM_OBJ       *clipDtmP=nullptr ;
 static DTM_POINT_ARRAY  **clipArraysPP=nullptr ;
 static BC_DTM_FEATURE   *clipFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scan Loading Next Dtm Feature Occurrence For A Feature Id") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"userFeatureId     = %8ld",userFeatureId) ;
    bcdtmWrite_message(0,0,0,"firstCall         = %8ld",firstCall) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
   }
/*
** Initialise
*/
 *featureFoundP = FALSE ;
/*
** Do First Call Processing
*/
 if( firstCall == TRUE )
   {
/*
**  Check Scan Feature Points Pointer
*/
    scanFeatureP->numDtmFeaturePts = 0 ;
    if( scanFeatureP->dtmFeaturePtsP != nullptr )
      {
       free(scanFeatureP->dtmFeaturePtsP) ;
       scanFeatureP->dtmFeaturePtsP = nullptr ;
      }
/*
**  Validate Fence
*/
    if( useFence == TRUE && ( fencePtsP == nullptr || numFencePts <= 2 ) ) useFence = FALSE ;
    if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
    if( useFence )
      {
       if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
       if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
      }
/*
**  Test For Valid Dtm Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
    if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
**  Initialise Static Clip And Scan Variables
*/
    dtmFeature    = 0 ;
    numClipNext   = 0 ;
    numClipArrays = 0 ;
/*
**  Build Clipping Dtm For Fence Operations
*/
    if( useFence == TRUE )
      {
/*
**     Build Clipping Tin
*/
       if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
      }
   }
/*
** Free Scan Feature Points Memory
*/
 if( scanFeatureP->dtmFeaturePtsP != nullptr ) { free(scanFeatureP->dtmFeaturePtsP) ; scanFeatureP->dtmFeaturePtsP = nullptr ; }
 scanFeatureP->numDtmFeaturePts = 0 ;
/*
** Check For Remaining Clipped Feature Sections To Be Loaded
*/
 if( numClipNext < numClipArrays )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Loading Remaining Clipped Sections") ;
    scanFeatureP->dtmFeatureType   = (DTMFeatureType)clipFeatureP->dtmFeatureType  ;
    scanFeatureP->dtmFeatureId     = clipFeatureP->dtmFeatureId  ;
    scanFeatureP->dtmUserTag       = clipFeatureP->dtmUserTag ;
    scanFeatureP->dtmFeaturePtsP   = clipArraysPP[numClipNext]->pointsP ;
    scanFeatureP->numDtmFeaturePts = clipArraysPP[numClipNext]->numPoints ;
    *featureFoundP = TRUE ;
    ++numClipNext ;
    if( numClipNext >= numClipArrays )
      {
       bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
       numClipNext = numClipArrays = 0 ;
      }
   }
/*
** Scan Dtm Object And Extract All Dtm Feature Occurrences For A Feature Id
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Next Feature") ;
    while ( dtmFeature < dtmP->numFeatures && *featureFoundP == FALSE )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError &&
           dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback &&
           dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted   &&
           dtmFeatureP->dtmFeatureId    == userFeatureId           )
         {
          if( dbg )
            {
             bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
             bcdtmWrite_message(0,0,0,"Feature Found ** dtmFeatureType = %s ** dtmFeatureState = %4ld ** dtmP->dtmState = %2ld",dtmFeatureTypeName,dtmFeatureP->dtmFeatureState,dtmP->dtmState) ;
            }
          // RobC. 2008/04/02 Kludge Fix
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull && dtmP->dtmState == DTMState::Tin && dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray ) dtmFeatureP->dtmFeatureState = DTMFeatureState::Tin ;
          if(dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull && dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
             if( bcdtmList_extractHullDtmObject(dtmP,&featurePtsP,&numFeaturePts)) goto errexit ;
            }
          else if( bcdtmObject_getPointsForDtmFeatureDtmObject(dtmP,dtmFeature,(DTM_TIN_POINT **) &featurePtsP ,&numFeaturePts)) goto errexit ;
          if( numFeaturePts > 0 )
            {
             fenceLoad  = TRUE ;
             clipResult = 0 ;
             if( useFence == TRUE )
               {
                fenceLoad  = FALSE ;
                if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,featurePtsP,numFeaturePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                if( clipResult == 1 || clipResult == 2 ) fenceLoad = TRUE ;
               }
/*
**           Set Scan Feature Variables
*/
             if( fenceLoad == TRUE )
               {
                scanFeatureP->dtmFeatureType   = (DTMFeatureType)dtmFeatureP->dtmFeatureType  ;
                scanFeatureP->dtmFeatureId     = dtmFeatureP->dtmFeatureId  ;
                scanFeatureP->dtmUserTag       = dtmFeatureP->dtmUserTag ;
                if( ! clipResult )
                  {
                   scanFeatureP->dtmFeaturePtsP   = featurePtsP ;
                   scanFeatureP->numDtmFeaturePts = numFeaturePts ;
                  }
                else
                  {
                   scanFeatureP->dtmFeaturePtsP   = clipArraysPP[0]->pointsP ;
                   scanFeatureP->numDtmFeaturePts = clipArraysPP[0]->numPoints ;
                   numClipNext  = 1 ;
                   clipFeatureP = dtmFeatureP ;
                   if( clipResult == 1 )
                     {
                      bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                      numClipNext = numClipArrays = 0 ;
                     }
                  }
                *featureFoundP = TRUE ;
                featurePtsP    = nullptr ;
               }
            }
         }
       ++dtmFeature ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( *featureFoundP == FALSE )
   {
    dtmFeature = 0 ;
    if( clipDtmP     != nullptr ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
    if( featurePtsP  != nullptr ) { free( featurePtsP) ; featurePtsP = nullptr ; }
    if( clipArraysPP != nullptr ) bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
    if( scanFeatureP->dtmFeaturePtsP != nullptr ) { free(scanFeatureP->dtmFeaturePtsP) ; scanFeatureP->dtmFeaturePtsP = nullptr ; }
   }
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *featureFoundP = FALSE ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Scan Load Next DTM Feature Occurence For A User Tag From A DTM Object
* @doc    Scan Load Next DTM Feature Occurence For A User Tag From A DTM Object
* @author Rob Cormack September 2007  rob.cormack@bentley.con
* @param  dtmP                  ==> Pointer To DTM object
* @param  userTag               ==> userTag
* @param  firstCall             ==> First Call <TRUE,FALSE>
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  featureFoundP         <== Feature Found <TRUE,FALSE>
* @param  scanFeature           <== Feature Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmScanLoad_nextDtmFeatureOccurrenceForUserTagFromDtmObject
(
 BC_DTM_OBJ  *dtmP,                 /* ==> Pointer To Dtm Object                        */
 DTMUserTag userTag,              /* ==> User Tag                                     */
 long     firstCall,                /* ==> First Call <TRUE,FALSE>                      */
 long     useFence,                 /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DTMFenceType fenceType,                /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape> */
 DTMFenceOption fenceOption,              /* ==> Fence Option <INSIDE(1),OVERLAP(2)>          */
 DPoint3d      *fencePtsP,               /* ==> DPoint3d Array Of Fence Points                    */
 long     numFencePts,              /* ==> Number Of Fence Points                       */
 long     *featureFoundP,           /* <== Scan Feature Found<TRUE,FALSE>               */
 BC_DTM_USER_FEATURE *scanFeatureP  /* <== Pointer To Scan Feature                      */
)
/*
** This Function Loads The Next Dtm Feature Occurrence For A User Tag
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long            clipResult,numFeaturePts,fenceLoad ;
 DPoint3d             *featurePtsP=nullptr ;
 BC_DTM_FEATURE  *dtmFeatureP ;
/*
** Static Variables For Maintaining Scan Context
*/
 static long dtmFeature=0  ;
/*
** Static Variables For Maintaining Clipping Context
*/
 static long numClipNext=0,numClipArrays=0,numHullPts=0 ;
 static BC_DTM_OBJ       *clipDtmP=nullptr ;
 static DTM_POINT_ARRAY  **clipArraysPP=nullptr ;
 static BC_DTM_FEATURE   *clipFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Scan Loading Next Dtm Feature Occurrence For A User Tag") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"userTag           = %8I64d",userTag) ;
    bcdtmWrite_message(0,0,0,"firstCall         = %8ld",firstCall) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
   }
/*
** Initialise
*/
 *featureFoundP = FALSE ;
/*
** Do First Call Processing
*/
 if( firstCall == TRUE )
   {
/*
**  Check Scan Feature Points Pointer
*/
    if( scanFeatureP->dtmFeaturePtsP != nullptr )
      {
       bcdtmWrite_message(2,0,0,"Pointer To Scan Feature Points Is Not nullptr") ;
       goto errexit ;
      }
/*
**  Validate Fence
*/
    if( useFence == TRUE && ( fencePtsP == nullptr || numFencePts <= 2 ) ) useFence = FALSE ;
    if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
    if( useFence )
      {
       if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
       if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
      }
/*
**  Test For Valid Dtm Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
    if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
**  Initialise Static Clip And Scan Variables
*/
    dtmFeature    = 0 ;
    numClipNext   = 0 ;
    numClipArrays = 0 ;
/*
**  Build Clipping Dtm For Fence Operations
*/
    if( useFence == TRUE )
      {
/*
**     Build Clipping Tin
*/
       if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
      }
   }
/*
** Free Scan Feature Points Memory
*/
 if( scanFeatureP->dtmFeaturePtsP != nullptr ) { free(scanFeatureP->dtmFeaturePtsP) ; scanFeatureP->dtmFeaturePtsP = nullptr ; }
 scanFeatureP->numDtmFeaturePts = 0 ;
/*
** Check For Remaining Clipped Feature Sections To Be Loaded
*/
 if( numClipNext < numClipArrays )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Loading Remaining Clipped Sections") ;
    scanFeatureP->dtmFeatureType   = (DTMFeatureType)clipFeatureP->dtmFeatureType  ;
    scanFeatureP->dtmFeatureId     = clipFeatureP->dtmFeatureId  ;
    scanFeatureP->dtmUserTag       = clipFeatureP->dtmUserTag ;
    scanFeatureP->dtmFeaturePtsP   = clipArraysPP[numClipNext]->pointsP ;
    scanFeatureP->numDtmFeaturePts = clipArraysPP[numClipNext]->numPoints ;
    *featureFoundP = TRUE ;
    ++numClipNext ;
    if( numClipNext >= numClipArrays )
      {
       bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
       numClipNext = numClipArrays = 0 ;
      }
   }
/*
** Scan Dtm Object And Extract All Dtm Feature Occurrences For A Feature Id
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Next Feature") ;
    while ( dtmFeature < dtmP->numFeatures && *featureFoundP == FALSE )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError &&
          dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback &&
           dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted   &&
           dtmFeatureP->dtmUserTag      == userTag )
         {
          if( bcdtmObject_getPointsForDtmFeatureDtmObject(dtmP,dtmFeature,(DTM_TIN_POINT **) &featurePtsP ,&numFeaturePts)) goto errexit ;
          if( numFeaturePts > 0 )
            {
             fenceLoad  = TRUE ;
             clipResult = 0 ;
             if( useFence == TRUE )
               {
                fenceLoad  = FALSE ;
                if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,featurePtsP,numFeaturePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                if( clipResult == 1 || clipResult == 2 ) fenceLoad = TRUE ;
               }
/*
**           Set Scan Feature Variables
*/
             if( fenceLoad == TRUE )
               {
                scanFeatureP->dtmFeatureType   = (DTMFeatureType)dtmFeatureP->dtmFeatureType  ;
                scanFeatureP->dtmFeatureId     = dtmFeatureP->dtmFeatureId  ;
                scanFeatureP->dtmUserTag       = dtmFeatureP->dtmUserTag ;
                if( ! clipResult )
                  {
                   scanFeatureP->dtmFeaturePtsP   = featurePtsP ;
                   scanFeatureP->numDtmFeaturePts = numFeaturePts ;
                  }
                else
                  {
                   scanFeatureP->dtmFeaturePtsP   = clipArraysPP[0]->pointsP ;
                   scanFeatureP->numDtmFeaturePts = clipArraysPP[0]->numPoints ;
                   numClipNext  = 1 ;
                   clipFeatureP = dtmFeatureP ;
                   if( clipResult == 1 )
                     {
                      bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                      numClipNext = numClipArrays = 0 ;
                     }
                  }
                *featureFoundP = TRUE ;
                featurePtsP    = nullptr ;
               }
            }
         }
       ++dtmFeature ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( *featureFoundP == FALSE )
   {
    dtmFeature = 0 ;
    if( clipDtmP     != nullptr ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
    if( featurePtsP  != nullptr ) { free( featurePtsP) ; featurePtsP = nullptr ; }
    if( clipArraysPP != nullptr ) bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
    if( scanFeatureP->dtmFeaturePtsP != nullptr ) { free(scanFeatureP->dtmFeaturePtsP) ; scanFeatureP->dtmFeaturePtsP = nullptr ; }
   }
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *featureFoundP = FALSE ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Creates A Scan Context For A DTM Dtm Object
* @doc    Creates A Scan Context For A DTM Dtm Object
* @param dtmP                 ==> Dtm Object Pointer
* @param dtmFeatureType       ==> Dtm Feature Type
* @param maxSpots             ==> Maximum Number Of Spots To Be Returned In One Call
* @param useFence             ==> Use Fence <TRUE,FALSE>
* @param fenceType,           ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param fenceOption          ==> Fence Option <INSIDE(1),OVERLAP(2)
* @param fencePtsP            ==> Point Array Of Fence Points
* @param numFencePts          ==> Number Of Fence Points
* @param dtmScanContextPP     <== Pointer To Created Scan Context
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack April 2005 rob.cormack@bentley.com
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmScanContextLoad_createScanContextForDtmObject
(
 BC_DTM_OBJ *dtmP,                      /* ==> Dtm Object Pointer                                 */
 DTMFeatureType dtmFeatureType,                   /* ==> Dtm Feature Type                                   */
 long maxSpots,                         /* ==> Max Spots To Be Returned For Any One Call          */
 long useFence,                         /* ==> Use Fence <TRUE,FALSE>                             */
 DTMFenceType fenceType,                        /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>       */
 DTMFenceOption fenceOption,                      /* ==> Fence Option <INSIDE(1),OVERLAP(2)                 */
 DPoint3d  *fencePtsP,                       /* ==> Point Array Of Fence Points                        */
 long numFencePts,                      /* ==> Number Of Fence Points                             */
 BC_DTM_SCAN_CONTEXT **dtmScanContextPP /* <== Pointer To Created Scan Context                    */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTMFenceOption clipOption=DTMFenceOption::None ;
 BC_DTM_OBJ *clipTinP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Scan Context For Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType   = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"maxSpots         = %8ld",maxSpots) ;
    bcdtmWrite_message(0,0,0,"useFence         = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceOption      = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP        = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts      = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"dtmScanContextPP = %p",*dtmScanContextPP) ;
   }
/*
** Delete Current Scan Context If Necessary
*/
 if( *dtmScanContextPP != nullptr ) bcdtmScanContextLoad_deleteScanContext(dtmScanContextPP) ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Valid Dtm Feature Type
*/
 if( bcdtmData_testForValidDtmObjectExportFeatureType(dtmFeatureType))
   {
    bcdtmWrite_message(2,0,0,"Invalid DTM Feature Type") ;
    goto errexit ;
   }
/*
** Write DTM Statistics
*/
 if( dbg ) bcdtmUtility_writeStatisticsDtmObject(dtmP) ;
/*
** Check For Valid Fence Option
*/
 if (fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Overlap) useFence = FALSE;
 if( fencePtsP == nullptr || numFencePts < 3 ) useFence = FALSE ;
 if( useFence == TRUE ) clipOption = fenceOption ;
/*
** Build Clipping Tin
*/
 if( useFence == TRUE )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipTinP,fencePtsP,numFencePts)) goto errexit ;
   }
/*
** Check Max Spots Setting
*/
 if( maxSpots <= 0 ) maxSpots = 100000 ;
/*
** Create Scan Context
*/
 *dtmScanContextPP = ( BC_DTM_SCAN_CONTEXT * ) malloc (sizeof(BC_DTM_SCAN_CONTEXT)) ;
 if( *dtmScanContextPP == nullptr )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
     goto errexit ;
   }
/*
** Initialise Scan Context
*/
 (*dtmScanContextPP)->dtmObjectType   = BC_DTM_OBJ_TYPE  ;
 (*dtmScanContextPP)->dtmFeatureType  = (DTMFeatureType)dtmFeatureType ;
 (*dtmScanContextPP)->tiledDtmP       = nullptr ;
 (*dtmScanContextPP)->dtmP            = dtmP ;
 (*dtmScanContextPP)->latP            = nullptr ;
 (*dtmScanContextPP)->clipTinP        = clipTinP ;
 (*dtmScanContextPP)->clipOption      = clipOption ;
 (*dtmScanContextPP)->clipType        = fenceType ;
 (*dtmScanContextPP)->maxSpots        = maxSpots ;
 (*dtmScanContextPP)->scanOffset1     = 0 ;
 (*dtmScanContextPP)->scanOffset2     = 0 ;
 (*dtmScanContextPP)->scanOffset3     = 0 ;
 clipTinP = nullptr ;
/*
** Clean Up
*/
 cleanup :
 if( clipTinP != nullptr ) bcdtmObject_destroyDtmObject(&clipTinP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Scan Context For Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Scan Context For Dtm Object Completed") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *dtmScanContextPP != nullptr ) bcdtmScanContextLoad_deleteScanContext(dtmScanContextPP) ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Deletes A DTM Scan Context
* @doc    Deletes A DTM Scan Context
* @param  dtmScanContextPP     ==> Pointer To Dtm Scan Context To Be Deleted
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack September 2007 rob.cormack@bentley.com
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmScanContextLoad_deleteScanContext(BC_DTM_SCAN_CONTEXT **dtmScanContextPP )
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Destroying Scan Context %P",*dtmScanContextPP) ;
/*
** Only Process If Scan Context Exists
*/
 if( *dtmScanContextPP != nullptr )
   {
    if( (*dtmScanContextPP)->clipTinP != nullptr )bcdtmObject_destroyDtmObject(&(*dtmScanContextPP)->clipTinP) ;
    free( *dtmScanContextPP) ;
    *dtmScanContextPP = nullptr ;
   }
/*
** Return
*/
 return(ret) ;
}
/*==============================================================================*//**
* @memo   Scan For Dtm Feature Type Occurrence Dtm Object
* @doc    Scan For Dtm Feature Type Occurrence Dtm Object
* @param dtmScanContextP      ==> Scan Context Pointer
* @param featureFoundP        <== Feature Found <TRUE,FALSE>
* @param dtmFeatureTypeP      <== Dtm Feature Type
* @param userTagP             <== User Tag For Feature
* @param userFeatureIdP            <== User Feature Id For Feature
* @param featPtsPP            <== Pointer To Feature Points
* @param numFeatPtsP          <== Number Of Feature Points
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack September 2005 rob.cormack@bentley.com
* @version
* @see None
*===============================================================================*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmScanContextLoad_scanForDtmFeatureTypeOccurrenceDtmObject
(
 BC_DTM_SCAN_CONTEXT *dtmScanContextP, /* ==> Scan Context Pointer                    */
 long *featureFoundP,                  /* <== Feature Found <TRUE,FALSE>              */
 DTMFeatureType* dtmFeatureTypeP,                /* <== Dtm Feature Type                        */
 DTMUserTag *userTagP,               /* <== User Tag For Feature                    */
 DTMFeatureId *userFeatureIdP,       /* <== User Feature Id For Feature             */
 DTM_POINT_ARRAY ***pointArraysPPP,    /* <== Pointer To Point Arrays                 */
 long *numPointArraysP                 /* <== Number Of Point Array Pointers          */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,p4,sp1,sp2,sp3,cPtr,sPnt,nPnt,fndType,numSpots,maxSpots ;
 DTMFenceOption clipOption;
 long  loadFlag, dtmFeature, numFeatPts, numFenceArrays = 0;
 DTMFeatureType dtmFeatureType;
 long   voidLine,voidTriangle,clipResult=0 ;
 char   dtmFeatureTypeName[50] ;
 DPoint3d    *p3dP,*featPtsP=nullptr      ;
 BC_DTM_OBJ        *dtmP=nullptr,*clipTinP=nullptr ;
 DTM_POINT_ARRAY   **fenceArraysPP=nullptr  ;
 BC_DTM_FEATURE    *dtmFeatureP ;
 static long fPtr=-1 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Context Scanning For Dtm Feature Type Occurrence From Dtm Object") ;
/*
** Check Existence Of Scan Context
*/
 if( dtmScanContextP == nullptr )
   {
    bcdtmWrite_message(2,0,0,"Dtm Scan Context Not Initialised") ;
    goto errexit ;
   }
/*
** Check For Correct Dtm Object Type
*/
 if( dtmScanContextP->dtmObjectType != BC_DTM_OBJ_TYPE  )
   {
    bcdtmWrite_message(2,0,0,"Invalid Dtm Scan Object Type") ;
    goto errexit ;
   }
/*
** Initialise Return Arguments
*/
 *featureFoundP   = FALSE ;
 *dtmFeatureTypeP = dtmScanContextP->dtmFeatureType ;
 *userTagP        = DTM_NULL_USER_TAG ;
 *userFeatureIdP  = DTM_NULL_FEATURE_ID ;
/*
** Check Pointer To Point Arrays Is nullptr
*/
 if( *pointArraysPPP != nullptr )
   {
    bcdtmWrite_message(2,0,0,"Pointer To Point Arrays Not Initialised To nullptr") ;
    goto errexit ;
   }
/*
** Set Scan Variables
*/
 sp1  = dtmScanContextP->scanOffset1 ;
 sp2  = dtmScanContextP->scanOffset2 ;
 sp3  = dtmScanContextP->scanOffset3 ;
 dtmP = dtmScanContextP->dtmP ;
 clipTinP   = dtmScanContextP->clipTinP ;
 clipOption = dtmScanContextP->clipOption ;
 maxSpots   = dtmScanContextP->maxSpots ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"sp1 = %8ld sp2 = %8ld sp3 = %8ld ** dtmP = %p clipOption = %2ld",sp1,sp2,sp3,dtmP,clipOption) ;
    bcdtmWrite_message(0,0,0,"dtmP->dtmState = %2ld",dtmP->dtmState) ;
   }
/*
**  Switch For Feature Type
*/
 dtmFeatureType = dtmScanContextP->dtmFeatureType ;
 if( dbg )
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName);
    bcdtmWrite_message(0,0,0,"Scanning For DTM Feature Type %s",dtmFeatureTypeName) ;
   }
/*
** Check For Tin Hull
*/
 if( dtmP->dtmState == DTMState::Tin && dtmFeatureType == DTMFeatureType::Hull ) dtmFeatureType = DTMFeatureType::TinHull ;
/*
** Switch For Feature Type
*/
 switch( dtmFeatureType )
   {
    case  DTMFeatureType::Spots   :        // All Points
    case  DTMFeatureType::RandomSpots :   // Points Not On A Dtm Feature
/*
**  Count Points
*/
    numSpots = 0  ;
    for( p1 = sp1 ; p1 < dtmP->numPoints && numSpots < maxSpots ; ++p1 )
      {
       if( nodeAddrP(dtmP,p1)->cPtr != dtmP->nullPtr  )
         {
          if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD))
            {
             loadFlag = TRUE ;
             if( dtmFeatureType == DTMFeatureType::RandomSpots && nodeAddrP(dtmP,p1)->fPtr != dtmP->nullPtr ) loadFlag = FALSE ;
             if( loadFlag )
               {
                if( clipOption != DTMFenceOption::None)
                  {
                   if( bcdtmFind_triangleDtmObject(clipTinP,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,&fndType,&p2,&p3,&p4) ) goto errexit ;
                   if( ! fndType ) loadFlag = FALSE ;
                  }
               }
             if( loadFlag == TRUE ) ++numSpots ;
            }
         }
      }
/*
**  Allocate Memory For Points
*/
    if( numSpots >  0 )
      {
       numFeatPts = numSpots ;
       featPtsP = (DPoint3d *) malloc( numSpots * sizeof(DPoint3d)) ;
       if( featPtsP == nullptr )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
/*
**     Copy Points To Feature Points
*/
       numSpots = 0 ;
       p3dP = featPtsP ;
       for( p1 = sp1 ; p1 < dtmP->numPoints && numSpots < numFeatPts ; ++p1 )
         {
          if( nodeAddrP(dtmP,p1)->cPtr != dtmP->nullPtr  )
            {
             if( ! bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,p1)->PCWD))
               {
                loadFlag = TRUE ;
                if( dtmFeatureType == DTMFeatureType::RandomSpots && nodeAddrP(dtmP,p1)->fPtr != dtmP->nullPtr ) loadFlag = FALSE ;
                if( loadFlag == TRUE )
                  {
                   if( clipOption != DTMFenceOption::None)
                     {
                      if( bcdtmFind_triangleDtmObject(clipTinP,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,&fndType,&p2,&p3,&p4) ) goto errexit ;
                      if( ! fndType ) loadFlag = FALSE ;
                     }
                  }
                if( loadFlag == TRUE )
                  {
                   p3dP->x = pointAddrP(dtmP,p1)->x ;
                   p3dP->y = pointAddrP(dtmP,p1)->y ;
                   p3dP->z = pointAddrP(dtmP,p1)->z ;
                   ++p3dP ;
                   ++numSpots ;
                  }
               }
            }
         }
/*
**  Set Return Values
*/
       *featureFoundP = TRUE ;
       *dtmFeatureTypeP = dtmFeatureType ;
       if( bcdtmMem_storePointsInPointerArrayToPointArray(pointArraysPPP,&featPtsP,numFeatPts )) goto errexit ;
       *numPointArraysP = 1    ;
       featPtsP = nullptr ;
       dtmScanContextP->scanOffset1 = p1 ;
      }
    break ;

    case  DTMFeatureType::TriangleEdge   :   // Triangle Edges
/*
**  Scan To Next Triangle Edge In Circular List
*/
        if (dtmP->dtmState == DTMState::Tin)
            {
            cPtr = nodeAddrP (dtmP, sp1)->cPtr;
            if (sp1 != 0 || sp2 != 0)
                {
                while (cPtr != dtmP->nullPtr && clistAddrP (dtmP, cPtr)->pntNum != sp2) cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                }
            /*
            **  Check For nullptr Next Triangle Edge
            */
    if( cPtr == dtmP->nullPtr )
                {
                ++sp1;
                if (sp1 < dtmP->numPoints) cPtr = nodeAddrP (dtmP, sp1)->cPtr;
                else                        cPtr = dtmP->nullPtr;
                }
            /*
            **     Scan Tin Points For Triangle Edge
*/
    while ( sp1 < dtmP->numPoints  && *featureFoundP == FALSE )
                {
                while (cPtr != dtmP->nullPtr && *featureFoundP == FALSE)
                    {
                    sp2 = clistAddrP (dtmP, cPtr)->pntNum;
                    cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                    /*
                    **        Test For New Triangle Edge
                    */
                    if (sp2 > sp1)
                        {
                        /*
                        **           Test For Void Line
                        */
                        if (bcdtmList_testForVoidLineDtmObject (dtmP, sp1, sp2, &voidLine)) goto errexit;
                        if (!voidLine)
               {
                            /*
                            **              Allocate memory For Feature Points
                            */
                            numFeatPts = 2;
                featPtsP = ( DPoint3d *) malloc( numFeatPts * sizeof(DPoint3d)) ;
                if( featPtsP == nullptr )
                                {
                                bcdtmWrite_message (1, 0, 0, "Memory Allocation Failure");
                                goto errexit;
                                }
                            /*
                            **              Copy Feature Points
                            */
                (featPtsP)->x   = pointAddrP(dtmP,sp1)->x ;
                (featPtsP)->y   = pointAddrP(dtmP,sp1)->y ;
                (featPtsP)->z   = pointAddrP(dtmP,sp1)->z ;
                (featPtsP+1)->x = pointAddrP(dtmP,sp2)->x ;
                (featPtsP+1)->y = pointAddrP(dtmP,sp2)->y ;
                (featPtsP+1)->z = pointAddrP(dtmP,sp2)->z ;
                            /*
                            **              Process Fence
                            */
                if( clipOption != DTMFenceOption::None)
                                {
                                if (bcdtmClip_featurePointArrayToTinHullDtmObject (clipTinP, clipOption, featPtsP, numFeatPts, &clipResult, &fenceArraysPP, &numFenceArrays)) goto errexit;
                                /*
                                **                 Free Feature Points memory
*/
                                if (featPtsP != nullptr)
                                    {
                                    free (featPtsP);
                      featPtsP = nullptr ;
                     }
                                numFeatPts = 0;
                                /*
                                **                Process Clipped Feature Sections
*/
                                if (numFenceArrays > 0)
                                    {
                                    *featureFoundP = TRUE;
                      *dtmFeatureTypeP = DTMFeatureType::TriangleEdge ;
                                    *pointArraysPPP = fenceArraysPP;
                                    *numPointArraysP = numFenceArrays;
                                    fenceArraysPP = nullptr;
                                    dtmScanContextP->scanOffset1 = sp1;
                                    dtmScanContextP->scanOffset2 = sp2;
                     }
                                }
                            /*
                            **              Store Feature Points In Point Array
                            */
                            else
                                {
                                *featureFoundP = TRUE;
                   *dtmFeatureTypeP = DTMFeatureType::TriangleEdge ;
                                if (bcdtmMem_storePointsInPointerArrayToPointArray (pointArraysPPP, &featPtsP, numFeatPts)) goto errexit;
                                *numPointArraysP = 1;
                                featPtsP = nullptr;
                                dtmScanContextP->scanOffset1 = sp1;
                                dtmScanContextP->scanOffset2 = sp2;
                  }
                            }
                        }
                    }
                /*
                **     Get Next Tin Point
*/
                if (*featureFoundP == FALSE)
                    {
                    ++sp1;
                    if (sp1 < dtmP->numPoints) cPtr = nodeAddrP (dtmP, sp1)->cPtr;
          else                     cPtr = dtmP->nullPtr ;
                    }
                }
            }
    break ;

    case  DTMFeatureType::Triangle   :   // Triangles
        if (dtmP->dtmState == DTMState::Tin)
            {
            /*
**  Scan To Next Triangle In Circular List
*/
            cPtr = nodeAddrP (dtmP, sp1)->cPtr;
            if (sp1 != 0 || sp2 != 0 || sp3 != 0)
                {
                while (cPtr != dtmP->nullPtr && clistAddrP (dtmP, cPtr)->pntNum != sp3) cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                }
            /*
            **  Check For nullptr Next Triangle Edge
            */
    if( cPtr == dtmP->nullPtr )
                {
                ++sp1;
                if (sp1 < dtmP->numPoints) cPtr = nodeAddrP (dtmP, sp1)->cPtr;
                else                     cPtr = dtmP->nullPtr;
                if (cPtr != dtmP->nullPtr)
                    {
                    if ((sp3 = bcdtmList_nextAntDtmObject (dtmP, sp1, clistAddrP (dtmP, cPtr)->pntNum)) < 0) goto errexit;
                    }
                }
            /*
            **  Scan Tin Points For Triangles
*/
    while ( sp1 < dtmP->numPoints  && *featureFoundP == FALSE )
                {
                while (cPtr != dtmP->nullPtr && *featureFoundP == FALSE)
                    {
                    sp2 = sp3;
                    sp3 = clistAddrP (dtmP, cPtr)->pntNum;
                    cPtr = clistAddrP (dtmP, cPtr)->nextPtr;
                    /*
**        Test For New Triangle
                    */
                    if (sp2 > sp1 && sp3 > sp1 && nodeAddrP (dtmP, sp3)->hPtr != sp1)
                        {
                        /*
                        **           Test For Void Triangle
*/
                        if (bcdtmList_testForVoidTriangleDtmObject (dtmP, sp1, sp2, sp3, &voidTriangle)) goto errexit;
                        if (!voidTriangle)
                            {
                            /*
                            **              Allocate memory For Triangle Points
                            */
                            numFeatPts = 4;
                featPtsP = ( DPoint3d *) malloc( numFeatPts * sizeof(DPoint3d)) ;
                if( featPtsP == nullptr )
                                {
                                bcdtmWrite_message (1, 0, 0, "Memory Allocation Failure");
                                goto errexit;
                                }
                            /*
                            **              Copy Feature Points
                            */
                (featPtsP)->x   = pointAddrP(dtmP,sp1)->x ;
                (featPtsP)->y   = pointAddrP(dtmP,sp1)->y ;
                (featPtsP)->z   = pointAddrP(dtmP,sp1)->z ;
                (featPtsP+1)->x = pointAddrP(dtmP,sp2)->x ;
                (featPtsP+1)->y = pointAddrP(dtmP,sp2)->y ;
                (featPtsP+1)->z = pointAddrP(dtmP,sp2)->z ;
                (featPtsP+2)->x = pointAddrP(dtmP,sp3)->x ;
                (featPtsP+2)->y = pointAddrP(dtmP,sp3)->y ;
                (featPtsP+2)->z = pointAddrP(dtmP,sp3)->z ;
                (featPtsP+3)->x = pointAddrP(dtmP,sp1)->x ;
                (featPtsP+3)->y = pointAddrP(dtmP,sp1)->y ;
                (featPtsP+3)->z = pointAddrP(dtmP,sp1)->z ;
                            /*
                            **              Process Fence
                            */
                if( clipOption != DTMFenceOption::None )
                                {
                                if (bcdtmClip_featurePointArrayToTinHullDtmObject (clipTinP, clipOption, featPtsP, numFeatPts, &clipResult, &fenceArraysPP, &numFenceArrays)) goto errexit;
                                /*
                                **                 Free Feature Points memory
                                */
                                if (featPtsP != nullptr)
                                    {
                                    free (featPtsP);
                      featPtsP = nullptr ;
                     }
                                numFeatPts = 0;
                                /*
                                **                 Process Clipped Feature Sections
*/
                                if (numFenceArrays > 0)
                                    {
                                    *featureFoundP = TRUE;
                      *dtmFeatureTypeP = DTMFeatureType::Triangle ;
                                    *pointArraysPPP = fenceArraysPP;
                                    *numPointArraysP = numFenceArrays;
                                    fenceArraysPP = nullptr;
                                    dtmScanContextP->scanOffset1 = sp1;
                                    dtmScanContextP->scanOffset2 = sp2;
                                    dtmScanContextP->scanOffset3 = sp3;
                     }
                                }
                            /*
                            **              Store Feature Points In Point Array
                            */
                            else
                                {
                                *featureFoundP = TRUE;
                   *dtmFeatureTypeP = DTMFeatureType::Triangle ;
                                if (bcdtmMem_storePointsInPointerArrayToPointArray (pointArraysPPP, &featPtsP, numFeatPts)) goto errexit;
                                *numPointArraysP = 1;
                                featPtsP = nullptr;
                                dtmScanContextP->scanOffset1 = sp1;
                                dtmScanContextP->scanOffset2 = sp2;
                                dtmScanContextP->scanOffset3 = sp3;
                  }
                            }
                        }
                    }
                /*
                **     Get Next Tin Point
*/
                if (*featureFoundP == FALSE)
                    {
                    ++sp1;
                    if (sp1 < dtmP->numPoints) cPtr = nodeAddrP (dtmP, sp1)->cPtr;
          else                     cPtr = dtmP->nullPtr ;
                    if (cPtr != dtmP->nullPtr)
                        {
                        if ((sp3 = bcdtmList_nextAntDtmObject (dtmP, sp1, clistAddrP (dtmP, cPtr)->pntNum)) < 0) goto errexit;
                        }
                    }
                }
            }
    break ;


    case  DTMFeatureType::TinHull :     // Tin Hull
/*
**   Set User Tags And Feature Id's
*/
     *userTagP = dtmP->nullUserTag ;
     *userFeatureIdP = dtmP->nullFeatureId ;
     bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(dtmP,DTMFeatureType::Hull,1,&dtmFeatureP,&dtmFeature) ;
     if( dtmFeatureP != nullptr )
       {
        *userTagP = dtmFeatureP->dtmUserTag ;
        *userFeatureIdP = dtmFeatureP->dtmFeatureId ;
       }
/*
**  Count Number Of Hull Points
*/
    if( dtmScanContextP->scanOffset1 == 0 )
      {
       numSpots = 0 ;
       sp1 = dtmP->hullPoint ;
       do
         {
          ++numSpots ;
          sp1 = nodeAddrP(dtmP,sp1)->hPtr ;
         } while ( sp1 != dtmP->hullPoint ) ;
/*
**     Allocate memory For Hull Points
*/
       numFeatPts = numSpots + 1  ;
       featPtsP = ( DPoint3d *) malloc( numFeatPts * sizeof(DPoint3d)) ;
       if( featPtsP == nullptr )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
/*
**     Copy Hull Points
*/
       p3dP = featPtsP ;
       sp1  = dtmP->hullPoint ;
       do
         {
          p3dP->x = pointAddrP(dtmP,sp1)->x ;
          p3dP->y = pointAddrP(dtmP,sp1)->y ;
          p3dP->z = pointAddrP(dtmP,sp1)->z ;
          ++p3dP ;
          sp1 = nodeAddrP(dtmP,sp1)->hPtr  ;
         } while ( sp1 != dtmP->hullPoint ) ;
       p3dP->x = pointAddrP(dtmP,sp1)->x ;
       p3dP->y = pointAddrP(dtmP,sp1)->y ;
       p3dP->z = pointAddrP(dtmP,sp1)->z ;
       ++p3dP ;
/*
**     Process Fence
*/
       if (clipOption != DTMFenceOption::None)
         {
          if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipTinP,clipOption,featPtsP,numFeatPts,&clipResult,&fenceArraysPP,&numFenceArrays) ) goto errexit ;
/*
**        Free Feature Points memory
*/
          if( featPtsP != nullptr )
            {
             free(featPtsP) ;
             featPtsP = nullptr ;
            }
          numFeatPts = 0 ;
/*
**        Process Clipped Feature Sections
*/
          if( numFenceArrays > 0 )
            {
             *featureFoundP = TRUE ;
             *dtmFeatureTypeP = DTMFeatureType::Hull ;
             *pointArraysPPP  = fenceArraysPP ;
             *numPointArraysP = numFenceArrays ;
             fenceArraysPP    = nullptr ;
             dtmScanContextP->scanOffset1 = 1 ;
            }
         }
/*
**    Store Feature Points In Point Array
*/
       else
         {
          *featureFoundP = TRUE ;
          *dtmFeatureTypeP = DTMFeatureType::Hull ;
          if( bcdtmMem_storePointsInPointerArrayToPointArray(pointArraysPPP,&featPtsP,numFeatPts )) goto errexit ;
          *numPointArraysP = 1    ;
          featPtsP= nullptr ;
          dtmScanContextP->scanOffset1 = 1 ;
         }
      }
    break ;


    case DTMFeatureType::GroupSpots    :
    case DTMFeatureType::Breakline    :
    case DTMFeatureType::ContourLine  :
    case DTMFeatureType::Void          :
    case DTMFeatureType::Island        :
    case DTMFeatureType::Hole          :
    case DTMFeatureType::Polygon       :
    case DTMFeatureType::Region        :
    case DTMFeatureType::Hull          :
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For DTM Feature Type = %4ld",dtmFeatureType) ;
/*
**     Scan To Next Feature Occurrence
*/
    for( dtmFeature = sp1 ; dtmFeature < dtmP->numFeatures && *featureFoundP == FALSE ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
         {
          if( dtmFeatureP->dtmFeatureType == dtmFeatureType )
            {
/*
**           Count Number Of Points In Feature
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Counting Number Of Feature Points") ;
             numSpots = 0 ;
             if( dtmP->dtmState == DTMState::Data ) numSpots = dtmFeatureP->numDtmFeaturePts ;
             else
               {
                sPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
                do
                  {
                   ++numSpots ;
                   if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
                   sPnt = nPnt ;
                  } while ( sPnt != dtmFeatureP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
                if( sPnt ==  dtmFeatureP->dtmFeaturePts.firstPoint ) ++numSpots ;
               }
/*
**           Allocate Memory For Feature Points
*/
             numFeatPts = numSpots  ;
             featPtsP = ( DPoint3d *) malloc( numFeatPts * sizeof(DPoint3d)) ;
             if( featPtsP == nullptr )
               {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               }
/*
**           Copy Feature Points
*/

             p3dP = featPtsP ;
             if( dtmP->dtmState == DTMState::Data )
               {
                for( sPnt = dtmFeatureP->dtmFeaturePts.firstPoint ; sPnt < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++sPnt )
                  {
                   p3dP->x = pointAddrP(dtmP,sPnt)->x ;
                   p3dP->y = pointAddrP(dtmP,sPnt)->y ;
                   p3dP->z = pointAddrP(dtmP,sPnt)->z ;
                   ++p3dP ;
                  }
               }
             else
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature Points") ;
                sPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
                do
                  {
                   p3dP->x = pointAddrP(dtmP,sPnt)->x ;
                   p3dP->y = pointAddrP(dtmP,sPnt)->y ;
                   p3dP->z = pointAddrP(dtmP,sPnt)->z ;
                   ++p3dP ;
                   if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
                   sPnt = nPnt ;
                  } while ( sPnt != dtmFeatureP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
                if( sPnt ==  dtmFeatureP->dtmFeaturePts.firstPoint )
                  {
                   p3dP->x = pointAddrP(dtmP,sPnt)->x ;
                   p3dP->y = pointAddrP(dtmP,sPnt)->y ;
                   p3dP->z = pointAddrP(dtmP,sPnt)->z ;
                   ++p3dP ;
                  }
               }
/*
**           Process Fence
*/
             if (clipOption != DTMFenceOption::None)
               {
/*
**              Group Spots
*/
                if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Feature") ;
                if( dtmFeatureType == DTMFeatureType::GroupSpots )
                  {
                   p1 = 0 ;
                   for( p3dP = featPtsP ; p3dP <= featPtsP + numFeatPts ;  ++p3dP )
                     {
                      if( bcdtmFind_triangleDtmObject(clipTinP,p3dP->x,p3dP->y,&fndType,&p2,&p3,&p4) ) goto errexit ;
                        {
                         if( (long)(p3dP-featPtsP) != p1 )
                           {
                            (featPtsP+p1)->x = p3dP->x ;
                            (featPtsP+p1)->y = p3dP->y ;
                            (featPtsP+p1)->z = p3dP->z ;
                           }
                         ++p1 ;
                        }
                     }
                   numFeatPts = p1 ;
                   if( numFeatPts > 0 )
                     {
                      if( bcdtmMem_storePointsInPointerArrayToPointArray(&fenceArraysPP,&featPtsP,numFeatPts )) goto errexit ;
                      numFenceArrays = 1 ;
                     }
                  }
/*
**              Other Features
*/
                else
                  {
                   if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipTinP,clipOption,featPtsP,numFeatPts,&clipResult,&fenceArraysPP,&numFenceArrays) ) goto errexit ;
                  }
/*
**              Free Feature Points memory
*/
                if( featPtsP != nullptr )
                  {
                   free(featPtsP) ;
                   featPtsP = nullptr ;
                  }
                numFeatPts = 0 ;
/*
**              Process Clipped Feature Sections
*/
                if( numFenceArrays > 0 )
                  {
                   *featureFoundP = TRUE ;
                   *dtmFeatureTypeP = dtmFeatureP->dtmFeatureType ;
                   *pointArraysPPP  = fenceArraysPP ;
                   *numPointArraysP = numFenceArrays ;
                   fenceArraysPP    = nullptr ;
                   *userFeatureIdP = dtmFeatureP->dtmFeatureId ;
                   *userTagP  = dtmFeatureP->dtmUserTag  ;
                   dtmScanContextP->scanOffset1 = dtmFeature + 1    ;
                  }
               }
/*
**           Store Feature Points In Point Array
*/
             else
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Storing Feature Points") ;
                *featureFoundP = TRUE ;
                if( bcdtmMem_storePointsInPointerArrayToPointArray(pointArraysPPP,&featPtsP,numFeatPts )) goto errexit ;
                *numPointArraysP = 1    ;
                featPtsP= nullptr ;
                *dtmFeatureTypeP = dtmFeatureP->dtmFeatureType ;
                *userFeatureIdP = dtmFeatureP->dtmFeatureId   ;
                *userTagP  = dtmFeatureP->dtmUserTag ;
                dtmScanContextP->scanOffset1 = dtmFeature + 1    ;
               }
            }
         }
      }
    break ;


     default :
    break ;
   }
/*
** Write If Feature Found
*/
 if( dbg )
   {
    if( *featureFoundP == TRUE ) bcdtmWrite_message(0,0,0,"DTM Feature Found") ;
    else                         bcdtmWrite_message(0,0,0,"DTM Feature Not Found") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( featPtsP != nullptr ) {  free(featPtsP) ; featPtsP = nullptr ;  }
 if( *featureFoundP == FALSE )
   {
    fPtr = -1 ;
    if( dtmScanContextP != nullptr )
      {
       dtmScanContextP->scanOffset1 = 0 ;
       dtmScanContextP->scanOffset2 = 0 ;
       dtmScanContextP->scanOffset3 = 0 ;
      }
   }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Scanning For Dtm Feature Type Occurrence From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Scanning For Dtm Feature Type Occurrence From Dtm Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *featureFoundP = FALSE ;
 *userTagP      = DTM_NULL_USER_TAG ;
 *userFeatureIdP     = DTM_NULL_FEATURE_ID    ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_slopeLinesBetweenDtmObjects
(
 BC_DTM_OBJ *dtm1P,            /* ==> Pointer To Dtm Object 1           */
 BC_DTM_OBJ *dtm2P,            /* ==> Pointer To Dtm Object 2           */
 DTMFeatureCallback loadFunctionP,       /* ==> Pointer To Load Function          */
 double majorInterval,         /* ==> Major Slope Line Interval         */
 double minorInterval,         /* ==> Minor Slope Line Interval         */
 void *userP                   /* ==> User Pointer Passed Back          */
)
/*
** This Function Loads Slope Indicators Between Two DTM's In Tin State
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    pp,hp,np,numInterval=0,majorSlopeInterval=0 ;
 double  dd,di,dh,dl,dx,dy,dz,radialLength ;
 double  ang,ang1,ang2,Hx,Hy,Hz,Rx,Ry,Rz ;
 DPoint3d     slopeLinePts[2] ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Loading Slope Lines Between Dtm Objects") ;
    bcdtmWrite_message(0,0,0,"dtm1P   = %p",dtm1P) ;
    bcdtmWrite_message(0,0,0,"dtm2P   = %p",dtm2P) ;
    bcdtmWrite_message(0,0,0,"majorInterval= %10.4lf",majorInterval) ;
    bcdtmWrite_message(0,0,0,"minorInterval= %10.4lf",minorInterval) ;
    if( dbg == 2 )
      {
       bcdtmWrite_toFileDtmObject(dtm1P,L"slope1.dtm") ;
       bcdtmWrite_toFileDtmObject(dtm2P,L"slope2.dtm") ;
      }
   }
/*
** Check For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(dtm2P)) goto errexit  ;
/*
** Check Both DTM's Are In Tin State
*/
 if( dtm1P->dtmState != DTMState::Tin || dtm2P->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Dtm Object Not In Tin State") ;
    goto errexit ;
   }
/*
** Check For Valid Intervals
*/
 if( majorInterval <= 0.0 ) { bcdtmWrite_message(1,0,0,"Invalid Major Interval Value") ; goto errexit  ; }
 if( minorInterval <= 0.0 ||  minorInterval > majorInterval ) minorInterval = majorInterval ;
/*
** Determine Number Of Minor Intervals per Major Interval
*/
 majorSlopeInterval = (long)(majorInterval/minorInterval) ;
/*
** Determine An Appropriate Radial Length For The Calculations
*/
 dx = dtm2P->xMax - dtm2P->xMin ;
 dy = dtm2P->yMax - dtm2P->yMin ;
 radialLength = sqrt(dx*dx+dy*dy) ;
/*
** Scan Tin Hull And Determine Points At The Minor Interval
*/
 di = dh = 0.0 ;
 hp = dtm1P->hullPoint ;
 if( ( pp = bcdtmList_nextClkDtmObject(dtm1P,hp,nodeAddrP(dtm1P,hp)->hPtr)) < 0 ) goto errexit  ;
/*
** Scan dtm1P Hull And Write Slope Lines
*/
 do
   {
    np = nodeAddrP(dtm1P,hp)->hPtr ;
    dd = bcdtmMath_pointDistanceDtmObject(dtm1P,hp,np) ;
    while ( di + minorInterval <= (dh+dd) || numInterval == 0 )
      {
       if( numInterval ) di = di + minorInterval ;
/*
**     Check If Interval Point Is Coincident With Tin Hull Point
*/
       if     ( fabs(di-dh) <= dtm1P->ppTol )
         {
          ang1 = bcdtmMath_getPointAngleDtmObject(dtm1P,hp,np) ;
          ang2 = bcdtmMath_getPointAngleDtmObject(dtm1P,hp,pp) ;
          if( ang1 < ang2 ) ang1 = ang1 + DTM_2PYE ;
          ang = ( ang1 + ang2 ) / 2.0 ;
          if( ang >= DTM_2PYE ) ang = ang - DTM_2PYE ;
          Hx = pointAddrP(dtm1P,hp)->x ;
          Hy = pointAddrP(dtm1P,hp)->y ;
          Hz = pointAddrP(dtm1P,hp)->z ;
         }
       else if( fabs(di-(dh+dd)) <= dtm1P->ppTol )
         {
          ang1 = bcdtmMath_getPointAngleDtmObject(dtm1P,np,nodeAddrP(dtm1P,np)->hPtr) ;
          ang2 = bcdtmMath_getPointAngleDtmObject(dtm1P,np,hp) ;
          if( ang1 < ang2 ) ang1 = ang1 + DTM_2PYE ;
          ang = ( ang1 + ang2 ) / 2.0 ;
          if( ang >= DTM_2PYE ) ang = ang - DTM_2PYE ;
          Hx = pointAddrP(dtm1P,np)->x ;
          Hy = pointAddrP(dtm1P,np)->y ;
          Hz = pointAddrP(dtm1P,np)->z ;
         }
       else
         {
          ang = bcdtmMath_getPointAngleDtmObject(dtm1P,hp,np) - DTM_PYE/2.0 ;
          if( ang < 0.0  ) ang += DTM_2PYE ;
          dl = di - dh ;
          dx = pointAddrP(dtm1P,np)->x - pointAddrP(dtm1P,hp)->x ;
          dy = pointAddrP(dtm1P,np)->y - pointAddrP(dtm1P,hp)->y ;
          dz = pointAddrP(dtm1P,np)->z - pointAddrP(dtm1P,hp)->z ;
          Hx = pointAddrP(dtm1P,hp)->x +  dx * dl / dd ;
          Hy = pointAddrP(dtm1P,hp)->y +  dy * dl / dd ;
          Hz = pointAddrP(dtm1P,hp)->z +  dz * dl / dd ;
         }
/*
**     Calculate Radial For Intersecting With Tin Hull
*/
       Rx = Hx + radialLength * cos(ang) ;
       Ry = Hy + radialLength * sin(ang) ;
/*
**     Calculate Intersection Of Radial With dtm2P Hull
*/
       bcdtmLoad_truncateRadialAtTinHullDtmObject(dtm2P,Hx,Hy,Rx,Ry,&Rx,&Ry,&Rz) ;
/*
**     Load Slope line
*/
       slopeLinePts[0].x = Hx ; slopeLinePts[0].y = Hy ; slopeLinePts[0].z = Hz ;
       slopeLinePts[1].x = Rx ; slopeLinePts[1].y = Ry ; slopeLinePts[1].z = Rz ;
       if( numInterval % majorSlopeInterval == 0 )
         {
          if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::SlopeLine,0,DTM_NULL_FEATURE_ID,slopeLinePts,2,userP)) goto errexit ;
         }
       else
         {
          if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::SlopeLine,1,DTM_NULL_FEATURE_ID,slopeLinePts,2,userP)) goto errexit ;
         }
       ++numInterval ;
      }
/*
** Set Variables For Next Hull Point
*/
    dh = dh + dd ;
    pp = hp ; hp = np ;
   } while ( hp != dtm1P->hullPoint) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Slope Lines Between Tin Objects Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Slope Lines Between Tin Objects Error") ;
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
BENTLEYDTM_Public int bcdtmLoad_truncateRadialAtTinHullDtmObject(BC_DTM_OBJ *dtmP,double Sx,double Sy,double Ex,double Ey,double *Hx,double *Hy,double *Hz)
/*
** This Function Truncates A Radial At Tin Hull
*/
{
 long    hp,np ;
 int     sd1,sd2  ;
 double  d1,d2,dz,X1,Y1,Z1,X2,Y2,Z2 ;
 double  Rxmin,Rxmax,Rymin,Rymax,Hxmin,Hxmax,Hymin,Hymax ;
/*
** Initialise
*/
 *Hx = Ex ; *Hy = Ey ; *Hz = 0.0 ;
 if( Sx <= *Hx ) { Rxmin =  Sx ; Rxmax = *Hx ; }
 else            { Rxmin = *Hx ; Rxmax =  Sx ; }
 if( Sy <= *Hy ) { Rymin =  Sy ; Rymax = *Hy ; }
 else            { Rymin = *Hy ; Rymax =  Sy ; }
/*
** Scan Tin Hull And Determine Closest Intersection With To Radial Start Point
*/
 hp = dtmP->hullPoint ;
 X1 = pointAddrP(dtmP,hp)->x ;
 Y1 = pointAddrP(dtmP,hp)->y ;
 Z1 = pointAddrP(dtmP,hp)->z ;
 do
   {
/*
** Get Next Point On Hull
*/
    np = nodeAddrP(dtmP,hp)->hPtr ;
    X2 = pointAddrP(dtmP,np)->x ;
    Y2 = pointAddrP(dtmP,np)->y ;
    Z2 = pointAddrP(dtmP,np)->z ;
/*
** Determine Max & Min Values For Hull Line
*/
    if( X1 <= X2 ) { Hxmin = X1 ; Hxmax = X2 ; }
    else           { Hxmin = X2 ; Hxmax = X1 ; }
    if( Y1 <= Y2 ) { Hymin = Y1 ; Hymax = Y2 ; }
    else           { Hymin = Y2 ; Hymax = Y1 ; }
/*
** Test For Intersection
*/
    if( Rxmin - 0.0001 < Hxmax && Rymin - 0.0001 < Hymax &&
        Rxmax + 0.0001 > Hxmin && Rymax + 0.0001 > Hymin     )
      {
       sd1 = bcdtmMath_sideOf(Sx,Sy,*Hx,*Hy,X1,Y1) ;
       sd2 = bcdtmMath_sideOf(Sx,Sy,*Hx,*Hy,X2,Y2) ;
       if( sd1 != sd2 )
         {
          sd1 = bcdtmMath_sideOf(X1,Y1,X2,Y2,Sx,Sy)   ;
          sd2 = bcdtmMath_sideOf(X1,Y1,X2,Y2,*Hx,*Hy) ;
          if( sd1 != sd2 )
            {
             bcdtmMath_normalIntersectCordLines(Sx,Sy,*Hx,*Hy,X1,Y1,X2,Y2,Hx,Hy) ;
             dz = Z2 - Z1 ;
             d1 = bcdtmMath_distance(X1,Y1,*Hx,*Hy) ;
             d2 = bcdtmMath_distance(X1,Y1,X2,Y2) ;
             *Hz = Z1 + dz * d1 / d2 ;
             if( Sx <= *Hx ) { Rxmin =  Sx ; Rxmax = *Hx ; }
             else            { Rxmin = *Hx ; Rxmax =  Sx ; }
             if( Sy <= *Hy ) { Rymin =  Sy ; Rymax = *Hy ; }
             else            { Rymin = *Hy ; Rymax =  Sy ; }
            }
         }
      }
/*
** Reset Variables For Next Loop
*/
    hp = np ;
    X1 = X2 ;
    Y1 = Y2 ;
    Z1 = Z2 ;
   } while ( hp != dtmP->hullPoint ) ;
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
}
/*==============================================================================*//**
* @memo   Loads DTM Feature Occurrences For User Tag From A DTM Object
* @doc    Loads DTM Feature Occurrences For User Tag From A DTM Object
* @author Rob Cormack June 2008  rob.cormack@bentley.con
* @param  dtmP                  ==> Pointer To DTM object
* @param  userTag               ==> User Tag For DTM Feature To Be Loaded
* @param  maxSpots              ==> Maximum Number Of Points To Load Per Call For Random and Group Spots
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_dtmFeaturesForUsertagDtmObject
(
 BC_DTM_OBJ  *dtmP,           /* ==> Pointer To DTM Dtm object                    */
 DTMUserTag userTag,        /* ==> Type Of DTM Feature To Be Loaded             */
 long    maxSpots,            /* ==> Maximum Number Of Spots Points To Load Per Call       */
 DTMFeatureCallback loadFunctionP,   /* ==> Pointer To Load Function                     */
 long    useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape> */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>          */
 DPoint3d     *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                    */
 long    numFencePts,         /* ==> Number Of Fence Points                       */
 void    *userP               /* ==> User Pointer Passed Back To User             */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DPoint3d *p3dP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interrupt Loading Dtm Feature For Usertag") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"userTag           = %8ld",userTag) ;
    bcdtmWrite_message(0,0,0,"maxSpots          = %8ld",maxSpots) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
    if( fencePtsP != nullptr && numFencePts > 0 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          p3dP->z = 0.0 ;
          bcdtmWrite_message(0,0,0,"fencePts[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == nullptr || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Validate Max Spots
*/
 if( maxSpots < 0 ) maxSpots = 1 ;
 if( maxSpots > 50000 ) maxSpots = 50000 ;
 if( maxSpots > dtmP->numPoints ) maxSpots = dtmP->numPoints ;
/*
** Load DTM Features For User Tag
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading DTM Feature Occurrences For User Tag") ;
 if( bcdtmInterruptLoad_dtmFeatureUsertagOccurrencesDtmObject(dtmP,userTag,maxSpots,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Dtm Feature For Usertag Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Dtm Feature For Usertag Error") ;
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
BENTLEYDTM_Private int bcdtmInterruptLoad_dtmFeatureUsertagOccurrencesDtmObject
(
 BC_DTM_OBJ   *dtmP,
 DTMUserTag userTag ,
 long maxSpots,
 DTMFeatureCallback loadFunctionP,
 long useFence,
 DTMFenceType fenceType,
 DTMFenceOption fenceOption,
 DPoint3d  *fencePtsP,
 long numFencePts,
 void *userP
)
/*
** This Function Loads All DTM Feature Occurrences For A User Tag
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long            pnt,pnt1,pnt2,pnt3,nextPnt,numPts,closeFlag,dtmFeature ;
 long            n,numSpots,numFeatureSpots,numFeaturePts,numClipArrays,clipResult;
 long            fndType,insideFence,fenceLoad ;
 DPoint3d             *p3dP,*featurePtsP=nullptr ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 BC_DTM_OBJ      *clipDtmP=nullptr  ;
 DTM_TIN_POINT   *pntP  ;
 DTM_POINT_ARRAY **clipArraysPP=nullptr ;
/*
** Write Entry Message
*/
// bcdtmWrite_message(0,0,0,"Interrupt Loading DTM Features For Usertag = %10I64d",userTag) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interrupt Loading DTM Features For Usertag") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"userTag           = %8I64d",userTag) ;
    bcdtmWrite_message(0,0,0,"maxSpots          = %8ld",maxSpots) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
    if( fencePtsP != nullptr && numFencePts > 0 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          p3dP->z = 0.0 ;
          bcdtmWrite_message(0,0,0,"fencePts[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Build Clipping Dtm For Fence Operations
*/
 if( useFence == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Clipping Tin") ;
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Clipping Tin Completed") ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
   }
/*
**  Scan Dtm Features And Return All Features With The UserTag Value
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmUserTag == userTag && ( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin ))
      {
/*
**     DTM Group Spot Feature - Accumulate Group Spot Points In Batches Of maxSpots
*/
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots )
         {
          if( bcdtmList_countNumberOfDtmFeaturePointsDtmObject(dtmP,dtmFeature,&numFeatureSpots,&closeFlag)) goto errexit ;
          numSpots = numFeatureSpots ;
          if( numSpots > maxSpots ) numSpots = maxSpots ;
          if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
          featurePtsP = ( DPoint3d * ) malloc ( numSpots * sizeof(DPoint3d)) ;
          if( featurePtsP == nullptr )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
/*
**       Scan Group Spot Points And Accumulate In Points Array
*/
         numSpots = 0 ;
         pnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
         for( numPts = 0 ; numPts < numFeatureSpots ; ++numPts )
           {
            pntP = pointAddrP(dtmP,pnt) ;
            if( useFence == FALSE )
              {
               (featurePtsP+numSpots)->x = pntP->x ;
               (featurePtsP+numSpots)->y = pntP->y ;
               (featurePtsP+numSpots)->z = pntP->z ;
               ++numSpots ;
              }
            else
              {
               insideFence = FALSE ;
               if( fenceType == DTMFenceType::Block && pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax ) insideFence = TRUE ; ;
               if( fenceType == DTMFenceType::Shape && pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
                 {
                  if( bcdtmFind_triangleDtmObject(dtmP,pntP->x,pntP->y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
                  if( fndType ) insideFence = TRUE ;
                 }
               fenceLoad = FALSE ;
               if( ( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )  && insideFence == TRUE  ) fenceLoad = TRUE ;
               if(   fenceOption == DTMFenceOption::Outside && insideFence == FALSE ) fenceLoad = TRUE ;
               if( fenceLoad == TRUE )
                 {
                  (featurePtsP+numSpots)->x = pntP->x ;
                  (featurePtsP+numSpots)->y = pntP->y ;
                  (featurePtsP+numSpots)->z = pntP->z ;
                  ++numSpots ;
                 }
              }
/*
**          Check For Max Spots
*/
            if( numSpots == maxSpots )
              {
               if( loadFunctionP((DTMFeatureType)dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,maxSpots,userP)) goto errexit ;
               numSpots = 0 ;
              }
/*
**          Get Next Group Spot Point
*/
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data ) ++pnt ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin  )
              {
               if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,pnt,&nextPnt)) goto errexit ;
               pnt = nextPnt ;
              }
           }
/*
**       Check For Remaining Spots
*/
          if( numSpots > 0 )
            {
             if( loadFunctionP( (DTMFeatureType)dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numSpots,userP)) goto errexit ;
            }
         }
/*
**     None Group Spot Feature
*/
       else
         {
          if( bcdtmObject_getPointsForDtmFeatureDtmObject(dtmP,dtmFeature,(DTM_TIN_POINT **) &featurePtsP ,&numFeaturePts)) goto errexit ;
          if( useFence == FALSE )
            {
             if( loadFunctionP((DTMFeatureType)dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
            }
          else
            {
             if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,featurePtsP,numFeaturePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
             if( clipResult == 1 ) if( loadFunctionP((DTMFeatureType)dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
             if( clipResult == 2 )
               {
                for( n = 0 ; n < numClipArrays ; ++n )
                  {
                   if( loadFunctionP((DTMFeatureType)dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                  }
                bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
               }
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( clipDtmP    != nullptr ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 if( featurePtsP != nullptr ) { free( featurePtsP) ; featurePtsP = nullptr ; }
 if( clipArraysPP != nullptr ) bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Interrupt Loading DTM Features For Usertag Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Interrupt Loading DTM Features For Usertag Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Loads DTM Feature Occurrences For A Feature Id From A DTM Object
* @doc    Loads DTM Feature Occurrences For A Feature Id From A DTM Object
* @author Rob Cormack June 2008  rob.cormack@bentley.con
* @param  dtmP                  ==> Pointer To DTM object
* @param  featureId             ==> Feature Id For DTM Features To Be Loaded
* @param  maxSpots              ==> Maximum Number Of Points To Load Per Call For Random and Group Spots
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_dtmFeaturesForFeatureIdDtmObject
(
 BC_DTM_OBJ     *dtmP,               /* ==> Pointer To DTM Dtm object                    */
 DTMFeatureId featureId,           /* ==> Type Of DTM Feature To Be Loaded             */
 long           maxSpots,            /* ==> Maximum Number Of Spots Points To Load Per Call       */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 long           useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape> */
 DTMFenceOption   fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>          */
 DPoint3d            *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                    */
 long           numFencePts,         /* ==> Number Of Fence Points                       */
 void           *userP               /* ==> User Pointer Passed Back To User             */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DPoint3d *p3dP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interrupt Loading Dtm Feature Type From Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"featureId         = %8I64d",featureId) ;
    bcdtmWrite_message(0,0,0,"maxSpots          = %8ld",maxSpots) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
    if( fencePtsP != nullptr && numFencePts > 0 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          p3dP->z = 0.0 ;
          bcdtmWrite_message(0,0,0,"fencePts[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == nullptr || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Validate Max Spots
*/
 if( maxSpots < 0 ) maxSpots = 1 ;
 if( maxSpots > 50000 ) maxSpots = 50000 ;
 if( maxSpots > dtmP->numPoints ) maxSpots = dtmP->numPoints ;
/*
** Load DTM FeatureId Occurrences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Occurrences Of Dtm FeatureIds") ;
 if( bcdtmInterruptLoad_dtmFeatureFeatureIdOccurrencesDtmObject(dtmP,featureId,maxSpots,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Dtm Feature Type From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Dtm Feature Type From Dtm Object Error") ;
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
BENTLEYDTM_Private int bcdtmInterruptLoad_dtmFeatureFeatureIdOccurrencesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureId featureId ,long maxSpots,DTMFeatureCallback loadFunctionP,long useFence,DTMFenceType fenceType,DTMFenceOption fenceOption,DPoint3d *fencePtsP,long numFencePts,void *userP )
/*
** This Function Loads All DTM Feature Occurrences For A Feature ID
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long            pnt,pnt1,pnt2,pnt3,nextPnt,numPts,closeFlag,dtmFeature ;
 long            n,numSpots,numFeatureSpots,numFeaturePts,numClipArrays,clipResult;
 long            fndType,insideFence,fenceLoad ;
 DPoint3d             *p3dP,*featurePtsP=nullptr ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 BC_DTM_OBJ      *clipDtmP=nullptr  ;
 DTM_TIN_POINT   *pntP  ;
 DTM_POINT_ARRAY **clipArraysPP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interrupt Loading DTM Features For Feature Id") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"featureId         = %8I64d",featureId) ;
    bcdtmWrite_message(0,0,0,"maxSpots          = %8ld",maxSpots) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
    if( fencePtsP != nullptr && numFencePts > 0 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          p3dP->z = 0.0 ;
          bcdtmWrite_message(0,0,0,"fencePts[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Write Request For Development Tracing Purposes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Interrupt Loading DTM Features For FeatureID = %10I64d",featureId) ;
/*
** Build Clipping Dtm For Fence Operations
*/
 if( useFence == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Clipping Tin") ;
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Clipping Tin Completed") ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
   }
/*
**  Scan Dtm Features And Return All Features With The UserTag Value
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureId == featureId && ( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin ))
      {
/*
**     DTM Group Spot Feature - Accumulate Group Spot Points In Batches Of maxSpots
*/
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots )
         {
          if( bcdtmList_countNumberOfDtmFeaturePointsDtmObject(dtmP,dtmFeature,&numFeatureSpots,&closeFlag)) goto errexit ;
          numSpots = numFeatureSpots ;
          if( numSpots > maxSpots ) numSpots = maxSpots ;
          if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
          featurePtsP = ( DPoint3d * ) malloc ( numSpots * sizeof(DPoint3d)) ;
          if( featurePtsP == nullptr )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
/*
**       Scan Group Spot Points And Accumulate In Points Array
*/
         numSpots = 0 ;
         pnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
         for( numPts = 0 ; numPts < numFeatureSpots ; ++numPts )
           {
            pntP = pointAddrP(dtmP,pnt) ;
            if( useFence == FALSE )
              {
               (featurePtsP+numSpots)->x = pntP->x ;
               (featurePtsP+numSpots)->y = pntP->y ;
               (featurePtsP+numSpots)->z = pntP->z ;
               ++numSpots ;
              }
            else
              {
               insideFence = FALSE ;
               if( fenceType == DTMFenceType::Block && pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax ) insideFence = TRUE ; ;
               if( fenceType == DTMFenceType::Shape && pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
                 {
                  if( bcdtmFind_triangleDtmObject(dtmP,pntP->x,pntP->y,&fndType,&pnt1,&pnt2,&pnt3)) goto errexit ;
                  if( fndType ) insideFence = TRUE ;
                 }
               fenceLoad = FALSE ;
               if( ( fenceOption == DTMFenceOption::Inside  || fenceOption == DTMFenceOption::Overlap )  && insideFence == TRUE  ) fenceLoad = TRUE ;
               if(   fenceOption == DTMFenceOption::Outside && insideFence == FALSE ) fenceLoad = TRUE ;
               if( fenceLoad == TRUE )
                 {
                  (featurePtsP+numSpots)->x = pntP->x ;
                  (featurePtsP+numSpots)->y = pntP->y ;
                  (featurePtsP+numSpots)->z = pntP->z ;
                  ++numSpots ;
                 }
              }
/*
**          Check For Max Spots
*/
            if( numSpots == maxSpots )
              {
               if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,maxSpots,userP)) goto errexit ;
               numSpots = 0 ;
              }
/*
**          Get Next Group Spot Point
*/
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data ) ++pnt ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin  )
              {
               if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,pnt,&nextPnt)) goto errexit ;
               pnt = nextPnt ;
              }
           }
/*
**       Check For Remaining Spots
*/
          if( numSpots > 0 )
            {
             if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numSpots,userP)) goto errexit ;
            }
         }
/*
**     None Group Spot Feature
*/
       else
         {
          if( bcdtmObject_getPointsForDtmFeatureDtmObject(dtmP,dtmFeature,(DTM_TIN_POINT **) &featurePtsP ,&numFeaturePts)) goto errexit ;
          if( useFence == FALSE )
            {
             if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
            }
          else
            {
             if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,featurePtsP,numFeaturePts,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
             if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
             if( clipResult == 2 )
               {
                for( n = 0 ; n < numClipArrays ; ++n )
                  {
                   if( bcdtmLoad_callUserLoadFunction(loadFunctionP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                  }
                bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
               }
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( clipDtmP    != nullptr ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 if( featurePtsP != nullptr ) { free( featurePtsP) ; featurePtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Interrupt Loading DTM Features For Usertag Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Interrupt Loading DTM Features For Usertag Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
