/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmGeopak.cpp $
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
BENTLEYDTM_EXPORT int bcdtmObject_createDataObject(DTM_DAT_OBJ **dataPP )
/*
** This Function Creates a DTMFeatureState::Data Object and
** Returns a Pointer To The Object
*/
{
 long ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Delete Data Object If It Exists
*/
 if( *dataPP != nullptr )
   {
    if( bcdtmObject_deleteDataObject(dataPP)) goto errexit ;
    *dataPP = nullptr ;
   }
/*
** Create Data Object
*/
 *dataPP = ( DTM_DAT_OBJ * ) malloc ( sizeof(DTM_DAT_OBJ)) ;
 if( *dataPP == nullptr ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Initialise Creation Of Data Object
*/   
 bcdtmObject_initialiseDataObject(*dataPP) ;
/*
** Log Creation Of Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Created Data Object %p",*dataPP) ;   
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
BENTLEYDTM_Public int bcdtmObject_initialiseDataObject(DTM_DAT_OBJ *dataP)
/*
** This Function Initialises A Data Object
*/
{
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDataObject(dataP)) return(DTM_ERROR) ;
/*
** Initialise Data Object
*/
 dataP->dtmFileType     = DTM_DAT_TYPE ;
 dataP->dtmFileVersion  = DTM_DAT_FILE_VERSION ;
 strcpy(dataP->userName,"") ;
 strcpy(dataP->dataObjectFileName,"") ;
 strcpy(dataP->userMessage,"") ;
 dataP->xMin = 0.0 ;
 dataP->yMin = 0.0 ;
 dataP->zMin = 0.0 ;
 dataP->xMax = 0.0 ;
 dataP->yMax = 0.0 ;
 dataP->zMax = 0.0 ;
 dataP->numPts = 0 ;
 dataP->memPts = 0 ;
 dataP->numFeatPts = 0 ;
 dataP->stateFlag = 0 ;
 dataP->iniMemPts  = DTM_INI_MEM_PTS;
 dataP->incMemPts  = DTM_INC_MEM_PTS ;
 dataP->plTol = DTM_PLTOL ;
 dataP->ppTol = DTM_PPTOL ;
 dataP->numDecDigits = DTM_NUM_DEC_DIGITS   ;
#ifdef _WIN32_WCE
 dataP->creationTime = 0 ;
#else
 _time32(&dataP->creationTime) ;
#endif
 dataP->modifiedTime = 0 ;
 dataP->userTime     = 0 ;
 dataP->refCount     = 0 ;
 dataP->userStatus   = 0 ;
 dataP->featureCodeP = nullptr  ;
 dataP->userTagP     = nullptr  ;
 dataP->pointsP      = nullptr  ;
 dataP->guidP        = nullptr  ;
/*
**  Job Completed
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_deleteDataObject(DTM_DAT_OBJ **dataPP)
/*
** This Function Deletes a DTMFeatureState::Data Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTM_DAT_OBJ *datP=*dataPP ;
/*
** Check For Null Data Object
*/
 if( *dataPP != nullptr ) 
   {
    if( bcdtmObject_freeMemoryDataObject(*dataPP)) goto errexit ;
    free(*dataPP) ;
    *dataPP = nullptr ;
   }
/*
** Log Destruction Of Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Destroyed Data Object %p",datP) ;   
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
BENTLEYDTM_Public int bcdtmObject_freeMemoryDataObject(DTM_DAT_OBJ *dataP)
/*
** This Function Frees A Data Object Memory
*/
{
/*
** Free Memory
*/
 dataP->numPts = dataP->memPts = dataP->numFeatPts = dataP->stateFlag = 0 ;
 dataP->xMin = dataP->yMin = dataP->zMin = 0.0 ;
 dataP->xMax = dataP->yMax = dataP->zMax = 0.0 ;
 if( dataP->featureCodeP != nullptr ) { free(dataP->featureCodeP) ; dataP->featureCodeP = nullptr ; }
 if( dataP->pointsP      != nullptr ) { free(dataP->pointsP)      ; dataP->pointsP = nullptr ; }
 if( dataP->userTagP     != nullptr ) { free(dataP->userTagP)     ; dataP->userTagP = nullptr ; }
/*
** Initialise Dat
*/
 bcdtmObject_initialiseDataObject(dataP) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_createTinObject(DTM_TIN_OBJ **tinPP)
/*
** This Function Creates a Tin Object and
** Returns a Pointer To The Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Delete Tin Object If It Exists
*/
 if( *tinPP != nullptr )
   {
    if( bcdtmObject_deleteTinObject(tinPP)) goto errexit ;
   }
/*
** Create Tin Object
*/
 *tinPP = ( DTM_TIN_OBJ * ) malloc ( sizeof(DTM_TIN_OBJ)) ; 
 if( *tinPP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Initialise Tin Object
*/
 bcdtmObject_initialiseTinObject(*tinPP) ;
/*
** Log Creation Of Tin Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Created Tin Object %p",*tinPP) ;   
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
BENTLEYDTM_Public int bcdtmObject_initialiseTinObject(DTM_TIN_OBJ *tinP )
/*
** This Function Initialises A Tin Object
*/
{
 int  ret=DTM_SUCCESS ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Initialise Tin
*/
 tinP->dtmFileType        = DTM_TIN_TYPE ;
 tinP->dtmFileVersion     = DTM_TIN_FILE_VERSION ;
 tinP->xMin               = 0.0 ;
 tinP->yMin               = 0.0 ;
 tinP->zMin               = 0.0 ;
 tinP->xMax               = 0.0 ;
 tinP->yMax               = 0.0 ;
 tinP->zMax               = 0.0 ;
 tinP->xRange             = 0.0 ;
 tinP->yRange             = 0.0 ;
 tinP->zRange             = 0.0 ;
 tinP->ppTol              = 0.0 ;
 tinP->plTol              = 0.0 ;
 tinP->mppTol             = 0.0 ;
 tinP->numDecDigits       = 0 ;
 tinP->numPts             = 0 ;
 tinP->numSortedPts       = 0 ;
 tinP->memPts             = 0 ;
 tinP->numTriangles       = 0 ;
 tinP->numLines           = 0 ;
 tinP->numFeatureTable    = 0 ;
 tinP->memFeatureTable    = 0 ;
 tinP->numFeatureList     = 0 ;
 tinP->memFeatureList     = 0 ;
 tinP->numFeatureMap      = 0 ;
 tinP->memFeatureMap      = 0 ;
 tinP->hullPnt            = DTM_NULL_PNT ;
 tinP->nextHullPnt        = DTM_NULL_PNT ;
 tinP->cListPtr           = 0 ;
 tinP->nullPtr            = DTM_NULL_PTR ;
 tinP->cListDelPtr        = DTM_NULL_PTR ;
 tinP->cListLastDelPtr    = DTM_NULL_PTR ;
 tinP->featureListDelPtr  = DTM_NULL_PTR ;
 tinP->nullPnt            = DTM_NULL_PNT ;
 tinP->nullUserTag        = DTM_NULL_USER_TAG ;
 tinP->nullGuid           = nullGuid    ; 
 tinP->iniMemPts          = DTM_INI_MEM_PTS;
 tinP->incMemPts          = DTM_INC_MEM_PTS ;
 tinP->iniMemFeatureTable = DTM_INI_MEM_FEATURES_TABLE ;
 tinP->incMemFeatureTable = DTM_INC_MEM_FEATURES_TABLE ;
 tinP->iniMemFeatureList  = DTM_INI_MEM_FEATURES_LIST ;
 tinP->incMemFeatureList  = DTM_INC_MEM_FEATURES_LIST ;
 tinP->iniMemFeatureMap   = DTM_INI_MEM_FEATURES_TABLE ;
 tinP->incMemFeatureMap   = DTM_INC_MEM_FEATURES_TABLE ;
 _time32(&tinP->creationTime) ;
 tinP->modifiedTime = 0 ;
 tinP->userTime     = 0 ;
 tinP->refCount     = 0 ;
 tinP->userStatus   = 0 ;
 tinP->SL1 = tinP->SL2 = tinP->SL3 = tinP->SL4 = tinP->SL5 = 0 ;
 tinP->SI641 = tinP->SI642 = tinP->SI643 = tinP->SI644 = tinP->SI645 = 0 ;
 tinP->SD1 = tinP->SD2 = tinP->SD3 = tinP->SD4 = tinP->SD5 = 0.0 ;
 tinP->SP1 = tinP->SP2 = tinP->SP3 = tinP->SP4 = tinP->SP5 = nullptr ;
 tinP->pointsP     = nullptr ;
 tinP->nodesP      = nullptr ; 
 tinP->cListP      = nullptr ;
 tinP->fTableP     = nullptr ;
 tinP->fListP      = nullptr ;
 tinP->fMapP       = nullptr ; 
 strcpy(tinP->userName,"") ;
 strcpy(tinP->tinObjectFileName,"") ;
 strcpy(tinP->userMessage,"") ;
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
BENTLEYDTM_EXPORT int bcdtmObject_deleteTinObject(DTM_TIN_OBJ **tinPP)
/*
** This Function Deletes a Tin Object
*/
{
 int ret=DTM_SUCCESS;
/*
** Check For Null Tin Object
*/
 if( *tinPP != nullptr ) 
   {
    if( bcdtmObject_freeMemoryTinObject(*tinPP)) goto errexit ;
    free(*tinPP) ;
    *tinPP = nullptr ;
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
BENTLEYDTM_Public int bcdtmObject_freeMemoryTinObject(DTM_TIN_OBJ *tinP)
/*
** This Function Frees A Tin Object Memory
*/
{
/*
** Free Memory
*/
 if( tinP->pointsP != nullptr ) { free(tinP->pointsP) ; tinP->pointsP = nullptr ; }
 if( tinP->nodesP  != nullptr ) { free(tinP->nodesP)  ; tinP->nodesP  = nullptr ; }
 if( tinP->cListP  != nullptr ) { free(tinP->cListP)  ; tinP->cListP  = nullptr ; }
 if( tinP->fTableP != nullptr ) { free(tinP->fTableP) ; tinP->fTableP = nullptr ; }
 if( tinP->fListP  != nullptr ) { free(tinP->fListP)  ; tinP->fListP  = nullptr ; }
/*
** Initialise Tin
*/
 bcdtmObject_initialiseTinObject(tinP) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_createLatticeObject(DTM_LAT_OBJ **latticePP)
/*
** This Function Creates a Lattice Object and
** Returns a Pointer To The Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Delete Lattice Object If It Exists
*/
 if( *latticePP != nullptr )
   {
    if( bcdtmObject_deleteLatticeObject(latticePP)) goto errexit ;
   }
/*   
** Create Lattice Object
*/
  *latticePP = ( DTM_LAT_OBJ * ) malloc ( sizeof(DTM_LAT_OBJ)) ;
  if( *latticePP == nullptr )
    {
     bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
     goto errexit ; 
    }
/*
** Initialise Lattice Object
*/	
 bcdtmObject_initialiseLatticeObject(*latticePP) ;
/*
** Log Creation Of Lattice Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Created Lattice Object %p",*latticePP) ;   
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
BENTLEYDTM_Public int bcdtmObject_initialiseLatticeObject(DTM_LAT_OBJ *latticeP )
/*
** This Function Initialises A latticeP Object
*/
{
/*
** Initialise latticeP
*/
 latticeP->dtmFileType = DTM_LAT_TYPE ;
 latticeP->dtmFileVersion = DTM_LAT_FILE_VERSION ;
 latticeP->INTMODE = latticeP->NXL = latticeP->NYL = latticeP->NOLATPTS = latticeP->NOACTPTS = 0 ;
 latticeP->L1 = latticeP->L2 = latticeP->L3 = latticeP->L4 = latticeP->L5 = latticeP->L6 = latticeP->L7 = 0 ;
 latticeP->NULLVAL = latticeP->FS1 = (float ) 0.0 ;
 latticeP->DX = latticeP->DY = 0.0 ;
 latticeP->LXMIN = latticeP->LYMIN = latticeP->LZMIN = 0.0 ;
 latticeP->LXMAX = latticeP->LYMAX = latticeP->LZMAX = 0.0 ;
 latticeP->LXDIF = latticeP->LYDIF = latticeP->LZDIF = 0.0 ;
 latticeP->D1 = latticeP->D2 = latticeP->D3 = latticeP->D4 = latticeP->D5 = latticeP->D6 = latticeP->D7 = latticeP->D8 = 0.0 ;
 strcpy(latticeP->userName,"") ;
 strcpy(latticeP->LatticeObjectFile,"") ;
 latticeP->LAT = nullptr ;
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
BENTLEYDTM_EXPORT int bcdtmObject_deleteLatticeObject(DTM_LAT_OBJ **latticePP)
/*
** This Function Deletes a Lattice Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTM_LAT_OBJ *latP=*latticePP ;
/*
** Check For Null Lattice Object
*/
 if( *latticePP != nullptr ) 
   {
    if( bcdtmObject_freeMemoryLatticeObject(*latticePP)) goto errexit ; 
    free(*latticePP) ;
    *latticePP = nullptr ;
   }
/*
** Log Destruction Of Lattice Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Destroyed Lattice Object %p",latP) ;   
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
BENTLEYDTM_Public int bcdtmObject_freeMemoryLatticeObject(DTM_LAT_OBJ *latticeP)
/*
** This Function Frees A Lattice Object Memory
*/
{
/*
** Free Memory
*/
 if( latticeP->LAT != nullptr ) 
   { 
    free(latticeP->LAT) ; 
    latticeP->LAT = nullptr ; 
   }
/*
** Initialise Lattice
*/
 bcdtmObject_initialiseLatticeObject(latticeP) ;
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
BENTLEYDTM_Public int bcdtmObject_testForValidDataObject(DTM_DAT_OBJ *dataP)
{
 // Not Implemented For Vancouver 
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_testForValidTinObject(DTM_TIN_OBJ *tinP)
{
 // Not Implemented For Vancouver 
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_testForValidLatticeObject(DTM_LAT_OBJ *latticeP)
{
 // Not Implemented For Vancouver 
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmGeopak_checkForCurrentGeopakDtmObject(BC_DTM_OBJ *dtmP)
{
 int ret=0;
// if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Current Geopak Object ** dtmP = %p DTM_CDOBJ = %p DTM_CTOBJ = %p",dtmP,DTM_CDOBJ,DTM_CTOBJ) ;
// if     ( dtmP == ( BC_DTM_OBJ *)DTM_CDOBJ ) ret = 1 ;
// else if( dtmP == ( BC_DTM_OBJ *)DTM_CTOBJ ) ret = 1 ;
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmGeopak_cloneDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ *cloneDtmP)
/*
** This Function Copies A Dtm Object To A Dtm Object
** This Is A Special Geopak Only Function
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long *l1P,*l2P,ofs,num,numPartition ;
 DPoint3d               *p3d1P,*p3d2P ;
 BC_DTM_FEATURE    *featureP,*dtmFeatureP ;
 DPoint3d     *pointP,*dtmPointP ;
 DTM_TIN_NODE      *nodeP,*dtmNodeP ;
 DTM_CIR_LIST      *clistP,*dtmClistP ;
 DTM_FEATURE_LIST  *flistP,*dtmFlistP ;
 /*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Geopak Cloning Dtm Object %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP                 = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"cloneDtmP            = %p",cloneDtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP->dtmState       = %8ld",dtmP->dtmState) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints      = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"cloneDtmP->dtmState  = %8ld",cloneDtmP->dtmState) ;
    bcdtmWrite_message(0,0,0,"cloneDtmP->numPoints = %8ld",cloneDtmP->numPoints) ;
   }
/*
** Check For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(cloneDtmP)) goto errexit ;
/*
** Clean DTM
*/
 if( bcdtmObject_initialiseDtmObject(cloneDtmP)) goto errexit ;
/*
** Copy  Header 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Header") ;
 cloneDtmP->CopyHeaderDetails (*dtmP);
/*
** Copy Features
*/
 if( dtmP->memFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Features") ;
/*
**  Allocate Memory For Feature Table
*/ 
    cloneDtmP->iniFeatures = dtmP->memFeatures ;
    if( bcdtmObject_allocateFeaturesMemoryDtmObject(cloneDtmP)) goto errexit ;
/*
**  Copy Features 
*/
    ofs = 0 ;
    numPartition = 0 ;
    featureP    = dtmP->fTablePP[numPartition] ;
    dtmFeatureP = cloneDtmP->fTablePP[numPartition] ; 
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
            dtmFeatureP->dtmFeaturePts.pointsPI =  bcdtmMemory_allocate(cloneDtmP, dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d)) ; 
             if( dtmFeatureP->dtmFeaturePts.pointsPI == 0 )
               {
                bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               }
             p3d1P = bcdtmMemory_getPointerP3D(cloneDtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
             p3d2P = bcdtmMemory_getPointerP3D(dtmP, featureP->dtmFeaturePts.pointsPI) ;
             while ( p3d1P < bcdtmMemory_getPointerP3D(cloneDtmP, dtmFeatureP->dtmFeaturePts.pointsPI) + dtmFeatureP->numDtmFeaturePts )
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
            dtmFeatureP->dtmFeaturePts.offsetPI = bcdtmMemory_allocate(cloneDtmP, dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;
             if( dtmFeatureP->dtmFeaturePts.offsetPI == 0 )
               {
                bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               }
             offsetP = bcdtmMemory_getPointerOffset(cloneDtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
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
       if( ofs == cloneDtmP->featurePartitionSize ) 
         {
          ++numPartition ;
          featureP    = dtmP->fTablePP[numPartition] ;
          dtmFeatureP = cloneDtmP->fTablePP[numPartition] ;
          ofs = 0 ;
         } 
       else 
         {
          ++featureP ;
          ++dtmFeatureP ;
         }
       ++cloneDtmP->numFeatures ;
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
    cloneDtmP->iniPoints = dtmP->memPoints ;
    if( bcdtmObject_allocatePointsMemoryDtmObject(cloneDtmP)) goto errexit ;
/*
**  Copy Points 
*/
    ofs = 0 ;
    numPartition = 0 ;
    pointP    = dtmP->pointsPP[numPartition] ; 
    dtmPointP = cloneDtmP->pointsPP[numPartition] ; 
    for( num = 0 ; num < dtmP->numPoints ; ++num )
      {
       *(dtmPointP) = *(pointP) ;
       ++ofs ;
       if( ofs == cloneDtmP->pointPartitionSize) 
         {
          ++numPartition ;
          pointP    = dtmP->pointsPP[numPartition] ; 
          dtmPointP = cloneDtmP->pointsPP[numPartition] ;
          ofs = 0 ;
         } 
       else 
         {
          ++pointP ;
          ++dtmPointP ;
         } 
       ++cloneDtmP->numPoints ;
      }  
   }
/*
** Copy Nodes
*/
  if( dtmP->nodesPP != nullptr && dtmP->memPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Nodes") ;
/*
**  Allocate Memory For Feature Table
*/ 
    if( bcdtmObject_allocateNodesMemoryDtmObject(cloneDtmP)) goto errexit ;
/*
**  Copy Nodes From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    nodeP    =  dtmP->nodesPP[numPartition] ; 
    dtmNodeP = cloneDtmP->nodesPP[numPartition] ; 
    for( num = 0 ; num < dtmP->numPoints ; ++num )
      {
       *(dtmNodeP) = *(nodeP) ;
       ++ofs ;
       if( ofs == cloneDtmP->nodePartitionSize) 
         {
          ++numPartition ;
          nodeP    =  dtmP->nodesPP[numPartition] ; 
          dtmNodeP = cloneDtmP->nodesPP[numPartition] ;
          ofs = 0 ;
         } 
       else 
         {
          ++nodeP  ; 
          ++dtmNodeP ;
         }
       ++cloneDtmP->numNodes ;
      }  
   }
/*
** Copy Circular List
*/
  if( dtmP->cListPP != nullptr )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Circular List") ;
/*
**  Allocate Memory For Feature Table
*/ 
    if( bcdtmObject_allocateCircularListMemoryDtmObject(cloneDtmP)) goto errexit ;
/*
**  Copy Circular List From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    clistP    =  dtmP->cListPP[numPartition] ; 
    dtmClistP = cloneDtmP->cListPP[numPartition] ; 
    for( num = 0 ; num < dtmP->cListPtr ; ++num )
      {
       *(dtmClistP) = *(clistP) ;
       ++ofs ;
       if( ofs == cloneDtmP->clistPartitionSize) 
         {
          ++numPartition ;
          clistP    =  dtmP->cListPP[numPartition] ; 
          dtmClistP = cloneDtmP->cListPP[numPartition] ;
          ofs = 0 ;
         } 
       else
         {
          ++clistP ; 
          ++dtmClistP ;
         }
       ++cloneDtmP->numClist ;
      }  
   }
/*
** Copy Feature List
*/
  if( dtmP->fListPP != nullptr )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature List") ;
/*
**  Allocate Memory For Feature Table
*/ 
    cloneDtmP->iniFlist = dtmP->memFlist ;
    if( bcdtmObject_allocateFeatureListMemoryDtmObject(cloneDtmP)) goto errexit ;
/*
**  Copy Feature List From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    flistP    =  dtmP->fListPP[numPartition] ; 
    dtmFlistP = cloneDtmP->fListPP[numPartition] ; 
    for( num = 0 ; num < dtmP->numFlist ; ++num )
      {
       *(dtmFlistP) = *(flistP) ;
       ++ofs ;
       if( ofs == cloneDtmP->flistPartitionSize) 
         {
          ++numPartition ;
          flistP    =  dtmP->fListPP[numPartition] ; 
          dtmFlistP = cloneDtmP->fListPP[numPartition] ;
          ofs = 0 ;
         } 
       else
         {
          ++flistP ;
          ++dtmFlistP ;
         } 
       ++cloneDtmP->numFlist ;
      }  
   }
/*
** Clean Up
*/
 cleanup :
/* 
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Geopak Cloning Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Geopak Cloning Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_EXPORT int bcdtmGeopak_readGeopakDatFileToDtmObject(BC_DTM_OBJ *dtmP,WCharCP datFileNameP)
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTM_DAT_OBJ *dataP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Geopak Dat File %ws",datFileNameP) ;
/*
** Check For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Clean Existing DTM
*/
 if( bcdtmObject_initialiseDtmObject(dtmP)) goto errexit ;
/*
** Create Data Object
*/
 if( bcdtmObject_createDataObject(&dataP)) goto errexit ;
/*
** Read Data File To Data Object
*/
 if( bcdtmRead_dataFileToDataObject(dataP,datFileNameP)) goto errexit ;
/*
** Copy Data Object To Dtm Object
*/
 if( bcdtmGeopak_copyDataObjectToDtmObject(dataP,dtmP)) goto errexit ;
/*
** Print DTM Coordinate Ranges
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Number of Geopak Dat Points = %6ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"****     Minimum      Maximum       Range") ;
    bcdtmWrite_message(0,0,0,"****   ============ ============ ============") ;
    bcdtmWrite_message(0,0,0,"**** x %12.3lf %12.3lf %12.3lf",dtmP->xMin,dtmP->xMax,dtmP->xMax-dtmP->xMin) ;
    bcdtmWrite_message(0,0,0,"**** y %12.3lf %12.3lf %12.3lf",dtmP->yMin,dtmP->yMax,dtmP->yMax-dtmP->yMin) ;
    bcdtmWrite_message(0,0,0,"**** z %12.3lf %12.3lf %12.3lf",dtmP->zMin,dtmP->zMax,dtmP->zMax-dtmP->zMin) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dataP != nullptr ) bcdtmObject_deleteDataObject(&dataP) ;
/*
**  Return
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
BENTLEYDTM_Public int bcdtmGeopak_copyDataObjectToDtmObject(DTM_DAT_OBJ *dataP,BC_DTM_OBJ *dtmP)
/*
** This Function Copies A Data Object To A Dtm Object
*/
{
 int               ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long              fsCode, fnCode, offset, numFeaturePts = 0;
 DTMFeatureType dtmFeatureType;
 DPoint3d               *points1P,*points2P,*featurePtsP=nullptr ;
 DTM_FEATURE_CODE  *fCodeP,*nCodeP ;
 DTMUserTag      userTag ;
 DTMFeatureId    userFeatureId ;
/*
** Check For Vaid DTM Data Object
*/
 if( bcdtmObject_testForValidDataObject(dataP)) goto errexit ;
/*
** Check For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Set Point Memory Allocation Parameters
*/
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,dataP->numPts,(dtmP)->incPoints) ;
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
       if( (dtmP)->numFeatures % 1000 == 0  || (dtmP)->numPoints % 10000 == 0 )
         {
          bcdtmWrite_message(0,0,0,"Number Of Features = %8ld Number Of Points = %8ld",(dtmP)->numFeatures,(dtmP)->numPoints) ;
         }
      }
/*
**  Get Start And Next Codes For DTM Feature
*/
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
       if( featurePtsP == nullptr )
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
          if( dataP->userTagP   != nullptr ) userTag  = *(dataP->userTagP + offset) ;  
          userFeatureId = (dtmP)->dtmFeatureIndex ;
          ++(dtmP)->dtmFeatureIndex ;
         } 
/*
**     Store In Dtm Object
*/ 
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,dtmFeatureType,userTag,2,&userFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
/*
**     Free Feature Points Memory
*/
       if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
/*
**     Set For Next Feature
*/
       fCodeP = nCodeP ;  
      }
   } 
/*
**  Write Stats
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Features = %8ld Number Of Points = %8ld",(dtmP)->numFeatures,(dtmP)->numPoints) ;
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
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
BENTLEYDTM_EXPORT int bcdtmGeopak_copyTinObjectToDtmObject(DTM_TIN_OBJ *tinP,BC_DTM_OBJ *dtmP)
/*
** This Function Copies A Data Object To A Dtm Object
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long ofs,numPartition ;
 DTM_FEATURE_TABLE *featP ;
 BC_DTM_FEATURE    *dtmFeatureP ;
 DPoint3d     *pointP,*dtmPointP ;
 DTM_TIN_NODE      *nodeP,*dtmNodeP ;
 DTM_CIR_LIST      *clistP,*dtmClistP ;
 DTM_FEATURE_LIST_VER200  *flistP ;
 DTM_FEATURE_LIST  *dtmFlistP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Tin Object %p To Dtm Object",tinP) ;
/*
** Check For Vaid Tin Object
*/
 if( bcdtmObject_testForValidTinObject(tinP)) goto errexit ;
/*
** Check For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Clean DTM
*/
 if( bcdtmObject_initialiseDtmObject(dtmP)) goto errexit ;
/*
** Write Out Tin Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"numFeatureTable = %8ld memFeatureTable = %8ld",tinP->numFeatureTable,tinP->memFeatureTable) ;
    bcdtmWrite_message(0,0,0,"numFeatureList  = %8ld memFeatureList  = %8ld",tinP->numFeatureList,tinP->memFeatureList) ;
    bcdtmWrite_message(0,0,0,"numPts          = %8ld memPts          = %8ld",tinP->numPts,tinP->memPts) ;
    bcdtmWrite_message(0,0,0,"numNode         = %8ld memNodes        = %8ld",tinP->numPts,tinP->memPts) ;
    bcdtmWrite_message(0,0,0,"clistPtr        = %8ld",tinP->cListPtr) ;
   } 
/*
** Copy Tin Object Header To Dtm Object Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Tin Object Header To Dtm Object Header") ;
 dtmP->dtmObjType           =  BC_DTM_OBJ_TYPE  ;
 dtmP->dtmObjVersion        =  BC_DTM_OBJ_VERSION ;
 dtmP->numLines             =  tinP->numLines ;
 dtmP->numTriangles         =  tinP->numTriangles ;
 dtmP->numFeatures          =  0 ;
 dtmP->memFeatures          =  0 ;
 dtmP->iniFeatures          =  tinP->iniMemFeatureTable ;
 dtmP->incFeatures          =  tinP->incMemFeatureTable ;
 dtmP->numFeaturePartitions =  0 ;
 dtmP->featurePartitionSize =  DTM_PARTITION_SIZE_FEATURE ;
 dtmP->numPoints            =  0 ;
 dtmP->memPoints            =  0 ;
 dtmP->iniPoints            =  tinP->iniMemPts ;
 dtmP->incPoints            =  tinP->incMemPts ;
 dtmP->numSortedPoints      =  tinP->numSortedPts ;
 dtmP->numPointPartitions   =  0 ;
 dtmP->pointPartitionSize   =  DTM_PARTITION_SIZE_POINT ;
 dtmP->numNodes             =  0 ;
 dtmP->memNodes             =  0 ;
 dtmP->numNodePartitions    =  0 ;
 dtmP->nodePartitionSize    =  DTM_PARTITION_SIZE_NODE ;
 dtmP->numClist             =  0 ;
 dtmP->memClist             =  0 ;
 dtmP->numClistPartitions   =  0 ;
 dtmP->clistPartitionSize   =  DTM_PARTITION_SIZE_CLIST ;
 dtmP->numFlist             =  0 ;
 dtmP->memFlist             =  0 ;
 dtmP->iniFlist             =  tinP->iniMemFeatureList ;
 dtmP->incFlist             =  tinP->incMemFeatureList ;
 dtmP->numFlistPartitions   =  0 ;
 dtmP->flistPartitionSize   =  DTM_PARTITION_SIZE_FLIST ;
 dtmP->dtmState             =  DTMState::Tin ;
 dtmP->nullPnt              =  tinP->nullPnt ;
 dtmP->nullPtr              =  tinP->nullPtr ;
 dtmP->nullUserTag          =  tinP->nullUserTag ;
 dtmP->dtmFeatureIndex      =  0 ; 
 dtmP->nullFeatureId        =  DTM_NULL_FEATURE_ID ;
 dtmP->cListPtr             =  tinP->cListPtr  ;
 if( tinP->cListDelPtr       != tinP->nullPtr ) dtmP->cListDelPtr =  tinP->cListDelPtr    ;
 if( tinP->featureListDelPtr != tinP->nullPtr ) dtmP->fListDelPtr =  tinP->featureListDelPtr ;
 dtmP->refCount             =  0 ;
 dtmP->userStatus           =  tinP->userStatus ;
 dtmP->creationTime         =  tinP->creationTime ;
 dtmP->modifiedTime         =  tinP->modifiedTime ;
 dtmP->hullPoint            =  tinP->hullPnt ;
 dtmP->nextHullPoint        =  tinP->nextHullPnt ;
 dtmP->userTime             =  tinP->userTime ;
 dtmP->ppTol                =  tinP->ppTol ;
 dtmP->plTol                =  tinP->plTol ; 
 dtmP->mppTol               =  tinP->mppTol ; 
 dtmP->xMin                 =  tinP->xMin ;
 dtmP->yMin                 =  tinP->yMin ;
 dtmP->zMin                 =  tinP->zMin ;
 dtmP->xMax                 =  tinP->xMax ;
 dtmP->yMax                 =  tinP->yMax ;
 dtmP->zMax                 = tinP->zMax ;
 dtmP->xRange               = tinP->xRange ;
 dtmP->yRange               = tinP->yRange ;
 dtmP->zRange               = tinP->zRange ;
 dtmP->fTablePP             = nullptr ;
 dtmP->pointsPP             = nullptr ;
 dtmP->nodesPP              = nullptr ;
 dtmP->cListPP              = nullptr ;
 dtmP->fListPP              = nullptr ;
 dtmP->DTMAllocationClass   = nullptr;
/*
** Copy Features
*/
 if( tinP->memFeatureTable > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Features") ;
/*
**  Allocate Memory For Feature Table
*/ 
    dtmP->iniFeatures = tinP->memFeatureTable ;
    if( bcdtmObject_allocateFeaturesMemoryDtmObject(dtmP)) goto errexit ;
/*
**  Copy Features From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    dtmFeatureP = dtmP->fTablePP[numPartition] ; 
    for( featP = tinP->fTableP ; featP < tinP->fTableP + tinP->numFeatureTable ; ++featP )
      {
       dtmFeatureP->dtmFeatureType  = featP->dtmFeatureType ;
       dtmFeatureP->dtmUserTag      = featP->userTag  ;
       dtmFeatureP->dtmFeatureId    = dtmP->dtmFeatureIndex ;
       ++dtmP->dtmFeatureIndex ;
       dtmFeatureP->dtmFeatureState = DTMFeatureState::Tin ;
       dtmFeatureP->dtmFeaturePts.firstPoint = dtmP->nullPnt ;
       if( featP->firstPnt != tinP->nullPnt )  dtmFeatureP->dtmFeaturePts.firstPoint = featP->firstPnt ;
       else                                    dtmFeatureP->dtmFeatureState          = DTMFeatureState::Deleted ;
       ++ofs ;
       if( ofs == dtmP->featurePartitionSize) 
         {
          ++numPartition ;
          dtmFeatureP = dtmP->fTablePP[numPartition] ;
          ofs = 0 ;
         } 
       else ++dtmFeatureP ;
       ++dtmP->numFeatures ;
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
    dtmP->iniPoints = tinP->memPts ;
    if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit ;
/*
**  Copy Points From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    dtmPointP = dtmP->pointsPP[numPartition] ; 
    for( pointP = tinP->pointsP ; pointP < tinP->pointsP + tinP->numPts ; ++pointP )
      {
       *(dtmPointP) = *(pointP) ;
       ++ofs ;
       if( ofs == dtmP->pointPartitionSize) 
         {
          ++numPartition ;
          dtmPointP = dtmP->pointsPP[numPartition] ;
          ofs = 0 ;
         } 
       else ++dtmPointP ;
       ++dtmP->numPoints ;
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
    if( bcdtmObject_allocateNodesMemoryDtmObject(dtmP)) goto errexit ;
/*
**  Copy Nodes From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    dtmNodeP = dtmP->nodesPP[numPartition] ; 
    for( nodeP = tinP->nodesP ; nodeP < tinP->nodesP + tinP->numPts ; ++nodeP )
      {
       *(dtmNodeP) = *(nodeP) ;
       ++ofs ;
       if( ofs == dtmP->nodePartitionSize) 
         {
          ++numPartition ;
          dtmNodeP = dtmP->nodesPP[numPartition] ;
          ofs = 0 ;
         } 
       else ++dtmNodeP ;
//       ++dtmP->numNodes ;
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
    if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit ;
/*
**  Copy Circular List From Tin To Dtm
*/
    ofs = 0 ;
    numPartition = 0 ;
    dtmClistP = dtmP->cListPP[numPartition] ; 
    for( clistP = tinP->cListP ; clistP < tinP->cListP + tinP->cListPtr ; ++clistP )
      {
       *(dtmClistP) = *(clistP) ;
       ++ofs ;
       if( ofs == dtmP->clistPartitionSize) 
         {
          ++numPartition ;
          dtmClistP = dtmP->cListPP[numPartition] ;
          ofs = 0 ;
         } 
       else ++dtmClistP ;
//       ++dtmP->numClist ;
      }  
   }
/*
** Copy Feature List
*/
  if( tinP->memFeatureList > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature List") ;
/*
**  Allocate Memory For Feature Table
*/ 
    dtmP->iniFlist = tinP->memFeatureList ;
    if( bcdtmObject_allocateFeatureListMemoryDtmObject(dtmP)) goto errexit ;
/*
**  Copy Feature List From Tin To Dtm
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature List") ;
    ofs = 0 ;
    numPartition = 0 ;
    dtmFlistP = dtmP->fListPP[numPartition] ; 
    for( flistP = tinP->fListP ; flistP < tinP->fListP + tinP->numFeatureList ; ++flistP )
      {
//       *(dtmFlistP) = *(flistP) ;
       dtmFlistP->nextPnt = flistP->nextPnt ;
       dtmFlistP->nextPtr = flistP->nextPtr ;
       dtmFlistP->dtmFeature = flistP->dtmFeature ;
       dtmFlistP->pntType = 1 ;
       ++ofs ;
       if( ofs == dtmP->flistPartitionSize) 
         {
          ++numPartition ;
          dtmFlistP = dtmP->fListPP[numPartition] ;
          ofs = 0 ;
         } 
       else ++dtmFlistP ;
       ++dtmP->numFlist ;
      }  
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
BENTLEYDTM_EXPORT int bcdtmGeopak_copyDtmObjectToTinObject 
(
 BC_DTM_OBJ  *dtmP,            /* ==> Pointer To DTM Object   */
 DTM_TIN_OBJ **tinPP           /* <== Pointer To Tin Object   */ 
)
/*
** This A DTM Object To A Tin Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_TRACE_VALUE(0) ;
 long node,point,cList,fList,fTable,numTinFeatures,numDtmFeatures,dtmTinFeature ;
 DPoint3d    *pointP ;
 DTM_TIN_NODE     *nodeP ;
 DTM_CIR_LIST     *cListP ;
 BC_DTM_FEATURE   *fTableP ;
 DTM_FEATURE_LIST *fListP ;
 DTM_FEATURE_TABLE tinFeature ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Copying DTM Object To Tin Object") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
   }
/*
**  Validate Tin 2 Object
*/
 if( *tinPP != nullptr ) 
   {
    if( bcdtmObject_testForValidTinObject(*tinPP)) *tinPP = nullptr ;
    if( *tinPP != nullptr ) bcdtmObject_deleteTinObject(tinPP) ; 
   }
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** DTM Must Be In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check DTM
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulated DTM = %p",dtmP) ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"Triangulated DTM Corrupted") ;
       goto errexit ;
      }
    else bcdtmWrite_message(0,0,0,"Triangulated DTM OK") ;
   }
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dtmP->numPoints    = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->memPoints    = %8ld",dtmP->memPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numLines     = %8ld",dtmP->numLines) ;
    bcdtmWrite_message(0,0,0,"dtmP->numTriangles = %8ld",dtmP->numTriangles) ;
    bcdtmWrite_message(0,0,0,"dtmP->cListPtr     = %8ld",dtmP->cListPtr) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFeatures  = %8ld",dtmP->numFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->memFeatures  = %8ld",dtmP->numFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFlist     = %8ld",dtmP->numFlist) ;
    bcdtmWrite_message(0,0,0,"dtmP->memFlist     = %8ld",dtmP->memFlist) ;
   }
/*
** Create Tin Object
*/
 if( bcdtmObject_createTinObject(tinPP)) goto errexit ;
/*
** Populate Tin Object Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Tin Object Header") ;
 (*tinPP)->dtmFileType        = DTM_TIN_TYPE ;
 (*tinPP)->dtmFileVersion     = DTM_TIN_FILE_VERSION ;
 (*tinPP)->xMin               = dtmP->xMin ;
 (*tinPP)->yMin               = dtmP->yMin ;
 (*tinPP)->zMin               = dtmP->zMin ;
 (*tinPP)->xMax               = dtmP->xMax ;
 (*tinPP)->yMax               = dtmP->yMax ;
 (*tinPP)->zMax               = dtmP->zMax ;
 (*tinPP)->xRange             = dtmP->xMax - dtmP->xMin ;
 (*tinPP)->yRange             = dtmP->yMax - dtmP->yMin ;
 (*tinPP)->zRange             = dtmP->zMax - dtmP->zMin ;
 (*tinPP)->ppTol              = dtmP->ppTol ;
 (*tinPP)->plTol              = dtmP->plTol ;
 (*tinPP)->mppTol             = dtmP->mppTol ;
 (*tinPP)->numDecDigits       = 0 ;
 (*tinPP)->numPts             = dtmP->numPoints ;
 (*tinPP)->numSortedPts       = dtmP->numSortedPoints ;
 (*tinPP)->memPts             = dtmP->memPoints ;
 (*tinPP)->numTriangles       = dtmP->numTriangles ;
 (*tinPP)->numLines           = dtmP->numLines ;
 (*tinPP)->numFeatureTable    = dtmP->numFeatures ;
 (*tinPP)->memFeatureTable    = dtmP->memFeatures ;
 (*tinPP)->numFeatureList     = dtmP->numFlist ;
 (*tinPP)->memFeatureList     = dtmP->memFlist ;
 (*tinPP)->numFeatureMap      = 0 ;
 (*tinPP)->memFeatureMap      = 0 ;
 (*tinPP)->hullPnt            = dtmP->hullPoint ;
 (*tinPP)->nextHullPnt        = dtmP->nextHullPoint ;
 (*tinPP)->cListPtr           = dtmP->cListPtr ;
 (*tinPP)->nullPtr            = dtmP->nullPtr ;
 (*tinPP)->cListDelPtr        = dtmP->cListDelPtr ;
 (*tinPP)->cListLastDelPtr    = dtmP->nullPtr ;
 (*tinPP)->featureListDelPtr  = DTM_NULL_PTR ;
 (*tinPP)->nullPnt            = dtmP->nullPnt ;
 (*tinPP)->nullUserTag        = dtmP->nullUserTag ;
 (*tinPP)->nullGuid           = nullGuid    ; 
 (*tinPP)->iniMemPts          = dtmP->iniPoints ;
 (*tinPP)->incMemPts          = dtmP->incPoints ;
 (*tinPP)->iniMemFeatureTable = dtmP->iniFeatures ;
 (*tinPP)->incMemFeatureTable = dtmP->incFeatures ;
 (*tinPP)->iniMemFeatureList  = dtmP->iniFlist ;
 (*tinPP)->incMemFeatureList  = dtmP->incFlist ;
 (*tinPP)->iniMemFeatureMap   = DTM_INI_MEM_FEATURES_TABLE ;
 (*tinPP)->incMemFeatureMap   = DTM_INC_MEM_FEATURES_TABLE ;
 (*tinPP)->creationTime       = dtmP->creationTime ;
 (*tinPP)->modifiedTime       = dtmP->modifiedTime ;
 (*tinPP)->userTime           = dtmP->userTime ;
 (*tinPP)->refCount           = dtmP->refCount ;
 (*tinPP)->userStatus         = dtmP->userStatus ;
 (*tinPP)->SL1   = (*tinPP)->SL2   = (*tinPP)->SL3   = (*tinPP)->SL4   = (*tinPP)->SL5 = 0 ;
 (*tinPP)->SI641 = (*tinPP)->SI642 = (*tinPP)->SI643 = (*tinPP)->SI644 = (*tinPP)->SI645 = 0 ;
 (*tinPP)->SD1 = (*tinPP)->SD2     = (*tinPP)->SD3   = (*tinPP)->SD4   = (*tinPP)->SD5 = 0.0 ;
 (*tinPP)->SP1 = (*tinPP)->SP2     = (*tinPP)->SP3   = (*tinPP)->SP4   = (*tinPP)->SP5 = nullptr ;
 (*tinPP)->pointsP     = nullptr ;
 (*tinPP)->nodesP      = nullptr ; 
 (*tinPP)->cListP      = nullptr ;
 (*tinPP)->fTableP     = nullptr ;
 (*tinPP)->fListP      = nullptr ;
 (*tinPP)->fMapP       = nullptr ; 
 strcpy((*tinPP)->userName,"") ;
 strcpy((*tinPP)->tinObjectFileName,"") ;
 strcpy((*tinPP)->userMessage,"") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Tin Object Header Completed") ;
/*
** Count And Set Number Of Geopak Tin Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numFeatures = %8ld dtmP->memFeatures = %8ld",dtmP->numFeatures,dtmP->memFeatures) ;
 numTinFeatures = 0 ;
 numDtmFeatures = 0 ;
 for( fTable = 0 ; fTable < dtmP->memFeatures ; ++fTable )
   {
    fTableP = ftableAddrP(dtmP,fTable) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeature = %4ld ** Type = %2ld ** State = %2ld",fTable,fTableP->dtmFeatureType,fTableP->dtmFeatureState) ;
    if( fTableP->dtmFeatureState == DTMFeatureState::Tin && fTableP->dtmFeatureType != DTMFeatureType::Hull ) ++numTinFeatures ;
    ++numDtmFeatures ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"numDtmFeatures    = %8ld numTinFeatures    = %8ld",numDtmFeatures,numTinFeatures) ;
 if( (*tinPP)->numFeatureTable == (*tinPP)->memFeatureTable )
   {
    (*tinPP)->numFeatureTable = numTinFeatures ;
    (*tinPP)->memFeatureTable = numTinFeatures ;
   } 
 else (*tinPP)->numFeatureTable = numTinFeatures ;
/*
** Allocate Memory For Tin Point Array
*/
 (*tinPP)->pointsP = (DPoint3d *) malloc( (*tinPP)->memPts * sizeof(DPoint3d)) ;
 if( (*tinPP)->pointsP == nullptr )  
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
    goto errexit ; 
   }
/*
** Allocate Memory For Tin Node Array
*/
 (*tinPP)->nodesP = (DTM_TIN_NODE *) malloc( (*tinPP)->memPts * sizeof(DTM_TIN_NODE)) ;
 if( (*tinPP)->nodesP == nullptr )  
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
    goto errexit ; 
   }
/*
** Allocate Memory For Tin Clist Array
*/
 if( (*tinPP)->numPts == (*tinPP)->memPts ) (*tinPP)->cListP = (DTM_CIR_LIST *) malloc( (*tinPP)->cListPtr * sizeof(DTM_CIR_LIST)) ;
 else                                       (*tinPP)->cListP = (DTM_CIR_LIST *) malloc( (*tinPP)->memPts * 6 * sizeof(DTM_CIR_LIST)) ;                 
 if( (*tinPP)->cListP == nullptr ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Allocate Memory For Tin Feature Table Array
*/
 if( (*tinPP)->memFeatureTable > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tin Feature Table Array") ;
    (*tinPP)->fTableP = (DTM_FEATURE_TABLE *) malloc( (*tinPP)->memFeatureTable * sizeof(DTM_FEATURE_TABLE)) ;
    if( (*tinPP)->fTableP == nullptr )
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
       goto errexit ; 
      }
   } 
/*
** Allocate Memory For Tin Feature List Array
*/
 if( (*tinPP)->memFeatureList > 0 )
   {
    (*tinPP)->fListP = (DTM_FEATURE_LIST_VER200 *) malloc( (*tinPP)->memFeatureList * sizeof(DTM_FEATURE_LIST_VER200)) ;
    if( (*tinPP)->fListP == nullptr )
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
   }
/*
** Copy Points Array
*/
 if( (*tinPP)->memPts > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Points Array") ;
    for( point = 0 ; point < dtmP->memPoints ; ++point )
      {
       pointP = pointAddrP(dtmP,point) ;
       *((*tinPP)->pointsP+point) = *pointP ;
      } 
/*
**  Copy Nodes Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Nodes Array") ;
    for( node = 0 ; node < dtmP->memPoints ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       *((*tinPP)->nodesP+node) = *nodeP ;
      }
   }
/*
** Copy Circular List
*/
 if( (*tinPP)->cListPtr > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Clist Array") ;
    for( cList = 0 ; cList < dtmP->cListPtr ; ++cList )
      {
       cListP = clistAddrP(dtmP,cList) ;
       *((*tinPP)->cListP+cList) = *cListP ;
      }
   }
/*
** Copy Feature List Array
*/
 if( (*tinPP)->memFeatureList > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature List Array") ;
    for( fList = 0 ; fList < dtmP->memFlist ; ++fList )
      {
       fListP = flistAddrP(dtmP,fList) ;
//       *((*tinPP)->fListP+fList) = *fListP ;
       ((*tinPP)->fListP+fList)->nextPnt = fListP->nextPnt ;
       ((*tinPP)->fListP+fList)->nextPtr = fListP->nextPtr ;
       ((*tinPP)->fListP+fList)->dtmFeature = fListP->dtmFeature ;
      }
   }
/*
** Copy Feature Table Array
*/
 if( (*tinPP)->memFeatureTable > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Features Array") ;
    dtmTinFeature = 0 ;
    for( fTable = 0 ; fTable < dtmP->memFeatures ; ++fTable )
      {
       fTableP = ftableAddrP(dtmP,fTable) ;
       if( fTableP->dtmFeatureState == DTMFeatureState::Tin && fTableP->dtmFeatureType != DTMFeatureType::Hull )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature %8ld ** Type = %8ld ** dtmTinFeature = %8ld",fTable,fTableP->dtmFeatureType,dtmTinFeature ) ;
          if( dtmTinFeature != fTable )
            {
             for( fList = 0 ; fList < dtmP->memFlist ; ++fList )
              {
               if( ((*tinPP)->fListP+fList)->dtmFeature == fTable ) 
                 {
                  if( dbg ) bcdtmWrite_message(0,0,0,"ftable = %6ld dtmTinFeature = %6ld ** flist = %8ld dtmFeature = %8ld",fTable,dtmTinFeature,fList,((*tinPP)->fListP+fList)->dtmFeature) ;
                  ((*tinPP)->fListP+fList)->dtmFeature = dtmTinFeature   ;
                 }
              }
            }
          tinFeature.dtmFeatureType    = fTableP->dtmFeatureType ;
          tinFeature.firstPnt          = fTableP->dtmFeaturePts.firstPoint ;
          tinFeature.internalToFeature = fTableP->internalToDtmFeature ;
          tinFeature.userTag           = fTableP->dtmUserTag ;
          *((*tinPP)->fTableP+dtmTinFeature)    = tinFeature ;
          ++dtmTinFeature ;
         } 
      }
   }

/*
** Check Tin Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Check Tin Object") ;
    if( bcdtmCheck_integrityTinObject(*tinPP))
      {
       bcdtmWrite_message(0,0,0,"Tin Object Corrupted") ;
       goto errexit ;
      }  
    bcdtmWrite_message(0,0,0,"Tin Object Valid") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM Object To Tin Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM Object To Tin Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *tinPP  != nullptr ) bcdtmObject_deleteTinObject(tinPP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmGeopak_convertTinFileToDtmFile
(
 WCharCP tinFileNameP,
 WCharCP dtmFileNameP,
 double ppTol
)
/*
** This Function Converts Geopak Tin Files To Bentley Civil DTM Files (*.bcDTM )
*/
{
 long    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    hullFeature,dtmFeature,numHullPts ;
 double  plTol ;
 DPoint3d     *hullPtsP=nullptr ;
 BC_DTM_OBJ  *dtmP=nullptr ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise Variables
*/
 if(  dbg )
   { 
    bcdtmWrite_message(0,0,0,"Converting Tin File To DTM File")     ;
    bcdtmWrite_message(0,0,0,"tinFileNameP      = %s",tinFileNameP) ;
    bcdtmWrite_message(0,0,0,"dtmFileNameP      = %s",dtmFileNameP) ;
    bcdtmWrite_message(0,0,0,"ppTol             = %15.12lf",ppTol)  ;
   }
/*
** Destroy Current Geopak Tin Object
*/
// if( DTM_CTOBJ != nullptr ) if( bcdtmObject_destroyDtmObject((BC_DTM_OBJ **)&DTM_CTOBJ)) goto errexit ;
// DTM_CTOBJ = nullptr ;
// DTM_CTOBJ_FILE[0] = 0 ;
/*
** Read Tin File
*/
 if( bcdtmRead_fromFileDtmObject(&dtmP,tinFileNameP)) goto errexit ;
/*
** Set Tolerances
*/
 if( dtmP->ppTol == 0.0 ) plTol = ppTol ;
 else                    
   { 
    ppTol = dtmP->ppTol ;
    plTol = dtmP->plTol ;
   }
 if( plTol < ppTol ) plTol = ppTol ;
/*
** Write Tin Tolerances
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"DTM Tolerances") ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints   = %7ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol       = %15.12lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol       = %15.12lf",dtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol      = %15.12lf",dtmP->mppTol) ;
    bcdtmWrite_message(0,0,0,"ppTol             = %15.12lf",ppTol) ;
    bcdtmWrite_message(0,0,0,"plTol             = %15.12lf",plTol) ;
   }
/*
** Check Hull Feature Is In DTM If Not Add It Before Triangulating
*/
 hullFeature = FALSE ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && hullFeature == FALSE ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull) hullFeature = TRUE;
   }
 if( hullFeature == FALSE )
   {
    if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Hull,dtmP->nullUserTag,1,&dtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
   }
/*
** Assign Feature Id's To DTM Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureId == dtmP->nullFeatureId)
      {
       dtmFeatureP->dtmFeatureId = dtmP->dtmFeatureIndex ;
       ++dtmP->dtmFeatureIndex ;
      }
   }
/*
** Retriangulate
*/
 bcdtmWrite_message(0,0,0,"Triangulating DTM Object") ;
 if( bcdtmObject_setTriangulationParametersDtmObject(dtmP,ppTol,plTol,1,0.0)) goto errexit ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Write DTM Object To File
*/
 if( bcdtmWrite_toFileDtmObject(dtmP,dtmFileNameP)) goto errexit ;
/*
** Set Current Tin Object
*/
// DTM_CTOBJ = (DTM_TIN_OBJ *) dtmP ;
// if( *dtmFileNameP != 0 ) wcscpy(DTM_CTOBJ_FILE,dtmFileNameP) ;
// else                     wcscpy(DTM_CTOBJ_FILE,L"MEMORY.DTMFeatureState::Tin") ;
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != nullptr ) { free(hullPtsP) ; hullPtsP = nullptr ; }
/* 
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Converting Tin File To DTM File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Converting Tin File To DTM File Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmGeopak_reportDuplicatePointErrorsDtmFile
(
 WCharCP dtmFileNameP,
 WCharCP reportFileNameP
)
/*
** This Function Reports Duplicate Points In a Data File
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 char    dateStr[50],timeStr[50] ;
 FILE    *rptFP=nullptr ;
 BC_DTM_OBJ *dtmP=nullptr;
 DTM_GEOPAK_REPORT geopakReport ; 
/*
** Initialise
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Reporting Duplicate Point Errors") ;
    bcdtmWrite_message(0,0,0,"dtmFileNameP    = %s",dtmFileNameP) ;
    bcdtmWrite_message(0,0,0,"reportFileNameP = %s",reportFileNameP) ;
   }
/*
** Read Geopak Dat File
*/
 if( bcdtmRead_geopakDatFileToDtmObject(&dtmP,dtmFileNameP))  goto errexit ;
/*
** Open Report File
*/
 rptFP = bcdtmFile_open(reportFileNameP,L"w") ;
 if( rptFP == nullptr ) 
  { 
   bcdtmWrite_message(1,0,0,"Error Opening Report File") ; 
   goto errexit ; 
  }
/*
** Write Report
*/
 fprintf(rptFP,"**********************************************************\n") ;
 fprintf(rptFP,"**\n") ;
 fprintf(rptFP,"**  Bentley Civil Duplicate Point Error Report\n") ;
 fprintf(rptFP,"**\n") ;
 fprintf(rptFP,"**********************************************************\n") ;
 fprintf(rptFP,"**\n") ;
 bcdtmUtl_getDateAndTime(dateStr,timeStr) ;
 fprintf(rptFP,"**  %s\n",dateStr) ;
 fprintf(rptFP,"**\n") ;
 fprintf(rptFP,"**  Data File : %ls\n",dtmFileNameP) ;
 fprintf(rptFP,"**\n") ;
/*
** Write Data File Statistics
*/
 fprintf(rptFP,"**  Number of Data Points = %6ld\n",dtmP->numPoints) ;
 fprintf(rptFP,"**      Minimum      Maximum       Range\n") ;
 fprintf(rptFP,"**    ============ ============ ============\n") ;
 fprintf(rptFP,"**  x %12.3lf %12.3lf %12.3lf\n",dtmP->xMin,dtmP->xMax,dtmP->xMax - dtmP->xMin) ;
 fprintf(rptFP,"**  y %12.3lf %12.3lf %12.3lf\n",dtmP->yMin,dtmP->yMax,dtmP->yMax - dtmP->yMin) ;
 fprintf(rptFP,"**  z %12.3lf %12.3lf %12.3lf\n",dtmP->zMin,dtmP->zMax,dtmP->zMax - dtmP->zMin) ;
 fprintf(rptFP,"**\n") ;
/*
** Report Duplicate Point Errors
*/
 geopakReport.dtmP          = dtmP ;
 geopakReport.fileP         = rptFP ;
 geopakReport.numErrors     = 0 ;
 geopakReport.loadFunctionP = nullptr ;
 if( bcdtmReport_duplicatePointErrorsDtmObject(dtmP,bcdtmGeopak_browseDuplicatePointErrors,&geopakReport)) goto errexit ;
 fprintf(rptFP,"**\n") ;
 fprintf(rptFP,"** %6ld Duplicate Point Errors Detected\n",geopakReport.numErrors) ;
 fprintf(rptFP,"**\n") ;
/*
** Cleanup
*/
 cleanup :
 if( rptFP != nullptr ) { fclose(rptFP) ; rptFP = nullptr ; }
 if( dtmP  != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
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
BENTLEYDTM_Private int bcdtmGeopak_browseDuplicatePointErrors(double x,double y,DTM_DUPLICATE_POINT_ERROR *dupPtsP,long numDupPts,void* userArgs) 
{
 DTM_GEOPAK_REPORT *userP = (DTM_GEOPAK_REPORT *)userArgs;
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 FILE *rptFP=nullptr ;
 char dtmFeatureTypeName[100] ;
 DTM_DUPLICATE_POINT_ERROR *dupP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Browsing Duplicate Point Errors") ;
/*
** Write Errors To Report File
*/
 rptFP = userP->fileP ;
 userP->numErrors = userP->numErrors + numDupPts ; 
 fprintf(rptFP,"**\n** Point x = %12.5lf y = %12.5lf Has %4ld Duplicate Point Errors\n",x,y,numDupPts) ;
 for( dupP = dupPtsP ; dupP < dupPtsP + numDupPts ; ++dupP )
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dupP->dtmFeatureType,dtmFeatureTypeName) ;
    if( fprintf(rptFP,"**** z = %12.5lf  Dtm Feature Type = %s\n",dupP->z,dtmFeatureTypeName) == -1 )
      {
       bcdtmWrite_message(1,0,0,"Error Writing File") ;
       goto errexit ;
      }
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Browsing Duplicate Point Errors Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Browsing Duplicate Point Errors Error") ;
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
BENTLEYDTM_EXPORT int bcdtmGeopak_reportCrossingFeaturesDtmFile
(
 WCharCP dtmFileNameP,
 WCharCP reportFileNameP,
 DTMFeatureCallback loadFunctionP
)
/*
** This Function Reports Crossing Features In A Geopak Dat File
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 char    dateStr[50],timeStr[50] ;
 DTMFeatureType    crossingFeatureList[2];
 long numCrossingFeatureList = 2;
 FILE    *rptFP=nullptr ;
 BC_DTM_OBJ *dtmP=nullptr;
 DTM_GEOPAK_REPORT geopakReport ; 
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Reporting Crossing Features") ;
    bcdtmWrite_message(0,0,0,"dtmFileNameP    = %s",dtmFileNameP) ;
    bcdtmWrite_message(0,0,0,"reportFileNameP = %s",reportFileNameP) ;
   }
/*
** Initialise
*/
 crossingFeatureList[0] = DTMFeatureType::Breakline ;
 crossingFeatureList[1] = DTMFeatureType::ContourLine ;
/*
** Read Geopak Dat File
*/
 if( bcdtmRead_geopakDatFileToDtmObject(&dtmP,dtmFileNameP))  goto errexit ;
/*
** Open Report File
*/
 rptFP = bcdtmFile_open(reportFileNameP,L"w") ;
 if( rptFP == nullptr ) 
  { 
   bcdtmWrite_message(1,0,0,"Error Opening Report File") ; 
   goto errexit ; 
  }
/*
** Write Report
*/
 fprintf(rptFP,"**********************************************************\n") ;
 fprintf(rptFP,"**\n") ;
 fprintf(rptFP,"**  Bentley Civil Crossing Feature Error Report\n") ;
 fprintf(rptFP,"**\n") ;
 fprintf(rptFP,"**********************************************************\n") ;
 fprintf(rptFP,"**\n") ;
 bcdtmUtl_getDateAndTime(dateStr,timeStr) ;
 fprintf(rptFP,"**  %s\n",dateStr) ;
 fprintf(rptFP,"**\n") ;
 fprintf(rptFP,"**  Data File : %ls\n",dtmFileNameP) ;
 fprintf(rptFP,"**\n") ;
/*
** Write Data File Statistics
*/
 fprintf(rptFP,"**  Number of Data Points = %6ld\n",dtmP->numPoints) ;
 fprintf(rptFP,"**      Minimum      Maximum       Range\n") ;
 fprintf(rptFP,"**    ============ ============ ============\n") ;
 fprintf(rptFP,"**  x %12.3lf %12.3lf %12.3lf\n",dtmP->xMin,dtmP->xMax,dtmP->xMax - dtmP->xMin) ;
 fprintf(rptFP,"**  y %12.3lf %12.3lf %12.3lf\n",dtmP->yMin,dtmP->yMax,dtmP->yMax - dtmP->yMin) ;
 fprintf(rptFP,"**  z %12.3lf %12.3lf %12.3lf\n",dtmP->zMin,dtmP->zMax,dtmP->zMax - dtmP->zMin) ;
 fprintf(rptFP,"**\n") ;
/*
** Report Crossing Features 
*/
 geopakReport.dtmP      = dtmP ;
 geopakReport.fileP     = rptFP ;
 geopakReport.numErrors = 0 ;
 geopakReport.loadFunctionP = loadFunctionP ;
 if( bcdtmReport_crossingFeaturesDtmObject(dtmP,crossingFeatureList,numCrossingFeatureList,bcdtmGeopak_browseCrossingFeatures,&geopakReport)) goto errexit ;
 fprintf(rptFP,"**\n") ;
 fprintf(rptFP,"** %6ld Crossing Feature Errors Detected\n",geopakReport.numErrors) ;
 fprintf(rptFP,"**\n") ;
/*
** Cleanup
*/
 cleanup :
 if( rptFP != nullptr ) { fclose(rptFP) ; rptFP = nullptr ; }
 if( dtmP  != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reporting Crossing Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reporting Crossing Features Error") ;
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
BENTLEYDTM_Private int bcdtmGeopak_browseCrossingFeatures
(
 DTM_CROSSING_FEATURE_ERROR &crossError,
 void *userArgs
) 
{
 DTM_GEOPAK_REPORT *userP = (DTM_GEOPAK_REPORT *)userArgs;
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long found,numFeaturePts,dtmFeature ;
 DPoint3d  *p3dP,*featurePtsP=nullptr ;
 FILE *rptFP=nullptr ;
 char dtmFeatureTypeName[100] ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Browsing Crossing Features ") ;
    bcdtmWrite_message(0,0,0,"crossError  = %p",&crossError) ;
    bcdtmWrite_message(0,0,0,"userP       = %p",userP) ;
    bcdtmWrite_message(0,0,0,"geopakReport.dtmP          = %p",userP->dtmP) ;
    bcdtmWrite_message(0,0,0,"geopakReport.fileP         = %p",userP->fileP) ;
    bcdtmWrite_message(0,0,0,"geopakReport.numErrors     = %8ld",userP->numErrors) ;
    bcdtmWrite_message(0,0,0,"geopakReport.loadFunctionP = %p",userP->loadFunctionP) ;
   } 
/*
** Write Errors To Report File
*/
 rptFP = userP->fileP ;
 ++userP->numErrors   ; 
 fprintf(rptFP,"**\n**  Intersecting Features At %12.5lf %12.5lf\n",crossError.intersectionX,crossError.intersectionY) ;
 if( crossError.dtmFeatureType1 == DTMFeatureType::Breakline   ) strcpy(dtmFeatureTypeName,"Break Line") ;
 if( crossError.dtmFeatureType1 == DTMFeatureType::ContourLine ) strcpy(dtmFeatureTypeName,"Contour Line") ;
 fprintf(rptFP,"****  Feature 1 ** ID = %10I64d ** Segment = %6ld ** z = %12.5lf ** %s\n",crossError.dtmFeatureId1,crossError.segmentOfset1,crossError.elevation1,dtmFeatureTypeName) ;
 if( crossError.dtmFeatureType2 == DTMFeatureType::Breakline   ) strcpy(dtmFeatureTypeName,"Break Line") ;
 if( crossError.dtmFeatureType2 == DTMFeatureType::ContourLine ) strcpy(dtmFeatureTypeName,"Contour Line") ;
 fprintf(rptFP,"****  Feature 2 ** ID = %10I64d ** Segment = %6ld ** z = %12.5lf ** %s\n",crossError.dtmFeatureId2,crossError.segmentOfset2,crossError.elevation2,dtmFeatureTypeName) ;
/*
** Write Errors To Log File
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**\n**  Intersecting Features At %12.5lf %12.5lf",crossError.intersectionX,crossError.intersectionY) ;
    bcdtmWrite_message(0,0,0,"****  Feature 1 ** ID = %10I64d ** Segment = %6ld ** z = %12.5lf ** %s",crossError.dtmFeatureId1,crossError.segmentOfset1,crossError.elevation1,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"****  Feature 2 ** ID = %10I64d ** Segment = %6ld ** z = %12.5lf ** %s",crossError.dtmFeatureId2,crossError.segmentOfset2,crossError.elevation2,dtmFeatureTypeName) ;
   }
/*
** Load Features
*/
 if( userP->loadFunctionP != nullptr )
   {
    found = FALSE ;
    for( dtmFeature = 0 ; dtmFeature < userP->dtmP->numFeatures && found == FALSE ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(userP->dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureId == crossError.dtmFeatureId1 )
         {
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(userP->dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( userP->loadFunctionP((DTMFeatureType)dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts,nullptr)) goto errexit ;
          if( dbg == 2 )
            {
             bcdtmWrite_message(0,0,0,"Number Of Points For Feature Id %10I64d = %6ld",dtmFeatureP->dtmFeatureId,numFeaturePts) ;
             for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
               {
                bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
               }
            } 
          if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
          found = TRUE ; 
         }
      }
    found = FALSE ;
    for( dtmFeature = 0 ; dtmFeature < userP->dtmP->numFeatures && found == FALSE ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(userP->dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureId == crossError.dtmFeatureId2 )
         {
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(userP->dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( dbg == 2 )
            {
             bcdtmWrite_message(0,0,0,"Number Of Points For Feature Id %10I64d = %6ld",dtmFeatureP->dtmFeatureId,numFeaturePts) ;
             for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
               {
                bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
               }
            } 
          if( userP->loadFunctionP((DTMFeatureType)dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts,nullptr)) goto errexit ;
          if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
          found = TRUE ; 
         }
      }
   }
/*
** Cleanup
*/
 cleanup :
 if( featurePtsP != nullptr ) { free(featurePtsP) ; featurePtsP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Browsing Crossing Features Errors Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Browsing Crossing Features Errors Error") ;
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
BENTLEYDTM_EXPORT int bcdtmGeopak_destroyAllExceptCurrentDtmObjects(void)
/*
** This Function Destroys All Dtm Objects
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,numNodeValues ;
 BC_DTM_OBJ *dtmP=nullptr,**nodeDtmPP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Destroying All Dtm Objects") ;
/*
** Check DTM Objects Exist
*/
 if( glbDtmObjBtreeP != nullptr )
   {
/*
**  Get Array Of Dtm Objects
*/
    if( bcdtmBtree_getArrayOfNodeValues(glbDtmObjBtreeP,&nodeDtmPP,&numNodeValues)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Dtm Objects = %8ld",numNodeValues) ;
/*
**  Scan Dtm Object Array List And Destroy Objects
*/
    for( n = 0 ; n < numNodeValues ; ++n )
      {
       dtmP =  *(nodeDtmPP+n) ;
//       if( dtmP != (BC_DTM_OBJ *) DTM_CDOBJ && dtmP != (BC_DTM_OBJ *) DTM_CTOBJ )
//         { 
          if( dbg ) bcdtmWrite_message(0,0,0,"Destroying Dtm Object[%6ld] = %p",n,dtmP) ;
          if( bcdtmObject_destroyDtmObject(&dtmP)) goto errexit ;
//         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( nodeDtmPP != nullptr ) free(nodeDtmPP) ;
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
BENTLEYDTM_EXPORT int bcdtmGeopak_moveFirstOccurrenceOfDtmFeatureTypeToPointArrayDtmObject
(
 BC_DTM_OBJ   *dtmP,
 DTMFeatureType dtmFeatureType,
 DTMUserTag *userTagP,
 DPoint3d          **featurePtsPP,
 long         *numFeaturePtsP
)
/*
** This Function Extracts And Removes A DTM Features From A Data Object
** Note ** If featurePtsPP is not nullptr , featurePtsPP memory will be freed 
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    dtmFeature,featureFound ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Move To Point Array First Occurrence Of Dtm Feature Type") ;
/*
** Initialise
*/
 *userTagP = DTM_NULL_USER_TAG ;
 *numFeaturePtsP = 0 ;
 if( *featurePtsPP != nullptr ) { free(*featurePtsPP) ; *featurePtsPP = nullptr ; }
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) )  goto errexit  ; 
/*
** Scan Features
*/
 featureFound = FALSE ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && featureFound == FALSE ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == dtmFeatureType && dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted )
      {
       *userTagP = dtmFeatureP->dtmUserTag ;
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,featurePtsPP,numFeaturePtsP)) goto errexit ;
       if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
       featureFound = TRUE ;
      } 
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
BENTLEYDTM_EXPORT int  bcdtmGeopak_deleteHullFromDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Deletes The Hull From A DTM Object
*/
{
 int ret=DTM_SUCCESS ;
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** DTM Must Be In Data State
*/
 if( dtmP->dtmState == DTMState::Data )
   {
    if( bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Hull)) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmGeopak_getPositiveUserTagListDtmObject
(
 BC_DTM_OBJ   *dtmP,
 DTMUserTag **userTagListPP,
 long         *numUserTagsP 
 )
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFeature,memUserTags=0,memUserTagsInc=1000 ;
 char dtmFeatureTypeName[100] ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *numUserTagsP = 0 ;
 if( *userTagListPP != nullptr ) { free(*userTagListPP) ; *userTagListPP = nullptr ; }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** DTM Must Be In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   } 
/*
** Scan DTM Features And Store All Positive User Tags
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmUserTag != DTM_NULL_USER_TAG && dtmFeatureP->dtmUserTag > 0 )
      {
/*
**     Write User Tag To Log File
*/
       if( dbg )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
          bcdtmWrite_message(0,0,0,"dtmFeature[%8ld] = %20s * %9ld * %12I64d",dtmFeature,dtmFeatureTypeName,dtmFeatureP->dtmFeaturePts.firstPoint,dtmFeatureP->dtmUserTag) ;
         }
/*
**     Check Memory For Storing User Tags
*/
       if( *numUserTagsP == memUserTags )
         {
          memUserTags = memUserTags + memUserTagsInc ;
          if( *userTagListPP == nullptr ) *userTagListPP = ( DTMUserTag * ) malloc( memUserTags * sizeof(DTMUserTag)) ;
          else                         *userTagListPP = ( DTMUserTag * ) realloc(*userTagListPP,memUserTags * sizeof(DTMUserTag)) ;
          if( *userTagListPP == nullptr ) 
            {
             bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Store User Tag
*/
       *(*userTagListPP+*numUserTagsP) = dtmFeatureP->dtmUserTag ;
       ++*numUserTagsP ;
      }
   }
/*
** Realloc Memory
*/
 if( memUserTags > *numUserTagsP )
   {
    if( *numUserTagsP > 0 )  *userTagListPP = ( DTMUserTag * ) realloc(*userTagListPP,*numUserTagsP * sizeof(DTMUserTag)) ;
    else if( *userTagListPP != nullptr ) { free(*userTagListPP) ; *userTagListPP = nullptr ; }
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
BENTLEYDTM_EXPORT int bcdtmExport_geopakTriangulationFromDtmObject
(
 BC_DTM_OBJ *dtmP,                  /* ==> Pointer To DTM Object To Export    */
 WCharCP tinFileNameP           /* ==> Geopak Tin File Name To Write To   */ 
)
/*
** This Function Exports A Triangulated DTM Object To A Geopak Tin File
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 FILE *tinFP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Exporting Triangulated DTM To Geopak Tin File") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"tinFileNameP = %s",tinFileNameP) ;
   }
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** DTM Must Be In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check Triangulation
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking DTM Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(1,0,0,"DTM Triangulation Invalid") ;
       goto errexit ;
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"DTM Triangulation Valid") ;
   }
/*
** Clean Dtm Object
*/
 if( dtmP->numPoints != dtmP->memPoints )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning DTM Object") ;
    if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
   }
/*
** Open Binary Tin File
*/
 tinFP = bcdtmFile_open(tinFileNameP,L"wb") ;
 if( tinFP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Geopak Tin File") ;
    goto errexit ;
   } 
/*
** Write To Geopak File
*/
 if( bcdtmWrite_writeAtFilePositionGeopakTinObjectDtmObject(dtmP,tinFP,0)) goto errexit ;
/*
** Close File
*/
 fclose(tinFP) ;
 tinFP = nullptr ;
/*
** Clean Up
*/
 cleanup :
 if( tinFP != nullptr ) { fclose(tinFP) ; tinFP = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Exporting Triangulated DTM To Geopak Tin File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Exporting Triangulated DTM To Geopak Tin File Error") ;
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
BENTLEYDTM_EXPORT int bcdtmImport_geopakTriangulationToDtmObject
(
 BC_DTM_OBJ **dtmPP,                /* ==> Pointer To DTM Object To Import     */
 WCharCP tinFileNameP           /* ==> Geopak Tin File Name To Read From   */ 
)
/*
** This Function Import A Geopak Tin File To A DTM Object
*/
{
 int ret=DTM_SUCCESS ;
/*
** Read Geopak Tin File
*/
 if( bcdtmRead_fromFileDtmObject(dtmPP,tinFileNameP)) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmImport_geopakTriangulationToExistingDtmObject
(
 BC_DTM_OBJ *dtmP,                 /* ==> Pointer To DTM Object To Import     */
 WCharCP tinFileNameP          /* ==> Geopak Tin File Name To Read From   */ 
)
/*
** This  Function Imports A Geopak Tin File To An DTM Object
*/
{
 int ret=DTM_SUCCESS ;
/*
** Read Geopak Tin File
*/
 if( bcdtmGeopak_readFromFileDtmObject(dtmP,tinFileNameP)) goto errexit ;
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
BENTLEYDTM_Private int bcdtmGeopak_readFromFileDtmObject(BC_DTM_OBJ *dtmP,WCharCP dtmFileNameP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 FILE *dtmFP=nullptr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm File %s",dtmFileNameP) ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP))
   {
    bcdtmWrite_message(2,0,0,"Not A Valid DTM Object") ;
    goto errexit ;
   }
/*
** Open Dtm File For Reading
*/
 dtmFP = bcdtmFile_open(dtmFileNameP,L"rb") ;
 if( dtmFP == nullptr )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Dtm File %s For Reading",dtmFileNameP) ;
    goto errexit ; 
   }
/*
** Read DTM Object At Zero File Position
*/
 if( bcdtmGeopak_readAtFilePositionDtmObject(dtmP,dtmFP,0)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( dtmFP != nullptr ) fclose(dtmFP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Dtm File %s Completed",dtmFileNameP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Dtm File %s Error",dtmFileNameP) ;
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
BENTLEYDTM_Private int bcdtmGeopak_readAtFilePositionDtmObject(BC_DTM_OBJ *dtmP,FILE *dtmFP,long filePosition)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 DTMObjectTypes dtmObjType;
 long objType,verNum;
 char buffer[8] ;
 DTM_DAT_OBJ     *dataP=nullptr ;
 DTM_TIN_OBJ     *tinP=nullptr ;
/*
** Write Entry Message
*/
 bcdtmWrite_message(0,0,0,"Geopak - Reading At File Position %8ld Dtm Object",filePosition) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Geopak - Reading At File Position %8ld Dtm Object",filePosition) ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP))
   {
    bcdtmWrite_message(2,0,0,"Not A Valid DTM Object") ;
    goto errexit ;
   }
/*
** Test For None nullptr File Pointer
*/
 if( dtmFP == nullptr )
   {
    bcdtmWrite_message(2,0,0,"Null File Pointer") ;
    goto errexit ;
   }
/*
** Check Lower Value Of File Position
*/
 if( filePosition < 0 )
   {
    bcdtmWrite_message(2,0,0,"File Position Range Error") ;
    goto errexit ;
   }
/*
** Seek To File Position
*/
 if( fseek(dtmFP,filePosition,SEEK_SET))
   {
    bcdtmWrite_message(1,0,0,"File Seek Error") ;
    goto errexit ; 
   }
/*
** Initialise DTM Object
*/
 dtmObjType = dtmP->dtmObjType ;
 bcdtmObject_initialiseDtmObject(dtmP) ;
 dtmP->dtmObjType = dtmObjType ;
/*
** Read File Type And Version Number
*/
 if( fread(buffer,8,1,dtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Not A Bentley Civil Dtm File") ;
    goto errexit ;
   }
/*
** Reposition File Pointer
*/
 fseek(dtmFP,-8,SEEK_CUR) ; 
/*
** Get Object Type And Version Number ;
*/  
 memcpy(&objType,&buffer[0],4) ;
 memcpy(&verNum,&buffer[4],4)  ;
/*
** Read DTM Object Type
*/
 switch( objType )
   {
    case DTM_DAT_TYPE :
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Geopak Data Object") ;
       if( bcdtmRead_atFilePositionDataObject(&dataP,dtmFP,filePosition)) goto errexit ;
       if( bcdtmGeopak_copyDataObjectToDtmObject(dataP,dtmP)) goto errexit ;
       if( bcdtmObject_deleteDataObject(&dataP)) goto errexit ;
    break ;

    case DTM_TIN_TYPE :
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Geopak Tin Object") ;
       if( bcdtmRead_atFilePositionTinObject(&tinP,dtmFP,filePosition)) goto errexit ;
       if( bcdtmGeopak_copyTinObjectToDtmObject(tinP,dtmP)) goto errexit ;
       if( bcdtmObject_deleteTinObject(&tinP)) goto errexit ;
       if( cdbg )
         {
          bcdtmWrite_message(0,0,0,"Checking DTM Object %p",dtmP) ;
          if( bcdtmCheck_tinComponentDtmObject(dtmP))
            {
             bcdtmWrite_message(2,0,0,"Checking DTM Object %p Error",dtmP) ;
             goto errexit ;
            }
          bcdtmWrite_message(2,0,0,"Checking DTM Object %p Completed",dtmP) ;
         } 
    break ;

    case BC_DTM_OBJ_TYPE :
    case BC_DTM_ELM_TYPE :
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Bentley Civil Dtm Object") ;
      switch( verNum )
        {
         case  BC_DTM_OBJ_VERSION_100 : 
           if( dbg ) bcdtmWrite_message(0,0,0,"BC_DTM_OBJ_VERSION_100") ;
           if( bcdtmRead_version100DtmObject(dtmP,dtmFP)) goto errexit ;
         break ;

         case  BC_DTM_OBJ_VERSION_200 : 
           if( dbg ) bcdtmWrite_message(0,0,0,"BC_DTM_OBJ_VERSION_200") ;
//TODO            if( bcdtmRead_version200DtmObject(dtmP,dtmFP)) goto errexit ;
         break ;

         case  BC_DTM_OBJ_VERSION :
           if( dbg ) bcdtmWrite_message(0,0,0,"BC_DTM_OBJ_VERSION") ;
            if( bcdtmRead_dtmObject(dtmP,dtmFP)) goto errexit ;
         break ;

         default :
            bcdtmWrite_message(1,0,0,"Incorrect Version For Bentley Civil Dtm Object") ;
            goto errexit ;
         break ;
        } ;
/*
**     Check DTM
*/
       if( cdbg == 2 )
         {
          bcdtmWrite_message(0,0,0,"Checking DTM Object %p",dtmP) ;
          if( bcdtmCheck_tinComponentDtmObject(dtmP))
            {
             bcdtmWrite_message(2,0,0,"Checking DTM Object %p Error",dtmP) ;
             goto errexit ;
            }
          bcdtmWrite_message(2,0,0,"Checking DTM Object %p Completed",dtmP) ;
         } 
    break ;

    default :
      bcdtmWrite_message(1,0,0,"Not A Bentley Civil Dtm Object Type") ;
      goto errexit ;
    break ;
   }  ;
/*
**  Update Modified Time
 */
 bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
 if( dataP != nullptr ) bcdtmObject_deleteDataObject(&dataP) ;
 if( tinP  != nullptr ) bcdtmObject_deleteTinObject(&tinP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading At File Position %8ld Dtm Object Completed",filePosition) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading At File Position %8ld Dtm Object Error",filePosition) ;
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
BENTLEYDTM_EXPORT int bcdtmGeopak_validateDtmLinearFeatureTypeDtmObject(BC_DTM_OBJ *dtmP,long fsCode,long fnCode, double ppTol)
{
 int  ret=DTM_SUCCESS ;
 long numErrors;
 DTMFeatureType  dtmFeatureType;
/*
** Get DTM Feature Type For Geopak Start And Finish Codes
*/
 if( bcdtmData_getAssociatedDtmFeatureTypeAndCodes(fsCode,&dtmFeatureType,&fsCode,&fnCode)) goto errexit ;
/*
** Validate Dtm Feature Type Occurrences
*/
 if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,dtmFeatureType,ppTol,&numErrors)) goto errexit ; 
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
BENTLEYDTM_EXPORT int bcdtmGeopak_moveDtmFeatureTypePointsToPointArrayDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,DTMUserTag *userTagP,DPoint3d **featPtsPP,long *numFeatPtsP) 
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFeature ;
 DPoint3d *p3dP ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Moving Dtm Feature Points To Point Array") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
   }
/*
** Initialise
*/
 *numFeatPtsP = 0 ;
 if( *featPtsPP != nullptr ) { free(*featPtsPP) ; *featPtsPP = nullptr ; }
 *userTagP = DTM_NULL_USER_TAG ;
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Scan To Feature Type
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && ! *numFeatPtsP ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == dtmFeatureType && dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted )
      {
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,featPtsPP,numFeatPtsP) ) goto errexit ;
       if( dbg ) 
         {
          bcdtmWrite_message(0,0,0,"Number Of Feature Points = %8ld",*numFeatPtsP) ; 
          for( p3dP = *featPtsPP ; p3dP < *featPtsPP + *numFeatPtsP ; ++p3dP )
            {
             bcdtmWrite_message(0,0,0,"Feature Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-*featPtsPP),p3dP->x,p3dP->y,p3dP->z) ; 
            } 
         }  
       *userTagP = dtmFeatureP->dtmUserTag ;
       if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
      } 
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Moving Dtm Feature Points To Point Array Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Moving Dtm Feature Points To Point Array Error") ;
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
BENTLEYDTM_EXPORT int bcdtmGeopak_copyTinStateDtmObjectToDataStateDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ *dataP )
/*
** This A Special Geopak Function To Copy A Tin State Dtm To A Data State Dtm
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long p1,p2,clc,dtmFeature,numFeatPts ;
 DPoint3d  tinLinePts[2],*featPtsP=nullptr ;
 BC_DTM_FEATURE *dtmFeatureP=nullptr ;
 DPoint3d  *pointP ;
 DTMFeatureId hullFeatureId ;
 DTMUserTag  hullUserTag ;
 DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Copying Tin State Dtm To Data State Dtm") ;
    bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dataP   = %p",dataP) ;
   } 
/*
** Check For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtmP))  goto errexit ;
 if( bcdtmObject_testForValidDtmObject(dataP)) goto errexit ;
/*
** Check Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated Dtm") ;
    goto errexit ;
   } 
/*
** Initialise Data State DTM
*/
 if( bcdtmObject_initialiseDtmObject(dataP)) goto errexit ;    
/*
**  Set Header Values
*/
 dataP->iniPoints       = dtmP->numPoints ;
 dataP->incPoints       = dtmP->incPoints ;
 dataP->dtmFeatureIndex = dtmP->dtmFeatureIndex ;
/*
**  Copy Tin Lines As Graphic Breaks
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
          pointP = pointAddrP(dtmP,p1) ;
          tinLinePts[0].x = pointP->x ; tinLinePts[0].y = pointP->y ; tinLinePts[0].z = pointP->z ; 
          pointP = pointAddrP(dtmP,p2) ;
          tinLinePts[1].x = pointP->x ; tinLinePts[1].y = pointP->y ; tinLinePts[1].z = pointP->z ; 
          if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::GraphicBreak,DTM_NULL_USER_TAG,1,&nullFeatureId,tinLinePts,2)) goto errexit ;  
         }
      } 
   }
/*
** Write Tin Features To Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Exporting Tin Features") ;
 hullUserTag   = dtmP->nullUserTag ;
 hullFeatureId = dtmP->nullFeatureId ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull ) 
         {
          hullUserTag   = dtmFeatureP->dtmUserTag ; 
          hullFeatureId = dtmFeatureP->dtmFeatureId ;
         }
       else
         {
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featPtsP,&numFeatPts)) goto errexit ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,featPtsP,numFeatPts)) goto errexit ;  
          if( featPtsP != nullptr )  { free(featPtsP) ; featPtsP = nullptr ; }
         }
      }
   }
/*
** Write Tin Hull To DTM Object As Boundary Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Exporting Tin Hull") ;
 if( bcdtmList_extractHullDtmObject(dtmP,&featPtsP,&numFeatPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(dataP,DTMFeatureType::Hull,hullUserTag,1,&hullFeatureId,featPtsP,numFeatPts)) goto errexit ;  
 if( featPtsP != nullptr )  { free(featPtsP) ; featPtsP = nullptr ; }
/*
** Clean Up
*/
 cleanup :
 if( featPtsP != nullptr ) free(featPtsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Tin State Dtm To Data State Dtm Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Tin State Dtm To Data State Dtm") ;
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
BENTLEYDTM_EXPORT int bcdtmData_getAssociatedDtmFeatureTypeAndCodes(long featureCode,DTMFeatureType* dtmFeatureTypeP,long *startCodeP,long *nextCodeP) 
/*
** This Function Gets The Associated Dtm Feature Type And Start And Next Codes For A featureCode
*/
{
 int  ret=DTM_ERROR ;
/*
** Initialise
*/
 *dtmFeatureTypeP = (DTMFeatureType) (*startCodeP = *nextCodeP = 0) ;
/*
** Assign Feature Type , Start And Next Codes
*/
 if( featureCode ==  1                      ) { *dtmFeatureTypeP = DTMFeatureType::RandomSpots   ; *startCodeP =  1 ; *nextCodeP =  1 ; ret = DTM_SUCCESS ; }
 else if( featureCode == 22 || featureCode == 23 ) { *dtmFeatureTypeP = DTMFeatureType::GroupSpots    ; *startCodeP = 22 ; *nextCodeP = 23 ; ret = DTM_SUCCESS ; }
 else if( featureCode ==  2 || featureCode ==  3 ) { *dtmFeatureTypeP = DTMFeatureType::Breakline    ; *startCodeP =  2 ; *nextCodeP =  3 ; ret = DTM_SUCCESS ; }
 else if( featureCode == 50 || featureCode == 51 ) { *dtmFeatureTypeP = DTMFeatureType::SoftBreakline    ; *startCodeP = 50 ; *nextCodeP = 51 ; ret = DTM_SUCCESS ; }
 else if( featureCode ==  4                      ) { *dtmFeatureTypeP = DTMFeatureType::Hull          ; *startCodeP =  4 ; *nextCodeP =  4 ; ret = DTM_SUCCESS ; }
 else if( featureCode == 52 || featureCode == 53 ) { *dtmFeatureTypeP = DTMFeatureType::DrapeHull    ; *startCodeP = 52 ; *nextCodeP = 53 ; ret = DTM_SUCCESS ; }
 else if( featureCode ==  5 || featureCode ==  6 ) { *dtmFeatureTypeP = DTMFeatureType::ContourLine  ; *startCodeP =  5 ; *nextCodeP =  6 ; ret = DTM_SUCCESS ; } 
 else if( featureCode ==  7 || featureCode ==  8 ) { *dtmFeatureTypeP = DTMFeatureType::Void          ; *startCodeP =  7 ; *nextCodeP =  8 ; ret = DTM_SUCCESS ; }
 else if( featureCode ==  9 || featureCode == 10 ) { *dtmFeatureTypeP = DTMFeatureType::Island        ; *startCodeP =  9 ; *nextCodeP = 10 ; ret = DTM_SUCCESS ; }
 else if( featureCode == 11 || featureCode == 12 ) { *dtmFeatureTypeP = DTMFeatureType::Hole          ; *startCodeP = 11 ; *nextCodeP = 12 ; ret = DTM_SUCCESS ; }      
 else if( featureCode == 13 || featureCode == 14 ) { *dtmFeatureTypeP = DTMFeatureType::GraphicBreak ; *startCodeP = 13 ; *nextCodeP = 14 ; ret = DTM_SUCCESS ; }      
 else if( featureCode == 15 || featureCode == 16 ) { *dtmFeatureTypeP = DTMFeatureType::DrapeVoid    ; *startCodeP = 15 ; *nextCodeP = 16 ; ret = DTM_SUCCESS ; }
 else if( featureCode == 17 || featureCode == 18 ) { *dtmFeatureTypeP = DTMFeatureType::BreakVoid    ; *startCodeP = 17 ; *nextCodeP = 18 ; ret = DTM_SUCCESS ; }
 else if( featureCode == 20 || featureCode == 21 ) { *dtmFeatureTypeP = DTMFeatureType::HullLine     ; *startCodeP = 20 ; *nextCodeP = 21 ; ret = DTM_SUCCESS ; }
 else if( featureCode == 30 || featureCode == 31 ) { *dtmFeatureTypeP = DTMFeatureType::VoidLine     ; *startCodeP = 30 ; *nextCodeP = 31 ; ret = DTM_SUCCESS ; }
 else if( featureCode == 32 || featureCode == 33 ) { *dtmFeatureTypeP = DTMFeatureType::HoleLine     ; *startCodeP = 32 ; *nextCodeP = 33 ; ret = DTM_SUCCESS ; }
 else if( featureCode == 40 || featureCode == 41 ) { *dtmFeatureTypeP = DTMFeatureType::SlopeToe     ; *startCodeP = 40 ; *nextCodeP = 41 ; ret = DTM_SUCCESS ; }
 else if( featureCode == 42 || featureCode == 43 ) { *dtmFeatureTypeP = DTMFeatureType::Polygon       ; *startCodeP = 42 ; *nextCodeP = 43 ; ret = DTM_SUCCESS ; }
/*
** Write Error Message
*/
 if( ret == DTM_ERROR ) bcdtmWrite_message(2,0,0,"Unknown Geopak Feature Code %2ld",featureCode) ;
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
BENTLEYDTM_Public int bcdtmCheck_integrityTinObject(DTM_TIN_OBJ *Tin)
/*
** This Function Checks The Integrity Of A Tin Object
**
*/
{
 long dbg=DTM_TRACE_VALUE(0) ; 
/*
** Check Topology
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
 if( bcdtmCheck_topologyTinObject(Tin,1,0)){ bcdtmWrite_message(1,0,0,"Tin Topology Invalid") ; goto errexit ; }
 if( dbg ) bcdtmWrite_message(0,0,0,"Tin Topology Valid") ;
/*
** Check Precision
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
 if( bcdtmCheck_precisionTinObject(Tin,0) ) { bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ; goto errexit ; }
 if( dbg ) bcdtmWrite_message(0,0,0,"Tin Precision Valid") ;
/*
** Check Topology DTM Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
 if( bcdtmCheck_topologyDtmFeaturesTinObject(Tin,0)){ bcdtmWrite_message(1,0,0,"Topolgy Error In Dtm Features") ; goto errexit ; }
 if( dbg ) bcdtmWrite_message(0,0,0,"Topology DTM Features Valid") ;
/*
** Check Sort Order Tin Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Sort Order") ;
 if( bcdtmCheck_sortOrderTinObject(Tin) ) { bcdtmWrite_message(1,0,0,"Sort Order Tin Object Invalid") ; goto errexit ; }
 if( dbg ) bcdtmWrite_message(0,0,0,"Tin Sort Order Valid") ;
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
| int bcdtmCheck_topologyTinObject()                                   |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCheck_topologyTinObject(DTM_TIN_OBJ *Tin,long MessageFlag,long WarnFlag)
/*
**
** This Function Checks The Topology Of the DTMFeatureState::Tin
** Return Values  = 0  No Error
**                > 0  Number Of Errors
**
*/ 
{

 int   err=0 ;
 long  p1,p2,p3,p4,clc ;
 DTM_TIN_NODE *pd ;
/*
** Process Circular List for Each Data Point
*/
 for( pd = Tin->nodesP ; pd < Tin->nodesP + Tin->numPts ; ++pd )
   {
    p1 = (long) ( pd - Tin->nodesP ) ;
    clc = pd->cPtr ;
    if( clc == Tin->nullPtr )
      {
       if( WarnFlag && MessageFlag )
         {
          bcdtmWrite_message(0,0,0,"Warning ** No Circular List For Point %6ld",p1) ;
         }
      }
    else if( clc < 0 || clc >= Tin->cListPtr )
      {
       bcdtmWrite_message(1,0,0,"Circular List Pointer Is Invalid for Point %6ld",p1) ;
       ++err  ;
      }
    else
      {
       p2 = (Tin->cListP+clc)->pntNum ;
       if( (p2 = bcdtmList_nextAntTinObject(Tin,p1,p2)) < 0 )
         {
          bcdtmWrite_message(2,0,0,"Circular List Corrupted Counter Clockwise ** %8ld %8ld",p1,p2) ;
          ++err ;
         }
       else
         {
          while( clc != Tin->nullPtr )
            {
             p3 = (Tin->cListP+clc)->pntNum ;
             p4 = bcdtmList_nextClockTinObject(Tin,p3,p1) ;
             if( p4 < 0 )
               {
                bcdtmWrite_message(0,0,0,"clc = %8ld  p3 = %6ld  p1 = %6ld",clc,p3,p1) ;
                bcdtmWrite_message(2,0,0,"Circular List Corrupted Clockwise ** %8ld %8ld",p1,p2) ;
                ++err ;
               }
             else if( (Tin->nodesP+p3)->hPtr != p1 && p4 != p2 )
               {
                bcdtmWrite_message(2,0,0,"Circular List Corrupted ** %8ld %8ld %8ld",p1,p2,p3) ;
                ++err  ;
               }
             p2 = p3 ;
             clc = (Tin->cListP+clc)->nextPtr ;
            }
         }
      }
   }
/*
** Print Number Of Errors
*/
 if( MessageFlag && err > 0 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Tin Topology Errors = %6d",err) ;
   }
/*
** Job Completed
*/
 return(err) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCheck_precisionTinObject(DTM_TIN_OBJ *Tin,long MessageFlag)
/*
** This Functions Checks The Precision Of A Tin
**
** Return Value = 0  No Precision Errors
**              = 1  System Error
**              = 2  Precision Errors
**
*/
{
 long   p1,p2,p3,clc,errcnt=0 ;
/*
** Scan Entire Tin
*/
 for ( p1 = 0 ; p1 < Tin->numPts ; ++p1 )
   {
    if( ( clc = (Tin->nodesP+p1)->cPtr ) != Tin->nullPtr )
      {
       if( ( p2 = bcdtmList_nextAntTinObject(Tin,p1,(Tin->cListP+clc)->pntNum)) < 0 ) return(1) ;
       while ( clc != Tin->nullPtr )
         {
          p3  = (Tin->cListP+clc)->pntNum ;
          clc = (Tin->cListP+clc)->nextPtr ;
          if( p2 > p1 && p3 > p1 && (Tin->nodesP+p1)->hPtr != p2 )
            {
             if( bcdtmMath_allPointSideOfTinObject(Tin,p1,p2,p3) >= 0 )
               {
                if( MessageFlag )
                  {
                   bcdtmWrite_message(1,0,0,"Precision Error ** Point = %8ld %8ld %8ld",p1,p2,p3) ;
                   bcdtmList_writeCircularListForPointTinObject(Tin,p1) ; 
                   bcdtmList_writeCircularListForPointTinObject(Tin,p2) ; 
                   bcdtmList_writeCircularListForPointTinObject(Tin,p3) ; 
                  }
                ++errcnt ;
               }
            }  
          p2 = p3 ;
         }
      }
   }
/*
** Check Tin Hull
*/
 if( ! errcnt )  errcnt = bcdtmCheck_forIntersectingTinHullLinesTinObject(Tin,MessageFlag) ;
/*
** Print Error Statistics
*/
 if( MessageFlag && errcnt > 0 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Tin Precision Errors = %8ld",errcnt) ;
   }
/*
** Job Completed
*/
 if( ! errcnt ) return(0) ;
 else           return(2) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
| bcdtmCheck_forIntersectingTinHullLinesTinObject()                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCheck_forIntersectingTinHullLinesTinObject(DTM_TIN_OBJ *Tin,long MessageFlag)
/*
** This Function Checks For Intersecting Tin Hull Lines
*/
{
 int    dbg=DTM_TRACE_VALUE(0),err,sd1,sd2 ;
 long   hp1,hp2,lp1,lp2 ;
 double xhn,xhm,yhn,yhm,xln,yln,xlm,ylm ;
/*
** Write Out Hull Points Develeopment Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Hull Points And Coordinates") ;
    bcdtmWrite_message(0,0,0,"===========================") ;
    hp1 = Tin->hullPnt ;
    do
      {
       bcdtmWrite_message(0,0,0,"%6ld ** %15.8lf %15.8lf %10.4lf",hp1,(Tin->pointsP+hp1)->x,(Tin->pointsP+hp1)->y,(Tin->pointsP+hp1)->z) ;
       hp1 = (Tin->nodesP+hp1)->hPtr ;
      } while ( hp1 != Tin->hullPnt ) ;
    bcdtmWrite_message(0,0,0,"%6ld ** %15.8lf %15.8lf %10.4lf",hp1,(Tin->pointsP+hp1)->x,(Tin->pointsP+hp1)->y,(Tin->pointsP+hp1)->z) ;
   }
/*
** Scan Hull And Check For Intersecting Tin Hull Lines
*/
 err=0 ;
 hp1 = Tin->hullPnt ;
 do
   {
    hp2 = (Tin->nodesP+hp1)->hPtr ;
    if( (Tin->pointsP+hp1)->x <= (Tin->pointsP+hp2)->x ) { xhn = (Tin->pointsP+hp1)->x ; xhm = (Tin->pointsP+hp2)->x ; }
    else                                           { xhm = (Tin->pointsP+hp1)->x ; xhn = (Tin->pointsP+hp2)->x ; }
    if( (Tin->pointsP+hp1)->y <= (Tin->pointsP+hp2)->y ) { yhn = (Tin->pointsP+hp1)->y ; yhm = (Tin->pointsP+hp2)->y ; }
    else                                           { yhm = (Tin->pointsP+hp1)->y ; yhn = (Tin->pointsP+hp2)->y ; }
    xhn = xhn - 0.001 ; yhn = yhn - 0.001 ;
    xhm = xhm + 0.001 ; yhm = yhm + 0.001 ; 
/*
** Scan All Hull Lines Looking For Intersection
*/
    lp1 = Tin->hullPnt ;
    do
      {
       lp2 = (Tin->nodesP+lp1)->hPtr ;
       if( lp1 != hp1 && lp1 != hp2 && lp2 != hp1 && lp2 != hp2 )
         {
          if( (Tin->pointsP+lp1)->x <= (Tin->pointsP+lp2)->x ) { xln = (Tin->pointsP+lp1)->x ; xlm = (Tin->pointsP+lp2)->x ; }
          else                                           { xlm = (Tin->pointsP+lp1)->x ; xln = (Tin->pointsP+lp2)->x ; }
          if( (Tin->pointsP+lp1)->y <= (Tin->pointsP+lp2)->y ) { yln = (Tin->pointsP+lp1)->y ; ylm = (Tin->pointsP+lp2)->y ; }
          else                                           { ylm = (Tin->pointsP+lp1)->y ; yln = (Tin->pointsP+lp2)->y ; }
          if( xln <= xhm && xlm >= xhn && yln <= yhm  && ylm >= yhn  )
            { 
             sd1 = bcdtmMath_sideOf((Tin->pointsP+hp1)->x,(Tin->pointsP+hp1)->y,(Tin->pointsP+hp2)->x,(Tin->pointsP+hp2)->y,(Tin->pointsP+lp1)->x,(Tin->pointsP+lp1)->y) ;
             sd2 = bcdtmMath_sideOf((Tin->pointsP+hp1)->x,(Tin->pointsP+hp1)->y,(Tin->pointsP+hp2)->x,(Tin->pointsP+hp2)->y,(Tin->pointsP+lp2)->x,(Tin->pointsP+lp2)->y) ;
             if( sd1 != sd2 && sd1 != 0 && sd2 != 0 )
               {
                sd1 = bcdtmMath_sideOf((Tin->pointsP+lp1)->x,(Tin->pointsP+lp1)->y,(Tin->pointsP+lp2)->x,(Tin->pointsP+lp2)->y,(Tin->pointsP+hp1)->x,(Tin->pointsP+hp1)->y) ;
                sd2 = bcdtmMath_sideOf((Tin->pointsP+lp1)->x,(Tin->pointsP+lp1)->y,(Tin->pointsP+lp2)->x,(Tin->pointsP+lp2)->y,(Tin->pointsP+hp2)->x,(Tin->pointsP+hp2)->y) ;
                if( sd1 != sd2 && sd1 != 0 && sd2 != 0 )
                  {
                   ++err ;
                   if( MessageFlag )
                     {
                      bcdtmWrite_message(0,0,0,"Hp1 = %6ld  Hp2 = %6ld",hp1,hp2) ;
                      bcdtmWrite_message(0,0,0,"Lp1 = %6ld  Lp2 = %6ld",lp1,lp2) ;
                      bcdtmWrite_message(0,0,0,"Intersected Hull Line ** %10.4lf %10.4lf ** %10.4lf %10.4lf",(Tin->pointsP+hp1)->x,(Tin->pointsP+hp1)->y,(Tin->pointsP+hp2)->x,(Tin->pointsP+hp2)->y) ;
                      bcdtmWrite_message(0,0,0,"                      ** %10.4lf %10.4lf ** %10.4lf %10.4lf",(Tin->pointsP+lp1)->x,(Tin->pointsP+lp1)->y,(Tin->pointsP+lp2)->x,(Tin->pointsP+lp2)->y) ;
                      bcdtmWrite_message(0,0,0,"") ;
                     } 
                  }
               }
            }
         }
       lp1 = lp2 ; 
      } while ( lp1 != Tin->hullPnt ) ;
/*
** Set For Next Hull Line
*/
    hp1 = hp2 ; 
   } while ( hp1 != Tin->hullPnt ) ;
/*
** Job Completed
*/
 return(err) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCheck_pointPrecisionTinObject(DTM_TIN_OBJ *Tin,long Point,long MessageFlag)
/*
** This Functions Checks The Precision Of A Tin 
**
** Return Value = 0  No Precision Errors
**              = 1  System Error
**              = 2  Precision Errors
**
*/
{
 long   p1,p2,p3,clc,errcnt=0 ;
/*
**  Initialise
*/
 bcdtmInitialise() ;
 p1 = Point ;
/*
** Scan Entire Tin
*/
 if( ( clc = (Tin->nodesP+p1)->cPtr ) != Tin->nullPtr )
   {
    if( ( p2 = bcdtmList_nextAntTinObject(Tin,p1,(Tin->cListP+clc)->pntNum)) < 0 ) return(1) ;
    while ( clc != Tin->nullPtr )
      {
       p3  = (Tin->cListP+clc)->pntNum ;
       clc = (Tin->cListP+clc)->nextPtr ;
       if( bcdtmMath_allPointSideOfTinObject(Tin,p1,p2,p3) >= 0 )
         {
          if( MessageFlag )
            {
             bcdtmWrite_message(1,0,0,"Precision Error ** Point = %8ld %8ld %8ld",p1,p2,p3) ;
             bcdtmList_writeCircularListForPointTinObject(Tin,p1) ; 
            }
          ++errcnt ;
         }
       p2 = p3 ;
      }
   }
/*
** Print Error Statistics
*/
 if( MessageFlag && errcnt > 0 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Precision Errors = %8ld",errcnt) ;
   }
/*
** Job Completed
*/
 if( ! errcnt ) return(0) ;
 else           return(2) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
| int bcdtmCheck_closePointsTinObject()                                |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCheck_forClosePointsTinObject(DTM_TIN_OBJ *Tin,double Pptol,long MessageFlag,long *NoClosePoints)
/*
** This Function Ckecks For Points In The Tin Closer Than Pptol
** Return Values  = 0 No System Errors
**                = 1 System Errors
** NoClosePoints  = Number Of Points Closer Than Pptol
*/
{
 long   p1,p2,clc ;
 double dp ;
/*
** Initialise
*/
 *NoClosePoints = 0 ;
/*
** Scan Connected Points 
*/
 for( p1 = 0 ; p1 < Tin->numPts ; ++p1 )
   {
    if( ( clc = (Tin->nodesP+p1)->cPtr ) != Tin->nullPtr ) 
      {
       while( clc != Tin->nullPtr )
         {
          p2  = (Tin->cListP+clc)->pntNum ; 
          clc = (Tin->cListP+clc)->nextPtr ;
          if( p2 > p1 )
            {
             if( fabs((Tin->pointsP+p1)->x - (Tin->pointsP+p2)->x) <= Pptol &&
                 fabs((Tin->pointsP+p1)->y - (Tin->pointsP+p2)->y) <= Pptol     )
               {
                if( (dp = bcdtmMath_pointDistanceTinObject(Tin,p1,p2)) <= Pptol )
                  {
                   ++(*NoClosePoints) ;
                   if( MessageFlag )
                     {
                      bcdtmWrite_message(0,0,0,"Connected Close Points %8ld %8ld ** %15.12lf",p1,p2,dp) ;
                     }
                  }
               }
            }
         }   
      }
   }
/*
** Scan Un Connected Points  ( Assumes Data Is Sorted )
*/
 for( p1 = 0 ; p1 < Tin->numPts ; ++p1 )
   {
    if( ( clc = (Tin->nodesP+p1)->cPtr ) != Tin->nullPtr ) 
      {
       p2 = p1 + 1 ;
       while( p2 < Tin->numPts )
         {
          if( (Tin->pointsP+p2)->x - (Tin->pointsP+p1)->x > Pptol )  p2 = Tin->numPts ;
          else
            {
             if( (Tin->nodesP+p2)->cPtr != Tin->nullPtr ) 
               {
                if( ! bcdtmList_testLineTinObject(Tin,p1,p2) )
                  {
                   if( (dp = bcdtmMath_pointDistanceTinObject(Tin,p1,p2)) <= Pptol )
                     {
                      ++(*NoClosePoints) ;
                      if( MessageFlag )
                        {
                         bcdtmWrite_message(0,0,0,"Un Connected Close Points %8ld %8ld ** %15.12lf",p1,p2,dp) ;
                        }
                     }
                  } 
               } 
            } 
          ++p2 ; 
         } 
      }
   } 
/*
** Print Out Statistics
*/
  if( MessageFlag || *NoClosePoints > 0 )
    {
     bcdtmWrite_message(0,0,0,"Number Of Close Points = %6ld",*NoClosePoints) ;
    } 
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
| int bcdtmCheck_forSliverTrianglesTinObject()                         |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCheck_forSliverTrianglesTinObject(DTM_TIN_OBJ *Tin,double Pltol,long MessageFlag,long *NoSliverTriangles)
/*
** This Function Ckecks For Sliver In The Tin Closer Than Pltol
**
** Return Values  = 0 No System Errors
**                = 1 System Errors
** NoSliverTriangles  = Number Of Sliver Triangles Closer Than Pltol
*/
{
 long   p1,p2,p3,clc,lineflag ;
 double dn,Xi,Yi ;
/*
** Initialise
*/
 *NoSliverTriangles = 0 ;
/*
** Scan Circular List 
*/
 for ( p1 = 0 ; p1 < Tin->numPts ; ++p1 )
   {
    if( ( clc = (Tin->nodesP+p1)->cPtr ) != Tin->nullPtr )
      {
       if( ( p2 = bcdtmList_nextAntTinObject(Tin,p1,(Tin->cListP+clc)->pntNum)) < 0 ) return(1) ;
       while ( clc != Tin->nullPtr )
         {
          p3  = (Tin->cListP+clc)->pntNum ;
          clc = (Tin->cListP+clc)->nextPtr ;
          if( /* p2 > p1 && p3 > p1 && */ (Tin->nodesP+p1)->hPtr != p2 )
            {
             dn = bcdtmMath_distanceOfPointFromLine(&lineflag,(Tin->pointsP+p2)->x,(Tin->pointsP+p2)->y,(Tin->pointsP+p3)->x,(Tin->pointsP+p3)->y,(Tin->pointsP+p1)->x,(Tin->pointsP+p1)->y,&Xi,&Yi) ;
             if( dn < Pltol && lineflag )
               {
                ++(*NoSliverTriangles) ;
                if( MessageFlag )
                  {
                   if( (Tin->nodesP+p3)->hPtr == p2 /*  || (Tin->nodesP+p2)->hPtr == p1 || (Tin->nodesP+p1)->hPtr == p3 */ )
                     {  
                      bcdtmWrite_message(0,0,0,"External Sliver Triangle %8ld %8ld %8ld ** %15.12lf",p1,p2,p3,dn) ;
                     }
                   else
                     {
                      bcdtmWrite_message(0,0,0,"Internal Sliver Triangle %8ld %8ld %8ld ** %15.12lf",p1,p2,p3,dn) ;
                     } 
                  }
               }
            }
          p2 = p3 ;
         }   
      }
   }
/*
** Print Out Statistics
*/
  if( MessageFlag || *NoSliverTriangles > 0 )
    {
     bcdtmWrite_message(0,0,0,"Number Of Sliver Triangles = %6ld",*NoSliverTriangles) ;
     
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
BENTLEYDTM_Public int bcdtmCheck_topologyDtmFeaturesTinObject(DTM_TIN_OBJ *tinP,long messageFlag)
/*
** This Function Checks The Connectivity Of DTM features in the DTMFeatureState::Tin
**
** Return Values  = 0 No System Errors
**                = 1 System Errors
**                = 2 Topology Errors
*/
{
 long  err,feature,numError=0 ;
 long  numHrdBrk=0,numSftBrk=0,numConLin=0,numVoid=0;
 long  numIsland=0,numHole=0,numSpot=0 ;
 DTM_FEATURE_TABLE *fP ; 
/*
**  Print Out Statistics 
*/
 if( messageFlag )
   {
    bcdtmWrite_message(0,0,0,"Size Of Feature Table   = %6ld",tinP->numFeatureTable) ;
    bcdtmWrite_message(0,0,0,"Size Of Feature List    = %6ld",tinP->numFeatureList) ;
    for( fP = tinP->fTableP  ; fP < tinP->fTableP + tinP->numFeatureTable ; ++fP )
      {
       if( fP->firstPnt != tinP->nullPnt )
         {
          if( fP->dtmFeatureType == DTMFeatureType::Breakline   ) ++numHrdBrk ;
          if( fP->dtmFeatureType == DTMFeatureType::SoftBreakline   ) ++numSftBrk ;
          if( fP->dtmFeatureType == DTMFeatureType::ContourLine ) ++numConLin ;
          if( fP->dtmFeatureType == DTMFeatureType::Void         ) ++numVoid   ;
          if( fP->dtmFeatureType == DTMFeatureType::Island       ) ++numIsland ;
          if( fP->dtmFeatureType == DTMFeatureType::Hole         ) ++numHole   ;
          if( fP->dtmFeatureType == DTMFeatureType::GroupSpots   ) ++numSpot   ;
         }
      }  
    bcdtmWrite_message(0,0,0,"Number Of Hard Breaks   = %6ld",numHrdBrk) ;
    bcdtmWrite_message(0,0,0,"Number Of Soft Breaks   = %6ld",numSftBrk) ;
    bcdtmWrite_message(0,0,0,"Number Of Contour Lines = %6ld",numConLin) ;
    bcdtmWrite_message(0,0,0,"Number Of Voids         = %6ld",numVoid) ;
    bcdtmWrite_message(0,0,0,"Number Of Islands       = %6ld",numIsland) ;
    bcdtmWrite_message(0,0,0,"Number Of Spot Features = %6ld",numSpot) ;
   }
/*
**  Check Connectivity Of DTM Features
*/
 for( feature = 0  ; feature < tinP->numFeatureTable ; ++feature )
   {
    if(( err = bcdtmList_checkConnectivityOfDtmFeatureTinObject(tinP,feature,1)) == 1 ) return(1) ;
    if( err == 2 ) ++numError ;
   }
/*
** Write Message
*/
 if( messageFlag && numError )
   {
    bcdtmWrite_message(0,0,0,"Number Of Feature Topology Errors = %6ld",numError) ;
   }
/*
** Job Completed
*/
 if( numError ) return(2) ;
 else           return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCheck_sortOrderDataObject(DTM_DAT_OBJ *Data )
/*
** This Function Checks The Sort Order Of A Data Object
*/
{
 DTM_DATA_POINT *pd ;
/*
** Check For Valid Data Object
*/
 if( bcdtmObject_testForValidDataObject(Data)) return(1) ;
/*
** Scan Points And Check Sort Order
*/
 for( pd = Data->pointsP ; pd < Data->pointsP + Data->numPts - 1 ; ++pd )
   {
    if( pd->x > (pd+1)->x || ( pd->x == (pd+1)->x && pd->y > (pd+1)->y )) 
      { 
       bcdtmWrite_message(0,0,0,"Sort Order Data Object Invalid") ;
       bcdtmWrite_message(0,0,0,"Point = %6ld  %15.8lf %15.8lf",(long)(pd - Data->pointsP),pd->x,pd->y) ; 
       bcdtmWrite_message(0,0,0,"Point = %6ld  %15.8lf %15.8lf",(long)(pd - Data->pointsP+1),(pd+1)->x,(pd+1)->y) ; 
       return(1) ;
      }
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
BENTLEYDTM_Public int bcdtmCheck_sortOrderTinObject(DTM_TIN_OBJ *Tin )
/*
** This Function Checks The Sort Order Of A Tin Object
*/
{
 DPoint3d *pd ;
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidTinObject(Tin)) return(1) ;
/*
** Scan Points And Check Sort Order
*/
 for( pd = Tin->pointsP ; pd < Tin->pointsP + Tin->numSortedPts - 1 ; ++pd )
   {
    if( pd->x > (pd+1)->x || ( pd->x == (pd+1)->x && pd->y > (pd+1)->y )) 
      { 
       bcdtmWrite_message(0,0,0,"Sort Order Tin Object Invalid") ;
       bcdtmWrite_message(0,0,0,"Point = %6ld  %15.8lf %15.8lf",(long)(pd - Tin->pointsP),pd->x,pd->y) ; 
       bcdtmWrite_message(0,0,0,"Point = %6ld  %15.8lf %15.8lf",(long)(pd - Tin->pointsP+1),(pd+1)->x,(pd+1)->y) ; 
       return(1) ;
      }
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
BENTLEYDTM_Public int bcdtmCheck_forKnotInDtmFeatureTinObject(DTM_TIN_OBJ *Tin,long Feature )
/*
** This Function Checks The Sort Order Of A Tin Object
*/
{
 long  sp,np ;
 DTM_TIN_NODE *pd ;
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidTinObject(Tin)) goto errexit ;
/*
** Check For Correct Feature Value
*/
 if( Feature < 0 || Feature >= Tin->numFeatureTable ) goto errexit ;
/*
** Null Out Sptr Values
*/
 for( pd = Tin->nodesP ; pd < Tin->nodesP + Tin->numPts ; ++pd ) pd->sPtr = Tin->nullPnt ;
/*
** Scan Feature Points And Check For Knot In Feature
*/
 sp = (Tin->fTableP+Feature)->firstPnt ;
 do
   {
    if( bcdtmList_getNextPointForDtmFeatureTinObject(Tin,Feature,sp,&np) ) goto errexit ;
    if( np != Tin->nullPnt && np != (Tin->fTableP+Feature)->firstPnt  )
      {
       if( (Tin->nodesP+np)->sPtr != Tin->nullPnt )
         { 
          bcdtmWrite_message(1,0,0,"Knot Detected In Feature %6ld At Point %6ld",Feature,np) ;
          goto errexit ;
         }
      } 
    (Tin->nodesP+sp)->sPtr = np ;
    sp = np ; 
   } while ( sp != Tin->nullPnt && sp != (Tin->fTableP+Feature)->firstPnt )  ;
/*
** Return
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCheck_dtmFeatureEndPointsTinObject(DTM_TIN_OBJ *tinP,long reportErrors)
/*
** This Function Checks The Point Dtm Feature List To Ensure
** That Points Marked As Dtm Feature Points Are The Correct End Point
** For The Dtm Feature
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  pnt,firstPnt,nextPnt=0,lastPnt,listPnt,listPtr,listPtr1,dtmFeature ;
/*
** Write Entry Message
*/
 if( reportErrors) dbg=DTM_TRACE_VALUE(0) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points") ;
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidTinObject(tinP)) goto errexit ;
/*
** Scan Tin Points For Feature End Points
*/
 for( pnt = 0 ; pnt < tinP->numPts ; ++pnt )
   {
    if( (tinP->nodesP+pnt)->cPtr != tinP->nullPtr )
      {
       listPtr = (tinP->nodesP+pnt)->fPtr ;
       while ( listPtr != tinP->nullPtr )
         {
          listPnt    = (tinP->fListP+listPtr)->nextPnt ;
          dtmFeature = (tinP->fListP+listPtr)->dtmFeature ; 
          listPtr    = (tinP->fListP+listPtr)->nextPtr ;
/*
**  Check For Feature End Point
*/
          if( listPnt == tinP->nullPnt )
            {
/*
** Scan To Last Point Of Feature
*/
             lastPnt  =  tinP->nullPnt ;
             firstPnt =  (tinP->fTableP+dtmFeature)->firstPnt ;
             listPtr1 =  (tinP->nodesP+firstPnt)->fPtr ;
             while ( listPtr1 != tinP->nullPtr )
               {
                while( listPtr1 != tinP->nullPtr && (tinP->fListP+listPtr1)->dtmFeature != dtmFeature ) listPtr1 = (tinP->fListP+listPtr1)->nextPtr ;
                if( listPtr1 != tinP->nullPtr )
                  {
                   nextPnt = (tinP->fListP+listPtr1)->nextPnt ;
                   if( nextPnt != tinP->nullPnt )
                     {
                      lastPnt  = nextPnt ;
                      listPtr1 = (tinP->nodesP+nextPnt)->fPtr ;
                     }
                  }
                if ( nextPnt == tinP->nullPnt || nextPnt == firstPnt ) listPtr1 = tinP->nullPtr ;
               }
/*
** Check For Closed Dtm Feature
*/
             if(  nextPnt == firstPnt ) lastPnt = tinP->nullPnt ;
/*
** Check Last Point Corresponds To End Point Of Feature
*/
             if( lastPnt != pnt )
               {
                if( ret == DTM_SUCCESS )
                  {
                   bcdtmWrite_message(1,0,0,"Dtm Feature End Point Error") ;
                   ret = DTM_ERROR ;
                  }
                if( reportErrors )
                  {
                   bcdtmWrite_message(0,0,0,"Point %6ld Is Not A End Point Of Dtm Feature %6ld",pnt,dtmFeature) ;
                  }
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
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points Error") ;
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
BENTLEYDTM_Public int bcdtmCheck_dtmFeatureListTinObject(DTM_TIN_OBJ *tinP,long reportErrors)
/*
** This Function Checks The Dtm Feature Lists To Ensure :-
**
** 1. Points With A Feature List Exist In The Feature Point List
** 2. The Feature Point List Has A Corresponding Point Feature List Entry
**
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  pnt,firstPnt,nextPnt=0,listPtr,listPtr1,dtmFeature,pntFound ;
 long  numFeatures=0,numFeaturePts=0,numPtsWithFeatures=0,numPntFeatures=0,numKnots=0,numNextPnt=0 ;
 DTM_FEATURE_TABLE *featP ; 
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature Lists") ;
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidTinObject(tinP)) goto errexit ;
/*
** Scan Tin Points For Features And Check Tin Point Is On Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Points Are On Associated Feature") ;
 for( pnt = 0 ; pnt < tinP->numPts ; ++pnt )
   {
/*
**  Scan Feature List For Point
*/ 
    if( (tinP->nodesP+pnt)->fPtr != tinP->nullPtr )
      {
       ++numPtsWithFeatures ;
       listPtr = (tinP->nodesP+pnt)->fPtr ;
       while ( listPtr != tinP->nullPtr )
         {
          nextPnt    = (tinP->fListP+listPtr)->nextPnt ;
          dtmFeature = (tinP->fListP+listPtr)->dtmFeature ; 
          listPtr    = (tinP->fListP+listPtr)->nextPtr ;
          ++numPntFeatures ;
/*
**        Scan To Feature Point
*/
          pntFound = FALSE ;
          firstPnt = (tinP->fTableP+dtmFeature)->firstPnt ;
          listPtr1 = (tinP->nodesP+firstPnt)->fPtr ;
          if( firstPnt == pnt ) pntFound = TRUE ;
          while ( listPtr1 != tinP->nullPtr && pntFound == FALSE )
            {
             while( listPtr1 != tinP->nullPtr && (tinP->fListP+listPtr1)->dtmFeature != dtmFeature ) listPtr1 = (tinP->fListP+listPtr1)->nextPtr ;
             if( listPtr1 != tinP->nullPtr )
               {
                nextPnt  = (tinP->fListP+listPtr1)->nextPnt ;
                listPtr1 = (tinP->nodesP+nextPnt)->fPtr ;
                if( nextPnt == pnt ) pntFound = TRUE ;
                if ( nextPnt == tinP->nullPnt || nextPnt == firstPnt ) listPtr1 = tinP->nullPtr ;
               }
            } 
         
/*
**        Check Point Found
*/
          if( pntFound == FALSE )
            {
             if( reportErrors ) bcdtmWrite_message(2,0,0,"Point %6ld Is Not In Dtm Feature %6ld",pnt,dtmFeature) ;
             ret = DTM_ERROR ;
            } 
         }
      }
   }
/*
** Scan Feature Points And Check Tin Point Has A Corresponding Feature Entry
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm Features Points Have An Associated Point Feature List") ;
 for( featP = tinP->fTableP  ; featP < tinP->fTableP + tinP->numFeatureTable ; ++featP )
   {
    if( ( pnt = featP->firstPnt ) != tinP->nullPnt )
      {
       ++numFeatures ; 
       dtmFeature = (long)(featP-tinP->fTableP) ;
       do
         {
          pntFound = FALSE ;
          nextPnt  = tinP->nullPnt ;
          listPtr1 = (tinP->nodesP+pnt)->fPtr ;
          while( listPtr1 != tinP->nullPtr && (tinP->fListP+listPtr1)->dtmFeature != dtmFeature ) listPtr1 = (tinP->fListP+listPtr1)->nextPtr ;
          if( listPtr1 != tinP->nullPtr )
            {
             pntFound = TRUE ;
             nextPnt  = (tinP->fListP+listPtr1)->nextPnt ;
             ++numFeaturePts ;
            }
/*
**        Check Point Has Feature List For Feature
*/
          if( pntFound == FALSE )
            {
             if( reportErrors ) bcdtmWrite_message(2,0,0,"Dtm Feature %6ld Has No Feature List Entry For Point %6ld",dtmFeature,pnt) ;
             ret = DTM_ERROR ;
            }
/*
**        Set Next Point
*/
          pnt = nextPnt ;
         } while ( pnt != tinP->nullPnt && pnt != featP->firstPnt ) ;  
      }
   }
/*
** Check For Knots In DTM Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Knots In DTM Features") ;
 for( featP = tinP->fTableP  ; featP < tinP->fTableP + tinP->numFeatureTable ; ++featP )
   {
    if( featP->firstPnt  != tinP->nullPnt )
      {
       dtmFeature = (long)(featP-tinP->fTableP) ;
       if( bcdtmCheck_forKnotInDtmFeatureTinObject(tinP,dtmFeature) ) 
         {
          ++numKnots ;
          if( reportErrors ) bcdtmWrite_message(2,0,0,"Knot Detected In Dtm Feature = %6ld",dtmFeature) ;
          ret = DTM_ERROR ;
         }
      }
   }
/*
** Check For Next Point Same As Current Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Next Point Errors") ;
 for( pnt = 0 ; pnt < tinP->numPts ; ++pnt )
   {
    if( (tinP->nodesP+pnt)->fPtr != tinP->nullPtr )
      {
       ++numPtsWithFeatures ;
       listPtr = (tinP->nodesP+pnt)->fPtr ;
       while ( listPtr != tinP->nullPtr )
         {
          nextPnt    = (tinP->fListP+listPtr)->nextPnt ;
          dtmFeature = (tinP->fListP+listPtr)->dtmFeature ; 
          listPtr    = (tinP->fListP+listPtr)->nextPtr ;
          if( nextPnt == pnt ) 
            {
             if( reportErrors ) bcdtmWrite_message(2,0,0,"pnt = %9ld ** nextPnt = %9ld dtmFeature = %6ld",pnt,nextPnt,dtmFeature) ;
             ret = DTM_ERROR ;
             ++numNextPnt ; 
            }
         }
      }
   }
/*
** Write Statistics
*/
 if( reportErrors )
   {
    bcdtmWrite_message(0,0,0,"Number Of Dtm Features           = %9ld",numFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Of Dtm Features Points    = %9ld",numFeaturePts) ;
    bcdtmWrite_message(0,0,0,"Number Of Points With Features   = %9ld",numPtsWithFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Of Point Feature Lists    = %9ld",numPntFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Of Feature Knots Detected = %9ld",numKnots) ;
    bcdtmWrite_message(0,0,0,"Number Of Next Point Errors      = %9ld",numNextPnt) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature Lists Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature Lists Error") ;
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
BENTLEYDTM_Public long bcdtmList_nextClockTinObject(DTM_TIN_OBJ *Tin,long p1,long p2)
{
 long clp,clh ;
/*
** Test Point Values Are In Correct Range
*/
 if( p1 >= Tin->numPts || p2 >= Tin->numPts || p1 < 0 || p2 < 0 )
   {
    bcdtmWrite_message(2,0,0,"Tin Point Range Error") ; 
    return(-99) ; 
   }
/*
** Scan Circular List
*/
 clp = clh = (Tin->nodesP+p1)->cPtr ;
 while( clp != DTM_NULL_PTR )
   {
    if( (Tin->cListP+clp)->pntNum == p2 )
      {
       clp = (Tin->cListP+clp)->nextPtr ;
       if( clp == DTM_NULL_PTR ) return ( (Tin->cListP+clh)->pntNum ) ;
                  else     return ( (Tin->cListP+clp)->pntNum ) ;
      }
    clp = (Tin->cListP+clp)->nextPtr ;
   }
 bcdtmWrite_message(2,0,0,"Circular List Error Clockwise ** %8ld %8ld",p1,p2) ;
 return(-99) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT long bcdtmList_nextAntTinObject(DTM_TIN_OBJ *Tin,long p1,long p2)
{
 long clp,cll ;
/*
** Test Point Values Are In Correct Range
*/
 if( p1 >= Tin->numPts || p2 >= Tin->numPts || p1 < 0 || p2 < 0 )
   { 
    bcdtmWrite_message(2,0,0,"Tin Point Range Error") ; 
    return(-99) ; 
   }
/*
** Scan Circular List
*/
 cll = DTM_NULL_PTR ; clp = (Tin->nodesP+p1)->cPtr ;
 while ( clp != DTM_NULL_PTR )
   {
    if( (Tin->cListP+clp)->pntNum == p2 )
      {
       if( cll != DTM_NULL_PTR ) return( (Tin->cListP+cll)->pntNum ) ;
       while( (Tin->cListP+clp)->nextPtr != DTM_NULL_PTR ) clp = (Tin->cListP+clp)->nextPtr  ;
       return( (Tin->cListP+clp)->pntNum ) ;
      }
    cll = clp ;
    clp = (Tin->cListP+clp)->nextPtr ;
   }
 bcdtmWrite_message(2,0,0,"Circular List Error Counter Clockwise ** %8ld %8ld",p1,p2) ;
 return(-99) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_writeCircularListForPointTinObject(DTM_TIN_OBJ *Tin,long P)
/*
** This Function Writes The Circular List For A Point
*/
{
 long   pt,cl ;
 double dx,dy,angle   ;
/*
** Header
*/
 bcdtmWrite_message(0,0,0,"**** Circular List For Point %6ld ****",P) ;
 if( P >= 0 && P < Tin->numPts )
   {
    bcdtmWrite_message(0,0,0,"x = %15.8lf y = %15.8lf z = %10.4lf", (Tin->pointsP+P)->x,(Tin->pointsP+P)->y,(Tin->pointsP+P)->z) ;
    bcdtmWrite_message(0,0,0,"cPtr = %9ld hPtr = %9ld tPtr = %9ld sPtr = %9ld hPtr = %9ld",(Tin->nodesP+P)->cPtr,(Tin->nodesP+P)->hPtr,(Tin->nodesP+P)->tPtr,(Tin->nodesP+P)->sPtr,(Tin->nodesP+P)->hPtr ) ;
   }
 else
   {
    bcdtmWrite_message(0,0,0,"No Circular List For Point %6ld ** numPts = %8ld",P,Tin->numPts) ;
    return(0) ;
   }  
/*
** List Points
*/
 cl = (Tin->nodesP+P)->cPtr ;
 while ( cl != Tin->nullPtr )
   {
    pt = (Tin->cListP+cl)->pntNum ;
    cl = (Tin->cListP+cl)->nextPtr ;
    dx = (Tin->pointsP+pt)->x - (Tin->pointsP+P)->x ;
    dy = (Tin->pointsP+pt)->y - (Tin->pointsP+P)->y ;
    if( dx == 0.0 && dy == 0.0 ) angle = 99.99 ;
    else  
      { 
       angle = atan2(dy,dx) ;
       if( angle < 0.0 ) angle += DTM_2PYE ;
      }
    bcdtmWrite_message(0,0,0,"%8ld ** %15.8lf %15.8lf %10.4lf ** %15.12lf",pt,(Tin->pointsP+pt)->x,(Tin->pointsP+pt)->y,(Tin->pointsP+pt)->z,angle) ;
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
BENTLEYDTM_Public int bcdtmMath_allPointSideOfTinObject(DTM_TIN_OBJ *Tin,long p1,long p2,long p3)
{
 int    ret=0 ;
 double sd1,sd2,sd3,x1,y1,x2,y2,x3,y3 ;
 x1  = (Tin->pointsP+p1)->x ; y1 = (Tin->pointsP+p1)->y ;
 x2  = (Tin->pointsP+p2)->x ; y2 = (Tin->pointsP+p2)->y ;
 x3  = (Tin->pointsP+p3)->x ; y3 = (Tin->pointsP+p3)->y ;
 sd1 = ((x1-x3) * (y2-y3)) - ((y1-y3) * (x2-x3))  ;
 sd2 = ((x2-x1) * (y3-y1)) - ((y2-y1) * (x3-x1))  ;
 sd3 = ((x3-x2) * (y1-y2)) - ((y3-y2) * (x1-x2))  ;
 if( ( sd1 < 0.0 && sd2 >= 0.0 ) || ( sd1 > 0.0 && sd2 <= 0.0) ) return(0) ;
 if( ( sd1 < 0.0 && sd3 >= 0.0 ) || ( sd1 > 0.0 && sd3 <= 0.0) ) return(0) ;
 if( sd1 <  0.0 ) ret = -1 ; /* Right of Line */
 if( sd1 == 0.0 ) ret =  0 ; /* On Line       */
 if( sd1 >  0.0 ) ret =  1 ; /* Left of Line  */
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_testLineTinObject(DTM_TIN_OBJ *Tin,long p1,long p2)
{
 long clc ;
 clc = (Tin->nodesP+p1)->cPtr ;
 while( clc != Tin->nullPtr )
   {
    if( p2 == (Tin->cListP+clc)->pntNum ) return(1) ;
    clc = (Tin->cListP+clc)->nextPtr ;
   }
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmMath_pointDistanceTinObject(DTM_TIN_OBJ *Tin,long p1,long p2)
{
 double x,y ;
 x = (Tin->pointsP+p2)->x - (Tin->pointsP+p1)->x ;
 y = (Tin->pointsP+p2)->y - (Tin->pointsP+p1)->y ;
 return(sqrt(x*x+y*y)) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_checkConnectivityOfDtmFeatureTinObject(DTM_TIN_OBJ *Tin,long dtmFeature,long MessageFlag)
/*
**
** This Function Checks The Connectivity  Of a Dtm Feature
** Return Values  == 0  No Errors
**                == 1  System Error
**                == 2  Closure Error
**   
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long npnt, spnt;
 DTMFeatureType dtmFeatureType;
 char dtmFeatureTypeName[50] ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of DTM Feature %6ld Of %6ld",dtmFeature,Tin->numFeatureTable) ;
/*
** Check For Valid Feature
*/
 if( (Tin->fTableP+dtmFeature)->firstPnt != Tin->nullPnt && (Tin->fTableP+dtmFeature)->dtmFeatureType != DTMFeatureType::RandomSpots ) 
   {
    dtmFeatureType = (Tin->fTableP+dtmFeature)->dtmFeatureType ;
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Feature Type = %4ld ** %s",dtmFeatureType,dtmFeatureTypeName) ;
/*
** Scan Feature And Copy Tptr Values To Sptr And Null Out Tptr Values
*/
   if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Feature And Copying Tptr Values To Sptr") ;
   spnt = (Tin->fTableP+dtmFeature)->firstPnt ;
   do
     {
      (Tin->nodesP+spnt)->sPtr = (Tin->nodesP+spnt)->tPtr ;
      (Tin->nodesP+spnt)->tPtr = Tin->nullPnt ;
      if( bcdtmList_getNextPointForDtmFeatureTinObject(Tin,dtmFeature,spnt,&npnt)) goto errexit ;
      spnt = npnt ;
     } while ( spnt != (Tin->fTableP+dtmFeature)->firstPnt && spnt != DTM_NULL_PNT ) ;
/*
** Copy Dtm Feature To Tptr List
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature To Tptr List") ;
    if( bcdtmList_copyDtmFeatureToTptrListTinObject(Tin,dtmFeature,&spnt)) goto errexit ;
/*
** Check Connectivity Of Tptr List
*/
    if( (Tin->fTableP+dtmFeature)->dtmFeatureType != DTMFeatureType::GroupSpots )
      { 
       if( bcdtmList_checkConnectivityTptrListTinObject(Tin,spnt,1)) 
         { 
          if( MessageFlag ) bcdtmWrite_message(2,0,0,"Connectivity Error In Dtm Feature %6ld Type = %4ld ** %20s",dtmFeature,dtmFeatureType,dtmFeatureTypeName) ;
          ret = 2 ; 
          goto errexit ; 
         }
      }
/*
** Check Polygonal Features Close
*/
    if( dtmFeatureType == DTMFeatureType::Void ||  dtmFeatureType == DTMFeatureType::Void ||  dtmFeatureType == DTMFeatureType::Hole ) 
      {
       npnt = spnt ;
       do
         {
          npnt = (Tin->nodesP+npnt)->tPtr ;
         } while ( npnt != Tin->nullPnt && npnt != spnt ) ; 
       if( npnt == Tin->nullPnt )
         {
          if( MessageFlag ) bcdtmWrite_message(0,0,0,"Closure Error In Feature %6ld Type = %20s",dtmFeature,dtmFeatureTypeName) ;
          ret = 2 ;
          goto errexit ; 
         }
      }
/*
**  Null Out Tptr List
*/
    if( bcdtmList_nullOutTptrListTinObject(Tin,spnt)) goto errexit ;
/*
** Scan Feature And Copy Sptr Values To Tptr
*/
   spnt = (Tin->fTableP+dtmFeature)->firstPnt ;
   do
     {
      (Tin->nodesP+spnt)->tPtr = (Tin->nodesP+spnt)->sPtr ;
      if( bcdtmList_getNextPointForDtmFeatureTinObject(Tin,dtmFeature,spnt,&npnt)) goto errexit ;
      spnt = npnt ;
     } while ( spnt != (Tin->fTableP+dtmFeature)->firstPnt && spnt != DTM_NULL_PNT ) ;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of DTM Feature %6ld Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of DTM Feature %6ld Error",dtmFeature) ;
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
BENTLEYDTM_Public int bcdtmList_getNextPointForDtmFeatureTinObject(DTM_TIN_OBJ *tinP,long dtmFeature,long currentPnt ,long *nextPnt)
/*
** This Function Gets The Next Point For A Dtm Feature
*/
{
 long listPtr ;
/*
** Initialise
*/
 *nextPnt = tinP->nullPnt ;
/*
** Check Dtm Feature Exists
*/
 if( dtmFeature >= 0 && dtmFeature < tinP->numFeatureTable ) 
   {
    if( (tinP->fTableP+dtmFeature)->firstPnt != tinP->nullPnt ) 
      { 
/*
**     Scan Dtm Features For Current Point
*/
       listPtr = (tinP->nodesP+currentPnt)->fPtr ; 
       while ( listPtr != tinP->nullPtr && *nextPnt == tinP->nullPnt )
         {
          if( (tinP->fListP+listPtr)->dtmFeature == dtmFeature ) *nextPnt = (tinP->fListP+listPtr)->nextPnt ;
          listPtr  = (tinP->fListP+listPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmList_nullOutTptrListTinObject(DTM_TIN_OBJ *Tin,long Spnt)
/*
** This Function Nulls Out The Tptr List
*/
{
 long sp,np ;
/*
** Check A Tptr List Exists
*/
 if( (Tin->nodesP+Spnt)->tPtr == Tin->nullPnt ) return(0) ;
/*
** Scan Tptr List And Null Out
*/
 sp = Spnt ;
 do
   { 
    np = (Tin->nodesP+sp)->tPtr ;
    (Tin->nodesP+sp)->tPtr = Tin->nullPnt ;
    sp = np ;
   } while ( sp != Spnt && sp != Tin->nullPnt ) ;
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
BENTLEYDTM_Public int bcdtmList_checkConnectivityTptrListTinObject(DTM_TIN_OBJ *tinP,long startPnt,long messageFlag) 
/*
** This Function Checks The Connectivity Of A Tptr List
**
** Return Values == DTM_SUCCESS  No Connectivity Errors
**               == DTM_ERROR    Connectivity Errors  
*/
{
 int  ret=DTM_SUCCESS ;
 long sp,tp,lp=0,knot ;
/*
** Initialise
*/
 if( startPnt < 0 || startPnt >= tinP->numPts ) ret = DTM_ERROR ;  
 else
   {
    if( (tinP->nodesP+startPnt)->tPtr < 0 || (tinP->nodesP+startPnt)->tPtr >= tinP->numPts  ) ret = DTM_ERROR ;
    else
      {
/*
** Check List Connectivity
*/
       sp = startPnt ;
       while ( (tinP->nodesP+sp)->tPtr != tinP->nullPnt && (tinP->nodesP+sp)->tPtr >= 0 )
         {
          if( ! bcdtmList_testLineTinObject(tinP,sp,(tinP->nodesP+sp)->tPtr) )
            {
             if( messageFlag ) bcdtmWrite_message(0,0,0,"Unconnected Points %8ld %8ld In Tptr Polygon",sp,(tinP->nodesP+sp)->tPtr) ;
             ret = DTM_ERROR ;
            } 
          tp = (tinP->nodesP+sp)->tPtr ; 
          (tinP->nodesP+sp)->tPtr = -((tinP->nodesP+sp)->tPtr + 1) ;
          sp = tp ; 
         }
/*
** Set Value Of Last Point In List
*/
       knot = 0 ;
       if( (tinP->nodesP+sp)->tPtr < 0 ) { lp = sp ; knot = 1 ; }
/*
** Reset Tptr Values Positive
*/
       sp = startPnt ;
       while( (tinP->nodesP+sp)->tPtr < 0  ) 
         {
          tp = -((tinP->nodesP+sp)->tPtr + 1 ) ;
          (tinP->nodesP+sp)->tPtr = tp ;
          sp = tp ;
         }
/*
** Test For Knot In List
*/  
      if( knot && lp != startPnt )
        { 
         if( messageFlag ) bcdtmWrite_message(0,0,0,"Knot At Point %8ld ",lp) ;
         ret = DTM_ERROR ;
        } 
     }
  }
/*
** Write Error Message If Necessary
*/
 if( ret == DTM_ERROR ) bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr List") ;
/*
** Return
*/
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_copyDtmFeatureToTptrListTinObject(DTM_TIN_OBJ *tinP,long dtmFeature,long *startPnt)
/*
** This Function Copies A Dtm Feature To A Tptr List
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long listPtr,nextPnt,firstPnt ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying DTM Feature %6ld To Tptr List",dtmFeature) ;
/*
** Initialise
*/
 *startPnt = tinP->nullPnt ;
/*
** Check For Valid Dtm Feature
*/
 if( dtmFeature < 0 || dtmFeature >= tinP->numFeatureTable )
   {
    bcdtmWrite_message(1,0,0,"Dtm Feature Range Error") ;
    goto errexit ;
   }
/*
** Scan Dtm Feature List And Copy To Tptr List
*/
 nextPnt = firstPnt = (tinP->fTableP+dtmFeature)->firstPnt ;
 listPtr = (tinP->nodesP+firstPnt)->fPtr ;
 while ( listPtr != tinP->nullPtr )
   {
    while ( listPtr != tinP->nullPtr  && (tinP->fListP+listPtr)->dtmFeature != dtmFeature ) listPtr = (tinP->fListP+listPtr)->nextPtr ;
    if( listPtr != tinP->nullPtr )
      {
       (tinP->nodesP+nextPnt)->tPtr = (tinP->fListP+listPtr)->nextPnt ;
       nextPnt = (tinP->fListP+listPtr)->nextPnt ;
       if( nextPnt != tinP->nullPnt ) listPtr = (tinP->nodesP+nextPnt)->fPtr ;
       if( nextPnt == tinP->nullPnt || nextPnt == firstPnt ) listPtr = tinP->nullPtr ; 
      } 
   }
/*
** Set Start Point
*/
 *startPnt = firstPnt ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM Feature %6ld To Tptr List Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM Feature %6ld To Tptr List Error",dtmFeature) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret != DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_getFeatureCodesForDtmFeatureType(DTMFeatureType dtmFeatureType,long *startCodeP,long *nextCodeP) 
/*
** This Function Returns The Start And Next Codes For A DTM Feature Type
*/
{
 int  ret=DTM_ERROR ;
/*
** Initialise
*/
 *startCodeP = *nextCodeP = -1 ;
/*
** Assign Feature Start And Next Codes
*/
 if( dtmFeatureType == DTMFeatureType::RandomSpots   ) { *startCodeP =  1 ; *nextCodeP =  1 ; }
 if( dtmFeatureType == DTMFeatureType::GroupSpots    ) { *startCodeP = 22 ; *nextCodeP = 23 ; }
 if( dtmFeatureType == DTMFeatureType::Breakline    ) { *startCodeP =  2 ; *nextCodeP =  3 ; }
 if( dtmFeatureType == DTMFeatureType::SoftBreakline    ) { *startCodeP = 50 ; *nextCodeP = 51 ; }
 if( dtmFeatureType == DTMFeatureType::Hull          ) { *startCodeP =  4 ; *nextCodeP =  4 ; }
 if( dtmFeatureType == DTMFeatureType::DrapeHull    ) { *startCodeP = 52 ; *nextCodeP = 53 ; }
 if( dtmFeatureType == DTMFeatureType::ContourLine  ) { *startCodeP =  5 ; *nextCodeP =  6 ; } 
 if( dtmFeatureType == DTMFeatureType::Void          ) { *startCodeP =  7 ; *nextCodeP =  8 ; }
 if( dtmFeatureType == DTMFeatureType::Island        ) { *startCodeP =  9 ; *nextCodeP = 10 ; }
 if( dtmFeatureType == DTMFeatureType::Hole          ) { *startCodeP = 11 ; *nextCodeP = 12 ; }      
 if( dtmFeatureType == DTMFeatureType::GraphicBreak ) { *startCodeP = 13 ; *nextCodeP = 14 ; }      
 if( dtmFeatureType == DTMFeatureType::DrapeVoid    ) { *startCodeP = 15 ; *nextCodeP = 16 ; }
 if( dtmFeatureType == DTMFeatureType::BreakVoid    ) { *startCodeP = 17 ; *nextCodeP = 18 ; }
 if( dtmFeatureType == DTMFeatureType::HullLine     ) { *startCodeP = 20 ; *nextCodeP = 21 ; }
 if( dtmFeatureType == DTMFeatureType::VoidLine     ) { *startCodeP = 30 ; *nextCodeP = 31 ; }
 if( dtmFeatureType == DTMFeatureType::HoleLine     ) { *startCodeP = 32 ; *nextCodeP = 33 ; }
 if( dtmFeatureType == DTMFeatureType::SlopeToe     ) { *startCodeP = 40 ; *nextCodeP = 41 ; }
 if( dtmFeatureType == DTMFeatureType::Polygon       ) { *startCodeP = 42 ; *nextCodeP = 43 ; }
/*
** Set Return Value
*/
 if( *startCodeP != -1 ) ret = DTM_SUCCESS ;
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
BENTLEYDTM_Public int bcdtmObject_deSortDataObject(DTM_DAT_OBJ *dataP)
/*
** This Function Unsorts A Data Object
*/
{
 int ret=DTM_SUCCESS ;
 long ofs ;
 DTM_DATA_POINT *tmpolyPtsPP=nullptr,*ptsP ;
 DTM_FEATURE_CODE *fcP ;
/*
** Test For Valid dataP Object
*/
 if( bcdtmObject_testForValidDataObject(dataP)) goto errexit ; ;
/*
** Only Process If Data Object Sorted
*/
 if( dataP->stateFlag ) 
   {
/*
** Allocate Memory For Temporary Points Array
*/
    tmpolyPtsPP = ( DTM_DATA_POINT * ) malloc ( dataP->numFeatPts * sizeof(DTM_DATA_POINT)) ;
    if( tmpolyPtsPP == nullptr ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
** Copy Data Object To Temporary Points Array
*/
    for( fcP = dataP->featureCodeP , ptsP = tmpolyPtsPP ; fcP < dataP->featureCodeP + dataP->numFeatPts ; ++fcP , ++ptsP )
      {
       ofs = *fcP / 100 ;
       ptsP->x = (dataP->pointsP+ofs)->x ;
       ptsP->y = (dataP->pointsP+ofs)->y ;
       ptsP->z = (dataP->pointsP+ofs)->z ;
      }
/*
** Reset Feature Codes
*/
    for( fcP = dataP->featureCodeP ; fcP < dataP->featureCodeP + dataP->numFeatPts ; ++fcP ) *fcP = *fcP % 100 ;
/*
** Free Data Object Points Memory
*/
    free( dataP->pointsP ) ;
/*
** Set Data Object Variables
*/
    dataP->pointsP = tmpolyPtsPP ;
    dataP->stateFlag = 0 ;
    dataP->numPts = dataP->memPts = dataP->numFeatPts ;
    tmpolyPtsPP = nullptr ;
   }
/*
** Clean Up
*/
 cleanup :
 if( tmpolyPtsPP != nullptr ) free(tmpolyPtsPP) ;
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
|											                         |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_setCurrentTinFileName(WCharCP TinFile)
{
    // Not Implemented For Vancouver 
    return(DTM_SUCCESS);
}
/*-------------------------------------------------------------------+
|                                                                    |
|											                         |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_setCurrentDataFileName(WCharCP DataFile)
{
    // Not Implemented For Vancouver 
    return(DTM_SUCCESS);
}
/*-------------------------------------------------------------------+
|                                                                    |
|											                         |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_setCurrentLatticeFileName(WCharCP LatticeFile)
{
    // Not Implemented For Vancouver 
    return(DTM_SUCCESS);
}
