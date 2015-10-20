/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmMem.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public BC_DTM_FEATURE * memftableAddrP(BC_DTM_OBJ *dtmP,long feature )
{
 return( dtmP->fTablePP[feature/dtmP->featurePartitionSize] + feature % dtmP->featurePartitionSize) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public  DPoint3d * mempointAddrP(BC_DTM_OBJ *dtmP,long point )
{
 return( dtmP->pointsPP[point/dtmP->pointPartitionSize] + point % dtmP->pointPartitionSize) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public  DTM_TIN_NODE * memnodeAddrP(BC_DTM_OBJ *dtmP,long node )
{
 return( dtmP->nodesPP[node/dtmP->nodePartitionSize] + node % dtmP->nodePartitionSize) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public  DTM_CIR_LIST * memclistAddrP(BC_DTM_OBJ *dtmP,long clPtr )
{
 return( dtmP->cListPP[clPtr/dtmP->clistPartitionSize] + clPtr % dtmP->clistPartitionSize) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public  DTM_FEATURE_LIST * memflistAddrP(BC_DTM_OBJ *dtmP,long flPtr )
{
 return( dtmP->fListPP[flPtr/dtmP->flistPartitionSize] + flPtr % dtmP->flistPartitionSize) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMem_storePointsInPointerArray(DTM_POINT_ARRAY **pointArrayPP,long numPointArray,DPoint3d **ptsPP, long numPts )
/*
** This Function Store Points In A Pointer Array To Point Array
*/
{
 int  ret=DTM_SUCCESS ;
 DTM_POINT_ARRAY *pointArrayP=NULL ;
/*
** Allocate Point Array Memory
*/
 pointArrayP = ( DTM_POINT_ARRAY * ) malloc ( sizeof(DTM_POINT_ARRAY));
 if( pointArrayP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   } 
/*
** Store Points In Point Array
*/
 pointArrayP->pointsP   = *ptsPP  ;
 pointArrayP->numPoints = numPts  ;
 *ptsPP = NULL ;
/*
** Store Point Array In Pointer Array 
*/
 *(pointArrayPP+numPointArray) = pointArrayP  ;
 pointArrayP = NULL ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( pointArrayP    != NULL ) { free(pointArrayP)    ; pointArrayP    = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMem_storePointsInPointerArrayToPointArray(DTM_POINT_ARRAY ***pointArrayPPP,DPoint3d **ptsPP, long numPts )
/*
** This Function Store Points In A Pointer Array To Point Array
*/
{
 int  ret=DTM_SUCCESS ;
 DTM_POINT_ARRAY *pointArrayP=NULL ;
/*
** Allocate Point Array Memory
*/
 pointArrayP = ( DTM_POINT_ARRAY * ) malloc ( sizeof(DTM_POINT_ARRAY));
 if( pointArrayP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   } 
/*
** Allocate Pointer To Point Array Memory
*/
 *pointArrayPPP = ( DTM_POINT_ARRAY ** ) malloc ( sizeof( DTM_POINT_ARRAY *));
 if( *pointArrayPPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   } 
/*
** Store Points In Point Array
*/
 pointArrayP->pointsP   = *ptsPP  ;
 pointArrayP->numPoints = numPts  ;
 *ptsPP = NULL ;
/*
** Store Point Array In Pointer Array To Point Array
*/
 *(*pointArrayPPP) = pointArrayP  ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( pointArrayP    != NULL ) { free(pointArrayP)    ; pointArrayP    = NULL ; }
 if( *pointArrayPPP != NULL ) { free(*pointArrayPPP) ; *pointArrayPPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMem_storePointsInExistingPointerArrayToPointArray(DTM_POINT_ARRAY **pointArrayPP,long numPointArrays,DPoint3d *ptsP, long numPts )
/*
** This Function Store Points In A Pointer Array To Point Array
*/
{
 int  ret=DTM_SUCCESS ;
 DTM_POINT_ARRAY *pointArrayP=NULL ;
/*
** Allocate Point Array Memory
*/
 pointArrayP = ( DTM_POINT_ARRAY * ) malloc ( sizeof(DTM_POINT_ARRAY));
 if( pointArrayP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   } 
/*
** Store Points In Point Array
*/
 pointArrayP->pointsP   = ptsP  ;
 pointArrayP->numPoints = numPts  ;
/*
** Store Point Array In Pointer Array To Point Array
*/
 *(pointArrayPP+numPointArrays) = pointArrayP  ;
 pointArrayP = NULL ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( pointArrayP    != NULL ) { free(pointArrayP)    ; pointArrayP    = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmMem_freePointerArrayToPointArrayMemory(DTM_POINT_ARRAY ***pointArraysPPP,long numPointArrays)
/*
** This Function Frees The Memory Allocated to A Pointer Array Of Point Arrays
*/
{
 int dbg=DTM_TRACE_VALUE(0) ;
 DTM_POINT_ARRAY **pointArrayPP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Freeing Pointer Array To Point Arrays") ;
    bcdtmWrite_message(0,0,0,"*pointArraysPPP = %p",*pointArraysPPP) ;
    bcdtmWrite_message(0,0,0,"numPointArrays  = %8ld",numPointArrays) ;
   }
/*
** Scan Pointer Array And Free Point Array Memory
*/
 if( *pointArraysPPP != NULL )
   { 
    for( pointArrayPP = *pointArraysPPP ; pointArrayPP < *pointArraysPPP + numPointArrays ; ++pointArrayPP )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Freeing Point Array *pointArrayPP = %p",*pointArrayPP) ;
       if( *pointArrayPP != NULL )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"***** Freeing Points = %p",(*pointArrayPP)->pointsP) ;
          if( (*pointArrayPP)->pointsP != NULL )
            {
             free((*pointArrayPP)->pointsP) ;
             (*pointArrayPP)->pointsP = NULL ;
             if( dbg ) bcdtmWrite_message(0,0,0,"***** Freeing Points = %p Completed",(*pointArrayPP)->pointsP) ;
            }
          free(*pointArrayPP) ;
          *pointArrayPP = NULL ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Freeing Point Array *pointArrayPP = %p Completed",*pointArrayPP) ;
         }
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Freeing Pointer Array To Point Arrays *pointArraysPPP = %p",*pointArraysPPP) ;
    free(*pointArraysPPP) ;
    *pointArraysPPP = NULL ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Freeing Pointer Array To Point Arrays *pointArraysPPP = %p Completed",*pointArraysPPP) ;
   } 
/*
** Return
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMem_allocatePointerArrayToPointArrayMemory(DTM_POINT_ARRAY ***pointArraysPPP,long numPointArrays,long *memPointArraysP,long memPointArraysInc)
/*
** This Function Allocates Pointer Array Memory For Point Arrays
*/
{
 int  ret=DTM_SUCCESS ;
 DTM_POINT_ARRAY **pointArrayPP ;
/*
** Set Size Of Pointer Array To Be Allocated
*/
 *memPointArraysP = *memPointArraysP + memPointArraysInc ;
/*
** Allocate memory
*/
 if( *pointArraysPPP == NULL ) *pointArraysPPP = ( DTM_POINT_ARRAY ** ) malloc ( *memPointArraysP * sizeof( DTM_POINT_ARRAY * )) ;
 else                          *pointArraysPPP = ( DTM_POINT_ARRAY ** ) realloc ( *pointArraysPPP, *memPointArraysP * sizeof( DTM_POINT_ARRAY * )) ;
 if( *pointArraysPPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Set New Pointer Arrays To Null
*/
 for( pointArrayPP = *pointArraysPPP + numPointArrays ; pointArrayPP < *pointArraysPPP + *memPointArraysP ; ++pointArrayPP )
   {
    *pointArrayPP = NULL ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

