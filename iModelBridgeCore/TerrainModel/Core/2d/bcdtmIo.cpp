/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmIo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
#include "bcDTMStream.h"
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmWrite_message(long MessageType,long MessageLevel,long MessageNumber,char *DtmMessage,...)
/*
** Called By DTM Functions for Outputting Messages
**
**  ==> MessageType   =  0 - Status  Message Write In MicroStation Message Field
**                    =  1 - User    Error Message
**                    =  2 - System  Error Message
**                    =  3 - Warning Message
**                    = 10 - Status  Message Write In MicroStation Error Message Field 
**  ==> MessageLevel  = Message Level
**  ==> MessageNumber = Message Number
**  ==> DtmMessage    = Message To Be Displayed
**
*/
{
 static long MessageFlag=1 ;
 char Dstr[50],Tstr[50],Message[2000] ;
 va_list arg_ptr ;
/*
** Initialise
*/
 Message[0] = Dstr[0] = Tstr[0] = '0' ;
 if( MessageType >= 1 && MessageType <= 3 ) MessageLevel = 0 ;
/*
** Display Message If Message Level Is Less Than Current DTM Message Setting
*/
 if( MessageLevel <=  5 /* DTM_MSG_LEVEL */ )
   {
/*
** Format Message
*/
    if( DtmMessage != NULL )
      {
       va_start(arg_ptr,DtmMessage) ;
       vsprintf(Message,DtmMessage,arg_ptr) ;
       va_end(arg_ptr) ;
      }  
/*
** Write To Log File If Opened
*/
    if( fpLOG != NULL )
      {
       bcdtmUtl_getDateAndTime(Dstr,Tstr) ;
       fprintf(fpLOG,"%s %s\n",Tstr,Message) ;
       fflush(fpLOG) ; 
      }
/*
**  Write to Standard Out
*/
#ifndef _WIN32_WCE
   if(stdout)
    {
    if( fpLOG == NULL )
        bcdtmUtl_getDateAndTime(Dstr,Tstr) ;
    printf("%s %s\n",Tstr,Message) ;
    }
#endif
/*
**  Update Global DTM Error Register
*/
    if( MessageType >= 1 && MessageType <= 2 ) 
      {
       Message[255] = 0 ; 
       DTM_DTM_ERROR_STATUS = MessageType ;
       DTM_DTM_ERROR_NUMBER = MessageNumber ;
       strcpy(DTM_DTM_ERROR_MESSAGE,Message) ;
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
BENTLEYDTM_Public int bcdtmWrite_logMessage(long Status,char *DtmMessage,...)
/*
** Called For Outputting Dirtectly To Log File
**
**  ==> Status   = 0  - Progress Message
**               > 0  - Error Message
**  ==> Message  =  Message To Be Displayed
**
*/
{
 char Dstr[30],Tstr[30],Message[200] ;
 va_list arg_ptr ;
/*
** Initialise
*/
 Message[0] = '0' ;
 if( fpLOG != NULL )
   {
    Status = Status / 10 ;
    bcdtmUtl_getDateAndTime(Dstr,Tstr) ;
    if( DtmMessage != NULL )
      {
       va_start(arg_ptr,DtmMessage) ;
       vsprintf(Message,DtmMessage,arg_ptr) ;
       va_end(arg_ptr) ;
      }  
    fprintf(fpLOG,"%s %s\n",Tstr,Message) ;
    fflush(fpLOG) ;
   }
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmWrite_toFileDtmObject(BC_DTM_OBJ *dtmP,WCharCP dtmFileNameP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 FILE *dtmFP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm File %s",dtmFileNameP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Open Dtm File For Writing
*/
 dtmFP = bcdtmFile_open(dtmFileNameP,L"wb") ;
 if( dtmFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Dtm File %s For Writing",dtmFileNameP) ;
    goto errexit ; 
   }
/*
** Write DTM Object At Zero File Position
*/
 if( bcdtmWrite_atFilePositionDtmObject(dtmP,dtmFP,0)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( dtmFP != NULL ) fclose(dtmFP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm File %s Completed",dtmFileNameP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm File %s Error",dtmFileNameP) ;
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
BENTLEYDTM_EXPORT int bcdtmWrite_atFilePositionDtmObject(BC_DTM_OBJ *dtmP,FILE *dtmFP,long filePosition)
{
    Bentley::TerrainModel::IBcDtmStream* dtmStreamP = NULL;
    int status;
    bcdtmStream_createFromFILE(dtmFP, &dtmStreamP);
    status = bcdtmWriteStream_atFilePositionDtmObject(dtmP, dtmStreamP, filePosition);
    bcdtmStream_destroy(&dtmStreamP);
    return status;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmWriteStream_atFilePositionDtmObject(BC_DTM_OBJ *dtmP,Bentley::TerrainModel::IBcDtmStream* dtmStreamP,long filePosition)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmType = dtmP->dtmObjType ;
 long n,numPartition,remPartition ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 BC_DTM_OBJ  dtmHeader ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Writing At File Position Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmStreamP   = %p",dtmStreamP) ;
    bcdtmWrite_message(0,0,0,"filePosition = %8ld",filePosition) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test For Geopak Active
*/
 if( DTM_GEOPAK_ACTIVE == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Geopak Object") ;
    if( bcdtmWriteStream_atFilePositionGeopakObjectDtmObject(dtmP,dtmStreamP,filePosition) ) goto errexit ;
    goto cleanup ; 
   }
/*
** Test For None NULL File Pointer
*/
 if( dtmStreamP == NULL )
   {
    bcdtmWrite_message(2,0,0,"Null File Pointer") ;
    goto errexit ;
   }
/*
** Check Lower Value Of File Position
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking File Position Range") ;
 if( filePosition < 0 )
   {
    bcdtmWrite_message(2,0,0,"File Position Range Error") ;
    goto errexit ;
   }
/*
** Seek To File Position
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Seeking To File Position") ;
 if( bcdtmStream_fseek(dtmStreamP,filePosition,SEEK_SET))
   {
    bcdtmWrite_message(1,0,0,"File Seek Error") ;
    goto errexit ; 
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Seeking To File Position Completed") ;
/*
** Write Dtm Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Header         ** Size = %9ld",sizeof(BC_DTM_OBJ)) ;
/*
**  If this is a DTMElement Make Changes For A DTM Object.
*/
 memcpy(&dtmHeader,dtmP,BCDTMSize) ;
 if( dtmType == BC_DTM_ELM_TYPE) 
   {
    dtmHeader.dtmObjType = BC_DTM_OBJ_TYPE ;
    dtmHeader.DTMAllocationClass = NULL ;
   }
// if( bcdtmStream_fwrite(&dtmHeader,sizeof(BC_DTM_OBJ),1,dtmStreamP) != 1 )
 if( bcdtmStream_fwrite(&dtmHeader,DTMIOHeaderSize,1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Writing Dtm File") ;
    goto errexit ; 
   }
/*
** Write Feature Table Array
*/
 if( dtmP->numFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature Table  ** Size = %9ld",sizeof(DTM_FEATURE_TABLE)*dtmP->numFeatures) ;
/*
**  Write Features
*/
    numPartition = 0 ; 
    remPartition = 0 ;
    dtmFeatureP  = dtmP->fTablePP[numPartition] ;
    for( n = 0 ; n < dtmP->numFeatures ; ++n )
      {
/*
**     Write Feature Header
*/
       if( bcdtmStream_fwrite(dtmFeatureP,sizeof(BC_DTM_FEATURE),1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing Dtm Feature Header") ;
          goto errexit ; 
         }
/*
**     Write Feature Points
*/
       switch ( dtmFeatureP->dtmFeatureState )
         {
          case DTMFeatureState::PointsArray :
          case DTMFeatureState::TinError    :
          case DTMFeatureState::Rollback     :
          if( bcdtmStream_fwrite(bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d),1,dtmStreamP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Writing Dtm Feature Points") ;
             goto errexit ; 
            }
          break   ;

          case DTMFeatureState::OffsetsArray :
          if( bcdtmStream_fwrite(bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI),dtmFeatureP->numDtmFeaturePts*sizeof(long),1,dtmStreamP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Writing Dtm File") ;
             goto errexit ; 
            }
          break ; 

          default :
          break   ;
         } ;
/*
**     Increment For Next Feature
*/
       ++remPartition ;
       if( remPartition == dtmP->featurePartitionSize )
         {
          ++numPartition ;
          remPartition = 0 ;
          dtmFeatureP  = dtmP->fTablePP[numPartition] ;
         }
       else ++dtmFeatureP ;
      }
   }
/*
** Write Points Array
*/
 if( dtmP->numPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Points Array   ** Size = %9ld",sizeof(DPoint3d)*dtmP->numPoints) ;
/*
**  Determine Number Of Partitions
*/
    numPartition = dtmP->numPoints / dtmP->pointPartitionSize ; 
    remPartition = dtmP->numPoints % dtmP->pointPartitionSize ;
/*
**  Write Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fwrite(dtmP->pointsPP[n],sizeof(DPoint3d) * dtmP->pointPartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing Dtm File") ;
          goto errexit ; 
         }
      }
/*
**  Write Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fwrite(dtmP->pointsPP[n],sizeof(DPoint3d) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing Dtm File") ;
          goto errexit ; 
         }
      }
   }
/*
** Write Nodes Array
*/
 if( dtmP->nodesPP != NULL ) dtmP->numNodes = dtmP->numPoints ;
 if( dtmP->numNodes > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Nodes Array    ** Size = %9ld",sizeof(DTM_TIN_NODE)*dtmP->numNodes) ;
/*
**  Determine Number Of Partitions
*/
    numPartition = dtmP->numNodes / dtmP->nodePartitionSize ; 
    remPartition = dtmP->numNodes % dtmP->nodePartitionSize ;
/*
**  Write Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fwrite(dtmP->nodesPP[n],sizeof(DTM_TIN_NODE) * dtmP->nodePartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing Dtm File") ;
          goto errexit ; 
         }
      }
/*
**  Write Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fwrite(dtmP->nodesPP[n],sizeof(DTM_TIN_NODE) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing Dtm File") ;
          goto errexit ; 
         }
      }
   }
/*
** Write Circular List
*/
 dtmP->numClist = dtmP->cListPtr ;
 if( dtmP->cListPP != NULL  )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Circular List  ** Size = %9ld",sizeof(DTM_CIR_LIST)*dtmP->cListPtr) ;
/*
**  Determine Number Of Partitions
*/
    numPartition = dtmP->numClist / dtmP->clistPartitionSize ; 
    remPartition = dtmP->numClist % dtmP->clistPartitionSize ;
/*
**  Write Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fwrite(dtmP->cListPP[n],sizeof(DTM_CIR_LIST) * dtmP->clistPartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing Dtm File") ;
          goto errexit ; 
         }
      }
/*
**  Write Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fwrite(dtmP->cListPP[n],sizeof(DTM_CIR_LIST) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing Dtm File") ;
          goto errexit ; 
         }
      }
   }
/*
** Write Feature List Array
*/
 if( dtmP->numFlist > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature List   ** Size = %9ld",sizeof(DTM_FEATURE_LIST)*dtmP->numFlist) ;
/*
**  Determine Number Of Partitions
*/
    numPartition = dtmP->numFlist / dtmP->flistPartitionSize ; 
    remPartition = dtmP->numFlist % dtmP->flistPartitionSize ;
/*
**  Write Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fwrite(dtmP->fListPP[n],sizeof(DTM_FEATURE_LIST) * dtmP->flistPartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing Dtm File") ;
          goto errexit ; 
         }
      }
/*
**  Write Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fwrite(dtmP->fListPP[n],sizeof(DTM_FEATURE_LIST) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing Dtm File") ;
          goto errexit ; 
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
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing At File Position Dtm Object Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing At File Position Dtm Object Error") ;
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
BENTLEYDTM_EXPORT int bcdtmRead_fromFileDtmObject(BC_DTM_OBJ **dtmPP,WCharCP dtmFileNameP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 FILE *dtmFP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm File %ws",dtmFileNameP) ;
/*
** Open Dtm File For Reading
*/
 dtmFP = bcdtmFile_open(dtmFileNameP,L"rb") ;
 if( dtmFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Dtm File %ws For Reading",dtmFileNameP) ;
    goto errexit ; 
   }
/*
** Read DTM Object At Zero File Position
*/
 if( bcdtmRead_atFilePositionDtmObject(dtmPP,dtmFP,0)) goto errexit ;
    
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"After Read ** dtmP = %p",*dtmPP) ;
    bcdtmWrite_message(0,0,0,"dtmP->fTablePP = %p ** numFeatures = %8ld memFeatures = %8ld",(*dtmPP)->fTablePP,(*dtmPP)->numFeatures,(*dtmPP)->memFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->pointsPP = %p ** numPoints   = %8ld memPoints   = %8ld",(*dtmPP)->pointsPP,(*dtmPP)->numPoints,(*dtmPP)->memPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->nodesPP  = %p ** numNodes    = %8ld memNodes    = %8ld",(*dtmPP)->nodesPP,(*dtmPP)->numNodes,(*dtmPP)->memNodes ) ;
    bcdtmWrite_message(0,0,0,"dtmP->cListPP  = %p ** numClist    = %8ld memClist    = %8ld",(*dtmPP)->cListPP,(*dtmPP)->numClist,(*dtmPP)->memClist ) ;
    bcdtmWrite_message(0,0,0,"dtmP->fListPP  = %p ** numFlist    = %8ld memFlist    = %8ld",(*dtmPP)->fListPP,(*dtmPP)->numFlist,(*dtmPP)->memFlist ) ;
    bcdtmWrite_message(0,0,0,"dtmP->cListPtr = %10ld ** dtmP->cListDelPtr  = %10ld ** dtmP->fListDelPtr = %10ld",(*dtmPP)->cListPtr,(*dtmPP)->cListDelPtr,(*dtmPP)->fListDelPtr ) ;
   }
 
/*
** Clean Up
*/
 cleanup :
 if( dtmFP != NULL ) fclose(dtmFP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Dtm File %ws Completed",dtmFileNameP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Dtm File %ws Error",dtmFileNameP) ;
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
BENTLEYDTM_EXPORT int bcdtmRead_atFilePositionDtmObject(BC_DTM_OBJ **dtmPP,FILE *dtmFP,long filePosition)
{
 Bentley::TerrainModel::IBcDtmStream* dtmStreamP = NULL;
 int status;
 bcdtmStream_createFromFILE(dtmFP, &dtmStreamP);
 status = bcdtmReadStream_atFilePositionDtmObject(dtmPP, dtmStreamP, filePosition);
 bcdtmStream_destroy(&dtmStreamP);
 return status;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmReadStream_atFilePositionDtmObject(BC_DTM_OBJ **dtmPP, Bentley::TerrainModel::IBcDtmStream* dtmStreamP,long filePosition)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long objType,verNum,offset,ident,verNumber,fileType ;
 char buffer[128] ;
 DTM_DAT_OBJ      *dataP=NULL ;
 DTM_TIN_OBJ      *tinP=NULL ;
 DTM_CIR_LIST     *clistP ;
 DTM_FEATURE_LIST *flistP ; 
 DTM_TIN_NODE     *nodeP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Object At File Position  = %10ld",filePosition) ;
/*
** Check For A NULL DTM Object
*/
 if( *dtmPP != NULL )
   {
    bcdtmWrite_message(2,0,0,"None NULL DTM Object") ;
    goto errexit ;
   }
/*
** Test For None NULL File Pointer
*/
 if( dtmStreamP == NULL )
   {
    bcdtmWrite_message(2,0,0,"Null Stream Pointer") ;
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
 if( bcdtmStream_fseek(dtmStreamP,filePosition,SEEK_SET))
   {
    bcdtmWrite_message(1,0,0,"File Seek Error") ;
    goto errexit ; 
   }
/*
** Read File Type And Version Number
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading File Type And Version Number") ;
 if( bcdtmStream_fread(buffer,8,1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Not A Bentley Civil Dtm File") ;
    goto errexit ;
   }
/*
** Reposition File Pointer
*/
 bcdtmStream_fseek(dtmStreamP,-8,SEEK_CUR) ; 
/*
** Get Object Type And Version Number ;
*/  
 memcpy(&objType,&buffer[0],4) ;
 memcpy(&verNum,&buffer[4],4) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Object Type ** %8x",objType) ;
/*
** Check For Early Version Geopak Tin Type
*/
 if( objType != DTM_DAT_TYPE     && objType != DTM_TIN_TYPE    && 
     objType != BC_DTM_OBJ_TYPE  && objType != BC_DTM_ELM_TYPE && 
     objType != BC_DTM_MRES_TYPE && objType != BC_DTMFeatureType  )
   {
    if( bcdtmStream_fseek(dtmStreamP,filePosition,SEEK_SET)  ) 
      { 
       bcdtmWrite_message(2,0,0,"File Seek Error ** fseek  = %s",strerror(errno))   ;
       goto errexit  ; 
      }
    if( bcdtmStream_fread(buffer,48,1,dtmStreamP) != 1 ) 
      { 
       bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
       goto errexit  ;
      }
    memcpy(&verNumber,&buffer[ 0],4) ;
    memcpy(&ident,&buffer[11],4) ;
    memcpy(&fileType,&buffer[ 1],4) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"ident ** %8x",ident) ;
    if( ident == 0x124c300 ) 
      {
       objType = DTM_TIN_TYPE ;
       verNum  = 3 ;
      }
    else if( ( verNumber == 10 || verNumber == 20 ) && fileType == 1 ) 
      {
       objType = DTM_TIN_TYPE ;
       verNum  = verNumber ;
      }
    else
      {
       objType = DTM_TIN_TYPE ;
      }  
   }
/*
** Write Object Type
*/
 if( dbg )
   {
    if     ( objType == DTM_DAT_TYPE        ) bcdtmWrite_message(0,0,0,"Reading Geopak Binary Data Object") ;
    else if( objType == DTM_TIN_TYPE        ) bcdtmWrite_message(0,0,0,"Reading Geopak Tin Object") ;
    else if( objType == BC_DTM_OBJ_TYPE     ) bcdtmWrite_message(0,0,0,"Reading bcLib DTM Object") ;
    else if( objType == BC_DTM_ELM_TYPE     ) bcdtmWrite_message(0,0,0,"Reading bcLib DTM Element") ;
    else if( objType == BC_DTM_MRES_TYPE    ) bcdtmWrite_message(0,0,0,"Reading bcLib DTM Multi Res") ;
    else if( objType == BC_DTMFeatureType ) bcdtmWrite_message(0,0,0,"Reading bcLib DTM Feature Type") ;
    else                                      bcdtmWrite_message(0,0,0,"Reading Unknown DTM Type") ;
   }
/*
** Read DTM Object Type
*/
 switch( objType )
   {
    case DTM_DAT_TYPE :
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Geopak Data Object") ;
       if( bcdtmReadStream_atFilePositionDataObject(&dataP,dtmStreamP,filePosition)) goto errexit ;
       if( bcdtmObject_copyDataObjectToDtmObject(dataP,dtmPP)) goto errexit ;
       if( bcdtmObject_deleteDataObject(&dataP)) goto errexit ;
    break ;

    case DTM_TIN_TYPE :
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Geopak Tin Object") ;
       if( bcdtmReadStream_atFilePositionTinObject(&tinP,dtmStreamP,filePosition)) goto errexit ;
       if( bcdtmObject_copyTinObjectToDtmObject(tinP,dtmPP)) goto errexit ;
       if( bcdtmObject_deleteTinObject(&tinP)) goto errexit ;
       if( cdbg == 1 )
         {
          bcdtmWrite_message(0,0,0,"Checking DTM Object %p ** dtmState = %2ld",*dtmPP,(*dtmPP)->dtmState) ;
          if( bcdtmCheck_tinComponentDtmObject(*dtmPP))
            {
             bcdtmWrite_message(2,0,0,"Checking DTM Object %p Error",*dtmPP) ;
             goto errexit ;
            }
          bcdtmWrite_message(2,0,0,"Checking DTM Object %p Completed",*dtmPP) ;
         } 
    break ;

    case BC_DTM_OBJ_TYPE :
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Bentley Civil Dtm Object") ;
      if( bcdtmObject_createDtmObject(dtmPP)) goto errexit ;
      switch( verNum )
        {
         case  BC_DTM_OBJ_VERSION_100 : 
           if( dbg ) bcdtmWrite_message(0,0,0,"BC_DTM_OBJ_VERSION_100") ;
           if( bcdtmReadStream_version100DtmObject(*dtmPP,dtmStreamP)) goto errexit ;
         break ;

         case  BC_DTM_OBJ_VERSION_200 :
           if( dbg ) bcdtmWrite_message(0,0,0,"BC_DTM_OBJ_VERSION_200") ;
           if( bcdtmReadStream_version200DtmObject(*dtmPP,dtmStreamP)) goto errexit ;
         break ;

         case  BC_DTM_OBJ_VERSION :
           if( dbg ) bcdtmWrite_message(0,0,0,"BC_DTM_OBJ_VERSION") ;
            if( bcdtmReadStream_dtmObject(*dtmPP,dtmStreamP)) goto errexit ;
         break ;

         default :
            bcdtmWrite_message(1,0,0,"Incorrect Version For Bentley Civil Dtm Object") ;
            goto errexit ;
         break ;
        } ;
/*
**     Check DTM
*/
       if( cdbg == 1 )
         {
          bcdtmWrite_message(0,0,0,"Checking DTM Object %p",*dtmPP) ;
          if( bcdtmCheck_tinComponentDtmObject(*dtmPP))
            {
             bcdtmWrite_message(2,0,0,"Checking DTM Object %p Error",*dtmPP) ;
             goto errexit ;
            }
          bcdtmWrite_message(2,0,0,"Checking DTM Object %p Completed",*dtmPP) ;
         } 
    break ;

    default :
      bcdtmWrite_message(1,0,0,"Not A Bentley Civil Dtm Object Type ** %8x",objType) ;
      goto errexit ;
    break ;
   }  ;
/*
** Reset Null Point And Null PTR Values
*/
 if( (*dtmPP)->nullPnt != DTM_NULL_PNT || (*dtmPP)->nullPtr != DTM_NULL_PTR )
   {
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Resetting DTM Null Pnt and Null Ptr Values") ;
       bcdtmWrite_message(0,0,0,"(*dtmPP)->nullPnt = %10ld DTM_NULL_PNT = %10ld",(*dtmPP)->nullPnt,DTM_NULL_PNT) ;
       bcdtmWrite_message(0,0,0,"(*dtmPP)->nullPtr = %10ld DTM_NULL_PTR = %10ld",(*dtmPP)->nullPtr,DTM_NULL_PTR) ;
      }
/*
**  Scan Nodes Array And Rest Null Values
*/
    if( (*dtmPP)->nodesPP != NULL )
      {
       for( offset = 0 ; offset < (*dtmPP)->memPoints ; ++offset )
         {
          nodeP = nodeAddrP(*dtmPP,offset) ;
          if( nodeP->hPtr  == (*dtmPP)->nullPnt ) nodeP->hPtr  = DTM_NULL_PNT ;
          if( nodeP->tPtr  == (*dtmPP)->nullPnt ) nodeP->tPtr  = DTM_NULL_PNT ;
          if( nodeP->sPtr  == (*dtmPP)->nullPnt ) nodeP->sPtr  = DTM_NULL_PNT ;
          if( nodeP->cPtr  == (*dtmPP)->nullPtr ) nodeP->cPtr  = DTM_NULL_PTR ;
          if( nodeP->fPtr  == (*dtmPP)->nullPtr ) nodeP->fPtr  = DTM_NULL_PTR ;
         }
      }
/*
**  Scan Circular List And Reset Null Values
*/
    if( (*dtmPP)->cListPP != NULL )
      {
       for( offset = 0 ; offset < (*dtmPP)->cListPtr ; ++offset )
         {
          clistP = clistAddrP(*dtmPP,offset) ;
          if( clistP->pntNum  == (*dtmPP)->nullPnt ) clistP->pntNum  = DTM_NULL_PNT ;
          if( clistP->nextPtr == (*dtmPP)->nullPtr ) clistP->nextPtr = DTM_NULL_PTR ;
         }
      }
/*
**  Scan Feature List And Reset Null Values
*/
    if( (*dtmPP)->fListPP != NULL )
      {
       for( offset = 0 ; offset < (*dtmPP)->memFlist ; ++offset )
         {
          flistP = flistAddrP(*dtmPP,offset) ;
          if( flistP->nextPnt == (*dtmPP)->nullPnt ) flistP->nextPnt = DTM_NULL_PNT ;
          if( flistP->nextPtr == (*dtmPP)->nullPtr ) flistP->nextPtr = DTM_NULL_PTR ;
         }
      }
/*
**  Reset Null Pnt And Null Ptr Values
*/
    if( (*dtmPP)->cListDelPtr == (*dtmPP)->nullPtr ) (*dtmPP)->cListDelPtr = DTM_NULL_PTR ;
    if( (*dtmPP)->fListDelPtr == (*dtmPP)->nullPtr ) (*dtmPP)->fListDelPtr = DTM_NULL_PTR ;
    (*dtmPP)->nullPnt = DTM_NULL_PNT ;
    (*dtmPP)->nullPtr = DTM_NULL_PTR ;
   }
/*
**  Check DTM
*/
 if( cdbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Checking DTM Object %p",*dtmPP) ;
    if( bcdtmCheck_tinComponentDtmObject(*dtmPP))
      {
       bcdtmWrite_message(2,0,0,"Checking DTM Object %p Error",*dtmPP) ;
       goto errexit ;
      }
    bcdtmWrite_message(2,0,0,"Checking DTM Object %p Completed",*dtmPP) ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( dataP != NULL ) bcdtmObject_deleteDataObject(&dataP) ;
 if( tinP  != NULL ) bcdtmObject_deleteTinObject(&tinP) ;
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
 if( *dtmPP != NULL ) bcdtmObject_destroyDtmObject(dtmPP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmRead_version100DtmObject(BC_DTM_OBJ *dtmP,FILE *dtmFP)
{
Bentley::TerrainModel::IBcDtmStream* dtmStreamP = NULL;
int status;

bcdtmStream_createFromFILE(dtmFP, &dtmStreamP);
status = bcdtmReadStream_version100DtmObject(dtmP, dtmStreamP);
bcdtmStream_destroy(&dtmStreamP);
return status;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmReadStream_version100DtmObject(BC_DTM_OBJ *dtmP,Bentley::TerrainModel::IBcDtmStream* dtmStreamP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,numPartition,remPartition,headerSize ;
 long numFeatures,numPoints,numNodes,numClist,numFlist,flist ;
 BC_DTM_OBJ_VER_100 dtmHeader ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 BC_DTM_FEATURE_VER100 oldFeature;
 DTM_FEATURE_LIST_VER200 oldFeatureList ;
 DTM_FEATURE_LIST *flistP ;
// DPoint3d *pntP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Object %p",dtmP) ;
/*
** Set Header Size
*/
 headerSize = sizeof(BC_DTM_OBJ_VER_100) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"headerSize = %8ld",headerSize) ;
 #ifndef _M_IX86
 headerSize = headerSize - 8 ;
 #endif ;
/*
** Read Dtm Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Header         ** Memory Size = %9ld",headerSize) ;
 if( bcdtmStream_fread(&dtmHeader,headerSize,1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Reading Dtm Header") ;
    goto errexit ; 
   }
/*
** Write Stats On Arrays
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"numPoints   = %8ld memPoints   = %8ld",dtmHeader.numPoints,dtmHeader.memPoints) ;
    bcdtmWrite_message(0,0,0,"numNodes    = %8ld memNodes    = %8ld",dtmHeader.numNodes,dtmHeader.memNodes ) ;
    bcdtmWrite_message(0,0,0,"clistPtr    = %8ld numClist    = %8ld memClist    = %8ld",dtmHeader.cListPtr,dtmHeader.numClist,dtmHeader.memClist ) ;
    bcdtmWrite_message(0,0,0,"numFeatures = %8ld memFeatures = %8ld",dtmHeader.numFeatures,dtmHeader.memFeatures) ;
    bcdtmWrite_message(0,0,0,"numFlist    = %8ld memFlist    = %8ld",dtmHeader.numFlist,dtmHeader.memFlist ) ;
   }
/*
** Copy header Values To DTM Object
*/
 dtmP->numLines             =  dtmHeader.numLines ;
 dtmP->numTriangles         =  dtmHeader.numTriangles ;
 dtmP->numFeatures          =  dtmHeader.numFeatures ;
 dtmP->memFeatures          =  dtmHeader.memFeatures ;
 dtmP->iniFeatures          =  dtmHeader.iniFeatures ;
 dtmP->incFeatures          =  dtmHeader.incFeatures ;
 dtmP->numFeaturePartitions =  dtmHeader.numFeaturePartitions ;
 dtmP->featurePartitionSize =  dtmHeader.featurePartitionSize ;
 dtmP->numPoints            =  dtmHeader.numPoints ;
 dtmP->memPoints            =  dtmHeader.memPoints ;
 dtmP->iniPoints            =  dtmHeader.iniPoints ;
 dtmP->incPoints            =  dtmHeader.incPoints ;
 dtmP->numPointPartitions   =  dtmHeader.numPointPartitions ;
 dtmP->pointPartitionSize   =  dtmHeader.pointPartitionSize ;
 dtmP->numNodes             =  dtmHeader.numNodes ;
 dtmP->memNodes             =  dtmHeader.memNodes ;
 dtmP->numNodePartitions    =  dtmHeader.numNodePartitions ;
 dtmP->nodePartitionSize    =  dtmHeader.nodePartitionSize ;
 dtmP->numClist             =  dtmHeader.numClist ;
 dtmP->memClist             =  dtmHeader.memClist ;
 dtmP->numClistPartitions   =  dtmHeader.numClistPartitions ;
 dtmP->clistPartitionSize   =  dtmHeader.clistPartitionSize ;
 dtmP->numFlist             =  dtmHeader.numFlist ;
 dtmP->memFlist             =  dtmHeader.memFlist ;
 dtmP->iniFlist             =  dtmHeader.iniFlist ;
 dtmP->incFlist             =  dtmHeader.incFlist ;
 dtmP->numFlistPartitions   =  dtmHeader.numFlistPartitions ;
 dtmP->flistPartitionSize   =  dtmHeader.flistPartitionSize ;
 dtmP->dtmState             =  dtmHeader.dtmState ;
 dtmP->dtmCleanUp           =  DTMCleanupFlags::None;
 dtmP->obsolete_dtmRestoreTriangles  =  0 ;
 dtmP->nullPnt              =  dtmHeader.nullPnt ;
 dtmP->nullPtr              =  dtmHeader.nullPtr ;
 dtmP->nullUserTag          =  dtmHeader.nullUserTag ;
 dtmP->nullFeatureId        =  dtmHeader.nullFeatureId ;
 dtmP->edgeOption           =  2  ;
 dtmP->cListPtr             =  dtmHeader.cListPtr  ;
 dtmP->cListDelPtr          =  dtmHeader.cListDelPtr ;
 dtmP->fListDelPtr          =  dtmHeader.fListDelPtr ;
 dtmP->refCount             =  0 ;
 dtmP->userStatus           =  dtmHeader.userStatus ;
 dtmP->creationTime         =  dtmHeader.creationTime ;
 dtmP->modifiedTime         =  dtmHeader.modifiedTime ;
 dtmP->userTime             =  dtmHeader.userTime ;
 dtmP->ppTol                =  dtmHeader.ppTol ;
 dtmP->plTol                =  dtmHeader.plTol ; 
 dtmP->mppTol               =  dtmHeader.mppTol ; 
 dtmP->maxSide              =  1000.0 ; 
 dtmP->xMin                 =  dtmHeader.xMin   ;
 dtmP->yMin                 =  dtmHeader.yMin   ;
 dtmP->zMin                 =  dtmHeader.zMin   ;
 dtmP->xMax                 =  dtmHeader.xMax   ;
 dtmP->yMax                 =  dtmHeader.yMax   ;
 dtmP->zMax                 =  dtmHeader.zMax   ;
 dtmP->xRange               =  dtmHeader.xRange ;
 dtmP->yRange               =  dtmHeader.yRange ;
 dtmP->zRange               =  dtmHeader.zRange ;
 dtmP->fTablePP             =  NULL ;
 dtmP->pointsPP             =  NULL ;
 dtmP->nodesPP              =  NULL ;
 dtmP->cListPP              =  NULL ;
 dtmP->fListPP              =  NULL ;
 dtmP->DTMAllocationClass   =  NULL ;
/*
** Allocation Memory For DTM Feature Table Array
*/
 if( dtmP->memFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Features Memory") ;
    numFeatures = dtmP->numFeatures ;
    dtmP->iniFeatures = dtmP->memFeatures ;
    dtmP->numFeatures = dtmP->memFeatures = 0 ;
    dtmP->fTablePP    = NULL ;
    if( bcdtmObject_allocateFeaturesMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Point Array
*/
 if( dtmP->memPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Points Memory") ;
    numPoints = dtmP->numPoints ;
    dtmP->iniPoints = dtmP->memPoints ;
    dtmP->numPoints = dtmP->memPoints = 0 ;
    dtmP->pointsPP    = NULL ;
    if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Nodes Array
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Nodes Memory") ;
    dtmP->numNodes = dtmP->memNodes = 0 ;
    dtmP->nodesPP  = NULL ;
    if( bcdtmObject_allocateNodesMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Circular List Array
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Circular List Memory") ;
    dtmP->numClist = dtmP->memClist = 0 ;
    dtmP->cListPP  = NULL ;
    if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Feature List Array
*/
 if( dtmP->dtmState == DTMState::Tin && dtmP->memFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Feature List Memory") ;
    numFlist = dtmP->numFlist ;
    dtmP->iniFlist = dtmP->memFlist ;
    dtmP->numFlist = dtmP->memFlist = 0 ;
    dtmP->fListPP  = NULL ;
    if( bcdtmObject_allocateFeatureListMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Read Feature Table Array
*/
 if( dtmP->fTablePP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Feature Table  ** Memory Size = %9ld",sizeof(DTM_FEATURE_TABLE)*numFeatures) ;
/*
**  Read Features
*/
    numPartition = 0 ; 
    remPartition = 0 ;
    dtmFeatureP  = dtmP->fTablePP[numPartition] ;
    for( n = 0 ; n < numFeatures ; ++n )
      {
/*
**     Read Feature Header
*/
       if( bcdtmStream_fread(&oldFeature,sizeof(BC_DTM_FEATURE_VER100),1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }

       dtmFeatureP->dtmFeatureState = oldFeature.dtmFeatureState;
       dtmFeatureP->internalToDtmFeature = oldFeature.internalToDtmFeature;
       dtmFeatureP->dtmFeatureType = oldFeature.dtmFeatureType;
       dtmFeatureP->dtmUserTag = oldFeature.dtmUserTag;
       dtmFeatureP->dtmFeatureId = oldFeature.dtmFeatureId;
       dtmFeatureP->numDtmFeaturePts = oldFeature.numDtmFeaturePts;
       dtmFeatureP->dtmFeaturePts.firstPoint = oldFeature.dtmFeaturePts.firstPoint;
/*
**     Read Feature Points
*/
       switch ( dtmFeatureP->dtmFeatureState )
         {
          case DTMFeatureState::PointsArray :
          case DTMFeatureState::TinError    :
          case DTMFeatureState::Rollback     :
          dtmFeatureP->dtmFeaturePts.pointsPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d)) ;
          if( bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Alloaction Failure") ;
             goto errexit ;
            }
          if( bcdtmStream_fread(bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d),1,dtmStreamP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
             goto errexit ; 
            }
          break   ;

          case DTMFeatureState::OffsetsArray :
          dtmFeatureP->dtmFeaturePts.offsetPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;
          if( dtmFeatureP->dtmFeaturePts.offsetPI == 0)
            {
             bcdtmWrite_message(1,0,0,"Memory Alloaction Failure") ;
             goto errexit ;
            }
          if( bcdtmStream_fread(bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI),dtmFeatureP->numDtmFeaturePts*sizeof(long),1,dtmStreamP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
             goto errexit ; 
            }
          break ; 

          default :
          break   ;
         } ;
/*
**     Increment For Next Feature
*/
       ++remPartition ;
       if( remPartition == dtmP->featurePartitionSize )
         {
          ++numPartition ;
          remPartition = 0 ;
          dtmFeatureP  = dtmP->fTablePP[numPartition] ;
         }
       else ++dtmFeatureP ;
      }
/*
**  Set Number Of Features
*/
    dtmP->numFeatures = numFeatures ;  
   }
/*
** Read Points Array
*/
 if( dtmP->pointsPP != NULL  )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Points Array   ** Memory Size = %9ld",sizeof(DPoint3d) * numPoints) ;
/*
**  Determine Number Of Partitions
*/
    numPartition = numPoints / dtmP->pointPartitionSize ; 
    remPartition = numPoints % dtmP->pointPartitionSize ;
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fread(dtmP->pointsPP[n],sizeof(DPoint3d) * dtmP->pointPartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fread(dtmP->pointsPP[n],sizeof(DPoint3d) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Set Number Of Points
*/
    dtmP->numPoints = numPoints ; 
/* 
n=0 ;
pntP = pointAddrP(dtmP,n) ;
bcdtmWrite_message(0,0,0,"point[%8ld] = %12.5lf %12.5lf %10.4lf",n,pntP->x,pntP->y,pntP->z) ;
n=dtmP->numPoints - 1 ;
pntP = pointAddrP(dtmP,n) ;
bcdtmWrite_message(0,0,0,"point[%8ld] = %12.5lf %12.5lf %10.4lf",n,pntP->x,pntP->y,pntP->z) ;
*/
   }
/*
** Read Nodes Array
*/
 if( dtmP->nodesPP != NULL )
   {
    numNodes = dtmP->numPoints ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Nodes Array    ** Memory Size = %9ld",sizeof(DPoint3d) * numNodes) ;
/*
**  Determine Number Of Partitions
*/ 
    numPartition = numNodes / dtmP->nodePartitionSize ; 
    remPartition = numNodes % dtmP->nodePartitionSize ;
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fread(dtmP->nodesPP[n],sizeof(DTM_TIN_NODE) * dtmP->nodePartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fread(dtmP->nodesPP[n],sizeof(DTM_TIN_NODE) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Set Number Of Nodes
*/
    dtmP->numNodes = numNodes ;  
  }
/*
** Read Circular List
*/
 if( dtmP->cListPP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Circular List  ** Memory Size = %9ld",sizeof(DTM_CIR_LIST)*dtmP->cListPtr) ;
/*
**  Determine Number Of Partitions
*/
    numClist = dtmP->cListPtr ;
    numPartition = numClist / dtmP->clistPartitionSize ; 
    remPartition = numClist % dtmP->clistPartitionSize ;
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fread(dtmP->cListPP[n],sizeof(DTM_CIR_LIST) * dtmP->clistPartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fread(dtmP->cListPP[n],sizeof(DTM_CIR_LIST) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Set Number Of Clist
*/
    dtmP->numClist = numClist ;  
   }
/*
** Read Feature List Array
*/
 if( dtmP->fListPP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Feature List   ** Memory Size = %9ld",sizeof(DTM_FEATURE_LIST) * numFlist) ;
    for( flist = 0 ; flist < numFlist ; ++flist )
      {
       flistP = flistAddrP(dtmP,flist) ;
       if( bcdtmStream_fread(&oldFeatureList,sizeof(DTM_FEATURE_LIST_VER200),1,dtmStreamP) != 1 ) goto errexit ;
       flistP->nextPnt = oldFeatureList.nextPnt ;
       flistP->nextPtr = oldFeatureList.nextPtr ;
       flistP->dtmFeature = oldFeatureList.dtmFeature ;
       flistP->pntType = 1 ; 
      }
/*
**  Set Number Of Flist
*/
    dtmP->numFlist = numFlist ;  
  }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_Public int bcdtmReadStream_version200DtmObject(BC_DTM_OBJ *dtmP,Bentley::TerrainModel::IBcDtmStream* dtmStreamP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,numPartition,remPartition,flist,headerSize;
 long numFeatures,numPoints,numNodes,numClist,numFlist ;
 BC_DTM_OBJ_VER_200 dtmHeader ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 BC_DTM_FEATURE_VER100 oldFeature;
 DTM_FEATURE_LIST *flistP ;
 DTM_FEATURE_LIST_VER200 oldFeatureList ;
// char buffer[100] ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Version 200 Dtm Object") ;
/*
** Set Header Size
*/
 headerSize = DTMIOHeaderSize_VER200 ;
 #ifndef _M_IX86
 headerSize = headerSize - 20 ;
 #endif ;
/*
** Read Dtm Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Header         ** Header Size = %9ld",headerSize) ;
 if( bcdtmStream_fread(&dtmHeader,headerSize,1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Reading Dtm Header") ;
    goto errexit ; 
   }
/*
** Copy header Values To DTM Object
*/
 dtmP->numLines             =  dtmHeader.numLines ;
 dtmP->numTriangles         =  dtmHeader.numTriangles ;
 dtmP->numFeatures          =  dtmHeader.numFeatures ;
 dtmP->memFeatures          =  dtmHeader.memFeatures ;
 dtmP->iniFeatures          =  dtmHeader.iniFeatures ;
 dtmP->incFeatures          =  dtmHeader.incFeatures ;
 dtmP->numFeaturePartitions =  dtmHeader.numFeaturePartitions ;
 dtmP->featurePartitionSize =  dtmHeader.featurePartitionSize ;
 dtmP->numPoints            =  dtmHeader.numPoints ;
 dtmP->numSortedPoints      =  dtmHeader.numSortedPoints ;
 dtmP->memPoints            =  dtmHeader.memPoints ;
 dtmP->iniPoints            =  dtmHeader.iniPoints ;
 dtmP->incPoints            =  dtmHeader.incPoints ;
 dtmP->numPointPartitions   =  dtmHeader.numPointPartitions ;
 dtmP->pointPartitionSize   =  dtmHeader.pointPartitionSize ;
 dtmP->numNodes             =  dtmHeader.numNodes ;
 dtmP->memNodes             =  dtmHeader.memNodes ;
 dtmP->numNodePartitions    =  dtmHeader.numNodePartitions ;
 dtmP->nodePartitionSize    =  dtmHeader.nodePartitionSize ;
 dtmP->numClist             =  dtmHeader.numClist ;
 dtmP->memClist             =  dtmHeader.memClist ;
 dtmP->numClistPartitions   =  dtmHeader.numClistPartitions ;
 dtmP->clistPartitionSize   =  dtmHeader.clistPartitionSize ;
 dtmP->numFlist             =  dtmHeader.numFlist ;
 dtmP->memFlist             =  dtmHeader.memFlist ;
 dtmP->iniFlist             =  dtmHeader.iniFlist ;
 dtmP->incFlist             =  dtmHeader.incFlist ;
 dtmP->numFlistPartitions   =  dtmHeader.numFlistPartitions ;
 dtmP->flistPartitionSize   =  dtmHeader.flistPartitionSize ;
 dtmP->dtmState             =  dtmHeader.dtmState ;
 dtmP->dtmCleanUp           =  DTMCleanupFlags::None;
 dtmP->obsolete_dtmRestoreTriangles = 0;
 dtmP->hullPoint            =  dtmHeader.hullPoint ;
 dtmP->nextHullPoint        =  dtmHeader.nextHullPoint ;    
 dtmP->nullPnt              =  dtmHeader.nullPnt ;
 dtmP->nullPtr              =  dtmHeader.nullPtr ;
 dtmP->nullUserTag          =  dtmHeader.nullUserTag ;
 dtmP->nullFeatureId        =  dtmHeader.nullFeatureId ;
 dtmP->edgeOption           =  dtmHeader.edgeOption ;
 dtmP->cListPtr             =  dtmHeader.cListPtr  ;
 dtmP->cListDelPtr          =  dtmHeader.cListDelPtr ;
 dtmP->fListDelPtr          =  dtmHeader.fListDelPtr ;
 dtmP->refCount             =  0 ;
 dtmP->userStatus           =  dtmHeader.userStatus ;
 dtmP->creationTime         =  dtmHeader.creationTime ;
 dtmP->modifiedTime         =  dtmHeader.modifiedTime ;
 dtmP->userTime             =  dtmHeader.userTime ;
 dtmP->ppTol                =  dtmHeader.ppTol ;
 dtmP->plTol                =  dtmHeader.plTol ; 
 dtmP->mppTol               =  dtmHeader.mppTol ; 
 dtmP->maxSide              =  dtmHeader.maxSide ; 
 dtmP->xMin                 =  dtmHeader.xMin   ;
 dtmP->yMin                 =  dtmHeader.yMin   ;
 dtmP->zMin                 =  dtmHeader.zMin   ;
 dtmP->xMax                 =  dtmHeader.xMax   ;
 dtmP->yMax                 =  dtmHeader.yMax   ;
 dtmP->zMax                 =  dtmHeader.zMax   ;
 dtmP->xRange               =  dtmHeader.xRange ;
 dtmP->yRange               =  dtmHeader.yRange ;
 dtmP->zRange               =  dtmHeader.zRange ;
 dtmP->fTablePP             =  NULL ;
 dtmP->pointsPP             =  NULL ;
 dtmP->nodesPP              =  NULL ;
 dtmP->cListPP              =  NULL ;
 dtmP->fListPP              =  NULL ;
 dtmP->DTMAllocationClass   =  NULL ;
/*
** Write Stats On Arrays
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dtmP->dtmState = %2ld",dtmP->dtmState) ;
    bcdtmWrite_message(0,0,0,"numFeatures = %8ld memFeatures = %8ld",dtmP->numFeatures,dtmP->memFeatures) ;
    bcdtmWrite_message(0,0,0,"numPoints   = %8ld memPoints   = %8ld",dtmP->numPoints,dtmP->memPoints) ;
    bcdtmWrite_message(0,0,0,"numNodes    = %8ld memNodes    = %8ld",dtmP->numNodes,dtmP->memNodes ) ;
    bcdtmWrite_message(0,0,0,"numClist    = %8ld memClist    = %8ld",dtmP->numClist,dtmP->memClist ) ;
    bcdtmWrite_message(0,0,0,"numFlist    = %8ld memFlist    = %8ld",dtmP->numFlist,dtmP->memFlist ) ;
   }
/*
** Allocation Memory For DTM Feature Table Array
*/
 if( dtmP->memFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Features Memory") ;
    numFeatures = dtmP->numFeatures ;
    dtmP->iniFeatures = dtmP->memFeatures ;
    dtmP->numFeatures = dtmP->memFeatures = 0 ;
    dtmP->fTablePP    = NULL ;
    if( bcdtmObject_allocateFeaturesMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Point Array
*/
 if( dtmP->memPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Points Memory") ;
    numPoints = dtmP->numPoints ;
    dtmP->iniPoints = dtmP->memPoints ;
    dtmP->numPoints = dtmP->memPoints = 0 ;
    dtmP->pointsPP    = NULL ;
    if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Nodes Array
*/
 if(  dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Nodes Memory") ;
    dtmP->numNodes = dtmP->memNodes = 0 ;
    dtmP->nodesPP  = NULL ;
    if( bcdtmObject_allocateNodesMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Circular List Array
*/
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Circular List Memory") ;
    dtmP->numClist = dtmP->memClist = 0 ;
    dtmP->cListPP  = NULL ;
    if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Feature List Array
*/
 if( dtmP->dtmState == DTMState::Tin && dtmP->memFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Feature List Memory") ;
    numFlist = dtmP->numFlist ;
    dtmP->iniFlist = dtmP->memFlist ;
    dtmP->numFlist = dtmP->memFlist = 0 ;
    dtmP->fListPP  = NULL ;
    if( bcdtmObject_allocateFeatureListMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Read Feature Table Array
*/
 if( dtmP->fTablePP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Feature Table  ** Memory Size = %9ld",sizeof(DTM_FEATURE_TABLE)*numFeatures) ;
/*
**  Read Features
*/
    numPartition = 0 ; 
    remPartition = 0 ;
    dtmFeatureP  = dtmP->fTablePP[numPartition] ;

    for( n = 0 ; n < numFeatures ; ++n )
      {
/*
**     Read Feature Header
*/
       if( bcdtmStream_fread(&oldFeature,sizeof(BC_DTM_FEATURE_VER100),1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }

       dtmFeatureP->dtmFeatureState = oldFeature.dtmFeatureState;
       dtmFeatureP->internalToDtmFeature = oldFeature.internalToDtmFeature;
       dtmFeatureP->dtmFeatureType = oldFeature.dtmFeatureType;
       dtmFeatureP->dtmUserTag = oldFeature.dtmUserTag;
       dtmFeatureP->dtmFeatureId = oldFeature.dtmFeatureId;
       dtmFeatureP->numDtmFeaturePts = oldFeature.numDtmFeaturePts;
       dtmFeatureP->dtmFeaturePts.firstPoint = oldFeature.dtmFeaturePts.firstPoint;
 //bcdtmWrite_message(0,0,0,"Feature[%8ld] ** Type = %2ld State = %2ld numPts = %8ld firstPoint = %8ld FeatureId = %10ld ",n,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeatureState,dtmFeatureP->numDtmFeaturePts,dtmFeatureP->dtmFeaturePts.firstPoint,dtmFeatureP->dtmFeatureId) ;
/*
**     Read Feature Points
*/
       switch ( dtmFeatureP->dtmFeatureState )
         {
          case DTMFeatureState::PointsArray :
          case DTMFeatureState::TinError    :
          case DTMFeatureState::Rollback     :
          dtmFeatureP->dtmFeaturePts.pointsPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d)) ;
          if( bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
          if( bcdtmStream_fread(bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d),1,dtmStreamP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
             goto errexit ; 
            }
          break   ;

          case DTMFeatureState::OffsetsArray :
          dtmFeatureP->dtmFeaturePts.offsetPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;
          if( dtmFeatureP->dtmFeaturePts.offsetPI == 0)
            {
             bcdtmWrite_message(1,0,0,"Memory Alloaction Failure") ;
             goto errexit ;
            }
          if( bcdtmStream_fread(bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI),dtmFeatureP->numDtmFeaturePts*sizeof(long),1,dtmStreamP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
             goto errexit ; 
            }
          break ; 

          default :
          break   ;
         } ;
/*
**     Increment For Next Feature
*/
       ++remPartition ;
       if( remPartition == dtmP->featurePartitionSize )
         {
          ++numPartition ;
          remPartition = 0 ;
          dtmFeatureP  = dtmP->fTablePP[numPartition] ;
         }
       else ++dtmFeatureP ;
      }
/*
**  Set Number Of Features
*/
    dtmP->numFeatures = numFeatures ;  
   }
/*
** Read Points Array
*/
 if( dtmP->pointsPP != NULL  )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Points Array   ** Memory Size = %9ld",sizeof(DPoint3d) * numPoints) ;
/*
**  Determine Number Of Partitions
*/
    numPartition = numPoints / dtmP->pointPartitionSize ; 
    remPartition = numPoints % dtmP->pointPartitionSize ;
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fread(dtmP->pointsPP[n],sizeof(DPoint3d) * dtmP->pointPartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fread(dtmP->pointsPP[n],sizeof(DPoint3d) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Set Number Of Points
*/
    dtmP->numPoints = numPoints ;  
   }
/*
** Read Nodes Array
*/
 if( dtmP->nodesPP != NULL )
   {
    numNodes = dtmP->numPoints ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Nodes Array    ** Memory Size = %9ld",sizeof(DPoint3d) * numNodes) ;
/*
**  Determine Number Of Partitions
*/ 
    numPartition = numNodes / dtmP->nodePartitionSize ; 
    remPartition = numNodes % dtmP->nodePartitionSize ;
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fread(dtmP->nodesPP[n],sizeof(DTM_TIN_NODE) * dtmP->nodePartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fread(dtmP->nodesPP[n],sizeof(DTM_TIN_NODE) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Set Number Of Nodes
*/
    dtmP->numNodes = numNodes ;  
  }
/*
** Read Circular List
*/
 if( dtmP->cListPP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Circular List  ** Memory Size = %9ld",sizeof(DTM_CIR_LIST)*dtmP->cListPtr) ;
/*
**  Determine Number Of Partitions
*/
    numClist = dtmP->cListPtr ;
    numPartition = numClist / dtmP->clistPartitionSize ; 
    remPartition = numClist % dtmP->clistPartitionSize ;
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fread(dtmP->cListPP[n],sizeof(DTM_CIR_LIST) * dtmP->clistPartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Circular List Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fread(dtmP->cListPP[n],sizeof(DTM_CIR_LIST) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Circular List Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Set Number Of Clist
*/
    dtmP->numClist = numClist ;  
   }
/*
** Read Feature List Array
*/
 if( dtmP->fListPP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Feature List   ** Memory Size = %9ld",sizeof(DTM_FEATURE_LIST) * numFlist) ;
    for( flist = 0 ; flist < numFlist ; ++flist )
      {
       flistP = flistAddrP(dtmP,flist) ;
       if( bcdtmStream_fread(&oldFeatureList,sizeof(DTM_FEATURE_LIST_VER200),1,dtmStreamP) != 1 ) goto errexit ;
       flistP->nextPnt = oldFeatureList.nextPnt ;
       flistP->nextPtr = oldFeatureList.nextPtr ;
       flistP->dtmFeature = oldFeatureList.dtmFeature ;
       flistP->pntType = 1 ; 
      }
/*
**  Set Number Of Flist
*/
    dtmP->numFlist = numFlist ;  
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_Public int bcdtmRead_dtmObject(BC_DTM_OBJ *dtmP,FILE *dtmFP)
{
 int ret=DTM_SUCCESS ;
 Bentley::TerrainModel::IBcDtmStream* dtmStreamP = NULL;
/*
** Create Stream
*/
 bcdtmStream_createFromFILE(dtmFP,&dtmStreamP);
/*
** Read From Stream
*/
 if( bcdtmReadStream_dtmObject(dtmP,dtmStreamP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( dtmStreamP != NULL ) bcdtmStream_destroy(&dtmStreamP);
/*
** Return
*/
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
BENTLEYDTM_Private void bcdtmObject_checkLastModifiedDate (BC_DTM_OBJ *dtmP)
    {
    const Int64 date1Jan1970 = 116444736000000000;
    Int64 lastModifiedTime = date1Jan1970 + ((Int64)dtmP->modifiedTime) * 10000000;

    if (dtmP->lastModifiedTime != 0)
        {
        Int64 timeDiff = dtmP->lastModifiedTime - lastModifiedTime;
        // Strip of nanoseconds and days.
        timeDiff /= 10000000;
        timeDiff /= 60 * 60 * 24;

        if (abs ((int)timeDiff) > 1)
            dtmP->lastModifiedTime = lastModifiedTime;
        }
    else
        dtmP->lastModifiedTime = lastModifiedTime;

    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmReadStream_dtmObject(BC_DTM_OBJ *dtmP,Bentley::TerrainModel::IBcDtmStream* dtmStreamP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long n,numPartition,remPartition ;
 long numFeatures,numPoints,numNodes,numClist,numFlist ;
 BC_DTM_FEATURE  *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Object %p",dtmP) ;
/*
** Read Dtm Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Header         ** Memory Size = %9ld",sizeof(BC_DTM_OBJ)) ;
// if( bcdtmStream_fread(dtmP,sizeof(BC_DTM_OBJ),1,dtmStreamP) != 1 )
 if( bcdtmStream_fread(dtmP,DTMIOHeaderSize,1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Reading Header Dtm Object %p ",dtmP) ;
    goto errexit ; 
   }

 bcdtmObject_checkLastModifiedDate(dtmP);
/*
** Write Stats On Arrays
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dtmP->fTablePP = %p ** numFeatures = %8ld memFeatures = %8ld",dtmP->fTablePP,dtmP->numFeatures,dtmP->memFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->pointsPP = %p ** numPoints   = %8ld memPoints   = %8ld",dtmP->pointsPP,dtmP->numPoints,dtmP->memPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->nodesPP  = %p ** numNodes    = %8ld memNodes    = %8ld",dtmP->nodesPP,dtmP->numNodes,dtmP->memNodes ) ;
    bcdtmWrite_message(0,0,0,"dtmP->cListPP  = %p ** numClist    = %8ld memClist    = %8ld",dtmP->cListPP,dtmP->numClist,dtmP->memClist ) ;
    bcdtmWrite_message(0,0,0,"dtmP->fListPP  = %p ** numFlist    = %8ld memFlist    = %8ld",dtmP->fListPP,dtmP->numFlist,dtmP->memFlist ) ;
    bcdtmWrite_message(0,0,0,"dtmP->cListDelPtr  = %10ld ** dtmP->fListDelPtr = %10ld",dtmP->cListDelPtr,dtmP->fListDelPtr ) ;
   }
/*
** Allocation Memory For DTM Feature Table Array
*/
// if( dtmP->fTablePP != NULL )
 if( dtmP->memFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Features Memory") ;
    numFeatures = dtmP->numFeatures ;
    dtmP->iniFeatures = dtmP->memFeatures ;
    dtmP->numFeatures = dtmP->memFeatures = 0 ;
    dtmP->fTablePP    = NULL ;
    if( bcdtmObject_allocateFeaturesMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Point Array
*/
// if( dtmP->pointsPP != NULL )
 if( dtmP->memPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Points Memory") ;
    numPoints = dtmP->numPoints ;
    dtmP->iniPoints = dtmP->memPoints ;
    dtmP->numPoints = dtmP->memPoints = 0 ;
    dtmP->pointsPP    = NULL ;
    if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Nodes Array
*/
// if( dtmP->nodesPP != NULL )
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Nodes Memory") ;
    dtmP->numNodes = dtmP->memNodes = 0 ;
    dtmP->nodesPP  = NULL ;
    if( bcdtmObject_allocateNodesMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Circular List Array
*/
// if( dtmP->cListPP != NULL )
 if( dtmP->dtmState == DTMState::Tin )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Circular List Memory") ;
    dtmP->numClist = dtmP->memClist = 0 ;
    dtmP->cListPP  = NULL ;
    if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Allocation Memory For DTM Feature List Array
*/
// if( dtmP->fListPP != NULL )
 if( dtmP->dtmState == DTMState::Tin && dtmP->memFlist > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Feature List Memory") ;
    numFlist = dtmP->numFlist ;
    dtmP->iniFlist = dtmP->memFlist ;
    dtmP->numFlist = dtmP->memFlist = 0 ;
    dtmP->fListPP  = NULL ;
    if( bcdtmObject_allocateFeatureListMemoryDtmObject(dtmP)) goto errexit ;
   }
/*
** Read Feature Table Array
*/
 if( dtmP->fTablePP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Feature Table  ** Memory Size = %9ld",sizeof(DTM_FEATURE_TABLE)*numFeatures) ;
/*
**  Read Features
*/
    numPartition = 0 ; 
    remPartition = 0 ;
    dtmFeatureP  = dtmP->fTablePP[numPartition] ;
    for( n = 0 ; n < numFeatures ; ++n )
      {
/*
**     Read Feature Header
*/
       if( bcdtmStream_fread(dtmFeatureP,sizeof(BC_DTM_FEATURE),1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
/*
**     Read Feature Points
*/
       switch ( dtmFeatureP->dtmFeatureState )
         {
          case DTMFeatureState::PointsArray :
          case DTMFeatureState::TinError    :
          case DTMFeatureState::Rollback     :
          dtmFeatureP->dtmFeaturePts.pointsPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d)) ;
          if( bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Alloaction Failure") ;
             goto errexit ;
            }
          if( bcdtmStream_fread(bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts*sizeof(DPoint3d),1,dtmStreamP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
             goto errexit ; 
            }
          break   ;

          case DTMFeatureState::OffsetsArray :
          dtmFeatureP->dtmFeaturePts.offsetPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts*sizeof(long)) ;
          if( dtmFeatureP->dtmFeaturePts.offsetPI == 0)
            {
             bcdtmWrite_message(1,0,0,"Memory Alloaction Failure") ;
             goto errexit ;
            }
          if( bcdtmStream_fread(bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI),dtmFeatureP->numDtmFeaturePts*sizeof(long),1,dtmStreamP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
             goto errexit ; 
            }
          break ; 

          default :
          break   ;
         } ;
/*
**     Increment For Next Feature
*/
       ++remPartition ;
       if( remPartition == dtmP->featurePartitionSize )
         {
          ++numPartition ;
          remPartition = 0 ;
          dtmFeatureP  = dtmP->fTablePP[numPartition] ;
         }
       else ++dtmFeatureP ;
      }
/*
**  Set Number Of Features
*/
    dtmP->numFeatures = numFeatures ;  
   }
/*
** Read Points Array
*/
 if( dtmP->pointsPP != NULL  )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Points Array   ** Memory Size = %9ld",sizeof(DPoint3d) * numPoints) ;
/*
**  Determine Number Of Partitions
*/
    numPartition = numPoints / dtmP->pointPartitionSize ; 
    remPartition = numPoints % dtmP->pointPartitionSize ;
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fread(dtmP->pointsPP[n],sizeof(DPoint3d) * dtmP->pointPartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fread(dtmP->pointsPP[n],sizeof(DPoint3d) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Set Number Of Points
*/
    dtmP->numPoints = numPoints ;  
   }
/*
** Read Nodes Array
*/
 if( dtmP->nodesPP != NULL )
   {
    numNodes = dtmP->numPoints ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Nodes Array    ** Memory Size = %9ld",sizeof(DPoint3d) * numNodes) ;
/*
**  Determine Number Of Partitions
*/ 
    numPartition = numNodes / dtmP->nodePartitionSize ; 
    remPartition = numNodes % dtmP->nodePartitionSize ;
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fread(dtmP->nodesPP[n],sizeof(DTM_TIN_NODE) * dtmP->nodePartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fread(dtmP->nodesPP[n],sizeof(DTM_TIN_NODE) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Set Number Of Nodes
*/
    dtmP->numNodes = numNodes ;  
  }
/*
** Read Circular List
*/
 if( dtmP->cListPP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Circular List  ** Memory Size = %9ld",sizeof(DTM_CIR_LIST)*dtmP->cListPtr) ;
/*
**  Determine Number Of Partitions
*/
    numClist = dtmP->cListPtr ;
    numPartition = numClist / dtmP->clistPartitionSize ; 
    remPartition = numClist % dtmP->clistPartitionSize ;
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fread(dtmP->cListPP[n],sizeof(DTM_CIR_LIST) * dtmP->clistPartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fread(dtmP->cListPP[n],sizeof(DTM_CIR_LIST) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Set Number Of Clist
*/
    dtmP->numClist = numClist ;  
   }
/*
** Read Feature List Array
*/
 if( dtmP->fListPP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Feature List   ** Memory Size = %9ld",sizeof(DTM_FEATURE_LIST) * numFlist) ;
/*
**  Determine Number Of Partitions
*/
    numPartition = numFlist / dtmP->flistPartitionSize ; 
    remPartition = numFlist % dtmP->flistPartitionSize ;
/*
**  Read Full Partitions
*/
    for( n = 0 ; n < numPartition  ; ++n )
      {
       if( bcdtmStream_fread(dtmP->fListPP[n],sizeof(DTM_FEATURE_LIST) * dtmP->flistPartitionSize,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Read Partial Partition
*/
    if( remPartition > 0 )
      {
       if( bcdtmStream_fread(dtmP->fListPP[n],sizeof(DTM_FEATURE_LIST) * remPartition,1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading Dtm Object %p ",dtmP) ;
          goto errexit ; 
         }
      }
/*
**  Set Number Of Flist
*/
    dtmP->numFlist = numFlist ;  
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Dtm Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Dtm Object %p Error",dtmP) ;
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
BENTLEYDTM_EXPORT int bcdtmRead_xyzFileToDtmObject(BC_DTM_OBJ **dtmPP,WCharCP xyzFileName)
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     fileType=0,numFileRecs=0,iniPts,incPts ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading XYZ File %s",xyzFileName) ;
/*
** Test For Existing Dtm Object
*/
 if( *dtmPP != NULL )
   {
    if( ! bcdtmObject_testForValidDtmObject(*dtmPP)) 
      {
       if( bcdtmObject_destroyDtmObject(dtmPP)) goto errexit ;
      }  
    *dtmPP = NULL ;
   } 
/*
** Auto Detect XYZ File Type
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Detecting XYZ File Type") ;
 if( bcdtmUtl_detectXYZFileType(xyzFileName,&fileType,&numFileRecs) != DTM_SUCCESS ) goto errexit ;
 if( dbg )
   { 
    if( fileType == 1 ) bcdtmWrite_message(0,0,0,"ASCII XYZ File Detected") ;
    if( fileType == 2 ) bcdtmWrite_message(0,0,0,"Binary XYZ File Detected") ;
    bcdtmWrite_message(0,0,0,"Estimated XYZ Record Entries = %6ld",numFileRecs) ;
   } 
/*
** Determine Memory Allocation Values For DTMFeatureState::Tin
*/
 incPts = numFileRecs / 10 ;
 if( incPts ==     0 ) incPts = 1000 ;
 if( incPts >  10000 ) incPts = 10000 ;
 if( fileType == 1   ) iniPts = numFileRecs + incPts ;
 else                  iniPts = numFileRecs ;
/*
** Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(dtmPP)) goto errexit ;
/*
** Set Dtm Object Memory Allocation Parameters 
*/
 (*dtmPP)->iniPoints = iniPts ;
 (*dtmPP)->incPoints = incPts ;
/*
** Read XYZ File
*/
 if( fileType == 1 ) if( bcdtmRead_xyzASCIIFileToDtmObject(*dtmPP,xyzFileName))  goto errexit ;
 if( fileType == 2 ) if( bcdtmRead_xyzBinaryFileToDtmObject(*dtmPP,xyzFileName)) goto errexit ;
/*
** Set Data Ranges
*/
 (*dtmPP)->xRange = (*dtmPP)->xMax - (*dtmPP)->xMin ;
 (*dtmPP)->yRange = (*dtmPP)->yMax - (*dtmPP)->yMin ;
 (*dtmPP)->zRange = (*dtmPP)->zMax - (*dtmPP)->zMin ;
/*
** If No Points Free memory And Return
*/
 if( (*dtmPP)->numPoints == 0 )
   {
    bcdtmObject_destroyDtmObject(dtmPP) ;
    bcdtmWrite_message(1,0,0,"No Data In %s",xyzFileName) ;
    goto errexit ; 
   }
/*
** Print XYZ Ranges
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Number of XYZ Points = %6ld",(*dtmPP)->numPoints) ;
    bcdtmWrite_message(0,0,0,"****     Minimum      Maximum       Range") ;
    bcdtmWrite_message(0,0,0,"****   ============ ============ ============") ;
    bcdtmWrite_message(0,0,0,"**** x %12.3lf %12.3lf %12.3lf",(*dtmPP)->xMin,(*dtmPP)->xMax,(*dtmPP)->xMax-(*dtmPP)->xMin) ;
    bcdtmWrite_message(0,0,0,"**** y %12.3lf %12.3lf %12.3lf",(*dtmPP)->yMin,(*dtmPP)->yMax,(*dtmPP)->yMax-(*dtmPP)->yMin) ;
    bcdtmWrite_message(0,0,0,"**** z %12.3lf %12.3lf %12.3lf",(*dtmPP)->zMin,(*dtmPP)->zMax,(*dtmPP)->zMax-(*dtmPP)->zMin) ;
   }
/*
** Clean Up
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading XYZ File %s Completed",xyzFileName) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading XYZ File %s Error",xyzFileName) ;
 cleanup :
/*
** All Done Return
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *dtmPP != NULL ) bcdtmObject_destroyDtmObject(dtmPP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmRead_xyzASCIIFileToDtmObject(BC_DTM_OBJ *dtmP,WCharCP xyzFile)
{
 int    ret=DTM_SUCCESS;
 char   inputBuffer[512],*bp,*bt ;
 double x,y,z ;
 DPoint3d    dtmPoint ;
 FILE  *xyzFP=NULL ;
/*
** Open XYZ File
*/
 if( ( xyzFP = bcdtmFile_open(xyzFile,L"r")) == NULL )
   { 
    bcdtmWrite_message(1,0,0,"Error Opening XYZ File %s",xyzFile) ;
    goto errexit ; 
   }
/*
** Read ASCII XYZ File
*/
 bp = inputBuffer ; *bp = 0 ; bt = inputBuffer + 510 ;
 while( fscanf(xyzFP,"%c",bp) != EOF )
   {
    if( *bp && *bp != 10 && *bp != 13) ++bp ;
    else
      {
       *bp = 0 ;
       if( ! bcdtmUtl_decodeXYZRecord(inputBuffer,&x,&y,&z) )
         {
          dtmPoint.x = x ;
          dtmPoint.y = y ;
          dtmPoint.z = z ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ;
         }
       bp = inputBuffer ; *bp = 0 ;
      }
    if( bp > bt ) bp = bt ;
   }
/*
** Clean Up
*/
 cleanup :
 if( xyzFP != NULL ) fclose(xyzFP) ;
/*
**  Job Completed
*/
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
BENTLEYDTM_Private int bcdtmRead_xyzBinaryFileToDtmObject(BC_DTM_OBJ *dtmP,WCharCP xyzFile)
{
 int    ret=DTM_SUCCESS ;
 char   inputBuffer[24] ;
 DPoint3d    dtmPoint ;
 FILE  *xyzFP=NULL ;
/*
** Open XYZ File
*/
 if( ( xyzFP = bcdtmFile_open(xyzFile,L"rb")) == NULL )
   { 
    bcdtmWrite_message(1,0,0,"Error Opening XYZ File %s",xyzFile) ;
    goto errexit ;
   }
/*
** Read Binary XYZ File
*/
 while( fread(inputBuffer,24,1,xyzFP) == 1 )
   {
    memcpy((char *)&dtmPoint.x ,&inputBuffer[ 0],8) ;
    memcpy((char *)&dtmPoint.y ,&inputBuffer[ 8],8) ;
    memcpy((char *)&dtmPoint.z ,&inputBuffer[16],8) ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( xyzFP != NULL ) fclose(xyzFP) ;
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
BENTLEYDTM_EXPORT int bcdtmRead_geopakDatFileToDtmObject(BC_DTM_OBJ **dtmPP,WCharCP datFileNameP)
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTM_DAT_OBJ *dataP=NULL ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Reading Geopak Dat File") ;
    bcdtmWrite_message(0,0,0,"*dtmPP           = %p",*dtmPP) ;
    bcdtmWrite_message(0,0,0,"datFileNameP     = %ws",datFileNameP) ;
   }
/*
** Test For Existing Dtm Object
*/
 if( *dtmPP != NULL )
   {
    if( ! bcdtmObject_testForValidDtmObject(*dtmPP)) 
      {
       if( bcdtmObject_destroyDtmObject(dtmPP)) goto errexit ;
      }  
    *dtmPP = NULL ;
   } 
/*
** Create Data Object
*/
 if( bcdtmObject_createDataObject(&dataP)) goto errexit ;
/*
** Read Data File To Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Geopak Dat File To Data Object") ;
 if( bcdtmRead_dataFileToDataObject(dataP,datFileNameP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dataP->numPts = %8ld",dataP->numPts) ;
/*
** Copy Data Object To Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Geopak Dat Object To DTM Object") ;
 if( bcdtmObject_copyDataObjectToDtmObject(dataP,dtmPP)) goto errexit ;
/*
** Print DTM Coordinate Ranges
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Number of Geopak Dat Points = %6ld",(*dtmPP)->numPoints) ;
    bcdtmWrite_message(0,0,0,"****     Minimum      Maximum       Range") ;
    bcdtmWrite_message(0,0,0,"****   ============ ============ ============") ;
    bcdtmWrite_message(0,0,0,"**** x %12.3lf %12.3lf %12.3lf",(*dtmPP)->xMin,(*dtmPP)->xMax,(*dtmPP)->xMax-(*dtmPP)->xMin) ;
    bcdtmWrite_message(0,0,0,"**** y %12.3lf %12.3lf %12.3lf",(*dtmPP)->yMin,(*dtmPP)->yMax,(*dtmPP)->yMax-(*dtmPP)->yMin) ;
    bcdtmWrite_message(0,0,0,"**** z %12.3lf %12.3lf %12.3lf",(*dtmPP)->zMin,(*dtmPP)->zMax,(*dtmPP)->zMax-(*dtmPP)->zMin) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dataP != NULL ) bcdtmObject_deleteDataObject(&dataP) ;
/*
**  Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Geopak Dat File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Geopak Dat File Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *dtmPP != NULL ) bcdtmObject_destroyDtmObject(dtmPP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmWrite_asciiGeopakDatFileFromDtmObject(BC_DTM_OBJ *dtmP,WCharCP datFileNameP,long numDecPts)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   dtmFeature,fCode,fsCode,fnCode,spotSize ;
 long   point,lastPoint,*pntOfsP ;
 unsigned char   *cP,*spotMarkP=NULL ;
 char   formatString[40],outBuffer[100] ;
 FILE   *dataFP=NULL ;
 DPoint3d    *p3dP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DPoint3d  *pointP=NULL ;
/*
** Write Status Message
*/
 bcdtmWrite_message(0,0,0,"Writing Dtm Object %p To ASCII Geopak Dat File %s",dtmP,datFileNameP) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing ASCII Dtm Object %p To Geopak Dat File %s",dtmP,datFileNameP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Initialise Output Format String 
*/
 if( numDecPts < 0 ) numDecPts = 0 ;
 if( numDecPts > 9 ) numDecPts = 9 ;
 strcpy(formatString,"%3ld %15.3lf %15.3lf %15.3lf") ;
 formatString[9] = formatString[17] = formatString[25] = ( char ) numDecPts + 48 ;
/*
** Print DTM Coordinate Ranges
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Number of Dtm Object Points = %6ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"****     Minimum      Maximum       Range") ;
    bcdtmWrite_message(0,0,0,"****   ============ ============ ============") ;
    bcdtmWrite_message(0,0,0,"**** x %12.3lf %12.3lf %12.3lf",dtmP->xMin,dtmP->xMax,dtmP->xMax-dtmP->xMin) ;
    bcdtmWrite_message(0,0,0,"**** y %12.3lf %12.3lf %12.3lf",dtmP->yMin,dtmP->yMax,dtmP->yMax-dtmP->yMin) ;
    bcdtmWrite_message(0,0,0,"**** z %12.3lf %12.3lf %12.3lf",dtmP->zMin,dtmP->zMax,dtmP->zMax-dtmP->zMin) ;
   }
/*
** Open File For Writing
*/
 dataFP = bcdtmFile_open(datFileNameP,L"w") ;
 if( dataFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Dat File %s",datFileNameP) ;
    goto errexit ;
   }
/*
** Switch On Dtm State
*/
 switch( dtmP->dtmState )
   {
    case DTMState::Data : 
    case DTMState::PointsSorted   : 
    case DTMState::DuplicatesRemoved :
/*
**  Allocate Mark Array For Random Spots
*/
    spotSize  = dtmP->numPoints/8 + 1 ;
    spotMarkP = ( unsigned char * ) malloc( spotSize * sizeof(char)) ;
    if( spotMarkP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( cP = spotMarkP ; cP < spotMarkP + spotSize ; ++cP ) *cP = 0 ;
/*
**  Scan Features And Write To Dat File
*/
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted )
         {
/*
**        Get Geopak Dat Feature Start And Next Codes
*/
          if( bcdtmData_getFeatureCodesForDtmFeatureType(dtmFeatureP->dtmFeatureType,&fsCode,&fnCode)) goto errexit ;
          if( fsCode == -1 || fnCode == -1 )
            {
             bcdtmWrite_message(2,0,0,"Unknown Feature Start And Next Codes") ;
             goto errexit ;
            } 
/*
**        Switch Depending Of Feature State
*/
          switch ( dtmFeatureP->dtmFeatureState )
            {
             case DTMFeatureState::Data :
               fCode     = fsCode ;
               point     = dtmFeatureP->dtmFeaturePts.firstPoint ;
               lastPoint = dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ;
               while( point <= lastPoint )
                 {
                  sprintf(outBuffer,formatString,fCode,pointP->x,pointP->y,pointP->z) ;
                  bcdtmConvert_eliminateBlanks(outBuffer) ;
                  if( fprintf(dataFP,"%s\n",outBuffer) < 0 ) 
                    {
                     bcdtmWrite_message(1,0,0,"Error Writing Data File %s",datFileNameP) ;
                     goto errexit ;
                    }
                  bcdtmFlag_setFlag(spotMarkP,point);
                  fCode = fnCode ;
                 }   
             break ;

             case DTMFeatureState::PointsArray :
             case DTMFeatureState::TinError :
             case DTMFeatureState::Rollback     :
               for( p3dP = bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ; p3dP < bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) + dtmFeatureP->numDtmFeaturePts ; ++p3dP ) 
                 {
                  sprintf(outBuffer,formatString,fCode,p3dP->x,p3dP->y,p3dP->z) ;
                  bcdtmConvert_eliminateBlanks(outBuffer) ;
                  if( fprintf(dataFP,"%s\n",outBuffer) < 0 ) 
                    {
                     bcdtmWrite_message(1,0,0,"Error Writing Data File %s",datFileNameP) ;
                     goto errexit ;
                    }
                  fCode = fnCode ;
                 }

             break ;

             case DTMFeatureState::OffsetsArray :
               for( pntOfsP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ; pntOfsP < bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) + dtmFeatureP->numDtmFeaturePts ; ++pntOfsP ) 
                 {
                  pointP = pointAddrP(dtmP,*pntOfsP) ;
                  sprintf(outBuffer,formatString,fCode,pointP->x,pointP->y,pointP->z) ;
                  bcdtmConvert_eliminateBlanks(outBuffer) ;
                  if( fprintf(dataFP,"%s\n",outBuffer) < 0 ) 
                    {
                     bcdtmWrite_message(1,0,0,"Error Writing Data File %s",datFileNameP) ;
                     goto errexit ;
                    }
                  bcdtmFlag_setFlag(spotMarkP,*pntOfsP);
                  fCode = fnCode ;
                 }
             break ;

             default :
                bcdtmWrite_message(2,0,0,"Unknown DTM Feaure State") ;
                goto errexit ;
             break ;
            } ;
         }  
      }
/*
**  Write Random Points - ( Those Not On any Features )
*/
    fCode = 1 ;
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       if( ! bcdtmFlag_testFlag(spotMarkP,point))
         {
          pointP = pointAddrP(dtmP,point) ;
          sprintf(outBuffer,formatString,fCode,pointP->x,pointP->y,pointP->z) ;
          bcdtmConvert_eliminateBlanks(outBuffer) ;
          if( fprintf(dataFP,"%s\n",outBuffer) < 0 ) 
            {
             bcdtmWrite_message(1,0,0,"Error Writing Data File %s",datFileNameP) ;
             goto errexit ;
            }
        }
      }
    break ;
 
    case DTMState::Tin :
      bcdtmWrite_message(2,0,0,"Write Geopak Dat File From Tin State Not Yet Implemented") ;
      goto errexit ;
    break ;                 
 
    default :
    break ;
   } ;
/*
** Clean Up
*/
 cleanup :
 if( spotMarkP != NULL ) free(spotMarkP) ;
 if( dataFP    != NULL ) fclose(dataFP) ;
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
BENTLEYDTM_EXPORT int bcdtmWrite_geopakDatFileFromDtmObject(BC_DTM_OBJ *dtmP,WCharCP datFileNameP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   dtmFeature,fCode,fsCode,fnCode,spotSize ;
 long   point,lastPoint,*pntOfsP ;
 unsigned char   *cP,*spotMarkP=NULL;
 char writeBuffer[40] ;
 FILE   *dataFP=NULL ;
 DPoint3d    *p3dP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DPoint3d  *pointP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Object %p To Binary Geopak Dat File %ws",dtmP,datFileNameP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Print DTM Coordinate Ranges
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Number of Dtm Object Points = %6ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"****     Minimum      Maximum       Range") ;
    bcdtmWrite_message(0,0,0,"****   ============ ============ ============") ;
    bcdtmWrite_message(0,0,0,"**** x %12.3lf %12.3lf %12.3lf",dtmP->xMin,dtmP->xMax,dtmP->xMax-dtmP->xMin) ;
    bcdtmWrite_message(0,0,0,"**** y %12.3lf %12.3lf %12.3lf",dtmP->yMin,dtmP->yMax,dtmP->yMax-dtmP->yMin) ;
    bcdtmWrite_message(0,0,0,"**** z %12.3lf %12.3lf %12.3lf",dtmP->zMin,dtmP->zMax,dtmP->zMax-dtmP->zMin) ;
   }
/*
** Open File For Writing
*/
 dataFP = bcdtmFile_open(datFileNameP,L"wb") ;
 if( dataFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Dat File %s",datFileNameP) ;
    goto errexit ;
   }
/*
** Switch On Dtm State
*/
 switch( dtmP->dtmState )
   {
    case DTMState::Data : 
    case DTMState::PointsSorted   : 
    case DTMState::DuplicatesRemoved :
/*
**  Allocate Mark Array For Random Spots
*/
    spotSize  = dtmP->numPoints/8 + 1 ;
    spotMarkP = ( unsigned char * ) malloc( spotSize * sizeof(char)) ;
    if( spotMarkP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( cP = spotMarkP ; cP < spotMarkP + spotSize ; ++cP ) *cP = 0 ;
/*
**  Scan Features And Write To Dat File
*/
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted )
         {
/*
**        Get Geopak Dat Feature Start And Next Codes
*/
          if( bcdtmData_getFeatureCodesForDtmFeatureType(dtmFeatureP->dtmFeatureType,&fsCode,&fnCode)) goto errexit ;
          if( fsCode == -1 || fnCode == -1 )
            {
             bcdtmWrite_message(2,0,0,"Unknown Feature Start And Next Codes") ;
             goto errexit ;
            } 
/*
**        Switch Depending Of Feature State
*/
          switch ( dtmFeatureP->dtmFeatureState )
            {
             case DTMFeatureState::Data :
               fCode     = fsCode ;
               point     = dtmFeatureP->dtmFeaturePts.firstPoint ;
               lastPoint = dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ;
               while( point <= lastPoint )
                 {
                  pointP = pointAddrP(dtmP,point) ;
                  memcpy(writeBuffer,&fCode,4) ;
                  memcpy(writeBuffer+4,&pointP->x,8) ;
                  memcpy(writeBuffer+12,&pointP->y,8) ;
                  memcpy(writeBuffer+20,&pointP->z,8) ;
                  if( bcdtmFwrite(writeBuffer,28,1,dataFP) != 1 )
                    {
                     bcdtmWrite_message(2,0,0,"Error Writing Geopak Dat File") ;
                     goto errexit ;
                    }
                  bcdtmFlag_setFlag(spotMarkP,point);
                  fCode = fnCode ;
                  ++point ;
                 }   
             break ;

             case DTMFeatureState::PointsArray :
             case DTMFeatureState::TinError :
             case DTMFeatureState::Rollback     :
               fCode     = fsCode ;
               for( p3dP = bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ; p3dP < bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) + dtmFeatureP->numDtmFeaturePts ; ++p3dP ) 
                 {
                  memcpy(writeBuffer,&fCode,4) ;
                  memcpy(writeBuffer+4,&p3dP->x,8) ;
                  memcpy(writeBuffer+12,&p3dP->y,8) ;
                  memcpy(writeBuffer+20,&p3dP->z,8) ;
                  if( bcdtmFwrite(writeBuffer,28,1,dataFP) != 1 )
                    {
                     bcdtmWrite_message(2,0,0,"Error Writing Geopak Dat File") ;
                     goto errexit ;
                    }
                  fCode = fnCode ;
                 }

             break ;

             case DTMFeatureState::OffsetsArray :
               fCode     = fsCode ;
               for( pntOfsP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ; pntOfsP < bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) + dtmFeatureP->numDtmFeaturePts ; ++pntOfsP ) 
                 {
                  pointP = pointAddrP(dtmP,*pntOfsP) ;
                  memcpy(writeBuffer,&fCode,4) ;
                  memcpy(writeBuffer+4,&pointP->x,8) ;
                  memcpy(writeBuffer+12,&pointP->y,8) ;
                  memcpy(writeBuffer+20,&pointP->z,8) ;
                  if( bcdtmFwrite(writeBuffer,28,1,dataFP) != 1 )
                    {
                     bcdtmWrite_message(2,0,0,"Error Writing Geopak Dat File") ;
                     goto errexit ;
                    }
                  bcdtmFlag_setFlag(spotMarkP,*pntOfsP);
                  fCode = fnCode ;
                 }
             break ;

             default :
                bcdtmWrite_message(2,0,0,"Unknown DTM Feaure State") ;
                goto errexit ;
             break ;
            } ;
         }  
      }
/*
**  Write Random Points - ( Those Not On any Features )
*/
    fCode = 1 ;
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       if( ! bcdtmFlag_testFlag(spotMarkP,point))
         {
          pointP = pointAddrP(dtmP,point) ;
          memcpy(writeBuffer,&fCode,4) ;
          memcpy(writeBuffer+4,&pointP->x,8) ;
          memcpy(writeBuffer+12,&pointP->y,8) ;
          memcpy(writeBuffer+20,&pointP->z,8) ;
          if( bcdtmFwrite(writeBuffer,28,1,dataFP) != 1 )
            {
             bcdtmWrite_message(2,0,0,"Error Writing Geopak Dat File") ;
             goto errexit ;
            }
         }
      }
    break ;
 
    case DTMState::Tin :
      bcdtmWrite_message(2,0,0,"Write Geopak Dat File From Tin State Not Yet Implemented") ;
      goto errexit ;
    break ;                 
 
    default :
    break ;
   } ;
/*
** Clean Up
*/
 cleanup :
 if( spotMarkP != NULL ) free(spotMarkP) ;
 if( dataFP    != NULL ) fclose(dataFP) ;
/*
**  Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm Object %p To Geopak Dat File %s Completed",dtmP,datFileNameP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm Object %p To Geopak Dat File %s Error",dtmP,datFileNameP) ;
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
BENTLEYDTM_EXPORT int bcdtmRead_openAndReadAtFilePositionDtmObject(BC_DTM_OBJ **dtmPP,WCharCP fileNameP,double filePosition)
{
 int ret=DTM_SUCCESS ;
 FILE  *tinFP = NULL;
/*
** Open Dtm File
*/
 if( (tinFP = bcdtmFile_open(fileNameP,L"rb")) != (FILE *) NULL )
   {
    if( bcdtmRead_atFilePositionDtmObject(dtmPP,tinFP,(long)filePosition)!= DTM_SUCCESS ) goto errexit ;
   }
 else
  {
   bcdtmWrite_message(1,0,0,"Error Opening Tin File") ;
   goto errexit ;
  }
/*
** Clean Up
*/
cleanup :
if( tinFP != NULL ) fclose(tinFP) ;
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
BENTLEYDTM_EXPORT int bcdtmWrite_atFilePositionGeopakObjectDtmObject(BC_DTM_OBJ *dtmP,FILE *dtmFP,long filePosition) 
    {
    Bentley::TerrainModel::IBcDtmStream* dtmStreamP = NULL;
    int status;

    bcdtmStream_createFromFILE(dtmFP, &dtmStreamP);
    status = bcdtmWriteStream_atFilePositionGeopakObjectDtmObject(dtmP, dtmStreamP, filePosition);
    bcdtmStream_destroy(&dtmStreamP);
    return status;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmWriteStream_atFilePositionGeopakObjectDtmObject(BC_DTM_OBJ *dtmP,Bentley::TerrainModel::IBcDtmStream* dtmStreamP,long filePosition) 
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Writing At File Position Geopak Object") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmStreamP   = %p",dtmStreamP) ;
    bcdtmWrite_message(0,0,0,"filePosition = %8ld",filePosition) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test For None NULL File Pointer
*/
 if( dtmStreamP == NULL )
   {
    bcdtmWrite_message(2,0,0,"Null Stream Pointer") ;
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
 if( bcdtmStream_fseek(dtmStreamP,filePosition,SEEK_SET))
   {
    bcdtmWrite_message(1,0,0,"File Seek Error") ;
    goto errexit ; 
   }
/*
** Test DTM Data State To Write Either A Geopak Dat Object Or Tin Object
*/   
 if( dtmP->dtmState == DTMState::Data ) if( bcdtmWriteStream_writeAtFilePositionGeopakDatObjectDtmObject(dtmP,dtmStreamP,filePosition)) goto errexit ;
 if( dtmP->dtmState == DTMState::Tin  ) if( bcdtmWriteStream_writeAtFilePositionGeopakTinObjectDtmObject(dtmP,dtmStreamP,filePosition)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing At File Position Geopak Object Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing At File Position Geopak Object Error") ;
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
BENTLEYDTM_Private int bcdtmWriteStream_writeAtFilePositionGeopakDatObjectDtmObject(BC_DTM_OBJ *dtmP,Bentley::TerrainModel::IBcDtmStream* dtmStreamP,long filePosition)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long point,dtmFeature,numDataFeatures ;
 long fcode, fsCode, fnCode, currentPoint, featurePoint, firstPoint;
 DTM_DAT_OBJ    geopakDat ;
 DPoint3d  *pointP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTMUserTag nullUserTag=DTM_NULL_USER_TAG ;

/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Geopak Data Object At File Position %8ld From DTM Object %p",filePosition,dtmP) ;
/*
** Count Number Of Data Features
*/
 numDataFeatures = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP  = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data ) ++numDataFeatures ;
   }
/*
** Initialise Tin Header
*/
 geopakDat.dtmFileType        = DTM_DAT_TYPE ;
 geopakDat.dtmFileVersion     = DTM_DAT_FILE_VERSION ;
 geopakDat.numPts             = dtmP->numPoints  ;
// geopakDat.memPts             = dtmP->memPoints  ;
 geopakDat.memPts             = dtmP->numPoints  ;
 geopakDat.iniMemPts          = dtmP->iniPoints  ;
 geopakDat.incMemPts          = dtmP->incPoints  ;
 geopakDat.numFeatPts         = dtmP->numPoints  ;
 geopakDat.refCount           = dtmP->refCount   ;
 geopakDat.userStatus         = dtmP->userStatus ;
 geopakDat.numDecDigits       = 4 ;
 geopakDat.stateFlag          = 0 ; 
 geopakDat.xMin               = dtmP->xMin ;
 geopakDat.yMin               = dtmP->yMin ;
 geopakDat.zMin               = dtmP->zMin ; 
 geopakDat.xMax               = dtmP->xMax ;
 geopakDat.yMax               = dtmP->yMax ;
 geopakDat.zMax               = dtmP->zMax ;
 geopakDat.ppTol              = dtmP->ppTol ;
 geopakDat.plTol              = dtmP->plTol ;
 geopakDat.creationTime       = dtmP->creationTime ;
 geopakDat.modifiedTime       = dtmP->modifiedTime ;
 geopakDat.userTime           = dtmP->userTime ;
 geopakDat.refCount           = dtmP->refCount ;
 geopakDat.userStatus         = dtmP->userStatus ;
 strcpy(geopakDat.userName,"") ;
 strcpy(geopakDat.dataObjectFileName,"") ;
 strcpy(geopakDat.userMessage,"") ;
 geopakDat.pointsP            = NULL ;
 geopakDat.userTagP           = NULL ;
 geopakDat.featureCodeP       = NULL ;
 geopakDat.guidP              = NULL ;

 if( dtmP->numPoints > 0 )geopakDat.pointsP      = ( DTM_DATA_POINT   * ) dtmP->pointsPP[0] ;
 if( numDataFeatures > 0 )geopakDat.userTagP     = ( DTMUserTag     * ) dtmP->fTablePP[0] ;
 if( numDataFeatures > 0 )geopakDat.featureCodeP = ( DTM_FEATURE_CODE * ) dtmP->fTablePP[0] ;
/*
** Write Dtm Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Header         ** Size = %9ld",sizeof(DTM_DAT_OBJ)) ;
 if( bcdtmStream_fwrite(&geopakDat,sizeof(DTM_DAT_OBJ),1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(0,0,0,"Error Writing Geopak Tin Header") ;
    goto errexit ;
   }
/*
** Write Points Array
*/
 if( geopakDat.pointsP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Points Array   ** Size = %9ld",sizeof(DTM_DATA_POINT)*dtmP->numPoints) ;
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       pointP = pointAddrP(dtmP,point) ;
       if( bcdtmStream_fwrite(pointP,sizeof(DPoint3d),1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(0,0,0,"Error Writing Geopak Tin Point") ;
          goto errexit ;
         }
      }
  } 
/*
** Write Feature Code Array
*/
 if( geopakDat.featureCodeP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature Codes") ;
/*
**  Write Features
*/
    currentPoint = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm User Tags") ;
       dtmFeatureP  = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Writing Feature %4ld of %4ld",dtmFeature+1,numDataFeatures) ;
/*
**        Write Random Point Feature Codes Until First Point Of Feature
*/
          fcode = (int)DTMFeatureType::RandomSpots ; 
          firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
          while( currentPoint < firstPoint )
            {
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Writing DTMFeatureType::RandomSpots currentPoint = %8ld firstPoint = %8ld",currentPoint,firstPoint) ;
             if( bcdtmStream_fwrite(&fcode,sizeof(long),1,dtmStreamP) != 1 )
               { 
                bcdtmWrite_message(0,0,0,"Error Writing Geopak Feature Code") ;
                goto errexit ;
               }
             ++currentPoint ;
            }
/*
**        Write Feature Codes
*/
          bcdtmData_getFeatureCodesForDtmFeatureType(dtmFeatureP->dtmFeatureType,&fsCode,&fnCode) ;
          fcode = fsCode ;
          for( featurePoint = firstPoint ; featurePoint < firstPoint + dtmFeatureP->numDtmFeaturePts ; ++featurePoint )
            {
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Writing Feature Code ** currentPoint = %8ld firstPoint = %8ld featurePoint = %8ld",currentPoint,firstPoint,featurePoint) ;
             if( bcdtmStream_fwrite(&fcode,sizeof(long),1,dtmStreamP) != 1 )
               {
                bcdtmWrite_message(0,0,0,"Error Writing Geopak Feature Code") ;
                goto errexit ;
               }
             fcode = fnCode ;
             ++currentPoint ;
            }
         }
      }
/*
**  Write Remaining Codes
*/
    while( currentPoint <  geopakDat.numPts )
      {
       if( bcdtmStream_fwrite(&fcode,sizeof(long),1,dtmStreamP) != 1 )
         { 
          bcdtmWrite_message(0,0,0,"Error Writing Geopak Feature Code") ;
          goto errexit ;
         }
       ++currentPoint ;
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"**** currentPoint = %8ld ** dataP->numPts = %8ld",currentPoint,geopakDat.numPts) ;
   }
/*
** Write User Tag Array
*/
 if( geopakDat.userTagP != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm User Tags") ;
/*
**  Write Features
*/
    currentPoint = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Writing Feature %4ld of %4ld",dtmFeature,numDataFeatures) ;
       dtmFeatureP  = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Writing Feature %4ld of %4ld",dtmFeature+1,numDataFeatures) ;
/*
**        Write Random Point User Tags Until First Point Of Feature
*/
          firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
          while( currentPoint < firstPoint )
            {
             if( bcdtmStream_fwrite(&nullUserTag,sizeof(DTMUserTag),1,dtmStreamP) != 1 )
               { 
                bcdtmWrite_message(0,0,0,"Error Writing Geopak User Tag") ;
                goto errexit ;
               }
             ++currentPoint ;
            }
/*
**        Write Feature Tags
*/
          for( featurePoint = firstPoint ; featurePoint < firstPoint + dtmFeatureP->numDtmFeaturePts ; ++featurePoint )
            {
             if( bcdtmStream_fwrite(&dtmFeatureP->dtmUserTag,sizeof(DTMUserTag),1,dtmStreamP) != 1 )
               {
                bcdtmWrite_message(0,0,0,"Error Writing Geopak User Tag") ;
                goto errexit ;
               }
             ++currentPoint ;
            }
         }
      }
/*
**  Write Remaining UIser Tags
*/
    while( currentPoint <  geopakDat.numPts )
      {
       if( bcdtmStream_fwrite(&nullUserTag,sizeof(DTMUserTag),1,dtmStreamP) != 1 )
         { 
          bcdtmWrite_message(0,0,0,"Error Writing Geopak User Tag") ;
          goto errexit ;
         }
       ++currentPoint ;
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"==== currentPoint = %8ld ** dataP->numPts = %8ld",currentPoint,geopakDat.numPts) ;
   }
/*
** Report File Position
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"File Position After Geaopak Data Object Write = %10ld",bcdtmStream_ftell(dtmStreamP)) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dat Object From Dtm Object At File Position Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dat Object From Dtm Object At File Position Error") ;
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
BENTLEYDTM_Public int bcdtmWrite_writeAtFilePositionGeopakTinObjectDtmObject(BC_DTM_OBJ *dtmP,FILE* dtmFP,long filePosition)
{
    Bentley::TerrainModel::IBcDtmStream* dtmStreamP = NULL;
    int status;

    bcdtmStream_createFromFILE(dtmFP,&dtmStreamP);
    status = bcdtmWriteStream_writeAtFilePositionGeopakTinObjectDtmObject(dtmP, dtmStreamP, filePosition);
    bcdtmStream_destroy(&dtmStreamP);
    return status;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmWriteStream_writeAtFilePositionGeopakTinObjectDtmObject
(
 BC_DTM_OBJ          *dtmP,
 Bentley::TerrainModel::IBcDtmStream* dtmStreamP,
 long                filePosition
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long n,point,dtmFeature,numTinFeatures,clPtr ;
 DTM_TIN_OBJ geopakTin ;
 DPoint3d *pointP ;
 DTM_TIN_NODE  *nodeP,node ;
 DTM_CIR_LIST  *clistP,clist ;
 DTM_FEATURE_TABLE  geopakFeature ;
 DTM_FEATURE_LIST *flistP ;
 DTM_FEATURE_LIST_VER200 fList ;
 BC_DTM_FEATURE *dtmFeatureP ;
 long *featureMapP=NULL,dtmTinFeature,headerSize ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Write Entry Message
*/
if( dbg ) bcdtmWrite_message(0,0,0,"Writing Tin  Object At File Position %8ld From Dtm Object %p",filePosition,dtmP) ;
/*
** Count Number Of Tin Features
*/
 numTinFeatures = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP  = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull ) ++numTinFeatures ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"numTinFeatures = %8ld",numTinFeatures) ;
/*
** Initialise Tin Header
*/
 geopakTin.dtmFileType        = DTM_TIN_TYPE ;
 geopakTin.dtmFileVersion     = DTM_TIN_FILE_VERSION ;
 geopakTin.xMin               = dtmP->xMin ;
 geopakTin.yMin               = dtmP->yMin ;
 geopakTin.zMin               = dtmP->zMin ; 
 geopakTin.xMax               = dtmP->xMax ;
 geopakTin.yMax               = dtmP->yMax ;
 geopakTin.zMax               = dtmP->zMax ;
 geopakTin.xRange             = dtmP->xRange ;
 geopakTin.yRange             = dtmP->yRange ;
 geopakTin.zRange             = dtmP->zRange ;
 geopakTin.ppTol              = dtmP->ppTol ;
 geopakTin.plTol              = dtmP->plTol ;
 geopakTin.mppTol             = dtmP->mppTol ;
 geopakTin.numDecDigits       = 4 ;
 geopakTin.numPts             = dtmP->numPoints ;
 geopakTin.numSortedPts       = dtmP->numSortedPoints ;
 geopakTin.memPts             = dtmP->numPoints ;
 geopakTin.iniMemPts          = dtmP->iniPoints ;
 geopakTin.incMemPts          = dtmP->incPoints ;
 geopakTin.numTriangles       = dtmP->numTriangles ;
 geopakTin.numLines           = dtmP->numLines ;
 geopakTin.numFeatureTable    = numTinFeatures ;
 geopakTin.memFeatureTable    = numTinFeatures ;
 geopakTin.iniMemFeatureTable = dtmP->iniFeatures ;
 geopakTin.incMemFeatureTable = dtmP->incFeatures ;
 geopakTin.numFeatureList     = dtmP->numFlist ;
 geopakTin.memFeatureList     = dtmP->numFlist ;
 geopakTin.iniMemFeatureList  = dtmP->iniFlist ;
 geopakTin.incMemFeatureList  = dtmP->incFlist ;
 geopakTin.numFeatureMap      = 0 ;
 geopakTin.memFeatureMap      = 0 ;
 geopakTin.iniMemFeatureMap   = 0 ;
 geopakTin.incMemFeatureMap   = 0 ;
 geopakTin.hullPnt            = dtmP->hullPoint ;
 geopakTin.nextHullPnt        = dtmP->nextHullPoint ;
 geopakTin.cListPtr           = dtmP->cListPtr ;
 geopakTin.nullPtr            = dtmP->nullPtr ;
 geopakTin.cListDelPtr        = dtmP->cListDelPtr ;
 geopakTin.cListLastDelPtr    = dtmP->nullPtr ;
 clPtr = dtmP->cListDelPtr ;
 while( clPtr != dtmP->nullPtr )
   {
    geopakTin.cListLastDelPtr = clPtr ;
    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
   } 
 geopakTin.featureListDelPtr  = dtmP->fListDelPtr ;
 geopakTin.nullPnt            = dtmP->nullPnt ;
 geopakTin.nullUserTag        = dtmP->nullUserTag ;
 geopakTin.nullGuid           = nullGuid    ; 
 geopakTin.creationTime       = dtmP->creationTime ;
 geopakTin.modifiedTime       = dtmP->modifiedTime ;
 geopakTin.userTime           = dtmP->userTime ;
 geopakTin.refCount           = dtmP->refCount ;
 geopakTin.userStatus         = dtmP->userStatus ;
 geopakTin.SL1 = geopakTin.SL2 = geopakTin.SL3 = geopakTin.SL4 = geopakTin.SL5 = 0 ;
 geopakTin.SI641 = geopakTin.SI642 = geopakTin.SI643 = geopakTin.SI644 = geopakTin.SI645 = 0 ;
 geopakTin.SD1 = geopakTin.SD2 = geopakTin.SD3 = geopakTin.SD4 = geopakTin.SD5 = 0.0 ;
 geopakTin.SP1 = geopakTin.SP2 = geopakTin.SP3 = geopakTin.SP4 = geopakTin.SP5 = NULL ;
 geopakTin.pointsP     = dtmP->pointsPP[0] ;
 geopakTin.nodesP      = dtmP->nodesPP[0] ;
 geopakTin.cListP      = dtmP->cListPP[0] ;
 geopakTin.fTableP     = NULL ;
 geopakTin.fListP      = NULL ;
 if( numTinFeatures > 0 )
   {
    if( dtmP->fTablePP != NULL ) geopakTin.fTableP = ( DTM_FEATURE_TABLE * ) dtmP->fTablePP[0] ;
    if( dtmP->fListPP  != NULL ) geopakTin.fListP  = ( DTM_FEATURE_LIST_VER200 *)dtmP->fListPP[0] ;
   }
 geopakTin.fMapP       = NULL ; 
 strcpy(geopakTin.userName,"") ;
 strcpy(geopakTin.tinObjectFileName,"") ;
 strcpy(geopakTin.userMessage,"") ;
/*
** Set Header Values For Backwards Comptability With Geopak SS2
*/ 
 if( geopakTin.cListDelPtr       == geopakTin.nullPtr ) geopakTin.cListDelPtr       = TIN_NULL_PTR ;
 if( geopakTin.cListLastDelPtr   == geopakTin.nullPtr ) geopakTin.cListLastDelPtr   = TIN_NULL_PTR ;
 if( geopakTin.featureListDelPtr == geopakTin.nullPtr ) geopakTin.featureListDelPtr = TIN_NULL_PTR ;
 geopakTin.nullPnt = TIN_NULL_PNT ;
 geopakTin.nullPtr = TIN_NULL_PTR ;
/*
** Write Dtm Header
*/
 headerSize = sizeof(DTM_TIN_OBJ) ;
 #ifndef _M_IX86
 headerSize = headerSize - 40 ;
 #endif ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Header         ** Size = %9ld",headerSize) ;
 if( bcdtmStream_fwrite(&geopakTin,headerSize,1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(0,0,0,"Error Writing Geopak Tin Header") ;
    goto errexit ;
   }
/*
** Write Points Array
*/
 if( dtmP->numPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Points Array   ** Size = %9ld",sizeof(DPoint3d)*dtmP->numPoints) ;
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       pointP = pointAddrP(dtmP,point) ;
       if( bcdtmStream_fwrite(pointP,sizeof(DPoint3d),1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(0,0,0,"Error Writing Geopak Tin Point") ;
          goto errexit ;
         }
      }
   }
/*
** Write Nodes Array
*/
 if( dtmP->numPoints > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Nodes  Array   ** Size = %9ld",sizeof(DPoint3d)*dtmP->numPoints) ;
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       nodeP = nodeAddrP(dtmP,point) ;
       memcpy(&node,nodeP,sizeof(DTM_TIN_NODE)) ;
       if( node.tPtr  == DTM_NULL_PNT ) node.tPtr  = TIN_NULL_PNT ;         
       if( node.sPtr  == DTM_NULL_PNT ) node.sPtr  = TIN_NULL_PNT ;         
       if( node.hPtr  == DTM_NULL_PNT ) node.hPtr  = TIN_NULL_PNT ;         
       if( node.cPtr  == DTM_NULL_PTR ) node.cPtr  = TIN_NULL_PTR ;         
       if( node.fPtr  == DTM_NULL_PTR ) node.fPtr  = TIN_NULL_PTR ;         
       if( bcdtmStream_fwrite(&node,sizeof(DTM_TIN_NODE),1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(0,0,0,"Error Writing Geopak Tin Point") ;
          goto errexit ;
         }
      }
   }
/*
** Write Circular List
*/
 if( dtmP->cListPtr > 0   )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Circular List  ** Size = %9ld",sizeof(DTM_CIR_LIST)*dtmP->cListPtr) ;
    for( point = 0 ; point < dtmP->cListPtr; ++point )
      {
       clistP = clistAddrP(dtmP,point) ;
       memcpy(&clist,clistP,sizeof(DTM_CIR_LIST)) ;
       if( clist.pntNum  == DTM_NULL_PNT ) clist.pntNum  = TIN_NULL_PNT ;
       if( clist.nextPtr == DTM_NULL_PTR ) clist.nextPtr = TIN_NULL_PTR ;
       if( bcdtmStream_fwrite(&clist,sizeof(DTM_CIR_LIST),1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(0,0,0,"Error Writing Geopak Circular List") ;
          goto errexit ;
         }
      }
   }
/*
** Write Feature Table Array
*/
 if( numTinFeatures > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature Table  ** Size = %9ld",sizeof(DTM_FEATURE_TABLE)*numTinFeatures) ;
    featureMapP = ( long *) malloc(dtmP->numFeatures*sizeof(long)) ;
    if( featureMapP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Write Features
*/
    dtmTinFeature = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP  = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull )
         {
          if( dbg == 2  ) bcdtmWrite_message(0,0,0,"Writing Feature %4ld",dtmFeatureP->dtmFeatureType) ;
          geopakFeature.firstPnt           = dtmFeatureP->dtmFeaturePts.firstPoint ;
          geopakFeature.dtmFeatureType     = dtmFeatureP->dtmFeatureType ;
          geopakFeature.internalToFeature  = dtmFeatureP->internalToDtmFeature ;
          geopakFeature.userTag            = dtmFeatureP->dtmUserTag ;
          geopakFeature.userGuid           = nullGuid ;
/*
**        Set Feature Map
*/
          *(featureMapP+dtmFeature) = dtmTinFeature ;
          ++dtmTinFeature ;
/*
**        Write Feature Header
*/
          if( bcdtmStream_fwrite(&geopakFeature,sizeof(DTM_FEATURE_TABLE),1,dtmStreamP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Writing Dtm Feature") ;
             goto errexit ; 
            }
         }
      }
   }
/*
** Write Feature List Array
*/
 if( dtmP->numFlist > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature List   ** Size = %9ld",sizeof(DTM_FEATURE_LIST)*dtmP->numFlist) ;
/*
**  Determine Number Of Partitions
*/
    for( n = 0 ; n < dtmP->numFlist ; ++n )
      {
       flistP = flistAddrP(dtmP,n) ;
       fList.nextPnt = flistP->nextPnt ;
       fList.nextPtr = flistP->nextPtr ;
       fList.dtmFeature = *(featureMapP+flistP->dtmFeature) ; 
       if( fList.nextPnt == DTM_NULL_PNT ) fList.nextPnt = TIN_NULL_PNT ;
       if( fList.nextPtr == DTM_NULL_PTR ) fList.nextPtr = TIN_NULL_PTR ;
       if( bcdtmStream_fwrite(&fList,sizeof(DTM_FEATURE_LIST_VER200),1,dtmStreamP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing Dtm Feature List") ;
          goto errexit ; 
         }
      }
   }
/*
** Report File Position
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"File Position After Geopak Tin Object Write = %10ld",bcdtmStream_ftell(dtmStreamP)) ;
/*
** Clean Up
*/
 cleanup :
 if( featureMapP != NULL ) { free(featureMapP) ; featureMapP = NULL ; }
/*
** Job Completed
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Tin Object From Dtm Object At File Position Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Tin Object From Dtm Object At File Position Error") ;
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
BENTLEYDTM_EXPORT int bcdtmWrite_checkDtmFeatureFile
( 
 WCharCP dtmFileNameP                                 //  Dtm Feature File Name
)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    numPoints=0,numFeatures=0 ;
 Int64 featureFileOffset ;
 BC_DTM_FEATURE_HEADER dtmHeader ;
 BC_DTM_FEATURE_RECORD dtmFeature ;
 FILE  *dtmFP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature File") ;
    bcdtmWrite_message(0,0,0,"dtmFileNameP = %s",dtmFileNameP) ;
   }
/*
** Open File
*/
 dtmFP = bcdtmFile_open(dtmFileNameP,L"rb") ;
 if( dtmFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Error Opening DTM Feature File") ;
    goto errexit ;
   }
/*
** Read DTM Feature File Header
*/
 if( fread(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Failed To Read DTM Feature File Header") ;
    goto errexit ;
   }
/*
** Check For bcLIB DTM Feature File
*/
 if( dtmHeader.dtmFileType != BC_DTMFeatureType )
   {
    bcdtmWrite_message(0,0,0,"Not A Bentley Civil DTM File Feature File") ;
    goto errexit ;
   } 
/*
** Check Version Number
*/
 if( dtmHeader.dtmVersionNumber != BC_DTM_FEATURE_VERSION )
   {
    bcdtmWrite_message(0,0,0,"Incorrect Version Number For The Bentley Civil DTM Feature File") ;
    goto errexit ;
   }
/*
**  Write File Header Variables
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"DTM Feature File Header Variables") ;
    bcdtmWrite_message(0,0,0,"**** dtmFileType        = %p",dtmHeader.dtmFileType)  ;
    bcdtmWrite_message(0,0,0,"**** dtmVersionNumber   = %10ld",dtmHeader.dtmVersionNumber)  ;
    bcdtmWrite_message(0,0,0,"**** numPoints          = %10ld",dtmHeader.numPoints)  ;
    bcdtmWrite_message(0,0,0,"**** numFeatures        = %10ld",dtmHeader.numFeatures)  ;
    bcdtmWrite_message(0,0,0,"**** featureFileOffset  = %10I64d",dtmHeader.featureFileOffset)  ;
    bcdtmWrite_message(0,0,0,"**** xMin               = %12.5lf",dtmHeader.xMin)  ;
    bcdtmWrite_message(0,0,0,"**** xMax               = %12.5lf",dtmHeader.xMax)  ;
    bcdtmWrite_message(0,0,0,"**** yMin               = %12.5lf",dtmHeader.yMin)  ;
    bcdtmWrite_message(0,0,0,"**** yMax               = %12.5lf",dtmHeader.yMax)  ;
    bcdtmWrite_message(0,0,0,"**** zMin               = %12.5lf",dtmHeader.zMin)  ;
    bcdtmWrite_message(0,0,0,"**** zMax               = %12.5lf",dtmHeader.zMax)  ;
   } 
/*
** Scan DTM Feature File And Count Points And DTM Features
*/
  featureFileOffset = dtmHeader.featureFileOffset ;
  _fseeki64(dtmFP,featureFileOffset,SEEK_SET) ;
  while( fread( &dtmFeature,sizeof(BC_DTM_FEATURE_RECORD),1,dtmFP) == 1 )
    {
     if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmFeatureType = %4ld numFeaturePts = %8ld",dtmFeature.dtmFeatureType,dtmFeature.numFeaturePoints) ; 
     ++numFeatures ;
     numPoints = numPoints + dtmFeature.numFeaturePoints ;
     featureFileOffset = featureFileOffset + sizeof(BC_DTM_FEATURE_RECORD) + dtmFeature.numFeaturePoints * sizeof(DPoint3d) ;
     _fseeki64(dtmFP,featureFileOffset,SEEK_SET) ;
    }
/*
** Write Stats
*/
  if( dbg )
    {
     bcdtmWrite_message(0,0,0,"Number Of Points   In DTM File = %10ld",numPoints) ;
     bcdtmWrite_message(0,0,0,"Number Of Features In DTM File = %10ld",numFeatures) ;
    }
/*
** check Consistency Dtm Feature File
*/
 if( numPoints !=  dtmHeader.numPoints )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Points In DTM Feature File") ;
    ret = DTM_ERROR ; 
   }
 if( numFeatures !=  dtmHeader.numFeatures )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Features In DTM Feature File") ;
    ret = DTM_ERROR ; 
   }
/*
** Clean Up
*/
 cleanup :
 if( dtmFP != NULL ) { fclose(dtmFP) ; dtmFP = NULL ; }
/*
** Job Completed
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature File Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature File Error") ;
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
BENTLEYDTM_EXPORT int bcdtmWrite_xyzFileToDtmFeatureFile
( 
 WCharCP xyzFileNameP,                                //  XYZ File Name
 WCharCP dtmFileNameP                                 //  Dtm Feature File Name
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  fileType,numFileRecs,maxPointSize=10000000 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing XYZ File To Dtm Feature File") ;
/*
** Auto Detect XYZ File Type
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Detecting XYZ File Type") ;
 if( bcdtmUtl_detectXYZFileType(xyzFileNameP,&fileType,&numFileRecs) != DTM_SUCCESS ) goto errexit ;
 if( dbg )
   { 
    if( fileType == 1 ) bcdtmWrite_message(0,0,0,"ASCII XYZ File Detected") ;
    if( fileType == 2 ) bcdtmWrite_message(0,0,0,"Binary XYZ File Detected") ;
    bcdtmWrite_message(0,0,0,"Estimated XYZ Record Entries = %6ld",numFileRecs) ;
   } 
/*
** Write To Dtm Feature File
*/
 if( fileType == 1 ) 
   { 
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading ASCII XYZ File") ;
    if( bcdtmWrite_xyzASCIIFileToDtmFeatureFile(xyzFileNameP,dtmFileNameP,maxPointSize)) goto errexit ;
   }
 if( fileType == 2 ) 
   { 
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Binary XYZ File") ;
    if( bcdtmWrite_xyzBinaryFileToDtmFeatureFile(xyzFileNameP,dtmFileNameP,maxPointSize)) goto errexit ; 
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing XYZ File To Dtm Feature File Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing XYZ File To Dtm Feature File Error") ;
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
BENTLEYDTM_Private int bcdtmWrite_xyzASCIIFileToDtmFeatureFile
(
 WCharCP xyzFileNameP,                             //  XYZ File Name
 WCharCP dtmFileNameP,                             //  Dtm Feature File Name
 long  maxPointSize                                 //  maximum Point Size For feature Records
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   point,firstPoint=1 ;
 double x,y,z ;
 char   inputBuffer[512],*bp,*bt ;
 DPoint3d    dtmPoint ;
 BC_DTM_OBJ *dtmP=NULL ;
 FILE  *xyzFP=NULL ;
 FILE  *dtmFP=NULL ;
 BC_DTM_FEATURE_HEADER dtmHeader ;
 BC_DTM_FEATURE_RECORD dtmFeature ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Writing ASCII XYZ File To Dtm Feature File Completed") ;
    bcdtmWrite_message(0,0,0,"xyzFileNameP    = %s",*xyzFileNameP) ;
    bcdtmWrite_message(0,0,0,"dtmFileNameP    = %s",*dtmFileNameP) ;
    bcdtmWrite_message(0,0,0,"maxPointSize    = %ld",maxPointSize) ;
   } 
/*
** Open XYZ File
*/
 if( ( xyzFP = bcdtmFile_open(xyzFileNameP,L"r")) == NULL )
   { 
    bcdtmWrite_message(1,0,0,"Error Opening XYZ File %s",xyzFileNameP) ;
    goto errexit ; 
   }
/*
** Open DTM Feature File
*/
 if( ( dtmFP = bcdtmFile_open(dtmFileNameP,L"wb+")) == NULL )
   { 
    bcdtmWrite_message(1,0,0,"Error Opening DTM Feature File %s",dtmFileNameP) ;
    goto errexit ; 
   }
/*
**  Initialise DTM Feature Record
*/
 dtmFeature.dtmFeatureType = DTMFeatureType::RandomSpots ;
 dtmFeature.dtmUserTag     = DTM_NULL_USER_TAG ;
 dtmFeature.dtmFeatureId   = DTM_NULL_FEATURE_ID ;
/*
**  Initialise DTM Header
*/
 dtmHeader.dtmFileType       = BC_DTMFeatureType ;
 dtmHeader.dtmVersionNumber  = BC_DTM_FEATURE_VERSION ;
 dtmHeader.numPoints         = 0 ;
 dtmHeader.numFeatures       = 0 ;
 dtmHeader.xMin              = 0.0 ;
 dtmHeader.yMin              = 0.0 ;
 dtmHeader.zMin              = 0.0 ;
 dtmHeader.xMax              = 0.0 ;
 dtmHeader.yMax              = 0.0 ;
 dtmHeader.zMax              = 0.0 ;
 dtmHeader.featureFileOffset = 0   ;
/*
** Write header
*/
 if( fwrite(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
    goto errexit ;
   }
/*
**  set Feature File offset
*/
 dtmHeader.featureFileOffset = _ftelli64(dtmFP) ;
/*
** Create DTm Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
/*
** Read ASCII XYZ File
*/
 bp = inputBuffer ; *bp = 0 ; bt = inputBuffer + 510 ;
 while( fscanf(xyzFP,"%c",bp) != EOF )
   {
    if( *bp && *bp != 10 && *bp != 13) ++bp ;
    else
      {
       *bp = 0 ;
       if( ! bcdtmUtl_decodeXYZRecord(inputBuffer,&x,&y,&z) )
         {
          if( firstPoint )
            {
             firstPoint      = 0 ; 
             dtmHeader.xMin = dtmHeader.xMax = x ;
             dtmHeader.yMin = dtmHeader.yMax = y ;
             dtmHeader.zMin = dtmHeader.zMax = z ;
            } 
          else
            {
             if( x < dtmHeader.xMin ) dtmHeader.xMin = x ;
             if( x > dtmHeader.xMax ) dtmHeader.xMax = x ;
             if( y < dtmHeader.yMin ) dtmHeader.yMin = y ;
             if( y > dtmHeader.yMax ) dtmHeader.yMax = y ;
             if( z < dtmHeader.zMin ) dtmHeader.zMin = z ;
             if( z > dtmHeader.zMax ) dtmHeader.zMax = z ;
            }
          dtmPoint.x = x ;
          dtmPoint.y = y ;
          dtmPoint.z = z ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ;
          if( dtmP->numPoints >= maxPointSize )
            {
/*
**           Write DTM Feature Record
*/
             ++dtmHeader.numFeatures ;
             dtmHeader.numPoints = dtmHeader.numPoints + dtmP->numPoints ;  
             dtmFeature.numFeaturePoints = dtmP->numPoints ;
             if( fwrite(&dtmFeature,sizeof(BC_DTM_FEATURE_RECORD),1,dtmFP) != 1 )
               {
                bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
                goto errexit ;
               }
/*
**           Write DTM Feature Points
*/
             for( point = 0 ; point < dtmP->numPoints ; ++point )
               {
                if( fwrite(pointAddrP(dtmP,point),sizeof(DPoint3d),1,dtmFP) != 1 )
                  {
                   bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
                   goto errexit ;
                  }
               }
             dtmP->numPoints = 0 ;
            }
         }
       bp = inputBuffer ; *bp = 0 ;
      }
    if( bp > bt ) bp = bt ;
   }
/*
** Write Remaining Points
*/
 if( dtmP->numPoints > 0 )
   {
/*
**  Write DTM Feature Record
*/
    ++dtmHeader.numFeatures ;
    dtmHeader.numPoints = dtmHeader.numPoints + dtmP->numPoints ;  
    dtmFeature.numFeaturePoints = dtmP->numPoints ;
    if( fwrite(&dtmFeature,sizeof(BC_DTM_FEATURE_RECORD),1,dtmFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
       goto errexit ;
      }
/*
**  Write DTM Feature Points
*/
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       if( fwrite(pointAddrP(dtmP,point),sizeof(DPoint3d),1,dtmFP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
          goto errexit ;
         }
      }
     dtmP->numPoints = 0 ;
    }
/*
** Rewite Header
*/
 _fseeki64(dtmFP,0,SEEK_SET) ;
 if( fwrite(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( xyzFP != NULL ) fclose(xyzFP) ;
 if( dtmFP != NULL ) fclose(dtmFP) ;
 if( dtmP  != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
**  Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing ASCII XYZ File To Dtm Feature File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing ASCII XYZ File To Dtm Feature File Error") ;
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
BENTLEYDTM_Private int bcdtmWrite_xyzBinaryFileToDtmFeatureFile
(
 WCharCP xyzFileNameP,                                //  XYZ File Name
 WCharCP dtmFileNameP,                                //  Dtm Feature File Name
 long  maxPointSize                                 //  maximum Point Size For feature Records
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   point,firstPoint=1 ;
 DPoint3d    dtmPoint ;
 BC_DTM_OBJ *dtmP=NULL ;
 FILE  *xyzFP=NULL ;
 FILE  *dtmFP=NULL ;
 BC_DTM_FEATURE_HEADER dtmHeader ;
 BC_DTM_FEATURE_RECORD dtmFeature ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Writing Binary XYZ File To Dtm Feature File") ;
    bcdtmWrite_message(0,0,0,"xyzFileNameP    = %s",xyzFileNameP) ;
    bcdtmWrite_message(0,0,0,"dtmFileNameP    = %s",dtmFileNameP) ;
    bcdtmWrite_message(0,0,0,"maxPointSize    = %ld",maxPointSize) ;
   } 
/*
** Open XYZ File
*/
 if( ( xyzFP = bcdtmFile_open(xyzFileNameP,L"rb")) == NULL )
   { 
    bcdtmWrite_message(1,0,0,"Error Opening XYZ File %s",xyzFileNameP) ;
    goto errexit ; 
   }
/*
** Open DTM Feature File
*/
 if( ( dtmFP = bcdtmFile_open(dtmFileNameP,L"wb+")) == NULL )
   { 
    bcdtmWrite_message(1,0,0,"Error Opening DTM Feature File %s",dtmFileNameP) ;
    goto errexit ; 
   }
/*
**  Initialise DTM Feature Record
*/
 dtmFeature.dtmFeatureType = DTMFeatureType::RandomSpots ;
 dtmFeature.dtmUserTag     = DTM_NULL_USER_TAG ;
 dtmFeature.dtmFeatureId   = DTM_NULL_FEATURE_ID ;
/*
**  Initialise DTM Header
*/
 dtmHeader.dtmFileType       = BC_DTMFeatureType ;
 dtmHeader.dtmVersionNumber  = BC_DTM_FEATURE_VERSION ;
 dtmHeader.numPoints         = 0 ;
 dtmHeader.numFeatures       = 0 ;
 dtmHeader.xMin              = 0.0 ;
 dtmHeader.yMin              = 0.0 ;
 dtmHeader.zMin              = 0.0 ;
 dtmHeader.xMax              = 0.0 ;
 dtmHeader.yMax              = 0.0 ;
 dtmHeader.zMax              = 0.0 ;
 dtmHeader.featureFileOffset = 0   ;
/*
** Write header
*/
 if( fwrite(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
    goto errexit ;
   }
/*
**  set Feature File offset
*/
 dtmHeader.featureFileOffset = _ftelli64(dtmFP) ;
/*
** Create DTm Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
/*
** Read Binary XYZ File
*/
 while( fread(&dtmPoint,sizeof(DPoint3d),1,xyzFP) == 1  )
   {
/*
**  Set Bounding Cube
*/
    if( firstPoint )
      {
       firstPoint = 0 ; 
       dtmHeader.xMin = dtmHeader.xMax = dtmPoint.x ;
       dtmHeader.yMin = dtmHeader.yMax = dtmPoint.y ;
       dtmHeader.zMin = dtmHeader.zMax = dtmPoint.z ;
      } 
    else
      {
       if( dtmPoint.x < dtmHeader.xMin ) dtmHeader.xMin = dtmPoint.x ;
       if( dtmPoint.x > dtmHeader.xMax ) dtmHeader.xMax = dtmPoint.x ;
       if( dtmPoint.y < dtmHeader.yMin ) dtmHeader.yMin = dtmPoint.y ;
       if( dtmPoint.y > dtmHeader.yMax ) dtmHeader.yMax = dtmPoint.y ;
       if( dtmPoint.z < dtmHeader.zMin ) dtmHeader.zMin = dtmPoint.z ;
       if( dtmPoint.z > dtmHeader.zMax ) dtmHeader.zMax = dtmPoint.z ;
      }
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ;
    if( dtmP->numPoints >= maxPointSize )
      {
/*
**     Write DTM Feature Record
*/
       ++dtmHeader.numFeatures ;
       dtmHeader.numPoints = dtmHeader.numPoints + dtmP->numPoints ;  
       dtmFeature.numFeaturePoints = dtmP->numPoints ;
       if( fwrite(&dtmFeature,sizeof(BC_DTM_FEATURE_RECORD),1,dtmFP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
          goto errexit ;
         }
/*
**     Write DTM Feature Points
*/
       for( point = 0 ; point < dtmP->numPoints ; ++point )
         {
          if( fwrite(pointAddrP(dtmP,point),sizeof(DPoint3d),1,dtmFP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
             goto errexit ;
            }
         }  
       dtmP->numPoints = 0 ;
      }
   }
/*
** Write Remaining Points
*/
 if( dtmP->numPoints > 0 )
   {
/*
**  Write DTM Feature Record
*/
    ++dtmHeader.numFeatures ;
    dtmHeader.numPoints = dtmHeader.numPoints + dtmP->numPoints ;  
    dtmFeature.numFeaturePoints = dtmP->numPoints ;
    if( fwrite(&dtmFeature,sizeof(BC_DTM_FEATURE_RECORD),1,dtmFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
       goto errexit ;
      }
/*
**  Write DTM Feature Points
*/
    for( point = 0 ; point < dtmP->numPoints ; ++point )
      {
       if( fwrite(pointAddrP(dtmP,point),sizeof(DPoint3d),1,dtmFP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
          goto errexit ;
         }
      }
     dtmP->numPoints = 0 ;
    }
/*
** Rewite Header
*/
 _fseeki64(dtmFP,0,SEEK_SET) ;
 if( fwrite(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( xyzFP != NULL ) fclose(xyzFP) ;
 if( dtmFP != NULL ) fclose(dtmFP) ;
 if( dtmP  != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
**  Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Binary XYZ File To Dtm Feature File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Binary XYZ File To Dtm Feature File Error") ;
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
BENTLEYDTM_Private int  bcdtmWrite_loadDtmFeatureFunction(DTMFeatureType dtmFeatureType,DTMUserTag dtmUserTag,DTMFeatureId dtmFeatureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP)
/*
** Writes The DTM Feature To The DTM Feature File
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DPoint3d  *p3dP ;
 BC_DTM_FEATURE_RECORD dtmFeatureRecord ;
 struct DtmFeatureLoad 
 {
   FILE *dtmFP ; 
   long numPoints , numFeatures ;
   double xMin,yMin,zMin,xMax,yMax,zMax ;
 } *dtmFeatureLoadP   ;
/*
** Write Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeatureType = %4ld ** numFeaturePoints = %8ld",dtmFeatureType,numFeaturePts) ;
/*
** Set Points 
*/
 dtmFeatureLoadP = ( struct DtmFeatureLoad *) userP ;
/*
** Scan Points To Get Bounding Rectangle
*/
 if( dtmFeatureLoadP->numPoints == 0 )
   {
    dtmFeatureLoadP->xMin = dtmFeatureLoadP->xMax = featurePtsP->x ;
    dtmFeatureLoadP->yMin = dtmFeatureLoadP->yMax = featurePtsP->y ;
    dtmFeatureLoadP->zMin = dtmFeatureLoadP->zMax = featurePtsP->z ;
   } 
 for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
   {
    if( p3dP->x < dtmFeatureLoadP->xMin ) dtmFeatureLoadP->xMin = p3dP->x ;
    if( p3dP->x > dtmFeatureLoadP->xMax ) dtmFeatureLoadP->xMax = p3dP->x ;
    if( p3dP->y < dtmFeatureLoadP->yMin ) dtmFeatureLoadP->yMin = p3dP->y ;
    if( p3dP->y > dtmFeatureLoadP->yMax ) dtmFeatureLoadP->yMax = p3dP->y ;
    if( p3dP->z < dtmFeatureLoadP->zMin ) dtmFeatureLoadP->zMin = p3dP->z ;
    if( p3dP->z > dtmFeatureLoadP->zMax ) dtmFeatureLoadP->zMax = p3dP->z ;
   }
/*
** Update Records
*/
 ++dtmFeatureLoadP->numFeatures ;
 dtmFeatureLoadP->numPoints = dtmFeatureLoadP->numPoints + (long)numFeaturePts ;
/*
** Write Dtm Feature Record To Dtm Feature File
*/
 dtmFeatureRecord.dtmFeatureType   = dtmFeatureType ;
 dtmFeatureRecord.dtmUserTag       = dtmUserTag ;
 dtmFeatureRecord.dtmFeatureId     = dtmFeatureId ;
 dtmFeatureRecord.numFeaturePoints = (long)numFeaturePts ;
 if( fwrite(&dtmFeatureRecord,sizeof(BC_DTM_FEATURE_RECORD),1,dtmFeatureLoadP->dtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Writing Dtm Feature Record") ;
    goto errexit ;
   } 
/*
** Write Dtm Feature Points To Dtm Feature File
*/
 if( fwrite(featurePtsP,sizeof(DPoint3d)*numFeaturePts,1,dtmFeatureLoadP->dtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Writing Dtm Feature Record") ;
    goto errexit ;
   } 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Load Function Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Load Function Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmWrite_message(0,0,0,"Error Exiting From Load Function") ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmWrite_dtmFeatureTypeToDtmFeatureFileDtmFile
( 
 WCharCP dtmFileNameP,                                // ==> Dtm File Name
 WCharCP dtmFeatureFileNameP,                         // ==> Dtm Feature File Name
 DTMFeatureType dtmFeatureType,                               // ==> Dtm Feature Type
 long fileOption                                    // ==> File Option <1=Create,2=Append>
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=NULL ; 
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm File To Dtm Feature File") ;
    bcdtmWrite_message(0,0,0,"dtmFileNameP         = %s",dtmFileNameP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureFileNameP  = %s",dtmFeatureFileNameP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType       = %4ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"fileOption           = %4ld",fileOption) ;
   } 
/*
** Read DTM File
*/
 if( bcdtmRead_fromFileDtmObject(&dtmP,dtmFileNameP)) goto errexit ;
/*
** Call Object Version
*/ 
 if( bcdtmWrite_dtmFeatureTypeToDtmFeatureFileDtmObject(dtmP,dtmFeatureFileNameP,dtmFeatureType,fileOption)) goto errexit ;  
/*
** Clean Up
*/
 cleanup :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Job Completed
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm File To Dtm Feature File Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm File To Dtm Feature File Error") ;
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
BENTLEYDTM_EXPORT int bcdtmWrite_dtmFeatureTypeToDtmFeatureFileDtmObject
( 
 BC_DTM_OBJ *dtmP,                                  // ==> Dtm Object
 WCharCP dtmFeatureFileNameP,                      // ==> Dtm Feature File Name
 DTMFeatureType dtmFeatureType,                               // ==> Dtm Feature Type
 long fileOption                                    // ==> File Option <1=Create,2=Append>
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 FILE *dtmFP=NULL ;
 BC_DTM_FEATURE_HEADER dtmHeader ;
 struct DtmFeatureLoad 
 {
   FILE *dtmFP ; 
   long numPoints , numFeatures ;
   double xMin,yMin,zMin,xMax,yMax,zMax ;
 } dtmFeatureLoad   ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm Object To Dtm Feature File") ;
    bcdtmWrite_message(0,0,0,"dtmP                 = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureFileNameP  = %s",dtmFeatureFileNameP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType       = %4ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"fileOption           = %4ld",fileOption) ;
   } 
/*
** Check File Option
*/
 if( fileOption != 1 && fileOption != 2 )
   {
    bcdtmWrite_message(1,0,0,"Invalid File Open Option") ;
    goto errexit ;
   } 
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test For Valid Dtm Export Feature Type
*/
 bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object Feature Type") ;
 if( bcdtmData_testForValidDtmObjectExportFeatureType(dtmFeatureType) == DTM_ERROR  )
   { 
    bcdtmWrite_message(2,0,0,"Invalid Dtm Feature Type %4ld",dtmFeatureType) ;
    goto errexit ; 
   } 
/*
** Open New Dtm Feature File
*/
 if( fileOption == 1 ) 
   {
    if( ( dtmFP = bcdtmFile_open(dtmFeatureFileNameP,L"wb+")) == NULL )
      { 
       bcdtmWrite_message(1,0,0,"Error Opening DTM Feature File %s",dtmFeatureFileNameP) ;
       goto errexit ; 
      }
/*
**  Initialise DTM Header
*/
    dtmHeader.dtmFileType       = BC_DTMFeatureType ;
    dtmHeader.dtmVersionNumber  = BC_DTM_FEATURE_VERSION ;
    dtmHeader.numPoints         = 0 ;
    dtmHeader.numFeatures       = 0 ;
    dtmHeader.xMin              = 0.0 ;
    dtmHeader.yMin              = 0.0 ;
    dtmHeader.zMin              = 0.0 ;
    dtmHeader.xMax              = 0.0 ;
    dtmHeader.yMax              = 0.0 ;
    dtmHeader.zMax              = 0.0 ;
    dtmHeader.featureFileOffset = 0   ;
/*
**  Write header
*/
    if( fwrite(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
       goto errexit ;
      }
/*
**  set Feature File offset
*/
    dtmHeader.featureFileOffset = _ftelli64(dtmFP) ;
   }
/*
** Open Existing Dtm Feature File
*/
 if( fileOption == 2 ) 
   {
/*
**  Open File
*/
    dtmFP = bcdtmFile_open(dtmFeatureFileNameP,L"rb+") ;
    if( dtmFP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Error Opening DTM Feature File") ;
       goto errexit ;
      }
/*
**  Read DTM Feature File Header
*/
    if( fread(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Failed To Read DTM Feature File Header") ;
       goto errexit ;
      }
/*
**  Check For bcLIB DTM Feature File
*/
    if( dtmHeader.dtmFileType != BC_DTMFeatureType )
      {
       bcdtmWrite_message(0,0,0,"Not A Bentley Civil DTM File Feature File") ;
       goto errexit ;
      } 
/*
**  Check Version Number
*/
    if( dtmHeader.dtmVersionNumber != BC_DTM_FEATURE_VERSION )
      {
       bcdtmWrite_message(0,0,0,"Incorrect Version Number For The Bentley Civil DTM Feature File") ;
       goto errexit ;
      }
/*
**  Write File Header Variables
*/
    if( dbg )
      { 
       bcdtmWrite_message(0,0,0,"DTM Feature File Header Variables") ;
       bcdtmWrite_message(0,0,0,"**** dtmFileType        = %p",dtmHeader.dtmFileType)  ;
       bcdtmWrite_message(0,0,0,"**** dtmVersionNumber   = %10ld",dtmHeader.dtmVersionNumber)  ;
       bcdtmWrite_message(0,0,0,"**** numPoints          = %10ld",dtmHeader.numPoints)  ;
       bcdtmWrite_message(0,0,0,"**** numFeatures        = %10ld",dtmHeader.numFeatures)  ;
       bcdtmWrite_message(0,0,0,"**** featureFileOffset  = %10I64d",dtmHeader.featureFileOffset)  ;
       bcdtmWrite_message(0,0,0,"**** xMin               = %12.5lf",dtmHeader.xMin)  ;
       bcdtmWrite_message(0,0,0,"**** xMax               = %12.5lf",dtmHeader.xMax)  ;
       bcdtmWrite_message(0,0,0,"**** yMin               = %12.5lf",dtmHeader.yMin)  ;
       bcdtmWrite_message(0,0,0,"**** yMax               = %12.5lf",dtmHeader.yMax)  ;
       bcdtmWrite_message(0,0,0,"**** zMin               = %12.5lf",dtmHeader.zMin)  ;
       bcdtmWrite_message(0,0,0,"**** zMax               = %12.5lf",dtmHeader.zMax)  ;
      } 
/*
**  Go To End Of DTM Feature File
*/
    _fseeki64(dtmFP,0,SEEK_END) ;
   }
/*
** Set Up Scan Structure
*/
 dtmFeatureLoad.dtmFP       = dtmFP ;
 dtmFeatureLoad.numPoints   = dtmHeader.numPoints ;
 dtmFeatureLoad.numFeatures = dtmHeader.numFeatures ;
 dtmFeatureLoad.xMin        = dtmHeader.xMin ;
 dtmFeatureLoad.yMin        = dtmHeader.yMin ;
 dtmFeatureLoad.zMin        = dtmHeader.zMin ;
 dtmFeatureLoad.xMax        = dtmHeader.xMax ;
 dtmFeatureLoad.yMax        = dtmHeader.yMax ;
 dtmFeatureLoad.zMax        = dtmHeader.zMax ;
/*
** Scan DTM And Write DtmFeature Type
*/
 if( bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject(dtmP,dtmFeatureType,500000,bcdtmWrite_loadDtmFeatureFunction,0,DTMFenceType::Block,DTMFenceOption::Overlap,NULL,0,&dtmFeatureLoad)) goto errexit ;
/*
** Update Header Items
*/
 dtmHeader.numPoints         = dtmFeatureLoad.numPoints ;
 dtmHeader.numFeatures       = dtmFeatureLoad.numFeatures ;
 dtmHeader.xMin              = dtmFeatureLoad.xMin ;
 dtmHeader.yMin              = dtmFeatureLoad.yMin ;
 dtmHeader.zMin              = dtmFeatureLoad.zMin ;
 dtmHeader.xMax              = dtmFeatureLoad.xMax  ;
 dtmHeader.yMax              = dtmFeatureLoad.yMax ;
 dtmHeader.zMax              = dtmFeatureLoad.zMax ;
/*
**  Write File Header Variables
*/
 if( dbg == 1 )
   { 
    bcdtmWrite_message(0,0,0,"DTM Feature File Header Variables") ;
    bcdtmWrite_message(0,0,0,"**** dtmFileType        = %p",dtmHeader.dtmFileType)  ;
    bcdtmWrite_message(0,0,0,"**** dtmVersionNumber   = %10ld",dtmHeader.dtmVersionNumber)  ;
    bcdtmWrite_message(0,0,0,"**** numPoints          = %10ld",dtmHeader.numPoints)  ;
    bcdtmWrite_message(0,0,0,"**** numFeatures        = %10ld",dtmHeader.numFeatures)  ;
    bcdtmWrite_message(0,0,0,"**** featureFileOffset  = %10I64d",dtmHeader.featureFileOffset)  ;
    bcdtmWrite_message(0,0,0,"**** xMin               = %12.5lf",dtmHeader.xMin)  ;
    bcdtmWrite_message(0,0,0,"**** xMax               = %12.5lf",dtmHeader.xMax)  ;
    bcdtmWrite_message(0,0,0,"**** yMin               = %12.5lf",dtmHeader.yMin)  ;
    bcdtmWrite_message(0,0,0,"**** yMax               = %12.5lf",dtmHeader.yMax)  ;
    bcdtmWrite_message(0,0,0,"**** zMin               = %12.5lf",dtmHeader.zMin)  ;
    bcdtmWrite_message(0,0,0,"**** zMax               = %12.5lf",dtmHeader.zMax)  ;
   } 
/*
** Rewite Header
*/
 _fseeki64(dtmFP,0,SEEK_SET) ;
 if( fwrite(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dtmFP != NULL ) fclose(dtmFP) ;
/*
** Job Completed
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm Object To Dtm Feature File Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm Object To Dtm Feature File Error") ;
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
BENTLEYDTM_EXPORT int bcdtmWrite_contoursToDtmFeatureFileDtmFile
( 
 WCharCP dtmFileNameP,                                // ==> Dtm File Name
 WCharCP dtmFeatureFileNameP,                         // ==> Dtm Feature File Name
 long fileOption,                                   // ==> File Option <1=Create,2=Append>
 double contourInterval                             // ==> Contour Interval
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=NULL ; 
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm File To Dtm Feature File") ;
    bcdtmWrite_message(0,0,0,"dtmFileNameP         = %s",dtmFileNameP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureFileNameP  = %s",dtmFeatureFileNameP) ;
    bcdtmWrite_message(0,0,0,"fileOption           = %4ld",fileOption) ;
    bcdtmWrite_message(0,0,0,"contourInterval      = %8.3lf",contourInterval) ;
   } 
/*
** Read DTM File
*/
 if( bcdtmRead_fromFileDtmObject(&dtmP,dtmFileNameP)) goto errexit ;
/*
** Call Object Version
*/ 
 if( bcdtmWrite_contoursToDtmFeatureFileDtmObject(dtmP,dtmFeatureFileNameP,fileOption,contourInterval)) goto errexit ;  
/*
** Clean Up
*/
 cleanup :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Job Completed
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm File To Dtm Feature File Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm File To Dtm Feature File Error") ;
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
BENTLEYDTM_EXPORT int bcdtmWrite_contoursToDtmFeatureFileDtmObject
( 
 BC_DTM_OBJ *dtmP,                                  // ==> Dtm Object
 WCharCP dtmFeatureFileNameP,                      // ==> Dtm Feature File Name
 long fileOption,                                   // ==> File Option <1=Create,2=Append>
 double contourInterval                             // ==> Contour Interval
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 FILE *dtmFP=NULL ;
 BC_DTM_FEATURE_HEADER dtmHeader ;
 struct DtmFeatureLoad 
 {
   FILE *dtmFP ; 
   long numPoints , numFeatures ;
   double xMin,yMin,zMin,xMax,yMax,zMax ;
 } dtmFeatureLoad   ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm Object To Dtm Feature File") ;
    bcdtmWrite_message(0,0,0,"dtmP                 = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureFileNameP  = %s",dtmFeatureFileNameP) ;
    bcdtmWrite_message(0,0,0,"fileOption           = %4ld",fileOption) ;
    bcdtmWrite_message(0,0,0,"contourInterval      = %8.3lf",contourInterval) ;
   } 
/*
** Check File Option
*/
 if( fileOption != 1 && fileOption != 2 )
   {
    bcdtmWrite_message(1,0,0,"Invalid File Open Option") ;
    goto errexit ;
   } 
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test For Tin State
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Open New Dtm Feature File
*/
 if( fileOption == 1 ) 
   {
    if( ( dtmFP = bcdtmFile_open(dtmFeatureFileNameP,L"wb+")) == NULL )
      { 
       bcdtmWrite_message(1,0,0,"Error Opening DTM Feature File %s",dtmFeatureFileNameP) ;
       goto errexit ; 
      }
/*
**  Initialise DTM Header
*/
    dtmHeader.dtmFileType       = BC_DTMFeatureType ;
    dtmHeader.dtmVersionNumber  = BC_DTM_FEATURE_VERSION ;
    dtmHeader.numPoints         = 0 ;
    dtmHeader.numFeatures       = 0 ;
    dtmHeader.xMin              = 0.0 ;
    dtmHeader.yMin              = 0.0 ;
    dtmHeader.zMin              = 0.0 ;
    dtmHeader.xMax              = 0.0 ;
    dtmHeader.yMax              = 0.0 ;
    dtmHeader.zMax              = 0.0 ;
    dtmHeader.featureFileOffset = 0   ;
/*
**  Write header
*/
    if( fwrite(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
       goto errexit ;
      }
/*
**  set Feature File offset
*/
    dtmHeader.featureFileOffset = _ftelli64(dtmFP) ;
   }
/*
** Open Existing Dtm Feature File
*/
 if( fileOption == 2 ) 
   {
/*
**  Open File
*/
    dtmFP = bcdtmFile_open(dtmFeatureFileNameP,L"rb+") ;
    if( dtmFP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Error Opening DTM Feature File") ;
       goto errexit ;
      }
/*
**  Read DTM Feature File Header
*/
    if( fread(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Failed To Read DTM Feature File Header") ;
       goto errexit ;
      }
/*
**  Check For bcLIB DTM Feature File
*/
    if( dtmHeader.dtmFileType != BC_DTMFeatureType )
      {
       bcdtmWrite_message(0,0,0,"Not A Bentley Civil DTM File Feature File") ;
       goto errexit ;
      } 
/*
**  Check Version Number
*/
    if( dtmHeader.dtmVersionNumber != BC_DTM_FEATURE_VERSION )
      {
       bcdtmWrite_message(0,0,0,"Incorrect Version Number For The Bentley Civil DTM Feature File") ;
       goto errexit ;
      }
/*
**  Write File Header Variables
*/
    if( dbg )
      { 
       bcdtmWrite_message(0,0,0,"DTM Feature File Header Variables") ;
       bcdtmWrite_message(0,0,0,"**** dtmFileType        = %p",dtmHeader.dtmFileType)  ;
       bcdtmWrite_message(0,0,0,"**** dtmVersionNumber   = %10ld",dtmHeader.dtmVersionNumber)  ;
       bcdtmWrite_message(0,0,0,"**** numPoints          = %10ld",dtmHeader.numPoints)  ;
       bcdtmWrite_message(0,0,0,"**** numFeatures        = %10ld",dtmHeader.numFeatures)  ;
       bcdtmWrite_message(0,0,0,"**** featureFileOffset  = %10I64d",dtmHeader.featureFileOffset)  ;
       bcdtmWrite_message(0,0,0,"**** xMin               = %12.5lf",dtmHeader.xMin)  ;
       bcdtmWrite_message(0,0,0,"**** xMax               = %12.5lf",dtmHeader.xMax)  ;
       bcdtmWrite_message(0,0,0,"**** yMin               = %12.5lf",dtmHeader.yMin)  ;
       bcdtmWrite_message(0,0,0,"**** yMax               = %12.5lf",dtmHeader.yMax)  ;
       bcdtmWrite_message(0,0,0,"**** zMin               = %12.5lf",dtmHeader.zMin)  ;
       bcdtmWrite_message(0,0,0,"**** zMax               = %12.5lf",dtmHeader.zMax)  ;
      } 
/*
**  Go To End Of DTM Feature File
*/
    _fseeki64(dtmFP,0,SEEK_END) ;
   }
/*
** Set Up Scan Structure
*/
 dtmFeatureLoad.dtmFP       = dtmFP ;
 dtmFeatureLoad.numPoints   = dtmHeader.numPoints ;
 dtmFeatureLoad.numFeatures = dtmHeader.numFeatures ;
 dtmFeatureLoad.xMin        = dtmHeader.xMin ;
 dtmFeatureLoad.yMin        = dtmHeader.yMin ;
 dtmFeatureLoad.zMin        = dtmHeader.zMin ;
 dtmFeatureLoad.xMax        = dtmHeader.xMax ;
 dtmFeatureLoad.yMax        = dtmHeader.yMax ;
 dtmFeatureLoad.zMax        = dtmHeader.zMax ;
/*
** Load Contours
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Contours To Dtm Feature File") ;
 if( bcdtmLoad_contoursFromDtmObject(dtmP,contourInterval,0.0,TRUE,dtmP->zMin,dtmP->zMax,FALSE,NULL,0,DTMContourSmoothing::None,0.0,0,0.0,FALSE,DTMFenceOption::Overlap,DTMFenceType::Shape,NULL,0,0,0,0.0,bcdtmWrite_loadDtmFeatureFunction,&dtmFeatureLoad)) goto errexit ;
/*
** Update Header Items
*/
 dtmHeader.numPoints         = dtmFeatureLoad.numPoints ;
 dtmHeader.numFeatures       = dtmFeatureLoad.numFeatures ;
 dtmHeader.xMin              = dtmFeatureLoad.xMin ;
 dtmHeader.yMin              = dtmFeatureLoad.yMin ;
 dtmHeader.zMin              = dtmFeatureLoad.zMin ;
 dtmHeader.xMax              = dtmFeatureLoad.xMax  ;
 dtmHeader.yMax              = dtmFeatureLoad.yMax ;
 dtmHeader.zMax              = dtmFeatureLoad.zMax ;
/*
**  Write File Header Variables
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"DTM Feature File Header Variables") ;
    bcdtmWrite_message(0,0,0,"**** dtmFileType        = %p",dtmHeader.dtmFileType)  ;
    bcdtmWrite_message(0,0,0,"**** dtmVersionNumber   = %10ld",dtmHeader.dtmVersionNumber)  ;
    bcdtmWrite_message(0,0,0,"**** numPoints          = %10ld",dtmHeader.numPoints)  ;
    bcdtmWrite_message(0,0,0,"**** numFeatures        = %10ld",dtmHeader.numFeatures)  ;
    bcdtmWrite_message(0,0,0,"**** featureFileOffset  = %10I64d",dtmHeader.featureFileOffset)  ;
    bcdtmWrite_message(0,0,0,"**** xMin               = %12.5lf",dtmHeader.xMin)  ;
    bcdtmWrite_message(0,0,0,"**** xMax               = %12.5lf",dtmHeader.xMax)  ;
    bcdtmWrite_message(0,0,0,"**** yMin               = %12.5lf",dtmHeader.yMin)  ;
    bcdtmWrite_message(0,0,0,"**** yMax               = %12.5lf",dtmHeader.yMax)  ;
    bcdtmWrite_message(0,0,0,"**** zMin               = %12.5lf",dtmHeader.zMin)  ;
    bcdtmWrite_message(0,0,0,"**** zMax               = %12.5lf",dtmHeader.zMax)  ;
   } 
/*
** Rewite Header
*/
 _fseeki64(dtmFP,0,SEEK_SET) ;
 if( fwrite(&dtmHeader,sizeof(BC_DTM_FEATURE_HEADER),1,dtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Writing DTM Feature File") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dtmFP != NULL ) fclose(dtmFP) ;
/*
** Job Completed
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm Object To Dtm Feature File Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing Dtm Feature From Dtm Object To Dtm Feature File Error") ;
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
BENTLEYDTM_EXPORT size_t bcdtmFwrite
(
 void *fromP,
 size_t numBytes,
 size_t numRecs,
 FILE *fileP
 )
 /*
 ** This Is A Wrapper Function For The C "fwrite" function
 ** This Function was written to overcome problems with writing large
 ** packets to a network file
 **
 ** Author : Rob Cormack 
 ** email  : rob.cormack@bentley.com 
 ** date   : 21 January 2007
 **
 */
{
    int     dbg=DTM_TRACE_VALUE(0) ;
    char    *bufferP ;
    size_t  n=0,numPackets=0 ;
    size_t  packetSize,remPacketSize,maxPacketSize=10000000 ;
    /*
    ** Write Entry Message
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"bcdtmFwrite ** fromP = %p numBytes = %9ld numRecs = %9ld fileP = %p",fromP,numBytes,numRecs,fileP) ;
    /*
    ** Determine Packet Size
    */
    packetSize = numBytes * numRecs ;
    if( dbg ) bcdtmWrite_message(0,0,0,"bcdtmFwrite ** packetSize = %8ld",packetSize) ;
    /*
    ** Only Write One Packet If Packet Size Is Less Than The Maximum Packet Size
    */
    if( packetSize <= maxPacketSize ) return( fwrite(fromP,numBytes,numRecs,fileP) )  ; 
    /*
    ** Determine Number Of Packets To Write
    */
    numPackets    = packetSize / maxPacketSize ;
    remPacketSize = packetSize % maxPacketSize ;
    /*
    ** Write Packets
    */
    bufferP = ( char * ) fromP ;
    for( n = 0 ; n < numPackets ; ++n )
    {
        if( dbg ) bcdtmWrite_message(0,0,0,"bcdtmFwrite ** Writing Packet %6ld",n) ;
        if( fwrite(bufferP,maxPacketSize,1,fileP) != 1 )
        {
            return(0) ;
        }
        bufferP = bufferP + maxPacketSize  ;
    } 
    /*
    ** Write Remaining Packet
    */
    if( remPacketSize > 0 )
    {
        if( dbg ) bcdtmWrite_message(0,0,0,"bcdtmFwrite ** Writing Remaining Packet = %6ld",remPacketSize) ;
        if( fwrite(bufferP,remPacketSize,1,fileP) != 1 )
        {
            return(0) ;
        }
    }
    /*
    ** Return
    */
    return(numRecs) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT size_t bcdtmFread
(
 void *toP,
 size_t numBytes,
 size_t numRecs,
 FILE *fileP
 )
 /*
 ** This Is A Wrapper Function For The C "fread" function
 ** This Function was written to overcome problems with reading large
 ** packets from a network file
 **
 ** Author : Rob Cormack 
 ** email  : rob.cormack@bentley.com 
 ** date   : 21 January 2007
 **
 */
{
    int     dbg=DTM_TRACE_VALUE(0) ;
    char    *bufferP ;
    size_t  n=0,numPackets=0 ;
    size_t  packetSize,remPacketSize,maxPacketSize=10000000 ;
    /*
    ** Write Entry Message
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"bcdtmFread ** toP = %p numBytes = %9ld numRecs = %9ld fileP = %p",toP,numBytes,numRecs,fileP) ;
    /*
    ** Determine Packet Size
    */
    packetSize = numBytes * numRecs ;
    if( dbg ) bcdtmWrite_message(0,0,0,"bcdtmFread ** packetSize = %8ld",packetSize) ;
    /*
    ** Only Read One Packet If Packet Size Is Less Than The Maximum Packet Size
    */
    if( packetSize <= maxPacketSize ) return( fread(toP,numBytes,numRecs,fileP) )  ; 
    /*
    ** Determine Number Of Packets To Read
    */
    numPackets    = packetSize / maxPacketSize  ;
    remPacketSize = packetSize % maxPacketSize ;
    /*
    ** Read Packets
    */
    bufferP = ( char * ) toP ;
    for( n = 0 ; n < numPackets ; ++n )
    {
        if( dbg ) bcdtmWrite_message(0,0,0,"bcdtmFread ** Reading Packet %6ld ** filePos = %10ld",n,ftell(fileP)) ;
        if( fread(bufferP,maxPacketSize,1,fileP) != 1 )
        {
            if( dbg ) bcdtmWrite_message(0,0,0,"Error Reading Packet %6ld",n) ;
            return(0) ;
        }
        bufferP = bufferP + maxPacketSize  ;
    } 
    /*
    ** Read Remaining Packet
    */
    if( remPacketSize > 0 )
    {
        if( dbg ) bcdtmWrite_message(0,0,0,"bcdtmFread ** Reading Remaining Packet = %6ld",remPacketSize) ;
        if( fread(bufferP,remPacketSize,1,fileP) != 1 )
        {
            return(0) ;
        }
    }
    /*
    ** Return
    */
    return(numRecs) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                								                     |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT FILE* bcdtmFile_open(WCharCP fileNameP, WCharCP openTypeP)
{
/*
** Open Project Wise File
*/
#ifdef USEPROJWISE
    FILE  *pwFP = NULL, *fileFP = NULL;
    wchar_t  dmsFileName[256];

    pwFP = bcFileWC_fopenPW(fileNameP, openTypeP) ;
    if (pwFP)
    {
        bcFileWC_ProjectWise_ConvertPWpathToDMS(dmsFileName, fileNameP);
        if (pwFP) bcFileWC_fclose(pwFP);
    }
    else 
        wcscpy(dmsFileName, fileNameP);
    fileFP = _wfopen(dmsFileName, openTypeP) ;
#else
    FILE  *fileFP = NULL;
    fileFP = _wfopen(fileNameP, openTypeP) ;
#endif
    return(fileFP);
}
/*-------------------------------------------------------------------+
|                                                                    |
|                								                     |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT void bcdtmFile_close( FILE *dtmFileP )
{
/*
** Close The File
*/
 if( dtmFileP != NULL ) fclose(dtmFileP) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmRead_xyzFileToPointArray(WCharCP xyzFileNameP,DPoint3d **ptsPP,long *numPtsP)
{
 int      dbg=DTM_TRACE_VALUE(0) ;
 long     fileType=0,fileEntries=0,memIniPts,memIncPts ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading XYZ File %ws",xyzFileNameP) ;
/*
** Auto Detect XYZ File Type
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Detecting XYZ File Type") ;
 if( bcdtmUtl_detectXYZFileType(xyzFileNameP,&fileType,&fileEntries) != DTM_SUCCESS ) goto errexit ;
 if( dbg )
   { 
    if( fileType == 1 ) bcdtmWrite_message(0,0,0,"ASCII XYZ File Detected") ;
    if( fileType == 2 ) bcdtmWrite_message(0,0,0,"Binary XYZ File Detected") ;
    bcdtmWrite_message(0,0,0,"Estimated XYZ Record Entries = %6ld",fileEntries) ;
   } 
/*
** Determine Memory Allocation Values 
*/
 if( fileType == 1 ) memIniPts = fileEntries + fileEntries / 1000 ;
 else                memIniPts = fileEntries ;
 memIncPts = fileEntries / 1000 ;
 if( memIncPts < 10000 ) memIncPts = 100000 ;
/*
** Read XYZ File
*/
 if( fileType == 1 ) if( bcdtmRead_xyzASCIIFileToPointArray(xyzFileNameP,ptsPP,numPtsP,memIniPts,memIncPts))  goto errexit ;
 if( fileType == 2 ) if( bcdtmRead_xyzBinaryFileToPointArray(xyzFileNameP,ptsPP,numPtsP,memIniPts,memIncPts)) goto errexit ;
/*
** If No Points Free memory And Return
*/
 if( *numPtsP == 0 )
   {
    if( *ptsPP != NULL ) free(*ptsPP) ;
    bcdtmWrite_message(1,0,0,"No Points In %s",xyzFileNameP) ;
    goto errexit ; 
   }
/*
** All Done Return
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 *numPtsP = 0 ; ; 
 if( *ptsPP != NULL ) { free(*ptsPP) ; ptsPP = NULL ; }
 return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmRead_xyzASCIIFileToPointArray(WCharCP xyzFileP,DPoint3d **ptsPP,long *numPtsP,long memIniPts,long memIncPts)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long   memPts ;
 char   bufferP[512],*bP,*bTopP ;
 double x,y,z ;
 FILE  *fpXYZ=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Reading XYZ ASCII File To Point Array") ;
    bcdtmWrite_message(0,0,0,"xyzFileP  = %s",xyzFileP) ;
    bcdtmWrite_message(0,0,0,"memIniPts = %8ld",memIniPts) ;
    bcdtmWrite_message(0,0,0,"memIncPts = %8ld",memIncPts) ;
   } 
/*
** Initialise
*/
 memPts = 0 ;
 *numPtsP = 0 ;
 if( *ptsPP != NULL ) { free(*ptsPP) ; *ptsPP = NULL ; }
/*
** Open XYZ File
*/
 if( ( fpXYZ = bcdtmFile_open(xyzFileP,L"r")) == NULL )
   { bcdtmWrite_message(1,0,0,"Error Opening XYZ File %s",xyzFileP) ; goto errexit ; }
/*
** Read ASCII XYZ File
*/
 bP = bufferP ;
 *bP = 0 ; 
 bTopP = bufferP + 510 ;
 while( fscanf(fpXYZ,"%c",bP) != EOF )
   {
    if( *bP && *bP != 10 && *bP != 13) ++bP ;
    else
      {
       *bP = 0 ;
       if( ! bcdtmUtl_decodeXYZRecord(bufferP,&x,&y,&z) )
         {
/*
**        Test For Memory Allocation
*/
          if( *numPtsP == memPts )  
            {
             if( memPts == 0 ) memPts = memIniPts ;
             else              memPts = memPts + memIncPts ;
/*
**           Allocate Memory
*/
             if( *ptsPP == NULL ) *ptsPP = ( DPoint3d * ) malloc ( memPts * sizeof(DPoint3d)) ;
             else                 *ptsPP = ( DPoint3d * ) realloc ( *ptsPP , memPts * sizeof(DPoint3d)) ;
             if( *ptsPP == NULL ) 
               { 
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               }
            } 
/*
**        Store Point 
*/
          (*ptsPP+*numPtsP)->x = x  ;
          (*ptsPP+*numPtsP)->y = y  ;
          (*ptsPP+*numPtsP)->z = z  ;
          ++*numPtsP ;
         }
       bP = bufferP ; *bP = 0 ;
      }
    if( bP > bTopP ) bP = bTopP ;
   }
/*
** Reallocate Memory
*/
 if( *numPtsP != memPts )
   {
    memPts = *numPtsP ;
    if( *ptsPP != NULL ) *ptsPP = ( DPoint3d * ) realloc (*ptsPP,memPts * sizeof(DPoint3d)) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( fpXYZ != NULL ) { fclose(fpXYZ) ; fpXYZ = NULL ; }
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
BENTLEYDTM_Private int bcdtmRead_xyzBinaryFileToPointArray(WCharCP xyzFileP,DPoint3d **ptsPP,long *numPtsP,long memIniPts,long memIncPts)
{
 int    ret=DTM_SUCCESS ;
 long   memPts ;
 char   bufferP[24] ;
 double x,y,z ;
 FILE  *fpXYZ=NULL ;
/*
** Initialise
*/
 memPts = 0 ;
 *numPtsP = 0 ;
 if( *ptsPP != NULL ) { free(*ptsPP) ; *ptsPP = NULL ; }
/*
** Open XYZ File
*/
 if( ( fpXYZ = bcdtmFile_open(xyzFileP,L"rb")) == NULL )
   { bcdtmWrite_message(1,0,0,"Error Opening Data File %s",xyzFileP) ; goto errexit ; }
/*
** Read Binary XYZ File
*/
 while( bcdtmFread(bufferP,24,1,fpXYZ) == 1 )
   {
    memcpy((char *)&x ,&bufferP[ 0],8) ;
    memcpy((char *)&y ,&bufferP[ 8],8) ;
    memcpy((char *)&z ,&bufferP[16],8) ;
/*
**  Test For Memory Allocation
*/
    if( *numPtsP == memPts )  
      {
       if( memPts == 0 ) memPts = memIniPts ;
       else              memPts = memPts * memIncPts ;
       if( *ptsPP == NULL ) *ptsPP = ( DPoint3d * ) malloc ( memPts * sizeof(DPoint3d)) ;
       else                 *ptsPP = ( DPoint3d * ) realloc (*ptsPP,memPts * sizeof(DPoint3d)) ;
       if( *ptsPP == NULL ) 
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      }
/*
**     Store Point 
*/
    (*ptsPP+*numPtsP)->x = x  ;
    (*ptsPP+*numPtsP)->y = y  ;
    (*ptsPP+*numPtsP)->z = z  ;
    ++*numPtsP ;
   }
/*
** Reallocate Memory
*/
 if( *numPtsP != memPts )
   {
    memPts = *numPtsP ;
    if( *ptsPP != NULL ) *ptsPP = ( DPoint3d * ) realloc (*ptsPP,memPts * sizeof(DPoint3d)) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( fpXYZ != NULL ) { fclose(fpXYZ) ; fpXYZ = NULL ; }
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
/*------------------------------------------------------------------*
|                                                                   |
|   int bcdtmWrite_binaryFileP3D                                    |
|                                                                   |
*------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmWrite_binaryFileP3D(WCharCP FileName, DPoint3d *DataPts, long numPts)
/*
** This Function Writes 3D Coordinates From a Binary File
*/
{
 DPoint3d   *p3d ;
 FILE  *fpBIN=NULL ;
/*
** Write Status Message
*/
 bcdtmWrite_message(0,1,0,"Writing Binary DPoint3d File %s",FileName) ;
/*
** Open Data File
*/
 if( ( fpBIN = bcdtmFile_open(FileName,L"wb") ) == NULL )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Binary File %s",FileName) ;
    return(1) ;
   }
/*
** Write Binary Data File
*/
 for( p3d = DataPts ; p3d < DataPts + numPts ; ++p3d ) 
   {
    if( bcdtmFwrite(p3d,sizeof(DPoint3d),1,fpBIN) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Writing Binary File %s",FileName) ;
       fclose(fpBIN) ;
       return(1) ;
      }
   }
/*
** Clean Up
*/
 fclose(fpBIN) ;
/*
** Job Completed
*/
 return(0) ;
}
/*------------------------------------------------------------------*
|                                                                   |
|                                                                   |
|                                                                   |
*------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmRead_binaryFileP3D(WCharCP FileName, DPoint3d **DataPts, long *numPts)
/*
** This Function Reads 3D Coordinates From a Binary File
*/
{
 long  MemAmt=500,MemInc=500 ;
 DPoint3d   *p3d1,*p3d2,Point ;
 FILE  *fpBIN=NULL ;
/*
** Write Status Message
*/
 bcdtmWrite_message(0,1,0,"Reading Binary XYZ File %s",FileName) ;
/*
** Initialise Variables
*/
 *numPts = 0 ;
/*
** Open Data File
*/
 if( ( fpBIN = bcdtmFile_open(FileName,L"rb") ) == NULL )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Binary File %s",FileName) ;
    return(1) ;
   }
/*
** Allocate Initial Memory
*/
 if(( *DataPts = ( DPoint3d * ) malloc ( MemAmt * sizeof(DPoint3d))) == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    fclose(fpBIN) ;
    return(1) ;
   }
/*
** Read Binary Data File
*/
 p3d1 = *DataPts ;
 p3d2 = &Point   ;
 while ( bcdtmFread(p3d2,sizeof(DPoint3d),1,fpBIN) == 1 )
   {
    *p3d1 = *p3d2 ;
    ++p3d1 ; ++*numPts ;
    if( *numPts >= MemAmt )
      {
       MemAmt = MemAmt + MemInc ;
       if( ( *DataPts = (DPoint3d*) realloc(*DataPts,MemAmt * sizeof(DPoint3d)) ) == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          fclose(fpBIN) ;
          return(1) ;
         }
       p3d1 = *DataPts + *numPts ;
      }
   }
/*
** Clean Up
*/
 fclose(fpBIN) ;
 if( ( *DataPts = (DPoint3d*) realloc(*DataPts,*numPts * sizeof(DPoint3d))) == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    return(1) ;
   }
/*
** Job Completed
*/
 return(0) ;
}
