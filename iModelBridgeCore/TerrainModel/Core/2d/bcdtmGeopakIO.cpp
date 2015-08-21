/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmGeopakIO.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
/*-------------------------------------------------------------------+
|                                                                    |
|  int bcdtmRead_dataFileToDataObject                                |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmRead_dataFileToDataObject(DTM_DAT_OBJ *Data,WCharCP DataFileName)
{
 long     dbg=DTM_TRACE_VALUE(0),FileType=0,FileEntries=0,FileDecPlaces=0 ;
 DTM_DATA_POINT   *pd ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Data File %s",DataFileName) ;
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDataObject(Data)) return(1) ;
/*
** Auto Detect Data File Type
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Detecting Data File Type") ;
 if( bcdtmUtl_detectDataFileType(DataFileName,&FileType,&FileEntries,&FileDecPlaces) ) return(1) ;
 if( dbg && FileType == 1 ) bcdtmWrite_message(0,0,0,"ASCII Data File Detected") ;
 if( dbg && FileType == 2 ) bcdtmWrite_message(0,0,0,"Binary Data File Detected") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Estimated Data Record Entries = %6ld",FileEntries) ;
/*
** Set Global Variables
*/
 DTM_LAST_DAT_FILE_TYPE        = FileType ;
 DTM_LAST_DAT_FILE_NUM_DEC_PTS = FileDecPlaces ;
/*
** Allocate Memory To Data Object
*/
 Data->iniMemPts = FileEntries ;
 if( bcdtmObject_allocateMemoryDataObject(Data)) return(1) ;
/*
** Read Data File
*/
 if( FileType == 1 )  if( bcdtmRead_dataFileASCIIToDataObject(Data,DataFileName))  return(1) ;
 if( FileType == 2 )  if( bcdtmRead_dataFileBinaryToDataObject(Data,DataFileName)) return(1) ;
/*
** If No Points Free memory And Return
*/
 if( Data->numPts == 0 )
   {
    if( Data->featureCodeP != NULL ) 
    {
        free(Data->featureCodeP) ;
        Data->featureCodeP = NULL;
    }
    if( Data->userTagP != NULL ) 
    {
        free(Data->userTagP) ;
        Data->userTagP = NULL;
    }
    if( Data->pointsP != NULL )
    {
        free(Data->pointsP) ;
        Data->pointsP = NULL;
    }
    bcdtmWrite_message(1,0,0,"No Data In %s",DataFileName) ;
    return(1) ; 
   }
/*
** Free Excess Data Memory
*/
 Data->numFeatPts = Data->memPts = Data->numPts ;
 Data->pointsP = ( DTM_DATA_POINT * ) realloc (Data->pointsP,sizeof(DTM_DATA_POINT)*Data->numPts) ;
 Data->featureCodeP = ( DTM_FEATURE_CODE * ) realloc (Data->featureCodeP,sizeof(DTM_FEATURE_CODE)*Data->numFeatPts) ;
 if( Data->userTagP != NULL )  Data->userTagP = ( DTMUserTag * ) realloc (Data->userTagP,sizeof(DTMUserTag)*Data->numFeatPts) ;
/*
** Get Data Ranges
*/
 Data->xMin = Data->xMax = Data->pointsP->x ;
 Data->yMin = Data->yMax = Data->pointsP->y ;
 Data->zMin = Data->zMax = Data->pointsP->z ;
 for( pd = Data->pointsP + 1 ; pd < Data->pointsP + Data->numPts ; ++pd )
   {
    if( pd->x < Data->xMin ) Data->xMin = pd->x ;
    if( pd->x > Data->xMax ) Data->xMax = pd->x ;
    if( pd->y < Data->yMin ) Data->yMin = pd->y ;
    if( pd->y > Data->yMax ) Data->yMax = pd->y ;
    if( pd->z < Data->zMin ) Data->zMin = pd->z ;
    if( pd->z > Data->zMax ) Data->zMax = pd->z ;
   }
/*
** Determine & Print Data Ranges
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"**** Number of Data Points = %6ld",Data->numPts) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"****     Minimum      Maximum       Range") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"****   ============ ============ ============") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"**** x %12.3lf %12.3lf %12.3lf",Data->xMin,Data->xMax,Data->xMax-Data->xMin) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"**** y %12.3lf %12.3lf %12.3lf",Data->yMin,Data->yMax,Data->yMax-Data->yMin) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"**** z %12.3lf %12.3lf %12.3lf",Data->zMin,Data->zMax,Data->zMax-Data->zMin) ;
/*
** All Done Return
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmRead_dataFileASCIIToDataObject(DTM_DAT_OBJ *Data,WCharCP DataFile)
{
 char   InputBuffer[512],*bp,*bt ;
 long   Fc ;
 double x,y,z ;
 FILE  *fpDATA=NULL ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
**  Open Data File
*/
 if( ( fpDATA = bcdtmFile_open(DataFile,L"r")) == NULL )
   { bcdtmWrite_message(1,0,0,"Error Opening Data File %s",DataFile) ; return(1) ; }
/*
** Read ASCII Data File
*/
 bp = InputBuffer ; *bp = 0 ; bt = InputBuffer + 510 ;
 while( fscanf(fpDATA,"%c",bp) != EOF )
   {
    if( *bp && *bp != 10 && *bp != 13) ++bp ;
    else
      {
       *bp = 0 ;
       if( ! bcdtmUtl_decodeDataRecord(InputBuffer,&Fc,&x,&y,&z) )
         {
          if( bcdtmObject_storePointInDataObject(Data,Fc,DTM_NULL_USER_TAG,nullGuid,x,y,z) )
            { fclose(fpDATA) ; return(1) ; }
         }
       bp = InputBuffer ; *bp = 0 ;
      }
    if( bp > bt ) bp = bt ;
   }
/*
** Close Data File
*/
 fclose(fpDATA) ;
/*
**  Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|    bcdtmRead_dataFileBinaryToDataObject()                            |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmRead_dataFileBinaryToDataObject(DTM_DAT_OBJ *Data,WCharCP DataFile)
{
 long   Fc ;
 char   InputBuffer[28] ;
 double x,y,z ;
 FILE  *fpDATA=NULL ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
**  Open Data File
*/
 if( ( fpDATA = bcdtmFile_open(DataFile,L"rb")) == NULL )
   { bcdtmWrite_message(1,0,0,"Error Opening Data File %s",DataFile) ; return(1) ; }
/*
** Read Binary Data File
*/
 while( bcdtmFread(InputBuffer,28,1,fpDATA) == 1 )
   {
    memcpy((char *)&Fc,&InputBuffer[ 0],4) ;
    memcpy((char *)&x ,&InputBuffer[ 4],8) ;
    memcpy((char *)&y ,&InputBuffer[12],8) ;
    memcpy((char *)&z ,&InputBuffer[20],8) ;
/*
**  Store Point In Object
*/
    if( bcdtmObject_storePointInDataObject(Data,Fc,DTM_NULL_USER_TAG,nullGuid,x,y,z) )
      { fclose(fpDATA) ; return(1) ; }
   }
/*
** Close Data File
*/
 fclose(fpDATA) ;
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
BENTLEYDTM_EXPORT int bcdtmObject_storePointInDataObject
(
 DTM_DAT_OBJ *dataP,         /* ==> Pointer To Dtm Data Object    */
 long featureCode,           /* ==> Dtm Feature Code              */
 DTMUserTag userTag,       /* ==> User Tag                      */
 DTM_GUID userGuid,          /* ==> User Guid                     */
 double x,                   /* ==> x Coordinate                  */
 double y,                   /* ==> y Coordinate                  */
 double z                    /* ==> z Coordinate                  */ 
)
/*
** This function Stores A Point In A Data Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTM_DATA_POINT    *ptP ;
 DTM_FEATURE_CODE  *fcP ;
 DTMUserTag      *utP ;
 DTM_GUID          *guidP ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Write Entry Message
*/
 userGuid = nullGuid ;
 if( dbg )  bcdtmWrite_message(0,0,0,"Storing Point In Data Object[%p]->%6ld ** %2d %12I64d %12.4lf %12.4lf %12.4lf",dataP,dataP->numPts,featureCode,userTag,x,y,z)  ;
/*
** Test For Valid Feature Code
*/
 if( ( featureCode >= 1 && featureCode <= 33 ) || ( featureCode >= 40 && featureCode <= 43 ) || ( featureCode >= 50 && featureCode <= 53 ) ) 
   { 
/*
** If Data Object Sorted Then DeSort
*/
    if( dataP->stateFlag ) if( bcdtmObject_deSortDataObject(dataP)) goto errexit ;
/*
** Test Memory Allocations
*/
    if( dataP->numPts == dataP->memPts ) 
      {
       if( bcdtmObject_allocateMemoryDataObject(dataP)) goto errexit  ;
      }
/*
** Test For A Non Null User Tag
*/
    if( dataP->userTagP == NULL && userTag != DTM_NULL_USER_TAG )
      {
       dataP->userTagP = (DTMUserTag *) malloc( dataP->memPts * sizeof(DTMUserTag)) ;
       if( dataP->userTagP == NULL )  { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
       for( utP = dataP->userTagP ; utP < dataP->userTagP + dataP->memPts ; ++utP ) *utP = DTM_NULL_USER_TAG ;
      }
/*
** Test For A Non Null Guid 
*/
    if( dataP->guidP == NULL && memcmp(&userGuid,&nullGuid,16) != 0  )
      {
       dataP->guidP = (DTM_GUID *) malloc( dataP->memPts * sizeof(DTM_GUID)) ;
       if( dataP->guidP == NULL )  { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit  ; }
       for( guidP = dataP->guidP ; guidP < dataP->guidP + dataP->memPts ; ++guidP ) *guidP = nullGuid ;
      }
/*
** Set Bounding Cube For Data Object
*/
    if( dataP->numPts == 0 )
      {
       dataP->xMin = dataP->xMax = x ;
       dataP->yMin = dataP->yMax = y ;
       dataP->zMin = dataP->zMax = z ;
      }
    else
      {
       if( x < dataP->xMin ) dataP->xMin = x ;
       if( x > dataP->xMax ) dataP->xMax = x ;
       if( y < dataP->yMin ) dataP->yMin = y ;
       if( y > dataP->yMax ) dataP->yMax = y ;
       if( z < dataP->zMin ) dataP->zMin = z ;
       if( z > dataP->zMax ) dataP->zMax = z ;
      }
/*
** Store Data Point
*/
    ptP = dataP->pointsP + dataP->numPts ;
    ptP->x = x ; 
    ptP->y = y ; 
    ptP->z = z ;
    fcP = dataP->featureCodeP + dataP->numPts ;
    *fcP = featureCode ; 
    if( dataP->userTagP != NULL ) 
      { 
       utP = dataP->userTagP + dataP->numPts ;
       *utP = userTag ;
      }
    if( dataP->guidP != NULL ) 
      { 
       guidP  = dataP->guidP + dataP->numPts ;
       *guidP = userGuid ;
      }
    ++dataP->numPts ;
    dataP->numFeatPts = dataP->numPts ;
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
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_allocateMemoryDataObject(DTM_DAT_OBJ *dataP)
/*
** This Routine Allocates Memory For A Data Object
*/
{
 int ret=DTM_SUCCESS ;
 DTM_DATA_POINT    *ptP ;
 DTM_FEATURE_CODE  *fcP ;
 DTMUserTag      *utP ;
 DTM_GUID          *guP ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Initial Allocation .
** Note :- Initial Memory For User Tags And Guids Is Allocated
** When They Are Detected In The Input Stream
*/
 if( dataP->memPts == 0 )
   {
    dataP->memPts = dataP->iniMemPts ;
    dataP->pointsP = ( DTM_DATA_POINT * ) malloc ( dataP->memPts * sizeof(DTM_DATA_POINT)) ;
    dataP->featureCodeP = ( DTM_FEATURE_CODE * ) malloc ( dataP->memPts * sizeof(DTM_FEATURE_CODE)) ;
    if( dataP->featureCodeP == NULL || dataP->pointsP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
   }
/*
**  Subsequent Allocation
*/
 else
   {
    dataP->memPts = dataP->memPts + dataP->incMemPts ;
    dataP->pointsP = ( DTM_DATA_POINT * ) realloc( dataP->pointsP,dataP->memPts * sizeof(DTM_DATA_POINT)) ;
    dataP->featureCodeP = ( DTM_FEATURE_CODE * ) realloc( dataP->featureCodeP,dataP->memPts * sizeof(DTM_FEATURE_CODE)) ;
    if( dataP->featureCodeP == NULL || dataP->pointsP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
** Allocate Memory For User Tags
*/
    if( dataP->userTagP != NULL )
      {
       dataP->userTagP = ( DTMUserTag * ) realloc( dataP->userTagP,dataP->memPts * sizeof(DTMUserTag)) ;
       if( dataP->userTagP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      } 
/*
** Allocate Memory For Guids
*/
    if( dataP->guidP != NULL )
      {
       dataP->guidP = ( DTM_GUID * ) realloc( dataP->guidP,dataP->memPts * sizeof(DTM_GUID)) ;
       if( dataP->guidP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
      } 
   }
/*
** Initialise Values
*/
 for( ptP = dataP->pointsP + dataP->numPts  ; ptP < dataP->pointsP + dataP->memPts ; ++ptP ) {  ptP->x = ptP->y = ptP->z = 0.0 ; }
 for( fcP = dataP->featureCodeP + dataP->numPts  ; fcP < dataP->featureCodeP + dataP->memPts ; ++fcP ) *fcP = 0 ;
 if( dataP->userTagP != NULL ) for( utP = dataP->userTagP + dataP->numPts ; utP < dataP->userTagP + dataP->memPts ; ++utP ) *utP = DTM_NULL_USER_TAG ;
 if( dataP->guidP    != NULL ) for( guP = dataP->guidP    + dataP->numPts ; guP < dataP->guidP    + dataP->memPts ; ++guP ) *guP = nullGuid    ;
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
 if( dataP->pointsP      != NULL ) { free(dataP->pointsP)      ; dataP->pointsP      = NULL ; }
 if( dataP->featureCodeP != NULL ) { free(dataP->featureCodeP) ; dataP->featureCodeP = NULL ; }
 if( dataP->userTagP     != NULL ) { free(dataP->userTagP)     ; dataP->userTagP     = NULL ; }
 if( dataP->guidP        != NULL ) { free(dataP->guidP)        ; dataP->guidP        = NULL ; }
 dataP->numPts = 0 ;
 dataP->memPts = 0 ;
 dataP->numFeatPts = 0 ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   bcdtmUtl_decodeDataRecord()                                        |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmUtl_decodeDataRecord(char *inbuf,long *fc,double *x,double *y,double *z)
{
 int   i,err= 0 ;
 char  *bp,ibuf[100] ;
/*
** Scan and Decode Data Record
*/
 bp = inbuf ;
 for ( i = 0 ; i < 4  && ! err ; ++i )
   {
    if( ! bcdtmUtl_getNextString(&bp,ibuf) ) return(1) ;
    else
      {
       if( i == 0 && ! bcdtmUtl_checkInteger(ibuf)) err = 1 ;
       if( i >  0 && ! bcdtmUtl_checkReal(ibuf)   ) err = 1 ;
       if( ! err  )
         {
          if( i == 0 ) sscanf(ibuf,"%ld",fc) ;
          if( i == 1 ) sscanf(ibuf,"%lf",x) ;
          if( i == 2 ) sscanf(ibuf,"%lf",y) ;
          if( i == 3 ) sscanf(ibuf,"%lf",z) ;
         }
      }
   }
 return(err) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_detectDataFileType( WCharCP fileName,long *fileType ,long *fileEntries,long *fileDecPlaces)
/*
** Routine to automatically detect data file type
** Return Values == 0   Success
**               == 1   Cannot Open Data File
** Arguements
**   ==> fileName  char *  Input  Data File Name
**   <== fileType  long *  Output File Type
**               == 1  Ascii  Data File
**               == 2  Binary Data File
**
** 
**  Rob Cormack Modified 4 Feb 2003 To Determine Number Of Decimal Points For
**  Subsequent Writing Of An Ascii Data File.
*/ 
{
 long ascCnt=0,binCnt=0,charCnt=0,lfCnt=0,numChar=0 ;
/*
 char cVal ;
*/
 long numDecPlaces,maxDecPlaces ;
 char *pc,*pchar,Buffer[512] ;
 FILE *fpTmp=NULL ;
/*
**  Open File
*/
 if( (fpTmp = bcdtmFile_open(fileName,L"rb")) == NULL )
   {
    bcdtmWrite_message(1,0,0,"Cannot Open Data File %ws For Reading",fileName) ;
    return(1) ;
   }
/*
** Read First 512 Characters
*/
 maxDecPlaces = 0 ;
 charCnt = (long)fread(Buffer,1,512,fpTmp) ;
 for( pchar = Buffer ; pchar < Buffer + charCnt ; ++pchar )
   {
    if( *pchar >= 32 && *pchar <=127 ) ++ascCnt ;
    else                               ++binCnt ;
    if( *pchar == '\r' || *pchar == '\n' ) --binCnt ;
    if( *pchar == '\n' ) ++lfCnt ; 
    if( *pchar == '.'  )
      {
       numDecPlaces = 0 ;
       pc = pchar ;
       while( pc < Buffer + charCnt && *pc != ' ' && *pc != '\r' && *pc != '\n' ) 
         {
          ++numDecPlaces ;
          ++pc ;
         }
       --numDecPlaces ;
       if( numDecPlaces > maxDecPlaces ) maxDecPlaces = numDecPlaces ; 
      }
   }
/*
** Read First 512 Characters
*/
/*
 while( fread(&cVal,1,1,fpTmp) == 1 && charCnt < 512 )
   {
    if( cVal >= 32 && cVal <=127 ) ++ascCnt ;
    else                           ++binCnt ;
    if( cVal == '\r' || cVal == '\n' ) --binCnt ;
    if( cVal == '\n' ) ++lfCnt ; 
    ++charCnt ;
   }
*/
/*
** Test For Empty File
*/
 if( charCnt == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Data File %ws is empty",fileName) ;
    fclose(fpTmp) ; 
    return(1) ; 
   }
/*
** Test For File Smaller Than 512 Characters
*/ 
 if( feof(fpTmp) ) binCnt -= 4 ;
 if( lfCnt == 0  )  lfCnt  = 1 ;
 lfCnt = charCnt / lfCnt ;
/*
** Go To End Of File
*/
 fseek(fpTmp,0,SEEK_END) ;
 numChar = ftell(fpTmp)  ;
/*
** Close File
*/
 fclose(fpTmp) ;
/*
** Set Return Values
*/
 if( binCnt > 0 ) { *fileType = 2 ; *fileEntries = numChar / 28  ; }
 else             { *fileType = 1 ; *fileEntries = numChar / lfCnt + 10  ; } 
/*
** If File Type Ascii , Determine Average Number Of Decimal Points
*/
 *fileDecPlaces = 0 ;
 if( *fileType == 1 ) *fileDecPlaces = maxDecPlaces ;
/*
** Check Binary File For Correct Size
*/
 if( *fileType == 2 )
   {
    if( numChar % 28 != 0 ) 
      {
       bcdtmWrite_message(1,0,0,"Error In Data File %s",fileName) ;
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
BENTLEYDTM_EXPORT int bcdtmRead_atFilePositionDataObject(DTM_DAT_OBJ **dataPP,FILE *fpDATA,long filePosition)
{
    Bentley::TerrainModel::IBcDtmStream* dtmStreamP = NULL;
    int status;
    bcdtmStream_createFromFILE(fpDATA, &dtmStreamP);
    status = bcdtmReadStream_atFilePositionDataObject(dataPP, dtmStreamP, filePosition);
    bcdtmStream_destroy(&dtmStreamP);
    return status;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmReadStream_atFilePositionDataObject(DTM_DAT_OBJ **dataPP,Bentley::TerrainModel::IBcDtmStream* dtmStreamP,long filePosition)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFileType,dtmFileVersion,buffer[2] ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"**** Reading Geopak Data Object At File Position = %10ld",filePosition) ;  
/*
** Check For Non Null File Pointer
*/
 if( dtmStreamP == NULL ) 
   { 
    bcdtmWrite_message(2,0,0,"Null Stream Pointer") ;
    goto errexit  ;
   }
/*
** Position File Pointer At File Position
*/
 if( bcdtmStream_fseek(dtmStreamP,filePosition,SEEK_SET) ) 
   { 
    bcdtmWrite_message(2,0,0,"File Seek Error - Reading Data Object") ; 
    goto errexit  ;
   }
/*
** Create Data Object
*/
 if( bcdtmObject_createDataObject(dataPP)) goto errexit  ;
/*
** Read File Type And Version Number
*/
 if( bcdtmStream_fread(buffer,8,1,dtmStreamP) != 1 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
    goto errexit ; 
   }
 memcpy(&dtmFileType,&buffer[0],4) ;
 memcpy(&dtmFileVersion,&buffer[1],4) ;
/*
** Test For A Bentley Civil Data Object
*/  
 if( dtmFileType != DTM_DAT_TYPE ) 
   { 
    bcdtmWrite_message(1,0,0,"Not A Bentley Civil Dtm Data Object") ;
    goto errexit ;
   }
/*
** Read Relevent Data Object Version 
*/
 switch ( dtmFileVersion )
   {
    case  400 :   /* Version Four Data Object */
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Ver 400 Data Object At File Position %9ld",filePosition) ;  
      if( bcdtmReadStream_atFilePositionVer400DataObject(*dataPP,dtmStreamP,filePosition)) goto errexit ;
    break ;

    case  500 :   /* Version Five Data Object */
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Ver 500 Data Object At File Position %9ld",filePosition) ;  
      if( bcdtmReadStream_atFilePositionVer500DataObject(*dataPP,dtmStreamP,filePosition)) goto errexit ;
    break ;

    case  501 :   /* Version Five One Data Object */
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Ver 501 Data Object At File Position %9ld",filePosition) ;  
      if( bcdtmReadStream_atFilePositionVer501DataObject(*dataPP,dtmStreamP,filePosition)) goto errexit ;
    break ;

    case  502 :   /* Version Five Two Data Object - Bentley Civil */
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Ver 502 Data Object At File Position %9ld",filePosition) ;  
      if( bcdtmReadStream_atFilePositionVer502DataObject(*dataPP,dtmStreamP,filePosition)) goto errexit ;
    break ;

    default :     /* Unknown Data Object File Version */
      bcdtmWrite_message(1,0,0,"Unknown Data Object File Version") ;
      goto errexit ;
    break ;
   } ;
/*
** Report File Position
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"File Position After Geopak Data Object Read = %10ld ** Size = %10ld",bcdtmStream_ftell(dtmStreamP),bcdtmStream_ftell(dtmStreamP)-filePosition) ;
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
 if( *dataPP != NULL ) bcdtmObject_deleteDataObject(dataPP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmReadStream_atFilePositionVer400DataObject(DTM_DAT_OBJ *dataP,Bentley::TerrainModel::IBcDtmStream *dtmStreamP,long FilePosition)
{
 int ret=DTM_SUCCESS ;
 long fndp ;
/*
** Define Structures For Version 400 Data Object
*/
 struct Dataod { double  x,y,z ;}  ;
 struct DataObj
  {
   long     dtmFileType,dtmFileVersion ;
   long     numPts,memPts,numFeatPts,incMemPts,iniMemPts,numDecDigits,stateFlag ;
   double   xMin,yMin,zMin,xMax,yMax,zMax ;
   double   ppTol,plTol ; 
   char     userName[DTM_FILE_SIZE] ;
   char     dataObjectFileName[DTM_FILE_SIZE] ;
   long     *featureCodeP ;
   long     *userTagP ; 
   struct Dataod *pointsP ;
  } dataObj ;
// struct Dataod *pod,*DataOd=NULL  ;
 long   oldUserTag,oldNullTag=888888888  ;
// DTM_DATA_POINT *fcP ; 
 DTMUserTag   *utP,userTag ;
/*
** Position File Pointer At File Position
*/
 if( bcdtmStream_fseek(dtmStreamP,FilePosition,SEEK_SET) ) 
   {
    bcdtmWrite_message(1,0,0,"File Seek Error - Reading Ver 4.0 Data Object") ;
    goto errexit  ;
   }
/*
** Read Data Header
*/
 if( bcdtmStream_fread(&dataObj,sizeof(struct DataObj),1,dtmStreamP) != 1 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
    goto errexit ;
   }
/*
** Check For Bentley Civil DTM Data Object
*/
 if( dataObj.dtmFileType != DTM_DAT_TYPE ) 
   { 
    bcdtmWrite_message(1,0,0,"Not A Bentley Civil Dtm Data Object") ;
    goto errexit ;
   }
/*
** Test For Version Number Of Data File
*/
  if( dataObj.dtmFileVersion != 400 ) 
   {
    bcdtmWrite_message(1,0,0,"Not A Version 400 Bentley Civil Data Object") ;
    goto errexit ;
   }
/*
** Set Data Object Header Variables
*/
 dataP->dtmFileType     = DTM_DAT_TYPE ;
 dataP->dtmFileVersion  = DTM_DAT_FILE_VERSION ;
 dataP->refCount        = 0;
 dataP->userStatus      = 0 ;
 dataP->creationTime    = 0 ;
 dataP->modifiedTime    = 0 ;
 dataP->userTime        = 0 ;
 dataP->xMin            = dataObj.xMin ;
 dataP->yMin            = dataObj.yMin ;
 dataP->zMin            = dataObj.zMin ;
 dataP->xMax            = dataObj.xMax ;
 dataP->yMax            = dataObj.yMax ;
 dataP->zMax            = dataObj.zMax ;
 dataP->numPts          = dataObj.numPts   ;
 dataP->memPts          = dataObj.memPts  ;
 dataP->numFeatPts      = dataObj.numFeatPts  ;
 dataP->stateFlag       = dataObj.stateFlag ;
 dataP->iniMemPts       = dataObj.iniMemPts ;
 dataP->incMemPts       = dataObj.incMemPts ;
 dataP->plTol           = dataObj.plTol ;
 dataP->ppTol           = dataObj.ppTol ;
 dataP->numDecDigits    = dataObj.numDecDigits ;
 dataP->featureCodeP    = NULL  ;
 dataP->userTagP        = NULL  ;
 dataP->pointsP         = NULL  ;
 dataP->guidP           = NULL  ;
 strcpy(dataP->userName,dataObj.userName) ;
 strcpy(dataP->dataObjectFileName,dataObj.dataObjectFileName) ;
 strcpy(dataP->userMessage,"") ;
/*
** Initialise
*/
 if( dataObj.memPts >= dataObj.numFeatPts ) fndp = dataObj.memPts ;
 else                                       fndp = dataObj.numFeatPts ;
/*
** Populate Arrays
*/
 if( dataObj.memPts > 0 )
   {
/*
**  Read And Populate Points Array
*/
    dataP->pointsP = ( DTM_DATA_POINT *) malloc( dataP->memPts * sizeof(DTM_DATA_POINT)) ;
    if( dataP->pointsP == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  RobC - Following Code Commented Out 20/9/2005
**  As The PointCode Has Been Removed From NGP Data Objects
**
    DataOd  = ( struct Dataod * )  malloc( dataP->memPts * sizeof(struct Dataod)) ; 
    if( DataOd == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    if( bcdtmStream_fread(DataOd,sizeof(struct Dataod )*dataObj.memPts,1,dtmStreamP) != 1 ) 
      { 
       bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
       goto errexit ; 
      }
    for ( fcP = dataP->pointsP , pod = DataOd ; fcP < dataP->pointsP + dataP->memPts ; ++fcP , ++pod )
      {
       fcP->x         = pod->x ;
       fcP->y         = pod->y ;
       fcP->z         = pod->z ;
      }
*/
/*
**  RobC - Following Code Added 20/9/2005
**  As The PointCode Has Been Removed From NGP Data Objects
*/
    if( bcdtmStream_fread(dataP->pointsP,sizeof(DTM_DATA_POINT)*dataP->memPts,1,dtmStreamP) != 1 ) 
      { 
       bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
       goto errexit ; 
      }
/*
** Free Memory
*/
//     if( DataOd != NULL )  
//       { 
//        free(DataOd)  ; 
//        DataOd = NULL ; 
//       }
/*
** Read And Populate Feature Code Array
*/
     dataP->featureCodeP = (DTM_FEATURE_CODE *) malloc(fndp * sizeof(DTM_FEATURE_CODE)) ;
     if( dataP->featureCodeP == NULL ) 
       { 
        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
       }
     if( bcdtmStream_fread(dataP->featureCodeP,sizeof(DTM_FEATURE_CODE)*fndp,1,dtmStreamP) != 1 ) 
       { 
        bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
        goto errexit ;
       }
/*
** Read And Populate User Tag Array
*/
    if( dataObj.userTagP != NULL )
      { 
       dataP->userTagP = (DTMUserTag *) malloc( fndp * sizeof(DTMUserTag)) ;
       if( dataP->userTagP == NULL ) 
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ; 
         }
       for( utP = dataP->userTagP ; utP < dataP->userTagP + fndp ; ++utP )
         { 
          if( bcdtmStream_fread(&oldUserTag,sizeof(long),1,dtmStreamP) != 1 ) 
            { 
             bcdtmWrite_message(1,0,0,"Error Reading Data Object") ; 
             goto errexit ; 
            }
          else
            {
             if( oldUserTag == oldNullTag ) userTag = DTM_NULL_USER_TAG ;
             else                           userTag = ( DTMUserTag ) oldUserTag ;
             memcpy(utP,&userTag,sizeof(DTMUserTag));
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
// if( DataOd != NULL ) free(DataOd) ; 
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
BENTLEYDTM_Public int bcdtmReadStream_atFilePositionVer500DataObject(DTM_DAT_OBJ *dataP,Bentley::TerrainModel::IBcDtmStream *dtmStreamP,long FilePosition)
{
 int ret=DTM_SUCCESS ;
 long fndp ;
/*
** Define Structures For Version 500 Data Object
*/
 struct Dataod { double  x,y,z ;}  ;
 struct DataObj
  {
   long     dtmFileType,dtmFileVersion ;
   long     numPts,memPts,numFeatPts,incMemPts,iniMemPts,numDecDigits,stateFlag ;
   long     UserData,UserStatus ;
#ifdef _WIN32_WCE 
   long     creationTime,modifiedTime,userTime ;
#else
   __time32_t   creationTime,modifiedTime,userTime ;
#endif
   double   xMin,yMin,zMin,xMax,yMax,zMax ;
   double   ppTol,plTol ; 
   char     userName[DTM_FILE_SIZE] ;
   char     dataObjectFileName[DTM_FILE_SIZE] ;
   char     userMessage[256] ;
   long     *featureCodeP ;
   Int64  *userTagP ; 
   struct Dataod   *pointsP ;
  } dataObj ;
// struct Dataod *pod,*DataOd=NULL ;
// DTM_DATA_POINT *fcP ; 
/*
** Position File Pointer At File Position
*/
 if( bcdtmStream_fseek(dtmStreamP,FilePosition,SEEK_SET) ) 
   {
    bcdtmWrite_message(1,0,0,"File Seek Error") ;
    goto errexit  ;
   }
/*
** Read Data Header
*/
 if( bcdtmStream_fread(&dataObj,sizeof(struct DataObj),1,dtmStreamP) != 1 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
    goto errexit ;
   }
/*
** Check For A Bentley Civil DTM Data Object
*/
 if( dataObj.dtmFileType != DTM_DAT_TYPE ) 
   { 
    bcdtmWrite_message(1,0,0,"Not A Bentley Civil Dtm Data Object") ;
    return(1) ;
   }
/*
** Test For Version Number Of Data File
*/
 if( dataObj.dtmFileVersion != 500 ) 
   {
    bcdtmWrite_message(1,0,0,"Not A Version 500 Bentley Civil Data Object") ;
    return(1) ;
   }
/*
** Set Data Object Header Variables
*/
 dataP->dtmFileType     = DTM_DAT_TYPE ;
 dataP->dtmFileVersion  = DTM_DAT_FILE_VERSION ;
 dataP->refCount        = 0;
 dataP->userStatus      = dataObj.UserStatus ;
 dataP->creationTime    = dataObj.creationTime ;
 dataP->modifiedTime    = dataObj.modifiedTime ;
 dataP->userTime        = dataObj.userTime ;
 dataP->xMin            = dataObj.xMin ;
 dataP->yMin            = dataObj.yMin ;
 dataP->zMin            = dataObj.zMin ;
 dataP->xMax            = dataObj.xMax ;
 dataP->yMax            = dataObj.yMax ;
 dataP->zMax            = dataObj.zMax ;
 dataP->numPts          = dataObj.numPts   ;
 dataP->memPts          = dataObj.memPts  ;
 dataP->numFeatPts      = dataObj.numFeatPts  ;
 dataP->stateFlag       = dataObj.stateFlag ;
 dataP->iniMemPts       = dataObj.iniMemPts ;
 dataP->incMemPts       = dataObj.incMemPts ;
 dataP->plTol           = dataObj.plTol ;
 dataP->ppTol           = dataObj.ppTol ;
 dataP->numDecDigits    = dataObj.numDecDigits ;
 dataP->featureCodeP    = NULL  ;
 dataP->userTagP        = NULL  ;
 dataP->pointsP         = NULL  ;
 dataP->guidP           = NULL  ;
 strcpy(dataP->userName,dataObj.userName) ;
 strcpy(dataP->dataObjectFileName,dataObj.dataObjectFileName) ;
 strcpy(dataP->userMessage,dataObj.userMessage) ;
/*
** Initialise
*/
 if( dataObj.memPts >= dataObj.numFeatPts ) fndp = dataObj.memPts ;
 else                                       fndp = dataObj.numFeatPts ;
/*
** Populate Arrays
*/
 if( dataObj.memPts > 0 )
   {
/*
**  Read And Populate Points Array
*/
    dataP->pointsP = ( DTM_DATA_POINT *) malloc( dataP->memPts * sizeof(DTM_DATA_POINT)) ;
    if( dataP->pointsP == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  RobC - Following Code Commented Out 20/9/2005
**  As The PointCode Has Been Removed From NGP Data Objects
**
*
    DataOd  = ( struct Dataod * )  malloc( dataP->memPts * sizeof(struct Dataod)) ; 
    if( DataOd == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    if( bcdtmStream_fread(DataOd,sizeof(struct Dataod )*dataP->memPts,1,dtmStreamP) != 1 ) 
      { 
       bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
       goto errexit ; 
      }
    for ( fcP = dataP->pointsP , pod = DataOd ; fcP < dataP->pointsP + dataP->memPts ; ++fcP , ++pod )
      {
       fcP->x         = pod->x ;
       fcP->y         = pod->y ;
       fcP->z         = pod->z ;
      }
*/
/*
**  RobC - Following Code Added 20/9/2005
**  As The PointCode Has Been Removed From NGP Data Objects
*/
    if( bcdtmStream_fread(dataP->pointsP,sizeof(DTM_DATA_POINT)*dataP->memPts,1,dtmStreamP) != 1 ) 
      { 
       bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
       goto errexit ; 
      }
/*
** Free Memory
*/
//     if( DataOd != NULL )  
//       { 
//        free(DataOd)  ; 
//        DataOd = NULL ; 
//       }
/*
** Read And Populate Feature Code Array
*/
     dataP->featureCodeP = (DTM_FEATURE_CODE *) malloc(fndp * sizeof(DTM_FEATURE_CODE)) ;
     if( dataP->featureCodeP == NULL ) 
       { 
        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
       }
     if( bcdtmStream_fread(dataP->featureCodeP,sizeof(DTM_FEATURE_CODE)*fndp,1,dtmStreamP) != 1 ) 
       { 
        bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
        goto errexit ;
       }
/*
** Read And Populate User Tag Array
*/
    if( dataObj.userTagP != NULL )
      { 
       dataP->userTagP = (DTMUserTag *) malloc( fndp * sizeof(DTMUserTag)) ;
       if( dataP->userTagP == NULL ) 
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ; 
         }
       if( bcdtmStream_fread(dataP->userTagP,sizeof(DTMUserTag)*fndp,1,dtmStreamP) != 1 ) 
         { 
          bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
          goto errexit ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
// if( DataOd != NULL ) free(DataOd) ; 
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
BENTLEYDTM_Public int bcdtmReadStream_atFilePositionVer501DataObject(DTM_DAT_OBJ *dataP,Bentley::TerrainModel::IBcDtmStream* dtmStreamP,long FilePosition)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long fndp,headerSize=0,buffer[4] ;
 DTM_DATA_POINT *pntsP ;
/*
** Define Structures For Version 501 Data Object
*/
 struct Dataod { long pointCode ; double  x,y,z ;} dataPts ;
 struct DataObj
  {
   long     dtmFileType,dtmFileVersion ;
   long     numPts,memPts,numFeatPts,incMemPts,iniMemPts,numDecDigits,stateFlag ;
   long     UserData,UserStatus ;
#ifdef _WIN32_WCE 
   long     creationTime,modifiedTime,userTime ;
#else
   __time32_t   creationTime,modifiedTime,userTime ;
#endif
   double   xMin,yMin,zMin,xMax,yMax,zMax ;
   double   ppTol,plTol ; 
   char     userName[DTM_FILE_SIZE] ;
   char     dataObjectFileName[DTM_FILE_SIZE] ;
   char     userMessage[256] ;
   long     *featureCodeP ;
   Int64  *userTagP ; 
   struct Dataod   *pointsP ;
  } dataObj ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Ver 501 Data Object At File Position %9ld",FilePosition) ; 
/*
** Position File Pointer At File Position
*/
 if( bcdtmStream_fseek(dtmStreamP,FilePosition,SEEK_SET) ) 
   {
    bcdtmWrite_message(1,0,0,"File Seek Error") ;
    goto errexit  ;
   }
/*
** Set Header Size
*/
 headerSize = sizeof(struct DataObj) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"headerSize = %8ld",headerSize) ;
 buffer[0] = 0 ;
 buffer[1] = 0 ;
 buffer[2] = 0 ;
 buffer[3] = 0 ;
 #ifndef _M_IX86
/*
** Read Header In For 64Bit System
*/
 headerSize = 648 - 16 ;
 if( bcdtmStream_fread(&dataObj,headerSize,1,dtmStreamP) != 1 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
    goto errexit  ;
   }
/*
** Read Pointers
*/
 if( bcdtmStream_fread(buffer,sizeof(long),4,dtmStreamP) != 4 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
    goto errexit  ;
   }
/*
** Set Pointer Values
*/
 dataObj.featureCodeP = ( DTM_FEATURE_CODE *) buffer[0] ;
 dataObj.userTagP     = ( DTMUserTag * ) buffer[1] ;
 dataObj.pointsP      = ( struct Dataod * ) buffer[2] ;
 #else
/*
** Read Header For 32Bit Version
*/
 if( bcdtmStream_fread(&dataObj,headerSize,1,dtmStreamP) != 1 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
    goto errexit ;
   }
#endif
/*
** Write Pointer Values
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"pointsP       = %p",dataObj.pointsP) ;
    bcdtmWrite_message(0,0,0,"featureCodeP  = %p",dataObj.featureCodeP) ;
    bcdtmWrite_message(0,0,0,"userTagP      = %p",dataObj.userTagP) ;
    bcdtmWrite_message(0,0,0,"File Position = %8ld",bcdtmStream_ftell(dtmStreamP)) ;
   }

/*
** Check For Bentley Civil DTM Data Object 
*/
 if( dataObj.dtmFileType != DTM_DAT_TYPE ) 
   { 
    bcdtmWrite_message(1,0,0,"Not A Bentley Civil Dtm Data Object") ;
    goto errexit ;
   }
/*
** Test For Version Number Of Data File
*/
  if( dataObj.dtmFileVersion != 501 ) 
   {
    bcdtmWrite_message(1,0,0,"Not A Version 501 Bentley Civil Data Object") ;
    goto errexit ;
   }
/*
** Set Data Object Header Variables
*/
 dataP->dtmFileType     = DTM_DAT_TYPE ;
 dataP->dtmFileVersion  = DTM_DAT_FILE_VERSION ;
 dataP->refCount        = 0;
 dataP->userStatus      = dataObj.UserStatus ;
 dataP->creationTime    = dataObj.creationTime ;
 dataP->modifiedTime    = dataObj.modifiedTime ;
 dataP->userTime        = dataObj.userTime ;
 dataP->xMin            = dataObj.xMin ;
 dataP->yMin            = dataObj.yMin ;
 dataP->zMin            = dataObj.zMin ;
 dataP->xMax            = dataObj.xMax ;
 dataP->yMax            = dataObj.yMax ;
 dataP->zMax            = dataObj.zMax ;
 dataP->numPts          = dataObj.numPts   ;
 dataP->memPts          = dataObj.memPts  ;
 dataP->numFeatPts      = dataObj.numFeatPts  ;
 dataP->stateFlag       = dataObj.stateFlag ;
 dataP->iniMemPts       = dataObj.iniMemPts ;
 dataP->incMemPts       = dataObj.incMemPts ;
 dataP->plTol           = dataObj.plTol ;
 dataP->ppTol           = dataObj.ppTol ;
 dataP->numDecDigits    = dataObj.numDecDigits ;
 dataP->featureCodeP    = NULL  ;
 dataP->userTagP        = NULL  ;
 dataP->pointsP         = NULL  ;
 dataP->guidP           = NULL  ;
 strcpy(dataP->userName,dataObj.userName) ;
 strcpy(dataP->dataObjectFileName,dataObj.dataObjectFileName) ;
 strcpy(dataP->userMessage,dataObj.userMessage) ;
/*
** Initialise
*/
 if( dataObj.memPts >= dataObj.numFeatPts ) fndp = dataObj.memPts ;
 else                                       fndp = dataObj.numFeatPts ;
/*
** Populate Arrays
*/
 if( dataObj.memPts > 0 )
   {
/*
**  Read And Populate Points Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Points Array") ;
    dataP->pointsP = ( DTM_DATA_POINT *) malloc( dataP->memPts * sizeof(DTM_DATA_POINT)) ;
    if( dataP->pointsP == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Read Points Array
*/ 
    for( pntsP = dataP->pointsP ; pntsP < dataP->pointsP + dataP->memPts ; ++pntsP )
      {
       if( bcdtmStream_fread(&dataPts,sizeof(struct Dataod),1,dtmStreamP) != 1 ) 
         {
          bcdtmWrite_message(1,0,0,"Error Reading Data Object Points Array") ;
          bcdtmWrite_message(0,0,0,"Number Of Points Read = %8ld of %8ld",(long)(pntsP-dataP->pointsP),dataP->memPts) ;
          goto errexit ;
         }
       pntsP->x = dataPts.x ;
       pntsP->y = dataPts.y ;
       pntsP->z = dataPts.z ;
      }  
/*
** Read And Populate Feature Code Array
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Reading Feature Codes") ;
     dataP->featureCodeP = (DTM_FEATURE_CODE *) malloc(fndp * sizeof(DTM_FEATURE_CODE)) ;
     if( dataP->featureCodeP == NULL ) 
       { 
        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
       }
     if( bcdtmStream_fread(dataP->featureCodeP,sizeof(DTM_FEATURE_CODE)*fndp,1,dtmStreamP) != 1 ) 
       { 
        bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
        goto errexit ;
       }
/*
** Read And Populate User Tag Array
*/
    if( dataObj.userTagP != NULL )
      { 
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading User Tags") ;
       dataP->userTagP = (DTMUserTag *) malloc( fndp * sizeof(DTMUserTag)) ;
       if( dataP->userTagP == NULL ) 
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ; 
         }
       if( bcdtmStream_fread(dataP->userTagP,sizeof(DTMUserTag)*fndp,1,dtmStreamP) != 1 ) 
         { 
          bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
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
BENTLEYDTM_Public int bcdtmReadStream_atFilePositionVer502DataObject(DTM_DAT_OBJ *dataP,Bentley::TerrainModel::IBcDtmStream* dtmStreamP,long filePosition)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long fndp,headerSize,buffer[4] ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Reading V502 Data Object") ;
    bcdtmWrite_message(0,0,0,"dataP        = %p",dataP) ;
    bcdtmWrite_message(0,0,0,"dtmStreamP   = %p",dtmStreamP) ;
    bcdtmWrite_message(0,0,0,"filePosition = %8ld",filePosition) ;
   }
/*
** Position File Pointer At File Position
*/
 if( bcdtmStream_fseek(dtmStreamP,filePosition,SEEK_SET) ) 
   {
    bcdtmWrite_message(1,0,0,"File Seek Error") ;
    goto errexit  ;
   }
/*
** Read Data Object Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Data Object Header") ;
 headerSize = sizeof(DTM_DAT_OBJ) ;
 buffer[0] = 0 ;
 buffer[1] = 0 ;
 buffer[2] = 0 ;
 buffer[3] = 0 ;

 #ifndef _M_IX86
/*
** Read Header Except Pointers
*/
 headerSize = 632 ;
 if( bcdtmStream_fread(dataP,headerSize,1,dtmStreamP) != 1 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
    goto errexit  ;
   }
/*
** Read Pointers
*/
 if( bcdtmStream_fread(buffer,sizeof(long),4,dtmStreamP) != 4 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
    goto errexit  ;
   }
/*
** Set Pointer Values
*/
 dataP->featureCodeP = ( DTM_FEATURE_CODE *) buffer[0] ;
 dataP->userTagP     = ( DTMUserTag * ) buffer[1] ;
 dataP->pointsP      = ( DTM_DATA_POINT * ) buffer[2] ;
 dataP->guidP        = ( DTM_GUID *) buffer[3] ;
 #else
/*
** Read Header Except Pointers
*/
 if( bcdtmStream_fread(dataP,headerSize,1,dtmStreamP) != 1 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
    goto errexit  ;
   }
 #endif
/*
** Write Header Pointer Arrays
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dataP->pointsP      = %p",dataP->pointsP) ;
    bcdtmWrite_message(0,0,0,"dataP->featureCodeP = %p",dataP->featureCodeP) ;
    bcdtmWrite_message(0,0,0,"dataP->userTagP     = %p",dataP->userTagP) ;
    bcdtmWrite_message(0,0,0,"dataP->guidP        = %p",dataP->guidP) ;
   }
/*
** Check For Bentley Civil Data File Type
*/
 if( dataP->dtmFileType != DTM_DAT_TYPE ) 
   {
    bcdtmWrite_message(1,0,0,"Not A Bentley Civil Dtm Data Object") ;
    goto errexit  ;
   }
/*
** Test For Correct Version Of Data Object
*/
 if( dataP->dtmFileVersion != 502 ) 
   { 
    bcdtmWrite_message(1,0,0,"Not A Version 502 Data Object") ;
    goto errexit  ;
   }
/*
** Only Read Data Object Arrays If Data Object Populated
*/
 if( dataP->memPts > 0 ) 
   {
/*
** Initialise
*/
    if( dataP->memPts >= dataP->numFeatPts ) fndp = dataP->memPts ;
    else                                     fndp = dataP->numFeatPts ;
/*
** Read Points Array
*/
   if( dbg ) bcdtmWrite_message(0,0,0,"Reading Points Array") ;
   dataP->pointsP = (DTM_DATA_POINT *) malloc( dataP->memPts * sizeof(DTM_DATA_POINT)) ;
   if( dataP->pointsP == NULL ) 
     { 
      bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
      goto errexit  ;
     }
   if( bcdtmStream_fread(dataP->pointsP,sizeof(DTM_DATA_POINT)*dataP->memPts,1,dtmStreamP) != 1 ) 
     { 
      bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
      goto errexit  ;
     }
/*
** Read Feature Code Array
*/
   if( dbg ) bcdtmWrite_message(0,0,0,"Reading Feature Code Array") ;
   dataP->featureCodeP = (DTM_FEATURE_CODE *) malloc(fndp * sizeof(DTM_FEATURE_CODE)) ;
   if( dataP->featureCodeP == NULL ) 
     { 
      bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
      goto errexit  ;
     }
   if( bcdtmStream_fread( dataP->featureCodeP,sizeof(DTM_FEATURE_CODE)*fndp,1,dtmStreamP) != 1 )  
     { 
      bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
      goto errexit  ;
     } 
/*
** Read User Tag Array
*/
   if( dataP->userTagP != NULL )
     { 
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading User Tag Array") ;
      dataP->userTagP = (DTMUserTag *) malloc( fndp * sizeof(DTMUserTag)) ;
      if( dataP->userTagP == NULL ) 
        { 
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit  ;
        }
      if( bcdtmStream_fread(dataP->userTagP,sizeof(DTMUserTag)*fndp,1,dtmStreamP) != 1 ) 
        { 
         bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
         goto errexit ;
        }
     }
/*
** Read Guid Array
*/
   if( dataP->guidP != NULL )
     { 
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Guid Array") ;
      dataP->guidP = (DTM_GUID *) malloc( fndp * sizeof(DTM_GUID)) ;
      if( dataP->guidP == NULL ) 
        { 
         bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
         goto errexit  ;
        }
      if( bcdtmStream_fread(dataP->guidP,sizeof(DTM_GUID)*fndp,1,dtmStreamP) != 1 ) 
        { 
         bcdtmWrite_message(1,0,0,"Error Reading Data Object") ;
         goto errexit ;
        }
     }
  }
/*
** Report File Position
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"File Position After Read = %10ld ** file Size = %8ld",bcdtmStream_ftell(dtmStreamP),bcdtmStream_ftell(dtmStreamP)-filePosition) ;
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
BENTLEYDTM_EXPORT int bcdtmRead_atFilePositionTinObject_custom(DTM_TIN_OBJ **Tin,WCharCP fileNameP,double FilePosition)
{
 int ret=DTM_SUCCESS;
 FILE  *tinFP = NULL;
 if( (tinFP = bcdtmFile_open(fileNameP,L"rb")) != (FILE *) NULL )
   {
    if( bcdtmRead_atFilePositionTinObject( Tin,tinFP,(long)FilePosition)!= DTM_SUCCESS ) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmRead_atFilePositionTinObject(DTM_TIN_OBJ **tinPP,FILE *tinFP,long filePosition)
{
Bentley::TerrainModel::IBcDtmStream* dtmStreamP = NULL;
int status;
bcdtmStream_createFromFILE(tinFP, &dtmStreamP);
status = bcdtmReadStream_atFilePositionTinObject(tinPP, dtmStreamP, filePosition);
bcdtmStream_destroy(&dtmStreamP);
return status;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmReadStream_atFilePositionTinObject(DTM_TIN_OBJ **tinPP,Bentley::TerrainModel::IBcDtmStream* dtmStreamP,long filePosition)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0);
 long  dtmFileType,dtmFileVersion,buffer[12],offset ;
 long  verNumber,ident,fileType ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Reading At File Position Geopak Tin Object") ;
    bcdtmWrite_message(0,0,0,"tinPP        = %p",*tinPP) ;
    bcdtmWrite_message(0,0,0,"dtmStreamP   = %p",dtmStreamP) ;
    bcdtmWrite_message(0,0,0,"filePosition = %8ld",filePosition) ;
   }
/*
** Check For Non Null File Pointer
*/
 if( dtmStreamP == NULL ) 
   { 
    bcdtmWrite_message(2,0,0,"Null File Pointer") ;
    goto errexit  ;
   }
/*
** Position File Pointer At File Position
*/
 if( bcdtmStream_fseek(dtmStreamP,filePosition,SEEK_SET)  ) 
  { 
   bcdtmWrite_message(2,0,0,"File Seek Error ** fseek  = %s",strerror(errno))   ;
   goto errexit  ; 
  }
/*
** Create Tin Object
*/
 if( bcdtmObject_createTinObject(tinPP)) goto errexit  ;
/*
** Read DTM File Type And Version Number
*/
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Reading File Type And Version Number")  ;
 if( bcdtmStream_fread(buffer,8,1,dtmStreamP) != 1 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
    goto errexit  ;
   }
 memcpy(&dtmFileType,&buffer[0],4) ;
 memcpy(&dtmFileVersion,&buffer[1],4) ;
/*
** Check For Early Version Geopak Tin File
*/
 if( dtmFileType != DTM_TIN_TYPE ) 
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
    if( ident == DTM_TIN_TYPE ) 
      {
       dtmFileType = DTM_TIN_TYPE ;
       dtmFileVersion = 3 ;
      }
    else if( ( verNumber == 10 || verNumber == 20 ) && fileType == 1 ) 
      {
       dtmFileType = DTM_TIN_TYPE ;
       dtmFileVersion = verNumber ;
      }
   }
/*
** Test For A Bentley Civil Tin Object File
*/  
 if( dtmFileType != DTM_TIN_TYPE ) 
   { 
    bcdtmWrite_message(1,0,0,"Not A Bentley Geopak Tin Object") ;
    goto errexit ;
   }
/*
** Read Relevent Tin Object Version 
*/
 switch ( dtmFileVersion )
   {
    case  3 :   /* Version Three Tin Object */
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Version 3 Tin Object") ;
      if( bcdtmReadStream_atFilePositionVer3TinObject(*tinPP,dtmStreamP,filePosition)) goto errexit ;
    break ;

    case  400 :   /* Version Four Tin Object */
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Version 400 Tin Object") ;
      if( bcdtmReadStream_atFilePositionVer400TinObject(*tinPP,dtmStreamP,filePosition)) goto errexit ;
    break ;

    case  500 :   /* Version Five Tin Object */
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Version 500 Tin Object") ;
      if( bcdtmReadStream_atFilePositionVer500TinObject(*tinPP,dtmStreamP,filePosition)) goto errexit ;
    break ;

    case  501 :   /*  Current Version Tin        */ 
      if( dbg ) bcdtmWrite_message(0,0,0,"Reading Version 501 Tin Object") ;
      if( bcdtmReadStream_atFilePositionVer501TinObject(*tinPP,dtmStreamP,filePosition)) goto errexit ;
    break ; 

    default :     /* Unknown Tin Object File Version */
      bcdtmWrite_message(1,0,0,"Unknown Tin Object File Version %4ld",dtmFileVersion) ;
      goto errexit ;
    break ;
   } ;
/*
** Reset Null Point And Null PTR Values
*/
 if( (*tinPP)->nullPnt != DTM_NULL_PNT || (*tinPP)->nullPtr != DTM_NULL_PTR )
   {
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Resetting Tin Null Pnt and Null Ptr Values") ;
/*
**  Scan Node And Reset Null Values
*/
    if( (*tinPP)->nodesP != NULL )
      {
       for( offset = 0 ; offset < (*tinPP)->memPts ; ++offset )
         {
          if( ((*tinPP)->nodesP+offset)->tPtr  == (*tinPP)->nullPnt ) ((*tinPP)->nodesP+offset)->tPtr  = DTM_NULL_PNT ;         
          if( ((*tinPP)->nodesP+offset)->sPtr  == (*tinPP)->nullPnt ) ((*tinPP)->nodesP+offset)->sPtr  = DTM_NULL_PNT ;         
          if( ((*tinPP)->nodesP+offset)->hPtr  == (*tinPP)->nullPnt ) ((*tinPP)->nodesP+offset)->hPtr  = DTM_NULL_PNT ;         
          if( ((*tinPP)->nodesP+offset)->cPtr  == (*tinPP)->nullPtr ) ((*tinPP)->nodesP+offset)->cPtr  = DTM_NULL_PTR ;         
          if( ((*tinPP)->nodesP+offset)->fPtr  == (*tinPP)->nullPtr ) ((*tinPP)->nodesP+offset)->fPtr  = DTM_NULL_PTR ;         
         }
      }
/*
**  Scan Circular List And Reset Null Values
*/
    if( (*tinPP)->cListP != NULL )
      {
       for( offset = 0 ; offset < (*tinPP)->cListPtr ; ++offset )
         {
          if( ((*tinPP)->cListP+offset)->pntNum  == (*tinPP)->nullPnt ) ((*tinPP)->cListP+offset)->pntNum  = DTM_NULL_PNT ;
          if( ((*tinPP)->cListP+offset)->nextPtr == (*tinPP)->nullPtr ) ((*tinPP)->cListP+offset)->nextPtr = DTM_NULL_PTR ;
         }
      }
/*
**  Scan Feature List And Reset Null Values
*/
    if( (*tinPP)->fListP != NULL )
      {
       for( offset = 0 ; offset < (*tinPP)->memFeatureList ; ++offset )
         {
          if( ((*tinPP)->fListP+offset)->nextPnt == (*tinPP)->nullPnt ) ((*tinPP)->fListP+offset)->nextPnt = DTM_NULL_PNT ;
          if( ((*tinPP)->fListP+offset)->nextPtr == (*tinPP)->nullPtr ) ((*tinPP)->fListP+offset)->nextPtr = DTM_NULL_PTR ;
         }
      }
/*
**  Reset Null Pnt And Null Ptr Values
*/
    if( (*tinPP)->cListDelPtr       == (*tinPP)->nullPtr ) (*tinPP)->cListDelPtr       = DTM_NULL_PTR ;
    if( (*tinPP)->cListLastDelPtr   == (*tinPP)->nullPtr ) (*tinPP)->cListLastDelPtr   = DTM_NULL_PTR ;
    if( (*tinPP)->featureListDelPtr == (*tinPP)->nullPtr ) (*tinPP)->featureListDelPtr = DTM_NULL_PTR ;
    (*tinPP)->nullPnt = DTM_NULL_PNT ;
    (*tinPP)->nullPtr = DTM_NULL_PTR ;
   }
/*
** Calculate Machine Precision For Tin ** Necesary For Pre V9 Tins
*/
 if( (*tinPP)->mppTol <= 0.0 )
   {
    bcdtmNormal_calculateMachinePrecisionForTinObject(*tinPP,&(*tinPP)->mppTol) ;
    if( (*tinPP)->ppTol <= (*tinPP)->mppTol ) (*tinPP)->ppTol = (*tinPP)->mppTol * 1000.0  ;
    if( (*tinPP)->plTol <= (*tinPP)->mppTol ) (*tinPP)->plTol = (*tinPP)->mppTol * 1000.0  ;
   }
/*
** Check Last Point Of Dtm Features ** Necessary For Pre NGP Tins
*/
 if( dtmFileVersion < 501 ) 
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Checking And Fixing Last Point") ;
    if( bcdtmInsert_checkAndFixLastPointOfDtmFeaturesTinObject((*tinPP))) goto errexit ;
   } 
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Check Tin Object Triangulation") ; 
    if( bcdtmCheck_integrityTinObject(*tinPP))
      {
       bcdtmWrite_message(2,0,0,"Tin Object Triangulation Invalid") ;
       goto errexit ;
      } 
    bcdtmWrite_message(2,0,0,"Tin Object Triangulation Valid") ;
   }
/*
** Report File Position
*/
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"File Position After Geopak Tin Object Read = %10ld",bcdtmStream_ftell(dtmStreamP)) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading At File Position Geopak Tin Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading At File Position Geopak Tin Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( *tinPP != NULL ) bcdtmObject_deleteTinObject(tinPP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmReadStream_atFilePositionVer3TinObject
(
 DTM_TIN_OBJ *tinP,
 Bentley::TerrainModel::IBcDtmStream* dtmStreamP,
 long   filePosition
)
/*
** This Routine reads the relevant parameters from the Tin file
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  n,Nvoid=0 ;
 DTM_TIN_POINT   *pntP ;
 DTM_TIN_NODE    *nodeP ;
// DTM_CIR_LIST    *clistP ; 
 struct
   {
    long    dtmFileVersion,FILTYP,MODEFLAG,numPts,NT,NL,hullPnt,nullPtr,nullPnt,cListPtr,NVOID,L1,L2,L3,L4,L5 ;
    double  XMIN,YMIN,ZMIN,XMAX,YMAX,ZMAX,XDIF,YDIF,ZDIF,D1,D2,D3,D4 ;
   } TRGH  ;
 struct  { long Lptr,Fptr,Tptr ; } node ;
 long   nv,vPtr,offset;
 DTMDirection direction;
 double area ;
 DTM_TIN_OBJ *newTinP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Reading At File Position V3 Geopak Tin Object") ;
    bcdtmWrite_message(0,0,0,"tinP         = %p",tinP) ;
    bcdtmWrite_message(0,0,0,"dtmStreamP   = %p",dtmStreamP) ;
    bcdtmWrite_message(0,0,0,"filePosition = %8ld",filePosition) ;
   }
/*
** Seek To File Position
*/
 if( bcdtmStream_fseek(dtmStreamP,filePosition,SEEK_SET)  ) 
   { 
    bcdtmWrite_message(2,0,0,"File Seek Error ** fseek  = %s",strerror(errno))   ;
    goto errexit  ; 
   }
/*
** Read Version 3 Tin File header
*/
 if( bcdtmStream_fread(&TRGH,sizeof(TRGH),1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Reading Dtm Header") ;
    goto errexit ; 
   }
/*
** Set Tin Header Values
*/
 tinP->dtmFileType       = DTM_TIN_TYPE    ;
 tinP->dtmFileVersion    = DTM_TIN_FILE_VERSION  ;
 tinP->numTriangles      = TRGH.NT   ;
 tinP->numLines          = TRGH.NL   ;
 tinP->hullPnt           = TRGH.hullPnt ;
 tinP->nextHullPnt       = TRGH.hullPnt ;
 tinP->iniMemPts         = TRGH.numPts  ;
 tinP->numDecDigits      =   4       ;
 tinP->ppTol             = 0.0       ; 
 tinP->ppTol             = 0.0       ;
 tinP->nullPtr           = TRGH.nullPtr ;
 tinP->nullPnt           = TRGH.nullPnt ;
 tinP->nullUserTag       = DTM_NULL_USER_TAG    ;
 tinP->numPts            = TRGH.numPts  ;
 tinP->memPts            = TRGH.numPts  ;
 tinP->numSortedPts      = TRGH.numPts  ;
 tinP->cListPtr          = TRGH.cListPtr ;
 tinP->cListDelPtr       = TRGH.nullPtr ;
 tinP->cListLastDelPtr   = TRGH.nullPtr ;
 tinP->featureListDelPtr = TRGH.nullPtr ; 
 tinP->numFeatureTable   = 0 ;
 tinP->memFeatureTable   = 0 ;
 tinP->numFeatureList    = 0 ;
 tinP->memFeatureList    = 0 ;
 tinP->iniMemPts          = DTM_INI_MEM_PTS;
 tinP->incMemPts          = DTM_INC_MEM_PTS ;
 tinP->iniMemFeatureTable = DTM_INI_MEM_FEATURES_TABLE ;
 tinP->incMemFeatureTable = DTM_INC_MEM_FEATURES_TABLE ;
 tinP->iniMemFeatureList  = DTM_INI_MEM_FEATURES_LIST ;
 tinP->incMemFeatureList  = DTM_INC_MEM_FEATURES_LIST ; 
 tinP->xMin               = TRGH.XMIN ;
 tinP->yMin               = TRGH.YMIN ;
 tinP->zMin               = TRGH.ZMIN ;
 tinP->xMax               = TRGH.XMAX ;
 tinP->yMax               = TRGH.YMAX ;
 tinP->zMax               = TRGH.ZMAX ;
 tinP->xRange             = TRGH.XMAX - TRGH.XMIN ;
 tinP->yRange             = TRGH.YMAX - TRGH.YMIN ;
 tinP->zRange             = TRGH.ZMAX - TRGH.ZMIN ;
 tinP->pointsP            = NULL ;
 tinP->cListP             = NULL ;
 tinP->fTableP            = NULL ;
 tinP->fListP             = NULL ; 
 tinP->fMapP              = NULL ;
 strcpy(tinP->userName,"") ;
 strcpy(tinP->tinObjectFileName,"") ;
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"numPts  = %8ld  memPts = %8ld",TRGH.numPts,TRGH.numPts) ;
    bcdtmWrite_message(0,0,0,"xMin  = %12.5f  xMax = %12.5lf xRange = %12.5lf",tinP->xMin,tinP->xMax,tinP->xRange) ;
    bcdtmWrite_message(0,0,0,"yMin  = %12.5f  yMax = %12.5lf yRange = %12.5lf",tinP->yMin,tinP->yMax,tinP->yRange) ;
    bcdtmWrite_message(0,0,0,"zMin  = %12.5f  zMax = %12.5lf zRange = %12.5lf",tinP->zMin,tinP->zMax,tinP->zRange) ;
    bcdtmWrite_message(0,0,0,"Nvoid = %12ld  ModeFlag = %12ld",TRGH.NVOID,TRGH.MODEFLAG) ;
   }
/*
** Allocate Memory For Tin Points
*/
 tinP->pointsP = ( DTM_TIN_POINT * ) malloc ( tinP->numPts * sizeof(DTM_TIN_POINT)) ;
 if( tinP->pointsP == NULL )
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
    goto errexit ; 
   }
/*
** Read Tin Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tin Points") ;
 if( bcdtmStream_fread(tinP->pointsP,sizeof(DTM_TIN_POINT)*tinP->numPts,1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Reading Tin File") ;
    goto errexit ; 
   }
 for( pntP = tinP->pointsP ; pntP < tinP->pointsP + tinP->numPts ; ++pntP )
   {  
    pntP->x = pntP->x + tinP->xMin ;
    pntP->y = pntP->y + tinP->yMin ;
    pntP->z = pntP->z + tinP->zMin ;
   }
/*
** Allocate Memory For Tin Nodes
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tin Nodes") ;
 tinP->nodesP = ( DTM_TIN_NODE * ) malloc ( tinP->numPts * sizeof(DTM_TIN_NODE)) ;
 if( tinP->nodesP == NULL )
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
    goto errexit ; 
   }
/*
** Read Tin Nodes
*/
 nodeP = tinP->nodesP ;
 for( n = 0 ; n < tinP->numPts ; ++n )
   {
    if( bcdtmStream_fread(&node,sizeof(node),1,dtmStreamP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Reading Tin File") ;
       goto errexit ; 
      }
    nodeP->cPtr = node.Lptr ;  
    nodeP->hPtr = node.Fptr ;  
    nodeP->tPtr = node.Tptr ;  
    nodeP->sPtr = TRGH.nullPnt ;  
    nodeP->fPtr = TRGH.nullPtr ;
// bcdtmWrite_message(0,0,0,"Node[%8ld] = %10ld %10ld %10ld",n,nodeP->cPtr,nodeP->hPtr,nodeP->tPtr) ;
    ++nodeP ;
   }
/*
** Allocate Storage For Circular List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Circular List") ;
 tinP->cListP  = ( DTM_CIR_LIST * ) malloc ( tinP->cListPtr * sizeof(DTM_CIR_LIST)) ;
 if( tinP->cListP == NULL ) 
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
    goto errexit ;
   } 
/*
** Read Circular List
*/
 if( bcdtmStream_fread(tinP->cListP,tinP->cListPtr*sizeof(DTM_CIR_LIST),1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Reading Tin File") ;
    goto errexit ; 
   }
/*
** Read Voids
*/
 if( TRGH.NVOID > 0 ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Void Features") ;
/*
**  Jump Over Partial Derivatives If They Are Present
*/
    if( TRGH.MODEFLAG == 2 )bcdtmStream_fseek(dtmStreamP,tinP->numPts*20,SEEK_CUR) ;
/*
**  Allocate Memory For Feature Table
*/
    tinP->memFeatureTable = Nvoid ; 
    tinP->fTableP  = ( DTM_FEATURE_TABLE *) malloc(tinP->memFeatureTable * sizeof(DTM_FEATURE_TABLE)) ;
    if( tinP->fTableP == NULL )
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
/*
**  Allocate Memory For Feature List
*/
    tinP->memFeatureList = tinP->numPts / 10 ;
    if( tinP->memFeatureList <= 0 ) tinP->memFeatureList = 1000 ;
    tinP->fListP = ( DTM_FEATURE_LIST_VER200 *) malloc(tinP->memFeatureList * sizeof(DTM_FEATURE_LIST_VER200)) ;
    if( tinP->fListP == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
/*
**  Read the VOIDS
*/
    for( nv = 0 ; nv < TRGH.NVOID ; ++nv )
      {
       if( bcdtmStream_fread(&vPtr,sizeof(long),1,dtmStreamP) != 1 )
         { 
          bcdtmWrite_message(1,0,0,"Error Reading Tin File") ;
          goto errexit ;
         }
/*
**     Check Connectivity Of Void
*/
       if( bcdtmList_checkConnectivityTptrPolygonTinObject(tinP,vPtr,0)) 
         {
          bcdtmWrite_message(1,0,0,"Connectivity Error In Void - Tin File Corrupted") ;
          goto errexit ;
         }
/*
**     Check Void Is Anti Clockwise
*/
       bcdtmMath_calculateAreaAndDirectionTptrPolygonTinObject(tinP,vPtr,&area,&direction) ;
       if (direction == DTMDirection::Clockwise) bcdtmList_reverseTptrPolygonTinObject (tinP, vPtr);
/*
**     Insert Void Into DTMFeatureState::Tin
*/
       if( bcdtmInsert_addDtmFeatureToTinObject(tinP,DTMFeatureType::Void,tinP->nullUserTag,tinP->nullGuid,vPtr,1)) goto errexit ; 
      }
   }
/*
** Reset Null Point And Null PTR Values
*/
 if( tinP->nullPnt != DTM_NULL_PNT || tinP->nullPtr != DTM_NULL_PTR )
   {
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Resetting Tin Null Pnt and Null Ptr Values") ;
/*
**  Scan Node And Reset Null Values
*/
    if( tinP->nodesP != NULL )
      {
       for( offset = 0 ; offset < tinP->memPts ; ++offset )
         {
          if( (tinP->nodesP+offset)->tPtr  == tinP->nullPnt ) (tinP->nodesP+offset)->tPtr  = DTM_NULL_PNT ;         
          if( (tinP->nodesP+offset)->sPtr  == tinP->nullPnt ) (tinP->nodesP+offset)->sPtr  = DTM_NULL_PNT ;         
          if( (tinP->nodesP+offset)->hPtr  == tinP->nullPnt ) (tinP->nodesP+offset)->hPtr  = DTM_NULL_PNT ;         
          if( (tinP->nodesP+offset)->cPtr  == tinP->nullPtr ) (tinP->nodesP+offset)->cPtr  = DTM_NULL_PTR ;         
          if( (tinP->nodesP+offset)->fPtr  == tinP->nullPtr ) (tinP->nodesP+offset)->fPtr  = DTM_NULL_PTR ;         
         }
      }
/*
**  Scan Circular List And Reset Null Values
*/
    if( tinP->cListP != NULL )
      {
       for( offset = 0 ; offset < tinP->cListPtr ; ++offset )
         {
          if( (tinP->cListP+offset)->pntNum  == tinP->nullPnt ) (tinP->cListP+offset)->pntNum  = DTM_NULL_PNT ;
          if( (tinP->cListP+offset)->nextPtr == tinP->nullPtr ) (tinP->cListP+offset)->nextPtr = DTM_NULL_PTR ;
         }
      }
/*
**  Scan Feature List And Reset Null Values
*/
    if( tinP->fListP != NULL )
      {
       for( offset = 0 ; offset < tinP->memFeatureList ; ++offset )
         {
          if( (tinP->fListP+offset)->nextPnt == tinP->nullPnt ) (tinP->fListP+offset)->nextPnt = DTM_NULL_PNT ;
          if( (tinP->fListP+offset)->nextPtr == tinP->nullPtr ) (tinP->fListP+offset)->nextPtr = DTM_NULL_PTR ;
         }
      }
/*
**  Reset Null Pnt And Null Ptr Values
*/
    if( tinP->cListDelPtr       == tinP->nullPtr ) tinP->cListDelPtr       = DTM_NULL_PTR ;
    if( tinP->cListLastDelPtr   == tinP->nullPtr ) tinP->cListLastDelPtr   = DTM_NULL_PTR ;
    if( tinP->featureListDelPtr == tinP->nullPtr ) tinP->featureListDelPtr = DTM_NULL_PTR ;
    tinP->nullPnt = DTM_NULL_PNT ;
    tinP->nullPtr = DTM_NULL_PTR ;
   }
/*
** Clean Tin
*/
 for( nodeP = tinP->nodesP  ; nodeP < tinP->nodesP + tinP->numPts ; ++nodeP ) nodeP->tPtr = tinP->nullPnt ;
 if( bcdtmTin_markInternalVoidPointsTinObject(tinP)) goto errexit ;
 if( bcdtmTin_compactFeatureTableTinObject(tinP))    goto errexit ;
 if( bcdtmTin_compactFeatureListTinObject(tinP))     goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( newTinP != NULL ) bcdtmObject_deleteTinObject(&newTinP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading At File Position V3 Geopak Tin Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading At File Position V3 Geopak Tin Object Error") ;
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
BENTLEYDTM_Public int bcdtmReadStream_atFilePositionVer400TinObject
( 
 DTM_TIN_OBJ *Tin,
 struct      Bentley::TerrainModel::IBcDtmStream* dtmStreamP,
 long        FilePosition
)
{
/*
**  Data Structures For Ver 400 Tin Objects
*/
 int headerSize;
 int dbg=DTM_TRACE_VALUE(0) ;
 struct Tinod { double x,y,z ; unsigned short PRGN,PCWD ; long hPtr,cPtr,tPtr,fPtr ; } Tind ;
 struct Tinoc { long  pntNum,nextPtr ; } ;
 struct Tinof { long  firstPnt; DTMFeatureType TYPE; long UTAG  ; } Tinf ;
 struct Tinol { long  pntNum,FEAT,nextPtr ; } ;
 struct Tinobj
   {
    long    dtmFileType,dtmFileVersion ;
    double  xMin,yMin,zMin,xMax,yMax,zMax,xRange,yRange,zRange,plTol,ppTol ;
    long    NT,NL,hullPnt,nextHullPnt,iniMemPts,incMemPts,iniMemFeatureList,incMemFeatureList,iniMemFeatureTable,incMemFeatureTable,numDecDigits,nullPtr,nullPnt ;
    long    numPts,memPts,Ndp,cListPtr,cListDelPtr,cListLastDelPtr,numFeatureTable,memFeatureTable,numFeaturePts,memFeatureList,featureListDelPtr ;
    char    userName[DTM_FILE_SIZE] ;
    char    tinObjectFileName[DTM_FILE_SIZE] ;
    struct  Tinod *pointsP ;
    struct  Tinoc *cListP ;
    struct  Tinof *fTableP ;
    struct  Tinol *cPtr ;
   } Tinobj ;
 DTM_TIN_POINT     *pd ;
 DTM_FEATURE_TABLE *pf ;
 DTM_TIN_NODE      *pn ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Reading At File Position Ver 400 Tin Object") ;
    bcdtmWrite_message(0,0,0,"Tin          = %p",Tin) ;
    bcdtmWrite_message(0,0,0,"dtmStreamP   = %p",dtmStreamP) ;
    bcdtmWrite_message(0,0,0,"FilePosition = %8ld",FilePosition) ;
   }
/*
** Check For Non Null File Pointer
*/
 if( dtmStreamP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Null Stream Pointer") ; 
    goto errexit ; 
   }
/*
** Position File Pointer At File Position
*/
 if( bcdtmStream_fseek(dtmStreamP,FilePosition,SEEK_SET) ) 
   { 
    bcdtmWrite_message(1,0,0,"File Seek Error") ; 
    goto errexit ; 
   }
/*
** Read Tin Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Ver 400 Header") ;
 
 #ifdef _M_IX86
 headerSize = sizeof(Tinobj);
#else
 headerSize = offsetof(struct Tinobj,pointsP) + (4 * 4) ;
 #endif ;

 if( bcdtmStream_fread(&Tinobj,headerSize,1,dtmStreamP) != 1 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Tin File %s",Tin->tinObjectFileName) ;
    return(1) ;  
   }
/*
** Check For GPK Tin File
*/
 if( Tinobj.dtmFileType != DTM_TIN_TYPE ) 
   { 
    bcdtmWrite_message(1,0,0,"Not A Bentley Civil Tin Object") ;
    goto errexit ; 
   }
/*
** Test For Latter Version Of Tin File
*/
 if( Tinobj.dtmFileVersion != 400) 
   { 
    bcdtmWrite_message(1,0,0,"Not Version 400 Tin Object - Update GEOPAK") ;
    goto errexit ;  
   }
/*
** Copy Ver 400 Header Items To Latest Version
*/ 
 Tin->dtmFileType = DTM_TIN_TYPE ;
 Tin->dtmFileVersion = DTM_TIN_FILE_VERSION ;
 strcpy(Tin->userName,Tinobj.userName) ;
 strcpy(Tin->tinObjectFileName,Tinobj.tinObjectFileName) ;
 Tin->xMin  = Tinobj.xMin ; Tin->yMin = Tinobj.yMin ; Tin->zMin = Tinobj.zMin ;
 Tin->xMax  = Tinobj.xMax ; Tin->yMax = Tinobj.yMax ; Tin->zMax = Tinobj.zMax ;
 Tin->xRange  = Tinobj.xRange ; Tin->yRange = Tinobj.yRange ; Tin->zRange = Tinobj.zRange ;
 Tin->ppTol  = Tinobj.ppTol ;
 Tin->ppTol  = Tinobj.plTol ; 
 Tin->numDecDigits      = Tinobj.numDecDigits   ;
 Tin->numPts            = Tinobj.numPts   ;
 Tin->numSortedPts      = Tinobj.Ndp   ;
 Tin->memPts            = Tinobj.memPts  ;
 Tin->numTriangles      = Tinobj.NT    ;
 Tin->numLines          = Tinobj.NL    ;
 Tin->numFeatureTable   = Tinobj.numFeatureTable ;
 Tin->memFeatureTable   = Tinobj.memFeatureTable ; 
 Tin->numFeatureList    = Tinobj.numFeaturePts ;
 Tin->memFeatureList    = Tinobj.memFeatureList ;
 Tin->hullPnt           = Tinobj.hullPnt  ; 
 Tin->nextHullPnt       = Tinobj.nextHullPnt   ;
 Tin->cListPtr          = Tinobj.cListPtr  ;
 Tin->nullPtr           = Tinobj.nullPtr ; 
 Tin->cListDelPtr       = Tinobj.cListDelPtr  ;
 Tin->cListLastDelPtr   = Tinobj.cListLastDelPtr ;
 Tin->featureListDelPtr = Tinobj.featureListDelPtr ;
 Tin->nullPnt           = Tinobj.nullPnt ;
 Tin->iniMemPts         = Tinobj.iniMemPts ;
 Tin->incMemPts         = Tinobj.incMemPts ;
 Tin->iniMemFeatureTable  = Tinobj.iniMemFeatureTable ;
 Tin->incMemFeatureTable  = Tinobj.incMemFeatureTable ;
 Tin->iniMemFeatureList = Tinobj.iniMemFeatureList ;
 Tin->incMemFeatureList = Tinobj.incMemFeatureList ; 
 Tin->pointsP  = NULL ;
 Tin->nodesP   = NULL ;
 Tin->cListP   = NULL ;
 Tin->fTableP  = NULL ;
 Tin->fListP   = NULL ; 
/*
** Write Header Values
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"numPts         = %8ld memPts         = %8ld",Tin->numPts,Tin->memPts) ;
    bcdtmWrite_message(0,0,0,"numFeatures    = %8ld memFeatures    = %8ld",Tin->numFeatureTable,Tin->memFeatureTable) ;
    bcdtmWrite_message(0,0,0,"numFeatureList = %8ld memFeatureList = %8ld",Tin->numFeatureList,Tin->memFeatureList) ;
    bcdtmWrite_message(0,0,0,"cListPtr       = %8ld",Tin->cListPtr) ;
   }
/*
** Read Tin Object Data If Tin Object Populated
*/
 if( Tin->numPts == 0 )
   {
    Tin->numPts = Tin->memPts = Tin->numFeatureTable = Tin->memFeatureTable = Tin->numFeatureList = Tin->memFeatureList = Tin->cListPtr = 0 ;
   }
 else
   {
/*
** Read Points Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Ver 400 Points Array") ;
    Tin->pointsP = (DTM_TIN_POINT *) malloc( Tin->memPts * sizeof(DTM_TIN_POINT)) ;
    Tin->nodesP  = (DTM_TIN_NODE  *) malloc( Tin->memPts * sizeof(DTM_TIN_NODE)) ;
    if( Tin->pointsP == NULL || Tin->nodesP == NULL ) 
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( pd = Tin->pointsP, pn = Tin->nodesP ; pd < Tin->pointsP + Tin->memPts ; ++pd , ++pn )
      {
       if( bcdtmStream_fread(&Tind,sizeof(struct Tinod),1,dtmStreamP) != 1 ) 
         { 
          bcdtmWrite_message(1,0,0,"Error Reading Tin File %s ",Tin->tinObjectFileName) ;
          goto errexit ;
         }
       pd->x = Tind.x ;
       pd->y = Tind.y ;
       pd->z = Tind.z ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Point[%8ld] = %12.5lf %12.5lf %10.4lf",(long)(pd-Tin->pointsP),pd->x,pd->y,pd->z) ; 
       pn->PRGN = Tind.PRGN ;
       pn->PCWD = Tind.PCWD ;
       pn->cPtr = Tind.cPtr ;
       pn->hPtr = Tind.hPtr ;
       pn->tPtr = Tind.tPtr ;
       pn->fPtr = Tind.fPtr ;
       pn->sPtr = Tin->nullPnt ;
      }
/*
** Read Circular Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Ver 400 Circular List") ;
    Tin->cListP = (DTM_CIR_LIST *) malloc( Tin->cListPtr * sizeof(DTM_CIR_LIST)) ;
    if( Tin->cListP == NULL ) 
      {  
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    if( bcdtmStream_fread(Tin->cListP,sizeof(DTM_CIR_LIST)*Tin->cListPtr,1,dtmStreamP) != 1 ) 
      { 
       bcdtmWrite_message(1,0,0,"Error Reading Tin File") ;
       goto errexit ;
      }
/*
** Read Feature Table Array
*/
    if( Tin->memFeatureTable > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Ver 400 Feature Table") ;
       Tin->fTableP = (DTM_FEATURE_TABLE *) malloc( Tin->memFeatureTable * sizeof(DTM_FEATURE_TABLE)) ;
       if( Tin->fTableP == NULL ) 
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       for( pf = Tin->fTableP ; pf < Tin->fTableP + Tin->memFeatureTable ; ++pf )
         {
          if( bcdtmStream_fread(&Tinf,sizeof(struct Tinof),1,dtmStreamP) != 1 ) 
            { 
             bcdtmWrite_message(1,0,0,"Error Reading Tin File") ;
             goto errexit ;
            }
          pf->firstPnt = Tinf.firstPnt ;
          pf->dtmFeatureType = Tinf.TYPE ;
          pf->userTag  = ( DTMUserTag ) Tinf.UTAG ;
          pf->userGuid = nullGuid ;
          pf->internalToFeature = DTM_NULL_PNT ;
          if( pf->userTag == (DTMUserTag ) Tin->nullPnt ) pf->userTag = DTM_NULL_USER_TAG ;
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Feature[%8ld] ** Type = %4ld Tag = %10I64d firstPnt = %8ld",(long)( pf-Tin->fTableP),pf->dtmFeatureType,pf->userTag,pf->firstPnt) ;
         }
      }
/*
**  Read Feature List Array
*/
    if( Tin->memFeatureList > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Ver 400 Feature List") ;
       Tin->fListP = (DTM_FEATURE_LIST_VER200 *) malloc( Tin->memFeatureList * sizeof(DTM_FEATURE_LIST)) ;
       if( Tin->fListP == NULL ) 
         {  
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
          goto errexit ; 
         }
       if( bcdtmStream_fread(Tin->fListP,sizeof(DTM_FEATURE_LIST_VER200)*Tin->memFeatureList,1,dtmStreamP) != 1 )
         { 
          bcdtmWrite_message(1,0,0,"Error Reading Tin File") ; 
          goto errexit ; 
         }
      }
   }
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
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmReadStream_atFilePositionVer500TinObject
(
 DTM_TIN_OBJ *tinP,
 struct      Bentley::TerrainModel::IBcDtmStream* dtmStreamP,
 long        FilePosition
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
**  Ver 500 Tin Object
*/
struct ver500Pts       {  double x,y,z ; unsigned short  PRGN,PCWD ;long FPTR,LPTR,TPTR,SPTR,NPTR ; } ver500Pnt ;
struct ver500CList     { long  PTNO,NEXT ; } ;
struct ver500FTable    { long  FPNT; DTMFeatureType TYPE; long INTERNALTO ; Int64 UTAG ; } ;
struct ver500FList     { long  PTNO,FEAT,NEXT ; } ;
struct ver500TinObject
{
 long    dtmFileType,dtmFileVersion ;
 double  TXMIN,TYMIN,TZMIN,TXMAX,TYMAX,TZMAX,TXDIF,TYDIF,TZDIF,PLTOL,PPTOL ;
 long    NT,NL,LPNT,LFP,SmemPolyPts,ImemPolyPts,MLINI,MLINC,MFINI,MFINC,NDD,NCLPTR,NPTVAL ;
 long    NDP,MNDP,Ndp,CLNP,CLDP,CLDPN,NFEAT,MFEAT,NLIST,MLIST,LISTDP ;
 long    refCount,UserStatus ;
 long    CreationTime,ModifiedTime,UserTime ;
 Int64 NTGVAL ;
 char    UserName[DTM_FILE_SIZE] ;
 char    TinObjectFile[DTM_FILE_SIZE] ;
 char    UserMessage[256] ;
 long    SL1,SL2,SL3,SL4,SL5 ;
 Int64 SI641,SI642,SI643,SI644,SI645 ;
 double  MPPTOL,SD2,SD3,SD4,SD5 ; 
 void    *SP1,*SP2,*SP3,*SP4,*SP5 ;
 struct ver500Pts      *DPTR ;
 struct ver500CList    *CPTR ;
 struct ver500FTable   *FPTR ;
 struct ver500FList    *LPTR ;
} ;
struct ver500TinObject tinObj ;
struct ver500FTable    fTable ;
DTM_FEATURE_TABLE      *ftP   ;
DTM_TIN_POINT  *pp ;
DTM_TIN_NODE   *np ;
long headerSize ;
DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Reading Version 500 Geopak Tin Object At File Position %9ld",FilePosition) ;
    bcdtmWrite_message(0,0,0,"tinP         = %p",tinP) ;
    bcdtmWrite_message(0,0,0,"dtmStreamP   = %p",dtmStreamP) ;
    bcdtmWrite_message(0,0,0,"FilePosition = %8ld",FilePosition) ;
   }
/*
** Check For Non Null File Pointer
*/
 if( dtmStreamP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Null Stream Pointer") ;
    goto errexit ;
   }
/*
** Position File Pointer At File Position
*/
 if( bcdtmStream_fseek(dtmStreamP,FilePosition,SEEK_SET) ) 
   { 
    bcdtmWrite_message(1,0,0,"File Seek Error") ; 
    goto errexit ;
   }
/*
** Set Header Size
*/
 headerSize = sizeof(struct ver500TinObject) ;
 #ifndef _M_IX86
 headerSize =  offsetof(struct ver500TinObject,SP1) + 40 ;
 #endif ;
/*
** Read Tin Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm Header         ** Header Size = %9ld",headerSize) ;
 if( bcdtmStream_fread(&tinObj,headerSize,1,dtmStreamP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
    goto errexit ; 
   }
/*
** Check Bentley Civil Object Type
*/
 if( tinObj.dtmFileType != DTM_TIN_TYPE )
   { 
    bcdtmWrite_message(1,0,0,"Not A Bentley Civil Dtm Tin Object") ; 
    goto errexit ; 
   }
/*
** Check File Type Version
*/
 if( tinObj.dtmFileVersion != 500 )
   { 
    bcdtmWrite_message(1,0,0,"Not A Bentley Civil Version 500 Tin Object") ; 
    goto errexit ; 
   }
/*
** Set Tin Header Variables
*/
 tinP->dtmFileType        = DTM_TIN_TYPE  ;
 tinP->dtmFileVersion     = DTM_TIN_FILE_VERSION ;
 tinP->xMin               = tinObj.TXMIN  ;
 tinP->yMin               = tinObj.TYMIN  ;
 tinP->zMin               = tinObj.TZMIN  ;
 tinP->xMax               = tinObj.TXMAX  ;
 tinP->yMax               = tinObj.TYMAX  ;
 tinP->zMax               = tinObj.TZMAX  ;
 tinP->xRange             = tinObj.TXDIF  ;
 tinP->yRange             = tinObj.TYDIF  ;
 tinP->zRange             = tinObj.TZDIF  ;
 tinP->ppTol              = tinObj.PPTOL  ;
 tinP->plTol              = tinObj.PLTOL  ;
 tinP->mppTol             = tinObj.MPPTOL ;
 tinP->numDecDigits       = tinObj.NDD    ;
 tinP->numPts             = tinObj.NDP    ;
 tinP->numSortedPts       = tinObj.Ndp    ;
 tinP->memPts             = tinObj.MNDP   ;
 tinP->numTriangles       = tinObj.NT     ;
 tinP->numLines           = tinObj.NL     ;
 tinP->numFeatureTable    = tinObj.NFEAT  ;
 tinP->memFeatureTable    = tinObj.MFEAT  ;
 tinP->numFeatureList     = tinObj.NLIST  ;
 tinP->memFeatureList     = tinObj.MLIST  ;
 tinP->hullPnt            = tinObj.LPNT   ;
 tinP->nextHullPnt        = tinObj.LFP    ;
 tinP->cListPtr           = tinObj.CLNP   ;
 tinP->cListDelPtr        = tinObj.CLDP   ;
 tinP->cListLastDelPtr    = tinObj.CLDPN  ;
 tinP->featureListDelPtr  = tinObj.LISTDP ;
 tinP->nullPnt            = tinObj.NPTVAL ;
 tinP->nullPtr            = tinObj.NCLPTR ;
 tinP->nullUserTag        = tinObj.NTGVAL ;
 tinP->nullGuid           = nullGuid      ; 
 tinP->iniMemPts          = tinObj.SmemPolyPts  ;
 tinP->incMemPts          = tinObj.ImemPolyPts  ;
 tinP->iniMemFeatureTable = tinObj.MFINI  ;
 tinP->incMemFeatureTable = tinObj.MFINC  ;
 tinP->iniMemFeatureList  = tinObj.MLINI  ;
 tinP->incMemFeatureList  = tinObj.MLINC  ;
 tinP->creationTime       = tinObj.CreationTime ;
 tinP->modifiedTime       = tinObj.ModifiedTime ;
 tinP->userTime           = tinObj.UserTime ;
 tinP->refCount           = tinObj.refCount ;
 tinP->userStatus         = tinObj.UserStatus ;
 tinP->pointsP            = NULL ;
 tinP->nodesP             = NULL ;
 tinP->cListP             = NULL ;
 tinP->fTableP            = NULL ;
 tinP->fListP             = NULL ;
 tinP->fMapP              = NULL ;
 strcpy(tinP->userName,tinObj.UserName) ;
 strcpy(tinP->tinObjectFileName,tinObj.TinObjectFile) ;
 strcpy(tinP->userMessage,tinObj.UserMessage) ;
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"NumPoints   = %10ld MemPoints   = %10ld",tinObj.NDP,tinObj.MNDP) ;
    bcdtmWrite_message(0,0,0,"NumFeatures = %10ld MemFeatures = %10ld",tinObj.NFEAT,tinObj.MFEAT) ;
    bcdtmWrite_message(0,0,0,"NumFList    = %10ld MemFList    = %10ld",tinObj.NLIST,tinObj.MLIST) ;
    bcdtmWrite_message(0,0,0,"NumCList    = %10ld",tinObj.CLNP) ;
    bcdtmWrite_message(0,0,0,"xMin = %12.5lf xMax = %12.5lf xRange = %12.5lf",tinObj.TXMIN,tinObj.TXMAX,tinObj.TXDIF) ;
    bcdtmWrite_message(0,0,0,"yMin = %12.5lf yMax = %12.5lf yRange = %12.5lf",tinObj.TYMIN,tinObj.TYMAX,tinObj.TYDIF) ;
    bcdtmWrite_message(0,0,0,"zMin = %12.5lf zMax = %12.5lf zRange = %12.5lf",tinObj.TZMIN,tinObj.TZMAX,tinObj.TZDIF) ;
    bcdtmWrite_message(0,0,0,"tinObj.MPPTOL = %16.14lf",tinObj.MPPTOL) ;
    bcdtmWrite_message(0,0,0,"tinObj.NPTVAL = %16ld",tinObj.NPTVAL) ;
    bcdtmWrite_message(0,0,0,"tinObj.NCLPTR = %16ld",tinObj.NCLPTR) ;
    bcdtmWrite_message(0,0,0,"tinObj.LPNT   = %16ld",tinObj.LPNT) ;
    bcdtmWrite_message(0,0,0,"tinObj.LFP    = %16ld",tinObj.LFP) ;
   } 
/*
** Read Tin Object Data If Tin Object Populated
*/
 if( tinP->numPts == 0 )
   {
    tinP->pointsP = NULL ;
    tinP->nodesP  = NULL ;
    tinP->cListP  = NULL ;
    tinP->fListP  = NULL ;
    tinP->fTableP = NULL ;
    tinP->numPts = tinP->memPts = tinP->numFeatureTable = tinP->memFeatureTable = tinP->numFeatureList = tinP->memFeatureList = tinP->cListPtr = 0 ;
   }
 else
   {
/*
**  Read Data Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Data Array") ;
    tinP->pointsP = (DTM_TIN_POINT *) malloc( tinP->memPts * sizeof(DTM_TIN_POINT)) ;
    if( tinP->pointsP == NULL )
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    tinP->nodesP = (DTM_TIN_NODE *) malloc( tinP->memPts * sizeof(DTM_TIN_POINT)) ;
    if( tinP->nodesP == NULL ) 
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
    for( np = tinP->nodesP , pp = tinP->pointsP ; np < tinP->nodesP + tinP->memPts ; ++pp , ++np )
      {
       if( bcdtmStream_fread(&ver500Pnt,sizeof(struct ver500Pts),1,dtmStreamP) != 1 ) 
         { 
          bcdtmWrite_message(1,0,0,"Error Reading Tin File %s ",tinP->tinObjectFileName) ;
          goto errexit ;  
         }
       pp->x = ver500Pnt.x ;
       pp->y = ver500Pnt.y ;
       pp->z = ver500Pnt.z ;
       np->PRGN = ver500Pnt.PRGN ;
       np->PCWD = ver500Pnt.PCWD ;
       np->cPtr = ver500Pnt.LPTR ;
       np->hPtr = ver500Pnt.FPTR ;
       np->tPtr = ver500Pnt.TPTR ;
       np->fPtr = ver500Pnt.NPTR ;
       np->sPtr = ver500Pnt.SPTR ;
      }
/*
**  Check For Defect Where Next Hull Point Is Not Set
*/
    if( tinObj.LFP == tinObj.NPTVAL )
      {
       tinP->nextHullPnt = (tinP->nodesP+tinObj.LPNT)->hPtr ;
      }
/*
**  Read Circular Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Circular List") ;
    tinP->cListP = (DTM_CIR_LIST *) malloc( tinP->cListPtr * sizeof(DTM_CIR_LIST)) ;
    if( tinP->cListP == NULL ) 
      {  
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ; 
      }
    if( bcdtmStream_fread(tinP->cListP,sizeof(DTM_CIR_LIST)*tinP->cListPtr,1,dtmStreamP) != 1 ) 
      { 
       bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
       goto errexit ;
      }
/*
**  Read Feature Table Array
*/
    if( tinP->memFeatureTable > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Feature Table") ;
       tinP->fTableP = (DTM_FEATURE_TABLE *) malloc( tinP->memFeatureTable * sizeof(DTM_FEATURE_TABLE)) ;
       if( tinP->fTableP == NULL )
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
          goto errexit ; 
         }
       for( ftP = tinP->fTableP ; ftP < tinP->fTableP + tinP->memFeatureTable ; ++ftP )
         { 
          if( bcdtmStream_fread(&fTable,sizeof(struct ver500FTable),1,dtmStreamP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
             goto errexit ; 
            }
          else
            {
             ftP->firstPnt          = fTable.FPNT ;
             ftP->dtmFeatureType    = fTable.TYPE ;
             ftP->userTag           = fTable.UTAG ;
             ftP->userGuid          = tinP->nullGuid    ;
             ftP->internalToFeature = fTable.INTERNALTO ;
            } 
         }
      }
/*
** Read Feature List Array
*/
    if( tinP->memFeatureList > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Feature List") ;
       tinP->fListP = ( DTM_FEATURE_LIST_VER200 *) malloc( tinP->memFeatureList * sizeof(DTM_FEATURE_LIST_VER200)) ;
       if( tinP->fListP == NULL ) 
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ; 
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Feature List") ;
       if( bcdtmStream_fread(tinP->fListP,sizeof(DTM_FEATURE_LIST_VER200)*tinP->memFeatureList,1,dtmStreamP) != 1 ) 
         { 
          bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
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
BENTLEYDTM_Public int bcdtmReadStream_atFilePositionVer501TinObject
(
 DTM_TIN_OBJ *tinP,
 struct      Bentley::TerrainModel::IBcDtmStream* dtmStreamP,
 long        filePosition
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long headerSize,point ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Reading Geopak Ver 501 Tin Object At File Position") ;
    bcdtmWrite_message(0,0,0,"tinP         = %p",tinP) ;
    bcdtmWrite_message(0,0,0,"dtmStreamP   = %p",dtmStreamP) ;
    bcdtmWrite_message(0,0,0,"filePosition = %8ld",filePosition) ;
   }
/*
** Check For Non Null File Pointer
*/
 if( dtmStreamP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Null Stream Pointer") ;
    goto errexit ;
   }
/*
** Position File Pointer At File Position
*/
 if( bcdtmStream_fseek(dtmStreamP,filePosition,SEEK_SET) ) 
   { 
    bcdtmWrite_message(1,0,0,"File Seek Error") ; 
    goto errexit ;
   }
/*
** Read Tin Header
*/
 headerSize = sizeof(DTM_TIN_OBJ) ;
 #ifndef _M_IX86
 headerSize = headerSize - 40 ;
 #endif ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tin Header         ** Header Size = %9ld",headerSize) ;
 if( bcdtmStream_fread(tinP,headerSize,1,dtmStreamP) != 1 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
    goto errexit ;
   }
/*
** Check For Bentley Civil Tin File
*/
 if( tinP->dtmFileType != DTM_TIN_TYPE ) 
   { 
    bcdtmWrite_message(1,0,0,"Not A Bentley Geopak Tin File") ;
    goto errexit ; 
   }
/*
** Test For 501 Version Of Tin File
*/
 if( tinP->dtmFileVersion != 501 ) 
   { 
    bcdtmWrite_message(1,0,0,"Not Version 501 Tin Object") ;
    goto errexit ;  
   }
/*
** Read Tin Object Data If Tin Object Populated
*/
 if( tinP->numPts == 0 )
   {
    tinP->pointsP = NULL ;
    tinP->nodesP  = NULL ;
    tinP->cListP  = NULL ;
    tinP->fListP  = NULL ;
    tinP->fTableP = NULL ;
    tinP->fMapP   = NULL ;
    tinP->numPts = tinP->memPts = tinP->numFeatureTable = tinP->memFeatureTable = tinP->numFeatureList = tinP->memFeatureList = tinP->cListPtr = 0 ;
   }
 else
   {
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"tinP->numPts      = %8ld tinP->memPts      = %8ld",tinP->numPts,tinP->memPts) ;
       bcdtmWrite_message(0,0,0,"tinP->numFeatures = %8ld tinP->memFeatures = %8ld",tinP->numFeatureTable,tinP->memFeatureTable) ;
       bcdtmWrite_message(0,0,0,"tinP->numList     = %8ld tinP->memList     = %8ld",tinP->numFeatureList,tinP->memFeatureList) ;
       bcdtmWrite_message(0,0,0,"tinP->cListPtr    = %8ld",tinP->cListPtr) ;
      } 

//  Null Out Pointers In Header
      
    tinP->pointsP = NULL ;
    tinP->nodesP  = NULL ;
    tinP->cListP  = NULL ;
    tinP->fListP  = NULL ;
    tinP->fTableP = NULL ;
    tinP->fMapP   = NULL ;
/*
**  Read Points Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tin Points Array ** memPts = %8ld",tinP->memPts) ;
    tinP->pointsP = (DTM_TIN_POINT *) malloc( tinP->memPts * sizeof(DTM_TIN_POINT)) ;
    if( tinP->pointsP == NULL )
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    if( bcdtmStream_fread(tinP->pointsP,sizeof(DTM_TIN_POINT)*tinP->memPts,1,dtmStreamP) != 1 ) 
      { 
       bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
       goto errexit ;
      }
    if( dbg )
      {
       for( point = 0 ; point < 10 ; ++point )
         {
          bcdtmWrite_message(0,0,0,"Point[%8ld] = %12.5lf %12.5lf %12.5lf",point,(tinP->pointsP+point)->x,(tinP->pointsP+point)->y,(tinP->pointsP+point)->z) ;
         }
       for( point = tinP->numPts - 10  ; point < tinP->numPts ; ++point )
         {
          bcdtmWrite_message(0,0,0,"Point[%8ld] = %12.5lf %12.5lf %12.5lf",point,(tinP->pointsP+point)->x,(tinP->pointsP+point)->y,(tinP->pointsP+point)->z) ;
         }
      }  
/*
**  Read Nodes Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tin Nodes Array  ** memPts = %8ld",tinP->memPts) ;
    tinP->nodesP = (DTM_TIN_NODE *) malloc( tinP->memPts * sizeof(DTM_TIN_NODE)) ;
    if( tinP->nodesP == NULL )
      { 
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    if( bcdtmStream_fread(tinP->nodesP,sizeof(DTM_TIN_POINT)*tinP->memPts,1,dtmStreamP) != 1 ) 
      { 
       bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
       goto errexit ;
      }
    if( dbg )
      {
       for( point = 0 ; point < 10 ; ++point )
         {
          bcdtmWrite_message(0,0,0,"Node[%8ld] ** fPtr = %10ld hPtr = %10ld",point,(tinP->nodesP+point)->fPtr,(tinP->nodesP+point)->hPtr) ;
         }
       for( point = tinP->numPts - 10  ; point < tinP->numPts ; ++point )
         {
          bcdtmWrite_message(0,0,0,"Node[%8ld] ** fPtr = %10ld hPtr = %10ld",point,(tinP->nodesP+point)->fPtr,(tinP->nodesP+point)->hPtr) ;
         }
      }   
/*
**  Read Circular Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tin Circular List Array ** cListPtr = %8ld",tinP->cListPtr) ;
    tinP->cListP = (DTM_CIR_LIST *) malloc( tinP->cListPtr * sizeof(DTM_CIR_LIST)) ;
    if( tinP->cListP == NULL ) 
      {  
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Size Of Circular List = %10ld",sizeof(DTM_CIR_LIST)*tinP->cListPtr) ;  
    if( bcdtmStream_fread(tinP->cListP,sizeof(DTM_CIR_LIST)*tinP->cListPtr,1,dtmStreamP) != 1 )
      { 
       bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
       goto errexit ;
      }
/*
**  Read Feature Table Array
*/
    if( tinP->memFeatureTable > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tin Feature Table Array  ** memFeatures = %8ld",tinP->memFeatureTable) ;
       tinP->fTableP = (DTM_FEATURE_TABLE *) malloc( tinP->memFeatureTable * sizeof(DTM_FEATURE_TABLE)) ;
       if( tinP->fTableP == NULL )  
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       if( bcdtmStream_fread(tinP->fTableP,sizeof(DTM_FEATURE_TABLE)*tinP->memFeatureTable,1,dtmStreamP) != 1 ) 
         { 
          bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
          goto errexit ;
         }
      }
    else tinP->fTableP = NULL ;
/*
**  Read Feature List Array
*/
    if( tinP->memFeatureList > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tin Feature List Array ** memFeatureList = %8ld",tinP->memFeatureList) ;
       tinP->fListP = (DTM_FEATURE_LIST_VER200 *) malloc( tinP->memFeatureList * sizeof(DTM_FEATURE_LIST)) ;
       if( tinP->fListP == NULL ) 
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
       if( bcdtmStream_fread(tinP->fListP,sizeof(DTM_FEATURE_LIST_VER200)*tinP->memFeatureList,1,dtmStreamP) != 1 ) 
         { 
          bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ;
          goto errexit ;
         }
      }
    else tinP->fListP = NULL ;
/*
**  Read Feature Map Array
*/
    if( tinP->memFeatureMap > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tin Feature Map Array") ;
/*
       tinP->fMapP = ( DTM_FEATURE_MAP * ) malloc( tinP->memFeatureMap * sizeof(DTM_FEATURE_MAP)) ;
       if( tinP->fMapP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
       if( bcdtmStream_fread(tinP->fMapP,sizeof(DTM_FEATURE_MAP)*tinP->memFeatureMap,1,dtmStreamP) != 1 ) { bcdtmWrite_message(1,0,0,"Error Reading Tin Object") ; goto errexit ; }
*/
       bcdtmStream_fseek(dtmStreamP,sizeof(DTM_FEATURE_MAP)*tinP->memFeatureMap,SEEK_CUR) ;
       tinP->memFeatureMap = 0 ;
       tinP->numFeatureMap = 0 ;
      }
    tinP->fMapP = NULL ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading At File Position Ver 501 Tin Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading At File Position Ver 501 Tin Object Error") ;
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
|   bcdtmTin_compactFeatureListTinObject                               |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTin_compactFeatureListTinObject(DTM_TIN_OBJ *Tin)
/*
** This routine Compacts the Feature List
*/
{
 int    dbg=DTM_TRACE_VALUE(0) ;
 long   cl1,cl2,cnt,*Ist,*sp ;
 DTM_TIN_NODE  *pd ;
/*
 DTM_FEATURE_LIST  *pl ;
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature List") ;
/*
**
*/
 if( Tin->fListP  == NULL ) { Tin->numFeatureList = Tin->memFeatureList = 0 ; return(0) ; }
 if( Tin->numFeatureList == 0    ) { Tin->memFeatureList = 0 ; free(Tin->fListP) ; Tin->fListP = NULL ; return(0) ; }
/*
** Look For Deleted Points Not In Delete Pointer List And Add To Delete Point List
*/
/*
 cnt = 0 ;
 cl1 = cl2 = Tin->featureListDelPtr ;
 while ( cl2 != Tin->nullPtr ) { cl1 = cl2 ; cl2 = (Tin->fListP+cl2)->nextPtr ; }
 for( pl = Tin->fListP ; pl < Tin->fListP + Tin->numFeatureList ; ++pl )
   {
    if( pl->pntNum == Tin->nullPnt )
      {
       bcdtmWrite_message(0,0,0,"[%6ld]  pntNum = %6ld FEAT = %6ld nextPtr = %6ld",(long)(pl-Tin->fListP),pl->pntNum,pl->dtmFeature,pl->nextPtr) ;
       if( Tin->featureListDelPtr == Tin->nullPtr ) Tin->featureListDelPtr = (long)(pl-Tin->fListP) ;
       else                             (Tin->fListP+cl1)->nextPtr = (long)(pl-Tin->fListP) ;
       pl->nextPtr = Tin->nullPtr ;
       cl1 = (long)(pl-Tin->fListP) ;
       ++cnt ;
      }
   }
 bcdtmWrite_message(0,0,0,"Number Of Deleted Entries Not In Delete List = %6ld",cnt) ;
*/
/*
** Check If Feature List Can be Compacted
*/
 if( Tin->featureListDelPtr != Tin->nullPtr )
   {
    cl1 = Tin->featureListDelPtr ; cnt = 0 ;
/*
** Allocate memory For Counts
*/
    Ist = ( long * ) malloc(Tin->numFeatureList * sizeof(long)) ;
    if( Ist == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return(1) ; }
    for( sp = Ist ; sp < Ist + Tin->numFeatureList ; ++sp ) *sp = 0 ;
/*
** Mark All Deleted Records
*/
    while( cl1 != Tin->nullPtr )
      { *(Ist+cl1) = 1 ; cl1 = (Tin->fListP+cl1)->nextPtr ; ++cnt ; }
/*
** Copy Over Deleted Records
*/
    for( cl1 = cl2 = 0 ; cl2 < Tin->numFeatureList ; ++cl2 )
      {
       if( ! *(Ist+cl2) )
         {
          if( cl1 != cl2 )  *(Tin->fListP+cl1) = *(Tin->fListP+cl2)  ;
          ++cl1 ;
         }
      }
/*
** Adjust Pointers
*/
    for( sp = Ist + 1 ; sp < Ist + Tin->numFeatureList ; ++sp ) *sp = *sp + *(sp-1) ;
    for( cl1 = 0 ; cl1 < Tin->numFeatureList ; ++cl1 )
      {
       cl2 = (Tin->fListP+cl1)->nextPtr ;
       if( cl2 != Tin->nullPtr ) (Tin->fListP+cl1)->nextPtr -= (long)*(Ist+cl2) ;
      }
    Tin->numFeatureList  = Tin->numFeatureList - cnt ;
    Tin->featureListDelPtr = Tin->nullPtr ;
/*
** Adjust Feature List Pointers
*/
   for( pd = Tin->nodesP ; pd < Tin->nodesP + Tin->numPts ; ++pd )
      if( pd->fPtr != Tin->nullPtr ) pd->fPtr -= (long)*(Ist+pd->fPtr) ;
/*
** Free Memory
*/
   free(Ist) ; Ist = NULL ;
  }
/*
** Realloc Memory
*/
 if( Tin->memFeatureList != Tin->numFeatureList )
   {
    Tin->memFeatureList =  Tin->numFeatureList ;
    Tin->fListP  = ( DTM_FEATURE_LIST_VER200 * ) realloc(Tin->fListP,Tin->memFeatureList*sizeof(DTM_FEATURE_LIST)) ;
   }
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature List Completed") ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmTin_compactFeatureTableTinObject(DTM_TIN_OBJ *tinP)
/*
** This Function Compacts the Feature Table. 
** It Removes Deleted Dtm Features
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   listOfs1,listOfs2,numDeletedFeatures,*offsetP=NULL,*ofsP ;
 DTM_FEATURE_TABLE  *fP ;
 DTM_FEATURE_LIST_VER200  *lP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature Table") ;
/*
** Check Integrity Of Feature Table
*/
 if( tinP->fTableP  == NULL && tinP->numFeatureTable > 0 )
   {
    bcdtmWrite_message(1,0,0,"Dtm Feature Table Corrupted") ;
    goto errexit ;
   }
/*
** Only Process If Dtm Features Exist
*/
 if( tinP->fTableP  != NULL && tinP->numFeatureTable > 0 ) 
   {
/*
** Check Dtm Feature End Points
*/
    if( cdbg ) 
      {
       bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points Before Compacting Feature Table") ;
       if( bcdtmCheck_dtmFeatureEndPointsTinObject(tinP,0))
         {
          bcdtmWrite_message(0,0,0,"Tin Dtm Feature End Point Errors") ;
          goto errexit  ;
         }
      }
/*
**  Count Number Of Deleted Features
*/
    numDeletedFeatures = 0 ;
    for( fP = tinP->fTableP ; fP < tinP->fTableP + tinP->numFeatureTable ; ++fP ) if( fP->firstPnt == tinP->nullPnt ) ++numDeletedFeatures ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Deleted Features = %6ld",numDeletedFeatures) ;
/*
**  Compact Feature Table If Deleted Features Exist
*/
    if( numDeletedFeatures > 0 )
      {
/*
** Allocate Memory For Dtm Feature Offsets
*/
       offsetP = ( long * ) malloc(tinP->numFeatureTable * sizeof(long)) ;
       if( offsetP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ;  }
       for( ofsP = offsetP ; ofsP < offsetP + tinP->numFeatureTable ; ++ofsP ) *ofsP = 0 ;
/*
** Mark All Deleted Records
*/
       for( fP = tinP->fTableP ; fP < tinP->fTableP + tinP->numFeatureTable ; ++fP )
         {
          if( fP->firstPnt == tinP->nullPnt )  *(offsetP+(long)(fP-tinP->fTableP)) = 1 ;
         }
/*
** Copy Over Deleted Records
*/
       for( listOfs1 = listOfs2 = 0 ; listOfs2 < tinP->numFeatureTable ; ++listOfs2 )
         {
          if( ! *(offsetP+listOfs2) )
            {
             if( listOfs1 != listOfs2 )*(tinP->fTableP+listOfs1) = *(tinP->fTableP+listOfs2) ;
             ++listOfs1 ;
            }
         }
/*
** Adjust Pointers
*/
       for( ofsP = offsetP + 1 ; ofsP < offsetP + tinP->numFeatureTable ; ++ofsP ) *ofsP = *ofsP + *(ofsP-1) ;
       tinP->numFeatureTable  = tinP->numFeatureTable - numDeletedFeatures ;
/*
** Adjust Feature List Feature Numbers
*/
       for( lP = tinP->fListP ; lP < tinP->fListP + tinP->numFeatureList ; ++lP )
         {
          if( lP->dtmFeature != tinP->nullPnt ) lP->dtmFeature -= (long)*(offsetP+lP->dtmFeature) ;
         }
/*
** Resize Feature List Memory
*/
       if( tinP->memFeatureTable != tinP->numFeatureTable )
         {
          tinP->memFeatureTable = tinP->numFeatureTable ;
          if( tinP->memFeatureTable > 0 ) tinP->fTableP  = (DTM_FEATURE_TABLE *) realloc(tinP->fTableP,tinP->memFeatureTable*sizeof(DTM_FEATURE_TABLE)) ;
          else
            { 
             free(tinP->fTableP) ; 
             tinP->fTableP = NULL ; 
            }
         }
      }
   }
/*
** Modify Tin Feature Variables If No Features Exist
*/
 if( tinP->fTableP  == NULL ) tinP->numFeatureTable = tinP->memFeatureTable = 0 ; 
 if( tinP->numFeatureTable == 0    ) 
   { 
    tinP->memFeatureTable = 0 ;
    if( tinP->fTableP != NULL ) { free(tinP->fTableP) ; tinP->fTableP = NULL ;  }
   }
/*
** Check Dtm Feature End Points
*/
 if( cdbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points After Compacting") ;
    if( bcdtmCheck_dtmFeatureEndPointsTinObject(tinP,0))
      {
       bcdtmWrite_message(0,0,0,"Tin Dtm Feature End Point Errors") ;
       goto errexit  ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( offsetP != NULL ) free(offsetP) ;
/*
** Job ComlPeted
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Compacting Feature Table Success") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Compacting Feature Table Error") ;
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
BENTLEYDTM_EXPORT int bcdtmTin_markInternalVoidPointsTinObject(DTM_TIN_OBJ *tinP)
/*
** This Functions Marks Void Points In The Tin
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  islandFeature,voidFeature ;
 DTM_FEATURE_TABLE *fP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points") ;
/*
** Scan Island Features And Check That They Have A Surrounding Void
*/
 for( fP = tinP->fTableP ; fP < tinP->fTableP + tinP->numFeatureTable ; ++fP )
   {
    if( fP->firstPnt != tinP->nullPnt && fP->dtmFeatureType == DTMFeatureType::Island )
      {
       islandFeature = (long)(fP-tinP->fTableP) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Finding Surrounding Void For Island Feature %6ld",islandFeature) ;
       if( bcdtmList_getVoidExternalToIslandTinObject(tinP,islandFeature,&voidFeature)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Void %6ld Surrounds Island %6ld",voidFeature,islandFeature) ;
/*
** If No Surrounding Void Set Island Feature To A Break Line
*/ 
       if( voidFeature == tinP->nullPnt ) 
         {
          (tinP->fTableP+islandFeature)->dtmFeatureType = DTMFeatureType::Breakline ;
         }
      }
   }
/*
** Mark Void Points 
*/
 if( bcdtmMark_voidPointsTinObject(tinP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Void Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Void Points Error") ;
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
BENTLEYDTM_Public int bcdtmInsert_addDtmFeatureToTinObject(DTM_TIN_OBJ *tinP,DTMFeatureType dtmFeatureType,DTMUserTag userTag,DTM_GUID userGuid,long startPoint,long clearFlag)
/*
** This Function Adds A DTM Feature To The Tin Object
** Assumes Feature Points Are Stored In The tPtr Array
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ; 
 long  np,sp,tableEntry=0 ; 
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Tin") ;
    bcdtmList_writeTptrListTinObject(tinP,startPoint) ;
   }
/*
** Check For Old UserTag Null Value And Set To New Null Value If Necessary
*/
 if( userTag == tinP->nullPnt ) userTag = tinP->nullUserTag ;
/*
** Initialise
*/
 sp = startPoint ; 
 np = tinP->nullPnt ;
/*
** Check For More Than One Point In Feature
*/
 if( dtmFeatureType != DTMFeatureType::GroupSpots && (tinP->nodesP+startPoint)->tPtr == tinP->nullPnt )  goto cleanup ;
/*
** Add Entry To Feature Table
*/
 if( bcdtmInsert_addToFeatureTableTinObject(tinP,dtmFeatureType,userTag,userGuid,startPoint,&tableEntry)) goto errexit  ;
/*
** Add Entry To Feature List
*/ 
 if( bcdtmInsert_addToFeatureListTinObject(tinP,tableEntry,startPoint,clearFlag)) goto errexit  ;
/*
** Write Points For Added Feature
*/
 if( dbg ) bcdtmList_writePointsForDtmFeatureTinObject(tinP,tinP->numFeatureTable-1) ;
/*
** Check Dtm Feature End Points To Ensure Modified List Structure Is Correct
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points After Adding Dtm Feature %6ld",tinP->numFeatureTable-1) ;
    if( bcdtmCheck_dtmFeatureEndPointsTinObject(tinP,cdbg))
      {
       bcdtmWrite_message(0,0,0,"Tin Dtm Feature End Point Errors") ;
       goto errexit  ;
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
BENTLEYDTM_Public int bcdtmInsert_addToFeatureTableTinObject(DTM_TIN_OBJ *tinP,DTMFeatureType dtmFeatureType,DTMUserTag userTag,DTM_GUID userGuid,long startPoint,long *tableEntryP)
/*
** This Function Adds A New Feature Entry To The Feature Table
*/
{
 int ret=DTM_SUCCESS ;
/*
** Initialise
*/
 *tableEntryP = tinP->nullPnt ; 
/*
** Test For Memory Allocation
*/
 if( tinP->numFeatureTable >= tinP->memFeatureTable ) 
   {
    if(bcdtmObject_allocateMemoryFeatureTableTinObject(tinP)) goto errexit ;
   }
/*
** Add Entry To Feature Table
*/
 (tinP->fTableP+tinP->numFeatureTable)->firstPnt = startPoint ;
 (tinP->fTableP+tinP->numFeatureTable)->dtmFeatureType = (DTMFeatureType)dtmFeatureType ;
 (tinP->fTableP+tinP->numFeatureTable)->userTag = userTag ;
 (tinP->fTableP+tinP->numFeatureTable)->userGuid = userGuid ;
 *tableEntryP = tinP->numFeatureTable ; 
 ++tinP->numFeatureTable ;
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
BENTLEYDTM_Private int bcdtmInsert_addToFeatureListTinObject(DTM_TIN_OBJ *tinP,long tableEntry,long startPoint,long clearFlag)
/*
** This Function Adds A New Feature Entry To The Feature List
*/
{
 int   ret=DTM_SUCCESS ;
 long  sp,np,nextEntry,nptr,lastListEntry ;
 DTM_TIN_NODE *pd ;
/*
** Initialise
*/
 sp = startPoint ;
 pd = tinP->nodesP  ;
 if( (pd+sp)->tPtr == tinP->nullPnt ) goto cleanup ;
/*
** Scan tPtr List and Add Feature List Records
*/
 do
   {
/*
**  Get Next Available Entry In Feature List Table
*/
    if( tinP->featureListDelPtr != tinP->nullPtr )
      { 
       nextEntry = tinP->featureListDelPtr ; 
       tinP->featureListDelPtr = (tinP->fListP+nextEntry)->nextPtr ; 
      }
    else
      { 
       if( tinP->numFeatureList >= tinP->memFeatureList )
         {
          if(bcdtmObject_allocateMemoryFeatureListTinObject(tinP)) goto errexit  ;
         }
       nextEntry = tinP->numFeatureList ;
      } 
/*
** Add Entry To Feature Table
*/
   (tinP->fListP + nextEntry)->nextPnt = (pd+sp)->tPtr ;
   (tinP->fListP + nextEntry)->nextPtr = tinP->nullPtr ;
   (tinP->fListP + nextEntry)->dtmFeature = tableEntry ;
   if( nextEntry == tinP->numFeatureList ) ++tinP->numFeatureList ; 
/*
** Set Pointers To Feature Table
*/
   if( ( nptr = (tinP->nodesP+sp)->fPtr) == tinP->nullPtr )  
     { 
      (tinP->nodesP+sp)->fPtr = nextEntry ; 
     } 
   else 
     { 
      lastListEntry = nptr ;
      while ( lastListEntry != tinP->nullPtr ) 
        { 
         nptr = lastListEntry ; 
         lastListEntry = (tinP->fListP+nptr)->nextPtr ; 
        }
      (tinP->fListP+nptr)->nextPtr = nextEntry ;
     } 
/*
** Get Next Point
*/
   np = (pd+sp)->tPtr ;
   if( clearFlag ) (pd+sp)->tPtr = tinP->nullPnt ;
   sp = np ;
  } while( sp != tinP->nullPnt && sp != startPoint ) ;
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
/*-------------------------------------------------------------------------+
|                                                                          |
|                                                                          |
|                                                                          |
+-------------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_allocateMemoryFeatureTableTinObject(DTM_TIN_OBJ *tinP)
/*
** This Function Allocates Memory For The Tin Object Feature Table
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 DTM_FEATURE_TABLE    *fP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Incrementing Memory For Tin Feature Table") ;
    bcdtmWrite_message(0,0,0,"tinP->numFeatureTable = %8ld tinP->memFeatureTable = %8ld tinP->incMemFeatureTable = %8ld",tinP->numFeatureTable,tinP->memFeatureTable,tinP->incMemFeatureTable) ;
   }
/*
** Determine Amount Of Memory To Be Allocated
*/
 if( tinP->fTableP == NULL ) tinP->memFeatureTable = tinP->iniMemFeatureTable ;
 else                        tinP->memFeatureTable = tinP->memFeatureTable + tinP->incMemFeatureTable ;
 if( tinP->fTableP == NULL ) tinP->fTableP = ( DTM_FEATURE_TABLE * ) malloc(tinP->memFeatureTable * sizeof(DTM_FEATURE_TABLE)) ;
 else                        tinP->fTableP = ( DTM_FEATURE_TABLE * ) realloc(tinP->fTableP,tinP->memFeatureTable * sizeof(DTM_FEATURE_TABLE)) ;
 if( tinP->fTableP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Initialise Feature Values
*/
 for( fP = tinP->fTableP + tinP->numFeatureTable ; fP < tinP->fTableP + tinP->memFeatureTable ; ++fP )
   {
   fP->dtmFeatureType = (DTMFeatureType)tinP->nullPnt;
   fP->firstPnt =  fP->internalToFeature = tinP->nullPnt ;
   fP->userTag = DTM_NULL_USER_TAG ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Incrementing memory for tin feature table completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Incrementing memory for tin feature table error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------------+
|                                                                          |
|                                                                          |
|                                                                          |
+-------------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_allocateMemoryFeatureListTinObject(DTM_TIN_OBJ *tinP)
/*
** This Function Allocates Memory For The Tin Object Feature List
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 DTM_FEATURE_LIST_VER200    *lP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Incrementing memory for tin feature list") ;
    bcdtmWrite_message(0,0,0,"tinP->numFeatureList = %8ld tinP->memFeatureList = %8ld tinP->incMemFeatureList = %8ld",tinP->numFeatureList,tinP->memFeatureList,tinP->incMemFeatureList) ;
   }
/*
** Determine Amount Of Memory To Be Allocated
*/
 if( tinP->fListP == NULL ) tinP->memFeatureList = tinP->iniMemFeatureList ;
 else                       tinP->memFeatureList = tinP->memFeatureList + tinP->incMemFeatureList ;
 if( tinP->fListP == NULL ) tinP->fListP = ( DTM_FEATURE_LIST_VER200 * ) malloc(tinP->memFeatureList * sizeof(DTM_FEATURE_LIST)) ;
 else                       tinP->fListP = ( DTM_FEATURE_LIST_VER200 * ) realloc(tinP->fListP,tinP->memFeatureList * sizeof(DTM_FEATURE_LIST)) ;
 if( tinP->fListP == NULL )  
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Initialise Feature Values
*/
 for( lP = tinP->fListP + tinP->numFeatureList ; lP < tinP->fListP + tinP->memFeatureList ; ++lP )
   {
    lP->nextPtr = DTM_NULL_PTR ;
    lP->nextPnt = lP->dtmFeature = tinP->nullPnt ;
   } 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Incrementing memory for tin feature list completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Incrementing memory for tin feature list error") ;
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
BENTLEYDTM_Public int bcdtmMark_voidPointsTinObject(DTM_TIN_OBJ *Tin) 
/*
** This Function Marks Void Points
*/
{
 long  dbg=DTM_TRACE_VALUE(0) ;
 DTM_FEATURE_TABLE *pf ;
 DTM_TIN_NODE *pd ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Voids Tin Object") ;
/*
** Null Out Tptr Values And Void Bit Settings
*/
 for( pd = Tin->nodesP ; pd < Tin->nodesP + Tin->numPts ; ++pd ) { pd->tPtr = Tin->nullPnt ; pd->PCWD = 0 ; }
/*
** Scan Feature Table And Process All Void Features
*/
 for( pf = Tin->fTableP ; pf < Tin->fTableP + Tin->numFeatureTable ; ++pf )
   {
    if( ( pf->dtmFeatureType == DTMFeatureType::Void || pf->dtmFeatureType == DTMFeatureType::Hole ) && pf->firstPnt != Tin->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Void Feature = %6ld Type = %4ld",(long)(pf-Tin->fTableP),pf->dtmFeatureType) ;
       if( bcdtmMark_pointsInternalToVoidTinObject(Tin,(long)(pf-Tin->fTableP))) goto errexit ;
      }
   }
/*
** Scan Feature Table And Process All Island Features
*/
 for( pf = Tin->fTableP ; pf < Tin->fTableP + Tin->numFeatureTable ; ++pf )
   {
    if( pf->dtmFeatureType == DTMFeatureType::Island && pf->firstPnt != Tin->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points External To Island Feature = %6ld Type = %4ld",(long)(pf-Tin->fTableP),pf->dtmFeatureType) ;
       if( bcdtmMark_pointsExternalToIslandTinObject(Tin,(long)(pf-Tin->fTableP))) goto errexit ;
      }
   }
/*
** Null Out Tptr Values 
*/
 for( pd = Tin->nodesP ; pd < Tin->nodesP + Tin->numPts ; ++pd ) pd->tPtr = Tin->nullPnt ; 
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Voids Completed") ;
 return(0) ;
/*
** Error Exit
*/
 errexit :
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Voids Error") ;
 return(1) ;
} 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_reverseTptrPolygonTinObject(DTM_TIN_OBJ *Tin,long Spnt )
/*
** This Function Reverses The Direction Of A tPtr Polygon
*/
{
 long p1,p2,p3 ;
/*
** Get Prior Pointer To Spnt
*/
 p1 = p2 = Spnt ;
 while ( (Tin->nodesP+p1)->tPtr != Spnt ) p1 = (Tin->nodesP+p1)->tPtr ;
 do
   {
    p3 = (Tin->nodesP+p2)->tPtr ;
    (Tin->nodesP+p2)->tPtr = p1 ;
    p1 = p2 ; p2 = p3   ;
   } while ( p2 != Spnt ) ;
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
BENTLEYDTM_Public int bcdtmMath_calculateAreaAndDirectionTptrPolygonTinObject(DTM_TIN_OBJ *Tin,long Spnt,double *Area,DTMDirection* Direction)
/*
** This Function Calculates The Area Of A tPtr Polygon
**
** Direction = 1 Clockwise
**           = 2 Anti Clockwise
*/
{
 long   sp,np ;
 double x,y ;
/*
** Determine Polygon Area
*/
 sp = Spnt ; *Area = 0.0 ; *Direction = DTMDirection::Clockwise ;
 do
   {
    np = (Tin->nodesP+sp)->tPtr ;
    x  = (Tin->pointsP+np)->x - (Tin->pointsP+sp)->x ;
    y  = (Tin->pointsP+np)->y - (Tin->pointsP+sp)->y ;
    *Area = *Area + x * y / 2.0 + x * (Tin->pointsP+sp)->y ;
    sp = np ;
   }  while ( sp != Spnt ) ;
/*
** Set Polygon Direction
*/
 if( *Area < 0.0 ) { *Direction = DTMDirection::AntiClockwise ; *Area = -*Area ; }
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
BENTLEYDTM_Public int bcdtmMath_calculateAreaAndDirectionSptrPolygonTinObject(DTM_TIN_OBJ *Tin,long Spnt,double *Area,DTMDirection* Direction)
/*
** This Function Calculates The Area Of A sPtr Polygon
**
** Direction = 1 Clockwise
**           = 2 Anti Clockwise
*/
{
 long   sp,np ;
 double x,y ;
/*
** Determine Polygon Area
*/
 sp = Spnt ; *Area = 0.0 ; *Direction = DTMDirection::Clockwise ;
 do
   {
    np = (Tin->nodesP+sp)->sPtr ;
    x  = (Tin->pointsP+np)->x - (Tin->pointsP+sp)->x ;
    y  = (Tin->pointsP+np)->y - (Tin->pointsP+sp)->y ;
    *Area = *Area + x * y / 2.0 + x * (Tin->pointsP+sp)->y ;
    sp = np ;
   }  while ( sp != Spnt ) ;
/*
** Set Polygon Direction
*/
 if( *Area < 0.0 ) { *Direction = DTMDirection::AntiClockwise ; *Area = -*Area ; }
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
BENTLEYDTM_Public int bcdtmList_checkConnectivityTptrPolygonTinObject(DTM_TIN_OBJ *tinP,long startPnt,long messageFlag) 
/*
** This Function Checks The Connectivity Of A Tptr Polygon
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
 if( startPnt < 0 || startPnt >= tinP->numPts ) 
   {
    if( messageFlag ) bcdtmWrite_message(0,0,0,"Tptr Polygon Start Point Range Error") ;
    ret = DTM_ERROR ;  
   } 
 else
   {
    if( (tinP->nodesP+startPnt)->tPtr < 0 || (tinP->nodesP+startPnt)->tPtr >= tinP->numPts  ) 
      {
       if( messageFlag ) bcdtmWrite_message(0,0,0,"Tptr Polygon Next Point Range Error") ;
       ret = DTM_ERROR ;
      }
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
         if( messageFlag ) bcdtmWrite_message(0,0,0,"Knot At Point %8ld ** %12.5lf %12.5lf %10.4lf ",lp,(tinP->pointsP+lp)->x,(tinP->pointsP+lp)->y,(tinP->pointsP+lp)->z) ;
         ret = DTM_ERROR ;
        } 
/*
** Must Detect A Knot At The Start Point For A Polygon
*/
      if( ! knot ) 
        { 
         if( messageFlag ) bcdtmWrite_message(0,0,0,"Start Point Knot Not Detected") ;
         ret = DTM_ERROR ;  
        }
     }
  }
/*
** Write Error Message If Necessary
*/
// if( ret == DTM_ERROR ) bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
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
BENTLEYDTM_Public int bcdtmInsert_checkAndFixLastPointOfDtmFeaturesTinObject(DTM_TIN_OBJ *tinP)
/*
** This Function Checks That The Last Point Of An Open
** DTM Feature Has A Null Next Point Pointer. This Is Required For
** All Tins Created Prior To Adding The Functionality To Store User Tags
** For Spot Features. Previously The List Structure Did Not Store The Dtm 
** Feature Type For The Last Dtm Feature Point .
**
** Date 13 September, 2004  RobC 
**
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ; 
 long  firstPnt,nextPnt,listPtr,lastPnt,dtmFeature ; 
 long  pointListPtr,nextEntry,lastListEntry ;
 char  dtmFeatureTypeName[50] ;
 DTM_FEATURE_TABLE *fP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking And Fixing Last Point Of Dtm Features") ; 
/*
** Check Dtm Feature End Points
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points") ;
    if( bcdtmCheck_dtmFeatureEndPointsTinObject(tinP,cdbg))
      {
       bcdtmWrite_message(0,0,0,"Tin Dtm Feature End Point Errors") ;
       goto errexit  ;
      }
   } 
/*
** Scan Dtm Features
*/
 for( fP = tinP->fTableP ; fP < tinP->fTableP + tinP->numFeatureTable ; ++fP )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Feature %8ld",(long)(fP-tinP->fTableP)) ;
    if( fP->firstPnt != tinP->nullPnt )
      {  
/*
** Scan Dtm Feature List
*/
       firstPnt   = fP->firstPnt ;
       nextPnt    = tinP->nullPnt ;
       lastPnt    = tinP->nullPnt ; 
       dtmFeature = (long)(fP-tinP->fTableP) ;
/*
** Scan Dtm Feature Points
*/
       if( dbg == 1 ) 
         {
          bcdtmWrite_message(0,0,0,"Dtm Feature = %6ld",dtmFeature) ;
          bcdtmWrite_message(0,0,0,"First Point = %9ld ** %12.4lf %12.4lf %10.4lf",firstPnt,(tinP->pointsP+firstPnt)->x,(tinP->pointsP+firstPnt)->y,(tinP->pointsP+firstPnt)->z) ;
         }
       listPtr = (tinP->nodesP+firstPnt)->fPtr ;
       while ( listPtr != tinP->nullPtr )
         {
          while( listPtr != tinP->nullPtr  && (tinP->fListP+listPtr)->dtmFeature != dtmFeature ) listPtr = (tinP->fListP+listPtr)->nextPtr ;
          if( listPtr != tinP->nullPtr )
            {
             nextPnt = (tinP->fListP+listPtr)->nextPnt ;
             if( dbg == 1 )
               {
                if( nextPnt != tinP->nullPnt ) bcdtmWrite_message(0,0,0,"Next  Point = %9ld ** %12.4lf %12.4lf %10.4lf",nextPnt,(tinP->pointsP+nextPnt)->x,(tinP->pointsP+nextPnt)->y,(tinP->pointsP+nextPnt)->z) ; 
                else                           bcdtmWrite_message(0,0,0,"Next  Point = %9ld",nextPnt) ;
               }
             if( nextPnt != tinP->nullPnt ) 
               {
                lastPnt = nextPnt ; 
                listPtr = (tinP->nodesP+nextPnt)->fPtr ;
               }
            }
          if( nextPnt == tinP->nullPnt || nextPnt == firstPnt ) listPtr = tinP->nullPtr ;
         }  
/*
**     Check For Open Dtm Feature
*/
       if( nextPnt != firstPnt )
         {
/*
**        If Old Feature List Method Add New Entry For Last Point
*/
          if( nextPnt != tinP->nullPnt )
            {
             if( dbg == 1 )
               {
                if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType((tinP->fTableP+dtmFeature)->dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
                bcdtmWrite_message(0,0,0,"Adding Entry For Last Point Of Dtm Feature %6ld ** %s",dtmFeature,dtmFeatureTypeName) ;
               } 
/*
**           Get Next Available Entry In Feature List Table
*/
             if( tinP->featureListDelPtr != tinP->nullPtr )
               { 
                nextEntry = tinP->featureListDelPtr ; 
                tinP->featureListDelPtr = (tinP->fListP+nextEntry)->nextPtr ; 
               }
             else
               { 
                if( tinP->numFeatureList >= tinP->memFeatureList )
                  {
                   if(bcdtmObject_allocateMemoryFeatureListTinObject(tinP)) goto errexit  ;
                  }
                nextEntry = tinP->numFeatureList ;
               } 
/*
**          Add Entry To Feature List
*/
            (tinP->fListP + nextEntry)->nextPnt = tinP->nullPnt ;
            (tinP->fListP + nextEntry)->nextPtr = tinP->nullPtr ;
            (tinP->fListP + nextEntry)->dtmFeature = dtmFeature   ;
            if( nextEntry == tinP->numFeatureList ) ++tinP->numFeatureList  ; 
/*
**          Set Pointers In Point List Table
*/
            if( ( pointListPtr = (tinP->nodesP+nextPnt)->fPtr) == tinP->nullPtr )  
              { 
               (tinP->nodesP+nextPnt)->fPtr = nextEntry ; 
              } 
            else 
              { 
               lastListEntry = pointListPtr ;
               while ( lastListEntry != tinP->nullPtr ) 
                 { 
                  pointListPtr = lastListEntry ; 
                  lastListEntry = (tinP->fListP+pointListPtr)->nextPtr ; 
                 }
               (tinP->fListP+pointListPtr)->nextPtr = nextEntry ;
              } 
/*
**          Write Modified Lists
*/
            if( dbg == 1 ) 
              {
/*
**             Write Dtm Feature Points
*/
               bcdtmList_writePointsForDtmFeatureTinObject(tinP,dtmFeature) ;
/*
**             Write List Of Dtm Features For Last Point
*/
               bcdtmList_writeDtmFeaturesForPointTinObject(tinP,lastPnt) ;
              }
           }  
        }
     } 
  }
/*
** Compact Feature List
*/
 if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Compacting Feature List") ;
 if( bcdtmTin_compactFeatureListTinObject(tinP) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking And Fixing Last Point Of Dtm Features Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking And Fixing Last Point Of Dtm Features Error") ; 
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
BENTLEYDTM_Public int bcdtmNormal_calculateMachinePrecisionForTinObject(DTM_TIN_OBJ *Tin,double *Mpptol)
/*
** This Function Calculates The Machine Precision For A Tin Object
*/
{
 double  Ls,Lm,Large ;
 double lpptol=1.0 ;
 long loop=0 ;
/*
** Initialise
*/
 *Mpptol = 0.0 ;
//bcdtmWrite_message(-10,0,0,"Calculating Machine Precision") ;
/*
** Get Largest Absolute Number
*/
 Large = fabs(Tin->xMax) ;
 if( fabs(Tin->xMin) > Large )  Large = fabs(Tin->xMin) ;
 if( fabs(Tin->yMax) > Large )  Large = fabs(Tin->yMax) ;
 if( fabs(Tin->yMin) > Large )  Large = fabs(Tin->yMin) ;
//bcdtmWrite_message(-10,0,0,"Large = %20.16lf",Large) ;
/*
** Iteratively get Midpoint
*/
 Ls = Lm = 0.0 ; 
 Lm = (Lm+Large) / 2.0 ;
 while ( loop < 80 && lpptol != *Mpptol && Lm != Ls && Lm != Large ) 
   { 
    ++loop ;
    lpptol = *Mpptol ;
//bcdtmWrite_message(-10,0,0,"Loop = %4ld ** Ls = %20.16lf ** Lm = %20.16lf ** Large = %20.16lf",loop,Ls,Lm,Large) ;
    Ls = Lm ;
    Lm = ( Lm + Large ) / 2.0 ;
   *Mpptol = Large - Ls ;
   } 
/*
** Set machine Precision
*/
 *Mpptol = Large - Ls ;
// bcdtmWrite_message(-10,0,0,"Calculating Machine Precision Completed ** mppTol = %20.16lf",*Mpptol) ;
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
BENTLEYDTM_Public int bcdtmMark_pointsExternalToIslandTinObject(DTM_TIN_OBJ *Tin,long islandFeature ) 
/*
** This Function Marks Points External To An Island As Void Points.
** islandFeature Is expressed as offset from Start Of Feature Table
*/
{
 int  ret=DTM_SUCCESS ;
 long dbg=DTM_TRACE_VALUE(0),sp,spnt,numMarked ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points External To Island Feature = %6ld",islandFeature) ;
/*
** Check For Non Null Tptr Values
*/
 if( dbg )  bcdtmList_reportAndSetToNullNoneNullTptrValuesTinObject(Tin,dbg) ;
/*
** Check For Valid Island Feature
*/
 if( (Tin->fTableP+islandFeature)->dtmFeatureType == DTMFeatureType::Island && (Tin->fTableP+islandFeature)->firstPnt != Tin->nullPnt ) 
   {
/*
** Copy Feature Points To Tptr Polygon
*/
    if( bcdtmList_copyDtmFeatureToTptrListTinObject(Tin,islandFeature,&spnt)) goto errexit ;
/*
** Mark Internal Island Points
*/
    if( bcdtmMark_voidPointsExternalToTptrIslandPolygonTinObject(Tin,spnt,&numMarked)) goto errexit ; 
/*
** Replaced RobC - June 2003
    if( bcdtmMark_externalTptrPolygonPointsAsVoidPointsTinObject(Tin,spnt,&NumMarked ) ) goto errexit ;
*/
/*
** Mark Non Void Points On Island Hull
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points On Void Hull") ;
    sp = spnt ;
    do
      {
       bcdtmFlag_clearVoidBitPCWD(&(Tin->nodesP+sp)->PCWD) ;
       sp = (Tin->nodesP+sp)->tPtr ;
      } while ( sp != spnt ) ;
/*
** Null Out Tptr Polygon
*/
    if( bcdtmList_nullOutTptrListTinObject(Tin,spnt)) goto errexit ;
/*
** Check For Non Null Tptr Values
*/
    if( dbg )  bcdtmList_reportAndSetToNullNoneNullTptrValuesTinObject(Tin,dbg) ;
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
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMark_voidPointsExternalToTptrIslandPolygonTinObject(DTM_TIN_OBJ *Tin,long sPnt,long *numMarked ) 
/*
** This Function marks void points external To a tPtr Island Polygon.
**
** Rob Cormack June 2003
** 
*/
{
 int  ret=DTM_SUCCESS ;
 long dbg=DTM_TRACE_VALUE(0),npnt,ppnt,lpnt,lp,clc,pnt,fPnt,lPnt ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points External To Tptr Island Polygon") ;
/*
** Initialise
*/
 *numMarked = 0 ;
 fPnt = lPnt = Tin->nullPnt ;
/*
** Check For Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListTinObject(Tin,sPnt) ;
 if( (Tin->nodesP+sPnt)->tPtr == Tin->nullPnt ) goto errexit ;
/*
** Scan Around Tptr Polygon And Mark Immediately External Points And Create Internal Tptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Immediately External To Tptr Polygon") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
 ppnt= sPnt ;
 pnt = (Tin->nodesP+sPnt)->tPtr ; 
 do
   {
    lp = npnt = (Tin->nodesP+pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineTinObject(Tin,npnt,pnt) && ! bcdtmList_testForVoidOrHoleHullLineTinObject(Tin,pnt,npnt) )
      {
       if(( lp = bcdtmList_nextClockTinObject(Tin,pnt,lp)) < 0 ) goto errexit ; ;
/*
** Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineTinObject(Tin,lp,pnt) && ! bcdtmList_testForVoidOrHoleHullLineTinObject(Tin,pnt,lp) )
         {
          if( (Tin->nodesP+lp)->tPtr == Tin->nullPnt && ! bcdtmList_testIfPointOnIslandVoidOrHoleHullTinObject(Tin,lp) )
            {
             if( lPnt == Tin->nullPnt ) { fPnt = lPnt = lp ;  }
             else                      { (Tin->nodesP+lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
             (Tin->nodesP+lPnt)->tPtr = -(lPnt+1) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"1Marking Point %6ld External To %6ld ** fPnt = %9ld lPnt = %9ld",lp,pnt,fPnt,lPnt) ;
            }
          if(( lp = bcdtmList_nextClockTinObject(Tin,pnt,lp)) < 0 ) goto errexit ; ;
         }
      }
/*
** Scan Anti Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt ;
       if( ! bcdtmList_testForIslandHullLineTinObject(Tin,pnt,ppnt) && ! bcdtmList_testForVoidOrHoleHullLineTinObject(Tin,ppnt,pnt) )
         {
          if(( lp = bcdtmList_nextAntTinObject(Tin,pnt,lp)) < 0 ) goto errexit ; ;
/*
** Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( lp != npnt && ! bcdtmList_testForIslandHullLineTinObject(Tin,pnt,lp) && ! bcdtmList_testForVoidOrHoleHullLineTinObject(Tin,lp,pnt) )
            {
             if( (Tin->nodesP+lp)->tPtr == Tin->nullPnt && ! bcdtmList_testIfPointOnIslandVoidOrHoleHullTinObject(Tin,lp) )
               {
                if( lPnt == Tin->nullPnt ) { fPnt = lPnt = lp ;  }
                else                      { (Tin->nodesP+lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
                (Tin->nodesP+lPnt)->tPtr = -(lPnt+1) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"2Marking Point %6ld External To %6ld ** fPnt = %9ld lPnt = %9ld",lp,pnt,fPnt,lPnt) ;
               }
             if(( lp = bcdtmList_nextAntTinObject(Tin,pnt,lp)) < 0 ) goto errexit ; ;
            }
         }
      }
/*
** Reset For Next Point On Tptr Polygon
*/
    ppnt = pnt ;  
    pnt  = npnt ; 
   } while ( ppnt!= sPnt ) ;
/*
** Scan External Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
 if( fPnt != Tin->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To External Marked Points") ;
    pnt = fPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld",pnt) ;
       lpnt = pnt ;
       clc = (Tin->nodesP+pnt)->cPtr ;
       while( clc != Tin->nullPtr )
         {
          lp  = (Tin->cListP+clc)->pntNum ;
          clc = (Tin->cListP+clc)->nextPtr ;
          if((Tin->nodesP+lp)->tPtr == Tin->nullPnt && ! bcdtmList_testIfPointOnIslandVoidOrHoleHullTinObject(Tin,lp) ) 
            { 
             if( dbg ) bcdtmWrite_message(0,0,0,"3Marking Point %6ld External To %6ld ** fPnt = %9ld lPnt = %9ld",lp,pnt,fPnt,lPnt) ;
             (Tin->nodesP+lPnt)->tPtr = -(lp+1) ; 
             lPnt = lp ; 
             (Tin->nodesP+lPnt)->tPtr = -(lPnt+1) ; 
            }
         }
       npnt = -(Tin->nodesP+pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Mark Void Points And Null Out Internal Tptr List
*/
 if( fPnt != Tin->nullPnt )
   {
    pnt = fPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Internal Tptr List") ;
    do
      {
       npnt = -(Tin->nodesP+pnt)->tPtr - 1 ;
       bcdtmFlag_setVoidBitPCWD(&(Tin->nodesP+pnt)->PCWD) ;
       (Tin->nodesP+pnt)->tPtr = Tin->nullPnt ;
       ++*numMarked ;
       pnt = npnt ;
      } while ( (Tin->nodesP+pnt)->tPtr != Tin->nullPnt ) ;
   }
/*
** Write Number Marked
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",*numMarked) ;
/*
** Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Void Points External To Tptr Island Polygon Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Marking Void Points External To Tptr Island Polygon Error") ;
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
BENTLEYDTM_Public int bcdtmMark_pointsInternalToVoidTinObject(DTM_TIN_OBJ *Tin,long VoidFeature ) 
/*
** This Function Marks Points Internal To A Void As Void Points.
** VoidFeature Is expressed as offset from Start Of Feature Table
*/
{
 long  dbg=DTM_TRACE_VALUE(0),sp,spnt,NumMarked ;
 DTMFeatureType type;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points Internal To Void Feature = %6ld",VoidFeature) ;
/*
** Initialise
*/
 type = (Tin->fTableP+VoidFeature)->dtmFeatureType ;
 spnt = (Tin->fTableP+VoidFeature)->firstPnt ;
/*
** Check For Non Null Tptr Values
*/
 if( dbg )  bcdtmList_reportAndSetToNullNoneNullTptrValuesTinObject(Tin,dbg) ;
/*
** Check For Valid Void Feature
*/
 if( ( type != DTMFeatureType::Void && type != DTMFeatureType::Hole ) || spnt == Tin->nullPnt ) goto cleanup ;
/*
** Copy Feature Points To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copy Void Feature To Tptr Polygon") ;
 if( bcdtmList_copyDtmFeatureToTptrListTinObject(Tin,VoidFeature,&spnt)) goto errexit   ;
/*
** Mark Internal Void Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Void Points") ;
 if( bcdtmMark_voidPointsInternalToTptrVoidPolygonTinObject(Tin,spnt,&NumMarked))  
/*
** Replaced RobC - June 2003
 if( bcdtmMark_internalTptrPolygonPointsAsVoidPointsTinObject(Tin,spnt,&NumMarked))  
*/
/*
** Mark Points On Void Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points On Void Hull") ;
 sp = spnt ;
 do
   {
    bcdtmFlag_clearVoidBitPCWD(&(Tin->nodesP+sp)->PCWD) ;
    sp = (Tin->nodesP+sp)->tPtr ;
   } while ( sp != spnt ) ;
/*
** Null Out Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Tptr Void Polygon") ;
 if( bcdtmList_nullOutTptrListTinObject(Tin,spnt)) goto errexit   ;
/*
** Check For Non Null Tptr Values
*/
 if( dbg )  bcdtmList_reportAndSetToNullNoneNullTptrValuesTinObject(Tin,dbg) ;
/*
** Clean Up
*/
 cleanup :
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
BENTLEYDTM_Public int bcdtmList_getVoidExternalToIslandTinObject(DTM_TIN_OBJ *tinP,long islandFeature,long *voidFeatureP)
/*
** This Function Gets The Void Immediately External To A Island 
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    hullPnt,nextPnt,priorPnt,listPnt,lowPnt,highPnt,intersectResult;
 long    firstPnt,listPtr,dtmFeature,voidPoint,numIslandPts=0,numVoidPts=0,noVoidPoint ;
 DPoint3d     *islandPtsP=NULL,*voidPtsP=NULL ;
 DTM_DAT_OBJ *dataP=NULL ;
 wchar_t   outFile[128] ;
 static long fileNum=0 ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Void External To Island") ;
/*
** Initialise
*/
 lowPnt  = tinP->numPts ;
 highPnt = 0 ;
 *voidFeatureP = tinP->nullPnt  ;
/*
** Only Process If Island Feature
*/
 if( islandFeature >= 0 && islandFeature < tinP->numFeatureTable )
   { 
    if( (tinP->fTableP+islandFeature)->dtmFeatureType  == DTMFeatureType::Island ) 
      { 
/*
** Scan Island Hull And Look For A Corresponding Void Hull Line
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Island Hull") ;
       noVoidPoint = TRUE ; 
       priorPnt = (tinP->fTableP+islandFeature)->firstPnt ;
       bcdtmList_getNextPointForDtmFeatureTinObject(tinP,islandFeature,priorPnt,&hullPnt) ;
       bcdtmList_getNextPointForDtmFeatureTinObject(tinP,islandFeature,hullPnt,&nextPnt) ;
       firstPnt = hullPnt ;
       do
         {
/*
** Check If Island Point Is A Void Point
*/
          voidPoint = FALSE ;
          listPtr = (tinP->nodesP+hullPnt)->fPtr ;
          while( listPtr != tinP->nullPtr && voidPoint == FALSE )
            {
             dtmFeature = (tinP->fListP+listPtr)->dtmFeature ;
             listPtr    = (tinP->fListP+listPtr)->nextPtr ; 
             if( (tinP->fTableP+dtmFeature)->dtmFeatureType == DTMFeatureType::Void && (tinP->fTableP+dtmFeature)->firstPnt != tinP->nullPnt ) voidPoint = TRUE ;
            } 
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Hull Point = %8ld ** Void Point = %1ld",hullPnt,voidPoint) ;          
/*
** If Void Point Test For Surrounding Void
*/
          if( voidPoint == TRUE )
            {
             noVoidPoint = FALSE ;
             if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Testing For Surrounding Void At Hull Point = %8ld ** %12.4lf %12.4lf %10.4lf",hullPnt,(tinP->pointsP+hullPnt)->x,(tinP->pointsP+hullPnt)->y,(tinP->pointsP+hullPnt)->z) ;          
             listPnt = nextPnt ;
             while ( listPnt != priorPnt && *voidFeatureP == tinP->nullPnt )
               {
                if( bcdtmList_testForSurroundingDtmFeatureTypeTinObject(tinP,hullPnt,nextPnt,priorPnt,DTMFeatureType::Void,voidFeatureP) != DTM_SUCCESS ) goto errexit ;
                if(( listPnt = bcdtmList_nextClockTinObject(tinP,hullPnt,listPnt)) < 0 ) goto errexit ;
                if( dbg && *voidFeatureP != tinP->nullPnt ) bcdtmWrite_message(0,0,0,"Surrounding Void Found = %6ld",*voidFeatureP) ;
               }
            }
/*
** Set For Next Island Hull Point
*/
          priorPnt = hullPnt ; 
          hullPnt  = nextPnt ;
          bcdtmList_getNextPointForDtmFeatureTinObject(tinP,islandFeature,hullPnt,&nextPnt) ;
         } while ( hullPnt != firstPnt && *voidFeatureP == tinP->nullPnt ) ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Hull Point = %8ld",hullPnt) ;          
/*
** Scan Out From Island Hull Looking For Surrounding Void
*/
       if( noVoidPoint == TRUE )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Out From Island Hull") ;
          if( bcdtmList_getVoidFeatureExternalToIslandFeatureTinObject(tinP,islandFeature,voidFeatureP) ) goto errexit ;
/*
** Check Void Feature Surrounds Island
*/
          if( *voidFeatureP != tinP->nullPnt )
            { 
             if( bcdtmList_copyDtmFeaturePointsToPointArrayTinObject(tinP,islandFeature,&islandPtsP,&numIslandPts) ) goto errexit ;
             if( bcdtmList_copyDtmFeaturePointsToPointArrayTinObject(tinP,*voidFeatureP,&voidPtsP,&numVoidPts) ) goto errexit ;
             if( bcdtmClip_checkPolygonsIntersect(islandPtsP,numIslandPts,voidPtsP,numVoidPts,&intersectResult)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Intersection Result = %2ld",intersectResult) ;
             if( intersectResult == 0 || intersectResult == 3 ) *voidFeatureP = tinP->nullPnt ;
            }
         } 
/*
** Write Island And Surrounding Voids To A Dat File - For Visual Checking
*/
       if( cdbg )
         {
          if( islandPtsP == NULL ) { if( bcdtmList_copyDtmFeaturePointsToPointArrayTinObject(tinP,islandFeature,&islandPtsP,&numIslandPts) ) goto errexit ; }
          if( voidPtsP   == NULL && *voidFeatureP != tinP->nullPnt ) { if( bcdtmList_copyDtmFeaturePointsToPointArrayTinObject(tinP,*voidFeatureP,&voidPtsP,&numVoidPts) ) goto errexit ; }
          if( bcdtmObject_createDataObject(&dataP)) goto errexit ;
          bcdtmObject_setMemoryAllocationParametersDataObject(dataP,numIslandPts+numVoidPts,100) ; 
          if( islandPtsP != NULL ) { if( bcdtmObject_storeDtmFeatureInDataObject(dataP,DTMFeatureType::Island,DTM_NULL_USER_TAG,nullGuid,islandPtsP,numIslandPts)) goto errexit ; }
          if( voidPtsP   != NULL ) { if( bcdtmObject_storeDtmFeatureInDataObject(dataP,DTMFeatureType::Void,DTM_NULL_USER_TAG,nullGuid,voidPtsP,numVoidPts)) goto errexit ; }
          if( *voidFeatureP != tinP->nullPnt ) swprintf(outFile,128,L"islandVoid%d.dat",fileNum) ;
          else                                 swprintf(outFile,128,L"islandVoidError%d.dat",fileNum) ;
          if( bcdtmWrite_dataFileFromDataObject(dataP,outFile) ) goto errexit ;
          bcdtmObject_deleteDataObject(&dataP) ;
          ++fileNum ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( islandPtsP != NULL ) free(islandPtsP) ;
 if( voidPtsP   != NULL ) free(voidPtsP) ;
 if( dataP      != NULL ) bcdtmObject_deleteDataObject(&dataP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Void External To Island Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Void External To Island Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 *voidFeatureP = tinP->nullPnt ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmList_testForSurroundingDtmFeatureTypeTinObject(DTM_TIN_OBJ *tinP,long featurePnt,long featureNextPnt,long featurePriorPnt,DTMFeatureType dtmFeatureType,long *surroundFeatureP) 
/*
** This Function Tests For A Surrounding Feature Type At A Point 
** On An Existing Feature 
*/
{
 int  ret=DTM_SUCCESS ;
 long cnt,listPtr,nextPnt,priorPnt,dtmFeature,clkPnt ;
/*
** Initialise
*/
 *surroundFeatureP = tinP->nullPnt ;
/*
** Scan Feature List For Point
*/
 listPtr = (tinP->nodesP+featurePnt)->fPtr ;
 while( listPtr != tinP->nullPtr && *surroundFeatureP == tinP->nullPnt )
   {
    if( (tinP->fListP+listPtr)->nextPnt == featurePnt && (tinP->fTableP+(tinP->fListP+listPtr)->dtmFeature)->dtmFeatureType == dtmFeatureType ) 
      {
       dtmFeature = (tinP->fListP+listPtr)->dtmFeature ;
       if( bcdtmList_getNextPointForDtmFeatureTinObject(tinP,dtmFeature,featurePnt,&nextPnt) ) goto errexit ;
       if( bcdtmList_getPriorPointForDtmFeatureTinObject(tinP,dtmFeature,featurePnt,&priorPnt) ) goto errexit ;
       if( priorPnt == featurePriorPnt && nextPnt == featureNextPnt )  *surroundFeatureP = dtmFeature ;
/*
**  Scan Clockwise From Prior Point To Next Point 
*/
       else
         {
          cnt = 0 ;            
          if( (clkPnt = bcdtmList_nextClockTinObject(tinP,featurePnt,priorPnt)) < 0 ) goto errexit ;        
          while ( clkPnt != nextPnt )
            {
             if( clkPnt == featurePriorPnt ) ++cnt ;
             if( clkPnt == featureNextPnt  ) ++cnt ;
             if( (clkPnt = bcdtmList_nextClockTinObject(tinP,featurePnt,clkPnt)) < 0 ) goto errexit ;        
            }  
          if     ( cnt == 2 ) *surroundFeatureP = dtmFeature ;
          else if( cnt == 1 && priorPnt == featurePriorPnt ) *surroundFeatureP = dtmFeature ;
          else if( cnt == 1 && nextPnt  == featureNextPnt  ) *surroundFeatureP = dtmFeature ;
         }  
      } 
    listPtr = (tinP->fListP+listPtr)->nextPtr ;
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
BENTLEYDTM_Private int bcdtmList_getVoidFeatureExternalToIslandFeatureTinObject(DTM_TIN_OBJ *tinP,long islandFeature,long *voidFeatureP) 
/*
** This Functions Gets The Void Feature External To An Island Feature. 
** It Is Only To Be Called Directly By "bcdtmList_getVoidExternalToIslandTinObject"
** The Function Is Programmed To Ignore Implicit Voids Between Islands
** 
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  startPnt,priorPnt,tptrPnt,nextPnt,clkPnt,listPnt,listPtr,firstPnt ;
 long  lastPnt,islandHull,voidHull,numMarked,nextIslandPnt,priorIslandPnt ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Void Feature External To Island") ; 
/*
** Initialise
*/
 numMarked = 0 ;
 startPnt = firstPnt = lastPnt = *voidFeatureP = tinP->nullPnt ;
/*
** Only Process If Island Feature
*/
 if( islandFeature >= 0 && islandFeature < tinP->numFeatureTable )
   { 
    if( (tinP->fTableP+islandFeature)->dtmFeatureType  == DTMFeatureType::Island ) 
      {
/*
** Check All Tptr Values Are Null
*/ 
       if( cdbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesTinObject(tinP,cdbg) ;
/*
** Copy Island Feature To Tptr List
*/
       if( bcdtmList_copyDtmFeatureToTptrListTinObject(tinP,islandFeature,&startPnt)) goto errexit ;
/*
** Scan Around Tptr Polygon And Mark External Points And Create Internal Tptr List
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Tptr Polygon") ; 
       priorPnt = startPnt ;
       tptrPnt  = (tinP->nodesP+startPnt)->tPtr ; 
       do
         {
          nextPnt = (tinP->nodesP+tptrPnt)->tPtr ;
          if(( clkPnt = bcdtmList_nextClockTinObject(tinP,tptrPnt,nextPnt)) < 0 ) goto errexit ;
          while ( clkPnt != priorPnt && *voidFeatureP == tinP->nullPnt )
            {
             if( (tinP->nodesP+clkPnt)->tPtr == tinP->nullPnt && clkPnt != lastPnt )
               { 
                if( bcdtmList_testIfPointOnDtmFeatureTinObject(tinP,DTMFeatureType::Island,clkPnt,&islandHull)) goto errexit ;
                if( bcdtmList_testIfPointOnDtmFeatureTinObject(tinP,DTMFeatureType::Void,clkPnt,&voidHull)) goto errexit ;
                if( islandHull == tinP->nullPnt ) *voidFeatureP = voidHull ;
/*
** Only Process If Surrounding Void Hull Not Found
*/
                if( *voidFeatureP == tinP->nullPnt )
                  { 
/*
** Mark Island Boundary
*/
                   if( islandHull != tinP->nullPnt )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"Marking Island Hull = %6ld",islandHull) ;
                      if( bcdtmList_addDtmFeatureToTptrListTinObject(tinP,islandHull,&firstPnt,&lastPnt,&numMarked) ) goto errexit ;
                     }
/*
** Mark Point
*/
                   else
                     {
                      if( voidHull == tinP->nullPnt )
                        {
                         if( firstPnt == tinP->nullPnt ) firstPnt = clkPnt ;
                         if( lastPnt  != tinP->nullPnt ) (tinP->nodesP+lastPnt)->tPtr = clkPnt ;
                         lastPnt = clkPnt ;
                         ++numMarked ;                  
                        }
                     }
                  }
               } 
             if(( clkPnt = bcdtmList_nextClockTinObject(tinP,tptrPnt,clkPnt)) < 0 ) goto errexit ;
            }
          priorPnt = tptrPnt ;  
          tptrPnt  = nextPnt ; 
         } while ( priorPnt != startPnt && *voidFeatureP == tinP->nullPnt ) ;
/*
** Scan Tptr List
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Tptr List") ; 
       tptrPnt = firstPnt ; 
       while ( tptrPnt != tinP->nullPnt && *voidFeatureP == tinP->nullPnt )
         {
          nextPnt = (tinP->nodesP+tptrPnt)->tPtr ;
/*
** If Point On Island Hull Scan Clockwise From Next Point To Prior Point
*/
          if( bcdtmList_testIfPointOnDtmFeatureTinObject(tinP,DTMFeatureType::Island,tptrPnt,&islandHull)) goto errexit ;
          if( islandHull != tinP->nullPnt )
            {
             bcdtmList_getNextPointForDtmFeatureTinObject(tinP,islandHull,tptrPnt,&nextIslandPnt) ;
             bcdtmList_getPriorPointForDtmFeatureTinObject(tinP,islandHull,tptrPnt,&priorIslandPnt) ;
             if( ( listPnt = bcdtmList_nextClockTinObject(tinP,tptrPnt,nextIslandPnt)) < 0 ) goto errexit ;
             while ( listPnt != priorIslandPnt && *voidFeatureP == tinP->nullPnt )
               {
                if((tinP->nodesP+listPnt)->tPtr == tinP->nullPnt && listPnt != lastPnt )
                  {
                   if( bcdtmList_testIfPointOnDtmFeatureTinObject(tinP,DTMFeatureType::Island,listPnt,&islandHull)) goto errexit ;
                   if( bcdtmList_testIfPointOnDtmFeatureTinObject(tinP,DTMFeatureType::Void,listPnt,&voidHull)) goto errexit ;
                   if( islandHull == tinP->nullPnt ) *voidFeatureP = voidHull ;
/*
** Only Process If Surrounding Void Hull Not Found
*/
                   if( *voidFeatureP == tinP->nullPnt )  
                     { 
/*
** Mark Island Boundary
*/
                      if( islandHull != tinP->nullPnt )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Marking Island Hull = %6ld",islandHull) ;
                         if( bcdtmList_addDtmFeatureToTptrListTinObject(tinP,islandHull,&firstPnt,&lastPnt,&numMarked) ) goto errexit ;
                        }
/*
** Mark Point
*/
                      else
                        {
                         if( voidHull == tinP->nullPnt )
                           {
                            (tinP->nodesP+lastPnt)->tPtr = listPnt ; 
                            lastPnt = listPnt ; 
                            ++numMarked ;                  
                           }
                        }
                     }
                  }
                if((listPnt = bcdtmList_nextClockTinObject(tinP,tptrPnt,listPnt)) < 0 ) goto errexit ;
               }  
            }
/*
** If Point Not On Island Hull Scan Cylic List
*/
          else
            { 
             listPtr = (tinP->nodesP+tptrPnt)->cPtr ;
             while( listPtr != tinP->nullPtr && *voidFeatureP == tinP->nullPnt )
               {
                listPnt = (tinP->cListP+listPtr)->pntNum ;
                listPtr = (tinP->cListP+listPtr)->nextPtr ;
                if((tinP->nodesP+listPnt)->tPtr == tinP->nullPnt && listPnt != lastPnt )
                  {
                   if( bcdtmList_testIfPointOnDtmFeatureTinObject(tinP,DTMFeatureType::Island,listPnt,&islandHull)) goto errexit ;
                   if( bcdtmList_testIfPointOnDtmFeatureTinObject(tinP,DTMFeatureType::Void,listPnt,&voidHull)) goto errexit ;
                   if( islandHull == tinP->nullPnt ) *voidFeatureP = voidHull ;
                   if( *voidFeatureP == tinP->nullPnt )  
                     { 
/*
** Mark Island Boundary
*/
                      if( islandHull != tinP->nullPnt )
                        {
                         if( bcdtmList_addDtmFeatureToTptrListTinObject(tinP,islandHull,&firstPnt,&lastPnt,&numMarked) ) goto errexit ;
                        }
/*
** Mark Point
*/
                      else
                        {
                         if( voidHull == tinP->nullPnt )
                           {
                            (tinP->nodesP+lastPnt)->tPtr = listPnt ; 
                            lastPnt = listPnt ; 
                            ++numMarked ;                  
                           }
                        }
                     }
                  }
               }
            }
/*
** Set To Next Point In Tptr List
*/
          tptrPnt = nextPnt ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Cleaning Up") ; 
    bcdtmWrite_message(0,0,0,"Num Marked    = %6ld",numMarked) ;
   }
/*
** Unmark Tptr List
*/ 
 numMarked = 0 ;
 while ( firstPnt != tinP->nullPnt )
   {
    ++numMarked ;
    nextPnt = (tinP->nodesP+firstPnt)->tPtr ;
    (tinP->nodesP+firstPnt)->tPtr = tinP->nullPnt ;
    firstPnt = nextPnt ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Num Un Marked = %6ld",numMarked) ;
/*
** Null Out Tptr Island Polygon
*/
 if( startPnt != tinP->nullPnt ) bcdtmList_nullOutTptrListTinObject(tinP,startPnt) ;
/*
** Check For Any None Null Tptr Values
*/
 if( cdbg) bcdtmList_reportAndSetToNullNoneNullTptrValuesTinObject(tinP,cdbg) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Void Feature External To Island Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Void Feature External To Island Error") ; 
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
BENTLEYDTM_Private int bcdtmList_addDtmFeatureToTptrListTinObject(DTM_TIN_OBJ *tinP,long dtmFeature,long *firstPntP,long *lastPntP,long *numAddedP) 
/*
** This Function Adds The Hull Of A Dtm Feature To The Tptr List
** It Is A Support Function Only And Should Not Be Called For Any Other Purpose
** Assumes Feature Is Of One Of The Following Types:-
** 1.  DTMFeatureType::Void ,
** 2.  DTMFeatureType::Island
**
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long firstDtmFeaturePnt,nextDtmFeaturePnt,nextPnt ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Tptr List") ; 
/*
** Scan Dtm Feature And Add Feature Points To Tptr List
*/
 nextDtmFeaturePnt = firstDtmFeaturePnt = (tinP->fTableP+dtmFeature)->firstPnt ;
 do
   {
    if( (tinP->nodesP+nextDtmFeaturePnt)->tPtr == tinP->nullPnt && nextDtmFeaturePnt != *lastPntP )
      {
       if( *firstPntP == tinP->nullPnt ) *firstPntP = nextDtmFeaturePnt ;
       if( *lastPntP  != tinP->nullPnt ) (tinP->nodesP+*lastPntP)->tPtr = nextDtmFeaturePnt ;
       *lastPntP = nextDtmFeaturePnt ;
       ++(*numAddedP) ;                  
      } 
    if( bcdtmList_getNextPointForDtmFeatureTinObject(tinP,dtmFeature,nextDtmFeaturePnt,&nextPnt) ) goto errexit ;
    nextDtmFeaturePnt = nextPnt ; 
   } while ( nextDtmFeaturePnt != firstDtmFeaturePnt ) ;  
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Tptr List Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Dtm Feature To Tptr List Error") ; 
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
BENTLEYDTM_Public int bcdtmList_testIfPointOnIslandVoidOrHoleHullTinObject(DTM_TIN_OBJ *Tin,long P1)
/*
** This Function Tests If P1 is On An Island Void Or Hole Hull
*/
{
 long clc ;
/*
** Test If Point Is On Island Void Or Hole Hull Line
*/
 clc = ( Tin->nodesP+P1)->fPtr ;
 while ( clc != Tin->nullPtr )
   {
    if( (Tin->fTableP+(Tin->fListP+clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
        (Tin->fTableP+(Tin->fListP+clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
        (Tin->fTableP+(Tin->fListP+clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      ) return(1) ;
    clc = (Tin->fListP+clc)->nextPtr ;  
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
BENTLEYDTM_Public int bcdtmList_testForVoidOrHoleHullLineTinObject(DTM_TIN_OBJ *Tin,long P1,long P2)
/*
** This Function Tests If The Line P1-P2 is A Void Or Hull Line
*/
{
 long clc ;
/*
** Test For Void Hull Line
*/
 clc = ( Tin->nodesP+P1)->fPtr ;
 while ( clc != Tin->nullPtr )
   {
    if((Tin->fListP+clc)->nextPnt == P2 )
      {
       if( (Tin->fTableP+(Tin->fListP+clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ||
           (Tin->fTableP+(Tin->fListP+clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole    )  return(1) ; 
      }
    clc = (Tin->fListP+clc)->nextPtr ;  
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
BENTLEYDTM_Public int bcdtmList_testForIslandHullLineTinObject(DTM_TIN_OBJ *Tin,long P1,long P2)
/*
** This Function Tests If The Line P1-P2 is On An Island Hull
*/
{
 long clc ;
/*
** Test For Island Line
*/
 clc = ( Tin->nodesP+P1)->fPtr ;
 while ( clc != Tin->nullPtr )
   {
    if((Tin->fListP+clc)->nextPnt == P2 )
      {
       if( (Tin->fTableP+(Tin->fListP+clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) return(1) ;
      }
    clc = (Tin->fListP+clc)->nextPtr ;  
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
BENTLEYDTM_Public int bcdtmMark_voidPointsInternalToTptrVoidPolygonTinObject(DTM_TIN_OBJ *Tin,long sPnt,long *numMarked ) 
/*
** This Function marks void Points internal To a tPtr Void Polygon.
**
** Rob Cormack June 2003
** 
*/
{
 int  ret=DTM_SUCCESS ;
 long dbg=DTM_TRACE_VALUE(0),npnt,ppnt,lpnt,lp,clc,pnt,fPnt,lPnt ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points Internal To Tptr Void Polygon") ;
/*
** Initialise
*/
 *numMarked = 0 ;
 fPnt = lPnt = Tin->nullPnt ;
/*
** Check For Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListTinObject(Tin,sPnt) ;
 if( (Tin->nodesP+sPnt)->tPtr == Tin->nullPnt ) goto errexit ;
/*
** Scan Around Tptr Polygon And Mark Immediately Internal Points And Create Internal Tptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Immediately Internal To Tptr Polygon") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
 ppnt= sPnt ;
 pnt = (Tin->nodesP+sPnt)->tPtr ; 
 do
   {
    lp = npnt = (Tin->nodesP+pnt)->tPtr ;
    if( ! bcdtmList_testForIslandHullLineTinObject(Tin,pnt,npnt) )
      {
       if(( lp = bcdtmList_nextAntTinObject(Tin,pnt,lp)) < 0 ) goto errexit ; 
/*
** Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
*/
       while ( lp != ppnt && ! bcdtmList_testForIslandHullLineTinObject(Tin,pnt,lp))
         {
          if( (Tin->nodesP+lp)->tPtr == Tin->nullPnt && ! bcdtmList_testIfPointOnIslandVoidOrHoleHullTinObject(Tin,lp))
            {
             if( lPnt == Tin->nullPnt ) { fPnt = lPnt = lp ;  }
             else                      { (Tin->nodesP+lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
             (Tin->nodesP+lPnt)->tPtr = -(lPnt+1) ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"1Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** fPnt = %9ld lPnt = %9ld",lp,(Tin->pointsP+lp)->x,(Tin->pointsP+lp)->y,(Tin->pointsP+lp)->z,pnt,fPnt,lPnt) ;
            }
          if(( lp = bcdtmList_nextAntTinObject(Tin,pnt,lp)) < 0 ) goto errexit ; ;
         }
      }
/*
** Scan Clockwise From Prior Point On Tptr Polygon To Next Point
*/
    if( lp != ppnt )
      {
       lp = ppnt  ;
       if( ! bcdtmList_testForIslandHullLineTinObject(Tin,ppnt,pnt) )
         {
          if(( lp = bcdtmList_nextClockTinObject(Tin,pnt,lp)) < 0 ) goto errexit ; 
/*
** Scan Clockwise From Next Point On Tptr Polygon To Prior Point
*/
          while ( lp != npnt && ! bcdtmList_testForIslandHullLineTinObject(Tin,lp,pnt))
            {
             if( (Tin->nodesP+lp)->tPtr == Tin->nullPnt && ! bcdtmList_testIfPointOnIslandVoidOrHoleHullTinObject(Tin,lp) )
               {
                if( lPnt == Tin->nullPnt ) { fPnt = lPnt = lp ;  }
                else                      { (Tin->nodesP+lPnt)->tPtr = -(lp+1) ; lPnt = lp ; }
                (Tin->nodesP+lPnt)->tPtr = -(lPnt+1) ;
                if( dbg == 1 ) bcdtmWrite_message(0,0,0,"2Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** fPnt = %9ld lPnt = %9ld",lp,(Tin->pointsP+lp)->x,(Tin->pointsP+lp)->y,(Tin->pointsP+lp)->z,pnt,fPnt,lPnt) ;
               }
             if(( lp = bcdtmList_nextClockTinObject(Tin,pnt,lp)) < 0 ) goto errexit ; ;
            }
         }
      }
/*
** Reset For Next Point On Tptr Polygon
*/
    ppnt = pnt ;  
    pnt  = npnt ; 
   } while ( ppnt!= sPnt ) ;
/*
** Scan External Tptr List And Mark Points Connected To Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
 if( fPnt != Tin->nullPnt )
   {
    pnt = fPnt ;
    do
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld",pnt) ;
       lpnt = pnt ;
       clc = (Tin->nodesP+pnt)->cPtr ;
       while( clc != Tin->nullPtr )
         {
          lp  = (Tin->cListP+clc)->pntNum ;
          clc = (Tin->cListP+clc)->nextPtr ;
          if((Tin->nodesP+lp)->tPtr == Tin->nullPnt && ! bcdtmList_testIfPointOnIslandVoidOrHoleHullTinObject(Tin,lp) ) 
            { 
             (Tin->nodesP+lPnt)->tPtr = -(lp+1) ; 
             lPnt = lp ; 
             (Tin->nodesP+lPnt)->tPtr = -(lPnt+1) ; 
             if( dbg == 3 ) bcdtmWrite_message(0,0,0,"3Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** fPnt = %9ld lPnt = %9ld",lp,(Tin->pointsP+lp)->x,(Tin->pointsP+lp)->y,(Tin->pointsP+lp)->z,pnt,fPnt,lPnt) ;
            }
         }
       npnt = -(Tin->nodesP+pnt)->tPtr - 1 ;
       pnt = npnt ;
      } while ( lpnt != pnt  ) ;
   }
/*
** Mark And Null Out Internal Tptr List
*/
 if( fPnt != Tin->nullPnt )
   {
    pnt = fPnt ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Internal Tptr List") ;
    do
      {
       npnt = -(Tin->nodesP+pnt)->tPtr - 1 ;
       bcdtmFlag_setVoidBitPCWD(&(Tin->nodesP+pnt)->PCWD) ;
       (Tin->nodesP+pnt)->tPtr = Tin->nullPnt ;
       ++*numMarked ;
       pnt = npnt ;
      } while ( (Tin->nodesP+pnt)->tPtr != Tin->nullPnt ) ;
   }
/*
** Write Number Marked
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",*numMarked) ;
/*
** Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Void Points Internal To Tptr Void Polygon Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Marking Void Points Internal To Tptr Void Polygon Error") ;
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
BENTLEYDTM_Public int bcdtmList_getPriorPointForDtmFeatureTinObject(DTM_TIN_OBJ *tinP,long dtmFeature,long currentPnt,long *priorPnt)
/*
** This Function Gets The Prior Point For A Dtm Feature
*/
{
 long listPtr,scanPnt,firstPnt,lastPnt ;
/*
** Initialise
*/
 *priorPnt = tinP->nullPnt ;
/*
** Check Dtm Feature Exists
*/
 if( dtmFeature >= 0 && dtmFeature < tinP->numFeatureTable ) 
   {
    if( ( firstPnt = (tinP->fTableP+dtmFeature)->firstPnt ) != tinP->nullPnt ) 
      { 
       scanPnt = lastPnt = firstPnt ;
       listPtr = (tinP->nodesP+scanPnt)->fPtr ;
/*
** Scan Dtm Feature Points To Current Point
*/
       while ( listPtr != tinP->nullPtr && *priorPnt == tinP->nullPnt )
         {
          while ( listPtr != tinP->nullPtr && (tinP->fListP+listPtr)->dtmFeature != dtmFeature ) listPtr = (tinP->fListP+listPtr)->nextPtr ;
          if( listPtr != tinP->nullPtr )
            {
             scanPnt = (tinP->fListP+listPtr)->nextPnt ;
             if( scanPnt != tinP->nullPnt )
               {
                if( scanPnt == currentPnt ) *priorPnt = lastPnt ;
                lastPnt = scanPnt ; 
                listPtr = (tinP->nodesP+scanPnt)->fPtr ; 
               }
             if( scanPnt == tinP->nullPnt || scanPnt == firstPnt ) listPtr = tinP->nullPtr ;
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
BENTLEYDTM_Public int bcdtmList_testIfPointOnDtmFeatureTinObject(DTM_TIN_OBJ *Tin,DTMFeatureType dtmFeatureType,long P,long *Feature) 
/*
** This Function Tests If a Point is on a DTM Faeture And If So Returns 
** The Feature Number For The Point
** 
*/
{
 long clc ;
/*
** Initialise
*/
 *Feature = Tin->nullPnt ;
/*
** Scan Feature List Points For Point
*/
 clc = (Tin->nodesP+P)->fPtr ;
 while( clc != Tin->nullPtr )
   {
   if ((Tin->fTableP + (Tin->fListP + clc)->dtmFeature)->dtmFeatureType == dtmFeatureType)
      { *Feature =  (Tin->fListP+clc)->dtmFeature ; return(0) ; }
    clc = (Tin->fListP+clc)->nextPtr ;
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
BENTLEYDTM_EXPORT int bcdtmList_copyDtmFeaturePointsToPointArrayTinObject(DTM_TIN_OBJ *tinP,long dtmFeature,DPoint3d **featurePtsPP,long *numFeaturePtsP) 
/*
** This Function Copies The Dtm Feature Points To A Point Array 
*/
{
 int  ret=DTM_SUCCESS ; 
 long listPtr,nextPnt,firstPnt,numPts ;
 DPoint3d  *p3dP ;
/*
** Initialise
*/
 *numFeaturePtsP = 0  ; 
 if( *featurePtsPP != NULL )
   {
    free(*featurePtsPP) ;
    *featurePtsPP = NULL ;
   }
/*
** Check Dtm Feature Exists
*/
 if( dtmFeature >= 0 && dtmFeature < tinP->numFeatureTable ) 
   { 
    if( ( firstPnt = nextPnt = (tinP->fTableP+dtmFeature)->firstPnt ) != tinP->nullPnt )
      { 
/*
** Count Number Of Points In Dtm Feature
*/
       numPts  = 1 ;
       listPtr = (tinP->nodesP+firstPnt)->fPtr ;
       while ( listPtr != tinP->nullPtr )
         {
          while ( listPtr != tinP->nullPtr  && (tinP->fListP+listPtr)->dtmFeature != dtmFeature ) listPtr = (tinP->fListP+listPtr)->nextPtr ;
          if( listPtr != tinP->nullPtr )
            {
             nextPnt = (tinP->fListP+listPtr)->nextPnt ;
             if(  nextPnt != tinP->nullPnt )
               {
                ++numPts ;
                listPtr = (tinP->nodesP+nextPnt)->fPtr ;
               } 
             if(  nextPnt == tinP->nullPnt || nextPnt == firstPnt ) listPtr = tinP->nullPtr ; 
            }
         } 
/*
** Allocate memory For Feature Points
*/
       *featurePtsPP = ( DPoint3d * ) malloc( numPts * sizeof(DPoint3d)) ;
       if( *featurePtsPP == NULL )
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; 
          goto errexit ; 
         }
/*
** Copy First Point
*/
       p3dP = *featurePtsPP ;
       p3dP->x = (tinP->pointsP+firstPnt)->x ;
       p3dP->y = (tinP->pointsP+firstPnt)->y ;
       p3dP->z = (tinP->pointsP+firstPnt)->z ;
       ++p3dP ;
/*
** Scan Dtm Feature And Copy Remaining Points
*/
       listPtr = (tinP->nodesP+firstPnt)->fPtr ;
       while ( listPtr != tinP->nullPtr )
         {
          while ( listPtr != tinP->nullPtr  && (tinP->fListP+listPtr)->dtmFeature != dtmFeature ) listPtr = (tinP->fListP+listPtr)->nextPtr ;
          if( listPtr != tinP->nullPtr )
            {
             nextPnt = (tinP->fListP+listPtr)->nextPnt ;
             if( nextPnt != tinP->nullPnt )
               {
                p3dP->x = (tinP->pointsP+nextPnt)->x ;
                p3dP->y = (tinP->pointsP+nextPnt)->y ;
                p3dP->z = (tinP->pointsP+nextPnt)->z ;
                ++p3dP ;
                listPtr = (tinP->nodesP+nextPnt)->fPtr ;
               }
             if( nextPnt == tinP->nullPnt || nextPnt == firstPnt )  listPtr = tinP->nullPtr ; 
            }
         } 
       *numFeaturePtsP = numPts ;
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


  
 
