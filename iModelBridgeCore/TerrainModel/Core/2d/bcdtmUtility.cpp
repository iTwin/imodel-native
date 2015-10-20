/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmUtility.cpp $
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
BENTLEYDTM_EXPORT int bcdtmUtility_setGeopakActive(void)
{
 int dbg=DTM_TRACE_VALUE(0) ;
/*
** Set Geopak Active
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Geopak Active") ;
 DTM_GEOPAK_ACTIVE = 1 ;
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
BENTLEYDTM_EXPORT int bcdtmUtility_getDtmStateDtmObject (BC_DTM_OBJ *dtmP, DTMState *dtmStateP)
/*
** This Is A Special Purpose API Function
** Returns The DTM State To The API
** dtmStateP   <==    1  Data State
**             <==    2  Tin State
**             <==    3  Mixed State  - Dtm Is Triangulated but dtmFeatures have been added since the dtm was triangulated
**             <==    4  Invalid State - Dtm Not In An State That Can be exposed at the API level
**
*/ 
{
 long dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *dtmStateP = DTMState::TinError ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Set State
*/
 if     ( dtmP->dtmState == DTMState::Data ) *dtmStateP = DTMState::PointsSorted ;
 else if( dtmP->dtmState == DTMState::Tin  )
   {
    *dtmStateP = DTMState::DuplicatesRemoved ;
/*
**  Scan Features To Determine If Any Features Have Been Added Since Triangulation
*/
    if( dtmP->numFeatures > 0 )   
      {
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && *dtmStateP == DTMState::DuplicatesRemoved ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray ) *dtmStateP =  DTMState::Tin ;
         }
      }
   } 
/*
**  Check For Valid DTM State
*/
 if(  *dtmStateP == DTMState::TinError  )
   {
    bcdtmWrite_message(2,0,0,"Invalid DTM API State") ;
    goto errexit ;
   }
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtility_getLastDtmErrorMessage(long *errorStatusP,long *errorNumberP,char **errorMessagePP)
/*
** This function gets the last dtm error number and message 
**
*/
{
/*
** Initialise
*/
 if( *errorMessagePP != NULL ) { free(*errorMessagePP) ; *errorMessagePP = NULL ; }
 *errorStatusP = DTM_DTM_ERROR_STATUS ;
 *errorNumberP = DTM_DTM_ERROR_NUMBER ;
 *errorMessagePP = ( char *) malloc ((strlen(DTM_DTM_ERROR_MESSAGE)+1)*sizeof(char)) ;
 if( *errorMessagePP == NULL ) goto errexit ;
 strcpy(*errorMessagePP,DTM_DTM_ERROR_MESSAGE) ;
/*
** Job Completed
*/
 return(DTM_SUCCESS) ;
/*
** Error Exit
*/
 errexit :
 return(DTM_ERROR) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtility_getCurrentDtmObject(BC_DTM_OBJ **dtmP,WCharCP dtmFileP)
/*
** This function Gets The Pointer To The current DTM Object
*/
{
/*
** Set Dtm Object To Current Dtm Object
*/
// *dtmP = DTM_CDTM ;
// wcscpy(dtmFileP,DTM_CDTM_FILE) ;
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
BENTLEYDTM_EXPORT int bcdtmUtility_setCurrentDataObject(DTM_DAT_OBJ *dtmP,WCharCP dtmFileP)
/*
** This function sets the current Tin Object
** Call By gpk DTM To Set The Current Geopk Tin Object 
** So It Not Destroyed By IbcDTM
**
*/
{
 int ret=DTM_SUCCESS ;
/*
** Set Current Tin Object
*/
/*
 DTM_CDOBJ = dtmP ;
 if( *dtmFileP != 0 )  wcscpy(DTM_CDOBJ_FILE,dtmFileP) ;
 else                  wcscpy(DTM_CDOBJ_FILE,L"MEMORY.DTM") ;
*/ 
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
BENTLEYDTM_EXPORT int bcdtmUtility_setCurrentTinObject(DTM_TIN_OBJ *dtmP,WCharCP dtmFileP)
/*
** This function sets the current Tin Object
** Call By gpk DTM To Set The Current Geopk Tin Object 
** So It Not Destroyed By IbcDTM
**
*/
{
 int ret=DTM_SUCCESS ;
/*
** Set Current Tin Object
*/
/*
 DTM_CTOBJ = dtmP ;
 if( *dtmFileP != 0 )  wcscpy(DTM_CTOBJ_FILE,dtmFileP) ;
 else                  wcscpy(DTM_CTOBJ_FILE,L"MEMORY.DTM") ;
*/ 
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
BENTLEYDTM_EXPORT int bcdtmUtility_setCurrentDtmObject(BC_DTM_OBJ *dtmP,WCharCP dtmFileP)
/*
** This function sets the current Dtm Object
**
*/
{
 int ret=DTM_SUCCESS ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Delete Current Dtm Object If It Exists
*/
/*
 if( DTM_CDTM != NULL && DTM_CDTM != dtmP ) 
   {
    if( bcdtmObject_destroyDtmObject(&DTM_CDTM)) goto errexit ;
   }
*/   
/*
** Set Current Dtm Object
*/
/*
 DTM_CDTM = dtmP ;
 if( *dtmFileP != 0 )  wcscpy(DTM_CDTM_FILE,dtmFileP) ;
 else                  wcscpy(DTM_CDTM_FILE,L"MEMORY.DTM") ;
*/ 
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
BENTLEYDTM_EXPORT int bcdtmUtility_destroyCurrentDtmObject(void)
/*
** This function sets the current Dtm Object
**
*/
{
 int ret=DTM_SUCCESS ;
 goto errexit ;
/*
** Delete Current Dtm Object If It Exists
*/
/*
 if( DTM_CDTM != NULL ) 
   {
    if( bcdtmObject_destroyDtmObject(&DTM_CDTM)) goto errexit ;
   }
*/   
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
BENTLEYDTM_EXPORT int bcdtmUtility_testForAndSetCurrentDtmObject(BC_DTM_OBJ **dtmPP,WCharCP dtmFileP)
/*
**
** Test If The Current Dtm File Is The Requested Dtm File.
** If Not The Requested Dtm File Is Read Into Memory.
**
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Testing For And Setting Current Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmPP         = %p",*dtmPP) ;
    bcdtmWrite_message(0,0,0,"dtmFileP      = %s",dtmFileP) ;
//    bcdtmWrite_message(0,0,0,"DTM_CDTM      = %p",DTM_CDTM) ;
//    bcdtmWrite_message(0,0,0,"DTM_CDTM_FILE = %s",DTM_CDTM_FILE) ;
   }
 goto errexit ;  
/*
** Test If Current Dtm File Is The Requested File
*/
/*
 if( wcscmp(dtmFileP,DTM_CDTM_FILE) != 0 || DTM_CDTM == NULL )
   {
    if( DTM_CDTM != NULL ) bcdtmObject_destroyDtmObject(&DTM_CDTM) ;
    DTM_CDTM_FILE[0] = 0 ;
    if( bcdtmRead_fromFileDtmObject(dtmPP,dtmFileP) ) 
      { 
       if( *dtmPP != NULL ) bcdtmObject_destroyDtmObject(dtmPP) ;
       goto errexit ; 
      }
    wcscpy(DTM_CDTM_FILE,dtmFileP) ;
    DTM_CDTM = *dtmPP ;
   }
*/   
/*
** Requested File Is Current File
*/
// else *dtmPP = DTM_CDTM ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For And Setting Current Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For And Setting Current Dtm Object Completed") ;
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
BENTLEYDTM_EXPORT int bcdtmUtility_getBoundingCubeDtmObject(BC_DTM_OBJ *dtmP,double *xMinP,double *yMinP,double *zMinP,double *xMaxP,double *yMaxP,double *zMaxP,double *xRangeP,double *yRangeP,double *zRangeP)
/*
** This Function Gets The Bounding Cube For A DTM Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 static int firstCall=1 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Log DTM Memory Array Parameters
*/ 
 if( dbg == 1 && firstCall )
   {
    bcdtmWrite_message(0,0,0,"Reading Bounding Cube ** dtmP = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP->fTablePP = %p ** numFeatures = %8ld memFeatures = %8ld",dtmP->fTablePP,dtmP->numFeatures,dtmP->memFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->pointsPP = %p ** numPoints   = %8ld memPoints   = %8ld",dtmP->pointsPP,dtmP->numPoints,dtmP->memPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->nodesPP  = %p ** numNodes    = %8ld memNodes    = %8ld",dtmP->nodesPP,dtmP->numNodes,dtmP->memNodes ) ;
    bcdtmWrite_message(0,0,0,"dtmP->cListPP  = %p ** numClist    = %8ld memClist    = %8ld",dtmP->cListPP,dtmP->numClist,dtmP->memClist ) ;
    bcdtmWrite_message(0,0,0,"dtmP->fListPP  = %p ** numFlist    = %8ld memFlist    = %8ld",dtmP->fListPP,dtmP->numFlist,dtmP->memFlist ) ;
    bcdtmWrite_message(0,0,0,"dtmP->cListPtr = %10ld ** dtmP->cListDelPtr  = %10ld ** dtmP->fListDelPtr = %10ld",dtmP->cListPtr,dtmP->cListDelPtr,dtmP->fListDelPtr ) ;
   }
/*
** Catch And Report DTM
*/ 
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Getting Bounding Cube dtmP = %p dtmP->dtmState = %2ld dtmP->dtmCleanUp = %2ld dtmP->numPoints = %8ld dtmP->numFeatures = %8ld",dtmP,dtmP->dtmState,dtmP->dtmCleanUp,dtmP->numPoints,dtmP->numFeatures) ;  
    if( firstCall ) bcdtmWrite_toFileDtmObject(dtmP,L"boundingCube.bcdtm") ;
    if( dbg == 2 ) bcdtmObject_reportStatisticsDtmObject(dtmP) ;
    firstCall = 0  ;
   }
/*
** Check DTM
*/    
 if( cdbg )
   {
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) 
      {
       bcdtmWrite_message(1,0,0,"Tin Invalid ** dtmP = %p",dtmP) ;
       goto errexit ;
      } 
    else  bcdtmWrite_message(0,0,0,"Tin Valid ** dtmP = %p",dtmP) ;
   }
/*
** Set Values
*/
 *xMinP = dtmP->xMin ; *xMaxP = dtmP->xMax ; *xRangeP = dtmP->xRange ;
 *yMinP = dtmP->yMin ; *yMaxP = dtmP->yMax ; *yRangeP = dtmP->yRange ;
 *zMinP = dtmP->zMin ; *zMaxP = dtmP->zMax ; *zRangeP = dtmP->zRange ;
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
BENTLEYDTM_EXPORT int bcdtmUtility_getStatisticsDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMState& dtmState,
 long& numPoints,
 long& numTinLines,
 long& numTriangles,
 long& clistSize,
 long& flistSize,
 long& numDtmFeatures,
 long& numBreaks,
 long& numContourLines,
 long& numVoids,
 long& numIslands,
 long& numHoles,
 long& numGroupSpots,
 bool& hasHull,
 long& dtmMemorySize
 )
/*
** This Function Gets Dtm Object Statistics
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Statistics For Dtm Object %p",dtmP) ; 
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Set Initial Values
*/
 dtmState        = dtmP->dtmState ;
 numPoints       = dtmP->numPoints ;
 numTinLines     = dtmP->numLines  ;
 numTriangles    = dtmP->numTriangles  ;
 clistSize       = dtmP->cListPtr ;
 numDtmFeatures  = dtmP->numFeatures ;
 flistSize       = dtmP->numFlist ;
 dtmMemorySize   = sizeof(BC_DTM_OBJ) + numPoints * sizeof(DPoint3d) + clistSize * sizeof(DTM_CIR_LIST) + numDtmFeatures * sizeof(DTM_FEATURE_TABLE) + flistSize * sizeof(DTM_FEATURE_LIST) ;
 numBreaks       = numContourLines = numVoids = numIslands = numHoles = numGroupSpots = 0 ;
 hasHull = false;
/*
** Count Number Of DTM Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature)
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureState != DTMFeatureState::TinError )
      {
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline )         ++numBreaks ; 
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::ContourLine )  ++numContourLines  ; 
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )         ++numVoids; 
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )       ++numIslands  ; 
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole )         ++numHoles  ; 
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots )   ++numGroupSpots  ; 
       else if (dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull)          hasHull = true;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Statistics For Dtm Object %p Completed",dtmP) ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Statistics For Dtm Object %p Error",dtmP) ; 
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
BENTLEYDTM_Public int bcdtmUtility_writeStatisticsDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Writes DTM Object Statistics To The Dtm Log File
*/
{
 int    ret=DTM_SUCCESS ;
 DTMState   dtmState ;
 long   numPoints,numTinLines,numTriangles,clistSize,numDtmFeatures ;
 long   flistSize,numBreaks,numContourLines,numVoids,numIslands,numHoles ;
 long   dtmMemorySize,numGroupSpots ;
 bool hasHull;
 double Xmin,Ymin,Zmin,Xmax,Ymax,Zmax,Xdif,Ydif,Zdif ;
/*
** Get dtmP Object Statistics
*/
 if( bcdtmUtility_getStatisticsDtmObject(dtmP,dtmState,numPoints,numTinLines,numTriangles,clistSize,flistSize,numDtmFeatures,numBreaks,numContourLines,numVoids,numIslands,numHoles,numGroupSpots,hasHull, dtmMemorySize) ) goto errexit ;
/*
** Write Values
*/
 bcdtmWrite_message(0,0,0,"Statistics For Dtm Object %p",dtmP) ;
 bcdtmWrite_message(0,0,0,"Dtm State                    =  %10ld",dtmP->dtmState) ;
 bcdtmWrite_message(0,0,0,"Number Of Points             =  %10ld",numPoints) ;
 bcdtmWrite_message(0,0,0,"Number Of Tin Lines          =  %10ld",numTinLines) ;
 bcdtmWrite_message(0,0,0,"Number Of Triangles          =  %10ld",numTriangles) ;
 bcdtmWrite_message(0,0,0,"Number Of Dtm Features       =  %10ld",numDtmFeatures) ;
 bcdtmWrite_message(0,0,0,"Number Of Dtm Feature Points =  %10ld",flistSize) ;
 bcdtmWrite_message(0,0,0,"Number Of Break Lines        =  %10ld",numBreaks) ;
 bcdtmWrite_message(0,0,0,"Number Of Contour Lines      =  %10ld",numContourLines) ;
 bcdtmWrite_message(0,0,0,"Number Of Voids              =  %10ld",numVoids) ;
 bcdtmWrite_message(0,0,0,"Number Of Islands            =  %10ld",numIslands) ;
 bcdtmWrite_message(0,0,0,"Number Of Holes              =  %10ld",numHoles) ;
 bcdtmWrite_message(0,0,0,"Number Of Group Spots        =  %10ld",numGroupSpots) ;
 bcdtmWrite_message(0,0,0,"Size Of Circular List        =  %10ld",clistSize) ;
 bcdtmWrite_message(0,0,0,"Memory Size Of Dtm Object    =  %10ld",dtmMemorySize) ;

 bcdtmWrite_message(0,0,0,"dtmP->hullPoint              =  %10ld",dtmP->hullPoint) ;
/*
** Get Data Ranges
*/
 if( bcdtmUtility_getBoundingCubeDtmObject(dtmP,&Xmin,&Ymin,&Zmin,&Xmax,&Ymax,&Zmax,&Xdif,&Ydif,&Zdif)) goto errexit ;
/*
** Write Bounding Cube Values
*/
 bcdtmWrite_message(0,0,0,"DTM Coordinate Ranges") ;
 bcdtmWrite_message(0,0,0,"") ;
 bcdtmWrite_message(0,0,0,"    Minimum      Maximum       Range")     ;
 bcdtmWrite_message(0,0,0,"  ============ ============ ============") ;
 bcdtmWrite_message(0,0,0,"x %12.3lf %12.3lf %12.3lf",Xmin,Xmax,Xdif) ;
 bcdtmWrite_message(0,0,0,"y %12.3lf %12.3lf %12.3lf",Ymin,Ymax,Ydif) ;
 bcdtmWrite_message(0,0,0,"z %12.3lf %12.3lf %12.3lf",Zmin,Zmax,Zdif) ;
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
BENTLEYDTM_Public int bcdtmUtility_copyTinDtmFeatureTypeToPolygonObjectDtmObject(BC_DTM_OBJ *dtmP,DTM_POLYGON_OBJ *polyP,DTMFeatureType dtmFeatureType,DTMUserTag userTag)
/*
** This Function Copies All Features Of A Dtm Feature Type To A Polygon Object
*/
{
 int   ret=DTM_SUCCESS ;
 long  dtmFeature ; 
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Test For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test For Valid Polygon Object
*/
 if( bcdtmPolygon_testForValidPolygonObject(polyP)) goto errexit ;
/*
** Scan Feature Table And Copy Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == dtmFeatureType )
      {
       if( bcdtmUtility_copyTinDtmFeatureToPolygonObjectDtmObject(dtmP,polyP,dtmFeature,userTag)) goto errexit ;
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
BENTLEYDTM_Public int bcdtmUtility_copyTinDtmFeatureToPolygonObjectDtmObject(BC_DTM_OBJ *dtmP,DTM_POLYGON_OBJ *polyP,long dtmFeature,DTMUserTag userTag)
/*
** This Function Copies a DTM Feature To A Polygon Object
*/
{
 int  ret=DTM_SUCCESS ;
 long startPnt ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Test For Valid Polygon Object
*/
 if( bcdtmPolygon_testForValidPolygonObject(polyP)) goto errexit ;
/*
** Test For Valid Dtm Feature
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures ) 
   { 
    bcdtmWrite_message(2,0,0,"Dtm Feature Range Error") ;
    goto errexit ;
   }
/*
** Copy Feature To Tptr Polygon
*/
 if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&startPnt)) goto errexit ;
/*
** Store Tptr Polygon In Polygon Object
*/
 if( bcdtmPolygon_storeTptrPolygonInPolygonObjectDtmObject(dtmP,polyP,startPnt,(long)userTag)) goto errexit ;
/*
** Null Out Tptr Polygon
*/
 bcdtmList_nullTptrListDtmObject(dtmP,startPnt) ;
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
BENTLEYDTM_EXPORT int bcdtmUtility_convertMbsToWcs(char *mbsP, wchar_t **wcsPP ) 
{
 int ret=DTM_SUCCESS ;
 size_t size ;
/*
** Convert To Wide Characters
*/
 if( *wcsPP != NULL ) { free( *wcsPP ) ; *wcsPP = NULL ; }
 size = strlen(mbsP) + 1 ;
 *wcsPP = ( wchar_t * ) malloc( size * sizeof(wchar_t)) ;
 if( *wcsPP == NULL ) 
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   } 
 mbstowcs(*wcsPP,mbsP,size) ;
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
BENTLEYDTM_EXPORT int bcdtmUtility_convertWcsToMbs( WCharCP wcsP, char **mbsPP ) 
{
 int ret=DTM_SUCCESS ;
 size_t size ;
/*
** Convert To Wide Characters
*/
 if( *mbsPP != NULL ) { free( *mbsPP ) ; *mbsPP = NULL ; }
 size = wcslen(wcsP) + 1 ;
 *mbsPP = ( char * ) malloc( size * sizeof(char)) ;
 if( *mbsPP == NULL ) 
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   } 
 wcstombs(*mbsPP,wcsP,size) ;
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
|											                         |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtility_boreHoleApp00DtmObject
(
 BC_DTM_OBJ  *groundDtmP ,
 BC_DTM_OBJ  *materialDtmP 
)
{
 int           ret=DTM_SUCCESS ;
 long          point,drapeFlag ;
 double        mz ;
 DPoint3d *pntP ;
/*
** Check For Valid DTM Objects
*/
 if( bcdtmObject_testForValidDtmObject(groundDtmP)) goto errexit ; 
 if( bcdtmObject_testForValidDtmObject(materialDtmP)) goto errexit ; 
/*
** Scan Ground Tin Points And Drape On Material Tin
*/
 for( point = 0 ; point < groundDtmP->numPoints ; ++point )
   {
    pntP = pointAddrP(groundDtmP,point) ;
    if( bcdtmDrape_pointDtmObject(materialDtmP,pntP->x,pntP->y,&mz,&drapeFlag)) goto errexit ;
    if( drapeFlag == 1 ) pntP->z = mz ;
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
BENTLEYDTM_Public int bcdtmUtility_setNullValuesForBackwardsCompatibilityTinObject
(
 DTM_TIN_OBJ          *tinP
)
{ 
 int  dbg=DTM_TRACE_VALUE(0) ;
 long offset, tinNullPnt=TIN_NULL_PNT,tinNullPtr=TIN_NULL_PTR ;
/*
** Reset Null Point And Null PTR Values
*/
 if( tinP->nullPnt != tinNullPnt || tinP->nullPtr != tinNullPtr )
   {
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Resetting Tin Null Pnt and Null Ptr Values") ;
/*
**  Scan Node And Reset Null Values
*/
    if( tinP->nodesP != NULL )
      {
       for( offset = 0 ; offset < tinP->memPts ; ++offset )
         {
          if( (tinP->nodesP+offset)->tPtr  == tinP->nullPnt ) (tinP->nodesP+offset)->tPtr  = tinNullPnt ;         
          if( (tinP->nodesP+offset)->sPtr  == tinP->nullPnt ) (tinP->nodesP+offset)->sPtr  = tinNullPnt ;         
          if( (tinP->nodesP+offset)->hPtr  == tinP->nullPnt ) (tinP->nodesP+offset)->hPtr  = tinNullPnt ;         
          if( (tinP->nodesP+offset)->cPtr  == tinP->nullPtr ) (tinP->nodesP+offset)->cPtr  = tinNullPtr ;         
          if( (tinP->nodesP+offset)->fPtr  == tinP->nullPtr ) (tinP->nodesP+offset)->fPtr  = tinNullPtr ;         
         }
      }
/*
**  Scan Circular List And Reset Null Values
*/
    if( tinP->cListP != NULL )
      {
       for( offset = 0 ; offset < tinP->cListPtr ; ++offset )
         {
          if( (tinP->cListP+offset)->pntNum  == tinP->nullPnt ) (tinP->cListP+offset)->pntNum  = tinNullPnt ;
          if( (tinP->cListP+offset)->nextPtr == tinP->nullPtr ) (tinP->cListP+offset)->nextPtr = tinNullPtr ;
         }
      }
/*
**  Scan Feature List And Reset Null Values
*/
    if( tinP->fListP != NULL )
      {
       for( offset = 0 ; offset < tinP->memFeatureList ; ++offset )
         {
          if( (tinP->fListP+offset)->nextPnt == tinP->nullPnt ) (tinP->fListP+offset)->nextPnt = tinNullPnt ;
          if( (tinP->fListP+offset)->nextPtr == tinP->nullPtr ) (tinP->fListP+offset)->nextPtr = tinNullPtr ;
         }
      }
/*
**  Reset Null Pnt And Null Ptr Values
*/
    if( tinP->cListDelPtr       == tinP->nullPtr ) tinP->cListDelPtr       = tinNullPtr ;
    if( tinP->cListLastDelPtr   == tinP->nullPtr ) tinP->cListLastDelPtr   = tinNullPtr ;
    if( tinP->featureListDelPtr == tinP->nullPtr ) tinP->featureListDelPtr = tinNullPtr ;
    tinP->nullPnt = tinNullPnt ;
    tinP->nullPtr = tinNullPtr ;
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
BENTLEYDTM_Public int bcdtmUtility_setNullValuesForForwardsCompatibilityTinObject
(
 DTM_TIN_OBJ          *tinP
)
{ 
 int  dbg=DTM_TRACE_VALUE(0) ;
 long offset ;
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
** Return
*/
 return(DTM_SUCCESS) ;    
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT long bcdtmClock(void)
/*
** Wrapper function To Isolate The Timing Functions For The Hand Held Applications
*/
{
#ifdef _WIN32_WCE
 return(0) ;
#else   
 return((long)clock()) ;
#endif
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT double bcdtmClock_elapsedTime(long Finish,long Start)
/*
** Wrapper function To Isolate The Timing Functions For The Hand Held Applications
*/
{
#ifdef _WIN32_WCE
 return(0.0) ;
#else   
 return((double)(Finish-Start)/CLOCKS_PER_SEC) ;
#endif
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmUtl_copy3DTo3D(DPoint3d *Points3D,long NumPoints,DPoint3d **NewPoints3D)
/*
** This Function Copies a 3D String To A 3D String
*/
{
 DPoint3d  *p3d1,*p3d2 ;
/*
** Allocate Memory
*/
 *NewPoints3D = ( DPoint3d * ) malloc( NumPoints * sizeof(DPoint3d)) ;
 if( *NewPoints3D == NULL )
   { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return(1) ; }
/*
** Copy Points
*/
 for( p3d1 = *NewPoints3D, p3d2 = Points3D ; p3d2 < Points3D + NumPoints ; ++p3d2, ++p3d1 )
   *p3d1 = *p3d2 ;
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
BENTLEYDTM_Public int bcdtmUtl_getDateAndTime(char *dstr, char *tstr)
{
#ifndef _WIN32_WCE
 __time32_t ltime ;
 _time32(&ltime) ;
 strcpy(dstr,_ctime32(&ltime)) ;
 memcpy(tstr,dstr+11,8) ;
 *(dstr+24) = 0 ;
 *(tstr+8)  = 0 ;
#else
 *dstr = 0 ;
 *tstr = 0 ;
#endif
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_detectXYZFileType( WCharCP fileName,long *fileType ,long *fileEntries)
/*
** Routine to automatically detect data file type
** Return Values == 0   Success
**               == 1   Cannot Open Data File
** Arguements
**   ==> fileName  char *  Input  Data File Name
**   <== fileType  long *  Output File Type
**               == 1  Ascii  Data File
**               == 2  Binary Data File
*/
{
 int  ret=DTM_SUCCESS ;
 long ascCnt=0,binCnt=0,charCnt=0,lfCnt=0,numChar=0 ;
 char cVal ;
 FILE *fpTmp=NULL ;
/*
**  Open File
*/
 if( (fpTmp = bcdtmFile_open(fileName,L"rb")) == NULL )
   {
    bcdtmWrite_message(1,0,0,"Cannot Open XYZ File %ws For Reading",fileName) ;
    goto errexit ;
   }
/*
** Read First 512 Characters
*/
 while( fread(&cVal,1,1,fpTmp) == 1 && charCnt < 512 )
   {
    if( cVal >= 32 && cVal <=127 ) ++ascCnt ;
    else                           ++binCnt ;
    if( cVal == '\r' || cVal == '\n' ) --binCnt ;
    if( cVal == '\n' ) ++lfCnt ; 
    ++charCnt ;
   }
 if( charCnt == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Data File %s is empty",fileName) ;
    goto errexit; 
   }
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
 fpTmp = NULL ;
/*
** Set Return Values
*/
 if( binCnt > 0 ) { *fileType = 2 ; *fileEntries = numChar / 24  ; }
 else             { *fileType = 1 ; *fileEntries = numChar / lfCnt + 10  ; } 
/*
** Check Binary File For Correct Size
*/
 if( *fileType == 2 )
   {
    if( numChar % 24 != 0 ) 
      {
       bcdtmWrite_message(1,0,0,"Size Error In Binary XYZ File %ws",fileName) ;
       goto errexit;
      } 
   } 
/*
** Clean Up
*/
 cleanup :
 if( fpTmp != NULL ) { fclose(fpTmp) ; fpTmp = NULL ; }
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
BENTLEYDTM_Public double bcdtmUtl_getTrgMax(double x[])
{
 double max ;
 max = x[0] ;
 if( x[1] > max ) max = x[1] ;
 if( x[2] > max ) max = x[2] ;
 return(max) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmUtl_getTrgMin(double x[])
{
 double min ;
 min = x[0] ;
 if( x[1] < min ) min = x[1] ;
 if( x[2] < min ) min = x[2] ;
 return(min) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmUtl_adjustValueDown(double Val,double Vreg,double Vinc)
{
 double Dfactor ;
 long   Lfactor ;
 Dfactor = ( Val - Vreg ) / Vinc ;
 if ( Dfactor < 0.0 )  --Dfactor ;
 Lfactor = ( long   ) Dfactor ; 
 Dfactor = ( double ) Lfactor ;
 return( Vreg + Dfactor * Vinc ) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public double bcdtmUtl_adjustValueUp(double Val,double Vreg,double Vinc)
{
 double Dfactor ;
 long   Lfactor ;
 Dfactor = ( Val - Vreg ) / Vinc ;
 if ( Dfactor > 0.0 )  ++Dfactor ;
 Lfactor = ( long   ) Dfactor ; 
 Dfactor = ( double ) Lfactor ;
 return( Vreg + Dfactor * Vinc ) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_freeMemory(void **Pointer)
/*
** This Function Frees Memory Allocated By The DLL
*/
{
 int dbg=DTM_TRACE_VALUE(0) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Freeing Pointer = %p",*Pointer) ;
/*
** Free Memory
*/
if( *Pointer != NULL ) { free(*Pointer) ; *Pointer = NULL ; }
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
BENTLEYDTM_EXPORT int bcdtmUtl_mallocMemory(void **Pointer,long MemoryAmount)
/*
** This Function Mallocs By The DLL
*/
{
/*
** Free Memory
*/
 *Pointer = ( void * ) malloc(MemoryAmount) ; 
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   bcdtmUtl_decodeXYZRecord()                                       |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_decodeXYZRecord(char *inbuf,double *x,double *y,double *z)
{
 int   i ;
 char  *bp,ibuf[100] ;
/*
** Scan and Decode Data Record
*/
 bp = inbuf ;
 for ( i = 0 ; i < 3 ; ++i )
   {
    if( ! bcdtmUtl_getNextString(&bp,ibuf)) return(DTM_ERROR) ;
    else
      {
       if( ! bcdtmUtl_checkReal(ibuf)) return(DTM_ERROR) ;
       if( i == 0 ) sscanf(ibuf,"%lf",x) ;
       if( i == 1 ) sscanf(ibuf,"%lf",y) ;
       if( i == 2 ) sscanf(ibuf,"%lf",z) ;
      }
   }
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_checkInteger(char *istr)
{
 long l,blnkflag=0,minflag=0 ;
 char *sp ;
 l = (long) strlen(istr) ;
 for( sp = istr ; sp < istr + l ; ++sp )
   {
    if( *sp != ' ')
      {
       blnkflag = 1 ;
       if( *sp == '-')
         if( minflag ) return(0) ; else minflag = 1 ;
       if( *sp < '0' || *sp > '9') return(0) ;
      }
    else if( blnkflag ) return(0) ;
   }
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_checkReal(char *istr)
{
 long l,blnkflag=0,dotflag=0,minflag=0 ;
 char *sp ;
 l = (long)strlen(istr) ;
 for( sp = istr ; sp < istr + l ; ++sp )
   {
    if( *sp != ' ')
      {
       blnkflag = 1 ;
       if( *sp == '-')
         {
          if( minflag ) return(0) ;
          minflag = 1 ; blnkflag = 0 ;
         }
       else
         {
          if( *sp == '.' )
            {
             if( dotflag ) return(0) ;
             dotflag = 1 ;
            }
          else  if( *sp < '0' || *sp > '9') return(0) ;
         }
      }
    else if( blnkflag ) return(0) ;
   }
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_getNextString(char **sptr, char *string)
{
 char *sp ;
 sp = string ; *sp = 0 ;
 while ( **sptr == ' ' ) ++*sptr ;
 while ( **sptr != ' ' && **sptr != 0 )
   { *sp = **sptr ; ++sp ; ++*sptr ; }
 *sp = 0 ;
 if( sp == string ) return(0) ; else return(1) ;
}
/*---------------------------------------------------------+
|                                                          |
|                                                          |
|                                                          |
+---------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmConvert_eliminateBlanks( char *msgStr )
{
 char *c1P,*c2P ;
 for( c1P = c2P = msgStr ; *c2P != 0 ; ++c2P )
   {
    if( *c2P != ' ' || *(c2P-1) != ' ' ) { *c1P = *c2P ; ++c1P ; }
   }
 *c1P = 0 ;
 return(0) ;
}
