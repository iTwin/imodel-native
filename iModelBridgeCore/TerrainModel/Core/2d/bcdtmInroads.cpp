/*
** bcdtmInroads.c - Used For Exporting bcDTM Tins To Inroad's DTM
*/
#include <windows.h>
#include <string.h>
#include <mbstring.h>
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtmInline.c"
#include "bcdtmclass.h"

#include "inroadstm.h"
#define MAXAREAFORVOIDORISLANDS 0.001

using namespace Bentley::DTM;
extern "C"
    {
    /*
** InRoads Function Prototypes
*/

typedef int (*tinStatsCallBackFunctionDef)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures);
typedef int (*tinRandomPointsCallBackFunctionDef)(long pntIndex,double X,double Y,double Z);
typedef int (*tinFeaturePointsCallBackFunctionDef)(long pntIndex,double X,double Y,double Z);
typedef int (*tinTrianglesCallBackFunctionDef)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex);
typedef int (*tinFeaturesCallBackFunctionDef)(long dtmFeatureType,__int64 BcDTMUserTag,__int64 BcDTMFeatureId,long *pointIndicesP,long numPointIndices);

int inroadsTM_setConvertGPKTinToDTMFunction (int (*callBackFunctionP)(
 wchar_t *tinFileNameP,
 tinStatsCallBackFunctionDef tinStatsCallBackFunctionP,
 tinRandomPointsCallBackFunctionDef tinRandomPointsCallBackFunctionP,
 tinFeaturePointsCallBackFunctionDef tinFeaturePointsCallBackFunctionP,
 tinTrianglesCallBackFunctionDef tinTrianglesCallBackFunctionP,
 tinFeaturesCallBackFunctionDef tinFeaturesCallBackFunctionP));
int inroadsTM_convertGPKTinToDTM(wchar_t *tinFileNameP,wchar_t *dtmFileNameP,wchar_t *nameP,wchar_t *descriptionP, int version);

// Import callback delegate
void (__stdcall *pFeatureCallbackFunc)(DTM_FEATURE_ID featureId, LPWSTR featureDefinitionName, LPWSTR featureName, LPWSTR description, int featureType, P3D* points, int numPoints);

#ifdef DDD
int inroadsTM_convertDTMToGPKTin(wchar_t *dtmFileNameP,wchar_t *tinFileNameP,
    int(*bcdtmInRoads_importGeopakTinFromInroadsDtm)( double maxTriLength, long  numTinPoints, long  numTinFeatures, wchar_t  *geopakTinFileNameP, int  (*setGeopakCallBackFunctionsP)()));

typedef int pointsCallBackDef (BC_DTM_OBJ* dtm, double x, double y, double z);
typedef int circularListCallBackDef (BC_DTM_OBJ *dtmP,long pointIndex,long *cirPointIndexP,long numCirPointIndex );
typedef int featurePointsCallBackDef (BC_DTM_OBJ *dtmP,wchar_t* dtmFeatureName, wchar_t*   dtmFeatureDescription, wchar_t*   dtmFeatureStyle, long       dtmFeatureType, P3D        *dtmFeaturePointsP, long       numDtmFeaturePoints, long       excludeFromTriangulation);

#endif
int dtmLink_setConvertGPKTinToDTMFunction (int (*callBackFunctionP)(
 wchar_t *tinFileNameP,
 int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures),
 int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
 int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
 int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
 int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 BcDTMUserTag,__int64 BcDTMFeatureId,long *pointIndicesP,long numPointIndices)
)
)
    {
     return inroadsTM_setConvertGPKTinToDTMFunction (callBackFunctionP);
    }
int dtmLink_convertGPKTinToDTM(wchar_t *tinFileNameP,wchar_t *dtmFileNameP,wchar_t *nameP,wchar_t *descriptionP)
    {
    return inroadsTM_convertGPKTinToDTM(tinFileNameP, dtmFileNameP, nameP, descriptionP, 0);
    }
#ifdef DD
int dtmLink_convertDTMToGPKTin(wchar_t *dtmFileNameP,wchar_t *tinFileNameP, int(*bcdtmInRoads_importGeopakTinFromInroadsDtm)( double maxTriLength, long  numTinPoints, long  numTinFeatures, wchar_t  *geopakTinFileNameP, int  (*setGeopakCallBackFunctionsP)()))
    {
    return inroadsTM_convertDTMToGPKTin (dtmFileNameP, tinFileNameP, bcdtmInRoads_importGeopakTinFromInroadsDtm);
    }
#endif
/*
** Global Variables
*/
static long glbImportMode=0 ;
static BC_DTM_OBJ *glbDtmP=0 ;
long  numVoidTriangles=0 ;
long  numNoneVoidTriangles=0 ;
#ifdef NN
/*
nativeCode int dtmLink_setConvertGPKTinToDTMFunction
(
    int (*pFunc)
    (
        char *tinFileNameP,
        int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures),
        int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
        int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
        int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
        int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 BcDTMUserTag,__int64 BcDTMFeatureId,long *pointIndicesP,long numPointIndices)
    )
)
{
    aecDTM_setConvertGPKTinToDTMFunction( pFunc );
    return SUCCESS;
}
int dtmLink_convertGPKTinToDTM
(
    wchar_t *tinFileNameP,
    wchar_t *dtmFileNameP,
    wchar_t *name,
    wchar_t *description
)
{
    return aecDTM_convertGPKTinToDTM( tinFileNameP, dtmFileNameP, name, description);
} ;

int dtmLink_convertDTMToGPKTin
(
    wchar_t *dtmFileNameP,
    wchar_t *tinFileNameP
)
{
    return aecDTM_convertDTMToGPKTin( dtmFileNameP, tinFileNameP);
} ;

*/


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
NATDLLEXP int bcdtmInRoads_importBclibDtmFromInroadsDtmFileOld(BC_DTM_OBJ **dtmPP, wchar_t *dtmFileNameP )
{
 int ret=DTM_SUCCESS,status=0,dbg=0 ;
 size_t  size ;
 wchar_t *w_tinFileNameP=NULL ;
 wchar_t *w_dtmFileNameP=NULL ;
 char tinFileName[10] ;
 long firstPnt,dtmFeature,cleanDTM=FALSE,removeVoid=FALSE ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
**  Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Importing bcLIB DTM From Inroads DTM File") ;
    bcdtmWrite_message(0,0,0,"dtmPP          = %p",*dtmPP) ;
    bcdtmWrite_message(0,0,0,"dtmFileNameP   = %ws",dtmFileNameP) ;
   }

/*
** Check For NULL Dtm Pointer
*/
 if( *dtmPP != NULL )
   {
    bcdtmWrite_message(2,0,0,"Requires NULL DTM pointer") ;
    goto errexit ;
   }
/*
** Convert To Wide Characters
*/
// size = mbstowcs(NULL,dtmFileNameP,strlen(dtmFileNameP)) ;
// if(( w_dtmFileNameP = malloc((size+1)*sizeof(wchar_t))) == NULL ) goto errexit ;
// mbstowcs(w_dtmFileNameP,dtmFileNameP,strlen(dtmFileNameP) + 1) ;

 strcpy(tinFileName,"xxxx") ;
 size = mbstowcs(NULL,tinFileName,strlen(tinFileName)) ;
 if(( w_tinFileNameP = (wchar_t*)malloc((size+1)*sizeof(wchar_t))) == NULL ) goto errexit ;
 mbstowcs(w_tinFileNameP,tinFileName,strlen(tinFileName) + 1) ;
/*
**  Set Golbal In Memory Mode Switch To Override The Creation Of A Geopak Tin File
**  And Set A Global Pointer To The DTM Object Created During The Import Process
*/
 glbImportMode = 1 ;
/*
**  Call Inroads Function To Initiate Import Of InRoads To Geopak Tin File
**  For The Purpose Of Importing Directly To An In Memory bcLIB DTM Object
**  We Will Use A Dummy Tin File Name And Pass The Pointer To The DTM
**
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calling Inroads Dtm") ;
// if( ( status = dtmLink_convertDTMToGPKTin (w_dtmFileNameP, w_tinFileNameP)) != DTM_SUCCESS )
 if( ( status = dtmLink_convertDTMToGPKTin (dtmFileNameP, w_tinFileNameP, &bcdtmInRoads_importGeopakTinFromInroadsDtm)) != DTM_SUCCESS )
   {
    if( status < 0 ) bcdtmWrite_message(1,0,0,"Failed To Load InRoads DLLs") ;
    else             bcdtmWrite_message(1,0,0,"Failed To Import Inroads DTM File ** Inroads Error = %6ld",status) ;
    goto errexit ;
   }
/*
** Set Poinet To Object
*/
 *dtmPP = glbDtmP ;
 if( *dtmPP == NULL ) goto errexit ;

// Remove Zero Area Voids - Fix For Defect 117426 - RobC 6 Feb 2013
  for( dtmFeature = 0 ; dtmFeature < (*dtmPP)->numFeatures ; ++dtmFeature )
    {
    removeVoid = FALSE ;
    dtmFeatureP = ftableAddrP(*dtmPP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMF_STATE_TIN && dtmFeatureP->dtmFeatureType == DTMF_VOID )
       {
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(*dtmPP,dtmFeature,&firstPnt)) goto errexit ;
       if( nodeAddrP(*dtmPP,nodeAddrP(*dtmPP,firstPnt)->tPtr)->tPtr == firstPnt ) removeVoid = TRUE ;
       if( bcdtmList_nullTptrListDtmObject(*dtmPP,firstPnt)) goto errexit ;
       if( removeVoid )
           {
           if( dbg ) bcdtmWrite_message(0,0,0,"Removing Void Feature %8ld",dtmFeature) ;
           cleanDTM = TRUE ;
           if( bcdtmInsert_removeDtmFeatureFromDtmObject(*dtmPP,dtmFeature)) goto errexit ;
           }
       }
   }
 if( cleanDTM )
    if( bcdtmList_cleanDtmObject(*dtmPP)) goto errexit ;

/*
** Clean Up
*/
 cleanup :
 glbDtmP = NULL ;
 glbImportMode = 0 ;
 if( w_dtmFileNameP != NULL ) free(w_dtmFileNameP) ;
 if( w_tinFileNameP != NULL ) free(w_tinFileNameP) ;
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing bcLIB DTM From Inroads DTM File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing bcLIB DTM From Inroads DTM File Error") ;
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
DTMPrivate int bcdtmInRoads_insertRectangleAroundTinDtmObject
(
 BC_DTM_OBJ *dtmP,
 double  xdec,                     /* ==> Decrement Tin Xmin                   */
 double  xinc,                     /* ==> Increment Tin Xmax                   */
 double  ydec,                     /* ==> Decrement Tin Ymin                   */
 double  yinc,                     /* ==> Increment Tin Ymax                   */
 DTM_FEATURE_ID *islandFeatureIdP  /* <== Island Feature Id For Prior Tin Hull */
 )
 /*
 ** This Function Adds A Surrounding Rectangle To A Triangulated DTM
 ** To Simulate An Inroads Triangulation In bcLIB DTM. This Is Done Prior To
 ** Exporting bcLIB Triangles To An Inroads Triangulation
 */
    {
    int ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
    long point,closePoint ;
    long dtmFeature,numHullPts,hullFeature,rectangleFeature,tinPoints[4] ;
    long priorPnt,startPnt,nextPnt,endPnt,clkPnt ;
    long cPriorPnt,cStartPnt,cNextPnt,cClkPnt ;
    long hullPnt ;
    P3D  rectanglePts[5],*hullPtsP=NULL ;
    BC_DTM_OBJ *tempDtmP=NULL ;
    BC_DTM_FEATURE *dtmFeatureP ;
    DPoint3d *pointP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Inserting Rectangle Around Tin") ;
        bcdtmWrite_message(0,0,0,"dtmP  = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"xdec  = %12.5lf",xdec) ;
        bcdtmWrite_message(0,0,0,"xinc  = %12.5lf",xinc) ;
        bcdtmWrite_message(0,0,0,"ydec  = %12.5lf",ydec) ;
        bcdtmWrite_message(0,0,0,"yinc  = %12.5lf",yinc) ;
        }
    /*
    ** Initialise
    */
    *islandFeatureIdP = nullFeatureId ;
    /*
    ** Test For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check DTM Is In Triangulated State
    */
    if( dtmP->dtmState != DTM_STATE_TIN )
        {
        bcdtmWrite_message(2,0,0,"Method Requires Triangulated Dtm") ;
        goto errexit ;
        }
    /*
    ** Write Stats Prior To Adding Rectangle
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Before Adding External Rectangle") ;
        bcdtmWrite_message(0,0,0,"dtmP->numPoints        = %8ld",dtmP->numPoints) ;
        bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints  = %8ld",dtmP->numSortedPoints) ;
        bcdtmWrite_message(0,0,0,"dtmP->numTriangles     = %8ld",dtmP->numTriangles) ;
        bcdtmWrite_message(0,0,0,"dtmP->numLines         = %8ld",dtmP->numLines) ;
        bcdtmWrite_message(0,0,0,"dtmP->numFeatures      = %8ld",dtmP->numFeatures) ;
        hullPnt = dtmP->hullPoint ;
        numHullPts = 0 ;
        do
          {
           ++numHullPts ;
           hullPnt = nodeAddrP(dtmP,hullPnt)->hPtr ;
          } while( hullPnt != dtmP->hullPoint ) ;
         ++numHullPts ;
         bcdtmWrite_message(0,0,0,"numHullPts            = %8ld",numHullPts) ;
        }
    /*
    ** Add Old Tin Hull As Island Feature
    */
     if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->hullPoint = %8ld",dtmP->hullPoint) ;
     if( bcdtmList_copyHptrListToTptrListDtmObject(dtmP,dtmP->hullPoint)) goto errexit ;
     if( dbg ) bcdtmWrite_message(0,0,0,"BB dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
     if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMF_ISLAND,-9999,dtmP->dtmFeatureIndex,dtmP->hullPoint,1)) goto errexit ;
     if( dbg ) bcdtmWrite_message(0,0,0,"AA dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
     *islandFeatureIdP = dtmP->dtmFeatureIndex ;
     dtmFeatureP = ftableAddrP(dtmP,dtmP->numFeatures-1) ;
     if( dbg ) bcdtmWrite_message(0,0,0,"islandFeatureIdP = %8ld",*islandFeatureIdP) ;
     if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeatureState = %4ld BcDTMFeatureId = %8I64d BcDTMUserTag = %8I64d ** dtmFeatureType = %4ld",dtmFeatureP->dtmFeatureState,dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureType) ;
     if( dtmFeatureP->dtmFeatureType != DTMF_ISLAND ) dtmFeatureP->dtmFeatureType = DTMF_ISLAND ;
     ++dtmP->dtmFeatureIndex ;
    /*
    ** Create Triangulation Of Existing Tin Hull And Surrounding Rectangle
    */
    if( bcdtmObject_createDtmObject(&tempDtmP) != DTM_SUCCESS ) goto errexit ;
    /*
    ** Extract Tin Hull
    */
    if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Hull Points = %6ld",numHullPts) ;
    /*
    ** Set Point Memory Allocation Parameters
    */
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,numHullPts*4+5,numHullPts) ;
    /*
    ** Store Tin Hull
    */
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMF_GRAPHIC_BREAK,1,1,&tempDtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMF_GRAPHIC_BREAK,1,1,&tempDtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMF_GRAPHIC_BREAK,1,1,&tempDtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMF_HARD_BREAK,1,1,&tempDtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    /*
    ** Store Surrounding Rectangle
    */
    rectanglePts[0].X = dtmP->xMin - xdec ; rectanglePts[0].Y = dtmP->yMin - ydec ; rectanglePts[0].Z = - 999 ;
    rectanglePts[1].X = dtmP->xMax + xinc ; rectanglePts[1].Y = dtmP->yMin - ydec ; rectanglePts[1].Z = - 999 ;
    rectanglePts[2].X = dtmP->xMax + xinc ; rectanglePts[2].Y = dtmP->yMax + yinc ; rectanglePts[2].Z = - 999 ;
    rectanglePts[3].X = dtmP->xMin - xdec ; rectanglePts[3].Y = dtmP->yMax + yinc ; rectanglePts[3].Z = - 999 ;
    rectanglePts[4].X = dtmP->xMin - xdec ; rectanglePts[4].Y = dtmP->yMin - ydec ; rectanglePts[4].Z = - 999 ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMF_HARD_BREAK,2,1,&tempDtmP->nullFeatureId,rectanglePts,5)) goto errexit ;
    if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(tempDtmP,L"padded.dat") ;
    /*
    ** Triangulate DTM
    */
    if( dbg )  bcdtmWrite_message(0,0,0,"Triangulating Temp Dtm Object ** tempDtmP->numPoints = %8ld",tempDtmP->numPoints) ; //
    DTM_NORMALISE_OPTION  = FALSE ;             // To Inhibit Normalisation Of Coordinates
    DTM_DUPLICATE_OPTION = FALSE ;             // To Inhibit Removal Of None Identical Points
    dtmP->ppTol = 0.0 ;
    dtmP->plTol = 0.0 ;
    if( bcdtmObject_createTinDtmObject(tempDtmP,1,0.0)) goto errexit ;
    DTM_NORMALISE_OPTION  = TRUE ;
    DTM_DUPLICATE_OPTION = TRUE ;
    if( dbg )  bcdtmWrite_message(0,0,0,"Triangulating Temp Dtm Object Completed ** tempDtmP->numPoints = %8ld",tempDtmP->numPoints) ;
    /*
    ** Check Triangulation
    */
    if( cdbg )
        {
         bcdtmWrite_message(0,0,0,"Checking Temp Triangulation") ;
         if( bcdtmCheck_tinComponentDtmObject(tempDtmP))
            {
            bcdtmWrite_message(2,0,0,"Triangulation Invalid") ;
            goto errexit ;
            }
         bcdtmWrite_message(0,0,0,"Temp Triangulation Valid") ;
        }
    /*
    ** Find Entry In Tin For Hull And Rectangle Features
    */
    hullFeature      = tempDtmP->nullPnt ;
    rectangleFeature = tempDtmP->nullPnt ;
    for( dtmFeature = 0 ; dtmFeature < tempDtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(tempDtmP,dtmFeature) ;
        if( dtmFeatureP->dtmUserTag == 1 && dtmFeatureP->dtmFeatureState == DTMF_STATE_TIN ) hullFeature = dtmFeature ;
        if( dtmFeatureP->dtmUserTag == 2 && dtmFeatureP->dtmFeatureState == DTMF_STATE_TIN ) rectangleFeature = dtmFeature ;
        }
    if( hullFeature == tempDtmP->nullPnt || rectangleFeature == tempDtmP->nullPnt )
        {
        bcdtmWrite_message(2,0,0,"Cannot Find Feature Entries") ;
        goto errexit ;
        }
    if( dbg ) bcdtmWrite_message(0,0,0,"hullFeature = %8ld ** rectangleFeature = %8ld",hullFeature,rectangleFeature) ;
    /*
    ** Add Rectangle Points To Tin
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Adding Rectangle Points To Tin") ;
    if( bcdtmInsert_addPointToDtmObject(dtmP,rectanglePts[0].X,rectanglePts[0].Y,rectanglePts[0].Z,&tinPoints[0])) goto errexit ;
    if( bcdtmInsert_addPointToDtmObject(dtmP,rectanglePts[1].X,rectanglePts[1].Y,rectanglePts[1].Z,&tinPoints[1])) goto errexit ;
    if( bcdtmInsert_addPointToDtmObject(dtmP,rectanglePts[2].X,rectanglePts[2].Y,rectanglePts[2].Z,&tinPoints[2])) goto errexit ;
    if( bcdtmInsert_addPointToDtmObject(dtmP,rectanglePts[3].X,rectanglePts[3].Y,rectanglePts[3].Z,&tinPoints[3])) goto errexit ;
    /*
    ** Add Rectangle Lines To Tin
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Adding Rectangle Lines To Tin") ;
    if( bcdtmList_insertLineDtmObject(dtmP,tinPoints[0],tinPoints[1])) goto errexit ;
    if( bcdtmList_insertLineDtmObject(dtmP,tinPoints[1],tinPoints[2])) goto errexit ;
    if( bcdtmList_insertLineDtmObject(dtmP,tinPoints[2],tinPoints[3])) goto errexit ;
    if( bcdtmList_insertLineDtmObject(dtmP,tinPoints[3],tinPoints[0])) goto errexit ;
    /*
    ** Scan Temp Tin And Set DTM Point Number In sPtr Array
    */
    for( point = 0 ; point < tempDtmP->numPoints ; ++point )
        {
        pointP = pointAddrP(tempDtmP,point) ;
        bcdtmFind_closestPointDtmObject(dtmP,pointP->X,pointP->Y,&closePoint) ;
        nodeAddrP(tempDtmP,point)->sPtr = closePoint  ;
        }
    /*
    ** Check For Duplicate Points
    */
    if( cdbg )
        {
        for( point = 0 ; point < tempDtmP->numPoints ; ++point )
            {
            for( startPnt = point + 1 ; startPnt < tempDtmP->numPoints ; ++startPnt )
                {
                if( nodeAddrP(tempDtmP,point)->sPtr == nodeAddrP(tempDtmP,startPnt)->sPtr )
                    {
                    bcdtmWrite_message(0,0,0,"Duplicate Points %8ld %8ld ** %8ld %8ld",point,startPnt,nodeAddrP(tempDtmP,point)->sPtr,nodeAddrP(tempDtmP,startPnt)->sPtr) ;
                    ret = DTM_ERROR ;
                    }
                }
            }
        if( ret == DTM_ERROR )
            {
            bcdtmWrite_message(2,0,0,"Duplicate Points In Rectangle Infill") ;
            goto errexit ;
            }
        }
    /*
    ** Fill Area External To Old Tin Hull
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Filling Area External To Hull") ;
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(tempDtmP,hullFeature,&startPnt)) goto errexit ;
    priorPnt = startPnt ;
    startPnt = nodeAddrP(tempDtmP,startPnt)->tPtr ;
    endPnt   = startPnt ;
    do
        {
        nextPnt   = nodeAddrP(tempDtmP,startPnt)->tPtr ;
        cStartPnt = nodeAddrP(tempDtmP,startPnt)->sPtr ;
        cNextPnt  = nodeAddrP(tempDtmP,nextPnt)->sPtr ;
        if( ( clkPnt = bcdtmList_nextClkDtmObject(tempDtmP,startPnt,nextPnt)) < 0 ) goto errexit ;
        while( clkPnt != priorPnt )
            {
            cClkPnt = nodeAddrP(tempDtmP,clkPnt)->sPtr ;
            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cStartPnt,cClkPnt,cNextPnt)) goto errexit ;
            cNextPnt = cClkPnt ;
            if( ( clkPnt = bcdtmList_nextClkDtmObject(tempDtmP,startPnt,clkPnt)) < 0 ) goto errexit ;
            }
        priorPnt = startPnt ;
        startPnt = nextPnt  ;
        } while (  startPnt != endPnt ) ;
        /*
        ** Null Tptr list
        */
        if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
        /*
        ** Fill Area Internal To Rectangle
        */
        if( dbg ) bcdtmWrite_message(0,0,0,"Filling Area Internal To Reactangle") ;
        if( bcdtmList_copyDtmFeatureToTptrListDtmObject(tempDtmP,rectangleFeature,&startPnt)) goto errexit ;
        priorPnt = startPnt ;
        startPnt = nodeAddrP(tempDtmP,startPnt)->tPtr ;
        endPnt   = startPnt ;
        do
            {
            nextPnt  = nodeAddrP(tempDtmP,startPnt)->tPtr ;
            cStartPnt = nodeAddrP(tempDtmP,startPnt)->sPtr ;
            cPriorPnt  = nodeAddrP(tempDtmP,priorPnt)->sPtr ;
            if( ( clkPnt = bcdtmList_nextClkDtmObject(tempDtmP,startPnt,priorPnt)) < 0 ) goto errexit ;
            while( clkPnt != nextPnt )
                {
                cClkPnt = nodeAddrP(tempDtmP,clkPnt)->sPtr ;
                if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cStartPnt,cClkPnt,cPriorPnt)) goto errexit ;
                cPriorPnt = cClkPnt ;
                if( ( clkPnt = bcdtmList_nextClkDtmObject(tempDtmP,startPnt,clkPnt)) < 0 ) goto errexit ;
                }
            priorPnt = startPnt ;
            startPnt = nextPnt  ;
            } while (  startPnt != endPnt ) ;
            /*
            ** Null Tptr list
            */
            if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
             /*
            ** Insert Void Feature Around Hull
            */
            if( bcdtmList_setConvexHullDtmObject(dtmP)) goto errexit ;
            if( bcdtmList_copyHptrListToTptrListDtmObject(dtmP,dtmP->hullPoint)) goto errexit ;
            if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMF_VOID,-9999,dtmP->dtmFeatureIndex,dtmP->hullPoint,1)) goto errexit ;
            ++dtmP->dtmFeatureIndex ;
            /*
            ** Scan For Hull Island And Void
            */
             if( cdbg )
               {
                for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature)
                  {
                   dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                   if( dtmFeatureP->dtmUserTag == -9999 ) bcdtmWrite_message(0,0,0,"dtmFeatureState = %4ld BcDTMFeatureId = %8I64d BcDTMUserTag = %8I64d ** dtmFeatureType = %4ld",dtmFeatureP->dtmFeatureState,dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureType) ;
                  }
               }
            /*
            ** Clean Dtm
            */
            if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning DTM Object") ;
            if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
            /*
            ** Scan For Hull Island And Void
            */
             if( cdbg )
               {
                for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature)
                  {
                   dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                   if( dtmFeatureP->dtmUserTag == -9999 ) bcdtmWrite_message(0,0,0,"dtmFeatureState = %4ld BcDTMFeatureId = %8I64d BcDTMUserTag = %8I64d ** dtmFeatureType = %4ld",dtmFeatureP->dtmFeatureState,dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureType) ;
                  }
               }
           /*
            ** Check Tin
            */
            if( cdbg )
                {
                 bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
                 if( bcdtmCheck_tinComponentDtmObject(dtmP))
                    {
                    bcdtmWrite_message(2,0,0,"Triangulation Invalid") ;
                    goto errexit ;
                    }
                 bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
                }
            /*
            ** Write Stats After Adding Rectangle
            */
            if( dbg )
              {
                bcdtmWrite_message(0,0,0,"After Adding External Rectangle") ;
                bcdtmWrite_message(0,0,0,"dtmP->numPoints        = %8ld",dtmP->numPoints) ;
                bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints  = %8ld",dtmP->numSortedPoints) ;
                bcdtmWrite_message(0,0,0,"dtmP->numTriangles     = %8ld",dtmP->numTriangles) ;
                bcdtmWrite_message(0,0,0,"dtmP->numLines         = %8ld",dtmP->numLines) ;
               if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"padded.tin") ;
               }
            /*
            ** Clean Up
            */
cleanup :
            DTM_NORMALISE_OPTION  = TRUE ;
            DTM_DUPLICATE_OPTION = TRUE ;
            if( hullPtsP != NULL ) { free(hullPtsP) ; hullPtsP = NULL ; }
            if( tempDtmP != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
            /*
            ** Job Completed
            */
            if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Rectangle Around Tin Completed") ;
            if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Rectangle Around Tin Error") ;
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
DTMPrivate int bcdtmInroads_getInroadsTriangleNumberDtmObject
(
 BC_DTM_OBJ       *dtmP,
 DTM_MX_TRG_INDEX *trgIndexP,
 long             trgPnt1,
 long             trgPnt2,
 long             trgPnt3,
 long             *trgNumP
 )
 /*
 ** This Function Finds The Entry In The Triangle Index For Triangle <trgPnt1,trgPnt2,trgPnt3>
 ** And Returns The Triangle Number
 **
 ** Note trgPnt1,trgPnt2,trgPnt3 Must Be In A Clockwise Direction
 **
 */
    {
    int     ret=DTM_SUCCESS ;
    long    sp,process ;
    DTM_MX_TRG_INDEX *bIndexP,*tIndexP    ;
    /*
    ** Initialise
    */
    *trgNumP = dtmP->nullPnt ;
    while( trgPnt1 > trgPnt2 || trgPnt1 > trgPnt3 ) { sp = trgPnt1 ; trgPnt1 = trgPnt2 ; trgPnt2 = trgPnt3 ; trgPnt3 = sp ; }
    /*
    ** Get First Entry For trgPnt1
    */
    bIndexP = trgIndexP + (trgIndexP+trgPnt1)->index  ;
    tIndexP = trgIndexP + dtmP->numTriangles ;
    /*
    ** Scan trgPnt1 Entries Looking For trgPnt2 && trgPnt3
    */
    process = 1 ;
    while ( bIndexP < tIndexP && process && *trgNumP == dtmP->nullPnt )
        {
        if( bIndexP->trgPnt1 != trgPnt1 ) process = 0 ;
        else
            {
            if( bIndexP->trgPnt2 == trgPnt2 && bIndexP->trgPnt3 == trgPnt3 )
                {
                *trgNumP = (long)(bIndexP-trgIndexP) ;
                }
            ++bIndexP ;
            }
        }
    /*
    ** If Triangle Not Found Error Exit
    */
    if( *trgNumP == dtmP->nullPnt ) goto errexit ;
    /*
    ** Cleanup
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
DTMPrivate int bcdtmInroads_loadFeaturesFromDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTM_FEATURE_ID hullFeatureId ,
 int (*loadFunctionP)(long dtmFeatureType,__int64 BcDTMUserTag,__int64 BcDTMFeatureId,long *pointIndicesP,long numPointIndices)
)
/*
** This Function Loads Inroads Tin Features From A DTM Object
*/
{
 int   ret=DTM_SUCCESS,dbg=0 ;
 long  *indexP,*pointIndicesP=NULL,memPointIndices=1000,memPointIndicesInc=1000 ;
 long  point,startPoint,dtmFeature,dtmFeatureType,numPoints,closeFlag ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Allocate Initial Memory For Storing The Point Indices
*/
 pointIndicesP = ( long * ) malloc( memPointIndices * sizeof(long)) ;
 if( pointIndicesP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Scan DTM Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMF_STATE_TIN )
      {
/*
**     Assign Feature Type
*/
       dtmFeatureType = dtmFeatureP->dtmFeatureType ;
/*
**     Check For Island representing origional tin hull
*/
       if( dtmFeatureP->dtmFeatureId == hullFeatureId )
         {
          dtmFeatureType = DTMF_HULL ;
          dtmFeatureP->dtmUserTag = dtmP->nullUserTag ;
         }
/*
**     Check For Void Representing InRoads Bounding Rectangle
*/
       if( dtmFeature == dtmP->numFeatures - 1 )
         {
          dtmFeatureType = DTMF_INROADS_RECTANGLE ;
          dtmFeatureP->dtmUserTag = dtmP->nullUserTag ;
         }
/*
**     Skip unsupported feature types)
 */
       if (dtmFeatureType != DTMF_INROADS_RECTANGLE && dtmFeatureType != DTMF_GROUP_SPOT && dtmFeatureType != DTMF_HARD_BREAK &&
            dtmFeatureType != DTMF_CONTOUR_LINE && dtmFeatureType != DTMF_HULL && dtmFeatureType != DTMF_VOID && dtmFeatureType != DTMF_ISLAND &&
            dtmFeatureType != DTMF_HOLE && dtmFeatureType != DTMF_POLYGON && dtmFeatureType != DTMF_ZERO_SLOPE_POLYGON)
            continue;
/*
**     Count Number Of Feature Points
*/
       if( bcdtmList_countNumberOfDtmFeaturePointsDtmObject(dtmP,dtmFeature,&numPoints,&closeFlag)) goto errexit ;
/*
**     Check Memory
*/
       if( numPoints > memPointIndices )
         {
          memPointIndices = memPointIndices + memPointIndicesInc ;
          if( memPointIndices < numPoints ) memPointIndices = numPoints  ;
          pointIndicesP = ( long * ) realloc(pointIndicesP,memPointIndices * sizeof(long)) ;
          if( pointIndicesP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
         }
/*
**     Copy Feature Points To Index Array
*/
       indexP = pointIndicesP ;
       startPoint  = point = dtmFeatureP->dtmFeaturePts.firstPoint ;
       do
         {
          *indexP = point ;
          ++indexP ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,point,&point)) goto errexit ;
         } while ( point != dtmP->nullPnt && point != startPoint ) ;
       if( closeFlag ) *indexP = *pointIndicesP ;
/*
**     Call Inroads Call Back Function
*/
       if( loadFunctionP(dtmFeatureType,(__int64) dtmFeatureP->dtmUserTag,(__int64)dtmFeatureP->dtmFeatureId,pointIndicesP,numPoints)) goto errexit ;
      }
   }
/*
** Cleanup
*/
 cleanup :
 if( pointIndicesP != NULL ) { free(pointIndicesP) ; pointIndicesP = NULL ; }
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
DTMPublic int bcdtmClip_toTptrPolygonLeavingFeaturesDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long clipOption)
{
 int    ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long   direction;//,dtmFeature ;
 double area ;
// BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Tptr Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPnt   = %8ld",startPnt) ;
    bcdtmWrite_message(0,0,0,"clipOption = %8ld",clipOption) ;
   }
/*
** Test For Valid Clip Option
*/
 if( clipOption != DTM_INTERNAL && clipOption != DTM_EXTERNAL )
   {
    bcdtmWrite_message(2,0,0,"Invalid Clip Option") ;
    goto errexit ;
   }
/*
** Test For Valid Dtm  Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTM_STATE_TIN )
   {
    bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
    goto errexit ;
   }
/*
** Check For Valid Tptr Polygon Start Point
*/
 if( startPnt < 0 || startPnt >= dtmP->numPoints || nodeAddrP(dtmP,startPnt)->tPtr < 0 || nodeAddrP(dtmP,startPnt)->tPtr >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Invalid Start Point For Clip Tptr Polygon") ;
    goto errexit ;
   }
/*
** Check Connectivity Of Tptr Polygon - Development Only
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Tptr Polygon") ;
    if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) goto errexit ;
   }
/*
** Check Direction Of Tptr Polygon And If Clockwise Set Direction Anti Clockwise
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Direction Tptr Polygon") ;
 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Polygon Area = %15.4lf Direction = %2ld",area,direction) ;
 if( direction == DTM_CLOCKWISE )
   {
    if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
   }
/*
** Check Integrity Of Dtm Object - Development Only
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) { bcdtmWrite_message(0,0,0,"DTM Corrupted") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"DTM OK") ;
   }
/*
** Clip Internal To Tptr Polygon
*/
 if( clipOption == DTM_INTERNAL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object Internal") ;
    if( bcdtmClip_internalToTptrPolygonDtmObject(dtmP,startPnt,1)) goto errexit ;
   }
/*
** Clip External To Tptr Polygon
*/
 if( clipOption == DTM_EXTERNAL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object External") ;
    if( bcdtmClip_externalToTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
   }
/*
** Clean Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Dtm Object ** dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
 if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Dtm Object Completed ** dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
/*
** Check Integrity Of Dtm Object - Development Only
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Clipped Dtm") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"DTM Corrupted After Clip") ;
       goto errexit ;
      }
    else bcdtmWrite_message(0,0,0,"DTM OK") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Tptr Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Tptr Polygon Error") ;
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
DTMPrivate int bcdtmInroads_clipUsingIslandFeatureIdDtmObject
(
 BC_DTM_OBJ       *dtmP,
 DTM_FEATURE_ID   BcDTMFeatureId  // Feature Id Of An Island Feature Used To Store The Old Tin Hull
 )
 /*
 ** This Is A Special Purpose Clean Up Function That Is Called After Exporting Inroads Triangles
 ** It Removes The External Void Triangles That Are Created To Simulate An Inroads Triangulation
 **
 */
    {
    int     ret=DTM_SUCCESS,dbg=0 ;
    long    dtmFeature,hullFeature,startPnt ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping DTM To Island Feature Id = %10I64d",BcDTMFeatureId) ;
    /*
    ** Test For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check DTM Is In Triangulated State
    */
    if( dtmP->dtmState != DTM_STATE_TIN )
        {
        bcdtmWrite_message(2,0,0,"Method Requires Triangulated Dtm") ;
        goto errexit ;
        }
    /*
    ** Write Stats Prior To Clipping
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Before Clipping To Island Feature") ;
        bcdtmWrite_message(0,0,0,"dtmP->numPoints        = %8ld",dtmP->numPoints) ;
        bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints  = %8ld",dtmP->numSortedPoints) ;
        bcdtmWrite_message(0,0,0,"dtmP->numTriangles     = %8ld",dtmP->numTriangles) ;
        bcdtmWrite_message(0,0,0,"dtmP->numLines         = %8ld",dtmP->numLines) ;
        bcdtmWrite_message(0,0,0,"dtmP->numFeatures      = %8ld",dtmP->numFeatures) ;
        }
    /*
    ** Scan For Hull Feature
    */
    hullFeature = dtmP->nullPnt ;
    for( dtmFeature = dtmP->numFeatures - 1 ; dtmFeature >= 0 && hullFeature == dtmP->nullPnt ; --dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureId == BcDTMFeatureId && dtmFeatureP->dtmFeatureState == DTMF_STATE_TIN )
            {
            hullFeature = dtmFeature ;
            }
        }
    /*
    ** Check Hull Feature Found
    */
    if( hullFeature == dtmP->nullPnt )
        {
        bcdtmWrite_message(2,0,0,"No Island Feature With Required Feature Id Found") ;
        goto errexit ;
        }
    /*
    ** Copy Feature Points To Tptr List
    */
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,hullFeature,&startPnt)) goto errexit ;
    /*
    ** Remove Island Feature
    */
    if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,hullFeature)) goto errexit ;
    /*
    ** Clip DTM To Tptr Polygon
    */
    if( bcdtmClip_toTptrPolygonLeavingFeaturesDtmObject(dtmP,startPnt,2)) goto errexit ;
    /*
    ** Write Stats After Clipping
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"After Clipping To Island Feature") ;
        bcdtmWrite_message(0,0,0,"dtmP->numPoints        = %8ld",dtmP->numPoints) ;
        bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints  = %8ld",dtmP->numSortedPoints) ;
        bcdtmWrite_message(0,0,0,"dtmP->numTriangles     = %8ld",dtmP->numTriangles) ;
        bcdtmWrite_message(0,0,0,"dtmP->numLines         = %8ld",dtmP->numLines) ;
        bcdtmWrite_message(0,0,0,"dtmP->numFeatures      = %8ld",dtmP->numFeatures) ;
        }
    /*
    ** Cleanup
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping DTM To Island Feature Id Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping DTM To Island Feature Id Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
#ifdef HH
 /*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMPrivate int bcdtmInRoads_insertLineBetweenVerticesDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long endPnt)
/*
** This Is A Special Purpose Function For Importing Inroads Tins Into bcLIB Tins
** And Should Not Be Used For Any Other Purpose
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   clPtr,insertPnt,listPnt,connectPnt,useNewInsertMethod=TRUE ;
 double minAngle=0.0,diffAngle ,insertAngle,listAngle;
 DTM_CIR_LIST *clistP ;
/*
** Write Entry Message
*/
 if( dbg ) if( bcdtmWrite_message(0,0,0,"Inserting Line Between Points")) goto errexit ;
/*
** Check If Points Are Connected
*/
 if( bcdtmList_testLineDtmObject(dtmP,startPnt,endPnt))
   {
    nodeAddrP(dtmP,startPnt)->tPtr = endPnt ;
   }
 else
   {
/*
**  Write Un Connected Points To log File
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Warning ** Unconnected Feature Points") ;
       bcdtmWrite_message(0,0,0,"P1 = %8ld ** %12.5lf %12.5lf %10.4lf",startPnt,pointAddrP(dtmP,startPnt)->X,pointAddrP(dtmP,startPnt)->Y,pointAddrP(dtmP,startPnt)->Z) ;
       bcdtmWrite_message(0,0,0,"P2 = %8ld ** %12.5lf %12.5lf %10.4lf",endPnt,pointAddrP(dtmP,endPnt)->X,pointAddrP(dtmP,endPnt)->Y,pointAddrP(dtmP,endPnt)->Z) ;
      }

//  RobC 6May2013 - To Account For Precision Problems with Below Angle Insert Method

    if( useNewInsertMethod )
      {
       if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,startPnt,endPnt,1,2)) goto errexit ;
      }

    else
      {
/*
**  Make Conection Following Break Line Angle
*/
    connectPnt = dtmP->nullPnt ;
    insertPnt  = startPnt ;
    while ( insertPnt != endPnt )
      {
       insertAngle = bcdtmMath_getPointAngleDtmObject(dtmP,insertPnt,endPnt) ;
       clPtr = nodeAddrP(dtmP,insertPnt)->cPtr ;
       while ( clPtr != dtmP->nullPtr )
         {
          clistP = clistAddrP(dtmP,clPtr) ;
          listPnt = clistP->pntNum  ;
          clPtr   = clistP->nextPtr ;
          listAngle = bcdtmMath_getPointAngleDtmObject(dtmP,insertPnt,listPnt) ;
/*
**        Calculate Difference Angle
*/
          if( dbg == 2 ) bcdtmWrite_message(0,0,0," listPnt = %8ld ** insertAngle = %12.10lf listAngle = %12.10lf",listPnt,insertAngle,listAngle) ;
          if( listAngle < insertAngle ) diffAngle = insertAngle - listAngle ;
          else                          diffAngle = listAngle - insertAngle ;
          if( diffAngle > DTM_PYE )     diffAngle = DTM_2PYE  - diffAngle ;
/*
**        Check For Minimum Difference Angle
*/
          if( connectPnt == dtmP->nullPnt || diffAngle < minAngle )
            {
             connectPnt = listPnt ;
             minAngle   = diffAngle  ;
            }
         }
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"minAngle = %15.12lf ** connectPnt = %8ld ** %12.5lf %12.5lf %10.4lf",minAngle,connectPnt,pointAddrP(dtmP,connectPnt)->X,pointAddrP(dtmP,connectPnt)->Y,pointAddrP(dtmP,connectPnt)->Z) ;
/*
**     Connect To Connect Point
*/
       if (nodeAddrP(dtmP,insertPnt)->tPtr != dtmP->nullPnt)
           {
           if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Already set, lines must cross?") ;
           goto errexit;
           }
       nodeAddrP(dtmP,insertPnt)->tPtr = connectPnt ;
       insertPnt = connectPnt ;
       connectPnt = dtmP->nullPnt ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Connected Via Following Break Line Angle") ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Line Between Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Line Between Points Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMPrivate int bcdtmInRoads_geopakPointsCallBackFunction( BC_DTM_OBJ *dtmP , double X,double Y,double Z )
{
 int ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 DPoint3d *pointP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"geopakPointsCallBackFunction ** dtmP = %p ** X = %12.5lf Y = %12.5lf Y = %12.5lf",dtmP,X,Y,Z) ;
/*
** Check For Valid Dtm Object
*/
 if( cdbg ) if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check Memory
*/
 if( dtmP->numPoints == dtmP->memPoints )  if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit ;
/*
** Store Point
*/
 pointP = pointAddrP(dtmP,dtmP->numPoints) ;
 pointP->X = X ;
 pointP->Y = Y ;
 pointP->Z = Z ;
 ++dtmP->numPoints ;
/*
** Clean Up
*/
 cleanup :
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"geopakPointsCallBackFunction Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"geopakPointsCallBackFunction Error") ;
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
DTMPrivate int bcdtmInRoads_geopakCircularListCallBackFunction(BC_DTM_OBJ *dtmP,long pointIndex,long *cirPointIndexP,long numCirPointIndex )
{
 int ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long numCirList ;
// DTM_TIN_NODE  *nodeP ;
// DTM_CIR_LIST  *clistP ;
 static long lastPointIndex=-1 ;
 BC_DTM_OBJ *temP=NULL ;
 long p1 ;
 DPoint3d *pointP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"geopakCircularListCallBackFunction ** pointIndex = %8ld",pointIndex) ;

 if( lastPointIndex == -1 && 0 )
   {
    if( bcdtmObject_createDtmObject(&temP)) goto errexit ;
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       pointP = pointAddrP(dtmP,p1) ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(temP,DTMF_RANDOM_SPOT,nullUserTag,1,&nullFeatureId,(P3D *)pointP,1)) goto errexit ;
      }
    bcdtmObject_triangulateDtmObject(temP) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld temP->numPoints = %8ld",dtmP->numPoints,temP->numPoints) ;
    bcdtmObject_destroyDtmObject(&temP) ;
   }
/*
** Check For Valid Dtm Object
*/
 if( cdbg ) if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check Point Index Is One Grater Than Last Point Index
*/
 if( cdbg )
   {
    if( pointIndex - lastPointIndex != 1 )
      {
       bcdtmWrite_message(0,0,0,"Warning ** Missing Point Index between lastPointIndex = %8ld  and pointIndex = %8ld",lastPointIndex,pointIndex) ;
//       goto errexit ;
      }
   }
 lastPointIndex = pointIndex ;
/*
** Check Memory
*/
 if( dtmP->nodesPP == NULL ) if( bcdtmObject_allocateNodesMemoryDtmObject(dtmP)) goto errexit ;
 if( dtmP->cListPP == NULL ) if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit ;
/*
** Store Circular List
*/
/*
 nodeP = nodeAddrP(dtmP,pointIndex) ;
 nodeP->cPtr = dtmP->cListPtr ;
 for( numCirList = 0 ; numCirList < numCirPointIndex ; ++numCirList )
   {
    clistP = clistAddrP(dtmP,dtmP->cListPtr) ;
    clistP->pntNum  = *(cirPointIndexP+numCirList) ;
    clistP->nextPtr = dtmP->nullPtr ;
    if( numCirList > 0 )
      {
       clistP = clistAddrP(dtmP,dtmP->cListPtr-1) ;
       clistP->nextPtr = dtmP->cListPtr ;
      }
    ++dtmP->cListPtr ;
    if( dtmP->cListPtr >= dtmP->numPoints * 6 )
      {
       bcdtmWrite_message(1,0,0,"Circular List Memory Exceeded") ;
       goto errexit ;
      }
   }
*/
 for( numCirList = 0 ; numCirList < numCirPointIndex ; ++numCirList )
   {
    if( ! bcdtmList_testLineDtmObject(dtmP,pointIndex,*(cirPointIndexP+numCirList)))
      {
       if( bcdtmList_insertLineDtmObject(dtmP,pointIndex,*(cirPointIndexP+numCirList))) goto errexit ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"geopakCircularListCallBackFunction Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"geopakCircularListCallBackFunction Error") ;
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
DTMPrivate int bcdtmInroads_dtmFeatureCallBackFunction(BC_DTM_OBJ *dtmP,long dtmFeatureType,long *dtmFeaturePointIndiciesP, long numDtmFeaturePointIndicies)
{
 int ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long index,firstPoint,lastPoint,nextPoint,listPnt,nextListPnt,direction ;
 double area ;
 char dtmFeatureTypeName[30] ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"bcdtmInroads_dtmFeatureCallBackFunction ** dtmFeatureType = %4ld dtmFeatureTypeName = %s ** numIndicies = %8ld",dtmFeatureType,dtmFeatureTypeName,numDtmFeaturePointIndicies) ;
   }
/*
** Check For Valid Dtm Feature Type
*/
 if( bcdtmData_testForValidDtmObjectImportFeatureType(dtmFeatureType))
   {
    bcdtmWrite_message(1,0,0,"Invalid Dtm Feature Type") ;
    goto errexit ;
   }
/*
**  Check Number Of Feature Indicies
*/
 if( ( dtmFeatureType == DTMF_GROUP_SPOT && numDtmFeaturePointIndicies < 1 ) || ( dtmFeatureType != DTMF_GROUP_SPOT && numDtmFeaturePointIndicies < 2 ) )
   {
    bcdtmWrite_message(1,0,0,"Not Enough Point Indicies For Feature") ;
    goto errexit ;
   }
/*
** Check For Valid Dtm Object
*/
 if( cdbg ) if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check Memory
*/
 if( dtmP->fTablePP == NULL  ) if( bcdtmObject_allocateFeaturesMemoryDtmObject(dtmP)) goto errexit ;
/*
** Check Point Indicies Are In Correct Range
*/
 if( cdbg )
   {
    for( index = 0 ; index < numDtmFeaturePointIndicies ; ++index )
      {
       if( *(dtmFeaturePointIndiciesP+index) < 0 || *(dtmFeaturePointIndiciesP+index) >= dtmP->numPoints )
         {
          bcdtmWrite_message(1,0,0,"Point Indice Range Error = %8ld",*(dtmFeaturePointIndiciesP+index)) ;
          goto errexit ;
         }
      }
   }
/*
** Store Feature Points In Tptr List
*/
 firstPoint = lastPoint = *dtmFeaturePointIndiciesP ;
 for( index = 1 ; index < numDtmFeaturePointIndicies ; ++index )
   {
    nextPoint = *(dtmFeaturePointIndiciesP+index) ;
    if( dtmFeatureType != DTMF_GROUP_SPOT ) { if( bcdtmInRoads_insertLineBetweenVerticesDtmObject(dtmP,lastPoint,nextPoint)) goto errexit ; }
    else nodeAddrP(dtmP,lastPoint)->tPtr = nextPoint ;
    lastPoint  = nextPoint ;
   }
/*
** Scan Tptr List And Check For Unconnected Points
*/
 if( cdbg && dtmFeatureType != DTMF_GROUP_SPOT )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Unconnected Feature Points") ;
    listPnt = firstPoint ;
    do
      {
       nextListPnt = nodeAddrP(dtmP,listPnt)->tPtr ;
       if( nextListPnt != dtmP->nullPnt )
         {
          if( ! bcdtmList_testLineDtmObject(dtmP,listPnt,nextListPnt))
            {
             bcdtmWrite_message(0,0,0,"Unconnected Feature Points") ;
             bcdtmWrite_message(0,0,0,"P1 = %8ld ** %12.5lf %12.5lf %10.4lf",listPnt,pointAddrP(dtmP,listPnt)->X,pointAddrP(dtmP,listPnt)->Y,pointAddrP(dtmP,listPnt)->Z) ;
             bcdtmWrite_message(0,0,0,"P2 = %8ld ** %12.5lf %12.5lf %10.4lf",nextListPnt,pointAddrP(dtmP,nextListPnt)->X,pointAddrP(dtmP,nextListPnt)->Y,pointAddrP(dtmP,nextListPnt)->Z) ;
            }
         }
       listPnt = nextListPnt ;
      } while ( listPnt != firstPoint && listPnt != dtmP->nullPnt ) ;
   }
/*
** Check Connectivity Tptr List
*/
 switch ( dtmFeatureType )
   {
    case DTMF_HULL :
    case DTMF_VOID :
      if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,firstPoint,dbg))
        {
         bcdtmWrite_message(1,0,0,"Feature Connectivity Error") ;
         goto errexit ;
        }

    default :
       if( dtmFeatureType != DTMF_GROUP_SPOT )
         {
          if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,firstPoint,dbg))
            {
             bcdtmWrite_message(1,0,0,"Feature Connectivity Error") ;
             goto errexit ;
            }
        }
    break ;
   }
/*
** Set Polygonal Dtm Features Anti Clockwise
*/
 if( dtmFeatureType == DTMF_HULL || dtmFeatureType == DTMF_VOID )
   {
    if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,firstPoint,&area,&direction) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Polygon Area = %15.4lf Direction = %2ld",area,direction) ;
    if( direction == DTM_CLOCKWISE ) if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,firstPoint)) goto errexit ;
   }
/*
** Add Dtm Feature To Tin
*/
 if( dtmFeatureType == DTMF_HULL ) dtmFeatureType = DTMF_POLYGON ;
 if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureType,nullUserTag,dtmP->dtmFeatureIndex,firstPoint,1)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigned Feature Id = %10ld",dtmP->dtmFeatureIndex) ;
 ++dtmP->dtmFeatureIndex ;
/*
** Clean Up
*/
 cleanup :
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"bcdtmInroads_dtmFeatureCallBackFunction Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"bcdtmInroads_dtmFeatureCallBackFunction Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMPrivate int bcdtmInroads_dtmFeaturePointsCallBackFunction
(
 BC_DTM_OBJ *dtmP,
 wchar_t*   dtmFeatureName,
 wchar_t*   dtmFeatureDescription,
 wchar_t*   dtmFeatureStyle,
 long       dtmFeatureType,
 P3D        *dtmFeaturePointsP,
 long       numDtmFeaturePoints,
 long       excludeFromTriangulation
)
{
 int ret=DTM_SUCCESS,dbg=0,cdbg=0;
 long firstPoint,lastPoint,nextPoint,listPnt,nextListPnt,direction,process ;
 long p1,p2,clc,numTinLines,nullTptrValues=FALSE,numCleanPts ;
 P3D  *p3dP,tinLinePts[2],*cleanPtsP=NULL ;
 double area ;
 static long dtmFeature=0  ;
 char dtmFeatureTypeName[30] ;
 DTM_FEATURE_ID  BcDTMFeatureId ;
 BC_DTM_OBJ  *temP=NULL ;
 DPoint3d *pointP ;
 DTM_TIN_NODE  *nodeP ;
 static long dtmFeatureCount=0 ;
 DTM_STR_INT_PTS *notP,*knotsP=NULL ;
 long             numKnots ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"Points Call Back ** bcdtmInroads_dtmFeatureCallBackFunction ** dtmFeatureType = %4ld dtmFeatureTypeName = %s exclude = %2ld ** numDtmFeaturePts = %8ld",dtmFeatureType,dtmFeatureTypeName,excludeFromTriangulation,numDtmFeaturePoints) ;
    if( dbg == 2 )
      {
       for( p3dP = dtmFeaturePointsP ; p3dP  < dtmFeaturePointsP + numDtmFeaturePoints ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-dtmFeaturePointsP)+1,p3dP->X,p3dP->Y,p3dP->Z) ;
         }
      }
   }
/*
** Check For Valid Dtm Feature Type
*/
 if( bcdtmData_testForValidDtmObjectImportFeatureType(dtmFeatureType))
   {
    bcdtmWrite_message(1,0,0,"Invalid Dtm Feature Type") ;
    goto errexit ;
   }
/*
**  Check Number Of Feature Points
*/
 if( ( dtmFeatureType == DTMF_GROUP_SPOT && numDtmFeaturePoints < 1 ) || ( dtmFeatureType != DTMF_GROUP_SPOT && numDtmFeaturePoints < 2 ) )
   {
    bcdtmWrite_message(1,0,0,"Not Enough Points For Feature") ;
//    goto errexit ;
    goto cleanup ;
   }
/*
** Check For Valid Dtm Object
*/
 if( cdbg ) if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Log DTM Type
*/
 if( dbg == 2 )
   {
    if( dtmP->nodesPP == NULL ) bcdtmWrite_message(0,0,0,"DTM In Data State") ;
    else                        bcdtmWrite_message(0,0,0,"DTM In Tin State") ;
   }

 /*
 ** Callback straight away if exclude from triangulation
 */
 if (excludeFromTriangulation!=0)
     {
     if (pFeatureCallbackFunc)
         pFeatureCallbackFunc( DTM_NULL_FEATURE_ID , dtmFeatureStyle, dtmFeatureName, dtmFeatureDescription, (int) dtmFeatureType, dtmFeaturePointsP, (int) numDtmFeaturePoints);

     goto cleanup;
     }

/*
** Check For Untriangulated DTM
*/
 if( dtmP->nodesPP == NULL )
     {
     /*
     **  Store DTM Feature
     */
     int ret = bcdtmObject_storeDtmFeatureInDtmObject(dtmP,dtmFeatureType,dtmP->nullUserTag,3,&BcDTMFeatureId,dtmFeaturePointsP,numDtmFeaturePoints);
     if (ret==0)
         {
         if (pFeatureCallbackFunc)
             pFeatureCallbackFunc(BcDTMFeatureId, dtmFeatureStyle, dtmFeatureName, dtmFeatureDescription, (int) dtmFeatureType, dtmFeaturePointsP, (int) numDtmFeaturePoints);
         }
     else
         {
         goto cleanup;
         }
/*
**  Return
*/
     goto cleanup ;
     }
/*
** Ignore Random Points For Triangulated DTM
*/
 if( dtmFeatureType == DTMF_RANDOM_SPOT ) goto cleanup ;
/*
** Check DTM Is In Tin State
*/
 if( dtmP->dtmState != DTM_STATE_TIN )
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld",dtmP->numPoints) ;
    dtmFeature = 0 ;
    dtmP->dtmState = DTM_STATE_TIN ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Setting Convex Hull") ;
    if( bcdtmList_setConvexHullDtmObject(dtmP)) goto errexit ;
/*
**  Check Tin Topology
*/
    if( bcdtmCheck_topologyDtmObject(dtmP,0))
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Tin Topology Invalid")  ;
 //      if( bcdtmInroads_fixTopologyDtmObject(dtmP)) goto errexit  ;
/*
**     Write Tin Lines To Dtm Object As Graphic Breaks
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Exporting Tin Lines") ;
       if( bcdtmObject_createDtmObject(&temP)) goto errexit ;
       for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
         {
          pointP = pointAddrP(dtmP,p1) ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(temP,DTMF_RANDOM_SPOT,nullUserTag,1,&nullFeatureId,(P3D *)pointP,1)) goto errexit ;
         }
       numTinLines = 0 ;
       for( p1 = 0 ; p1 < dtmP->numPoints; ++p1 )
         {
          clc = nodeAddrP(dtmP,p1)->cPtr ;
          while ( clc != dtmP->nullPtr )
            {
             p2  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if( p2 > p1 )
               {
                pointP = pointAddrP(dtmP,p1) ;
                tinLinePts[0].X = pointP->X ; tinLinePts[0].Y = pointP->Y ; tinLinePts[0].Z = pointP->Z ;
                pointP = pointAddrP(dtmP,p2) ;
                tinLinePts[1].X = pointP->X ; tinLinePts[1].Y = pointP->Y ; tinLinePts[1].Z = pointP->Z ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(temP,DTMF_GRAPHIC_BREAK,nullUserTag,1,&nullFeatureId,tinLinePts,2)) goto errexit ;
                ++numTinLines ;
               }
            }
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"temP->numPoints = %8ld ** numTinLines = %8ld",temP->numPoints,numTinLines) ;
/*
**     Triangulate
*/
       temP->edgeOption = 1 ;
       if( bcdtmObject_triangulateDtmObject(temP)) goto errexit ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"temP->numPoints = %8ld",temP->numPoints) ;
/*
**     Copy Temp To DTM
*/
       bcdtmObject_initialiseDtmObject(dtmP) ;
       *dtmP = *temP ;
       temP->fTablePP = NULL ;
       temP->pointsPP = NULL ;
       temP->nodesPP  = NULL ;
       temP->cListPP  = NULL ;
       temP->fListPP  = NULL ;
       bcdtmObject_destroyDtmObject(&temP) ;
/*
**     Set the triangles to Rollback
*/
       bcdtmObject_setTriangleRollBackDtmObject (dtmP);
/*
**     Recheck Topology
*/
       if( bcdtmCheck_topologyDtmObject(dtmP,0))
         {
          bcdtmWrite_message(1,0,0,"Tin Topology Invalid") ;
          goto errexit ;
         }
      }
/*
**  Check Tin Precision
*/
    if( cdbg == 1 )
      {
       if( bcdtmCheck_precisionDtmObject(dtmP,0) )
         {
          bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ;
          goto errexit ;
         }
       else if( dbg )  bcdtmWrite_message(0,0,0,"Tin Precision Valid") ;
      }
/*
**  Clean Tin
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning DTM Object") ;
    if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
/*
**  Check Triangulation After Cleaning
*/
    if( cdbg == 1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Triangulated Dtm After Cleaning Features") ;
       if( bcdtmCheck_tinComponentDtmObject(dtmP))
         {
          bcdtmWrite_message(1,0,0,"Triangulated Dtm Invalid") ;
          goto errexit ;
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"Triangulated Dtm Valid") ;
      }
   }
/*
** Check For Knots In Imported Interior Or Exterior Boundary
*/
 if( dtmFeatureType == DTMF_HULL || dtmFeatureType == DTMF_VOID )
   {
    bcdtmClean_checkP3DStringForKnots(dtmFeaturePointsP,numDtmFeaturePoints,&knotsP,&numKnots) ;
    if( numKnots > 0 )
      {
       if( dbg )
         {
          if( dtmFeatureType == DTMF_HULL ) bcdtmWrite_message(0,0,0,"Knots Detected In Exterior Boundary") ;
          if( dtmFeatureType == DTMF_VOID ) bcdtmWrite_message(0,0,0,"Knots Detected In Interior Boundary") ;
         }
       if( dbg == 2 )
         {
          bcdtmWrite_message(0,0,0,"Number Of Knots = %8ld",numKnots) ;
          for( notP = knotsP ; notP < knotsP + numKnots ; ++notP )
            {
             bcdtmWrite_message(0,0,0,"Knot[%4ld] ** seg1 = %4ld seg2 = %4ld ** %12.5lf %12.5lf %10.4lf",(long)(notP-knotsP),notP->Segment1,notP->Segment2,notP->X,notP->Y,notP->Z) ;
            }
          for( p3dP = dtmFeaturePointsP ; p3dP < dtmFeaturePointsP + numDtmFeaturePoints ; ++p3dP )
            {
             bcdtmWrite_message(0,0,0,"Feature Point[%8ld] ** %12.5lf %12.5lf %10.4lf",(long)(p3dP-dtmFeaturePointsP),p3dP->X,p3dP->Y,p3dP->Z) ;
            }
         }

       //  RobC Added 2/May/2013 ** Externally Clean The Polygon

       numCleanPts = numDtmFeaturePoints;
       cleanPtsP = ( P3D * ) malloc(numCleanPts*sizeof(P3D)) ;
       if( cleanPtsP == NULL )
          {
           bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
           goto errexit ;
          }
       memcpy(cleanPtsP,dtmFeaturePointsP,numDtmFeaturePoints*sizeof(P3D)) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Externally Cleaning Polygon Feature") ;
       if( bcdtmClean_externalPointArrayPolygon(&cleanPtsP,&numCleanPts,dtmP->ppTol)) goto errexit ;
       bcdtmClean_checkP3DStringForKnots(cleanPtsP,numCleanPts,&knotsP,&numKnots) ;
       if( numKnots > 0 )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Unable To Clean Polygon") ;
          goto cleanup ;
         }
       else if( dbg ) bcdtmWrite_message(0,0,0,"Polygon Cleaned") ;
       numDtmFeaturePoints = numCleanPts ;
       dtmFeaturePointsP = cleanPtsP;
      }
    if( knotsP != NULL ) { free(knotsP) ; knotsP = NULL ; }
    if( bcdtmMath_getPolygonDirectionP3D(dtmFeaturePointsP,numDtmFeaturePoints,&direction,&area) ) goto errexit ;
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"dtmFeatureType[%4ld] ** Area = %15.4lf Direction = %2ld",dtmFeatureType,area,direction) ;
    if( direction == 1 ) bcdtmMath_reversePolygonDirectionP3D(dtmFeaturePointsP,numDtmFeaturePoints) ;
   }
/*
** Increment Feature
*/
 ++dtmFeature ;
/*
** Check Memory
*/
 if( dtmP->fTablePP == NULL  ) if( bcdtmObject_allocateFeaturesMemoryDtmObject(dtmP)) goto errexit ;
/*
** Store Feature Points In Tptr List
*/
 p3dP = dtmFeaturePointsP ;
 bcdtmFind_closestPointDtmObject(dtmP,p3dP->X,p3dP->Y,&firstPoint) ;
 lastPoint = firstPoint ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"firstPoint = %8ld",firstPoint) ;
    bcdtmWrite_message(0,0,0,"First Point Data = %12.5lf %12.5lf %10.4lf",p3dP->X,p3dP->Y,p3dP->Z) ;
    bcdtmWrite_message(0,0,0,"First Point Tin  = %12.5lf %12.5lf %10.4lf",pointAddrP(dtmP,firstPoint)->X,pointAddrP(dtmP,firstPoint)->Y,pointAddrP(dtmP,firstPoint)->Z) ;
   }
 process = 1 ;
 nullTptrValues = FALSE ;
 for( p3dP = dtmFeaturePointsP + 1 ; p3dP  < dtmFeaturePointsP + numDtmFeaturePoints && process ; ++p3dP )
   {
    bcdtmFind_closestPointDtmObject(dtmP,p3dP->X,p3dP->Y,&nextPoint) ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"lastPoint = %8ld nextPoint = %8ld nextPoint->tPtr = %10ld",lastPoint,nextPoint,nodeAddrP(dtmP,nextPoint)->tPtr)  ;
    if( dtmFeatureType != DTMF_GROUP_SPOT )
      {
       if( bcdtmInRoads_insertLineBetweenVerticesDtmObject(dtmP,lastPoint,nextPoint)) goto errexit ;
      }
    else
      {
       nodeAddrP(dtmP,lastPoint)->tPtr = nextPoint ;
      }
    lastPoint  = nextPoint ;
    if( nodeAddrP(dtmP,lastPoint)->tPtr != dtmP->nullPnt )
      {
       process = 0 ;
       firstPoint = lastPoint ;
       nullTptrValues = TRUE ;
      }
   }
/*
** Log Tptr List For DTM Feature
*/
 if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,firstPoint) ;
/*
** Scan Tptr List And Check For Unconnected Points
*/
 if( cdbg && dtmFeatureType != DTMF_GROUP_SPOT )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Unconnected Feature Points") ;
    listPnt = firstPoint ;
    do
      {
       nextListPnt = nodeAddrP(dtmP,listPnt)->tPtr ;
       if( nextListPnt != dtmP->nullPnt )
         {
          if( ! bcdtmList_testLineDtmObject(dtmP,listPnt,nextListPnt))
            {
             bcdtmWrite_message(0,0,0,"Unconnected Feature Points") ;
             bcdtmWrite_message(0,0,0,"P1 = %8ld ** %12.5lf %12.5lf %10.4lf",listPnt,pointAddrP(dtmP,listPnt)->X,pointAddrP(dtmP,listPnt)->Y,pointAddrP(dtmP,listPnt)->Z) ;
             bcdtmWrite_message(0,0,0,"P2 = %8ld ** %12.5lf %12.5lf %10.4lf",nextListPnt,pointAddrP(dtmP,nextListPnt)->X,pointAddrP(dtmP,nextListPnt)->Y,pointAddrP(dtmP,nextListPnt)->Z) ;
            }
         }
       listPnt = nextListPnt ;
      } while ( listPnt != firstPoint && listPnt != dtmP->nullPnt ) ;
   }
/*
** Check Connectivity Tptr List
*/
 switch ( dtmFeatureType )
   {
    case DTMF_HULL :
    case DTMF_VOID :
      if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,firstPoint,dbg))
        {
         bcdtmWrite_message(1,0,0,"Feature Connectivity Error") ;
         if( dbg )
           {
            bcdtmList_writeTptrListDtmObject(dtmP,firstPoint) ;
            bcdtmWrite_message(0,0,0,"Number Of Feature Points = %8ld",numDtmFeaturePoints) ;
            for( p3dP = dtmFeaturePointsP ; p3dP < dtmFeaturePointsP + numDtmFeaturePoints ; ++p3dP )
              {
               bcdtmWrite_message(0,0,0,"FeaturePoint[%5ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-dtmFeaturePointsP),p3dP->X,p3dP->Y,p3dP->Z)  ;
              }
           }
         goto errexit ;
        }

    default :
       if( dtmFeatureType != DTMF_GROUP_SPOT )
         {
          if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,firstPoint,1))
            {
             bcdtmWrite_message(1,0,0,"Feature Connectivity Error") ;
             if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,firstPoint) ;
              bcdtmList_nullTptrValuesDtmObject(dtmP) ;
             goto errexit ;
            }
        }
    break ;
   }
/*
** Check Polygonal Dtm Features Anti Clockwise
*/
 if( dtmFeatureType == DTMF_HULL || dtmFeatureType == DTMF_VOID )
   {
    if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,firstPoint,&area,&direction) ) goto errexit ;
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Tptr Polygon Area = %15.4lf Direction = %2ld",area,direction) ;
    if( direction == DTM_CLOCKWISE )
      {
       bcdtmList_nullTptrValuesDtmObject(dtmP) ;
       goto cleanup ;
      }
   }
/*
** Add Dtm Feature To Tin
*/
 if( dtmFeatureType == DTMF_HULL ) dtmFeatureType = DTMF_POLYGON ;
 ret = bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureType,nullUserTag,dtmP->dtmFeatureIndex,firstPoint,1);
 if( dtmFeatureType == DTMF_POLYGON ) dtmFeatureType = DTMF_HULL ;
 if (ret==DTM_SUCCESS)
     {
     if (pFeatureCallbackFunc)
        pFeatureCallbackFunc(dtmP->dtmFeatureIndex, dtmFeatureStyle, dtmFeatureName, dtmFeatureDescription, (int) dtmFeatureType, dtmFeaturePointsP, numDtmFeaturePoints);
     }
 else
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"Error Inserting Feature %s",dtmFeatureTypeName) ;
    bcdtmList_nullTptrValuesDtmObject(dtmP) ;
    goto errexit ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigned Feature Id = %10ld",dtmP->dtmFeatureIndex) ;
 ++dtmP->dtmFeatureIndex ;

//  Check For None Null Tptr Values

 if( cdbg == 1 )
   {
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       nodeP = nodeAddrP(dtmP,p1) ;
       if( nodeP->tPtr != dtmP->nullPnt || nodeP->sPtr != dtmP->nullPnt )
         {
          bcdtmWrite_message(0,0,0,"None Null Tptr or Sptr Value After Inserting Feature %8ld",dtmFeatureCount) ;
          goto errexit ;
         }
      }
    ++dtmFeatureCount ;
   }

/*
** Null Tptr Values
*/
 if( nullTptrValues == TRUE )
   {
    bcdtmList_nullTptrValuesDtmObject(dtmP) ;
   }
/*
**  Check Triangulation After Inserting Feature
*/
 if( cdbg == 2 )
   {
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
       bcdtmWrite_message(1,0,0,"Triangulated Dtm Invalid After Inserting Feature %8ld ** Type = ",dtmFeature,dtmFeatureTypeName) ;
       goto errexit ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( knotsP    != NULL ) free(knotsP) ;
 if( cleanPtsP != NULL ) free(cleanPtsP) ;
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"bcdtmInroads_dtmFeatureCallBackFunction Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"bcdtmInroads_dtmFeatureCallBackFunction Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMPrivate int bcdtmInroads_trimToInroadsExteriorBoundaryDtmObject(BC_DTM_OBJ *dtmP)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 long process,dtmFeature,firstPoint,direction ;
 double area ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Trimming To Inroads Exterior Boundary") ;
/*
** Scan To Inroads Exterior Boundary Feature
*/
 process = 1 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && process ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMF_STATE_TIN && dtmFeatureP->dtmFeatureType == DTMF_POLYGON )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"InRoads DTM Exterior Boundary Found") ;
       if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&firstPoint)) goto errexit ;
       if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,firstPoint,&area,&direction) ) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Exterior Boundary Area = %12.5lf Direction = %2ld",area,direction) ;
       if( direction == DTM_CLOCKWISE ) if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,firstPoint)) goto errexit ;
       if( bcdtmList_cleanTptrPolygonDtmObject(dtmP,firstPoint)) goto errexit ;
       if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
       if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,firstPoint,dbg)) goto errexit ;
       if( bcdtmClip_toTptrPolygonDtmObject(dtmP,firstPoint,DTM_EXTERNAL)) goto errexit ;
       bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
       process = 0 ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Trimming To Inroads Exterior Boundary Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Trimming To Inroads Exterior Boundary Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMPrivate int bcdtmInroads_deletePointDtmObject
(
 BC_DTM_OBJ *dtmP,                   // ==> Pointer To DTM
 long       point                    // ==> Point To Delete
)
/*
** This Routine Deletes A Points from the Circular List
*/
{
 int ret=DTM_SUCCESS,dbg=0 ;
 long clc,clp ;

// Write Entry Message

 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Deleting Point") ;
    bcdtmWrite_message(0,0,0,"dtmP   = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"point  = %8ld",point) ;
   }

/*
** Validate Point
*/
 if( point >=0 && point <= dtmP->numPoints )
   {
    if( ( clc = nodeAddrP(dtmP,point)->cPtr ) != dtmP->nullPtr )
      {

//     Scan Point And Delete Lines

       while ( clc != dtmP->nullPtr )
         {
          clp = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmList_deleteLineDtmObject(dtmP,point,clp)) goto errexit ;
         }
/*
**     Null Data Point
*/
       nodeAddrP(dtmP,point)->cPtr = dtmP->nullPtr ;
       nodeAddrP(dtmP,point)->fPtr = dtmP->nullPtr ;
       nodeAddrP(dtmP,point)->hPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,point)->tPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,point)->sPtr = dtmP->nullPnt ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Point Error") ;
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
DTMPrivate int bcdtmInroads_removeBoundingRectangleDtmObject
(
 BC_DTM_OBJ *dtmP
)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 long point ;

// Write Entry Message

 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Inroads Bounding Rectangle") ;

// Set Bounding Cube

 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;

// Find And Delete Closest Point To Bounding Cube Points

 bcdtmFind_closestPointDtmObject(dtmP,dtmP->xMin,dtmP->yMin,&point) ;
 if( bcdtmInroads_deletePointDtmObject(dtmP,point)) goto errexit ;

 bcdtmFind_closestPointDtmObject(dtmP,dtmP->xMax,dtmP->yMin,&point) ;
 if( bcdtmInroads_deletePointDtmObject(dtmP,point)) goto errexit ;

 bcdtmFind_closestPointDtmObject(dtmP,dtmP->xMax,dtmP->yMax,&point) ;
 if( bcdtmInroads_deletePointDtmObject(dtmP,point)) goto errexit ;

 bcdtmFind_closestPointDtmObject(dtmP,dtmP->xMin,dtmP->yMax,&point) ;
 if( bcdtmInroads_deletePointDtmObject(dtmP,point)) goto errexit ;

// Reset Convex Hull

 if( bcdtmList_setConvexHullDtmObject(dtmP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Inroads Bounding Rectangle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Inroads Bounding Rectangle Error") ;
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
NATDLLEXP int bcdtmInRoads_testForConnectedPointsDtmObject(BC_DTM_OBJ *dtmP,long point1,long point2)
{
 return(bcdtmList_testLineDtmObject(dtmP,point1,point2)) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMPrivate int bcdtmInroads_fixTopologyDtmObject(BC_DTM_OBJ *dtmP)
/*
**  This Is A special Purpose Function
**  Should Not Be Used For Any Other Purpose Than Finding
**  And Fixing Topology Errors In Imported Inroads DTM's
**
*/
{
 int   ret=DTM_SUCCESS,dbg=0 ;
 long  p1,p2,p3,clPtr ;
 BC_DTM_OBJ *temP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Fixing Topology DTM Object") ;

/*
**   Process Circular List for Each Data Point
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    clPtr = nodeAddrP(dtmP,p1)->cPtr ;
    if( clPtr != dtmP->nullPtr && nodeAddrP(dtmP,p1)->hPtr == dtmP->nullPnt )
      {
       if( (p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while( clPtr != dtmP->nullPtr )
         {
          p3 = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          if( ! bcdtmList_testLineDtmObject(dtmP,p2,p3) && bcdtmMath_pointSideOfDtmObject(dtmP,p1,p3,p2) > 0 )
            {
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"Topology Error Found ** Unconnected Points %8ld %8ld",p2,p3) ;
                bcdtmWrite_message(0,0,0,"P2 = %8ld ** %12.5lf %12.5lf %10.4lf",p2,pointAddrP(dtmP,p2)->X,pointAddrP(dtmP,p2)->Y,pointAddrP(dtmP,p2)->Z) ;
                bcdtmWrite_message(0,0,0,"P3 = %8ld ** %12.5lf %12.5lf %10.4lf",p3,pointAddrP(dtmP,p3)->X,pointAddrP(dtmP,p3)->Y,pointAddrP(dtmP,p3)->Z) ;
               }
             bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld dtmP->clPtr = %8ld",dtmP->numPoints,dtmP->cListPtr) ;
             if( bcdtmList_insertLineDtmObject(dtmP,p2,p3)) goto errexit ;
            }
          p2 = p3 ;
         }
      }
   }
 if( bcdtmList_setConvexHullDtmObject(dtmP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Fixing Topology DTM Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Fixing Topology DTM Object Error") ;
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
NATDLLEXP int bcdtmInRoads_getPaddingTrianglesForExteriorBoundary
(
 P3D   *hullPtsP ,                                                    // ==> Pointer To Exterior Boundary Points
 long  numHullPts ,                                                   // ==> Number Of Exterior Boundary Points
 int  (trianglesCallBackFunctionP)( P3D trianglePts[], void *userP),  // ==> Triangle Points Call Back Function
 void  *userP                                                         // ==> User Pointer Pass Backed To User In Call Back Function
)
{
 int           ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long          p1,p2,p3,clPtr,voidTriangle ;
 double        xMin,xMax,yMin,yMax,zMin,zMax ;
 double        xdec,ydec,xinc,yinc,xrange,yrange ;
 P3D           *p3dP,rectanglePts[5],trianglePts[3] ;
 BC_DTM_OBJ    *dtmP=NULL ;
 DTM_CIR_LIST  *clistP ;
 DTM_TIN_NODE  *nodeP ;
 DPoint3d *pntP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Padding Rectangles") ;
/*
**  Get Bounding Cude For Hull Points
*/
 xMin = xMax = hullPtsP->X ;
 yMin = yMax = hullPtsP->Y ;
 zMin = zMax = hullPtsP->Z ;
 for( p3dP = hullPtsP + 1 ; p3dP < hullPtsP + numHullPts ; ++p3dP )
   {
    if( p3dP->X < xMin ) xMin = p3dP->X ;
    if( p3dP->X > xMax ) xMax = p3dP->X ;
    if( p3dP->Y < yMin ) yMin = p3dP->Y ;
    if( p3dP->Y > yMax ) yMax = p3dP->Y ;
    if( p3dP->Z < zMin ) zMin = p3dP->Z ;
    if( p3dP->Z > zMax ) zMax = p3dP->Z ;
   }
/*
** Place Rectangle Around Hull 5% Larger Than The Hull Bounding Rectangle
*/
 xrange = ( xMax - xMin ) / 20.0 ;
 yrange = ( yMax - yMin ) / 20.0 ;
 xdec = xrange ;
 ydec = yrange ;
 xinc = xrange ;
 yinc = yrange ;
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
/*
** Set Point Memory Allocation Parameters
*/
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,numHullPts*5+5,numHullPts) ;
/*
** Store Tin Hull
*/
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMF_GRAPHIC_BREAK,1,1,&dtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMF_GRAPHIC_BREAK,1,1,&dtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMF_GRAPHIC_BREAK,1,1,&dtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMF_HARD_BREAK,1,1,&dtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMF_VOID,1,1,&dtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
/*
** Store Surrounding Rectangle
*/
 rectanglePts[0].X = xMin - xdec ; rectanglePts[0].Y = yMin - ydec ; rectanglePts[0].Z = - 999 ;
 rectanglePts[1].X = xMax + xinc ; rectanglePts[1].Y = yMin - ydec ; rectanglePts[1].Z = - 999 ;
 rectanglePts[2].X = xMax + xinc ; rectanglePts[2].Y = yMax + yinc ; rectanglePts[2].Z = - 999 ;
 rectanglePts[3].X = xMin - xdec ; rectanglePts[3].Y = yMax + yinc ; rectanglePts[3].Z = - 999 ;
 rectanglePts[4].X = xMin - xdec ; rectanglePts[4].Y = yMin - ydec ; rectanglePts[4].Z = - 999 ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMF_HARD_BREAK,2,1,&dtmP->nullFeatureId,rectanglePts,5)) goto errexit ;
/*
** Triangulate DTM
*/
 if( dbg )  bcdtmWrite_message(0,0,0,"Triangulating Dtm Object ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
 DTM_NORMALISE_OPTION = FALSE ;             // To Inhibit Normalisation Of Coordinates
 DTM_DUPLICATE_OPTION = FALSE ;             // To Inhibit Merging Of Points Closer Than The Point To Point Tolerance
 dtmP->ppTol = 0.0 ;
 dtmP->plTol = 0.0 ;
 if( bcdtmObject_createTinDtmObject(dtmP,1,0.0)) goto errexit ;
 DTM_NORMALISE_OPTION = TRUE ;
 DTM_DUPLICATE_OPTION = TRUE ;
 if( dbg )  bcdtmWrite_message(0,0,0,"Triangulating Dtm Object Completed ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Check Triangulation
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(2,0,0,"Triangulation Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
   }
/*
** Scan Tin And Pass Back Triangles
*/
 for( p1 = 0 ; p1 < dtmP->numPoints  ; ++p1 )
   {
    nodeP = nodeAddrP(dtmP,p1) ;
    if( ( clPtr = nodeP->cPtr) != dtmP->nullPtr )
      {
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       while ( clPtr != dtmP->nullPtr )
         {
          clistP = clistAddrP(dtmP,clPtr) ;
          clPtr  = clistP->nextPtr ;
          p3     = clistP->pntNum ;
          if( p2 > p1 && p3 > p1 && nodeP->hPtr != p2 )
            {
             if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
             if( voidTriangle == FALSE )
               {
                pntP = pointAddrP(dtmP,p1) ;
                trianglePts[0].X = pntP->X ;
                trianglePts[0].Y = pntP->Y ;
                trianglePts[0].Z = pntP->Z ;
                pntP = pointAddrP(dtmP,p2) ;
                trianglePts[1].X = pntP->X ;
                trianglePts[1].Y = pntP->Y ;
                trianglePts[1].Z = pntP->Z ;
                pntP = pointAddrP(dtmP,p3) ;
                trianglePts[2].X = pntP->X ;
                trianglePts[2].Y = pntP->Y ;
                trianglePts[2].Z = pntP->Z ;
                if ( trianglesCallBackFunctionP(trianglePts,userP)) goto errexit ;
               }
            }
          p2 = p3 ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 DTM_NORMALISE_OPTION = TRUE ;
 DTM_DUPLICATE_OPTION = TRUE ;
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Padding Rectangles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Padding Rectangles Error") ;
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
NATDLLEXP int bcdtmInRoads_resolveVoidsDtmObject
(
 BC_DTM_OBJ *dtmP           // ==> Pointer To Exterior Boundary Points
)
{
 int ret=DTM_SUCCESS,dbg=0,cdbg=0;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Resolving Inroads Voids") ;
    bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
   }
/*
** Test For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

/*
** Clean Up
*/
 cleanup :
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Inroads Voids Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Inroads Voids Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
#endif

///<summary>Set the feature callback up for CreateFeatures on import</summary>
///<author>james.goode</author>
///<date>9/2011</date>
NATDLLEXP void bcdtmInRoads_setFeatureCallbackFunction(void (__stdcall *pFunc)(DTM_FEATURE_ID featureId, LPWSTR featureDefinitionName, LPWSTR featureName,  LPWSTR description, int featureType, P3D* points, int numPoints))
    {
    pFeatureCallbackFunc = pFunc;
    }

///<summary>Release the feature callback</summary>
///<author>james.goode</author>
///<date>9/2011</date>
NATDLLEXP void bcdtmInRoads_releaseFeatureCallbackFunction()
    {
    pFeatureCallbackFunc = NULL;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMPrivate int bcdtmInroads_loadTrianglesFromDtmObject
(
 BC_DTM_OBJ *dtmP,
 int (*loadFunctionP)(long trgNum, long trgPnt1, long trgPnt2,long trgPnt3, long voidTriangle, long side1Trg, long side2Trg, long side3Trg)
)
 /*
 ** This Function Loads Inroads Triangles From A DTM Object
 */
    {
    int  ret=DTM_SUCCESS,dbg=0 ;
    long p1,p2,p3,clc,numTriangles,voidTriangle ;
    long trgNum,side1Trg,side2Trg,side3Trg ;
    DTM_CIR_LIST *clistP ;
    DTM_MX_TRG_INDEX *indexP,*trgIndexP=NULL ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Loading Inroads Triangles From BcCivil DTM") ;
        bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"loadFunctionP  = %p",loadFunctionP) ;
        }
    /*
    ** Test For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check DTM Is In Triangulated State
    */
    if( dtmP->dtmState != DTM_STATE_TIN )
        {
        bcdtmWrite_message(2,0,0,"Method Requires Triangulated Dtm") ;
        goto errexit ;
        }
    /*
    ** Check Load Function Is Set
    */
    if( loadFunctionP == NULL )
        {
        bcdtmWrite_message(1,0,0,"Load Function Not Set") ;
        goto errexit ;
        }
    /*
    ** Allocate Memory For Triangle Index
    */
    trgIndexP = ( DTM_MX_TRG_INDEX * ) malloc ( dtmP->numTriangles * sizeof( DTM_MX_TRG_INDEX )) ;
    if( trgIndexP == NULL )
        {
        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
        }
    /*
    ** Populate Triangle Index
    */
    numTriangles = 0 ;
    indexP = trgIndexP ;
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
        {
        (trgIndexP+p1)->index = numTriangles ;
        if( nodeAddrP(dtmP,p1)->cPtr != dtmP->nullPtr )
            {
            clc = nodeAddrP(dtmP,p1)->cPtr;
            if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) return(1) ;
            while( clc != dtmP->nullPtr )
                {
                clistP = clistAddrP(dtmP,clc) ;
                p3     = clistP->pntNum ;
                clc    = clistP->nextPtr ;
                if( p2 > p1 && p3 > p1 && nodeAddrP(dtmP,p3)->hPtr != p1 )
                    {
                    if( numTriangles >= dtmP->numTriangles )
                        {
                        bcdtmWrite_message(2,0,0,"Too Many Index Triangles %6ld of %6ld ",numTriangles,dtmP->numTriangles) ;
                        goto errexit ;
                        }
                    indexP->trgPnt1 = p1 ;
                    indexP->trgPnt2 = p2 ;
                    indexP->trgPnt3 = p3 ;
                    ++indexP ;
                    ++numTriangles   ;
                    }
                p2 = p3 ;
                }
            }
        }
    /*
    ** Check Index Size
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"numTriangles = %8ld ** dtmP->numTriangles = %8ld",numTriangles,dtmP->numTriangles) ;
    if( numTriangles != dtmP->numTriangles )
        {
        bcdtmWrite_message(1,0,0,"Triangle Index Size Incorrect") ;
        goto errexit ;
        }
    /*
    ** Scan Triangle Index And Load Triangles
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Loading Inroads Triangles") ;
    for( trgNum = 0 ,indexP = trgIndexP ; indexP < trgIndexP + numTriangles ; ++indexP , ++trgNum )
        {
        /*
        **  Get Adjancies For Each Triangle Side
        */
        side1Trg = -1 ;
        if( nodeAddrP(dtmP,indexP->trgPnt2)->hPtr != indexP->trgPnt1 )
            {
            if( ( p1 = bcdtmList_nextClkDtmObject(dtmP,indexP->trgPnt2,indexP->trgPnt1)) < 0 ) goto errexit ;
            if( bcdtmInroads_getInroadsTriangleNumberDtmObject(dtmP,trgIndexP,p1,indexP->trgPnt2,indexP->trgPnt1,&side1Trg))
                {
                bcdtmWrite_message(1,0,0,"Failed To Get Adjacency For Side1") ;
                goto errexit ;
                }
            }
        side2Trg = -1 ;
        if( nodeAddrP(dtmP,indexP->trgPnt3)->hPtr != indexP->trgPnt2 )
            {
            if( ( p1 = bcdtmList_nextClkDtmObject(dtmP,indexP->trgPnt3,indexP->trgPnt2)) < 0 ) goto errexit ;
            if( bcdtmInroads_getInroadsTriangleNumberDtmObject(dtmP,trgIndexP,p1,indexP->trgPnt3,indexP->trgPnt2,&side2Trg) )
                {
                bcdtmWrite_message(1,0,0,"Failed To Get Adjacency For Side2") ;
                goto errexit ;
                }
            }
        side3Trg = -1 ;
        if( nodeAddrP(dtmP,indexP->trgPnt1)->hPtr != indexP->trgPnt3 )
            {
            if( ( p1 = bcdtmList_nextClkDtmObject(dtmP,indexP->trgPnt1,indexP->trgPnt3)) < 0 ) goto errexit ;
            if( bcdtmInroads_getInroadsTriangleNumberDtmObject(dtmP,trgIndexP,p1,indexP->trgPnt1,indexP->trgPnt3,&side3Trg))
                {
                bcdtmWrite_message(1,0,0,"Failed To Get Adjacency For Side3") ;
                goto errexit ;
                }
            }
        /*
        **  Test For Void triangles
        */
        if( bcdtmList_testForVoidTriangleDtmObject(dtmP,indexP->trgPnt1,indexP->trgPnt2,indexP->trgPnt3,&voidTriangle)) goto errexit ;
        /*
        **  Call Load Function
        */
        if( voidTriangle ) ++numVoidTriangles ;
        else               ++numNoneVoidTriangles ;
        if( loadFunctionP(trgNum,indexP->trgPnt1,indexP->trgPnt2,indexP->trgPnt3,voidTriangle,side1Trg,side2Trg,side3Trg)) goto errexit ;
        }
    /*
    ** Clean Up
    */
cleanup :
    if( trgIndexP != NULL ) { free(trgIndexP) ; trgIndexP = NULL ; }
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Inroads Triangles From BcCivil DTM Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Inroads Triangles From BcCivil DTM Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
#ifdef NN
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
NATDLLEXP int bcdtmInRoads_importGeopakTinFromInroadsDtm
(
 double maxTriLength,                  // Maximum triangle side length
 long  numTinPoints,                   // Number Of Triangulated Tin Points In InRoads Dtm
 long  numTinFeatures,                 // Number Of Tin Dtm Features In Inroads Dtm
 wchar_t  *geopakTinFileNameP,         // Geopak Tin File Name
 int  (*setGeopakCallBackFunctionsP)() // Inroads Call Back Functions To Set Geopak Call Back Functions
)
{
 int ret=DTM_SUCCESS,dbg=0,cdbg=0;
 long dtmFeature,voidsInDTM,numHullPts=0 ;
 long exteriorBoundary,trimHull=TRUE,removeBoundingRectangle=TRUE  ;
 P3D  *hullPtsP=NULL ;
 BC_DTM_OBJ *dtmP=NULL ;
 BC_DTM_OBJ *temP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;
 DTM_FEATURE_ID hullId ;
 DTM_USER_TAG   hullUserTag ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Importing Geopak Tin From Inroads Dtm") ;
    bcdtmWrite_message(0,0,0,"maxTriLength                = %8.3lf",maxTriLength) ;
    bcdtmWrite_message(0,0,0,"numTinPoints                = %8ld",numTinPoints) ;
    bcdtmWrite_message(0,0,0,"numTinFeatures              = %8ld",numTinFeatures) ;
    bcdtmWrite_message(0,0,0,"geopakTinFileNameP          = %s",geopakTinFileNameP) ;
    bcdtmWrite_message(0,0,0,"setGeopakCallBackFunctionsP = %p",setGeopakCallBackFunctionsP) ;
   }
/*
** Validate Input
*/
 if( numTinPoints < 3 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Tin Points Is Less Than 3") ;
//    goto errexit ;
   }
/*
** Save Geopak Active
*/
//DHIsthis required? geopakActiveSetting = DTM_GEOPAK_ACTIVE ;
/*
** Create Dtm Object For Importing Inroads Dtm
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
/*
** Allocate Initial Memory For Tin Points - Not Necessary But Helps Manage Memory Better
*/
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,numTinPoints,10000)) goto errexit ;
/*
** Allocate Initial Memory For Tin Features - Not Necessary But Helps Manage Memory Better
*/
 if( numTinFeatures > 0 ) if( bcdtmObject_setFeatureMemoryAllocationParametersDtmObject(dtmP,numTinFeatures,numTinFeatures)) goto errexit ;
/*
** Call Inroads Call Back Function To Set Geopak Call Back Functions
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Importing DTM") ;

//ToDo if( setGeopakCallBackFunctionsP(dtmP,bcdtmInRoads_geopakPointsCallBackFunction,bcdtmInRoads_geopakCircularListCallBackFunction,bcdtmInroads_dtmFeaturePointsCallBackFunction) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"DTM Imported") ;
/*
** Check For Voids In Imported DTM
*/
 voidsInDTM = FALSE ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && voidsInDTM == FALSE ; ++dtmFeature )
   {
    if( ftableAddrP(dtmP,dtmFeature)->dtmFeatureType == DTMF_VOID ) voidsInDTM = TRUE ;
   }
/*
**  Resolve Voids If They Are Present
*/
 if( voidsInDTM == TRUE )
   {
//    if( bcdtmInroads_resolveVoidsDtmObject(dtmP)) goto errexit ;
   }
/*
** Check For Untriangulated DTM
*/
 if( dtmP->nodesPP == NULL )
   {
    trimHull = FALSE ;
/*
**  Triangulate DTM
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
    if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
   }
/*
** Check DTM Is In Tin State
*/
 if( dtmP->dtmState != DTM_STATE_TIN )
   {
    dtmP->dtmState = DTM_STATE_TIN ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Setting Convex Hull") ;
    if( bcdtmList_setConvexHullDtmObject(dtmP)) goto errexit ;
/*
**  Check Tin Topology
*/
    if( bcdtmCheck_topologyDtmObject(dtmP,0))
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Tin Topology Invalid")  ;
       if( bcdtmInroads_fixTopologyDtmObject(dtmP)) goto errexit  ;
       if( bcdtmCheck_topologyDtmObject(dtmP,0))
         {
          bcdtmWrite_message(1,0,0,"Tin Topology Invalid") ;
          goto errexit ;
         }
      }
/*
**  Check Tin Precision
*/
    if( cdbg == 1 )
      {
       if( bcdtmCheck_precisionDtmObject(dtmP,0) )
         {
          bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ;
          goto errexit ;
         }
       else if( dbg )  bcdtmWrite_message(0,0,0,"Tin Precision Valid") ;
      }
/*
**  Clean Tin
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning DTM Object") ;
    if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
   }
/*
** Check Triangulation Prior To Removing External Void Triangles
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Triangulated Dtm After Inserting Features") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(1,0,0,"Triangulated Dtm Invalid") ;
       goto errexit ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Triangulated Dtm Valid") ;
   }
/*
** Remove All Void Triangles External To Inroads Exterior Boundary
*/
 if( trimHull == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Void Triangles") ;

//  Check Exterior Boundary Is Present

    exteriorBoundary = FALSE ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && exteriorBoundary == FALSE ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMF_STATE_TIN && dtmFeatureP->dtmFeatureType == DTMF_POLYGON )
         {
          exteriorBoundary = TRUE ;
          hullId = dtmFeatureP->dtmFeatureId ;
          hullUserTag = dtmFeatureP->dtmUserTag ;
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&hullPtsP,&numHullPts)) goto errexit ;
         }
      }

//  Trim To Exterior Boundary

    if( exteriorBoundary == TRUE )
      {
       if( bcdtmInroads_trimToInroadsExteriorBoundaryDtmObject(dtmP)) goto errexit ;

//     Store Hull Feature

       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMF_HULL,hullUserTag,2,&hullId,hullPtsP,numHullPts)) goto errexit ;

//     Set Hull Feature To Tin State

       if( dbg ) bcdtmWrite_message(0,0,0,"Setting Hull Feature To Tin state") ;
       dtmFeatureP = ftableAddrP(dtmP,dtmP->numFeatures-1) ;
       if( dtmFeatureP->dtmFeaturePts.offsetPI != 0 )
         {
          bcdtmMemory_free(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI) ;
          dtmFeatureP->dtmFeaturePts.offsetPI = 0 ;
         }
       dtmFeatureP->dtmFeatureState = DTMF_STATE_TIN ;
       dtmFeatureP->dtmFeaturePts.firstPoint = dtmP->hullPoint ;
      }

//  Remove Inroads Bounding Rectangle

    else if ( removeBoundingRectangle == TRUE )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Removing Bounding Rectangle") ;
       if( bcdtmInroads_removeBoundingRectangleDtmObject(dtmP)) goto errexit ;

//     Remove Max Edge Triangles

       if( maxTriLength > 0.0 )
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"maxTriLength = %10.4lf",maxTriLength) ;
          if( bcdtmTin_removeExternalMaxSideTrianglesDtmObject(dtmP,maxTriLength)) goto errexit ;
         }
      }

//  Check Tin

    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       if( bcdtmObject_createDtmObject(&temP)) goto errexit ;
       if( bcdtmGeopak_cloneDtmObject(dtmP,temP)) goto errexit ;
       bcdtmObject_initialiseDtmObject(dtmP) ;
       *dtmP = *temP ;
       temP->fTablePP = NULL ;
       temP->pointsPP = NULL ;
       temP->nodesPP  = NULL ;
       temP->cListPP  = NULL ;
       temP->fListPP  = NULL ;
       bcdtmObject_destroyDtmObject(&temP) ;
      }

/*
** Check Triangulation After Removing External Void Triangles
*/
    if( cdbg )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Triangulated Dtm After Removing External Void Triangles") ;
       if( bcdtmCheck_tinComponentDtmObject(dtmP))
         {
          bcdtmWrite_message(1,0,0,"Triangulated Dtm Invalid") ;
          goto errexit ;
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"Triangulated Dtm Valid") ;
      }
   }
/*
** Clean DTM Object
*/
 if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;

//  Report Feature Ids

/*
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    bcdtmWrite_message(0,0,0,"dtmFeature[%8ld] ** TYPE = %4ld State = %4ld Id = %10I64d",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeatureState,dtmFeatureP->dtmFeatureId) ;
   }
  bcdtmWrite_message(0,0,0,"dtmP->dtmFeatureIndex = %10ld",dtmP->dtmFeatureIndex) ;
*/

/*
** Check Triangulation After Cleaning Triangulated DTM Object
*/
 if( cdbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Checking Triangulated Dtm") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(1,0,0,"Triangulated Dtm Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Triangulated Dtm Valid") ;
   }
/*
** Save As Geopak Tin
*/
 if( glbImportMode ) glbDtmP = dtmP ;
 else
   {
    DTM_GEOPAK_ACTIVE = 1 ;
    if( bcdtmWrite_toFileDtmObject(dtmP,geopakTinFileNameP)) goto errexit ;
   }
/*
** Set triangulation options for the dtm from inroads settings
*/
  if (maxTriLength!=0.0)
    {
    dtmP->edgeOption = 3;
    dtmP->maxSide = fabs(maxTriLength);
    }
  else
    {
    dtmP->edgeOption = 1;
    dtmP->maxSide = 100;
    }

//  Log Imported DTM Stats

 if( dbg == 1 )
   {
    bcdtmObject_reportStatisticsDtmObject(dtmP) ;
    bcdtmWrite_toFileDtmObject(dtmP,L"importedInroadsDtm.bcdtm") ;
   }

/*
** Clean Up
*/
 cleanup :
//DHIsthis required?  DTM_GEOPAK_ACTIVE = geopakActiveSetting ;
 if( hullPtsP != NULL ) free(hullPtsP) ;
 if( ! glbImportMode && dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Geopak Tin From Inroads Dtm Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Geopak Tin From Inroads Dtm Error") ;
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
NATDLLEXP int bcdtmInRoads_importTinFromDtmObject
(
 BC_DTM_OBJ *dtmP,
 int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long numFeatures),
 int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
 int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
 int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
 int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 BcDTMUserTag,__int64 BcDTMFeatureId,long *pointIndicesP,long numPointIndices)
)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 long node,numRandomPoints=0,numFeaturePoints=0 ;
 double xinc,xdec,ydec,yinc,xrange,yrange ;
 DTM_TIN_NODE *nodeP ;
 DPoint3d *pntP ;
 DTM_FEATURE_ID islandFeatureId ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Importing Tin From Dtm Object") ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM Is In Triangulated State
*/
 if( dtmP->dtmState != DTM_STATE_TIN )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated Dtm") ;
    goto errexit ;
   }
/*
** Place Rectangle Around Tin 5% larger than the Tin Hull bounding rectangle
*/
 xrange = ( dtmP->xMax - dtmP->xMin ) / 20.0 ;
 yrange = ( dtmP->yMax - dtmP->yMin ) / 20.0 ;
 xdec = xrange ;
 ydec = yrange ;
 xinc = xrange ;
 yinc = yrange ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Insert Rectangle Around Tin And Fill With Triangles") ;
 if( bcdtmInRoads_insertRectangleAroundTinDtmObject(dtmP,xdec,xinc,ydec,yinc,&islandFeatureId)) goto errexit ;
/*
** Count Number Of Random And Feature Points
*/
 for( node = 0 ; node < dtmP->numNodes ; ++node )
   {
    nodeP = nodeAddrP(dtmP,node) ;
    if( nodeP->fPtr == dtmP->nullPtr ) ++numRandomPoints ;
    else                               ++numFeaturePoints ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"numRandomPoints = %8ld numFeaturePoints = %8ld",numRandomPoints,numFeaturePoints) ;
/*
** Adjust Point Counts For Inroads Bounding Rectangle Void Feature
*/
 numRandomPoints  = numRandomPoints  + 4 ;
 numFeaturePoints = numFeaturePoints - 4 ;
 if( numFeaturePoints < 0 ) numFeaturePoints = 0 ;
/*
** Call Inroads Tin Stats Function
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calling InRoads Tin Stats Call Back Function") ;
 if( tinStatsCallBackFunctionP(numRandomPoints,numFeaturePoints,dtmP->numTriangles,dtmP->numFeatures)) goto errexit ;
/*
** Call InRoads Random Points Function
*/
 numRandomPoints = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Calling InRoads Random Points Function") ;
 for( node = 0 ; node < dtmP->numNodes ; ++node )
   {
    nodeP = nodeAddrP(dtmP,node) ;
    if( nodeP->fPtr == dtmP->nullPtr )
      {
       pntP = pointAddrP(dtmP,node) ;
       if( tinRandomPointsCallBackFunctionP(node,pntP->X,pntP->Y,pntP->Z)) goto errexit ;
       ++numRandomPoints ;
       }
    else                  // Check For Inroads Bounding Rectangle Void
      {
       if( flistAddrP(dtmP,nodeP->fPtr)->dtmFeature == dtmP->numFeatures - 1 )
         {
          pntP = pointAddrP(dtmP,node) ;
          if( tinRandomPointsCallBackFunctionP(node,pntP->X,pntP->Y,pntP->Z)) goto errexit ;
          ++numRandomPoints ;
         }
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Random Points Written = %8ld",numRandomPoints) ;
/*
** Call InRoads Feature Points Function
*/
 numFeaturePoints = 0 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Calling InRoads Feature Points Call Back Function") ;
 for( node = 0 ; node < dtmP->numNodes ; ++node )
   {
    nodeP = nodeAddrP(dtmP,node) ;
    if( nodeP->fPtr != dtmP->nullPtr ) // Check For Inroads Bounding Rectangle Void
      {
       if( flistAddrP(dtmP,nodeP->fPtr)->dtmFeature != dtmP->numFeatures - 1 )
         {
          pntP = pointAddrP(dtmP,node) ;
          if( tinFeaturePointsCallBackFunctionP(node,pntP->X,pntP->Y,pntP->Z)) goto errexit ;
          ++numFeaturePoints ;
         }
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Feature Points Written = %8ld",numFeaturePoints) ;
/*
** Call Inroads Triangle Function
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calling InRoads Triangles Call Back Function") ;
 if( bcdtmInroads_loadTrianglesFromDtmObject(dtmP,tinTrianglesCallBackFunctionP)) goto errexit ;
/*
** Call Inroads Feature Function
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calling InRoads Features Call Back Function") ;
 if( bcdtmInroads_loadFeaturesFromDtmObject(dtmP,islandFeatureId,tinFeaturesCallBackFunctionP)) goto errexit ;
/*
** Remove InRoad's simulated triangles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Simulated Inroads Triangles") ;
 if( bcdtmInroads_clipUsingIslandFeatureIdDtmObject(dtmP,islandFeatureId)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Tin From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Tin From Dtm Object Error") ;
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
NATDLLEXP int bcdtmInRoads_importGeopakTinFromFile
(
 wchar_t *tinFileNameP,
 int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures),
 int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
 int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
 int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
 int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 BcDTMUserTag,__int64 BcDTMFeatureId,long *pointIndicesP,long numPointIndices)
)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Importing From Geopak Tin File") ;
/*
** Read Geopak Tin File To bcLIB DTM
*/
 if( glbImportMode ) dtmP = glbDtmP ;
 else if( bcdtmRead_fromFileDtmObject(&dtmP,tinFileNameP)) goto errexit ;
/*
** Import From Dtm Object
*/
 if( bcdtmInRoads_importTinFromDtmObject(dtmP,tinStatsCallBackFunctionP,tinRandomPointsCallBackFunctionP,tinFeaturePointsCallBackFunctionP,tinTrianglesCallBackFunctionP,tinFeaturesCallBackFunctionP)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"numVoidTriangles     = %8ld",numVoidTriangles) ;
    bcdtmWrite_message(0,0,0,"numNoneVoidTriangles = %8ld",numNoneVoidTriangles) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( ! glbImportMode && dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing From Geopak Tin File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing From Geopak Tin File Error") ;
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
NATDLLEXP int bcdtmInRoads_exportBclibDtmToInroadsDtmFile(BC_DTM_OBJ *dtmP, wchar_t *dtmFileNameP,wchar_t *nameP,wchar_t *descriptionP )
{
 int ret=DTM_SUCCESS,status=0,dbg=0 ;
 size_t  size ;
 char  tinFileName[10] ;
 wchar_t *w_tinFileNameP=NULL, *w_dtmFileNameP=NULL, *w_nameP=NULL, *w_descriptionP=NULL ;
/*
**  Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Exporting bcLIB DTM To Inroads DTM File") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFileNameP   = %s",dtmFileNameP) ;
    bcdtmWrite_message(0,0,0,"nameP          = %s",nameP) ;
    bcdtmWrite_message(0,0,0,"descriptionP   = %s",descriptionP) ;
   }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTM_STATE_TIN )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Convert To Wide Characters
*/
/*
 size = mbstowcs(NULL,dtmFileNameP,strlen(dtmFileNameP)) ;
 if(( w_dtmFileNameP = malloc((size+1)*sizeof(wchar_t))) == NULL ) goto errexit ;
 mbstowcs(w_dtmFileNameP,dtmFileNameP,strlen(dtmFileNameP) + 1) ;

 size = mbstowcs(NULL,nameP,strlen(nameP)) ;
 if(( w_nameP = malloc((size+1)*sizeof(wchar_t))) == NULL ) goto errexit ;
 mbstowcs(w_nameP,nameP,strlen(nameP) + 1) ;

 size = mbstowcs(NULL,descriptionP,strlen(descriptionP)) ;
 if(( w_descriptionP = malloc((size+1)*sizeof(wchar_t))) == NULL ) goto errexit ;
 mbstowcs(w_descriptionP,descriptionP,strlen(descriptionP) + 1) ;
*/

 strcpy(tinFileName,"xxxx") ;
 size = mbstowcs(NULL,tinFileName,strlen(tinFileName)) ;
 if(( w_tinFileNameP = (wchar_t*)malloc((size+1)*sizeof(wchar_t))) == NULL ) goto errexit ;
 mbstowcs(w_tinFileNameP,tinFileName,strlen(tinFileName) + 1) ;
/*
**  Set Golbal Import Mode Switch To Override The Creation Of A Geopak Tin File
**  And Set A Global Pointer To The DTM Object Created During The Import Geopak Tin File Method
*/
 glbImportMode = 1 ;
/*
** Set Global Pointer To Object
*/
 glbDtmP = dtmP ;
/*
**  Call Inroads Function To Initiate Import Of A Geopak Tin File
**  For The Purpose Of Importing Directly To An In Memory bcLIB DTM Object
**  We Will Use A Dummy Tin File Name And Pass The Pointer To The DTM
**
*/
 status = dtmLink_setConvertGPKTinToDTMFunction(bcdtmInRoads_importGeopakTinFromFile) ;
 if( status < 0  )
   {
    bcdtmWrite_message(1,0,0,"Failed To Load InRoads DLLs ** Error = %u",GetLastError()) ;
    goto errexit ;
   }
// if( ( status = dtmLink_convertGPKTinToDTM(w_tinFileNameP, w_dtmFileNameP, w_nameP, w_descriptionP)) != DTM_SUCCESS )
 if( ( status = dtmLink_convertGPKTinToDTM(w_tinFileNameP, dtmFileNameP, nameP, descriptionP)) != DTM_SUCCESS )
   {
    bcdtmWrite_message(1,0,0,"Failed To Export Inroads DTM File ** Inroads Error = %6ld",status) ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 glbDtmP = NULL ;
 glbImportMode = 0 ;
 if( w_dtmFileNameP != NULL ) free(w_dtmFileNameP) ;
 if( w_nameP        != NULL ) free(w_nameP) ;
 if( w_descriptionP != NULL ) free(w_descriptionP) ;
 if( w_tinFileNameP != NULL ) free(w_tinFileNameP) ;
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Exporting bcLIB DTM To Inroads DTM File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Exporting bcLIB DTM To Inroads DTM File Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
#ifdef NNN
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
NATDLLEXP int bcdtmInRoads_importGeopakTinFromTinObject
(
 DTM_TIN_OBJ *tinP,
 int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures),
 int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
 int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
 int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
 int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 BcDTMUserTag,__int64 BcDTMFeatureId,long *pointIndicesP,long numPointIndices)
)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Importing From Geopak Tin Object") ;
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidTinObject(tinP)) goto errexit ;
/*
** Copy Tin Object To Dtm Object
*/
 if( bcdtmObject_copyTinObjectToDtmObject(tinP,&dtmP)) goto errexit ;
/*
** Import From Dtm Object
*/
 if( bcdtmInRoads_importTinFromDtmObject(dtmP,tinStatsCallBackFunctionP,tinRandomPointsCallBackFunctionP,tinFeaturePointsCallBackFunctionP,tinTrianglesCallBackFunctionP,tinFeaturesCallBackFunctionP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
**      Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing From Geopak Tin Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing From Geopak Tin Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
#endif
void inroadsTM_addCPFeature (__int64 featureID, LPWSTR featureName, LPWSTR featureDefinition);
void inroadsTM_clearCPFeatureNamesList ();

struct ImporterArg
    {
//    const InroadsImporter* importer;
    RefCountedPtr<IBcDTM> dtm;
    bool foundPerimeter;
    RefCountedPtr<IBcDTM> tempDtmP;
//    InroadsTriangulationPreserver* adjustTriangulation;
    };

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static void AddFeature (void* dat, bvector<DPoint3d>& points, long type, struct CIVdtmftr* ftrP)
    {
    if (points.size() > 0)
        {
        ImporterArg* pArg = (ImporterArg*)dat;
        wchar_t style[CIV_C_NAMSIZ+1];
        memset(style, 0, sizeof(style));
        if (ftrP->numStyles > 0)
            memcpy (style, ftrP->s1->nam, sizeof(ftrP->s1->nam));

        if (ftrP->flg & DTM_C_FTRTIN)
            {
             if (pFeatureCallbackFunc)
                 pFeatureCallbackFunc( DTM_NULL_FEATURE_ID, style, ftrP->nam, ftrP->des, type, (P3D*)&points[0], (int)points.size ());
            }
        else
            {
            if (type == DTMF_RANDOM_SPOT) // DTMF_RandomSpots
                pArg->dtm->addRandomSpots (&points[0], (int)points.size ());
            else
                {
                BcDTMUserTag BcDTMUserTag = 0;
                BcDTMFeatureId id;
                if (type == DTMF_GROUP_SPOT)
                    pArg->dtm->addSpotFeature(&points[0], (int)points.size (), BcDTMUserTag, &id);
                else
                    pArg->dtm->addLinearFeature (type, &points[0], (int)points.size (), BcDTMUserTag, &id);
             if (pFeatureCallbackFunc)
                 pFeatureCallbackFunc( id, style, ftrP->nam, ftrP->des, type, (P3D*)&points[0], (int)points.size ());
                if (type == DTMF_HULL                                  )
                    pArg->foundPerimeter = true;
                }
            }
        points.clear ();
        }
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int SendFeaturesCallback (void* dat, struct CIVdtmsrf* srf, int ftrTyp, struct CIVdtmftr* ftrP)
    {
    if (ftrP->numPnts > 0)
        {
        long type = -1;

        switch (ftrTyp)
            {
            case DTM_C_DTMREGFTR:
                if (wcslen (ftrP->nam) > 0)
                    type = DTMF_GROUP_SPOT;
                else
                    type = DTMF_RANDOM_SPOT;
                break;

            case DTM_C_DTMBRKFTR:
                type = DTMF_HARD_BREAK;
                break;

            case DTM_C_DTMINTFTR:
                type = DTMF_VOID;
                break;

            case DTM_C_DTMEXTFTR:
                type = DTMF_HULL;
                break;

            case DTM_C_DTMCTRFTR:
                type = DTMF_CONTOUR_LINE;
                break;
            default:
                type = -1;
                break;
            }

        bvector<DPoint3d> points;
        if (ftrP->p1)
            {
            for (int i = 0; i < ftrP->numPnts; i++)
                {
                if (i > 0 && !(ftrP->p1[i].flg & DTM_C_PNTPUD) && (ftrTyp != DTM_C_DTMREGFTR && ftrTyp != DTM_C_DTMEXTFTR))
                    {
                    AddFeature (dat, points, type, ftrP);
                    points.clear ();
                    }

                if ((ftrP->p1[i].flg & DTM_C_PNTDEL) == FALSE)
                    {
                    points.push_back (ftrP->p1[i].cor);
                    }
                }

            AddFeature (dat, points, type, ftrP);
            }
        else
            {
            ftrP->p1 = nullptr;
            }
        }

    return SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int SendTriangleCallback (void* dat, long, DPoint3d*,  struct CIVdtmtin* t, unsigned long)
    {
    int side = bcdtmMath_sideOf (t->p1->cor.x, t->p1->cor.y, t->p3->cor.x, t->p3->cor.y, t->p2->cor.x, t->p2->cor.y);
    // Check for colinear triangle.
    if (side == 0)
        return DTM_SUCCESS;

    ImporterArg* pArg = (ImporterArg*)dat;
    DPoint3d pts[4];
    if (side > 0)
        {
        pts[3] = pts[0] = t->p1->cor;
        pts[1] = t->p2->cor;
        pts[2] = t->p3->cor;
        }
    else
        {
        pts[3] = pts[0] = t->p1->cor;
        pts[1] = t->p3->cor;
        pts[2] = t->p2->cor;
        }
    if (pArg->tempDtmP.IsNull())
        pArg->tempDtmP = IBcDTM::make ();
    BcDTMFeatureId featureId = DTM_NULL_FEATURE_ID;
    pArg->tempDtmP->addLinearFeature(BC_DTMFEATURE_GRAPHIC_BREAK, pts,4,&featureId);
    return DTM_SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static void AddSurfacePerimeter (ImporterArg& arg, struct CIVdtmsrf* pSrf)
    {
    DPoint3d* vrtsP = NULL;
    long numVrts = 0;
    if (inroadsTM_getSurfacePerimeter (&numVrts, &vrtsP, pSrf) == SUCCESS)
        {
        BcDTMUserTag BcDTMUserTag = 0;
        BcDTMFeatureId id = DTM_NULL_FEATURE_ID;
        arg.dtm->addLinearFeature (DTMF_HULL, vrtsP, numVrts, BcDTMUserTag, &id);

        arg.foundPerimeter = true;
        if (pFeatureCallbackFunc)
            pFeatureCallbackFunc(id, L"", L"", L"", DTMF_HULL, (P3D*)vrtsP, (int)numVrts);
        }
    if (vrtsP) free (vrtsP);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int addFeatureToTM(int featureType, __int64  eltId, BcDTMFeatureId id, Dpoint3d *featurePtsP, int numFeaturePts, void *userP)
    {
    BcDTMP bcDtmP = (BcDTMP)userP;
    if (featureType == BC_DTMFEATURE_TRIANGLE)
        featureType = BC_DTMFEATURE_GRAPHIC_BREAK;

    BcDTMFeatureId              featureId = DTM_NULL_FEATURE_ID;

    bcDtmP->addLinearFeature(featureType, featurePtsP, numFeaturePts, &featureId);
    if (featureType != BC_DTMFEATURE_GRAPHIC_BREAK)
        {
        if (pFeatureCallbackFunc)
            pFeatureCallbackFunc(featureId, L"", L"", L"", featureType, (P3D*)featurePtsP, (int)numFeaturePts);
        }
    return SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int addFeatureToTMFromDefinition (int featureType, __int64  eltId, BcDTMFeatureId id, Dpoint3d *featurePtsP, int numFeaturePts, void *userP)
    {
    BcDTMP bcDtmP = (BcDTMP)userP;
    if (featureType == BC_DTMFEATURE_TRIANGLE)
        featureType = BC_DTMFEATURE_GRAPHIC_BREAK;
    else
        {
        if (numFeaturePts <= 2)
            return SUCCESS;
        if (numFeaturePts == 3)
            {
            int side = bcdtmMath_sideOf (featurePtsP[0].x, featurePtsP[0].y, featurePtsP[1].x, featurePtsP[1].y, featurePtsP[2].x, featurePtsP[2].y);
            if (side == 0)
                return SUCCESS;
            }

        long direction;
        double area;
        if (bcdtmMath_getPolygonDirectionP3D ((P3D*)featurePtsP, numFeaturePts, &direction, &area) != DTM_SUCCESS)
            return DTM_SUCCESS;
        if (area < MAXAREAFORVOIDORISLANDS)
            return DTM_SUCCESS;
        // ToDo check if this void/island already exists.
        }
    BcDTMFeatureId              featureId = DTM_NULL_FEATURE_ID;

    bcDtmP->addLinearFeature (featureType, featurePtsP, numFeaturePts, &featureId);
    if (featureType != BC_DTMFEATURE_GRAPHIC_BREAK)
        {
        if (pFeatureCallbackFunc)
            pFeatureCallbackFunc (featureId, L"", L"", L"", featureType, (P3D*)featurePtsP, (int)numFeaturePts);
        }
    return SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
NATDLLEXP int bcdtmInRoads_mergeInTrianglesDtmFile (IBcDTMP dtm, IBcDTMP dtmTriangles)
    {
    if (bcdtmObject_triangulateStmTrianglesDtmObject (dtmTriangles->getTinHandle()))
        return DTM_ERROR;
    dtmTriangles->browseFeatures (BC_DTMFEATURE_VOID, 1000, dtm, &addFeatureToTMFromDefinition);
    dtmTriangles->browseFeatures (BC_DTMFEATURE_ISLAND, 1000, dtm, &addFeatureToTMFromDefinition);
    dtmTriangles->browseFeatures (BC_DTMFEATURE_TRIANGLE, 1000, dtm, &addFeatureToTMFromDefinition);
    return DTM_SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
NATDLLEXP int bcdtmInRoads_importBclibDtmFromInroadsDtmFile (BC_DTM_OBJ **dtmPP, wchar_t *dtmFileNameP)
    {
    struct CIVdtmsrf* pSrf = NULL;
    if (DTM_SUCCESS == inroadsTM_load (&pSrf, NULL, NULL, dtmFileNameP))
        {
        ImporterArg arg;
        arg.dtm = IBcDTM::make();
        arg.tempDtmP = NULL;
        arg.foundPerimeter = false;
        inroadsTM_sendAllFeatures (NULL, pSrf, DTM_C_NOBREK, 0, SendFeaturesCallback, (void*)&arg);
        if (!arg.foundPerimeter)
            AddSurfacePerimeter (arg, pSrf);

        inroadsTM_sendAllTriangles (NULL, pSrf, DTM_C_NOBREK, SendTriangleCallback, &arg);

        inroadsTM_deleteSurface (NULL, pSrf, FALSE);
        if (arg.tempDtmP.IsValid())
            {
            bcdtmInRoads_mergeInTrianglesDtmFile (arg.dtm.get(), arg.tempDtmP.get());

            if (!arg.foundPerimeter)
                {
                DPoint3d* pts;
                long numPts = 0;
                if (SUCCESS == arg.tempDtmP->getHullVertices (&pts, &numPts))
                    {
                    BcDTMFeatureId featureId = DTM_NULL_FEATURE_ID;
                    arg.dtm->addLinearFeature (BC_DTMFEATURE_HULL, pts, numPts,&featureId);
                    if (pFeatureCallbackFunc)
                        pFeatureCallbackFunc(featureId, L"", L"", L"", DTMF_HULL, (P3D*)pts, (int)numPts);
                    free (pts);
                    }
                }
            }

        *dtmPP = arg.dtm->getTinHandle();
        (*dtmPP)->refCount++;
        return DTM_SUCCESS;
        }
    return DTM_ERROR;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
NATDLLEXP void bcdtmInRoads_addCPFeature (__int64 featureID, LPWSTR featureName, LPWSTR featureDefinition)
    {
    inroadsTM_addCPFeature (featureID, featureName, featureDefinition);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
NATDLLEXP void bcdtmInRoads_clearCPFeatureNamesList ()
    {
    inroadsTM_clearCPFeatureNamesList ();
    }
}