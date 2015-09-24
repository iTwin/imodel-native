/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmObj.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
 #include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
#include <mutex>

std::mutex s_dtmMutex;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_createVer200DtmObject(BC_DTM_OBJ_VER_200 **dtmPP )
/*
** This Function Creates a BC DTM Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 static int dtmObjectCount=0 ;
/*
** Check Dtm Object Pointer
*/
 if(  dtmPP == NULL ) { bcdtmWrite_message(2,0,0,"Null Dtm Object Pointer Address") ; goto errexit ; }
 if( *dtmPP != NULL ) { bcdtmWrite_message(2,0,0,"None Null Dtm Object Pointer")    ; goto errexit ; }
/*
** Create B Tree For BC DTM Dtm Objects Exists. If It Doesn't Exist Then Create It
*/
        {
        std::lock_guard<std::mutex> lock (s_dtmMutex);
        if (glbDtmObjBtreeP == NULL) if (bcdtmBtree_createBtree (&glbDtmObjBtreeP, BC_DTM_MAX_OBJS)) goto errexit;
        /*
        ** Check Number Of Entries
        */
        if (glbDtmObjBtreeP->activeNodes >= BC_DTM_MAX_OBJS)
            {
            bcdtmWrite_message (1, 0, 0, "Maximum BC Dtm Dtm Objects Exceeded");
            goto errexit;
            }
            }
/*
** Create Dtm Object
*/ 
 *dtmPP = ( BC_DTM_OBJ_VER_200 * ) malloc( sizeof( BC_DTM_OBJ_VER_200 )) ;
 if( *dtmPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Initialize Dtm Object
*/ 
 (*dtmPP)->dtmObjType           =  BC_DTM_OBJ_TYPE  ;
 (*dtmPP)->dtmObjVersion        =  BC_DTM_OBJ_VERSION ;
 (*dtmPP)->numLines             =  0 ;
 (*dtmPP)->numTriangles         =  0 ;
 (*dtmPP)->numFeatures          =  0 ;
 (*dtmPP)->memFeatures          =  0 ;
 (*dtmPP)->iniFeatures          =  BC_DTM_INI_FEATURES ;
 (*dtmPP)->incFeatures          =  BC_DTM_INC_FEATURES ;
 (*dtmPP)->numFeaturePartitions =  0 ;
 (*dtmPP)->featurePartitionSize =  DTM_PARTITION_SIZE_FEATURE ;
 (*dtmPP)->numPoints            =  0 ;
 (*dtmPP)->memPoints            =  0 ;
 (*dtmPP)->numSortedPoints      =  0 ;
 (*dtmPP)->iniPoints            =  BC_DTM_INI_POINTS ;
 (*dtmPP)->incPoints            =  BC_DTM_INC_POINTS ;
 (*dtmPP)->numPointPartitions   =  0 ;
 (*dtmPP)->pointPartitionSize   =  DTM_PARTITION_SIZE_POINT ;
 (*dtmPP)->numNodes             =  0 ;
 (*dtmPP)->memNodes             =  0 ;
 (*dtmPP)->numNodePartitions    =  0 ;
 (*dtmPP)->nodePartitionSize    =  DTM_PARTITION_SIZE_NODE ;
 (*dtmPP)->numClist             =  0 ;
 (*dtmPP)->memClist             =  0 ;
 (*dtmPP)->numClistPartitions   =  0 ;
 (*dtmPP)->clistPartitionSize   =  DTM_PARTITION_SIZE_CLIST ;
 (*dtmPP)->numFlist             =  0 ;
 (*dtmPP)->memFlist             =  0 ;
 (*dtmPP)->iniFlist             =  BC_DTM_INI_FLIST ;
 (*dtmPP)->incFlist             =  BC_DTM_INC_FLIST ;
 (*dtmPP)->numFlistPartitions   =  0 ;
 (*dtmPP)->flistPartitionSize   =  DTM_PARTITION_SIZE_FLIST ;
 (*dtmPP)->dtmState             =  DTMState::Data ;
 (*dtmPP)->nullPnt              =  DTM_NULL_PNT ;
 (*dtmPP)->nullPtr              =  DTM_NULL_PTR ;
 (*dtmPP)->dtmFeatureIndex      =  0 ;
 (*dtmPP)->nullUserTag          =  DTM_NULL_USER_TAG ;
 (*dtmPP)->nullFeatureId        =  DTM_NULL_FEATURE_ID ;
 (*dtmPP)->edgeOption           =  2  ;
 (*dtmPP)->cListPtr             =  0  ;
 (*dtmPP)->cListDelPtr          =  DTM_NULL_PTR ;
 (*dtmPP)->fListDelPtr          =  DTM_NULL_PTR ;
 (*dtmPP)->refCount             =  0 ;
 (*dtmPP)->userStatus           =  0 ;
 (*dtmPP)->creationTime         =  0 ;
 (*dtmPP)->modifiedTime         =  0 ;
 (*dtmPP)->userTime             =  0 ;
 (*dtmPP)->ppTol                = DTM_PPTOL ;
 (*dtmPP)->plTol                = DTM_PLTOL ; 
 (*dtmPP)->maxSide              = 1000.0 ; 
 (*dtmPP)->transMatrix[0][0]    = 1.0 ; 
 (*dtmPP)->transMatrix[0][1]    = 0.0 ; 
 (*dtmPP)->transMatrix[0][2]    = 0.0 ; 
 (*dtmPP)->transMatrix[0][3]    = 0.0 ; 
 (*dtmPP)->transMatrix[1][0]    = 0.0 ; 
 (*dtmPP)->transMatrix[1][1]    = 1.0 ; 
 (*dtmPP)->transMatrix[1][2]    = 0.0 ; 
 (*dtmPP)->transMatrix[1][3]    = 0.0 ; 
 (*dtmPP)->transMatrix[2][0]    = 0.0 ; 
 (*dtmPP)->transMatrix[2][1]    = 0.0 ; 
 (*dtmPP)->transMatrix[2][2]    = 1.0 ; 
 (*dtmPP)->transMatrix[2][3]    = 0.0 ; 
 (*dtmPP)->transMatrix[3][0]    = 0.0 ; 
 (*dtmPP)->transMatrix[3][1]    = 0.0 ; 
 (*dtmPP)->transMatrix[3][2]    = 0.0 ; 
 (*dtmPP)->transMatrix[3][3]    = 1.0 ; 
 (*dtmPP)->mppTol               = 0.0 ; 
 (*dtmPP)->xMin                 = 0.0 ;
 (*dtmPP)->yMin                 = 0.0 ;
 (*dtmPP)->zMin                 = 0.0 ;
 (*dtmPP)->xMax                 = 0.0 ;
 (*dtmPP)->yMax                 = 0.0 ;
 (*dtmPP)->zMax                 = 0.0 ;
 (*dtmPP)->xRange               = 0.0 ;
 (*dtmPP)->yRange               = 0.0 ;
 (*dtmPP)->zRange               = 0.0 ;
 (*dtmPP)->fTablePP             = NULL ;
 (*dtmPP)->pointsPP             = NULL ;
 (*dtmPP)->nodesPP              = NULL ;
 (*dtmPP)->cListPP              = NULL ;
 (*dtmPP)->fListPP              = NULL ;
 (*dtmPP)->DTMAllocationClass = NULL;
 /*
 ** Add Dtm Object To B Tree
 */
     {
     std::lock_guard<std::mutex> lock (s_dtmMutex);
     if (bcdtmBtree_addNode (glbDtmObjBtreeP, (BC_DTM_OBJ *)*dtmPP)) goto errexit;
     }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Created[%5ld] BC_DTM_OBJ %p",dtmObjectCount,*dtmPP) ;
 ++dtmObjectCount ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( *dtmPP != NULL )  { free(*dtmPP) ; *dtmPP = NULL ; }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/

BENTLEYDTM_EXPORT int bcdtmObject_createDTMExtended (BC_DTM_OBJ_EXTENDED ** dtmObjectExtendedPP)
    {
    int ret = DTM_SUCCESS;
    BC_DTM_OBJ_EXTENDED* dtmObjectExtendedP = new BC_DTM_OBJ_EXTENDED() ;
    if( dtmObjectExtendedP == NULL )
        {
        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
        } 
    memset (dtmObjectExtendedP, 0, sizeof (BC_DTM_OBJ_EXTENDED));
    dtmObjectExtendedP->triangulationCheckStopCallBackP = NULL ;
    dtmObjectExtendedP->rollBackInfoP = NULL ;
    dtmObjectExtendedP->progressCallBackP = NULL;

    *dtmObjectExtendedPP = dtmObjectExtendedP;
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Return
    */
    return ret;
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
BENTLEYDTM_EXPORT int bcdtmObject_destoryDTMExtended (BC_DTM_OBJ_EXTENDED ** dtmObjectExtendedPP)
{
 BC_DTM_OBJ_EXTENDED* dtmObjectExtendedP = *dtmObjectExtendedPP;
 if (dtmObjectExtendedP)
   {
   delete dtmObjectExtendedP ; 
   }
 *dtmObjectExtendedPP = 0;
 return DTM_SUCCESS;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_createDtmObject(BC_DTM_OBJ **dtmPP )
/*
** This Function Creates a BC DTM Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 static int dtmObjectCount=0 ;
/*
** Check Dtm Object Pointer
*/
 if(  dtmPP == NULL ) { bcdtmWrite_message(2,0,0,"Null Dtm Object Pointer Address") ; goto errexit ; }
 if( *dtmPP != NULL ) { bcdtmWrite_message(2,0,0,"None Null Dtm Object Pointer")    ; goto errexit ; }
/*
** Create B Tree For BC DTM Dtm Objects Exists. If It Doesn't Exist Then Create It
*/
    {
            std::lock_guard<std::mutex> lock (s_dtmMutex);
            if (glbDtmObjBtreeP == NULL) if (bcdtmBtree_createBtree (&glbDtmObjBtreeP, BC_DTM_MAX_OBJS)) goto errexit;
            /*
            ** Check Number Of Entries
            */
            if (glbDtmObjBtreeP->activeNodes >= BC_DTM_MAX_OBJS)
                {
                bcdtmWrite_message (1, 0, 0, "Maximum BC Dtm Dtm Objects Exceeded");
                goto errexit;
                }
            }
/*
** Create Dtm Object
*/ 
 *dtmPP = new BC_DTM_OBJ ();

 if( *dtmPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Initialize Dtm Object
*/ 
 memset (*dtmPP, 0, BCDTMSize);
 (*dtmPP)->dtmObjType           =  BC_DTM_OBJ_TYPE  ;
 (*dtmPP)->dtmObjVersion        =  BC_DTM_OBJ_VERSION ;
 (*dtmPP)->numLines             =  0 ;
 (*dtmPP)->numTriangles         =  0 ;
 (*dtmPP)->numFeatures          =  0 ;
 (*dtmPP)->memFeatures          =  0 ;
 (*dtmPP)->iniFeatures          =  BC_DTM_INI_FEATURES ;
 (*dtmPP)->incFeatures          =  BC_DTM_INC_FEATURES ;
 (*dtmPP)->numFeaturePartitions =  0 ;
 (*dtmPP)->featurePartitionSize =  DTM_PARTITION_SIZE_FEATURE ;
 (*dtmPP)->numPoints            =  0 ;
 (*dtmPP)->memPoints            =  0 ;
 (*dtmPP)->numSortedPoints      =  0 ;
 (*dtmPP)->iniPoints            =  BC_DTM_INI_POINTS ;
 (*dtmPP)->incPoints            =  BC_DTM_INC_POINTS ;
 (*dtmPP)->numPointPartitions   =  0 ;
 (*dtmPP)->pointPartitionSize   =  DTM_PARTITION_SIZE_POINT ;
 (*dtmPP)->numNodes             =  0 ;
 (*dtmPP)->memNodes             =  0 ;
 (*dtmPP)->numNodePartitions    =  0 ;
 (*dtmPP)->nodePartitionSize    =  DTM_PARTITION_SIZE_NODE ;
 (*dtmPP)->numClist             =  0 ;
 (*dtmPP)->memClist             =  0 ;
 (*dtmPP)->numClistPartitions   =  0 ;
 (*dtmPP)->clistPartitionSize   =  DTM_PARTITION_SIZE_CLIST ;
 (*dtmPP)->numFlist             =  0 ;
 (*dtmPP)->memFlist             =  0 ;
 (*dtmPP)->iniFlist             =  BC_DTM_INI_FLIST ;
 (*dtmPP)->incFlist             =  BC_DTM_INC_FLIST ;
 (*dtmPP)->numFlistPartitions   =  0 ;
 (*dtmPP)->flistPartitionSize   =  DTM_PARTITION_SIZE_FLIST ;
 (*dtmPP)->dtmState             =  DTMState::Data ;
 (*dtmPP)->dtmCleanUp           =  DTMCleanupFlags::None ;
 (*dtmPP)->obsolete_dtmRestoreTriangles = 0;
 (*dtmPP)->nullPnt              =  DTM_NULL_PNT ;
 (*dtmPP)->nullPtr              =  DTM_NULL_PTR ;
 (*dtmPP)->dtmFeatureIndex      =  0 ;
 (*dtmPP)->nullUserTag          =  DTM_NULL_USER_TAG ;
 (*dtmPP)->nullFeatureId        =  DTM_NULL_FEATURE_ID ;
 (*dtmPP)->edgeOption           =  2  ;
 (*dtmPP)->cListPtr             =  0  ;
 (*dtmPP)->cListDelPtr          =  DTM_NULL_PTR ;
 (*dtmPP)->fListDelPtr          =  DTM_NULL_PTR ;
 (*dtmPP)->refCount             =  0 ;
 (*dtmPP)->userStatus           =  0 ;
 (*dtmPP)->creationTime         =  0 ;
 (*dtmPP)->modifiedTime         =  0 ;
 (*dtmPP)->userTime             =  0 ;
#ifndef _WIN32_WCE 
 _time32(&(*dtmPP)->creationTime) ;
#endif 
 (*dtmPP)->ppTol                = DTM_PPTOL ;
 (*dtmPP)->plTol                = DTM_PLTOL ; 
 (*dtmPP)->maxSide              = 1000.0 ; 
 (*dtmPP)->mppTol               = 0.0 ; 
 (*dtmPP)->xMin                 = 0.0 ;
 (*dtmPP)->yMin                 = 0.0 ;
 (*dtmPP)->zMin                 = 0.0 ;
 (*dtmPP)->xMax                 = 0.0 ;
 (*dtmPP)->yMax                 = 0.0 ;
 (*dtmPP)->zMax                 = 0.0 ;
 (*dtmPP)->xRange               = 0.0 ;
 (*dtmPP)->yRange               = 0.0 ;
 (*dtmPP)->zRange               = 0.0 ;
 (*dtmPP)->fTablePP             = NULL ;
 (*dtmPP)->pointsPP             = NULL ;
 (*dtmPP)->nodesPP              = NULL ;
 (*dtmPP)->cListPP              = NULL ;
 (*dtmPP)->fListPP              = NULL ;
 (*dtmPP)->DTMAllocationClass   = NULL ;
 (*dtmPP)->extended = NULL ; 
 bcdtmObject_updateLastModifiedTime (*dtmPP);
 /*
 ** Add Dtm Object To B Tree
 */
     {
     std::lock_guard<std::mutex> lock (s_dtmMutex);
     if (bcdtmBtree_addNode (glbDtmObjBtreeP, *dtmPP)) goto errexit;
     }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Created[%5ld] BC_DTM_OBJ %p",dtmObjectCount,*dtmPP) ;
 ++dtmObjectCount ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( *dtmPP != NULL )  { free(*dtmPP) ; *dtmPP = NULL ; }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_updateDtmObjectForDtmElement 
(
 BC_DTM_OBJ *dtmP,
 void *headerP,
 void *featureArraysP,
 void *pointArraysP,
 void *nodeArraysP,
 void *fListArraysP,
 void *cListArraysP
)
{
    BC_DTM_OBJ old;
    
// bcdtmWrite_message(0,0,0,"Updating DTM Object %p For Dtm Element dtmP->numFlist = %8ld dtmP->memFlist = %8ld",dtmP,dtmP->numFlist,dtmP->memFlist) ;    

    memcpy(&old, dtmP, BCDTMSize);
    memcpy(dtmP, headerP, DTMIOHeaderSize_VER200);

    dtmP->dtmObjType = old.dtmObjType;
    dtmP->refCount = old.refCount;

    if (dtmP->numFeaturePartitions)
        {
        dtmP->fTablePP = (BC_DTM_FEATURE**)realloc(old.fTablePP, sizeof(BC_DTM_FEATURE *) * dtmP->numFeaturePartitions);
        memcpy(dtmP->fTablePP, featureArraysP, sizeof(BC_DTM_FEATURE *) * dtmP->numFeaturePartitions);
        }
    else if (dtmP->fTablePP)
        {
        free (dtmP->fTablePP);
        dtmP->fTablePP = NULL;
        }


    if (dtmP->numPointPartitions)
        {
        dtmP->pointsPP = (DTM_TIN_POINT**)realloc(old.pointsPP, sizeof(DTM_TIN_POINT *) * dtmP->numPointPartitions);
        memcpy(dtmP->pointsPP, pointArraysP, sizeof(DTM_TIN_POINT *) * dtmP->numPointPartitions);
        }
    else if (dtmP->pointsPP)
        {
        free (dtmP->pointsPP);
        dtmP->pointsPP = NULL;
        }

    if (dtmP->numNodePartitions)
        {
        dtmP->nodesPP = (DTM_TIN_NODE**)realloc(old.nodesPP, sizeof(DTM_TIN_NODE *) * dtmP->numNodePartitions);
        memcpy(dtmP->nodesPP, nodeArraysP , sizeof(DTM_TIN_NODE *) * dtmP->numNodePartitions);
        }
    else if (dtmP->nodesPP)
        {
        free (dtmP->nodesPP);
        dtmP->nodesPP = NULL;
        }

    if (dtmP->numClistPartitions)
        {
        dtmP->cListPP = (DTM_CIR_LIST**)realloc(old.cListPP, sizeof(DTM_CIR_LIST *) * dtmP->numClistPartitions);
        memcpy(dtmP->cListPP, cListArraysP, sizeof(DTM_CIR_LIST *) * dtmP->numClistPartitions);
        }
    else if (dtmP->cListPP)
        {
        free (dtmP->cListPP);
        dtmP->cListPP = NULL;
        }

    if (dtmP->numFlistPartitions)
        {
        dtmP->fListPP = (DTM_FEATURE_LIST**)realloc(old.fListPP, sizeof(DTM_FEATURE_LIST *) * dtmP->numFlistPartitions);
        memcpy(dtmP->fListPP, fListArraysP, sizeof(DTM_FEATURE_LIST *) * dtmP->numFlistPartitions);
        }
    else if (dtmP->fListPP)
        {
        free (dtmP->fListPP);
        dtmP->fListPP = NULL;
        }

    return DTM_SUCCESS;
}

#ifdef DTMElement
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_createDtmObjectForDtmElement (BC_DTM_OBJ **dtmPP, BC_DTM_OBJ *dtmHeaderP)
/*
** This Function Creates a BC DTM Object
*/
{
 int ret=DTM_SUCCESS ;
 BC_DTM_OBJ *dtmElmP=NULL ;
 
bcdtmWrite_message(0,0,0,"Creating DTM Object For Dtm Element") ;    
 
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmHeaderP)) goto errexit ;
/*
** Create Dtm Element
*/ 
 dtmElmP = new BC_DTM_OBJ () ;
 if( dtmElmP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 bcdtmWrite_message(0,0,0,"dtmElp = %p",dtmElmP) ;
/*
** Copy Header Values From Dtm Object To Element
*/
 memset(dtmElmP, 0, BCDTMSize);
 memcpy(dtmElmP, dtmHeaderP, DTMIOHeaderSize_VER200);
 dtmElmP->dtmObjType = BC_DTM_ELM_TYPE  ;
 dtmElmP->refCount = 0  ;
 *dtmPP = dtmElmP ;
 dtmElmP = NULL ;
/*
** Copy DTM Element Header To x Attributes
*/
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
BENTLEYDTM_EXPORT int bcdtmObject_createDtmElementFromDtmObject
(
 BC_DTM_OBJ *dtmP,              /* ==> Pointer To Dtm Object          */
 void* memoryAllocator,
 void *(*managedFunctionP)(),   /* ==> Pointer To Managed Function    */
 void *userP                    /* ==> User Pointer Passed Back       */           
)
/*
** This Function Creates a BC DTM MicroStation Element From A BC DTM Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,numPartition,remPartition ;
 BC_DTM_OBJ       dtmElement,*headerElmP ;
 DTM_TIN_POINT    *pointsElemP ;
 DTM_TIN_NODE     *nodesElemP ;
 DTM_CIR_LIST     *clistElemP ;
 DTM_FEATURE_LIST *flistElemP ;
 BC_DTM_FEATURE   *ftableElemP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Dtm Element From Dtm Object %p",dtmP) ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Copy DTM Header
*/
 memcpy(&dtmElement,dtmP,BCDTMSize) ;
/*
** Write Stats On Dtm Arrays
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dtmElement.fTablePP = %p ** numFeatures = %8ld memFeatures = %8ld",dtmElement.fTablePP,dtmElement.numFeatures,dtmElement.memFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmElement.pointsPP = %p ** numPoints   = %8ld memPoints   = %8ld",dtmElement.pointsPP,dtmElement.numPoints,dtmElement.memPoints) ;
    bcdtmWrite_message(0,0,0,"dtmElement.nodesPP  = %p ** numNodes    = %8ld memNodes    = %8ld",dtmElement.nodesPP,dtmElement.numNodes,dtmElement.memNodes ) ;
    bcdtmWrite_message(0,0,0,"dtmElement.cListPP  = %p ** numClist    = %8ld memClist    = %8ld",dtmElement.cListPP,dtmElement.numClist,dtmElement.memClist ) ;
    bcdtmWrite_message(0,0,0,"dtmElement.fListPP  = %p ** numFlist    = %8ld memFlist    = %8ld",dtmElement.fListPP,dtmElement.numFlist,dtmElement.memFlist ) ;
   }
/*
** Copy Feature Table Array
*/
 if( dtmElement.fTablePP != NULL  )
   {
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Copying Dtm Feature Array   ** Memory Size = %9ld",sizeof(BC_DTM_FEATURE) * dtmElement.numFeatures) ;
       bcdtmWrite_message(0,0,0,"**** numFeatures = %10ld partitionSize = %9ld numPartitions = %9ld",dtmElement.numFeatures,dtmElement.featurePartitionSize,dtmElement.numFeaturePartitions) ;
      }
/*
**  Determine Number Of Partitions
*/
    numPartition = dtmElement.numFeatures / dtmElement.featurePartitionSize ; 
    remPartition = dtmElement.numFeatures % dtmElement.featurePartitionSize ;
/*
**  Allocate Memory For Storing Dtm Element Feature Partitions
*/
    dtmElement.fTablePP = ( BC_DTM_FEATURE ** ) malloc ( dtmElement.numFeaturePartitions * sizeof( BC_DTM_FEATURE *)) ;
    if( dtmElement.fTablePP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Copy Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
/*
**     Store Feature In x Attributes
*/
       ftableElemP = ( BC_DTM_FEATURE * ) managedFunctionP(n,DTMPartition::Feature,dtmP->fTablePP[n],sizeof(BC_DTM_FEATURE)*dtmElement.featurePartitionSize,userP) ;
       if( ftableElemP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Feature Partition Memory Allocation Failure") ;
          goto errexit ; 
         } 
       else dtmElement.fTablePP[n] = ftableElemP ;
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
/*
**     Store Feature In x Attributes
*/
       ftableElemP = ( BC_DTM_FEATURE * )managedFunctionP(n,DTMPartition::Feature,dtmP->fTablePP[n],sizeof(BC_DTM_FEATURE)*remPartition,userP) ;
       if( ftableElemP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Feature Element Memory Allocation Failure") ;
          goto errexit ; 
         } 
       else dtmElement.fTablePP[n] = ftableElemP ;
      }
/*
**  Set Number Of Memory Features
*/
    dtmElement.memFeatures = dtmElement.numFeatures ;
   }
/*
** Copy Points Array
*/
 if( dtmElement.pointsPP != NULL  )
   {
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Copying Dtm Points Array    ** Memory Size = %9ld",dtmElement.numPointPartitions,dtmElement.pointPartitionSize,sizeof(DTM_TIN_POINT)*dtmElement.numPoints) ;
       bcdtmWrite_message(0,0,0,"**** numPoints   = %10ld partitionSize = %9ld numPartitions = %9ld",dtmElement.numPoints,dtmElement.pointPartitionSize,dtmElement.numPointPartitions) ;
      }

/*
**  Determine Number Of Partitions
*/
    numPartition = dtmElement.numPoints / dtmElement.pointPartitionSize ; 
    remPartition = dtmElement.numPoints % dtmElement.pointPartitionSize ;
/*
**  Allocate Memory For Storing Point Partitions
*/
    dtmElement.pointsPP = ( DTM_TIN_POINT ** ) malloc ( dtmElement.numPointPartitions * sizeof(DTM_TIN_POINT *)) ;
    if( dtmElement.pointsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Copy Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
/*
**     Store Points In x Attributes
*/
       pointsElemP = ( DTM_TIN_POINT * ) managedFunctionP(n,DTMPartition::Point,dtmP->pointsPP[n],sizeof(DTM_TIN_POINT)*dtmElement.pointPartitionSize,userP) ;
       if( pointsElemP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Points Partition Memory Allocation Failure") ;
          goto errexit ; 
         } 
       else dtmElement.pointsPP[n] = pointsElemP ;
      }
/*
**  Copy Partial Partition
*/
    if( remPartition > 0 )
      {
/*
**     Store Points In x Attributes
*/
       pointsElemP = ( DTM_TIN_POINT * )managedFunctionP(n,DTMPartition::Point,dtmP->pointsPP[n],sizeof(DTM_TIN_POINT)*remPartition,userP) ;
       if( pointsElemP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Points Partition Memory Allocation Failure") ;
          goto errexit ; 
         } 
       else dtmElement.pointsPP[n] = pointsElemP ;
      }
/*
**  Set Number Of Memory Points
*/
    dtmElement.memPoints = dtmElement.numPoints ;
   }
/*
** Copy Nodes Array
*/
 if( dtmElement.nodesPP != NULL  )
   {
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Copying Dtm Nodes Array    ** Memory Size = %9ld",sizeof(DTM_TIN_NODE) * dtmElement.numNodes) ;
       bcdtmWrite_message(0,0,0,"**** numNodes    = %10ld partitionSize = %9ld numPartitions = %9ld",dtmElement.numNodes,dtmElement.nodePartitionSize,dtmElement.numNodePartitions) ;
      }
/*
**  Determine Number Of Partitions
*/
    numPartition = dtmElement.numPoints / dtmElement.nodePartitionSize ; 
    remPartition = dtmElement.numPoints % dtmElement.nodePartitionSize ;
/*
**  Allocate Memory For Storing Node Partitions
*/
    dtmElement.nodesPP = ( DTM_TIN_NODE ** ) malloc ( dtmElement.numNodePartitions * sizeof( DTM_TIN_NODE *)) ;
    if( dtmElement.nodesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Copy Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
/*
**     Store Nodes In x Attributes
*/
       nodesElemP = ( DTM_TIN_NODE * )managedFunctionP(n,DTMPartition::Node,dtmP->nodesPP[n],sizeof(DTM_TIN_NODE)*dtmElement.nodePartitionSize,userP) ;
       if( nodesElemP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Nodes Partition Memory Allocation Failure") ;
          goto errexit ; 
         } 
       else dtmElement.nodesPP[n] = nodesElemP ;
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
/*
**     Store Nodes In x Attributes
*/
       nodesElemP = ( DTM_TIN_NODE * ) managedFunctionP(n,DTMPartition::Node,dtmP->nodesPP[n],sizeof(DTM_TIN_NODE)*remPartition,userP) ;
       if( nodesElemP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Nodes Partition Memory Allocation Failure") ;
          goto errexit ; 
         } 
       else dtmElement.nodesPP[n] = nodesElemP ;
      }
/*
**  Set Number Of Memory Nodes
*/
    dtmElement.memNodes = dtmElement.numNodes = dtmElement.numPoints ;
   }
/*
** Copy Circular List
*/
 if( dtmElement.cListPP != NULL  )
   {
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Copying Dtm Clist Array    ** Memory Size = %9ld",sizeof(DTM_CIR_LIST) * dtmElement.cListPtr) ;
       bcdtmWrite_message(0,0,0,"**** numClist    = %10ld partitionSize = %9ld numPartitions = %9ld",dtmElement.numClist,dtmElement.clistPartitionSize,dtmElement.numClistPartitions) ;
      }
/*
**  Determine Number Of Partitions
*/
    numPartition = dtmElement.cListPtr / dtmElement.clistPartitionSize ; 
    remPartition = dtmElement.cListPtr % dtmElement.clistPartitionSize ;
/*
**  Allocate Memory For Storing Clist Partitions
*/
    dtmElement.cListPP = ( DTM_CIR_LIST ** ) malloc ( dtmElement.numClistPartitions * sizeof( DTM_CIR_LIST *)) ;
    if( dtmElement.cListPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Copy Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
/*
**     Store Clist In x Attributes
*/
       clistElemP = ( DTM_CIR_LIST * )managedFunctionP(n,DTMPartition::CList,dtmP->cListPP[n],sizeof(DTM_CIR_LIST)* dtmElement.clistPartitionSize,userP) ;
       if( clistElemP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Clist Partition Memory Allocation Failure") ;
          goto errexit ; 
         } 
       else dtmElement.cListPP[n] = clistElemP ;
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
/*
**     Store Clist In x Attributes
*/
       clistElemP = ( DTM_CIR_LIST * )managedFunctionP(n,DTMPartition::CList,dtmP->cListPP[n],sizeof(DTM_CIR_LIST)*remPartition,userP) ;
       if( clistElemP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Clist Partition Memory Allocation Failure") ;
          goto errexit ; 
         } 
       else dtmElement.cListPP[n] = clistElemP ;
      }
/*
**  Set Number Of Memory Clist
*/
    dtmElement.memClist = dtmElement.numClist = dtmElement.cListPtr ;
   }
/*
** Copy Feature List Array
*/
 if( dtmElement.fListPP != NULL  )
   {
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Copying Dtm Flist Array    ** Memory Size = %9ld",sizeof(DTM_FEATURE_LIST) * dtmElement.cListPtr) ;
       bcdtmWrite_message(0,0,0,"**** numFlist    = %10ld partitionSize = %9ld numPartitions = %9ld",dtmElement.numFlist,dtmElement.flistPartitionSize,dtmElement.numFlistPartitions) ;
      } 
/*
**  Determine Number Of Partitions
*/
    numPartition = dtmElement.numFlist / dtmElement.flistPartitionSize ; 
    remPartition = dtmElement.numFlist % dtmElement.flistPartitionSize ;
/*
**  Allocate Memory For Storing Flist Partitions
*/
    dtmElement.fListPP = ( DTM_FEATURE_LIST ** ) malloc ( dtmElement.numFlistPartitions * sizeof( DTM_FEATURE_LIST *)) ;
    if( dtmElement.fListPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
/*
**     Store Flist In x Attributes
*/
       flistElemP = ( DTM_FEATURE_LIST * ) managedFunctionP(n,DTMPartition::FList,dtmP->fListPP[n],sizeof(DTM_FEATURE_LIST)*dtmElement.flistPartitionSize,userP) ;
       if( flistElemP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Flist Partition Memory Allocation Failure") ;
          goto errexit ; 
         } 
       else dtmElement.fListPP[n] = flistElemP ;
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
/*
**     Store Flist In x Attributes
*/
       flistElemP = ( DTM_FEATURE_LIST * ) managedFunctionP(n,DTMPartition::FList,dtmP->fListPP[n],sizeof(DTM_FEATURE_LIST)*remPartition,userP) ;
       if( flistElemP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Flist Partition Memory Allocation Failure") ;
          goto errexit ; 
         } 
       else dtmElement.fListPP[n] = flistElemP ;
      }
/*
**  Set Number Of Memory Flist
*/
    dtmElement.memFlist = dtmElement.numFlist ;
   }
/*
** Store DTM Header Partition
*/
 dtmElement.dtmObjType = BC_DTM_ELM_TYPE ; 
 headerElmP = managedFunctionP(0,DTMPartition::Header,&dtmElement,sizeof(BC_DTM_OBJ),userP) ;
 if( headerElmP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Header Partition Memory Allocation Failure") ;
    goto errexit ; 
   } 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Dtm Element From Dtm Object %p Completed",dtmP) ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Dtm Element From Dtm Object %p Error",dtmP) ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
#endif
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_testForInMemoryDtmObject(BC_DTM_OBJ *dtmP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long node,priorNode,nodeFound,nodeLevel ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Testing For In Memory DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
   }
/*
** Check For None Null Object
*/
 if (dtmP == NULL) goto errexit;
     {
     std::lock_guard<std::mutex> lock (s_dtmMutex);
     /*
**  Check B Tree Exits
*/
     if (glbDtmObjBtreeP == NULL) goto errexit;
     /*
     **  Find Entry For Dtm Object In Btree
     */
     if (bcdtmBtree_findNode (glbDtmObjBtreeP, dtmP, &node, &priorNode, &nodeFound, &nodeLevel)) goto errexit;
     }
/*
**  Check Node Found
*/
 if( ! nodeFound ) ret = DTM_ERROR ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For In Memory DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For In Memory DTM Object Error") ;
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
BENTLEYDTM_EXPORT int bcdtmObject_testForValidDtmObject(BC_DTM_OBJ *dtmP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long node,priorNode,nodeFound,nodeLevel ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Testing For Valid DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    if( dtmP != NULL )
      { 
       if     ( dtmP->dtmObjType == BC_DTM_OBJ_TYPE ) bcdtmWrite_message(0,0,0,"Object Type = BC_DTM_OBJ_TYPE") ;
       else if( dtmP->dtmObjType == BC_DTM_ELM_TYPE ) bcdtmWrite_message(0,0,0,"Object Type = BC_DTM_ELM_TYPE") ;
       else                                           bcdtmWrite_message(0,0,0,"Object Type = Unknown") ;
      }
   }
/*
** Check For None Null Object
*/
 if( dtmP == NULL ) goto errexit ;
/*
** Check For DTM Object Or DTM Element
*/
 if (dtmP->dtmObjType != BC_DTM_OBJ_TYPE && dtmP->dtmObjType != BC_DTM_ELM_TYPE) goto errexit;
 /*
 **  Check B Tree Exits
 */
     {
     std::lock_guard<std::mutex> lock (s_dtmMutex);
     if (glbDtmObjBtreeP == NULL) goto errexit;
     /*
     **  Find Entry For Dtm Object In Btree
     */
     if (bcdtmBtree_findNode (glbDtmObjBtreeP, dtmP, &node, &priorNode, &nodeFound, &nodeLevel)) goto errexit;
     }
/*
**  Check Node Found
*/
 if( ! nodeFound ) ret = DTM_ERROR ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"BC DTM Dtm Object %p Valid",dtmP) ;
 if( ret != DTM_SUCCESS ) bcdtmWrite_message(2,0,0,"Invalid BC DTM Dtm Object %p",dtmP) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_initialiseVer200DtmObject( BC_DTM_OBJ_VER_200 *dtmP )
/*
** This Function Initializes A Dtm Object 
*/
{
 int  ret=DTM_SUCCESS ;
 long n,dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject((BC_DTM_OBJ *)dtmP)) goto errexit ;
/*
** Free Feature Memory
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP((BC_DTM_OBJ *)dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
      {
       if( dtmFeatureP->numDtmFeaturePts > 0 )
         {
          free(bcdtmMemory_getPointerP3D((BC_DTM_OBJ *)dtmP,dtmFeatureP->dtmFeaturePts.pointsPI)) ;
         }
      } 
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
      {
       if( dtmFeatureP->numDtmFeaturePts > 0 )
         {
          if( dtmFeatureP->dtmFeaturePts.pointsPI != 0)
            {
             bcdtmMemory_free((BC_DTM_OBJ *)dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
             dtmFeatureP->dtmFeaturePts.pointsPI = 0;
            }
         }     
      }
   }
/*
** Free Partition Memory And Partition Arrays
*/
 if( dtmP->fTablePP != NULL ) 
   { 
    for( n = 0 ; n < dtmP->numFeaturePartitions ; ++n ) free(dtmP->fTablePP[n]) ;
    free(dtmP->fTablePP) ; 
    dtmP->fTablePP = NULL ; 
   }
 if( dtmP->pointsPP != NULL ) 
   { 
    for( n = 0 ; n < dtmP->numPointPartitions   ; ++n ) free(dtmP->pointsPP[n]) ;
    free(dtmP->pointsPP) ;
    dtmP->pointsPP = NULL ; 
   }
 if( dtmP->nodesPP != NULL ) 
   { 
    for( n = 0 ; n < dtmP->numNodePartitions   ; ++n ) free(dtmP->nodesPP[n]) ;
    free(dtmP->nodesPP) ;
    dtmP->nodesPP = NULL ; 
   }
 if( dtmP->cListPP != NULL ) 
   { 
    for( n = 0 ; n < dtmP->numClistPartitions   ; ++n ) free(dtmP->cListPP[n]) ;
    free(dtmP->cListPP) ;
    dtmP->cListPP = NULL ; 
   }
 if( dtmP->fListPP != NULL ) 
   { 
    for( n = 0 ; n < dtmP->numFlistPartitions   ; ++n ) free(dtmP->fListPP[n]) ;
    free(dtmP->fListPP) ;
    dtmP->fListPP = NULL ; 
   }
/*
** Initialize Dtm Object
*/ 
 dtmP->dtmObjType           =  BC_DTM_OBJ_TYPE  ;
 dtmP->dtmObjVersion        =  BC_DTM_OBJ_VERSION ;
 dtmP->numLines             =  0 ;
 dtmP->numTriangles         =  0 ;
 dtmP->numFeatures          =  0 ;
 dtmP->memFeatures          =  0 ;
 dtmP->iniFeatures          =  BC_DTM_INI_FEATURES ;
 dtmP->incFeatures          =  BC_DTM_INC_FEATURES ;
 dtmP->numFeaturePartitions =  0 ;
 dtmP->featurePartitionSize =  DTM_PARTITION_SIZE_FEATURE ;
 dtmP->numPoints            =  0 ;
 dtmP->memPoints            =  0 ;
 dtmP->numSortedPoints      =  0 ;
 dtmP->iniPoints            =  BC_DTM_INI_POINTS ;
 dtmP->incPoints            =  BC_DTM_INC_POINTS ;
 dtmP->numPointPartitions   =  0 ;
 dtmP->pointPartitionSize   =  DTM_PARTITION_SIZE_POINT ;
 dtmP->numNodes             =  0 ;
 dtmP->memNodes             =  0 ;
 dtmP->numNodePartitions    =  0 ;
 dtmP->nodePartitionSize    =  DTM_PARTITION_SIZE_NODE ;
 dtmP->numClist             =  0 ;
 dtmP->memClist             =  0 ;
 dtmP->cListPtr             =  0 ;
 dtmP->numClistPartitions   =  0 ;
 dtmP->clistPartitionSize   =  DTM_PARTITION_SIZE_CLIST ;
 dtmP->numFlist             =  0 ;
 dtmP->memFlist             =  0 ;
 dtmP->iniFlist             =  BC_DTM_INI_FLIST ;
 dtmP->incFlist             =  BC_DTM_INC_FLIST ;
 dtmP->numFlistPartitions   =  0 ;
 dtmP->flistPartitionSize   =  DTM_PARTITION_SIZE_FLIST ;
 dtmP->dtmState             =  DTMState::Data ;
 dtmP->nullPnt              =  DTM_NULL_PNT ;
 dtmP->nullPtr              =  DTM_NULL_PTR ;
 dtmP->dtmFeatureIndex      =  0 ;
 dtmP->nullUserTag          =  DTM_NULL_USER_TAG ;
 dtmP->nullFeatureId        =  DTM_NULL_FEATURE_ID ;
 dtmP->edgeOption           =  2  ;
 dtmP->cListPtr             =  0  ;
 dtmP->cListDelPtr          =  DTM_NULL_PTR ;
 dtmP->fListDelPtr          =  DTM_NULL_PTR ;
 dtmP->refCount             =  0 ;
 dtmP->userStatus           =  0 ;
 dtmP->creationTime         =  0 ;
 dtmP->modifiedTime         =  0 ;
 dtmP->userTime             =  0 ;
 dtmP->ppTol                = DTM_PPTOL ;
 dtmP->plTol                = DTM_PLTOL ; 
 dtmP->maxSide              = 1000.0 ; 
 dtmP->mppTol               = 0.0 ; 
 dtmP->transMatrix[0][0]    = 1.0 ; 
 dtmP->transMatrix[0][1]    = 0.0 ; 
 dtmP->transMatrix[0][2]    = 0.0 ; 
 dtmP->transMatrix[0][3]    = 0.0 ; 
 dtmP->transMatrix[1][0]    = 0.0 ; 
 dtmP->transMatrix[1][1]    = 1.0 ; 
 dtmP->transMatrix[1][2]    = 0.0 ; 
 dtmP->transMatrix[1][3]    = 0.0 ; 
 dtmP->transMatrix[2][0]    = 0.0 ; 
 dtmP->transMatrix[2][1]    = 0.0 ; 
 dtmP->transMatrix[2][2]    = 1.0 ; 
 dtmP->transMatrix[2][3]    = 0.0 ; 
 dtmP->transMatrix[3][0]    = 0.0 ; 
 dtmP->transMatrix[3][1]    = 0.0 ; 
 dtmP->transMatrix[3][2]    = 0.0 ; 
 dtmP->transMatrix[3][3]    = 1.0 ; 
 dtmP->xMin                 = 0.0 ;
 dtmP->yMin                 = 0.0 ;
 dtmP->zMin                 = 0.0 ;
 dtmP->xMax                 = 0.0 ;
 dtmP->yMax                 = 0.0 ;
 dtmP->zMax                 = 0.0 ;
 dtmP->xRange               = 0.0 ;
 dtmP->yRange               = 0.0 ;
 dtmP->zRange               = 0.0 ;
 dtmP->fTablePP             = NULL ;
 dtmP->pointsPP             = NULL ;
 dtmP->nodesPP              = NULL ;
 dtmP->cListPP              = NULL ;
 dtmP->fListPP              = NULL ;
 dtmP->DTMAllocationClass   = NULL ;
/*
** Clean Up
*/
 cleanup :
/*
**  Job Completed
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
BENTLEYDTM_EXPORT int bcdtmObject_initialiseDtmObject( BC_DTM_OBJ *dtmP )
/*
** This Function Initializes A Dtm Object 
*/
{
 int  ret=DTM_SUCCESS ;
 long n,dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Free Feature Memory
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
      {
       if( dtmFeatureP->numDtmFeaturePts > 0 )
         {
          free(bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI)) ;
         }
      } 
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
      {
       if( dtmFeatureP->numDtmFeaturePts > 0 )
         {
          if( dtmFeatureP->dtmFeaturePts.pointsPI != 0)
            {
             bcdtmMemory_free(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
             dtmFeatureP->dtmFeaturePts.pointsPI = 0;
            }
         }     
      }
   }
/*
** Free Extended Section
*/
 if( dtmP->extended != NULL )
   bcdtmObject_destoryDTMExtended(&dtmP->extended );

 dtmP->ClearUpAppData ();

/*
** Free Partition Memory And Partition Arrays
*/
 if( dtmP->fTablePP != NULL ) 
   { 
    for( n = 0 ; n < dtmP->numFeaturePartitions ; ++n ) free(dtmP->fTablePP[n]) ;
    free(dtmP->fTablePP) ; 
    dtmP->fTablePP = NULL ; 
   }
 if( dtmP->pointsPP != NULL ) 
   { 
    for( n = 0 ; n < dtmP->numPointPartitions   ; ++n ) free(dtmP->pointsPP[n]) ;
    free(dtmP->pointsPP) ;
    dtmP->pointsPP = NULL ; 
   }
 if( dtmP->nodesPP != NULL ) 
   { 
    for( n = 0 ; n < dtmP->numNodePartitions   ; ++n ) free(dtmP->nodesPP[n]) ;
    free(dtmP->nodesPP) ;
    dtmP->nodesPP = NULL ; 
   }
 if( dtmP->cListPP != NULL ) 
   { 
    for( n = 0 ; n < dtmP->numClistPartitions   ; ++n ) free(dtmP->cListPP[n]) ;
    free(dtmP->cListPP) ;
    dtmP->cListPP = NULL ; 
   }
 if( dtmP->fListPP != NULL ) 
   { 
    for( n = 0 ; n < dtmP->numFlistPartitions   ; ++n ) free(dtmP->fListPP[n]) ;
    free(dtmP->fListPP) ;
    dtmP->fListPP = NULL ; 
   }
/*
** Initialize Dtm Object
*/ 
 memset (dtmP, 0, BCDTMSize);
 dtmP->dtmObjType           =  BC_DTM_OBJ_TYPE  ;
 dtmP->dtmObjVersion        =  BC_DTM_OBJ_VERSION ;
 dtmP->numLines             =  0 ;
 dtmP->numTriangles         =  0 ;
 dtmP->numFeatures          =  0 ;
 dtmP->memFeatures          =  0 ;
 dtmP->iniFeatures          =  BC_DTM_INI_FEATURES ;
 dtmP->incFeatures          =  BC_DTM_INC_FEATURES ;
 dtmP->numFeaturePartitions =  0 ;
 dtmP->featurePartitionSize =  DTM_PARTITION_SIZE_FEATURE ;
 dtmP->numPoints            =  0 ;
 dtmP->memPoints            =  0 ;
 dtmP->numSortedPoints      =  0 ;
 dtmP->iniPoints            =  BC_DTM_INI_POINTS ;
 dtmP->incPoints            =  BC_DTM_INC_POINTS ;
 dtmP->numPointPartitions   =  0 ;
 dtmP->pointPartitionSize   =  DTM_PARTITION_SIZE_POINT ;
 dtmP->numNodes             =  0 ;
 dtmP->memNodes             =  0 ;
 dtmP->numNodePartitions    =  0 ;
 dtmP->nodePartitionSize    =  DTM_PARTITION_SIZE_NODE ;
 dtmP->numClist             =  0 ;
 dtmP->memClist             =  0 ;
 dtmP->cListPtr             =  0 ;
 dtmP->numClistPartitions   =  0 ;
 dtmP->clistPartitionSize   =  DTM_PARTITION_SIZE_CLIST ;
 dtmP->numFlist             =  0 ;
 dtmP->memFlist             =  0 ;
 dtmP->iniFlist             =  BC_DTM_INI_FLIST ;
 dtmP->incFlist             =  BC_DTM_INC_FLIST ;
 dtmP->numFlistPartitions   =  0 ;
 dtmP->flistPartitionSize   =  DTM_PARTITION_SIZE_FLIST ;
 dtmP->dtmState             =  DTMState::Data ;
 dtmP->dtmCleanUp           =  DTMCleanupFlags::None ;
 dtmP->obsolete_dtmRestoreTriangles = 0;
 dtmP->nullPnt              =  DTM_NULL_PNT ;
 dtmP->nullPtr              =  DTM_NULL_PTR ;
 dtmP->dtmFeatureIndex      =  0 ;
 dtmP->nullUserTag          =  DTM_NULL_USER_TAG ;
 dtmP->nullFeatureId        =  DTM_NULL_FEATURE_ID ;
 dtmP->edgeOption           =  2  ;
 dtmP->cListPtr             =  0  ;
 dtmP->cListDelPtr          =  DTM_NULL_PTR ;
 dtmP->fListDelPtr          =  DTM_NULL_PTR ;
 dtmP->refCount             =  0 ;
 dtmP->userStatus           =  0 ;
 dtmP->creationTime         =  0 ;
 dtmP->modifiedTime         =  0 ;
 dtmP->userTime             =  0 ;
#ifndef _WIN32_WCE 
 _time32(&dtmP->creationTime) ;
#endif 
 dtmP->ppTol                = DTM_PPTOL ;
 dtmP->plTol                = DTM_PLTOL ; 
 dtmP->maxSide              = 1000.0 ; 
 dtmP->mppTol               = 0.0 ; 
 dtmP->xMin                 = 0.0 ;
 dtmP->yMin                 = 0.0 ;
 dtmP->zMin                 = 0.0 ;
 dtmP->xMax                 = 0.0 ;
 dtmP->yMax                 = 0.0 ;
 dtmP->zMax                 = 0.0 ;
 dtmP->xRange               = 0.0 ;
 dtmP->yRange               = 0.0 ;
 dtmP->zRange               = 0.0 ;
 dtmP->fTablePP             = NULL ;
 dtmP->pointsPP             = NULL ;
 dtmP->nodesPP              = NULL ;
 dtmP->cListPP              = NULL ;
 dtmP->fListPP              = NULL ;
 dtmP->DTMAllocationClass   = NULL ;
 dtmP->extended             = NULL ; 
 bcdtmObject_updateLastModifiedTime (dtmP) ;

/*
** Clean Up
*/
 cleanup :
/*
**  Job Completed
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
BENTLEYDTM_EXPORT int bcdtmObject_destroyDtmObject(BC_DTM_OBJ **dtmPP)
/*
** This Function Destroys A Dtm Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,node,priorNode,nodeFound,nodeLevel,offset,feature,partitionNum;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Destroying DTM Object") ;
    bcdtmWrite_message(0,0,0,"*dtmPP               = %p",*dtmPP) ;
    if( dbg == 2 )
      { 
       bcdtmWrite_message(0,0,0,"(*dtmPP)->dtmObjType = %8ld",(*dtmPP)->dtmObjType) ;
       if( *dtmPP != NULL )
         { 
          bcdtmWrite_message(0,0,0,"(*dtmPP)-refCount    =  %8ld",(*dtmPP)->refCount) ;
          if     ( (*dtmPP)->dtmObjType == BC_DTM_OBJ_TYPE ) bcdtmWrite_message(0,0,0,"(*dtmPP)->dtmObjType = BC_DTM_OBJ_TYPE") ;
          else if( (*dtmPP)->dtmObjType == BC_DTM_ELM_TYPE ) bcdtmWrite_message(0,0,0,"(*dtmPP)->dtmObjType = BC_DTM_ELM_TYPE") ;
          else                                               bcdtmWrite_message(0,0,0,"(*dtmPP)->dtmObjType = Unknown[%4x]",(*dtmPP)->dtmObjType) ;
         }
      }   
   }
/*
** Check For Valid Dtm Object
*/
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if (bcdtmObject_testForValidDtmObject (*dtmPP)) goto errexit;
 /*
 ** Check If Current DTM Object Is The Same Object
 */
 /*
  if( *dtmPP == DTM_CDTM )
  {
  if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Dtm Object Is Current Dtm Object") ;
  currentDtmObj = TRUE ;
  }
  */
 /*
 ** Find Entry For Dtm Object In Btree
 */
     {
     std::lock_guard<std::mutex> lock (s_dtmMutex);
     if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Finding Btree Entry For Dtm Object");
     if (bcdtmBtree_findNode (glbDtmObjBtreeP, *dtmPP, &node, &priorNode, &nodeFound, &nodeLevel)) goto errexit;
     if (dbg == 2) bcdtmWrite_message (0, 0, 0, "nodeFound = %2ld node = %6ld", nodeFound, node);
     }
/*
** Tell bcMemory that we are being freed
*/
 bcdtmMemory_release (*dtmPP);
/*
** Scan Features And Free Any Feature Point Memory
*/
    if( (*dtmPP)->fTablePP != NULL && (*dtmPP)->numFeatures > 0 ) 
    { 
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Freeing Feature Point Memory") ;
    offset       = 0 ;
    partitionNum = 0 ;
    dtmFeatureP  = (*dtmPP)->fTablePP[partitionNum]  ;
    for( feature = 0 ; feature < (*dtmPP)->numFeatures ; ++feature )
        {
        if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Freeing Feature %6ld Points Memory",feature) ;
        if((*dtmPP)->dtmObjType != BC_DTM_ELM_TYPE)
            {
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray  && dtmFeatureP->dtmFeaturePts.pointsPI != 0 ) bcdtmMemory_free(*dtmPP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray && dtmFeatureP->dtmFeaturePts.offsetPI != 0 ) bcdtmMemory_free(*dtmPP,dtmFeatureP->dtmFeaturePts.offsetPI) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError     && dtmFeatureP->dtmFeaturePts.pointsPI != 0 ) bcdtmMemory_free(*dtmPP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
            if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback     && dtmFeatureP->dtmFeaturePts.pointsPI != 0 ) bcdtmMemory_free (*dtmPP, dtmFeatureP->dtmFeaturePts.pointsPI);
            }
        ++offset ;
        if( offset == (*dtmPP)->featurePartitionSize ) 
            {
            offset = 0 ;
            ++partitionNum ;
            dtmFeatureP = (*dtmPP)->fTablePP[partitionNum]   ;
            }
        else ++dtmFeatureP ; 
        }
    }
/*
** Free Partition Memory And Partition Arrays
*/
    if( (*dtmPP)->fTablePP != NULL ) 
    { 
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Freeing Feature Table Memory") ;
    if((*dtmPP)->dtmObjType != BC_DTM_ELM_TYPE)    for( n = 0 ; n < (*dtmPP)->numFeaturePartitions ; ++n ) bcdtmMemory_freePartition((*dtmPP), DTMPartition::Feature, n, (*dtmPP)->fTablePP[n]) ;
    free((*dtmPP)->fTablePP) ; 
    (*dtmPP)->fTablePP = NULL ; 
    }
    if( (*dtmPP)->pointsPP != NULL ) 
    { 
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Freeing Points Memory") ;
    if((*dtmPP)->dtmObjType != BC_DTM_ELM_TYPE)    for( n = 0 ; n < (*dtmPP)->numPointPartitions   ; ++n ) bcdtmMemory_freePartition((*dtmPP), DTMPartition::Point, n, (*dtmPP)->pointsPP[n]) ;
    free((*dtmPP)->pointsPP) ;
    (*dtmPP)->pointsPP = NULL ; 
    }
    if( (*dtmPP)->nodesPP != NULL ) 
    { 
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Freeing Nodes Memory") ;
    if((*dtmPP)->dtmObjType != BC_DTM_ELM_TYPE)    for( n = 0 ; n < (*dtmPP)->numNodePartitions ; ++n ) bcdtmMemory_freePartition((*dtmPP), DTMPartition::Node, n, (*dtmPP)->nodesPP[n]) ;
    free((*dtmPP)->nodesPP) ;
    (*dtmPP)->nodesPP = NULL ; 
    }
    if( (*dtmPP)->cListPP != NULL ) 
    { 
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Freeing Circular List Memory") ;
    if((*dtmPP)->dtmObjType != BC_DTM_ELM_TYPE)    for( n = 0 ; n < (*dtmPP)->numClistPartitions ; ++n ) bcdtmMemory_freePartition((*dtmPP), DTMPartition::CList, n, (*dtmPP)->cListPP[n]) ;
    free((*dtmPP)->cListPP) ;
    (*dtmPP)->cListPP = NULL ; 
    }
    if( (*dtmPP)->fListPP != NULL ) 
    { 
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Freeing Feature List Memory") ;
    if((*dtmPP)->dtmObjType != BC_DTM_ELM_TYPE) for( n = 0 ; n < (*dtmPP)->numFlistPartitions ; ++n ) bcdtmMemory_freePartition((*dtmPP), DTMPartition::FList, n, (*dtmPP)->fListPP[n]) ;
    free((*dtmPP)->fListPP) ;
    (*dtmPP)->fListPP = NULL ; 
    }
/*
** Destroy Extended DTM
*/   
  if ((*dtmPP)->extended != NULL )
      bcdtmObject_destoryDTMExtended( &(*dtmPP)->extended );
  (*dtmPP)->ClearUpAppData ();
/*
** Free Dtm Object
*/
 delete *dtmPP;
 *dtmPP = NULL ;
/*
** Null Current Dtm Object
*/
/*
 if( currentDtmObj == TRUE )
   {
    DTM_CDTM = NULL ;
    wcscpy(DTM_CDTM_FILE,L"") ; 
   }
*/   
/*
** Remove Entry For Dtm Object In Btree
*/
 if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Removing Node For Dtm Object From Btree");
     {
     std::lock_guard<std::mutex> lock (s_dtmMutex);
     if (bcdtmBtree_removeNode (glbDtmObjBtreeP, node)) goto errexit;
     }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Destroying DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Destroying DTM Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 bcdtmWrite_message(0,0,0,"Destroying DTM Object Error") ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_destroyAllDtmObjects(void)
/*
** This Function Destroys All Dtm Objects
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,numNodeValues ;
 BC_DTM_OBJ *dtmP=NULL,**nodeDtmPP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Destroying All Dtm Objects") ;
/*
** Check DTM Objects Exist
*/
 if( glbDtmObjBtreeP != NULL )
   {
/*
**  Get Array Of Dtm Objects
*/
    if( bcdtmBtree_getArrayOfNodeValues(glbDtmObjBtreeP,&nodeDtmPP,&numNodeValues)) goto errexit ;
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Number Of Dtm Objects = %8ld",numNodeValues) ;
       for( n = 0 ; n < numNodeValues ; ++n )
         {
          dtmP = *(nodeDtmPP+n) ;
          bcdtmWrite_message(0,0,0,"Dtm Object[%6ld] = %p",n,dtmP) ;
         }
      }   
/*
**  RobC - 17Jan12 - Changes For A DTM Object In The Extended Component Being Deleted     
*/
    for( n = 0 ; n < numNodeValues ; ++n )
      {
       dtmP =  *(nodeDtmPP+n) ;
       if( dtmP->extended != NULL )
         {
          bcdtmObject_destoryDTMExtended (&dtmP->extended);
          dtmP->extended = NULL ;  
         }
       dtmP->ClearUpAppData ();
      }
/*
**  Get list Of Remaining DTM Objects
*/      
    if( bcdtmBtree_getArrayOfNodeValues(glbDtmObjBtreeP,&nodeDtmPP,&numNodeValues)) goto errexit ;
/*
**  Delete Remaining Objects
*/    
    for( n = 0 ; n < numNodeValues ; ++n )
      {
       dtmP =  *(nodeDtmPP+n) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Destroying Dtm Object[%6ld] = %p",n,dtmP) ;
       if( bcdtmObject_destroyDtmObject(&dtmP)) goto errexit ;
      } 
  }
/*
** Clean Up
*/
 cleanup :
 if( nodeDtmPP != NULL ) free(nodeDtmPP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Destroying All Dtm Objects Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Destroying All Dtm Objects Error") ;
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
BENTLEYDTM_EXPORT int bcdtmObject_reportActiveDtmObjects( long reportOption, long *numActiveObjectsP , unsigned long *totMemoryUsageP )
/*
** This Function Counts The Number Of Active Dtm Objects
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,numNodeValues=0 ;
 unsigned long memUsage=0,totMemUsage=0 ;
 BC_DTM_OBJ *dtmP=NULL,**nodeDtmPP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reporting Active DTM Objects") ;
/*
** Initialise
*/
 *numActiveObjectsP = 0 ;
 *totMemoryUsageP   = 0 ;
/*
** Check DTM Objects Exist
*/
 if( glbDtmObjBtreeP != NULL )
   {
/*
**  Get Array Of Dtm Objects
*/
    if( bcdtmBtree_getArrayOfNodeValues(glbDtmObjBtreeP,&nodeDtmPP,&numNodeValues)) goto errexit ;
/*
**  Scan Dtm Object Array List And Report Object Usage
*/
    if( reportOption ) bcdtmWrite_message(0,0,0,"Number Of Dtm Objects = %8ld",numNodeValues) ;
    for( n = 0 ; n < numNodeValues ; ++n )
      {
       dtmP = *(nodeDtmPP+n) ;
       memUsage = sizeof(BC_DTM_OBJ) ;
       if( dtmP->pointsPP != NULL ) memUsage = memUsage + dtmP->memPoints   * sizeof(DTM_TIN_POINT) ;
       if( dtmP->nodesPP  != NULL ) memUsage = memUsage + dtmP->memPoints   * sizeof(DTM_TIN_NODE) ;
       if( dtmP->cListPP  != NULL ) memUsage = memUsage + dtmP->memClist    * sizeof(DTM_CIR_LIST) ;
       if( dtmP->fListPP  != NULL ) memUsage = memUsage + dtmP->memFlist    * sizeof(DTM_FEATURE_LIST) ;
       if( dtmP->fTablePP != NULL ) memUsage = memUsage + dtmP->memFeatures * sizeof(BC_DTM_FEATURE) ;
       if( reportOption ) bcdtmWrite_message(0,0,0,"dtmObject[%4ld] ** dtmP = %p ** dtmState = %2ld refCount = %4ld ** memUsage = %10ld",n,dtmP,dtmP->dtmState,dtmP->refCount,memUsage) ;
       totMemUsage = totMemUsage + memUsage ;
      }
/*
**  Set Return Values
*/
    *numActiveObjectsP = numNodeValues ;
    *totMemoryUsageP   = totMemUsage   ;
    if( reportOption ) 
      {
       bcdtmWrite_message(0,0,0,"Number Of Active Dtm Objects = %8ld ** Total Memory Usage = %10ld",numNodeValues,totMemUsage) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( nodeDtmPP != NULL ) free(nodeDtmPP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reporting Active DTM Objects Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reporting Active DTM Objects Error") ;
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
BENTLEYDTM_EXPORT int  bcdtmObject_storeDtmFeatureInDtmObject
(
 BC_DTM_OBJ *dtmP,               /* ==> Pointer To Dtm Dtm Object      */
 DTMFeatureType dtmFeatureType,          /* ==> Dtm Feature Type To Be Stored  */
 DTMUserTag  userTag,          /* ==> User Tag                       */
 long     indexOption,           /* ==> Index Option                   */
 DTMFeatureId *userFeatureIdP, /* ==> Feature Id                     */
 DPoint3dCP   featurePtsP,          /* ==> Feature Points                 */
 long     numFeaturePts          /* ==> Number Of Feature Points       */
)
/*
** This Function Stores A DTM Feature In A BC Dtm Object
**
** indexOption == 1  Store Null Feature Index( nullFeatureId ) as userFeatureIdP
**             == 2  Store Feature Index userFeatureIdP as is
**             == 3  Set And Store Feature Index From Dtm Feature Index dtmP->dtmFeatureIndex
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long saveIniPts,saveIncPts,dtmFeature ;
 DPoint3dCP p3dP ;
 char dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE *dtmFeatureP=NULL ;
 DTM_TIN_POINT *pointP ;
/*
** Write Entry Message
*/
// if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
// bcdtmWrite_message(0,0,0,"Storing Dtm Feature %s In Dtm Object %p ** numFeaturePts  = %8ld userTag = %10I64d** dtmP->numPoints = %8ld",dtmFeatureTypeName,dtmP,numFeaturePts,userTag,dtmP->numPoints) ;
 if( dbg )
   {
    if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Storing Dtm Feature In Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld ** %s",dtmFeatureType,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"userTag        = %8I64d",userTag) ;
    bcdtmWrite_message(0,0,0,"userFeatureId  = %8I64d",*userFeatureIdP) ;
    bcdtmWrite_message(0,0,0,"featurePtsP    = %p",featurePtsP) ;
    bcdtmWrite_message(0,0,0,"numFeaturePts  = %8ld",numFeaturePts) ;
    if( dbg == 2 )
      {
       for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP) 
         {
          bcdtmWrite_message(0,0,0,"Feature Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }   
   }
/*
** Check For Valid DTM Store Feature Type
*/
 if( bcdtmData_testForValidDtmObjectImportFeatureType(dtmFeatureType))
   {
    bcdtmWrite_message(2,0,0,"Invalid Import DTM Feature Type") ;
    goto errexit ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check For Valid DTM Add States
*/
 if( dtmP->dtmState != DTMState::Data && dtmP->dtmState != DTMState::Tin ) 
   {
    bcdtmWrite_message(2,0,0,"Invalid DTM State For Storing Features") ;
    goto errexit ;
   } 
/*
** If A Hull Feature Is Being Stored Check That A Hull Feature Doesn't Already Exist In The DTM
*/
 if( dtmFeatureType == DTMFeatureType::Hull || dtmFeatureType == DTMFeatureType::DrapeHull )
   {
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull || dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeHull ) && ( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin ))
         {
          bcdtmWrite_message(1,0,0,"Hull Feature Already Exists In DTM") ;
          goto errexit ; 
         }
      }
   }
/*
** Check Feature Index Option
*/
 if( indexOption < 1 || indexOption > 3 )
   {
    bcdtmWrite_message(2,0,0,"Invalid Dtm Feature Index Option") ;
    goto errexit ;
   }
/*
** Set User Tags And Feature Ids To Null For DTMFeatureType::RandomSpots
*/
 if( dtmFeatureType == DTMFeatureType::RandomSpots )
   {
    userTag  = DTM_NULL_USER_TAG ;
    *userFeatureIdP = DTM_NULL_FEATURE_ID   ;
    indexOption = 1 ;
   }
/*
** Check Features Memory
*/
 if( dtmP->numFeatures == dtmP->memFeatures ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Features Array") ;
    if( bcdtmObject_allocateFeaturesMemoryDtmObject(dtmP)) goto errexit  ;
   }
/*
** Switch Depending On DTM State
*/
 switch ( dtmP->dtmState )
   {
    case  DTMState::Data :
    if( dbg ) bcdtmWrite_message(0,0,0,"DTM In Data State") ;
/*
**  Check Points Memory
*/
    if( dtmFeatureType != DTMFeatureType::DrapeVoid && dtmFeatureType != DTMFeatureType::DrapeHull )
      {
       if( dtmP->numPoints + numFeaturePts > dtmP->memPoints ) 
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Points Array") ;
           saveIniPts = dtmP->iniPoints ;
          saveIncPts = dtmP->incPoints ;
          if( dtmP->memPoints == 0 && dtmP->iniPoints < numFeaturePts ) dtmP->iniPoints = numFeaturePts ;
          if( dtmP->memPoints >  0 && dtmP->memPoints + dtmP->incPoints - dtmP->numPoints < numFeaturePts ) dtmP->incPoints = numFeaturePts  ;
          if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit  ;
          dtmP->iniPoints = saveIniPts ;
          dtmP->incPoints = saveIncPts ;
         }
      }
/*
**  Store Feature In BC Dtm Object
*/
    if( dtmFeatureType != DTMFeatureType::RandomSpots )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Storing Dtm Feature In Features Array") ;
/*
**     Determine Next Feature Address
*/
       dtmFeatureP  = ftableAddrP(dtmP,dtmP->numFeatures)  ;
/*
**     Store Feature
*/
       if( dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureType == DTMFeatureType::DrapeHull )
         {
          dtmFeatureP->dtmFeatureState           =  DTMFeatureState::PointsArray ;
          dtmFeatureP->dtmFeatureType            =  (DTMFeatureType)dtmFeatureType   ;
          dtmFeatureP->dtmUserTag                =  userTag          ;
          dtmFeatureP->internalToDtmFeature      =  dtmP->nullPnt;
          dtmFeatureP->dtmFeaturePts._64bitPad   =  0;
          if( indexOption == 1 ) dtmFeatureP->dtmFeatureId = DTM_NULL_FEATURE_ID ;
          if( indexOption == 2 ) dtmFeatureP->dtmFeatureId = *userFeatureIdP ;
          if( indexOption == 3 ) 
            {
             *userFeatureIdP = dtmFeatureP->dtmFeatureId = dtmP->dtmFeatureIndex ;
             ++dtmP->dtmFeatureIndex ;
            }
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
          dtmFeatureP->numDtmFeaturePts =  numFeaturePts    ;
          ++dtmP->numFeatures ;
         }
       else
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Storing Feature Header") ;
          dtmFeatureP->dtmFeatureState           =  DTMFeatureState::Data ;
          dtmFeatureP->dtmFeatureType            =  (DTMFeatureType)dtmFeatureType   ;
          dtmFeatureP->internalToDtmFeature      =  dtmP->nullPnt;
          dtmFeatureP->dtmFeaturePts._64bitPad   =  0;
          dtmFeatureP->dtmUserTag                =  userTag          ;
          dtmFeatureP->dtmFeaturePts.firstPoint  =  dtmP->numPoints  ;
          dtmFeatureP->numDtmFeaturePts          =  numFeaturePts    ;
          if( indexOption == 1 ) dtmFeatureP->dtmFeatureId = DTM_NULL_FEATURE_ID ;
          if( indexOption == 2 ) dtmFeatureP->dtmFeatureId = *userFeatureIdP ;
          if( indexOption == 3 ) 
            {
             *userFeatureIdP = dtmFeatureP->dtmFeatureId = dtmP->dtmFeatureIndex ;
             ++dtmP->dtmFeatureIndex ;
            }
          ++dtmP->numFeatures ;
         }
      }
/*
**  Add Points To Object
*/
    if( dtmFeatureType != DTMFeatureType::DrapeVoid && dtmFeatureType != DTMFeatureType::DrapeHull )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Storing Dtm Feature Points In Points Array") ;
       for( p3dP = featurePtsP ;  p3dP < featurePtsP + numFeaturePts ; ++p3dP )
         {
/*
**        Check Points Memory
*/
          if( dtmP->numPoints == dtmP->memPoints ) 
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Points Array") ;
             if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit  ;
            }
/*
**        Get Next Point Address
*/
          pointP = pointAddrP(dtmP,dtmP->numPoints) ;
/*
**        Set Bounding Cube For Dtm Object
*/
          if( dtmP->numPoints == 0 )
            {
             dtmP->xMin = dtmP->xMax = p3dP->x ;
             dtmP->yMin = dtmP->yMax = p3dP->y ;
             dtmP->zMin = dtmP->zMax = p3dP->z ;
            }
          else
            {
             if( p3dP->x < dtmP->xMin ) dtmP->xMin = p3dP->x ;
             if( p3dP->x > dtmP->xMax ) dtmP->xMax = p3dP->x ;
             if( p3dP->y < dtmP->yMin ) dtmP->yMin = p3dP->y ;
             if( p3dP->y > dtmP->yMax ) dtmP->yMax = p3dP->y ;
             if( p3dP->z < dtmP->zMin ) dtmP->zMin = p3dP->z ;
             if( p3dP->z > dtmP->zMax ) dtmP->zMax = p3dP->z ;
            }
/*
**        Store Point
*/
          pointP->x = p3dP->x ;
          pointP->y = p3dP->y ;
          pointP->z = p3dP->z ;
/*
**        Increment Number Of Points
*/
          ++dtmP->numPoints  ;
         }
      }
    break ;

    case  DTMState::Tin :

       if( dbg ) bcdtmWrite_message(0,0,0,"DTM In Tin State") ;
/*
**     Determine Next Feature Address
*/
       dtmFeatureP  = ftableAddrP(dtmP,dtmP->numFeatures)  ;
       dtmFeatureP->dtmFeatureState           =  DTMFeatureState::PointsArray ;
       dtmFeatureP->dtmFeatureType            =  (DTMFeatureType)dtmFeatureType   ;
       dtmFeatureP->dtmUserTag                =  userTag          ;
       dtmFeatureP->internalToDtmFeature      =  dtmP->nullPnt ;
       dtmFeatureP->dtmFeaturePts._64bitPad   =  0;
       if( indexOption == 1 ) dtmFeatureP->dtmFeatureId = DTM_NULL_FEATURE_ID ;
       if( indexOption == 2 ) dtmFeatureP->dtmFeatureId = *userFeatureIdP ;
       if( indexOption == 3 ) 
         {
          *userFeatureIdP = dtmFeatureP->dtmFeatureId = dtmP->dtmFeatureIndex ;
          ++dtmP->dtmFeatureIndex ;
         }
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
       dtmFeatureP->numDtmFeaturePts          =  numFeaturePts    ;
       ++dtmP->numFeatures ;

       if( dbg == 2 )
         {
          if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
          bcdtmWrite_message(0,0,0,"Added Feature Type %s featureId = %I64d ** %12.5lf %12.5lf %10.4lf",dtmFeatureTypeName,*userFeatureIdP,bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI)->x,bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI)->y,bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI)->z) ;
          for( p3dP = ( DPoint3d *) dtmFeatureP->dtmFeaturePts.pointsPI ; p3dP < ( DPoint3d *) dtmFeatureP->dtmFeaturePts.pointsPI + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
            {
             bcdtmWrite_message(0,0,0,"Feature Point[%8ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-(DPoint3d *)dtmFeatureP->dtmFeaturePts.pointsPI),p3dP->x,p3dP->y,p3dP->z) ;
            }
         }
       
    break ;

    default :
    break ;

   } ;
/*
**  Update Modified Time
 */
   bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Storing Dtm Feature In Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Storing Dtm Feature In Dtm Object Error") ;
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
BENTLEYDTM_EXPORT int bcdtmObject_setPointTolerancesDtmObject
(
 BC_DTM_OBJ *dtmP,
 double p2pTolerance,
 double p2lTolerance
)
/*
** This Function Sets The Point Tolerances For A Dtm Object
*/
{
 int ret=DTM_SUCCESS ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Validate
*/
 if( p2pTolerance <= 0.0 ) p2pTolerance = DTM_PPTOL ;
 if( p2lTolerance <= 0.0 ) p2lTolerance = DTM_PLTOL  ;
/*
** Set Memory Allocation Parameters
*/
 dtmP->ppTol = p2pTolerance ;
 dtmP->plTol = p2lTolerance ;
/*
** Clean Up
*/
 cleanup :
/*
**  Job Completed
*/
 return(DTM_SUCCESS) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_incReferenceCountDtmObject
(
 BC_DTM_OBJ *dtmP
)
/*
** This Function Increments The Reference Count For A Dtm Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Incrementing Ref Count For %p State = %2ld From %2ld To %2ld",dtmP,dtmP->dtmState,dtmP->refCount,dtmP->refCount+1) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Increment Reference Count
*/
 ++dtmP->refCount ;
/*
** Clean Up
*/
 cleanup :
/*
**  Job Completed
*/
 return(DTM_SUCCESS) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_decReferenceCountDtmObject
(
 BC_DTM_OBJ *dtmP
)
/*
** This Function Decrements The Reference Count For A Dtm Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Decrementing Ref Count For %p State = %2ld From %2ld To %2ld",dtmP,dtmP->dtmState,dtmP->refCount,dtmP->refCount-1) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Decrement Reference Count
*/
 if( dtmP->refCount > 0 ) --dtmP->refCount ;
/*
** Clean Up
*/
 cleanup :
/*
**  Job Completed
*/
 return(DTM_SUCCESS) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_setReferenceCountDtmObject
(
 BC_DTM_OBJ *dtmP ,
 long       count 
)
/*
** This Function Sets The Reference Count For A Dtm Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Ref Count For %p State = %2ld To %2ld",dtmP,dtmP->dtmState,count) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Set Reference Count
*/
  dtmP->refCount = count ;
/*
** Clean Up
*/
 cleanup :
/*
**  Job Completed
*/
 return(DTM_SUCCESS) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_getReferenceCountDtmObject
(
 BC_DTM_OBJ *dtmP ,
 long       *countP 
)
/*
** This Function Gets The Reference Count For A Dtm Object
*/
{
 int ret=DTM_SUCCESS ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Get Reference Count
*/
 *countP = dtmP->refCount ;
/*
** Clean Up
*/
 cleanup :
/*
**  Job Completed
*/
 return(DTM_SUCCESS) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_setFeatureMemoryAllocationParametersDtmObject
(
 BC_DTM_OBJ *dtmP,
 long iniFeatures,
 long incFeatures
)
/*
** This Function Sets The Feature Memory Allocation Parameters For A Dtm Object
*/
{
 int ret=DTM_SUCCESS ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Validate
*/
 if( iniFeatures < 1000 ) iniFeatures = 1000 ;
 if( incFeatures <  100 ) incFeatures = 100  ;
 if( incFeatures > iniFeatures ) incFeatures = iniFeatures ;
/*
** Set Memory Allocation Parameters
*/
 dtmP->iniFeatures = iniFeatures ;
 dtmP->incFeatures = incFeatures ;
/*
** Clean Up
*/
 cleanup :
/*
**  Job Completed
*/
 return(DTM_SUCCESS) ;
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
BENTLEYDTM_Public int  bcdtmObject_allocateFeaturesMemoryDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Allocates Memory For DTM Features
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,feature,numPartitions,remPartition,numCurPartitions,remCurPartition,partitionNum,partitionOfs,partitionSize ;
 BC_DTM_FEATURE *dtmFeatureP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Allocating Memory For Features Array") ;
    bcdtmWrite_message(0,0,0,"dtmP                       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFeatures          = %8ld",dtmP->numFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->memFeatures          = %8ld",dtmP->memFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->iniFeatures          = %8ld",dtmP->iniFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->incFeatures          = %8ld",dtmP->incFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->featurePartitionSize = %8ld",dtmP->featurePartitionSize) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFeaturePartitions = %8ld",dtmP->numFeaturePartitions) ;
   }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Initial Allocation
*/
 if( dtmP->memFeatures == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Initial Memory For Features Array") ;
    dtmP->memFeatures = dtmP->iniFeatures ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->memFeatures = %8ld",dtmP->memFeatures) ;
/*
**  Determine Number Of Feature Partitions
*/
    numPartitions = dtmP->memFeatures / dtmP->featurePartitionSize + 1 ; 
    remPartition  = dtmP->memFeatures % dtmP->featurePartitionSize ;
    if( remPartition == 0 ) { --numPartitions ; remPartition = dtmP->featurePartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPartitions = %8ld ** remPartition = %8ld",numPartitions,remPartition) ;
/*
**  Allocate Memory For Pointers To Feature Partitions
*/
    dtmP->numFeaturePartitions = numPartitions ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Feature Partition Pointers") ;
    dtmP->fTablePP  = ( BC_DTM_FEATURE  ** ) malloc( dtmP->numFeaturePartitions * sizeof( BC_DTM_FEATURE * )) ;
    if( dtmP->fTablePP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
    for( n = 0 ; n < dtmP->numFeaturePartitions ; ++n ) dtmP->fTablePP[n] = NULL ;
/*
**  Allocate Memory For Feature Partitions
*/
    for( n = 0 ; n < dtmP->numFeaturePartitions ; ++n )
      {
/*
**     Determine Partition Size
*/
       if( n < dtmP->numFeaturePartitions - 1 ) partitionSize = dtmP->featurePartitionSize ;
       else                                     partitionSize = remPartition ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
/*
**     Allocate Partition Memory
*/
       dtmP->fTablePP[n] = ( BC_DTM_FEATURE * ) bcdtmMemory_allocatePartition(dtmP, DTMPartition::Feature, n, partitionSize * sizeof( BC_DTM_FEATURE)) ;
       if( dtmP->fTablePP[n] == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld Completed",partitionSize,n) ;
      }
   }
/*
** Incremental Allocation
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Incremental Memory For Features Array") ;
/*
**  Determine Current Number Of Partitions
*/
    numCurPartitions  = dtmP->memFeatures / dtmP->featurePartitionSize + 1 ;
    remCurPartition   = dtmP->memFeatures % dtmP->featurePartitionSize ;
    if( remCurPartition == 0 ) { --numCurPartitions ; remCurPartition = dtmP->featurePartitionSize ; }
    if( numCurPartitions != dtmP->numFeaturePartitions )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"numCurPartitions = %6ld ** dtmP->numFeaturePartitions = %6ld",numCurPartitions,dtmP->numFeaturePartitions) ;
       bcdtmWrite_message(2,0,0,"Inconsistent Number Of Feature Partitions") ;
      } 
/*
**  Increment Number Of Memory Features
*/
    dtmP->memFeatures = dtmP->memFeatures + dtmP->incFeatures ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->memFeatures = %8ld",dtmP->memFeatures) ;
/*
**  Determine Required Number Of Partitions
*/
    numPartitions = dtmP->memFeatures / dtmP->featurePartitionSize + 1 ; 
    remPartition  = dtmP->memFeatures % dtmP->featurePartitionSize ;
    if( remPartition == 0 ) { --numPartitions ; remPartition = dtmP->featurePartitionSize ; }
    if( dbg ) 
      { 
       bcdtmWrite_message(0,0,0,"dtmP->numFeaturePartitions = %8ld",dtmP->numFeaturePartitions) ;
       bcdtmWrite_message(0,0,0,"numPartitions = %8ld remPartition = %8ld",numPartitions,remPartition) ;
      } 
/*
**  Memory Allocation Within Current Partition
*/
    if( numPartitions == dtmP->numFeaturePartitions )
      {
       dtmP->fTablePP[dtmP->numFeaturePartitions-1] = ( BC_DTM_FEATURE * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::Feature, dtmP->numFeaturePartitions - 1, dtmP->fTablePP[dtmP->numFeaturePartitions-1],remPartition*sizeof(BC_DTM_FEATURE)) ;      
       if( dtmP->fTablePP[dtmP->numFeaturePartitions-1] == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Memory Allocation Over A Number Of Partitions
*/
    if( numPartitions > dtmP->numFeaturePartitions )
      {
/*
**     Allocate Memory For Additional Feature Partitions
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Memory For Additional Feature Partition Pointers") ;
       dtmP->fTablePP  = ( BC_DTM_FEATURE  ** ) realloc(dtmP->fTablePP,numPartitions * sizeof( BC_DTM_FEATURE * )) ;
       if( dtmP->fTablePP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ; 
         }
       for( n = dtmP->numFeaturePartitions ; n < numPartitions ; ++n ) dtmP->fTablePP[n] = NULL ;
/*
**     Allocate Memory In Current Partition
*/
       if( remCurPartition < dtmP->featurePartitionSize )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Reallocating Memory Witin Current Partition") ;
          dtmP->fTablePP[dtmP->numFeaturePartitions-1] = ( BC_DTM_FEATURE * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::Feature, dtmP->numFeaturePartitions - 1, dtmP->fTablePP[dtmP->numFeaturePartitions-1],dtmP->featurePartitionSize*sizeof(BC_DTM_FEATURE)) ;      
          if( dtmP->fTablePP[dtmP->numFeaturePartitions-1] == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Allocate Memory For Feature Partitions
*/
       for( n = dtmP->numFeaturePartitions ; n < numPartitions ; ++n )
         {
/*
**        Determine Partition Size
*/
          if( n < numPartitions - 1 ) partitionSize = dtmP->featurePartitionSize ;
          else                        partitionSize = remPartition ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
/*
**        Allocate Partition Memory
*/
          dtmP->fTablePP[n] = ( BC_DTM_FEATURE * ) bcdtmMemory_allocatePartition(dtmP, DTMPartition::Feature, n, partitionSize * sizeof( BC_DTM_FEATURE)) ;
          if( dtmP->fTablePP[n] == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
      }
/*
**  Reset Number Of Partitions
*/
    dtmP->numFeaturePartitions = numPartitions ;
   }
/*
** Initialise Allocated Memory
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"Initialising Allocated Memory") ;
  partitionNum = dtmP->numFeatures / dtmP->featurePartitionSize ;
  partitionOfs = dtmP->numFeatures % dtmP->featurePartitionSize ;
  dtmFeatureP = dtmP->fTablePP[partitionNum] + partitionOfs ;
  for( feature = dtmP->numFeatures ; feature < dtmP->memFeatures ; ++feature )
    {
     dtmFeatureP->dtmFeatureState          =  DTMFeatureState::Unused  ;
     dtmFeatureP->dtmFeatureType           =  (DTMFeatureType)0           ;
     dtmFeatureP->dtmUserTag               =  DTM_NULL_USER_TAG ;
     dtmFeatureP->dtmFeatureId             =  DTM_NULL_FEATURE_ID    ;
     dtmFeatureP->dtmFeaturePts.pointsPI   =  0        ;
     dtmFeatureP->numDtmFeaturePts         =  0           ;
     ++partitionOfs ;
     if( partitionOfs == dtmP->featurePartitionSize ) 
       {
        ++partitionNum ;
        partitionOfs = 0 ;
        dtmFeatureP = dtmP->fTablePP[partitionNum]   ;
       }
     else ++dtmFeatureP ; 
    }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Allocating Memory For Features Array Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Allocating Memory For Features Array Error") ;
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
BENTLEYDTM_EXPORT int bcdtmObject_setPointMemoryAllocationParametersDtmObject
(
 BC_DTM_OBJ *dtmP,
 long iniPoints,
 long incPoints
)
/*
** This Function Sets The Point Memory Allocation Parameters For A Dtm Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Set Point Memory Allocation Parameters") ;
    bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"iniPoints = %8ld",iniPoints) ;
    bcdtmWrite_message(0,0,0,"incPoints = %8ld",incPoints) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Validate
*/
 if( iniPoints < 1000 ) iniPoints = 1000 ;
 if( incPoints <  100 ) incPoints = 100  ;
 if( incPoints > iniPoints ) incPoints = iniPoints ;
/*
** Set Memory Allocation Parameters
*/
 dtmP->iniPoints = iniPoints ;
 dtmP->incPoints = incPoints ;
/*
** Clean Up
*/
 cleanup :
/*
**  Job Completed
*/
 return(DTM_SUCCESS) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_allocatePointsMemoryDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Allocates Memory For DTM Points
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,point,numPartitions,remPartition,numCurPartitions,remCurPartition,partitionNum,partitionOfs,partitionSize ;
 DTM_TIN_POINT *dtmPointP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Points Array") ;
    bcdtmWrite_message(0,0,0,"dtmP                     = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints          = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->memPoints          = %8ld",dtmP->memPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->iniPoints          = %8ld",dtmP->iniPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->incPoints          = %8ld",dtmP->incPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->pointPartitionSize = %8ld",dtmP->pointPartitionSize) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPointPartitions = %8ld",dtmP->numPointPartitions) ;
   }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Initial Allocation
*/
 if( dtmP->memPoints == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Initial Memory For Points Array") ;
    dtmP->memPoints = dtmP->iniPoints ;
    // Check that memPoints is big enough to store the numPoints.
    while(dtmP->memPoints < dtmP->numPoints)
        dtmP->memPoints = dtmP->memPoints + dtmP->incPoints ;

    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->memPoints = %8ld",dtmP->memPoints) ;
/*
**  Determine Number Of Point Partitions
*/
    numPartitions = dtmP->memPoints / dtmP->pointPartitionSize + 1 ; 
    remPartition  = dtmP->memPoints % dtmP->pointPartitionSize ;
    if( remPartition == 0 ) { --numPartitions ; remPartition = dtmP->pointPartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPartitions = %8ld ** remPartition = %8ld",numPartitions,remPartition) ;
/*
**  Allocate Memory For Pointers To Point Partitions
*/
    dtmP->numPointPartitions = numPartitions ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Point Partition Pointers") ;
    dtmP->pointsPP  = ( DTM_TIN_POINT  ** ) malloc( dtmP->numPointPartitions * sizeof( DTM_TIN_POINT * )) ;
    if( dtmP->pointsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
    for( n = 0 ; n < dtmP->numPointPartitions ; ++n ) dtmP->pointsPP[n] = NULL ;
/*
**  Allocate Memory For Point Partitions
*/
    for( n = 0 ; n < dtmP->numPointPartitions ; ++n )
      {
/*
**     Determine Partition Size
*/
       if( n < dtmP->numPointPartitions - 1 ) partitionSize = dtmP->pointPartitionSize ;
       else                                   partitionSize = remPartition ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
/*
**     Allocate Partition Memory
*/
       dtmP->pointsPP[n] = ( DTM_TIN_POINT * ) bcdtmMemory_allocatePartition(dtmP, DTMPartition::Point, n, partitionSize * sizeof( DTM_TIN_POINT)) ;
       if( dtmP->pointsPP[n] == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
   }
/*
** Incremental Allocation
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Incremental Memory For Points Array") ;
/*
**  Determine Current Number Of Partitions
*/
    numCurPartitions  = dtmP->memPoints / dtmP->pointPartitionSize + 1 ;
    remCurPartition   = dtmP->memPoints % dtmP->pointPartitionSize ;
    if( remCurPartition == 0 ) { --numCurPartitions ; remCurPartition = dtmP->pointPartitionSize ; }
    if( numCurPartitions != dtmP->numPointPartitions )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"numCurPartitions = %6ld ** dtmP->numPointPartitions = %6ld",numCurPartitions,dtmP->numPointPartitions) ;
       bcdtmWrite_message(2,0,0,"Inconsistent Number Of Point Partitions") ;
      } 
/*
**  Increment Number Of Memory Points
*/
    dtmP->memPoints = dtmP->memPoints + dtmP->incPoints ;

    // Check that memPoints is big enough to store the numPoints.
    while(dtmP->memPoints < dtmP->numPoints)
        dtmP->memPoints = dtmP->memPoints + dtmP->incPoints ;

    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->memPoints = %8ld",dtmP->memPoints) ;
/*
**  Determine Required Number Of Partitions
*/
    numPartitions = dtmP->memPoints / dtmP->pointPartitionSize + 1 ; 
    remPartition  = dtmP->memPoints % dtmP->pointPartitionSize ;
    if( remPartition == 0 ) { --numPartitions ; remPartition = dtmP->pointPartitionSize ; }
    if( dbg ) 
      { 
       bcdtmWrite_message(0,0,0,"dtmP->numPointPartitions = %8ld",dtmP->numPointPartitions) ;
       bcdtmWrite_message(0,0,0,"numPartitions = %8ld remPartition = %8ld",numPartitions,remPartition) ;
      } 
/*
**  Memory Allocation Within Current Partition
*/
    if( numPartitions == dtmP->numPointPartitions )
      {
       dtmP->pointsPP[dtmP->numPointPartitions-1] = ( DTM_TIN_POINT * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::Point, dtmP->numPointPartitions-1, dtmP->pointsPP[dtmP->numPointPartitions-1],remPartition*sizeof(DTM_TIN_POINT)) ;      
       if( dtmP->pointsPP[dtmP->numPointPartitions-1] == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Memory Allocation Over A Number Of Partitions
*/
    if( numPartitions > dtmP->numPointPartitions )
      {
/*
**     Allocate Memory For Additional Point Partitions
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Memory For Additional Point Partition Pointers") ;
       dtmP->pointsPP  = ( DTM_TIN_POINT  ** ) realloc(dtmP->pointsPP,numPartitions * sizeof( DTM_TIN_POINT * )) ;
       if( dtmP->pointsPP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ; 
         }
       for( n = dtmP->numPointPartitions ; n < numPartitions ; ++n ) dtmP->pointsPP[n] = NULL ;
/*
**     Allocate Memory In Current Partition
*/
       if( remCurPartition < dtmP->pointPartitionSize )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Reallocating Memory Witin Current Partition") ;
          dtmP->pointsPP[dtmP->numPointPartitions-1] = ( DTM_TIN_POINT * ) bcdtmMemory_reallocatePartition( dtmP, DTMPartition::Point, dtmP->numPointPartitions-1, dtmP->pointsPP[dtmP->numPointPartitions-1],dtmP->pointPartitionSize*sizeof(DTM_TIN_POINT)) ;      
          if( dtmP->pointsPP[dtmP->numPointPartitions-1] == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Allocate Memory For Point Partitions
*/
       for( n = dtmP->numPointPartitions ; n < numPartitions ; ++n )
         {
/*
**        Determine Partition Size
*/
          if( n < numPartitions - 1 ) partitionSize = dtmP->pointPartitionSize ;
          else                        partitionSize = remPartition ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
/*
**        Allocate Partition Memory
*/
          dtmP->pointsPP[n] = ( DTM_TIN_POINT * ) bcdtmMemory_allocatePartition(dtmP, DTMPartition::Point, n, partitionSize * sizeof( DTM_TIN_POINT)) ;
          if( dtmP->pointsPP[n] == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
      }
/*
**  Reset Number Of Partitions
*/
    dtmP->numPointPartitions = numPartitions ;
   }
/*
** Initialise Allocated Memory
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"Initialising Allocated Memory") ;
  partitionNum = dtmP->numPoints / dtmP->pointPartitionSize ;
  partitionOfs = dtmP->numPoints % dtmP->pointPartitionSize ;
  dtmPointP = dtmP->pointsPP[partitionNum] + partitionOfs ;
  for( point = dtmP->numPoints ; point < dtmP->memPoints ; ++point )
    {
     dtmPointP->x = 0.0  ;
     dtmPointP->y = 0.0  ;
     dtmPointP->z = 0.0 ;
     ++partitionOfs ;
     if( partitionOfs == dtmP->pointPartitionSize ) 
       {
        ++partitionNum ;
        partitionOfs = 0 ;
        dtmPointP = dtmP->pointsPP[partitionNum]   ;
       }
     else ++dtmPointP ; 
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
BENTLEYDTM_Public int  bcdtmObject_allocateNodesMemoryDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Allocates Memory For DTM Nodes
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,node,numPartitions,remPartition,numCurPartitions,remCurPartition,partitionNum,partitionOfs,partitionSize ;
 DTM_TIN_NODE *dtmNodeP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Allocating Memory For Nodes Array") ;
    bcdtmWrite_message(0,0,0,"dtmP                    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP->numNodes          = %8ld",dtmP->numNodes) ;
    bcdtmWrite_message(0,0,0,"dtmP->memNodes          = %8ld",dtmP->memNodes) ;
    bcdtmWrite_message(0,0,0,"dtmP->memPoints         = %8ld",dtmP->memPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->nodePartitionSize = %8ld",dtmP->nodePartitionSize) ;
    bcdtmWrite_message(0,0,0,"dtmP->numNodePartitions = %8ld",dtmP->numNodePartitions) ;
   }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Initial Allocation
*/
 if( dtmP->memNodes == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Initial Memory For Nodes Array") ;
    dtmP->memNodes = dtmP->memPoints ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->memNodes = %8ld",dtmP->memNodes) ;
/*
**  Determine Number Of Node Partitions
*/
    numPartitions = dtmP->memNodes / dtmP->nodePartitionSize + 1 ; 
    remPartition  = dtmP->memNodes % dtmP->nodePartitionSize ;
    if( remPartition == 0 ) { --numPartitions ; remPartition = dtmP->nodePartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPartitions = %8ld ** remPartition = %8ld",numPartitions,remPartition) ;
/*
**  Allocate Memory For Pointers To Node Partitions
*/
    dtmP->numNodePartitions = numPartitions ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Node Partition Pointers") ;
    dtmP->nodesPP  = ( DTM_TIN_NODE  ** ) malloc( dtmP->numNodePartitions * sizeof( DTM_TIN_NODE * )) ;
    if( dtmP->nodesPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
    for( n = 0 ; n < dtmP->numNodePartitions ; ++n ) dtmP->nodesPP[n] = NULL ;
/*
**  Allocate Memory For Node Partitions
*/
    for( n = 0 ; n < dtmP->numNodePartitions ; ++n )
      {
/*
**     Determine Partition Size
*/
       if( n < dtmP->numNodePartitions - 1 ) partitionSize = dtmP->nodePartitionSize ;
       else                                  partitionSize = remPartition ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
/*
**     Allocate Partition Memory
*/
       dtmP->nodesPP[n] = ( DTM_TIN_NODE * ) bcdtmMemory_allocatePartition(dtmP, DTMPartition::Node, n, partitionSize * sizeof( DTM_TIN_NODE)) ; 
       if( dtmP->nodesPP[n] == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
   }
/*
** Incremental Allocation
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Incremental Memory For Nodes Array") ;
/*
**  Determine Current Number Of Partitions
*/
    numCurPartitions  = dtmP->memNodes / dtmP->nodePartitionSize + 1 ;
    remCurPartition   = dtmP->memNodes % dtmP->nodePartitionSize ;
    if( remCurPartition == 0 ) { --numCurPartitions ; remCurPartition = dtmP->nodePartitionSize ; }
    if( numCurPartitions != dtmP->numNodePartitions )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"numCurPartitions = %6ld ** dtmP->numNodePartitions = %6ld",numCurPartitions,dtmP->numNodePartitions) ;
       bcdtmWrite_message(2,0,0,"Inconsistent Number Of Node Partitions") ;
      } 
/*
**  Increment Number Of Memory Nodes
*/
    dtmP->numNodes = dtmP->numPoints ;
    dtmP->memNodes = dtmP->memPoints ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->memNodes = %8ld",dtmP->memNodes) ;
/*
**  Determine Required Number Of Partitions
*/
    numPartitions = dtmP->memNodes / dtmP->nodePartitionSize + 1 ; 
    remPartition  = dtmP->memNodes % dtmP->nodePartitionSize ;
    if( remPartition == 0 ) { --numPartitions ; remPartition = dtmP->nodePartitionSize ; }
    if( dbg ) 
      { 
       bcdtmWrite_message(0,0,0,"dtmP->numNodePartitions = %8ld",dtmP->numNodePartitions) ;
       bcdtmWrite_message(0,0,0,"numPartitions = %8ld remPartition = %8ld",numPartitions,remPartition) ;
      } 
/*
**  Memory Allocation Within Current Partition
*/
    if( numPartitions == dtmP->numNodePartitions )
      {
       dtmP->nodesPP[dtmP->numNodePartitions-1] = ( DTM_TIN_NODE * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::Node, dtmP->numNodePartitions-1, dtmP->nodesPP[dtmP->numNodePartitions-1], remPartition * sizeof( DTM_TIN_NODE)) ;
       if( dtmP->nodesPP[dtmP->numNodePartitions-1] == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Memory Allocation Over A Number Of Partitions
*/
    if( numPartitions > dtmP->numNodePartitions )
      {
/*
**     Allocate Memory For Additional Node Partitions
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Memory For Additional Node Partition Pointers") ;
       dtmP->nodesPP  = ( DTM_TIN_NODE  ** ) realloc(dtmP->nodesPP,numPartitions * sizeof( DTM_TIN_NODE * )) ;
       if( dtmP->nodesPP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ; 
         }
       for( n = dtmP->numNodePartitions ; n < numPartitions ; ++n ) dtmP->nodesPP[n] = NULL ;
/*
**     Allocate Memory In Current Partition
*/
       if( remCurPartition < dtmP->nodePartitionSize )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Reallocating Memory Witin Current Partition") ;
          dtmP->nodesPP[dtmP->numNodePartitions-1] = ( DTM_TIN_NODE * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::Node, dtmP->numNodePartitions-1, dtmP->nodesPP[dtmP->numNodePartitions-1], dtmP->nodePartitionSize* sizeof( DTM_TIN_NODE)) ;
          if( dtmP->nodesPP[dtmP->numNodePartitions-1] == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Allocate Memory For Node Partitions
*/
       for( n = dtmP->numNodePartitions ; n < numPartitions ; ++n )
         {
/*
**        Determine Partition Size
*/
          if( n < numPartitions - 1 ) partitionSize = dtmP->nodePartitionSize ;
          else                        partitionSize = remPartition ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
/*
**        Allocate Partition Memory
*/
          dtmP->nodesPP[n] = ( DTM_TIN_NODE * ) bcdtmMemory_allocatePartition(dtmP, DTMPartition::Node, n, partitionSize * sizeof( DTM_TIN_NODE)) ;
          if( dtmP->nodesPP[n] == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
      }
/*
**  Reset Number Of Partitions
*/
    dtmP->numNodePartitions = numPartitions ;
   }
/*
** Initialise Allocated Memory
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"Initialising Allocated Memory") ;
  partitionNum = dtmP->numNodes / dtmP->nodePartitionSize ;
  partitionOfs = dtmP->numNodes % dtmP->nodePartitionSize ;
  dtmNodeP  = dtmP->nodesPP[partitionNum] + partitionOfs ;
  for( node = dtmP->numNodes ; node < dtmP->memNodes ; ++node )
    {
     dtmNodeP->cPtr =  dtmP->nullPtr ;
     dtmNodeP->fPtr =  dtmP->nullPtr ;
     dtmNodeP->hPtr =  dtmP->nullPnt ;
     dtmNodeP->sPtr =  dtmP->nullPnt ;
     dtmNodeP->tPtr =  dtmP->nullPnt ;
     dtmNodeP->PCWD =  0 ;
     dtmNodeP->PRGN =  0 ;
     ++partitionOfs ;
     if( partitionOfs == dtmP->nodePartitionSize ) 
       {
        ++partitionNum ;
        partitionOfs = 0 ;
        dtmNodeP = dtmP->nodesPP[partitionNum]   ;
       }
     else ++dtmNodeP ; 
    }
/*
** Set Number Of Nodes To Number Of Points
*/
 dtmP->numNodes = dtmP->numPoints ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Allocating Memory For Nodes Array Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Allocating Memory For Nodes Array Error") ;
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
BENTLEYDTM_Public int  bcdtmObject_allocateCircularListMemoryDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Allocates Memory For DTM Clist
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,clist,numPartitions,remPartition,numCurPartitions,remCurPartition,partitionNum,partitionOfs,partitionSize ;
 DTM_CIR_LIST *dtmClistP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Allocating Memory For Clist Array") ;
    bcdtmWrite_message(0,0,0,"dtmP                     = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP->numClist           = %8ld",dtmP->numClist) ;
    bcdtmWrite_message(0,0,0,"dtmP->memClist           = %8ld",dtmP->memClist) ;
    bcdtmWrite_message(0,0,0,"dtmP->memPoints          = %8ld",dtmP->memPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->clistPartitionSize = %8ld",dtmP->clistPartitionSize) ;
    bcdtmWrite_message(0,0,0,"dtmP->numClistPartitions = %8ld",dtmP->numClistPartitions) ;
   }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Initial Allocation
*/
 dtmP->numClist = dtmP->cListPtr ;
 if( dtmP->memClist == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Initial Memory For Clist Array") ;
    dtmP->memClist = dtmP->memPoints * 6 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->memClist = %8ld",dtmP->memClist) ;
/*
**  Determine Number Of Clist Partitions
*/
    numPartitions = dtmP->memClist / dtmP->clistPartitionSize + 1 ; 
    remPartition  = dtmP->memClist % dtmP->clistPartitionSize ;
    if( remPartition == 0 ) { --numPartitions ; remPartition = dtmP->clistPartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPartitions = %8ld ** remPartition = %8ld",numPartitions,remPartition) ;
/*
**  Allocate Memory For Pointers To Clist Partitions
*/
    dtmP->numClistPartitions = numPartitions ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Clist Partition Pointers") ;
    dtmP->cListPP  = ( DTM_CIR_LIST  ** ) malloc( dtmP->numClistPartitions * sizeof( DTM_CIR_LIST * )) ;
    if( dtmP->cListPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
    for( n = 0 ; n < dtmP->numClistPartitions ; ++n ) dtmP->cListPP[n] = NULL ;
/*
**  Allocate Memory For Clist Partitions
*/
    for( n = 0 ; n < dtmP->numClistPartitions ; ++n )
      {
/*
**     Determine Partition Size
*/
       if( n < dtmP->numClistPartitions - 1 ) partitionSize = dtmP->clistPartitionSize ;
       else                                     partitionSize = remPartition ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
/*
**     Allocate Partition Memory
*/
       dtmP->cListPP[n] = ( DTM_CIR_LIST * ) bcdtmMemory_allocatePartition(dtmP, DTMPartition::CList, n, partitionSize * sizeof( DTM_CIR_LIST)) ;
       if( dtmP->cListPP[n] == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
   }
/*
** Incremental Allocation
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Incremental Memory For Clist Array") ;
/*
**  Determine Current Number Of Partitions
*/
    numCurPartitions  = dtmP->memClist / dtmP->clistPartitionSize + 1 ;
    remCurPartition   = dtmP->memClist % dtmP->clistPartitionSize ;
    if( remCurPartition == 0 ) { --numCurPartitions ; remCurPartition = dtmP->clistPartitionSize ; }
    if( numCurPartitions != dtmP->numClistPartitions )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"numCurPartitions = %6ld ** dtmP->numClistPartitions = %6ld",numCurPartitions,dtmP->numClistPartitions) ;
       bcdtmWrite_message(2,0,0,"Inconsistent Number Of Clist Partitions") ;
      } 
/*
**  Increment Number Of Memory Clist
*/
    dtmP->memClist = dtmP->memPoints * 6 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->memClist = %8ld",dtmP->memClist) ;
/*
**  Determine Required Number Of Partitions
*/
    numPartitions = dtmP->memClist / dtmP->clistPartitionSize + 1 ; 
    remPartition  = dtmP->memClist % dtmP->clistPartitionSize ;
    if( remPartition == 0 ) { --numPartitions ; remPartition = dtmP->clistPartitionSize ; }
    if( dbg ) 
      { 
       bcdtmWrite_message(0,0,0,"dtmP->numClistPartitions = %8ld",dtmP->numClistPartitions) ;
       bcdtmWrite_message(0,0,0,"numPartitions = %8ld remPartition = %8ld",numPartitions,remPartition) ;
      } 
/*
**  Memory Allocation Within Current Partition
*/
    if( numPartitions == dtmP->numClistPartitions )
      {
       dtmP->cListPP[dtmP->numClistPartitions-1] = ( DTM_CIR_LIST * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::CList, dtmP->numClistPartitions-1, dtmP->cListPP[dtmP->numClistPartitions-1], remPartition * sizeof( DTM_CIR_LIST)) ;
       if( dtmP->cListPP[dtmP->numClistPartitions-1] == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Memory Allocation Over A Number Of Partitions
*/
    if( numPartitions > dtmP->numClistPartitions )
      {
/*
**     Allocate Memory For Additional Clist Partitions
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Memory For Additional Clist Partition Pointers") ;
       dtmP->cListPP  = ( DTM_CIR_LIST  ** ) realloc(dtmP->cListPP,numPartitions * sizeof( DTM_CIR_LIST * )) ;
       if( dtmP->cListPP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ; 
         }
       for( n = dtmP->numClistPartitions ; n < numPartitions ; ++n ) dtmP->cListPP[n] = NULL ;
/*
**     Allocate Memory In Current Partition
*/
       if( remCurPartition < dtmP->clistPartitionSize )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Reallocating Memory Witin Current Partition") ;
          dtmP->cListPP[dtmP->numClistPartitions-1] = ( DTM_CIR_LIST * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::CList, dtmP->numClistPartitions-1, dtmP->cListPP[dtmP->numClistPartitions-1], dtmP->clistPartitionSize * sizeof( DTM_CIR_LIST)) ;
          if( dtmP->cListPP[dtmP->numClistPartitions-1] == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Allocate Memory For Clist Partitions
*/
       for( n = dtmP->numClistPartitions ; n < numPartitions ; ++n )
         {
/*
**        Determine Partition Size
*/
          if( n < numPartitions - 1 ) partitionSize = dtmP->clistPartitionSize ;
          else                        partitionSize = remPartition ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
/*
**        Allocate Partition Memory
*/
          dtmP->cListPP[n] = ( DTM_CIR_LIST * ) bcdtmMemory_allocatePartition(dtmP, DTMPartition::CList, n, partitionSize * sizeof( DTM_CIR_LIST)) ;
          if( dtmP->cListPP[n] == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
      }
/*
**  Reset Number Of Partitions
*/
    dtmP->numClistPartitions = numPartitions ;
   }
/*
** Initialise Allocated Memory
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"Initialising Allocated Memory") ;
  partitionNum = dtmP->numClist / dtmP->clistPartitionSize ;
  partitionOfs = dtmP->numClist % dtmP->clistPartitionSize ;
  dtmClistP = dtmP->cListPP[partitionNum] + partitionOfs ;
  for( clist = dtmP->numClist ; clist < dtmP->memClist ; ++clist )
    {
     dtmClistP->nextPtr =  DTM_NULL_PTR   ;
     dtmClistP->pntNum  =  DTM_NULL_PNT   ;
     ++partitionOfs ;
     if( partitionOfs == dtmP->clistPartitionSize ) 
       {
        ++partitionNum ;
        partitionOfs = 0 ;
        dtmClistP = dtmP->cListPP[partitionNum]   ;
       }
     else ++dtmClistP ; 
    }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Allocating Memory For Clist Array Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Allocating Memory For Clist Array Error") ;
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
BENTLEYDTM_Public int  bcdtmObject_allocateFeatureListMemoryDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Allocates Memory For DTM Flist
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,flist,numPartitions,remPartition,numCurPartitions,remCurPartition,partitionNum,partitionOfs,partitionSize ;
 DTM_FEATURE_LIST *dtmFlistP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Allocating Memory For Feature List Array") ;
    bcdtmWrite_message(0,0,0,"dtmP                     = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFlist           = %8ld",dtmP->numFlist) ;
    bcdtmWrite_message(0,0,0,"dtmP->memFlist           = %8ld",dtmP->memFlist) ;
    bcdtmWrite_message(0,0,0,"dtmP->iniFlist           = %8ld",dtmP->iniFlist) ;
    bcdtmWrite_message(0,0,0,"dtmP->incFlist           = %8ld",dtmP->incFlist) ;
    bcdtmWrite_message(0,0,0,"dtmP->flistPartitionSize = %8ld",dtmP->flistPartitionSize) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFlistPartitions = %8ld",dtmP->numFlistPartitions) ;
   }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Initial Allocation
*/
 if( dtmP->memFlist == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Initial Memory For Flist Array") ;
    dtmP->memFlist = dtmP->iniFlist ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->memFlist = %8ld",dtmP->memFlist) ;
/*
**  Determine Number Of Flist Partitions
*/
    numPartitions = dtmP->memFlist / dtmP->flistPartitionSize + 1 ; 
    remPartition  = dtmP->memFlist % dtmP->flistPartitionSize ;
    if( remPartition == 0 ) { --numPartitions ; remPartition = dtmP->flistPartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPartitions = %8ld ** remPartition = %8ld",numPartitions,remPartition) ;
/*
**  Allocate Memory For Pointers To Flist Partitions
*/
    dtmP->numFlistPartitions = numPartitions ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Flist Partition Pointers") ;
    dtmP->fListPP  = ( DTM_FEATURE_LIST  ** ) malloc( dtmP->numFlistPartitions * sizeof( DTM_FEATURE_LIST * )) ;
    if( dtmP->fListPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
    for( n = 0 ; n < dtmP->numFlistPartitions ; ++n ) dtmP->fListPP[n] = NULL ;
/*
**  Allocate Memory For Flist Partitions
*/
    for( n = 0 ; n < dtmP->numFlistPartitions ; ++n )
      {
/*
**     Determine Partition Size
*/
       if( n < dtmP->numFlistPartitions - 1 ) partitionSize = dtmP->flistPartitionSize ;
       else                                   partitionSize = remPartition ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
/*
**     Allocate Partition Memory
*/
       dtmP->fListPP[n] = ( DTM_FEATURE_LIST * ) bcdtmMemory_allocatePartition(dtmP, DTMPartition::FList, n, partitionSize * sizeof( DTM_FEATURE_LIST)) ;
       if( dtmP->fListPP[n] == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"Partition Memory Allocated") ;
      }
   }
/*
** Incremental Allocation
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Incremental Memory For Flist Array") ;
/*
**  Determine Current Number Of Partitions
*/
    numCurPartitions  = dtmP->memFlist / dtmP->flistPartitionSize + 1 ;
    remCurPartition   = dtmP->memFlist % dtmP->flistPartitionSize ;
    if( remCurPartition == 0 ) { --numCurPartitions ; remCurPartition = dtmP->flistPartitionSize ; }
    if( numCurPartitions != dtmP->numFlistPartitions )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"numCurPartitions = %6ld ** dtmP->numFlistPartitions = %6ld",numCurPartitions,dtmP->numFlistPartitions) ;
       bcdtmWrite_message(2,0,0,"Inconsistent Number Of Flist Partitions") ;
      } 
/*
**  Increment Number Of Memory Flist
*/
    dtmP->memFlist = dtmP->memFlist + dtmP->incFlist ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->memFlist = %8ld",dtmP->memFlist) ;
/*
**  Determine Required Number Of Partitions
*/
    numPartitions = dtmP->memFlist / dtmP->flistPartitionSize + 1 ; 
    remPartition  = dtmP->memFlist % dtmP->flistPartitionSize ;
    if( remPartition == 0 ) { --numPartitions ; remPartition = dtmP->flistPartitionSize ; }
    if( dbg ) 
      { 
       bcdtmWrite_message(0,0,0,"dtmP->numFlistPartitions = %8ld",dtmP->numFlistPartitions) ;
       bcdtmWrite_message(0,0,0,"numPartitions = %8ld remPartition = %8ld",numPartitions,remPartition) ;
      } 
/*
**  Memory Allocation Within Current Partition
*/
    if( numPartitions == dtmP->numFlistPartitions )
      {
       dtmP->fListPP[dtmP->numFlistPartitions-1] = ( DTM_FEATURE_LIST * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::FList, dtmP->numFlistPartitions-1, dtmP->fListPP[dtmP->numFlistPartitions-1], remPartition * sizeof( DTM_FEATURE_LIST )) ;
       if( dtmP->fListPP[dtmP->numFlistPartitions-1] == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**  Memory Allocation Over A Number Of Partitions
*/
    if( numPartitions > dtmP->numFlistPartitions )
      {
/*
**     Allocate Memory For Additional Flist Partitions
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"**** Allocating Memory For Additional Flist Partition Pointers") ;
       dtmP->fListPP  = ( DTM_FEATURE_LIST  ** ) realloc(dtmP->fListPP,numPartitions * sizeof( DTM_FEATURE_LIST * )) ;
       if( dtmP->fListPP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ; 
         }
       for( n = dtmP->numFlistPartitions ; n < numPartitions ; ++n ) dtmP->fListPP[n] = NULL ;
/*
**     Allocate Memory In Current Partition
*/
       if( remCurPartition < dtmP->flistPartitionSize )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Reallocating Memory Witin Current Partition") ;
          dtmP->fListPP[dtmP->numFlistPartitions-1] = ( DTM_FEATURE_LIST * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::FList, dtmP->numFlistPartitions-1, dtmP->fListPP[dtmP->numFlistPartitions-1], dtmP->flistPartitionSize * sizeof( DTM_FEATURE_LIST )) ;
          if( dtmP->fListPP[dtmP->numFlistPartitions-1] == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Allocate Memory For Flist Partitions
*/
       for( n = dtmP->numFlistPartitions ; n < numPartitions ; ++n )
         {
/*
**        Determine Partition Size
*/
          if( n < numPartitions - 1 ) partitionSize = dtmP->flistPartitionSize ;
          else                        partitionSize = remPartition ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
/*
**        Allocate Partition Memory
*/
          dtmP->fListPP[n] = ( DTM_FEATURE_LIST * ) bcdtmMemory_allocatePartition(dtmP, DTMPartition::FList, n, partitionSize * sizeof( DTM_FEATURE_LIST )) ;
          if( dtmP->fListPP[n] == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
      }
/*
**  Reset Number Of Partitions
*/
    dtmP->numFlistPartitions = numPartitions ;
   }
/*
** Initialise Allocated Memory
*/
  if( dbg ) bcdtmWrite_message(0,0,0,"Initialising Allocated Memory") ;
  partitionNum = dtmP->numFlist / dtmP->flistPartitionSize ;
  partitionOfs = dtmP->numFlist % dtmP->flistPartitionSize ;
  dtmFlistP = dtmP->fListPP[partitionNum] + partitionOfs ;
  for( flist = dtmP->numFlist ; flist < dtmP->memFlist ; ++flist )
    {
     dtmFlistP->dtmFeature = dtmP->nullPnt ;
     dtmFlistP->nextPnt    = dtmP->nullPnt ;
     dtmFlistP->nextPtr    = dtmP->nullPtr ;
     dtmFlistP->pntType    = 1 ;
     ++partitionOfs ;
     if( partitionOfs == dtmP->flistPartitionSize ) 
       {
        ++partitionNum ;
        partitionOfs = 0 ;
        dtmFlistP = dtmP->fListPP[partitionNum]   ;
       }
     else ++dtmFlistP ; 
    }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Allocating Memory For Feature List Array Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Allocating Memory For Feature List Array Error") ;
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
BENTLEYDTM_Public int  bcdtmObject_incrementTinMemoryDtmObject(BC_DTM_OBJ *dtmP) 
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Incrementing Memory ** dtmP->numPoints = %8ld dtmP->memPoints = %8ld dtmP->incPoints = %8ld",dtmP->numPoints,dtmP->memPoints,dtmP->incPoints) ;
/*
** Alocate Points Memory
*/
 if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP) ) goto errexit ;
/*
** Alocate Nodes Memory
*/
 if( bcdtmObject_allocateNodesMemoryDtmObject(dtmP) ) goto errexit ;
/*
** Allocate Circular List Memory
*/
 if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP) ) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmObject_resizeMemoryDtmObject(BC_DTM_OBJ *dtmP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,numPartition,memPartition,remNumPartition,remMemPartition ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resizing Memory Dtm Object %p",dtmP) ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Resize Feature Memory
*/
 if( dtmP->numFeatures < dtmP->memFeatures )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resizing Feature Memory ** dtmP->numFeatures = %8ld dtmP->memFeatures = %8ld",dtmP->numFeatures,dtmP->memFeatures) ;
/*
**  Determine Partition Extent For The Feature Memory Required
*/
    numPartition    = dtmP->numFeatures / dtmP->featurePartitionSize + 1 ;
    remNumPartition = dtmP->numFeatures % dtmP->featurePartitionSize ;
    if( remNumPartition == 0 ) { --numPartition ; remNumPartition = dtmP->featurePartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPartition = %8ld ** remNumPartition = %8ld",numPartition,remNumPartition) ;
/*
**  Determine Partition Extent For The Feature Memory Allocated
*/
    memPartition    = dtmP->memFeatures / dtmP->featurePartitionSize + 1 ;
    remMemPartition = dtmP->memFeatures % dtmP->featurePartitionSize ;
    if( remMemPartition == 0 ) { --memPartition ; remMemPartition = dtmP->featurePartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"memPartition = %8ld ** remMemPartition = %8ld",memPartition,remMemPartition) ;
/*
**  Free Memory From Unused Feature Partitions
*/
    for( n = numPartition ; n < memPartition ; ++n )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Freeing Memory For Partition %6ld",n) ;
       if( dtmP->fTablePP[n] != NULL ) 
         {
          bcdtmMemory_freePartition(dtmP, DTMPartition::Feature, n, dtmP->fTablePP[n]) ;
          dtmP->fTablePP[n] = NULL ;
         }
      }
/*
**  Realloc Memory For Last Partition
*/
    if( remNumPartition < dtmP->featurePartitionSize )
      {
       dtmP->fTablePP[numPartition-1] = ( BC_DTM_FEATURE * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::Feature, numPartition-1, dtmP->fTablePP[numPartition-1],remNumPartition * sizeof(BC_DTM_FEATURE)) ; 
      }
/*
**  Realloc Partition Pointer Memory
*/
    if( numPartition < dtmP->numFeaturePartitions )
      {
       dtmP->numFeaturePartitions = numPartition ;
       dtmP->fTablePP = ( BC_DTM_FEATURE ** ) realloc (dtmP->fTablePP,dtmP->numFeaturePartitions * sizeof(BC_DTM_FEATURE *)) ; 
      }
/*
**  Reset Number Of Memory Features And Partitions
*/
    dtmP->memFeatures = dtmP->numFeatures ;
   }
/*
** Resize Point Memory
*/
 if( dtmP->numPoints < dtmP->memPoints )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resizing Point Memory   ** dtmP->numPoints = %8ld dtmP->memPoints = %8ld",dtmP->numPoints,dtmP->memPoints) ;
/*
**  Determine Partition Extent For The Point Memory Required
*/
    numPartition    = dtmP->numPoints / dtmP->pointPartitionSize + 1 ;
    remNumPartition = dtmP->numPoints % dtmP->pointPartitionSize ;
    if( remNumPartition == 0 ) { --numPartition ; remNumPartition = dtmP->pointPartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPartition = %8ld ** remNumPartition = %8ld",numPartition,remNumPartition) ;
/*
**  Determine Partition Extent For The Point Memory Allocated
*/
    memPartition    = dtmP->memPoints / dtmP->pointPartitionSize + 1 ;
    remMemPartition = dtmP->memPoints % dtmP->pointPartitionSize ;
    if( remMemPartition == 0 ) { --memPartition ; remMemPartition = dtmP->pointPartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"memPartition = %8ld ** remMemPartition = %8ld",memPartition,remMemPartition) ;
/*
**  Free Memory From Unused Point Partitions
*/
    for( n = numPartition ; n < memPartition ; ++n )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Freeing Memory For Partition %6ld",n) ;
       if( dtmP->pointsPP[n] != NULL ) 
         {
          bcdtmMemory_freePartition(dtmP, DTMPartition::Point, n, dtmP->pointsPP[n]) ;
          dtmP->pointsPP[n] = NULL ;
         }
      }
/*
**  Realloc Memory For Last Partition
*/
    if( remNumPartition < dtmP->pointPartitionSize )
      {
       dtmP->pointsPP[numPartition-1] = ( DTM_TIN_POINT * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::Point, numPartition-1, dtmP->pointsPP[numPartition-1],remNumPartition * sizeof(DTM_TIN_POINT)) ; 
      }
/*
**  Realloc Partition Pointer Memory
*/
    if( numPartition < dtmP->numPointPartitions )
      {
       dtmP->numPointPartitions = numPartition ;
       dtmP->pointsPP = ( DTM_TIN_POINT ** ) realloc (dtmP->pointsPP,dtmP->numPointPartitions * sizeof(DTM_TIN_POINT *)) ; 
      }
/*
**  Reset Number Of Memory Points And Partitions
*/
    dtmP->memPoints = dtmP->numPoints ;
   }
/*
** Resize Nodes Memory
*/
 if( dtmP->nodesPP != NULL ) dtmP->numNodes = dtmP->numPoints ;
 if( dtmP->numNodes < dtmP->memNodes )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resizing Node Memory    ** dtmP->numNodes = %8ld dtmP->memNodes = %8ld",dtmP->numNodes,dtmP->memNodes) ;
/*
**  Determine Partition Extent For The Node Memory Required
*/
    numPartition    = dtmP->numNodes / dtmP->nodePartitionSize + 1 ;
    remNumPartition = dtmP->numNodes % dtmP->nodePartitionSize ;
    if( remNumPartition == 0 ) { --numPartition ; remNumPartition = dtmP->nodePartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPartition = %8ld ** remNumPartition = %8ld",numPartition,remNumPartition) ;
/*
**  Determine Partition Extent For The Node Memory Allocated
*/
    memPartition    = dtmP->memNodes / dtmP->nodePartitionSize + 1 ;
    remMemPartition = dtmP->memNodes % dtmP->nodePartitionSize ;
    if( remMemPartition == 0 ) { --memPartition ; remMemPartition = dtmP->nodePartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"memPartition = %8ld ** remMemPartition = %8ld",memPartition,remMemPartition) ;
/*
**  Free Memory From Unused Node Partitions
*/
    for( n = numPartition ; n < memPartition ; ++n )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Freeing Memory For Partition %6ld",n) ;
       if( dtmP->nodesPP[n] != NULL ) 
         {
          bcdtmMemory_freePartition(dtmP, DTMPartition::Node, n, dtmP->nodesPP[n]) ;
          dtmP->nodesPP[n] = NULL ;
         }
      }
/*
**  Realloc Memory For Last Partition
*/
    if( remNumPartition < dtmP->nodePartitionSize )
      {
       dtmP->nodesPP[numPartition-1] = ( DTM_TIN_NODE * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::Node, numPartition-1, dtmP->nodesPP[numPartition-1],remNumPartition * sizeof(DTM_TIN_NODE)) ; 
      }
/*
**  Realloc Partition Nodeer Memory
*/
    if( numPartition < dtmP->numNodePartitions )
      {
       dtmP->numNodePartitions = numPartition ;
       dtmP->nodesPP = ( DTM_TIN_NODE ** ) realloc (dtmP->nodesPP,dtmP->numNodePartitions * sizeof(DTM_TIN_NODE *)) ; 
      }
/*
**  Reset Number Of Memory Nodes And Partitions
*/
    dtmP->memNodes = dtmP->numNodes ;
   }
/*
** Resize Clist Memory
*/
 dtmP->numClist = dtmP->cListPtr ;
 if( dtmP->numClist < dtmP->memClist )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resizing Clist Memory   ** dtmP->numClist = %8ld dtmP->memClist = %8ld",dtmP->numClist,dtmP->memClist) ;
/*
**  Determine Partition Extent For The Clist Memory Required
*/
    numPartition    = dtmP->numClist / dtmP->clistPartitionSize + 1 ;
    remNumPartition = dtmP->numClist % dtmP->clistPartitionSize ;
    if( remNumPartition == 0 ) { --numPartition ; remNumPartition = dtmP->clistPartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPartition = %8ld ** remNumPartition = %8ld",numPartition,remNumPartition) ;
/*
**  Determine Partition Extent For The Clist Memory Allocated
*/
    memPartition    = dtmP->memClist / dtmP->clistPartitionSize + 1 ;
    remMemPartition = dtmP->memClist % dtmP->clistPartitionSize ;
    if( remMemPartition == 0 ) { --memPartition ; remMemPartition = dtmP->clistPartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"memPartition = %8ld ** remMemPartition = %8ld",memPartition,remMemPartition) ;
/*
**  Free Memory From Unused Clist Partitions
*/
    for( n = numPartition ; n < memPartition ; ++n )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Freeing Memory For Partition %6ld",n) ;
       if( dtmP->cListPP[n] != NULL ) 
         {
          bcdtmMemory_freePartition(dtmP, DTMPartition::CList, n, dtmP->cListPP[n]) ;
          dtmP->cListPP[n] = NULL ;
         }
      }
/*
**  Realloc Memory For Last Partition
*/
    if( remNumPartition < dtmP->clistPartitionSize )
      {
       dtmP->cListPP[numPartition-1] = ( DTM_CIR_LIST * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::CList, numPartition-1, dtmP->cListPP[numPartition-1],remNumPartition * sizeof(DTM_CIR_LIST)) ; 
      }
/*
**  Realloc Partition Pointer Memory
*/
    if( numPartition < dtmP->numClistPartitions )
      {
       dtmP->numClistPartitions = numPartition ;
       dtmP->cListPP = ( DTM_CIR_LIST ** ) realloc (dtmP->cListPP,dtmP->numClistPartitions * sizeof(DTM_CIR_LIST *)) ; 
      }
/*
**  Reset Number Of Memory Clist
*/
    dtmP->memClist = dtmP->numClist ;
   }
/*
** Resize Flist Memory
*/
/*
** Resize Flist Memory
*/
 if( dtmP->numFlist < dtmP->memFlist )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resizing Flist Memory   ** dtmP->numFlist = %8ld dtmP->memFlist = %8ld",dtmP->numFlist,dtmP->memFlist) ;
/*
**  Determine Partition Extent For The Flist Memory Required
*/
    numPartition    = dtmP->numFlist / dtmP->flistPartitionSize + 1 ;
    remNumPartition = dtmP->numFlist % dtmP->flistPartitionSize ;
    if( remNumPartition == 0 ) { --numPartition ; remNumPartition = dtmP->flistPartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPartition = %8ld ** remNumPartition = %8ld",numPartition,remNumPartition) ;
/*
**  Determine Partition Extent For The Flist Memory Allocated
*/
    memPartition    = dtmP->memFlist / dtmP->flistPartitionSize + 1 ;
    remMemPartition = dtmP->memFlist % dtmP->flistPartitionSize ;
    if( remMemPartition == 0 ) { --memPartition ; remMemPartition = dtmP->flistPartitionSize ; }
    if( dbg ) bcdtmWrite_message(0,0,0,"memPartition = %8ld ** remMemPartition = %8ld",memPartition,remMemPartition) ;
/*
**  Free Memory From Unused Flist Partitions
*/
    for( n = numPartition ; n < memPartition ; ++n )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Freeing Memory For Partition %6ld",n) ;
       if( dtmP->fListPP[n] != NULL ) 
         {
          bcdtmMemory_freePartition(dtmP, DTMPartition::FList, n, dtmP->fListPP[n]) ;
          dtmP->fListPP[n] = NULL ;
         }
      }
/*
**  Realloc Memory For Last Partition
*/
    if( remNumPartition < dtmP->flistPartitionSize )
      {
       dtmP->fListPP[numPartition-1] = ( DTM_FEATURE_LIST * ) bcdtmMemory_reallocatePartition(dtmP, DTMPartition::FList, numPartition-1, dtmP->fListPP[numPartition-1],remNumPartition * sizeof(DTM_FEATURE_LIST)) ; 
      }
/*
**  Realloc Partition Pointer Memory
*/
    if( numPartition < dtmP->numFlistPartitions )
      {
       dtmP->numFlistPartitions = numPartition ;
       dtmP->fListPP = ( DTM_FEATURE_LIST ** ) realloc (dtmP->fListPP,dtmP->numFlistPartitions * sizeof(DTM_FEATURE_LIST *)) ; 
      }
/*
**  Reset Number Of Memory Flist
*/
    dtmP->memFlist = dtmP->numFlist ;
   }
/*
** Write Sizeof Resized Arrays
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Resized Feature Memory  ** dtmP->numFeatures = %8ld dtmP->memFeatures = %8ld",dtmP->numFeatures,dtmP->memFeatures) ;
    bcdtmWrite_message(0,0,0,"Resized Point Memory    ** dtmP->numPoints   = %8ld dtmP->memPoints   = %8ld",dtmP->numPoints,dtmP->memPoints) ;
    bcdtmWrite_message(0,0,0,"Resized Node Memory     ** dtmP->numNodes    = %8ld dtmP->memNodes    = %8ld",dtmP->numNodes,dtmP->memNodes) ;
    bcdtmWrite_message(0,0,0,"Resized Clist Memory    ** dtmP->numClist    = %8ld dtmP->memClist    = %8ld",dtmP->numClist,dtmP->memClist) ;
    bcdtmWrite_message(0,0,0,"Resized Flist Memory    ** dtmP->numFlist    = %8ld dtmP->memFlist    = %8ld",dtmP->numFlist,dtmP->memFlist) ;
  }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resizing Memory Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resizing Memory Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_reportMemoryUsageDtmObject
(
 BC_DTM_OBJ     *dtmP ,                      /* ==> Pointer To DTM Object               */ 
 unsigned long  *headMemAmountP,             /* <== Memory Used By DTM Header           */
 unsigned long  *featureMemAmountP,          /* <== Memory Used By DTM Features         */ 
 unsigned long  *featPtsMemAmountP,          /* <== Memory Used By DTM Feature Points   */
 unsigned long  *pointMemAmountP,            /* <== Memory Used By DTM Points           */ 
 unsigned long  *nodeMemAmountP,             /* <== Memory Used By DTM Nodes            */
 unsigned long  *clistMemAmountP,            /* <== Memory Used By DTM Circular List    */
 unsigned long  *flistMemAmountP,            /* <== Memory Used By DTM Feature List     */ 
 unsigned long  *totalMemAmountP             /* <== Total Memory Used By DTM Object     */
)
{
 int   ret=DTM_SUCCESS;
 long  dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *headMemAmountP     = 0 ;
 *featureMemAmountP  = 0 ;
 *pointMemAmountP    = 0 ;
 *nodeMemAmountP     = 0 ;
 *flistMemAmountP    = 0 ;
 *clistMemAmountP    = 0 ;
 if(featPtsMemAmountP)
     *featPtsMemAmountP  = 0 ;
 *totalMemAmountP    = 0 ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Calculate Memory Usage
*/
 *headMemAmountP    = sizeof(BC_DTM_OBJ) + dtmP->numFeaturePartitions * sizeof( BC_DTM_FEATURE * ) + dtmP->numPointPartitions * sizeof( DTM_TIN_POINT * ) ; 
 *featureMemAmountP = dtmP->memFeatures * sizeof(BC_DTM_FEATURE) ;
 *pointMemAmountP   = dtmP->memPoints   * sizeof(DTM_TIN_POINT) ;
 if( dtmP->nodesPP != NULL ) 
   {
    *nodeMemAmountP = dtmP->memPoints * sizeof(DTM_TIN_NODE) ;
    *headMemAmountP = *headMemAmountP + dtmP->numPointPartitions * sizeof( DTM_TIN_NODE * ) ;
   } 
 if( dtmP->fListPP != NULL )
   {
    *flistMemAmountP = dtmP->memFlist * sizeof(DTM_FEATURE_LIST) ;
    *headMemAmountP = *headMemAmountP + dtmP->numFlistPartitions * sizeof( DTM_FEATURE_LIST * ) ;
   }
 if( dtmP->cListPP != NULL ) 
   {
    *clistMemAmountP = dtmP->cListPtr * sizeof(DTM_CIR_LIST) ;
    *headMemAmountP = *headMemAmountP + dtmP->numClistPartitions * sizeof( DTM_CIR_LIST * ) ;
   }
/*
** Scan Feature Points And Accumulate Point Memory Associated With Features
*/
 if(featPtsMemAmountP)
     {
     for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray  && dtmFeatureP->dtmFeaturePts.pointsPI != 0) *featPtsMemAmountP = *featPtsMemAmountP + dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray && dtmFeatureP->dtmFeaturePts.offsetPI != 0) *featPtsMemAmountP = *featPtsMemAmountP + dtmFeatureP->numDtmFeaturePts * sizeof(long) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError     && dtmFeatureP->dtmFeaturePts.pointsPI != 0) *featPtsMemAmountP = *featPtsMemAmountP + dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d) ;
        }
     }
/*
** Set Total Amount Of Memory
*/
 *totalMemAmountP = *headMemAmountP + *featureMemAmountP + *pointMemAmountP + *nodeMemAmountP + *clistMemAmountP + *flistMemAmountP ;
 if(featPtsMemAmountP)
     *totalMemAmountP += *featPtsMemAmountP;

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
BENTLEYDTM_EXPORT int bcdtmObject_reportMemoryUsageAllDtmObjects
(
 unsigned long  *headMemAmountP,             /* <== Memory Used By DTM Header           */
 unsigned long  *featureMemAmountP,          /* <== Memory Used By DTM Features         */ 
 unsigned long  *featPtsMemAmountP,          /* <== Memory Used By DTM Feature Points   */
 unsigned long  *pointMemAmountP,            /* <== Memory Used By DTM Points           */ 
 unsigned long  *nodeMemAmountP,             /* <== Memory Used By DTM Nodes            */
 unsigned long  *clistMemAmountP,            /* <== Memory Used By DTM Circular List    */
 unsigned long  *flistMemAmountP,            /* <== Memory Used By DTM Feature List     */ 
 unsigned long  *totalMemAmountP             /* <== Total Memory Used By DTM Object     */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long          n,numNodeValues ;
 unsigned long headMemAmount,featureMemAmount,pointMemAmount,featPtsMemAmount,nodeMemAmount ;
 unsigned long clistMemAmount,flistMemAmount,totalMemAmount ;
 BC_DTM_OBJ    *dtmP,**nodeDtmPP=NULL ;
/*
** Initialise
*/
 *headMemAmountP     = 0 ;
 *featureMemAmountP  = 0 ;
 *pointMemAmountP    = 0 ;
 *featPtsMemAmountP  = 0 ;
 *nodeMemAmountP     = 0 ;
 *flistMemAmountP    = 0 ;
 *clistMemAmountP    = 0 ;
 *featPtsMemAmountP  = 0 ;
 *totalMemAmountP    = 0 ;
/*
** Get Array Of Dtm Objects
*/
 if( bcdtmBtree_getArrayOfNodeValues(glbDtmObjBtreeP,&nodeDtmPP,&numNodeValues)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reporting Memory Usage For %8ld Dtm Objects",numNodeValues) ;
/*
** Scan Dtm Object Array List And Calulate Memory Usage
*/
 for( n = 0 ; n < numNodeValues ; ++n )
   {
    dtmP = *(nodeDtmPP+n) ;
    if( bcdtmObject_reportMemoryUsageDtmObject(dtmP,&headMemAmount,&featureMemAmount,&featPtsMemAmount,&pointMemAmount,&nodeMemAmount,&clistMemAmount,&flistMemAmount,&totalMemAmount)) goto errexit ;
    *headMemAmountP    = *headMemAmountP    + headMemAmount    ;
    *featureMemAmountP = *featureMemAmountP + featureMemAmount ;
    *featPtsMemAmountP = *featPtsMemAmountP + featPtsMemAmount ;
    *pointMemAmountP   = *pointMemAmountP   + pointMemAmount   ;
    *nodeMemAmountP    = *nodeMemAmountP    + nodeMemAmount    ;
    *clistMemAmountP   = *clistMemAmountP   + clistMemAmount   ;
    *flistMemAmountP   = *flistMemAmountP   + flistMemAmount   ;
    *totalMemAmountP   = *totalMemAmountP   + totalMemAmount   ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Object[%6ld] = %p ** head = %10ld feature = %10ld featPts = %10ld points = %10ld total = %10ld",n,dtmP,headMemAmount,featureMemAmount,featPtsMemAmount,pointMemAmount,totalMemAmount) ;
   }
/*
** Write Totals
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"All Dtm Objects               ** head = %10ld feature = %10ld featPts = %10ld points = %10ld total = %10ld",*headMemAmountP,*featureMemAmountP,*featPtsMemAmountP,*pointMemAmountP,*totalMemAmountP) ;
/*
** Clean Up
*/
 cleanup :
 if( nodeDtmPP != NULL ) free(nodeDtmPP) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_reportEntityUsageDtmObject
(
 BC_DTM_OBJ *dtmP ,
 unsigned long  *numFeaturesP,
 unsigned long  *memFeaturesP,
 unsigned long  *numPointsP,
 unsigned long  *memPointsP
)
{
 int ret=DTM_SUCCESS ;
/*
** Initialise
*/
 *numFeaturesP = 0 ;
 *memFeaturesP = 0 ;
 *numPointsP   = 0 ;
 *memPointsP   = 0 ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Set Entity Amounts
*/
 *numFeaturesP = dtmP->numFeatures ; 
 *memFeaturesP = dtmP->memFeatures ;
 *numPointsP   = dtmP->numPoints   ;
 *memPointsP   = dtmP->numPoints   ;
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
BENTLEYDTM_EXPORT int bcdtmObject_reportEntityUsageAllDtmObjects
(
 unsigned long  *numFeaturesP,
 unsigned long  *memFeaturesP,
 unsigned long  *numPointsP,
 unsigned long  *memPointsP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long          n,numNodeValues ;
 unsigned long numFeatures,memFeatures,numPoints,memPoints ;
 BC_DTM_OBJ    *dtmP,**nodeDtmPP=NULL ;
/*
** Initialise
*/
 *numFeaturesP = 0 ;
 *memFeaturesP = 0 ;
 *numPointsP   = 0 ;
 *memPointsP   = 0 ;
/*
** Get Array Of Dtm Objects
*/
 if( bcdtmBtree_getArrayOfNodeValues(glbDtmObjBtreeP,&nodeDtmPP,&numNodeValues)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reporting Entity Usage For %8ld Dtm Objects",numNodeValues) ;
/*
** Scan Dtm Object Array List And Accumulate Entity Usage
*/
 for( n = 0 ; n < numNodeValues ; ++n )
   {
    dtmP = *(nodeDtmPP+n) ;
    if( bcdtmObject_reportEntityUsageDtmObject(dtmP,&numFeatures,&memFeatures,&numPoints,&memPoints)) goto errexit ;
    *numFeaturesP = *numFeaturesP + numFeatures ;
    *memFeaturesP = *memFeaturesP + memFeatures ;
    *numPointsP   = *numPointsP   + numPoints   ;
    *memPointsP   = *memPointsP   + memPoints   ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Object[%6ld] = %p ** numFeatures = %10ld memFeatures = %10ld numPoints = %10ld memPoints = %10ld",n,dtmP,numFeatures,memFeatures,numPoints,memPoints) ;
   }
/*
** Write Totals
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"All Dtm Objects               ** numFeatures = %10ld memFeatures = %10ld numPoints = %10ld memPoints = %10ld",*numFeaturesP,*memFeaturesP,*numPointsP,*memPointsP) ;
/*
** Clean Up
*/
 cleanup :
 if( nodeDtmPP != NULL ) free(nodeDtmPP) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_copyDataObjectToDtmObject(DTM_DAT_OBJ *dataP,BC_DTM_OBJ **dtmPP)
/*
** This Function Copies A Data Object To A Dtm Object
*/
{
 int               ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long              fsCode, fnCode, offset, numFeaturePts = 0;
 DTMFeatureType dtmFeatureType;
 DPoint3d               *points1P,*points2P,*featurePtsP=NULL ;
 DTM_FEATURE_CODE  *fCodeP,*nCodeP ;
 DTMUserTag      userTag ;
 DTMFeatureId    userFeatureId ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Copying Data Object To Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dataP     = %p",dataP) ;
    bcdtmWrite_message(0,0,0,"dtmPP     = %p",*dtmPP) ;
   }
/*
** Check For Vaid DTM Data Object
*/
 if( bcdtmObject_testForValidDataObject(dataP)) goto errexit ;
/*
** Write Some Data Object Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dataP->stateFlag   =  %8ld",dataP->stateFlag) ; 
    bcdtmWrite_message(0,0,0,"dataP->numPts      =  %8ld",dataP->numPts) ; 
    bcdtmWrite_message(0,0,0,"dataP->numFeatPts  =  %8ld",dataP->numFeatPts) ; 
   }
/*
** DeSort Data Object If In Sorted State - 
*/
 if( dataP->stateFlag ) if( bcdtmObject_deSortDataObject(dataP)) goto errexit  ;
/*
** Check For Null Dtm Object
*/
 if( *dtmPP != NULL )
   {
    bcdtmWrite_message(2,0,0,"None Null Dtm Object") ;
    goto errexit ;
   }
/*
** Create BC DTM Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Dtm Object") ;
 if( bcdtmObject_createDtmObject(dtmPP)) 
   {
    bcdtmWrite_message(0,0,0,"Error Creating Dtm Object") ;
    goto errexit ;
   }
/*
** Set Point Memory Allocation Parameters
*/
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(*dtmPP,dataP->numPts,(*dtmPP)->incPoints) ;
/*
** Scan Data Object And Populate BC Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Data Object") ;
 for( fCodeP = dataP->featureCodeP ; fCodeP < dataP->featureCodeP + dataP->numFeatPts ; ++fCodeP )
   {
/*
**  Write Stats
*/
    if( dbg )
      {
       if( (*dtmPP)->numFeatures % 1000 == 0  || (*dtmPP)->numPoints % 10000 == 0 )
         {
          bcdtmWrite_message(0,0,0,"Number Of Features = %8ld Number Of Points = %8ld",(*dtmPP)->numFeatures,(*dtmPP)->numPoints) ;
         }
      }
/*
**  Get Start And Next Codes For DTM Feature
*/
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Geopak Feature Code = %4ld",*fCodeP) ;
    if( bcdtmData_getAssociatedDtmFeatureTypeAndCodes(*fCodeP,&dtmFeatureType,&fsCode,&fnCode)) goto errexit ;
/*
**  Ignore Invalid Feature Codes
*/
    if( fsCode > 0 )
      {
/*
**     Scan To End Of DTM Feature 
*/
       nCodeP = fCodeP + 1 ;
       while( nCodeP < dataP->featureCodeP + dataP->numFeatPts && *nCodeP == fnCode ) ++nCodeP ;
       --nCodeP ;
/*
**     Calculate Number Of Points
*/
       numFeaturePts = (long)(nCodeP-fCodeP) + 1 ; 
       if( numFeaturePts == 1 && ( dtmFeatureType != DTMFeatureType::RandomSpots && dtmFeatureType != DTMFeatureType::GroupSpots )) dtmFeatureType = DTMFeatureType::RandomSpots ;
/*
**     Allocate Memory To Store Points
*/
       featurePtsP = ( DPoint3d * ) malloc(numFeaturePts*sizeof(DPoint3d)) ;
       if( featurePtsP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }     
/*
**     Copy Points To Feature Points
*/
       offset = (long)(fCodeP-dataP->featureCodeP) ;
       for ( points1P = featurePtsP , points2P = ( DPoint3d * ) dataP->pointsP + offset ; points1P < featurePtsP + numFeaturePts ; ++points1P , ++points2P )
         {
          *points1P = *points2P ;
         }
/*
**     Set UserTag And Feature Id
*/
       userTag  = DTM_NULL_USER_TAG ;
       userFeatureId = DTM_NULL_FEATURE_ID    ;
       if( dtmFeatureType != DTMFeatureType::RandomSpots )
         {
          if( dataP->userTagP   != NULL ) userTag  = *(dataP->userTagP + offset) ;  
          userFeatureId = (*dtmPP)->dtmFeatureIndex ;
          ++(*dtmPP)->dtmFeatureIndex ;
         } 
//bcdtmWrite_message(0,0,0,"userFeatureId = %10I64d",userFeatureId) ;
/*
**     Store In Dtm Object
*/ 
       if( bcdtmObject_storeDtmFeatureInDtmObject(*dtmPP,dtmFeatureType,userTag,2,&userFeatureId,featurePtsP,numFeaturePts)) goto errexit ;

//dtmFeatureP = ftableAddrP(*dtmPP,(*dtmPP)->numFeatures-1) ;
//bcdtmWrite_message(0,0,0,"dtmFeatureP->dtmFeatureId = %10I64d",dtmFeatureP->dtmFeatureId) ;

/*
**     Free Feature Points Memory
*/
       if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/*
**     Set For Next Feature
*/
       fCodeP = nCodeP ;  
      }
   } 
/*
**  Write Stats
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Features = %8ld Number Of Points = %8ld",(*dtmPP)->numFeatures,(*dtmPP)->numPoints) ;
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/* 
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Data Object To Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Data Object To Dtm Object Error") ;
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
BENTLEYDTM_EXPORT int bcdtmObject_copyTinObjectToDtmObject(DTM_TIN_OBJ *tinP,BC_DTM_OBJ **dtmPP)
/*
** This Function Copies A Tin Object To A Dtm Object
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long ofs,numPartition,flist ;
 DTM_FEATURE_TABLE *featP ;
 BC_DTM_FEATURE    *dtmFeatureP ;
 DTM_TIN_POINT     *pointP,*dtmPointP ;
 DTM_TIN_NODE      *nodeP,*dtmNodeP ;
 DTM_CIR_LIST      *clistP,*dtmClistP ;
 DTM_FEATURE_LIST  *dtmFlistP ;
 DTM_FEATURE_LIST_VER200  *flistP ;
 /*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Tin Object %p To Dtm Object",tinP) ;
/*
** Check For Null Dtm Object
*/
 if( *dtmPP != NULL )
   {
    bcdtmWrite_message(2,0,0,"None Null Dtm Object") ;
    goto errexit ;
   }
/*
** Check For Vaid Tin Object
*/
 if( bcdtmObject_testForValidTinObject(tinP)) goto errexit ;
/*
** Create Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Dtm Object") ;
 if( bcdtmObject_createDtmObject(dtmPP)) 
   {
    bcdtmWrite_message(0,0,0,"Error Creating Dtm Object") ;
    goto errexit ;
   }
/*
** Write Out Tin Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"numPts          = %8ld memPts          = %8ld",tinP->numPts,tinP->memPts) ;
    bcdtmWrite_message(0,0,0,"numNode         = %8ld memNodes        = %8ld",tinP->numPts,tinP->memPts) ;
    bcdtmWrite_message(0,0,0,"numFeatureTable = %8ld memFeatureTable = %8ld",tinP->numFeatureTable,tinP->memFeatureTable) ;
    bcdtmWrite_message(0,0,0,"numFeatureList  = %8ld memFeatureList  = %8ld",tinP->numFeatureList,tinP->memFeatureList) ;
    bcdtmWrite_message(0,0,0,"clistPtr        = %8ld",tinP->cListPtr) ;
    bcdtmWrite_message(0,0,0,"xMin = %12.5lf xMax = %12.5lf xRange = %12.5lf",tinP->xMin,tinP->xMax,tinP->xRange) ;
    bcdtmWrite_message(0,0,0,"yMin = %12.5lf yMax = %12.5lf yRange = %12.5lf",tinP->yMin,tinP->yMax,tinP->yRange) ;
    bcdtmWrite_message(0,0,0,"zMin = %12.5lf zMax = %12.5lf zRange = %12.5lf",tinP->zMin,tinP->zMax,tinP->zRange) ;
   } 
/*
** Copy Tin Object Header To Dtm Object Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Tin Object Header To Dtm Object Header") ;
 (*dtmPP)->dtmObjType           =  BC_DTM_OBJ_TYPE  ;
 (*dtmPP)->dtmObjVersion        =  BC_DTM_OBJ_VERSION ;
 (*dtmPP)->numLines             =  tinP->numLines ;
 (*dtmPP)->numTriangles         =  tinP->numTriangles ;
 (*dtmPP)->numFeatures          =  0 ;
 (*dtmPP)->memFeatures          =  0 ;
 (*dtmPP)->iniFeatures          =  tinP->iniMemFeatureTable ;
 (*dtmPP)->incFeatures          =  tinP->incMemFeatureTable ;
 (*dtmPP)->numFeaturePartitions =  0 ;
 (*dtmPP)->featurePartitionSize =  DTM_PARTITION_SIZE_FEATURE ;
 (*dtmPP)->numPoints            =  0 ;
 (*dtmPP)->memPoints            =  0 ;
 (*dtmPP)->iniPoints            =  tinP->iniMemPts ;
 (*dtmPP)->incPoints            =  tinP->incMemPts ;
 (*dtmPP)->numSortedPoints      =  tinP->numSortedPts ;
 (*dtmPP)->numPointPartitions   =  0 ;
 (*dtmPP)->pointPartitionSize   =  DTM_PARTITION_SIZE_POINT ;
 (*dtmPP)->numNodes             =  0 ;
 (*dtmPP)->memNodes             =  0 ;
 (*dtmPP)->numNodePartitions    =  0 ;
 (*dtmPP)->nodePartitionSize    =  DTM_PARTITION_SIZE_NODE ;
 (*dtmPP)->numClist             =  0 ;
 (*dtmPP)->memClist             =  0 ;
 (*dtmPP)->numClistPartitions   =  0 ;
 (*dtmPP)->clistPartitionSize   =  DTM_PARTITION_SIZE_CLIST ;
 (*dtmPP)->numFlist             =  0 ;
 (*dtmPP)->memFlist             =  0 ;
 (*dtmPP)->iniFlist             =  tinP->iniMemFeatureList ;
 (*dtmPP)->incFlist             =  tinP->incMemFeatureList ;
 (*dtmPP)->numFlistPartitions   =  0 ;
 (*dtmPP)->flistPartitionSize   =  DTM_PARTITION_SIZE_FLIST ;
 (*dtmPP)->dtmState             =  DTMState::Tin ;
 (*dtmPP)->nullPnt              =  tinP->nullPnt ;
 (*dtmPP)->nullPtr              =  tinP->nullPtr ;
 (*dtmPP)->nullUserTag          =  tinP->nullUserTag ;
 (*dtmPP)->dtmFeatureIndex      =  0 ; 
 (*dtmPP)->nullFeatureId        =  DTM_NULL_FEATURE_ID ;
 (*dtmPP)->cListPtr             =  tinP->cListPtr  ;
 if( tinP->cListDelPtr       != tinP->nullPtr ) (*dtmPP)->cListDelPtr =  tinP->cListDelPtr    ;
 else                                           (*dtmPP)->cListDelPtr =  tinP->nullPtr ; 
 if( tinP->featureListDelPtr != tinP->nullPtr ) (*dtmPP)->fListDelPtr =  tinP->featureListDelPtr ;
 else                                           (*dtmPP)->fListDelPtr =  tinP->nullPtr ;
 (*dtmPP)->refCount             =  0 ;
 (*dtmPP)->userStatus           =  tinP->userStatus ;
 (*dtmPP)->creationTime         =  tinP->creationTime ;
 (*dtmPP)->modifiedTime         =  tinP->modifiedTime ;
 (*dtmPP)->hullPoint            =  tinP->hullPnt ;
 (*dtmPP)->nextHullPoint        =  tinP->nextHullPnt ;
 (*dtmPP)->userTime             =  tinP->userTime ;
 (*dtmPP)->ppTol                =  tinP->ppTol ;
 (*dtmPP)->plTol                =  tinP->plTol ; 
 (*dtmPP)->mppTol               =  tinP->mppTol ; 
 (*dtmPP)->xMin                 =  tinP->xMin ;
 (*dtmPP)->yMin                 =  tinP->yMin ;
 (*dtmPP)->zMin                 =  tinP->zMin ;
 (*dtmPP)->xMax                 =  tinP->xMax ;
 (*dtmPP)->yMax                 =  tinP->yMax ;
 (*dtmPP)->zMax                 =  tinP->zMax ;
 (*dtmPP)->xRange               = tinP->xRange ;
 (*dtmPP)->yRange               = tinP->yRange ;
 (*dtmPP)->zRange               = tinP->zRange ;
 (*dtmPP)->fTablePP             = NULL ;
 (*dtmPP)->pointsPP             = NULL ;
 (*dtmPP)->nodesPP              = NULL ;
 (*dtmPP)->cListPP              = NULL ;
 (*dtmPP)->fListPP              = NULL ;
 (*dtmPP)->DTMAllocationClass   = NULL;

/*
** Copy Features
*/
 if( tinP->memFeatureTable > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Features") ;
/*
**  Allocate Memory For Feature Table
*/ 
    (*dtmPP)->iniFeatures = tinP->memFeatureTable ;
    if( bcdtmObject_allocateFeaturesMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Features From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    dtmFeatureP = (*dtmPP)->fTablePP[numPartition] ; 
    for( featP = tinP->fTableP ; featP < tinP->fTableP + tinP->numFeatureTable ; ++featP )
      {
       dtmFeatureP->dtmFeatureType  = featP->dtmFeatureType ;
       dtmFeatureP->dtmUserTag      = featP->userTag  ;
       dtmFeatureP->dtmFeatureId    = (*dtmPP)->dtmFeatureIndex ;
       ++(*dtmPP)->dtmFeatureIndex ;
       dtmFeatureP->dtmFeatureState = DTMFeatureState::Tin ;
       dtmFeatureP->dtmFeaturePts.firstPoint = (*dtmPP)->nullPnt ;
       if( featP->firstPnt != tinP->nullPnt )  dtmFeatureP->dtmFeaturePts.firstPoint = featP->firstPnt ;
       else                                    dtmFeatureP->dtmFeatureState          = DTMFeatureState::Deleted ;
       ++ofs ;
       if( ofs == (*dtmPP)->featurePartitionSize) 
         {
          ++numPartition ;
          dtmFeatureP = (*dtmPP)->fTablePP[numPartition] ;
          ofs = 0 ;
         } 
       else ++dtmFeatureP ;
       ++(*dtmPP)->numFeatures ;
      }  
   }
/*
** Copy Points
*/
  if( tinP->memPts > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Points") ;
/*
**  Allocate Memory For Feature Table
*/ 
    (*dtmPP)->iniPoints = tinP->memPts ;
    if( bcdtmObject_allocatePointsMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Points From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    dtmPointP = (*dtmPP)->pointsPP[numPartition] ; 
    for( pointP = tinP->pointsP ; pointP < tinP->pointsP + tinP->numPts ; ++pointP )
      {
       *(dtmPointP) = *(pointP) ;
       ++ofs ;
       if( ofs == (*dtmPP)->pointPartitionSize) 
         {
          ++numPartition ;
          dtmPointP = (*dtmPP)->pointsPP[numPartition] ;
          ofs = 0 ;
         } 
       else ++dtmPointP ;
       ++(*dtmPP)->numPoints ;
      }  
   }
/*
** Copy Nodes
*/
  if( tinP->memPts > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Nodes") ;
/*
**  Allocate Memory For Nodes
*/ 
    if( bcdtmObject_allocateNodesMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Nodes From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    dtmNodeP = (*dtmPP)->nodesPP[numPartition] ; 
    for( nodeP = tinP->nodesP ; nodeP < tinP->nodesP + tinP->numPts ; ++nodeP )
      {
       *(dtmNodeP) = *(nodeP) ;
       ++ofs ;
       if( ofs == (*dtmPP)->nodePartitionSize) 
         {
          ++numPartition ;
          dtmNodeP = (*dtmPP)->nodesPP[numPartition] ;
          ofs = 0 ;
         } 
       else ++dtmNodeP ;
      }  
   }
/*
** Copy Circular List
*/
  if( tinP->cListPtr > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Circular List") ;
/*
**  Allocate Memory For Circular List
*/ 
    if( bcdtmObject_allocateCircularListMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Circular List From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    dtmClistP = (*dtmPP)->cListPP[numPartition] ; 
    for( clistP = tinP->cListP ; clistP < tinP->cListP + tinP->cListPtr ; ++clistP )
      {
       *(dtmClistP) = *(clistP) ;
       ++ofs ;
       if( ofs == (*dtmPP)->clistPartitionSize) 
         {
          ++numPartition ;
          dtmClistP = (*dtmPP)->cListPP[numPartition] ;
          ofs = 0 ;
         } 
       else ++dtmClistP ;
//       ++(*dtmPP)->numClist ;
      }  
   }
/*
** Copy Feature List
*/
  if( tinP->memFeatureList > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature List") ;
/*
**  Allocate Memory For Feature List
*/ 
    (*dtmPP)->iniFlist = tinP->memFeatureList ;
    if( bcdtmObject_allocateFeatureListMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Feature List From Tin To Dtm
*/
    for( flist = 0 , flistP = tinP->fListP ; flist < tinP->numFeatureList ; ++flist , ++flistP )
      {
       dtmFlistP = flistAddrP((*dtmPP),flist) ;
       dtmFlistP->nextPnt = flistP->nextPnt ;
       dtmFlistP->nextPtr = flistP->nextPtr ;
       dtmFlistP->dtmFeature = flistP->dtmFeature ;
       dtmFlistP->pntType = 1 ;
       ++(*dtmPP)->numFlist ;
      }
/*
    ofs = 0 ;
    numPartition = 0 ;
    dtmFlistP = (*dtmPP)->fListPP[numPartition] ; 
    for( flistP = tinP->fListP ; flistP < tinP->fListP + tinP->numFeatureList ; ++flistP )
      {
       *(dtmFlistP) = *(flistP) ;
       ++ofs ;
       if( ofs == (*dtmPP)->flistPartitionSize) 
         {
          ++numPartition ;
          dtmFlistP = (*dtmPP)->fListPP[numPartition] ;
          ofs = 0 ;
         } 
       else ++dtmFlistP ;
       ++(*dtmPP)->numFlist ;
      }  
*/
   }
/*
** Clean Up
*/
 cleanup :
/* 
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Tin Object %p To Dtm Object Completed",tinP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Tin Object %p To Dtm Object Error",tinP) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_cloneDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **dtmPP)
/*
** This Function Clones A Dtm Object
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long *l1P,*l2P,ofs,num,numPartition ;
 DPoint3d               *p3d1P,*p3d2P ;
 BC_DTM_FEATURE    *featureP,*dtmFeatureP ;
 DTM_TIN_POINT     *pointP,*dtmPointP ;
 DTM_TIN_NODE      *nodeP,*dtmNodeP   ;
 DTM_CIR_LIST      *clistP,*dtmClistP ;
 DTM_FEATURE_LIST  *flistP,*dtmFlistP ;
 /*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cloning Dtm Object %p",dtmP) ;
/*
** Check For Null Dtm Object
*/
 if( *dtmPP != NULL )
   {
    bcdtmWrite_message(2,0,0,"None Null Dtm Object") ;
    goto errexit ;
   }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"dtmState          = %8ld",dtmP->dtmState) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints   = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol       = %20.15lf",dtmP->ppTol) ;
   }
/*
** Create Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Dtm Object") ;
 if( bcdtmObject_createDtmObject(dtmPP)) 
   {
    bcdtmWrite_message(0,0,0,"Error Creating Dtm Object") ;
    goto errexit ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Created Dtm Object %p",*dtmPP) ;
/*
** Copy  Header 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Header") ;
 (*dtmPP)->CopyHeaderDetails (*dtmP) ;
 // If this is an DTMElement then we can't clone unless we make a call to create a new DTMElement,  At the moment change this back to a standand Memory DTM.
 if((*dtmPP)->dtmObjType == BC_DTM_ELM_TYPE) (*dtmPP)->dtmObjType = BC_DTM_OBJ_TYPE;
 (*dtmPP)->refCount    = 0 ; 
 (*dtmPP)->numFeatures = 0 ;
 (*dtmPP)->memFeatures = 0 ;
 (*dtmPP)->numPoints   = 0 ;
 (*dtmPP)->memPoints   = 0 ;
 (*dtmPP)->numNodes    = 0 ;
 (*dtmPP)->memNodes    = 0 ;
 (*dtmPP)->numClist    = 0 ;
 (*dtmPP)->memClist    = 0 ;
 (*dtmPP)->numFlist    = 0 ;
 (*dtmPP)->memFlist    = 0 ;
 (*dtmPP)->refCount    = 0 ;
 (*dtmPP)->pointsPP    = NULL ;
 (*dtmPP)->nodesPP     = NULL ;
 (*dtmPP)->fTablePP    = NULL ;
 (*dtmPP)->fListPP     = NULL ;
 (*dtmPP)->cListPP     = NULL ;
 (*dtmPP)->DTMAllocationClass   = NULL;
 (*dtmPP)->extended = NULL;
/*
** Copy Features
*/
 if( dtmP->memFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Features") ;
/*
**  Allocate Memory For Feature Table
*/ 
    (*dtmPP)->iniFeatures = dtmP->memFeatures ;
    if( bcdtmObject_allocateFeaturesMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Features 
*/
    ofs = 0 ;
    numPartition = 0 ;
    featureP    = dtmP->fTablePP[numPartition] ;
    dtmFeatureP = (*dtmPP)->fTablePP[numPartition] ; 
    for( num = 0 ; num < dtmP->numFeatures ; ++num )
      {
       *dtmFeatureP = *featureP ; 
       switch( dtmFeatureP->dtmFeatureState )
         {
          case DTMFeatureState::PointsArray :
          case DTMFeatureState::TinError    :
          case DTMFeatureState::Rollback     :
          if( featureP->dtmFeaturePts.pointsPI != 0)
            {
            dtmFeatureP->dtmFeaturePts.pointsPI =  bcdtmMemory_allocate(*dtmPP, dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d)) ; 
             if( dtmFeatureP->dtmFeaturePts.pointsPI == 0 )
               {
                bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               }
             p3d1P = bcdtmMemory_getPointerP3D(*dtmPP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
             p3d2P = bcdtmMemory_getPointerP3D(dtmP, featureP->dtmFeaturePts.pointsPI) ;
             while ( p3d1P < bcdtmMemory_getPointerP3D(*dtmPP, dtmFeatureP->dtmFeaturePts.pointsPI) + dtmFeatureP->numDtmFeaturePts )
               {
                *p3d1P = *p3d2P ;
                ++p3d1P ;
                ++p3d2P ;
               }
            } 
          break ;

          case DTMFeatureState::OffsetsArray :
          if( featureP->dtmFeaturePts.offsetPI != 0)
            {
            long* offsetP;
            dtmFeatureP->dtmFeaturePts.offsetPI = bcdtmMemory_allocate(*dtmPP, dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;
             if( dtmFeatureP->dtmFeaturePts.offsetPI == 0 )
               {
                bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               }
             offsetP = bcdtmMemory_getPointerOffset(*dtmPP,dtmFeatureP->dtmFeaturePts.offsetPI);
              l1P = offsetP ;
              l2P = bcdtmMemory_getPointerOffset(dtmP, featureP->dtmFeaturePts.offsetPI) ;
              
             while ( l1P < offsetP + dtmFeatureP->numDtmFeaturePts )
               {
                *l1P = *l2P ;
                ++l1P ;
                ++l2P ;
               }
            } 
          break ; 
         } ;
       ++ofs ;
       if( ofs == (*dtmPP)->featurePartitionSize ) 
         {
          ++numPartition ;
          featureP    = dtmP->fTablePP[numPartition] ;
          dtmFeatureP = (*dtmPP)->fTablePP[numPartition] ;
          ofs = 0 ;
         } 
       else 
         {
          ++featureP ;
          ++dtmFeatureP ;
         }
       ++(*dtmPP)->numFeatures ;
      }  
   }
/*
** Copy Points
*/
  if( dtmP->memPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Points") ;
/*
**  Allocate Memory For Feature Table
*/ 
    (*dtmPP)->iniPoints = dtmP->memPoints ;
    if( bcdtmObject_allocatePointsMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Points 
*/
    ofs = 0 ;
    numPartition = 0 ;
    pointP    = dtmP->pointsPP[numPartition] ; 
    dtmPointP = (*dtmPP)->pointsPP[numPartition] ; 
    for( num = 0 ; num < dtmP->numPoints ; ++num )
      {
       *(dtmPointP) = *(pointP) ;
       ++ofs ;
       if( ofs == (*dtmPP)->pointPartitionSize) 
         {
          ++numPartition ;
          pointP    = dtmP->pointsPP[numPartition] ; 
          dtmPointP = (*dtmPP)->pointsPP[numPartition] ;
          ofs = 0 ;
         } 
       else 
         {
          ++pointP ;
          ++dtmPointP ;
         } 
       ++(*dtmPP)->numPoints ;
      }  
   }
/*
** Copy Nodes
*/
  if( dtmP->nodesPP != NULL && dtmP->memPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Nodes") ;
/*
**  Allocate Memory For Feature Table
*/ 
    if( bcdtmObject_allocateNodesMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Nodes From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    nodeP    =  dtmP->nodesPP[numPartition] ; 
    dtmNodeP = (*dtmPP)->nodesPP[numPartition] ; 
    for( num = 0 ; num < dtmP->numPoints ; ++num )
      {
       *(dtmNodeP) = *(nodeP) ;
       ++ofs ;
       if( ofs == (*dtmPP)->nodePartitionSize) 
         {
          ++numPartition ;
          nodeP    =  dtmP->nodesPP[numPartition] ; 
          dtmNodeP = (*dtmPP)->nodesPP[numPartition] ;
          ofs = 0 ;
         } 
       else 
         {
          ++nodeP  ; 
          ++dtmNodeP ;
         }
       ++(*dtmPP)->numNodes ;
      }  
   }
/*
** Copy Circular List
*/
  if( dtmP->cListPP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Circular List") ;
/*
**  Allocate Memory For Feature Table
*/ 
    if( bcdtmObject_allocateCircularListMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Circular List From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    clistP    =  dtmP->cListPP[numPartition] ; 
    dtmClistP = (*dtmPP)->cListPP[numPartition] ; 
    for( num = 0 ; num < dtmP->cListPtr ; ++num )
      {
       *(dtmClistP) = *(clistP) ;
       ++ofs ;
       if( ofs == (*dtmPP)->clistPartitionSize) 
         {
          ++numPartition ;
          clistP    =  dtmP->cListPP[numPartition] ; 
          dtmClistP = (*dtmPP)->cListPP[numPartition] ;
          ofs = 0 ;
         } 
       else
         {
          ++clistP ; 
          ++dtmClistP ;
         }
       ++(*dtmPP)->numClist ;
      }  
   }
/*
** Copy Feature List
*/
  if( dtmP->fListPP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature List") ;
/*
**  Allocate Memory For Feature Table
*/ 
    (*dtmPP)->iniFlist = dtmP->memFlist ;
    if( bcdtmObject_allocateFeatureListMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Feature List From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    flistP    =  dtmP->fListPP[numPartition] ; 
    dtmFlistP = (*dtmPP)->fListPP[numPartition] ; 
    for( num = 0 ; num < dtmP->numFlist ; ++num )
      {
       *(dtmFlistP) = *(flistP) ;
       ++ofs ;
       if( ofs == (*dtmPP)->flistPartitionSize) 
         {
          ++numPartition ;
          flistP    =  dtmP->fListPP[numPartition] ; 
          dtmFlistP = (*dtmPP)->fListPP[numPartition] ;
          ofs = 0 ;
         } 
       else
         {
          ++flistP ;
          ++dtmFlistP ;
         } 
       ++(*dtmPP)->numFlist ;
      }  
   }
/*
** Clean Up
*/
 cleanup :
/* 
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cloning Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cloning Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_appendDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P)
/*
** This Function Appends DTM2 (dtmP2) to DTM1 (dtm1P)
** It Does This By Copying The DTM Features From DTM2 To DTM1
** A Hull Feature Is Only Appended If One Does Not Exist In DTM1
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long sPnt,nPnt,fPnt,lPnt,dtmFeature,numFeaturePts,hullPresent=FALSE ;
 unsigned char *pointMarkP=NULL ;
 DPoint3d  *featurePtsP=NULL ;
 DTM_TIN_POINT   *pointP ;
 BC_DTM_FEATURE  *dtmFeatureP = NULL;
 DTMFeatureId  featureId ;
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
 if( pointMarkP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Scan DTM1 Features For Presence Of A Hull Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Hull Feature") ;
 hullPresent = FALSE ;
 for( dtmFeature = 0 ; dtmFeature < dtm1P->numFeatures && hullPresent == FALSE ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtm1P,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError )
      {
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull ) hullPresent = TRUE ;
      }
   }
/*
** Scan DTM 2 Features And Store In DTM1
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Appending DTM2 Features To DTM1") ;
 for( dtmFeature = 0 ; dtmFeature < dtm2P->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtm2P,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
      {
/*
**     Check For Hull Feature
*/
       if( dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull || ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull && hullPresent == FALSE ))
         {  
/*
**        Store DTM2 Feature In DTM1
*/
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtm2P,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,3,&featureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
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
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
 for( sPnt = 0 ; sPnt < dtm2P->numPoints ; ++sPnt )
   {
    if( ! bcdtmFlag_testFlag(pointMarkP,sPnt))
      {
/*
**     Check Memory
*/
       if( featurePtsP == NULL )
         {
          featurePtsP = ( DPoint3d * ) malloc( 1000 * sizeof(DPoint3d)) ;
          if( featurePtsP == NULL )
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
          DTMUserTag userTag = DTM_NULL_USER_TAG;
          if (dtmFeatureP != NULL) userTag = dtmFeatureP->dtmUserTag;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,DTMFeatureType::RandomSpots,userTag,1,&featureId,featurePtsP,numFeaturePts)) goto errexit ;
          numFeaturePts = 0 ;
         }
      }
   }
/*
** Store Remaining Random Spots In DTM1
*/
 if( numFeaturePts > 0 )
   {
    DTMUserTag userTag = DTM_NULL_USER_TAG;
    if (dtmFeatureP != NULL) userTag = dtmFeatureP->dtmUserTag;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,DTMFeatureType::RandomSpots,userTag,3,&featureId,featurePtsP,numFeaturePts)) goto errexit ;
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
 if( pointMarkP  != NULL ) { free(pointMarkP)  ; pointMarkP = NULL  ; }
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
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
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_appendWithUsertagDtmObject
(
 BC_DTM_OBJ *dtm1P,
 BC_DTM_OBJ *dtm2P,
 DTMUserTag userTag
 )
/*
** This Function Appends DTM2 (dtmP2) to DTM1 (dtm1P)
** It Does This By Copying The DTM Features From DTM2 To DTM1
** A Hull Feature Is Only Appended If One Does Not Exist In DTM1
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long sPnt,nPnt,fPnt,lPnt,dtmFeature,numFeaturePts,hullPresent=FALSE ;
 unsigned char *pointMarkP=NULL ;
 DPoint3d  *featurePtsP=NULL ;
 DTM_TIN_POINT   *pointP ;
 BC_DTM_FEATURE  *dtmFeatureP = NULL;
 DTMFeatureId  featureId ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Appending With User Tag DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtm1P   = %p",dtm1P) ;
    bcdtmWrite_message(0,0,0,"dtm2P   = %p",dtm2P) ;
    bcdtmWrite_message(0,0,0,"userTag = %10I64d",userTag) ;
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
 if( pointMarkP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Scan DTM1 Features For Presence Of A Hull Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Hull Feature") ;
 hullPresent = FALSE ;
 for( dtmFeature = 0 ; dtmFeature < dtm1P->numFeatures && hullPresent == FALSE ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtm1P,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError  && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
      {
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull ) hullPresent = TRUE ;
      }
   }
/*
** Scan DTM 2 Features And Store In DTM1
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Appending DTM2 Features To DTM1") ;
 for( dtmFeature = 0 ; dtmFeature < dtm2P->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtm2P,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError )
      {
/*
**     Check For Hull Feature
*/
       if( dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull || ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull && hullPresent == FALSE ))
         {  
/*
**        Store DTM2 Feature In DTM1
*/
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtm2P,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,dtmFeatureP->dtmFeatureType,userTag,3,&featureId,featurePtsP,numFeaturePts)) goto errexit ;
          if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
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
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
 for( sPnt = 0 ; sPnt < dtm2P->numPoints ; ++sPnt )
   {
    if( ! bcdtmFlag_testFlag(pointMarkP,sPnt))
      {
/*
**     Check Memory
*/
       if( featurePtsP == NULL )
         {
          featurePtsP = ( DPoint3d * ) malloc( 1000 * sizeof(DPoint3d)) ;
          if( featurePtsP == NULL )
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
          DTMUserTag userTag = DTM_NULL_USER_TAG;
          if (dtmFeatureP != NULL) userTag = dtmFeatureP->dtmUserTag;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,DTMFeatureType::RandomSpots,userTag,1,&featureId,featurePtsP,numFeaturePts)) goto errexit ;
          numFeaturePts = 0 ;
         }
      }
   }
/*
** Store Remaining Random Spots In DTM1
*/
 if( numFeaturePts > 0 )
   {
    DTMUserTag userTag = DTM_NULL_USER_TAG;
    if (dtmFeatureP != NULL) userTag = dtmFeatureP->dtmUserTag;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtm1P,DTMFeatureType::RandomSpots,userTag,3,&featureId,featurePtsP,numFeaturePts)) goto errexit ;
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
 if( pointMarkP  != NULL ) { free(pointMarkP)  ; pointMarkP = NULL  ; }
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
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
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_countNumberOfDtmFeatureTypeOccurrencesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,long *numOccurencesP) 
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long feature,partNum,partOfs ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Counting Number Of Feature Type Occurrences") ;
/*
** Initialise
*/
 *numOccurencesP = 0 ;
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
       if( dtmFeatureP->dtmFeatureType == dtmFeatureType ) ++*numOccurencesP ;
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
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Counting Number Of Feature Type Occurrences Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Counting Number Of Feature Type Occurrences Error") ;
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_getPointsForDtmFeatureDtmObject
(
 BC_DTM_OBJ    *dtmP,
 long          dtmFeature,
 DTM_TIN_POINT **featPtsPP,
 long          *numFeatPtsP
) 
/*
** This Function Writes Points For A Dtm Feature 
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,listPtr,nextPnt=0,firstPnt,point ;
// char dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_TIN_POINT  *pntP,*pointP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Points For Dtm Feature = %8ld",dtmFeature) ;
/*
** Initialise
*/
 *numFeatPtsP = 0 ;
 if( *featPtsPP != NULL ) { free(*featPtsPP) ; *featPtsPP = NULL ; }
/*
** Validate
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature Range Error") ;
    goto errexit ;
   }
/*
** Set Feature Address
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**  Ignore Deleted Features
*/
 if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted )
   {
/*
**  Count Number Of Feature Points For Feature In Tin State 
*/
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       bcdtmList_countNumberOfPointsForDtmTinFeatureDtmObject(dtmP,dtmFeature,numFeatPtsP) ;
      }
    else  *numFeatPtsP = dtmFeatureP->numDtmFeaturePts ;
/*
**  Allocate memory To Store Feature Points
*/
    *featPtsPP = ( DTM_TIN_POINT * ) malloc( *numFeatPtsP * sizeof( DTM_TIN_POINT )) ;
    if( *featPtsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Method To Write Points Is Dependent On The Dtm Feature State
*/
    switch( dtmFeatureP->dtmFeatureState )
      {
/*
**     Get Points From DTM Points Array
*/ 
       case DTMFeatureState::Data : 
       for( n = 0 , pntP = *featPtsPP ; n < dtmFeatureP->numDtmFeaturePts ; ++n , ++pntP )
         {
          point  = dtmFeatureP->dtmFeaturePts.firstPoint+n ;
          pointP = pointAddrP(dtmP,point) ;
          *pntP  = *pointP ;
         } 
       break ;
/*
**     Get Points From Feature Points Array
*/ 
       case DTMFeatureState::PointsArray : 
       case DTMFeatureState::TinError    : 
       case DTMFeatureState::Rollback     : 
          memcpy(*featPtsPP,bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts*sizeof(DTM_TIN_POINT)) ;
       break ;
/*
**     Get Points From Point Offset Array
*/ 
       case DTMFeatureState::OffsetsArray : 
       for( n = 0 , pntP = *featPtsPP ; n < dtmFeatureP->numDtmFeaturePts ; ++n , ++pntP )
         {
          point  = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[n] ;
          pointP = pointAddrP(dtmP,point) ;
          *pntP  = *pointP ;
         } 
       break ;
/*
**     Get Points From Tin
*/ 
       case DTMFeatureState::Tin  :       // Dtm Feature In Tin 
       if( ( firstPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
         { 
/*
**        Initialise Scan Variables
*/
          pntP   = *featPtsPP ;
          pointP = pointAddrP(dtmP,firstPnt) ;
          *pntP  = *pointP ;
          ++pntP ;
/*
**        Scan Dtm Feature List Pointers
*/
          listPtr  = nodeAddrP(dtmP,firstPnt)->fPtr ;
          while ( listPtr != dtmP->nullPtr )
            {
             while ( listPtr != dtmP->nullPtr  && flistAddrP(dtmP,listPtr)->dtmFeature != dtmFeature ) listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
             if( listPtr != dtmP->nullPtr )
               {
                nextPnt = flistAddrP(dtmP,listPtr)->nextPnt ;
                if( nextPnt != dtmP->nullPnt )
                  {
                   pointP = pointAddrP(dtmP,nextPnt) ;
                   *pntP  = *pointP ;
                   ++pntP ;
                   listPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
                  } 
                if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) listPtr = dtmP->nullPtr ;
               }
            } 
         }
       break ;
/*
**     Default 
*/
       default :
       bcdtmWrite_message(2,0,0,"Unknown Dtm Feature State %2ld Not Yet Implemented",ftableAddrP(dtmP,dtmFeature)->dtmFeatureState) ;
       goto errexit ; 
       break ;
      } ;
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
 BENTLEYDTM_EXPORT int bcdtmObject_getPointByIndexDtmObject(BC_DTM_OBJ *dtmP, long index, DTM_TIN_POINT* pt)
 /*
 ** This Function returns the point at an index.
 */
 {
 int  ret=DTM_SUCCESS;
/*
** Check For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
 if( index < 0 || index >= dtmP->numPoints) goto errexit;
 *pt = *pointAddrP(dtmP, index);
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
BENTLEYDTM_Public int bcdtmObject_getPointOffsetsForDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long **featPtsPP,long *numFeatPtsP) 
/*
** This Function Writes Points For A Dtm Feature 
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,listPtr,nextPnt=0,firstPnt ;
 long *pntP   ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Points For Dtm Feature = %8ld",dtmFeature) ;
/*
** Initialise
*/
 *numFeatPtsP = 0 ;
 if( *featPtsPP != NULL ) { free(*featPtsPP) ; *featPtsPP = NULL ; }
/*
** Validate
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
   {
    bcdtmWrite_message(2,0,0,"Dtm Feature Range Error") ;
    goto errexit ;
   }
/*
** Set Feature Address
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**  Ignore Deleted Features
*/
 if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted )
   {
/*
**  Count Number Of Feature Points For Feature In Tin State 
*/
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       bcdtmList_countNumberOfPointsForDtmTinFeatureDtmObject(dtmP,dtmFeature,numFeatPtsP) ;
      }
    else
      *numFeatPtsP = dtmFeatureP->numDtmFeaturePts ;
/*
**  Allocate memory To Store Feature Points
*/
    *featPtsPP = ( long * ) malloc( *numFeatPtsP * sizeof( long )) ;
    if( *featPtsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Method To Write Points Is Dependent On The Dtm Feature State
*/
    switch( dtmFeatureP->dtmFeatureState )
      {
/*
**     Get Points From DTM Points Array
*/ 
       case DTMFeatureState::Data : 
       for( n = 0 , pntP = *featPtsPP ; n < dtmFeatureP->numDtmFeaturePts ; ++n , ++pntP )
         {
          *pntP = dtmFeatureP->dtmFeaturePts.firstPoint+n ;
         } 
       break ;
/*
**     Get Points From Feature Points Array
*/ 
       case DTMFeatureState::PointsArray : 
       case DTMFeatureState::TinError    : 
       case DTMFeatureState::Rollback     :
       break ;
/*
**     Get Points From Point Offset Array
*/ 
       case DTMFeatureState::OffsetsArray : 
           {
           long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
       for( n = 0 , pntP = *featPtsPP ; n < dtmFeatureP->numDtmFeaturePts ; ++n , ++pntP )
         {
          *pntP  = offsetP[n] ;
         } 
           }
       break ;
/*
**     Get Points From Tin
*/ 
       case DTMFeatureState::Tin  :       // Dtm Feature In Tin 
       if( ( firstPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
         { 
/*
**        Initialise Scan Variables
*/
          pntP  = *featPtsPP ;
          *pntP = firstPnt ;
          ++pntP ;
/*
**        Scan Dtm Feature List Pointers
*/
          listPtr  = nodeAddrP(dtmP,firstPnt)->fPtr ;
          while ( listPtr != dtmP->nullPtr )
            {
             while ( listPtr != dtmP->nullPtr  && flistAddrP(dtmP,listPtr)->dtmFeature != dtmFeature ) listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
             if( listPtr != dtmP->nullPtr )
               {
                nextPnt = flistAddrP(dtmP,listPtr)->nextPnt ;
                if( nextPnt != dtmP->nullPnt )
                  {
                   *pntP  = nextPnt ;
                   listPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
                  } 
                if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) listPtr = dtmP->nullPtr ;
               }
            } 
         }
       break ;
/*
**     Default 
*/
       default :
       bcdtmWrite_message(2,0,0,"Unknown Dtm Feature State %2ld Not Yet Implemented",ftableAddrP(dtmP,dtmFeature)->dtmFeatureState) ;
       goto errexit ; 
       break ;
      } ;
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
BENTLEYDTM_EXPORT int bcdtmObject_setToDTMElement (BC_DTM_OBJ *dtmP, void* allocator)
/*
** This call sets the DTMElement memory allocator in the dtm header.
*/
    {
    if (dtmP->dtmObjType == BC_DTM_OBJ_TYPE && nullptr == allocator)
        {
        BeAssert (false);
        return DTM_ERROR;
        }
    dtmP->dtmObjType = BC_DTM_ELM_TYPE;
    dtmP->DTMAllocationClass = (IDTMElementMemoryAllocator*)allocator;
    return DTM_SUCCESS;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_assignFeatureIdsDtmObject(BC_DTM_OBJ *dtmP )
/*
** This Function Assigns Feature Ids To Features With Null Feature Ids
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFeature ;
 BC_DTM_FEATURE  *dtmFeatureP = NULL;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Assigning Feature Ids DTM Object") ;
    bcdtmWrite_message(0,0,0,"dtmP   = %p",dtmP) ;
   }
/*
** Check For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Scan DTM Features And Set Feature Ids
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError  && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
      {
       if( dtmFeatureP->dtmFeatureId == DTM_NULL_FEATURE_ID )
         {
          dtmFeatureP->dtmFeatureId = dtmP->dtmFeatureIndex ;
          ++dtmP->dtmFeatureIndex ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Assigning Feature Ids DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Assigning Feature Ids DTM Object Error") ;
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
BENTLEYDTM_EXPORT int bcdtmObject_copyToMemoryBlockDtmObject( BC_DTM_OBJ *dtmP, char **memoryBlockPP , unsigned long *memoryBlockSizeP ) 
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  dtmFeature,point,node,clist,flist,*offsetP ;
 DPoint3d   *pntsP ;
 BC_DTM_FEATURE   *dtmFeatureP ;
 DTM_FEATURE_LIST *flistP ;
 DTM_TIN_NODE     *nodeP ;
 DTM_CIR_LIST     *clistP ;
 DTM_TIN_POINT    *pointP ;  
 unsigned long memPos=0,headMemory,featureMemory,featurePtsMemory,pointsMemory,nodesMemory,clistMemory,flistMemory,totalMemory ;
/*
**  Write Enty Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Copying DTM To Memory Block") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"memoryBlockPP   = %p",*memoryBlockPP) ;
    bcdtmWrite_message(0,0,0,"memoryBlockSize = %8ld",*memoryBlockSizeP) ;
   }
/*
** Initialise
*/
 *memoryBlockSizeP = 0 ;
 if( *memoryBlockPP != NULL ) { free(*memoryBlockPP) ; *memoryBlockPP = NULL ; }
/*
** Check For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Get Memory Used By DTM Object
*/
 if( bcdtmObject_reportMemoryUsageDtmObject(dtmP,&headMemory,&featureMemory,&featurePtsMemory,&pointsMemory,&nodesMemory,&clistMemory,&flistMemory,&totalMemory) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Object %p Memory Size = %10u",dtmP,totalMemory) ;
/*
** Allocate Memory
*/
 *memoryBlockSizeP = totalMemory  ;
 *memoryBlockPP    = ( char * ) malloc( *memoryBlockSizeP * sizeof(char)) ;
 if( *memoryBlockPP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Copy Dtm Header To Memory Block
*/
 memPos = 0 ;
 memcpy(*memoryBlockPP+memPos,dtmP,BCDTMSize) ;
 memPos = memPos + BCDTMSize ;
/*
** Copy Features
*/
 if( dtmP->numFeatures > 0 )
   {
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
/*
**     Copy Feature
*/
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       memcpy(*memoryBlockPP+memPos,dtmFeatureP,sizeof(BC_DTM_FEATURE)) ;
       memPos = memPos + sizeof(BC_DTM_FEATURE) ;
/*
**     Copy Feature Points Depending On Feature State
*/
       switch( dtmFeatureP->dtmFeatureState )
         {
          case DTMFeatureState::PointsArray :
          case DTMFeatureState::TinError    :
          case DTMFeatureState::Rollback     :
          if( dtmFeatureP->dtmFeaturePts.pointsPI != 0)
            { 
             pntsP = bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
             memcpy(*memoryBlockPP+memPos,pntsP,dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d)) ;
             memPos = memPos + dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d) ;
            }
          break ;

          case DTMFeatureState::OffsetsArray :
          if( dtmFeatureP->dtmFeaturePts.offsetPI != 0)
            {
             offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
             memcpy(*memoryBlockPP+memPos,offsetP,dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;
             memPos = memPos + dtmFeatureP->numDtmFeaturePts*sizeof(long) ;
            }
          break ;

          default :
          break   ; 

         } ;
      }  
   }
/*
** Copy Points
*/
 if( dtmP->numPoints > 0 )
   {
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       pointP = pointAddrP(dtmP,point) ;
       memcpy(*memoryBlockPP+memPos,pointP,sizeof(DPoint3d)) ;
       memPos = memPos + sizeof(DPoint3d) ;
      } 
   }
/*
** Copy Nodes
*/
 if( dtmP->numNodes > 0 )
   {
    for( node = 0 ; node < dtmP->numNodes ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       memcpy(*memoryBlockPP+memPos,nodeP,sizeof(DTM_TIN_NODE)) ;
       memPos = memPos + sizeof(DTM_TIN_NODE) ;
      } 
   }
/*
** Copy Circular List
*/
 if( dtmP->cListPP != NULL )
   {
    for( clist = 0 ; clist < dtmP->cListPtr ; ++clist )
      {
       clistP = clistAddrP(dtmP,clist) ;
       memcpy(*memoryBlockPP+memPos,clistP,sizeof(DTM_CIR_LIST)) ;
       memPos = memPos + sizeof(DTM_CIR_LIST) ;
      }
   }
/*
** Copy Feature List
*/
 if( dtmP->numFlist > 0 )
   {
    for( flist = 0 ; flist < dtmP->numFlist ; ++flist )
      {
       flistP = flistAddrP(dtmP,flist) ;
       memcpy(*memoryBlockPP+memPos,flistP,sizeof(DTM_FEATURE_LIST)) ;
       memPos = memPos + sizeof(DTM_FEATURE_LIST) ;
      } 
   }
/*
**  Write Memory Bytes Copied
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"*memoryBlockSizeP = %10u  ** memPos = %10u",*memoryBlockSizeP,memPos) ;
/*
** Realloc Memory
*/
 if( memPos < *memoryBlockSizeP )
   {
    *memoryBlockPP = ( char * ) realloc(*memoryBlockPP,memPos*sizeof(char)) ;
    *memoryBlockSizeP = memPos ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM To Memory Block Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM To Memory Block Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *memoryBlockSizeP = 0 ;
 if( *memoryBlockPP != NULL ) { free(*memoryBlockPP) ; *memoryBlockPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_createFromMemoryBlockDtmObject( BC_DTM_OBJ **dtmPP, char *memoryBlockP , unsigned long memoryBlockSize ) 
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  saveIni,dtmObjectType,dtmObjectVersion ;
 long  dtmFeature,point,node,clist,flist,*offsetP ;
 DPoint3d              *pntsP ;
 BC_DTM_FEATURE   *dtmFeatureP ;
 DTM_FEATURE_LIST *flistP ;
 DTM_TIN_NODE     *nodeP ;
 DTM_CIR_LIST     *clistP ;
 DTM_TIN_POINT    *pointP ;  
 unsigned long    memPos=0 ;
/*
**  Write Enty Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating DTM From Memory Block") ;
    bcdtmWrite_message(0,0,0,"*dtmPP          = %p",*dtmPP) ;
    bcdtmWrite_message(0,0,0,"memoryBlockP    = %p",memoryBlockP) ;
    bcdtmWrite_message(0,0,0,"memoryBlockSize = %8ld",memoryBlockSize) ;
   }
/*
** Check Object Type And Version
*/
 if( memoryBlockSize < BCDTMSize )
   {
    bcdtmWrite_message(1,0,0,"Not A DTM Object Memory Block") ;
    goto errexit ;
   }
 memcpy(&dtmObjectType,memoryBlockP,4) ;
 memcpy(&dtmObjectVersion,memoryBlockP+4,4) ;
 if( dtmObjectType != BC_DTM_OBJ_TYPE ) 
   {
    bcdtmWrite_message(1,0,0,"Not A DTM Object Memory Block") ;
    goto errexit ;
   }
 if( dtmObjectVersion != BC_DTM_OBJ_VERSION ) 
   {
    bcdtmWrite_message(1,0,0,"Invalid DTM Object Memory Block Version") ;
    goto errexit ;
   }
 if( *dtmPP != NULL )
   {
    bcdtmWrite_message(1,0,0,"Method Requires NULL DTM Pointer") ;
    goto errexit ;
   }
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(dtmPP)) goto errexit ;
/*
** Copy Header To DTM Object
*/
 memcpy(*dtmPP,memoryBlockP,BCDTMSize) ;
 memPos = BCDTMSize ;
/*
** Copy Features
*/
 if( (*dtmPP)->memFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Features From Memory Block ** MemFeatures = %8ld",(*dtmPP)->memFeatures) ;
/*
**  Allocate Memory For Feature Table
*/ 
     saveIni = (*dtmPP)->iniFeatures ;
    (*dtmPP)->iniFeatures = (*dtmPP)->memFeatures ;
    (*dtmPP)->memFeatures = 0 ;
    if( bcdtmObject_allocateFeaturesMemoryDtmObject(*dtmPP)) goto errexit ;
    (*dtmPP)->iniFeatures = saveIni ;
/*
**  Copy Features From Memory Block To DTM Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Features From Memory Block") ;
    for( dtmFeature = 0 ; dtmFeature < (*dtmPP)->numFeatures ; ++dtmFeature )
      {
/*
**     Copy Feature
*/
       dtmFeatureP = ftableAddrP(*dtmPP,dtmFeature) ;
       memcpy(dtmFeatureP,memoryBlockP+memPos,sizeof(BC_DTM_FEATURE)) ;
       memPos = memPos + sizeof(BC_DTM_FEATURE) ;
/*
**     Copy Feature Points Depending On Feature State
*/
       switch( dtmFeatureP->dtmFeatureState )
         {
          case DTMFeatureState::PointsArray :
          case DTMFeatureState::TinError    :
          case DTMFeatureState::Rollback     :
          if( dtmFeatureP->dtmFeaturePts.pointsPI != 0)
            { 
             pntsP = bcdtmMemory_getPointerP3D(*dtmPP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
             memcpy(pntsP,memoryBlockP+memPos,dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d)) ;
             memPos = memPos + dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d) ;
            }
          break ;

          case DTMFeatureState::OffsetsArray :
          if( dtmFeatureP->dtmFeaturePts.offsetPI != 0)
            {
             offsetP = bcdtmMemory_getPointerOffset(*dtmPP,dtmFeatureP->dtmFeaturePts.offsetPI);
             memcpy(offsetP,memoryBlockP+memPos,dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;
             memPos = memPos + dtmFeatureP->numDtmFeaturePts*sizeof(long) ;
            }
          break ;

          default :
          break   ; 

         } ;
      }  
   }
/*
** Copy Points
*/
  if( (*dtmPP)->memPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Points") ;
/*
**  Allocate Memory For Points Table
*/ 
    saveIni = (*dtmPP)->iniPoints ;
    (*dtmPP)->iniPoints = (*dtmPP)->memPoints ;
    (*dtmPP)->memPoints = 0 ;
    if( bcdtmObject_allocatePointsMemoryDtmObject(*dtmPP)) goto errexit ;
    (*dtmPP)->iniPoints = saveIni ;
/*
**  Copy Points
*/
    for( point = 0 ; point < (*dtmPP)->numPoints ; ++point )
      {
       pointP = pointAddrP(*dtmPP,point) ;
       memcpy(pointP,memoryBlockP+memPos,sizeof(DPoint3d)) ;
       memPos = memPos + sizeof(DPoint3d) ;
      } 
   }
/*
** Copy Nodes
*/
  if( (*dtmPP)->nodesPP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Nodes") ;
/*
**  Allocate Memory For Nodes Table
*/ 
    saveIni = (*dtmPP)->iniPoints ;
    (*dtmPP)->iniPoints = (*dtmPP)->memPoints ;
    (*dtmPP)->memNodes = 0 ;
    if( bcdtmObject_allocateNodesMemoryDtmObject(*dtmPP)) goto errexit ;
    (*dtmPP)->iniPoints = saveIni ;
/*
**  Copy Nodes
*/
    for( node = 0 ; node < (*dtmPP)->numPoints ; ++node )
      {
       nodeP = nodeAddrP(*dtmPP,node) ;
       memcpy(nodeP,memoryBlockP+memPos,sizeof(DTM_TIN_NODE)) ;
       memPos = memPos + sizeof(DTM_TIN_NODE) ;
      } 
   }
/*
** Copy Circular List
*/
  if( (*dtmPP)->cListPP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Circular List") ;
/*
**  Allocate Memory For Circular List
*/ 
    (*dtmPP)->memClist = 0 ;
    if( bcdtmObject_allocateCircularListMemoryDtmObject(*dtmPP)) goto errexit ;
/*
**  Copy Circular List 
*/
    for( clist = 0 ; clist < (*dtmPP)->cListPtr ; ++clist )
      {
       clistP = clistAddrP(*dtmPP,clist) ;
       memcpy(clistP,memoryBlockP+memPos,sizeof(DTM_CIR_LIST)) ;
       memPos = memPos + sizeof(DTM_CIR_LIST) ;
      }
   }
/*
** Copy Feature List
*/
 if( (*dtmPP)->memFlist > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature List") ;
/*
**  Allocate Memory For Feature List Table
*/ 
    saveIni = (*dtmPP)->iniFlist ;
    (*dtmPP)->iniFlist = (*dtmPP)->memFlist ;
    (*dtmPP)->memFlist  = 0 ;
    if( bcdtmObject_allocateFeatureListMemoryDtmObject(*dtmPP)) goto errexit ;
    (*dtmPP)->iniFlist = saveIni ;
/*
**  Copy Feature List
*/
    for( flist = 0 ; flist < (*dtmPP)->numFlist ; ++flist )
      {
       flistP = flistAddrP(*dtmPP,flist) ;
       memcpy(flistP,memoryBlockP+memPos,sizeof(DTM_FEATURE_LIST)) ;
       memPos = memPos + sizeof(DTM_FEATURE_LIST) ;
      } 
   }
/*
**  Write Memory Bytes Copied
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"*memoryBlockSize = %10u  ** memPos = %10u",memoryBlockSize,memPos) ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating DTM From Memory Block Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating DTM From Memory Block Error") ;
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
BENTLEYDTM_EXPORT int bcdtmObject_reportStatisticsDtmObject(BC_DTM_OBJ *dtmP)
{
 int ret=DTM_SUCCESS ;
 long n,m,dtmFeature,featuresFound,featureCounts[1000]={10000*0} ;
 long numFeaturePoints=0,numRandomPoints=0 ;
 char dtmStateName[100],dtmFeatureTypeName[100],dtmFeatureStateName[100] ;
 long numFeaturePts,reportFeatures=FALSE ;
 DPoint3d  *p3dP,*featurePtsP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Check For Valid DTM Object
*/
 bcdtmWrite_message(0,0,0,"*********** Reporting Statistics DTM Object = %p",dtmP) ;
 if( bcdtmObject_testForValidDtmObject(dtmP))
   {
    bcdtmWrite_message(0,0,0,"Not A Valid DTM Object") ;
    goto cleanup ;
   } 
/*
** Set DTM State
*/
  if     ( dtmP->dtmState == DTMState::Data                )   strcpy(dtmStateName,"DTMState::Data") ; 
  else if( dtmP->dtmState == DTMState::PointsSorted       )   strcpy(dtmStateName,"DTMState::PointsSorted") ;
  else if( dtmP->dtmState == DTMState::DuplicatesRemoved  )   strcpy(dtmStateName,"DTMState::DuplicatesRemoved") ;
  else if( dtmP->dtmState == DTMState::Tin                 )   strcpy(dtmStateName,"DTMState::Tin") ;
  else if( dtmP->dtmState == DTMState::TinError           )   strcpy(dtmStateName,"DTMState::TinError") ; 
  else                                                         strcpy(dtmStateName,"DTMState::INVALID") ; 

 if     ( dtmP->dtmObjType == BC_DTM_OBJ_TYPE ) bcdtmWrite_message(0,0,0,"DTM Type     = BC_DTM_OBJ_TYPE") ;
 else if( dtmP->dtmObjType == BC_DTM_ELM_TYPE ) bcdtmWrite_message(0,0,0,"DTM Type     = BC_DTM_ELM_TYPE") ;
 bcdtmWrite_message(0,0,0,"Number Of DTM Points     = %8ld",dtmP->numPoints) ;
 bcdtmWrite_message(0,0,0,"x ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->xMin,dtmP->xMax,dtmP->xMax-dtmP->xMin) ;
 bcdtmWrite_message(0,0,0,"y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->yMin,dtmP->yMax,dtmP->yMax-dtmP->yMin) ;
 bcdtmWrite_message(0,0,0,"z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->zMin,dtmP->zMax,dtmP->zMax-dtmP->zMin) ;
 bcdtmWrite_message(0,0,0,"DTM                   = %p",dtmP) ;   
 bcdtmWrite_message(0,0,0,"DTM State             = %s",dtmStateName) ;   
 bcdtmWrite_message(0,0,0,"DTM Extended          = %p",dtmP->extended) ;   
 bcdtmWrite_message(0,0,0,"DTM CleanUp           = %10ld",dtmP->dtmCleanUp) ;   
 bcdtmWrite_message(0,0,0,"DTM Feature Index     = %10ld",dtmP->dtmFeatureIndex) ;   
 bcdtmWrite_message(0,0,0,"DTM pptol             = %20.15lf",dtmP->ppTol) ;   
 bcdtmWrite_message(0,0,0,"DTM pltol             = %20.15lf",dtmP->plTol) ;   
 bcdtmWrite_message(0,0,0,"DTM mppltol           = %20.15lf",dtmP->mppTol) ;   
 bcdtmWrite_message(0,0,0,"DTM numPoints         = %10ld",dtmP->numPoints) ;   
 bcdtmWrite_message(0,0,0,"DTM memPoints         = %10ld",dtmP->memPoints) ;   
 bcdtmWrite_message(0,0,0,"DTM numFeatures       = %10ld",dtmP->numFeatures) ;   
 bcdtmWrite_message(0,0,0,"DTM memFeatures       = %10ld",dtmP->memFeatures) ;   
 bcdtmWrite_message(0,0,0,"DTM numFList          = %10ld",dtmP->numFlist) ;   
 bcdtmWrite_message(0,0,0,"DTM memFList          = %10ld",dtmP->memFlist) ;   
/*
** Count Number Of Feature Points
*/
 numFeaturePoints = 0 ;
 numRandomPoints  = dtmP->numPoints ;
 if( dtmP->numFeatures > 0 )
   {
    if( dtmP->dtmState == DTMState::Data )
      {
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
            {
             numFeaturePoints = numFeaturePoints + dtmFeatureP->numDtmFeaturePts ;
            } 
         }
       numRandomPoints = dtmP->numPoints - numFeaturePoints ; 
      } 
    if( dtmP->dtmState == DTMState::Tin )
      {
       numRandomPoints = 0 ;
       numFeaturePoints = 0 ;     
       for( n = 0 ; n < dtmP->numPoints ; ++n )
         {
          if( nodeAddrP(dtmP,n)->fPtr == dtmP->nullPtr ) ++numRandomPoints ;
          else                                           ++numFeaturePoints ;
         }
      } 
   }
 bcdtmWrite_message(0,0,0,"Number Of Feature Points = %8ld",numFeaturePoints) ;
 bcdtmWrite_message(0,0,0,"Number Of Random  Points = %8ld",numRandomPoints) ;
/*
** Report Triangle Stats
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    bcdtmWrite_message(0,0,0,"Number Of DTM Lines      = %8ld",dtmP->numLines) ;
    bcdtmWrite_message(0,0,0,"Number Of DTM Triangles  = %8ld",dtmP->numTriangles) ;
   }
/*
** Count Features
*/
 bcdtmWrite_message(0,0,0,"Number Of DTM Features   = %8ld",dtmP->numFeatures) ;
 if( dtmP->numFeatures > 0 )
   {  
    for( n = (int)DTMFeatureState::Unused ; n <= (int)DTMFeatureState::Rollback ; ++n )
      {
       featuresFound = 0 ;
       for( m = 0 ; m < 1000 ; ++m )featureCounts[m] = 0 ;
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == (DTMFeatureState)n )
            {
             ++featureCounts[(long)dtmFeatureP->dtmFeatureType] ;
             ++featuresFound  ;
            }
         } 
       if( featuresFound )
         {
          if( n == 0 ) strcpy(dtmFeatureStateName,"DTMFeatureState::Unused") ; 
          if( n == 1 ) strcpy(dtmFeatureStateName,"DTMFeatureState::Data") ; 
          if( n == 2 ) strcpy(dtmFeatureStateName,"DTMFeatureState::PointsArray") ; 
          if( n == 3 ) strcpy(dtmFeatureStateName,"DTMFeatureState::OffsetsArray") ; 
          if( n == 4 ) strcpy(dtmFeatureStateName,"DTMFeatureState::Tin") ; 
          if( n == 5 ) strcpy(dtmFeatureStateName,"DTMFeatureState::TinError") ; 
          if( n == 6 ) strcpy(dtmFeatureStateName,"DTMFeatureState::Deleted") ; 
          if( n == 7 ) strcpy(dtmFeatureStateName,"DTMFeatureState::Rollback") ;  
          bcdtmWrite_message(0,0,0,"Number Of %-25s State Features = %8ld",dtmFeatureStateName,featuresFound) ;  
          for( m = 0 ; m < 1000 ; ++m )
            {
             if( featureCounts[m] )
               {
                if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType((DTMFeatureType)m,dtmFeatureTypeName)) goto errexit ;
                bcdtmWrite_message(0,0,0,"**** %20s  ** Number = %8ld",dtmFeatureTypeName,featureCounts[m]) ;
               } 
            }
         }
      } 
   }
/*
** Report Feature Points
*/
 if( reportFeatures )
   {
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
       if( bcdtmData_getDtmFeatureStateNameFromDtmFeatureState(dtmFeatureP->dtmFeatureState,dtmFeatureStateName)) goto errexit ;
       bcdtmWrite_message(0,0,0,"Dtm Feature[%6ld] ** State = %-20s Type = %-20s ** userTag = %11I64d featureId = %11I64d ** state = %2ld",dtmFeature,dtmFeatureStateName,dtmFeatureTypeName,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmFeatureState) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray || dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback )
         {
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;         
          for( p3dP = featurePtsP ; p3dP <  featurePtsP + numFeaturePts ; ++p3dP )
            {
             bcdtmWrite_message(0,0,0,"Feature Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
            }
         } 
      }
  }
 bcdtmWrite_message(0,0,0,"*********** Reporting Statistics DTM Object = %p Completed",dtmP) ;

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

struct CleanupCaller
    {
    BC_DTM_OBJ& m_dtm;
    CleanupCaller (BC_DTM_OBJ& dtm) : m_dtm (dtm) {}
    void CallHandler (BcDTMAppData& handler) const { handler._OnCleanup (m_dtm); }
    };

BC_DTM_OBJ::BC_DTM_OBJ ()
    {
    memset (&dtmObjType, 0, offsetof (BC_DTM_OBJ, appData));
    }

BC_DTM_OBJ::~BC_DTM_OBJ ()
    {
    }

BcDTMAppData* BC_DTM_OBJ::FindAppData (const BcDTMAppData::Key& key) const
    {
    return appData.FindAppData (key);
    }
StatusInt BC_DTM_OBJ::AddAppData (const BcDTMAppData::Key& key, BcDTMAppData* data)
    {
    return appData.AddAppData (key, data, *this);
    }

void BC_DTM_OBJ::ClearUpAppData ()
    {
    appData.CallAll (CleanupCaller (*this));
    appData.m_list.clear ();  // (Don't allow SetDgnModel to call _OnCacheRelease on handlers.)
    }

void BC_DTM_OBJ::CopyHeaderDetails (const BC_DTM_OBJ& copy)
    {
    memcpy (this, &copy, offsetof (BC_DTM_OBJ, refCount));
    // If this is an DTMElement then we can't clone unless we make a call to create a new DTMElement,  At the moment change this back to a standand Memory DTM.
    if (dtmObjType == BC_DTM_ELM_TYPE)
        dtmObjType = BC_DTM_OBJ_TYPE;

    numFeatures = 0;
    memFeatures = 0;
    numPoints = 0;
    memPoints = 0;
    numNodes = 0;
    memNodes = 0;
    numClist = 0;
    memClist = 0;
    numFlist = 0;
    memFlist = 0;
    }
